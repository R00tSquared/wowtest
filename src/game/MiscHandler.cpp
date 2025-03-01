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
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "Database/DatabaseImpl.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "Player.h"
#include "GossipDef.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "UpdateData.h"
#include "LootMgr.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include <zlib/zlib.h>
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Object.h"
#include "BattleGround.h"
#include "OutdoorPvP.h"
#include "SpellAuras.h"
#include "Pet.h"
#include "SocialMgr.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "Group.h"
#include "Spell.h"
#include "GuildMgr.h"
#include "ChannelMgr.h"

void WorldSession::HandleRepopRequestOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: Recvd CMSG_REPOP_REQUEST Message");

    if (GetPlayer()->isAlive() || GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    // the world update order is sessions, players, creatures
    // the netcode runs in parallel with all of these
    // creatures can kill players
    // so if the server is lagging enough the player can
    // release spirit after he's killed but before he is updated
    if (GetPlayer()->getDeathState() == JUST_DIED)
    {
        sLog.outDebug("HandleRepopRequestOpcode: got request after player %s(%d) was killed and before he was updated", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        GetPlayer()->KillPlayer();
    }

    //this is spirit release confirm?
    GetPlayer()->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT, true);
    GetPlayer()->BuildPlayerRepop();
    GetPlayer()->RepopAtGraveyard();
}

void WorldSession::HandleWhoOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4+1+1+4+4+4+4);

    sLog.outDebug("WORLD: Recvd CMSG_WHO Message");
    //recv_data.hexlike();

    uint32 clientcount = 0;

    uint32 level_min, level_max, racemask, classmask, zones_count, str_count;
    uint32 zoneids[10];                                     // 10 is client limit
    std::string player_name, guild_name;

    recv_data >> level_min;                                 // maximal player level, default 0
    recv_data >> level_max;                                 // minimal player level, default 100 (MAX_LEVEL)
    recv_data >> player_name;                               // player name, case sensitive...

    // recheck
    CHECK_PACKET_SIZE(recv_data,4+4+(player_name.size()+1)+1+4+4+4+4);

    recv_data >> guild_name;                                // guild name, case sensitive...

    // recheck
    CHECK_PACKET_SIZE(recv_data,4+4+(player_name.size()+1)+(guild_name.size()+1)+4+4+4+4);

    recv_data >> racemask;                                  // race mask
    recv_data >> classmask;                                 // class mask
    recv_data >> zones_count;                               // zones count, client limit=10 (2.0.10)

    if (zones_count > 10)
        return;                                             // can't be received from real client or broken packet

    // recheck
    CHECK_PACKET_SIZE(recv_data,4+4+(player_name.size()+1)+(guild_name.size()+1)+4+4+4+(4*zones_count)+4);

    for (uint32 i = 0; i < zones_count; i++)
    {
        uint32 temp;
        recv_data >> temp;                                  // zone id, 0 if zone is unknown...
        zoneids[i] = temp;
        sLog.outDebug("Zone %u: %u", i, zoneids[i]);
    }

    recv_data >> str_count;                                 // user entered strings count, client limit=4 (checked on 2.0.10)

    if (str_count > 4)
        return;                                             // can't be received from real client or broken packet

    // recheck
    CHECK_PACKET_SIZE(recv_data,4+4+(player_name.size()+1)+(guild_name.size()+1)+4+4+4+(4*zones_count)+4+(1*str_count));

    sLog.outDebug("Minlvl %u, maxlvl %u, name %s, guild %s, racemask %u, classmask %u, zones %u, strings %u", level_min, level_max, player_name.c_str(), guild_name.c_str(), racemask, classmask, zones_count, str_count);

    std::wstring str[4];                                    // 4 is client limit
    for (uint32 i = 0; i < str_count; i++)
    {
        // recheck (have one more byte)
        CHECK_PACKET_SIZE(recv_data,recv_data.rpos());

        std::string temp;
        recv_data >> temp;                                  // user entered string, it used as universal search pattern(guild+player name)?

        if (!Utf8toWStr(temp,str[i]))
            continue;

        wstrToLower(str[i]);

        sLog.outDebug("String %u: %s", i, temp.c_str());
    }

    std::wstring wplayer_name;
    std::wstring wguild_name;
    if (!(Utf8toWStr(player_name, wplayer_name) && Utf8toWStr(guild_name, wguild_name)))
        return;
    wstrToLower(wplayer_name);
    wstrToLower(wguild_name);

    // client send in case not set max level value 100 but mangos support 255 max level,
    // update it to show GMs with characters after 100 level
    if (level_max >= MAX_LEVEL)
        level_max = STRONG_MAX_LEVEL;

    uint32 team = _player->GetTeam();
    bool allowTwoSideWhoList = sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST);
    bool gmInWhoList         = sWorld.getConfig(CONFIG_GM_IN_WHO_LIST);

    WorldPacket data(SMSG_WHO, 50);                         // guess size
    data << clientcount;                                    // clientcount place holder
    data << clientcount;                                    // clientcount place holder

    bool countFound = sWorld.getConfig(CONFIG_WHO_SHOW_OVERALL);

    ACE_GUARD(ACE_Thread_Mutex, guard, *HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (!HasPermissions(PERM_GMT_HDEV))
        {
            // player can see member of other team only if CONFIG_ALLOW_TWO_SIDE_WHO_LIST
            if (itr->second->GetTeam() != team && !allowTwoSideWhoList)
                continue;

            // player can see MODERATOR, GAME MASTER, ADMINISTRATOR only if CONFIG_GM_IN_WHO_LIST
            if (itr->second->GetSession()->HasPermissions(PERM_GMT) && !gmInWhoList)
                continue;
        }

        //do not process players which are not in world
        if (!itr->second->IsInWorld())
            continue;

        // check if target is globally visible for player
        if (!itr->second->IsVisibleGloballyfor(_player) && !itr->second->GetSession()->isFakeBot())
            continue;

        // check if target's level is in level range
        uint32 lvl = itr->second->GetLevel();
        if (lvl < level_min || lvl > level_max)
            continue;

        // check if class matches classmask
        uint32 class_ = itr->second->GetClass();
        if (!(classmask & (1 << class_)))
            continue;

        // check if race matches racemask
        uint32 race = itr->second->GetRace();
        if (!(racemask & (1 << race)))
            continue;

        bool hidden = itr->second->IsHidden();

        if (hidden)
        {
            race = 0;
            class_ = 0;
        }

        uint32 pzoneid = hidden ? 0 : itr->second->GetCachedZone();
        if (pzoneid && !GetPlayer()->isGameMaster() && sWorld.getConfig(CONFIG_ENABLE_FAKE_WHO_ON_ARENA) && itr->second->InArena())
        {
            Player *pPlayer = itr->second;
            WorldLocation const& loc = pPlayer->GetBattleGroundEntryPoint();
            pzoneid = sTerrainMgr.GetZoneId(loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z);
        }

        if (itr->second->GetSession()->isFakeBot())
            pzoneid = itr->second->fakebot_zone_id;

        uint8 gender = hidden ? GENDER_NONE : itr->second->GetGender();

        bool z_show = true;
        for (uint32 i = 0; i < zones_count; i++)
        {
            if (zoneids[i] == pzoneid)
            {
                z_show = true;
                break;
            }

            z_show = false;
        }
        if (!z_show)
            continue;

        std::string pname = hidden ? "#hidden" : itr->second->GetName();
        std::wstring wpname;
        if (!Utf8toWStr(pname,wpname))
            continue;
        wstrToLower(wpname);

        if (!(wplayer_name.empty() || wpname.find(wplayer_name) != std::wstring::npos))
            continue;

        std::string gname = hidden ? "#hidden" : sGuildMgr.GetGuildNameById(itr->second->GetGuildId());
        std::wstring wgname;
        if (!Utf8toWStr(gname,wgname))
            continue;
        wstrToLower(wgname);

        if (!(wguild_name.empty() || wgname.find(wguild_name) != std::wstring::npos))
            continue;

        std::string aname;
        if (AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(pzoneid))
            aname = areaEntry->area_name[GetSessionDbcLocale()];

        bool s_show = true;
        for (uint32 i = 0; i < str_count; i++)
        {
            if (!str[i].empty())
            {
                if (wgname.find(str[i]) != std::wstring::npos ||
                    wpname.find(str[i]) != std::wstring::npos ||
                    Utf8FitTo(aname, str[i]))
                {
                    s_show = true;
                    break;
                }

                s_show = false;
            }
        }
        if (!s_show)
            continue;

        // 50 is maximum player count sent to client - can be overridden
        // through config, but is unstable
        if (++clientcount <= 50)
        {
            data << pname;                                  // player name
            data << gname;                                  // guild name
            data << uint32(lvl);                            // player level
            data << uint32(class_);                         // player class
            data << uint32(race);                           // player race
            data << uint8(gender);                          // player gender
            data << uint32(pzoneid);                        // player zone id

            if (!countFound && clientcount == 50)
                break;
        }
    }

    data.put(0, clientcount <= 50 ? clientcount : 50);                //insert right count
    data.put(4, clientcount);                                         //insert right count

    SendPacket(&data);
    sLog.outDebug("WORLD: Send SMSG_WHO Message");
}

void WorldSession::HandleLogoutRequestOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: Recvd CMSG_LOGOUT_REQUEST Message, security - %llu", GetPermissions());

    if (uint64 lguid = GetPlayer()->GetLootGUID())
        DoLootRelease(lguid);

    //Can not logout if...
    if (!GetPlayer()->isGameMaster())
    {
        if (GetPlayer()->IsInCombat() ||                        //...is in combat
            GetPlayer()->duel ||                        //...is in Duel
            GetPlayer()->HasAura(9454, 0) ||             //...is frozen by GM via freeze command
            //...is jumping ...is falling
            GetPlayer()->HasUnitMovementFlag(MOVEFLAG_FALLING | MOVEFLAG_FALLINGFAR))
        {
            WorldPacket data(SMSG_LOGOUT_RESPONSE, (2 + 4));
            data << uint8(0xC);
            data << uint32(0);
            data << uint8(0);
            SendPacket(&data);
            LogoutRequest(0);
            return;
        }
    }

    //instant logout in taverns/cities or on taxi or for admins, gm's, mod's if its enabled in mangosd.conf
    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || GetPlayer()->IsTaxiFlying() ||
        HasPermissions(sWorld.getConfig(CONFIG_INSTANT_LOGOUT)))
    {
        LogoutPlayer(true);
        return;
    }

    // not set flags if player can't free move to prevent lost state at logout cancel
    if (GetPlayer()->CanFreeMove())
    {
        if (!GetPlayer()->IsFlying())
            GetPlayer()->SetStandState(UNIT_STAND_STATE_SIT);

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, (8+4));    // guess size
        data << GetPlayer()->GetPackGUID();
        data << (uint32)2;
        SendPacket(&data);
        GetPlayer()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    }

    WorldPacket data(SMSG_LOGOUT_RESPONSE, 5);
    data << uint32(0);
    data << uint8(0);
    SendPacket(&data);
    LogoutRequest(time(NULL));
}

void WorldSession::HandlePlayerLogoutOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: Recvd CMSG_PLAYER_LOGOUT Message");
}

void WorldSession::HandleLogoutCancelOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: Recvd CMSG_LOGOUT_CANCEL Message");

    LogoutRequest(0);

    WorldPacket data(SMSG_LOGOUT_CANCEL_ACK, 0);
    SendPacket(&data);

    // not remove flags if can't free move - its not set in Logout request code.
    if (GetPlayer()->CanFreeMove())
    {
        //!we can move again
        data.Initialize(SMSG_FORCE_MOVE_UNROOT, 8);       // guess size
        data << GetPlayer()->GetPackGUID();
        data << uint32(0);
        SendPacket(&data);

        //! Stand Up
        GetPlayer()->SetStandState(UNIT_STAND_STATE_STAND);

        //! DISABLE_ROTATE
        if (!GetPlayer()->isCharmed())
            GetPlayer()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    }

    sLog.outDebug("WORLD: sent SMSG_LOGOUT_CANCEL_ACK Message");
}

void WorldSession::HandleTogglePvP(WorldPacket & recv_data)
{
    // this opcode can be used in two ways: Either set explicit new status or toggle old status
    if (recv_data.size() == 1)
    {
        bool newPvPStatus;
        recv_data >> newPvPStatus;
        GetPlayer()->ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, newPvPStatus);
    }
    else
        GetPlayer()->ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);

    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY) || sWorld.isPvPArea(GetPlayer()->GetCachedArea()))
        GetPlayer()->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);

    if (GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
    {
        if (!GetPlayer()->IsPvP() || GetPlayer()->pvpInfo.endTimer != 0)
            GetPlayer()->UpdatePvP(true, true);
    }
    else
    {
        if (!GetPlayer()->pvpInfo.inHostileArea && GetPlayer()->IsPvP())
            GetPlayer()->pvpInfo.endTimer = time(NULL);     // start toggle-off
    }

    /*if(OutdoorPvP * pvp = _player->GetOutdoorPvP())
    {
        pvp->HandlePlayerActivityChanged(_player);
    }*/
}

void WorldSession::HandleZoneUpdateOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 newZone;
    recv_data >> newZone;

    sLog.outDetail("WORLD: Recvd ZONE_UPDATE: %u", newZone);

    Player* p = GetPlayer();

    // not changing the zone, but only area
    if (newZone == p->GetCachedZone())
    {
        uint32 newarea = p->GetAreaId();
        if (p->GetCachedArea() != newarea)
            p->UpdateArea(newarea);
    }
    else // changing the zone -> check on the server if change is accepted (otherwise will change zone in the update)
    {
        uint32 realZone = p->GetZoneId();
        if (realZone != p->GetCachedZone())
            p->UpdateZone(realZone);
        else // not yet in the zone server-side -> schedule zone update soon
            p->ScheduleZoneUpdate();
    }

    // sent at real zone change in the OutdoorPvPMgr::HandlePlayerEnterZone
    // GetPlayer()->SendInitWorldStates(true,newZone);
}

void WorldSession::HandleSetTargetOpcode(WorldPacket & recv_data)
{
    // When this packet send?
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid ;
    recv_data >> guid;

    _player->SetUInt64Value(UNIT_FIELD_TARGET,guid); // should be obsolete

    // update reputation list if need
    Unit* unit = _player->GetMap()->GetUnit(guid);
    if (!unit)
        return;

    if (FactionTemplateEntry const* factionTemplateEntry = sFactionTemplateStore.LookupEntry(unit->getFaction()))
        _player->GetReputationMgr().SetVisible(factionTemplateEntry);
}

void WorldSession::HandleSetSelectionOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    if (_player->HasUnitState(UNIT_STAT_MELEE_ATTACKING)) // Client ALWAYS sends stop_attack before switching targets.
        // Player can only haave UNIT_STAT_MELEE_ATTACKING if it didn't know about the attack at the moment of switch (cause of ping) -> thus it can't stop attack themself,
        // so we need to do it here, server-side
    {
        if (_player->getVictimGUID() != guid)
            _player->AttackStop();
    }
    _player->SaveSelection(guid); // also sets selection inside

    // update reputation list if need
    Unit* unit = _player->GetMap()->GetUnit(guid);
    if (!unit)
        return;

    if (FactionTemplateEntry const* factionTemplateEntry = sFactionTemplateStore.LookupEntry(unit->getFaction()))
        _player->GetReputationMgr().SetVisible(factionTemplateEntry);
}

void WorldSession::HandleStandStateChangeOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    sLog.outDebug("WORLD: Received CMSG_STAND_STATE_CHANGE" );
    uint8 animstate;
    recv_data >> animstate;

    if (!GetPlayer()->isPossessed() && !GetPlayer()->isCharmed())
        _player->SetStandState(animstate);
}

void WorldSession::HandleFriendListOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4);
    sLog.outDebug("WORLD: Received CMSG_CONTACT_LIST");
    uint32 unk;
    recv_data >> unk;
    sLog.outDebug("unk value is %u", unk);
    _player->GetSocial()->SendSocialList();
}

void WorldSession::HandleAddFriendOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 1+1);

    sLog.outDebug("WORLD: Received CMSG_ADD_FRIEND");

    std::string friendName = GetHellgroundString(LANG_FRIEND_IGNORE_UNKNOWN);
    std::string friendNote;

    recv_data >> friendName;

    // recheck
    CHECK_PACKET_SIZE(recv_data, (friendName.size()+1)+1);

    recv_data >> friendNote;

    if (!normalizePlayerName(friendName))
        return;

    RealmDataDatabase.escape_string(friendName);            // prevent SQL injection - normal name don't must changed by this call

    sLog.outDebug("WORLD: %s asked to add friend : '%s'",
        GetPlayer()->GetName(), friendName.c_str());

    RealmDataDatabase.AsyncPQuery(&WorldSession::HandleAddFriendOpcodeCallBack, GetAccountId(), friendNote, "SELECT guid, race, account FROM characters WHERE name = '%s'", friendName.c_str());
}

void WorldSession::HandleAddFriendOpcodeCallBack(QueryResultAutoPtr result, uint32 accountId, std::string friendNote)
{
    uint64 friendGuid;
    uint64 friendAcctid;
    uint32 team;
    FriendsResult friendResult;

    WorldSession * session = sWorld.FindSession(accountId);

    if (!session || !session->GetPlayer())
        return;

    friendResult = FRIEND_NOT_FOUND;
    friendGuid = 0;

    if (result)
    {
        friendGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        team = Player::TeamForRace((*result)[1].GetUInt8());
        friendAcctid = (*result)[2].GetUInt32();

        if (session->HasPermissions(PERM_GMT_HDEV) || sWorld.getConfig(CONFIG_ALLOW_GM_FRIEND) || !AccountMgr::HasPermissions(friendAcctid, PERM_GMT))
            if (friendGuid)
            {
                bool CAN = true;
                if (sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION) && session->GetPlayer()->InBattleGroundOrArena())
                {
                    if (team != session->GetPlayer()->TeamForRace(session->GetPlayer()->GetRace()))
                        CAN = false;
                }

                if (friendGuid==session->GetPlayer()->GetGUID())
                    friendResult = FRIEND_SELF;
                else if ((session->GetPlayer()->GetTeam() != team || !CAN) && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND) && !session->HasPermissions(PERM_GMT_HDEV))
                    friendResult = FRIEND_ENEMY;
                else if (session->GetPlayer()->GetSocial()->HasFriend(GUID_LOPART(friendGuid)))
                    friendResult = FRIEND_ALREADY;
                else
                {                    
                    Player* pFriend = ObjectAccessor::GetPlayerInWorld(friendGuid);
                    
                    if (pFriend && pFriend->IsInWorld() && pFriend->IsVisibleGloballyfor(session->GetPlayer()) && !pFriend->IsHidden())
                        friendResult = FRIEND_ADDED_ONLINE;
                    else // when adding friend - it should not list new friend as online - as it may cause loss of private information about if i am online or not. lol
                    // however this should be announced when other player accepts friendship and this is online
                    // for now return old friend-add.
                        friendResult = FRIEND_ADDED_OFFLINE;
                    if (!session->GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(friendGuid), false))
                    {
                        friendResult = FRIEND_LIST_FULL;
                        sLog.outDebug("WORLD: %s's friend list is full.", session->GetPlayer()->GetName());
                    }
                }
                session->GetPlayer()->GetSocial()->SetFriendNote(GUID_LOPART(friendGuid), friendNote);
            }
    }

    sSocialMgr.SendFriendStatus(session->GetPlayer(), friendResult, GUID_LOPART(friendGuid), false);

    sLog.outDebug("WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelFriendOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    uint64 FriendGUID;

    sLog.outDebug("WORLD: Received CMSG_DEL_FRIEND");

    recv_data >> FriendGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(FriendGUID), false);

    sSocialMgr.SendFriendStatus(GetPlayer(), FRIEND_REMOVED, GUID_LOPART(FriendGUID), false);

    sLog.outDebug("WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleAddIgnoreOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    sLog.outDebug("WORLD: Received CMSG_ADD_IGNORE");

    std::string IgnoreName = GetHellgroundString(LANG_FRIEND_IGNORE_UNKNOWN);

    recv_data >> IgnoreName;

    if (!normalizePlayerName(IgnoreName))
        return;

    RealmDataDatabase.escape_string(IgnoreName);            // prevent SQL injection - normal name don't must changed by this call

    sLog.outDebug("WORLD: %s asked to Ignore: '%s'",
        GetPlayer()->GetName(), IgnoreName.c_str());

    RealmDataDatabase.AsyncPQuery(&WorldSession::HandleAddIgnoreOpcodeCallBack, GetAccountId(), "SELECT guid FROM characters WHERE name = '%s'", IgnoreName.c_str());
}

void WorldSession::HandleAddIgnoreOpcodeCallBack(QueryResultAutoPtr result, uint32 accountId)
{
    uint64 IgnoreGuid;
    FriendsResult ignoreResult;

    WorldSession * session = sWorld.FindSession(accountId);

    if (!session || !session->GetPlayer())
        return;

    ignoreResult = FRIEND_IGNORE_NOT_FOUND;
    IgnoreGuid = 0;

    if (result)
    {
        IgnoreGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);

        if (IgnoreGuid)
        {
            Player * tmp = ObjectAccessor::GetPlayerInWorldOrNot(IgnoreGuid);
            if (!tmp || !tmp->GetSession()->HasPermissions(PERM_GMT_HDEV))  // add only players
            {
                if (IgnoreGuid==session->GetPlayer()->GetGUID())            // not add yourself
                    ignoreResult = FRIEND_IGNORE_SELF;
                else if (session->GetPlayer()->GetSocial()->HasIgnore(GUID_LOPART(IgnoreGuid)))
                    ignoreResult = FRIEND_IGNORE_ALREADY;
                else
                {
                    ignoreResult = FRIEND_IGNORE_ADDED;

                    // ignore list full
                    if (!session->GetPlayer()->GetSocial()->AddToSocialList(GUID_LOPART(IgnoreGuid), true))
                        ignoreResult = FRIEND_IGNORE_FULL;
                }
            }
        }
    }

    sSocialMgr.SendFriendStatus(session->GetPlayer(), ignoreResult, GUID_LOPART(IgnoreGuid), false);

    sLog.outDebug("WORLD: Sent (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleDelIgnoreOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    uint64 IgnoreGUID;

    sLog.outDebug("WORLD: Received CMSG_DEL_IGNORE");

    recv_data >> IgnoreGUID;

    _player->GetSocial()->RemoveFromSocialList(GUID_LOPART(IgnoreGUID), true);

    sSocialMgr.SendFriendStatus(GetPlayer(), FRIEND_IGNORE_REMOVED, GUID_LOPART(IgnoreGUID), false);

    sLog.outDebug("WORLD: Sent motd (SMSG_FRIEND_STATUS)");
}

void WorldSession::HandleSetFriendNoteOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+1);
    uint64 guid;
    std::string note;
    recv_data >> guid >> note;
    _player->GetSocial()->SetFriendNote(guid, note);
}

void WorldSession::HandleBugOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4+1+4+1);

    uint32 suggestion, contentlen;
    std::string content;
    uint32 typelen;
    std::string type;

    recv_data >> suggestion >> contentlen >> content;

    //recheck
    CHECK_PACKET_SIZE(recv_data,4+4+(content.size()+1)+4+1);

    recv_data >> typelen >> type;

    if (suggestion == 0)
        sLog.outDebug("WORLD: Received CMSG_BUG [Bug Report]");
    else
        sLog.outDebug("WORLD: Received CMSG_BUG [Suggestion]");

    sLog.outDebug("%s", type.c_str());
    sLog.outDebug("%s", content.c_str());

    RealmDataDatabase.escape_string(type);
    RealmDataDatabase.escape_string(content);
    RealmDataDatabase.PExecute ("INSERT INTO bugreport (type,content) VALUES('%s', '%s')", type.c_str(), content.c_str());
}

void WorldSession::HandleCorpseReclaimOpcode(WorldPacket &recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDetail("WORLD: Received CMSG_RECLAIM_CORPSE");
    if (GetPlayer()->isAlive())
        return;

    // do not allow corpse reclaim in arena
    if (GetPlayer()->InArena())
        return;

    // body not released yet
    if (!GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        return;

    Corpse *corpse = GetPlayer()->GetCorpse();

    if (!corpse)
        return;

    // prevent resurrect before 30-sec delay after body release not finished
    //if (corpse->GetGhostTime() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType()==CORPSE_RESURRECTABLE_PVP) > time(NULL))
    if (GetPlayer()->GetDeathTimer() + GetPlayer()->GetCorpseReclaimDelay(corpse->GetType()==CORPSE_RESURRECTABLE_PVP) > time(NULL))
        return;

    float dist = corpse->GetDistance(GetPlayer());
    sLog.outDebug("Corpse 3D Distance: \t%f",dist);
    if (dist > CORPSE_RECLAIM_RADIUS)
        return;

    uint64 guid;
    recv_data >> guid;

    // resurrect
    GetPlayer()->ResurrectPlayer(GetPlayer()->InBattleGroundOrArena() ? 1.0f : 0.5f);

    // spawn bones
    GetPlayer()->SpawnCorpseBones();
}

void WorldSession::HandleResurrectResponseOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+1);

    sLog.outDetail("WORLD: Received CMSG_RESURRECT_RESPONSE");

    if (GetPlayer()->isAlive())
        return;

    uint64 guid;
    uint8 status;
    recv_data >> guid;
    recv_data >> status;

    if (status == 0)
    {
        GetPlayer()->clearResurrectRequestData();           // reject
        return;
    }

    if (!GetPlayer()->isRessurectRequestedBy(guid))
        return;

    GetPlayer()->ResurectUsingRequestData();
}

void WorldSession::HandleAreaTriggerOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4);

    sLog.outDebug("WORLD: Received CMSG_AREATRIGGER");

    uint32 Trigger_ID;

    recv_data >> Trigger_ID;
    sLog.outDebug("Trigger ID:%u",Trigger_ID);

    if (GetPlayer()->IsTaxiFlying())
    {
        sLog.outDebug("Player '%s' (GUID: %u) in flight, ignore Area Trigger ID:%u",GetPlayer()->GetName(),GetPlayer()->GetGUIDLow(), Trigger_ID);
        return;
    }

    AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(Trigger_ID);
    if (!atEntry)
    {
        sLog.outDebug("Player '%s' (GUID: %u) send unknown (by DBC) Area Trigger ID:%u",GetPlayer()->GetName(),GetPlayer()->GetGUIDLow(), Trigger_ID);
        return;
    }

    if (GetPlayer()->GetMapId()!=atEntry->mapid)
    {
        sLog.outDebug("Player '%s' (GUID: %u) too far (trigger map: %u player map: %u), ignore Area Trigger ID: %u", GetPlayer()->GetName(), atEntry->mapid, GetPlayer()->GetMapId(), GetPlayer()->GetGUIDLow(), Trigger_ID);
        return;
    }

    // delta is safe radius
    const float delta = 5.0f;
    // check if player in the range of areatrigger
    Player* pl = GetPlayer();

    pl->SendCombatStats(1 << COMBAT_STATS_AREA_TRIGGER, "CMSG_AREATRIGGER player pos %f %f %f at %u (%f %f %f %f|%f %f %f %f)", NULL,
        pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), Trigger_ID, atEntry->x, atEntry->y, atEntry->z, atEntry->radius,
        atEntry->box_orientation, atEntry->box_x, atEntry->box_y, atEntry->box_z);

    if (atEntry->radius > 0)
    {
        // if we have radius check it
        float dist = pl->GetDistance(atEntry->x,atEntry->y,atEntry->z);
        if (dist > atEntry->radius + delta)
        {
            sLog.outDebug("Player '%s' (GUID: %u) too far (radius: %f distance: %f), ignore Area Trigger ID: %u",
                pl->GetName(), pl->GetGUIDLow(), atEntry->radius, dist, Trigger_ID);
            return;
        }
    }
    else
    {
        // we have only extent
        float dx = pl->GetPositionX() - atEntry->x;
        float dy = pl->GetPositionY() - atEntry->y;
        float dz = pl->GetPositionZ() - atEntry->z;
        double es = sin(atEntry->box_orientation);
        double ec = cos(atEntry->box_orientation);
        // calc rotated vector based on extent axis
        double rotateDx = dx*ec + dy*es;
        double rotateDy = dx*es - dy*ec;

        if ((fabs(rotateDx) > atEntry->box_x/2 + delta) ||
            (fabs(rotateDy) > atEntry->box_y/2 + delta) ||
            (fabs(dz) > atEntry->box_z/2 + delta))
        {
            sLog.outDebug("Player '%s' (GUID: %u) too far (1/2 box X: %f 1/2 box Y: %f 1/2 box Z: %f rotate dX: %f rotate dY: %f dZ:%f), ignore Area Trigger ID: %u",
                pl->GetName(), pl->GetGUIDLow(), atEntry->box_x/2, atEntry->box_y/2, atEntry->box_z/2, rotateDx, rotateDy, dz, Trigger_ID);
            return;
        }
    }

    if (sScriptMgr.OnAreaTrigger(GetPlayer(), atEntry))
        return;

    uint32 quest_id = sObjectMgr.GetQuestForAreaTrigger(Trigger_ID);
    if (quest_id && GetPlayer()->isAlive() && GetPlayer()->IsActiveQuest(quest_id))
    {
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);
        if (pQuest)
        {
            if (GetPlayer()->GetQuestStatus(quest_id) == QUEST_STATUS_INCOMPLETE)
                GetPlayer()->AreaExploredOrEventHappens(quest_id);
        }
    }

    if (sObjectMgr.IsTavernAreaTrigger(Trigger_ID))
    {
        // set resting flag we are in the inn
        GetPlayer()->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        GetPlayer()->InnEnter(time(NULL), atEntry->mapid, atEntry->x, atEntry->y, atEntry->z);
        GetPlayer()->SetRestType(REST_TYPE_IN_TAVERN);
        return;
    }

    if (GetPlayer()->InBattleGroundOrArena())
    {
        BattleGround* bg = GetPlayer()->GetBattleGround();
        if (bg)
            if (bg->GetStatus() == STATUS_IN_PROGRESS)
                bg->HandleAreaTrigger(GetPlayer(), Trigger_ID);

        return;
    }

    if (OutdoorPvP * pvp = GetPlayer()->GetOutdoorPvP())
    {
        if (pvp->HandleAreaTrigger(_player, Trigger_ID))
            return;
    }

    // NULL if all values default (non teleport trigger)
    AreaTrigger const* at = sObjectMgr.GetAreaTrigger(Trigger_ID);
    if (!at)
        return;

    if (!GetPlayer()->Satisfy(sObjectMgr.GetAccessRequirement(at->access_id), at->target_mapId, true))
        return;

    GetPlayer()->TeleportTo(at->target_mapId,at->target_X,at->target_Y,at->target_Z,at->target_Orientation,TELE_TO_NOT_LEAVE_TRANSPORT);
}

void WorldSession::HandleUpdateAccountData(WorldPacket &/*recv_data*/)
{
    sLog.outDetail("WORLD: Received CMSG_UPDATE_ACCOUNT_DATA");
    //recv_data.hexlike();
}

void WorldSession::HandleRequestAccountData(WorldPacket& /*recv_data*/)
{
    sLog.outDetail("WORLD: Received CMSG_REQUEST_ACCOUNT_DATA");
    //recv_data.hexlike();
}

void WorldSession::HandleSetActionButtonOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1+2+1+1);

    sLog.outDebug( "WORLD: Received CMSG_SET_ACTION_BUTTON");
    uint8 button, misc, type;
    uint16 action;
    recv_data >> button >> action >> misc >> type;
    sLog.outDetail("BUTTON: %u ACTION: %u TYPE: %u MISC: %u", button, action, type, misc);
    if (action==0)
    {
        sLog.outDetail("MISC: Remove action from button %u", button);

        GetPlayer()->removeActionButton(button);
    }
    else
    {
        if (type==ACTION_BUTTON_MACRO || type==ACTION_BUTTON_CMACRO)
        {
            sLog.outDetail("MISC: Added Macro %u into button %u", action, button);
            GetPlayer()->addActionButton(button,action,type,misc);
        }
        else if (type==ACTION_BUTTON_SPELL)
        {
            sLog.outDetail("MISC: Added Action %u into button %u", action, button);
            GetPlayer()->addActionButton(button,action,type,misc);
        }
        else if (type==ACTION_BUTTON_ITEM)
        {
            sLog.outDetail("MISC: Added Item %u into button %u", action, button);
            GetPlayer()->addActionButton(button,action,type,misc);
        }
        else
            sLog.outLog(LOG_DEFAULT, "ERROR: MISC: Unknown action button type %u for action %u into button %u", type, action, button);
    }
}

void WorldSession::HandleCompleteCinema(WorldPacket & recv_data)
{
    sLog.outDebug("WORLD: Received CMSG_COMPLETE_CINEMATIC");

    uint32 Cinematic_ID = GetPlayer()->getWatchingCinematic();

    CinematicSequencesEntry const* cinematic = sCinematicSequencesStore.LookupEntry(Cinematic_ID);

    if (!cinematic)
    {
        sLog.outDebug("Player '%s' (GUID: %u) send unknown (by DBC) Cinematic Sequence ID:%u",GetPlayer()->GetName(),GetPlayer()->GetGUIDLow(), Cinematic_ID);
        return;
    }

    GetPlayer()->setWatchingCinematic(0);
    GetPlayer()->GetCamera().UpdateVisibilityForOwner();

	// join LFG
	// if you enable Cinematics, uncomment code in HandleCompleteCinema()

	if (GetPlayer()->GetSession()->IsAccountFlagged(ACC_LANG_CHOOSED) && !GetPlayer()->IsPlayerCustomFlagged(PL_CUSTOM_INV_TO_GLOBAL))
	{
		// join LFG
		// if you enable Cinematics, uncomment code in HandleCompleteCinema()

		bool is_russian = GetPlayer()->GetSession()->isRussian();

		if (ChannelMgr* cMgr = channelMgr(GetPlayer()->GetTeam()))
		{
			Channel *chn = nullptr;

			//if (is_russian)
			//    chn = cMgr->GetJoinChannel("Russian", 0);
			//else
			//    chn = cMgr->GetJoinChannel("English", 0);

			chn = cMgr->GetJoinChannel("Global", 0);

			if (chn) {
				chn->Invite(GetPlayer()->GetGUID(), GetPlayer()->GetName(), false);
			}
		}

		// don't check anything, just add flag
		GetPlayer()->AddPlayerCustomFlag(PL_CUSTOM_INV_TO_GLOBAL);
	}

    if (sScriptMgr.OnCompletedCinematic(GetPlayer(), cinematic))
        return;
}

void WorldSession::HandleNextCinematicCamera(WorldPacket & recv_data)
{
    debug_log("WORLD: Which movie to play");
}

void WorldSession::HandleMoveTimeSkippedOpcode(WorldPacket & /*recv_data*/)
{
    /*  WorldSession::Update(WorldTimer::getMSTime());*/
    debug_log("WORLD: Time Lag/Synchronization Resent/Update");

    /*
        CHECK_PACKET_SIZE(recv_data,8+4);
        uint64 guid;
        uint32 time_skipped;
        recv_data >> guid;
        recv_data >> time_skipped;
        sLog.outDebug("WORLD: CMSG_MOVE_TIME_SKIPPED");

        /// TODO
        must be need use in Trinity
        We substract server Lags to move time (AntiLags)
        for exmaple
        GetPlayer()->ModifyLastMoveTime(-int32(time_skipped));
    */
}

void WorldSession::HandleFeatherFallAck(WorldPacket &/*recv_data*/)
{
    debug_log("WORLD: CMSG_MOVE_FEATHER_FALL_ACK");
}

void WorldSession::HandleMoveUnRootAck(WorldPacket&/* recv_data*/)
{
    /*
        CHECK_PACKET_SIZE(recv_data,8+8+4+4+4+4+4);

        sLog.outDebug("WORLD: CMSG_FORCE_MOVE_UNROOT_ACK");
        recv_data.hexlike();
        uint64 guid;
        uint64 unknown1;
        uint32 unknown2;
        float PositionX;
        float PositionY;
        float PositionZ;
        float Orientation;

        recv_data >> guid;
        recv_data >> unknown1;
        recv_data >> unknown2;
        recv_data >> PositionX;
        recv_data >> PositionY;
        recv_data >> PositionZ;
        recv_data >> Orientation;

        // TODO for later may be we can use for anticheat
        debug_log("Guid " I64FMTD,guid);
        debug_log("unknown1 " I64FMTD,unknown1);
        debug_log("unknown2 %u",unknown2);
        debug_log("X %f",PositionX);
        debug_log("Y %f",PositionY);
        debug_log("Z %f",PositionZ);
        debug_log("O %f",Orientation);
    */
}

void WorldSession::HandleMoveRootAck(WorldPacket&/* recv_data*/)
{
    /*
        CHECK_PACKET_SIZE(recv_data,8+8+4+4+4+4+4);

        sLog.outDebug("WORLD: CMSG_FORCE_MOVE_ROOT_ACK");
        recv_data.hexlike();
        uint64 guid;
        uint64 unknown1;
        uint32 unknown2;
        float PositionX;
        float PositionY;
        float PositionZ;
        float Orientation;

        recv_data >> guid;
        recv_data >> unknown1;
        recv_data >> unknown2;
        recv_data >> PositionX;
        recv_data >> PositionY;
        recv_data >> PositionZ;
        recv_data >> Orientation;

        // for later may be we can use for anticheat
        debug_log("Guid " I64FMTD,guid);
        debug_log("unknown1 " I64FMTD,unknown1);
        debug_log("unknown1 %u",unknown2);
        debug_log("X %f",PositionX);
        debug_log("Y %f",PositionY);
        debug_log("Z %f",PositionZ);
        debug_log("O %f",Orientation);
    */
}

void WorldSession::HandleSetActionBar(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    uint8 ActionBar;

    recv_data >> ActionBar;

    if (!GetPlayer())                                        // ignore until not logged (check needed because STATUS_AUTHED)
    {
        if (ActionBar!=0)
            sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleSetActionBar in not logged state with value: %u, ignored",uint32(ActionBar));
        return;
    }

    GetPlayer()->SetByteValue(PLAYER_FIELD_BYTES, 2, ActionBar);
}

void WorldSession::HandlePlayedTime(WorldPacket& /*recv_data*/)
{
    uint32 TotalTimePlayed = GetPlayer()->GetTotalPlayedTime();
    uint32 LevelPlayedTime = GetPlayer()->GetLevelPlayedTime();

    WorldPacket data(SMSG_PLAYED_TIME, 8);
    data << TotalTimePlayed;
    data << LevelPlayedTime;
    SendPacket(&data);
}

void WorldSession::HandleInspectOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    uint64 guid;
    recv_data >> guid;
    debug_log("Inspected guid is %llu", guid);

    //_player->SetSelection(guid);

    Player *plr = sObjectMgr.GetPlayerInWorld(guid);
    if (!plr)                                                // wrong player
        return;

    bool Casted = false;
    for (uint8 i = 0; i < 3; ++i)
    {
        Item* item = plr->GetItemByPos(255, EQUIPMENT_SLOT_MAINHAND + i);
        if (!item)
            continue;

        if (!plr->GetTransmogManager()->GetActiveTransEntry(i, plr->GetTransmogManager()->GetWeaponTypeFromSubclass(item->GetProto()->SubClass, i, item->GetProto()->Class), true))
            continue;

        if (item && item->GetEntry() != plr->GetUInt32Value(PLAYER_VISIBLE_ITEM_16_0 + i * 16))
        {
            if (!Casted)
            {
                int32 RandomBP = rand()%999999;
                plr->CastCustomSpell(plr, 55148, &RandomBP, 0, 0, true); // 55148 forces update values at transmogged indexes
                GetPlayer()->CastCustomSpell(plr, 55149, &RandomBP, 0, 0, true);
                Casted = true;
            }
            plr->ForceValuesUpdateAtIndex(PLAYER_VISIBLE_ITEM_16_0 + i * 16);
        }
    }

    uint32 talent_points = 0x3D;
    uint32 guid_size = plr->GetPackGUID().size();
    WorldPacket data(SMSG_INSPECT_TALENT, guid_size+4+talent_points);
    data << plr->GetPackGUID();
    data << uint32(talent_points);

    // fill by 0 talents array
    for (uint32 i = 0; i < talent_points; ++i)
        data << uint8(0);

    if (sWorld.getConfig(CONFIG_TALENTS_INSPECTING) || _player->isGameMaster())
    {
        // find class talent tabs (all players have 3 talent tabs)
        uint32 const* talentTabIds = GetTalentTabPages(plr->GetClass());

        uint32 talentTabPos = 0;                            // pos of first talent rank in tab including all prev tabs
        for (uint32 i = 0; i < 3; ++i)
        {
            uint32 talentTabId = talentTabIds[i];

            // fill by real data
            for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
            {
                TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);
                if (!talentInfo)
                    continue;

                // skip another tab talents
                if (talentInfo->TalentTab != talentTabId)
                    continue;

                // find talent rank
                uint32 curtalent_maxrank = 0;
                for (uint32 k = 5; k > 0; --k)
                {
                    if (talentInfo->RankID[k-1] && 
                        /*SpellMgr::GetHighestSpellRankForPlayer(talentInfo->RankID[k-1], plr) From the time this function ALWAYS returns highest rank of spell without checking if player knows or not this spellid - BUT IT WAS THE "FIX"
                        FOR FAERIE FIRE VISUAL (BUT IT WASN'T FIXING ANYTHING! CAUSE IT'S INSCEPT BY OTHER PLAYER - NOT YOUR OWN TALENTS, AND FAERIE FIRE IS ONLY BUGGED IN YOUR OWN TALENTS!) IN TALENT TAB*/
                        plr->HasSpell(talentInfo->RankID[k-1]))
                    {
                        curtalent_maxrank = k;
                        break;
                    }
                }

                // now check for 1 rank talents but multiple spell ranks (like faerie fire feral)
                /*if (!curtalent_maxrank && SpellMgr::GetHighestSpellRankForPlayer(talentInfo->RankID[0], plr))
                    curtalent_maxrank = 1;*/ // This is useless

                // not learned talent
                if (!curtalent_maxrank)
                    continue;

                // 1 rank talent bit index
                uint32 curtalent_index = talentTabPos + GetTalentInspectBitPosInTab(talentId);

                uint32 curtalent_rank_index = curtalent_index+curtalent_maxrank-1;

                // slot/offset in 7-bit bytes
                uint32 curtalent_rank_slot7   = curtalent_rank_index / 7;
                uint32 curtalent_rank_offset7 = curtalent_rank_index % 7;

                // rank pos with skipped 8 bit
                uint32 curtalent_rank_index2 = curtalent_rank_slot7 * 8 + curtalent_rank_offset7;

                // slot/offset in 8-bit bytes with skipped high bit
                uint32 curtalent_rank_slot = curtalent_rank_index2 / 8;
                uint32 curtalent_rank_offset =  curtalent_rank_index2 % 8;

                // apply mask
                uint32 val = data.read<uint8>(guid_size + 4 + curtalent_rank_slot);
                val |= (1 << curtalent_rank_offset);
                data.put<uint8>(guid_size + 4 + curtalent_rank_slot, val & 0xFF);
            }

            talentTabPos += GetTalentTabInspectBitSize(talentTabId);
        }
    }

    SendPacket(&data);
}

void WorldSession::HandleInspectHonorStatsOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    uint64 guid;
    recv_data >> guid;

    Player *player = sObjectMgr.GetPlayerInWorld(guid);

    if (!player)
    {
        //sLog.outLog(LOG_DEFAULT, "ERROR: InspectHonorStats: WTF, player %llu not found...",guid);
        // no reason to log, this is caused by inspecting 'ghosts' of players that recently logged off
        return;
    }

    WorldPacket data(MSG_INSPECT_HONOR_STATS, 8+1+4*4);
    data << uint64(player->GetGUID());
    data << uint8(player->GetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_KILLS));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION));
    data << uint32(player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
    SendPacket(&data);
}

void WorldSession::HandleWorldTeleportOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4+4+4+4+4);

    // write in client console: worldport 469 452 6454 2536 180 or /console worldport 469 452 6454 2536 180
    // Received opcode CMSG_WORLD_TELEPORT
    // Time is ***, map=469, x=452.000000, y=6454.000000, z=2536.000000, orient=3.141593

    //sLog.outDebug("Received opcode CMSG_WORLD_TELEPORT");

    if (GetPlayer()->IsTaxiFlying())
    {
        sLog.outDebug("Player '%s' (GUID: %u) in flight, ignore worldport command.",GetPlayer()->GetName(),GetPlayer()->GetGUIDLow());
        return;
    }

    uint32 time;
    uint32 mapid;
    float PositionX;
    float PositionY;
    float PositionZ;
    float Orientation;

    recv_data >> time;                                      // time in m.sec.
    recv_data >> mapid;
    recv_data >> PositionX;
    recv_data >> PositionY;
    recv_data >> PositionZ;
    recv_data >> Orientation;                               // o (3.141593 = 180 degrees)
    debug_log("Time %u sec, map=%u, x=%f, y=%f, z=%f, orient=%f", time/1000, mapid, PositionX, PositionY, PositionZ, Orientation);

    if (HasPermissions(PERM_ADM))
        GetPlayer()->TeleportTo(mapid, PositionX, PositionY, PositionZ, Orientation);
    else
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
    sLog.outDebug("Received worldport command from player %s", GetPlayer()->GetName());
}

void WorldSession::HandleWhoisOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 1);

    sLog.outDebug("Received opcode CMSG_WHOIS");
    std::string charname;
    recv_data >> charname;

    if (!HasPermissions(PERM_ADM))
    {
        SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
        return;
    }

    if (charname.empty() || !normalizePlayerName (charname))
    {
        SendNotification(LANG_NEED_CHARACTER_NAME);
        return;
    }

    Player *plr = sObjectMgr.GetPlayerInWorld(charname.c_str());

    if (!plr)
    {
        SendNotification(LANG_PLAYER_NOT_EXIST_OR_OFFLINE, charname.c_str());
        return;
    }

    uint32 accid = plr->GetSession()->GetAccountId();

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT username, email, last_ip FROM account WHERE account_id = '%u'", accid);
    if (!result)
    {
        SendNotification(LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND, charname.c_str());
        return;
    }

    Field *fields = result->Fetch();
    std::string acc = fields[0].GetCppString();
    if (acc.empty())
        acc = "Unknown";
    std::string email = fields[1].GetCppString();
    if (email.empty())
        email = "Unknown";
    std::string lastip = fields[2].GetCppString();
    if (lastip.empty())
        lastip = "Unknown";

    std::string msg = charname + "'s " + "account is " + acc + ", e-mail: " + email + ", last ip: " + lastip;

    WorldPacket data(SMSG_WHOIS, msg.size()+1);
    data << msg;
    _player->SendPacketToSelf(&data);

    sLog.outDebug("Received whois command from player %s for character %s", GetPlayer()->GetName(), charname.c_str());
}

void WorldSession::HandleReportSpamOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 1+8);
    sLog.outDebug("WORLD: CMSG_REPORT_SPAM");
    recv_data.hexlike();

    uint8 spam_type;                                        // 0 - mail, 1 - chat
    uint64 spammer_guid;
    uint32 unk1 = 0;
    uint32 unk2 = 0;
    uint32 unk3 = 0;
    uint32 unk4 = 0;
    std::string description = "";
    recv_data >> spam_type;                                 // unk 0x01 const, may be spam type (mail/chat)
    recv_data >> spammer_guid;                              // player guid
    switch (spam_type)
    {
        case 0:
            CHECK_PACKET_SIZE(recv_data, recv_data.rpos()+4+4+4);
            recv_data >> unk1;                              // const 0
            recv_data >> unk2;                              // probably mail id
            recv_data >> unk3;                              // const 0
            break;
        case 1:
            CHECK_PACKET_SIZE(recv_data, recv_data.rpos()+4+4+4+4+1);
            recv_data >> unk1;                              // probably language
            recv_data >> unk2;                              // message type?
            recv_data >> unk3;                              // probably channel id
            recv_data >> unk4;                              // unk random value
            recv_data >> description;                       // spam description string (messagetype, channel name, player name, message)
            break;
    }

    // NOTE: all chat messages from this spammer automatically ignored by spam reporter until logout in case chat spam.
    // if it's mail spam - ALL mails from this spammer automatically removed by client

    // Complaint Received message
    WorldPacket data(SMSG_COMPLAIN_RESULT, 1);
    data << uint8(0);
    SendPacket(&data);

    sLog.outDebug("REPORT SPAM: type %u, guid %u, unk1 %u, unk2 %u, unk3 %u, unk4 %u, message %s", spam_type, GUID_LOPART(spammer_guid), unk1, unk2, unk3, unk4, description.c_str());
}

void WorldSession::HandleRealmStateRequestOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("CMSG_REALM_SPLIT");

    uint32 unk;
    std::string split_date = "01/01/01";
    recv_data >> unk;

    WorldPacket data(SMSG_REALM_SPLIT, 4+4+split_date.size()+1);
    data << unk;
    data << uint32(0x00000000);                             // realm split state
    // split states:
    // 0x0 realm normal
    // 0x1 realm split
    // 0x2 realm split pending
    data << split_date;
    SendPacket(&data);
    //sLog.outDebug("response sent %u", unk);
}

void WorldSession::HandleFarSightOpcode(WorldPacket & recv_data)
{
    uint8 apply;
    recv_data >> apply;

    WorldObject* obj = _player->GetFarsightTarget();
    if (!obj)
        return;

    switch (apply)
    {
        case 0:
            _player->GetCamera().ResetView(false);
            break;
        case 1:
            _player->GetCamera().SetView(obj, false);
            if (_player->GetPet()) // reinitialize pet bar
                _player->PetSpellInitialize();
            break;
    }
}

void WorldSession::HandleChooseTitleOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("CMSG_SET_TITLE");

    int32 title;
    recv_data >> title;

    // -1 at none
    if (title > 0 && title < 128)
    {
       if (!GetPlayer()->HasTitle(title))
            return;
    }
    else
        title = 0;

    GetPlayer()->SetUInt32Value(PLAYER_CHOSEN_TITLE, title);
}

void WorldSession::HandleTimeSyncResp(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4+4);

    sLog.outDebug("CMSG_TIME_SYNC_RESP");

    uint32 counter, clientTicks;
    recv_data >> counter >> clientTicks;

    if (counter != _player->m_timeSyncCounter - 1)
        sLog.outDebug("Wrong time sync counter from player %s (cheater?)", _player->GetName());

    sLog.outDebug("Time sync received: counter %u, client ticks %u, time since last sync %u", counter, clientTicks, clientTicks - _player->m_timeSyncClient);

    uint32 ourTicks = clientTicks + (WorldTimer::getMSTime() - _player->m_timeSyncServer);

    // diff should be small
    sLog.outDebug("Our ticks: %u, diff %u, latency %u", ourTicks, ourTicks - clientTicks, GetLatency());

    _player->m_timeSyncClient = clientTicks;
}

void WorldSession::HandleResetInstancesOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: CMSG_RESET_INSTANCES");
    if (Group *pGroup = _player->GetGroup())
    {
        if (!pGroup->IsLeader(_player->GetGUID()))
            return;

        std::list<GroupMemberSlot> memberSlotList = pGroup->GetMemberSlots();
        for (std::list<GroupMemberSlot>::const_iterator citr = memberSlotList.begin(); citr != memberSlotList.end(); ++citr)
        {
            if (Player *pl = sObjectMgr.GetPlayerInWorld(citr->guid))
            {
                const MapEntry *mapEntry = sMapStore.LookupEntry(pl->GetMapId());
                if (mapEntry->IsDungeon() || mapEntry->IsRaid())
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleResetInstancesOpcode: player %d tried to reset instances while player %d inside raid instance!", _player->GetGUIDLow(), pl->GetGUIDLow());
                    _player->SendResetInstanceFailed(0, pl->GetMapId());
                    return;
                }
            }
            else
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleResetInstancesOpcode: player %d tried to reset instances while player %llu offline!", _player->GetGUIDLow(), citr->guid);
                //_player->SendResetInstanceFailed(0, /* mapid pl ktorego nie ma ;] */);
                return;
            }
        }

        pGroup->ResetInstances(INSTANCE_RESET_ALL, _player);
    }
    else
        _player->ResetInstances(INSTANCE_RESET_ALL);
}

void WorldSession::HandleDungeonDifficultyOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("MSG_SET_DUNGEON_DIFFICULTY");

    uint32 mode;
    recv_data >> mode;

    if (mode == _player->GetDifficulty())
        return;

    if (mode > DIFFICULTY_HEROIC)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleDungeonDifficultyOpcode: player %d sent an invalid instance mode %d!", _player->GetGUIDLow(), mode);
        return;
    }

    // cannot reset while in an instance
    Map *map = _player->GetMap();
    if (map && map->IsDungeon())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleDungeonDifficultyOpcode: player %d tried to reset the instance while inside!", _player->GetGUIDLow());
        return;
    }

    if (_player->GetLevel() < LEVELREQUIREMENT_HEROIC)
        return;
    Group *pGroup = _player->GetGroup();
    if (pGroup)
    {
        /*
        if (pGroup->isRaidGroup())
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleDungeonDifficultyOpcode: player %d tried to change difficulty while in raid group!", _player->GetGUIDLow());
            ChatHandler(this).SendSysMessage(LANG_CHANGE_DIFFICULTY_RAID);
            return;
        }
        */

        std::list<GroupMemberSlot> memberSlotList = pGroup->GetMemberSlots();

        for (std::list<GroupMemberSlot>::const_iterator citr = memberSlotList.begin(); citr != memberSlotList.end(); ++citr)
        {
            Player * pl = sObjectMgr.GetPlayerInWorld(citr->guid);
            if (pl)
            {
                const MapEntry *mapEntry = sMapStore.LookupEntry(pl->GetMapId());
                if (mapEntry->IsDungeon())
                {
                    sLog.outDebug("ERROR: WorldSession::HandleDungeonDifficultyOpcode: player %d tried to change difficulty while player %d inside the instance!", _player->GetGUIDLow(), pl->GetGUIDLow());
                    ChatHandler(this).SendSysMessage(LANG_CHANGE_DIFFICULTY_INSIDE);
                    return;
                }
            }
            else
            {
                sLog.outDebug("ERROR: WorldSession::HandleDungeonDifficultyOpcode: player %d tried to change difficulty while player %llu offline!", _player->GetGUIDLow(), citr->guid);
                ChatHandler(this).SendSysMessage(LANG_CHANGE_DIFFICULTY_OFFLINE);
                return;
            }
        }

        if (pGroup->IsLeader(_player->GetGUID()))
        {
            // the difficulty is set even if the instances can't be reset
            //_player->SendDungeonDifficulty(true);
            pGroup->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY, _player);
            pGroup->SetDifficulty(mode);
        }
    }
    else
    {
        _player->ResetInstances(INSTANCE_RESET_CHANGE_DIFFICULTY);
        _player->SetDifficulty(mode);
    }
}

void WorldSession::HandleNewUnknownOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("New Unknown Opcode %u", recv_data.GetOpcode());
    recv_data.hexlike();
    /*
    New Unknown Opcode 837
    STORAGE_SIZE: 60
    02 00 00 00 00 00 00 00 | 00 00 00 00 01 20 00 00
    89 EB 33 01 71 5C 24 C4 | 15 03 35 45 74 47 8B 42
    BA B8 1B 40 00 00 00 00 | 00 00 00 00 77 66 42 BF
    23 91 26 3F 00 00 60 41 | 00 00 00 00

    New Unknown Opcode 837
    STORAGE_SIZE: 44
    02 00 00 00 00 00 00 00 | 00 00 00 00 00 00 80 00
    7B 80 34 01 84 EA 2B C4 | 5F A1 36 45 C9 39 1C 42
    BA B8 1B 40 CE 06 00 00 | 00 00 80 3F
    */
}

void WorldSession::HandleDismountOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: CMSG_CANCEL_MOUNT_AURA");
    //recv_data.hexlike();

    //If player is not mounted, so go out :)
    if (!_player->IsMounted())                              // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_CHAR_NON_MOUNTED);
        return;
    }

    if (_player->IsTaxiFlying())                               // not blizz like; no any messages on blizz
    {
        ChatHandler(this).SendSysMessage(LANG_YOU_IN_FLIGHT);
        return;
    }

    _player->Unmount();
    // _player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED); // copy of this is just below - we need to know aura id that we are removing
    Unit::AuraList const& mountAuras = _player->GetAurasByType(SPELL_AURA_MOUNTED);
        
    Unit::AuraList::const_iterator iter, next;
    int32 mountToRestore = 0;
    for (iter = mountAuras.begin(); iter != mountAuras.end(); iter = next)
    {
        next = iter;
        ++next;

        if (*iter)
        {
            mountToRestore = (*iter)->GetId();
            _player->RemoveAurasDueToSpell((*iter)->GetId());
            if (!mountAuras.empty())
                next = mountAuras.begin();
            else
                break;
        }
    }
    if (mountToRestore)
        _player->CastCustomSpell(_player, 55181, &mountToRestore, 0, 0, true); // 55181 - mount savior trigger dummy aura
}

void WorldSession::HandleMoveFlyModeChangeAckOpcode(WorldPacket & recv_data)
{
    // fly mode on/off
    sLog.outDebug("WORLD: CMSG_MOVE_SET_CAN_FLY_ACK");
    //recv_data.hexlike();
    ObjectGuid guid;
    MovementInfo movementInfo;

    recv_data >> guid;                                      // guid
    recv_data.read_skip<uint32>();                          // unk
    recv_data >> movementInfo;
    recv_data.read_skip<uint32>();                          // unk2

    if (guid == _player->GetGUID())
        _player->SetUnitMovementFlags(movementInfo.GetMovementFlags());
}

void WorldSession::HandleRequestPetInfoOpcode(WorldPacket & /*recv_data */)
{
    /*
        sLog.outDebug("WORLD: CMSG_REQUEST_PET_INFO");
        recv_data.hexlike();
    */
}

void WorldSession::HandleSetTaxiBenchmarkOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 1);

    uint8 mode;
    recv_data >> mode;

    sLog.outDebug("Client used \"/timetest %d\" command", mode);
}

// Refer-A-Friend
void WorldSession::HandleGrantLevel(WorldPacket& recv_data)
{
    debug_log("WORLD: CMSG_GRANT_LEVEL");

    ObjectGuid guid;
    recv_data >> guid.ReadAsPacked();

    if (!guid.IsPlayer())
        return;

    Player * target = sObjectMgr.GetPlayerInWorld(guid);

    // cheating and other check
    ReferAFriendError err = _player->GetReferFriendError(target, false);

    if (err)
    {
        _player->SendReferFriendError(err, target);
        return;
    }

    target->AccessGrantableLevel(_player->GetObjectGuid());

    WorldPacket data(SMSG_PROPOSE_LEVEL_GRANT, 8);
    data << _player->GetPackGUID();
    target->GetSession()->SendPacket(&data);

    ChatHandler(this).PSendSysMessage(LANG_YOU_OFFERED_GRANT_LEVEL, target->GetName());
}

void WorldSession::HandleAcceptGrantLevel(WorldPacket& recv_data)
{
    debug_log("WORLD: CMSG_ACCEPT_LEVEL_GRANT");

    ObjectGuid guid;
    recv_data >> guid.ReadAsPacked();

    if (!guid.IsPlayer())
        return;

    if (!_player->IsAccessGrantableLevel(guid))
        return;

    _player->AccessGrantableLevel(ObjectGuid()); // empty ObjectGuid
    Player * grant_giver = sObjectMgr.GetPlayerInWorld(guid);

    if (!grant_giver)
        return;

    if (grant_giver->GetGrantableLevels())
        grant_giver->ChangeGrantableLevels(0);
    else
        return;

    sLog.outLog(LOG_RAF, "Player %s (guid %u) accepted a level from %s (guid %u)",
        _player->GetName(), _player->GetGUIDLow(), grant_giver->GetName(), grant_giver->GetGUIDLow());

    _player->GiveLevel(_player->GetLevel() + 1);

    _player->LevelReached(_player->GetLevel()); // level already changed, use current
}
