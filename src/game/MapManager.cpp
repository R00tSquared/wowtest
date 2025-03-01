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

#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Transports.h"
#include "GridDefines.h"
#include "InstanceData.h"
#include "World.h"
#include "CellImpl.h"
#include "Corpse.h"
#include "ObjectMgr.h"
#include "GridMap.h"
#include "GameEvent.h"

#include "BattleGround.h"

MapManager::MapManager() : i_gridCleanUpDelay(sWorld.getConfig(CONFIG_INTERVAL_GRIDCLEAN))
{
}

MapManager::~MapManager()
{
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        delete iter->second;

    for (TransportSet::iterator i = m_Transports.begin(); i != m_Transports.end(); ++i)
        delete *i;

    Map::DeleteStateMachine();
}

void MapManager::Initialize()
{
    Map::InitStateMachine();

    InitMaxInstanceId();

    int num_threads(sWorld.getConfig(CONFIG_NUMTHREADS));
    // Start mtmaps if needed.
    if (m_updater.activate(num_threads) == -1)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MapUpdater cannot be activated !!!!!");
        abort();
    }
}

void MapManager::InitializeVisibilityDistanceInfo()
{
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        (*iter).second->InitVisibilityDistance();
}

Map* MapManager::CreateMap(uint32 id, WorldObject const* obj)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, Guard, Lock, NULL);

    Map * m = NULL;

    const MapEntry* entry = sMapStore.LookupEntry(id);
    if (!entry)
        return NULL;

    if (entry->Instanceable())
    {
        ASSERT(obj->GetTypeId() == TYPEID_PLAYER);
        //create InstanceMap object
        if (obj->GetTypeId() == TYPEID_PLAYER)
            m = CreateInstance(id, (Player*)obj);
    }
    else
    {
        //create regular Continent map
        if (obj)
            m = FindMap(id, sObjectMgr.GetSingleInstance(id, obj->GetPositionX(), obj->GetPositionY()));
        else
            m = FindMap(id, INSTANCEID_NULL); // this should not happen though, But just in case we will find the basic map to not create new map in its place
        
        if (m == NULL)
        {
            m = new Map(id, i_gridCleanUpDelay, 0, DIFFICULTY_NORMAL);
            //add map into container
            i_maps[MapID(id)] = m;
            
            //on first map create -> also create all of its singleinstances
            /*if (!obj)
            {
                uint32 start = MAP530_END;
                uint32 end = MAP530_END;
                switch (id)
                {
                    case 0:
                        start = MAP0_START;
                        end = MAP0_END;
                        break;
                    case 1:
                        start = MAP1_START;
                        end = MAP1_END;
                        break;
                    case 530:
                        start = MAP530_START;
                        end = MAP530_END;
                        break;
                    default:
                        break;
                }
                for (uint32 i = start; i < end; ++i)
                    i_maps[MapID(id, i)] = new Map(id, i_gridCleanUpDelay, i, DIFFICULTY_NORMAL);
            }*/
        }
    }

    return m;
}

Map* MapManager::CreateBgMap(uint32 mapid, uint32 instanceID, BattleGround* bg)
{
    TerrainInfo * pData = sTerrainMgr.LoadTerrain(mapid);

    ACE_GUARD_RETURN(ACE_Thread_Mutex, Guard, Lock, NULL);
    return CreateBattleGroundMap(mapid, instanceID, bg);
}

Map* MapManager::FindMap(uint32 mapid, uint32 instanceId) const
{
    //ACE_GUARD_RETURN(ACE_Thread_Mutex, _guard, Lock, NULL);

    MapMapType::const_iterator iter = i_maps.find(MapID(mapid, instanceId));
    if (iter == i_maps.end())
         return NULL;

    //this is a small workaround for transports
    if(instanceId == 0 && iter->second->Instanceable())
    {
        ASSERT(false);
        return NULL;
    }

    return iter->second;
}

/*
    checks that do not require a map to be created
    will send transfer error messages on fail
*/
bool MapManager::CanPlayerEnter(uint32 mapid, Player* player)
{
    const MapEntry *entry = sMapStore.LookupEntry(mapid);
    if (!entry)
        return false;

    const char *mapName = entry->name[player->GetSession()->GetSessionDbcLocale()];
    if (entry->IsDungeon())
    {
        Season season = sWorld.getSeason();
        if (!player->isGameMaster())
        {
            switch (mapid)
            {
				case 550: // TK
				case 548: // SSK
				{
					if (season < SEASON_2)
					{
						player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY1);
						return false;
					}
					break;
				}
                case 564: // Black Temple
                case 534: // Hyjal
                {
                    if (season < SEASON_3)
                    {
                        player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY1);
                        return false;
                    }
                    break;
                }
                case 568: // Zul'Aman
                {
                    if (season < SEASON_4)
                    {
                        player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY1);
                        return false;
                    }
                    break;
                }
                case 585: // Magister's Terrace
                case 580: // Sunwell
                {
                    if (season < SEASON_5)
                    {
                        player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY1);
                        return false;
                    }
                    break;
                }
                default:
                    break;
            }
        }

       // if (sWorld.isEasyRealm() && entry->IsRaid() && player->GetDifficulty() == DIFFICULTY_HEROIC)
       // {
       //     if (sWorld.AvailableHeroics().count(mapid) == 0)
       //     {
       //         player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY2);
			    //return false;
       //     }
       // }

		//if ( && sWorld.getConfig(CONFIG_CLOSED_HEROIC_MASK))
		//{
		//	uint32 heroic_map_mask[9][2] = {
		//		{MAP_KARA,1},
		//		{MAP_MAGTH,2},
		//		{MAP_GRUUL,4},
		//		{MAP_SSK,8},
		//		{MAP_TK,16},
		//		{MAP_HS,32},
		//		{MAP_BT,64},
		//		{MAP_ZA,128},
		//		{MAP_SWP,256},
		//	};
		//	
		//	for (auto v : heroic_map_mask)
		//	{
		//		if (v[0] == mapid && sWorld.getConfig(CONFIG_CLOSED_HEROIC_MASK) & v[1])
		//		{
		//			player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY2);
		//			return false;
		//		}
		//	}
		//}

        if (entry->IsRaid())
        {
            // GMs can avoid raid limitations
            if (!player->isGameMaster() && !sWorld.getConfig(CONFIG_INSTANCE_IGNORE_RAID))
            {
                // can only enter in a raid group
                Group* group = player->GetGroup();
                if (!group || !group->isRaidGroup())
                {
                    // probably there must be special opcode, because client has this string constant in GlobalStrings.lua
                    // TODO: this is not a good place to send the message
                    player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetHellgroundString(810), mapName);
                    sLog.outDebug("MAP: Player '%s' must be in a raid group to enter instance of '%s'", player->GetName(), mapName);
                    return false;
                }
            }
        }

        //The player has a heroic mode and tries to enter into instance which has no a heroic mode
        if (!entry->SupportsHeroicMode() && player->GetDifficulty() == DIFFICULTY_HEROIC)
        {
            //Send aborted message
            player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY2);
            return false;
        }

        //if (!player->isGameMaster() && entry->IsRaid() && player->GetDifficulty() == DIFFICULTY_HEROIC) // heroic raid, 60 or 70. Check by map and event id
        //{
        //    switch (mapid)
        //    {
        //        case 548: //| 548 | instance_serpent_shrine    |
        //        {
        //            if (!sGameEventMgr.IsActiveEvent(1032)) // SerpentShrine
        //            {
        //                player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY2);
        //                return false;
        //            }
        //            break;
        //        }
        //        case 564: //| 564 | instance_black_temple      |
        //        {
        //            if (!sGameEventMgr.IsActiveEvent(1031)) // Black Temple
        //            {
        //                player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY2);
        //                return false;
        //            }
        //            break;
        //        }
        //        case 580: //| 580 | instance_sunwell_plateau   |
        //        {
        //            if (!sGameEventMgr.IsActiveEvent(1030)) // Sunwell
        //            {
        //                player->SendTransferAborted(mapid, TRANSFER_ABORT_DIFFICULTY2);
        //                return false;
        //            }
        //            break;
        //        }
        //        default:
        //            break;
        //    }
        //}

        if (!player->isAlive())
        {
            if (Corpse *corpse = player->GetCorpse())
            {
                // let enter in ghost mode in instance that connected to inner instance with corpse
                uint32 instance_map = corpse->GetMapId();
                do
                {
                    if (instance_map == mapid)
                        break;

                    InstanceTemplate const* instance = ObjectMgr::GetInstanceTemplate(instance_map);
                    instance_map = instance ? instance->parent : 0;
                }
                while (instance_map);

                if (!instance_map)
                {
                    player->GetSession()->SendAreaTriggerMessage(player->GetSession()->GetHellgroundString(811), mapName);
                    sLog.outDebug("MAP: Player '%s' doesn't has a corpse in instance '%s' and can't enter", player->GetName(), mapName);
                    return false;
                }
                sLog.outDebug("MAP: Player '%s' has corpse in instance '%s' and can enter", player->GetName(), mapName);
            }
            else
            {
                sLog.outDebug("Map::CanEnter - player '%s' is dead but doesn't have a corpse!", player->GetName());
            }
        }

        // Requirements
        InstanceTemplate const* instance = ObjectMgr::GetInstanceTemplate(mapid);
        if (!instance)
            return false;

        const AccessRequirement* aReq = sObjectMgr.GetAccessRequirement(instance->access_id);

        InstanceSave *pSave = player->GetInstanceSave(mapid);
        uint32 instanceId = pSave ? pSave->GetSaveInstanceId() : 0;
        if (!player->isGameMaster() && !player->CheckInstanceCount(instanceId, aReq && aReq->levelMin < 60))
        {
            sLog.outDebug("MAP: Player '%s' can't enter instance %u on map %u. Has already entered too many instances.", player->GetName(), instanceId, mapid);
            player->SendTransferAborted(mapid, TRANSFER_ABORT_TOO_MANY_INSTANCES);
            return false;
        }

        return player->Satisfy(aReq, mapid, true);
    }
    else
        return true;
}

void MapManager::DeleteInstance(uint32 mapid, uint32 instanceId)
{
    ACE_GUARD(ACE_Thread_Mutex, Guard, Lock);
    MapMapType::iterator iter = i_maps.find(MapID(mapid, instanceId));
    if(iter != i_maps.end())
    {
        Map * pMap = iter->second;
        if (pMap->Instanceable())
        {
            i_maps.erase(iter);

            pMap->UnloadAll();
            delete pMap;
        }
    }
}

typedef std::list<std::pair<Map*, uint32> > DelayedMapList;
void MapManager::Update(uint32 diff)
{
    DiffRecorder diffRecorder(__FUNCTION__, sWorld.getConfig(CONFIG_MIN_LOG_UPDATE));

    DelayedMapList delayedUpdate;
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end();)
    {
        if (iter->second->CanUnload(diff))
        {
            iter->second->UnloadAll();
            delete iter->second;

            i_maps.erase(iter++);
        }
        else
        {
            Map::UpdateHelper helper(iter->second);
            if (helper.ProcessUpdate())
                helper.Update(delayedUpdate);

            ++iter;
        }
    }

    m_updater.wait();

    diffRecorder.RecordTimeFor("UpdateMaps");

    for (DelayedMapList::iterator iter = delayedUpdate.begin(); iter != delayedUpdate.end(); ++iter)
        iter->first->DelayedUpdate(iter->second);

    delayedUpdate.clear();

    diffRecorder.RecordTimeFor("Delayed update");

    for (TransportSet::iterator iter = m_Transports.begin(); iter != m_Transports.end(); ++iter)
    {
        WorldObject::UpdateHelper helper(*iter);
        helper.Update(diff);
    }

     diffRecorder.RecordTimeFor("UpdateTransports");
}

bool MapManager::ExistMap(uint32 mapid, float x,float y)
{
    GridPair p = Hellground::ComputeGridPair(x,y);

    int gx=63-p.x_coord;
    int gy=63-p.y_coord;

    return GridMap::ExistMap(mapid,gx,gy);
}

bool MapManager::IsValidMAP(uint32 mapid)
{
    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);
    return mEntry && (!mEntry->Instanceable() || ObjectMgr::GetInstanceTemplate(mapid));
}

void MapManager::UnloadAll()
{
    for (MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
        iter->second->UnloadAll();

    while (!i_maps.empty())
    {
        Map *temp = i_maps.begin()->second;
        i_maps.erase(i_maps.begin());
        delete temp;
    }

    sTerrainMgr.UnloadAll();

    m_updater.deactivate();
}

void MapManager::InitMaxInstanceId()
{
    i_MaxInstanceId = 0;

    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT MIN(id), MAX(id) FROM instance");
    if (result)
    {
        uint32 minInstId = result->Fetch()[0].GetUInt32();
        i_MaxInstanceId = result->Fetch()[1].GetUInt32();

        if (minInstId && minInstId <= SINGLEINSTANCE_RESERVED_INSTANCES_LAST)
        {
            sLog.outLog(LOG_CRASH, "Instances must not have instance id lower than %u", SINGLEINSTANCE_RESERVED_INSTANCES_LAST + 1);
            exit(1);
        }
    }

    if (i_MaxInstanceId <= SINGLEINSTANCE_RESERVED_INSTANCES_LAST)
        i_MaxInstanceId = SINGLEINSTANCE_RESERVED_INSTANCES_LAST + 1;
}

uint32 MapManager::GetNumInstances()
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, Guard, Lock, 0);
    uint32 ret = 0;
    for (MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map *map = itr->second;
        if (!map->IsDungeon())
            continue;

        ret += 1;
    }

    return ret;
}

uint32 MapManager::GetNumPlayersInInstances()
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, Guard, Lock, 0);
    uint32 ret = 0;
    for (MapMapType::iterator itr = i_maps.begin(); itr != i_maps.end(); ++itr)
    {
        Map *map = itr->second;
        if (!map->IsDungeon())
            continue;

        ret += map->GetPlayers().getSize();
    }

    return ret;
}

///// returns a new or existing Instance
///// in case of battlegrounds it will only return an existing map, those maps are created by bg-system
Map* MapManager::CreateInstance(uint32 id, Player * player)
{
    Map* map = NULL;
    Map * pNewMap = NULL;
    uint32 NewInstanceId = 0;                                   // instanceId of the resulting map
    const MapEntry* entry = sMapStore.LookupEntry(id);

    if (entry->IsBattleGroundOrArena())
    {
        // find existing bg map for player
        NewInstanceId = player->GetBattleGroundId();
        //ASSERT(NewInstanceId);
        map = FindMap(id, NewInstanceId);
        //ASSERT(map);

    }
    else if (InstanceSave* pSave = player->GetInstanceSave(id))
    {
        // solo/perm/group
        NewInstanceId = pSave->GetSaveInstanceId();
        map = FindMap(id, NewInstanceId);
        // it is possible that the save exists but the map doesn't
        if (!map)
            pNewMap = CreateInstanceMap(id, NewInstanceId, DungeonDifficulties(pSave->GetDifficulty()), pSave);
    }
    else
    {
        // if no instanceId via group members or instance saves is found
        // the instance will be created for the first time
        NewInstanceId = GenerateInstanceId();

        DungeonDifficulties diff = DungeonDifficulties(player->GetGroup() ? player->GetGroup()->GetDifficulty() : player->GetDifficulty());
        pNewMap = CreateInstanceMap(id, NewInstanceId, diff);
    }

    //add a new map object into the registry
    if (pNewMap)
    {
        i_maps[MapID(id, NewInstanceId)] = pNewMap;
        map = pNewMap;
    }

    return map;
}

InstanceMap* MapManager::CreateInstanceMap(uint32 id, uint32 InstanceId, DungeonDifficulties difficulty, InstanceSave *save)
{
    // make sure we have a valid map id
    const MapEntry* entry = sMapStore.LookupEntry(id);
    if (!entry)
    {
        sLog.outLog(LOG_CRASH, "ERROR: CreateInstanceMap: no entry for map %d", id);
        ASSERT(false);
    }

    if (!ObjectMgr::GetInstanceTemplate(id))
    {
        sLog.outLog(LOG_CRASH, "ERROR: CreateInstanceMap: no instance template for map %d", id);
        ASSERT(false);
    }

    // some instances only have one difficulty
    if (entry && !entry->SupportsHeroicMode())
        difficulty = DIFFICULTY_NORMAL;

    debug_log("MapInstanced::CreateInstanceMap: %s map instance %d for %d created with difficulty %d", save?"":"new ", InstanceId, id, difficulty);

    InstanceMap *map = new InstanceMap(id, i_gridCleanUpDelay, InstanceId, difficulty);
    ASSERT(map->IsDungeon());

    bool load_data = save != NULL;
    map->CreateInstanceData(load_data);

    return map;
}

BattleGroundMap* MapManager::CreateBattleGroundMap(uint32 id, uint32 InstanceId, BattleGround* bg)
{
    debug_log("MapInstanced::CreateBattleGroundMap: instance:%d for map:%d and bgType:%d created.", InstanceId, id, bg->GetTypeID());

	// AV is always heroic
	//uint8 mode = bg->GetTypeID() == BATTLEGROUND_AV ? DIFFICULTY_HEROIC : DIFFICULTY_NORMAL;

    BattleGroundMap *map = new BattleGroundMap(id, i_gridCleanUpDelay, InstanceId, DIFFICULTY_NORMAL, bg);
    ASSERT(map->IsBattleGroundOrArena());
    bg->SetMap(map);

    //add map into map container
    i_maps[MapID(id, InstanceId)] = map;

    return map;
}
