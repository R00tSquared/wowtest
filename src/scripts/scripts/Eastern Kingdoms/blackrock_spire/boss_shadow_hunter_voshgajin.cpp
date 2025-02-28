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
SDName: Boss_Shadow_Hunter_Voshgajin
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_CURSEOFBLOOD      24673
#define SPELL_HEX               16708
#define SPELL_CLEAVE            20691

struct boss_shadowvoshAI : public ScriptedAI
{
    boss_shadowvoshAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameCurseOfBlood_Timer;
    Timer _ChangedNameHex_Timer;
    Timer _ChangedNameCleave_Timer;

    void Reset()
    {
        _ChangedNameCurseOfBlood_Timer.Reset(2000);
        _ChangedNameHex_Timer.Reset(8000);
        _ChangedNameCleave_Timer.Reset(14000);

        //m_creature->CastSpell(m_creature,SPELL_ICEARMOR,true);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameCurseOfBlood_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CURSEOFBLOOD);
            _ChangedNameCurseOfBlood_Timer = 45000;
        }

        if (_ChangedNameHex_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_HEX);
            _ChangedNameHex_Timer = 15000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 7000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_shadowvosh(Creature *_Creature)
{
    return new boss_shadowvoshAI (_Creature);
}

void AddSC_boss_shadowvosh()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_shadow_hunter_voshgajin";
    newscript->GetAI = &GetAI_boss_shadowvosh;
    newscript->RegisterSelf();
}


