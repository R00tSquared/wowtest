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

#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Item.h"
#include "Corpse.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "Map.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "ObjectGuid.h"
#include "World.h"

#include <cmath>

Pet * ObjectAccessor::GetPet(uint64 guid)
{
    return HashMapHolder<Pet>::Find(guid);
}

Player* ObjectAccessor::GetPlayerInWorld(uint64 guid)
{
    Player * plr = GetPlayerInWorldOrNot(guid);
    if (!plr || !plr->IsInWorld())
        return NULL;

    return plr;
}

Player* ObjectAccessor::GetPlayerByNameInWorld(const char *name)
{
    std::string tmp = name;
    PlayerName2PlayerMapType::const_accessor a;
    if (i_playerName2Player.find(a, tmp))
        if (a->second->IsInWorld())
            return a->second;

    return NULL;
}

Player* ObjectAccessor::GetPlayerByNameInWorld(std::string &name)
{
    PlayerName2PlayerMapType::const_accessor a;
    if (i_playerName2Player.find(a, name))
        if (a->second->IsInWorld())
            return a->second;

    return NULL;
}

Player * ObjectAccessor::GetPlayerInWorldOrNot(uint64 guid)
{
    return HashMapHolder<Player>::Find(guid);
}

Player * ObjectAccessor::GetPlayerOnline(uint64 guid)
{
    if (Player* p = ObjectAccessor::GetPlayerInWorld(guid))
        return p;
    else
        if (Player* p = ObjectAccessor::GetPlayerInWorldOrNot(guid))
        {
            if (p->IsBeingTeleported())
                return p;
        }

    return NULL;
}

HashMapHolder<Player>::MapType& ObjectAccessor::GetPlayers()
{
    return HashMapHolder<Player>::GetContainer();
}

bool ObjectAccessor::RemovePlayerName(Player *pl)
{
    return i_playerName2Player.erase(pl->GetName());
}

bool ObjectAccessor::RemovePlayer(Player *pl)
{
    return (HashMapHolder<Player>::Remove(pl) && RemovePlayerName(pl));
}

bool ObjectAccessor::RemovePlayer(uint64 guid)
{
    return (RemovePlayerName(GetPlayerInWorldOrNot(guid)) && HashMapHolder<Player>::Remove(guid));
}

bool ObjectAccessor::AddPlayerName(Player *pl)
{
    std::string tmp = pl->GetName();
    PlayerName2PlayerMapType::accessor a;
    if (i_playerName2Player.insert(a, tmp))
    {
        a->second = pl;
        return true;
    }

    return false;
}

bool ObjectAccessor::AddPlayer(Player *pl)
{
    return (HashMapHolder<Player>::Insert(pl) && AddPlayerName(pl));
}


void ObjectAccessor::AddPet(Pet* pet)
{
    HashMapHolder<Pet>::Insert(pet);
}

void ObjectAccessor::RemovePet(Pet* pet)
{
    HashMapHolder<Pet>::Remove(pet);
}

void ObjectAccessor::RemovePet(uint64 pet)
{
    HashMapHolder<Pet>::Remove(pet);
}

Corpse * ObjectAccessor::GetCorpse(uint64 guid)
{
    return HashMapHolder<Corpse>::Find(guid);
}

Corpse * ObjectAccessor::GetCorpse(WorldObject const &u, uint64 guid)
{
    Corpse * tmp = HashMapHolder<Corpse>::Find(guid);

    if (!tmp)
        return NULL;

    if (tmp->GetMapId() != u.GetMapId())
        return NULL;

    if (tmp->GetAnyInstanceId() != u.GetAnyInstanceId())
        return NULL;

    return tmp;
}

Corpse* ObjectAccessor::GetCorpseForPlayerGUID(uint64 guid)
{
    Player2CorpsesMapType::const_accessor a;
    if (i_player2corpse.find(a, guid))
    {
        ASSERT(a->second->GetType() != CORPSE_BONES);
        return a->second;
    }

    return NULL;
}

void ObjectAccessor::RemoveCorpse(Corpse *corpse)
{
    ASSERT(corpse && corpse->GetType() != CORPSE_BONES);

    Player2CorpsesMapType::const_accessor a;
    if (!i_player2corpse.find(a, corpse->GetOwnerGUID()))
        return;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = Hellground::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    sObjectMgr.DeleteCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID());
    corpse->RemoveFromWorld();

    ACE_GUARD(LockType, g, i_corpseGuard);
    i_player2corpse.erase(a);
}

void ObjectAccessor::AddCorpse(Corpse *corpse)
{
    ASSERT(corpse && corpse->GetType() != CORPSE_BONES);

    Player2CorpsesMapType::accessor a;
    ASSERT(!i_player2corpse.find(a, corpse->GetOwnerGUID()));
    a.release();

    ACE_GUARD(LockType, g, i_corpseGuard);
    i_player2corpse.insert(a, corpse->GetOwnerGUID());
    a->second = corpse;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = Hellground::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    sObjectMgr.AddCorpseCellData(corpse->GetMapId(),cell_id,corpse->GetOwnerGUID(),corpse->GetAnyInstanceId());
}


void ObjectAccessor::AddCorpsesToGrid(GridPair const& gridpair,GridType& grid,Map* map)
{
    ACE_GUARD(LockType, g, i_corpseGuard);
    for (Player2CorpsesMapType::iterator iter = i_player2corpse.begin(); iter != i_player2corpse.end(); ++iter)
        if (iter->second->GetGrid()==gridpair)
    {
        // verify, if the corpse in our instance (add only corpses which are)
        if (map->GetAnyInstanceId() == iter->second->GetAnyInstanceId())
            grid.AddWorldObject(iter->second);
    }
}

Corpse* ObjectAccessor::ConvertCorpseForPlayer(uint64 player_guid, bool insignia)
{
    Corpse *corpse = GetCorpseForPlayerGUID(player_guid);
    if (!corpse)
    {
        //in fact this function is called from several places
        //even when player doesn't have a corpse, not an error
        //sLog.outLog(LOG_DEFAULT, "ERROR: Try remove corpse that not in map for GUID %ul", player_guid);
        return NULL;
    }

    debug_log("Deleting Corpse and spawning bones.\n");

    // remove corpse from player_guid -> corpse map
    RemoveCorpse(corpse);

    // remove resurrectble corpse from grid object registry (loaded state checked into call)
    // do not load the map if it's not loaded
    Map *map = sMapMgr.FindMap(corpse->GetMapId(), corpse->GetAnyInstanceId());
    if (map)
        map->Remove(corpse, false);

    // remove corpse from DB
    corpse->DeleteFromDB();

    Corpse * bones = NULL;
    // create the bones only if the map and the grid is loaded at the corpse's location
    // ignore bones creating option in case insignia
    if (map && (insignia ||
       (map->IsBattleGroundOrArena() ? sWorld.getConfig(CONFIG_DEATH_BONES_BG_OR_ARENA) : sWorld.getConfig(CONFIG_DEATH_BONES_WORLD))) &&
        !map->IsRemovalGrid(corpse->GetPositionX(), corpse->GetPositionY()))
    {
        // Create bones, don't change Corpse
        bones = new Corpse;
        bones->Create(corpse->GetGUIDLow());

        for (int i = 3; i < CORPSE_END; i++)                    // don't overwrite guid and object type
            bones->SetUInt32Value(i, corpse->GetUInt32Value(i));

        bones->SetGrid(corpse->GetGrid());
        // bones->m_time = m_time;                              // don't overwrite time
        // bones->m_inWorld = m_inWorld;                        // don't overwrite world state
        // bones->m_type = m_type;                              // don't overwrite type
        bones->Relocate(corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetOrientation());
        bones->SetMapId(corpse->GetMapId());
        bones->SetInstanceId(corpse->GetAnyInstanceId());

        bones->SetUInt32Value(CORPSE_FIELD_FLAGS, CORPSE_FLAG_UNK2 | CORPSE_FLAG_BONES);
        bones->SetUInt64Value(CORPSE_FIELD_OWNER, 0);

        for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            if (corpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
                bones->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
        }

        // add bones in grid store if grid loaded where corpse placed
        map->Add(bones);
    }

    // all references to the corpse should be removed at this point
    delete corpse;

    return bones;
}

Corpse * ObjectAccessor::GetCorpse(uint32 mapid, float x, float y, uint64 guid)
{
    Corpse * corpse = HashMapHolder<Corpse>::Find(guid);

    if (corpse && corpse->GetMapId() == mapid)
    {
        CellPair p = Hellground::ComputeCellPair(x,y);
        if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: ObjectAccessor::GetCorpse: invalid coordinates supplied X:%f Y:%f grid cell [%u:%u]", x, y, p.x_coord, p.y_coord);
            return NULL;
        }

        CellPair q = Hellground::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
        if (q.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || q.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: ObjectAccessor::GetCorpse: object %llu has invalid coordinates X:%f Y:%f grid cell [%u:%u]", corpse->GetGUID(), corpse->GetPositionX(), corpse->GetPositionY(), q.x_coord, q.y_coord);
            return NULL;
        }

        int32 dx = int32(p.x_coord) - int32(q.x_coord);
        int32 dy = int32(p.y_coord) - int32(q.y_coord);

        if (dx > -2 && dx < 2 && dy > -2 && dy < 2)
            return corpse;
    }

    return NULL;
}

void ObjectAccessor::RemoveOldCorpses()
{
    time_t now = time(NULL);
    Player2CorpsesMapType::iterator next;
    for (Player2CorpsesMapType::iterator itr = i_player2corpse.begin(); itr != i_player2corpse.end(); itr = next)
    {
        next = itr;
        ++next;

        if (!itr->second->IsExpired(now))
            continue;

        ConvertCorpseForPlayer(itr->first);
    }
}

void ObjectAccessor::SaveAllPlayersWithSleep()
{
    ACE_GUARD(ACE_Thread_Mutex, guard, *HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& plist = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = plist.begin(); itr != plist.end(); ++itr)
        itr->second->SaveToDB();

    ACE_Based::Thread::Sleep(1000 + plist.size() * 20); // give 1 second + 20 ms per player to be safe
}

/// Define the static member of HashMapHolder

template <class T> tbb::concurrent_hash_map< uint64, T* > HashMapHolder<T>::m_objectMap;
template <class T> ACE_Thread_Mutex HashMapHolder<T>::i_lock;

/// Global definitions for the hashmap storage

template class HashMapHolder<Player>;
template class HashMapHolder<Pet>;
template class HashMapHolder<Corpse>;
