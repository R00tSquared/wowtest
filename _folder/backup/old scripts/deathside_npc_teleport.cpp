// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

void SendDefaultMenu_TeleNpc(Player* Plr, Creature* pCreature, uint32 action)
{
    // Not allow in combat
    if (Plr->isInCombat())
    {
        Plr->CLOSE_GOSSIP_MENU();
        Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_IN_COMBAT));
        return;
    }
 
    switch(action)
    {
        case 1000: //Orgrimmar
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 1582.05f, -4418.27f, 8.05f, 0);
            break;
        case 2000: //Stormwind
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -8843.74f, 611.06f, 92.76f, 0);
            break;
        case 3000: //Common
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ARMORY), GOSSIP_SENDER_MAIN, 3005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MOON_MINE), GOSSIP_SENDER_MAIN, 3015);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_HOLY_HOLT), GOSSIP_SENDER_MAIN, 3025);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 10000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 3005:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -2992.82f, 6364.22f, 95.1822f, 0);
            break;
        case 3015:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -10839.11f, -3020.59f, 48.5f, 0);
            break;
        case 3020:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -558.455f, -3842.81f, 240.554f, 5.49f);
            break;
        case 3025:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -4814.63f, -1034.63f, 438.682f, 0);
            break;
        case 4000:
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GRUULS_LAIR), GOSSIP_SENDER_MAIN, 4005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SCARLET_MONASTERY), GOSSIP_SENDER_MAIN, 4010);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_NAXXRAMAS), GOSSIP_SENDER_MAIN, 4015);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CORE_OF_HORROR), GOSSIP_SENDER_MAIN, 4020);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_TEMPLE_LOST_SOULS), GOSSIP_SENDER_MAIN, 4025);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 10000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 4005:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 3526.54f, 5151.16f, -0.484f, 0);
            break;
        case 4010:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, 2891.64f, -810.264f, 160.331f, 3.13f);
            break;
        case 4015:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, 3101.8f, -3714.33f, 132.811f, 0);
            break;
        case 4020:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(230, 1209.46f, -366.757f, -93.1152f, 0);
            break;
        case 4025:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -341.563f, 3139.97f, -102.928f, 0);
            break;
        case 5000:
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GURUBASHI_ARENA), GOSSIP_SENDER_MAIN, 5005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DIRE_MAUL), GOSSIP_SENDER_MAIN, 5010);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 10000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5005:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -13204.9f, 275.904f, 22.5799f, 0);
            break;
        case 5010:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -3750.61f, 1092.94f, 131.969f, 4.57f);
            break;
        case 6000:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -5495.41f, -454.557f, 396.438f, 6.02f);
            break;
        case 7000:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -5091.5f, -800.298f, 495.128f, 0);
            break;
        case 10000:
            if (Plr->GetTeam() == ALLIANCE)
                Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STORMWIND), GOSSIP_SENDER_MAIN, 2000);
            else
                Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ORGRIMMAR), GOSSIP_SENDER_MAIN, 1000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MAIN), GOSSIP_SENDER_MAIN, 3000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_INSTANCES), GOSSIP_SENDER_MAIN, 4000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ARENAS), GOSSIP_SENDER_MAIN, 5000);
            if (Plr->GetTeam() == ALLIANCE)
                Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_IRONFORGE), GOSSIP_SENDER_MAIN, 7000);
            else
                Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_IRONFORGE), GOSSIP_SENDER_MAIN, 6000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
    }
}

bool DeathSide_Npc_Teleporter(Player* Plr, Creature* pCreature)
 {
    Plr->PlayerTalkClass->ClearMenus();
    if (Plr->GetTeam() == ALLIANCE)
        Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STORMWIND), GOSSIP_SENDER_MAIN, 2000);
    else
        Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ORGRIMMAR), GOSSIP_SENDER_MAIN, 1000);
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MAIN), GOSSIP_SENDER_MAIN, 3000);
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_INSTANCES), GOSSIP_SENDER_MAIN, 4000);
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ARENAS), GOSSIP_SENDER_MAIN, 5000);
    if (Plr->GetTeam() == ALLIANCE)
        Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_IRONFORGE), GOSSIP_SENDER_MAIN, 7000);
    else
        Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_IRONFORGE), GOSSIP_SENDER_MAIN, 6000);
    Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
    return true;
}

 bool DeathSide_Npc_Teleporter_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
 {
    // Main menu
    if (uiSender == GOSSIP_SENDER_MAIN)
    SendDefaultMenu_TeleNpc(pPlayer, pCreature, uiAction);
 
    return true;
 }

 void AddSC_DeathSide_Npc_Teleporter()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Npc_Teleporter";
     newscript->pGossipHello           = &DeathSide_Npc_Teleporter;
     newscript->pGossipSelect  = &DeathSide_Npc_Teleporter_Gossip;
     newscript->RegisterSelf();
 }