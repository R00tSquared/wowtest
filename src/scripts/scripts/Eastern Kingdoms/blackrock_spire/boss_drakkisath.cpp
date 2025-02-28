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
SDName: Boss_Drakkisath
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_FIRENOVA                  23462
#define SPELL_CLEAVE                    20691
#define SPELL_CONFLIGURATION            16805
#define SPELL_THUNDERCLAP               15548               //Not sure if right ID. 23931 would be a harder possibility.

struct boss_drakkisathAI : public ScriptedAI
{
    boss_drakkisathAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameFireNova_Timer;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameConfliguration_Timer;
    Timer _ChangedNameThunderclap_Timer;

    void Reset()
    {
        _ChangedNameFireNova_Timer.Reset(6000);
        _ChangedNameCleave_Timer.Reset(8000);
        _ChangedNameConfliguration_Timer.Reset(15000);
        _ChangedNameThunderclap_Timer.Reset(17000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameFireNova_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FIRENOVA);
            _ChangedNameFireNova_Timer = 10000;
        }
        

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 8000;
        }
        

        if (_ChangedNameConfliguration_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CONFLIGURATION);
            _ChangedNameConfliguration_Timer = 18000;
        }
        

        if (_ChangedNameThunderclap_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_THUNDERCLAP);
            _ChangedNameThunderclap_Timer = 20000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_drakkisath(Creature *_Creature)
{
    return new boss_drakkisathAI (_Creature);
}

void AddSC_boss_drakkisath()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_drakkisath";
    newscript->GetAI = &GetAI_boss_drakkisath;
    newscript->RegisterSelf();
}


