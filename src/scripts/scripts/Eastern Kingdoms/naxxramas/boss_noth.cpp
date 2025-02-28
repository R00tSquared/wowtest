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
SDName: Boss_Noth
SD%Complete: 99
SDComment: Debug
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum
{
    SAY_AGGRO1                          = -1533075,
    SAY_AGGRO2                          = -1533076,
    SAY_AGGRO3                          = -1533077,
    SAY_SUMMON                          = -1533078,
    SAY_SLAY1                           = -1533079,
    SAY_SLAY2                           = -1533080,
    SAY_DEATH                           = -1533081,

    EMOTE_WARRIOR                       = -1533130,
    EMOTE_SKELETON                      = -1533131,
    EMOTE_TELEPORT                      = -1533132,
    EMOTE_TELEPORT_RETURN               = -1533133,

    SPELL_TELEPORT                      = 29216,
    SPELL_TELEPORT_RETURN               = 29231,

    SPELL_BLINK_1                       = 29208,
    SPELL_BLINK_2                       = 29209,
    SPELL_BLINK_3                       = 29210,
    SPELL_BLINK_4                       = 29211,

    SPELL_CRIPPLE                       = 29212,
    SPELL_CURSE_PLAGUEBRINGER           = 29213,

    SPELL_BERSERK                       = 26662,            // guesswork, but very common berserk spell in naxx

    SPELL_SUMMON_WARRIOR_1              = 29247,
    SPELL_SUMMON_WARRIOR_2              = 29248,
    SPELL_SUMMON_WARRIOR_3              = 29249,

    SPELL_SUMMON_WARRIOR_THREE          = 29237,

    SPELL_SUMMON_CHAMP01                = 29217,
    SPELL_SUMMON_CHAMP02                = 29224,
    SPELL_SUMMON_CHAMP03                = 29225,
    SPELL_SUMMON_CHAMP04                = 29227,
    SPELL_SUMMON_CHAMP05                = 29238,
    SPELL_SUMMON_CHAMP06                = 29255,
    SPELL_SUMMON_CHAMP07                = 29257,
    SPELL_SUMMON_CHAMP08                = 29258,
    SPELL_SUMMON_CHAMP09                = 29262,
    SPELL_SUMMON_CHAMP10                = 29267,

    SPELL_SUMMON_GUARD01                = 29226,
    SPELL_SUMMON_GUARD02                = 29239,
    SPELL_SUMMON_GUARD03                = 29256,
    SPELL_SUMMON_GUARD04                = 29268,

    PHASE_GROUND                        = 0,
    PHASE_BALCONY                       = 1,

    PHASE_SKELETON_1                    = 1,
    PHASE_SKELETON_2                    = 2,
    PHASE_SKELETON_3                    = 3
};

struct boss_nothAI : public ScriptedAI
{
    boss_nothAI(Creature* c) : ScriptedAI(c), summons(me)
    {
        pInstance = (instance_naxxramas*)c->GetInstanceData();
        Reset();
    }

    instance_naxxramas* pInstance;
    SummonList summons;

    uint8 Phase;
    uint8 PhaseSub;
    Timer PhaseTimer;

    Timer BlinkTimer;
    Timer CurseTimer;
    Timer SummonTimer;
    Timer CheckFlagTimer;

    void Reset()
    {
        Phase = PHASE_GROUND;
        PhaseSub = PHASE_GROUND;
        PhaseTimer.Reset(90000);

        BlinkTimer.Reset(25000);
        CurseTimer.Reset(4000);
        SummonTimer.Reset(12000);
        CheckFlagTimer.Reset(3000);
        summons.DespawnAll();
    }

    void EnterCombat(Unit *who)
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_AGGRO1, me); break;
            case 1: DoScriptText(SAY_AGGRO2, me); break;
            case 2: DoScriptText(SAY_AGGRO3, me); break;
        }

        if (pInstance)
            pInstance->SetData(DATA_NOTH_THE_PLAGUEBRINGER, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->DoZoneInCombat();
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
            summoned->AI()->AttackStart(target);
        summons.Summon(summoned);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
        if(pInstance)
            pInstance->SetData(DATA_NOTH_THE_PLAGUEBRINGER, DONE);
    }

    void JustReachedHome() override
    {
        if (pInstance)
            pInstance->SetData(DATA_NOTH_THE_PLAGUEBRINGER, FAIL);
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pCaster == me && pSpell->Effect[0] == SPELL_EFFECT_LEAP)
            DoCast(me, SPELL_CRIPPLE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Phase == PHASE_GROUND)
        {
            if(CheckFlagTimer.Expired(diff))
            {
                if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                CheckFlagTimer = 3000;
            }

            if (PhaseTimer.Expired(diff))
            {
                DoCast(me, SPELL_TELEPORT);
                DoScriptText(EMOTE_TELEPORT, me);
                me->GetMotionMaster()->MoveIdle();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                Phase = PHASE_BALCONY;
                ++PhaseSub;

                switch (PhaseSub)               // Set Duration of Skeleton phase
                {
                    case PHASE_SKELETON_1: PhaseTimer = 70000;  break;
                    case PHASE_SKELETON_2: PhaseTimer = 97000;  break;
                    case PHASE_SKELETON_3: PhaseTimer = 120000; break;
                }
                return;
            }

            if (BlinkTimer.Expired(diff))
            {
                static uint32 const auiSpellBlink[4] =
                {
                    SPELL_BLINK_1, SPELL_BLINK_2, SPELL_BLINK_3, SPELL_BLINK_4
                };

                DoCast(me, auiSpellBlink[urand(0, 3)]);
                DoResetThreat();
                BlinkTimer = 25000;
            }

            if (CurseTimer.Expired(diff))
            {
                me->CastCustomSpell(SPELL_CURSE_PLAGUEBRINGER, SPELLVALUE_MAX_TARGETS, 6, NULL, true); //HeroicMode ? 6 : 20
                CurseTimer = 28000;
            }

            if (SummonTimer.Expired(diff))
            {
                DoScriptText(SAY_SUMMON, me);
                DoScriptText(EMOTE_WARRIOR, me);

                // It's not very clear how many warriors it should summon, so we'll just leave it as random for now
                if (urand(0, 1))
                {
                    static uint32 const auiSpellSummonPlaguedWarrior[3] =
                    {
                        SPELL_SUMMON_WARRIOR_1, SPELL_SUMMON_WARRIOR_2, SPELL_SUMMON_WARRIOR_3
                    };

                    for (uint8 i = 0; i < 2; ++i)
                        DoCast(me, auiSpellSummonPlaguedWarrior[urand(0, 2)], true);
                }
                else
                    DoCast(me, SPELL_SUMMON_WARRIOR_THREE, true);

                SummonTimer = 30000;
            }

            DoMeleeAttackIfReady();
        }
        else                                                // PHASE_BALCONY
        {
            if(CheckFlagTimer.Expired(diff))
            {
                if(!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                CheckFlagTimer = 3000;
            }

            if (PhaseTimer.Expired(diff))
            {
                DoCast(me, SPELL_TELEPORT_RETURN);
                DoScriptText(EMOTE_TELEPORT_RETURN, me);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->GetMotionMaster()->MoveChase(me->GetVictim());
                switch (PhaseSub)
                {
                    case PHASE_SKELETON_1: PhaseTimer = 110000; break;
                    case PHASE_SKELETON_2: PhaseTimer = 180000; break;
                    case PHASE_SKELETON_3:
                        PhaseTimer = 0;
                        // Go Berserk after third Balcony Phase
                        DoCast(me, SPELL_BERSERK, true);
                        break;
                }
                Phase = PHASE_GROUND;

                return;
            }

            if (SummonTimer.Expired(diff))
            {
                DoScriptText(EMOTE_SKELETON, me);

                static uint32 const auiSpellSummonPlaguedChampion[10] =
                {
                    SPELL_SUMMON_CHAMP01, SPELL_SUMMON_CHAMP02, SPELL_SUMMON_CHAMP03, SPELL_SUMMON_CHAMP04, SPELL_SUMMON_CHAMP05, SPELL_SUMMON_CHAMP06, SPELL_SUMMON_CHAMP07, SPELL_SUMMON_CHAMP08, SPELL_SUMMON_CHAMP09, SPELL_SUMMON_CHAMP10
                };

                static uint32 const auiSpellSummonPlaguedGuardian[4] =
                {
                    SPELL_SUMMON_GUARD01, SPELL_SUMMON_GUARD02, SPELL_SUMMON_GUARD03, SPELL_SUMMON_GUARD04
                };

                // A bit unclear how many in each sub phase
                switch (PhaseSub)
                {
                    case PHASE_SKELETON_1:
                    {
                        for (uint8 i = 0; i < 4; ++i)
                            DoCast(me, auiSpellSummonPlaguedChampion[urand(0, 9)], true);

                        break;
                    }
                    case PHASE_SKELETON_2:
                    {
                        for (uint8 i = 0; i < 2; ++i)
                        {
                            DoCast(me, auiSpellSummonPlaguedChampion[urand(0, 9)], true);
                            DoCast(me, auiSpellSummonPlaguedGuardian[urand(0, 3)], true);
                        }
                        break;
                    }
                    case PHASE_SKELETON_3:
                    {
                        for (uint8 i = 0; i < 4; ++i)
                            DoCast(me, auiSpellSummonPlaguedGuardian[urand(0, 3)], true);

                        break;
                    }
                }

                SummonTimer = 30000;
            }
        }
    }
};

CreatureAI* GetAI_boss_noth(Creature *_Creature)
{
    return new boss_nothAI (_Creature);
}

void AddSC_boss_noth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_noth";
    newscript->GetAI = &GetAI_boss_noth;
    newscript->RegisterSelf();
}
