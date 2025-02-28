/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2017 Hellground <http://wow-hellground.com/>
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

#include "ByteBuffer.h"
#include "TargetedMovementGenerator.h"
#include "Log.h"
#include "Player.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "World.h"

#include "Spell.h"

#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

#include <cmath>

#define EVADE_TIMER 7 * MILLISECONDS
#define RECHECK_TIMER 100 // should not be greater or it will cause chase-attack-bugs
#define RECALCULATE_RANGE 1 // just use default value
#define RECALCULATE_RANGE_PET 0.4 // make it smooth

template<class T, typename D>
float TargetedMovementGeneratorMedium<T, D>::GetVelocitySpeed(T& owner)
{
	if (owner.IsInCombat() || !_target.isValid())
		return 0;

	// Do not allow speed boosting when in pvp instances
	if (const MapEntry* map = sMapStore.LookupEntry(owner.GetMapId()))
		if (map->IsBattleGroundOrArena())
			return 0;

	if (_target->HasInArc(M_PI, owner.ToUnit()) != !_target->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_BACKWARD)))
		return 0;

	const UnitMoveType type = _target->m_movementInfo.GetSpeedType();
	float speed = owner.GetSpeed(type);

	const float bonus = (_target->GetDistance(owner.GetPositionX(), owner.GetPositionY(), owner.GetPositionZ()) / speed);
	return std::max(owner.GetSpeed(type), std::min((speed + bonus), 40.0f));
}

template<class T, typename D>
void TargetedMovementGeneratorMedium<T, D>::_setTargetLocation(T &owner)
{
	if (!_target.isValid() || !_target->IsInWorld())
		return;

	float x, y, z;
	bool targetIsVictim = owner.getVictimGUID() == _target->GetGUID();

	// for debug
	uint32 debug_owner_guid = owner.GetGUIDLow();
	uint32 debug_owner_entry = owner.GetEntry();
	uint32 debug_target_guid = _target->ToUnit()->GetGUIDLow();
	uint32 debug_target_entry = _target->ToUnit()->GetEntry();

	bool evade = false;
	if (owner.GetObjectGuid().IsCreature())
	{
		Creature* creature = owner.ToCreature();
		if (creature && creature->IsInEvadeMode())
			return;

		if (targetIsVictim && _target->isCharmedOwnedByPlayerOrPlayer())
		{
			Unit* unitTarget = _target->ToUnit();

			// if only one target -> evade
			if (!creature->GetMap()->IsDungeon() && !creature->inRangedAttackMode() && _target->isFlying() && creature->getThreatManager().getThreatList().size() == 1 && creature->GetDistanceZ(unitTarget) > CREATURE_Z_ATTACK_RANGE)
				evade = true;

			// start counting timer otherwise
			if (!IsReachable() && !CantReachEvadeTimer)
			{
				CantReachEvadeTimer = 1; //start counting if need
				sLog.DEBUG("--- CantReachEvadeTimer is set %u %s -> target: %u %s", creature->GetGUIDLow(), creature->GetName(), unitTarget->GetGUIDLow(), unitTarget->GetName());
			}

			if (evade)
			{
				CantReachEvadeTimer = EVADE_TIMER; //instant evade
				sLog.DEBUG("--- CantReachEvadeTimer set EVADE_TIMER %u %s -> target: %u %s", creature->GetGUIDLow(), creature->GetName(), unitTarget->GetGUIDLow(), unitTarget->GetName());
			}
		}
	}

	if (owner.GetObjectGuid().IsPet() && this->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
	{
		Pet* pet = owner.ToPet();
		float oreintation = _target->GetOrientation();
		float size = owner.GetObjectSize();

		// some summons are bugged
		if (size > 3)
			size = 3;

		if (pet->getPetType() == MINI_PET)
		{
			size = 0.1;
			oreintation = std::fmod(1.14f + oreintation, M_PI * 2);
		}

		if (pet->getPetType() == GUARDIAN_PET)
			oreintation = std::fmod(M_PI / 2 + oreintation, M_PI * 2);

		_target->GetNearPoint(x, y, z, size, _offset, _angle + abs(oreintation));
	}
	else if (_offset && _target->IsWithinDistInMap(&owner, 2 * _offset))
	{
		if (!owner.IsStopped())
			return;

		owner.GetPosition(x, y, z);
	}
	else if (!_offset)
	{
		if (_target->IsWithinMeleeRange(&owner, MELEE_RANGE - 0.5f))
		{
			if (!owner.IsStopped())
				owner.StopMoving();

			return;
		}

		// this should prevent weird behavior on tight spaces like lines between columns and bridge on BEM
		if (Pet* pet = owner.ToPet())
			_target->GetPosition(x, y, z);
		else if (!_target->isFlying())
			// to nearest random contact position
			_target->GetRandomContactPoint(&owner, x, y, z, 0, MELEE_RANGE - 0.5f);
	}
	else
	{
		if (_target->IsWithinDistInMap(&owner, _offset))
		{
			if (!owner.IsStopped())
				owner.StopMoving();

			return;
		}

		// to at _offset distance from target and _angle from target facing
		_target->GetNearPoint(x, y, z, owner.GetObjectSize(), _offset, _angle);
	}

	if (abs(_target->GetPositionZ() - z) > 5.0f) // get nearpoint is normalizing position for ground, enable fly and swim
		z = _target->GetPositionZ();

	if (!_offset && targetIsVictim && (!owner.CanReachWithMeleeAutoAttackAtPosition(_target.getTarget(), x, y, z) || !_target->IsWithinLOS(x, y, z)))
		_target->GetPosition(x, y, z);

	if (!_path)
		_path = new PathFinder(&owner);

	// allow pets following their master to cheat while generating paths
	bool forceDest = (owner.GetObjectGuid().IsPet() && owner.HasUnitState(UNIT_STAT_FOLLOW));

	//if (owner.GetMapId() == 556)
	//sLog.outLog(LOG_TMP, "TargetedMovementGeneratorMedium: Creature %s (entry %u, guid %u) moved to XYZ %f %f %f, pathtype %u, creature map %u",
	//	owner.GetName(), owner.GetEntry(), owner.GetGUIDLow(), x, y, z, _path->getPathType(), owner.GetMapId());

	bool result = _path->calculate(x, y, z, forceDest);
	//if (!result || _path->getPathType() & PATHFIND_NOPATH)
	//    return;

	// why we should recalculate it?
	// causes taxture bugs in Karazhan and etc
	// .go xyz -11161 -1875 117 532
	//if (!forceDest && _path->getPathType() & PATHFIND_NOPATH)
	//	result = _path->calculate(x, y, z, true);
	//if (!result)
	//	return;

	_targetReached = false;
	static_cast<MovementGenerator*>(this)->_recalculateTravel = false;
	_target->GetPosition(m_fTargetLastX, m_fTargetLastY, m_fTargetLastZ);

	Movement::MoveSplineInit init(owner);
	init.MovebyPath(_path->getPath());
	init.SetWalk(((D*)this)->EnableWalking(owner));

	// GENSENTODO later...
	//if (float new_speed = GetVelocitySpeed(owner))
	//	init.SetVelocity(new_speed);

	init.Launch();
}

template<>
void TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::UpdateFinalDistance(float /*fDistance*/)
{
	// nothing to do for Player
}

template<>
void TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::UpdateFinalDistance(float /*fDistance*/)
{
	// nothing to do for Player
}

template<>
void TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::UpdateFinalDistance(float fDistance)
{
	_offset = fDistance;
	static_cast<MovementGenerator*>(this)->_recalculateTravel = true;
}

template<>
void TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::UpdateFinalDistance(float fDistance)
{
	_offset = fDistance;
	static_cast<MovementGenerator*>(this)->_recalculateTravel = true;
}

template<class T, typename D>
bool TargetedMovementGeneratorMedium<T, D>::Update(T &owner, const uint32 & time_diff)
{
	if (!_target.isValid() || !_target->IsInWorld())
		return false;

	if (!owner.isAlive())
		return true;

	// prevent crash after creature killed pet
	if (static_cast<D*>(this)->_lostTarget(owner))
		return true;

	_recheckTimer.Update(time_diff);
	if (_recheckTimer.Passed())
	{
		bool targetMoved = false;
		float recalculateRange = RECALCULATE_RANGE;
		_recheckTimer.Reset(RECHECK_TIMER);

		// move pets at following at the same time when owner moved (detected)
		if (owner.GetObjectGuid().IsPet() && this->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
			targetMoved = _target->m_movementInfo.HasMovementFlag(MovementFlags(MOVEFLAG_MASK_MOVING_FORWARD | MOVEFLAG_BACKWARD | MOVEFLAG_PITCH_UP | MOVEFLAG_PITCH_DOWN));

		if (!targetMoved)
		{
			targetMoved = !_target->IsWithinDist3d(m_fTargetLastX, m_fTargetLastY, m_fTargetLastZ, recalculateRange);
			//if (!owner.GetObjectGuid().IsPet())
				if (targetMoved || owner.IsStopped()) // Chase movement may be interrupted
					targetMoved = _offset ? !_target->_IsWithinDist(&owner, _offset * 2, true) :
					!_target->IsWithinMeleeRange(&owner, MELEE_RANGE - 0.5f);
		}

		if (targetMoved)
		{
			_setTargetLocation(owner);

			if (CantReachEvadeTimer)
			{
				if (CantReachEvadeTimer >= EVADE_TIMER)
				{
					if (owner.ToCreature()->AI()->FindNextTargetExceptCurrent())
					{
						CantReachEvadeTimer = 0;
						sLog.DEBUG("--- CantReachEvadeTimer NEW_TARGET");
					}
					else
					{
						owner.ToCreature()->AI()->EnterEvadeModeAndRegen();
						sLog.DEBUG("--- CantReachEvadeTimer EnterEvadeModeAndRegen");
					}

					Player* player = _target->ToPlayer();
					if (player)
					{
						sLog.outLog(LOG_SPECIAL, "Creature %u (guid %u) evaded - attacker player %s (guid %u)", owner.ToCreature()->GetCreatureInfo()->Entry, owner.ToCreature()->GetGUIDLow(), player->GetName(), player->GetGUIDLow());

						if (player->GetSession())
							owner.ToCreature()->Whisper(player->GetSession()->GetHellgroundString(16555), _target->GetGUID());
					}
					else if (owner.ToCreature()->isPet())
						sLog.outLog(LOG_SPECIAL, "Creature %u (guid %u) evaded - attacker pet entry %s (owner guid %u)", owner.ToCreature()->GetCreatureInfo()->Entry, owner.ToCreature()->GetGUIDLow(), _target->GetEntry(), GUID_LOPART(_target->GetCharmerOrOwnerGUID()));
					else
						sLog.outLog(LOG_SPECIAL, "Creature %u (guid %u) evaded - attacker creature %u (guid %u)", owner.ToCreature()->GetCreatureInfo()->Entry, owner.ToCreature()->GetGUIDLow(), _target->GetEntry(), _target->GetGUIDLow());
				}
				else
					CantReachEvadeTimer += time_diff;
			}
		}
		else if (CantReachEvadeTimer) // target is within reachable dest range -> turn off the timer if needed
		{
			CantReachEvadeTimer = 0;
			sLog.DEBUG("--- CantReachEvadeTimer CantReachEvadeTimer reset");
		}
	}

	if (owner.IsStopped())
	{
		if (this->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
		{
			if (owner.GetObjectGuid().IsCreatureOrPet())
			{
				if (!owner.HasInArc(0.01f, _target.getTarget()))
					owner.SetInFront(_target.getTarget());
			}
			else
			{
				if (!owner.HasInArc(M_PI_F / 2.0f, _target.getTarget()))
					owner.SetFacingTo(owner.GetAngleTo(_target.getTarget()));
			}
		}

		if (!_targetReached)
		{
			_targetReached = true;
			static_cast<D*>(this)->_reachTarget(owner);

			if (owner.GetObjectGuid().IsPet() && this->GetMovementGeneratorType() == FOLLOW_MOTION_TYPE)
			{
				float o1 = ((Creature*)owner.ToCreature())->GetOrientation();
				float o2 = _target->GetOrientation();
				
				if (abs(o1 - o2) > 0.2f)
				{
					owner.SetFacingTo(o2);
					owner.SetOrientation(o2);
				}
			}
		}
		//GENSENTODO
		//else
		//{
		//	if (owner.GetObjectGuid().IsCreatureOrPet() && this->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
		//	{
		//		float dist = owner.GetDistance(_target.getTarget());

		//		if (dist < 0.49f)
		//		{
		//			float x, y, z;
		//			_target->GetRandomContactPoint(&owner, x, y, z, 0, MELEE_RANGE - 0.5f);
		//			owner.MonsterMove(x, y, z);
		//		}
		//	}
		//}
	}
	else
	{
		if (static_cast<MovementGenerator*>(this)->_recalculateTravel)
			_setTargetLocation(owner);
	}

	return true;
}

//-----------------------------------------------//
template<class T>
void ChaseMovementGenerator<T>::_reachTarget(T &owner)
{
	if (Creature *creature = owner.ToCreature())
		if (creature->IsAIEnabled)
			creature->AI()->MovementInform(CHASE_MOTION_TYPE, 2);

	if (owner.IsWithinMeleeRange(this->_target.getTarget()))
		owner.Attack(this->_target.getTarget(), true);
}

template<>
void ChaseMovementGenerator<Player>::Initialize(Player &owner)
{
	owner.StopMoving();
	owner.addUnitState(UNIT_STAT_CHASE);
	_setTargetLocation(owner);
}

template<>
void ChaseMovementGenerator<Creature>::Initialize(Creature &owner)
{
	owner.StopMoving();
	owner.SetWalk(false);
	owner.addUnitState(UNIT_STAT_CHASE);
	_setTargetLocation(owner);

	if (owner.IsAIEnabled)
		owner.AI()->MovementInform(CHASE_MOTION_TYPE, 1);
}

template<class T>
void ChaseMovementGenerator<T>::Finalize(T &owner)
{
	Interrupt(owner);

	if (Creature* creature = owner.ToCreature())
	{
		if (creature->IsAIEnabled)
			creature->AI()->MovementInform(CHASE_MOTION_TYPE, 0);

		if (creature->isPet())
			return;
	}
}

template<class T>
void ChaseMovementGenerator<T>::Interrupt(T &owner)
{
	owner.StopMoving();
	owner.ClearUnitState(UNIT_STAT_CHASE);
}

template<class T>
void ChaseMovementGenerator<T>::Reset(T &owner)
{
	Initialize(owner);
}

template<>
bool ChaseMovementGenerator<Creature>::EnableWalking(Creature &creature) const
{
	return creature.GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_ALWAYS_WALK;
}

template<>
bool ChaseMovementGenerator<Player>::EnableWalking(Player &) const
{
	return false;
}

//-----------------------------------------------//
template<>
bool FollowMovementGenerator<Creature>::EnableWalking(Creature &creature) const
{
	return _target.isValid() && _target->IsWalking();
}

template<>
bool FollowMovementGenerator<Player>::EnableWalking(Player &) const
{
	return false;
}

template<>
void FollowMovementGenerator<Player>::_updateSpeed(Player &/*u*/)
{
	// nothing to do for Player
}

template<>
void FollowMovementGenerator<Creature>::_updateSpeed(Creature &u)
{
	// handled by @petspeed
	// pet only sync speed with owner
	//if (!((Creature&)u).isPet() || !_target.isValid() || _target->GetGUID() != u.GetOwnerGUID())
	//	return;

	//u.UpdateSpeed(MOVE_RUN, true);
	//u.UpdateSpeed(MOVE_WALK, true);
	//u.UpdateSpeed(MOVE_SWIM, true);
}

template<>
void FollowMovementGenerator<Player>::Initialize(Player &owner)
{
	owner.StopMoving();

	owner.addUnitState(UNIT_STAT_FOLLOW);
	_updateSpeed(owner);
	_setTargetLocation(owner);
}

template<>
void FollowMovementGenerator<Creature>::Initialize(Creature &owner)
{
	owner.StopMoving();

	owner.addUnitState(UNIT_STAT_FOLLOW);
	_updateSpeed(owner);
	_setTargetLocation(owner);
}

template<class T>
void FollowMovementGenerator<T>::Finalize(T &owner)
{
	Interrupt(owner);
}

template<class T>
void FollowMovementGenerator<T>::Interrupt(T &owner)
{
	owner.StopMoving();

	owner.ClearUnitState(UNIT_STAT_FOLLOW);
	_updateSpeed(owner);
}

template<class T>
void FollowMovementGenerator<T>::Reset(T &owner)
{
	Initialize(owner);
}

//-----------------------------------------------//
template void TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::_setTargetLocation(Player &);
template void TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::_setTargetLocation(Player &);
template void TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::_setTargetLocation(Creature &);
template void TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::_setTargetLocation(Creature &);

template float TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::GetVelocitySpeed(Player&);
template float TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::GetVelocitySpeed(Player&);
template float TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::GetVelocitySpeed(Creature&);
template float TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::GetVelocitySpeed(Creature&);

template bool TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::Update(Player &, const uint32 &);
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::Update(Player &, const uint32 &);
template bool TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::Update(Creature &, const uint32 &);
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::Update(Creature &, const uint32 &);

template void ChaseMovementGenerator<Player>::_reachTarget(Player &);
template void ChaseMovementGenerator<Creature>::_reachTarget(Creature &);
template void ChaseMovementGenerator<Player>::Finalize(Player &);
template void ChaseMovementGenerator<Creature>::Finalize(Creature &);
template void ChaseMovementGenerator<Player>::Interrupt(Player &);
template void ChaseMovementGenerator<Creature>::Interrupt(Creature &);
template void ChaseMovementGenerator<Player>::Reset(Player &);
template void ChaseMovementGenerator<Creature>::Reset(Creature &);

template void FollowMovementGenerator<Player>::Finalize(Player &);
template void FollowMovementGenerator<Creature>::Finalize(Creature &);
template void FollowMovementGenerator<Player>::Interrupt(Player &);
template void FollowMovementGenerator<Creature>::Interrupt(Creature &);
template void FollowMovementGenerator<Player>::Reset(Player &);
template void FollowMovementGenerator<Creature>::Reset(Creature &);
