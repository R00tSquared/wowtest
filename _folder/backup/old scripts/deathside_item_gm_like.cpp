// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "Language.h"
#include "Chat.h"

#define NPC_TEXT_GM_LIKE_TICKET 990021
#define NPC_TEXT_GM_LIKE_BASIC_MENU 990022

bool DeathSide_Item_GM_Like_Hello(Player* Plr, Item* scriptItem, SpellCastTargets const& targets)
{
    Plr->PlayerTalkClass->ClearMenus();
    std::string ticketText = Plr->getTicketTextByItemGUID(scriptItem->GetGUIDLow());
    WorldPacket data(SMSG_NPC_TEXT_UPDATE, 100);
    data << uint32(NPC_TEXT_GM_LIKE_TICKET);

    for (uint32 i = 0; i < MAX_GOSSIP_TEXT_OPTIONS; ++i)
    {
        data << float(0);
        data << ticketText;
        data << ticketText;
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
    }
    Plr->SendPacketToSelf(&data);

    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOOK_TICKET), GOSSIP_SENDER_MAIN, 2000);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_DOT, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_PROBLEM_SOLVED), GOSSIP_SENDER_MAIN, 2001);
    Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_INTERACT_1, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_DELETE_ITEM), GOSSIP_SENDER_MAIN, 2002);

    Plr->SEND_GOSSIP_MENU(NPC_TEXT_GM_LIKE_BASIC_MENU,scriptItem->GetGUID());
    return true;
}

bool Deathside_Item_GM_Like_Gossip(Player* Plr, Item* scriptItem, uint32 uiSender, uint32 uiAction, SpellCastTargets const& targets)
{
    Plr->PlayerTalkClass->ClearMenus();

    switch(uiAction)
    {
        case 2000:
            Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 2003); // back;
            Plr->SEND_GOSSIP_MENU(NPC_TEXT_GM_LIKE_TICKET,scriptItem->GetGUID());
            return true;
        case 2001:
            if (Plr->setTicketApprovedByItemGUID(scriptItem->GetGUIDLow()))
            {
                Plr->DestroyItem(scriptItem->GetBagSlot(), scriptItem->GetSlot(), true);
            }
            else
            {
                char chrErr[256];
                sprintf(chrErr, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
                ChatHandler(Plr).SendSysMessage(chrErr);
            }
            break;
        case 2002:
            Plr->DestroyItem(scriptItem->GetBagSlot(), scriptItem->GetSlot(), true);
            break;
        case 2003: // go back
            DeathSide_Item_GM_Like_Hello(Plr, scriptItem, targets);
            return true;
        default:
            break;
    }
    Plr->CLOSE_GOSSIP_MENU();
    return true;
}

 void AddSC_DeathSide_Item_GM_Like()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Item_GM_Like";
     newscript->pItemUse          = &DeathSide_Item_GM_Like_Hello;
     newscript->pGossipSelectItem          = &Deathside_Item_GM_Like_Gossip;
     newscript->RegisterSelf();
 }