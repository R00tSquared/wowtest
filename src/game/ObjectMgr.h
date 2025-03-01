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

#ifndef HELLGROUND_OBJECTMGR_H
#define HELLGROUND_OBJECTMGR_H

#include "ace/Singleton.h"

#include "Log.h"
#include "Object.h"
#include "Bag.h"
#include "Creature.h"
#include "Player.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Corpse.h"
#include "QuestDef.h"
#include "ItemPrototype.h"
#include "NPCHandler.h"
#include "Database/DatabaseEnv.h"
#include "Mail.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Database/SQLStorage.h"
#include <ace/Atomic_Op.h>
#include <ace/Thread_Mutex.h>

#include <string>
#include <map>
#include <limits>

extern SQLStorage sCreatureStorage;
extern SQLStorage sCreatureDataAddonStorage;
extern SQLStorage sCreatureInfoAddonStorage;
extern SQLStorage sCreatureModelStorage;
extern SQLStorage sEquipmentStorage;
extern SQLStorage sGOStorage;
extern SQLStorage sPageTextStore;
extern SQLStorage sItemStorage;
extern SQLStorage sInstanceTemplate;
extern SQLStorage sSpellTemplate;
extern SQLStorage sSpellTemplateTEST;

class Group;
class Guild;
class ArenaTeam;
class Item;

struct GameTele
{
    float  position_x;
    float  position_y;
    float  position_z;
    float  orientation;
    uint32 mapId;
    std::string name;
    std::wstring wnameLow;
};

typedef UNORDERED_MAP<uint32, GameTele > GameTeleMap;
typedef std::list<GossipOption> CacheNpcOptionList;

struct AreaTrigger
{
    uint32 access_id;
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
};

typedef std::set<uint32> CellGuidSet;
typedef std::map<uint32/*player guid*/,uint32/*instance*/> CellCorpseSet;
struct CellObjectGuids
{
    CellGuidSet creatures;
    CellGuidSet gameobjects;
    CellCorpseSet corpses;
};
typedef UNORDERED_MAP<uint32/*cell_id*/,CellObjectGuids> CellObjectGuidsMap;
typedef UNORDERED_MAP<uint32/*(mapid,spawnMode) pair*/,CellObjectGuidsMap> MapObjectGuids;

typedef UNORDERED_MAP<uint64/*(instance,guid) pair*/,time_t> RespawnTimes;

// trinity string ranges
// #define MIN_CUSTOM_TEXT_ID -2000000
// #define MAX_CUSTOM_TEXT_ID -2999999
// #define MIN_SCRIPT_TEXT_ID -1000000 // those 4 are defined in scripts. here just for comment
// #define MAX_SCRIPT_TEXT_ID -1999999
#define MIN_CREATURE_AI_TEXT_STRING_ID (-1)                 // 'creature_ai_texts'
#define MAX_CREATURE_AI_TEXT_STRING_ID (-999999)
#define MIN_HELLGROUND_STRING_ID       1                    // 'HELLGROUND_string'
#define MAX_HELLGROUND_STRING_ID       1999999999
#define MIN_DB_SCRIPT_STRING_ID        2000000000 // 'db_script_string'
#define MAX_DB_SCRIPT_STRING_ID        2000009999
#define MIN_DB_AUTOBROADCAST_STRING_ID        2000010000 // 'autobroadcast'
#define MAX_DB_AUTOBROADCAST_STRING_ID        2000019999
#define MIN_DB_ANNOUNCES_STRING_ID        2000020000 // 'announces'
#define MAX_DB_ANNOUNCES_STRING_ID        2000029999

struct HellgroundStringLocale
{
    std::vector<std::string> Content;                       // 0 -> default, i -> i-1 locale index
};

typedef std::map<uint32,uint32> CreatureLinkedRespawnMap;
typedef UNORDERED_MAP<uint32,CreatureData> CreatureDataMap;
typedef UNORDERED_MAP<uint32,GameObjectData> GameObjectDataMap;
typedef UNORDERED_MAP<uint32,CreatureLocale> CreatureLocaleMap;
typedef UNORDERED_MAP<uint32,GameObjectLocale> GameObjectLocaleMap;
typedef UNORDERED_MAP<uint32,ItemLocale> ItemLocaleMap;
typedef UNORDERED_MAP<uint32,QuestLocale> QuestLocaleMap;
typedef UNORDERED_MAP<uint32,NpcTextLocale> NpcTextLocaleMap;
typedef UNORDERED_MAP<uint32,PageTextLocale> PageTextLocaleMap;
typedef UNORDERED_MAP<uint32,HellgroundStringLocale> HellgroundStringLocaleMap;
typedef UNORDERED_MAP<uint32,NpcOptionLocale> NpcOptionLocaleMap;
typedef UNORDERED_MAP<uint16,Timer> OpcodesCooldown;

typedef std::multimap<uint32,uint32> QuestRelations;

struct PetLevelInfo
{
    PetLevelInfo() : health(0), mana(0) { for (int i=0; i < MAX_STATS; ++i) stats[i] = 0; }

    uint16 stats[MAX_STATS];
    uint16 health;
    uint16 mana;
    uint16 armor;
};

// We assume the rate is in general the same for all three types below, but chose to keep three for scalability and customization
struct RepRewardRate
{
    float quest_rate;           // We allow rate = 0.0 in database. For this case,
    float creature_rate;        // it means that no reputation are given at all
    float spell_rate;           // for this faction/rate type.
};

struct ReputationOnKillEntry
{
    uint32 repfaction1;
    uint32 repfaction2;
    bool is_teamaward1;
    uint32 reputation_max_cap1;
    int32 repvalue1;
    bool is_teamaward2;
    uint32 reputation_max_cap2;
    int32 repvalue2;
    bool team_dependent;
};

struct RepSpilloverTemplate
{
    uint32 faction[MAX_SPILLOVER_FACTIONS];
    float faction_rate[MAX_SPILLOVER_FACTIONS];
    uint32 faction_rank[MAX_SPILLOVER_FACTIONS];
};

struct PetCreateSpellEntry
{
    uint32 spellid[4];
};

#define WEATHER_SEASONS 4
struct WeatherSeasonChances
{
    uint32 rainChance;
    uint32 snowChance;
    uint32 stormChance;
};

struct WeatherZoneChances
{
    WeatherSeasonChances data[WEATHER_SEASONS];
};

struct GraveYardData
{
    uint32 safeLocId;
    PlayerTeam team;
};
typedef std::multimap<uint32,GraveYardData> GraveYardMap;

enum ConditionType
{                                                           // value1       value2  for the Condition enumed
    CONDITION_NONE                  = 0,                    // 0            0
    CONDITION_AURA                  = 1,                    // spell_id     effindex
    CONDITION_ITEM                  = 2,                    // item_id      count
    CONDITION_ITEM_EQUIPPED         = 3,                    // item_id      0
    CONDITION_ZONEID                = 4,                    // zone_id      0
    CONDITION_REPUTATION_RANK       = 5,                    // faction_id   min_rank
    CONDITION_TEAM                  = 6,                    // player_team  0,      (469 - Alliance 67 - Horde)
    CONDITION_SKILL                 = 7,                    // skill_id     skill_value
    CONDITION_QUESTREWARDED         = 8,                    // quest_id     0
    CONDITION_QUESTTAKEN            = 9,                    // quest_id     0,      for condition true while quest active.
    CONDITION_AD_COMMISSION_AURA    = 10,                   // 0            0,      for condition true while one from AD �ommission aura active
    CONDITION_NO_AURA               = 11,                   // spell_id     effindex
    CONDITION_ACTIVE_EVENT          = 12,                   // event_id
    CONDITION_INSTANCE_DATA         = 13,                   // entry        data
    CONDITION_UNK                   = 14,
    CONDITION_RAIDCHEST             = 15,                   // CUSTOM: Class mask and spec, add flags together for all classes where condition is true
    CONDITION_HEROIC_RAID           = 16,				    // CUSOM
};

#define MAX_CONDITION                 16                    // maximum value in ConditionType enum
#define CONDITION_DONT_LOAD           100                   // condition used to skip the load of certain loot row

struct PlayerCondition
{
    ConditionType condition;                                // additional condition type
    uint32  value1;                                         // data for the condition - see ConditionType definition
    uint32  value2;
    uint32  value3;

    PlayerCondition(uint8 _condition = 0, uint32 _value1 = 0, uint32 _value2 = 0, uint32 _value3 = 0)
        : condition(ConditionType(_condition)), value1(_value1), value2(_value2), value3(_value3) {}

    static bool IsValid(ConditionType condition, uint32 value1, uint32 value2);
    // Checks correctness of values
    bool Meets(Player const * APlayer, uint32 special = 0) const;               // Checks if the player meets the condition
    bool operator == (PlayerCondition const& lc) const
    {
        return (lc.condition == condition && lc.value1 == value1 && lc.value2 == value2 && lc.value3 == value3);
    }
};

// NPC gossip text id
typedef UNORDERED_MAP<uint32, uint32> CacheNpcTextIdMap;
typedef std::list<GossipOption> CacheNpcOptionList;

typedef UNORDERED_MAP<uint32, VendorItemData> CacheVendorItemMap;
typedef UNORDERED_MAP<uint32, TrainerSpellData> CacheTrainerSpellMap;

enum SkillRangeType
{
    SKILL_RANGE_LANGUAGE,                                   // 300..300
    SKILL_RANGE_LEVEL,                                      // 1..max skill for level
    SKILL_RANGE_MONO,                                       // 1..1, grey monolite bar
    SKILL_RANGE_RANK,                                       // 1..skill for known rank
    SKILL_RANGE_NONE,                                       // 0..0 always
};

SkillRangeType GetSkillRangeType(SkillLineEntry const *pSkill, bool racial);

#define MAX_PLAYER_NAME 12                                  // max allowed by client name length
#define MAX_INTERNAL_PLAYER_NAME 15                         // max server internal player name length (> MAX_PLAYER_NAME for support declined names)

bool normalizePlayerName(std::string& name);

struct HELLGROUND_IMPORT_EXPORT LanguageDesc
{
    Language lang_id;
    uint32   spell_id;
    uint32   skill_id;
};

extern LanguageDesc lang_description[LANGUAGES_COUNT];
HELLGROUND_IMPORT_EXPORT LanguageDesc const* GetLanguageDescByID(uint32 lang);

// don't go over SINGLEINSTANCE_RESERVED_INSTANCES_LAST
enum
{
    MAP0_START = 1,
    MAP0_TOP_NORTH = 1,
    MAP0_MIDDLE_NORTH = 2,
    MAP0_IRONFORGE_AREA = 3,
    MAP0_MIDDLE = 4,    // Burning stepps, Redridge monts, Blasted lands
    MAP0_STORMWIND_AREA = 5,    // Stormwind, Elwynn forest, Redridge Mts
    MAP0_SOUTH = 6,    // Southern phase of the continent
    MAP0_END = 7,

    MAP1_START = 11,
    MAP1_NORTH = 11,
    MAP1_DUROTAR = 12,
    MAP1_MIDDLE_EST = 13,   // Mulgore, Barrens, Dustwallow Marsh
    MAP1_SOUTH = 14,   // Feralas and south
    MAP1_VALLEY = 15,   // Orc and Troll starting area
    MAP1_END = 16,

    MAP530_START = 20,
    MAP530_DEF = 20,
    MAP530_END = 21,
};

class ObjectMgr
{
    friend class ACE_Singleton<ObjectMgr, ACE_Null_Mutex>;
    ObjectMgr();

    public:
        ~ObjectMgr();

        typedef UNORDERED_MAP<uint32, Item*> ItemMap;

        typedef std::set< Group * > GroupSet;
        typedef std::list<Roll*> Rolls;

        typedef UNORDERED_MAP<uint32, ArenaTeam*> ArenaTeamMap;
        typedef UNORDERED_MAP<uint32, Quest*> QuestMap;
        typedef UNORDERED_MAP<uint32, AreaTrigger> AreaTriggerMap;
        typedef UNORDERED_MAP<uint32, AccessRequirement> AccessRequirementMap;

        typedef UNORDERED_MAP<uint32, RepRewardRate > RepRewardRateMap;
        typedef UNORDERED_MAP<uint32, ReputationOnKillEntry> RepOnKillMap;
        typedef UNORDERED_MAP<uint32, RepSpilloverTemplate> RepSpilloverTemplateMap;

        typedef UNORDERED_MAP<uint32, WeatherZoneChances> WeatherZoneMap;
        typedef UNORDERED_MAP<uint32, PetCreateSpellEntry> PetCreateSpellMap;

        UNORDERED_MAP<uint32, uint32> TransportEventMap;

        Player* GetPlayerInWorld(const char* name) const { return sObjectAccessor.GetPlayerByNameInWorld(name);}
        Player* GetPlayerInWorld(uint64 guid) const { return ObjectAccessor::GetPlayerInWorld(guid); }
        Player* GetPlayerOnline(uint64 guid) const { return ObjectAccessor::GetPlayerOnline(guid);  }

        static GameObjectInfo const *GetGameObjectInfo(uint32 id) { return sGOStorage.LookupEntry<GameObjectInfo>(id); }

        void LoadGameobjectInfo();
        void AddGameobjectInfo(GameObjectInfo *goinfo);

        Group * GetGroupByLeader(const uint64 &guid) const;
        void AddGroup(Group* group) { mGroupSet.insert(group); }
        void RemoveGroup(Group* group) { mGroupSet.erase(group); }
        GroupSet::iterator GetGroupSetBegin() { return mGroupSet.begin(); }
        GroupSet::iterator GetGroupSetEnd()   { return mGroupSet.end(); }
        void UpdateRolls(uint32 diff);
        void AddRoll(Roll* r) { mLootRolls.push_back(r); };
        void CountRollVote(const uint64& playerGUID, const uint64& Guid, uint8 Choice);

        HELLGROUND_EXPORT ArenaTeam* GetArenaTeamById(const uint32 arenateamid) const;

        HELLGROUND_EXPORT void ModifyArenaRating(uint32 guid, int32 amount, uint8 type);
		HELLGROUND_EXPORT void BGModStats(uint32 guid, bool won, bool change_rating);

        HELLGROUND_IMPORT_EXPORT ArenaTeam* GetArenaTeamByName(const std::string& arenateamname) const;
        ArenaTeam* GetArenaTeamByCaptain(uint64 const& guid) const;
        HELLGROUND_IMPORT_EXPORT void AddArenaTeam(ArenaTeam* arenaTeam);
        void RemoveArenaTeam(uint32 Id);
        ArenaTeamMap::iterator GetArenaTeamMapBegin() { return mArenaTeamMap.begin(); }
        ArenaTeamMap::iterator GetArenaTeamMapEnd()   { return mArenaTeamMap.end(); }

        uint32 GenerateTempArenaTeamId();

        static CreatureInfo const *GetCreatureTemplate(uint32 id);
        CreatureModelInfo const *GetCreatureModelInfo(uint32 modelid);
        CreatureModelInfo const* GetCreatureModelRandomGender(uint32 display_id);
        uint32 ChooseDisplayId(uint32 team, const CreatureInfo *cinfo, const CreatureData *data = NULL, GameEventCreatureData const* eventData = NULL);
        EquipmentInfo const *GetEquipmentInfo(uint32 entry);
        static SpellEntry const* GetSpellEntry(uint32 spell_id)
        {
            return sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
        }

        static CreatureDataAddon const *GetCreatureAddon(uint32 lowguid)
        {
            return sCreatureDataAddonStorage.LookupEntry<CreatureDataAddon>(lowguid);
        }

        static CreatureDataAddon const *GetCreatureTemplateAddon(uint32 entry)
        {
            return sCreatureInfoAddonStorage.LookupEntry<CreatureDataAddon>(entry);
        }

        HELLGROUND_IMPORT_EXPORT static ItemPrototype const* GetItemPrototype(uint32 id) { return sItemStorage.LookupEntry<ItemPrototype>(id); }

        static InstanceTemplate const* GetInstanceTemplate(uint32 map)
        {
            return sInstanceTemplate.LookupEntry<InstanceTemplate>(map);
        }

        PetLevelInfo const* GetPetLevelInfo(uint32 creature_id, uint32 level) const;

        PlayerClassInfo const* GetPlayerClassInfo(uint32 class_) const
        {
            if (class_ >= MAX_CLASSES) return NULL;
            return &playerClassInfo[class_];
        }
        void GetPlayerClassLevelInfo(uint32 class_,uint32 level, PlayerClassLevelInfo* info) const;

        PlayerInfo const* GetPlayerInfo(uint32 race, uint32 class_) const
        {
            if (race   >= MAX_RACES)   return NULL;
            if (class_ >= MAX_CLASSES) return NULL;
            PlayerInfo const* info = &playerInfo[race][class_];
            if (info->displayId_m==0 || info->displayId_f==0) return NULL;
            return info;
        }
        void GetPlayerLevelInfo(uint32 race, uint32 class_,uint32 level, PlayerLevelInfo* info) const;

        uint64 GetPlayerGUIDByName(std::string name) const;
        bool GetPlayerNameByGUID(const uint64 &guid, std::string &name) const;
        uint32 GetPlayerTeamByGUID(const uint64 &guid) const;
        uint32 GetPlayerAccountIdByGUID(const uint64 &guid) const;
        uint32 GetPlayerAccountIdByPlayerName(const std::string& name) const;

        uint32 GetNearestTaxiNode(float x, float y, float z, uint32 mapid, uint32 team);
        void GetTaxiPath(uint32 source, uint32 destination, uint32 &path, uint32 &cost);
        uint16 GetTaxiMount(uint32 id, uint32 team);

        Quest const* GetQuestTemplate(uint32 quest_id) const
        {
            QuestMap::const_iterator itr = mQuestTemplates.find(quest_id);
            return itr != mQuestTemplates.end() ? itr->second : NULL;
        }
        QuestMap const& GetQuestTemplates() const { return mQuestTemplates; }

        uint32 GetQuestForAreaTrigger(uint32 Trigger_ID) const
        {
            QuestAreaTriggerMap::const_iterator itr = mQuestAreaTriggerMap.find(Trigger_ID);
            if (itr != mQuestAreaTriggerMap.end())
                return itr->second;
            return 0;
        }

        bool IsTavernAreaTrigger(uint32 Trigger_ID) const
        {
            return mTavernAreaTriggerSet.find(Trigger_ID) != mTavernAreaTriggerSet.end();
        }

        GossipText const * GetGossipText(uint32 Text_ID) const;

        WorldSafeLocsEntry const *GetClosestGraveYard(float x, float y, float z, uint32 MapId, PlayerTeam team);
        bool AddGraveYardLink(uint32 id, uint32 zone, PlayerTeam team, bool inDB = true);
        void RemoveGraveYardLink(uint32 id, uint32 zone, PlayerTeam team, bool inDB = false);
        void LoadGraveyardZones();
        GraveYardData const* FindGraveYardData(uint32 id, uint32 zone);

        AreaTrigger const* GetAreaTrigger(uint32 trigger) const
        {
            AreaTriggerMap::const_iterator itr = mAreaTriggers.find(trigger);
            if (itr != mAreaTriggers.end())
                return &itr->second;
            return NULL;
        }

        AccessRequirement const* GetAccessRequirement(uint32 requirement) const
        {
            AccessRequirementMap::const_iterator itr = mAccessRequirements.find(requirement);
            if (itr != mAccessRequirements.end())
                return &itr->second;
            return NULL;
        }

        AreaTrigger const* GetGoBackTrigger(uint32 Map) const;
        AreaTrigger const* GetMapEntranceTrigger(uint32 Map) const;

        RepRewardRate const* GetRepRewardRate(uint32 factionId) const
        {
            RepRewardRateMap::const_iterator itr = m_RepRewardRateMap.find(factionId);
            if (itr != m_RepRewardRateMap.end())
                return &itr->second;

            return NULL;
        }

        ReputationOnKillEntry const* GetReputationOnKillEntry(uint32 id) const
        {
            // custom heroic creatures
            if (id > 9300000)
                id -= 9300000;

            RepOnKillMap::const_iterator itr = mRepOnKill.find(id);
            if (itr != mRepOnKill.end())
                return &itr->second;

            return NULL;
        }

        RepSpilloverTemplate const* GetRepSpilloverTemplate(uint32 factionId) const
        {
            RepSpilloverTemplateMap::const_iterator itr = m_RepSpilloverTemplateMap.find(factionId);
            if (itr != m_RepSpilloverTemplateMap.end())
                return &itr->second;

            return NULL;
        }

        PetCreateSpellEntry const* GetPetCreateSpellEntry(uint32 id) const
        {
            PetCreateSpellMap::const_iterator itr = mPetCreateSpell.find(id);
            if (itr != mPetCreateSpell.end())
                return &itr->second;

            return NULL;
        }

        void LoadArenaTeams();
        void LoadGroups();
        void LoadQuests();
        void LoadQuestRelations()
        {
            LoadGameobjectQuestRelations();
            LoadGameobjectInvolvedRelations();
            LoadCreatureQuestRelations();
            LoadCreatureInvolvedRelations();
        }
        void LoadGameobjectQuestRelations();
        void LoadGameobjectInvolvedRelations();
        void LoadCreatureQuestRelations();
        void LoadCreatureInvolvedRelations();

        QuestRelations mGOQuestRelations;
        QuestRelations mGOQuestInvolvedRelations;
        QuestRelations mCreatureQuestRelations;
        QuestRelations mCreatureQuestInvolvedRelations;

        void LoadTransportEvents();

        bool LoadHellgroundStrings(DatabaseType& db, char const* table, int32 min_value, int32 max_value);
        bool LoadHellgroundStrings() { return LoadHellgroundStrings(GameDataDatabase,"hellground_string",MIN_HELLGROUND_STRING_ID,MAX_HELLGROUND_STRING_ID); }

        void LoadPetCreateSpells();
        void LoadCreatureLocales();
        void LoadCreatureTemplates();
        void LoadCreatures();
        void LoadCreatureLinkedRespawn();
        bool CheckCreatureLinkedRespawn(uint32 guid, uint32 linkedGuid) const;
        bool SetCreatureLinkedRespawn(uint32 guid, uint32 linkedGuid);
        void LoadCreatureRespawnTimes();
        void LoadUnqueuedAccountList();
        bool IsUnqueuedAccount(uint64 accid);
        void LoadCreatureAddons();
        void LoadCreatureModelInfo();
        void LoadEquipmentTemplates();
        void LoadGameObjectLocales();
        void LoadGameobjects();
        void LoadGameobjectRespawnTimes();
        void LoadItemPrototypes();
        void LoadItemLocales();
        void LoadQuestLocales();
        void LoadNpcTextLocales();
        void LoadPageTextLocales();
        void LoadNpcOptionLocales();
        void LoadInstanceTemplate();

        void LoadGossipText();

        void LoadAreaTriggerTeleports();
        void LoadAccessRequirements();
        void LoadQuestAreaTriggers();
        void LoadTavernAreaTriggers();
        void LoadGameObjectForQuests();

        void LoadItemTexts();
        void LoadPageTexts();

        void LoadPlayerInfo();
        void LoadPetLevelInfo();
        void LoadExplorationBaseXP();
        void LoadPetNames();
        void LoadPetNumber();
        void LoadCorpses();
        void LoadFishingBaseSkillLevel();

        void LoadReputationRewardRate();
        void LoadReputationOnKill();
        void LoadReputationSpilloverTemplate();

        void LoadWeatherZoneChances();
        void LoadGameTele();

        void LoadNpcOptions();
        void LoadNpcTextId();
        void LoadVendors();
        void LoadTrainerSpell();

        void LoadSpellTemplate();

        void LoadOpcodesCooldown();
        OpcodesCooldown GetOpcodesCooldown() { return _opcodesCooldown; }

        void LoadRevision();

        bool AddWardenWarning(uint32 accountid); // returns true if already has one, false if not

        std::string GeneratePetName(uint32 entry);
        uint32 GetBaseXP(uint32 level);

        int32 GetFishingBaseSkillLevel(uint32 entry) const
        {
            FishingBaseSkillMap::const_iterator itr = mFishingBaseForArea.find(entry);
            return itr != mFishingBaseForArea.end() ? itr->second : 0;
        }

        void ReturnOrDeleteOldMails(bool serverUp);

        void SetHighestGuids();
        /*This method is used for generating guids for temporary creatures, which are not saved into DB*/
        uint32 GenerateLowGuid(HighGuid guidhigh);
        /* used for generating temporary object guids - these guids are only used in log files. they may repeat every 49 days as uint32 seconds*/
        uint32 GenerateTempGuid(TempGuid guidType);
        uint32 GenerateAuctionID();
        uint32 GenerateMailID();
        uint32 GenerateItemTextID();
        uint32 GeneratePetNumber();
        uint32 GenerateArenaTeamId();

        uint32 CreateItemText(std::string text);
        std::string GetItemText(uint32 id)
        {
            ItemTextMap::const_iterator itr = mItemTexts.find(id);
            if (itr != mItemTexts.end())
                return itr->second;
            else
                return "There is no info for this item";
        }

        typedef std::multimap<int32, uint32> ExclusiveQuestGroups;
        ExclusiveQuestGroups mExclusiveQuestGroups;

        WeatherZoneChances const* GetWeatherChances(uint32 zone_id) const
        {
            WeatherZoneMap::const_iterator itr = mWeatherZoneMap.find(zone_id);
            if (itr != mWeatherZoneMap.end())
                return &itr->second;
            else
                return NULL;
        }

        CellObjectGuids const& GetCellObjectGuids(uint16 mapid, uint8 spawnMode, uint32 cell_id)
        {
            return mMapObjectGuids[MAKE_PAIR32(mapid,spawnMode)][cell_id];
        }

        CreatureData const* GetCreatureData(uint32 guid) const
        {
            CreatureDataMap::const_iterator itr = mCreatureDataMap.find(guid);
            if (itr==mCreatureDataMap.end()) return NULL;
            return &itr->second;
        }

        CreatureData& NewOrExistCreatureData(uint32 guid) { return mCreatureDataMap[guid]; }
        CreatureDataMap const& GetCreatureDataMap() { return mCreatureDataMap; }
        void DeleteCreatureData(uint32 guid);

        uint32 GetLinkedRespawnGuid(uint32 guid) const
        {
            CreatureLinkedRespawnMap::const_iterator itr = mCreatureLinkedRespawnMap.find(guid);
            if (itr == mCreatureLinkedRespawnMap.end()) return 0;
            return itr->second;
        }

        CreatureLocale const* GetCreatureLocale(uint32 entry) const
        {
            CreatureLocaleMap::const_iterator itr = mCreatureLocaleMap.find(entry);
            if (itr==mCreatureLocaleMap.end()) return NULL;
            return &itr->second;
        }

        void GetCreatureLocaleStrings(uint32 entry, int32 loc_idx, char const** namePtr, char const** subnamePtr = NULL) const;

        GameObjectLocale const* GetGameObjectLocale(uint32 entry) const
        {
            GameObjectLocaleMap::const_iterator itr = mGameObjectLocaleMap.find(entry);
            if (itr==mGameObjectLocaleMap.end()) return NULL;
            return &itr->second;
        }

        ItemLocale const* GetItemLocale(uint32 entry) const
        {
            ItemLocaleMap::const_iterator itr = mItemLocaleMap.find(entry);
            if (itr==mItemLocaleMap.end()) return NULL;
            return &itr->second;
        }

        HELLGROUND_IMPORT_EXPORT void GetItemLocaleStrings(uint32 entry, int32 loc_idx, std::string* namePtr, std::string* descriptionPtr = NULL) const;

        QuestLocale const* GetQuestLocale(uint32 entry) const
        {
            QuestLocaleMap::const_iterator itr = mQuestLocaleMap.find(entry);
            if (itr==mQuestLocaleMap.end()) return NULL;
            return &itr->second;
        }

        void GetQuestLocaleStrings(uint32 entry, int32 loc_idx, std::string* titlePtr) const;

        NpcTextLocale const* GetNpcTextLocale(uint32 entry) const
        {
            NpcTextLocaleMap::const_iterator itr = mNpcTextLocaleMap.find(entry);
            if (itr==mNpcTextLocaleMap.end()) return NULL;
            return &itr->second;
        }

        PageTextLocale const* GetPageTextLocale(uint32 entry) const
        {
            PageTextLocaleMap::const_iterator itr = mPageTextLocaleMap.find(entry);
            if (itr==mPageTextLocaleMap.end()) return NULL;
            return &itr->second;
        }

        NpcOptionLocale const* GetNpcOptionLocale(uint32 entry) const
        {
            NpcOptionLocaleMap::const_iterator itr = mNpcOptionLocaleMap.find(entry);
            if (itr==mNpcOptionLocaleMap.end()) return NULL;
            return &itr->second;
        }

        const char* GetNpcOptionLocaleString(uint32 entry, int loc_idx) const;

        typedef std::string NpcTextArray[MAX_GOSSIP_TEXT_OPTIONS];
        void GetNpcTextLocaleStringsAll(uint32 entry, int32 loc_idx, NpcTextArray *text0_Ptr, NpcTextArray* text1_Ptr) const;
        void GetNpcTextLocaleStrings0(uint32 entry, int32 loc_idx, std::string* text0_0_Ptr, std::string* text1_0_Ptr) const;

        GameObjectData const* GetGOData(uint32 guid) const
        {
            GameObjectDataMap::const_iterator itr = mGameObjectDataMap.find(guid);
            if (itr==mGameObjectDataMap.end()) return NULL;
            return &itr->second;
        }

        GameObjectData& NewGOData(uint32 guid) { return mGameObjectDataMap[guid]; }
        GameObjectDataMap const& GetGODataMap() { return mGameObjectDataMap; }
        void DeleteGOData(uint32 guid);

        HellgroundStringLocale const* GetHellgroundStringLocale(int32 entry) const
        {
            HellgroundStringLocaleMap::const_iterator itr = mHellgroundStringLocaleMap.find(entry);
            if (itr==mHellgroundStringLocaleMap.end()) return NULL;
            return &itr->second;
        }
        const char *GetHellgroundString(int32 entry, int locale_idx) const;
        const char *GetHellgroundStringForDBCLocale(int32 entry) const { return GetHellgroundString(entry,DBCLocaleIndex); }
        int32 GetDBCLocaleIndex() const { return DBCLocaleIndex; }
        void SetDBCLocaleIndex(uint32 lang) { DBCLocaleIndex = GetIndexForLocale(LocaleConstant(lang)); }

        void AddCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid, uint32 instance);
        void DeleteCorpseCellData(uint32 mapid, uint32 cellid, uint32 player_guid);

        time_t GetCreatureRespawnTime(uint32 loguid, uint32 instance) { return mCreatureRespawnTimes[MAKE_PAIR64(loguid,instance)]; }
        void SaveCreatureRespawnTime(uint32 loguid, uint32 instance, uint32 map_id, bool is_raid, time_t t);
        time_t GetGORespawnTime(uint32 loguid, uint32 instance) { return mGORespawnTimes[MAKE_PAIR64(loguid,instance)]; }
        void SaveGORespawnTime(uint32 loguid, uint32 instance, time_t t);
        void DeleteRespawnTimeForInstance(uint32 instance);

        // grid objects
        void AddCreatureToGrid(uint32 guid, CreatureData const* data);
        void RemoveCreatureFromGrid(uint32 guid, CreatureData const* data);
        void AddGameobjectToGrid(uint32 guid, GameObjectData const* data);
        void RemoveGameobjectFromGrid(uint32 guid, GameObjectData const* data);

        uint32 AddGOData(uint32 entry, uint32 ArtKit, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0, float rotation0 = 0, float rotation1 = 0, float rotation2 = 0, float rotation3 = 0);
        uint32 AddCreData(uint32 entry, uint32 team, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0);

        // reserved names
        void LoadReservedPlayersNames();
        bool IsReservedName(const std::string& name, uint32 accid = 0);

        // name with valid structure and symbols
        HELLGROUND_EXPORT static bool IsValidName(const std::string& name, bool create = false);
        static bool IsValidCharterName(const std::string& name);
        static bool IsValidPetName(const std::string& name);

        static bool CheckDeclinedNames(std::wstring const& name, DeclinedName const& names);

        void LoadSpellDisabledEntrys();
        bool IsPlayerSpellDisabled(uint32 spellid) { return (m_DisabledPlayerSpells.count(spellid) != 0); }
        bool IsCreatureSpellDisabled(uint32 spellid) { return (m_DisabledCreatureSpells.count(spellid) != 0); }
        bool IsPetSpellDisabled(uint32 spellid) { return (m_DisabledPetSpells.count(spellid) != 0); }

        HELLGROUND_IMPORT_EXPORT int GetIndexForLocale(LocaleConstant loc);
        LocaleConstant GetLocaleForIndex(int i);

        uint16 GetConditionId(ConditionType condition, uint32 value1, uint32 value2, uint32 value3);
        
        // raidchest
        bool SkipItemCondition(Player const* player, uint16 condition_id) const
        {
            return mConditions[condition_id].Meets(player, 1);
        }
        
        bool IsPlayerMeetToCondition(Player const* player, uint16 condition_id) const
        {
            sLog.outLog(LOG_TMP, "RC_DEBUG IsPlayerMeetToCondition - Player %s condition %u", player->GetName(), condition_id);
            
            if (condition_id >= mConditions.size())
                return false;

            return mConditions[condition_id].Meets(player);
        }

        GameTele const* GetGameTele(uint32 id) const
        {
            GameTeleMap::const_iterator itr = m_GameTeleMap.find(id);
            if (itr==m_GameTeleMap.end()) return NULL;
            return &itr->second;
        }
        GameTele const* GetGameTele(const std::string& name) const;
        GameTeleMap const& GetGameTeleMap() const { return m_GameTeleMap; }
        bool AddGameTele(GameTele& data);
        bool DeleteGameTele(const std::string& name);

        CacheNpcOptionList const& GetNpcOptions() const { return m_mCacheNpcOptionList; }

        uint32 GetNpcGossip(uint32 entry) const
        {
            CacheNpcTextIdMap::const_iterator iter = m_mCacheNpcTextIdMap.find(entry);
            if (iter == m_mCacheNpcTextIdMap.end())
                return 0;

            return iter->second;
        }

        TrainerSpellData const* GetNpcTrainerSpells(uint32 entry) const
        {
            CacheTrainerSpellMap::const_iterator  iter = m_mCacheTrainerSpellMap.find(entry);
            if (iter == m_mCacheTrainerSpellMap.end())
                return NULL;

            return &iter->second;
        }

        VendorItemData const* GetNpcVendorItemList(uint32 entry) const
        {
            CacheVendorItemMap::const_iterator  iter = m_mCacheVendorItemMap.find(entry);
            if (iter == m_mCacheVendorItemMap.end())
                return NULL;

            return &iter->second;
        }
        void AddVendorItem(uint32 entry,uint32 item, uint32 maxcount, uint32 incrtime, uint32 ExtendedCost, bool savetodb = true); // for event
        bool RemoveVendorItem(uint32 entry,uint32 item, bool savetodb = true); // for event
        bool IsVendorItemValid(uint32 vendor_entry, uint32 item, uint32 maxcount, uint32 ptime, uint32 ExtendedCost, Player* pl = NULL, std::set<uint32>* skip_vendors = NULL, uint32 ORnpcflag = 0) const;

        HELLGROUND_EXPORT static ObjectMgr* getObjectMgrFromScripts();

        /*Returns 0 if this position in this map is not instanciated*/
        uint32 GetSingleInstance(uint32 map, float x, float y) const;
        void ScheduleSingleInstanceChange(Player* player);
        void SingleInstanceChangeApply();

        void ModifyPersonalRating(ArenaTeamMember& atm, Player* plr, int32 mod, uint32 slot)
        {
            if (atm.personal_rating + mod < 0)
                atm.personal_rating = 0;
            else
                atm.personal_rating += mod;

            if (plr)
                plr->SetArenaTeamInfoField(slot, 5, atm.personal_rating);
        }

		std::set<uint32> m_recent_bossfightinstance_entry; // recent entries to prevent duplicates and select from DB

    protected:

        // first free id for selected id type
        uint32 m_auctionid;
        uint32 m_mailid;
        ACE_Atomic_Op<ACE_Thread_Mutex, uint32> m_ItemTextId; // thread-safe
        uint32 m_arenaTeamId;
        uint32 m_arenaTeamIdTemp;

        uint32 m_hiPetNumber;

        // first free low guid for seelcted guid type
        uint32 m_hiCharGuid;
        uint32 m_hiCreatureGuid;
        uint32 m_hiPetGuid;
        ACE_Atomic_Op<ACE_Thread_Mutex, uint32> m_hiItemGuid; // thead-safe
        uint32 m_hiGoGuid;
        uint32 m_hiDoGuid;
        uint32 m_hiCorpseGuid;

        uint32 m_tempArenaGuid;
        uint32 m_tempGroupGuid;

        QuestMap            mQuestTemplates;

        typedef UNORDERED_MAP<uint32, GossipText> GossipTextMap;
        typedef UNORDERED_MAP<uint32, uint32> QuestAreaTriggerMap;
        typedef UNORDERED_MAP<uint32, std::string> ItemTextMap;
        typedef std::set<uint32> TavernAreaTriggerSet;
        typedef std::set<uint32> GameObjectForQuestSet;

        GroupSet                mGroupSet;
        Rolls                   mLootRolls;
        /* unordered map */
        ArenaTeamMap            mArenaTeamMap;

        ItemTextMap             mItemTexts;

        QuestAreaTriggerMap     mQuestAreaTriggerMap;
        TavernAreaTriggerSet    mTavernAreaTriggerSet;
        GossipTextMap           mGossipText;
        AreaTriggerMap          mAreaTriggers;
        AccessRequirementMap    mAccessRequirements;

        RepRewardRateMap        m_RepRewardRateMap;
        RepOnKillMap            mRepOnKill;
        RepSpilloverTemplateMap m_RepSpilloverTemplateMap;

        WeatherZoneMap          mWeatherZoneMap;

        PetCreateSpellMap       mPetCreateSpell;

        //character reserved names
        typedef std::set<std::pair<std::string,uint32>> ReservedNamesMap;
        ReservedNamesMap        m_ReservedNames;

        std::set<uint32>        m_DisabledPlayerSpells;
        std::set<uint32>        m_DisabledCreatureSpells;
        std::set<uint32>        m_DisabledPetSpells;
        std::set<uint64>        m_UnqueuedAccounts;

        GraveYardMap            mGraveYardMap;

        GameTeleMap             m_GameTeleMap;

        typedef                 std::vector<LocaleConstant> LocalForIndex;
        LocalForIndex           m_LocalForIndex;
        int GetOrNewIndexForLocale(LocaleConstant loc);

        int DBCLocaleIndex;

    private:
        void ConvertCreatureAddonAuras(CreatureDataAddon* addon, char const* table, char const* guidEntryStr);
        void LoadQuestRelationsHelper(QuestRelations& map,char const* table);

        typedef std::map<uint32,PetLevelInfo*> PetLevelInfoMap;
        // PetLevelInfoMap[creature_id][level]
        PetLevelInfoMap petInfo;                            // [creature_id][level]

        PlayerClassInfo playerClassInfo[MAX_CLASSES];

        void BuildPlayerLevelInfo(uint8 race, uint8 class_, uint8 level, PlayerLevelInfo* plinfo) const;
        PlayerInfo playerInfo[MAX_RACES][MAX_CLASSES];

        typedef std::map<uint32,uint32> BaseXPMap;          // [area level][base xp]
        BaseXPMap mBaseXPTable;

        typedef std::map<uint32,int32> FishingBaseSkillMap; // [areaId][base skill level]
        FishingBaseSkillMap mFishingBaseForArea;

        typedef std::map<uint32,std::vector<std::string> > HalfNameMap;
        HalfNameMap PetHalfName0;
        HalfNameMap PetHalfName1;

        MapObjectGuids mMapObjectGuids;
        CreatureDataMap mCreatureDataMap;
        CreatureLinkedRespawnMap mCreatureLinkedRespawnMap;
        CreatureLocaleMap mCreatureLocaleMap;
        GameObjectDataMap mGameObjectDataMap;
        GameObjectLocaleMap mGameObjectLocaleMap;
        ItemLocaleMap mItemLocaleMap;
        QuestLocaleMap mQuestLocaleMap;
        NpcTextLocaleMap mNpcTextLocaleMap;
        PageTextLocaleMap mPageTextLocaleMap;
        HellgroundStringLocaleMap mHellgroundStringLocaleMap;
        NpcOptionLocaleMap mNpcOptionLocaleMap;
        RespawnTimes mCreatureRespawnTimes;
        RespawnTimes mGORespawnTimes;

        OpcodesCooldown _opcodesCooldown;

        // Storage for Conditions. First element (index 0) is reserved for zero-condition (nothing required)
        typedef std::vector<PlayerCondition> ConditionStore;
        ConditionStore mConditions;

        CacheNpcOptionList m_mCacheNpcOptionList;
        CacheNpcTextIdMap m_mCacheNpcTextIdMap;
        CacheVendorItemMap m_mCacheVendorItemMap;
        CacheTrainerSpellMap m_mCacheTrainerSpellMap;

        ACE_Thread_Mutex    m_mutex_scheduledSInstanceChange;
        std::set<Player*> m_scheduledSInstanceChanges;
};

enum LanguageType
{
    LT_BASIC_LATIN    = 0x0000,
    LT_EXTENDEN_LATIN = 0x0001,
    LT_CYRILLIC       = 0x0002,
    LT_EAST_ASIA      = 0x0004,
    LT_ANY            = 0xFFFF
};

#define sObjectMgr (*ACE_Singleton<ObjectMgr, ACE_Null_Mutex>::instance())
#define getObjectMgr (ObjectMgr::getObjectMgrFromScripts)

// scripting access functions
HELLGROUND_IMPORT_EXPORT bool LoadHellgroundStrings(DatabaseType& db, char const* table,int32 start_value = MAX_CREATURE_AI_TEXT_STRING_ID, int32 end_value = std::numeric_limits<int32>::min());
HELLGROUND_IMPORT_EXPORT GameObjectInfo const *GetGameObjectInfo(uint32 id);
HELLGROUND_IMPORT_EXPORT CreatureInfo const *GetCreatureInfo(uint32 id);
HELLGROUND_IMPORT_EXPORT CreatureInfo const* GetCreatureTemplateStore(uint32 entry);
HELLGROUND_IMPORT_EXPORT Quest const* GetQuestTemplateStore(uint32 entry);

#endif
