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
SDName: Boss_Mother_Smolderweb
SD%Complete: 100
SDComment: Uncertain how often mother's milk is cast
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_CRYSTALIZE                16104
#define SPELL_MOTHERSMILK               16468
#define SPELL_SUMMON_SPIRE_SPIDERLING   16103

struct boss_mothersmolderwebAI : public ScriptedAI
{
    boss_mothersmolderwebAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameCrystalize_Timer;
    Timer _ChangedNameMothersMilk_Timer;

    void Reset()
    {
        _ChangedNameCrystalize_Timer.Reset(20000);
        _ChangedNameMothersMilk_Timer.Reset(10000);
    }

    void EnterCombat(Unit *who) { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if( m_creature->GetHealth() <= damage )
            m_creature->CastSpell(m_creature,SPELL_SUMMON_SPIRE_SPIDERLING,true);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if( _ChangedNameCrystalize_Timer.Expired(diff) )
        {
            DoCast(m_creature,SPELL_CRYSTALIZE);
            _ChangedNameCrystalize_Timer = 15000;
        }

        if( _ChangedNameMothersMilk_Timer.Expired(diff) )
        {
            DoCast(m_creature,SPELL_MOTHERSMILK);
            _ChangedNameMothersMilk_Timer = 5000+rand()%7500;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_mothersmolderweb(Creature *_Creature)
{
    return new boss_mothersmolderwebAI (_Creature);
}

void AddSC_boss_mothersmolderweb()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_mother_smolderweb";
    newscript->GetAI = &GetAI_boss_mothersmolderweb;
    newscript->RegisterSelf();
}


