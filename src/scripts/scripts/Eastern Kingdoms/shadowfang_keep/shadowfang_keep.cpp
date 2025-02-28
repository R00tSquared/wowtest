// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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
SDName: Shadowfang_Keep
SD%Complete: 75
SDComment: npc_shadowfang_prisoner using escortAI for movement to door. Might need additional code in case being attacked. Add proper texts/say().
SDCategory: Shadowfang Keep
EndScriptData */

/* ContentData
npc_shadowfang_prisoner
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "def_shadowfang_keep.h"

/*######
## npc_shadowfang_prisoner
######*/

enum eEnums
{
    SAY_FREE_AS             = -1033000,
    SAY_OPEN_DOOR_AS        = -1033001,
    SAY_POST_DOOR_AS        = -1033002,
    SAY_FREE_AD             = -1033003,
    SAY_OPEN_DOOR_AD        = -1033004,
    SAY_POST1_DOOR_AD       = -1033005,
    SAY_POST2_DOOR_AD       = -1033006,
    SAY_RETH_DEATH_REACT_1  = -1966603,
    SAY_RETH_DEATH_REACT_2  = -1966604,

    SPELL_UNLOCK            = 6421,
    SPELL_ASHCROMBE_TP      = 6422,
    NPC_ASH                 = 3850,

    SPELL_DARK_OFFERING     = 7154,
    SPELL_SLICE_AND_DICE    = 6434,
    SPELL_GOUGE             = 13579,
    SPELL_FROST_NOVA        = 122,
    SPELL_FLAMESTRIKE       = 2120
};

#define GOSSIP_ITEM_DOOR        16195

struct npc_shadowfang_prisonerAI : public npc_escortAI
{
    npc_shadowfang_prisonerAI(Creature *c) : npc_escortAI(c)
    {
        pInstance = (c->GetInstanceData());
        uiNpcEntry = c->GetEntry();
        eventPassed = false;
    }

    ScriptedInstance *pInstance;
    uint32 uiNpcEntry;
    Timer EventTimer;
    Timer SayTimer;
    uint8 sayPhase;
    bool eventPassed;
   
    Timer GougeTimer;
    Timer FrostNovaTimer;
    Timer FlameStrikeTimer;

    void WaypointReached(uint32 uiPoint)
    {
        switch(uiPoint)
        {
            case 0:
                if (uiNpcEntry == NPC_ASH)
                    DoScriptText(SAY_FREE_AS, me);
                else
                    DoScriptText(SAY_FREE_AD, me);
                break;
            case 10:
                if (uiNpcEntry == NPC_ASH)
                    DoScriptText(SAY_OPEN_DOOR_AS, me);
                else
                    DoScriptText(SAY_OPEN_DOOR_AD, me);
                break;
            case 11:
                if (uiNpcEntry == NPC_ASH)
                    DoCast(me, SPELL_UNLOCK);
				else
					me->HandleEmoteCommand(EMOTE_STATE_USESTANDING);
                break;
            case 12:
                if (uiNpcEntry == NPC_ASH)
                    DoScriptText(SAY_POST_DOOR_AS, me);
                else
                    DoScriptText(SAY_POST1_DOOR_AD, me);

                if (pInstance)
                    pInstance->SetData(TYPE_FREE_NPC, DONE);
                break;
            case 13:
                if (uiNpcEntry == NPC_ASH)
                    DoCast(me, SPELL_ASHCROMBE_TP);
                else
                    DoScriptText(SAY_POST2_DOOR_AD, me);
				    me->SetWalk(false);
                break;

					
        }
    }

    void Reset() 
    {
        EventTimer.Reset(3000);
        SayTimer.Reset(0);

        FrostNovaTimer.Reset(1000);
        FlameStrikeTimer.Reset(3000);

        GougeTimer.Reset(5000);

        sayPhase = 0;
        m_creature->SetReactState(REACT_PASSIVE);
    }

    void EnterCombat(Unit* /*who*/) 
    {
        if (m_creature->GetReactState() == REACT_PASSIVE)
            m_creature->SetReactState(REACT_AGGRESSIVE);
        
        if (uiNpcEntry != NPC_ASH)
        {
            m_creature->CastSpell(m_creature, SPELL_SLICE_AND_DICE, true);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (uiNpcEntry == NPC_ASH)
        {
            if (!eventPassed)
            {
                if (EventTimer.Expired(diff))
                {
                    if (Creature* bossRethilgore = (Creature*)FindCreature(3914, 75, m_creature))
                    {
                        if (!bossRethilgore->isAlive())
                        {
                            SayTimer = 1000;
                            EventTimer = 0;
                            return;
                        }
                    }

                    EventTimer = 3000;
                }

                if (SayTimer.Expired(diff))
                {
                    switch(sayPhase)
                    {
                        case 0:
                            if (Creature* npc_3849 = (Creature*)FindCreature(3849, 25, m_creature))
                                DoScriptText(SAY_RETH_DEATH_REACT_1, npc_3849);

                            SayTimer = 2000;
                            sayPhase++;
                        break;
                        case 1:
                            if (Creature* npc_3850 = (Creature*)FindCreature(3850, 25, m_creature))
                                DoScriptText(SAY_RETH_DEATH_REACT_2, npc_3850);
                                
                            SayTimer = 0;
                            eventPassed = true;
                        break;
                    }
                }
            }
        }

        npc_escortAI::UpdateAI(diff);

        if (!UpdateVictim())
           return;

        
        if (uiNpcEntry == NPC_ASH)
        {
            if (FrostNovaTimer.Expired(diff))
            {
                m_creature->CastSpell(m_creature, SPELL_FROST_NOVA, false);

                FrostNovaTimer = 0;
            }

            if (FlameStrikeTimer.Expired(diff))
            {
                m_creature->CastSpell(m_creature->GetVictim(), SPELL_FLAMESTRIKE, false);

                FlameStrikeTimer = 8000;
            }
        }
        else
        {
            if (GougeTimer.Expired(diff))
            {
                m_creature->CastSpell(m_creature->GetVictim(), SPELL_GOUGE, true);

                GougeTimer = urand(10000, 15000);
            }
        }
        
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_shadowfang_prisoner(Creature* pCreature)
{
    return new npc_shadowfang_prisonerAI(pCreature);
}

bool GossipHello_npc_shadowfang_prisoner(Player* pPlayer, Creature* pCreature)
{
    ScriptedInstance* pInstance = (pCreature->GetInstanceData());

    if (pInstance && pInstance->GetData(TYPE_FREE_NPC) != DONE && pInstance->GetData(TYPE_RETHILGORE) == DONE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_DOOR), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_shadowfang_prisoner(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (npc_escortAI* pEscortAI = CAST_AI(npc_shadowfang_prisonerAI, pCreature->AI()))
        {
            pEscortAI->Start(false, false);
        }
    }
    return true;
}

struct npc_arugal_voidwalkerAI : public ScriptedAI
{
    npc_arugal_voidwalkerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (pCreature->GetInstanceData());;
    }

    ScriptedInstance* pInstance;

    uint32 uiDarkOffering;

    void Reset()
    {
        uiDarkOffering = urand(290,10);
    }

    void UpdateAI(uint32 const uiDiff)
    {
        if (!UpdateVictim())
            return;

        if (uiDarkOffering <= uiDiff)
        {
            if (Creature* pFriend = GetClosestCreatureWithEntry(me, me->GetEntry(), 25.0))
            {
                if (pFriend)
                    DoCast(pFriend,SPELL_DARK_OFFERING);
            }
            else
                DoCast(me,SPELL_DARK_OFFERING);
            uiDarkOffering = urand(4400,12500);
        } else uiDarkOffering -= uiDiff;

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (pInstance)
            pInstance->SetData(TYPE_FENRUS, pInstance->GetData(TYPE_FENRUS) + 1);
    }
};

CreatureAI* GetAI_npc_arugal_voidwalker(Creature* pCreature)
{
    return new npc_arugal_voidwalkerAI(pCreature);
}

void AddSC_shadowfang_keep()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_shadowfang_prisoner";
    newscript->pGossipHello =  &GossipHello_npc_shadowfang_prisoner;
    newscript->pGossipSelect = &GossipSelect_npc_shadowfang_prisoner;
    newscript->GetAI = &GetAI_npc_shadowfang_prisoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_arugal_voidwalker";
    newscript->GetAI = &GetAI_npc_arugal_voidwalker;
    newscript->RegisterSelf();
}
