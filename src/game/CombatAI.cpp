// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2009 TrinityCore <http://www.trinitycore.org/>
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

#include "CombatAI.h"
#include "SpellMgr.h"

int AggressorAI::Permissible(const Creature *creature)
{
    // have some hostile factions, it will be selected by IsHostileTo check at MoveInLineOfSight
    if (!creature->isCivilian() && !creature->IsNeutralToAll())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

void AggressorAI::UpdateAI(const uint32 /*diff*/)
{
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}

// some day we will delete these useless things
int CombatAI::Permissible(const Creature *creature)
{
    return PERMIT_BASE_NO;
}

void CombatAI::InitializeAI()
{
    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if (me->m_spells[i] && GetSpellStore()->LookupEntry<SpellEntry>(me->m_spells[i]))
            spells.push_back(me->m_spells[i]);

    CreatureAI::InitializeAI();
}

void CombatAI::Reset()
{
    events.Reset();
}

void CombatAI::JustDied(Unit *killer)
{
    for (SpellVct::iterator i = spells.begin(); i != spells.end(); ++i)
        if (AISpellEntry[*i].condition == AICOND_DIE)
            me->CastSpell(killer, *i, true);
}

void CombatAI::EnterCombat(Unit *who)
{
    for (SpellVct::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        if (AISpellEntry[*i].condition == AICOND_AGGRO)
            me->CastSpell(who, *i, false);
        else if (AISpellEntry[*i].condition == AICOND_COMBAT)
            events.ScheduleEvent(*i, AISpellEntry[*i].cooldown + rand()%AISpellEntry[*i].cooldown);
    }
}

void CombatAI::UpdateAI(const uint32 diff)
{
    if (!UpdateVictim())
        return;

    events.Update(diff);

    if (me->HasUnitState(UNIT_STAT_CASTING))
        return;

    if (uint32 spellId = events.ExecuteEvent())
    {
        DoCast(spellId);
        events.ScheduleEvent(spellId, AISpellEntry[spellId].cooldown + rand()%AISpellEntry[spellId].cooldown);
    }
    else
        DoMeleeAttackIfReady();
}


/////////////////
//CasterAI
/////////////////

void CasterAI::InitializeAI()
{
    CombatAI::InitializeAI();

    float m_attackDist = 30.0f;
    for (SpellVct::iterator itr = spells.begin(); itr != spells.end(); ++itr)
        if (AISpellEntry[*itr].condition == AICOND_COMBAT && m_attackDist > GetAISpellEntry(*itr)->maxRange)
            m_attackDist = GetAISpellEntry(*itr)->maxRange;
    if (m_attackDist == 30.0f)
        m_attackDist = MELEE_RANGE;
}

void CasterAI::EnterCombat(Unit *who)
{
    if (spells.empty())
        return;

    uint32 spell = rand()%spells.size();
    uint32 count = 0;
    for (SpellVct::iterator itr = spells.begin(); itr != spells.end(); ++itr, ++count)
    {
        if (AISpellEntry[*itr].condition == AICOND_AGGRO)
            me->CastSpell(who, *itr, false);
        else if (AISpellEntry[*itr].condition == AICOND_COMBAT)
        {
            uint32 cooldown = GetAISpellEntry(*itr)->realCooldown;
            if (count == spell)
            {
                DoCast(spells[spell]);
                cooldown += me->GetCurrentSpellCastTime(*itr);
            }
            events.ScheduleEvent(*itr, cooldown);
        }
    }
}

void CasterAI::UpdateAI(const uint32 diff)
{
    if (!UpdateVictim())
        return;

    events.Update(diff);

    if (me->HasUnitState(UNIT_STAT_CASTING))
        return;

    if (uint32 spellId = events.ExecuteEvent())
    {
        DoCast(spellId);
        uint32 casttime = me->GetCurrentSpellCastTime(spellId);
        events.ScheduleEvent(spellId, (casttime ? casttime : 500) + GetAISpellEntry(spellId)->realCooldown);
    }
}