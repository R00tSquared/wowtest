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
SDName: Boss_Grilek
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SPELL_AVARTAR                24646                  //The Enrage Spell
#define SPELL_GROUNDTREMOR            6524

struct boss_grilekAI : public ScriptedAI
{
    boss_grilekAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    Timer _ChangedNameAvartar_Timer;
    Timer _ChangedNameGroundTremor_Timer;
    ScriptedInstance * pInstance;

    void Reset()
    {
        _ChangedNameAvartar_Timer.Reset(15000 + rand()%10000);
        _ChangedNameGroundTremor_Timer.Reset(8000 + rand()%8000);
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
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameAvartar_Timer.Expired(diff))
        {

            DoCast(m_creature, SPELL_AVARTAR);
            Unit* target = NULL;

            target = SelectUnit(SELECT_TARGET_RANDOM,1, 200, true, m_creature->getVictimGUID());

            if(DoGetThreat(m_creature->GetVictim()))
                DoModifyThreatPercent(m_creature->GetVictim(),-50);
            if (target)
                AttackStart(target);

            _ChangedNameAvartar_Timer = 25000 + rand()%10000;
        }
        

        if (_ChangedNameGroundTremor_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_GROUNDTREMOR);
            _ChangedNameGroundTremor_Timer = 12000 + rand()%4000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_grilek(Creature *_Creature)
{
    return new boss_grilekAI (_Creature);
}

void AddSC_boss_grilek()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_grilek";
    newscript->GetAI = &GetAI_boss_grilek;
    newscript->RegisterSelf();
}


