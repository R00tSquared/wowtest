// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* ###
## deathside_npc_enchanter
### */

#include "precompiled.h"
#include "Language.h"

#define ENCHANT_ILVL_CAP 136

uint32 enchants[160] =
{
    2667, 2713, 2506, 2670, 1904, 1903,                                     // 2h Enchants 6
    3273, 3225, 3222, 2343, 2672, 2671, 2673, 2674, 2675, 2669,             // 1H pg 1 10
    2668, 963, 2666, 2567, 1898, 1900, 803, 3223, 2506, 2713,               // 1H pg 2 10
    2661, 2659, 2381, 2933, 1950, 3233, 1144,                               // Chest 7
    2658, 2656, 2657, 1147, 3826, 2649, 2940, 2939, 3606,                   // boots 9
    2934, 1594, 2322, 2937, 684, 2564, 3246, 2935, 2616,                    // Gloves 1 9
    2615, 2614, 2613, 931, 930, 3860, 3604, 3603,                           // Gloves 2 8
    1593, 1884, 2617, 1891, 369, 2679, 2648, 2647,                          // Bracers 1 8
    2650, 2649, 3759, 3760, 3762, 3761, 3757, 3758,                         // Bracers 2 8  
    1952, 3849, 2654, 1888, 2655, 3229, 1071, 1890, 926, 848,               // Shield 10
    2930, 2928, 3791, 2931, 2929,                                           // Rings 5
    910, 2622, 2648, 2662, 368, 2619, 1257, 1441, 2620, 2664, 2938,         // Cloaks 1 11
    3243, 2664, 2622, 2621, 3728, 3722, 3730,                               // Cloaks 2 7
    3096, 3003, 3002, 3001, 2999, 3004, 2589, 3842, 3818, 3817, 3813, 3795, // Head 12
    2748, 3012, 2746, 2841, 3013,                                           // Legs 5
    2995, 2993, 2997, 2991, 2717, 2721, 2715, 2716, 2978, 2980, 2982, 2986, // Shoulders 11
    2523, 2723, 2724,                                                       // Ranged 3
    // again weapon enchant for off-hand
    3273, 3225, 3222, 2343, 2672, 2671, 2673, 2674, 2675, 2669,             // O1H pg 1 10
    2668, 963, 2666, 2567, 1898, 1900, 803, 3223, 2506, 2713                // O1H pg 2 10
};

bool GossipHello_deathside_npc_enchanter(Player* player, Item* pItem, SpellCastTargets const& targets)
{
    if (player->IsInCombat() || player->HasInvisibilityAura() || player->HasStealthAura() || player->InBattleGroundOrArena() || player->InArena())
    {
        ChatHandler(player).SendSysMessage(LANG_CANT_USE_NOW);
        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    player->PlayerTalkClass->ClearMenus();
    player->ADD_GOSSIP_ITEM(9, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_2H_WEAPON), GOSSIP_SENDER_MAIN, 1);
    player->ADD_GOSSIP_ITEM(9, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_1H_MAIN_HAND), GOSSIP_SENDER_MAIN, 2);
    player->ADD_GOSSIP_ITEM(9, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_OFF_HAND), GOSSIP_SENDER_MAIN, 21);
    player->ADD_GOSSIP_ITEM(9,  player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_RANG_WEAPON), GOSSIP_SENDER_MAIN, 20);
    player->ADD_GOSSIP_ITEM(9, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SHIELD), GOSSIP_SENDER_MAIN, 10);
    player->ADD_GOSSIP_ITEM(8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_HEAD), GOSSIP_SENDER_MAIN, 14);
    player->ADD_GOSSIP_ITEM(8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_CHEST),   GOSSIP_SENDER_MAIN, 4);
    player->ADD_GOSSIP_ITEM(8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SHOULDER), GOSSIP_SENDER_MAIN, 16);
    player->ADD_GOSSIP_ITEM(8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_LEGS), GOSSIP_SENDER_MAIN, 15);
    player->ADD_GOSSIP_ITEM(8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_FEET),    GOSSIP_SENDER_MAIN, 5);
    player->ADD_GOSSIP_ITEM(8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_HANDS),   GOSSIP_SENDER_MAIN, 6);
    player->ADD_GOSSIP_ITEM(8, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_WRIST),   GOSSIP_SENDER_MAIN, 8);
    player->ADD_GOSSIP_ITEM(6, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_RINGS),   GOSSIP_SENDER_MAIN, 11);
    player->ADD_GOSSIP_ITEM(6, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_CLOAK),   GOSSIP_SENDER_MAIN, 12);  
    player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
    return true;
}

void AddEnchant(Player *pPlayer, Item *pItem, uint32 enchantid)
{
    if (!pItem)
        return;
    
    if (pItem->GetProto()->ItemLevel > ENCHANT_ILVL_CAP)
    { 
        pPlayer->GetSession()->SendNotification(pPlayer->GetSession()->GetHellgroundString(LANG_ENCH_TOO_HIGH_LVL_ITEM), pItem->GetProto()->Name1);
        return;
    }
    // remove old enchanting before applying new if equipped
    pPlayer->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, false);

    pItem->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchantid, 0, 0);

    // add new enchanting if equipped
    pPlayer->ApplyEnchantment(pItem, PERM_ENCHANTMENT_SLOT, true);

    pPlayer->GetSession()->SendNotification(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_ENCHANTED), pItem->GetProto()->Name1);
}

bool GossipSelect_deathside_npc_enchanter(Player* player, Item* pItem, uint32 sender, uint32 action, SpellCastTargets const& targets)
{
    if (player->IsInCombat() || player->HasInvisibilityAura() || player->HasStealthAura())
    {
        ChatHandler(player).SendSysMessage(LANG_CANT_USE_NOW);
        player->CLOSE_GOSSIP_MENU();
            return true;
    }

    if (pItem->GetProto()->ItemLevel > ENCHANT_ILVL_CAP)
    {
        player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_ENCH_TOO_HIGH_LVL_ITEM), pItem->GetProto()->Name1);
        return true;
    }

    if((action >= 1 && action <= 6) || (action >= 8 && action <= 22) || (action >= 30 && action <= 33) || (action >= 36 && action <= 70) || (action >= 72 && action <= 79) || (action >= 89 && action <= 98) || (action == 105) || (action >= 107 && action <= 130) || (action >= 138 && action <= 144) || (action >= 150 && action <= 189) || (action == 1000))
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case 1000: // Home Page
                GossipHello_deathside_npc_enchanter(player, pItem, targets);
                break;
            case 1: // 2hand Weapon
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_70_AP), GOSSIP_SENDER_MAIN, 30);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_35_AGILITY), GOSSIP_SENDER_MAIN, 33);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 2: // 1 Hand Weapon Page 1
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_DEATHFROST), GOSSIP_SENDER_MAIN, 36);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_EXECUTIONER), GOSSIP_SENDER_MAIN, 37);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_AGILITY), GOSSIP_SENDER_MAIN, 38);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_81_HB_27_SPD), GOSSIP_SENDER_MAIN, 39);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SOULFROST), GOSSIP_SENDER_MAIN, 40);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SUNFIRE), GOSSIP_SENDER_MAIN, 41);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MONGOOSE), GOSSIP_SENDER_MAIN, 42);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SPELLSURGE), GOSSIP_SENDER_MAIN, 43);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_BATTLEMASTER), GOSSIP_SENDER_MAIN, 44);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_40_SPD), GOSSIP_SENDER_MAIN, 45);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_PAGE_2), GOSSIP_SENDER_MAIN, 3);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 3: // 1 Hand Weapon Page 2
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_STRENGHT), GOSSIP_SENDER_MAIN, 46);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_7_DMG), GOSSIP_SENDER_MAIN, 47);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_30_INTELLECT), GOSSIP_SENDER_MAIN, 48);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_SPIRIT), GOSSIP_SENDER_MAIN, 49);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_LIFESTEALING), GOSSIP_SENDER_MAIN, 50);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_CRUSADER), GOSSIP_SENDER_MAIN, 51);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_FIERY_WEAPON), GOSSIP_SENDER_MAIN, 52);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_ADAM_WEP_CHAIN), GOSSIP_SENDER_MAIN, 53);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_ELEM_WEP_STONE), GOSSIP_SENDER_MAIN, 54);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_ADAM_WEP_STONE), GOSSIP_SENDER_MAIN, 55);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_PAGE_1), GOSSIP_SENDER_MAIN, 2);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 4:  // Chest.
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_6_ALL_STATS), GOSSIP_SENDER_MAIN, 56);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_150_HEALTH), GOSSIP_SENDER_MAIN, 57);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_8_MANA_SEC), GOSSIP_SENDER_MAIN, 58);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_RESILIENCE), GOSSIP_SENDER_MAIN, 59);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_DEFENCE), GOSSIP_SENDER_MAIN, 60);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_150_MANA), GOSSIP_SENDER_MAIN, 61);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_SPIRIT), GOSSIP_SENDER_MAIN, 62);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 5:  // Boots.
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_5_SNARE_ROOT), GOSSIP_SENDER_MAIN, 63);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_4_HP_MP_SEC), GOSSIP_SENDER_MAIN, 64);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_AGILITY), GOSSIP_SENDER_MAIN, 65);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_STAMINA), GOSSIP_SENDER_MAIN, 68);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_9_STAM_MIN_SP), GOSSIP_SENDER_MAIN, 69);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_6_AGIL_MIN_SP), GOSSIP_SENDER_MAIN, 70);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 6: // Gloves
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_10_SPELL_CRIT), GOSSIP_SENDER_MAIN, 72);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_26_ATTACK_POW), GOSSIP_SENDER_MAIN, 73);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_35_HB_12_SPD), GOSSIP_SENDER_MAIN, 74);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_SPD), GOSSIP_SENDER_MAIN, 75);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_STRENGHT), GOSSIP_SENDER_MAIN, 76);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_AGILITY), GOSSIP_SENDER_MAIN, 77);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_SPELL_HIT), GOSSIP_SENDER_MAIN, 79);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 8: // Bracers Page 1
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_24_AP), GOSSIP_SENDER_MAIN, 89);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_5_SPIRIT), GOSSIP_SENDER_MAIN, 90);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_30_HB_10_SPD), GOSSIP_SENDER_MAIN, 91);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_4_ALL_STATS), GOSSIP_SENDER_MAIN, 92);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_INTELLECT), GOSSIP_SENDER_MAIN, 93);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_6_MANA_SEC), GOSSIP_SENDER_MAIN, 94);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_DEFENCE), GOSSIP_SENDER_MAIN, 95);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_STRENGHT), GOSSIP_SENDER_MAIN, 96);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_PAGE_2), GOSSIP_SENDER_MAIN, 9);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 9: // Bracers Page 2
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_SPD), GOSSIP_SENDER_MAIN, 97);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_STAMINA_BRAC), GOSSIP_SENDER_MAIN, 98);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_PAGE_1), GOSSIP_SENDER_MAIN, 8);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 10: // Shield.
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_DEFENCE), GOSSIP_SENDER_MAIN, 105);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_INTELLECT_SHI), GOSSIP_SENDER_MAIN, 107);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_RESISTANCE), GOSSIP_SENDER_MAIN, 108);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_BLOCK), GOSSIP_SENDER_MAIN, 109);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_RESILIENCE), GOSSIP_SENDER_MAIN, 110);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_18_STAMINA), GOSSIP_SENDER_MAIN, 111);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_9_SPIRIT), GOSSIP_SENDER_MAIN, 112);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_FROST_RESISTANCE), GOSSIP_SENDER_MAIN, 113);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_LESSER_PROTECTION), GOSSIP_SENDER_MAIN, 114);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
    
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 11: // Rings
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_HB_7_SPD), GOSSIP_SENDER_MAIN, 115);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_SPD), GOSSIP_SENDER_MAIN, 116);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_4_ALL_STATS_RING), GOSSIP_SENDER_MAIN, 118);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_STRIKING), GOSSIP_SENDER_MAIN, 119);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
    
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 12: // Cloak
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_STEALTH), GOSSIP_SENDER_MAIN, 120);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_DODGE), GOSSIP_SENDER_MAIN, 121);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_DEFENCE_CLOAK), GOSSIP_SENDER_MAIN, 122);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_120_ARMOR), GOSSIP_SENDER_MAIN, 123);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_AGILITY_CLOAK), GOSSIP_SENDER_MAIN, 124);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_FIRE_RES), GOSSIP_SENDER_MAIN, 125);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_ARCANE_RES), GOSSIP_SENDER_MAIN, 126);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_SHADOW_RES), GOSSIP_SENDER_MAIN, 127);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_NATURE_RES), GOSSIP_SENDER_MAIN, 128);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_7_ALL_RES), GOSSIP_SENDER_MAIN, 129);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_SPELL_PENETR), GOSSIP_SENDER_MAIN, 130);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 14: // Head
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_17_STR_16_INT), GOSSIP_SENDER_MAIN, 138);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_34_AP_16_HIT), GOSSIP_SENDER_MAIN, 139);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_22_SPD_14_SP_HIT), GOSSIP_SENDER_MAIN, 140);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_35_HB_12_SPD_7M), GOSSIP_SENDER_MAIN, 141);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_16_DEF_17_DODGE), GOSSIP_SENDER_MAIN, 142);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_18_STM_20_RES), GOSSIP_SENDER_MAIN, 143);
                if (player->GetClass() == CLASS_WARLOCK)
                {
                    player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_10_STM_18_SPD), GOSSIP_SENDER_MAIN, 144);
                }
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 15: // Legs.
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_35_SPD_20_STM), GOSSIP_SENDER_MAIN, 150);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_50_AP_12_CRIT), GOSSIP_SENDER_MAIN, 151);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_66_HB_22_SPD_20STM), GOSSIP_SENDER_MAIN, 152);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_10_STAMINA), GOSSIP_SENDER_MAIN, 153);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_40_STM_12_AGIL), GOSSIP_SENDER_MAIN, 154);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 16: // Shoulder.
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_SP_CRIT_12_SPD), GOSSIP_SENDER_MAIN, 155);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_6MP_22_HB), GOSSIP_SENDER_MAIN, 156);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_CRIT_20_AP), GOSSIP_SENDER_MAIN, 157);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_DEF_10_DODGE), GOSSIP_SENDER_MAIN, 158);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_26_AP_14_CRIT), GOSSIP_SENDER_MAIN, 159);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_SPD_14_SP_CRIT), GOSSIP_SENDER_MAIN, 160);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_31_HB_11_SPD_5MP), GOSSIP_SENDER_MAIN, 161);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_16_STM_100_ARMOR), GOSSIP_SENDER_MAIN, 162);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_15_DODGE_10_DEF), GOSSIP_SENDER_MAIN, 163);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_33_HB_11_SPD_4MP), GOSSIP_SENDER_MAIN, 164);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_18_SPD_10_SP_CR), GOSSIP_SENDER_MAIN, 165);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_30_AP_10_CRIT), GOSSIP_SENDER_MAIN, 166);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 20: // Ranged
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_30_HIT), GOSSIP_SENDER_MAIN, 167);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_12_DMG), GOSSIP_SENDER_MAIN, 168);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_28_CRIT), GOSSIP_SENDER_MAIN, 169);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 21: // Off-hand Weapon Page 1
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_DEATHFROST), GOSSIP_SENDER_MAIN, 170);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_EXECUTIONER), GOSSIP_SENDER_MAIN, 171);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_AGILITY), GOSSIP_SENDER_MAIN, 172);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_81_HB_27_SPD), GOSSIP_SENDER_MAIN, 173);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SOULFROST), GOSSIP_SENDER_MAIN, 174);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SUNFIRE), GOSSIP_SENDER_MAIN, 175);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MONGOOSE), GOSSIP_SENDER_MAIN, 176);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_SPELLSURGE), GOSSIP_SENDER_MAIN, 177);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_BATTLEMASTER), GOSSIP_SENDER_MAIN, 178);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_40_SPD), GOSSIP_SENDER_MAIN, 179);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_PAGE_2), GOSSIP_SENDER_MAIN, 22);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            case 22: // Off-hand Weapon Page 2
            {
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_STRENGHT), GOSSIP_SENDER_MAIN, 180);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_7_DMG), GOSSIP_SENDER_MAIN, 181);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_30_INTELLECT), GOSSIP_SENDER_MAIN, 182);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_20_SPIRIT), GOSSIP_SENDER_MAIN, 183);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_LIFESTEALING), GOSSIP_SENDER_MAIN, 184);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_CRUSADER), GOSSIP_SENDER_MAIN, 185);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_FIERY_WEAPON), GOSSIP_SENDER_MAIN, 186);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_ADAM_WEP_CHAIN), GOSSIP_SENDER_MAIN, 187);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_ELEM_WEP_STONE), GOSSIP_SENDER_MAIN, 188);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_ADAM_WEP_STONE), GOSSIP_SENDER_MAIN, 189);
                player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_PAGE_1), GOSSIP_SENDER_MAIN, 21);
                player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ENCH_MAIN_MENU), GOSSIP_SENDER_MAIN, 1000);
                player->SEND_GOSSIP_MENU(990103, pItem->GetGUID());
            }
                break;
            default:
            {                
                uint32 enchantid = enchants[(action - 30)];
    
                Item *itemTarget = NULL;
    
                if (action >= 30 && action <= 55) // Weapon Enchants.
                {
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (itemTarget && action < 36 && itemTarget->GetProto()->InventoryType != INVTYPE_2HWEAPON)
                    {
                        GossipHello_deathside_npc_enchanter(player, pItem, targets);
                        break;
                    }
    
                    if (action >= 36)
                    {
                        AddEnchant(player, itemTarget, enchantid);
                        if (itemTarget && itemTarget->GetProto()->InventoryType != INVTYPE_WEAPONMAINHAND)
                            itemTarget = NULL;
                    }
                }
                else if (action >= 170 && action <= 190) // Offhand Weapon Enchants.
                {
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (itemTarget)
                    {
                        if(itemTarget->GetProto()->InventoryType == INVTYPE_WEAPON)
                        {
                            AddEnchant(player, itemTarget, enchantid);
                            GossipHello_deathside_npc_enchanter(player, pItem, targets);
                        }
                        else if(itemTarget->GetProto()->InventoryType == INVTYPE_WEAPONOFFHAND)
                        {
                            AddEnchant(player, itemTarget, enchantid);
                            GossipHello_deathside_npc_enchanter(player, pItem, targets);
                        }
                        else
                        {
                            itemTarget = 0;
                            GossipHello_deathside_npc_enchanter(player, pItem, targets);
                        }
                    }
                    break;
                }
                else if (action >= 56 && action <= 62) // Chest Enchants.
                { 
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST);
                }
                else if (action >= 63 && action <= 71) // Boot Enchants.
                {
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET);
                }
                else if (action >= 72 && action <= 88) // Glove Enchants
                { 
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HANDS);
                }
                else if (action >= 89 && action <= 104) // Bracer Enchants
                {
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_WRISTS);
                }
                else if (action >= 105 && action <= 114) // Shield Enchants.
                { 
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (itemTarget && itemTarget->GetProto()->InventoryType != INVTYPE_SHIELD)
                        itemTarget = NULL;
                }
                else if (action >= 115 && action <= 119) // Ring Enchants.
                { 
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER1);
                    AddEnchant(player, itemTarget, enchantid);
    
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FINGER2);
                }
                else if (action >= 120 && action <= 137) // Cloak Enchants.
                { 
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK);
                }
                else if (action >= 138 && action <= 149) // Head Enchants.
                { 
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_HEAD);
                }
                else if (action >= 150 && action <= 154) // Leg Enchants.
                { 
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS);
                }
                else if (action >= 155 && action <= 166)
                { // Shoulder Enchants.
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_SHOULDERS);
                }
                else if (action >= 167 && action <= 169)
                { // Ranged Enchants.
                        
                    itemTarget = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
                    if (itemTarget && action <= 169 && (itemTarget->GetProto()->InventoryType != INVTYPE_RANGED && itemTarget->GetProto()->InventoryType != INVTYPE_RANGEDRIGHT))
                        {
                            GossipHello_deathside_npc_enchanter(player, pItem, targets);
                            break;
                        }
                }
    
                AddEnchant(player, itemTarget, enchantid);
                GossipHello_deathside_npc_enchanter(player, pItem, targets);
                break;
            }
        }
    }
    else
    {
        ChatHandler(player).SendSysMessage(LANG_SCRIPT_ERROR);
        player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

void AddSC_deathside_npc_enchanter()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="deathside_npc_enchanter";
    newscript->pItemUse = &GossipHello_deathside_npc_enchanter;
    newscript->pGossipSelectItem = &GossipSelect_deathside_npc_enchanter;
    newscript->RegisterSelf();
};