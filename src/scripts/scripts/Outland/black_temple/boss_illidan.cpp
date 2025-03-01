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
SDName: boss_illidan_stormrage
SD%Complete: 90
SDComment: Somewhat of a workaround for Parasitic Shadowfiend, unable to summon GOs for Cage Trap.
SDCategory: Black Temple
EndScriptData */

#include "precompiled.h"
#include "def_black_temple.h"
#include "boss_illidan.h"

struct boss_illidan_stormrageAI : public BossAI
{
    boss_illidan_stormrageAI(Creature* c) : BossAI(c, 1)
    {
        ForceSpellCast(me, SPELL_ILLIDAN_KNEEL_INTRO, INTERRUPT_AND_CAST_INSTANTLY);
    }

    uint8 m_flameCount;
    uint32 m_hoverPoint;

    Timer_UnCheked m_combatTimer;
    Timer_UnCheked m_enrageTimer;

    bool b_maievPhase;
    bool b_maievDone;
    bool maievIsSummonned;

    IllidanPhase m_phase;

    void Reset()
    {
        events.Reset();
        ClearCastQueue();
        summons.DespawnAll();

        m_combatTimer.Reset(1000);
        m_enrageTimer.Reset(25 * 60000 + 34000); // DBM value

        m_hoverPoint = 0;
        m_flameCount = 0;

        b_maievDone = false;
        b_maievPhase = false;
        maievIsSummonned = false;

        m_phase = PHASE_NULL;

        SetWarglaivesEquipped(false);

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetLevitate(false);
        me->setHover(false);
        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
        me->SetSelection(0);

        instance->SetData(EVENT_ILLIDANSTORMRAGE, NOT_STARTED);
    }

    void SetWarglaivesEquipped(bool equip)
    {
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, equip ? 45479 : 0);
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, equip ? 45481 : 0);
        me->SetByteValue(UNIT_FIELD_BYTES_2, 0, equip ? SHEATH_STATE_MELEE : SHEATH_STATE_UNARMED);
    }

    void ChangePhase(IllidanPhase phase)
    {
        if (m_phase == PHASE_DEATH) // do not enter any other phase if we're in phase death
            return;

        StopAutocast();
        ClearCastQueue();
        events.CancelEventsByGCD(m_phase);

        me->SetWalk(false);

        switch (m_phase = phase)
        {
            case PHASE_FIVE:
            {
                events.ScheduleEvent(EVENT_ILLIDAN_SOFT_ENRAGE, 40000, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_CAGE_TRAP, urand(25000, 32000), m_phase);
                me->SetLevitate(false);
                me->setHover(false);
            }
            case PHASE_THREE:
            {
                events.ScheduleEvent(EVENT_ILLIDAN_AGONIZING_FLAMES, urand(28000, 35000), m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_CHANGE_PHASE, m_phase == PHASE_FIVE ? 60000 : urand(40000, 55000), m_phase);
                me->SetLevitate(false);
                me->setHover(false);
                if (Creature *pMaiev = GetClosestCreatureWithEntry(me, 23197, 200.0f))
                    pMaiev->AI()->DoAction(EVENT_MAIEV_RANGE_ATTACK); // SET MELEE ATTACK TYPE
            }
            case PHASE_ONE:
            {
                events.ScheduleEvent(EVENT_ILLIDAN_SHEAR, 10000, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_FLAME_CRASH, urand(25000, 35000), m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_DRAW_SOUL, urand(35000, 45000), m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_PARASITIC_SHADOWFIEND, 30000, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_RANDOM_YELL, urand(32000, 35000), m_phase);
                                
                me->SetLevitate(false);
                me->setHover(false);

                SetWarglaivesEquipped(true);

                DoResetThreat();

                if (m_phase == PHASE_ONE)
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    instance->SetData(EVENT_ILLIDANSTORMRAGE, IN_PROGRESS);
                    me->setActive(true);

                    Map::PlayerList const &plList = me->GetMap()->GetPlayers();
                    for (Map::PlayerList::const_iterator i = plList.begin(); i != plList.end(); ++i)
                    {
                        if (Player* pPlayer = i->getSource())
                        {
                            if (pPlayer->isGameMaster())
                                continue;

                            if (pPlayer->isAlive() && me->IsWithinDistInMap(pPlayer, 150.0f))
                                me->AddThreat(pPlayer, 3000.0f);
                        }
                    }

                    ForceSpellCast(me, SPELL_ILLIDAN_DUAL_WIELD);
                    events.ScheduleEvent(EVENT_ILLIDAN_SUMMON_MINIONS, 1000, m_phase);
                }
                break;
            }
            case PHASE_TWO:
            {
                m_flameCount = 0;

                DoScriptText(YELL_ILLIDAN_PHASE_TWO, me);

                me->RemoveAllAurasNotCreatureAddon();
                me->InterruptNonMeleeSpells(false);

                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);

                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveAllAttackers();

                me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
                me->SetLevitate(true);
                me->setHover(true);

                me->GetMotionMaster()->MovePoint(0, CENTER_X +5.0f, CENTER_Y, CENTER_Z, UNIT_ACTION_CONTROLLED);

                RespawnGlaiveTargets();

                SetAutocast(SPELL_ILLIDAN_FIREBALL, 3000, false, CAST_RANDOM, 0, true);

                events.ScheduleEvent(EVENT_ILLIDAN_THROW_GLAIVE, 15000, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_THROW_GLAIVE, 16000, m_phase);

                events.ScheduleEvent(EVENT_ILLIDAN_SUMMON_TEAR, 17000, m_phase);

                events.ScheduleEvent(EVENT_ILLIDAN_EYE_BLAST, urand(30000, 40000), m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_DARK_BARRAGE, 80000, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_CHANGE_PHASE, 2000, m_phase);
                break;
            }
            case PHASE_FOUR:
            {
                if (Creature *pMaiev = GetClosestCreatureWithEntry(me, 23197, 200.0f))
                    pMaiev->AI()->DoAction(EVENT_MAIEV_RANGE_ATTACK); // SET RANGE ATTACK TYPE

                SetAutocast(SPELL_ILLIDAN_SHADOW_BLAST, 3000, false, CAST_TANK);

                me->SetLevitate(false);
                me->setHover(false); // set both to true after his casting state is fixed in demon form

                events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_NO1, 0, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_FLAME_BURST, 20000, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_SHADOW_DEMON, 30000, m_phase);
                break;
            }
            case PHASE_MAIEV:
            {
                b_maievPhase = false;

                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
                me->SetLevitate(false);
                me->setHover(false);

                events.ScheduleEvent(EVENT_ILLIDAN_INPRISON_RAID, 500, m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_SUMMON_MAIEV, 6000, m_phase);
                break;
            }
            case PHASE_DEATH:
            {
                me->AttackStop();
                me->RemoveAllAuras();
                me->SetLevitate(false);
                me->setHover(false);

                ForceSpellCast(me, SPELL_ILLIDAN_DEATH_OUTRO, INTERRUPT_AND_CAST_INSTANTLY);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);

                if (Creature *pMaiev = GetClosestCreatureWithEntry(me, 23197, 200.0f))
                {
                    pMaiev->AttackStop();
                    pMaiev->AI()->DoAction(EVENT_MAIEV_END_FIGHT_SPEECH); // teleport to Illidan, and speech
                }

                events.ScheduleEvent(EVENT_ILLIDAN_DEATH_SPEECH, 6000, m_phase);
                break;
            }
        }
    }

    void RespawnGlaiveTargets()
    {
        GlaiveTargetRespawner respawner;
        Hellground::ObjectWorker<Creature, GlaiveTargetRespawner> worker(respawner);
        Cell::VisitGridObjects(me, worker, 200.0f);
    }

    void CastEyeBlast()
    {
        Locations initial = EyeBlast[urand(0,4)];
        if (Creature* pTrigger = me->SummonTrigger(initial.x, initial.y, initial.z, 0, 13000))
        {
            RespawnGlaiveTargets();

            if (Creature *pGlaive = GetClosestCreatureWithEntry(pTrigger, GLAIVE_TARGET, 70.0f, true))
            {
                WorldLocation final;
                pTrigger->GetNearPoint(final.coord_x, final.coord_y, final.coord_z, 80.0f, false, pTrigger->GetOrientationTo(pGlaive));
                final.coord_z = 354.519f;
                pTrigger->SetSpeed(MOVE_RUN, 1.0f);
                pTrigger->GetMotionMaster()->MovePoint(0, final.coord_x, final.coord_y, final.coord_z, true, UNIT_ACTION_CONTROLLED);

                pTrigger->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                m_hoverPoint = urand(0,3);
                me->GetMotionMaster()->MovePoint(1, HoverPosition[m_hoverPoint].x, HoverPosition[m_hoverPoint].y, HoverPosition[m_hoverPoint].z);

                ForceSpellCastWithScriptText(pTrigger, SPELL_ILLIDAN_EYE_BLAST, YELL_ILLIDAN_EYE_BLAST, INTERRUPT_AND_CAST_INSTANTLY, false, true);
            }
        }
    }

    bool PhaseOneThreeFive()
    {
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ILLIDAN_SUMMON_MINIONS:
                {
                    if (HealthBelowPct(90.0f))
                    {
                        DoScriptText(YELL_ILLIDAN_SUMMON_MINIONS, me);

                        // Call back Akama to deal with minions
                        if (Creature *pAkama = instance->GetCreature(instance->GetData64(DATA_AKAMA)))
                        {
                            pAkama->AttackStop();
                            pAkama->DeleteThreatList();
                            pAkama->SetReactState(REACT_PASSIVE);

                            DoModifyThreatPercent(pAkama, -101);

                            pAkama->GetMotionMaster()->MoveIdle();
                            pAkama->AI()->DoAction(EVENT_AKAMA_MINIONS_FIGHT); // EVENT_AKAMA_MINIONS_FIGHT
                        }
                    }
                    else
                        events.ScheduleEvent(EVENT_ILLIDAN_SUMMON_MINIONS, 1000, PHASE_ONE);

                    break;
                }
                case EVENT_ILLIDAN_CHANGE_PHASE:
                {
                    ChangePhase(PHASE_FOUR);
                    return false;
                }
                case EVENT_ILLIDAN_RANDOM_YELL:
                {
                    DoScriptText(RAND(YELL_ILLIDAN_TAUNT_NO1, YELL_ILLIDAN_TAUNT_NO2, YELL_ILLIDAN_TAUNT_NO3, YELL_ILLIDAN_TAUNT_NO4), me);
                    events.ScheduleEvent(EVENT_ILLIDAN_RANDOM_YELL, urand(30000, 35000));
                    break;
                }
                case EVENT_ILLIDAN_SHEAR:
                {
                    AddSpellToCast(me->GetVictim(), SPELL_ILLIDAN_SHEAR);
                    events.ScheduleEvent(EVENT_ILLIDAN_SHEAR, 10000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_FLAME_CRASH:
                {
                    AddSpellToCast(me->GetVictim(), SPELL_ILLIDAN_FLAME_CRASH);
                    events.ScheduleEvent(EVENT_ILLIDAN_FLAME_CRASH, urand(25000, 30000), m_phase);
                    break;
                }
                case EVENT_ILLIDAN_DRAW_SOUL:
                {
                    AddSpellToCast(me, SPELL_ILLIDAN_DRAW_SOUL);
                    events.ScheduleEvent(EVENT_ILLIDAN_DRAW_SOUL, urand(35000, 45000), m_phase);
                    break;
                }
                case EVENT_ILLIDAN_PARASITIC_SHADOWFIEND:
                {
                    if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 250.0f, true, me->getVictimGUID()))
                        AddSpellToCast(pTarget, SPELL_ILLIDAN_PARASITIC_SHADOWFIEND, false, true);

                    events.ScheduleEvent(EVENT_ILLIDAN_PARASITIC_SHADOWFIEND, 30000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_AGONIZING_FLAMES:
                {
                    if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 250.0f, true, 0, 15.0f))
                        AddSpellToCast(pTarget, SPELL_ILLIDAN_AGONIZING_FLAMES, false, true);

                    events.ScheduleEvent(EVENT_ILLIDAN_AGONIZING_FLAMES, urand(30000, 35000), m_phase);
                    break;
                }
                case EVENT_ILLIDAN_SOFT_ENRAGE:
                {
                    ForceSpellCast(me, SPELL_ILLIDAN_ENRAGE);
                    break;
                }
                case EVENT_ILLIDAN_CAGE_TRAP:
                {
                    if (Creature *pMaiev = GetClosestCreatureWithEntry(me, 23197, 200.0f))
                        pMaiev->AI()->DoAction(EVENT_MAIEV_CAGE_TRAP); // Force to place trap

                    events.ScheduleEvent(EVENT_ILLIDAN_CAGE_TRAP, urand(20000, 30000), m_phase);
                    break;
                }
            }
        }
        return true;
    }

    bool PhaseTwo()
    {
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ILLIDAN_CHANGE_PHASE:
                {
                    if (m_flameCount >= 2)
                    {
                        ClearCastQueue();
                        StopAutocast();
                        me->GetMotionMaster()->MovePoint(0, CENTER_X, CENTER_Y, CENTER_Z);
                        events.ScheduleEvent(EVENT_ILLIDAN_RETURN_GLAIVE, 3000, m_phase);
                        break;
                    }

                    events.ScheduleEvent(EVENT_ILLIDAN_CHANGE_PHASE, 2000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_LAND:
                {
                    me->SetLevitate(false);
                    me->setHover(false);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    me->SetReactState(REACT_AGGRESSIVE);
                    ChangePhase(PHASE_THREE);
                    break;
                }
                case EVENT_ILLIDAN_RETURN_GLAIVE:
                {
                    me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                    summons.Cast(BLADE_OF_AZZINOTH, SPELL_ILLIDAN_GLAIVE_RETURN, me);
                    summons.DespawnEntry(BLADE_OF_AZZINOTH);
                    events.ScheduleEvent(EVENT_ILLIDAN_LAND, 1000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_THROW_GLAIVE:
                {
                    AddSpellToCast(SPELL_ILLIDAN_THROW_GLAIVE, CAST_NULL);

                    SetWarglaivesEquipped(false);
                    break;
                }
                case EVENT_ILLIDAN_SUMMON_TEAR:
                {
                    StartAutocast();
                    break;
                }
                case EVENT_ILLIDAN_EYE_BLAST:
                {
                    CastEyeBlast();
                    events.ScheduleEvent(EVENT_ILLIDAN_EYE_BLAST, urand(30000, 35000), m_phase);
                    break;
                }
                case EVENT_ILLIDAN_DARK_BARRAGE:
                {
                    if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                        AddSpellToCast(pTarget, SPELL_ILLIDAN_DARK_BARRAGE);

                    events.ScheduleEvent(EVENT_ILLIDAN_DARK_BARRAGE, 44000, m_phase);
                    break;
                }
            }
        }
        return false;
    }

    bool PhaseFour()
    {
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ILLIDAN_TRANSFORM_NO1:
                {
                    me->GetMotionMaster()->Clear(false);

                    DoScriptText(YELL_ILLIDAN_DEMON_FORM, me);

                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_TRANSFORM_1, INTERRUPT_AND_CAST_INSTANTLY);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_NO2, 1200, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_NO2:
                {
                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_DEMON_TRANSFORM_1);
                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_TRANSFORM_2, INTERRUPT_AND_CAST_INSTANTLY);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_NO3, 3900, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_NO3:
                {
                    DoResetThreat();
                    SetWarglaivesEquipped(false);

                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_DEMON_TRANSFORM_2);
                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_FORM, INTERRUPT_AND_CAST_INSTANTLY, true);
                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_TRANSFORM_2, INTERRUPT_AND_CAST_INSTANTLY);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_NO4, 2900, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_NO4:
                {
                    StartAutocast();

                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_DEMON_TRANSFORM_2);
                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_TRANSFORM_3, INTERRUPT_AND_CAST_INSTANTLY);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_NO5, 3400, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_NO5:
                {
                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_DEMON_TRANSFORM_3);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_BACKNO1, 61000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_FLAME_BURST:
                {
                    AddSpellToCast(me, SPELL_ILLIDAN_FLAME_BURST);
                    events.ScheduleEvent(EVENT_ILLIDAN_FLAME_BURST, 20000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_SHADOW_DEMON:
                {
                    ForceSpellCast(me, SPELL_ILLIDAN_SHADOW_DEMON_CAST, INTERRUPT_AND_CAST_INSTANTLY);

                    float px, py, pz;
                    for (uint8 i = 0; i < 4; i++)
                    {
                        m_creature->GetNearPoint(px, py, pz, 10.0f);
                        m_creature->CastSpell(px, py, pz, SPELL_ILLIDAN_SHADOW_DEMON, true);
                    }
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_BACKNO1:
                {
                    ClearCastQueue();
                    StopAutocast();
                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_TRANSFORM_1, INTERRUPT_AND_CAST_INSTANTLY);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_BACKNO2, 1500, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_BACKNO2:
                {
                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_DEMON_TRANSFORM_1);
                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_TRANSFORM_2, INTERRUPT_AND_CAST_INSTANTLY);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_BACKNO3, 4000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_BACKNO3:
                {
                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_DEMON_FORM);

                    SetWarglaivesEquipped(true);
                    events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_BACKNO4, 3000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_TRANSFORM_BACKNO4:
                {
                    me->RemoveAurasDueToSpell(SPELL_ILLIDAN_DEMON_TRANSFORM_2);
                    ForceSpellCast(me, SPELL_ILLIDAN_DEMON_TRANSFORM_3, INTERRUPT_AND_CAST_INSTANTLY);
                    events.ScheduleEvent(EVENT_ILLIDAN_CHANGE_PHASE, 3500, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_CHANGE_PHASE:
                {
                    if (b_maievPhase)
                    {
                        ChangePhase(PHASE_MAIEV);
                        break;
                    }

                    if (HealthBelowPct(30.0f))
                        ChangePhase(PHASE_FIVE);
                    else
                        ChangePhase(PHASE_THREE);

                    me->GetMotionMaster()->MoveChase(me->GetVictim());
                    break;
                }
            }
        }
        return false;
    }

    bool PhaseMaiev()
    {
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ILLIDAN_INPRISON_RAID:
                {
                    ForceSpellCast(SPELL_ILLIDAN_INPRISON_RAID, CAST_SELF, INTERRUPT_AND_CAST_INSTANTLY, true);
                    // ForceSpellCastWithScriptText(SPELL_ILLIDAN_INPRISON_RAID, CAST_SELF, YELL_ILLIDAN_INPRISON_RAID, INTERRUPT_AND_CAST_INSTANTLY, true);
                    DoScriptText(YELL_ILLIDAN_INPRISON_RAID, me);
                    break;
                }
                case EVENT_ILLIDAN_SUMMON_MAIEV:
                {
                    ForceSpellCast(me, SPELL_ILLIDAN_SUMMON_MAIEV);
                    maievIsSummonned = true;
                    break;
                }
            }
        }
        return false;
    }

    bool PhaseDeath()
    {
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ILLIDAN_DEATH_SPEECH:
                {
                    DoScriptText(YELL_ILLIDAN_DEATH_SPEECH, me);
                    events.ScheduleEvent(EVENT_ILLIDAN_KILL, 20000, m_phase);
                    break;
                }
                case EVENT_ILLIDAN_KILL:
                {
                    if (Creature *pAkama = instance->GetCreature(instance->GetData64(DATA_AKAMA)))
                        pAkama->AI()->DoAction(EVENT_AKAMA_RETURN_ILLIDAN);

                    me->Kill(me, false);
                    instance->SetData(EVENT_ILLIDANSTORMRAGE, DONE);
                    break;
                }
            }
        }
        return false;
    }

    bool HandlePhase(IllidanPhase m_phase)
    {
        switch(m_phase)
        {
            case PHASE_ONE:
            case PHASE_THREE:
            case PHASE_FIVE:
                return PhaseOneThreeFive();
            case PHASE_TWO:
                return PhaseTwo();
            case PHASE_FOUR:
                return PhaseFour();
            case PHASE_MAIEV:
                return PhaseMaiev();
            case PHASE_DEATH:
                return PhaseDeath();
            default:
                return true;
        }
    }

    void JustDied(Unit* killer)
    {
        summons.DespawnAllExcept(23197);
    }

    void KilledUnit(Unit *pWho)
    {
        if (pWho == me)
            return;

        DoScriptText(RAND(YELL_ILLIDAN_SLAIN, YELL_ILLIDAN_SLAIN2), me);
    }

    void AttackStart(Unit *pWho)
    {
        if (m_phase == PHASE_TWO || m_phase == PHASE_NULL)
            return;

        if (m_phase == PHASE_FOUR)
            ScriptedAI::AttackStartNoMove(pWho);
        else
            ScriptedAI::AttackStart(pWho);
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (m_phase == PHASE_TWO || m_phase == PHASE_NULL)
            return;

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (damage > me->GetHealth() && done_by != me)
        {
            damage = 0;
            if (m_phase != PHASE_DEATH && maievIsSummonned)
                ChangePhase(PHASE_DEATH);
        }
    }

    void MovementInform(uint32 MovementType, uint32 uiData)
    {
    }

    void EnterEvadeMode()
    {
        me->setActive(false);
        me->SetLevitate(false);
        me->setHover(false);
        summons.DespawnAll();
        events.Reset();

        me->SetReactState(REACT_PASSIVE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        instance->SetData(EVENT_ILLIDANSTORMRAGE, NOT_STARTED);

        if (Creature *pAkama = instance->GetCreature(instance->GetData64(DATA_AKAMA)))
        {
            pAkama->AI()->EnterEvadeMode();
            pAkama->GetMotionMaster()->Clear(false, true); // need reset waypoint movegen, to test
            pAkama->AI()->Reset();
            pAkama->GetMotionMaster()->MovePath(2109, false); // PATH_AKAMA_DOOR_EVERNT_AFTER
        }

        BossAI::EnterEvadeMode();
    }

    void JustReachedHome()
    {
        ForceSpellCast(me, SPELL_ILLIDAN_KNEEL_INTRO, INTERRUPT_AND_CAST_INSTANTLY);
    }

    void JustRespawned()
    {
        ForceSpellCast(me, SPELL_ILLIDAN_KNEEL_INTRO, INTERRUPT_AND_CAST_INSTANTLY);
    }

    void OnAuraApply(Aura *aura, Unit *, bool stackApply)
    {
        if (aura->GetId() == SPELL_CAGED)
            events.RescheduleEvent(EVENT_ILLIDAN_CHANGE_PHASE, 15000, m_phase);
    }

    void DoAction(const int32 action)
    {
        switch (action)
        {
            case EVENT_ILLIDAN_START:
            {
                DoScriptText(YELL_ILLIDAN_AGGRO, me);
                me->SetReactState(REACT_AGGRESSIVE);

                me->RemoveAurasDueToSpell(SPELL_ILLIDAN_KNEEL_INTRO);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

                ChangePhase(PHASE_ONE);
                break;
            }
            case EVENT_ILLIDAN_CHANGE_PHASE:
            {
                ChangePhase(PHASE_FIVE);
                break;
            }
            case EVENT_ILLIDAN_FLAME_DEATH:
            {
                m_flameCount++;
                break;
            }
        }
    }

    bool UpdateVictim()
    {
        switch (m_phase)
        {
            case PHASE_TWO:
            case PHASE_MAIEV:
            case PHASE_DEATH:
            {
                if (me->GetMap()->GetAlivePlayersCountExceptGMs() == 0)
                {
                    EnterEvadeMode();
                    return false;
                }
                return true;
            }
            default:
                return ScriptedAI::UpdateVictim();
        }
    }
   
    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        if (me->IsInCombat())
        {
            if (HandlePhase(m_phase))
                DoMeleeAttackIfReady();

            CastNextSpellIfAnyAndReady(diff);
        }

        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 200.0f, 2.5f);

        if (m_combatTimer.Expired(diff)) // zero aggro every second
        {
            if (Creature *pAkama = instance->GetCreature(instance->GetData64(DATA_AKAMA)))
                DoModifyThreatPercent(pAkama, -101);
            if (Creature *pMaiev = GetClosestCreatureWithEntry(me, 23197, 200.0f))
                DoModifyThreatPercent(pMaiev, -101);
            m_combatTimer = 1000;
        }

        if (m_enrageTimer.Expired(diff))
        {
            ForceSpellCastWithScriptText(me, SPELL_ILLIDAN_HARD_ENRAGE, YELL_ILLIDAN_HARD_ENRAGE, INTERRUPT_AND_CAST_INSTANTLY);
            m_enrageTimer = 25000;
        }
        

        if (m_phase == PHASE_ONE && HealthBelowPct(65.0f))
        {
            ChangePhase(PHASE_TWO);
            return;
        }

        if (!b_maievDone && HealthBelowPct(30.0f))
        {
            if (m_phase == PHASE_FOUR)
            {
                me->InterruptNonMeleeSpells(false);

                events.CancelEventsByGCD(m_phase);
                events.ScheduleEvent(EVENT_ILLIDAN_TRANSFORM_BACKNO1, 0, m_phase);
                b_maievPhase = true;
            }
            else
                ChangePhase(PHASE_MAIEV);

            b_maievDone = true;
        }
    }
};

struct boss_illidan_akamaAI : public BossAI
{
    boss_illidan_akamaAI(Creature* c) : BossAI(c, 2){}

    bool allowUpdate;
    bool doorEvent;

    uint32 m_pathId;

    void Reset()
    {
        ClearCastQueue();
        events.Reset();
        summons.DespawnAll();

        allowUpdate = false;
        doorEvent = false;

        m_pathId = 0;

        me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_UNARMED);

        SetAutocast(SPELL_AKAMA_CHAIN_LIGHTNING, 10000, false, CAST_TANK);

        me->SetUInt32Value(UNIT_NPC_FLAGS, 0);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        if (instance->GetData(EVENT_ILLIDARIDOOR) == DONE)
        {
            me->SetVisibility(VISIBILITY_ON);
            me->SetHomePosition(746.506f, 232.543f, 352.996f, 1.85f); //normal pos before illidan fight
        }
        else
        {
            me->SetVisibility(VISIBILITY_OFF);
            me->DestroyForNearbyPlayers();
            float x, y, z;
            me->GetRespawnCoord(x, y, z);
            me->SetHomePosition(x, y, z, 2.53f);
        }

        me->SetReactState(REACT_PASSIVE);


    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (!me->GetDBTableGUIDLow())
            return;

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void MovementInform(uint32 type, uint32 data)
    {
        if (type == WAYPOINT_MOTION_TYPE)
        {
            if (doorEvent)
            {
                if (data == 15 && m_pathId == PATH_AKAMA_DOOR_EVENT_BEFORE)
                {
                    m_pathId = 0;
                    events.ScheduleEvent(EVENT_AKAMA_DOOR_CAST_FAIL, 2000);
                }

                if (data == 17 && m_pathId == PATH_AKAMA_DOOR_EVENT_AFTER)
                {
                    if (Creature *pAkama = instance->GetCreature(instance->GetData64(DATA_AKAMA)))
                        pAkama->SetVisibility(VISIBILITY_ON);

                    m_pathId = 0;
                    me->DisappearAndDie();
                }
            }
            else
            {
                if (m_pathId == PATH_AKAMA_MINION_EVENT)
                {
                    if (data == 0)
                        DoScriptText(YELL_AKAMA_FIGHT_MINIONS, me);

                    if (data == 9)
                    {
                        m_pathId = 0;
                        events.ScheduleEvent(EVENT_AKAMA_SUMMON_ELITE, 1000);
                    }
                }
                else
                    if (data == 17)
                        me->Respawn();
            }
        }

        if (type == POINT_MOTION_TYPE)
        {
            if (data == 0)
            {
                me->SetSelection(instance->GetData64(DATA_ILLIDANSTORMRAGE));
                events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO1, 500);
            }
        }
    }

    void HandleDoorEvent()
    {
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_AKAMA_DOOR_CAST_FAIL:
                {
                    AddSpellToCastWithScriptText(SPELL_AKAMA_DOOR_CAST_FAIL, CAST_NULL, SAY_AKAMA_DOOR_SPEECH_NO1);
                    events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO1, 8500);
                    break;
                }
                case EVENT_AKAMA_TALK_SEQUENCE_NO1:
                {
                    DoScriptText(SAY_AKAMA_DOOR_SPEECH_NO2, me);
                    events.ScheduleEvent(EVENT_AKAMA_SUMMON_SPIRITS, 10000);
                    break;
                }
                case EVENT_AKAMA_SUMMON_SPIRITS:
                {
                    for (uint8 i = 0; i < 2; i++)
                    {
                        if (Creature *pSpirit = me->SummonCreature(uint32(SpiritSpawns[i][0]), SpiritSpawns[i][1], SpiritSpawns[i][2], SpiritSpawns[i][3], me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 35000))
                            pSpirit->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    }

                    events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO2, 1000);
                    break;
                }
                case EVENT_AKAMA_TALK_SEQUENCE_NO2:
                {
                    if (Creature *pCreature = GetClosestCreatureWithEntry(me, SpiritSpawns[0][0], 30.0f))
                        DoScriptText(SAY_SPIRIT_DOOR_SPEECH_NO1, pCreature);

                    events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO3, 3000);
                    break;
                }
                case EVENT_AKAMA_TALK_SEQUENCE_NO3:
                {
                    if (Creature *pCreature = GetClosestCreatureWithEntry(me, SpiritSpawns[1][0], 30.0f))
                        DoScriptText(SAY_SPIRIT_DOOR_SPEECH_NO2, pCreature);

                    events.ScheduleEvent(EVENT_AKAMA_DOOR_CAST_SUCCESS, 8000);
                    break;
                }
                case EVENT_AKAMA_DOOR_CAST_SUCCESS:
                {
                    for (uint8 i = 0; i < 2; i++)
                    {
                        if (Creature *pSpirit = GetClosestCreatureWithEntry(me, uint32(SpiritSpawns[i][0]), 30.0f))
                            pSpirit->CastSpell((Unit*)NULL, SPELL_DEATHSWORN_DOOR_CHANNEL, false);
                    }

                    AddSpellToCast(SPELL_AKAMA_DOOR_CAST_SUCCESS, CAST_NULL);
                    events.ScheduleEvent(EVENT_AKAMA_DOOR_OPEN, 15000);
                    break;
                }
                case EVENT_AKAMA_DOOR_OPEN:
                {
                    instance->SetData(EVENT_ILLIDARIDOOR, DONE);
                    instance->HandleGameObject(instance->GetData64(DATA_GAMEOBJECT_ILLIDAN_GATE), true);
                    events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO4, 3000);
                    break;
                }
                case EVENT_AKAMA_TALK_SEQUENCE_NO4:
                {
                    DoScriptText(SAY_AKAMA_DOOR_SPEECH_NO3, me);
                    events.ScheduleEvent(EVENT_AKAMA_DOOR_MOVE_PATH, 10000);
                    break;
                }
                case EVENT_AKAMA_DOOR_MOVE_PATH:
                {
                    m_pathId = PATH_AKAMA_DOOR_EVENT_AFTER;
                    me->GetMotionMaster()->MovePath(PATH_AKAMA_DOOR_EVENT_AFTER, false);
                    break;
                }
            }
        }
        CastNextSpellIfAnyAndReady();
    }

    void EnterEvadeMode()
    {
        ClearCastQueue();
        events.Reset();
        summons.DespawnAll();

        me->InterruptNonMeleeSpells(true);
        me->RemoveAllAurasNotCreatureAddon();
        me->DeleteThreatList();
        me->CombatStop(true);
    }

    void KilledUnit(Unit *pWho)
    {
        if (summons.empty())
            return;

        if (Unit *pUnit = GetClosestCreatureWithEntry(me, 23226, 40.0f))
            AttackStartNoMove(pUnit);
    }

    void DoAction(const int32 action)
    {
        switch (action)
        {
            case EVENT_AKAMA_RETURN_ILLIDAN:
            {
                events.CancelEvent(EVENT_AKAMA_SUMMON_ELITE);
                summons.DespawnAll();

                EnterEvadeMode();

                if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                {
                    float x,y,z;
                    pIllidan->GetNearPoint(x, y, z, 0.0f, 8.0f, -pIllidan->GetOrientationTo(CENTER_X, CENTER_Y));

                    me->Relocate(x, y, z);
                    me->SendHeartBeat();
                    me->StopMoving();
                }
                break;
            }
            case EVENT_AKAMA_SET_DOOR_EVENT:
            {
                doorEvent = true;
                me->SetVisibility(VISIBILITY_ON);
                break;
            }
            case EVENT_AKAMA_START:
            {
                Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE));
                if (!pIllidan || !pIllidan->isAlive())
                    return;

                allowUpdate = true;

                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

                if (doorEvent)
                {
                    m_pathId = PATH_AKAMA_DOOR_EVENT_BEFORE;
                    me->GetMotionMaster()->MovePath(PATH_AKAMA_DOOR_EVENT_BEFORE, false);
                }
                else
                {
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(0, 744.116f, 304.922f, 352.996f);
                }
                break;
            }
            case EVENT_AKAMA_MINIONS_FIGHT:
            {
                StopAutocast();
                me->InterruptNonMeleeSpells(true);
                me->ClearInCombat();

                m_pathId = PATH_AKAMA_MINION_EVENT;

                me->GetMotionMaster()->MovePath(PATH_AKAMA_MINION_EVENT, false);
                break;
            }
            case EVENT_AKAMA_END_ILLIDAN:
            {
                DoScriptText(SAY_AKAMA_ILLIDAN_END, me);
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
            if (!allowUpdate && !UpdateVictim())
                return;

        events.Update(diff);

        if (doorEvent)
        {
            HandleDoorEvent();
            return;
        }

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_AKAMA_TALK_SEQUENCE_NO1:
                {
                    if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                    {
                        me->SetSelection(pIllidan->GetGUID());
                        pIllidan->SetSelection(me->GetGUID());
                        pIllidan->RemoveAurasDueToSpell(SPELL_ILLIDAN_KNEEL_INTRO);
                        me->SetFacingToObject(pIllidan);
                        DoScriptText(SAY_ILLIDAN_NO1, pIllidan);
                    }

                    events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO2, 12000);
                    return;
                }
                case EVENT_AKAMA_TALK_SEQUENCE_NO2:
                {
                    DoScriptText(SAY_AKAMA_NO1, me);
                    events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO3, 10000);
                    return;
                }
                case EVENT_AKAMA_TALK_SEQUENCE_NO3:
                {
                    if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                        DoScriptText(SAY_ILLIDAN_NO2, pIllidan);

                    events.ScheduleEvent(EVENT_AKAMA_TALK_SEQUENCE_NO4, 7000);
                    return;
                }
                case EVENT_AKAMA_TALK_SEQUENCE_NO4:
                {
                    DoScriptText(SAY_AKAMA_NO2, me);
                    events.ScheduleEvent(EVENT_AKAMA_ILLIDAN_FIGHT, 4500);
                    me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                    return;
                }
                case EVENT_AKAMA_ILLIDAN_FIGHT:
                {
                    if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                    {
                        pIllidan->AI()->DoAction(EVENT_ILLIDAN_START);
                        AttackStart(pIllidan);

                        StartAutocast();
                    }
                    break;
                }
                case EVENT_AKAMA_SUMMON_ELITE:
                {
                    if (Creature *pElite = DoSummon(23226, me, 5.0f, 10000.0f, TEMPSUMMON_DEAD_DESPAWN))
                    {
                        pElite->AddThreat(me, 10000.0f);
                        pElite->AI()->AttackStart(me);

                        if (!me->GetVictim())
                            AttackStartNoMove(pElite);

                        StartAutocast();

                        events.ScheduleEvent(EVENT_AKAMA_SUMMON_ELITE, 9000);
                    }
                    break;
                }
            }
        }

        if (HealthBelowPct(20.0f))
        {
            ForceSpellCast(me, SPELL_AKAMA_POTION, INTERRUPT_AND_CAST_INSTANTLY);
            return;
        }

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

struct boss_illidan_maievAI : public BossAI
{
    boss_illidan_maievAI(Creature *c) : BossAI(c, 3){};

    bool m_canMelee;

    void Reset()
    {
        m_canMelee = false;
    }

    void IsSummonedBy(Unit *pSummoner)
    {
        me->Relocate(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() +1.0f);
        me->SendHeartBeat();

        ForceSpellCast(me, SPELL_MAIEV_TELEPORT_VISUAL);

        me->StopMoving();

        if (pSummoner->GetTypeId() == TYPEID_UNIT)
            ((Creature*)pSummoner)->SetSelection(me->GetGUID());

        me->SetReactState(REACT_PASSIVE);
        me->SetSelection(pSummoner->GetGUID());
        events.ScheduleEvent(EVENT_MAIEV_TALK_SEQUENCE_NO1, 6000);
    }

    void DoAction(const int32 action)
    {
        switch (action)
        {
            case EVENT_MAIEV_RANGE_ATTACK:
            {
                if (m_canMelee)
                {
                    m_canMelee = false;
                    me->GetMotionMaster()->Clear(false);

                    if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                    {
                        float x, y, z;
                        pIllidan->GetNearPoint(x, y, z, 0.0f, 45.0f, -pIllidan->GetOrientationTo(CENTER_X, CENTER_Y));
                        z = 354.519;

                        me->Relocate(x, y, z);
                        me->UpdateObjectVisibility();

                        ForceSpellCast(me, SPELL_MAIEV_TELEPORT_VISUAL, INTERRUPT_AND_CAST_INSTANTLY);
                        me->StopMoving();

                        me->SetSelection(pIllidan->GetGUID());
                    }

                    SetAutocast(SPELL_MAIEV_THROW_DAGGER, 2000, false, CAST_TANK);
                    StartAutocast();
                }
                else
                {
                    m_canMelee = true;
                    StopAutocast();

                    if (me->GetVictim())
                        DoStartMovement(me->GetVictim());
                }
                break;
            }
            case EVENT_MAIEV_CAGE_TRAP:
            {
                me->GetMotionMaster()->Clear(false);

                float x, y, z;
                me->GetNearPoint(x, y, z, 0.0f, 25.0f, frand(0, 2*M_PI));
                z = 354.519;

                me->Relocate(x, y, z);
                me->SendHeartBeat();

                ForceSpellCast(me, SPELL_MAIEV_TELEPORT_VISUAL, INTERRUPT_AND_CAST_INSTANTLY);
                ForceSpellCast(me, SPELL_MAIEV_SUMMON_CAGE_TRAP, INTERRUPT_AND_CAST_INSTANTLY, true);

                if (me->GetVictim())
                    DoStartMovement(me->GetVictim());

                break;
            }
            case EVENT_MAIEV_END_FIGHT_SPEECH:
            {
                me->GetMotionMaster()->Clear(false);

                if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                {
                    float x, y, z;
                    pIllidan->GetNearPoint(x, y, z, 0.0f, 7.0f, pIllidan->GetOrientation());
                    z = 354.519;

                    me->Relocate(x, y, z);
                    me->SendHeartBeat();

                    me->SetSelection(pIllidan->GetGUID());
                }
                ForceSpellCast(me, SPELL_MAIEV_TELEPORT_VISUAL, INTERRUPT_AND_CAST_INSTANTLY);
                events.CancelEvent(EVENT_MAIEV_RANDOM_TAUNT);
                events.ScheduleEvent(EVENT_MAIEV_END_SPEECH_2, 24000);

                DoScriptText(YELL_MAIEV_ILLIDAN_END, me);
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MAIEV_TALK_SEQUENCE_NO1:
                {
                    DoScriptText(YELL_MAIEV_TALK_SEQUENCE_NO1, me);
                    events.ScheduleEvent(EVENT_MAIEV_TALK_SEQUENCE_NO2, 9000);
                    break;
                }
                case EVENT_MAIEV_TALK_SEQUENCE_NO2:
                {
                    if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                        DoScriptText(YELL_ILLIDAN_TALK_SQUENCE_NO1, pIllidan);

                    events.ScheduleEvent(EVENT_MAIEV_TALK_SEQUENCE_NO3, 6000);
                    break;
                }
                case EVENT_MAIEV_TALK_SEQUENCE_NO3:
                {
                    DoScriptText(YELL_MAIEV_TALK_SEQUENCE_NO2, me);
                    events.ScheduleEvent(EVENT_MAIEV_BEGIN_FIGHT, 3000);
                    break;
                }
                case EVENT_MAIEV_BEGIN_FIGHT:
                {
                    if (Creature *pIllidan = instance->GetCreature(instance->GetData64(DATA_ILLIDANSTORMRAGE)))
                    {
                        pIllidan->SetReactState(REACT_AGGRESSIVE);
                        pIllidan->AI()->DoAction(EVENT_ILLIDAN_CHANGE_PHASE);
                        pIllidan->AI()->AttackStart(me);
                        me->SetSelection(pIllidan->GetGUID());
                        AttackStart(pIllidan);
                    }
                    events.ScheduleEvent(EVENT_MAIEV_RANDOM_TAUNT, urand(30000, 40000));
                    break;
                }
                case EVENT_MAIEV_RANDOM_TAUNT:
                {
                    DoScriptText(RAND(MAIEV_TAUNT_NO1, MAIEV_TAUNT_NO2, MAIEV_TAUNT_NO3, MAIEV_TAUNT_NO4), me);
                    events.ScheduleEvent(EVENT_MAIEV_RANDOM_TAUNT, urand(30000, 40000));
                    break;
                }
                case EVENT_MAIEV_END_SPEECH_2:
                {
                    DoScriptText(YELL_MAIEV_ILLIDAN_END_2, me);
                    events.ScheduleEvent(EVENT_MAIEV_END_SPEECH_3, 10000);
                    break;
                }
                case EVENT_MAIEV_END_SPEECH_3:
                {
                    events.ScheduleEvent(EVENT_MAIEV_DESPAWN, 3000);
                    DoScriptText(YELL_MAIEV_ILLIDAN_END_3, me);
                    break;
                }
                case EVENT_MAIEV_DESPAWN:
                {
                    ForceSpellCast(me, SPELL_MAIEV_TELEPORT_VISUAL, INTERRUPT_AND_CAST_INSTANTLY);
                    me->DisappearAndDie();
                    if (Creature *pAkama = instance->GetCreature(instance->GetData64(DATA_AKAMA)))
                        pAkama->AI()->DoAction(EVENT_AKAMA_END_ILLIDAN);
                }
            }
        }

        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady(diff);

        if (m_canMelee)
            DoMeleeAttackIfReady();
    }
};

struct boss_illidan_glaiveAI : public Scripted_NoMovementAI
{
    boss_illidan_glaiveAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    Timer_UnCheked m_summonTimer;

    uint64 m_tearGUID;

    void MoveInLineOfSight(Unit *pWho){}

    void IsSummonedBy(Unit *pSummoner)
    {
        m_tearGUID = 0;

        m_summonTimer = 2000;

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

        if (!pInstance)
            return;

        if (Creature *pIllidan = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDANSTORMRAGE)))
            pIllidan->AI()->JustSummoned(me);
    }

    void JustSummoned(Creature *pWho)
    {
        m_tearGUID = pWho->GetGUID();
        ForceSpellCast(pWho, SPELL_GLAIVE_CHANNEL);

        if (Creature *pIllidan = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDANSTORMRAGE)))
            pIllidan->AI()->JustSummoned(pWho);
    }

    void UpdateAI(const uint32 diff)
    {

        if (m_summonTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_GLAIVE_SUMMON_TEAR);
            m_summonTimer = 0;
        }


        CastNextSpellIfAnyAndReady();
    }
};

struct boss_illidan_flameofazzinothAI : public ScriptedAI
{
    boss_illidan_flameofazzinothAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        pInstance = c->GetInstanceData();

        me->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 8.0f);
        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 8.0f);
        me->SetReactState(REACT_PASSIVE);
    }

    ScriptedInstance *pInstance;

    EventMap events;
    SummonList summons;
    Timer_UnCheked check_timer;

    uint64 m_owner;

    void Reset()
    {
        events.Reset();
        summons.DespawnAll();
        ClearCastQueue();

        m_owner = 0;
        check_timer.Reset(2000);

        m_creature->SetMeleeDamageSchool(SPELL_SCHOOL_FIRE);
        m_creature->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
    }

    void EnterCombat(Unit *pWho)
    {
        events.ScheduleEvent(EVENT_FLAME_RANGE_CHECK, 2000);
        events.ScheduleEvent(EVENT_FLAME_FLAME_BLAST, urand(25000, 30000));
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void JustDied(Unit *pKiller)
    {
        if (Creature *pIllidan = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDANSTORMRAGE)))
            pIllidan->AI()->DoAction(EVENT_ILLIDAN_FLAME_DEATH);

        me->RemoveCorpse();
    }

    void JustSummoned(Creature *pWho)
    {
        summons.Summon(pWho);
    }

    void IsSummonedBy(Unit *pWho)
    {
        m_owner = pWho->GetGUID();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (check_timer.Expired(diff))
        {
            me->SetWalk(false);
            me->SetSpeed(MOVE_RUN, 2.5f);
            check_timer = 2000;
        }
        

        events.Update(diff);
        while(uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_FLAME_RANGE_CHECK:
                {
                    DoZoneInCombat();

                    if (Unit *pTarget = SelectUnit(SELECT_TARGET_FARTHEST, 0, 200.0f, true, 0, 48.0f))
                    {
                        AttackStart(pTarget);

                        me->SetSpeed(MOVE_RUN, 20.0f);
                        me->CastSpell(pTarget, SPELL_FLAME_CHARGE, true);

                        check_timer = 5000;
                    }

                    if (Creature *pOwner = me->GetMap()->GetCreature(m_owner))
                    {
                        if (!me->IsWithinDistInMap(pOwner, 30.0f) && !me->HasAura(SPELL_FLAME_ENRAGE, 0))
                            ForceSpellCast(me, SPELL_FLAME_ENRAGE);
                    }

                    events.ScheduleEvent(EVENT_FLAME_RANGE_CHECK, 2000);
                    break;
                }
                case EVENT_FLAME_FLAME_BLAST:
                {
                    AddSpellToCast(me->GetVictim(), SPELL_FLAME_FLAME_BLAST);
                    AddSpellToCast(me->GetVictim(), SPELL_FLAME_BLAZE);
                    events.ScheduleEvent(EVENT_FLAME_FLAME_BLAST, urand(16000, 25000));
                    break;
                }
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct boss_illidan_shadowdemonAI : public ScriptedAI
{
    boss_illidan_shadowdemonAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    uint64 m_targetGUID;
    Timer m_checkTimer;

    void Reset()
    {
        me->SetReactState(REACT_PASSIVE);

        m_checkTimer.Reset(2000);
        m_targetGUID = 0;
    }

    void MovementInform(uint32 type, uint32 data)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (Unit *pTarget = me->GetUnit(m_targetGUID))
            ForceSpellCast(pTarget, SPELL_SHADOW_DEMON_CONSUME_SOUL, INTERRUPT_AND_CAST_INSTANTLY);
    }

    void IsSummonedBy(Unit *pSummoner)
    {
        DoZoneInCombat();

        Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 150.0f, true);
        if (!target)
            return;
        if(target->HasAura(SPELL_ILLIDAN_INPRISON_RAID))
            me->DisappearAndDie();
        m_targetGUID = target->GetGUID();

        ForceSpellCast(target, SPELL_SHADOW_DEMON_FOUND_TARGET);
        ForceSpellCast(me, SPELL_SHADOW_DEMON_PASSIVE);
    }

    void JustDied(Unit *pKiller)
    {
        if (Unit *pUnit = me->GetUnit(m_targetGUID))
        {
            pUnit->RemoveAurasByCasterSpell(SPELL_SHADOW_DEMON_BEAM, me->GetGUID());
            pUnit->RemoveAurasByCasterSpell(SPELL_SHADOW_DEMON_PARALYZE, me->GetGUID());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (m_targetGUID)
        {
            if (m_checkTimer.Expired(diff))
            {
                Unit *pUnit = me->GetUnit(m_targetGUID);
                if (pUnit && pUnit->HasAura(SPELL_ILLIDAN_INPRISON_RAID))
                    me->DisappearAndDie();

                if (!pUnit || pUnit->isDead() || pUnit->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                {
                    DoZoneInCombat();

                    if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    {
                        m_targetGUID = pTarget->GetGUID();
                        ForceSpellCast(pTarget, SPELL_SHADOW_DEMON_FOUND_TARGET);
                    }
                }

                m_checkTimer = 2000;
            }
           
        }

        CastNextSpellIfAnyAndReady();
    }
};

struct boss_illidan_parasite_shadowfiendAI : public ScriptedAI
{
    boss_illidan_parasite_shadowfiendAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    void IsSummonedBy(Unit *pSummoner)
    {
        if (!pInstance)
            return;

        ForceSpellCast(me, 41913, INTERRUPT_AND_CAST, true);

        if (Creature *pIllidan = pInstance->GetCreature(pInstance->GetData64(DATA_ILLIDANSTORMRAGE)))
            pIllidan->AI()->JustSummoned(me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->GetVictim() || me->GetVictim()->GetTypeId() == TYPEID_UNIT)
        {
            DoZoneInCombat();
            if (Unit *pTemp = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true, 0, 5.0f))
            {
                if (me->GetVictim())
                    DoModifyThreatPercent(me->GetVictim(), -101);

                me->AddThreat(pTemp, 10000.0f);
                ScriptedAI::AttackStart(pTemp);
            }
            else
                me->DisappearAndDie();
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

bool GossipSelect_boss_illidan_akama(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF) // Time to begin the Event
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->AI()->DoAction(EVENT_AKAMA_START);
    }
    else
        pPlayer->CLOSE_GOSSIP_MENU();

    return true;
}

bool GossipHello_boss_illidan_akama(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    player->SEND_GOSSIP_MENU(10465, _Creature->GetGUID());
    return true;
}

bool GOUse_boss_illidan_cage_trap(Player* pPlayer, GameObject* pGo)
{
    if (pGo->GetGoState() == GO_STATE_ACTIVE)
        return false;

    pGo->CastSpell((Unit*)NULL, SPELL_MAIEV_CAGE_TRAP_TRIGGER);
    pGo->SetGoState(GO_STATE_ACTIVE);
    return true;
}

struct boss_illidan_cage_beamerAI : public ScriptedAI
{
    boss_illidan_cage_beamerAI(Creature *c) : ScriptedAI(c), summons(c){}

    SummonList summons;

    void JustSummoned(Creature *pWho)
    {
        uint32 spellId = 0;
        uint32 entry = 0;

        switch (pWho->GetEntry())
        {
            case 23296:
                spellId = 40704;
                entry = 23292;
                break;
            case 23297:
                spellId = 40707;
                entry = 23293;
                break;
            case 23298:
                spellId = 40708;
                entry = 23294;
                break;
            case 23299:
                spellId = 40709;
                entry = 23295;
                break;
        }

        if (Creature *pTarget = GetClosestCreatureWithEntry(me, entry, 12.0f))
        {
            pWho->CastSpell(pTarget, spellId, false);
            pTarget->CastSpell(pWho, spellId, false);
        }
    }

    void DoAction(const int32 action)
    {
        if (Creature *pIllidan = GetClosestCreatureWithEntry(me, 22917, 8.0f))
        {
            if (pIllidan->HasAura(SPELL_CAGED, 0) || CAST_AI(boss_illidan_stormrageAI, pIllidan->AI())->m_phase == PHASE_FOUR)
                return;

            me->SetOrientation(me->GetOrientationTo(pIllidan));

            // set 8 beam triggers
            for (uint32 i = 40696; i <= 40703; i++)
                me->CastSpell(pIllidan, i, false);

            pIllidan->RemoveAurasDueToSpell(SPELL_ILLIDAN_ENRAGE);
            pIllidan->CastSpell(pIllidan, SPELL_CAGED, true);
        }
        me->ForcedDespawn(15000);
    }
};

CreatureAI* GetAI_boss_illidan_stormrage(Creature *_Creature)
{
    return new boss_illidan_stormrageAI(_Creature);
}

CreatureAI* GetAI_boss_illidan_akama(Creature *_Creature)
{
    return new boss_illidan_akamaAI(_Creature);
}

CreatureAI* GetAI_boss_illidan_maiev(Creature *_Creature)
{
    return new boss_illidan_maievAI(_Creature);
}

CreatureAI* GetAI_boss_illidan_glaive(Creature *_Creature)
{
    return new boss_illidan_glaiveAI(_Creature);
}

CreatureAI* GetAI_boss_illidan_shadowdemon(Creature *_Creature)
{
    return new boss_illidan_shadowdemonAI(_Creature);
}

CreatureAI* GetAI_boss_illidan_flameofazzinoth(Creature *_Creature)
{
    return new boss_illidan_flameofazzinothAI(_Creature);
}

CreatureAI* GetAI_boss_illidan_cage_beamer(Creature *_Creature)
{
    return new boss_illidan_cage_beamerAI(_Creature);
}

CreatureAI* GetAI_boss_illidan_parasite_shadowfiend(Creature *_Creature)
{
    return new boss_illidan_parasite_shadowfiendAI(_Creature);
}

void AddSC_boss_illidan()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "boss_illidan_stormrage";
    newscript->GetAI = &GetAI_boss_illidan_stormrage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_akama";
    newscript->GetAI = &GetAI_boss_illidan_akama;
    newscript->pGossipHello = &GossipHello_boss_illidan_akama;
    newscript->pGossipSelect = &GossipSelect_boss_illidan_akama;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_maiev";
    newscript->GetAI = &GetAI_boss_illidan_maiev;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_glaive";
    newscript->GetAI = &GetAI_boss_illidan_glaive;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_shadowdemon";
    newscript->GetAI = &GetAI_boss_illidan_shadowdemon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_flameofazzinoth";
    newscript->GetAI = &GetAI_boss_illidan_flameofazzinoth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_cage_trap";
    newscript->pGOUse = &GOUse_boss_illidan_cage_trap;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_cage_beamer";
    newscript->GetAI = &GetAI_boss_illidan_cage_beamer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_illidan_parasite_shadowfiend";
    newscript->GetAI = &GetAI_boss_illidan_parasite_shadowfiend;
    newscript->RegisterSelf();
}
