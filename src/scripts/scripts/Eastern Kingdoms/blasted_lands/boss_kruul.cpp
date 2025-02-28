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
SDName: Boss_Kruul
SD%Complete: 100
SDComment: Highlord Kruul are presumably no longer in-game on regular bases, however future events could bring him back.
SDCategory: Bosses
EndScriptData */

#include "precompiled.h"
#include "GameEvent.h"

#define SPELL_SHADOWVOLLEY          21341
#define SPELL_CLEAVE                20677
#define SPELL_THUNDERCLAP           23931
#define SPELL_TWISTEDREFLECTION     21063
#define SPELL_VOIDBOLT              21066
#define SPELL_RAGE                  21340
#define SPELL_CAPTURESOUL           21054

struct boss_kruulAI : public ScriptedAI
{
    boss_kruulAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameShadowVolley_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameThunderClap_Timer;
    Timer _ChangedNameTwistedReflection_Timer;
    Timer _ChangedNameVoidBolt_Timer;
    Timer _ChangedNameRage_Timer;
    Timer _ChangedNameHound_Timer;
    int Rand;
    int RandX;
    int RandY;

    void Reset()
    {
        _ChangedNameShadowVolley_Timer.Reset(10000);
        _ChangedNameCleave_Timer.Reset(14000);
        _ChangedNameThunderClap_Timer.Reset(20000);
        _ChangedNameTwistedReflection_Timer.Reset(25000);
        _ChangedNameVoidBolt_Timer.Reset(30000);
        _ChangedNameRage_Timer.Reset(60000);                                 //Cast rage after 1 minute
        _ChangedNameHound_Timer.Reset(8000);
    }

    void JustDied(Unit* victim)
    {
        CreateEventChest(victim);
    }

    void EnterCombat(Unit *who)
    {
        MultiboxCheck();
    }

    void KilledUnit()
    {
        // When a player, pet or totem gets killed, Lord Kazzak casts this spell to instantly regenerate 70,000 health.
        DoCast(m_creature,SPELL_CAPTURESOUL);

    }

    void SummonHounds(Unit* victim)
    {
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
        if(Creature* Summoned = DoSpawnCreature(19207, RandX, RandY, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
            ((CreatureAI*)Summoned->AI())->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameShadowVolley_Timer.Expired(diff))
        {
            if (rand()%100 < 46)
            {
                DoCast(m_creature->GetVictim(),SPELL_SHADOWVOLLEY);
            }

            _ChangedNameShadowVolley_Timer = 5000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            if (rand()%100 < 50)
            {
                DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            }

            _ChangedNameCleave_Timer = 10000;

            MultiboxCheck();
        }

        if (_ChangedNameThunderClap_Timer.Expired(diff))
        {
            if (rand()%100 < 20)
            {
                DoCast(m_creature->GetVictim(),SPELL_THUNDERCLAP);
            }

            _ChangedNameThunderClap_Timer = 12000;
        }

        if (_ChangedNameTwistedReflection_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_TWISTEDREFLECTION);
            _ChangedNameTwistedReflection_Timer = 30000;
        }

        if (_ChangedNameVoidBolt_Timer.Expired(diff))
        {
            if (rand()%100 < 40)
            {
                DoCast(m_creature->GetVictim(),SPELL_VOIDBOLT);
            }

            _ChangedNameVoidBolt_Timer = 18000;
        }

        if (_ChangedNameRage_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_RAGE);
            _ChangedNameRage_Timer = 70000;
        }

        if (_ChangedNameHound_Timer.Expired(diff))
        {
            SummonHounds(m_creature->GetVictim());
            SummonHounds(m_creature->GetVictim());
            SummonHounds(m_creature->GetVictim());

            _ChangedNameHound_Timer = 45000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_kruul(Creature *_Creature)
{
    return new boss_kruulAI (_Creature);
}

void AddSC_boss_kruul()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_kruul";
    newscript->GetAI = &GetAI_boss_kruul;
    newscript->RegisterSelf();
}


