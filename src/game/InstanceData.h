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

#ifndef HELLGROUND_INSTANCE_DATA_H
#define HELLGROUND_INSTANCE_DATA_H

#include <vector>

#include "ZoneScript.h"
//#include "GameObject.h"
#include "Map.h"
#include "Player.h"

class Map;
class Unit;
class Player;
class GameObject;
class Creature;

enum EncounterState
{
    NOT_STARTED   = 0,
    IN_PROGRESS   = 1,
    FAIL          = 2,
    DONE          = 3,
    SPECIAL       = 4,
    TO_BE_DECIDED = 5,
};

typedef std::set<GameObject*> DoorSet;

enum DoorType
{
    DOOR_TYPE_ROOM = 0,
    DOOR_TYPE_PASSAGE,
    MAX_DOOR_TYPES,
};

struct BossInfo
{
    BossInfo() : state(TO_BE_DECIDED) {}
    EncounterState state;
    DoorSet door[MAX_DOOR_TYPES];
};

struct DoorInfo
{
    explicit DoorInfo(BossInfo *_bossInfo, DoorType _type)
        : bossInfo(_bossInfo), type(_type) {}
    BossInfo *bossInfo;
    DoorType type;
};

typedef std::multimap<uint32 /*entry*/, DoorInfo> DoorInfoMap;

struct DoorData
{
    uint32 entry, bossId;
    DoorType type;
};

struct RaidPrintStats
{
    std::string name;
    bool is_healer;
};

class HELLGROUND_IMPORT_EXPORT InstanceData : public ZoneScript
{
    public:

        explicit InstanceData(Map *map) : instance(map) {}
        virtual ~InstanceData() {}

        Map *instance;

        //On creation, NOT load.
        virtual void Initialize() {}

        //On load
        virtual void Load(const char * data) { LoadBossState(data); }

        //When save is needed, this function generates the data
        virtual std::string GetSaveData() { return GetBossSaveData(); }

        void SaveToDB();

        virtual void Update(uint32 /*diff*/) {}

        //Used by the map's CanEnter function.
        //This is to prevent players from entering during boss encounters.
        virtual bool IsEncounterInProgress() const;
        virtual void ResetEncounterInProgress();

        //Called when a player successfully enters the instance.
        virtual void OnPlayerEnter(Player *) {}

        //Called when a player deaths in the instance.
        virtual void OnPlayerDeath(Player *) {}

        //Called when player deals damage/heals someone. Mainly for pve logs.
        virtual void OnPlayerDealDamage(Player*, uint32) {};
        virtual void OnPlayerHealDamage(Player*, uint32) {};

        //Called on creature death (after CreatureAI::JustDied)
        virtual void OnCreatureDeath(Creature *) {}

        //Called when a gameobject is created
        void OnGameObjectCreate(GameObject *go, bool add) { if (add) OnObjectCreate(go); }

        //called on creature creation
        void OnCreatureCreate(Creature *, bool add);
        
        // called on creature evade
        virtual void OnCreatureEvade(Creature* /*creature*/) {}

        //Handle open / close objects
        //use HandleGameObject(NULL,boolen,GO); in OnObjectCreate in instance scripts
        //use HandleGameObject(GUID,boolen,NULL); in any other script
        void HandleGameObject(uint64 GUID, bool open, GameObject *go = NULL);

        virtual bool SetBossState(uint32 id, EncounterState state);

        Creature *GetCreature(uint64 guid){ return instance->GetCreature(guid); }
        Creature *GetCreatureById(uint32 entry){ return instance->GetCreatureById(entry); }

        virtual uint32 GetEncounterForEntry(uint32 entry) { return 0; }
        virtual uint32 GetRequiredEncounterForEntry(uint32 entry) { return 0; }

        /* used with creature_linked_respawn table to make creatures non-attackable until certain event is done. 
        may also be used to make script-special changes or sets them dead, so don't use it to make bosses non-attackable!*/
        virtual void HandleInitCreatureState(Creature * mob);
        /* used with creature_linked_respawn table to make creatures non-attackable until certain event is done.
        may also be used to make script-special changes or sets them dead, so don't use it to make bosses non-attackable!*/
        virtual void HandleRequiredEncounter(uint32 encounter);

        void LogPossibleCheaters(const char* cheatName);

        virtual uint32 GetCustomData() { return 0; };
    protected:
        void SetBossNumber(uint32 number) { bosses.resize(number); }
        void LoadDoorData(const DoorData *data);

        void AddDoor(GameObject *door, bool add);
        void UpdateDoorState(GameObject *door);

        std::string LoadBossState(const char * data);
        std::string GetBossSaveData();

        UNORDERED_MAP<uint32, std::vector<uint64> > requiredEncounterToMobs;

    private:
        std::vector<BossInfo> bosses;
        DoorInfoMap doors;
		uint32 start_time;

        virtual void OnObjectCreate(GameObject *) {}
        virtual void OnCreatureCreate(Creature *, uint32 entry) {}
};
/*6 for 25 raid
10 for 40 raid
2 for 10 raid
3 for 15 raid*/
#define GBK_ALLOWED_NON_MEMBERS (float)0.25

class HELLGROUND_IMPORT_EXPORT GBK_handler
{
    struct GBKStats
    {
        GBKStats() : healing(0), damage(0), deaths(0) {};
        uint32 healing;
        uint32 damage;
        uint8 deaths;
    };
public:
    GBK_handler(Map* map) : m_timer(0), m_encounter(GBK_NONE), m_last_encounter(GBK_NONE), m_map(map), m_evade_count(0) {};

    void HealingDone(uint32 guid, uint32 amount)
    {
        if (m_encounter != GBK_NONE)
            stats[guid].healing += amount;
    };
    void DamageDone(uint32 guid, uint32 amount)
    {
        if (m_encounter != GBK_NONE)
            stats[guid].damage += amount;
    };
    void PlayerDied(uint32 guid)
    {
        if (m_encounter != GBK_NONE)
            stats[guid].deaths++;
    };
    
    void StopCombat(GBK_Encounters encounter, bool win);
    void StartCombat(GBK_Encounters encounter);
	//bool FirstBoss(GBK_Encounters encounter);
	bool LastBoss(GBK_Encounters encounter);

    void RessurectAfterEvade();
    void DebugLog(std::string msg, GBK_Encounters encounter);

private:
    GBK_Encounters m_encounter;
    GBK_Encounters m_last_encounter;
    uint32 m_timer;
    std::map<uint32, GBKStats> stats;
    Map* m_map;
    uint32 m_evade_count;
};

#endif
