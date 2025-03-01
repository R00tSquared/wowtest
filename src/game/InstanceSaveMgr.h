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

#ifndef HELLGROUND_INSTANCESAVEMGR_H
#define HELLGROUND_INSTANCESAVEMGR_H

#include "Platform/Define.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include <list>
#include <map>
#include "Utilities/UnorderedMap.h"
#include "Database/DatabaseEnv.h"
#include "SharedDefines.h"

struct InstanceTemplate;
struct MapEntry;
class Player;
class Group;

/*
    Holds the information necessary for creating a new map for an existing instance
    Is referenced in three cases:
    - player-instance binds for solo players (not in group)
    - player-instance binds for permanent heroic/raid saves
    - group-instance binds (both solo and permanent) cache the player binds for the group leader
*/
class InstanceSave
{
    friend class InstanceSaveManager;
    public:
        /* Created either when:
           - any new instance is being generated
           - the first time a player bound to InstanceId logs in
           - when a group bound to the instance is loaded */
        InstanceSave(uint16 MapId, uint32 InstanceId, uint8 difficulty, time_t resetTime, bool canReset);

        /* Unloaded when m_playerList and m_groupList become empty
           or when the instance is reset */
        ~InstanceSave();

        uint8 GetPlayerCount() { return m_playerList.size(); }
        uint8 GetGroupCount() { return m_groupList.size(); }

        /* A map corresponding to the InstanceId/MapId does not always exist.
        InstanceSave objects may be created on player logon but the maps are
        created and loaded only when a player actually enters the instance. */
        uint32 GetSaveInstanceId() { return m_instanceid; }
        uint32 GetMapId() { return m_mapid; }

        /* Saved when the instance is generated for the first time */
        void SaveToDB();
        /* When the instance is being reset (permanently deleted) */
        void DeleteFromDB();

        /* for normal instances this corresponds to max(creature respawn time) + X hours
           for raid/heroic instances this caches the global respawn time for the map */
        time_t GetResetTime() { return m_resetTime; }
        void SetResetTime(time_t resetTime) { m_resetTime = resetTime; }
        time_t GetResetTimeForDB();

        InstanceTemplate const* GetTemplate();
        MapEntry const* GetMapEntry();

        bool HasPlayer(uint64 guid);

        /* online players bound to the instance (perm/solo)
           does not include the members of the group unless they have permanent saves */
        void AddPlayer(uint64 guid) { m_playerList.push_back(guid); }
        bool RemovePlayer(uint64 guid) { m_playerList.remove(guid); return UnloadIfEmpty(); }
        /* all groups bound to the instance */
        void AddGroup(Group *group) { m_groupList.push_back(group); }
        bool RemoveGroup(Group *group) { m_groupList.remove(group); return UnloadIfEmpty(); }

        /* instances cannot be reset (except at the global reset time)
           if there are players permanently bound to it
           this is cached for the case when those players are offline */
        bool CanReset() { return m_canReset; }
        void SetCanReset(bool canReset) { m_canReset = canReset; }

        /* currently it is possible to omit this information from this structure
           but that would depend on a lot of things that can easily change in future */
        uint8 GetDifficulty() { return m_difficulty; }
        bool isHeroic() { return m_difficulty == DIFFICULTY_HEROIC; }

        typedef std::list<uint64> PlayerListType;
        typedef std::list<Group*> GroupListType;
    private:
        bool UnloadIfEmpty();
        /* the only reason the instSave-object links are kept is because
           the object-instSave links need to be broken at reset time
           TODO: maybe it's enough to just store the number of players/groups */
        PlayerListType m_playerList;
        GroupListType m_groupList;
        time_t m_resetTime;
        uint32 m_instanceid;
        uint16 m_mapid;
        uint8 m_difficulty;
        bool m_canReset;
};

class InstanceSaveManager
{
    friend class ACE_Singleton<InstanceSaveManager, ACE_Thread_Mutex>;
    InstanceSaveManager();

    friend class InstanceSave;
    public:
        ~InstanceSaveManager();

        typedef std::map<uint32 /*InstanceId*/, InstanceSave*> InstanceSaveMap;
        typedef UNORDERED_MAP<uint32 /*InstanceId*/, InstanceSave*> InstanceSaveHashMap;
        typedef std::map<uint32 /*mapId*/, InstanceSaveMap> InstanceSaveMapMap;

        /* resetTime is a global propery of each (raid/heroic) map
           all instances of that map reset at the same time */
        struct InstResetEvent
        {
            uint8 type;
            uint16 mapid;
            uint16 instanceId;
            bool heroic;
            InstResetEvent(uint8 t, uint16 m, uint16 i, bool h) : type(t), mapid(m), instanceId(i), heroic(h) {}
            bool operator == (const InstResetEvent& e) { return e.instanceId == instanceId; }
        };
        typedef std::multimap<time_t /*resetTime*/, InstResetEvent> ResetTimeQueue;
        typedef std::map<std::pair<uint32/*mapId*/, bool/*heroic*/>, time_t /*resetTime*/> ResetTimeMap;

        void CleanupInstances();
        void PackInstances();

        void LoadResetTimes();
        time_t GetResetTimefor (uint32 mapid, bool heroic) { return m_resetTimeByMapId[std::make_pair(mapid, heroic)]; }
        void ScheduleReset(bool add, time_t time, InstResetEvent event);

        void Update();

        InstanceSave* AddInstanceSave(uint32 mapId, uint32 instanceId, uint8 difficulty, time_t resetTime, bool canReset, bool load = false);
        void RemoveInstanceSave(uint32 InstanceId);
        static void DeleteInstanceFromDB(uint32 instanceid);

        InstanceSave *GetInstanceSave(uint32 InstanceId);

        /* statistics */
        uint32 GetNumInstanceSaves() { return m_instanceSaveById.size(); }
        uint32 GetNumBoundPlayersTotal();
        uint32 GetNumBoundGroupsTotal();

    private:
        void _ResetOrWarnAll(uint32 mapid, bool warn, uint32 timeleft, bool heroic);
        void _ResetInstance(uint32 mapid, uint32 instanceId);
        void _ResetSave(InstanceSaveHashMap::iterator &itr);
        void _DelHelper(DatabaseType &db, bool distinc, const char *fields, const char *table, const char *queryTail, ...);
        // used during global instance resets
        bool lock_instLists;
        // fast lookup by instance id
        InstanceSaveHashMap m_instanceSaveById;
        // fast lookup for reset times
        ResetTimeMap m_resetTimeByMapId;
        ResetTimeQueue m_resetTimeQueue;
};

#define sInstanceSaveManager (*ACE_Singleton<InstanceSaveManager, ACE_Thread_Mutex>::instance())
#endif
