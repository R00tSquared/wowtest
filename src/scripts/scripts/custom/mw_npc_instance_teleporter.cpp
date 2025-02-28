// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

bool Instance_Teleporter_Hello(Player* Plr, Creature* source)
{
    if (!sWorld.isEasyRealm())
    {
        Plr->CLOSE_GOSSIP_MENU();
        return true;
    }

    bool is_book = (source->GetEntry() == 1136) ? true : false;

    if (is_book && !Plr->GetSession()->isPremium())
    {
        Plr->CLOSE_GOSSIP_MENU();
        ChatHandler(Plr->GetSession()).SendSysMessage(LANG_PREMIUM_NOT_ACTIVE);
        return true;
    }

    Plr->PlayerTalkClass->ClearMenus();

    Plr->ADD_GOSSIP_ITEM(2, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_DUNGEONS), GOSSIP_SENDER_MAIN, 5000);
    Plr->ADD_GOSSIP_ITEM(2, Plr->GetSession()->GetHellgroundString(16672), GOSSIP_SENDER_MAIN, 6000); //Solo dungeons
    Plr->ADD_GOSSIP_ITEM(2, Plr->GetSession()->GetHellgroundString(16673), GOSSIP_SENDER_MAIN, 7000); //Other instances

    // Main Menu for Alliance
    if (Plr->GetTeam() == ALLIANCE)
        Plr->ADD_GOSSIP_ITEM(2, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ALLIANCE_CITIES), GOSSIP_SENDER_MAIN, 1000);
    else // Main Menu for Horde
        Plr->ADD_GOSSIP_ITEM(2, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_HORDE_CITIES), GOSSIP_SENDER_MAIN, 2000);

    Plr->ADD_GOSSIP_ITEM(2, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEUTRAL_TOWNS), GOSSIP_SENDER_MAIN, 3000);

    // Book of Teleportation
    if (is_book)
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_TELEPORT_BACK), GOSSIP_SENDER_MAIN, 9000);
    else
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(16661), GOSSIP_SENDER_MAIN, 1);

    Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());

    Plr->CastSpell(Plr, 1206, false);
    return true;
}

void SendDefaultMenu_TeleItem(Player* Plr, Creature* source, uint32 action)
{
    if (!sWorld.isEasyRealm())
    {
        Plr->CLOSE_GOSSIP_MENU();
        return;
    }

    bool is_book = (source->GetEntry() == 1136) ? true : false;

    if (is_book && !Plr->GetSession()->isPremium())
    {
        Plr->CLOSE_GOSSIP_MENU();
        ChatHandler(Plr->GetSession()).SendSysMessage(LANG_PREMIUM_NOT_ACTIVE);
        return;
    }

    // Not allow in combat
    if (Plr->IsInCombat() || Plr->InBattleGroundOrArena())
    {
        Plr->CLOSE_GOSSIP_MENU();
        Plr->GetSession()->SendNotification(LANG_SCRIPT_YOU_ARE_IN_COMBAT);
        return;
    }

    Plr->PlayerTalkClass->ClearMenus();
    switch (action)
    {
    case 1:
        if (!is_book)
        {
            Plr->GetSession()->SendBindPoint(source, -1963.590, 5496.124, -12.427, 3703, 530);
            ChatHandler(Plr->GetSession()).SendSysMessage(16675);
            Plr->CLOSE_GOSSIP_MENU();
        }
        break;
    case 2: //Back To Main Menu
        Instance_Teleporter_Hello(Plr, source);
        break;
    case 1000: //Alliance Town
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DARNASSUS), GOSSIP_SENDER_MAIN, 1001);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_EXODAR), GOSSIP_SENDER_MAIN, 1005);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_IRONFORGE), GOSSIP_SENDER_MAIN, 1010);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STORMWIND), GOSSIP_SENDER_MAIN, 1015);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    case 2000: //Horde Town
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ORGRIMMAR), GOSSIP_SENDER_MAIN, 2001);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SILVERMOON), GOSSIP_SENDER_MAIN, 2005);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_THUNDER_BLUFF), GOSSIP_SENDER_MAIN, 2010);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_UNDERCITY), GOSSIP_SENDER_MAIN, 2015);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    case 3000: //Neutral Town
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SHATTRATH), GOSSIP_SENDER_MAIN, 3035);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BOOTY_BAY), GOSSIP_SENDER_MAIN, 3005);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_EVERLOOK), GOSSIP_SENDER_MAIN, 3015);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GADGETZAN), GOSSIP_SENDER_MAIN, 3020);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MUDSPOCKET), GOSSIP_SENDER_MAIN, 3025);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RACTHET), GOSSIP_SENDER_MAIN, 3030);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ISLE_QUEL_DANAS), GOSSIP_SENDER_MAIN, 3040);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_AERIS_LANDING), GOSSIP_SENDER_MAIN, 3045);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_THORIUM_POINT), GOSSIP_SENDER_MAIN, 3195);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BRONZEBEARD_CAMP), GOSSIP_SENDER_MAIN, 3065);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CENARION_HOLD), GOSSIP_SENDER_MAIN, 3070);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CENRION_REGUGE), GOSSIP_SENDER_MAIN, 3075);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_COSMOWRENCH), GOSSIP_SENDER_MAIN, 3080);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 3050);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    case 3050:
        if (Plr->GetReputationMgr().GetReputation(932) /* aldor */ > 3 /* neutral */)
            Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ALTAR_OF_SHATAR), GOSSIP_SENDER_MAIN, 3055);
        else
            Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SOTS), GOSSIP_SENDER_MAIN, 3175);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_EMERALD_SNACTUARY), GOSSIP_SENDER_MAIN, 3085);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_TIMBERMAW_HOLD), GOSSIP_SENDER_MAIN, 3200);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_VALORS_REST), GOSSIP_SENDER_MAIN, 3100);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_HALAA), GOSSIP_SENDER_MAIN, 3105);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_HARBORAGE), GOSSIP_SENDER_MAIN, 3110);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_WIZARD_ROW), GOSSIP_SENDER_MAIN, 3115);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_LIGHTS_HOPE_CHAPEL), GOSSIP_SENDER_MAIN, 3120);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MARSHALS_REFUGE), GOSSIP_SENDER_MAIN, 3125);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ECO_DOME_MIDREALM), GOSSIP_SENDER_MAIN, 3135);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MIRAGE_RACEWAY), GOSSIP_SENDER_MAIN, 3140);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STORMSPIRE), GOSSIP_SENDER_MAIN, 3190);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 3090);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 3000);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    case 3090:
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_NESINGWARE_EXPED), GOSSIP_SENDER_MAIN, 3150);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_NIGHTHAVEN), GOSSIP_SENDER_MAIN, 3155);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_OGRILA), GOSSIP_SENDER_MAIN, 3160);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_PROTECTORATE_W_P), GOSSIP_SENDER_MAIN, 3165);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SPOREGGAR), GOSSIP_SENDER_MAIN, 3180);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STEAMWHEEDLE_PORT), GOSSIP_SENDER_MAIN, 3185);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 3050);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;

    case 5000: //Actual raids
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SUNWELL), GOSSIP_SENDER_MAIN, 7025);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACK_TEMPLE), GOSSIP_SENDER_MAIN, 8005);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15458), GOSSIP_SENDER_MAIN, 6021); //Hyjal
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ZULAMAN), GOSSIP_SENDER_MAIN, 7075);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_KARAZHAN), GOSSIP_SENDER_MAIN, 7030);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15463), GOSSIP_SENDER_MAIN, 8037); //Serpentshrine Cavern
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15464), GOSSIP_SENDER_MAIN, 8042); //Tempest Keep: The Eye
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GRUULS_LAIR), GOSSIP_SENDER_MAIN, 8015);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15461), GOSSIP_SENDER_MAIN, 8021); //Magtheridon's Lair
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15459), GOSSIP_SENDER_MAIN, 7081); // Naxxramas
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ONYXIA_LAIR), GOSSIP_SENDER_MAIN, 6020);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;

    case 6000: //Solo dungeons  
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15466), GOSSIP_SENDER_MAIN, 8022); //Magister's Terrace
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15473), GOSSIP_SENDER_MAIN, 8025); //Tempest Keep: The Arcatraz 
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15474), GOSSIP_SENDER_MAIN, 8035); //Tempest Keep: The Botanica 
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15475), GOSSIP_SENDER_MAIN, 8036); //Tempest Keep: The Mechanar 
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15470), GOSSIP_SENDER_MAIN, 8010); //Coilfang: The Steamvault 
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15471), GOSSIP_SENDER_MAIN, 8033); //Coilfang: The Underbog   
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15472), GOSSIP_SENDER_MAIN, 8034); //Coilfang: The Slave Pens
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15467), GOSSIP_SENDER_MAIN, 8020); //Hellfire Citadel: The Shattered Halls
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15468), GOSSIP_SENDER_MAIN, 8030); //Hellfire Citadel: The Blood Furnace  
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15469), GOSSIP_SENDER_MAIN, 8031); //Hellfire Citadel: Ramparts
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 6100);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;

    case 6100: //Solo dungeons 2
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15476), GOSSIP_SENDER_MAIN, 8038); //Auchindoun: Shadow Labyrinth 
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15477), GOSSIP_SENDER_MAIN, 8039); //Auchindoun: Sethekk Halls    
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15478), GOSSIP_SENDER_MAIN, 8040); //Auchindoun: Mana-Tombs       
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15479), GOSSIP_SENDER_MAIN, 8041); //Auchindoun: Auchenai Crypts  
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15480), GOSSIP_SENDER_MAIN, 6005); //Old Hillsbrad Foothills  
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(15481), GOSSIP_SENDER_MAIN, 6006); //The Black Morass
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 6000);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    
    case 7000: //Other instances 1
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_TEMPLE_AQ), GOSSIP_SENDER_MAIN, 6045);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RUINS_AQ), GOSSIP_SENDER_MAIN, 6040);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MOLTEN_CORE), GOSSIP_SENDER_MAIN, 7035);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BWL), GOSSIP_SENDER_MAIN, 7010);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACKFATHOM_DEEPS), GOSSIP_SENDER_MAIN, 6001);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DIRE_MAUL), GOSSIP_SENDER_MAIN, 6010);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MARAUDON), GOSSIP_SENDER_MAIN, 6015);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RAZORFEN_DOWNS), GOSSIP_SENDER_MAIN, 6030);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 7100);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    case 7100: //Other instances 2
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RAZORFEN_KRAUL), GOSSIP_SENDER_MAIN, 6035);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_WAILING_CAVERNS), GOSSIP_SENDER_MAIN, 6050);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ZULFARRAK), GOSSIP_SENDER_MAIN, 6055);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ZULGURUB), GOSSIP_SENDER_MAIN, 7080);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ULDAMAN), GOSSIP_SENDER_MAIN, 7070);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SCARLET_MONASTERY), GOSSIP_SENDER_MAIN, 7040);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SCHOLOMANCE), GOSSIP_SENDER_MAIN, 7045);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SHF_KEEP), GOSSIP_SENDER_MAIN, 7050);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 7200);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 7000);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    case 7200: //Other instances 3
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STRATHOLME), GOSSIP_SENDER_MAIN, 7055);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SUNKEN_TEMPLE), GOSSIP_SENDER_MAIN, 7060);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACKROCK_DEPTHS), GOSSIP_SENDER_MAIN, 7001);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACKROCK_SPIRE), GOSSIP_SENDER_MAIN, 7005);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GNOMEREGAN), GOSSIP_SENDER_MAIN, 7020);
        Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DEADMINES), GOSSIP_SENDER_MAIN, 7015);

        if (Plr->GetTeam() == HORDE)
            Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RAGEFIRE_CHASM), GOSSIP_SENDER_MAIN, 6025);
        else
            Plr->ADD_GOSSIP_ITEM(5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STOCKADES), GOSSIP_SENDER_MAIN, 7065);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 7100);
        Plr->ADD_GOSSIP_ITEM(7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2);
        Plr->SEND_GOSSIP_MENU(990122, source->GetGUID());
        break;
    default: // cast teleport
        Plr->CLOSE_GOSSIP_MENU();

        if (Plr->IsMounted())
        {
            Plr->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
            Plr->Unmount();
        }

        int32 bpaction = action;
        Plr->CastCustomSpell(Plr, 55116, &bpaction, NULL, NULL, is_book ? false : true);
        break;
    }
}

bool Instance_Teleporter_Gossip(Player* pPlayer, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    // Main menu
    if (uiSender == GOSSIP_SENDER_MAIN)
        SendDefaultMenu_TeleItem(pPlayer, creature, uiAction);

    return true;
}

void AddSC_Instance_Teleporter()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "instance_teleporter";
    newscript->pGossipHello = &Instance_Teleporter_Hello;
    newscript->pGossipSelect = &Instance_Teleporter_Gossip;
    newscript->RegisterSelf();
}