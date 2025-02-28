// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"

//bool close(Player* player, Creature* creature, uint32 string)
//{
//    creature->Whisper(player->GetSession()->GetHellgroundString(string), player->GetGUID());
//    Instance_Teleporter_Hello(player, creature);
//    return false;
//}

// 15 gossip items max!

bool Npc_Command_Hello(Player* player, Creature* creature)
{
    player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_RAIDS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_INSTANCES), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_NO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    player->SEND_GOSSIP_MENU(990101, creature->GetGUID());
    return true;
}

bool Npc_Command_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    if (player->isInCombat())
    {
        player->CLOSE_GOSSIP_MENU();
        player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_IN_COMBAT));
        return true;
    }

    switch (uiAction)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
    {
        player->ADD_GOSSIP_ITEM(2, "The Sunwell", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(990102, creature->GetGUID());
        return true;
    }
    }

    player->CLOSE_GOSSIP_MENU();
    return true;
}

void AddSC_Npc_Command()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_command";
    newscript->pGossipHello = &Npc_Command_Hello;
    newscript->pGossipSelect = &Npc_Command_Gossip;
    newscript->RegisterSelf();
}