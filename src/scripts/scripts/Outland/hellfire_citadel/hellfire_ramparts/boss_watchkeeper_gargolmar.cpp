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
SDName: Boss_Watchkeeper_Gargolmar
SD%Complete: 80
SDComment: Healing adds script implemented via DB.
SDCategory: Hellfire Citadel, Hellfire Ramparts
EndScriptData */

#include "precompiled.h"
#include "hellfire_ramparts.h"

enum WatchkeeperGargolmar
{
    SAY_TAUNT               = -1543000,
    SAY_HEAL                = -1543001,
    SAY_SURGE               = -1543002,
    SAY_AGGRO_1             = -1543003,
    SAY_AGGRO_2             = -1543004,
    SAY_AGGRO_3             = -1543005,
    SAY_KILL_1              = -1543006,
    SAY_KILL_2              = -1543007,
    SAY_DIE                 = -1543008,
    
    SPELL_MORTAL_WOUND      = 30641,
    H_SPELL_MORTAL_WOUND    = 36814,
    SPELL_SURGE             = 34645,
    SPELL_RETALIATION       = 22857,
    SPELL_OVERPOWER         = 32154,
    SPELL_CHARGE_VISUAL     = 40602
};

struct boss_watchkeeper_gargolmarAI : public ScriptedAI
{
    boss_watchkeeper_gargolmarAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;

    bool HeroicMode;

    Timer Surge_Timer;
    Timer MortalWound_Timer;
    Timer Retaliation_Timer;
    Timer Overpower_Timer;

    bool HasTaunted;
    bool YelledForHeal;

    void Reset()
    {
        Surge_Timer.Reset(5000);
        MortalWound_Timer.Reset(4000);
        Retaliation_Timer = 0;
        Overpower_Timer = 0;

        HasTaunted = false;
        YelledForHeal = false;

        if (pInstance)
            pInstance->SetData(DATA_GARGOLMAR, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), me);

        if (pInstance)
            pInstance->SetData(DATA_GARGOLMAR, IN_PROGRESS);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if (!me->GetVictim() && who->isTargetableForAttack() && ( me->IsHostileTo( who )) && who->isInAccessiblePlacefor(me) )
        {
            if (!me->CanFly() && me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                return;

            float attackRadius = me->GetAttackDistance(who);
            if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
            {
                AttackStart(who);
            }
            else if (!HasTaunted && me->IsWithinDistInMap(who, 60.0f))
            {
                DoScriptText(SAY_TAUNT, me);
                HasTaunted = true;
            }
        }
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DIE, me);

        // Can't find any info about their despawn after boss death
        /*std::list<Creature*> helpers = FindAllCreaturesWithEntry(17309, 100);
        for(std::list<Creature *>::iterator i = helpers.begin(); i != helpers.end(); i++)
        {
            (*i)->ForcedDespawn(500);
        }*/
        if (pInstance)
            pInstance->SetData(DATA_GARGOLMAR, DONE);
    }

    void DoMeleeAttackIfReady()
    {
        if (!me->IsNonMeleeSpellCast(false))
        {
            if (me->isAttackReady() && me->IsWithinMeleeRange(me->GetVictim()))
            {
                if (!Overpower_Timer.GetInterval())
                {
                    uint32 health = me->GetVictim()->GetHealth();
                    me->AttackerStateUpdate(me->GetVictim());
                    if (me->GetVictim() && health == me->GetVictim()->GetHealth())
                    {
                        DoCast(me->GetVictim(), SPELL_OVERPOWER, false);
                        Overpower_Timer = 5000;
                    }
                }
                else me->AttackerStateUpdate(me->GetVictim());
                me->resetAttackTimer();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (MortalWound_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),HeroicMode ? H_SPELL_MORTAL_WOUND : SPELL_MORTAL_WOUND);
            MortalWound_Timer = urand(5000, 13000);
        }

        if (Surge_Timer.Expired(diff))
        {
            DoScriptText(SAY_SURGE, me);

            if (Unit* target = SelectUnit(SELECT_TARGET_FARTHEST,0,GetSpellMaxRange(SPELL_SURGE), true))
            {
                DoCast(target, SPELL_CHARGE_VISUAL,true);
                DoCast(target, SPELL_SURGE, true);
            }
            Surge_Timer = 5000+rand()%8000;
        }

        if (Overpower_Timer.Expired(diff))
        {
            // implemented in DoMeleeAttackIfReady()
            Overpower_Timer = 0;
        }

        if (me->GetHealthPercent() < 20)
        {
            if (Retaliation_Timer.Expired(diff))
            {
                DoCast(me,SPELL_RETALIATION);
                Retaliation_Timer = 30000;
            }
        }

        if (!YelledForHeal)
        {
            if (me->GetHealthPercent() < 40)
            {
                DoScriptText(SAY_HEAL, me);
                YelledForHeal = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_watchkeeper_gargolmarAI(Creature *_Creature)
{
    return new boss_watchkeeper_gargolmarAI (_Creature);
}

void AddSC_boss_watchkeeper_gargolmar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_watchkeeper_gargolmar";
    newscript->GetAI = &GetAI_boss_watchkeeper_gargolmarAI;
    newscript->RegisterSelf();
}

