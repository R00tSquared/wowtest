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
SDName: Boss_Marli
SD%Complete: 80
SDComment: Charging healers and casters not working. Perhaps wrong Spell Timers.
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SAY_AGGRO               -1309005
#define SAY_TRANSFORM           -1309006
#define SAY_SPIDER_SPAWN        -1309007
#define SAY_DEATH               -1309008

#define SPELL_CHARGE              22911
#define SPELL_ASPECT_OF_MARLI     24686                     // A stun spell
#define SPELL_ENVOLWINGWEB        24110
#define SPELL_POISONVOLLEY        24099
#define SPELL_SPIDER_FORM         24084

//The Spider Spells
#define SPELL_LEVELUP             24312                     //Not right Spell.

struct boss_marliAI : public ScriptedAI
{
    boss_marliAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer _ChangedNameSpawnStartSpiders_Timer;
    Timer _ChangedNamePoisonVolley_Timer;
    Timer _ChangedNameSpawnSpider_Timer;
    Timer _ChangedNameCharge_Timer;
    Timer _ChangedNameAspect_Timer;
    Timer _ChangedNameTransform_Timer;
    Timer _ChangedNameTransformBack_Timer;

    bool Spawned;
    bool PhaseTwo;

    void Reset()
    {
        _ChangedNameSpawnStartSpiders_Timer.Reset(1000);
        _ChangedNamePoisonVolley_Timer.Reset(15000);
        _ChangedNameSpawnSpider_Timer.Reset(30000);
        _ChangedNameCharge_Timer.Reset(1500);
        _ChangedNameAspect_Timer.Reset(12000);
        _ChangedNameTransform_Timer.Reset(45000);
        _ChangedNameTransformBack_Timer.Reset(25000);

        Spawned = false;
        PhaseTwo = false;
        pInstance->SetData(DATA_MARLIEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        pInstance->SetData(DATA_MARLIEVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);
        if(pInstance)
            pInstance->SetData(DATA_MARLIEVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(m_creature->GetVictim() && m_creature->isAlive())
        {
            if(_ChangedNamePoisonVolley_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_POISONVOLLEY);
                _ChangedNamePoisonVolley_Timer = 10000 + rand()%10000;
            }


            if(!PhaseTwo && _ChangedNameAspect_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_ASPECT_OF_MARLI);
                _ChangedNameAspect_Timer = 13000 + rand()%5000;
            }



            if(!Spawned && _ChangedNameSpawnStartSpiders_Timer.Expired(diff))
            {
                DoScriptText(SAY_SPIDER_SPAWN, m_creature);

                Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if(!target)
                    return;
                
                Creature* Spider = NULL;
                for (uint32 i = 0; i < 4; ++i)
                {
                    if (Spider = m_creature->SummonCreature(15041, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                        Spider->AI()->AttackStart(target);
                }

                Spawned = true;
            }

            if(_ChangedNameSpawnSpider_Timer.Expired(diff))
            {
                Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if(!target)
                    return;

                if(Creature* Spider = m_creature->SummonCreature(15041, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                    Spider->AI()->AttackStart(target);
                _ChangedNameSpawnSpider_Timer = 12000 + rand()%5000;
            }

            if(!PhaseTwo && _ChangedNameTransform_Timer.Expired(diff))
            {
                DoScriptText(SAY_TRANSFORM, m_creature);
                DoCast(m_creature,SPELL_SPIDER_FORM);
                const CreatureInfo *cinfo = m_creature->GetCreatureInfo();
                m_creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 35)) * m_creature->GetCreatureDamageMod());
                m_creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 35)) * m_creature->GetCreatureDamageMod());
                m_creature->UpdateDamagePhysical(BASE_ATTACK);
                DoCast(m_creature->GetVictim(),SPELL_ENVOLWINGWEB);

                if(DoGetThreat(m_creature->GetVictim()))
                    DoModifyThreatPercent(m_creature->GetVictim(),-100);

                PhaseTwo = true;
                _ChangedNameTransform_Timer = 35000 + rand()%25000;
            }


            if(PhaseTwo)
            {
                if(_ChangedNameCharge_Timer.Expired(diff))
                {
                    Unit* target = NULL;
                    int i = 0 ;
                    while (i < 3)                           // max 3 tries to get a random target with power_mana
                    {
                        ++i;                                //not aggro leader
                        target = SelectUnit(SELECT_TARGET_RANDOM,1, GetSpellMaxRange(SPELL_CHARGE), true, m_creature->getVictimGUID());
                        if(target)
                            if (target->getPowerType() == POWER_MANA)
                                i=3;
                    }

                    if(target)
                    {
                        DoCast(target, SPELL_CHARGE);
                        //m_creature->Relocate(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0);
                        //m_creature->SendMonsterMove(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, true,1);
                        AttackStart(target);
                    }

                    _ChangedNameCharge_Timer = 8000;
                }


                if(_ChangedNameTransformBack_Timer.Expired(diff))
                {
                    m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,15220);
                    const CreatureInfo *cinfo = m_creature->GetCreatureInfo();
                    m_creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 1)) * m_creature->GetCreatureDamageMod());
                    m_creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 1)) * m_creature->GetCreatureDamageMod());
                    m_creature->UpdateDamagePhysical(BASE_ATTACK);

                    PhaseTwo = false;
                    _ChangedNameTransformBack_Timer = 25000 + rand()%15000;
                }
            }

            DoMeleeAttackIfReady();
        }
    }
};

//Spawn of Marli
struct mob_spawn_of_marliAI : public ScriptedAI
{
    mob_spawn_of_marliAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameLevelUp_Timer;

    void Reset()
    {
        _ChangedNameLevelUp_Timer.Reset(3000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI (const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if(_ChangedNameLevelUp_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_LEVELUP);
            _ChangedNameLevelUp_Timer = 3000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_marli(Creature *_Creature)
{
    return new boss_marliAI (_Creature);
}

CreatureAI* GetAI_mob_spawn_of_marli(Creature *_Creature)
{
    return new mob_spawn_of_marliAI (_Creature);
}

void AddSC_boss_marli()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_marli";
    newscript->GetAI = &GetAI_boss_marli;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_spawn_of_marli";
    newscript->GetAI = &GetAI_mob_spawn_of_marli;
    newscript->RegisterSelf();
}


