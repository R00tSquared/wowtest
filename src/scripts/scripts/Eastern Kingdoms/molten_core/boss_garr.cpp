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
SDName: Boss_Garr
SD%Complete: 50
SDComment: Adds NYI
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

// Garr spells
#define SPELL_ANTIMAGICPULSE        19492
#define SPELL_MAGMASHACKLES         19496
#define SPELL_ENRAGE                19516                   //Stacking enrage (stacks to 10 times)

//Add spells
#define SPELL_ERUPTION              19497
#define SPELL_IMMOLATE              20294

struct boss_garrAI : public ScriptedAI
{
    boss_garrAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    Timer _ChangedNameAntiMagicPulse_Timer;
    Timer _ChangedNameMagmaShackles_Timer;
    uint64 Add[8];
    bool Enraged[8];

    void Reset()
    {
        _ChangedNameAntiMagicPulse_Timer.Reset(25000);                       //These times are probably wrong
        _ChangedNameMagmaShackles_Timer.Reset(15000);

        if (pInstance && pInstance->GetData(DATA_GARR_EVENT) != DONE)
            pInstance->SetData(DATA_GARR_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_GARR_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_GARR_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameAntiMagicPulse_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_ANTIMAGICPULSE);
            _ChangedNameAntiMagicPulse_Timer = 10000 + rand()%5000;
        }

        if (_ChangedNameMagmaShackles_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_MAGMASHACKLES);
            _ChangedNameMagmaShackles_Timer = 8000 + rand()%4000;
        }

        DoMeleeAttackIfReady();
    }
};

struct mob_fireswornAI : public ScriptedAI
{
    mob_fireswornAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameImmolate_Timer;

    void Reset()
    {
        _ChangedNameImmolate_Timer.Reset(4000);                              //These times are probably wrong
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameImmolate_Timer.Expired(diff))
        {
             if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                DoCast(target,SPELL_IMMOLATE);

            _ChangedNameImmolate_Timer = 5000 + rand()%5000;
        }

        //Cast Erruption and let them die
        if (m_creature->GetHealth() <= m_creature->GetMaxHealth() * 0.10)
        {
            DoCast(m_creature->GetVictim(),SPELL_ERUPTION);
            m_creature->setDeathState(JUST_DIED);
            m_creature->RemoveCorpse();
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_garr(Creature *_Creature)
{
    return new boss_garrAI (_Creature);
}

CreatureAI* GetAI_mob_firesworn(Creature *_Creature)
{
    return new mob_fireswornAI (_Creature);
}

void AddSC_boss_garr()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_garr";
    newscript->GetAI = &GetAI_boss_garr;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_firesworn";
    newscript->GetAI = &GetAI_mob_firesworn;
    newscript->RegisterSelf();
}


