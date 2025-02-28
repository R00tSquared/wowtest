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
SDName: Boss_Hazzarah
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SPELL_MANABURN         26046
#define SPELL_SLEEP            24664

struct boss_hazzarahAI : public ScriptedAI
{
    boss_hazzarahAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    Timer _ChangedNameManaBurn_Timer;
    Timer _ChangedNameSleep_Timer;
    Timer _ChangedNameIllusions_Timer;
    ScriptedInstance * pInstance;

    void Reset()
    {
        _ChangedNameManaBurn_Timer.Reset(4000 + rand()%6000);
        _ChangedNameSleep_Timer.Reset(10000 + rand()%8000);
        _ChangedNameIllusions_Timer.Reset(10000 + rand()%8000);
        pInstance->SetData(DATA_EDGEOFMADNESSEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        pInstance->SetData(DATA_EDGEOFMADNESSEVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        pInstance->SetData(DATA_EDGEOFMADNESSEVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameManaBurn_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MANABURN);
            _ChangedNameManaBurn_Timer = 8000 + rand()%8000;
        }
        

        if (_ChangedNameSleep_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SLEEP);
            _ChangedNameSleep_Timer = 12000 + rand()%8000;
        }
        

        if (_ChangedNameIllusions_Timer.Expired(diff))
        {
            //We will summon 3 illusions that will spawn on a random gamer and attack this gamer
            //We will just use one model for the beginning
            Unit* target = NULL;
            for(int i = 0; i < 3;i++)
            {
                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if(!target)
                    return;

                if(Creature* Illusion = m_creature->SummonCreature(15163, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 30000))
                    ((CreatureAI*)Illusion->AI())->AttackStart(target);
            }

            _ChangedNameIllusions_Timer = 15000 + rand()%10000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_hazzarah(Creature *_Creature)
{
    return new boss_hazzarahAI (_Creature);
}

void AddSC_boss_hazzarah()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_hazzarah";
    newscript->GetAI = &GetAI_boss_hazzarah;
    newscript->RegisterSelf();
}


