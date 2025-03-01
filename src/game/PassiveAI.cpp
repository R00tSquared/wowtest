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

#include "PassiveAI.h"
#include "Creature.h"
#include "TemporarySummon.h"

PassiveAI::PassiveAI(Creature *c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }
PossessedAI::PossessedAI(Creature *c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE); }
NullCreatureAI::NullCreatureAI(Creature *c) : CreatureAI(c) { me->SetReactState(REACT_PASSIVE);}

void PassiveAI::UpdateAI(const uint32)
{
    if (me->IsInCombat() && me->GetAttackers().empty())
        EnterEvadeMode();
}

void PossessedAI::AttackStart(Unit *target)
{
    if (target && me->Attack(target, true))
        me->GetMotionMaster()->MoveChase(target);
}

void PossessedAI::UpdateAI(const uint32 diff)
{
    if (me->GetVictim())
    {
        if (!me->canAttack(me->GetVictim()))
            me->AttackStop();
        else
            DoMeleeAttackIfReady();
    }
}

void PossessedAI::JustDied(Unit *u)
{
    // We died while possessed, disable our loot
    me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

void PossessedAI::KilledUnit(Unit* victim)
{
    // We killed a creature, disable victim's loot
    if (victim->GetTypeId() == TYPEID_UNIT)
        victim->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
}

void CritterAI::DamageTaken(Unit *done_by, uint32 &)
{
    if (!me->HasUnitState(UNIT_STAT_FLEEING))
        me->SetFeared(true, done_by, 10000);
}

void CritterAI::EnterEvadeMode()
{
    if (me->HasUnitState(UNIT_STAT_FLEEING))
        me->SetFeared(false, NULL);

    CreatureAI::EnterEvadeMode();
}

void TriggerAI::IsSummonedBy(Unit *summoner)
{
    if (me->m_spells[0])
        me->CastSpell(me, me->m_spells[0], false, 0, 0, summoner->GetGUID());
}
