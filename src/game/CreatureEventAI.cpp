// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2009 TrinityCore <http://www.trinitycore.org/>
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

#include "Common.h"
#include "CreatureEventAI.h"
#include "CreatureEventAIMgr.h"
#include "ObjectMgr.h"
#include "Spell.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GameEvent.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "InstanceData.h"
#include "SpellMgr.h"
#include "Chat.h"
#include "CreatureAIImpl.h"

bool CreatureEventAIHolder::UpdateRepeatTimer(Creature* creature, uint32 repeatMin, uint32 repeatMax)
{
    if (repeatMin == repeatMax)
        Time = repeatMin;
    else if (repeatMax > repeatMin)
        Time = urand(repeatMin, repeatMax);
    else
    {
        sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Creature %u using Event %u (Type = %u) has RandomMax < RandomMin. Event repeating disabled.", creature->GetEntry(), Event.event_id, Event.event_type);
        Enabled = false;
        return false;
    }

    return true;
}

int CreatureEventAI::Permissible(const Creature *creature)
{
    if (creature->GetAIName() == "EventAI")
        return PERMIT_BASE_SPECIAL;
    return PERMIT_BASE_NO;
}

CreatureEventAI::CreatureEventAI(Creature *c) : CreatureAI(c)
{
    // Need make copy for filter unneeded steps and safe in case table reload
    CreatureEventAI_Event_Map::const_iterator CreatureEvents = sCreatureEAIMgr.GetCreatureEventAIMap().find(int64(me->GetEntry()));
    if (CreatureEvents != sCreatureEAIMgr.GetCreatureEventAIMap().end())
    {
        std::vector<CreatureEventAI_Event>::const_iterator i;
        for (i = (*CreatureEvents).second.begin(); i != (*CreatureEvents).second.end(); ++i)
        {
            //Debug check
            #ifndef HELLGROUND_DEBUG
            if ((*i).event_flags & EFLAG_DEBUG_ONLY)
                continue;
            #endif
            if (((*i).event_flags & (EFLAG_HEROIC | EFLAG_NORMAL)) && m_creature->GetMap()->IsDungeon())
            {
                if ((m_creature->GetMap()->IsHeroic() && (*i).event_flags & EFLAG_HEROIC) ||
                    (!m_creature->GetMap()->IsHeroic() && (*i).event_flags & EFLAG_NORMAL))
                {
                    //event flagged for instance mode
                    CreatureEventAIList.push_back(CreatureEventAIHolder(*i));
                }
                continue;
            }
            CreatureEventAIList.push_back(CreatureEventAIHolder(*i));
        }
    }

    // Need make copy for filter unneeded steps and safe in case table reload
    CreatureEvents = sCreatureEAIMgr.GetCreatureEventAIMap().find(-int64(me->GetDBTableGUIDLow()));
    if (CreatureEvents != sCreatureEAIMgr.GetCreatureEventAIMap().end())
    {
        std::vector<CreatureEventAI_Event>::const_iterator i;
        for (i = (*CreatureEvents).second.begin(); i != (*CreatureEvents).second.end(); ++i)
        {

            //Debug check
            #ifndef HELLGROUND_DEBUG
            if ((*i).event_flags & EFLAG_DEBUG_ONLY)
                continue;
            #endif
            if (((*i).event_flags & (EFLAG_HEROIC | EFLAG_NORMAL)) && m_creature->GetMap()->IsDungeon())
            {
                if ((m_creature->GetMap()->IsHeroic() && (*i).event_flags & EFLAG_HEROIC) ||
                    (!m_creature->GetMap()->IsHeroic() && (*i).event_flags & EFLAG_NORMAL))
                {
                    //event flagged for instance mode
                    CreatureEventAIList.push_back(CreatureEventAIHolder(*i));
                }
                continue;
            }
            CreatureEventAIList.push_back(CreatureEventAIHolder(*i));
        }
    }

    // EventMap had events but they were not added because they must be for instance
    if (CreatureEventAIList.empty())
        sLog.outLog(LOG_DEFAULT, "ERROR: CreatureEventAI: Creature %u has events but no events added to list because of instance flags.", m_creature->GetEntry());

    bEmptyList = CreatureEventAIList.empty();
    Phase = 0;
    CombatMovementEnabled = true;
    MeleeEnabled = true;
    AttackDistance = 0.0f;
    AttackAngle = 0.0f;
    LastSpellMaxRange = 0;
    summoned = NULL;
    MoveChaseOnAttackTimer = 0;

    eventAISummonedList.clear();

    InvinceabilityHpLevel = 0;

    CreatureEventAI_Event cevent;
    cevent.event_id = 0;
    cevent.entryOrGUID = 0;
    cevent.event_type = EVENT_T_TIMER;
    cevent.event_inverse_phase_mask = 0;
    cevent.event_chance = 100;
    cevent.event_flags = 0;
    cevent.timer.initialMin = 0;
    cevent.timer.initialMax = 2000;
    cevent.timer.repeatMin = 2000;
    cevent.timer.repeatMax = 3000;
    cevent.action[0].type = ACTION_T_CHECK_OUT_OF_THREAT;
    cevent.action[1].type = ACTION_T_NONE;
    cevent.action[2].type = ACTION_T_NONE;

    CreatureEventAIList.push_back(CreatureEventAIHolder(cevent));

    //Handle Spawned Events
    if (!bEmptyList)
    {
        for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
        {
            if (SpawnedEventConditionsCheck((*i).Event))
                ProcessEvent(*i);
        }
    }
}

bool CreatureEventAI::ProcessEvent(CreatureEventAIHolder& pHolder, Unit* pActionInvoker)
{
    if (!pHolder.Enabled || pHolder.Time)
        return false;

    //Check the inverse phase mask (event doesn't trigger if current phase bit is set in mask)
    if (pHolder.Event.event_inverse_phase_mask & (1 << Phase))
        return false;

    CreatureEventAI_Event const& event = pHolder.Event;
    
    //Check event conditions based on the event type, also reset events
    switch (event.event_type)
    {
        case EVENT_T_TIMER:
            if (!m_creature->IsInCombat())
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.timer.repeatMin,event.timer.repeatMax);
            break;
        case EVENT_T_TIMER_OOC:
            if (m_creature->IsInCombat() || m_creature->IsInEvadeMode())
                return false;
            // Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature, event.timer.repeatMin, event.timer.repeatMax);
            break;
        case EVENT_T_TIMER_GENERIC:
            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.timer.repeatMin,event.timer.repeatMax);
            break;
        case EVENT_T_HP:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetMaxHealth())
                return false;

            uint32 perc = (m_creature->GetHealth()*100) / m_creature->GetMaxHealth();

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.percent_range.repeatMin,event.percent_range.repeatMax);
            break;
        }
        case EVENT_T_MANA:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetMaxPower(POWER_MANA))
                return false;

            uint32 perc = (m_creature->GetPower(POWER_MANA)*100) / m_creature->GetMaxPower(POWER_MANA);

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.percent_range.repeatMin,event.percent_range.repeatMax);
            break;
        }
        case EVENT_T_AGGRO:
            break;
        case EVENT_T_KILL:
            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.kill.repeatMin,event.kill.repeatMax);
            break;
        case EVENT_T_DEATH:
        case EVENT_T_EVADE:
            break;
        case EVENT_T_SPELLHIT:
            //Spell hit is special case, param1 and param2 handled within CreatureEventAI::SpellHit

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.spell_hit.repeatMin,event.spell_hit.repeatMax);
            break;
        case EVENT_T_RANGE:
            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.range.repeatMin,event.range.repeatMax);
            break;
        case EVENT_T_OOC_LOS:
            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.ooc_los.repeatMin,event.ooc_los.repeatMax);
            break;
        case EVENT_T_OOC_LOS_SPECIAL:
            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.raw.param3,event.raw.param4);
            break;
        case EVENT_T_RESET:
        case EVENT_T_SPAWNED:
            break;
        case EVENT_T_TARGET_HP:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim() || !m_creature->GetVictim()->GetMaxHealth())
                return false;

            uint32 perc = (m_creature->GetVictim()->GetHealth()*100) / m_creature->GetVictim()->GetMaxHealth();

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.percent_range.repeatMin,event.percent_range.repeatMax);
            break;
        }
        case EVENT_T_TARGET_CASTING:
            if (!m_creature->IsInCombat() || !m_creature->GetVictim() || !m_creature->GetVictim()->IsNonMeleeSpellCast(false, false, true))
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.target_casting.repeatMin,event.target_casting.repeatMax);
            break;
        case EVENT_T_FRIENDLY_HP:
        {
            if (!m_creature->IsInCombat())
                return false;

            CreatureEventAI_EventComputedData const& data = (*sCreatureEAIMgr.GetEAIComputedDataMap().find(event.event_id)).second; // always found
            Unit* pUnit = SelectLowestHpFriendly(float(event.friendly_hp.radius), float(event.friendly_hp.hpDeficit), data.friendlyHp.targetSelf);

            if (!pUnit)
                return false;

            pActionInvoker = pUnit;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.friendly_hp.repeatMin,event.friendly_hp.repeatMax);
            break;
        }
        case EVENT_T_FRIENDLY_IS_CC:
        {
            if (!m_creature->IsInCombat())
                return false;

            std::list<Creature*> pList;
            FindFriendlyCC(pList, event.friendly_is_cc.radius);

            //List is empty
            if (pList.empty())
                return false;

            //We don't really care about the whole list, just return first available
            pActionInvoker = *(pList.begin());

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.friendly_is_cc.repeatMin,event.friendly_is_cc.repeatMax);
            break;
        }
        case EVENT_T_FRIENDLY_MISSING_BUFF:
        {
            std::list<Creature*> pList;
            FindFriendlyMissingBuff(pList, event.friendly_buff.radius, event.friendly_buff.spellId);

            //List is empty
            if (pList.empty())
                return false;

            //We don't really care about the whole list, just return first available
            pActionInvoker = *(pList.begin());

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.friendly_buff.repeatMin,event.friendly_buff.repeatMax);
            break;
        }
        case EVENT_T_SUMMONED_UNIT:
        case EVENT_T_SUMMONED_JUST_DIED:
        case EVENT_T_SUMMONED_JUST_DESPAWN:
        {
            //Prevent event from occuring on no unit or non creatures
            if (!pActionInvoker || pActionInvoker->GetTypeId()!=TYPEID_UNIT)
                return false;

            //Creature id doesn't match up
            if (((Creature*)pActionInvoker)->GetEntry() != event.summon_unit.creatureId)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.summon_unit.repeatMin,event.summon_unit.repeatMax);
            break;
        }
        case EVENT_T_TARGET_MANA:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim() || !m_creature->GetVictim()->GetMaxPower(POWER_MANA))
                return false;

            uint32 perc = (m_creature->GetVictim()->GetPower(POWER_MANA)*100) / m_creature->GetVictim()->GetMaxPower(POWER_MANA);

            if (perc > event.percent_range.percentMax || perc < event.percent_range.percentMin)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.percent_range.repeatMin,event.percent_range.repeatMax);
            break;
        }
        case EVENT_T_REACHED_HOME:
        case EVENT_T_RECEIVE_EMOTE:
        case EVENT_T_QUEST_ACCEPT:
            break;
        case EVENT_T_BUFFED:
        {
            //Note: checked only aura for effect 0, if need check aura for effect 1/2 then
            // possible way: pack in event.buffed.amount 2 uint16 (ammount+effectIdx)
            Aura* aura = m_creature->GetAura(event.buffed.spellId,0);
            if (!aura || aura->GetStackAmount() < event.buffed.amount)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.buffed.repeatMin,event.buffed.repeatMax);
            break;
        }
        case EVENT_T_TARGET_BUFFED:
        {
            //Prevent event from occuring on no target or me is not in combat
            if (!m_creature->IsInCombat() || !m_creature->GetVictim())
                return false;

            //Note: checked only aura for effect 0, if need check aura for effect 1/2 then
            // possible way: pack in event.buffed.amount 2 uint16 (ammount+effectIdx)
            Aura* aura = m_creature->GetVictim()->GetAura(event.buffed.spellId,0);
            if (!aura || aura->GetStackAmount() < event.buffed.amount)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.buffed.repeatMin,event.buffed.repeatMax);
            break;
        }
        case EVENT_T_MISSING_AURA:
        {
            //Note: checked only aura for effect 0, if need check aura for effect 1/2 then
            // possible way: pack in event.buffed.amount 2 uint16 (ammount+effectIdx)
            if(m_creature->GetAuras().count(Unit::spellEffectPair(event.buffed.spellId,0)) >= event.buffed.amount)
                return false;
            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.buffed.repeatMin,event.buffed.repeatMax);
            break;
        }
        case EVENT_T_TARGET_MISSING_AURA:
        {
            if (!m_creature->IsInCombat() || !m_creature->GetVictim())
                return false;

            if(m_creature->GetVictim()->GetAuras().count(Unit::spellEffectPair(event.buffed.spellId,0)) >= event.buffed.amount)
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature,event.buffed.repeatMin,event.buffed.repeatMax);
            break;
        }
        case EVENT_T_RECEIVE_AI_EVENT:
            break;
        case EVENT_T_REACH_POINT:
        {
            break;
        }
        case EVENT_T_TARGET_HAS_AURA_TYPE:
        {
            //Prevent event from occuring on no unit
            if (!me->GetVictim())
                return false;

            if (!me->GetVictim()->HasAuraType(AuraType(event.raw.param1)))
                return false;

            //Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature, event.raw.param3, event.raw.param4);
            break;
        }
        case EVENT_T_SELECT_ATTACKING_TARGET:
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, event.selectTarget.maxRange, true, 0, event.selectTarget.minRange))
                m_eventTarget = target;
            else
                return false;
            // Repeat Timers
            pHolder.UpdateRepeatTimer(m_creature, event.selectTarget.repeatMin, event.selectTarget.repeatMax);
            break;
        }
        default:
            sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Creature %u using Event %u has invalid Event Type(%u), missing from ProcessEvent() Switch.", m_creature->GetEntry(), pHolder.Event.event_id, pHolder.Event.event_type);
            break;
    }

    SendDebug("Processing event %u; type %u, flags %u, chance %u",
        event.event_id, event.event_type, event.event_flags, event.event_chance);

    //Disable non-repeatable events
    if (!(pHolder.Event.event_flags & EFLAG_REPEATABLE))
        pHolder.Enabled = false;

    if(pHolder.Event.event_flags & EFLAG_NOT_CCED)
    {
        if(me->isCrowdControlled())
            return false;
    }

    //Store random here so that all random actions match up
    uint32 rnd = rand();

    //Return if chance for event is not met
    if (pHolder.Event.event_chance <= rnd % 100)
        return false;

    //Process actions
    for (uint32 j = 0; j < MAX_ACTIONS; j++)
        ProcessAction(pHolder.Event.action[j], rnd, pHolder.Event.event_id, pActionInvoker);

    return true;
}

void CreatureEventAI::ProcessAction(CreatureEventAI_Action const& action, uint32 rnd, uint32 EventId, Unit* pActionInvoker)
{
    if (action.type == ACTION_T_NONE)
        return;
    SendDebug("Processing action type %u", action.type);
    switch (action.type)
    {
        case ACTION_T_RANDOM_SAY:
        {
            if (!action.text.TextId[0] || !action.text.TextId[1])
                return;

            int32 temp = 0;
            // we should rand between two values
            int32 randomcount = action.text.TextId[1] - action.text.TextId[0];
            temp = action.text.TextId[0]+irand(randomcount, 0);

            if (temp)
            {
                Unit* target = NULL;

                if (pActionInvoker)
                {
                    if (pActionInvoker->GetTypeId() == TYPEID_PLAYER)
                        target = pActionInvoker;
                    else if (Unit* owner = pActionInvoker->GetOwner())
                    {
                        if (owner->GetTypeId() == TYPEID_PLAYER)
                            target = owner;
                    }
                }
                else if ((target = m_creature->GetVictim()))
                {
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        if (Unit* owner = target->GetOwner())
                            if (owner->GetTypeId() == TYPEID_PLAYER)
                                target = owner;
                }

                DoScriptText(temp, m_creature, target);
            }
            break;
        }
        case ACTION_T_TEXT:
        {
            if (!action.text.TextId[0])
                return;

            int32 temp = 0;

            if (action.text.TextId[1] && action.text.TextId[2])
                temp = action.text.TextId[rand()%3];
            else if (action.text.TextId[1] && urand(0,1))
                temp = action.text.TextId[1];
             else
                temp = action.text.TextId[0];

            if (temp)
            {
                Unit* target = NULL;

                if (pActionInvoker)
                {
                    if (pActionInvoker->GetTypeId() == TYPEID_PLAYER)
                        target = pActionInvoker;
                    else if (Unit* owner = pActionInvoker->GetOwner())
                    {
                        if (owner->GetTypeId() == TYPEID_PLAYER)
                            target = owner;
                    }
                }
                else if ((target = m_creature->GetVictim()))
                {
                    if (target->GetTypeId() != TYPEID_PLAYER)
                        if (Unit* owner = target->GetOwner())
                            if (owner->GetTypeId() == TYPEID_PLAYER)
                                target = owner;
                }

                DoScriptText(temp, m_creature, target);
            }
            break;
        }
        case ACTION_T_TEXT_WITH_TARGET:
        {
            if (!action.raw.param1)
                return;

            int32 temp = 0;

            temp = action.raw.param1;

            if (temp)
            {
                Unit* target = GetTargetByType(action.raw.param2, pActionInvoker);
                if(target)
                    DoScriptText(temp, m_creature, target);
            }
            break;
        }
        case ACTION_T_SET_FACTION:
        {
            if (action.set_faction.factionId)
                m_creature->setFaction(action.set_faction.factionId);
            else
            {
                if (CreatureInfo const* ci = GetCreatureTemplateStore(m_creature->GetEntry()))
                {
                    //if no id provided, assume reset and then use default
                    if (m_creature->getFaction() != ci->faction_A)
                        m_creature->setFaction(ci->faction_A);
                }
            }
            break;
        }
        case ACTION_T_MORPH_TO_ENTRY_OR_MODEL:
        {
            if (action.morph.creatureId || action.morph.modelId)
            {
                //set model based on entry from creature_template
                if (action.morph.creatureId)
                {
                    if (CreatureInfo const* ci = GetCreatureTemplateStore(action.morph.creatureId))
                    {
                        uint32 display_id = sObjectMgr.ChooseDisplayId(0,ci);
                        m_creature->SetDisplayId(display_id);
                    }
                }
                //if no param1, then use value from param2 (modelId)
                else
                    m_creature->SetDisplayId(action.morph.modelId);
            }
            else
                m_creature->DeMorph();
            break;
        }
        case ACTION_T_SOUND:
            m_creature->SendPlaySound(action.sound.soundId,false);
            break;
        case ACTION_T_EMOTE:
            m_creature->HandleEmoteCommand(action.emote.emoteId);
            break;
        case ACTION_T_AUTO_EMOTE:
            me->HandleEmote(action.emote.emoteId);
            break;
        case ACTION_T_LOAD_EQUIPMENT:
        {
            if(action.raw.param2 == 0)
                me->LoadEquipment(action.raw.param1);
            else
                me->LoadEquipment(action.raw.param1, true);
            break;
        }
        case ACTION_T_RANDOM_SOUND:
        {
            int32 temp = GetRandActionParam(rnd, action.random_sound.soundId1, action.random_sound.soundId2, action.random_sound.soundId3);
            if (temp >= 0)
                m_creature->SendPlaySound(temp,false);
            break;
        }
        case ACTION_T_RANDOM_EMOTE:
        {
            int32 temp = GetRandActionParam(rnd, action.random_emote.emoteId1, action.random_emote.emoteId2, action.random_emote.emoteId3);
            if (temp >= 0)
                m_creature->HandleEmoteCommand(temp);
            break;
        }
        case ACTION_T_CAST:
        {
            Unit* target = GetTargetByType(action.cast.target, pActionInvoker);
            Unit* caster = m_creature;

            if (!target && action.cast.target != TARGET_T_NULL)
                return;

            if (action.cast.castFlags & CAST_FORCE_TARGET_SELF && action.cast.target != TARGET_T_NULL)
                caster = target;

            //Allowed to cast only if not casting (unless we interrupt ourself) or if spell is triggered
            bool canCast = !caster->HasUnitState(UNIT_STAT_LOST_CONTROL) && (!caster->IsNonMeleeSpellCast(false) || (action.cast.castFlags & (CAST_TRIGGERED | CAST_INTURRUPT_PREVIOUS)));

            // If cast flag CAST_AURA_NOT_PRESENT is active, check if target already has aura on them
            if (action.cast.castFlags & CAST_AURA_NOT_PRESENT)
            {
                if (target->HasAura(action.cast.spellId,0))
                    return;
            }

            if (canCast)
            {
                const SpellEntry* tSpell = GetSpellStore()->LookupEntry<SpellEntry>(action.cast.spellId);

                //Verify that spell exists
                if (tSpell)
                {
                    //Check if cannot cast spell
                    if (!(action.cast.castFlags & (CAST_FORCE_TARGET_SELF | CAST_FORCE_CAST)) &&
                        !CanCast(target, tSpell, (action.cast.castFlags & CAST_TRIGGERED)))
                    {
                        SendDebug("Cannot cast (flags %u, spell %u)",action.cast.castFlags,action.cast.spellId);
                        //Melee current victim if flag not set
                        if (CombatMovementEnabled)
                        {
                            // Melee current victim if flag not set
							// Gensen: there was MELEE_RANGE instead of AttackDistance, but it caused bugs with creatures using scripts like ID 21878 (Boar)
                            if (!(action.cast.castFlags & CAST_NO_MELEE_IF_OOM))
                                m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim(), AttackDistance, 0.0f);
                        }
                        else if (!CombatMovementEnabled)
                        {
                            LastSpellMaxRange = 0.0f;
                        }
                    }
                    else
                    {
                        if (!CombatMovementEnabled)
                        {
							SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(action.cast.spellId);

                            if (spellInfo && !(spellInfo->rangeIndex == 2 || spellInfo->rangeIndex == 1) && target != m_creature)
                            {
                                SpellRangeEntry const* spellRange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
                                if (spellRange)
                                    LastSpellMaxRange = spellRange->maxRange;
                            }
                        }
                        SendDebug("Casting spell (flags %u, spell %u)", action.cast.castFlags, action.cast.spellId);
                        //Interrupt any previous spell
                        if (action.cast.castFlags & CAST_INTURRUPT_PREVIOUS && caster->IsNonMeleeSpellCast(false))
                            caster->InterruptNonMeleeSpells(false);

                        caster->CastSpell(target, action.cast.spellId, (action.cast.castFlags & CAST_TRIGGERED));
                    }

                }
                else
                    sLog.outLog(LOG_DB_ERR, "CreatureEventAI: event %d creature %d attempt to cast spell that doesn't exist %d", EventId, m_creature->GetEntry(), action.cast.spellId);
            }
            else
                SendDebug("Casting blocked by canCast (flags %u, spell %u)", action.cast.castFlags, action.cast.spellId);
            break;
        }
        case ACTION_T_CAST_GUID:
        {
            CreatureData const* cr_data = sObjectMgr.GetCreatureData(action.castguid.targetGUID);

            Unit* target = m_creature->GetMap()->GetCreature(MAKE_NEW_GUID(action.castguid.targetGUID, cr_data->id, HIGHGUID_UNIT));
            Unit* caster = m_creature;

            if (!target)
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: event %d creature %d attempt to cast spell on targetGUID %llu that doesn't exists in map", EventId, m_creature->GetEntry(), action.castguid.targetGUID);
                return;
            }

            //Allowed to cast only if not casting (unless we interrupt ourself) or if spell is triggered
            bool canCast = !caster->IsNonMeleeSpellCast(false) || (action.castguid.castFlags & (CAST_TRIGGERED | CAST_INTURRUPT_PREVIOUS));

            // If cast flag CAST_AURA_NOT_PRESENT is active, check if target already has aura on them
            if (action.castguid.castFlags & CAST_AURA_NOT_PRESENT)
            {
                if (target->HasAura(action.castguid.spellId,0))
                    return;
            }

            if (canCast)
            {
                const SpellEntry* tSpell = GetSpellStore()->LookupEntry<SpellEntry>(action.castguid.spellId);

                //Verify that spell exists
                if (tSpell)
                {
                    //Check if cannot cast spell
                    if (!(action.castguid.castFlags & (CAST_FORCE_TARGET_SELF | CAST_FORCE_CAST)) &&
                        !CanCast(target, tSpell, (action.castguid.castFlags & CAST_TRIGGERED)))
                    {
                        //Melee current victim if flag not set
                        if (!(action.castguid.castFlags & CAST_NO_MELEE_IF_OOM))
                        {
                            if (m_creature->HasUnitState(UNIT_STAT_CHASE))
                                m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim(), MELEE_RANGE, 0.0f);
                        }

                    }
                    else
                    {
                        //Interrupt any previous spell
                        if (caster->IsNonMeleeSpellCast(false) && action.castguid.castFlags & CAST_INTURRUPT_PREVIOUS)
                            caster->InterruptNonMeleeSpells(false);

                        caster->CastSpell(target, action.castguid.spellId, (action.castguid.castFlags & CAST_TRIGGERED));
                    }
                }
                else
                    sLog.outLog(LOG_DB_ERR, "CreatureEventAI: event %d creature %d attempt to cast spell that doesn't exist %d", EventId, m_creature->GetEntry(), action.castguid.spellId);
            }
            break;
        }
        case ACTION_T_SUMMON:
        {
            Unit* target = GetTargetByType(action.summon.target, pActionInvoker);

            Creature* pCreature = NULL;

            if (action.summon.duration)
                pCreature = m_creature->SummonCreature(action.summon.creatureId, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, action.summon.duration);
            else
                pCreature = m_creature->SummonCreature(action.summon.creatureId, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);

            if (!pCreature)
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: failed to spawn creature %u. Spawn event %d is on creature %d", action.summon.creatureId, EventId, m_creature->GetEntry());
            else if (action.summon.target != TARGET_T_SELF && target)
                pCreature->AI()->AttackStart(target);
            break;
        }
        case ACTION_T_THREAT_SINGLE_PCT:
            if (Unit* target = GetTargetByType(action.threat_single_pct.target, pActionInvoker))
                m_creature->getThreatManager().modifyThreatPercent(target, action.threat_single_pct.percent);
            break;
        case ACTION_T_THREAT_ALL_PCT:
        {
            std::list<HostileReference*>& threatList = m_creature->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::iterator i = threatList.begin(); i != threatList.end(); ++i)
                if (Unit* Temp = Unit::GetUnit(*m_creature,(*i)->getUnitGuid()))
                    m_creature->getThreatManager().modifyThreatPercent(Temp, action.threat_all_pct.percent);
            break;
        }
        case ACTION_T_QUEST_EVENT:
            if (Unit* target = GetTargetByType(action.quest_event.target, pActionInvoker))
                if (target->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)target)->AreaExploredOrEventHappens(action.quest_event.questId);
            break;
        case ACTION_T_CAST_EVENT:
            if (Unit* target = GetTargetByType(action.cast_event.target, pActionInvoker))
                if (target->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)target)->CastCreatureOrGO(action.cast_event.creatureId, m_creature->GetGUID(), action.cast_event.spellId);
            break;
        case ACTION_T_SET_UNIT_FIELD:
        {
            Unit* target = GetTargetByType(action.set_unit_field.target, pActionInvoker);

            // not allow modify important for integrity object fields
            if (action.set_unit_field.field < OBJECT_END || action.set_unit_field.field >= UNIT_END)
                return;

            if (target)
                target->SetUInt32Value(action.set_unit_field.field, action.set_unit_field.value);

            break;
        }
        case ACTION_T_SET_UNIT_FLAG:
            if (Unit* target = GetTargetByType(action.unit_flag.target, pActionInvoker))
                target->SetFlag(UNIT_FIELD_FLAGS, action.unit_flag.value);
            break;
        case ACTION_T_REMOVE_UNIT_FLAG:
            if (Unit* target = GetTargetByType(action.unit_flag.target, pActionInvoker))
                target->RemoveFlag(UNIT_FIELD_FLAGS, action.unit_flag.value);
            break;
        case ACTION_T_AUTO_ATTACK:
            MeleeEnabled = action.auto_attack.state != 0;
            break;
        case ACTION_T_COMBAT_MOVEMENT:
            // ignore no affect case
            if (CombatMovementEnabled==(action.combat_movement.state!=0))
                return;

            CombatMovementEnabled = action.combat_movement.state != 0;

            //Allow movement (create new targeted movement gen only if idle)
            if (CombatMovementEnabled)
            {
                if (action.combat_movement.melee && m_creature->IsInCombat())
                    if (Unit* victim = m_creature->GetVictim())
                        m_creature->SendMeleeAttackStart(victim->GetGUID());

                m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim(), AttackDistance, AttackAngle);
            }
            else
            {
                if (action.combat_movement.melee && m_creature->IsInCombat())
                    if (Unit* victim = m_creature->GetVictim())
						m_creature->SendMeleeAttackStop(victim);

                if (!m_creature->HasUnitState(UNIT_STAT_LOST_CONTROL))
                    m_creature->GetMotionMaster()->StopControlledMovement();
            }
            break;
        case ACTION_T_COMBAT_STOP:
            _EnterEvadeMode();
            break;
        case ACTION_T_CHECK_OUT_OF_THREAT:
            if (me->GetVictim() && me->IsOutOfThreatArea(me->GetVictim()))
                me->AI()->EnterEvadeMode();

            break;
        case ACTION_T_SET_PHASE:
            Phase = action.set_phase.phase;
            break;
        case ACTION_T_INC_PHASE:
        {
            int32 new_phase = int32(Phase)+action.set_inc_phase.step;
            if (new_phase < 0)
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Event %d decrease Phase under 0. CreatureEntry = %d", EventId, m_creature->GetEntry());
                Phase = 0;
            }
            else if (new_phase >= MAX_PHASE)
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Event %d incremented Phase above %u. Phase mask cannot be used with phases past %u. CreatureEntry = %d", EventId, MAX_PHASE-1, MAX_PHASE-1, m_creature->GetEntry());
                Phase = MAX_PHASE-1;
            }
            else
                Phase = new_phase;

            break;
        }
        case ACTION_T_EVADE:
            EnterEvadeMode();
            break;
        case ACTION_T_FLEE_FOR_ASSIST:
            m_creature->DoFleeToGetAssistance();
            break;
        case ACTION_T_QUEST_EVENT_ALL:
            if (pActionInvoker && pActionInvoker->GetTypeId() == TYPEID_PLAYER)
            {
                if (Unit* Temp = Unit::GetUnit(*m_creature,pActionInvoker->GetGUID()))
                    if (Temp->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)Temp)->GroupEventHappens(action.quest_event_all.questId,m_creature);
            }
            break;
        case ACTION_T_CAST_EVENT_ALL:
        {
            std::list<HostileReference*>& threatList = m_creature->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::iterator i = threatList.begin(); i != threatList.end(); ++i)
                if (Unit* Temp = Unit::GetUnit(*m_creature,(*i)->getUnitGuid()))
                    if (Temp->GetTypeId() == TYPEID_PLAYER)
                        ((Player*)Temp)->CastCreatureOrGO(action.cast_event_all.creatureId, m_creature->GetGUID(), action.cast_event_all.spellId);
            break;
        }
        case ACTION_T_REMOVEAURASFROMSPELL:
            if (Unit* target = GetTargetByType(action.remove_aura.target, pActionInvoker))
                target->RemoveAurasDueToSpell(action.remove_aura.spellId);
            break;
        case ACTION_T_RANGED_MOVEMENT:
            AttackDistance = (float)action.ranged_movement.distance;
            AttackAngle = action.ranged_movement.angle/180.0f*M_PI;

            if (CombatMovementEnabled)
                m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim(), AttackDistance, AttackAngle);

            break;
        case ACTION_T_RANDOM_PHASE:
            Phase = GetRandActionParam(rnd, action.random_phase.phase1, action.random_phase.phase2, action.random_phase.phase3);
            break;
        case ACTION_T_RANDOM_PHASE_RANGE:
            if (action.random_phase_range.phaseMin <= action.random_phase_range.phaseMax)
                Phase = urand(action.random_phase_range.phaseMin, action.random_phase_range.phaseMax);
            else
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: ACTION_T_RANDOM_PHASE_RANGE cannot have Param2 < Param1. Event = %d. CreatureEntry = %d", EventId, m_creature->GetEntry());
            break;
        case ACTION_T_SUMMON_ID:
        {
            Unit* target = GetTargetByType(action.summon_id.target, pActionInvoker);

            CreatureEventAI_Summon_Map::const_iterator i = sCreatureEAIMgr.GetCreatureEventAISummonMap().find(action.summon_id.spawnId);
            if (i == sCreatureEAIMgr.GetCreatureEventAISummonMap().end())
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: failed to spawn creature %u. Summon map index %u does not exist. EventID %d. CreatureID %d", action.summon_id.creatureId, action.summon_id.spawnId, EventId, m_creature->GetEntry());
                return;
            }

            Creature* pCreature = NULL;
            if ((*i).second.SpawnTimeSecs)
                pCreature = m_creature->SummonCreature(action.summon_id.creatureId, (*i).second.position_x, (*i).second.position_y, (*i).second.position_z, (*i).second.orientation, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, (*i).second.SpawnTimeSecs);
            else
                pCreature = m_creature->SummonCreature(action.summon_id.creatureId, (*i).second.position_x, (*i).second.position_y, (*i).second.position_z, (*i).second.orientation, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);

            if (!pCreature)
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: failed to spawn creature %u. EventId %d.Creature %d", action.summon_id.creatureId, EventId, m_creature->GetEntry());
            else if (action.summon_id.target != TARGET_T_SELF && target)
                pCreature->AI()->AttackStart(target);

            break;
        }
        case ACTION_T_KILLED_MONSTER:
            //first attempt player who tapped creature
            if (Player* pPlayer = m_creature->GetLootRecipient())
                pPlayer->RewardPlayerAndGroupAtEvent(action.killed_monster.creatureId, m_creature);
            else
            {
                //if not available, use pActionInvoker
                if (Unit* pTarget = GetTargetByType(action.killed_monster.target, pActionInvoker))
                    if (Player* pPlayer2 = pTarget->GetCharmerOrOwnerPlayerOrPlayerItself())
                        pPlayer2->RewardPlayerAndGroupAtEvent(action.killed_monster.creatureId, m_creature);
            }
            break;
        case ACTION_T_SET_INST_DATA:
        {
            InstanceData* pInst = (InstanceData*)m_creature->GetInstanceData();
            if (!pInst)
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Event %d attempt to set instance data without instance script. Creature %d", EventId, m_creature->GetEntry());
                return;
            }

            pInst->SetData(action.set_inst_data.field, action.set_inst_data.value);
            break;
        }
        case ACTION_T_SET_INST_DATA64:
        {
            Unit* target = GetTargetByType(action.set_inst_data64.target, pActionInvoker);
            if (!target)
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Event %d attempt to set instance data64 but Target == NULL. Creature %d", EventId, m_creature->GetEntry());
                return;
            }

            InstanceData* pInst = (InstanceData*)m_creature->GetInstanceData();
            if (!pInst)
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Event %d attempt to set instance data64 without instance script. Creature %d", EventId, m_creature->GetEntry());
                return;
            }

            pInst->SetData(action.set_inst_data64.field, target->GetGUID());
            break;
        }
        case ACTION_T_UPDATE_TEMPLATE:
            if (m_creature->GetEntry() == action.update_template.creatureId)
            {

                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Event %d ACTION_T_UPDATE_TEMPLATE call with param1 == current entry. Creature %d", EventId, m_creature->GetEntry());
                return;
            }

            m_creature->UpdateEntry(action.update_template.creatureId, action.update_template.team ? HORDE : ALLIANCE);
            break;
        case ACTION_T_DIE:
            if (m_creature->isDead())
            {

                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: Event %d ACTION_T_DIE on dead creature. Creature %d", EventId, m_creature->GetEntry());
                return;
            }
            m_creature->Kill(m_creature, false);
            break;
        case ACTION_T_ZONE_COMBAT_PULSE:
        {
            m_creature->SetInCombatWithZone();
            break;
        }
        case ACTION_T_CALL_FOR_HELP:
        {
            m_creature->CallForHelp(action.call_for_help.radius);
            break;
        }
        case ACTION_T_LOAD_PATH:
        {
            me->GetMotionMaster()->MovePath(action.loadpath.PathId, bool(action.loadpath.Repeatable));
            break;
        }
        break;

        // TRINITY ONLY
        case ACTION_T_MOVE_RANDOM_POINT: //dosen't work in combat
        {
            float x,y,z;
            me->GetNearPoint(x, y, z, me->GetObjectSize() / 3, action.raw.param1);
            me->GetMotionMaster()->MovePoint(0,x,y,z);
            break;
        }
        case ACTION_T_MOVE_RANDOM_AROUND:
        {
            me->GetMotionMaster()->MoveRandomAroundPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), action.raw.param1);
            break;
        }
        case ACTION_T_SET_STAND_STATE:
            me->SetStandState(UnitStandStateType(action.raw.param1));
            break;
        case ACTION_T_SET_PHASE_MASK:
            //me->SetPhaseMask(action.raw.param1, true);
            break;
        case ACTION_T_SET_VISIBILITY:
            me->SetVisibility(UnitVisibility(action.raw.param1));
            break;
        case ACTION_T_SET_ACTIVE:
            me->setActive(action.raw.param1 ? true : false);
            break;
        case ACTION_T_SET_AGGRESSIVE:
            me->SetReactState(ReactStates(action.raw.param1));
            break;
        case ACTION_T_ATTACK_START_PULSE:
        {
            // if we're searching for target in radius action.raw.param1, then we must be able to start attacking
            // Ensure it with SetAggroRange
            me->SetAggroRange((float)action.raw.param1);
            AttackStart(me->SelectNearestTarget((float)action.raw.param1));
            // After select is done we don't know for sure if we should continue aggroing on such a high range -> set aggro range to default
            me->SetAggroRange(0);
            break;
        }
        case ACTION_T_SUMMON_GO:
        {
            GameObject* pObject = NULL;

            float x,y,z;
            m_creature->GetPosition(x,y,z);

            if (pActionInvoker && 
               (m_creature->GetEntry() == 15262 || m_creature->GetEntry() == 15277 ||
                m_creature->GetEntry() == 15338 || m_creature->GetEntry() == 15355))
            {
                pObject = pActionInvoker->SummonGameObject(action.raw.param1, x, y, z, 0, 0, 0, 0, 0, action.raw.param2); 
            }
            else
                pObject = m_creature->SummonGameObject(action.raw.param1, x, y, z, 0, 0, 0, 0, 0, action.raw.param2);
            if (!pObject)
            {
                sLog.outLog(LOG_DB_ERR, "TSCR: EventAI failed to spawn object %u. Spawn event %d is on creature %d", action.raw.param1, EventId, m_creature->GetEntry());
            }
            break;
        }
        case ACTION_T_SET_SHEATH:
        {
            m_creature->SetByteValue(UNIT_FIELD_BYTES_2, 0, SheathState(action.set_sheath.sheath));
            break;
        }
        case ACTION_T_FORCE_DESPAWN:
        {
            m_creature->ForcedDespawn(action.forced_despawn.msDelay);
            break;
        }
        case ACTION_T_SET_INVINCIBILITY_HP_LEVEL:
        {
            if (action.invincibility_hp_level.is_percent)
                InvinceabilityHpLevel = m_creature->GetMaxHealth()*action.invincibility_hp_level.hp_level/100;
            else
                InvinceabilityHpLevel = action.invincibility_hp_level.hp_level;
            break;
        }
        case ACTION_T_MOUNT_TO_ENTRY_OR_MODEL:
        {
            if (action.mount.creatureId || action.mount.modelId)
            {
                // set model based on entry from creature_template
                if (action.mount.creatureId)
                {
                    if (CreatureInfo const* cInfo = GetCreatureTemplateStore(action.morph.creatureId))
                    {
                        uint32 display_id = sObjectMgr.ChooseDisplayId(0,cInfo);
                        m_creature->Mount(display_id);
                    }
                }
                // if no param1, then use value from param2 (modelId)
                else
                    m_creature->Mount(action.mount.modelId);
            }
            else
                m_creature->Unmount();

            break;
        }
        case ACTION_T_THROW_AI_EVENT:
        {
            Unit* target = GetTargetByType(action.throwEvent.target, pActionInvoker);
            if (!target)
            {
                sLog.outLog(LOG_DB_ERR, "Event %d attempt to start relay script but Target == nullptr. Creature %d", EventId, m_creature->GetEntry());
                return;
            }
            SendAIEventAround(AIEventType(action.throwEvent.eventType), pActionInvoker, 1, action.throwEvent.radius);
            break;
        }
        case ACTION_T_REMOVE_CORPSE:
        {
            m_creature->RemoveCorpse();
            break;
        }
        case ACTION_T_MOVE_POINT:
        {
            CreatureEventAI_Positions_Map::const_iterator i = sCreatureEAIMgr.GetCreatureEventAIPositionsMap().find(action.movepoint.positionId);
            if (i == sCreatureEAIMgr.GetCreatureEventAIPositionsMap().end())
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: failed to movepoint %u. Position index %u does not exist. EventID %d.", action.movepoint.positionId, action.movepoint.positionId, EventId);
                return;
            }            

            me->GetMotionMaster()->MovePoint(action.movepoint.positionId, (*i).second.position_x, (*i).second.position_y, (*i).second.position_z);
            break;
        }
        case ACTION_T_SET_HOME_POSITION:
        {
            CreatureEventAI_Positions_Map::const_iterator i = sCreatureEAIMgr.GetCreatureEventAIPositionsMap().find(action.sethomepos.positionId);
            if (i == sCreatureEAIMgr.GetCreatureEventAIPositionsMap().end())
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: failed to sethomepos %u. Position index %u does not exist. EventID %d.", action.sethomepos.positionId, action.sethomepos.positionId, EventId);
                return;
            }            

            me->SetHomePosition((*i).second.position_x, (*i).second.position_y, (*i).second.position_z, (*i).second.orientation);
            break;
        }
        case ACTION_T_SET_ORIENTATION:
        {
            CreatureEventAI_Positions_Map::const_iterator i = sCreatureEAIMgr.GetCreatureEventAIPositionsMap().find(action.setorientation.positionId);
            if (i == sCreatureEAIMgr.GetCreatureEventAIPositionsMap().end())
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: failed to setorientation %u. Position index %u does not exist. EventID %d.", action.setorientation.positionId, action.setorientation.positionId, EventId);
                return;
            }            

            me->SetOrientation((*i).second.orientation);
            me->SetFacingTo((*i).second.orientation);
            break;
        }
        case ACTION_T_NEAR_TELEPORT:
        {
            CreatureEventAI_Positions_Map::const_iterator i = sCreatureEAIMgr.GetCreatureEventAIPositionsMap().find(action.raw.param1);
            if (i == sCreatureEAIMgr.GetCreatureEventAIPositionsMap().end())
            {
                sLog.outLog(LOG_DB_ERR, "CreatureEventAI: failed to teleportto %u. Position index %u does not exist. EventID %d.", action.raw.param1, action.raw.param1, EventId);
                return;
            }            

            me->NearTeleportTo((*i).second.position_x, (*i).second.position_y, (*i).second.position_z, (*i).second.orientation);
            break;
        }
        case ACTION_T_SET_MOVE_SPEED:
        {
            switch(action.raw.param1)
            {
                case 0:
                    me->SetSpeed(MOVE_WALK, action.raw.param2);
                    break;
                case 1:
                    me->SetSpeed(MOVE_RUN, action.raw.param2);
                    break;
                case 2:
                    me->SetSpeed(MOVE_RUN_BACK, action.raw.param2);
                    break;
                case 3:
                    me->SetSpeed(MOVE_SWIM, action.raw.param2);
                    break;
                case 4:
                    me->SetSpeed(MOVE_SWIM_BACK, action.raw.param2);
                    break;
                case 5:
                    me->SetSpeed(MOVE_TURN_RATE, action.raw.param2);
                    break;
                case 6:
                    me->SetSpeed(MOVE_FLIGHT, action.raw.param2);
                    break;
                case 7:
                    me->SetSpeed(MOVE_FLIGHT_BACK, action.raw.param2);
                    break;
                default:
                    me->SetSpeed(MOVE_WALK, action.raw.param2);
                    break;
            }
            break;
        }
        case ACTION_T_SET_WALK:
        {
            switch(action.raw.param1)
            {
                case 1:
                    me->SetWalk(true);
                    break;
                case 2:
                    me->SetWalk(false);
                    break;
                default:
                    break;
            }
            break;
        }
        case ACTION_T_SET_HOVER_LEVITATE:
        {
            switch(action.raw.param1)
            {
                case 0:
                    me->SetLevitate(false);
                    me->setHover(false);
                    break;
                case 1:
                    me->setHover(true);
                    break;
                case 2:
                    me->SetLevitate(true);
                    break;
                case 3:
                    me->SetLevitate(false);
                    me->setHover(false);
                    break;
                default: break;
            }
            break;
        }
        case ACTION_T_SET_DEFAULT_MOVEMENT_TYPE:
        {
            switch(action.raw.param1)
            {
                case 1:
                    me->SetDefaultMovementType(IDLE_MOTION_TYPE);
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    break;
                case 2:
                    me->SetDefaultMovementType(RANDOM_MOTION_TYPE);
                    me->GetMotionMaster()->MoveRandomAroundPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), action.raw.param2 != 0 ? action.raw.param2 : 5);
                    me->GetMotionMaster()->Initialize();
                    break;
                case 3:
                    if(action.raw.param2 != 0)
                        me->LoadPath(action.raw.param2);
                    me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                    me->GetMotionMaster()->Initialize();
                    break;
                default: break;
            }
            break;
        }
        case ACTION_T_SET_FLYING:
        {
            me->SetFlying(action.raw.param1);
            break;
        }
        case ACTION_T_MOUNT:
        {
            switch(action.raw.param1)
            {
                case 1:
                    me->Unmount();
                    break;
                case 2:
                    me->Mount(action.raw.param2);
                    break;
                default: break;
            }
            break;
        }
        case ACTION_T_HANDLE_GAMEOBJECT:
        {
            GameObject* pGo = NULL;

            Hellground::NearestGameObjectEntryInObjectRangeCheck go_check(*me, action.raw.param1, action.raw.param2);
            Hellground::ObjectLastSearcher<GameObject, Hellground::NearestGameObjectEntryInObjectRangeCheck> searcher(pGo, go_check);

            Cell::VisitGridObjects(me, searcher, action.raw.param2);
            if (pGo)
                pGo->SetGoState(action.raw.param3 ? GO_STATE_ACTIVE : GO_STATE_READY);
            break;
        }
        case ACTION_T_DISABLE_MMAPS:
        {
            if(action.raw.param1 == 1)
                me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);
            else
                me->ClearUnitState(UNIT_STAT_IGNORE_PATHFINDING);
            break;
        }
        case ACTION_T_SET_EVADABLE_AT_DIST:
        {
            if(action.raw.param1 == 1)
                me->SetIsDistanceToHomeEvadable(false);
            else
                me->SetIsDistanceToHomeEvadable(true);
            break;
        }
        case ACTION_T_DYNAMIC_MOVEMENT:
        {
            if (action.dynamicMovement.state && CombatMovementEnabled || !action.dynamicMovement.state && !CombatMovementEnabled)
                break;

            CombatMovementEnabled = action.dynamicMovement.state;
            SetCombatMove(!CombatMovementEnabled, true);
            break;
        }
        case ACTION_T_FORCE_REACTION:
        {
            if (Unit* target = GetTargetByType(action.raw.param1, pActionInvoker))
            {
                if (target->GetTypeId() == TYPEID_PLAYER)
                {
                    ((Player*)target)->GetReputationMgr().ApplyForceReaction(action.raw.param2, ReputationRank(action.raw.param3), true);
                    ((Player*)target)->GetReputationMgr().SendForceReactions();
                }
            }
            break;
        }
        case ACTION_T_INTERRUPT_SPELLS:
        {
            if (action.raw.param1 == 1)
                me->InterruptNonMeleeSpells(true);
            else
                me->InterruptNonMeleeSpells(false);
            break;
        }
        case ACTION_T_CAST_DEFAULT:
        {
            Unit* target = GetTargetByType(action.raw.param2, pActionInvoker);
            if (action.raw.param3 == 1)
                me->CastSpell(target ? target : me, action.raw.param1, true);
            else
                me->CastSpell(target ? target : me, action.raw.param1, false);
            break;
        }
        case ACTION_T_MOVE_FOLLOW:
        {
            Unit* target = GetTargetByType(action.raw.param1, pActionInvoker);
            if (target)
            {
                me->GetMotionMaster()->MoveFollow(target, action.raw.param2, action.raw.param3);
            }
            break;
        }
        case ACTION_T_SET_GO_GUID_LOOT_STATE:
        {
            GameObject* pGo = NULL;

            Hellground::NearestGameObjectEntryInObjectRangeCheck go_check(*me, action.raw.param1, action.raw.param2);
            Hellground::ObjectLastSearcher<GameObject, Hellground::NearestGameObjectEntryInObjectRangeCheck> searcher(pGo, go_check);

            Cell::VisitGridObjects(me, searcher, action.raw.param2);
            if (pGo)
                pGo->SetLootState((LootState)action.raw.param3);
            break;
        }
        case ACTION_T_APPLY_IMMUNITY:
        {
            me->ApplySpellImmune(action.raw.param1, action.raw.param2, action.raw.param3, true);
            break;
        }
        case ACTION_T_REMOVE_IMMUNITY:
        {
            me->ApplySpellImmune(action.raw.param1, action.raw.param2, action.raw.param3, false);
            break;
        }
    }
}

void CreatureEventAI::JustRespawned()
{
    Reset();

    if (bEmptyList)
        return;

    // Reset generic timer
	for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        if ((*i).Event.event_type == EVENT_T_TIMER_GENERIC)
			if ((*i).UpdateRepeatTimer(m_creature, i->Event.timer.initialMin, i->Event.timer.initialMax))
				(*i).Enabled = true;
    }

    //Handle Spawned Events
    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
        if (SpawnedEventConditionsCheck((*i).Event))
            ProcessEvent(*i);
}

void CreatureEventAI::Reset()
{
    EventUpdateTime.Reset(EVENT_UPDATE_TIME);
    EventDiff = 0;

    if (bEmptyList)
        return;

    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        if ((*i).Event.event_type == EVENT_T_RESET)
            ProcessEvent(*i);
    }

    //Reset all events to enabled
    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        CreatureEventAI_Event const& event = (*i).Event;
        switch (event.event_type)
        {
            //Reset all out of combat timers
            case EVENT_T_TIMER_OOC:
            {
                if ((*i).UpdateRepeatTimer(m_creature,event.timer.initialMin,event.timer.initialMax))
                    (*i).Enabled = true;
                break;
            }
            //default:
            //TODO: enable below code line / verify this is correct to enable events previously disabled (ex. aggro yell), instead of enable this in void EnterCombat()
            //(*i).Enabled = true;
            //(*i).Time = 0;
            //break;
        }
    }
}

void CreatureEventAI::JustReachedHome()
{
    m_creature->LoadCreaturesAddon();

    if (!bEmptyList)
    {
        for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
        {
            if ((*i).Event.event_type == EVENT_T_REACHED_HOME)
                ProcessEvent(*i);
        }
    }
    Reset();
}

void CreatureEventAI::MovementInform(uint32 MovementType, uint32 Data)
{
    if (bEmptyList)
        return;

    // Handle MovementInform events
    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        if ((*i).Event.event_type == EVENT_T_REACH_POINT && (*i).Event.raw.param1 == Data)
            ProcessEvent(*i);
    }

    if (MovementType != POINT_MOTION_TYPE)
        return;
}

void CreatureEventAI::EnterEvadeMode()
{
    CreatureAI::EnterEvadeMode();

    if (bEmptyList)
        return;

    //Handle Evade events
    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        if ((*i).Event.event_type == EVENT_T_EVADE)
            ProcessEvent(*i);
    }
}

void CreatureEventAI::JustDied(Unit* killer)
{
    Reset();

    if (bEmptyList)
        return;

    //Handle Evade events
    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        if ((*i).Event.event_type == EVENT_T_DEATH)
            ProcessEvent(*i, killer);
    }

    eventAISummonedList.clear();

    // reset phase after any death state events
    Phase = 0;
}

void CreatureEventAI::KilledUnit(Unit* victim)
{
    if (bEmptyList || victim->GetTypeId() != TYPEID_PLAYER)
        return;

    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        if ((*i).Event.event_type == EVENT_T_KILL)
            ProcessEvent(*i, victim);
    }
}

void CreatureEventAI::JustSummoned(Creature* pUnit)
{
    if (bEmptyList || !pUnit)
        return;

    uint32 entry = pUnit->GetEntry();
    uint32 level = me->GetLevel();
    uint32 hp = 0;

    pUnit->SetLevel(level);

    switch (entry)
    {
        case 12922: //imp
            hp = 17.8 * level - 54;
            break;
        case 10928: //succub
            hp = 63 * level - 986;
            break;
        case 8996:  //void
            hp = 55 * level - 116;
            break;
        default:
            break;
    }

    // set new hp val
    if (hp)
    {
        pUnit->SetHealth(hp);
        pUnit->SetMaxHealth(hp);
    }

    eventAISummonedList.push_back(pUnit->GetGUID());

    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        if ((*i).Event.event_type == EVENT_T_SUMMONED_UNIT)
            ProcessEvent(*i, pUnit);
    }
}

void CreatureEventAI::SummonedCreatureDespawn(Creature* summoned)
{
    for (auto& i : CreatureEventAIList)
    {
        if (i.Event.event_type == EVENT_T_SUMMONED_JUST_DESPAWN)
            ProcessEvent(i, summoned);
    }
}
/*
void CreatureEventAI::OnQuestAccept(Player* pPlayer, Quest const* pQuest)
{
    for (auto& i : CreatureEventAIList)
    {
        if (i.Event.event_type == EVENT_T_QUEST_ACCEPT)
        {
            if(pQuest->GetQuestId() == i.Event.quest.questId)
                ProcessEvent(i, pPlayer);
        }
    }
}
*/
void CreatureEventAI::SummonedCreatureDies(Creature* summoned)
{
    for (auto& i : CreatureEventAIList)
    {
        if (i.Event.event_type == EVENT_T_SUMMONED_JUST_DIED)
            ProcessEvent(i, summoned);
    }
}

void CreatureEventAI::ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 /*miscValue*/)
{
    for (std::list<CreatureEventAIHolder>::iterator itr = CreatureEventAIList.begin(); itr != CreatureEventAIList.end(); ++itr)
    {
        if ((*itr).Event.event_type == EVENT_T_RECEIVE_AI_EVENT &&
                (*itr).Event.receiveAIEvent.eventType == eventType && (!(*itr).Event.receiveAIEvent.senderEntry || (*itr).Event.receiveAIEvent.senderEntry == sender->GetEntry()))
            ProcessEvent(*itr);
    }
}

void CreatureEventAI::EnterCombat(Unit *enemy)
{
    CreatureAI::EnterCombat(enemy);
    
    if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == RANDOM_MOTION_TYPE)
		m_creature->GetMotionMaster()->StopControlledMovement();
	
	//Check for on combat start events
    if (!bEmptyList)
    {
        for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
        {
            CreatureEventAI_Event const& event = (*i).Event;
            switch (event.event_type)
            {
                case EVENT_T_AGGRO:
                    (*i).Enabled = true;
                    ProcessEvent(*i, enemy);
                    break;
                    //Reset all in combat timers
                case EVENT_T_TIMER:
                    if ((*i).UpdateRepeatTimer(m_creature,event.timer.initialMin,event.timer.initialMax))
                        (*i).Enabled = true;
                    break;
                // Reset some special combat timers using repeatMin/Max
                case EVENT_T_SELECT_ATTACKING_TARGET:
                    if ((*i).UpdateRepeatTimer(m_creature, event.selectTarget.repeatMin, event.selectTarget.repeatMax))
                        (*i).Enabled = true;
                    break;
                    //All normal events need to be re-enabled and their time set to 0
                default:
                    (*i).Enabled = true;
                    (*i).Time = 0;
                    break;
            }
        }
    }

    EventUpdateTime.Reset(EVENT_UPDATE_TIME);
    EventDiff = 0;
}
void CreatureEventAI::OnSpellInterrupt(SpellEntry const* spellInfo)
{
    if (me->isSchoolProhibited((SpellSchoolMask)spellInfo->SchoolMask))
    {
        CombatMovementEnabled = true;
        AttackDistance = 0.0f;
        m_creature->GetMotionMaster()->MoveChase(me->GetVictim(), AttackDistance, AttackAngle);
    }
}

void CreatureEventAI::AttackStart(Unit *who)
{
    if (!who)
        return;

    if (m_creature->Attack(who, MeleeEnabled))
    {
        if (CombatMovementEnabled)
        {
            if (!bEmptyList) // if damage spell with castingtime > 0 then it should be ranged npc
            {
                for (std::list<CreatureEventAIHolder>::iterator itr = CreatureEventAIList.begin(); itr != CreatureEventAIList.end(); ++itr)
                {
                    std::vector<float> ActionSpellRange{ 0.0, 0.0, 0.0 };
                    float rangeMax = 0.0;
                    if ((*itr).Event.event_type == EVENT_T_TIMER && ((*itr).Time <= 4000))
                    {
                        if ((*itr).Event.action[0].type == ACTION_T_CAST)
                        {
                            if (!me->HasSpellCooldown((*itr).Event.action[0].cast.spellId))
                            {
                                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>((*itr).Event.action[0].cast.spellId);
                                for (int i = 0; i < 3; i++)
                                {
                                    if (spellInfo && (spellInfo->Effect[i] == SPELL_EFFECT_SCHOOL_DAMAGE) && (SpellMgr::GetSpellCastTime(spellInfo) > 0))
                                        MoveChaseOnAttackTimer = 1000;
                                        //ActionSpellRange[0] = (SpellMgr::GetSpellMaxRange((*itr).Event.action[0].cast.spellId) / 2);
                                }
                            }
                        }
                        else if ((*itr).Event.action[1].type == ACTION_T_CAST)
                        {
                            if (!me->HasSpellCooldown((*itr).Event.action[1].cast.spellId))
                            {
                                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>((*itr).Event.action[1].cast.spellId);
                                for (int i = 0; i < 3; i++)
                                {
                                    if (spellInfo && (spellInfo->Effect[i] == SPELL_EFFECT_SCHOOL_DAMAGE) && (SpellMgr::GetSpellCastTime(spellInfo) > 0))
                                        MoveChaseOnAttackTimer = 1000;
                                        // ActionSpellRange[1] = (SpellMgr::GetSpellMaxRange((*itr).Event.action[1].cast.spellId) / 2);
                                }
                            }
                        }
                        else if ((*itr).Event.action[2].type == ACTION_T_CAST)
                        {
                            if (!me->HasSpellCooldown((*itr).Event.action[2].cast.spellId))
                            {
                                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>((*itr).Event.action[2].cast.spellId);
                                for (int i = 0; i < 3; i++)
                                {
                                    if (spellInfo && (spellInfo->Effect[i] == SPELL_EFFECT_SCHOOL_DAMAGE) && (SpellMgr::GetSpellCastTime(spellInfo) > 0))
                                        MoveChaseOnAttackTimer = 1000;
                                        // ActionSpellRange[2] = (SpellMgr::GetSpellMaxRange((*itr).Event.action[2].cast.spellId) / 2);
                                }
                            }
                        }

                        /*rangeMax = *std::max_element(ActionSpellRange.begin(), ActionSpellRange.end());

                        if (rangeMax > 5.0f)
                            AttackDistance = *std::max_element(ActionSpellRange.begin(), ActionSpellRange.end());*/
                    }
                }
            }
            if (!MoveChaseOnAttackTimer)
            {
                m_creature->GetMotionMaster()->MoveChase(who, AttackDistance, AttackAngle);
                // AttackDistance = 0.0f;
            }
        }
    }
}

void CreatureEventAI::MoveInLineOfSight(Unit *who)
{
    // Gensen: i added IsDungeon() condition because if NPC sees me, he must attack me, otherwise i can go out of combat in instances
    if (!me->GetMap()->IsDungeon() && me->GetVictim())
        return;

	if (!me->CanFly() && me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
		return;

    //Check for OOC LOS Event
    if (!bEmptyList)
    {
        for (std::list<CreatureEventAIHolder>::iterator itr = CreatureEventAIList.begin(); itr != CreatureEventAIList.end(); ++itr)
        {
            if ((*itr).Event.event_type == EVENT_T_OOC_LOS)
            {
                //can trigger if closer than fMaxAllowedRange
                float fMaxAllowedRange = (*itr).Event.ooc_los.maxRange;

                //if range is ok and we are actually in LOS
                if (m_creature->IsWithinDistInMap(who, fMaxAllowedRange) && m_creature->IsWithinLOSInMap(who))
                {
                    //if friendly event&&who is not hostile OR hostile event&&who is hostile
                    if ((((*itr).Event.ooc_los.noHostile == 1) && !m_creature->IsHostileTo(who)) ||
                        ((!(*itr).Event.ooc_los.noHostile) && (me->IsHostileTo(who) || who->IsHostileTo(me))) ||
                        (((*itr).Event.ooc_los.noHostile == 2) && (who->GetTypeId() == TYPEID_PLAYER) && !m_creature->IsHostileTo(who)))
                        ProcessEvent(*itr, who);
                }
            }
            else if((*itr).Event.event_type == EVENT_T_OOC_LOS_SPECIAL)
            {
                float fMaxAllowedRange = (*itr).Event.raw.param1;
                if (m_creature->IsWithinDistInMap(who, fMaxAllowedRange) && m_creature->IsWithinLOSInMap(who))
                {
                    if (who->GetTypeId() == TYPEID_PLAYER)
                    {
                        if(((Player*)who)->GetQuestStatus((*itr).Event.raw.param2) == QUEST_STATUS_COMPLETE && ((Player*)who)->GetQuestRewardStatus((*itr).Event.raw.param2))
                            ProcessEvent(*itr, who);
                    }
                }
            }
        }
    }

    CreatureAI::MoveInLineOfSight(who);
}

void CreatureEventAI::SpellHit(Unit* pUnit, const SpellEntry* pSpell)
{

    if (bEmptyList)
        return;

    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
        if ((*i).Event.event_type == EVENT_T_SPELLHIT)
            //If spell id matches (or no spell id) & if spell school matches (or no spell school)
            if (!(*i).Event.spell_hit.spellId || pSpell->Id == (*i).Event.spell_hit.spellId)
                if (pSpell->SchoolMask & (*i).Event.spell_hit.schoolMask)
                    ProcessEvent(*i, pUnit);
}

void CreatureEventAI::UpdateAI(const uint32 diff)
{
    //Check if we are in combat (also updates calls threat update code)
    bool Combat = UpdateVictim();

    if (!bEmptyList)
    {
        //Events are only updated once every EVENT_UPDATE_TIME ms to prevent lag with large amount of events
        EventDiff += diff;
        if (EventUpdateTime.Expired(diff))
        {
            //Check for time based events
            for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
            {
                //Decrement Timers
                if ((*i).Time)
                {
                    if ((*i).Time > EventDiff)
                    {
                        //Do not decrement timers if event cannot trigger in this phase
                        if (!((*i).Event.event_inverse_phase_mask & (1 << Phase)))
                            (*i).Time -= EventDiff;

                        //Skip processing of events that have time remaining
                        continue;
                    }
                    else (*i).Time = 0;
                }

                //Events that are updated every EVENT_UPDATE_TIME
                switch ((*i).Event.event_type)
                {
                    case EVENT_T_TIMER_OOC:
                    case EVENT_T_TIMER_GENERIC:
                    case EVENT_T_MISSING_AURA:
                    case EVENT_T_BUFFED:
                        ProcessEvent(*i);
                        break;
                    case EVENT_T_TIMER:
                    case EVENT_T_MANA:
                    case EVENT_T_HP:
                    case EVENT_T_TARGET_HP:
                    case EVENT_T_TARGET_CASTING:
                    case EVENT_T_FRIENDLY_HP:
                    case EVENT_T_TARGET_BUFFED:
                    case EVENT_T_TARGET_MISSING_AURA:
                    case EVENT_T_TARGET_MANA:
                    case EVENT_T_SELECT_ATTACKING_TARGET:
                    case EVENT_T_FRIENDLY_IS_CC:
                        if (me->GetVictim())
                            ProcessEvent(*i);
                        break;
                    case EVENT_T_RANGE:
                        if (me->GetVictim())
                        {
                            if (m_creature->IsInMap(m_creature->GetVictim()))
                            {
                                if (m_creature->IsInRange(m_creature->GetVictim(),(float)(*i).Event.range.minDist,(float)(*i).Event.range.maxDist))
                                    ProcessEvent(*i);
                            }
                        }
                        break;
                }
            }

            EventDiff = 0;
            EventUpdateTime = EVENT_UPDATE_TIME;
        }
        
    }

    //Melee Auto-Attack
    if (Combat && me->GetVictim())
    {
        if (MoveChaseOnAttackTimer)
        {
            if (MoveChaseOnAttackTimer <= diff)
            {
                m_creature->GetMotionMaster()->MoveChase(me->GetVictim(), AttackDistance, AttackAngle);
                MoveChaseOnAttackTimer = 0;
            }
            else
                MoveChaseOnAttackTimer -= diff;
        }

        // Update creature dynamic movement position before doing anything else
        if (!CombatMovementEnabled)
        {
            if (!m_creature->HasUnitState(UNIT_STAT_CAN_NOT_REACT) && !m_creature->IsNonMeleeSpellCast(false))
            {
                if (LastSpellMaxRange && m_creature->IsInRange(m_creature->GetVictim(), 0, (LastSpellMaxRange / 1.5f)))
                    SetCombatMove(false, true);
                else
                    SetCombatMove(true, true);
            }
        }
        else if (MeleeEnabled)
            DoMeleeAttackIfReady();
    }
}

inline uint32 CreatureEventAI::GetRandActionParam(uint32 rnd, uint32 param1, uint32 param2, uint32 param3)
{
    switch (rnd % 3)
    {
        case 0: return param1;
        case 1: return param2;
        case 2: return param3;
    }
    return 0;
}

inline int32 CreatureEventAI::GetRandActionParam(uint32 rnd, int32 param1, int32 param2, int32 param3)
{
    switch (rnd % 3)
    {
        case 0: return param1;
        case 1: return param2;
        case 2: return param3;
    }
    return 0;
}

inline Unit* CreatureEventAI::GetTargetByType(uint32 Target, Unit* pActionInvoker)
{
    Unit* target = nullptr;
    switch (Target)
    {
        case TARGET_T_SELF:
            return m_creature;
        case TARGET_T_HOSTILE:
            return m_creature->GetVictim();
        case TARGET_T_HOSTILE_SECOND_AGGRO:
            target = SelectUnit(SELECT_TARGET_TOPAGGRO, 1);
            break;
        case TARGET_T_HOSTILE_LAST_AGGRO:
            target = SelectUnit(SELECT_TARGET_BOTTOMAGGRO, 0);
            break;
        case TARGET_T_HOSTILE_RANDOM:
            target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            break;
        case TARGET_T_HOSTILE_RANDOM_NOT_TOP:
            target = SelectUnit(SELECT_TARGET_RANDOM, 1);
            break;
        case TARGET_T_ACTION_INVOKER:
            return pActionInvoker;
        case TARGET_T_ACTION_INVOKER_NOT_PLAYER:
            return pActionInvoker->GetTypeId()==TYPEID_PLAYER ? NULL : pActionInvoker;
        case TARGET_T_SUMMONER:
        {
            if(m_creature->IsTemporarySummon())
            {
                if(TemporarySummon* tmp_summon = reinterpret_cast<TemporarySummon*>(m_creature))
                    return tmp_summon->GetSummoner();
            }
            return NULL;
        }
        case TARGET_T_EVENT_SPECIFIC:
            return pActionInvoker;
        case TARGET_T_HOSTILE_RANDOM_PLAYER_NOT_TOP:
            target = SelectUnit(SELECT_TARGET_RANDOM, 1, 0, true);
            break;
        case TARGET_T_EVENT_TARGET:
            if(m_eventTarget)
                return m_eventTarget;
        case TARGET_T_NULL:
        default:
            return NULL;
    };

    // ensure we have hostile creature (dont target self by accident)
    if (target && m_creature->IsFriendlyTo(target))
        return NULL;
    return target;
}

Unit* CreatureEventAI::SelectLowestHpFriendly(float range, uint32 MinHPDiff, bool targetSelf)
{
    Unit* pUnit = NULL;

    Hellground::MostHPMissingInRange u_check(m_creature, range, MinHPDiff, targetSelf);
    Hellground::UnitLastSearcher<Hellground::MostHPMissingInRange> searcher(pUnit, u_check);
    /*
    typedef TYPELIST_4(GameObject, Creature*except pets*, DynamicObject, Corpse*Bones*) AllGridObjectTypes;
    This means that if we only search grid then we cannot possibly return pets or players so this is safe
    */
    Cell::VisitGridObjects(m_creature, searcher, range);
    return pUnit;
}

void CreatureEventAI::FindFriendlyCC(std::list<Creature*>& _list, float range)
{
    Hellground::FriendlyCCedInRange u_check(m_creature, range);
    Hellground::ObjectListSearcher<Creature, Hellground::FriendlyCCedInRange> searcher(_list, u_check);

    Cell::VisitGridObjects(m_creature, searcher, range);
}

void CreatureEventAI::FindFriendlyMissingBuff(std::list<Creature*>& _list, float range, uint32 spellid)
{
    Hellground::FriendlyMissingBuffInRange u_check(m_creature, range, spellid);
    Hellground::ObjectListSearcher<Creature, Hellground::FriendlyMissingBuffInRange> searcher(_list, u_check);

    Cell::VisitGridObjects(m_creature,searcher, range);
}

//*********************************
//*** Functions used globally ***

void CreatureEventAI::DoScriptText(int32 textEntry, WorldObject* pSource, Unit* target)
{
    if (!pSource)
    {
        sLog.outLog(LOG_DB_ERR, "CreatureEventAI: DoScriptText entry %i, invalid Source pointer.",textEntry);
        return;
    }

    if (textEntry >= 0)
    {
        sLog.outLog(LOG_DB_ERR, "CreatureEventAI: DoScriptText with source entry %u (TypeId=%u, guid=%u) attempts to process text entry %i, but text entry must be negative.",pSource->GetEntry(),pSource->GetTypeId(),pSource->GetGUIDLow(),textEntry);
        return;
    }

    CreatureEventAI_TextMap::const_iterator i = sCreatureEAIMgr.GetCreatureEventAITextMap().find(textEntry);

    if (i == sCreatureEAIMgr.GetCreatureEventAITextMap().end())
    {
        sLog.outLog(LOG_DB_ERR, "CreatureEventAI: DoScriptText with source entry %u (TypeId=%u, guid=%u) could not find text entry %i.",pSource->GetEntry(),pSource->GetTypeId(),pSource->GetGUIDLow(),textEntry);
        return;
    }

    sLog.outDebug("CreatureEventAI: DoScriptText: text entry=%i, Sound=%u, Type=%u, Language=%u, Emote=%u",textEntry,(*i).second.SoundId,(*i).second.Type,(*i).second.Language,(*i).second.Emote);

    if ((*i).second.SoundId)
    {
        if (GetSoundEntriesStore()->LookupEntry((*i).second.SoundId))
            pSource->SendPlaySound((*i).second.SoundId,false);
        else
            sLog.outLog(LOG_DB_ERR, "CreatureEventAI: DoScriptText entry %i tried to process invalid sound id %u.",textEntry,(*i).second.SoundId);
    }

    if ((*i).second.Emote)
    {
        if (pSource->GetTypeId() == TYPEID_UNIT || pSource->GetTypeId() == TYPEID_PLAYER)
        {
            ((Unit*)pSource)->HandleEmoteCommand((*i).second.Emote);
        }
        else
            sLog.outLog(LOG_DB_ERR, "CreatureEventAI: DoScriptText entry %i tried to process emote for invalid TypeId (%u).",textEntry,pSource->GetTypeId());
    }

    switch ((*i).second.Type)
    {
        case CHAT_TYPE_SAY:
            pSource->MonsterSay(textEntry, (*i).second.Language, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_YELL:
            pSource->MonsterYell(textEntry, (*i).second.Language, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_TEXT_EMOTE:
            pSource->MonsterTextEmote(textEntry, target ? target->GetGUID() : 0);
            break;
        case CHAT_TYPE_BOSS_EMOTE:
            pSource->MonsterTextEmote(textEntry, target ? target->GetGUID() : 0, true);
            break;
        case CHAT_TYPE_WHISPER:
        {
            if (target && target->GetTypeId() == TYPEID_PLAYER)
                pSource->MonsterWhisper(textEntry, target->GetGUID());
            else sLog.outLog(LOG_DB_ERR, "CreatureEventAI: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", textEntry);
        }break;
        case CHAT_TYPE_BOSS_WHISPER:
        {
            if (target && target->GetTypeId() == TYPEID_PLAYER)
                pSource->MonsterWhisper(textEntry, target->GetGUID(), true);
            else sLog.outLog(LOG_DB_ERR, "CreatureEventAI: DoScriptText entry %i cannot whisper without target unit (TYPEID_PLAYER).", textEntry);
        }break;
        case CHAT_TYPE_ZONE_YELL:
            pSource->MonsterYellToZone(textEntry, (*i).second.Language, target ? target->GetGUID() : 0);
            break;
    }
}

bool CreatureEventAI::CanCast(Unit* Target, SpellEntry const *Spell, bool Triggered)
{
    //No target so we can't cast
    if (!Target || !Spell)
        return false;

    //Silenced so we can't cast
    if (!Triggered && me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return false;

    //Check for power
    if (!Triggered && me->GetPower((Powers)Spell->powerType) < SpellMgr::CalculatePowerCost(Spell, me, SpellMgr::GetSpellSchoolMask(Spell), NULL))
        return false;

    SpellRangeEntry const *TempRange = NULL;

    TempRange = GetSpellRangeStore()->LookupEntry(Spell->rangeIndex);

    //Spell has invalid range store so we can't use it
    if (!TempRange)
        return false;

    //Unit is out of range of this spell
    if (!m_creature->IsInRange(Target,TempRange->minRange,TempRange->maxRange))
        return false;

    if (!Triggered && !m_creature->IsWithinLOSInMap(Target))
        return false;

    return true;
}

void CreatureEventAI::ReceiveEmote(Player* pPlayer, uint32 text_emote)
{
    if (bEmptyList)
        return;

    for (std::list<CreatureEventAIHolder>::iterator itr = CreatureEventAIList.begin(); itr != CreatureEventAIList.end(); ++itr)
    {
        if ((*itr).Event.event_type == EVENT_T_RECEIVE_EMOTE)
        {
            if ((*itr).Event.receive_emote.emoteId != text_emote)
                return;

            sLog.outLog(LOG_TMP, "RC_DEBUG ReceiveEmote - Player %s condition %u", pPlayer->GetName(), m_creature->GetEntry());

            PlayerCondition pcon((*itr).Event.receive_emote.condition,(*itr).Event.receive_emote.conditionValue1,(*itr).Event.receive_emote.conditionValue2);
            if (pcon.Meets(pPlayer))
            {
                sLog.outDebug("CreatureEventAI: ReceiveEmote CreatureEventAI: Condition ok, processing");
                ProcessEvent(*itr, pPlayer);
            }
        }
    }
}

void CreatureEventAI::DamageTaken(Unit* done_by, uint32& damage)
{
    if (InvinceabilityHpLevel > 0 && m_creature->GetHealth() < InvinceabilityHpLevel+damage)
    {
        if (m_creature->GetHealth() <= InvinceabilityHpLevel)
            damage = 0;
        else
            damage = m_creature->GetHealth() - InvinceabilityHpLevel;
    }
}

bool CreatureEventAI::SpawnedEventConditionsCheck(CreatureEventAI_Event const& event)
{
    if (event.event_type != EVENT_T_SPAWNED)
        return false;

    switch (event.spawned.condition)
    {
        case SPAWNED_EVENT_ALWAY:
            // always
            return true;
        case SPAWNED_EVENT_MAP:
            // map ID check
            return m_creature->GetMapId() == event.spawned.conditionValue1;
        case SPAWNED_EVENT_ZONE:
            // zone ID check
            return m_creature->GetZoneId() == event.spawned.conditionValue1 || m_creature->GetAreaId() == event.spawned.conditionValue1;
        default:
            break;
    }

    return false;
}

void CreatureEventAI::GetDebugInfo(ChatHandler& reader)
{
    std::ostringstream str;
    str << "Debug info for EventAI of " << me->GetName() << "(" << me->GetEntry() << " : " << me->GetDBTableGUIDLow();
    str << ") consists of " << CreatureEventAIList.size() << " event entries\n";
    for (std::list<CreatureEventAIHolder>::iterator i = CreatureEventAIList.begin(); i != CreatureEventAIList.end(); ++i)
    {
        str << "Event " << i->Event.event_id << " : type " << i->Event.event_type << " timer " << i->Time;
        str << (i->Enabled ? " enabled\n" : " disabled\n");
    }

    reader.SendSysMessage(str.str().c_str());
}

void CreatureEventAI::SetCombatMove(bool enable, bool stopOrStartMovement /*=false*/)
{
    if (enable)
        CombatMovementEnabled = true;
    else
        CombatMovementEnabled = false;

    if (stopOrStartMovement && me->GetVictim())     // Only change current movement while in combat
    {
        if (!me->HasUnitState(UNIT_STAT_CAN_NOT_REACT))
        {
            if (enable)
                me->GetMotionMaster()->MoveChase(me->GetVictim(), 0, 0);
            else if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
                me->StopMoving();
        }
    }
}