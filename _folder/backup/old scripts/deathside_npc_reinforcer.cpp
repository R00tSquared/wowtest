// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "deathside_npc_reinforcer.h"
#include "Language.h"

struct DeathSide_Npc_ReinforcerAI : public Scripted_NoMovementAI
{
    DeathSide_Npc_ReinforcerAI(Creature *c) : Scripted_NoMovementAI(c) {}

    std::list<PlayerGUID_8Items> Player_Item_GUIDS_List;
};

std::list<PlayerGUID_8Items>::iterator FindInList8Items(Creature* _Creature, uint64 playerGUID)
{
    std::list<PlayerGUID_8Items>::iterator itr;
    if (!((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.empty())
    {
        for (itr = ((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.begin();
            itr != ((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.end(); ++itr)
        {
            // delete if no player or can't interact anymore
            if (!(Unit::GetPlayerInWorld(itr->PlayerGUID)) || !((Unit::GetPlayerInWorld(itr->PlayerGUID)))->GetNPCIfCanInteractWith(_Creature->GetGUID(), UNIT_NPC_FLAG_NONE))
            {
                ((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.erase(itr);
                if (!((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.empty())
                {
                    itr = ((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.begin();
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

void AddToTheList(Creature* _Creature, PlayerGUID_8Items &WhatToAdd)
{
    std::list<PlayerGUID_8Items>::iterator itr = FindInList8Items(_Creature, WhatToAdd.PlayerGUID);
    
    // addind new player to the list
    if (((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.empty() 
        || itr == ((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.end())
    {
        ((DeathSide_Npc_ReinforcerAI*)_Creature->AI())->Player_Item_GUIDS_List.push_back(WhatToAdd);
    }
    else // recheck player that we've found
    {
        itr->PlayerGUID = WhatToAdd.PlayerGUID;
        memcpy(&itr->ItemsGUID, &WhatToAdd.ItemsGUID, sizeof(itr->ItemsGUID));
    }
}

uint64 DeathSide_Npc_Reinforcer_Manipulation(uint8 &slot, int32 GossipNumber, Player * pPlayer)
{
    uint8 stat_type;
    uint8 level;
    while (slot < 10)
    {
        if (slot == 1 || slot == 3) // neck and shirt?
        {
            slot++;
            continue;
        }
        Item *pItem = pPlayer->GetItemByPos(255, slot);
        slot++;
        if (pItem)
        {
            if (ItemPrototype const* proto = pItem->GetProto())
            {
                char ItemName[256];
                std::string ItemNameLocale;
                if (proto->Name1 && proto->Quality == 5 && (proto->ItemId >= 903180 || proto->ItemId == 339111))
                {
                    if (uint16 EnchantId = pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                    {
                        if (FindReinforcementEntryByEnchantId(EnchantId, stat_type, level))
                            sprintf(ItemName, "[%s] [%u]", pPlayer->GetItemNameLocale(proto, &ItemNameLocale), level + 1);
                    }
                    else
                        sprintf(ItemName, "[%s] [%u]", pPlayer->GetItemNameLocale(proto, &ItemNameLocale), 0);
                    pPlayer->ADD_GOSSIP_ITEM(0, ItemName, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF*10 + GossipNumber);
                    return pItem->GetGUID();
                }
            }
        }
    }
    return 0;
 }

bool DeathSide_Npc_Reinforcer_Gossip_Manipulation(uint64 guid, Player *pPlayer, Creature* pCreature, uint8 Stat, uint8 Level)
{
    Item *Item = pPlayer->GetItemByGuid(guid);
    if (!Item)
    {
        char chrErr[256];
        sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 10);
        pCreature->Whisper(chrErr, pPlayer->GetGUID());
        return false;
    }
    if (uint16 EnchantId = Item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT)) // has enchantment, but we're not sure if it's reinforcer
    {
        uint8 statType;
        uint8 LevelTest;
        if (Level > 0 && Level != 5) // level 5 means enchant delete
        {
            if (!FindReinforcementEntryByEnchantId(EnchantId, statType, LevelTest))// if previous enchant is not reinforcers
            {
                char chrErr[256];
                sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 25);
                pCreature->Whisper(chrErr, pPlayer->GetGUID());
                return false;
            }
            else if (statType != Stat || (LevelTest +1) != Level) // if enchant has other stat or previous level is wrong
            {
                char chrErr[256];
                sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 26);
                pCreature->Whisper(chrErr, pPlayer->GetGUID());
                return false;
            }
        }
    }
    else if (Level > 0)// if has no chant and level > 0 -> trying to hack
    {
        char chrErr[256];
        sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 27);
        pCreature->Whisper(chrErr, pPlayer->GetGUID());
        return false;
    }
    
    pCreature->CastSpell(pCreature, 32990, true); //Enchanting Cast Visual (Rank 1)
    if (Level != 0)
        pPlayer->ApplyEnchantment(Item,TEMP_ENCHANTMENT_SLOT,false);
    if (Level == 5)
    {
        Item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT); // remove chantment
        pPlayer->CheckLevel5Reinforcement();
    }
    else
    {
        uint8 chance = (90-Level*10);
        if (urand(0, 99) < chance)
        {
            Item->SetEnchantment(TEMP_ENCHANTMENT_SLOT, Reinforcements[Stat][Level], 1, 0);
            pPlayer->ApplyEnchantment(Item,TEMP_ENCHANTMENT_SLOT,true);
            if (Level == 4) // chanted last level
                pPlayer->CheckLevel5Reinforcement();
        }
        else
            Item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT); // not lucky :D
    }
    return true;
}

bool DeathSide_Npc_Reinforcer_Hello(Player* pPlayer, Creature* pCreature)
{
    pPlayer->PlayerTalkClass->ClearMenus();
    uint8 slot = 0;
    PlayerGUID_8Items WhatToAdd;
    for (uint8 i = 0; i < 8; i++)
    {
        WhatToAdd.ItemsGUID[i] = DeathSide_Npc_Reinforcer_Manipulation(slot, i, pPlayer);
        if (!WhatToAdd.ItemsGUID[i])
            break;
    }
    if (!WhatToAdd.ItemsGUID[0])
    {
        pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_NO_LEG_ARMOR_EQUIPPED), pPlayer->GetGUID());
        return true;
    }
    WhatToAdd.PlayerGUID = pPlayer->GetGUID();
    AddToTheList(pCreature, WhatToAdd);
    pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_REINFORCEMENT_TABLE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF*5);
    pPlayer->SEND_GOSSIP_MENU(TextIdStart-1,pCreature->GetGUID());
    return true;
 }

bool Deathside_Npc_Reinforcer_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    Item* pItem;
    std::list<PlayerGUID_8Items>::iterator itr = FindInList8Items(pCreature, pPlayer->GetGUID());
    if (((DeathSide_Npc_ReinforcerAI*)pCreature->AI())->Player_Item_GUIDS_List.empty() 
            || itr == ((DeathSide_Npc_ReinforcerAI*)pCreature->AI())->Player_Item_GUIDS_List.end())
    {
        char chrErr[256];
        sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 11);
        pCreature->Whisper(chrErr, pPlayer->GetGUID());
        return false;
    }

    if (uiAction == GOSSIP_ACTION_INFO_DEF*40)
    {
        DeathSide_Npc_Reinforcer_Hello(pPlayer, pCreature);
    }
    else if (uiAction >= GOSSIP_ACTION_INFO_DEF*50 && uiAction <= (GOSSIP_ACTION_INFO_DEF*50+10)) // last number from 0 to 9
    {
        // 50001
        char chr[5];
        sprintf(chr, "%u", uiAction);
        pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF*40);
        pPlayer->SEND_GOSSIP_MENU(TextIdStart+(chr[4]-48),pCreature->GetGUID());
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF*5) // Tablica usileniy
    {
        char ItemName[64];
        for (uint8 stat_type = 0; stat_type < REINFORCE_STAT_COUNT; stat_type++)
        {
            sprintf(ItemName, "%s", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCHANT_NAME_START_AP + stat_type));
            pPlayer->ADD_GOSSIP_ITEM(0, ItemName, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF*50 + stat_type);
        }
        pPlayer->SEND_GOSSIP_MENU(TextIdStart-2,pCreature->GetGUID());
    }
    else if (uiAction >= GOSSIP_ACTION_INFO_DEF*100)// 100110
    {
        char chr[6];
        sprintf(chr, "%u", uiAction);
        pItem = pPlayer->GetItemByGuid(itr->ItemsGUID[(chr[3]-48)]);
        if (!pItem)
        {
            char chrErr[256];
            sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 12);
            pCreature->Whisper(chrErr, pPlayer->GetGUID());
            return false;
        }
        char ItemName[256];
        std::string ItemNameLocale;
        sprintf(ItemName, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_DO_REINFORCE), pPlayer->GetItemNameLocale(pItem->GetProto(), &ItemNameLocale), 1);
        char chr2[512];
        sprintf(chr2, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MSG_CODEBOX_REINFORCE), pPlayer->GetItemNameLocale(pItem->GetProto(), &ItemNameLocale), 90);
        pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, ItemName, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + (uiAction-GOSSIP_ACTION_INFO_DEF*100), chr2, 0, false);
        pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF*40);
        pPlayer->SEND_GOSSIP_MENU(TextIdStart+(chr[4]-48),pCreature->GetGUID());
    }
    else if (uiAction >= GOSSIP_ACTION_INFO_DEF*10)
    {
        pItem = pPlayer->GetItemByGuid(itr->ItemsGUID[uiAction-GOSSIP_ACTION_INFO_DEF*10]);
        if (!pItem)
        {
            char chrErr[256];
            sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 13);
            pCreature->Whisper(chrErr, pPlayer->GetGUID());
            return false;
        }

        uint8 stat_type;
        uint8 level;
        char ItemName[256];
        std::string ItemNameLocale;
        if (uint16 EnchantId = pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        {
            if (FindReinforcementEntryByEnchantId(EnchantId, stat_type, level))
            {
                char chr2[512];
                level++;
                if (level < 5) // can chant more
                {
                    sprintf(ItemName, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_DO_REINFORCE), pPlayer->GetItemNameLocale(pItem->GetProto(), &ItemNameLocale), level + 1);
                    sprintf(chr2, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MSG_CODEBOX_REINFORCE), pPlayer->GetItemNameLocale(pItem->GetProto(), &ItemNameLocale), 90-10*level);
                    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, ItemName, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 
                        (uiAction-GOSSIP_ACTION_INFO_DEF*10)*100 + stat_type*10 + level, chr2, 0, false);
                }
                sprintf(chr2, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MSG_CODEBOX_DESTROY), pPlayer->GetItemNameLocale(pItem->GetProto(), &ItemNameLocale), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_CODE_DESTROY));
                pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_DESTROY_REINFORCE), GOSSIP_SENDER_MAIN, 10 + (uiAction-GOSSIP_ACTION_INFO_DEF*10), chr2, 0, true);
                pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF*40);
            }
            pPlayer->SEND_GOSSIP_MENU(TextIdStart+stat_type,pCreature->GetGUID());
        }
        else
        {
            for (stat_type = 0; stat_type < REINFORCE_STAT_COUNT; stat_type++)
            {
                sprintf(ItemName, "%s", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCHANT_NAME_START_AP + stat_type));
                pPlayer->ADD_GOSSIP_ITEM(0, ItemName, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF*100 + (uiAction-GOSSIP_ACTION_INFO_DEF*10)*100 + stat_type*10);
            }
            pPlayer->SEND_GOSSIP_MENU(TextIdStart-2,pCreature->GetGUID());
        }
    }
    else // 1110
    {
        char Chr[4];
        sprintf(Chr, "%u", uiAction);
        // switch to gossipnumber
        // do enchant for XYZ, where X - Item, Y - stat, Z - level
        uint8 Item = Chr[1]-48;
        uint8 Stat = Chr[2]-48;
        uint8 Level = Chr[3]-48;

        if (pPlayer->HasItemCount(REAGENT_START+ Stat, Level+2))
        {
            if (DeathSide_Npc_Reinforcer_Gossip_Manipulation(itr->ItemsGUID[Item], pPlayer, pCreature, Stat, Level))//make chant
                pPlayer->DestroyItemCount(REAGENT_START+ Stat, Level+2, true, false);
        }
        else
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_NOT_ENOUGH_RUNES), pPlayer->GetGUID());

        DeathSide_Npc_Reinforcer_Hello(pPlayer, pCreature);
    }
    return true;
}

bool Deathside_Npc_Reinforcer_Gossip_Code( Player *pPlayer, Creature *pCreature, uint32 uiSender, uint32 uiAction, const char* sCode )
{
    char Chr[2];
    sprintf(Chr, "%u", uiAction);
    uint8 Item = Chr[1]-48;

    int i = -1;
    std::string Str = sCode;
    if (StringToUpper(Str))
    {
        try
        {
            i = strcmp(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_CODE_DESTROY), Str.c_str());
        } catch(char *str) {error_db_log(str);}
    }
    if (i != 0)
    {
        DeathSide_Npc_Reinforcer_Hello(pPlayer, pCreature);
        return false;
    }

    std::list<PlayerGUID_8Items>::iterator itr = FindInList8Items(pCreature, pPlayer->GetGUID());
    if (!(((DeathSide_Npc_ReinforcerAI*)pCreature->AI())->Player_Item_GUIDS_List.empty() 
            || itr == ((DeathSide_Npc_ReinforcerAI*)pCreature->AI())->Player_Item_GUIDS_List.end()))
        DeathSide_Npc_Reinforcer_Gossip_Manipulation(itr->ItemsGUID[Item], pPlayer, pCreature, REINFORCE_STAT_COUNT, REINFORCE_LEVEL_COUNT/*Only that matters here is LEVEL - REINFORCE_LEVEL_COUNT*/);//Remove chant
    DeathSide_Npc_Reinforcer_Hello(pPlayer, pCreature);
    return false;
}

CreatureAI* GetAI_DeathSide_Npc_ReinforcerAI(Creature* pCreature)
{
return new DeathSide_Npc_ReinforcerAI (pCreature);
}

 void AddSC_DeathSide_Npc_Reinforcer()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Npc_Reinforcer";
     newscript->GetAI = &GetAI_DeathSide_Npc_ReinforcerAI;
     newscript->pGossipHello          = &DeathSide_Npc_Reinforcer_Hello;
     newscript->pGossipSelect          = &Deathside_Npc_Reinforcer_Gossip;
     newscript->pGossipSelectWithCode = &Deathside_Npc_Reinforcer_Gossip_Code;
     newscript->RegisterSelf();
 }
