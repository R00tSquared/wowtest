// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

/* Script Data Start
SDName: Gnomeregan
SDAuthor: Manuel
SD%Complete: 90%
SDComment: Some visual effects are not implemented.
Script Data End */

#include "precompiled.h"
#include "gnomeregan.h"
#include "escort_ai.h"
#include "follower_ai.h"

#define GOSSIP_START_EVENT 16139

enum eBlastmasterEmiShortfuse
{
    GOSSIP_TEXT_EMI     = 1693,

    SAY_BLASTMASTER_0   = -1090000,
    SAY_BLASTMASTER_1   = -1090001,
    SAY_BLASTMASTER_2   = -1090002,
    SAY_BLASTMASTER_3   = -1090003,
    SAY_BLASTMASTER_4   = -1090004,
    SAY_BLASTMASTER_5   = -1090005,
    SAY_BLASTMASTER_6   = -1090006,
    SAY_BLASTMASTER_7   = -1090007,
    SAY_BLASTMASTER_8   = -1090008,
    SAY_BLASTMASTER_9   = -1090009,
    SAY_BLASTMASTER_10  = -1090010,
    SAY_BLASTMASTER_11  = -1090011,
    SAY_BLASTMASTER_12  = -1090012,
    SAY_BLASTMASTER_13  = -1090013,
    SAY_BLASTMASTER_14  = -1090014,
    SAY_BLASTMASTER_15  = -1090015,
    SAY_BLASTMASTER_16  = -1090016,
    SAY_BLASTMASTER_17  = -1090017,
    SAY_BLASTMASTER_18  = -1090018,
    SAY_BLASTMASTER_19  = -1090019,
    SAY_BLASTMASTER_20  = -1090020,
    SAY_BLASTMASTER_21  = -1090021,
    SAY_BLASTMASTER_22  = -1090022,
    SAY_BLASTMASTER_23  = -1090023,
    SAY_BLASTMASTER_24  = -1090024,
    SAY_BLASTMASTER_25  = -1090025,
    SAY_BLASTMASTER_26  = -1090026,
    SAY_BLASTMASTER_27  = -1090027,

    SAY_GRUBBIS         = -1090028
};

const float SpawnPosition[][4] =
{
    {-557.630,-114.514,-152.209,0.641},
    {-555.263,-113.802,-152.737,0.311},
    {-552.154,-112.476,-153.349,0.621},
    {-548.692,-111.089,-154.090,0.621},
    {-546.905,-108.340,-154.877,0.729},
    {-547.736,-105.154,-155.176,0.372},
    {-547.274,-114.109,-153.952,0.735},
    {-552.534,-110.012,-153.577,0.747},
    {-550.708,-116.436,-153.103,0.679},
    {-554.030,-115.983,-152.635,0.695},
    {-494.595,-87.516,149.116,3.344},
    {-493.349,-90.845,-148.882,3.717},
    {-491.995,-87.619,-148.197,3.230},
    {-490.732,-90.739,-148.091,3.230},
    {-490.554,-89.114,-148.055,3.230},
    {-495.240,-90.808,-149.493,3.238},
    {-494.195,-89.553,-149.131,3.254}
};

struct npc_blastmaster_emi_shortfuseAI : public npc_escortAI
{
    npc_blastmaster_emi_shortfuseAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        pInstance = pCreature->GetInstanceData();
        pCreature->RestoreFaction();
        Reset();
    }

    ScriptedInstance* pInstance;

    uint8 uiPhase;
    uint32 uiTimer;

    std::list<uint64> SummonList;
    std::list<uint64> GoSummonList;

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            uiTimer = 0;
            uiPhase = 0;

            RestoreAll();

            SummonList.clear();
            GoSummonList.clear();
        }
    }

    void NextStep(uint32 uiTimerStep,bool bNextStep = true,uint8 uiPhaseStep = 0)
    {
        uiTimer = uiTimerStep;
        if (bNextStep)
            ++uiPhase;
        else
            uiPhase = uiPhaseStep;
    }

    void CaveDestruction(bool bBool)
    {
        if (GoSummonList.empty())
            return;

        for (std::list<uint64>::const_iterator itr = GoSummonList.begin(); itr != GoSummonList.end(); ++itr)
        {
            if (GameObject* pGo = GameObject::GetGameObject(*me, *itr))
            {
                if (pGo)
                {
                    if (Creature *trigger = pGo->SummonTrigger(pGo->GetPositionX(), pGo->GetPositionY(),pGo->GetPositionZ(), 0, 1))
                    {
                        //visual effects are not working! ¬¬
                        trigger->CastSpell(trigger,11542,true);
                        trigger->CastSpell(trigger,35470,true);
                    }
                    pGo->RemoveFromWorld();
                    //pGo->CastSpell(me,12158); makes all die?!
                }
            }
        }

        if (bBool)
        {
            if (pInstance)
                if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_RIGHT)))
                    pInstance->HandleGameObject(0LL,false,pGo);
        }else
            if (pInstance)
                if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_LEFT)))
                    pInstance->HandleGameObject(0LL,false,pGo);
    }

    void SetInFace(bool bBool)
    {
        if (!pInstance)
            return;

        if (bBool)
        {
            if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_RIGHT)))
                me->SetFacingToObject(pGo);
        }else
            if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_LEFT)))
                me->SetFacingToObject(pGo);
    }

    void RestoreAll()
    {
        if (!pInstance)
            return;

        if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_RIGHT)))
            pInstance->HandleGameObject(0LL,false,pGo);

        if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_LEFT)))
            pInstance->HandleGameObject(0LL,false,pGo);

        if (!GoSummonList.empty())
            for (std::list<uint64>::const_iterator itr = GoSummonList.begin(); itr != GoSummonList.end(); ++itr)
            {
                if (GameObject* pGo = GameObject::GetGameObject(*me, *itr))
                    pGo->RemoveFromWorld();
            }

        if (!SummonList.empty())
            for (std::list<uint64>::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
            {
                if (Creature* pSummon = Unit::GetCreature(*me, *itr))
                {
                    if (pSummon->isAlive())
                        pSummon->DisappearAndDie();
                    else
                        pSummon->RemoveCorpse();
                }
            }
    }

    void AggroAllPlayers(Creature* pTemp)
    {
        Map::PlayerList const &PlList = me->GetMap()->GetPlayers();

        if (PlList.isEmpty())
            return;

        for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
        {
            if (Player* pPlayer = i->getSource())
            {
                if (pPlayer->isGameMaster())
                    continue;

                if (pPlayer->isAlive())
                {
                    pTemp->SetInCombatWith(pPlayer);
                    pPlayer->SetInCombatWith(pTemp);
                    pTemp->AddThreat(pPlayer, 0.0f);
                }
            }
        }
    }

    void WaypointReached(uint32 uiPoint)
    {
        //just in case
        if (GetPlayerForEscort())
            if (me->getFaction() != GetPlayerForEscort()->getFaction())
                me->setFaction(GetPlayerForEscort()->getFaction());

        switch(uiPoint)
        {
        case 3:
            SetEscortPaused(true);
            NextStep(2000,false,3);
            break;
        case 7:
            SetEscortPaused(true);
            NextStep(2000,false,4);
            break;
        case 9:
            NextStep(1000,false,8);
            break;
        case 10:
            NextStep(25000,false,10);
            break;
        case 11:
            SetEscortPaused(true);
            SetInFace(true);
            NextStep(1000,false,11);
            break;
        case 12:
            NextStep(25000,false,18);
            break;
        case 13:
            Summon(7);
            NextStep(25000,false,19);
            break;
        case 14:
            SetInFace(false);
            DoScriptText(SAY_BLASTMASTER_26,me);
            SetEscortPaused(true);
            NextStep(5000,false,20);
            break;
        }
    }

    void SetData(uint32 uiI,uint32 uiValue)
    {
        switch(uiI)
        {
        case 1:
            SetEscortPaused(true);
            DoScriptText(SAY_BLASTMASTER_0,me);
            NextStep(1500,true);
            break;
        case 2:
            if (!pInstance)
                return;

            switch(uiValue)
            {
            case 1:
                pInstance->SetData(TYPE_EVENT, IN_PROGRESS);
                break;
            case 2:
                pInstance->SetData(TYPE_EVENT, DONE);
                NextStep(5000,false,22);
                break;
            }
            break;
        }
    }

    void Summon(uint8 uiCase)
    {
        switch(uiCase)
        {
        case 1:
            for(int  i = 0; i < 10; ++i)
                me->SummonCreature(NPC_CAVERNDEEP_AMBUSHER, SpawnPosition[i][0], SpawnPosition[i][1], SpawnPosition[i][2], SpawnPosition[i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1800000);
            break;
        case 2:
            if (GameObject* pGo = me->SummonGameObject(183410, -533.140,-105.322,-156.016, 0, 0, 0, 0, 0, 1000))
            {
                GoSummonList.push_back(pGo->GetGUID());
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE); //We can't use it!
            }
            Summon(3);
            break;
        case 3:
            for(int  i = 0; i < 4; ++i)
                me->SummonCreature(NPC_CAVERNDEEP_AMBUSHER, SpawnPosition[i][0], SpawnPosition[i][1], SpawnPosition[i][2], SpawnPosition[i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1800000);
            DoScriptText(SAY_BLASTMASTER_19,me);
            break;
        case 4:
            if (GameObject* pGo = me->SummonGameObject(183410, -542.199,-96.854,-155.790, 0, 0, 0, 0, 0, 1000))
            {
                GoSummonList.push_back(pGo->GetGUID());
                pGo->SetFlag(GAMEOBJECT_FLAGS,GO_FLAG_NOTSELECTABLE);
            }
            break;
        case 5:
            for(int  i = 0; i < 3; ++i)
                me->SummonCreature(NPC_CAVERNDEEP_AMBUSHER, SpawnPosition[i][0], SpawnPosition[i][1], SpawnPosition[i][2], SpawnPosition[i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1800000);
            DoScriptText(SAY_BLASTMASTER_15,me);
            break;
        case 6:
            for(int  i = 10; i < 15; ++i)
                me->SummonCreature(NPC_CAVERNDEEP_AMBUSHER, SpawnPosition[i][0], SpawnPosition[i][1], SpawnPosition[i][2], SpawnPosition[i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1800000);
            break;
        case 7:
            if (GameObject* pGo = me->SummonGameObject(183410, -507.820,-103.333,-151.353, 0, 0, 0, 0, 0, 1000))
            {
                GoSummonList.push_back(pGo->GetGUID());
                pGo->SetFlag(GAMEOBJECT_FLAGS,GO_FLAG_NOTSELECTABLE); //We can't use it!
                Summon(6);
            }
            break;
        case 8:
            if (GameObject* pGo = me->SummonGameObject(183410, -511.829,-86.249,-151.431, 0, 0, 0, 0, 0, 1000))
            {
                GoSummonList.push_back(pGo->GetGUID());
                pGo->SetFlag(GAMEOBJECT_FLAGS,GO_FLAG_NOTSELECTABLE); //We can't use it!
            }
            break;
        case 9:
            if (Creature* pGrubbis = me->SummonCreature(NPC_GRUBBIS, SpawnPosition[15][0], SpawnPosition[15][1], SpawnPosition[15][2], SpawnPosition[15][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1800000))
                DoScriptText(SAY_GRUBBIS,pGrubbis);
            me->SummonCreature(NPC_CHOMPER, SpawnPosition[16][0], SpawnPosition[16][1], SpawnPosition[16][2], SpawnPosition[16][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1800000);
            break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (uiPhase)
        {
            if (uiTimer <= uiDiff)
            {
                switch(uiPhase)
                {
                case 1:
                    DoScriptText(SAY_BLASTMASTER_1,me);
                    NextStep(1500,true);
                    break;
                case 2:
                    SetEscortPaused(false);
                    NextStep(0,false,0);
                    break;
                case 3:
                    DoScriptText(SAY_BLASTMASTER_2,me);
                    SetEscortPaused(false);
                    NextStep(0,false,0);
                    break;
                case 4:
                    DoScriptText(SAY_BLASTMASTER_3,me);
                    NextStep(3000,true);
                    break;
                case 5:
                    DoScriptText(SAY_BLASTMASTER_4,me);
                    NextStep(3000,true);
                    break;
                case 6:
                    SetInFace(true);
                    DoScriptText(SAY_BLASTMASTER_5,me);
                    Summon(1);
                    if (pInstance)
                        if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_RIGHT)))
                            pInstance->HandleGameObject(0LL,true,pGo);
                    NextStep(3000,true);
                    break;
                case 7:
                    DoScriptText(SAY_BLASTMASTER_6,me);
                    SetEscortPaused(false);
                    NextStep(0,false,0);
                    break;
                case 8:
                    me->HandleEmoteCommand(EMOTE_STATE_WORK);
                    NextStep(25000,true);
                    break;
                case 9:
                    Summon(2);
                    NextStep(0,false);
                    break;
                case 10:
                    Summon(4);
                    NextStep(0,false);
                    break;
                case 11:
                    DoScriptText(SAY_BLASTMASTER_17,me);
                    NextStep(5000,true);
                    break;
                case 12:
                    DoScriptText(SAY_BLASTMASTER_18,me);
                    NextStep(5000,true);
                    break;
                case 13:
                    DoScriptText(SAY_BLASTMASTER_20,me);
                    CaveDestruction(true);
                    NextStep(8000,true);
                    break;
                case 14:
                    DoScriptText(SAY_BLASTMASTER_21,me);
                    NextStep(8500,true);
                    break;
                case 15:
                    DoScriptText(SAY_BLASTMASTER_22,me);
                    NextStep(2000,true);
                    break;
                case 16:
                    DoScriptText(SAY_BLASTMASTER_23,me);
                    SetInFace(false);
                    if (pInstance)
                        if (GameObject* pGo = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GO_CAVE_IN_LEFT)))
                            pInstance->HandleGameObject(0LL,true,pGo);
                    NextStep(2000,true);
                    break;
                case 17:
                    SetEscortPaused(false);
                    DoScriptText(SAY_BLASTMASTER_24,me);
                    Summon(6);
                    NextStep(0,false);
                    break;
                case 18:
                    Summon(7);
                    NextStep(0,false);
                    break;
                case 19:
                    SetInFace(false);
                    Summon(8);
                    DoScriptText(SAY_BLASTMASTER_25,me);
                    NextStep(0,false);
                    break;
                case 20:
                    DoScriptText(SAY_BLASTMASTER_27,me);
                    NextStep(2000,true);
                    break;
                case 21:
                    Summon(9);
                    NextStep(0,false);
                    break;
                case 22:
                    CaveDestruction(false);
                    DoScriptText(SAY_BLASTMASTER_20,me);
                    NextStep(0,false);
                    break;
                }
            } else uiTimer -= uiDiff;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void JustSummoned(Creature* pSummon)
    {
        SummonList.push_back(pSummon->GetGUID());
        AggroAllPlayers(pSummon);
    }
};

CreatureAI* GetAI_npc_blastmaster_emi_shortfuse(Creature* pCreature)
{
    return new npc_blastmaster_emi_shortfuseAI(pCreature);
}

bool GossipHello_npc_blastmaster_emi_shortfuse(Player* pPlayer, Creature* pCreature)
{
    ScriptedInstance* pInstance = pCreature->GetInstanceData();

    if (pInstance && pInstance->GetData(TYPE_EVENT) == NOT_STARTED)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_START_EVENT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_EMI, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_blastmaster_emi_shortfuse(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_blastmaster_emi_shortfuseAI, pCreature->AI()))
            pEscortAI->Start(true, false,pPlayer->GetGUID());

        pCreature->setFaction(pPlayer->getFaction());
        pCreature->AI()->SetData(1,0);

        pPlayer->CLOSE_GOSSIP_MENU();
    }
    return true;
}

struct boss_grubbisAI : public ScriptedAI
{
    boss_grubbisAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        SetDataSummoner();
    }

    void SetDataSummoner()
    {
        if (!me->IsTemporarySummon())
            return;

        TemporarySummon* tmp_summon = reinterpret_cast<TemporarySummon*>(me);
        if(tmp_summon)
        {
            if (Unit* summon = tmp_summon->GetSummoner())
                if (Creature* creature = summon->ToCreature())
                    creature->AI()->SetData(2, 1);
        }
    }

    void UpdateAI(const uint32 /*diff*/)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (!me->IsTemporarySummon())
            return;

        TemporarySummon* tmp_summon = reinterpret_cast<TemporarySummon*>(me);
        if(tmp_summon)
        {
            if (Unit* summon = tmp_summon->GetSummoner())
                if (Creature* creature = summon->ToCreature())
                    creature->AI()->SetData(2, 2);
        }
    }
};

CreatureAI* GetAI_boss_grubbis(Creature* pCreature)
{
    return new boss_grubbisAI(pCreature);
}

enum eKernobee
{
    QUEST_A_FINE_MESS = 2904,
    LOC_X = -339,
    LOC_Y = 1,
    LOC_Z = -153
};

struct npc_kernobee : public FollowerAI
{
    npc_kernobee(Creature* c) : FollowerAI(c) {}

    void Reset() {}

    void UpdateFollowerAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
        {
            if (me->GetDistance(LOC_X, LOC_Y, LOC_Z) < 20)
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_A_FINE_MESS) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->GroupEventHappens(QUEST_A_FINE_MESS, me);
                }
                SetFollowComplete();
            }
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kernobee(Creature *_Creature)
{
    return new npc_kernobee(_Creature);
}

bool QuestAccept_npc_kernobee(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_A_FINE_MESS)
    {
        if (npc_kernobee* pkernobee = CAST_AI(npc_kernobee, pCreature->AI()))
        {
            pkernobee->StartFollow(pPlayer, 0, pQuest);
        }
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    return true;
}

void AddSC_gnomeregan()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_blastmaster_emi_shortfuse";
    newscript->pGossipHello =  &GossipHello_npc_blastmaster_emi_shortfuse;
    newscript->pGossipSelect = &GossipSelect_npc_blastmaster_emi_shortfuse;
    newscript->GetAI = &GetAI_npc_blastmaster_emi_shortfuse;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_boss_grubbis";
    newscript->GetAI = &GetAI_boss_grubbis;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_kernobee";
    newscript->GetAI = &GetAI_npc_kernobee;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kernobee;
    newscript->RegisterSelf();
}

