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

#ifndef HELLGROUND_HOMEMOVEMENTGENERATOR_H
#define HELLGROUND_HOMEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class Creature;

template <class T>
class HELLGROUND_IMPORT_EXPORT HomeMovementGenerator;

template <>
class HELLGROUND_IMPORT_EXPORT HomeMovementGenerator<Creature> : public MovementGeneratorMedium< Creature, HomeMovementGenerator<Creature> >
{
    public:
        HomeMovementGenerator() : arrived(false) {}
        ~HomeMovementGenerator() {}

        void Initialize(Creature &);
        void Finalize(Creature &);
        void Interrupt(Creature &) {}
        void Reset(Creature &);
        bool Update(Creature &, const uint32 &);

        const char* Name() const { return "<Home>"; }
        MovementGeneratorType GetMovementGeneratorType() const { return HOME_MOTION_TYPE; }

    private:
        void _setTargetLocation(Creature &);
        bool arrived;
};

#endif
