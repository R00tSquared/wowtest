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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "SharedDefines.h"
#include "Player.h"
#include "BattleGroundMgr.h"
#include "BattleGroundAV.h"
#include "BattleGroundAB.h"
#include "BattleGroundEY.h"
#include "BattleGroundWS.h"
#include "BattleGroundNA.h"
#include "BattleGroundBE.h"
#include "BattleGroundAA.h"
#include "BattleGroundRL.h"
#include "BattleGround.h"
#include "MapManager.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "ArenaTeam.h"
#include "World.h"
#include "WorldPacket.h"
//#include "ProgressBar.h"
#include "GameEvent.h"
#include "ScriptMgr.h"
#include <unordered_map>

/*********************************************************/
/***            BATTLEGROUND QUEUE SYSTEM              ***/
/*********************************************************/

BattleGroundQueue::BattleGroundQueue()
{
    for (int i = 0; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        queuedPlayersCount[i] = 0;
    }
}

BattleGroundQueue::~BattleGroundQueue()
{
    m_QueuedPlayers.clear();
    for (int i = 0; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (uint32 j = 0; j < BG_QUEUE_GROUP_TYPES_COUNT; ++j)
        {
            for (GroupsQueueType::iterator itr = m_QueuedGroups[i][j].begin(); itr!= m_QueuedGroups[i][j].end(); ++itr)
                delete (*itr);

            m_QueuedGroups[i][j].clear();
        }

        queuedPlayersCount[i] = 0;
    }
}

// selection pool initialization, used to clean up from prev selection
void BattleGroundQueue::SelectionPool::Init()
{
    SelectedGroups.clear();
    PlayerCount = 0;
}

// remove group info from selection pool
// returns true when we need to try to add new group to selection pool
// returns false when selection pool is ok or when we kicked smaller group than we need to kick
// sometimes it can be called on empty selection pool
bool BattleGroundQueue::SelectionPool::KickGroup(uint32 size)
{
    //find maxgroup or LAST group with size == size and kick it
    bool found = false;
    GroupsQueueType::iterator groupToKick = SelectedGroups.begin();
    for (GroupsQueueType::iterator itr = groupToKick; itr != SelectedGroups.end(); ++itr)
    {
        if (abs((int32)((*itr)->Players.size() - size)) <= 1)
        {
            groupToKick = itr;
            found = true;
        }
        else if (!found && (*itr)->Players.size() >= (*groupToKick)->Players.size())
            groupToKick = itr;
    }

    //if pool is empty, do nothing
    if (GetPlayerCount())
    {
        //update player count
        GroupQueueInfo* ginfo = (*groupToKick);
        SelectedGroups.erase(groupToKick);
        PlayerCount -= ginfo->Players.size();
        //return false if we kicked smaller group or there are enough players in selection pool
        if (ginfo->Players.size() <= size + 1)
            return false;
    }
    return true;
}

// add group to selection pool
// used when building selection pools
// returns true if we can invite more players, or when we added group to selection pool
// returns false when selection pool is full
bool BattleGroundQueue::SelectionPool::AddGroup(GroupQueueInfo *ginfo, uint32 free_slots)
{
    //if group is larger than desired count - don't allow to add it to pool
    if (!ginfo->IsInvitedToBGInstanceGUID && free_slots >= PlayerCount + ginfo->Players.size())
    {
        SelectedGroups.push_back(ginfo);
        // increase selected players count
        PlayerCount += ginfo->Players.size();
        return true;
    }

    if (PlayerCount < free_slots)
        return true;
    return false;
}

// add group to bg queue with the given leader and bg specifications
GroupQueueInfo * BattleGroundQueue::AddGroup(Player *leader, BattleGroundTypeId BgTypeId, BattleGroundBracketId bracketId, uint8 ArenaType, bool isRated, bool isEntryPointAnywhere, bool isPremade, uint32 arenaRating, uint32 arenateamid)
{
    // create new ginfo
    // cannot use the method like in addplayer, because that could modify an in-queue group's stats
    // (e.g. leader leaving queue then joining as individual again)
    GroupQueueInfo* ginfo = new GroupQueueInfo;
    ginfo->BgTypeId                  = BgTypeId;
    ginfo->ArenaType                 = ArenaType;
    ginfo->ArenaTeamId               = arenateamid;
    ginfo->IsRated                   = isRated;
    ginfo->IsEntryPointAnywhere      = isEntryPointAnywhere;
    ginfo->IsInvitedToBGInstanceGUID = 0;
    ginfo->JoinTime                  = WorldTimer::getMSTime();
    // solo 3v3 reg randomizer (only Alliance to easy sort) (updated: lol it's make no sense)
    //if (ArenaType == 3 && isRated)
    //    ginfo->Team = ALLIANCE;
    // Battleground change team if needed
    //if (!ArenaType && sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION) && leader->GetDummyAura(54844))
    //    ginfo->Team = leader->GetTeam() == ALLIANCE ? HORDE : ALLIANCE;
    //else
    //    ginfo->Team                      = leader->GetTeam();
    ginfo->Team                      = leader->GetTeam();
    ginfo->ArenaTeamRating           = arenaRating;
    ginfo->OpponentsTeamRating       = 0;

    ginfo->PremadeLeaderGUID         = isPremade ? leader->GetGUIDLow() : 0;
    ginfo->LeaderClassMask           = ArenaType ? leader->GetClassMask() : leader->GetBGClassMask();
    ginfo->LeaderIsSemiHealer        = leader->IsSemiHealer();
    ginfo->LeaderName                = leader->GetName();
    ginfo->CurrentBGId = 0;

    ginfo->Players.clear();

    //compute index (if group is premade or joined a rated match) to queues
    uint32 index = (!isRated && !isPremade) ? BG_QUEUE_NORMAL : BG_QUEUE_PREMADE;

    debug_log("Adding Group to BattleGroundQueue bgTypeId : %u, bracket_id : %u, index : %u", BgTypeId, bracketId, index);

    m_QueuedGroups[bracketId][index].push_back(ginfo);

    // return ginfo, because it is needed to add players to this group info
    return ginfo;
}

//add player to playermap
void BattleGroundQueue::AddPlayer(Player *plr, GroupQueueInfo *ginfo)
{
    IncreaseQueuedPlayersCount(plr->GetBattleGroundBracketIdFromLevel(ginfo->BgTypeId));

    //if player isn't in queue, he is added, if already is, then values are overwritten, no memory leak
    PlayerQueueInfo& info = m_QueuedPlayers[plr->GetGUID()];
    info.InviteTime                 = 0;
    info.LastInviteTime             = 0;
    info.LastOnlineTime             = WorldTimer::getMSTime();
    info.GroupInfo                  = ginfo;
    info.ClassMask                  = ginfo->ArenaType ? plr->GetClassMask() : plr->GetBGClassMask();

    // add the pinfo to ginfo's list
    ginfo->Players[plr->GetGUID()]  = &info;
}

//remove player from queue and from group info, if group info is empty then remove it too
void BattleGroundQueue::RemovePlayer(const uint64& guid, bool decreaseInvitedCount)
{
    //Player *plr = sObjectMgr.GetPlayerInWorld(guid);

    int32 bracket_id = -1;                                     // signed for proper for-loop finish
    QueuedPlayersMap::iterator itr;

    //remove player from map, if he's there
    itr = m_QueuedPlayers.find(guid);
    if (itr == m_QueuedPlayers.end())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGroundQueue: couldn't find player to remove GUID: %u", GUID_LOPART(guid));
        return;
    }

    GroupQueueInfo* group = itr->second.GroupInfo;

    // clean same IP container
    if (group->ArenaType)
    {
        Player* player = sObjectMgr.GetPlayerInWorld(guid);
        if (player)
            Player::RemovArenaQueueHashIP(player);
        else
            Player::RemovArenaQueueHashIP(nullptr, guid);
    }

    GroupsQueueType::iterator group_itr, group_itr_tmp;
    // mostly people with the highest levels are in battlegrounds, thats why
    // we count from MAX_BATTLEGROUND_QUEUES - 1 to 0
    // variable index removes useless searching in other team's queue
    uint32 index = (group->Team == HORDE) ? TEAM_HORDE : TEAM_ALLIANCE;

    // moonwell: not properly implemented for tmp/real queues (i guess)
    for (int32 bracket_id_tmp = MAX_BATTLEGROUND_BRACKETS - 1; bracket_id_tmp >= 0 && bracket_id == -1; --bracket_id_tmp)
    {
        //we must check premade and normal team's queue - because when players from premade are joining bg,
        //they leave groupinfo so we can't use its players size to find out index
        for (uint8 queue_type : {BG_QUEUE_NORMAL, BG_QUEUE_PREMADE})
        {
            for (group_itr_tmp = m_QueuedGroups[bracket_id_tmp][queue_type].begin(); group_itr_tmp != m_QueuedGroups[bracket_id_tmp][queue_type].end(); ++group_itr_tmp)
            {
                if ((*group_itr_tmp) == group)
                {
                    bracket_id = bracket_id_tmp;
                    group_itr = group_itr_tmp;
                    // Store the index to be able to erase the iterator
                    index = queue_type;
                    break;
                }
            }
        }
    }

    //player can't be in queue without group, but just in case
    if (bracket_id == -1)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGroundQueue: ERROR Cannot find groupinfo for player GUID: %u", GUID_LOPART(guid));
        return;
    }
    debug_log("BattleGroundQueue: Removing player GUID %u, from bracket_id %u", GUID_LOPART(guid), (uint32)bracket_id);

    // ALL variables are correctly set
    // We can ignore leveling up in queue - it should not cause crash
    // remove player from group
    // if only one player there, remove group

    if (!group->IsInvitedToBGInstanceGUID)
        DecreaseQueuedPlayersCount(BattleGroundBracketId(bracket_id));

    // remove player queue info from group queue info
    std::map<uint64, PlayerQueueInfo*>::iterator pitr = group->Players.find(guid);
    if (pitr != group->Players.end())
        group->Players.erase(pitr);

    // if invited to bg, and should decrease invited count, then do it
    if (decreaseInvitedCount && group->IsInvitedToBGInstanceGUID)
    {
        BattleGround* bg = sBattleGroundMgr.GetBattleGround(group->IsInvitedToBGInstanceGUID, group->BgTypeId);
        if (bg)
            bg->DecreaseInvitedCount(group->Team);
            
    }

    // remove player queue info
    m_QueuedPlayers.erase(itr);

    // remove group queue info if needed
    if (group->Players.empty())
    {
        m_QueuedGroups[bracket_id][index].erase(group_itr);
        delete group;
    }
    // if group wasn't empty, so it wasn't deleted, and player have left a rated
    // queue -> everyone from the group should leave too
    // don't remove recursively if already invited to bg!
    else if (!group->IsInvitedToBGInstanceGUID && group->IsRated)
    {
        // remove next player, this is recursive
        // first send removal information
        if (Player *plr2 = sObjectMgr.GetPlayerInWorld(group->Players.begin()->first))
        {
            BattleGround * bg = sBattleGroundMgr.GetBattleGroundTemplate(group->BgTypeId);
            BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(group->BgTypeId, group->ArenaType);
            uint32 queueSlot = plr2->GetBattleGroundQueueIndex(bgQueueTypeId);
            plr2->RemoveBattleGroundQueueId(bgQueueTypeId); // must be called this way, because if you move this call to
                                                            // queue->removeplayer, it causes bugs
            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, plr2->GetTeam(), queueSlot, STATUS_NONE, 0, 0);
            plr2->SendPacketToSelf(&data);
        }
        // then actually delete, this may delete the group as well!
        RemovePlayer(group->Players.begin()->first, decreaseInvitedCount);
    }
}

bool BattleGroundQueue::InviteGroupToBG(GroupQueueInfo * ginfo, BattleGround * bg, PlayerTeam side)
{
    // set side if needed
    if (side)
        ginfo->Team = side;

    if (!ginfo->IsInvitedToBGInstanceGUID)
    {
        // not yet invited
        // set invitation
        ginfo->IsInvitedToBGInstanceGUID = bg->GetBgInstanceId();
        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());

        // loop through the players
        for(std::map<uint64,PlayerQueueInfo*>::iterator itr = ginfo->Players.begin(); itr != ginfo->Players.end(); ++itr)
        {
            // set status
            itr->second->InviteTime = WorldTimer::getMSTime();
            itr->second->LastInviteTime = WorldTimer::getMSTime();

            // get the player
            Player* plr = sObjectMgr.GetPlayerInWorld(itr->first);
            // if offline, skip him
            if(!plr)
                continue;

            // invite the player
            sBattleGroundMgr.InvitePlayer(plr, bgQueueTypeId, bg->GetBgInstanceId(), bg->GetTypeID(), ginfo->Team);

            WorldPacket data;

            uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);

			// dangerous because mutex?
			// remove from all queues except current when invited (it's should be like that i guess)
			for (uint8 i = 0; i < 3; ++i)
			{
				if (i == queueSlot)
					continue;
				
				BattleGroundQueueTypeId bgTypeId = plr->GetBattleGroundQueueTypeId(i);
				if (bgTypeId)
				{
					plr->RemoveBattleGroundQueueId(bgTypeId); // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
					sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, side ? side : plr->GetTeam(), i, STATUS_NONE, 0, 0);
					sBattleGroundMgr.m_BattleGroundQueues[bgTypeId].RemovePlayer(plr->GetGUID(), true);
					
					// should we update it?
					//sBattleGroundMgr.m_BattleGroundQueues[bgTypeId].Update(bg->GetTypeID(), plr->GetBattleGroundBracketIdFromLevel(bg->GetTypeID()), ginfo->ArenaType, ginfo->IsRated, ginfo->ArenaTeamRating);
					
					plr->SendPacketToSelf(&data);
				}
			}

            if (bg->isArena()) 
                sLog.outLog(LOG_ARENA, "Arena rated invite player %s (GUID: %u, Team: %u, ArenaType: %u, IP: %s)", plr->GetName(), plr->GetGUIDLow(), ginfo->ArenaTeamId, ginfo->ArenaType, plr->GetSession()->GetRemoteAddress().c_str());
            else
            {
                const char* group_info = (ginfo->Players.size() > 1) ? ginfo->LeaderName : "none";
                sLog.outLog(LOG_BG, "ID %u: Player %s invited to %s, leader %s, jointime %u, classmask %u (%s)", bg->GetBgInstanceId(), plr->GetName(), GetBGTeamName(ginfo->Team), group_info, ginfo->JoinTime, itr->second->ClassMask, bg->GetPlayerCountInfo().c_str());
            }
            
            // send status packet
			sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, side ? side : plr->GetTeam(), queueSlot, STATUS_WAIT_JOIN, bg->isBattleGround() ? INVITE_ACCEPT_WAIT_TIME_BG : INVITE_ACCEPT_WAIT_TIME_ARENA, 0);

            plr->SendPacketToSelf(&data);
        }
        return true;
    }

    return false;
}

// used to remove the Enter Battle window if the battle has already ended, but someone still has it
// (this can happen in arenas mainly, since the preparation is shorter than the timer for the bgqueueremove event
void BattleGroundQueue::BGEndedRemoveInvites(BattleGround *bg)
{
    BattleGroundBracketId bracket_id = bg->GetBracketId();
    uint32 bgInstanceId = bg->GetBgInstanceId();
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
    GroupsQueueType::iterator itr, next;
    for(uint32 i = 0; i < BG_QUEUE_GROUP_TYPES_COUNT; i++)
    {
        itr = m_QueuedGroups[bracket_id][i].begin();
        next = itr;
        while (next != m_QueuedGroups[bracket_id][i].end())
        {
            // must do this way, because the groupinfo will be deleted when all playerinfos are removed
            itr = next;
            ++next;
            GroupQueueInfo * ginfo = (*itr);
            // if group was invited to this bg instance, then remove all references
            if (ginfo->IsInvitedToBGInstanceGUID == bgInstanceId)
            {
                // after removing this much playerinfos, the ginfo will be deleted, so we'll use a for loop
                uint32 to_remove = ginfo->Players.size();
                PlayerTeam team = ginfo->Team;
                for (uint32 j = 0; j < to_remove; j++)
                {
                    // always remove the first one in the group
                    std::map<uint64, PlayerQueueInfo* >::iterator itr2 = ginfo->Players.begin();
                    if (itr2 == ginfo->Players.end())
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Empty Players in ginfo, this should never happen!");
                        return;
                    }
                    // get the player
                    Player* plr = sObjectMgr.GetPlayerInWorld(itr2->first);
                    if (!plr)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Player offline when trying to remove from GroupQueueInfo, this should never happen.");
                        continue;
                    }

                    // get the queueslot
                    uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
                    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES) // player is in queue
                    {
                        plr->RemoveBattleGroundQueueId(bgQueueTypeId);
                        // remove player from queue, this might delete the ginfo as well! don't use that pointer after this!
                        RemovePlayer(itr2->first, true);
                        WorldPacket data;
                        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, team, queueSlot, STATUS_NONE, 0, 0);
                        plr->SendPacketToSelf(&data);
                    }
                }
            }
        }
    }
}

void BattleGroundQueue::FillPlayersToBG(BattleGround* bg, BattleGroundBracketId bracket_id)
{
    // this is used only to invite players
    // distribution is done at porting to bg in GetBestBGTeamOnDistribute()

    uint32 free_slots = bg->GetFreeBGSlots();

    if (free_slots == 0)
        return;

    auto inviteGroups = [&](GroupsQueueType& groups) {
        for (auto itr = groups.begin(); itr != groups.end() && free_slots > 0; ++itr)
        {
            if ((*itr)->IsInvitedToBGInstanceGUID)
                continue;
            
            if (free_slots == 0)
                break;

            if ((*itr)->Players.size() > free_slots)
                continue;

            free_slots -= (*itr)->Players.size();
            m_SelectionPools.AddGroup(*itr, ALLIANCE);
        }
    };

    inviteGroups(m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE]);
    inviteGroups(m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL]);

    // now everything is set, invite players
    for (const auto& group : m_SelectionPools.SelectedGroups)
        InviteGroupToBG(group, bg, group->Team);
}


// this method tries to create battleground or arena with MinPlayersPerTeam against MinPlayersPerTeam
void BattleGroundQueue::TryStartBattleground(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers)
{
    if (sWorld.getConfig(CONFIG_BG_EVENTS_ENABLED) && !sBattleGroundMgr.IsBGEventActive(bgTypeId))
        return; // can only start new battlegrounds for 'active' battlegrounds (except for testing purposes)
    
    uint32 queued = 0;
    
    for (auto queueType : { BG_QUEUE_PREMADE, BG_QUEUE_NORMAL })
    {
        for (auto itr = m_QueuedGroups[bracket_id][queueType].begin(); itr != m_QueuedGroups[bracket_id][queueType].end(); ++itr)
        {
            if ((*itr)->IsInvitedToBGInstanceGUID)
                continue;

            queued += (*itr)->Players.size();
        }
    }

    if (queued < minPlayers * 2 && !(sBattleGroundMgr.isTesting() && queued))
        return;

    //uint32 teamCount[BG_TEAMS_COUNT];
    //teamCount[TEAM_ALLIANCE] = 0;
    //teamCount[TEAM_HORDE] = 0;

    //auto IncreaseTeamCount = [&](GroupQueueInfo* itr)
    //{
    //    uint32 team = itr->Team == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE;
    //    teamCount[team] += itr->Players.size();
    //};

    //std::unordered_map<uint32, uint32> tmp_map;

    //// ----- BG_QUEUE_PREMADE 
    //// first and add it without any special checks
    //// for example we can add premade with 3 healers and it will be fine
    //for (auto itr = m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].begin(); itr != m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].end(); ++itr)
    //{
    //    if ((*itr)->IsInvitedToBGInstanceGUID)
    //        continue;

    //    PlayerTeam team;
    //    if (teamCount[TEAM_ALLIANCE] < teamCount[TEAM_HORDE])
    //        team = ALLIANCE;
    //    else if (teamCount[TEAM_ALLIANCE] > teamCount[TEAM_HORDE])
    //        team = HORDE;
    //    else
    //        team = urand(0, 1) ? ALLIANCE : HORDE;

    //    (*itr)->Team = team;

    //    if (m_SelectionPools.AddGroup(*(itr), maxPlayers * 2))
    //    {
    //        for (std::map<uint64, PlayerQueueInfo*>::iterator pitr = (*itr)->Players.begin(); pitr != (*itr)->Players.end(); ++pitr)
    //        {
    //            Player* plr = sObjectAccessor.GetPlayerInWorld(pitr->first);
    //            if (!plr)
    //                continue;

    //            ++tmp_map[pitr->second->ClassMask];
    //        }

    //        IncreaseTeamCount(*itr);
    //    }
    //        
    //}

    //// ----- BG_QUEUE_NORMAL
    //// generate class frequency first
    //// class, count
    //for (GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].begin(); itr != m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].end(); ++itr)
    //{
    //    if ((*itr)->IsInvitedToBGInstanceGUID) // Skip when invited
    //        continue;

    //    for (std::map<uint64, PlayerQueueInfo*>::iterator pitr = (*itr)->Players.begin(); pitr != (*itr)->Players.end(); ++pitr)
    //    {
    //        Player* plr = sObjectAccessor.GetPlayerInWorld(pitr->first);
    //        if (!plr)
    //            continue;

    //        ++tmp_map[pitr->second->ClassMask];
    //    }
    //}

    //// copy map to vector (best way)
    //std::vector<std::pair<uint32, uint32>> count_classmask(tmp_map.begin(), tmp_map.end());

    //// from greater to less - it's important!
    //// premade (dru dru) joined, 2:0, now there are 2 rogs and 1 mag 1 war in queue and they will join like that
    //// 1) dru dru mag | war rog rog
    //// 2) dru dru rog | rog mag war - better (now)
    //std::sort(count_classmask.begin(), count_classmask.end(), [](const std::pair<uint32, uint32>& a, const std::pair<uint32, uint32>& b) {
    //    return a.second > b.second;
    //    });

    //// just add one by one, no big deal
    //PlayerTeam last_team = teamCount[TEAM_HORDE] > teamCount[TEAM_ALLIANCE] ? HORDE : ALLIANCE;
    //for (auto& cc : count_classmask)
    //{  
    //    for (GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].begin(); itr != m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].end(); ++itr)
    //    {
    //        if ((*itr)->IsInvitedToBGInstanceGUID)
    //            continue;

    //        for (std::map<uint64, PlayerQueueInfo*>::iterator pitr = (*itr)->Players.begin(); pitr != (*itr)->Players.end(); ++pitr)
    //        {
    //            Player* plr = sObjectAccessor.GetPlayerInWorld(pitr->first);
    //            if (!plr)
    //                continue;

    //            if (pitr->second->ClassMask == cc.first)
    //            {
    //                (*itr)->Team = last_team == ALLIANCE ? HORDE : ALLIANCE;

    //                if (m_SelectionPools.AddGroup(*itr, maxPlayers * 2))
    //                {
    //                    last_team = (*itr)->Team;
    //                    IncreaseTeamCount(*itr);
    //                }

    //                break;
    //            }
    //        }
    //    }
    //}
    
    for (uint8 queue_type : {BG_QUEUE_NORMAL, BG_QUEUE_PREMADE}) 
    {
        for (GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][queue_type].begin(); itr != m_QueuedGroups[bracket_id][queue_type].end(); ++itr)
        {
            if ((*itr)->IsInvitedToBGInstanceGUID) // Skip when invited
                continue;

            m_SelectionPools.AddGroup(*itr, maxPlayers * 2);
        }
    }

    // start bg if possible
    BattleGround* bg = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracket_id, ARENA_TYPE_NONE, false);
    if (!bg)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGroundQueue::Update - Cannot create battleground: %u", bgTypeId);
        return;
    }

    // invite those selection pools
    for (GroupsQueueType::const_iterator citr = m_SelectionPools.SelectedGroups.begin(); citr != m_SelectionPools.SelectedGroups.end(); ++citr)
        InviteGroupToBG((*citr), bg, (*citr)->Team);

    bg->StartBattleGround();
}

void BattleGroundQueue::TryStart2v2TestArena(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    size_t totalPlayers = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].size() + m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].size() * 2;

    uint32 max_players = 4;

    if (sBattleGroundMgr.GetDebugArenaId())
        max_players = 2;

    uint32 max_players_per_team = max_players / 2;

    if (totalPlayers < max_players)
        return;

    uint32 horde_count = 0;
    uint32 alliance_count = 0;

    m_SelectionPools.Init();

    for (GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].begin(); itr != m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].end(); ++itr)
    {
        if ((*itr)->IsInvitedToBGInstanceGUID) // Skip when invited
            continue;

        if (m_SelectionPools.GetPlayerCount() == max_players)
            break;

        if (horde_count < max_players_per_team)
        {
            (*itr)->Team = HORDE;
            m_SelectionPools.AddGroup((*itr), max_players);
            horde_count += max_players_per_team;
        }
        else if (alliance_count < max_players_per_team)
        {
            (*itr)->Team = ALLIANCE;
            m_SelectionPools.AddGroup((*itr), max_players);
            alliance_count += max_players_per_team;
        }
    }

    for (GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].begin(); itr != m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].end(); ++itr)
    {
        if ((*itr)->IsInvitedToBGInstanceGUID) // Skip when invited
            continue;
        
        if (m_SelectionPools.GetPlayerCount() == max_players)
            break;

        if (horde_count < max_players_per_team)
        {
            (*itr)->Team = HORDE;
            m_SelectionPools.AddGroup((*itr), max_players);
            horde_count += 1;
        }
        else if (alliance_count < max_players_per_team)
        {
            (*itr)->Team = ALLIANCE;
            m_SelectionPools.AddGroup((*itr), max_players);
            alliance_count += 1;
        }
    }

    if (m_SelectionPools.GetPlayerCount() == max_players)
    {
        BattleGround* arena = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracket_id, ARENA_TYPE_2v2, false);
        if (!arena)
        {
            sLog.outLog(LOG_CRITICAL, "ERROR: BattlegroundQueue::Update couldn't create arena instance for skirmish arena match!");
            return;
        }

        for (GroupsQueueType::const_iterator citr = m_SelectionPools.SelectedGroups.begin(); citr != m_SelectionPools.SelectedGroups.end(); ++citr)
            InviteGroupToBG((*citr), arena, (*citr)->Team);

        arena->StartBattleGround();
        return;
    }
}

void BattleGroundQueue::TryStart2v2Arena(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    if (m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].size() < 2)
        return;

    uint32 stepbystepTime = sWorld.getConfig(CONFIG_ARENA_STEP_BY_STEP_TIME);
    uint32 stepbystepValue = sWorld.getConfig(CONFIG_ARENA_STEP_BY_STEP_VALUE);

    uint32 arenaMinRating;
    uint32 arenaMaxRating;
    uint32 stepbystepChange;

    m_SelectionPools.Init();

    auto CalculateStep = [&](GroupQueueInfo* team)
    {
        return stepbystepValue * (uint8)((WorldTimer::getMSTime() - team->JoinTime) / stepbystepTime);
    };

    auto CheckStepRating = [&](uint32 team_rating, uint32 step)
    {
        return (step > team_rating || team_rating - step <= arenaMaxRating) && team_rating + step >= arenaMinRating;
    };

    for (GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].begin(); itr != m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].end(); ++itr)
    {
        if ((*itr)->IsInvitedToBGInstanceGUID)
            continue;

        arenaMinRating = (*itr)->ArenaTeamRating - sBattleGroundMgr.GetMaxRatingDifference();
        arenaMaxRating = (*itr)->ArenaTeamRating + sBattleGroundMgr.GetMaxRatingDifference();

        //sLog.outLog(LOG_SPECIAL, "2v2 queue: %u try to find team with rating between %u and %u", (*itr)->ArenaTeamId, arenaMinRating, arenaMaxRating);

        for (GroupsQueueType::iterator _itr = m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].begin(); _itr != m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].end(); ++_itr)
        {
            // team leaved from queue
            if (*itr == nullptr)
                break;

            if ((*_itr)->ArenaTeamId == (*itr)->ArenaTeamId || (*_itr)->IsInvitedToBGInstanceGUID)
                continue;

            stepbystepChange = CalculateStep((*_itr));

            //sLog.outLog(LOG_SPECIAL, "2v2 queue: - %u, rating %u (step_rating %u-%u, step: %u)", (*_itr)->ArenaTeamId, (*_itr)->ArenaTeamRating, (*_itr)->ArenaTeamRating - stepbystepChange, (*_itr)->ArenaTeamRating + stepbystepChange, stepbystepChange);

            //if (sWorld.ArenaTeamLosesRowSkip((*itr)->ArenaTeamId, (*_itr)->ArenaTeamId))
            //    continue;

            if (CheckStepRating((*_itr)->ArenaTeamRating, stepbystepChange))
            {
                //itr -> HORDE
                //_itr -> ALLIANCE

                (*itr)->Team = HORDE;
                m_SelectionPools.AddGroup((*itr), 4);

                (*_itr)->Team = ALLIANCE;
                m_SelectionPools.AddGroup((*_itr), 4);

                if (m_SelectionPools.GetPlayerCount() == 4)
                {
                    BattleGround* arena = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracket_id, ARENA_TYPE_2v2, true);
                    if (!arena)
                    {
                        sLog.outLog(LOG_CRITICAL, "ERROR: BattlegroundQueue::Update couldn't create arena instance for rated arena match!");
                        return;
                    }

                    (*itr)->OpponentsTeamRating = (*_itr)->ArenaTeamRating;
                    (*_itr)->OpponentsTeamRating = (*itr)->ArenaTeamRating;

                    InviteGroupToBG((*itr), arena, HORDE);
                    InviteGroupToBG((*_itr), arena, ALLIANCE);

                    arena->StartBattleGround();

                    //sLog.outLog(LOG_SPECIAL, "2v2 queue: - arena started between %u and %u", (*itr)->ArenaTeamId, (*_itr)->ArenaTeamId);
                    return;
                }

                //sLog.outLog(LOG_SPECIAL, "2v2 queue: - pools are not 2v2 (actually %u and %u)", m_SelectionPools[BG_TEAM_ALLIANCE].GetPlayerCount(), m_SelectionPools[BG_TEAM_HORDE].GetPlayerCount());
                return;
            }
        }
    }

    //sLog.outLog(LOG_SPECIAL, "2v2 queue: function exit");
}

void BattleGroundQueue::TryStartSolo3v3Arena(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    // https://godbolt.org/z/Esa87c334

    if (m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].size() < SOLO_3v3_MIN_PLAYERS * 2)
        return;

    // total queued count
    uint32 totalQueued = m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].size();
    uint32 teamclassmask[BG_TEAMS_COUNT];
    uint32 teamcount[BG_TEAMS_COUNT];
    for (int i = 0; i < BG_TEAMS_COUNT; i++)
    {
        teamclassmask[i] = 0;
        teamcount[i] = 0;
    }

    bool can_start = false;

    // key container to easy sort
    // count class
    // 5 1
    // 4 2
    // 4 3 

    // --- needed to solve this problem
    // 1 1 2
    // 2 3 3
    // ->
    // 1 2 3
    // 1 2 3
    // --- and this
    // (1,2 - healer classes)
    // 1 2 2
    // 3 3 4
    // + 4
    // ->
    // 1 3 4
    // 2 3 4
    // --- and also when 3 healer reg last
    // (1,2 - healer classes)
    // 1 2 3
    // 3 3 4
    // + 2, +5
    // ->
    // 1 3 4
    // 2 3 5

    // we need to use it to form count_class vector
    // it is needed to select first MAJORITY of classes
    // 3 druids in reg, 1 rog, 1 mage, 1 war
    // so handle druids first!
    // UPDATED: this is breaks JoinTime logic, so don't use it!
    //std::vector<std::pair<uint32, uint32>> count_classmask;

    // join time, GroupQueueInfo*
    std::multimap<uint32, GroupQueueInfo*, std::less<uint32>> jointimelist;

    auto CreateCountClass = [&]()
    {
        // cleanup
        if (!jointimelist.empty())
            jointimelist.clear();

        //if (!count_classmask.empty())
        //    count_classmask.clear();

        // need to generate every time because we changing team in next loop
        // need to get players by JoinTime first
        //std::unordered_map<uint32, uint32> tmp_map;
        for (GroupsQueueType::iterator itr = m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].begin(); itr != m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].end(); ++itr)
        {
            if ((*itr)->IsInvitedToBGInstanceGUID) // Skip when invited
                continue;

            std::map<uint64, PlayerQueueInfo*>* grpPlr = &(*itr)->Players;
            for (std::map<uint64, PlayerQueueInfo*>::iterator grpPlrItr = grpPlr->begin(); grpPlrItr != grpPlr->end(); grpPlrItr++)
            {
                Player* plr = sObjectAccessor.GetPlayerInWorld(grpPlrItr->first);
                if (!plr)
                    continue;

                jointimelist.insert(std::make_pair((*itr)->JoinTime, *itr));
                //++tmp_map[(*itr)->LeaderClassMask];
            }
        }
        
        // copy map to vector (best way)
        //std::vector<std::pair<uint32, uint32>> count_classmask(tmp_map.begin(), tmp_map.end());

        //// from greater to less - it's important!
        //std::sort(count_classmask.begin(), count_classmask.end(), [](const std::pair<uint32, uint32>& a, const std::pair<uint32, uint32>& b) {
        //    return a.second > b.second;
        //    });

        //return count_classmask;
    };

    auto CanJoinSolo3v3 = [&](uint32 team, uint32 classmask, bool healer, GroupQueueInfo* queueinfo, uint32& role_counter)
    {
        if (teamcount[team] >= SOLO_3v3_MIN_PLAYERS)
            return false;
        
        if (teamclassmask[team] & classmask)
            return false;

        // only one healer per team
        if (healer && (teamclassmask[team] & CLASS_HEALER_MASK))
            return false;

        if (m_SelectionPools.AddGroup(queueinfo, SOLO_3v3_MIN_PLAYERS * 2))
        {
            uint32 current_team = team == TEAM_ALLIANCE ? ALLIANCE : HORDE;
            
            if (current_team != queueinfo->Team)
                queueinfo->Team = GetOtherTeam(PlayerTeam(queueinfo->Team));

            teamclassmask[team] |= healer ? (classmask | CLASS_HEALER_MASK) : classmask;
            ++teamcount[team];
            ++role_counter;
            return true;
        }

        return false;
    };

    uint32 loopLimit = 15;
    while (loopLimit > 0)
    {
        if (can_start)
            break;
        
        m_SelectionPools.Init();

        for (int i = 0; i < BG_TEAMS_COUNT; i++)
        {
            teamclassmask[i] = 0;
            teamcount[i] = 0;
        }

        //count_classmask = CreateCountClass();
        CreateCountClass();

        // healers check
        uint32 healers_count = 0;
        for (auto& p : jointimelist)
        {
            if (healers_count == 2)
                break;

            auto queueinfo = p.second;
            auto classmask = p.second->LeaderClassMask;
            auto healer = p.second->LeaderIsSemiHealer;

            // not healer
            if (!healer)
                continue;

            if (!CanJoinSolo3v3(TEAM_ALLIANCE, classmask, healer, queueinfo, healers_count))
                CanJoinSolo3v3(TEAM_HORDE, classmask, healer, queueinfo, healers_count);
        }

        // if we don't have 2 healers then reset m_SelectionPools and invite only 6 dps
        if (healers_count != 2)
        {
            m_SelectionPools.Init();
            for (int i = 0; i < BG_TEAMS_COUNT; i++)
            {
                teamclassmask[i] = 0;
                teamcount[i] = 0;
            }
            healers_count = 0;
        }

        // dd check
        uint32 dps_count = 0;   
        for (auto& p : jointimelist)
        {
            auto queueinfo = p.second;
            auto classmask = p.second->LeaderClassMask;
            auto healer = p.second->LeaderIsSemiHealer;

            if (healer)
                continue;

            if (dps_count == 6 || dps_count == 4 && healers_count == 2)
            {
                if (teamcount[TEAM_HORDE] == 3 && teamcount[TEAM_ALLIANCE] == 3)
                    can_start = true;
                else
                    sLog.outLog(LOG_NOTIFY, "Arena3v3: team #1");

                break;
            }

            // randomizer :D
            if (urand(0, 1))
            {
                if (CanJoinSolo3v3(TEAM_ALLIANCE, classmask, healer, queueinfo, dps_count))
                    continue;

                if (CanJoinSolo3v3(TEAM_HORDE, classmask, healer, queueinfo, dps_count))
                    continue;
            }
            else
            {
                if (CanJoinSolo3v3(TEAM_HORDE, classmask, healer, queueinfo, dps_count))
                    continue;

                if (CanJoinSolo3v3(TEAM_ALLIANCE, classmask, healer, queueinfo, dps_count))
                    continue;
            }
        }

        if (dps_count == 6 || dps_count == 4 && healers_count == 2)
        {
            if (teamcount[TEAM_HORDE] == 3 && teamcount[TEAM_ALLIANCE] == 3)
                can_start = true;  
            else
                sLog.outLog(LOG_NOTIFY, "Arena3v3: team #2");

            break;
        }

        --loopLimit;
    }

    if (!can_start)
    {
        if (jointimelist.size() >= 6)
        {
            std::stringstream ss;
            for (auto& plr : jointimelist)
            {
                ss << plr.second->LeaderName << "(" << plr.first << " " << plr.second->LeaderClassMask << ") - ";
            }

            sLog.outLog(LOG_SPECIAL, "Arena3v3 MATCHDEBUG: Can't create match with: %s", ss.str().c_str());
        }

        return;
    }

    // Create temp arena team and store arenaTeamId
    ArenaTeam* arenaTeams[BG_TEAMS_COUNT];
    if (!CreateTempArenaTeamForQueue(arenaTeams))
    {
        sLog.outLog(LOG_CRITICAL, "Can't create CreateTempArenaTeamForQueue()!");
        return;
    }

    BattleGround* arena = sBattleGroundMgr.CreateNewBattleGround(bgTypeId, bracket_id, ARENA_TYPE_3v3, true);
    if (!arena)
        return;

    // invite those selection pools
    for (GroupsQueueType::const_iterator citr = m_SelectionPools.SelectedGroups.begin(); citr != m_SelectionPools.SelectedGroups.end(); ++citr)
    {
        if ((*citr)->Team == ALLIANCE)
        {
            (*citr)->ArenaTeamId = arenaTeams[TEAM_ALLIANCE]->GetId();
            InviteGroupToBG((*citr), arena, (*citr)->Team);
        }
        else
        {
            (*citr)->ArenaTeamId = arenaTeams[TEAM_HORDE]->GetId();
            InviteGroupToBG((*citr), arena, (*citr)->Team);
        }
    }

    arena->SetArenaTeamIdForTeam(ALLIANCE, arenaTeams[TEAM_ALLIANCE]->GetId());
    arena->SetArenaTeamIdForTeam(HORDE, arenaTeams[TEAM_HORDE]->GetId());

    // start bg
    arena->StartBattleGround();
}

bool BattleGroundQueue::CreateTempArenaTeamForQueue(ArenaTeam *arenaTeams[])
{
    uint32 total_players = 0;
    std::vector<Player*> team_members;

    for (uint32 team = 0; team < BG_TEAMS_COUNT; team++)
    {
        PlayerTeam current_team = team == TEAM_ALLIANCE ? ALLIANCE : HORDE;
        
        team_members.clear();
        uint32 team_count = 0;

        for (GroupsQueueType::const_iterator citr = m_SelectionPools.SelectedGroups.begin(); citr != m_SelectionPools.SelectedGroups.end(); ++citr)
        {
            if (team_count >= SOLO_3v3_MIN_PLAYERS)
            {
                sLog.outLog(LOG_CRITICAL, "CreateTempArenaTeamForQueue() team_count >= SOLO_3v3_MIN_PLAYERS");
                return false;
            }

            if ((*citr)->Team != current_team)
                continue;

            for (std::map<uint64, PlayerQueueInfo*>::iterator itr = (*citr)->Players.begin(); itr != (*citr)->Players.end(); ++itr)
            {
                if (Player* plr = sObjectAccessor.GetPlayerInWorldOrNot(itr->first))
                {
                    team_members.push_back(plr);
                    ++total_players;
                }
                break;
            }
        }

        ArenaTeam* tempArenaTeam = new ArenaTeam();  // delete it when all players have left the arena match. Stored in sArenaTeamMgr
        if (!tempArenaTeam)
        {
            sLog.outLog(LOG_CRITICAL, "Can't create tempArenaTeam - %u %u", team_members.size(), team);
            return false;
        }

        if (!tempArenaTeam->CreateTempForSolo3v3(team_members, team))
        {
            delete tempArenaTeam;
            return false;
        }

        sObjectMgr.AddArenaTeam(tempArenaTeam);
        arenaTeams[team] = tempArenaTeam;
    }

    return total_players == SOLO_3v3_MIN_PLAYERS * 2;
}

/*
this method is called when group is inserted, or player / group is removed from BG Queue - there is only one player's status changed, so we don't use while(true) cycles to invite whole queue
it must be called after fully adding the members of a group to ensure group joining
should be called from BattleGround::RemovePlayer function in some cases
*/
void BattleGroundQueue::Update(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint8 arenaType, bool isRated, uint32 arenaRating)
{
    //if no players in queue - do nothing
    if(m_QueuedGroups[bracket_id][BG_QUEUE_PREMADE].empty() && m_QueuedGroups[bracket_id][BG_QUEUE_NORMAL].empty())
        return;

    BattleGround* bg_template = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg_template)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: Update: bg template not found for %u", bgTypeId);
        return;
    }

    m_SelectionPools.Init();

    BattleGroundTypeId bgType = sBattleGroundMgr.GetDebugArenaId() ? sBattleGroundMgr.GetDebugArenaId() : bgTypeId;

    //battleground with free slot for player should be always in the beggining of the queue
    // maybe it would be better to create bgfreeslotqueue for each bracket_id
    //if (bg_template->isBattleGround())
    if (!bg_template->isArena())
    {
        BGFreeSlotQueueType::iterator itr, next;
        for (itr = sBattleGroundMgr.BGFreeSlotQueue[bgTypeId].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[bgTypeId].end(); itr = next)
        {
            next = itr;
            ++next;
            // DO NOT allow queue manager to invite new player to arena
            if ((*itr)->isBattleGround() && (*itr)->GetTypeID() == bgTypeId && (*itr)->GetBracketId() == bracket_id &&
                (*itr)->GetStatus() > STATUS_WAIT_QUEUE && (*itr)->GetStatus() < STATUS_WAIT_LEAVE)
            {
                BattleGround* bg = *itr; //we have to store battleground pointer here, because when battleground is full, it is removed from free queue (not yet implemented!!)
                // and iterator is invalid

                // clear selection pools
                m_SelectionPools.Init();

                // call a function that does the job for us
                FillPlayersToBG(bg, bracket_id);

                if (bg->GetFreeBGSlots() == 0)
                {
                    // remove BG from BGFreeSlotQueue
                    bg->RemoveFromBGFreeSlotQueue();
                }
            }
        }

        //for (auto& bg : sBattleGroundMgr.BGFreeSlotQueue[bgTypeId])
        //{
        //    if (bg->isBattleGround() && bg->GetTypeID() == bgTypeId && bg->GetBracketId() == bracket_id &&
        //        bg->GetStatus() > STATUS_WAIT_QUEUE && bg->GetStatus() < STATUS_WAIT_LEAVE)
        //    {
        //        FillPlayersToBG(bg, bracket_id);

        //        if (!bg->HasFreeSlots())
        //        {
        //            bg->RemoveFromBGFreeSlotQueue();
        //        }
        //    }
        //}
        
        TryStartBattleground(bgTypeId, bracket_id, bg_template->GetMinPlayersPerTeam(), bg_template->GetMaxPlayersPerTeam());
    }
    else if (arenaType == ARENA_TYPE_3v3)
    {
        TryStartSolo3v3Arena(bgType, bracket_id);
    }
    else if (arenaType == ARENA_TYPE_2v2)
    {
        BattleGroundTypeId bg_type = sBattleGroundMgr.GetDebugArenaId() ? sBattleGroundMgr.GetDebugArenaId() : bgType;

        if (isRated)
            TryStart2v2Arena(bg_type, bracket_id);
        else
            TryStart2v2TestArena(bg_type, bracket_id);
    }
}

/*********************************************************/
/***            BATTLEGROUND QUEUE EVENTS              ***/
/*********************************************************/

bool BGQueueInviteEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = sObjectMgr.GetPlayerInWorld( m_PlayerGuid );
    // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
    if (!plr)
        return true;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_BgInstanceGUID, m_BgTypeId);
    //if battleground ended and its instance deleted - do nothing
    if (!bg)
        return true;

    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
    uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)         // player is in queue or in battleground
    {
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_mutex.acquire(); // mutex for .find
        // check if player is invited to this bg ... this check must be here, because when player leaves queue and joins another, it would cause a problems
        BattleGroundQueue::QueuedPlayersMap const& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
        BattleGroundQueue::QueuedPlayersMap::const_iterator qItr = qpMap.find(m_PlayerGuid);
        if (qItr != qpMap.end() && qItr->second.GroupInfo->IsInvitedToBGInstanceGUID == m_BgInstanceGUID)
        {
            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, qItr->second.GroupInfo->Team, queueSlot, STATUS_WAIT_JOIN, bg->isBattleGround() ? INVITATION_REMIND_TIME_BG : INVITATION_REMIND_TIME_ARENA, 0);
            plr->SendPacketToSelf(&data);
        }
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_mutex.release();
    }
    return true;                                            //event will be deleted
}
void BGQueueInviteEvent::Abort(uint64 /*e_time*/)
{
    //do nothing
}

/*
    this event has many possibilities when it is executed:
    1. player is in battleground ( he clicked enter on invitation window )
    2. player left battleground queue and he isn't there any more
    3. player left battleground queue and he joined it again and IsInvitedToBGInstanceGUID = 0
    4. player left queue and he joined again and he has been invited to same battleground again -> we should not remove him from queue yet
    5. player is invited to bg and he didn't choose what to do and timer expired - only in this condition we should call queue::RemovePlayer
    we must remove player in the 5. case even if battleground object doesn't exist!
*/
bool BGQueueRemoveEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* plr = sObjectMgr.GetPlayerInWorld( m_PlayerGuid );
    if (!plr)
        // player logged off (we should do nothing, he is correctly removed from queue in another procedure)
        return true;

    BattleGround* bg = sBattleGroundMgr.GetBattleGround(m_BgInstanceGUID, m_BgTypeId);
    if (!bg)
        return true;

    debug_log("Battleground: removing player %u from bg queue for instance %u because of not pressing enter battle in time.",plr->GetGUIDLow(),m_BgInstanceGUID);
    
    BattleGroundBracketId bracket = plr->GetBattleGroundBracketIdFromLevel(bg->GetTypeID());

    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType());
    uint32 queueSlot = plr->GetBattleGroundQueueIndex(bgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES) // player is in queue
    {
        sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_mutex.acquire(); // mutex for .find and .RemovePlayer
        // check if player is invited to this bg ... this check must be here, because when player leaves queue and joins another, it would cause a problems
        BattleGroundQueue::QueuedPlayersMap& qpMap = sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_QueuedPlayers;
        BattleGroundQueue::QueuedPlayersMap::iterator qMapItr = qpMap.find(m_PlayerGuid);
        if (qMapItr != qpMap.end() && qMapItr->second.GroupInfo && qMapItr->second.GroupInfo->IsInvitedToBGInstanceGUID == m_BgInstanceGUID)
        {
            // lose rating when IsInvitedToBGInstanceGUID and leaving queue in 2v2
            if (qMapItr->second.GroupInfo->IsRated && qMapItr->second.GroupInfo->ArenaType == ARENA_TYPE_2v2)
            {
                ArenaTeam * at = sObjectMgr.GetArenaTeamById(qMapItr->second.GroupInfo->ArenaTeamId);
                if (at)
                {
                    at->MemberLost(plr, qMapItr->second.GroupInfo->OpponentsTeamRating);
                    at->SaveToDB();
                    at->Stats(plr->GetSession());
                }
            }
            // deserter for BG
            else if (bg->isBattleGround())
            {
                //plr->AddAura(SPELL_BG_DESERTER, plr, DESERTER_BG_DURATION_AFK);
                sLog.outLog(LOG_BG, "ID %u: Player %s didn't accept invitation (%s)", bg->GetBgInstanceId(), plr->GetName(), bg->GetPlayerCountInfo().c_str());
            }

            plr->RemoveBattleGroundQueueId(bgQueueTypeId);
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].RemovePlayer(m_PlayerGuid, true);
            sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bg->GetTypeID(), bg->GetBracketId());
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_mutex.release();

            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, m_PlayersTeam, queueSlot, STATUS_NONE, 0, 0);
            plr->SendPacketToSelf(&data);
        }
        else
            sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].m_mutex.release();
    }
    else
        debug_log("Battleground: Player was already removed from queue");

    //event will be deleted
    return true;
}

void BGQueueRemoveEvent::Abort(uint64 /*e_time*/)
{
    //do nothing
}

/*********************************************************/
/***            BATTLEGROUND MANAGER                   ***/
/*********************************************************/

BattleGroundMgr::BattleGroundMgr() : m_AutoDistributionTimeChecker(60000), m_ApAnnounce(false), debugArenaId(BATTLEGROUND_TYPE_NONE)
{
    for(uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; i++)
        m_BattleGrounds[i].clear();

    m_NextRatingDiscardUpdate = sWorld.getConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
    m_Testing=false;

    // LADDER
    for (uint32 i = 0; i < ARENA_TYPE_5v5; ++i)
    {
        for (uint32 j = 0; j < LADDER_CNT; ++j)
        {
            m_ladder[i][j].TeamName = "";
            m_ladder[i][j].Id = 0;
            m_ladder[i][j].Wins = 0;
            m_ladder[i][j].Loses = 0;
            m_ladder[i][j].Rating = 0;
            for (uint32 y = 0; y < LADDER_MAX_MEMBERS_CNT; ++y)
            {
                m_ladder[i][j].MembersInfo[y].Class = 0;
                m_ladder[i][j].MembersInfo[y].Race = 0;
                m_ladder[i][j].MembersInfo[y].Wins = 0;
                m_ladder[i][j].MembersInfo[y].Loses = 0;
                m_ladder[i][j].MembersInfo[y].Rating = 0;
                m_ladder[i][j].MembersInfo[y].PlayerName = "";
            }
        }
    }
    m_updateLadderTimer.Reset(10*MINUTE*MILLISECONDS);
    m_updateQueueTimer.Reset(5*MILLISECONDS);
    UpdateLadder();
    m_BGStillActive = BATTLEGROUND_TYPE_NONE;
    last_bgevent_hour = -1;
}

BattleGroundMgr::~BattleGroundMgr()
{
    DeleteAllBattleGrounds();

    // item exchanges - gotta clear mem
    for (std::map <uint32/*item_entry*/, entry_class_arr>::const_iterator i = m_arena_items_exchange.begin(); i != m_arena_items_exchange.end(); ++i)
    {
        delete[](*i).second.analog_class_arr;
        delete[](*i).second.analog_entry_arr;
    }
    m_arena_items_exchange.clear();
}

void BattleGroundMgr::DeleteAllBattleGrounds()
{
    for(uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; i++)
    {
        for(BattleGroundSet::iterator itr = m_BattleGrounds[i].begin(); itr != m_BattleGrounds[i].end();)
        {
            BattleGround * bg = itr->second;
            m_BattleGrounds[i].erase(itr++);
            delete bg;
        }
    }

    // destroy template battlegrounds that listed only in queues (other already terminated)
    for(uint32 bgTypeId = 0; bgTypeId < MAX_BATTLEGROUND_TYPE_ID; ++bgTypeId)
    {
        // ~BattleGround call unregistring BG from queue
        while(!BGFreeSlotQueue[bgTypeId].empty())
            delete BGFreeSlotQueue[bgTypeId].front();
    }
}

// used to update running battlegrounds, and delete finished ones
void BattleGroundMgr::Update(uint32 diff)
{
    BattleGroundSet::iterator itr, next;
    for (uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; i++)
    {
        itr = m_BattleGrounds[i].begin();
        // skip updating battleground template
        if (itr != m_BattleGrounds[i].end())
            ++itr;

        for(; itr != m_BattleGrounds[i].end(); itr = next)
        {
            next = itr;
            ++next;

            // use the SetDeleteThis variable
            // direct deletion caused crashes
            if (itr->second->m_SetDeleteThis)
            {
                BattleGround *bg = itr->second;
                if (bg)
                {
                    m_BattleGrounds[i].erase(itr);
                    delete bg;
                }
            }
        }
    }

    if (!m_QueueUpdateScheduler.empty())
    {
         //copy vector and clear the other
        std::vector<uint32> scheduled(m_QueueUpdateScheduler);
        m_QueueUpdateScheduler.clear();
        for (uint8 i = 0; i < scheduled.size(); i++)
        {
            BattleGroundQueueTypeId bgQueueTypeId = BattleGroundQueueTypeId(scheduled[i] >> 16);
            BattleGroundTypeId bgTypeId = BattleGroundTypeId((scheduled[i] >> 8) & 255);
            BattleGroundBracketId bracket_id = BattleGroundBracketId(scheduled[i] & 255);
            m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, bracket_id);
        }
    }
    else if (m_updateQueueTimer.Expired(diff))
    {
        for (uint8 i = BATTLEGROUND_QUEUE_AV; i <= BATTLEGROUND_QUEUE_EY; i++)
        {
            for (uint8 j = BG_BRACKET_ID_FIRST; j < BG_BRACKET_ID_LAST; j++)
            {
                m_BattleGroundQueues[i].Update(BATTLEGROUND_AV, BattleGroundBracketId(j));
                m_BattleGroundQueues[i].Update(BATTLEGROUND_WS, BattleGroundBracketId(j));
                m_BattleGroundQueues[i].Update(BATTLEGROUND_AB, BattleGroundBracketId(j));
                m_BattleGroundQueues[i].Update(BATTLEGROUND_EY, BattleGroundBracketId(j));
            }
		}
		m_updateQueueTimer = 5*MILLISECONDS;
	}   

    // if rating difference counts, maybe force-update queues
    if (sWorld.getConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE) && sWorld.getConfig(CONFIG_ARENA_RATING_DISCARD_TIMER))
    {
        // it's time to force update
        if (m_NextRatingDiscardUpdate.Expired(diff))
        {
            // forced update for level 70 rated arenas - battleground_template should start from level 70!
            m_BattleGroundQueues[BATTLEGROUND_QUEUE_2v2].Update(BATTLEGROUND_AA,BG_BRACKET_ID_FIRST,ARENA_TYPE_2v2,true,0);
            m_BattleGroundQueues[BATTLEGROUND_QUEUE_3v3].Update(BATTLEGROUND_AA,BG_BRACKET_ID_FIRST,ARENA_TYPE_3v3,true,0);
            //m_BattleGroundQueues[BATTLEGROUND_QUEUE_5v5].Update(BATTLEGROUND_AA, BG_BRACKET_ID_FIRST, ARENA_TYPE_5v5, true, 0); 
            // 3v3 and 5v5 queues disabled for better times :)
            //m_BattleGroundQueues[BATTLEGROUND_QUEUE_3v3_SOLO].Update(BATTLEGROUND_AA, BG_BRACKET_ID_FIRST, ARENA_TYPE_3v3_SOLO, true, 0);
            //m_BattleGroundQueues[BATTLEGROUND_QUEUE_1v1].Update(BATTLEGROUND_AA,BG_BRACKET_ID_FIRST,ARENA_TYPE_1v1,true,0);
            m_NextRatingDiscardUpdate = sWorld.getConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
        }
    }

    if (m_updateLadderTimer.Expired(diff))
    {
        UpdateLadder();
        m_updateLadderTimer = 10 * MINUTE*MILLISECONDS;
    }
    
    if (sWorld.getConfig(CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS))
    {
        if (m_AutoDistributionTimeChecker.Expired(diff))
        {
            if (time(NULL) > m_NextAutoDistributionTime)
            {
                DistributeArenaPoints();
                m_NextAutoDistributionTime = m_NextAutoDistributionTime + BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY * sWorld.getConfig(CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS);
                RealmDataDatabase.PExecute("UPDATE saved_variables SET NextArenaPointDistributionTime = '%llu'", m_NextAutoDistributionTime);
            }
            m_AutoDistributionTimeChecker = 60000; // check in 10 minutes
        }
    }

    time_t t = time(NULL);
    tm* tm = localtime(&t);
    if (last_bgevent_hour != tm->tm_hour)
    {
        last_bgevent_hour = tm->tm_hour;
        sWorld.SelectNextBGEvent(tm->tm_hour);
    }
}

void BattleGroundMgr::ScheduleQueueUpdate(BattleGroundQueueTypeId bgQueueTypeId, BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id)
{
    //This method must be atomic!
    //we will use only 1 number created of bgTypeId and bracket_id
    uint32 schedule_id = (bgQueueTypeId << 16) | (bgTypeId << 8) | bracket_id;
    bool found = false;
    for (uint8 i = 0; i < m_QueueUpdateScheduler.size(); i++)
    {
        if (m_QueueUpdateScheduler[i] == schedule_id)
        {
            found = true;
            break;
        }
    }

    if (!found)
        m_QueueUpdateScheduler.push_back(schedule_id);
}

void BattleGroundMgr::BuildBattleGroundStatusPacket(WorldPacket *data, BattleGround *bg, PlayerTeam team, uint8 QueueSlot, uint8 StatusID, uint32 Time1, uint32 Time2, uint32 arenatype, uint8 israted)
{
    // we can be in 3 queues in same time...
    if(StatusID == 0)
    {
        data->Initialize(SMSG_BATTLEFIELD_STATUS, 4*3);
        *data << uint32(QueueSlot);                         // queue id (0...2)
        *data << uint64(0);
        return;
    }

    data->Initialize(SMSG_BATTLEFIELD_STATUS, (4+1+1+4+2+4+1+4+4+4));
    *data << uint32(QueueSlot);                             // queue id (0...2) - player can be in 3 queues in time
    // uint64 in client
    *data << uint64( uint64(arenatype ? arenatype : bg->GetArenaType()) | (uint64(0x0D) << 8) | (uint64(bg->GetTypeID()) << 16) | (uint64(0x1F90) << 48) );
    *data << uint32(0);                                     // unknown
    // alliance/horde for BG and skirmish/rated for Arenas
    *data << uint8(bg->isArena() ? ( israted ? israted : bg->isRated() ) : bg->GetTeamIndexByTeamId(team));

    *data << uint32(StatusID);                              // status
    switch(StatusID)
    {
        case STATUS_WAIT_QUEUE:                             // status_in_queue
            *data << uint32(Time1);                         // average wait time, milliseconds
            *data << uint32(Time2);                         // time in queue, updated every minute!, milliseconds
            break;
        case STATUS_WAIT_JOIN:                              // status_invite
            *data << uint32(bg->GetMapId());                // map id
            *data << uint32(Time1);                         // time to remove from queue, milliseconds
            break;
        case STATUS_IN_PROGRESS:                            // status_in_progress
            *data << uint32(bg->GetMapId());                // map id
            *data << uint32(Time1);                         // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
            *data << uint32(Time2);                         // time from bg start, milliseconds
            *data << uint8(0x1);                            // Lua_GetBattlefieldArenaFaction (bool)
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: Unknown BG status!");
            break;
    }
}

void BattleGroundMgr::BuildPvpLogDataPacket(WorldPacket *data, BattleGround *bg)
{
    if (bg->isArena() && bg->GetStatus() == STATUS_WAIT_JOIN)
        return;

    uint8 type = (bg->isArena() ? 1 : 0);
                                                            // last check on 2.4.1
    data->Initialize(MSG_PVP_LOG_DATA, (1+1+4+40*bg->GetPlayerScoresSize()));
    *data << uint8(type);                                   // type (battleground=0/arena=1)

    if(type)                                                // arena
    {
        // it seems this must be according to BG_WINNER_A/H and _NOT_ BG_TEAM_A/H
        ArenaTeam* at1 = sObjectMgr.GetArenaTeamById(bg->m_ArenaTeamIds[1]); // Winner
        ArenaTeam* at2 = sObjectMgr.GetArenaTeamById(bg->m_ArenaTeamIds[0]); // Loser

        *data << uint32(at1 ? uint32(at1->GetRating()) : 0);
        *data << uint32(at1 ? uint32(at1->GetRating() + bg->m_ArenaTeamRatingChanges[1]) : 0);
        *data << uint32(at2 ? uint32(at2->GetRating()) : 0);
        *data << uint32(at2 ? uint32(at2->GetRating() + bg->m_ArenaTeamRatingChanges[0]) : 0);

        *data << (at1 ? at1->GetName() : "Unknown");
        *data << (at2 ? at2->GetName() : "Unknown");
    }

    if (bg->GetStatus() != STATUS_WAIT_LEAVE)
    {
        *data << uint8(0);                                  // bg not ended
    }
    else
    {
        *data << uint8(1);                                  // bg ended
        *data << uint8(bg->GetWinner());                    // who win
    }

    *data << (int32)(bg->GetPlayerScoresSize());

    for(BattleGroundScoreMap::const_iterator itr = bg->GetPlayerScoresBegin(); itr != bg->GetPlayerScoresEnd(); ++itr)
    {
        *data << (uint64)itr->first;
        *data << (int32)itr->second->KillingBlows;
        if (type == 0)
        {
            *data << (int32)itr->second->HonorableKills; // Those three items can be used to Log some arena events.
            *data << (int32)itr->second->Deaths;
            *data << (int32)(itr->second->BonusHonor);
        }
        else
        {
            Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
            PlayerTeam team = bg->GetPlayerTeam(itr->first);
            if (!team && plr)
                team = plr->GetBGTeam();
            if (( bg->GetWinner()==0 && team == ALLIANCE ) || ( bg->GetWinner()==1 && team==HORDE ))
                *data << uint8(1);
            else
                *data << uint8(0);
        }
        *data << (int32)itr->second->DamageDone;             // damage done
        *data << (int32)itr->second->HealingDone;            // healing done
        switch(bg->GetTypeID())                              // battleground specific things
        {
            case BATTLEGROUND_AV:
                *data << (uint32)0x00000005;                // count of next fields
                *data << (uint32)((BattleGroundAVScore*)itr->second)->GraveyardsAssaulted;  // GraveyardsAssaulted
                *data << (uint32)((BattleGroundAVScore*)itr->second)->GraveyardsDefended;   // GraveyardsDefended
                *data << (uint32)((BattleGroundAVScore*)itr->second)->TowersAssaulted;      // TowersAssaulted
                *data << (uint32)((BattleGroundAVScore*)itr->second)->TowersDefended;       // TowersDefended
                *data << (uint32)((BattleGroundAVScore*)itr->second)->SecondaryObjectives;  // SecondaryObjectives - free some of the Lieutnants
                break;
            case BATTLEGROUND_WS:
                *data << (uint32)0x00000002;                // count of next fields
                *data << (uint32)((BattleGroundWGScore*)itr->second)->FlagCaptures;         // flag captures
                *data << (uint32)((BattleGroundWGScore*)itr->second)->FlagReturns;          // flag returns
                break;
            case BATTLEGROUND_AB:
                *data << (uint32)0x00000002;                // count of next fields
                *data << (uint32)((BattleGroundABScore*)itr->second)->BasesAssaulted;       // bases asssulted
                *data << (uint32)((BattleGroundABScore*)itr->second)->BasesDefended;        // bases defended
                break;
            case BATTLEGROUND_EY:
                *data << (uint32)0x00000001;                // count of next fields
                *data << (uint32)((BattleGroundEYScore*)itr->second)->FlagCaptures;         // flag captures
                break;
            case BATTLEGROUND_NA:
            case BATTLEGROUND_BE:
            case BATTLEGROUND_AA:
            case BATTLEGROUND_RL:
                *data << (int32)0;                          // 0
                break;
            default:
                debug_log("Unhandled MSG_PVP_LOG_DATA for BG id %u", bg->GetTypeID());
                *data << (int32)0;
                break;
        }
    }
}
void BattleGroundMgr::BuildGroupJoinedBattlegroundPacket(WorldPacket *data, BattleGroundTypeId bgTypeId)
{
    /*bgTypeId is:
    0 - Your group has joined a battleground queue, but you are not eligible
    1 - Your group has joined the queue for AV
    2 - Your group has joined the queue for WS
    3 - Your group has joined the queue for AB
    4 - Your group has joined the queue for NA
    5 - Your group has joined the queue for BE Arena
    6 - Your group has joined the queue for All Arenas
    7 - Your group has joined the queue for EotS*/
    data->Initialize(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
    *data << uint32(bgTypeId);
}

void BattleGroundMgr::BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value)
{
    data->Initialize(SMSG_UPDATE_WORLD_STATE, 4+4);
    *data << uint32(field);
    *data << uint32(value);
}

void BattleGroundMgr::BuildPlaySoundPacket(WorldPacket *data, uint32 soundid)
{
    data->Initialize(SMSG_PLAY_SOUND, 4);
    *data << uint32(soundid);
}

void BattleGroundMgr::BuildPlayerLeftBattleGroundPacket(WorldPacket *data, const uint64& guid)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_LEFT, 8);
    *data << uint64(guid);
}

void BattleGroundMgr::BuildPlayerJoinedBattleGroundPacket(WorldPacket *data, Player *plr)
{
    data->Initialize(SMSG_BATTLEGROUND_PLAYER_JOINED, 8);
    *data << uint64(plr->GetGUID());
}

void BattleGroundMgr::InvitePlayer(Player* plr, BattleGroundQueueTypeId bgQueueTypeId, uint32 bgInstanceGUID, BattleGroundTypeId bgTypeId, PlayerTeam team)
{
    // set invited player counters:
    BattleGround* bg = GetBattleGround(bgInstanceGUID, bgTypeId);
    if(!bg)
        return;
    bg->IncreaseInvitedCount(team);

    BattleGroundBracketId bgQueueBracketId = plr->GetBattleGroundBracketIdFromLevel(bg->GetTypeID());
    sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].DecreaseQueuedPlayersCount(bgQueueBracketId);

    plr->SetInviteForBattleGroundQueueType(bgQueueTypeId, bgInstanceGUID);

    // set the arena teams for rated matches
    if(bg->isArena() && bg->isRated())
    {
        switch(bg->GetArenaType())
        {
            case ARENA_TYPE_2v2:
            {
                bg->SetArenaTeamIdForTeam(team, plr->GetArenaTeamId(0));
                break;
            }
            case ARENA_TYPE_3v3:
            {
                bg->SetArenaTeamIdForTeam(team, plr->GetArenaTeamId(1));
                break;
            }
            case ARENA_TYPE_5v5:
            {
                bg->SetArenaTeamIdForTeam(team, plr->GetArenaTeamId(2));
                break;
            }
            //case ARENA_TYPE_1v1:
            //{
            //    bg->SetArenaTeamIdForTeam(team, plr->GetArenaTeamId(3));
            //    break;
            //}
            default:
                break;
        }
    }

    // create invite events:
    //add events to player's counters ---- this is not good way - there should be something like global event processor, where we should add those events
    
    if (bg->isArena())
    {
        BGQueueInviteEvent* inviteEvent = new BGQueueInviteEvent(plr->GetGUID(), bgInstanceGUID, bgTypeId);
        plr->m_Events.AddEvent(inviteEvent, plr->m_Events.CalculateTime(INVITATION_REMIND_TIME_ARENA));
        BGQueueRemoveEvent* removeEvent = new BGQueueRemoveEvent(plr->GetGUID(), bgInstanceGUID, bgTypeId, team);
        plr->m_Events.AddEvent(removeEvent, plr->m_Events.CalculateTime(INVITE_ACCEPT_WAIT_TIME_ARENA));
    }
    else
    {
        BGQueueInviteEvent* inviteEvent = new BGQueueInviteEvent(plr->GetGUID(), bgInstanceGUID, bgTypeId);
        plr->m_Events.AddEvent(inviteEvent, plr->m_Events.CalculateTime(INVITATION_REMIND_TIME_BG));
        BGQueueRemoveEvent* removeEvent = new BGQueueRemoveEvent(plr->GetGUID(), bgInstanceGUID, bgTypeId, team);
        plr->m_Events.AddEvent(removeEvent, plr->m_Events.CalculateTime(INVITE_ACCEPT_WAIT_TIME_BG));
    }
}

BattleGround * BattleGroundMgr::GetBattleGround(uint32 InstanceID, BattleGroundTypeId bgTypeId)
{
    //search if needed
    BattleGroundSet::iterator itr;
    if (bgTypeId == BATTLEGROUND_TYPE_NONE)
    {
        for(uint32 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; i++)
        {
            itr = m_BattleGrounds[i].find(InstanceID);
            if (itr != m_BattleGrounds[i].end())
                return itr->second;
        }
        return NULL;
    }
    itr = m_BattleGrounds[bgTypeId].find(InstanceID);
    return ( (itr != m_BattleGrounds[bgTypeId].end()) ? itr->second : NULL );
}

BattleGround * BattleGroundMgr::GetBattleGroundTemplate(BattleGroundTypeId bgTypeId)
{
    //map is sorted and we can be sure that lowest instance id has only BG template
    return m_BattleGrounds[bgTypeId].empty() ? NULL : m_BattleGrounds[bgTypeId].begin()->second;
}

// create a new battleground that will really be used to play
BattleGround * BattleGroundMgr::CreateNewBattleGround(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint8 arenaType, bool isRated)
{
    // get the template BG
    BattleGround *bg_template = GetBattleGroundTemplate(bgTypeId);
    if (!bg_template)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: CreateNewBattleGround - bg template not found for %u", bgTypeId);
        return NULL;
    }

    //for arenas there is random map used
    if (bg_template->isArena())
    {
        // don't change - debugArenaId requires strict position
        BattleGroundTypeId arenas[] = {BATTLEGROUND_NA, BATTLEGROUND_BE, BATTLEGROUND_RL};

        if (!debugArenaId)
            bgTypeId = arenas[urand(0, 2)];
        
        bg_template = GetBattleGroundTemplate(bgTypeId);
       
        if (!bg_template)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Battleground: CreateNewBattleGround - bg template not found for %u", bgTypeId);
            return NULL;
        }
    }

    BattleGround *bg = NULL;
    // create a copy of the BG template
    switch(bgTypeId)
    {
        case BATTLEGROUND_AV:
            bg = new BattleGroundAV(*(BattleGroundAV*)bg_template);
            break;
        case BATTLEGROUND_WS:
            bg = new BattleGroundWS(*(BattleGroundWS*)bg_template);
            break;
        case BATTLEGROUND_AB:
            bg = new BattleGroundAB(*(BattleGroundAB*)bg_template);
            break;
        case BATTLEGROUND_NA:
            bg = new BattleGroundNA(*(BattleGroundNA*)bg_template);
            break;
        case BATTLEGROUND_BE:
            bg = new BattleGroundBE(*(BattleGroundBE*)bg_template);
            break;
        case BATTLEGROUND_AA:
            bg = new BattleGroundAA(*(BattleGroundAA*)bg_template);
            break;
        case BATTLEGROUND_EY:
            bg = new BattleGroundEY(*(BattleGroundEY*)bg_template);
            break;
        case BATTLEGROUND_RL:
            bg = new BattleGroundRL(*(BattleGroundRL*)bg_template);
            break;
        default:
            //error, but it is handled few lines above
            return 0;
    }

    // generate a new instance id
    bg->SetInstanceID(sMapMgr.GenerateInstanceId());         // set instance id

    // reset the new bg (set status to status_wait_queue from status_none)
    bg->Reset();

    // start the joining of the bg
    bg->SetStatus(STATUS_WAIT_JOIN);
    bg->SetBracketId(bracket_id);
    bg->SetArenaType(arenaType);
    bg->SetRated(isRated);
    bg->CalculateBracketLevelRange();

    bg->SetBonus(IsBGWithBonus(bgTypeId));

    sMapMgr.CreateBgMap(bg->GetMapId(), bg->GetBgInstanceId(), bg);

    // add BG to free slot queue
    bg->AddToBGFreeSlotQueue();

    // add bg to update list
    AddBattleGround(bg->GetBgInstanceId(), bg->GetTypeID(), bg);

    if (!bg_template->isArena())
        sLog.outLog(LOG_BG, "ID %u: Created bgtype %u, bracket_id %u, rated %u", bg->GetBgInstanceId(), bgTypeId, bracket_id, isRated);

    return bg;
}

// used to create the BG templates
uint32 BattleGroundMgr::CreateBattleGround(BattleGroundTypeId bgTypeId, bool IsArena, uint32 MinPlayersPerTeam, uint32 MaxPlayersPerTeam, uint32 LevelMin, uint32 LevelMax, char* BattleGroundName, uint32 MapID, float Team1StartLocX, float Team1StartLocY, float Team1StartLocZ, float Team1StartLocO, float Team2StartLocX, float Team2StartLocY, float Team2StartLocZ, float Team2StartLocO)
{
    // Create the BG
    BattleGround *bg = NULL;
    switch(bgTypeId)
    {
        case BATTLEGROUND_AV: bg = new BattleGroundAV; break;
        case BATTLEGROUND_WS: bg = new BattleGroundWS; break;
        case BATTLEGROUND_AB: bg = new BattleGroundAB; break;
        case BATTLEGROUND_NA: bg = new BattleGroundNA; break;
        case BATTLEGROUND_BE: bg = new BattleGroundBE; break;
        case BATTLEGROUND_AA: bg = new BattleGroundAA; break;
        case BATTLEGROUND_EY: bg = new BattleGroundEY; break;
        case BATTLEGROUND_RL: bg = new BattleGroundRL; break;
        default:              bg = new BattleGround;   break;                           // placeholder for non implemented BG
    }

    bg->SetMapId(MapID);
    bg->SetTypeID(bgTypeId);
    bg->SetInstanceID(0);
    bg->SetArenaorBGType(IsArena);
    bg->SetMinPlayersPerTeam(MinPlayersPerTeam);
    bg->SetMaxPlayersPerTeam(MaxPlayersPerTeam);
    bg->SetMinPlayers(MinPlayersPerTeam * 2);
    bg->SetMaxPlayers(MaxPlayersPerTeam * 2);
    bg->SetName(BattleGroundName);
    bg->SetTeamStartLoc(ALLIANCE, Team1StartLocX, Team1StartLocY, Team1StartLocZ, Team1StartLocO);
    bg->SetTeamStartLoc(HORDE,    Team2StartLocX, Team2StartLocY, Team2StartLocZ, Team2StartLocO);
    bg->SetLevelRange(LevelMin, LevelMax);

    // add bg to update list
    AddBattleGround(bg->GetBgInstanceId(), bg->GetTypeID(), bg);

    // return some not-null value, bgTypeId is good enough for me
    return bgTypeId;
}
void BattleGroundMgr::CreateInitialBattleGrounds()
{
    float AStartLoc[4];
    float HStartLoc[4];
    uint32 MaxPlayersPerTeam, MinPlayersPerTeam, MinLvl, MaxLvl, start1, start2;
    BattlemasterListEntry const *bl;
    WorldSafeLocsEntry const *start;

    uint32 count = 0;

    //                                                       0   1                 2                 3      4      5                6              7             8
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT id, MinPlayersPerTeam,MaxPlayersPerTeam,MinLvl,MaxLvl,AllianceStartLoc,AllianceStartO,HordeStartLoc,HordeStartO FROM battleground_template");

    if (!result)
    {
        //BarGoLink bar(1);

        ////bar.step();

        sLog.outString();
        sLog.outLog(LOG_DB_ERR, ">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        ////bar.step();

        uint32 bgTypeID_ = fields[0].GetUInt32();

        // can be overwrited by values from DB
        bl = sBattlemasterListStore.LookupEntry(bgTypeID_);
        if (!bl)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Battleground ID %u not found in BattlemasterList.dbc. Battleground not created.", bgTypeID_);
            continue;
        }

        BattleGroundTypeId bgTypeID = BattleGroundTypeId(bgTypeID_);

        MaxPlayersPerTeam = bl->maxplayersperteam;
        MinPlayersPerTeam = bl->maxplayersperteam/2;
        MinLvl = bl->minlvl;
        MaxLvl = bl->maxlvl;

        if (fields[1].GetUInt32())
            MinPlayersPerTeam = fields[1].GetUInt32();

        if (fields[2].GetUInt32())
            MaxPlayersPerTeam = fields[2].GetUInt32();

        if (bl->type == TYPE_ARENA)
        {
            MinLvl = sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL);
            MaxLvl = sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL);
        }
        else
        {
            if (fields[3].GetUInt32())
                MinLvl = fields[3].GetUInt32();

            if (fields[4].GetUInt32())
                MaxLvl = fields[4].GetUInt32();
        }

        start1 = fields[5].GetUInt32();

        start = sWorldSafeLocsStore.LookupEntry(start1);
        if (start)
        {
            AStartLoc[0] = start->x;
            AStartLoc[1] = start->y;
            AStartLoc[2] = start->z;
            AStartLoc[3] = fields[6].GetFloat();
        }
        else if (bgTypeID == BATTLEGROUND_AA)
        {
            AStartLoc[0] = 0;
            AStartLoc[1] = 0;
            AStartLoc[2] = 0;
            AStartLoc[3] = fields[6].GetFloat();
        }
        else
        {
            sLog.outLog(LOG_DB_ERR, "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.",bgTypeID,start1);
            continue;
        }

        start2 = fields[7].GetUInt32();

        start = sWorldSafeLocsStore.LookupEntry(start2);
        if (start)
        {
            HStartLoc[0] = start->x;
            HStartLoc[1] = start->y;
            HStartLoc[2] = start->z;
            HStartLoc[3] = fields[8].GetFloat();
        }
        else if (bgTypeID == BATTLEGROUND_AA)
        {
            HStartLoc[0] = 0;
            HStartLoc[1] = 0;
            HStartLoc[2] = 0;
            HStartLoc[3] = fields[8].GetFloat();
        }
        else
        {
            sLog.outLog(LOG_DB_ERR, "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.",bgTypeID,start2);
            continue;
        }

        bool IsArena = (bl->type == TYPE_ARENA);
        //sLog.outDetail("Creating battleground %s, %u-%u", bl->name[sWorld.GetDBClang()], MinLvl, MaxLvl);
        if (!CreateBattleGround(bgTypeID, IsArena, MinPlayersPerTeam, MaxPlayersPerTeam, MinLvl, MaxLvl, bl->name[sWorld.GetDefaultDbcLocale()], bl->mapid[0], AStartLoc[0], AStartLoc[1], AStartLoc[2], AStartLoc[3], HStartLoc[0], HStartLoc[1], HStartLoc[2], HStartLoc[3]))
            continue;

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u battlegrounds", count);
}

void BattleGroundMgr::InitAutomaticArenaPointDistribution()
{
	if (sWorld.getConfig(CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS))
	{
		sLog.outDebug("Initializing Automatic Arena Point Distribution");
		QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT NextArenaPointDistributionTime FROM saved_variables");
		if (!result)
		{
			sLog.outDebug("Battleground: Next arena point distribution time not found in SavedVariables, reseting it now.");
			m_NextAutoDistributionTime = time(NULL) + BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY * sWorld.getConfig(CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS);
			RealmDataDatabase.PExecute("INSERT INTO saved_variables (NextArenaPointDistributionTime) VALUES ('%llu')", m_NextAutoDistributionTime);
		}
		else
			m_NextAutoDistributionTime = (*result)[0].GetUInt64();

		time_t t = time(NULL);
		if (m_NextAutoDistributionTime <= t)
		{
			m_NextAutoDistributionTime = time(NULL) + BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY * sWorld.getConfig(CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS);
			sLog.outLog(LOG_CRITICAL, "m_NextAutoDistributionTime is broken! Fix it!");
		}

        sLog.outDebug("Automatic Arena Point Distribution initialized.");
    }
}

void BattleGroundMgr::DistributeArenaPoints()
{
	if (sWorld.isEasyRealm())
		return;
	
	// used to distribute arena points based on last week's stats
    //sWorld.SendGlobalText("Flushing Arena points based on team ratings, this may take a few minutes. Please stand by...", NULL);

    //temporary structure for storing maximum points to add values for all players
    std::map<uint32, uint32> PlayerPoints;

    //at first update all points for all team members
    for (ObjectMgr::ArenaTeamMap::iterator team_itr = sObjectMgr.GetArenaTeamMapBegin(); team_itr != sObjectMgr.GetArenaTeamMapEnd(); ++team_itr)
    {
        if (ArenaTeam * at = team_itr->second)
        {
			// skip 5v5, because it's using for BG
			if (at->GetType() == ARENA_TYPE_5v5)
				continue;
			
			at->UpdateArenaPointsHelper(PlayerPoints);
        }
    }

    //sWorld.SendGlobalText("Distributing arena points to players...", NULL);

    uint32 cycles = 0;
    //cycle that gives points to all players
    for (std::map<uint32, uint32>::iterator plr_itr = PlayerPoints.begin(); plr_itr != PlayerPoints.end(); ++plr_itr, cycles++)
    {
        //update to database
        RealmDataDatabase.PExecute("UPDATE characters SET arenaPoints = arenaPoints + '%u' WHERE `guid` = '%u'", plr_itr->second, plr_itr->first);
        sLog.outLog(LOG_ARENA_FLUSH, "Player guid: %u added arena points (update): %u", plr_itr->first, plr_itr->second);
        //add points if player is online
        Player* pl = sObjectMgr.GetPlayerInWorld(plr_itr->first);
        if (pl)
        {
            pl->ModifyArenaPoints(plr_itr->second);
            sLog.outLog(LOG_ARENA_FLUSH, "Player guid: %u added arena points: %u", plr_itr->first, plr_itr->second);
        }
            

        if (cycles == 500)
        {
            RealmDataDatabase.Ping(); // keep connection alive, we can stay in here for some time and we dont execute any SQL directly
            cycles = 0;
        }
    }

    PlayerPoints.clear();

    //sWorld.SendGlobalText("Finished setting arena points for online players.", NULL);

    //sWorld.SendGlobalText("Modifying played count, arena points etc. for loaded arena teams, sending updated stats to online players...", NULL);
    for (ObjectMgr::ArenaTeamMap::iterator titr = sObjectMgr.GetArenaTeamMapBegin(); titr != sObjectMgr.GetArenaTeamMapEnd(); ++titr)
    {
        if (ArenaTeam * at = titr->second)
        {
            // skip if TEMP solo 3v3
            if (at->GetId() >= 0xFFF00000)
                continue;

			// skip 5v5, because it's using for BG
			if (at->GetType() == ARENA_TYPE_5v5)
				continue;
            
            at->FinishWeek();                              // set played this week etc values to 0 in memory, too
            at->SaveToDB();                                // save changes
            at->NotifyStatsChanged();                      // notify the players of the changes
        }
    }

    //sWorld.SendGlobalText("Modification done.", NULL);

    //sWorld.SendGlobalText("Done flushing Arena points.", NULL);

    sWorld.SendWorldText(LANG_ARENA_FLUSH, 0);
}

void BattleGroundMgr::BuildBattleGroundListPacket(WorldPacket *data, ObjectGuid guid, Player* plr, BattleGroundTypeId bgTypeId)
{
    if (!plr)
        return;

    uint32 PlayerLevel = plr->GetLevel();

    data->Initialize(SMSG_BATTLEFIELD_LIST);
    *data << guid;                                          // battlemaster guid
    *data << uint32(bgTypeId);                              // battleground id
    if(bgTypeId == BATTLEGROUND_AA)                         // arena
    {
        *data << uint8(5);                                  // unk
        *data << uint32(0);                                 // unk
    }
    else                                                    // battleground
    {
        *data << uint8(0x00);                               // unk

        size_t count_pos = data->wpos();
        uint32 count = 0;
        *data << uint32(0);                                 // number of bg instances

        for(BattleGroundSet::iterator itr = m_BattleGrounds[bgTypeId].begin(); itr != m_BattleGrounds[bgTypeId].end(); ++itr)
        {
              // skip sending battleground template
            if( itr == m_BattleGrounds[bgTypeId].begin() )
                continue;
            if( PlayerLevel >= itr->second->GetMinLevel() && PlayerLevel <= itr->second->GetMaxLevel() )
            {
                *data << uint32(itr->second->GetBgInstanceId());
                ++count;
            }
        }
        data->put<uint32>( count_pos , count);
    }
}

bool BattleGroundMgr::SendToBattleGround(Player *pl, uint32 instanceId, BattleGroundTypeId bgTypeId)
{
    BattleGround *bg = GetBattleGround(instanceId, bgTypeId);
    if (bg)
    {
        uint32 mapid = bg->GetMapId();
        float x, y, z, O;
        PlayerTeam team = pl->GetBGTeam();
        /*if (team==0)
            team = pl->GetTeam();*/ // already in GetBGTeam()
        bg->GetTeamStartLoc(team, x, y, z, O);

        sLog.outDetail("BATTLEGROUND: Sending %s to map %u, X %f, Y %f, Z %f, O %f", pl->GetName(), mapid, x, y, z, O);
        return pl->TeleportTo(mapid, x, y, z, O);
    }
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: player %u trying to port to nonexistent bg instance %u",pl->GetGUIDLow(), instanceId);
    }

	return false;
}

void BattleGroundMgr::SendAreaSpiritHealerQueryOpcode(Player *pl, BattleGround *bg, const uint64& guid)
{
    WorldPacket data(SMSG_AREA_SPIRIT_HEALER_TIME, 12);
    uint32 time_ = 30000 - bg->GetLastResurrectTime();      // resurrect every 30 seconds
    if (time_ == uint32(-1))
        time_ = 0;
    data << guid << time_;
    pl->SendPacketToSelf(&data);
}

bool BattleGroundMgr::IsArenaType(BattleGroundTypeId bgTypeId)
{
    return (bgTypeId == BATTLEGROUND_AA ||
        bgTypeId == BATTLEGROUND_BE ||
        bgTypeId == BATTLEGROUND_NA ||
        bgTypeId == BATTLEGROUND_RL);
}

BattleGroundQueueTypeId BattleGroundMgr::BGQueueTypeId(BattleGroundTypeId bgTypeId, uint8 arenaType)
{
    switch (bgTypeId)
    {
        case BATTLEGROUND_WS:
            return BATTLEGROUND_QUEUE_WS;
        case BATTLEGROUND_AB:
            return BATTLEGROUND_QUEUE_AB;
        case BATTLEGROUND_AV:
            return BATTLEGROUND_QUEUE_AV;
        case BATTLEGROUND_EY:
            return BATTLEGROUND_QUEUE_EY;
        case BATTLEGROUND_AA:
        case BATTLEGROUND_NA:
        case BATTLEGROUND_RL:
        case BATTLEGROUND_BE:
            switch (arenaType)
            {
                case ARENA_TYPE_2v2:
                    return BATTLEGROUND_QUEUE_2v2;
                case ARENA_TYPE_3v3:
                    return BATTLEGROUND_QUEUE_3v3;
                case ARENA_TYPE_5v5:
                    return BATTLEGROUND_QUEUE_5v5;
                //case ARENA_TYPE_1v1:
                //    return BATTLEGROUND_QUEUE_1v1;
                default:
                    return BATTLEGROUND_QUEUE_NONE;
            }
        default:
            return BATTLEGROUND_QUEUE_NONE;
    }
}

BattleGroundTypeId BattleGroundMgr::BGTemplateId(BattleGroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_WS:
            return BATTLEGROUND_WS;
        case BATTLEGROUND_QUEUE_AB:
            return BATTLEGROUND_AB;
        case BATTLEGROUND_QUEUE_AV:
            return BATTLEGROUND_AV;
        case BATTLEGROUND_QUEUE_EY:
            return BATTLEGROUND_EY;
        case BATTLEGROUND_QUEUE_2v2:
        case BATTLEGROUND_QUEUE_3v3:
        case BATTLEGROUND_QUEUE_5v5:
        //case BATTLEGROUND_QUEUE_3v3_SOLO:
        //case BATTLEGROUND_QUEUE_1v1:
            return BATTLEGROUND_AA;
        default:
            return BattleGroundTypeId(0);
    }
}

ArenaType BattleGroundMgr::BGArenaType(BattleGroundQueueTypeId bgQueueTypeId)
{
    switch (bgQueueTypeId)
    {
        case BATTLEGROUND_QUEUE_2v2:
            return ARENA_TYPE_2v2;
        case BATTLEGROUND_QUEUE_3v3:
            return ARENA_TYPE_3v3;
        case BATTLEGROUND_QUEUE_5v5:
            return ARENA_TYPE_5v5;
        //case BATTLEGROUND_QUEUE_3v3_SOLO:
        //    return ARENA_TYPE_3v3_SOLO;
        //case BATTLEGROUND_QUEUE_1v1:
        //    return ARENA_TYPE_1v1;
        default:
            return ARENA_TYPE_NONE;
    }
}

void BattleGroundMgr::ToggleTesting()
{
    m_Testing = !m_Testing;
    if (m_Testing)
        sWorld.SendGlobalText("BattleGrounds are set to 1v1 for debugging. So, don't join as group.", NULL);
    else
        sWorld.SendGlobalText("BattleGrounds are set to normal playercount.", NULL);
}

void BattleGroundMgr::SetDebugArenaId(BattleGroundTypeId id)
{
    debugArenaId=id;
    //solo3v3MinPlayers = id ? 3 : 2;
}

uint32 BattleGroundMgr::GetMaxRatingDifference() const
{
    return sWorld.getConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE);
}

uint32 BattleGroundMgr::GetRatingDiscardTimer() const
{
    return sWorld.getConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
}

uint32 BattleGroundMgr::GetPrematureFinishTime() const
{
    if (sWorld.getConfig(CONFIG_IS_LOCAL))
		return 0;
    
    return sWorld.getConfig(CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

bool BattleGroundMgr::IsPrematureFinishTimerEnabled() const
{
    return sWorld.getConfig(CONFIG_BATTLEGROUND_TIMER_INFO);
}

bool BattleGroundMgr::IsWSGEndAfterEnabled() const
{
    return sWorld.getConfig(CONFIG_BATTLEGROUND_WSG_END_AFTER_ENABLED);
}

uint32 BattleGroundMgr::GetWSGEndAfterTime() const
{
    return sWorld.getConfig(CONFIG_BATTLEGROUND_WSG_END_AFTER_TIME);
}

bool BattleGroundMgr::IsAVEndAfterEnabled() const
{
    return sWorld.getConfig(CONFIG_BATTLEGROUND_AV_END_AFTER_ENABLED);
}

uint32 BattleGroundMgr::GetAVEndAfterTime() const
{
    return sWorld.getConfig(CONFIG_BATTLEGROUND_AV_END_AFTER_TIME);
}

uint32 BattleGroundMgr::GetArenaEndAfterTime() const
{
    return sWorld.getConfig(CONFIG_ARENA_END_AFTER_TIME);
}

HELLGROUND_EXPORT bool BattleGroundMgr::IsBGWithBonus(BattleGroundTypeId bgTypeId)
{
    uint32 eventId = 0;
    switch (bgTypeId)
    {
        // eventID is from game_event table
        case BATTLEGROUND_AV:
            eventId = 18;
            break;
        case BATTLEGROUND_EY:
            eventId = 21;
            break;
        case BATTLEGROUND_WS:
            eventId = 19;
            break;
        case BATTLEGROUND_AB:
            eventId = 20;
            break;
        default:
            break;
    }
    return sGameEventMgr.IsActiveEvent(eventId);
}

HELLGROUND_EXPORT bool BattleGroundMgr::IsBGEventActive(BattleGroundTypeId bgTypeId)
{
    uint32 eventId = 0;
    switch (bgTypeId)
    {
        // eventID is from game_event table
        case BATTLEGROUND_AV: 
            eventId = 153;
            break;
        case BATTLEGROUND_EY: 
            eventId = 151;
            break;
        case BATTLEGROUND_WS: 
            eventId = 152;
            break;
        case BATTLEGROUND_AB: 
            eventId = 150;
            break;
        default:
            break;
    }
    return sGameEventMgr.IsActiveEvent(eventId);
}

void BattleGroundMgr::SetBGEventEnding(uint16 event_id)
{ 
    switch (event_id)
    {
        // eventID is from game_event table
        case 153:
            m_BGStillActive = BATTLEGROUND_AV;
            break;
        case 151:
            m_BGStillActive = BATTLEGROUND_EY;
            break;
        case 152:
            m_BGStillActive = BATTLEGROUND_WS;
            break;
        case 150:
            m_BGStillActive = BATTLEGROUND_AB;
            break;
        default:
            m_BGStillActive = BATTLEGROUND_TYPE_NONE;
            break;
    }

    CheckBGEventAllEnded();
}

void BattleGroundMgr::CheckBGEventAllEnded()
{
    if (m_BGStillActive == BATTLEGROUND_TYPE_NONE)
        return;

    for (BattleGroundSet::iterator itr = m_BattleGrounds[m_BGStillActive].begin(); itr != m_BattleGrounds[m_BGStillActive].end(); ++itr)
    {
        // skip sending battleground template
        if (itr == m_BattleGrounds[m_BGStillActive].begin())
            continue;

        if (itr->second->GetStatus() < STATUS_WAIT_LEAVE) // something is active -> return
            return;
    }
    m_BGStillActive = BATTLEGROUND_TYPE_NONE;
}

void BattleGroundMgr::LoadBattleMastersEntry()
{
    mBattleMastersMap.clear();                              // need for reload case

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry,bg_template FROM battlemaster_entry");

    if (!result)
    {
        //BarGoLink bar(1);
        ////bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 battlemaster entries - table is empty!");
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    uint32 count = 0;

    do
    {
        ++count;
        ////bar.step();

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId  = fields[1].GetUInt32();
        if (!sBattlemasterListStore.LookupEntry(bgTypeId))
        {
            sLog.outLog(LOG_DB_ERR, "Table `battlemaster_entry` contain entry %u for not existed battleground type %u, ignored.",entry,bgTypeId);
            continue;
        }

        mBattleMastersMap[entry] = BattleGroundTypeId(bgTypeId);

    }
    while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u battlemaster entries", count);
}

void BattleGroundMgr::LoadArenaRestrictions()
{
    // clear the previous information
    // specs
    memset(&m_arena_specs, 0, sizeof(m_arena_specs));

    // item restrictions
    m_arena_item_restrictions.clear();

    // item exchanges
    for (std::map <uint32/*item_entry*/, entry_class_arr>::const_iterator i = m_arena_items_exchange.begin(); i != m_arena_items_exchange.end(); ++i)
    {
        delete[](*i).second.analog_class_arr;
        delete[](*i).second.analog_entry_arr;
    }
    m_arena_items_exchange.clear();

    if (!sWorld.getConfig(CONFIG_ARENA_PVE_RESTRICTED))
    {
        sLog.outString();
        sLog.outString(">> Arena PVE restrictions are disabled. Not loading data.");
        return;
    }

    QueryResultAutoPtr resultSpecs = GameDataDatabase.Query("SELECT id, spell FROM arena_restrictions_specs");

    if (!resultSpecs)
    {
        //BarGoLink barSpecs(1);
        //barSpecs.step();
        
        sLog.outString();
        sLog.outString(">> Loaded 0 arena restriction specs! Not loading item restrictions (pointless)!");
        return;
    }

    //BarGoLink barSpecs(resultSpecs->GetRowCount());
    
    uint32 count = 0;
    
    do
    {
        //barSpecs.step();
        
        Field *fields = resultSpecs->Fetch();
        
        uint32 id = fields[0].GetUInt32();
        uint32 spell_id = fields[1].GetUInt32();
        if (id == 0 || id > 64)
        {
            sLog.outLog(LOG_DB_ERR, "Table `arena_restrictions_specs` contain entry %u which is out of scope (should be 1-64, cause it is a flag), ignored.", id);
            continue;
        }
        
        if (!spell_id)
        {
            sLog.outLog(LOG_DB_ERR, "Table `arena_restrictions_specs` contain entry %u which has spell_id 0, ignored.", id);
            continue;
        }
        
        m_arena_specs[count].spec_flag = uint64(1) << (id-1);
        m_arena_specs[count].spell_id = spell_id;

        ++count;

    } while (resultSpecs->NextRow());
    
    sLog.outString();
    sLog.outString(">> Loaded %u arena spec entries", count);
    



    /////////////////////////////////////////////// SPECS LOADED, NOW LOADING ITEMS PER SPEC //////////////////
    QueryResultAutoPtr resultItems = GameDataDatabase.Query("SELECT `entry`, `flag` FROM arena_restrictions_items");
    
    if (!resultItems)
    {
        //BarGoLink barItems(1);
        //barItems.step();
    
        sLog.outString();
        sLog.outString(">> Loaded 0 arena restriction items!");
        return;
    }
    
    //BarGoLink barItems(resultItems->GetRowCount());
    
    count = 0;
    
    do
    {
        ++count;
        //barItems.step();
        
        Field *fields = resultItems->Fetch();
        
        uint32 item_id = fields[0].GetUInt32();
        uint64 allow_flag = fields[1].GetUInt64();
        
        m_arena_item_restrictions[item_id] = allow_flag;

    } while (resultItems->NextRow());
    
    sLog.outString();
    sLog.outString(">> Loaded %u arena item restrictions", count);




    /////////////////////////////////////////////// ITEMS LOADED, NOW LOADING ITEMS EXCHANGES //////////////////
    typedef std::pair<uint32/*analog item entry*/, uint32/*class*/> entry_class_pair;
    std::map <uint32/*item_entry*/, std::vector<entry_class_pair>> tempMap;

    QueryResultAutoPtr resultAnalogs = GameDataDatabase.Query("SELECT entry, analog, analog_class FROM arena_restrictions_analogs");

    if (!resultAnalogs)
    {
        //BarGoLink barAnalogs(1);
        //barAnalogs.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 arena restriction items!");
        return;
    }

    //BarGoLink barAnalogs(resultAnalogs->GetRowCount());

    count = 0;

    do
    {
        ++count;
        //barAnalogs.step();

        Field *fields = resultAnalogs->Fetch();

        uint32 item_id = fields[0].GetUInt32();
        uint32 analog_item_id = fields[1].GetUInt32();
        uint32 analog_class = fields[2].GetUInt32();

        tempMap[item_id].push_back(std::make_pair(analog_item_id, analog_class));
        // later change where data is saved from vector to an array (array doesnt care if there are 1 or 2 elements, it will take x1 or x2 space)

    } while (resultAnalogs->NextRow());

    // move from temp container to constant container
    for (std::map <uint32/*item_entry*/, std::vector<entry_class_pair>>::const_iterator i = tempMap.begin(); i != tempMap.end(); ++i)
    {
        // (*i).second.shrink_to_fit(); // --- not needed, cause it is temp anyway. size() will give us actual count of entries

        uint32 itemId = (*i).first;
        uint32 arrSize = (*i).second.size();
        m_arena_items_exchange[itemId].size = arrSize;
        m_arena_items_exchange[itemId].analog_entry_arr = new uint32[arrSize];
        m_arena_items_exchange[itemId].analog_class_arr = new uint32[arrSize];

        for (uint32 j = 0; j < arrSize; ++j)
        {
            m_arena_items_exchange[itemId].analog_entry_arr[j] = (*i).second[j].first;
            m_arena_items_exchange[itemId].analog_class_arr[j] = (*i).second[j].second;
        }
    }

    sLog.outString();
    sLog.outString(">> Loaded %u arena item analogs", count);
}

void BattleGroundMgr::LoadArenaItemsLogging()
{
    m_arena_items_to_log.clear();

    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT entry, cost FROM arena_items");

    if (!result)
    {
        //BarGoLink barAnalogs(1);
        //barAnalogs.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 arena items to log!");
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    uint32 count = 0;

    do
    {
        ++count;
        ////bar.step();

        Field *fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 cost = fields[1].GetUInt32();

        arena_items_to_log i = std::make_pair(entry, cost);
        m_arena_items_to_log.insert(i);
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u arena items to log", count);
}

bool BattleGroundMgr::IsArenaTypeRestrictedPVE(uint8 arenatype)
{
    if (!sWorld.getConfig(CONFIG_ARENA_PVE_RESTRICTED))
        return false;

    return arenatype == ARENA_TYPE_2v2;
}

bool BattleGroundMgr::IsPlayerArenaRestricted(Player* plr, uint8 arenatype)
{
    if (!IsArenaTypeRestrictedPVE(arenatype))
        return false;

    if (plr->GetFreeTalentPoints())
        return true;

    return IsPlayerArenaItemRestricted(plr);
}

HELLGROUND_EXPORT bool BattleGroundMgr::IsPlayerArenaItemRestricted(Player* plr)
{
    if (!sWorld.getConfig(CONFIG_ARENA_PVE_RESTRICTED))
        return false;

    uint64 specFlag = 0;
    ArenaRestrictedGetPlayerSpec(plr, specFlag);

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        Item* pItem = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && IsItemArenaRestricted(pItem->GetEntry(), specFlag))
            return true;
    }

    return false;
}

HELLGROUND_EXPORT bool BattleGroundMgr::IsItemArenaRestricted(uint32 itemId, uint64 const &specFlag)
{
    std::map<uint32, uint64>::const_iterator itr = m_arena_item_restrictions.find(itemId);
    if (itr != m_arena_item_restrictions.end())
    {
        // second is allow_flag
        if (!(itr->second & specFlag)) // item is not allowed
            return true;
    }
    return false;
}

bool BattleGroundMgr::IsItemArenaRestricted(uint32 itemId, const Player* const plr)
{
    uint64 specFlag = 0;
    ArenaRestrictedGetPlayerSpec(plr, specFlag);
    return IsItemArenaRestricted(itemId, specFlag);
}

HELLGROUND_EXPORT void BattleGroundMgr::ArenaRestrictedGetPlayerSpec(const Player* const plr, uint64 &specFlag)
{
    for (uint32 i = 0; i < 64; ++i)
    {
        if (!m_arena_specs[i].spec_flag)
            break; // specs have ended

        if (plr->HasSpell(m_arena_specs[i].spell_id))
        {
            specFlag = m_arena_specs[i].spec_flag;
            break; // break for spec search
        }
    }
}

HELLGROUND_EXPORT void BattleGroundMgr::ArenaRestrictedSendSwapList(Creature* sender, Player* plr, uint32 item_slot)
{
    if (!sWorld.getConfig(CONFIG_ARENA_PVE_RESTRICTED))
        return;

    uint64 specFlag = 0;
    ArenaRestrictedGetPlayerSpec(plr, specFlag);

    Item* pItem = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, item_slot);
    if (pItem && sBattleGroundMgr.IsItemArenaRestricted(pItem->GetEntry(), specFlag))
    {
        for (auto itr = m_arena_items_exchange_restricted.begin(); itr != m_arena_items_exchange_restricted.end(); ++itr)
        {
            Player *player = ObjectAccessor::GetPlayerInWorld((*itr).first);
            // delete if no player
            if (!player)
            {
                m_arena_items_exchange_restricted.erase(itr);
                if (!m_arena_items_exchange_restricted.empty())
                {
                    itr = m_arena_items_exchange_restricted.begin();
                    continue;
                }
                else
                    break;
            }
        }
        m_arena_items_exchange_restricted[plr->GetGUID()] = pItem->GetGUID();
    }
    else
    {
        sender->Whisper(plr->GetSession()->GetHellgroundString(LANG_RESTRICTION_NO_ITEM_OR_NOT_RESTRICTED), plr->GetGUID());
        sScriptMgr.OnGossipSelect(plr, sender, /*GOSSIP_SENDER_MAIN*/1, /*GOSSIP_ACTION_INFO_DEF + 59*/1059, NULL);
        return;
    }

    /*FROM SHOP*/
    auto tryFind = m_arena_items_exchange.find(pItem->GetEntry());
    if (tryFind == m_arena_items_exchange.end()) // this item has no analogs
    {
        sender->Whisper(plr->GetSession()->GetHellgroundString(LANG_RESTRICTION_NO_ANALOGS), plr->GetGUID());
        sScriptMgr.OnGossipSelect(plr, sender, /*GOSSIP_SENDER_MAIN*/1, /*GOSSIP_ACTION_INFO_DEF + 59*/1059, NULL);
        return;
    }

    // ITEM SENDING
    ItemPrototype const* lastItemProto = ObjectMgr::GetItemPrototype(GO_BACK_ITEM_ARENA_ITEMS_EXCHANGE);
    if (!lastItemProto)
    {
        // just go to arena queue
        sScriptMgr.OnGossipSelect(plr, sender, /*GOSSIP_SENDER_MAIN*/1, /*GOSSIP_ACTION_INFO_DEF + 59*/1059, NULL);
        return;
    }

    ItemPrototype const* itemProto = NULL;
    uint32 arrSize = (*tryFind).second.size;
    uint32 classMask = plr->GetClassMask();
    uint8 numitems = arrSize;
    // now need to decrease numitems, cause not all items are available to us
    for (uint32 j = 0; j < arrSize; ++j)
    {
        if ((*tryFind).second.analog_class_arr[j] // there is class restriction
            && !(classMask & (*tryFind).second.analog_class_arr[j]) /* we're not the specified class*/)
            --numitems;
    }

    uint8 go_back_buttons;
    if (numitems) // it is possible that there will be no items in this category for this class
        //example numitems 1
        go_back_buttons = (numitems - 1) / 9 + 1; // 6 ? 10. 9 ? 10. 10 ? 2. 17 ? 2. 18 ? 2. 19 ? 3.
    else
    {
        sender->Whisper(plr->GetSession()->GetHellgroundString(LANG_RESTRICTION_NO_ANALOGS_FOR_YOU), plr->GetGUID());
        sScriptMgr.OnGossipSelect(plr, sender, /*GOSSIP_SENDER_MAIN*/1, /*GOSSIP_ACTION_INFO_DEF + 59*/1059, NULL);
        return;
    }

    WorldPacket data(SMSG_LIST_INVENTORY, (8 + 1 + (go_back_buttons * 10) * 8 * 4));

    data << uint64(sender->GetGUID());
    data << uint8(go_back_buttons * 10);

    uint8 curNum = 0;
    for (uint8 count = 0; count < (go_back_buttons * 10);)
    {
        // count must be increased before all other
        ++count;
        if ((count % 10) == 0) // each page tenth item is go_back
        {
            data << uint32(count);
            data << uint32(lastItemProto->ItemId);
            data << uint32(lastItemProto->DisplayInfoID);
            data << uint32(0xFFFFFFFF);
            data << uint32(0);
            data << uint32(lastItemProto->MaxDurability);
            data << uint32(1); // buyCount - should never be more than 1
            data << uint32(0);
        }
        else if (curNum < arrSize)
        {
            // find the item that player can use
            while (curNum < arrSize
                && (*tryFind).second.analog_class_arr[curNum] // there is class restriction
                && !(classMask & (*tryFind).second.analog_class_arr[curNum]) /* we're not the specified class*/)
                ++curNum;
            
            if (curNum < arrSize)
            {
                itemProto = ObjectMgr::GetItemPrototype((*tryFind).second.analog_entry_arr[curNum]);
                ++curNum;
            }
            else
                itemProto = NULL;
            if (itemProto)
            {
                data << uint32(count);
                data << uint32(itemProto->ItemId);
                data << uint32(itemProto->DisplayInfoID);
                data << uint32(0xFFFFFFFF);
                data << uint32(0); // in gold
                data << uint32(itemProto->MaxDurability);
                data << uint32(1); // buyCount - should never be more than 1
                data << uint32(0);
            }
            else // empty space
            {
                data << uint32(count);
                data << uint32(0);
                data << uint32(0);
                data << uint32(0xFFFFFFFF);
                data << uint32(0);
                data << uint32(0);
                data << uint32(1); // buyCount - should never be more than 1
                data << uint32(0);
            }
        }
        else // empty space
        {
            data << uint32(count);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0xFFFFFFFF);
            data << uint32(0);
            data << uint32(0);
            data << uint32(1); // buyCount - should never be more than 1
            data << uint32(0);
        }
    }

    plr->GetSession()->SendPacket(&data);
}

//void BattleGroundMgr::ArenaRestrictedHandleBuyPacket(Creature* sender, Player*plr, uint32 buy_item_id, ItemPrototype const *pProto)
//{
//    if (!sWorld.getConfig(CONFIG_ARENA_PVE_RESTRICTED))
//        return;
//
//    /*FROM SHOP*/
//    if (buy_item_id == GO_BACK_ITEM_ARENA_ITEMS_EXCHANGE)
//        sScriptMgr.OnGossipSelect(plr, sender, /*GOSSIP_SENDER_MAIN*/1, /*GOSSIP_ACTION_INFO_DEF + 59*/1059, NULL);
//    // buying something
//    else
//    {
//        if (!buy_item_id)
//            return;
//
//        auto ptrRestrItemGuid = m_arena_items_exchange_restricted.find(plr->GetGUID());
//        uint64 restricted_item_guid = ptrRestrItemGuid != m_arena_items_exchange_restricted.end() ? (*ptrRestrItemGuid).second : 0;
//        
//        // couldn't find the guid of item that player wants to change
//        if (!restricted_item_guid)
//            return;
//
//        Item* restrItem = plr->GetItemByGuid(restricted_item_guid);
//        if (!restrItem) // player has no restricted item with this guid
//            return;
//
//        if (!plr->ArenaRestrictedCanSwap(restrItem->GetGUIDLow())) // player has already swapped this item for something else
//            return;
//
//        auto analogPtr = m_arena_items_exchange.find(restrItem->GetEntry());
//        if (analogPtr == m_arena_items_exchange.end())
//            return; // there is no analogs for this restricted item
//
//        uint32* arrItem = (*analogPtr).second.analog_entry_arr;
//        uint32* arrClass = (*analogPtr).second.analog_class_arr;
//        uint32 arrSize = (*analogPtr).second.size;
//        uint32 classMask = plr->GetClassMask();
//
//        bool found = false;
//        for (uint32 i = 0; i < arrSize; ++i)
//        {
//            if (arrItem[i] == buy_item_id &&
//                (!arrClass[i] // there is no class restriction
//                || (classMask & arrClass[i]) /* we're the specified class*/))
//            {
//                found = true;
//                break;
//            }
//        }
//
//        if (!found) // there is NO such analog for this item id!
//            return;
//
//        ItemPosCountVec dest;
//        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, buy_item_id, 1);
//        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
//        {
//            plr->SendEquipError(msg, NULL, NULL);
//            return;
//        }
//
//        if (Item *it = plr->StoreNewItem(dest, buy_item_id, true))
//        {
//            WorldPacket data(SMSG_BUY_ITEM, (8 + 4 + 4 + 4));
//            data << sender->GetGUID();
//            data << (uint32)(/*vendor_slot*/1);                // numbered from 1 at client
//            data << (uint32)(0xFFFFFFFF);
//            data << (uint32)1;
//            plr->SendPacketToSelf(&data);
//
//            plr->SendNewItem(it, 1, true, false, false);
//
//            plr->ArenaRestrictedAddSwap(restrItem, it);
//
//            it->SetEnchantment(PERM_ENCHANTMENT_SLOT, restrItem->GetEnchantmentId(PERM_ENCHANTMENT_SLOT), 0, 0);
//
//            plr->SaveToDB();
//        }
//        // send player back to swappings
//        sScriptMgr.OnGossipSelect(plr, sender, /*GOSSIP_SENDER_MAIN*/1, /*GOSSIP_ACTION_INFO_DEF + 59*/1059, NULL);
//    }
//}

HELLGROUND_EXPORT BattleGroundMgr* BattleGroundMgr::getBattleGroundMgrFromScripts()
{
    return &sBattleGroundMgr;
}

ArenaType BattleGroundMgr::getArenaTypeBySlot(uint8 arenaSlot)
{
    switch (arenaSlot)
    {
        case 0:
            return ARENA_TYPE_2v2;
        case 1:
            return ARENA_TYPE_3v3;
        case 2:
            return ARENA_TYPE_5v5;
        //case 3:
        //    return ARENA_TYPE_1v1;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: Unknown arena slot %u at HandleBattlemasterJoinArena()", arenaSlot);
            return ARENA_TYPE_NONE;
    }
}

void UpdateLadderTeamMembersCallback(QueryResultAutoPtr result, uint32 arena_type_and_pos)
{
    if (!result)
        return;

    uint32 arena_type = arena_type_and_pos & 0x0000FFFF;
    uint32 teamPos = (arena_type_and_pos & 0xFFFF0000) >> 16;

    Ladder_TeamInfo* teamInfo = sBattleGroundMgr.getLadderForModify(arena_type);

    Ladder_PlayerInfo* plrInfo = &(teamInfo[teamPos].MembersInfo[0]);

    // "info" should already be set to 0
    uint32 pos = 0;
    do
    {
        Field *fields = result->Fetch();

        plrInfo[pos].PlayerName = fields[0].GetCppString();
        plrInfo[pos].Race = fields[1].GetUInt32();
        plrInfo[pos].Class = fields[2].GetUInt32();
        plrInfo[pos].Wins = fields[3].GetUInt32();
        plrInfo[pos].Loses = fields[4].GetUInt32();
        plrInfo[pos].Rating = fields[5].GetUInt32();

        ++pos;
    } while (result->NextRow());
}

void UpdateLadderTeamsCallback(QueryResultAutoPtr result, uint32 arena_type)
{
    if (!result)
        return;

    Ladder_TeamInfo* teamInfo = sBattleGroundMgr.getLadderForModify(arena_type);

    // set ALL to 0 before using
    for (uint32 j = 0; j < LADDER_CNT; ++j)
    {
        teamInfo[j].TeamName = "";
        teamInfo[j].Id = 0;
        teamInfo[j].Wins = 0;
        teamInfo[j].Loses = 0;
        teamInfo[j].Rating = 0;
        for (uint32 y = 0; y < LADDER_MAX_MEMBERS_CNT; ++y)
        {
            teamInfo[j].MembersInfo[y].Class = 0;
            teamInfo[j].MembersInfo[y].Race = 0;
            teamInfo[j].MembersInfo[y].Wins = 0;
            teamInfo[j].MembersInfo[y].Loses = 0;
            teamInfo[j].MembersInfo[y].Rating = 0;
            teamInfo[j].MembersInfo[y].PlayerName = "";
        }
    }

    uint32 pos = 0;
    do
    {
        Field *fields = result->Fetch();

        teamInfo[pos].Id = fields[0].GetUInt32();
        teamInfo[pos].TeamName = fields[1].GetCppString();
        teamInfo[pos].Wins = fields[2].GetUInt32();
        teamInfo[pos].Loses = fields[3].GetUInt32();
        teamInfo[pos].Rating = fields[4].GetUInt32();

        RealmDataDatabase.AsyncPQuery(&UpdateLadderTeamMembersCallback, arena_type + (pos << 16),
            "select a.name, a.race, a.class, b.wons_season, (b.played_season - b.wons_season), b.personal_rating from characters a, arena_team_member b "
            "where a.guid = b.guid and b.arenateamid = %u order by b.personal_rating LIMIT %u", teamInfo[pos].Id, arena_type * 2);

        ++pos;
    } while (result->NextRow());
}

void BattleGroundMgr::UpdateLadder()
{
    RealmDataDatabase.AsyncPQuery(&UpdateLadderTeamsCallback, (uint32)ARENA_TYPE_2v2,
        "select a.arenateamid, a.name, b.wins2,(b.played-b.wins2),b.rating from arena_team a, arena_team_stats b where a.type=%u "
        "and a.arenateamid=b.arenateamid order by b.rating desc limit %u", (uint32)ARENA_TYPE_2v2, LADDER_CNT);

    RealmDataDatabase.AsyncPQuery(&UpdateLadderTeamsCallback, (uint32)ARENA_TYPE_3v3,
        "select a.arenateamid, a.name, b.wins2,(b.played-b.wins2),b.rating from arena_team a, arena_team_stats b where a.type=%u "
        "and a.arenateamid=b.arenateamid order by b.rating desc limit %u", (uint32)ARENA_TYPE_3v3, LADDER_CNT);

	RealmDataDatabase.AsyncPQuery(&UpdateLadderTeamsCallback, (uint32)ARENA_TYPE_5v5,
		"select a.arenateamid, a.name, b.wins2,(b.played-b.wins2),b.rating from arena_team a, arena_team_stats b where a.type=%u "
		"and a.arenateamid=b.arenateamid order by b.rating desc limit %u", (uint32)ARENA_TYPE_5v5, LADDER_CNT);
}