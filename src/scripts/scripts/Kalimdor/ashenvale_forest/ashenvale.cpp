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
SDName: Ashenvale
SD%Complete: 70
SDComment: Quest support: 6544, 6482
SDCategory: Ashenvale Forest
EndScriptData */

/* ContentData
npc_torek
npc_ruul_snowhoof
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*####
# npc_torek
####*/

#define SAY_READY                   -1000106
#define SAY_MOVE                    -1000107
#define SAY_PREPARE                 -1000108
#define SAY_WIN                     -1000109
#define SAY_END                     -1000110

#define SPELL_REND                  11977
#define SPELL_THUNDERCLAP           8078

#define QUEST_TOREK_ASSULT          6544

#define ENTRY_SPLINTERTREE_RAIDER   12859
#define ENTRY_DURIEL                12860
#define ENTRY_SILVERWING_SENTINEL   12896
#define ENTRY_SILVERWING_WARRIOR    12897

struct npc_torekAI : public npc_escortAI
{
    npc_torekAI(Creature *c) : npc_escortAI(c) {}

    Timer Rend_Timer;
    Timer Thunderclap_Timer;
    Timer Despawn_Timer;
    bool Completed;
    std::list<uint64> SplintertreeRaiderList;

    void Reset()
    {
        Rend_Timer.Reset(5000);
        Thunderclap_Timer.Reset(8000);
        Despawn_Timer.Reset(0);
        Completed = false;
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStart(who);
        for (std::list<uint64>::iterator itr = SplintertreeRaiderList.begin(); itr != SplintertreeRaiderList.end(); ++itr)
        {
            if (Creature* it = me->GetCreature(*itr))
            {
                float x, y, z;
                it->GetPosition(x, y, z);
                it->SetHomePosition(x, y, z, 0);
                it->AI()->AttackStart(who);
            }
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void SetFormation()
    {
        uint32 Count = 0;
        
        std::list<Creature*> SplintertreeRaiderListPtr = FindAllCreaturesWithEntry(ENTRY_SPLINTERTREE_RAIDER, 15);
        for (std::list<Creature*>::iterator itr = SplintertreeRaiderListPtr.begin(); itr != SplintertreeRaiderListPtr.end(); ++itr)
        {
            float fAngle = Count < 3 ? (M_PI*2)/3 - (Count*2*(M_PI*2)/3) : 0.0f;
            if ((*itr)->isAlive())
            {
                (*itr)->GetMotionMaster()->MoveFollow(me, 1.5f, fAngle);
                (*itr)->SetSpeed(MOVE_WALK, 1.1f, true);
                (*itr)->SetSpeed(MOVE_RUN, 1.1f, true);
                (*itr)->setFaction(FACTION_ESCORT_H_NEUTRAL_ACTIVE);
            }
            SplintertreeRaiderList.push_back((*itr)->GetGUID());
            ++Count;
        }
        me->setFaction(FACTION_ESCORT_H_NEUTRAL_ACTIVE);
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (i)
        {
            case 1:
                SetFormation();
                DoScriptText(SAY_MOVE, m_creature, pPlayer);
                break;
            case 8:
                DoScriptText(SAY_PREPARE, m_creature, pPlayer);
                break;
            case 19:
                m_creature->SummonCreature(ENTRY_DURIEL,1776.73,-2049.06,109.83,1.54,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,25000);
                m_creature->SummonCreature(ENTRY_SILVERWING_SENTINEL,1774.64,-2049.41,109.83,1.40,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,25000);
                m_creature->SummonCreature(ENTRY_SILVERWING_WARRIOR,1778.73,-2049.50,109.83,1.67,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,25000);
                break;
            case 20:
                DoScriptText(SAY_WIN, m_creature, pPlayer);
                Completed = true;
                pPlayer->GroupEventHappens(QUEST_TOREK_ASSULT,m_creature);
                break;
            case 21:
                DoScriptText(SAY_END, m_creature, pPlayer);
                Despawn_Timer = 10000;
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        if (Rend_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_REND);
            Rend_Timer = 20000;
        }
        

        if (Thunderclap_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_THUNDERCLAP);
            Thunderclap_Timer = 30000;
        }
        
        
        if(Despawn_Timer.Expired(diff))
        {
            for (std::list<uint64>::iterator itr = SplintertreeRaiderList.begin(); itr != SplintertreeRaiderList.end(); ++itr)
            {
                if (Creature* it = me->GetCreature(*itr))
                {
                    if (it->isAlive())
                        it->DisappearAndDie();
                }
            }
            me->DisappearAndDie();
        }
    }
};

bool QuestAccept_npc_torek(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (pPlayer && quest->GetQuestId() == QUEST_TOREK_ASSULT)
    {
        //TODO: find companions, make them follow Torek, at any time (possibly done by mangos/database in future?)
        DoScriptText(SAY_READY, pCreature, pPlayer);
        pCreature->setFaction(113);

        if (npc_escortAI* pEscortAI = CAST_AI(npc_torekAI, pCreature->AI()))
            pEscortAI->Start(true, true, pPlayer->GetGUID(), quest);
    }

    return true;
}

CreatureAI* GetAI_npc_torek(Creature *_Creature)
{
    npc_torekAI* thisAI = new npc_torekAI(_Creature);

    thisAI->AddWaypoint(0, 1782.63, -2241.11, 109.73, 5000);
    thisAI->AddWaypoint(1, 1788.88, -2240.17, 111.71);
    thisAI->AddWaypoint(2, 1797.49, -2238.11, 112.31);
    thisAI->AddWaypoint(3, 1803.83, -2232.77, 111.22);
    thisAI->AddWaypoint(4, 1806.65, -2217.83, 107.36);
    thisAI->AddWaypoint(5, 1811.81, -2208.01, 107.45);
    thisAI->AddWaypoint(6, 1820.85, -2190.82, 100.49);
    thisAI->AddWaypoint(7, 1829.60, -2177.49, 96.44);
    thisAI->AddWaypoint(8, 1837.98, -2164.19, 96.71);       //prepare
    thisAI->AddWaypoint(9, 1839.99, -2149.29, 96.78);
    thisAI->AddWaypoint(10, 1835.14, -2134.98, 96.80);
    thisAI->AddWaypoint(11, 1823.57, -2118.27, 97.43);
    thisAI->AddWaypoint(12, 1814.99, -2110.35, 98.38);
    thisAI->AddWaypoint(13, 1806.60, -2103.09, 99.19);
    thisAI->AddWaypoint(14, 1798.27, -2095.77, 100.04);
    thisAI->AddWaypoint(15, 1783.59, -2079.92, 100.81);
    thisAI->AddWaypoint(16, 1776.79, -2069.48, 101.77);
    thisAI->AddWaypoint(17, 1776.82, -2054.59, 109.82);
    thisAI->AddWaypoint(18, 1776.88, -2047.56, 109.83);
    thisAI->AddWaypoint(19, 1776.86, -2036.55, 109.83);
    thisAI->AddWaypoint(20, 1776.90, -2024.56, 109.83);     //win
    thisAI->AddWaypoint(21, 1776.87, -2028.31, 109.83,60000);//stay
    thisAI->AddWaypoint(22, 1776.90, -2028.30, 109.83);

    return (CreatureAI*)thisAI;
}

/*####
# npc_ruul_snowhoof
####*/

#define QUEST_FREEDOM_TO_RUUL    6482
#define GO_CAGE                  178147

struct npc_ruul_snowhoofAI : public npc_escortAI
{
    npc_ruul_snowhoofAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch(i)
        {
        case 0: 
        {
                m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                GameObject* Cage = FindGameObject(GO_CAGE, 20, m_creature);
                if(Cage)
                    Cage->SetGoState(GO_STATE_ACTIVE);
                break;
        }
        case 13:
                m_creature->SummonCreature(3922, 3449.218018, -587.825073, 174.978867, 4.714445, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3921, 3446.384521, -587.830872, 175.186279, 4.714445, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3926, 3444.218994, -587.835327, 175.380600, 4.714445, TEMPSUMMON_DEAD_DESPAWN, 60000);
                break;
        case 19:
                m_creature->SummonCreature(3922, 3508.344482, -492.024261, 186.929031, 4.145029, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3921, 3506.265625, -490.531006, 186.740128, 4.239277, TEMPSUMMON_DEAD_DESPAWN, 60000);
                m_creature->SummonCreature(3926, 3503.682373, -489.393799, 186.629684, 4.349232, TEMPSUMMON_DEAD_DESPAWN, 60000);
                break;

        case 21:
                pPlayer->GroupEventHappens(QUEST_FREEDOM_TO_RUUL, m_creature);
                break;
        }
    }

    void EnterCombat(Unit* who) {}

    void Reset()
    {
        if (GameObject* Cage = FindGameObject(GO_CAGE, 20, m_creature))
            Cage->SetGoState(GO_STATE_READY);
    }
};

bool QuestAccept_npc_ruul_snowhoof(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_FREEDOM_TO_RUUL)
    {
        pCreature->setFaction(113);

        if (npc_escortAI* pEscortAI = CAST_AI(npc_ruul_snowhoofAI, (pCreature->AI())))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);
    }
    return true;
}

CreatureAI* GetAI_npc_ruul_snowhoofAI(Creature *_Creature)
{
    npc_ruul_snowhoofAI* ruul_snowhoofAI = new npc_ruul_snowhoofAI(_Creature);

    ruul_snowhoofAI->AddWaypoint(0, 3347.250089, -694.700989, 159.925995);
    ruul_snowhoofAI->AddWaypoint(1, 3341.527039, -694.725891, 161.124542, 4000);
    ruul_snowhoofAI->AddWaypoint(2, 3338.351074, -686.088138, 163.444000);
    ruul_snowhoofAI->AddWaypoint(3, 3352.744873, -677.721741, 162.316269);
    ruul_snowhoofAI->AddWaypoint(4, 3370.291016, -669.366943, 160.751358);
    ruul_snowhoofAI->AddWaypoint(5, 3381.479492, -659.449097, 162.545303);
    ruul_snowhoofAI->AddWaypoint(6, 3389.554199, -648.500000, 163.651825);
    ruul_snowhoofAI->AddWaypoint(7, 3396.645020, -641.508911, 164.216019);
    ruul_snowhoofAI->AddWaypoint(8, 3410.498535, -634.299622, 165.773453);
    ruul_snowhoofAI->AddWaypoint(9, 3418.461426, -631.791992, 166.477615);
    ruul_snowhoofAI->AddWaypoint(10, 3429.500000, -631.588745, 166.921265);
    ruul_snowhoofAI->AddWaypoint(11,3434.950195, -629.245483, 168.333969);
    ruul_snowhoofAI->AddWaypoint(12,3438.927979, -618.503235, 171.503143);
    ruul_snowhoofAI->AddWaypoint(13,3444.217529, -609.293640, 173.077972, 1000); // Ambush 1
    ruul_snowhoofAI->AddWaypoint(14,3460.505127, -593.794189, 174.342255);
    ruul_snowhoofAI->AddWaypoint(15,3480.283203, -578.210327, 176.652313);
    ruul_snowhoofAI->AddWaypoint(16,3492.912842, -562.335449, 181.396301);
    ruul_snowhoofAI->AddWaypoint(17,3495.230957, -550.977600, 184.652267);
    ruul_snowhoofAI->AddWaypoint(18,3496.247070, -529.194214, 188.172028);
    ruul_snowhoofAI->AddWaypoint(19,3497.619385, -510.411499, 188.345322, 1000); // Ambush 2
    ruul_snowhoofAI->AddWaypoint(20,3498.498047, -497.787506, 185.806274);
    ruul_snowhoofAI->AddWaypoint(21,3484.218750, -489.717529, 182.389862, 4000); // End

    return (CreatureAI*)ruul_snowhoofAI;
}

/*####
# npc_muglash
####*/

enum eEnums
{
    SAY_MUG_START1          = -1800054,
    SAY_MUG_START2          = -1800055,
    SAY_MUG_BRAZIER         = -1800056,
    SAY_MUG_BRAZIER_WAIT    = -1800057,
    SAY_MUG_ON_GUARD        = -1800058,
    SAY_MUG_REST            = -1800059,
    SAY_MUG_DONE            = -1800060,
    SAY_MUG_GRATITUDE       = -1800061,
    SAY_MUG_PATROL          = -1800062,
    SAY_MUG_RETURN          = -1800063,

    QUEST_VORSHA            = 6641,

    GO_NAGA_BRAZIER         = 178247,

    NPC_WRATH_RIDER         = 3713,
    NPC_WRATH_SORCERESS     = 3717,
    NPC_WRATH_RAZORTAIL     = 3712,

    NPC_WRATH_PRIESTESS     = 3944,
    NPC_WRATH_MYRMIDON      = 3711,
    NPC_WRATH_SEAWITCH      = 3715,

    NPC_VORSHA              = 12940,
    NPC_MUGLASH             = 12717
};

static float m_afFirstNagaCoord[3][3]=
{
    {3603.504150, 1122.631104, 1.635},                      // rider
    {3589.293945, 1148.664063, 5.565},                      // sorceress
    {3609.925537, 1168.759521, -1.168}                      // razortail
};

static float m_afSecondNagaCoord[3][3]=
{
    {3609.925537, 1168.759521, -1.168},                     // witch
    {3645.652100, 1139.425415, 1.322},                      // priest
    {3583.602051, 1128.405762, 2.347}                       // myrmidon
};

static float m_fVorshaCoord[]={3633.056885, 1172.924072, -5.388};

struct npc_muglashAI : public npc_escortAI
{
    npc_muglashAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint32 m_uiWaveId;
    Timer m_uiEventTimer;
    Timer m_uiPausedCheckTimer;
    uint64 m_uiBrazierGUID;
    bool m_bIsBrazierExtinguished;

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->SetHomePosition(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0);
        pSummoned->SetAggroRange(50.0f);
        pSummoned->AI()->AttackStart(m_creature);
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        switch(i)
        {
            case 0:
                if (pPlayer)
                    DoScriptText(SAY_MUG_START2, m_creature, pPlayer);
                break;
            case 24:
                if (pPlayer)
                    DoScriptText(SAY_MUG_BRAZIER, m_creature, pPlayer);

                if (GameObject* pGo = FindGameObject(GO_NAGA_BRAZIER, INTERACTION_DISTANCE*2, m_creature))
                {
                    m_uiBrazierGUID = pGo->GetGUID();
                    
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                    SetEscortPaused(true);
                }
                break;
            case 25:
                DoScriptText(SAY_MUG_GRATITUDE, m_creature);

                if (pPlayer)
                    pPlayer->GroupEventHappens(QUEST_VORSHA, m_creature);
                break;
            case 26:
                DoScriptText(SAY_MUG_PATROL, m_creature);
                break;
            case 27:
                if(m_uiBrazierGUID)
                {
                    if (GameObject* pGo = GameObject::GetGameObject(*m_creature, m_uiBrazierGUID))
                    {
                        pGo->SetLootState(GO_JUST_DEACTIVATED);
                        pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                        pGo->SetGoState(GO_STATE_READY);
                    }
                }
                DoScriptText(SAY_MUG_RETURN, m_creature);
                break;
        }
    }

    void EnterCombat(Unit* pWho)
    {
        if (HasEscortState(STATE_ESCORT_PAUSED))
        {
            if (urand(0, 1))
            DoScriptText(SAY_MUG_ON_GUARD, m_creature);
            return;
        }
    }

    void Reset()
    {
        if(!HasEscortState(STATE_ESCORT_ESCORTING))
            m_uiBrazierGUID = 0;
        if(!HasEscortState(STATE_ESCORT_PAUSED))
        {
            m_uiPausedCheckTimer.Reset(120000);   //after 2 minutes pausing event ends with fail
            m_uiEventTimer.Reset(10000);
            m_uiWaveId = 0;
            m_bIsBrazierExtinguished = false;
        }
    }

    void JustDied(Unit* pKiller)
    {
        m_creature->Respawn();
        if (Player* pPlayer = GetPlayerForEscort())
        {
            if (Group* pGroup = pPlayer->GetGroup())
            {
                for (GroupReference* pRef = pGroup->GetFirstMember(); pRef != NULL; pRef = pRef->next())
                {
                    if (Player* pMember = pRef->getSource())
                    {
                        if (pMember->GetQuestStatus(QUEST_VORSHA) == QUEST_STATUS_INCOMPLETE)
                            pMember->FailQuest(QUEST_VORSHA);
                    }
                }
            }
            else
            {
                if (pPlayer->GetQuestStatus(QUEST_VORSHA) == QUEST_STATUS_INCOMPLETE)
                    pPlayer->FailQuest(QUEST_VORSHA);
            }
            if(m_uiBrazierGUID)
            {
                if (GameObject* pGo = GameObject::GetGameObject(*pPlayer, m_uiBrazierGUID))
                {
                    pGo->SetLootState(GO_JUST_DEACTIVATED);
                    pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                    pGo->SetGoState(GO_STATE_READY);
                }
            }
        }
    }

    void DoWaveSummon()
    {
        switch(m_uiWaveId)
        {
            case 1:
                m_creature->SummonCreature(NPC_WRATH_RIDER,     m_afFirstNagaCoord[0][0], m_afFirstNagaCoord[0][1], m_afFirstNagaCoord[0][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_SORCERESS, m_afFirstNagaCoord[1][0], m_afFirstNagaCoord[1][1], m_afFirstNagaCoord[1][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_RAZORTAIL, m_afFirstNagaCoord[2][0], m_afFirstNagaCoord[2][1], m_afFirstNagaCoord[2][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                break;
            case 2:
                m_creature->SummonCreature(NPC_WRATH_PRIESTESS, m_afSecondNagaCoord[0][0], m_afSecondNagaCoord[0][1], m_afSecondNagaCoord[0][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_MYRMIDON,  m_afSecondNagaCoord[1][0], m_afSecondNagaCoord[1][1], m_afSecondNagaCoord[1][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                m_creature->SummonCreature(NPC_WRATH_SEAWITCH,  m_afSecondNagaCoord[2][0], m_afSecondNagaCoord[2][1], m_afSecondNagaCoord[2][2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                break;
            case 3:
                m_creature->SummonCreature(NPC_VORSHA, m_fVorshaCoord[0], m_fVorshaCoord[1], m_fVorshaCoord[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                break;
            case 4:
                SetEscortPaused(false);
                DoScriptText(SAY_MUG_DONE, m_creature);
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        npc_escortAI::UpdateAI(uiDiff);

        if (!m_creature->GetVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if(m_uiPausedCheckTimer.Expired(uiDiff))
                {
                    SetEscortPaused(false);
                    m_creature->Kill(m_creature, false);
                    m_creature->RemoveCorpse();
                }
                

                if(m_bIsBrazierExtinguished)
                {
                    if (m_uiEventTimer.Expired(uiDiff))
                    {
                        ++m_uiWaveId;
                        DoWaveSummon();
                        if(m_uiWaveId == 3)
                            m_uiEventTimer = 2000;
                        else
                            m_uiEventTimer = 10000;
                    }
                }
            }
            return;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_muglash(Creature* pCreature)
{
    return new npc_muglashAI(pCreature);
}

bool QuestAccept_npc_muglash(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_VORSHA)
    {
        if (npc_muglashAI* pEscortAI = CAST_AI(npc_muglashAI, pCreature->AI()))
        {
            DoScriptText(SAY_MUG_START1, pCreature);
            pCreature->setFaction(113);

            pEscortAI->Start(true, true, pPlayer->GetGUID(), pQuest, true);
        }
    }
    return true;
}

bool GOUse_go_naga_brazier(Player* pPlayer, GameObject* pGo)
{
    if (Creature* pCreature = GetClosestCreatureWithEntry(pGo, NPC_MUGLASH, INTERACTION_DISTANCE*2))
    {
        if (npc_muglashAI* pEscortAI = CAST_AI(npc_muglashAI, pCreature->AI()))
        {
            DoScriptText(SAY_MUG_BRAZIER_WAIT, pCreature);
            pEscortAI->m_bIsBrazierExtinguished = true;
            return false;
        }
    }
    return false;
}

#define NPC_EARTHEN_RING_GUIDE      25324

bool ItemUse_item_Totemic_Beacon(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    float x,y,z;
    player->GetNearPoint(x,y,z, 0.0f, 3.0f, frand(0, 2*M_PI));
    player->SummonCreature(NPC_EARTHEN_RING_GUIDE, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 120000);
    return false;
}

#define NPC_ICECALLERBRIATHA        25949
#define SPELL_FROST_ARMOR           12544
#define SPELL_FROSTBOLT             20792

struct npc_Heretic_EmisaryAI : public ScriptedAI
{
    npc_Heretic_EmisaryAI(Creature* c) : ScriptedAI(c) {}

    Timer FrostArmorTimer;
    Timer FrostboltTimer;
    Timer TalkTimer;
    uint32 Phase;
    uint32 Check;
    uint64 player;
    bool EventStarted;

    void Reset()
    {
        Phase = 0;

        FrostArmorTimer.Reset(1000);
        FrostboltTimer.Reset(1000);
        TalkTimer.Reset(5000);
        EventStarted = false;
    }

    void MoveInLineOfSight(Unit * unit)
    {
        if(EventStarted)
            return;

        if(unit->GetTypeId() == TYPEID_PLAYER && unit->HasAura(46337, 0) && ((Player*)unit)->GetQuestStatus(11891) == QUEST_STATUS_INCOMPLETE)
        {
            EventStarted = true;
            Phase = 0;
            player = unit->GetGUID();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (FrostArmorTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature, SPELL_FROST_ARMOR, false);
            FrostArmorTimer = 30*MINUTE*MILLISECONDS;
        }

        if (!UpdateVictim())
        {
            if (EventStarted)
            {
                if (TalkTimer.Expired(diff))
                {
                    Creature * Briatha = GetClosestCreatureWithEntry(me, NPC_ICECALLERBRIATHA, 20);

                    if (Briatha && Briatha->isAlive()) 
                    {
                        switch(Phase)
                        {
                        case 0:
                            DoScriptText(-1230060, Briatha, 0);
                            Phase++;
                            break;
                        case 1:
                            DoScriptText(-1230061, me, 0);
                            Phase++;
                            break;
                        case 2:
                            DoScriptText(-1230062, Briatha, 0);
                            Phase++;
                            break;
                        case 3:
                            DoScriptText(-1230063, me, 0);
                            Phase++;
                            break;
                        case 4:
                            DoScriptText(-1230064, Briatha, 0);
                            Phase = 0;
                            if(Player* Player_ = (Player*)(me->GetUnit(player)))
                                if(Player_->HasAura(46337, 0))
                                    Player_->AreaExploredOrEventHappens(11891);
                            EventStarted = false;
                            break;
                        }
                        TalkTimer = 5000;
                    }
                    else
                        EventStarted = false;
                }
            }

            return;
        }
        
        if (FrostboltTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_FROSTBOLT, false);
            FrostboltTimer = 3000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_Heretic_Emisary(Creature *_Creature)
{
    return new npc_Heretic_EmisaryAI(_Creature);
}

/*###
# npc_feero_ironhand
###*/

enum FeeroIronhand
{
    NPC_DARK_STRAND_ASSASSINS   = 3879,
    NPC_FORSAKEN_SCOUT          = 3893,
    NPC_CAEDAKAR_LEFT           = 3900,                
    NPC_ALIGAR_MIDDLE           = 3898,
    NPC_BALIZAR_RIGHT           = 3899,
    QUEST_SUPPLIES_TO_AUBERDINE = 976
};

struct npc_feero_ironhandAI : public npc_escortAI
{
    npc_feero_ironhandAI(Creature *c) : npc_escortAI(c) {}

    void JustSummoned(Creature* summoned)
    {
        if (summoned)
            summoned->AI()->AttackStart(m_creature);
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch(i)
        {
            case 19:
                DoScriptText(-1230065, me, 0);
                me->SummonCreature(NPC_DARK_STRAND_ASSASSINS, 3491.09f, 214.76f, 11.36f, 3.30f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DARK_STRAND_ASSASSINS, 3491.18f, 212.28f, 11.25f, 3.03f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DARK_STRAND_ASSASSINS, 3490.74f, 210.59f, 11.32f, 3.03f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_DARK_STRAND_ASSASSINS, 3490.46f, 208.78f, 11.39f, 2.93f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 20:
                DoScriptText(-1230066, me, 0);
                break;
            case 27:
                DoScriptText(-1230067, me, 0);
                me->SummonCreature(NPC_FORSAKEN_SCOUT, 3782.51f, 145.57f, 8.54f, 2.93f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_FORSAKEN_SCOUT, 3778.84f, 143.46f, 8.41f, 2.93f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_FORSAKEN_SCOUT, 3782.26f, 149.61f, 8.34f, 2.93f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 28:
                DoScriptText(-1230068, me, 0);
                break;
            case 41:
                DoScriptText(-1230069, me, 0);
                me->SummonCreature(NPC_CAEDAKAR_LEFT, 4108.34f, 53.69f, 26.18f, 2.93f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_ALIGAR_MIDDLE, 4114.55f, 54.48f, 27.21f, 2.93f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                me->SummonCreature(NPC_BALIZAR_RIGHT, 4116.81f, 50.14f, 26.15f, 2.93f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 42:
                DoScriptText(-1230070, me, 0);
                break;
            case 43:
                DoScriptText(-1230071, me, 0);
                SetRun();
                pPlayer->GroupEventHappens(QUEST_SUPPLIES_TO_AUBERDINE, me);
                break;
        }
    }
};

bool QuestAccept_npc_feero_ironhand(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_SUPPLIES_TO_AUBERDINE)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_feero_ironhandAI, (pCreature->AI())))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);
    }
    return true;
}

CreatureAI* GetAI_npc_feero_ironhand(Creature *_Creature)
{
    npc_feero_ironhandAI* npc_feero_ironhand = new npc_feero_ironhandAI(_Creature);

    npc_feero_ironhand->AddWaypoint(0, 3175.930908, 193.541306, 3.483540);
    npc_feero_ironhand->AddWaypoint(1,  3187.917969, 197.117691, 4.699296);
    npc_feero_ironhand->AddWaypoint(2,  3203.483643, 192.349060, 5.908475);
    npc_feero_ironhand->AddWaypoint(3,  3219.118408, 182.236420, 6.588406);
    npc_feero_ironhand->AddWaypoint(4,  3229.847412, 191.230438, 7.494555);
    npc_feero_ironhand->AddWaypoint(5,  3225.035156, 199.438843, 7.096720);
    npc_feero_ironhand->AddWaypoint(6,  3227.651855, 210.760071, 8.629334);
    npc_feero_ironhand->AddWaypoint(7,  3232.935303, 223.724869, 10.052238);
    npc_feero_ironhand->AddWaypoint(8,  3263.071777, 225.984848, 10.645896);
    npc_feero_ironhand->AddWaypoint(9,  3284.759521, 220.414124, 10.950543);
    npc_feero_ironhand->AddWaypoint(10, 3315.646973, 210.198044, 11.967686);
    npc_feero_ironhand->AddWaypoint(11, 3341.024414, 214.290497, 13.320419);
    npc_feero_ironhand->AddWaypoint(12, 3367.133789, 224.587524, 11.867117);
    npc_feero_ironhand->AddWaypoint(13, 3409.073242, 226.385315, 9.215232);
    npc_feero_ironhand->AddWaypoint(14, 3432.292236, 225.396271, 10.028325);
    npc_feero_ironhand->AddWaypoint(15, 3454.865723, 219.339172, 12.593150);
    npc_feero_ironhand->AddWaypoint(16, 3470.463867, 214.818161, 13.264424);
    npc_feero_ironhand->AddWaypoint(17, 3481.416992, 212.556610, 12.354552, 2000); // say_1
    npc_feero_ironhand->AddWaypoint(18, 3500.315674, 210.936295, 10.226085); // say_death_1
    npc_feero_ironhand->AddWaypoint(19, 3532.806641, 215.041473, 8.372272);
    npc_feero_ironhand->AddWaypoint(20, 3565.314209, 217.748749, 5.300299);
    npc_feero_ironhand->AddWaypoint(21, 3601.654297, 217.771378, 1.299005);
    npc_feero_ironhand->AddWaypoint(22, 3602.654297, 217.771378, 1.299005);
    npc_feero_ironhand->AddWaypoint(23, 3638.605713, 212.525879, 1.433142);
    npc_feero_ironhand->AddWaypoint(24, 3680.757324, 200.308197, 3.385010);
    npc_feero_ironhand->AddWaypoint(25, 3725.670410, 180.395966, 6.314014);
    npc_feero_ironhand->AddWaypoint(26, 3762.346924, 159.685959, 7.388617);
    npc_feero_ironhand->AddWaypoint(27, 3774.541260, 151.170029, 7.799640, 2000); // say_2
    npc_feero_ironhand->AddWaypoint(28, 3789.697754, 140.396774, 9.062237); // say_death_2
    npc_feero_ironhand->AddWaypoint(29, 3821.424072, 111.609528, .258650);
    npc_feero_ironhand->AddWaypoint(30, 3850.376465, 84.710922, 13.941991);
    npc_feero_ironhand->AddWaypoint(31, 3875.349121, 60.388409, 14.988914);
    npc_feero_ironhand->AddWaypoint(32, 3908.238525, 35.209225, 15.332011);
    npc_feero_ironhand->AddWaypoint(33, 3942.200928, 14.888245, 16.969385);
    npc_feero_ironhand->AddWaypoint(34, 3976.427246, -0.073566, 16.968657);
    npc_feero_ironhand->AddWaypoint(35, 4008.343750, -6.628914, 16.464090);
    npc_feero_ironhand->AddWaypoint(36, 4029.483643, -6.640755, 16.549721);
    npc_feero_ironhand->AddWaypoint(37, 4050.055908, 1.488156, 15.746178);
    npc_feero_ironhand->AddWaypoint(38, 4083.412109, 14.085828, 15.851171);
    npc_feero_ironhand->AddWaypoint(39, 4098.462891, 20.032930, 17.252523);
    npc_feero_ironhand->AddWaypoint(40, 4105.861816, 34.792000, 20.284599);
    npc_feero_ironhand->AddWaypoint(41, 4110.536133, 45.538300, 23.154394, 2000); // say_3_A
    npc_feero_ironhand->AddWaypoint(42, 4112.863281, 51.454445, 26.165501); // say_3_B
    npc_feero_ironhand->AddWaypoint(43, 4126.177246, 53.689651, 26.399027, 3000); // say_death_3
    npc_feero_ironhand->AddWaypoint(44, 4149.127441, 46.833157, 24.660984);
    npc_feero_ironhand->AddWaypoint(45, 4164.439941, 55.935448, 26.793362); // DisappearAndDie();


    return (CreatureAI*)npc_feero_ironhand;
}

void AddSC_ashenvale()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_torek";
    newscript->GetAI = &GetAI_npc_torek;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_torek;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ruul_snowhoof";
    newscript->GetAI = &GetAI_npc_ruul_snowhoofAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_ruul_snowhoof;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_muglash";
    newscript->GetAI = &GetAI_npc_muglash;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_muglash;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_naga_brazier";
    newscript->pGOUse = &GOUse_go_naga_brazier;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="item_Totemic_Beacon";
    newscript->pItemUse = &ItemUse_item_Totemic_Beacon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_Heretic_Emisary";
    newscript->GetAI = &GetAI_npc_Heretic_Emisary;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_feero_ironhand";
    newscript->GetAI = &GetAI_npc_feero_ironhand;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_feero_ironhand;
    newscript->RegisterSelf();
}
