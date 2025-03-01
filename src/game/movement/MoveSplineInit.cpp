// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "packet_builder.h"
#include "../Unit.h"

namespace Movement
{
    UnitMoveType SelectSpeedType(uint32 moveFlags)
    {
        if (moveFlags & (MOVEFLAG_FLYING | MOVEFLAG_LEVITATING))
        {
            if (moveFlags & MOVEFLAG_BACKWARD /*&& speed_obj.flight >= speed_obj.flight_back*/)
                return MOVE_FLIGHT_BACK;
            else
                return MOVE_FLIGHT;
        }
        else if (moveFlags & MOVEFLAG_SWIMMING)
        {
            if (moveFlags & MOVEFLAG_BACKWARD /*&& speed_obj.swim >= speed_obj.swim_back*/)
                return MOVE_SWIM_BACK;
            else
                return MOVE_SWIM;
        }
        else if (moveFlags & MOVEFLAG_WALK_MODE)
        {
            //if ( speed_obj.run > speed_obj.walk )
            return MOVE_WALK;
        }
        else if (moveFlags & MOVEFLAG_BACKWARD /*&& speed_obj.run >= speed_obj.run_back*/)
            return MOVE_RUN_BACK;

        return MOVE_RUN;
    }

    int32 MoveSplineInit::Launch()
    {
        MoveSpline& move_spline = *unit.movespline;

        Vector3 real_position(unit.GetPositionX(),unit.GetPositionY(),unit.GetPositionZ());
        // there is a big chance that current position is unknown if current state is not finalized, need compute it
        // this also allows calculate spline position and update map position in much greater intervals
        if (!move_spline.Finalized())
            real_position = move_spline.ComputePosition();

        if (args.path.empty())
        {
            // should i do the things that user should do?
            MoveTo(real_position);
        }

        // current first vertex
        args.path[0] = real_position;
        uint32 moveFlags = unit.m_movementInfo.GetMovementFlags();
        if (args.flags.runmode)
            moveFlags &= ~MOVEFLAG_WALK_MODE;
        else
            moveFlags |= MOVEFLAG_WALK_MODE;

        moveFlags |= (MOVEFLAG_SPLINE_ENABLED|MOVEFLAG_FORWARD);

        if (args.velocity == 0.f) // ignore swim speed and flight speed because its not used in generic scripting - always possible to override
            args.velocity = unit.GetSpeed(SelectSpeedType(moveFlags &~ (MOVEFLAG_FLYING | MOVEFLAG_SWIMMING)));

		if (args.velocity <= 0.f)
			sLog.outLog(LOG_SPECIAL, "unit %s %u trying to launch spline with %f speed", unit.GetName(), unit.GetGUIDLow(), args.velocity);

        if (!args.Validate(&unit))
            return 0;

        if (moveFlags & MOVEFLAG_ROOT)
            moveFlags &= ~MOVEFLAG_MOVING;

        unit.m_movementInfo.SetMovementFlags((MovementFlags)moveFlags);
        move_spline.Initialize(args);

        WorldPacket data(SMSG_MONSTER_MOVE, 64);
        data << unit.GetPackGUID();
        PacketBuilder::WriteMonsterMove(move_spline, data);
        unit.BroadcastPacket(&data,true);
        unit.SendCombatStats(1 << COMBAT_STATS_TEST, "spline launch", NULL);

        return move_spline.Duration();
    }

    MoveSplineInit::MoveSplineInit(Unit& m) : unit(m)
    {
        // mix existing state into new
        args.flags.runmode = !unit.m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE);
        args.flags.flying = unit.m_movementInfo.HasMovementFlag((MovementFlags)(MOVEFLAG_FLYING|MOVEFLAG_LEVITATING));
    }

    void MoveSplineInit::SetFacing(const Unit * target)
    {
        args.flags.EnableFacingTarget();
        args.facing.target = target->GetObjectGuid().GetRawValue();
    }

    void MoveSplineInit::SetFacing(float angle)
    {
        args.facing.angle = G3D::wrap(angle, 0.f, (float)G3D::twoPi());
        args.flags.EnableFacingAngle();
    }
}
