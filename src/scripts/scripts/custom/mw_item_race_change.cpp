// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "Chat.h"

bool MW_race_change_item_A(Player* Plr, Item* pItem, SpellCastTargets const& targets)
{
    Plr->PlayerTalkClass->ClearMenus();

    switch(Plr->GetClass())
    {
        case CLASS_WARRIOR:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_HUMAN),     GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_NIGHT_ELF), GOSSIP_SENDER_MAIN, 1004);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DWARF),     GOSSIP_SENDER_MAIN, 1003);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_GNOME),     GOSSIP_SENDER_MAIN, 1007);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DRAENEI),     GOSSIP_SENDER_MAIN, 1011);
            break;                                                                                   
        }                                                                                                   
        case CLASS_PALADIN:                                                                          
        {                                                                                            
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_HUMAN),     GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DWARF),     GOSSIP_SENDER_MAIN, 1003);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DRAENEI),     GOSSIP_SENDER_MAIN, 1011);
            break;                                                                                   
        }                                                                                            
        case CLASS_ROGUE:                                                                            
        {                                                                                            
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_HUMAN),     GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_NIGHT_ELF), GOSSIP_SENDER_MAIN, 1004);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DWARF),     GOSSIP_SENDER_MAIN, 1003);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_GNOME),     GOSSIP_SENDER_MAIN, 1007);
            break;                                                                                   
        }                                                                                            
        case CLASS_PRIEST:                                                                           
        {                                                                                            
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_HUMAN),     GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_NIGHT_ELF), GOSSIP_SENDER_MAIN, 1004);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DWARF),     GOSSIP_SENDER_MAIN, 1003);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DRAENEI),     GOSSIP_SENDER_MAIN, 1011);
            break;                                                                                   
        }                                                                                            
        case CLASS_MAGE:                                                                             
        {                                                                                            
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_HUMAN),     GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_GNOME),     GOSSIP_SENDER_MAIN, 1007);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DRAENEI),     GOSSIP_SENDER_MAIN, 1011);
            break;                                                                                   
        }                                                                                            
        case CLASS_WARLOCK:                                                                          
        {                                                                                            
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_HUMAN),     GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_GNOME),     GOSSIP_SENDER_MAIN, 1007);
            break;                                                                                   
        }                                                                                            
        case CLASS_HUNTER:                                                                           
        {                                                                                            
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_NIGHT_ELF), GOSSIP_SENDER_MAIN, 1004);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DWARF),     GOSSIP_SENDER_MAIN, 1003);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DRAENEI),     GOSSIP_SENDER_MAIN, 1011);
            break;                                                                                   
        }                                                                                            
        case CLASS_SHAMAN:                                                                           
        {                                                                                            
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DRAENEI),     GOSSIP_SENDER_MAIN, 1011);
            break;                                                                                         
        }                                                                                                  
        case CLASS_DRUID:                                                                                  
        {                                                                                                  
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_NIGHT_ELF), GOSSIP_SENDER_MAIN, 1004);
            break;
        }
    }

    Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pItem->GetGUID());

    Plr->CastSpell(Plr, pItem->GetProto()->Spells[0].SpellId/*1206*/, false, pItem);
    return true;
}

bool MW_race_change_item_H(Player* Plr, Item* pItem, SpellCastTargets const& targets)
{
    Plr->PlayerTalkClass->ClearMenus();

    switch (Plr->GetClass())
    {
        case CLASS_WARRIOR:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ORC), GOSSIP_SENDER_MAIN, 1002);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_UNDEAD), GOSSIP_SENDER_MAIN, 1005);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TAUREN), GOSSIP_SENDER_MAIN, 1006);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TROLL), GOSSIP_SENDER_MAIN, 1008);
            break;
        }
        case CLASS_PALADIN:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_BLOOD_ELF), GOSSIP_SENDER_MAIN, 1010);
            break;
        }
        case CLASS_ROGUE:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ORC), GOSSIP_SENDER_MAIN, 1002);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_UNDEAD), GOSSIP_SENDER_MAIN, 1005);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TROLL), GOSSIP_SENDER_MAIN, 1008);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_BLOOD_ELF), GOSSIP_SENDER_MAIN, 1010);
            break;
        }
        case CLASS_PRIEST:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_UNDEAD), GOSSIP_SENDER_MAIN, 1005);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TROLL), GOSSIP_SENDER_MAIN, 1008);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_BLOOD_ELF), GOSSIP_SENDER_MAIN, 1010);
            break;
        }
        case CLASS_MAGE:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_UNDEAD), GOSSIP_SENDER_MAIN, 1005);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TROLL), GOSSIP_SENDER_MAIN, 1008);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_BLOOD_ELF), GOSSIP_SENDER_MAIN, 1010);
            break;
        }
        case CLASS_WARLOCK:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ORC), GOSSIP_SENDER_MAIN, 1002);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_UNDEAD), GOSSIP_SENDER_MAIN, 1005);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_BLOOD_ELF), GOSSIP_SENDER_MAIN, 1010);
            break;
        }
        case CLASS_HUNTER:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ORC), GOSSIP_SENDER_MAIN, 1002);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TAUREN), GOSSIP_SENDER_MAIN, 1006);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TROLL), GOSSIP_SENDER_MAIN, 1008);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_BLOOD_ELF), GOSSIP_SENDER_MAIN, 1010);
            break;
        }
        case CLASS_SHAMAN:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ORC), GOSSIP_SENDER_MAIN, 1002);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TAUREN), GOSSIP_SENDER_MAIN, 1006);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TROLL), GOSSIP_SENDER_MAIN, 1008);
            break;
        }
        case CLASS_DRUID:
        {
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_TAUREN), GOSSIP_SENDER_MAIN, 1006);
            break;
        }
    }

    Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pItem->GetGUID());

    Plr->CastSpell(Plr, pItem->GetProto()->Spells[0].SpellId/*1206*/, false, pItem);
    return true;
}

bool MW_race_change_item_gossip(Player* Plr, Item* pItem, uint32 uiSender, uint32 uiAction, SpellCastTargets const& targets)
{
    if (uiAction > 1000 && uiAction < 1012 && uiAction != 1009)
        uiAction -= 1000;
    else
    {
        char chrErr[256];
        sprintf(chrErr, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        ChatHandler(Plr).SendSysMessage(chrErr);
        return false;
    }

    // uiAction is race at this point, 1-11 except 9
    if (Plr->m_changeRaceTo)
    {
        ChatHandler(Plr).SendSysMessage(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ALREADY_CHANGING));
        Plr->CLOSE_GOSSIP_MENU();
        return true;
    }

    if (Plr->GetRace() != uiAction)
    {
        int32 bpaction = uiAction;
        Plr->CastCustomSpell(Plr, 55163, &bpaction, NULL, NULL, false);
        Plr->CLOSE_GOSSIP_MENU();
    }
    else
    {
        ChatHandler(Plr).SendSysMessage(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_CANT_SAME_RACE));
        Plr->CLOSE_GOSSIP_MENU();
    }
 
    return true;
}

void AddSC_MW_race_change_item()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "MW_race_change_item_A";
    newscript->pItemUse           = &MW_race_change_item_A;
    newscript->pGossipSelectItem  = &MW_race_change_item_gossip;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "MW_race_change_item_H";
    newscript->pItemUse = &MW_race_change_item_H;
    newscript->pGossipSelectItem = &MW_race_change_item_gossip;
    newscript->RegisterSelf();
}