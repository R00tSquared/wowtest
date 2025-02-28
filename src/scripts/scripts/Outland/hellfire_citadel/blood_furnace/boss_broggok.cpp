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
SDName: Boss_Broggok
SD%Complete: 100
SDComment:
SDCategory: Hellfire Citadel, Blood Furnace
EndScriptData */

#include "precompiled.h"
#include "def_blood_furnace.h"

#define SAY_AGGRO               -1542008

#define SPELL_SLIME_SPRAY       (HeroicMode ? 38458 : 30913)
#define SPELL_POISON_BOLT       (HeroicMode ? 38459 : 30917)
#define SPELL_POISON_CLOUD      30916
#define SPELL_POISON            (HeroicMode ? 38462 : 30914)

enum eEvents
{
    EVENT_NULL,
    EVENT_1_CAGE,
    EVENT_2_CAGE,
    EVENT_3_CAGE,
    EVENT_4_CAGE,
    EVENT_FIGHT
};

struct CellPosition
{
    float x, y, z, o;
};

static CellPosition CellLocation[]=
{
    { 501.118f,  84.228f, 9.657f, 3.188f },
    { 411.324f,  84.250f, 9.657f, 0.004f },
    { 500.440f, 115.296f, 9.657f, 3.141f },
    { 411.689f, 115.502f, 9.657f, 6.196f }
};

struct boss_broggokAI : public ScriptedAI
{
    boss_broggokAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = c->GetInstanceData();
    }

    Timer AcidSpray_Timer;
    Timer PoisonSpawn_Timer;
    Timer PoisonBolt_Timer;
    Timer checkTimer;
    Timer SummonTimer;

    SummonList summons;
    ScriptedInstance *pInstance;

    eEvents phase;

    std::map<uint64, uint8> prisoners;

    void Reset()
    {
        AcidSpray_Timer.Reset(10000);
        PoisonSpawn_Timer.Reset(8000);
        PoisonBolt_Timer.Reset(5000);
        checkTimer.Reset(3000);
        SummonTimer.Reset(0);

        summons.DespawnAll();
        prisoners.clear();

        if (pInstance)
            pInstance->SetData(DATA_BROGGOKEVENT, NOT_STARTED);

        me->SetReactState(REACT_PASSIVE);

        phase = EVENT_NULL;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);

        for (int i = 1; i <= 5; i++)
            pInstance->HandleGameObject(pInstance->GetData64(i), false, NULL);

        for (int i = 0; i < 4; i++)
        {
            for(int j = 0; j < 4; j++)
            {
                if (Creature *pPrisoner = me->SummonCreature(17429, CellLocation[i].x +frand(-2.0, 2.0), CellLocation[i].y +frand(-2.0, 2.0), CellLocation[i].z,  CellLocation[i].o, TEMPSUMMON_DEAD_DESPAWN, 2000))
                {
                    pPrisoner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    pPrisoner->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                    pPrisoner->SetReactState(REACT_PASSIVE);

                    pPrisoner->SetDisplayId(16332);
                    summons.Summon(pPrisoner);

                    prisoners[pPrisoner->GetGUID()] = i+1;
                }
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        phase = EVENT_FIGHT;
    }

    void JustDied(Unit *pKiller)
    {
        if (pInstance)
            pInstance->SetData(DATA_BROGGOKEVENT, DONE);
        

        std::list<Creature*> poisonCloud = FindAllCreaturesWithEntry(17662, 100);
        if (!poisonCloud.empty())
        {
            for (std::list<Creature*>::iterator i = poisonCloud.begin(); i != poisonCloud.end(); i++)
                (*i)->ForcedDespawn(1);
        }

        summons.DespawnAll();
        prisoners.clear();
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->setFaction(16);
        pSummoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        pSummoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void DoAction(const int32 param)
    {
        switch (param)
        {
            case EVENT_1_CAGE:
                if (pInstance)
                    pInstance->SetData(DATA_BROGGOKEVENT, IN_PROGRESS);
            case EVENT_2_CAGE:
            case EVENT_3_CAGE:
            case EVENT_4_CAGE:
            {
                phase = eEvents(param);
                if(HeroicMode)
                    SummonTimer = 120000;
                break;
            }
            default:
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                me->SetReactState(REACT_AGGRESSIVE);

                DoZoneInCombat();
                pInstance->HandleGameObject(pInstance->GetData64(5), true, NULL);
                return;
        }

        pInstance->HandleGameObject(pInstance->GetData64(phase), true, NULL);
        for (std::map<uint64, uint8>::iterator it = prisoners.begin(); it != prisoners.end(); it++)
        {
            if (it->second == phase)
            {
                if (Creature *pPrisoner = me->GetCreature(it->first))
                {
                    pPrisoner->SetAggroRange(90.0f);
                    pPrisoner->SetReactState(REACT_AGGRESSIVE);

                    pPrisoner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    pPrisoner->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                    pPrisoner->AI()->DoZoneInCombat();
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (checkTimer.Expired(diff))
            {
                if (phase != EVENT_NULL && phase != EVENT_FIGHT)
                {
                    bool found = false;
                    for (std::map<uint64, uint8>::iterator it = prisoners.begin(); it != prisoners.end(); it++)
                    {
                        if (it->second == phase)
                        {
                            if (Creature *pPrisoner = me->GetCreature(it->first))
                            {
                                if (pPrisoner->isAlive())
                                {
                                    found = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!found)
                        DoAction(uint8(phase) +1);

                    if(HeroicMode)
                    {
                        if(SummonTimer.Expired(diff))
                        {
                            DoAction(uint8(phase) +1);
                            SummonTimer = 120000;
                        }
                    }
                }
                checkTimer = 2000;
            }

            return;
        }

        if (AcidSpray_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(),SPELL_SLIME_SPRAY);
            AcidSpray_Timer = urand(12000, 18000);
        }

        if (PoisonBolt_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_POISON_BOLT);
            PoisonBolt_Timer = urand(8000, 14000);;
        }

        if (PoisonSpawn_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_POISON_CLOUD);
            PoisonSpawn_Timer = HeroicMode ? urand(4000, 8000) : urand(10000, 20000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_broggokAI(Creature *_Creature)
{
    return new boss_broggokAI (_Creature);
}

bool GOUse_go_broggok_lever(Player* pPlayer, GameObject* pGo)
{
    if(InstanceData *pInstance = pGo->GetInstanceData())
    {
        if (pInstance->GetData(DATA_BROGGOKEVENT) == IN_PROGRESS)
            return true;

        if (Creature *pBoss = GetClosestCreatureWithEntry(pPlayer, 17380, 200.0f))
            pBoss->AI()->DoAction(EVENT_1_CAGE);
    }
    pGo->UseDoorOrButton(5);
    return true;
}

struct mob_broggok_poisoncloudAI : public Scripted_NoMovementAI
{
    mob_broggok_poisoncloudAI(Creature *c) : Scripted_NoMovementAI(c) {}

    Timer Timer_Poison;

    void Reset()
    {
        Timer_Poison.Reset(1000);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
        SetCombatMovement(false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(Timer_Poison.Expired(diff))
        {
            DoCast(me, SPELL_POISON, true);
            Timer_Poison = 0;
        }
    }
};

CreatureAI* GetAI_mob_broggok_poisoncloudAI(Creature *_Creature)
{
    return new mob_broggok_poisoncloudAI(_Creature);
}

void AddSC_boss_broggok()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_broggok";
    newscript->GetAI = &GetAI_boss_broggokAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_broggok_lever";
    newscript->pGOUse = &GOUse_go_broggok_lever;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_broggok_poisoncloud";
    newscript->GetAI = &GetAI_mob_broggok_poisoncloudAI;
    newscript->RegisterSelf();
}
