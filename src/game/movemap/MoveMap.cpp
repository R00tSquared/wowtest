// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
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

#include "GridMap.h"
#include "Log.h"
#include "World.h"

#include "MoveMap.h"
#include "MoveMapSharedDefines.h"

namespace MMAP
{
    // ######################## MMapFactory ########################
    // our global singelton copy
    MMapManager *g_MMapManager = NULL;

    MMapManager* MMapFactory::createOrGetMMapManager()
    {
        if(g_MMapManager == NULL)
            g_MMapManager = new MMapManager();

        return g_MMapManager;
    }

    void MMapFactory::clear()
    {
        if(g_MMapManager)
        {
            delete g_MMapManager;
            g_MMapManager = NULL;
        }
    }

    // ######################## MMapManager ########################
    MMapManager::~MMapManager()
    {
        for (MMapDataSet::iterator i = loadedMMaps.begin(); i != loadedMMaps.end(); ++i)
            delete i->second;

        // by now we should not have maps loaded
        // if we had, tiles in MMapData->mmapLoadedTiles, their actual data is lost!
    }

    bool MMapManager::loadMapData(uint32 mapId)
    {
        loadedMMaps_lock.acquire_read();
        // we already have this map loaded?
        if (loadedMMaps.find(mapId) != loadedMMaps.end())
        {
            loadedMMaps_lock.release();
            return true;
        }
        loadedMMaps_lock.release();

        // load and init dtNavMesh - read parameters from file
        uint32 pathLen = sWorld.GetDataPath().length() + strlen("mmaps/%03i.mmap")+1;
        char *fileName = new char[pathLen];
        snprintf(fileName, pathLen, (sWorld.GetDataPath()+"mmaps/%03i.mmap").c_str(), mapId);

        FILE* file = fopen(fileName, "rb");
        if (!file)
        {
            sLog.outDebug("MMAP:loadMapData: Error: Could not open mmap file '%s'", fileName);
            delete [] fileName;
            return false;
        }

        dtNavMeshParams params;
        fread(&params, sizeof(dtNavMeshParams), 1, file);
        fclose(file);

        dtNavMesh* mesh = dtAllocNavMesh();
        ASSERT(mesh);
        if (DT_SUCCESS != mesh->init(&params))
        {
            dtFreeNavMesh(mesh);
            sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:loadMapData: Failed to initialize dtNavMesh for mmap %03u from file %s", mapId, fileName);
            delete [] fileName;
            return false;
        }

        delete [] fileName;

        sLog.outDetail("MMAP:loadMapData: Loaded %03i.mmap", mapId);

        // store inside our map list
        MMapData* mmap_data = new MMapData(mesh);
        mmap_data->mmapLoadedTiles.clear();

        loadedMMaps_lock.acquire_write();
        if (loadedMMaps.find(mapId) == loadedMMaps.end())
            loadedMMaps.insert(std::pair<uint32, MMapData*>(mapId, mmap_data));
        else
            delete mmap_data;
        loadedMMaps_lock.release();
        return true;
    }

    uint32 MMapManager::packTileID(int32 x, int32 y)
    {
        return uint32(x << 16 | y);
    }

    bool MMapManager::loadMap(uint32 mapId, int32 x, int32 y)
    {
        // make sure the mmap is loaded and ready to load tiles
        if(!loadMapData(mapId))
            return false;

        // get this mmap data
        loadedMMaps_lock.acquire_read();
        MMapData* mmap = loadedMMaps[mapId];
        loadedMMaps_lock.release();
        ASSERT(mmap->navMesh);

        // check if we already have this tile loaded
        uint32 packedGridPos = packTileID(x, y);
        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, mmap->m_mutex_LoadedTiles, false);
        if (mmap->mmapLoadedTiles.find(packedGridPos) != mmap->mmapLoadedTiles.end())
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:loadMap: Asked to load already loaded navmesh tile. %03u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        // load this tile :: mmaps/MMMXXYY.mmtile
        uint32 pathLen = sWorld.GetDataPath().length() + strlen("mmaps/%03i%02i%02i.mmtile")+1;
        char *fileName = new char[pathLen];
        snprintf(fileName, pathLen, (sWorld.GetDataPath()+"mmaps/%03i%02i%02i.mmtile").c_str(), mapId, x, y);

        FILE *file = fopen(fileName, "rb");
        if (!file)
        {
            sLog.outDebug("MMAP:loadMap: Could not open mmtile file '%s'", fileName);
            delete [] fileName;
            return false;
        }
        delete [] fileName;

        // read header
        MmapTileHeader fileHeader;
        fread(&fileHeader, sizeof(MmapTileHeader), 1, file);

        if (fileHeader.mmapMagic != MMAP_MAGIC)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:loadMap: Bad header in mmap %03u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        if (fileHeader.mmapVersion != MMAP_VERSION)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:loadMap: %03u%02i%02i.mmtile was built with generator v%i, expected v%i",
                                                mapId, x, y, fileHeader.mmapVersion, MMAP_VERSION);
            return false;
        }

        unsigned char* data = (unsigned char*)dtAlloc(fileHeader.size, DT_ALLOC_PERM);
        ASSERT(data);

        size_t result = fread(data, fileHeader.size, 1, file);
        if(!result)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:loadMap: Bad header or data in mmap %03u%02i%02i.mmtile", mapId, x, y);
            fclose(file);
            return false;
        }

        fclose(file);

        dtMeshHeader* header = (dtMeshHeader*)data;
        dtTileRef tileRef = 0;

        // memory allocated for data is now managed by detour, and will be deallocated when the tile is removed
        if(DT_SUCCESS == mmap->navMesh->addTile(data, fileHeader.size, DT_TILE_FREE_DATA, 0, &tileRef))
        {
            mmap->mmapLoadedTiles.insert(std::pair<uint32, dtTileRef>(packedGridPos, tileRef));
            ++loadedTiles;
            //sLog.outDetail("MMAP:loadMap: Loaded mmtile %03i[%02i,%02i] into %03i[%02i,%02i]", mapId, x, y, mapId, header->x, header->y);
            //sLog.outLog(LOG_TMP, "MMAP:loadMap: Loaded mmtile %03i[%02i,%02i] into %03i[%02i,%02i]", mapId, x, y, mapId, header->x, header->y);
            return true;
        }
        else
        {
            //sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:loadMap: Could not load %03u%02i%02i.mmtile into navmesh", mapId, x, y);
			//sLog.outLog(LOG_TMP, "ERROR: MMAP:loadMap: Could not load %03u%02i%02i.mmtile into navmesh", mapId, x, y);
            dtFree(data);
            return false;
        }

        return false;
    }

    bool MMapManager::unloadMap(uint32 mapId, int32 x, int32 y)
    {
        // check if we have this map loaded
        if (loadedMMaps.find(mapId) == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            sLog.outDebug("MMAP:unloadMap: Asked to unload not loaded navmesh map. %03u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        MMapData* mmap = loadedMMaps[mapId];

        // check if we have this tile loaded
        uint32 packedGridPos = packTileID(x, y);
        if (mmap->mmapLoadedTiles.find(packedGridPos) == mmap->mmapLoadedTiles.end())
        {
            // file may not exist, therefore not loaded
            sLog.outDebug("MMAP:unloadMap: Asked to unload not loaded navmesh tile. %03u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        dtTileRef tileRef = mmap->mmapLoadedTiles[packedGridPos];

        // unload, and mark as non loaded
        if(DT_SUCCESS != mmap->navMesh->removeTile(tileRef, NULL, NULL))
        {
            // this is technically a memory leak
            // if the grid is later reloaded, dtNavMesh::addTile will return error but no extra memory is used
            // we cannot recover from this error - assert out
            sLog.outLog(LOG_CRASH, "ERROR: MMAP:unloadMap: Could not unload %03u%02i%02i.mmtile from navmesh", mapId, x, y);
            ASSERT(false);
        }
        else
        {
            mmap->mmapLoadedTiles.erase(packedGridPos);
            --loadedTiles;
            sLog.outDetail("MMAP:unloadMap: Unloaded mmtile %03i[%02i,%02i] from %03i", mapId, x, y, mapId);
            return true;
        }

        return false;
    }

    bool MMapManager::unloadMap(uint32 mapId)
    {
        if (loadedMMaps.find(mapId) == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            sLog.outDebug("MMAP:unloadMap: Asked to unload not loaded navmesh map %03u", mapId);
            return false;
        }

        // unload all tiles from given map
        MMapData* mmap = loadedMMaps[mapId];
        for (MMapTileSet::iterator i = mmap->mmapLoadedTiles.begin(); i != mmap->mmapLoadedTiles.end(); ++i)
        {
            uint32 x = (i->first >> 16);
            uint32 y = (i->first & 0x0000FFFF);
            if(DT_SUCCESS != mmap->navMesh->removeTile(i->second, NULL, NULL))
                sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:unloadMap: Could not unload %03u%02i%02i.mmtile from navmesh", mapId, x, y);
            else
            {
                --loadedTiles;
                sLog.outDetail("MMAP:unloadMap: Unloaded mmtile %03i[%02i,%02i] from %03i", mapId, x, y, mapId);
            }
        }

        delete mmap;
        loadedMMaps.erase(mapId);
        sLog.outDetail("MMAP:unloadMap: Unloaded %03i.mmap", mapId);

        return true;
    }

    bool MMapManager::unloadMapInstance(uint32 mapId, uint32 instanceId)
    {
        // check if we have this map loaded
        if (loadedMMaps.find(mapId) == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            sLog.outDebug("MMAP:unloadMapInstance: Asked to unload not loaded navmesh map %03u", mapId);
            return false;
        }

        MMapData* mmap = loadedMMaps[mapId];
        dtNavMeshQuery* navMeshQuery = NULL;
        mmap->m_mutex_navMeshQuery.acquire_write();

        NavMeshQuerySet::iterator it = mmap->navMeshQueries.find(instanceId);
        if (it == mmap->navMeshQueries.end())
        {
            mmap->m_mutex_navMeshQuery.release();
            sLog.outDebug("MMAP:unloadMapInstance: Asked to unload not loaded dtNavMeshQuery mapId %03u instanceId %u", mapId, instanceId);
            return false;
        }
        else
            navMeshQuery = it->second;

        mmap->navMeshQueries.erase(it);
        mmap->m_mutex_navMeshQuery.release();
        dtFreeNavMeshQuery(navMeshQuery);

        sLog.outDetail("MMAP:unloadMapInstance: Unloaded mapId %03u instanceId %u", mapId, instanceId);

        return true;
    }

    dtNavMesh const* MMapManager::GetNavMesh(uint32 mapId)
    {
        if (loadedMMaps.find(mapId) == loadedMMaps.end())
            return NULL;

        return loadedMMaps[mapId]->navMesh;
    }

    dtNavMeshQuery const* MMapManager::GetNavMeshQuery(uint32 mapId, uint32 instanceId)
    {
        if (loadedMMaps.find(mapId) == loadedMMaps.end())
            return NULL;

        MMapData* mmap = loadedMMaps[mapId];
        dtNavMeshQuery* navMeshQuery = NULL;
        mmap->m_mutex_navMeshQuery.acquire_read();

        NavMeshQuerySet::iterator itRead = mmap->navMeshQueries.find(instanceId);
        if (itRead == mmap->navMeshQueries.end())
        {
            mmap->m_mutex_navMeshQuery.release();
            mmap->m_mutex_navMeshQuery.acquire_write();
            // need to recheck, because inbetween release() and acqure_write() someone else could have written navMeshQuery
            NavMeshQuerySet::iterator itWrite = mmap->navMeshQueries.find(instanceId);
            if (itWrite == mmap->navMeshQueries.end())
            {
                // allocate mesh query
                navMeshQuery = dtAllocNavMeshQuery();
                ASSERT(navMeshQuery);

                if (DT_SUCCESS != navMeshQuery->init(mmap->navMesh, 1024))
                {
                    mmap->m_mutex_navMeshQuery.release();
                    dtFreeNavMeshQuery(navMeshQuery);
                    sLog.outLog(LOG_DEFAULT, "ERROR: MMAP:GetNavMeshQuery: Failed to initialize dtNavMeshQuery for mapId %03u instanceId %u", mapId, instanceId);
                    return NULL;
                }

                sLog.outDetail("MMAP:GetNavMeshQuery: created dtNavMeshQuery for mapId %03u instanceId %u", mapId, instanceId);
                mmap->navMeshQueries.insert(std::pair<uint32, dtNavMeshQuery*>(instanceId, navMeshQuery));
            }
            else
                navMeshQuery = itWrite->second;
        }
        else
            navMeshQuery = itRead->second;

        mmap->m_mutex_navMeshQuery.release();

        return navMeshQuery;
    }
}
