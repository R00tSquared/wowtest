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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundNA.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Language.h"
#include "WorldPacket.h"

BattleGroundNA::BattleGroundNA()
{
    m_BgObjects.resize(BG_NA_OBJECT_MAX);
    m_BgCreatures.resize(2);
}

BattleGroundNA::~BattleGroundNA()
{

}

void BattleGroundNA::Update(uint32 diff)
{
    BattleGround::Update(diff);

    // after bg start we get there
    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        ModifyStartDelayTime(diff);

        if (!(m_Events & 0x01))
        {
            m_Events |= 0x01;
            // setup here, only when at least one player has ported to the map
            if (!SetupBattleGround())
            {
                EndNow();
                return;
            }
            for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_4; i++)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            for (uint32 i = BG_NA_OBJECT_T_CR_1; i <= BG_NA_OBJECT_T_CR_2; i++)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            SetStartDelayTime(START_DELAY1);
            SendMessageToAll(LANG_ARENA_ONE_MINUTE);

            AddCreature(693071, 0, HORDE, 4023.93, 2966.84, 12.17, 1.50);
            AddCreature(693071, 1, ALLIANCE, 4089.35, 2874.51, 12.15, 4.57);
        }
        // After 30 seconds, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY2 && !(m_Events & 0x04))
        {
            m_Events |= 0x04;
            SendMessageToAll(LANG_ARENA_THIRTY_SECONDS);
        }
        // After 15 seconds, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY3 && !(m_Events & 0x08))
        {
            m_Events |= 0x08;
            SendMessageToAll(LANG_ARENA_FIFTEEN_SECONDS);
        }
        // delay expired (1 minute)
        else if (GetStartDelayTime() <= 0 && !(m_Events & 0x10))
        {
            m_Events |= 0x10;

            for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; i++)
                DoorOpen(i);

            SpawnBGObject(BG_NA_OBJECT_BUFF_1, 60);
            SpawnBGObject(BG_NA_OBJECT_BUFF_2, 60);

            BGStarted();
            SetStartDelayTime(0);

            // If players have just left before arena even started
            if (!GetAlivePlayersCountByTeam(ALLIANCE) || !GetAlivePlayersCountByTeam(HORDE) || Solo3v3ArenaFinishIfNotFull())
            {
                if (GetArenaType() == ARENA_TYPE_3v3)
                    SetRated(false);
                m_ShouldEndTime = 1000;
            }               
        }
    }

    /*if(GetStatus() == STATUS_IN_PROGRESS)
    {
        // update something
    }*/
}

void BattleGroundNA::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundNAScore* sc = new BattleGroundNAScore;

    m_PlayerScores[plr->GetGUID()] = sc;

    UpdateWorldState(0xa0f, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0xa10, GetAlivePlayersCountByTeam(HORDE));
}

void BattleGroundNA::RemovePlayer(Player* /*plr*/, uint64 /*guid*/)
{
    if (GetStatus() == STATUS_WAIT_LEAVE || GetStatus() == STATUS_WAIT_JOIN)
        return;

    UpdateWorldState(0xa0f, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0xa10, GetAlivePlayersCountByTeam(HORDE));

    if (!GetAlivePlayersCountByTeam(ALLIANCE) || !GetAlivePlayersCountByTeam(HORDE))
        m_ShouldEndTime = 1000; // end arena in 1 sec, one of the teams is fully dead
}

void BattleGroundNA::HandleKillPlayer(Player *player, Player *killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (!killer)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGroundNA: Killer player not found");
        return;
    }

    BattleGround::HandleKillPlayer(player,killer);

    UpdateWorldState(0xa0f, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0xa10, GetAlivePlayersCountByTeam(HORDE));

    if (!GetAlivePlayersCountByTeam(ALLIANCE) || !GetAlivePlayersCountByTeam(HORDE))
        m_ShouldEndTime = 1000; // end arena in 1 sec, one of the teams is fully dead
}

bool BattleGroundNA::HandlePlayerUnderMap(Player *player, float z)
{
    if (z > 3.0f)
        return false;

    player->NearTeleportTo(4055.504395,2919.660645,13.611241,player->GetOrientation(),false);
    return true;
}

void BattleGroundNA::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    //uint32 SpellId = 0;
    //uint64 buff_guid = 0;
    switch (Trigger)
    {
        case 4536:                                          // buff trigger?
        case 4537:                                          // buff trigger?
            break;
        default:
            if(!Source->IsSpectator())
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: WARNING: Unhandled AreaTrigger in Battleground: %u . PlayerGUID: %s , PlayerClass: %u. PX: %f PY: %f PZ: %f", Trigger, Source->GetName(), Source->GetClass(), Source->GetPositionX(), Source->GetPositionY(), Source->GetPositionZ());
                Source->NearTeleportTo(4055.504395, 2919.660645, 13.611241, Source->GetOrientation(), false);
            }
            break;
    }

    //if(buff_guid)
    //    HandleTriggerBuff(buff_guid,Source);
}

void BattleGroundNA::FillInitialWorldStates(WorldPacket &data)
{
    data << uint32(0xa0f) << uint32(GetAlivePlayersCountByTeam(ALLIANCE));           // 7
    data << uint32(0xa10) << uint32(GetAlivePlayersCountByTeam(HORDE));           // 8
    data << uint32(0xa11) << uint32(1);           // 9
}

void BattleGroundNA::ResetBGSubclass()
{

}

bool BattleGroundNA::SetupBattleGround()
{
    // gates
    if (  !AddObject(BG_NA_OBJECT_DOOR_1, BG_NA_OBJECT_TYPE_DOOR_1, 4031.854, 2966.833, 12.6462, -2.648788, 0, 0, 0.9697962, -0.2439165, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_2, BG_NA_OBJECT_TYPE_DOOR_2, 4081.179, 2874.97, 12.39171, 0.4928045, 0, 0, 0.2439165, 0.9697962, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_3, BG_NA_OBJECT_TYPE_DOOR_3, 4023.709, 2981.777, 10.70117, -2.648788, 0, 0, 0.9697962, -0.2439165, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_DOOR_4, BG_NA_OBJECT_TYPE_DOOR_4, 4090.064, 2858.438, 10.23631, 0.4928045, 0, 0, 0.2439165, 0.9697962, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_T_CR_1, BG_NA_OBJECT_TYPE_CRYS_1, 4029.951172f, 2970.304688f, 12.170729f, 5.230761f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_NA_OBJECT_T_CR_2, BG_NA_OBJECT_TYPE_CRYS_1, 4082.929688f, 2871.796631f, 12.172503f, 2.021443f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY)
    // buffs
        || !AddObject(BG_NA_OBJECT_BUFF_1, BG_NA_OBJECT_TYPE_BUFF_1, 4009.189941, 2895.250000, 13.052700, -1.448624, 0, 0, 0.6626201, -0.7489557, 120)
        || !AddObject(BG_NA_OBJECT_BUFF_2, BG_NA_OBJECT_TYPE_BUFF_2, 4103.330078, 2946.350098, 13.051300, -0.06981307, 0, 0, 0.03489945, -0.9993908, 120))
    {
        sLog.outLog(LOG_DB_ERR, "BatteGroundNA: Failed to spawn some object!");
        return false;
    }

    return true;
}

/*
20:12:14 id:036668 [S2C] SMSG_INIT_WORLD_STATES (706 = 0x02C2) len: 86
0000: 2f 02 00 00 72 0e 00 00 00 00 00 00 09 00 11 0a  |  /...r...........
0010: 00 00 01 00 00 00 0f 0a 00 00 00 00 00 00 10 0a  |  ................
0020: 00 00 00 00 00 00 d4 08 00 00 00 00 00 00 d8 08  |  ................
0030: 00 00 00 00 00 00 d7 08 00 00 00 00 00 00 d6 08  |  ................
0040: 00 00 00 00 00 00 d5 08 00 00 00 00 00 00 d3 08  |  ................
0050: 00 00 00 00 00 00                                |  ......
*/

