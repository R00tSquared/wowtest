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
SDName: Boss_The_Best
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"
#include "def_blackrock_spire.h"

#define SPELL_FLAMEBREAK            16785
#define SPELL_IMMOLATE              20294
#define SPELL_TERRIFYINGROAR        14100

struct boss_thebeastAI : public ScriptedAI
{
    boss_thebeastAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (instance_blackrock_spire*)c->GetInstanceData();
    }

    instance_blackrock_spire* pInstance;

    Timer _ChangedNameFlamebreak_Timer;
    Timer _ChangedNameImmolate_Timer;
    Timer _ChangedNameTerrifyingRoar_Timer;
    Timer _ChangedNamecheckTimer;

    void Reset()
    {
        _ChangedNameFlamebreak_Timer.Reset(12000);
        _ChangedNameImmolate_Timer.Reset(3000);
        _ChangedNameTerrifyingRoar_Timer.Reset(23000);
        _ChangedNamecheckTimer.Reset(1);
        pInstance->SetData(DATA_THE_BEAST, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat(80.0f);
        pInstance->SetData(DATA_THE_BEAST, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_THE_BEAST, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNamecheckTimer.Expired(diff))
        {
            DoZoneInCombat();
            _ChangedNamecheckTimer = 3000;
        }
        

        if (_ChangedNameFlamebreak_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FLAMEBREAK);
            _ChangedNameFlamebreak_Timer = 10000;
        }
        

        if (_ChangedNameImmolate_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target)
                AddSpellToCast(target, SPELL_IMMOLATE);
            _ChangedNameImmolate_Timer = 8000;
        }
        

        if (_ChangedNameTerrifyingRoar_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_TERRIFYINGROAR);
            _ChangedNameTerrifyingRoar_Timer = 20000;
        }
        

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_thebeast(Creature *_Creature)
{
    return new boss_thebeastAI (_Creature);
}

void AddSC_boss_thebeast()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_the_beast";
    newscript->GetAI = &GetAI_boss_thebeast;
    newscript->RegisterSelf();
}


