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

#include "Common.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "Database/DatabaseEnv.h"
#include "ItemEnchantmentMgr.h"

void AddItemsSetItem(Player*player,Item *item)
{
    ItemPrototype const *proto = item->GetProto();
    uint32 setid = proto->ItemSet;

    ItemSetEntry const *set = sItemSetStore.LookupEntry(setid);

    if (!set)
    {
        sLog.outLog(LOG_DB_ERR, "Item set %u for item (id %u) not found, mods not applied.",setid,proto->ItemId);
        return;
    }

    if (set->required_skill_id && player->GetSkillValue(set->required_skill_id) < set->required_skill_value)
        return;

    ItemSetEffect *eff = NULL;

    for (size_t x = 0; x < player->ItemSetEff.size(); ++x)
    {
        if (player->ItemSetEff[x] && player->ItemSetEff[x]->setid == setid)
        {
            eff = player->ItemSetEff[x];
            break;
        }
    }

    if (!eff)
    {
        eff = new ItemSetEffect;
        memset(eff,0,sizeof(ItemSetEffect));
        eff->setid = setid;

        size_t x = 0;
        for (; x < player->ItemSetEff.size(); x++)
            if (!player->ItemSetEff[x])
                break;

        if (x < player->ItemSetEff.size())
            player->ItemSetEff[x]=eff;
        else
            player->ItemSetEff.push_back(eff);
    }

    ++eff->item_count;

    for (uint32 x=0;x<8;x++)
    {
        if (!set->spells [x])
            continue;
        //not enough for  spell
        if (set->items_to_triggerspell[x] > eff->item_count)
            continue;

        uint32 z=0;
        for (;z<8;z++)
            if (eff->spells[z] && eff->spells[z]->Id==set->spells[x])
                break;

        if (z < 8)
            continue;

        //new spell
        for (uint32 y=0;y<8;y++)
        {
            if (!eff->spells[y])                             // free slot
            {
                SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(set->spells[x]);
                if (!spellInfo)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: unknown spell id %u in items set %u effects", set->spells[x],setid);
                    break;
                }

                // spell cast only if fit form requirement, in other case will cast at form change
                player->ApplyEquipSpell(spellInfo,NULL,true);
                eff->spells[y] = spellInfo;
                break;
            }
        }
    }
}

void RemoveItemsSetItem(Player*player,ItemPrototype const *proto)
{
    uint32 setid = proto->ItemSet;

    ItemSetEntry const *set = sItemSetStore.LookupEntry(setid);

    if (!set)
    {
        sLog.outLog(LOG_DB_ERR, "Item set #%u for item #%u not found, mods not removed.",setid,proto->ItemId);
        return;
    }

    ItemSetEffect *eff = NULL;
    size_t setindex = 0;
    for (;setindex < player->ItemSetEff.size(); setindex++)
    {
        if (player->ItemSetEff[setindex] && player->ItemSetEff[setindex]->setid == setid)
        {
            eff = player->ItemSetEff[setindex];
            break;
        }
    }

    // can be in case now enough skill requirement for set appling but set has been appliend when skill requirement not enough
    if (!eff)
        return;

    --eff->item_count;

    for (uint32 x=0;x<8;x++)
    {
        if (!set->spells[x])
            continue;

        // enough for spell
        if (set->items_to_triggerspell[x] <= eff->item_count)
            continue;

        for (uint32 z=0;z<8;z++)
        {
            if (eff->spells[z] && eff->spells[z]->Id==set->spells[x])
            {
                // spell can be not active if not fit form requirement
                player->ApplyEquipSpell(eff->spells[z],NULL,false);
                eff->spells[z]=NULL;
                break;
            }
        }
    }

    if (!eff->item_count)                                    //all items of a set were removed
    {
        ASSERT(eff == player->ItemSetEff[setindex]);
        delete eff;
        player->ItemSetEff[setindex] = NULL;
    }
}

bool ItemCanGoIntoBag(ItemPrototype const *pProto, ItemPrototype const *pBagProto)
{
    if (!pProto || !pBagProto)
        return false;

    switch (pBagProto->Class)
    {
        case ITEM_CLASS_CONTAINER:
            switch (pBagProto->SubClass)
            {
                case ITEM_SUBCLASS_CONTAINER:
                    return true;
                case ITEM_SUBCLASS_SOUL_CONTAINER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_SOUL_SHARDS))
                        return false;
                    return true;
                case ITEM_SUBCLASS_HERB_CONTAINER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_HERBS))
                        return false;
                    return true;
                case ITEM_SUBCLASS_ENCHANTING_CONTAINER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_ENCHANTING_SUPP))
                        return false;
                    return true;
                case ITEM_SUBCLASS_MINING_CONTAINER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_MINING_SUPP))
                        return false;
                    return true;
                case ITEM_SUBCLASS_ENGINEERING_CONTAINER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_ENGINEERING_SUPP))
                        return false;
                    return true;
                case ITEM_SUBCLASS_GEM_CONTAINER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_GEMS))
                        return false;
                    return true;
                case ITEM_SUBCLASS_LEATHERWORKING_CONTAINER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_LEATHERWORKING_SUPP))
                        return false;
                    return true;
                default:
                    return false;
            }
        case ITEM_CLASS_QUIVER:
            switch (pBagProto->SubClass)
            {
                case ITEM_SUBCLASS_QUIVER:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_ARROWS))
                        return false;
                    return true;
                case ITEM_SUBCLASS_AMMO_POUCH:
                    if (!(pProto->BagFamily & BAG_FAMILY_MASK_BULLETS))
                        return false;
                    return true;
                default:
                    return false;
            }
    }
    return false;
}

Item::Item()
{
    m_objectType |= TYPEMASK_ITEM;
    m_objectTypeId = TYPEID_ITEM;
                                                            // 2.3.2 - 0x18
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID);

    m_valuesCount = ITEM_END;
    m_slot = 0;
    uState = ITEM_NEW;
    uQueuePos = -1;
    m_container = NULL;
    m_lootGenerated = false;
    mb_in_trade = false;
}

bool Item::Create(uint32 guidlow, uint32 itemid, Player const* owner)
{
    Object::_Create(guidlow, 0, HIGHGUID_ITEM);

    SetEntry(itemid);
    SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);

    SetUInt64Value(ITEM_FIELD_OWNER, owner ? owner->GetGUID() : 0);
    SetUInt64Value(ITEM_FIELD_CONTAINED, owner ? owner->GetGUID() : 0);

    ItemPrototype const *itemProto = ObjectMgr::GetItemPrototype(itemid);
    if (!itemProto)
        return false;

    SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);
    SetUInt32Value(ITEM_FIELD_MAXDURABILITY, itemProto->MaxDurability);
    SetUInt32Value(ITEM_FIELD_DURABILITY, itemProto->MaxDurability);

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        SetSpellCharges(i,itemProto->Spells[i].SpellCharges);

    SetUInt32Value(ITEM_FIELD_FLAGS, itemProto->Flags);
    SetUInt32Value(ITEM_FIELD_DURATION, abs(itemProto->Duration));

    debug_info = { GetGUIDLow(), GetEntry(), itemProto->Name1 };

    return true;
}


void Item::AddToClientUpdateList()
{
    if (Player* pl = GetOwner())
        pl->GetMap()->AddUpdateObject(this);
}

void Item::RemoveFromClientUpdateList()
{
    if (Player* pl = GetOwner())
        pl->GetMap()->RemoveUpdateObject(this);
}

void Item::BuildUpdate(UpdateDataMapType& data_map)
{
    if (Player* pl = GetOwner())
        BuildFieldsUpdate(pl, data_map);

    ClearUpdateMask(false);
}

void Item::UpdateDuration(Player* owner, uint32 diff)
{
    if (!GetUInt32Value(ITEM_FIELD_DURATION))
        return;

    sLog.outDebug("Item::UpdateDuration Item (Entry: %u Duration %u Diff %u)",GetEntry(),GetUInt32Value(ITEM_FIELD_DURATION),diff);

    if (GetUInt32Value(ITEM_FIELD_DURATION)<=diff)
    {
        owner->DestroyItem(GetBagSlot(), GetSlot(), true, "DURATION_DESTROY");
        return;
    }

    SetUInt32Value(ITEM_FIELD_DURATION, GetUInt32Value(ITEM_FIELD_DURATION) - diff);
    SetState(ITEM_CHANGED, owner);                          // save new time in database
}

int32 Item::GetSpellCharges(uint8 index, bool isNormal) const
{
    int32 val = GetInt32Value(ITEM_FIELD_SPELL_CHARGES + index);
    if (isNormal)
    {
        val = (val < -1) ? -val : val;
    }
    return val;
}

void Item::SetSpellCharges(uint8 index, int32 value)
{
    SetInt32Value(ITEM_FIELD_SPELL_CHARGES + index, (value > 0) ? -value : value);
}

void Item::SaveToDB()
{
    uint32 guid = GetGUIDLow();
    switch (uState)
    {
        case ITEM_NEW:
        {
            static SqlStatementID deleteItem;
            static SqlStatementID saveItem;

            SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteItem, "DELETE FROM item_instance WHERE guid = ?");
            stmt.PExecute(guid);

            stmt = RealmDataDatabase.CreateStatement(saveItem, "INSERT INTO item_instance (guid, owner_guid, data) VALUES (?, ?, ?)");

            std::ostringstream ss;
            for (uint16 i = 0; i < m_valuesCount; i++)
                ss << GetUInt32Value(i) << " ";

            stmt.PExecute(guid, GUID_LOPART(GetOwnerGUID()), ss.str().c_str());
        }
        break;
        case ITEM_CHANGED:
        {
            static SqlStatementID updateItem;
            static SqlStatementID updateGift;

            SqlStatement stmt = RealmDataDatabase.CreateStatement(updateItem, "UPDATE item_instance SET data = ?,  owner_guid = ? WHERE guid = ?");

            std::ostringstream ss;
            for (uint16 i = 0; i < m_valuesCount; i++)
                ss << GetUInt32Value(i) << " ";

            stmt.PExecute(ss.str().c_str(), GUID_LOPART(GetOwnerGUID()), guid);

            if (HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_WRAPPED))
            {
                stmt = RealmDataDatabase.CreateStatement(updateGift, "UPDATE character_gifts SET guid = ? WHERE item_guid = ?");
                stmt.PExecute(GUID_LOPART(GetOwnerGUID()), GetGUIDLow());
            }
        }
        break;
        case ITEM_REMOVED:
        {
            static SqlStatementID deleteItem;
            static SqlStatementID deleteItemText;
            static SqlStatementID deleteGift;

            SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteItem, "DELETE FROM item_instance WHERE guid = ?");
            stmt.PExecute(guid);

            if (GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID) > 0)
            {
                stmt = RealmDataDatabase.CreateStatement(deleteItemText, "DELETE FROM item_text WHERE id = ?");
                stmt.PExecute(GetUInt32Value(ITEM_FIELD_ITEM_TEXT_ID));
            }

            if (HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_WRAPPED))
            {
                stmt = RealmDataDatabase.CreateStatement(deleteGift, "DELETE FROM character_gifts WHERE item_guid = ?");
                stmt.PExecute(GetGUIDLow());
            }

            // Delete the items if this is a container
            if (!loot.isLooted())
                ItemContainerDeleteLootMoneyAndLootItemsFromDB();

            delete this;
            return;
        }
        case ITEM_UNCHANGED:
            break;
    }
    SetState(ITEM_UNCHANGED);
}

bool Item::LoadFromDB(uint32 guid, uint64 owner_guid, QueryResultAutoPtr result)
{
    // create item before any checks for store correct guid
    // and allow use "FSetState(ITEM_REMOVED); SaveToDB();" for deleting item from DB
    Object::_Create(guid, 0, HIGHGUID_ITEM);

    if (!result)
        result = RealmDataDatabase.PQuery("SELECT data FROM item_instance WHERE guid = '%u'", guid);

    if (!result)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Item (GUID: %u owner: %u) not found in table `item_instance`, can't load. ",guid,GUID_LOPART(owner_guid));
        return false;
    }

    Field *fields = result->Fetch();

    if (!LoadValues(fields[0].GetString()))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Item #%d have broken data in `data` field. Can't be loaded.",guid);
        return false;
    }

    bool need_save = false;                                 // need explicit save data at load fixes

    // overwrite possible wrong/corrupted guid
    uint64 new_item_guid = MAKE_NEW_GUID(guid,0, HIGHGUID_ITEM);
    if (GetUInt64Value(OBJECT_FIELD_GUID) != new_item_guid)
    {
        SetUInt64Value(OBJECT_FIELD_GUID, MAKE_NEW_GUID(guid,0, HIGHGUID_ITEM));
        need_save = true;
    }

    ItemPrototype const* proto = GetProto();
    if (!proto)
        return false;

    // recalculate suffix factor
    if (GetItemRandomPropertyId() < 0)
    {
        if (UpdateItemSuffixFactor())
            need_save = true;
    }

    // Remove bind flag for items vs NO_BIND set
    if (IsSoulBound() && proto->Bonding == NO_BIND)
    {
        ApplyModFlag(ITEM_FIELD_FLAGS,ITEM_FLAGS_BINDED, false);
        need_save = true;
    }

    // update duration if need, and remove if not need
    if ((proto->Duration==0) != (GetUInt32Value(ITEM_FIELD_DURATION)==0))
    {
        SetUInt32Value(ITEM_FIELD_DURATION,abs(proto->Duration));
        need_save = true;
    }

    // set correct owner
    if (owner_guid != 0 && GetOwnerGUID() != owner_guid)
    {
        SetOwnerGUID(owner_guid);
        need_save = true;
    }

    if (need_save)                                           // normal item changed state set not work at loading
    {
        std::ostringstream ss;
        ss << "UPDATE item_instance SET data = '";
        for (uint16 i = 0; i < m_valuesCount; i++)
            ss << GetUInt32Value(i) << " ";
        ss << "', owner_guid = '" << GUID_LOPART(GetOwnerGUID()) << "' WHERE guid = '" << guid << "'";

        RealmDataDatabase.Execute(ss.str().c_str());
    }

    debug_info = { GetGUIDLow(), GetEntry(), proto->Name1 };

    return true;
}

void Item::DeleteFromDB()
{
    RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'",GetGUIDLow());

    // Delete the items if this is a container
    if (!loot.isLooted())
        ItemContainerDeleteLootMoneyAndLootItemsFromDB();
}

void Item::DeleteFromInventoryDB()
{
    RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'",GetGUIDLow());
}

ItemPrototype const *Item::GetProto() const
{
    return ObjectMgr::GetItemPrototype(GetEntry());
}

Player* Item::GetOwner()const
{
    return sObjectMgr.GetPlayerInWorld(GetOwnerGUID());
}

int32 Item::GenerateItemRandomPropertyId(uint32 item_id)
{
    ItemPrototype const *itemProto = sItemStorage.LookupEntry<ItemPrototype>(item_id);

    if (!itemProto)
        return 0;

    // item must have one from this field values not null if it can have random enchantments
    if ((!itemProto->RandomProperty) && (!itemProto->RandomSuffix))
        return 0;

    // item can have not null only one from field values
    if ((itemProto->RandomProperty) && (itemProto->RandomSuffix))
    {
        sLog.outLog(LOG_DB_ERR, "Item template %u have RandomProperty==%u and RandomSuffix==%u, but must have one from field =0",itemProto->ItemId,itemProto->RandomProperty,itemProto->RandomSuffix);
        return 0;
    }

    // RandomProperty case
    if (itemProto->RandomProperty)
    {
        uint32 randomPropId = GetItemEnchantMod(itemProto->RandomProperty);
        ItemRandomPropertiesEntry const *random_id = sItemRandomPropertiesStore.LookupEntry(randomPropId);
        if (!random_id)
        {
            sLog.outLog(LOG_DB_ERR, "Enchantment id #%u used but it doesn't have records in 'ItemRandomProperties.dbc'",randomPropId);
            return 0;
        }

        return random_id->ID;
    }
    // RandomSuffix case
    else
    {
        uint32 randomPropId = GetItemEnchantMod(itemProto->RandomSuffix);
        ItemRandomSuffixEntry const *random_id = sItemRandomSuffixStore.LookupEntry(randomPropId);
        if (!random_id)
        {
            sLog.outLog(LOG_DB_ERR, "Enchantment id #%u used but it doesn't have records in sItemRandomSuffixStore.",randomPropId);
            return 0;
        }

        return -int32(random_id->ID);
    }
}

void Item::SetItemRandomProperties(int32 randomPropId)
{
    if (!randomPropId)
        return;

    if (randomPropId > 0)
    {
        ItemRandomPropertiesEntry const *item_rand = sItemRandomPropertiesStore.LookupEntry(randomPropId);
        if (item_rand)
        {
            if (GetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID) != int32(item_rand->ID))
            {
                SetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID,item_rand->ID);
                SetState(ITEM_CHANGED);
            }
            for (uint32 i = PROP_ENCHANTMENT_SLOT_2; i < PROP_ENCHANTMENT_SLOT_2 + 3; ++i)
                SetEnchantment(EnchantmentSlot(i),item_rand->enchant_id[i - PROP_ENCHANTMENT_SLOT_2],0,0);
        }
    }
    else
    {
        ItemRandomSuffixEntry const *item_rand = sItemRandomSuffixStore.LookupEntry(-randomPropId);
        if (item_rand)
        {
            if (GetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID) != -int32(item_rand->ID) ||
                !GetItemSuffixFactor())
            {
                SetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID,-int32(item_rand->ID));
                UpdateItemSuffixFactor();
                SetState(ITEM_CHANGED);
            }

            for (uint32 i = PROP_ENCHANTMENT_SLOT_0; i < PROP_ENCHANTMENT_SLOT_0 + 3; ++i)
                SetEnchantment(EnchantmentSlot(i),item_rand->enchant_id[i - PROP_ENCHANTMENT_SLOT_0],0,0);
        }
    }
}

bool Item::UpdateItemSuffixFactor()
{
    uint32 suffixFactor = GenerateEnchSuffixFactor(GetEntry());
    if (GetItemSuffixFactor()==suffixFactor)
        return false;
    SetUInt32Value(ITEM_FIELD_PROPERTY_SEED,suffixFactor);
    return true;
}

void Item::SetState(ItemUpdateState state, Player *forplayer)
{
    if (uState == ITEM_NEW && state == ITEM_REMOVED)
    {
        // pretend the item never existed
        RemoveFromUpdateQueueOf(forplayer);
        delete this;
        return;
    }

    if (state != ITEM_UNCHANGED)
    {
        // new items must stay in new state until saved
        if (uState != ITEM_NEW) uState = state;
        AddToUpdateQueueOf(forplayer);
    }
    else
    {
        // unset in queue
        // the item must be removed from the queue manually
        uQueuePos = -1;
        uState = ITEM_UNCHANGED;
    }
}

void Item::AddToUpdateQueueOf(Player *player)
{
    if (IsInUpdateQueue()) return;

    if (!player)
    {
        player = GetOwner();
        if (!player)
        {
            sLog.outDebug("Item::AddToUpdateQueueOf - GetPlayer didn't find a player matching owner's guid (%u)!", GUID_LOPART(GetOwnerGUID()));
            return;
        }
    }

    if (player->GetGUID() != GetOwnerGUID())
    {
        sLog.outDebug("Item::AddToUpdateQueueOf - Owner's guid (%u) and player's guid (%u) don't match!", GUID_LOPART(GetOwnerGUID()), player->GetGUIDLow());
        return;
    }

    if (player->m_itemUpdateQueueBlocked) return;

    player->m_itemUpdateQueue.push_back(this);
    uQueuePos = player->m_itemUpdateQueue.size()-1;
}

void Item::RemoveFromUpdateQueueOf(Player *player)
{
    if (!IsInUpdateQueue()) return;

    if (!player)
    {
        player = GetOwner();
        if (!player)
        {
            sLog.outDebug("Item::RemoveFromUpdateQueueOf - GetPlayer didn't find a player matching owner's guid (%u)!", GUID_LOPART(GetOwnerGUID()));
            return;
        }
    }

    if (player->GetGUID() != GetOwnerGUID())
    {
        sLog.outDebug("Item::RemoveFromUpdateQueueOf - Owner's guid (%u) and player's guid (%u) don't match!", GUID_LOPART(GetOwnerGUID()), player->GetGUIDLow());
        return;
    }
    ASSERT(player->mMitems.find(GetGUIDLow()) == player->mMitems.end());
    // we remove item, lets make sure there is no trace of it
    if (player->m_itemUpdateQueueBlocked) return;

    player->m_itemUpdateQueue[uQueuePos] = NULL;
    uQueuePos = -1;
}

uint8 Item::GetBagSlot() const
{
    return m_container ? m_container->GetSlot() : uint8(INVENTORY_SLOT_BAG_0);
}

bool Item::IsEquipped() const
{
    return !IsInBag() && m_slot < EQUIPMENT_SLOT_END;
}

bool Item::CanBeTraded() const
{
    if (IsSoulBound())
        return false;
    if (IsBag() && (Player::IsBagPos(GetPos()) || !((Bag const*)this)->IsEmpty()))
        return false;

    if (Player* owner = GetOwner())
    {
        if (owner->CanUnequipItem(GetPos(),false) !=  EQUIP_ERR_OK)
            return false;
        if (owner->GetLootGUID()==GetGUID())
            return false;
    }

    if (IsBoundByEnchant())
        return false;

    return true;
}

bool Item::IsBoundByEnchant() const
{
    // Check all enchants for soulbound
    for (uint32 enchant_slot = PERM_ENCHANTMENT_SLOT; enchant_slot < MAX_ENCHANTMENT_SLOT; ++enchant_slot)
    {
        uint32 enchant_id = GetEnchantmentId(EnchantmentSlot(enchant_slot));
        if (!enchant_id)
            continue;

        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!enchantEntry)
            continue;

        if (enchantEntry->slot & ENCHANTMENT_CAN_SOULBOUND)
            return true;
    }
    return false;
}

bool Item::IsFitToSpellRequirements(SpellEntry const* spellInfo) const
{
    ItemPrototype const* proto = GetProto();

    if (spellInfo->EquippedItemClass != -1)                 // -1 == any item class
    {
        if (spellInfo->EquippedItemClass != int32(proto->Class))
            return false;                                   //  wrong item class
        
        if (spellInfo->EquippedItemSubClassMask != 0) // 0 == any subclass
        {
            // @!legendary_weapon - axe case
            // for Human there's two checks for sword and mace, so it is enough to check just sword to avoid shitting the code
            bool ignore_item_subclass = proto->ItemId == ITEM_LEGENDARY_AXE && spellInfo->Id == 20597 &&
                ((spellInfo->EquippedItemSubClassMask & (1 << 8)) == 0 || (spellInfo->EquippedItemSubClassMask & (1 << 5)) == 0 || (spellInfo->EquippedItemSubClassMask & (1 << 1)) == 0) ? true : false;

            // GENSENTODO?

            if (!ignore_item_subclass)
            {
                if ((spellInfo->EquippedItemSubClassMask & (1 << proto->SubClass)) == 0)
                    return false;                               // subclass not present in mask
            }
        }
    }

    if (spellInfo->EquippedItemInventoryTypeMask != 0)       // 0 == any inventory type
    {
        if ((spellInfo->EquippedItemInventoryTypeMask  & (1 << proto->InventoryType)) == 0)
            return false;                                   // inventory type not present in mask
    }

    return true;
}

void Item::SetEnchantment(EnchantmentSlot slot, uint32 id, uint32 duration, uint32 charges)
{
    // Better lost small time at check in comparison lost time at item save to DB.
    if ((GetEnchantmentId(slot) == id) && (GetEnchantmentDuration(slot) == duration) && (GetEnchantmentCharges(slot) == charges))
        return;

    SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + slot*MAX_ENCHANTMENT_OFFSET + ENCHANTMENT_ID_OFFSET,id);
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + slot*MAX_ENCHANTMENT_OFFSET + ENCHANTMENT_DURATION_OFFSET,duration);
    SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + slot*MAX_ENCHANTMENT_OFFSET + ENCHANTMENT_CHARGES_OFFSET,charges);
    SetState(ITEM_CHANGED);
}

void Item::SetEnchantmentDuration(EnchantmentSlot slot, uint32 duration)
{
    if (GetEnchantmentDuration(slot) == duration)
        return;

    SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + slot*MAX_ENCHANTMENT_OFFSET + ENCHANTMENT_DURATION_OFFSET,duration);
    SetState(ITEM_CHANGED);
}

void Item::SetEnchantmentCharges(EnchantmentSlot slot, uint32 charges)
{
    if (GetEnchantmentCharges(slot) == charges)
        return;

    SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + slot*MAX_ENCHANTMENT_OFFSET + ENCHANTMENT_CHARGES_OFFSET,charges);
    SetState(ITEM_CHANGED);
}

void Item::ClearEnchantment(EnchantmentSlot slot)
{
    if (!GetEnchantmentId(slot))
        return;

    for (uint8 x = 0; x < 3; ++x)
        SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1 + slot*MAX_ENCHANTMENT_OFFSET + x, 0);
    SetState(ITEM_CHANGED);
}

bool Item::GemsFitSockets() const
{
    bool fits = true;
    for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
    {
        uint8 SocketColor = GetProto()->Socket[enchant_slot-SOCK_ENCHANTMENT_SLOT].Color;

        uint32 enchant_id = GetEnchantmentId(EnchantmentSlot(enchant_slot));
        if (!enchant_id)
        {
            if (SocketColor) fits &= false;
            continue;
        }

        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!enchantEntry)
        {
            if (SocketColor) fits &= false;
            continue;
        }

        uint8 GemColor = 0;

        uint32 gemid = enchantEntry->GemID;
        if (gemid)
        {
            ItemPrototype const* gemProto = sItemStorage.LookupEntry<ItemPrototype>(gemid);
            if (gemProto)
            {
                GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
                if (gemProperty)
                    GemColor = gemProperty->color;
            }
        }

        fits &= (GemColor & SocketColor) ? true : false;
    }
    return fits;
}

uint8 Item::GetGemCountWithID(uint32 GemID) const
{
    uint8 count = 0;
    for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
    {
        uint32 enchant_id = GetEnchantmentId(EnchantmentSlot(enchant_slot));
        if (!enchant_id)
            continue;

        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!enchantEntry)
            continue;

        if (GemID == enchantEntry->GemID)
            ++count;
    }
    return count;
}

bool Item::IsLimitedToAnotherMapOrZone(uint32 cur_mapId, uint32 cur_zoneId) const
{
    ItemPrototype const* proto = GetProto();
    return proto && (proto->Map && proto->Map != cur_mapId || proto->Area && proto->Area != cur_zoneId);
}

// Though the client has the information in the item's data field,
// we have to send SMSG_ITEM_TIME_UPDATE to display the remaining
// time.
void Item::SendTimeUpdate(Player* owner)
{
    if (!GetUInt32Value(ITEM_FIELD_DURATION))
        return;

    WorldPacket data(SMSG_ITEM_TIME_UPDATE, (8+4));
    data << (uint64)GetGUID();
    data << (uint32)GetUInt32Value(ITEM_FIELD_DURATION);
    owner->SendPacketToSelf(&data);
}

Item* Item::CreateItem(uint32 item, uint32 count, Player const* player)
{
    if (count < 1)
        return NULL;                                        //don't create item at zero count

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(item);
    if (pProto)
    {
        if (count > pProto->GetMaxStackSize())
            count = pProto->GetMaxStackSize();

        ASSERT(count !=0 && "pProto->Stackable==0 but checked at loading already");

        Item *pItem = NewItemOrBag(pProto);
        if (pItem->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_ITEM), item, player))
        {
            pItem->SetCount(count);
            return pItem;
        }
        else
            delete pItem;
    }
    return NULL;
}

Item* Item::CloneItem(uint32 count, Player const* player) const
{
    Item* newItem = CreateItem(GetEntry(), count, player);
    if (!newItem)
        return NULL;

    newItem->SetUInt32Value(ITEM_FIELD_CREATOR,      GetUInt32Value(ITEM_FIELD_CREATOR));
    newItem->SetUInt32Value(ITEM_FIELD_GIFTCREATOR,  GetUInt32Value(ITEM_FIELD_GIFTCREATOR));
    newItem->SetUInt32Value(ITEM_FIELD_FLAGS,        GetUInt32Value(ITEM_FIELD_FLAGS));
    newItem->SetUInt32Value(ITEM_FIELD_DURATION,     GetUInt32Value(ITEM_FIELD_DURATION));
    newItem->SetItemRandomProperties(GetItemRandomPropertyId());
    return newItem;
}

uint32 Item::GetSkill()
{
    const static uint32 item_weapon_skills[MAX_ITEM_SUBCLASS_WEAPON] =
    {
        SKILL_AXES,     SKILL_2H_AXES,  SKILL_BOWS,          SKILL_GUNS         , SKILL_MACES,
        SKILL_2H_MACES, SKILL_POLEARMS, SKILL_SWORDS,        SKILL_2H_SWORDS    , 0,
        SKILL_STAVES,   0,              0,                   SKILL_FIST_WEAPONS , 0,
        SKILL_DAGGERS,  SKILL_THROWN,   SKILL_ASSASSINATION, SKILL_CROSSBOWS    , SKILL_WANDS,
        SKILL_FISHING
    };

    const static uint32 item_armor_skills[MAX_ITEM_SUBCLASS_ARMOR] =
    {
        0,SKILL_CLOTH,SKILL_LEATHER,SKILL_MAIL,SKILL_PLATE_MAIL,0,SKILL_SHIELD,0,0,0
    };

    const ItemPrototype* pProto = GetProto();

    switch (pProto->Class)
    {
    case ITEM_CLASS_WEAPON:
        if (pProto->SubClass >= MAX_ITEM_SUBCLASS_WEAPON)
            return 0;
        else
            return item_weapon_skills[pProto->SubClass];

    case ITEM_CLASS_ARMOR:
        if (pProto->SubClass >= MAX_ITEM_SUBCLASS_ARMOR)
            return 0;
        else
            return item_armor_skills[pProto->SubClass];

    default:
        return 0;
    }
};

void Item::ItemContainerSaveLootToDB()
{
    // Saves the money and item loot associated with an openable item to the DB

    if (loot.isLooted()) // no money and no loot
        return;

    uint32 container_id = GetGUIDLow();
    RealmDataDatabase.BeginTransaction();
    loot.containerID = container_id; // Save this for when a LootItem is removed

    // Save money
    if (loot.gold > 0)
    {
        static SqlStatementID stmt_money_del;
        static SqlStatementID stmt_money_ins;
        SqlStatement stmt = RealmDataDatabase.CreateStatement(stmt_money_del, "DELETE FROM item_loot_money WHERE container_id = ?");
        stmt.addUInt32(container_id);
        stmt.Execute();

        SqlStatement stmti = RealmDataDatabase.CreateStatement(stmt_money_ins, "INSERT INTO item_loot_money (container_id, money) VALUES (?,?)");
        stmti.addUInt32(container_id);
        stmti.addUInt32(loot.gold);
        stmti.Execute();
    }

    // Save items
    if (!loot.isLooted())
    {
        static SqlStatementID stmt_items_del;
        SqlStatement stmt_items = RealmDataDatabase.CreateStatement(stmt_items_del, "DELETE FROM item_loot_items WHERE container_id = ?");
        stmt_items.addUInt32(container_id);
        stmt_items.Execute();

        // Now insert the items
        for (LootItemList::const_iterator _li = loot.items.begin(); _li != loot.items.end(); _li++)
        {
            // When an item is looted, it doesn't get removed from the items collection
            //  but we don't want to resave it.
            if (!_li->canSave)
                continue;

            Player* const plr = GetOwner();
            if (!_li->AllowedForPlayer(plr, 2))
                continue;

            static SqlStatementID stmt_items_ins;
            stmt_items = RealmDataDatabase.CreateStatement(stmt_items_ins, "INSERT INTO item_loot_items (container_id, item_id, item_count, ffa, blocked, counted, needs_quest, rnd_prop, rnd_suffix) VALUES (?,?,?,?,?,?,?,?,?)");

            // container_id, item_id, item_count, follow_rules, ffa, blocked, counted, under_threshold, needs_quest, rnd_prop, rnd_suffix
            stmt_items.addUInt32(container_id);
            stmt_items.addUInt32(_li->itemid);
            stmt_items.addUInt32(_li->count);
            stmt_items.addBool(_li->freeforall);
            stmt_items.addBool(_li->is_blocked);
            stmt_items.addBool(_li->is_counted);
            stmt_items.addBool(_li->needs_quest);
            stmt_items.addInt32(_li->randomPropertyId);
            stmt_items.addUInt32(_li->randomSuffix);
            stmt_items.Execute();
        }
    }

    RealmDataDatabase.CommitTransaction();
}

bool Item::ItemContainerLoadLootFromDB()
{
    // Loads the money and item loot associated with an openable item from the DB

    // Default. If there are no records for this item then it will be rolled for in Player::SendLoot()
    m_lootGenerated = false;

    uint32 container_id = GetGUIDLow();

    // Save this for later use
    loot.containerID = container_id;

    // First, see if there was any money loot. This gets added directly to the container.
    QueryResultAutoPtr money_result = RealmDataDatabase.PQuery("SELECT money FROM item_loot_money WHERE container_id = %u", container_id);

    if (money_result)
    {
        Field* fields = money_result->Fetch();
        loot.gold = fields[0].GetUInt32();
    }

    // Next, load any items that were saved
    QueryResultAutoPtr item_result = RealmDataDatabase.PQuery("SELECT item_id, item_count, ffa, blocked, counted, needs_quest, rnd_prop, rnd_suffix FROM item_loot_items WHERE container_id = %u", container_id);
    if (item_result)
    {
        // Get a LootTemplate for the container item. This is where
        //  the saved loot was originally rolled from, we will copy conditions from it
        LootTemplate const* lt = LootTemplates_Item.GetLootfor(GetEntry());

        if (lt)
        {
            do
            {
                // Create an empty LootItem
                LootItem loot_item = LootItem();

                // Fill in the rest of the LootItem from the DB
                Field* fields = item_result->Fetch();

                // item_id, itm_count, ffa, blocked, counted, needs_quest, rnd_prop, rnd_suffix
                loot_item.itemid = fields[0].GetUInt32();
                loot_item.count = fields[1].GetUInt32();
                loot_item.freeforall = fields[2].GetBool();
                loot_item.is_blocked = fields[3].GetBool();
                loot_item.is_counted = fields[4].GetBool();
                loot_item.canSave = true;
                loot_item.needs_quest = fields[5].GetBool();
                loot_item.randomPropertyId = fields[6].GetUInt32();
                loot_item.randomSuffix = fields[7].GetUInt32();

                ItemPrototype const *pProto = sObjectMgr.GetItemPrototype(loot_item.itemid);
                if (!pProto)
                {
                    sLog.outLog(LOG_CRITICAL, "ItemContainerLoadLootFromDB trying to load not existing item %u", loot_item.itemid);
                    continue;
                }

                // Copy the extra loot conditions from the item in the loot template
                lt->CopyConditions(&loot_item);

                // If container item is in a bag, add that player as an allowed looter
                // if (GetBagSlot())
                //    loot_item.allowedGUIDs.insert(GetOwner()->GetGUIDLow());

                // Finally add the LootItem to the container
                loot.items.push_back(loot_item);

                // Increment unlooted count
                loot.unlootedCount++;

            } while (item_result->NextRow());
        }
    }

    // Mark the item if it has loot so it won't be generated again on open
    m_lootGenerated = !loot.isLooted();

    return m_lootGenerated;
}

void Item::ItemContainerDeleteLootItemsFromDB()
{
    // Deletes items associated with an openable item from the DB

    uint32 containerId = GetGUIDLow();
    static SqlStatementID stmt_del_items;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(stmt_del_items, "DELETE FROM item_loot_items WHERE container_id = ?");
    stmt.PExecute(containerId);
}

void Item::ItemContainerDeleteLootItemFromDB(uint32 itemID)
{
    // Deletes a single item associated with an openable item from the DB

    uint32 containerId = GetGUIDLow();
    static SqlStatementID stmt_del_item;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(stmt_del_item, "DELETE FROM item_loot_items WHERE container_id = ? AND item_id = ?");
    stmt.PExecute(containerId, itemID);
}

void Item::ItemContainerDeleteLootMoneyFromDB()
{
    // Deletes the money loot associated with an openable item from the DB

    uint32 containerId = GetGUIDLow();
    static SqlStatementID stmt_del_item_mon;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(stmt_del_item_mon, "DELETE FROM item_loot_money WHERE container_id = ?");
    stmt.PExecute(containerId);
}

void Item::ItemContainerDeleteLootMoneyAndLootItemsFromDB()
{
    // Deletes money and items associated with an openable item from the DB

    ItemContainerDeleteLootMoneyFromDB();
    ItemContainerDeleteLootItemsFromDB();
}