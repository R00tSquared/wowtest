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

#ifndef HELLGROUND_CREATUREGROUPS_H
#define HELLGROUND_CREATUREGROUPS_H

#include "Common.h"

class CreatureGroup;

struct FormationMemberInfo
{
    uint32 groupID; // might be 0 if group is temp
    float follow_dist;
    float follow_angle;
};

struct FormationGroupInfo
{
    uint32 m_groupId; // CANNOT be 0!
    uint32 m_leaderGUID; // might be 0 if has no leader or leader is assigned via script
    uint32 m_flagAI; // How this group behaves
};

class HELLGROUND_IMPORT_EXPORT CreatureGroupManager
{
public:
    static void AddCreatureToGroupIfNeeded(Creature *member); // if group exists -> adds to group. If not -> creates group. Used for DB info groups/members
    static void LeaveGroupIfHas(Creature *member);
    static void LoadCreatureFormations();

    // used in scripts
    static bool AddTempCreatureToDBgroup(Creature* member, int32 tempId); // if group exists -> adds to group. If not - creates group. If no info found or info has 0 group -> returns false
    static uint32 AddTempCreatureToTempGroup(uint32 groupId, Creature* member, int32 tempId); // if group exists -> adds to group. If not - creates group. If successful returns group ID where creature was added
};

typedef std::map<int32/*memberDBGUID or negative temp entry*/, FormationMemberInfo*> FormationMemberInfoMapType;
typedef std::map<uint32/*groupID*/, FormationGroupInfo*> FormationGroupInfoMapType;

extern FormationMemberInfoMapType FormationMemberInfoMap; // so it can be used in Creature

typedef std::map<uint64, FormationMemberInfo *>  CreatureGroupMemberType;

class HELLGROUND_IMPORT_EXPORT CreatureGroup
{
    // those are used for managing groups
    friend class CreatureGroupManager;
    private:
        // loaded variables
        FormationGroupInfo* m_info;

        // internal variables
        Creature *m_leader; // assigned if added member GUID is leaderGUID
        CreatureGroupMemberType m_members;
        uint32 m_movingUnits;
        bool m_Respawned;
        bool m_Evaded;
        bool m_IsTemp;
        bool m_turnAround;

        // There are three ways of creating formations:
        // 1. Both members and group info are in DB. Members are constant and use positive GUID numbers. GroupID is defined. Group info is defined.
        // With this groups are searched automatically.
        // 2. Both members and group info are in DB. Members are temporary and use negative GUID numbers (ID's of info to search). GroupID is defined. Group info is defined.
        // With this you need to call AddTempCreatureToDBgroup(Creature *member, int32 tempID) with the negative tempID
        // 3. Only members info is in DB. Members are temporary and use negative GUID numbers (ID's of info to search). GroupID is 0. No group info.
        // Call AddTempCreatureToTempGroup(uint32 groupId, Creature *member, int32 tempID) with negative tempID to add members to group and/or create group itself
        
        //Group cannot be created empty
        explicit CreatureGroup(FormationGroupInfo* info, bool isTemp = false) :
            m_leader(NULL), m_movingUnits(0), m_Respawned(false), m_Evaded(false), m_turnAround(false), m_info(info), m_IsTemp(isTemp) {}

        ~CreatureGroup() 
        {
            if (m_IsTemp) 
                delete m_info;
        }

        bool isEmpty() const { return m_members.empty(); }

        void AddMember(Creature *member, FormationMemberInfo* info);
        void RemoveMember(Creature *member);
        void RespawnFormation(Creature *member); // called from EvadeFormation

    public:
        uint32 GetId() { return m_info->m_groupId; }

        Creature* getLeader() const { return m_leader; }

        // these two used from scripts
        void SetAI(uint32 ai) { m_info->m_flagAI = ai; };
        void SetLeader(Creature* leader) { m_leader = leader; };

        // used to respawn and evade whole formation
        void EvadeFormation(Creature *member);

        void FormationReset(); // reinitializes member movement if leader dies and member is following leader

        void LeaderMoveTo(float x, float y, float z, bool turnAround);
        void MemberAttackStart(Creature* member, Unit *target);
        Creature* GetNextRandomCreatureGroupMember(Creature* member, float radius);

        void ReachedWaypoint() {  if( m_movingUnits > 0 ) m_movingUnits--; }
        void ClearMovingUnits() { m_movingUnits = 0; }
        bool AllUnitsReachedWaypoint() const { return m_movingUnits == 0; }

        /*used to show formation members ingame*/
        CreatureGroupMemberType const& GetMembers() { return m_members; }
};

#endif
