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
SDName: Boss_Grizzle
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

#define SPELL_GROUNDTREMOR          6524
#define SPELL_FRENZY                28371

struct boss_grizzleAI : public ScriptedAI
{
    boss_grizzleAI(Creature *c) : ScriptedAI(c) {}

    Timer GroundTremor_Timer;
    Timer Frenzy_Timer;

    void Reset()
    {
        GroundTremor_Timer.Reset(12000);
        Frenzy_Timer.Reset(1);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (GroundTremor_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_GROUNDTREMOR);
            GroundTremor_Timer = 8000;
        }


        //Frenzy_Timer
        if ( me->GetHealthPercent() < 51 )
        {
            if (Frenzy_Timer.Expired(diff))
            {
                DoCast(me,SPELL_FRENZY);
                DoTextEmote(-1200200,NULL);

                Frenzy_Timer = 15000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_grizzle(Creature *creature)
{
    return new boss_grizzleAI (creature);
}

void AddSC_boss_grizzle()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_grizzle";
    newscript->GetAI = &GetAI_boss_grizzle;
    newscript->RegisterSelf();
}

