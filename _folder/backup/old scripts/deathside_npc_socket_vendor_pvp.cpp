// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"
#include "Player.h"

bool GossipHello_deathside_npc_socket_vendor_pvp(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_GEM_META), GOSSIP_SENDER_MAIN, 692020);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_GEM_BLUE), GOSSIP_SENDER_MAIN, 692021);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_GEM_RED), GOSSIP_SENDER_MAIN, 692022);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_GEM_YELLOW), GOSSIP_SENDER_MAIN, 692023);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_GEM_GREEN), GOSSIP_SENDER_MAIN, 692024);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_GEM_PURPLE), GOSSIP_SENDER_MAIN, 692025);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_GEM_ORANGE), GOSSIP_SENDER_MAIN, 692026);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}
bool GossipSelect_deathside_npc_socket_vendor_pvp(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, creature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        creature->Whisper(chrErr, player->GetGUID());
        return false;
    }
    player->GetSession()->SendListInventory(Found->GetGUID());
    player->CLOSE_GOSSIP_MENU();
    return true;
}

bool GossipHello_deathside_npc_luxury_vendor(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_GADGETS), GOSSIP_SENDER_MAIN, 992015);
    // player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_TRANSMOGRIFICATIONS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_MOUNTS), GOSSIP_SENDER_MAIN, 992017);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_TABARDS), GOSSIP_SENDER_MAIN, 992018);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_COMPANIONS), GOSSIP_SENDER_MAIN, 992019);
    // player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_SHIRT), GOSSIP_SENDER_MAIN, 992020);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_luxury_vendor(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            player->PlayerTalkClass->ClearMenus();
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_D1D2), GOSSIP_SENDER_MAIN, 992050);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_D3), GOSSIP_SENDER_MAIN, 992055);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_T1T5), GOSSIP_SENDER_MAIN, 992051);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_A1A3), GOSSIP_SENDER_MAIN, 992052);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_OFFSET), GOSSIP_SENDER_MAIN, 992053);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LUX_WEAPON), GOSSIP_SENDER_MAIN, 992054);
            player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
            break;
        }
        default:
        {
            Unit *Found = FindCreature(action, 5.0f, creature);
            if(!Found)
            {
                char chrErr[256];
                sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
                creature->Whisper(chrErr, player->GetGUID());
                return false;
            }
            player->GetSession()->SendListInventory(Found->GetGUID());
            player->CLOSE_GOSSIP_MENU();
            break;
        }
    }
    return true;
}

bool GossipHello_deathside_npc_services(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(0, "Guild manager(Управление гильдией)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->ADD_GOSSIP_ITEM(0, "Change appearance(Изменить внешность)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
    player->ADD_GOSSIP_ITEM(0, "Change race(Изменить расу)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_services(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            player->ADD_GOSSIP_ITEM(0, "How do I form a guild?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->ADD_GOSSIP_ITEM(0, "I want to create a guild crest.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->ADD_GOSSIP_ITEM(0, "Buy guild tabard.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+4:
        {
            player->GetSession()->SendPetitionShowList(creature->GetGUID());
            player->PlayerTalkClass->CloseGossip();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+5:
        {
            player->PlayerTalkClass->CloseGossip();
            player->GetSession()->SendTabardVendorActivate(creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+6:
        {
            player->GetSession()->SendListInventory(creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+2:
        {
            // barber shop menu
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+3:
        {
            // RaceList
            break;
        }
    }
    return true;
}

bool GossipHello_deathside_npc_multiitems(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_CLOTH), GOSSIP_SENDER_MAIN, 992021);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LEATHER), GOSSIP_SENDER_MAIN, 992022);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIL), GOSSIP_SENDER_MAIN, 992023);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_PLATE), GOSSIP_SENDER_MAIN, 992024);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_multiitems(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, creature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        creature->Whisper(chrErr, player->GetGUID());
        return false;
    }
    player->GetSession()->SendListInventory(Found->GetGUID());
    player->CLOSE_GOSSIP_MENU();
    return true;
}

bool GossipHello_deathside_npc_multiitems_cost(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_CLOTH), GOSSIP_SENDER_MAIN, 992021);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_CLOTH_BG), GOSSIP_SENDER_MAIN, 992071);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LEATHER_BG), GOSSIP_SENDER_MAIN, 992072);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIL_BG), GOSSIP_SENDER_MAIN, 992073);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_PLATE_BG), GOSSIP_SENDER_MAIN, 992074);
    if(player->GetClass() == CLASS_MAGE || player->GetClass() == CLASS_WARLOCK || player->GetClass() == CLASS_PRIEST)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_CLOTH_ARENA), GOSSIP_SENDER_MAIN, 992075);
    if(player->GetClass() == CLASS_ROGUE || player->GetClass() == CLASS_DRUID)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_LEATHER_ARENA), GOSSIP_SENDER_MAIN, 992076);
    if(player->GetClass() == CLASS_SHAMAN || player->GetClass() == CLASS_HUNTER)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIL_ARENA), GOSSIP_SENDER_MAIN, 992077);
    if(player->GetClass() == CLASS_WARRIOR || player->GetClass() == CLASS_PALADIN)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_PLATE_ARENA), GOSSIP_SENDER_MAIN, 992078);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_multiitems_cost(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, creature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        creature->Whisper(chrErr, player->GetGUID());
        return false;
    }
    player->GetSession()->SendListInventory(Found->GetGUID());
    player->CLOSE_GOSSIP_MENU();
    return true;
}

bool GossipHello_deathside_npc_multiweapon_cost(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WEAPON_F), GOSSIP_SENDER_MAIN, 992084);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WEAPON_BG), GOSSIP_SENDER_MAIN, 992079);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WEAPON_ARENA), GOSSIP_SENDER_MAIN, 992080);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_multiweapon_cost(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, creature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        creature->Whisper(chrErr, player->GetGUID());
        return false;
    }
    player->GetSession()->SendListInventory(Found->GetGUID());
    player->CLOSE_GOSSIP_MENU();
    return true;
}

bool GossipHello_deathside_npc_multiaccessories_cost(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ACCESSORIES), GOSSIP_SENDER_MAIN, 992085);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ACCESSORIES_BG), GOSSIP_SENDER_MAIN, 992081);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ACCESSORIES_ARENA), GOSSIP_SENDER_MAIN, 992082);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_multiaccessories_cost(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, creature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        creature->Whisper(chrErr, player->GetGUID());
        return false;
    }
    player->GetSession()->SendListInventory(Found->GetGUID());
    player->CLOSE_GOSSIP_MENU();
    return true;
}

bool GossipHello_deathside_npc_multirelic_cost(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_RELIC), GOSSIP_SENDER_MAIN, 992086);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_RELIC_ARENA), GOSSIP_SENDER_MAIN, 992083);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_multirelic_cost(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, creature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        creature->Whisper(chrErr, player->GetGUID());
        return false;
    }
    player->GetSession()->SendListInventory(Found->GetGUID());
    player->CLOSE_GOSSIP_MENU();
    return true;
}

bool GossipHello_deathside_npc_teleport_battlerealm(Player *player, Creature *creature)
{
    if(player->GetMapId() == 564)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_TELE_START), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    else
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_TELE_HIDDEN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
    
    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_teleport_battlerealm(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (sender == GOSSIP_SENDER_MAIN)
    {
        switch(action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->TeleportTo(0, -4221.37, -3370.85, 231.94, 0);
                player->CLOSE_GOSSIP_MENU();
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->TeleportTo(564, 600.82, 402.29, 187.08, 0);
                player->CLOSE_GOSSIP_MENU();
                break;
            default:
                player->CLOSE_GOSSIP_MENU();
                break;
        }
    }
    return true;
}

bool GossipHello_deathside_npc_welcome(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WELCOME_ENGLISH), GOSSIP_SENDER_MAIN, 992998);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_WELCOME_RUSSIAN), GOSSIP_SENDER_MAIN, 992999);
    player->SEND_GOSSIP_MENU(30000, creature->GetGUID());
    return true;
}

bool GossipSelect_deathside_npc_welcome(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    Unit *Found = FindCreature(action, 5.0f, creature);
    if(!Found)
    {
        char chrErr[256];
        sprintf(chrErr, player->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
        creature->Whisper(chrErr, player->GetGUID());
        return false;
    }
    player->PrepareQuestMenu(Found->GetGUID());
    player->SendPreparedQuest(Found->GetGUID());
    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, Found->GetGUID());
    return true;
}

void AddSC_deathside_npc_socket_vendor_pvp()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "deathside_npc_socket_vendor_pvp";
    newscript->pGossipHello = &GossipHello_deathside_npc_socket_vendor_pvp;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_socket_vendor_pvp;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_luxury_vendor";
    newscript->pGossipHello = &GossipHello_deathside_npc_luxury_vendor;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_luxury_vendor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_services";
    newscript->pGossipHello = &GossipHello_deathside_npc_services;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_services;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_multiitems";
    newscript->pGossipHello = &GossipHello_deathside_npc_multiitems;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_multiitems;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_multiitems_cost";
    newscript->pGossipHello = &GossipHello_deathside_npc_multiitems_cost;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_multiitems_cost;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_multiweapon_cost";
    newscript->pGossipHello = &GossipHello_deathside_npc_multiweapon_cost;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_multiweapon_cost;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_multiaccessories_cost";
    newscript->pGossipHello = &GossipHello_deathside_npc_multiaccessories_cost;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_multiaccessories_cost;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_multirelic_cost";
    newscript->pGossipHello = &GossipHello_deathside_npc_multirelic_cost;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_multirelic_cost;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_teleport_battlerealm";
    newscript->pGossipHello = &GossipHello_deathside_npc_teleport_battlerealm;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_teleport_battlerealm;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "deathside_npc_welcome";
    newscript->pGossipHello = &GossipHello_deathside_npc_welcome;
    newscript->pGossipSelect = &GossipSelect_deathside_npc_welcome;
    newscript->RegisterSelf();
}