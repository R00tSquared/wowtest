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
SDName: Boss_Magistrate_Barthilas
SD%Complete: 70
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "def_stratholme.h"

#define SPELL_DRAININGBLOW    16793
#define SPELL_CROWDPUMMEL    10887
#define SPELL_MIGHTYBLOW    14099
#define SPELL_FURIOUS_ANGER     16791

#define MODEL_NORMAL            10433
#define MODEL_HUMAN             3637

struct boss_magistrate_barthilasAI : public ScriptedAI
{
    boss_magistrate_barthilasAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)m_creature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameDrainingBlow_Timer;
    Timer _ChangedNameCrowdPummel_Timer;
    Timer _ChangedNameMightyBlow_Timer;
    Timer _ChangedNameFuriousAnger_Timer;
    uint32 AngerCount;

    void Reset()
    {
        _ChangedNameDrainingBlow_Timer.Reset(20000);
        _ChangedNameCrowdPummel_Timer.Reset(15000);
        _ChangedNameMightyBlow_Timer.Reset(10000);
        _ChangedNameFuriousAnger_Timer.Reset(5000);
        AngerCount = 0;

        if (m_creature->isAlive())
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_NORMAL);
        else
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_HUMAN);
    }

    void MoveInLineOfSight(Unit *who)
    {
        //nothing to see here yet

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* Killer)
    {
        m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_HUMAN);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameFuriousAnger_Timer.Expired(diff))
        {
            _ChangedNameFuriousAnger_Timer = 4000;
            if (AngerCount > 25)
                return;

            ++AngerCount;
            m_creature->CastSpell(m_creature,SPELL_FURIOUS_ANGER,false);
        }

        if (_ChangedNameDrainingBlow_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_DRAININGBLOW);
            _ChangedNameDrainingBlow_Timer = 15000;
        }

        if (_ChangedNameCrowdPummel_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CROWDPUMMEL);
            _ChangedNameCrowdPummel_Timer = 15000;
        }

        if (_ChangedNameMightyBlow_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MIGHTYBLOW);
            _ChangedNameMightyBlow_Timer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_magistrate_barthilas(Creature *_Creature)
{
    return new boss_magistrate_barthilasAI (_Creature);
}

void AddSC_boss_magistrate_barthilas()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_magistrate_barthilas";
    newscript->GetAI = &GetAI_boss_magistrate_barthilas;
    newscript->RegisterSelf();
}


