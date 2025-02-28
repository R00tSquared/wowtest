// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/** \file
    \ingroup u2w
*/

#include <sstream>

#include "WorldSocket.h"                                    // must be first to make ACE happy with ACE includes in it
#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Group.h"
#include "Guild.h"
#include "World.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "BattleGroundMgr.h"
#include "OutdoorPvPMgr.h"
#include "Language.h"                                       // for CMSG_CANCEL_MOUNT_AURA handler
#include "Chat.h"
#include "SocialMgr.h"
#include "WardenWin.h"
#include "WardenMac.h"
#include "WardenChat.h"
#include "GuildMgr.h"
#include "TicketMgr.h"

bool MapSessionFilter::Process(WorldPacket * packet)
{
    OpcodeHandler const& opHandle = opcodeTable[packet->GetOpcode()];

    //let's check if our opcode can be really processed in Map::Update()
    if (opHandle.packetProcessing == PROCESS_INPLACE)
        return true;

    if (opHandle.packetProcessing == PROCESS_THREADUNSAFE)
        return false;

    Player * plr = m_pSession->GetPlayer();

    if (!plr)
        return false;

    //in Map::Update() we do not process packets where player is not in world!
    return plr->IsInWorld();
}

//we should process ALL packets when player is not in world/logged in
//OR packet handler is not thread-safe!

bool WorldSessionFilter::Process(WorldPacket* packet)
{
    OpcodeHandler const& opHandle = opcodeTable[packet->GetOpcode()];

    //check if packet handler is supposed to be safe
    if (opHandle.packetProcessing == PROCESS_INPLACE)
        return true;

    if (opHandle.packetProcessing == PROCESS_THREADUNSAFE)
         return true;
    //no player attached? -> our client! ^^
    Player * plr = m_pSession->GetPlayer();
    if (!plr)
        return true;
    //lets process all packets for non-in-the-world player
    return (plr->IsInWorld() == false);
}

/// WorldSession constructor
WorldSession::WorldSession(uint32 id, WorldSocket *sock, uint32 login_id, uint32 recruiter, uint32 rafcoins, time_t premium_until, uint64 permissions, uint8 expansion, LocaleConstant locale, uint32 muteRemain, std::string mute_reason, uint32 trollMuteRemain, std::string trollmute_reason, uint64 accFlags, uint16 opcDisabled, BotType botType) :
LookingForGroup_auto_join(false), LookingForGroup_auto_add(false), m_muteRemain(muteRemain), m_muteReason(mute_reason),
m_trollMuteRemain(trollMuteRemain), m_trollmuteReason(trollmute_reason), _player(NULL), m_Socket(sock),
m_permissions(permissions), _accountId(id), _loginId(login_id), _recruiterAcc(recruiter), _rafCoins(rafcoins), _premium_until(premium_until), m_expansion(expansion), m_opcodesDisabled(opcDisabled), m_botType(botType),
m_sessionDbcLocale(sWorld.GetAvailableDbcLocale(locale)), m_sessionDbLocaleIndex(sObjectMgr.GetIndexForLocale(locale)),
_logoutTime(0), m_inQueue(false), m_playerLoading(false), m_playerLogout(false), m_playerSave(false), m_playerRecentlyLogout(false), m_latency(0), m_clientTimeDelay(0), m_clientTimeDelayReset(true),
m_accFlags(accFlags), m_Warden(NULL), AntiDOS(this), m_IPHash("")
{
	_updateBotTimer.Reset(1*MILLISECONDS);

    _kickTimer.Reset(sWorld.getConfig(CONFIG_SESSION_UPDATE_IDLE_KICK));

    // sock will not exist for bot players, so check on it
    if (sock)
    {
        m_Address = sock->GetRemoteAddress();
        sock->AddReference();

        static SqlStatementID updateAccOnline;
        SqlStatement stmt = AccountsDatabase.CreateStatement(updateAccOnline, "UPDATE account SET online = 1 WHERE account_id = ?");
        stmt.PExecute(GetAccountId());
		
        if (GetPermissions() < PERM_ADM)
		    _opcodesCooldown = sObjectMgr.GetOpcodesCooldown();
    }
}

/// WorldSession destructor
WorldSession::~WorldSession()
{
    ///- unload player if not unloaded
    if (_player)
        LogoutPlayer(true);

    /// - If have unclosed socket, close it
    if (m_Socket)
    {
        m_Socket->CloseSocket ();
        m_Socket->RemoveReference ();
        m_Socket = NULL;
    }

    if (m_Warden)
        delete m_Warden;

    WorldPacket* packet;
    while (_recvQueue.next(packet))
        delete packet;

    static SqlStatementID updateAccountOnline;
    static SqlStatementID updateCharactersOnline;

    SqlStatement stmt = AccountsDatabase.CreateStatement(updateAccountOnline, "UPDATE account SET online = 0 WHERE account_id = ?");
    stmt.PExecute(GetAccountId());

    stmt = RealmDataDatabase.CreateStatement(updateCharactersOnline, "UPDATE characters SET online = 0 WHERE account = ?");
    stmt.PExecute(GetAccountId());

    if ((m_permissions >= PERM_GM_TRIAL) && !(m_permissions & PERM_ADM))
        sTicketMgr.RemoveGm(this);
}

void WorldSession::SizeError(WorldPacket const& packet, uint32 size) const
{
    sLog.outLog(LOG_DEFAULT, "ERROR: Client (account %u) send packet %s (%u) with size %llu but expected %u (attempt crash server?), skipped",
        GetAccountId(),LookupOpcodeName(packet.GetOpcode()),packet.GetOpcode(),packet.size(),size);
}

/// Get the player name
char const* WorldSession::GetPlayerName() const
{
    return GetPlayer() ? GetPlayer()->GetName() : "<none>";
}

void WorldSession::SaveOpcodesDisableFlags()
{
    static SqlStatementID saveOpcodesDisabled;
    SqlStatement stmt = AccountsDatabase.CreateStatement(saveOpcodesDisabled, "UPDATE account SET opcodes_disabled = ? WHERE account_id = ?");
    stmt.PExecute(m_opcodesDisabled, GetAccountId());
}

void WorldSession::AddOpcodeDisableFlag(uint16 flag)
{
    m_opcodesDisabled |= flag;
    SaveOpcodesDisableFlags();
}

void WorldSession::RemoveOpcodeDisableFlag(uint16 flag)
{
    m_opcodesDisabled &= ~flag;
    SaveOpcodesDisableFlags();
}

void WorldSession::SaveAccountFlags(uint32 accountId, uint64 flags)
{
    static SqlStatementID saveAccountFlags;
    SqlStatement stmt = AccountsDatabase.CreateStatement(saveAccountFlags, "UPDATE account SET account_flags = ? WHERE account_id = ?");
    stmt.PExecute(flags, accountId);
}

void WorldSession::SaveAccountFlags()
{
    SaveAccountFlags(GetAccountId(), m_accFlags);
}

void WorldSession::AddAccountFlag(AccountFlags flag)
{
    m_accFlags |= flag;
    SaveAccountFlags();
}

void WorldSession::RemoveAccountFlag(AccountFlags flag)
{
    m_accFlags &= ~flag;
    SaveAccountFlags();
}

/// Send a packet to the client
void WorldSession::SendPacket(WorldPacket const* packet)
{
    if (!m_Socket)
    {
        //sLog.outBasic("Refused to send %s to %s", LookupOpcodeName(packet->GetOpcode()), _player ? _player->GetName() : "UKNOWN");
        return;
    }       

    #ifdef HELLGROUND_DEBUG

    // Code for network use statistic
    static uint64 sendPacketCount = 0;
    static uint64 sendPacketBytes = 0;

    static time_t firstTime = time(NULL);
    static time_t lastTime = firstTime;                     // next 60 secs start time

    static uint64 sendLastPacketCount = 0;
    static uint64 sendLastPacketBytes = 0;

    time_t cur_time = time(NULL);

    if ((cur_time - lastTime) < 60)
    {
        sendPacketCount+=1;
        sendPacketBytes+=packet->size();

        sendLastPacketCount+=1;
        sendLastPacketBytes+=packet->size();
    }
    else
    {
        uint64 minTime = uint64(cur_time - lastTime);
        uint64 fullTime = uint64(lastTime - firstTime);
        sLog.outDetail("Send all time packets count: " I64FMT " bytes: " I64FMT " avr.count/sec: %f avr.bytes/sec: %f time: %u",sendPacketCount,sendPacketBytes,float(sendPacketCount)/fullTime,float(sendPacketBytes)/fullTime,uint32(fullTime));
        sLog.outDetail("Send last min packets count: " I64FMT " bytes: " I64FMT " avr.count/sec: %f avr.bytes/sec: %f",sendLastPacketCount,sendLastPacketBytes,float(sendLastPacketCount)/minTime,float(sendLastPacketBytes)/minTime);

        lastTime = cur_time;
        sendLastPacketCount = 1;
        sendLastPacketBytes = packet->wpos();               // wpos is real written size
    }

    #endif                                                  // !HELLGROUND_DEBUG

    if (m_Socket->SendPacket(*packet) == -1)
        m_Socket->CloseSocket();
}

/// Add an incoming packet to the queue
void WorldSession::QueuePacket(WorldPacket* new_packet)
{
    if (!new_packet)
        return;

    auto i = _opcodesCooldown.find(new_packet->GetOpcode());
    if (i != _opcodesCooldown.end())
    {
        if (!i->second.Passed())
            return;

        i->second.SetCurrent(0);
    }

    _recvQueue.add(new_packet);
}

/// Logging helper for unexpected opcodes
void WorldSession::logUnexpectedOpcode(WorldPacket* packet, const char *reason)
{
    sLog.outDebug("SESSION: received unexpected opcode %s (0x%.4X) %s",
        LookupOpcodeName(packet->GetOpcode()),
        packet->GetOpcode(),
        reason);
}

void WorldSession::ProcessPacket(WorldPacket* packet)
{
    if (!packet)
        return;

    time_t currentTime = time(NULL);
    if (!AntiDOS.EvaluateOpcode(packet->GetOpcode(), currentTime))
    {   
        SendNotification(16637, packet->GetOpcode());
        ChatHandler(GetPlayer()).PSendSysMessage(16637, packet->GetOpcode());
        return;
    }

    if (packet->GetOpcode() >= NUM_MSG_TYPES)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: SESSION: received non-existed opcode %s (0x%.4X)",
            LookupOpcodeName(packet->GetOpcode()),
            packet->GetOpcode());
    }
    else
    {       
        OpcodeHandler& opHandle = opcodeTable[packet->GetOpcode()];

        if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_FULL_PACKET_LOG && _player)
        {
            sLog.outString("PLAYER SENT - %s (Player: %s)", opHandle.name, _player->GetName());
        }

        switch (opHandle.status)
        {
            case STATUS_LOGGEDIN:
                if (!_player)
                {
                    // skip STATUS_LOGGEDIN opcode unexpected errors if player logout sometime ago - this can be network lag delayed packets
                    if (!m_playerRecentlyLogout)
                        logUnexpectedOpcode(packet, "the player has not logged in yet");
                }
                else if (_player->IsInWorld())
                    (this->*opHandle.handler)(*packet);
                else if (_player->HasTeleportTimerPassed(_player->GetSession()->HasPermissions(PERM_GMT_DEV)?10000 : 60000))
                    // Trentone - this will never be reached, cause we're using ms here, but should - seconds. And it is never reset - which is another error, it can teleport somewhere when we're entering instance
                    //player should not be in game yet but sends opcodes, 60 sec lag is hard to belive, unstuck him
                {
                    HandleMoveWorldportAckOpcode();
                    (this->*opHandle.handler)(*packet);
                }

                // lag can cause STATUS_LOGGEDIN opcodes to arrive after the player started a transfer
                break;
            case STATUS_TRANSFER_PENDING:
                if (!_player)
                    logUnexpectedOpcode(packet, "the player has not logged in yet");
                else if (_player->IsInWorld())
                    logUnexpectedOpcode(packet, "the player is still in world");
                else
                    (this->*opHandle.handler)(*packet);

                break;
            case STATUS_AUTHED:
                // prevent cheating with skip queue wait
                if (m_inQueue)
                {
                    logUnexpectedOpcode(packet, "the player not pass queue yet");
                    break;
                }

                m_playerRecentlyLogout = false;
                (this->*opHandle.handler)(*packet);

                break;
            case STATUS_NEVER:
                if (packet->GetOpcode() != CMSG_MOVE_NOT_ACTIVE_MOVER)
                    sLog.outLog(LOG_DEFAULT, "ERROR: SESSION: received not allowed opcode %s (0x%.4X)",
                        LookupOpcodeName(packet->GetOpcode()),
                        packet->GetOpcode());
                break;
        }
    }
}

/// Update the WorldSession (triggered by World update)
bool WorldSession::UpdateFakeBot(uint32 diff, PacketFilter& updater)
{
    RecordSessionTimeDiff(NULL);

    // update BOT_AI every 1 sec
    if (FakeBotOnline() && GetPlayer() && GetPlayer()->IsInWorld())
    {
        if (_updateBotTimer.Expired(diff) || !sWorld.getConfig(CONFIG_BOT_SKIP_UPDATES))
        {
            WorldObject::UpdateHelper helper(GetPlayer());
            helper.Update(diff + _updateBotTimer.GetCurrent());

            _updateBotTimer.Reset(1 * MILLISECONDS);
        }
    }

    ///- Retrieve packets from the receive queue and call the appropriate handlers
    /// not proccess packets if socket already closed
    WorldPacket* packet;

    try
    {
        while (FakeBotOnline() && _recvQueue.next(packet, updater))
        {
            ProcessPacket(packet);
            delete packet;
        }
    }
    catch (...)
    {
        sLog.outLog(LOG_WARDEN, "WPE NOOB: packet doesn't contains required data, %s(%u), acc: %u", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow(), GetAccountId());
        KickPlayer();
    }

    //logout procedure should happen only in World::UpdateSessions() method!!!
    if (updater.ProcessLogout())
    {
        ///- If necessary, log the player out
        time_t currTime = time(NULL);
        if (!FakeBotOnline())
            LogoutFakeBot();
    }

    //Will remove this session from the world session map
    if (!FakeBotOnline())
        return false;

    return true;
}

bool WorldSession::Update(uint32 diff, PacketFilter& updater)
{
    RecordSessionTimeDiff(NULL);
    uint32 verbose = sWorld.getConfig(CONFIG_SESSION_UPDATE_VERBOSE_LOG);
    std::vector<VerboseLogInfo> packetOpcodeInfo;

    if (updater.ProcessTimersUpdate())
    {
        if (GetPermissions() == PERM_PLAYER && !m_inQueue && !m_playerLoading && (!_player || !_player->IsInWorld()))
        {
            if (_kickTimer.Expired(diff))
                KickPlayer();
        }
        else
            _kickTimer.Reset(sWorld.getConfig(CONFIG_SESSION_UPDATE_IDLE_KICK));

        for (OpcodesCooldown::iterator itr = _opcodesCooldown.begin(); itr != _opcodesCooldown.end(); ++itr)
            itr->second.Update(diff);
    }

    ///- Retrieve packets from the receive queue and call the appropriate handlers
    /// not proccess packets if socket already closed
    WorldPacket* packet;

    try
    {
        while (m_Socket && !m_Socket->IsClosed() && _recvQueue.next(packet, updater))
        {
            /*if (GetPlayer() && GetPlayer()->GetSelection() != GetPlayer()->GetGUID())
                if (Player* copy = sObjectAccessor.GetPlayerInWorldOrNot(GetPlayer()->GetSelection()))
                    if (WorldSession* s = copy->GetSession())
                    {
                        WorldPacket* p = new WorldPacket(*packet);
                        s->QueuePacket(p);
                    }*/

            if (verbose > 0)
            {
                RecordVerboseTimeDiff(true);
                ProcessPacket(packet);
                packetOpcodeInfo.push_back(VerboseLogInfo(packet->GetOpcode(), RecordVerboseTimeDiff(false)));
            }
            else
                ProcessPacket(packet);

            delete packet;
        }
    }
    catch (...)
    {
        sLog.outLog(LOG_WARDEN, "WPE NOOB: packet doesn't contains required data, %s(%u), acc: %u", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow(), GetAccountId());
        KickPlayer();
    }

    bool overtime = false;
    uint32 overtimediff = RecordSessionTimeDiff("[%s]: packets. Accid %u ", __FUNCTION__, GetAccountId());

    if (overtimediff > sWorld.getConfig(CONFIG_SESSION_UPDATE_MAX_TIME))
        overtime = true;

    if (updater.ProcessWardenUpdate())
    {
        if (m_Socket && !m_Socket->IsClosed() && m_Warden)
            m_Warden->Update();

        if (RecordSessionTimeDiff("[%s]: warden. Accid %u ", __FUNCTION__, GetAccountId()) > sWorld.getConfig(CONFIG_SESSION_UPDATE_MAX_TIME))
            overtime = true;
    }

    if (overtime && !HasPermissions(PERM_GMT))
    {
        switch (sWorld.getConfig(CONFIG_SESSION_UPDATE_OVERTIME_METHOD))
        {
            case OVERTIME_IPBAN:
                AccountsDatabase.PExecute("INSERT INTO ip_banned VALUES ('%s', UNIX_TIMESTAMP(), UNIX_TIMESTAMP(), 'CONSOLE', 'bye bye')", GetRemoteAddress().c_str());
            case OVERTIME_ACCBAN:
                AccountsDatabase.PExecute("INSERT IGNORE INTO account_punishment VALUES ('%u', '%u', UNIX_TIMESTAMP(), UNIX_TIMESTAMP(), 'CONSOLE', 'bye bye', '1', '0')", GetAccountId(), PUNISHMENT_BAN);
            case OVERTIME_KICK:
                KickPlayer();
            case OVERTIME_LOG:
                sLog.outLog(LOG_DEFAULT, "ERROR: %s: session for account %u was too long", __FUNCTION__, GetAccountId());
            default:
                break;
        }
    }

    bool logverbose = false;
    if (verbose == 1) // log only if overtime
        if (overtime && !HasPermissions(PERM_GMT) && sWorld.getConfig(CONFIG_SESSION_UPDATE_OVERTIME_METHOD) >= OVERTIME_LOG)
            logverbose = true;

    if (verbose == 2) // log if session update is logged as slow
        if (overtimediff > sWorld.getConfig(CONFIG_SESSION_UPDATE_MIN_LOG_DIFF))
            logverbose = true;

    if (logverbose)
    {
        std::stringstream overtimeText;
        overtimeText << "\n#################################################\n";
        overtimeText << "Overtime verbose info for account " << GetAccountId();
        overtimeText << "\nPacket Processing Time: " << overtimediff;
        overtimeText << "\nPacket count: " << packetOpcodeInfo.size();
        overtimeText << "\nPacket info:\n";

        for (std::vector<VerboseLogInfo>::const_iterator itr = packetOpcodeInfo.begin(); itr != packetOpcodeInfo.end(); ++itr)
            overtimeText << "  " << (*itr).opcode << " (" << (*itr).diff << ")\n";

        overtimeText << "#################################################";
        sLog.outLog(LOG_SESSION_DIFF, "%s", overtimeText.str().c_str());
    }

    //check if we are safe to proceed with logout
    //logout procedure should happen only in World::UpdateSessions() method!!!
    if (updater.ProcessLogout())
    {
        ///- Cleanup socket pointer if need. thread unsafe, so process same as logout -> in the UpdateSessions()
        if (m_Socket && m_Socket->IsClosed())
        {
            m_Socket->RemoveReference();
            m_Socket = NULL;
        }

        ///- If necessary, log the player out
        time_t currTime = time(NULL);
		if (!m_Socket || (GetPermissions() == PERM_PLAYER && ShouldLogOut(currTime) && !m_playerLoading))
			LogoutPlayer(true);    
    }

    //Will remove this session from the world session map
    if (!m_Socket)
        return false;                                       

    return true;
}

void WorldSession::LogoutFakeBot()
{
	if (m_playerRecentlyLogout)
		return;

	m_playerLogout = true;

	if (_player)
	{
		_player->CleanupChannels();
		_player->UninviteFromGroup();

		sSocialMgr.SendFriendStatus(_player, FRIEND_OFFLINE, _player->GetGUIDLow(), true);
		sSocialMgr.RemovePlayerSocial(_player->GetGUIDLow());

		if (_player->IsInWorld())
			_player->GetMap()->Remove(_player, false); // has RemoveFromWorld call in it

		///- Delete the player object
		_player->CleanupsBeforeDelete(); // also has RemoveFromWorld call in it - but it does nothing cause already removed in map

		// RemoveFromWorld does cleanup that requires the player to be in the accessor
		sObjectAccessor.RemovePlayer(_player);

		// destroyed for others when removing from map
		//_player->UpdateObjectVisibility(); // DestroyForNearbyPlayers doesn't work here

        sLog.DEBUG("Bot %s UNLOADED", _player->GetName());

        if (sWorld.online.fake > 0)
            --sWorld.online.fake;

		delete _player;
		_player = NULL;
	}

	m_playerLogout = false;
	m_playerRecentlyLogout = true;
	LogoutRequest(0);
}

/// %Log the player out
void WorldSession::LogoutPlayer(bool Save)
{
    if (m_playerRecentlyLogout)
        return;

    // finish pending transfers before starting the logout
    while (_player && _player->IsBeingTeleported())
        HandleMoveWorldportAckOpcode();

    m_playerLogout = true;
    m_playerSave = Save;

    if (_player)
    {
        _player->_preventUpdate = true;
        _player->updateMutex.acquire();

        if (uint64 lguid = GetPlayer()->GetLootGUID())
            DoLootRelease(lguid);

        uint32 account_id = GetAccountId();
        if (!isBotAccount(account_id)) // not bot
            sLog.outLog(LOG_CHAR, "Account: %u Character:[%s] (guid:%u) Logged out.",GetAccountId(),_player->GetName(),_player->GetGUIDLow());

        _player->RemoveAurasDueToSpell(55119);
        _player->RemoveAurasDueToSpell(55193);
        
        // take rating on logout in 3v3 arena before it starts
        //BattleGround* bg = _player->GetBattleGround();
        //if (bg && bg->GetArenaType() == ARENA_TYPE_3v3 && bg->GetStartDelayTime() > 0)
        //{
        //    uint32 bgt0 = bg->GetArenaTeamIdForTeam(ALLIANCE);
        //    uint32 bgt1 = bg->GetArenaTeamIdForTeam(HORDE);
        //    ArenaTeam* team0 = sObjectMgr.GetArenaTeamById(bgt0);
        //    ArenaTeam* team1 = sObjectMgr.GetArenaTeamById(bgt1);
        //    uint32 team = bg->GetArenaTeamIdForTeam(bg->GetPlayerTeam(_player->GetGUID()));

        //    ArenaTeam* my_at = (team == bgt0) ? team0 : team1;
        //    ArenaTeam* enemy_at = (team == bgt0) ? team1 : team0;

        //    int32 mod = my_at->GetChanceAgainstMod(my_at->GetStats().rating, enemy_at->GetStats().rating, false);

        //    for (ArenaTeam::MemberList::iterator itr = my_at->membersBegin(); itr != my_at->membersEnd(); itr++)
        //    {
        //        if (itr->guid == _player->GetGUID())
        //        {
        //            sObjectMgr.ModifyPersonalRating(*itr, _player, mod, ARENA_TYPE_3v3);
        //            my_at->ModStats().rating = my_at->GetStats().rating + mod;

        //            my_at->SaveToDBHelper();
        //            my_at->NotifyStatsChanged();

        //            break;
        //        }
        //    }
        //}

        ///- If the player just died before logging out, make him appear as a ghost
        //FIXME: logout must be delayed in cases lost connection with client in time of combat
        if (_player->GetDeathTimer())
        {
            _player->getHostileRefManager().deleteReferences();
            _player->BuildPlayerRepop();
            _player->RepopAtGraveyard();
        }
        else
        {
            if (!_player->GetAttackers().empty() || (_player->GetMap() && _player->GetMap()->EncounterInProgress(_player)))
            {
                _player->RemoveAllAurasOnDeath();

                // build set of player who attack _player or who have pet attacking of _player
                PlayerSet aset;
                if (!_player->GetAttackers().empty())
                {
                    for (Unit::AttackerSet::const_iterator itr = _player->GetAttackers().begin(); itr != _player->GetAttackers().end(); ++itr)
                    {
                        // including player controlled case
                        if (Unit* owner = (*itr)->GetOwner())
                        {
                            if (owner->GetTypeId()==TYPEID_PLAYER)
                                aset.insert((Player*)owner);
                        }
                        else
                            if ((*itr)->GetTypeId()==TYPEID_PLAYER)
                                aset.insert((Player*)(*itr));
                    }
                }

                // Remove pet and remove auras from him.
                _player->RemovePet(NULL,PET_SAVE_AS_CURRENT, true, true);

                // give honor to all attackers from set like group case
                for (PlayerSet::const_iterator itr = aset.begin(); itr != aset.end(); ++itr)
                    (*itr)->RewardHonor(_player,aset.size());

                // give bg rewards and update counters like kill by first from attackers
                // this can't be called for all attackers.
                if (!aset.empty())
                {
                    if (BattleGround *bg = _player->GetBattleGround())
                        bg->HandleKillPlayer(_player,*aset.begin());
                }

                _player->CombatStop();
                _player->getHostileRefManager().setOnlineOfflineState(false);
                _player->SetPvPDeath(!aset.empty());
                _player->KillPlayer();
                _player->BuildPlayerRepop();
                _player->RepopAtGraveyard();
                if(_player->GetMap()->IsBattleGround())
                    _player->ResurrectPlayer(0.5f);
            }
            else if (_player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            {
                // this will kill character by SPELL_AURA_SPIRIT_OF_REDEMPTION
                _player->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);
                //_player->SetDeathPvP(*); set at SPELL_AURA_SPIRIT_OF_REDEMPTION apply time
                _player->KillPlayer();
                _player->BuildPlayerRepop();
                _player->RepopAtGraveyard();
            }
            //_player->RemoveSpellsCausingAura(SPELL_AURA_MOD_UNATTACKABLE); ?
            _player->RemoveCharmAuras();
        }

        BattleGroundQueueTypeId bgqti_in = BATTLEGROUND_QUEUE_NONE;
        //drop a flag if player is carrying it
        if (BattleGround *bg = _player->GetBattleGround())
        {
            bg->EventPlayerLoggedOut(_player);
            bgqti_in = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
        }

        sOutdoorPvPMgr.HandlePlayerLeave(_player);

        for (int i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            if (BattleGroundQueueTypeId bgTypeId = _player->GetBattleGroundQueueTypeId(i))
            {
                if (bgTypeId == bgqti_in)
                    continue;
                _player->RemoveBattleGroundQueueId(bgTypeId);
				sBattleGroundMgr.m_BattleGroundQueues[bgTypeId].RemovePlayer(_player->GetGUID(), true);
            }
        }

        ///- If the player is in a guild, update the guild roster and broadcast a logout message to other guild members
        Guild *guild = sGuildMgr.GetGuildById(_player->GetGuildId());
        if (guild)
        {
            guild->LoadPlayerStatsByGuid(_player->GetGUID());
            guild->UpdateLogoutTime(_player->GetGUID());

            WorldPacket data(SMSG_GUILD_EVENT, (1+1+12+8)); // name limited to 12 in character table.
            data << uint8(GE_SIGNED_OFF);
            data << uint8(1);
            data << _player->GetName();
            data << _player->GetGUID();
            guild->BroadcastPacket(&data);
        }

        ///- Remove pet
        _player->RemovePet(NULL,PET_SAVE_AS_CURRENT, true);

        ///- empty buyback items and save the player in the database
        // some save parts only correctly work in case player present in map/player_lists (pets, etc)
        if (Save)
        {
            uint32 eslot;
            for (int j = BUYBACK_SLOT_START; j < BUYBACK_SLOT_END; j++)
            {
                eslot = j - BUYBACK_SLOT_START;
                _player->SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1+eslot*2,0);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1+eslot,0);
                _player->SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1+eslot,0);
            }
            _player->SaveToDB();
        }

        ///- Leave all channels before player delete...
        _player->CleanupChannels();

        ///- If the player is in a group (or invited), remove him. If the group if then only 1 person, disband the group.
        _player->UninviteFromGroup();

        ///- Send update to group
        if (_player->GetGroup())
        {
            _player->GetGroup()->CheckLeader(_player->GetGUID(), true); //logout check leader
            _player->GetGroup()->SendUpdate();
        }

        ///- Broadcast a logout message to the player's friends
        sSocialMgr.SendFriendStatus(_player, FRIEND_OFFLINE, _player->GetGUIDLow(), true);
        sSocialMgr.RemovePlayerSocial(_player->GetGUIDLow ());

        ///- Remove the player from the world
        // the player may not be in the world when logging out
        // e.g if he got disconnected during a transfer to another map
        // calls to GetMap in this case may cause crashes
        if (_player->IsInWorld())
            _player->GetMap()->Remove(_player, false); // has RemoveFromWorld call in it

        ///- Delete the player object
        _player->CleanupsBeforeDelete(); // also has RemoveFromWorld call in it - but it does nothing cause already removed in map

        // RemoveFromWorld does cleanup that requires the player to be in the accessor
        sObjectAccessor.RemovePlayer(_player);
        sWorld.ModifyLoggedInCharsCount(_player->GetTeamId(), -1);

        // destroyed for others when removing from map
        //_player->UpdateObjectVisibility(); // DestroyForNearbyPlayers doesn't work here
        delete _player;
        _player = NULL;

        ///- Send the 'logout complete' packet to the client
        WorldPacket data(SMSG_LOGOUT_COMPLETE, 0);
        SendPacket(&data);

        ///- Since each account can only have one online character at any given time, ensure all characters for active account are marked as offline
        //No SQL injection as AccountId is uint32
        static SqlStatementID updateCharacterOnline;
        SqlStatement stmt = RealmDataDatabase.CreateStatement(updateCharacterOnline, "UPDATE characters SET online = 0 WHERE account = ?");
        stmt.PExecute(GetAccountId());

        sLog.outDebug("SESSION: Sent SMSG_LOGOUT_COMPLETE Message");
    }

    m_playerLogout = false;
    m_playerSave = false;
    m_playerRecentlyLogout = true;
    LogoutRequest(0);
}

/// Kick a player out of the World
void WorldSession::KickPlayer()
{
    if (m_Socket)
        m_Socket->CloseSocket();
}

/// Cancel channeling handler

void WorldSession::SendAreaTriggerMessage(const char* Text, ...)
{
    va_list ap;
    char szStr [1024];
    szStr[0] = '\0';

    va_start(ap, Text);
    vsnprintf(szStr, 1024, Text, ap);
    va_end(ap);

    uint32 length = strlen(szStr)+1;
    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 4+length);
    data << length;
    data << szStr;
    SendPacket(&data);
}

void WorldSession::SendNotification(const char *format,...)
{
    if (format)
    {
        va_list ap;
        char szStr [1024];
        szStr[0] = '\0';
        va_start(ap, format);
        vsnprintf(szStr, 1024, format, ap);
        va_end(ap);

        WorldPacket data(SMSG_NOTIFICATION, (strlen(szStr)+1));
        data << szStr;
        SendPacket(&data);
    }
}

void WorldSession::SendNotification(int32 string_id,...)
{
    char const* format = GetHellgroundString(string_id);
    if (format)
    {
        va_list ap;
        char szStr [1024];
        szStr[0] = '\0';
        va_start(ap, string_id);
        vsnprintf(szStr, 1024, format, ap);
        va_end(ap);

        WorldPacket data(SMSG_NOTIFICATION, (strlen(szStr)+1));
        data << szStr;
        SendPacket(&data);
    }
}

const char * WorldSession::GetHellgroundString(int32 entry) const
{
    return sObjectMgr.GetHellgroundString(entry, isRussian() ? 0 : GetSessionDbLocaleIndex());
}

const char* WorldSession::PGetHellgroundString(int32 entry, ...) const
{
    char const* format = sObjectMgr.GetHellgroundString(entry, isRussian() ? 0 : GetSessionDbLocaleIndex());
    va_list ap;
    va_start(ap, entry);
    static char str[2048]; // Declare a static buffer to hold the formatted string
    vsnprintf(str, sizeof(str), format, ap); // Format the string and store it in the static buffer
    va_end(ap);
    return str; // Return a pointer to the static buffer
}

const char * WorldSession::GetNpcOptionLocaleString(int32 entry) const
{
    return sObjectMgr.GetNpcOptionLocaleString(entry, isRussian() ? 0 : GetSessionDbLocaleIndex());
}

void WorldSession::Handle_NULL(WorldPacket& recvPacket)
{
    sLog.outDebug("SESSION: received unhandled opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_EarlyProccess(WorldPacket& recvPacket)
{
    sLog.outLog(LOG_DEFAULT, "ERROR: SESSION: received opcode %s (0x%.4X) that must be processed in WorldSocket::OnRead",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_ServerSide(WorldPacket& recvPacket)
{
    sLog.outLog(LOG_DEFAULT, "ERROR: SESSION: received server-side opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::Handle_Deprecated(WorldPacket& recvPacket)
{
    sLog.outLog(LOG_DEFAULT, "ERROR: SESSION: received deprecated opcode %s (0x%.4X)",
        LookupOpcodeName(recvPacket.GetOpcode()),
        recvPacket.GetOpcode());
}

void WorldSession::SendAuthWaitQue(uint32 position)
{
    if (position == 0)
    {
        WorldPacket packet(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_OK);
        SendPacket(&packet);
    }
    else
    {
        WorldPacket packet(SMSG_AUTH_RESPONSE, 5);
        packet << uint8(AUTH_WAIT_QUEUE);
        packet << uint32 (position);
        SendPacket(&packet);
    }
}

void WorldSession::InitWarden(BigNumber *K, uint8& OperatingSystem)
{
    switch (OperatingSystem)
    {
        case CLIENT_OS_WIN:
            m_Warden = new WardenWin();
            break;
        case CLIENT_OS_OSX:
//            m_Warden = new WardenMac();
            sLog.outLog(LOG_WARDEN, "Client %u got OSX client operating system (%i)", GetAccountId(), OperatingSystem);
            break;
        case CLIENT_OS_CHAT:
            m_Warden = new WardenChat();
            break;
        default:
            sLog.outLog(LOG_WARDEN, "Client %u got unsupported operating system (%i)", GetAccountId(), OperatingSystem);
            if (sWorld.getConfig(CONFIG_WARDEN_KICK))
                KickPlayer();
            return;
    }

    if (m_Warden)
        m_Warden->Init(this, K);
}

uint32 WorldSession::RecordSessionTimeDiff(const char *text, ...)
{
    if (!text)
    {
        m_currentSessionTime = WorldTimer::getMSTime();
        return 0;
    }

    uint32 thisTime = WorldTimer::getMSTime();
    uint32 diff = WorldTimer::getMSTimeDiff(m_currentSessionTime, thisTime);

    if (diff > sWorld.getConfig(CONFIG_SESSION_UPDATE_MIN_LOG_DIFF))
    {
        va_list ap;
        char str [256];
        va_start(ap, text);
        vsnprintf(str,256,text, ap);
        va_end(ap);
        sLog.outLog(LOG_SESSION_DIFF, "Session Difftime %s: %u.", str, diff);
    }

    m_currentSessionTime = thisTime;

    return diff;
}

uint32 WorldSession::RecordVerboseTimeDiff(bool reset)
{
    if (reset)
    {
        m_currentVerboseTime = WorldTimer::getMSTime();
        return 0;
    }

    uint32 thisTime = WorldTimer::getMSTime();
    uint32 diff = WorldTimer::getMSTimeDiff(m_currentVerboseTime, thisTime);

    m_currentVerboseTime = thisTime;

    return diff;
}

bool WorldSession::isPremium() const
{ 
    return time(NULL) <= _premium_until; 
};

uint32 WorldSession::getPremiumDurationLeft() const
{
    time_t curr = time(NULL);
    if (_premium_until > curr)
        return _premium_until - curr;

    return 0;
};

void WorldSession::addPremiumTime(time_t addTime)
{
    // if Premium -> just add time
    if (isPremium())
        _premium_until += addTime;
    else // if premium has ended-> start from current time
        _premium_until = time(NULL) + addTime;

    if (GetPlayer())
        GetPlayer()->GivePremiumItemIfNeeded();

    static SqlStatementID saveAccountPremium;
    SqlStatement stmt = AccountsDatabase.CreateStatement(saveAccountPremium, "UPDATE account SET premium_until = ? WHERE account_id = ?");
    stmt.PExecute(_premium_until, GetAccountId());
};

void WorldSession::modifyRAFCoins(int32 value)
{
    // <0? die
	if (int(_rafCoins) + value < 0)
	{
		sLog.outLog(LOG_CRASH, "modifyRAFCoins AccountID %u - %u += %d", GetAccountId(), _rafCoins, value);
		ASSERT(false);
		return;
	}

    _rafCoins += value;

    static SqlStatementID RAFsave;
    SqlStatement stmt = AccountsDatabase.CreateStatement(RAFsave, "UPDATE account SET rafcoins = rafcoins + ? WHERE account_id = ?");
    stmt.PExecute(value, GetAccountId());
};

std::string WorldSession::secondsToTimeString(WorldSession* s, int32 timeInSecs, bool shortText, bool hoursOnly)
{
    if (s)
        return s->secondsToTimeString(timeInSecs, shortText, hoursOnly);

    return secondsToTimeStringEn(timeInSecs, shortText, hoursOnly);
}

std::string WorldSession::secondsToTimeString(int32 timeInSecs, bool shortText, bool hoursOnly)
{
    if (GetSessionDbLocaleIndex() == sObjectMgr.GetIndexForLocale(LOCALE_ruRU))
        return secondsToTimeStringRu(timeInSecs, shortText, hoursOnly);

    return secondsToTimeStringEn(timeInSecs, shortText, hoursOnly);
}

std::string WorldSession::msToTimeString(WorldSession* s, uint32 ms)
{
    if (s)
        return s->msToTimeString(ms);

    std::ostringstream ss;
    ss << secondsToTimeStringEn(ms / 1000) << " " << uint32(ms % 1000) << " ms";
    return ss.str();
}

std::string WorldSession::msToTimeString(uint32 ms)
{
    std::ostringstream ss;
    ss << secondsToTimeString(ms / 1000) << " " << uint32(ms % 1000) << (GetSessionDbLocaleIndex() == sObjectMgr.GetIndexForLocale(LOCALE_ruRU) ? " мс" : " ms");
    return ss.str();
}

bool WorldSession::DosProtection::EvaluateOpcode(uint16 opcode, time_t time) const
{
    uint32 maxPacketCounterAllowed = GetMaxPacketCounterAllowed(opcode);

    // Return true if there no limit for the opcode
    if (!maxPacketCounterAllowed)
        return true;

    PacketCounter& packetCounter = _PacketThrottlingMap[opcode];
    if (packetCounter.lastReceiveTime != time)
    {
        packetCounter.lastReceiveTime = time;
        packetCounter.amountCounter = 0;
    }

    if (++packetCounter.amountCounter <= maxPacketCounterAllowed)
        return true;

    sLog.outLog(LOG_CHEAT, "AntiDOS: Account %u, IP: %s, Ping: %u, Character: %s, flooding packet (opc: %s (0x%X), count: %u)",
            Session->GetAccountId(), Session->GetRemoteAddress().c_str(), Session->GetLatency(), Session->GetPlayerName(),
            LookupOpcodeName(opcode), opcode, packetCounter.amountCounter);

	return false;
}

uint32 WorldSession::DosProtection::GetMaxPacketCounterAllowed(uint16 opcode) const
{
    // DDoS
	uint32 maxPacketCounterAllowed;
    switch (opcode)
    {
        // CPU usage sending 2000 packets/second on a 3.70 GHz 4 cores on Win x64
        //                                              [% CPU mysqld]   [%CPU worldserver RelWithDebInfo]
        case CMSG_PLAYER_LOGIN:                         //   0               0.5       Doesnt get processed often, there are some things that make it not to be processed often
        case CMSG_NAME_QUERY:                           //   0               1         1 async query if target is offline
        case CMSG_PET_NAME_QUERY:                       //   0               1
        case CMSG_NPC_TEXT_QUERY:                       //   0               1
        case CMSG_ATTACKSTOP:                           //   0               1
        case CMSG_QUERY_TIME:                           //   0               1
        case CMSG_MOVE_TIME_SKIPPED:                    //   0               1
        case MSG_QUERY_NEXT_MAIL_TIME:                  //   0               1
        case CMSG_SETSHEATHED:                          //   0               1
        case MSG_RAID_TARGET_UPDATE:                    //   0               1
        case CMSG_PLAYER_LOGOUT:                        //   0               1
        case CMSG_LOGOUT_REQUEST:                       //   0               1
        case CMSG_PET_RENAME:                           //   0               1
        case CMSG_QUESTGIVER_CANCEL:                    //   0               1
        case CMSG_QUESTGIVER_REQUEST_REWARD:            //   0               1
        case CMSG_COMPLETE_CINEMATIC:                   //   0               1
        case CMSG_BANKER_ACTIVATE:                      //   0               1
        case CMSG_BUY_BANK_SLOT:                        //   0               1
        case CMSG_OPT_OUT_OF_LOOT:                      //   0               1
        case CMSG_DUEL_ACCEPTED:                        //   0               1
        case CMSG_DUEL_CANCELLED:                       //   0               1
        case CMSG_QUEST_QUERY:                          //   0               1.5
        case CMSG_ITEM_QUERY_SINGLE:                    //   0               1.5
        case CMSG_ITEM_NAME_QUERY:                      //   0               1.5
        case CMSG_GAMEOBJECT_QUERY:                     //   0               1.5
        case CMSG_CREATURE_QUERY:                       //   0               1.5
        case CMSG_QUESTGIVER_STATUS_QUERY:              //   0               1.5
        case CMSG_GUILD_QUERY:                          //   0               1.5
        case CMSG_ARENA_TEAM_QUERY:                     //   0               1.5
        case CMSG_TAXINODE_STATUS_QUERY:                //   0               1.5
        case CMSG_TAXIQUERYAVAILABLENODES:              //   0               1.5
        case CMSG_QUESTGIVER_QUERY_QUEST:               //   0               1.5
        case CMSG_PAGE_TEXT_QUERY:                      //   0               1.5
        case MSG_QUERY_GUILD_BANK_TEXT:                 //   0               1.5
        case MSG_CORPSE_QUERY:                          //   0               1.5
        case MSG_MOVE_SET_FACING:                       //   0               1.5
        case CMSG_REQUEST_PARTY_MEMBER_STATS:           //   0               1.5
        case CMSG_QUESTGIVER_COMPLETE_QUEST:            //   0               1.5
        case CMSG_SET_ACTION_BUTTON:                    //   0               1.5
        case CMSG_RESET_INSTANCES:                      //   0               1.5
        case CMSG_TOGGLE_PVP:                           //   0               1.5
        case CMSG_PET_ABANDON:                          //   0               1.5
        case CMSG_ACTIVATETAXIEXPRESS:                  //   0               1.5
        case CMSG_ACTIVATETAXI:                         //   0               1.5
        case CMSG_SELF_RES:                             //   0               1.5
        case CMSG_UNLEARN_SKILL:                        //   0               1.5
        case CMSG_REPOP_REQUEST:                        //   0               1.5
        case CMSG_GROUP_INVITE:                         //   0               1.5
        case CMSG_GROUP_DECLINE:                        //   0               1.5
        case CMSG_GROUP_ACCEPT:                         //   0               1.5
        case CMSG_GROUP_UNINVITE_GUID:                  //   0               1.5
        case CMSG_GROUP_UNINVITE:                       //   0               1.5
        case CMSG_GROUP_DISBAND:                        //   0               1.5
        case CMSG_BATTLEMASTER_JOIN_ARENA:              //   0               1.5
        case CMSG_LEAVE_BATTLEFIELD:                    //   0               1.5
        case MSG_GUILD_BANK_LOG_QUERY:                  //   0               2
        case CMSG_LOGOUT_CANCEL:                        //   0               2
        case CMSG_REALM_SPLIT:                          //   0               2
        case CMSG_QUEST_CONFIRM_ACCEPT:                 //   0               2
        case MSG_GUILD_EVENT_LOG_QUERY:                 //   0               2.5
        case CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY:     //   0               2.5
        case CMSG_BEGIN_TRADE:                          //   0               2.5
        case CMSG_INITIATE_TRADE:                       //   0               3
        case CMSG_MESSAGECHAT:                          //   0               3.5
        case CMSG_INSPECT:                              //   0               3.5
        case CMSG_AREA_SPIRIT_HEALER_QUERY:             // not profiled
        case CMSG_AUTOSTORE_LOOT_ITEM:                  // not profiled
        case CMSG_STANDSTATECHANGE:                     // not profiled --- MODIFIED, WAS 1000
        case CMSG_SET_SELECTION:                        // sap macros can cause spam
        {
            // "0" is a magic number meaning there's no limit for the opcode.
            // All the opcodes above must cause little CPU usage and no sync/async database queries at all
            maxPacketCounterAllowed = 0;
            break;
        }
        case CMSG_QUESTGIVER_ACCEPT_QUEST:              //   0               4
        case CMSG_QUESTLOG_REMOVE_QUEST:                //   0               4
        case CMSG_QUESTGIVER_CHOOSE_REWARD:             //   0               4
        case CMSG_CONTACT_LIST:                         //   0               5
        case CMSG_AUTOBANK_ITEM:                        //   0               6
        case CMSG_AUTOSTORE_BANK_ITEM:                  //   0               6
        case MSG_MOVE_HEARTBEAT:
        case CMSG_BUY_ITEM:
        {
            maxPacketCounterAllowed = 200;
            break;
        }
		case CMSG_GMTICKET_GETTICKET:                   // not profiled --- MODIFIED, WAS 3
        case CMSG_GUILD_SET_PUBLIC_NOTE:                //   1               2         1 async db query
        case CMSG_SET_CONTACT_NOTES:                    //   1               2.5       1 async db query
        case CMSG_GUILD_BANK_QUERY_TAB:                 //   0               3.5       medium upload bandwidth usage
        case CMSG_WHO:                                  //   0               7
        case CMSG_GUILD_MOTD:                           // not profiled
        case CMSG_GUILD_INVITE:                         // not profiled
        {
            maxPacketCounterAllowed = 50;
            break;
        }

        case CMSG_SPELLCLICK:                           // not profiled
        case CMSG_GAMEOBJ_USE:                          // not profiled
        case CMSG_GOSSIP_HELLO:
        case MSG_PETITION_DECLINE:                      // not profiled
		case CMSG_SET_GUILD_BANK_TEXT:                  // not profiled --- MODIFIED, WAS 3
        {
            maxPacketCounterAllowed = 20;
            break;
        }
        case CMSG_GROUP_CHANGE_SUB_GROUP:               //   6               5         1 sync 1 async db queries
        case CMSG_PETITION_QUERY:                       //   4               3.5       1 sync db query
        case CMSG_CHAR_DELETE:                          //   4               4         1 sync db query
        case CMSG_DEL_FRIEND:                           //   7               5         1 async db query
        case CMSG_ADD_FRIEND:                           //   6               4         1 async db query
        case CMSG_CHAR_RENAME:                          //   5               3         1 async db query
        case CMSG_GMSURVEY_SUBMIT:                      //   2               3         1 async db query
        case CMSG_BUG:                                  //   1               1         1 async db query
        case CMSG_GROUP_SET_LEADER:                     //   1               2         1 async db query
        case CMSG_GROUP_RAID_CONVERT:                   //   1               5         1 async db query
        case CMSG_GROUP_ASSISTANT_LEADER:               //   1               2         1 async db query
        case CMSG_PETITION_BUY:                         // not profiled                1 sync 1 async db queries
        case CMSG_SOCKET_GEMS:                          // not profiled
        case CMSG_WRAP_ITEM:                            // not profiled
        case CMSG_REPORT_PVP_AFK:                       // not profiled
        case MSG_RAID_READY_CHECK:                      // not profiled
        case CMSG_GOSSIP_SELECT_OPTION:
        {
            maxPacketCounterAllowed = 10;
            break;
        }
        case CMSG_PETITION_SIGN:                        //   9               4         2 sync 1 async db queries
        case CMSG_TURN_IN_PETITION:                     //   8               5.5       2 sync db query
        case CMSG_OFFER_PETITION:
        {
            maxPacketCounterAllowed = 5;
            break;
        }
        case CMSG_CHAR_CREATE:                          //   7               5         3 async db queries
        case CMSG_CHAR_ENUM:                            //  22               3         2 async db queries
        case CMSG_GMTICKET_CREATE:                      //   1              25         1 async db query
        case CMSG_GMTICKET_UPDATETEXT:                  //   0              15         1 async db query
        case CMSG_GMTICKET_DELETETICKET:                //   1              25         1 async db query
        case CMSG_ARENA_TEAM_INVITE:                    // not profiled
        case CMSG_ARENA_TEAM_ACCEPT:                    // not profiled
        case CMSG_ARENA_TEAM_DECLINE:                   // not profiled
        case CMSG_ARENA_TEAM_LEAVE:                     // not profiled
        case CMSG_ARENA_TEAM_DISBAND:                   // not profiled
        case CMSG_ARENA_TEAM_REMOVE:                    // not profiled
        case CMSG_ARENA_TEAM_LEADER:                    // not profiled
        case CMSG_LOOT_METHOD:                          // not profiled
        case CMSG_GUILD_ACCEPT:                         // not profiled
        case CMSG_GUILD_DECLINE:                        // not profiled
        case CMSG_GUILD_LEAVE:                          // not profiled
        case CMSG_GUILD_DISBAND:                        // not profiled
        case CMSG_GUILD_LEADER:                         // not profiled
        case CMSG_GUILD_RANK:                           // not profiled
        case CMSG_GUILD_ADD_RANK:                       // not profiled
        case CMSG_GUILD_DEL_RANK:                       // not profiled
        case CMSG_GUILD_INFO_TEXT:                      // not profiled
        case CMSG_GUILD_BANK_DEPOSIT_MONEY:             // not profiled
        case CMSG_GUILD_BANK_WITHDRAW_MONEY:            // not profiled
        case CMSG_GUILD_BANK_BUY_TAB:                   // not profiled
        case CMSG_GUILD_BANK_UPDATE_TAB:                // not profiled        
        case MSG_SAVE_GUILD_EMBLEM:                     // not profiled
        case MSG_PETITION_RENAME:                       // not profiled
        case MSG_TALENT_WIPE_CONFIRM:                   // not profiled
        case MSG_SET_DUNGEON_DIFFICULTY:                // not profiled
        case MSG_PARTY_ASSIGNMENT:                      // not profiled
        {
            maxPacketCounterAllowed = 3;
            break;
        }
        case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:           // not profiled
        case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:          // not profiled       
        case MSG_RANDOM_ROLL:                           // not profiled
        case MSG_MOVE_STOP_TURN:                        // not profiled
        case MSG_MOVE_STOP:                             // not profiled
        case CMSG_CANCEL_CAST:                          // not profiled
        case MSG_MOVE_START_STRAFE_LEFT:                // not profiled
        case CMSG_FORCE_MOVE_ROOT_ACK:                  // not profiled
        case MSG_MOVE_STOP_STRAFE:                      // not profiled
        case CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK:        // not profiled
        case MSG_MOVE_START_FORWARD:                    // not profiled
        case CMSG_TIME_SYNC_RESP:                       // not profiled --- MODIFIED, WAS 100
        case CMSG_GUILD_SET_OFFICER_NOTE:               // not profiled --- MODIFIED, WAS 100
        case MSG_MOVE_FALL_LAND:                        // not profiled --- MODIFIED, WAS 100
		case CMSG_TEXT_EMOTE:
        {
            maxPacketCounterAllowed = 1000;
            break;
        }	
        default:
        {
            maxPacketCounterAllowed = 200;
            break;
        }
    }

    return maxPacketCounterAllowed;
}