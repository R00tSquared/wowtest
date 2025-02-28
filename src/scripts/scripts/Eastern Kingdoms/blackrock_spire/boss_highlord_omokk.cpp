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
SDName: Boss_Highlord_Omokk
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_WARSTOMP          24375
#define SPELL_CLEAVE            15579
#define SPELL_STRIKE            18368
#define SPELL_REND              18106
#define SPELL_SUNDERARMOR       24317
#define SPELL_KNOCKAWAY         20686
#define SPELL_SLOW              22356

struct boss_highlordomokkAI : public ScriptedAI
{
    boss_highlordomokkAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameWarStomp_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameStrike_Timer;
    Timer _ChangedNameRend_Timer;
    Timer _ChangedNameSunderArmor_Timer;
    Timer _ChangedNameKnockAway_Timer;
    Timer _ChangedNameSlow_Timer;

    void Reset()
    {
        _ChangedNameWarStomp_Timer.Reset(15000);
        _ChangedNameCleave_Timer.Reset(6000);
        _ChangedNameStrike_Timer.Reset(10000);
        _ChangedNameRend_Timer.Reset(14000);
        _ChangedNameSunderArmor_Timer.Reset(2000);
        _ChangedNameKnockAway_Timer.Reset(18000);
        _ChangedNameSlow_Timer.Reset(24000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameWarStomp_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_WARSTOMP);
            _ChangedNameWarStomp_Timer = 14000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 8000;
        }

        if (_ChangedNameStrike_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_STRIKE);
            _ChangedNameStrike_Timer = 10000;
        }

        if (_ChangedNameRend_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_REND);
            _ChangedNameRend_Timer = 18000;
        }

        if (_ChangedNameSunderArmor_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SUNDERARMOR);
            _ChangedNameSunderArmor_Timer = 25000;
        }

        if (_ChangedNameKnockAway_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_KNOCKAWAY);
            _ChangedNameKnockAway_Timer = 12000;
        }

        if (_ChangedNameSlow_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SLOW);
            _ChangedNameSlow_Timer = 18000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_highlordomokk(Creature *_Creature)
{
    return new boss_highlordomokkAI (_Creature);
}

void AddSC_boss_highlordomokk()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_highlord_omokk";
    newscript->GetAI = &GetAI_boss_highlordomokk;
    newscript->RegisterSelf();
}


