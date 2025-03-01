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

#ifndef HELLGROUND_LOOTMGR_H
#define HELLGROUND_LOOTMGR_H

#include "ItemEnchantmentMgr.h"
#include "ByteBuffer.h"
#include "Utilities/LinkedReference/RefManager.h"

#include "MapManager.h"

#include <map>
#include <vector>

class Creature;

enum RollType
{
    ROLL_PASS         = 0,
    ROLL_NEED         = 1,
    ROLL_GREED        = 2
};

#define MAX_NR_LOOT_ITEMS 16
// note: the client cannot show more than 16 items total
#define MAX_NR_QUEST_ITEMS 32
// unrelated to the number of quest items shown, just for reserve

enum LootMethod
{
    FREE_FOR_ALL      = 0,
    ROUND_ROBIN       = 1,
    MASTER_LOOT       = 2,
    GROUP_LOOT        = 3,
    NEED_BEFORE_GREED = 4
};

enum PermissionTypes
{
    ALL_PERMISSION           = 0,
    GROUP_LOOTER_PERMISSION  = 1,
    GROUP_NONE_PERMISSION    = 2,
    MASTER_PERMISSION        = 3,
    NONE_PERMISSION          = 4
};

class WorldObject;
class Player;
class LootStore;

struct LootStoreItem
{
    uint32  itemid;                                         // id of the item
    float   chance;                                         // always positive, chance to drop for both quest and non-quest items, chance to be used for refs
    int32   mincountOrRef;                                  // mincount for drop items (positive) or minus referenced TemplateleId (negative)
    uint8   group       :8;
    uint8   maxcount    :8;                                 // max drop count for the item (mincountOrRef positive) or Ref multiplicator (mincountOrRef negative)
    uint16  conditionId :16;                                // additional loot condition Id
    bool    needs_quest :1;                                 // quest drop (negative ChanceOrQuestChance in DB)
    bool    useNoConfigRates:1;

    // Constructor, converting ChanceOrQuestChance -> (chance, needs_quest)
    // displayid is filled in IsValid() which must be called after
    LootStoreItem(uint32 _itemid, float _chanceOrQuestChance, int8 _group, uint8 _conditionId, int32 _mincountOrRef, uint8 _maxcount, bool _useNoConfigRates)
        : itemid(_itemid), chance(fabs(_chanceOrQuestChance)), mincountOrRef(_mincountOrRef),
        group(_group), maxcount(_maxcount), conditionId(_conditionId),
        needs_quest(_chanceOrQuestChance < 0), useNoConfigRates(_useNoConfigRates) {}

    bool Roll(bool is_heroic = false) const;                                      // Checks if the entry takes it's chance (at loot generation)
    bool IsValid(LootStore const& store, uint32 entry) const;
                                                            // Checks correctness of values
};

struct LootItem
{
    uint32  itemid;
    uint32  randomSuffix;
    int32   randomPropertyId;
    uint16  conditionId       :16;                          // allow compiler pack structure
    uint8   count             : 8;
    bool    is_looted         : 1;
    bool    is_blocked        : 1;
    bool    freeforall        : 1;                          // free for all
    bool    is_counted        : 1;
    bool    needs_quest       : 1;                          // quest drop
    bool    canSave           : 1;

    // Constructor, copies most fields from LootStoreItem, generates random count and random suffixes/properties
    // Should be called for non-reference LootStoreItem entries only (mincountOrRef > 0)
    explicit LootItem(LootStoreItem const& li);

    explicit LootItem(uint32 id);

    // Empty constructor for creating an empty LootItem to be filled in with DB data
    LootItem() {};

    // Basic checks for player/item compatibility - if false no chance to see the item in the loot
    bool AllowedForPlayer(Player const * player, uint32 debug) const;
};

struct QuestItem
{
    uint8   index;                                          // position in quest_items;
    bool    is_looted;

    QuestItem()
        : index(0), is_looted(false) {}

    QuestItem(uint8 _index, bool _islooted = false)
        : index(_index), is_looted(_islooted) {}
};

struct Loot;
class LootTemplate;

typedef std::vector<QuestItem> QuestItemList;
typedef std::vector<LootItem> LootItemList;
typedef std::map<uint32, QuestItemList *> QuestItemMap;
typedef std::vector<LootStoreItem> LootStoreItemList;
typedef UNORDERED_MAP<uint32, LootTemplate*> LootTemplateMap;

typedef std::set<uint32> LootIdSet;

class LootStore
{
    public:
        explicit LootStore(char const* name, char const* entryName) : m_name(name), m_entryName(entryName) {}
        virtual ~LootStore() { Clear(); }

        void Verify() const;

        void LoadAndCollectLootIds(LootIdSet& ids_set);
        void CheckLootRefs(LootIdSet* ref_set = NULL) const;// check existence reference and remove it from ref_set
        void ReportUnusedIds(LootIdSet const& ids_set) const;
        void ReportNotExistedId(uint32 id) const;

        bool HaveLootfor (uint32 loot_id) const { return m_LootTemplates.find(loot_id) != m_LootTemplates.end(); }
        bool HaveQuestLootfor (uint32 loot_id) const;
        bool HaveQuestLootForPlayer(uint32 loot_id,Player* player) const;

        LootTemplate const* GetLootfor (uint32 loot_id) const;

        char const* GetName() const { return m_name; }
        char const* GetEntryName() const { return m_entryName; }
    protected:
        void LoadLootTable();
        void Clear();
    private:
        LootTemplateMap m_LootTemplates;
        char const* m_name;
        char const* m_entryName;
};

class LootTemplate
{
    class  LootGroup;                                       // A set of loot definitions for items (refs are not allowed inside)
    typedef std::vector<LootGroup> LootGroups;

    public:
        // Adds an entry to the group (at loading stage)
        void AddEntry(LootStoreItem& item);
        // Rolls for every item in the template and adds the rolled items the the loot
        void Process(Loot& loot, LootStore const& store, uint8 GroupId = 0, Player* player = nullptr) const;

        void CopyConditions(LootItem* li) const;

        // True if template includes at least 1 quest drop entry
        bool HasQuestDrop(LootTemplateMap const& store, uint8 GroupId = 0) const;
        // True if template includes at least 1 quest drop for an active quest of the player
        bool HasQuestDropForPlayer(LootTemplateMap const& store, Player const * player, uint8 GroupId = 0) const;

        // Checks integrity of the template
        void Verify(LootStore const& store, uint32 Id) const;
        void CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const;
    private:
        LootStoreItemList Entries;                          // not grouped only
        LootGroups        Groups;                           // groups have own (optimised) processing, grouped entries go there
};

//=====================================================

class LootValidatorRef :  public Reference<Loot, LootValidatorRef>
{
    public:
        LootValidatorRef() {}
        void targetObjectDestroyLink() {}
        void sourceObjectDestroyLink() {}
};

//=====================================================

class LootValidatorRefManager : public RefManager<Loot, LootValidatorRef>
{
    public:
        typedef LinkedListHead::Iterator< LootValidatorRef > iterator;

        LootValidatorRef* getFirst() { return (LootValidatorRef*)RefManager<Loot, LootValidatorRef>::getFirst(); }
        LootValidatorRef* getLast() { return (LootValidatorRef*)RefManager<Loot, LootValidatorRef>::getLast(); }

        iterator begin() { return iterator(getFirst()); }
        iterator end() { return iterator(NULL); }
        iterator rbegin() { return iterator(getLast()); }
        iterator rend() { return iterator(NULL); }
};

struct LootView;

ByteBuffer& operator<<(ByteBuffer& b, LootItem const& li);
ByteBuffer& operator<<(ByteBuffer& b, LootView const& lv);

//=====================================================
struct Loot
{
    friend ByteBuffer& operator<<(ByteBuffer& b, LootView const& lv);

    QuestItemMap const& GetPlayerQuestItems() const { return PlayerQuestItems; }
    QuestItemMap const& GetPlayerFFAItems() const { return PlayerFFAItems; }

    std::vector<LootItem> items;
    std::vector<LootItem> quest_items;
    std::set<uint32> unique_items;
    std::set<uint64> players_allowed_to_loot; // only creatures use it.
    std::map<uint32, std::set<std::string>> item_takers_hashes;
    //std::set<uint32> mother_accs_allowed_to_loot; // only creatures use it. For REALM_X100

    // required by round robin to see who can open loot
    ItemQualities max_quality;
    bool everyone_can_open;

    uint32 gold;
    uint8 unlootedCount;
    uint64 looterGUID;
    uint64 looterTimer;
    uint64 looterCheckTimer;

    // GUIDLow of container that holds this loot (item_instance.entry)
    //  Only set for inventory items that can be right-click looted
    uint32 containerID;

    Loot(uint32 _gold = 0) : gold(_gold), unlootedCount(0), m_lootLoadedFromDB(false), m_bossGUID(0), m_creatureGUID(0), m_mapID(0, 0), looterGUID(0), max_quality(ITEM_QUALITY_POOR), everyone_can_open(false),
        looterTimer(0), looterCheckTimer(0), containerID(0), m_from_entry(0), cast_item_entry(0), m_source(LootSourceType(0)) {}
    ~Loot() { clear(); }

    // For deleting items at loot removal since there is no backward interface to the Item()
    void DeleteLootItemFromContainerItemDB(uint32 itemID);
    void DeleteLootMoneyFromContainerItemDB();

    // if loot becomes invalid this reference is used to inform the listener
    void addLootValidatorRef(LootValidatorRef* pLootValidatorRef)
    {
        i_LootValidatorRefManager.insertFirst(pLootValidatorRef);
    }

    void clear()
    {
        for (QuestItemMap::iterator itr = PlayerQuestItems.begin(); itr != PlayerQuestItems.end(); ++itr)
            delete itr->second;
        PlayerQuestItems.clear();

        for (QuestItemMap::iterator itr = PlayerFFAItems.begin(); itr != PlayerFFAItems.end(); ++itr)
            delete itr->second;
        PlayerFFAItems.clear();

        PlayersLooting.clear();
        items.clear();
        quest_items.clear();
        unique_items.clear();
        gold = 0;
        unlootedCount = 0;
        looterGUID = 0;
        i_LootValidatorRefManager.clearReferences();

        item_takers_hashes.clear();
    }

    void AddItemTakerHash(uint32 item, std::string hash) { item_takers_hashes[item].insert(hash); };
    bool ItemTakersHashIPMatch(uint32 item, const std::string& hash) 
    {
        const auto& itemTakersHashes = item_takers_hashes[item];
        return itemTakersHashes.find(hash) != itemTakersHashes.end();
    }

    bool empty() const { return items.empty() && gold == 0; }
    bool isLooted() const { return gold == 0 && unlootedCount == 0; }

    void NotifyItemRemoved(uint8 lootIndex);
    void NotifyQuestItemRemoved(uint8 questIndex);
    void NotifyMoneyRemoved();
    void ReleaseAll();
    void AddLooter(uint64 GUID) { PlayersLooting.insert(GUID); }
    void RemoveLooter(uint64 GUID) { PlayersLooting.erase(GUID); }
    bool HasLooters() { return !PlayersLooting.empty(); }

    void generateMoneyLoot(uint32 minAmount, uint32 maxAmount);
    void FillLoot(uint32 loot_id, LootStore const& store, Player* loot_owner, bool personal, uint32 from_entry = 0, bool save_to_db = true);

    void LogEliteLoot(Player *owner);
    void saveLootToDB(Player *owner);
    void RemoveSavedLootFromDB();

    bool IsPlayerAllowedToLoot(Player *player, WorldObject *object);

    // Inserts the item into the loot (called by LootTemplate processors)
    void AddItem(LootStoreItem const & item);

    void setItemLooted(LootItem *pLootItem, Player* looter);
    void removeItemFromSavedLoot(LootItem *pLootItem);

    void setCreatureGUID(Creature *pCreature);

    void FillLootFromDB(Creature *pCreature, Player* pLootOwner);
    bool LootLoadedFromDB() { return m_lootLoadedFromDB; }

    LootItem* LootItemInSlot(uint32 lootslot, Player* player, QuestItem** qitem = NULL, QuestItem** ffaitem = NULL);

    LootItem* LootItemInSlot(uint32 lootslot);

    uint32 GetMaxSlotInLootFor(Player* player) const;

    void RemoveQuestLoot(Player* player);

    void LogLooted(LootItem *pLootItem, Player* looter, bool PartyLoot);

	LootSourceType m_source;
	uint32 m_from_entry;
	MapID m_mapID;
	uint64 m_bossGUID;
	uint64 m_creatureGUID;
    uint32 cast_item_entry;  // opened with cast item like key->chest

    private:
        void RemoveSavedLootFromDB(Creature *pCreature);

        void FillNotNormalLootFor(Player* player);
        QuestItemList* FillFFALoot(Player* player);
        QuestItemList* FillQuestLoot(Player* player);
        void FillNonQuestNonFFAConditionalLoot(Player* player); // count conditionals in fact

        std::set<uint64> PlayersLooting;
        QuestItemMap PlayerQuestItems;
        QuestItemMap PlayerFFAItems;

        bool m_lootLoadedFromDB;

        // All rolls are registered here. They need to know, when the loot is not valid anymore
        LootValidatorRefManager i_LootValidatorRefManager;
};

struct LootView
{
    Loot &loot;
    Player *viewer;
    PermissionTypes permission;
    LootView(Loot &_loot, Player *_viewer,PermissionTypes _permission = ALL_PERMISSION)
        : loot(_loot), viewer(_viewer), permission(_permission) {}
};

extern LootStore LootTemplates_Creature;
extern LootStore LootTemplates_Fishing;
extern LootStore LootTemplates_Gameobject;
extern LootStore LootTemplates_Item;
extern LootStore LootTemplates_Pickpocketing;
extern LootStore LootTemplates_Skinning;
extern LootStore LootTemplates_Disenchant;
extern LootStore LootTemplates_Prospecting;
extern LootStore LootTemplates_QuestMail;

void LoadLootTemplates_Creature();
void LoadLootTemplates_Fishing();
void LoadLootTemplates_Gameobject();
void LoadLootTemplates_Item();
void LoadLootTemplates_Pickpocketing();
void LoadLootTemplates_Skinning();
void LoadLootTemplates_Disenchant();
void LoadLootTemplates_Prospecting();
void LoadLootTemplates_QuestMail();
void LoadLootTemplates_Reference();

inline void LoadLootTables()
{
    LoadLootTemplates_Creature();
    LoadLootTemplates_Fishing();
    LoadLootTemplates_Gameobject();
    LoadLootTemplates_Item();
    LoadLootTemplates_Pickpocketing();
    LoadLootTemplates_Skinning();
    LoadLootTemplates_Disenchant();
    LoadLootTemplates_Prospecting();
    LoadLootTemplates_QuestMail();
    LoadLootTemplates_Reference();
}

#endif

