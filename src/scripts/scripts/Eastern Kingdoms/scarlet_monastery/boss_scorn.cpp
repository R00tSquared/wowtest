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
SDName: Boss_Scorn
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"

#define SPELL_LICHSLAP                  28873
#define SPELL_FROSTBOLTVOLLEY           8398
#define SPELL_MINDFLAY                  17313
#define SPELL_FROSTNOVA                 15531

struct boss_scornAI : public ScriptedAI
{
    boss_scornAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameLichSlap_Timer;
    Timer _ChangedNameFrostboltVolley_Timer;
    Timer _ChangedNameMindFlay_Timer;
    Timer _ChangedNameFrostNova_Timer;

    void Reset()
    {
        _ChangedNameLichSlap_Timer.Reset(45000);
        _ChangedNameFrostboltVolley_Timer.Reset(30000);
        _ChangedNameMindFlay_Timer.Reset(30000);
        _ChangedNameFrostNova_Timer.Reset(30000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameLichSlap_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_LICHSLAP);
            _ChangedNameLichSlap_Timer = 45000;
        }

        if (_ChangedNameFrostboltVolley_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTBOLTVOLLEY);
            _ChangedNameFrostboltVolley_Timer = 20000;
        }

        if (_ChangedNameMindFlay_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MINDFLAY);
            _ChangedNameMindFlay_Timer = 20000;
        }

        if (_ChangedNameFrostNova_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTNOVA);
            _ChangedNameFrostNova_Timer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_scorn(Creature *_Creature)
{
    return new boss_scornAI (_Creature);
}

void AddSC_boss_scorn()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_scorn";
    newscript->GetAI = &GetAI_boss_scorn;
    newscript->RegisterSelf();
}


