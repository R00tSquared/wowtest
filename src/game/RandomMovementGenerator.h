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

#ifndef HELLGROUND_RANDOMMOVEMENTGENERATOR_H
#define HELLGROUND_RANDOMMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class RandomMovementGenerator : public MovementGeneratorMedium< T, RandomMovementGenerator<T> >
{
    public:
        explicit RandomMovementGenerator(const Creature &) :
			i_nextMoveTime(0), i_wanderDistance(5.0f), i_wanderSteps(0) {}
        explicit RandomMovementGenerator(float x, float y, float z, float radius, float verticalZ = 0.0f) :
            i_nextMoveTime(0), i_wanderDistance(radius), i_wanderSteps(0), i_x(x), i_y(y), i_z(z), i_verticalZ(verticalZ) {}

        void _setRandomLocation(T &);
        void Initialize(T &);
        void Finalize(T &);
        void Interrupt(T &);
        void Reset(T &);
        bool Update(T &, const uint32 &);

        const char* Name() const { return "<Random>"; }
        MovementGeneratorType GetMovementGeneratorType() const { return RANDOM_MOTION_TYPE; }

    private:
        TimeTrackerSmall i_nextMoveTime;
        float i_x, i_y, i_z;
        float i_verticalZ;

		float i_wanderDistance;
		uint8 i_wanderSteps;
};

#endif
