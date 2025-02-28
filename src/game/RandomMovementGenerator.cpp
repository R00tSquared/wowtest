// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
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

#include "Creature.h"
#include "MapManager.h"
#include "RandomMovementGenerator.h"
#include "Map.h"
#include "Util.h"

#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

template<>
RandomMovementGenerator<Creature>::RandomMovementGenerator(const Creature & creature) : i_nextMoveTime(0)
{
    float orientUnused;
    creature.GetRespawnCoord(i_x, i_y, i_z, &orientUnused, &i_wanderDistance);
    // TODO - add support for flying mobs using some distance
    i_verticalZ = 0.0f;
}

template<>
void RandomMovementGenerator<Creature>::_setRandomLocation(Creature &creature)
{
	Position dest;
	creature.GetRespawnCoord(dest.x, dest.y, dest.z, &dest.o, &i_wanderDistance);

	if (i_wanderDistance < 0.1f)
		i_wanderDistance = 0.1f;

	const float range = frand(0.1f, i_wanderDistance);
	if (!creature.GetMap()->GetReachableRandomPosition(&creature, dest.x, dest.y, dest.z, range))
		return;

	Movement::MoveSplineInit init(creature);
	init.MoveTo(dest.x, dest.y, dest.z);
	init.SetWalk(true);
	init.Launch();

	if (i_wanderSteps) // Creature has yet to do steps before pausing
	{
		--i_wanderSteps;
		i_nextMoveTime.Reset(urand(1000, 2000));
	}
	else
	{
		// Creature has made all its steps, time for a little break
		i_nextMoveTime.Reset(urand(4, 10) * 1000); // Retails seems to use rounded numbers so we do as well
		i_wanderSteps = urand(0, ((i_wanderDistance <= 1.0f) ? 2 : 8));
	}
	static_cast<MovementGenerator*>(this)->_recalculateTravel = false;
}

template<>
void RandomMovementGenerator<Creature>::Initialize(Creature &creature)
{
    if (!creature.isAlive())
        return;

    creature.addUnitState(UNIT_STAT_ROAMING);
    _setRandomLocation(creature);
}

template<>
void RandomMovementGenerator<Creature>::Reset(Creature &creature)
{
    Initialize(creature);
}

template<>
void RandomMovementGenerator<Creature>::Interrupt(Creature &creature)
{
    creature.ClearUnitState(UNIT_STAT_ROAMING);
    creature.SetWalk(false);
}

template<>
void RandomMovementGenerator<Creature>::Finalize(Creature &creature)
{
    creature.ClearUnitState(UNIT_STAT_ROAMING);
    creature.SetWalk(false);
}

template<>
bool RandomMovementGenerator<Creature>::Update(Creature &creature, const uint32 &diff)
{
    if (creature.IsStopped() || static_cast<MovementGenerator*>(this)->_recalculateTravel)
    {
        if (i_nextMoveTime.Expired(diff) || static_cast<MovementGenerator*>(this)->_recalculateTravel)
            _setRandomLocation(creature);
    }
    return true;
}
