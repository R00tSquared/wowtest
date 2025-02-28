// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Transmogrification.h"
#include "Language.h"
#include "Spell.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"

Transmogrification::Transmogrification(Player* player)  : _owner(player)
{
    ActiveTransmogrificationListsArray = nullptr;
    m_shownList.weapon = false;
    m_shownList.slot = 0;
    m_shownList.type = 0;
    _itemGUID = 0;
}

Transmogrification::~Transmogrification()
{
    for (uint8 slot = 0; slot < 11; ++slot)
        TransmogrificationListsArray[slot].clear();
    for (uint8 slot = 0; slot < 3; ++slot)
        TransmogrificationListsArrayWeapon[slot].clear();
    if (ActiveTransmogrificationListsArray != nullptr)
        delete [] ActiveTransmogrificationListsArray;
}

uint32 Transmogrification::GetActiveTransmogInSlot(uint8 slot)
{
    // don't store weapon models!!!
    
    switch (slot)
    {
    case 0:
        slot = 0;
        break;
    case 2:
        slot = 1;
        break;
    case 3:
        slot = 2;
        break;
    case 4:
        slot = 3;
        break;
    case 5:
        slot = 4;
        break;
    case 6:
        slot = 5;
        break;
    case 7:
        slot = 6;
        break;
    case 8:
        slot = 7;
        break;
    case 9:
        slot = 8;
        break;
    case 14:
        slot = 9;
        break;
    case 18:
        slot = 10;
        break;
    default:
        return 0;
    }

    if (ActiveTransmogrificationListsArray == nullptr || !ActiveTransmogrificationListsArray[slot].itemid)
        return 0;

    return ActiveTransmogrificationListsArray[slot].itemid;
}

int32 Transmogrification::TransmogSlotToInventorySlot(uint8 slot)
{
    switch (slot)
    {
    case 0:
        slot = 0;
        break;
    case 1:
        slot = 2;
        break;
    case 2:
        slot = 3;
        break;
    case 3:
        slot = 4;
        break;
    case 4:
        slot = 5;
        break;
    case 5:
        slot = 6;
        break;
    case 6:
        slot = 7;
        break;
    case 7:
        slot = 8;
        break;
    case 8:
        slot = 9;
        break;
    case 9:
        slot = 14;
        break;
    case 10:
        slot = 18;
        break;
    default:
        return -1;
    }

    return slot;
}

void Transmogrification::SendTransmogrification(WorldSession* session, Player* receiver)
{
    WorldPacket data(SMSG_MIRRORIMAGE_DATA, 68);

    data << _owner->GetGUID();

    uint32 model = _owner->GetDisplayId();

    if (!_owner->HasAura(55119))
    {
        sLog.outLog(LOG_DEFAULT, "Transmogrification error - no aura 55119 but still had flag_clone");
        model = 16358; // pink pig
    }

    data << model;
    //sLog.outString("model: %u", model);

    // race shirt
    if (_owner->HasAura(55401))
    {
        std::pair<uint8, uint8> morphval = _owner->GetMorphShirtRaceGender(_owner);
        data << morphval.first;
        data << morphval.second;
    }
    else
    {
        switch (model)
        {
            //RACE_HUMAN          = 1,
        case 49: // male
        {
            data << (uint8)RACE_HUMAN;
            data << (uint8)0;
            break;
        }
        case 50: // female
        {
            data << (uint8)RACE_HUMAN;
            data << (uint8)1;
            break;
        }
        //RACE_ORC            = 2,
        case 51: // male
        {
            data << (uint8)RACE_ORC;
            data << (uint8)0;
            break;
        }
        case 52: // female
        {
            data << (uint8)RACE_ORC;
            data << (uint8)1;
            break;
        }
        //RACE_DWARF          = 3,
        case 53: // male
        {
            data << (uint8)RACE_DWARF;
            data << (uint8)0;
            break;
        }
        case 54: // female
        {
            data << (uint8)RACE_DWARF;
            data << (uint8)1;
            break;
        }
        //RACE_NIGHTELF       = 4,
        case 55: // male
        {
            data << (uint8)RACE_NIGHTELF;
            data << (uint8)0;
            break;
        }
        case 56: // female
        {
            data << (uint8)RACE_NIGHTELF;
            data << (uint8)1;
            break;
        }
        //RACE_UNDEAD_PLAYER  = 5,
        case 57: // male
        {
            data << (uint8)RACE_UNDEAD_PLAYER;
            data << (uint8)0;
            break;
        }
        case 58: // female
        {
            data << (uint8)RACE_UNDEAD_PLAYER;
            data << (uint8)1;
            break;
        }
        //RACE_TAUREN         = 6,
        case 59: // male
        {
            data << (uint8)RACE_TAUREN;
            data << (uint8)0;
            break;
        }
        case 60: // female
        {
            data << (uint8)RACE_TAUREN;
            data << (uint8)1;
            break;
        }
        //RACE_GNOME          = 7,
        case 1563: // male
        {
            data << (uint8)RACE_GNOME;
            data << (uint8)0;
            break;
        }
        case 1564: // female
        {
            data << (uint8)RACE_GNOME;
            data << (uint8)1;
            break;
        }
        //RACE_TROLL          = 8,
        case 1478: // male
        {
            data << (uint8)RACE_TROLL;
            data << (uint8)0;
            break;
        }
        case 1479: // female
        {
            data << (uint8)RACE_TROLL;
            data << (uint8)1;
            break;
        }
        case 21963: // male forest troll (big troll)
        case 21964: // male forest troll (big troll)
        {
            data << (uint8)RACE_TROLL;
            data << (uint8)0;
            break;
        }
        //RACE_GOBLIN         = 9,
        // done with truemorph
        //RACE_BLOODELF       = 10,
        case 15476: // male
        {
            data << (uint8)RACE_BLOODELF;
            data << (uint8)0;
            break;
        }
        case 15475: // female
        {
            data << (uint8)RACE_BLOODELF;
            data << (uint8)1;
            break;
        }
        //RACE_DRAENEI        = 11,
        case 16125: // male
        {
            data << (uint8)RACE_DRAENEI;
            data << (uint8)0;
            break;
        }
        case 16126: // female
        {
            data << (uint8)RACE_DRAENEI;
            data << (uint8)1;
            break;
        }
        //RACE_FEL_ORC        = 12,
        // done with normal model
        //RACE_NAGA           = 13,
        case 17402: // male
        {
            data << (uint8)RACE_NAGA;
            data << (uint8)0;
            break;
        }
        case 17403: // female
        {
            data << (uint8)RACE_NAGA;
            data << (uint8)1;
            break;
        }
        //RACE_BROKEN         = 14,
        // done with normal model
        //RACE_SKELETON       = 15,
        case 17578: // male
        {
            data << (uint8)RACE_SKELETON;
            data << (uint8)0;
            break;
        }
        default:
        {
            data << (uint8)_owner->GetRace();
            data << (uint8)_owner->GetGender();
            break;
        }
        }
    }

    // *data << (uint8)playerTarget->GetClass();
    
    // skin, face, hair, haircolor
    data << (uint8)_owner->GetByteValue(PLAYER_BYTES, 0);
    data << (uint8)_owner->GetByteValue(PLAYER_BYTES, 1);
    data << (uint8)_owner->GetByteValue(PLAYER_BYTES, 2);
    data << (uint8)_owner->GetByteValue(PLAYER_BYTES, 3);

    // facial hair
    data << (uint8)_owner->GetByteValue(PLAYER_BYTES_2, 0);
    
    // guild id
    data << (uint32)_owner->GetGuildId();

    bool sendreal = _owner->InArena() && !_owner->IsInSameRaidWith(receiver);
    for (uint8 i = 0; i < 11; ++i)
    {
        if ((i == 0 && (_owner->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM) || model == 17578 || model == 21963 || model == 21964)) || (i == 9 && _owner->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK)) || model == 17403 && i == 1)
        {
            data << 0;
            continue;
        }
        uint32 DisplayOriginal = _owner->GetItemDisplayIdInSlot(INVENTORY_SLOT_BAG_0, TransmogrificationSlots[i]);
        data << uint32((ActiveTransmogrificationListsArray == nullptr || !ActiveTransmogrificationListsArray[i].displayid || !DisplayOriginal || sendreal) ? DisplayOriginal : ActiveTransmogrificationListsArray[i].displayid);
        //sLog.outLog(LOG_DEFAULT, "Added display %u", uint32((ActiveTransmogrificationListsArray == nullptr || !ActiveTransmogrificationListsArray[i].displayid || !DisplayOriginal) ? DisplayOriginal : ActiveTransmogrificationListsArray[i].displayid));
    }
    session->SendPacket(&data);
}

bool Transmogrification::IsAcceptableTransmogrModel(uint32 model)
{
    switch (model)
    {
        // all NOT RACE MORPHS
        case 856: // polymorph sheep, mob id 16372, 1933
        case 857: // polymorph sheep, mob id 16372, 1933
        case 11634: // polymorph lasher, mob id 6509
        case 10045: // mob id 3681 (wisp)
        case 16361: // turtle pig
        case 16358: // turtle pig
            return true;
        default:
            return false;
    }
}

bool Transmogrification::IsRaceShirtModel(uint32 model)
{
    switch (model)
    {
        // FUN realm shirts
        case 49: // male
        case 50: // female
        case 51: // male
        case 52: // female
        case 53: // male
        case 54: // female
        case 55: // male
        case 56: // female
        case 57: // male
        case 58: // female
        case 59: // male
        case 60: // female
        case 1563: // male
        case 1564: // female
        case 1478: // male
        case 1479: // female
        case 21963: // male forest troll (big troll)
        case 21964: // male forest troll (big troll)
        case 15476: // male
        case 15475: // female
        case 16125: // male
        case 16126: // female
        case 17402: // male
        case 17403: // female
        case 17578: // male
            return true;
        default:
            return false;
    }
}

void Transmogrification::LoadTransmog()
{
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT active, itemId FROM character_transmogrification WHERE guid='%u'", _owner->GetGUIDLow());

    bool HaveActiveTransmogr = false;
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            bool active = fields[0].GetBool();
            int32 signedItemId = fields[1].GetInt32();

            bool OneHandedOffHand = signedItemId < 0;
            uint32 itemId = OneHandedOffHand ? abs(signedItemId) : signedItemId;

            ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype(itemId);
            if (!itemProto || !itemProto->DisplayInfoID)
                continue;

            switch (itemProto->InventoryType)
            {
                case INVTYPE_WEAPON:
                case INVTYPE_SHIELD:
                case INVTYPE_RANGED:
                case INVTYPE_2HWEAPON:
                case INVTYPE_WEAPONMAINHAND:
                case INVTYPE_WEAPONOFFHAND:
                case INVTYPE_HOLDABLE:
                case INVTYPE_THROWN:
                case INVTYPE_RANGEDRIGHT:
                {
                    uint8 slot1 = 10;
                    switch (itemProto->InventoryType)
                    {
                        case INVTYPE_WEAPON:
                        {
                            if (OneHandedOffHand)
                                slot1 = 1;
                            else
                                slot1 = 0;
                            break;
                        }
                        case INVTYPE_2HWEAPON:
                        case INVTYPE_WEAPONMAINHAND:
                            slot1 = 0;
                            break;
                        case INVTYPE_SHIELD:
                        case INVTYPE_WEAPONOFFHAND:
                        case INVTYPE_HOLDABLE:
                            slot1 = 1;
                            break;
                        case INVTYPE_THROWN:
                        case INVTYPE_RANGED:
                        case INVTYPE_RANGEDRIGHT:
                            slot1 = 2;
                            break;
                    }

                    TransmogrificationStructWeapon trans = {active, uint8(itemProto->Class), uint8(itemProto->SubClass), itemId};

                    if (active)
                    {
                        if (slot1 != 10)
                        {
                            TransmogrificationListsArrayWeapon[slot1].push_front(trans);
                            _owner->SetVisibleItemSlot(EQUIPMENT_SLOT_MAINHAND+slot1, _owner->GetItemByPos(255, EQUIPMENT_SLOT_MAINHAND+slot1));
                        }
                    }
                    else
                    {
                        if (slot1 != 10)
                            TransmogrificationListsArrayWeapon[slot1].push_back(trans);
                    }
                    break;
                }
                default:
                {

                    uint32 slot = 0;
                    switch (itemProto->InventoryType)
                    {
                        case INVTYPE_HEAD:
                            slot = 0;
                            break;
                        case INVTYPE_SHOULDERS:
                            slot = 1;
                            break;
                        case INVTYPE_BODY:
                            slot = 2;
                            break;
                        case INVTYPE_CHEST:
                        case INVTYPE_ROBE:
                            slot = 3;
                            break;
                        case INVTYPE_WAIST:
                            slot = 4;
                            break;
                        case INVTYPE_LEGS:
                            slot = 5;
                            break;
                        case INVTYPE_FEET:
                            slot = 6;
                            break;
                        case INVTYPE_WRISTS:
                            slot = 7;
                            break;
                        case INVTYPE_HANDS:
                            slot = 8;
                            break;
                        case INVTYPE_CLOAK:
                            slot = 9;
                            break;
                        case INVTYPE_TABARD:
                            slot = 10;
                            break;
                    }

                    if (active)
                    {
                        CreateActiveTransmogStructureIfNeeded();
                        TransmogrificationStruct trans = {itemProto->DisplayInfoID, itemId};
                        ActiveTransmogrificationListsArray[slot] = trans;
                        HaveActiveTransmogr = true;
                    }
                    else
                        TransmogrificationListsPushFront(slot, itemId);
                }
            }
        }
        while (result->NextRow());
    }

    if (HaveActiveTransmogr)
        _owner->CastSpell(_owner, 55118, true);
    else if (_owner->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_CLONED) && _owner->GetAurasByType(SPELL_AURA_MIRROR_IMAGE).empty())
    {
        _owner->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_CLONED);
        _owner->CastSpell(_owner, 55118, true);
    }
}

bool Transmogrification::Add(bool weapon, uint8 slot, uint32 itemid)
{
    if (ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemid))
    {
        if (weapon)
        {
            TransmogrificationStructWeapon newStruct = {false, uint8(pProto->Class), uint8(pProto->SubClass), itemid};
            if (slot == 1)
                RealmDataDatabase.PExecute("INSERT INTO character_transmogrification VALUES (%u, 0, -%u)", _owner->GetGUIDLow(), itemid);
            else
                RealmDataDatabase.PExecute("INSERT INTO character_transmogrification VALUES (%u, 0, %u)", _owner->GetGUIDLow(), itemid);
            TransmogrificationListsWeaponPushBack(slot, newStruct);
        }
        else
        {
            CreateActiveTransmogStructureIfNeeded();
            RealmDataDatabase.PExecute("INSERT INTO character_transmogrification VALUES ('%u', '0', '%u')", _owner->GetGUIDLow(), itemid);
            TransmogrificationListsPushFront(slot, itemid);
        }
        SendTransmogList(m_shownList.weapon, m_shownList.slot, m_shownList.type);
        return true;
    }
    SendTransmogList(m_shownList.weapon, m_shownList.slot, m_shownList.type);
    return false;
}

bool Transmogrification::Remove(bool weapon, uint8 slot, uint32 itemid)
{
    bool result = false;
    if (weapon)
    {
        if (slot == 1)
            RealmDataDatabase.PExecute("DELETE FROM character_transmogrification WHERE guid=%u AND itemId=-%u", _owner->GetGUIDLow(), itemid);
        else
            RealmDataDatabase.PExecute("DELETE FROM character_transmogrification WHERE guid=%u AND itemId=%u", _owner->GetGUIDLow(), itemid);

        std::list<TransmogrificationStructWeapon>::iterator itr;
        std::list<TransmogrificationStructWeapon>::iterator end;
        GetPossibleTransmigrificationsWeaponItr(slot, itr, end);
        
        bool update = false;
        for (; itr!= end; ++itr)
        {
            if (itr->itemId != itemid)
                continue;

            result = true;
            update = itr->active;
            TransmogrificationListsWeaponErase(slot, itr);
            break;
        }
        if (update)
            _owner->SetVisibleItemSlot(EQUIPMENT_SLOT_MAINHAND+slot,_owner->GetItemByPos(255, EQUIPMENT_SLOT_MAINHAND+slot));
    }
    else
    {
        RealmDataDatabase.PExecute("DELETE FROM character_transmogrification WHERE guid = '%u' AND itemId = '%u'", _owner->GetGUIDLow(), itemid);
        if (GetActiveTransmogrStructure() != nullptr && ((GetActiveTransmogrStructure())[slot]).itemid == itemid)
        {
            result = true;
            ((GetActiveTransmogrStructure())[slot]).itemid = 0;
            ((GetActiveTransmogrStructure())[slot]).displayid = 0;
            _owner->CastSpell(_owner, 55118, true);
        }
        else
        {
            // all possible transmogs
            std::list<uint32>::iterator itr;
            std::list<uint32>::iterator end;
            GetPossibleTransmigrificationsItr(slot, itr, end);
            for (; itr != end; ++itr)
            {
                if (*itr != itemid)
                    continue;

                result = true;
                TransmogrificationListsErase(slot, itr);
                break;
            }
        }
    }
    SendTransmogList(m_shownList.weapon, m_shownList.slot, m_shownList.type);
    return result;
}

bool Transmogrification::RemoveTransmogItemid(uint32 itemid)
{   
    RealmDataDatabase.PExecute("DELETE from character_transmogrification WHERE guid = %u AND (itemId = %u or itemId = -%u)", _owner->GetGUIDLow(), itemid, itemid);

    std::list<TransmogrificationStructWeapon>::iterator itr1;
    std::list<TransmogrificationStructWeapon>::iterator end1;

    for (auto i = 0; i < 3; ++i) {
        if (TransmogrificationListsArrayWeapon[i].size() == 0)
            continue;

        GetPossibleTransmigrificationsWeaponItr(i, itr1, end1);

        for (; itr1 != end1; ++itr1)
        {
            if (itr1->itemId != itemid)
                continue;

            TransmogrificationListsArrayWeapon[i].erase(itr1);
            break;
        }
    }

    std::list<uint32>::iterator itr2;
    std::list<uint32>::iterator end2;
    for (auto i = 0; i < 11; ++i) {
        if (TransmogrificationListsArray[i].size() == 0)
            continue;

        GetPossibleTransmigrificationsItr(i, itr2, end2);

        for (; itr2 != end2; ++itr2)
        {
            if (*itr2 != itemid)
                continue;

            TransmogrificationListsArray[i].erase(itr2);
            break;
        }
    }

    return true;
}

bool Transmogrification::Select(bool Weapon, uint8 slot, uint8 type, uint32 itemId)
{
    bool result = false;
    if (Weapon)
    {
        std::list<TransmogrificationStructWeapon>::iterator itr;
        std::list<TransmogrificationStructWeapon>::iterator itrToDelete;
        std::list<TransmogrificationStructWeapon>::iterator itrToDelete2;
        std::list<TransmogrificationStructWeapon>::iterator end;
        GetPossibleTransmigrificationsWeaponItr(slot, itr, end);
        TransmogrificationStructWeapon newStruct = {true,0,0,0};
        TransmogrificationStructWeapon oldStruct = {false,0,0,0};
        for (; itr!= end; ++itr)
        {
            if (itr->itemId == itemId)
            {
                newStruct.itemClass = itr->itemClass;
                newStruct.subclass = itr->subclass;
                newStruct.itemId = itr->itemId;
                itrToDelete = itr;
            }
            else
            {
                switch (slot)
                {
                case 0:
                    if (itr->active && itr->itemClass == 2 && (((ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_AXE ||  
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_MACE || 
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_SWORD || 
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_FIST || 
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_DAGGER) && (itr->subclass == ITEM_SUBCLASS_WEAPON_AXE || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_MACE || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_SWORD || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_FIST || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_DAGGER)) || 
                        ((ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_AXE2 || 
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_MACE2 || 
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_POLEARM || 
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                        ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_STAFF) && (itr->subclass == ITEM_SUBCLASS_WEAPON_AXE2 || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_MACE2 || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_POLEARM || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                        itr->subclass == ITEM_SUBCLASS_WEAPON_STAFF))))
                        {
                            oldStruct.itemClass = itr->itemClass;
                            oldStruct.subclass = itr->subclass;
                            oldStruct.itemId = itr->itemId;
                            itrToDelete2 = itr;
                        }
                    break;
                case 1:
                    if (itr->active && ((itr->itemClass == 2 && type < 5) || ((itr->itemClass == 4 && type > 4) && itr->subclass == ItemTransmogsOffSubClasses[type])))
                    {
                        oldStruct.itemClass = itr->itemClass;
                        oldStruct.subclass = itr->subclass;
                        oldStruct.itemId = itr->itemId;
                        itrToDelete2 = itr;
                    }
                    break;
                case 2:
                    {
                        if (itr->itemClass == 2)
                        {
                            if (itr->active && (((ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_BOW ||
                                ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_GUN ||
                                ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_CROSSBOW) && (itr->subclass == ITEM_SUBCLASS_WEAPON_BOW ||
                                itr->subclass == ITEM_SUBCLASS_WEAPON_GUN ||
                                itr->subclass == ITEM_SUBCLASS_WEAPON_CROSSBOW)) || ((ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_THROWN || ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_WAND) && 
                                (itr->subclass == ITEM_SUBCLASS_WEAPON_THROWN || itr->subclass == ITEM_SUBCLASS_WEAPON_WAND))))
                            {
                                {
                                    oldStruct.itemClass = itr->itemClass;
                                    oldStruct.subclass = itr->subclass;
                                    oldStruct.itemId = itr->itemId;
                                    itrToDelete2 = itr;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
        if (newStruct.itemId)
        {
            TransmogrificationListsWeaponErase(slot, itrToDelete);
            if (slot == 1)
                RealmDataDatabase.PExecute("UPDATE character_transmogrification SET active = 1 WHERE guid=%u AND itemId=-%u", _owner->GetGUIDLow(), newStruct.itemId);
            else 
                RealmDataDatabase.PExecute("UPDATE character_transmogrification SET active = 1 WHERE guid=%u AND itemId=%u", _owner->GetGUIDLow(), newStruct.itemId);
            TransmogrificationListsWeaponPushFront(slot, newStruct);
        }

        if (oldStruct.itemId)
        {
            TransmogrificationListsWeaponErase(slot, itrToDelete2);
            if (slot == 1)
                RealmDataDatabase.PExecute("UPDATE character_transmogrification SET active = 0 WHERE guid=%u AND itemId=-%u", _owner->GetGUIDLow(), oldStruct.itemId);
            else
                RealmDataDatabase.PExecute("UPDATE character_transmogrification SET active = 0 WHERE guid=%u AND itemId=%u", _owner->GetGUIDLow(), oldStruct.itemId);
            TransmogrificationListsWeaponPushBack(slot, oldStruct);
        }
        _owner->SetVisibleItemSlot(EQUIPMENT_SLOT_MAINHAND+slot,_owner->GetItemByPos(255, EQUIPMENT_SLOT_MAINHAND+slot));
    }
    else
    {
        CreateActiveTransmogStructureIfNeeded();
        if (itemId)
            RealmDataDatabase.PExecute("UPDATE character_transmogrification SET active = 1 WHERE guid = '%u' AND itemId = '%u'", _owner->GetGUIDLow(), itemId);
        if (((GetActiveTransmogrStructure())[slot]).itemid == itemId)
        {
            return false; // just do nothing. it is already active
        }
        else
        {
            if (itemId)
            {
                // all possible transmogs
                std::list<uint32>::iterator itr;
                std::list<uint32>::iterator end;
                GetPossibleTransmigrificationsItr(slot, itr, end);
                for (; itr != end; ++itr)
                {
                    if (*itr == itemId)
                        break;
                }
                if (itr == end) // error, this shouldn't happen
                    return false;
                bool swap = ((GetActiveTransmogrStructure())[slot]).itemid;
                if (swap)// != 0
                    // here we swap active and possible transmogrs
                    std::swap(((GetActiveTransmogrStructure())[slot]).itemid, *itr);
                else
                    ((GetActiveTransmogrStructure())[slot]).itemid = *itr;

                ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype((((GetActiveTransmogrStructure())[slot]).itemid));
                if (!itemProto || !itemProto->DisplayInfoID)
                {
                    ((GetActiveTransmogrStructure())[slot]).displayid = 0;
                    return false;
                }
                ((GetActiveTransmogrStructure())[slot]).displayid = itemProto->DisplayInfoID;
                if (swap)
                    RealmDataDatabase.PExecute("UPDATE character_transmogrification SET active = 0 WHERE guid = '%u' AND itemId = '%u'", _owner->GetGUIDLow(), *itr);
                else
                    TransmogrificationListsErase(slot, itr);
            }
            else // just resetting to original
            {
                if (uint32 HadEntry = ((GetActiveTransmogrStructure())[slot]).itemid)
                {
                    RealmDataDatabase.PExecute("UPDATE character_transmogrification SET active = 0 WHERE guid = '%u' AND itemId = '%u'", _owner->GetGUIDLow(), HadEntry);
                    ((GetActiveTransmogrStructure())[slot]).itemid = 0;
                    ((GetActiveTransmogrStructure())[slot]).displayid = 0;
                    TransmogrificationListsPushFront(slot, HadEntry);
                }
            }
            _owner->CastSpell(_owner, 55118, true);
        }
    }
    SendTransmogList(m_shownList.weapon, m_shownList.slot, m_shownList.type);
    return result;
}

void Transmogrification::SendTransmogList(bool weapon, uint8 slot, uint8 type)
{
    m_shownList.weapon = weapon;
    m_shownList.slot = slot;
    m_shownList.type = type;

    ItemPrototype const* firstItemProto = NULL;
    ItemPrototype const* secondItemProto = NULL;
    ItemPrototype const* lastItemProto = ObjectMgr::GetItemPrototype(GO_BACK_ITEM);
    ItemPrototype const* itemProto = NULL;
    uint32 itemId = 0;
    uint8 numitems = 1; // active item or question mark is ALWAYS there
    ItemPrototype const* items[MAX_TRANSMOG_LIST_SIZE]; // 96 is max number of items + 1 active + 3 free slots = 10 pages
    for (uint8 i = 0; i < MAX_TRANSMOG_LIST_SIZE; i++)
        items[i] = NULL;

    // Active model
    if (uint32 activeTrans = GetActiveTransEntry(slot, type, weapon))
    {
        firstItemProto = ObjectMgr::GetItemPrototype(activeTrans); // != 0
        secondItemProto = ObjectMgr::GetItemPrototype(SELECT_ORIGINAL_ITEM);
        numitems++;
        if (!firstItemProto)
            firstItemProto = ObjectMgr::GetItemPrototype(QUESTION_MARK_ITEM);
    }
    else
        firstItemProto = ObjectMgr::GetItemPrototype(QUESTION_MARK_ITEM);

    if (weapon)
    {
        if (secondItemProto)
            numitems += 2; // 3 empty slots
        else
            numitems += 3;
        //[Lala] -> Активировать/Удалить
        //[Suka] -> Активировать/Удалить
        std::list<TransmogrificationStructWeapon>::iterator itr;
        std::list<TransmogrificationStructWeapon>::iterator end;
        GetPossibleTransmigrificationsWeaponItr(slot, itr, end);
            
        uint32 possibleEntry;
        for (; itr!= end; ++itr)
        {
            possibleEntry = 0;
            switch (slot)
            {
            case 0:
                if (!itr->active && itr->itemClass == 2 && (((ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_AXE ||  
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_MACE || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_SWORD || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_FIST || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_DAGGER) && (itr->subclass == ITEM_SUBCLASS_WEAPON_AXE || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_MACE || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_SWORD || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_FIST || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_DAGGER)) || 
                    ((ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_AXE2 || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_MACE2 || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_POLEARM || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_STAFF) && (itr->subclass == ITEM_SUBCLASS_WEAPON_AXE2 || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_MACE2 || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_POLEARM || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_STAFF))))
                        possibleEntry = itr->itemId;
                break;
            case 1:
                if (!itr->active && ((itr->itemClass == 2 && type < 5) || ((itr->itemClass == 4 && type > 4) && itr->subclass == ItemTransmogsOffSubClasses[type])))
                    possibleEntry = itr->itemId;
                break;
            case 2:
                {
                    if (itr->itemClass == 2)
                    {
                        if (!itr->active && (((ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_BOW ||
                            ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_GUN ||
                            ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_CROSSBOW) && (itr->subclass == ITEM_SUBCLASS_WEAPON_BOW ||
                            itr->subclass == ITEM_SUBCLASS_WEAPON_GUN ||
                            itr->subclass == ITEM_SUBCLASS_WEAPON_CROSSBOW)) || ((ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_THROWN || ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_WAND) && 
                            (itr->subclass == ITEM_SUBCLASS_WEAPON_THROWN || itr->subclass == ITEM_SUBCLASS_WEAPON_WAND))))
                        {
                            possibleEntry = itr->itemId;
                        }
                    }
                    break;
                }
            }

            if (possibleEntry)
            {
                itemProto = ObjectMgr::GetItemPrototype(possibleEntry);
                if (!itemProto)
                    possibleEntry = 0;
                            
            }
            if (possibleEntry)
            {
                items[numitems-4] = itemProto;
                numitems++;
            }
        }
    }
    else
    {
        if (GetTransmogrificationSize(slot))
        {
            //[Lala] -> Активировать/Удалить
            //[Suka] -> Активировать/Удалить
            if (secondItemProto)
                numitems += 2; // 3 empty slots
            else
                numitems += 3;
            std::list<uint32>::iterator itr;
            std::list<uint32>::iterator end;
            GetPossibleTransmigrificationsItr(slot, itr, end);
            for (; itr != end; ++itr)
            {
                itemId = *itr;
                if (itemId != 0)
                {
                    itemProto = ObjectMgr::GetItemPrototype(itemId);
                    if (!itemProto)
                        itemId = 0;
                }
                if (itemId != 0)
                {
                    items[numitems-4] = itemProto;
                    numitems++;
                }
            }
        }
    }
    
    //example numitems 16
    uint8 go_back_buttons = (numitems-1)/9 + 1; // 6 ? 10. 9 ? 10. 10 ? 2. 17 ? 2. 18 ? 2. 19 ? 3.
    
    WorldPacket data(SMSG_LIST_INVENTORY, (8+1+(go_back_buttons * 10)*8*4));

    data << uint64(_itemGUID);
    data << uint8(go_back_buttons * 10);

    data << uint32(1);
    data << uint32(firstItemProto->ItemId);
    data << uint32(firstItemProto->DisplayInfoID);
    data << uint32(0xFFFFFFFF);
    data << uint32(0);
    data << uint32(firstItemProto->MaxDurability);
    data << uint32(1); // buyCount - should never be more than 1
    data << uint32(0);

    if (secondItemProto)
    {
        data << uint32(1);
        data << uint32(secondItemProto->ItemId);
        data << uint32(secondItemProto->DisplayInfoID);
        data << uint32(0xFFFFFFFF);
        data << uint32(0);
        data << uint32(secondItemProto->MaxDurability);
        data << uint32(1); // buyCount - should never be more than 1
        data << uint32(0);
    }
    
    
    uint8 ia = 0;
    if (secondItemProto)
        ia++;
    for (; ia < 3; ia++)
    {
        data << uint32(ia+2);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0xFFFFFFFF);
        data << uint32(0);
        data << uint32(0);
        data << uint32(1); // buyCount - should never be more than 1
        data << uint32(0);
    }

    uint8 decrementer = 0;
    for (int i = 0; i < (go_back_buttons * 10); i++)
    {
        if (((i+5) % 10) == 0) // each page tenth item is go_back
        {
            data << uint32(i+5);
            data << uint32(lastItemProto->ItemId);
            data << uint32(lastItemProto->DisplayInfoID);
            data << uint32(0xFFFFFFFF);
            data << uint32(0);
            data << uint32(lastItemProto->MaxDurability);
            data << uint32(1); // buyCount - should never be more than 1
            data << uint32(0);
            decrementer++;
        }
        else if (items[i-decrementer])
        {
            data << uint32(i+5);
            data << uint32(items[i-decrementer]->ItemId);
            data << uint32(items[i-decrementer]->DisplayInfoID);
            data << uint32(0xFFFFFFFF);
            data << uint32(0);
            data << uint32(items[i-decrementer]->MaxDurability);
            data << uint32(1); // buyCount - should never be more than 1
            data << uint32(0);
        }
        else
        {
            data << uint32(i+5);
            data << uint32(0);
            data << uint32(0);
            data << uint32(0xFFFFFFFF);
            data << uint32(0);
            data << uint32(0);
            data << uint32(1); // buyCount - should never be more than 1
            data << uint32(0);
        }
    }

    _owner->GetSession()->SendPacket(&data);
}

uint32 Transmogrification::GetActiveTransEntry(uint8 slot, uint8 type, bool Weapon)
{
    if (Weapon)
    {
        std::list<TransmogrificationStructWeapon>::iterator itr;
        std::list<TransmogrificationStructWeapon>::iterator end;
        GetPossibleTransmigrificationsWeaponItr(slot, itr, end);
        
        for (; itr!= end; ++itr)
        {
            if (!itr->active)
                continue;

            switch (slot)
            {
            case 0:
                if (itr->active && itr->itemClass == 2 && (((ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_AXE ||  
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_MACE || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_SWORD || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_FIST || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_DAGGER) && (itr->subclass == ITEM_SUBCLASS_WEAPON_AXE || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_MACE || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_SWORD || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_FIST || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_DAGGER)) || 
                    ((ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_AXE2 || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_MACE2 || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_POLEARM || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                    ItemTransmogsMainSubClasses[type] == ITEM_SUBCLASS_WEAPON_STAFF) && (itr->subclass == ITEM_SUBCLASS_WEAPON_AXE2 || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_MACE2 || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_POLEARM || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                    itr->subclass == ITEM_SUBCLASS_WEAPON_STAFF))))
                    {
                        return itr->itemId;
                    }
                break;
            case 1:
                if (itr->active && ((itr->itemClass == 2 && type < 5) || ((itr->itemClass == 4 && type > 4) && itr->subclass == ItemTransmogsOffSubClasses[type])))
                {
                    return itr->itemId;
                }
                break;
            case 2:
                {
                    if (itr->itemClass == 2)
                    {
                        if (itr->active && (((ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_BOW ||
                            ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_GUN ||
                            ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_CROSSBOW) && (itr->subclass == ITEM_SUBCLASS_WEAPON_BOW ||
                            itr->subclass == ITEM_SUBCLASS_WEAPON_GUN ||
                            itr->subclass == ITEM_SUBCLASS_WEAPON_CROSSBOW)) || ((ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_THROWN || ItemTransmogsRangedSubClasses[type] == ITEM_SUBCLASS_WEAPON_WAND) && 
                            (itr->subclass == ITEM_SUBCLASS_WEAPON_THROWN || itr->subclass == ITEM_SUBCLASS_WEAPON_WAND))))
                        {
                            {
                                return itr->itemId;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    else if (GetActiveTransmogrStructure() != nullptr)
        return ((GetActiveTransmogrStructure())[slot]).itemid;
    return 0;
}


uint8 Transmogrification::CouldEquipItem(uint8 slot, ItemPrototype const* pProto) const
{
    // May be here should be more stronger checks; STUNNED checked
    // ROOT, CONFUSED, DISTRACTED, FLEEING this needs to be checked.
    if (_owner->HasUnitState(UNIT_STAT_STUNNED))
        return EQUIP_ERR_YOU_ARE_STUNNED;

    if (BattleGround* bg = _owner->GetBattleGround())
        if (bg->isArena() && bg->GetStatus() == STATUS_IN_PROGRESS)
            return EQUIP_ERR_NOT_DURING_ARENA_MATCH;
                
    if (_owner->IsNonMeleeSpellCast(false))
    {
        // exclude spells with transform item effect
        if (!_owner->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
            (_owner->GetCurrentSpell(CURRENT_GENERIC_SPELL)->GetSpellEntry()->Effect[0] != SPELL_EFFECT_SUMMON_CHANGE_ITEM &&
            _owner->GetCurrentSpell(CURRENT_GENERIC_SPELL)->GetSpellEntry()->Effect[1] != SPELL_EFFECT_SUMMON_CHANGE_ITEM &&
            _owner->GetCurrentSpell(CURRENT_GENERIC_SPELL)->GetSpellEntry()->Effect[2] != SPELL_EFFECT_SUMMON_CHANGE_ITEM))

            return EQUIP_ERR_CANT_DO_RIGHT_NOW;
    }
            
    uint8 eslot = _owner->FindEquipSlot(pProto, slot, true);
    if (eslot == NULL_SLOT)
        return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

    return EQUIP_ERR_OK;
}


bool Transmogrification::SatisfySlotRequirements(ItemPrototype const* itemProto, uint8 slot, uint32 & reasonStringEntry, bool Weapon)
{
    reasonStringEntry = 0; // when returning false this almost always is a number. When 0 - then error was sent in other way

    if (Weapon)
    {
        uint32 itemId = itemProto->ItemId;
        std::list<TransmogrificationStructWeapon>::iterator itr;
        std::list<TransmogrificationStructWeapon>::iterator end;
        GetPossibleTransmigrificationsWeaponItr(slot, itr, end);
        bool DoHave = false;
        for (; itr!= end; ++itr)
        {
            if (itr->itemId != itemId)
                continue;

            DoHave = true;
            break;
        }
        if (DoHave)
        {
            reasonStringEntry = LANG_SCRIPT_TRANS_ALREADY_HAVE;
            return false;
        }

        uint8 eqslot = EQUIPMENT_SLOT_MAINHAND + slot;
        bool isOkItem = false;
        switch (slot)
        {
            case 0:
                if (itemProto->Class == 2 && (((ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_AXE ||  
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_MACE || 
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_SWORD || 
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_FIST || 
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_DAGGER) && (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_FIST || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER)) || 
                    ((ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_AXE2 || 
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_MACE2 || 
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_POLEARM || 
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                    ItemTransmogsMainSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_STAFF) && (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2 || 
                    itemProto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF))))
                        isOkItem = true;
                break;
            case 1:
                if ((itemProto->Class == 2 && m_shownList.type < 5) || ((itemProto->Class == 4 && m_shownList.type > 4) && itemProto->SubClass == ItemTransmogsOffSubClasses[m_shownList.type]))
                    isOkItem = true;
                break;
            case 2:
                {
                    if (itemProto->Class == 2)
                    {
                        if (((ItemTransmogsRangedSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_BOW ||
                            ItemTransmogsRangedSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_GUN ||
                            ItemTransmogsRangedSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_CROSSBOW) && (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_BOW ||
                            itemProto->SubClass == ITEM_SUBCLASS_WEAPON_GUN ||
                            itemProto->SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW)) || ((ItemTransmogsRangedSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_THROWN || ItemTransmogsRangedSubClasses[m_shownList.type] == ITEM_SUBCLASS_WEAPON_WAND) && 
                            (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN || itemProto->SubClass == ITEM_SUBCLASS_WEAPON_WAND)))
                            isOkItem = true;
                    }
                    break;
                }
        }

        if (!isOkItem)
        {
            reasonStringEntry = LANG_SCRIPT_TRANS_WRONG_ITEM;
            return false;
        }

        uint8 msg = CouldEquipItem(eqslot, itemProto);
        if (msg != EQUIP_ERR_OK)
        {
            _owner->SendEquipError(msg, NULL, NULL);
            return false;
        }

        // restrict to save blizzlike legendaries models
        switch (itemProto->ItemId)
        {
            case 18348:
            case 18608:
            case 19896:
            case 19910:
            case 29353:
            case 29355:
                reasonStringEntry = LANG_SCRIPT_TRANS_WRONG_ITEM;
                return false;
            default:
                break;
        }

        if (GetTransmogrificationWeaponSize(slot) > MAX_TRANSMOG_MODELS)
        {
            reasonStringEntry = LANG_SCRIPT_REACHED_MAX_TRANS;
            return false;
        }

        if (!_owner->HasItemCount(TRANSMOG_BATTERY_1, 1))
        {
            reasonStringEntry = LANG_NO_TRANS_REAGENT1;
            return false;
        }

        return true;
    }


    // Here NON WEAPON

    std::list<uint32>::iterator itr;
    std::list<uint32>::iterator end;
    GetPossibleTransmigrificationsItr(slot, itr, end);
    bool DoHave = GetActiveTransmogrStructure() != nullptr && ((GetActiveTransmogrStructure())[slot]).itemid == itemProto->ItemId;
    for (; itr != end && !DoHave/*up to 10 items seen*/; ++itr)
    {
        if (itemProto->ItemId == *itr)
            DoHave = true;
        else
            continue;
    }
    if (DoHave)
    {
        reasonStringEntry = LANG_SCRIPT_TRANS_ALREADY_HAVE;
        return false;
    }

    /*uint8 itemSubclassAvailable;
    switch (_owner->GetClass())
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
            itemSubclassAvailable = ITEM_SUBCLASS_ARMOR_PLATE;
            break;
        case CLASS_HUNTER:
        case CLASS_SHAMAN:
            itemSubclassAvailable = ITEM_SUBCLASS_ARMOR_MAIL;
            break;
        case CLASS_ROGUE:
        case CLASS_DRUID:
            itemSubclassAvailable = ITEM_SUBCLASS_ARMOR_LEATHER;
            break;
        case CLASS_PRIEST:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            itemSubclassAvailable = ITEM_SUBCLASS_ARMOR_CLOTH;
            break;
        default:
            itemSubclassAvailable = ITEM_SUBCLASS_ARMOR_SHIELD; // just so player can't use it
            break;
    }

    if (itemProto->SubClass != itemSubclassAvailable && slot != 2 && slot != 9 && slot != 10)
    {
        reasonStringEntry = LANG_SCRIPT_TRANS_WRONG_ITEM;
        return false;
    }*/

    uint8 msg = CouldEquipItem(TransmogrificationSlots[m_shownList.slot], itemProto);
    if (msg != EQUIP_ERR_OK)
    {
        _owner->SendEquipError(msg, NULL, NULL);
        return false;
    }

    if (GetTransmogrificationSize(slot) > MAX_TRANSMOG_MODELS || GetTransmogrificationSize(slot) > (MAX_TRANSMOG_MODELS - 1) && (GetActiveTransmogrStructure() != nullptr) && ((GetActiveTransmogrStructure()[slot]).itemid)) // can add more transmogr
    {
        reasonStringEntry = LANG_SCRIPT_REACHED_MAX_TRANS;
        return false;
    }

    // shoudlers, head
    if (itemProto->InventoryType == INVTYPE_SHOULDERS || itemProto->InventoryType == INVTYPE_HEAD)
    {
        if (!_owner->HasItemCount(TRANSMOG_BATTERY_2, 1))
        {
            reasonStringEntry = LANG_NO_TRANS_REAGENT2;
            return false;
        }
    }
    // chest, legs
    else if (itemProto->InventoryType == INVTYPE_ROBE || itemProto->InventoryType == INVTYPE_CHEST || itemProto->InventoryType == INVTYPE_LEGS)
    {
        if (!_owner->HasItemCount(TRANSMOG_BATTERY_3, 1))
        {
            reasonStringEntry = LANG_NO_TRANS_REAGENT3;
            return false;
        }
    }
    // all other
    else if (itemProto->InventoryType == INVTYPE_CLOAK || itemProto->InventoryType == INVTYPE_HANDS || itemProto->InventoryType == INVTYPE_FEET || itemProto->InventoryType == INVTYPE_WAIST || itemProto->InventoryType == INVTYPE_WRISTS || itemProto->InventoryType == INVTYPE_TABARD)
    {
        if (!_owner->HasItemCount(TRANSMOG_BATTERY_4, 1))
        {
            reasonStringEntry = LANG_NO_TRANS_REAGENT4;
            return false;
        }
    }
    else
    {
        // wtf?
        reasonStringEntry = LANG_SIMPLE_ERROR;
        return false;
    }

    return true;
}

void Transmogrification::HandleAddTransmogPacket(uint64 itemguid)
{    
    Item *pItem = _owner->GetItemByGuid(itemguid);
    ItemPrototype const *pProto = NULL;
    if (pItem)
    {
        // prevent sell not owner item
        if (_owner->GetGUID()!=pItem->GetOwnerGUID())
        {
            _owner->SendSellError(SELL_ERR_UNK, NULL, itemguid, 0);
            return;
        }

        // prevent sell currently looted item
        if (_owner->GetLootGUID()==pItem->GetGUID())
        {
            _owner->SendSellError(SELL_ERR_UNK, NULL, itemguid, 0);
            return;
        }

        pProto = pItem->GetProto();
        if (!pProto)
        {
            _owner->SendSellError(SELL_ERR_UNK, NULL, itemguid, 0);
            return;
        }

        // exclude this
        switch (pProto->ItemId)
        {
        case 22736: // Adonis
        case 30318:
        case 30317:
        case 30312:
        case 30311:
        case 30316:
        case 30314:
        case 30313:
            _owner->SendSellError(SELL_ERR_UNK, NULL, itemguid, 0);
            return;
        }
    }
    else
    {
        _owner->SendSellError(SELL_ERR_UNK, NULL, itemguid, 0);
        return;
    }

    /*if (urand(0,1) == 0)
    {
    WorldPacket data(SMSG_GOSSIP_MESSAGE, (100));         // guess size
    data << uint64(GetGUID());
    data << uint32(0);                                      // new 2.4.0
    data << uint32(1);
    data << uint32(1);          // max count 0x0F

    
        data << uint32(1);
        data << uint8(1);
        data << uint8(1);                    // makes pop up box password
        data << uint32(1);                  // money required to open menu, 2.0.3
        data << "gmessage";                           // text for gossip item
        data << "gBoxMessage";                        // accept text (related to money) pop up box, 2.0.3

    SendPacketToSelf(&data);
    }*/ // Packet of agreement - SHUTS THE VENDOR WINDOW!

    uint32 reasonStringEntry = 0;
    if (!SatisfySlotRequirements(pProto, m_shownList.slot, reasonStringEntry, m_shownList.weapon) || pItem->IsBindedNotWith(_owner->GetGUID()))
    {
        if (reasonStringEntry)
            _owner->GetSession()->SendNotification(_owner->GetSession()->GetHellgroundString(reasonStringEntry));
        _owner->SendSellError(SELL_ERR_UNK, NULL, itemguid, 0);
        return;
    }

    int32 customBP = 1000000000 + (m_shownList.weapon ? 100000000 : 0) + m_shownList.slot*1000000;

    pItem->SetBinding(true);
    customBP += pItem->GetEntry();
    _owner->CastCustomSpell(_owner, 55150, &customBP, NULL, NULL, false);
    _owner->SendSellError(SELL_ERR_UNK, NULL, itemguid, 0);
    return;
}

void Transmogrification::HandleSelectTransmogPacket(uint32 itemid)
{
    if (!itemid)
        return;

    if (itemid == GO_BACK_ITEM)
        GoToMainMenu();
    else
        Select(m_shownList.weapon, m_shownList.slot, m_shownList.type, (itemid == SELECT_ORIGINAL_ITEM || itemid == QUESTION_MARK_ITEM) ? 0 : itemid);
}

void Transmogrification::GoToMainMenu()
{
    SpellCastTargets const targets;
    Item *pItem = _owner->GetItemByGuid(_itemGUID);
    if (pItem)
        sScriptMgr.OnItemUse(_owner, pItem, targets);
}

uint8 Transmogrification::GetWeaponTypeFromSubclass(uint8 subclass, uint8 slot, uint8 Class)
{
    switch (slot)
    {
    case 0: 
        for (uint8 i = 0; i < MaxTypesForSlot[slot]; ++i)
        {
            if (ItemTransmogsMainSubClasses[i] == subclass)
                return i;
        }
        break;
    case 1: 
        for (uint8 i = 0; i < MaxTypesForSlot[slot]; ++i)
        {
            if ((Class != 2 && i < 5) || (Class == 2 && i > 4))
                continue;

            if (ItemTransmogsOffSubClasses[i] == subclass)
                return i;
        }
        break;
    case 2: 
        for (uint8 i = 0; i < MaxTypesForSlot[slot]; ++i)
        {
            if (ItemTransmogsRangedSubClasses[i] == subclass)
                return i;
        }
        break;
    }
    return 0;
}