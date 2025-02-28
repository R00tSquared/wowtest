// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "Language.h"
#include "BattleGroundMgr.h"

bool MW_bg_reg_anywhere_Hello(Player* pPlayer, Creature* pCreature)
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

    return true;
}

bool MW_bg_reg_anywhere_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 9:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_WS))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_WS); // WSG
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 10:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AB))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AB); // AB
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 11:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_EY))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_EY); // EYE
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 12:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AV))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AV); // AV
            break;
        }
    }
    return true;
}

void AddSC_MW_bg_reg_anywhere()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "MW_bg_reg_anywhere";
    newscript->pGossipHello          = &MW_bg_reg_anywhere_Hello;
    newscript->pGossipSelect          = &MW_bg_reg_anywhere_Gossip;
    newscript->RegisterSelf();
}
