// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

void SendDefaultMenu_TeleNpc_pvp(Player* Plr, Creature* pCreature, uint32 action)
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
        case 1000: 
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -11216.30f, -1701.31f, -28.34f, 0);
            break;
        case 3000:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -558.455f, -3842.81f, 240.554f, 5.49f);
            break;
        case 5005:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -13204.9f, 275.904f, 22.5799f, 0);
            break;
        case 5010:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -3750.61f, 1092.94f, 131.969f, 4.57f);
            break;
         case 5015:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -2102.943f, 6742.289f, -3.334f, 0);
            break;    
        case 5020:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 2898.399f, 5981.790f, 2.211f, 0);
            break;
        case 4000:
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GURUBASHI_ARENA), GOSSIP_SENDER_MAIN, 5005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DIRE_MAUL), GOSSIP_SENDER_MAIN, 5010);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ARENA_RING), GOSSIP_SENDER_MAIN, 5015);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RING_OF_BLOOD), GOSSIP_SENDER_MAIN, 5020);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_NAGRAND), GOSSIP_SENDER_MAIN, 4004);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_TANARIS), GOSSIP_SENDER_MAIN, 4003);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 10000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
         case 4001:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 1317.321f, -4386.533f, 26.239f, 0);
            break;                
         case 4002:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -9100.371f, 321.795f, 93.712f, 0);
            break;    
         case 4003:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -7920.832f, -3908.220f, 9.076f, 0);
            break;    
         case 4004:
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -2139.770f, 7763.947f, 154.757f, 0);
            break;    
         case 5000:
             Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ORGRIMMAR), GOSSIP_SENDER_MAIN, 4001);
             Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STORMWIND), GOSSIP_SENDER_MAIN, 4002);
             Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 10000);
             Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
             break;
        case 10000:
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DUEL_ZONES), GOSSIP_SENDER_MAIN, 5000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_PVP_ZONES), GOSSIP_SENDER_MAIN, 4000);    
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CRYPT_START_LOC), GOSSIP_SENDER_MAIN, 1000);
            if (Plr->GetClass() == CLASS_HUNTER)
                Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_PET_PLACE), GOSSIP_SENDER_MAIN, 3000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
    }
}

bool DeathSide_Npc_Teleporter_pvp(Player* Plr, Creature* pCreature)
 {
    Plr->PlayerTalkClass->ClearMenus();
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DUEL_ZONES), GOSSIP_SENDER_MAIN, 5000);
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_PVP_ZONES), GOSSIP_SENDER_MAIN, 4000);    
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CRYPT_START_LOC), GOSSIP_SENDER_MAIN, 1000);
    if (Plr->GetClass() == CLASS_HUNTER)
        Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_PET_PLACE), GOSSIP_SENDER_MAIN, 3000);
    Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
    return true;
}

 bool DeathSide_Npc_Teleporter_pvp_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
 {
    // Main menu
    if (uiSender == GOSSIP_SENDER_MAIN)
    SendDefaultMenu_TeleNpc_pvp(pPlayer, pCreature, uiAction);
 
    return true;
 }

 void AddSC_DeathSide_Npc_Teleporter_pvp()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Npc_Teleporter_pvp";
     newscript->pGossipHello           = &DeathSide_Npc_Teleporter_pvp;
     newscript->pGossipSelect  = &DeathSide_Npc_Teleporter_pvp_Gossip;
     newscript->RegisterSelf();
 }