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
SDName: Boss_Interrogator_Vishas
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"

#define SPELL_POWERWORDSHIELD           6065

#define SAY_AGGRO                       -1200281
#define SAY_HEALTH1                     -1200282
#define SAY_HEALTH2                     -1200283
//#define SAY_DEATH                       "Purged by pain!"

#define SOUND_AGGRO                     5847
#define SOUND_HEALTH1                   5849
#define SOUND_HEALTH2                   5850
#define SOUND_DEATH                     5848

struct boss_interrogator_vishasAI : public ScriptedAI
{
    boss_interrogator_vishasAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameYell_Timer;
    Timer _ChangedNamePowerWordShield_Timer;

    void Reset()
    {
        _ChangedNameYell_Timer.Reset(6000000);
        _ChangedNamePowerWordShield_Timer.Reset(60000);
    }

    void EnterCombat(Unit *who)
    {
        DoYell(-1200281,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(m_creature,SOUND_AGGRO);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        //If we are low on hp Do sayings
        if ( me->GetHealthPercent() <= 60 && !m_creature->IsNonMeleeSpellCast(false))
        {
            if (_ChangedNameYell_Timer.Expired(diff))
            {
                DoYell(-1200282,LANG_UNIVERSAL,NULL);
                DoPlaySoundToSet(m_creature,SOUND_HEALTH1);
                return;

                //60 seconds until we should cast this agian
                _ChangedNameYell_Timer = 60000;
            }
        }

        if ( me->GetHealthPercent() <= 30 && !m_creature->IsNonMeleeSpellCast(false))
        {
            if (_ChangedNameYell_Timer.Expired(diff))
            {
                DoYell(-1200283,LANG_UNIVERSAL,NULL);
                DoPlaySoundToSet(m_creature,SOUND_HEALTH2);
                return;

                //60 seconds until we should cast this agian
                _ChangedNameYell_Timer = 6000000;
            }
        }

        if (_ChangedNamePowerWordShield_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_POWERWORDSHIELD);
            _ChangedNamePowerWordShield_Timer = 60000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_interrogator_vishas(Creature *_Creature)
{
    return new boss_interrogator_vishasAI (_Creature);
}

void AddSC_boss_interrogator_vishas()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_interrogator_vishas";
    newscript->GetAI = &GetAI_boss_interrogator_vishas;
    newscript->RegisterSelf();
}


