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
SDName: Boss_Baroness_Anastari
SD%Complete: 90
SDComment: MC disabled
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "def_stratholme.h"

#define SPELL_BANSHEEWAIL   16565
#define SPELL_BANSHEECURSE    16867
#define SPELL_SILENCE    18327
//#define SPELL_POSSESS   17244

struct boss_baroness_anastariAI : public ScriptedAI
{
    boss_baroness_anastariAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)m_creature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameBansheeWail_Timer;
    Timer _ChangedNameBansheeCurse_Timer;
    Timer _ChangedNameSilence_Timer;
    //uint32 Possess_Timer;

    void Reset()
    {
        _ChangedNameBansheeWail_Timer.Reset(1000);
        _ChangedNameBansheeCurse_Timer.Reset(11000);
        _ChangedNameSilence_Timer.Reset(13000);
        //Possess_Timer = 35000;
    }

    void EnterCombat(Unit *who)
    {
         if (pInstance)
             pInstance->SetData(TYPE_BARONESS,IN_PROGRESS);
    }

     void JustDied(Unit* Killer)
     {
         if (pInstance)
             pInstance->SetData(TYPE_BARONESS,SPECIAL);
     }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameBansheeWail_Timer.Expired(diff))
        {
            if (rand()%100 < 95)
                DoCast(m_creature->GetVictim(),SPELL_BANSHEEWAIL);
            //4 seconds until we should cast this again
            _ChangedNameBansheeWail_Timer = 4000;
        }

        if (_ChangedNameBansheeCurse_Timer.Expired(diff))
        {
            if (rand()%100 < 75)
                DoCast(m_creature->GetVictim(),SPELL_BANSHEECURSE);
            //18 seconds until we should cast this again
            _ChangedNameBansheeCurse_Timer = 18000;
        }

        if (_ChangedNameSilence_Timer.Expired(diff))
        {
            if (rand()%100 < 80)
                DoCast(m_creature->GetVictim(),SPELL_SILENCE);
            //13 seconds until we should cast this again
            _ChangedNameSilence_Timer = 13000;
        }

        /*  Possess_Timer -= diff;
                    if (Possess_Timer <= diff)
        {
        //Cast
          if (rand()%100 < 65)
        {
        Unit* target = NULL;
        target = SelectUnit(SELECT_TARGET_RANDOM,0);
        if (target)DoCast(target,SPELL_POSSESS);
        }
        //50 seconds until we should cast this again
        Possess_Timer.Delay(50000);
        }
        */

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_baroness_anastari(Creature *_Creature)
{
    return new boss_baroness_anastariAI (_Creature);
}

void AddSC_boss_baroness_anastari()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_baroness_anastari";
    newscript->GetAI = &GetAI_boss_baroness_anastari;
    newscript->RegisterSelf();
}


