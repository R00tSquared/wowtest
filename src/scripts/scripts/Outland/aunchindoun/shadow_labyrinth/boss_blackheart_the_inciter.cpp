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
SDName: Boss_Blackheart_the_Inciter
SD%Complete: 100
SDComment:
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "precompiled.h"
#include "def_shadow_labyrinth.h"
#include "PlayerAI.h"

#define SPELL_INCITE_CHAOS    33676
#define SPELL_CHARGE          33709
#define SPELL_WAR_STOMP       33707

#define SAY_INTRO1          -1555008
#define SAY_INTRO2          -1555009
#define SAY_INTRO3          -1555010
#define SAY_AGGRO1          -1555011
#define SAY_AGGRO2          -1555012
#define SAY_AGGRO3          -1555013
#define SAY_SLAY1           -1555014
#define SAY_SLAY2           -1555015
#define SAY_HELP            -1555016
#define SAY_DEATH           -1555017

#define SAY2_INTRO1         -1555018
#define SAY2_INTRO2         -1555019
#define SAY2_INTRO3         -1555020
#define SAY2_AGGRO1         -1555021
#define SAY2_AGGRO2         -1555022
#define SAY2_AGGRO3         -1555023
#define SAY2_SLAY1          -1555024
#define SAY2_SLAY2          -1555025
#define SAY2_HELP           -1555026
#define SAY2_DEATH          -1555027

static uint32 trashEntry[]=
{
    18631,
    18633,
    18635,
    18637,
    18640,
    18642,
    18848
};

struct boss_blackheart_the_inciterAI : public ScriptedAI
{
    boss_blackheart_the_inciterAI(Creature *c) : ScriptedAI(c), Summons(me)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    SummonList Summons;

    bool InciteChaos;
    Timer InciteChaos_Timer;
    Timer InciteChaosWait_Timer;
    Timer Charge_Timer;
    Timer Knockback_Timer;

    void Reset()
    {
        InciteChaos = false;
        InciteChaos_Timer.Reset(20000);
        InciteChaosWait_Timer.Reset(15000);
        Charge_Timer.Reset(5000);
        Knockback_Timer.Reset(15000);

        if (pInstance)
            pInstance->SetData(DATA_BLACKHEARTTHEINCITEREVENT, NOT_STARTED);
        Summons.DespawnAll();
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustSummoned(Creature* sum)
    {
        Summons.Summon(sum);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        if(pInstance)
            pInstance->SetData(DATA_BLACKHEARTTHEINCITEREVENT, DONE);
    }

    void TrashAggro()
    {
        std::list<Creature*> TrashEntryList;
        std::list<Creature*> TrashList;

        TrashList.clear();

        for(uint8 i = 0; i < 7; ++i)
        {
            TrashEntryList.clear();
            TrashEntryList = FindAllCreaturesWithEntry(trashEntry[i], 90.0f);

            for(std::list<Creature*>::iterator iter = TrashEntryList.begin(); iter != TrashEntryList.end(); ++iter)
                TrashList.push_back(*iter);
        }

        if(!TrashList.empty())
        {
            for(std::list<Creature*>::iterator i = TrashList.begin(); i != TrashList.end(); ++i)
            {
                if((*i)->GetDBTableGUIDLow() != 67088)
                {
                    (*i)->setActive(true);
                    (*i)->AI()->AttackStart(me->GetVictim());
                }
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);

        DoZoneInCombat();
        TrashAggro();

        if (pInstance)
            pInstance->SetData(DATA_BLACKHEARTTHEINCITEREVENT, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        if(InciteChaos)
        {
            if (InciteChaosWait_Timer.Expired(diff))
            {
                InciteChaosWait_Timer = 0;
                InciteChaos = false;
                DoZoneInCombat();
                DoResetThreat();
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 50, true);
                if(target)
                    AttackStart(target);
            }
            return;
        }

        if (!UpdateVictim())
            return;
        else
            TrashAggro();

        if (InciteChaos_Timer.Expired(diff))
        {
            InciteChaos_Timer = 40000;
            InciteChaosWait_Timer = 16000;
            std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
            if(m_threatlist.size() < 2)
                return;

            DoCast(me, SPELL_INCITE_CHAOS);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            InciteChaos = true;
            return;
        }

        //Charge_Timer
        if (Charge_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_FARTHEST, 0, 50, true))
                DoCast(target, SPELL_CHARGE);
            Charge_Timer = 25000;
        }

        //Knockback_Timer
        if (Knockback_Timer.Expired(diff))
        {
            DoCast(me, SPELL_WAR_STOMP);
            Knockback_Timer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_blackheart_the_inciter(Creature *_Creature)
{
    return new boss_blackheart_the_inciterAI (_Creature);
}

void AddSC_boss_blackheart_the_inciter()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_blackheart_the_inciter";
    newscript->GetAI = &GetAI_boss_blackheart_the_inciter;
    newscript->RegisterSelf();
}

