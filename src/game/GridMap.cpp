// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
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
//#include "ProgressBar.h"
#include "Log.h"
#include "GridStates.h"
#include "CellImpl.h"
#include "Map.h"
#include "DBCEnums.h"
#include "DBCStores.h"
#include "GridMap.h"
#include "VMapFactory.h"
#include "movemap/MoveMap.h"
#include "World.h"
#include "Database/DatabaseEnv.h"

#include "Util.h"

char const* MAP_MAGIC         = "MAPS";
char const* MAP_VERSION_MAGIC = "v1.2";
char const* MAP_AREA_MAGIC    = "AREA";
char const* MAP_HEIGHT_MAGIC  = "MHGT";
char const* MAP_LIQUID_MAGIC  = "MLIQ";

GridMap::GridMap()
{
    //m_flags = 0;
    _waterNeedsVMap = 2; // 2 by default. Set to false on load from DB if certain grid should not use water VMap
    _specialLiquid = 2; // 2 by default. Set to false on load from DB if certain grid doesnt have lava/slime/Dark_water

    // Area data
    m_gridArea = 0;
    m_area_map = NULL;

    // Height level data
    m_gridHeight = INVALID_HEIGHT_VALUE;
    m_gridGetHeight = &GridMap::getHeightFromFlat;
    m_V9 = NULL;
    m_V8 = NULL;

    // Liquid data
    m_liquidType    = 0;
    m_liquid_offX   = 0;
    m_liquid_offY   = 0;
    m_liquid_width  = 0;
    m_liquid_height = 0;
    m_liquidLevel = INVALID_HEIGHT_VALUE;
    m_liquid_type = NULL;
    m_liquid_map  = NULL;
}

GridMap::~GridMap()
{
    unloadData();
}

bool GridMap::loadData(char *filename, const uint32 mapId, const uint32 x, const uint32 y)
{
    // Unload old data if exist
    unloadData();

    // by default GridMap uses VMap. use=2 is unchecked (using). use=1 means do use. use=0 means turn off
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT `use_vmap`, `special_liquid` FROM `map_water` WHERE `map_id`=%u AND `tile_x`=%u AND `tile_y`=%u LIMIT 1", mapId, x, y);
    if (result)
    {
        _waterNeedsVMap = (*result)[0].GetUInt8();
        _specialLiquid = (*result)[1].GetUInt8();
    }

    GridMapFileHeader header;
    // Not return error if file not found
    FILE *in = fopen(filename, "rb");
    if (!in)
        return true;

    fread(&header, sizeof(header),1,in);
    if (header.mapMagic     == *((uint32 const*)(MAP_MAGIC)) &&
        header.versionMagic == *((uint32 const*)(MAP_VERSION_MAGIC)) &&
        IsAcceptableClientBuild(header.buildMagic))
    {
        // loadup area data
        if (header.areaMapOffset && !loadAreaData(in, header.areaMapOffset, header.areaMapSize))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Error loading map area data\n");
            fclose(in);
            return false;
        }

        // loadup height data
        if (header.heightMapOffset && !loadHeightData(in, header.heightMapOffset, header.heightMapSize))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Error loading map height data\n");
            fclose(in);
            return false;
        }

        // loadup liquid data
        if (header.liquidMapOffset && !loadGridMapLiquidData(in, header.liquidMapOffset, header.liquidMapSize))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Error loading map liquids data\n");
            fclose(in);
            return false;
        }

        fclose(in);
        return true;
    }

    sLog.outLog(LOG_DEFAULT, "ERROR: Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.", filename);
    fclose(in);
    return false;
}

void GridMap::unloadData()
{
    if (m_area_map)
        delete[] m_area_map;

    if (m_V9)
        delete[] m_V9;

    if (m_V8)
        delete[] m_V8;

    if (m_liquid_type)
        delete[] m_liquid_type;

    if (m_liquid_map)
        delete[] m_liquid_map;

    m_area_map = NULL;
    m_V9 = NULL;
    m_V8 = NULL;
    m_liquid_type = NULL;
    m_liquid_map  = NULL;
    m_gridGetHeight = &GridMap::getHeightFromFlat;
}

bool GridMap::loadAreaData(FILE *in, uint32 offset, uint32 /*size*/)
{
    GridMapAreaHeader header;
    fseek(in, offset, SEEK_SET);
    fread(&header, sizeof(header), 1, in);
    if (header.fourcc != *((uint32 const*)(MAP_AREA_MAGIC)))
        return false;

    m_gridArea = header.gridArea;
    if (!(header.flags & MAP_AREA_NO_AREA))
    {
        m_area_map = new uint16 [16*16];
        fread(m_area_map, sizeof(uint16), 16*16, in);
    }

    return true;
}

bool GridMap::loadHeightData(FILE *in, uint32 offset, uint32 /*size*/)
{
    GridMapHeightHeader header;
    fseek(in, offset, SEEK_SET);
    fread(&header, sizeof(header), 1, in);
    if (header.fourcc != *((uint32 const*)(MAP_HEIGHT_MAGIC)))
        return false;

    m_gridHeight = header.gridHeight;
    if (!(header.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        if ((header.flags & MAP_HEIGHT_AS_INT16))
        {
            m_uint16_V9 = new uint16 [129*129];
            m_uint16_V8 = new uint16 [128*128];
            fread(m_uint16_V9, sizeof(uint16), 129*129, in);
            fread(m_uint16_V8, sizeof(uint16), 128*128, in);
            m_gridIntHeightMultiplier = (header.gridMaxHeight - header.gridHeight) / 65535;
            m_gridGetHeight = &GridMap::getHeightFromUint16;
        }
        else if ((header.flags & MAP_HEIGHT_AS_INT8))
        {
            m_uint8_V9 = new uint8 [129*129];
            m_uint8_V8 = new uint8 [128*128];
            fread(m_uint8_V9, sizeof(uint8), 129*129, in);
            fread(m_uint8_V8, sizeof(uint8), 128*128, in);
            m_gridIntHeightMultiplier = (header.gridMaxHeight - header.gridHeight) / 255;
            m_gridGetHeight = &GridMap::getHeightFromUint8;
        }
        else
        {
            m_V9 = new float [129*129];
            m_V8 = new float [128*128];
            fread(m_V9, sizeof(float), 129*129, in);
            fread(m_V8, sizeof(float), 128*128, in);
            m_gridGetHeight = &GridMap::getHeightFromFloat;
        }
    }
    else
        m_gridGetHeight = &GridMap::getHeightFromFlat;

    return true;
}

bool GridMap::loadGridMapLiquidData(FILE *in, uint32 offset, uint32 /*size*/)
{
    GridMapLiquidHeader header;
    fseek(in, offset, SEEK_SET);
    fread(&header, sizeof(header), 1, in);
    if (header.fourcc != *((uint32 const*)(MAP_LIQUID_MAGIC)))
        return false;

    m_liquidType    = header.liquidType;
    m_liquid_offX   = header.offsetX;
    m_liquid_offY   = header.offsetY;
    m_liquid_width  = header.width;
    m_liquid_height = header.height;
    m_liquidLevel   = header.liquidLevel;

    if (!(header.flags & MAP_LIQUID_NO_TYPE))
    {
        m_liquid_type = new uint8 [16*16];
        fread(m_liquid_type, sizeof(uint8), 16*16, in);
    }

    if (!(header.flags & MAP_LIQUID_NO_HEIGHT))
    {
        m_liquid_map = new float [m_liquid_width*m_liquid_height];
        fread(m_liquid_map, sizeof(float), m_liquid_width*m_liquid_height, in);
    }

    return true;
}

uint16 GridMap::getArea(float x, float y)
{
    if (!m_area_map)
        return m_gridArea;

    x = 16 * (32 - x/SIZE_OF_GRIDS);
    y = 16 * (32 - y/SIZE_OF_GRIDS);
    int lx = (int)x & 15;
    int ly = (int)y & 15;
    return m_area_map[lx*16 + ly];
}

float GridMap::getHeightFromFlat(float /*x*/, float /*y*/) const
{
    return m_gridHeight;
}

float GridMap::getHeightFromFloat(float x, float y) const
{
    if (!m_V8 || !m_V9)
        return m_gridHeight;

    x = MAP_RESOLUTION * (32 - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y/SIZE_OF_GRIDS);

    int x_int = (int)x;
    int y_int = (int)y;
    x -= x_int;
    y -= y_int;
    x_int&=(MAP_RESOLUTION - 1);
    y_int&=(MAP_RESOLUTION - 1);

    // Height stored as: h5 - its v8 grid, h1-h4 - its v9 grid
    // +--------------> X
    // | h1-------h2     Coordinates is:
    // | | \  1  / |     h1 0,0
    // | |  \   /  |     h2 0,1
    // | | 2  h5 3 |     h3 1,0
    // | |  /   \  |     h4 1,1
    // | | /  4  \ |     h5 1/2,1/2
    // | h3-------h4
    // V Y
    // For find height need
    // 1 - detect triangle
    // 2 - solve linear equation from triangle points
    // Calculate coefficients for solve h = a*x + b*y + c

    float a,b,c;
    // Select triangle:
    if (x+y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = m_V9[(x_int)*129 + y_int];
            float h2 = m_V9[(x_int+1)*129 + y_int];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h2-h1;
            b = h5-h1-h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = m_V9[x_int*129 + y_int  ];
            float h3 = m_V9[x_int*129 + y_int+1];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = m_V9[(x_int+1)*129 + y_int  ];
            float h4 = m_V9[(x_int+1)*129 + y_int+1];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = m_V9[(x_int)*129 + y_int+1];
            float h4 = m_V9[(x_int+1)*129 + y_int+1];
            float h5 = 2 * m_V8[x_int*128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return a * x + b * y + c;
}

float GridMap::getHeightFromUint8(float x, float y) const
{
    if (!m_uint8_V8 || !m_uint8_V9)
        return m_gridHeight;

    x = MAP_RESOLUTION * (32 - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y/SIZE_OF_GRIDS);

    int x_int = (int)x;
    int y_int = (int)y;
    x -= x_int;
    y -= y_int;
    x_int&=(MAP_RESOLUTION - 1);
    y_int&=(MAP_RESOLUTION - 1);

    int32 a, b, c;
    uint8 *V9_h1_ptr = &m_uint8_V9[x_int*128 + x_int + y_int];
    if (x+y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32 h1 = V9_h1_ptr[  0];
            int32 h2 = V9_h1_ptr[129];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h2-h1;
            b = h5-h1-h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32 h1 = V9_h1_ptr[0];
            int32 h3 = V9_h1_ptr[1];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32 h2 = V9_h1_ptr[129];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32 h3 = V9_h1_ptr[  1];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint8_V8[x_int*128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return (float)((a * x) + (b * y) + c)*m_gridIntHeightMultiplier + m_gridHeight;
}

float GridMap::getHeightFromUint16(float x, float y) const
{
    if (!m_uint16_V8 || !m_uint16_V9)
        return m_gridHeight;

    x = MAP_RESOLUTION * (32 - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y/SIZE_OF_GRIDS);

    int x_int = (int)x;
    int y_int = (int)y;
    x -= x_int;
    y -= y_int;
    x_int&=(MAP_RESOLUTION - 1);
    y_int&=(MAP_RESOLUTION - 1);

    int32 a, b, c;
    uint16 *V9_h1_ptr = &m_uint16_V9[x_int*128 + x_int + y_int];
    if (x+y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32 h1 = V9_h1_ptr[  0];
            int32 h2 = V9_h1_ptr[129];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h2-h1;
            b = h5-h1-h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32 h1 = V9_h1_ptr[0];
            int32 h3 = V9_h1_ptr[1];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32 h2 = V9_h1_ptr[129];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32 h3 = V9_h1_ptr[  1];
            int32 h4 = V9_h1_ptr[130];
            int32 h5 = 2 * m_uint16_V8[x_int*128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return (float)((a * x) + (b * y) + c)*m_gridIntHeightMultiplier + m_gridHeight;
}

/*float GridMap::getLiquidLevel(float x, float y)
{
    if (!m_liquid_map)
        return m_liquidLevel;

    x = MAP_RESOLUTION * (32 - x/SIZE_OF_GRIDS);
    y = MAP_RESOLUTION * (32 - y/SIZE_OF_GRIDS);

    int cx_int = ((int)x & (MAP_RESOLUTION-1)) - m_liquid_offY;
    int cy_int = ((int)y & (MAP_RESOLUTION-1)) - m_liquid_offX;

    if (cx_int < 0 || cx_int >=m_liquid_height)
        return INVALID_HEIGHT;
    if (cy_int < 0 || cy_int >=m_liquid_width)
        return INVALID_HEIGHT;

    return m_liquid_map[cx_int*m_liquid_width + cy_int];
}*/

/*uint8 GridMap::getTerrainType(float x, float y)
{
    if (!m_liquid_type)
        return (uint8)m_liquidType;

    x = 16 * (32 - x/SIZE_OF_GRIDS);
    y = 16 * (32 - y/SIZE_OF_GRIDS);
    int lx = (int)x & 15;
    int ly = (int)y & 15;
    return m_liquid_type[lx*16 + ly];
}*/

// Get water state on map
GridMapLiquidStatus GridMap::getLiquidStatus(float x, float y, float z, GridMapLiquidData *data)
{
    // Check water type (if no water return)
    if (!m_liquid_type && !m_liquidType)
        return LIQUID_MAP_NO_WATER;

    // Get cell
    float cx = MAP_RESOLUTION * (32 - x/SIZE_OF_GRIDS);
    float cy = MAP_RESOLUTION * (32 - y/SIZE_OF_GRIDS);

    int x_int = (int)cx & (MAP_RESOLUTION-1);
    int y_int = (int)cy & (MAP_RESOLUTION-1);

    // Check water type in cell. At this moment either (m_liquid_type exists) OR (m_liquidType exists and is not 0)
    uint8 type = m_liquid_type ? m_liquid_type[(x_int>>3)*16 + (y_int>>3)] : m_liquidType;
    if (!(MAP_ALL_LIQUIDS & type))
        return LIQUID_MAP_NO_WATER;

    // Check water level:
    // Check water height map
    int lx_int = x_int - m_liquid_offY;
    int ly_int = y_int - m_liquid_offX;
    if (lx_int < 0 || lx_int >=m_liquid_height)
        return LIQUID_MAP_NO_WATER;
    if (ly_int < 0 || ly_int >=m_liquid_width)
        return LIQUID_MAP_NO_WATER;

    // Get water level
    float liquid_level = m_liquid_map ? m_liquid_map[lx_int*m_liquid_width + ly_int] : m_liquidLevel;
    // Get ground level (sub 0.2 for fix some errors)
    float ground_level = getHeight(x, y);

    // Check water level and ground level
    if (liquid_level < ground_level || z < ground_level - 2)
        return LIQUID_MAP_NO_WATER;

    // All ok in water -> store data
    if (data)
    {
        data->type  = type;
        data->level = liquid_level;
        data->depth_level = ground_level;
    }

    // For speed check as int values
    int delta = int((liquid_level - z) * 10);

    // Get position delta
    if (delta > 20)                                         // Under water
        return LIQUID_MAP_UNDER_WATER;
    if (delta > 0)                                          // In water
        return LIQUID_MAP_IN_WATER;
    if (delta > -1)                                         // Walk on water
        return LIQUID_MAP_WATER_WALK;
                                                            // Above water
    return LIQUID_MAP_ABOVE_WATER;
}

bool GridMap::ExistMap(uint32 mapid,int gx,int gy)
{
    int len = sWorld.GetDataPath().length()+strlen("maps/%03u%02u%02u.map")+1;
    char* tmp = new char[len];
    snprintf(tmp, len, (char *)(sWorld.GetDataPath()+"maps/%03u%02u%02u.map").c_str(),mapid,gx,gy);

    FILE *pf=fopen(tmp,"rb");

    if(!pf)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Check existing of map file '%s': not exist!",tmp);
        delete[] tmp;
        return false;
    }

    GridMapFileHeader header;
    fread(&header, sizeof(header), 1, pf);
    if (header.mapMagic     != *((uint32 const*)(MAP_MAGIC)) ||
        header.versionMagic != *((uint32 const*)(MAP_VERSION_MAGIC)) ||
        !IsAcceptableClientBuild(header.buildMagic))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Map file '%s' is non-compatible version (outdated?). Please, create new using ad.exe program.",tmp);
        delete [] tmp;
        fclose(pf);                                         //close file before return
        return false;
    }

    delete [] tmp;
    fclose(pf);
    return true;
}
bool GridMap::ExistVMap(uint32 mapid,int gx,int gy)
{
    if (VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager())
    {
        // x and y are swapped !! => fixed now
        bool exists = vmgr->existsMap((sWorld.GetDataPath()+ "vmaps").c_str(), mapid, gx,gy);
        if (!exists)
        {
            std::string name = vmgr->getDirFileName(mapid,gx,gy);
            sLog.outLog(LOG_DEFAULT, "ERROR: VMap file '%s' is missing or point to wrong version vmap file, redo vmaps with latest vmap_assembler.exe program", (sWorld.GetDataPath()+"vmaps/"+name).c_str());
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
TerrainInfo::TerrainInfo(uint32 mapid, TerrainSpecifics terrainspecifics) : m_mapId(mapid)
{
    for (int k = 0; k < MAX_NUMBER_OF_GRIDS; ++k)
    {
        for (int i = 0; i < MAX_NUMBER_OF_GRIDS; ++i)
        {
            m_GridMaps[i][k] = NULL;
            m_GridRef[i][k] = 0;
        }
    }

    //clean up GridMap objects every minute
    const uint32 iCleanUpInterval = 60;
    //schedule start randomly
    const uint32 iRandomStart = urand(20, 40);

    i_timer.SetInterval(iCleanUpInterval * 1000);
    i_timer.SetCurrent(iRandomStart * 1000);

    m_specifics = new MapTemplate(terrainspecifics);
}

TerrainInfo::~TerrainInfo()
{
     for (int k = 0; k < MAX_NUMBER_OF_GRIDS; ++k)
         for (int i = 0; i < MAX_NUMBER_OF_GRIDS; ++i)
             delete m_GridMaps[i][k];

     VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(m_mapId);
     MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(m_mapId);

     delete m_specifics;
     m_specifics = nullptr;
}

GridMap * TerrainInfo::Load(const uint32 x, const uint32 y)
{
     ASSERT(x < MAX_NUMBER_OF_GRIDS);
     ASSERT(y < MAX_NUMBER_OF_GRIDS);

     //reference grid as a first step
     RefGrid(x, y);

     //quick check if GridMap already loaded
     GridMap * pMap = m_GridMaps[x][y];
     if(!pMap)
         pMap = LoadMapAndVMap(x, y);

     return pMap;
}

//schedule lazy GridMap object cleanup
void TerrainInfo::Unload(const uint32 x, const uint32 y)
{
     ASSERT(x < MAX_NUMBER_OF_GRIDS);
     ASSERT(y < MAX_NUMBER_OF_GRIDS);

     if(m_GridMaps[x][y])
     {
         //decrease grid reference count...
         if(UnrefGrid(x, y) == 0)
         {
             //TODO: add your additional logic here

         }
     }
}

//call this method only
void TerrainInfo::CleanUpGrids(const uint32 diff)
{
     if (!i_timer.Expired(diff))
         return;

     for (int y = 0; y < MAX_NUMBER_OF_GRIDS; ++y)
     {
         for (int x = 0; x < MAX_NUMBER_OF_GRIDS; ++x)
         {
             const int16& iRef = m_GridRef[x][y];
             GridMap * pMap = m_GridMaps[x][y];

             //delete those GridMap objects which have refcount = 0
             if(pMap && iRef == 0 )
             {
                 m_GridMaps[x][y] = NULL;
                 //delete grid data if reference count == 0
                 pMap->unloadData();
                 delete pMap;

                 //unload VMAPS...
                 VMAP::VMapFactory::createOrGetVMapManager()->unloadMap(m_mapId, x, y);
                 MMAP::MMapFactory::createOrGetMMapManager()->unloadMap(m_mapId, x, y);
             }
         }
     }

     i_timer.SetCurrent(0);
}

int TerrainInfo::RefGrid(const uint32& x, const uint32& y)
{
     ASSERT(x < MAX_NUMBER_OF_GRIDS);
     ASSERT(y < MAX_NUMBER_OF_GRIDS);

     LOCK_GUARD _lock(m_refMutex);
     return (m_GridRef[x][y] += 1);
}

int TerrainInfo::UnrefGrid(const uint32& x, const uint32& y)
{
     ASSERT(x < MAX_NUMBER_OF_GRIDS);
     ASSERT(y < MAX_NUMBER_OF_GRIDS);

     int16& iRef = m_GridRef[x][y];

     LOCK_GUARD _lock(m_refMutex);
     if(iRef > 0)
         return (iRef -= 1);

     return 0;
}

float TerrainInfo::GetHeight(float x, float y, float z, bool pUseVmaps, float maxSearchDist) const
{
	// THIS IS BUGGED! Creatures may fall under textures like this one 19294 (.npc follow)
	//// find raw .map surface under Z coordinates
	//float mapHeight = VMAP_INVALID_HEIGHT_VALUE;            // Store Height obtained by maps
	//float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;           // Store Height obtained by vmaps (in "corridor" of z (or slightly above z)

	//float z2 = z + 2.f;
	//if (GridMap *gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
	//{
	//	float _mapheight = gmap->getHeight(x, y);

	//	// look from a bit higher pos to find the floor, ignore under surface case
	//	if (z2 > _mapheight)
	//		mapHeight = _mapheight;
	//}

	//if (pUseVmaps)
	//{
	//	VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
	//	// if mapHeight has been found search vmap height at least until mapHeight point
	//	// this prevent case when original Z "too high above ground and vmap height search fail"
	//	// this will not affect most normal cases (no map in instance, or stay at ground at continent)
	//	if (mapHeight > INVALID_HEIGHT && z2 - mapHeight > maxSearchDist)
	//		maxSearchDist = z2 - mapHeight + 1.0f;      // 1.0 make sure that we not fail for case when map height near but above for vamp height

	//	// look from a bit higher pos to find the floor
	//	vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, maxSearchDist);

	//	// if not found in expected range, look for infinity range (case of far above floor, but below terrain-height)
	//	if (vmapHeight <= INVALID_HEIGHT)
	//		vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, 10000.0f);

	//	// look upwards
	//	if (vmapHeight <= INVALID_HEIGHT && mapHeight > z2 && std::abs(z2 - mapHeight) > 30.f)
	//		vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, -maxSearchDist);

	//	// still not found, look near terrain height
	//	if (vmapHeight <= INVALID_HEIGHT && mapHeight > INVALID_HEIGHT && z2 < mapHeight)
	//		vmapHeight = vmgr->getHeight(GetMapId(), x, y, mapHeight + 2.0f, DEFAULT_HEIGHT_SEARCH);
	//}

	//// mapHeight set for any above raw ground Z or <= INVALID_HEIGHT
	//// vmapheight set for any under Z value or <= INVALID_HEIGHT
	//if (vmapHeight > INVALID_HEIGHT)
	//{
	//	if (mapHeight > INVALID_HEIGHT)
	//	{
	//		// we have mapheight and vmapheight and must select more appropriate

	//		// we are already under the surface or vmap height above map heigt
	//		// or if the distance of the vmap height is less the land height distance

	//		/*ERRORS: Happenned with "z < mapHeight && vmapHeight < mapHeight"
	//		Poly PathFinder:
	//	   In DuskWood:
	//	   x -10347.3643, y -493.415314, diff beween map and vmap: 1.01816177, actual map height 51.5365257, poly z 50.5183640, will return vmap 23.8834515 --
	//			   poly position is 1.01 under the map height. MMAP problem. Diff 1.01 Is the biggest to be found

	//	   In WSG players run client-side under the mapheight, but only for < 0.1,
	//		   but still GetHeight then returns closest point under the ground. This is "dropping into the tunnel" bug.

	//		   Thus for checking if we're under the map we're using z2, which is 2 yds higher than our actual z, so if we're a little under the ground -> we won't fall more*/

	//		if (z2 < mapHeight || vmapHeight > mapHeight || fabs(mapHeight - z) > fabs(vmapHeight - z))
	//			return vmapHeight;
	//		else
	//			return mapHeight;                           // better use .map surface height
	//	}
	//	else
	//		return vmapHeight;                              // we have only vmapHeight (if have)
	//}

	//return mapHeight;

	float mapHeight = VMAP_INVALID_HEIGHT_VALUE;            // Store Height obtained by maps
	float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;           // Store Height obtained by vmaps (in "corridor" of z (or slightly above z)

	float z2 = z + 2.f;

	// find raw .map surface under Z coordinates (or well-defined above)
	if (GridMap* gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
		mapHeight = gmap->getHeight(x, y);

	if (pUseVmaps)
	{
		VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
		// if mapHeight has been found search vmap height at least until mapHeight point
		// this prevent case when original Z "too high above ground and vmap height search fail"
		// this will not affect most normal cases (no map in instance, or stay at ground at continent)
		if (mapHeight > INVALID_HEIGHT && z2 - mapHeight > maxSearchDist)
			maxSearchDist = z2 - mapHeight + 1.0f;      // 1.0 make sure that we not fail for case when map height near but above for vamp height

		// look from a bit higher pos to find the floor
		vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, maxSearchDist);

		// if not found in expected range, look for infinity range (case of far above floor, but below terrain-height)
		if (vmapHeight <= INVALID_HEIGHT)
			vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, 10000.0f);

		// look upwards
		if (vmapHeight <= INVALID_HEIGHT && mapHeight > z2 && std::abs(z2 - mapHeight) > 30.f)
			vmapHeight = vmgr->getHeight(GetMapId(), x, y, z2, -maxSearchDist);

		// still not found, look near terrain height
		if (vmapHeight <= INVALID_HEIGHT && mapHeight > INVALID_HEIGHT && z2 < mapHeight)
			vmapHeight = vmgr->getHeight(GetMapId(), x, y, mapHeight + 2.0f, DEFAULT_HEIGHT_SEARCH);
	}

	// mapHeight set for any above raw ground Z or <= INVALID_HEIGHT
	// vmapheight set for any under Z value or <= INVALID_HEIGHT
	if (vmapHeight > INVALID_HEIGHT)
	{
		if (mapHeight > INVALID_HEIGHT)
		{
			// we have mapheight and vmapheight and must select more appropriate

			// we are already under the surface or vmap height above map heigt
			if (z2 < mapHeight || vmapHeight > mapHeight || fabs(mapHeight - z) > fabs(vmapHeight - z))
			/*if (z < mapHeight || vmapHeight > mapHeight)*/
				return vmapHeight;
			return mapHeight;
			// better use .map surface height
		}
		else
			return vmapHeight;                              // we have only vmapHeight (if have)
	}

	return mapHeight;
}

bool TerrainInfo::getGridGroundHitPos(float x, float y, float z, float& rx, float& ry, float& rz, float angleCos, float angleSin, float dist, float checkForHeight) const
{
    GridMap *s_gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y);
    GridMap *e_gmap = const_cast<TerrainInfo*>(this)->GetGrid(rx, ry);

    bool sameGrid = false;
    float startGHeight;
    float endGHeight;
    if (s_gmap)
    {
        startGHeight = s_gmap->getHeight(x, y);
        if (s_gmap == e_gmap)
            sameGrid = true;
    }
    else
        startGHeight = VMAP_INVALID_HEIGHT_VALUE;

    if (e_gmap)
        endGHeight = s_gmap->getHeight(rx, ry);
    else
        endGHeight = VMAP_INVALID_HEIGHT_VALUE;

    // if start or end is on gridGround - then go on checking
    if (((z + 2.0f > startGHeight) && (z - 2.0f < startGHeight)) ||
        (((z + 2.0f > endGHeight) && (z - 2.0f < endGHeight))))
    {
        uint32 stepCount = ceil(dist); // 1 step for <= 1 yd. 2 steps for <= 2 yds, etc.
        float step = dist / (float)stepCount;
        float stepZ = (rz - z) / stepCount;

        float nextX = x;
        float nextY = y;
        float nextZ = z + checkForHeight;
        for (uint32 j = 0; j < stepCount; ++j)
        {
            nextX += step * angleCos;
            nextY += step * angleSin;
            nextZ += stepZ;

            GridMap* tempGrid = sameGrid ? s_gmap : const_cast<TerrainInfo*>(this)->GetGrid(nextX, nextY);

            float nextGridHeight = tempGrid->getHeight(nextX, nextY);
            if (nextGridHeight > nextZ)
            {
                if (nextGridHeight < nextZ + 2.0f) // all slopes with 63.4 degree slope or lower will not cause MAX_PRECISION checks. So most of them will be stopped here.
                {
                    // apply last allowed pos
                    rx = x;
                    ry = y;
                    rz = z;
                    return true;
                }
                else // we can be entering into the cave or it's just a very sharp slope, need to increase the precision
                {
                    // if we're here, then angle to this new ground pos is > 89 degrees. So no hit is found and result pos remains unchanged.
                    if (step <= GRID_GROUND_HIT_POS_MAX_PRECISION)
                        return false;

                    // update the precision, "+1" is added, because on next cycle check j will be increased by 1, so stepCount should also be bigger by 1
                    stepCount = ceil(step / GRID_GROUND_HIT_POS_MAX_PRECISION) + 1; // 1 step for <= GRID_GROUND_HIT_POS_MAX_PRECISION, etc. 
                    step = step / (float)stepCount;
                    stepZ = (nextZ - z) / stepCount;
                    j = 0;

                    // move nextPoint to previous allowed pos
                    nextX = x;
                    nextY = y;
                    nextZ = z;
                    continue;
                }
            }

            // these are checked and fine now. Will be "currently allowed" position
            x = nextX;
            y = nextY;
            z = nextZ;
        }
    }

    return false; // no hit found
}

inline bool IsOutdoorWMO(uint32 mogpFlags, uint32 mapid)
{
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    return (mogpFlags & 0x8000) || (mapEntry->Expansion() && mogpFlags & 0x8);
}

bool TerrainInfo::IsOutdoors(float x, float y, float z) const
{
     uint32 mogpFlags;
     int32 adtId, rootId, groupId;

     // no wmo found? -> outside by default
     if (!GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
         return true;

     return IsOutdoorWMO(mogpFlags, m_mapId);
}

bool TerrainInfo::GetAreaInfo(float x, float y, float z, uint32 &flags, int32 &adtId, int32 &rootId, int32 &groupId) const
{
     float vmap_z = z;
     VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
     if (vmgr->getAreaInfo(GetMapId(), x, y, vmap_z, flags, adtId, rootId, groupId))
     {
         // check if there's terrain between player height and object height
         if(GridMap *gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
         {
             float _mapheight = gmap->getHeight(x,y);
             // z + 2.0f condition taken from GetHeight(), not sure if it's such a great choice...
             if(z + 2.0f > _mapheight &&  _mapheight > vmap_z)
                 return false;
         }
         return true;
     }
     return false;
}

uint16 TerrainInfo::GetAreaFlag(float x, float y, float z, bool *isOutdoors) const
{
     uint32 mogpFlags;
     int32 adtId, rootId, groupId;
     WMOAreaTableEntry const* wmoEntry = 0;
     AreaTableEntry const* atEntry = 0;
     bool haveAreaInfo = false;

     if(GetAreaInfo(x, y, z, mogpFlags, adtId, rootId, groupId))
     {
         haveAreaInfo = true;
         wmoEntry = GetWMOAreaTableEntryByTripple(rootId, adtId, groupId);
         if(wmoEntry)
             atEntry = GetAreaEntryByAreaID(wmoEntry->areaId);
     }

     uint16 areaflag;
     if (atEntry)
         areaflag = atEntry->exploreFlag;
     else
     {
         if(GridMap *gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
             areaflag = gmap->getArea(x, y);
         // this used while not all *.map files generated (instances)
         else
             areaflag = GetAreaFlagByMapId(GetMapId());
     }

     if (isOutdoors)
     {
         if (haveAreaInfo)
             *isOutdoors = IsOutdoorWMO(mogpFlags, m_mapId);
         else
             *isOutdoors = true;
     }
     return areaflag;
}

/*uint8 TerrainInfo::GetTerrainType(float x, float y ) const
{
     if(GridMap *gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
         return gmap->getTerrainType(x, y);
     else
         return 0;
}*/

uint8 TerrainInfo::waterNeedsVMap(float x, float y) const
{
    if (GridMap *gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
        return gmap->waterNeedsVMap();
    else
        return 0;
}

uint8 TerrainInfo::hasSpecialLiquid(float x, float y) const
{
    if (GridMap *gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
        return gmap->hasSpecialLiquid();
    else
        return 0;
}

uint32 TerrainInfo::GetAreaId(float x, float y, float z) const
{
     return TerrainManager::GetAreaIdByAreaFlag(GetAreaFlag(x,y,z),m_mapId);
}

uint32 TerrainInfo::GetZoneId(float x, float y, float z) const
{
     return TerrainManager::GetZoneIdByAreaFlag(GetAreaFlag(x,y,z),m_mapId);
}

void TerrainInfo::GetZoneAndAreaId(uint32& zoneid, uint32& areaid, float x, float y, float z) const
{
     TerrainManager::GetZoneAndAreaIdByAreaFlag(zoneid,areaid,GetAreaFlag(x,y,z),m_mapId);
}

bool TerrainInfo::getLiquidStatusVMapOnlyForTest(float x, float y, float z, GridMapLiquidData *data) const
{
    // Check surface in x, y point for liquid
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    float liquid_level, ground_level = INVALID_HEIGHT_VALUE;
    uint32 liquid_type;
    if (vmgr->GetLiquidLevel(GetMapId(), x, y, z, MAP_ALL_LIQUIDS, liquid_level, ground_level, liquid_type))
    {
        if (data)
        {
            data->type = liquid_type;
            data->level = liquid_level;
            data->depth_level = ground_level;
        }

        // Check water level and ground level
        if (liquid_level > ground_level)
            return true;
    }
    return false;
}

GridMapLiquidStatus TerrainInfo::getLiquidStatusIfSwimmingOrSpecial(float x, float y, float z, GridMapLiquidData *data, bool isSwimming) const
{
    // Check surface in x, y point for liquid
    if (GridMap* grid = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        // if grid has special liquid like lava/slime/dark_water -> always check.
        // or check if we're swimming in a normal water
        if (isSwimming || grid->hasSpecialLiquid())
            return grid->waterNeedsVMap() ? getLiquidStatusWithVMap(x, y, z, data) : grid->getLiquidStatus(x, y, z, data);
    }

    return LIQUID_MAP_NO_WATER;
}

GridMapLiquidStatus TerrainInfo::getLiquidStatusWithVMap(float x, float y, float z, GridMapLiquidData *data) const
{
     GridMapLiquidStatus result = LIQUID_MAP_NO_WATER;
     VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
     float liquid_level, ground_level = INVALID_HEIGHT_VALUE;
     uint32 liquid_type;
     // ground_level is changed only if GetLiquidLevel() returns true. Need to know that for gmap->getLiquidStatus() call
     if (vmgr->GetLiquidLevel(GetMapId(), x, y, z, MAP_ALL_LIQUIDS, liquid_level, ground_level, liquid_type))
     {
         debug_log("getLiquidStatus(): vmap liquid level: %f ground: %f type: %u", liquid_level, ground_level, liquid_type);
         // Check water level and ground level
         if (liquid_level > ground_level && z > ground_level - 2)
         {
             // All ok in water -> store data
             if (data)
             {
                 data->type  = liquid_type;
                 data->level = liquid_level;
                 data->depth_level = ground_level;
             }

             // For speed check as int values
             int delta = int((liquid_level - z) * 10);

             // Get position delta
             if (delta > 20)                   // Under water. > 2.0f under water
                 return LIQUID_MAP_UNDER_WATER;
             if (delta > 0 )                   // In water. Still under water level (it may be that only our feet are in water)
                 return LIQUID_MAP_IN_WATER;
             if (delta > -1)                   // Walk on water. Above water.
                 return LIQUID_MAP_WATER_WALK;
             result = LIQUID_MAP_ABOVE_WATER; // Much above water
         }
     }
     if(GridMap* gmap = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
     {
         GridMapLiquidData map_data;
         GridMapLiquidStatus map_result = gmap->getLiquidStatus(x, y, z, &map_data);
         // Not override LIQUID_MAP_ABOVE_WATER with LIQUID_MAP_NO_WATER:
         if (map_result != LIQUID_MAP_NO_WATER && (map_data.level > ground_level))
         {
             if (data)
                 *data = map_data;
             return map_result;
         }
     }
     return result;
}

bool TerrainInfo::IsInWater(float x, float y, float pZ) const
{
    // Check surface in x, y point for liquid
    if (GridMap* grid = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        if (GridMapLiquidStatus s = (grid->waterNeedsVMap() ? getLiquidStatusWithVMap(x, y, pZ) : grid->getLiquidStatus(x, y, pZ)))
            return s == LIQUID_MAP_IN_WATER || s == LIQUID_MAP_UNDER_WATER;
    }
    return false;
}

bool TerrainInfo::IsInWaterOrSlightlyAbove(float x, float y, float pZ) const
{
    // Check surface in x, y point for liquid
    if (GridMap* grid = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        pZ -= 1;
        if (GridMapLiquidStatus s = (grid->waterNeedsVMap() ? getLiquidStatusWithVMap(x, y, pZ) : grid->getLiquidStatus(x, y, pZ)))
            return s == LIQUID_MAP_IN_WATER || s == LIQUID_MAP_UNDER_WATER;
    }
    return false;
}

/**
 * Function find higher form water or ground height for current floor
 *
 * @param x, y, z    Coordinates original point at floor level
 *
 * @param pGround    optional arg for retrun calculated by function work ground height, it let avoid in caller code recalculate height for point if it need
 *
 * @param swim       z coordinate can be calculated for select above/at or under z coordinate (for fly or swim/walking by bottom)
 *                   in last cases for in water returned under water height for avoid client set swimming unit as saty at water.
 *
 * @return           calculated z coordinate
 */
float TerrainInfo::GetWaterOrGroundLevel(float x, float y, float z, float* pGround /*= NULL*/, bool swim /*= false*/) const
{
    if (GridMap* grid = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = GetHeight(x, y, z, true, DEFAULT_WATER_SEARCH); // actually it's floor. Or it could be said that it's ground for our water
        if (pGround)
            *pGround = ground_z;

        GridMapLiquidData liquid_status;

        GridMapLiquidStatus res = grid->waterNeedsVMap() ? getLiquidStatusWithVMap(x, y, ground_z, &liquid_status) :
                                                     grid->getLiquidStatus(x, y, ground_z, &liquid_status);
        if (res)
        {
            float liquidZ = swim ? liquid_status.level - 2.0f : liquid_status.level;
            if (liquidZ > ground_z)
                return liquidZ;
        }
        return ground_z;
    }

    return VMAP_INVALID_HEIGHT_VALUE;
}

GridMap* TerrainInfo::GetGrid(const float x, const float y)
{
    // half opt method
    uint32 gx = uint32(32 - x / SIZE_OF_GRIDS);                       //grid x
    uint32 gy = uint32(32 - y / SIZE_OF_GRIDS);                       //grid y

    if (gx > MAX_NUMBER_OF_GRIDS)
        return NULL;

    if (gy > MAX_NUMBER_OF_GRIDS)
        return NULL;

    //quick check if GridMap already loaded
    GridMap * pMap = m_GridMaps[gx][gy];
    if(!pMap)
         pMap = LoadMapAndVMap(gx, gy);

    return pMap;
}

GridMap* TerrainInfo::LoadMapAndVMap(const uint32 x, const uint32 y)
{
    //double checked lock pattern
    if(!m_GridMaps[x][y])
    {
        LOCK_GUARD lock(m_mutex);

        if(!m_GridMaps[x][y])
        {
            GridMap * map = new GridMap();

            // map file name
            char *tmp=NULL;
            int len = sWorld.GetDataPath().length()+strlen("maps/%03u%02u%02u.map")+1;
            tmp = new char[len];
            snprintf(tmp, len, (char *)(sWorld.GetDataPath()+"maps/%03u%02u%02u.map").c_str(),m_mapId, x, y);
            sLog.outDetail("Loading map %s",tmp);

            if(!map->loadData(tmp, m_mapId, x, y))
            {
                sLog.outLog(LOG_CRASH, "ERROR: Error load map file: \n %s\n", tmp);
                //ASSERT(false);
            }

            delete [] tmp;

            //load VMAPs for current map/grid...
            const MapEntry * i_mapEntry = sMapStore.LookupEntry(m_mapId);
            const char* mapName = i_mapEntry ? i_mapEntry->name[sWorld.GetDefaultDbcLocale()] : "UNNAMEDMAP\x0";

            int vmapLoadResult = VMAP::VMapFactory::createOrGetVMapManager()->loadMap((sWorld.GetDataPath()+ "vmaps").c_str(),  m_mapId, x, y);
            switch(vmapLoadResult)
            {
            case VMAP::VMAP_LOAD_RESULT_OK:
               sLog.outDetail("VMAP loaded name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, m_mapId, x,y,x,y);                break;
            case VMAP::VMAP_LOAD_RESULT_ERROR:
                sLog.outDetail("Could not load VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, m_mapId, x, y,x,y);
                break;
            case VMAP::VMAP_LOAD_RESULT_IGNORED:
                debug_log("Ignored VMAP name:%s, id:%d, x:%d, y:%d (vmap rep.: x:%d, y:%d)", mapName, m_mapId, x,y,x,y);
                break;
            }

            MMAP::MMapFactory::createOrGetMMapManager()->loadMap(m_mapId, x, y);

            m_GridMaps[x][y] = map;
        }
    }

    return  m_GridMaps[x][y];
}

float TerrainInfo::GetWaterLevel(float x, float y, float z, float* pGround /*= NULL*/) const
{
    if (GridMap* grid = const_cast<TerrainInfo*>(this)->GetGrid(x, y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = GetHeight(x, y, z, true, DEFAULT_WATER_SEARCH);
        if (pGround)
            *pGround = ground_z;

        GridMapLiquidData liquid_status;

        GridMapLiquidStatus res = grid->waterNeedsVMap() ? getLiquidStatusWithVMap(x, y, ground_z, &liquid_status) : 
                                                     grid->getLiquidStatus(x, y, ground_z, &liquid_status);
        if (!res)
            return VMAP_INVALID_HEIGHT_VALUE;

        return liquid_status.level;
    }

    return VMAP_INVALID_HEIGHT_VALUE;
}

bool TerrainInfo::IsLineOfSightEnabled() const
{
    const TerrainSpecifics* specifics = GetSpecifics();
    if (specifics == nullptr)
        return true;

    if (specifics->lineofsight == F_ALWAYS_DISABLED)
        return false;

    if (specifics->lineofsight == F_ALWAYS_ENABLED)
        return sWorld.getConfig(CONFIG_VMAP_LOS_ENABLED);

    if (sWorld.getConfig(CONFIG_COREBALANCER_ENABLED))
    {
        if (specifics->lineofsight <= sWorld.GetCoreBalancerTreshold())
            return false;
    }

    return specifics->lineofsight != F_ALWAYS_DISABLED && sWorld.getConfig(CONFIG_VMAP_LOS_ENABLED);
}

bool TerrainInfo::IsPathFindingEnabled() const
{
    const TerrainSpecifics* specifics = GetSpecifics();
    if (specifics == nullptr)
        return false;

    if (specifics->pathfinding == F_ALWAYS_DISABLED)
        return false;

    if (specifics->pathfinding == F_ALWAYS_ENABLED)
        return sWorld.getConfig(CONFIG_MMAP_ENABLED);

    if (sWorld.getConfig(CONFIG_COREBALANCER_ENABLED))
    {
        if (specifics->pathfinding <= sWorld.GetCoreBalancerTreshold())
            return false;
    }

    return GetSpecifics()->pathfinding != F_ALWAYS_DISABLED && sWorld.getConfig(CONFIG_MMAP_ENABLED);
}

float TerrainInfo::GetVisibilityDistance()
{
    const TerrainSpecifics* specifics = GetSpecifics();
    if (specifics == nullptr)
        return DEFAULT_VISIBILITY_DISTANCE;

    float visibility = specifics->visibility;
    if (sWorld.GetCoreBalancerTreshold() >= CB_VISIBILITY_PENALTY)
        visibility -= sWorld.getConfig(CONFIG_COREBALANCER_VISIBILITY_PENALTY);

    return visibility;
}

//////////////////////////////////////////////////////////////////////////

TerrainManager::TerrainManager()
{
}

TerrainManager::~TerrainManager()
{
    for (TerrainDataMap::iterator it = i_TerrainMap.begin(); it != i_TerrainMap.end(); ++it)
        delete it->second;
}

void TerrainManager::LoadTerrainSpecifics()
{
    i_TerrainSpecifics.clear();

    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT `entry`, `visibility`, `pathfinding`, `lineofsight`, `ainotifyperiod`, `viewupdatedistance` FROM `map_template`");
    if (!result)
    {
        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 map template data. DB table `map_template` is empty.");
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        //bar.step();

        uint32 mapid = fields[0].GetUInt32();

        TerrainSpecifics& info = i_TerrainSpecifics[mapid];

        info.visibility = fields[1].GetFloat();
        info.pathfinding = FeaturePriority(fields[2].GetUInt8());
        info.lineofsight = FeaturePriority(fields[3].GetUInt8());

        info.ainotifyperiod = fields[4].GetUInt32();
        // in database we have this in yards, but in core that is as square
        info.viewupdatedistance = fields[5].GetUInt32()*fields[5].GetUInt32();

    }
    while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %llu map template data.", i_TerrainSpecifics.size());
}

TerrainInfo * TerrainManager::LoadTerrain(const uint32 mapId)
{
    ACE_GUARD_RETURN(ACE_Thread_Mutex, Guard, TerrainMgrLock, NULL);

    TerrainInfo * ptr = NULL;
    TerrainDataMap::const_iterator iter = i_TerrainMap.find(mapId);
    if(iter == i_TerrainMap.end())
    {
        ptr = new TerrainInfo(mapId, i_TerrainSpecifics[mapId]);
        i_TerrainMap[mapId] = ptr;
    }
    else
        ptr = (*iter).second;

    return ptr;
}

void TerrainManager::UnloadTerrain(const uint32 mapId)
{
    if (sWorld.getConfig(CONFIG_GRID_UNLOAD) == 0)
        return;

    ACE_GUARD(ACE_Thread_Mutex, Guard, TerrainMgrLock);

    TerrainDataMap::iterator iter = i_TerrainMap.find(mapId);
    if(iter != i_TerrainMap.end())
    {
        TerrainInfo * ptr = (*iter).second;
        //lets check if this object can be actually freed
        if(ptr->IsReferenced() == false)
        {
            i_TerrainMap.erase(iter);
            delete ptr;
        }
    }
}

void TerrainManager::Update(const uint32 diff)
{
    //global garbage collection for GridMap objects and VMaps
    for (TerrainDataMap::iterator iter = i_TerrainMap.begin(); iter != i_TerrainMap.end(); ++iter)
        iter->second->CleanUpGrids(diff);
}

void TerrainManager::UnloadAll()
{
    for (TerrainDataMap::iterator it = i_TerrainMap.begin(); it != i_TerrainMap.end(); ++it)
        delete it->second;

    i_TerrainMap.clear();
}

uint32 TerrainManager::GetAreaIdByAreaFlag(uint16 areaflag,uint32 map_id)
{
    AreaTableEntry const *entry = GetAreaEntryByAreaFlagAndMap(areaflag,map_id);

    if (entry)
        return entry->ID;
    else
        return 0;
}

uint32 TerrainManager::GetZoneIdByAreaFlag(uint16 areaflag,uint32 map_id)
{
    AreaTableEntry const *entry = GetAreaEntryByAreaFlagAndMap(areaflag,map_id);

    if( entry )
        return ( entry->zone != 0 ) ? entry->zone : entry->ID;
    else
        return 0;
}

void TerrainManager::GetZoneAndAreaIdByAreaFlag(uint32& zoneid, uint32& areaid, uint16 areaflag,uint32 map_id)
{
    AreaTableEntry const *entry = GetAreaEntryByAreaFlagAndMap(areaflag,map_id);

    areaid = entry ? entry->ID : 0;
    zoneid = entry ? (( entry->zone != 0 ) ? entry->zone : entry->ID) : 0;
}
