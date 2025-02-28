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
SDName: Boss_Magmadar
SD%Complete: 75
SDComment: Conflag on ground nyi, fear causes issues without VMAPs
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define EMOTE_FRENZY                -1409001

#define SPELL_FRENZY                19451
#define SPELL_MAGMASPIT             19449                   //This is actually a buff he gives himself
#define SPELL_PANIC                 19408
#define SPELL_LAVABOMB              19411                   //This calls a dummy server side effect that isn't implemented yet
#define SPELL_LAVABOMB_ALT          19428                   //This is the spell that the lava bomb casts

struct boss_magmadarAI : public ScriptedAI
{
    boss_magmadarAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    Timer _ChangedNameFrenzy_Timer;
    Timer _ChangedNamePanic_Timer;
    Timer _ChangedNameLavabomb_Timer;

    void Reset()
    {
        _ChangedNameFrenzy_Timer.Reset(30000);
        _ChangedNamePanic_Timer.Reset(20000);
        _ChangedNameLavabomb_Timer.Reset(12000);

        m_creature->CastSpell(m_creature,SPELL_MAGMASPIT,true);

        if (pInstance)
            pInstance->SetData(DATA_MAGMADAR_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_MAGMADAR_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_MAGMADAR_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameFrenzy_Timer.Expired(diff))
        {
            DoScriptText(EMOTE_FRENZY, m_creature);
            DoCast(m_creature,SPELL_FRENZY);
            _ChangedNameFrenzy_Timer = 15000;
        }

        if (_ChangedNamePanic_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_PANIC);
            _ChangedNamePanic_Timer = 35000;
        }

        if (_ChangedNameLavabomb_Timer.Expired(diff))
        {
            if( Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0) )
                DoCast(target,SPELL_LAVABOMB_ALT);

            _ChangedNameLavabomb_Timer = 12000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_magmadar(Creature *_Creature)
{
    return new boss_magmadarAI (_Creature);
}

void AddSC_boss_magmadar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_magmadar";
    newscript->GetAI = &GetAI_boss_magmadar;
    newscript->RegisterSelf();
}


