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

#include "TemporarySummon.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"

TemporarySummon::TemporarySummon(uint64 summoner) :
Creature(), m_type(TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN), m_timer(0), m_lifetime(0), m_summoner(summoner), DieWithSummoner(false)
{
     m_tempSummon = true;
}

void TemporarySummon::Update(uint32 update_diff, uint32 diff)
{
    if (m_deathState == DEAD)
    {
        UnSummon();
        return;
    }

	Creature *creature = ToCreature();

    switch (m_type)
    {
        case TEMPSUMMON_MANUAL_DESPAWN:
            break;
        case TEMPSUMMON_TIMED_DESPAWN:
        {
            if (m_timer <= update_diff)
            {
                UnSummon();
                return;
            }

            m_timer -= update_diff;
            break;
        }
        case TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT:
        {
            if (!IsInCombat())
            {	
				if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }

				if (m_deathState == CORPSE && creature->loot.unlootedCount > 0)
					return;

                m_timer -= update_diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;

            break;
        }

        case TEMPSUMMON_CORPSE_TIMED_DESPAWN:
        {
            if (m_deathState == CORPSE)
            {
                if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= update_diff;
            }
            break;
        }
        case TEMPSUMMON_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            break;
        }
        case TEMPSUMMON_DEAD_DESPAWN:
        {
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!IsInCombat())
            {
                if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= update_diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        case TEMPSUMMON_TIMED_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!IsInCombat() && isAlive())
            {
                if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= update_diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        default:
            UnSummon();
            sLog.outLog(LOG_DEFAULT, "ERROR: Temporary summoned creature (entry: %u) have unknown type %u of ",GetEntry(),m_type);
            return;
    }

    if (DieWithSummoner)
    {
        if (Unit* summoner = GetSummoner())
        {
            if (summoner->isDead() && m_type != TEMPSUMMON_MANUAL_DESPAWN && summoner->GetTypeId() == TYPEID_UNIT)
            {
                UnSummon();
                return;
            }
        }
    }

    Creature::Update(update_diff, diff);
}

void TemporarySummon::Summon(TemporarySummonType type, uint32 lifetime)
{
    m_type = type;
    m_timer = lifetime;
    m_lifetime = lifetime;

    GetMap()->Add((Creature*)this);
}

void TemporarySummon::UnSummon()
{
    AddObjectToRemoveList();

    Unit* sum = m_summoner ? GetMap()->GetUnit(m_summoner) : NULL;
    if (sum  && sum->GetTypeId() == TYPEID_UNIT && ((Creature*)sum)->IsAIEnabled)
    {
        ((Creature*)sum)->AI()->SummonedCreatureDespawn(this);
    }
}
