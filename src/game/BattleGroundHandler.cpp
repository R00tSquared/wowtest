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

#include "Common.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "World.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "Chat.h"
#include "BattleGroundMgr.h"
#include "BattleGroundWS.h"
#include "BattleGround.h"
#include "ArenaTeam.h"
#include "Language.h"

void WorldSession::HandleBattleGroundHelloOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    uint64 guid;
    recv_data >> guid;
    sLog.outDebug("WORLD: Recvd CMSG_BATTLEMASTER_HELLO Message from: %llu", guid);

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BATTLEMASTER);
    if (!unit)
        return;

    // Stop the npc if moving
    unit->StopMoving();

    BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(unit->GetEntry());

    if (!_player->GetBGAccessByLevel(bgTypeId))
    {
                                                            // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    SendBattlegGroundList(ObjectGuid(guid), bgTypeId);
}

void WorldSession::SendBattlegGroundList(ObjectGuid guid, BattleGroundTypeId bgTypeId )
{
    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, guid, _player, bgTypeId);
    SendPacket( &data );
}

void WorldSession::HandleBattleGroundJoinOpcode(WorldPacket & recv_data )
{
    // only battlegrounds, not arena
	uint64 guid;
    uint32 bgTypeId_;
    uint32 instanceId;
    uint8 joinAsGroup;
    Group * grp;

    recv_data >> guid;                                      // battlemaster guid
    recv_data >> bgTypeId_;                                 // battleground type id (DBC id)
    recv_data >> instanceId;                                // instance id, 0 if First Available selected
    recv_data >> joinAsGroup;                               // join as group

    if (!sBattlemasterListStore.LookupEntry(bgTypeId_))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: invalid bgtype (%u) received. possible cheater? player guid %u",bgTypeId_,_player->GetGUIDLow());
        return;
    }

    bool player_battlemaster = GUID_HIPART(guid) != HIGHGUID_PLAYER;

    Creature* unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BATTLEMASTER);
    if (!player_battlemaster && !unit)
        return;

    uint32 closedFor = sWorld.getConfig(TIME_BG_CLOSEDFOR);
    uint32 closedFrom = sWorld.getConfig(TIME_BG_CLOSING);

    if (!sBattleGroundMgr.isTesting() && closedFor)
    {
        if (closedFor >= 24)
        {
            ChatHandler(this).PSendSysMessage(LANG_BG_IS_CLOSED);
            return;
        }

        if (time_t t = time(NULL))
        {
            tm* aTm = localtime(&t);
            aTm->tm_hour = closedFrom;
            aTm->tm_min = 0;
            aTm->tm_sec = 0;
            time_t startClosing = mktime(aTm);
            if (startClosing > t) // need to take previous day
                startClosing -= DAY;

            time_t endClosing = startClosing + closedFor*HOUR;

            if (t > startClosing && t < endClosing)
            {
                ChatHandler(this).PSendSysMessage(LANG_BG_IS_CLOSED_TIMED, closedFrom, (closedFrom + closedFor) >= 24 ? (closedFrom + closedFor) - 24 : (closedFrom + closedFor));
                return;
            }
        }
    }

    BattleGroundTypeId bgTypeId = BattleGroundTypeId(bgTypeId_);

    debug_log( "WORLD: Recvd CMSG_BATTLEMASTER_JOIN Message from (GUID: %u TypeId:%u)", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));

    // can do this, since it's battleground, not arena
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId, 0);

    // ignore if player is already in BG
    if (_player->InBattleGroundOrArena())
        return;

    //if (sWorld.getConfig(CONFIG_19_LVL_ADAPTATIONS) && (_player->GetLevel() != 70 && !(_player->GetLevel() >= TWINK_LEVEL_MIN && _player->GetLevel() <= TWINK_LEVEL_MAX)))
    //{
    //    ChatHandler(_player).PSendSysMessage(LANG_ONLY_FOR_19_AND_70);
    //    return;
    //}

	// x100 is only 70 lvl
	if (sWorld.isEasyRealm() || bgTypeId == BATTLEGROUND_AV)
	{
		if (_player->GetLevel() != 70)
		{
			ChatHandler(_player).PSendSysMessage(15140);
			return;
		}
	}

    // get bg instance or bg template if instance not found
    BattleGround *bg = NULL;
    if (instanceId)
        bg = sBattleGroundMgr.GetBattleGround(instanceId, bgTypeId);

    if (!bg && !(bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId)))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: no available bg / template found");
        return;
    }

    //For REALM_X100 only allow to join BG which is event-ed at the moment
    if (sWorld.getConfig(CONFIG_BG_EVENTS_ENABLED) && !sBattleGroundMgr.IsBGEventActive(bgTypeId) && !sBattleGroundMgr.IsBGEventEnding(bgTypeId))
    {
        SendBattleGroundOrArenaJoinError(_player->GetSession()->GetHellgroundString(LANG_THIS_BG_CLOSED));
        return;
    }

    BattleGroundBracketId bgBracketId = _player->GetBattleGroundBracketIdFromLevel(bgTypeId);

    uint32 ateamId = _player->GetArenaTeamId(2);
    ArenaTeam* at = sObjectMgr.GetArenaTeamById(ateamId);
    if (!at)
    {
        _player->CreateRatedBGTeam();
        ateamId = _player->GetArenaTeamId(2);
        at = sObjectMgr.GetArenaTeamById(ateamId);

        if (!at)
        {
            _player->GetSession()->SendNotInArenaTeamPacket(2);
            return;
        }
    }

    bool testing = sBattleGroundMgr.isTesting();

    // check queue conditions
    if (!joinAsGroup)
    {
        const char* err = _player->GetBattlegroundJoinError(ARENA_TYPE_NONE);
        if (err != nullptr)
        {
            SendBattleGroundOrArenaJoinError(err);
            return;
        }

        // check if already in queue
        if (_player->GetBattleGroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            //player is already in this queue
            return;
        // check if has free queue slots
        if (!_player->HasFreeBattleGroundQueueId())
            return;

        if (_player->IsSpectator()) // cant reg if spectating
        {
            SendBattleGroundOrArenaJoinError(_player->GetSession()->GetHellgroundString(LANG_BG_GROUP_MEMBER_NO_FREE_QUEUE_SLOTS));
            return;
        }
    }
    else
    {
        grp = _player->GetGroup();
        // no group found, error
        if (!grp)
            return;

        bool can = grp->CanJoinBattleGroundQueue(bgTypeId, bgQueueTypeId, 0, bg->GetMaxPlayersPerTeam(), false, 0, _player->GetSession());
        if (!testing && !can)
        {
            return;
        }
    }

    WorldLocation leaderSafePoint;
    /* should only save recall position if not isEntryPointAnywhere, cause otherwise players could teleport with the help of BG to a premium-player.
       Reg with normal BG, then group-reg with premium, and enter/leave normal BG -> voila, teleported to a premium player. We are avoiding it.*/
    if (player_battlemaster)
        leaderSafePoint = _player->GetSafeRecallPosition();

    // _player->GetGroup() was already checked, grp is already initialized
    if (joinAsGroup)
    {
        GroupQueueInfo * ginfo = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, bgBracketId, 0, false, !player_battlemaster, joinAsGroup, 0);
        for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *member = itr->getSource();
            if (!member) continue;   // this should never happen

            uint32 queueSlot = member->AddBattleGroundQueueId(bgQueueTypeId);           // add to queue

            // store entry point coords (same as leader entry point)
            if (player_battlemaster)
                member->SetBattleGroundEntryPoint(leaderSafePoint);

            WorldPacket data;
                                                            // send status packet (in queue)
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, member->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, 0, 0);
            member->SendPacketToSelf(&data);
            sBattleGroundMgr.BuildGroupJoinedBattlegroundPacket(&data, bgTypeId);
            member->SendPacketToSelf(&data);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(member, ginfo);
            //sLog.outLog(LOG_BG, "Player %s queued AS GROUP (leader %s) for bgtype %u (queue %u)", member->GetName(), grp->GetLeaderName(), bgTypeId, bgQueueTypeId);
        }
        sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
    }
    else
    {
        // already checked if queueSlot is valid, now just get it
        uint32 queueSlot = _player->AddBattleGroundQueueId(bgQueueTypeId);

        // store entry point coords
        if (player_battlemaster)
            _player->SetBattleGroundEntryPoint(leaderSafePoint);

        WorldPacket data;
                                                            // send status packet (in queue)
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, 0, 0);
        SendPacket(&data);

        GroupQueueInfo * ginfo = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, bgBracketId, 0, false, !player_battlemaster, false, 0);
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(_player, ginfo);
        sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, _player->GetBattleGroundBracketIdFromLevel(bgTypeId));
        //sLog.outLog(LOG_BG, "Player %s queued SOLO for bgtype %u (queue %u)", _player->GetName(), bgTypeId, bgQueueTypeId);
    }

    if (sWorld.getConfig(CONFIG_BATTLEGROUND_QUEUE_INFO))
    {
        uint32 queued = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].GetQueuedPlayersCount(bgBracketId);
        ChatHandler(_player).PSendSysMessage(LANG_BG_PLAYERS_QUEUED, queued);

        if (sWorld.isEasyRealm() && !_player->GetSession()->isPremium() && urand(0, 2) == 0)
            ChatHandler(_player).SendSysMessage(15624);
    }
}

void WorldSession::HandleBattleGroundPlayerPositionsOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: Recvd MSG_BATTLEGROUND_PLAYER_POSITIONS Message");

    BattleGround *bg = _player->GetBattleGround();
    if (!bg)                                                 // can't be received if player not in battleground
        return;

    if (bg->GetTypeID() == BATTLEGROUND_WS)
    {
        uint32 count1 = 0;
        uint32 count2 = 0;

        Player *ap = sObjectMgr.GetPlayerInWorld(((BattleGroundWS*)bg)->GetAllianceFlagPickerGUID());
        if (ap) ++count2;

        Player *hp = sObjectMgr.GetPlayerInWorld(((BattleGroundWS*)bg)->GetHordeFlagPickerGUID());
        if (hp) ++count2;

        WorldPacket data(MSG_BATTLEGROUND_PLAYER_POSITIONS, (4+4+16*count1+16*count2));
        data << count1;                                     // alliance flag holders count
        /*for (uint8 i = 0; i < count1; i++)
        {
            data << uint64(0);                              // guid
            data << (float)0;                               // x
            data << (float)0;                               // y
        }*/
        data << count2;                                     // horde flag holders count
        if (ap)
        {
            // Horde team can always track alliance flag picker
            if (_player->GetBGTeam() == HORDE)
            {
                data << (uint64)ap->GetGUID();
                data << (float)ap->GetPositionX();
                data << (float)ap->GetPositionY();
            }

            if (_player->GetBGTeam() == ALLIANCE)
            {
                if (BG_WS_FLAG_UPDATE_TIME < (time(NULL) - ((BattleGroundWS*)bg)->m_AllianceFlagUpdate))
                {
                    data << (uint64)ap->GetGUID();
                    data << (float)ap->GetPositionX();
                    data << (float)ap->GetPositionY();
                }
            }
        }
        if (hp)
        {
            // Alliance team can always track horde flag picker
            if (_player->GetBGTeam() == ALLIANCE)
            {
                data << (uint64)hp->GetGUID();
                data << (float)hp->GetPositionX();
                data << (float)hp->GetPositionY();
            }

            if (_player->GetBGTeam() == HORDE)
            {
                if (BG_WS_FLAG_UPDATE_TIME < (time(NULL) - ((BattleGroundWS*)bg)->m_HordeFlagUpdate))
                {
                    data << (uint64)hp->GetGUID();
                    data << (float)hp->GetPositionX();
                    data << (float)hp->GetPositionY();
                }
            }
        }

        SendPacket(&data);
    }
}

void WorldSession::HandleBattleGroundPVPlogdataOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: Recvd MSG_PVP_LOG_DATA Message");

    BattleGround *bg = _player->GetBattleGround();
    if (!bg || bg->isArena())
        return;

    WorldPacket data;
    sBattleGroundMgr.BuildPvpLogDataPacket(&data, bg);
    SendPacket(&data);

    sLog.outDebug("WORLD: Sent MSG_PVP_LOG_DATA Message");
}

void WorldSession::HandleBattleGroundListOpcode(WorldPacket &recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("WORLD: Recvd CMSG_BATTLEFIELD_LIST Message");

    uint32 bgTypeId;
    recv_data >> bgTypeId;                                  // id from DBC

    BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(bgTypeId);
    if (!bl)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: invalid bgtype received.");
        return;
    }

    WorldPacket data;
    sBattleGroundMgr.BuildBattleGroundListPacket(&data, ObjectGuid(_player->GetGUID()), _player, BattleGroundTypeId(bgTypeId));
    SendPacket(&data);
}

// port to battleground or arena or leave queue
void WorldSession::HandleBattleGroundPlayerPortOpcode(WorldPacket &recv_data)
{
    debug_log( "WORLD: Recvd CMSG_BATTLEFIELD_PORT Message");

    uint8 type;                                             // arenatype if arena
    uint8 unk2;                                             // unk, can be 0x0 (may be if was invited?) and 0x1
    uint32 instanceId;
    uint32 bgTypeId_;                                       // type id from dbc
    uint16 unk;                                             // 0x1F90 constant?
    uint8 action;                                           // enter battle 0x1, leave queue 0x0

    recv_data >> type >> unk2 >> bgTypeId_ >> unk >> action;

    if (!sBattlemasterListStore.LookupEntry(bgTypeId_))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: invalid bgtype (%u) received.",bgTypeId_);
        // update battleground slots for the player to fix his UI and sent data.
        // this is a HACK, I don't know why the client starts sending invalid packets in the first place.
        // it usually happens with extremely high latency (if debugging / stepping in the code for example)
        if (_player->InBattleGroundOrArenaQueue())
        {
            // update all queues, send invitation info if player is invited, queue info if queued
            for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
            {
                BattleGroundQueueTypeId bgQueueTypeId = _player->GetBattleGroundQueueTypeId(i);
                if (!bgQueueTypeId)
                    continue;

                BattleGroundTypeId bgTypeId = BattleGroundMgr::BGTemplateId(bgQueueTypeId);
                BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
                BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = qpMap.find(_player->GetGUID());
                // if the player is not in queue, continue
                if (itrPlayerStatus == qpMap.end())
                    continue;

                // no group information, this should never happen
                if (!itrPlayerStatus->second.GroupInfo)
                    continue;

                BattleGround* bg = NULL;

                // get possibly needed data from groupinfo
                uint8 arenatype = itrPlayerStatus->second.GroupInfo->ArenaType;
                uint8 israted = itrPlayerStatus->second.GroupInfo->IsRated;
                uint8 status = 0;


                if (!itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID)
                {
                    // not invited to bg, get template
                    bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
                    status = STATUS_WAIT_QUEUE;
                }
                else
                {
                    // get the bg we're invited to
                    bg = sBattleGroundMgr.GetBattleGround(itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID, bgTypeId);
                    status = STATUS_WAIT_JOIN;
                }

                // if bg not found, then continue
                if (!bg)
                    continue;

                // don't invite if already in the instance
                if (_player->InBattleGroundOrArena() && _player->GetBattleGround() && _player->GetBattleGround()->GetBgInstanceId() == bg->GetBgInstanceId())
                    continue;

                // re - invite player with proper data
                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, itrPlayerStatus->second.GroupInfo->Team ? itrPlayerStatus->second.GroupInfo->Team : _player->GetTeam(), i, status, bg->isBattleGround() ? INVITE_ACCEPT_WAIT_TIME_BG : INVITE_ACCEPT_WAIT_TIME_ARENA, 0, arenatype, israted);
                SendPacket(&data);
            }
        }
        return;
    }

    BattleGroundTypeId bgTypeId = BattleGroundTypeId(bgTypeId_);

    // get the bg what we were invited to
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId, type);
    BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
    BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = qpMap.find(_player->GetGUID());
    if (itrPlayerStatus == qpMap.end())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: itrplayerstatus not found.");
        return;
    }
    instanceId = itrPlayerStatus->second.GroupInfo->IsInvitedToBGInstanceGUID;

    // if action == 1, then instanceId is _required_
    if (!instanceId && action == 1)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: instance not found.");
        return;
    }

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(instanceId, bgTypeId);

    // bg template might and must be used in case of leaving queue, when instance is not created yet
    if (!bg && action == 0)
        bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattlegroundHandler: bg_template not found for type id %u.", bgTypeId);
        return;
    }

    bgTypeId = bg->GetTypeID();

    if (_player->InBattleGroundOrArenaQueue())
    {
        uint32 queueSlot = 0;
        PlayerTeam team = TEAM_NONE;
        uint32 arenatype = 0;
        bool israted = false;
        bool isEntryPointAnywhere = false;
        uint32 rating = 0;
        uint32 opponentsRating = 0;

        BattleGroundQueue::QueuedPlayersMap& qpMap2 = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
        BattleGroundQueue::QueuedPlayersMap::iterator pitr = qpMap2.find(_player->GetGUID());
        if (pitr != qpMap2.end() && pitr->second.GroupInfo)
        {
            team = pitr->second.GroupInfo->Team;
            arenatype = pitr->second.GroupInfo->ArenaType;
            israted = pitr->second.GroupInfo->IsRated;
            isEntryPointAnywhere = pitr->second.GroupInfo->IsEntryPointAnywhere;
            rating = pitr->second.GroupInfo->ArenaTeamRating;
            opponentsRating = pitr->second.GroupInfo->OpponentsTeamRating;
        }
        else
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: Invalid player queue info!");
            return;
        }

        //if player is trying to enter battleground (not arena!) and he has deserter debuff, we must just remove him from queue
        if (action != 0)
        {
            const char* err = _player->GetBattlegroundJoinError((ArenaType)arenatype, true);
            if (err != nullptr)
            {
                SendBattleGroundOrArenaJoinError(err);
                action = 0;
            }
        }

        //For REALM_X100 only allow to join BG which is event-ed at the moment
        if (arenatype == 0 && sWorld.getConfig(CONFIG_BG_EVENTS_ENABLED) && !(sBattleGroundMgr.IsBGEventActive(bgTypeId) || sBattleGroundMgr.IsBGEventEnding(bgTypeId)))
        {
            SendBattleGroundOrArenaJoinError(_player->GetSession()->GetHellgroundString(LANG_THIS_BG_CLOSED));
            action = 0;
        }

        //if (!getBattleGroundMgr()->GetDebugArenaId() && arenatype == ARENA_TYPE_3v3)
        //{
        //    uint32 current_resilience = _player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_TAKEN_SPELL);
        //    if (current_resilience < SOLO_3V3_REQUIRED_RESILIENCE)
        //    {
        //        ChatHandler(_player).PSendSysMessage(LANG_QUEUE_NOT_ENOUGH_RESILIENCE, uint32(SOLO_3V3_REQUIRED_RESILIENCE), current_resilience);
        //        return;
        //    }
        //}

		std::string ip = _player->GetSession()->m_IPHash.c_str();

        WorldPacket data;

        BattleGroundBracketId bgBracketId = _player->GetBattleGroundBracketIdFromLevel(bgTypeId);

        auto removePlayerFromQueue = [&]() {
            uint32 queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);
            _player->RemoveBattleGroundQueueId(bgQueueTypeId);
            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, team, queueSlot, STATUS_NONE, 0, 0);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), true);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, bgBracketId, arenatype, israted, rating);
            SendPacket(&data);
        };

        // todo
        bool is_leader = bool(pitr->second.GroupInfo->PremadeLeaderGUID);

        switch (action)
        {
            case 1:
                // port to battleground or arena
                team = bg->isBattleGround() ? bg->GetBestBGTeamOnDistribute(pitr->second.GroupInfo) : pitr->second.GroupInfo->Team;
                pitr->second.GroupInfo->Team = team;
                pitr->second.GroupInfo->CurrentBGId = bg->GetBgInstanceId();

                if (team == TEAM_NONE)
                {
                    ChatHandler(_player).PSendSysMessage(16746);
                    removePlayerFromQueue();
                }
                
                if (_player->IsSpectator() && _player->InArena())
                    _player->LeaveBattleground();

                // cheating?
                if (!_player->IsInvitedForBattleGroundQueueType(bgQueueTypeId))
                    return;              

                queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId);

                // resurrect the player
                if (!_player->isAlive())
                {
                    _player->ResurrectPlayer(1.0f);
                    _player->SpawnCorpseBones();
                }
                
                if (isEntryPointAnywhere) // must be done before interrupting taxi to not recall into the air after BG ends
                    _player->SaveOwnBattleGroundEntryPoint();

                // stop taxi flight at port
                _player->InterruptTaxiFlying();

                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, team, queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
                _player->SendPacketToSelf(&data);
                // remove battleground queue status from BGmgr
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(_player->GetGUID(), false);
                // this is still needed here if battleground "jumping" shouldn't add deserter debuff
                // also this required to prevent stuck at old battleground after SetBattleGroundId set to new
                if (BattleGround *currentBg = _player->GetBattleGround())
                    currentBg->RemovePlayerAtLeave(_player->GetGUID(), false, true, false);

                // set the destination instance id
                _player->SetBattleGroundId(bg->GetBgInstanceId(), bgTypeId);
                // set the destination team
                _player->SetBGTeam(team);
                if (sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION) && sBattleGroundMgr.IsBattleGroundType(bgTypeId))
                {
                    if (team == ALLIANCE && _player->GetTeam() == HORDE)
                    {
                        _player->setFactionForRace(RACE_HUMAN);
                    }
                    else if (team == HORDE && _player->GetTeam() == ALLIANCE)
                    {
                        _player->setFactionForRace(RACE_ORC);
                    }
                }

                _player->WorthHonor = false; // actually he is worth honor, it is vice versa as it's spelled.

                if (arenatype && israted) // is arena and israted
                {
                    if (Item* thrown = _player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
                        if (thrown->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) >= 200 /*thrown for sure*/)
                            _player->DurabilityRepair(((INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_RANGED), false, 0, false);
                }

				// player started teleporting, but not yet added to battleground
                // @todo
                if (sBattleGroundMgr.SendToBattleGround(_player, instanceId, bgTypeId) && bg->isBattleGround())
                {
                    _player->bg_premade_leader_guid = is_leader; // pitr->second.GroupInfo is removing somewhere... debug: volatile uint32 a = pitr->second.GroupInfo->PremadeLeaderGUID;
                }

                // Left for Dead
                //_player->RemoveAurasDueToSpell(55115);

                //sLog.outLog(arenatype ? LOG_ARENA : LOG_BG, "ID %u: Player %s ported, arenatype %u", instanceId, _player->GetName(), arenatype);

                break;
            case 0:
			{
				// leave queue
                removePlayerFromQueue();
				break;
			}
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Battleground port: unknown action %u", action);
                break;
        }
    }
}

void WorldSession::HandleBattleGroundLeaveOpcode(WorldPacket &recv_data)
{
    sLog.outDebug("WORLD: Recvd CMSG_LEAVE_BATTLEFIELD Message");

    if (_player->IsSpectator() && _player->InArena())
    {
        _player->SetSpectator(false);
        _player->TeleportTo(_player->_recallPosition.mapid, _player->_recallPosition.coord_x, _player->_recallPosition.coord_y, _player->_recallPosition.coord_z, _player->_recallPosition.orientation);
        return;
    }

    WorldPacket recv_data_temp = recv_data;
    uint8 type;
    recv_data_temp >> type;

    // don't allow to leave from solo 3v3 when STATUS_WAIT_JOIN
    if (BattleGround *bg = _player->GetBattleGround())
        if (type == ARENA_TYPE_3v3 && bg->GetStatus() == STATUS_WAIT_JOIN)
            return;

    recv_data.read_skip<uint8>();                           // unk1
    recv_data.read_skip<uint8>();                           // unk2
    recv_data.read_skip<uint32>();                          // BattleGroundTypeId
    recv_data.read_skip<uint16>();                          // unk3

    _player->LeaveBattleground();
}

void WorldSession::HandleBattlefieldStatusOpcode(WorldPacket & /*recv_data*/)
{
    // empty opcode
    debug_log( "WORLD: Battleground status" );

    WorldPacket data;

    // TODO: we must put player back to battleground in case disconnect (< 5 minutes offline time) or teleport player on login(!) from battleground map to entry point
    if (_player->InBattleGroundOrArena())
    {
        BattleGround *bg = _player->GetBattleGround();
        if (bg)
        {
            BattleGroundQueueTypeId bgQueueTypeId_tmp = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
            uint32 queueSlot = _player->GetBattleGroundQueueIndex(bgQueueTypeId_tmp);
            if ((bg->GetStatus() <= STATUS_IN_PROGRESS))
            {
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetBGTeam(), queueSlot, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
                SendPacket(&data);
            }
            for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
            {
                BattleGroundQueueTypeId bgQueueTypeId = _player->GetBattleGroundQueueTypeId(i);
                if (i == queueSlot || !bgQueueTypeId)
                    continue;
                BattleGroundTypeId bgTypeId = BattleGroundMgr::BGTemplateId(bgQueueTypeId);
                uint8 arenatype = BattleGroundMgr::BGArenaType(bgQueueTypeId);
                uint8 isRated = 0;
                PlayerTeam team = _player->GetTeam();
                BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
                BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = qpMap.find(_player->GetGUID());
                if (itrPlayerStatus == qpMap.end())
                    continue;
                if (itrPlayerStatus->second.GroupInfo)
                {
                    arenatype = itrPlayerStatus->second.GroupInfo->ArenaType;
                    isRated = itrPlayerStatus->second.GroupInfo->IsRated;
                    team = itrPlayerStatus->second.GroupInfo->Team;
                }
                BattleGround *bg2 = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
                if (bg2)
                {
                    //in this call is small bug, this call should be filled by player's waiting time in queue
                    //this call nulls all timers for client :
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg2, team, i, STATUS_WAIT_QUEUE, 0, 0,arenatype,isRated);
                    SendPacket(&data);
                }
            }
        }
    }
    else
    {
        // we should update all queues? .. i'm not sure if this code is correct
        for (uint32 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; i++)
        {
            BattleGroundQueueTypeId bgQueueTypeId = _player->GetBattleGroundQueueTypeId(i);
            if (!bgQueueTypeId)
                continue;

            BattleGroundTypeId bgTypeId = BattleGroundMgr::BGTemplateId(bgQueueTypeId);
            uint8 arenatype = BattleGroundMgr::BGArenaType(bgQueueTypeId);
            uint8 isRated = 0;
            PlayerTeam team = _player->GetTeam();
            BattleGround *bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
            BattleGroundQueue::QueuedPlayersMap::iterator itrPlayerStatus = qpMap.find(_player->GetGUID());
            if (itrPlayerStatus == qpMap.end())
                continue;
            if (itrPlayerStatus->second.GroupInfo)
            {
                arenatype = itrPlayerStatus->second.GroupInfo->ArenaType;
                isRated = itrPlayerStatus->second.GroupInfo->IsRated;
                team = itrPlayerStatus->second.GroupInfo->Team;
            }
            if (bg)
            {
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, team, i, STATUS_WAIT_QUEUE, 0, 0, arenatype, isRated);
                SendPacket(&data);
            }
        }
    }
}

void WorldSession::HandleAreaSpiritHealerQueryOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("WORLD: CMSG_AREA_SPIRIT_HEALER_QUERY");

    CHECK_PACKET_SIZE(recv_data, 8);

    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    uint64 guid;
    recv_data >> guid;

    Creature *unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                            // it's not spirit service
        return;

    sBattleGroundMgr.SendAreaSpiritHealerQueryOpcode(_player, bg, guid);
}

void WorldSession::HandleAreaSpiritHealerQueueOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("WORLD: CMSG_AREA_SPIRIT_HEALER_QUEUE");

    CHECK_PACKET_SIZE(recv_data, 8);

    BattleGround *bg = _player->GetBattleGround();
    if (!bg)
        return;

    uint64 guid;
    recv_data >> guid;

    Creature *unit = GetPlayer()->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->isSpiritService())                            // it's not spirit service
        return;

    bg->AddPlayerToResurrectQueue(guid, _player->GetGUID());
}

void WorldSession::HandleArenaJoinOpcode(WorldPacket & recv_data)
{
    debug_log("WORLD: CMSG_BATTLEMASTER_JOIN_ARENA");
    //recv_data.hexlike();

    uint64 guid;                                            // arena Battlemaster guid
    uint8 arenaslot;                                        // 2v2, 3v3 or 5v5
    uint8 asGroup;                                          // asGroup
    uint8 isRated;                                          // isRated
    Group * grp;

    recv_data >> guid >> arenaslot >> asGroup >> isRated;

    // ignore if we already in BG or BG queue
    if (_player->InBattleGroundOrArena())
        return;

    bool player_battlemaster = GUID_HIPART(guid) != HIGHGUID_PLAYER;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BATTLEMASTER);
    if (!player_battlemaster && !unit)
        return;

    uint8 arenatype = 0;
    uint32 arenaRating = 0;

    arenatype = BattleGroundMgr::getArenaTypeBySlot(arenaslot);
    if (arenatype == ARENA_TYPE_NONE)
        return;

    // disallow 3v3 arena as group and skirmish (needed by solo 3v3)
    if (arenatype == ARENA_TYPE_3v3 && (asGroup || !isRated))
        return;

	// disabled because it's used as rated BG
	if (arenatype == ARENA_TYPE_5v5)
		return;

    // check existance
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(BATTLEGROUND_AA);
    if (!bg)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: template bg (all arenas) not found");
        return;
    }

    BattleGroundTypeId bgTypeId = bg->GetTypeID();
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bgTypeId, arenatype);
    BattleGroundBracketId bgBracketId = _player->GetBattleGroundBracketIdFromLevel(bgTypeId);

	uint32 ateamId = 0;
	if (isRated)
	{
		ateamId = _player->GetArenaTeamId(arenaslot);
		// check real arena team existence only here (if it was moved to group->CanJoin .. () then we would have to get it twice)
		ArenaTeam * at = sObjectMgr.GetArenaTeamById(ateamId);
		if (!at)
		{
            if (arenatype == ARENA_TYPE_2v2 || _player->GetLevel() < 70)
            {
                _player->GetSession()->SendNotInArenaTeamPacket(arenatype);
                return;
            }
            else if (arenatype == ARENA_TYPE_3v3)
            {
                _player->CreateRated3v3Team();
            }

            ateamId = _player->GetArenaTeamId(arenaslot);
            at = sObjectMgr.GetArenaTeamById(ateamId);

            if (!at)
            {
                _player->GetSession()->SendNotInArenaTeamPacket(arenatype);
                return;
            }
		}

		// get the team rating for queue
		arenaRating = at->GetRating();

        // time checks
        if (isRated)
        {
            uint32 end = sWorld.getConfig(CONFIG_ARENA_SEASON_END);
            uint32 t = time(NULL);
            if (end && end < t)
            {
                ChatHandler(this).SendSysMessage(15579);
                return;
            }

            if (time_t t = time(NULL))
            {
                tm* aTm = localtime(&t);

                uint32 closedFor;
                uint32 closedFrom;
                switch (arenatype)
                {
                case ARENA_TYPE_2v2:
                    closedFrom = sWorld.getConfig(TIME_ARENA_CLOSING2v2);
                    closedFor = sWorld.getConfig(TIME_ARENA_CLOSEDFOR2v2);
                    break;
                case ARENA_TYPE_3v3:
                    closedFrom = sWorld.getConfig(TIME_ARENA_CLOSING3v3);
                    closedFor = sWorld.getConfig(TIME_ARENA_CLOSEDFOR3v3);
                    break;
                case ARENA_TYPE_5v5:
                    closedFrom = sWorld.getConfig(TIME_ARENA_CLOSING5v5);
                    closedFor = sWorld.getConfig(TIME_ARENA_CLOSEDFOR5v5);
                    break;
                    //case ARENA_TYPE_1v1:
                    //    closedFrom = sWorld.getConfig(TIME_ARENA_CLOSING1v1);
                    //    closedFor = sWorld.getConfig(TIME_ARENA_CLOSEDFOR1v1);
                    //    break;
                default:
                    return; // no other possible
                }

                // actualize 2v2 arena - make it available instead of 3v3
                // Time.Arena2v2.ClosedFor = 21
                // Time.Arena2v2.Closing = 1
                // Time.Arena3v3.ClosedFor = 3
                // Time.Arena3v3.Closing = 22
                // - seems ok
                if (closedFor)
                {
                    if (closedFor >= 24)
                    {
                        ChatHandler(this).PSendSysMessage(LANG_ARENA_IS_CLOSED, arenatype, arenatype);
                        return;
                    }

                    aTm->tm_hour = closedFrom;
                    aTm->tm_min = 0;
                    aTm->tm_sec = 0;
                    time_t startClosing = mktime(aTm);
                    if (startClosing > t) // need to take previous day
                        startClosing -= DAY;

                    time_t endClosing = startClosing + closedFor * HOUR;

                    if (t > startClosing && t < endClosing)
                    {
                        time_t t = time(NULL);
                        tm* aTm = localtime(&t);

                        char exp_chr[32];
                        snprintf(exp_chr, 32, "%02d:%02d:%02d", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
                        
                        ChatHandler(this).PSendSysMessage(LANG_ARENA_IS_CLOSED_TIMED, arenatype, arenatype, closedFrom, (closedFrom + closedFor) >= 24 ? (closedFrom + closedFor) - 24 : (closedFrom + closedFor), exp_chr);
                        return;
                    }
                }
            }
        }
	}

    // check queue conditions
	if (!asGroup)
	{
		// check if already in queue
		if (_player->GetBattleGroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
			//player is already in this queue
			return;
		// check if has free queue slots
		if (!_player->HasFreeBattleGroundQueueId())
			return;
		if (_player->IsSpectator()) // cant reg if someone is spectating
		{
			SendBattleGroundOrArenaJoinError(_player->GetSession()->GetHellgroundString(LANG_BG_GROUP_MEMBER_NO_FREE_QUEUE_SLOTS));
			return;
		}

        const char* err = _player->GetBattlegroundJoinError((ArenaType)arenatype);
        if (err != nullptr)
        {
            SendBattleGroundOrArenaJoinError(err);
            return;
        }
    }
    else
    {
        grp = _player->GetGroup();
        // no group found, error
        if (!grp)
            return;
        bool can = grp->CanJoinBattleGroundQueue(bgTypeId, bgQueueTypeId, arenatype, arenatype, (bool)isRated, arenaslot, _player->GetSession());
        if (!can)
        {
            return;
        }   
    }

    WorldLocation leaderSafePoint;
    /* should only save recall position if not isEntryPointAnywhere, cause otherwise players could teleport with the help of BG to a premium-player.
    Reg with normal BG, then group-reg with premium, and enter/leave normal BG -> voila, teleported to a premium player. We are avoiding it.*/
    if (player_battlemaster)
        leaderSafePoint = _player->GetSafeRecallPosition();

    if (asGroup)
    {
        bool as_premade = !isRated;

        GroupQueueInfo * ginfo = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, bgBracketId, arenatype, isRated, !player_battlemaster, as_premade, arenaRating, ateamId);

        //if (isRated && arenaslot == 0)
        //    sLog.outLog(LOG_SPECIAL, "2v2 queue: arena team id %u, leader %s queued with rating %u", _player->GetArenaTeamId(arenaslot), _player->GetName(), arenaRating);
        for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *member = itr->getSource();
            if (!member) continue;

            uint32 queueSlot = member->AddBattleGroundQueueId(bgQueueTypeId);// add to queue
			member->AddArenaQueueHashIP(1); // checked before at CanJoinBattleGroundQueue

            // store entry point coords (same as leader entry point)
            if (player_battlemaster)
                member->SetBattleGroundEntryPoint(leaderSafePoint);

            WorldPacket data;
            // send status packet (in queue)
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, member->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, 0, 0, arenatype, isRated);
            member->SendPacketToSelf(&data);
            sBattleGroundMgr.BuildGroupJoinedBattlegroundPacket(&data, bgTypeId);
            member->SendPacketToSelf(&data);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(member, ginfo);

            // should never happen
            if (arenatype == ARENA_TYPE_3v3)
                sLog.outLog(LOG_CRITICAL, "Arena3v3 ERROR: Player %s joined queue AS GROUP for arena queue type %u bg type %u", member->GetName(), bgQueueTypeId, bgTypeId);

            if (ginfo->ArenaTeamRating >= 1800 && !member->IsHidden())
                ChatHandler(member).PSendSysMessage(15559);
        }

        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattleGroundBracketIdFromLevel(bgTypeId), arenatype, isRated, arenaRating);
    }
    else
    {
        uint32 queueSlot = _player->AddBattleGroundQueueId(bgQueueTypeId);

		_player->AddArenaQueueHashIP(2); // checked before at CanJoinBattleGroundQueue

        // store entry point coords
        if (player_battlemaster)
            _player->SetBattleGroundEntryPoint(leaderSafePoint);

        WorldPacket data;
        // send status packet (in queue)
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), queueSlot, STATUS_WAIT_QUEUE, 0, 0, arenatype, isRated);
        SendPacket(&data);
        GroupQueueInfo * ginfo = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddGroup(_player, bgTypeId, bgBracketId, arenatype, isRated, !player_battlemaster, false, arenaRating);
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].AddPlayer(_player, ginfo);
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, _player->GetBattleGroundBracketIdFromLevel(bgTypeId), arenatype, isRated, arenaRating);
        
        if (arenatype == ARENA_TYPE_3v3)
            sLog.outLog(LOG_ARENA, "Player %s joined Arena3v3 bg type %u (queue %u) talents %s, healer %u, in queue %u", _player->GetName(), bgTypeId, bgQueueTypeId, _player->PrintTalentCount(_player->GetActiveSpec()).c_str(), _player->IsSemiHealer(), sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].GetQueuedPlayersCount(bgBracketId));
    }

    // queue stats for all arenas
    int string_id;
    switch (arenatype)
    {
        case ARENA_TYPE_3v3:
			string_id = 15268;
			break;
        case ARENA_TYPE_2v2:
            if (isRated)
				string_id = 16751;
			else
				string_id = 16750;
            break;
        default:
            string_id = 0;
    }

    ChatHandler(_player).PSendSysMessage(string_id, sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].GetQueuedPlayersCount(bgBracketId));

    if (sWorld.isEasyRealm() && !_player->GetSession()->isPremium() && urand(0, 2) == 2)
        ChatHandler(_player).SendSysMessage(15625);
}

void WorldSession::HandleBattleGroundReportAFK(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    uint64 playerGuid;
    recv_data >> playerGuid;
    Player *reportedPlayer = sObjectMgr.GetPlayerInWorld(playerGuid);

    if (!reportedPlayer || !_player->GetBattleGround())
    {
        ChatHandler(_player).SendSysMessage(499);
        return;
    }

    reportedPlayer->ReportedAfkBy(_player);
}

void WorldSession::SendBattleGroundOrArenaJoinError(const char* msg)
{
	WorldPacket data;

    //ChatHandler(_player).SendSysMessage(err);
    //WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    //data << (uint32)0xFFFFFFFE;
    //_player->SendPacketToSelf(&data);

	//if (err == BG_JOIN_ERR_GROUP_TOO_MANY_BG && maxPlayers)
	//{
	//	char chr[255];
	//	sprintf(chr, GetHellgroundString(16556), maxPlayers);
	//	msg = chr;
	//}
	//else
	//{
	//	uint32 string;

	//	switch (err)
	//	{
	//	case BG_JOIN_ERR_OFFLINE_MEMBER:
	//		string = LANG_BG_GROUP_OFFLINE_MEMBER;
	//		break;
	//	case BG_JOIN_ERR_GROUP_TOO_MANY:
	//		string = LANG_BG_GROUP_TOO_LARGE;
	//		break;
	//	case BG_JOIN_ERR_MIXED_FACTION:
	//		string = LANG_BG_GROUP_MIXED_FACTION;
	//		break;
	//	case BG_JOIN_ERR_MIXED_LEVELS:
	//		string = LANG_BG_GROUP_MIXED_LEVELS;
	//		break;
	//	case BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE:
	//		string = LANG_BG_GROUP_MEMBER_ALREADY_IN_QUEUE;
	//		break;
	//	case BG_JOIN_ERR_GROUP_DESERTER:
	//		string = LANG_BG_GROUP_MEMBER_DESERTER;
	//		break;
	//	case BG_JOIN_ERR_ALL_QUEUES_USED:
	//		string = LANG_BG_GROUP_MEMBER_NO_FREE_QUEUE_SLOTS;
	//		break;
	//	case BG_JOIN_ERR_GROUP_NOT_ENOUGH:
	//	case BG_JOIN_ERR_MIXED_ARENATEAM:
	//		return;
	//	case BG_JOIN_ERR_BG_RESTRICTED:
	//		string = LANG_BG_GROUP_MEMBER_ARENA_RESTRICTED;
	//		break;
	//	case BG_JOIN_ERR_BG_CLOSED:
	//		string = LANG_THIS_BG_CLOSED;
	//		break;
 //       case BG_JOIN_ERR_BG_SAME_IP:
 //           string = 16624;
 //           break;
	//	default:
	//		return;
	//	}

	//	msg = GetHellgroundString(string);
	//}

	ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, msg, NULL);
	SendPacket(&data);
    return;
}
