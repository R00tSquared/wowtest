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
SDName: Boss_Gatewatcher_Gyrokill
SD%Complete: 90
SDComment:
SDCategory: Tempest Keep, The Mechanar
EndScriptData */

#include "precompiled.h"
#include "def_mechanar.h"

#define SAY_AGGRO                       -1554000
#define SAY_SAW_ATTACK1                 -1554001
#define SAY_SAW_ATTACK2                 -1554002
#define SAY_SLAY1                       -1554003
#define SAY_SLAY2                       -1554004
#define SAY_DEATH                       -1554005

#define SPELL_STREAM_OF_MACHINE_FLUID   35311
#define SPELL_SAW_BLADE                 35318
#define H_SPELL_SAW_BLADE               39192
#define SPELL_SHADOW_POWER              35322
#define H_SPELL_SHADOW_POWER            39193

struct boss_gatewatcher_gyro_killAI : public ScriptedAI
{
    boss_gatewatcher_gyro_killAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance *pInstance;

    bool HeroicMode;

    Timer_UnCheked Shadow_Power_Timer;
    Timer_UnCheked Saw_Blade_Timer;
    Timer_UnCheked Stream_of_Machine_Fluid_Timer;

    void Reset()
    {
        Shadow_Power_Timer.Reset(25000);
        Saw_Blade_Timer.Reset(35000);
        Stream_of_Machine_Fluid_Timer.Reset(50000);

        if(pInstance)
            pInstance->SetData(DATA_GYROKILL_EVENT, NOT_STARTED);

    }
    void EnterCombat(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(DATA_GYROKILL_EVENT, IN_PROGRESS);

        DoScriptText(SAY_AGGRO, me);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);

        if (!pInstance)
            return;

        if(pInstance)
            pInstance->SetData(DATA_GYROKILL_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        //Shadow Power
        if(Shadow_Power_Timer.Expired(diff))
        {
            DoCast(me,HeroicMode ? H_SPELL_SHADOW_POWER : SPELL_SHADOW_POWER);
            Shadow_Power_Timer = 20000 + rand()%8000;
        }

        //Saw Blade
        if(Saw_Blade_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),HeroicMode ? H_SPELL_SAW_BLADE : SPELL_SAW_BLADE);
            DoScriptText(RAND(SAY_SAW_ATTACK1, SAY_SAW_ATTACK2), me);

            Saw_Blade_Timer = 30000;
        }

        //Stream of Machine Fluid
        if(Stream_of_Machine_Fluid_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_STREAM_OF_MACHINE_FLUID);
            Stream_of_Machine_Fluid_Timer = 35000 + rand()%15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_gatewatcher_gyro_kill(Creature *_Creature)
{
    return new boss_gatewatcher_gyro_killAI (_Creature);
}

void AddSC_boss_gatewatcher_gyro_kill()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gatewatcher_gyro_kill";
    newscript->GetAI = &GetAI_boss_gatewatcher_gyro_kill;
    newscript->RegisterSelf();
}

