#include "precompiled.h"
#include "Language.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "Log.h"

struct SpecInfo {
    uint32 string;
    SpecMasks specMask;
};

struct ClassInfo {
    uint32 string;
    Classes Class;
};

std::vector<SpecInfo> specs = {
    {16701, SPEC_MASK_PHYSICAL_DD}, //Physical damage
    {16702, SPEC_MASK_SPELL_DD}, //Magical damage
    {16703, SPEC_MASK_HEALER}, //Healing
    {16704, SPEC_MASK_TANK} //Tank
};

std::vector<ClassInfo> classes = {
    {13033, CLASS_WARRIOR},
    {13034, CLASS_PALADIN},
    {13035, CLASS_HUNTER},
    {13036, CLASS_ROGUE},
    {13037, CLASS_PRIEST},
    {13038, CLASS_SHAMAN},
    {13039, CLASS_MAGE},
    {13040, CLASS_WARLOCK},
    {13041, CLASS_DRUID}
};

uint8 AvailableSpecMask(uint32 Class)
{
    uint8 available_mask = 0;
    
    switch (Class)
    {
    case CLASS_WARRIOR:
        available_mask = 9; //physdmg + tank
        break;
    case CLASS_PALADIN:
        available_mask = 15; //all
        break;
    case CLASS_HUNTER:
    case CLASS_ROGUE:
        available_mask = 1; //physdmg
        break;
    case CLASS_MAGE:
    case CLASS_WARLOCK:
        available_mask = 2; //spelldmg
        break;
    case CLASS_PRIEST:
        available_mask = 6; //spelldmg + heal
        break;
    case CLASS_SHAMAN:
        available_mask = 7; //physdmg + spelldmg + heal
        break;
    case CLASS_DRUID:
        available_mask = 15; //all
        break;
    }

    return available_mask;
}

bool GossipHello_raidchest(Player* player, uint32 option)
{
    std::string current_class = "";

    for (auto& cl : classes)
    {
        if (cl.Class == player->raid_chest_info.Class)
        {
            current_class = player->GetSession()->PGetHellgroundString(16713, player->GetSession()->GetHellgroundString(cl.string));
            break;
        }
    }

    if (current_class.empty())
        return true;

    std::string current_type = player->GetSession()->PGetHellgroundString(16717, player->GetSession()->GetHellgroundString(player->raid_chest_info.only_pve ? 16718 : 16719));

    if (current_type.empty())
        return true;

    std::string current_spec = player->GetSession()->GetHellgroundString(16705);
    if (player->raid_chest_info.specMask)
    {
        bool found = false;
        for (auto& spec : specs)
        {
            if (spec.specMask == player->raid_chest_info.specMask)
            {
                found = true;
                current_spec = player->GetSession()->PGetHellgroundString(16714, player->GetSession()->GetHellgroundString(spec.string));
                break;
            }
        }

        if (!found)
        {
            // paladin spelldmg+healer
            if (player->raid_chest_info.specMask == 6)
                current_spec = player->GetSession()->PGetHellgroundString(16714, player->GetSession()->GetHellgroundString(16716));
            // druid tank+physdmg
            else if (player->raid_chest_info.specMask == 9)
                current_spec = player->GetSession()->PGetHellgroundString(16714, player->GetSession()->GetHellgroundString(16715));
            else
            {
                sLog.outLog(LOG_CRITICAL, "GossipHello_raidchest can't set spec");
                return true;
            }
        }
    }

    std::string current_weapon = player->GetSession()->GetHellgroundString(16706);
    if (option == 1)
    {
        if (player->raid_chest_info.leg_weapon)
        {
            for (auto& weapon : legendary_weapons)
            {
                if (weapon.entry == player->raid_chest_info.leg_weapon)
                {
                    current_weapon = player->GetItemLink(weapon.entry, false, ITEM_LINK_NO_COLOR);
                    break;
                }
            }
        }
    }

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, current_class.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 300);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, current_type.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, current_spec.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 200);

    if (option == 1)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, current_weapon.c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000000);

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, player->GetSession()->GetHellgroundString(16712), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(990132, player->GetGUID());
    return true;
}

bool GossipSelect_raidchest(Player* player,  uint32 uiSender, uint32 action, uint32 option)
{
    sLog.outLog(LOG_TMP, "RC_DEBUG - Player %s action %u option %u", player->GetName(), action, option);
    
    if (player->IsNonMeleeSpellCast(true))
    {
        GossipHello_raidchest(player, option);
        return true;
    }
    
    uint32 act = action - GOSSIP_ACTION_INFO_DEF;
    
    // info
    if (act == 1)
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(12002), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->SEND_GOSSIP_MENU(990131, player->GetGUID());
        return true;
    }
    // weapon
    else if (act >= 1000000 && act < 2000000)
    {
        if (option != 1)
            return true;
        
        // select weapon
        if (act == 1000000)
        {
            for (auto& weapon : legendary_weapons)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetItemLink(weapon.entry, false, ITEM_LINK_NO_COLOR).c_str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1000000 + weapon.entry);
            }
            player->SEND_GOSSIP_MENU(990136, player->GetGUID());
            return true;
        }
        // set weapon
        else
        {
            act -= 1000000;
            
            for (const auto& weapon : legendary_weapons) {
                if (weapon.entry == act) {
                    player->raid_chest_info.leg_weapon = act;
                    ChatHandler(player).PSendSysMessage(16711, player->GetItemLink(act).c_str());
                    break;
                }
            }
        }
    }
    // type
    else if (act >= 100 && act < 200)
    {
        // select type
        if (act == 100)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(16718), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(16719), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100 + 2);
            player->SEND_GOSSIP_MENU(990134, player->GetGUID());
            return true;
        }
        // set type
        else
        {
            act -= 100;

            if (act == 1)
                player->raid_chest_info.only_pve = true;
            else
                player->raid_chest_info.only_pve = false;

            player->raid_chest_info.specMask = SPEC_MASK_NONE;
        }
    }
    // spec
    else if (act >= 200 && act < 300)
    {
        // select spec
        if (act == 200)
        {
            for (const auto& spec : specs) {
                // no tank specs for pvp-only
                if (!player->raid_chest_info.only_pve && spec.specMask & SPEC_MASK_TANK)
                    continue;

                if (player->raid_chest_info.Class && (spec.specMask & AvailableSpecMask(player->raid_chest_info.Class)))
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(spec.string), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 200 + spec.specMask);
            }
            player->SEND_GOSSIP_MENU(990134, player->GetGUID());
            return true;
        }
        // set spec
        else
        {
            act -= 200;
            
            switch (act)
            {
            case SPEC_MASK_PHYSICAL_DD:
            case SPEC_MASK_SPELL_DD:
            case SPEC_MASK_HEALER:
            case SPEC_MASK_TANK:
                player->raid_chest_info.specMask = SpecMasks(act);
                ChatHandler(player).SendSysMessage(16710);
            }
        }
    }
    // class
    else if (act >= 300 && act < 400)
    {
        // select class
        if (act == 300)
        {
            for (auto& cl : classes)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, player->GetSession()->GetHellgroundString(cl.string), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 300 + cl.Class);

            player->SEND_GOSSIP_MENU(990133, player->GetGUID());
            return true;
        }
        // set class
        else
        {
            act -= 300;
            
            switch (act)
            {
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_HUNTER:
            case CLASS_ROGUE:
            case CLASS_PRIEST:
            case CLASS_SHAMAN:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
            case CLASS_DRUID:
                player->raid_chest_info.Class = Classes(act);
                player->raid_chest_info.specMask = SPEC_MASK_NONE;

                if (player->GetClass() != act)
                    ChatHandler(player).SendSysMessage(16709);
            }
        }
    }

    if (player->raid_chest_info.Class == CLASS_PALADIN && player->raid_chest_info.specMask & SPEC_MASK_SPELL_DD && player->raid_chest_info.only_pve)
    {
        ChatHandler(player).SendSysMessage(15547);
        player->raid_chest_info.specMask = SpecMasks(6);
    }
    else if (player->raid_chest_info.Class == CLASS_DRUID && player->raid_chest_info.specMask & SPEC_MASK_TANK)
    {
        ChatHandler(player).SendSysMessage(15548);
        player->raid_chest_info.specMask = SpecMasks(9);
    }

    GossipHello_raidchest(player, option);
    return true;
}

void AddSC_raidchest()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "mw_player_raidchest";
    newscript->pGossipHelloPlayer = &GossipHello_raidchest;
    newscript->pGossipSelectPlayer = &GossipSelect_raidchest;
    newscript->RegisterSelf();
}