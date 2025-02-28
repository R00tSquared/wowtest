// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Map.h"

bool Path_Opener_Hello(Player* player, Creature* creature)
{
    InstanceData* pInstance = creature->GetInstanceData();
    // hyjal
    //if (creature->GetMap()->GetId() == 534)
    //{
    //    if(pInstance->GetData(20) != DONE)
    //        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(15494), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    //}
    //else 
    if (creature->GetMap()->GetId() == 564)
    {
        if (pInstance->GetData(43) != DONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(15493), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }
    //else if (creature->GetMap()->GetId() == 580)
    //{
    //    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(15549), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    //}
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_NO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    
    player->SEND_GOSSIP_MENU(1, creature->GetGUID());
    return true;
}

bool Path_Opener_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    return false; // disabled
    
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        InstanceData* pInstance = creature->GetInstanceData();

        Map* map = creature->GetMap();
        if (!map)
            return false;

        if (map->IsHeroic())
            return false;

        // hyjal
        //if (map->GetId() == 534)
        //{
        //    if (pInstance->GetData(20) != DONE)
        //        pInstance->SetData(20, DONE);

        //    creature->AddObjectToRemoveList();
        //}
        // Black Temple
        //else 
        if (map->GetId() == 564)
        {
            if (pInstance->GetData(43) != DONE)
                pInstance->SetData(43, DONE);

            creature->AddObjectToRemoveList();
        }
        // SWP
        //else if (map->GetId() == 580)
        //{
        //    if (pInstance->GetData(34) != DONE)
        //        pInstance->SetData(34, DONE);

        //    player->TeleportTo(580, 1713.99, 1111.50, 52.76, 0);
        //}
    }

    player->CLOSE_GOSSIP_MENU();
    return true;
}

void AddSC_Npc_Path_Opener()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_path_opener";
    newscript->pGossipHello = &Path_Opener_Hello;
    newscript->pGossipSelect = &Path_Opener_Gossip;
    newscript->RegisterSelf();
}