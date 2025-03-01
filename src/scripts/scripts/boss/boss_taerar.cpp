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
SDName: Taerar
SD%Complete: 90
SDComment: Mark of Nature & Teleport NYI
SDCategory: Bosses
EndScriptData */

#include "precompiled.h"

//Spells of Taerar

#define SPELL_SLEEP                 24777
#define SPELL_NOXIOUSBREATH         24818
#define SPELL_TAILSWEEP             15847
//#define SPELL_MARKOFNATURE          25040                   // Not working
#define SPELL_ARCANEBLAST           24857
#define SPELL_BELLOWINGROAR         22686
#define SPELL_SUMMONSHADE           24843

//Spells of Shades of Taerar

#define SPELL_POSIONCLOUD           24840
#define SPELL_POSIONBREATH          20667

struct boss_taerarAI : public ScriptedAI
{
    boss_taerarAI(Creature *c) : ScriptedAI(c) {}

    Timer Sleep_Timer;
    Timer NoxiousBreath_Timer;
    Timer TailSweep_Timer;
    //int32 MarkOfNature_Timer;
    Timer ArcaneBlast_Timer;
    Timer BellowingRoar_Timer;
    Timer Shades_Timer;
    Timer Summon1_Timer;
    Timer Summon2_Timer;
    Timer Summon3_Timer;
    int Rand;
    int RandX;
    int RandY;
    bool Shades;

    void Reset()
    {
        Sleep_Timer.Reset(15000 + rand()%5000);
        NoxiousBreath_Timer.Reset(8000);
        TailSweep_Timer.Reset(4000);
        //MarkOfNature_Timer = 45000;
        ArcaneBlast_Timer.Reset(12000);
        BellowingRoar_Timer.Reset(30000);
        Summon1_Timer.Reset(1);
        Summon2_Timer.Reset(1);
        Summon3_Timer.Reset(1);
        Shades_Timer.Reset(60000);                               //The time that Taerar is banished
        Shades = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void SummonShades(Unit* victim)
    {
        if(!victim)
            return;

        Rand = rand()%15;
        switch (rand()%2)
        {
            case 0: RandX = 0 - Rand; break;
            case 1: RandX = 0 + Rand; break;
        }
        Rand = 0;
        Rand = rand()%15;
        switch (rand()%2)
        {
            case 0: RandY = 0 - Rand; break;
            case 1: RandY = 0 + Rand; break;
        }
        Rand = 0;
        if(Creature* Summoned = DoSpawnCreature(15302, RandX, RandY, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
            ((CreatureAI*)Summoned->AI())->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Shades)
        {
            if (Shades_Timer.Expired(diff))
            {
                //Become unbanished again
                m_creature->setFaction(14);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //m_creature->m_canMove = true;
                Shades = false;
            } 
            else
                return;
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (Sleep_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_SLEEP);

            Sleep_Timer = 8000 + rand()%7000;
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

        if (ArcaneBlast_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_ARCANEBLAST);
            ArcaneBlast_Timer = 7000 + rand()%5000;
        }
      
        if (BellowingRoar_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_BELLOWINGROAR);
            BellowingRoar_Timer = 20000 + rand()%10000;
        }
           

        //Summon 3 Shades
        if ( !Shades  && (int) (me->GetHealthPercent() +0.5) == 75)
        {
            if (Summon1_Timer.Expired(diff))
            {
                //Inturrupt any spell casting
                m_creature->InterruptNonMeleeSpells(false);
                m_creature->setFaction(35);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //m_creature->m_canMove = false;

                //Cast
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                SummonShades(target);
                SummonShades(target);
                SummonShades(target);
                Summon1_Timer = 120000;
                Shades = true;
                Shades_Timer = 60000;
            }
            
        }

        //Summon 3 Shades
        if ( !Shades  && (int) (me->GetHealthPercent() +0.5) == 50)
        {
            if (Summon2_Timer.Expired(diff))
            {
                //Inturrupt any spell casting
                m_creature->InterruptNonMeleeSpells(false);
                m_creature->setFaction(35);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //m_creature->m_canMove = false;

                //Cast
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                SummonShades(target);
                SummonShades(target);
                SummonShades(target);
                Summon2_Timer = 120000;
                Shades = true;
                Shades_Timer = 60000;
            }
        }

        //Summon 3 Shades
        if ( !Shades  && (int) (me->GetHealthPercent() +0.5) == 25)
        {
            if (Summon3_Timer.Expired(diff))
            {
                //Inturrupt any spell casting
                m_creature->InterruptNonMeleeSpells(false);
                m_creature->setFaction(35);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                //m_creature->m_canMove = false;

                //Cast
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                SummonShades(target);
                SummonShades(target);
                SummonShades(target);
                Summon3_Timer = 120000;
                Shades = true;
                Shades_Timer = 60000;
            }
        }

        DoMeleeAttackIfReady();
    }
};

// Shades of Taerar Script

struct boss_shadeoftaerarAI : public ScriptedAI
{
    boss_shadeoftaerarAI(Creature *c) : ScriptedAI(c) {}

    Timer PoisonCloud_Timer;
    Timer PosionBreath_Timer;

    void Reset()
    {
        PoisonCloud_Timer.Reset(8000);
        PosionBreath_Timer.Reset(12000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (PoisonCloud_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_POSIONCLOUD);
            PoisonCloud_Timer = 30000;
        }

        if (PosionBreath_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_POSIONBREATH);
            PosionBreath_Timer = 12000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_taerar(Creature *_Creature)
{
    return new boss_taerarAI (_Creature);
}

CreatureAI* GetAI_boss_shadeoftaerar(Creature *_Creature)
{
    return new boss_shadeoftaerarAI (_Creature);
}

void AddSC_boss_taerar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_taerar";
    newscript->GetAI = &GetAI_boss_taerar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_shade_of_taerar";
    newscript->GetAI = &GetAI_boss_shadeoftaerar;
    newscript->RegisterSelf();
}

