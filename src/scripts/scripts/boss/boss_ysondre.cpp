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
SDName: Ysondre
SD%Complete: 90
SDComment: Mark of Nature & Teleport missing
SDCategory: Bosses
EndScriptData */

#include "precompiled.h"

#define SPELL_SLEEP                24777
#define SPELL_NOXIOUSBREATH        24818
#define SPELL_TAILSWEEP            15847
//#define SPELL_MARKOFNATURE         25040                    // Not working
#define SPELL_LIGHTNINGWAVE        24819
#define SPELL_SUMMONDRUIDS         24795

//druid spells
#define SPELL_MOONFIRE             21669

// Ysondre script
struct boss_ysondreAI : public ScriptedAI
{
    boss_ysondreAI(Creature *c) : ScriptedAI(c) {}

    Timer Sleep_Timer;
    Timer NoxiousBreath_Timer;
    Timer TailSweep_Timer;
    //int32 MarkOfNature_Timer;
    Timer LightningWave_Timer;
    Timer SummonDruids1_Timer;
    Timer SummonDruids2_Timer;
    Timer SummonDruids3_Timer;
    int Rand;
    int RandX;
    int RandY;

    void Reset()
    {
        Sleep_Timer.Reset(15000 + rand()%5000);
        NoxiousBreath_Timer.Reset(8000);
        TailSweep_Timer.Reset(4000);
        //MarkOfNature_Timer = 45000;
        LightningWave_Timer.Reset(12000);
        SummonDruids1_Timer.Reset(1);
        SummonDruids2_Timer.Reset(1);
        SummonDruids3_Timer.Reset(1);
    }

    void EnterCombat(Unit *who)
    {
    }

    void SummonDruids(Unit* victim)
    {
        if(!victim)
            return;

        Rand = rand()%10;
        switch (rand()%2)
        {
            case 0: RandX = 0 - Rand; break;
            case 1: RandX = 0 + Rand; break;
        }
        Rand = 0;
        Rand = rand()%10;
        switch (rand()%2)
        {
            case 0: RandY = 0 - Rand; break;
            case 1: RandY = 0 + Rand; break;
        }
        Rand = 0;
        if(Creature* Summoned = DoSpawnCreature(15260, RandX, RandY, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
            ((CreatureAI*)Summoned->AI())->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Sleep_Timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_SLEEP);

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

        if (LightningWave_Timer.Expired(diff))
        {
            //Cast LIGHTNINGWAVE on a Random target
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_LIGHTNINGWAVE);

            LightningWave_Timer = 7000 + rand()%5000;
        }

        //Summon Druids
        if ( (int) (me->GetHealthPercent() +0.5) == 75)
        {
            if (SummonDruids1_Timer.Expired(diff))
            {
                // summon 10 druids
                Unit* target = NULL;
                for(int i = 0; i < 10;i++)
                {
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);
                    SummonDruids(target);
                }

                SummonDruids1_Timer = 60000;
            }
        }

        //Summon Druids
        if ( (int) (me->GetHealthPercent() +0.5) == 50)
        {
            if (SummonDruids2_Timer.Expired(diff))
            {
                // summon 10 druids
                Unit* target = NULL;
                for(int i = 0; i < 10;i++)
                {
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);
                    SummonDruids(target);
                }

                SummonDruids2_Timer = 60000;
            }
        }

        //Summon Druids
        if ( (int) (me->GetHealthPercent() +0.5) == 25)
        {
            if (SummonDruids3_Timer.Expired(diff))
            {
                // summon 10 druids
                Unit* target = NULL;
                for(int i = 0; i < 10;i++)
                {
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);
                    SummonDruids(target);
                }

                SummonDruids3_Timer = 60000;
            }
        }
        DoMeleeAttackIfReady();
    }
};
// Summoned druid script
struct mob_dementeddruidsAI : public ScriptedAI
{
    mob_dementeddruidsAI(Creature *c) : ScriptedAI(c) {}

    Timer MoonFire_Timer;

    void Reset()
    {
        MoonFire_Timer = 3000;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (MoonFire_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MOONFIRE);
            MoonFire_Timer = 5000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ysondre(Creature *_Creature)
{
    return new boss_ysondreAI (_Creature);
}

CreatureAI* GetAI_mob_dementeddruids(Creature *_Creature)
{
    return new mob_dementeddruidsAI (_Creature);
}

void AddSC_boss_ysondre()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_ysondre";
    newscript->GetAI = &GetAI_boss_ysondre;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_dementeddruids";
    newscript->GetAI = &GetAI_mob_dementeddruids;
    newscript->RegisterSelf();
}

