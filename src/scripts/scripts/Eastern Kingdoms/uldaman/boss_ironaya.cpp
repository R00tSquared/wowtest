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
SDName: Boss_Ironaya
SD%Complete: 100
SDComment:
SDCategory: Uldaman
EndScriptData */

#include "precompiled.h"

#define SPELL_ARCINGSMASH           39144
#define SPELL_KNOCKAWAY             22893
#define SPELL_WSTOMP                16727

#define SAY_AGGRO           -1200303
#define SOUND_AGGRO         5851

struct boss_ironayaAI : public ScriptedAI
{
    boss_ironayaAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameArcing_Timer;
    bool hasCastWstomp;
    bool hasCastKnockaway;

    void Reset()
    {
        _ChangedNameArcing_Timer.Reset(3000);
        hasCastKnockaway = false;
        hasCastWstomp = false;
    }

    void EnterCombat(Unit *who)
    {
        DoYell(-1200303,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(m_creature,SOUND_AGGRO);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //If we are <50% hp do knockaway ONCE
        if (!hasCastKnockaway && m_creature->GetHealth()*2 < m_creature->GetMaxHealth())
        {
            m_creature->CastSpell(m_creature->GetVictim(),SPELL_KNOCKAWAY, true);

            // current aggro target is knocked away pick new target
            Unit* Target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);

            if (!Target || Target == m_creature->GetVictim())
                Target = SelectUnit(SELECT_TARGET_TOPAGGRO, 1);

            if (Target)
                m_creature->TauntApply(Target);

            //Shouldn't cast this agian
            hasCastKnockaway = true;
        }

        if (_ChangedNameArcing_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_ARCINGSMASH);
            _ChangedNameArcing_Timer = 13000;
        }

        if (!hasCastWstomp && m_creature->GetHealth()*4 < m_creature->GetMaxHealth())
        {
            DoCast(m_creature,SPELL_WSTOMP);
            hasCastWstomp = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ironaya(Creature *_Creature)
{
    return new boss_ironayaAI (_Creature);
}

void AddSC_boss_ironaya()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_ironaya";
    newscript->GetAI = &GetAI_boss_ironaya;
    newscript->RegisterSelf();
}


