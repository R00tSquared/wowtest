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

#include "SkillExtraItems.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
//#include "ProgressBar.h"
#include "Player.h"
#include "Chat.h"
#include <map>

// some type definitions
// no use putting them in the header file, they're only used in this .cpp

// struct to store information about extra item creation
// one entry for every spell that is able to create an extra item
struct SkillExtraItemEntry
{
    // the spell id of the specialization required to create extra items
    uint32 requiredSpecialization;
    // the chance to create one additional item
    float additionalCreateChance;
    // maximum number of extra items created per crafting
    uint8 additionalMaxNum;

    SkillExtraItemEntry()
        : requiredSpecialization(0), additionalCreateChance(0.0f), additionalMaxNum(0) {}

    SkillExtraItemEntry(uint32 rS, float aCC, uint8 aMN)
        : requiredSpecialization(rS), additionalCreateChance(aCC), additionalMaxNum(aMN) {}
};

// map to store the extra item creation info, the key is the spellId of the creation spell, the mapped value is the assigned SkillExtraItemEntry
typedef std::map<uint32,SkillExtraItemEntry> SkillExtraItemMap;

SkillExtraItemMap SkillExtraItemStore;

// loads the extra item creation info from DB
void LoadSkillExtraItemTable()
{
    uint32 count = 0;

    SkillExtraItemStore.clear();                            // need for reload

    //                                                       0        1                       2                       3
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT spellId, requiredSpecialization, additionalCreateChance, additionalMaxNum FROM skill_extra_item_template");

    if (result)
    {
        //BarGoLink bar(result->GetRowCount());

        do
        {
            Field *fields = result->Fetch();
            //bar.step();

            uint32 spellId = fields[0].GetUInt32();

            if (!GetSpellStore()->LookupEntry<SpellEntry>(spellId))
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Skill specialization %u has non-existent spell id in `skill_extra_item_template`!", spellId);
                continue;
            }

            uint32 requiredSpecialization = fields[1].GetUInt32();
            if (!GetSpellStore()->LookupEntry<SpellEntry>(requiredSpecialization))
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Skill specialization %u have not existed required specialization spell id %u in `skill_extra_item_template`!", spellId,requiredSpecialization);
                continue;
            }

            float additionalCreateChance = fields[2].GetFloat();
            if (additionalCreateChance <= 0.0f)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Skill specialization %u has too low additional create chance in `skill_extra_item_template`!", spellId);
                continue;
            }

            uint8 additionalMaxNum = fields[3].GetUInt8();
            if (!additionalMaxNum)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Skill specialization %u has 0 max number of extra items in `skill_extra_item_template`!", spellId);
                continue;
            }

            SkillExtraItemEntry& skillExtraItemEntry = SkillExtraItemStore[spellId];

            skillExtraItemEntry.requiredSpecialization = requiredSpecialization;
            skillExtraItemEntry.additionalCreateChance = additionalCreateChance;
            skillExtraItemEntry.additionalMaxNum       = additionalMaxNum;

            ++count;
        } while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u spell specialization definitions", count);
    }
    else
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 spell specialization definitions. DB table `skill_extra_item_template` is empty.");
    }
}

bool canCreateExtraItems(Player * player, uint32 spellId, float &additionalChance, uint8 &additionalMax)
{
    // get the info for the specified spell
    SkillExtraItemMap::const_iterator ret = SkillExtraItemStore.find(spellId);
    if (ret==SkillExtraItemStore.end())
        return false;

    SkillExtraItemEntry const* specEntry = &ret->second;

    // if no entry, then no extra items can be created
    if (!specEntry)
        return false;

    // the player doesn't have the required specialization, return false
    if (!player->HasSpell(specEntry->requiredSpecialization))
        return false;

    // Transmutation Master is disabled for x100
    // select * from skill_extra_item_template where requiredSpecialization=28672; 
    if (specEntry->requiredSpecialization == 28672)
    {
        ChatHandler(player).SendSysMessage(16606);
        return false;
    }

    // set the arguments to the appropriate values
    additionalChance = specEntry->additionalCreateChance;
    additionalMax = specEntry->additionalMaxNum;

    // enable extra item creation
    return true;
}

