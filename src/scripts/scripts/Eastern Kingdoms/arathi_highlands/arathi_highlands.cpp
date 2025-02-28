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
SDName: Arathi Highlands
SD%Complete: 100
SDComment: Quest support: 665
SDCategory: Arathi Highlands
EndScriptData */

/* ContentData
npc_professor_phizzlethorpe
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_professor_phizzlethorpe
######*/

#define SAY_PROGRESS_1      -1000235
#define SAY_PROGRESS_2      -1000236
#define SAY_PROGRESS_3      -1000237
#define EMOTE_PROGRESS_4    -1000238
#define SAY_AGGRO           -1000239
#define SAY_PROGRESS_5      -1000240
#define SAY_PROGRESS_6      -1000241
#define SAY_PROGRESS_7      -1000242
#define EMOTE_PROGRESS_8    -1000243
#define SAY_PROGRESS_9      -1000244

#define QUEST_SUNKEN_TREASURE   665
#define MOB_VENGEFUL_SURGE  2776

struct npc_professor_phizzlethorpeAI : public npc_escortAI
{
    npc_professor_phizzlethorpeAI(Creature *c) : npc_escortAI(c) {}
  
    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        switch(i)
        {
        case 4:DoScriptText(SAY_PROGRESS_2, m_creature, player);break;
        case 5:DoScriptText(SAY_PROGRESS_3, m_creature, player);break;
        case 8:DoScriptText(EMOTE_PROGRESS_4, m_creature);break;
        case 9:
            {
            m_creature->SummonCreature(MOB_VENGEFUL_SURGE, -2052.96, -2142.49, 20.15, 1.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
            m_creature->SummonCreature(MOB_VENGEFUL_SURGE, -2052.96, -2142.49, 20.15, 1.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
            break;
            }
        case 10:DoScriptText(SAY_PROGRESS_5, m_creature, player);break;
        case 11:DoScriptText(SAY_PROGRESS_6, m_creature, player);break;
        case 19:DoScriptText(SAY_PROGRESS_7, m_creature, player); break;
        case 20:
            DoScriptText(EMOTE_PROGRESS_8, m_creature);
            DoScriptText(SAY_PROGRESS_9, m_creature, player);
            if(player)
                player->GroupEventHappens(QUEST_SUNKEN_TREASURE, m_creature);
            break;
        }
    }

    void JustSummoned(Creature *summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void EnterCombat(Unit* pWho)
    {
        m_creature->setFaction(35);
        DoScriptText(SAY_AGGRO, m_creature);
    }
};

bool QuestAccept_npc_professor_phizzlethorpe(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_SUNKEN_TREASURE)
    {
        DoScriptText(SAY_PROGRESS_1, pCreature, pPlayer);
        if (npc_escortAI* pEscortAI = CAST_AI(npc_professor_phizzlethorpeAI, (pCreature->AI())))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);

        pCreature->setFaction(113);
    }
    return true;
}

CreatureAI* GetAI_npc_professor_phizzlethorpeAI(Creature *_Creature)
{
    npc_professor_phizzlethorpeAI* professor_phizzlethorpeAI = new npc_professor_phizzlethorpeAI(_Creature);

    professor_phizzlethorpeAI->AddWaypoint(0, -2066.45, -2085.96, 9.08);
    professor_phizzlethorpeAI->AddWaypoint(1, -2077.99, -2105.33, 13.24);
    professor_phizzlethorpeAI->AddWaypoint(2, -2074.60, -2109.67, 14.24);
    professor_phizzlethorpeAI->AddWaypoint(3, -2076.60, -2117.46, 16.67);
    professor_phizzlethorpeAI->AddWaypoint(4, -2073.51, -2123.46, 18.42, 2000);
    professor_phizzlethorpeAI->AddWaypoint(5, -2073.51, -2123.46, 18.42, 4000);
    professor_phizzlethorpeAI->AddWaypoint(6, -2066.60, -2131.85, 21.56);
    professor_phizzlethorpeAI->AddWaypoint(7, -2053.85, -2143.19, 20.31);
    professor_phizzlethorpeAI->AddWaypoint(8, -2043.49, -2153.73, 20.20, 12000);
    professor_phizzlethorpeAI->AddWaypoint(9, -2043.49, -2153.73, 20.20, 14000);
    professor_phizzlethorpeAI->AddWaypoint(10, -2043.49, -2153.73, 20.20, 10000);
    professor_phizzlethorpeAI->AddWaypoint(11, -2043.49, -2153.73, 20.20, 2000);
    professor_phizzlethorpeAI->AddWaypoint(12, -2053.85, -2143.19, 20.31);
    professor_phizzlethorpeAI->AddWaypoint(13, -2066.60, -2131.85, 21.56);
    professor_phizzlethorpeAI->AddWaypoint(14, -2073.51, -2123.46, 18.42);
    professor_phizzlethorpeAI->AddWaypoint(15, -2076.60, -2117.46, 16.67);
    professor_phizzlethorpeAI->AddWaypoint(16, -2074.60, -2109.67, 14.24);
    professor_phizzlethorpeAI->AddWaypoint(17, -2077.99, -2105.33, 13.24);
    professor_phizzlethorpeAI->AddWaypoint(18, -2066.45, -2085.96, 9.08);
    professor_phizzlethorpeAI->AddWaypoint(19, -2066.41, -2086.21, 8.97, 6000);
    professor_phizzlethorpeAI->AddWaypoint(20, -2066.41, -2086.21, 8.97, 2000);

    return (CreatureAI*)professor_phizzlethorpeAI;
}

struct npc_shakes_obreenAI : public ScriptedAI
{
    npc_shakes_obreenAI(Creature* c) : ScriptedAI(c), Summons(c)
    {
    }

    SummonList Summons;
    uint32 SummonTimer;
    uint32 ClickTimer;
    uint8 Wave;
    bool clicked;
    bool EventStarted;
    std::list<uint64> AssistantsList;
    uint64 PlayerGUID;
    uint32 AttackersSlain;
    Timer ResetTimer;

    void Reset()
    {
        clicked = false;
        EventStarted = false;
        if(!EventStarted)
        {
            PlayerGUID = 0;
            AttackersSlain = 0;
            Wave = 0;
            SummonTimer = 0;
            AttackersSlain = 0;
            ResetTimer = 0;
        }
    }

    void JustSummoned(Creature *summoned)
    {
        Summons.Summon(summoned);
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* /*killer*/)
    {
        AttackersSlain++;
        if(AttackersSlain >= 3)
        {
            if(Wave == 2)
            {
                if(Player* plr = Unit::GetPlayerInWorld(PlayerGUID))
                    plr->AreaExploredOrEventHappens(667);
                ResetTimer = 1000;
            }
        }
    }

    void EventStart()
    {
        if(!EventStarted)
        {
            EventStarted = true;
            SummonTimer = 15000;
            me->setFaction(495);
            std::list<Creature*> AssistantsListPtrs = FindAllCreaturesWithEntry(2636, 50);
            if(Creature* Nilzlix = (Creature*)FindCreature(2767, 50, me))
                AssistantsListPtrs.push_back(Nilzlix);

            if(Creature* Lolo = (Creature*)FindCreature(2766, 50, me))
                AssistantsListPtrs.push_back(Lolo);

            for (std::list<Creature*>::iterator itr = AssistantsListPtrs.begin(); itr != AssistantsListPtrs.end(); ++itr)
            {
                (*itr)->setFaction(495);
                AssistantsList.push_back((*itr)->GetGUID());
            }
        }
    }

    void CannonCheck()
    {
        if(!clicked)
        {
            std::list<Creature*> raiders = FindAllCreaturesWithEntry(2595, 100); // Daggerspine Raider
            std::list<Creature*> sorceress = FindAllCreaturesWithEntry(2596, 100); // Daggerspine Sorceress
            raiders.merge(sorceress);
        
            if(raiders.empty())
                return;
        
            for (std::list<Creature*>::iterator itr = raiders.begin(); itr != raiders.end(); ++itr)
            {
                if(me->GetDistance((*itr)) > 30 && (*itr)->isAlive())
                    me->Kill((*itr));
            }
            clicked = true;
            ClickTimer = 10000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(ClickTimer)
        {
            if(ClickTimer < diff)
            {
                clicked = false;
                ClickTimer = 0;
            } else ClickTimer -= diff;
        }

        if(ResetTimer.Expired(diff))
        {
            for (std::list<uint64>::iterator itr = AssistantsList.begin(); itr != AssistantsList.end(); ++itr)
            {
                if (Creature* it = me->GetCreature(*itr))
                    it->setFaction(390);
            }
            AssistantsList.clear();
            SummonTimer = 0;
            Wave = 0;
            EventStarted = false;
            me->setFaction(390);
            ResetTimer = 0;
        }

        if (EventStarted)
        {
            if (SummonTimer < diff)
            {
                if(Wave == 0)
                {
                    Creature* Naga1 = me->SummonCreature(2596, -2152.797607, -1974.181763, 14.911042, 5.397616, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga2 = me->SummonCreature(2595, -2152.130127, -1969.829346, 15.671812, 5.408823, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga3 = me->SummonCreature(2595, -2158.961914, -1973.348267, 16.098627, 5.232110, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Naga1->GetMotionMaster()->MovePoint(1, -2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga1->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga2->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga3->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga2->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    Naga3->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    SummonTimer = 30000;
                    Wave++;
                }
                else if(Wave == 1)
                {
                    Creature* Naga1 = me->SummonCreature(2596, -2152.797607, -1974.181763, 14.911042, 5.397616, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga2 = me->SummonCreature(2595, -2152.130127, -1969.829346, 15.671812, 5.408823, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Naga1->GetMotionMaster()->MovePoint(1, -2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga1->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga2->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga2->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    SummonTimer = 30000;
                    Wave++;
                }
                else if(Wave == 2)
                {
                    Creature* Naga1 = me->SummonCreature(2596, -2152.797607, -1974.181763, 14.911042, 5.397616, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga2 = me->SummonCreature(2595, -2152.130127, -1969.829346, 15.671812, 5.408823, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga3 = me->SummonCreature(2595, -2158.961914, -1973.348267, 16.098627, 5.232110, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Naga1->GetMotionMaster()->MovePoint(1, -2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga1->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga2->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga3->SetHomePosition(-2091.708252, -2028.020020, 2.889296, 5.813303);
                    Naga2->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    Naga3->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    ResetTimer = 120000;
                }
            }
            else
                SummonTimer -= diff;
        }
        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_shakes_obreen(Creature *_Creature)
{
    return new npc_shakes_obreenAI(_Creature);
}

bool QuestAccept_npc_shakes_obreen(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 667)
    {
        CAST_AI(npc_shakes_obreenAI, pCreature->AI())->EventStart();
        CAST_AI(npc_shakes_obreenAI, pCreature->AI())->PlayerGUID = pPlayer->GetGUID();
    }
    return true;
}

bool GOUse_go_arathi_cannon(Player *player, GameObject* go)
{
    if (player->GetQuestStatus(667) != QUEST_STATUS_INCOMPLETE)
        return false;

    if(Creature* shakes_obreen = GetClosestCreatureWithEntry(go, 2610, 50))
        ((npc_shakes_obreenAI*)shakes_obreen->AI())->CannonCheck();
    return true;
}

enum
{
    SAY_START               = -1000948,
    SAY_REACH_BOTTOM        = -1000949,
    SAY_AGGRO_KINELORY      = -1000950,
    SAY_AGGRO_JORELL        = -1000951,
    SAY_WATCH_BACK          = -1000952,
    EMOTE_BELONGINGS        = -1000953,
    SAY_DATA_FOUND          = -1000954,
    SAY_ESCAPE              = -1000955,
    SAY_FINISH              = -1000956,
    EMOTE_HAND_PACK         = -1000957,

    // ToDo: find the healing spell id!
    SPELL_BEAR_FORM         = 4948,

    NPC_JORELL              = 2733,
    NPC_QUAE                = 2712,

    QUEST_HINTS_NEW_PLAGUE  = 660
};

struct npc_kineloryAI : public npc_escortAI
{
    npc_kineloryAI(Creature *c) : npc_escortAI(c) {}
  
    Timer BearFormTimer;

    void Reset()
    {
        BearFormTimer.Reset(urand(1000, 5000));
    }

    void EnterCombat(Unit* pWho)
    {
        if (pWho->GetEntry() == NPC_JORELL)
            DoScriptText(SAY_AGGRO_JORELL, pWho, me);
        else if (roll_chance_i(10))
            DoScriptText(SAY_AGGRO_KINELORY, me);
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        switch(i)
        {
            case 9:
                DoScriptText(SAY_REACH_BOTTOM, me);
                break;
            case 16:
                DoScriptText(SAY_WATCH_BACK, me);
                DoScriptText(EMOTE_BELONGINGS, me);
                break;
            case 17:
                DoScriptText(SAY_DATA_FOUND, me);
                break;
            case 18:
                DoScriptText(SAY_ESCAPE, me);
                if (Player* pPlayer = GetPlayerForEscort())
                    me->SetFacingToObject(pPlayer);
                SetRun();
                break;
            case 33:
                DoScriptText(SAY_FINISH, me);
                if (Creature* pQuae = GetClosestCreatureWithEntry(me, NPC_QUAE, 10.0f))
                {
                    DoScriptText(EMOTE_HAND_PACK, me, pQuae);
                    me->SetFacingToObject(pQuae);
                }
                break;
            case 34:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_HINTS_NEW_PLAGUE, me);
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(BearFormTimer.Expired(diff))
        {
            DoCast(me, SPELL_BEAR_FORM, false);
            BearFormTimer = 25000;
        }
        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_kinelory(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_HINTS_NEW_PLAGUE)
    {
        DoScriptText(SAY_START, pCreature, pPlayer);
        if (npc_escortAI* pEscortAI = CAST_AI(npc_kineloryAI, (pCreature->AI())))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest, true);

        pCreature->setFaction(113);
    }
    return true;
}

CreatureAI* GetAI_npc_kineloryAI(Creature *_Creature)
{
    npc_kineloryAI* kineloryAI = new npc_kineloryAI(_Creature);

    kineloryAI->AddWaypoint(0, -1416.91, -3044.12, 36.21);
    kineloryAI->AddWaypoint(1, -1408.43, -3051.35, 37.79);
    kineloryAI->AddWaypoint(2, -1399.45, -3069.20, 31.25);
    kineloryAI->AddWaypoint(3, -1400.28, -3083.14, 27.06);
    kineloryAI->AddWaypoint(4, -1405.30, -3096.72, 26.36);
    kineloryAI->AddWaypoint(5, -1406.12, -3105.95, 24.82);
    kineloryAI->AddWaypoint(6, -1417.41, -3106.80, 16.61);
    kineloryAI->AddWaypoint(7, -1433.06, -3101.55, 12.56);
    kineloryAI->AddWaypoint(8, -1439.86, -3086.36, 12.29);
    kineloryAI->AddWaypoint(9, -1450.48, -3065.16, 12.58, 5000);
    kineloryAI->AddWaypoint(10, -1456.15, -3055.53, 12.54);
    kineloryAI->AddWaypoint(11, -1459.41, -3035.16, 12.11);
    kineloryAI->AddWaypoint(12, -1472.47, -3034.18, 12.44);
    kineloryAI->AddWaypoint(13, -1495.57, -3034.48, 12.55);
    kineloryAI->AddWaypoint(14, -1524.91, -3035.47, 13.15);
    kineloryAI->AddWaypoint(15, -1549.05, -3037.77, 12.98);
    kineloryAI->AddWaypoint(16, -1555.69, -3028.02, 13.64, 3000);
    kineloryAI->AddWaypoint(17, -1555.69, -3028.02, 13.64, 5000);
    kineloryAI->AddWaypoint(18, -1555.69, -3028.02, 13.64, 2000);
    kineloryAI->AddWaypoint(19, -1551.19, -3037.78, 12.96);
    kineloryAI->AddWaypoint(20, -1584.60, -3048.77, 13.67);
    kineloryAI->AddWaypoint(21, -1602.14, -3042.82, 15.12);
    kineloryAI->AddWaypoint(22, -1610.68, -3027.42, 17.22);
    kineloryAI->AddWaypoint(23, -1601.65, -3007.97, 24.65);
    kineloryAI->AddWaypoint(24, -1581.05, -2992.32, 30.85);
    kineloryAI->AddWaypoint(25, -1559.95, -2979.51, 34.30);
    kineloryAI->AddWaypoint(26, -1536.51, -2969.78, 32.64);
    kineloryAI->AddWaypoint(27, -1511.81, -2961.09, 29.12);
    kineloryAI->AddWaypoint(28, -1484.83, -2960.87, 32.54);
    kineloryAI->AddWaypoint(29, -1458.23, -2966.80, 40.52);
    kineloryAI->AddWaypoint(30, -1440.20, -2971.20, 43.15);
    kineloryAI->AddWaypoint(31, -1427.85, -2989.15, 38.09);
    kineloryAI->AddWaypoint(32, -1420.27, -3008.91, 35.01);
    kineloryAI->AddWaypoint(33, -1427.58, -3032.53, 32.31, 5000);
    kineloryAI->AddWaypoint(34, -1427.40, -3035.17, 32.26);
    return (CreatureAI*)kineloryAI;
}

void AddSC_arathi_highlands()
{
    Script * newscript;

    newscript = new Script;
    newscript->Name = "npc_professor_phizzlethorpe";
    newscript->GetAI = &GetAI_npc_professor_phizzlethorpeAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_professor_phizzlethorpe;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_arathi_cannon";
    newscript->pGOUse = &GOUse_go_arathi_cannon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_shakes_obreen";
    newscript->GetAI = &GetAI_npc_shakes_obreen;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_shakes_obreen;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_kinelory";
    newscript->GetAI = &GetAI_npc_kineloryAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kinelory;
    newscript->RegisterSelf();
}

