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
SDName: Boss_Doctor_Theolen_Krastinov
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"
#include "def_scholomance.h"

#define SPELL_REND              18106
#define SPELL_BLACKHAND         18103
#define SPELL_FRENZY            28371

struct boss_theolenkrastinovAI : public ScriptedAI
{
    boss_theolenkrastinovAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameRend_Timer;
    Timer _ChangedNameBlackhand_Timer;
    Timer _ChangedNameFrenzy_Timer;

    void Reset()
    {
        _ChangedNameRend_Timer.Reset(8000);
        _ChangedNameBlackhand_Timer.Reset(9000);
        _ChangedNameFrenzy_Timer.Reset(1);
    }

    void JustDied(Unit *killer)
    {
        ScriptedInstance *pInstance = (m_creature->GetInstanceData()) ? (m_creature->GetInstanceData()) : NULL;
        if(pInstance)
        {
            pInstance->SetData(DATA_DOCTORTHEOLENKRASTINOV_DEATH, 0);

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

        if (_ChangedNameRend_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_REND);
            _ChangedNameRend_Timer = 10000;
        }

        if (_ChangedNameBlackhand_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_BLACKHAND);
            _ChangedNameBlackhand_Timer = 10000;
        }

        //Frenzy_Timer
        if ( me->GetHealthPercent() < 26 )
        {
            if (_ChangedNameFrenzy_Timer.Expired(diff))
            {
                DoCast(m_creature,SPELL_FRENZY);
                DoTextEmote("goes into a killing frenzy!",NULL);

                _ChangedNameFrenzy_Timer = 8000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_theolenkrastinov(Creature *_Creature)
{
    return new boss_theolenkrastinovAI (_Creature);
}

void AddSC_boss_theolenkrastinov()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_doctor_theolen_krastinov";
    newscript->GetAI = &GetAI_boss_theolenkrastinov;
    newscript->RegisterSelf();
}


