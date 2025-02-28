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
SDName: Boss_Halycon
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_CROWDPUMMEL       10887
#define SPELL_MIGHTYBLOW        14099

#define ADD_1X                  -169.839203
#define ADD_1Y                  -324.961395
#define ADD_1Z                  64.401443
#define ADD_1O                  3.124724

struct boss_halyconAI : public ScriptedAI
{
    boss_halyconAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameCrowdPummel_Timer;
    Timer _ChangedNameMightyBlow_Timer;
    bool Summoned;

    void Reset()
    {
        _ChangedNameCrowdPummel_Timer.Reset(8000);
        _ChangedNameMightyBlow_Timer.Reset(14000);
        Summoned = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameCrowdPummel_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CROWDPUMMEL);
            _ChangedNameCrowdPummel_Timer = 14000;
        }


        if (_ChangedNameMightyBlow_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MIGHTYBLOW);
            _ChangedNameMightyBlow_Timer = 10000;
        }

        //Summon Gizrul
        if ( !Summoned && me->GetHealthPercent() < 25 )
        {
            m_creature->SummonCreature(10268,ADD_1X,ADD_1Y,ADD_1Z,ADD_1O,TEMPSUMMON_TIMED_DESPAWN,300000);
            Summoned = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_halycon(Creature *_Creature)
{
    return new boss_halyconAI (_Creature);
}

void AddSC_boss_halycon()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_halycon";
    newscript->GetAI = &GetAI_boss_halycon;
    newscript->RegisterSelf();
}


