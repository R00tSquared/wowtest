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
SDName: Boss_Anubshiah
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"

#define SPELL_SHADOWBOLT            17228
#define SPELL_CURSEOFTONGUES        15470
#define SPELL_CURSEOFWEAKNESS       17227
#define SPELL_DEMONARMOR            11735
#define SPELL_ENVELOPINGWEB         15471

struct boss_anubshiahAI : public ScriptedAI
{
    boss_anubshiahAI(Creature *c) : ScriptedAI(c) {}

    Timer ShadowBolt_Timer;
    Timer CurseOfTongues_Timer;
    Timer CurseOfWeakness_Timer;
    Timer DemonArmor_Timer;
    Timer EnvelopingWeb_Timer;

    void Reset()
    {
        ShadowBolt_Timer.Reset(7000);
        CurseOfTongues_Timer.Reset(24000);
        CurseOfWeakness_Timer.Reset(12000);
        DemonArmor_Timer.Reset(3000);
        EnvelopingWeb_Timer.Reset(16000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (ShadowBolt_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SHADOWBOLT);
            ShadowBolt_Timer = 7000;
        }
            

        if (CurseOfTongues_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 200, true))
                DoCast(target,SPELL_CURSEOFTONGUES);
            CurseOfTongues_Timer = 18000;
        }
        
        if (CurseOfWeakness_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_CURSEOFWEAKNESS);
            CurseOfWeakness_Timer = 45000;
        }
        

        if (DemonArmor_Timer.Expired(diff))
        {
            DoCast(me,SPELL_DEMONARMOR);
            DemonArmor_Timer = 300000;
        }
        
        if (EnvelopingWeb_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_ENVELOPINGWEB);
            EnvelopingWeb_Timer = 12000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_anubshiah(Creature *creature)
{
    return new boss_anubshiahAI (creature);
}

void AddSC_boss_anubshiah()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_anubshiah";
    newscript->GetAI = &GetAI_boss_anubshiah;
    newscript->RegisterSelf();
}

