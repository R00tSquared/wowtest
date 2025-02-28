// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#include "PoolManager.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
//#include "ProgressBar.h"
#include "Log.h"
#include "MapManager.h"
#include "World.h"

////////////////////////////////////////////////////////////
// template class SpawnedPoolData

// Method that tell amount spawned objects/subpools
uint32 SpawnedPoolData::GetSpawnedObjects(uint32 pool_id) const
{
    SpawnedPoolPools::const_iterator itr = mSpawnedPools.find(pool_id);
    return itr != mSpawnedPools.end() ? itr->second : 0;
}

// Method that tell if a creature is spawned currently
template<>
bool SpawnedPoolData::IsSpawnedObject<Creature>(uint32 db_guid) const
{
    return mSpawnedCreatures.find(db_guid) != mSpawnedCreatures.end();
}

// Method that tell if a gameobject is spawned currently
template<>
bool SpawnedPoolData::IsSpawnedObject<GameObject>(uint32 db_guid) const
{
    return mSpawnedGameobjects.find(db_guid) != mSpawnedGameobjects.end();
}

// Method that tell if a pool is spawned currently
template<>
bool SpawnedPoolData::IsSpawnedObject<void>(uint32 sub_pool_id) const
{
    return mSpawnedPools.find(sub_pool_id) != mSpawnedPools.end();
}

template<>
void SpawnedPoolData::AddSpawn<Creature>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedCreatures.insert(db_guid);
    ++mSpawnedPools[pool_id];
}

template<>
void SpawnedPoolData::AddSpawn<GameObject>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedGameobjects.insert(db_guid);
    ++mSpawnedPools[pool_id];
}

template<>
void SpawnedPoolData::AddSpawn<void>(uint32 sub_pool_id, uint32 pool_id)
{
    mSpawnedPools[sub_pool_id] = 0;
    ++mSpawnedPools[pool_id];
}

template<>
void SpawnedPoolData::RemoveSpawn<Creature>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedCreatures.erase(db_guid);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

template<>
void SpawnedPoolData::RemoveSpawn<GameObject>(uint32 db_guid, uint32 pool_id)
{
    mSpawnedGameobjects.erase(db_guid);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

template<>
void SpawnedPoolData::RemoveSpawn<void>(uint32 sub_pool_id, uint32 pool_id)
{
    mSpawnedPools.erase(sub_pool_id);
    uint32& val = mSpawnedPools[pool_id];
    if (val > 0)
        --val;
}

////////////////////////////////////////////////////////////
// Methods of template class PoolGroup

// Method to add a gameobject/creature guid to the proper list depending on pool type and chance value
template <class T>
void PoolGroup<T>::AddEntry(PoolObject& poolitem)
{
    if (poolitem.chance != 0 && maxLimit == 1)
        ExplicitlyChanced.push_back(poolitem);
    else
        EqualChanced.push_back(poolitem);
}

// Method to check the chances are proper in this object pool
template <class T>
bool PoolGroup<T>::CheckPool() const
{
    if (EqualChanced.size() == 0)
    {
        float chance = 0;
        for (uint32 i=0; i<ExplicitlyChanced.size(); ++i)
            chance += ExplicitlyChanced[i].chance;
        if (chance != 100 && chance != 0)
            return false;
    }
    return true;
}

template <class T>
PoolObject* PoolGroup<T>::RollOne(SpawnedPoolData& spawns, uint32 triggerFrom)
{
    if (!ExplicitlyChanced.empty())
    {
        float roll = (float)rand_chance();

        for (uint32 i = 0; i < ExplicitlyChanced.size(); ++i)
        {
            roll -= ExplicitlyChanced[i].chance;
            // Triggering object is marked as spawned at this time and can be also rolled (respawn case)
            // so this need explicit check for this case
            if (roll < 0 && (ExplicitlyChanced[i].guid == triggerFrom || !spawns.IsSpawnedObject<T>(ExplicitlyChanced[i].guid)))
                return &ExplicitlyChanced[i];
        }
    }

    if (!EqualChanced.empty())
    {
        uint32 index = urand(0, EqualChanced.size() - 1);
        // Fill a list of possible rolls
        std::vector<uint32> possible_rolls;
        for (int i = 0; i < EqualChanced.size(); ++i)
            if (EqualChanced[i].guid == triggerFrom || !spawns.IsSpawnedObject<T>(EqualChanced[i].guid))
                possible_rolls.push_back(i);
        if (!possible_rolls.empty())
        {
            index = urand(0, possible_rolls.size() - 1);
            return &EqualChanced[possible_rolls[index]];
        }
    }

    return NULL;
}

// Main method to despawn a creature or gameobject in a pool
// If no guid is passed, the pool is just removed (event end case)
// If guid is filled, cache will be used and no removal will occur, it just fill the cache
template<class T>
void PoolGroup<T>::DespawnObject(SpawnedPoolData& spawns, uint32 guid)
{
    for (size_t i = 0; i < EqualChanced.size(); ++i)
    {
        // if spawned
        if (spawns.IsSpawnedObject<T>(EqualChanced[i].guid))
        {
            // any or specially requested
            if (!guid || EqualChanced[i].guid == guid)
            {
                Despawn1Object(EqualChanced[i].guid);
                spawns.RemoveSpawn<T>(EqualChanced[i].guid,poolId);
            }
        }
    }

    for (size_t i = 0; i < ExplicitlyChanced.size(); ++i)
    {
        // spawned
        if (spawns.IsSpawnedObject<T>(ExplicitlyChanced[i].guid))
        {
            // any or specially requested
            if (!guid || ExplicitlyChanced[i].guid == guid)
            {
                Despawn1Object(ExplicitlyChanced[i].guid);
                spawns.RemoveSpawn<T>(ExplicitlyChanced[i].guid,poolId);
            }
        }
    }
}

// Method that is actualy doing the removal job on one creature
template<>
void PoolGroup<Creature>::Despawn1Object(uint32 guid)
{
    if (CreatureData const* data = sObjectMgr.GetCreatureData(guid))
    {
        sObjectMgr.RemoveCreatureFromGrid(guid, data);

        // FIXME: pool system must have local state for each instanced map copy
        // Current code preserve existed single state for all instanced map copies way
        // specially because pool system not spawn object in instanceable maps
        MapEntry const* mapEntry = sMapStore.LookupEntry(data->mapid);

        // temporary limit pool system full power work to continents
        if (mapEntry && !mapEntry->Instanceable())
        {
            if (Map* map = sMapMgr.FindMap(data->mapid, data->sInstId))
            {
                if (Creature* pCreature = map->GetCreature(ObjectGuid(HIGHGUID_UNIT, data->id, guid)))
                {
                    pCreature->AddObjectToRemoveList();
                    // clear respawn time for despawned object because it's respawn time is "moved" to another object.
                    // (corpse time already done and won't be moved when this happens)
                    sObjectMgr.SaveCreatureRespawnTime(pCreature->GetDBTableGUIDLow(), 0, data->mapid, mapEntry->IsRaid(), 0);
                }
            }
        }
    }
}

// Same on one gameobject
template<>
void PoolGroup<GameObject>::Despawn1Object(uint32 guid)
{
    if (GameObjectData const* data = sObjectMgr.GetGOData(guid))
    {
        sObjectMgr.RemoveGameobjectFromGrid(guid, data);

        // FIXME: pool system must have local state for each instanced map copy
        // Current code preserve existed single state for all instanced map copies way
        // specially because pool system not spawn object in instanceable maps
        MapEntry const* mapEntry = sMapStore.LookupEntry(data->mapid);

        // temporary limit pool system full power work to continents
        if (mapEntry && !mapEntry->Instanceable())
        {
            if (Map* map = sMapMgr.FindMap(data->mapid, data->sInstId))
            {
                if (GameObject* pGameobject = map->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, data->id, guid)))
                {
                    pGameobject->AddObjectToRemoveList();
                    // clear respawn time for despawned object because it's respawn time is "moved" to another object.
                    sObjectMgr.SaveGORespawnTime(pGameobject->GetDBTableGUIDLow(), 0, 0);
                }
            }
        }
    }
}

// Same on one pool
template<>
void PoolGroup<void>::Despawn1Object(uint32 child_pool_id)
{
    sPoolMgr.DespawnPool(child_pool_id);
}

// Method for a pool only to remove any found record causing a circular dependency loop
template<>
void PoolGroup<void>::RemoveOneRelation(uint16 child_pool_id)
{
    for (PoolObjectList::iterator itr = ExplicitlyChanced.begin(); itr != ExplicitlyChanced.end(); ++itr)
    {
        if(itr->guid == child_pool_id)
        {
            ExplicitlyChanced.erase(itr);
            break;
        }
    }
    for (PoolObjectList::iterator itr = EqualChanced.begin(); itr != EqualChanced.end(); ++itr)
    {
        if(itr->guid == child_pool_id)
        {
            EqualChanced.erase(itr);
            break;
        }
    }
}

template <class T>
void PoolGroup<T>::SpawnObject(SpawnedPoolData& spawns, uint32 triggerFromPoolOrObject, uint32 triggerFromObject)
{
    int count = maxLimit - spawns.GetSpawnedObjects(poolId);
    bool triggered = triggerFromPoolOrObject;
    if (triggerFromPoolOrObject > 1) // 0 if instant spawn. 1 if was not spawned and not instant
        ++count; // this object still counts as spawned, so we need to increase limit by 1

    // This will spawn the rest of pool
    for (int i = 0; i < count; ++i)
    {
        PoolObject* obj = RollOne(spawns, triggerFromPoolOrObject);
        if (!obj)
            continue;

        if (obj->guid == triggerFromPoolOrObject)
        {
            triggerFromPoolOrObject = 0; // do not allow to RollOne for triggerFrom anymore. This object will be respawned in common object's Update()

            // do not call addSpawn if this is creature or gameobject, but do call if it is a pool
            // triggerFromObject exists if it is pool call
            if (triggerFromObject)
                Spawn1Object(obj, triggerFromObject); // reroll object to be created in this pool
            continue;
        }

        spawns.AddSpawn<T>(obj->guid,poolId);
        // for fully new pools and any objects send 1 if there was a trigger
        Spawn1Object(obj, triggered);
    }

    // current object not respawned - so need to despawn it
    if (triggerFromPoolOrObject > 1)
        DespawnObject(spawns, triggerFromPoolOrObject);
}

// Method that is actualy doing the spawn job on 1 creature
template <>
void PoolGroup<Creature>::Spawn1Object(PoolObject* obj, uint32 triggerFromObject)
{
    if (CreatureData const* data = sObjectMgr.GetCreatureData(obj->guid))
    {
        sObjectMgr.AddCreatureToGrid(obj->guid, data);

        MapEntry const* mapEntry = sMapStore.LookupEntry(data->mapid);

        // FIXME: pool system must have local state for each instanced map copy
        // Current code preserve existed single state for all instanced map copies way
        if (mapEntry && !mapEntry->Instanceable())
        {
            // Spawn if necessary (loaded grids only)
            Map* map = sMapMgr.FindMap(data->mapid, data->sInstId);

            // if new spawn replaces a just despawned object, move just despawned objects respawn time to this object.
            // gotta set respawn before spawning so object spawns as despawned.
            if (triggerFromObject)
                sObjectMgr.SaveCreatureRespawnTime(obj->guid, 0, data->mapid, mapEntry->IsRaid(), time(NULL) + data->spawntimesecs);
            // if no triggerFromObject -> spawns with current respawn time from DB, it could be none or something.

            // We use spawn coords to spawn
            if (map && map->IsLoaded(data->posX, data->posY))
            {
                Creature* pCreature = new Creature;
                //debug_log("Spawning creature %u",obj->guid);
                if (!pCreature->LoadFromDB(obj->guid, map))
                {
                    delete pCreature;
                    return;
                }
                else
                    map->Add(pCreature);
            }
        }
    }
}

// Same for 1 gameobject
template <>
void PoolGroup<GameObject>::Spawn1Object(PoolObject* obj, uint32 triggerFromObject)
{
    if (GameObjectData const* data = sObjectMgr.GetGOData(obj->guid))
    {
        sObjectMgr.AddGameobjectToGrid(obj->guid, data);

        MapEntry const* mapEntry = sMapStore.LookupEntry(data->mapid);

        // FIXME: pool system must have local state for each instanced map copy
        // Current code preserve existed single state for all instanced map copies way
        if (mapEntry && !mapEntry->Instanceable())
        {
            // Spawn if necessary (loaded grids only)
            Map* map = sMapMgr.FindMap(data->mapid, data->sInstId);

            // if new spawn replaces a just despawned object, move just despawned objects respawn time to this object.
            // gotta set respawn before spawning so object spawns as despawned.
            if (triggerFromObject)
                sObjectMgr.SaveGORespawnTime(obj->guid, 0, time(NULL) + data->spawntimesecs);
            // if no triggerFromObject -> spawns with current respawn time from DB, it could be none or something.

            // We use spawn coords to spawn
            if (map && map->IsLoaded(data->posX, data->posY))
            {
                GameObject* pGameobject = new GameObject;
                //debug_log("Spawning gameobject %u", obj->guid);
                if (!pGameobject->LoadFromDB(obj->guid, map))
                {
                    delete pGameobject;
                    return;
                }
                else
                    map->Add(pGameobject);
            }
        }
    }
}

// Same for 1 pool
template <>
void PoolGroup<void>::Spawn1Object(PoolObject* obj, uint32 triggerFromObject)
{
    sPoolMgr.SpawnPool(obj->guid, triggerFromObject);
}

////////////////////////////////////////////////////////////
// Methods of class PoolManager

PoolManager::PoolManager()
{
}

// Check listing all pool spawns in single instanceable map or only in non-instanceable maps
// This applied to all pools have common mother pool
struct PoolMapChecker
{
    typedef std::map<uint32,MapEntry const*> Pool2Maps;
    Pool2Maps m_pool2maps;

    bool CheckAndRemember(uint32 mapid, uint32 pool_id, char const* tableName, char const* elementName)
    {
        MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
        if (!mapEntry)
            return false;

        MapEntry const* poolMapEntry = GetPoolMapEntry(pool_id);

        // if not listed then just remember
        if (!poolMapEntry)
        {
            m_pool2maps[pool_id] = mapEntry;
            return true;
        }

        // if at same map, then all ok
        if (poolMapEntry == mapEntry)
            return true;

        // pool spawns must be at single instanceable map
        if (mapEntry->Instanceable())
        {
            sLog.outLog(LOG_DB_ERR, "`%s` has %s spawned at instanceable map %u when one or several other spawned at different map %u in pool id %i, skipped.",
                tableName, elementName, mapid, poolMapEntry->MapID, pool_id);
            return false;
        }

        // pool spawns must be at single instanceable map
        if (poolMapEntry->Instanceable())
        {
            sLog.outLog(LOG_DB_ERR, "`%s` has %s spawned at map %u when one or several other spawned at different instanceable map %u in pool id %i, skipped.",
                tableName, elementName, mapid, poolMapEntry->MapID, pool_id);
            return false;
        }

        // pool spawns can be at different non-instanceable maps
        return true;
    }

    MapEntry const* GetPoolMapEntry(uint32 pool_id) const
    {
        Pool2Maps::const_iterator p2m_itr = m_pool2maps.find(pool_id);
        return p2m_itr != m_pool2maps.end() ? p2m_itr->second : NULL;
    }
};

void PoolManager::LoadFromDB()
{
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT MAX(entry) FROM pool_template");
    if (!result)
    {
        sLog.outString(">> Table pool_template is empty.");
        sLog.outString();
        return;
    }
    else
    {
        Field *fields = result->Fetch();
        max_pool_id = fields[0].GetUInt16();
        if (!max_pool_id)
            return;
    }

    uint32 count = 0;
    PoolMapChecker mapChecker;
    mPoolCreatureGroups.resize(max_pool_id + 1);
    mPoolGameobjectGroups.resize(max_pool_id + 1);
    mPoolPoolGroups.resize(max_pool_id + 1);

    result = GameDataDatabase.Query("SELECT entry,max_limit FROM pool_template");
    //BarGoLink bar((int)result->GetRowCount());
    do
    {
        ++count;
        Field *fields = result->Fetch();
        //bar.step();
        uint16 pool_id = fields[0].GetUInt16();

        mPoolCreatureGroups[pool_id].SetLimit(fields[1].GetUInt32());
        mPoolGameobjectGroups[pool_id].SetLimit(fields[1].GetUInt32());
        mPoolPoolGroups[pool_id].SetLimit(fields[1].GetUInt32());
        if (fields[1].GetUInt32() == 0)
            sLog.outLog(LOG_DB_ERR, "limit of a pool %u is 0", pool_id);

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u objects pools", count);

    // Creatures
    mCreatureSearchMap.clear();
    //                                   1     2           3
    result = GameDataDatabase.Query("SELECT guid, pool_entry, chance FROM pool_creature");
    count = 0;
    if (!result)
    {
        //BarGoLink bar2(1);
        //bar2.step();

        sLog.outString();
        sLog.outString(">> Loaded %u creatures in pools", count );
    }
    else
    {

        //BarGoLink bar2((int)result->GetRowCount());
        do
        {
            Field *fields = result->Fetch();

            //bar2.step();

            uint32 guid    = fields[0].GetUInt32();
            uint16 pool_id = fields[1].GetUInt16();
            float chance   = fields[2].GetFloat();

            CreatureData const* data = sObjectMgr.GetCreatureData(guid);
            if (!data)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_creature` has a non existing creature spawn (GUID: %u) defined for pool id (%u), skipped.", guid, pool_id );
                continue;
            }
            if (pool_id > max_pool_id)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_creature` pool id (%i) is out of range compared to max pool id in `pool_template`, skipped.",pool_id);
                continue;
            }
            if (chance < 0 || chance > 100)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_creature` has an invalid chance (%f) for creature guid (%u) in pool id (%i), skipped.", chance, guid, pool_id);
                continue;
            }

            if (!mapChecker.CheckAndRemember(data->mapid, pool_id, "pool_creature", "creature guid"))
                continue;

            ++count;

            PoolObject plObject = PoolObject(guid, chance);
            PoolGroup<Creature>& cregroup = mPoolCreatureGroups[pool_id];
            cregroup.SetPoolId(pool_id);
            cregroup.AddEntry(plObject);
            SearchPair p(guid, pool_id);
            mCreatureSearchMap.insert(p);

        } while (result->NextRow());
        sLog.outString();
        sLog.outString( ">> Loaded %u creatures in pools", count );
    }

    // Gameobjects
    mGameobjectSearchMap.clear();
    //                                   1     2           3
    result = GameDataDatabase.Query("SELECT guid, pool_entry, chance FROM pool_gameobject");
    count = 0;
    if (!result)
    {
        //BarGoLink bar2(1);
        //bar2.step();

        sLog.outString();
        sLog.outString(">> Loaded %u gameobject in pools", count );
    }
    else
    {

        //BarGoLink bar2((int)result->GetRowCount());
        do
        {
            Field *fields = result->Fetch();

            //bar2.step();

            uint32 guid    = fields[0].GetUInt32();
            uint16 pool_id = fields[1].GetUInt16();
            float chance   = fields[2].GetFloat();

            GameObjectData const* data = sObjectMgr.GetGOData(guid);
            if (!data)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` has a non existing gameobject spawn (GUID: %u) defined for pool id (%u), skipped.", guid, pool_id );
                continue;
            }
            GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(data->id);
            if (goinfo->type != GAMEOBJECT_TYPE_CHEST &&
                goinfo->type != GAMEOBJECT_TYPE_GOOBER &&
                goinfo->type != GAMEOBJECT_TYPE_FISHINGHOLE)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` has a not lootable gameobject spawn (GUID: %u, type: %u) defined for pool id (%u), skipped.", guid, goinfo->type, pool_id );
                continue;
            }
            if (pool_id > max_pool_id)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` pool id (%i) is out of range compared to max pool id in `pool_template`, skipped.",pool_id);
                continue;
            }
            if (chance < 0 || chance > 100)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_gameobject` has an invalid chance (%f) for gameobject guid (%u) in pool id (%i), skipped.", chance, guid, pool_id);
                continue;
            }

            if (!mapChecker.CheckAndRemember(data->mapid, pool_id, "pool_gameobject", "gameobject guid"))
                continue;

            ++count;

            PoolObject plObject = PoolObject(guid, chance);
            PoolGroup<GameObject>& gogroup = mPoolGameobjectGroups[pool_id];
            gogroup.SetPoolId(pool_id);
            gogroup.AddEntry(plObject);
            SearchPair p(guid, pool_id);
            mGameobjectSearchMap.insert(p);

        } while( result->NextRow() );
        sLog.outString();
        sLog.outString( ">> Loaded %u gameobject in pools", count );
    }

    result = GameDataDatabase.Query("SELECT pool_id, mother_pool, chance FROM pool_pool WHERE mother_pool IN (select pool_id FROM pool_pool)");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint16 child_pool_id = fields[0].GetUInt16();
            uint16 mother_pool_id = fields[1].GetUInt16();

            sLog.outLog(LOG_DB_ERR, "`pool_pool` mother_pool id (%u) is referenced by another child pool (%u). Only one level deep is allowed. Skipped.", mother_pool_id, child_pool_id);
        } while (result->NextRow());
    }

    // Pool of pools                     1        2            3                                 only allow one level of pool_pool
    result = GameDataDatabase.Query("SELECT pool_id, mother_pool, chance FROM pool_pool WHERE mother_pool NOT IN (select pool_id FROM pool_pool)");
    count = 0;
    if( !result )
    {
        //BarGoLink bar2(1);
        //bar2.step();

        sLog.outString();
        sLog.outString(">> Loaded %u pools in pools", count );
    }
    else
    {

        //BarGoLink bar2((int)result->GetRowCount());
        do
        {
            Field *fields = result->Fetch();

            //bar2.step();

            uint16 child_pool_id = fields[0].GetUInt16();
            uint16 mother_pool_id = fields[1].GetUInt16();
            float chance = fields[2].GetFloat();

            if (mother_pool_id > max_pool_id)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_pool` mother_pool id (%i) is out of range compared to max pool id in `pool_template`, skipped.", mother_pool_id);
                continue;
            }
            if (child_pool_id > max_pool_id)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_pool` included pool_id (%i) is out of range compared to max pool id in `pool_template`, skipped.", child_pool_id);
                continue;
            }
            if (mother_pool_id == child_pool_id)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_pool` pool_id (%i) includes itself, dead-lock detected, skipped.", child_pool_id);
                continue;
            }
            if (chance < 0 || chance > 100)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_pool` has an invalid chance (%f) for pool id (%u) in mother pool id (%i), skipped.", chance, child_pool_id, mother_pool_id);
                continue;
            }

            if (mPoolCreatureGroups[child_pool_id].GetLimit() != 1 || mPoolGameobjectGroups[child_pool_id].GetLimit() != 1)
            {
                sLog.outLog(LOG_DB_ERR, "`pool_pool` has an invalid limit for pool id (%u) in mother pool id (%i), limit should be 1, skipped.", child_pool_id, mother_pool_id);
                continue;
            }

            ++count;

            PoolObject plObject = PoolObject(child_pool_id, chance);
            PoolGroup<void>& plgroup = mPoolPoolGroups[mother_pool_id];
            plgroup.SetPoolId(mother_pool_id);
            plgroup.AddEntry(plObject);
            SearchPair p(child_pool_id, mother_pool_id);
            mPoolSearchMap.insert(p);

        } while (result->NextRow());

        // Now check for circular reference
        uint32 levelsDeep = 0;
        for (uint16 i = 0; i <= max_pool_id; ++i)
        {
            std::set<uint16> checkedPools;
            for (SearchMap::iterator poolItr = mPoolSearchMap.find(i); poolItr != mPoolSearchMap.end(); poolItr = mPoolSearchMap.find(poolItr->second))
            {
                // if child pool not have map data then it empty or have not checked child then will checked and all line later
                if (MapEntry const* childMapEntry = mapChecker.GetPoolMapEntry(poolItr->first))
                {
                    if (!mapChecker.CheckAndRemember(childMapEntry->MapID, poolItr->second, "pool_pool", "pool with creature/gameobject"))
                    {
                        mPoolPoolGroups[poolItr->second].RemoveOneRelation(poolItr->first);
                        mPoolSearchMap.erase(poolItr);
                        --count;
                        break;
                    }
                }

                checkedPools.insert(poolItr->first);
                if (checkedPools.find(poolItr->second) != checkedPools.end())
                {
                    std::ostringstream ss;
                    ss << "The pool(s) ";
                    for (std::set<uint16>::const_iterator itr = checkedPools.begin(); itr != checkedPools.end(); ++itr)
                        ss << *itr << " ";
                    ss << "create(s) a circular reference, which can cause the server to freeze.\nRemoving the last link between mother pool "
                        << poolItr->first << " and child pool " << poolItr->second;
                    sLog.outLog(LOG_DB_ERR, "%s", ss.str().c_str());
                    mPoolPoolGroups[poolItr->second].RemoveOneRelation(poolItr->first);
                    mPoolSearchMap.erase(poolItr);
                    --count;
                    break;
                }
            }
        }

        sLog.outString();
        sLog.outString(">> Loaded %u pools in mother pools", count);
    }

    /*result = GameDataDatabase.Query("SELECT pool_entry, SUM(chance) FROM pool_gameobject WHERE pool_entry NOT IN (select * from (SELECT pool_entry FROM pool_gameobject WHERE chance = 0) as t) GROUP BY pool_entry HAVING sum(chance) < 100");
	
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint16 entry = fields[0].GetUInt16();
            float sum = fields[1].GetFloat();

            sLog.outLog(LOG_DB_ERR, "`pool_gameobject` pool_entry %u chance sum lower than 100%% and no 0%% chanced objects. Defined chance sum is %f%%. Undefined behaviour.", entry, sum);
        } while (result->NextRow());
    }

    result = GameDataDatabase.Query("SELECT pool_entry, SUM(chance) FROM pool_gameobject WHERE pool_entry IN (select * from (SELECT pool_entry FROM pool_gameobject WHERE chance = 0) as t) GROUP BY pool_entry HAVING sum(chance) > 99");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint16 entry = fields[0].GetUInt16();
            float sum = fields[1].GetFloat();

            sLog.outLog(LOG_DB_ERR, "`pool_gameobject` pool_entry %u chance sum over 100%%. Defined chance sum is %f%%. Undefined behaviour.", entry, sum);
        } while (result->NextRow());
    }

    result = GameDataDatabase.Query("SELECT pool_entry, SUM(chance) FROM pool_creature WHERE pool_entry NOT IN (select * from (SELECT pool_entry FROM pool_creature WHERE chance = 0) as t) GROUP BY pool_entry HAVING sum(chance) < 100");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint16 entry = fields[0].GetUInt16();
            float sum = fields[1].GetFloat();

            sLog.outLog(LOG_DB_ERR, "`pool_creature` pool_entry %u chance sum lower than 100%% and no 0%% chanced objects. Defined chance sum is %f%%. Undefined behaviour.", entry, sum);
        } while (result->NextRow());
    }

    result = GameDataDatabase.Query("SELECT pool_entry, SUM(chance) FROM pool_creature WHERE pool_entry IN (select * from (SELECT pool_entry FROM pool_creature WHERE chance = 0) as t) GROUP BY pool_entry HAVING sum(chance) > 99");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint16 entry = fields[0].GetUInt16();
            float sum = fields[1].GetFloat();

            sLog.outLog(LOG_DB_ERR, "`pool_creature` pool_entry %u chance sum over 100%%. Defined chance sum is %f%%. Undefined behaviour.", entry, sum);
        } while (result->NextRow());
    }

    result = GameDataDatabase.Query("SELECT mother_pool, SUM(chance) FROM pool_pool WHERE mother_pool NOT IN (select * from (SELECT mother_pool FROM pool_pool WHERE chance = 0) as t) GROUP BY mother_pool HAVING sum(chance) < 100");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint16 entry = fields[0].GetUInt16();
            float sum = fields[1].GetFloat();

            sLog.outLog(LOG_DB_ERR, "`pool_pool` mother_pool %u chance sum lower than 100%% and no 0%% chanced objects. Defined chance sum is %f%%. Undefined behaviour.", entry, sum);
        } while (result->NextRow());
    }

    result = GameDataDatabase.Query("SELECT mother_pool, SUM(chance) FROM pool_pool WHERE mother_pool IN (select * from (SELECT mother_pool FROM pool_pool WHERE chance = 0) as t) GROUP BY mother_pool HAVING sum(chance) > 99");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint16 entry = fields[0].GetUInt16();
            float sum = fields[1].GetFloat();

            sLog.outLog(LOG_DB_ERR, "`pool_pool` mother_pool %u chance sum over 100%%. Defined chance sum is %f%%. Undefined behaviour.", entry, sum);
        } while (result->NextRow());
    }*/
}

// The initialize method will spawn all pools not in an event and not in another pool
void PoolManager::Initialize()
{
    uint32 count = 0;

    if (!max_pool_id)
    {
        sLog.outBasic("Pool handling system is offline, 0 pools spawned.");
        return;
    }

    for(uint16 pool_entry = 0; pool_entry <= max_pool_id; ++pool_entry)
    {
        if (IsPartOfAPool<void>(pool_entry) == 0)
        {
            if (!CheckPool(pool_entry))
            {
                sLog.outLog(LOG_DB_ERR, "Pool Id (%u) has all creatures or gameobjects with explicit chance sum <>100 and no equal chance defined. The pool system cannot pick one to spawn.", pool_entry);
                continue;
            }
            SpawnPool(pool_entry, 0);
            count++;
        }
    }

    sLog.outBasic("Pool handling system initialized, %u pools spawned.", count);
}

// Call to spawn a pool, if cache if true the method will spawn only if cached entry is different
// If it's same, the creature is respawned only (added back to map)
template<>
void PoolManager::SpawnPoolGroup<Creature>(uint16 pool_id, uint32 triggerFromPoolOrObject, uint32 triggerFromObject)
{
    if (!mPoolCreatureGroups[pool_id].isEmpty())
        mPoolCreatureGroups[pool_id].SpawnObject(mSpawnedData, triggerFromPoolOrObject, triggerFromObject);
}

// Call to spawn a pool, if cache if true the method will spawn only if cached entry is different
// If it's same, the gameobject is respawned only (added back to map)
template<>
void PoolManager::SpawnPoolGroup<GameObject>(uint16 pool_id, uint32 triggerFromPoolOrObject, uint32 triggerFromObject)
{
    if (!mPoolGameobjectGroups[pool_id].isEmpty())
        mPoolGameobjectGroups[pool_id].SpawnObject(mSpawnedData, triggerFromPoolOrObject, triggerFromObject);
}

// Call to spawn a pool, if cache if true the method will spawn only if cached entry is different
// If it's same, the pool is respawned only
template<>
void PoolManager::SpawnPoolGroup<void>(uint16 pool_id, uint32 triggerFromPoolOrObject, uint32 triggerFromObject)
{
    if (!mPoolPoolGroups[pool_id].isEmpty())
        mPoolPoolGroups[pool_id].SpawnObject(mSpawnedData, triggerFromPoolOrObject, triggerFromObject);
}

void PoolManager::SpawnPool(uint16 pool_id, uint32 triggerFromObject)
{
    SpawnPoolGroup<void>(pool_id, triggerFromObject, 0);
    SpawnPoolGroup<GameObject>(pool_id, triggerFromObject, 0);
    SpawnPoolGroup<Creature>(pool_id, triggerFromObject, 0);
}

// Call to despawn a pool, all gameobjects/creatures in this pool are removed
void PoolManager::DespawnPool(uint16 pool_id)
{
    if (!mPoolCreatureGroups[pool_id].isEmpty())
        mPoolCreatureGroups[pool_id].DespawnObject(mSpawnedData);

    if (!mPoolGameobjectGroups[pool_id].isEmpty())
        mPoolGameobjectGroups[pool_id].DespawnObject(mSpawnedData);

    if (!mPoolPoolGroups[pool_id].isEmpty())
        mPoolPoolGroups[pool_id].DespawnObject(mSpawnedData);
}

// Method that check chance integrity of the creatures and gameobjects in this pool
bool PoolManager::CheckPool(uint16 pool_id) const
{
    return pool_id <= max_pool_id &&
        mPoolGameobjectGroups[pool_id].CheckPool() &&
        mPoolCreatureGroups[pool_id].CheckPool() &&
        mPoolPoolGroups[pool_id].CheckPool();
}

// Call to update the pool when a gameobject/creature part of pool [pool_id] is ready to respawn
// Here we cache only the creature/gameobject whose guid is passed as parameter
// Then the spawn pool call will use this cache to decide
template<typename T>
void PoolManager::UpdatePool(uint16 pool_id, uint32 triggerFrom)
{
    if (uint16 motherpoolid = IsPartOfAPool<void>(pool_id))
        SpawnPoolGroup<void>(motherpoolid, pool_id, triggerFrom);
    else
        SpawnPoolGroup<T>(pool_id, triggerFrom, 0);
}

template void PoolManager::UpdatePool<void>(uint16 pool_id, uint32 db_guid_or_pool_id);
template void PoolManager::UpdatePool<GameObject>(uint16 pool_id, uint32 db_guid_or_pool_id);
template void PoolManager::UpdatePool<Creature>(uint16 pool_id, uint32 db_guid_or_pool_id);

bool PoolManager::IsSpawnedOrNotInPoolGameobject(uint32 db_guid) const
{
    return IsPartOfAPool<GameObject>(db_guid) ?
        mSpawnedData.IsSpawnedObject<GameObject>(db_guid) : true;
}
