// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Swamp_of_Sorrows
SD%Complete: 100
SDComment: Quest support: 1393
SDCategory: Swap of Sorrows
EndScriptData */

/* ContentData
npc_galen_goodward
EndContentData */

/*#########
##npc_galen_goodward
#########*/

#include "precompiled.h"
#include "escort_ai.h"

enum eGalen
{
    GILAN_SAY_START_1            = -1780131,
    GILAN_SAY_START_2            = -1780132,
    GILAN_SAY_UNDER_ATTACK_1     = -1780133,
    GILAN_SAY_UNDER_ATTACK_2     = -1780134,
    GILAN_SAY_END                = -1780135,
    GILAN_EMOTE_END_1            = -1780136,
    GILAN_EMOTE_END_2            = -1780137,

    QUEST_GALENS_ESCAPE           = 1393,
    GO_GALENS_CAGE                = 37118
};

struct npc_galen_goodwardAI : public npc_escortAI
{
    npc_galen_goodwardAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint32 m_uiPostEventTimer;

    void Reset() 
    { 
        m_uiPostEventTimer = 0;
    }

    void EnterCombat(Unit* )
    {
        DoScriptText(RAND(GILAN_SAY_UNDER_ATTACK_1,GILAN_SAY_UNDER_ATTACK_2), me);
    }

    void WaypointReached(uint32 uiPointId)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch(uiPointId)
        {
        case 1:
            DoScriptText(GILAN_SAY_START_2, me);
            break;
        case 16:            
            m_uiPostEventTimer = 10000;
            DoScriptText(GILAN_SAY_END, me, pPlayer);
            SetRun(true);
            if (Player* pPlayer = GetPlayerForEscort())
                pPlayer->GroupEventHappens(QUEST_GALENS_ESCAPE, me);
            break;
        case 17:
            DoScriptText(GILAN_EMOTE_END_2, me, pPlayer);
            break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
        {
            if (m_uiPostEventTimer && m_uiPostEventTimer <= uiDiff)
            {
                if (!me->GetVictim() && me->isAlive())
                {
                    Player* pPlayer = GetPlayerForEscort();

                    DoScriptText(GILAN_EMOTE_END_1, me, pPlayer);
                    Reset();
                    return;
                }

            } else m_uiPostEventTimer -= uiDiff;

            return;
        }

        DoMeleeAttackIfReady();
    }

};

bool QuestAccept_npc_galen_goodward(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GALENS_ESCAPE)
    {
        if (GameObject* pGo =FindGameObject(GO_GALENS_CAGE, INTERACTION_DISTANCE, pCreature))
            pGo->UseDoorOrButton();

        pCreature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
        pCreature->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
        pCreature->SetAggroRange(10.0);
        DoScriptText(GILAN_SAY_START_1, pCreature);

        if (npc_galen_goodwardAI* pEscortAI = CAST_AI(npc_galen_goodwardAI,pCreature->AI()))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest, true);
    }
    return true;
}
CreatureAI* GetAI_npc_galen_goodward(Creature *pCreature)
{
    return new npc_galen_goodwardAI(pCreature);
}

bool GossipHello_npc_5353(Player *player, Creature *creature)
{
    if(creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16207), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(1995, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_5353(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16208), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(1997, creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+2:
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16209), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(1998, creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+3:
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16210), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(1999, creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+4:
        {
            if(player->GetQuestStatus(3373) == QUEST_STATUS_COMPLETE)
                player->CastSpell(player, 12578, false);
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        default: break;
    }
    return true;
}

void AddSC_swamp_of_sorrows()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_galen_goodward";
    newscript->GetAI = &GetAI_npc_galen_goodward;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_galen_goodward;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_5353";
    newscript->pGossipHello = &GossipHello_npc_5353;
    newscript->pGossipSelect = &GossipSelect_npc_5353;
    newscript->RegisterSelf();
}
