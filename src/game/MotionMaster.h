/*
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
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

#ifndef HELLGROUND_MOTIONMASTER_H
#define HELLGROUND_MOTIONMASTER_H

#include "Common.h"
#include "SharedDefines.h"
#include "StateMgrImpl.h"

#include <stack>
#include <vector>

class PathFinder;
class MovementGenerator;
class Unit;

enum RotateDirection
{
    ROTATE_DIRECTION_LEFT,
    ROTATE_DIRECTION_RIGHT,
};

// Creature Entry ID used for way points show, visible only for GMs
#define VISUAL_WAYPOINT 1

#define DEFAULT_WALK_SPEED 24.0f
#define SPEED_CHARGE       42.0f

class HELLGROUND_IMPORT_EXPORT MotionMaster 
{
    public:

        explicit MotionMaster(Unit *unit);
        ~MotionMaster();

        void Initialize();
        void Clear(bool reset = true, bool all = false); // if reset true and all false - same as MoveIdle() - drops all states
        void MovementExpired() { Clear(); } // drops ALL states! just like MoveIdle();
        void StopControlledMovement();

        void MoveIdle();
        void MoveRandomAroundPoint(float x, float y, float z, float radius, float verticalZ = 0.0f);
        void MoveTargetedHome();
        void MoveFollow(Unit* target, float dist, float angle);
        void MoveChase(Unit* target, float dist = 0.0f, float angle = 0.0f);
        void MoveConfused();
        void MoveFleeing(Unit* enemy, uint32 timeLimit = 0);
        void MovePoint(uint32 id, float x,float y,float z, bool generatePath = true, bool callStop = true, UnitActionId actionId = UNIT_ACTION_ASSISTANCE);
        void MoveCharge(float x, float y, float z, float speed = SPEED_CHARGE);
        void MoveCharge(PathFinder path, float speed = SPEED_CHARGE);
        void MoveSeekAssistance(float x,float y,float z,float lengthLimit);
        void MoveSeekAssistanceDistract(uint32 timer);
        void MovePath(uint32 path_id, bool repeatable);
        void MoveRotate(uint32 time, RotateDirection direction);
        void MoveDistract(uint32 timeLimit);
        void MoveFall();

        MovementGeneratorType GetCurrentMovementGeneratorType() const;

        void propagateSpeedChange();

        class UnitStateMgr* impl();
        MovementGenerator*  top();
        Unit* GetOwner() { return m_owner; };
        bool  empty();

        bool GetDestination(float &x, float &y, float &z);

    private:
        void Mutate(MovementGenerator* mgen, UnitActionId slot);                  // use Move* functions instead

        Unit*       m_owner;
        uint8       m_cleanFlag;
};

#endif
