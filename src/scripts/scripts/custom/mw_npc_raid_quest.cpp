#include "precompiled.h"
#include "Language.h"
#include "World.h"
#include "ObjectMgr.h"

uint32 mc_reward_count = 70;

bool rq_Hello(Player* player, Creature* creature)
{
    for (const auto& bst : boss_souls_template)
    {
        ItemPrototype const* pProto = getObjectMgr()->GetItemPrototype(bst.item_soul);
        if (!pProto)
        {
            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_ERROR), player->GetGUID());
            player->CLOSE_GOSSIP_MENU();
            return false;
        }
        
        if ((1 << (bst.id - 1)) & player->custom_data.souls_quests_done)
            continue;

        // Soul of Kel'Thusad 0/1 < RED
        // Soul of Kel'Thusad 1/1 < GREEN

        bool has = player->HasItemCount(bst.item_soul, 1) ? true : false;

        const char* color = has ? "|cff2dff0d" : "|cffff0d0d";

        char buffer[250];
        sprintf(buffer, "%s %s%u/1|r - |TInterface\\icons\\INV_Bijou_Gold:18|t x%u", player->GetLocalizedItemName(bst.item_soul).c_str(), color, player->HasItemCount(bst.item_soul, 1) ? 1 : 0, bst.reward_count);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, buffer, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + bst.item_soul);
    }

    player->SEND_GOSSIP_MENU(990125, creature->GetGUID());
    return true;
}

bool rq_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    uint32 item = uiAction - GOSSIP_ACTION_INFO_DEF;

    bool good_item = false;
    BossSouls bs;
    for (const auto& bst : boss_souls_template)
    {
        if (bst.item_soul == item && !((1 << (bst.id - 1)) & player->custom_data.souls_quests_done))
        {
            good_item = true;
            bs = bst;
            break;
        }
    }

    uint32 reward_count = bs.reward_count;

    uint32 bonus = player->CalculateBonus(reward_count);
    reward_count += bonus;

    // send ad message
    if (!bonus && urand(0,2) == 0)
        ChatHandler(player).SendSysMessage(15622);

    if (!good_item || !player->GiveItem(ITEM_EMBLEM_OF_TRIUMPH, reward_count, item, 1))
    {
        rq_Hello(player, creature);
        return true;
    }

    if (bonus)
        player->AddEvent(new PSendSysMessageEvent(*player, 16653, player->GetItemLink(ITEM_EMBLEM_OF_TRIUMPH).c_str(), bonus), 2000);

    player->custom_data.souls_quests_done |= (1 << (bs.id - 1));

    if (((1 << (boss_souls_template.size())) - 1) == player->custom_data.souls_quests_done)
        player->custom_data.souls_quests_done = 0;

    RealmDataDatabase.PExecute("INSERT INTO character_custom_data (guid,souls_quests_done) VALUES (%u,%u) ON DUPLICATE KEY UPDATE souls_quests_done = '%u'", player->GetGUIDLow(), player->custom_data.souls_quests_done, player->custom_data.souls_quests_done);
    
    player->KilledMonster(690716, 0, 693035);

    player->SaveToDB();

    rq_Hello(player, creature);
    return true;
}

void AddSC_mw_npc_raid_quest()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "mw_npc_raid_quest";
    newscript->pGossipHello = &rq_Hello;
    newscript->pGossipSelect = &rq_Gossip;
    newscript->RegisterSelf();
}