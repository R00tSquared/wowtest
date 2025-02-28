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
SDName: Boss_instructormalicia
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"
#include "def_scholomance.h"

#define SPELL_CALLOFGRAVES         17831
#define SPELL_CORRUPTION           11672
#define SPELL_FLASHHEAL            10917
#define SPELL_RENEW                10929
#define SPELL_HEALINGTOUCH         9889

struct boss_instructormaliciaAI : public ScriptedAI
{
    boss_instructormaliciaAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameCallOfGraves_Timer;
    Timer _ChangedNameCorruption_Timer;
    Timer _ChangedNameFlashHeal_Timer;
    Timer _ChangedNameRenew_Timer;
    Timer _ChangedNameHealingTouch_Timer;
    int32 FlashCounter;
    int32 TouchCounter;

    void Reset()
    {
        _ChangedNameCallOfGraves_Timer.Reset(4000);
        _ChangedNameCorruption_Timer.Reset(8000);
        _ChangedNameFlashHeal_Timer.Reset(38000);
        _ChangedNameRenew_Timer.Reset(32000);
        _ChangedNameHealingTouch_Timer.Reset(45000);
        FlashCounter = 0;
        TouchCounter = 0;
    }

    void JustDied(Unit *killer)
    {
        ScriptedInstance *pInstance = (m_creature->GetInstanceData()) ? (m_creature->GetInstanceData()) : NULL;
        if(pInstance)
        {
            pInstance->SetData(DATA_INSTRUCTORMALICIA_DEATH, 0);

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

        if (_ChangedNameCallOfGraves_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CALLOFGRAVES);
            _ChangedNameCallOfGraves_Timer = 65000;
        }

        if (_ChangedNameCorruption_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_CORRUPTION);

            _ChangedNameCorruption_Timer = 24000;
        }

        if (_ChangedNameRenew_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_RENEW);
            _ChangedNameRenew_Timer = 10000;
        }

        if (_ChangedNameFlashHeal_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_FLASHHEAL);

            //5 Flashheals will be cast
            if (FlashCounter < 2)
            {
                _ChangedNameFlashHeal_Timer = 5000;
                FlashCounter++;
            }
            else
            {
                FlashCounter=0;
                _ChangedNameFlashHeal_Timer = 30000;
            }
        }

        if (_ChangedNameHealingTouch_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_HEALINGTOUCH);

            //3 Healingtouchs will be cast
            if (_ChangedNameHealingTouch_Timer.GetInterval() < 2)
            {
                _ChangedNameHealingTouch_Timer = 5500;
                TouchCounter++;
            }
            else
            {
                TouchCounter=0;
                _ChangedNameHealingTouch_Timer = 30000;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_instructormalicia(Creature *_Creature)
{
    return new boss_instructormaliciaAI (_Creature);
}

void AddSC_boss_instructormalicia()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_instructor_malicia";
    newscript->GetAI = &GetAI_boss_instructormalicia;
    newscript->RegisterSelf();
}


