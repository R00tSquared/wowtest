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
SDName: Boss_Azshir_the_Sleepless
SD%Complete: 80
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"

#define SPELL_CALLOFTHEGRAVE            17831
#define SPELL_TERRIFY                   7399
#define SPELL_SOULSIPHON                7290

struct boss_azshir_the_sleeplessAI : public ScriptedAI
{
    boss_azshir_the_sleeplessAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameSoulSiphon_Timer;
    Timer _ChangedNameCallOftheGrave_Timer;
    Timer _ChangedNameTerrify_Timer;

    void Reset()
    {
        _ChangedNameSoulSiphon_Timer.Reset(1);
        _ChangedNameCallOftheGrave_Timer.Reset(30000);
        _ChangedNameTerrify_Timer.Reset(20000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        //If we are <50% hp cast Soul Siphon rank 1
        if ( me->GetHealthPercent() <= 50 && !m_creature->IsNonMeleeSpellCast(false))
        {
            if (_ChangedNameSoulSiphon_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_SOULSIPHON);
                return;

                _ChangedNameSoulSiphon_Timer = 20000;
            }
        }

        if (_ChangedNameCallOftheGrave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CALLOFTHEGRAVE);
            _ChangedNameCallOftheGrave_Timer = 30000;
        }

        if (_ChangedNameTerrify_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_TERRIFY);
            _ChangedNameTerrify_Timer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_azshir_the_sleepless(Creature *_Creature)
{
    return new boss_azshir_the_sleeplessAI (_Creature);
}

void AddSC_boss_azshir_the_sleepless()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_azshir_the_sleepless";
    newscript->GetAI = &GetAI_boss_azshir_the_sleepless;
    newscript->RegisterSelf();
}


