#include "precompiled.h"
#include "Language.h"
#include "Spell.h"

#define ITEM_ENTRY_LIMIT 50000

std::set<uint32> item_enchants = { 20748,20747,20745,22521,23123,20749,20746,20744,22522,20750,34539 };
std::set<uint32> slots = { EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND };

bool GossipHello_weapon(Player* player, uint32 option)
{
    bool weapon_found = false;
    for (auto& slot : slots)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (item && player->IsCustomLegendaryWeapon(item->GetEntry()))
        {
            weapon_found = true;
            break;
        }
    }

    if (!weapon_found)
    {
        ChatHandler(player).SendSysMessage(16720);
        return true;
    }

    bool item_enchant_found = false;
    for (auto& item_enchant : item_enchants)
    {
        if (item_enchant > ITEM_ENTRY_LIMIT)
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
        
        if (player->HasItemCount(item_enchant, 1))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetItemLink(item_enchant, false, ITEM_LINK_NO_COLOR), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + item_enchant);
            item_enchant_found = true;
        }
    }

    if (!item_enchant_found)
    {
        ChatHandler(player).SendSysMessage(16720);
        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    player->SEND_GOSSIP_MENU(990137, player->GetGUID());
    return true;
}

bool GossipSelect_weapon(Player* player, uint32 uiSender, uint32 action, uint32 option)
{
    uint32 act = action - GOSSIP_ACTION_INFO_DEF;
  
    // enchant selected, select slot now
    // 20748 - enchant
    // 2074801 - enchant main hand
    if (act < 50000)
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12885), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + act * 100 + 1);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12886), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + act * 100 + 2);
        player->SEND_GOSSIP_MENU(990137, player->GetGUID());
        return true;
    }
    // do enchant
    else
    {
        uint8 action_slot = act % 10;
        uint32 action_item = (act - action_slot) / 100;

        if (action_item >= ITEM_ENTRY_LIMIT || item_enchants.find(action_item) == item_enchants.end())
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        Item* weapon = player->GetItemByPos(INVENTORY_SLOT_BAG_0, action_slot == 1 ? EQUIPMENT_SLOT_MAINHAND : EQUIPMENT_SLOT_OFFHAND);
        if (!weapon || !player->IsCustomLegendaryWeapon(weapon->GetEntry()))
        {
            ChatHandler(player).SendSysMessage(16720);
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        Item* item;
        for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
        {
            item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (item && item->GetEntry() == action_item)
            {
                SpellCastTargets targets;
                targets.setItemTarget(weapon);

                WorldPacket packet;
                packet << uint8(255) << uint8(i) << uint8(0) << uint8(0) << item->GetGUID() << targets;
                player->GetSession()->HandleUseItemOpcode(packet);
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            if (Bag* pBag = (Bag*)player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    item = player->GetItemByPos(i, j);
                    if (item && item->GetEntry() == action_item)
                    {
                        SpellCastTargets targets;
                        targets.setItemTarget(weapon);

                        WorldPacket packet;
                        packet << uint8(i) << uint8(j) << uint8(0) << uint8(0) << item->GetGUID() << targets;
                        player->GetSession()->HandleUseItemOpcode(packet);
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    }
                }
            }
        }
    }

    player->CLOSE_GOSSIP_MENU();
    return true;
}

void AddSC_enchant_legendary()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "mw_player_enchant_legweapon";
    newscript->pGossipHelloPlayer = &GossipHello_weapon;
    newscript->pGossipSelectPlayer = &GossipSelect_weapon;
    newscript->RegisterSelf();
}