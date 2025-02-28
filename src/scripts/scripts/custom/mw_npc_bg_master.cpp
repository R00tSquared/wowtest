#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "GameEvent.h"
#include "BattleGroundMgr.h"
#include "ArenaTeam.h"

void getMarksAndRatioByAction(uint32 uiAction, uint32 &OldMark, uint32 &NewMark, uint32 &OldCount, uint32 &NewCount)
{
    switch (uiAction)
    {
    case GOSSIP_ACTION_INFO_DEF + 100 + 1:
    case GOSSIP_ACTION_INFO_DEF + 100 + 2:
    case GOSSIP_ACTION_INFO_DEF + 100 + 3:
        OldMark = 20558; // warsong
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 4:
    case GOSSIP_ACTION_INFO_DEF + 100 + 5:
    case GOSSIP_ACTION_INFO_DEF + 100 + 6:
        OldMark = 20559; // Arathi
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 7:
    case GOSSIP_ACTION_INFO_DEF + 100 + 8:
    case GOSSIP_ACTION_INFO_DEF + 100 + 9:
        OldMark = 29024; // Eye of the storm
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 10:
    case GOSSIP_ACTION_INFO_DEF + 100 + 11:
    case GOSSIP_ACTION_INFO_DEF + 100 + 12:
        OldMark = 20560; // Alterac valley
        break;
    }
    switch (uiAction)
    {
    case GOSSIP_ACTION_INFO_DEF + 100 + 4:
    case GOSSIP_ACTION_INFO_DEF + 100 + 7:
    case GOSSIP_ACTION_INFO_DEF + 100 + 10:
        NewMark = 20558; // warsong
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 1:
    case GOSSIP_ACTION_INFO_DEF + 100 + 8:
    case GOSSIP_ACTION_INFO_DEF + 100 + 11:
        NewMark = 20559; // Arathi
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 2:
    case GOSSIP_ACTION_INFO_DEF + 100 + 5:
    case GOSSIP_ACTION_INFO_DEF + 100 + 12:
        NewMark = 29024; // Eye of the storm
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 3:
    case GOSSIP_ACTION_INFO_DEF + 100 + 6:
    case GOSSIP_ACTION_INFO_DEF + 100 + 9:
        NewMark = 20560; // Alterac valley
        break;
    }
    switch (uiAction)
    {
    case GOSSIP_ACTION_INFO_DEF + 100 + 1:
        OldCount = 8;
        NewCount = 3;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 2:
        OldCount = 8;
        NewCount = 3;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 3:
        OldCount =  4;
        NewCount = 1;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 4:
        OldCount = 3;
        NewCount = 2;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 5:
        OldCount = 2;
        NewCount = 1;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 6:
        OldCount = 3;
        NewCount = 1;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 7:
        OldCount = 3;
        NewCount = 2;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 8:
        OldCount = 2;
        NewCount = 1;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 9:
        OldCount = 3;
        NewCount = 1;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 10:
        OldCount = 1;
        NewCount = 1;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 11:
        OldCount = 4;
        NewCount = 3;
        break;
    case GOSSIP_ACTION_INFO_DEF + 100 + 12:
        OldCount = 4;
        NewCount = 3;
        break;
    }
}

void RegBG(Player* player, Creature* creature)
{	
	//if (!sWorld.getConfig(CONFIG_BG_EVENTS_ENABLED))
	//{
	//	player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
	//	player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
	//	player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
	//	player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
	//	player->SEND_GOSSIP_MENU(68, creature->GetGUID());
	//}
	//else
	//{

	//}

	char chr[256];
	bool ending = false;
	uint8 active = 0;

	if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_WS))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15641), player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
		++active;
	}
	else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_WS))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15642), player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
		ending = true;
	}

	if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_AB))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15641), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
		++active;
	}
	else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_AB))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15642), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
		ending = true;
	}

	if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_EY))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15641), player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
		++active;
	}
	else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_EY))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15642), player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
		ending = true;
	}

	if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_AV))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15641), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
		++active;
	}
	else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_AV))
	{
		sprintf(chr, player->GetSession()->GetHellgroundString(15642), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY));
		player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
		ending = true;
	}

	if (ending || active > 1)
	{
		player->SEND_GOSSIP_MENU(68, creature->GetGUID());
		return;
	}
	else // only 1 variant - use it instantly
	{
		player->PlayerTalkClass->ClearMenus();
		player->CLOSE_GOSSIP_MENU();

		for (uint32 i = BATTLEGROUND_AV; i <= BATTLEGROUND_EY; ++i)
		{
			// skip arenas
			if (i > BATTLEGROUND_AB && i < BATTLEGROUND_EY)
				continue;

			if (getBattleGroundMgr()->IsBGEventActive(BattleGroundTypeId(i)))
			{
				if (!creature->sendBgNotAvailableByLevel(player, BattleGroundTypeId(i)))
					player->GetSession()->SendBattlegGroundList(ObjectGuid(creature->GetGUID()), BattleGroundTypeId(i));
				break;
			}
		}
	}

	return;
}

bool BM_Hello(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (!player->InBattleGroundOrArena())
    {
		// CreateRatedBGTeam
		//if (sWorld.isEasyRealm() && player->GetLevel() == 70 && !player->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_5v5)))
		//	player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_REGISTRATION_BG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1, player->GetSession()->GetHellgroundString(15653), 0, false);
		//else
		
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(LANG_SCRIPT_REGISTRATION_BG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

	// ladder
	if (sWorld.getConfig(CONFIG_RATED_BG_ENABLED))
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, player->GetSession()->GetHellgroundString(15654), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 62);

    //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(LANG_BG_STATS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 76);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, player->GetSession()->GetHellgroundString(LANG_MARK_EXCHANGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);

	if (sWorld.isEasyRealm())
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, player->GetSession()->GetHellgroundString(LANG_HONOR_EXCHANGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 99);

    //uint32 bonus_text = (player->GetSession()->isPremium()) ? 15532 : 15530;
    //player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(bonus_text), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);

	if (player->isGameMaster())
	{
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "<GM TEST>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 98);
	}

	if (creature->GetEntry() == 695005) // Tiny Butch
	{
		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(16194), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 97);
	}

    if (sWorld.getConfig(CONFIG_IS_LOCAL))
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "DEBUG BG", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    }

    player->SEND_GOSSIP_MENU(68, creature->GetGUID());
    return true;
}

bool BM_Gossip(Player *player, Creature *creature, uint32 uiSender, uint32 uiAction)
{
    // allow whitelisted
    if (player->InBattleGroundOrArena() && uiAction <= GOSSIP_ACTION_INFO_DEF + 12)
    {
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
    
    char chr[256];
    switch (uiAction)
    {
	case GOSSIP_ACTION_INFO_DEF + 97:
	{
		if (creature->GetEntry() == 695005)
			player->CastSpell(player, 55325, false);
		
		break;
	}
	case GOSSIP_ACTION_INFO_DEF + 98:
	{
		// TEST
		
		// yell
		if (player->GetMaxHealth() == 1)
		{
			DoScriptText(-1109006, creature, player);
		}
		else if (player->GetMaxHealth() == 2)
		{
			DoScriptText(-1109006, creature, player, true);
		}
		// say
		else if (player->GetMaxHealth() == 3)
		{
			DoScriptText(-1001167, creature, player, true);
		}
		else if (player->GetMaxHealth() == 4)
		{
			DoScriptText(-1001167, creature);
		}

		break;
	}
    case GOSSIP_ACTION_INFO_DEF + 1:
    {
		if (player->InBattleGroundOrArena()) 
		{
			player->CLOSE_GOSSIP_MENU();
			return true;
		}

		RegBG(player, creature);
		break;
    }
    case GOSSIP_ACTION_INFO_DEF + 2:
    {
        if (!sWorld.getConfig(CONFIG_IS_LOCAL))
            return true;
        
        sBattleGroundMgr.ToggleTesting();
        ChatHandler(player).PSendSysMessage("DEBUG BG %s", sBattleGroundMgr.isTesting() ? "ENABLED" : "DISABLED");
        BM_Hello(player, creature);
        break;
    }

    case GOSSIP_ACTION_INFO_DEF + 8:
    {
        // called by player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENAS_BLIZZLIKE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+8);
        // not used at the moment
        if (!creature->sendBgNotAvailableByLevel(player, BATTLEGROUND_AA))
            player->GetSession()->SendBattlegGroundList(ObjectGuid(creature->GetGUID()), BATTLEGROUND_AA); // Arena
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 9:
    {
        if (!creature->sendBgNotAvailableByLevel(player, BATTLEGROUND_WS))
            player->GetSession()->SendBattlegGroundList(ObjectGuid(creature->GetGUID()), BATTLEGROUND_WS); // WSG
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 10:
    {
        if (!creature->sendBgNotAvailableByLevel(player, BATTLEGROUND_AB))
            player->GetSession()->SendBattlegGroundList(ObjectGuid(creature->GetGUID()), BATTLEGROUND_AB); // AB
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 11:
    {
        if (!creature->sendBgNotAvailableByLevel(player, BATTLEGROUND_EY))
            player->GetSession()->SendBattlegGroundList(ObjectGuid(creature->GetGUID()), BATTLEGROUND_EY); // EYE
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 12:
    {
        if (!creature->sendBgNotAvailableByLevel(player, BATTLEGROUND_AV))
            player->GetSession()->SendBattlegGroundList(ObjectGuid(creature->GetGUID()), BATTLEGROUND_AV); // AV
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 13:
    {
        BM_Hello(player, creature);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 14: // go to main menu
    {
        BM_Hello(player, creature);
        break;
    }

	case GOSSIP_ACTION_INFO_DEF + 62: // ladder
	{
		Ladder_TeamInfo const* ladder5v5 = getBattleGroundMgr()->getLadder(ARENA_TEAM_5v5);

		for (uint32 i = 0; i < LADDER_CNT; i++)
		{
			// no team - break. Means no other teams as well.
			if (!ladder5v5[i].Id)
				break;

			// there must be a member
			const Ladder_PlayerInfo &member = ladder5v5[i].MembersInfo[0];

			char gossip[256];
			sprintf(gossip, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_1_STATS),
				i + 1, member.PlayerName.c_str(), PlayerRaceAndClassToString(member.Race, member.Class).c_str(), member.Wins, member.Loses, member.Rating);
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 62); // goes back here again. Click "back" or "main menu" to get out of here
		}

		player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
		player->SEND_GOSSIP_MENU(990031, creature->GetGUID()); // INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES ('990031', 'Position in rating. Name(Race, Class) - Wins / Loses - Rating')
		break;
	}

    // BG stats
    //case GOSSIP_ACTION_INFO_DEF + 76:
    //{
    //    uint32 games = 0;
    //    uint32 wins = 0;
    //    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT bg_games,bg_wins FROM characters_bg WHERE guid = %u", player->GetGUIDLow());
    //    if (result)
    //    {
    //        games = (*result)[0].GetUInt32();
    //        wins = (*result)[1].GetUInt32();
    //    }

    //    char gossip[1024];
    //    sprintf(gossip, player->GetSession()->GetHellgroundString(LANG_BG_WINS_CURR), games, wins);
    //    player->ADD_GOSSIP_ITEM(0, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 76);
    //    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
    //    player->SEND_GOSSIP_MENU(68, creature->GetGUID());
    //    break;
    //}
    case GOSSIP_ACTION_INFO_DEF + 99:
    {
        player->GetSession()->SendListInventory(creature->GetGUID());
        player->CLOSE_GOSSIP_MENU();
        break;
    }
	// exchange marks
    case GOSSIP_ACTION_INFO_DEF + 100:
    {
        char chr[256];
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 13);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 14);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 15);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 16);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        break;
    }
    // exchange marks
    case GOSSIP_ACTION_INFO_DEF + 100 + 1:
    case GOSSIP_ACTION_INFO_DEF + 100 + 2:
    case GOSSIP_ACTION_INFO_DEF + 100 + 3:
    case GOSSIP_ACTION_INFO_DEF + 100 + 4:
    case GOSSIP_ACTION_INFO_DEF + 100 + 5:
    case GOSSIP_ACTION_INFO_DEF + 100 + 6:
    case GOSSIP_ACTION_INFO_DEF + 100 + 7:
    case GOSSIP_ACTION_INFO_DEF + 100 + 8:
    case GOSSIP_ACTION_INFO_DEF + 100 + 9:
    case GOSSIP_ACTION_INFO_DEF + 100 + 10:
    case GOSSIP_ACTION_INFO_DEF + 100 + 11:
    case GOSSIP_ACTION_INFO_DEF + 100 + 12:
    {
        ItemPosCountVec dest;
        uint32 OldCount = 0;
        uint32 NewCount = 0;
        uint32 ItemId = 0;
        uint32 NewItemId = 0;
        getMarksAndRatioByAction(uiAction, ItemId, NewItemId, OldCount, NewCount);
        if (!ItemId || !NewItemId || !OldCount || !NewCount)
        {
            char chrErr[256];
            sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
            player->GetSession()->SendNotification(chrErr);
            return false;
        }

        if (!player->HasItemCount(ItemId, OldCount))
        {
            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_NOT_ENOUGH_MARKS), player->GetGUID());
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, NewItemId, NewCount);
        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
        {
            player->SendEquipError(msg, NULL, NULL);
            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_INVENTORY_NO_SPACE), player->GetGUID());
            player->CLOSE_GOSSIP_MENU();
            return false;
        }
		player->DestroyItemCount(ItemId, OldCount, true, false, "MARK_EXCHANGED");
        Item* Item = player->StoreNewItem(dest, NewItemId, true, Item::GenerateItemRandomPropertyId(NewItemId));
        player->SendNewItem(Item, NewCount, true, false);

        switch (uiAction)
        {
        case GOSSIP_ACTION_INFO_DEF + 100 + 1:
        case GOSSIP_ACTION_INFO_DEF + 100 + 2:
        case GOSSIP_ACTION_INFO_DEF + 100 + 3:
        {
            BM_Gossip(player, creature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 13);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 100 + 4:
        case GOSSIP_ACTION_INFO_DEF + 100 + 5:
        case GOSSIP_ACTION_INFO_DEF + 100 + 6:
        {
            BM_Gossip(player, creature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 14);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 100 + 7:
        case GOSSIP_ACTION_INFO_DEF + 100 + 8:
        case GOSSIP_ACTION_INFO_DEF + 100 + 9:
        {
            BM_Gossip(player, creature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 15);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 100 + 10:
        case GOSSIP_ACTION_INFO_DEF + 100 + 11:
        case GOSSIP_ACTION_INFO_DEF + 100 + 12:
        {
            BM_Gossip(player, creature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 16);
            break;
        }
        }
        return true;
    }
    case GOSSIP_ACTION_INFO_DEF + 100 + 13:
    {
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_5_FROM_2), 8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 1);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_5_FROM_2), 8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 2);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), 4, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 3);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 100 + 14:
    {
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG), 2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 4);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), 2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG), player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 5);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 6);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 100 + 15:
    {
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG), 2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 7);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), 2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 8);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 9);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 100 + 16:
    {
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_1_FROM_1), player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG), player->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 10);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 4, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 11);
        sprintf(chr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 4, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG), 3, player->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 12);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
        player->SEND_GOSSIP_MENU(68, creature->GetGUID());
        break;
    }
    }

    return true;
}

void AddSC_BM()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_bg_multi";
    newscript->pGossipHello = &BM_Hello;
    newscript->pGossipSelect = &BM_Gossip;
    newscript->RegisterSelf();
}
