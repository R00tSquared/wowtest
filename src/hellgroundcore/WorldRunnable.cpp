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

/** \file
    \ingroup Trinityd
*/

#include "WorldSocketMgr.h"
#include "Common.h"
#include "World.h"
#include "WorldRunnable.h"
#include "Timer.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "BattleGroundMgr.h"
#include "InstanceSaveMgr.h"

#include "Database/DatabaseEnv.h"

#define WORLD_SLEEP_CONST 50

/// Heartbeat for the World
void WorldRunnable::run()
{
    ///- Init new SQL thread for the world database
    GameDataDatabase.ThreadStart();                            // let thread do safe mySQL requests (one connection call enough)
    sWorld.InitResultQueue();

    WorldTimer::tick(); //initialize world timer
    ///- While we have not World::m_stopEvent, update the world
    while (!World::IsStopped())
    {
        ++World::m_worldLoopCounter;
        uint32 diff = WorldTimer::tick();
        if (diff < WORLD_SLEEP_CONST)
        {
            ACE_Based::Thread::Sleep(WORLD_SLEEP_CONST - diff);
            diff = WorldTimer::tickTimeRenew(); // need to update current time after sleep
        }

        sWorld.Update(diff);
    }

    sWorld.UnloadAllFakeBots();
    sWorld.KickAll();                                       // save and kick all players
    sWorld.UpdateSessions(uint32(1));                       // real players unload required UpdateSessions call

    // unload battleground templates before different singletons destroyed
    sBattleGroundMgr.DeleteAllBattleGrounds();

    sWorldSocketMgr->StopNetwork();
    sMapMgr.UnloadAll();                                    // unload all grids (including locked in memory)

    ///- End the database thread
    GameDataDatabase.ThreadEnd();                              // free mySQL thread resources
}
