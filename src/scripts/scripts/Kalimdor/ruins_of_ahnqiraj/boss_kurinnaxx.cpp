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
SDName: Boss_Kurinnaxx
SD%Complete: 100
SDComment: Timers may be incorrect, not working summon
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_ruins_of_ahnqiraj.h"

#define SPELL_MORTALWOUND       25646
#define SPELL_SANDTRAP          25648 //sandtrap summoning
#define SPELL_ENRAGE            26527
#define SPELL_CLEAVE            25814
#define SPELL_THRASH_AURA       3417
#define SPELL_SUMMON            26446 //sometimes summons player in front of him

struct boss_kurinnaxxAI : public ScriptedAI
{
    boss_kurinnaxxAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    GameObject* Trap;
    Timer MORTALWOUND_Timer;
    Timer SANDTRAP_Timer;
    Timer CLEAVE_Timer;
    //uint32 SUMMON_Chance;
    uint32 i;
    bool trap;

    void Reset()
    {
        i=0;
        Trap = NULL;
        MORTALWOUND_Timer.Reset(9000); //15 sec duration
        SANDTRAP_Timer.Reset(7000); //according to videos, as I couldn't find any timers...
        CLEAVE_Timer.Reset(8000); //a guess
        trap = false;

        if (pInstance)
            pInstance->SetData(DATA_KURINNAXX, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        m_creature->CastSpell(m_creature, SPELL_THRASH_AURA, true);

        if (pInstance)
            pInstance->SetData(DATA_KURINNAXX, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_KURINNAXX, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        //If we are <30% cast enrage
        if (i==0 && me->GetHealthPercent() <= 30 && !m_creature->IsNonMeleeSpellCast(false))
        {
            i=1;
            DoCast(m_creature->GetVictim(),SPELL_ENRAGE);
        }

        if (MORTALWOUND_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MORTALWOUND);
            MORTALWOUND_Timer = 9000;
        }

        if (SANDTRAP_Timer.Expired(diff))
        {
            if (trap)
            {
                if (Trap = FindGameObject(180647, 100, m_creature))
                Trap->Delete(); //one trap at a time
            }
            if(Unit* sand_trap_target = SelectUnit(SELECT_TARGET_RANDOM, 0, 70, true))
                sand_trap_target->CastSpell(sand_trap_target, SPELL_SANDTRAP, true, 0, 0, me->GetGUID());// summon sand trap under victim

            if (!trap)
                trap = true; //at least one trap exist
            SANDTRAP_Timer = 7000;
        }

        if(CLEAVE_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_CLEAVE);
            CLEAVE_Timer = 6000 + rand()%6000;
        }
        

        /*
        if ((SUMMON_Chance = urand(0, 100))%100 == 0) //1% chance to summon enemy every update
            DoCast(m_creature->GetVictim(), SPELL_SUMMON);
            A bit buggy.
        */
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_kurinnaxx(Creature *_Creature)
{
    return new boss_kurinnaxxAI (_Creature);
}

void AddSC_boss_kurinnaxx()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_kurinnaxx";
    newscript->GetAI = &GetAI_boss_kurinnaxx;
    newscript->RegisterSelf();
}

