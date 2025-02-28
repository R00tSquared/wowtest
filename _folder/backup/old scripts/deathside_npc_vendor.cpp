// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "deathside_npc_vendor.h"
#include "Language.h"

struct DeathSide_Npc_VendorAI : public Scripted_NoMovementAI
{
    DeathSide_Npc_VendorAI(Creature *c) : Scripted_NoMovementAI(c) {}

    std::list<PlayerGUID_15Items> Player_Item_GUIDS_List;
};

std::list<PlayerGUID_15Items>::iterator FindInList15Items(Creature* _Creature, uint64 playerGUID)
{
    std::list<PlayerGUID_15Items>::iterator itr;
    if (!((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.empty())
    {
        for (itr = ((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.begin();
            itr != ((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.end(); ++itr)
        {
            // delete if no player or can't interact anymore
            if (!(Unit::GetPlayerInWorld(itr->PlayerGUID)) || !((Unit::GetPlayerInWorld(itr->PlayerGUID)))->GetNPCIfCanInteractWith(_Creature->GetGUID(), UNIT_NPC_FLAG_NONE))
            {
                ((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.erase(itr);
                if (!((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.empty())
                {
                    itr = ((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.begin();
                    continue;
                }
                else
                    break;
            }
            if (itr->PlayerGUID == playerGUID) // found needed itr - there's player here
                break;
        }
    }
    return itr;
}

void AddToTheList(Creature* _Creature, PlayerGUID_15Items &WhatToAdd)
{
    std::list<PlayerGUID_15Items>::iterator itr = FindInList15Items(_Creature, WhatToAdd.PlayerGUID);
    
    // addind new player to the list
    if (((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.empty() 
        || itr == ((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.end())
    {
        ((DeathSide_Npc_VendorAI*)_Creature->AI())->Player_Item_GUIDS_List.push_back(WhatToAdd);
    }
    else // recheck player that we've found
    {
        itr->PlayerGUID = WhatToAdd.PlayerGUID;
        memcpy(&itr->ItemsGUID, &WhatToAdd.ItemsGUID, sizeof(itr->ItemsGUID));
    }
}

uint64 DeathSide_Npc_Vendor_Manipulation(uint8 &slot, int32 GossipNumber, Player * pPlayer)
{
    while (slot < 18)
    {
        Item *pItem = pPlayer->GetItemByPos(255, slot);
        slot++;
        if (pItem)
        {
            if (ItemPrototype const* proto = pItem->GetProto())
            {
                if (proto->Name1 && proto->Quality == 5 && (proto->ItemId >= 903180 || proto->ItemId == 339111))
                {
                    uint32 count = 0;
                    switch (proto->InventoryType)
                    {
                        case 17:
                        case 2: count = 50; break;
                        case 1:
                        case 3:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 14:
                        case 13:
                        case 16:
                        case 20:
                        case 21:
                        case 22:
                        case 23:
                        case 10: count = 25; break;
                        case 12: count = 20; break;
                        case 11:
                        case 15:
                        case 25:
                        case 26:
                        case 28: count = 30; break;
                        default:
                            break;
                    }
                    if (!count)
                        continue;
                    char ItemName[256];
                    std::string ItemNameLocale;
                    sprintf(ItemName, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_NPC_VENDOR_TEXT), pPlayer->GetItemNameLocale(proto, &ItemNameLocale), count);
                    char chr2[512];
                    sprintf(chr2, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MSG_CODEBOX_SELL_NPC_VENDOR), count, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_CODE_SELL_NPC_VENDOR));
                    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, ItemName, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + GossipNumber, chr2, 0, true);
                    return pItem->GetGUID();
                }
            }
        }
    }
    return 0;
 }

void DeathSide_Npc_Vendor_Gossip_Manipulation(uint64 guid, Player *pPlayer, Creature* pCreature)
{
    Item *Item = pPlayer->GetItemByGuid(guid);
    if (!Item)
        return;
    ItemPosCountVec dest;
    uint32 count;
    switch (Item->GetProto()->InventoryType)
    {
        case 17:
        case 2: count = 50; break;
        case 1:
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 14:
        case 13:
        case 16:
        case 20:
        case 21:
        case 22:
        case 23:
        case 10: count = 25; break;
        case 12: count = 20; break;
        case 11:
        case 15:
        case 25:
        case 26:
        case 28: count = 30; break;
        default:
            return;
    }
    uint32 BadgeId = 29434;
    uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, BadgeId, count);
    if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
    {
        pPlayer->SendEquipError(msg, NULL, NULL);
        pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_INVENTORY_NO_SPACE), pPlayer->GetGUID());
        return;
    }
    uint32 destroycount = 1;
    pPlayer->DestroyItemCount(Item, destroycount, true);
    Item = pPlayer->StoreNewItem(dest, BadgeId, true, Item::GenerateItemRandomPropertyId(BadgeId));
    pPlayer->SendNewItem(Item,1,true,false);
}

bool DeathSide_Npc_Vendor_Hello(Player* pPlayer, Creature* pCreature)
{
    pPlayer->PlayerTalkClass->ClearMenus();
    uint8 slot = 0;
    PlayerGUID_15Items WhatToAdd;
    for (uint8 i = 0; i < 15; i++)
    {
        WhatToAdd.ItemsGUID[i] = DeathSide_Npc_Vendor_Manipulation(slot, i, pPlayer);
        if (!WhatToAdd.ItemsGUID[i])
            break;
    }
    if (!WhatToAdd.ItemsGUID[0])
    {
        pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_NO_LEG_ITEM_EQUIPPED), pPlayer->GetGUID());
        return true;
    }
    WhatToAdd.PlayerGUID = pPlayer->GetGUID();
    AddToTheList(pCreature, WhatToAdd);
    pPlayer->SEND_GOSSIP_MENU(13,pCreature->GetGUID());
    return true;
 }

bool Deathside_Npc_Vendor_Gossip_Code( Player *pPlayer, Creature *pCreature, uint32 uiSender, uint32 uiAction, const char* sCode )
{
    int i = -1;
    std::string Str = sCode;
    if (StringToUpper(Str))
    {
        try
        {
            i = strcmp(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_CODE_SELL_NPC_VENDOR), Str.c_str());
        } catch(char *str) {error_db_log(str);}
    }

    std::list<PlayerGUID_15Items>::iterator itr = FindInList15Items(pCreature, pPlayer->GetGUID());
    if (!(((DeathSide_Npc_VendorAI*)pCreature->AI())->Player_Item_GUIDS_List.empty() 
            || itr == ((DeathSide_Npc_VendorAI*)pCreature->AI())->Player_Item_GUIDS_List.end()))
    {
        if (i == 0)
            DeathSide_Npc_Vendor_Gossip_Manipulation(itr->ItemsGUID[uiAction-GOSSIP_ACTION_INFO_DEF], pPlayer, pCreature);
        ((DeathSide_Npc_VendorAI*)pCreature->AI())->Player_Item_GUIDS_List.erase(itr);
    }
    pPlayer->CLOSE_GOSSIP_MENU();
    return false;
}

CreatureAI* GetAI_DeathSide_Npc_VendorAI(Creature* pCreature)
{
return new DeathSide_Npc_VendorAI (pCreature);
}

 void AddSC_DeathSide_Npc_Vendor()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Npc_Vendor";
     newscript->GetAI = &GetAI_DeathSide_Npc_VendorAI;
     newscript->pGossipHello          = &DeathSide_Npc_Vendor_Hello;
     newscript->pGossipSelectWithCode = &Deathside_Npc_Vendor_Gossip_Code;
     newscript->RegisterSelf();
 }
