// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "Language.h"
#include "ObjectAccessor.h"

void getMarksAndRatioByAction(uint32 uiAction, uint32 &OldMark, uint32 &NewMark, uint32 &OldCount, uint32 &NewCount)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        case GOSSIP_ACTION_INFO_DEF + 2:
        case GOSSIP_ACTION_INFO_DEF + 3:
            OldMark = 20558; // warsong
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
        case GOSSIP_ACTION_INFO_DEF + 5:
        case GOSSIP_ACTION_INFO_DEF + 6:
            OldMark = 20559; // Arathi
            break;
        case GOSSIP_ACTION_INFO_DEF + 7:
        case GOSSIP_ACTION_INFO_DEF + 8:
        case GOSSIP_ACTION_INFO_DEF + 9:
            OldMark = 29024; // Eye of the storm
            break;
        case GOSSIP_ACTION_INFO_DEF + 10:
        case GOSSIP_ACTION_INFO_DEF + 11:
        case GOSSIP_ACTION_INFO_DEF + 12:
            OldMark = 20560; // Alterac valley
            break;
    }
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 4:
        case GOSSIP_ACTION_INFO_DEF + 7:
        case GOSSIP_ACTION_INFO_DEF + 10:
            NewMark = 20558; // warsong
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
        case GOSSIP_ACTION_INFO_DEF + 8:
        case GOSSIP_ACTION_INFO_DEF + 11:
            NewMark = 20559; // Arathi
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
        case GOSSIP_ACTION_INFO_DEF + 5:
        case GOSSIP_ACTION_INFO_DEF + 12:
            NewMark = 29024; // Eye of the storm
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
        case GOSSIP_ACTION_INFO_DEF + 6:
        case GOSSIP_ACTION_INFO_DEF + 9:
            NewMark = 20560; // Alterac valley
            break;
    }
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            OldCount = 8;
            NewCount = 3;
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            OldCount = 8;
            NewCount = 3;
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            OldCount = getWorld()->getConfig(CONFIG_REALM_TYPE) == 0/*REALM_X1*/ ? 3 : 4;
            NewCount = 1;
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
            OldCount = 3;
            NewCount = 2;
            break;
        case GOSSIP_ACTION_INFO_DEF + 5:
            OldCount = 2;
            NewCount = 1;
            break;
        case GOSSIP_ACTION_INFO_DEF + 6:
            OldCount = getWorld()->getConfig(CONFIG_REALM_TYPE) == 0/*REALM_X1*/ ? 2 : 3;
            NewCount = 1;
            break;
        case GOSSIP_ACTION_INFO_DEF + 7:
            OldCount = 3;
            NewCount = 2;
            break;
        case GOSSIP_ACTION_INFO_DEF + 8:
            OldCount = 2;
            NewCount = 1;
            break;
        case GOSSIP_ACTION_INFO_DEF + 9:
            OldCount = getWorld()->getConfig(CONFIG_REALM_TYPE) == 0/*REALM_X1*/ ? 2 : 3;
            NewCount = 1;
            break;
        case GOSSIP_ACTION_INFO_DEF + 10:
            OldCount = 1;
            NewCount = 1;
            break;
        case GOSSIP_ACTION_INFO_DEF + 11:
            OldCount = 4;
            NewCount = 3;
            break;
        case GOSSIP_ACTION_INFO_DEF + 12:
            OldCount = 4;
            NewCount = 3;
            break;
    }
}

bool DeathSide_Npc_Mark_Exchanger_Hello(Player* pPlayer, Creature* pCreature)
{
    char chr[256];
    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+13);
    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+14);
    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+15);
    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_START), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+16);
    
    pPlayer->SEND_GOSSIP_MENU(68,pCreature->GetGUID());
    return true;
 }

bool Deathside_Npc_Mark_Exchanger_Gossip( Player *pPlayer, Creature *pCreature, uint32 uiSender, uint32 uiAction)
{
    char chr[256];
    switch (uiAction)
    {
        // After player requested the action
        case GOSSIP_ACTION_INFO_DEF+1:
        case GOSSIP_ACTION_INFO_DEF+2:
        case GOSSIP_ACTION_INFO_DEF+3:
        case GOSSIP_ACTION_INFO_DEF+4:
        case GOSSIP_ACTION_INFO_DEF+5:
        case GOSSIP_ACTION_INFO_DEF+6:
        case GOSSIP_ACTION_INFO_DEF+7:
        case GOSSIP_ACTION_INFO_DEF+8:
        case GOSSIP_ACTION_INFO_DEF+9:
        case GOSSIP_ACTION_INFO_DEF+10:
        case GOSSIP_ACTION_INFO_DEF+11:
        case GOSSIP_ACTION_INFO_DEF+12:
        {
            ItemPosCountVec dest;
            uint32 OldCount = 0;
            uint32 NewCount = 0;
            uint32 ItemId = 0;
            uint32 NewItemId = 0;
            getMarksAndRatioByAction(uiAction, ItemId, NewItemId, OldCount, NewCount);
            if (!ItemId || !NewItemId || !OldCount || !NewCount)
            {
                char chrErr[256];
                sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
                pPlayer->GetSession()->SendNotification(chrErr);
                return false;
            }

            if (!pPlayer->HasItemCount(ItemId, OldCount))
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_NOT_ENOUGH_MARKS), pPlayer->GetGUID());
                pPlayer->CLOSE_GOSSIP_MENU();
                return false;
            }

            uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, NewItemId, NewCount);
            if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
            {
                pPlayer->SendEquipError(msg, NULL, NULL);
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_INVENTORY_NO_SPACE), pPlayer->GetGUID());
                pPlayer->CLOSE_GOSSIP_MENU();
                return false;
            }
            pPlayer->DestroyItemCount(ItemId, OldCount, true, false);
            Item* Item = pPlayer->StoreNewItem(dest, NewItemId, true, Item::GenerateItemRandomPropertyId(NewItemId));
            pPlayer->SendNewItem(Item,NewCount,true,false);

            switch (uiAction)
            {
                case GOSSIP_ACTION_INFO_DEF+1:
                case GOSSIP_ACTION_INFO_DEF+2:
                case GOSSIP_ACTION_INFO_DEF+3:
                {
                    Deathside_Npc_Mark_Exchanger_Gossip(pPlayer, pCreature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
                    break;
                }
                case GOSSIP_ACTION_INFO_DEF+4:
                case GOSSIP_ACTION_INFO_DEF+5:
                case GOSSIP_ACTION_INFO_DEF+6:
                {
                    Deathside_Npc_Mark_Exchanger_Gossip(pPlayer, pCreature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
                    break;
                }
                case GOSSIP_ACTION_INFO_DEF+7:
                case GOSSIP_ACTION_INFO_DEF+8:
                case GOSSIP_ACTION_INFO_DEF+9:
                {
                    Deathside_Npc_Mark_Exchanger_Gossip(pPlayer, pCreature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);
                    break;
                }
                case GOSSIP_ACTION_INFO_DEF+10:
                case GOSSIP_ACTION_INFO_DEF+11:
                case GOSSIP_ACTION_INFO_DEF+12:
                {
                    Deathside_Npc_Mark_Exchanger_Gossip(pPlayer, pCreature, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 16);
                    break;
                }
            }
            return true;
        }
        case GOSSIP_ACTION_INFO_DEF+13:
        {
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_5_FROM_2), 8, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG), 3,  pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_5_FROM_2), 8, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG), 3,  pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), getWorld()->getConfig(CONFIG_REALM_TYPE) == 0/*REALM_X1*/ ? 3 : 4, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+14:
        {
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 3, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG), 2,  pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), 2, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), getWorld()->getConfig(CONFIG_REALM_TYPE) == 0/*REALM_X1*/ ? 2 : 3, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+15:
        {
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 3, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG), 2,  pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), 2, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+8);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1), getWorld()->getConfig(CONFIG_REALM_TYPE) == 0/*REALM_X1*/ ? 2 : 3, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+9);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+16:
        {
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_1_FROM_1), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+10);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 4, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG), 3,  pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+11);
            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2), 4, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY_ENG), 3,  pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM_ENG));
            pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+12);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+17:
        {
            DeathSide_Npc_Mark_Exchanger_Hello(pPlayer, pCreature);
            return true;
        }
    }
    pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 17);
    pPlayer->SEND_GOSSIP_MENU(68,pCreature->GetGUID());
    return true;
}

 void AddSC_DeathSide_Npc_Mark_Exchanger()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Npc_Mark_Exchanger";
     newscript->pGossipHello          = &DeathSide_Npc_Mark_Exchanger_Hello;
     newscript->pGossipSelect         = &Deathside_Npc_Mark_Exchanger_Gossip;
     newscript->RegisterSelf();
 }
