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
SDName: Boss_Landslide
SD%Complete: 100
SDComment:
SDCategory: Maraudon
EndScriptData */

#include "precompiled.h"

#define SPELL_KNOCKAWAY         18670
#define SPELL_TRAMPLE           5568
#define SPELL_LANDSLIDE         21808

struct boss_landslideAI : public ScriptedAI
{
    boss_landslideAI(Creature *c) : ScriptedAI(c) {}

    Timer KnockAway_Timer;
    Timer Trample_Timer;
    Timer Landslide_Timer;

    void Reset()
    {
        KnockAway_Timer.Reset(8000);
        Trample_Timer.Reset(2000);
        Landslide_Timer.Reset(1);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        if (KnockAway_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_KNOCKAWAY);
            KnockAway_Timer = 15000;
        }

        if (Trample_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_TRAMPLE);
            Trample_Timer = 8000;
        }

        //Landslide
        if ( me->GetHealthPercent() < 50 )
        {
            if (Landslide_Timer.Expired(diff))
            {
                m_creature->InterruptNonMeleeSpells(false);
                DoCast(m_creature,SPELL_LANDSLIDE);
                Landslide_Timer = 60000;
            } 
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_landslide(Creature *_Creature)
{
    return new boss_landslideAI (_Creature);
}

void AddSC_boss_landslide()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_landslide";
    newscript->GetAI = &GetAI_boss_landslide;
    newscript->RegisterSelf();
}

