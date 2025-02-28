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
SDName: Boss_Ebonroc
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

#define SPELL_SHADOWFLAME           22539
#define SPELL_WINGBUFFET            18500
#define SPELL_SHADOWOFEBONROC       23340
#define SPELL_THRASH                3391

struct boss_ebonrocAI : public ScriptedAI
{
    boss_ebonrocAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    Timer _ChangedNameShadowFlame_Timer;
    Timer _ChangedNameWingBuffet_Timer;
    Timer _ChangedNameShadowOfEbonroc_Timer;
    Timer _ChangedNameThrash_Timer;

    void Reset()
    {
        _ChangedNameShadowFlame_Timer.Reset(15000);                          //These times are probably wrong
        _ChangedNameWingBuffet_Timer.Reset(30000);
        _ChangedNameShadowOfEbonroc_Timer.Reset(45000);
        _ChangedNameThrash_Timer.Reset(25000);

        if (pInstance && pInstance->GetData(DATA_EBONROC_EVENT) != DONE)
            pInstance->SetData(DATA_EBONROC_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();

        if (pInstance)
            pInstance->SetData(DATA_EBONROC_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_EBONROC_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        if (_ChangedNameShadowFlame_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWFLAME);
            _ChangedNameShadowFlame_Timer = 12000 + rand()%3000;
        }

        if (_ChangedNameWingBuffet_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_WINGBUFFET);
            _ChangedNameWingBuffet_Timer = 25000;
        }

        if (_ChangedNameShadowOfEbonroc_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWOFEBONROC);
            _ChangedNameShadowOfEbonroc_Timer = 25000 + rand()%10000;
        }

        if (_ChangedNameThrash_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_THRASH);
            _ChangedNameThrash_Timer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_ebonroc(Creature *_Creature)
{
    return new boss_ebonrocAI (_Creature);
}

void AddSC_boss_ebonroc()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_ebonroc";
    newscript->GetAI = &GetAI_boss_ebonroc;
    newscript->RegisterSelf();
}


