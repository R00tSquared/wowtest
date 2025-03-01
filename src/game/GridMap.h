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

#ifndef HELLGROUND_GRIDMAP_H
#define HELLGROUND_GRIDMAP_H

#include "ace/Singleton.h"

#include "Platform/Define.h"
#include "DBCStructure.h"
#include "GridDefines.h"
#include "Object.h"
#include "SharedDefines.h"

#include <bitset>
#include <list>

class Creature;
class Unit;
class WorldPacket;
class InstanceData;
class Group;
struct ScriptInfo;
struct ScriptAction;
class BattleGround;
class Map;

struct GridMapFileHeader
{
    uint32 mapMagic;
    uint32 versionMagic;
    uint32 buildMagic;
    uint32 areaMapOffset;
    uint32 areaMapSize;
    uint32 heightMapOffset;
    uint32 heightMapSize;
    uint32 liquidMapOffset;
    uint32 liquidMapSize;
    uint32 holesOffset;
    uint32 holesSize;
};

#define MAP_AREA_NO_AREA      0x0001

struct GridMapAreaHeader
{
    uint32 fourcc;
    uint16 flags;
    uint16 gridArea;
};

template<typename Countable>
class HELLGROUND_IMPORT_EXPORT Referencable
{
    public:
        Referencable() { m_count = 0; }

        void AddRef() { ++m_count; }
        bool Release() { return (--m_count < 1); }
        bool IsReferenced() const { return (m_count > 0); }

    private:
        Referencable(const Referencable&);
        Referencable& operator=(const Referencable&);

        Countable m_count;
};

typedef ACE_Atomic_Op<ACE_Thread_Mutex, long> AtomicLong;

#define MAP_HEIGHT_NO_HEIGHT  0x0001
#define MAP_HEIGHT_AS_INT16   0x0002
#define MAP_HEIGHT_AS_INT8    0x0004

struct GridMapHeightHeader
{
    uint32 fourcc;
    uint32 flags;
    float gridHeight;
    float gridMaxHeight;
};

#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

struct GridMapLiquidHeader
{
    uint32 fourcc;
    uint16 flags;
    uint16 liquidType;
    uint8 offsetX;
    uint8 offsetY;
    uint8 width;
    uint8 height;
    float liquidLevel;
};

/*enum GridMapLiquidStatus Moved to GridDefines.h */

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

#define MAP_ALL_LIQUIDS   (MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN | MAP_LIQUID_TYPE_MAGMA | MAP_LIQUID_TYPE_SLIME)

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20

/*struct GridMapLiquidData Moved to GridDefines.h */
class GridMap
{
    private:
        //uint32 m_flags;
        /*There is VMap water in this grid. 0 - no, 1 - yes, 2 yes by default (not checked)*/
        uint8 _waterNeedsVMap;
        /*lava/slime/dark_water in this grid. 0 - no, 1 - yes, 2 yes by default (not checked)*/
        uint8 _specialLiquid;

        // Area data
        uint16 m_gridArea;
        uint16 *m_area_map;

        // Height level data
        float m_gridHeight;
        float m_gridIntHeightMultiplier;
        union
        {
            float *m_V9;
            uint16 *m_uint16_V9;
            uint8 *m_uint8_V9;
        };
        union
        {
            float *m_V8;
            uint16 *m_uint16_V8;
            uint8 *m_uint8_V8;
        };

        // Liquid data
        uint16 m_liquidType;
        uint8 m_liquid_offX;
        uint8 m_liquid_offY;
        uint8 m_liquid_width;
        uint8 m_liquid_height;
        float m_liquidLevel;
        uint8 *m_liquid_type;
        float *m_liquid_map;

        bool loadAreaData(FILE *in, uint32 offset, uint32 size);
        bool loadHeightData(FILE *in, uint32 offset, uint32 size);
        bool loadGridMapLiquidData(FILE *in, uint32 offset, uint32 size);

        // Get height functions and pointers
        typedef float (GridMap::*pGetHeightPtr) (float x, float y) const;
        pGetHeightPtr m_gridGetHeight;
        float getHeightFromFloat(float x, float y) const;
        float getHeightFromUint16(float x, float y) const;
        float getHeightFromUint8(float x, float y) const;
        float getHeightFromFlat(float x, float y) const;

    public:
        GridMap();
        ~GridMap();

        bool loadData(char* filename, const uint32 mapId, const uint32 x, const uint32 y);
        void unloadData();

        static bool ExistMap(uint32 mapid, int gx, int gy);
        static bool ExistVMap(uint32 mapid, int gx, int gy);

        uint16 getArea(float x, float y);
        float getHeight(float x, float y) { return (this->*m_gridGetHeight)(x, y); }
        //float getLiquidLevel(float x, float y);
        //uint8 getTerrainType(float x, float y);
        GridMapLiquidStatus getLiquidStatus(float x, float y, float z, GridMapLiquidData *data = 0);
        uint8 waterNeedsVMap() { return _waterNeedsVMap; };
        uint8 hasSpecialLiquid() { return _specialLiquid; };
};

#define MAX_HEIGHT            100000.0f                     // can be use for find ground height at surface
#define INVALID_HEIGHT       -100000.0f                     // for check, must be equal to VMAP_INVALID_HEIGHT, real value for unknown height is VMAP_INVALID_HEIGHT_VALUE
#define INVALID_HEIGHT_VALUE -200000.0f                     // for return, must be equal to VMAP_INVALID_HEIGHT_VALUE, check value for unknown height is VMAP_INVALID_HEIGHT
#define MAX_FALL_DISTANCE     250000.0f                     // "unlimited fall" to find VMap ground if it is available, just larger than MAX_HEIGHT - INVALID_HEIGHT
#define DEFAULT_HEIGHT_SEARCH     10.0f                     // default search distance to find height at nearby locations
#define DEFAULT_WATER_SEARCH      50.0f                     // default search distance to case detection water level

// 3.75 yds is considered a minimum height of a hole in the map (this is taken from NE buff house door height in the WSG). 89 degrees tangens (when dividing height by distance) is 57.29. We consider > 89 degrees ground as a holes
#define GRID_GROUND_HIT_POS_MAX_PRECISION (3.75f / 60.0f)

enum FeaturePriority
{
    F_ALWAYS_DISABLED = 0,
    F_LOW_PRIORITY    = 1,
    F_MID_PRIORITY    = 2,
    F_HIGH_PRIORITY   = 3,

    F_ALWAYS_ENABLED = 6//CB_TRESHOLD_MAX +1,
};

typedef struct MapTemplate
{
    MapTemplate()
    {
        visibility = DEFAULT_VISIBILITY_DISTANCE;
        pathfinding = F_ALWAYS_DISABLED;
        lineofsight = F_ALWAYS_ENABLED;

        ainotifyperiod = 1000;
        viewupdatedistance = 10;
    }

    float visibility;

    FeaturePriority pathfinding;
    FeaturePriority lineofsight;

    uint32 ainotifyperiod;
    uint32 viewupdatedistance;

} TerrainSpecifics;

//class for sharing and managing GridMap objects
class HELLGROUND_IMPORT_EXPORT TerrainInfo : public Referencable<AtomicLong>
{
    public:
        TerrainInfo(uint32 mapid, TerrainSpecifics terrainspecifics);
        ~TerrainInfo();

        uint32 GetMapId() const { return m_mapId; }

        //TODO: move all terrain/vmaps data info query functions
        //from 'Map' class into this class
        /*Ensures grid is loaded at given coords (if coords are ok)
        May return VMAP_INVALID_HEIGHT_VALUE if there's no height under the given point found*/
        float GetHeight(float x, float y, float z, bool pCheckVMap=true, float maxSearchDist=DEFAULT_HEIGHT_SEARCH) const;
        /*Tries to find grid ground between start pos and end pos and move end pos closer to start pos if it hits ground on the way.
		It is problematic to determine, because grid ground can jump, if target/we are on the vmap ground. This is caused by holes in grid ground (caves, stairs, etc)*/
        bool getGridGroundHitPos(float x, float y, float z, float& rx, float& ry, float& rz, float angleCos, float angleSin, float dist, float checkForHeight) const;
        float GetWaterLevel(float x, float y, float z, float* pGround = NULL) const;
        /*May return VMAP_INVALID_HEIGHT_VALUE (groundZ as well) if there's no height under the given point found*/
        float GetWaterOrGroundLevel(float x, float y, float z, float* pGround = NULL, bool swim = false) const;
        /*Returns true if would swim at these coordinates. False on water walking or above water*/
        bool IsInWater(float x, float y, float z) const;
        /*Returns true if would swim at these coordinates, but makes z coordinate lower by 1 to also include waterwalk and same-as-water-level z or over-the-water polies (mmap has them over water)*/
        bool IsInWaterOrSlightlyAbove(float x, float y, float z) const;

        /*If returned anything but LIQUID_MAP_NO_WATER, then it was useful and VMap is needed*/
        bool getLiquidStatusVMapOnlyForTest(float x, float y, float z, GridMapLiquidData *data) const;
        /*If grid has no special liquid -> then only checks if isSwimming==true*/
        GridMapLiquidStatus getLiquidStatusIfSwimmingOrSpecial(float x, float y, float z, GridMapLiquidData *data, bool isSwimming) const;

        uint16 GetAreaFlag(float x, float y, float z, bool *isOutdoors=0) const;
        //uint8 GetTerrainType(float x, float y ) const;
        uint8 waterNeedsVMap(float x, float y) const;
        uint8 hasSpecialLiquid(float x, float y) const;

        uint32 GetAreaId(float x, float y, float z) const;
        uint32 GetZoneId(float x, float y, float z) const;
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid, float x, float y, float z) const;

        bool GetAreaInfo(float x, float y, float z, uint32 &mogpflags, int32 &adtId, int32 &rootId, int32 &groupId) const;
        bool IsOutdoors(float x, float y, float z) const;

        TerrainSpecifics const* GetSpecifics() const { return m_specifics; }

        //this method should be used only by TerrainManager
        //to cleanup unreferenced GridMap objects - they are too heavy
        //to destroy them dynamically, especially on highly populated servers
        //THIS METHOD IS NOT THREAD-SAFE!!!! AND IT SHOULDN'T BE THREAD-SAFE!!!!
        void CleanUpGrids(const uint32 diff);

        float GetVisibilityDistance();

        bool IsLineOfSightEnabled() const;
        bool IsPathFindingEnabled() const;

    protected:
        friend class Map;
        //load/unload terrain data
        GridMap * Load(const uint32 x, const uint32 y);
        void Unload(const uint32 x, const uint32 y);

    private:
        TerrainInfo(const TerrainInfo&);
        TerrainInfo& operator=(const TerrainInfo&);

        /*Ensures adequate x and y. Ensures grid is loaded*/
        GridMap * GetGrid( const float x, const float y );
        GridMap * LoadMapAndVMap(const uint32 x, const uint32 y );

        int RefGrid(const uint32& x, const uint32& y);
        int UnrefGrid(const uint32& x, const uint32& y);

        GridMapLiquidStatus getLiquidStatusWithVMap(float x, float y, float z, GridMapLiquidData *data = 0) const;

        const uint32 m_mapId;

        TerrainSpecifics* m_specifics;

        GridMap *m_GridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        int16 m_GridRef[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];

        //global garbage collection timer
        Timer i_timer;

        typedef ACE_Thread_Mutex LOCK_TYPE;
        typedef ACE_Guard<LOCK_TYPE> LOCK_GUARD;
        LOCK_TYPE m_mutex;
        LOCK_TYPE m_refMutex;
};

//class for managing TerrainData object and all sort of geometry querying operations
class TerrainManager
{
    friend class ACE_Singleton<TerrainManager, ACE_Thread_Mutex>;
    TerrainManager();

    typedef UNORDERED_MAP<uint32, TerrainInfo *> TerrainDataMap;
    typedef UNORDERED_MAP<uint32, TerrainSpecifics > TerrainsSpecificsMap;

    public:
        void LoadTerrainSpecifics();

        TerrainInfo* LoadTerrain(const uint32 mapId);
        void UnloadTerrain(const uint32 mapId);

        void Update(const uint32 diff);
        void UnloadAll();

        uint16 GetAreaFlag(uint32 mapid, float x, float y, float z) const
        {
            TerrainInfo *pData = const_cast<TerrainManager*>(this)->LoadTerrain(mapid);
            return pData->GetAreaFlag(x, y, z);
        }
        uint32 GetAreaId(uint32 mapid, float x, float y, float z) const
        {
            return TerrainManager::GetAreaIdByAreaFlag(GetAreaFlag(mapid, x, y, z),mapid);
        }
        uint32 GetZoneId(uint32 mapid, float x, float y, float z) const
        {
            return TerrainManager::GetZoneIdByAreaFlag(GetAreaFlag(mapid, x, y, z),mapid);
        }
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid, uint32 mapid, float x, float y, float z)
        {
            TerrainManager::GetZoneAndAreaIdByAreaFlag(zoneid,areaid,GetAreaFlag(mapid, x, y, z),mapid);
        }

        static uint32 GetAreaIdByAreaFlag(uint16 areaflag,uint32 map_id);
        static uint32 GetZoneIdByAreaFlag(uint16 areaflag,uint32 map_id);
        static void GetZoneAndAreaIdByAreaFlag(uint32& zoneid, uint32& areaid, uint16 areaflag,uint32 map_id);

    private:
        ~TerrainManager();

        TerrainManager(const TerrainManager &);
        TerrainManager& operator=(const TerrainManager &);

        ACE_Thread_Mutex TerrainMgrLock;
        TerrainDataMap i_TerrainMap;
        TerrainsSpecificsMap i_TerrainSpecifics;
};

#define sTerrainMgr (*ACE_Singleton<TerrainManager, ACE_Thread_Mutex>::instance())
#endif
