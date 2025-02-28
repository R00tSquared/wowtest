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
SDName: Boss_Lucifron
SD%Complete: 100
SDComment:
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define SPELL_IMPENDINGDOOM 19702
#define SPELL_LUCIFRONCURSE 19703
#define SPELL_SHADOWSHOCK   20603

struct boss_lucifronAI : public ScriptedAI
{
    boss_lucifronAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    Timer _ChangedNameImpendingDoom_Timer;
    Timer _ChangedNameLucifronCurse_Timer;
    Timer _ChangedNameShadowShock_Timer;

    void Reset()
    {
        _ChangedNameImpendingDoom_Timer.Reset(10000);                        //Initial cast after 10 seconds so the debuffs alternate
        _ChangedNameLucifronCurse_Timer.Reset(20000);                        //Initial cast after 20 seconds
        _ChangedNameShadowShock_Timer.Reset(6000);                           //6 seconds

        if (pInstance)
            pInstance->SetData(DATA_LUCIFRON_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_LUCIFRON_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_LUCIFRON_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameImpendingDoom_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_IMPENDINGDOOM);
            _ChangedNameImpendingDoom_Timer = 20000;
        }

        if (_ChangedNameLucifronCurse_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_LUCIFRONCURSE);
            _ChangedNameLucifronCurse_Timer = 15000;
        }

        if (_ChangedNameShadowShock_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWSHOCK);
            _ChangedNameShadowShock_Timer = 6000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_lucifron(Creature *_Creature)
{
    return new boss_lucifronAI (_Creature);
}

void AddSC_boss_lucifron()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_lucifron";
    newscript->GetAI = &GetAI_boss_lucifron;
    newscript->RegisterSelf();
}


