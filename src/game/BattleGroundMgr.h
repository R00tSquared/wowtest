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

#ifndef HELLGROUND_BATTLEGROUNDMGR_H
#define HELLGROUND_BATTLEGROUNDMGR_H

#include "ace/Singleton.h"

#include "Common.h"
#include "BattleGround.h"
#include "ArenaTeam.h"

typedef std::map<uint32, BattleGround*> BattleGroundSet;

//this container can't be deque, because deque doesn't like removing the last element - if you remove it, it invalidates next iterator and crash appears
typedef std::list<BattleGround*> BGFreeSlotQueueType;

#define MAX_BATTLEGROUND_QUEUES 7                           // for level ranges 10-19, 20-29, 30-39, 40-49, 50-59, 60-69, 70+
#define SOLO_3v3_MIN_PLAYERS 3 // solo3v3debug value (i dont want to create getters because we have too much checks in code)

typedef UNORDERED_MAP<uint32, BattleGroundTypeId> BattleMastersMap;

#define BATTLEGROUND_ARENA_POINT_DISTRIBUTION_DAY   86400   // seconds in a day

enum BattleGroundQueueGroupTypes
{
    BG_QUEUE_PREMADE   = 0,
    BG_QUEUE_NORMAL    = 1,
};
#define BG_QUEUE_GROUP_TYPES_COUNT 2

// LADDER
typedef struct
{
    std::string PlayerName; // same as TeamName (if player name wasn't changed)
    uint32 Wins;
    uint32 Loses;
    uint32 Rating;
    uint32 Race;
    uint32 Class;
} Ladder_PlayerInfo;

#define LADDER_MAX_MEMBERS_CNT (ARENA_TYPE_3v3 * 2)

typedef struct
{
    std::string TeamName;
    uint32 Id;
    uint32 Wins;
    uint32 Loses;
    uint32 Rating;
    Ladder_PlayerInfo MembersInfo[LADDER_MAX_MEMBERS_CNT]; // yea it takes more space than needed, but we can have 2 callbacks instead of one per ladder
} Ladder_TeamInfo;

#define LADDER_CNT 10

class BattleGround;
class BattleGroundQueue
{
    public:
        BattleGroundQueue();
        ~BattleGroundQueue();

        void Update(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint8 arenaType = 0, bool isRated = false, uint32 minRating = 0);

        void FillPlayersToBG(BattleGround* bg, BattleGroundBracketId bracket_id);
        //bool CheckPremadeMatch(BattleGroundBracketId bracket_id, uint32 MaxPlayersPerTeam, uint32 MinPlayersPerTeam);
        void TryStartBattleground(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint32 minPlayers, uint32 maxPlayers);
        //bool CheckSkirmishForSameFaction(BattleGroundBracketId bracket_id, uint32 minPlayersPerTeam);
        GroupQueueInfo * AddGroup(Player* leader, BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint8 ArenaType, bool isRated, bool isEntryPointAnywhere, bool isPremade, uint32 ArenaRating, uint32 ArenaTeamId = 0);
        void AddPlayer(Player *plr, GroupQueueInfo *ginfo);
        void RemovePlayer(const uint64& guid, bool decreaseInvitedCount);
        void DecreaseGroupLength(uint32 queueId, uint32 AsGroup);
        void BGEndedRemoveInvites(BattleGround * bg);

        typedef std::map<uint64, PlayerQueueInfo> QueuedPlayersMap;
        QueuedPlayersMap m_QueuedPlayers;

        //we need constant add to begin and constant remove / add from the end, therefore deque suits our problem well
        typedef std::list<GroupQueueInfo*> GroupsQueueType;

        /*
        This two dimensional array is used to store All queued groups
        First dimension specifies the bgTypeId
        Second dimension specifies the player's group types -
             BG_QUEUE_PREMADE_ALLIANCE  is used for premade alliance groups and alliance rated arena teams
             BG_QUEUE_PREMADE_HORDE     is used for premade horde groups and horde rated arena teams
             BG_QUEUE_NORMAL_ALLIANCE   is used for normal (or small) alliance groups or non-rated arena matches
             BG_QUEUE_NORMAL_HORDE      is used for normal (or small) horde groups or non-rated arena matches
        */
        GroupsQueueType m_QueuedGroups[MAX_BATTLEGROUND_BRACKETS][BG_QUEUE_GROUP_TYPES_COUNT];

        // class to select and invite groups to bg
        class SelectionPool
        {
        public:
            void Init();
            bool AddGroup(GroupQueueInfo *ginfo, uint32 free_slots);
            bool KickGroup(uint32 size);
            uint32 GetPlayerCount() const {return PlayerCount;}
        public:
            GroupsQueueType SelectedGroups;
        private:
            uint32 PlayerCount;
        };

        //one selection pool for horde, other one for alliance
        SelectionPool m_SelectionPools;

        typedef ACE_Atomic_Op<ACE_Thread_Mutex, uint32> atomicUInt32;
        atomicUInt32 queuedPlayersCount[MAX_BATTLEGROUND_BRACKETS];

        ACE_Thread_Mutex m_mutex;

        void TryStartSolo3v3Arena(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id);
        void TryStart2v2Arena(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id);
        void TryStart2v2TestArena(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id);
        //std::multimap<uint32, GroupQueueInfo*> RandomizeSolo3v3Arena(BattleGroundBracketId bracket_id);
        bool CreateTempArenaTeamForQueue(ArenaTeam *arenaTeams[]);

        uint32 GetQueuedPlayersCount(BattleGroundBracketId bracketId)
        {
            if (bracketId >= MAX_BATTLEGROUND_BRACKETS)
                return 0;

            return queuedPlayersCount[bracketId].value();
        }

        void IncreaseQueuedPlayersCount(BattleGroundBracketId bracketId) 
        {
            if (bracketId < MAX_BATTLEGROUND_BRACKETS) ++queuedPlayersCount[bracketId];
        }

        void DecreaseQueuedPlayersCount(BattleGroundBracketId bracketId)
        {
            if (bracketId < MAX_BATTLEGROUND_BRACKETS) --queuedPlayersCount[bracketId];
        }
    private:

        bool InviteGroupToBG(GroupQueueInfo * ginfo, BattleGround * bg, PlayerTeam side);
};

/*
    This class is used to invite player to BG again, when minute lasts from his first invitation
    it is capable to solve all possibilities
*/
class BGQueueInviteEvent : public BasicEvent
{
    public:
        BGQueueInviteEvent(const uint64& pl_guid, uint32 BgInstanceGUID, BattleGroundTypeId BgTypeId) :
          m_PlayerGuid(pl_guid), m_BgInstanceGUID(BgInstanceGUID), m_BgTypeId(BgTypeId)
          {
          };
        virtual ~BGQueueInviteEvent() {};

        virtual bool Execute(uint64 e_time, uint32 p_time);
        virtual void Abort(uint64 e_time);
    private:
        uint64 m_PlayerGuid;
        uint32 m_BgInstanceGUID;
        BattleGroundTypeId m_BgTypeId;
};

/*
    This class is used to remove player from BG queue after 2 minutes from first invitation
*/
class BGQueueRemoveEvent : public BasicEvent
{
    public:
        BGQueueRemoveEvent(const uint64& pl_guid, uint32 bgInstanceGUID, BattleGroundTypeId BgTypeId, PlayerTeam playersTeam)
            : m_PlayerGuid(pl_guid), m_BgInstanceGUID(bgInstanceGUID), m_BgTypeId(BgTypeId), m_PlayersTeam(playersTeam)
        {}

        virtual ~BGQueueRemoveEvent() {}

        virtual bool Execute(uint64 e_time, uint32 p_time);
        virtual void Abort(uint64 e_time);
    private:
        uint64 m_PlayerGuid;
        uint32 m_BgInstanceGUID;
        PlayerTeam m_PlayersTeam;
        BattleGroundTypeId m_BgTypeId;
};

/* Flag of allowed spec (corresponds to Id of a spec in DB) and spell_id on which this spec is identified*/
struct ArenaSpecId
{
    uint64 spec_flag;
    uint32 spell_id;
};

struct entry_class_arr
{
    uint32* analog_entry_arr;
    uint32* analog_class_arr;
    uint32 size;
};

typedef std::pair<uint32, uint32> arena_items_to_log;

/*id copied from Shop*/
#define GO_BACK_ITEM_ARENA_ITEMS_EXCHANGE 693103

static TeamId GetTeamIndexByTeamId(uint32 Team) { return Team == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
static TeamId GetOtherTeam(TeamId team) { return (team == TEAM_HORDE ? TEAM_ALLIANCE : TEAM_HORDE); }
static PlayerTeam GetOtherTeam(PlayerTeam team) { return team ? ((team == ALLIANCE) ? HORDE : ALLIANCE) : TEAM_NONE; }

class BattleGroundMgr
{
    friend class ACE_Singleton<BattleGroundMgr, ACE_Null_Mutex>;
    BattleGroundMgr();

    public:
        ~BattleGroundMgr();

        void Update(uint32 diff);
        void ScheduleQueueUpdate(BattleGroundQueueTypeId bgQueueTypeId, BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id);

        /* Packet Building */
        void BuildPlayerJoinedBattleGroundPacket(WorldPacket *data, Player *plr);
        void BuildPlayerLeftBattleGroundPacket(WorldPacket *data, const uint64& guid);
        void BuildBattleGroundListPacket(WorldPacket *data, ObjectGuid guid, Player *plr, BattleGroundTypeId bgTypeId);
        void BuildGroupJoinedBattlegroundPacket(WorldPacket *data, BattleGroundTypeId bgTypeId);
        void BuildUpdateWorldStatePacket(WorldPacket *data, uint32 field, uint32 value);
        void BuildPvpLogDataPacket(WorldPacket *data, BattleGround *bg);
        void BuildBattleGroundStatusPacket(WorldPacket *data, BattleGround *bg, PlayerTeam team, uint8 QueueSlot, uint8 StatusID, uint32 Time1, uint32 Time2, uint32 arenatype = 0, uint8 israted = 0);
        void BuildPlaySoundPacket(WorldPacket *data, uint32 soundid);

        void SendAreaSpiritHealerQueryOpcode(Player *pl, BattleGround *bg, const uint64& guid);

        /* Player invitation */
        // called from Queue update, or from Addplayer to queue
        void InvitePlayer(Player* plr, BattleGroundQueueTypeId bgQueueTypeId, uint32 bgInstanceGUID, BattleGroundTypeId bgTypeId, PlayerTeam team);

        /* Battlegrounds */
        BattleGround* GetBattleGround(uint32 InstanceID, BattleGroundTypeId bgTypeId); //there must be uint32 because MAX_BATTLEGROUND_TYPE_ID means unknown

        BattleGround* GetBattleGroundTemplate(BattleGroundTypeId bgTypeId);
        BattleGround* CreateNewBattleGround(BattleGroundTypeId bgTypeId, BattleGroundBracketId bracket_id, uint8 arenaType, bool isRated);

        uint32 CreateBattleGround(BattleGroundTypeId bgTypeId, bool IsArena, uint32 MinPlayersPerTeam, uint32 MaxPlayersPerTeam, uint32 LevelMin, uint32 LevelMax, char* BattleGroundName, uint32 MapID, float Team1StartLocX, float Team1StartLocY, float Team1StartLocZ, float Team1StartLocO, float Team2StartLocX, float Team2StartLocY, float Team2StartLocZ, float Team2StacOrtLo);

        void AddBattleGround(uint32 InstanceID, BattleGroundTypeId bgTypeId, BattleGround* BG) { m_BattleGrounds[bgTypeId][InstanceID] = BG; };
        void RemoveBattleGround(uint32 instanceID, BattleGroundTypeId bgTypeId) { m_BattleGrounds[bgTypeId].erase(instanceID); }

        void CreateInitialBattleGrounds();
        void DeleteAllBattleGrounds();

        bool SendToBattleGround(Player *pl, uint32 InstanceID, BattleGroundTypeId bgTypeId);

        /* Battleground queues */
        //these queues are instantiated when creating BattlegroundMrg
        BattleGroundQueue m_BattleGroundQueues[MAX_BATTLEGROUND_QUEUE_TYPES]; // public, because we need to access them in BG handler code

        BGFreeSlotQueueType BGFreeSlotQueue[MAX_BATTLEGROUND_TYPE_ID];

        uint32 GetMaxRatingDifference() const;
        uint32 GetRatingDiscardTimer()  const;
        uint32 GetPrematureFinishTime() const;

        bool IsPrematureFinishTimerEnabled() const;
        bool IsWSGEndAfterEnabled() const;
        uint32 GetWSGEndAfterTime() const;
        bool IsAVEndAfterEnabled() const;
        uint32 GetAVEndAfterTime() const;
        uint32 GetArenaEndAfterTime() const;

        void InitAutomaticArenaPointDistribution();
        void DistributeArenaPoints();
        void ToggleTesting();

        BattleGroundTypeId GetDebugArenaId() { return debugArenaId; }
        void SetDebugArenaId(BattleGroundTypeId id);

        void LoadBattleMastersEntry();
        void LoadArenaRestrictions();
        void LoadArenaItemsLogging();
        bool ShouldArenaItemLog(uint32 entry, uint32 cost)
        {
            arena_items_to_log i = std::make_pair(entry, cost);
            if (m_arena_items_to_log.find(i) != m_arena_items_to_log.end())
                return true;
            return false;
        }
        BattleGroundTypeId GetBattleMasterBG(uint32 entry) const
        {
            BattleMastersMap::const_iterator itr = mBattleMastersMap.find(entry);
            if (itr != mBattleMastersMap.end())
                return itr->second;
            return BATTLEGROUND_TYPE_NONE;
        }

        
        bool isTesting() const { return m_Testing; }

        static bool IsArenaType(BattleGroundTypeId bgTypeId);
        static bool IsBattleGroundType(BattleGroundTypeId bgTypeId) { return !BattleGroundMgr::IsArenaType(bgTypeId); }
        static BattleGroundQueueTypeId BGQueueTypeId(BattleGroundTypeId bgTypeId, uint8 arenaType);
        static BattleGroundTypeId BGTemplateId(BattleGroundQueueTypeId bgQueueTypeId);
        static ArenaType BGArenaType(BattleGroundQueueTypeId bgQueueTypeId);

        HELLGROUND_EXPORT static bool IsBGWithBonus(BattleGroundTypeId bgTypeId);
        HELLGROUND_EXPORT static bool IsBGEventActive(BattleGroundTypeId bgTypeId);
        /*After restart there is no 'ending' status battlegrounds. This returns true for BGs with bonuses that are still played at the moment*/
        HELLGROUND_EXPORT bool IsBGEventEnding(BattleGroundTypeId bgTypeId) { return m_BGStillActive == bgTypeId;  }
        void SetBGEventEnding(uint16 event_id);
        void CheckBGEventAllEnded();

        int32 inArenasCount[3];

        static bool IsArenaTypeRestrictedPVE(uint8 arenatype);
        /* ToDo: Trentone: Make this method trigger-like so it will save arena_restricted in Player, but only recalculate it on item equip.*/
        bool IsPlayerArenaRestricted(Player* plr, uint8 arenatype);
        HELLGROUND_IMPORT_EXPORT bool IsPlayerArenaItemRestricted(Player* plr);
        bool IsItemArenaRestricted(uint32 itemId, const Player* const plr);
        HELLGROUND_IMPORT_EXPORT bool IsItemArenaRestricted(uint32 itemId, uint64 const& specFlag);
        // ToDo: Trentone: Spec could also be trigger-like after all the talents were learned
        HELLGROUND_IMPORT_EXPORT void ArenaRestrictedGetPlayerSpec(const Player* const, uint64 &specFlag);

        HELLGROUND_IMPORT_EXPORT void ArenaRestrictedSendSwapList(Creature* sender, Player* plr, uint32 item_slot);
        //void ArenaRestrictedHandleBuyPacket(Creature* sender, Player*plr, uint32 buy_item_id, ItemPrototype const *pProto);

        HELLGROUND_EXPORT static BattleGroundMgr* getBattleGroundMgrFromScripts();

        static ArenaType getArenaTypeBySlot(uint8 arenaSlot);

        Ladder_TeamInfo const* getLadder(uint32 arena_type) const { return &(m_ladder[arena_type-1][0]); };
        Ladder_TeamInfo* getLadderForModify(uint32 arena_type) { return &(m_ladder[arena_type - 1][0]); };

        //static uint8 solo3v3MinPlayers;
		time_t m_NextAutoDistributionTime;
        int8 last_bgevent_hour;

        BattleGroundSet m_BattleGrounds[MAX_BATTLEGROUND_TYPE_ID];

    private:
        void UpdateLadder();

        BattleMastersMap    mBattleMastersMap;
        std::vector<uint32> m_QueueUpdateScheduler;
        ArenaSpecId m_arena_specs[64];
        /* uint32 - item, uint64 - flag of an allowed spec*/
        std::map<uint32, uint64> m_arena_item_restrictions;
        std::map <uint32/*item_entry*/, entry_class_arr> m_arena_items_exchange;
        /* Plr Guid and item guid are uint64, cause they are only temporary and are used for search*/
        std::map <uint64/*plr guid*/, uint64> m_arena_items_exchange_restricted;
        std::set<arena_items_to_log> m_arena_items_to_log;

        /* Battlegrounds */
        Timer m_NextRatingDiscardUpdate;
        Timer m_AutoDistributionTimeChecker;
        bool   m_Testing;
        bool   m_ApAnnounce;
        BattleGroundTypeId   debugArenaId;

        // LADDER
        Timer m_updateLadderTimer; // we will update ladder once per 10 minutes
        Timer m_updateQueueTimer;
        Ladder_TeamInfo m_ladder[ARENA_TYPE_5v5][LADDER_CNT]; // three ladders support, 2v2, 3v3 and BG

        // Bonus battlegrounds: for still active battleground type
        BattleGroundTypeId m_BGStillActive;
};

#define sBattleGroundMgr (*ACE_Singleton<BattleGroundMgr, ACE_Null_Mutex>::instance())
#define getBattleGroundMgr (BattleGroundMgr::getBattleGroundMgrFromScripts)

#endif
