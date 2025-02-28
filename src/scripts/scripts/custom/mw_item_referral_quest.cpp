// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

bool MW_referral_quest_item(Player* Plr, Item* pItem, SpellCastTargets const& targets)
{
    Plr->PlayerTalkClass->ClearMenus();

    Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(16603), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +1);
    Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(16602), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +2);
    Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(16604), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +3);
    Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(16605), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF +4);
    Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pItem->GetGUID());
    return true;
}

bool MW_referral_quest_item_Gossip(Player* pPlayer, Item* pItem, uint32 uiSender, uint32 uiAction, SpellCastTargets const& targets)
{
    // item, chance
    std::multimap<uint32, std::pair<uint32, uint32>> items;

    switch (uiAction)
    {
    // spd
    case GOSSIP_ACTION_INFO_DEF + 1:
    {
        items.insert({ 1, { 22861, 10 } });
        items.insert({ 1, { 22866, 10 } });
        items.insert({ 1, { 13512, 15 } });
        items.insert({ 1, { 22861, 25 } });
        items.insert({ 1, { 28103, 25 } });

        items.insert({ 2, { 22832, 25 } });
        items.insert({ 2, { 22839, 25 } });
        items.insert({ 2, { 12662, 25 } });
        items.insert({ 2, { 22788, 25 } });

        items.insert({ 3, { 27657, 100 } });

        items.insert({ 4, { 20749, 10 } });
    }
    break;
    // melee
    case GOSSIP_ACTION_INFO_DEF + 2:
        items.insert({ 1, { 22854, 10 } });
        items.insert({ 1, { 22831, 15 } });
        items.insert({ 1, { 22840, 15 } });

        items.insert({ 2, { 22838, 15 } });
        items.insert({ 2, { 7676 , 15 } });
        items.insert({ 2, { 27498, 15 } });
        items.insert({ 2, { 27503, 15 } });

        items.insert({ 3, { 27664, 50 } });
        items.insert({ 3, { 27658, 50 } });

        items.insert({ 4, { 23529, 10 } });
        items.insert({ 4, { 28421, 10 } });
        break;
    // healers
    case GOSSIP_ACTION_INFO_DEF + 3:
        items.insert({ 1, { 32067, 15 } });
        items.insert({ 1, { 22825, 15 } });
        items.insert({ 1, { 22840, 15 } });
        items.insert({ 1, { 22853, 25 } });
        items.insert({ 1, { 13511, 15 } });

        items.insert({ 2, { 22832, 25 } });
        items.insert({ 2, { 12662, 25 } });

        items.insert({ 3, { 27666, 100 } });

        items.insert({ 4, { 20748, 10 } });
        break;
    // tanks
    case GOSSIP_ACTION_INFO_DEF + 4:
        items.insert({ 1, { 22851, 10 } });
        items.insert({ 1, { 22831, 15 } });
        items.insert({ 1, { 32062, 15 } });
        items.insert({ 1, { 32068, 15 } });

        items.insert({ 2, { 22849, 25 } });
        items.insert({ 2, { 27500, 15 } });
        items.insert({ 2, { 27498, 15 } });
        items.insert({ 2, { 27503, 15 } });
        items.insert({ 2, { 22797, 20 } });

        items.insert({ 3, { 33052, 100 } });

        items.insert({ 4, { 23529, 10 } });
        items.insert({ 4, { 28421, 10 } });
        items.insert({ 4, { 22522, 10 } });
        break;
    }

    for (uint8 x = 0; x <= 1; ++x)
    {
        uint32 rand = urand(0, 99);
        for (uint8 i = 1; i < 5; ++i)
        {
            auto range = items.equal_range(i);
            uint32 chance = 0;
            for (auto it = range.first; it != range.second; ++it) {
                chance += it->second.second;

                if (rand < chance)
                {
                    pPlayer->GiveItem(it->second.first, 1);
                    break;
                }
            }
        }
    }

    pPlayer->PlayerTalkClass->ClearMenus();
    pPlayer->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true, "REFERRAL_DESTROY");
    return true;
}

 void AddSC_MW_referral_quest_item()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "mw_supply";
     newscript->pItemUse           = &MW_referral_quest_item;
     newscript->pGossipSelectItem  = &MW_referral_quest_item_Gossip;
     newscript->RegisterSelf();
 }