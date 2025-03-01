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
#include "BattleGroundWS.h"
#include "BattleGroundMgr.h"
#include "Creature.h"
#include "GameObject.h"
#include "Chat.h"
#include "MapManager.h"
#include "Language.h"
#include "World.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"

// these variables aren't used outside of this file, so declare them only here
enum BG_WSG_Rewards
{
    BG_WSG_WIN = 0,
    BG_WSG_FLAG_CAP,
    BG_WSG_MAP_COMPLETE, // unused
    BG_WSG_REWARD_NUM
};

uint32 BG_WSG_Honor[BG_HONOR_MODE_NUM][BG_WSG_REWARD_NUM] = {
    //WIN_HONOR_COINT must be here
    { 0, 40, 40 }, // normal honor
    {60,40,80},  // holiday (unused)
};

uint32 BG_WSG_Reputation[BG_HONOR_MODE_NUM][BG_WSG_REWARD_NUM] = {
    {0,35,0}, // normal reputation
    {0,45,0},  // holiday
};

BattleGroundWS::BattleGroundWS()
{
    m_BothFlagsKept = false;
    m_BgObjects.resize(BG_WS_OBJECT_MAX);
    m_BgCreatures.resize(BG_CREATURES_MAX_WS);
    m_AllianceFlagUpdate = 0;
    m_HordeFlagUpdate = 0;
}

BattleGroundWS::~BattleGroundWS()
{
}

void BattleGroundWS::Update(uint32 diff)
{
    BattleGround::Update(diff);

    if (sBattleGroundMgr.IsWSGEndAfterEnabled())
    {
        if (m_TimeElapsedSinceBeggining > sBattleGroundMgr.GetWSGEndAfterTime() && GetStatus() == STATUS_IN_PROGRESS)
        {
            EndBattleGround(GetWinningTeam());
            return;
        }

        if(m_TimeElapsedSinceBeggining> sBattleGroundMgr.GetWSGEndAfterTime()/2 &&
            m_TimeElapsedSinceBeggining/120000 > (m_TimeElapsedSinceBeggining - diff)/120000) //warning every 2 mins
            SendMessageToAll(LANG_BATTLEGROUND_WILL_END, (sBattleGroundMgr.GetWSGEndAfterTime() - m_TimeElapsedSinceBeggining + diff) / 60000);
    }

    // after bg start we get there (once)
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

//            for (uint32 i = WS_SPIRIT_MAIN_ALLIANCE; i <= WS_SPIRIT_MAIN_HORDE; i++)
//                SpawnBGCreature(i, RESPAWN_IMMEDIATELY);

            for (uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_H_4; i++)
            {
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);
                DoorClose(i);
            }
            for (uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; i++)
                SpawnBGObject(i, RESPAWN_ONE_DAY);

            // spawn multi npcs
            AddCreature(693070, 2, HORDE, 948.92, 1432.36, 345.39, 3.15);
            AddCreature(693070, 3, ALLIANCE, 1506.28, 1484.79, 352.03, 6.27);

            AddRadiusNpc();

            SetStartDelayTime(START_DELAY0);
        }
        // After 1 minute, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY1 && !(m_Events & 0x04))
        {
            m_Events |= 0x04;
            SendMessageToAll(LANG_BG_WS_ONE_MINUTE);
        }
        // After 1,5 minute, warning is signalled
        else if (GetStartDelayTime() <= START_DELAY2 && !(m_Events & 0x08))
        {
            m_Events |= 0x08;
            SendMessageToAll(LANG_BG_WS_HALF_MINUTE);
        }
        // After 2 minutes, gates OPEN ! x)
        else if (GetStartDelayTime() < 0 && !(m_Events & 0x10))
        {
            m_Events |= 0x10;
            
            for (uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_A_4; i++)
                DoorOpen(i);
            for (uint32 i = BG_WS_OBJECT_DOOR_H_1; i <= BG_WS_OBJECT_DOOR_H_2; i++)
                DoorOpen(i);

            SpawnBGObject(BG_WS_OBJECT_DOOR_A_5, RESPAWN_ONE_DAY);
            SpawnBGObject(BG_WS_OBJECT_DOOR_A_6, RESPAWN_ONE_DAY);
            SpawnBGObject(BG_WS_OBJECT_DOOR_H_3, RESPAWN_ONE_DAY);
            SpawnBGObject(BG_WS_OBJECT_DOOR_H_4, RESPAWN_ONE_DAY);

            for (uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; i++)
                SpawnBGObject(i, RESPAWN_IMMEDIATELY);

            BGStarted();
        }
    }
    else if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (m_FlagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            if (m_FlagsTimer[TEAM_ALLIANCE].Expired(diff))
            {
                m_FlagsTimer[TEAM_ALLIANCE] = 0;
                RespawnFlag(ALLIANCE, true);
            }
        }
        if (m_FlagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_GROUND)
        {
            if (m_FlagsDropTimer[TEAM_ALLIANCE].Expired(diff))
            {
                m_FlagsDropTimer[TEAM_ALLIANCE] = 0;
                RespawnFlagAfterDrop(ALLIANCE);
            }
        }
        if (m_FlagState[TEAM_HORDE] == BG_WS_FLAG_STATE_WAIT_RESPAWN)
        {
            if (m_FlagsTimer[TEAM_HORDE].Expired(diff))
            {
                m_FlagsTimer[TEAM_HORDE] = 0;
                RespawnFlag(HORDE, true);
            }
        }
        if (m_FlagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_GROUND)
        {
            if (m_FlagsDropTimer[TEAM_HORDE].Expired(diff))
            {
                m_FlagsDropTimer[TEAM_HORDE] = 0;
                RespawnFlagAfterDrop(HORDE);
            }
        }
        if (m_BothFlagsKept)
        {
            if (GetFlagState(ALLIANCE) == BG_WS_FLAG_STATE_ON_BASE && GetFlagState(HORDE) == BG_WS_FLAG_STATE_ON_BASE)
            {
                m_BothFlagsKept = false;
                m_FlagSpellForceTimer = 0;
                m_FlagDebuffState = 0;
            }
            else
            {
                m_FlagSpellForceTimer += diff;
                if (m_FlagDebuffState == 0 && m_FlagSpellForceTimer >= 600000)  //10 minutes GENSENTODO
                {
                    if (Player* plr = sObjectMgr.GetPlayerInWorld(m_FlagKeepers[0]))
                        plr->CastSpell(plr, BG_WS_SPELL_FOCUSED_ASSAULT, true);
                    if (Player* plr = sObjectMgr.GetPlayerInWorld(m_FlagKeepers[1]))
                        plr->CastSpell(plr, BG_WS_SPELL_FOCUSED_ASSAULT, true);
                    m_FlagDebuffState = 1;
                }
                else if (m_FlagDebuffState == 1 && m_FlagSpellForceTimer >= 900000) //15 minutes
                {
                    if (Player* plr = sObjectMgr.GetPlayerInWorld(m_FlagKeepers[0]))
                    {
                        plr->RemoveAurasDueToSpell(BG_WS_SPELL_FOCUSED_ASSAULT);
                        plr->CastSpell(plr, BG_WS_SPELL_BRUTAL_ASSAULT, true);
                    }
                    if (Player* plr = sObjectMgr.GetPlayerInWorld(m_FlagKeepers[1]))
                    {
                        plr->RemoveAurasDueToSpell(BG_WS_SPELL_FOCUSED_ASSAULT);
                        plr->CastSpell(plr, BG_WS_SPELL_BRUTAL_ASSAULT, true);
                    }
                    m_FlagDebuffState = 2;
                }
            }
        }
        else
        {
            m_FlagSpellForceTimer = 0; //reset timer.
            m_FlagDebuffState = 0;
        }
    }
}

void BattleGroundWS::AddPlayer(Player *plr)
{
    BattleGround::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattleGroundWGScore* sc = new BattleGroundWGScore;

    m_PlayerScores[plr->GetGUID()] = sc;
}

void BattleGroundWS::RespawnFlag(PlayerTeam Team, bool captured)
{
    if (Team == ALLIANCE)
    {
        sLog.outDebug("Respawn Alliance flag");
        m_FlagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_BASE;
        m_AllianceFlagUpdate = 0;
    }
    else
    {
        sLog.outDebug("Respawn Horde flag");
        m_FlagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_BASE;
        m_HordeFlagUpdate = 0;
    }

    if (captured)
    {
        //when map_update will be allowed for battlegrounds this code will be useless
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
        SendMessageToAll(LANG_BG_WS_F_PLACED);

        PlaySoundToAll(BG_WS_SOUND_FLAGS_RESPAWNED);        // flag respawned sound...
    }
}

void BattleGroundWS::RespawnFlagAfterDrop(PlayerTeam team)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    RespawnFlag(team,false);
    if (team == ALLIANCE)
    {
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);

        SendMessageToAll(LANG_BG_WS_ALLIANCE_FLAG_RESPAWNED);
    }
    else
    {
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
        SendMessageToAll(LANG_BG_WS_HORDE_FLAG_RESPAWNED);
    }

    PlaySoundToAll(BG_WS_SOUND_FLAGS_RESPAWNED);

    Map * tmpMap = GetMap();

    if (!tmpMap)
        return;

    GameObject *obj = tmpMap->GetGameObject(GetDroppedFlagGUID(team));
    if (obj)
        obj->Delete();
    else
        sLog.outLog(LOG_DEFAULT, "ERROR: unknown droped flag bg, guid: %u",GUID_LOPART(GetDroppedFlagGUID(team)));

    SetDroppedFlagGUID(0,team);
}

void BattleGroundWS::EventPlayerCapturedFlag(Player *Source)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 type = 0;
    PlayerTeam winner = TEAM_NONE;
    int32 message;

    //TODO FIX reputation and honor gains for low level players!

    Source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    if (Source->GetBGTeam() == ALLIANCE)
    {
        if (!this->IsHordeFlagPickedup())
            return;
        SetHordeFlagPicker(0);                              // must be before aura remove to prevent 2 events (drop+capture) at the same time
                                                            // horde flag in base (but not respawned yet)
        m_FlagState[TEAM_HORDE] = BG_WS_FLAG_STATE_WAIT_RESPAWN;
                                                            // Drop Horde Flag from Player
        Source->RemoveAurasDueToSpell(BG_WS_SPELL_WARSONG_FLAG);

        if (m_FlagDebuffState == 1 || Source->HasAura(BG_WS_SPELL_FOCUSED_ASSAULT))
            Source->RemoveAurasDueToSpell(BG_WS_SPELL_FOCUSED_ASSAULT);
        if (m_FlagDebuffState == 2 || Source->HasAura(BG_WS_SPELL_BRUTAL_ASSAULT))
            Source->RemoveAurasDueToSpell(BG_WS_SPELL_BRUTAL_ASSAULT);

        message = LANG_BG_WS_CAPTURED_HF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        if (GetTeamScore(ALLIANCE) < BG_WS_MAX_TEAM_SCORE)
            AddPoint(ALLIANCE, 1);
        PlaySoundToAll(BG_WS_SOUND_FLAG_CAPTURED_ALLIANCE);
        RewardReputationToTeam(890, BG_WSG_Reputation[m_HonorMode][BG_WSG_FLAG_CAP], ALLIANCE);          // +35 reputation
        RewardHonorToTeam(BG_WSG_Honor[m_HonorMode][BG_WSG_FLAG_CAP], ALLIANCE);                    // +40 bonushonor
    }
    else
    {
        if (!this->IsAllianceFlagPickedup())
            return;
        SetAllianceFlagPicker(0);                           // must be before aura remove to prevent 2 events (drop+capture) at the same time
                                                            // alliance flag in base (but not respawned yet)
        m_FlagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_WAIT_RESPAWN;
                                                            // Drop Alliance Flag from Player
        Source->RemoveAurasDueToSpell(BG_WS_SPELL_SILVERWING_FLAG);

        if (m_FlagDebuffState == 1 || Source->HasAura(BG_WS_SPELL_FOCUSED_ASSAULT))
          Source->RemoveAurasDueToSpell(BG_WS_SPELL_FOCUSED_ASSAULT);
        if (m_FlagDebuffState == 2 || Source->HasAura(BG_WS_SPELL_BRUTAL_ASSAULT))
          Source->RemoveAurasDueToSpell(BG_WS_SPELL_BRUTAL_ASSAULT);

        message = LANG_BG_WS_CAPTURED_AF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        if (GetTeamScore(HORDE) < BG_WS_MAX_TEAM_SCORE)
            AddPoint(HORDE, 1);
        PlaySoundToAll(BG_WS_SOUND_FLAG_CAPTURED_HORDE);
        RewardReputationToTeam(889, BG_WSG_Reputation[m_HonorMode][BG_WSG_FLAG_CAP], HORDE);             // +35 reputation
        RewardHonorToTeam(BG_WSG_Honor[m_HonorMode][BG_WSG_FLAG_CAP], HORDE);                       // +40 bonushonor
    }

    SpawnBGObject(BG_WS_OBJECT_H_FLAG, BG_WS_FLAG_RESPAWN_TIME);
    SpawnBGObject(BG_WS_OBJECT_A_FLAG, BG_WS_FLAG_RESPAWN_TIME);

    SendMessageToAll(message, type, Source->GetSession(), Source->GetGUID());

    UpdateFlagState(Source->GetBGTeam(), 1);                  // flag state none
    UpdateTeamScore(Source->GetBGTeam());
    // only flag capture should be updated
    UpdatePlayerScore(Source, SCORE_FLAG_CAPTURES, 1);      // +1 flag captures...

    if (GetTeamScore(ALLIANCE) == BG_WS_MAX_TEAM_SCORE)
        winner = ALLIANCE;

    if (GetTeamScore(HORDE) == BG_WS_MAX_TEAM_SCORE)
        winner = HORDE;

    if (winner)
    {
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 0);
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 0);
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, 1);
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, 1);

        // BG_WSG_Honor
        //RewardHonorToTeam(BG_WSG_Honor[m_HonorMode][BG_WSG_WIN], winner);
        EndBattleGround(winner);
    }
    else
    {
        m_FlagsTimer[GetTeamIndexByTeamId(Source->GetBGTeam()) ? 0 : 1].Reset(BG_WS_FLAG_RESPAWN_TIME);
    }

    m_BothFlagsKept = false;
}

void BattleGroundWS::EventPlayerDroppedFlag(Player *Source)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        // if not running, do not cast things at the dropper player (prevent spawning the "dropped" flag), neither send unnecessary messages
        // just take off the aura
        if (Source->GetBGTeam() == ALLIANCE)
        {
            if (!this->IsHordeFlagPickedup())
                return;
            if (GetHordeFlagPickerGUID() == Source->GetGUID())
            {
                SetHordeFlagPicker(0);
                Source->RemoveAurasDueToSpell(BG_WS_SPELL_WARSONG_FLAG);
            }
        }
        else
        {
            if (!this->IsAllianceFlagPickedup())
                return;
            if (GetAllianceFlagPickerGUID() == Source->GetGUID())
            {
                SetAllianceFlagPicker(0);
                Source->RemoveAurasDueToSpell(BG_WS_SPELL_SILVERWING_FLAG);
            }
        }
        return;
    }

    int32 message;
    uint8 type = 0;
    bool set = false;

    if (Source->GetBGTeam() == ALLIANCE)
    {
        if (!this->IsHordeFlagPickedup())
            return;
        if (GetHordeFlagPickerGUID() == Source->GetGUID())
        {
            SetHordeFlagPicker(0);
            Source->RemoveAurasDueToSpell(BG_WS_SPELL_WARSONG_FLAG);
            if (m_FlagDebuffState == 1)
              Source->RemoveAurasDueToSpell(BG_WS_SPELL_FOCUSED_ASSAULT);
            if (m_FlagDebuffState == 2)
              Source->RemoveAurasDueToSpell(BG_WS_SPELL_BRUTAL_ASSAULT);
            m_FlagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_GROUND;
            message = LANG_BG_WS_DROPPED_HF;
            type = CHAT_MSG_BG_SYSTEM_HORDE;
            Source->CastSpell(Source, BG_WS_SPELL_WARSONG_FLAG_DROPPED, true);
            set = true;
        }
    }
    else
    {
        if (!this->IsAllianceFlagPickedup())
            return;
        if (GetAllianceFlagPickerGUID() == Source->GetGUID())
        {
            SetAllianceFlagPicker(0);
            Source->RemoveAurasDueToSpell(BG_WS_SPELL_SILVERWING_FLAG);
            if (m_FlagDebuffState == 1)
              Source->RemoveAurasDueToSpell(BG_WS_SPELL_FOCUSED_ASSAULT);
            if (m_FlagDebuffState == 2)
              Source->RemoveAurasDueToSpell(BG_WS_SPELL_BRUTAL_ASSAULT);
            m_FlagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_GROUND;
            message = LANG_BG_WS_DROPPED_AF;
            type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
            Source->CastSpell(Source, BG_WS_SPELL_SILVERWING_FLAG_DROPPED, true);
            set = true;
        }
    }

    if (set)
    {
        Source->CastSpell(Source, SPELL_RECENTLY_DROPPED_FLAG, true);
        UpdateFlagState(Source->GetBGTeam(), BG_WS_FLAG_STATE_WAIT_RESPAWN);

        SendMessageToAll(message, type, Source->GetSession(), Source->GetGUID());

        if (Source->GetBGTeam() == ALLIANCE)
            UpdateWorldState(BG_WS_FLAG_UNK_HORDE, uint32(-1));
        else
            UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, uint32(-1));

        m_FlagsDropTimer[GetTeamIndexByTeamId(Source->GetBGTeam()) ? 0 : 1].Reset(BG_WS_FLAG_DROP_TIME);
    }
}

void BattleGroundWS::EventPlayerClickedOnFlag(Player *Source, GameObject* target_obj)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    int32 message;
    uint8 type = 0;
 
    //alliance flag picked up from base
    if (Source->GetBGTeam() == HORDE && this->GetFlagState(ALLIANCE) == BG_WS_FLAG_STATE_ON_BASE
        && this->m_BgObjects[BG_WS_OBJECT_A_FLAG] == target_obj->GetGUID())
    {
        m_AllianceFlagUpdate = time(NULL); // Set the time
        message = LANG_BG_WS_PICKEDUP_AF;
        type = CHAT_MSG_BG_SYSTEM_HORDE;
        PlaySoundToAll(BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP);
        SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_ONE_DAY);
        SetAllianceFlagPicker(Source->GetGUID());
        m_FlagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_PLAYER;
        //update world state to show correct flag carrier
        UpdateFlagState(HORDE, BG_WS_FLAG_STATE_ON_PLAYER);
        UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 1);
        Source->CastSpell(Source, BG_WS_SPELL_SILVERWING_FLAG, true);
        if (m_FlagDebuffState == 1)
            Source->CastSpell(Source, BG_WS_SPELL_FOCUSED_ASSAULT, true);
        if (m_FlagDebuffState == 2)
            Source->CastSpell(Source, BG_WS_SPELL_BRUTAL_ASSAULT, true);
        if (m_FlagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_PLAYER)
            m_BothFlagsKept = true;
    }

    //horde flag picked up from base
    if (Source->GetBGTeam() == ALLIANCE && this->GetFlagState(HORDE) == BG_WS_FLAG_STATE_ON_BASE
        && this->m_BgObjects[BG_WS_OBJECT_H_FLAG] == target_obj->GetGUID())
    {
        m_HordeFlagUpdate = time(NULL); // Set the time
        message = LANG_BG_WS_PICKEDUP_HF;
        type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        PlaySoundToAll(BG_WS_SOUND_HORDE_FLAG_PICKED_UP);
        SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_ONE_DAY);
        SetHordeFlagPicker(Source->GetGUID());
        m_FlagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_PLAYER;
        //update world state to show correct flag carrier
        UpdateFlagState(ALLIANCE, BG_WS_FLAG_STATE_ON_PLAYER);
        UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 1);
        Source->CastSpell(Source, BG_WS_SPELL_WARSONG_FLAG, true);
        if (m_FlagDebuffState == 1)
            Source->CastSpell(Source, BG_WS_SPELL_FOCUSED_ASSAULT, true);
        if (m_FlagDebuffState == 2)
            Source->CastSpell(Source, BG_WS_SPELL_BRUTAL_ASSAULT, true);
        if (m_FlagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_PLAYER)
            m_BothFlagsKept = true;
    }

    //Alliance flag on ground(not in base) (returned or picked up again from ground!)
    if (this->GetFlagState(ALLIANCE) == BG_WS_FLAG_STATE_ON_GROUND && Source->IsWithinDistInMap(target_obj, 10) && target_obj->GetGOInfo()->id == BG_OBJECT_A_FLAG_GROUND_WS_ENTRY)
    {
        if (Source->GetBGTeam() == ALLIANCE)
        {
            message = LANG_BG_WS_RETURNED_AF;
            type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
            UpdateFlagState(HORDE, BG_WS_FLAG_STATE_WAIT_RESPAWN);
            RespawnFlag(ALLIANCE, false);
            SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
            PlaySoundToAll(BG_WS_SOUND_FLAG_RETURNED);
            UpdatePlayerScore(Source, SCORE_FLAG_RETURNS, 1);
        }
        else
        {
            message = LANG_BG_WS_PICKEDUP_AF;
            type = CHAT_MSG_BG_SYSTEM_HORDE;
            PlaySoundToAll(BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP);
            SpawnBGObject(BG_WS_OBJECT_A_FLAG, RESPAWN_ONE_DAY);
            SetAllianceFlagPicker(Source->GetGUID());
            Source->CastSpell(Source, BG_WS_SPELL_SILVERWING_FLAG, true);
            m_FlagState[TEAM_ALLIANCE] = BG_WS_FLAG_STATE_ON_PLAYER;
            UpdateFlagState(HORDE, BG_WS_FLAG_STATE_ON_PLAYER);
            if (m_FlagDebuffState == 1)
              Source->CastSpell(Source,BG_WS_SPELL_FOCUSED_ASSAULT,true);
            if (m_FlagDebuffState == 2)
              Source->CastSpell(Source,BG_WS_SPELL_BRUTAL_ASSAULT,true);
            UpdateWorldState(BG_WS_FLAG_UNK_ALLIANCE, 1);
        }
        //called in HandleGameObjectUseOpcode:
        //target_obj->Delete();
    }

    //Horde flag on ground(not in base) (returned or picked up again)
    if (this->GetFlagState(HORDE) == BG_WS_FLAG_STATE_ON_GROUND && Source->IsWithinDistInMap(target_obj, 10) && target_obj->GetGOInfo()->id == BG_OBJECT_H_FLAG_GROUND_WS_ENTRY)
    {
        if (Source->GetBGTeam() == HORDE)
        {
            message = LANG_BG_WS_RETURNED_HF;
            type = CHAT_MSG_BG_SYSTEM_HORDE;
            UpdateFlagState(ALLIANCE, BG_WS_FLAG_STATE_WAIT_RESPAWN);
            RespawnFlag(HORDE, false);
            SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
            PlaySoundToAll(BG_WS_SOUND_FLAG_RETURNED);
            UpdatePlayerScore(Source, SCORE_FLAG_RETURNS, 1);
        }
        else
        {
            message = LANG_BG_WS_PICKEDUP_HF;
            type = CHAT_MSG_BG_SYSTEM_ALLIANCE;
            PlaySoundToAll(BG_WS_SOUND_HORDE_FLAG_PICKED_UP);
            SpawnBGObject(BG_WS_OBJECT_H_FLAG, RESPAWN_ONE_DAY);
            SetHordeFlagPicker(Source->GetGUID());
            Source->CastSpell(Source, BG_WS_SPELL_WARSONG_FLAG, true);
            m_FlagState[TEAM_HORDE] = BG_WS_FLAG_STATE_ON_PLAYER;
            UpdateFlagState(ALLIANCE, BG_WS_FLAG_STATE_ON_PLAYER);
            if (m_FlagDebuffState == 1)
              Source->CastSpell(Source,BG_WS_SPELL_FOCUSED_ASSAULT,true);
            if (m_FlagDebuffState == 2)
              Source->CastSpell(Source,BG_WS_SPELL_BRUTAL_ASSAULT,true);
            UpdateWorldState(BG_WS_FLAG_UNK_HORDE, 1);
        }
        //called in HandleGameObjectUseOpcode:
        //target_obj->Delete();
    }

    if (!type)
        return;

    SendMessageToAll(message, type, Source->GetSession(), Source->GetGUID());

    Source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
}

void BattleGroundWS::RemovePlayer(Player *plr, uint64 guid)
{
    // sometimes flag aura not removed :(
    if (IsAllianceFlagPickedup() && m_FlagKeepers[TEAM_ALLIANCE] == guid)
    {
        if (!plr)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: BattleGroundWS: Removing offline player who has the FLAG!!");
            SetAllianceFlagPicker(0);
            RespawnFlag(ALLIANCE, false);
        }
        else
            EventPlayerDroppedFlag(plr);
    }
    if (IsHordeFlagPickedup() && m_FlagKeepers[TEAM_HORDE] == guid)
    {
        if (!plr)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: BattleGroundWS: Removing offline player who has the FLAG!!");
            SetHordeFlagPicker(0);
            RespawnFlag(HORDE, false);
        }
        else
            EventPlayerDroppedFlag(plr);
    }
}

void BattleGroundWS::UpdateFlagState(PlayerTeam team, uint32 value)
{
    if (team == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_STATE_ALLIANCE, value);
    else
        UpdateWorldState(BG_WS_FLAG_STATE_HORDE, value);
}

void BattleGroundWS::UpdateTeamScore(PlayerTeam team)
{
    if (team == ALLIANCE)
        UpdateWorldState(BG_WS_FLAG_CAPTURES_ALLIANCE, GetTeamScore(team));
    else
        UpdateWorldState(BG_WS_FLAG_CAPTURES_HORDE, GetTeamScore(team));
}

void BattleGroundWS::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    //uint32 SpellId = 0;
    //uint64 buff_guid = 0;
    switch (Trigger)
    {
        case 3686:                                          // Alliance elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            //buff_guid = m_BgObjects[BG_WS_OBJECT_SPEEDBUFF_1];
            break;
        case 3687:                                          // Horde elixir of speed spawn. Trigger not working, because located inside other areatrigger, can be replaced by IsWithinDist(object, dist) in BattleGround::Update().
            //buff_guid = m_BgObjects[BG_WS_OBJECT_SPEEDBUFF_2];
            break;
        case 3706:                                          // Alliance elixir of regeneration spawn
            //buff_guid = m_BgObjects[BG_WS_OBJECT_REGENBUFF_1];
            break;
        case 3708:                                          // Horde elixir of regeneration spawn
            //buff_guid = m_BgObjects[BG_WS_OBJECT_REGENBUFF_2];
            break;
        case 3707:                                          // Alliance elixir of berserk spawn
            //buff_guid = m_BgObjects[BG_WS_OBJECT_BERSERKBUFF_1];
            break;
        case 3709:                                          // Horde elixir of berserk spawn
            //buff_guid = m_BgObjects[BG_WS_OBJECT_BERSERKBUFF_2];
            break;
        case 3646:                                          // Alliance Flag spawn
            if (m_FlagState[TEAM_HORDE] && !m_FlagState[TEAM_ALLIANCE])
                if (GetHordeFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source);
            break;
        case 3647:                                          // Horde Flag spawn
            if (m_FlagState[TEAM_ALLIANCE] && !m_FlagState[TEAM_HORDE])
                if (GetAllianceFlagPickerGUID() == Source->GetGUID())
                    EventPlayerCapturedFlag(Source);
            break;
        case 3649:                                          // unk1
        case 3688:                                          // unk2
        case 4628:                                          // unk3
        case 4629:                                          // unk4
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }

    //if(buff_guid)
    //    HandleTriggerBuff(buff_guid,Source);
}

bool BattleGroundWS::SetupBattleGround()
{
    // flags
    if (  !AddObject(BG_WS_OBJECT_A_FLAG, BG_OBJECT_A_FLAG_WS_ENTRY, 1540.423f, 1481.325f, 351.8284f, 3.089233f, 0, 0, 0.9996573f, 0.02617699f, BG_WS_FLAG_RESPAWN_TIME/1000)
        || !AddObject(BG_WS_OBJECT_H_FLAG, BG_OBJECT_H_FLAG_WS_ENTRY, 916.0226f, 1434.405f, 345.413f, 0.01745329f, 0, 0, 0.008726535f, 0.9999619f, BG_WS_FLAG_RESPAWN_TIME/1000)
        // buffs
        || !AddObject(BG_WS_OBJECT_SPEEDBUFF_1, BG_OBJECTID_SPEEDBUFF_ENTRY, 1449.93f, 1470.71f, 342.6346f, -1.64061f, 0, 0, 0.7313537f, -0.6819983f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_SPEEDBUFF_2, BG_OBJECTID_SPEEDBUFF_ENTRY, 1005.171f, 1447.946f, 335.9032f, 1.64061f, 0, 0, 0.7313537f, 0.6819984f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_REGENBUFF_1, BG_OBJECTID_REGENBUFF_ENTRY, 1317.506f, 1550.851f, 313.2344f, -0.2617996f, 0, 0, 0.1305263f, -0.9914448f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_REGENBUFF_2, BG_OBJECTID_REGENBUFF_ENTRY, 1110.451f, 1353.656f, 316.5181f, -0.6806787f, 0, 0, 0.333807f, -0.9426414f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_BERSERKBUFF_1, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1320.09f, 1378.79f, 314.7532f, 1.186824f, 0, 0, 0.5591929f, 0.8290376f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_BERSERKBUFF_2, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1139.688f, 1560.288f, 306.8432f, -2.443461f, 0, 0, 0.9396926f, -0.3420201f, BUFF_RESPAWN_TIME)
        // alliance gates
        || !AddObject(BG_WS_OBJECT_DOOR_A_1, BG_OBJECT_DOOR_A_1_WS_ENTRY, 1503.335f, 1493.466f, 352.1888f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_2, BG_OBJECT_DOOR_A_2_WS_ENTRY, 1492.478f, 1457.912f, 342.9689f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_3, BG_OBJECT_DOOR_A_3_WS_ENTRY, 1468.503f, 1494.357f, 351.8618f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_4, BG_OBJECT_DOOR_A_4_WS_ENTRY, 1471.555f, 1458.778f, 362.6332f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_5, BG_OBJECT_DOOR_A_5_WS_ENTRY, 1492.347f, 1458.34f, 342.3712f, -0.03490669f, 0, 0, 0.01745246f, -0.9998477f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_6, BG_OBJECT_DOOR_A_6_WS_ENTRY, 1503.466f, 1493.367f, 351.7352f, -0.03490669f, 0, 0, 0.01745246f, -0.9998477f, RESPAWN_IMMEDIATELY)
        // horde gates
        || !AddObject(BG_WS_OBJECT_DOOR_H_1, BG_OBJECT_DOOR_H_1_WS_ENTRY, 949.1663f, 1423.772f, 345.6241f, -0.5756807f, -0.01673368f, -0.004956111f, -0.2839723f, 0.9586737f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_H_2, BG_OBJECT_DOOR_H_2_WS_ENTRY, 953.0507f, 1459.842f, 340.6526f, -1.99662f, -0.1971825f, 0.1575096f, -0.8239487f, 0.5073641f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_H_3, BG_OBJECT_DOOR_H_3_WS_ENTRY, 949.9523f, 1422.751f, 344.9273f, 0.0f, 0, 0, 0, 1, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_H_4, BG_OBJECT_DOOR_H_4_WS_ENTRY, 950.7952f, 1459.583f, 342.1523f, 0.05235988f, 0, 0, 0.02617695f, 0.9996573f, RESPAWN_IMMEDIATELY)
       )
    {
        sLog.outLog(LOG_DB_ERR, "BatteGroundWS: Failed to spawn some object BattleGround not created!");
        return false;
    }

    WorldSafeLocsEntry const *sg = sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_MAIN_ALLIANCE);
    if (!sg || !AddSpiritGuide(WS_SPIRIT_MAIN_ALLIANCE, sg->x, sg->y, sg->z, 3.124139f, ALLIANCE))
    {
        sLog.outLog(LOG_DB_ERR, "BatteGroundWS: Failed to spawn Alliance spirit guide! BattleGround not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_MAIN_HORDE);
    if (!sg || !AddSpiritGuide(WS_SPIRIT_MAIN_HORDE, sg->x, sg->y, sg->z, 3.193953f, HORDE))
    {
        sLog.outLog(LOG_DB_ERR, "BatteGroundWS: Failed to spawn Horde spirit guide! BattleGround not created!");
        return false;
    }

    sLog.outDebug("BatteGroundWS: BG objects and spirit guides spawned");

    return true;
}

void BattleGroundWS::ResetBGSubclass()
{
    m_FlagKeepers[TEAM_ALLIANCE]     = 0;
    m_FlagKeepers[TEAM_HORDE]        = 0;
    m_DroppedFlagGUID[TEAM_ALLIANCE] = 0;
    m_DroppedFlagGUID[TEAM_HORDE]    = 0;
    m_FlagState[TEAM_ALLIANCE]       = BG_WS_FLAG_STATE_ON_BASE;
    m_FlagState[TEAM_HORDE]          = BG_WS_FLAG_STATE_ON_BASE;
    m_TeamScores[TEAM_ALLIANCE]      = 0;
    m_TeamScores[TEAM_HORDE]         = 0;

    /* Spirit nodes is static at this BG and then not required deleting at BG reset.
    if (m_BgCreatures[WS_SPIRIT_MAIN_ALLIANCE])
        DelCreature(WS_SPIRIT_MAIN_ALLIANCE);

    if (m_BgCreatures[WS_SPIRIT_MAIN_HORDE])
        DelCreature(WS_SPIRIT_MAIN_HORDE);
    */
}

void BattleGroundWS::HandleKillPlayer(Player *player, Player *killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);

    BattleGround::HandleKillPlayer(player, killer);
}

void BattleGroundWS::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{

    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if (itr == m_PlayerScores.end())                         // player not found
        return;

    switch (type)
    {
        case SCORE_FLAG_CAPTURES:                           // flags captured
            ((BattleGroundWGScore*)itr->second)->FlagCaptures += value;
            break;
        case SCORE_FLAG_RETURNS:                            // flags returned
            ((BattleGroundWGScore*)itr->second)->FlagReturns += value;
            break;
        default:
            BattleGround::UpdatePlayerScore(Source, type, value);
            break;
    }
}

WorldSafeLocsEntry const* BattleGroundWS::GetClosestGraveYard(float x, float y, float /*z*/, PlayerTeam team)
{
    //if status in progress, it returns main graveyards with spiritguides
    //else it will return the graveyard in the flagroom - this is especially good
    //if a player dies in preparation phase - then the player can't cheat
    //and teleport to the graveyard outside the flagroom
    //and start running around, while the doors are still closed
    if (team == ALLIANCE)
    {
        if (GetStatus() == STATUS_IN_PROGRESS)
            return sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_MAIN_ALLIANCE);
        else
            return sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_FLAGROOM_ALLIANCE);
    }
    else
    {
        if (GetStatus() == STATUS_IN_PROGRESS)
            return sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_MAIN_HORDE);
        else
            return sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_FLAGROOM_HORDE);
    }
}

void BattleGroundWS::FillInitialWorldStates(WorldPacket& data)
{
    data << uint32(BG_WS_FLAG_CAPTURES_ALLIANCE) << uint32(GetTeamScore(ALLIANCE));
    data << uint32(BG_WS_FLAG_CAPTURES_HORDE) << uint32(GetTeamScore(HORDE));

    if (m_FlagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_GROUND)
        data << uint32(BG_WS_FLAG_UNK_ALLIANCE) << uint32(-1);
    else if (m_FlagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_PLAYER)
        data << uint32(BG_WS_FLAG_UNK_ALLIANCE) << uint32(1);
    else
        data << uint32(BG_WS_FLAG_UNK_ALLIANCE) << uint32(0);

    if (m_FlagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_GROUND)
        data << uint32(BG_WS_FLAG_UNK_HORDE) << uint32(-1);
    else if (m_FlagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_PLAYER)
        data << uint32(BG_WS_FLAG_UNK_HORDE) << uint32(1);
    else
        data << uint32(BG_WS_FLAG_UNK_HORDE) << uint32(0);

    data << uint32(BG_WS_FLAG_CAPTURES_MAX) << uint32(BG_WS_MAX_TEAM_SCORE);

    if (m_FlagState[TEAM_HORDE] == BG_WS_FLAG_STATE_ON_PLAYER)
        data << uint32(BG_WS_FLAG_STATE_ALLIANCE) << uint32(2);
    else
        data << uint32(BG_WS_FLAG_STATE_ALLIANCE) << uint32(1);

    if (m_FlagState[TEAM_ALLIANCE] == BG_WS_FLAG_STATE_ON_PLAYER)
        data << uint32(BG_WS_FLAG_STATE_HORDE) << uint32(2);
    else
        data << uint32(BG_WS_FLAG_STATE_HORDE) << uint32(1);

}

