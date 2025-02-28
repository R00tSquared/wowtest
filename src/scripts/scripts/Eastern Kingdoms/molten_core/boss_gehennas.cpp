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
SDName: Boss_Gehennas
SD%Complete: 90
SDComment: Adds MC NYI
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define SPELL_SHADOWBOLT            19728
#define SPELL_RAINOFFIRE            19717
#define SPELL_GEHENNASCURSE         19716

struct boss_gehennasAI : public ScriptedAI
{
    boss_gehennasAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    Timer _ChangedNameShadowBolt_Timer;
    Timer _ChangedNameRainOfFire_Timer;
    Timer _ChangedNameGehennasCurse_Timer;

    void Reset()
    {
        _ChangedNameShadowBolt_Timer.Reset(6000);
        _ChangedNameRainOfFire_Timer.Reset(10000);
        _ChangedNameGehennasCurse_Timer.Reset(12000);

        if (pInstance && pInstance->GetData(DATA_GEHENNAS_EVENT) != DONE)
            pInstance->SetData(DATA_GEHENNAS_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_GEHENNAS_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_GEHENNAS_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameShadowBolt_Timer.Expired(diff))
        {
            if( Unit* bTarget = SelectUnit(SELECT_TARGET_RANDOM,1) )
                DoCast(bTarget,SPELL_SHADOWBOLT);
            _ChangedNameShadowBolt_Timer = 7000;
        }

        if (_ChangedNameRainOfFire_Timer.Expired(diff))
        {
            if( Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_RAINOFFIRE);

            _ChangedNameRainOfFire_Timer = 4000 + rand()%8000;
        }

        if (_ChangedNameGehennasCurse_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_GEHENNASCURSE);
            _ChangedNameGehennasCurse_Timer = 22000 + rand()%8000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_gehennas(Creature *_Creature)
{
    return new boss_gehennasAI (_Creature);
}

void AddSC_boss_gehennas()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gehennas";
    newscript->GetAI = &GetAI_boss_gehennas;
    newscript->RegisterSelf();
}


