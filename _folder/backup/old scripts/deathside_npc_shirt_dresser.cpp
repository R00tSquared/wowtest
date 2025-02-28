// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

void SendDefaultMenu_npc_shirt_dresser(Player* Plr, Creature* pCreature, uint32 action)
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
        case 1000: //Great Shirt
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Humans]", GOSSIP_SENDER_MAIN, 1001);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Gnomes]", GOSSIP_SENDER_MAIN, 1002);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Draenei]", GOSSIP_SENDER_MAIN, 1003);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Night Elves]", GOSSIP_SENDER_MAIN, 1004);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Dwarfs]", GOSSIP_SENDER_MAIN, 1005);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Orcs]", GOSSIP_SENDER_MAIN, 1006);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Blood Elves]", GOSSIP_SENDER_MAIN, 1007);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Taurens]", GOSSIP_SENDER_MAIN, 1008);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Trolls]", GOSSIP_SENDER_MAIN, 1009);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
             Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 2000: //Shirt - different prices
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 10000 золота", GOSSIP_SENDER_MAIN, 1000);            
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 8000 золота", GOSSIP_SENDER_MAIN, 4000);
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 7000 золота", GOSSIP_SENDER_MAIN, 5000);
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 6000 золота", GOSSIP_SENDER_MAIN, 6000);
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 5000 золота", GOSSIP_SENDER_MAIN, 7000);
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 4000 золота", GOSSIP_SENDER_MAIN, 8000);
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 3000 золота", GOSSIP_SENDER_MAIN, 9000);
            Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 2000 золота", GOSSIP_SENDER_MAIN, 10000);
             Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 4000: //Shirts for 8000 gold
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Theldelor]", GOSSIP_SENDER_MAIN, 4001);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 5000: //Shirts for 7000 gold
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Stormwind Guard]", GOSSIP_SENDER_MAIN, 5001);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Orgrimmar Guard]", GOSSIP_SENDER_MAIN, 5002);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 6000: //Shirts for 6000 gold
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Illidan]", GOSSIP_SENDER_MAIN, 6001);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Maiev]", GOSSIP_SENDER_MAIN, 6002);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Netherkurse]", GOSSIP_SENDER_MAIN, 6003);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Lady Jaina]", GOSSIP_SENDER_MAIN, 6004);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Uther]", GOSSIP_SENDER_MAIN, 6005);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 7000: //Shirts for 5000 gold page 1
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Fel Commander]", GOSSIP_SENDER_MAIN, 7001);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Worgens]", GOSSIP_SENDER_MAIN, 7002);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Archimonde]", GOSSIP_SENDER_MAIN, 7003);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Infinite Dragon]", GOSSIP_SENDER_MAIN, 7004);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Seer Legion]", GOSSIP_SENDER_MAIN, 7005);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Time Warden]", GOSSIP_SENDER_MAIN, 7006);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Vindicator]", GOSSIP_SENDER_MAIN, 7007);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Tichondrius]", GOSSIP_SENDER_MAIN, 7008);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Alexei Barov]", GOSSIP_SENDER_MAIN, 7009);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Teron]", GOSSIP_SENDER_MAIN, 7010);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 7100);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 7100: //Shirts for 5000 gold page 2
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Attumen]", GOSSIP_SENDER_MAIN, 7011);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Pit Lord]", GOSSIP_SENDER_MAIN, 7012);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Rexxar]", GOSSIP_SENDER_MAIN, 7013);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Samuro]", GOSSIP_SENDER_MAIN, 7014);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Vol'jin]", GOSSIP_SENDER_MAIN, 7015);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Gul'dan]", GOSSIP_SENDER_MAIN, 7016);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 7000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 8000: //Shirts for 4000 gold
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Medivh]", GOSSIP_SENDER_MAIN, 8001);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Ethereal]", GOSSIP_SENDER_MAIN, 8002);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Doom Guard]", GOSSIP_SENDER_MAIN, 8003);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Butcher]", GOSSIP_SENDER_MAIN, 8004);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Unknown]", GOSSIP_SENDER_MAIN, 8005);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Nagas]", GOSSIP_SENDER_MAIN, 8006);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 9000: //Shirts for 3000 gold
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Goblin Agent]", GOSSIP_SENDER_MAIN, 9001);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Ogre Warrior]", GOSSIP_SENDER_MAIN, 9002);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Fallen]", GOSSIP_SENDER_MAIN, 9003);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Wrath Priestess]", GOSSIP_SENDER_MAIN, 9004);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Ogre Mage]", GOSSIP_SENDER_MAIN, 9005);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Dead]", GOSSIP_SENDER_MAIN, 9006);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Deputy]", GOSSIP_SENDER_MAIN, 9007);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Death Seeker]", GOSSIP_SENDER_MAIN, 9008);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Fel Soldier]", GOSSIP_SENDER_MAIN, 9009);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Razorfen Warrior]", GOSSIP_SENDER_MAIN, 9010);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, 9100);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 9100:
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Dalaran Wizard]", GOSSIP_SENDER_MAIN, 9011);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Vampire]", GOSSIP_SENDER_MAIN, 9012);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Flayer]", GOSSIP_SENDER_MAIN, 9013);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Zombie Alchemish]", GOSSIP_SENDER_MAIN, 9014);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Scarecrow]", GOSSIP_SENDER_MAIN, 9015);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Necromant]", GOSSIP_SENDER_MAIN, 9016);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Imp]", GOSSIP_SENDER_MAIN, 9017);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Lasher]", GOSSIP_SENDER_MAIN, 9018);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Ghoul]", GOSSIP_SENDER_MAIN, 9019);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, 9000);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        case 10000: //Shirts for 2000 gold
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Dryad]", GOSSIP_SENDER_MAIN, 10001);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Satyr]", GOSSIP_SENDER_MAIN, 10002);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Demon Hunter]", GOSSIP_SENDER_MAIN, 10003);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Diver]", GOSSIP_SENDER_MAIN, 10004);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Engineers]", GOSSIP_SENDER_MAIN, 10005);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Yeti]", GOSSIP_SENDER_MAIN, 10006);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Robot X012]", GOSSIP_SENDER_MAIN, 10007);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Zombie]", GOSSIP_SENDER_MAIN, 10008);
            Plr->ADD_GOSSIP_ITEM( 5, "[Shirt of Cursed Mage]", GOSSIP_SENDER_MAIN, 10009);
            Plr->ADD_GOSSIP_ITEM( 7, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, 2000);
            Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
            break;
        //////////////////////////////////////////////////Great Shirt///////////////////////////////////////////////////////////////
         
        case 1001: // Shirt of Humans
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33459, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1002: // Shirt of Gnomes
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33458, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1003: // Shirt  of Draenei
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33475, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1004: // Shirt of Night Elves
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33476, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1005: // hirt of Dwarfs
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33477, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1006: // Shirt of OrcsS
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33403, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1007: // Shirt of Blood Elves
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33402, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1008: // Shirt of Taurens
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33457, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 1009: // Shirt of Trolls
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33478, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 4001: // Shirt of Theldelor
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43636, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 5001: // Shirt of Stormwind Guard
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43629, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 5002: // Shirt of Orgrimmar Guard
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43630, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 6001: // Shirt of Illidan
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33436, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 6002: // Shirt of Maiev
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33446, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 6003: // Shirt of Netherkurse
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43627, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 6004: // Shirt of Lady Jaina
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43631, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 6005: // Shirt of Uther
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43634, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7001: // Shirt of Fel Commander
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33432, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7002: // Shirt of Worgens
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33411, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7003: // Shirt of Archimonde
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33471, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7004: // Shirt of Infinite Dragon
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33467, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7005: // Shirt of Seer Legion
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43910, true);
            Plr->CastSpell(Plr, 54827, true);
            break; 
        case 7006: // Shirt of Time Warden
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33448, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7007: // Shirt of Vindicator
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33451, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7008: // Shirt of Tichondrius
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33444, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7009: // Shirt of Alexei Barov
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43626, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7010: // Shirt of Teron
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43912, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7011: // Shirt of Attumen
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43913, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7012: // Shirt of Pit Lord
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43915, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7013: // Shirt of Rexxar
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43917, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7014: // Shirt of Samuro
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43632, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7015: // Shirt of Vol'jin
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43633, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 7016: // Shirt of Gul'dan
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43635, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 8001: // Shirt of Medivh
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33426, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 8002: // Shirt of Ethereal
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33434, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 8003: // Shirt of Doom Guard
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33468, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 8004: // Shirt of Butcher
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33442, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 8005: // Shirt of Unknown
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33443, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 8006: // Shirt of Nagas
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33474, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9001: // Shirt of Goblin Agent
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43909, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9002: // Shirt of Ogre Warrior
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33473, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9003: // Shirt of Fallen
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33433, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9004: // Shirt of Wrath Priestess
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33428, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9005: // Shirt of Ogre Mage
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33472, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9006: // Shirt of Dead
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33466, true);
            Plr->CastSpell(Plr, 54827, true);
            break; 
        case 9007: // Shirt of Deputy
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43908, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9008: // Shirt of Death Seeker
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33449, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9009: // Shirt of Fel Soldier
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33430, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9010: // Shirt of Razorfen Warrior
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43625, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9011: // Shirt of Dalaran Wizard
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43628, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9012: // Shirt of Vampire
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33450, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9013: // Shirt of Flayer
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33447, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9014: // Shirt of Zombie Alchemish
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33441, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9015: // Shirt of Scarecrow
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33437, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9016: // Shirt of Necromant
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33440, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9017: // Shirt of Imp
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43623, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9018: // Shirt of Lasher
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43914, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 9019: // Shirt of Ghoul
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43916, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10001: // Shirt of Dryad
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43907, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10002: // Shirt of Satyr
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33435, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10003: // Shirt of Demon Hunter
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33429, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10004: // Shirt of Diver
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33469, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10005: // Shirt of Engineers
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33470, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10006: // Shirt of Yeti
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33431, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10007: // Shirt of Robot X012
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33438, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10008: // Shirt of Zombie 
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 33439, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
        case 10009: // Shirt of Cursed Mage
            Plr->CLOSE_GOSSIP_MENU();
            Plr->CastSpell(Plr, 43911, true);
            Plr->CastSpell(Plr, 54827, true);
            break;
    }
}

bool DeathSide_npc_shirt_dresser(Player* Plr, Creature* pCreature)
 {
    Plr->PlayerTalkClass->ClearMenus();
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 10000 золота", GOSSIP_SENDER_MAIN, 1000);            
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 8000 золота", GOSSIP_SENDER_MAIN, 4000);
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 7000 золота", GOSSIP_SENDER_MAIN, 5000);
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 6000 золота", GOSSIP_SENDER_MAIN, 6000);
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 5000 золота", GOSSIP_SENDER_MAIN, 7000);
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 4000 золота", GOSSIP_SENDER_MAIN, 8000);
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 3000 золота", GOSSIP_SENDER_MAIN, 9000);
    Plr->ADD_GOSSIP_ITEM( 5, "Рубашки за 2000 золота", GOSSIP_SENDER_MAIN, 10000);
    Plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,pCreature->GetGUID());
    return true;
}

 bool DeathSide_npc_shirt_dresser_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
 {
    // Main menu
    if (uiSender == GOSSIP_SENDER_MAIN)
    SendDefaultMenu_npc_shirt_dresser(pPlayer, pCreature, uiAction);
 
    return true;
 }

 void AddSC_DeathSide_npc_shirt_dresser()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_npc_shirt_dresser";
     newscript->pGossipHello           = &DeathSide_npc_shirt_dresser;
     newscript->pGossipSelect  = &DeathSide_npc_shirt_dresser_Gossip;
     newscript->RegisterSelf();
 }