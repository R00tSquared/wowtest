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
SDName: Boss_Gatewatcher_Ironhand
SD%Complete: 75
SDComment:
SDCategory: Tempest Keep, The Mechanar
EndScriptData */

#include "precompiled.h"
#include "def_mechanar.h"

#define SAY_AGGRO_1                     -1554006
#define SAY_HAMMER_1                    -1554007
#define SAY_HAMMER_2                    -1554008
#define SAY_SLAY_1                      -1554009
#define SAY_SLAY_2                      -1554010
#define SAY_DEATH_1                     -1554011
#define EMOTE_HAMMER                    -1554012

// Spells to be cast
#define SPELL_SHADOW_POWER              35322
#define H_SPELL_SHADOW_POWER            39193
#define SPELL_HAMMER_PUNCH              35326
#define SPELL_JACKHAMMER                35327
#define H_SPELL_JACKHAMMER              39194
#define SPELL_STREAM_OF_MACHINE_FLUID   35311

// Gatewatcher Iron-Hand AI
struct boss_gatewatcher_iron_handAI : public ScriptedAI
{
    boss_gatewatcher_iron_handAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance *pInstance;

    bool HeroicMode;

    Timer ShadowPower_Timer;
    Timer Jackhammer_Timer;
    Timer StreamOfMachineFluid_Timer;
    Timer HammerYell_Timer;

    void Reset()
    {
        ShadowPower_Timer.Reset(25000);
        Jackhammer_Timer.Reset(15000);
        StreamOfMachineFluid_Timer.Reset(30000);
        HammerYell_Timer.Reset(0);

        if(pInstance)
            pInstance->SetData(DATA_IRONHAND_EVENT, NOT_STARTED);

    }
    void EnterCombat(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(DATA_IRONHAND_EVENT, IN_PROGRESS);

        DoScriptText(SAY_AGGRO_1, me);
    }

    void KilledUnit(Unit* victim)
    {
        if (rand()%2)
            return;

        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH_1, me);

        if (!pInstance)
            return;

        if(pInstance)
            pInstance->SetData(DATA_IRONHAND_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(ShadowPower_Timer.Expired(diff))
        {
            DoCast(me, HeroicMode ? H_SPELL_SHADOW_POWER : SPELL_SHADOW_POWER);
            ShadowPower_Timer = 20000 + rand()%8000;
        }

        if(Jackhammer_Timer.Expired(diff))
        {
            DoScriptText(EMOTE_HAMMER, me);
            DoCast(me->GetVictim(), HeroicMode ? H_SPELL_JACKHAMMER : SPELL_JACKHAMMER);
            HammerYell_Timer = 1500;
            Jackhammer_Timer = 30000;
        }

        if(HammerYell_Timer.Expired(diff))
        {
            DoScriptText(RAND(SAY_HAMMER_1, SAY_HAMMER_2), me);
            HammerYell_Timer = 0;
        }
    
        if(StreamOfMachineFluid_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_STREAM_OF_MACHINE_FLUID);
            StreamOfMachineFluid_Timer = 35000 + rand()%15000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_gatewatcher_iron_hand(Creature *_Creature)
{
    return new boss_gatewatcher_iron_handAI (_Creature);
}

void AddSC_boss_gatewatcher_iron_hand()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gatewatcher_iron_hand";
    newscript->GetAI = &GetAI_boss_gatewatcher_iron_hand;
    newscript->RegisterSelf();
}

