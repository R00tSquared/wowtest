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

#ifndef HELLGROUND_BATTLEGROUND_H
#define HELLGROUND_BATTLEGROUND_H

#include "Common.h"
#include "SharedDefines.h"
#include "MapManager.h"

class Creature;
class GameObject;
class Group;
class Player;
class WorldPacket;

struct WorldSafeLocsEntry;

//#define WIN_HONOR_COINT 100
//#define BG_REPORTS_TO_KICK 5
#define BG_REPORTS_TO_DEBUFF 2
#define BG_IDLE_AURA_DURATION 60 * 1.5 * 1000
#define BG_IDLE_AURA_DURATION_ALTERAC 60 * 3 * 1000
#define SPELL_AURA_BG_PLAYER_IDLE 43680

struct GroupQueueInfo;                                      // type predefinition
struct PlayerQueueInfo                                      // stores information for players in queue
{
    uint32  InviteTime;                                     // first invite time
    uint32  LastInviteTime;                                 // last invite time
    uint32  LastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    uint32  ClassMask;                                      // contains healer flag also                                   
    GroupQueueInfo* GroupInfo;                             // pointer to the associated groupqueueinfo
};

struct GroupQueueInfo                                       // stores information about the group in queue (also used when joined as solo!)
{
    std::map<uint64, PlayerQueueInfo*> Players;             // player queue info map
    PlayerTeam  Team;                                       // Player team (ALLIANCE/HORDE)
    BattleGroundTypeId BgTypeId;                            // battleground type id
    bool    IsRated;                                        // rated
    bool    IsEntryPointAnywhere;                           // used for premium-queued players
    uint8   ArenaType;                                      // 2v2, 3v3, 5v5 or 0 when BG
    uint32  ArenaTeamId;                                    // team id if rated match
    uint32  JoinTime;                                       // time when group was added (time in seconds since server starts)
    uint32  IsInvitedToBGInstanceGUID;                      // was invited to certain BG
    uint32  ArenaTeamRating;                                // if rated match, inited to the rating of the team
    uint32  OpponentsTeamRating;                            // for rated arena matches
    uint32  PremadeLeaderGUID; // just to track player.last_group_id, sets only if premade
    uint32  LeaderClassMask;
    bool    LeaderIsSemiHealer;
    const char* LeaderName;
    uint32 CurrentBGId; // if any member of group is in bg, this is set to bg id

    TeamId GetBGTeam()
    {
        return Team == HORDE ? TEAM_HORDE : TEAM_ALLIANCE;
    }
};

enum BattleGroundSounds
{
    SOUND_HORDE_WINS                = 8454,
    SOUND_ALLIANCE_WINS             = 8455,
    SOUND_BG_START                  = 3439,
    SOUND_BG_START_L70ETC           = 11803,
};

enum BattleGroundQuests
{
    SPELL_WS_QUEST_REWARD           = 43483,
    SPELL_AB_QUEST_REWARD           = 43484,
    SPELL_AV_QUEST_REWARD           = 43475,
    SPELL_AV_QUEST_KILLED_BOSS      = 23658,
    SPELL_EY_QUEST_REWARD           = 43477,
    SPELL_AB_QUEST_REWARD_4_BASES   = 24061,
    SPELL_AB_QUEST_REWARD_5_BASES   = 24064
};

enum BattleGroundSpells
{
    SPELL_WAITING_FOR_RESURRECT     = 2584,                 // Waiting to Resurrect
    SPELL_SPIRIT_HEAL_CHANNEL       = 22011,                // Spirit Heal Channel
    SPELL_SPIRIT_HEAL               = 22012,                // Spirit Heal
    SPELL_RESURRECTION_VISUAL       = 24171,                // Resurrection Impact Visual
    SPELL_ARENA_PREPARATION         = 32727,                // use this one, 32728 not correct
    SPELL_ALLIANCE_GOLD_FLAG        = 32724,
    SPELL_ALLIANCE_GREEN_FLAG       = 32725,
    SPELL_HORDE_GOLD_FLAG           = 35774,
    SPELL_HORDE_GREEN_FLAG          = 35775,
    SPELL_PREPARATION               = 44521,                // Preparation
    SPELL_SPIRIT_HEAL_MANA          = 44535,                // Spirit Heal
    SPELL_RECENTLY_DROPPED_FLAG     = 42792,                // Recently Dropped Flag
    SPELL_AURA_PLAYER_INACTIVE      = 43681,                // Inactive
    SPELL_SPIRIT_HEAL_PET_WARLOCK   = 54667,
};

enum BattleGroundTimeIntervals
{
    RESURRECTION_INTERVAL           = 30000,                // ms
    REMIND_INTERVAL                 = 30000,                // ms
    INVITATION_REMIND_TIME_BG       = 45000,                // ms
    INVITATION_REMIND_TIME_ARENA    = 45000,                // ms
    INVITE_ACCEPT_WAIT_TIME_BG      = 60000,                // ms
    INVITE_ACCEPT_WAIT_TIME_ARENA   = 60000,                // ms
    TIME_TO_AUTOREMOVE              = 120000,               // ms
    MAX_OFFLINE_TIME                = 60000,               // ms
    START_DELAY0                    = 120000,               // ms
    START_DELAY1                    = 60000,                // ms
    START_DELAY2                    = 30000,                // ms
    START_DELAY3                    = 15000,                // ms used only in arena
    RESPAWN_ONE_DAY                 = 86400,                // secs
    RESPAWN_IMMEDIATELY             = 0,                    // secs
    BUFF_RESPAWN_TIME               = 180,                  // secs
    BG_HONOR_SCORE_TICKS            = 330                   // points
};

enum BattleGroundBuffObjects
{
    BG_OBJECTID_SPEEDBUFF_ENTRY     = 179871,
    BG_OBJECTID_REGENBUFF_ENTRY     = 179904,
    BG_OBJECTID_BERSERKERBUFF_ENTRY = 179905
};

const uint32 Buff_Entries[3] = { BG_OBJECTID_SPEEDBUFF_ENTRY, BG_OBJECTID_REGENBUFF_ENTRY, BG_OBJECTID_BERSERKERBUFF_ENTRY };
// ALI/HORDE       WSG/AB/EY/ALTERAC/NAGRAND/RUINS/BLADE_EDGE           X Y Z
const float insidePos[2][7][3] = 
{
    {
        {1504,1476,352}, {1285,1281,-16}, {2515,1596,1265}, {784,-493,99}, {4030,2969,12}, {1278,1731,31}, {6290,285,5},
    },
    
    {
        {950,1439,345}, {708, 708, -17}, {1815,1539,1264}, {-1376,-540,55}, {4082,2871,12}, {1293,1601,31}, {6186,238,5},
    }
};

const float outsidePos[2][7][3] = 
{
    {
        {1462,1476,357}, {1278,1275,-18}, {2502,1597,1259}, {772,-492,98}, {4032,2965,12}, {1279,1724,34}, {6285,279,5},
    },
    
    {
        {989,1439,353}, {712,713,-20}, {1824,1538,1260}, {-1367,-531,53}, {4080,2876,12}, {1292,1607,34}, {6191,243,5},
    }
};

enum BattleGroundStatus
{
    STATUS_NONE         = 0,
    STATUS_WAIT_QUEUE   = 1,
    STATUS_WAIT_JOIN    = 2,
    STATUS_IN_PROGRESS  = 3,
    STATUS_WAIT_LEAVE   = 4                                 // custom
};

struct BattleGroundPlayer
{
    uint32  LastOnlineTime;                                 // for tracking and removing offline players from queue after 5 minutes
    PlayerTeam  Team;                                           // Player's team
	uint32	PlayedTime;
	uint32	ClassMask;
    std::string haship;
	bool	Rated;
};

struct BattleGroundObjectInfo
{
    BattleGroundObjectInfo() : object(NULL), timer(0), spellid(0) {}

    GameObject  *object;
    int32       timer;
    uint32      spellid;
};

// handle the queue types and bg types separately to enable joining queue for different sized arenas at the same time
enum BattleGroundQueueTypeId
{
    BATTLEGROUND_QUEUE_NONE   = 0,
    BATTLEGROUND_QUEUE_AV     = 1,
    BATTLEGROUND_QUEUE_WS     = 2,
    BATTLEGROUND_QUEUE_AB     = 3,
    BATTLEGROUND_QUEUE_EY     = 4,
    BATTLEGROUND_QUEUE_2v2    = 5,
    BATTLEGROUND_QUEUE_3v3    = 6, // solo 3v3
    BATTLEGROUND_QUEUE_5v5    = 7,
    //BATTLEGROUND_QUEUE_1v1    = 8,
    //BATTLEGROUND_QUEUE_3v3_SOLO = 9,
};
#define MAX_BATTLEGROUND_QUEUE_TYPES 8

enum BattleGroundBracketId                                  // bracketId for level ranges
{
    BG_BRACKET_ID_FIRST          = 0,                       // brackets start from specific BG min level and each include 10 levels range
    BG_BRACKET_ID_LAST           = 6,                       // so for start level 10 will be 10-19, 20-29, ...  all greater max bg level included in last breaket

    MAX_BATTLEGROUND_BRACKETS    = 7                        // used as one from values, so in enum
};

enum ScoreType
{
    SCORE_KILLING_BLOWS         = 1,
    SCORE_DEATHS                = 2,
    SCORE_HONORABLE_KILLS       = 3,
    SCORE_BONUS_HONOR           = 4,
    //EY, but in MSG_PVP_LOG_DATA opcode!
    SCORE_DAMAGE_DONE           = 5,
    SCORE_HEALING_DONE          = 6,
    //WS
    SCORE_FLAG_CAPTURES         = 7,
    SCORE_FLAG_RETURNS          = 8,
    //AB
    SCORE_BASES_ASSAULTED       = 9,
    SCORE_BASES_DEFENDED        = 10,
    //AV
    SCORE_GRAVEYARDS_ASSAULTED  = 11,
    SCORE_GRAVEYARDS_DEFENDED   = 12,
    SCORE_TOWERS_ASSAULTED      = 13,
    SCORE_TOWERS_DEFENDED       = 14,
    SCORE_MINES_CAPTURED        = 15,
    SCORE_LEADERS_KILLED        = 16,
    SCORE_SECONDARY_OBJECTIVES  = 17
    // TODO : implement them
};

enum ArenaType
{
    ARENA_TYPE_NONE         = 0,
    //ARENA_TYPE_1v1          = 1,
    ARENA_TYPE_2v2          = 2,
    ARENA_TYPE_3v3          = 3, // 3v3 soloqueue
    ARENA_TYPE_5v5          = 5
};

enum BattleGroundType
{
    TYPE_BATTLEGROUND     = 3,
    TYPE_ARENA            = 4
};

enum BattleGroundWinner
{
    WINNER_HORDE            = 0,
    WINNER_ALLIANCE         = 1,
    WINNER_NONE             = 2
};

#define BG_TEAMS_COUNT  2 // TeamId

class BattleGroundScore
{
    public:
        BattleGroundScore() : KillingBlows(0), Deaths(0), HonorableKills(0),
            BonusHonor(0), DamageDone(0), HealingDone(0)
        {}
        virtual ~BattleGroundScore() {}                     //virtual destructor is used when deleting score from scores map

        uint32 KillingBlows;
        uint32 Deaths;
        uint32 HonorableKills;
        uint32 BonusHonor;
        uint32 DamageDone;
        uint32 HealingDone;
};

typedef std::map<uint64, BattleGroundScore*> BattleGroundScoreMap;

enum BGHonorMode
{
    BG_NORMAL = 0,
    BG_BONUS  = 1,
    BG_HONOR_MODE_NUM
};

/*
This class is used to:
1. Add player to battleground
2. Remove player from battleground
3. some certain cases, same for all battlegrounds
4. It has properties same for all battlegrounds
*/
class HELLGROUND_IMPORT_EXPORT BattleGround
{
    friend class BattleGroundMgr;

    public:
        //uint32 getHonorMode() { return m_HonorMode; }
        /* Construction */
        BattleGround();
        /*BattleGround(const BattleGround& bg);*/
        virtual ~BattleGround();
        virtual void Update(uint32 diff);                   // must be implemented in BG subclass of BG specific update code, but must in begginning call parent version
        virtual bool SetupBattleGround()                    // must be implemented in BG subclass
        {
            return true;
        }
        void Reset();                                       // resets all common properties for battlegrounds
        virtual void ResetBGSubclass()                      // must be implemented in BG subclass
        {
        }

        /* Battleground */
        // Get methods:
        char const* GetName() const         { return m_Name; }
        BattleGroundTypeId GetTypeID() const { return m_TypeID; }
        BattleGroundBracketId GetBracketId() const { return m_BracketId; }
        uint32 GetQueueType() const         { return m_Queue_type; }
        uint32 GetBgInstanceId() const        { return m_InstanceID; }
        BattleGroundStatus GetStatus() const            { return m_Status; }
        uint32 GetStartTime() const         { return m_StartTime; }
        uint32 InProgressDuration() const    { return m_InProgressDuration; }
        uint32 GetEndTime() const           { return m_EndTime; }
        uint32 GetLastResurrectTime() const { return m_LastResurrectTime; }
        uint32 GetMaxPlayers() const        { return m_MaxPlayers; }
        uint32 GetMinPlayers() const        { return m_MinPlayers; }

        uint32 GetMinLevel() const          { return m_LevelMin; }
        uint32 GetMaxLevel() const          { return m_LevelMax; }

        uint32 GetMaxPlayersPerTeam() const { return m_MaxPlayersPerTeam; }
        uint32 GetMinPlayersPerTeam() const { return m_MinPlayersPerTeam; }

        int GetStartDelayTime() const       { return m_StartDelayTime; }
        uint8 GetArenaType() const          { return m_ArenaType; }
        uint8 GetWinner() const             { return m_Winner; }
        uint32 GetBattlemasterEntry() const;

        // Set methods:
        void SetName(char const* Name)      { m_Name = Name; }
        void SetTypeID(BattleGroundTypeId TypeID) { m_TypeID = TypeID; }
        void SetBracketId(BattleGroundBracketId ID) { m_BracketId = ID; }
        void SetQueueType(uint32 ID)        { m_Queue_type = ID; }
        void SetInstanceID(uint32 InstanceID) { m_InstanceID = InstanceID; }
        void SetStatus(BattleGroundStatus Status);
        void SetStartTime(uint32 Time)      { m_StartTime = Time; }
        void SetEndTime(uint32 Time)        { m_EndTime = Time; }
        void SetLastResurrectTime(uint32 Time) { m_LastResurrectTime = Time; }
        void SetMaxPlayers(uint32 MaxPlayers) { m_MaxPlayers = MaxPlayers; }
        void SetMinPlayers(uint32 MinPlayers) { m_MinPlayers = MinPlayers; }
        void SetLevelRange(uint32 min, uint32 max) { m_LevelMin = min; m_LevelMax = max; }
        void SetBracketLevelRange(uint32 min, uint32 max) { m_BracketLevelMin = min; m_BracketLevelMax = max; }
        void SetRated(bool state)           { m_IsRated = state; }
        void SetArenaType(uint8 type)       { m_ArenaType = type; }
        void SetArenaorBGType(bool _isArena) { m_IsArena = _isArena; }
        void SetWinner(uint8 winner)        { m_Winner = winner; }

        void CalculateBracketLevelRange()
        {
            m_BracketLevelMin = 10 * m_BracketId+ m_LevelMin;
            m_BracketLevelMax = (m_BracketLevelMin == 70 ? 70 : m_BracketLevelMin + 9);
        }

        void AnnounceBGStart();

        void ModifyStartDelayTime(int diff) { m_StartDelayTime -= diff; }  //}
        void SetStartDelayTime(int Time)    { m_StartDelayTime = Time; }   //} FIXME: convert to [Timer]

        void SetMaxPlayersPerTeam(uint32 MaxPlayers) { m_MaxPlayersPerTeam = MaxPlayers; }
        void SetMinPlayersPerTeam(uint32 MinPlayers) { m_MinPlayersPerTeam = MinPlayers; }

        void AddToBGFreeSlotQueue();                        //this queue will be useful when more battlegrounds instances will be available
        void RemoveFromBGFreeSlotQueue();                   //this method could delete whole BG instance, if another free is available

        void DecreaseInvitedCount(PlayerTeam team)      { (team == ALLIANCE) ? --m_InvitedAlliance : --m_InvitedHorde; }
        void IncreaseInvitedCount(PlayerTeam team)      { (team == ALLIANCE) ? ++m_InvitedAlliance : ++m_InvitedHorde; }
        uint32 GetInvitedCount(PlayerTeam team) const
        {
            if (team == ALLIANCE)
                return m_InvitedAlliance;
            else
                return m_InvitedHorde;
        }

        uint32 GetFreeBGSlots() const;
        //uint32 GetFreeSlotsForTeam(PlayerTeam Team) const;
        uint32 GetFreeBGSlotsForTeam(PlayerTeam team, bool over_limit = false) const;

        bool isArena() const        { return m_IsArena; }
        bool isBattleGround() const { return !m_IsArena; }
        bool isRated() const        { return m_IsRated; }

        typedef std::map<uint64, BattleGroundPlayer> BattleGroundPlayerMap;
        BattleGroundPlayerMap const& GetPlayers() const { return m_Players; }
        uint32 GetPlayersSize() const { return m_Players.size(); }
        uint32 GetRemovedPlayersSize() const { return m_RemovedPlayers.size(); }

        std::map<uint64, BattleGroundScore*>::const_iterator GetPlayerScoresBegin() const { return m_PlayerScores.begin(); }
        std::map<uint64, BattleGroundScore*>::const_iterator GetPlayerScoresEnd() const { return m_PlayerScores.end(); }
        uint32 GetPlayerScoresSize() const { return m_PlayerScores.size(); }

        uint32 GetReviveQueueSize() const { return m_ReviveQueue.size(); }

        void AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid);
        void RemovePlayerFromResurrectQueue(uint64 player_guid);

        void StartBattleGround();

        GameObject* GetBGObject(uint32 type);
        Creature* GetBGCreature(uint32 type);
        /* Location */
        void SetMapId(uint32 MapID) { m_MapId = MapID; }
        uint32 GetMapId() const { return m_MapId; }
        void SetMap(Map* map){ m_Map = map; }
        Map* GetMap();

        void SetTeamStartLoc(PlayerTeam PlayerTeam, float X, float Y, float Z, float O);
        void GetTeamStartLoc(PlayerTeam TeamID, float &X, float &Y, float &Z, float &O) const
        {
            TeamId idx = GetTeamIndexByTeamId(TeamID);
            X = m_TeamStartLocX[idx];
            Y = m_TeamStartLocY[idx];
            Z = m_TeamStartLocZ[idx];
            O = m_TeamStartLocO[idx];
        }

        /* Packet Transfer */
        // method that should fill worldpacket with actual world states (not yet implemented for all battlegrounds!)
        virtual void FillInitialWorldStates(WorldPacket& /*data*/) {}
        void SendPacketToTeam(PlayerTeam TeamID, WorldPacket *packet, Player *sender = NULL, bool self = true);
        void SendPacketToAll(WorldPacket *packet);
        void YellToAll(Creature* creature, const char* text, uint32 language);
        void PlaySoundToTeam(uint32 SoundID, PlayerTeam TeamID);
        void PlaySoundToAll(uint32 SoundID);
        void CastSpellOnTeam(uint32 SpellID, PlayerTeam TeamID);
        void RewardHonorToTeam(uint32 Honor, PlayerTeam TeamID);
        void RewardReputationToTeam(uint32 faction_id, uint32 Reputation, PlayerTeam TeamID);
        void RewardItem(Player *plr, uint32 itemId, uint32 count);
        void BGReward(Player *plr,bool win);
        void SendRewardMarkByMail(Player *plr,uint32 mark, uint32 count);
        void RewardQuest(Player *plr);
        void UpdateWorldState(uint32 Field, uint32 Value);
        void UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player *Source);
        /* winner - 0 - no one wins*/
        void EndBattleGround(PlayerTeam winner);
        void BlockMovement(Player *plr);

        //void PrepareMessageToAll(char const *format,...);
        //void SendMessageToAll(char const* text);
        void SendMessageToAll(int32 string_id, uint8 type, WorldSession* session, const uint64 guid, ...);
        void SendMessageToAll(int32 string_id, ...);
        //void SendMessageToTeam(uint32 team, char const* text);
        void SendMessageToTeam(PlayerTeam team, int32 string_id, ...);
        void SendNotifyToTeam(PlayerTeam team, int32 string_id, ...);
        void SendObjectiveComplete(uint32 id, PlayerTeam TeamID/*, float x, float y*/);

        /* Raid Group */
        Group *GetBgRaid(PlayerTeam TeamID) const { return TeamID == ALLIANCE ? m_BgRaids[TEAM_ALLIANCE] : m_BgRaids[TEAM_HORDE]; }
        void SetBgRaid(PlayerTeam TeamID, Group *bg_raid);

        virtual void UpdatePlayerScore(Player *Source, uint32 type, uint32 value);

        static TeamId GetTeamIndexByTeamId(PlayerTeam Team) { return Team == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
        uint32 GetPlayersCountByTeam(PlayerTeam Team) const { return m_PlayersCount[GetTeamIndexByTeamId(Team)]; }
        uint32 GetPlayersCount() const { return m_PlayersCount[TEAM_ALLIANCE] + m_PlayersCount[TEAM_HORDE]; }
        uint32 GetAlivePlayersCountByTeam(PlayerTeam Team) const;   // used in arenas to correctly handle death in spirit of redemption / last stand etc. (killer = killed) cases
        void UpdatePlayersCountByTeam(PlayerTeam Team, bool remove)
        {
            if (remove)
                --m_PlayersCount[GetTeamIndexByTeamId(Team)];
            else
                ++m_PlayersCount[GetTeamIndexByTeamId(Team)];
        }

        // used for rated arena battles
        void SetArenaTeamIdForTeam(PlayerTeam Team, uint32 ArenaTeamId) { m_ArenaTeamIds[GetTeamIndexByTeamId(Team)] = ArenaTeamId; }
        uint32 GetArenaTeamIdForTeam(PlayerTeam Team) const { return m_ArenaTeamIds[GetTeamIndexByTeamId(Team)]; }
        void SetArenaTeamRatingChangeForTeam(PlayerTeam Team, int32 RatingChange) { m_ArenaTeamRatingChanges[GetTeamIndexByTeamId(Team)] = RatingChange; }
        int32 GetArenaTeamRatingChangeForTeam(PlayerTeam Team) const { return m_ArenaTeamRatingChanges[GetTeamIndexByTeamId(Team)]; }
        bool Solo3v3ArenaFinishIfNotFull();

        /* Triggers handle */
        // must be implemented in BG subclass
        virtual void HandleAreaTrigger(Player* /*Source*/, uint32 /*Trigger*/) {}
        // must be implemented in BG subclass if need AND call base class generic code
        virtual void HandleKillPlayer(Player *player, Player *killer);
        virtual void HandleKillUnit(Creature* /*unit*/, Player* /*killer*/);

        /* Battleground events */
        /* these functions will return true event is possible, but false if player is bugger */
        virtual void EventPlayerDroppedFlag(Player* /*player*/) {}
        virtual void EventPlayerClickedOnFlag(Player* /*player*/, GameObject* /*target_obj*/) {}
        virtual void EventPlayerCapturedFlag(Player* /*player*/) {}
        void EventPlayerLoggedOut(Player* player);

        /* Death related */
        virtual WorldSafeLocsEntry const* GetClosestGraveYard(float /*x*/, float /*y*/, float /*z*/, PlayerTeam /*team*/)  { return NULL; }

        virtual void AddPlayer(Player *plr);                // must be implemented in BG subclass

        virtual void RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket, bool punish);
                                                            // can be extended in in BG subclass

        void HandleTriggerBuff(uint64 const& go_guid);
        void SetBonus(bool is_bonus);

        // TODO: make this protected:
        typedef std::vector<uint64> BGObjects;
        typedef std::vector<uint64> BGCreatures;
        BGObjects m_BgObjects;
        BGCreatures m_BgCreatures;
        void SpawnBGObject(uint32 type, uint32 respawntime);
        bool AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime = 0);
//        void SpawnBGCreature(uint32 type, uint32 respawntime);
        Creature* AddCreature(uint32 entry, uint32 type, PlayerTeam teamval, float x, float y, float z, float o, uint32 respawntime = 0);
        bool DelCreature(uint32 type);
        bool DelObject(uint32 type, bool setGoState = true);
        bool AddSpiritGuide(uint32 type, float x, float y, float z, float o, PlayerTeam team);
        int32 GetObjectType(uint64 guid);

        void DoorOpen(uint32 type);
        void DoorClose(uint32 type);
        const char *GetHellgroundString(int32 entry);

        virtual bool HandlePlayerUnderMap(Player * plr, float z);
        void AddOrSetPlayerToCorrectBgGroup(Player *plr, uint64 guid, PlayerTeam team);
        // since arenas can be AvA or Hvh, we have to get the "temporary" team of a player
        PlayerTeam GetPlayerTeam(uint64 guid);
        bool IsPlayerInBattleGround(uint64 guid);

        void SetDeleteThis(){ m_SetDeleteThis = true; }

        uint8 SetPlayerReady(uint64 guid);
        void RestorePet(Player* plr);

        uint32 GetTempArenaGuid() const { return m_TempArenaRatedGUID; }

		bool m_RatedBG;
		bool IsPlayerBGRated(uint64 guid);

        // @!bg_balancer init
        //player queued
        //player invited - add class to teamClassMaskCount at BattleGroundQueue::InviteGroupToBG
        //if player enters battle - ignore
        //if player leaves before enter - remove from teamClassMaskCount at BattleGroundQueue::RemovePlayer
        //if player leaves after enter - remove from teamClassMaskCount at BattleGround::RemovePlayerAtLeave
        std::map<uint32, uint32> teamClassMaskCount[BG_TEAMS_COUNT];
        uint32 GetTeamClassMaskCount(TeamId team, uint32 classMask) { return teamClassMaskCount[team][classMask]; };
        
        void AddRadiusNpc();
        std::string GetPlayerCountInfo();
        PlayerTeam GetBestBGTeamOnDistribute(GroupQueueInfo* gqi, bool only_check = false);

        uint32 GetTimeElapsed() { return m_TimeElapsedSinceBeggining; };

        uint32 m_playerMaxKillCount[BG_TEAMS_COUNT];
        TimeTracker m_doubleLoginCheckTimer;
        uint16 m_ShouldEndTime;
        bool m_alteracAnnounced;

        PlayerTeam GetWinningTeam();
        virtual uint32 GetTeamScore(PlayerTeam team) const;

    protected:
        //this method is called, when BG cannot spawn its own spirit guide, or something is wrong, It correctly ends BattleGround
        void EndNow();

        std::set<uint64>                        m_guidsReady[2];


        /* Scorekeeping */
                                                            // Player scores
        std::map<uint64, BattleGroundScore*>    m_PlayerScores;
        // must be implemented in BG subclass
        virtual void RemovePlayer(Player * /*player*/, uint64 /*guid*/) {}

        /* Player lists, those need to be accessible by inherited classes */
        BattleGroundPlayerMap  m_Players;
                                                            // Spirit Guide guid + Player list GUIDS
        std::map<uint64, std::vector<uint64> >  m_ReviveQueue;

        /*
        these are important variables used for starting messages
        */
        uint8 m_Events;

        bool   m_BuffChange;
        uint32 m_TimeElapsedSinceBeggining;
        uint32 m_score[2];                    //array that keeps general team scores, used to determine who gets most marks when bg ends prematurely

        BGHonorMode m_HonorMode;
        void BGStarted();
        void BGRealStats(Player* p = nullptr);

    private:
        /* Battleground */
        BattleGroundTypeId m_TypeID;
        BattleGroundBracketId m_BracketId;
        uint32 m_InstanceID;                                //BattleGround Instance's GUID!
        BattleGroundStatus m_Status;
        uint32 m_StartTime;
        uint32 m_InProgressDuration;
        uint32 m_EndTime;
        uint32 m_LastResurrectTime;
        uint32 m_Queue_type;
        uint8  m_ArenaType;                                 // 2=2v2, 3=3v3, 5=5v5
        bool   m_InBGFreeSlotQueue;                         // used to make sure that BG is only once inserted into the BattleGroundMgr.BGFreeSlotQueue[bgTypeId] deque
        bool   m_SetDeleteThis;                             // used for safe deletion of the bg after end / all players leave
        // this variable is not used .... it can be found in many other ways... but to store it in BG object instance is useless
        //uint8  m_BattleGroundType;                        // 3=BG, 4=arena
        //instead of uint8 (in previous line) is bool used
        bool   m_IsArena;
        uint8  m_Winner;                                    // 0=alliance, 1=horde, 2=none
        int32  m_StartDelayTime;							// time before gates are open
        bool   m_IsRated;                                   // is this battle rated? (only for arena!)
        bool   m_PrematureCountDown;
        uint32 m_PrematureCountDownTimer;
        char const *m_Name;
        bool   m_forceClose;

        uint64 m_progressStart;

        /* Player lists */
        std::vector<uint64> m_ResurrectQueue;               // Player GUID
        std::map<uint64, uint8> m_RemovedPlayers;           // uint8 is remove type (0 - bgqueue, 1 - bg, 2 - resurrect queue)

        /* Invited counters are useful for player invitation to BG - do not allow, if BG is started to one faction to have 2 more players than another faction */
        /* Invited counters will be changed only when removing already invited player from queue, removing player from battleground and inviting player to BG */
        /* Invited players counters*/
        uint32 m_InvitedAlliance;
        uint32 m_InvitedHorde;

        /* Raid Group */
        Group *m_BgRaids[2];                                // 0 - alliance, 1 - horde

        /* Players count by team */
        uint32 m_PlayersCount[2];

        /* Arena team ids by team */
        uint32 m_ArenaTeamIds[2];

        int32 m_ArenaTeamRatingChanges[2];

        /* Limits */
        uint32 m_LevelMin;
        uint32 m_LevelMax;
        uint32 m_BracketLevelMin;
        uint32 m_BracketLevelMax;
        uint32 m_MaxPlayersPerTeam;
        uint32 m_MaxPlayers;
        uint32 m_MinPlayersPerTeam;
        uint32 m_MinPlayers;

        /* Location */
        uint32 m_MapId;
        float m_TeamStartLocX[2];
        float m_TeamStartLocY[2];
        float m_TeamStartLocZ[2];
        float m_TeamStartLocO[2];
        Map * m_Map;

        /* Custom */
        uint32 m_TempArenaRatedGUID;

        uint32 m_ValidStartPositionTimer;

        std::map<uint32, std::map<uint32, uint8>> m_SamePlayerKillCount;
        uint8 GetSamePlayerKillCount(uint32 killer, uint32 victim);
};

#endif
