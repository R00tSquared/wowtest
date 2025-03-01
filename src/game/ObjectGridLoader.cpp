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

#include "ObjectGridLoader.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "Corpse.h"
#include "World.h"
#include "CellImpl.h"
#include "CreatureAI.h"
#include "GridDefines.h"

class ObjectGridRespawnMover
{
    public:
        ObjectGridRespawnMover() {}

        void Move(GridType &grid);

        template<class T> void Visit(GridRefManager<T> &) {}
        void Visit(CreatureMapType &m);
};

void
ObjectGridRespawnMover::Move(GridType &grid)
{
    TypeContainerVisitor<ObjectGridRespawnMover, GridTypeMapContainer > mover(*this);
    grid.Visit(mover);
}

void
ObjectGridRespawnMover::Visit(CreatureMapType &m)
{
    // creature in unloading grid can have respawn point in another grid
    // if it will be unloaded then it will not respawn in original grid until unload/load original grid
    // move to respawn point to prevent this case. For player view in respawn grid this will be normal respawn.
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end();)
    {
        Creature * c = iter->getSource();
        ++iter;

        ASSERT(!c->isPet() && "ObjectGridRespawnMover don't must be called for pets");

        Cell const& cur_cell  = c->GetCurrentCell();

        float resp_x, resp_y, resp_z;
        c->GetRespawnCoord(resp_x, resp_y, resp_z);
        CellPair resp_val = Hellground::ComputeCellPair(resp_x, resp_y);
        Cell resp_cell(resp_val);

        if (cur_cell.DiffGrid(resp_cell))
        {
            c->GetMap()->CreatureRespawnRelocation(c);
            // false result ignored: will be unload with other creatures at grid
        }
    }
}

// for loading world object at grid loading (Corpses)
class ObjectWorldLoader
{
    public:
        explicit ObjectWorldLoader(ObjectGridLoader& gloader)
            : i_cell(gloader.i_cell), i_grid(gloader.i_grid), i_map(gloader.i_map), i_corpses (0)
            {}

        void Visit(CorpseMapType &m);

        template<class T> void Visit(GridRefManager<T>&) { }

    private:
        Cell i_cell;
        NGridType &i_grid;
        Map* i_map;
    public:
        uint32 i_corpses;
};

template<class T> void addUnitState(T* /*obj*/, CellPair const& /*cell_pair*/)
{
}

template<> void addUnitState(Creature *obj, CellPair const& cell_pair)
{
    Cell cell(cell_pair);

    obj->SetCurrentCell(cell);
    if (obj->isSpiritService())
        obj->setDeathState(DEAD);
}

template <class T>
void LoadHelper(CellGuidSet const& guid_set, CellPair &cell, GridRefManager<T> &m, uint32 &count, Map* map, GridType& grid)
{
    for (CellGuidSet::const_iterator i_guid = guid_set.begin(); i_guid != guid_set.end(); ++i_guid)
    {
        T* obj = new T;
        uint32 guid = *i_guid;
        //sLog.outString("DEBUG: LoadHelper from table: %s for (guid: %u) Loading",table,guid);
        if (!obj->LoadFromDB(guid, map))
        {
            delete obj;
            continue;
        }

        grid.AddGridObject(obj);

        addUnitState(obj,cell);
        obj->AddToWorld();

        if (obj->isActiveObject())
            map->AddToActive(obj);

        obj->GetViewPoint().Event_AddedToWorld(&grid);

        ++count;

    }
}

void LoadHelper(CellCorpseSet const& cell_corpses, CellPair &cell, CorpseMapType &m, uint32 &count, Map* map, GridType& grid)
{
    if (cell_corpses.empty())
        return;

    for (CellCorpseSet::const_iterator itr = cell_corpses.begin(); itr != cell_corpses.end(); ++itr)
    {
        if (itr->second != map->GetAnyInstanceId())
            continue;

        uint32 player_guid = itr->first;

        Corpse *obj = sObjectAccessor.GetCorpseForPlayerGUID(player_guid);
        if (!obj)
            continue;

        grid.AddWorldObject(obj);

        addUnitState(obj,cell);
        obj->AddToWorld();

        if (obj->isActiveObject())
            map->AddToActive(obj);

        ++count;
    }
}

void ObjectGridLoader::Visit(GameObjectMapType &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids const& cell_guids = sObjectMgr.GetCellObjectGuids(i_map->GetId(), i_map->GetSpawnMode(), cell_id);

    GridType& grid = (*i_map->getNGrid(i_cell.GridX(),i_cell.GridY())) (i_cell.CellX(),i_cell.CellY());
    LoadHelper(cell_guids.gameobjects, cell_pair, m, i_gameObjects, i_map, grid);
}

void ObjectGridLoader::Visit(CreatureMapType &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    CellObjectGuids const& cell_guids = sObjectMgr.GetCellObjectGuids(i_map->GetId(), i_map->GetSpawnMode(), cell_id);
    GridType& grid = (*i_map->getNGrid(i_cell.GridX(),i_cell.GridY())) (i_cell.CellX(),i_cell.CellY());
    LoadHelper(cell_guids.creatures, cell_pair, m, i_creatures, i_map, grid);
}

void ObjectWorldLoader::Visit(CorpseMapType &m)
{
    uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
    uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
    CellPair cell_pair(x,y);
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    // corpses are always added to spawn mode 0 and they are spawned by their instance id
    CellObjectGuids const& cell_guids = sObjectMgr.GetCellObjectGuids(i_map->GetId(), 0, cell_id);
    GridType& grid = (*i_map->getNGrid(i_cell.GridX(),i_cell.GridY())) (i_cell.CellX(),i_cell.CellY());
    LoadHelper(cell_guids.corpses, cell_pair, m, i_corpses, i_map, grid);
}

void ObjectGridLoader::Load(GridType &grid)
{
    {
        TypeContainerVisitor<ObjectGridLoader, GridTypeMapContainer > loader(*this);
        grid.Visit(loader);
    }

    {
        ObjectWorldLoader wloader(*this);
        TypeContainerVisitor<ObjectWorldLoader, WorldTypeMapContainer > loader(wloader);
        grid.Visit(loader);
        i_corpses = wloader.i_corpses;
    }
}

void ObjectGridLoader::LoadN(void)
{
    i_gameObjects = 0; i_creatures = 0; i_corpses = 0;
    i_cell.data.Part.cell_y = 0;
    for (unsigned int x=0; x < MAX_NUMBER_OF_CELLS; ++x)
    {
        i_cell.data.Part.cell_x = x;
        for (unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
        {
            i_cell.data.Part.cell_y = y;
            GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> loader;
            loader.Load(i_grid(x, y), *this);
        }
    }
    sLog.outDebug("%u GameObjects, %u Creatures, and %u Corpses/Bones loaded for grid %u on map %u", i_gameObjects, i_creatures, i_corpses,i_grid.GetGridId(), i_map->GetId());
}

void ObjectGridUnloader::MoveToRespawnN()
{
    for (unsigned int x=0; x < MAX_NUMBER_OF_CELLS; ++x)
    {
        for (unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
        {
            ObjectGridRespawnMover mover;
            mover.Move(i_grid(x, y));
        }
    }
}

void ObjectGridUnloader::Unload(GridType &grid)
{
    TypeContainerVisitor<ObjectGridUnloader, GridTypeMapContainer > unloader(*this);
    grid.Visit(unloader);
}

template<class T>
void ObjectGridUnloader::Visit(GridRefManager<T> &m)
{
    // remove all cross-reference before deleting
    for (typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
        iter->getSource()->CleanupsBeforeDelete();

    while (!m.isEmpty())
    {
        T *obj = m.getFirst()->getSource();
        // if option set then object already saved at this moment
        if (!sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
            obj->SaveRespawnTime();
        ///- object will get delinked from the manager when deleted
        delete obj;
    }
}

void ObjectGridStoper::Stop(GridType &grid)
{
    TypeContainerVisitor<ObjectGridStoper, GridTypeMapContainer > stoper(*this);
    grid.Visit(stoper);
}

void ObjectGridStoper::Visit(CreatureMapType &m)
{
    // stop any fights at grid de-activation and remove dynobjects created at cast by creatures
    for (CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        iter->getSource()->RemoveAllDynObjects();
        if (iter->getSource()->IsInCombat())
        {
            iter->getSource()->CombatStop();
            iter->getSource()->DeleteThreatList();
            iter->getSource()->AI()->EnterEvadeMode();
        }
    }
}

void ObjectGridCleaner::Stop(GridType &grid)
{
    TypeContainerVisitor<ObjectGridCleaner, GridTypeMapContainer > stoper(*this);
    grid.Visit(stoper);
}

void ObjectGridCleaner::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        iter->getSource()->CleanupsBeforeDelete();
}

template<class T>
void ObjectGridCleaner::Visit(GridRefManager<T> &m)
{
    for (typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
        iter->getSource()->RemoveFromWorld();
}

template void ObjectGridUnloader::Visit(CreatureMapType &);
template void ObjectGridUnloader::Visit(GameObjectMapType &);
template void ObjectGridUnloader::Visit(DynamicObjectMapType &);
template void ObjectGridUnloader::Visit(CorpseMapType &);
template void ObjectGridCleaner::Visit<GameObject>(GameObjectMapType &);
template void ObjectGridCleaner::Visit<DynamicObject>(DynamicObjectMapType &);
template void ObjectGridCleaner::Visit<Corpse>(CorpseMapType &);
