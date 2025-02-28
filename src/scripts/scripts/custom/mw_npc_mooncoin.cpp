#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"

//#EXCHANGE
//if (i == 0)
//{           
//    sprintf(chr, player->GetSession()->GetHellgroundString(LANG_HONOR_EXCHANGER_AP), items[i].icon, items[i].cost, MC_ICON, items[i].item);
//}
//else if (i == 1)
//{
//    const char* honor_icon = (player->GetTeam() == 0) ? "Interface\\PVPFrame\\PVP-Currency-Alliance" : "Interface\\PVPFrame\\PVP-Currency-Horde";
//    sprintf(chr, player->GetSession()->GetHellgroundString(LANG_HONOR_EXCHANGER_MARKS), honor_icon, items[i].cost, MC_ICON, items[i].item);
//}
//else 

#define GOSSIP_RESERVED_START 1006
// 1000 + 2 actions + 3 buy items

struct ItemInfo {
    uint32 id;
    const char* icon;
    uint32 mc_to_give;
    uint32 take_count;
};

// select a.entry,b.iconname from item_template a, aowow_icons b where a.entry between 693021 and 693026 and a.displayid=b.id;
// select iconname from aowow_icons where id=47817; (MODEL ID)

#define MC_ICON "Interface\\icons\\INV_Misc_Coin_03"

ItemInfo buy[] =
{
    // icon, mc_to_give, need_mc
    { 0, "Interface\\icons\\INV_Bijou_Gold", 3, 1 }, // EOT
    { 1, "Interface\\PVPFrame\\PVP-ArenaPoints-Icon", 1, 350 }, // AP
    { 2, "Interface\\icons\\INV_Misc_Rune_07", 1, 2000 }, // HONOR
    { 3, "Interface\\icons\\Spell_Holy_ChampionsBond", 1, 40 }, // BOJ
};

//ItemInfo sell[] =
//{
//    // tail -10000 /server/core3/logs/buy.log | awk '{print $8}' | sort | uniq -c | sort -nr
//    // 1 MC = 1 COIN
//    
//    // icon, item, cost
//    { "Interface\\icons\\INV_Misc_Key_04", LEGENDARY_KEY, 21 },
//    { "Interface\\icons\\INV_Misc_Key_03", ANTIQUE_KEY, 11 },
//    { "Interface\\icons\\INV_Misc_Key_04", EPIC_KEY, 10 },
//    { "Interface\\icons\\INV_Misc_OrnateBox", 0, 5 }, // Random chest
//    { "Interface\\icons\\INV_Misc_Bag_03", 19932, 11 }, // Battery Bag
//    { "Interface\\icons\\INV_Scroll_08", BUFF_SCROLL, 6 },
//    { "Interface\\icons\\INV_Misc_Food_91", 23086, 8 }, // Fruitcake x20
//    // DEPRECATED
//    //{ "Interface\\icons\\INV_Scroll_10", 693004, 7000 },
//    //{ "Interface\\icons\\INV_Potion_11", 693026, 300 },
//    //{ "Interface\\icons\\INV_Misc_OrnateBox", 29887, 125000 },
//};

//const uint32 sell_count = sizeof(sell) / sizeof(*sell);
const uint32 buy_count = sizeof(buy) / sizeof(*buy);

bool HonorExchanger_Hello(Player* player, Creature* creature)
{
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15590), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15591), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->SEND_GOSSIP_MENU(990108, creature->GetGUID());
    return true;
}

bool HonorExchanger_Gossip(Player *player, Creature *creature, uint32 uiSender, uint32 uiAction)
{     
    // exchange
    if (uiAction == 500)
    {
        HonorExchanger_Hello(player, creature);
        return true;
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->GetSession()->SendListInventory(creature->GetGUID());
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
    // buy
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 2)
    {
        char chr[256] = "";

        for (uint32 i = 0; i < buy_count; i++)
        {
            if (i == 0) // eot
            {
                sprintf(chr, player->GetSession()->GetHellgroundString(16634), buy[i].icon, buy[i].take_count, MC_ICON, buy[i].mc_to_give);
            }
            else if (i == 1) // ap
            {
                sprintf(chr, player->GetSession()->GetHellgroundString(15587), buy[i].icon, buy[i].take_count, MC_ICON, buy[i].mc_to_give);
            }
            else if (i == 2) // honor
            {
                sprintf(chr, player->GetSession()->GetHellgroundString(15588), buy[i].icon, buy[i].take_count, MC_ICON, buy[i].mc_to_give);
            }
            else if (i == 3) // boj
            {
                sprintf(chr, player->GetSession()->GetHellgroundString(15589), buy[i].icon, buy[i].take_count, MC_ICON, buy[i].mc_to_give);
            }
            else
            {
                player->CLOSE_GOSSIP_MENU();
                return false;
            }

            player->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3 + i);
        }

        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 500);
        player->SEND_GOSSIP_MENU(990108, creature->GetGUID());
    }
    // ap, honor, BOJ
    else if (uiAction >= GOSSIP_ACTION_INFO_DEF + 3 && uiAction < GOSSIP_ACTION_INFO_DEF + 3 + buy_count)
    {
        uint32 i = uiAction - GOSSIP_ACTION_INFO_DEF - 3;
        
        if (!player->CanStoreItemCount(MOON_COIN, buy[i].mc_to_give))
            return false;
      
        if (i == 0)
        {
            if (!player->GiveItem(MOON_COIN, buy[i].mc_to_give, ITEM_EMBLEM_OF_TRIUMPH, buy[i].take_count))
            {
                HonorExchanger_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 2);
                return false;
            }
        }
        else if (i == 1)
        {
            if (int32(player->GetArenaPoints() - buy[i].take_count) < 0)
            {
                player->SendEquipError(EQUIP_ERR_NOT_ENOUGH_ARENA_POINTS, NULL, NULL);
                HonorExchanger_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 2);
                return false;
            }

            player->ModifyArenaPoints(-int32(buy[i].take_count));
            player->GiveItem(MOON_COIN, buy[i].mc_to_give); //item is count
        }
        else if (i == 2)
        {
            if (int32(player->GetHonorPoints() - buy[i].take_count) < 0)
            {
                player->SendEquipError(EQUIP_ERR_NOT_ENOUGH_HONOR_POINTS, NULL, NULL);
                HonorExchanger_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 2);
                return false;
            }

            player->ModifyHonorPoints(-int32(buy[i].take_count));
            player->GiveItem(MOON_COIN, buy[i].mc_to_give); //item is count
        }
        else if (i == 3)
        {
            if (!player->GiveItem(MOON_COIN, buy[i].mc_to_give, 29434, buy[i].take_count))
            {
                HonorExchanger_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 2);
                return false;
            }
        }

        // @!newbie_quest
        if (i == 0)
            player->KilledMonster(690708, 0, 693033);

        sLog.outLog(LOG_BUY, "HonorExchanger Buy: Player name %s action %u (item %u, cost %u)", player->GetName(), i, buy[i].mc_to_give, buy[i].take_count);

        HonorExchanger_Gossip(player, creature, uiSender, GOSSIP_ACTION_INFO_DEF + 2);
        return true;
    }

    return true;
}

void AddSC_HonorExchanger()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "HonorExchanger";
    newscript->pGossipHello = &HonorExchanger_Hello;
    newscript->pGossipSelect = &HonorExchanger_Gossip;
    newscript->RegisterSelf();
}
