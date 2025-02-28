// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Map.h"

bool NR_Hello(Player* player, Creature* creature)
{
    char gossip11[1024];
    sprintf(gossip11, player->GetSession()->GetHellgroundString(15573), player->GetSession()->GetAccountId());
    player->ADD_GOSSIP_ITEM(0, gossip11, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    
    char gossip[1024];
    sprintf(gossip, player->GetSession()->GetHellgroundString(15567), player->GetSession()->players_invited);
    player->ADD_GOSSIP_ITEM(0, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    
    char gossip1[1024];
    sprintf(gossip1, player->GetSession()->GetHellgroundString(15568), player->GetSession()->active_referrals);
    player->ADD_GOSSIP_ITEM(0, gossip1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "----------------------", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    char gossip2[1024];
    sprintf(gossip2, player->GetSession()->GetHellgroundString(15569), player->GetSession()->GetRAFCoins());
    player->ADD_GOSSIP_ITEM(6, gossip2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    // shop

	if (sWorld.isEasyRealm())
	{
		player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(15571), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // 1 coin = 50 MC
	}
	else
	{
		player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(16554), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // 1 coin = 50 Gold
	}

    player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(15572), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2); // 5 coins = Bone Gryphon

    player->SEND_GOSSIP_MENU(990117, creature->GetGUID());
    return true;
}

bool NR_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
		// isBeta
		if (player->GetSession()->GetRAFCoins() >= 1)
        {
			if (sWorld.isEasyRealm())
			{
				if (player->CanStoreItemCount(MOON_COIN, 50))
				{
					player->GiveItem(MOON_COIN, 50);
					player->GetSession()->modifyRAFCoins(-1);
					sLog.outLog(LOG_RAF, "Player %s bougth Moon Coin x50", player->GetName());
				}
			}
			else
			{
				// 50 Gold
				if (player->CanStoreItemCount(695004, 5))
				{
					player->GiveItem(695004, 5);
					player->GetSession()->modifyRAFCoins(-1);
					sLog.outLog(LOG_RAF, "Player %s bougth Gold x100", player->GetName());
				}
			}
        }
        else
            ChatHandler(player).PSendSysMessage(15570);
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 2)
    {
        if (player->GetSession()->GetRAFCoins() >= 5)
        {
            if (player->CanStoreItemCount(BONE_GRYPHON_SCALE, 1) && !player->HasItemCount(BONE_GRYPHON, 1))
            {
                player->GetSession()->modifyRAFCoins(-5);
                player->GiveItem(BONE_GRYPHON_SCALE, 1);
                sLog.outLog(LOG_RAF, "Player %s bougth Bone Gryphon Scale", player->GetName());
            }
        }
        else
            ChatHandler(player).PSendSysMessage(15570);
    }

    NR_Hello(player,creature);
    return true;
}

void AddSC_Npc_Referral()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_referral";
    newscript->pGossipHello = &NR_Hello;
    newscript->pGossipSelect = &NR_Gossip;
    newscript->RegisterSelf();
}