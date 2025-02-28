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
SDName: Boss_Vectus
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"

#define SPELL_FIRESHIELD        19626
#define SPELL_BLASTWAVE         13021
#define SPELL_FRENZY            28371

struct boss_vectusAI : public ScriptedAI
{
    boss_vectusAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameFireShield_Timer;
    Timer _ChangedNameBlastWave_Timer;
    Timer _ChangedNameFrenzy_Timer;

    void Reset()
    {
        _ChangedNameFireShield_Timer.Reset(2000);
        _ChangedNameBlastWave_Timer.Reset(14000);
        _ChangedNameFrenzy_Timer.Reset(1);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameFireShield_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_FIRESHIELD);
            _ChangedNameFireShield_Timer = 90000;
        }

        if (_ChangedNameBlastWave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_BLASTWAVE);
            _ChangedNameBlastWave_Timer = 12000;
        }

        //Frenzy_Timer
        if ( me->GetHealthPercent() < 25 )
        {
            if (_ChangedNameFrenzy_Timer.Expired(diff))
            {
                DoCast(m_creature,SPELL_FRENZY);
                DoTextEmote(-1200285,NULL);

                _ChangedNameFrenzy_Timer = 24000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_vectus(Creature *_Creature)
{
    return new boss_vectusAI (_Creature);
}

void AddSC_boss_vectus()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_vectus";
    newscript->GetAI = &GetAI_boss_vectus;
    newscript->RegisterSelf();
}


