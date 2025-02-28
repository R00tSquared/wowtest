// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Amnennar_the_coldbringer
SD%Complete: 100
SDComment:
SDCategory: Razorfen Downs
EndScriptData */

#include "precompiled.h"

#define SAY_0             -1200437
#define SAY_1             -1200438
#define SAY_SLAY          -1200439
#define SOUND_AGGRO       5825
#define SOUND_SLAY        5826
#define SOUND_SUMMON      5829

#define SPELL_AMNENNARSWRATH        13009
#define SPELL_FROSTBOLT             10179
#define SPELL_FROST_NOVA            15531 

struct boss_amnennar_the_coldbringerAI : public ScriptedAI
{
    boss_amnennar_the_coldbringerAI(Creature *c) : ScriptedAI(c), summons(m_creature) {}

    Timer AmnenarsWrath_Timer;
    Timer FrostBolt_Timer;
    Timer FrostNova_Timer;
    bool Spectrals;
    bool Spectrals2;
    int Rand;
    int RandX;
    int RandY;
    SummonList summons;

    void Reset()
    {
        ClearCastQueue();

        AmnenarsWrath_Timer.Reset(8000);
        FrostBolt_Timer.Reset(1000);
        FrostNova_Timer.Reset (4000);
        Spectrals = false;
        Spectrals2 = false;
        summons.DespawnAll();
    }

    void EnterCombat(Unit *who)
    {
        DoYell(-1200437,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(me,SOUND_AGGRO);
    }

    void KilledUnit()
    {
        DoYell(SAY_SLAY, LANG_UNIVERSAL, NULL);
        DoPlaySoundToSet(me, SOUND_SLAY);
    }

    void SummonSpectrals(Unit* victim)
    {
        Rand = rand()%5;
        switch (rand()%2)
        {
            case 0: RandX = 0 - Rand; break;
            case 1: RandX = 0 + Rand; break;
        }
        Rand = 0;
        Rand = rand()%5;
        switch (rand()%2)
        {
            case 0: RandY = 0 - Rand; break;
            case 1: RandY = 0 + Rand; break;
        }
        Rand = 0;

        if (Creature* Summoned = DoSpawnCreature(8585, RandX, RandY, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 60000))
            ((CreatureAI*)Summoned->AI())->AttackStart(victim);
    }
    void JustSummoned(Creature* summoned)
    {
        summons.Summon(summoned);
    }

    void JustDied(Unit* killer)
    {
        summons.DespawnAll();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (AmnenarsWrath_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(),SPELL_AMNENNARSWRATH);
            AmnenarsWrath_Timer = 12000;
        }

        if (FrostBolt_Timer.Expired(diff))
        {
            Unit *pTarget = NULL;
            pTarget = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (pTarget)
                AddSpellToCast(pTarget,SPELL_FROSTBOLT);

            FrostBolt_Timer = 8000;
        }

        if (FrostNova_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_FROST_NOVA);
            FrostNova_Timer = 15000;
        }
        
        if (!Spectrals && me->GetHealthPercent() <= 60)
        {
            DoYell(-1200438, LANG_UNIVERSAL, NULL);
            DoPlaySoundToSet(me, SOUND_SUMMON);

            Unit *pTarget = NULL;
            pTarget = SelectUnit(SELECT_TARGET_RANDOM,0);

            if (pTarget)
            {
                SummonSpectrals(pTarget);
                SummonSpectrals(pTarget);
                SummonSpectrals(pTarget);
            }
            Spectrals = true;
        }

        if (!Spectrals2 && me->GetHealthPercent() <= 30)
        {
            DoYell(-1200438, LANG_UNIVERSAL, NULL);
            DoPlaySoundToSet(me, SOUND_SUMMON);

            Unit *pTarget = NULL;
            pTarget = SelectUnit(SELECT_TARGET_RANDOM,0);

            if (pTarget)
            {
                SummonSpectrals(pTarget);
                SummonSpectrals(pTarget);
                SummonSpectrals(pTarget);
            }
            Spectrals2 = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_amnennar_the_coldbringer(Creature* pCreature)
{
    return new boss_amnennar_the_coldbringerAI (pCreature);
}

void AddSC_boss_amnennar_the_coldbringer()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_amnennar_the_coldbringer";
    newscript->GetAI = &GetAI_boss_amnennar_the_coldbringer;
    newscript->RegisterSelf();
}
