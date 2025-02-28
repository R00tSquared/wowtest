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

#include "InstanceSaveMgr.h"
#include "Common.h"
#include "Database/SQLStorage.h"

#include "Player.h"
#include "GridNotifiers.h"
#include "WorldSession.h"
#include "Log.h"
#include "GridStates.h"
#include "CellImpl.h"
#include "Map.h"
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "Timer.h"
#include "GridNotifiersImpl.h"
#include "Transports.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Group.h"
#include "InstanceData.h"
//#include "ProgressBar.h"
#include "GameEvent.h"

InstanceSaveManager::InstanceSaveManager() : lock_instLists(false)
{

}

InstanceSaveManager::~InstanceSaveManager()
{
}
/*
- adding instance into manager
- called from InstanceMap::Add, _LoadBoundInstances, LoadGroups
*/
InstanceSave* InstanceSaveManager::AddInstanceSave(uint32 mapId, uint32 instanceId, uint8 difficulty, time_t resetTime, bool canReset, bool load)
{
    InstanceSave *save = GetInstanceSave(instanceId);
    if (save) return save;

    const MapEntry* entry = sMapStore.LookupEntry(mapId);
    if (!entry || instanceId == 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: InstanceSaveManager::AddInstanceSave: mapid = %d, instanceid = %d!", mapId, instanceId);
        return NULL;
    }

    if (!resetTime)
    {
        // initialize reset time
        // for normal instances if no creatures are killed the instance will reset in two hours
        if (entry->map_type == MAP_RAID || difficulty == DIFFICULTY_HEROIC)
            resetTime = GetResetTimefor (mapId, difficulty == DIFFICULTY_HEROIC);
        else
        {
            resetTime = time(NULL) + 2 * HOUR;
            // normally this will be removed soon after in InstanceMap::Add, prevent error
            ScheduleReset(true, resetTime, InstResetEvent(0, mapId, instanceId, false)); // it is 100% not heroic
        }
    }

    sLog.outDebug("InstanceSaveManager::AddInstanceSave: mapid = %d, instanceid = %d", mapId, instanceId);

    save = new InstanceSave(mapId, instanceId, difficulty, resetTime, canReset);
    if (!load)
        save->SaveToDB();

    m_instanceSaveById[instanceId] = save;
    return save;
}

InstanceSave *InstanceSaveManager::GetInstanceSave(uint32 InstanceId)
{
    InstanceSaveHashMap::iterator itr = m_instanceSaveById.find(InstanceId);
    return itr != m_instanceSaveById.end() ? itr->second : NULL;
}

void InstanceSaveManager::DeleteInstanceFromDB(uint32 instanceid)
{
    RealmDataDatabase.BeginTransaction();
    RealmDataDatabase.PExecute("DELETE FROM instance WHERE id = '%u'", instanceid);
    RealmDataDatabase.PExecute("DELETE FROM group_saved_loot WHERE instanceId='%u'", instanceid);
    RealmDataDatabase.PExecute("DELETE FROM character_instance WHERE instance = '%u'", instanceid);
    RealmDataDatabase.PExecute("DELETE FROM group_instance WHERE instance = '%u'", instanceid);
    RealmDataDatabase.CommitTransaction();
    // respawn times should be deleted only when the map gets unloaded
}

void InstanceSaveManager::RemoveInstanceSave(uint32 InstanceId)
{
    InstanceSaveHashMap::iterator itr = m_instanceSaveById.find(InstanceId);
    if (itr != m_instanceSaveById.end())
    {
        // save the resettime for normal instances only when they get unloaded
        if (time_t resettime = itr->second->GetResetTimeForDB())
            RealmDataDatabase.PExecute("UPDATE instance SET resettime = '%llu' WHERE id = '%u'", (uint64)resettime, InstanceId);

        InstanceSave *temp = itr->second;
        m_instanceSaveById.erase(itr);
        delete temp;
    }
}

InstanceSave::InstanceSave(uint16 MapId, uint32 InstanceId, uint8 difficulty,
                           time_t resetTime, bool canReset)
: m_mapid(MapId), m_instanceid(InstanceId), m_resetTime(resetTime),
  m_difficulty(difficulty), m_canReset(canReset)
{
}

InstanceSave::~InstanceSave()
{
    // the players and groups must be unbound before deleting the save
    ASSERT(m_playerList.empty() && m_groupList.empty());
}

/*
    Called from AddInstanceSave
*/
void InstanceSave::SaveToDB()
{
    // save instance data too
    std::string data;

    Map *map = sMapMgr.FindMap(GetMapId(), m_instanceid);
    if (map)
    {
        ASSERT(map->IsDungeon());
        if (InstanceData *iData = ((InstanceMap*)map)->GetInstanceData())
        {
            data = iData->GetSaveData();
            if (!data.empty())
                RealmDataDatabase.escape_string(data);
        }
    }

    static SqlStatementID insertInstance;

    SqlStatement stmt = RealmDataDatabase.CreateStatement(insertInstance, "INSERT INTO instance VALUES (?, ?, ?, ?, 0, ?)");
    stmt.addUInt32(m_instanceid);
    stmt.addUInt32(GetMapId());
    stmt.addUInt64(uint64(GetResetTimeForDB()));
    stmt.addUInt8(GetDifficulty());
    stmt.addString(data);
    stmt.Execute();
}

time_t InstanceSave::GetResetTimeForDB()
{
    // only save the reset time for normal instances
    const MapEntry *entry = sMapStore.LookupEntry(GetMapId());
    if (!entry || entry->map_type == MAP_RAID || GetDifficulty() == DIFFICULTY_HEROIC)
        return 0;
    else
        return GetResetTime();
}

bool InstanceSave::HasPlayer(uint64 guid)
{
    return std::any_of(m_playerList.begin(), m_playerList.end(), [guid](uint64 p) { return p == guid; });
}

// to cache or not to cache, that is the question
InstanceTemplate const* InstanceSave::GetTemplate()
{
    return ObjectMgr::GetInstanceTemplate(m_mapid);
}

MapEntry const* InstanceSave::GetMapEntry()
{
    return sMapStore.LookupEntry(m_mapid);
}

void InstanceSave::DeleteFromDB()
{
    InstanceSaveManager::DeleteInstanceFromDB(GetSaveInstanceId());
}

/* true if the instance save is still valid */
bool InstanceSave::UnloadIfEmpty()
{
    if (m_playerList.empty() && m_groupList.empty())
    {
        if (!sInstanceSaveManager.lock_instLists)
            sInstanceSaveManager.RemoveInstanceSave(GetSaveInstanceId());

        return false;
    }
    else
        return true;
}

void InstanceSaveManager::_DelHelper(DatabaseType &db, bool distinct, const char *fields, const char *table, const char *queryTail, ...)
{
    Tokens fieldTokens = StrSplit(fields, ", ");
    ASSERT(fieldTokens.size() != 0);

    va_list ap;
    char szQueryTail [MAX_QUERY_LEN];
    va_start(ap, queryTail);
    int res = vsnprintf(szQueryTail, MAX_QUERY_LEN, queryTail, ap);
    va_end(ap);

    QueryResultAutoPtr result;
    if (distinct)
        result = db.PQuery("SELECT DISTINCT %s FROM %s %s", fields, table, szQueryTail);
    else
        result = db.PQuery("SELECT %s FROM %s %s", fields, table, szQueryTail);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            std::ostringstream ss;
            for (size_t i = 0; i < fieldTokens.size(); i++)
            {
                std::string fieldValue = fields[i].GetCppString();
                db.escape_string(fieldValue);
                ss << (i != 0 ? " AND " : "") << fieldTokens[i] << " = '" << fieldValue << "'";
            }
            db.PExecute("DELETE FROM %s WHERE %s", table, ss.str().c_str());
        } while (result->NextRow());
    }
}

void InstanceSaveManager::CleanupInstances()
{    
    uint64 now = (uint64)time(NULL);

    //BarGoLink bar(2);
    //bar.step();

    // load reset times and clean expired instances
    sInstanceSaveManager.LoadResetTimes();

    RealmDataDatabase.BeginTransaction();
    // clean character/group - instance binds with invalid group/characters
    _DelHelper(RealmDataDatabase, false, "character_instance.guid, instance", "character_instance", "LEFT JOIN characters ON character_instance.guid = characters.guid WHERE characters.guid IS NULL");
    _DelHelper(RealmDataDatabase, false, "group_instance.leaderGuid, instance", "group_instance", "LEFT JOIN characters ON group_instance.leaderGuid = characters.guid LEFT JOIN groups ON group_instance.leaderGuid = groups.leaderGuid WHERE characters.guid IS NULL OR groups.leaderGuid IS NULL");

    // clean instances that do not have any players or groups bound to them
    _DelHelper(RealmDataDatabase, false,  "id, map, difficulty", "instance", "LEFT JOIN character_instance ON character_instance.instance = id LEFT JOIN group_instance ON group_instance.instance = id WHERE character_instance.instance IS NULL AND group_instance.instance IS NULL");

    // clean invalid instance references in other tables
    _DelHelper(RealmDataDatabase, false, "character_instance.guid, instance", "character_instance", "LEFT JOIN instance ON character_instance.instance = instance.id WHERE instance.id IS NULL");
    _DelHelper(RealmDataDatabase, false, "group_instance.leaderGuid, instance", "group_instance", "LEFT JOIN instance ON group_instance.instance = instance.id WHERE instance.id IS NULL");
    
    _DelHelper(RealmDataDatabase, true, "group_saved_loot.instanceId", "group_saved_loot", "LEFT JOIN instance ON group_saved_loot.instanceId = instance.id WHERE instance.id IS NULL");
    _DelHelper(RealmDataDatabase, true, "boss_fights_evades.instance_id", "boss_fights_evades", "LEFT JOIN instance ON boss_fights_evades.instance_id = instance.id WHERE instance.id IS NULL");

    // clean creature/gameobject respawn times
    RealmDataDatabase.DirectExecute("DELETE FROM creature_respawn WHERE creature_respawn.instance != 0 AND NOT EXISTS (SELECT id FROM instance WHERE instance.id = creature_respawn.instance)");
    RealmDataDatabase.DirectExecute("DELETE FROM gameobject_respawn WHERE gameobject_respawn.instance != 0 AND NOT EXISTS (SELECT id FROM instance WHERE instance.id = gameobject_respawn.instance)");

    RealmDataDatabase.CommitTransaction();
    //bar.step();
    sLog.outString();
    sLog.outString(">> Instances cleaned up.");
}

void InstanceSaveManager::PackInstances()
{
    // this routine renumbers player instance associations in such a way so they start from 1 and go up
    // TODO: this can be done a LOT more efficiently

    // obtain set of all associations
    std::set< uint32 > InstanceSet;

    // all valid ids are in the instance table
    // any associations to ids not in this table are assumed to be
    // cleaned already in CleanupInstances
    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT id FROM instance");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            InstanceSet.insert(fields[0].GetUInt32());
        }
        while (result->NextRow());
    }

    //BarGoLink bar(InstanceSet.size() + 1);
    //bar.step();

    uint32 InstanceNumber = 1;
    // we do assume std::set is sorted properly on integer value
    for (std::set< uint32 >::iterator i = InstanceSet.begin(); i != InstanceSet.end(); ++i)
    {
        if (*i != InstanceNumber)
        {
            // remap instance id
            RealmDataDatabase.BeginTransaction();
            RealmDataDatabase.PExecute("UPDATE creature_respawn SET instance = '%u' WHERE instance = '%u'", InstanceNumber, *i);
            RealmDataDatabase.PExecute("UPDATE gameobject_respawn SET instance = '%u' WHERE instance = '%u'", InstanceNumber, *i);
            RealmDataDatabase.PExecute("UPDATE corpse SET instance = '%u' WHERE instance = '%u'", InstanceNumber, *i);
            RealmDataDatabase.PExecute("UPDATE character_instance SET instance = '%u' WHERE instance = '%u'", InstanceNumber, *i);
            RealmDataDatabase.PExecute("UPDATE instance SET id = '%u' WHERE id = '%u'", InstanceNumber, *i);
            RealmDataDatabase.PExecute("UPDATE group_instance SET instance = '%u' WHERE instance = '%u'", InstanceNumber, *i);
            RealmDataDatabase.PExecute("UPDATE group_saved_loot SET instanceId = '%u' WHERE instanceId = '%u'", InstanceNumber, *i);
            RealmDataDatabase.PExecute("UPDATE characters SET instance_id = '%u' WHERE instance_id = '%u'", InstanceNumber, *i);
            RealmDataDatabase.CommitTransaction();
        }

        ++InstanceNumber;
        //bar.step();
    }

    sLog.outString();
    sLog.outString(">> Instance numbers remapped, next instance id is %u", InstanceNumber);
}

void InstanceSaveManager::LoadResetTimes()
{
    time_t now = time(NULL);
    time_t today = (now / DAY) * DAY;

    // NOTE: Use DirectPExecute for tables that will be queried later

    // get the current reset times for normal instances (these may need to be updated)
    // these are only kept in memory for InstanceSaves that are loaded later
    // resettime = 0 in the DB for raid/heroic instances so those are skipped
    typedef std::map<uint32, std::pair<std::pair<uint32, bool>, uint64> > ResetTimeMapType;
    ResetTimeMapType InstResetTime;
    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT id, map, difficulty, resettime FROM instance WHERE resettime > 0");
    if (result)
    {
        do
        {
            if (uint64 resettime = (*result)[3].GetUInt64())
            {
                uint32 id = (*result)[0].GetUInt32();
                uint32 mapid = (*result)[1].GetUInt32();
                bool heroic = (*result)[2].GetBool();
                InstResetTime[id] = std::make_pair(std::make_pair(mapid, heroic), resettime);
            }
        } while (result->NextRow());

        // update reset time for normal instances with the max creature respawn time + X hours
        result = RealmDataDatabase.Query("SELECT MAX(respawntime), instance FROM creature_respawn WHERE instance > 0 GROUP BY instance");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 instance = fields[1].GetUInt32();
                uint64 resettime = fields[0].GetUInt64() + 2 * HOUR;
                ResetTimeMapType::iterator itr = InstResetTime.find(instance);
                if (itr != InstResetTime.end() && itr->second.second != resettime)
                {
                    RealmDataDatabase.DirectPExecute("UPDATE instance SET resettime = '%llu' WHERE id = '%u'", resettime, instance);
                    itr->second.second = resettime;
                }
            } while (result->NextRow());
        }

        // schedule the reset times
        for (ResetTimeMapType::iterator itr = InstResetTime.begin(); itr != InstResetTime.end(); ++itr)
            if (itr->second.second > now)
                ScheduleReset(true, itr->second.second, InstResetEvent(0, itr->second.first.first/*mapId*/, itr->first/*instId*/, itr->second.first.second/*heroic*/));
    }

    // load the global respawn times for raid/heroic instances
    uint32 diff = sWorld.getConfig(CONFIG_INSTANCE_RESET_TIME_HOUR) * HOUR;
    result = RealmDataDatabase.Query("SELECT mapid, heroic, resettime FROM instance_reset");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 mapid = fields[0].GetUInt32();
            bool heroic = fields[1].GetBool();
            InstanceTemplate const* temp = ObjectMgr::GetInstanceTemplate(mapid);
            if (!temp)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: InstanceSaveManager::LoadResetTimes: invalid mapid %u in instance_reset!", mapid);
                RealmDataDatabase.DirectPExecute("DELETE FROM instance_reset WHERE mapid = '%u'", mapid);
                continue;
            }

            // update the reset time if the hour in the configs changes
            uint64 oldresettime = fields[2].GetUInt64();
            uint64 newresettime = (oldresettime / DAY) * DAY + diff;
            if (oldresettime != newresettime)
                RealmDataDatabase.DirectPExecute("UPDATE instance_reset SET resettime = '%llu' WHERE mapid = '%u' AND heroic = '%u'", newresettime, mapid, heroic);

            m_resetTimeByMapId[std::make_pair(mapid, heroic)] = newresettime;
        } while (result->NextRow());
    }

    // clean expired instances, references to them will be deleted in CleanupInstances
    // must be done before calculating new reset times

    _DelHelper(RealmDataDatabase, false, "id, map, difficulty", "instance", "LEFT JOIN instance_reset ON mapid = map AND instance.difficulty = instance_reset.heroic WHERE (instance.resettime < '%llu' AND instance.resettime > '0') OR (NOT instance_reset.resettime IS NULL AND instance_reset.resettime < '%llu')", (uint64)now, (uint64)now);

    // calculate new global reset times for expired instances and those that have never been reset yet
    // add the global reset times to the priority queue
    for (uint32 i = 0; i < sInstanceTemplate.GetMaxEntry(); i++)
    {
        InstanceTemplate const* temp = sObjectMgr.GetInstanceTemplate(i);
        if (!temp) continue;
        // only raid/heroic maps have a global reset time
        const MapEntry* entry = sMapStore.LookupEntry(temp->map);
        if (!entry || !entry->HasResetTime())
            continue;

        for (int32 heroic = DIFFICULTY_NORMAL; heroic <= DIFFICULTY_HEROIC; ++heroic)
        {
            uint32 period = heroic ? temp->reset_delay_heroic * DAY : temp->reset_delay_raid * DAY;
            if (period == 0)
                continue;

            time_t t = m_resetTimeByMapId[std::make_pair(temp->map, bool(heroic))];
            if (!t)
            {
                // initialize the reset time
                t = today + period + diff;
                RealmDataDatabase.DirectPExecute("INSERT INTO instance_reset VALUES ('%u', '%u', '%llu')", i, heroic, (uint64)t);
            }

            if (t < now)
            {
                // assume that expired instances have already been cleaned
                // calculate the next reset time
                t = (t / DAY) * DAY;
                t += ((today - t) / period + 1) * period + diff;
                RealmDataDatabase.DirectPExecute("UPDATE instance_reset SET resettime = '%llu' WHERE mapid = '%u' AND heroic = '%u'", (uint64)t, i, heroic);
            }

            m_resetTimeByMapId[std::make_pair(temp->map, bool(heroic))] = t;

            // schedule the global reset/warning
            uint8 type = 1;
            static int tim[4] = {3600, 900, 300, 60};
            for (; type < 4; type++)
                if (t - tim[type-1] > now) break;
            ScheduleReset(true, t - tim[type-1], InstResetEvent(type, i, 0, bool(heroic)));
        }
    }
}

void InstanceSaveManager::ScheduleReset(bool add, time_t time, InstResetEvent event)
{
    if (add)
        m_resetTimeQueue.insert(std::pair<time_t, InstResetEvent>(time, event));
    else
    {
        // find the event in the queue and remove it
        ResetTimeQueue::iterator itr;
        std::pair<ResetTimeQueue::iterator, ResetTimeQueue::iterator> range;
        range = m_resetTimeQueue.equal_range(time);
        for (itr = range.first; itr != range.second; ++itr)
            if (itr->second == event) { m_resetTimeQueue.erase(itr); return; }
        // in case the reset time changed (should happen very rarely), we search the whole queue
        if (itr == range.second)
        {
            for (itr = m_resetTimeQueue.begin(); itr != m_resetTimeQueue.end(); ++itr)
                if (itr->second == event) { m_resetTimeQueue.erase(itr); return; }
            if (itr == m_resetTimeQueue.end())
                sLog.outLog(LOG_DEFAULT, "ERROR: InstanceSaveManager::ScheduleReset: cannot cancel the reset, the event(%d,%d,%d) was not found!", event.type, event.mapid, event.instanceId);
        }
    }
}

void InstanceSaveManager::Update()
{
    time_t now = time(NULL), t;
    while (!m_resetTimeQueue.empty() && (t = m_resetTimeQueue.begin()->first) < now)
    {
        InstResetEvent &event = m_resetTimeQueue.begin()->second;
        if (event.type == 0)
        {
            // for individual normal instances, max creature respawn + X hours
            _ResetInstance(event.mapid, event.instanceId);
            m_resetTimeQueue.erase(m_resetTimeQueue.begin());
        }
        else
        {
            // global reset/warning for a certain map
            time_t resetTime = GetResetTimefor (event.mapid, event.heroic);
            _ResetOrWarnAll(event.mapid, event.type != 4, resetTime - now, event.heroic);
            if (event.type != 4)
            {
                // schedule the next warning/reset
                event.type++;
                static int tim[4] = {3600, 900, 300, 60};
                ScheduleReset(true, resetTime - tim[event.type-1], event);
            }
            else
                sMapMgr.InitMaxInstanceId(); // recalculate max instance id

            m_resetTimeQueue.erase(m_resetTimeQueue.begin());
        }
    }
}

void InstanceSaveManager::_ResetSave(InstanceSaveHashMap::iterator &itr)
{
    // unbind all players bound to the instance
    // do not allow UnbindInstance to automatically unload the InstanceSaves
    lock_instLists = true;
    InstanceSave::PlayerListType &pList = itr->second->m_playerList;
    while (!pList.empty())
    {
        Player *player = ObjectAccessor::GetPlayerInWorldOrNot(*pList.begin());
        if (!player)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: _ResetSave. Player GUID:%llu is still in instance save but no longer in object accessor",*pList.begin());
            continue;
        }
        player->UnbindInstance(itr->second->GetMapId(), itr->second->GetDifficulty(), true);
    }

    InstanceSave::GroupListType &gList = itr->second->m_groupList;
    while (!gList.empty())
    {
        Group *group = *(gList.begin());
        group->UnbindInstance(itr->second->GetMapId(), itr->second->GetDifficulty(), true);
    }

    RealmDataDatabase.PExecute("DELETE FROM group_saved_loot WHERE instanceid = '%u'", itr->second->GetSaveInstanceId());

    delete itr->second;
    itr->second = nullptr;

    m_instanceSaveById.erase(itr++);

    lock_instLists = false;
}

void InstanceSaveManager::_ResetInstance(uint32 mapid, uint32 instanceId)
{
    sLog.outDebug("InstanceSaveMgr::_ResetInstance %u, %u", mapid, instanceId);

    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry || !mapEntry->Instanceable())
         return;

    InstanceSaveHashMap::iterator itr = m_instanceSaveById.find(instanceId);
    if (itr != m_instanceSaveById.end())
    {
        const MapEntry *mapEntry = sMapStore.LookupEntry(mapid);
        if (mapEntry->IsRaid())
            sLog.outLog(LOG_DEFAULT, "ERROR: Called _ResetInstance for mapid: %u, canreset: %u", mapid, itr->second->CanReset());

        _ResetSave(itr);
    }

    DeleteInstanceFromDB(instanceId);                       // even if save not loaded

    Map * iMap = sMapMgr.FindMap(mapid, instanceId);
    if (iMap && iMap->IsDungeon())
        ((InstanceMap*)iMap)->Reset(INSTANCE_RESET_RESPAWN_DELAY);
    else
        sObjectMgr.DeleteRespawnTimeForInstance(instanceId);// even if map is not loaded
}

void InstanceSaveManager::_ResetOrWarnAll(uint32 mapid, bool warn, uint32 timeLeft, bool heroic)
{
    // global reset for all instances of the given map
    // note: this isn't fast but it's meant to be executed very rarely
    MapEntry const *mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry->Instanceable())
         return;

    uint64 now = (uint64)time(NULL);

    if (!warn)
    {
        // this is called one minute before the reset time
        InstanceTemplate const* temp = sObjectMgr.GetInstanceTemplate(mapid);
        if (!temp || (heroic && !temp->reset_delay_heroic) || (!heroic && !temp->reset_delay_raid))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: InstanceSaveManager::ResetOrWarnAll: no instance template or reset delay for map %d", mapid);
            return;
        }

        // remove all binds to instances of the given map
        for (InstanceSaveHashMap::iterator itr = m_instanceSaveById.begin(); itr != m_instanceSaveById.end();)
        {
            if (itr->second->GetMapId() == mapid && (itr->second->isHeroic() == heroic))
                _ResetSave(itr);
            else
                ++itr;
        }

        // delete them from the DB, even if not loaded
        RealmDataDatabase.BeginTransaction();
        RealmDataDatabase.PExecute("DELETE FROM character_instance USING character_instance LEFT JOIN instance ON character_instance.instance = id WHERE map = '%u' AND difficulty = '%u'", mapid, heroic);
        RealmDataDatabase.PExecute("DELETE FROM group_instance USING group_instance LEFT JOIN instance ON group_instance.instance = id WHERE map = '%u' AND difficulty = '%u'", mapid, heroic);
        RealmDataDatabase.PExecute("DELETE FROM instance WHERE map = '%u' AND difficulty = '%u'", mapid, heroic);
        RealmDataDatabase.CommitTransaction();

        // calculate the next reset time
        uint32 diff = sWorld.getConfig(CONFIG_INSTANCE_RESET_TIME_HOUR) * HOUR;
        uint32 period = heroic ? temp->reset_delay_heroic * DAY : temp->reset_delay_raid * DAY;
        uint64 next_reset = ((now + timeLeft + MINUTE) / DAY * DAY) + period + diff;
        // update it in the DB
        RealmDataDatabase.PExecute("UPDATE instance_reset SET resettime = '%llu' WHERE mapid = '%d' AND heroic = '%u'", next_reset, mapid, heroic);

        // schedule next reset.
        m_resetTimeByMapId[std::make_pair(mapid, bool(heroic))] = (time_t) next_reset;
        ScheduleReset(true, (time_t) (next_reset-3600), InstResetEvent(1, mapid, 0, heroic));
        
        if (mapid == MAP_SWP)
            sGameEventMgr.SWPResetHappens();
    }

    // note: this isn't fast but it's meant to be executed very rarely
    const MapManager::MapMapType& maps = sMapMgr.Maps();

    MapManager::MapMapType::const_iterator iter_last = maps.lower_bound(MapID(mapid + 1));
    for(MapManager::MapMapType::const_iterator mitr = maps.lower_bound(MapID(mapid)); mitr != iter_last; ++mitr)
    {
        Map *map2 = mitr->second;
        if (map2->GetId() != mapid)
            break;

        if (map2->IsHeroic() != heroic)
            continue;

        if (warn)
            ((InstanceMap*)map2)->SendResetWarnings(timeLeft);
        else
            ((InstanceMap*)map2)->Reset(INSTANCE_RESET_GLOBAL);
    }

    // TODO: delete creature/gameobject respawn times even if the maps are not loaded
}

uint32 InstanceSaveManager::GetNumBoundPlayersTotal()
{
    uint32 ret = 0;
    for (InstanceSaveHashMap::iterator itr = m_instanceSaveById.begin(); itr != m_instanceSaveById.end(); ++itr)
        ret += itr->second->GetPlayerCount();
    return ret;
}

uint32 InstanceSaveManager::GetNumBoundGroupsTotal()
{
    uint32 ret = 0;
    for (InstanceSaveHashMap::iterator itr = m_instanceSaveById.begin(); itr != m_instanceSaveById.end(); ++itr)
        ret += itr->second->GetGroupCount();
    return ret;
}
