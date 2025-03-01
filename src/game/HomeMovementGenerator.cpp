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

#include "HomeMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "WorldPacket.h"

#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

void HomeMovementGenerator<Creature>::Initialize(Creature & owner)
{
    owner.addUnitState(UNIT_STAT_IGNORE_ATTACKERS);
    _setTargetLocation(owner);
}

void HomeMovementGenerator<Creature>::Reset(Creature &)
{
}

void HomeMovementGenerator<Creature>::_setTargetLocation(Creature & owner)
{
    if (owner.HasUnitState(UNIT_STAT_NOT_MOVE))
        return;

    Movement::MoveSplineInit init(owner);
    float x, y, z, o;

    // at apply we can select more nice return points base at current move generator
    owner.GetHomePosition(x, y, z, o);

    init.SetFacing(o);

    init.MoveTo(x,y,z, true);
    init.SetWalk(false);
    init.Launch();

    arrived = false;
    owner.ClearUnitState(UNIT_STAT_ALL_STATE);
}

bool HomeMovementGenerator<Creature>::Update(Creature &owner, const uint32& time_diff)
{
    arrived = owner.movespline->Finalized();
    return !arrived;
}

void HomeMovementGenerator<Creature>::Finalize(Creature& owner)
{
    // should be removed only on restorereactstate event finish
    owner.ClearUnitState(UNIT_STAT_IGNORE_ATTACKERS);

    if (arrived)
    {
        owner.SetWalk(true);
        owner.LoadCreaturesAddon(true);

        owner.AI()->JustReachedHome();
    }
}
