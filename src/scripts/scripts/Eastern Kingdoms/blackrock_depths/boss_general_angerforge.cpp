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
SDName: Boss_General_Angerforge
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

#define SPELL_MIGHTYBLOW            14099
#define SPELL_HAMSTRING             9080
#define SPELL_CLEAVE                20691

struct boss_general_angerforgeAI : public ScriptedAI
{
    boss_general_angerforgeAI(Creature *c) : ScriptedAI(c) {}

    Timer MightyBlow_Timer;
    Timer HamString_Timer;
    Timer Cleave_Timer;
    Timer Adds_Timer;
    bool Medics;
    int Rand1;
    int Rand1X;
    int Rand1Y;
    int Rand2;
    int Rand2X;
    int Rand2Y;

    void Reset()
    {
        MightyBlow_Timer.Reset(8000);
        HamString_Timer.Reset(12000);
        Cleave_Timer.Reset(16000);
        Adds_Timer.Reset(1);
        Medics = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void SummonAdds(Unit* victim)
    {
        Rand1 = rand()%15;
        switch (rand()%2)
        {
            case 0: Rand1X = 0 - Rand1; break;
            case 1: Rand1X = 0 + Rand1; break;
        }
        Rand1 = 0;
        Rand1 = rand()%15;
        switch (rand()%2)
        {
            case 0: Rand1Y = 0 - Rand1; break;
            case 1: Rand1Y = 0 + Rand1; break;
        }
        Rand1 = 0;
        if(Creature* SummonedAdds = DoSpawnCreature(8901, Rand1X, Rand1Y, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
            ((CreatureAI*)SummonedAdds->AI())->AttackStart(victim);
    }

    void SummonMedics(Unit* victim)
    {
        Rand2 = rand()%10;
        switch (rand()%2)
        {
            case 0: Rand2X = 0 - Rand2; break;
            case 1: Rand2X = 0 + Rand2; break;
        }
        Rand2 = 0;
        Rand2 = rand()%10;
        switch (rand()%2)
        {
            case 0: Rand2Y = 0 - Rand2; break;
            case 1: Rand2Y = 0 + Rand2; break;
        }
        Rand2 = 0;
        if(Creature* SummonedMedics = DoSpawnCreature(8894, Rand2X, Rand2Y, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
            ((CreatureAI*)SummonedMedics->AI())->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (MightyBlow_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_MIGHTYBLOW);
            MightyBlow_Timer = 18000;
        }
        
        if (HamString_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_HAMSTRING);
            HamString_Timer = 15000;
        }
        
        if (Cleave_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_CLEAVE);
            Cleave_Timer = 9000;
        }
        

        //Adds_Timer
        if ( me->GetHealthPercent() < 21 )
        {
            if (Adds_Timer.Expired(diff))
            {
                // summon 3 Adds every 25s
                SummonAdds(me->GetVictim());
                SummonAdds(me->GetVictim());
                SummonAdds(me->GetVictim());

                Adds_Timer = 25000;
            }
        }

        //Summon Medics
        if ( !Medics && me->GetHealthPercent() < 21 )
        {
            SummonMedics(me->GetVictim());
            SummonMedics(me->GetVictim());
            Medics = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_general_angerforge(Creature *creature)
{
    return new boss_general_angerforgeAI (creature);
}

void AddSC_boss_general_angerforge()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_general_angerforge";
    newscript->GetAI = &GetAI_boss_general_angerforge;
    newscript->RegisterSelf();
}

