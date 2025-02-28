// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

bool GossipHello_deathside_npc_gossip_vendor_x100(Player *Plr, Creature *pCreature)
{
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Тканная броня", GOSSIP_SENDER_MAIN, 100001);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Кожаная броня", GOSSIP_SENDER_MAIN, 100002);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Кольчужная броня", GOSSIP_SENDER_MAIN, 100003);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Латная броня", GOSSIP_SENDER_MAIN, 100004);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Одноручные оружия", GOSSIP_SENDER_MAIN, 100005);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Двуручные оружия", GOSSIP_SENDER_MAIN, 100006);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Оружия дальнего боя", GOSSIP_SENDER_MAIN, 100007);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Леворучные оружия", GOSSIP_SENDER_MAIN, 100008);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Ювелирные изделия", GOSSIP_SENDER_MAIN, 100009);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Аксессуары", GOSSIP_SENDER_MAIN, 100010);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Манускрипты/идолы/тотемы", GOSSIP_SENDER_MAIN, 100011);

    Plr->SEND_GOSSIP_MENU(30000, pCreature->GetGUID());
    return true;
}
bool GossipSelect_deathside_npc_gossip_vendor_x100(Player *Plr, Creature *pCreature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, pCreature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 15);
        pCreature->Whisper(chrErr, Plr->GetGUID());
        return false;
    }
    Plr->GetSession()->SendListInventory(Found->GetGUID());
    Plr->CLOSE_GOSSIP_MENU();
    return true;
}

void AddSC_deathside_npc_gossip_vendor_x100()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "deathside_npc_gossip_vendor_x100";
    newscript->pGossipHello = &GossipHello_deathside_npc_gossip_vendor_x100;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_gossip_vendor_x100;
    newscript->RegisterSelf();
}