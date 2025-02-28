// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

void SendDefaultMenu_Npc_Teleport_X(Player* Plr, Creature* pCreature, uint32 action)
{
    // Not allow in combat
    if (Plr->isInCombat())
    {
        Plr->CLOSE_GOSSIP_MENU();
        Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_IN_COMBAT));
        return;
    }
 
    switch(action)
    {
        case 1000: //Alliance Town
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DARNASSUS), GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_EXODAR), GOSSIP_SENDER_MAIN, 1005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_IRONFORGE), GOSSIP_SENDER_MAIN, 1010);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STORMWIND), GOSSIP_SENDER_MAIN, 1015);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 5005);
             Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 2000: //Horde Town
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ORGRIMMAR), GOSSIP_SENDER_MAIN, 2001);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SILVERMOON), GOSSIP_SENDER_MAIN, 2005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_THUNDER_BLUFF), GOSSIP_SENDER_MAIN, 2010);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_UNDERCITY), GOSSIP_SENDER_MAIN, 2015);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 5005);
             Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 3000: //Neutral Town
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BOOTY_BAY), GOSSIP_SENDER_MAIN, 3005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_EVERLOOK), GOSSIP_SENDER_MAIN, 3015);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GADGETZAN), GOSSIP_SENDER_MAIN, 3020);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MUDSPOCKET), GOSSIP_SENDER_MAIN, 3025);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RACTHET), GOSSIP_SENDER_MAIN, 3030);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SHATTRATH), GOSSIP_SENDER_MAIN, 3035);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ISLE_QUEL_DANAS), GOSSIP_SENDER_MAIN, 3040);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_AERIS_LANDING), GOSSIP_SENDER_MAIN, 3045);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_THORIUM_POINT), GOSSIP_SENDER_MAIN, 3195);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BRONZEBEARD_CAMP), GOSSIP_SENDER_MAIN, 3065);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CENARION_HOLD), GOSSIP_SENDER_MAIN, 3070);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CENRION_REGUGE), GOSSIP_SENDER_MAIN, 3075);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_COSMOWRENCH), GOSSIP_SENDER_MAIN, 3080);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 3050);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 5005);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 3050:
            if (Plr->GetReputationMgr().GetReputation(932) /* aldor */  > 3 /* neutral */ )
                Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ALTAR_OF_SHATAR), GOSSIP_SENDER_MAIN, 3055);
            else
                Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SOTS), GOSSIP_SENDER_MAIN, 3175);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_EMERALD_SNACTUARY), GOSSIP_SENDER_MAIN, 3085);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_TIMBERMAW_HOLD), GOSSIP_SENDER_MAIN, 3200);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_VALORS_REST), GOSSIP_SENDER_MAIN, 3100);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_HALAA), GOSSIP_SENDER_MAIN, 3105);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_HARBORAGE), GOSSIP_SENDER_MAIN, 3110);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_WIZARD_ROW), GOSSIP_SENDER_MAIN, 3115);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_LIGHTS_HOPE_CHAPEL), GOSSIP_SENDER_MAIN, 3120);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MARSHALS_REFUGE), GOSSIP_SENDER_MAIN, 3125);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ECO_DOME_MIDREALM), GOSSIP_SENDER_MAIN, 3135);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MIRAGE_RACEWAY), GOSSIP_SENDER_MAIN, 3140);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STORMSPIRE), GOSSIP_SENDER_MAIN, 3190);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 3090);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 3000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 5005);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 3090:
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_NESINGWARE_EXPED), GOSSIP_SENDER_MAIN, 3150);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_NIGHTHAVEN), GOSSIP_SENDER_MAIN, 3155);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_OGRILA), GOSSIP_SENDER_MAIN, 3160);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_PROTECTORATE_W_P), GOSSIP_SENDER_MAIN, 3165);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SPOREGGAR), GOSSIP_SENDER_MAIN, 3180);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STEAMWHEEDLE_PORT), GOSSIP_SENDER_MAIN, 3185);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 3050);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 5005);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5000: //Dungeons
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_KALIMDOR), GOSSIP_SENDER_MAIN, 5010);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_EASTERN_KINGDOMS), GOSSIP_SENDER_MAIN, 5015);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_OUTLAND), GOSSIP_SENDER_MAIN, 5025);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 5005);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5005: //Back To Main Menu
            // Main Menu for Alliance
            if (Plr->GetTeam() == ALLIANCE)
                Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ALLIANCE_CITIES), GOSSIP_SENDER_MAIN, 1000);
            else // Main Menu for Horde
                Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_HORDE_CITIES), GOSSIP_SENDER_MAIN, 2000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEUTRAL_TOWNS), GOSSIP_SENDER_MAIN, 3000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_DUNGEONS), GOSSIP_SENDER_MAIN, 5000);
             Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5010: //Kalimdor
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACKFATHOM_DEEPS), GOSSIP_SENDER_MAIN, 6001);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_CAVERNS_OF_TIME), GOSSIP_SENDER_MAIN, 6005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DIRE_MAUL), GOSSIP_SENDER_MAIN, 6010);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MARAUDON), GOSSIP_SENDER_MAIN, 6015);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ONYXIA_LAIR), GOSSIP_SENDER_MAIN, 6020);
            if (Plr->GetTeam() == HORDE)
                Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RAGEFIRE_CHASM), GOSSIP_SENDER_MAIN, 6025);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RAZORFEN_DOWNS), GOSSIP_SENDER_MAIN, 6030);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RAZORFEN_KRAUL), GOSSIP_SENDER_MAIN, 6035);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_RUINS_AQ), GOSSIP_SENDER_MAIN, 6040);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_TEMPLE_AQ), GOSSIP_SENDER_MAIN, 6045);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_WAILING_CAVERNS), GOSSIP_SENDER_MAIN, 6050);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ZULFARRAK), GOSSIP_SENDER_MAIN, 6055);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 5000);   
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5015: //Eastern Kingdoms 1
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACKROCK_DEPTHS), GOSSIP_SENDER_MAIN, 7001);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACKROCK_SPIRE), GOSSIP_SENDER_MAIN, 7005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BWL), GOSSIP_SENDER_MAIN, 7010);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_DEADMINES), GOSSIP_SENDER_MAIN, 7015);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GNOMEREGAN), GOSSIP_SENDER_MAIN, 7020);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SUNWELL), GOSSIP_SENDER_MAIN, 7025);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_KARAZHAN), GOSSIP_SENDER_MAIN, 7030);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_MOLTEN_CORE), GOSSIP_SENDER_MAIN, 7035);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 5020);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 5000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5020: //Eastern Kingdoms 2
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SCARLET_MONASTERY), GOSSIP_SENDER_MAIN, 7040);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SCHOLOMANCE), GOSSIP_SENDER_MAIN, 7045);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SHF_KEEP), GOSSIP_SENDER_MAIN, 7050);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STRATHOLME), GOSSIP_SENDER_MAIN, 7055);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_SUNKEN_TEMPLE), GOSSIP_SENDER_MAIN, 7060);
            if (Plr->GetTeam() == ALLIANCE)
                Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_STOCKADES), GOSSIP_SENDER_MAIN, 7065);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ULDAMAN), GOSSIP_SENDER_MAIN, 7070);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ZULAMAN), GOSSIP_SENDER_MAIN, 7075);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_ZULGURUB), GOSSIP_SENDER_MAIN, 7080);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 5015);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 5005);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5025: //Outland    
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_AUCHINDOUN), GOSSIP_SENDER_MAIN, 8001);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_BLACK_TEMPLE), GOSSIP_SENDER_MAIN, 8005);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_COIL_RESERVOIR), GOSSIP_SENDER_MAIN, 8010);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_GRUULS_LAIR), GOSSIP_SENDER_MAIN, 8015);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_HF_CITADEL), GOSSIP_SENDER_MAIN, 8020);
            Plr->ADD_GOSSIP_ITEM( 5, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_LOCATION_TEMPEST_KEEP), GOSSIP_SENDER_MAIN, 8025);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 5000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        //////////////////////////////////////////////////ALLIANCE///////////////////////////////////////////////////////////////
         
        case 1001: // Darnassus
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 9947.52f, 2482.73f, 1316.21f, 4.73f);
            break;
        case 1005: // Exodar
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -3954.20f, -11656.54f, -138.69f, 1.66f);
            break;
        case 1010: // Ironforge
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -4924.07f, -951.95f, 501.55f, 5.40f);
            break;
        case 1015: // Stormwind
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -8960.14f, 516.266f, 96.3568f, 0.68f);
            break;
        //////////////////////////////////////////////////HORDE///////////////////////////////////////////////////////////////
        case 2001: // Orgrimmar
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 1664.23f, -4345.12f, 62.0102f, 3.867f);
            break;
        case 2005: // Silvermoon
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 9526.51f, -7300.27f, 15.2325f, 3.1f);
            break;
        case 2010: // Thunder Bluff
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -1197.04f, 30.9699f, 176.949f, 4.729000f);
            break;
        case 2015: // Undercity
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, 1595.73f, 240.264f, 62.028f, 6.27f);
            break;
        //////////////////////////////////////////////////NEUTRAL///////////////////////////////////////////////////////////////
        case 3005:// Booty Bay
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -14438.2f, 472.22f, 15.32f, 0.65);
            break;
        case 3015: //Everlook
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 6724.77f, -4610.68f, 720.78f, 4.78f);
            break;
        case 3020: //Gadgetzan
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -7173.26f, -3785.60f, 8.37f, 6.13f);
            break;
        case 3025: //Mudsprocket
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -4564.79f, -3172.38f, 33.93f, 3.21f);
            break;
        case 3030: //Ratchet
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -956.664f, -3754.71f, 5.33239f, 0.996637f);
            break;
        case 3035:// Shattrath City
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -1850.209961f, 5435.821777f, -10.961435f, 3.403913f);
            break;
        case 3040:// Isle Of Quel'Danas
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 12947.4f, -6893.31f, 5.68398f, 3.09154f);
            break;
        case 3045:// Aeris Landing
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -2105.47f, 8587.09f, 18.22f, 0.03f);
            break;
        case 3055: // Altar of Sha'tar
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -3045.61f, 821.313f, -10.5034f, 5.7f);
            break;
        case 3065: // Bronzebeard Encampment
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -8029.3818359375f, 1104.29541015625f, 6.27918004989624f, 1);
            break;
        case 3070: // Cenarion Hold
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -6812.36767578125f, 803.790771484375f, 51.4796485900879f, 3.0f);
            break;
        case 3075: // Cenarion Refuge
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -201.32048034668f, 5551.98828125f, 23.3083515167236f, 6);
            break;
        case 3080: // Cosmowrench
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 2955.91845703125f, 1782.75708007813f, 143.437622070313f, 1);
            break;
        case 3085: // Emerald Sanctuary
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 4000.341796875f, -1295.60144042969f, 254.221771240234f, 4);
            break;
        case 3100: // Valor's Rest
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -6387.18f, -297.97f, -2.388f, 4.81f);
            break;
        case 3105: // Halaa
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -1603.86f, 7765.25f, -21.9298f, 1.55f);
            break;
        case 3110: // Harborage
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -10106.6181640625f, -2817.39013671875f, 22.005012512207f, 5);
            break;
        case 3115: // Kirin'Var Village
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 2124.54931640625f, 2302.89208984375f, 73.6288986206055f, 0);
            break;
        case 3120: // Light's Hope Chapel
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, 2287.22f, -5324.61f, 90.91f, 2.13f);
            break;
        case 3125: // Marshal's Refuge
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -6277.1f, -1126.65f, -224.935f, 3.97f);
            break;
        case 3135: // Midrealm Post
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 3401.38452148438f, 2922.79467773438f, 158.908935546875f, 6);
            break;
        case 3140: // Mirage Raceway
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -6202.0283203125f, -3902.23828125f, -60.2820510864258f, 0);
            break;
        case 3150: // Nesingwary's Expedition
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -11583.6962890625f, -38.9316635131836f, 11.0776624679565f, 5);
            break;
        case 3155: // Nighthaven
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 7796.24462890625f, -2574.78857421875f, 488.476989746094f, 0);
            break;
        case 3160: // Ogri'la
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 2310.12f, 7288.73f, 365.617f, 0.18f);
            break;
        case 3165: // Protectorate Watch Post
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 4247.81f, 2210.92f, 137.719f, 5.08f);
            break;
        case 3175: // Sanctum of the Stars
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -4076.57f, 1081.55f, 33.4521f, 1.93f);
            break;
        case 3180: // Sporeggar
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 224.467f, 8533.64f, 23.4459f, 2.64f);
            break;
        case 3185: // Steamwheedle Port
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -6933.19f, -4936.27f, 0.71469f, 1.47f);
            break;
        case 3190: // Stormspire
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 4200.9f, 3100.61f, 335.821f, 4.08f);
            break;
        case 3195: // Thorium Point
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -6501.11f, -1171.78f, 309.215f, 2.17f);
            break;
        case 3200: // Timbermaw Hold
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 6997.32f, -2106.95f, 588.566f, 4.93f);
            break;
        //////////////////////////////////////////////////KALIMDOR///////////////////////////////////////////////////////////////
        case 6001:// Blackfathom Deeps
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 4248.72f, 744.35f, -24.67f, 1.34f);
            break;
        case 6005:// Caverns of Time
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -8173.66f, -4746.36f, 33.84f, 4.94f);
            break;
        case 6010:// Dire Maul
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -3960.95f, 1130.64f, 161.05f, 0.0f);
            break;
        case 6015:// Maraudon
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -1431.33f, 2962.34f, 98.23f, 4.74f);
            break;
        case 6020:// Onyxia's Lair
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -4707.44f, -3726.82f, 54.6723f, 3.8f);
            break;
        case 6025:// Ragefire Chasm
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, 1814.47f, -4419.46f, -18.78f, 5.28f);
            break;
        case 6030:// Razorfen Downs
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -4657.88f, -2525.59f, 81.4f, 4.16f);
            break;
        case 6035:// Razorfen Kraul
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -4463.6f, -1664.53f, 82.26f, 0.85f);
            break;
        case 6040:// Ruins of Ahn'Qiraj
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -8413.33f, 1501.27f, 29.64f, 2.61f);
            break;
        case 6045:// Temple of Ahn'Qiraj
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -8245.837891f, 1983.736206f, 129.071686f, 0.936195f);
            break;
        case 6050:// Wailing Caverns
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -722.53f,-2226.30f,16.94f,2.71f);
            break;
        case 6055:// Zul'Farrak
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(1, -6801.9f, -2890.22f, 8.88f, 6.25f);
            break;
        //////////////////////////////////////////////////EASTERN KINGDOMS///////////////////////////////////////////////////////////////
        case 7001:// Blackrock Depths
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -7180.57, -920.04f, 165.49f, 5.02f);
            break;
        case 7005:// Blackrock Spire
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -7526.77f, -1225.64f, 285.73f, 5.31f);
            break;
        case 7010:// Blackwing Lair
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(469, -7672.61f, -1107.21f, 396.65f, 3.75f);
            break;
        case 7015:// Deadmines
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -11208.2f, 1675.92f, 24.57f, 1.48f);
            break;
        case 7020:// Gnomeregan
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -5163.32f, 927.18f, 257.158, 1.44f);
            break;
        case 7025:// Isle Of Quel'Danas
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 12589.5f, -6775.5f, 15.0916f, 3.02f);
            break;
        case 7030:// Karazhan
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -11119.6f, -2011.42f, 47.09f, 0.65f);
            break;
        case 7035:// Molten Core
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(230, 1114.85f, -457.76f, -102.81f, 3.83f);
            break;
        case 7040:// Scarlet Monastery
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, 2843.89f,-693.74f,139.32f,5.11f);
            break;
        case 7045:// Scholomance
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, 1273.06f, -2574.01f, 92.66f, 2.06f);
            break;
        case 7050:// Shadowfang Keep
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -239.54f, 1550.8f, 76.89f, 1.18f);
            break;
        case 7055:// Stratholme
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, 3370.76f, -3343.63f, 142.26f, 5.23f);
            break;
        case 7060:// Sunken Temple
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -10346.92f, -3851.90f, -43.41f, 6.09f);
            break;
        case 7065:// The Stockade
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -8766.89f, 844.6f, 88.43f, 0.69f);
            break;
        case 7070:// Uldaman
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -6070.72f, -2955.33f, 209.78f, 0.05f);
            break;
        case 7075:// Zul'Aman
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 6851.09f, -7979.71f, 183.54f, 4.72f);
            break;
        case 7080:// Zul'Gurub
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(0, -11916.4f, -1216.23f, 92.28f, 4.75f);
            break;
        //////////////////////////////////////////////////OUTLAND///////////////////////////////////////////////////////////////
        case 8001:// Auchindoun
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -3322.92f, 4931.02f, -100.56f, 1.86f);
            break;
        case 8005:// Black Temple
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -3649.1f, 317.33f, 35.19f, 2.97f);
            break;
        case 8010:// Coilfang Reservoir
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 721.08f, 6846.77f, -68.75f, 0.34f);
            break;
        case 8015:// Gruul's Lair
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 3539.01f, 5082.36f, 1.69f, 0.0f);
            break;
        case 8020:// Hellfire Citadel
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, -292.71f, 3146.77f, 31.60f, 2.05f);
            break;
        case 8025:// Tempest Keep
            Plr->CLOSE_GOSSIP_MENU();
            Plr->TeleportTo(530, 3087.62f, 1376.27f, 184.8f, 4.63f);
            break;    
    }
}

bool DeathSide_Npc_Teleporter_X(Player* Plr, Creature* pCreature)
 {
    Plr->PlayerTalkClass->ClearMenus();
    // Main Menu for Alliance
    if (Plr->GetTeam() == ALLIANCE)
        Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ALLIANCE_CITIES), GOSSIP_SENDER_MAIN, 1000);
    else // Main Menu for Horde
        Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_HORDE_CITIES), GOSSIP_SENDER_MAIN, 2000);
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEUTRAL_TOWNS), GOSSIP_SENDER_MAIN, 3000);
    Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_DUNGEONS), GOSSIP_SENDER_MAIN, 5000);
    Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());

    return true;
}

 bool DeathSide_Npc_Teleporter_X_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
 {
    // Main menu
    if (uiSender == GOSSIP_SENDER_MAIN)
    SendDefaultMenu_Npc_Teleport_X(pPlayer, pCreature, uiAction);
 
    return true;
 }

 void AddSC_DeathSide_Npc_Teleporter_X()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Npc_Teleporter_X";
     newscript->pGossipHello      = &DeathSide_Npc_Teleporter_X;
     newscript->pGossipSelect     = &DeathSide_Npc_Teleporter_X_Gossip;
     newscript->RegisterSelf();
 }