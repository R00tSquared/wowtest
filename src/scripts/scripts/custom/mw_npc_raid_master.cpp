#include "precompiled.h"
#include "Language.h"
#include "GameEvent.h"
#include "World.h"
#include "InstanceSaveMgr.h"
#include "ObjectMgr.h"

bool raid_master_Hello(Player* player, Creature* creature)
{  
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, player->GetSession()->GetHellgroundString(16666), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (time_t resetTime = sInstanceSaveManager.GetResetTimefor(MAP_SWP, false))
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->PGetHellgroundString(16631, player->GetSession()->secondsToTimeString(resetTime - time(NULL)).c_str()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    }
    if (time_t resetTime = sInstanceSaveManager.GetResetTimefor(MAP_SWP, true))
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->PGetHellgroundString(16748, player->GetSession()->secondsToTimeString(resetTime - time(NULL)).c_str()), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    }

    player->SEND_GOSSIP_MENU(990126, creature->GetGUID());
    player->KilledMonster(690714, 0, 693035);
    return true;
}

void SendAllRaidsInfo(Player* player, Creature* creature)
{
    for (const auto& pair : sWorld.creature_map_mod)
    {
        int mapId = pair.first;
        if (mapId > 0)
            continue;

        mapId = -mapId;

        MapEntry const* map = sMapStore.LookupEntry(mapId);
        if (!map)
            continue;

        std::string map_name = std::string(*map->name);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, map_name.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + mapId);
    }

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(12002), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    player->SEND_GOSSIP_MENU(990140, creature->GetGUID());
}

bool raid_master_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    uint32 action = uiAction - GOSSIP_ACTION_INFO_DEF;
    if (action <= 0)
    {
        raid_master_Hello(player, creature);
        return true;
    }

    switch (action)
    {
    case 1: // Raid instance information
        SendAllRaidsInfo(player, creature);
        return true;
    default:
        auto raid_info = sWorld.creature_map_mod.find(action);
        if (raid_info != sWorld.creature_map_mod.end())
        {
            MapEntry const* map = sMapStore.LookupEntry(action);
            if (!map)
                return true;

            std::string map_name = std::string(*map->name);
            ChatHandler(player).SendSysMessage(("\n|cfffcfcfc----- " + map_name + " -----|r").c_str());
            player->SendCustomRaidInfo(action, false);
            raid_master_Gossip(player, creature, 0, 1 + GOSSIP_ACTION_INFO_DEF);
            player->KilledMonster(690723, 0, 693035);
            return true;
        }
    }

    raid_master_Hello(player, creature);
    return true;
}

void AddSC_DeathSide_Npc_Raid_Master()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "mw_raid_master";
    newscript->pGossipHello = &raid_master_Hello;
    newscript->pGossipSelect = &raid_master_Gossip;
    newscript->RegisterSelf();
}