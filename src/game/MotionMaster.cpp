// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"
#include "ConfusedMovementGenerator.h"
#include "FleeingMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "RandomMovementGenerator.h"
#include "movement/MoveSpline.h"
#include "movement/MoveSplineInit.h"

#include <cassert>

inline bool isStatic(MovementGenerator *mv)
{
    return (mv == &si_idleMovement);
}

void MotionMaster::Initialize()
{
    // stop current move
    m_owner->StopMoving();
    m_owner->SendCombatStats(1 << COMBAT_STATS_CRASHTEST, "mm init", NULL);

    // set new default movement generator
    if (Creature* creature = m_owner->ToCreature())
    {
        MovementGenerator* movement = FactorySelector::selectMovementGenerator(creature);
        if (movement)
            Mutate(movement, UNIT_ACTION_DOWAYPOINTS);
    }
}

MotionMaster::~MotionMaster()
{
}

void MotionMaster::MoveIdle()
{
    impl()->DropAllStates();
}

void MotionMaster::MoveRandomAroundPoint(float x, float y, float z, float radius, float verticalZ)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster: Player %u attempt to move random.", m_owner->GetGUIDLow());
    else
        Mutate(new RandomMovementGenerator<Creature>(x, y, z, radius, verticalZ), UNIT_ACTION_DOWAYPOINTS);
}

void MotionMaster::MoveTargetedHome()
{
    m_owner->GetUnitStateMgr().DropActionHigherThen(UNIT_ACTION_PRIORITY_HOME);

    if (m_owner->GetTypeId() == TYPEID_UNIT && !((Creature*)m_owner)->GetCharmerOrOwnerGUID())
    {
        Mutate(new HomeMovementGenerator<Creature>(), UNIT_ACTION_HOME);
    }
    else if (m_owner->GetTypeId() == TYPEID_UNIT && ((Creature*)m_owner)->GetCharmerOrOwnerGUID())
    {
        // Possessed creatures shouldn't have FollowMovegen assigned :P
        if (m_owner->HasUnitState(UNIT_STAT_POSSESSED))
            return;

        if (Unit *target = ((Creature*)m_owner)->GetCharmerOrOwner())
            Mutate(new FollowMovementGenerator<Creature>(*target,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE), UNIT_ACTION_HOME);
    }
    else
        sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster: LowGUID: %u attempt targeted home", m_owner->GetGUIDLow());
}

void MotionMaster::MoveConfused()
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new ConfusedMovementGenerator<Player>(), UNIT_ACTION_CONFUSED);
    else
        Mutate(new ConfusedMovementGenerator<Creature>(), UNIT_ACTION_CONFUSED);
}

// dist 0 by default. Sometimes uses MELEE_RANGE. Sometimes dist is big enough (ranged NPCs)
void MotionMaster::MoveChase(Unit* target, float dist, float angle)
{
    // ignore movement request if target not exist
    if (!target || m_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new ChaseMovementGenerator<Player>(*target,dist,angle), UNIT_ACTION_CHASE);
    else
        Mutate(new ChaseMovementGenerator<Creature>(*target,dist,angle), UNIT_ACTION_CHASE);
}

// dist is mostly 0-2. PET follow dist is 1. Sometimes scripts use really big dist
void MotionMaster::MoveFollow(Unit* target, float dist, float angle)
{
    // ignore movement request if target not exist
    if (!target || m_owner->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return;

    m_owner->GetMotionMaster()->StopControlledMovement();

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new FollowMovementGenerator<Player>(*target,dist,angle), UNIT_ACTION_DOWAYPOINTS);
    else
        Mutate(new FollowMovementGenerator<Creature>(*target, dist, angle), UNIT_ACTION_DOWAYPOINTS);
}

void MotionMaster::MovePoint(uint32 id, float x, float y, float z, bool generatePath, bool callStop, UnitActionId actionId /*= UNIT_ACTION_ASSISTANCE*/)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new PointMovementGenerator<Player>(id,x,y,z, generatePath, callStop), actionId);
    else
        Mutate(new PointMovementGenerator<Creature>(id,x,y,z, generatePath, callStop), actionId);
}

void MotionMaster::MoveSeekAssistance(float x, float y, float z, float lengthLimit)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster: %s attempt to seek assistance", m_owner->GetGuidStr().c_str());
    else
        Mutate(new AssistanceMovementGenerator(x,y,z,lengthLimit), UNIT_ACTION_ASSISTANCE);
}

void MotionMaster::MoveSeekAssistanceDistract(uint32 time)
{
    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster: %s attempt to call distract after assistance", m_owner->GetGuidStr().c_str());
    else
        Mutate(new AssistanceDistractMovementGenerator(time), UNIT_ACTION_ASSISTANCE);
}

void MotionMaster::MoveFleeing(Unit* enemy, uint32 time)
{
    if (!enemy)
        return;

    if (m_owner->GetTypeId() == TYPEID_PLAYER)
        Mutate(new FleeingMovementGenerator<Player>(enemy->GetObjectGuid()), UNIT_ACTION_FEARED);
    else
    {
        if (time)
            Mutate(new TimedFleeingMovementGenerator(enemy->GetObjectGuid(), time), UNIT_ACTION_FEARED);
        else
            Mutate(new FleeingMovementGenerator<Creature>(enemy->GetObjectGuid()), UNIT_ACTION_FEARED);
    }
}

void MotionMaster::MovePath(uint32 path_id, bool repeatable)
{
    if (m_owner->GetTypeId() == TYPEID_UNIT)
    {
        if (GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster: Creature %s (Entry %u) attempt to MoveWaypoint() but creature is already using waypoint", m_owner->GetGuidStr().c_str(), m_owner->GetEntry());
            return;
        }

        Creature* creature = (Creature*)m_owner;

        Mutate(new WaypointMovementGenerator<Creature>(path_id, repeatable), UNIT_ACTION_DOWAYPOINTS);
    }
    else
        sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster: Non-creature %s attempt to MoveWaypoint()", m_owner->GetGuidStr().c_str());
}

void MotionMaster::MoveRotate(uint32 time, RotateDirection direction)
{
    if (!time)
        return;
    Mutate(new RotateMovementGenerator(time, direction), UNIT_ACTION_CONTROLLED);
}

void MotionMaster::MoveDistract(uint32 timer)
{
    Mutate(new DistractMovementGenerator(timer), UNIT_ACTION_DISTRACT);
}

void MotionMaster::Mutate(MovementGenerator* mgen, UnitActionId stateId)
{
    impl()->PushAction(stateId, mgen);
}

void MotionMaster::propagateSpeedChange()
{
    impl()->CurrentAction()->UnitSpeedChanged();
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
    return const_cast<MotionMaster*>(this)->top()->GetMovementGeneratorType();
}

bool MotionMaster::GetDestination(float &x, float &y, float &z)
{
    if (m_owner->movespline->Finalized())
        return false;

    const G3D::Vector3& dest = m_owner->movespline->FinalDestination();
    x = dest.x;
    y = dest.y;
    z = dest.z;
    return true;
}

MotionMaster::MotionMaster(Unit *unit) : m_owner(unit)
{
}

/** Drops all states */
void MotionMaster::Clear(bool reset /*= true*/, bool all /*= false*/)
{
    if (all)
        impl()->InitDefaults();
    else if (reset)
        impl()->DropAllStates();
}

void MotionMaster::StopControlledMovement()
{
    impl()->DropAllControlledStates();
}

MovementGenerator* MotionMaster::top()
{
    UnitActionPtr mgen = impl()->CurrentAction();
    if (!mgen)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster::top() %s has empty states list!", m_owner->GetGuidStr().c_str());
        return &si_idleMovement;
    }
    return ((MovementGenerator*)&*mgen);
}

UnitStateMgr* MotionMaster::impl()
{
    ASSERT(m_owner);
    UnitStateMgr* mgr = &m_owner->GetUnitStateMgr();
    ASSERT(mgr);
    return mgr;
}

bool MotionMaster::empty()
{
    MovementGenerator* mgen = top();
    if (isStatic(mgen))
        return true;

    return (mgen->GetMovementGeneratorType() == IDLE_MOTION_TYPE);
}

void MotionMaster::MoveFall()
{
    // use larger distance for vmap height search than in most other cases
    float tz = m_owner->GetTerrain()->GetHeight(m_owner->GetPositionX(), m_owner->GetPositionY(), m_owner->GetPositionZ(), true, MAX_FALL_DISTANCE);
    if (tz <= INVALID_HEIGHT)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MotionMaster::MoveFall: unable retrive a proper height at map %u (x: %f, y: %f, z: %f).",
            m_owner->GetMap()->GetId(), m_owner->GetPositionX(), m_owner->GetPositionX(), m_owner->GetPositionZ());
        return;
    }

    // Abort too if the ground is very near
    if (fabs(m_owner->GetPositionZ() - tz) < 0.1f)
        return;

    Mutate(new EffectMovementGenerator(0), UNIT_ACTION_EFFECT);
    Movement::MoveSplineInit init(*m_owner);
    init.MoveTo(m_owner->GetPositionX(),m_owner->GetPositionY(),tz);
    init.SetFall();
    init.Launch();
}

void MotionMaster::MoveCharge(float x, float y, float z, float speed /*= SPEED_CHARGE*/)
{
    Mutate(new EffectMovementGenerator(EVENT_CHARGE), UNIT_ACTION_EFFECT);
    Movement::MoveSplineInit init(*m_owner);
    init.MoveTo(x,y,z);
    init.SetVelocity(speed);
    init.Launch();
}

void MotionMaster::MoveCharge(PathFinder path, float speed /*= SPEED_CHARGE*/)
{
    Mutate(new EffectMovementGenerator(EVENT_CHARGE), UNIT_ACTION_EFFECT);
    Movement::MoveSplineInit init(*m_owner);
    init.MovebyPath(path.getPath());
    init.SetVelocity(speed);
    init.Launch();
}
