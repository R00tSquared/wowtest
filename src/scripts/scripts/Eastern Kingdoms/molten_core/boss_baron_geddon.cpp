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
SDName: Boss_Baron_Geddon
SD%Complete: 100
SDComment: DMG in DB is propably wrong (30 on plate tank?)
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define EMOTE_SERVICE               -1409000

#define SPELL_INFERNO               19695
#define SPELL_IGNITEMANA            19659
#define SPELL_LIVINGBOMB            20475
#define SPELL_ARMAGEDDOM            20478 //20479 triggered
#define SPELL_SUMMONPLAYER          20477 //not implemented

struct boss_baron_geddonAI : public ScriptedAI
{
    boss_baron_geddonAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer _ChangedNameInferno_Timer;
    Timer _ChangedNameIgniteMana_Timer;
    Timer _ChangedNameLivingBomb_Timer;
    Timer Armageddon_Timer;

    void Reset()
    {
        _ChangedNameInferno_Timer.Reset(10000);
        _ChangedNameIgniteMana_Timer.Reset(30000);
        _ChangedNameLivingBomb_Timer.Reset(35000);
        Armageddon_Timer.Reset(1);

        if (pInstance && pInstance->GetData(DATA_BARON_GEDDON_EVENT) != DONE)
            pInstance->SetData(DATA_BARON_GEDDON_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_BARON_GEDDON_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_BARON_GEDDON_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_creature->HasAura(19695, 1))
            return;

        //If we are <2% hp cast Armageddom
        if (me->GetHealthPercent() <= 2 && Armageddon_Timer.Expired(diff))
        {
            Armageddon_Timer = 9000;    //We don't want him to cast while being under Armageddon effect
            _ChangedNameInferno_Timer.Reset(9000);
            _ChangedNameIgniteMana_Timer.Reset(9000);
            _ChangedNameLivingBomb_Timer.Reset(9000);
            m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature,SPELL_ARMAGEDDOM);
            DoScriptText(EMOTE_SERVICE, m_creature);
            return;
        }

        if (_ChangedNameInferno_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_INFERNO, false);
            _ChangedNameInferno_Timer = 22000;
        }

        if (_ChangedNameIgniteMana_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                DoCast(target,SPELL_IGNITEMANA);

            _ChangedNameIgniteMana_Timer = 30000;
        }

        if (_ChangedNameLivingBomb_Timer.Expired(diff))
        {
           if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
               DoCast(target,SPELL_LIVINGBOMB);

            _ChangedNameLivingBomb_Timer = 35000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_baron_geddon(Creature *_Creature)
{
    return new boss_baron_geddonAI (_Creature);
}

void AddSC_boss_baron_geddon()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_baron_geddon";
    newscript->GetAI = &GetAI_boss_baron_geddon;
    newscript->RegisterSelf();
}


