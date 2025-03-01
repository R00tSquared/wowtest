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
SDName: Boss_Gorosh_the_Dervish
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

#define SPELL_WHIRLWIND             15589
#define SPELL_MORTALSTRIKE          24573

struct boss_gorosh_the_dervishAI : public ScriptedAI
{
    boss_gorosh_the_dervishAI(Creature *c) : ScriptedAI(c) {}

    uint32 WhirlWind_Timer;
    uint32 MortalStrike_Timer;

    void Reset()
    {
        WhirlWind_Timer = 12000;
        MortalStrike_Timer = 22000;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        WhirlWind_Timer -= diff;
        if (WhirlWind_Timer <= diff)
        {
            DoCast(me,SPELL_WHIRLWIND);
            WhirlWind_Timer = 15000;
        }
        

        MortalStrike_Timer -= diff;
        if (MortalStrike_Timer <= diff)
        {
            DoCast(me->GetVictim(),SPELL_MORTALSTRIKE);
            MortalStrike_Timer = 15000;
        }

          

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_gorosh_the_dervish(Creature *creature)
{
    return new boss_gorosh_the_dervishAI (creature);
}

void AddSC_boss_gorosh_the_dervish()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gorosh_the_dervish";
    newscript->GetAI = &GetAI_boss_gorosh_the_dervish;
    newscript->RegisterSelf();
}

