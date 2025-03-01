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
#include "BattleGroundBE.h"
#include "Creature.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "WorldPacket.h"
#include "Language.h"

BattleGroundBE::BattleGroundBE()
{
    m_BgObjects.resize(BG_BE_OBJECT_MAX);
    m_BgCreatures.resize(2);
}

BattleGroundBE::~BattleGroundBE()
{

}

void BattleGroundBE::Update(uint32 diff)
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
            for (uint32 i = BG_BE_OBJECT_DOOR_1; i <= BG_BE_OBJECT_DOOR_4; i++)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            SpawnBGObject(BG_BE_OBJECT_BUFF_1, RESPAWN_ONE_DAY);
            SpawnBGObject(BG_BE_OBJECT_BUFF_2, RESPAWN_ONE_DAY);

            for (uint32 i = BG_BE_OBJECT_T_CR_1; i <= BG_BE_OBJECT_T_CR_2; i++)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            SetStartDelayTime(START_DELAY1);
            SendMessageToAll(LANG_ARENA_ONE_MINUTE);

            AddCreature(693071, 0, HORDE, 6184.22, 240.05, 5.36, 4.51);
            AddCreature(693071, 1, ALLIANCE, 6292.85, 283.35, 5.42, 1.28);
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

            for (uint32 i = BG_BE_OBJECT_DOOR_1; i <= BG_BE_OBJECT_DOOR_2; i++)
                DoorOpen(i);

            SpawnBGObject(BG_BE_OBJECT_BUFF_1, 60);
            SpawnBGObject(BG_BE_OBJECT_BUFF_2, 60);

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

void BattleGroundBE::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundBEScore* sc = new BattleGroundBEScore;

    m_PlayerScores[plr->GetGUID()] = sc;

    UpdateWorldState(0x9f1, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0x9f0, GetAlivePlayersCountByTeam(HORDE));
}

void BattleGroundBE::RemovePlayer(Player* /*plr*/, uint64 /*guid*/)
{
    if (GetStatus() == STATUS_WAIT_LEAVE || GetStatus() == STATUS_WAIT_JOIN)
        return;

    UpdateWorldState(0x9f1, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0x9f0, GetAlivePlayersCountByTeam(HORDE));

    if (!GetAlivePlayersCountByTeam(ALLIANCE) || !GetAlivePlayersCountByTeam(HORDE))
        m_ShouldEndTime = 1000; // end arena in 1 sec, one of the teams is fully dead
}

void BattleGroundBE::HandleKillPlayer(Player *player, Player *killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (!killer)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Killer player not found");
        return;
    }

    BattleGround::HandleKillPlayer(player,killer);

    UpdateWorldState(0x9f1, GetAlivePlayersCountByTeam(ALLIANCE));
    UpdateWorldState(0x9f0, GetAlivePlayersCountByTeam(HORDE));
    
    if (!GetAlivePlayersCountByTeam(ALLIANCE) || !GetAlivePlayersCountByTeam(HORDE))
        m_ShouldEndTime = 1000; // end arena in 1 sec, one of the teams is fully dead
}

bool BattleGroundBE::HandlePlayerUnderMap(Player *player, float z)
{
    if (z > -10.0f)
        return false;

    player->NearTeleportTo(6238.930176, 262.963470, 2, player->GetOrientation(), false);
    return true;
}

void BattleGroundBE::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    //uint32 SpellId = 0;
    //uint64 buff_guid = 0;
    switch (Trigger)
    {
        case 4538:                                          // buff trigger?
            //buff_guid = m_BgObjects[BG_BE_OBJECT_BUFF_1];
            break;
        case 4539:                                          // buff trigger?
            //buff_guid = m_BgObjects[BG_BE_OBJECT_BUFF_2];
            break;
        default:
            if(!Source->IsSpectator())
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: WARNING: Unhandled AreaTrigger in Battleground: %u . PlayerGUID: %s , PlayerClass: %u. PX: %f PY: %f PZ: %f", Trigger, Source->GetName(), Source->GetClass(), Source->GetPositionX(), Source->GetPositionY(), Source->GetPositionZ());
                Source->NearTeleportTo(6238.930176, 262.963470, 0.889519, Source->GetOrientation(), false);
            }
            break;
    }

    //if(buff_guid)
    //    HandleTriggerBuff(buff_guid,Source);
}

void BattleGroundBE::FillInitialWorldStates(WorldPacket &data)
{
    data << uint32(0x9f1) << uint32(GetAlivePlayersCountByTeam(ALLIANCE));           // 7
    data << uint32(0x9f0) << uint32(GetAlivePlayersCountByTeam(HORDE));           // 8
    data << uint32(0x9f3) << uint32(1);           // 9
}

void BattleGroundBE::ResetBGSubclass()
{

}

bool BattleGroundBE::SetupBattleGround()
{
    // gates
    if (  !AddObject(BG_BE_OBJECT_DOOR_1, BG_BE_OBJECT_TYPE_DOOR_1, 6287.277f, 282.1877f, 3.810925f, -2.260201f, 0, 0, 0.9044551f, -0.4265689f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_BE_OBJECT_DOOR_2, BG_BE_OBJECT_TYPE_DOOR_2, 6189.546f, 241.7099f, 3.101481f, 0.8813917f, 0, 0, 0.4265689f, 0.9044551f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_BE_OBJECT_DOOR_3, BG_BE_OBJECT_TYPE_DOOR_3, 6299.116f, 296.5494f, 3.308032f, 0.8813917f, 0, 0, 0.4265689f, 0.9044551f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_BE_OBJECT_DOOR_4, BG_BE_OBJECT_TYPE_DOOR_4, 6177.708f, 227.3481f, 3.604374f, -2.260201f, 0, 0, 0.9044551f, -0.4265689f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_BE_OBJECT_T_CR_1, BG_BE_OBJECT_TYPE_CRYS_1, 6289.799316f, 284.867371f, 5.055864f, 0.850584f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_BE_OBJECT_T_CR_2, BG_BE_OBJECT_TYPE_CRYS_1, 6187.76123f, 239.245834f, 5.011842f, 0.889847f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY)
        // buffs
        || !AddObject(BG_BE_OBJECT_BUFF_1, BG_BE_OBJECT_TYPE_BUFF_1, 6249.042f, 275.3239f, 11.22033f, -1.448624f, 0, 0, 0.6626201f, -0.7489557f, 120)
        || !AddObject(BG_BE_OBJECT_BUFF_2, BG_BE_OBJECT_TYPE_BUFF_2, 6228.26f, 249.566f, 11.21812f, -0.06981307f, 0, 0, 0.03489945f, -0.9993908f, 120))
    {
        sLog.outLog(LOG_DB_ERR, "BatteGroundBE: Failed to spawn some object!");
        return false;
    }

    return true;
}

void BattleGroundBE::UpdatePlayerScore(Player* Source, uint32 type, uint32 value)
{

    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if (itr == m_PlayerScores.end())                         // player not found...
        return;

    //there is nothing special in this score
    BattleGround::UpdatePlayerScore(Source,type,value);

}

/*
21:45:46 id:231310 [S2C] SMSG_INIT_WORLD_STATES (706 = 0x02C2) len: 86
0000: 32 02 00 00 76 0e 00 00 00 00 00 00 09 00 f3 09  |  2...v...........
0010: 00 00 01 00 00 00 f1 09 00 00 01 00 00 00 f0 09  |  ................
0020: 00 00 02 00 00 00 d4 08 00 00 00 00 00 00 d8 08  |  ................
0030: 00 00 00 00 00 00 d7 08 00 00 00 00 00 00 d6 08  |  ................
0040: 00 00 00 00 00 00 d5 08 00 00 00 00 00 00 d3 08  |  ................
0050: 00 00 00 00 00 00                                |  ......

spell 32724 - Gold Team
spell 32725 - Green Team
35774 Gold Team
35775 Green Team
*/

