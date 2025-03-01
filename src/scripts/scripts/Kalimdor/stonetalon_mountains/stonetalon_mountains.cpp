// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Stonetalon_Mountains
SD%Complete: 95
SDComment: Quest support: 6627, 6523
SDCategory: Stonetalon Mountains
EndScriptData */

/* ContentData
npc_braug_dimspirit
npc_kaya_flathoof
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_braug_dimspirit
######*/

#define GOSSIP_HBD1 16374
#define GOSSIP_HBD2 16375
#define GOSSIP_HBD3 16376
#define GOSSIP_HBD4 16377
#define GOSSIP_HBD5 16378

bool GossipHello_npc_braug_dimspirit(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(6627) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HBD1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HBD2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HBD3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HBD4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HBD5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(5820, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(5819, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_braug_dimspirit(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player,6766,false);

    }
    if (action == GOSSIP_ACTION_INFO_DEF+2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->AreaExploredOrEventHappens(6627);
    }
    return true;
}

/*######
## npc_kaya_flathoof
######*/

#define SAY_START   -1000347
#define SAY_AMBUSH  -1000348
#define SAY_END     -1000349

#define QUEST_PK    6523
#define MOB_GB      11912
#define MOB_GR      11910
#define MOB_GS      11913

struct npc_kaya_flathoofAI : public npc_escortAI
{
    npc_kaya_flathoofAI(Creature* c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if(!player)
            return;

        switch(i)
        {
        case 22:
            DoScriptText(SAY_AMBUSH, m_creature);
            m_creature->SummonCreature(MOB_GB, -48.53, -503.34, -46.31, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            m_creature->SummonCreature(MOB_GR, -38.85, -503.77, -45.90, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            m_creature->SummonCreature(MOB_GS, -36.37, -496.23, -45.71, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            break;
        case 23: m_creature->SetInFront(player);
            DoScriptText(SAY_END, m_creature, player);
            player->GroupEventHappens(QUEST_PK, m_creature);
            break;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void Reset(){}
};

bool QuestAccept_npc_kaya_flathoof(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_PK)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_kaya_flathoofAI, creature->AI()))
            pEscortAI->Start(true, true, player->GetGUID(), quest);

        DoScriptText(SAY_START, creature);
        creature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
        creature->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
    }
    return true;
}

CreatureAI* GetAI_npc_kaya_flathoofAI(Creature *_Creature)
{
    npc_kaya_flathoofAI* thisAI = new npc_kaya_flathoofAI(_Creature);

    thisAI->AddWaypoint(0, 122.37, -345.80, 3.59);
    thisAI->AddWaypoint(1, 113.69, -350.01, 4.54);
    thisAI->AddWaypoint(2, 107.32, -353.09, 3.33);
    thisAI->AddWaypoint(3, 99.25, -342.43, 2.87);
    thisAI->AddWaypoint(4, 111.19, -315.60, 3.71);
    thisAI->AddWaypoint(5, 109.99, -293.92, 5.16);
    thisAI->AddWaypoint(6, 104.59, -268.27, 4.78);
    thisAI->AddWaypoint(7, 82.80, -247.28, 5.73);
    thisAI->AddWaypoint(8, 66.44, -245.99, 5.85);
    thisAI->AddWaypoint(9, 34.36, -246.01, 5.97);
    thisAI->AddWaypoint(10, 13.24, -245.61, 5.25);
    thisAI->AddWaypoint(11, -10.27, -248.66, 4.69);
    thisAI->AddWaypoint(12, -26.07, -262.76, 0.01);
    thisAI->AddWaypoint(13, -33.15, -282.03, -4.12);
    thisAI->AddWaypoint(14, -28.42, -315.52, -8.56);
    thisAI->AddWaypoint(15, -32.05, -339.34, -10.84);
    thisAI->AddWaypoint(16, -35.22, -358.11, -16.20);
    thisAI->AddWaypoint(17, -51.57, -391.63, -24.85);
    thisAI->AddWaypoint(18, -58.58, -409.08, -29.97);
    thisAI->AddWaypoint(19, -60.37, -441.23, -36.80);
    thisAI->AddWaypoint(20, -59.03, -476.39, -44.98);
    thisAI->AddWaypoint(21, -53.18, -490.31, -46.11);
    thisAI->AddWaypoint(22, -43.77, -497.99, -46.13, 3000);// summon
    thisAI->AddWaypoint(23, -41.77, -518.15, -46.13, 5000);//end

    return (CreatureAI*)thisAI;
}

struct npc_4276AI : public ScriptedAI
{
    npc_4276AI(Creature *c) : ScriptedAI(c), Summons(c)
    {
        EventActive = false;
    }

    Timer SummonTimer;
    uint8 Phase;
    uint32 AttackersSlain;
    SummonList Summons;
    uint64 PlayerGUID;
    uint32 failResetTimer;
    bool EventActive;

    void Reset()
    {
        if(!EventActive)
        {
            SummonTimer.Reset(0);
            Phase = 0;
            AttackersSlain = 0;
            PlayerGUID = 0;
            failResetTimer = 0;
            EventActive = false;
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    }

    void JustSummoned(Creature *summoned)
    {
        Summons.Summon(summoned);
        summoned->AI()->AttackStart(me);
    }

    void StartEvent(Player* pPlayer)
    {
        PlayerGUID = pPlayer->GetGUID();
        SummonTimer = 1000;
        EventActive = true;
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* killer)
    {
        AttackersSlain++;
        if(AttackersSlain >= 8)
        {
            if(Player* plr = Unit::GetPlayerInWorld(PlayerGUID))
                plr->GroupEventHappens(1090, me);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            EventActive = false;
            Reset();
        }
    }

    void JustDied()
    {
        Summons.DespawnAll();
    }

    void UpdateAI(const uint32 diff)
    {
        if(SummonTimer.Expired(diff))
        {
            failResetTimer = 300000;
            switch(Phase)
            {
                case 0:
                    me->SummonCreature(3998, 945.64, -257.08, -3.09, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    me->SummonCreature(4001, 940.322, -255.836, -2.275, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    me->SummonCreature(3998, 937.578, -257.946, -2.304, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    SummonTimer = 60000;
                    Phase = 1;
                    break;
                case 1:
                    me->SummonCreature(3998, 935.544, -255.689, -2.408, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    me->SummonCreature(4001, 936.26, -257.875, -2.37, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    me->SummonCreature(4001, 936.26, -257.875, -2.37, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    SummonTimer = 60000;
                    Phase = 2;
                    break;
                case 2:
                    me->SummonCreature(3998, 936.26, -257.875, -2.37, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    me->SummonCreature(4003, 935.544, -255.689, -2.408, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    me->SummonCreature(4001, 936.26, -257.875, -2.37, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000);
                    SummonTimer = 0;
                    Phase = 0;
                    break;
                default: break;
            }
        }

        if (failResetTimer)
        {
            if (failResetTimer > diff)
                failResetTimer -= diff;
            else
            {
                EventActive = false;
                Reset();
                me->Kill(me);
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
   }
};

CreatureAI* GetAI_npc_4276(Creature* pCreature)
{
    return new npc_4276AI(pCreature);
}

bool QuestAccept_npc_4276(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if(pQuest->GetQuestId() == 1090)
    {
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        ((npc_4276AI*)pCreature->AI())->StartEvent(pPlayer);
    }
    return true;
}

/*######
## AddSC
######*/

void AddSC_stonetalon_mountains()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_braug_dimspirit";
    newscript->pGossipHello = &GossipHello_npc_braug_dimspirit;
    newscript->pGossipSelect = &GossipSelect_npc_braug_dimspirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kaya_flathoof";
    newscript->GetAI = &GetAI_npc_kaya_flathoofAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kaya_flathoof;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_4276";
    newscript->GetAI = &GetAI_npc_4276;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_4276;
    newscript->RegisterSelf();
}

