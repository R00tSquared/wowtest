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
SDName: Boss_Flamegor
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

#define EMOTE_FRENZY            -1469031

#define SPELL_SHADOWFLAME        22539
#define SPELL_WINGBUFFET         23339
#define SPELL_FRENZY             23342                      //This spell periodically triggers fire nova

struct boss_flamegorAI : public ScriptedAI
{
    boss_flamegorAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    Timer _ChangedNameShadowFlame_Timer;
    Timer _ChangedNameWingBuffet_Timer;
    Timer _ChangedNameFrenzy_Timer;

    void Reset()
    {
        _ChangedNameShadowFlame_Timer.Reset(21000);                          //These times are probably wrong
        _ChangedNameWingBuffet_Timer.Reset(35000);
        _ChangedNameFrenzy_Timer.Reset(10000);

        if (pInstance && pInstance->GetData(DATA_FLAMEGOR_EVENT) != DONE)
            pInstance->SetData(DATA_FLAMEGOR_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();

        if (pInstance)
            pInstance->SetData(DATA_FLAMEGOR_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_FLAMEGOR_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        if (_ChangedNameShadowFlame_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWFLAME);
            _ChangedNameShadowFlame_Timer = 15000 + rand()%7000;
        }

        if (_ChangedNameWingBuffet_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_WINGBUFFET);
            if(DoGetThreat(m_creature->GetVictim()))
                DoModifyThreatPercent(m_creature->GetVictim(),-75);

            _ChangedNameWingBuffet_Timer = 25000;
        }

        if (_ChangedNameFrenzy_Timer.Expired(diff))
        {
            DoScriptText(EMOTE_FRENZY, m_creature);
            DoCast(m_creature,SPELL_FRENZY);
            _ChangedNameFrenzy_Timer = 8000 + (rand()%2000);
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_flamegor(Creature *_Creature)
{
    return new boss_flamegorAI (_Creature);
}

void AddSC_boss_flamegor()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_flamegor";
    newscript->GetAI = &GetAI_boss_flamegor;
    newscript->RegisterSelf();
}


