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
SDName: Boss_Arcanist_Doan
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"

#define SPELL_POLYMORPH                 12826
#define SPELL_AOESILENCE                8988
#define SPELL_ARCANEEXPLOSION3          8438
#define SPELL_ARCANEEXPLOSION4          8439
#define SPELL_FIREAOE                   9435
#define SPELL_BLINK                     1953
#define SPELL_FIREBALL                  21162
#define SPELL_MANASHIELD4               10191
#define SPELL_ARCANEBUBBLE              9438

#define SAY_AGGRO                       -1200271
#define SAY_SPECIALAE                   -1200272

#define SOUND_AGGRO                     5842
#define SOUND_SPECIALAE                 5843

struct boss_arcanist_doanAI : public ScriptedAI
{
    boss_arcanist_doanAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameFullAOE_Timer;
    Timer _ChangedNamePolymorph_Timer;
    Timer _ChangedNameYell_Timer;
    Timer _ChangedNameArcaneBubble_Timer;
    Timer _ChangedNameAoESilence_Timer;
    Timer _ChangedNameArcaneExplosion3_Timer;
    Timer _ChangedNameArcaneExplosion4_Timer;
    Timer _ChangedNameBlink_Timer;
    Timer _ChangedNameFireball_Timer;
    Timer _ChangedNameManaShield4_Timer;

    void Reset()
    {
        _ChangedNameFullAOE_Timer.Reset(5000);
        _ChangedNamePolymorph_Timer.Reset(1);
        _ChangedNameYell_Timer.Reset(2000);
        _ChangedNameArcaneBubble_Timer.Reset(3000);
        _ChangedNameAoESilence_Timer.Reset(20000);
        _ChangedNameArcaneExplosion3_Timer.Reset(10000);
        _ChangedNameArcaneExplosion4_Timer.Reset(10000);
        _ChangedNameBlink_Timer.Reset(40000);
        _ChangedNameFireball_Timer.Reset(6000);
        _ChangedNameManaShield4_Timer.Reset(70000);
    }

    void EnterCombat(Unit *who)
    {
        DoYell(-1200271,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(m_creature,SOUND_AGGRO);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        //If we are <50% hp cast Arcane Bubble and start casting SPECIAL FIRE AOE
        if (me->GetHealthPercent() <= 50 && !m_creature->IsNonMeleeSpellCast(false))
        {
            if (_ChangedNamePolymorph_Timer.Expired(diff))
            {
                Unit* target = NULL;

                target = SelectUnit(SELECT_TARGET_RANDOM,0);
                if (target)DoCast(target,SPELL_POLYMORPH);
                _ChangedNamePolymorph_Timer = 40000;
            }

            if (_ChangedNameYell_Timer.Expired(diff))
            {
                DoYell(-1200272,LANG_UNIVERSAL,NULL);
                DoPlaySoundToSet(m_creature,SOUND_SPECIALAE);
                _ChangedNameYell_Timer = 40000;
            }

            if (_ChangedNameArcaneBubble_Timer.Expired(diff))
            {
                DoCast(m_creature,SPELL_ARCANEBUBBLE);
                _ChangedNameArcaneBubble_Timer = 40000;
            }

            if (_ChangedNameFullAOE_Timer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(),SPELL_FIREAOE);
                _ChangedNameFullAOE_Timer = 40000;
            }
        }

        if (_ChangedNameAoESilence_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_AOESILENCE);
            _ChangedNameAoESilence_Timer = 30000;
        }

        if (_ChangedNameArcaneExplosion3_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_ARCANEEXPLOSION3);
            _ChangedNameArcaneExplosion3_Timer = 8000;
        }

        if (_ChangedNameArcaneExplosion4_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_ARCANEEXPLOSION4);
            _ChangedNameArcaneExplosion4_Timer = 10000;
        }

        if (_ChangedNameBlink_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_BLINK);
            _ChangedNameBlink_Timer = 30000;
        }

        if (_ChangedNameFireball_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FIREBALL);
            _ChangedNameFireball_Timer = 12000;
        }

        if (_ChangedNameManaShield4_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_MANASHIELD4);
            _ChangedNameManaShield4_Timer = 70000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_arcanist_doan(Creature *_Creature)
{
    return new boss_arcanist_doanAI (_Creature);
}

void AddSC_boss_arcanist_doan()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_arcanist_doan";
    newscript->GetAI = &GetAI_boss_arcanist_doan;
    newscript->RegisterSelf();
}


