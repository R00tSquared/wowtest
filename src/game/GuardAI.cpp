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

#include "GuardAI.h"
#include "Log.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "World.h"
#include "CreatureAIImpl.h"

int GuardAI::Permissible(const Creature *creature)
{
    if (creature->isGuard())
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

GuardAI::GuardAI(Creature *c) : CreatureAI(c), i_victimGuid(0), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void GuardAI::MoveInLineOfSight(Unit *u)
{
    // Ignore Z for flying creatures
    if (!m_creature->CanFly() && m_creature->GetDistanceZ(u) > CREATURE_Z_ATTACK_RANGE)
        return;

    if (m_creature->canAttack(u) && (u->IsHostileToPlayers() || m_creature->IsHostileTo(u)))
    {
        float attackRadius = m_creature->GetAttackDistance(u);
        if (m_creature->IsWithinDistInMap(u, attackRadius) && u->isInAccessiblePlacefor(m_creature))
        {
            if (!m_creature->GetVictim())
            {
                AttackStart(u);
                //u->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
            }
            else
                m_creature->AddThreat(u, 0.0f);
        }
        
    }
}

void GuardAI::EnterEvadeMode()
{
    if (!m_creature->isAlive())
    {
        debug_log("Creature stopped attacking because he's dead [guid=%u]", m_creature->GetGUIDLow());
        m_creature->GetMotionMaster()->MoveIdle();

        i_state = STATE_NORMAL;

        i_victimGuid = 0;
        m_creature->CombatStop(true);
        m_creature->DeleteThreatList();
        return;
    }

    Unit* pVictim = m_creature->GetMap()->GetUnit(i_victimGuid);

    if (!pVictim)
    {
        debug_log("Creature stopped attacking because victim is non exist [guid=%u]", m_creature->GetGUIDLow());
    }
    else if (!pVictim->isAlive())
    {
        debug_log("Creature stopped attacking because victim is dead [guid=%u]", m_creature->GetGUIDLow());
    }
    else if (pVictim->HasStealthAura())
    {
        debug_log("Creature stopped attacking because victim is using stealth [guid=%u]", m_creature->GetGUIDLow());
    }
    else if (pVictim->IsTaxiFlying())
    {
        debug_log("Creature stopped attacking because victim is flying away [guid=%u]", m_creature->GetGUIDLow());
    }
    else
    {
        debug_log("Creature stopped attacking because victim outran him [guid=%u]", m_creature->GetGUIDLow());
    }

    m_creature->RemoveAllAuras();
    m_creature->DeleteThreatList();
    i_victimGuid = 0;
    m_creature->CombatStop(true);
    i_state = STATE_NORMAL;

    // Remove ChaseMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
    if (me->HasUnitState(UNIT_STAT_CHASE))
        m_creature->GetMotionMaster()->MoveTargetedHome();

    m_creature->UpdateSpeed(MOVE_RUN, true);
}

void GuardAI::UpdateAI(const uint32 /*diff*/)
{
    // update i_victimGuid if m_creature->GetVictim() !=0 and changed
    if (!UpdateVictim())
        return;

    i_victimGuid = m_creature->getVictimGUID();

    DoMeleeAttackIfReady();
}

bool GuardAI::IsVisible(Unit *pl) const
{
    return m_creature->IsWithinDistInMap(pl,sWorld.getConfig(CONFIG_SIGHT_GUARD)) &&
           pl->isVisibleForOrDetect(m_creature, m_creature, true);
}

void GuardAI::JustDied(Unit *killer)
{
    if (Player* pkiller = killer->GetCharmerOrOwnerPlayerOrPlayerItself())
        m_creature->SendZoneUnderAttackMessage(pkiller);
}
