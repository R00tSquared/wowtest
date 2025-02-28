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
SDName: Boss_Overlord_Wyrmthalak
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_BLASTWAVE         11130
#define SPELL_SHOUT             23511
#define SPELL_CLEAVE            20691
#define SPELL_KNOCKAWAY         20686

#define ADD_1X -39.355381
#define ADD_1Y -513.456482
#define ADD_1Z 88.472046
#define ADD_1O 4.679872

#define ADD_2X -49.875881
#define ADD_2Y -511.896942
#define ADD_2Z 88.195160
#define ADD_2O 4.613114

struct boss_overlordwyrmthalakAI : public ScriptedAI
{
    boss_overlordwyrmthalakAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameBlastWave_Timer;
    Timer _ChangedNameShout_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameKnockaway_Timer;
    bool Summoned;

    void Reset()
    {
        _ChangedNameBlastWave_Timer.Reset(20000);
        _ChangedNameShout_Timer.Reset(2000);
        _ChangedNameCleave_Timer.Reset(6000);
        _ChangedNameKnockaway_Timer.Reset(12000);
        Summoned = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameBlastWave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_BLASTWAVE);
            _ChangedNameBlastWave_Timer = 20000;
        }

        if (_ChangedNameShout_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHOUT);
            _ChangedNameShout_Timer = 10000;
        }


        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 7000;
        }

        if (_ChangedNameKnockaway_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_KNOCKAWAY);
            _ChangedNameKnockaway_Timer = 14000;
        }

        //Summon two Beserks
        if ( !Summoned && me->GetHealthPercent() < 51 )
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);

            if (Creature* SummonedCreature = m_creature->SummonCreature(9216,ADD_1X,ADD_1Y,ADD_1Z,ADD_1O,TEMPSUMMON_TIMED_DESPAWN,300000))
                ((CreatureAI*)SummonedCreature->AI())->AttackStart(target);

            if (Creature* SummonedCreature = m_creature->SummonCreature(9268,ADD_2X,ADD_2Y,ADD_2Z,ADD_2O,TEMPSUMMON_TIMED_DESPAWN,300000))
                ((CreatureAI*)SummonedCreature->AI())->AttackStart(target);

            Summoned = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_overlordwyrmthalak(Creature *_Creature)
{
    return new boss_overlordwyrmthalakAI (_Creature);
}

void AddSC_boss_overlordwyrmthalak()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_overlord_wyrmthalak";
    newscript->GetAI = &GetAI_boss_overlordwyrmthalak;
    newscript->RegisterSelf();
}


