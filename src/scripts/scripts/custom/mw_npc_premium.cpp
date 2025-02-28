// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "Language.h"
#include "BattleGroundMgr.h"

bool MW_npc_premium_Hello(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->InBattleGroundOrArena())
    {
        ChatHandler(pPlayer).SendSysMessage(16553);
        return true;
    }
    
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_VENDOR_TRADE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
    if (!pPlayer->HasSpellCooldown(30524))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SUMMON_MAILBOX), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 20);
    else
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SUMMON_MAILBOX_COOLDOWN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_SHOW_BANK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_AUCTION_ALLIED), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 16);
    //pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_AUCTION_NEUTRAL), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 17);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_CREATE_REGISTER_ARENA_TEAM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    //pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_REGISTRATION_ARENA), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_REGISTRATION_BG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_TRAINER, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_RESET_TALENTS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_RESET_TALENTS_CONFIRM), 0, false);

    // robot info below
    char chrVisibility[256];
    sprintf(chrVisibility, pPlayer->GetSession()->GetHellgroundString(LANG_PREMIUM_VISIBILITY), 
        (pPlayer->GetSession()->IsAccountFlagged(ACC_PREMIUM_NPC_INVISIBLE) ? "" : pPlayer->GetSession()->GetHellgroundString(LANG_PREMIUM_VISIBILITY_OFF)));

    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, chrVisibility, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 19);

	if (pCreature->GetOwnerGUID() == pPlayer->GetGUID() && pCreature->isPet() && ((Pet*)pCreature)->getPetType() == MINI_PET)
	{
		char chr[128];
		uint32 premiumDurLeft = pPlayer->GetSession()->getPremiumDurationLeft();
		std::string timeStr = pPlayer->GetSession()->secondsToTimeString(premiumDurLeft, true, premiumDurLeft >= HOUR);
		sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_PREMIUM_DUR_LEFT), timeStr.c_str());
		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
	}

    pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_PREMIUM_UNSUMMON), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 18);

    // and also restore faction here after neutral auction (if used)
    if (pPlayer->getFaction() != pCreature->getFaction())
        pCreature->setFaction(pPlayer->getFaction());

    pPlayer->SEND_GOSSIP_MENU(68,pCreature->GetGUID());
    return true;
}

bool MW_npc_premium_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (pPlayer->InBattleGroundOrArena())
    {
        ChatHandler(pPlayer).SendSysMessage(16553);
        return true;
    }
    
    if (!(pCreature->GetOwnerGUID() == pPlayer->GetGUID() && pCreature->isPet() && ((Pet*)pCreature)->getPetType() == MINI_PET))
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_PREMIUM_NOT_OWNER), pPlayer->GetGUID());
        return true;
    }

    if (!pPlayer->GetSession()->isPremium())
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_PREMIUM_NOT_ACTIVE), pPlayer->GetGUID());
        pPlayer->RemoveMiniPet();
        return true;
    }

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            if (!sWorld.getConfig(CONFIG_BG_EVENTS_ENABLED))
            {
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                pPlayer->SEND_GOSSIP_MENU(68, pCreature->GetGUID());
            }
            else
            {
                char chr[256];
                bool ending = false;

                if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_WS))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15641), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
                }
                else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_WS))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15642), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
                    ending = true;
                }

                if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_AB))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15641), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                }
                else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_AB))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15642), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                    ending = true;
                }

                if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_EY))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15641), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                }
                else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_EY))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15642), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                    ending = true;
                }

                if (getBattleGroundMgr()->IsBGEventActive(BATTLEGROUND_AV))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15641), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                }
                else if (getBattleGroundMgr()->IsBGEventEnding(BATTLEGROUND_AV))
                {
                    sprintf(chr, pPlayer->GetSession()->GetHellgroundString(15642), pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                    ending = true;
                }

                if (ending)
                {
                    pPlayer->SEND_GOSSIP_MENU(68, pCreature->GetGUID());
                }
                else // only 1 variant - use it instantly
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    pPlayer->CLOSE_GOSSIP_MENU();

                    for (uint32 i = BATTLEGROUND_AV; i <= BATTLEGROUND_EY; ++i)
                    {
                        // skip arenas
                        if (i > BATTLEGROUND_AB && i < BATTLEGROUND_EY)
                            continue;

                        if (getBattleGroundMgr()->IsBGEventActive(BattleGroundTypeId(i)))
                        {
                            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BattleGroundTypeId(i)))
                                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BattleGroundTypeId(i));
                            break;
                        }
                    }
                }
            }
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+3:
        {
            pPlayer->GetSession()->SendPetitionShowList(pCreature->GetGUID()); // POKUPKA CHARTERA ARENI ILI REGISTRACIYA
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        //case GOSSIP_ACTION_INFO_DEF + 8:
        //{
        //    if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AA))
        //        pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AA); // Arena
        //    pPlayer->CLOSE_GOSSIP_MENU();
        //    break;
        //}
        case GOSSIP_ACTION_INFO_DEF + 9:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_WS))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_WS); // WSG
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 10:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AB))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AB); // AB
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 11:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_EY))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_EY); // EYE
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 12:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AV))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AV); // AV
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 14:
        {
            pPlayer->SEND_VENDORLIST(pCreature->GetGUID());
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 15:
        {
            pPlayer->SEND_BANKERLIST(pCreature->GetGUID());
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 17: // neutral auction
        {
            //pCreature->setFaction(855); // everlook. No other normal way to make one creature both neutral and factioned auctioneer
            // fall through
        }
        case GOSSIP_ACTION_INFO_DEF + 16: // ally auction
        {
            pPlayer->SEND_AUCTIONLIST(pCreature);
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 18: // unsummon
        {
            pPlayer->RemoveMiniPet();
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 19: // switch visibility
        {
            if (pPlayer->GetSession()->IsAccountFlagged(ACC_PREMIUM_NPC_INVISIBLE))
            {
                pPlayer->GetSession()->RemoveAccountFlag(ACC_PREMIUM_NPC_INVISIBLE);

                pCreature->SetVisibility(VISIBILITY_ON);
                // Invisibility visual
                pCreature->RemoveAurasDueToSpell(55315);
            }
            else
            {
                pPlayer->GetSession()->AddAccountFlag(ACC_PREMIUM_NPC_INVISIBLE);

                pCreature->SetVisibility(VISIBILITY_OFF);
                // Invisibility visual
                pCreature->CastSpell(pCreature, 55315, true);
            }

            MW_npc_premium_Hello(pPlayer, pCreature);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 20: // summon mailbox. Lasts 5 minutes. Cooldown 5 minutes
        {
            pPlayer->CastSpell(pPlayer, 30524, false);
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 21:
        {
            if (pPlayer->InArenaQueue())
            {
                ChatHandler(pPlayer).SendSysMessage(16636);
                return true;
            }
            
            pPlayer->resetTalents(true);
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 60: // go to main menu
        {
            MW_npc_premium_Hello(pPlayer, pCreature);
            break;
        }
        default:
        {
            char chrErr[256];
            sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), uiAction);
            pCreature->Whisper(chrErr, pPlayer->GetGUID());
            break;
        }
    }
    return true;
}

void AddSC_MW_npc_premium()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "MW_npc_premium";
    newscript->pGossipHello          = &MW_npc_premium_Hello;
    newscript->pGossipSelect          = &MW_npc_premium_Gossip;
    newscript->RegisterSelf();
}
