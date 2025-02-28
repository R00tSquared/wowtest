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
SDName: Feralas
SD%Complete: 100
SDComment: Quest support: 3520, 2767. Special vendor Gregan Brewspewer
SDCategory: Feralas
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"

/*######
## npc_gregan_brewspewer
######*/

#define GOSSIP_HELLO 16312

bool GossipHello_npc_gregan_brewspewer(Player* player, Creature* creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (creature->isVendor() && player->GetQuestStatus(3909) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HELLO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(2433, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_gregan_brewspewer(Player* player, Creature* creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU(2434, creature->GetGUID());
    }
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST(creature->GetGUID());
    return true;
}

/*######
## npc_screecher_spirit
######*/

bool GossipHello_npc_screecher_spirit(Player* player, Creature* creature)
{
    player->SEND_GOSSIP_MENU(2039, creature->GetGUID());
    player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    creature->ForcedDespawn(3000);

    return true;
}

/*######
## npc_oox22fe
######*/

enum oox22fe
{
    SAY_OOX_START         = -1060000,
    SAY_OOX_AGGRO1        = -1060001,
    SAY_OOX_AGGRO2        = -1060002,
    SAY_OOX_AMBUSH        = -1060003,
    SAY_OOX_END           = -1060005,

    NPC_YETI              = 7848,
    NPC_GORILLA           = 5260,
    NPC_WOODPAW_REAVER    = 5255,
    NPC_WOODPAW_BRUTE     = 5253,
    NPC_WOODPAW_ALPHA     = 5258,
    NPC_WOODPAW_MYSTIC    = 5254,

    QUEST_RESCUE_OOX22FE  = 2767
};

struct npc_oox22feAI : public npc_escortAI
{
    npc_oox22feAI(Creature* creature) : npc_escortAI(creature) {}

    void Reset() {}

    void EnterCombat(Unit* who)
    {
        switch (urand(0, 9))
        {
            case 0:
                DoScriptText(SAY_OOX_AGGRO1, me);
                break;
            case 1:
                DoScriptText(SAY_OOX_AGGRO2, me);
                break;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(me);
    }

    void WaypointReached(uint32 i)
    {
        switch (i)
        {
            case 11:
                DoScriptText(SAY_OOX_AMBUSH, me);
                break;
            case 12:
                me->SummonCreature(NPC_YETI, -4850.91f, 1595.67f, 72.99f, 2.85f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_YETI, -4851.76f, 1599.37f, 73.85f, 2.85f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_YETI, -4852.81f, 1592.63f, 72.28f, 2.85f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 23:
                DoScriptText(SAY_OOX_AMBUSH, me);
                break;
            case 24:
                me->SummonCreature(NPC_GORILLA, -4609.56f, 1991.80f, 57.24f, 3.74f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_GORILLA, -4613.00f, 1994.45f, 57.2f, 3.78f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_GORILLA, -4619.37f, 1998.11f, 57.72f, 3.84f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 31:
                DoScriptText(SAY_OOX_AMBUSH, me);
                break;
            case 32:
                me->SummonCreature(NPC_WOODPAW_REAVER, -4425.14f, 2075.87f, 47.77f, 3.77f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_WOODPAW_BRUTE , -4426.68f, 2077.98f, 47.57f, 3.77f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_WOODPAW_MYSTIC, -4428.33f, 2080.24f, 47.43f, 3.87f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_WOODPAW_ALPHA , -4430.04f, 2075.54f, 46.83f, 3.81f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 40:
                DoScriptText(SAY_OOX_END, me);
                if (Player* player = GetPlayerForEscort())
                    player->GroupEventHappens(QUEST_RESCUE_OOX22FE, me);
                break;
        }
    }
};

CreatureAI* GetAI_npc_oox22fe(Creature* creature)
{
    return new npc_oox22feAI(creature);
}

bool QuestAccept_npc_oox22fe(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_RESCUE_OOX22FE)
    {
        creature->SetStandState(UNIT_STAND_STATE_STAND);
        DoScriptText(SAY_OOX_START, creature);
        creature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
        if (npc_escortAI* EscortAI = CAST_AI(npc_oox22feAI, creature->AI()))
            EscortAI->Start(true, true, player->GetGUID());
    }
    return true;
}

/*####
# npc_kindal_moonriver
####*/

struct npc_kindal_moonriverAI : public npc_escortAI
{
    npc_kindal_moonriverAI(Creature *c) : npc_escortAI(c) {}
    
    uint32 CheckTimer;
    uint32 Counter;
    uint64 PlayerTarget;
    bool Check;
    bool Check2;

    void Reset()
    {
        CheckTimer = 0;
        Counter = 0;
        PlayerTarget = 0;
        Check = false;
        Check2 = false;
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();
        PlayerTarget = player->GetGUID();

        if (!player)
            return;

        switch(i)
        {
            case 0:
            {
                SetRun(true);
                me->SetSpeed(MOVE_RUN, 1);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                break;
            }
            case 9: 
            {
                SetEscortPaused(true);
                CheckTimer = 5000;
                Check = true;
                break;
            }
            case 14:
            {
                std::list<Creature*> CSpiteDarter = FindAllCreaturesWithEntry(7997, 15);
                for(std::list<Creature *>::iterator i = CSpiteDarter.begin(); i != CSpiteDarter.end(); i++)
                {
                    Counter++;
                }
                break;
            }
        }
    }
    
    void UpdateEscortAI(const uint32 diff)
    {
        if(Check)
        {
            if(CheckTimer < diff)
            {
                if(GameObject* pGo = FindGameObject(143979, 15, m_creature))
                {
                    if (pGo->GetGoState() == GO_STATE_ACTIVE)
                    {
                        std::list<Creature*> CSpiteDarter = FindAllCreaturesWithEntry(7997, 17);
                        for(std::list<Creature *>::iterator i = CSpiteDarter.begin(); i != CSpiteDarter.end(); i++)
                        {
                            (*i)->GetMotionMaster()->MoveFollow(me, urand(3,6), urand(M_PI, M_PI/4));
                        }
                        Check = false;
                        SetEscortPaused(false);
                        CheckTimer = 0;
                    }
                }
            } else CheckTimer -= diff;
        }
        if ((Counter >= 6) && (!Check2))
        {
            if(Player* player = (Player*)(me->GetUnit(PlayerTarget)))
                player->AreaExploredOrEventHappens(2969);
            std::list<Creature*> CSpiteDarter = FindAllCreaturesWithEntry(7997, 25);
            for(std::list<Creature *>::iterator i = CSpiteDarter.begin(); i != CSpiteDarter.end(); i++)
            {
                (*i)->DisappearAndDie();
            }
            Check2 = true;
        }
    }
};

CreatureAI* GetAI_npc_kindal_moonriver(Creature *_Creature)
{
    npc_kindal_moonriverAI* kindal_moonriver = new npc_kindal_moonriverAI(_Creature);
    kindal_moonriver->AddWaypoint(0, -4481.669434, 858.596619, 74.547379);
    kindal_moonriver->AddWaypoint(1, -4483.198242, 862.926147, 74.732735);
    kindal_moonriver->AddWaypoint(2, -4490.454102, 867.182739, 82.75333);
    kindal_moonriver->AddWaypoint(3, -4516.172852, 866.819824, 81.190041);
    kindal_moonriver->AddWaypoint(4, -4556.539551, 890.977356, 57.835964);
    kindal_moonriver->AddWaypoint(5, -4572.731934, 878.130676, 63.139954);
    kindal_moonriver->AddWaypoint(6, -4560.463379, 840.006653, 60.058517);
    kindal_moonriver->AddWaypoint(7, -4554.316406, 808.883911, 60.997368);
    kindal_moonriver->AddWaypoint(8, -4537.373535, 802.671387, 60.227024);
    kindal_moonriver->AddWaypoint(9, -4531.855469, 807.203796, 59.888496);
    kindal_moonriver->AddWaypoint(10, -4537.373535, 802.671387, 60.227024); // go back
    kindal_moonriver->AddWaypoint(11, -4554.316406, 808.883911, 60.997368);
    kindal_moonriver->AddWaypoint(12, -4554.316406, 808.883911, 60.997368);
    kindal_moonriver->AddWaypoint(13, -4560.463379, 840.006653, 60.058517);
    kindal_moonriver->AddWaypoint(14, -4572.731934, 878.130676, 63.139954);
    kindal_moonriver->AddWaypoint(15, -4556.539551, 890.977356, 57.835964);
    kindal_moonriver->AddWaypoint(16, -4516.172852, 866.819824, 81.190041);
    kindal_moonriver->AddWaypoint(17, -4490.454102, 867.182739, 82.75333);
    kindal_moonriver->AddWaypoint(18, -4483.198242, 862.926147, 74.732735);
    kindal_moonriver->AddWaypoint(19, -4481.669434, 858.596619, 74.547379);
    return (CreatureAI*)kindal_moonriver;
}

bool QuestAccept_npc_kindal_moonriver(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 2969)
        ((npc_escortAI*)creature->AI())->Start(true, false, player->GetGUID(), quest, true);
    return true;
}

enum
{
    SAY_ESCORT_START                    = -1001106,
    SAY_WANDER_1                        = -1001107,
    SAY_WANDER_2                        = -1001108,
    SAY_WANDER_3                        = -1001109,
    SAY_WANDER_4                        = -1001110,
    SAY_WANDER_DONE_1                   = -1001111,
    SAY_WANDER_DONE_2                   = -1001112,
    SAY_WANDER_DONE_3                   = -1001113,
    EMOTE_WANDER                        = -1001114,
    SAY_EVENT_COMPLETE_1                = -1001115,
    SAY_EVENT_COMPLETE_2                = -1001116,

    SPELL_SHAYS_BELL                    = 11402,
    NPC_ROCKBITER                       = 7765,
    QUEST_ID_WANDERING_SHAY             = 2845,
};

struct npc_shay_leafrunnerAI : public FollowerAI
{
    npc_shay_leafrunnerAI(Creature* pCreature) : FollowerAI(pCreature)
    {
        WanderTimer = 0;
        Reset();
    }

    uint32 WanderTimer;
    bool IsRecalled;
    bool IsComplete;

    void Reset()
    {
        IsRecalled = false;
        IsComplete = false;
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!IsComplete && pWho->GetEntry() == NPC_ROCKBITER && m_creature->IsWithinDistInMap(pWho, 20.0f))
        {
            Player* pPlayer = GetLeaderForFollower();
            if (!pPlayer)
                return;

            DoScriptText(SAY_EVENT_COMPLETE_1, m_creature);
            DoScriptText(SAY_EVENT_COMPLETE_2, pWho);

            // complete quest
            pPlayer->GroupEventHappens(QUEST_ID_WANDERING_SHAY, m_creature);
            SetFollowComplete(true);
            m_creature->ForcedDespawn(30000);
            IsComplete = true;
            WanderTimer = 0;

            // move to Rockbiter
            float fX, fY, fZ;
            pWho->GetNearPoint(fX, fY, fZ, INTERACTION_DISTANCE);
            m_creature->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
        }
        else if (IsRecalled && pWho->GetTypeId() == TYPEID_PLAYER && pWho->IsWithinDistInMap(pWho, INTERACTION_DISTANCE))
        {
            WanderTimer = 60000;
            IsRecalled = false;

            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_WANDER_DONE_1, m_creature); break;
                case 1: DoScriptText(SAY_WANDER_DONE_2, m_creature); break;
                case 2: DoScriptText(SAY_WANDER_DONE_3, m_creature); break;
            }
        }
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_SHAYS_BELL)
        {
            // resume following
            IsRecalled = true;
            SetFollowPaused(false);
        }
    }

    void UpdateFollowerAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;

        if (WanderTimer)
        {
            if (WanderTimer <= uiDiff)
            {
                // set follow paused and wander in a random point
                SetFollowPaused(true);
                DoScriptText(EMOTE_WANDER, m_creature);
                WanderTimer = 0;

                switch (urand(0, 3))
                {
                    case 0: DoScriptText(SAY_WANDER_1, m_creature); break;
                    case 1: DoScriptText(SAY_WANDER_2, m_creature); break;
                    case 2: DoScriptText(SAY_WANDER_3, m_creature); break;
                    case 3: DoScriptText(SAY_WANDER_4, m_creature); break;
                }
                m_creature->GetMotionMaster()->MoveRandomAroundPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 20.0f);
            }
            else
                WanderTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_shay_leafrunner(Creature* pCreature)
{
    return new npc_shay_leafrunnerAI(pCreature);
}

bool QuestAccept_npc_shay_leafrunner(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_WANDERING_SHAY)
    {
        DoScriptText(SAY_ESCORT_START, pCreature);
        if (npc_shay_leafrunnerAI* pShayAI = CAST_AI(npc_shay_leafrunnerAI, pCreature->AI()))
        {
            pShayAI->StartFollow(pPlayer, FACTION_ESCORT_N_FRIEND_PASSIVE, pQuest);
            pShayAI->WanderTimer = 30000;
        }
    }
    return true;
}

/*######
## AddSC
######*/

void AddSC_feralas()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_gregan_brewspewer";
    newscript->pGossipHello = &GossipHello_npc_gregan_brewspewer;
    newscript->pGossipSelect = &GossipSelect_npc_gregan_brewspewer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_screecher_spirit";
    newscript->pGossipHello = &GossipHello_npc_screecher_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_oox22fe";
    newscript->GetAI = &GetAI_npc_oox22fe;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_oox22fe;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_kindal_moonriver";
    newscript->GetAI = &GetAI_npc_kindal_moonriver;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kindal_moonriver;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_shay_leafrunner";
    newscript->GetAI = &GetAI_npc_shay_leafrunner;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_shay_leafrunner;
    newscript->RegisterSelf();
}

