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
SDName: Boss_Ras_Frostwhisper
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"

#define SPELL_FROSTBOLT         21369
#define SPELL_ICEARMOR          18100                       //This is actually a buff he gives himself
#define SPELL_FREEZE            18763
#define SPELL_FEAR              26070
#define SPELL_CHILLNOVA         18099
#define SPELL_FROSTVOLLEY       8398

struct boss_rasfrostAI : public ScriptedAI
{
    boss_rasfrostAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameIceArmor_Timer;
    Timer _ChangedNameFrostbolt_Timer;
    Timer _ChangedNameFreeze_Timer;
    Timer _ChangedNameFear_Timer;
    Timer _ChangedNameChillNova_Timer;
    Timer _ChangedNameFrostVolley_Timer;

    void Reset()
    {
        _ChangedNameIceArmor_Timer.Reset(2000);
        _ChangedNameFrostbolt_Timer.Reset(8000);
        _ChangedNameChillNova_Timer.Reset(12000);
        _ChangedNameFreeze_Timer.Reset(18000);
        _ChangedNameFrostVolley_Timer.Reset(24000);
        _ChangedNameFear_Timer.Reset(45000);

        m_creature->CastSpell(m_creature,SPELL_ICEARMOR,true);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameIceArmor_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_ICEARMOR);
            _ChangedNameIceArmor_Timer = 180000;
        }

        if (_ChangedNameFrostbolt_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0, GetSpellMaxRange(SPELL_FROSTBOLT), true);
            if (target)
                DoCast(target,SPELL_FROSTBOLT);

            _ChangedNameFrostbolt_Timer = 8000;
        }

        if (_ChangedNameFreeze_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FREEZE);
            _ChangedNameFreeze_Timer = 24000;
        }

        if (_ChangedNameFear_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FEAR);
            _ChangedNameFear_Timer = 30000;
        }

        if (_ChangedNameChillNova_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CHILLNOVA);
            _ChangedNameChillNova_Timer = 14000;
        }

        if (_ChangedNameFrostVolley_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTVOLLEY);
            _ChangedNameFrostVolley_Timer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_rasfrost(Creature *_Creature)
{
    return new boss_rasfrostAI (_Creature);
}

void AddSC_boss_rasfrost()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_boss_ras_frostwhisper";
    newscript->GetAI = &GetAI_boss_rasfrost;
    newscript->RegisterSelf();
}


