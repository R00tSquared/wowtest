#ifndef HELLGROUND_TRANSMOGRIFICATION_H
#define HELLGROUND_TRANSMOGRIFICATION_H

#include "Player.h"

#define QUESTION_MARK_ITEM 693035
#define SELECT_ORIGINAL_ITEM 693036
#define GO_BACK_ITEM 693037
#define MAX_TRANSMOG_MODELS 76
#define MAX_TRANSMOG_LIST_SIZE 84 // 85 items
                                  // 4 slots for active/orig/2_empty
                                  // 10 slots for GoBack
                                  // 1 slot for item delete when reached max trans size

const uint8 ItemTransmogsMainSubClasses[10] = {0,1,4,5,6,7,8,10,13,15};
const uint8 ItemTransmogsOffSubClasses[7] = {0,4,7,13,15,6,0};
const uint8 ItemTransmogsRangedSubClasses[5] = {16,19,2,3,18};
const uint8 MaxTypesForSlot[3] = {10, 7, 5}; // 10 7 5
const uint8 MaxTypesToDisplay[3] = {2, 3, 2}; // 10 7 5
const uint8 TypesToDisplay[3][3] = 
{
    {5, 6, 0},
    {2, 5, 6},
    {3, 1, 0}
}; // 10 7 5

struct TransmogrificationShownList
{
    bool weapon;
    uint8 slot;
    uint8 type;
};

struct TransmogrificationStruct
{
    uint32 displayid;
    uint32 itemid;
};

struct TransmogrificationStructWeapon
{
    bool active;
    uint8 itemClass;
    uint8 subclass;
    uint32 itemId;
};

const uint8 TransmogrificationSlots[11] = {
        0,                    //EQUIPMENT_SLOT_HEAD,     
        2,                    //EQUIPMENT_SLOT_SHOULDERS,
        3,                    //EQUIPMENT_SLOT_BODY,     
        4,                    //EQUIPMENT_SLOT_CHEST,    
        5,                    //EQUIPMENT_SLOT_WAIST,    
        6,                    //EQUIPMENT_SLOT_LEGS,     
        7,                    //EQUIPMENT_SLOT_FEET,     
        8,                    //EQUIPMENT_SLOT_WRISTS,   
        9,                    //EQUIPMENT_SLOT_HANDS,    
        14,                    //EQUIPMENT_SLOT_BACK,     
        18                    //EQUIPMENT_SLOT_TABARD    
    };

class HELLGROUND_IMPORT_EXPORT Transmogrification
{
    public:

        explicit Transmogrification(Player* player);
        ~Transmogrification();

        void SendTransmogrification(WorldSession* session, Player* receiver); // used to send transmog with MIRROR_AURA

        static bool IsAcceptableTransmogrModel(uint32 model); // used in model system
        static bool IsRaceShirtModel(uint32 model); // used in model system

        void LoadTransmog(); // used when loading player - then load transmog

        bool Add(bool weapon, uint8 slot, uint32 itemid);
        bool Remove(bool weapon, uint8 slot, uint32 itemid);
        bool Select(bool Weapon, uint8 slot, uint8 type, uint32 itemId);

        bool RemoveTransmogItemid(uint32 itemid);

        void SendTransmogList(bool weapon, uint8 slot, uint8 type); // To select active, remove active
        //void SendTransmogDeleteList(bool weapon, uint8 slot); // To delete items from list: goes in like Item - Delete, Item - Delete. in vendor list.

        uint32 GetActiveTransEntry(uint8 slot, uint8 type, bool Weapon); // to get active entry

        bool SatisfySlotRequirements(ItemPrototype const* itemProto, uint8 slot, uint32 & reasonStringEntry, bool Weapon); // can we add this to this slot?

        bool HasActiveTransmogrification()
        {
            if (ActiveTransmogrificationListsArray == nullptr)
                return false;
            for (uint8 i = 0; i < 11; ++i)
                if (ActiveTransmogrificationListsArray[i].itemid)
                    return true;
            return false;
        }

        void HandleAddTransmogPacket(uint64 itemguid);
        void HandleSelectTransmogPacket(uint32 itemid);
        void GoToMainMenu();

        void SetItemGUID(uint64 itemguid){ _itemGUID = itemguid;};
        uint64 GetItemGUID(){ return _itemGUID;};

        uint8 GetWeaponTypeFromSubclass(uint8 subclass, uint8 slot, uint8 Class);
        uint32 GetActiveTransmogInSlot(uint8 slot);
        int32 TransmogSlotToInventorySlot(uint8 slot);

        TransmogrificationStruct* GetActiveTransmogrStructure()
        {
            return ActiveTransmogrificationListsArray;
        }

    private:

        void CreateActiveTransmogStructureIfNeeded()
        {
            if (ActiveTransmogrificationListsArray == nullptr)
            {
                ActiveTransmogrificationListsArray = new TransmogrificationStruct[11];
                TransmogrificationStruct transEmpty = {0, 0};
                for (uint8 cnt = 0; cnt < 11; ++cnt)
                    ActiveTransmogrificationListsArray[cnt] = transEmpty;
            }
        }

        void GetPossibleTransmigrificationsItr(uint8 slot, std::list<uint32>::iterator &begin, std::list<uint32>::iterator &end)
        {
            begin = TransmogrificationListsArray[slot].begin();
            end = TransmogrificationListsArray[slot].end();
        }

        void GetPossibleTransmigrificationsWeaponItr(uint8 slot, std::list<TransmogrificationStructWeapon>::iterator &begin, std::list<TransmogrificationStructWeapon>::iterator &end)
        {
            begin = TransmogrificationListsArrayWeapon[slot].begin();
            end = TransmogrificationListsArrayWeapon[slot].end();
        }

        uint8 GetTransmogrificationWeaponSize(uint8 slot)
        {
            return TransmogrificationListsArrayWeapon[slot].size();
        }

        uint8 GetTransmogrificationSize(uint8 slot)
        {
            return TransmogrificationListsArray[slot].size();
        }

        void TransmogrificationListsPushFront(uint8 slot, uint32 ItemId)
        {
            TransmogrificationListsArray[slot].push_front(ItemId);
        }

        void TransmogrificationListsWeaponPushFront(uint8 slot, TransmogrificationStructWeapon newstruct)
        {
            TransmogrificationListsArrayWeapon[slot].push_front(newstruct);
        }

        void TransmogrificationListsWeaponPushBack(uint8 slot, TransmogrificationStructWeapon newstruct)
        {
            TransmogrificationListsArrayWeapon[slot].push_back(newstruct);
        }

        void TransmogrificationListsErase(uint8 slot, std::list<uint32>::iterator itr)
        {
            TransmogrificationListsArray[slot].erase(itr);
        }

        void TransmogrificationListsWeaponErase(uint8 slot, std::list<TransmogrificationStructWeapon>::iterator itr)
        {
            TransmogrificationListsArrayWeapon[slot].erase(itr);
        }

        uint8 CouldEquipItem(uint8 slot, ItemPrototype const* pProto) const;

        //////////////////////////////////////////////////

        TransmogrificationStruct* ActiveTransmogrificationListsArray; // pointer exists only if there is ANY transmogr on character. displayId, ItemId
        std::list<uint32> TransmogrificationListsArray[11]; // only ItemId
        std::list<TransmogrificationStructWeapon> TransmogrificationListsArrayWeapon[3]; // bool active, uint8 subclass, uint32 itemId
        Player* _owner;
        uint64 _itemGUID;
        TransmogrificationShownList m_shownList;
        uint16 _sortId;
};
#endif