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
SDName: Boss_Moira_Bronzbeard
SD%Complete: 92
SDComment: Healing of Emperor NYI
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

#define SPELL_HEAL              10917
#define SPELL_RENEW             10929
#define SPELL_SHIELD            10901
#define SPELL_MINDBLAST         10947
#define SPELL_SHADOWWORDPAIN    10894
#define SPELL_SMITE             10934

struct boss_moira_bronzebeardAI : public ScriptedAI
{
    boss_moira_bronzebeardAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameMindBlast_Timer;
    Timer _ChangedNameShadowWordPain_Timer;
    Timer _ChangedNameSmite_Timer;
    bool Heal;

    void Reset()
    {                      //These times are probably wrong
        _ChangedNameMindBlast_Timer.Reset(16000);
        _ChangedNameShadowWordPain_Timer.Reset(2000);
        _ChangedNameSmite_Timer.Reset(8000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameMindBlast_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_MINDBLAST);
            _ChangedNameMindBlast_Timer = 14000;
        }
        

        if (_ChangedNameShadowWordPain_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SHADOWWORDPAIN);
            _ChangedNameShadowWordPain_Timer = 18000;
        }
        

        if (_ChangedNameSmite_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SMITE);
            _ChangedNameSmite_Timer = 10000;
        }
           

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_moira_bronzebeard(Creature *creature)
{
    return new boss_moira_bronzebeardAI (creature);
}

void AddSC_boss_moira_bronzebeard()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_moira_bronzebeard";
    newscript->GetAI = &GetAI_boss_moira_bronzebeard;
    newscript->RegisterSelf();
}


