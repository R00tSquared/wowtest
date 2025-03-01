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

#include "IdleMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"

IdleMovementGenerator si_idleMovement;

void IdleMovementGenerator::Reset(Unit& /*owner*/)
{
}

void RotateMovementGenerator::Interrupt(Unit& unit)
{
    unit.ClearUnitState(UNIT_STAT_ROTATING);
}

void RotateMovementGenerator::Initialize(Unit& owner)
{
    if (!owner.HasUnitState(UNIT_STAT_NOT_MOVE))
        owner.StopMoving();

    if (owner.GetVictim() && !owner.ToCreature()->hasIgnoreVictimSelection())
        owner.SetInFront(owner.GetVictim());

    owner.addUnitState(UNIT_STAT_ROTATING);
    owner.AttackStop(); 
}

bool RotateMovementGenerator::Update(Unit& owner, const uint32& diff)
{
    float angle = owner.GetOrientation();
    if (m_direction == ROTATE_DIRECTION_LEFT)
    {
        angle += (float)diff * M_PI * 2 / m_maxDuration;
        while (angle >= M_PI * 2) angle -= M_PI * 2;
    }
    else
    {
        angle -= (float)diff * M_PI * 2 / m_maxDuration;
        while (angle < 0) angle += M_PI * 2;
    }

    owner.SetOrientation(angle);
    owner.SetFacingTo(angle);
    // owner.SendHeartBeat(); // this is a hack. we do not have anything correct to send in the beginning

    if (owner.GetTypeId() == TYPEID_UNIT)
    {
        if (owner.IsAIEnabled)
            ((Creature *)&owner)->AI()->MovementInform(ROTATE_MOTION_TYPE, 1);
    }

    if (m_duration > diff)
        m_duration -= diff;
    else
        return false;

    return true;
}

void RotateMovementGenerator::Finalize(Unit &unit)
{
    unit.ClearUnitState(UNIT_STAT_ROTATING);
    if (unit.GetTypeId() == TYPEID_UNIT)
        ((Creature*)&unit)->AI()->MovementInform(ROTATE_MOTION_TYPE, 0);

    unit.AddEvent(new AttackResumeEvent(unit), ATTACK_DISPLAY_DELAY);
}

void DistractMovementGenerator::Initialize(Unit& owner)
{
    owner.addUnitState(UNIT_STAT_DISTRACTED);
}

void DistractMovementGenerator::Finalize(Unit& owner)
{
    owner.ClearUnitState(UNIT_STAT_DISTRACTED);
    owner.AddEvent(new AttackResumeEvent(owner), ATTACK_DISPLAY_DELAY);
}

void DistractMovementGenerator::Reset(Unit& owner)
{
}

void DistractMovementGenerator::Interrupt(Unit& owner)
{
    Finalize(owner); // state is interrupted on MoveTargetedHome() and other
    m_timer = 0; // after any interrupt remove this state as soon as possible
}

bool DistractMovementGenerator::Update(Unit& /*owner*/, const uint32& time_diff)
{
    if(time_diff > m_timer)
        return false;

    m_timer -= time_diff;
    return true;
}

/*void AssistanceDistractMovementGenerator::Finalize(Unit &unit)
{
    unit.ClearUnitState(UNIT_STAT_DISTRACTED);
    ((Creature*)&unit)->SetReactState(REACT_AGGRESSIVE);

    unit.AddEvent(new AttackResumeEvent(unit), ATTACK_DISPLAY_DELAY);
}*/
