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
SDName: Boss_High_Inquisitor_Faribanks
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"

#define SPELL_CURSEOFBLOOD      8282
#define SPELL_DISPELMAGIC       15090
#define SPELL_FEAR              12096
#define SPELL_HEAL              12039
#define SPELL_POWERWORDSHIELD   11647
#define SPELL_SLEEP             8399


struct boss_high_inquisitor_fairbanksAI : public ScriptedAI
{
    boss_high_inquisitor_fairbanksAI(Creature *c) : ScriptedAI(c) {}

    Timer CurseOfBloodTimer;
    Timer DispelMagicTimer;
    Timer FearTimer;
    Timer HealTimer;
    Timer SleepTimer;

    bool PowerWordShieldCheck;

    void Reset()
    {
        CurseOfBloodTimer.Reset(urand(3000, 5000));
        DispelMagicTimer.Reset(30000);
        FearTimer.Reset(40000);
        HealTimer.Reset(30000);
        SleepTimer.Reset(20000);
        PowerWordShieldCheck = false;

        m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
    }

    void EnterCombat(Unit *who)
    {
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_creature->GetHealthPercent() <= 30 && !PowerWordShieldCheck)
        {
            m_creature->CastSpell(m_creature, SPELL_POWERWORDSHIELD, false);
            PowerWordShieldCheck = true;
        }

        if (HealTimer.Expired(diff))
        {
            if (m_creature->GetHealthPercent() <= 30)
            {
                m_creature->InterruptNonMeleeSpells(true);
                m_creature->CastSpell(m_creature, SPELL_HEAL, false);
                HealTimer = 10000;
            }
        }

        if (FearTimer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_FEAR), true))
            {
                m_creature->CastSpell(target, SPELL_FEAR, false);
                FearTimer = 40000;
            }
        }

        if (SleepTimer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_SLEEP), true))
            {
                m_creature->CastSpell(target, SPELL_SLEEP, false);
                SleepTimer = 40000;
            } 
        }

        if (DispelMagicTimer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_DISPELMAGIC), true))
            {
                m_creature->CastSpell(target, SPELL_DISPELMAGIC, false);
                DispelMagicTimer = 30000;
            } 
        }

        if (CurseOfBloodTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_CURSEOFBLOOD, false);
            CurseOfBloodTimer = 25000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_high_inquisitor_fairbanks(Creature *_Creature)
{
    return new boss_high_inquisitor_fairbanksAI (_Creature);
}

void AddSC_boss_high_inquisitor_fairbanks()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_high_inquisitor_fairbanks";
    newscript->GetAI = &GetAI_boss_high_inquisitor_fairbanks;
    newscript->RegisterSelf();
}


