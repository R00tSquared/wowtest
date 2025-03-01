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

#include "ConfusedMovementGenerator.h"

#include "Creature.h"
#include "Player.h"

#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

template <>
void ConfusedMovementGenerator<Player>::Initialize(Player &unit)
{
    _generateMovement(unit);

    unit.InterruptNonMeleeSpells(false);

    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_CONFUSED_MOVING);
}

template <>
void ConfusedMovementGenerator<Creature>::Initialize(Creature &unit)
{
    _generateMovement(unit);

    unit.InterruptNonMeleeSpells(false);

    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_CONFUSED_MOVING);
}

template<class UNIT>
void ConfusedMovementGenerator<UNIT>::Reset(UNIT &u)
{
    Initialize(u);
}

template<class UNIT>
void ConfusedMovementGenerator<UNIT>::Interrupt(UNIT &unit)
{
    unit.StopMoving();
    unit.ClearUnitState(UNIT_STAT_CONFUSED_MOVING);
}

template<class UNIT>
void ConfusedMovementGenerator<UNIT>::_generateMovement(UNIT &unit)
{
    for (uint8 idx = 0; idx < MAX_RANDOM_POINTS; ++idx)
        unit.GetValidPointInAngle(_randomPosition[idx], WANDER_DISTANCE, frand(0, 2*M_PI), true);
}

template<class UNIT>
bool ConfusedMovementGenerator<UNIT>::Update(UNIT &unit, const uint32 &diff)
{
    unit.SetSelection(0);

    if (_nextMoveTime.Expired(diff) || static_cast<MovementGenerator*>(this)->_recalculateTravel)
    {
        uint32 nextMove = urand(0, MAX_RANDOM_POINTS-1);

        Movement::MoveSplineInit init(unit);

        PathFinder path(&unit);
        path.setPathLengthLimit(30.0f);
        bool result = path.calculate(_randomPosition[nextMove].x, _randomPosition[nextMove].y, _randomPosition[nextMove].z);
        if (!result || path.getPathType() & PATHFIND_NOPATH)
            init.MoveTo(_randomPosition[nextMove].x, _randomPosition[nextMove].y, _randomPosition[nextMove].z);
        else
            init.MovebyPath(path.getPath());

        init.SetWalk(true);
        init.Launch();

        static_cast<MovementGenerator*>(this)->_recalculateTravel = false;
        _nextMoveTime.Reset(urand(0, 2000));
    }
    return true;
}

template<>
void ConfusedMovementGenerator<Player>::Finalize(Player& unit)
{
    unit.StopMoving();

    unit.ClearUnitState(UNIT_STAT_CONFUSED_MOVING);
    unit.AddEvent(new AttackResumeEvent(unit), ATTACK_DISPLAY_DELAY);
}

template<>
void ConfusedMovementGenerator<Creature>::Finalize(Creature& unit)
{
    unit.StopMoving();
    // GENSENTODO
    // ship -> logout -> same position
    // using this causing crash with RemoveNotOwnSingleTargetAuras()
    //unit.SelectVictim();

    unit.ClearUnitState(UNIT_STAT_CONFUSED_MOVING);
    unit.AddEvent(new AttackResumeEvent(unit), ATTACK_DISPLAY_DELAY);
}

template void ConfusedMovementGenerator<Player>::Interrupt(Player &);
template void ConfusedMovementGenerator<Creature>::Interrupt(Creature &);
template void ConfusedMovementGenerator<Player>::Reset(Player &);
template void ConfusedMovementGenerator<Creature>::Reset(Creature &);
template bool ConfusedMovementGenerator<Player>::Update(Player &, const uint32 &);
template bool ConfusedMovementGenerator<Creature>::Update(Creature &, const uint32 &);
template void ConfusedMovementGenerator<Player>::Finalize(Player &);
template void ConfusedMovementGenerator<Creature>::Finalize(Creature &);