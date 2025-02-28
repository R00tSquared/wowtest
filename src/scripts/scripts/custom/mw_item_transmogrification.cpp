// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "Language.h"
#include "mw_item_transmogrification.h"
#include "Chat.h"
#include "Transmogrification.h"

// CAN add legendaries to transmog, but only lowlevel legendaries

//              4     0             0            0             0     0     0     0     0       0                      max uint32
//               gossip             slot         slot          Itemid-----6_digits--------Itemid                                
// weapons       gossip             0-9(type)    0-2           Up to 999999 entry        
// armor         gossip             0-1(slot)    0-9           Up to 999999 entry
// weapon gossips 10-23
// armor gossips 24-37
// common gossips 38-41


// gossip
// 1000000000 + gossip_number*100000000 + type * 10000000 + slot * 1000000 + itemId

bool isWeaponGossip(uint32 uiAction)
{
    char chr[10];
    sprintf(chr, "%u", uiAction);
    uint32 gossip = (chr[0] - 48) * 10 + (chr[1] - 48);
    return gossip > 9 && gossip < 24;
}

void getInfoFromAction(uint32 uiAction, uint8 & gossip, uint8 & type, uint8 & slot, uint32 & itemId)
{
    gossip = 0;
    type = 0;
    slot = 0;
    itemId = 0;
    char chr[10];
    sprintf(chr, "%u", uiAction);
    gossip = (chr[0] - 48) * 10 + (chr[1] - 48);
    if (gossip > 9 && gossip < 24) // weapon
    {
        type = chr[2] - 48;
        slot = chr[3] - 48;
    }
    else if (gossip > 23 && gossip < 38 || gossip == 39/*info slot*/) // armor
        slot = (chr[2] - 48) ? 10 : (chr[3] - 48);
    itemId = (chr[4] - 48) * 100000 + (chr[5] - 48) * 10000 + (chr[6] - 48) * 1000 + (chr[7] - 48) * 100 + (chr[8] - 48) * 10 + (chr[9] - 48);
}

bool DeathSide_Item_Transmogrification_Hello(Player* Plr, Item* scriptItem, SpellCastTargets const& targets)
{
    Plr->GetTransmogManager()->SetItemGUID(scriptItem->GetGUID());

    Plr->PlayerTalkClass->ClearMenus();
    const char* noItem = ""; // just no string
    char ItemName[256];
    std::string ItemNameLocale;
    ItemPrototype const* itemProto = NULL;
    uint32 itemId = 0;
    for (uint8 i = 0; i < 11; ++i)
    {
        if (sWorld.isEasyRealm() && (i == 2))
            continue;

        if (itemId = Plr->GetTransmogManager()->GetActiveTransEntry(i, 0, false))
        {
            itemProto = Plr->GetProtoFromScriptsByEntry(itemId);
            if (!itemProto)
                itemId = 0;
        }

        if (i > 10)
            sprintf(ItemName, "Unsupported item slot");
        else
        {
            if (itemId)
                sprintf(ItemName, "|cff0000cc%s|r", Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_SLOT_HEAD_A_START + i));
            else
                sprintf(ItemName, "%s", Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_SLOT_HEAD_A_START + i));
        }
        Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, ItemName, GOSSIP_SENDER_MAIN, 2400000000 + i * 1000000);
    }
    Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAINHAND), GOSSIP_SENDER_MAIN, 1000000000);
    Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_OFFHAND), GOSSIP_SENDER_MAIN, 1000000000 + 1000000);
    Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_RANGED), GOSSIP_SENDER_MAIN, 1000000000 + 2000000);

    //Plr->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_TRANS_INFO_MAIN_START), GOSSIP_SENDER_MAIN, 3900000000); // FAQ

    //ChatHandler(Plr).SendSysMessage(Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_TRANS_BETA));
    Plr->SEND_GOSSIP_MENU(990000, scriptItem->GetGUID());

    Plr->CastSpell(Plr, scriptItem->GetProto()->Spells[0].SpellId/*1206*/, false, scriptItem);
    return true;
}

bool Deathside_Item_Transmogrification_Gossip(Player* Plr, Item* scriptItem, uint32 uiSender, uint32 uiAction, SpellCastTargets const& targets)
{
    if (Plr->isGameMaster())
    {
        char trara[256];
        sprintf(trara, "%u", uiAction);
        Plr->Say(trara, LANG_UNIVERSAL);
    }

    Plr->PlayerTalkClass->ClearMenus();
    const char* noItem = ""; // no string
    std::list<uint32>::iterator itr;
    std::list<uint32>::iterator end;
    uint8 slot;
    uint8 type;
    char ItemName[256];
    std::string ItemNameLocale;
    ItemPrototype const* itemProto = NULL;
    uint32 itemId;
    uint8 gossip;
    getInfoFromAction(uiAction, gossip, type, slot, itemId);
    bool Weapon = isWeaponGossip(uiAction);
    uint32 reasonStringEntry = 0;

    // weapon gossips 10-23
    // 10 - First page - selecting slot -> Adding and types
    // armor gossips 24-37
    // 24 - First page - selecting slot -> Second page - save/reset/allsaved
    // common gossips 38-41
    switch (gossip) // 10-42
    {
    case 39:
        if (!slot)
        {
            for (uint8 o = 1; o < 7; ++o)
                Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_TRANS_INFO_MAIN_START + o), GOSSIP_SENDER_MAIN, (uint32)3900000000 + o * 1000000/*slot*/);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, (uint32)38 * 100000000);
            Plr->SEND_GOSSIP_MENU(990017, scriptItem->GetGUID());
        }
        else
        {
            if (slot == 2)
                Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, (uint32)39 * 100000000 + 8 * 1000000);
            else if (slot == 6)
                Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, (uint32)39 * 100000000 + 9 * 1000000);
            if (slot == 8)
                Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, (uint32)39 * 100000000 + 2 * 1000000);
            else if (slot == 9)
                Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, (uint32)39 * 100000000 + 6 * 1000000);
            else
                Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, (uint32)39 * 100000000);
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, (uint32)38 * 100000000);
            Plr->SEND_GOSSIP_MENU(990010 + slot, scriptItem->GetGUID());
        }
        break;
    case 38: // main menu
        DeathSide_Item_Transmogrification_Hello(Plr, scriptItem, targets);
        break;
    case 24:

        Plr->GetTransmogManager()->SendTransmogList(Weapon, slot, type);
        break;
    case 27:
    case 14:
    {
        if (itemProto = Plr->GetProtoFromScriptsByEntry(itemId))
        {
            sprintf(ItemName, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ACTIVATE_MODEL), Plr->GetItemNameLocale(itemProto, &ItemNameLocale));
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, ItemName, GOSSIP_SENDER_MAIN, (gossip + 1) * 100000000 + type * 10000000 + slot * 1000000 + itemId);

            sprintf(ItemName, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_DELETE_TRANS), Plr->GetItemNameLocale(itemProto, &ItemNameLocale));
            char chr2[512];
            sprintf(chr2, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_TRANS_MSG_CODEBOX_DELETE), Plr->GetItemNameLocale(itemProto, &ItemNameLocale), Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_TRANS_CODE_SURE));
            Plr->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, ItemName, GOSSIP_SENDER_MAIN, (gossip + 2) * 100000000 + type * 10000000 + slot * 1000000 + itemId, chr2, 0, true);
        }
        Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, (gossip > 26 ? (gossip - 3) : (gossip - 4)) * 100000000 + slot * 1000000);
        Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, (uint32)38 * 100000000);
        Plr->SEND_GOSSIP_MENU(990004, scriptItem->GetGUID());
        break;
    }
    case 10: // Second page for weapons: ADD + all possible transmogs
    {
        uint32 stringStart = LANG_SCRIPT_TYPE_MAINHAND_0_START;
        if (slot == 1)
            stringStart = LANG_SCRIPT_TYPE_OFFHAND_0_START;
        else if (slot == 2)
            stringStart = LANG_SCRIPT_TYPE_RANGED_0_START;
        uint32 activeItem = 0;
        for (uint8 i = 0; i < MaxTypesToDisplay[slot]; ++i)
        {
            if (slot == 1 && i == 0/*sword*/ && !Plr->CanDualWield())
                continue;

            uint16 skill = 0;
            if (slot == 1)
            {
                if (TypesToDisplay[slot][i] == 6) // books
                    skill = 100; // just do nothing somehow
                else if (TypesToDisplay[slot][i] == 5) // shields
                    skill = SKILL_SHIELD;
                else
                    skill = item_transmog_skills[ItemTransmogsOffSubClasses[TypesToDisplay[slot][i]]];
            }
            else if (slot == 2)
                skill = item_transmog_skills[ItemTransmogsRangedSubClasses[TypesToDisplay[slot][i]]];
            else
                skill = item_transmog_skills[ItemTransmogsMainSubClasses[TypesToDisplay[slot][i]]];

            if (skill == 100)
            {
                if (!CanIUseHoldable[Plr->GetClass() - 1])
                    continue;
            }
            else if (skill && (
                (skill == SKILL_SWORDS &&
                (Plr->GetSkillValue(SKILL_AXES) == 0 && Plr->GetSkillValue(SKILL_MACES) == 0 && Plr->GetSkillValue(SKILL_SWORDS) == 0 && Plr->GetSkillValue(SKILL_FIST_WEAPONS) == 0 && Plr->GetSkillValue(SKILL_DAGGERS) == 0)) ||
                (skill == SKILL_2H_SWORDS &&
                (Plr->GetSkillValue(SKILL_2H_AXES) == 0 && Plr->GetSkillValue(SKILL_2H_MACES) == 0 && Plr->GetSkillValue(SKILL_2H_SWORDS) == 0 && Plr->GetSkillValue(SKILL_STAVES) == 0 && Plr->GetSkillValue(SKILL_POLEARMS) == 0)) ||
                (skill == SKILL_GUNS &&
                (Plr->GetSkillValue(SKILL_BOWS) == 0 && Plr->GetSkillValue(SKILL_GUNS) == 0 && Plr->GetSkillValue(SKILL_CROSSBOWS) == 0)) ||
                (skill == SKILL_WANDS &&
                (Plr->GetSkillValue(SKILL_WANDS) == 0 && Plr->GetSkillValue(SKILL_THROWN) == 0)) ||
                (skill == SKILL_SHIELD &&
                (Plr->GetSkillValue(SKILL_SHIELD) == 0))
                ))
                continue;

            if (activeItem = Plr->GetTransmogManager()->GetActiveTransEntry(slot, TypesToDisplay[slot][i], Weapon))
            {
                itemProto = Plr->GetProtoFromScriptsByEntry(activeItem);
                if (!itemProto)
                    activeItem = 0;
            }
            if (activeItem)
                sprintf(ItemName, "|cff0000cc%s|r", Plr->GetSession()->GetHellgroundString(stringStart + TypesToDisplay[slot][i]));
            else
                sprintf(ItemName, "%s", Plr->GetSession()->GetHellgroundString(stringStart + TypesToDisplay[slot][i]));
            Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, ItemName, GOSSIP_SENDER_MAIN, 12 * 100000000 + TypesToDisplay[slot][i] * 10000000 + slot * 1000000 + activeItem);
        }
        Plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, (uint32)38 * 100000000);
        Plr->SEND_GOSSIP_MENU(990002, scriptItem->GetGUID());
        break;
    }
    case 12:
    {
        Plr->GetTransmogManager()->SendTransmogList(Weapon, slot, type);
        break;
    }
    }
    return true;
}

void AddSC_DeathSide_Item_Transmogrification()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "DeathSide_Item_Transmogrification";
    newscript->pItemUse = &DeathSide_Item_Transmogrification_Hello;
    newscript->pGossipSelectItem = &Deathside_Item_Transmogrification_Gossip;
    newscript->RegisterSelf();
}