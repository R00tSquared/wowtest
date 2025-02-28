#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "GameEvent.h"
#include "BattleGroundMgr.h"
#include "ArenaTeam.h"

typedef struct
{
    uint32 ArenaId;
    uint32 InstanceId;
} ArenaIdInstance;

ArenaIdInstance Arenas[15];
const uint32 ArenaIDs[3] =
{
    559, 562, 572
};
bool ThereIsArena;

void DeathSide_Arena_Spectator_Map_InstId_Manipulation(uint32 mapId, uint32 InstId, Map *map, uint8 &GossipNumber, Player * player);
void DeathSide_Arena_Spectator_Gossip_Manipulation(uint32 InstanceId, uint32 MapId, Player *player);
void AddLadderGossipItems(Player* player, Creature* creature, const Ladder_TeamInfo* ladder, uint32 gossipAction, uint32 gossipMenuId, uint32 langString, bool isSolo);

bool AM_Hello(Player* player, Creature* creature)
{
    if (!player || !creature) {
        sLog.outLog(LOG_CRITICAL, "AM_Gossip: player or creature is null");
        return false;
    }

    if (!player->GetSession()) {
        sLog.outLog(LOG_CRITICAL, "AM_Gossip: player->GetSession() is null");
        return false;
    }
    
    if (player->GetLevel() < 70)
	{
		player->SEND_GOSSIP_MENU(990119, creature->GetGUID());
		return true;
	}
	
	bool easy = sWorld.isEasyRealm();
	
	if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (easy && !player->Solo3v3LastTeammates.empty())
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(LANG_RATE_TEAMMATE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 76);
    }

    if (!player->InBattleGroundOrArena())
    {
		if (easy)
		{
			//if(sWorld.getConfig(CONFIG_19_LVL_ADAPTATIONS) && (player->GetLevel() >= TWINK_LEVEL_MIN && player->GetLevel() <= TWINK_LEVEL_MAX))
			//	player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_JOIN_2V2_SKIRMISH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 75);

			if (!player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_3v3)))
				player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SQ_REG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 74, player->GetSession()->GetHellgroundString(LANG_SQ_TEAM_WILL_BE_CREATED), 0, false);
			else
				player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SQ_REG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 74);
		}

        if (sWorld.getConfig(TIME_ARENA_CLOSEDFOR2v2) != 24)
		    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_JOIN_2V2_RATED), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);

        // 2v2 skirmish
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(16747), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 55);

		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_CRE_REG_BLIZZLIKE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_SPECTATING), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    }

	if (easy)
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, player->GetSession()->GetHellgroundString(LANG_HONOR_EXCHANGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 99);
    
	player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 61);

	uint32 t = time(NULL);

	if (!easy)
	{
		uint32 arena_end = getBattleGroundMgr()->m_NextAutoDistributionTime;
		std::string arena_ends_in = arena_end > t ? player->GetSession()->secondsToTimeString(arena_end - t, true, true) : "0";
		char buf[256];
		sprintf(buf, player->GetSession()->GetHellgroundString(12974), arena_ends_in.c_str());
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, buf, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
	}

    uint32 end = sWorld.getConfig(CONFIG_ARENA_SEASON_END); 
	if (end)
	{
		std::string ends_in = end > t ? player->GetSession()->secondsToTimeString(end - t, true, true) : "-";
		char curr[256];
		sprintf(curr, player->GetSession()->GetHellgroundString(15576), ends_in.c_str());
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, curr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 58);
	}

    //uint32 bonus_text = (player->GetSession()->isPremium()) ? 15531 : 15529;
    //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(bonus_text), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 77);

    if (sWorld.getConfig(CONFIG_IS_LOCAL))
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "DEBUG ARENA", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

    player->SEND_GOSSIP_MENU(68, creature->GetGUID());
    return true;
}

bool AM_Gossip(Player *player, Creature *creature, uint32 uiSender, uint32 uiAction)
{
    uint32 PosNumber = 0;
    uint32 PosNumber1 = 0;
    uint32 Team1Member = 0;

    switch (uiAction)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
    {
        if (!sWorld.getConfig(CONFIG_IS_LOCAL))
            return true;

        sBattleGroundMgr.SetDebugArenaId(BATTLEGROUND_NA);
        ChatHandler(player).PSendSysMessage("DEBUG ARENA %s", sBattleGroundMgr.GetDebugArenaId() ? "ENABLED" : "DISABLED");
        AM_Hello(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 2:
    {
        if (player->InBattleGroundOrArena()) {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        
        if (!sWorld.getConfig(CONFIG_ARENA_SPECTATORS_ENABLE))
        {
            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_SPECTATE_OFF), player->GetGUID());
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        // check if should block spectator if 2v2 arena was recently played
        if (!player->isGameMaster())
        {
            time_t t = time(NULL);
            auto& aps = sWorld.m_ArenaPlayersIPs;

            if (!aps.empty())
            {
                for (auto it = aps.begin(); it != aps.end();)
                {
                    if (player->GetSession()->GetRemoteAddress().c_str() == it->first)
                    {
                        if (it->second >= t)
                        {
                            char chr[256];
                            snprintf(chr, sizeof(chr), player->GetSession()->GetHellgroundString(LANG_SPECTATOR_BLOCKED),
                                player->GetSession()->secondsToTimeString(it->second - t).c_str());
                            creature->Whisper(chr, player->GetGUID());

                            player->CLOSE_GOSSIP_MENU();
                            return true;
                        }
                        else
                        {
                            it = aps.erase(it);
                            continue;
                        }
                    }
                    ++it;
                }
            }
        }

        player->PlayerTalkClass->ClearMenus();

        const MapManager::MapMapType& maps = player->GetMapsFromScripts();
        uint8 gossipNumber = 20;
        for (uint8 i = 0; i < 3; i++)
        {
            MapManager::MapMapType::const_iterator iter_last = maps.lower_bound(MapID(ArenaIDs[i] + 1));
            for (MapManager::MapMapType::const_iterator mitr = maps.lower_bound(MapID(ArenaIDs[i])); mitr != iter_last && gossipNumber < 35; ++mitr)
            {
                if (mitr->first.nMapId != ArenaIDs[i])
                {
                    char chrErr[256];
                    sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
                    creature->Whisper(chrErr, player->GetGUID());
                    break;
                }
                DeathSide_Arena_Spectator_Map_InstId_Manipulation(ArenaIDs[i], mitr->first.nInstanceId, mitr->second, gossipNumber, player);
                // gossipNumber increased in function
            }
        }

        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Nagrand Arena] [2 vs 2] with team ratings [1889] and [1898]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Ruins of Lordaeron] [2 vs 2] with team ratings [1516] and [1519]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Ruins of Lordaeron] [2 vs 2] with team ratings [1536] and [1500]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Nagrand Arena] [3 vs 3] with team ratings [1711] and [1778]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Nagrand Arena] [2 vs 2] with team ratings [1672] and [1599]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Blade's Edge Arena] [2 vs 2] with team ratings [1832] and [1808]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Ruins of Lordaeron] [2 vs 2] with team ratings [1785] and [1676]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Ruins of Lordaeron] [2 vs 2] with team ratings [1512] and [1565]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Blade's Edge Arena] [2 vs 2] with team ratings [1743] and [1720]", GOSSIP_SENDER_MAIN, 1);
        //player->ADD_GOSSIP_ITEM(9, "Arena match on [Nagrand Arena] [3 vs 3] with team ratings [1512] and [1632]", GOSSIP_SENDER_MAIN, 1);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_REFRESH_LIST), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 3:
    {
        if (player->InBattleGroundOrArena()) {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        
        player->GetSession()->SendPetitionShowList(creature->GetGUID()); // POKUPKA CHARTERA ARENI ILI REGISTRACIYA
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    // join 2v2
    case GOSSIP_ACTION_INFO_DEF + 13:
    {
        if (player->InBattleGroundOrArena()) {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        //// can't queue wihout group
        //if (!player->GetGroup())
        //{
        //    creature->Whisper(player->GetSession()->GetHellgroundString(LANG_YOU_NOT_IN_GROUP), player->GetGUID());
        //    player->CLOSE_GOSSIP_MENU();
        //    break;
        //}

        WorldPacket Data;
        Data << (uint64)creature->GetGUID() << (uint8)0/*2v2 arena slot*/ << (uint8)1/*asGroup*/ << (uint8)1/*isRated*/;
        player->GetSession()->HandleArenaJoinOpcode(Data);
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 14:
    {
        BattleGroundQueueTypeId bgTypeId1 = player->GetBattleGroundQueueTypeId(0);
        BattleGroundQueueTypeId bgTypeId2 = player->GetBattleGroundQueueTypeId(1);
        BattleGroundQueueTypeId bgTypeId3 = player->GetBattleGroundQueueTypeId(2);
        if (bgTypeId1 == 5 || bgTypeId1 == 6 || bgTypeId1 == 7 || bgTypeId2 == 5 || bgTypeId2 == 6 || bgTypeId2 == 7 || bgTypeId3 == 5 || bgTypeId3 == 6 || bgTypeId3 == 7 ||
            bgTypeId1 == 8 || bgTypeId2 == 8 || bgTypeId3 == 8) // arena
        {
            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_ARENA_QUEUE), player->GetGUID());
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        if (bgTypeId1 && bgTypeId2 && bgTypeId3)
        {
            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_BG_QUEUE), player->GetGUID());
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        
        ChatHandler(player).SendSysMessage(15502);
        player->CLOSE_GOSSIP_MENU();
        AM_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 2);
        break;
    }
    // join 2v2 skirmish
    case GOSSIP_ACTION_INFO_DEF + 55: // skirmish 2v2
    {
        if (player->InBattleGroundOrArena()) {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        
        WorldPacket Data;
        Data << (uint64)creature->GetGUID() << (uint8)0/*2v2 arena slot*/ << (uint8)bool(player->GetGroup()) << (uint8)0/*isRated*/;
        player->GetSession()->HandleArenaJoinOpcode(Data);
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 58: // arena season
    {
        ChatHandler(player).SendSysMessage(15577);
        AM_Hello(player, creature);
        break;
    }
    //case GOSSIP_ACTION_INFO_DEF + 59: // arena items exchange menu
    //{
    //    uint64 specFlag = 0;
    //    getBattleGroundMgr()->ArenaRestrictedGetPlayerSpec(player, specFlag);

    //    uint32 cnt = 0; // cannot exceed 15 due to interface problems
    //    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    //    {
    //        Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
    //        if (pItem)
    //        {
    //            if (getBattleGroundMgr()->IsItemArenaRestricted(pItem->GetEntry(), specFlag))
    //            {
    //                std::string itemNameLocale; // non-initialized. Initialized in "GetItemNameLocale"
    //                const ItemPrototype* pProto = pItem->GetProto();

    //                char chr[256];
    //                sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ITEM_RESTRICTED), player->GetItemNameLocale(pProto, &itemNameLocale));
    //                creature->Whisper(chr, player->GetGUID());

    //                if (player->ArenaRestrictedCanSwap(pItem->GetGUIDLow()))
    //                {
    //                    // 100000 should be enough, only using blizzlike items here
    //                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetItemNameLocale(pProto, &itemNameLocale), GOSSIP_SENDER_MAIN, 2000 + i);
    //                    if (++cnt >= 15)
    //                        break;
    //                }
    //            }
    //        }
    //    }

    //    if (cnt)
    //    {
    //        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 50);
    //        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
    //        player->SEND_GOSSIP_MENU(100000, creature->GetGUID());
    //    }
    //    else
    //    {
    //        creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_RESTRICTION_ALL_SWAPPED), player->GetGUID());
    //        AM_Gossip(player, creature, 0, GOSSIP_ACTION_INFO_DEF + 50);
    //    }
    //    break;
    //}
    case GOSSIP_ACTION_INFO_DEF + 60: // go to main menu
    {
        AM_Hello(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 61: // Arena Ladder. Now support only 1v1 and 2v2
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);

		if (sWorld.isEasyRealm())
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_SOLO_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 62);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
        player->SEND_GOSSIP_MENU(990030, creature->GetGUID()); // INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES ('990030', 'Choose the bracket you want to view.')
        break;
    }
    // 3v3 solo ladder
    case GOSSIP_ACTION_INFO_DEF + 62:
    {
        AddLadderGossipItems(player, creature, getBattleGroundMgr()->getLadder(ARENA_TEAM_3v3),
            GOSSIP_ACTION_INFO_DEF + 62, 990031, LANG_SCRIPT_ARENA_LADDER_1_STATS, true);
        break;
    }

    // 2v2 ladder
    case GOSSIP_ACTION_INFO_DEF + 63:
    {
        AddLadderGossipItems(player, creature, getBattleGroundMgr()->getLadder(ARENA_TEAM_2v2),
            GOSSIP_ACTION_INFO_DEF + 64, 990032, LANG_SCRIPT_ARENA_LADDER_2_TEAM_STATS, false);
        break;
    }
    // 2v2 team members
    case GOSSIP_ACTION_INFO_DEF + 64:
    case GOSSIP_ACTION_INFO_DEF + 65:
    case GOSSIP_ACTION_INFO_DEF + 66:
    case GOSSIP_ACTION_INFO_DEF + 67:
    case GOSSIP_ACTION_INFO_DEF + 68:
    case GOSSIP_ACTION_INFO_DEF + 69:
    case GOSSIP_ACTION_INFO_DEF + 70:
    case GOSSIP_ACTION_INFO_DEF + 71:
    case GOSSIP_ACTION_INFO_DEF + 72:
    case GOSSIP_ACTION_INFO_DEF + 73:
    {
        const Ladder_TeamInfo &thisTeam = (getBattleGroundMgr()->getLadder(ARENA_TEAM_2v2))[uiAction - (GOSSIP_ACTION_INFO_DEF + 64)];

        for (uint32 i = 0; i < LADDER_MAX_MEMBERS_CNT; i++)
        {
            const Ladder_PlayerInfo &member = thisTeam.MembersInfo[i];
            if (!member.Rating) // faster to check by rating than by name?
                break; // no other members as well

            char gossip[256];
            sprintf(gossip, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_2_PLAYER),
                member.PlayerName.c_str(), PlayerRaceAndClassToString(member.Race, member.Class).c_str(), member.Wins, member.Loses, member.Rating);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 64);
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
        player->SEND_GOSSIP_MENU(990033, creature->GetGUID()); // INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES ('990033', 'Position. Name(Race, Class) - Wins / Loses - Personal Rating')

        if (!player || !creature)
        {
            sLog.outLog(LOG_CRITICAL, "AM_Gossip: player or creature is null");
            return false;
        }

        break;
    }
    // join 3v3
    case GOSSIP_ACTION_INFO_DEF + 74:
    {
        if (player->InBattleGroundOrArena()) {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        WorldPacket Data;
        Data << (uint64)creature->GetGUID() << (uint8)ArenaTeam::GetSlotByType(ARENA_TEAM_3v3)/*3v3 slot*/ << (uint8)0/*asGroup*/ << (uint8)1/*isRated*/;
        player->GetSession()->HandleArenaJoinOpcode(Data);

        player->CLOSE_GOSSIP_MENU();
        break;
    }
    // 19 lvl arena
    case GOSSIP_ACTION_INFO_DEF + 75:
    {
        if (player->InBattleGroundOrArena()) {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        
        // should never happen
        //if (player->GetLevel() != 19) {
        if (player->GetLevel() < TWINK_LEVEL_MIN || player->GetLevel() > TWINK_LEVEL_MAX) {
            player->CLOSE_GOSSIP_MENU();
            break;
        }

        uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_2v2);
        bool isgroup = (player->GetGroup()) ? true : false;

        WorldPacket Data;
        Data << (uint64)creature->GetGUID() << (uint8)slot << (uint8)isgroup/*asGroup*/ << (uint8)0/*isRated*/;
        player->GetSession()->HandleArenaJoinOpcode(Data);
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 76:
    {
        if (player->Solo3v3LastTeammates.empty())
        {
            AM_Hello(player, creature);
            break;
        }

        // 2000000000 - LIKE
        // 3000000000 - REPORT
        uint8 i = 0;
        for (auto& teammate : player->Solo3v3LastTeammates)
        {
            if (teammate.first != 0 || player->GetGUID() == teammate.second.guid)
                continue;

            // :)
            if (i == 1)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 76);

            uint32 id;
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, Player::PrintNameAndClassInitials(teammate.second.race, teammate.second.Class, teammate.second.name).c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 76);

            for (uint8 x = 0; x < 2; ++x)
            {
                uint32 act;

                if (x == 0)
                {
                    act = 2000000000;
                    id = LANG_LIKE_TEAMMATE;
                }
                else
                {
                    act = 3000000000;
                    id = LANG_REPORT_TEAMMATE;
                }

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, player->GetSession()->GetHellgroundString(id), GOSSIP_SENDER_MAIN, act + teammate.second.guid);
            }      

            ++i;            
        }


        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 77:
    {
        AM_Hello(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 99:
    {
        player->GetSession()->SendListInventory(creature->GetGUID());
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    default:
    {
        // from GOSSIP_ACTION_INFO_DEF+20 to GOSSIP_ACTION_INFO_DEF+34 are arena spectator gossips
        if (uiAction >= 1020 && ((uiAction - 1020) < 15))
        {
            BattleGroundQueueTypeId bgTypeId1 = player->GetBattleGroundQueueTypeId(0);
            BattleGroundQueueTypeId bgTypeId2 = player->GetBattleGroundQueueTypeId(1);
            BattleGroundQueueTypeId bgTypeId3 = player->GetBattleGroundQueueTypeId(2);
            if (bgTypeId1 == 5 || bgTypeId1 == 6 || bgTypeId1 == 7 || bgTypeId2 == 5 || bgTypeId2 == 6 || bgTypeId2 == 7 || bgTypeId3 == 5 || bgTypeId3 == 6 || bgTypeId3 == 7 ||
                bgTypeId1 == 8 || bgTypeId2 == 8 || bgTypeId3 == 8) // arena
            {
                creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_ARENA_QUEUE), player->GetGUID());
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
            if (bgTypeId1 && bgTypeId2 && bgTypeId3)
            {
                creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_BG_QUEUE), player->GetGUID());
                player->CLOSE_GOSSIP_MENU();
                return true;
            }

            if (player->InBattleGroundOrArena())
            {
                player->CLOSE_GOSSIP_MENU();
                return true;
            }

            DeathSide_Arena_Spectator_Gossip_Manipulation(Arenas[uiAction - 1020].InstanceId, Arenas[uiAction - 1020].ArenaId, player);

            player->CLOSE_GOSSIP_MENU();
            AM_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 2);
        }
        // solo 3v3 like/report
        else if (uiAction >= 2000000000 && uiAction < 4000000000)
        {
            if (player->Solo3v3LastTeammates.empty())
            {
                player->CLOSE_GOSSIP_MENU();
                break;
            }

            uint8 action; // 1 - like, 2 - report
            uint64 tmguid;
            uint64 myguid = player->GetGUID();

            uint8 busy_count = 0;
            for (auto &teammate : player->Solo3v3LastTeammates)
            {                
                if (teammate.first != 0)
                {
                    ++busy_count;
                    continue;
                }

                // don't allow to rate teeammates when they are in arena because rating bugs
                //Player* tplr = Unit::GetPlayerInWorld(teammate.second.guid);
                //if (tplr)
                //{
                //    BattleGround* bg = tplr->GetBattleGround();
                //    if (bg && (bg->GetStatus() == STATUS_WAIT_QUEUE || bg->GetStatus() == STATUS_WAIT_JOIN || bg->GetStatus() == STATUS_IN_PROGRESS))
                //    {
                //        ChatHandler(player).PSendSysMessage(15582);
                //        AM_Hello(player, creature);
                //        return true;
                //    }
                //}
        
                if (teammate.second.guid == uiAction - 2000000000)
                {
                    action = 1;
                    tmguid = uiAction - 2000000000;
                    teammate.first = action;
                    ++busy_count;
                }
                else if (teammate.second.guid == uiAction - 3000000000)
                {
                    action = 2;
                    tmguid = uiAction - 3000000000;
                    teammate.first = action;
                    ++busy_count;
                }
            }
            
            ASSERT(action && tmguid);

            // clear container if 2 teammates was liked/reported
            if (busy_count == 2)
                player->Solo3v3LastTeammates.clear();

            getObjectMgr()->ModifyArenaRating(tmguid, action == 1 ? 1 : -1, ARENA_TEAM_3v3);

            ChatHandler(player).PSendSysMessage(LANG_RATE_SUCCESS);

            std::string tmname;
            Player* tplayer = Unit::GetPlayerInWorld(tmguid);
            if (tplayer)
            {
                if (action == 1)
                {
                    ChatHandler(tplayer).PSendSysMessage(LANG_LIKED);
                    tplayer->SendSpellVisual(6375);
                }
                else if (action == 2)
                {
                    ChatHandler(tplayer).PSendSysMessage(LANG_REPORTED);
                    tplayer->SendSpellVisual(495);
                }

                tmname = tplayer->GetName();
            }

            // even if offline
            if (action == 2)
            {
                //add reports if reported
                //if noob was reported once or more
                auto range = sWorld.m_arenareports.equal_range(tmguid);
                for (auto it = range.first; it != range.second; ++it)
                    // already reported by me
                    if (it->second == player->GetGUID())
                    {
                        AM_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 76);
                        return true;
                    }
                
                if (tmname.empty())
                    sObjectMgr.GetPlayerNameByGUID(tmguid, tmname);

                sLog.outLog(LOG_ARENA, "ArenaReport: Player %s (%u) was reported by %s (%u)", tmname.c_str(), (uint32)tmguid, player->GetName(), player->GetGUIDLow());
                sWorld.m_arenareports.insert(std::make_pair(tmguid, myguid));
            }

            AM_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 76);
            return true;
            
            //BattleGround* bg = NULL;
            //bg = player->GetBattleGround();
            //if (!player->InArena() || !bg || !bg->isArena() || bg->GetArenaType() != ARENA_TYPE_3v3)
            //{
            //    player->CLOSE_GOSSIP_MENU();
            //    return true;
            //}

            //uint32 myguid = player->GetGUID();

            //// is noob exists in my team?
            //// it's working because we using two temp 3v3 team
            //uint32 myteam = bg->GetArenaTeamIdForTeam(bg->GetPlayerTeam(player->GetGUID()));
            //ArenaTeam* at = getObjectMgr()->GetArenaTeamById(myteam); // 1 == 3v3

            //uint32 noobguid;
            //bool found = false;
            //for (ArenaTeam::MemberList::iterator itr = at->membersBegin(); itr != at->membersEnd(); ++itr)
            //{
            //    noobguid = itr->guid;
            //    if (noobguid == uiAction - 10000)
            //    {
            //        found = true;
            //        break;
            //    }
            //}

            //if (!found)
            //{
            //    creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), myguid);
            //    player->CLOSE_GOSSIP_MENU();
            //    return true;
            //}

            //// reporting myself?
            //if (myguid == noobguid)
            //{
            //    creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), myguid);
            //    player->CLOSE_GOSSIP_MENU();
            //    return true;
            //}

            //// if noob was reported once or more
            //auto range = sWorld.m_arenareports.equal_range(noobguid);
            //if (range.first != range.second)
            //    for (auto it = range.first; it != range.second; ++it)
            //        // already reported by me
            //        if (it->second == myguid)
            //        {
            //            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SQ_ALREADY_REPORTED), myguid);
            //            player->CLOSE_GOSSIP_MENU();
            //            return true;
            //        }

            //// add report
            //sWorld.m_arenareports.insert(std::make_pair(noobguid, myguid));

            //creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SQ_REPORTED), myguid);

            ////send msg to noob
            //if (Player* noob = Unit::GetPlayerInWorld(noobguid))
            //    ChatHandler(noob).PSendSysMessage(LANG_SQ_YOU_WAS_REPORTED, player->GetName());
        }
        else
        {
            char chrErr[256];
            sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), uiAction);
            creature->Whisper(chrErr, player->GetGUID());
        }
        break;
    }
    }
    return true;
}

void DeathSide_Arena_Spectator_Map_InstId_Manipulation(uint32 mapId, uint32 InstId, Map *map, uint8 &GossipNumber, Player * player)
{
    Map::PlayerList const &PlayerList = map->GetPlayers();
    if (!PlayerList.isEmpty())
    {
        Player* itrplayer = NULL;
        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            if (!(itr->getSource()->IsInWorld()))
                continue;

            if (itr->getSource()->IsSpectator())
                continue;

            if (itrplayer = itr->getSource())
                break;
        }
        BattleGround * bg = NULL;
        if (itrplayer)
            bg = itrplayer->GetBattleGround();

        // && bg->GetStatus() == STATUS_IN_PROGRESS
		// non-rated need to be displayed for testing purposes
        if (bg && (bg->GetStatus() == STATUS_IN_PROGRESS || bg->GetStatus() == STATUS_WAIT_JOIN))
        {
            char gossip[256];
            uint8 ArenaId;
            switch (mapId)
            {
            case 572: ArenaId = 0; break;
            case 559: ArenaId = 1; break;
            default: ArenaId = 2; break;
            }

            uint32 gossip_id = (bg->GetStatus() == STATUS_IN_PROGRESS) ? GOSSIP_ACTION_INFO_DEF + GossipNumber : GOSSIP_ACTION_INFO_DEF + 14;

            sprintf(gossip, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_FIGHT_VERSUS),
                player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_START_LORD + ArenaId),
                bg->GetArenaType(), bg->GetArenaType(), 0, 0);
            player->ADD_GOSSIP_ITEM(9, gossip, GOSSIP_SENDER_MAIN, gossip_id);
            //ThereIsArena = true;
            Arenas[GossipNumber - 20].ArenaId = mapId;
            Arenas[GossipNumber - 20].InstanceId = InstId;
            GossipNumber++;
			//if (!bg->isRated())
			//{

			//}
			//else
			//{
			//	uint32 aTeamId = bg->GetArenaTeamIdForTeam(ALLIANCE);
			//	uint32 hTeamId = bg->GetArenaTeamIdForTeam(HORDE);
			//	if (aTeamId && hTeamId)
			//	{
			//		ArenaTeam* aTeam = getObjectMgr()->GetArenaTeamById(aTeamId);
			//		ArenaTeam* hTeam = getObjectMgr()->GetArenaTeamById(hTeamId);

			//		if (aTeam && hTeam)
			//		{
			//			uint32 gossip_id = (bg->GetStatus() == STATUS_IN_PROGRESS) ? GOSSIP_ACTION_INFO_DEF + GossipNumber : GOSSIP_ACTION_INFO_DEF + 14;

			//			sprintf(gossip, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_FIGHT_VERSUS),
			//				player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_START_LORD + ArenaId),
			//				bg->GetArenaType(), bg->GetArenaType(), aTeam->GetRating(), hTeam->GetRating());
			//			player->ADD_GOSSIP_ITEM(9, gossip, GOSSIP_SENDER_MAIN, gossip_id);
			//			//ThereIsArena = true;
			//			Arenas[GossipNumber - 20].ArenaId = mapId;
			//			Arenas[GossipNumber - 20].InstanceId = InstId;
			//			GossipNumber++;
			//		}
			//	}
			//}
        }
    }
}

void AddLadderGossipItems(Player* player, Creature* creature, const Ladder_TeamInfo* ladder, uint32 gossipAction, uint32 gossipMenuId, uint32 langString, bool isSolo)
{
    if (!ladder) {
        sLog.outLog(LOG_CRITICAL, "AM_Gossip: ladder is null for gossipAction %u", gossipAction);
        return;
    }

    for (uint32 i = 0; i < LADDER_CNT; i++)
    {
        if (!ladder[i].Id)
            break;

        if (isSolo) { // 3v3 solo ladder
            if (ladder[i].MembersInfo[0].PlayerName.empty())
                continue;

            const Ladder_PlayerInfo& member = ladder[i].MembersInfo[0];

            char gossip[256];
            snprintf(gossip, sizeof(gossip), player->GetSession()->GetHellgroundString(langString),
                i + 1, member.PlayerName.c_str(),
                PlayerRaceAndClassToString(member.Race, member.Class).c_str(),
                member.Wins, member.Loses, member.Rating);

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossip, GOSSIP_SENDER_MAIN, gossipAction);
        }
        else { // 2v2 team ladder
            char gossip[256];
            snprintf(gossip, sizeof(gossip), player->GetSession()->GetHellgroundString(langString),
                i + 1, ladder[i].TeamName.c_str(),
                ladder[i].Wins, ladder[i].Loses, ladder[i].Rating);

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossip, GOSSIP_SENDER_MAIN, gossipAction + i);
        }
    }

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 61);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
    player->SEND_GOSSIP_MENU(gossipMenuId, creature->GetGUID());
}

void DeathSide_Arena_Spectator_Gossip_Manipulation(uint32 InstanceId, uint32 MapId, Player *player)
{
    player->SaveRecallPosition();
    Player *pl = NULL;
    Map *map = player->FindMapFromScripts(MapId, InstanceId);
    if (!map)
        return;

    BattleGround * bg = NULL;

    Map::PlayerList const &PlayerListForGossip = map->GetPlayers();
    if (!PlayerListForGossip.isEmpty())
    {
        for (const auto& playergs : PlayerListForGossip)
        {
            if (Player* p = playergs.getSource())
            {
                if (p && !bg)
                {
                    bg = p->GetBattleGround();
                    if (bg->GetStatus() != STATUS_IN_PROGRESS)
                    {
                        ChatHandler(player).PSendSysMessage(15502);
                        return;
                    }

                    if (p->IsHidden() && bg->GetArenaType() == ARENA_TYPE_2v2)
                    {
                        ChatHandler(player).PSendSysMessage(15558);
                        return;
                    }
                }
            }
        }
        
        pl = PlayerListForGossip.begin()->getSource();
    }

    if (pl && pl->InArena())
    {
        //player->RemoveAllAurasOnDeath();
        Pet* pet = player->GetPet();
        if (pet)
            player->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT);
        player->SetTemporaryUnsummonedPetNumber(0);
        player->SetBattleGroundId(pl->GetBattleGroundId(), pl->GetBattleGround()->GetTypeID());
        player->SaveOwnBattleGroundEntryPoint();
        player->TeleportTo(pl->GetMapId(), pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetOrientation());
        player->SetSpectator(true);
    }
}

void AddSC_AM()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_arena_multi";
    newscript->pGossipHello = &AM_Hello;
    newscript->pGossipSelect = &AM_Gossip;
    newscript->RegisterSelf();
}
