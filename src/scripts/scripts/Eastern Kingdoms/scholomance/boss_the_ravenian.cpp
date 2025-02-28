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
SDName: Boss_the_ravenian
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"
#include "def_scholomance.h"

#define SPELL_TRAMPLE           15550
#define SPELL_CLEAVE            20691
#define SPELL_SUNDERINCLEAVE    25174
#define SPELL_KNOCKAWAY         10101

#define SAY_AGGRO1              -1200284

struct boss_theravenianAI : public ScriptedAI
{
    boss_theravenianAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameTrample_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameSunderingCleave_Timer;
    Timer _ChangedNameKnockAway_Timer;
    bool HasYelled;

    void Reset()
    {
        _ChangedNameTrample_Timer.Reset(24000);
        _ChangedNameCleave_Timer.Reset(15000);
        _ChangedNameSunderingCleave_Timer.Reset(40000);
        _ChangedNameKnockAway_Timer.Reset(32000);
        HasYelled = false;
    }

    void JustDied(Unit *killer)
    {
        ScriptedInstance *pInstance = (m_creature->GetInstanceData()) ? (m_creature->GetInstanceData()) : NULL;
        if(pInstance)
        {
            pInstance->SetData(DATA_THERAVENIAN_DEATH, 0);

            if(pInstance->GetData(DATA_CANSPAWNGANDLING))
                m_creature->SummonCreature(1853, 180.73, -9.43856, 75.507, 1.61399, TEMPSUMMON_DEAD_DESPAWN, 0);
        }
    }

    void EnterCombat(Unit *who)
    {
        DoYell(-1200284, LANG_UNIVERSAL, NULL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameTrample_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_TRAMPLE);
            _ChangedNameTrample_Timer = 10000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 7000;
        }

        if (_ChangedNameSunderingCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SUNDERINCLEAVE);
            _ChangedNameSunderingCleave_Timer = 20000;
        }

        if (_ChangedNameKnockAway_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_KNOCKAWAY);
            _ChangedNameKnockAway_Timer = 12000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_theravenian(Creature *_Creature)
{
    return new boss_theravenianAI (_Creature);
}

void AddSC_boss_theravenian()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_the_ravenian";
    newscript->GetAI = &GetAI_boss_theravenian;
    newscript->RegisterSelf();
}


