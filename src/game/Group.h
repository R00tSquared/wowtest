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

#ifndef HELLGROUND_GROUP_H
#define HELLGROUND_GROUP_H

#include "GroupReference.h"
#include "GroupRefManager.h"
#include "LootMgr.h"
#include "BattleGround.h"

#include <map>
#include <vector>

#define MAXGROUPSIZE 5
#define MAXRAIDSIZE 40
#define MAX_RAID_SUBGROUPS MAXRAIDSIZE/MAXGROUPSIZE
#define TARGETICONCOUNT 8

enum RollVote
{
    PASS              = 0,
    NEED              = 1,
    GREED             = 2,
    NOT_EMITED_YET    = 3,
    NOT_VALID         = 4
};

enum GroupMemberFlags
{
    MEMBER_FLAG_NONE        = 0x00,
    MEMBER_FLAG_ASSISTANT   = 0x01,
    MEMBER_FLAG_MAINTANK    = 0x02,
    MEMBER_FLAG_MAINASSIST  = 0x04
};

enum GroupMemberOnlineStatus
{
    MEMBER_STATUS_OFFLINE   = 0x0000,
    MEMBER_STATUS_ONLINE    = 0x0001,
    MEMBER_STATUS_PVP       = 0x0002,
    MEMBER_STATUS_UNK0      = 0x0004,                       // dead? (health=0)
    MEMBER_STATUS_UNK1      = 0x0008,                       // ghost? (health=1)
    MEMBER_STATUS_UNK2      = 0x0010,                       // never seen
    MEMBER_STATUS_UNK3      = 0x0020,                       // never seen
    MEMBER_STATUS_UNK4      = 0x0040,                       // appears with dead and ghost flags
    MEMBER_STATUS_UNK5      = 0x0080,                       // never seen
    MEMBER_STATUS_RAF       = 0x0100,                       // RAF status in party/raid
};

enum GroupType
{
    GROUPTYPE_NORMAL = 0,
    GROUPTYPE_RAID   = 1
};

class BattleGround;

enum GroupUpdateFlags
{
    GROUP_UPDATE_FLAG_NONE              = 0x00000000,       // nothing
    GROUP_UPDATE_FLAG_STATUS            = 0x00000001,       // uint16, flags
    GROUP_UPDATE_FLAG_CUR_HP            = 0x00000002,       // uint16
    GROUP_UPDATE_FLAG_MAX_HP            = 0x00000004,       // uint16
    GROUP_UPDATE_FLAG_POWER_TYPE        = 0x00000008,       // uint8
    GROUP_UPDATE_FLAG_CUR_POWER         = 0x00000010,       // uint16
    GROUP_UPDATE_FLAG_MAX_POWER         = 0x00000020,       // uint16
    GROUP_UPDATE_FLAG_LEVEL             = 0x00000040,       // uint16
    GROUP_UPDATE_FLAG_ZONE              = 0x00000080,       // uint16
    GROUP_UPDATE_FLAG_POSITION          = 0x00000100,       // uint16, uint16
    GROUP_UPDATE_FLAG_AURAS             = 0x00000200,       // uint64 mask, for each bit set uint16 spellid + uint8 unk
    GROUP_UPDATE_FLAG_PET_GUID          = 0x00000400,       // uint64 pet guid
    GROUP_UPDATE_FLAG_PET_NAME          = 0x00000800,       // pet name, NULL terminated string
    GROUP_UPDATE_FLAG_PET_MODEL_ID      = 0x00001000,       // uint16, model id
    GROUP_UPDATE_FLAG_PET_CUR_HP        = 0x00002000,       // uint16 pet cur health
    GROUP_UPDATE_FLAG_PET_MAX_HP        = 0x00004000,       // uint16 pet max health
    GROUP_UPDATE_FLAG_PET_POWER_TYPE    = 0x00008000,       // uint8 pet power type
    GROUP_UPDATE_FLAG_PET_CUR_POWER     = 0x00010000,       // uint16 pet cur power
    GROUP_UPDATE_FLAG_PET_MAX_POWER     = 0x00020000,       // uint16 pet max power
    GROUP_UPDATE_FLAG_PET_AURAS         = 0x00040000,       // uint64 mask, for each bit set uint16 spellid + uint8 unk, pet auras...
    GROUP_UPDATE_PET                    = 0x0007FC00,       // all pet flags
    GROUP_UPDATE_FULL                   = 0x0007FFFF,       // all known flags
};

#define GROUP_UPDATE_FLAGS_COUNT          20
                                                                // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19
static const uint8 GroupUpdateLength[GROUP_UPDATE_FLAGS_COUNT] = { 0, 2, 2, 2, 1, 2, 2, 2, 2, 4, 8, 8, 1, 2, 2, 2, 1, 2, 2, 8};

class InstanceSave;

class Roll : public LootValidatorRef
{
    public:
        Roll(uint64 _guid, LootItem const& li)
            : itemGUID(_guid), itemid(li.itemid), itemRandomPropId(li.randomPropertyId), itemRandomSuffix(li.randomSuffix),
            totalPlayersRolling(0), totalNeed(0), totalGreed(0), totalPass(0), itemSlot(0) {}
        ~Roll() { }
        void setLoot(Loot *pLoot) { link(pLoot, this); }
        Loot *getLoot() { return getTarget(); }
        void targetObjectBuildLink();

        void SendLootStartRoll(uint32 CountDown);
        void SendLootRoll(const uint64& SourceGuid, const uint64& TargetGuid, uint8 RollNumber, uint8 RollType);
        void SendLootRollWon(const uint64& SourceGuid, const uint64& TargetGuid, uint8 RollNumber, uint8 RollType);
        void SendLootAllPassed();
        void CountTheRoll();
        bool CountRollVote(const uint64& playerGUID, uint8 Choice);

        uint64 itemGUID;
        uint32 itemid;
        int32  itemRandomPropId;
        uint32 itemRandomSuffix;
        typedef std::map<uint64, RollVote> PlayerVote;
        PlayerVote playerVote;                              //vote position correspond with player position (in group)
        uint8 totalPlayersRolling;
        uint8 totalNeed;
        uint8 totalGreed;
        uint8 totalPass;
        uint8 itemSlot;
        uint32 rollTimer;
};

struct InstanceGroupBind
{
    InstanceSave *save;
    bool perm;
    /* permanent InstanceGroupBinds exist iff the leader has a permanent
       PlayerInstanceBind for the same instance. */
    InstanceGroupBind() : save(NULL), perm(false) {}
};

struct GroupMemberSlot
{
    uint64      guid;
    std::string name;
    uint8       group;
    bool        assistant;
};

/** request member stats checken **/
/** todo: uninvite people that not accepted invite **/
class HELLGROUND_IMPORT_EXPORT Group
{
    public:

        typedef std::list<GroupMemberSlot> MemberSlotList;
        typedef MemberSlotList::const_iterator member_citerator;

        typedef UNORDERED_MAP< uint32 /*mapId*/, InstanceGroupBind> BoundInstancesMap;
    protected:
        typedef MemberSlotList::iterator member_witerator;
        typedef std::set<Player*> InvitesList;
    public:
        Group();
        ~Group();

        // group manipulation methods
        bool   Create(const uint64 &guid, const char * name, bool lfg = false);
        bool   LoadGroupFromDB(const uint64 &leaderGuid, QueryResultAutoPtr result = QueryResultAutoPtr(NULL), bool loadMembers = true);
        bool   LoadMemberFromDB(uint32 guidLow, uint8 subgroup, bool assistant);
        bool   AddInvite(Player *player);
        uint32 RemoveInvite(Player *player);
        void   RemoveAllInvites();
        bool   AddLeaderInvite(Player *player);
        bool   AddMember(const uint64 &guid, const char* name, bool lfg = false);
                                                            // method: 0=just remove, 1=kick
        uint32 RemoveMember(const uint64 &guid, const uint8 &method);
        void   ChangeLeader(const uint64 &guid);
        void   CheckLeader(const uint64 &guid, bool isLogout);
        bool   ChangeLeaderToFirstOnlineMember();
        void   SetLootMethod(LootMethod method) { m_lootMethod = method; }
        void   SetLooterGuid(const uint64 &guid) { m_looterGuid = guid; }
        void   UpdateLooterGuid(WorldObject* object, bool ifneed = false);
        void   SetLootThreshold(ItemQualities threshold) { m_lootThreshold = threshold; }
        void   Disband(bool hideDestroy=false);

        // properties accessories
        bool IsFull() const { return isRaidGroup() ? (m_memberSlots.size() >= MAXRAIDSIZE) : (m_memberSlots.size() >= MAXGROUPSIZE); }
        bool isRaidGroup() const { return m_groupType==GROUPTYPE_RAID; }
        bool isBGGroup()   const { return m_bgGroup != NULL; }
        bool IsCreated()   const { return GetMembersCount() > 0; }
        const uint64& GetLeaderGUID() const { return m_leaderGuid; }
        const char * GetLeaderName() const { return m_leaderName.c_str(); }
        LootMethod    GetLootMethod() const { return m_lootMethod; }
        const uint64& GetLooterGuid() const { return m_looterGuid; }
        ItemQualities GetLootThreshold() const { return m_lootThreshold; }

        // member manipulation methods
        bool IsMember(const uint64& guid) const { return _getMemberCSlot(guid) != m_memberSlots.end(); }
        bool IsLeader(const uint64& guid) const { return (GetLeaderGUID() == guid); }
        uint64 GetMemberGUID(const std::string& name)
        {
            for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if (itr->name == name)
                {
                    return itr->guid;
                }
            }
            return 0;
        }
        bool IsAssistant(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if (mslot==m_memberSlots.end())
                return false;

            return mslot->assistant;
        }
        Player* GetInvited(const uint64& guid) const;
        Player* GetInvited(const std::string& name) const;

        bool SameSubGroup(uint64 guid1,const uint64& guid2) const
        {
            member_citerator mslot2 = _getMemberCSlot(guid2);
            if (mslot2==m_memberSlots.end())
                return false;

            return SameSubGroup(guid1,&*mslot2);
        }

        bool SameSubGroup(uint64 guid1, GroupMemberSlot const* slot2) const
        {
            member_citerator mslot1 = _getMemberCSlot(guid1);
            if (mslot1==m_memberSlots.end() || !slot2)
                return false;

            return (mslot1->group==slot2->group);
        }

        bool HasFreeSlotSubGroup(uint8 subgroup) const
        {
            return (m_subGroupsCounts && m_subGroupsCounts[subgroup] < MAXGROUPSIZE);
        }

        bool SameSubGroup(Player const* member1, Player const* member2) const;

        MemberSlotList const& GetMemberSlots() const { return m_memberSlots; }
        GroupReference* GetFirstMember() { return m_memberMgr.getFirst(); }
        uint32 GetMembersCount() const { return m_memberSlots.size(); }
        void GetDataForXPAtKill(Unit const* victim, uint32& count,uint32& sum_level, Player* & member_with_max_level, Player* & not_gray_member_with_max_level, Player* & not_gray_with_def_rates);
        uint8 GetMemberFlags(member_citerator &citr) const;
        
        const char* GetMemberName(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if (mslot != m_memberSlots.end())
                return mslot->name.c_str();

            return "NULL";
        }
        
        uint8 GetMemberGroup(uint64 guid) const
        {
            member_citerator mslot = _getMemberCSlot(guid);
            if (mslot==m_memberSlots.end())
                return (MAXRAIDSIZE/MAXGROUPSIZE+1);

            return mslot->group;
        }

        // some additional raid methods
        void ConvertToRaid();

        void SetBattlegroundGroup(BattleGround *bg);
        bool CanJoinBattleGroundQueue(BattleGroundTypeId bgTypeId, BattleGroundQueueTypeId bgQueueTypeId, uint32 MinPlayerCount, uint32 MaxPlayerCount, bool isRated, uint32 arenaSlot, WorldSession* leader);

        void ChangeMembersGroup(Player* leader, const uint64 &guid, const uint8 &group);
        void ChangeMembersGroup(Player* leader, Player *player, const uint8 &group);

        void SetAssistant(uint64 guid, const bool &state)
        {
            if (!isRaidGroup())
                return;
            if (_setAssistantFlag(guid, state))
                SendUpdate();
        }
        void SetMainTank(uint64 guid)
        {
            if (!isRaidGroup())
                return;

            if (_setMainTank(guid))
                SendUpdate();
        }
        void SetMainAssistant(uint64 guid)
        {
            if (!isRaidGroup())
                return;

            if (_setMainAssistant(guid))
                SendUpdate();
        }

        void SetTargetIcon(uint8 id, uint64 guid);
        void SetDifficulty(uint8 difficulty);
        uint8 GetDifficulty() { return m_difficulty; }
        uint16 InInstance();
        void ResetInstances(uint8 method, Player* SendMsgTo);

        // -no description-
        //void SendInit(WorldSession *session);
        void SendTargetIconList(WorldSession *session);
        void SendUpdate();
        void SendPlayerUpdate(Player* player);
        void Update(uint32 diff);
        void UpdatePlayerOutOfRange(Player* pPlayer);
                                                            // ignore: GUID of player that will be ignored
        void BroadcastPacket(WorldPacket *packet, bool ignorePlayersInBGRaid, int group=-1, uint64 ignore=0);
        void BroadcastReadyCheck(WorldPacket *packet);
        void OfflineReadyCheck();

        /*********************************************************/
        /***                   LOOT SYSTEM                     ***/
        /*********************************************************/

        void PrepareLootRolls(const uint64& playerGUID, Loot *loot, WorldObject* object);
        void SendMasterLoot(Loot *loot, WorldObject* object);
        void SendRoundRobin(Loot *loot, WorldObject* object);
        bool IsRoundRobinLootType() { return m_lootMethod == GROUP_LOOT || m_lootMethod == NEED_BEFORE_GREED || m_lootMethod == ROUND_ROBIN; }
        bool IsRollLootType() { return m_lootMethod == GROUP_LOOT || m_lootMethod == NEED_BEFORE_GREED; }

        void LinkMember(GroupReference *pRef) { m_memberMgr.insertFirst(pRef); }
        void DelinkMember(GroupReference* /*pRef*/) { }

        InstanceGroupBind* BindToInstance(InstanceSave *save, bool permanent, bool load = false);
        void UnbindInstance(uint32 mapid, uint8 difficulty, bool unload = false);
        InstanceGroupBind* GetBoundInstance(uint32 mapid, uint8 difficulty);
        BoundInstancesMap& GetBoundInstances(uint8 difficulty) { return m_boundInstances[difficulty]; }

        // FG: evil hacks
        void BroadcastGroupUpdate(void);

        //void SetRaidRules(Player* leader, std::string rule, bool second);
        //void SendRulesTo(Player* plr);
        //bool SetBoundRules(uint32 instId); // returns false if rules are already bound
        //bool RaidRulesExist() { return m_raidRules[0] != ""; };
        
        uint32 GetTempGroupGUID() { return m_tempGroupGuid; };
        //void Group::RemoveHashIPFromAll();

    protected:
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant=false);
        bool _addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group);
        bool _removeMember(const uint64 &guid);             // returns true if leader has changed
        void _setLeader(const uint64 &guid);

        bool _setMembersGroup(const uint64 &guid, const uint8 &group);
        bool _setAssistantFlag(const uint64 &guid, const bool &state);
        bool _setMainTank(const uint64 &guid);
        bool _setMainAssistant(const uint64 &guid);

        void _homebindIfInstance(Player *player);

        void _initRaidSubGroupsCounter()
        {
            // Sub group counters initialization
            if (!m_subGroupsCounts)
                m_subGroupsCounts = new uint8[MAX_RAID_SUBGROUPS];

            memset((void*)m_subGroupsCounts, 0, (MAX_RAID_SUBGROUPS)*sizeof(uint8));

            for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
                ++m_subGroupsCounts[itr->group];
        }

        member_citerator _getMemberCSlot(uint64 Guid) const
        {
            for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_memberSlots.end();
        }

        member_witerator _getMemberWSlot(uint64 Guid)
        {
            for (member_witerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
            {
                if (itr->guid == Guid)
                    return itr;
            }
            return m_memberSlots.end();
        }

        void SubGroupCounterIncrease(uint8 subgroup)
        {
            if (m_subGroupsCounts)
                ++m_subGroupsCounts[subgroup];
        }

        void SubGroupCounterDecrease(uint8 subgroup)
        {
            if (m_subGroupsCounts)
                --m_subGroupsCounts[subgroup];
        }

        MemberSlotList      m_memberSlots;
        GroupRefManager     m_memberMgr;
        InvitesList         m_invitees;
        uint64              m_leaderGuid;
        std::string         m_leaderName;
        uint64              m_mainTank;
        uint64              m_mainAssistant;
        GroupType           m_groupType;
        uint8               m_difficulty;
        BattleGround*       m_bgGroup;
        uint64              m_targetIcons[TARGETICONCOUNT];
        LootMethod          m_lootMethod;
        ItemQualities       m_lootThreshold;
        uint64              m_looterGuid;
        BoundInstancesMap   m_boundInstances[TOTAL_DIFFICULTIES];
        uint8*              m_subGroupsCounts;
        time_t              m_leaderLogoutTime;
        std::string         m_raidRules[2]; // 2 messages for rules
        std::set<uint32>    m_raidRulesBound;
        uint32              m_tempGroupGuid;        
};
#endif

