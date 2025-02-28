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
SDName: Boss_Warmaster_Voone
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_SNAPKICK          15618
#define SPELL_CLEAVE            15579
#define SPELL_UPPERCUT          10966
#define SPELL_MORTALSTRIKE      16856
#define SPELL_PUMMEL            15615
#define SPELL_THROWAXE          16075

struct boss_warmastervooneAI : public ScriptedAI
{
    boss_warmastervooneAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameSnapkick_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameUppercut_Timer;
    Timer _ChangedNameMortalStrike_Timer;
    Timer _ChangedNamePummel_Timer;
    Timer _ChangedNameThrowAxe_Timer;

    void Reset()
    {
        _ChangedNameSnapkick_Timer.Reset(8000);
        _ChangedNameCleave_Timer.Reset(14000);
        _ChangedNameUppercut_Timer.Reset(20000);
        _ChangedNameMortalStrike_Timer.Reset(12000);
        _ChangedNamePummel_Timer.Reset(32000);
        _ChangedNameThrowAxe_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameSnapkick_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SNAPKICK);
            _ChangedNameSnapkick_Timer = 6000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 12000;
        }

        if (_ChangedNameUppercut_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_UPPERCUT);
            _ChangedNameUppercut_Timer = 14000;
        }else 

        if (_ChangedNameMortalStrike_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MORTALSTRIKE);
            _ChangedNameMortalStrike_Timer = 10000;
        }

        if (_ChangedNamePummel_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_PUMMEL);
            _ChangedNamePummel_Timer = 16000;
        }

        if (_ChangedNameThrowAxe_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_THROWAXE);
            _ChangedNameThrowAxe_Timer = 8000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_warmastervoone(Creature *_Creature)
{
    return new boss_warmastervooneAI (_Creature);
}

void AddSC_boss_warmastervoone()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_warmaster_voone";
    newscript->GetAI = &GetAI_boss_warmastervoone;
    newscript->RegisterSelf();
}


