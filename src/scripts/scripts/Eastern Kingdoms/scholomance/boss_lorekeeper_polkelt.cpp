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
SDName: Boss_Lorekeeper_Polkelt
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"
#include "def_scholomance.h"

#define SPELL_VOLATILEINFECTION      24928
#define SPELL_DARKPLAGUE             18270
#define SPELL_CORROSIVEACID          23313
#define SPELL_NOXIOUSCATALYST        18151

struct boss_lorekeeperpolkeltAI : public ScriptedAI
{
    boss_lorekeeperpolkeltAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameVolatileInfection_Timer;
    Timer _ChangedNameDarkplague_Timer;
    Timer _ChangedNameCorrosiveAcid_Timer;
    Timer _ChangedNameNoxiousCatalyst_Timer;

    void Reset()
    {
        _ChangedNameVolatileInfection_Timer.Reset(38000);
        _ChangedNameDarkplague_Timer.Reset(8000);
        _ChangedNameCorrosiveAcid_Timer.Reset(45000);
        _ChangedNameNoxiousCatalyst_Timer.Reset(35000);
    }

    void JustDied(Unit *killer)
    {
        ScriptedInstance *pInstance = (m_creature->GetInstanceData()) ? (m_creature->GetInstanceData()) : NULL;
        if(pInstance)
        {
            pInstance->SetData(DATA_LOREKEEPERPOLKELT_DEATH, 0);

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

        if (_ChangedNameVolatileInfection_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_VOLATILEINFECTION);
            _ChangedNameVolatileInfection_Timer = 32000;
        }

        if (_ChangedNameDarkplague_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_DARKPLAGUE);
            _ChangedNameDarkplague_Timer = 8000;
        }

        if (_ChangedNameCorrosiveAcid_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CORROSIVEACID);
            _ChangedNameCorrosiveAcid_Timer = 25000;
        }

        if (_ChangedNameNoxiousCatalyst_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_NOXIOUSCATALYST);
            _ChangedNameNoxiousCatalyst_Timer = 38000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_lorekeeperpolkelt(Creature *_Creature)
{
    return new boss_lorekeeperpolkeltAI (_Creature);
}

void AddSC_boss_lorekeeperpolkelt()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_lorekeeper_polkelt";
    newscript->GetAI = &GetAI_boss_lorekeeperpolkelt;
    newscript->RegisterSelf();
}


