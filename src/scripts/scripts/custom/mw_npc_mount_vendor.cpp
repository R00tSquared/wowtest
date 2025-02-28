#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"


// several NPCs standing in Shattrath
// each one has this script
// depending on entry use different templates
// click -> "I want to buy [Super Mount]", then confirm

const uint32 VendorsAndMounts[15][3] =
{
    // NPC entry, gossip ID, mount item entry
    { 693034, 990141, LEOPARD },
    { 693035, 990142, UNICORN },
    { 693033, 990143, PINK_ELEKK },

    { 693052, 990040, BRONZE_DRAGON },
    { 693053, 990041, SMOKY_PANTHER },
    { 693054, 990042, BONE_GRYPHON },
    { 693055, 990043, SNOW_BEAR },
    { 693056, 990044, GLAD_DRAKE },
    { 693057, 990045, MERC_DRAKE },
    { 693058, 990046, VENG_DRAKE },
    { 693059, 990047, ROTTEN_BEAR },
    { 693060, 990048, RED_QIRAJI },
    { 693061, 990049, TRANSMOGRIFICATOR },
    { 693097, 990050, DRAGONHAWK },
    { 693123, 990120, FANTASTIC_TURTLE },
};

bool MountVendor_Hello(Player* player, Creature* creature)
{
    bool found;
    uint32 text;

    for (const uint32 *val : VendorsAndMounts)
    {
        if (creature->GetEntry() == val[0])
        {
            if (ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(val[2]))
            { 
                std::string itemNameLocale;
                char ItemName[256];
                sprintf(ItemName, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ITEM_RESTRICTED), player->GetItemNameLocale(pProto, &itemNameLocale));
                
                char chr[256];
                sprintf(chr, player->GetSession()->GetHellgroundString(LANG_MOUNT_VENDOR), itemNameLocale.c_str());
                
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_MONEY_BAG, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + val[2], player->GetSession()->GetHellgroundString(LANG_MOUNT_VENDOR_CONFIRM), 0, false);
                
                text = val[1];
                
                found = true;
                break;
            }
        }
    }

    if (!found)
        return false;

    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_NO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->SEND_GOSSIP_MENU(text, creature->GetGUID());
    return true;
}

bool MountVendor_Gossip(Player *player, Creature *creature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction > GOSSIP_ACTION_INFO_DEF + 1)
    {
        uint32 item = uiAction - GOSSIP_ACTION_INFO_DEF;

        for (const uint32 *val : VendorsAndMounts)
        {
            if (creature->GetEntry() == val[0] && item == val[2])
            {
                switch (item)
                {
                    case TRANSMOGRIFICATOR:
                    {
                        uint32 wins = 0;
                        //uint32 rwins = 0;

                        // oh direct select is bad... ;(
                        QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT bg_wins FROM characters_bg WHERE guid = %u", player->GetGUIDLow());
                        if (result)
                        {
                            wins = (*result)[0].GetUInt32();
                            //rwins = (*result)[1].GetUInt32();
                        }

                        if ((player->GetArenaPersonalRating(1)/*1 == 3x3*/ >= 2200 || player->GetArenaPersonalRating(0)/*0 == 2x2*/ >= 2200) && player->GetArenaPoints() >= 5000)
                        {
                            if (player->GiveItem(item, 1))
                                player->ModifyArenaPoints(-5000);
                        }
                        else if (wins >= 700 && player->GetHonorPoints() >= 75000)
                        {                           
                            if (player->GiveItem(item, 1))
                                player->ModifyHonorPoints(-75000);
                        }
                        else
                        {
                            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_CONDITIONS_NOT_MET), player->GetGUID());
                        }

                        break;
                    } 
                    case DRAGONHAWK:
                    {
                        if (player->GetSession()->isPremium())
                        {
                            player->GiveItem(item, 1);
                        }
                        else
                        {
                            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_CONDITIONS_NOT_MET), player->GetGUID());
                        }

                        break;
                    }
                    case LEOPARD:
                    case UNICORN:
                    case PINK_ELEKK:
                    {
                        player->GiveItem(item, 1, ELIXIR_OF_HARMONY, 1); // obtained from vote chest
                        break;
                    }
                    case BRONZE_DRAGON:
                    {
                        if (player->GetTotalKills() >= 50000 && player->GetHonorPoints() >= 75000)
                        {
                            if (player->GiveItem(item, 1))
                                player->ModifyHonorPoints(-75000);
                        }
                        else
                        {
                            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_CONDITIONS_NOT_MET), player->GetGUID());                      
                        }

                        break;                        
                    }
                    case ROTTEN_BEAR:
                    case RED_QIRAJI:
                    {
						// disabled
						if (item == ROTTEN_BEAR)
						{
							creature->Whisper("DISABLED", player->GetGUID());
							break;
						}
						
						uint32 rating_req = (item == ROTTEN_BEAR) ? 2200 : 2400;
                        
                        if ((player->GetArenaPersonalRating(1)/*1 == 3x3*/ >= rating_req || player->GetArenaPersonalRating(0)/*0 == 2x2*/ >= rating_req) && player->GetArenaPoints() >= 5000)
                        {                        
                            if (player->GiveItem(item, 1))
                                player->ModifyArenaPoints(-5000);
                        }
                        else
                        {
                            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_CONDITIONS_NOT_MET), player->GetGUID());
                        }

                        break;
                    }
                    case BONE_GRYPHON:
                    {
                        player->GiveItem(item, 1, BONE_GRYPHON_SCALE, 1); // reward for 5 active players invited
                        break;
                    }
                    case SNOW_BEAR:
                    {
                        if (!player->HasItemCount(29434, 1000) || !player->HasItemCount(23793,65))
                            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_CONDITIONS_NOT_MET), player->GetGUID());
                        else
                        {
                            player->DestroyItemCount(29434, 1000, true, false, "BUY_CUSTOM_MOUNT");
                            player->DestroyItemCount(23793, 65, true, false, "BUY_CUSTOM_MOUNT");
                            player->GiveItem(item, 1); // just for farming
                        }
 
                        break;
                    }
                    case SMOKY_PANTHER:
                    {
                        if (player->GetTotalPlayedTime() >= DAY * 15)
                        {
                            player->GiveItem(item, 1);
                        }
                        else
                        {
                            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_CONDITIONS_NOT_MET), player->GetGUID());
                        }

                        break;
                    }
                    // glad drakes
                    case GLAD_DRAKE:
                    {
                        player->GiveItem(item, 1, 7986, 1);
                        break;
                    }
                    case MERC_DRAKE:
                    {
                        player->GiveItem(item, 1, 7987, 1);
                        break;
                    }
                    case VENG_DRAKE:
                    {
                        player->GiveItem(item, 1, 7988, 1);
                        break;
                    }
					case FANTASTIC_TURTLE:
					{
						player->GiveItem(item, 1, 8166, 1);
						break;
					}
                }

                break;
            }
        }
    }

    player->CLOSE_GOSSIP_MENU();
    return true;
}

void AddSC_MountVendor()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "MountVendor";
    newscript->pGossipHello = &MountVendor_Hello;
    newscript->pGossipSelect = &MountVendor_Gossip;
    newscript->RegisterSelf();
}