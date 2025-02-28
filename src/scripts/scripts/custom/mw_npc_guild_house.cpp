#include "precompiled.h"
#include "Language.h"
#include "World.h"
#include "GuildMgr.h"
#include "Guild.h"
#include "GameEvent.h"

#define PAGE 100000000
#define MAX_MEMBER_PAGE_INFO 12

bool Hello(Player* player, Creature* creature)
{
    //player->DestroyItemCount(pItem, count, true);

    uint8 show_max = 10;
    uint8 pos = 0;

    bool my_guild_found = false;
    uint32 my_guild = player->GetGuildId();
    uint32 my_guild_deposit = 0;

    // too bad to sort in each call...
    // TODO - MIGHT CAUSE HEAVY LOAD!
    if (!sWorld.guild_house_ranks.empty())
    {
        std::sort(sWorld.guild_house_ranks.begin(), sWorld.guild_house_ranks.end(), std::greater<std::pair<uint32, uint32>>());

        for (auto ghr : sWorld.guild_house_ranks)
        {
            if (ghr.second > PAGE)
            {
                ChatHandler(player).SendSysMessage("Error!");
                return false;
            }

            Guild *guild = getGuildMgr()->GetGuildById(ghr.second);

            if (guild)
            {
                ++pos;

                std::string gi;

                if (my_guild == ghr.second)
                {
                    gi = "|cff2522f2" + std::to_string(pos) + ". " + "<" + guild->GetName() + "> - " + std::to_string(ghr.first) + "|r";
                    my_guild_deposit = ghr.first;
                }
                else
                    gi = std::to_string(pos) + ". " + "<" + guild->GetName() + "> - " + std::to_string(ghr.first);

                player->ADD_GOSSIP_ITEM(0, gi, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + PAGE + ghr.second);
                --show_max;
            }
        }
    }

    std::string guild_house_owner = "-";

    if (sWorld.m_guild_house_owner)
    {
        if (Guild *go = getGuildMgr()->GetGuildById(sWorld.m_guild_house_owner))
            guild_house_owner = go->GetName();
    }

    char curr[256];
    sprintf(curr, player->GetSession()->GetHellgroundString(15515), guild_house_owner.c_str());
    player->ADD_GOSSIP_ITEM(8, curr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    uint32 event_id = EVENT_GUILD_HOUSE;
    std::string ends_in = "0";
    auto gmdm = getGameEventMgr()->mGameEvent;
    if (gmdm[event_id].end) // if exist
    {
        time_t currenttime = time(NULL);
        ends_in = GetTimeString(gmdm[event_id].occurence * MINUTE - ((currenttime - gmdm[event_id].start) % (gmdm[event_id].occurence * MINUTE)));
    }

    char end[256];
    sprintf(end, player->GetSession()->GetHellgroundString(15516), ends_in.c_str());
    player->ADD_GOSSIP_ITEM(8, end, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (my_guild)
    {
        char don[256];
        sprintf(don, player->GetSession()->GetHellgroundString(15517), my_guild_deposit);

        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_VENDOR, don, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2, player->GetSession()->GetHellgroundString(15518), 0, true);
    }     
    
    player->SEND_GOSSIP_MENU(990116, creature->GetGUID());

    return true;
}

bool Gossip(Player *player, Creature *creature, uint32 uiSender, uint32 uiAction)
{   
    ChatHandler(player).SendSysMessage("Unavailable");
    return false;
    
    uint32 action = uiAction - GOSSIP_ACTION_INFO_DEF;

    if (action > PAGE)
    {
        uint32 start_page = floor(action) / PAGE;
        uint32 guild = action - PAGE * start_page;

        if (!getGuildMgr()->GetGuildById(guild))
            return false;

        auto ghi = sWorld.guild_house_info;

        typedef std::multimap<uint32, GuildHouseDonators>::iterator MMAPIterator;
        std::pair<MMAPIterator, MMAPIterator> result = sWorld.guild_house_info.equal_range(guild);

        uint32 total_items = std::distance(result.first, result.second);

        if (!total_items)
            return false;

        uint32 current_page = 0;
        uint32 pos = 0;
        uint32 total_pages = 1;

        if (total_items > MAX_MEMBER_PAGE_INFO)
            total_pages = ceil(floor(total_items) / MAX_MEMBER_PAGE_INFO);

        if (start_page > total_pages)
            start_page = 1;

        // make it with multimap & distance
        for (MMAPIterator it = result.first; it != result.second; it++, ++pos)
        {
            if (pos % MAX_MEMBER_PAGE_INFO == 0)
                ++current_page;
            
            if (current_page < start_page)
                continue;

            // don't need to iterrate
            if (current_page > start_page)
                break;
            
            std::string di = it->second.date + " - " + it->second.char_name + " - " + std::to_string(it->second.amount);
            player->ADD_GOSSIP_ITEM(0, di, GOSSIP_SENDER_MAIN, uiAction);
        }

        if (total_pages > 1 && start_page != total_pages)
            player->ADD_GOSSIP_ITEM(4, player->GetSession()->GetHellgroundString(12001), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + (PAGE * start_page + PAGE) + guild);

        if (start_page > 1)
            player->ADD_GOSSIP_ITEM(4, player->GetSession()->GetHellgroundString(12002), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + (PAGE * start_page - PAGE) + guild);

        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(12003), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(990115, creature->GetGUID()); // TODO
    } 
    else if (action == 1)
    {
        Hello(player, creature);
    }

    return true;
}

bool GossipCoded(Player *player, Creature *creature, uint32 uiSender, uint32 uiAction, const char* sCode)
{
    ChatHandler(player).SendSysMessage("Unavailable");
    return false;
    
    uint32 action = uiAction - GOSSIP_ACTION_INFO_DEF;

    if (action == 2) //donate
    {
        uint32 guild = player->GetGuildId();

        if (!getGuildMgr()->GetGuildById(guild))
        {
            Hello(player, creature);
            return false;
        }       
        
        uint32 count = (uint32)atoi(sCode); 
        if (count == 0 || count > 100)
        {
            ChatHandler(player).SendSysMessage(15519);
            Hello(player, creature);
            return false;
        }

        if (!player->HasItemCount(MOON_COIN, count))
        {
            ChatHandler(player).SendSysMessage(15206);
            Hello(player, creature);
            return false;
        }
        
        // only 100 coins / hour per guild
        time_t currenttime = time(NULL);
        auto ghi = sWorld.guild_house_info;
        auto itr = ghi.find(guild);
        uint32 sum = 0;

        typedef std::multimap<uint32, GuildHouseDonators>::iterator MMAPIterator;
        std::pair<MMAPIterator, MMAPIterator> result = ghi.equal_range(guild);
        uint32 total_items = std::distance(result.first, result.second);

        if (total_items)
        {
            for (MMAPIterator it = result.first; it != result.second; it++)
            {
                if (StringToUnixtime(it->second.date) > currenttime - HOUR)
                    sum += it->second.amount;
            }

            if (sum >= 100)
            {
                ChatHandler(player).PSendSysMessage(15525);
                Hello(player, creature);
                return false;
            }

            if (sum + count > 100)
            {
                ChatHandler(player).PSendSysMessage(15526, 100 - sum);
                Hello(player, creature);
                return false;
            }
        }


        player->DestroyItemCount(MOON_COIN, count, true, false,"GUILD_HOUSE_DONATION");
        player->SaveToDB();

        GuildHouseDonators ghd;
        ghd.char_name = player->GetName();
        ghd.amount = count;
        ghd.date = GetTimestampStr();
        
        sWorld.UpdateGuildHouse(guild, ghd, true);

        ChatHandler(player).PSendSysMessage(15520, count);

        Hello(player, creature);
    }
  
    return true;
}

void AddSC_Guild_House()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_guildhouse";
    newscript->pGossipHello = &Hello;
    newscript->pGossipSelect = &Gossip;
    newscript->pGossipSelectWithCode = &GossipCoded;
    newscript->RegisterSelf();
}
