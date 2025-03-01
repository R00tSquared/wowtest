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
SDName: boss_timmy_the_cruel
SD%Complete: 100
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"

#define SAY_SPAWN   -1200297

#define SPELL_RAVENOUSCLAW    17470

struct boss_timmy_the_cruelAI : public ScriptedAI
{
    boss_timmy_the_cruelAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameRavenousClaw_Timer;
    bool HasYelled;

    void Reset()
    {
        _ChangedNameRavenousClaw_Timer.Reset(10000);
        HasYelled = false;
    }

    void EnterCombat(Unit *who)
    {
        if (!HasYelled)
        {
            DoYell(-1200297,LANG_UNIVERSAL,NULL);
            HasYelled = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (_ChangedNameRavenousClaw_Timer.Expired(diff))
        {
            //Cast
            DoCast(m_creature->GetVictim(),SPELL_RAVENOUSCLAW);
            //15 seconds until we should cast this again
            _ChangedNameRavenousClaw_Timer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_timmy_the_cruel(Creature *_Creature)
{
    return new boss_timmy_the_cruelAI (_Creature);
}

void AddSC_boss_timmy_the_cruel()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_timmy_the_cruel";
    newscript->GetAI = &GetAI_boss_timmy_the_cruel;
    newscript->RegisterSelf();
}


