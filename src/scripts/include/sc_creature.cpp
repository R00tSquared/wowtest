// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "precompiled.h"
#include "Item.h"
#include "Spell.h"
#include "ObjectMgr.h"

// Spell summary for ScriptedAI::SelectSpell
struct TSpellSummary
{
    uint8 Targets;                                          // set of enum SelectTarget
    uint8 Effects;                                          // set of enum SelectEffect
} *SpellSummary;

void SummonList::DoAction(uint32 entry, uint32 info) const
{
    for (const_iterator i = begin(); i != end(); ++i)
        if (Creature *summon = m_creature->GetCreature(*i))
            if (summon->IsAIEnabled && (!entry || summon->GetEntry() == entry))
                summon->AI()->DoAction(info);
}

void SummonList::Cast(uint32 entry, uint32 spell, Unit* target) const
{
    for (const_iterator i = begin(); i != end(); ++i)
        if (Creature *summon = m_creature->GetCreature(*i))
            if (!entry || summon->GetEntry() == entry)
                summon->CastSpell(target, spell, true);
}

void SummonList::Despawn(Creature *summon)
{
    erase(summon->GetGUID());
}

void SummonList::DespawnEntry(uint32 entry)
{
    for (iterator i = begin(); i != end(); )
    {
        if (Creature *summon = m_creature->GetCreature(*i))
        {
            if (summon->GetEntry() == entry)
            {
                summon->setDeathState(JUST_DIED);
                summon->RemoveCorpse();
                i = erase(i);
            }
            else
                ++i;
        }
        else
            i = erase(i);
    }
}

void SummonList::RemoveByEntry(uint32 entry)
{
    for (iterator i = begin(); i != end(); )
    {
        if (Creature *summon = m_creature->GetCreature(*i))
        {
            if (summon->GetEntry() == entry)
                i = erase(i);
            else
                ++i;
        }
        else
            i = erase(i);
    }
}

void SummonList::CastAuraOnEntry(uint32 entry, uint32 spellId, bool apply) const
{
    for (const_iterator i = begin(); i != end(); ++i)
    {
        if (Creature *summon = m_creature->GetCreature(*i))
        {
            if (summon->GetEntry() == entry)
            {
                if (apply)
                    summon->AddAura(spellId, summon);
                else
                    summon->RemoveAurasDueToSpell(spellId);
            }
        }
    }
}

void SummonList::DespawnAll()
{
    for (iterator i = begin(); i != end(); ++i)
    {
        if (Creature *summon = m_creature->GetCreature(*i))
        {
            summon->setDeathState(JUST_DIED);
            summon->RemoveCorpse();
        }
    }
    clear();
}

void SummonList::DespawnAllExcept(uint32 entry)
{
    for(iterator i = begin(); i != end(); ++i)
    {
        if(Creature *summon = Unit::GetCreature(*m_creature, *i))
        {
            if(summon->GetEntry() != entry)
            {
                summon->setDeathState(JUST_DIED);
                summon->RemoveCorpse();
                i = erase(i);
                --i;
            }
        }
    }
}

bool SummonList::isEmpty() const
{
    return empty();
}

ScriptedAI::ScriptedAI(Creature* pCreature) :
CreatureAI(pCreature), m_creature(pCreature), IsFleeing(false), m_bCombatMovement(true), m_uiEvadeCheckCooldown(2500),
autocast(false), m_specialThingTimer(1000)
{
    HeroicMode = m_creature->GetMap()->IsHeroic();
}

void ScriptedAI::AttackStartNoMove(Unit* pWho, movementCheckType type)
{
    if (!pWho)
        return;

    if (me->IsInEvadeMode())
        return;

    if(m_creature->Attack(pWho, false))
        DoStartNoMovement(pWho, type);
}

void ScriptedAI::AttackStart(Unit* pWho)
{
    if (!pWho)
        return;

    if (me->IsInEvadeMode())
        return;

    if (m_creature->Attack(pWho, true))
         DoStartMovement(pWho);
}

void ScriptedAI::AttackStart(Unit* pWho, bool melee)
{
    if (!pWho)
        return;

    if (!melee)
        AttackStartNoMove(pWho);
    else
        AttackStart(pWho);
}

void ScriptedAI::UpdateAI(const uint32 uiDiff)
{
    //Check if we have a current target
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}

void ScriptedAI::DoStartMovement(Unit* pVictim, float fDistance, float fAngle)
{
    if (pVictim)
        m_creature->GetMotionMaster()->MoveChase(pVictim, fDistance, fAngle);
}

void ScriptedAI::DoStartNoMovement(Unit* pVictim, movementCheckType type)
{
    if (!pVictim)
        return;

    switch(type)
    {
        case 1:
            me->SetWalk(false);
            casterTimer = 2000;
            break;
        case 2:
            me->SetWalk(false);
            casterTimer = 3000;
            break;
        default:
            break;
    }

    m_creature->GetMotionMaster()->StopControlledMovement();
}

void ScriptedAI::CheckCasterNoMovementInRange(uint32 diff, float maxrange)
{
    if (!UpdateVictim() || !me->GetVictim())
        return;

    if (!me->IsInMap(me->GetVictim()))
        return;

    if (casterTimer.GetTimeLeft() > 2000)  // just in case
        casterTimer.Reset(2000);


    if (casterTimer.Expired(diff))
    {
        if (me->HasUnitState(UNIT_STAT_CANNOT_AUTOATTACK))
        {
            casterTimer = 1000;
            return;
        }

        // go to victim
        if (!me->IsWithinDistInMap(me->GetVictim(), maxrange) || !me->IsWithinLOSInMap(me->GetVictim()))
        {
            float x, y, z;
            me->GetVictim()->GetPosition(x, y, z);
            me->UpdateAllowedPositionZ(x, y, z); // so we end up on the ground, and not on players jump pos
            me->SetSpeed(MOVE_RUN, 1.5);
            me->GetMotionMaster()->MovePoint(40, x, y, z);  //to not possibly collide with any Movement Inform check
            casterTimer = 200; // Trentone - isn't it too fast?
            return;
        }
        else
            me->GetMotionMaster()->StopControlledMovement();

        casterTimer = 2000;
    }
}

void ScriptedAI::CheckShooterNoMovementInRange(uint32 diff, float maxrange)
{
    if (!UpdateVictim() || !me->GetVictim())
        return;

    if (!me->IsInMap(me->GetVictim()))
        return;

    if (casterTimer.GetTimeLeft() > 3000)  // just in case
        casterTimer.Reset(3000);

    if (casterTimer.Expired(diff))
    {
        if (me->HasUnitState(UNIT_STAT_CANNOT_AUTOATTACK))
        {
            casterTimer = 1000;
            return;
        }

        // if victim in melee range, than chase it
        if (me->IsWithinDistInMap(me->GetVictim(), 5.0))
        {
            if (!me->HasUnitState(UNIT_STAT_CHASE))
                DoStartMovement(me->GetVictim());
            else
            {
                casterTimer = 3000;
                return;
            }
        }
        else if (me->HasUnitState(UNIT_STAT_CHASE))
            me->GetMotionMaster()->StopControlledMovement();

        // when victim is in distance, stop and shoot
        if (!me->IsWithinDistInMap(me->GetVictim(), maxrange) || !me->IsWithinLOSInMap(me->GetVictim()))
        {
            float x, y, z;
            me->GetVictim()->GetPosition(x, y, z);
            me->UpdateAllowedPositionZ(x, y, z); // so we end up on the ground, and not on players jump pos
            me->SetSpeed(MOVE_RUN, 1.5);
            me->GetMotionMaster()->MovePoint(41, x, y, z);  //to not possibly collide with any Movement Inform check
            casterTimer = 200; // Trentone - isn't it too fast?
            return;
        }
        else
            me->GetMotionMaster()->StopControlledMovement();

        casterTimer = 3000;
    }
}

void ScriptedAI::DoStopAttack()
{
    if (m_creature->GetVictim())
        m_creature->AttackStop();

}

Unit* ScriptedAI::SelectCastTarget(uint32 spellId, castTargetMode targetMode)
{
    switch (targetMode)
    {
        case CAST_TANK:
            return me->GetVictim();
        case CAST_NULL:
            return NULL;
        case CAST_RANDOM:
        case CAST_RANDOM_WITHOUT_TANK:
        {
            SpellEntry const* pSpell = GetSpellStore()->LookupEntry<SpellEntry>(spellId);
            return SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(spellId), pSpell->AttributesEx3 & SPELL_ATTR_EX3_PLAYERS_ONLY, targetMode == CAST_RANDOM_WITHOUT_TANK ? me->getVictimGUID() : 0);
        }
        case CAST_SELF:
            return me;
        case CAST_LOWEST_HP_FRIENDLY:
        {
            SpellEntry const* pSpell = GetSpellStore()->LookupEntry<SpellEntry>(spellId);
            return SelectLowestHpFriendly(GetSpellMaxRange(spellId));
        }
        case CAST_THREAT_SECOND:
        {
            SpellEntry const* pSpell = GetSpellStore()->LookupEntry<SpellEntry>(spellId);
            return SelectUnit(SELECT_TARGET_TOPAGGRO, 1, GetSpellMaxRange(spellId), pSpell->AttributesEx3 & SPELL_ATTR_EX3_PLAYERS_ONLY);
        }
        default:
            return NULL;
    };
}

void ScriptedAI::CastNextSpellIfAnyAndReady(uint32 diff)
{
    // clear spell list if caster isn't alive
    if (!m_creature->isAlive())
    {
        spellList.clear();
        autocast = false;
        return;
    }

    bool cast = false;

    if (m_creature->HasUnitState(UNIT_STAT_CASTING) || me->IsNonMeleeSpellCast(true))
        cast = true;

    if (!spellList.empty() && !cast)
    {
        SpellToCast temp(spellList.front());
        spellList.pop_front();

        // creature can't cast if lost control - moved here to drop spells if controlled instead not adding them to queue
        if (m_creature->isCrowdControlled() || m_creature->IsPolymorphed())
            return;

        if (!temp.spellId)
            return;

        if (temp.isDestCast)
        {
            m_creature->CastSpell(temp.castDest[0], temp.castDest[1], temp.castDest[2], temp.spellId, temp.triggered);
            cast = true;
            return;
        }

        if (temp.scriptTextEntry)
        {
            if (temp.targetGUID && temp.setAsTarget)
            {
                if (Unit* target = m_creature->GetUnit(temp.targetGUID))
                    DoScriptText(temp.scriptTextEntry, m_creature, target);
            }
            else
                DoScriptText(temp.scriptTextEntry, m_creature, m_creature->GetVictim());
        }

        if (temp.targetGUID)
        {
            Unit * tempU = m_creature->GetUnit(*m_creature, temp.targetGUID);

            if (tempU && tempU->IsInWorld() && tempU->isAlive() && tempU->IsInMap(m_creature))
                if (temp.spellId)
                {
                    if(temp.setAsTarget && !m_creature->hasIgnoreVictimSelection())
                        m_creature->SetSelection(temp.targetGUID);
                    if(temp.hasCustomValues)
                        m_creature->CastCustomSpell(tempU, temp.spellId, &temp.damage[0], &temp.damage[1], &temp.damage[2], temp.triggered);
                    else
                        m_creature->CastSpell(tempU, temp.spellId, temp.triggered);
                }
        }
        else
        {
            if(temp.hasCustomValues)
                m_creature->CastCustomSpell((Unit*)NULL, temp.spellId, &temp.damage[0], &temp.damage[1], &temp.damage[2], temp.triggered);
            else
                m_creature->CastSpell((Unit*)NULL, temp.spellId, temp.triggered);
        }

        cast = true;
    }

    if (autocast)
    {
        if (autocastTimer.Expired(diff))
        {
            if (!cast)
            {
                Unit * victim = NULL;

                switch (autocastMode)
                {
                    case CAST_TANK:
                    {
                        victim = m_creature->GetVictim();
                        // prevent from LoS exploiting, probably some general check should be implemented for this
                        uint8 i = 0;
                        SpellEntry const *spellInfo = GetSpellStore()->LookupEntry<SpellEntry>(autocastId);

                        // while (!victim or not in los) and i < threatlist size
                        while ((!victim || (!SpellMgr::SpellIgnoreLOS(spellInfo, 0) && !m_creature->IsWithinLOSInMap(victim))) && i < m_creature->getThreatManager().getThreatList().size())
                        {
                            ++i;
                            victim = SelectUnit(SELECT_TARGET_TOPAGGRO, i, GetSpellMaxRange(autocastId), true);
                        }
                        break;
                    }
                    case CAST_NULL:
                        m_creature->CastSpell((Unit*)NULL, autocastId, false);
                        break;
                    case CAST_RANDOM:
                        victim = SelectUnit(SELECT_TARGET_RANDOM, 0, autocastTargetRange, autocastTargetPlayer);
                        break;
                    case CAST_RANDOM_WITHOUT_TANK:
                        victim = SelectUnit(SELECT_TARGET_RANDOM, 1, autocastTargetRange, autocastTargetPlayer, m_creature->getVictimGUID());
                        break;
                    case CAST_SELF:
                        victim = m_creature;
                        break;
                    case CAST_THREAT_SECOND:
                        victim = SelectUnit(SELECT_TARGET_TOPAGGRO, 1, autocastTargetRange, autocastTargetPlayer);
                        break;
                    default:    //unsupported autocast, stop
                        {
                            autocast = false;
                            return;
                        }
                }

                if (victim && !m_creature->hasIgnoreVictimSelection())
                {
                    m_creature->SetSelection(victim->GetGUID());    // for autocast always target actual victim
                    m_creature->CastSpell(victim, autocastId, false);
                }

                autocastTimer = autocastTimerDef;
            }
        }
    }
}

void ScriptedAI::DoCast(Unit* victim, uint32 spellId, bool triggered)
{
    if (/*!victim || */m_creature->HasUnitState(UNIT_STAT_CASTING) && !triggered)
        return;

    //m_creature->StopMoving();
    m_creature->CastSpell(victim, spellId, triggered);
}

void ScriptedAI::DoCastAOE(uint32 spellId, bool triggered)
{
    if(!triggered && m_creature->HasUnitState(UNIT_STAT_CASTING))
        return;

    m_creature->CastSpell((Unit*)NULL, spellId, triggered);
}

void ScriptedAI::DoCastSpell(Unit* who,SpellEntry const *spellInfo, bool triggered)
{
    if (/*!who || */m_creature->IsNonMeleeSpellCast(false))
        return;

    m_creature->CastSpell(who, spellInfo, triggered);
}

void ScriptedAI::AddSpellToCast(Unit* victim, uint32 spellId, bool triggered, bool visualTarget)
{
    SpellToCast temp(victim ? victim->GetGUID() : 0, spellId, triggered, 0, visualTarget);

    spellList.push_back(temp);
}

void ScriptedAI::AddCustomSpellToCast(Unit* victim, uint32 spellId, int32 dmg0, int32 dmg1, int32 dmg2, bool triggered, bool visualTarget)
{
    SpellToCast temp(victim ? victim->GetGUID() : 0, spellId, dmg0, dmg1, dmg2, triggered, 0, visualTarget);

    spellList.push_back(temp);
}

void ScriptedAI::AddSpellToCast(float x, float y, float z, uint32 spellId, bool triggered, bool visualTarget)
{
    SpellToCast temp(x, y, z, spellId, triggered, 0, visualTarget);

    spellList.push_back(temp);
}

void ScriptedAI::AddSpellToCastWithScriptText(Unit* victim, uint32 spellId, int32 scriptTextEntry, bool triggered, bool visualTarget)
{
    SpellToCast temp(victim ? victim->GetGUID() : 0, spellId, triggered, scriptTextEntry, visualTarget);

    spellList.push_back(temp);
}

void ScriptedAI::AddSpellToCast(uint32 spellId, castTargetMode targetMode, bool triggered, bool visualTarget)
{
    Unit *pTarget = SelectCastTarget(spellId, targetMode);
    if (!pTarget && targetMode != CAST_NULL)
        return;

    uint64 targetGUID = pTarget ? pTarget->GetGUID() : 0;
    SpellToCast temp(targetGUID, spellId, triggered, 0, visualTarget);

    spellList.push_back(temp);
}

void ScriptedAI::AddCustomSpellToCast(uint32 spellId, castTargetMode targetMode, int32 dmg0, int32 dmg1, int32 dmg2, bool triggered)
{
    Unit *pTarget = SelectCastTarget(spellId, targetMode);
    if (!pTarget && targetMode != CAST_NULL)
        return;

    uint64 targetGUID = pTarget ? pTarget->GetGUID() : 0;
    SpellToCast temp(targetGUID, spellId, dmg0, dmg1, dmg2, triggered, 0, false);

    spellList.push_back(temp);
}

void ScriptedAI::AddSpellToCastWithScriptText(uint32 spellId, castTargetMode targetMode, int32 scriptTextEntry, bool triggered, bool visualTarget)
{
    Unit *pTarget = SelectCastTarget(spellId, targetMode);
    if (!pTarget && targetMode != CAST_NULL)
        return;

    uint64 targetGUID = pTarget ? pTarget->GetGUID() : 0;
    SpellToCast temp(targetGUID, spellId, triggered, scriptTextEntry, visualTarget);

    spellList.push_back(temp);
}

void ScriptedAI::ForceSpellCast(Unit *victim, uint32 spellId, interruptSpell interruptCurrent, bool triggered, bool visualTarget)
{
    switch (interruptCurrent)
    {
        case INTERRUPT_AND_CAST:
            m_creature->InterruptNonMeleeSpells(false);
            break;
        case INTERRUPT_AND_CAST_INSTANTLY:
            if(visualTarget && !m_creature->hasIgnoreVictimSelection())
                m_creature->SetSelection(victim->GetGUID());

            m_creature->CastSpell(victim, spellId, triggered);
            return;
        default:
            break;
    }

    SpellToCast temp(victim ? victim->GetGUID() : 0, spellId, triggered, 0, visualTarget);

    spellList.push_front(temp);
}

void ScriptedAI::ForceSpellCastWithScriptText(Unit *victim, uint32 spellId, int32 scriptTextEntry, interruptSpell interruptCurrent, bool triggered, bool visualTarget)
{
    switch(interruptCurrent)
    {
        case INTERRUPT_AND_CAST:
            m_creature->InterruptNonMeleeSpells(false);
            break;
        case INTERRUPT_AND_CAST_INSTANTLY:
            if (scriptTextEntry)
                DoScriptText(scriptTextEntry, m_creature, victim);

            if (visualTarget && !m_creature->hasIgnoreVictimSelection())
                m_creature->SetSelection(victim->GetGUID());

            m_creature->CastSpell(victim, spellId, triggered);
            return;
        default:
            break;
    }

    SpellToCast temp(victim ? victim->GetGUID() : 0, spellId, triggered, scriptTextEntry, visualTarget);

    spellList.push_front(temp);
}

void ScriptedAI::ForceSpellCast(uint32 spellId, castTargetMode targetMode, interruptSpell interruptCurrent, bool triggered)
{
    Unit *pTarget = SelectCastTarget(spellId, targetMode);
    if (!pTarget && targetMode != CAST_NULL)
        return;

    uint64 targetGUID = pTarget ? pTarget->GetGUID() : 0;

    switch (interruptCurrent)
    {
        case INTERRUPT_AND_CAST:
            m_creature->InterruptNonMeleeSpells(false);
            break;
        case INTERRUPT_AND_CAST_INSTANTLY:
            m_creature->CastSpell(pTarget, spellId, triggered);
            return;
        default:
            break;
    }

    SpellToCast temp(targetGUID, spellId, triggered, 0, false);

    spellList.push_front(temp);
}

void ScriptedAI::ForceSpellCastWithScriptText(uint32 spellId, castTargetMode targetMode, int32 scriptTextEntry, interruptSpell interruptCurrent, bool triggered)
{
    Unit *pTarget = SelectCastTarget(spellId, targetMode);
    if (!pTarget && targetMode != CAST_NULL)
        return;

    uint64 targetGUID = pTarget ? pTarget->GetGUID() : 0;

    switch (interruptCurrent)
    {
        case INTERRUPT_AND_CAST:
            m_creature->InterruptNonMeleeSpells(false);
            break;
        case INTERRUPT_AND_CAST_INSTANTLY:
            if (m_creature->GetVictim() && scriptTextEntry)
                DoScriptText(scriptTextEntry, m_creature, m_creature->GetVictim());

            m_creature->CastSpell(pTarget, spellId, triggered);
            return;
        default:
            break;
    }

    SpellToCast temp(targetGUID, spellId, triggered, scriptTextEntry, false);

    spellList.push_front(temp);
}

void ScriptedAI::SetAutocast(uint32 spellId, uint32 timer, bool startImmediately, castTargetMode mode, uint32 range, bool player)
{
    if (!spellId)
        return;

    autocastId = spellId;

    autocastTimer = timer;

    if (startImmediately)
        autocastTimer.SetCurrent(timer);

    autocastTimerDef = timer;

    autocastMode = mode;
    autocastTargetRange = range;
    autocastTargetPlayer = player;

    autocast = startImmediately;
}

void ScriptedAI::RemoveFromCastQueue(uint32 spellId)
{
    if (!spellId || spellList.empty())
        return;

    for (std::list<SpellToCast>::iterator itr = spellList.begin(); itr != spellList.end(); )
    {
        std::list<SpellToCast>::iterator tmpItr = itr;
        itr++;
        if ((*tmpItr).spellId == spellId)
            spellList.erase(tmpItr);
    }
}

void ScriptedAI::RemoveFromCastQueue(uint64 targetGUID)
{
    if (!targetGUID || spellList.empty())
        return;

    for (std::list<SpellToCast>::iterator itr = spellList.begin(); itr != spellList.end(); )
    {
        std::list<SpellToCast>::iterator tmpItr = itr;
        itr++;
        if ((*tmpItr).targetGUID == targetGUID)
            spellList.erase(tmpItr);
    }
}

void ScriptedAI::DoSay(const char* text, uint32 language, Unit* target, bool SayEmote)
{
    if (target)
    {
        m_creature->Say(text, language, target->GetGUID());
        if(SayEmote)
            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
    }
    else m_creature->Say(text, language, 0);
}

void ScriptedAI::DoYell(const char* text, uint32 language, Unit* target)
{
    if (target) m_creature->Yell(text, language, target->GetGUID());
    else m_creature->Yell(text, language, 0);
}

void ScriptedAI::DoTextEmote(const char* text, Unit* target, bool IsBossEmote)
{
    if (target) m_creature->TextEmote(text, target->GetGUID(), IsBossEmote);
    else m_creature->TextEmote(text, 0, IsBossEmote);
}

void ScriptedAI::DoWhisper(const char* text, Unit* reciever, bool IsBossWhisper)
{
    if (!reciever || reciever->GetTypeId() != TYPEID_PLAYER)
        return;

    m_creature->Whisper(text, reciever->GetGUID(), IsBossWhisper);
}

void ScriptedAI::DoSay(int32 text, uint32 language, Unit* target, bool SayEmote)
{
	if (target)
	{
		m_creature->Say(text, language, target->GetGUID());
		if (SayEmote)
			m_creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
	}
	else m_creature->Say(text, language, 0);
}

void ScriptedAI::DoYell(int32 text, uint32 language, Unit* target)
{
	if (target) m_creature->Yell(text, language, target->GetGUID());
	else m_creature->Yell(text, language, 0);
}

void ScriptedAI::DoTextEmote(int32 text, Unit* target, bool IsBossEmote)
{
	if (target) m_creature->TextEmote(text, target->GetGUID(), IsBossEmote);
	else m_creature->TextEmote(text, 0, IsBossEmote);
}

void ScriptedAI::DoWhisper(int32 text, Unit* reciever, bool IsBossWhisper)
{
	if (!reciever || reciever->GetTypeId() != TYPEID_PLAYER)
		return;

	m_creature->Whisper(text, reciever->GetGUID(), IsBossWhisper);
}

void ScriptedAI::DoPlaySoundToSet(WorldObject* unit, uint32 sound)
{
    if (!unit)
        return;

    if (!GetSoundEntriesStore()->LookupEntry(sound))
    {
        error_log("TSCR: Invalid soundId %u used in DoPlaySoundToSet (by unit TypeId %u, guid %llu)", sound, unit->GetTypeId(), unit->GetGUID());
        return;
    }

    WorldPacket data(Opcodes(4));
    data.SetOpcode(SMSG_PLAY_SOUND);
    data << uint32(sound);
    unit->BroadcastPacket(&data,false);
}

Creature* ScriptedAI::DoSpawnCreature(uint32 id, float x, float y, float z, float angle, uint32 type, uint32 despawntime)
{
    return m_creature->SummonCreature(id,m_creature->GetPositionX() + x,m_creature->GetPositionY() + y,m_creature->GetPositionZ() + z, angle, (TemporarySummonType)type, despawntime);
}

SpellEntry const* ScriptedAI::SelectSpell(Unit* pTarget, int32 uiSchool, int32 uiMechanic, SelectTargetType selectTargets, uint32 uiPowerCostMin, uint32 uiPowerCostMax, float fRangeMin, float fRangeMax, SelectEffect selectEffects)
{
    //No target so we can't cast
    if (!pTarget)
        return NULL;

    //Silenced so we can't cast
    if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
        return NULL;

    //Using the extended script system we first create a list of viable spells
    SpellEntry const* apSpell[CREATURE_MAX_SPELLS];
    memset(apSpell, 0, sizeof(SpellEntry*)*CREATURE_MAX_SPELLS);

    uint32 uiSpellCount = 0;

    SpellEntry const* pTempSpell;
    SpellRangeEntry const* pTempRange;

    //Check if each spell is viable(set it to null if not)
    for (uint32 i = 0; i < CREATURE_MAX_SPELLS; i++)
    {
        pTempSpell = GetSpellStore()->LookupEntry<SpellEntry>(m_creature->m_spells[i]);

        //This spell doesn't exist
        if (!pTempSpell)
            continue;

        // Targets and Effects checked first as most used restrictions
        //Check the spell targets if specified
        if (selectTargets && !(SpellSummary[m_creature->m_spells[i]].Targets & (1 << (selectTargets-1))))
            continue;

        //Check the type of spell if we are looking for a specific spell type
        if (selectEffects && !(SpellSummary[m_creature->m_spells[i]].Effects & (1 << (selectEffects-1))))
            continue;

        //Check for school if specified
        if (uiSchool >= 0 && pTempSpell->SchoolMask & uiSchool)
            continue;

        //Check for spell mechanic if specified
        if (uiMechanic >= 0 && pTempSpell->Mechanic != uiMechanic)
            continue;

        //Make sure that the spell uses the requested amount of power
        if (uiPowerCostMin && pTempSpell->manaCost < uiPowerCostMin)
            continue;

        if (uiPowerCostMax && pTempSpell->manaCost > uiPowerCostMax)
            continue;

        //Continue if we don't have the mana to actually cast this spell
        if (pTempSpell->manaCost > m_creature->GetPower((Powers)pTempSpell->powerType))
            continue;

        //Get the Range
        pTempRange = GetSpellRangeStore()->LookupEntry(pTempSpell->rangeIndex);

        //Spell has invalid range store so we can't use it
        if (!pTempRange)
            continue;

        //Check if the spell meets our range requirements
        if (fRangeMin && pTempRange->maxRange < fRangeMin)
            continue;

        if (fRangeMax && pTempRange->maxRange > fRangeMax)
            continue;

        //Check if our target is in range
        if (m_creature->IsWithinDistInMap(pTarget, pTempRange->minRange) || !m_creature->IsWithinDistInMap(pTarget, pTempRange->maxRange))
            continue;

        //All good so lets add it to the spell list
        apSpell[uiSpellCount] = pTempSpell;
        ++uiSpellCount;
    }

    //We got our usable spells so now lets randomly pick one
    if (!uiSpellCount)
        return NULL;

    return apSpell[rand()%uiSpellCount];
}

float ScriptedAI::GetSpellMaxRange(uint32 id)
{
    SpellEntry const *spellInfo = GetSpellStore()->LookupEntry<SpellEntry>(id);
    if(!spellInfo)
        return 0;

    SpellRangeEntry const *range = GetSpellRangeStore()->LookupEntry(spellInfo->rangeIndex);
    if(!range)
        return 0;
    return range->maxRange;
}

void FillSpellSummary()
{
    SpellSummary = new TSpellSummary[GetSpellStore()->GetMaxEntry()];

    SpellEntry const* TempSpell;

    for (int i=0; i < GetSpellStore()->GetMaxEntry(); i++ )
    {
        SpellSummary[i].Effects = 0;
        SpellSummary[i].Targets = 0;

        TempSpell = GetSpellStore()->LookupEntry<SpellEntry>(i);
        //This spell doesn't exist
        if (!TempSpell)
            continue;

        for (int j=0; j<3; j++)
        {
            //Spell targets self
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SELF-1);

            //Spell targets a single enemy
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ENEMY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DST_TARGET_ENEMY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_ENEMY-1);

            //Spell targets AoE at enemy
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_SRC ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_DST ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DEST_DYNOBJ_ENEMY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_ENEMY-1);

            //Spell targets an enemy
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ENEMY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DST_TARGET_ENEMY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_SRC ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_AREA_ENEMY_DST ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_DEST_DYNOBJ_ENEMY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_ENEMY-1);

            //Spell targets a single friend(or self)
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ALLY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_PARTY )
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_SINGLE_FRIEND-1);

            //Spell targets aoe friends
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_TARGET ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_AOE_FRIEND-1);

            //Spell targets any friend(or self)
            if ( TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_ALLY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_TARGET_PARTY ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_CASTER ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_UNIT_PARTY_TARGET ||
                TempSpell->EffectImplicitTargetA[j] == TARGET_SRC_CASTER)
                SpellSummary[i].Targets |= 1 << (SELECT_TARGET_ANY_FRIEND-1);

            //Make sure that this spell includes a damage effect
            if ( TempSpell->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE ||
                TempSpell->Effect[j] == SPELL_EFFECT_INSTAKILL ||
                TempSpell->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE ||
                TempSpell->Effect[j] == SPELL_EFFECT_HEALTH_LEECH )
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_DAMAGE-1);

            //Make sure that this spell includes a healing effect (or an apply aura with a periodic heal)
            if ( TempSpell->Effect[j] == SPELL_EFFECT_HEAL ||
                TempSpell->Effect[j] == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                TempSpell->Effect[j] == SPELL_EFFECT_HEAL_MECHANICAL ||
                (TempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA  && TempSpell->EffectApplyAuraName[j]== 8 ))
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_HEALING-1);

            //Make sure that this spell applies an aura
            if ( TempSpell->Effect[j] == SPELL_EFFECT_APPLY_AURA )
                SpellSummary[i].Effects |= 1 << (SELECT_EFFECT_AURA-1);
        }
    }
}

void ScriptedAI::DoResetThreat()
{
    if (!m_creature->CanHaveThreatList() || m_creature->getThreatManager().isThreatListEmpty())
    {
        error_log("TSCR: DoResetThreat called for creature that either cannot have threat list or has empty threat list (m_creature entry = %d)", m_creature->GetEntry());

        return;
    }

    std::list<HostileReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
    std::list<HostileReference*>::iterator itr;

    for(itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
    {
        Unit* pUnit = NULL;
        pUnit = Unit::GetUnit((*m_creature), (*itr)->getUnitGuid());
        if(pUnit && DoGetThreat(pUnit))
            DoModifyThreatPercent(pUnit, -100);
    }
}

float ScriptedAI::DoGetThreat(Unit* pUnit)
{
    if(!pUnit) return 0.0f;
    return m_creature->getThreatManager().getThreat(pUnit);
}

void ScriptedAI::DoModifyThreatPercent(Unit *pUnit, int32 pct)
{
    if(!pUnit) return;
    m_creature->getThreatManager().modifyThreatPercent(pUnit, pct);
}

void ScriptedAI::DoTeleportTo(float x, float y, float z, uint32 time)
{
    if (time)
    {
        float speed = me->GetDistance(x, y, z) / ((float)time * 0.001f);
        me->MonsterMoveWithSpeed(x, y, z, speed);
    }
    else
        me->NearTeleportTo(x, y, z, me->GetOrientation(), false);
}

void ScriptedAI::DoTeleportPlayer(Unit* pUnit, float x, float y, float z, float o)
{
    if(!pUnit || pUnit->GetTypeId() != TYPEID_PLAYER)
    {
        if(pUnit)
            error_log("TSCR: Creature %llu (Entry: %u) Tried to teleport non-player unit (Type: %u GUID: %llu) to x: %f y:%f z: %f o: %f. Aborted.", m_creature->GetGUID(), m_creature->GetEntry(), pUnit->GetTypeId(), pUnit->GetGUID(), x, y, z, o);
        return;
    }

    ((Player*)pUnit)->TeleportTo(pUnit->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}

void ScriptedAI::DoTeleportAll(float x, float y, float z, float o)
{
    Map *map = m_creature->GetMap();
    if (!map->IsDungeon())
        return;

    Map::PlayerList const &PlayerList = map->GetPlayers();
    for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        if (Player* i_pl = i->getSource())
            if (i_pl->isAlive())
                i_pl->TeleportTo(m_creature->GetMapId(), x, y, z, o, TELE_TO_NOT_LEAVE_COMBAT);
}

Unit* FindCreature(uint32 entry, float range, Unit* Finder)
{
    if(!Finder)
        return NULL;

    Creature* target = NULL;
    Hellground::AllCreaturesOfEntryInRange check(Finder, entry, range);
    Hellground::ObjectSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(target, check);

    Cell::VisitAllObjects(Finder, searcher, range);
    return target;
}

GameObject* FindGameObject(uint32 entry, float range, Unit* Finder)
{
    if(!Finder)
        return NULL;
    GameObject* target = NULL;
    Hellground::AllGameObjectsWithEntryInGrid go_check(entry);
    Hellground::ObjectSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher(target, go_check);
    Cell::VisitGridObjects(Finder, searcher, range);
    return target;
}

void ScriptedAI::SetEquipmentSlots(bool bLoadDefault, int32 uiMainHand, int32 uiOffHand, int32 uiRanged)
{
    if (bLoadDefault)
    {
        if (CreatureInfo const* pInfo = GetCreatureTemplateStore(m_creature->GetEntry()))
            m_creature->LoadEquipment(pInfo->equipmentId,true);

        return;
    }

    if (uiMainHand >= 0)
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 0, uint32(uiMainHand));

    if (uiOffHand >= 0)
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1, uint32(uiOffHand));

    if (uiRanged >= 0)
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2, uint32(uiRanged));
}

void ScriptedAI::SetCombatMovement(bool bCombatMove)
{
    m_bCombatMovement = bCombatMove;
}

void Scripted_NoMovementAI::AttackStart(Unit* pWho)
{
    if (!pWho)
        return;

    if (m_creature->Attack(pWho, true))
        DoStartNoMovement(pWho);
}

void ScriptedAI::MultiboxCheck()
{
    std::list<Player*> playerList = FindAllPlayersInRange(50);
    std::map<std::string, uint8> IP_count;

    if (!playerList.empty())
    {
        for (std::list<Player*>::iterator i = playerList.begin(); i != playerList.end(); i++)
        {
            WorldSession *session = (*i)->GetSession();
            if (session)
            {
                // kill instantly all players over >2 same ips limit
                if (IP_count.find(session->GetRemoteAddress()) != IP_count.end())
                    if (IP_count[session->GetRemoteAddress()] > 2)
                        (*i)->Kill((*i));
                    else
                        ++IP_count[session->GetRemoteAddress()];
                else
                    IP_count.insert(std::make_pair(session->GetRemoteAddress(), 1));
            }
        }
    }

    IP_count.clear();
}

void ScriptedAI::DoSpecialThings(uint32 diff, SpecialThing flags, float range, float speedRate)
{
    if (m_specialThingTimer.Expired(diff))
    {
        if (flags & DO_PULSE_COMBAT)
            DoZoneInCombat(range);
        
        // dima: bugged 18.02.25
        //if (flags & DO_SPEED_UPDATE)
        //{
        //    me->SetSpeed(MOVE_WALK, speedRate, true);
        //    me->SetSpeed(MOVE_RUN, speedRate, true);
        //}

        if (flags & DO_EVADE_CHECK)
        {
            // need to call SetHomePosition ?
            WorldLocation home = me->GetHomePosition();
            if (!me->IsWithinDistInMap(&home, range, true) || GetClosestPlayer(me, range) == NULL)
                EnterEvadeMode();
        }

        m_specialThingTimer = 1000;
    }
}

BossAI::BossAI(Creature *c, uint32 id) : ScriptedAI(c),
    bossId(id), summons(me), instance(c->GetInstanceData())
{
}

void BossAI::_Reset()
{
    events.Reset();
    summons.DespawnAll();
    if (instance)
        instance->SetBossState(bossId, NOT_STARTED);
}

void BossAI::_JustDied()
{
    events.Reset();
    summons.DespawnAll();
    if (instance)
        instance->SetBossState(bossId, DONE);
}

void BossAI::_EnterCombat()
{
    DoZoneInCombat();
    if (instance)
        instance->SetBossState(bossId, IN_PROGRESS);
}

void BossAI::JustSummoned(Creature *summon)
{
    summons.Summon(summon);
    summon->AI()->DoZoneInCombat();
}

void BossAI::SummonedCreatureDespawn(Creature *summon)
{
    summons.Despawn(summon);
}

/*
   Constructor for DialogueHelper

   @param   pDialogueArray The static const array of DialogueEntry holding the information about the dialogue. This array MUST be terminated by {0,0,0}
*/
DialogueHelper::DialogueHelper(DialogueEntry const* pDialogueArray) :
    m_pInstance(NULL),
    m_pDialogueArray(pDialogueArray),
    m_pCurrentEntry(NULL),
    m_pDialogueTwoSideArray(NULL),
    m_pCurrentEntryTwoSide(NULL),
    m_uiTimer(0),
    m_bIsFirstSide(true),
    m_bCanSimulate(false)
{}

/*
   Constructor for DialogueHelper (Two Sides)

   @param   pDialogueTwoSideArray The static const array of DialogueEntryTwoSide holding the information about the dialogue. This array MUST be terminated by {0,0,0,0,0}
*/
DialogueHelper::DialogueHelper(DialogueEntryTwoSide const* pDialogueTwoSideArray) :
    m_pInstance(NULL),
    m_pDialogueArray(NULL),
    m_pCurrentEntry(NULL),
    m_pDialogueTwoSideArray(pDialogueTwoSideArray),
    m_pCurrentEntryTwoSide(NULL),
    m_uiTimer(0),
    m_bIsFirstSide(true),
    m_bCanSimulate(false)
{}

/*
   Function to start a (part of a) dialogue

   @param   iTextEntry The TextEntry of the dialogue that will be started (must be always the entry of first side)
*/
void DialogueHelper::StartNextDialogueText(int32 iTextEntry)
{
    //sLog.outLog(LOG_DEFAULT, "DialogueHelper::Line 1185");
    // Find iTextEntry
    bool bFound = false;

    if (m_pDialogueArray)                                   // One Side
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1191");
        for (DialogueEntry const* pEntry = m_pDialogueArray; pEntry->iTextEntry; ++pEntry)
        {
            if (pEntry->iTextEntry == iTextEntry)
            {
                m_pCurrentEntry = pEntry;
                bFound = true;
                break;
            }
        }
    }
    else                                                    // Two Sides
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1204");
        for (DialogueEntryTwoSide const* pEntry = m_pDialogueTwoSideArray; pEntry->iTextEntry; ++pEntry)
        {
            if (pEntry->iTextEntry == iTextEntry)
            {
                m_pCurrentEntryTwoSide = pEntry;
                bFound = true;
                break;
            }
        }
    }

    if (!bFound)
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1218");
        return;
    }
    DoNextDialogueStep();
}

// Internal helper function to do the actual say of a DialogueEntry
void DialogueHelper::DoNextDialogueStep()
{
    //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1227");
    // Last Dialogue Entry done?
    if ((m_pCurrentEntry && !m_pCurrentEntry->iTextEntry) || (m_pCurrentEntryTwoSide && !m_pCurrentEntryTwoSide->iTextEntry))
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1231");
        m_uiTimer = 0;
        return;
    }

    // Get Text, SpeakerEntry and Timer
    int32 iTextEntry = 0;
    uint32 uiSpeakerEntry = 0;

    if (m_pDialogueArray)                               // One Side
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1242");
        uiSpeakerEntry = m_pCurrentEntry->uiSayerEntry;
        iTextEntry = m_pCurrentEntry->iTextEntry;

        m_uiTimer = m_pCurrentEntry->uiTimer;
    }
    else                                                // Two Sides
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1250");
        // Second Entries can be 0, if they are the entry from first side will be taken
        uiSpeakerEntry = !m_bIsFirstSide && m_pCurrentEntryTwoSide->uiSayerEntryAlt ? m_pCurrentEntryTwoSide->uiSayerEntryAlt : m_pCurrentEntryTwoSide->uiSayerEntry;
        iTextEntry = !m_bIsFirstSide && m_pCurrentEntryTwoSide->iTextEntryAlt ? m_pCurrentEntryTwoSide->iTextEntryAlt : m_pCurrentEntryTwoSide->iTextEntry;

        m_uiTimer = m_pCurrentEntryTwoSide->uiTimer;
    }

    // Simulate Case
    if (uiSpeakerEntry && iTextEntry < 0)
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1261");
        // Use Speaker if directly provided
        Creature* pSpeaker = GetSpeakerByEntry(uiSpeakerEntry);
        if (m_pInstance && !pSpeaker)                       // Get Speaker from instance
        {
            //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1266");
            /*if (m_bCanSimulate)                             // Simulate case
                m_pInstance->DoOrSimulateScriptTextForThisInstance(iTextEntry, uiSpeakerEntry);
            else*/
                pSpeaker = m_pInstance->GetCreatureById(uiSpeakerEntry);
                return;
        }

        if (pSpeaker)
        {
            //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1276");
            DoScriptText(iTextEntry, pSpeaker);
        }
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1279");
    }

    JustDidDialogueStep(m_pDialogueArray ?  m_pCurrentEntry->iTextEntry : m_pCurrentEntryTwoSide->iTextEntry);
    //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1283");
    // Increment position
    if (m_pDialogueArray)
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1287");
        ++m_pCurrentEntry;
    }
    else
    {
        //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1292");
        ++m_pCurrentEntryTwoSide;
    }
    //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1295");
}

// Call this function within any DialogueUpdate method. This is required for saying next steps in a dialogue
void DialogueHelper::DialogueUpdate(uint32 uiDiff)
{
    if (m_uiTimer)
    {
        if (m_uiTimer <= uiDiff)
        {
            //sLog.outLog(LOG_DEFAULT, "DialogueHelper::1305");
            DoNextDialogueStep();
        }
        else
            m_uiTimer -= uiDiff;
    }
}

Creature* GetClosestCreatureWithEntry(WorldObject* pSource, uint32 Entry, float MaxSearchRange, bool alive, bool inLoS)
{
    Creature *pCreature = NULL;
    Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*pSource, Entry, alive, MaxSearchRange, inLoS);
    Hellground::ObjectLastSearcher<Creature, Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

    Cell::VisitGridObjects(pSource, searcher, MaxSearchRange);
    return pCreature;
}

GameObject* GetClosestGameObjectWithEntry(WorldObject* source, uint32 entry, float maxSearchRange)
{
    GameObject *pGameObject = NULL;
    Hellground::NearestGameObjectEntryInObjectRangeCheck go_check(*source, entry, maxSearchRange);
    Hellground::ObjectLastSearcher<GameObject, Hellground::NearestGameObjectEntryInObjectRangeCheck> searcher(pGameObject, go_check);

    Cell::VisitGridObjects(source, searcher, maxSearchRange);
    return pGameObject;
}

class AnyAlivePlayerExceptGm
{
    public:
        AnyAlivePlayerExceptGm(WorldObject const* obj) : _obj(obj) {}
        bool operator()(Player* u)
        {
            return u->isAlive() && !u->isGameMaster();
        }

    private:
        WorldObject const* _obj;
};

Player* GetClosestPlayer(WorldObject* source, float maxSearchRange)
{
    Player* player = NULL;
    AnyAlivePlayerExceptGm check(source);
    Hellground::ObjectSearcher<Player, AnyAlivePlayerExceptGm> checker(player, check);

    Cell::VisitWorldObjects(source, checker, maxSearchRange);
    return player;
}
