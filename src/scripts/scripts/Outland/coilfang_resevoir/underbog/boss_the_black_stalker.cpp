// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_the_black_stalker
SD%Complete: 95
SDComment: Timers may be incorrect
SDCategory: Coilfang Resevoir, Underbog
EndScriptData */

#include "precompiled.h"
#include "def_underbog.h"

enum
{
    SPELL_LEVITATE             = 31704,
    SPELL_SUSPENSION           = 31719,
    SPELL_LEVITATION_PULSE     = 31701,
    SPELL_MAGNETIC_PULL        = 31705,
    SPELL_CHAIN_LIGHTNING      = 31717,
    SPELL_STATIC_CHARGE        = 31715,
    SPELL_SUMMON_SPORE_STRIDER = 38755,
    ENTRY_SPORE_STRIDER        = 22299
    
};

struct boss_the_black_stalkerAI : public ScriptedAI
{
    boss_the_black_stalkerAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = me->GetMap()->IsHeroic();
        c->GetPosition(wLoc);
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    WorldLocation wLoc;
    bool HeroicMode;
    Timer_UnCheked SporeStriders_Timer;
    Timer_UnCheked Levitate_Timer;
    Timer_UnCheked ChainLightning_Timer;
    Timer_UnCheked StaticCharge_Timer;
    uint64 LevitatedTarget;
    Timer_UnCheked LevitatedTarget_Timer;
    bool InAir;
    Timer_UnCheked check_Timer;
    std::list<uint64> Striders;

    void Reset()
    {
        Levitate_Timer.Reset(12000);
        ChainLightning_Timer.Reset(6000);
        StaticCharge_Timer.Reset(10000);
        SporeStriders_Timer.Reset(10000 + rand() % 5000);
        check_Timer.Reset(5000);
        LevitatedTarget = 0;
        LevitatedTarget_Timer = 0;
        Striders.clear();
        if(pInstance)
            pInstance->SetData(TYPE_BLACKSTALKER, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(TYPE_BLACKSTALKER, IN_PROGRESS);
    }

    void JustSummoned(Creature *summon)
    {
        if(summon && summon->GetEntry() == ENTRY_SPORE_STRIDER)
        {
            Striders.push_back(summon->GetGUID());
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true, me->getVictimGUID()))
                summon->AI()->AttackStart(target);
            else
                if(me->GetVictim())
                    summon->AI()->AttackStart(me->GetVictim());
        }
    }

    void JustDied(Unit *who)
    {
        for(std::list<uint64>::iterator i = Striders.begin(); i != Striders.end(); ++i)
        {
            if(Creature *strider = Unit::GetCreature(*me, *i))
            {
                strider->SetLootRecipient(NULL);
                strider->DealDamage(strider,strider->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                strider->RemoveCorpse();
            }
        }
        if(pInstance)
        {
            if(HeroicMode)
                me->SetCorpseDelay(3600000);
            pInstance->SetData(TYPE_BLACKSTALKER, DONE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // Evade if too far
        if (check_Timer.Expired(diff))
        {
            if(!me->IsWithinDistInMap(&wLoc, 60))
            {
                EnterEvadeMode();
                return;
            }
            check_Timer = 1000;
        }

        // Spore Striders
        if (HeroicMode && SporeStriders_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SUMMON_SPORE_STRIDER);
            SporeStriders_Timer = 10000+rand()%5000;
        }

        // Levitate
        if(LevitatedTarget)
        {
            if (LevitatedTarget_Timer.Expired(diff))
            {
                if(Unit* target = (Unit*)Unit::GetUnit(*me, LevitatedTarget))
                {
                    if(!target->HasAura(SPELL_LEVITATE,0))
                    {
                        LevitatedTarget = 0;
                        return;
                    }
                    if(InAir)
                    {
                        target->AddAura(SPELL_SUSPENSION, target);
                        LevitatedTarget = 0;
                    }
                    else
                    {
                        target->CastSpell(target, SPELL_MAGNETIC_PULL, true);
                        InAir = true;
                        LevitatedTarget_Timer = 1500;
                    }
                }
                else
                    LevitatedTarget = 0;
            }
        }
        if (Levitate_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true, me->getVictimGUID()))
            {
                AddSpellToCast(target, SPELL_LEVITATE);
                LevitatedTarget = target->GetGUID();
                LevitatedTarget_Timer = 2000;
                InAir = false;
            }
            Levitate_Timer = 12000+rand()%3000;
        }

        // Chain Lightning
        if (ChainLightning_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0))
                AddSpellToCast(target, SPELL_CHAIN_LIGHTNING);
            ChainLightning_Timer = 7000;
        }

        // Static Charge
        if (StaticCharge_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0,30,true))
                AddSpellToCast(target, SPELL_STATIC_CHARGE);
            StaticCharge_Timer = 10000;
        }

        CastNextSpellIfAnyAndReady();

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_the_black_stalker(Creature *_Creature)
{
    return new boss_the_black_stalkerAI (_Creature);
}

void AddSC_boss_the_black_stalker()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_the_black_stalker";
    newscript->GetAI = &GetAI_boss_the_black_stalker;
    newscript->RegisterSelf();
}

