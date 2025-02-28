// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "ChannelMgr.h"
#include "Channel.h"

#include <array>
//bool close(Player* player, Creature* creature, uint32 string)
//{
//    creature->Whisper(player->GetSession()->GetHellgroundString(string), player->GetGUID());
//    Instance_Teleporter_Hello(player, creature);
//    return false;
//}

// 15420-15434 - 15435-15448
std::array<uint32, 15> strings_x100 =
{
	15420, // 0  What is server rates?                                 
	15421, // 1  Where to get starting equipment?                      
	15422, // 2  Where are the portals to Shattrath and another cities?
	15423, // 3  How to quickly get professions                        
	15424, // 4  What is HP/damage reduction for creatures?            
	15425, // 5  Where is teleporter to instances?                     
	15426, // 6  Where to register BG and arena?                       
	15427, // 7  Where to read server rules?                           
	15428, // 8  Where to report bugs?                                 
	15429, // 9  How to get good gear to be taken to raids?            
	15430, // 10 How to change character appearance/race/sex?          
	15431, // 11 How to contact support?                               
	15432, // 12 How to change message language?                       
	15433, // 13 How to open donate shop?                              
	15434, // 14 What bonuses can I get by inviting friends?           
};

// 15660-15674 - 15675-15688
std::array<uint32, 15> strings_x5 =
{
	15660, // 0  What is server rates?                                 
	15661, // 1  What is currently progression phase?               
	15662, // 2  How to set experience rates to x1?
	15663, // 3  How to freeze experience gaining?                      
	15664, // 4  Where to get Talent Book for dual spec?        
	15665, // 5  Can I group and interact with other faction?                 
	15666, // 6  Is there any promotions for players?             
	15427, // 7  Where to read server rules?                           
	15428, // 8  Where to report bugs?                                 
	15667, // 9  Where can I find the events schedule?          
	15430, // 10 How to change character appearance/race/sex?          
	15431, // 11 How to contact support?                               
	15432, // 12 How to change message language?                       
	15433, // 13 How to open donate shop?                              
	15434, // 14 What bonuses can I get by inviting friends?       
};

uint32 str_size = 15;

void Action_Teleport(Player* player, Creature* creature, uint32 action, bool teleport);
void Send_List(Player* player, Creature* creature);

bool Npc_Welcome_Hello(Player* player, Creature* creature)
{ 
    if (!player->GetSession()->IsAccountFlagged(ACC_LANG_CHOOSED))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_WELCOME_SELECT_EN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_WELCOME_SELECT_RU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->SEND_GOSSIP_MENU(990111, creature->GetGUID());
    }
    else
    {
        Send_List(player, creature);
    }

    return true;
}

bool Npc_Welcome_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
	std::array<uint32, 15> strings = sWorld.isEasyRealm() ? strings_x100 : strings_x5;

	uint32 action = uiAction - GOSSIP_ACTION_INFO_DEF;

    // choosed en
    if (action == 1)
    {       
        player->GetSession()->RemoveAccountFlag(ACC_INFO_LANG_RU);
		player->GetSession()->AddAccountFlag(ACC_LANG_CHOOSED);
        Send_List(player, creature);
		player->IntiveToGlobalChannel(1);

    }
    // choosed ru
    else if (action == 2)
    {        
        player->GetSession()->AddAccountFlag(ACC_INFO_LANG_RU);
        player->GetSession()->AddAccountFlag(ACC_LANG_CHOOSED);
        ChatHandler(player).SendSysMessage(LANG_INFO_LANGUAGE_RUSSIAN);
		player->IntiveToGlobalChannel(2);

        Send_List(player, creature);
    }
    // main menu
    else if (action == 3)
    {
        Send_List(player, creature);
    }
    // info
    else if (action >= 10 && action <= str_size + 10)
    {
        action -= 10;
		ChatHandler(player).SendSysMessage(strings[action] + 15);

		if(sWorld.isEasyRealm())
			Action_Teleport(player, creature, action, false);
		else
			Send_List(player, creature);
    }
    // teleport
    else if (action >= 100 && action <= str_size + 100)
    {
        action -= 100;
		Action_Teleport(player, creature, action, true);
    }
     
    return true;
}

void Send_List(Player* player, Creature* creature)
{
	std::array<uint32, 15> strings = sWorld.isEasyRealm() ? strings_x100 : strings_x5;

	uint8 icon;
    for (uint8 i = 0; i < str_size; ++i)
    {
        icon = GOSSIP_ICON_CHAT;
		player->ADD_GOSSIP_ITEM(icon, player->GetSession()->GetHellgroundString(strings[i]), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10 + i);
    }

    player->SEND_GOSSIP_MENU(990112, creature->GetGUID());
}

void Action_Teleport(Player* player, Creature* creature, uint32 action, bool teleport)
{
	if (!sWorld.isEasyRealm())
		return;
	
	WorldLocationPlain loc;

    switch (action)
    {

    case 1: // Where to get starting equipment?
        loc = { 530, -1984.51, 5521.50, -12.42, 2.29 };
        break;
    case 2: // Where are the portals to Shattrath and another cities?
        if (player->GetTeam() == HORDE)
            loc = { 1, 1433.44, -4424.31, 25.23, 3.65 };
        else
            loc = { 0, -8928.23, 542.16, 94.29, 3.76 };
        break;
    case 3: // Where to gain professions up to 300/375?
        loc = { 530, -1988.69, 5512.95, -12.42, 3.49 };
        break;
    case 5: // Where is teleporter to instances?
        loc = { 530, -1930.90, 5480.27, -12.42, 5.50 };
        break;
    case 6: // Where to register BG and arena?
        loc = { 530, -1937.81, 5567.11, -12.42, 2.14 };
        break;
    case 10: // How to change character appearance/race/sex?
        loc = { 530, -1981.69, 5530.39, -12.42, 0.87 };
        break;
    default:
        Send_List(player, creature);
        return;
    }

    if (teleport)
    {
		player->TeleportTo(loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation);
    }
    else
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, player->GetSession()->GetHellgroundString(LANG_WELCOME_TELEPORT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + action);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_NO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->SEND_GOSSIP_MENU(990113, creature->GetGUID());
    }

    return;
}

void AddSC_Npc_Welcome()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_welcome";
    newscript->pGossipHello = &Npc_Welcome_Hello;
    newscript->pGossipSelect = &Npc_Welcome_Gossip;
    newscript->RegisterSelf();
}