// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "Shop.h"

bool DeathSide_npc_shop(Player* Plr, Creature* pCreature)
{
    sWorld.GetShop()->SendShopList(pCreature, Plr, 2001/*basic menu*/);
    return true;
}

 bool DeathSide_npc_shop_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
 {
    if (uiAction >= 100000) // accepting item buy
    {
        sWorld.GetShop()->HandleShopDoBuy(pCreature, pPlayer, uiAction-100000, uiSender);
        return true;
    }
    else if (uiAction >= 1999)
    {
        sWorld.GetShop()->SendShopList(pCreature, pPlayer, uiAction);
        return true;
    }

    return true;
 }

 void AddSC_DeathSide_npc_shop()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_npc_shop";
     newscript->pGossipHello           = &DeathSide_npc_shop;
     newscript->pGossipSelect  = &DeathSide_npc_shop_Gossip;
     newscript->RegisterSelf();
 }