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

#include "Database/DatabaseEnv.h"
#include "Log.h"
//#include "ProgressBar.h"
#include "ObjectAccessor.h"
#include "World.h"
#include "Util.h"
#include "SkillDiscovery.h"
#include "SpellMgr.h"
#include <map>

struct SkillDiscoveryEntry
{
    uint32  spellId;
    float   chance;

    SkillDiscoveryEntry()
        : spellId(0), chance(0) {}

    SkillDiscoveryEntry(uint16 _spellId, float _chance)
        : spellId(_spellId), chance(_chance) {}
};

typedef std::list<SkillDiscoveryEntry> SkillDiscoveryList;
typedef UNORDERED_MAP<int32, SkillDiscoveryList> SkillDiscoveryMap;

static SkillDiscoveryMap SkillDiscoveryStore;

void LoadSkillDiscoveryTable()
{

    SkillDiscoveryStore.clear();                            // need for reload

    uint32 count = 0;

    //                                                           0           1           2         3
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT s.spellId, s.reqSpell, s.chance, p.season_start_event FROM skill_discovery_template s "
                                                       "LEFT OUTER JOIN progressive_skill_discovery_template p ON s.spellId = p.spellId");

    Season season = sWorld.getSeasonFromDB();

    if (result)
    {
        //BarGoLink bar(result->GetRowCount());

        std::ostringstream ssNonDiscoverableEntries;

        do
        {
            Field *fields = result->Fetch();
            //bar.step();

            uint32 spellId         = fields[0].GetUInt32();
            int32  reqSkillOrSpell = fields[1].GetInt32();
            float  chance          = fields[2].GetFloat();

            int16 SeasonGameEvent = fields[3].GetInt16(); // 200 to 204, or 0 if no info on it - always available
            if (SeasonGameEvent && (SeasonGameEvent < 200 || SeasonGameEvent > 204))
            {
                sLog.outLog(LOG_DB_ERR, "Table `progressive_skill_discovery_template` has spellId (%u) that has wrong season event id (must be 200 to 204), skipped.", spellId);
                continue;
            }

            if (!(SeasonGameEvent == 0 || Season(SeasonGameEvent - 200) <= season))
            {
                // don't load at all spellIds that shouldn't be available in this season
                continue;
            }

            if (chance <= 0)                               // chance
            {
                ssNonDiscoverableEntries << "spellId = " << spellId << " reqSkillOrSpell = " << reqSkillOrSpell << " chance = " << chance << "\n";
                continue;
            }

            if (reqSkillOrSpell > 0)                         // spell case
            {
                SpellEntry const* spellEntry = GetSpellStore()->LookupEntry<SpellEntry>(reqSkillOrSpell);
                if (!spellEntry)
                {
                    sLog.outLog(LOG_DB_ERR, "Spell (ID: %u) have not existed spell (ID: %i) in `reqSpell` field in `skill_discovery_template` table",spellId,reqSkillOrSpell);
                    continue;
                }

                if (spellEntry->Mechanic != MECHANIC_DISCOVERY)
                {
                    sLog.outLog(LOG_DB_ERR, "Spell (ID: %u) not have have MECHANIC_DISCOVERY (28) value in Mechanic field in spell.dbc but listed in `skill_discovery_template` table",spellId);
                    continue;
                }

                SkillDiscoveryStore[reqSkillOrSpell].push_back(SkillDiscoveryEntry(spellId, chance));
            }
            else if (reqSkillOrSpell == 0)                 // skill case
            {
                SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(spellId);
                SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(spellId);

                if (lower==upper)
                {
                    sLog.outLog(LOG_DB_ERR, "Spell (ID: %u) not listed in `SkillLineAbility.dbc` but listed with `reqSpell`=0 in `skill_discovery_template` table",spellId);
                    continue;
                }

                for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
                {
                    SkillDiscoveryStore[-int32(_spell_idx->second->skillId)].push_back(SkillDiscoveryEntry(spellId, chance));
                }
            }
            else
            {
                sLog.outLog(LOG_DB_ERR, "Spell (ID: %u) have negative value in `reqSpell` field in `skill_discovery_template` table",spellId);
                continue;
            }
            ++count;
        } while (result->NextRow());

        sLog.outString();
        sLog.outString(">> Loaded %u skill discovery definitions", count);
        if (!ssNonDiscoverableEntries.str().empty())
            sLog.outLog(LOG_DB_ERR, "Some items can't be successfully discovered: have in chance field value < 0.000001 in `skill_discovery_template` DB table . List:\n%s",ssNonDiscoverableEntries.str().c_str());
    }
    else
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 skill discovery definitions. DB table `skill_discovery_template` is empty.");
    }
}

uint32 GetSkillDiscoverySpell(uint32 skillId, uint32 spellId, Player* player)
{
    // check spell case
    SkillDiscoveryMap::iterator tab = SkillDiscoveryStore.find(spellId);

    if (tab != SkillDiscoveryStore.end())
    {
        for (SkillDiscoveryList::iterator item_iter = tab->second.begin(); item_iter != tab->second.end(); ++item_iter)
        {
            if (roll_chance_f(item_iter->chance * sWorld.getConfig(RATE_SKILL_DISCOVERY))
                && !player->HasSpell(item_iter->spellId))
                return item_iter->spellId;
        }

        // return 0; // Don't return 0 here, try to go for skill-based discovery spells
    }

    // check skill line case
    tab = SkillDiscoveryStore.find(-(int32)skillId);
    if (tab != SkillDiscoveryStore.end())
    {
        for (SkillDiscoveryList::iterator item_iter = tab->second.begin(); item_iter != tab->second.end(); ++item_iter)
        {
            if (roll_chance_f(item_iter->chance * sWorld.getConfig(RATE_SKILL_DISCOVERY))
                && !player->HasSpell(item_iter->spellId))
                return item_iter->spellId;
        }

        // return 0;
    }

    return 0;
}

