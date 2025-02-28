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

#include "Creature.h"
#include "CreatureGroups.h"
#include "ObjectMgr.h"
//#include "ProgressBar.h"
#include "CreatureAI.h"

#define MAX_DESYNC 5.0f

FormationMemberInfoMapType   FormationMemberInfoMap; // info about members
FormationGroupInfoMapType   FormationGroupInfoMap; // info about groups
uint32 nextTempGroupId;

void CreatureGroupManager::AddCreatureToGroupIfNeeded(Creature *member)
{
    uint32 lowGuid = member->GetDBTableGUIDLow();
    if (!lowGuid)
        return;

    FormationMemberInfoMapType::iterator frmdata = FormationMemberInfoMap.find(lowGuid); // if no DB guid - nothing will be found
    if (frmdata == FormationMemberInfoMap.end())
        return;

    Map *map = member->GetMap();
    if (!map)
        return;

    CreatureGroupHolderType::iterator itr = map->CreatureGroupHolder.find(frmdata->second->groupID);

    //Add member to an existing group
    if (itr != map->CreatureGroupHolder.end())
    {
        sLog.outDebug("Group found: %u, inserting creature GUID: %u, Group InstanceID %u", frmdata->second->groupID, member->GetGUIDLow(), member->GetAnyInstanceId());
        itr->second->AddMember(member, frmdata->second);
    }
    //Create new group
    else
    {
        FormationGroupInfoMapType::iterator itr = FormationGroupInfoMap.find(frmdata->second->groupID);
        if (itr != FormationGroupInfoMap.end())
        {
            sLog.outDebug("Group not found: %u. Creating new DB group.", frmdata->second->groupID);
            CreatureGroup* group = new CreatureGroup(itr->second);
            map->CreatureGroupHolder[frmdata->second->groupID] = group;
            group->AddMember(member, frmdata->second);
        }
    }
}

void CreatureGroupManager::LeaveGroupIfHas(Creature *member)
{
    CreatureGroup* group = member->GetFormation();
    if (!group)
        return;
    sLog.outDebug("Deleting member pointer to GUID: %u from group %u", group->GetId(), member->GetDBTableGUIDLow());
    group->RemoveMember(member);

    if (group->isEmpty())
    {
        Map *map = member->GetMap();
        if (!map)
            return;

        sLog.outDebug("Deleting group with InstanceID %u", member->GetAnyInstanceId());
        map->CreatureGroupHolder.erase(group->GetId());
        delete group;
    }
}

void CreatureGroupManager::LoadCreatureFormations()
{
    //Clear existing map
    FormationGroupInfoMap.clear();
    FormationMemberInfoMap.clear();

    //Get group data
    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT `groupID`, `leaderGUID`, `flagAI` FROM `creature_formation_group` ORDER BY `groupID` ASC");

    if (!result)
    {
        sLog.outLog(LOG_DB_ERR, "The table `creature_formation_group` is empty or corrupted");
        return;
    }

    uint32 total_records = result->GetRowCount();
    //BarGoLink bar(total_records);
    Field *fields;

    FormationGroupInfo *group_info;
    //Loading data...
    do
    {
        fields = result->Fetch();

        //bar.step();
        //Load group member data
        group_info = new FormationGroupInfo;
        group_info->m_groupId = fields[0].GetUInt32();
        group_info->m_leaderGUID = fields[1].GetUInt32(); // might be 0
        group_info->m_flagAI = fields[2].GetUInt32();
        if (group_info->m_leaderGUID && !sObjectMgr.GetCreatureData(group_info->m_leaderGUID))
        {
            sLog.outLog(LOG_DB_ERR, "Table `creature_formation_group` has an invalid record groupID %u: m_leaderGUID %u does not exist in DB.",
                group_info->m_groupId, group_info->m_leaderGUID);
            group_info->m_leaderGUID = 0;
        }

        FormationGroupInfoMap[group_info->m_groupId] = group_info;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u formation group defines", total_records);
    sLog.outString();

    nextTempGroupId = FormationGroupInfoMap.rbegin()->first + 1000; // temp group ids. formation manipulation commands look into DB for max ID

    result = GameDataDatabase.PQuery("SELECT `groupID`, `memberGUID`, `dist`, `angle` FROM `creature_formation_member` ORDER BY `groupID` ASC");

    if (!result)
    {
        sLog.outLog(LOG_DB_ERR, "The table `creature_formation_member` is empty or corrupted");
        return;
    }

    total_records = result->GetRowCount();
    //BarGoLink bar2(total_records);

    FormationMemberInfo *member_info;
    //Loading data...
    do
    {
        fields = result->Fetch();

        //bar2.step();
        //Load group member data
        member_info = new FormationMemberInfo;
        member_info->groupID = fields[0].GetUInt32(); // might be 0
        int32 memberGUIDorEntry = fields[1].GetInt32(); // might be negative
        member_info->follow_dist = fields[2].GetFloat(); // might be 0
        member_info->follow_angle = fields[3].GetFloat(); // might be 0
        if (member_info->groupID) // both if member is constant or temporary
        {
            FormationGroupInfoMapType::iterator grinfo = FormationGroupInfoMap.find(member_info->groupID);
            if (grinfo == FormationGroupInfoMap.end())
            {
                sLog.outLog(LOG_DB_ERR, "Table `creature_formation_member` has an invalid record memberGUID %d: no groupInfo found for groupID %u",
                    memberGUIDorEntry, member_info->groupID);
                delete member_info;
                continue;
            }
            else if (grinfo->second->m_leaderGUID == memberGUIDorEntry)
            {
                member_info->follow_dist = 0;
                member_info->follow_angle = 0;
                sLog.outLog(LOG_DB_ERR, "Table `creature_formation_member` has an invalid record memberGUID %d: he is a leader and has follow info, should not.",
                    memberGUIDorEntry, group_info->m_leaderGUID);
            }

            if (memberGUIDorEntry > 0) // member is constant
            {
                // group exists at the moment, we can use grinfo variable, however there might be no leader
                if (grinfo->second->m_leaderGUID) // there must be leader
                {
                    const CreatureData* leader = sObjectMgr.GetCreatureData(grinfo->second->m_leaderGUID);
                    const CreatureData* member = sObjectMgr.GetCreatureData(memberGUIDorEntry);
                    if (!leader || !member || leader->mapid != member->mapid)
                    {
                        sLog.outLog(LOG_DB_ERR, "Table `creature_formation_member` has an invalid record memberGUID %d groupID %u. No leader or member found or their maps are different.",
                            memberGUIDorEntry, member_info->groupID);
                        delete member_info;
                        continue;
                    }
                }
                else // no leader, but member should still exist
                {
                    const CreatureData* member = sObjectMgr.GetCreatureData(memberGUIDorEntry);
                    if (!member)
                    {
                        sLog.outLog(LOG_DB_ERR, "Table `creature_formation_member` has an invalid record memberGUID %d. Member not found.",
                            memberGUIDorEntry, member_info->groupID);
                        delete member_info;
                        continue;
                    }
                }
            }
        }

        FormationMemberInfoMap[memberGUIDorEntry] = member_info;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u formation member defines", total_records);
    sLog.outString();
}

bool CreatureGroupManager::AddTempCreatureToDBgroup(Creature* member, int32 tempId)
{
    if (!tempId || tempId > 0)
        return false;

    FormationMemberInfoMapType::iterator frmdata = FormationMemberInfoMap.find(tempId); // if no DB guid - nothing will be found
    if (frmdata == FormationMemberInfoMap.end())
        return false;

    Map *map = member->GetMap();
    if (!map)
        return false;

    CreatureGroupHolderType::iterator itr = map->CreatureGroupHolder.find(frmdata->second->groupID);

    //Add member to an existing group
    if (itr != map->CreatureGroupHolder.end())
    {
        sLog.outDebug("Group found: %u, inserting creature GUID: %u, Group InstanceID %u", frmdata->second->groupID, member->GetGUIDLow(), member->GetAnyInstanceId());
        itr->second->AddMember(member, frmdata->second);
    }
    //Create new group
    else
    {
        FormationGroupInfoMapType::iterator itr = FormationGroupInfoMap.find(frmdata->second->groupID);
        if (itr != FormationGroupInfoMap.end())
        {
            sLog.outDebug("Group not found: %u. Creating new DB group.", frmdata->second->groupID);
            CreatureGroup* group = new CreatureGroup(itr->second);
            map->CreatureGroupHolder[frmdata->second->groupID] = group;
            group->AddMember(member, frmdata->second);
        }
        else
            return false;
    }
    return true;
}

uint32 CreatureGroupManager::AddTempCreatureToTempGroup(uint32 groupId, Creature* member, int32 tempId)
{
    if (!tempId || tempId > 0)
        return 0;

    FormationMemberInfoMapType::iterator frmdata = FormationMemberInfoMap.find(tempId); // if no DB guid - nothing will be found
    if (frmdata == FormationMemberInfoMap.end())
        return 0;

    Map *map = member->GetMap();
    if (!map)
        return 0;

    CreatureGroupHolderType::iterator itr = map->CreatureGroupHolder.find(groupId);

    //Add member to an existing group
    if (itr != map->CreatureGroupHolder.end())
    {
        sLog.outDebug("Group found: %u, inserting creature GUID: %u, Group InstanceID %u", groupId, member->GetGUIDLow(), member->GetAnyInstanceId());
        itr->second->AddMember(member, frmdata->second);
        return groupId;
    }
    //Create new group
    else
    {
        FormationGroupInfoMapType::iterator itr = FormationGroupInfoMap.find(groupId);
        if (itr == FormationGroupInfoMap.end())
        {
            sLog.outDebug("Group not found: %u. Creating new DB group.", groupId);

            FormationGroupInfo* temp = new FormationGroupInfo;
            temp->m_groupId = nextTempGroupId;
            temp->m_leaderGUID = 0;
            temp->m_flagAI = 0;

            CreatureGroup* group = new CreatureGroup(temp, true);
            map->CreatureGroupHolder[nextTempGroupId] = group;
            group->AddMember(member, frmdata->second);
            return nextTempGroupId++;
        }
        else
            return 0;
    }
    return 0;
}

void CreatureGroup::AddMember(Creature *member, FormationMemberInfo* info)
{
    sLog.outDebug("CreatureGroup::AddMember: Adding unit GUIDLow: %u.", member->GetGUIDLow());

    //Check if it is a leader
    if (m_info->m_leaderGUID && member->GetDBTableGUIDLow() == m_info->m_leaderGUID)
    {
        sLog.outDebug("Unit GUID: %u is formation leader. Adding group.", member->GetGUIDLow());
        m_leader = member;
    }

    m_members[member->GetGUID()] = info; // might add temporary info
    member->SetFormation(this);
}

void CreatureGroup::RemoveMember(Creature *member)
{
    if (m_leader == member)
        m_leader = NULL;

    m_members.erase(member->GetGUID());
    member->SetFormation(NULL);
}

void CreatureGroup::MemberAttackStart(Creature *member, Unit *target)
{
    if (!member || !target)
        return;

    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        sLog.outDebug("GROUP ATTACK: group instance id %u calls member instid %u", m_leader ? m_leader->GetAnyInstanceId() : 0, member->GetAnyInstanceId());
        //sLog.outDebug("Group member found: guid %u, entry %u, attacked by %s.", member->GetGUID(), member->GetEntry(), itr->second, member->GetVictim()->GetName());

        //Skip one check
        if (itr->first == member->GetGUID())
            continue;

        if (Creature *mem = member->GetMap()->GetCreature(itr->first))
        {
            if (!mem->isAlive())
                continue;

            if (mem->GetVictim())
                continue;

			if (mem->isCivilian() || mem->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE))
				continue;

			if (!mem->canAttack(target))
				continue;

			mem->AI()->AttackStart(target);
        }
    }

    m_Respawned = false;
    m_Evaded = false;
}

void CreatureGroup::FormationReset()
{
    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (itr->first != m_leader->GetGUID())
        {
            if (Creature *mem = m_leader->GetMap()->GetCreature(itr->first))
            {
                // in most cases they should all be dead or in combat, cause formation starts attack together
                if (!mem->isAlive() || mem->IsInCombat())
                    continue;

                // not following leader at the moment
                if (mem->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                    continue;

                // stop from following leader and reinitialize movement
                mem->GetMotionMaster()->Initialize();

                sLog.outDebug("Set init movement for member GUID: %u", mem->GetGUIDLow());
            }
        }
    }
}

void CreatureGroup::RespawnFormation(Creature *member)
{
    if(!member || m_Respawned)
        return;

    m_Respawned = true;

    if (Map* map = member->GetMap())
    {
        for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        {
            // Called at EnterEvadeMode, do not check self
            if (itr->first == member->GetGUID())
                continue;

            if (Creature* mem = map->GetCreature(itr->first))
            {
                if (mem->isAlive())
                    continue;

                if (map->IsDungeon() && (map->IsRaid() || map->IsHeroic()))
                    if (mem->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
                        continue;

                mem->Respawn();
            }
        }
    }
}

void CreatureGroup::EvadeFormation(Creature *member)
{
    if (!member || m_Evaded)
        return;

    m_Evaded = true;
    RespawnFormation(member);

    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        // Called at EnterEvadeMode, do not check self
        if (itr->first == member->GetGUID())
            continue;

        if (Creature *mem = member->GetMap()->GetCreature(itr->first))
            mem->AI()->EnterEvadeMode();
    }
}

void CreatureGroup::LeaderMoveTo(float x, float y, float z, bool turnAround)
{
    m_movingUnits = 0;
    if (!m_leader)
        return;

    float pathOrient = m_leader->GetOrientationTo(x, y); // old angle was wrong!

    if (turnAround) // more than 180 degrees
        m_turnAround = !m_turnAround;

    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        Creature *member = m_leader->GetMap()->GetCreature(itr->first);
        if (!member || member == m_leader || !member->isAlive() || member->GetVictim())
            continue;

        // It is 0 if member is in front of leader. M_PI/2 if member is left from the leader
        float angle = m_turnAround ? float(M_PI) * 2 - itr->second->follow_angle : itr->second->follow_angle;
        float dist = itr->second->follow_dist;

        // if follow distance is 0 then don't follow leader
        if (!dist)
            continue;

        float dx = x + cos(angle + pathOrient) * dist;
        float dy = y + sin(angle + pathOrient) * dist;
        float dz = z;

        Hellground::NormalizeMapCoord(dx);
        Hellground::NormalizeMapCoord(dy);

        member->UpdateGroundPositionZ(dx, dy, dz);

        if (member->IsWithinDistInMap(m_leader, dist + MAX_DESYNC))
        {
            uint32 moveFlags = m_leader->GetUnitMovementFlags();
            if (!member->m_movementInfo.HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
                moveFlags &= ~MOVEFLAG_SPLINE_ENABLED;

            member->SetUnitMovementFlags(moveFlags);
        }
        else // this actually should not happen, cause leader is always waiting for everyone to reach their point before moving again
            // this can only happen if member couldn't reached destination was too far away.
        {
            // jak sie za bardzo rozjada xO
            if (!member->IsWithinDistInMap(m_leader, 40.0f))
                member->NearTeleportTo(m_leader->GetPositionX(), m_leader->GetPositionY(), m_leader->GetPositionZ(), 0.0f);
            else
                member->SetWalk(false);
        }

        member->GetMotionMaster()->MovePoint(0, dx, dy, dz, true, true, UNIT_ACTION_HOME); // Trentone fixme: same as waypoints, those depend on waypoints and must not work via movemaps, but via straight go to point
        member->SetHomePosition(m_leader->GetPositionX(), m_leader->GetPositionY(), m_leader->GetPositionZ(), pathOrient);
        m_movingUnits++;
    }
}

Creature* CreatureGroup::GetNextRandomCreatureGroupMember(Creature* member, float radius)
{
    std::vector<Creature*> nearMembers;
    nearMembers.reserve(m_members.size()*2);

    for (CreatureGroupMemberType::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        if (Creature *mem = member->GetMap()->GetCreature(itr->first))
        {
            // IsHostileTo check controlled by enemy
            if (itr->first != member->GetGUID() && member->IsWithinDistInMap(mem, radius)
            && !member->IsHostileTo(mem) && mem->isAlive() && !mem->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
                nearMembers.push_back(mem);
        }
    }

    if (nearMembers.empty())
        return NULL;

    uint32 randTarget = urand(0,nearMembers.size()-1);
    return nearMembers[randTarget];
}
