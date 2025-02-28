// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
SDName: Instance_Shattered_Halls
SD%Complete: 99
SDComment:
SDCategory: Hellfire Citadel, Shattered Halls
EndScriptData */

/* ContentData
boss_blood_guard_porung
EndContentData */

#include "precompiled.h"
#include "def_shattered_halls.h"

enum BloodGuardPorung
{
    SPELL_CLEAVE        = 15496,
    SPELL_FLAME_ARROW   = 30952,
    SPELL_FIRE_PATCH_N  = 23971,
    SPELL_FIRE_PATCH_H  = 30928,

    NPC_ZEALOT          = 17462,
    NPC_HUNTER          = 17427,
    NPC_FLAME_ARROW_TR  = 17687,
    NPC_SCOUT           = 17693,

    PORUNG_YELL_1       = -1811007,
    PORUNG_YELL_2       = -1811008,
    PORUNG_YELL_3       = -1811009,
    PORUNG_YELL_4       = -1811010
};

struct SumonPos
{
    float x, y, z;
};

static SumonPos Pos[]=
{
    {502.24f, 339.12f, 2.105f},
    {503.24f, 292.17f, 1.937f}
};

static SumonPos ArrowPos_Left[]=
{
    {472.946, 319.281, 1.920},
    {407.173, 319.647, 1.911},
    {385.921, 313.350, 1.945},
    {442.892, 314.691, 1.905},
    {433.696, 309.448, 1.913},
    {417.231, 311.963, 1.934}
};

static SumonPos ArrowPos_Right[]=
{
    {431.761, 317.977, 1.914},
    {462.704, 312.561, 1.933},
    {398.575, 315.727, 1.913},
    {391.278, 319.930, 1.930},
    {451.451, 317.357, 1.940},
    {376.653, 319.861, 1.926}
};

struct npc_guard_hunterAI : public ScriptedAI
{
    npc_guard_hunterAI(Creature *c) : ScriptedAI(c) {}

    bool EventStartLeft;
    bool EventStartRight;
    Timer ArrowTimer;
    uint8 lastActive;
    uint8 preLastActive;

    std::vector<uint32> RandomPosList;

    void Reset() 
    {
        EventStartLeft = false;
        EventStartRight = false;
        ArrowTimer.Reset(1000);
        lastActive = 0;
        preLastActive = 0;
        for(uint8 i = 0; i < 6; i++)
            RandomPosList.push_back(i);
        std::random_shuffle(RandomPosList.begin(), RandomPosList.end());
    }

    void MoveInLineOfSight(Unit* who)
    {
        ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventStartLeft)
        {
            if(ArrowTimer.Expired(diff))
            {
                if(!RandomPosList.empty())
                {
                    if(Creature* ArrowTrigger = me->SummonCreature(NPC_FLAME_ARROW_TR, ArrowPos_Left[RandomPosList.front()].x, ArrowPos_Left[RandomPosList.front()].y, ArrowPos_Left[RandomPosList.front()].z, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000))
                    {
                        ArrowTrigger->SetVisibility(VISIBILITY_OFF);
                        me->CastSpell(ArrowTrigger, SPELL_FLAME_ARROW, true);
                    }
                    preLastActive = lastActive;
                    lastActive = RandomPosList.front();
                    RandomPosList.erase(RandomPosList.begin());
                    ArrowTimer = 5000;
                }
                else
                {
                    for(uint8 i = 0; i < 6; i++)
                    {
                        if(i == lastActive || i == preLastActive)
                            continue;
                        else
                            RandomPosList.push_back(i);
                    }
 
                    std::random_shuffle(RandomPosList.begin(), RandomPosList.end());
                    ArrowTimer = 5000;
                }
            }
        }
        else if(EventStartRight)
        {
            if(ArrowTimer.Expired(diff))
            {
                if(!RandomPosList.empty())
                {
                    if(Creature* ArrowTrigger = me->SummonCreature(NPC_FLAME_ARROW_TR, ArrowPos_Right[RandomPosList.front()].x, ArrowPos_Right[RandomPosList.front()].y, ArrowPos_Right[RandomPosList.front()].z, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000))
                    {
                        ArrowTrigger->setFaction(me->getFaction());
                        ArrowTrigger->SetLevel(me->GetLevel());
                        me->CastSpell(ArrowTrigger, SPELL_FLAME_ARROW, true);
                    }
                    preLastActive = lastActive;
                    lastActive = RandomPosList.front();
                    RandomPosList.erase(RandomPosList.begin());
                    ArrowTimer = 5000;
                }
                else
                {
                    for(uint8 i = 0; i < 6; i++)
                    {
                        if(i == lastActive || i == preLastActive)
                            continue;
                        else
                            RandomPosList.push_back(i);
                    }
 
                    std::random_shuffle(RandomPosList.begin(), RandomPosList.end());
                    ArrowTimer = 5000;
                }
            }
        }
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_guard_hunterAI(Creature *_Creature)
{
    return new npc_guard_hunterAI (_Creature);
}

struct npc_flame_arrow_trAI : public ScriptedAI
{
    npc_flame_arrow_trAI(Creature *c) : ScriptedAI(c) {}

    void Reset() 
    {
        SetCombatMovement(false);
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_FLAME_ARROW)
        {
            if(caster && caster->GetTypeId() == TYPEID_UNIT)
                me->CastSpell(me, HeroicMode ? SPELL_FIRE_PATCH_H : SPELL_FIRE_PATCH_N, true);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_flame_arrow_tr(Creature *_Creature)
{
    return new npc_flame_arrow_trAI (_Creature);
}

/*** when our eventstarter reach boss, boss Yell: "Archers, form ranks! On my mark!"
 * Right after that words 6 Zealots runs to its positions and 2 run on players - 8 zealots.
 * (After that 1 zealot run on players every time you attack any of Zealot from 1st and 2nd pack
 * On 3rd pack there are again 2 Zealots running on players, but only after death of 3rd pack
 * In total we should win 12 zealots)
 * After 3 seconds boss yell "Ready!"
 * After 4 seconds boss yell "Aim!"
 * After 5 seconds boss yell "Fire!" and archers begin to fire spell in random positions! Not in players like on CC.
 ***/

struct boss_blood_guard_porungAI : public ScriptedAI
{
    boss_blood_guard_porungAI(Creature *c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    SummonList Summons;
    Timer Cleave_Timer;

    bool EventStart;
    Timer YellPhraseTimer;
    Timer CheckZealotTimer;
    uint64 PhraseCounter;
    uint64 ZealotSummon[6];
    bool WaveOneCalled;
    bool WaveTwoCalled;
    bool WaveThreeCalled;
    bool ShouldCallCheckTimer;

    void Reset() 
    {
        Cleave_Timer.Reset(10000);
        EventStart = false;
        YellPhraseTimer.Reset(0);
        CheckZealotTimer.Reset(3000);
        PhraseCounter = 0;
        WaveOneCalled = false;
        WaveTwoCalled = false;
        WaveThreeCalled = false;
        ShouldCallCheckTimer = true;

        for(uint8 i = 0; i < 6; i++)
            ZealotSummon[i] = 0;

        Summons.DespawnAll();
        if (pInstance)
        {
            if (Creature* guardZealot1 = pInstance->GetCreature(pInstance->GetData64(DATA_ZEALOT_1)))
                guardZealot1->Respawn();
            if (Creature* guardZealot2 = pInstance->GetCreature(pInstance->GetData64(DATA_ZEALOT_2)))
                guardZealot2->Respawn();
            if (Creature* guardZealot3 = pInstance->GetCreature(pInstance->GetData64(DATA_ZEALOT_3)))
                guardZealot3->Respawn();
            if (Creature* guardScout = pInstance->GetCreatureById(NPC_SCOUT))
                guardScout->Respawn();
        }

		// 08.02.23 disabled because bugged (missed AI scripts in DB?)
        //me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_ATTACKABLE_2);
        //me->SetReactState(REACT_PASSIVE);
        //std::list<Creature*> lCreatureListPtr = FindAllCreaturesWithEntry(17427, 50);
        //if(lCreatureListPtr.empty())
        //    return;
        //for(std::list<Creature*>::iterator itr = lCreatureListPtr.begin(); itr != lCreatureListPtr.end(); ++itr)
        //{
        //    if((*itr)->isAlive())
        //    {
        //        (*itr)->SetReactState(REACT_PASSIVE);
        //        (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_ATTACKABLE_2);
        //    }
        //}
        if (pInstance)
            pInstance->SetData(TYPE_PORUNG, NOT_STARTED); 
    }

    void DoSummon()
    {
        if(Creature* summon = me->SummonCreature(NPC_ZEALOT, Pos[1].x, Pos[1].y, Pos[1].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotSummon[0] = summon->GetGUID();
            summon->GetMotionMaster()->MovePoint(1, 459.417, 308.960, 1.944, true, false, UNIT_ACTION_DOWAYPOINTS);
            summon->SetHomePosition(459.417, 308.960, 1.944, 0);
        }
        if(Creature* summon = me->SummonCreature(NPC_ZEALOT, Pos[1].x, Pos[1].y, Pos[1].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotSummon[1] = summon->GetGUID();
            summon->GetMotionMaster()->MovePoint(1, 421.126, 309.933, 1.944, true, false, UNIT_ACTION_DOWAYPOINTS);
            summon->SetHomePosition(421.126, 309.933, 1.944, 0);
        }
        if(Creature* summon = me->SummonCreature(NPC_ZEALOT, Pos[1].x, Pos[1].y, Pos[1].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotSummon[2] = summon->GetGUID();
            summon->GetMotionMaster()->MovePoint(1, 385.381, 309.905, 1.946, true, false, UNIT_ACTION_DOWAYPOINTS);
            summon->SetHomePosition(385.381, 309.905, 1.946, 0);
        }
        // right side
        if(Creature* summon = me->SummonCreature(NPC_ZEALOT, Pos[0].x, Pos[0].y, Pos[0].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotSummon[3] = summon->GetGUID();
            summon->GetMotionMaster()->MovePoint(1, 454.782, 319.751, 1.939, true, false, UNIT_ACTION_DOWAYPOINTS);
            summon->SetHomePosition(454.782, 319.751, 1.939, 0);
        }
        if(Creature* summon = me->SummonCreature(NPC_ZEALOT, Pos[0].x, Pos[0].y, Pos[0].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotSummon[4] = summon->GetGUID();
            summon->GetMotionMaster()->MovePoint(1, 418.351, 320.713, 1.937, true, false, UNIT_ACTION_DOWAYPOINTS);
            summon->SetHomePosition(418.351, 320.713, 1.937, 0);
        }
        if(Creature* summon = me->SummonCreature(NPC_ZEALOT, Pos[0].x, Pos[0].y, Pos[0].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotSummon[5] = summon->GetGUID(); 
            summon->GetMotionMaster()->MovePoint(1, 384.840, 321.314, 1.945, true, false, UNIT_ACTION_DOWAYPOINTS);
            summon->SetHomePosition(384.840, 321.314, 1.945, 0);
        }

        // Central
        if(Creature* ZealotLeft = me->SummonCreature(NPC_ZEALOT, Pos[1].x, Pos[1].y, Pos[1].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotLeft->SetAggroRange(200.0f);
            ZealotLeft->AI()->AttackStart(ZealotLeft->SelectNearestTarget(200));
        }
        if(Creature* ZealotRight = me->SummonCreature(NPC_ZEALOT, Pos[0].x, Pos[0].y, Pos[0].z, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0))
        {
            ZealotRight->SetAggroRange(200.0f);
            ZealotRight->AI()->AttackStart(ZealotRight->SelectNearestTarget(200));
        }

        if(pInstance)
        {
            for(uint8 i = 0; i < 6; i++)
                pInstance->GetCreature(ZealotSummon[i])->SetReactState(REACT_AGGRESSIVE);
        }
    }

    void DoSummonOne(uint32 ZealotNumber)
    {
        if(pInstance)
        {
            if(Creature* SummonedZealot = pInstance->GetCreature(ZealotSummon[ZealotNumber])->SummonCreature(17462, Pos[1].x, Pos[1].y, Pos[1].z, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
            {
                SummonedZealot->SetAggroRange(200.0);
                SummonedZealot->AI()->AttackStart(SummonedZealot->SelectNearestTarget(200));
            }
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
        {
            EventStart = true;
            DoScriptText(PORUNG_YELL_1, me);
            DoSummon();
            YellPhraseTimer = 3000;
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_ATTACKABLE_2, false);
            me->SetReactState(REACT_AGGRESSIVE);
            std::list<Creature*> lCreatureListPtr = FindAllCreaturesWithEntry(17427, 30);
            if(lCreatureListPtr.empty())
                return;
            for(std::list<Creature*>::iterator itr = lCreatureListPtr.begin(); itr != lCreatureListPtr.end(); ++itr)
            {
                if((*itr)->isAlive())
                {
                    (*itr)->SetReactState(REACT_AGGRESSIVE);
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_ATTACKABLE_2, false);
                }
            }
        }
    }

    void JustSummoned(Creature *pSummoned)
    {
        Summons.Summon(pSummoned);
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_PORUNG, DONE);
    }

    void EnterCombat(Unit *who)
    {
        if(Creature* hunter = me->GetMap()->GetCreatureById(17693))
        {
            if(hunter->isAlive())
            {
                hunter->setActive(true);
                hunter->GetMotionMaster()->MovePath(1079640, false);
            }
        }

        if (pInstance)
            pInstance->SetData(TYPE_PORUNG, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventStart)
        {
            if(YellPhraseTimer.Expired(diff))
            {
                if(PhraseCounter == 0)
                {
                    DoScriptText(PORUNG_YELL_2, me);
                    PhraseCounter++;
                    YellPhraseTimer = 1000;
                }
                else if(PhraseCounter == 1)
                {
                    DoScriptText(PORUNG_YELL_3, me);
                    PhraseCounter++;
                    YellPhraseTimer = 1000;
                }
                else if(PhraseCounter == 2)
                {
                    DoScriptText(PORUNG_YELL_4, me);

                    PhraseCounter = 0;
                    YellPhraseTimer = 0;
                    if(pInstance)
                    {
                        if (Creature* pHunterLeft = pInstance->GetCreature(pInstance->GetData64(DATA_HUNTER_1)))
                            CAST_AI(npc_guard_hunterAI, pHunterLeft->AI())->EventStartLeft = true;

                        if (Creature* pHunterRight = pInstance->GetCreature(pInstance->GetData64(DATA_HUNTER_2)))
                            CAST_AI(npc_guard_hunterAI, pHunterRight->AI())->EventStartRight = true;
                    }
                }
            }

            if(CheckZealotTimer.Expired(diff))
            {
                if(!pInstance)
                    return;

                if(!WaveOneCalled)
                {
                    if((pInstance->GetCreature(ZealotSummon[0]) && pInstance->GetCreature(ZealotSummon[0])->IsInCombat()) || (pInstance->GetCreature(ZealotSummon[3]) && pInstance->GetCreature(ZealotSummon[3])->IsInCombat()))
                    {
                        if(pInstance->GetCreature(ZealotSummon[0]) && pInstance->GetCreature(ZealotSummon[0])->IsInCombat())
                            DoSummonOne(0);
                        else if(pInstance->GetCreature(ZealotSummon[3]) && pInstance->GetCreature(ZealotSummon[3])->IsInCombat())
                            DoSummonOne(3);
                        WaveOneCalled = true;
                        CheckZealotTimer = 3000;
                    }
                }
                if(WaveOneCalled && !WaveTwoCalled)
                {
                    if((pInstance->GetCreature(ZealotSummon[1]) && pInstance->GetCreature(ZealotSummon[1])->IsInCombat()) || (pInstance->GetCreature(ZealotSummon[4]) && pInstance->GetCreature(ZealotSummon[4])->IsInCombat()))
                    {
                        if(pInstance->GetCreature(ZealotSummon[1]) && pInstance->GetCreature(ZealotSummon[1])->IsInCombat())
                            DoSummonOne(1);
                        else if(pInstance->GetCreature(ZealotSummon[4]) && pInstance->GetCreature(ZealotSummon[4])->IsInCombat())
                            DoSummonOne(4);
                        WaveTwoCalled = true;
                        CheckZealotTimer = 3000;
                    }
                }
                if(WaveOneCalled && WaveTwoCalled && !WaveThreeCalled)
                {
                    if((pInstance->GetCreature(ZealotSummon[2]) && pInstance->GetCreature(ZealotSummon[2])->isDead()) && (pInstance->GetCreature(ZealotSummon[5]) && pInstance->GetCreature(ZealotSummon[5])->isDead()))
                    {
                        DoSummonOne(5);
                        WaveThreeCalled = true;
                        CheckZealotTimer = 0;
                        ShouldCallCheckTimer = false;
                    }
                }
                if(ShouldCallCheckTimer)
                    CheckZealotTimer = 3000;
            }

        }
        if (!UpdateVictim())
            return;

        if (Cleave_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE, false);
            Cleave_Timer = 7500 + rand()%5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_blood_guard_porung(Creature *_Creature)
{
    return new boss_blood_guard_porungAI (_Creature);
}

void AddSC_boss_blood_guard_porung()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_blood_guard_porung";
    newscript->GetAI = &GetAI_boss_blood_guard_porung;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_guard_hunter";
    newscript->GetAI = &GetAI_npc_guard_hunterAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_flame_arrow_tr";
    newscript->GetAI = &GetAI_npc_flame_arrow_tr;
    newscript->RegisterSelf();
}
