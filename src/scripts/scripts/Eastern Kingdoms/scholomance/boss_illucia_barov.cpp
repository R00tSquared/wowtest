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
SDName: Boss_Illucia_Barov
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"
#include "def_scholomance.h"

#define SPELL_CURSEOFAGONY      18671
#define SPELL_SHADOWSHOCK       20603
#define SPELL_SILENCE           15487
#define SPELL_FEAR              6215

struct boss_illuciabarovAI : public ScriptedAI
{
    boss_illuciabarovAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameCurseOfAgony_Timer;
    Timer _ChangedNameShadowShock_Timer;
    Timer _ChangedNameSilence_Timer;
    Timer _ChangedNameFear_Timer;

    void Reset()
    {
        _ChangedNameCurseOfAgony_Timer.Reset(18000);
        _ChangedNameShadowShock_Timer.Reset(9000);
        _ChangedNameSilence_Timer.Reset(5000);
        _ChangedNameFear_Timer.Reset(30000);
    }

    void JustDied(Unit *killer)
    {
        ScriptedInstance *pInstance = (m_creature->GetInstanceData()) ? (m_creature->GetInstanceData()) : NULL;
        if(pInstance)
        {
            pInstance->SetData(DATA_LADYILLUCIABAROV_DEATH, 0);

            if(pInstance->GetData(DATA_CANSPAWNGANDLING))
                m_creature->SummonCreature(1853, 180.73, -9.43856, 75.507, 1.61399, TEMPSUMMON_DEAD_DESPAWN, 0);
        }
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameCurseOfAgony_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CURSEOFAGONY);
            _ChangedNameCurseOfAgony_Timer = 30000;
        }

        if (_ChangedNameShadowShock_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_SHADOWSHOCK);

            _ChangedNameShadowShock_Timer = 12000;
        }

        if (_ChangedNameSilence_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SILENCE);
            _ChangedNameSilence_Timer = 14000;
        }

        if (_ChangedNameFear_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FEAR);
            _ChangedNameFear_Timer = 30000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_illuciabarov(Creature *_Creature)
{
    return new boss_illuciabarovAI (_Creature);
}

void AddSC_boss_illuciabarov()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_illucia_barov";
    newscript->GetAI = &GetAI_boss_illuciabarov;
    newscript->RegisterSelf();
}


