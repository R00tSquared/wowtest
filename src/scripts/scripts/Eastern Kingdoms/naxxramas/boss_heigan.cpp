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
SDName: Boss_Heigan
SD%Complete: 98
SDComment: Debug. Gates. Missing poison inside the tunnel in phase 2 -- removed in TBC
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum
{
    PHASE_GROUND            = 1,
    PHASE_PLATFORM          = 2,

    SAY_AGGRO1              = -1533109,
    SAY_AGGRO2              = -1533110,
    SAY_AGGRO3              = -1533111,
    SAY_SLAY                = -1533112,
    SAY_TAUNT1              = -1533113,
    SAY_TAUNT2              = -1533114,
    SAY_TAUNT3              = -1533115,
    SAY_TAUNT4              = -1533117,
    SAY_CHANNELING          = -1533116,
    SAY_DEATH               = -1533118,
    EMOTE_TELEPORT          = -1533136,
    EMOTE_RETURN            = -1533137,

    // Spells by boss
    SPELL_DECREPIT_FEVER    = 29998,
    SPELL_DISRUPTION        = 29310, // Mana Burn
    SPELL_TELEPORT          = 30211,
    SPELL_PLAGUE_CLOUD      = 29350,
    // SPELL_PLAGUE_WAVE_SLOW    = 29351,                // removed from dbc. activates the traps during phase 1; triggers spell 30116, 30117, 30118, 30119 each 10 secs
    // SPELL_PLAGUE_WAVE_FAST    = 30114,                // removed from dbc. activates the traps during phase 2; triggers spell 30116, 30117, 30118, 30119 each 3 secs

    MAX_PLAYERS_TELEPORT    = 3,
    
    NPC_PLAGUE_WAVE         = 17293
};

static const float aTunnelLoc[4] = {2905.63f, -3769.96f, 273.62f, 3.13f};

struct boss_heiganAI : public ScriptedAI
{
    boss_heiganAI(Creature* pCreature) : ScriptedAI(pCreature), Summons(me)
    {
        pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    instance_naxxramas* m_pInstance;

    SummonList Summons;

    uint8 Phase;
    uint8 PhaseEruption;

    uint32 TeleportTimer;
    uint32 FeverTimer;
    uint32 DisruptionTimer;
    uint32 EruptionTimer;
    uint32 PhaseTimer;
    uint32 TauntTimer;
    uint32 StartChannelingTimer;

    void ResetPhase()
    {
        PhaseEruption = 0;
        FeverTimer = 4000;
        EruptionTimer = Phase == PHASE_GROUND ? 15000 : 7500;
        DisruptionTimer = 5000;
        StartChannelingTimer = 1000;
        PhaseTimer = Phase == PHASE_GROUND ? 90000 : 45000;
        TeleportTimer = 60000;
    }

    void Reset()
    {
        Phase = PHASE_GROUND;
        TauntTimer = urand(20000, 60000);               // TODO, find information
        ResetPhase();
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, me); break;
            case 1: DoScriptText(SAY_AGGRO2, me); break;
            case 2: DoScriptText(SAY_AGGRO3, me); break;
        }

        if (pInstance)
            pInstance->SetData(DATA_HEIGAN_THE_UNCLEAN, IN_PROGRESS);
    }

    void JustSummoned(Creature* pSummoned)
    {
        Summons.Summon(pSummoned);
        if(pSummoned->GetEntry() == NPC_PLAGUE_WAVE) // Do not delete this, it's needed for not available cast
        {
            pSummoned->setFaction(me->getFaction());
            pSummoned->SetLevel(me->GetLevel());
            pSummoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            pSummoned->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);
            pSummoned->SetReactState(REACT_PASSIVE);
        }
    }

    void KilledUnit(Unit* /*pVictim*/)
    {
        DoScriptText(SAY_SLAY, me);
    }
    
    void JustDied(Unit* /*pKiller*/)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(DATA_HEIGAN_THE_UNCLEAN, DONE);

        Summons.DespawnAll();
    }

    void JustReachedHome()
    {
        if (pInstance)
            pInstance->SetData(DATA_HEIGAN_THE_UNCLEAN, FAIL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Phase == PHASE_GROUND)
        {
            // Teleport to platform
            if (PhaseTimer < diff)
            {
                AddSpellToCast(SPELL_TELEPORT, CAST_SELF);
                DoScriptText(EMOTE_TELEPORT, me);
                me->GetMotionMaster()->MoveIdle();
                Phase = PHASE_PLATFORM;
                ResetPhase();
            }
            else
                PhaseTimer -= diff;

            // Fever
            if (FeverTimer < diff)
            {
                AddSpellToCast(SPELL_DECREPIT_FEVER, CAST_RANDOM);
                FeverTimer = 21000;
            }
            else
                FeverTimer -= diff;

            // Disruption
            if (DisruptionTimer < diff)
            {
                AddSpellToCast(SPELL_DISRUPTION, CAST_NULL);
                DisruptionTimer = 10000;
            }
            else
                DisruptionTimer -= diff;

            if (TeleportTimer < diff)
            {
                float fX, fY, fZ;
                // Teleport players in the tunnel
                for (uint8 i = 0; i < MAX_PLAYERS_TELEPORT; i++)
                {
                    if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                    {
                        me->GetRandomPoint(aTunnelLoc[0], aTunnelLoc[1], aTunnelLoc[2], 5.0f, fX, fY, fZ);
                        pTarget->NearTeleportTo(fX, fY, fZ, aTunnelLoc[3]);
                    }
                }
                TeleportTimer = 70000;
            }
            else
                TeleportTimer -= diff;

        }
        else                                                // Platform Phase
        {
            if (PhaseTimer <= diff)                   // Return to fight
            {
                me->InterruptNonMeleeSpells(true);
                DoScriptText(EMOTE_RETURN, me);
                me->GetMotionMaster()->MoveChase(me->GetVictim());

                Phase = PHASE_GROUND;
                ResetPhase();
                return;
            }
            else
                PhaseTimer -= diff;

            if (StartChannelingTimer)
            {
                if (StartChannelingTimer <= diff)
                {
                    DoScriptText(SAY_CHANNELING, me);
                    AddSpellToCast(SPELL_PLAGUE_CLOUD, CAST_NULL);
                    StartChannelingTimer = 0; // no more
                }
                else
                    StartChannelingTimer -= diff;
            }
        }

        // Taunt
        if (TauntTimer < diff)
        {
            switch (urand(0, 3))
            {
                case 0: DoScriptText(SAY_TAUNT1, me); break;
                case 1: DoScriptText(SAY_TAUNT2, me); break;
                case 2: DoScriptText(SAY_TAUNT3, me); break;
                case 3: DoScriptText(SAY_TAUNT4, me); break;
            }
            TauntTimer = urand(20000, 70000);
        }
        else
            TauntTimer -= diff;

        // Handling of the erruptions, this is not related to melee attack or spell-casting
        if (!m_pInstance)
            return;

        // Eruption
        if (EruptionTimer < diff)
        {
            for (uint8 uiArea = 0; uiArea < MAX_HEIGAN_TRAP_AREAS; ++uiArea)
            {
                // Actually this is correct :P
                if (uiArea == (PhaseEruption % 6) || uiArea == 6 - (PhaseEruption % 6))
                    continue;

                m_pInstance->DoTriggerHeiganTraps(uiArea, me->GetGUID());
            }

            EruptionTimer = Phase == PHASE_GROUND ? 10000 : 3000;
            ++PhaseEruption;
        }
        else
            EruptionTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_heigan(Creature* pCreature)
{
    return new boss_heiganAI(pCreature);
}

void AddSC_boss_heigan()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_heigan";
    newscript->GetAI = &GetAI_boss_heigan;
    newscript->RegisterSelf();
}
