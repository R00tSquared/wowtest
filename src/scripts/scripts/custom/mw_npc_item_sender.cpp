#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"

const uint32 action_item = 2000;

struct ItemInfo {
    const char* icon;
    uint32 item;
};

ItemInfo items[] =
{
    //{ "Interface\\icons\\INV_Misc_Key_04", LEGENDARY_KEY },
    { "Interface\\icons\\INV_Bijou_Gold", ITEM_EMBLEM_OF_TRIUMPH },
    { "Interface\\icons\\INV_Misc_Coin_03", MOON_COIN },
    { "Interface\\icons\\INV_Misc_Key_04", EPIC_KEY },
    { "Interface\\icons\\INV_Misc_Key_03", ANTIQUE_KEY },
    { "Interface\\icons\\INV_Misc_Key_04", LEGENDARY_KEY },
	{ "Interface\\icons\\INV_Staff_01", FLAWLESS_LEGENDARY_KEY },
    { "Interface\\icons\\INV_Misc_Key_01", AZERITE_KEY },
    { "Interface\\icons\\INV_Misc_Key_02", GHOST_KEY },
    { "Interface\\icons\\INV_Scroll_08", BUFF_SCROLL },
    { "Interface\\icons\\Spell_Holy_ChampionsBond", ITEM_BADGE },
    { "Interface\\icons\\INV_Helmet_06", SECOND_BADGE }, // BOJ
};

const uint32 items_count = sizeof(items) / sizeof(*items);

bool IS_Hello(Player* player, Creature* creature)
{
    char chr[256] = "";
    std::string nameloc;

    char selectedname[256] = "";
    if (player->selectedCharacterGuid && !player->selectedCharacterName.empty())
        sprintf(selectedname, player->GetSession()->GetHellgroundString(15627), player->selectedCharacterName.c_str());
    else
        sprintf(selectedname, player->GetSession()->GetHellgroundString(15628));

    //player->ADD_GOSSIP_ITEM_EXTENDED(5, selectedname, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1, player->GetSession()->GetHellgroundString(15638), 0, true);
    player->ADD_GOSSIP_ITEM(5, selectedname, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (player->selectedCharacterGuid && !player->selectedCharacterName.empty())
    {
        for (uint32 i = 0; i < items_count; ++i)
        {
            uint32 item = items[i].item;
            std::string name = player->GetLocalizedItemName(item);

            ItemPrototype const* pProto = getObjectMgr()->GetItemPrototype(item);
            if (!pProto)
            {
                creature->Whisper(player->GetSession()->GetHellgroundString(LANG_ERROR), player->GetGUID());
                player->CLOSE_GOSSIP_MENU();
                return false;
            }

            // strange bug... don't touch
            std::string str = player->GetSession()->PGetHellgroundString(15629, items[i].icon, name.c_str());
            player->ADD_GOSSIP_ITEM_EXTENDED(2, str.c_str(), GOSSIP_SENDER_MAIN, action_item + item, player->GetSession()->PGetHellgroundString(15631, name.c_str(), pProto->Stackable), 0, true);
        }
    }

    player->SEND_GOSSIP_MENU(990124, creature->GetGUID());
    return true;
}

bool IS_Select(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    // character list
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT guid,name,race,class FROM characters WHERE account = '%u' AND guid != '%u'", player->GetSession()->GetAccountId(), player->GetGUIDLow());
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();

                uint32 guid = fields[0].GetUInt32();

                std::string str = std::string(fields[1].GetString()) + " (" + PlayerRaceAndClassToString(fields[2].GetUInt8(), fields[3].GetUInt8()) + ")";
                player->ADD_GOSSIP_ITEM(0, str.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000 + guid);
            } while (result->NextRow());
        }

        player->SEND_GOSSIP_MENU(990124, creature->GetGUID());
    }
    else if (uiAction >= GOSSIP_ACTION_INFO_DEF + 1000)
    {
        uint32 guid = uiAction - GOSSIP_ACTION_INFO_DEF - 1000;
        
        if (guid == player->GetGUIDLow())
        {
            ChatHandler(player).SendSysMessage(LANG_ERROR);
            IS_Hello(player, creature);
            return false;
        }
        
        QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT name FROM characters WHERE account = '%u' AND guid = '%u'", player->GetSession()->GetAccountId(), guid);
        if (!result)
        {
            ChatHandler(player).SendSysMessage(LANG_ERROR);
            IS_Hello(player, creature);
            return false;
        }

        std::string name = result->Fetch()[0].GetCppString();

        // fine!
        player->selectedCharacterGuid = guid;
        player->selectedCharacterName = name;
        ChatHandler(player).SendSysMessage(16676);
        IS_Hello(player, creature);
    }

    return false;
}

bool IS_GossipCoded(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction, const char* sCode)
{
    if (player->IsPlayerCustomFlagged(PL_CUSTOM_NEWBIE_QUEST_LOCK))
    {
        ChatHandler(player).PSendSysMessage(16700);
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
    
    uint32 current_gs = player->GetGearScore();
    uint32 needed_gs = GS_GOOD;
    if (current_gs < needed_gs)
    {
        ChatHandler(player).PSendSysMessage(15136, current_gs, needed_gs);
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
    
    if (uiAction > action_item)
    {
        if (!player->selectedCharacterGuid || player->selectedCharacterName.empty())
        {
            // should never gets here
            ChatHandler(player).SendSysMessage(15635);
            IS_Hello(player, creature);
            return false;
        }

        uint32 item = uiAction - action_item;
        uint32 count = (uint32)atoi(sCode);

        ItemPrototype const* pProto = getObjectMgr()->GetItemPrototype(item);
        if (!pProto)
        {
            ChatHandler(player).SendSysMessage("ERROR #2");
            player->CLOSE_GOSSIP_MENU();
            return false;
        }

        // check if item can be transferred by this NPC
        bool can_transfer = false;

        // check other items
        for (uint32 i = 0; i < items_count; ++i)
        {
            if (item == items[i].item)
            {
                can_transfer = true;
                break;
            }
        }

        if (!can_transfer)
        {
            ChatHandler(player).SendSysMessage("ERROR #5");
            IS_Hello(player, creature);
            return false;
        }

        uint32 stackable = pProto->Stackable;
        if (count == 0 || count > stackable)
        {
            ChatHandler(player).SendSysMessage(player->GetSession()->GetHellgroundString(15636));
            IS_Hello(player, creature);
            return false;
        }

        if (!player->HasItemCount(item, count, false))
        {
            ChatHandler(player).SendSysMessage(player->GetSession()->GetHellgroundString(15630));
            IS_Hello(player, creature);
            return false;
        }

        player->DestroyItemCount(item, count, true, false, "SCRIPT_ITEM_SENDER");
        player->SaveToDB();

        std::string subject = player->GetName();

        RealmDataDatabase.PExecute("INSERT INTO mail_external (`id`, `receiver`, `subject`, `message`, `item`, `item_count`) VALUES (NULL, %u, '%s', '', %u, %u)",
            player->selectedCharacterGuid, subject.c_str(), item, count);

        sLog.outLog(LOG_TRADE, "Player %s (Account: %u) transferred item: %s (Entry: %u Count: %u) to player: %s (GUID: %u)",
            player->GetName(), player->GetSession()->GetAccountId(), pProto->Name1, item, count, player->selectedCharacterName.c_str(), player->selectedCharacterGuid);

        ChatHandler(player).PSendSysMessage(15637, player->selectedCharacterName.c_str());
    }

    IS_Hello(player, creature);
    return false;
}

void AddSC_Npc_Item_Sender()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "ItemSender";
    newscript->pGossipHello = &IS_Hello;
    newscript->pGossipSelect = &IS_Select;
    newscript->pGossipSelectWithCode = &IS_GossipCoded;
    newscript->RegisterSelf();
}
