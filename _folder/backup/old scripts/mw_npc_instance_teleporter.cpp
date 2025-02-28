// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Map.h"

//bool close(Player* player, Creature* creature, uint32 string)
//{
//    creature->Whisper(player->GetSession()->GetHellgroundString(string), player->GetGUID());
//    Instance_Teleporter_Hello(player, creature);
//    return false;
//}

//580 15456 The Sunwell
//564 15457 Black Temple
//534 15458 Hyjal
//533 15459 Naxxramas
//532 15460 Karazhan
//544 15461 Magtheridon
//565 15462 Gruul's Lair
//548 15463 Serpentshrine Cavern
//550 15464 Tempest Keep
//568 15465 Zul'Aman
//585 15466 Magister's Terrace
//560 15480 Old Hillsbrad Foothills
//269 15481 The Black Morass
//540 15467 Hellfire Citadel : The Shattered Halls
//542 15468 Hellfire Citadel : The Blood Furnace
//543 15469 Hellfire Citadel : Ramparts
//545 15470 Coilfang : The Steamvault
//546 15471 Coilfang : The Underbog
//547 15472 Coilfang : The Slave Pens
//552 15473 Tempest Keep : The Arcatraz
//553 15474 Tempest Keep : The Botanica
//554 15475 Tempest Keep : The Mechanar
//555 15476 Auchindoun : Shadow Labyrinth
//556 15477 Auchindoun : Sethekk Halls
//557 15478 Auchindoun : Mana - Tombs
//558 15479 Auchindoun : Auchenai Crypts

const uint32 action_list = 2000;
const uint32 action_inside = 3000;
const uint32 action_normal = 4000;
const uint32 action_heroic = 5000;
const uint32 action_tp = 6000;

// 15 gossip items max!
void SendGossip(Player* player, Creature* creature, uint32 action);

bool Instance_Teleporter_Hello(Player* player, Creature* creature)
{
    player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(15561), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(15562), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_NO), GOSSIP_SENDER_MAIN, 0);
    player->SEND_GOSSIP_MENU(990101, creature->GetGUID());
    return true;
}

bool Instance_Teleporter_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    if (player->isInCombat())
    {
        player->CLOSE_GOSSIP_MENU();
        player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_IN_COMBAT));
        return true;
    }

    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        SendGossip(player, creature, action_list + 580);
        SendGossip(player, creature, action_list + 564);
        SendGossip(player, creature, action_list + 534);
        SendGossip(player, creature, action_list + 544);
        SendGossip(player, creature, action_list + 565);
        SendGossip(player, creature, action_list + 532);
        SendGossip(player, creature, action_list + 548);
        SendGossip(player, creature, action_list + 550);
        SendGossip(player, creature, action_list + 568);
        SendGossip(player, creature, action_list + 533);
        SendGossip(player, creature, action_list + 249);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 999);
        player->SEND_GOSSIP_MENU(990102, creature->GetGUID());
        return true;
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 2)
    {
        SendGossip(player, creature, action_list + 585);
        SendGossip(player, creature, action_list + 269);
        SendGossip(player, creature, action_list + 540);
        SendGossip(player, creature, action_list + 542);
        SendGossip(player, creature, action_list + 543);
        SendGossip(player, creature, action_list + 545);
        SendGossip(player, creature, action_list + 546);
        SendGossip(player, creature, action_list + 547);
        SendGossip(player, creature, action_list + 552);
        SendGossip(player, creature, action_list + 553);
        SendGossip(player, creature, action_list + 554);
        SendGossip(player, creature, action_list + 555);
        SendGossip(player, creature, action_list + 560);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 998);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 999);
        player->SEND_GOSSIP_MENU(990102, creature->GetGUID());
        return true;
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 3)
    {
        player->TeleportTo(0, -13337.67, 70.24, 23.09, 0);
        return true;
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 4)
    {
        player->TeleportTo(0, -4213.81, -3335.68, 232.05, 0);
        return true;
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 998)
    {
        SendGossip(player, creature, action_list + 556);
        SendGossip(player, creature, action_list + 557);
        SendGossip(player, creature, action_list + 558);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 999);
        player->SEND_GOSSIP_MENU(990102, creature->GetGUID());
        return true;
    }
    else if (uiAction == GOSSIP_ACTION_INFO_DEF + 999)
    {
        Instance_Teleporter_Hello(player, creature);
        return true;
    }
    else if (uiAction > action_list && uiAction < action_tp + 1000)
    {
        uint8 type = floor(uiAction / 1000);

        SendGossip(player, creature, uiAction);

        if (type == 2 || type == 3)
            player->SEND_GOSSIP_MENU(990102, creature->GetGUID());

        return true;
    }

    player->CLOSE_GOSSIP_MENU();
    return true;
}

std::string BoJMinMax(uint32 min, uint32 max)
{
    return (min == max) ? std::to_string(min) : std::to_string(min) + '-' + std::to_string(max);
}

void SendGossip(Player* player, Creature* creature, uint32 action)
{
    //instanceteleporterchange
	
	// boj drop
    // select a.entry,b.name,a.mincountorref,a.maxcount from creature_loot_template a left join creature_template b on a.entry=b.entry where a.item=29434 order by mincountorref;

    // arr[map] = {string, gossip, normal_max, normal_dmg, normal_hp, heroic_max, heroic_dmg, heroic_hp, normal_boj_min, normal_boj_max, heroic_boj_min, heroic_boj_max, tele_coord}
    std::map <uint32, std::vector<double>> arr;
	arr[580] = { 15456, 1, 15, 20, 25, 25, 100, 100, 12, 18, 60, 90, 530, 12574.09, -6774.81, 15.09 };   //The Sunwell                          
	arr[564] = { 15457, 2, 15, 17, 20, 25, 100, 100, 12, 18, 36, 54, 530, -3649.91, 317.46, 35.28 };     //Black Temple                         
	arr[534] = { 15458, 4, 15, 17, 20, 25, 100, 100, 12, 18, 36, 36, 1, -8177.89, -4181.22, -167.55 };   //Hyjal                                
	arr[533] = { 15459, 10, 0, 0, 0, 15, 150, 100, 0, 0, 16, 32, 0, 3101.64, -3704.07, 131.41 };       //Naxxramas                            
	arr[532] = { 15460, 9, 10, 50, 40, 0, 0, 0, 12,18,0,0, 0, -11118.90, -2010.32, 47.08 };         //Karazhan                             
	arr[544] = { 15461, 8, 15, 20, 10, 15, 120, 90, 18, 18, 48, 48, 530, -316.32, 3094.59, -116.43 };        //Magtheridon's Lair                   
	arr[565] = { 15462, 7, 15, 20, 25, 15, 120, 120, 12,18,18,48, 530, 3536.84, 5098.66, 3.99 };           //Gruul's Lair      
	arr[249] = { 15643, 37, 0, 0, 0, 15, 260, 410, 0, 0, 48, 48, 1, -4692.15, -3716.12, 49.24 };           //Onyxia
	arr[548] = { 15463, 6, 15, 15, 17, 0, 0, 0, 12,18,0,0, 530, 820.02, 6864.93, -66.75 };          //Serpentshrine Cavern                 
	arr[550] = { 15464, 5, 15, 15, 17, 0, 0, 0, 12,18,0,0, 530, 3099.36, 1518.72, 190.30 };         //Tempest Keep                         
	arr[568] = { 15465, 3, 10, 50, 50, 0, 0, 0, 12,18,0,0, 530, 6851.77, -7972.56, 179.24 };        //Zul'Aman                             
	arr[585] = { 15466, 21, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, 12884.59, -7317.68, 65.50 };        //Magister's Terrace                   
	arr[540] = { 15467, 24, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, -305.79, 3061.62, -2.53 };          //Hellfire Citadel: The Shattered Halls
	arr[542] = { 15468, 25, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, -291.32, 3149.10, 31.55 };          //Hellfire Citadel: The Blood Furnace  
	arr[543] = { 15469, 26, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, -360.67, 3071.89, -15.09 };         //Hellfire Citadel: Ramparts           
	arr[545] = { 15470, 27, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, 794.53, 6927.81, -80.47 };          //Coilfang: The Steamvault             
	arr[546] = { 15471, 28, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, 779.39, 6758.12, -72.53 };          //Coilfang: The Underbog               
	arr[547] = { 15472, 29, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, 717.28, 6979.87, -73.02 };          //Coilfang: The Slave Pens             
	arr[552] = { 15473, 34, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, 3308.91, 1340.71, 505.55 };         //Tempest Keep: The Arcatraz           
	arr[553] = { 15474, 35, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, 3407.11, 1488.47, 182.83 };         //Tempest Keep: The Botanica           
	arr[554] = { 15475, 36, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, 2867.12, 1549.42, 252.15 };         //Tempest Keep: The Mechanar           
	arr[555] = { 15476, 30, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, -3627.89, 4941.97, -101.04 };       //Auchindoun: Shadow Labyrinth         
	arr[556] = { 15477, 31, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, -3362.19, 4664.12, -101.04 };       //Auchindoun: Sethekk Halls            
	arr[557] = { 15478, 32, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, -3104.17, 4945.52, -101.50 };       //Auchindoun: Mana-Tombs               
	arr[558] = { 15479, 33, 0, 0, 0, 5, 10, 50, 0,0,6,6, 530, -3362.04, 5209.85, -101.05 };       //Auchindoun: Auchenai Crypts          
	arr[560] = { 15480, 22, 0, 0, 0, 5, 10, 50, 0,0,6,6, 1, -8404.29, -4070.62, -208.58 };        //Old Hillsbrad Foothills              
	arr[269] = { 15481, 23, 0, 0, 0, 5, 10, 50, 0,0,6,6, 1, -8761.95, -4185.10, -209.49 };        //The Black Morass                       

    // strings reserved until 15620
    std::map <uint32, std::pair<uint32, std::vector<uint32>>> messages;
    // 15596 | Bosses drop only Corrupted Badges of Justice |
    messages[15596] = { 1, { 585, 540, 542, 543, 545, 546, 547, 552, 553, 554, 555, 556, 557, 558, 560, 269 } };
    // 15597 | Bosses drop one random item for transfogrification |
    messages[15597] = { 1, { 580, 564, 534 } };
    // 15598 | Get a Ghost Key for defeating last boss (need quest) |
    messages[15598] = { 1, { 533 } };
    // 15599 | Item drop chance is higher than in normal mode |
    messages[15599] = { 1, { 580, 564, 534, 533 } };
    // 15600 | Drop rates from trash reduced to x1 rates and only one random item drops from bosses. |
    messages[15600] = { 0, { 580 } };
    // 15601 | Drop rates from trash reduced to x1 rates and only one random item drops from final boss. |
    messages[15601] = { 0, { 564, 534 } };
    // 15602 | Azerite Key for defeating last boss (need quest)
    messages[15602] = { 1, { 544, 565, 249 } };

    // this is pretty smart ;)
    auto printStrings = [&](uint32 map, uint32 heroic)
    {
        for (auto val : messages)
        {
            if (val.second.first == heroic)
            {
                for (auto val_map : val.second.second)
                {
                    if (val_map == map)
                        ChatHandler(player->GetSession()).PSendSysMessage(val.first);
                }
            }
        }
    };

    uint32 map = action % 1000;
    uint8 type = floor(action / 1000);

    if (arr.find(map) == arr.end())
        return;

    uint32 string = arr[map][0];
    uint32 gossip = arr[map][1]; //unused
    uint32 normal_max = arr[map][2];
    uint32 normal_dmg = arr[map][3];
    uint32 normal_hp = arr[map][4];
    uint32 heroic_max = arr[map][5];
    uint32 heroic_dmg = arr[map][6];
    uint32 heroic_hp = arr[map][7];

    uint32 normal_boj_min = arr[map][8];
    uint32 normal_boj_max = arr[map][9];
    uint32 heroic_boj_min = arr[map][10];
    uint32 heroic_boj_max = arr[map][11];

    uint32 tele_map = arr[map][12];
    float tele_x = arr[map][13];
    float tele_y = arr[map][14];
    float tele_z = arr[map][15];

    uint8 modes = 0;
    if (normal_dmg && normal_hp && normal_boj_min && normal_boj_max)
        modes |= 1;

    if (heroic_dmg && heroic_hp && heroic_boj_min && heroic_boj_max)
        modes |= 2;

    if (!modes)
        return;

    std::stringstream ss;

    // type
    // 2 - list 
    // 3 - inside list
    // 4 - click normal
    // 5 - click heroic
    // 6 - teleport
    if (type == 2)
    {
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(string), GOSSIP_SENDER_MAIN, action_inside + map);
    }
    else if (type == 3)
    {
        if (modes & 1)
            player->ADD_GOSSIP_ITEM(4, player->GetSession()->GetHellgroundString(15594), GOSSIP_SENDER_MAIN, action_normal + map);

        if (modes & 2)
            player->ADD_GOSSIP_ITEM(4, player->GetSession()->GetHellgroundString(15595), GOSSIP_SENDER_MAIN, action_heroic + map);

        // teleport
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(15620), GOSSIP_SENDER_MAIN, action_tp + map);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 999);
    }
    else if (type == 4)
    {
        if ((modes & 1) == 0)
            return;

        ss << player->GetSession()->GetHellgroundString(string) << ' ' << player->GetSession()->GetHellgroundString(15622);
        ChatHandler(player->GetSession()).PSendSysMessage(15621, ss.str().c_str(), normal_max, normal_dmg, normal_hp, BoJMinMax(normal_boj_min, normal_boj_max).c_str());
        printStrings(map, 0);

        Instance_Teleporter_Gossip(player, creature, 0, action_inside + map);
    }
    else if (type == 5)
    {
        if ((modes & 2) == 0)
            return;
        
        ss << player->GetSession()->GetHellgroundString(string) << ' ' << player->GetSession()->GetHellgroundString(15623);
        ChatHandler(player->GetSession()).PSendSysMessage(15621, ss.str().c_str(), heroic_max, heroic_dmg, heroic_hp, BoJMinMax(heroic_boj_min, heroic_boj_max).c_str());
        printStrings(map, 1);

        Instance_Teleporter_Gossip(player, creature, 0, action_inside + map);
    }
    else if (type == 6)
    {
        player->TeleportTo(tele_map, tele_x, tele_y, tele_z, 0);
    }
}

void AddSC_Instance_Teleporter()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "instance_teleporter";
    newscript->pGossipHello = &Instance_Teleporter_Hello;
    newscript->pGossipSelect = &Instance_Teleporter_Gossip;
    //newscript->GetAI = &GetAI_Instance_Teleporter;
    newscript->RegisterSelf();
}