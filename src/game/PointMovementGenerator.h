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

#ifndef HELLGROUND_POINTMOVEMENTGENERATOR_H
#define HELLGROUND_POINTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "FollowerReference.h"

class Creature;

template<class UNIT>
class PointMovementGenerator : public MovementGeneratorMedium< UNIT, PointMovementGenerator<UNIT> >
{
    friend class AssistanceMovementGenerator;
    public:
        PointMovementGenerator(uint32 id, float x, float y, float z, bool generatePath = false, bool callStop = true) : _id(id),
            _x(x), _y(y), _z(z), _generatePath(generatePath), m_callStopMove( callStop ) {}

        virtual void Initialize(UNIT &);
        virtual void Finalize(UNIT &);
        void Interrupt(UNIT &);
        void Reset(UNIT &);
        bool Update(UNIT &, const uint32 &);

        void GetDestination(float &x, float &y, float &z) { x = _x; y = _y; z = _z; }

        const char* Name() const { return "<Point>"; }
        MovementGeneratorType GetMovementGeneratorType() const { return POINT_MOTION_TYPE; }

    private:
        uint32 _id;
        float _x, _y, _z;
        bool _generatePath;
        bool m_callStopMove;
};

class AssistanceMovementGenerator : public PointMovementGenerator<Creature>
{
    public:
        AssistanceMovementGenerator(float _x, float _y, float _z, float pathLimit) :
            PointMovementGenerator<Creature>(0, _x, _y, _z, true), _pathLengthLimit(pathLimit){}

        const char* Name() const { return "<Assistance>"; }
        MovementGeneratorType GetMovementGeneratorType() const { return ASSISTANCE_MOTION_TYPE; }
        void Initialize(Creature &);
        void Finalize(Creature &);

    private:
        float _pathLengthLimit;
};

// Does almost nothing - just doesn't allows previous movegen interrupt current effect.
class HELLGROUND_IMPORT_EXPORT EffectMovementGenerator : public MovementGenerator
{
    public:
        explicit EffectMovementGenerator(uint32 Id) : m_Id(Id) {}
        void Initialize(Unit &);
        void Finalize(Unit &);
        void Interrupt(Unit &u);
        void Reset(Unit &) {}
        bool Update(Unit &, const uint32 &);

        const char* Name() const { return "<Effect>"; }
        MovementGeneratorType GetMovementGeneratorType() const { return EFFECT_MOTION_TYPE; }

        uint32 EffectId() const { return m_Id; }

    private:
        uint32 m_Id;
};

#endif
