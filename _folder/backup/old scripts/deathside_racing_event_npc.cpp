// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"

bool DeathSide_Racing_Event_Npc_Hello(Player* pPlayer, Creature* pCreature)
{
    pPlayer->PlayerTalkClass->ClearMenus();
    if (pPlayer->isGameMaster())
        pPlayer->ADD_GOSSIP_ITEM( 5, "1", GOSSIP_SENDER_MAIN, 1000);
    else
        pPlayer->ADD_GOSSIP_ITEM( 5, "2", GOSSIP_SENDER_MAIN, 2000);

    pPlayer->SEND_GOSSIP_MENU(68,pCreature->GetGUID());
    return true;
 }

bool Deathside_Racing_Event_Npc_Gossip( Player *pPlayer, Creature *pCreature, uint32 uiSender, uint32 uiAction)
{
    /*switch (uiAction)
    {
        case 1000:
            // Here announce on all the world about race
            // Here set the event begin for the script like eventStarted = true;
        case 2000:
            // here add guid of the player to the array if there's no such player in the array
            // else
            // say you already had a race or you are already registered for the next race

            // say that the player "X" had registered

            // If there's enough players (10) for the race to start - check is everyone in the world, is everyone existing and then teleport to the start place, adding a morph, 
    }*/
    pPlayer->CLOSE_GOSSIP_MENU();
    return false;
}

 void AddSC_DeathSide_Racing_Event()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Racing_Npc";
     newscript->pGossipHello          = &DeathSide_Racing_Event_Npc_Hello;
     newscript->pGossipSelect        = &Deathside_Racing_Event_Npc_Gossip;
     newscript->RegisterSelf();
 }
