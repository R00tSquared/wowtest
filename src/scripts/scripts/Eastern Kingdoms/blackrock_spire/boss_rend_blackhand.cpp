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
SDName: Boss_Rend_Blackhand
SD%Complete: 100
SDComment: Intro event NYI
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_WHIRLWIND                 26038
#define SPELL_CLEAVE                    20691
#define SPELL_THUNDERCLAP               23931               //Not sure if he cast this spell

struct boss_rend_blackhandAI : public ScriptedAI
{
    boss_rend_blackhandAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameWhirlWind_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameThunderclap_Timer;

    void Reset()
    {
        _ChangedNameWhirlWind_Timer.Reset(20000);
        _ChangedNameCleave_Timer.Reset(5000);
        _ChangedNameThunderclap_Timer.Reset(9000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameWhirlWind_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_WHIRLWIND);
            _ChangedNameWhirlWind_Timer = 18000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 10000;
        }

        if (_ChangedNameThunderclap_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_THUNDERCLAP);
            _ChangedNameThunderclap_Timer = 16000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_rend_blackhand(Creature *_Creature)
{
    return new boss_rend_blackhandAI (_Creature);
}

void AddSC_boss_rend_blackhand()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_rend_blackhand";
    newscript->GetAI = &GetAI_boss_rend_blackhand;
    newscript->RegisterSelf();
}


