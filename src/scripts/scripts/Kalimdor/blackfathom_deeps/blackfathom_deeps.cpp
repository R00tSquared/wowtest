// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2010-2012 OregonCore <http://www.oregoncore.com/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2012 ScriptDev2 <http://www.scriptdev2.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"
#include "escort_ai.h"
#include "blackfathom_deeps.h"

enum eSpells
{
    SPELL_BLESSING_OF_BLACKFATHOM                           = 8733,
    SPELL_RAVAGE                                            = 8391,
    SPELL_FROST_NOVA                                        = 865,
    SPELL_FROST_BOLT_VOLLEY                                 = 8398,
    SPELL_TELEPORT_DARNASSUS                                = 9268
};

#define GOSSIP_ITEM_MORRIDUNE 16276

const Position HomePosition = Position(-815.817,-145.299,-25.870, 0);

bool GoHello_blackfathom_fire(Player * /*pPlayer*/, GameObject* pGo)
{
    ScriptedInstance *pInstance = pGo->GetInstanceData();

    if (pInstance)
    {
        pGo->SetGoState(GO_STATE_ACTIVE);
        pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
        pInstance->SetData(DATA_FIRE, pInstance->GetData(DATA_FIRE) + 1);
        return true;
    }
    return false;
}

struct npc_blackfathom_deeps_eventAI : public ScriptedAI
{
    npc_blackfathom_deeps_eventAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        if (pCreature->IsTemporarySummon())
        {
            pCreature->SetHomePosition(HomePosition.x, HomePosition.y, HomePosition.z, HomePosition.o);
            AttackPlayer();
        }

        pInstance = pCreature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint32 uiRavageTimer;
    uint32 uiFrostNovaTimer;
    uint32 uiFrostBoltVolleyTimer;

    bool bFlee;

    void Reset()
    {
        bFlee = false;

        uiRavageTimer           = urand(5000,8000);
        uiFrostNovaTimer        = urand(9000,12000);
        uiFrostBoltVolleyTimer  = urand(2000,4000);
    }

    void AttackPlayer()
    {
        Map::PlayerList const &PlList = me->GetMap()->GetPlayers();

        if (PlList.isEmpty())
            return;

        for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
        {
            if (Player* pPlayer = i->getSource())
            {
                if (pPlayer->isAlive())
                {
                    me->SetInCombatWith(pPlayer);
                    pPlayer->SetInCombatWith(me);
                    me->AddThreat(pPlayer, 0.0f);
                }
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;

        switch (me->GetEntry())
        {
            case NPC_AKU_MAI_SNAPJAW:
            {
                if (uiRavageTimer <= uiDiff)
                {
                    DoCast(me->GetVictim(), SPELL_RAVAGE);
                    uiRavageTimer = urand(9000,14000);
                } else uiRavageTimer -= uiDiff;
                break;
            }
            case NPC_MURKSHALLOW_SOFTSHELL:
            case NPC_BARBED_CRUSTACEAN:
            {
                if (!bFlee && HealthBelowPct(15))
                {
                    bFlee = true;
                    me->DoFleeToGetAssistance();
                }
                break;
            }
            case NPC_AKU_MAI_SERVANT:
            {
                if (uiFrostBoltVolleyTimer <= uiDiff)
                {
                    if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                    {
                        if (pTarget)
                            DoCast(pTarget, SPELL_FROST_BOLT_VOLLEY);
                    }
                    uiFrostBoltVolleyTimer = urand(5000,8000);
                } else uiFrostBoltVolleyTimer -= uiDiff;
                if (uiFrostNovaTimer <= uiDiff)
                {
                    DoCastAOE(SPELL_FROST_NOVA,false);
                    uiFrostNovaTimer = urand(25000,30000);
                } else uiFrostNovaTimer -= uiDiff;
                break;
            }
        }
        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (!me->IsTemporarySummon())
            return;
        if (pInstance)
            pInstance->SetData(DATA_EVENT, pInstance->GetData(DATA_EVENT) + 1);
    }
};

CreatureAI* GetAI_npc_blackfathom_deeps_event(Creature* pCreature)
{
    return new npc_blackfathom_deeps_eventAI (pCreature);
}

enum eMorridune
{
    SAY_MORRIDUNE_1 = -1048003,
    SAY_MORRIDUNE_2 = -1048004
};

struct npc_morriduneAI : public npc_escortAI
{
    npc_morriduneAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        DoScriptText(SAY_MORRIDUNE_1,pCreature);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        Start(false,false, 0, nullptr, false, false);
    }

    void WaypointReached(uint32 uiPoint)
    {
        switch(uiPoint)
        {
            case 4:
                SetEscortPaused(true);
                me->SetOrientation(1.775791);
                me->SetFacingTo(1.775791);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                DoScriptText(SAY_MORRIDUNE_2,me);
                break;
        }
    }
};

CreatureAI* GetAI_npc_morridune(Creature* pCreature)
{
    return new npc_morriduneAI (pCreature);
}

bool GossipHello_npc_morridune(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_MORRIDUNE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_morridune(Player* pPlayer, Creature* /*pCreature*/, uint32 /*uiSender*/, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->TeleportTo(1,9952.239,2284.277,1341.394,1.595);
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}

void AddSC_blackfathom_deeps()
{
    Script* newscript = new Script;

    newscript->Name = "go_blackfathom_fire";
    newscript->pGOUse = &GoHello_blackfathom_fire;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_blackfathom_deeps_event";
    newscript->GetAI = &GetAI_npc_blackfathom_deeps_event;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_morridune";
    newscript->GetAI = &GetAI_npc_morridune;
    newscript->pGossipHello = &GossipHello_npc_morridune;
    newscript->pGossipSelect = &GossipSelect_npc_morridune;
    newscript->RegisterSelf();
}
