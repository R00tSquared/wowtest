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
SDName: Boss_Magmus
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

#define SPELL_FIERYBURST        13900
#define SPELL_WARSTOMP          24375

struct boss_magmusAI : public ScriptedAI
{
    boss_magmusAI(Creature *c) : ScriptedAI(c) {}

    Timer FieryBurst_Timer;
    Timer WarStomp_Timer;

    void Reset()
    {
        FieryBurst_Timer.Reset(5000);
        WarStomp_Timer.Reset(1);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (FieryBurst_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_FIERYBURST);
            FieryBurst_Timer = 6000;
        }
        

        //WarStomp_Timer
        if ( me->GetHealthPercent() < 51 )
        {
            if (WarStomp_Timer.Expired(diff))
            {
                DoCast(me->GetVictim(),SPELL_WARSTOMP);
                WarStomp_Timer = 8000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_magmus(Creature *creature)
{
    return new boss_magmusAI (creature);
}

void AddSC_boss_magmus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_magmus";
    newscript->GetAI = &GetAI_boss_magmus;
    newscript->RegisterSelf();
}

