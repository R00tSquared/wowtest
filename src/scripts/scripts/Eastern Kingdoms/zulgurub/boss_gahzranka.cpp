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
SDName: Boss_Gahz'ranka
SD%Complete: 85
SDComment: Massive Geyser with knockback not working. Spell buggy.
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SPELL_FROSTBREATH            21099
#define SPELL_MASSIVEGEYSER          22421                  //Not working. Cause its a summon...
#define SPELL_SLAM                   24326

struct boss_gahzrankaAI : public ScriptedAI
{
    boss_gahzrankaAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    Timer _ChangedNameFrostbreath_Timer;
    Timer _ChangedNameMassiveGeyser_Timer;
    Timer _ChangedNameSlam_Timer;
    ScriptedInstance * pInstance;

    void Reset()
    {
        _ChangedNameFrostbreath_Timer.Reset(8000);
        _ChangedNameMassiveGeyser_Timer.Reset(25000);
        _ChangedNameSlam_Timer.Reset(17000);

        if (pInstance->GetData(DATA_GAHZRANKAEVENT) == DONE)
            m_creature->ForcedDespawn();
        else
            pInstance->SetData(DATA_GAHZRANKAEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        pInstance->SetData(DATA_GAHZRANKAEVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        pInstance->SetData(DATA_GAHZRANKAEVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameFrostbreath_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTBREATH);
            _ChangedNameFrostbreath_Timer = 7000 + rand()%4000;
        }
        

        if (_ChangedNameMassiveGeyser_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MASSIVEGEYSER);
            DoResetThreat();

            _ChangedNameMassiveGeyser_Timer = 22000 + rand()%10000;
        }
        

        if (_ChangedNameSlam_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SLAM);
            _ChangedNameSlam_Timer = 12000 + rand()%8000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_gahzranka(Creature *_Creature)
{
    return new boss_gahzrankaAI (_Creature);
}

void AddSC_boss_gahzranka()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gahzranka";
    newscript->GetAI = &GetAI_boss_gahzranka;
    newscript->RegisterSelf();
}


