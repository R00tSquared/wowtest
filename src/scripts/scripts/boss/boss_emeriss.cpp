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
SDName: Emeriss
SD%Complete: 90
SDComment: Teleport function & Mark of Nature missing
SDCategory: Bosses
EndScriptData */

#include "precompiled.h"

#define SPELL_SLEEP                24777
#define SPELL_NOXIOUSBREATH        24818
#define SPELL_TAILSWEEP            15847
//#define SPELL_MARKOFNATURE         25040   // Not working
#define SPELL_VOLATILEINFECTION    24928
#define SPELL_CORRUPTIONOFEARTH    24910

struct boss_emerissAI : public ScriptedAI
{
    boss_emerissAI(Creature *c) : ScriptedAI(c) {}

    Timer Sleep_Timer;
    Timer NoxiousBreath_Timer;
    Timer TailSweep_Timer;
    //int32 MarkOfNature_Timer;
    Timer VolatileInfection_Timer;
    Timer CorruptionofEarth1_Timer;
    Timer CorruptionofEarth2_Timer;
    Timer CorruptionofEarth3_Timer;

    void Reset()
    {
        Sleep_Timer.Reset(15000 + rand()%5000);
        NoxiousBreath_Timer.Reset(8000);
        TailSweep_Timer.Reset(4000);
        //MarkOfNature_Timer = 45000;
        VolatileInfection_Timer.Reset(12000);
        CorruptionofEarth1_Timer.Reset(1);
        CorruptionofEarth2_Timer.Reset(1);
        CorruptionofEarth3_Timer.Reset(1);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (Sleep_Timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_SLEEP);

            Sleep_Timer = 8000 + rand()%8000;
        }
        
        if (NoxiousBreath_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_NOXIOUSBREATH);
            NoxiousBreath_Timer = 14000 + rand()%6000;
        }

        if (TailSweep_Timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_TAILSWEEP);

            TailSweep_Timer = 2000;
        }
        
        //MarkOfNature_Timer -= diff;
        //if (MarkOfNature_Timer <= diff)
        //{
        //    DoCast(m_creature->GetVictim(),SPELL_MARKOFNATURE);
        //    MarkOfNature_Timer += 45000;
        //}

        if (VolatileInfection_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_VOLATILEINFECTION);
            VolatileInfection_Timer = 7000 + rand()%5000;
        }
        

        // Trentone, maybe it is < 75???
        //CorruptionofEarth_Timer
        if ( (int) (me->GetHealthPercent() +0.5) == 75)
        {
            if (CorruptionofEarth1_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_CORRUPTIONOFEARTH);

                //1 minutes for next one. Means not again with this health value
                CorruptionofEarth1_Timer = 60000;
            }


        }

        //CorruptionofEarth_Timer
        if ( (int) (me->GetHealthPercent() +0.5) == 50)
        {
            if (CorruptionofEarth2_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_CORRUPTIONOFEARTH);

                //1 minutes for next one. Means not again with this health value
                CorruptionofEarth2_Timer = 60000;
            }
        }

        //CorruptionofEarth_Timer
        if ( (int) (me->GetHealthPercent() +0.5) == 25)
        {
            if (CorruptionofEarth3_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_CORRUPTIONOFEARTH);

                //1 minutes for next one. Means not again with this health value
                CorruptionofEarth3_Timer = 60000;
            }
            
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_emeriss(Creature *_Creature)
{
    return new boss_emerissAI (_Creature);
}

void AddSC_boss_emeriss()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_emeriss";
    newscript->GetAI = &GetAI_boss_emeriss;
    newscript->RegisterSelf();
}

