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

#include <stdlib.h>
#include <functional>
#include "ItemEnchantmentMgr.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
//#include "ProgressBar.h"
#include <list>
#include <vector>
#include "Util.h"

struct EnchStoreItem
{
    uint32  ench;
    float   chance;

    EnchStoreItem()
        : ench(0), chance(0) {}

    EnchStoreItem(uint32 _ench, float _chance)
        : ench(_ench), chance(_chance) {}
};

typedef std::vector<EnchStoreItem> EnchStoreList;
typedef UNORDERED_MAP<uint32, EnchStoreList> EnchantmentStore;

static EnchantmentStore RandomItemEnch;

void LoadRandomEnchantmentsTable()
{
    RandomItemEnch.clear();                                 // for reload case

    EnchantmentStore::iterator tab;
    uint32 entry, ench;
    float chance;
    uint32 count = 0;

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, ench, chance FROM item_enchantment_template");

    if (result)
    {
        //BarGoLink bar(result->GetRowCount());

        do
        {
            Field *fields = result->Fetch();
            //bar.step();

            entry = fields[0].GetUInt32();
            ench = fields[1].GetUInt32();
            chance = fields[2].GetFloat();

            if (chance > 0.000001f && chance <= 100.0f)
                RandomItemEnch[entry].push_back(EnchStoreItem(ench, chance));

            ++count;
        } while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u Item Enchantment definitions", count);
    }
    else
    {
        sLog.outString();
        sLog.outLog(LOG_DB_ERR, ">> Loaded 0 Item Enchantment definitions. DB table `item_enchantment_template` is empty.");
    }
}

uint32 GetItemEnchantMod(uint32 entry)
{
    if (!entry) return 0;

    EnchantmentStore::iterator tab = RandomItemEnch.find(entry);

    if (tab == RandomItemEnch.end())
    {
        sLog.outLog(LOG_DB_ERR, "Item RandomProperty / RandomSuffix id #%u used in `item_template` but it doesn't have records in `item_enchantment_template` table.",entry);
        return 0;
    }

    double dRoll = rand_chance();
    float fCount = 0;

    for (EnchStoreList::iterator ench_iter = tab->second.begin(); ench_iter != tab->second.end(); ++ench_iter)
    {
        fCount += ench_iter->chance;

        if (fCount > dRoll) return ench_iter->ench;
    }

    //we could get here only if sum of all enchantment chances is lower than 100%
    dRoll =  (irand(0, (int)floor(fCount * 100) + 1)) / 100;
    fCount = 0;

    for (EnchStoreList::iterator ench_iter = tab->second.begin(); ench_iter != tab->second.end(); ++ench_iter)
    {
        fCount += ench_iter->chance;

        if (fCount > dRoll) return ench_iter->ench;
    }

    return 0;
}

uint32 GenerateEnchSuffixFactor(uint32 item_id)
{
    ItemPrototype const *itemProto = ObjectMgr::GetItemPrototype(item_id);

    if (!itemProto)
        return 0;
    if (!itemProto->RandomSuffix)
        return 0;

    RandomPropertiesPointsEntry const *randomProperty = sRandomPropertiesPointsStore.LookupEntry(itemProto->ItemLevel);
    if (!randomProperty)
        return 0;

    uint32 suffixFactor;
    switch (itemProto->InventoryType)
    {
        // Items of that type don`t have points
        case INVTYPE_NON_EQUIP:
        case INVTYPE_BAG:
        case INVTYPE_TABARD:
        case INVTYPE_AMMO:
        case INVTYPE_QUIVER:
        case INVTYPE_RELIC:
            return 0;
            // Select point coefficient
        case INVTYPE_HEAD:
        case INVTYPE_BODY:
        case INVTYPE_CHEST:
        case INVTYPE_LEGS:
        case INVTYPE_2HWEAPON:
        case INVTYPE_ROBE:
            suffixFactor = 0;
            break;
        case INVTYPE_SHOULDERS:
        case INVTYPE_WAIST:
        case INVTYPE_FEET:
        case INVTYPE_HANDS:
        case INVTYPE_TRINKET:
            suffixFactor = 1;
            break;
        case INVTYPE_NECK:
        case INVTYPE_WRISTS:
        case INVTYPE_FINGER:
        case INVTYPE_SHIELD:
        case INVTYPE_CLOAK:
        case INVTYPE_HOLDABLE:
            suffixFactor = 2;
            break;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPONOFFHAND:
            suffixFactor = 3;
            break;
        case INVTYPE_RANGED:
        case INVTYPE_THROWN:
        case INVTYPE_RANGEDRIGHT:
            suffixFactor = 4;
            break;
        default:
            return 0;
    }
    // Select rare/epic modifier
    switch (itemProto->Quality)
    {
        case ITEM_QUALITY_UNCOMMON:
            return randomProperty->UncommonPropertiesPoints[suffixFactor];
        case ITEM_QUALITY_RARE:
            return randomProperty->RarePropertiesPoints[suffixFactor];
        case ITEM_QUALITY_EPIC:
            return randomProperty->EpicPropertiesPoints[suffixFactor];
        case ITEM_QUALITY_LEGENDARY:
        case ITEM_QUALITY_ARTIFACT:
            return 0;                                       // not have random properties
        default:
            break;
    }
    return 0;
}

