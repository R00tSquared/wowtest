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

#ifndef HELLGROUND_BATTLEGROUNDWS_H
#define HELLGROUND_BATTLEGROUNDWS_H

#include "BattleGround.h"

enum BG_WS_TimerOrScore
{
    BG_WS_MAX_TEAM_SCORE    = 3,
    BG_WS_FLAG_RESPAWN_TIME = 23000,
    BG_WS_FLAG_DROP_TIME    = 10000,
    BG_WS_SPELL_FORCE_TIME  = 600000,
    BG_WS_SPELL_BRUTAL_TIME = 900000,
    BG_WS_FLAG_UPDATE_TIME  = 45
};

enum BG_WS_Sound
{
    BG_WS_SOUND_FLAG_CAPTURED_ALLIANCE  = 8173,
    BG_WS_SOUND_FLAG_CAPTURED_HORDE     = 8213,
    BG_WS_SOUND_FLAG_PLACED             = 8232,
    BG_WS_SOUND_FLAG_RETURNED           = 8192,
    BG_WS_SOUND_HORDE_FLAG_PICKED_UP    = 8212,
    BG_WS_SOUND_ALLIANCE_FLAG_PICKED_UP = 8174,
    BG_WS_SOUND_FLAGS_RESPAWNED         = 8232
};

enum BG_WS_SpellId
{
    BG_WS_SPELL_WARSONG_FLAG            = 23333,
    BG_WS_SPELL_WARSONG_FLAG_DROPPED    = 23334,
    BG_WS_SPELL_SILVERWING_FLAG         = 23335,
    BG_WS_SPELL_SILVERWING_FLAG_DROPPED = 23336,
    BG_WS_SPELL_FOCUSED_ASSAULT         = 46392,
    BG_WS_SPELL_BRUTAL_ASSAULT          = 46393
};

enum BG_WS_WorldStates
{
    BG_WS_FLAG_UNK_ALLIANCE       = 1545,
    BG_WS_FLAG_UNK_HORDE          = 1546,
//    FLAG_UNK                      = 1547,
    BG_WS_FLAG_CAPTURES_ALLIANCE  = 1581,
    BG_WS_FLAG_CAPTURES_HORDE     = 1582,
    BG_WS_FLAG_CAPTURES_MAX       = 1601,
    BG_WS_FLAG_STATE_HORDE        = 2338,
    BG_WS_FLAG_STATE_ALLIANCE     = 2339
};

enum BG_WS_ObjectTypes
{
    BG_WS_OBJECT_DOOR_A_1       = 0,
    BG_WS_OBJECT_DOOR_A_2       = 1,
    BG_WS_OBJECT_DOOR_A_3       = 2,
    BG_WS_OBJECT_DOOR_A_4       = 3,
    BG_WS_OBJECT_DOOR_A_5       = 4,
    BG_WS_OBJECT_DOOR_A_6       = 5,
    BG_WS_OBJECT_DOOR_H_1       = 6,
    BG_WS_OBJECT_DOOR_H_2       = 7,
    BG_WS_OBJECT_DOOR_H_3       = 8,
    BG_WS_OBJECT_DOOR_H_4       = 9,
    BG_WS_OBJECT_A_FLAG         = 10,
    BG_WS_OBJECT_H_FLAG         = 11,
    BG_WS_OBJECT_SPEEDBUFF_1    = 12,
    BG_WS_OBJECT_SPEEDBUFF_2    = 13,
    BG_WS_OBJECT_REGENBUFF_1    = 14,
    BG_WS_OBJECT_REGENBUFF_2    = 15,
    BG_WS_OBJECT_BERSERKBUFF_1  = 16,
    BG_WS_OBJECT_BERSERKBUFF_2  = 17,
    BG_WS_OBJECT_MAX            = 18
};

enum BG_WS_ObjectEntry
{
    BG_OBJECT_DOOR_A_1_WS_ENTRY          = 179918,
    BG_OBJECT_DOOR_A_2_WS_ENTRY          = 179919,
    BG_OBJECT_DOOR_A_3_WS_ENTRY          = 179920,
    BG_OBJECT_DOOR_A_4_WS_ENTRY          = 179921,
    BG_OBJECT_DOOR_A_5_WS_ENTRY          = 180322,
    BG_OBJECT_DOOR_A_6_WS_ENTRY          = 180322,
    BG_OBJECT_DOOR_H_1_WS_ENTRY          = 179916,
    BG_OBJECT_DOOR_H_2_WS_ENTRY          = 179917,
    BG_OBJECT_DOOR_H_3_WS_ENTRY          = 180322,
    BG_OBJECT_DOOR_H_4_WS_ENTRY          = 180322,
    BG_OBJECT_A_FLAG_WS_ENTRY            = 179830,
    BG_OBJECT_H_FLAG_WS_ENTRY            = 179831,
    BG_OBJECT_A_FLAG_GROUND_WS_ENTRY     = 179785,
    BG_OBJECT_H_FLAG_GROUND_WS_ENTRY     = 179786
};

enum BG_WS_FlagState
{
    BG_WS_FLAG_STATE_ON_BASE      = 0,
    BG_WS_FLAG_STATE_WAIT_RESPAWN = 1,
    BG_WS_FLAG_STATE_ON_PLAYER    = 2,
    BG_WS_FLAG_STATE_ON_GROUND    = 3
};

enum BG_WS_Graveyards
{
    WS_GRAVEYARD_FLAGROOM_ALLIANCE = 769,
    WS_GRAVEYARD_FLAGROOM_HORDE    = 770,
    WS_GRAVEYARD_MAIN_ALLIANCE     = 771,
    WS_GRAVEYARD_MAIN_HORDE        = 772
};

enum BG_WS_CreatureTypes
{
    WS_SPIRIT_MAIN_ALLIANCE   = 0,
    WS_SPIRIT_MAIN_HORDE      = 1,
    BG_CREATURES_MAX_WS       = 9
};

class BattleGroundWGScore : public BattleGroundScore
{
    public:
        BattleGroundWGScore() : FlagCaptures(0), FlagReturns(0) {};
        virtual ~BattleGroundWGScore() {};
        uint32 FlagCaptures;
        uint32 FlagReturns;
};

class BattleGroundWS : public BattleGround
{
    friend class BattleGroundMgr;

    public:
        /* Construction */
        BattleGroundWS();
        ~BattleGroundWS();
        void Update(uint32 diff);

        /* inherited from BattlegroundClass */
        virtual void AddPlayer(Player *plr);

        /* BG Flags */
        uint64 GetAllianceFlagPickerGUID() const    { return m_FlagKeepers[TEAM_ALLIANCE]; }
        uint64 GetHordeFlagPickerGUID() const       { return m_FlagKeepers[TEAM_HORDE]; }
        void SetAllianceFlagPicker(uint64 guid)     { m_FlagKeepers[TEAM_ALLIANCE] = guid; }
        void SetHordeFlagPicker(uint64 guid)        { m_FlagKeepers[TEAM_HORDE] = guid; }
        bool IsAllianceFlagPickedup() const         { return m_FlagKeepers[TEAM_ALLIANCE] != 0; }
        bool IsHordeFlagPickedup() const            { return m_FlagKeepers[TEAM_HORDE] != 0; }
        void RespawnFlag(PlayerTeam Team, bool captured);
        void RespawnFlagAfterDrop(PlayerTeam Team);
        uint8 GetFlagState(PlayerTeam team)             { return m_FlagState[GetTeamIndexByTeamId(team)]; }
        void AddTimedAura(uint32 aura);
        void RemoveTimedAura(uint32 aura);
        bool IsBrutalTimerDone;
        bool IsForceTimerDone;

        /* Battleground Events */
        virtual void EventPlayerDroppedFlag(Player *Source);
        virtual void EventPlayerClickedOnFlag(Player *Source, GameObject* target_obj);
        virtual void EventPlayerCapturedFlag(Player *Source);
        
        void RemovePlayer(Player *plr, uint64 guid);
        void HandleAreaTrigger(Player *Source, uint32 Trigger);
        void HandleKillPlayer(Player *player, Player *killer);
        bool SetupBattleGround();
        virtual void ResetBGSubclass();
        virtual WorldSafeLocsEntry const* GetClosestGraveYard(float x, float y, float z, PlayerTeam team);

        void UpdateFlagState(PlayerTeam team, uint32 value);
        void UpdateTeamScore(PlayerTeam team);
        void UpdatePlayerScore(Player *Source, uint32 type, uint32 value);
        void SetDroppedFlagGUID(uint64 guid, PlayerTeam TeamID)  { m_DroppedFlagGUID[GetTeamIndexByTeamId(TeamID)] = guid;}
        uint64 GetDroppedFlagGUID(PlayerTeam TeamID)             { return m_DroppedFlagGUID[GetTeamIndexByTeamId(TeamID)];}
        virtual void FillInitialWorldStates(WorldPacket& data);

        /* Scorekeeping */
        virtual uint32 GetTeamScore(PlayerTeam team) const            { return m_TeamScores[GetTeamIndexByTeamId(team)]; }
        void AddPoint(PlayerTeam TeamID, uint32 Points = 1)     { m_TeamScores[GetTeamIndexByTeamId(TeamID)] += Points; m_score[GetTeamIndexByTeamId(TeamID)] =  m_TeamScores[GetTeamIndexByTeamId(TeamID)];}
        void SetTeamPoint(PlayerTeam TeamID, uint32 Points = 0) { m_TeamScores[GetTeamIndexByTeamId(TeamID)] = Points; m_score[GetTeamIndexByTeamId(TeamID)] =  m_TeamScores[GetTeamIndexByTeamId(TeamID)];}
        void RemovePoint(PlayerTeam TeamID, uint32 Points = 1)  { m_TeamScores[GetTeamIndexByTeamId(TeamID)] -= Points; m_score[GetTeamIndexByTeamId(TeamID)] =  m_TeamScores[GetTeamIndexByTeamId(TeamID)]; }

        uint64 m_AllianceFlagUpdate;
        uint64 m_HordeFlagUpdate;
    private:
        uint64 m_FlagKeepers[2];                            // 0 - alliance, 1 - horde
        uint64 m_DroppedFlagGUID[2];
        uint8 m_FlagState[2];                               // for checking flag state
        uint32 m_TeamScores[2];
        Timer m_FlagsTimer[2];
        Timer m_FlagsDropTimer[2];

        int32 m_FlagSpellForceTimer;
        int32 m_FlagSpellBrutalTimer;
        bool m_BothFlagsKept;
        uint8 m_FlagDebuffState;                            // 0 - no debuffs, 1 - focused assault, 2 - brutal assault
};
#endif

