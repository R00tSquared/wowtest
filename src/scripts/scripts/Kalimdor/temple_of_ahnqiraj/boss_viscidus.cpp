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
SDName: Boss_Viscidus
SD%Complete: 90
SDComment: Server side spells implementation need to be checked.
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_temple_of_ahnqiraj.h"

enum
{
    // emotes
    EMOTE_SLOW                  = -1531041,
    EMOTE_FREEZE                = -1531042,
    EMOTE_FROZEN                = -1531043,
    EMOTE_CRACK                 = -1531044,
    EMOTE_SHATTER               = -1531045,
    EMOTE_EXPLODE               = -1531046,

    // Timer_UnCheked spells
    SPELL_POISON_SHOCK          = 25993,
    SPELL_POISONBOLT_VOLLEY     = 25991,
    SPELL_TOXIN                 = 26575,                    // Triggers toxin cloud - 25989

    // Debuffs gained by the boss on frost damage
    SPELL_VISCIDUS_SLOWED       = 26034,
    SPELL_VISCIDUS_SLOWED_MORE  = 26036,
    SPELL_VISCIDUS_FREEZE       = 25937,

    // When frost damage exceeds a certain limit, then boss explodes
    SPELL_REJOIN_VISCIDUS       = 25896,
    SPELL_VISCIDUS_EXPLODE      = 25938,
    SPELL_VISCIDUS_SUICIDE      = 26003,                    // cast when boss explodes and is below 5% Hp - should trigger 26002
    SPELL_DESPAWN_GLOBS         = 26608,

    // SPELL_MEMBRANE_VISCIDUS   = 25994,                   // damage reduction spell - removed from DBC
    // SPELL_VISCIDUS_WEAKNESS   = 25926,                   // aura which procs at damage - should trigger the slow spells - removed from DBC
    // SPELL_VISCIDUS_SHRINKS    = 25893,                   // removed from DBC
    // SPELL_VISCIDUS_SHRINKS_2  = 27934,                   // removed from DBC
    // SPELL_VISCIDUS_GROWS      = 25897,                   // removed from DBC
    // SPELL_SUMMON_GLOBS        = 25885,                   // summons npc 15667 using spells from 25865 to 25884; All spells have target coords - removed from DBC
    // SPELL_VISCIDUS_TELEPORT   = 25904,                   // removed from DBC
    // SPELL_SUMMONT_TRIGGER     = 26564,                   // summons 15992 - removed from DBC

    MORPH_ID_ORIGINAL            = 15686,
    MORPH_ID_INVISIBLE            = 11686,
    NPC_GLOB_OF_VISCIDUS        = 15667,
    NPC_VISCIDUS_TRIGGER        = 15922,                    // handles aura 26575

    MAX_VISCIDUS_GLOBS          = 20,                       // there are 20 summoned globs; each glob = 5% hp

    // hitcounts
    HITCOUNT_SLOW               = 100,
    HITCOUNT_SLOW_MORE          = 150,
    HITCOUNT_FREEZE             = 200,
    HITCOUNT_CRACK              = 50,
    HITCOUNT_SHATTER            = 100,
    HITCOUNT_EXPLODE            = 150,

    // phases
    PHASE_NORMAL                = 1,
    PHASE_FROZEN                = 2,
    PHASE_EXPLODED              = 3,
};

static const uint32 auiGlobSummonSpells[MAX_VISCIDUS_GLOBS] = { 25865, 25866, 25867, 25868, 25869, 25870, 25871, 25872, 25873, 25874, 25875, 25876, 25877, 25878, 25879, 25880, 25881, 25882, 25883, 25884 };

struct boss_viscidusAI : public ScriptedAI
{
    boss_viscidusAI(Creature* pCreature) : ScriptedAI(pCreature), summons(pCreature)
    {
        pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    SummonList summons;

    uint8 Phase;

    uint32 HitCount;
    uint32 ToxinTimer;
    uint32 ExplodeDelayTimer;
    uint32 PoisonShockTimer;
    uint32 PoisonBoltVolleyTimer;

    std::list<uint64> GlobesGuidList;

    bool YelledSlow;
    bool YelledFreeze;

    void Reset()
    {
        Phase                 = PHASE_NORMAL;
        HitCount              = 0;

        ExplodeDelayTimer     = 0;
        ToxinTimer            = 30000;
        PoisonShockTimer      = urand(7000, 12000);
        PoisonBoltVolleyTimer = urand(10000, 15000);

        YelledSlow = false;
        YelledFreeze = false;

        SetCombatMovement(true);
        // me->SetVisibility(VISIBILITY_ON);
        me->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_ID_ORIGINAL);
        me->SetRooted(false);
        me->SetStandState(UNIT_STAND_STATE_STAND);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_NOT_SELECTABLE, false);
        me->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);
        summons.DespawnAll();
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        DoCast(me, SPELL_TOXIN, true);

        DoZoneInCombat();
        if (pInstance)
            pInstance->SetData(DATA_VISCIDUS, IN_PROGRESS);
        
        me->SetPlayerDamageReqInPct(0.05);
    }

    void JustReachedHome()
    {
        if (pInstance)
            pInstance->SetData(DATA_VISCIDUS, FAIL);

        DoCast(me, SPELL_DESPAWN_GLOBS, true);
    }

    void JustDied(Unit* pKiller)
    {
        if (pInstance)
            pInstance->SetData(DATA_VISCIDUS, DONE);
        summons.DespawnAll();
        if(pKiller->GetTypeId() == TYPEID_PLAYER)
            sLog.outLog(LOG_DEFAULT, "AQ40/Viscidus: pKiller is player with guid: %u", pKiller->GetGUID());
        else
        {
            if(((Creature*)pKiller)->isPet())
                sLog.outLog(LOG_DEFAULT, "AQ40/Viscidus: pKiller is pet with guid: %u", pKiller->GetGUID());
            else
                sLog.outLog(LOG_DEFAULT, "AQ40/Viscidus: pKiller is creature with guid: %u", pKiller->GetGUID());
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        summons.Summon(pSummoned);
        if (pSummoned->GetEntry() == NPC_GLOB_OF_VISCIDUS)
        {
            float fX, fY, fZ;
            me->GetRespawnCoord(fX, fY, fZ);
            pSummoned->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
            GlobesGuidList.push_back(pSummoned->GetGUID());
        }
        else if (pSummoned->GetEntry() == NPC_VISCIDUS_TRIGGER)
        {
            pSummoned->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            pSummoned->SetLevel(me->GetLevel());
            pSummoned->setFaction(me->getFaction());
            pSummoned->CastSpell(pSummoned, SPELL_TOXIN, true);
        }
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* pKiller)
    {
        if (pSummoned->GetEntry() == NPC_GLOB_OF_VISCIDUS)
        {
            // shrink - modify scale
            // me->ApplyPercentModFloatValue(OBJECT_FIELD_SCALE_X, float(-4), true);
            // me->UpdateModelData(); // https://github.com/cmangos/mangos-tbc/blob/d3e679c8c988ed1eaa917c43185475a92bbd0547/src/game/Unit.cpp#L9208
            me->SetHealth(me->GetHealth() - (me->GetMaxHealth() * 0.05f));
            GlobesGuidList.remove(pSummoned->GetGUID());

            // suicide if required
            if (me->GetHealthPercent() <= 5.0f)
            {
                // me->SetVisibility(VISIBILITY_ON);
                me->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_ID_ORIGINAL);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_NOT_SELECTABLE, false);
                me->SetRooted(false);
                DoCast(me, SPELL_VISCIDUS_SUICIDE, true);
                me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NONE, NULL, false);
            }
            else if (GlobesGuidList.empty())
            {
                // me->SetVisibility(VISIBILITY_ON);
                me->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_ID_ORIGINAL);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_NOT_SELECTABLE, false);
                me->SetRooted(false);
                Phase = PHASE_NORMAL;

                SetCombatMovement(true);
                DoStartMovement(me->GetVictim());
            }
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiType, uint32 uiPointId)
    {
        if (pSummoned->GetEntry() != NPC_GLOB_OF_VISCIDUS || uiType != POINT_MOTION_TYPE || !uiPointId)
            return;

        if(uiPointId == 1)
        {
            GlobesGuidList.remove(pSummoned->GetGUID());
            pSummoned->CastSpell(me, SPELL_REJOIN_VISCIDUS, true);
            pSummoned->ForcedDespawn(1000);

            if (GlobesGuidList.empty())
            {
                // me->SetVisibility(VISIBILITY_ON);
                me->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_ID_ORIGINAL);
                me->SetRooted(false);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_NOT_SELECTABLE, false);
                Phase = PHASE_NORMAL;
    
                SetCombatMovement(true);
                DoStartMovement(me->GetVictim());
            }
        }
    }

    void DamageTaken(Unit* pDealer, uint32& uiDamage)
    {
        if (pDealer == me)
            return;

        // apply missing aura: 50% damage reduction;
        uiDamage = uiDamage * 0.5f;

        if (Phase != PHASE_FROZEN)
            return;

        ++HitCount;

        // only count melee attacks
        if (pDealer->HasUnitState(UNIT_STAT_MELEE_ATTACKING) && HitCount >= HITCOUNT_EXPLODE)
        {
            if ((me->GetHealthPercent()) <= 5.0f)
            {
                DoCast(me, SPELL_VISCIDUS_SUICIDE, true);
                me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NONE, NULL, false);
            }
            else
            {
                DoCast(me, SPELL_VISCIDUS_EXPLODE, true);
                DoScriptText(EMOTE_EXPLODE, me);
                Phase = PHASE_EXPLODED;
                HitCount = 0;
                GlobesGuidList.clear();
                uint32 uiGlobeCount = (me->GetHealthPercent()) / 5.0f;

                for (uint8 i = 0; i < uiGlobeCount; ++i)
                    DoCast(me, auiGlobSummonSpells[i], true);

                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_FREEZE);
                ExplodeDelayTimer = 2000;

                SetCombatMovement(false);
                me->GetMotionMaster()->MoveIdle();
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_NOT_SELECTABLE);
            }
        }
        else if (HitCount == HITCOUNT_SHATTER)
            DoScriptText(EMOTE_SHATTER, me);
        else if (HitCount == HITCOUNT_CRACK)
            DoScriptText(EMOTE_CRACK, me);
    }

    void SpellHit(Unit* /*pCaster*/, const SpellEntry* pSpell)
    {
        if (Phase != PHASE_NORMAL)
            return;

        // only count frost damage
        if (pSpell->SchoolMask == SPELL_SCHOOL_MASK_FROST)
        {
            ++HitCount;

            if (HitCount >= HITCOUNT_FREEZE)
            {
                Phase = PHASE_FROZEN;
                HitCount = 0;

                DoScriptText(EMOTE_FROZEN, me);
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_SLOWED_MORE);
                DoCast(me, SPELL_VISCIDUS_FREEZE, true);
                YelledSlow = false;
                YelledFreeze = false;
            }
            else if (HitCount == HITCOUNT_SLOW_MORE)
            {
                if(!YelledFreeze)
                {
                    DoScriptText(EMOTE_FREEZE, me);
                    YelledFreeze = true;
                }
                me->RemoveAurasDueToSpell(SPELL_VISCIDUS_SLOWED);
                DoCast(me, SPELL_VISCIDUS_SLOWED_MORE, true);
            }
            else if (HitCount == HITCOUNT_SLOW)
            {
                if(!YelledSlow)
                {
                    DoScriptText(EMOTE_SLOW, me);
                    YelledSlow = true;
                }
                DoCast(me, SPELL_VISCIDUS_SLOWED, true);
            }
        }
    }

    void EventPulse(Unit* pSender, uint32 PulseEventNumber)
    {
        if (Phase == PHASE_EXPLODED)
            return;

        // reset phase if not already exploded
        Phase = PHASE_NORMAL;
        HitCount = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ExplodeDelayTimer)
        {
            if (ExplodeDelayTimer <= diff)
            {
                // Make invisible
                // me->SetVisibility(VISIBILITY_OFF);
                me->SetUInt32Value(UNIT_FIELD_DISPLAYID, MORPH_ID_INVISIBLE);
                me->SetRooted(true);

                // Teleport to room center
                float fX, fY, fZ, fO;
                me->GetRespawnCoord(fX, fY, fZ, &fO);
                me->NearTeleportTo(fX, fY, fZ, fO);
                ExplodeDelayTimer = 0;
            }
            else
                ExplodeDelayTimer -= diff;
        }

        if (Phase != PHASE_NORMAL)
            return;

        if (PoisonShockTimer < diff)
        {
            AddSpellToCast(SPELL_POISON_SHOCK, CAST_NULL);
            PoisonShockTimer = urand(7000, 12000);
        }
        else
            PoisonShockTimer -= diff;

        if (PoisonBoltVolleyTimer < diff)
        {
            AddSpellToCast(SPELL_POISONBOLT_VOLLEY, CAST_NULL);
            PoisonBoltVolleyTimer = urand(10000, 15000);
        }
        else
            PoisonBoltVolleyTimer -= diff;

        if (ToxinTimer < diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
                me->SummonCreature(NPC_VISCIDUS_TRIGGER, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);
            ToxinTimer = 30000;
        }
        else
            ToxinTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_viscidus(Creature* pCreature)
{
    return new boss_viscidusAI(pCreature);
}

bool EffectAuraDummy_spell_aura_dummy_viscidus_freeze(const Aura* pAura, bool bApply)
{
    // On Aura removal inform the boss
    if (pAura->GetId() == SPELL_VISCIDUS_FREEZE && pAura->GetEffIndex() == 1 && !bApply)
    {
        if (Creature* pTarget = (Creature*)pAura->GetTarget())
            pTarget->AI()->EventPulse(pTarget, 1);
    }
    return true;
}

// TODO Remove this 'script' when combat can be proper prevented from core-side
struct npc_glob_of_viscidusAI : public ScriptedAI
{
    npc_glob_of_viscidusAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    uint32 CheckTimer;

    void Reset()
    {
        me->SetSpeed(MOVE_WALK, 0.3f, true);
        me->SetSpeed(MOVE_RUN, 0.3f, true);
        CheckTimer = urand(1000, 3000);
    }

    void AttackStart(Unit* /*pWho*/) { }
    void MoveInLineOfSight(Unit* /*pWho*/) { }
    void UpdateAI(const uint32 diff)
    {
        if(CheckTimer < diff) // Wowwiki: "These Globs will begin to move towards the center of the room, slow at first, but continuously and steadily gathering speed."
        {
            me->SetSpeed(MOVE_WALK, 0.3 + me->GetSpeedRate(MOVE_WALK), true);
            me->SetSpeed(MOVE_RUN, 0.3 + me->GetSpeedRate(MOVE_RUN), true);
            CheckTimer = urand(2500, 4000);
        }
        else CheckTimer -= diff;
    }
};

CreatureAI* GetAI_npc_glob_of_viscidus(Creature* pCreature)
{
    return new npc_glob_of_viscidusAI(pCreature);
}

struct npc_viscidus_triggerAI : public Scripted_NoMovementAI
{
    npc_viscidus_triggerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) { }

    void Reset() { }

    void UpdateAI(const uint32 /*diff*/) {}
};

CreatureAI* GetAI_npc_viscidus_trigger(Creature* pCreature)
{
    return new npc_viscidus_triggerAI(pCreature);
}

void AddSC_boss_viscidus()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_viscidus";
    pNewScript->GetAI = &GetAI_boss_viscidus;
    pNewScript->pEffectAuraDummy = &EffectAuraDummy_spell_aura_dummy_viscidus_freeze;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_glob_of_viscidus";
    pNewScript->GetAI = &GetAI_npc_glob_of_viscidus;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_viscidus_trigger";
    pNewScript->GetAI = &GetAI_npc_viscidus_trigger;
    pNewScript->RegisterSelf();
}