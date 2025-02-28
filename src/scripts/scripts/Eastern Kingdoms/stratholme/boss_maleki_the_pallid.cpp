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
SDName: boss_maleki_the_pallid
SD%Complete: 100
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "def_stratholme.h"

#define SPELL_FROSTBOLT    17503
#define SPELL_DRAINLIFE    20743
#define SPELL_DRAIN_MANA    17243
#define SPELL_ICETOMB    16869

struct boss_maleki_the_pallidAI : public ScriptedAI
{
    boss_maleki_the_pallidAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)m_creature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameFrostbolt_Timer;
    Timer _ChangedNameIceTomb_Timer;
    Timer _ChangedNameDrainLife_Timer;

    void Reset()
    {
        _ChangedNameFrostbolt_Timer.Reset(1000);
        _ChangedNameIceTomb_Timer.Reset(16000);
        _ChangedNameDrainLife_Timer.Reset(31000);
    }

    void EnterCombat(Unit *who)
    {
         if (pInstance)
             pInstance->SetData(TYPE_PALLID,IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_PALLID,SPECIAL);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameFrostbolt_Timer.Expired(diff))
        {
             if (rand()%100 < 90)
                DoCast(m_creature->GetVictim(),SPELL_FROSTBOLT);
            _ChangedNameFrostbolt_Timer = 3500;
        }

        if (_ChangedNameIceTomb_Timer.Expired(diff))
        {
            if (rand()%100 < 65)
                DoCast(m_creature->GetVictim(),SPELL_ICETOMB);
            _ChangedNameIceTomb_Timer = 28000;
        }

        if (_ChangedNameDrainLife_Timer.Expired(diff))
        {
              if (rand()%100 < 55)
                DoCast(m_creature->GetVictim(),SPELL_DRAINLIFE);
            _ChangedNameDrainLife_Timer = 31000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_maleki_the_pallid(Creature *_Creature)
{
    return new boss_maleki_the_pallidAI (_Creature);
}

void AddSC_boss_maleki_the_pallid()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_maleki_the_pallid";
    newscript->GetAI = &GetAI_boss_maleki_the_pallid;
    newscript->RegisterSelf();
}


