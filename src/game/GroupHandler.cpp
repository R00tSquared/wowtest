// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2009 TrinityCore <http://www.trinitycore.org/>
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
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "SocialMgr.h"
#include "Util.h"
#include "ChatLog.h"
#include "Chat.h"
#include "Language.h"

/* differeces from off:
    -you can uninvite yourself - is is useful
    -you can accept invitation even if leader went offline
*/
/* todo:
    -group_destroyed msg is sent but not shown
    -reduce xp gaining when in raid group
    -quest sharing has to be corrected
    -FIX sending PartyMemberStats
*/

void WorldSession::SendPartyResult(PartyOperation operation, const std::string& member, PartyResult res)
{
    WorldPacket data(SMSG_PARTY_COMMAND_RESULT, (8+member.size()+1));
    data << (uint32)operation;
    data << member;
    data << (uint32)res;

    SendPacket(&data);
}

void WorldSession::HandleGroupInviteOpcode(WorldPacket & recv_data)
{
    std::string membername;
    recv_data >> membername;

    // attempt add selected player

    // cheating
    if (!normalizePlayerName(membername))
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_CANT_FIND_TARGET);
        return;
    }

    Player *player = sObjectMgr.GetPlayerInWorld(membername.c_str());

    // no player
    if (!player)
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_CANT_FIND_TARGET);
        return;
    }

    // restrict invite to GMs
    if (!sWorld.getConfig(CONFIG_ALLOW_GM_GROUP) && !GetPlayer()->isGameMaster() && player->isGameMaster())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_CANT_FIND_TARGET);
        return;
    }
    // can't group with
    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) && GetPlayer()->GetTeam() != player->GetTeam())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_UNFRIENDLY);
        return;
    }

    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) && (sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION) && (GetPlayer()->InBattleGroundOrArena() || player->InBattleGroundOrArena())))
    {
        if (GetPlayer()->TeamForRace(GetPlayer()->GetRace()) != player->TeamForRace(player->GetRace()))
        {
            SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_UNFRIENDLY);
            return;
        }
    }

    if (GetPlayer()->GetInstanciableInstanceId() != 0 && player->GetInstanciableInstanceId() != 0 && GetPlayer()->GetInstanciableInstanceId() != player->GetInstanciableInstanceId() && GetPlayer()->GetMapId() == player->GetMapId())
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_NOT_IN_YOUR_INSTANCE);
        return;
    }
    if (sWorld.getConfig(CONFIG_FFA_DISALLOWGROUP) && (player->IsFFAPvP() || GetPlayer()->IsFFAPvP())){
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_IGNORE_YOU);
        return;
    }
    // just ignore us
    if (player->GetInstanciableInstanceId() != 0 && player->GetDifficulty() != GetPlayer()->GetDifficulty() || player->InBattleGroundOrArena() || GetPlayer()->m_HomebindTimer ||
        (player->m_HomebindTimer && /*if not in same instance - forbid*/
        !(GetPlayer()->GetInstanciableInstanceId() != 0 && player->GetInstanciableInstanceId() != 0 && GetPlayer()->GetInstanciableInstanceId() == player->GetInstanciableInstanceId() && GetPlayer()->GetMapId() == player->GetMapId())))
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_IGNORE_YOU);
        return;
    }

    // OK result but not send invite
    if (player->GetSocial()->HasIgnore(GetPlayer()->GetGUIDLow()))
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_IGNORE_YOU);
        return;
    }

    Group *group = GetPlayer()->GetGroup();
    if (group && group->isBGGroup())
        group = GetPlayer()->GetOriginalGroup();

    Group *group2 = player->GetGroup();
    if (group2 && group2->isBGGroup())
        group2 = player->GetOriginalGroup();

    // player already in another group or invited
    if (group2 || player->GetGroupInvite() || player->bot_FakeGroup)
    {
        SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_ALREADY_IN_GROUP);
        return;
    }

    if (group)
    {
        // not have permissions for invite
        if (!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
        {
            SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_YOU_NOT_LEADER);
            return;
        }
        // not have place
        if (group->IsFull())
        {
            SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_PARTY_FULL);
            return;
        }

        if (group->isRaidGroup() && player->IsInGurubashiEvent())
        {
            SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_TARGET_IGNORE_YOU);
            ChatHandler(GetPlayer()).SendSysMessage(LANG_EVENT_GURUBASHI_NO_RAID);
            return;
        }
    }

    // ok, but group not exist, start a new group
    // but don't create and save the group to the DB until
    // at least one person joins
    if (!group)
    {
        group = new Group;
        // new group: if can't add then delete
        if (!group->AddLeaderInvite(GetPlayer()))
        {
            delete group;
            return;
        }
        if (!group->AddInvite(player))
        {
            delete group;
            return;
        }
    }
    else
    {
        // already existed group: if can't add then just leave
        if (!group->AddInvite(player))
        {
            return;
        }
    }

    // ok, we do it
    WorldPacket data(SMSG_GROUP_INVITE, 10);                // guess size
    data << GetPlayer()->GetName();
    player->SendPacketToSelf(&data);

    SendPartyResult(PARTY_OP_INVITE, membername, PARTY_RESULT_OK);
}

void WorldSession::HandleGroupAcceptOpcode(WorldPacket & /*recv_data*/)
{
    Group *group = GetPlayer()->GetGroupInvite();
    if (!group) return;

    if (group->GetLeaderGUID() == GetPlayer()->GetGUID())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: HandleGroupAcceptOpcode: player %s(%d) tried to accept an invite to his own group", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        return;
    }

    // remove in from ivites in any case
    group->RemoveInvite(GetPlayer());

    /** error handling **/
    /********************/

    // not have place
    if (group->IsFull())
    {
        SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_PARTY_FULL);
        return;
    }

    Player* leader = sObjectMgr.GetPlayerInWorld(group->GetLeaderGUID());

    // forming a new group, create it
    if (!group->IsCreated())
    {
        if (leader)
        {
            group->RemoveInvite(leader);

            // When there is a Rogue in party, instance capacity is increased by 1.
            //ChatHandler(leader).PSendSysMessage(15482);
        }

        group->Create(group->GetLeaderGUID(), group->GetLeaderName());
        sObjectMgr.AddGroup(group);
    }

    // everything's fine, do it
    if (!group->AddMember(GetPlayer()->GetGUID(), GetPlayer()->GetName()))
        return;

    uint8 subgroup = group->GetMemberGroup(GetPlayer()->GetGUID());

    GetPlayer()->SetGroup(group, subgroup);

    group->BroadcastGroupUpdate();
}

void WorldSession::HandleGroupDeclineOpcode(WorldPacket & /*recv_data*/)
{
    Group  *group  = GetPlayer()->GetGroupInvite();
    if (!group)
        return;

    Player *leader = sObjectMgr.GetPlayerInWorld(group->GetLeaderGUID());

    GetPlayer()->UninviteFromGroup();

    /** error handling **/
    if (!leader || !leader->GetSession())
        return;
    /********************/

    WorldPacket data(SMSG_GROUP_DECLINE, 10);             // guess size
    data << GetPlayer()->GetName();
    leader->SendPacketToSelf(&data);
}

void WorldSession::HandleGroupUninviteGuidOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    //can't uninvite yourself
    if (guid == GetPlayer()->GetGUID())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleGroupUninviteGuidOpcode: leader %s(%d) tried to uninvite himself from the group.", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        return;
    }

    PartyResult res = GetPlayer()->CanUninviteFromGroup();
    if (res != PARTY_RESULT_OK)
    {
        SendPartyResult(PARTY_OP_LEAVE, "", res);
        return;
    }

    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
        return;

    if (grp->IsMember(guid))
    {
        if (grp->IsAssistant(GetPlayer()->GetGUID()))
        {
            if (grp->IsLeader(guid) || grp->IsAssistant(guid)) // an assistant cannot kick leader or another assistant
            {
                SendPartyResult(PARTY_OP_LEAVE, "", PARTY_RESULT_YOU_NOT_LEADER);
                return;
            }

            if (sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_ASSIST_KICK))
            {
                if (uint32 cooldown = sWorld.GetAssistKickCooldown(GetPlayer()->GetGUIDLow()))
                {
                    std::string timeStr = secondsToTimeString(cooldown);
                    SendNotification(GetHellgroundString(LANG_RAID_ASSIST_CANT_KICK_WAIT), timeStr.c_str());
                    ChatHandler(GetPlayer()).PSendSysMessage(LANG_RAID_ASSIST_CANT_KICK_WAIT, timeStr.c_str());
                    return;
                }
            }
        }

        Player* plr = sObjectAccessor.GetPlayerInWorldOrNot(guid);
        if (plr && !plr->IsInWorld())
        {
            SendPartyResult(PARTY_OP_LEAVE, plr->GetName(), PARTY_RESULT_CANT_FIND_TARGET);
            return;
        }

        sWorld.AssistKickUsed(GetPlayer()->GetGUIDLow());
		GetPlayer()->LogKicked(guid);
        Player::RemoveFromGroup(grp,guid);
        return;
    }

    if (Player* plr = grp->GetInvited(guid))
    {
        plr->UninviteFromGroup();
        return;
    }

    SendPartyResult(PARTY_OP_LEAVE, "", PARTY_RESULT_NOT_IN_YOUR_PARTY);
}

void WorldSession::HandleGroupUninviteNameOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    std::string membername;
    recv_data >> membername;

    // player not found
    if (!normalizePlayerName(membername))
        return;

    // can't uninvite yourself
    if (GetPlayer()->GetName() == membername)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleGroupUninviteNameOpcode: leader %s(%d) tried to uninvite himself from the group.", GetPlayer()->GetName(), GetPlayer()->GetGUIDLow());
        return;
    }

    PartyResult res = GetPlayer()->CanUninviteFromGroup();
    if (res != PARTY_RESULT_OK)
    {
        SendPartyResult(PARTY_OP_LEAVE, "", res);
        return;
    }

    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
        return;

    if (uint64 guid = grp->GetMemberGUID(membername))
    {
        if (grp->IsAssistant(GetPlayer()->GetGUID()))
        {
            if (grp->IsLeader(guid) || grp->IsAssistant(guid)) // an assistant cannot kick leader or another assistant
            {
                SendPartyResult(PARTY_OP_LEAVE, "", PARTY_RESULT_YOU_NOT_LEADER);
                return;
            }

            if (sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_ASSIST_KICK))
            {
                if (uint32 cooldown = sWorld.GetAssistKickCooldown(GetPlayer()->GetGUIDLow()))
                {
                    std::string timeStr = secondsToTimeString(cooldown);
                    SendNotification(GetHellgroundString(LANG_RAID_ASSIST_CANT_KICK_WAIT), timeStr.c_str());
                    ChatHandler(GetPlayer()).PSendSysMessage(LANG_RAID_ASSIST_CANT_KICK_WAIT, timeStr.c_str());
                    return;
                }
            }
        }

        Player* plr = sObjectAccessor.GetPlayerInWorldOrNot(guid);
        if (plr && !plr->IsInWorld())
        {
            SendPartyResult(PARTY_OP_LEAVE, membername, PARTY_RESULT_CANT_FIND_TARGET);
            return;
        }

        sWorld.AssistKickUsed(GetPlayer()->GetGUIDLow());
		GetPlayer()->LogKicked(guid);
        Player::RemoveFromGroup(grp,guid);
        return;
    }

    if (Player* plr = grp->GetInvited(membername))
    {
        plr->UninviteFromGroup();
        return;
    }

    SendPartyResult(PARTY_OP_LEAVE, membername, PARTY_RESULT_NOT_IN_YOUR_PARTY);
}

void WorldSession::HandleGroupSetLeaderOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    uint64 guid;
    recv_data >> guid;

    Player *player = sObjectMgr.GetPlayerInWorld(guid);

    /** error handling **/
    if (!player || !group->IsLeader(GetPlayer()->GetGUID()) || player->GetGroup() != group)
        return;
    /********************/

    // everything's fine, do it
    group->ChangeLeader(guid);
}

void WorldSession::HandleGroupLeaveOpcode(WorldPacket & /*recv_data*/)
{
    if (!GetPlayer()->GetGroup())
        return;

    if (_player->InBattleGroundOrArena())
    {
        SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_INVITE_RESTRICTED);
        return;
    }

    /** error handling **/
    /********************/

    // everything's fine, do it
    SendPartyResult(PARTY_OP_LEAVE, "", PARTY_RESULT_OK);

	GetPlayer()->LogKicked(GetPlayer()->GetGUID());
    GetPlayer()->RemoveFromGroup();
}

void WorldSession::HandleLootMethodOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+8+4);

    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (GetPlayer()->InBattleGroundOrArena())
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()))
        return;

    uint32 lootMethod;
    uint64 lootMaster;
    uint32 lootThreshold;
    recv_data >> lootMethod >> lootMaster >> lootThreshold;

    // everything's fine, do it
    group->SetLootMethod((LootMethod)lootMethod);
    group->SetLooterGuid(lootMaster);
    group->SetLootThreshold((ItemQualities)lootThreshold);
    group->SendUpdate();

	GetPlayer()->LogLootMethod((LootMethod)lootMethod);
}

void WorldSession::HandleLootRoll(WorldPacket &recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4+1);

    if (!GetPlayer()->GetGroup())
        return;

    uint64 Guid;
    uint32 NumberOfPlayers;
    uint8  Choice;
    recv_data >> Guid;                                      //guid of the item rolled
    recv_data >> NumberOfPlayers;
    recv_data >> Choice;                                    //0: pass, 1: need, 2: greed

    //sLog.outDebug("WORLD RECIEVE CMSG_LOOT_ROLL, From:%u, Numberofplayers:%u, Choice:%u", (uint32)Guid, NumberOfPlayers, Choice);

    // everything's fine, do it
    sObjectMgr.CountRollVote(GetPlayer()->GetGUID(), Guid, Choice);
}

void WorldSession::HandleMinimapPingOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4);

    if (!GetPlayer()->GetGroup())
        return;

    float x, y;
    recv_data >> x;
    recv_data >> y;

    //sLog.outDebug("Received opcode MSG_MINIMAP_PING X: %f, Y: %f", x, y);

    /** error handling **/
    /********************/

    // everything's fine, do it
    WorldPacket data(MSG_MINIMAP_PING, (8+4+4));
    data << GetPlayer()->GetGUID();
    data << x;
    data << y;
    GetPlayer()->GetGroup()->BroadcastPacket(&data, false, -1, GetPlayer()->GetGUID());
}

void WorldSession::HandleRandomRollOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4);

    uint32 minimum, maximum, roll;
    recv_data >> minimum;
    recv_data >> maximum;

    /** error handling **/
    if (minimum > maximum || maximum > 10000)                // < 32768 for urand call
        return;
    /********************/

    // everything's fine, do it
    roll = urand(minimum, maximum);

    //sLog.outDebug("ROLL: MIN: %u, MAX: %u, ROLL: %u", minimum, maximum, roll);

    WorldPacket data(MSG_RANDOM_ROLL, 4+4+4+8);
    data << minimum;
    data << maximum;
    data << roll;
    data << GetPlayer()->GetGUID();
    if (GetPlayer()->GetGroup())
    {
        GetPlayer()->GetGroup()->BroadcastPacket(&data, false);
		GetPlayer()->LogRoll(minimum, maximum, roll, 1);
    }
    else
        SendPacket(&data);
}

void WorldSession::HandleRaidIconTargetOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    uint8  x;
    recv_data >> x;

    /** error handling **/
    /********************/

    // everything's fine, do it
    if (x == 0xFF)                                           // target icon request
    {
        group->SendTargetIconList(this);
    }
    else                                                    // target icon update
    {
        // recheck
        CHECK_PACKET_SIZE(recv_data,1+8);

        if (!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
            return;

        uint64 guid;
        recv_data >> guid;
        group->SetTargetIcon(x, guid);
    }
}

void WorldSession::HandleRaidConvertOpcode(WorldPacket & /*recv_data*/)
{
    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (_player->InBattleGroundOrArena())
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetGUID()) || group->GetMembersCount() < 2)
        return;
    /********************/

    // everything's fine, do it (is it 0 (PARTY_OP_INVITE) correct code)
    SendPartyResult(PARTY_OP_INVITE, "", PARTY_RESULT_OK);
    group->SetDifficulty(DIFFICULTY_NORMAL);
    group->ConvertToRaid();

    ChatHandler(this).SendSysMessage(GetHellgroundString(16641));
}

void WorldSession::HandleGroupChangeSubGroupOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1+1);

    // we will get correct pointer for group here, so we don't have to check if group is BG raid
    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    std::string name;
    uint8 groupNr;
    recv_data >> name;

    // recheck
    CHECK_PACKET_SIZE(recv_data,(name.size()+1)+1);

    recv_data >> groupNr;

    if (groupNr >= MAX_RAID_SUBGROUPS)
        return;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
        return;

    if (!group->HasFreeSlotSubGroup(groupNr))
        return;
    /********************/

    // everything's fine, do it
    if (Player* player = sObjectMgr.GetPlayerInWorld(name.c_str()))
        group->ChangeMembersGroup(GetPlayer(), player, groupNr);
    else if (uint64 guid = sObjectMgr.GetPlayerGUIDByName(name.c_str()))
        group->ChangeMembersGroup(GetPlayer(), guid, groupNr);


}

void WorldSession::HandleGroupAssistantOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+1);

    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    uint64 guid;
    uint8 flag;
    recv_data >> guid;
    recv_data >> flag;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    group->SetAssistant(guid, flag == 0 ? false : true);
}

void WorldSession::HandleGroupPromoteOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 1+1+8);

    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    uint8 mainAssistant, mainTank;
    uint64 guid;
    recv_data >> mainAssistant >> mainTank;
    recv_data >> guid;

    /** error handling **/
    if (!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
        return;
    /********************/

    // everything's fine, do it
    if (mainAssistant)
        group->SetMainAssistant(guid);
    else
        group->SetMainTank(guid);
}

void WorldSession::HandleRaidReadyCheckOpcode(WorldPacket & recv_data)
{
    Group *group = GetPlayer()->GetGroup();
    if (!group)
        return;

    // request
    if (recv_data.empty())
    {
        if (!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
            return;

        if (GetPlayer()->InBattleGroundOrArena())
        {
            ChatHandler(GetPlayer()).SendSysMessage(12740);
            return;
        }

        // everything's fine, do it
        WorldPacket data(MSG_RAID_READY_CHECK, 8);
        data << GetPlayer()->GetGUID();
        group->BroadcastPacket(&data, -1);

        group->OfflineReadyCheck();

        // Send info about if this raid is considered guild-raid or not.
        Map* map = GetPlayer()->GetMap();
        if (map && map->IsDungeon())
        {
            if (((InstanceMap*)map)->Calculate_GBK_Guild())
                ChatHandler(this).SendSysMessage(GetHellgroundString(LANG_GBK_INSTANCE_RAID_GUILD));
            else
                ChatHandler(this).SendSysMessage(GetHellgroundString(LANG_GBK_INSTANCE_RAID_NOT_GUILD));
        }
    }
    else                                                    // answer
    {
        uint8 state;
        recv_data >> state;

        // everything's fine, do it
        WorldPacket data(MSG_RAID_READY_CHECK_CONFIRM, 9);
        data << GetPlayer()->GetGUID();
        data << state;
        group->BroadcastReadyCheck(&data);
    }
}

void WorldSession::HandleRaidReadyCheckFinishOpcode(WorldPacket & recv_data)
{
    //Group* group = GetPlayer()->GetGroup();
    //if(!group)
    //    return;

    //if(!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()))
    //    return;

    // Is any reaction need?
}

void WorldSession::BuildPartyMemberStatsChangedPacket(Player *player, WorldPacket *data)
{
    uint32 mask = player->GetGroupUpdateFlag();

    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)                // if update power type, update current/max power also
        mask |= (GROUP_UPDATE_FLAG_CUR_POWER | GROUP_UPDATE_FLAG_MAX_POWER);

    if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)            // same for pets
        mask |= (GROUP_UPDATE_FLAG_PET_CUR_POWER | GROUP_UPDATE_FLAG_PET_MAX_POWER);

    uint32 byteCount = 0;
    for (int i = 1; i < GROUP_UPDATE_FLAGS_COUNT; ++i)
        if (mask & (1 << i))
            byteCount += GroupUpdateLength[i];

    data->Initialize(SMSG_PARTY_MEMBER_STATS, 8 + 4 + byteCount);
    *data << player->GetPackGUID();
    *data << uint32(mask);

    if (mask & GROUP_UPDATE_FLAG_STATUS)
    {
        if (player)
        {
            if (player->IsPvP())
                *data << (uint16) (MEMBER_STATUS_ONLINE | MEMBER_STATUS_PVP);
            else
                *data << (uint16) MEMBER_STATUS_ONLINE;
        }
        else
            *data << (uint16) MEMBER_STATUS_OFFLINE;
    }

    if (mask & GROUP_UPDATE_FLAG_CUR_HP)
        *data << (uint16) player->GetHealth();

    if (mask & GROUP_UPDATE_FLAG_MAX_HP)
        *data << (uint16) player->GetMaxHealth();

    Powers powerType = player->getPowerType();
    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)
        *data << (uint8) powerType;

    if (mask & GROUP_UPDATE_FLAG_CUR_POWER)
        *data << (uint16) player->GetPower(powerType);

    if (mask & GROUP_UPDATE_FLAG_MAX_POWER)
        *data << (uint16) player->GetMaxPower(powerType);

    if (mask & GROUP_UPDATE_FLAG_LEVEL)
        *data << (uint16) player->GetLevel();

    if (mask & GROUP_UPDATE_FLAG_ZONE)
        *data << (uint16) player->GetCachedZone();

    if (mask & GROUP_UPDATE_FLAG_POSITION)
        *data << (uint16) player->GetPositionX() << (uint16) player->GetPositionY();

    if (mask & GROUP_UPDATE_FLAG_AURAS)
    {
        uint64 auramask = player->GetAuraUpdateMask();
        *data << uint64(auramask);
        for (uint32 i = 0; i < MAX_AURAS; ++i)
        {
            if (auramask & (uint64(1) << i))
            {
                uint32 updatedAura=player->GetUInt32Value(uint16(UNIT_FIELD_AURA + i));
                *data << uint16(updatedAura);
                *data << uint8(1);
            }
        }
    }

    Pet *pet = player->GetPet();
    if (mask & GROUP_UPDATE_FLAG_PET_GUID)
    {
        if (pet)
            *data << (uint64) pet->GetGUID();
        else
            *data << (uint64) 0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_NAME)
    {
        if (pet)
            *data << pet->GetName();
        else
            *data << (uint8)  0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MODEL_ID)
    {
        if (pet)
            *data << (uint16) pet->GetDisplayId();
        else
            *data << (uint16) 0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_CUR_HP)
    {
        if (pet)
            *data << (uint16) pet->GetHealth();
        else
            *data << (uint16) 0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MAX_HP)
    {
        if (pet)
            *data << (uint16) pet->GetMaxHealth();
        else
            *data << (uint16) 0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)
    {
        if (pet)
            *data << (uint8)  pet->getPowerType();
        else
            *data << (uint8)  0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_CUR_POWER)
    {
        if (pet)
            *data << (uint16) pet->GetPower(pet->getPowerType());
        else
            *data << (uint16) 0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MAX_POWER)
    {
        if (pet)
            *data << (uint16) pet->GetMaxPower(pet->getPowerType());
        else
            *data << (uint16) 0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_AURAS)
    {
        if (pet)
        {
            uint64 auramask = pet->GetAuraUpdateMask();
            *data << uint64(auramask);
            for (uint32 i = 0; i < MAX_AURAS; ++i)
            {
                if (auramask & (uint64(1) << i))
                {
                    uint32 updatedAura=pet->GetUInt32Value(uint16(UNIT_FIELD_AURA + i));
                    *data << uint16(updatedAura);
                    *data << uint8(1);
                }
            }
        }
        else
            *data << (uint64) 0;
    }
}

/*this procedure handles clients CMSG_REQUEST_PARTY_MEMBER_STATS request*/
void WorldSession::HandleRequestPartyMemberStatsOpcode(WorldPacket &recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    sLog.outDebug("WORLD: Received CMSG_REQUEST_PARTY_MEMBER_STATS");
    uint64 Guid;
    recv_data >> Guid;

    Player *player = ObjectAccessor::GetPlayerInWorldOrNot(Guid);
    if (!player)
    {
        WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 3+4+2);
        data.appendPackGUID(Guid);
        data << (uint32) GROUP_UPDATE_FLAG_STATUS;
        data << (uint16) MEMBER_STATUS_OFFLINE;
        SendPacket(&data);
        return;
    }

    Pet *pet = player->GetPet();

    WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 4+2+2+2+1+2*6+8+1+8);
    data << player->GetPackGUID();

    uint32 mask1 = 0x00040BFF;                              // common mask, real flags used 0x000040BFF
    if (pet)
        mask1 = 0x7FFFFFFF;                                 // for hunters and other classes with pets

    uint16 online_status = isRAFConnectedWith(player->GetSession()) ? (MEMBER_STATUS_ONLINE | MEMBER_STATUS_RAF) : MEMBER_STATUS_ONLINE;

    Powers powerType = player->getPowerType();
    data << (uint32) mask1;                                 // group update mask
    data << (uint16) online_status;                         // member's online status
    data << (uint16) player->GetHealth();                   // GROUP_UPDATE_FLAG_CUR_HP
    data << (uint16) player->GetMaxHealth();                // GROUP_UPDATE_FLAG_MAX_HP
    data << (uint8)  powerType;                             // GROUP_UPDATE_FLAG_POWER_TYPE
    data << (uint16) player->GetPower(powerType);           // GROUP_UPDATE_FLAG_CUR_POWER
    data << (uint16) player->GetMaxPower(powerType);        // GROUP_UPDATE_FLAG_MAX_POWER
    data << (uint16) player->GetLevel();                    // GROUP_UPDATE_FLAG_LEVEL
    data << (uint16) player->GetCachedZone();               // GROUP_UPDATE_FLAG_ZONE
    data << (uint16) player->GetPositionX();                // GROUP_UPDATE_FLAG_POSITION
    data << (uint16) player->GetPositionY();                // GROUP_UPDATE_FLAG_POSITION

    uint64 auramask = 0;
    size_t maskPos = data.wpos();
    data << (uint64) auramask;                              // placeholder
    for (uint8 i = 0; i < MAX_AURAS; ++i)
    {
        if (uint32 aura = player->GetUInt32Value(UNIT_FIELD_AURA + i))
        {
            auramask |= (uint64(1) << i);
            data << uint16(aura);
            data << uint8(1);
        }
    }
    data.put<uint64>(maskPos,auramask);                     // GROUP_UPDATE_FLAG_AURAS

    if (pet)
    {
        Powers petpowertype = pet->getPowerType();
        data << (uint64) pet->GetGUID();                    // GROUP_UPDATE_FLAG_PET_GUID
        data << pet->GetName();                             // GROUP_UPDATE_FLAG_PET_NAME
        data << (uint16) pet->GetDisplayId();               // GROUP_UPDATE_FLAG_PET_MODEL_ID
        data << (uint16) pet->GetHealth();                  // GROUP_UPDATE_FLAG_PET_CUR_HP
        data << (uint16) pet->GetMaxHealth();               // GROUP_UPDATE_FLAG_PET_MAX_HP
        data << (uint8)  petpowertype;                      // GROUP_UPDATE_FLAG_PET_POWER_TYPE
        data << (uint16) pet->GetPower(petpowertype);       // GROUP_UPDATE_FLAG_PET_CUR_POWER
        data << (uint16) pet->GetMaxPower(petpowertype);    // GROUP_UPDATE_FLAG_PET_MAX_POWER

        uint64 petauramask = 0;
        size_t petMaskPos = data.wpos();
        data << (uint64) petauramask;                       // placeholder
        for (uint8 i = 0; i < MAX_AURAS; ++i)
        {
            if (uint32 petaura = pet->GetUInt32Value(UNIT_FIELD_AURA + i))
            {
                petauramask |= (uint64(1) << i);
                data << (uint16) petaura;
                data << (uint8)  1;
            }
        }
        data.put<uint64>(petMaskPos,petauramask);           // GROUP_UPDATE_FLAG_PET_AURAS
    }
    else
    {
        data << (uint8)  0;                                 // GROUP_UPDATE_FLAG_PET_NAME
        data << (uint64) 0;                                 // GROUP_UPDATE_FLAG_PET_AURAS
    }

    SendPacket(&data);
}

/*!*/void WorldSession::HandleRequestRaidInfoOpcode(WorldPacket & /*recv_data*/)
{
    // every time the player checks the character screen
    _player->SendRaidInfo();
}

/*void WorldSession::HandleGroupCancelOpcode(WorldPacket & recv_data)
{
    sLog.outDebug("WORLD: got CMSG_GROUP_CANCEL.");
}*/

void WorldSession::HandleGroupPassOnLootOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4);

    sLog.outDebug("WORLD: Received CMSG_GROUP_PASS_ON_LOO");

    uint32 passOnLoot;
    recv_data >> passOnLoot;

    // ignore if player not loaded
    if (!GetPlayer())                                        // needed because STATUS_AUTHED
    {
        return;
    }
    GetPlayer()->SetPassOnGroupLoot(passOnLoot != 0);
}

