#include "precompiled.h"
#include "Language.h"

#define CLASS_MASTER 693128
#define PET_MASTER 693129

std::unordered_map<uint32, std::vector<uint32>> pets = {
    {16973, {LANG_SCRIPT_PET_VULTURE, 1}},
    {20998, {LANG_SCRIPT_PET_SPIDER, 1}},
    {20749, {LANG_SCRIPT_PET_L_SERPENT, 1}},
    {20728, {LANG_SCRIPT_PET_RAPTOR, 1}},
    {7456,  {LANG_SCRIPT_PET_OWL, 1}},
    {22100, {LANG_SCRIPT_PET_SCORPID, 1}},
    {16117, {LANG_SCRIPT_PET_BOAR, 1}},
    {21723, {LANG_SCRIPT_PET_WILD_CAT, 1}},
    {23219, {LANG_SCRIPT_PET_WARP_CHASER, 1}},
    {3107,  {16618, 1}}, //Crab  
    {17731, {16619, 1}}, //Nether Ray       
    {18128, {16620, 1}}, //Sporebat

    {2521, {LANG_SCRIPT_PET_GORILLA, 2}},
    {2753, {LANG_SCRIPT_PET_WOLF , 2}},
    {3653, {LANG_SCRIPT_PET_TURTLE, 2}},
    {4425, {LANG_SCRIPT_PET_BAT, 2}},
    {5762, {LANG_SCRIPT_PET_SERPENT, 2}},
    {14228,{LANG_SCRIPT_PET_HYENA, 2}},
    {15043,{LANG_SCRIPT_PET_CROCOLISK, 2}},
    {15650,{LANG_SCRIPT_PET_DRAGONHAWK, 2}},
    {16933,{LANG_SCRIPT_PET_RAVAGER, 2}},
    {17347,{16621, 2}}, //Bear    
    {3245, {16622, 2}}, //Tallstrider        
    {21042,{16623, 2}}, //Carrion Bird
};

bool trainer_master_Hello(Player* player, Creature* creature)
{
    if (player->IsInCombat())
    {
        player->CLOSE_GOSSIP_MENU();
        player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_IN_COMBAT));
        return true;
    }

    if (creature->GetEntry() == CLASS_MASTER)
    {
        // 1 Learn spells 1
        // 1 Reset talents 2
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_TRAIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(LANG_SCRIPT_RESET_TALENTS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    }
    else if (creature->GetEntry() == PET_MASTER)
    {        
        if (player->GetClass() != CLASS_HUNTER)
        {
            player->CLOSE_GOSSIP_MENU();
            player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_NOT_HUNTER));
            return true;
        }

        // 2 Tame new pet p1 4 
        // 2 Tame new pet p2 5
        // 2 Learn all pet spelld 6
        // 2 Reset pet talents 7
        // 2 Stables 8
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(16613), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(16614), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(16615), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
        player->ADD_GOSSIP_ITEM(2, player->GetSession()->GetHellgroundString(16616), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
    }

    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    return true;
}

bool trainer_master_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    if (player->IsInCombat())
    {
        player->CLOSE_GOSSIP_MENU();
        player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_IN_COMBAT));
        return true;
    }

    if (creature->GetEntry())
    {
        if (creature->GetEntry() == CLASS_MASTER)
        {
            switch (uiAction)
            {
                case GOSSIP_ACTION_INFO_DEF + 1:
                {
                    if (!player->LearnAllSpells(true, false))
                        ChatHandler(player).PSendSysMessage(16617);

                    player->CLOSE_GOSSIP_MENU();
                    break;
                }
                case GOSSIP_ACTION_INFO_DEF + 2:
                {
                    //player->resetTalents(true);
                    player->SendTalentWipeConfirm(creature->GetGUID());
                    player->CLOSE_GOSSIP_MENU();
                    break;
                }
            }
        }
        else if (creature->GetEntry() == PET_MASTER)
        {
            // tame new pet
            if (uiAction > GOSSIP_ACTION_1)
            {
                if (player->GetLevel() < 20)
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(16612));
                    return true;
                }
                
                if (player->GetClass() != CLASS_HUNTER)
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_ARE_NOT_HUNTER));
                    return true;
                }

                if (player->GetPet())
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(16610));
                    return true;
                }

                uiAction -= (uint32)GOSSIP_ACTION_1;

                Creature* NewPet = creature->SummonCreature(uiAction, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 100);
                if (!NewPet)
                    return false;

                NewPet->SetLevel(player->GetLevel());
                NewPet->SetVisibility(VISIBILITY_OFF);

                player->CastSpell(NewPet, 2650, true);

                player->CLOSE_GOSSIP_MENU();
                return true;
            }

            switch (uiAction)
            {
            // Tame new pet
            case GOSSIP_ACTION_INFO_DEF + 4:
            {
                for (const auto& pair : pets) {
                    uint32_t key = pair.first;
                    const std::vector<uint32_t>& values = pair.second;

                    // page
                    if (values[1] == 1)
                        player->ADD_GOSSIP_ITEM(7, player->GetSession()->GetHellgroundString(values[0]), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_1 + key);
                }

                player->ADD_GOSSIP_ITEM(7, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;
            }
            // Tame new pet
            case GOSSIP_ACTION_INFO_DEF + 5:
            {
                for (const auto& pair : pets) {
                    uint32_t key = pair.first;
                    const std::vector<uint32_t>& values = pair.second;

                    // page
                    if (values[1] == 2)
                        player->ADD_GOSSIP_ITEM(7, player->GetSession()->GetHellgroundString(values[0]), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_1 + key);
                }

                player->ADD_GOSSIP_ITEM(7, player->GetSession()->GetHellgroundString(LANG_SCRIPT_NEXT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;
            }
            // Learn pet spells
            case GOSSIP_ACTION_INFO_DEF + 6:
            {
                if (!player->GetPet())
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(16611));
                    return true;
                }
                
                if (!player->LearnAllSpells(true, true))
                    ChatHandler(player).PSendSysMessage(16617);

                player->CLOSE_GOSSIP_MENU();
                break;
            }
            // Reset pet talents
            case GOSSIP_ACTION_INFO_DEF + 7:
            {
                if (!player->GetPet())
                {
                    player->CLOSE_GOSSIP_MENU();
                    player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(16611));
                    return true;
                }
                
                player->SendPetSkillWipeConfirm();
                player->CLOSE_GOSSIP_MENU();
                break;
            }            
            // Stables
            case GOSSIP_ACTION_INFO_DEF + 8:
            {
                player->GetSession()->SendStablePet(creature->GetGUID());
                player->CLOSE_GOSSIP_MENU();
                break;
            }
            }
        }
    }

    return true;
}

void AddSC_DeathSide_Npc_Pet_Master()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "mw_trainer_master";
    newscript->pGossipHello = &trainer_master_Hello;
    newscript->pGossipSelect = &trainer_master_Gossip;
    newscript->RegisterSelf();
}