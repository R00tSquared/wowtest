#include "precompiled.h"
#include "Language.h"

void CreateGossips(Player* player, std::map<uint32, uint32> gossips)
{
    for (auto& gossip : gossips)
    {
        uint32 vendor_id = gossip.first;
        uint32 string_id = gossip.second;
        
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, player->GetSession()->GetHellgroundString(string_id), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + vendor_id);
    }
}

bool Hello_Multivendor(Player* player, Creature* creature)
{
    auto& mv = sWorld.multivendors;
    
    if (!mv.count(creature->GetEntry()))
        return true;

    CreateGossips(player, mv[creature->GetEntry()]);
    player->SEND_GOSSIP_MENU(68, creature->GetGUID());

    return true;
}

bool Gossip_Multivendor(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    uint32 vendor_id = uiAction - GOSSIP_ACTION_INFO_DEF;
    if (vendor_id < NPC_MULTIVENDOR_FIRST || vendor_id > NPC_MULTIVENDOR_LAST)
        return true;

    player->GetSession()->SendListInventory(creature->GetGUID(), vendor_id);
    return true;
}

void AddSC_NPC_Multivendor()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "mw_npc_multivendor";
    newscript->pGossipHello = &Hello_Multivendor;
    newscript->pGossipSelect = &Gossip_Multivendor;
    newscript->RegisterSelf();
}