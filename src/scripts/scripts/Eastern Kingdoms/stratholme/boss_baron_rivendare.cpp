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
SDName: Boss_Baron_Rivendare
SD%Complete: 70
SDComment: aura applied/defined in database
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "def_stratholme.h"

//#define SAY_0    "Intruders! More pawns of the Argent Dawn, no doubt. I already count one of their number among my prisoners. Withdraw from my domain before she is executed!"
//#define SAY_1    "You're still here? Your foolishness is amusing! The Argent Dawn wench needn't suffer in vain. Leave at once and she shall be spared!"
//#define SAY_2    "I shall take great pleasure in taking this poor wretch's life! It's not too late, she needn't suffer in vain. Turn back and her death shall be merciful!"
//#define SAY_3    "May this prisoner's death serve as a warning. None shall defy the Scourge and live!"
//#define SAY_4    "So you see fit to toy with the Lich King's creations? Ramstein, be sure to give the intruders a proper greeting."
//#define SAY_5    "Time to take matters into my own hands. Come. Enter my domain and challenge the might of the Scourge!"

#define ADD_1X 4017.403809
#define ADD_1Y -3339.703369
#define ADD_1Z 115.057655
#define ADD_1O 5.487860

#define ADD_2X 4013.189209
#define ADD_2Y -3351.808350
#define ADD_2Z 115.052254
#define ADD_2O 0.134280

#define ADD_3X 4017.738037
#define ADD_3Y -3363.478016
#define ADD_3Z 115.057274
#define ADD_3O 0.723313

#define ADD_4X 4048.877197
#define ADD_4Y -3363.223633
#define ADD_4Z 115.054253
#define ADD_4O 3.627735

#define ADD_5X 4051.777588
#define ADD_5Y -3350.893311
#define ADD_5Z 115.055351
#define ADD_5O 3.066176

#define ADD_6X 4048.375977
#define ADD_6Y -3339.966309
#define ADD_6Z 115.055222
#define ADD_6O 2.457497

#define SPELL_SHADOWBOLT    17393
#define SPELL_CLEAVE        15284
#define SPELL_MORTALSTRIKE  15708

#define SPELL_UNHOLY_AURA   17467
#define SPELL_RAISEDEAD     17473                           //triggers death pact (17471)

#define SPELL_RAISE_DEAD1   17475
#define SPELL_RAISE_DEAD2   17476
#define SPELL_RAISE_DEAD3   17477
#define SPELL_RAISE_DEAD4   17478
#define SPELL_RAISE_DEAD5   17479
#define SPELL_RAISE_DEAD6   17480

struct boss_baron_rivendareAI : public ScriptedAI
{
    boss_baron_rivendareAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)m_creature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameShadowBolt_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameMortalStrike_Timer;
    //    Timer _ChangedNameRaiseDead_Timer;
    Timer _ChangedNameSummonSkeletons_Timer;

    void Reset()
    {
        _ChangedNameShadowBolt_Timer.Reset(5000);
        _ChangedNameCleave_Timer.Reset(8000);
        _ChangedNameMortalStrike_Timer.Reset(12000);
        //        _ChangedNameRaiseDead_Timer.Reset(30000);
        _ChangedNameSummonSkeletons_Timer.Reset(34000);
        if (pInstance)
            pInstance->SetData(TYPE_BARON,NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(TYPE_BARON,IN_PROGRESS);
    }

    void JustSummoned(Creature* summoned)
    {
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
             summoned->AI()->AttackStart(target);
    }

     void JustDied(Unit* Killer)
     {
         if (pInstance)
             pInstance->SetData(TYPE_BARON,DONE);
     }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameShadowBolt_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                DoCast(m_creature->GetVictim(),SPELL_SHADOWBOLT);

            _ChangedNameShadowBolt_Timer = 10000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            //13 seconds until we should cast this again
            _ChangedNameCleave_Timer = 7000 + (rand()%10000);
        }

        if (_ChangedNameMortalStrike_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MORTALSTRIKE);
            _ChangedNameMortalStrike_Timer = 10000 + (rand()%15000);
        }

        //            if (_ChangedNameRaiseDead_Timer.Expired(diff))
        //          {
        //      DoCast(m_creature,SPELL_RAISEDEAD);
        //                _ChangedNameRaiseDead_Timer = 45000;
        //            }

        if (_ChangedNameSummonSkeletons_Timer.Expired(diff))
        {
            m_creature->SummonCreature(11197,ADD_1X,ADD_1Y,ADD_1Z,ADD_1O,TEMPSUMMON_TIMED_DESPAWN,29000);
            m_creature->SummonCreature(11197,ADD_2X,ADD_2Y,ADD_2Z,ADD_2O,TEMPSUMMON_TIMED_DESPAWN,29000);
            m_creature->SummonCreature(11197,ADD_3X,ADD_3Y,ADD_3Z,ADD_3O,TEMPSUMMON_TIMED_DESPAWN,29000);
            m_creature->SummonCreature(11197,ADD_4X,ADD_4Y,ADD_4Z,ADD_4O,TEMPSUMMON_TIMED_DESPAWN,29000);
            m_creature->SummonCreature(11197,ADD_5X,ADD_5Y,ADD_5Z,ADD_5O,TEMPSUMMON_TIMED_DESPAWN,29000);
            m_creature->SummonCreature(11197,ADD_6X,ADD_6Y,ADD_6Z,ADD_6O,TEMPSUMMON_TIMED_DESPAWN,29000);

            //34 seconds until we should cast this again
            _ChangedNameSummonSkeletons_Timer = 34000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_baron_rivendare(Creature *_Creature)
{
    return new boss_baron_rivendareAI (_Creature);
}

void AddSC_boss_baron_rivendare()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_baron_rivendare";
    newscript->GetAI = &GetAI_boss_baron_rivendare;
    newscript->RegisterSelf();
}


