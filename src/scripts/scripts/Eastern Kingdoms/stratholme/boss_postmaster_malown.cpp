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
SDName: boss_postmaster_malown
SD%Complete: 50
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"

//Spell ID to summon this guy is 24627 "Summon Postmaster Malown"
//He should be spawned along with three other elites once the third postbox has been opened

//#define SAY_MALOWNED    "You just got MALOWNED!"

#define SPELL_WAILINGDEAD    7713
#define SPELL_BACKHAND    6253
#define SPELL_CURSEOFWEAKNESS    8552
#define SPELL_CURSEOFTONGUES    12889
#define SPELL_CALLOFTHEGRAVE    17831

struct boss_postmaster_malownAI : public ScriptedAI
{
    boss_postmaster_malownAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameWailingDead_Timer;
    Timer _ChangedNameBackhand_Timer;
    Timer _ChangedNameCurseOfWeakness_Timer;
    Timer _ChangedNameCurseOfTongues_Timer;
    Timer _ChangedNameCallOfTheGrave_Timer;
    bool HasYelled;

    void Reset()
    {
        _ChangedNameWailingDead_Timer.Reset(19000); //lasts 6 sec
        _ChangedNameBackhand_Timer.Reset(8000); //2 sec stun
        _ChangedNameCurseOfWeakness_Timer.Reset(20000); //lasts 2 mins
        _ChangedNameCurseOfTongues_Timer.Reset(22000);
        _ChangedNameCallOfTheGrave_Timer.Reset(25000);
        HasYelled = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameWailingDead_Timer.Expired(diff))
        {
            //Cast
            if (rand()%100 < 65) //65% chance to cast
            {
                DoCast(m_creature->GetVictim(),SPELL_WAILINGDEAD);
            }
            //19 seconds until we should cast this again
            _ChangedNameWailingDead_Timer = 19000;
        }

        if (_ChangedNameBackhand_Timer.Expired(diff))
        {
            //Cast
            if (rand()%100 < 45) //45% chance to cast
            {
                DoCast(m_creature->GetVictim(),SPELL_BACKHAND);
            }
            //8 seconds until we should cast this again
            _ChangedNameBackhand_Timer = 8000;
        }

        if (_ChangedNameCurseOfWeakness_Timer.Expired(diff))
        {
            //Cast
            if (rand()%100 < 3) //3% chance to cast
            {
                DoCast(m_creature->GetVictim(),SPELL_CURSEOFWEAKNESS);
            }
            //20 seconds until we should cast this again
            _ChangedNameCurseOfWeakness_Timer = 20000;
        }

        if (_ChangedNameCurseOfTongues_Timer.Expired(diff))
        {
            //Cast
            if (rand()%100 < 3) //3% chance to cast
            {
                DoCast(m_creature->GetVictim(),SPELL_CURSEOFTONGUES);
            }
            //22 seconds until we should cast this again
            _ChangedNameCurseOfTongues_Timer = 22000;
        }

        if (_ChangedNameCallOfTheGrave_Timer.Expired(diff))
        {
            //Cast
            if (rand()%100 < 5) //5% chance to cast
            {
                DoCast(m_creature->GetVictim(),SPELL_CALLOFTHEGRAVE);
            }
            //25 seconds until we should cast this again
            _ChangedNameCallOfTheGrave_Timer = 25000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_postmaster_malown(Creature *_Creature)
{
    return new boss_postmaster_malownAI (_Creature);
}

void AddSC_boss_postmaster_malown()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_postmaster_malown";
    newscript->GetAI = &GetAI_boss_postmaster_malown;
    newscript->RegisterSelf();
}


