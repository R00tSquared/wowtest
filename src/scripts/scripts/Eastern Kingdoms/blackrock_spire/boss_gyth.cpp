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
SDName: Boss_Gyth
SD%Complete: 100
SDComment:
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"
#include "def_blackrock_spire.h"

enum
{
    SAY_NEFARIUS_BUFF_GYTH  = -1229017,
    EMOTE_KNOCKED_OFF       = -1229019,

    SPELL_CHROMATIC_CHAOS   = 16337,                // casted by Nefarius at 50%
    SPELL_REND_MOUNTS       = 16167,
    SPELL_SUMMON_REND       = 16328,
    SPELL_CORROSIVE_ACID    = 16359, // or 20667, don't know
    SPELL_FREEZE            = 16350,
    SPELL_FLAME_BREATH      = 20712,
    SPELL_KNOCK_AWAY        = 10101,
};

struct boss_gythAI : public ScriptedAI
{
    boss_gythAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (instance_blackrock_spire*)c->GetInstanceData();
    }

    instance_blackrock_spire* pInstance;

    uint32 CorrosiveAcidTimer;
    uint32 FreezeTimer;
    uint32 FlamebreathTimer;
    uint32 KnockAwayTimer;

    bool SummonedRend;
    bool HasChromaticChaos;

    void Reset()
    {
        CorrosiveAcidTimer = 8000;
        FreezeTimer        = 11000;
        FlamebreathTimer   = 4000;
        KnockAwayTimer     = 23000;
        SummonedRend       = false;
        HasChromaticChaos  = false;

        DoCast(me, SPELL_REND_MOUNTS);
    }

    void JustDied(Unit* pKiller)
    {
        if (!SummonedRend)
        {
            DoCast(me, SPELL_SUMMON_REND);
            me->RemoveAurasDueToSpell(SPELL_REND_MOUNTS);
            SummonedRend = true;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        DoScriptText(EMOTE_KNOCKED_OFF, pSummoned);
    }

    void DamageTaken(Unit* pKiller, uint32& damage)
    {
        if(!SummonedRend)
        {
            if (damage >= me->GetHealth())
                me->SetHealth(me->GetMaxHealth() * 0.10);
        }
    }
        
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // Chromatic Chaos at 50%
        if (!HasChromaticChaos && me->GetHealthPercent() < 50.0f)
        {
            if (pInstance)
            {
                if (Creature* pNefarius = pInstance->instance->GetCreatureById(NPC_LORD_VICTOR_NEFARIUS))
                {
                    pNefarius->CastSpell(me, SPELL_CHROMATIC_CHAOS, true);
                    DoScriptText(SAY_NEFARIUS_BUFF_GYTH, pNefarius);
                    HasChromaticChaos = true;
                }
            }
        }

        // CorrosiveAcid_Timer
        if (CorrosiveAcidTimer < diff)
        {
            AddSpellToCast(me, SPELL_CORROSIVE_ACID);
            CorrosiveAcidTimer = 7000;
        }
        else
            CorrosiveAcidTimer -= diff;

        // Freeze_Timer
        if (FreezeTimer < diff)
        {
            AddSpellToCast(me, SPELL_FREEZE);
            FreezeTimer = 16000;
        }
        else
            FreezeTimer -= diff;

        // Flamebreath_Timer
        if (FlamebreathTimer < diff)
        {
            AddSpellToCast(me, SPELL_FLAME_BREATH);
            FlamebreathTimer = 10500;
        }
        else
            FlamebreathTimer -= diff;

        if (KnockAwayTimer < diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_KNOCK_AWAY);
            KnockAwayTimer = 23000;
        }
        else
            KnockAwayTimer -= diff;

        // Summon Rend
        if (!SummonedRend && me->GetHealthPercent() < 11.0f)
        {
            DoCast(me, SPELL_SUMMON_REND);
            me->RemoveAurasDueToSpell(SPELL_REND_MOUNTS);
            SummonedRend = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_gyth(Creature *_Creature)
{
    return new boss_gythAI (_Creature);
}

void AddSC_boss_gyth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gyth";
    newscript->GetAI = &GetAI_boss_gyth;
    newscript->RegisterSelf();
}


