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
#include "Log.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "QuestDef.h"
#include "Player.h"
#include "Creature.h"
#include "Spell.h"
#include "Group.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "InstanceData.h"
#include "ObjectAccessor.h"
#include "Formulas.h"
#include "Pet.h"
#include "Util.h"
#include "Totem.h"
#include "BattleGround.h"
#include "OutdoorPvP.h"
#include "PointMovementGenerator.h"
#include "InstanceSaveMgr.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "CreatureGroups.h"
#include "PetAI.h"
#include "PassiveAI.h"
#include "CreatureAI.h"
#include "VMapFactory.h"
#include "UnitAI.h"
#include "MovementGenerator.h"
#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"
#include "TemporarySummon.h"

#include <math.h>
#include "Chat.h"

float baseMoveSpeed[MAX_MOVE_TYPE] =
{
    2.5f,                                                   // MOVE_WALK
    7.0f,                                                   // MOVE_RUN
    4.5f,                                                   // MOVE_RUN_BACK
    4.722222f,                                              // MOVE_SWIM
    2.5f,                                                   // MOVE_SWIM_BACK
    3.141594f,                                              // MOVE_TURN_RATE
    7.0f,                                                   // MOVE_FLIGHT
    4.5f,                                                   // MOVE_FLIGHT_BACK
};

void InitTriggerAuraData();

// auraTypes contains attacker auras capable of proc'ing cast auras
static Unit::AuraTypeSet GenerateAttakerProcCastAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_DUMMY);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_SPELL);
    auraTypes.insert(SPELL_AURA_MOD_HASTE);
    auraTypes.insert(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    return auraTypes;
}

// auraTypes contains victim auras capable of proc'ing cast auras
static Unit::AuraTypeSet GenerateVictimProcCastAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_DUMMY);
    auraTypes.insert(SPELL_AURA_PRAYER_OF_MENDING);
    auraTypes.insert(SPELL_AURA_PRAYER_OF_MENDING_NPC);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_SPELL);
    return auraTypes;
}

// auraTypes contains auras capable of proc effect/damage (but not cast) for attacker
static Unit::AuraTypeSet GenerateAttakerProcEffectAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_MOD_DAMAGE_DONE);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_DAMAGE);
    auraTypes.insert(SPELL_AURA_MOD_CASTING_SPEED);
    auraTypes.insert(SPELL_AURA_MOD_RATING);
    return auraTypes;
}

// auraTypes contains auras capable of proc effect/damage (but not cast) for victim
static Unit::AuraTypeSet GenerateVictimProcEffectAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_MOD_RESISTANCE);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_DAMAGE);
    auraTypes.insert(SPELL_AURA_MOD_PARRY_PERCENT);
    auraTypes.insert(SPELL_AURA_MOD_BLOCK_PERCENT);
    auraTypes.insert(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    return auraTypes;
}

static Unit::AuraTypeSet attackerProcCastAuraTypes = GenerateAttakerProcCastAuraTypes();
static Unit::AuraTypeSet attackerProcEffectAuraTypes = GenerateAttakerProcEffectAuraTypes();

static Unit::AuraTypeSet victimProcCastAuraTypes = GenerateVictimProcCastAuraTypes();
static Unit::AuraTypeSet victimProcEffectAuraTypes   = GenerateVictimProcEffectAuraTypes();

// auraTypes contains auras capable of proc'ing for attacker and victim
static Unit::AuraTypeSet GenerateProcAuraTypes()
{
    InitTriggerAuraData();

    Unit::AuraTypeSet auraTypes;
    auraTypes.insert(attackerProcCastAuraTypes.begin(),attackerProcCastAuraTypes.end());
    auraTypes.insert(attackerProcEffectAuraTypes.begin(),attackerProcEffectAuraTypes.end());
    auraTypes.insert(victimProcCastAuraTypes.begin(),victimProcCastAuraTypes.end());
    auraTypes.insert(victimProcEffectAuraTypes.begin(),victimProcEffectAuraTypes.end());
    return auraTypes;
}

static Unit::AuraTypeSet procAuraTypes = GenerateProcAuraTypes();

bool IsPassiveStackableSpell(uint32 spellId)
{
    if (!SpellMgr::IsPassiveSpell(spellId))
        return false;

    SpellEntry const* spellProto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellProto)
        return false;

    for (int j = 0; j < 3; ++j)
    {
        if (procAuraTypes.find(Unit::AuraTypeSet::value_type(spellProto->EffectApplyAuraName[j])) != procAuraTypes.end())
            return false;
    }

    return true;
}

void MovementInfo::Read(ByteBuffer &data)
{
    data >> moveFlags;
    data >> moveFlags2;
    data >> time;
    data >> pos.x;
    data >> pos.y;
    data >> pos.z;
    data >> pos.o;

    if (HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        data >> t_guid;
        data >> t_pos.x;
        data >> t_pos.y;
        data >> t_pos.z;
        data >> t_pos.o;
        data >> t_time;
    }

    if (HasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)))
        data >> s_pitch;

    data >> fallTime;
    if (HasMovementFlag(MOVEFLAG_FALLING))
    {
        data >> j_velocity;
        data >> j_sinAngle;
        data >> j_cosAngle;
        data >> j_xyspeed;
    }

    if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data >> u_unk1;
}

void MovementInfo::Write(ByteBuffer &data) const
{
    data << moveFlags;
    data << moveFlags2;
    data << time;
    data << pos.x;
    data << pos.y;
    data << pos.z;
    data << pos.o;

    if (HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        data << t_guid;
        data << t_pos.x;
        data << t_pos.y;
        data << t_pos.z;
        data << t_pos.o;
        data << t_time;
    }

    if (HasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)))
        data << s_pitch;

    data << fallTime;

    if (HasMovementFlag(MOVEFLAG_FALLING))
    {
        data << j_velocity;
        data << j_sinAngle;
        data << j_cosAngle;
        data << j_xyspeed;
    }

    if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        data << u_unk1;
}


CastSpellEvent::CastSpellEvent(Unit& owner, uint64 target, uint32 spellId, int32* bp0, int32* bp1, int32* bp2, bool triggered, uint64 orginalCaster):
    BasicEvent(), m_owner(owner), m_target(target),  m_spellId(spellId), m_triggered(triggered), m_orginalCaster(orginalCaster), m_custom(true)
{
    if(bp0)
        m_values.AddSpellMod(SPELLVALUE_BASE_POINT0, *bp0);
    if(bp1)
        m_values.AddSpellMod(SPELLVALUE_BASE_POINT1, *bp1);
    if(bp2)
        m_values.AddSpellMod(SPELLVALUE_BASE_POINT2, *bp2);
}

bool CastSpellEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Unit *target = NULL;
    if(m_target)
    {
        target = m_owner.GetUnit(m_target);
        if(!target)
            return true;
    }

    if(m_custom)
        m_owner.CastCustomSpell(m_spellId, m_values, target, m_triggered, NULL, NULL, m_orginalCaster);
    else
        m_owner.CastSpell(target, m_spellId, m_triggered, NULL, NULL, m_orginalCaster);
    return true;
}

Unit::Unit() :
    WorldObject(), i_motionMaster(this), movespline(new Movement::MoveSpline()),
    _threatManager(this), _hostileRefManager(this), m_stateMgr(this),
    IsAIEnabled(false), NeedChangeAI(false), i_AI(NULL), i_disabledAI(NULL),
    m_procDeep(0), m_AI_locked(false), m_removedAurasCount(0), _lastDamagedTime(0)
{
    m_modAuras = new AuraList[TOTAL_AURAS];
    m_objectType |= TYPEMASK_UNIT;
    m_objectTypeId = TYPEID_UNIT;
                                                            // 2.3.2 - 0x70
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_LIVING | UPDATEFLAG_HAS_POSITION);

    m_attackTimer[BASE_ATTACK]   = 0;
    m_attackTimer[OFF_ATTACK]    = 0;
    m_attackTimer[RANGED_ATTACK] = 0;

    m_modAttackSpeedPct[BASE_ATTACK]   = 1.0f;
    m_modAttackSpeedPct[OFF_ATTACK]    = 1.0f;
    m_modAttackSpeedPct[RANGED_ATTACK] = 1.0f;

    m_extraAttacks = 0;
    m_canDualWield = false;

    m_state = 0;
    m_form = FORM_NONE;
    m_deathState = ALIVE;

    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
        m_currentSpells[i] = NULL;

    m_addDmgOnce = 0;

    for (uint8 i = 0; i < MAX_TOTEM; ++i)
        m_TotemSlot[i] = 0;

    m_ObjectSlot[0] = m_ObjectSlot[1] = m_ObjectSlot[2] = m_ObjectSlot[3] = 0;

    m_AurasUpdateIterator = m_Auras.end();
    m_Visibility = VISIBILITY_ON;

    m_interruptMask = 0;
    m_detectInvisibilityMask = 0;
    m_invisibilityMask = 0;
    m_transform = 0;
    m_ShapeShiftFormSpellId = 0;
    m_canModifyStats = false;

    for (uint8 i = 0; i < MAX_SPELL_IMMUNITY; ++i)
        m_spellImmune[i].clear();

    for (int i = 0; i < UNIT_MOD_END; ++i)
    {
        m_auraModifiersGroup[i][BASE_VALUE] = 0.0f;
        m_auraModifiersGroup[i][BASE_PCT] = 1.0f;
        m_auraModifiersGroup[i][TOTAL_VALUE] = 0.0f;
        m_auraModifiersGroup[i][TOTAL_PCT] = 1.0f;
    }

    // implement 50% base damage from offhand
    m_auraModifiersGroup[UNIT_MOD_DAMAGE_OFFHAND][TOTAL_PCT] = 0.5f;

    for (uint8 i = 0; i < MAX_ATTACK; i++)
    {
        m_weaponDamage[i][MINDAMAGE] = BASE_MINDAMAGE;
        m_weaponDamage[i][MAXDAMAGE] = BASE_MAXDAMAGE;
    }

    for (uint8 i = 0; i < MAX_STATS; ++i)
        m_createStats[i] = 0.0f;

    m_attacking = NULL;
    m_modMeleeHitChance = 0.0f;
    m_modRangedHitChance = 0.0f;
    m_modSpellHitChance = 0.0f;
    m_baseSpellCritChance = 5;

    m_CombatTimer = 0;
    m_lastManaUse = 0;

    for (uint8 i = 0; i < MAX_SPELL_SCHOOL; ++i)
        m_threatModifier[i] = 1.0f;

    m_isSorted = true;

    for (uint8 i = 0; i < MAX_MOVE_TYPE; ++i)
    {
        m_speed_rate[i] = 1.0f;
        //m_max_speed_rate[i] = 1.0f;
    }

    m_charmInfo = NULL;

    SetUnitMovementFlags(MOVEFLAG_NONE);
    m_reducedThreatPercent = 0;
    m_misdirectionTargetGUID = 0;

    // remove aurastates allowing special moves
    for (uint8 i = 0; i < MAX_REACTIVE; ++i)
        m_reactiveTimer[i] = 0;

    m_meleeAPAttackerBonus = 0;
    m_GMToSendCombatStats = 0;
    m_CombatStatsFlag = 0;

    _AINotifyScheduled = false;

    WorthHonor = false;
    m_lastSanctuaryTime = 0;

	bot_FakeGroup = false;

    m_custom = 0;
}

////////////////////////////////////////////////////////////
// Methods of class Unit
Unit::~Unit()
{
    // set current spells as deletable
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
    {
        if (m_currentSpells[i])
        {
            m_currentSpells[i]->SetReferencedFromCurrent(false);
            m_currentSpells[i] = NULL;
        }
    }

    RemoveAllGameObjects();
    RemoveAllDynObjects();
    _DeleteAuras();

    delete m_charmInfo;
    delete movespline;

    for (int i = 0; i < TOTAL_AURAS; i++)
    {
        while (!m_modAuras[i].empty())
        {
            delete m_modAuras[i].front();
            m_modAuras[i].pop_front();
        }
    }

    delete [] m_modAuras;

    ASSERT(!m_attacking);
    ASSERT(m_attackers.empty());
}

EventProcessor* Unit::GetEvents()
{
    return &m_Events;
}

void Unit::KillAllEvents(bool force)
{
    //MAPLOCK_WRITE(this, MAP_LOCK_TYPE_DEFAULT);
    GetEvents()->KillAllEvents(force);
}

void Unit::AddEvent(BasicEvent* Event, uint64 e_time, bool set_addtime)
{
    //MAPLOCK_WRITE(this, MAP_LOCK_TYPE_DEFAULT);
    if (set_addtime)
        GetEvents()->AddEvent(Event, GetEvents()->CalculateTime(e_time), set_addtime);
    else
        GetEvents()->AddEvent(Event, e_time, set_addtime);
}

void Unit::UpdateEvents(uint32 update_diff, uint32 time)
{
    /*{
        MAPLOCK_READ(this, MAP_LOCK_TYPE_DEFAULT);
        GetEvents()->RenewEvents();
    }*/

    GetEvents()->Update(update_diff);
}

void Unit::Update(uint32 update_diff, uint32 p_time)
{
    // WARNING! Order of execution here is important, do not change.
    // Spells must be processed with event system BEFORE they go to _UpdateSpells.
    // Or else we may have some SPELL_STATE_FINISHED spells stalled in pointers, that is bad.

    UpdateEvents(update_diff, p_time);

	if (sWorld.getConfig(CONFIG_BOT_SKIP_UPDATES) && GetTypeId() == TYPEID_PLAYER && ((Player*)this)->GetSession()->isFakeBot())
		return;

    if (!IsInWorld())
        return;

    _UpdateSpells(update_diff);

    if (!HasUnitState(UNIT_STAT_CANNOT_AUTOATTACK))
    {
        if (uint32 base_att = getAttackTimer(BASE_ATTACK))
            setAttackTimer(BASE_ATTACK, (update_diff >= base_att ? 0 : base_att - update_diff));

        if (uint32 off_att = getAttackTimer(OFF_ATTACK))
            setAttackTimer(OFF_ATTACK, (update_diff >= off_att ? 0 : off_att - update_diff));
    }

    if (uint32 ranged_att = getAttackTimer(RANGED_ATTACK))
        setAttackTimer(RANGED_ATTACK, (update_diff >= ranged_att ? 0 : ranged_att - update_diff));

    // update abilities available only for fraction of time
    UpdateReactives(update_diff);

    ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, GetHealth()*100 < GetMaxHealth()*20);
    ModifyAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, GetHealth()*100 < GetMaxHealth()*35);

    UpdateSplineMovement(p_time);
    GetUnitStateMgr().Update(p_time);
}

bool Unit::haveOffhandWeapon() const
{
    if (GetTypeId() == TYPEID_PLAYER)
        return ((Player*)this)->GetWeaponForAttack(OFF_ATTACK,true);
    else
        return m_canDualWield;
}

void Unit::MonsterMove(float x, float y, float z)
{
    Movement::MoveSplineInit init(*this);
    init.MoveTo(x, y, z, true, false);
    init.Launch();
}

void Unit::MonsterMoveWithSpeed(float x, float y, float z, float speed, bool time, bool generatePath, bool forceDestination)
{
    if (time)
        speed = GetDistance(x, y, z) / ((float)speed * 0.001f);

    Movement::MoveSplineInit init(*this);
    init.MoveTo(x,y,z, generatePath, forceDestination);
    if (speed)
        init.SetVelocity(speed);
    init.Launch();
}

void Unit::SendMonsterStop()
{
    WorldPacket data(SMSG_MONSTER_MOVE, (17 + GetPackGUID().size()));
    data << GetPackGUID();
    data << GetPositionX() << GetPositionY() << GetPositionZ();
    data << WorldTimer::getMSTime();
    data << uint8(1);
    BroadcastPacket(&data, true);
    SendCombatStats(1 << COMBAT_STATS_TEST, "monster stop", NULL);
}

bool Unit::HasMorePowerfulBuff(Unit* caster, SpellEntry const* newSpellInfo)
{
    // can be no caster -> then check calculatesimplevalue

    // cannot be no newSpellInfo
    if (!newSpellInfo)
        return false;

    // first is auraType, second is miscvalueA and miscvalueB
    std::pair<uint32, std::pair<uint32, uint32>> spellAuraNameThis[3];
    std::pair<uint32, std::pair<uint32, uint32>> spellAuraNameFound[3];
    uint8 AuraCountThis = 0;
    uint8 AuraCountFound;
    uint8 findIndex = 3;
    for (uint8 i = 0; i < 3; ++i)
    {
        if (newSpellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && newSpellInfo->EffectApplyAuraName[i])
        {
            spellAuraNameThis[i].first = newSpellInfo->EffectApplyAuraName[i];
            spellAuraNameThis[i].second.first = newSpellInfo->EffectMiscValue[i];
            spellAuraNameThis[i].second.second = newSpellInfo->EffectMiscValueB[i];
            findIndex = AuraCountThis;
            ++AuraCountThis;
        }
        else
        {
            spellAuraNameThis[i].first = 0;
            spellAuraNameThis[i].second.first = 0;
            spellAuraNameThis[i].second.second = 0;
        }
    }

    if (!AuraCountThis)
        return false; // newSpellInfo has no auras at all

    Unit::AuraList const& auras = GetAurasByType(AuraType(spellAuraNameThis[findIndex].first));
    if (auras.empty())
    // if this happened - then target has no such type of aura that newSpellInfo is -> this means that new spell has aura that old did not have for sure
        return false;

    uint64 casterGUID = caster ? caster->GetGUID() : 0;

    std::set<std::pair<uint32, uint64> > aurasDone;
    // so now we have some auras - we must check are they same in comparison with our newSpellInfo
    for (AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura* aura = *itr;
        if (!aura)
            continue;

        std::pair<uint32, uint64> auraPair(aura->GetId(), aura->GetCasterGUID());
        // prevent rolling twice for two effects of the same spell
        if(aurasDone.find(auraPair) != aurasDone.end())
            continue;

        aurasDone.insert(auraPair); // do not check same id and caster spell, cause we check by spellInfo+all aura effects at once anyway.

        if (aura->IsPassive())
        // this means this aura spell is not newSpellInfo like for sure
            continue;

        SpellEntry const* foundSpellInfo = aura->GetSpellProto();

        if (!foundSpellInfo)
            continue;

        if (!SpellMgr::IsPositiveSpell(foundSpellInfo->Id))
            continue;

        if (SpellMgr::GetSpellDuration(foundSpellInfo) < 60000)
            continue;

        if (aura->m_procCharges > 0) // just skip spells with charges.
            continue;

        bool sameCaster = aura->GetCasterGUID() == casterGUID;
        if (!SpellMgr::IsNoStackSpellDueToSpell(foundSpellInfo->Id, newSpellInfo->Id, sameCaster)) // spells are stackable
            continue;

        AuraCountFound = 0;
        for (uint8 i = 0; i < 3; ++i)
        {
            if (foundSpellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && foundSpellInfo->EffectApplyAuraName[i])
            {
                spellAuraNameFound[i].first = foundSpellInfo->EffectApplyAuraName[i];
                spellAuraNameFound[i].second.first = foundSpellInfo->EffectMiscValue[i];
                spellAuraNameFound[i].second.second = foundSpellInfo->EffectMiscValueB[i];
                ++AuraCountFound;
            }
            else
            {
                spellAuraNameFound[i].first = 0;
                spellAuraNameFound[i].second.first = 0;
                spellAuraNameFound[i].second.second = 0;
            }
        }

        // if new aura has more from found aura -> allow new
        if (AuraCountThis > AuraCountFound) // there will be atleast one aura anyway (the aura that we're using to get to spellInfo)
            continue;

        // now we need to set newSpell and foundSpell apply auras in tact.
        // for case when new aura has 2 auras, found aura has 3 auras (which include those 2)
        // in this case we need to check 2 auras to 2 auras of found spell to decide if new aura could be better
        // If found spell has no auras which new spell has (check includes difference in miscValues) -> allow new spell
        bool canCast;
        bool newIsBetter = false;
        bool oldIsBetter = false;
        bool sameAsOld = false;
        for (uint8 i = 0; i < 3; ++i)
        {
            if (spellAuraNameThis[i].first) // new effect has such aura
            {
                canCast = true;
                for (uint8 j = 0; j < 3; ++j) // find it in new aura
                {
                    if (spellAuraNameThis[i].first == spellAuraNameFound[j].first
                        && spellAuraNameThis[i].second.first == spellAuraNameFound[j].second.first
                        && spellAuraNameThis[i].second.second == spellAuraNameFound[j].second.second)
                    {
                        // if any of new is better than found aura -> allow new
                        if (Aura* aura = GetAura(foundSpellInfo->Id, j))
                        {
                            int32 NewValue = caster ? caster->CalculateSpellDamage(newSpellInfo, i, newSpellInfo->EffectBasePoints[i], false) : newSpellInfo->CalculateSimpleValue(i);
                            int32 auraValue = aura->GetModifierValue();
                            bool signsDiffer = (NewValue > 0 && auraValue < 0) || (NewValue < 0 && auraValue > 0);
                            NewValue = abs(NewValue);
                            auraValue = abs(auraValue);

                            // newSpellInfo is positive, foundSpellInfo is too. So if NewValue and auraValue have different signs -> NewValue will be considered better
                            if (NewValue > auraValue || signsDiffer)
                                newIsBetter = true; // can cast if atleast one effect is better
                            else if (NewValue == auraValue && aura->GetAuraDuration() < SpellMgr::GetSpellDuration(foundSpellInfo))
                                sameAsOld = true; // can cast only if all basepoints are the same
                            else
                                oldIsBetter = true; // can cast only if there is atleast one effect which is better
                        }
                        canCast = false; // such aura is found in foundAura -> dont stop the cycle -> check next one
                    }
                }
                if (canCast)
                    break;
            }
        }
        if (canCast) // found spell has not atleast one of the newspell auras
            continue;

        if (newIsBetter)
            continue;

        if (sameAsOld && !oldIsBetter)
            continue;

        return true;
    }
    return false;
}

void Unit::UpdateSplineMovement(uint32 t_diff)
{
    enum
    {
        POSITION_UPDATE_DELAY = 200,
        POSITION_UPDATE_CHARGE_DELAY = 100
    };

    bool charging = HasUnitState(UNIT_STAT_CHARGING);

    if (IsStopped() || (CantMove() && !charging))
        return;

    movespline->updateState(t_diff);
    bool arrived = movespline->Finalized();

    if (arrived)
        DisableSpline();

    if (m_movesplineTimer.Expired(t_diff) || arrived)
    {
        m_movesplineTimer.Reset(charging ? POSITION_UPDATE_CHARGE_DELAY : POSITION_UPDATE_DELAY);
        Movement::Location loc = movespline->ComputePosition();

        if (GetTypeId() == TYPEID_PLAYER)
            ToPlayer()->SetPosition(loc.x,loc.y,loc.z,loc.orientation);
        else
            Unit::SetPosition(loc.x,loc.y,loc.z,loc.orientation);
    }
}

void Unit::DisableSpline()
{
    m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEFLAG_SPLINE_ENABLED|MOVEFLAG_FORWARD));
    movespline->_Interrupt();
}
void Unit::resetAttackTimer(WeaponAttackType type)
{
    m_attackTimer[type] = uint32(GetAttackTime(type) * m_modAttackSpeedPct[type]);
}

bool Unit::IsWithinCombatRange(const Unit *obj, float dist2compare) const
{
    if (!obj || !IsInMap(obj))
        return false;

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool Unit::IsWithinMeleeRange(Unit *obj, float dist) const
{
    if (!obj || !IsInMap(obj))
        return false;

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float maxdist = std::max(dist + sizefactor,NOMINAL_MELEE_RANGE);

    if (IsMoving() && !IsWalking() && obj->IsMoving() && !obj->IsWalking())
        maxdist += MELEE_LEEWAY;

	if (ToCreature() || obj->ToCreature())
	{
		return dx * dx + dy * dy < maxdist * maxdist; // && fabs(GetPositionZ() - obj->GetPositionZ()) < 7.0f
	}

    return distsq < maxdist * maxdist;
}

bool Unit::CanReachWithMeleeAutoAttackAtPosition(Unit const* pVictim, float x, float y, float z, float flat_mod /*= 0.0f*/) const
{
	if (!pVictim || !pVictim->IsInWorld())
		return false;

	float reach = GetCombatReach(pVictim, false, flat_mod);

	float dx = x - pVictim->GetPositionX();
	float dy = y - pVictim->GetPositionY();
	float dz = z - pVictim->GetPositionZ();

	float zReach = 36;
	return (dx * dx + dy * dy < reach * reach) && ((dz * dz) < zReach);
}

float Unit::GetCombatReach(bool forMeleeRange /*=false*/) const
{
	float reach = m_floatValues[UNIT_FIELD_COMBATREACH];
	return (forMeleeRange && reach < 1.5f) ? 1.5f : reach;
}

float Unit::GetCombatReach(Unit const* pVictim, bool ability, float flat_mod) const
{
	float victimReach = (pVictim && pVictim->IsInWorld())
		? pVictim->GetCombatReach(true)
		: 0.0f;

	float reach = GetCombatReach(true) + victimReach + flat_mod;

	reach += MELEE_RANGE;
	if (reach < NOMINAL_MELEE_RANGE)
		reach = NOMINAL_MELEE_RANGE;

	// Melee leeway mechanic.
	// When both player and target has > 70% of normal runspeed, and are moving,
	// the player gains an additional 2.66yd of melee range.
	if ((m_movementInfo.HasMovementFlag(MOVEFLAG_MOVING) && !m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE)) &&
		(pVictim->m_movementInfo.HasMovementFlag(MOVEFLAG_MOVING) && !pVictim->m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE)))
		reach += 2 * MELEE_RANGE;

	return reach;
}

void Unit::GetRandomContactPoint(const Unit* obj, float &x, float &y, float &z, float distance2dMin, float distance2dMax) const
{
    float combat_reach = GetCombatReach();
    if (combat_reach < 0.1) // sometimes bugged for players
        combat_reach = DEFAULT_COMBAT_REACH;

    uint32 attacker_number = GetAttackers().size();
    if (attacker_number > 0) --attacker_number;
    GetNearPoint(x,y,z,obj->GetCombatReach(), distance2dMin+(distance2dMax-distance2dMin)*rand_norm()
                 , GetOrientationTo(obj) + (attacker_number ? (M_PI/2 - M_PI * rand_norm()) * (float)attacker_number / combat_reach / 3 : 0));
}

void Unit::RemoveMovementImpairingAuras()
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        SpellEntry const* spellInfo = iter->second->GetSpellProto();
        if (spellInfo->AttributesCu & SPELL_ATTR_CU_MOVEMENT_IMPAIR)
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveSpellsCausingAura(AuraType auraType)
{
    if (auraType >= TOTAL_AURAS)
        return;

    AuraList::iterator iter, next;
    for (iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end(); iter = next)
    {
        next = iter;
        ++next;

        if (*iter)
        {
            RemoveAurasDueToSpell((*iter)->GetId());
            if (!m_modAuras[auraType].empty())
                next = m_modAuras[auraType].begin();
            else
                return;
        }
    }
}

void Unit::RemoveAuraTypeByCaster(AuraType auraType, uint64 casterGUID)
{
    if (auraType >= TOTAL_AURAS)
        return;

    for (AuraList::iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end();)
    {
        Aura *aur = *iter;
        ++iter;

        if (aur)
        {
            uint32 removedAuras = m_removedAurasCount;
            RemoveAurasByCasterSpell(aur->GetId(), casterGUID);
            if (m_removedAurasCount > removedAuras + 1)
                iter = m_modAuras[auraType].begin();
        }
    }
}

bool Unit::hasNegativeAuraWithInterruptFlag(uint32 flag)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); ++iter)
    {
        if (!iter->second->IsPositive() && iter->second->GetSpellProto()->AuraInterruptFlags & flag)
            return true;
    }
    return false;
}

void Unit::RemoveAurasWithInterruptFlags(uint32 flag, uint32 except, bool PositiveOnly)
{
    if (!(m_interruptMask & flag))
        return;

    // interrupt auras
    AuraList::iterator iter;
    for (iter = m_interruptableAuras.begin(); iter != m_interruptableAuras.end();)
    {
        Aura *aur = *iter;
        ++iter;

        if (aur && (aur->GetSpellProto()->AuraInterruptFlags & flag) && (!PositiveOnly || aur->IsPositive()))
        {
            if (aur->IsInUse())
                sLog.outLog(LOG_DEFAULT, "ERROR: Aura %u is trying to remove itself! Flag %u. May cause crash!", aur->GetId(), flag);

            else if (!except || aur->GetId() != except)
            {
                uint32 removedAuras = m_removedAurasCount;

                RemoveAurasDueToSpell(aur->GetId());
                if (m_removedAurasCount > removedAuras + 1)
                    iter = m_interruptableAuras.begin();
            }
        }
    }

    // interrupt channeled spell
    if (Spell* spell = m_currentSpells[CURRENT_CHANNELED_SPELL])
        if (spell->getState() == SPELL_STATE_CASTING
            && (spell->GetSpellEntry()->ChannelInterruptFlags & flag)
            && spell->GetSpellEntry()->Id != except)
            InterruptNonMeleeSpells(false);

    UpdateInterruptMask();
}

void Unit::UpdateInterruptMask()
{
    m_interruptMask = 0;
    for (AuraList::iterator i = m_interruptableAuras.begin(); i != m_interruptableAuras.end(); ++i)
    {
        if (*i)
            m_interruptMask |= (*i)->GetSpellProto()->AuraInterruptFlags;
    }
    if (Spell* spell = m_currentSpells[CURRENT_CHANNELED_SPELL])
        if (spell->getState() == SPELL_STATE_CASTING)
            m_interruptMask |= spell->GetSpellEntry()->ChannelInterruptFlags;
}

bool Unit::HasAuraType(AuraType auraType) const
{
    return (!m_modAuras[auraType].empty());
}

bool Unit::HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName  ,uint64 familyFlags) const
{
    if (!HasAuraType(auraType)) return false;
    AuraList const &auras = GetAurasByType(auraType);
    for (AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        if (SpellEntry const *iterSpellProto = (*itr)->GetSpellProto())
            if (iterSpellProto->SpellFamilyName == familyName && iterSpellProto->SpellFamilyFlags & familyFlags)
                return true;
    return false;
}

/*uint32 Unit::GetAurasAmountByMiscValue(AuraType auraType, uint32 misc)
{
    uint32 count = 0;
    Unit::AuraList mAuras = GetAurasByType(SPELL_AURA_MECHANIC_IMMUNITY);
    for (Unit::AuraList::iterator iter = mAuras.begin(); iter != mAuras.end(); ++iter)
    {
        if ((*iter)->GetMiscValue() == misc)
            ++count;
    }

    return count;
}*/

bool Unit::HasAuraByCasterWithFamilyFlags(uint64 pCaster, uint32 familyName,  uint64 familyFlags, const Aura * except) const
{
    const AuraMap & tmpMap = GetAuras();
    SpellEntry const * tmpSpellEntry;
    for (AuraMap::const_iterator itr = tmpMap.begin(); itr != tmpMap.end(); ++itr)
    {
        if ((!except || except != itr->second))
        {
            if (tmpSpellEntry = itr->second->GetSpellProto())
            {
                if (tmpSpellEntry->SpellFamilyName == familyName && tmpSpellEntry->SpellFamilyFlags & familyFlags && (itr->second->GetCasterGUID() == pCaster))
                    return true;
            }
        }
    }

    return false;
}

/* Called by DealDamage for auras that have a chance to be dispelled on damage taken. */
void Unit::RemoveSpellbyDamageTaken(uint32 damage, uint32 except, bool direct)
{
    uint32 dispelable = 0;
    if (direct)
        damage = damage*3/2;
    std::list<std::pair<uint32, uint64> > aurasToRemove;
    std::set<std::pair<uint32, uint64> > aurasDone;
    for (auto i = m_ccAuras.begin(); i != m_ccAuras.end(); ++i)
    {
        std::pair<uint32, uint64> auraPair(i->first->GetId(), i->first->GetCasterGUID());
        // prevent rolling twice for two effects of the same spell
        if(aurasDone.find(auraPair) != aurasDone.end())
            continue;

        aurasDone.insert(auraPair);

        if (i->first && (!except || i->first->GetId() != except))
        {
            if (SpellMgr::GetDiminishingReturnsGroupForSpell(i->first->GetSpellProto(), false) == DIMINISHING_ENSLAVE)
                continue;

            if (damage >= i->second)
                aurasToRemove.push_back(auraPair);
            else
                i->second -= damage; // go until reaches 0

            dispelable++;
        }
    }

    SendCombatStats(1<<COMBAT_STATS_DISPEL_CHANCE, "Unit::RemoveSpellbyDamageTaken removing %u auras (dispelable %u, total %u)", 0, aurasToRemove.size(), dispelable, m_ccAuras.size());

    for (std::list<std::pair<uint32, uint64> >::iterator i = aurasToRemove.begin(); i != aurasToRemove.end(); ++i)
        RemoveAurasByCasterSpell(i->first, i->second);
}

void Unit::SendDamageLog(DamageLog *damageInfo)
{
    switch (damageInfo->opcode)
    {
        case SMSG_ATTACKERSTATEUPDATE:
            SendAttackStateUpdate(((MeleeDamageLog *)damageInfo));
            break;
        case SMSG_SPELLNONMELEEDAMAGELOG:
            SendSpellNonMeleeDamageLog(((SpellDamageLog *)damageInfo));
            break;
        case SMSG_PERIODICAURALOG:
            // TODO
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: Unsupported opcode in SendDamageLog: %d!", damageInfo->opcode);
        case 1: // dealdamage ktory nie powinien wysylac loga
            break;
    }
}

bool SpellCantDealDmgToPlayer(uint32 id)
{
    switch (id)
    {
        case 35139:         // Throw Heavy Bomb (bomb from DR.Doom q)
        case 33836:         // Dropping Heavy Bomb (bomb from q in HP)
            return true;
        default:
            return false;
    }
}

// tymczasowo to kopia starej funkcji - trzeba zmienic
uint32 Unit::DealDamage(DamageLog *damageInfo, DamageEffectType damagetype, const SpellEntry *spellProto, bool durabilityLoss, bool& duel_just_ended)
{
    Unit *pVictim = damageInfo->target;
    if (!pVictim->isAlive() || pVictim->IsTaxiFlying() || pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode() || pVictim->HasAura(27827, 2))
        return 0;

	// remove affects from attacker at any non-DoT damage (including 0 damage)
	if (damagetype != DOT)
	{
		// Since patch 1.5.0 sitting characters always stand up on attack (even if stunned)
		if (!pVictim->IsStandState() && (pVictim->GetTypeId() == TYPEID_PLAYER || !pVictim->HasUnitState(UNIT_STAT_STUNNED)))
			pVictim->SetStandState(UNIT_STAND_STATE_STAND);
	}

    //You don't lose health from damage taken from another player while in a sanctuary
    if (pVictim != this && isCharmedOwnedByPlayerOrPlayer() && pVictim->isCharmedOwnedByPlayerOrPlayer() &&
        pVictim->GetOwner() != this && pVictim->isInSanctuary())
        return 0;

    // Do not deal damage from AoE spells when target is immune to it
    if (!pVictim->isAttackableByAOE() && spellProto && SpellMgr::IsAreaOfEffectSpell(spellProto))
        return 0;

    if (spellProto && damageInfo->damage > 0)
        damageInfo->damage = float(damageInfo->damage) * SpellSpecialMod(spellProto->Id);

    if (pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        // hacky way -.-
        if (spellProto && SpellCantDealDmgToPlayer(spellProto->Id))
            return 0;

        // Handle Blessed Life
        if (pVictim->GetClass() == CLASS_PALADIN)
        {
            AuraList procTriggerAuras = pVictim->GetAurasByType(SPELL_AURA_PROC_TRIGGER_SPELL);
            for (AuraList::iterator i = procTriggerAuras.begin(); i != procTriggerAuras.end(); ++i)
            {
                switch ((*i)->GetSpellProto()->Id)
                {
                     case 31828: // Rank 1
                     case 31829: // Rank 2
                     case 31830: // Rank 3
                     {
                         if (roll_chance_i((*i)->GetSpellProto()->procChance))
                            damageInfo->damage /= 2; // Trentone check for double-compatibility
                         break;
                     }
                }
            }
        }
    }

    if (pVictim->GetTypeId() == TYPEID_UNIT)
    {
        if (((Creature *)pVictim)->IsAIEnabled)
            ((Creature *)pVictim)->AI()->DamageTaken(this, damageInfo->damage);

        if (!pVictim->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER) && !((Creature*)pVictim)->isPet())
        {
            //Set Loot
            switch (GetTypeId())
            {
                case TYPEID_PLAYER:
                {
                    ((Creature *)pVictim)->SetLootRecipient(this);
                    //Set tagged
                    ((Creature *)pVictim)->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);
                    break;
                }
                case TYPEID_UNIT:
                {
                    if (((Creature*)this)->isPet())
                    {
                        ((Creature *)pVictim)->SetLootRecipient(this->GetOwner());
                        ((Creature *)pVictim)->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);
                    }
                    break;
                }
            }
        }
    }

    //Script Event damage made on players by Unit
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsAIEnabled)
        if (damageInfo->damage)
            ((Creature*)this)->AI()->DamageMade(pVictim, damageInfo->damage, damagetype == DIRECT_DAMAGE, damageInfo->schoolMask);

    // && !(spellProto && spellProto->Id == 33619)
    if ((damageInfo->damage || damageInfo->absorb) && !(spellProto && spellProto->AttributesEx4 & SPELL_ATTR_EX4_DAMAGE_DOESNT_BREAK_AURAS))
    {
        if (!spellProto || !(spellProto->AttributesEx4 & SPELL_ATTR_EX4_DAMAGE_DOESNT_BREAK_AURAS))
        {
            pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DAMAGE, spellProto ? spellProto->Id : 0);
            pVictim->RemoveSpellbyDamageTaken(damageInfo->damage, spellProto ? spellProto->Id : 0, damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE);
            if (damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE)
            {
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DIRECT_DAMAGE, spellProto ? spellProto->Id : 0);
                if (pVictim->GetTypeId() == TYPEID_UNIT && !pVictim->ToCreature()->isPet())
                    pVictim->SetLastDamagedTime(time(NULL));
            }
            if (damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE || (damagetype == DOT && damageInfo->damage))
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DIRECT_DAMAGE_OR_NON_ABSORBED_DOT, spellProto ? spellProto->Id : 0);
        }
        else// if (spellProto->AttributesEx4 & SPELL_ATTR_EX4_DAMAGE_DOESNT_BREAK_AURAS) // if got here - 100% got this attribute
        {
            pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DAMAGE, spellProto->Id, true);
            if (damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE)
            {
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DIRECT_DAMAGE, spellProto->Id, true);
            }
            if (damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE || (damagetype == DOT && damageInfo->damage))
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DIRECT_DAMAGE_OR_NON_ABSORBED_DOT, spellProto->Id, true);
        }

        // Rage from any damage taken
        if (pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->getPowerType() == POWER_RAGE))
            ((Player*)pVictim)->RewardRage(damageInfo->rageDamage, 0, false, GetObjectGuid().IsCreature() ? (Creature*)this : NULL);

        if (Spell* spell = pVictim->m_currentSpells[CURRENT_GENERIC_SPELL])
            if (damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE)
                if (spell->GetSpellEntry()->Id == 21651 || spell->GetSpellEntry()->Id == 26868 ||
                    (spell->GetSpellEntry()->Id == 3365 && pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->IsInGurubashiEvent()))
                    pVictim->InterruptSpell(CURRENT_GENERIC_SPELL, true, true);
    }

    if (damageInfo->damage)
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            // no xp,health if type 8 /critters/
            if (pVictim->GetCreatureType() == CREATURE_TYPE_CRITTER)
            {
                // allow loot only if has loot_id in creature_template
                if (damageInfo->damage >= pVictim->GetHealth())
                {
                    pVictim->setDeathState(JUST_DIED);
                    pVictim->SetHealth(0);

                    CreatureInfo const* cInfo = ((Creature*)pVictim)->GetCreatureInfo();
                    if (cInfo && cInfo->lootid)
                        pVictim->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

                    // some critters required for quests
                    if (GetTypeId() == TYPEID_PLAYER)
                        ((Player*)this)->KilledMonster(pVictim->GetEntry(),pVictim->GetGUID());
                }
                else
                    pVictim->ModifyHealth(- (int32)damageInfo->damage);

                SendDamageLog(damageInfo);
                return damageInfo->damage;
            }
        }

        uint32 health = pVictim->GetHealth();
        sLog.outDetail("deal dmg:%d to health:%d ", damageInfo->damage,health);

        // duel ends when player has 1 or less hp
        bool duel_hasEnded = false;
        if (pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->duel && damageInfo->damage >= (float)health-1)
        {
            // prevent kill only if killed in duel and killed by opponent or opponent controlled creature
            //if (((Player*)pVictim)->duel->opponent == this || ((Player*)pVictim)->duel->opponent->GetGUID() == GetOwnerGUID() || pVictim == this/*this shall prevent spell reflection/ shadow word death kill*/)
            //    damageInfo->damage = 0;

            duel_hasEnded = true;
        }

        // Rage from Damage made (only from direct weapon damage)
        // rageDamage contains damage that we've done or could've done, only changed by Unit::RollMeleeHit
        if (damageInfo->rageDamage && damagetype == DIRECT_DAMAGE && this != pVictim && GetTypeId() == TYPEID_PLAYER && (getPowerType() == POWER_RAGE))
        {
            switch (damageInfo->attackType)
            {
            default:
            {
                float factor = damageInfo->attackType == OFF_ATTACK ? 1.75f : 3.5f;

                if (damageInfo->opcode == SMSG_ATTACKERSTATEUPDATE)
                    if (((MeleeDamageLog*)damageInfo)->procEx & PROC_EX_CRITICAL_HIT)
                        factor *= 2.0f;

                uint32 weaponSpeedHitFactor = uint32(GetAttackTime(damageInfo->attackType) / 1000.0f * factor);
                ((Player*)this)->RewardRage(damageInfo->rageDamage, weaponSpeedHitFactor, true);
            }
            break;
            case RANGED_ATTACK:
                break;
            }
        }

		// player to player
        if (pVictim->GetTypeId() == TYPEID_PLAYER && GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)pVictim)->InBattleGroundOrArena())
            {
                Player *killer = ((Player*)this);
                if (killer != ((Player*)pVictim))
                    if (BattleGround *bg = killer->GetBattleGround())
                        bg->UpdatePlayerScore(killer, SCORE_DAMAGE_DONE, damageInfo->damage);
            }

			// try to catch damage bugs
			if (pVictim != this && spellProto && damageInfo->damage > 10000)
			{
				sLog.outLog(LOG_SPECIAL, "HIGH_DAMAGE: Player %s deal damage %u to player %s with spell %u (%s)", GetName(), damageInfo->damage, pVictim->GetName(), spellProto->Id, spellProto->SpellName[0]);
			}
        }

        if (pVictim->GetTypeId() == TYPEID_UNIT && !((Creature*)pVictim)->isPet())
        {
            if (!((Creature*)pVictim)->hasLootRecipient())
                ((Creature*)pVictim)->SetLootRecipient(this);

            if (GetCharmerOrOwnerPlayerOrPlayerItself())
                ((Creature*)pVictim)->LowerPlayerDamageReq(health < damageInfo->damage ?  health : damageInfo->damage);
        }

        if (health <= damageInfo->damage)
        {
            debug_log("DealDamage: victim just died");

            if (duel_hasEnded)
            {
                ASSERT(pVictim->GetTypeId() == TYPEID_PLAYER);
                Player* he = (Player*)pVictim;

                ASSERT(he->duel);

                float hp = (float)he->GetMaxHealth() / 10.0f;
                if (hp < 1)
                    hp = 1;

                he->SetHealth(hp);

                Player* temp = he->duel->opponent;

                temp->CombatStopWithPets(true);
                he->CombatStopWithPets(true);
                he->CastSpell(he, 7267, true);                  // beg
                he->DuelComplete(DUEL_WON);
                he->GetCamera().UpdateVisibilityForOwner(); // should remove bug with no-tablet duel
                temp->GetCamera().UpdateVisibilityForOwner(); // should remove bug with no-tablet duel
                duel_just_ended = true;
            }
            else
                Kill(pVictim, durabilityLoss);

            if (pVictim->GetTypeId() == TYPEID_PLAYER && pVictim != this)
                SetContestedPvP();
        }
        else                                                    // if (health <= damage)
        {
            pVictim->ModifyHealth(- (int32)damageInfo->damage);

            if (damagetype != DOT)
            {
                if (!GetVictim())
                {
                    // if not have main target then attack state with target (including AI call)
                    //start melee attacks only after melee hit

                    // Do not start combat if caster is player and target is playerOrPet nad playerOrPet is friendly to caster
                    if (!(GetTypeId() == TYPEID_PLAYER && pVictim->IsFriendlyTo(this) && pVictim->GetCharmerOrOwnerPlayerOrPlayerItself()))
                        Attack(pVictim,(damagetype == DIRECT_DAMAGE));
                }
            }

            if (pVictim->GetTypeId() != TYPEID_PLAYER)
            {
                Unit *threatTarget = this;

                float threat = damageInfo->damage * sSpellMgr.GetSpellThreatMultiplier(spellProto);

                //SpellMgr::ApplySpellThreatModifiers(spellProto, threat);

                if (damageInfo->threatTarget)
                {
                    if (Unit *pTarget = GetMap()->GetUnit(damageInfo->threatTarget))
                        threatTarget = pTarget;
                }

                pVictim->AddThreat(threatTarget, threat, SpellSchoolMask(damageInfo->schoolMask), spellProto);
                if (pVictim->ToPet() && pVictim->IsAIEnabled)
                    pVictim->ToPet()->AI()->ownerOrMeAttackedBy(threatTarget->GetGUID());
            }
            else                                                // victim is a player
            {
                // random durability for items (HIT TAKEN)
                if (!((Player*)pVictim)->InBattleGroundOrArena() && roll_chance_f(sWorld.getConfig(RATE_DURABILITY_LOSS_DAMAGE)))
                {
                  EquipmentSlots slot = EquipmentSlots(urand(0,EQUIPMENT_SLOT_END-1));
                    ((Player*)pVictim)->DurabilityPointLossForEquipSlot(slot);
                }

                if (Pet* pet = pVictim->GetPet())
                {
                    if (pet->IsAIEnabled)
                        pet->AI()->ownerOrMeAttackedBy(GetGUID());
                }
            }

            if (GetTypeId()==TYPEID_PLAYER)
            {
                // random durability for items (HIT DONE)
                if (!((Player*)this)->InBattleGroundOrArena() && roll_chance_f(sWorld.getConfig(RATE_DURABILITY_LOSS_DAMAGE)))
                {
                    EquipmentSlots slot = EquipmentSlots(urand(0,EQUIPMENT_SLOT_END-1));
                    ((Player*)this)->DurabilityPointLossForEquipSlot(slot);
                }
            }

            if (damagetype != NODAMAGE && damageInfo->damage)
            {
                if (pVictim != this && pVictim->GetTypeId() == TYPEID_PLAYER) // does not support creature push_back
                {
                    if (damagetype != DOT)
                    {
                        if (Spell* spell = pVictim->m_currentSpells[CURRENT_GENERIC_SPELL])
                        {
                            if (spell->getState() == SPELL_STATE_PREPARING)
                            {
                                uint32 interruptFlags = spell->GetSpellEntry()->InterruptFlags;
                                if (interruptFlags & SPELL_INTERRUPT_FLAG_DAMAGE)
                                    pVictim->InterruptNonMeleeSpells(false);
                                else if (interruptFlags & SPELL_INTERRUPT_FLAG_PUSH_BACK)
                                {
                                    if (!spellProto || (spellProto && !(spellProto->AttributesCu & SPELL_ATTR_CU_NO_PUSHBACK)))
                                        spell->Delayed();
                                }
                            }
                        }
                    }

                    if (Spell* spell = pVictim->m_currentSpells[CURRENT_CHANNELED_SPELL])
                    {
                        if (spell->getState() == SPELL_STATE_CASTING)
                        {
                            uint32 channelInterruptFlags = spell->GetSpellEntry()->ChannelInterruptFlags;
                            if (damagetype != DOT && channelInterruptFlags & CHANNEL_INTERRUPT_FLAG_DELAY)
                            {
                                if (!spellProto || (spellProto && !(spellProto->AttributesCu & SPELL_ATTR_CU_NO_PUSHBACK)))
                                    spell->DelayedChannel();
                            }
                        }
                    }
                }
            }
        }

        if (GetMap() && GetMap()->IsDungeon() && pVictim != this)
        {
            Player* chief = GetCharmerOrOwnerPlayerOrPlayerItself();
            if (chief && pVictim != chief)
            {
                if (InstanceData* idata = ((InstanceMap*)GetMap())->GetInstanceData())
                    idata->OnPlayerDealDamage(chief, damageInfo->damage);
            }
        }
    }

    // send damage to client here - after modifications
    SendDamageLog(damageInfo);

    // log if player does > 30000 dmg to a boss
    if (damageInfo->damage > 30000 &&
        pVictim->GetTypeId() == TYPEID_UNIT &&
        ((Creature*)pVictim)->GetCreatureInfo()->rank == 3 &&
        GetTypeId() == TYPEID_PLAYER)
    {
        switch (pVictim->GetEntry())
        {
        // ignore these
        case 15928: // Thaddius
        case 25158: // fake Brut
        case 22948: // Bloodboil
        case 21215: // Leotheras
        case 15691: // Curator
            break;

        default:
            sLog.outLog(LOG_CRITICAL, "HIGH DAMAGE - %u! %s (entry: %u) to %s (entry: %u), xyz m: %f,%f,%f %u", damageInfo->damage, GetName(), GetEntry(), pVictim->GetName(), pVictim->GetEntry(), pVictim->GetPositionX(), pVictim->GetPositionY(), pVictim->GetPositionZ(), pVictim->GetMapId());
        }
    }
    
    ScheduleAINotify(GetTerrain()->GetSpecifics()->ainotifyperiod);
    return damageInfo->damage;
}

uint32 Unit::DealDamage(Unit *pVictim, uint32 damage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellEntry const *spellProto, bool durabilityLoss)
{
    DamageLog damageInfo(1, this, pVictim, damageSchoolMask);
    damageInfo.damage = damage;

    bool duel_just_ended = false;
    return DealDamage(&damageInfo, damagetype, spellProto, durabilityLoss, duel_just_ended);
}

void Unit::CastStop(uint32 except_spellid)
{
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        if (m_currentSpells[i] && m_currentSpells[i]->GetSpellEntry()->Id!=except_spellid)
            InterruptSpell(i,false, false);
}

SpellCastResult Unit::CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastSpell: unknown spell id %i by caster: %s %u) triggered: %s", spellId,
            (GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),
            (GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()),
            triggered ? "true" : "false");
        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    return CastSpell(Victim,spellInfo,triggered,castItem,triggeredByAura, originalCaster);
}

SpellCastResult Unit::CastSpell(Unit* Victim,SpellEntry const *spellInfo, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastSpell: unknown spell by caster: %s %u)", (GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    if (!SpellMgr::CheckVictimAppropriate(spellInfo, Victim, false))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastSpell: spell id %i by caster: %s %u) does not have unit target", spellInfo->Id, (GetTypeId() == TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"), (GetTypeId() == TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
    }

    SpellCastTargets targets;

    if (spellInfo->Targets & (TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION))
        targets.setDestination(Victim);

    if (castItem)
        debug_log("WORLD: cast Item spellId - %i", spellInfo->Id);

    if (!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster);

    if (Victim)
        targets.setUnitTarget(Victim);
    else
    {
        spell->FillTargetMap();
        targets = spell->m_targets;
    }

    spell->m_CastItem = castItem;
    return spell->prepare(&targets, triggeredByAura);
}

SpellCastResult Unit::CastCustomSpell(Unit* target, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    CustomSpellValues values;
    if (bp0) values.AddSpellMod(SPELLVALUE_BASE_POINT0, *bp0);
    if (bp1) values.AddSpellMod(SPELLVALUE_BASE_POINT1, *bp1);
    if (bp2) values.AddSpellMod(SPELLVALUE_BASE_POINT2, *bp2);
    return CastCustomSpell(spellId, values, target, triggered, castItem, triggeredByAura, originalCaster);
}

SpellCastResult Unit::CastCustomSpell(uint32 spellId, SpellValueMod mod, uint32 value, Unit* target, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    CustomSpellValues values;
    values.AddSpellMod(mod, value);
    return CastCustomSpell(spellId, values, target, triggered, castItem, triggeredByAura, originalCaster);
}

SpellCastResult Unit::CastCustomSpell(uint32 spellId, CustomSpellValues const &value, Unit* Victim, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastCustomSpell: unknown spell id %i by caster: %s %u) triggered: %s", spellId,
            (GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),
            (GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()),
            triggered ? "true" : "false");
        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    if (!SpellMgr::CheckVictimAppropriate(spellInfo, Victim))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastSpell: spell id %i by caster: %s %u) does not have unit target", spellInfo->Id, (GetTypeId() == TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"), (GetTypeId() == TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
    }

    SpellCastTargets targets;

    //check destination
    if (spellInfo->Targets & (TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION))
        targets.setDestination(Victim);

    if (!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster);

    // for max targets spell mod, custom values should be here, before filling target map
    for (CustomSpellValues::const_iterator itr = value.begin(); itr != value.end(); ++itr)
        spell->SetSpellValue(itr->first, itr->second);

    // if victim is defined use it, if not, search for targets
    if (Victim)
        targets.setUnitTarget(Victim);
    else
    {
        spell->FillTargetMap();
        targets = spell->m_targets;
    }

    if (castItem)
    {
        debug_log("WORLD: cast Item spellId - %i", spellInfo->Id);
        spell->m_CastItem = castItem;
    }

    return spell->prepare(&targets, triggeredByAura);
}

// used for scripting
SpellCastResult Unit::CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastSpell(x,y,z): unknown spell id %i by caster: %s %u)", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    if (castItem)
        debug_log("WORLD: cast Item spellId - %i", spellInfo->Id);

    if (!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster);

    SpellCastTargets targets;
    targets.setDestination(x, y, z);
    spell->m_CastItem = castItem;
    return spell->prepare(&targets, triggeredByAura);
}

// used for scripting
SpellCastResult Unit::CastSpell(GameObject *go, uint32 spellId, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    if (!go)
        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastSpell(x,y,z): unknown spell id %i by caster: %s %u)", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    if (!(spellInfo->Targets & (TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_OBJECT_UNK)))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CastSpell: spell id %i by caster: %s %u) is not gameobject spell", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
    }

    if (castItem)
        debug_log("WORLD: cast Item spellId - %i", spellInfo->Id);

    if (!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster);

    SpellCastTargets targets;
    targets.setGOTarget(go);
    spell->m_CastItem = castItem;
    return spell->prepare(&targets, triggeredByAura);
}

// Obsolete func need remove, here only for comotability vs another patches
uint32 Unit::SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool isTriggeredSpell, bool useSpellDamage)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellID);
    SpellDamageLog damageInfo(sSpellMgr.GetSpellAnalog(spellInfo), this, pVictim, spellInfo->SchoolMask);
    damage = SpellDamageBonus(pVictim, spellInfo, damage, SPELL_DIRECT_DAMAGE);
    CalculateSpellDamageTaken(&damageInfo, damage, spellInfo);
    //SendSpellNonMeleeDamageLog(&damageInfo);
    DealSpellDamage(&damageInfo, true);
    return damageInfo.damage;
}

void Unit::CalculateSpellDamageTaken(SpellDamageLog *damageInfo, int32 damage, SpellEntry const *spellInfo, WeaponAttackType attackType, bool crit, bool blocked)
{
    if (damage < 0)
        return;

    Unit *pVictim = damageInfo->target;
    if (!pVictim || !pVictim->isAlive())
        return;

    if (damageInfo->schoolMask & SPELL_SCHOOL_MASK_NORMAL  && (spellInfo->AttributesCu & SPELL_ATTR_CU_IGNORE_ARMOR) == 0)
        damage = CalcArmorReducedDamage(pVictim, damage);

    damageInfo->rageDamage = damage;

    SpellSchoolMask damageSchoolMask = SpellSchoolMask(damageInfo->schoolMask);
    uint32 crTypeMask = pVictim->GetCreatureTypeMask();
    // Check spell crit chance
    //bool crit = isSpellCrit(pVictim, spellInfo, damageSchoolMask, attackType);
    // Per-school calc
    switch (spellInfo->DmgClass)
    {
        // Melee and Ranged Spells
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
        {
            // Physical Damage

            if (crit)
            {
                damageInfo->hitInfo |= SPELL_HIT_TYPE_CRIT;

                // Calculate crit bonus
                uint32 crit_bonus = damage;
                // Apply crit_damage bonus for melee spells
                if (Player* modOwner = GetSpellModOwner())
                    modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);
                damage += crit_bonus;

                // Apply SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE or SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE
                int32 critPctDamageMod=0;
                critPctDamageMod += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS);
                if (attackType == RANGED_ATTACK)
                    critPctDamageMod += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE);
                else
                    critPctDamageMod += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE);

                // Increase crit damage from SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
                critPctDamageMod += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, crTypeMask);

                if (critPctDamageMod!=0)
                    damage = int32((damage) * float((100.0f + critPctDamageMod)/100.0f));

                // Resilience - reduce crit damage
                if (pVictim->GetTypeId()==TYPEID_PLAYER)
                    damage -= ((Player*)pVictim)->GetMeleeCritDamageReduction(damage);

                // jezeli crit to zmieniamy
                damageInfo->rageDamage = damage;
            }
            // Spell weapon based damage CAN BE crit & blocked at same time
            if (blocked)
            {
                damageInfo->blocked = uint32(pVictim->GetShieldBlockValue());
                if (damage < damageInfo->blocked)
                    damageInfo->blocked = damage;
                damage -= damageInfo->blocked;
            }
        }
        break;
        // Magical Attacks
        case SPELL_DAMAGE_CLASS_NONE:
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            // If crit add critical bonus
            if (crit)
            {
                damageInfo->hitInfo |= SPELL_HIT_TYPE_CRIT;
                damage = SpellCriticalBonus(spellInfo, damage, pVictim);

                int32 critPctDamageMod=0;
                critPctDamageMod += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS);
                if (critPctDamageMod!=0)
                    damage = int32((damage) * float((100.0f + critPctDamageMod)/100.0f));

                // Resilience - reduce crit damage
                if (pVictim->GetTypeId()==TYPEID_PLAYER)
                    damage -= ((Player*)pVictim)->GetSpellCritDamageReduction(damage);

                damageInfo->rageDamage = damage;
            }
        }
        break;
    }

    // Calculate absorb resist
    if (damage > 0)
    {
        if (SpellMgr::IsPartialyResistable(spellInfo))
        {
            CalcAbsorbResist(pVictim, damageSchoolMask, SPELL_DIRECT_DAMAGE, damage, &damageInfo->absorb, &damageInfo->resist);
        }
        else
        {
            damageInfo->resist = 0;
            CalcAbsorb(pVictim, damageSchoolMask, damage, &damageInfo->absorb, &damageInfo->resist);
        }
        damage -= damageInfo->absorb + damageInfo->resist;
        damageInfo->rageDamage = damageInfo->rageDamage <= damageInfo->absorb ? 0 : damageInfo->rageDamage - damageInfo->absorb;
    }
    else
        damage = 0;
    damageInfo->damage = damage;
}

void Unit::DealSpellDamage(SpellDamageLog *damageInfo, bool durabilityLoss)
{
    if (!damageInfo)
        return;

    Unit *pVictim = damageInfo->target;

    if (!this || !pVictim)
        return;

    if (!pVictim->isAlive() || pVictim->IsTaxiFlying() || pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode())
        return;

    SpellEntry const *spellProto = sSpellTemplate.LookupEntry<SpellEntry>(damageInfo->spell_id);
    if (spellProto == NULL)
    {
        sLog.outDebug("Unit::DealSpellDamage have wrong damageInfo->SpellID: %u", damageInfo->spell_id);
        return;
    }

    //You don't lose health from damage taken from another player while in a sanctuary
    //You still see it in the combat log though
    if (pVictim != this && GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER &&
        (pVictim->isInSanctuary() || isInSanctuary()))
        return;

    // update at damage Judgement aura duration that applied by attacker at victim
    if (damageInfo->damage && spellProto->Id == 35395)
    {
        AuraMap& vAuras = pVictim->GetAuras();
        for (AuraMap::iterator itr = vAuras.begin(); itr != vAuras.end(); ++itr)
        {
            SpellEntry const *spellInfo = (*itr).second->GetSpellProto();
            if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo->AttributesEx3 & 0x40000)
            {
                (*itr).second->SetAuraDuration((*itr).second->GetAuraMaxDuration());
                (*itr).second->UpdateAuraDuration();
            }
        }
    }

    // Call default DealDamage
    bool duel_just_ended = false;
    DealDamage(damageInfo, SPELL_DIRECT_DAMAGE, spellProto, durabilityLoss, duel_just_ended);
}

//TODO for melee need create structure as in
void Unit::CalculateMeleeDamage(MeleeDamageLog *damageInfo)
{
    if (!this->isAlive())
        return;

    if (!damageInfo->target || !damageInfo->target->isAlive())
        return;

    // Select HitInfo/procAttacker/procVictim flag based on attack type
    switch (damageInfo->attackType)
    {
        case BASE_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT | PROC_FLAG_SUCCESSFUL_MAINHAND_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;// | PROC_FLAG_TAKEN_MAINHAND_HIT;
            damageInfo->hitInfo      = HITINFO_NORMALSWING2;
            break;
        case OFF_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT | PROC_FLAG_SUCCESSFUL_OFFHAND_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;// | PROC_FLAG_TAKEN_OFFHAND_HIT; // not used
            damageInfo->hitInfo = HITINFO_LEFTSWING;
            break;
        default:
            break;
    }

    // Physical Immune check
    if (damageInfo->target->IsImmunedToDamage(SpellSchoolMask(damageInfo->schoolMask)))
    {
       damageInfo->hitInfo       |= HITINFO_NORMALSWING;
       damageInfo->targetState    = VICTIMSTATE_IS_IMMUNE;

       damageInfo->procEx |= PROC_EX_IMMUNE;
       damageInfo->damage         = 0;
       damageInfo->rageDamage     = 0;
       return;
    }

    damageInfo->damage += CalculateDamage (damageInfo->attackType, false);

    // Add melee damage bonus
    MeleeDamageBonus(damageInfo->target, &damageInfo->damage, damageInfo->attackType);

    if (damageInfo->schoolMask & SPELL_SCHOOL_MASK_NORMAL)
        damageInfo->damage = CalcArmorReducedDamage(damageInfo->target, damageInfo->damage);

    damageInfo->rageDamage = damageInfo->damage;
    RollMeleeHit(damageInfo);

    SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: rageDamage = %d", damageInfo->target, damageInfo->rageDamage);

    // Calculate absorb resist
    if (int32(damageInfo->damage) > 0)
    {
        // Calculate absorb & resists
        CalcAbsorbResist(damageInfo->target, SpellSchoolMask(damageInfo->schoolMask), DIRECT_DAMAGE, damageInfo->damage, &damageInfo->absorb, &damageInfo->resist);
        damageInfo->damage -= damageInfo->absorb + damageInfo->resist;

        if (damageInfo->absorb)
        {
            damageInfo->rageDamage = damageInfo->rageDamage <= damageInfo->absorb ? 0 : damageInfo->rageDamage - damageInfo->absorb;
            damageInfo->hitInfo |= HITINFO_ABSORB;
            damageInfo->procEx  |= PROC_EX_ABSORB;
        }
        else
            damageInfo->procEx |= PROC_EX_DIRECT_DAMAGE;

        if (damageInfo->resist)
            damageInfo->hitInfo |= HITINFO_RESIST;

        if (damageInfo->damage)
            damageInfo->procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;
    }
    else // Umpossible get negative result but....
        damageInfo->damage = 0;
}

void Unit::DealMeleeDamage(MeleeDamageLog *damageInfo, bool durabilityLoss)
{
    if (!damageInfo)
        return;
    Unit *pVictim = damageInfo->target;

    if (!this || !pVictim)
        return;

    if (!pVictim->isAlive() || pVictim->IsTaxiFlying() || pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode())
        return;

    //You don't lose health from damage taken from another player while in a sanctuary
    //You still see it in the combat log though
    if (pVictim != this && GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER && pVictim->isInSanctuary())
        return;

    // Hmmmm dont like this emotes cloent must by self do all animations
    if (damageInfo->hitInfo & HITINFO_CRITICALHIT)
        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);

    if (damageInfo->blocked && damageInfo->targetState != VICTIMSTATE_BLOCKS)
        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYSHIELD);

    if (damageInfo->targetState == VICTIMSTATE_PARRY)
    {
        // Get attack timers
        float offtime  = float(pVictim->getAttackTimer(OFF_ATTACK));
        float basetime = float(pVictim->getAttackTimer(BASE_ATTACK));
        // Reduce attack time
        if (pVictim->GetTypeId() != TYPEID_PLAYER && ((Creature*)pVictim)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN)
        {
            ; // No parry haste
        }
        else if (pVictim->haveOffhandWeapon() && offtime < basetime)
        {
            float percent20 = pVictim->GetAttackTime(OFF_ATTACK) * 0.20;
            float percent60 = 3 * percent20;
            if (offtime > percent20 && offtime <= percent60)
            {
                pVictim->setAttackTimer(OFF_ATTACK, uint32(percent20));
            }
            else if (offtime > percent60)
            {
                offtime -= 2 * percent20;
                pVictim->setAttackTimer(OFF_ATTACK, uint32(offtime));
            }
        }
        else
        {
            float percent20 = pVictim->GetAttackTime(BASE_ATTACK) * 0.20;
            float percent60 = 3 * percent20;
            if (basetime > percent20 && basetime <= percent60)
            {
                pVictim->setAttackTimer(BASE_ATTACK, uint32(percent20));
            }
            else if (basetime > percent60)
            {
                basetime -= 2 * percent20;
                pVictim->setAttackTimer(BASE_ATTACK, uint32(basetime));
            }
        }
    }

    // Call default DealDamage
    bool duel_just_ended = false;
    DealDamage(damageInfo, DIRECT_DAMAGE, NULL, durabilityLoss, duel_just_ended);

    // If this is a creature and it attacks from behind it has a probability to daze it's victim when dealing damage
    if (damageInfo->damage && damageInfo->targetState == VICTIMSTATE_NORMAL &&
        GetTypeId() != TYPEID_PLAYER && !((Creature*)this)->GetCharmerOrOwnerGUID() && !pVictim->HasInArc(M_PI, this)
        && (pVictim->GetTypeId() == TYPEID_PLAYER || !((Creature*)pVictim)->isWorldBoss()))
    {
        // -probability is between 0% and 40%
        // 20% base chance
        float Probability = 20;

        //there is a newbie protection, at level 10 just 7% base chance; assuming linear function
        if (pVictim->GetLevel() < 30)
            Probability = 0.65f*pVictim->GetLevel()+0.5;

        uint32 VictimDefense=pVictim->GetDefenseSkillValue();
        uint32 AttackerMeleeSkill=GetUnitMeleeSkill();
        int32 SkillDiff = VictimDefense - AttackerMeleeSkill;   // with 125 difference as defense "capp"

        Probability -= SkillDiff/6.25;  //linear factor here with 0% for "capp" value

        if (Probability > 40)
            Probability = 40;

        if (Probability > 0 && roll_chance_f(Probability))
            CastSpell(pVictim, 1604, true);
    }

    // update at damage Judgement aura duration that applied by attacker at victim
    if (damageInfo->damage)
    {
        AuraMap& vAuras = pVictim->GetAuras();
        for (AuraMap::iterator itr = vAuras.begin(); itr != vAuras.end(); ++itr)
        {
            SpellEntry const *spellInfo = (*itr).second->GetSpellProto();
            if (spellInfo->AttributesEx3 & 0x40000 && spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && ((*itr).second->GetCasterGUID() == GetGUID()))
            {
                (*itr).second->SetAuraDuration((*itr).second->GetAuraMaxDuration());
                (*itr).second->UpdateAuraDuration();
            }
        }
    }

    if (GetTypeId() == TYPEID_PLAYER)
        ((Player *)this)->CastItemCombatSpell(pVictim, damageInfo->attackType, damageInfo->procVictim, damageInfo->procEx);

    if (duel_just_ended)
        return;

    // Do effect if any damage done to target
    if (damageInfo->procVictim & PROC_FLAG_TAKEN_ANY_DAMAGE)
    {
        // victim's damage shield
        std::set<Aura*> alreadyDone;
        uint32 removedAuras = pVictim->m_removedAurasCount;
        AuraList const& vDamageShields = pVictim->GetAurasByType(SPELL_AURA_DAMAGE_SHIELD);
        for (AuraList::const_iterator i = vDamageShields.begin(), next = vDamageShields.begin(); i != vDamageShields.end(); i = next)
        {
           next++;
           if (alreadyDone.find(*i) == alreadyDone.end())
           {
               alreadyDone.insert(*i);
               uint32 damage=(*i)->GetModifier()->m_amount;

               SpellEntry const *spellProto = sSpellTemplate.LookupEntry<SpellEntry>((*i)->GetId());
               if (!spellProto)
                   continue;

               SpellMissInfo missInfo = pVictim->SpellHitResult(this, spellProto, false, true);
               if(missInfo != SPELL_MISS_NONE)
               {
                   pVictim->SendSpellMiss(this, spellProto->Id, missInfo);
                   continue;
               }

               damage = pVictim->SpellDamageBonus(this, spellProto, damage, SPELL_DIRECT_DAMAGE);

               WorldPacket data(SMSG_SPELLDAMAGESHIELD,(8+8+4+4+4));
               data << uint64(pVictim->GetGUID());
               data << uint64(GetGUID());
               data << uint32(sSpellMgr.GetSpellAnalog((spellProto)));
               data << uint32(damage);
               data << uint32(spellProto->SchoolMask);
               pVictim->BroadcastPacket(&data, true);

               pVictim->DealDamage(this, damage, SPELL_DIRECT_DAMAGE, SpellMgr::GetSpellSchoolMask(spellProto), spellProto, true);

               if (pVictim->m_removedAurasCount > removedAuras)
               {
                   removedAuras = pVictim->m_removedAurasCount;
                   next = vDamageShields.begin();
               }
           }
        }
    }

    ProcDamageAndSpell(damageInfo->target, damageInfo->procAttacker, damageInfo->procVictim, damageInfo->procEx, damageInfo->damage, damageInfo->attackType);
}


void Unit::HandleEmoteCommand(uint32 anim_id)
{
    WorldPacket data(SMSG_EMOTE, 12);
    data << anim_id << GetGUID();
    ASSERT(data.size() == 12);

    BroadcastPacket(&data, true);
}

void Unit::HandleEmote(uint32 emote_id)
{
    if (!emote_id)
        SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
    else if (EmotesEntry const* emoteEntry = sEmotesStore.LookupEntry(emote_id))
    {
        if (emoteEntry->EmoteType) // 1,2 states, 0 command
            SetUInt32Value(UNIT_NPC_EMOTESTATE, emote_id);
        else
            HandleEmoteCommand(emote_id);
    }
}

uint32 Unit::CalcArmorReducedDamage(Unit* pVictim, const uint32 damage)
{
    uint32 newdamage = 0;
    float armor = pVictim->GetArmor();
    // Ignore enemy armor by SPELL_AURA_MOD_TARGET_RESISTANCE aura
    armor += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, SPELL_SCHOOL_MASK_NORMAL);

    if (armor < 0.0f)
        armor = 0.0f;

    float levelModifier = GetLevel();
    if (levelModifier > 59)
        levelModifier = levelModifier + (4.5f * (levelModifier-59));

    float tmpvalue = 0.1f * armor / (8.5f * levelModifier + 40);
    tmpvalue = tmpvalue/(1.0f + tmpvalue);

    if (tmpvalue < 0.0f)
        tmpvalue = 0.0f;

    if (tmpvalue > 0.75f)
        tmpvalue = 0.75f;

    newdamage = uint32(damage - (damage * tmpvalue));

    return (newdamage > 1) ? newdamage : 1;
}

void Unit::CalcAbsorbResist(Unit *pVictim, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 & damage, uint32 *absorb, uint32 *resist)
{
    if (!pVictim || !pVictim->isAlive() || !damage)
        return;

    // Magic damage, check for resists
    if (schoolMask & ~SPELL_SCHOOL_MASK_NORMAL)
    {
        // Get base victim resistance for school
        float victimResistance = (float)pVictim->GetResistance(GetFirstSchoolInMask(schoolMask));
        // Ignore resistance by self SPELL_AURA_MOD_TARGET_RESISTANCE aura
        if(GetTypeId() == TYPEID_PLAYER)
            victimResistance += (float)GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE);
        else
            victimResistance += (float)GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, schoolMask);

        if (Player* player = ToPlayer())
            victimResistance -= float(player->GetSpellPenetrationItemMod());

        if (victimResistance < 0.0f || schoolMask & SPELL_SCHOOL_MASK_HOLY)
            victimResistance = 0.0f;

        if (Creature* pCre = pVictim->ToCreature())
        {
            int32 leveldiff = int32(pCre->getLevelForTarget(this)) - int32(getLevelForTarget(pCre));
            if (leveldiff > 0)
                victimResistance += leveldiff * 5;
        }

        if (GetObjectGuid().IsCreature() && this->ToCreature()->GetEntry() == 25166 && schoolMask & SPELL_SCHOOL_MASK_FIRE)
            victimResistance = 0.0f;

        victimResistance *= (float)(0.15f / GetLevel());
        if (victimResistance < 0.0f)
            victimResistance = 0.0f;
        if (victimResistance > 0.75f)
            victimResistance = 0.75f;
        float ran = (float)rand_norm();
        uint32 faq[4] = {1,4,6,4};
        uint8 m = 0;
        float Binom = 0.0f;
        for (uint8 i = 0; i < 4; i++)
        {
            Binom += (powf(victimResistance, i) * powf((1-victimResistance), (4-i)))*faq[i];
            if (ran > Binom)
                ++m;
            else
                break;
        }
        if (damagetype == DOT && m == 4)
            *resist += uint32(damage - 1);
        else
            *resist += uint32(damage * m / 4);
        if (*resist > damage)
            *resist = damage;
    }
    else
        *resist = 0;

    CalcAbsorb(pVictim, schoolMask, damage, absorb, resist);
}

void Unit::CalcAbsorb(Unit *pVictim,SpellSchoolMask schoolMask, const uint32 damage, uint32 *absorb, uint32 *resist)
{
    if (!pVictim || !pVictim->isAlive() || !damage)
        return;

    int32 RemainingDamage = damage - *resist;

    // Need to remove expired auras after
    bool expiredExists = false;

    // absorb without mana cost
    int32 reflectDamage = 0;
    Aura* reflectAura = NULL;
    AuraList const& vSchoolAbsorb = pVictim->GetAurasByType(SPELL_AURA_SCHOOL_ABSORB);
    for (AuraList::const_iterator i = vSchoolAbsorb.begin(); i != vSchoolAbsorb.end() && RemainingDamage > 0; ++i)
    {
        int32 *p_absorbAmount = &(*i)->GetModifier()->m_amount;

        // should not happen....
        if (*p_absorbAmount <=0)
        {
            expiredExists = true;
            continue;
        }

        if (((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
            continue;

        if ((*i)->GetId() == 40251)  //for NOT remove Shadow of Death aura when dmg > absorb value
            continue;

        // Cheat Death
        if ((*i)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_ROGUE && (*i)->GetSpellProto()->SpellIconID == 2109 && pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)pVictim)->HasSpellCooldown(31231))
            {
                if (Aura* reducer = pVictim->GetDummyAura(45182))
                {
                    float mod = -((Player*)pVictim)->GetRatingBonusValue(CR_CRIT_TAKEN_MELEE)*8.0f; // % of damage reduced
                    if (mod < reducer->GetModifier()->m_amount) // up to 90%
                        mod = reducer->GetModifier()->m_amount;
                    RemainingDamage *= (mod + 100.0f)/100; // never negative
                }
                continue;
            }

            if (pVictim->GetHealth() <= RemainingDamage)
            {
                int32 chance = *p_absorbAmount;
                if (roll_chance_i(chance))
                {
                    pVictim->CastSpell(pVictim,31231,true);
                    ((Player*)pVictim)->AddSpellCooldown(31231,time(NULL)+60);

                    // with health > 10% lost health until health==10%, in other case no losses
                    uint32 health10 = pVictim->GetMaxHealth()/10;
                    RemainingDamage = pVictim->GetHealth() > health10 ? pVictim->GetHealth() - health10 : 0;
                }
            }
            continue;
        }

        int32 currentAbsorb;

        //Reflective Shield
        if ((pVictim != this))
        {
            if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_PRIEST && (*i)->GetSpellProto()->SpellFamilyFlags == 0x1)
            {
                if (Unit* caster = (*i)->GetCaster())
                {
                    AuraList const& vOverRideCS = caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (AuraList::const_iterator k = vOverRideCS.begin(); k != vOverRideCS.end(); ++k)
                    {
                        switch ((*k)->GetModifier()->m_miscvalue)
                        {
                            case 5065:                          // Rank 1
                            case 5064:                          // Rank 2
                            case 5063:                          // Rank 3
                            case 5062:                          // Rank 4
                            case 5061:                          // Rank 5
                            {
                                if (RemainingDamage >= *p_absorbAmount)
                                    reflectDamage = *p_absorbAmount * (*k)->GetModifier()->m_amount/100;
                                else
                                    reflectDamage = (*k)->GetModifier()->m_amount * RemainingDamage/100;
                                reflectAura = *i;

                            } break;
                            default: break;
                        }

                        if (reflectDamage)
                            break;
                    }
                }
            }
            else
            {                       //Lady Malandes Reflective Shield
                if ((*i)->GetSpellProto()->Id == 41475)
                {
                    if (RemainingDamage >= *p_absorbAmount)
                        reflectDamage = *p_absorbAmount * 0.5;
                    else
                        reflectDamage = RemainingDamage * 0.5;
                    reflectAura = *i;
                }
            }
        }

        if (RemainingDamage >= *p_absorbAmount)
        {
            currentAbsorb = *p_absorbAmount;
            expiredExists = true;
        }
        else
        {
            currentAbsorb = RemainingDamage;
        }

        *p_absorbAmount -= currentAbsorb;
        RemainingDamage -= currentAbsorb;
    }
    // do not cast spells while looping auras; auras can get invalid otherwise
    if (reflectDamage)
        pVictim->CastCustomSpell(this, 33619, &reflectDamage, NULL, NULL, true, NULL, reflectAura);

    // Remove all expired absorb auras
    if (expiredExists)
    {
        for (AuraList::const_iterator i = vSchoolAbsorb.begin(); i != vSchoolAbsorb.end();)
        {
            Aura *aur = (*i);
            ++i;
            // Balance of Power cosmetics workaround, do not remove this aura when expires
            if (aur->GetId() == 41341)
                continue;
            if (aur->GetModifier()->m_amount <= 0)
            {
                uint32 removedAuras = pVictim->m_removedAurasCount;
                pVictim->RemoveAurasDueToSpell(aur->GetId());
                if (removedAuras + 1 < pVictim->m_removedAurasCount)
                    i = vSchoolAbsorb.begin();
            }
        }
    }

    // absorb by mana cost
    AuraList const& vManaShield = pVictim->GetAurasByType(SPELL_AURA_MANA_SHIELD);
    for (AuraList::const_iterator i = vManaShield.begin(), next; i != vManaShield.end() && RemainingDamage > 0; i = next)
    {
        next = i; ++next;
        int32 *p_absorbAmount = &(*i)->GetModifier()->m_amount;

        // check damage school mask
        if (((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
            continue;

        int32 currentAbsorb;
        if (RemainingDamage >= *p_absorbAmount)
            currentAbsorb = *p_absorbAmount;
        else
            currentAbsorb = RemainingDamage;

        float manaMultiplier = (*i)->GetSpellProto()->EffectMultipleValue[(*i)->GetEffIndex()];
        if (Player *modOwner = pVictim->GetSpellModOwner())
            modOwner->ApplySpellMod((*i)->GetId(), SPELLMOD_MULTIPLE_VALUE, manaMultiplier);

        if (manaMultiplier)
        {
            int32 maxAbsorb = int32(pVictim->GetPower(POWER_MANA) / manaMultiplier);
            if (currentAbsorb > maxAbsorb)
                currentAbsorb = maxAbsorb;
        }

        *p_absorbAmount -= currentAbsorb;
        if (*p_absorbAmount <= 0)
        {
            pVictim->RemoveAurasDueToSpell((*i)->GetId());
            next = vManaShield.begin();
        }

        int32 manaReduction = int32(currentAbsorb * manaMultiplier);
        pVictim->ApplyPowerMod(POWER_MANA, manaReduction, false);

        RemainingDamage -= currentAbsorb;
    }
    // only split damage if not damaging yourself
    if (pVictim != this)
    {
        bool duel_just_ended = false;
        
        AuraList const& vSplitDamageFlat = pVictim->GetAurasByType(SPELL_AURA_SPLIT_DAMAGE_FLAT);
        for (AuraList::const_iterator i = vSplitDamageFlat.begin(), next; i != vSplitDamageFlat.end() && RemainingDamage >= 0; i = next)
        {
            next = i; ++next;
            int32 *p_absorbAmount = &(*i)->GetModifier()->m_amount;

            // check damage school mask
            if (((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
                continue;

            // Damage can be splitted only if aura has an alive caster
            Unit *caster = (*i)->GetCaster();
            if (!caster || caster == pVictim || !caster->IsInWorld() || !caster->isAlive() || caster->IsImmunedToDamage((SpellSchoolMask)(*i)->GetSpellProto()->SchoolMask))
                continue;

            int32 currentAbsorb;
            if (RemainingDamage >= (*i)->GetModifier()->m_amount)
                currentAbsorb = (*i)->GetModifier()->m_amount;
            else
                currentAbsorb = RemainingDamage;

            RemainingDamage -= currentAbsorb;

            SpellDamageLog damageInfo(sSpellMgr.GetSpellAnalog((*i)->GetSpellProto()), this, caster, (*i)->GetSpellProto()->SchoolMask);
            damageInfo.damage = currentAbsorb;

            DealDamage(&damageInfo, DOT, (*i)->GetSpellProto(), false, duel_just_ended);
        }

        AuraList const& vSplitDamagePct = pVictim->GetAurasByType(SPELL_AURA_SPLIT_DAMAGE_PCT);
        for (AuraList::const_iterator i = vSplitDamagePct.begin(), next; i != vSplitDamagePct.end() && RemainingDamage >= 0; i = next)
        {
            next = i; ++next;

            // check damage school mask
            if (((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
               continue;

            // Damage can be splitted only if aura has an alive caster
            Unit *caster = (*i)->GetCaster();
            if (!caster || caster == pVictim || !caster->IsInWorld() || !caster->isAlive() || caster->IsImmunedToDamage((SpellSchoolMask)(*i)->GetSpellProto()->SchoolMask))
                continue;

            int32 splitted = int32(RemainingDamage * (*i)->GetModifier()->m_amount / 100.0f);

            RemainingDamage -= splitted;

            SpellDamageLog damageInfo(sSpellMgr.GetSpellAnalog((*i)->GetSpellProto()), this, caster, (*i)->GetSpellProto()->SchoolMask);
            damageInfo.damage = splitted;

            DealDamage(&damageInfo, DOT, (*i)->GetSpellProto(), false, duel_just_ended);
        }
    }

    *absorb = damage - RemainingDamage - *resist;
}

bool Unit::CalcBinaryResist(Unit *pVictim, SpellSchoolMask schoolMask) {

    if (!pVictim || !pVictim->isAlive())
        return false;

    // Magic damage, check for resists
    if (schoolMask & ~SPELL_SCHOOL_MASK_NORMAL)
    {
        // Get base victim resistance for school
        float effectiveResistance = (float)pVictim->GetResistance(GetFirstSchoolInMask(schoolMask));
        // Ignore resistance by self SPELL_AURA_MOD_TARGET_RESISTANCE aura
        if(GetTypeId() == TYPEID_PLAYER)
            effectiveResistance += (float)GetInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE);
        else
            effectiveResistance += (float)GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, schoolMask);

        effectiveResistance *= (float)(0.15f / GetLevel());

        if (effectiveResistance < 0.0f)
            effectiveResistance = 0.0f;
        if (effectiveResistance > 0.75f)
            effectiveResistance = 0.75f;

        float ran = (float)rand_norm();
        return ran < effectiveResistance;
    }

    return false;
}

void Unit::AttackerStateUpdate(Unit *pVictim, WeaponAttackType attType, bool extra)
{
    if (!extra && HasUnitState(UNIT_STAT_CANNOT_AUTOATTACK) || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
        return;

	// don't allow to combat with non attackable units
	if (isCharmedOwnedByPlayerOrPlayer() && pVictim->HasFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE)))
		return;

    if (!pVictim->isAlive())
        return;

    if (pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        if (dynamic_cast<Player *>(pVictim)->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY))
            AttackStop();
    }

    CombatStart(pVictim);
    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ATTACK);

    // melee attack spell cast at main hand attack only
    if (attType == BASE_ATTACK && m_currentSpells[CURRENT_MELEE_SPELL] && !extra)
    {
        m_currentSpells[CURRENT_MELEE_SPELL]->cast();
        return;
    }

    if (pVictim->HasAuraType(SPELL_AURA_ADD_CASTER_HIT_TRIGGER))
    {
        Unit::AuraList const& hitTriggerAuras = pVictim->GetAurasByType(SPELL_AURA_ADD_CASTER_HIT_TRIGGER);
        for (Unit::AuraList::const_iterator itr = hitTriggerAuras.begin(); itr != hitTriggerAuras.end(); ++itr)
        {
            if (Unit* hitTarget = (*itr)->GetCaster())
            {
                if(!hitTarget->isAlive())
                    continue;

                if ((*itr)->m_procCharges > 0)
                {
                    (*itr)->SetAuraProcCharges((*itr)->m_procCharges-1);
                    (*itr)->UpdateAuraCharges();
                    if ((*itr)->m_procCharges <= 0)
                        pVictim->RemoveAurasByCasterSpell((*itr)->GetId(), (*itr)->GetCasterGUID());
                }
                pVictim = hitTarget;
                break;
            }
        }
    }

    MeleeDamageLog damageInfo(this, pVictim, this->GetMeleeDamageSchoolMask(), attType);
    CalculateMeleeDamage(&damageInfo);

    DealMeleeDamage(&damageInfo, true);
}

void Unit::HandleProcExtraAttackFor(Unit* victim)
{
    if (m_extraAttacks)
    {
        while (m_extraAttacks)
        {
            AttackerStateUpdate(victim, BASE_ATTACK, true);
            if (m_extraAttacks > 0)
                --m_extraAttacks;
        }
    }
}

void Unit::RollMeleeHit(MeleeDamageLog *damageInfo) const
{
    // This is only wrapper

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_DPS_TESTING)
    {
        RollMeleeHit(damageInfo, int32(10000), int32(0), int32(0), int32(0), int32(0));
        return;
    }

    // Miss chance based on melee
    //float miss_chance = MeleeMissChanceCalc(pVictim, attType);
    float miss_chance = MeleeSpellMissChance(damageInfo->target, damageInfo->attackType, int32(GetWeaponSkillValue(damageInfo->attackType, damageInfo->target)) - int32(damageInfo->target->GetDefenseSkillValue(this)), 0);

    // Critical hit chance
    float crit_chance = GetUnitCriticalChance(damageInfo->attackType, damageInfo->target);

    // stunned target cannot dodge and this is check in GetUnitDodgeChance() (returned 0 in this case)
    float dodge_chance = damageInfo->target->GetUnitDodgeChance();
    if(HasAuraType(SPELL_AURA_MOD_ENEMY_DODGE)) // TRENTONE CHECK
        dodge_chance += GetTotalAuraModifier(SPELL_AURA_MOD_ENEMY_DODGE);
    if(dodge_chance < 0)
        dodge_chance = 0;

    float block_chance = damageInfo->target->GetUnitBlockChance();
    float parry_chance = damageInfo->target->GetUnitParryChance();

    // Useful if want to specify crit & miss chances for melee, else it could be removed
    debug_log("MELEE OUTCOME: miss %f crit %f dodge %f parry %f block %f", miss_chance,crit_chance,dodge_chance,parry_chance,block_chance);

    RollMeleeHit(damageInfo, int32(crit_chance*100), int32(miss_chance*100), int32(dodge_chance*100),int32(parry_chance*100),int32(block_chance*100));
}

void Unit::RollMeleeHit(MeleeDamageLog *damageInfo, int32 crit_chance, int32 miss_chance, int32 dodge_chance, int32 parry_chance, int32 block_chance) const
{
    Unit *pVictim = damageInfo->target;
    WeaponAttackType attType =  damageInfo->attackType;

    if (pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode())
    {
        damageInfo->hitInfo    |= HITINFO_MISS|HITINFO_SWINGNOHITSOUND;
        damageInfo->targetState = VICTIMSTATE_EVADES;

        damageInfo->procEx |= PROC_EX_EVADE;
        damageInfo->damage = 0;
        damageInfo->rageDamage = 0;
        return;
    }

    int32 attackerMaxSkillValueForLevel = GetMaxSkillValueForLevel(pVictim);
    int32 victimMaxSkillValueForLevel = pVictim->GetMaxSkillValueForLevel(this);

    int32 attackerWeaponSkill = GetWeaponSkillValue(attType,pVictim);
    int32 victimDefenseSkill = pVictim->GetDefenseSkillValue(this);

    // bonus from skills is 0.04% against players and 0.1% against mobs
    int32 skillDiff  = attackerWeaponSkill - victimMaxSkillValueForLevel;
    int32 skillBonus = pVictim->GetTypeId() == TYPEID_PLAYER ? skillDiff * 4  : skillDiff * 10;
    int32 skillParryBonus = pVictim->GetTypeId() == TYPEID_PLAYER ? skillDiff * 4 : (skillBonus > 10 ? skillBonus * 60 : skillBonus * 10);
    int32 skillCritBonus = (attackerMaxSkillValueForLevel - victimDefenseSkill) * 4;
    int32 sum = 0;
    int32 roll = urand (0, 10000);

    DEBUG_LOG ("RollMeleeOutcomeAgainst: skill bonus of %d for attacker", skillBonus);
    DEBUG_LOG ("RollMeleeOutcomeAgainst: rolled %d, miss %d, dodge %d, parry %d, block %d, crit %d",
        roll, miss_chance, dodge_chance, parry_chance, block_chance, crit_chance);

    sum += miss_chance;
    SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: miss chance = %d", pVictim, miss_chance);

    if (roll < sum)
    {
        damageInfo->hitInfo    |= HITINFO_MISS;
        damageInfo->targetState = VICTIMSTATE_NORMAL;

        damageInfo->procEx |= PROC_EX_MISS;
        damageInfo->damage = 0;
        damageInfo->rageDamage = 0;
        return;
    }

    int32 expertise_reduction = 0;
    if (GetTypeId() == TYPEID_PLAYER)
        expertise_reduction = int32(((Player*)this)->GetExpertiseDodgeOrParryReduction(attType)*100);

    bool fromBehind = !pVictim->HasInArc(M_PI, this);

    if (!pVictim->IsStandState())
    {
        crit_chance = 10000;
        dodge_chance = 0;
        parry_chance = 0;
        block_chance = 0;
    }
    else if (fromBehind)
    {
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            dodge_chance = 0;

        parry_chance = 0;
        block_chance = 0;
    }

    if (pVictim->HasUnitState(UNIT_STAT_CASTING))
    {
        dodge_chance = 0;
        parry_chance = 0;
        block_chance = 0;
    }

    sLog.outDebug("RollMeleeOutcomeAgainst: skill bonus of %d for attacker", skillBonus);
    sLog.outDebug("RollMeleeOutcomeAgainst: rolled %d, miss %d, dodge %d, parry %d, block %d, crit %d",
        roll, miss_chance, dodge_chance, parry_chance, block_chance, crit_chance);

    if (dodge_chance)
    {
        dodge_chance -= expertise_reduction + skillBonus;
        // Modify dodge chance by attacker SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
        dodge_chance += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_COMBAT_RESULT_CHANCE, VICTIMSTATE_DODGE)*100;
        // Modify dodge chance by SPELL_AURA_MOD_ENEMY_DODGE
        dodge_chance += GetTotalAuraModifier(SPELL_AURA_MOD_ENEMY_DODGE) * 100;
        if (dodge_chance > 0)
        {
            SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: dodge chance = %d", pVictim, dodge_chance);
            sum += dodge_chance;
            if (roll < sum)
            {
                damageInfo->targetState  = VICTIMSTATE_DODGE;
                damageInfo->procEx |= PROC_EX_DODGE;
                damageInfo->rageDamage = damageInfo->damage;
                damageInfo->damage = 0;
                return;
            }
        }
    }

    if (parry_chance)
    {
        parry_chance -= expertise_reduction + skillParryBonus;
        if (parry_chance > 0)
        {
            SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: parry chance = %d", pVictim, parry_chance);
            sum += parry_chance;
            if (roll < sum)
            {
                damageInfo->targetState  = VICTIMSTATE_PARRY;
                damageInfo->procEx |= PROC_EX_PARRY;
                damageInfo->rageDamage = damageInfo->damage;
                damageInfo->damage = 0;
                return;
            }
        }
    }

    // Max 40% chance to score a glancing blow against mobs that are higher level (can do only players and pets and not with ranged weapon)
    if (GetLevel() < pVictim->getLevelForTarget(this) &&
        (GetTypeId() == TYPEID_PLAYER || ((Creature*)this)->isPet())  &&
        pVictim->GetTypeId() != TYPEID_PLAYER && !((Creature*)pVictim)->isPet())
    {
        // cap possible value (with bonuses > max skill)
        int32 skill = attackerWeaponSkill;
        int32 maxskill = attackerMaxSkillValueForLevel;
        skill = (skill > maxskill) ? maxskill : skill;

        int32 glancing_chance = (10 + (victimDefenseSkill - skill)) * 100;
        glancing_chance = glancing_chance > 4000 ? 4000 : glancing_chance;
        SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: glancing chance = %d", pVictim, glancing_chance);
        sum += glancing_chance;
        if (roll < sum)
        {
            damageInfo->hitInfo |= HITINFO_GLANCING;
            damageInfo->targetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx |= PROC_EX_NORMAL_HIT;
            int32 leveldif = int32(pVictim->GetLevel()) - int32(GetLevel());
            if (leveldif > 3) leveldif = 3;
            float reducePercent = 1 - leveldif * 0.1f;
            damageInfo->damage   = uint32(reducePercent *  damageInfo->damage);
            damageInfo->rageDamage = damageInfo->damage;
            return;
        }
    }

    if (GetTypeId() == TYPEID_UNIT && ((Creature *)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_BLOCK_ON_ATTACK)
        block_chance = 0;

    if (Player* player = const_cast<Player*>(pVictim->ToPlayer()))
    {
        Item *tmpitem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (!tmpitem || !tmpitem->GetProto()->Block || tmpitem->IsBroken())
            block_chance = 0;
    }

    if (block_chance)
    {
        block_chance -= skillBonus;
        if (block_chance > 0)
        {
            SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: block chance = %d", pVictim, block_chance);
            sum += block_chance;
            if (roll < sum)
            {
                damageInfo->targetState = VICTIMSTATE_NORMAL;
                damageInfo->procEx |= PROC_EX_BLOCK;
                damageInfo->blocked = damageInfo->target->GetShieldBlockValue();

                if (damageInfo->blocked >= damageInfo->damage)
                {
                    damageInfo->targetState = VICTIMSTATE_BLOCKS;
                    damageInfo->blocked = damageInfo->damage;
                }
                else
                    damageInfo->procEx |= PROC_EX_NORMAL_HIT;     // Partial blocks can still cause attacker procs

                damageInfo->rageDamage = damageInfo->damage;
                damageInfo->damage -= damageInfo->blocked;
                return;
            }
        }
    }

    if (crit_chance)
    {
        crit_chance += skillCritBonus;
        if (crit_chance > 0)
        {
            SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: crit chance = %d", pVictim, crit_chance);
            sum += crit_chance;
            if (roll < sum)
            {
                damageInfo->hitInfo     |= HITINFO_CRITICALHIT;
                damageInfo->targetState  = VICTIMSTATE_NORMAL;

                damageInfo->procEx |= PROC_EX_CRITICAL_HIT;

                // critical bonus damage calculation
                damageInfo->damage += damageInfo->damage;
                int32 mod = 0;

                mod += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS);
                mod += damageInfo->target->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE);

                uint32 crTypeMask = damageInfo->target->GetCreatureTypeMask();

                // Increase crit damage from SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
                mod += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, crTypeMask);
                if (mod != 0)
                    damageInfo->damage = int32((damageInfo->damage) * float((100.0f + mod)/100.0f));

                // Resilience - reduce crit damage
                if (pVictim->GetTypeId() == TYPEID_PLAYER)
                {
                    uint32 resilienceReduction = ((Player*)pVictim)->GetMeleeCritDamageReduction(damageInfo->damage);
                    damageInfo->damage      -= resilienceReduction;
                }

                damageInfo->rageDamage = damageInfo->damage;
                return;
            }
        }
    }

    if (ToCreature() && !(ToCreature()->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_CRUSH) && !((Creature*)this)->isPet()) /*Only autoattack can be crushing blow*/
    {
        // mobs can score crushing blows if they're 3 or more levels above victim
        // or when their weapon skill is 15 or more above victim's defense skill
        int32 crushing_chance = victimDefenseSkill;
        int32 tmpmax = victimMaxSkillValueForLevel;
        // having defense above your maximum (from items, talents etc.) has no effect
        crushing_chance = crushing_chance > tmpmax ? tmpmax : crushing_chance;
        // tmp = mob's level * 5 - player's current defense skill
        crushing_chance = attackerMaxSkillValueForLevel - crushing_chance;
        if (crushing_chance >= 15)
        {
            // add 2% chance per lacking skill point, min. is 15%
            crushing_chance = crushing_chance * 200 - 1500;
            SendCombatStats(1<<COMBAT_STATS_MELEE_ROLL, "RollMeleeHit: crushing chance = %d", pVictim, crushing_chance);
            sum += crushing_chance;
            if (roll < sum)
            {
                damageInfo->hitInfo     |= HITINFO_CRUSHING;
                damageInfo->targetState  = VICTIMSTATE_NORMAL;
                damageInfo->procEx |= PROC_EX_NORMAL_HIT;
                // 150% normal damage
                damageInfo->damage += (damageInfo->damage / 2);
                damageInfo->rageDamage = damageInfo->damage;
                return;
            }
        }
    }

    damageInfo->targetState = VICTIMSTATE_NORMAL;
    damageInfo->procEx |= PROC_EX_NORMAL_HIT;
    return;
}

uint32 Unit::CalculateDamage (WeaponAttackType attType, bool normalized)
{
    float min_damage, max_damage;

    if (normalized && GetTypeId()==TYPEID_PLAYER)
        ((Player*)this)->CalculateMinMaxDamage(attType,normalized,min_damage, max_damage);
    else
    {
        switch (attType)
        {
            case RANGED_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
                break;
            case BASE_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXDAMAGE);
                break;
            case OFF_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
                break;
                // Just for good manner
            default:
                min_damage = 0.0f;
                max_damage = 0.0f;
                break;
        }
    }

    if (min_damage > max_damage)
    {
        std::swap(min_damage,max_damage);
    }

    if (max_damage == 0.0f)
        max_damage = 5.0f;

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_DPS_TESTING)
        return round((min_damage + max_damage) / 2);

    return urand((uint32)min_damage, (uint32)max_damage);
}

float Unit::CalculateLevelPenalty(SpellEntry const* spellProto) const
{
    if (spellProto->spellLevel == 0 || spellProto->maxLevel == 0)
        return 1.0f;

    if (spellProto->Id == 28880) // Gift of Naaru, TODO: more general check for racial spells
        return 1.0f;

    float lvlPenalty = 0.0f;

    // should we check spellLevel, baseLevel or levelReq ? Oo
    if (spellProto->spellLevel < 20)
        lvlPenalty = (20.0f - spellProto->spellLevel) * 3.75f;

    // next rank min lvl + 5 = current rank maxLevel + 6 for most spells
    float lvlFactor = (float(spellProto->maxLevel) + 6.0f) / float(GetLevel());
    if (lvlFactor > 1.0f)
        lvlFactor = 1.0f;

    return (100.0f - lvlPenalty) * lvlFactor / 100.0f;
}

void Unit::SendMeleeAttackStart(uint64 victimGUID)
{
    WorldPacket data(SMSG_ATTACKSTART, 8+8);
    data << uint64(GetGUID());
    data << uint64(victimGUID);

    BroadcastPacket(&data, true);
    debug_log("WORLD: Sent SMSG_ATTACKSTART");
}

void Unit::SendMeleeAttackStop(Unit* victim)
{
    WorldPacket data(SMSG_ATTACKSTOP, (4+16));              // we guess size
    data << GetPackGUID();
	data << (victim ? victim->GetPackGUID() : PackedGuid()); // can be 0x00...
	data << uint32(victim && victim->isDead());              // can be 0x1
    BroadcastPacket(&data, true);
}

int32 Unit::GetCurrentSpellCastTime(uint32 spell_id) const
{
    if (Spell const * spell = FindCurrentSpellBySpellId(spell_id))
        return spell->GetCastTime();
    return 0;
}

uint32 Unit::GetCurrentSpellId() const
{
    for (int i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        if(m_currentSpells[i])
            return m_currentSpells[i]->GetSpellEntry()->Id;

    return 0;
}

Spell* Unit::GetCurrentSpell(CurrentSpellTypes type) const
{
    return m_currentSpells[type];
}

SpellEntry const* Unit::GetCurrentSpellProto(CurrentSpellTypes type) const
{
    return (m_currentSpells[type] ? m_currentSpells[type]->GetSpellEntry() : NULL);
}

// Melee based spells can be miss, parry or dodge on this step
// Crit or block - determined on damage calculation phase! (and can be both in some time)
float Unit::MeleeSpellMissChance(const Unit *pVictim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const
{
    // Calculate hit chance (more correct for chance mod)
    int32 HitChance;

    // PvP - PvE melee chances
    if (spellId || attType == RANGED_ATTACK || !haveOffhandWeapon())
        HitChance = 95.0f;
    else
        HitChance = 76.0f;

    // Hit chance depends from victim auras
    if (attType == RANGED_ATTACK)
        HitChance += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE);
    else
        HitChance += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE);

    // Spellmod from SPELLMOD_RESIST_MISS_CHANCE
    if (spellId)
    {
        if (pVictim->GetObjectGuid().IsCreature() && pVictim->ToCreature()->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_1PCT_TAUNT_RESIST)
        {
            const SpellEntry* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
            if (SpellMgr::IsTauntSpell(spellInfo))
                return 1.0f;
        }

        if (Player *modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod(spellId, SPELLMOD_RESIST_MISS_CHANCE, HitChance);
    }

    // Miss = 100 - hit
    float miss_chance= 100.0f - HitChance;

    if (GetTypeId() == TYPEID_UNIT && ((Creature *)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_CANT_MISS)
        return 0.0f;

    // Bonuses from attacker aura and ratings
    if (attType == RANGED_ATTACK)
        miss_chance -= m_modRangedHitChance;
    else
        miss_chance -= m_modMeleeHitChance;

    // bonus from skills is 0.04%
    // miss_chance -= skillDiff * 0.04f;
    int32 diff = -skillDiff;
    if (pVictim->GetTypeId()==TYPEID_PLAYER)
        miss_chance += diff > 0 ? diff * 0.04 : diff * 0.02;
    else
        miss_chance += diff > 10 ? 2 + (diff - 10) * 0.4 : diff * 0.1;

    // Limit miss chance from 0 to 60%
    if (miss_chance < 0.0f)
        return 0.0f;
    if (miss_chance > 60.0f)
        return 60.0f;

    return miss_chance;
}


int32 Unit::GetSpellMechanicResistChance(const SpellEntry *spell)
{
    if (!spell)
        return 0;

    if (!spell->Mechanic)
        return 0;

    return GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MECHANIC_RESISTANCE, spell->Mechanic);
}

int32 Unit::GetEffectMechanicResistChance(const SpellEntry *spell, uint8 eff)
{
    if (!spell)
        return 0;

    if (spell->Effect[eff] == 0)
        return 0;

    if (int32 effect_mech = SpellMgr::GetEffectMechanic(spell, eff))
        return GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MECHANIC_RESISTANCE, effect_mech);

    return 0;
}

// Melee based spells hit result calculations
SpellMissInfo Unit::MeleeSpellHitResult(Unit *pVictim, SpellEntry const *spell, bool cMiss)
{
    WeaponAttackType attType = BASE_ATTACK;

    if (spell->DmgClass == SPELL_DAMAGE_CLASS_RANGED)
        attType = RANGED_ATTACK;

    // bonus from skills is 0.04% per skill Diff
    int32 attackerWeaponSkill = (!((spell->DmgClass == SPELL_DAMAGE_CLASS_RANGED && !(spell->Attributes & SPELL_ATTR_RANGED)) /*example: avenger's shield or hammer of wrath*/ ||
        spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)) /* example: taunt, demoralizing roar*/
        ? int32(GetWeaponSkillValue(attType,pVictim))
        : int32(GetMaxSkillValueForLevel());

    int32 skillDiff = attackerWeaponSkill - int32(pVictim->GetMaxSkillValueForLevel(this));
    int32 fullSkillDiff = attackerWeaponSkill - int32(pVictim->GetDefenseSkillValue(this));

    uint32 roll = urand (0, 9999);

    uint32 tmp = 0;

    bool isCasting = pVictim->IsNonMeleeSpellCast(false);
    bool lostControl = pVictim->HasUnitState(UNIT_STAT_LOST_CONTROL);

    bool canDodge = !isCasting && !lostControl;
    bool canParry = !isCasting && !lostControl;
    bool canBlock = spell->AttributesEx3 & SPELL_ATTR_EX3_UNK3 && !isCasting && !lostControl;

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_DPS_TESTING)
    {
        canDodge = false;
        canParry = false;
        canBlock = false;
    }

    if (Player* player = pVictim->ToPlayer())
    {
        Item *tmpitem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (!tmpitem || !tmpitem->GetProto()->Block || tmpitem->IsBroken())
            canBlock = false;
    }

    // Creature has un-blockable attack info
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_BLOCK_ON_ATTACK)
        canBlock = false;

    //We use SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY until right Attribute was found
    bool canMiss = !(spell->Attributes & SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY) && cMiss ||
        spell->AttributesEx3 & SPELL_ATTR_EX3_CANT_MISS || spell->AttributesEx3 & SPELL_ATTR_EX3_UNK15;
    // TRENTONE CHECK SPELL_ATTR_EX3_CANT_MISS logic could be wrong

    if (canMiss)
    {
        uint32 missChance = uint32(MeleeSpellMissChance(pVictim, attType, fullSkillDiff, spell->Id)*100.0f);

        SendCombatStats(1<<COMBAT_STATS_MELEE_RESULT, "MeleeSpellHitResult (id=%d): miss chance = %d", pVictim, spell->Id, missChance);
        // Roll miss
        tmp += missChance;
        if (tmp > roll)
            return SPELL_MISS_MISS;
    }

    // Chance resist mechanic
    int32 resist_chance = pVictim->GetSpellMechanicResistChance(spell)*100;
    SendCombatStats(1<<COMBAT_STATS_MELEE_RESULT, "MeleeSpellHitResult (id=%d): mechanic resist chance = %d", pVictim, spell->Id, resist_chance);
    tmp += resist_chance;
    if (tmp > roll) // resist 0 ? 0 cant be > than 0; resist 10000 ? 10000 always > 9999
        return SPELL_MISS_RESIST;

    // Some spells cannot be parried, dodged nor blocked
    if (spell->Attributes & SPELL_ATTR_IMPOSSIBLE_DODGE_PARRY_BLOCK || spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
        return SPELL_MISS_NONE;

    // Handle ranged attacks
    if (attType == RANGED_ATTACK)
    {
        // Wand attacks can't miss
        if (spell->Category == 351)
            return SPELL_MISS_NONE;

        // Other ranged attacks cannot be parried or dodged
        // Can be blocked under suitable circumstances
        canParry = false;
        canDodge = false;
    }

    // Check for attack from behind
    if (!pVictim->HasInArc(M_PI,this))
    {
        // Can`t dodge from behind in PvP (but its possible in PvE)
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            canDodge = false;
        // Can`t parry or block
        canParry = false;
        canBlock = false;
    }

    // Rogue talent`s cant be dodged
    AuraList const& mCanNotBeDodge = GetAurasByType(SPELL_AURA_IGNORE_COMBAT_RESULT);
    for (AuraList::const_iterator i = mCanNotBeDodge.begin(); i != mCanNotBeDodge.end(); ++i)
    {
        if ((*i)->GetModifier()->m_miscvalue == VICTIMSTATE_DODGE)       // can't be dodged rogue finishing move
        {
            if (spell->SpellFamilyName==SPELLFAMILY_ROGUE && (spell->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE__FINISHING_MOVE))
            {
                canDodge = false;
                break;
            }
        }
    }

    if (canDodge)
    {
        // Roll dodge
        float dodgeChance = pVictim->GetUnitDodgeChance() * 100.0f - skillDiff * 4;
        // Reduce enemy dodge chance by SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
        dodgeChance += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_COMBAT_RESULT_CHANCE, VICTIMSTATE_DODGE) * 100.0f;
        dodgeChance += GetTotalAuraModifier(SPELL_AURA_MOD_ENEMY_DODGE) * 100.0f;
        // Reduce dodge chance by attacker expertise rating
        if (GetTypeId() == TYPEID_PLAYER)
            dodgeChance -= ((Player*)this)->GetExpertiseDodgeOrParryReduction(attType) * 100.0f;
        if (dodgeChance < 0)
            dodgeChance = 0;

        SendCombatStats(1<<COMBAT_STATS_MELEE_RESULT, "MeleeSpellHitResult (id=%d): dodge chance = %d", pVictim, spell->Id, dodgeChance);
        tmp += (int32)dodgeChance;
        if (tmp > roll)
            return SPELL_MISS_DODGE;
    }

    if (canParry)
    {
        // Roll parry
        float parryChance = pVictim->GetUnitParryChance() * 100.0f - skillDiff * 4;
        // Reduce parry chance by attacker expertise rating
        if (GetTypeId() == TYPEID_PLAYER)
            parryChance -= ((Player*)this)->GetExpertiseDodgeOrParryReduction(attType) * 100.0f;
        if (parryChance < 0)
            parryChance = 0;

        SendCombatStats(1<<COMBAT_STATS_MELEE_RESULT, "MeleeSpellHitResult (id=%d): parry chance = %d", pVictim, spell->Id, parryChance);
        tmp += (int32)parryChance;
        if (tmp > roll)
            return SPELL_MISS_PARRY;
    }

    if (canBlock)
    {
        float blockChance = pVictim->GetUnitBlockChance() * 100.0f - skillDiff * 4;
        if (blockChance < 0)
            blockChance = 0;

        SendCombatStats(1<<COMBAT_STATS_MELEE_RESULT, "MeleeSpellHitResult (id=%d): block chance = %d", pVictim, spell->Id, blockChance);
        tmp += (int32)blockChance;
        if (tmp > roll)
            return SPELL_MISS_BLOCK;
    }

    return SPELL_MISS_NONE;
}

// TODO need use unit spell resistances in calculations
SpellMissInfo Unit::MagicSpellHitResult(Unit *pVictim, SpellEntry const *spell)
{
    // Can`t miss on dead target (on skinning for example)
    if (!pVictim->isAlive())
        return SPELL_MISS_NONE;

    SpellSchoolMask schoolMask = SpellMgr::GetSpellSchoolMask(spell);
    // PvP - PvE spell misschances per leveldif > 2
    int32 lchance = pVictim->GetTypeId() == TYPEID_PLAYER ? 7 : 11;
    int32 leveldif = int32(pVictim->getLevelForTarget(this)) - int32(getLevelForTarget(pVictim));

    // Base hit chance from attacker and victim levels
    int32 modHitChance;
    if (leveldif < 3)
        modHitChance = 96 - leveldif;
    else
        modHitChance = 94 - (leveldif - 2) * lchance;

    // Spellmod from SPELLMOD_RESIST_MISS_CHANCE
    if (Player *modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spell->Id, SPELLMOD_RESIST_MISS_CHANCE, modHitChance);
    // Increase from attacker SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT auras
    modHitChance+=GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT, schoolMask);
    // Chance hit from victim SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE auras
    modHitChance+= pVictim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE, schoolMask);
    // Reduce spell hit chance for Area of effect spells from victim SPELL_AURA_MOD_AOE_AVOIDANCE aura
    if (SpellMgr::IsAreaOfEffectSpell(spell))
        modHitChance-=pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_AOE_AVOIDANCE);
    // Reduce spell hit chance for dispel mechanic spells from victim SPELL_AURA_MOD_DISPEL_RESIST
    if (SpellMgr::IsDispelSpell(spell))
        modHitChance-=pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_DISPEL_RESIST);
    // Chance resist mechanic (select max value from every mechanic spell effect)
    int32 resist_chance = pVictim->GetSpellMechanicResistChance(spell);
    // Apply mod
    modHitChance-=resist_chance;

    // Chance resist debuff
    modHitChance-=pVictim->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_DEBUFF_RESISTANCE, int32(spell->Dispel));

    int32 HitChance = modHitChance * 100;
    // Increase hit chance from attacker SPELL_AURA_MOD_SPELL_HIT_CHANCE and attacker ratings
    HitChance += int32(m_modSpellHitChance*100.0f);

    // Decrease hit chance from victim rating bonus
    if (pVictim->GetTypeId()==TYPEID_PLAYER)
        HitChance -= int32(((Player*)pVictim)->GetRatingBonusValue(CR_HIT_TAKEN_SPELL)*100.0f);

    if(SpellMgr::GetDiminishingReturnsGroupForSpell(spell, false) == DIMINISHING_ENSLAVE)
        HitChance -= int32(pVictim->GetDiminishing(DIMINISHING_ENSLAVE) * 1000);

    switch (spell->Id)
    {
        case 17639: // Wail of the Banshee AGAINST OVER 60 Level
        case 835: // Tidal Charm AGAINST OVER 60 Level
        case 29164: // Stygian Grasp AGAINST OVER 60 Level
        {
            if (pVictim->GetLevel() > 60)
                HitChance -= 5000;
            break;
        }
        default:
            break;
    }

    if (HitChance <  100) HitChance =  100;
    if (HitChance > 9900) HitChance = 9900;

    if (pVictim->GetObjectGuid().IsCreature() && pVictim->ToCreature()->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_1PCT_TAUNT_RESIST)
    {
        if (SpellMgr::IsTauntSpell(spell))
            HitChance = 9900;
    }

    uint32 rand = urand(0,10000);

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_DPS_TESTING)
        rand = 0;

    //if (pVictim->HasAura(31224) && SpellMgr::GetSpellSchoolMask(spell) & SPELL_SCHOOL_MASK_MAGIC)
    //    sLog.outLog(LOG_SPECIAL, "CloakOfShadows: %s casted spell %u against %s with Roll %.2f HitChance %.2f - %s", GetName(), spell->Id, pVictim->GetName(), float(rand)/100, float(HitChance)/100, HitChance > rand ? "HIT!" : "miss");

    if (rand > HitChance)
    {
        SendCombatStats(1<<COMBAT_STATS_MAGIC_RESULT, "MagicSpellHitResult (id=%d): hit chance = %d - missed!", pVictim, spell->Id, HitChance);
        return SPELL_MISS_RESIST;
    }

    // binary resist spells
    if (SpellMgr::IsBinaryResistable(spell) && CalcBinaryResist(pVictim, schoolMask))
    {
        SendCombatStats(1<<COMBAT_STATS_MAGIC_RESULT, "MagicSpellHitResult (id=%d): hit chance = %d - resisted!", pVictim, spell->Id, HitChance);
        return SPELL_MISS_RESIST;
    }
    SendCombatStats(1<<COMBAT_STATS_MAGIC_RESULT, "MagicSpellHitResult (id=%d): hit chance = %d - hit!", pVictim, spell->Id, HitChance);
    return SPELL_MISS_NONE;
}

// Calculate spell hit result can be:
// Every spell can: Evade/Immune/Reflect/Sucesful hit
// For melee based spells:
//   Miss
//   Dodge
//   Parry
// For spells
//   Resist
SpellMissInfo Unit::SpellHitResult(Unit *pVictim, SpellEntry const *spell, bool CanReflect, bool canMiss)
{
    if (ToCreature() && ToCreature()->isTotem())
        if (Unit *owner = GetOwner())
            return owner->SpellHitResult(pVictim, spell, CanReflect, canMiss);

    // Return evade for units in evade mode
    if (pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode() && this != pVictim)
    {
        if (sWorld.IsShutdowning() && sWorld.GetShutdownTimer() < BOSS_AUTOEVADE_RESTART_TIME && ((Creature*)pVictim)->isWorldBoss())
        {
            if (Player* plr = ToPlayer())
                ChatHandler(plr).SendSysMessage(16662);
        }

        return SPELL_MISS_EVADE;
    }

    // Check for immune (Not using charges -> 
    // -> will be checked WITH charges after resist is being decided. If no resist -> look for immune again (IsImmunedToSpellMechanic))
    // It is supposed that charged spells are only positive (unlike cyclone, which would make you immune to positive spells too)
    if (pVictim->IsImmunedToSpell(spell, false))
        return SPELL_MISS_IMMUNE;

    switch (spell->Id)
    {
        case 12733: // Fearless USED BY OVER 60 Level
        case 23097: // Fire Reflector USED BY OVER 60 Level
        case 23131: // Ice Reflector USED BY OVER 60 Level
        case 23132: // Shadow Reflector USED BY OVER 60 Level
        {
            if (pVictim->GetLevel() > 60 && urand(0,1))
                return SPELL_MISS_RESIST;
            break;
        }
        default:
            break;
    }

    // All positive spells + dispels on friendly target can`t miss

    // TODO: client not show miss log for this spells - so need find info for this in dbc and use it!
    if ((SpellMgr::IsPositiveSpell(spell->Id) || SpellMgr::IsDispel(spell))
        &&(IsFriendlyTo(pVictim)))  //prevent from affecting enemy by "positive" spell
        return SPELL_MISS_NONE;

    // Spells of Vengeful Spirit (Teron fight) can't miss
    if (spell->Id == 40157 ||
       spell->Id == 40175 ||
       spell->Id == 40314 ||
       spell->Id == 40325 ||
       spell->Id == 43621 ||
       spell->Id == 43648)
       return SPELL_MISS_NONE;

    // Check for immune (use charges)
    // Check if Spell cannot be immuned
    if (!(spell->Attributes & SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY))
        if (pVictim->IsImmunedToDamage(SpellMgr::GetSpellSchoolMask(spell)))
            return SPELL_MISS_IMMUNE;

    // fix HandlePreventFleeing
    if (spell->Mechanic == MECHANIC_FEAR && pVictim->HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return SPELL_MISS_IMMUNE;

    if (this == pVictim)
        return SPELL_MISS_NONE;

    // Try victim reflect spell
    if (CanReflect)
    {
        int32 reflectchance = pVictim->GetTotalAuraModifier(SPELL_AURA_REFLECT_SPELLS);
        Unit::AuraList const& mReflectSpellsSchool = pVictim->GetAurasByType(SPELL_AURA_REFLECT_SPELLS_SCHOOL);
        for (Unit::AuraList::const_iterator i = mReflectSpellsSchool.begin(); i != mReflectSpellsSchool.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & SpellMgr::GetSpellSchoolMask(spell))
                reflectchance += (*i)->GetModifierValue();

        if (reflectchance > 0 && roll_chance_i(reflectchance))
        {
            // Start triggers for remove charges if need (trigger only for victim, and mark as active spell)
            //ProcDamageAndSpell(pVictim, PROC_FLAG_NONE, PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT, PROC_EX_REFLECT, 1, BASE_ATTACK, spell);
            return SPELL_MISS_REFLECT;
        }
    }

    SpellMissInfo result = SPELL_MISS_NONE; 
    switch (spell->DmgClass) // all those inside can return spell_miss_none
    {
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
            result = MeleeSpellHitResult(pVictim, spell, canMiss);
            break;
        case SPELL_DAMAGE_CLASS_NONE:
        {
            if (spell->Id == 13327 && pVictim->GetLevel() > 60 && urand(0,1)) // Reckless Charge AGAINST OVER 60 Level
                result = SPELL_MISS_RESIST;
            else
                result = SPELL_MISS_NONE;
            break;
        }
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            if (spell->Attributes & SPELL_ATTR_ABILITY && spell->SchoolMask == SPELL_SCHOOL_MASK_NORMAL)
                result = MeleeSpellHitResult(pVictim, spell, canMiss); // depend on hit rating, not spell hit rating, and also cannot be dodged/parried/blocked
            else
                result = MagicSpellHitResult(pVictim, spell);
            break;
        }
    }
    // result is one of those SPELL_MISS_NONE, SPELL_MISS_MISS, SPELL_MISS_RESIST, SPELL_MISS_DODGE, SPELL_MISS_PARRY, SPELL_MISS_BLOCK
    if (result != SPELL_MISS_NONE && result != SPELL_MISS_BLOCK) // everything but BLOCK or NO_MISS is better than charged
        return result;

    if (pVictim->IsImmunedToSpellMechanic(spell->Mechanic, true)) // Charged immunity is better than block
        return SPELL_MISS_IMMUNE;

    return result; // SPELL_MISS_NONE or SPELL_MISS_BLOCK is here
}

uint32 Unit::GetDefenseSkillValue(Unit const* target) const
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // in PvP use full skill instead current skill value
        uint32 value = (target && (target->isCharmedOwnedByPlayerOrPlayer()))
            ? ((Player*)this)->GetMaxSkillValue(SKILL_DEFENSE)
            : ((Player*)this)->GetSkillValue(SKILL_DEFENSE);
        value += uint32(((Player*)this)->GetRatingBonusValue(CR_DEFENSE_SKILL));
        return value;
    }
    else
        return GetUnitMeleeSkill(target);
}

float Unit::GetUnitDodgeChance() const
{
    if (HasUnitState(UNIT_STAT_LOST_CONTROL | UNIT_STAT_CASTING))
        return 0.0f;

    if (GetTypeId() == TYPEID_PLAYER)
        return GetFloatValue(PLAYER_DODGE_PERCENTAGE);
    else
    {
        if (((Creature const*)this)->isTotem())
            return 0.0f;
        else
        {
            float dodge = 5.0f;
            dodge += GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);
            return dodge > 0.0f ? dodge : 0.0f;
        }
    }
}

float Unit::GetUnitParryChance() const
{
    if (IsNonMeleeSpellCast(false) || HasUnitState(UNIT_STAT_LOST_CONTROL))
        return 0.0f;

    float chance = 0.0f;

    if (GetTypeId() == TYPEID_PLAYER)
    {
        Player const* player = (Player const*)this;
        if (player->CanParry())
        {
            Item *tmpitem = player->GetWeaponForAttack(BASE_ATTACK,true);
            if (!tmpitem)
                tmpitem = player->GetWeaponForAttack(OFF_ATTACK,true);

            if (tmpitem)
                chance = GetFloatValue(PLAYER_PARRY_PERCENTAGE);
        }
    }
    else if (GetTypeId() == TYPEID_UNIT)
    {
        Creature *c = (Creature*)this;
        if (c->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY)
            return 0.0f;

        if(c->GetCreatureType() == CREATURE_TYPE_HUMANOID || c->GetCreatureInfo()->equipmentId || c->isWorldBoss())
        {
            chance = 5.0f;
            chance += GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);
        }
    }

    return chance > 0.0f ? chance : 0.0f;
}

float Unit::GetUnitBlockChance() const
{
    if (IsNonMeleeSpellCast(false) || HasUnitState(UNIT_STAT_LOST_CONTROL))
        return 0.0f;

    if (GetTypeId() == TYPEID_PLAYER)
    {
        Player const* player = (Player const*)this;
        if (player->CanBlock())
            return GetFloatValue(PLAYER_BLOCK_PERCENTAGE);
        // is player but has no block ability or no not broken shield equipped
        return 0.0f;
    }
    else
    {
        if (((Creature const*)this)->isTotem() || ((Creature*)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_BLOCK)
            return 0.0f;
        else
        {
            float block = 5.0f;
            block += GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);
            return block > 0.0f ? block : 0.0f;
        }
    }
}

float Unit::GetUnitCriticalChance(WeaponAttackType attackType, const Unit *pVictim) const
{
    float crit;

    if (GetTypeId() == TYPEID_PLAYER)
    {
        switch (attackType)
        {
            case BASE_ATTACK:
                crit = GetFloatValue(PLAYER_CRIT_PERCENTAGE);
                break;
            case OFF_ATTACK:
                crit = GetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE);
                break;
            case RANGED_ATTACK:
                crit = GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE);
                break;
                // Just for good manner
            default:
                crit = 0.0f;
                break;
        }
    }
    else
    {
        // unit case
        if (((Creature*)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_CRIT)
            return 0.0f;

        crit = 5.0f;
        crit += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PERCENT);
    }

    // flat aura mods
    if (attackType == RANGED_ATTACK)
        crit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE);
    else
        crit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE);

    crit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE);

    // reduce crit chance from Rating for players
    if (pVictim->GetTypeId()==TYPEID_PLAYER)
    {
        if (attackType==RANGED_ATTACK)
            crit -= ((Player*)pVictim)->GetRatingBonusValue(CR_CRIT_TAKEN_RANGED);
        else
            crit -= ((Player*)pVictim)->GetRatingBonusValue(CR_CRIT_TAKEN_MELEE);
    }

    if (crit < 0.0f)
        crit = 0.0f;
    return crit;
}

uint32 Unit::GetWeaponSkillValue (WeaponAttackType attType, Unit const* target) const
{
    uint32 value = 0;
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (target && target->isCharmedOwnedByPlayerOrPlayer())
            return GetMaxSkillValueForLevel();

        Item* item = ((Player*)this)->GetWeaponForAttack(attType,true);

        // feral or unarmed skill only for base attack
        if (attType != BASE_ATTACK && !item)
        {
            if (attType == RANGED_ATTACK && GetClass() == CLASS_PALADIN) //hammer
                return GetMaxSkillValueForLevel();
            return 0;
        }

        if (IsInFeralForm(true))
            return GetMaxSkillValueForLevel();              // always maximized SKILL_FERAL_COMBAT in fact

        // weapon skill or (unarmed for base attack)
        uint32  skill = item && item->GetSkill() != SKILL_FIST_WEAPONS ? item->GetSkill() : SKILL_UNARMED;

        // in PvP use full skill instead current skill value
        value = (target && target->isCharmedOwnedByPlayerOrPlayer())
            ? ((Player*)this)->GetMaxSkillValue(skill)
            : ((Player*)this)->GetSkillValue(skill);
        // Modify value from ratings
        value += uint32(((Player*)this)->GetRatingBonusValue(CR_WEAPON_SKILL));
        switch (attType)
        {
            case BASE_ATTACK:   value+=uint32(((Player*)this)->GetRatingBonusValue(CR_WEAPON_SKILL_MAINHAND));break;
            case OFF_ATTACK:    value+=uint32(((Player*)this)->GetRatingBonusValue(CR_WEAPON_SKILL_OFFHAND));break;
            case RANGED_ATTACK: value+=uint32(((Player*)this)->GetRatingBonusValue(CR_WEAPON_SKILL_RANGED));break;
        }
    }
    else
        value = GetUnitMeleeSkill(target);
   return value;
}

void Unit::_DeleteAuras()
{
    while (!m_removedAuras.empty())
    {
        delete m_removedAuras.front();
        m_removedAuras.pop_front();
    }
}

void Unit::_UpdateSpells(uint32 time)
{
    if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
        _UpdateAutoRepeatSpell();

    // remove finished spells from current pointers
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
    {
        if (m_currentSpells[i] && m_currentSpells[i]->getState() == SPELL_STATE_FINISHED)
        {
            m_currentSpells[i]->SetReferencedFromCurrent(false);
            m_currentSpells[i] = NULL;                      // remove pointer
        }
    }

    // m_AurasUpdateIterator can be updated in inderect called code at aura remove to skip next planned to update but removed auras
    AuraMap::iterator eraseIter;
    for (m_AurasUpdateIterator = m_Auras.begin(); m_AurasUpdateIterator != m_Auras.end();)
    {
        Aura* i_aura = m_AurasUpdateIterator->second;
        eraseIter = m_AurasUpdateIterator;
        ++m_AurasUpdateIterator;                            // need shift to next for allow update if need into aura update

        if (i_aura)
            i_aura->Update(time);
        else
            m_Auras.erase(eraseIter);
    }

    // remove expired auras
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end();)
    {
        if (i->second->IsExpired())
            RemoveAura(i, AURA_REMOVE_BY_EXPIRE);
        else
             ++i;
    }

    _DeleteAuras();

    if (!m_gameObj.empty())
    {
        std::list<GameObject*>::iterator itr;
        for (itr = m_gameObj.begin(); itr != m_gameObj.end();)
        {
            if (!(*itr)->isSpawned())
            {
                (*itr)->SetOwnerGUID(0);
                (*itr)->SetRespawnTime(0);
                (*itr)->Delete();
                m_gameObj.erase(itr++);
            }
            else
                ++itr;
        }
    }
}

void Unit::_UpdateAutoRepeatSpell()
{
    //check "realtime" interrupts
    bool isMoving = (GetTypeId() == TYPEID_PLAYER && ((Player*)this)->isMoving());
    if (isMoving || IsNonMeleeSpellCast(false, false, true))
    {
        // cancel wand shoot
        if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->GetSpellEntry()->Category == 351)
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);

        m_AutoRepeatFirstCast = true;

        // prioritize auto shot only if interruptor is spell, if interrupted by moving -> auto shot is not priority
        m_AutoRepeatPrioritized = !isMoving;
        
        return;
    }

    //apply delay
    if (m_AutoRepeatFirstCast && getAttackTimer(RANGED_ATTACK) < 500)
    {
        setAttackTimer(RANGED_ATTACK, 500);
    }

    m_AutoRepeatFirstCast = false;

    if (isAttackReady(RANGED_ATTACK))
        TriggerAutocastSpell();
}

void Unit::TriggerAutocastSpell()
{
    // Check if able to cast
    SpellCastResult result = m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->CheckCast(true);
    if (result != SPELL_CAST_OK && result != SPELL_FAILED_NOT_READY) // ignore normal cooldowns, we handle timers in autocast mechanic
    {
        InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
        return;
    }

    Spell* spell = new Spell(this, m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->GetSpellEntry(), true, 0);
    spell->prepare(&(m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_targets));

    if (!IsStandState())
        SetStandState(UNIT_STAND_STATE_STAND);

    resetAttackTimer(RANGED_ATTACK);
}

void Unit::SetCurrentCastSpell(Spell* spell)
{
    ASSERT(spell);                                         // NULL may be never passed here, use InterruptSpell or InterruptNonMeleeSpells

    CurrentSpellTypes SpellType = spell->GetCurrentContainer();

    if (spell == GetCurrentSpell(SpellType))
        return;

    // break same type spell if it is not delayed
    InterruptSpell(SpellType, false);

    // special breakage effects:
    switch (SpellType)
    {
        case CURRENT_GENERIC_SPELL:
        {
            // generic spells always break channeled not delayed spells
            InterruptSpell(CURRENT_CHANNELED_SPELL,false);

            // autorepeat breaking
            if (Spell* current = GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
            {
                // break autorepeat if not Auto Shot
                if (current->GetSpellEntry()->Category == 351)
                    InterruptSpell(CURRENT_AUTOREPEAT_SPELL);

                m_AutoRepeatFirstCast = true;
                m_AutoRepeatPrioritized = true; // do prioritize AutoShot, if we're moving -> it will unprioritize it in an update
            }

            if (spell->GetCastTime())
                addUnitState(UNIT_STAT_CASTING);

            break;
        }
        case CURRENT_CHANNELED_SPELL:
        {
            // channel spells always break generic non-delayed and any channeled spells
            InterruptSpell(CURRENT_GENERIC_SPELL, false);
            InterruptSpell(CURRENT_CHANNELED_SPELL);

            // it also does break autorepeat if not Auto Shot
            if (Spell* current = GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
            {
                // break autorepeat if not Auto Shot
                if (current->GetSpellEntry()->Category == 351)
                    InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            }

            if (spell->GetCastTime() || SpellMgr::GetSpellDuration(spell->GetSpellEntry()))
                addUnitState(UNIT_STAT_CASTING);

            break;
        }
        case CURRENT_AUTOREPEAT_SPELL:
        {
            // only Auto Shoot does not break anything
            if (spell->GetSpellEntry()->Category == 351)
            {
                // generic autorepeats break generic non-delayed and channeled non-delayed spells
                InterruptSpell(CURRENT_GENERIC_SPELL,false);
                InterruptSpell(CURRENT_CHANNELED_SPELL,false);
            }

            // special action: set first cast flag
            m_AutoRepeatFirstCast = true;
            m_AutoRepeatPrioritized = true;
            break;
        }

        default:
            break;
    }

    // current spell (if it is still here) may be safely deleted now
    if (Spell* current = GetCurrentSpell(SpellType))
        current->SetReferencedFromCurrent(false);

    // set new current spell
    m_currentSpells[SpellType] = spell;
    spell->SetReferencedFromCurrent(true);
}

void Unit::InterruptSpell(uint32 spellType, bool withDelayed, bool withInstant)
{
    ASSERT(spellType < CURRENT_MAX_SPELL);

    Spell* spell = GetCurrentSpell(CurrentSpellTypes(spellType));

    if (spell
        && (withDelayed || spell->getState() != SPELL_STATE_DELAYED)
        && (withInstant || spell->GetCastTime() > 0))
    {
        // for example, do not let self-stun aura interrupt itself
        if (!spell->IsInterruptable())
            return;

        // send autorepeat cancel message for autorepeat spells so the client will retry casting it after target comes back into LoS
        if (spellType == CURRENT_AUTOREPEAT_SPELL && GetTypeId() == TYPEID_PLAYER)
            ToPlayer()->SendAutoRepeatCancel();

        if (spell->getState() != SPELL_STATE_FINISHED)
        {
            SpellEntry const* spellInfo = m_currentSpells[spellType]->GetSpellEntry();
            spell->cancel(SPELL_FAILED_INT_TRUE_INTERRUPT);
            if(GetTypeId() == TYPEID_UNIT && ToCreature()->IsAIEnabled)
                ToCreature()->AI()->OnSpellInterrupt(spellInfo);
        }

        m_currentSpells[spellType] = NULL;
        spell->SetReferencedFromCurrent(false);
    }
}

void Unit::FinishSpell(CurrentSpellTypes spellType, bool ok /*= true*/)
{
    Spell* spell = GetCurrentSpell(spellType);
    if (!spell)
        return;

    if (spellType == CURRENT_CHANNELED_SPELL)
        spell->SendChannelUpdate(0);

    spell->finish(ok);
}

bool Unit::IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled, bool skipAutorepeat) const
{
    // We don't do loop here to explicitly show that melee spell is excluded.
    // Maybe later some special spells will be excluded too.

    // generic spells are cast when they are not finished and not delayed
    if (Spell* current = GetCurrentSpell(CURRENT_GENERIC_SPELL))
    {
        if (current->getState() != SPELL_STATE_FINISHED && (withDelayed || current->getState() != SPELL_STATE_DELAYED))
            return true;
    }
    // channeled spells may be delayed, but they are still considered cast
    else if (!skipChanneled)
    {
        if (Spell* current = GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        {
            if (current->getState() != SPELL_STATE_FINISHED)
                return true;
        }
    }

    // autorepeat spells may be finished or delayed, but they are still considered cast
    else if (!skipAutorepeat && GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
        return true;

    return false;
}

void Unit::InterruptNonMeleeSpells(bool withDelayed, uint32 spell_id, bool withInstant)
{
    // generic spells are interrupted if they are not finished or delayed
    if (m_currentSpells[CURRENT_GENERIC_SPELL] && (!spell_id || m_currentSpells[CURRENT_GENERIC_SPELL]->GetSpellEntry()->Id==spell_id))
        InterruptSpell(CURRENT_GENERIC_SPELL,withDelayed,withInstant);

    // autorepeat spells are interrupted if they are not finished or delayed
    if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL] && (!spell_id || m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->GetSpellEntry()->Id==spell_id))
        InterruptSpell(CURRENT_AUTOREPEAT_SPELL,withDelayed,withInstant);

    // channeled spells are interrupted if they are not finished, even if they are delayed
    if (m_currentSpells[CURRENT_CHANNELED_SPELL] && (!spell_id || m_currentSpells[CURRENT_CHANNELED_SPELL]->GetSpellEntry()->Id==spell_id))
        InterruptSpell(CURRENT_CHANNELED_SPELL,true,true);
}

Spell* Unit::FindCurrentSpellBySpellId(uint32 spell_id) const
{
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
        if (m_currentSpells[i] && m_currentSpells[i]->GetSpellEntry()->Id==spell_id)
            return m_currentSpells[i];
    return NULL;
}

bool Unit::isInFront(Unit const* target, float distance,  float arc) const
{
    return IsWithinDistInMap(target, distance) && HasInArc(arc, target);
}

bool Unit::isInFront(GameObject const* target, float distance,  float arc) const
{
    return IsWithinDistInMap(target, distance) && HasInArc(arc, target);
}

bool Unit::isInBack(Unit const* target, float distance, float arc) const
{
    return IsWithinDistInMap(target, distance) && !HasInArc(2 * M_PI - arc, target);
}

bool Unit::isInBack(GameObject const* target, float distance, float arc) const
{
    return IsWithinDistInMap(target, distance) && !HasInArc(2 * M_PI - arc, target);
}

bool Unit::isInLine(Unit const* target, float distance) const
{
    if (!HasInArc(M_PI, target) || !IsWithinDistInMap(target, distance))
        return false;

    float width = GetObjectSize() + target->GetObjectSize() * 0.5f;
    float angle = GetAngleTo(target);
    return fabs(sin(angle)) * GetExactDist2d(target->GetPositionX(), target->GetPositionY()) < width;
}

bool Unit::isInLine(GameObject const* target, float distance) const
{
    if (!HasInArc(M_PI, target) || !IsWithinDistInMap(target, distance))
        return false;

    float width = GetObjectSize() + target->GetObjectSize() * 0.5f;
    float angle = GetAngleTo(target);
    return fabs(sin(angle)) * GetExactDist2d(target->GetPositionX(), target->GetPositionY()) < width;
}

bool Unit::isBetween(WorldObject *s, WorldObject *e, float offset) const
{
    float xn, yn, xp, yp, xh, yh;

    xn = s->GetPositionX();
    yn = s->GetPositionY();

    xp = e->GetPositionX();
    yp = e->GetPositionY();

    xh = GetPositionX();
    yh = GetPositionY();

    // check if target is between
    if (s->GetExactDist2d(xh,yh) >= s->GetExactDist2d(xp,yp) || e->GetExactDist2d(xh,yh) >= s->GetExactDist2d(xp,yp))
        return false;

    // check distance from the line
    return (fabs((xn-xp)*yh + (yp-yn)*xh - xn*yp + xp*yn) / s->GetExactDist2d(xp,yp) < offset);
}

bool Unit::isInAccessiblePlacefor (Creature const* c) const
{
    // IsSwimming() for players is a VERY EASY method, however for units it is a CPU-HUNGRY method
    // Most of NPC targets are actually players (pets are still units!)
    // Most creatures can indeed swim, but some cannot
    if (IsSwimming())
        return c->CanSwim();
    else
        return c->CanWalk() || c->CanFly();
}

bool Unit::IsSwimming() const
{
    if (!Hellground::IsValidMapCoord(GetPositionX(), GetPositionY(), GetPositionZ()))
        return false;

    return GetTerrain()->IsInWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

void Unit::DeMorph()
{
    SetDisplayId(GetNativeDisplayId());
}

int32 Unit::GetTotalAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    bool outdoors = true;
    if (sWorld.getConfig(CONFIG_VMAP_INDOOR_CHECK))
        outdoors = GetTerrain()->IsOutdoors(GetPositionX(),GetPositionY(),GetPositionZ());

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
        if(outdoors || !((*i)->GetSpellProto()->Attributes & SPELL_ATTR_OUTDOORS_ONLY))
            modifier += (*i)->GetModifierValue();

    return modifier;
}

float Unit::GetTotalAuraMultiplier(AuraType auratype) const
{
    float multiplier = 1.0f;

    bool outdoors = true;
    if (sWorld.getConfig(CONFIG_VMAP_INDOOR_CHECK))
        outdoors = GetTerrain()->IsOutdoors(GetPositionX(),GetPositionY(),GetPositionZ());

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
        if(outdoors || !((*i)->GetSpellProto()->Attributes & SPELL_ATTR_OUTDOORS_ONLY))
            multiplier *= (100.0f + (*i)->GetModifierValue())/100.0f;

    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    bool outdoors = true;
    if (sWorld.getConfig(CONFIG_VMAP_INDOOR_CHECK))
        outdoors = GetTerrain()->IsOutdoors(GetPositionX(),GetPositionY(),GetPositionZ());

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if(outdoors || !((*i)->GetSpellProto()->Attributes & SPELL_ATTR_OUTDOORS_ONLY))
        {
            int32 amount = (*i)->GetModifierValue();
            if (amount > modifier)
                modifier = amount;
        }
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        int32 amount = (*i)->GetModifierValue();
        if (amount < modifier)
            modifier = amount;
    }

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue & misc_mask)
            modifier += (*i)->GetModifierValue();
    }
    return modifier;
}

float Unit::GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue & misc_mask)
            multiplier *= (100.0f + (*i)->GetModifierValue())/100.0f;
    }
    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        int32 amount = (*i)->GetModifierValue();
        if (mod->m_miscvalue & misc_mask && amount > modifier)
            modifier = amount;
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        int32 amount = (*i)->GetModifierValue();
        if (mod->m_miscvalue & misc_mask && amount < modifier)
            modifier = amount;
    }

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue == misc_value)
            modifier += (*i)->GetModifierValue();
    }
    return modifier;
}

float Unit::GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue == misc_value)
            multiplier *= (100.0f + (*i)->GetModifierValue())/100.0f;
    }
    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        int32 amount = (*i)->GetModifierValue();
        if (mod->m_miscvalue == misc_value && amount > modifier)
            modifier = amount;
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        int32 amount = (*i)->GetModifierValue();
        if (mod->m_miscvalue == misc_value && amount < modifier)
            modifier = amount;
    }

    return modifier;
}

void Unit::AddAuraCreate(uint32 spellId, Unit* target)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (spellInfo)
    {
        for (uint32 i = 0; i < 3; i++)
        {
            uint8 eff = spellInfo->Effect[i];
            if (eff >= TOTAL_SPELL_EFFECTS)
                continue;
            if (SpellMgr::IsAreaAuraEffect(eff) ||
                eff == SPELL_EFFECT_APPLY_AURA ||
                eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
            {
                Aura* Aur = CreateAura(spellInfo, i, NULL, target);
                this->AddAura(Aur);
            }
        }
    }
}

bool Unit::AddAura(Aura *Aur)
{
    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if ((!isAlive() || (GetTypeId()==TYPEID_PLAYER && (((Player*)this)->IsSpectator()))) && !(Aur->GetSpellProto()->Attributes & (SPELL_ATTR_PASSIVE | SPELL_ATTR_CASTABLE_WHILE_DEAD)) && !SpellMgr::IsDeathPersistentSpell(Aur->GetSpellProto()) &&
        (GetTypeId()!=TYPEID_PLAYER || !((Player*)this)->GetSession()->PlayerLoading()))
    {
        delete Aur;
        return false;
    }

    if (Aur->GetTarget() != this)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Aura (spell %u eff %u) add to aura list of %s (lowguid: %u) but Aura target is %s (lowguid: %u)",
            Aur->GetId(),Aur->GetEffIndex(),(GetTypeId()==TYPEID_PLAYER?"player":"creature"),GetGUIDLow(),
            (Aur->GetTarget()->GetTypeId()==TYPEID_PLAYER?"player":"creature"),Aur->GetTarget()->GetGUIDLow());
        delete Aur;
        return false;
    }

    SpellEntry const* aurSpellEntry = Aur->GetSpellProto();

    spellEffectPair spair = spellEffectPair(Aur->GetId(), Aur->GetEffIndex());

    bool stackModified = false;
    // passive and persistent auras can stack with themselves any number of times (with NPCs windfury exception)
    if (Aur->GetId() == 32912 || (!Aur->IsPassive() && !Aur->IsPersistent()))
    {
        bool isDotOrHot = false;
        for (uint8 i = 0; i < 3; i++)
        {
            switch (aurSpellEntry->EffectApplyAuraName[i])
            {
                // DOT or HOT from different casters will stack
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_POWER_BURN_MANA:
                case SPELL_AURA_OBS_MOD_MANA:
                case SPELL_AURA_OBS_MOD_HEALTH:
                    isDotOrHot = true;
                    break;
            }
        }

        for (AuraMap::iterator i2 = m_Auras.lower_bound(spair); i2 != m_Auras.upper_bound(spair);)
        {
            if (Aur->DiffPerCaster() && Aur->GetCasterGUID() != i2->second->GetCasterGUID())
            {
                i2++;
                continue;
            }

            if (i2->second->GetId() == 31944 || i2->second->GetId() == 32911)    //HACK check for Doomfire DoT stacking and NPCs windfury
            {
                RemoveAura(i2, AURA_REMOVE_BY_STACK);
                i2 = m_Auras.lower_bound(spair);
                continue;
            }

            if (aurSpellEntry->Id == 28093 || aurSpellEntry->Id == 20007)       // Allow mongoose procs from different weapons stack
            {
                if (Aur->GetCastItemGUID() != i2->second->GetCastItemGUID())
                {
                    i2++;
                    continue;
                }
            }
            
            if (aurSpellEntry->Id == 37408)
            {
                if (Aur->GetCasterGUID() != i2->second->GetCasterGUID())
                {
                    i2++;
                    continue;
                }
            }

            if (i2->second->GetCasterGUID() == Aur->GetCasterGUID() || Aur->StackNotByCaster() || (Aur->GetCaster() && Aur->GetCaster()->GetTypeId() != TYPEID_PLAYER))    // always stack auras from different creatures
            {
                if (!stackModified)
                {
                    // replace aura if next will > spell StackAmount
                    if (aurSpellEntry->StackAmount)
                    {
                        // prevent adding stack more than once
                        stackModified = true;
                        Aur->SetStackAmount(i2->second->GetStackAmount());
                        Aur->SetPeriodicTimer(i2->second->GetPeriodicTimer());
                        if (Aur->GetStackAmount() < aurSpellEntry->StackAmount)
                            Aur->SetStackAmount(Aur->GetStackAmount()+1);
                    }
                    // this will allow to stack dots and hots cast by different creatures
                    if(i2->second->GetCasterGUID() == Aur->GetCasterGUID() || Aur->StackNotByCaster() || aurSpellEntry->StackAmount)
                    {
                        RemoveAura(i2, AURA_REMOVE_BY_STACK);
                        i2 = m_Auras.lower_bound(spair);
                        continue;
                    }
                }
            }
            if (isDotOrHot)
            {
                ++i2;
                continue;
            }
            RemoveAura(i2,AURA_REMOVE_BY_STACK);
            i2=m_Auras.lower_bound(spair);
            continue;
        }
    }

    // passive auras stack with all (except passive spell proc auras)
    if ((!Aur->IsPassive() || !IsPassiveStackableSpell(Aur->GetId())) &&
        !(Aur->GetId() == 20584 || Aur->GetId() == 8326))
    {
        if (!RemoveNoStackAurasDueToAura(Aur))
        {
            delete Aur;
            return false;                                   // couldn't remove conflicting aura with higher rank
        }
    }

    // update single target auras list (before aura add to aura list, to prevent unexpected remove recently added aura)
    if (Aur->IsSingleTarget() && Aur->GetTarget())
    {
        // caster pointer can be deleted in time aura remove, find it by guid at each iteration
        for (;;)
        {
            Unit* caster = Aur->GetCaster();
            if (!caster)                                     // caster deleted and not required adding scAura
                break;

            bool restart = false;
            AuraList& scAuras = caster->GetSingleCastAuras();
            for (AuraList::iterator itr = scAuras.begin(); itr != scAuras.end(); ++itr)
            {
                if ((*itr)->GetTarget() != Aur->GetTarget() &&
                    SpellMgr::IsSingleTargetSpells((*itr)->GetSpellProto(),aurSpellEntry))
                {
                    if ((*itr)->IsInUse())
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Aura (Spell %u Effect %u) is in process but attempt removed at aura (Spell %u Effect %u) adding, need add stack rule for IsSingleTargetSpell", (*itr)->GetId(), (*itr)->GetEffIndex(),Aur->GetId(), Aur->GetEffIndex());
                        continue;
                    }
                    (*itr)->GetTarget()->RemoveAura((*itr)->GetId(), (*itr)->GetEffIndex());
                    restart = true;
                    break;
                }
            }

            if (!restart)
            {
                // done
                scAuras.push_back(Aur);
                break;
            }
        }
    }

    // add aura, register in lists and arrays
    Aur->_AddAura();
    m_Auras.insert(AuraMap::value_type(spellEffectPair(Aur->GetId(), Aur->GetEffIndex()), Aur)); // NO GETSPELLANALOG - CAUSE IT'S REAL SERVER-SIDE AURA
    if (Aur->GetModifier()->m_auraname < TOTAL_AURAS)
    {
        m_modAuras[Aur->GetModifier()->m_auraname].push_back(Aur);
        if (Aur->GetSpellProto()->AuraInterruptFlags)
        {
            m_interruptableAuras.push_back(Aur);
            AddInterruptMask(Aur->GetSpellProto()->AuraInterruptFlags);
        }
        if ((Aur->GetSpellProto()->Attributes & SPELL_ATTR_BREAKABLE_BY_DAMAGE)
            && (Aur->GetModifier()->m_auraname != SPELL_AURA_MOD_POSSESS)) //only dummy aura is breakable
        {
            /*Examples:
            Old:
            lvl 70 - 2000 damage mean
            lvl 60 - 1700 damage mean
            lvl 50 - 1400 damage mean
            lvl 40 - 1100 damage mean
            lvl 20 - 500 damage mean
            lvl 10 - 200 damage mean
            lvl 9 - 170 damage mean
            lvl 8 - 50 damage mean
            lvl 1 - 50 damage mean
            
            New (if basis is 2000 damage on lvl 70):
            lvl 70 - 2000 damage mean
            lvl 60 - 1532 damage mean
            lvl 50 - 1174 damage mean
            lvl 40 - 864 damage mean
            lvl 20 - 367 damage mean
            lvl 10 - 202 damage mean
            lvl 9 - 189 damage mean
            lvl 8 - 174 damage mean
            lvl 1 - 70 damage mean

            New (if basis is 1300 damage on lvl 70):
            lvl 70 - 1300 damage mean
            lvl 60 - 995 damage mean
            lvl 50 - 763 damage mean
            lvl 40 - 561 damage mean
            lvl 20 - 238 damage mean
            lvl 10 - 131 damage mean
            lvl 9 - 122 damage mean
            lvl 8 - 113 damage mean
            lvl 1 - 45 damage mean

            level 70 has multiplier of 5.4369, so basis for level 20 is 239 (1300 / 5.43) (multiplier is 1.0 at level 20, so this is the basis)
            */
            uint32 max_dmg = 0;
            GtNPCManaCostScalerEntry const* casterScaler = sGtNPCManaCostScalerStore.LookupEntry(GetLevel() - 1);
            if (casterScaler)
                max_dmg = casterScaler->ratio * 239.0f;

            max_dmg = urand(max_dmg/2, max_dmg+(max_dmg/2)); // for level 70 -> from 650 to 1950

            m_ccAuras[Aur] =  max_dmg;
        }
    }

    Aur->ApplyModifier(true,true);

    uint32 id = Aur->GetId();
    SpellEntry const *spellInfo = Aur->GetSpellProto();
    if (spellInfo->AttributesCu & SPELL_ATTR_CU_LINK_AURA)
    {
        if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(id + SPELL_LINK_AURA))
            for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                if (*itr < 0)
                    ApplySpellImmune(id, IMMUNITY_ID, -(*itr), true);
                else if (Unit* caster = Aur->GetCaster())
                    caster->AddAura(*itr, this);
    }

    if (GetTypeId() == TYPEID_UNIT && IsAIEnabled)
        ((Creature*)this)->AI()->OnAuraApply(Aur, Aur->GetCaster(), stackModified);

    if (spellInfo->AttributesCu & SPELL_ATTR_CU_BLOCK_STEALTH)
        ModifyAuraState(AURA_STATE_FAERIE_FIRE, true);

    // message
	//if (sWorld.isEasyRealm())
	//{
	//	Player* p = (Player*)this;
	//	
	//	if (p->GetSession() && p->GetClass() == CLASS_PALADIN &&
	//		(aurSpellEntry->Id == 27168 || aurSpellEntry->Id == 20914 || aurSpellEntry->Id == 20913 || aurSpellEntry->Id == 20912 || aurSpellEntry->Id == 20911)
	//		&& Aur->GetModifier()->m_auraname == SPELL_AURA_PROC_TRIGGER_DAMAGE)
	//		ChatHandler(p).SendSysMessage(15640);
	//}

    sLog.outDebug("Aura %u now is in use", Aur->GetModifier()->m_auraname);
    return true;
}

void Unit::RemoveRankAurasDueToSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return;
    AuraMap::iterator i,next;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        ++next;
        uint32 i_spellId = (*i).second->GetId();
        if ((*i).second && i_spellId && i_spellId != spellId)
        {
            if (sSpellMgr.IsRankSpellDueToSpell(spellInfo,i_spellId))
            {
                RemoveAurasDueToSpell(i_spellId);

                if (m_Auras.empty())
                    break;
                else
                    next =  m_Auras.begin();
            }
        }
    }
}

bool Unit::RemoveNoStackAurasDueToAura(Aura *Aur)
{
    if (!Aur)
        return false;

    SpellEntry const* spellProto = Aur->GetSpellProto();
    if (!spellProto)
        return false;

    uint32 spellId = Aur->GetId();
    uint32 effIndex = Aur->GetEffIndex();

    SpellSpecific spellId_spec = SpellMgr::GetSpellSpecific(spellId);

    AuraMap::iterator i,next;

    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        ++next;
        if (!(*i).second) continue;

        SpellEntry const* i_spellProto = (*i).second->GetSpellProto();

        if (!i_spellProto)
            continue;

        uint32 i_spellId = i_spellProto->Id;

        if (spellId==i_spellId)
            continue;

        if (SpellMgr::IsPassiveSpell(i_spellId))
        {
            if (IsPassiveStackableSpell(i_spellId))
                continue;

            // passive non-stackable spells not stackable only with another rank of same spell
            if (!sSpellMgr.IsRankSpellDueToSpell(spellProto, i_spellId))
                continue;
        }

		if (SpellMgr::IsChanneledSpell(i_spellProto))
			continue; // do not remove channeled auras

        uint32 i_effIndex = (*i).second->GetEffIndex();

        bool is_triggered_by_spell = false;
        // prevent triggered aura of removing aura that triggered it
        for (int j = 0; j < 3; ++j)
            if (i_spellProto->EffectTriggerSpell[j] == spellProto->Id)
                is_triggered_by_spell = true;
        if (is_triggered_by_spell) continue;

        for (int j = 0; j < 3; ++j)
        {
            // prevent remove dummy triggered spells at next effect aura add
            switch (spellProto->Effect[j])                   // main spell auras added added after triggered spell
            {
                case SPELL_EFFECT_DUMMY:
                    switch (spellId)
                    {
                        case 5420: if (i_spellId==34123) is_triggered_by_spell = true; break;
                    }
                    break;
            }

            if (is_triggered_by_spell)
                break;

            // prevent remove form main spell by triggered passive spells
            switch (i_spellProto->EffectApplyAuraName[j])    // main aura added before triggered spell
            {
                case SPELL_AURA_MOD_SHAPESHIFT:
                    switch (i_spellId)
                    {
                        case 24858: if (spellId==24905)                  is_triggered_by_spell = true; break;
                        case 33891: if (spellId==5420 || spellId==34123) is_triggered_by_spell = true; break;
                        case 34551: if (spellId==22688)                  is_triggered_by_spell = true; break;
                    }
                    break;
            }
        }

        if (is_triggered_by_spell)
            continue;

        bool sameCaster = Aur->GetCasterGUID() == (*i).second->GetCasterGUID();

        // Seal of Command 0.75 sec delay
        if (spellId_spec == SPELL_SEAL && i_spellProto->SpellIconID == 561 && i_spellProto->SpellFamilyFlags & 0x0000000002000000)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (Aura* Aur = GetAura(i_spellId, i))
                {
                    Aur->SetAuraDuration(500);
                    Aur->UpdateAuraDuration();
                    Aur->special_flag = AURA_SPECIAL_SEALTWIST; // seal is delayed (special value)
                }
            }
            return true;
        }

        if (SpellMgr::IsNoStackSpellDueToSpell(spellId, i_spellId, sameCaster))
        {
            // Its a parent aura (create this aura in ApplyModifier)
            if ((*i).second->IsInUse())
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Aura (Spell %u Effect %u) is in process but attempt removed at aura (Spell %u Effect %u) adding, need add stack rule for Unit::RemoveNoStackAurasDueToAura", i->second->GetId(), i->second->GetEffIndex(),Aur->GetId(), Aur->GetEffIndex());
                continue;
            }

            uint64 caster = (*i).second->GetCasterGUID();
            // Remove all auras by aura caster
            for (uint8 a=0;a<3;++a)
            {
                spellEffectPair spair = spellEffectPair(i_spellId, a);
                for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
                {
                    if (iter->second->GetCasterGUID()==caster)
                    {
                        RemoveAura(iter, AURA_REMOVE_BY_STACK);
                        iter = m_Auras.lower_bound(spair);
                    }
                    else
                        ++iter;
                }
            }

            if (m_Auras.empty())
                break;
            else
                next =  m_Auras.begin();
        }
    }
    return true;
}

void Unit::RemoveAura(uint32 spellId, uint32 effindex, Aura* except)
{
    spellEffectPair spair = spellEffectPair(spellId, effindex);
    for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
    {
        volatile size_t count = m_Auras.count(spair);
        if (iter->second && iter->second!=except && !iter->second->IsInUse())
        {
            RemoveAura(iter);
            iter = m_Auras.lower_bound(spair);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasByCasterSpell(uint32 spellId, uint64 casterGUID)
{
    for (int k = 0; k < 3; ++k)
    {
        spellEffectPair spair = spellEffectPair(spellId, k);
        for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
        {
            if (iter->second->GetCasterGUID() == casterGUID)
            {
                RemoveAura(iter);
                iter = m_Auras.upper_bound(spair);          // overwrite by more appropriate
            }
            else
                ++iter;
        }
    }
}

void Unit::RemoveAurasWithFamilyFlagsAndTypeByCaster(uint32 familyName,  uint64 familyFlags, AuraType aurType, uint64 casterGUID)
{
    Unit::AuraList const& auras = GetAurasByType(aurType);
    for (Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end();)
    {
        if ((*itr)->GetCasterGUID() == casterGUID)
        {
            SpellEntry const* itr_spell = (*itr)->GetSpellProto();
            if (itr_spell && itr_spell->SpellFamilyName == familyName && (itr_spell->SpellFamilyFlags & familyFlags))
            {
                RemoveAurasDueToSpell(itr_spell->Id);
                itr = auras.begin();
                continue;
            }
        }
        ++itr;
    }
}

void Unit::SetAurasDurationByCasterSpell(uint32 spellId, uint64 casterGUID, int32 duration)
{
    for (uint8 i = 0; i < 3; ++i)
    {
        spellEffectPair spair = spellEffectPair(spellId, i);
        for (AuraMap::const_iterator itr = GetAuras().lower_bound(spair); itr != GetAuras().upper_bound(spair); ++itr)
        {
            if (itr->second->GetCasterGUID()==casterGUID)
            {
                itr->second->SetAuraDuration(duration);
                break;
            }
        }
    }
}

Aura* Unit::GetAuraByCasterSpell(uint32 spellId, uint64 casterGUID)
{
    // Returns first found aura from spell-use only in cases where effindex of spell doesn't matter!
    for (uint8 i = 0; i < 3; ++i)
    {
        spellEffectPair spair = spellEffectPair(spellId, i);
        for (AuraMap::const_iterator itr = GetAuras().lower_bound(spair); itr != GetAuras().upper_bound(spair); ++itr)
        {
            if (itr->second->GetCasterGUID()==casterGUID)
                return itr->second;
        }
    }
    return NULL;
}

void Unit::RemoveAurasDueToSpellByDispel(uint32 spellId, uint64 casterGUID, Unit *dispeler)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        Aura *aur = iter->second;
        if (aur->GetId() == spellId && aur->GetCasterGUID() == casterGUID)
        {
            // Custom dispel case
            // Unstable Affliction
            if (aur->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && (aur->GetSpellProto()->SpellFamilyFlags & 0x010000000000LL) && aur->GetSpellProto()->SpellIconID == 2039)
            {
                int32 damage = aur->GetModifier()->m_amount*9;
                uint64 caster_guid = aur->GetCasterGUID();

                // Remove aura
                RemoveAura(iter, AURA_REMOVE_BY_DISPEL);

                // backfire damage and silence
                dispeler->CastCustomSpell(dispeler, 31117, &damage, NULL, NULL, true, NULL, NULL,caster_guid);

                iter = m_Auras.begin();                     // iterator can be invalidate at cast if self-dispel
            }
            else
                RemoveAura(iter, AURA_REMOVE_BY_DISPEL);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToSpellBySteal(uint32 spellId, uint64 casterGUID, Unit *stealer)
{
    for (uint8 eff = 0; eff < 3; ++eff)
    {
        bool isStack = false;
        bool onlyDispel = false;
        spellEffectPair spair = spellEffectPair(spellId, eff);
        for(AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair); iter++)
        {
            if(iter->second->GetCasterGUID() == casterGUID)
            {
                Aura *aur = iter->second;
                //int32 basePoints = aur->GetBasePoints(); // i suppose even if we do take real spells 'power' then we should take m_modifier.m_amount, but not basePoints
                // construct the new aura for the attacker
                Aura * new_aur = CreateAura(aur->GetSpellProto(), aur->GetEffIndex(), NULL/*&basePoints*/, stealer);
                if (!new_aur)
                    continue;

                const int32 max_dur = 2*MINUTE*1000;    // max duration 2 minutes (in msecs)
                // Unregister _before_ adding to stealer
                aur->UnregisterSingleCastAura();
                if (aur->GetSpellProto()->StackAmount != 0) // no check for isStack, cause we need to delete new_aur
                {
                    if (Aura* hasAura = stealer->GetAura(aur->GetId(), aur->GetEffIndex()))
                    {
                        if (hasAura->GetStackAmount() < aur->GetSpellProto()->StackAmount)
                        {
                            delete new_aur;
                            isStack = true;
                            new_aur = hasAura;
                        }
                        else
                            onlyDispel = true;
                    }
                }

                // check for similar aura on player stealer, not to override better spell or with longer duration
                else if (GetTypeId() == TYPEID_PLAYER)
                {
                    Unit::AuraMap const& auras = stealer->GetAuras();
                    for (Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                    {
                        if (stealer->GetTypeId() == TYPEID_PLAYER && SpellMgr::GetSpellDuration(aur->GetSpellProto()) >= 60000 
                            && !SpellMgr::IsPassiveSpell(aur->GetSpellProto()) && SpellMgr::IsPositiveSpell(aur->GetSpellProto()->Id) &&
                            stealer->HasMorePowerfulBuff(stealer, aur->GetSpellProto()))
                        {
                            onlyDispel = true;
                            break;
                        }
                    }
                }

                if (isStack)
                {
                    // reapply modifier with reduced stack amount
                    new_aur->ApplyModifier(false,true);
                    new_aur->SetStackAmount(new_aur->GetStackAmount()+1);
                    new_aur->ApplyModifier(true,true);
                    new_aur->UpdateSlotCounterAndDuration();
                }
                // add the new aura to stealer when needed
                else if (!onlyDispel)
                {
                    // set its duration and maximum duration
                    int32 dur = aur->GetAuraDuration();
                    new_aur->SetAuraMaxDuration(max_dur > dur && dur != -1 ? dur : max_dur);
                    new_aur->SetAuraDuration(max_dur > dur && dur != -1 ? dur : max_dur);
                    // strange but intended behaviour: Stolen single target auras won't be treated as single targeted
                    new_aur->SetIsSingleTarget(false);
                    stealer->AddAura(new_aur);
                    stealer->ApplyAdditionalSpell(stealer, aur->GetSpellProto());
					new_aur->UpdateAuraDuration();
                    new_aur->SetStealed(true);
                }
                else
                    delete new_aur;

                // Remove aura as dispel
                if (aur->GetStackAmount() > 1)
                {
                    // reapply modifier with reduced stack amount
                    aur->ApplyModifier(false,true);
                    aur->SetStackAmount(aur->GetStackAmount()-1);
                    aur->ApplyModifier(true,true);

                    aur->UpdateSlotCounterAndDuration();
                }
                else
                    RemoveAura(iter,AURA_REMOVE_BY_DISPEL);
                break;
            }
        }
    }
}

void Unit::RemoveAurasDueToSpellByCancel(uint32 spellId)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if (iter->second->GetId() == spellId)
            RemoveAura(iter, AURA_REMOVE_BY_CANCEL);
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithAttribute_NonPassive(uint32 flags)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        SpellEntry const *spell = iter->second->GetSpellProto();
        if (spell->Attributes & flags && !iter->second->IsPassive())
            RemoveAura(iter);
        else
            ++iter;
    }
}

bool Unit::HasAurasWithDispelMask(uint32 dispelMask)
{
    AuraMap& auras = GetAuras();
    for (AuraMap::iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        if (((1 << itr->second->GetSpellProto()->Dispel) & dispelMask) && !itr->second->IsPassive())
            return true;
    }
    return false;
}

void Unit::RemoveAurasWithDispelType(DispelType type)
{
    // Create dispel mask by dispel type
    uint32 dispelMask = SpellMgr::GetDispellMask(type);
    // Dispel all existing auras vs current dispel type
    AuraMap& auras = GetAuras();
    for (AuraMap::iterator itr = auras.begin(); itr != auras.end();)
    {
        SpellEntry const* spell = itr->second->GetSpellProto();
        if ((1<<spell->Dispel) & dispelMask)
        {
            // Dispel aura
            RemoveAurasDueToSpell(spell->Id);
            itr = auras.begin();
        }
        else
            ++itr;
    }
}

void Unit::RemoveAllAurasButPermanent()    // warlock pet summon with current pet - removing all auras
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        SpellEntry const *spell = iter->second->GetSpellProto();
        int32 duration = SpellMgr::GetSpellDuration(spell);
        if (duration > 0) // Master Demonologist counts as not passive, so duration check will be right, though soul link is not passive and is not removed too. Anyway i think it's better like that
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToDurationAndRecoveryTime()                // DUEL UNBUFF
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        SpellEntry const *spell = iter->second->GetSpellProto();
        int32 cooldown = SpellMgr::GetSpellRecoveryTime(spell);
        int32 duration = SpellMgr::GetSpellDuration(spell);
        if (cooldown > 0 && duration > 0 && cooldown >= duration)
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAuraFromStackByDispel(uint32 spellId, uint64 casterGUID)
{
    for (uint8 effIndex = 0; effIndex < 3; ++effIndex)
        RemoveSingleAuraFromStackByDispel(spellId, casterGUID, effIndex);
}

void Unit::RemoveSingleAuraFromStackByDispel(uint32 spellId, uint64 casterGUID, uint8 eff)
{
    spellEffectPair spair = spellEffectPair(spellId, eff);
    for(AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair); iter++)
    {
        if(iter->second->GetCasterGUID() == casterGUID)
        {
            if (iter->second->GetStackAmount() > 1)
            {
                // reapply modifier with reduced stack amount
                iter->second->ApplyModifier(false,true);
                iter->second->SetStackAmount(iter->second->GetStackAmount()-1);
                iter->second->ApplyModifier(true,true);

                iter->second->UpdateSlotCounterAndDuration();
            } else
                RemoveAura(iter, AURA_REMOVE_BY_DISPEL);
            break;
        }
    }
}

void Unit::RemoveSingleAuraFromStack(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
    {
        if (iter->second->GetStackAmount() > 1)
        {
            // reapply modifier with reduced stack amount
            iter->second->ApplyModifier(false,true);
            iter->second->SetStackAmount(iter->second->GetStackAmount()-1);
            iter->second->ApplyModifier(true,true);

            if (GetTypeId() == TYPEID_UNIT && IsAIEnabled)
                ((Creature *)this)->AI()->OnAuraRemove(iter->second, true);

            iter->second->UpdateSlotCounterAndDuration();
            return; // not remove aura if stack amount > 1
        }
        RemoveAura(iter);
    }
}

void Unit::RemoveSingleAuraFromStackByCaster(uint32 spellId, uint32 effindex, uint64 casterGUID)
{
    spellEffectPair spair = spellEffectPair(spellId, effindex);
    for(AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair); iter++)
    {
        if(iter->second->GetCasterGUID() == casterGUID)
        {
            if (iter->second->GetStackAmount() > 1)
            {
                // reapply modifier with reduced stack amount
                iter->second->ApplyModifier(false,true);
                iter->second->SetStackAmount(iter->second->GetStackAmount()-1);
                iter->second->ApplyModifier(true,true);

                if (GetTypeId() == TYPEID_UNIT && IsAIEnabled)
                    ((Creature *)this)->AI()->OnAuraRemove(iter->second, true);

                iter->second->UpdateSlotCounterAndDuration();
            } else
                RemoveAura(iter);
            break;
        }
    }
}

void Unit::RemoveAurasDueToSpell(uint32 spellId, Aura* except)
{
    for (int i = 0; i < 3; ++i)
        RemoveAura(spellId,i,except);
}

void Unit::RemoveAurasDueToItemSpell(Item* castItem,uint32 spellId)
{
    for (int k=0; k < 3; ++k)
    {
        spellEffectPair spair = spellEffectPair(spellId, k);
        for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
        {
            if (iter->second->GetCastItemGUID() == castItem->GetGUID())
            {
                RemoveAura(iter);
                iter = m_Auras.upper_bound(spair);          // overwrite by more appropriate
            }
            else
                ++iter;
        }
    }
}

void Unit::RemoveNotOwnSingleTargetAuras()
{
    // single target auras from other casters
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if (iter->second->GetCasterGUID()!=GetGUID() && SpellMgr::IsSingleTargetSpell(iter->second->GetSpellProto()))
            RemoveAura(iter);
        else
            ++iter;
    }

    // single target auras at other targets
    AuraList& scAuras = GetSingleCastAuras();

    volatile uint32 debug_guid = GetGUIDLow();
    volatile uint32 debug_map_id = GetMapId();
    volatile uint32 debug_instance = GetInstanciableInstanceId();
    volatile uint32 debug_scAuras_size = scAuras.size();

    //auto iter = scAuras.begin();
    //while (iter != scAuras.end()) 
    //{
    //    Aura* aura = *iter;

    //    if (aura->GetTarget() != this) 
    //    {
    //        uint32 removedAuras = m_removedAurasCount;
    //        aura->GetTarget()->RemoveAura(aura->GetId(), aura->GetEffIndex());
    //        if (m_removedAurasCount > removedAuras + 1)
    //            iter = scAuras.begin();
    //        else
    //            ++iter;
    //    }
    //    else 
    //        ++iter;
    //}
    
    // 08.02.25 causing crashes
    //for (AuraList::iterator iter = scAuras.begin(); iter != scAuras.end();)
    //{
    //    Aura* aur = *iter;
    //    ++iter;

    //    if (aur && aur->GetTarget()!=this)
    //    {
    //        uint32 removedAuras = m_removedAurasCount;
    //        aur->GetTarget()->RemoveAura(aur->GetId(),aur->GetEffIndex());
    //        if (m_removedAurasCount > removedAuras + 1)
    //            iter = scAuras.begin();
    //    }
    //}

    for (AuraList::iterator iter = scAuras.begin(); iter != scAuras.end();)
    {
        Aura* aur = *iter;
        if (!aur)
        {
            ++iter;
            continue;
        }

        ++iter;
        Unit* target = aur->GetTarget();
        if (!target || target == this)
            continue;

        uint32 removedAuras = m_removedAurasCount;
        target->RemoveAura(aur->GetId(), aur->GetEffIndex());

        if (m_removedAurasCount > removedAuras + 1)
        {
            if (!scAuras.empty())
                iter = scAuras.begin();
            else
                break;
        }
    }
}

void Unit::RemoveAura(AuraMap::iterator &i, AuraRemoveMode mode)
{
    Aura* Aur = i->second;

	if (!Aur)
		return;

	// DANGEROUS?
	// don't need to return here?
	// infinity loop 
	// .mod hp 1 -> .aura 33493 -> .cast 45438
	if (Aur->IsInUse())
	{
	    sLog.outLog(LOG_CRITICAL, "Aura %u IsInUse() for name: %s", i->first.first, GetName());
	    sLog.outLog(LOG_CRASH, "Aura %u IsInUse() for name: %s", i->first.first, GetName());
		ASSERT(false);

	    //return;
	} 

    if (this->GetTypeId() == TYPEID_UNIT && this->IsAIEnabled)
        ((Creature *)this)->AI()->OnAuraRemove(Aur, false);

    // if unit currently update aura list then make safe update iterator shift to next
    if (m_AurasUpdateIterator == i)
        ++m_AurasUpdateIterator;

    // some ShapeshiftBoosts at remove trigger removing other auras including parent Shapeshift aura
    // remove aura from list before to prevent deleting it before
    m_Auras.erase(i);
    ++m_removedAurasCount;                                       // internal count used by unit update

    SpellEntry const* AurSpellEntry = Aur->GetSpellProto();
    Unit* caster = NULL;
    Aur->UnregisterSingleCastAura();

    // remove from list before mods removing (prevent cyclic calls, mods added before including to aura list - use reverse order)
    if (Aur->GetModifier()->m_auraname < TOTAL_AURAS)
    {
        m_modAuras[Aur->GetModifier()->m_auraname].remove(Aur); //**

        if (Aur->GetSpellProto()->AuraInterruptFlags)
        {
            m_interruptableAuras.remove(Aur);
            UpdateInterruptMask();
        }

        if ((Aur->GetSpellProto()->Attributes & SPELL_ATTR_BREAKABLE_BY_DAMAGE)
            && (Aur->GetModifier()->m_auraname != SPELL_AURA_MOD_POSSESS)) //only dummy aura is breakable
        {
            m_ccAuras.erase(Aur);
        }
    }

    // Set remove mode
    Aur->SetRemoveMode(mode);

    // Statue unsummoned at aura remove
    Totem* statue = NULL;
    bool channeled = false;
    if (Aur->GetAuraDuration() && !Aur->IsPersistent() && SpellMgr::IsChanneledSpell(AurSpellEntry))
    {
        if (!caster)                                         // can be already located for IsSingleTargetSpell case
            caster = Aur->GetCaster();

        if (caster && caster->isAlive())
        {
            // stop caster chanelling state
            if (caster->m_currentSpells[CURRENT_CHANNELED_SPELL]
                //prevent recurential call
                && caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->getState() != SPELL_STATE_FINISHED)
            {
                if (caster==this || !SpellMgr::IsAreaOfEffectSpell(AurSpellEntry))
                {
                    // remove auras only for non-aoe spells or when chanelled aura is removed
                    // because aoe spells don't require aura on target to continue
                    if (AurSpellEntry->EffectApplyAuraName[Aur->GetEffIndex()]!=SPELL_AURA_PERIODIC_DUMMY
                        && AurSpellEntry->EffectApplyAuraName[Aur->GetEffIndex()]!= SPELL_AURA_DUMMY)
                        //don't stop channeling of scripted spells (this is actually a hack)
                    {
                        caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->cancel(SPELL_FAILED_INT_AURA_REMOVED);
                        caster->m_currentSpells[CURRENT_CHANNELED_SPELL]=NULL;

                    }
                }

                if (caster->GetTypeId()==TYPEID_UNIT && ((Creature*)caster)->isTotem() && ((Totem*)caster)->GetTotemType()==TOTEM_STATUE)
                    statue = ((Totem*)caster);
            }

            // Unsummon summon as possessed creatures on spell cancel
            if (caster->GetTypeId() == TYPEID_PLAYER)
            {
                for (int i = 0; i < 3; ++i)
                {
                    if (AurSpellEntry->Effect[i] == SPELL_EFFECT_SUMMON &&
                        AurSpellEntry->EffectMiscValueB[i] == SUMMON_TYPE_POSESSED)
                         //AurSpellEntry->EffectMiscValueB[i] == SUMMON_TYPE_POSESSED2 ||
                         //AurSpellEntry->EffectMiscValueB[i] == SUMMON_TYPE_POSESSED3))
                    {
                        ((Player*)caster)->StopCastingCharm();
                        break;
                    }
                }
            }
        }
    }

    sLog.outDebug("Aura %u (%u) now is remove mode %d", Aur->GetId(), Aur->GetModifier()->m_auraname, mode);
    //ASSERT(!Aur->IsInUse());
    Aur->ApplyModifier(false,true);

    Aur->SetStackAmount(0);

    // set aura to be removed during unit::_updatespells
    m_removedAuras.push_back(Aur);

    Aur->_RemoveAura();

    bool stack = false || mode == AURA_REMOVE_BY_STACK;
    spellEffectPair spair = spellEffectPair(Aur->GetId(), Aur->GetEffIndex());
    for (AuraMap::const_iterator itr = GetAuras().lower_bound(spair); itr != GetAuras().upper_bound(spair); ++itr)
    {
        if (itr->second->GetCasterGUID() == GetGUID())
        {
            stack = true;
        }
    }

    if (sSpellMgr.GetSpellElixirMask(Aur->GetId()) & (ELIXIR_UNSTABLE_MASK | ELIXIR_SHATTRATH_MASK))
    {
        if (Unit* target = Aur->GetTarget())
        {
            if (AurSpellEntry->EffectTriggerSpell[1])
                target->RemoveAurasDueToSpell(AurSpellEntry->EffectTriggerSpell[1]);  // remove triggered effect of flask on remove
            if (AurSpellEntry->EffectTriggerSpell[2])
                target->RemoveAurasDueToSpell(AurSpellEntry->EffectTriggerSpell[2]);  // remove triggered effect of flask on remove
        }
    }

    if (!stack)
    {
        // Remove all triggered by aura spells vs unlimited duration
        Aur->CleanupTriggeredSpells();

        // Remove Linked Auras
        uint32 id = Aur->GetId();
        SpellEntry const *spellInfo = Aur->GetSpellProto();
        if (spellInfo->AttributesCu & SPELL_ATTR_CU_LINK_REMOVE)
        {
            if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(-(int32)id))
                for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    if (*itr < 0)
                        RemoveAurasDueToSpell(-(*itr));
                    else if (Unit* caster = Aur->GetCaster())
                        CastSpell(this, *itr, true, 0, 0, caster->GetGUID());
        }
        if (spellInfo->AttributesCu & SPELL_ATTR_CU_LINK_AURA)
        {
            if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(id + SPELL_LINK_AURA))
                for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    if (*itr < 0)
                        ApplySpellImmune(id, IMMUNITY_ID, -(*itr), false);
                    else
                        RemoveAurasDueToSpell(*itr);
        }
        if (spellInfo->AttributesCu & SPELL_ATTR_CU_BLOCK_STEALTH)
        {
            bool found = false;
            for (AuraMap::iterator itr = m_Auras.begin(); itr != m_Auras.end(); itr++)
            {
                if (itr->second->GetSpellProto()->AttributesCu & SPELL_ATTR_CU_BLOCK_STEALTH)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                ModifyAuraState(AURA_STATE_FAERIE_FIRE, false);
        }
    }

    if (statue)
        statue->UnSummon();

    i = m_Auras.begin();
}

void Unit::RemoveAllAuras()
{
    while (!m_Auras.empty())
    {
        AuraMap::iterator iter = m_Auras.begin();
        RemoveAura(iter);
    }
}

void Unit::RemoveArenaAuras(bool removeOnlyNegative)
{
    // in join, remove positive buffs, on end, remove negative
    // used to remove positive visible auras in arenas
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if (!(iter->second->GetSpellProto()->AttributesEx4 & (1<<21)) // don't remove stances, shadowform, pally/hunter auras
            && (!iter->second->IsPassive() || (iter->second->GetSpellProto()->AttributesEx4 & SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA))                               // don't remove passive auras
            && ((!(iter->second->GetSpellProto()->Attributes & SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY) || iter->second->GetSpellProto()->Id == 55117) || !(iter->second->GetSpellProto()->Attributes & SPELL_ATTR_UNK8))   // not unaffected by invulnerability auras or not having that unknown flag (that seemed the most probable)
            && (iter->second->GetId() != 54830)
            && (iter->second->GetId() != SPELL_ARENA_DESERTER)
            && (iter->second->GetId() != SPELL_BG_DESERTER) // default deserter
            && (iter->second->GetId() != 37800) // GM invis
            && (removeOnlyNegative ? !iter->second->IsPositive() : true) && !(iter->second->GetSpellProto()->Attributes & SPELL_ATTR_CASTABLE_WHILE_DEAD))                    // remove all buffs on enter, negative buffs on leave*/
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAllAurasOnDeath()
{
    // used just after dieing to remove all visible auras
    // and disable the mods for the passive ones
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if (!iter->second->IsPassive() && !iter->second->IsDeathPersistent())
            RemoveAura(iter, AURA_REMOVE_BY_DEATH);
        else
            ++iter;
    }
    RemoveCharmAuras();
}

void Unit::DelayAura(uint32 spellId, uint32 effindex, int32 delaytime)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
    {
        if (iter->second->GetAuraDuration() < delaytime)
            iter->second->SetAuraDuration(0);
        else
            iter->second->SetAuraDuration(iter->second->GetAuraDuration() - delaytime);
        iter->second->UpdateAuraDuration();
        sLog.outDebug("Aura %u partially interrupted on unit %u, new duration: %u ms",iter->second->GetModifier()->m_auraname, GetGUIDLow(), iter->second->GetAuraDuration());
    }
}

void Unit::_RemoveAllAuraMods()
{
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        (*i).second->ApplyModifier(false);
    }
}

void Unit::_ApplyAllAuraMods()
{
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        (*i).second->ApplyModifier(true);
    }
}

void Unit::ApplyTabardAura()
{
    // this was stupid, but i'll keep it just in case
    //uint32 trigger_auras[] = { 55341, 55343, 55345, 55347, 55349, 55351, 55353, 55355, 55357, 55359, 55361, 55363, 55365, 55367, 55369, 55371, 55373, 55375, 55377 };

    //for (uint32 trigger_aura : trigger_auras)
    //{
    //    AuraMap::const_iterator iter = m_Auras.find(spellEffectPair(trigger_aura, 0));
    //    if (iter != m_Auras.end())
    //    {
    //        // -1 because VISUAL AURA (SPELL) is always previous
    //        //AddAura(aura-1, this);
    //        uint32 visual_aura = trigger_aura - 1;

    //        SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(visual_aura);
    //        if (!spellproto)
    //            return;
    //
    //        ((Player*)this)->ApplyEquipSpell(spellproto, item, true, false);
    //        return;
    //    }           
    //}

    // we must do this by this way, because we can't just re-apply some auras in one diff
    // also must be triggered by an item to make aura disappear on unequip
    if (Item *pItem = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TABARD))
        ((Player*)this)->ApplyItemEquipSpell(pItem, true);       
}

Aura* Unit::GetAura(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
        return iter->second;
    return NULL;
}

bool Unit::HasAura(uint32 spellId) const
{
	for (int i = 0; i < 3 ; ++i)
    {
        AuraMap::const_iterator iter = m_Auras.find(spellEffectPair(spellId, i));
        if (iter != m_Auras.end())
            return true;
    }
    return false;
}

void Unit::AddDynObject(DynamicObject* dynObj)
{
    m_dynObjGUIDs.push_back(dynObj->GetGUID());
}

void Unit::RemoveDynObject(uint32 spellid)
{
    if (m_dynObjGUIDs.empty())
        return;
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = GetMap()->GetDynamicObject(*i);
        if (!dynObj) // may happen if a dynobj is removed when grid unload
        {
            i = m_dynObjGUIDs.erase(i);
        }
        else if (spellid == 0 || dynObj->GetSpellId() == spellid)
        {
            dynObj->Delete();
            i = m_dynObjGUIDs.erase(i);
        }
        else
            ++i;
    }
}

void Unit::RemoveAllDynObjects()
{
    if (Map *map = GetMap())
    {
        while (!m_dynObjGUIDs.empty())
        {
            DynamicObject* dynObj = map->GetDynamicObject(*m_dynObjGUIDs.begin());

            if (dynObj)
                dynObj->Delete();

            m_dynObjGUIDs.erase(m_dynObjGUIDs.begin());
        }
    }
}

DynamicObject * Unit::GetDynObject(uint32 spellId, uint32 effIndex)
{
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = GetMap()->GetDynamicObject(*i);
        if (!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
            continue;
        }

        if (dynObj->GetSpellId() == spellId && dynObj->GetEffIndex() == effIndex)
            return dynObj;
        ++i;
    }
    return NULL;
}

DynamicObject * Unit::GetDynObject(uint32 spellId)
{
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = GetMap()->GetDynamicObject(*i);
        if (!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
            continue;
        }

        if (dynObj->GetSpellId() == spellId)
            return dynObj;
        ++i;
    }
    return NULL;
}

void Unit::AddGameObject(GameObject* gameObj)
{
    ASSERT(gameObj && gameObj->GetOwnerGUID()==0);
    m_gameObj.push_back(gameObj);
    gameObj->SetOwnerGUID(GetGUID());
}

void Unit::RemoveGameObject(GameObject* gameObj, bool del)
{
    ASSERT(gameObj && gameObj->GetOwnerGUID()==GetGUID());

    gameObj->SetOwnerGUID(0);

    // GO created by some spell
    if (uint32 spellid = gameObj->GetSpellId())
    {
        RemoveAurasDueToSpell(spellid);

        if (GetTypeId() == TYPEID_PLAYER)
        {
            SpellEntry const* createBySpell = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
            // Need activate spell use for owner
            if (createBySpell && createBySpell->Attributes & SPELL_ATTR_DISABLED_WHILE_ACTIVE)
                // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existed cases)
                ((Player*)this)->SendCooldownEvent(createBySpell);
        }
    }

    m_gameObj.remove(gameObj);
    if (del)
    {
        gameObj->SetRespawnTime(0);
        gameObj->Delete();
    }
}

void Unit::RemoveGameObject(uint32 spellid, bool del)
{
    if (m_gameObj.empty())
        return;
    std::list<GameObject*>::iterator i, next;
    for (i = m_gameObj.begin(); i != m_gameObj.end(); i = next)
    {
        next = i;
        if (spellid == 0 || (*i)->GetSpellId() == spellid)
        {
            (*i)->SetOwnerGUID(0);
            if (del)
            {
                (*i)->SetRespawnTime(0);
                (*i)->Delete();
            }

            next = m_gameObj.erase(i);
        }
        else
            ++next;
    }
}

void Unit::RemoveAllGameObjects()
{
    // remove references to unit
    for (std::list<GameObject*>::iterator i = m_gameObj.begin(); i != m_gameObj.end();)
    {
        (*i)->SetOwnerGUID(0);
        (*i)->SetRespawnTime(0);
        (*i)->Delete();
        i = m_gameObj.erase(i);
    }
}

void Unit::SendSpellNonMeleeDamageLog(SpellDamageLog *log)
{
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (16+4+4+1+4+4+1+1+4+4+1)); // we guess size
    data << log->target->GetPackGUID();
    data << log->source->GetPackGUID();
    data << uint32(log->spell_id);
    data << uint32(log->damage);                             //damage amount
    data << uint8 (log->schoolMask);                         //damage school
    data << uint32(log->absorb);                             //AbsorbedDamage
    data << uint32(log->resist);                             //resist
    data << uint8 (log->damageType);                         // damsge type? flag
    data << uint8 (0);                                       //unused
    data << uint32(log->blocked);                            //blocked
    data << uint32(log->hitInfo & SPELL_HIT_TYPE_CRIT ? 0x27 : 0x25);
    data << uint8 (0);                                       // flag to use extend data
    BroadcastPacket(&data, true);
}

void Unit::SendSpellNonMeleeDamageLog(Unit *target,uint32 SpellID,uint32 Damage, SpellSchoolMask damageSchoolMask,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked, bool CriticalHit)
{
    sLog.outDebug("Sending: SMSG_SPELLNONMELEEDAMAGELOG");
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (16+4+4+1+4+4+1+1+4+4+1)); // we guess size
    data << target->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(Damage-AbsorbedDamage-Resist-Blocked);
    data << uint8(damageSchoolMask);                        // spell school
    data << uint32(AbsorbedDamage);                         // AbsorbedDamage
    data << uint32(Resist);                                 // resist
    data << uint8(PhysicalDamage);                          // if 1, then client show spell name (example: %s's ranged shot hit %s for %u school or %s suffers %u school damage from %s's spell_name
    data << uint8(0);                                       // unk isFromAura
    data << uint32(Blocked);                                // blocked
    data << uint32(CriticalHit ? 0x27 : 0x25);              // hitType, flags: 0x2 - SPELL_HIT_TYPE_CRIT, 0x10 - replace caster?
    data << uint8(0);                                       // isDebug?
    BroadcastPacket(&data, true);
}

void Unit::ProcDamageAndSpell(Unit *pVictim, uint32 procAttacker, uint32 procVictim, uint32 procExtra, uint32 amount, WeaponAttackType attType, SpellEntry const *procSpell, bool canTrigger)
{
    // Not much to do if no flags are set.
    if (procAttacker && canTrigger)
        ProcDamageAndSpellfor (false,pVictim,procAttacker, procExtra,attType, procSpell, amount);

    // Now go on with a victim's events'n'auras
    // Not much to do if no flags are set or there is no victim
    if (pVictim && (pVictim->isAlive() || pVictim->isDying()) && procVictim && 
        (!procSpell || !(procSpell->AttributesEx4 & SPELL_ATTR_EX4_DAMAGE_DOESNT_BREAK_AURAS))) 
        // 'shield' procs also don't proc back-damage, cause they're not controlled, however they can proc positive, like Impact from Molten Armor
        // Gensen: before change 12.01.23 //pVictim->ProcDamageAndSpellfor (true,this,procVictim, procExtra, attType, procSpell, amount);
		pVictim->ProcDamageAndSpellfor(true, this, procVictim, !procSpell && !pVictim->IsStandState() ? procExtra & ~PROC_EX_CRITICAL_HIT : procExtra, attType, procSpell, amount);

    // Standing up on damage taken must happen after proc checks.
    if (pVictim && pVictim->isAlive() && pVictim->HasUnitState(UNIT_STAT_STAND_UP_PENDING))
    {
        pVictim->SetStandState(UNIT_STAND_STATE_STAND);
        pVictim->ClearUnitState(UNIT_STAT_STAND_UP_PENDING);
    }
}

void Unit::SendSpellMiss(Unit *target, uint32 spellID, SpellMissInfo missInfo)
{
    WorldPacket data(SMSG_SPELLLOGMISS, (4+8+1+4+8+1));
    data << uint32(spellID);
    data << uint64(GetGUID());
    data << uint8(0);                                       // can be 0 or 1
    data << uint32(1);                                      // target count
    // for (i = 0; i < target count; ++i)
    data << uint64(target->GetGUID());                      // target GUID
    data << uint8(missInfo);
    // end loop
    BroadcastPacket(&data, true);
}

void Unit::SendAttackStateUpdate(MeleeDamageLog *damageInfo)
{
    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (16+84));    // we guess size
    data << (uint32)damageInfo->hitInfo;
    data << GetPackGUID();
    data << damageInfo->target->GetPackGUID();
    data << (uint32)(damageInfo->damage);     // Full damage

    data << (uint8)1;                         // Sub damage count
    //===  Sub damage description
    data << (uint32)(damageInfo->schoolMask); // School of sub damage
    data << (float)damageInfo->damage;        // sub damage
    data << (uint32)damageInfo->damage;       // Sub Damage
    data << (uint32)damageInfo->absorb;       // Absorb
    data << (uint32)damageInfo->resist;       // Resist
    //=================================================
    data << (uint32)damageInfo->targetState;
    data << (uint32)0;
    data << (uint32)0;
    data << (uint32)damageInfo->blocked;
    BroadcastPacket(&data, true);/**/
}

void Unit::SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount)
{
    sLog.outDebug("WORLD: Sending SMSG_ATTACKERSTATEUPDATE");

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (16+45));    // we guess size
    data << (uint32)HitInfo;
    data << GetPackGUID();
    data << target->GetPackGUID();
    data << (uint32)(Damage-AbsorbDamage-Resist-BlockedAmount);

    data << (uint8)SwingType;                               // count?

    // for (i = 0; i < SwingType; ++i)
    data << (uint32)damageSchoolMask;
    data << (float)(Damage-AbsorbDamage-Resist-BlockedAmount);
    // still need to double check damage
    data << (uint32)(Damage-AbsorbDamage-Resist-BlockedAmount);
    data << (uint32)AbsorbDamage;
    data << (uint32)Resist;
    // end loop

    data << (uint32)TargetState;

    if (AbsorbDamage == 0)                                 //also 0x3E8 = 0x3E8, check when that happens
        data << (uint32)0;
    else
        data << (uint32)-1;

    data << (uint32)0;
    data << (uint32)BlockedAmount;

    BroadcastPacket(&data, true);
}
bool Unit::HandleHasteAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const * procSpell, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 cooldown)
{
    SpellEntry const *hasteSpell = triggeredByAura->GetSpellProto();

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId()==TYPEID_PLAYER
        ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    uint32 triggered_spell_id = 0;
    Unit* target = pVictim;
    int32 basepoints0 = 0;

    switch (hasteSpell->SpellFamilyName)
    {
        case SPELLFAMILY_ROGUE:
        {
            switch (hasteSpell->Id)
            {
                // Blade Flurry
                case 13877:
                case 33735:
                {
                    target = SelectNearbyTarget(8.0f);
                    if (!target)
                        return false;
                    basepoints0 = damage;
                    triggered_spell_id = 22482;
                    break;
                }
            }
            break;
        }
    }

    // processed charge only counting case
    if (!triggered_spell_id)
        return true;

    SpellEntry const* triggerEntry = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!triggerEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleHasteAuraProc: Spell %u have not existed triggered spell %u",hasteSpell->Id,triggered_spell_id);
        return false;
    }

    // default case
    if (!target || target != this && !target->isAlive())
        return false;

    if (cooldown && GetTypeId()==TYPEID_PLAYER && ((Player*)this)->HasSpellCooldown(triggered_spell_id))
        return false;

    if (basepoints0)
        CastCustomSpell(target,triggered_spell_id,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
    else
        CastSpell(target,triggered_spell_id,true,castItem,triggeredByAura);

    if (cooldown && GetTypeId()==TYPEID_PLAYER)
        ((Player*)this)->AddSpellCooldown(triggered_spell_id,time(NULL) + cooldown);

    return true;
}

bool Unit::HandleDummyAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const * procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown)
{
    SpellEntry const *dummySpell = triggeredByAura->GetSpellProto ();
    uint32 effIndex = triggeredByAura->GetEffIndex ();

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId() == TYPEID_PLAYER
        ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    uint32 triggered_spell_id = 0;
    Unit* target = pVictim;
    int32 basepoints0 = 0;
    uint64 originalCaster = 0;

    switch (dummySpell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (dummySpell->Id)
            {
                // Eye of Eye
                case 9799:
                case 25988:
                {
                    // prevent damage back from weapon special attacks
                    if (!procSpell || procSpell->DmgClass != SPELL_DAMAGE_CLASS_MAGIC)
                        return false;

                    // return damage % to attacker but < 50% own total health
                    basepoints0 = triggeredByAura->GetModifier()->m_amount*int32(damage)/100;
                    if (basepoints0 > GetMaxHealth()/2)
                        basepoints0 = GetMaxHealth()/2;

                    triggered_spell_id = 25997;
                    break;
                }
                // Sweeping Strikes
                case 12328:
                case 18765:
                case 35429:
                {
                    // prevent trigger from self
                    if (procSpell && procSpell->Id == 12723)
                        return false;

                    target = SelectNearbyTarget(8.0f, target);
                    if (!target)
                        return false;

                    triggered_spell_id = 12723;

                    if (procSpell)
                    {
                        // Execute will transfer normal swing damage amount if 2nd target HP is above 20%
                        if (procSpell->Id == 20647 && target->GetHealth() > target->GetMaxHealth() *0.2f)
                        {
                            damage = CalculateDamage(BASE_ATTACK, false);
                            MeleeDamageBonus(target, &damage, BASE_ATTACK);
                            basepoints0 = damage;
                            break;
                        }

                        // Limit WhirlWind to hit one target applying 1s cooldown
                        if (procSpell->SpellFamilyName == SPELLFAMILY_WARRIOR && procSpell->SpellFamilyFlags & 0x400000000LL)
                            cooldown = 1;
                    }

                    float armor = pVictim->GetArmor();
                    // Ignore enemy armor by SPELL_AURA_MOD_TARGET_RESISTANCE aura
                    armor += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, SPELL_SCHOOL_MASK_NORMAL);

                    if (armor < 0.0f)
                        armor = 0.0f;

                    float levelModifier = GetLevel();
                    if (levelModifier > 59)
                        levelModifier = levelModifier + (4.5f * (levelModifier-59));

                    float mitigation = 0.1f * armor / (8.5f * levelModifier + 40);
                    mitigation = mitigation/(1.0f + mitigation);

                    if (mitigation < 0.0f)
                        mitigation = 0.0f;

                    if (mitigation > 0.75f)
                        mitigation = 0.75f;

                    // calculate base damage before armor mitigation
                    damage = uint32(damage / (1.0f - mitigation));

                    basepoints0 = damage;
                    break;
                }
                // Unstable Power
                case 24658:
                {
                    if (!procSpell || procSpell->Id == 24659)
                        return false;
                    // Need remove one 24659 aura
                    RemoveSingleAuraFromStack(24659, 0);
                    RemoveSingleAuraFromStack(24659, 1);
                    return true;
                }
                // Restless Strength
                case 24661:
                {
                    // Need remove one 24662 aura
                    RemoveSingleAuraFromStack(24662, 0);
                    return true;
                }
                // Adaptive Warding (Frostfire Regalia set)
                case 28764:
                {
                    if (!procSpell)
                        return false;

                    // find Mage Armor
                    bool found = false;
                    AuraList const& mRegenInterupt = GetAurasByType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
                    for (AuraList::const_iterator iter = mRegenInterupt.begin(); iter != mRegenInterupt.end(); ++iter)
                    {
                        if (SpellEntry const* iterSpellProto = (*iter)->GetSpellProto())
                        {
                            if (iterSpellProto->SpellFamilyName==SPELLFAMILY_MAGE && (iterSpellProto->SpellFamilyFlags & 0x10000000))
                            {
                                found=true;
                                break;
                            }
                        }
                    }
                    if (!found)
                        return false;

                    switch (GetFirstSchoolInMask(SpellMgr::GetSpellSchoolMask(procSpell)))
                    {
                        case SPELL_SCHOOL_NORMAL:
                        case SPELL_SCHOOL_HOLY:
                            return false;                   // ignored
                        case SPELL_SCHOOL_FIRE:   triggered_spell_id = 28765; break;
                        case SPELL_SCHOOL_NATURE: triggered_spell_id = 28768; break;
                        case SPELL_SCHOOL_FROST:  triggered_spell_id = 28766; break;
                        case SPELL_SCHOOL_SHADOW: triggered_spell_id = 28769; break;
                        case SPELL_SCHOOL_ARCANE: triggered_spell_id = 28770; break;
                        default:
                            return false;
                    }

                    target = this;
                    break;
                }
                // Obsidian Armor (Justice Bearer`s Pauldrons shoulder)
                case 27539:
                {
                    if (!procSpell)
                        return false;

                    switch (GetFirstSchoolInMask(SpellMgr::GetSpellSchoolMask(procSpell)))
                    {
                        case SPELL_SCHOOL_NORMAL:
                            return false;                   // ignore
                        case SPELL_SCHOOL_HOLY:   triggered_spell_id = 27536; break;
                        case SPELL_SCHOOL_FIRE:   triggered_spell_id = 27533; break;
                        case SPELL_SCHOOL_NATURE: triggered_spell_id = 27538; break;
                        case SPELL_SCHOOL_FROST:  triggered_spell_id = 27534; break;
                        case SPELL_SCHOOL_SHADOW: triggered_spell_id = 27535; break;
                        case SPELL_SCHOOL_ARCANE: triggered_spell_id = 27540; break;
                        default:
                            return false;
                    }

                    target = this;
                    break;
                }
                // Mana Leech (Passive) (Priest Pet Aura)
                case 28305:
                {
                    // Cast on owner
                    target = GetOwner();
                    if (!target)
                        return false;

                    basepoints0 = int32(damage * 2.5f);     // manaregen
                    triggered_spell_id = 34650;
                    break;
                }
                case 30039:
                {
                    // Transference just does the same healing to filcher, it doesnt
                    // actually redirect
                    basepoints0 = damage;
                    triggered_spell_id = 30107;
                    target = this; // Target_script
                    break;
                }
                // Twisted Reflection (boss spell)
                case 21063:
                    triggered_spell_id = 21064;
                    break;
                // Vampiric Aura (boss spell)
                case 38196:
                {
                    basepoints0 = 3 * damage;               // 300%
                    if (basepoints0 < 0)
                        return false;

                    triggered_spell_id = 31285;
                    target = this;
                    break;
                }
                // Consuming Strikes
                case 41248:
                {
                    basepoints0 = damage;               // 100%
                    if (basepoints0 < 0)
                        return false;

                    triggered_spell_id = 41249;
                    target = this;
                    break;

                }
                // Aura of Madness (Darkmoon Card: Madness trinket)
                //=====================================================
                // 39511 Sociopath: +35 strength (Paladin, Rogue, Druid, Warrior)
                // 40997 Delusional: +70 attack power (Rogue, Hunter, Paladin, Warrior, Druid)
                // 40998 Kleptomania: +35 agility (Warrior, Rogue, Paladin, Hunter, Druid)
                // 40999 Megalomania: +41 damage/healing (Druid, Shaman, Priest, Warlock, Mage, Paladin)
                // 41002 Paranoia: +35 spell/melee/ranged crit strike rating (All classes)
                // 41005 Manic: +35 haste (spell, melee and ranged) (All classes)
                // 41009 Narcissism: +35 intellect (Druid, Shaman, Priest, Warlock, Mage, Paladin, Hunter)
                // 41011 Martyr Complex: +35 stamina (All classes)
                // 41406 Dementia: Every 5 seconds either gives you +5% damage/healing. (Druid, Shaman, Priest, Warlock, Mage, Paladin)
                // 41409 Dementia: Every 5 seconds either gives you -5% damage/healing. (Druid, Shaman, Priest, Warlock, Mage, Paladin)
                case 39446:
                {
                    if (GetTypeId() != TYPEID_PLAYER || !this->isAlive())
                        return false;

                    // Select class defined buff
                    switch (GetClass())
                    {
                        case CLASS_PALADIN:                 // 39511,40997,40998,40999,41002,41005,41009,41011,41409
                        case CLASS_DRUID:                   // 39511,40997,40998,40999,41002,41005,41009,41011,41409
                        {
                            uint32 RandomSpell[]={39511,40997,40998,40999,41002,41005,41009,41011,41409};
                            triggered_spell_id = RandomSpell[ irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        case CLASS_ROGUE:                   // 39511,40997,40998,41002,41005,41011
                        case CLASS_WARRIOR:                 // 39511,40997,40998,41002,41005,41011
                        {
                            uint32 RandomSpell[]={39511,40997,40998,41002,41005,41011};
                            triggered_spell_id = RandomSpell[ irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        case CLASS_PRIEST:                  // 40999,41002,41005,41009,41011,41406,41409
                        case CLASS_SHAMAN:                  // 40999,41002,41005,41009,41011,41406,41409
                        case CLASS_MAGE:                    // 40999,41002,41005,41009,41011,41406,41409
                        case CLASS_WARLOCK:                 // 40999,41002,41005,41009,41011,41406,41409
                        {
                            uint32 RandomSpell[]={40999,41002,41005,41009,41011,41406,41409};
                            triggered_spell_id = RandomSpell[ irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        case CLASS_HUNTER:                  // 40997,40999,41002,41005,41009,41011,41406,41409
                        {
                            uint32 RandomSpell[]={40997,40999,41002,41005,41009,41011,41406,41409};
                            triggered_spell_id = RandomSpell[ irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        default:
                            return false;
                    }

                    target = this;
                    if (roll_chance_i(10))
                        ((Player*)this)->Say("This is Madness!", LANG_UNIVERSAL);
                    break;
                }
                /*
                // TODO: need find item for aura and triggered spells
                // Sunwell Exalted Caster Neck (??? neck)
                // cast ??? Light's Wrath if Exalted by Aldor
                // cast ??? Arcane Bolt if Exalted by Scryers*/
                case 46569:
                    return false;                           // disable for while
                /*
                {
                    if (GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = ???
                        break;
                    }
                    // Get Scryers reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(934) == REP_EXALTED)
                    {
                        triggered_spell_id = ???
                        break;
                    }
                    return false;
                }/**/
                // Sunwell Exalted Caster Neck (Shattered Sun Pendant of Acumen neck)
                // cast 45479 Light's Wrath if Exalted by Aldor
                // cast 45429 Arcane Bolt if Exalted by Scryers
                case 45481:
                {
                    if (GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45479;
                        break;
                    }
                    // Get Scryers reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(934) == REP_EXALTED)
                    {
                        if (this->IsFriendlyTo(target))
                            return false;

                        triggered_spell_id = 45429;
                        break;
                    }
                    return false;
                }
                // Sunwell Exalted Melee Neck (Shattered Sun Pendant of Might neck)
                // cast 45480 Light's Strength if Exalted by Aldor
                // cast 45428 Arcane Strike if Exalted by Scryers
                case 45482:
                {
                    if (GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45480;
                        break;
                    }
                    // Get Scryers reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(934) == REP_EXALTED)
                    {
                        triggered_spell_id = 45428;
                        break;
                    }
                    return false;
                }
                // Sunwell Exalted Tank Neck (Shattered Sun Pendant of Resolve neck)
                // cast 45431 Arcane Insight if Exalted by Aldor
                // cast 45432 Light's Ward if Exalted by Scryers
                case 45483:
                {
                    if (GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45432;
                        break;
                    }
                    // Get Scryers reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(934) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45431;
                        break;
                    }
                    return false;
                }
                // Sunwell Exalted Healer Neck (Shattered Sun Pendant of Restoration neck)
                // cast 45478 Light's Salvation if Exalted by Aldor
                // cast 45430 Arcane Surge if Exalted by Scryers
                case 45484:
                {
                    if (GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45478;
                        break;
                    }
                    // Get Scryers reputation rank
                    if (((Player *)this)->GetReputationMgr().GetRank(934) == REP_EXALTED)
                    {
                        triggered_spell_id = 45430;
                        break;
                    }
                    return false;
                }
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // Magic Absorption
            if (dummySpell->SpellIconID == 459)             // only this spell have SpellIconID == 459 and dummy aura
            {
                if (getPowerType() != POWER_MANA)
                    return false;

                // mana reward
                basepoints0 = (triggeredByAura->GetModifier()->m_amount * GetMaxPower(POWER_MANA) / 100);
                target = this;
                triggered_spell_id = 29442;
                break;
            }
            // Master of Elements
            if (dummySpell->SpellIconID == 1920)
            {
                if (!procSpell)
                    return false;

                // mana cost save
                basepoints0 = procSpell->manaCost * triggeredByAura->GetModifier()->m_amount / 100;
                if (basepoints0 <= 0)
                    return false;

                target = this;
                triggered_spell_id = 29077;
                break;
            }

            // Incanter's Regalia set (add trigger chance to Mana Shield)
            if (dummySpell->SpellFamilyFlags & 0x0000000000008000LL)
            {
                if (GetTypeId() != TYPEID_PLAYER || !HasAura(37424, 0))
                    return false;

                target = this;
                triggered_spell_id = 37436;
                break;
            }

            switch (dummySpell->Id)
            {
                // Ignite
                case 11119:
                case 11120:
                case 12846:
                case 12847:
                case 12848:
                {
                    if (procSpell && procSpell->Id == 34913) // Don't proc from Molten Armor
                        return false;
                    if (triggeredByAura->GetCaster()->IsFriendlyTo(pVictim))
                        return false;

                    switch (dummySpell->Id)
                    {
                        case 11119: basepoints0 = damage * (GetDummyAura(54889) ? 0.08f : 0.04f); break;
                        case 11120: basepoints0 = damage * (GetDummyAura(54889) ? 0.12f : 0.08f); break;
                        case 12846: basepoints0 = damage * (GetDummyAura(54889) ? 0.16f : 0.12f); break;
                        case 12847: basepoints0 = damage * (GetDummyAura(54889) ? 0.20f : 0.16f); break;
                        case 12848: basepoints0 = damage * (GetDummyAura(54889) ? 0.24f : 0.20f); break;
                         default:
                             sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleDummyAuraProc: non handled spell id: %u (IG)",dummySpell->Id);
                             return false;
                     }

                    AuraList const &DoT = pVictim->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (AuraList::const_iterator itr = DoT.begin(); itr != DoT.end(); ++itr)
                        if ((*itr)->GetId() == 12654 && (*itr)->GetCaster() == this)
                            if ((*itr)->GetBasePoints() > 0)
                                basepoints0 += int((*itr)->GetBasePoints()/((*itr)->GetTickNumber() + 1));

                    triggered_spell_id = 12654;
                    break;
                }
                // Combustion
                case 11129:
                {
                    //last charge and crit
                    if (triggeredByAura->m_procCharges <= 1 && (procEx & PROC_EX_CRITICAL_HIT))
                    {
                        RemoveAurasDueToSpell(28682);       //-> remove Combustion auras
                        return true;                        // charge counting (will removed)
                    }

                    CastSpell(this, 28682, true, castItem, triggeredByAura);
                    return (procEx & PROC_EX_CRITICAL_HIT);// charge update only at crit hits, no hidden cooldowns
                }
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Retaliation
            if (dummySpell->SpellFamilyFlags==0x0000000800000000LL)
            {
                // check attack comes not from behind
                if (!HasInArc(M_PI, pVictim))
                    return false;

                triggered_spell_id = 22858;
                break;
            }
            else if (dummySpell->SpellIconID == 1697)  // Second Wind
            {
                // only for spells and hit/crit (trigger start always) and not start from self cast spells (5530 Mace Stun Effect for example)
                if (procSpell == 0 || !(procEx & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)) || this == pVictim)
                    return false;
                // Need stun or root mechanic
                if (procSpell->Mechanic != MECHANIC_ROOT && procSpell->Mechanic != MECHANIC_STUN)
                {
                    int32 i;
                    for (i=0; i<3; i++)
                        if (procSpell->EffectMechanic[i] == MECHANIC_ROOT || procSpell->EffectMechanic[i] == MECHANIC_STUN)
                            break;
                    if (i == 3)
                        return false;
                }

                switch (dummySpell->Id)
                {
                    case 29838: triggered_spell_id=29842; break;
                    case 29834: triggered_spell_id=29841; break;
                    default:
                        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleDummyAuraProc: non handled spell id: %u (SW)",dummySpell->Id);
                    return false;
                }

                target = this;
                break;
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Seed of Corruption
            if (dummySpell->SpellFamilyFlags & 0x0000001000000000LL)
            {
                if (procSpell && procSpell->Id == 27285)
                    return false;
                Modifier* mod = triggeredByAura->GetModifier();
                // if damage is more than need or target die from damage deal finish spell
                if (mod->m_amount <= damage || GetHealth() <= damage || procFlag & PROC_FLAG_KILLED)
                {
                    // remember guid before aura delete
                    uint64 casterGuid = triggeredByAura->GetCasterGUID();

                    // Remove aura (before cast for prevent infinite loop handlers)
                    RemoveAurasByCasterSpell(triggeredByAura->GetId(), casterGuid);

                    // Cast finish spell (triggeredByAura already not exist!)
                    if (Unit* caster = GetUnit(*this, casterGuid))
                        caster->CastSpell(this, 27285, true, castItem);
                    return true;                            // no hidden cooldown
                }

                // Damage counting
                mod->m_amount-=damage;
                return true;
            }
            // Seed of Corruption (Mobs cast) - no die req
            if (dummySpell->SpellFamilyFlags == 0x00LL && dummySpell->SpellIconID == 1932)
            {
                Modifier* mod = triggeredByAura->GetModifier();
                // if damage is more than need deal finish spell
                if (mod->m_amount <= damage)
                {
                    // remember guid before aura delete
                    uint64 casterGuid = triggeredByAura->GetCasterGUID();

                    // Remove aura (before cast for prevent infinite loop handlers)
                    RemoveAurasByCasterSpell(triggeredByAura->GetId(), casterGuid);

                    // Cast finish spell (triggeredByAura already not exist!)
                    if (Unit* caster = GetUnit(*this, casterGuid))
                        caster->CastSpell(this, 32865, true, castItem);
                    return true;                            // no hidden cooldown
                }
                // Damage counting
                mod->m_amount-=damage;
                return true;
            }
            switch (dummySpell->Id)
            {
                // Nightfall
                case 18094:
                case 18095:
                {
                    target = this;
                    triggered_spell_id = 17941;
                    break;
                }
                //Soul Leech
                case 30293:
                case 30295:
                case 30296:
                {
                    // health
                    basepoints0 = int32(damage*triggeredByAura->GetModifier()->m_amount/100);
                    target = this;
                    triggered_spell_id = 30294;
                    break;
                }
                // Shadowflame (Voidheart Raiment set bonus)
                case 37377:
                {
                    triggered_spell_id = 37379;
                    break;
                }
                // Pet Healing (Corruptor Raiment or Rift Stalker Armor)
                case 37381:
                {
                    target = GetPet();
                    if (!target)
                        target = GetEnslaved();
                    if (!target || !target->isAlive())
                        return false;

                    // heal amount
                    basepoints0 = damage * 0.01 * triggeredByAura->GetModifier()->m_amount;
                    triggered_spell_id = 37382;
                    break;
                }
                // Shadowflame Hellfire (Voidheart Raiment set bonus)
                case 39437:
                {
                    triggered_spell_id = 37378;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            // Vampiric Touch
            if (dummySpell->SpellFamilyFlags & 0x0000040000000000LL)
            {
                if (!pVictim || pVictim->isDead())
                    return false;

                // pVictim is caster of aura
                if (triggeredByAura->GetCasterGUID() != pVictim->GetGUID())
                    return false;

                if (!pVictim->IsInMap(triggeredByAura->GetCaster()))
                    return false;

                // energize amount
                basepoints0 = triggeredByAura->GetModifier()->m_amount*damage/100;
                pVictim->CastCustomSpell(pVictim,34919,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
                return true;                                // no hidden cooldown
            }
            switch (dummySpell->Id)
            {
                // Vampiric Embrace
                case 15286:
                {
                    if (!pVictim || !pVictim->isAlive())
                        return false;

                    // pVictim is caster of aura
                    if (triggeredByAura->GetCasterGUID() != pVictim->GetGUID())
                        return false;

                    if (!pVictim->IsInMap(triggeredByAura->GetCaster()))
                        return false;

                    // heal amount
                    basepoints0 = triggeredByAura->GetModifier()->m_amount*damage/100;
                    pVictim->CastCustomSpell(pVictim,15290,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
                    return true;                                // no hidden cooldown
                }
                // Priest Tier 6 Trinket (Ashtongue Talisman of Acumen)
                case 40438:
                {
                    // Shadow Word: Pain
                    if (procSpell->SpellFamilyFlags & 0x0000000000008000LL)
                        triggered_spell_id = 40441;
                    // Renew
                    else if (procSpell->SpellFamilyFlags & 0x0000000000000040LL)
                        triggered_spell_id = 40440;
                    else
                        return false;

                    target = this;
                    break;
                }
                // Oracle Healing Bonus ("Garments of the Oracle" set)
                case 26169:
                {
                    // heal amount
                    basepoints0 = int32(damage * 10/100);
                    target = this;
                    triggered_spell_id = 26170;
                    break;
                }
                // Frozen Shadoweave (Shadow's Embrace set) warning! its not only priest set
                case 39372:
                {
                    if (!procSpell || (SpellMgr::GetSpellSchoolMask(procSpell) & (SPELL_SCHOOL_MASK_FROST | SPELL_SCHOOL_MASK_SHADOW))==0)
                        return false;

                    if (!isAlive())
                        return false;

                    // do NOT proc on cross map interaction(rare case)
                    if (!IsInWorld() || (pVictim && !IsInMap(pVictim)))
                        return false;

                    // heal amount
                    basepoints0 = int32(damage * 2 / 100);
                    target = this;
                    triggered_spell_id = 39373;
                    break;
                }
                // Vestments of Faith (Priest Tier 3) - 4 pieces bonus
                case 28809:
                {
                    triggered_spell_id = 28810;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch (dummySpell->Id)
            {
                // Healing Touch (Dreamwalker Raiment set)
                case 28719:
                {
                    // mana back
                    basepoints0 = int32(procSpell->manaCost * 30 / 100);
                    target = this;
                    triggered_spell_id = 28742;
                    break;
                }
                // Healing Touch Refund (Idol of Longevity trinket)
                case 28847:
                {
                    target = this;
                    triggered_spell_id = 28848;
                    break;
                }
                // Mana Restore (Malorne Raiment set / Malorne Regalia set)
                case 37288:
                case 37295:
                {
                    target = this;
                    triggered_spell_id = 37238;
                    break;
                }
                // Druid Tier 6 Trinket
                case 40442:
                {
                    float  chance;

                    // Starfire
                    if (procSpell->SpellFamilyFlags & 0x0000000000000004LL)
                    {
                        triggered_spell_id = 40445;
                        chance = 25.f;
                    }
                    // Rejuvenation
                    else if (procSpell->SpellFamilyFlags & 0x0000000000000010LL)
                    {
                        triggered_spell_id = 40446;
                        chance = 25.f;
                    }
                    // Mangle (cat/bear)
                    else if (procSpell->SpellFamilyFlags & 0x0000044000000000LL)
                    {
                        triggered_spell_id = 40452;
                        chance = 40.f;
                    }
                    else
                        return false;

                    if (!roll_chance_f(chance))
                        return false;

                    target = this;
                    break;
                }
                // Maim Interrupt - handled in Spell::SpellDamageWeaponDmg
                /*
                case 44835:
                {
                    // Deadly Interrupt Effect
                    triggered_spell_id = 32747;
                    break;
                }
                */
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            switch (dummySpell->Id)
            {
                // Deadly Throw Interrupt
                case 32748:
                {
                    // Prevent cast Deadly Throw Interrupt on self from last effect (apply dummy) of Deadly Throw
                    if (this == pVictim)
                        return false;

                    triggered_spell_id = 32747;
                    break;
                }
            }
            // Quick Recovery
            if (dummySpell->SpellIconID == 2116)
            {
                if (!procSpell)
                    return false;

                // only rogue's finishing moves (maybe need additional checks)
                if (procSpell->SpellFamilyName!=SPELLFAMILY_ROGUE ||
                    (procSpell->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE__FINISHING_MOVE) == 0)
                    return false;

                // energy cost save
                basepoints0 = procSpell->manaCost * triggeredByAura->GetModifier()->m_amount/100;
                if (basepoints0 <= 0)
                    return false;

                target = this;
                triggered_spell_id = 31663;
                break;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Thrill of the Hunt
            if (dummySpell->SpellIconID == 2236)
            {
                if (!procSpell)
                    return false;

                // mana cost save
                basepoints0 = (procSpell->manaCost + procSpell->ManaCostPercentage * (GetCreateMana() / 100)) * 40 / 100;
                if (basepoints0 <= 0)
                    return false;

                target = this;
                triggered_spell_id = 34720;
                break;
            }
            // improved kill command
            else if (dummySpell->Id == 37483)
            {
                triggered_spell_id = 37482;
                target = this;
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Seal of Righteousness - melee proc dummy
            if (dummySpell->SpellFamilyFlags&0x000000008000000LL && triggeredByAura->GetEffIndex()==0)
            {
                if (GetTypeId() != TYPEID_PLAYER)
                    return false;

                uint32 spellId;
                switch (triggeredByAura->GetId())
                {
                    case 21084: spellId = 25742; break;     // Rank 1
                    case 20287: spellId = 25740; break;     // Rank 2
                    case 20288: spellId = 25739; break;     // Rank 3
                    case 20289: spellId = 25738; break;     // Rank 4
                    case 20290: spellId = 25737; break;     // Rank 5
                    case 20291: spellId = 25736; break;     // Rank 6
                    case 20292: spellId = 25735; break;     // Rank 7
                    case 20293: spellId = 25713; break;     // Rank 8
                    case 27155: spellId = 27156; break;     // Rank 9
                    default:
                        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleDummyAuraProc: non handled possibly SoR (Id = %u)", triggeredByAura->GetId());
                        return false;
                }
                Item *item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                float speed = (item ? item->GetProto()->Delay : BASE_ATTACK_TIME)/1000.0f;

                float damageBasePoints;
                if (item && item->GetProto()->InventoryType == INVTYPE_2HWEAPON)
                    // two hand weapon
                    damageBasePoints=1.20f*triggeredByAura->GetModifier()->m_amount * 1.2f * 1.03f * speed/100.0f + 1;
                else
                    // one hand weapon/no weapon
                    damageBasePoints=0.85f*ceil(triggeredByAura->GetModifier()->m_amount * 1.2f * 1.03f * speed/100.0f) - 1;

                int32 damagePoint = int32(damageBasePoints + 0.03f * (GetWeaponDamageRange(BASE_ATTACK,MINDAMAGE)+GetWeaponDamageRange(BASE_ATTACK,MAXDAMAGE))/2.0f) + 1;

                // apply damage bonuses manually
                if (damagePoint >= 0)
                    damagePoint = SpellDamageBonus(pVictim, dummySpell, damagePoint, SPELL_DIRECT_DAMAGE);

                CastCustomSpell(pVictim,spellId,&damagePoint,NULL,NULL,true,NULL);  // proc can't miss so we don't provide triggeredByAura
                return true;                                // no hidden cooldown
            }

            // Seal of Blood do damage trigger
            if (dummySpell->SpellFamilyFlags & 0x0000040000000000LL)
            {
                switch (triggeredByAura->GetEffIndex())
                {
                    case 0:
                        triggered_spell_id = 31893;
                        //if (procSpell && procSpell->Id == 20424) // Seal of Command proc -> double Seal of Blood proc!
                        //{
                        //    // default case
                        //    if (!target || target != this && !target->isAlive())
                        //        return false;

                        //    CastSpell(target, triggered_spell_id, true, castItem, triggeredByAura, originalCaster);
                        //}
                        break;
                    /*case 1:
                    {
                        basepoints0 = triggeredByAura->GetModifier()->m_amount * damage * 35 / 10000;

                        target = this;

                        triggered_spell_id = 32221;
                        break;
                    }*/ // moved to DoAllEffectOnTarget for right damage calculation
                }
            }

            switch (dummySpell->Id)
            {
                // Holy Power (Redemption Armor set)
                case 28789:
                {
                    if (!pVictim)
                        return false;

                    // Set class defined buff
                    switch (pVictim->GetClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28795;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28793;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28791;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28790;     // Increases the friendly target's armor
                            break;
                        default:
                            return false;
                    }
                    break;
                }
                //Seal of Vengeance
                case 31801:
                {
                    if (effIndex != 0)                       // effect 1,2 used by seal unleashing code
                        return false;

                    triggered_spell_id = 31803;

                     // Only Autoattack can stack debuff and deal bonus damage
                    if (procFlag & PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT)
                        return false;

                    // On target with 5 stacks of Holy Vengeance direct damage is done
                    Aura* sealAura = NULL;
                    Unit::AuraList const& auras = pVictim->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                    {
                        if ((*itr)->GetId() == 31803 && (*itr)->GetCasterGUID() == GetGUID())
                        {
                            if ((*itr)->GetStackAmount() >= 5)
                                sealAura = *itr;

                            break;
                        }
                    }

                    if (sealAura)
                    {
                        Item *item = NULL;
                        if (ToPlayer())
                            item = ToPlayer()->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                        float speed = (item ? item->GetProto()->Delay : BASE_ATTACK_TIME)/1000.0f;
                        int32 bp0 = 10*speed;
                        CastCustomSpell(pVictim, 42463, &bp0, 0,0, true);
                    }
                    break;
                }
                // Spiritual Att.
                case 31785:
                case 33776:
                {
                    // if healed by another unit (pVictim)
                    if (this == pVictim)
                        return false;

                    // heal amount
                    basepoints0 = triggeredByAura->GetModifierValue()*std::min(damage,GetMaxHealth() - GetHealth())/100;
                    target = this;
                    
                    if (GetMap()->IsDungeon()/*dungeon or raid*/ && !GetMap()->IsHeroicRaid())
                    {
                        float mod = sWorld.getConfig(RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE); // in non-raid-heroic dungeons all damage is same -> taken from config
                        if (mod < 1.0f)
                            basepoints0 /= mod;
                    }

                    if (basepoints0)
                        triggered_spell_id = 31786;
                    break;
                }
                // Paladin Tier 6 Trinket (Ashtongue Talisman of Zeal)
                case 40470:
                {
                    if (!procSpell)
                        return false;

                    float  chance;

                    // Flash of light/Holy light
                    if (procSpell->SpellFamilyFlags & 0x00000000C0000000LL)
                    {
                        triggered_spell_id = 40471;
                        chance = 15.f;
                    }
                    // Judgement
                    else if (procSpell->SpellFamilyFlags & 0x0000000000800000LL)
                    {
                        triggered_spell_id = 40472;
                        chance = 50.f;
                    }
                    else
                        return false;

                    if (!roll_chance_f(chance))
                        return false;

                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            switch (dummySpell->Id)
            {
                // Totemic Power (The Earthshatterer set)
                case 28823:
                {
                    if (!pVictim)
                        return false;

                    // Set class defined buff
                    switch (pVictim->GetClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28824;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28825;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28826;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28827;     // Increases the friendly target's armor
                            break;
                        default:
                            return false;
                    }
                    break;
                }
                // Lesser Healing Wave (Totem of Flowing Water Relic)
                case 28849:
                {
                    target = this;
                    triggered_spell_id = 28850;
                    break;
                }
                // Windfury Weapon Rank 5 - NOT players version
                case 33727:
                {
                    if (GetTypeId() == TYPEID_PLAYER)
                        return false;

                    SpellEntry const* windfurySpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(33727);

                    int32 extra_attack_power = CalculateSpellDamage(windfurySpellEntry,0,windfurySpellEntry->EffectBasePoints[0]);

                    // Value gained from additional AP
                    basepoints0 = int32(extra_attack_power/14.0f * GetAttackTime(BASE_ATTACK)/1000);
                    triggered_spell_id = 25504;

                    // Attack Twice
                    for (uint32 i = 0; i<2; ++i)
                        CastCustomSpell(pVictim,triggered_spell_id,&basepoints0,NULL,NULL,true,NULL,triggeredByAura);

                    return true;
                }
                // Windfury Weapon (Passive) 1-5 Ranks
                case 33757:
                {
                    if (GetTypeId() != TYPEID_PLAYER || IsInFeralForm(true))
                        return false;

                    if (!castItem || !castItem->IsEquipped())
                        return false;

                    if (triggeredByAura && castItem->GetGUID() != triggeredByAura->GetCastItemGUID())
                        return false;

                    static const uint32 WF_RANK_1 = 33757;
                    // custom cooldown processing case
                    if (cooldown && ((Player*)this)->HasSpellCooldown(WF_RANK_1))
                        return false;

                    uint32 spellId;
                    switch (castItem->GetEnchantmentId(EnchantmentSlot(TEMP_ENCHANTMENT_SLOT)))
                    {
                        case 283: spellId = 33757; break;   //1 Rank
                        case 284: spellId = 33756; break;   //2 Rank
                        case 525: spellId = 33755; break;   //3 Rank
                        case 1669:spellId = 33754; break;   //4 Rank
                        case 2636:spellId = 33727; break;   //5 Rank
                        default:
                        {
                            sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleDummyAuraProc: non handled item enchantment (rank?) %u for spell id: %u (Windfury)",
                                castItem->GetEnchantmentId(EnchantmentSlot(TEMP_ENCHANTMENT_SLOT)),dummySpell->Id);
                            return false;
                        }
                    }

                    SpellEntry const* windfurySpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
                    if (!windfurySpellEntry)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleDummyAuraProc: non existed spell id: %u (Windfury)",spellId);
                        return false;
                    }

                    int32 extra_attack_power = CalculateSpellDamage(windfurySpellEntry,0,windfurySpellEntry->EffectBasePoints[0]);

                    // totem of the astral winds
                    if (HasAura(34244))
                        extra_attack_power += 80;

                    // Off-Hand case
                    if (castItem->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                    {
                        // Value gained from additional AP
                        basepoints0 = int32(extra_attack_power/14.0f * GetAttackTime(OFF_ATTACK)/1000);
                        triggered_spell_id = 33750;
                    }
                    // Main-Hand case
                    else
                    {
                        // Value gained from additional AP
                        basepoints0 = int32(extra_attack_power/14.0f * GetAttackTime(BASE_ATTACK)/1000);
                        triggered_spell_id = 25504;
                    }

                    // apply cooldown before cast to prevent processing itself
                    if (cooldown)
                        ((Player*)this)->AddSpellCooldown(WF_RANK_1,time(NULL) + cooldown);

                    // Attack Twice
                    for (uint32 i = 0; i<2; ++i)                                                   // if we set castitem it will force our m_cantrigger = true to false for windfury weapon due to later checks in prepareDataForTriggerSystem()
                        CastCustomSpell(pVictim,triggered_spell_id,&basepoints0,NULL,NULL,true,NULL/*castItem*/,triggeredByAura);

                    return true;
                }
                // Shaman Tier 6 Trinket
                case 40463:
                {
                    if (!procSpell)
                        return false;

                    float  chance;
                    if (procSpell->SpellFamilyFlags & 0x0000000000000001LL)
                    {
                        triggered_spell_id = 40465;         // Lightning Bolt
                        chance = 15.f;
                    }
                    else if (procSpell->SpellFamilyFlags & 0x0000000000000080LL)
                    {
                        triggered_spell_id = 40465;         // Lesser Healing Wave
                        chance = 10.f;
                    }
                    else if (procSpell->SpellFamilyFlags & 0x0000001000000000LL)
                    {
                        triggered_spell_id = 40466;         // Stormstrike
                        chance = 50.f;
                    }
                    else
                        return false;

                    if (!roll_chance_f(chance))
                        return false;

                    target = this;
                    break;
                }
            }

            // Earth Shield
            if (dummySpell->SpellFamilyFlags==0x40000000000LL)
            {
                basepoints0 = triggeredByAura->GetModifier()->m_amount;
                triggered_spell_id = 379;

                // Adding cooldown to earth shield caster, so earth shield cast on creature still will have cooldown
                if (Unit *caster = triggeredByAura->GetCaster())
                    if (caster->GetTypeId() == TYPEID_PLAYER && cooldown)
                    {
                        if (((Player*)caster)->HasSpellCooldown(triggered_spell_id))
                            return false;
                        ((Player*)caster)->AddSpellCooldown(triggered_spell_id,time(NULL) + cooldown);
                    }

                CastCustomSpell(this, triggered_spell_id, &basepoints0, NULL, NULL, true);
                return true;
            }
            // Lightning Overload
            if (dummySpell->SpellIconID == 2018)            // only this spell have SpellFamily Shaman SpellIconID == 2018 and dummy aura
            {
                if (!procSpell || GetTypeId() != TYPEID_PLAYER || !pVictim)
                    return false;

                // custom cooldown processing case
                if (cooldown && GetTypeId()==TYPEID_PLAYER && ((Player*)this)->HasSpellCooldown(dummySpell->Id))
                    return false;

                uint32 spellId = 0;
                // Every Lightning Bolt and Chain Lightning spell have duplicate vs half damage and zero cost
                switch (procSpell->Id)
                {
                    // Lightning Bolt
                    case   403: spellId = 45284; break;     // Rank  1
                    case   529: spellId = 45286; break;     // Rank  2
                    case   548: spellId = 45287; break;     // Rank  3
                    case   915: spellId = 45288; break;     // Rank  4
                    case   943: spellId = 45289; break;     // Rank  5
                    case  6041: spellId = 45290; break;     // Rank  6
                    case 10391: spellId = 45291; break;     // Rank  7
                    case 10392: spellId = 45292; break;     // Rank  8
                    case 15207: spellId = 45293; break;     // Rank  9
                    case 15208: spellId = 45294; break;     // Rank 10
                    case 25448: spellId = 45295; break;     // Rank 11
                    case 25449: spellId = 45296; break;     // Rank 12
                    // Chain Lightning
                    case   421: spellId = 45297; break;     // Rank  1
                    case   930: spellId = 45298; break;     // Rank  2
                    case  2860: spellId = 45299; break;     // Rank  3
                    case 10605: spellId = 45300; break;     // Rank  4
                    case 25439: spellId = 45301; break;     // Rank  5
                    case 25442: spellId = 45302; break;     // Rank  6
                        // self procs LO effect can proc another LO
                    case 45284: case 45286: case 45287: case 45288: case 45289: case 45290:
                    case 45291: case 45292: case 45293: case 45294: case 45295: case 45296:
                    case 45297: case 45298: case 45299: case 45300: case 45301: case 45302:
                        spellId = procSpell->Id; break;
                    default:
                        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleDummyAuraProc: non handled spell id: %u (LO)", procSpell->Id);
                        return false;
                }

                // Remove cooldown (Chain Lightning - have Category Recovery time)
                if (procSpell->SpellFamilyFlags & 0x0000000000000002LL)
                    ((Player*)this)->RemoveSpellCooldown(spellId);

                CastSpell(pVictim, spellId, true, castItem, ( spellId == procSpell->Id ? triggeredByAura : NULL));
                // LB can trigger LO, LO can trigger second LO, but second LO should be not allowed to trigger anything

                if (cooldown && GetTypeId()==TYPEID_PLAYER)
                    ((Player*)this)->AddSpellCooldown(dummySpell->Id,time(NULL) + cooldown);

                return true;
            }
            break;
        }
        case SPELLFAMILY_POTION:
        {
            if (dummySpell->Id == 17619)
            {
                basepoints0 = damage * 4 / 10;
                triggered_spell_id = 21399;
            }
        }
        default:
            break;
    }

    // processed charge only counting case
    if (!triggered_spell_id)
        return true;

    SpellEntry const* triggerEntry = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!triggerEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleDummyAuraProc: Spell %u have not existed triggered spell %u",dummySpell->Id,triggered_spell_id);
        return false;
    }

    // default case
    if (!target || target!=this && !target->isAlive())
        return false;

    if (cooldown && GetTypeId()==TYPEID_PLAYER && ((Player*)this)->HasSpellCooldown(triggered_spell_id))
        return false;

    if (basepoints0)
        CastCustomSpell(target, triggered_spell_id, &basepoints0, NULL, NULL, true, castItem, triggeredByAura, originalCaster);
    else
        CastSpell(target, triggered_spell_id, true, castItem, triggeredByAura, originalCaster);

    if (cooldown && GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->AddSpellCooldown(triggered_spell_id,time(NULL) + cooldown);

    return true;
}

bool Unit::HandleProcTriggerSpell(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellEntry const *procSpell, uint32 procFlags, uint32 procEx, uint32 cooldown)
{
    // Get triggered aura spell info
    SpellEntry const* auraSpellEntry = triggeredByAura->GetSpellProto();

    // Basepoints of trigger aura
    int32 triggerAmount = triggeredByAura->GetModifier()->m_amount;

    // Set trigger spell id, target, custom basepoints
    uint32 trigger_spell_id = auraSpellEntry->EffectTriggerSpell[triggeredByAura->GetEffIndex()];
    Unit*  target = NULL;
    int32  basepoints0 = 0;

    if (triggeredByAura->GetModifier()->m_auraname == SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE)
        basepoints0 = triggerAmount;

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId()==TYPEID_PLAYER
        ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    if (castItem && !isAlive())
        return false;

    // Try handle unknown trigger spells
    if (sSpellTemplate.LookupEntry<SpellEntry>(trigger_spell_id) == NULL)
    switch (auraSpellEntry->SpellFamilyName)
    {
     //=====================================================================
     // Generic class
     // ====================================================================
     // .....
     //=====================================================================
     case SPELLFAMILY_GENERIC:
//     if (auraSpellEntry->Id==34082)      // Advantaged State (DND)
//          trigger_spell_id = ???;
     if (auraSpellEntry->Id == 23780)      // Aegis of Preservation (Aegis of Preservation trinket)
          trigger_spell_id = 23781;
//     else if (auraSpellEntry->Id==43504) // Alterac Valley OnKill Proc Aura
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==37030) // Chaotic Temperament
//          trigger_spell_id = ;
     else if (auraSpellEntry->Id==43820)   // Charm of the Witch Doctor (Amani Charm of the Witch Doctor trinket)
     {
          // Pct value stored in dummy
          basepoints0 = pVictim->GetCreateHealth() * auraSpellEntry->EffectBasePoints[1] / 100;
          target = pVictim;
          break;
     }
     else if (auraSpellEntry->Id==41248) // Consuming Strikes
     {
         basepoints0 = damage;
         target = pVictim;
         trigger_spell_id = 41249;
     }
//     else if (auraSpellEntry->Id==41054) // Copy Weapon
//          trigger_spell_id = 41055;
//     else if (auraSpellEntry->Id==31255) // Deadly Swiftness (Rank 1)
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==5301)  // Defensive State (DND)
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==13358) // Defensive State (DND)
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==16092) // Defensive State (DND)
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==24949) // Defensive State 2 (DND)
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==40329) // Demo Shout Sensor
//          trigger_spell_id = ;
     // Desperate Defense (Stonescythe Whelp, Stonescythe Alpha, Stonescythe Ambusher)
     else if (auraSpellEntry->Id == 33896)
         trigger_spell_id = 33898;
//     else if (auraSpellEntry->Id==18943) // Double Attack
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==19194) // Double Attack
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==19817) // Double Attack
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==19818) // Double Attack
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==22835) // Drunken Rage
//          trigger_spell_id = 14822;
 /*
     else if (auraSpellEntry->SpellIconID==191) // Elemental Response
     {
         switch (auraSpellEntry->Id && auraSpellEntry->AttributesEx==0)
         {
         case 34191:
         case 34329:
         case 34524:
         case 34582:
         case 36733:break;
         default:
             sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u miss posibly Elemental Response",auraSpellEntry->Id);
             return false;
         }
         //This generic aura self-triggers a different spell for each school of magic that lands on the wearer:
         switch (procSpell->School)
         {
             case SPELL_SCHOOL_FIRE:   trigger_spell_id = 34192;break;//Fire:     34192
             case SPELL_SCHOOL_FROST:  trigger_spell_id = 34193;break;//Frost:    34193
             case SPELL_SCHOOL_ARCANE: trigger_spell_id = 34194;break;//Arcane:   34194
             case SPELL_SCHOOL_NATURE: trigger_spell_id = 34195;break;//Nature:   34195
             case SPELL_SCHOOL_SHADOW: trigger_spell_id = 34196;break;//Shadow:   34196
             case SPELL_SCHOOL_HOLY:   trigger_spell_id = 34197;break;//Holy:     34197
             case SPELL_SCHOOL_NORMAL: trigger_spell_id = 34198;break;//Physical: 34198
             default:
                 sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u Elemental Response wrong school",auraSpellEntry->Id);
             return false;
         }
     }*/
//     else if (auraSpellEntry->Id==6542)  // Enraged Defense
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==40364) // Entangling Roots Sensor
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==33207) // Gossip NPC Periodic - Fidget
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==35321) // Gushing Wound
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==38363) // Gushing Wound
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==39215) // Gushing Wound
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==40250) // Improved Duration
//          trigger_spell_id = ;
     else if (auraSpellEntry->Id == 24905)   // Moonkin Form (Passive)
     {
         // Elune's Touch (instead non-existed triggered spell) 30% from AP
         trigger_spell_id = 33926;
         basepoints0 = GetTotalAttackPowerValue(BASE_ATTACK) * 30 / 100;
         target = this;
     }
     else if (auraSpellEntry->Id == 46939 || auraSpellEntry->Id == 27522)
     {
         // On successful melee or ranged attack gain $29471s1 mana and if possible drain $27526s1 mana from the target.
         if (pVictim && pVictim->GetPower(POWER_MANA))
             CastSpell(pVictim, 27526, true, castItem, triggeredByAura);
         else
             CastSpell(this, 29471, true, castItem, triggeredByAura);
         return true;
     }
//     else if (auraSpellEntry->Id==43453) // Rune Ward
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==7137)  // Shadow Charge (Rank 1)
//          trigger_spell_id = ;
       // Shaleskin (Shaleskin Flayer, Shaleskin Ripper) 30023 trigger
//     else if (auraSpellEntry->Id==36576)
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==34783) // Spell Reflection
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==36096) // Spell Reflection
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==36207) // Steal Weapon
//          trigger_spell_id = ;
//     else if (auraSpellEntry->Id==35205) // Vanish
     break;
     //=====================================================================
     // Mage
     //=====================================================================
     // Blazing Speed (Rank 1,2) trigger = 18350
     //=====================================================================
     case SPELLFAMILY_MAGE:
     // Blazing Speed
     if (auraSpellEntry->SpellIconID == 2127)
     {
         switch (auraSpellEntry->Id)
         {
             case 31641:  // Rank 1
             case 31642:  // Rank 2
                 trigger_spell_id = 31643;
             break;
             default:
                 sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u miss posibly Blazing Speed",auraSpellEntry->Id);
             return false;
         }
     }
     break;
     //=====================================================================
     // Warrior
     //=====================================================================
     // Rampage (Rank 1-3) trigger = 18350
     //=====================================================================
     case SPELLFAMILY_WARRIOR:
     // Rampage
     if (auraSpellEntry->SpellIconID == 2006 && auraSpellEntry->SpellFamilyFlags==0x100000)
     {
         switch (auraSpellEntry->Id)
         {
             case 29801: trigger_spell_id = 30029; break;       // Rank 1
             case 30030: trigger_spell_id = 30031; break;       // Rank 2
             case 30033: trigger_spell_id = 30032; break;       // Rank 3
             default:
                 sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u not handled in Rampage",auraSpellEntry->Id);
             return false;
         }
     }
     break;
     //=====================================================================
     // Warlock
     //=====================================================================
     // Pyroclasm             trigger = 18350
     // Drain Soul (Rank 1-5) trigger = 0
     //=====================================================================
     case SPELLFAMILY_WARLOCK:
     {
         // Pyroclasm MASS
         if (auraSpellEntry->SpellIconID == 1137)
         {
             if (!pVictim || !pVictim->isAlive() || pVictim == this || procSpell == NULL)
                 return false;
             // Calculate spell tick count for spells
             uint32 tick = 1; // Default tick = 1

             // Hellfire have 15 tick
             if (procSpell->SpellFamilyFlags&0x0000000000000040LL)
                 tick = 15;
             // Rain of Fire have 4 tick
             else if (procSpell->SpellFamilyFlags&0x0000000000000020LL)
                 tick = 4;
             else
                 return false;

             // Calculate chance = baseChance / tick
             float chance = 0;
             switch (auraSpellEntry->Id)
             {
                 case 18096: chance = 13.0f / tick; break;
                 case 18073: chance = 26.0f / tick; break;
             }
             // Roll chance
             if (!roll_chance_f(chance))
                 return false;

             trigger_spell_id = 18093;
         }
         // Pyroclasm DIRECT
         else if (auraSpellEntry->SpellIconID == 1139)
            {
                if (!pVictim || !pVictim->isAlive() || pVictim == this || procSpell == NULL)
                    return false;

                // Calculate chance = baseChance / tick
                float chance = 0;
                switch (auraSpellEntry->Id)
                {
                    case 54641: chance = 13.0f; break;
                    case 54642: chance = 26.0f; break;
                }
                // Roll chance
                if (!roll_chance_f(chance))
                    return false;

                trigger_spell_id = 18093;
            }
            // Aftermath MASS
         else if (auraSpellEntry->SpellIconID == 1649)
            {
                if (!pVictim || !pVictim->isAlive() || pVictim == this || procSpell == NULL)
                    return false;
                // Calculate spell tick count for spells
                uint32 tick = 1; // Default tick = 1

                // Hellfire have 15 tick
                if (procSpell->SpellFamilyFlags&0x0000000000000040LL)
                    tick = 15;
                // Rain of Fire have 4 tick
                else if (procSpell->SpellFamilyFlags&0x0000000000000020LL)
                    tick = 4;
                else
                    return false;

                // Calculate chance = baseChance / tick
                float chance = 0;
                switch (auraSpellEntry->Id)
                {
                    case 54643: chance = 2.0f / tick; break;
                    case 54644: chance = 4.0f / tick; break;
                    case 54645: chance = 6.0f / tick; break;
                    case 54646: chance = 8.0f / tick; break;
                    case 54647: chance = 10.0f / tick; break;
                }
                // Roll chance
                if (!roll_chance_f(chance))
                    return false;

                trigger_spell_id = 18118;
            }
            // Aftermath DIRECT
            else if (auraSpellEntry->SpellIconID == 11)
            {
                if (!pVictim || !pVictim->isAlive() || pVictim == this || procSpell == NULL)
                    return false;

                // Calculate chance = baseChance / tick
                float chance = 0;
                switch (auraSpellEntry->Id)
                {
                    case 18119: chance = 2.0f; break;
                    case 18120: chance = 4.0f; break;
                    case 18121: chance = 6.0f; break;
                    case 18122: chance = 8.0f; break;
                    case 18123: chance = 10.0f; break;
                }
                // Roll chance
                if (!roll_chance_f(chance))
                    return false;

                trigger_spell_id = 18118;
            }
         // Drain Soul
         else if (auraSpellEntry->SpellFamilyFlags & 0x0000000000004000LL)
         {
             Unit::AuraList const& mAddFlatModifier = GetAurasByType(SPELL_AURA_ADD_FLAT_MODIFIER);
             for (Unit::AuraList::const_iterator i = mAddFlatModifier.begin(); i != mAddFlatModifier.end(); ++i)
             {
                 if ((*i)->GetModifier()->m_miscvalue == SPELLMOD_CHANCE_OF_SUCCESS && (*i)->GetSpellProto()->SpellIconID == 113)
                 {
                     int32 value2 = CalculateSpellDamage((*i)->GetSpellProto(),2,(*i)->GetSpellProto()->EffectBasePoints[2]);
                     basepoints0 = value2 * GetMaxPower(POWER_MANA) / 100;
                 }
             }
             if (basepoints0 == 0)
                 return false;
             trigger_spell_id = 18371;
         }
         break;
     }
     //=====================================================================
     // Priest
     //=====================================================================
     // Greater Heal Refund         trigger = 18350
     // Blessed Recovery (Rank 1-3) trigger = 18350
     // Shadowguard (1-7)           trigger = 28376
     //=====================================================================
     case SPELLFAMILY_PRIEST:
     {
         // Greater Heal Refund
         if (auraSpellEntry->Id==37594)
             trigger_spell_id = 37595;
         // Shadowguard
         else if (auraSpellEntry->SpellFamilyFlags==0x100080000000LL && auraSpellEntry->SpellVisual==7958)
         {
             switch (auraSpellEntry->Id)
             {
                 case 18137: trigger_spell_id = 28377; break;   // Rank 1
                 case 19308: trigger_spell_id = 28378; break;   // Rank 2
                 case 19309: trigger_spell_id = 28379; break;   // Rank 3
                 case 19310: trigger_spell_id = 28380; break;   // Rank 4
                 case 19311: trigger_spell_id = 28381; break;   // Rank 5
                 case 19312: trigger_spell_id = 28382; break;   // Rank 6
                 case 25477: trigger_spell_id = 28385; break;   // Rank 7
                 default:
                     sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u not handled in SG", auraSpellEntry->Id);
                 return false;
             }
         }
         // Blessed Recovery
         else if (auraSpellEntry->SpellIconID == 1875)
         {
             switch (auraSpellEntry->Id)
             {
                 case 27811: trigger_spell_id = 27813; break;
                 case 27815: trigger_spell_id = 27817; break;
                 case 27816: trigger_spell_id = 27818; break;
                 default:
                     sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u not handled in BR", auraSpellEntry->Id);
                 return false;
             }
             basepoints0 = damage * triggerAmount / 100 / 3;
             target = this;
         }
         break;
     }
     //=====================================================================
     // Druid
     // ====================================================================
     // Druid Forms Trinket  trigger = 18350
     // Entangling Roots     trigger = 30023
     // Leader of the Pack   trigger = 18350
     //=====================================================================
     case SPELLFAMILY_DRUID:
     {
         // Druid Forms Trinket
         if (auraSpellEntry->Id==37336)
         {
             switch (m_form)
             {
                 case 0:              trigger_spell_id = 37344;break;
                 case FORM_CAT:       trigger_spell_id = 37341;break;
                 case FORM_BEAR:
                 case FORM_DIREBEAR:  trigger_spell_id = 37340;break;
                 case FORM_TREE:      trigger_spell_id = 37342;break;
                 case FORM_MOONKIN:   trigger_spell_id = 37343;break;
                 default:
                     return false;
             }
         }
//         else if (auraSpellEntry->Id==40363)// Entangling Roots ()
//             trigger_spell_id = ????;
         // Leader of the Pack
         else if (auraSpellEntry->Id == 24932)
         {
             if (triggerAmount == 0)
                 return false;
             basepoints0 = triggerAmount * GetMaxHealth() / 100;
             trigger_spell_id = 34299;
         }
         break;
     }
     //=====================================================================
     // Hunter
     // ====================================================================
     // ......
     //=====================================================================
     //case SPELLFAMILY_HUNTER:
     //    switch (auraSpellEntry->Id)
     //    {
     //    }
     //break;
     //=====================================================================
     // Paladin
     // ====================================================================
     // Blessed Life                   trigger = 31934
     // Healing Discount               trigger = 18350
     // Illumination (Rank 1-5)        trigger = 18350
     // Judgement of Light (Rank 1-5)  trigger = 5373
     // Judgement of Wisdom (Rank 1-4) trigger = 1826
     // Lightning Capacitor            trigger = 18350
     //=====================================================================
     case SPELLFAMILY_PALADIN:
     {
         // Healing Discount
         if (auraSpellEntry->Id==37705)
         {
             trigger_spell_id = 37706;
             target = this;
         }
         // Judgement of Light and Judgement of Wisdom
         else if (auraSpellEntry->SpellFamilyFlags & 0x0000000000080000LL)
         {
             switch (auraSpellEntry->Id)
             {
                 // Judgement of Light
                 case 20185: trigger_spell_id = 20267;break; // Rank 1
                 case 20344: trigger_spell_id = 20341;break; // Rank 2
                 case 20345: trigger_spell_id = 20342;break; // Rank 3
                 case 20346: trigger_spell_id = 20343;break; // Rank 4
                 case 27162: trigger_spell_id = 27163;break; // Rank 5
                 // Judgement of Wisdom
                 case 20186: trigger_spell_id = 20268;break; // Rank 1
                 case 20354: trigger_spell_id = 20352;break; // Rank 2
                 case 20355: trigger_spell_id = 20353;break; // Rank 3
                 case 27164: trigger_spell_id = 27165;break; // Rank 4
                 default:
                     sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u miss posibly Judgement of Light/Wisdom", auraSpellEntry->Id);
                 return false;
             }

             if (pVictim->GetTypeId() == TYPEID_PLAYER)
             {
                 if (((Player*)pVictim)->HasSpellCooldown(trigger_spell_id))
                     return false;

                 ((Player*)pVictim)->AddSpellCooldown(trigger_spell_id, time(NULL) +1);
             }

             // Improved Judgement of Light: bonus heal from t4 set
             if (Unit *caster = triggeredByAura->GetCaster())
             {
                 if (auraSpellEntry->SpellIconID == 299)
                 {
                     if (Aura *aur = caster->GetAura(37182, 0))
                     {
                         int bp = aur->GetModifierValue();
                         pVictim->CastCustomSpell(pVictim, trigger_spell_id, &bp, NULL, NULL, true, castItem, triggeredByAura);
                     }
                 }
             }

             pVictim->CastSpell(pVictim, trigger_spell_id, true, castItem, triggeredByAura);
             return true;                        // no hidden cooldown
         }
         // Illumination
         else if (auraSpellEntry->SpellIconID==241)
         {
             if (!procSpell)
                 return false;
             // procspell is triggered spell but we need mana cost of original cast spell
             uint32 originalSpellId = procSpell->Id;
             // Holy Shock
             if (procSpell->SpellFamilyFlags & 0x1000000000000LL) // Holy Shock heal
             {
                 switch (procSpell->Id)
                 {
                     case 25914: originalSpellId = 20473; break;
                     case 25913: originalSpellId = 20929; break;
                     case 25903: originalSpellId = 20930; break;
                     case 27175: originalSpellId = 27174; break;
                     case 33074: originalSpellId = 33072; break;
                     default:
                         sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u not handled in HShock",procSpell->Id);
                     return false;
                 }
             }
             SpellEntry const *originalSpell = sSpellTemplate.LookupEntry<SpellEntry>(originalSpellId);
             if (!originalSpell)
             {
                 sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u unknown but selected as original in Illu",originalSpellId);
                 return false;
             }
             // percent stored in effect 1 (class scripts) base points
             basepoints0 = originalSpell->manaCost * (auraSpellEntry->EffectBasePoints[1] + 1) / 100;
             trigger_spell_id = 20272;
             target = this;
         }
         // Lightning Capacitor
         else if (auraSpellEntry->Id==37657)
         {
             if (!pVictim || !pVictim->isAlive())
                 return false;

             trigger_spell_id = 37661;

             if (ToPlayer()->HasSpellCooldown(trigger_spell_id))
                 return false;

             // stacking
             CastSpell(this, 37658, true, NULL, triggeredByAura);
             // counting
             Aura * dummy = GetDummyAura(37658);
             if (!dummy)
                 return false;
             // release at 3 aura in stack (cont contain in basepoint of trigger aura)
             if (dummy->GetStackAmount() <= 2)
                 return false;

             RemoveAurasDueToSpell(37658);
             target = pVictim;

         }
         break;
     }
     //=====================================================================
     // Shaman
     //====================================================================
     // Lightning Shield             trigger = 18350
     // Mana Surge                   trigger = 18350
     // Nature's Guardian (Rank 1-5) trigger = 18350
     //=====================================================================
     case SPELLFAMILY_SHAMAN:
     {
         //Lightning Shield (overwrite non existing triggered spell call in spell.dbc
         if (auraSpellEntry->SpellFamilyFlags==0x00000400 && auraSpellEntry->SpellVisual==37)
         {
             switch (auraSpellEntry->Id)
             {
                 case   324: trigger_spell_id = 26364; break;  // Rank 1
                 case   325: trigger_spell_id = 26365; break;  // Rank 2
                 case   905: trigger_spell_id = 26366; break;  // Rank 3
                 case   945: trigger_spell_id = 26367; break;  // Rank 4
                 case  8134: trigger_spell_id = 26369; break;  // Rank 5
                 case 10431: trigger_spell_id = 26370; break;  // Rank 6
                 case 10432: trigger_spell_id = 26363; break;  // Rank 7
                 case 25469: trigger_spell_id = 26371; break;  // Rank 8
                 case 25472: trigger_spell_id = 26372; break;  // Rank 9
                 default:
                     sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleProcTriggerSpell: Spell %u not handled in LShield", auraSpellEntry->Id);
                 return false;
             }
         }
         // Lightning Shield (The Ten Storms set)
         else if (auraSpellEntry->Id == 23551)
         {
             trigger_spell_id = 23552;
             target = pVictim;
         }
         // Damage from Lightning Shield (The Ten Storms set)
         else if (auraSpellEntry->Id == 23552)
             trigger_spell_id = 27635;
         // Mana Surge (The Earthfury set)
         else if (auraSpellEntry->Id == 23572)
         {
             if (!procSpell)
                 return false;
             basepoints0 = procSpell->manaCost * 35 / 100;
             trigger_spell_id = 23571;
             target = this;
         }
         else if (auraSpellEntry->SpellIconID == 2013) //Nature's Guardian
         {
             // Check health condition - should drop to less 30% (damage deal after this!)
             if (!(10*(int32(GetHealth() - damage)) < 3 * GetMaxHealth()))
                 return false;

             if (pVictim && pVictim->isAlive())
                 pVictim->getThreatManager().modifyThreatPercent(this,-10);

             basepoints0 = triggerAmount * GetMaxHealth() / 100;
             trigger_spell_id = 31616;
             target = this;
         }
         break;
     }
     // default
     default:
         break;
    }

    // All ok. Check current trigger spell
    SpellEntry const* triggerEntry = sSpellTemplate.LookupEntry<SpellEntry>(trigger_spell_id);
    if (triggerEntry == NULL)
    {
        // Not cast unknown spell
        sLog.outDebug("Unit::HandleProcTriggerSpell: Spell %u have 0 in EffectTriggered[%d], not handled custom case?",auraSpellEntry->Id,triggeredByAura->GetEffIndex());
        return false;
    }

    // check if triggering spell can stack with current target's auras (if not - don't proc)
    // don't check if
    // aura is passive (talent's aura)
    // trigger_spell_id's aura is already active (allow to refresh triggered auras)
    // trigger_spell_id's triggeredByAura is already active (for example shaman's shields)
    AuraMap::iterator i,next;
    uint32 aura_id = 0;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        ++next;

        if (!(*i).second)
            continue;

        aura_id = (*i).second->GetSpellProto()->Id;
        if (SpellMgr::IsPassiveSpell(aura_id) || aura_id == trigger_spell_id || aura_id == triggeredByAura->GetSpellProto()->Id)
            continue;

        if (SpellMgr::IsNoStackSpellDueToSpell(trigger_spell_id, (*i).second->GetSpellProto()->Id, ((*i).second->GetCasterGUID() == GetGUID()), true))
            return false;
    }

    // Custom requirements (not listed in procEx) Warning! damage dealing after this
    // Custom triggered spells
    switch (auraSpellEntry->Id)
    {
        // Persistent Shield (Scarab Brooch trinket)
        // This spell originally trigger 13567 - Dummy Trigger (vs dummy effect)
        case 26467:
        {
            basepoints0 = damage * 15 / 100;
            target = pVictim;
            trigger_spell_id = 26470;
            break;
        }
        // Cheat Death
        case 28845:
        {
            // When your health drops below 20% ....
            if (GetHealth() - damage > GetMaxHealth() / 5 || GetHealth() < GetMaxHealth() / 5)
                return false;
            break;
        }
        // Deadly Swiftness (Rank 1)
        case 31255:
        {
            // whenever you deal damage to a target who is below 20% health.
            if (pVictim->GetHealth() > pVictim->GetMaxHealth() / 5)
                return false;

            target = this;
            trigger_spell_id = 22588;
            break;
        }
        // Greater Heal Refund (Avatar Raiment set)
        case 37594:
        {
            // Not give if target alredy have full health
            if (pVictim->GetHealth() == pVictim->GetMaxHealth())
                return false;
            // If your Greater Heal brings the target to full health, you gain $37595s1 mana.
            if (pVictim->GetHealth() + damage < pVictim->GetMaxHealth())
                return false;
            break;
        }
        // Bonus Healing (Crystal Spire of Karabor mace)
        case 40971:
        {
            // If your target is below $s1% health
            if (pVictim->GetHealth() > pVictim->GetMaxHealth() * triggerAmount / 100)
                return false;
            break;
        }
        // Energy Storm (used by Zul'jin)
        case 43983:
        // Spell Bomb (used by Anzu)
        case 40303:
        {
            if (procSpell && procSpell->powerType == POWER_MANA && (procSpell->manaCost || procSpell->ManaCostPercentage))
                target = this;
            else
                return false;
            break;
        }
        // Evasive Maneuvers (Commendation of Kael`thas trinket)
        case 45057:
        {
            // reduce you below $s1% health
            if (GetHealth() - damage > GetMaxHealth() * triggerAmount / 100)
                return false;
            break;
        }
        // Pendant of the Violet Eye
        case 29601:
        {
            if(!procSpell)
                return false;
            // arcane missiles - trigger on misile, dont triger on cast
            if(procSpell->SpellFamilyName == SPELLFAMILY_MAGE && procSpell->SpellFamilyFlags == 0x200000)
                break;
            if (procSpell->SpellFamilyName == SPELLFAMILY_MAGE && procSpell->SpellFamilyFlags == 0x800)
                return false;
            if(procSpell->powerType != POWER_MANA)
                return false;
            if(!procSpell->manaCost && !procSpell->ManaCostPercentage && !procSpell->manaCostPerlevel)
                return false;
            break;
        }
        // Molten Shields
        case 30482:
        {
            if (procFlags & (PROC_FLAG_TAKEN_RANGED_SPELL_HIT | PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT))
            {
                if (!ToPlayer())
                    break;

                float chance = ToPlayer()->HasSpell(11094) ? 50.0f : ToPlayer()->HasSpell(13043) ? 100.0f : 0.0f;
                if (!chance || !roll_chance_f(chance))
                    return false;
            }
            else if (!(procFlags & (PROC_FLAG_TAKEN_MELEE_HIT | PROC_FLAG_TAKEN_MELEE_SPELL_HIT)))
                return false;
            break;
        }
        // Trinket - Hand of Justice
        case 15600:
        {
            float chance = 0;
            chance = 6.02-0.067*GetLevel();
            if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_ALWAYS_PROC || roll_chance_f(chance))
            {
                trigger_spell_id = 15601; // TRENTONE check for compability
                break;
            }
            else
                return false;
            break;
        }
        case 9160: // Sleep. Not above lvl 50
        {
            if (pVictim && pVictim->GetLevel() > 50)
                return false;
            break;
        }
    }

    // Custom basepoints/target for exist spell
    // dummy basepoints or other customs
    switch (trigger_spell_id)
    {
		case 34936: // All backlashes
        {
            if (GetDummyAura(58000))
                if (urand(0, 100) > 10) // Not yet decided, just to look as it will be
                    CastSpell(pVictim, 20000/*searing_pain_like_ability*/, true);
            break;
        }
        // Self-proc fixes
        // SPELL_AURA_PROC_TRIGGER_SPELL doesn't work on spell reflected (caster) target anyway, so just check by "pVictim == this"
        case 46579: // Deathfrost
		{
			// do we need this???
			// don't proc from physical spells like Rupture	
			if (procSpell->SchoolMask == SPELL_SCHOOL_MASK_NORMAL)
				return false;
		}
        case 45055: // Shadow Bolt triggered by Timbal's Focusing Crystal
        case 15269: // Blackout
        {
            if (!pVictim || !pVictim->isAlive() || pVictim == this)
                return false;
            break;
        }
        case 12536:     // Arcane Concentration
        {
            if (procSpell && procSpell->SpellFamilyFlags & 0x200000) // arcane missiles ticks shouldn't proc it
                return false;
            break;
        }
        case 6946:
        {
            trigger_spell_id = 41356;
            break;
        }
        case 7099:  // Curse of Mending
        case 39703: // Curse of Mending
        case 29494: // Temptation
        case 20233: // Improved Lay on Hands (cast on target)
        {
            target = pVictim;
            break;
        }
        // Combo points add triggers (need add combopoint only for main tatget, and after possible combopoints reset)
        case 15250: // Rogue Setup
        {
            if (!pVictim || pVictim != GetVictim())   // applied only for main target
                return false;
            break;                                   // continue normal case
        }
        // Finish moves that add combo
        case 14189: // Seal Fate (Netherblade set)
            // prevent proc Seal Fate on eviscerate/deadly throw ;P familyflags are messed for rogue o.O (exception for netherblade 4p bonus
            if (triggeredByAura->GetId() != 37168 && procSpell->AttributesEx & SPELL_ATTR_EX_REQ_COMBO_POINTS1)
                return false;
        case 14157: // Ruthlessness
        {
            // avoid double proc, and dont proc from deadly throw
            if (procSpell->Id == 26679 || !pVictim || pVictim == this)
                return false;
            break;
        }
        // Hunter: Expose Weakness
        case 34501:
        {
            basepoints0 = int32(GetStat(STAT_AGILITY) *0.25);
            int32 basepoints1 = int32(GetStat(STAT_AGILITY) *0.25);

            CastCustomSpell(pVictim,trigger_spell_id,&basepoints0,&basepoints1,NULL,true,castItem,triggeredByAura);
            return true;
        }
        // Shamanistic Rage triggered spell
        case 30824:
        {
            basepoints0 = int32(GetTotalAttackPowerValue(BASE_ATTACK) * triggerAmount / 100);
            trigger_spell_id = 30824;
            break;
        }
        //Gift of the Doomsayer
        case 36174:
        case 39011:
        {
            target = triggeredByAura ? triggeredByAura->GetCaster() : NULL;
            break;
        }
        case 41914:
        case 41917:
        {
            target = pVictim;

            if (!target || target->HasAura(41914, 0) || target->HasAura(41917, 0))
                return false;

            if (target->GetTypeId() == TYPEID_UNIT)
                return false;

            break;
        }
    }

    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(trigger_spell_id);
    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Unit:HandleProcTriggerSpell not found SpellEntry for spell_id: %u.", trigger_spell_id);
        return false;
    }

    // not allow proc extra attack spell at extra attack
    if (m_extraAttacks && spellInfo->HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS))
        return false;

    if (cooldown && GetTypeId()==TYPEID_PLAYER && ((Player*)this)->HasSpellCooldown(trigger_spell_id))
        return false;

    // only for windfury proc for a moment
    if(GetTypeId() == TYPEID_UNIT && ((Creature*)this)->HasSpellCooldown(trigger_spell_id))
        return false;

    // try detect target manually if not set
    if (target == NULL)
       target = !(procFlags & PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL) && SpellMgr::IsPositiveSpell(trigger_spell_id) ? this : pVictim;

    // default case
    if (!target || target!=this && !target->isAlive())
        return false;
    
    // apply spell cooldown before casting to prevent triggering spells with SPELL_EFFECT_ADD_EXTRA_ATTACKS if spell has hidden cooldown
    if (cooldown && GetTypeId()==TYPEID_PLAYER)
        ((Player*)this)->AddSpellCooldown(trigger_spell_id,time(NULL) + cooldown);

    if (basepoints0)
        CastCustomSpell(target,trigger_spell_id,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
    else
        CastSpell(target,trigger_spell_id,true,castItem,triggeredByAura);

    // workaround: 3 sec cooldown for NPCs windfury proc
    if(trigger_spell_id == 32910 && GetTypeId() == TYPEID_UNIT)
        ((Creature*)this)->_AddCreatureSpellCooldown(32910, time(NULL) + 3);

    return true;
}

bool Unit::HandleOverrideClassScriptAuraProc(Unit *pVictim, Aura *triggeredByAura, SpellEntry const *procSpell, uint32 cooldown)
{
    int32 scriptId = triggeredByAura->GetModifier()->m_miscvalue;

    if (!pVictim || !pVictim->isAlive())
        return false;

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId()==TYPEID_PLAYER
        ? ((Player*)this)->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    uint32 triggered_spell_id = 0;

    switch (scriptId)
    {
        case 836:                                           // Improved Blizzard (Rank 1)
        {
            if (!procSpell || procSpell->SpellVisual!=9487)
                return false;
            triggered_spell_id = 12484;
            break;
        }
        case 988:                                           // Improved Blizzard (Rank 2)
        {
            if (!procSpell || procSpell->SpellVisual!=9487)
                return false;
            triggered_spell_id = 12485;
            break;
        }
        case 989:                                           // Improved Blizzard (Rank 3)
        {
            if (!procSpell || procSpell->SpellVisual!=9487)
                return false;
            triggered_spell_id = 12486;
            break;
        }
        case 4086:                                          // Improved Mend Pet (Rank 1)
        case 4087:                                          // Improved Mend Pet (Rank 2)
        {
            int32 chance = triggeredByAura->GetSpellProto()->EffectBasePoints[triggeredByAura->GetEffIndex()];
            if (!roll_chance_i(chance))
                return false;

            triggered_spell_id = 24406;
            break;
        }
        case 4533:                                          // Dreamwalker Raiment 2 pieces bonus
        {
            // Chance 50%
            if (!roll_chance_i(50))
                return false;

            switch (pVictim->getPowerType())
            {
                case POWER_MANA:   triggered_spell_id = 28722; break;
                case POWER_RAGE:   triggered_spell_id = 28723; break;
                case POWER_ENERGY: triggered_spell_id = 28724; break;
                default:
                    return false;
            }
            break;
        }
        case 4537:                                          // Dreamwalker Raiment 6 pieces bonus
            triggered_spell_id = 28750;                     // Blessing of the Claw
            break;
        case 5497:                                          // Improved Mana Gems (Serpent-Coil Braid)
            triggered_spell_id = 37445;                     // Mana Surge
            break;
    }

    // not processed
    if (!triggered_spell_id)
        return false;

    // standard non-dummy case
    SpellEntry const* triggerEntry = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!triggerEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::HandleOverrideClassScriptAuraProc: Spell %u triggering for class script id %u",triggered_spell_id,scriptId);
        return false;
    }

    if (cooldown && GetTypeId()==TYPEID_PLAYER && ((Player*)this)->HasSpellCooldown(triggered_spell_id))
        return false;

    CastSpell(pVictim, triggered_spell_id, true, castItem, triggeredByAura);

    if (cooldown && GetTypeId()==TYPEID_PLAYER)
        ((Player*)this)->AddSpellCooldown(triggered_spell_id,time(NULL) + cooldown);

    return true;
}

void Unit::setPowerType(Powers new_powertype)
{
    SetByteValue(UNIT_FIELD_BYTES_0, 3, new_powertype);

    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);
    }
    else if (((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit *owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_POWER_TYPE);
        }
    }

    switch (new_powertype)
    {
        default:
        case POWER_MANA:
            break;
        case POWER_RAGE:
            SetMaxPower(POWER_RAGE,GetCreatePowers(POWER_RAGE));
            SetPower(  POWER_RAGE,0);
            break;
        case POWER_FOCUS:
            SetMaxPower(POWER_FOCUS,GetCreatePowers(POWER_FOCUS));
            SetPower(  POWER_FOCUS,GetCreatePowers(POWER_FOCUS));
            break;
        case POWER_ENERGY:
            SetMaxPower(POWER_ENERGY,GetCreatePowers(POWER_ENERGY));
            SetPower(  POWER_ENERGY,0);
            break;
        case POWER_HAPPINESS:
            SetMaxPower(POWER_HAPPINESS,GetCreatePowers(POWER_HAPPINESS));
            SetPower(POWER_HAPPINESS,GetCreatePowers(POWER_HAPPINESS));
            break;
    }
}

FactionTemplateEntry const* Unit::getFactionTemplateEntry() const
{
    FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(getFaction());
    if (!entry)
    {
        static uint64 guid = 0;                             // prevent repeating spam same faction problem

        if (GetGUID() != guid)
        {
            if (GetTypeId() == TYPEID_PLAYER)
                sLog.outLog(LOG_DEFAULT, "ERROR: Player %s have invalid faction (faction template id) #%u", ((Player*)this)->GetName(), getFaction());
            else
                sLog.outLog(LOG_DEFAULT, "ERROR: Creature (template id: %u) have invalid faction (faction template id) #%u", ((Creature*)this)->GetCreatureInfo()->Entry, getFaction());
            guid = GetGUID();
        }
    }
    return entry;
}

bool Unit::IsHostileTo(Unit const* unit) const
{
    // always non-hostile to self
    if (unit == this)
        return false;

    // always non-hostile to GM in GM mode
    if (unit->GetTypeId() == TYPEID_PLAYER && ((Player const*)unit)->isGameMaster())
        return false;

    // always hostile to enemy
	if (IsInCombat() && (GetVictim() == unit || unit->GetVictim() == this))
        return true;

    // if is neutral to all, so it won't be hostile ;P
    if (IsNeutralToAll() || unit->IsNeutralToAll())
        return false;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always hostile to owner's enemy
    if (testerOwner && (testerOwner->GetVictim() == unit || unit->GetVictim() == testerOwner))
        return true;

    // always hostile to enemy owner
    if (targetOwner && (GetVictim() == targetOwner || targetOwner->GetVictim() == this))
        return true;

    // always hostile to owner of owner's enemy
    if (testerOwner && targetOwner && (testerOwner->GetVictim() == targetOwner || targetOwner->GetVictim() == testerOwner))
        return true;

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // always non-hostile to target with common owner, or to owner/pet
    if (tester == target)
        return false;

    // special cases (Duel, etc)
    if (tester->GetTypeId() == TYPEID_PLAYER && target->GetTypeId() == TYPEID_PLAYER)
    {
        Player const* pTester = (Player const*)tester;
        Player const* pTarget = (Player const*)target;

        // Duel
        if (pTester->duel && pTester->duel->opponent == pTarget && pTester->duel->startTime != 0)
            return true;

        // Group
        if (pTester->GetGroup() && pTester->GetGroup() == pTarget->GetGroup())
            return false;

        // Sanctuary
        if (pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY) && pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY))
            return false;

        // PvP FFA state
        if (pTester->IsFFAPvP() && pTarget->IsFFAPvP())
            return true;

        if (pTester->InBattleGroundOrArena() && pTarget->InBattleGroundOrArena())
            return true; // it would already return false if they would've been in the same group. Same group = same BG team.

        //= PvP states
        // Green/Blue (can't attack)
        if (pTester->GetTeam() == pTarget->GetTeam())
            return false;

        // Red (can attack) if true, Blue/Yellow (can't attack) in another case
        return pTester->IsPvP() && pTarget->IsPvP();
    }

    // faction base cases
    FactionTemplateEntry const*tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateEntry const*target_faction = target->getFactionTemplateEntry();
    if (!tester_faction || !target_faction)
        return false;

    if (target->isAttackingPlayer() && tester->IsContestedGuard())
        return true;

    // PvC forced reaction and reputation case
    if (tester->GetTypeId() == TYPEID_PLAYER)
    {
        if (target_faction->faction)
        {
            // forced reactions
            if (ReputationRank const* force =((Player*)tester)->GetReputationMgr().GetForcedRankIfAny(target_faction))
                return *force <= REP_HOSTILE;

            // if faction have reputation then hostile state for tester at 100% dependent from at_war state
            if (FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction))
                if (FactionState const* factionState = ((Player*)tester)->GetReputationMgr().GetState(raw_target_faction))
                    return (factionState->Flags & FACTION_FLAG_AT_WAR);
        }
    }
    // CvP forced reaction and reputation case
    else if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (tester_faction->faction)
        {
            // forced reaction
            if(ReputationRank const* force = ((Player*)target)->GetReputationMgr().GetForcedRankIfAny(tester_faction))
                return *force <= REP_HOSTILE;

            // apply reputation state
            FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction);
            if (raw_tester_faction && raw_tester_faction->reputationListID >= 0)
                return ((Player const*)target)->GetReputationMgr().GetRank(raw_tester_faction) <= REP_HOSTILE;
        }
    }

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsHostileTo(*target_faction);
}

bool Unit::IsFriendlyTo(Unit const* unit) const
{
    // always friendly to self
    if (unit==this)
        return true;

    // always friendly to GM in GM mode
    if (unit->GetTypeId()==TYPEID_PLAYER && ((Player const*)unit)->isGameMaster())
        return true;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always non-friendly to owner's enemy
    if (testerOwner && (testerOwner->GetVictim()==unit || unit->GetVictim()==testerOwner))
        return false;

    // always non-friendly to enemy owner
    if (targetOwner && (GetVictim()==targetOwner || targetOwner->GetVictim()==this))
        return false;

    // always non-friendly to owner of owner's enemy
    if (testerOwner && targetOwner && (testerOwner->GetVictim()==targetOwner || targetOwner->GetVictim()==testerOwner))
        return false;

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // always friendly to target with common owner, or to owner/pet
    if (tester==target)
        return true;

    // special cases (Duel)
    if (tester->GetTypeId()==TYPEID_PLAYER && target->GetTypeId()==TYPEID_PLAYER)
    {
        Player const* pTester = (Player const*)tester;
        Player const* pTarget = (Player const*)target;

        // Duel
        if (pTester->duel && pTester->duel->opponent == target && pTester->duel->startTime != 0)
            return false;

        // Group
        if (pTester->GetGroup() && pTester->GetGroup()==pTarget->GetGroup())
            return true;

        // Sanctuary
        if (pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY) && pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY))
            return true;

        // PvP FFA state
        if (pTester->IsFFAPvP() && pTarget->IsFFAPvP())
            return false;

        if (pTester->InBattleGroundOrArena() && pTarget->InBattleGroundOrArena())
            return false; // it would already return true if they would've been in the same group. Same group = same BG team.

        //= PvP states
        // Green/Blue (non-attackable)
        if (pTester->GetTeam() == pTarget->GetTeam())
            return true;

        // Blue (friendly/non-attackable) if not PVP, or Yellow/Red in another case (attackable)
        return !pTarget->IsPvP();
    }

	// DANGEROUS! try to fix this case:
	// p1 attacks p2 with spell -> then invite to group -> can't heal
	// non-friendly to enemy
	if (GetVictim() == unit || unit->GetVictim() == this)
		return false;

    // faction base cases
    FactionTemplateEntry const*tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateEntry const*target_faction = target->getFactionTemplateEntry();
    if (!tester_faction || !target_faction)
        return false;

    if (target->isAttackingPlayer() && tester->IsContestedGuard())
        return false;

    // PvC forced reaction and reputation case
    if (tester->GetTypeId()==TYPEID_PLAYER)
    {
        if (tester_faction->faction)
        {
            // forced reaction
            if(ReputationRank const* force =((Player*)tester)->GetReputationMgr().GetForcedRankIfAny(target_faction))
                return *force >= REP_FRIENDLY;

            // if faction have reputation then friendly state for tester at 100% dependent from at_war state
            if (FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction))
                if (FactionState const* FactionState = ((Player*)tester)->GetReputationMgr().GetState(raw_target_faction))
                    return !(FactionState->Flags & FACTION_FLAG_AT_WAR);
        }
    }
    // CvP forced reaction and reputation case
    else if (target->GetTypeId()==TYPEID_PLAYER)
    {
        if (tester_faction->faction)
        {
            // forced reaction
            if(ReputationRank const* force =((Player*)target)->GetReputationMgr().GetForcedRankIfAny(tester_faction))
                return *force >= REP_FRIENDLY;

            // apply reputation state
            if (FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction))
                if (raw_tester_faction->reputationListID >=0)
                    return ((Player const*)target)->GetReputationMgr().GetRank(raw_tester_faction) >= REP_FRIENDLY;
        }
    }

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsFriendlyTo(*target_faction);
}

bool Unit::IsHostileToPlayers() const
{
    FactionTemplateEntry const* my_faction = getFactionTemplateEntry();
    if (!my_faction || !my_faction->faction)
        return false;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if (raw_faction && raw_faction->reputationListID >=0)
        return false;

    return my_faction->IsHostileToPlayers();
}

bool Unit::IsNeutralToAll() const
{
    if (GetCreatureType() == CREATURE_TYPE_CRITTER)
        return true;

    FactionTemplateEntry const* my_faction = getFactionTemplateEntry();
    if (!my_faction || !my_faction->faction)
        return true;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if (raw_faction && raw_faction->reputationListID >=0)
        return false;

    return my_faction->IsNeutralToAll();
}

bool Unit::Attack(Unit *victim, bool meleeAttack)
{
    if (!victim || victim == this)
        return false;

    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_TARGET)
        return false;

    // dead units can neither attack nor be attacked
    if (!isAlive() || !IsInWorld() || !victim->IsInWorld() || !victim->isAlive())
        return false;

    if (victim->HasUnitState(UNIT_STAT_IGNORE_ATTACKERS))
        return false;

    // do not attack players when controlling Vengeful Spirit (with Possess Spirit Immune aura)
    if (victim->GetTypeId() == TYPEID_PLAYER && ((Player*)victim)->HasAura(40282, 0))
        return false;

    // player cannot attack in mount state
    if (GetTypeId()==TYPEID_PLAYER && IsMounted())
        return false;

    // nobody can attack GM in GM-mode
    if (victim->GetTypeId()==TYPEID_PLAYER)
    {
        if (((Player*)victim)->isGameMaster())
            return false;
    }
    else
    {
        if (((Creature*)victim)->IsInEvadeMode())
            return false;
    }

    // remove SPELL_AURA_MOD_UNATTACKABLE at attack (in case non-interruptible spells stun aura applied also that not let attack)
    if (HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && !HasAura(40282) && !HasAura(17624))    // do not remove Possess Spirit Immune aura when attacking
        RemoveSpellsCausingAura(SPELL_AURA_MOD_UNATTACKABLE);                                 // same for Flask of petrification
                                                                                              // TODO/FIXME: do not remove any auras that cause any form of LOST_CONTROL
    if (m_attacking)
    {
        if (m_attacking == victim)
        {
            // switch to melee attack from ranged/magic
            if (meleeAttack && !HasUnitState(UNIT_STAT_MELEE_ATTACKING))
            {
                addUnitState(UNIT_STAT_MELEE_ATTACKING);
                SendMeleeAttackStart(victim->GetGUID());
                return true;
            }
            return false;
        }
        AttackStop();
    }

    //Set our target
    if (GetTypeId() != TYPEID_PLAYER) // should not change selection for players
        SetSelection(victim->GetGUID());
    if (Creature *me = ToCreature())
    {
        bool activeWhenInCombat  = false;
        if (GetMap()->Instanceable())
            activeWhenInCombat = sWorld.getConfig(CONFIG_COMBAT_ACTIVE_IN_INSTANCES);
        else
            activeWhenInCombat = sWorld.getConfig(CONFIG_COMBAT_ACTIVE_ON_CONTINENTS);

        if (activeWhenInCombat)
        {
            if (victim->ToPlayer() || !sWorld.getConfig(CONFIG_COMBAT_ACTIVE_FOR_PLAYERS_ONLY))
                setActive(true, ACTIVE_BY_COMBAT);
            else
                setActive(false, ACTIVE_BY_COMBAT);
        }
    }

    if (meleeAttack)
        addUnitState(UNIT_STAT_MELEE_ATTACKING);

	// prevents bugs with no-turn in combat
	if (GetTypeId() == TYPEID_UNIT && ToCreature()->HasActiveEmote())
		ToCreature()->ClearEmote();

    m_attacking = victim;
    m_attacking->_addAttacker(this);

    //if(m_attacking->GetTypeId()==TYPEID_UNIT && ((Creature*)m_attacking)->IsAIEnabled)
    //    ((Creature*)m_attacking)->AI()->AttackedBy(this);

    if (GetTypeId() == TYPEID_UNIT && !((Creature*)this)->isPet())
    {
        // should not let player enter combat by right clicking target
        SetInCombatWith(victim);
        if (victim->GetTypeId() == TYPEID_PLAYER)
        {
            //sLog.outDebug("SetInCombatWith attacker entry: %u, victim name: %s", this->GetEntry() ? this->GetEntry() : 0, victim->GetName() ? victim->GetName() : 0);
            victim->SetInCombatWith(this);
        }
        AddThreat(victim, 0.0f);

        WorldPacket data(SMSG_AI_REACTION, 12);
        data << uint64(GetGUID());
        data << uint32(AI_REACTION_AGGRO);                  // Aggro sound
        BroadcastPacket(&data, true);

        ((Creature*)this)->CallAssistance();
    }

    // delay offhand weapon attack to next attack time
    if (haveOffhandWeapon())
        resetAttackTimer(OFF_ATTACK);

    if (meleeAttack)
        SendMeleeAttackStart(victim->GetGUID());

	//if (IsUnit() && victim->IsPlayer() && 
	//	(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE)))
	//	sLog.outLog(LOG_NOTIFY, "Passive creature %s (entry %u) unit_flags %u attacking %s (guid %u)", GetName(), GetEntry(), GetUInt32Value(UNIT_FIELD_FLAGS), victim->GetName(), victim->GetGUID());

    return true;
}

bool Unit::AttackStop()
{
    if (!m_attacking)
        return false;

    m_attacking->_removeAttacker(this);

    //Clear our target, don't affect players - their target should not be cleared
    if (GetTypeId() != TYPEID_PLAYER)
        SetSelection(0);

    ClearUnitState(UNIT_STAT_MELEE_ATTACKING);

    InterruptSpell(CURRENT_MELEE_SPELL);

    if (GetTypeId()==TYPEID_UNIT)
    {
        // reset call assistance
        ((Creature*)this)->SetNoCallAssistance(false);
        ((Creature*)this)->SetNoSearchAssistance(false);
    }

	SendMeleeAttackStop(m_attacking);
	m_attacking = NULL;

    return true;
}

void Unit::CombatStop(bool cast)
{
    if (cast && IsNonMeleeSpellCast(false))
        InterruptNonMeleeSpells(false);

    AttackStop();
    RemoveAllAttackers();

    if (GetObjectGuid().IsPlayer())
        ToPlayer()->SendAttackSwingCancelAttack();     // melee and ranged forced attack cancel

    ClearInCombat();
}

void Unit::CombatStopExceptDoZoneInCombat()
{
    if (!GetInstanceData() || !GetInstanceData()->IsEncounterInProgress())
    {
        // CombatStop also removes attackers
        CombatStop();
        // ClearInCombat() is called automatically in Unit::Update when there are no hostileRef
        getHostileRefManager().deleteReferences();  // stop all fighting
    }
    else // with EncounterInProgress should only set threat to 0 for those who did threat with DoZoneInCombat()
    {
        bool leaveCombat = true;
        HostileReference *ref = getHostileRefManager().getFirst();
        while (ref)
        {
            Unit* target = ref->getSource()->getOwner();
            HostileReference* nextRef = ref->next();

            if (!target)
            {
                ref = nextRef;
                continue;
            }

            if (target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsDoZoneInCombatThreat())
            {
                ref->setThreat(0);
                leaveCombat = false;
            }
            else
            {
                ref->removeReference();
                delete ref;
                // CombatStop removes attackers, but we're not sure if we will leave combat!
                if (target->getVictimGUID() == GetGUID())
                    target->AttackStop();
            }
            ref = nextRef;
        }
        if (leaveCombat)
            CombatStop();
    }
}

void Unit::CombatStopWithPets(bool cast)
{
    CombatStop(cast);
    if (Pet* pet = GetPet())
        pet->CombatStop(cast);
    if (Unit* charm = GetCharm())
        charm->CombatStop(cast);
    if (GetTypeId()==TYPEID_PLAYER)
    {
        GuardianPetList const& guardians = ((Player*)this)->GetGuardians();
        for (GuardianPetList::const_iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
            if (Unit* guardian = Unit::GetUnit(*this,*itr))
                guardian->CombatStop(cast);
    }
}

bool Unit::isAttackingPlayer() const
{
    if (HasUnitState(UNIT_STAT_ATTACK_PLAYER))
        return true;

    Pet* pet = GetPet();
    if (pet && pet->isAttackingPlayer())
        return true;

    Unit* charmed = GetCharm();
    if (charmed && charmed->isAttackingPlayer())
        return true;

    for (int8 i = 0; i < MAX_TOTEM; i++)
    {
        if (m_TotemSlot[i])
        {
            Creature *totem = GetMap()->GetCreature(m_TotemSlot[i]);
            if (totem && totem->isAttackingPlayer())
                return true;
        }
    }

    return false;
}

void Unit::RemoveAllAttackers()
{
    while (!m_attackers.empty())
    {
        AttackerSet::iterator iter = m_attackers.begin();
        if (!(*iter)->AttackStop())
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: Unit has an attacker that isn't attacking it!");
            m_attackers.erase(iter);
        }
    }
}

void Unit::ModifyAuraState(AuraState flag, bool apply)
{
    if (apply)
    {
        if (!HasFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1)))
        {
            SetFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1));
            if (GetTypeId() == TYPEID_PLAYER)
            {
                const PlayerSpellMap& sp_list = ((Player*)this)->GetSpellMap();
                for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                {
                    if (itr->second.state == PLAYERSPELL_REMOVED)
                        continue;

                    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                    if (!spellInfo || !SpellMgr::IsPassiveSpell(itr->first))
                        continue;

                    if (spellInfo->CasterAuraState == flag)
                        CastSpell(this, itr->first, true, NULL);
                }
            }
        }
    }
    else
    {
        if (HasFlag(UNIT_FIELD_AURASTATE,1<<(flag-1)))
        {
            RemoveFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1));
            Unit::AuraMap& tAuras = GetAuras();
            for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
            {
                SpellEntry const* spellProto = (*itr).second ? (*itr).second->GetSpellProto() : NULL;
                if (spellProto && spellProto->CasterAuraState == flag)
                {
                    // exceptions (applied at state but not removed at state change)
                    // Rampage
                    if (spellProto->SpellIconID==2006 && spellProto->SpellFamilyName==SPELLFAMILY_WARRIOR && spellProto->SpellFamilyFlags==0x100000)
                    {
                        ++itr;
                        continue;
                    }

                    RemoveAura(itr);
                }
                else
                    ++itr;
            }
        }
    }
}

Unit *Unit::GetOwner() const
{
    uint64 ownerid = GetOwnerGUID();
    if (!ownerid)
        return NULL;
    return GetMap()->GetUnit(ownerid);
}

Unit *Unit::GetCharmer() const
{
    if (uint64 charmerid = GetCharmerGUID())
        return GetMap()->GetUnit(charmerid);
    return NULL;
}

Player* Unit::GetCharmerOrOwnerPlayerOrPlayerItself() const
{
    uint64 guid = GetCharmerOrOwnerGUID();
    if (IS_PLAYER_GUID(guid))
        return ObjectAccessor::GetPlayerInWorldOrNot(guid);

    return GetTypeId()==TYPEID_PLAYER ? (Player*)this : NULL;
}

Pet* Unit::GetPet() const
{
    if (uint64 pet_guid = GetPetGUID())
    {
        if (Pet* pet = ObjectAccessor::GetPet(pet_guid))
            return pet;

        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::GetPet: Pet %u not exist.",GUID_LOPART(pet_guid));
        const_cast<Unit*>(this)->SetPet(0);
    }

    return NULL;
}

Unit* Unit::GetCharm() const
{
    if (uint64 charm_guid = GetCharmGUID())
    {
        if (Unit* charm = Unit::GetUnit(*this, charm_guid))
            return charm;

        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::GetCharm: Charmed creature %u not exist.",GUID_LOPART(charm_guid));
        const_cast<Unit*>(this)->SetCharm(0);
    }

    return NULL;
}

Unit* Unit::GetEnslaved() const
{
    if (uint64 charm_guid = GetCharmGUID())
    {
        if (Unit* enslaved = Unit::GetUnit(*this, charm_guid))
            return enslaved->HasAuraByCasterWithFamilyFlags(this->GetGUID(), SPELLFAMILY_WARLOCK, 0x800LL) ? enslaved : NULL;

        sLog.outLog(LOG_DEFAULT, "ERROR: Unit::GetCharm: Charmed creature %u not exist.", GUID_LOPART(charm_guid));
        const_cast<Unit*>(this)->SetCharm(0);
    }

    return NULL;
}

float Unit::GetCombatDistance(const Unit* target) const
{
    float radius = target->GetFloatValue(UNIT_FIELD_COMBATREACH) + GetFloatValue(UNIT_FIELD_COMBATREACH);
    float dx = GetPositionX() - target->GetPositionX();
    float dy = GetPositionY() - target->GetPositionY();
    float dz = GetPositionZ() - target->GetPositionZ();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - radius;
    return ( dist > 0 ? dist : 0);
}

void Unit::SetPet(Pet* pet)
{
    SetUInt64Value(UNIT_FIELD_SUMMON, pet ? pet->GetGUID() : 0);
}

void Unit::SetCharm(Unit* pet)
{
    if (GetTypeId() == TYPEID_PLAYER)
        SetUInt64Value(UNIT_FIELD_CHARM, pet ? pet->GetGUID() : 0);
}

void Unit::RemoveBindSightAuras()
{
    RemoveSpellsCausingAura(SPELL_AURA_BIND_SIGHT);
}

void Unit::RemoveCharmAuras()
{
    RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM);
    RemoveSpellsCausingAura(SPELL_AURA_AOE_CHARM);
    RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS_PET);
    RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS);
}

void Unit::UnsummonAllTotems()
{
    for (int8 i = 0; i < MAX_TOTEM; ++i)
    {
        if (!m_TotemSlot[i])
            continue;

        Creature *OldTotem = GetMap()->GetCreature(m_TotemSlot[i]);
        if (OldTotem && OldTotem->isTotem())
            ((Totem*)OldTotem)->UnSummon();
    }
}

void Unit::SendHealSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical)
{
    // we guess size
    WorldPacket data(SMSG_SPELLHEALLOG, (8+8+4+4+1));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(Damage);
    data << uint8(critical ? 1 : 0);
    data << uint8(0);                                       // unused in client?
    BroadcastPacket(&data, true);
}

void Unit::SendEnergizeSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, Powers powertype)
{
    WorldPacket data(SMSG_SPELLENERGIZELOG, (8+8+4+4+4+1));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(powertype);
    data << uint32(Damage);
    BroadcastPacket(&data, true);
}

uint32 Unit::SpellDamageBonus(Unit *pVictim, SpellEntry const *spellProto, uint32 pdamage, DamageEffectType damagetype, CasterModifiers *casterModifiers)
{
    if (!spellProto || !pVictim || damagetype==DIRECT_DAMAGE)
        return pdamage;

    int32 BonusDamage = 0;
    if (GetTypeId()==TYPEID_UNIT)
    {
        // Pets just add their bonus damage to their spell damage
        // note that their spell damage is just gain of their own auras
        if (((Creature*)this)->isPet())
        {
            if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
                BonusDamage = ((Pet*)this)->getPetType() == HUNTER_PET ? ((Pet*)this)->GetBonusDamage()*0.33 : ((Pet*)this)->GetBonusDamage();
            else if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_MELEE && ((Pet*)this)->getPetType() == HUNTER_PET)
                BonusDamage = ((Pet*)this)->GetTotalAttackPowerValue(BASE_ATTACK)*0.07;
        }
        // For totems get damage bonus from owner (statue isn't totem in fact)
        else if (((Creature*)this)->isTotem() && ((Totem*)this)->GetTotemType()!=TOTEM_STATUE)
        {
            if (Unit* owner = GetOwner())
                return owner->SpellDamageBonus(pVictim, spellProto, pdamage, damagetype, casterModifiers);
        }
    }

    if (spellProto->AttributesCu & SPELL_ATTR_CU_FIXED_DAMAGE)
        return pdamage;

    // Damage Done
    uint32 CastingTime = !SpellMgr::IsChanneledSpell(spellProto) ? SpellMgr::GetSpellBaseCastTimeNotNegative(spellProto) : SpellMgr::GetSpellDuration(spellProto);

    uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();
    // Taken/Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit  = SpellBaseDamageBonus(SpellMgr::GetSpellSchoolMask(spellProto))+BonusDamage;
    DoneAdvertisedBenefit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS, creatureTypeMask);
    if (casterModifiers)
    {
        if (casterModifiers->Apply)
            DoneAdvertisedBenefit = casterModifiers->AdvertisedBenefit;
        else
            casterModifiers->AdvertisedBenefit = DoneAdvertisedBenefit;
    }
    int32 TakenAdvertisedBenefit = SpellBaseDamageBonusForVictim(SpellMgr::GetSpellSchoolMask(spellProto), pVictim);

    // Damage over Time spells bonus calculation
    float DotFactor = 1.0f;
    int DotTicks = 6;
    if (damagetype == DOT)
    {
        int32 DotDuration = SpellMgr::GetSpellDuration(spellProto);
        // 200% limit
        if (DotDuration > 0)
        {
            if (DotDuration > 30000) DotDuration = 30000;
            if (!SpellMgr::IsChanneledSpell(spellProto)) DotFactor = DotDuration / 15000.0f;
            int x = 0;
            for (int j = 0; j < 3; j++)
            {
                if (spellProto->Effect[j] == SPELL_EFFECT_APPLY_AURA && (
                    spellProto->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_DAMAGE ||
                    spellProto->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_LEECH))
                {
                    x = j;
                    break;
                }
            }
            if (spellProto->EffectAmplitude[x] != 0)
                DotTicks = DotDuration / spellProto->EffectAmplitude[x];
            if (DotTicks)
            {
                DoneAdvertisedBenefit /= DotTicks;
                TakenAdvertisedBenefit /= DotTicks;
            }
        }
    }

    // Taken/Done total percent damage auras
    float DoneTotalMod = 1.0f;
    float TakenTotalMod = 1.0f;

    // ..done
    AuraList const& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for (AuraList::const_iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
    {
        switch ((*i)->GetId())
        {
            case 6057:  // M Wand Spec 1
            case 6085:  // M Wand Spec 2
            case 14524: // P Wand Spec 1
            case 14525: // P Wand Spec 2
            case 14526: // P Wand Spec 3
            case 14527: // P Wand Spec 4
            case 14528: // P Wand Spec 5
                if (spellProto->Id != 5019) // Wand Shoot
                    continue;
            default: 
                break;
        }

        if (((*i)->GetModifier()->m_miscvalue & SpellMgr::GetSpellSchoolMask(spellProto)) &&
            (GetTypeId() != TYPEID_PLAYER || ((Player*)this)->HasItemFitToSpellReqirements((*i)->GetSpellProto())))
        {
            DoneTotalMod *= ((*i)->GetModifierValue() +100.0f)/100.0f;
        }
    }

    AuraList const& mDamageDoneVersus = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS);
    for (AuraList::const_iterator i = mDamageDoneVersus.begin();i != mDamageDoneVersus.end(); ++i)
        if (creatureTypeMask & uint32((*i)->GetModifier()->m_miscvalue))
            DoneTotalMod *= ((*i)->GetModifierValue() +100.0f)/100.0f;

    if (casterModifiers)
    {
        if (casterModifiers->Apply)
            DoneTotalMod = casterModifiers->DamagePercentDone;
        else
            casterModifiers->DamagePercentDone = DoneTotalMod;
    }

    // ..taken
    AuraList const& mModDamagePercentTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for (AuraList::const_iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i)
        if ((*i)->GetModifier()->m_miscvalue & SpellMgr::GetSpellSchoolMask(spellProto))
            TakenTotalMod *= ((*i)->GetModifierValue() +100.0f)/100.0f;

    // .. taken pct: scripted (increases damage of * against targets *)
    AuraList const& mOverrideClassScript = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        switch ((*i)->GetModifier()->m_miscvalue)
        {
            //Molten Fury
            case 4920: case 4919:
            {
                if (pVictim->HasAuraState(AURA_STATE_HEALTHLESS_20_PERCENT))
                    TakenTotalMod *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
                break;
            }
            case 6427: case 6428:                           // Dirty Deeds
            {
                if (pVictim->HasAuraState(AURA_STATE_HEALTHLESS_35_PERCENT) && 
                    spellProto->SpellFamilyName == SPELLFAMILY_ROGUE && spellProto->Attributes & SPELL_ATTR_ABILITY)
                {
                    Aura* eff0 = GetAura((*i)->GetId(), 0);
                    if (!eff0 || (*i)->GetEffIndex() != 1)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Spell structure of DD (%u) changed.", (*i)->GetId());
                        continue;
                    }

                    // effect 0 have expected value but in negative state
                    TakenTotalMod *= (-eff0->GetModifier()->m_amount + 100.0f) / 100.0f;
                }
                break;
            }
        }
    }

    bool hasmangle=false;
    // .. taken pct: dummy auras
    AuraList const& mDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
    for (AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
    {
        switch ((*i)->GetSpellProto()->SpellIconID)
        {
            //Mangle
            case 2312:
                // don't apply mod twice
                if (hasmangle)
                    break;
                hasmangle=true;
                for (int j=0;j<3;j++)
                {
                    if (SpellMgr::GetSpellMechanic(spellProto) == MECHANIC_BLEED || SpellMgr::GetEffectMechanic(spellProto, j) == MECHANIC_BLEED)
                    {
                        TakenTotalMod *= (100.0f+(*i)->GetModifier()->m_amount)/100.0f;
                        break;
                    }
                }
                break;
        }
    }

    // Distribute Damage over multiple effects, reduce by AoE
    CastingTime = GetCastingTimeForBonus(spellProto, damagetype, CastingTime);

    if (spellProto->HasApplyAura(SPELL_AURA_PERIODIC_LEECH) || spellProto->HasEffect(SPELL_EFFECT_HEALTH_LEECH))
        CastingTime /= 2;
    else if (spellProto->AttributesCu & SPELL_ATTR_CU_NO_SPELL_DMG_COEFF)
        CastingTime = 0;

    switch (spellProto->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            if (spellProto->Id == 40293)
            {
                CastingTime = 0;
            }
            else if (spellProto->SpellFamilyFlags & 0x100000000LL)
            {
                CastingTime = 0;
            }
            // Darkmoon Card: Vengeance - 0.1%
            else if (spellProto->SpellVisual == 9850 && spellProto->SpellIconID == 2230)
            {
                CastingTime = 3.5;
            }
            else if (spellProto->Id == 43427 || spellProto->Id == 46194 || spellProto->Id == 44176) // Ice Lance (Hex Lord Malacrass / Yazzai)
            {
                CastingTime /= 3;                            // applied 1/3 bonuses in case generic target
                if (pVictim->isFrozen())                     // and compensate this for frozen target.
                    TakenTotalMod *= 3.0f;
            }
            else if (spellProto->Id == 16608) // demon forged breastplate drain life 40%
            {
                CastingTime = 3500;
                DotFactor = 0.32f;
            }
            else if (spellProto->Id == 18817) // skullflame shield drain life 100%
            {
                CastingTime = 3500;
            }
            break;
        case SPELLFAMILY_MAGE:
            // Mana Tap(Racial)
            if (spellProto->Id == 28734)
            {
                return pdamage += GetLevel(); // new HG           
            }
            // Ice Lance
            else if ((spellProto->SpellFamilyFlags & 0x20000LL) && spellProto->SpellIconID == 186)
            {
                CastingTime /= 3;                           // applied 1/3 bonuses in case generic target
                if (pVictim->isFrozen())                     // and compensate this for frozen target.
                    TakenTotalMod *= 3.0f;
            }
            // Pyroblast - 115% of Fire Damage, DoT - 20% of Fire Damage
            else if ((spellProto->SpellFamilyFlags & 0x400000LL) && spellProto->SpellIconID == 184)
            {
                DotFactor = damagetype == DOT ? 0.2f : 1.0f;
                CastingTime = damagetype == DOT ? 3500 : 4025;
            }
            // Fireball - 100% of Fire Damage, DoT - 0% of Fire Damage
            else if ((spellProto->SpellFamilyFlags & 0x1LL) && spellProto->SpellIconID == 185)
            {
                DotFactor = damagetype == DOT ? 0.0f : 1.0f;
                CastingTime = damagetype == DOT ? 0 : 3500;
            }
            // Arcane Missiles triggered spell
            else if ((spellProto->SpellFamilyFlags & 0x200000LL) && spellProto->SpellIconID == 225)
            {
                CastingTime = 1000;
            }
            // Blizzard triggered spell
            else if ((spellProto->SpellFamilyFlags & 0x80080LL) && spellProto->SpellIconID == 285)
            {
                CastingTime = 333;
            }
            // dragon's breath AND Blast Wave 13.57%
            else if ((spellProto->SpellFamilyFlags & 0x800000LL) && (spellProto->SpellIconID == 1548 || spellProto->SpellIconID == 292))
            {
                CastingTime = 475;
            }
            // Cone of Cold 13.57%
            else if ((spellProto->SpellFamilyFlags & 0x180200LL) && spellProto->SpellIconID == 35)
            {
                CastingTime = 475;
            }
            // Frost Nova 13.57%
            else if ((spellProto->SpellFamilyFlags & 0x80040LL) && spellProto->SpellIconID == 193)
            {
                CastingTime = 475;
            }
            // Flamestrike 17.61% instant. 10.96% overall DoT
            else if ((spellProto->SpellFamilyFlags & 0x4LL) && spellProto->SpellIconID == 37)
            {
                DotFactor = damagetype == DOT ? 0.1644f : 1.0f;
                CastingTime = damagetype == DOT ? 3500 : 616;
            }
            break;
        case SPELLFAMILY_WARLOCK:
            // Life Tap
            if ((spellProto->SpellFamilyFlags & 0x40000LL) && spellProto->SpellIconID == 208)
            {
                CastingTime = 2800;                         // 80% from +shadow damage
                DoneTotalMod = 1.0f;
                TakenTotalMod = 1.0f;
            }
            // Dark Pact
            else if ((spellProto->SpellFamilyFlags & 0x80000000LL) && spellProto->SpellIconID == 154 && GetPetGUID())
            {
                CastingTime = 3360;                         // 96% from +shadow damage
                DoneTotalMod = 1.0f;
                TakenTotalMod = 1.0f;
            }
            // Soul Fire - 115% of Fire Damage
            else if ((spellProto->SpellFamilyFlags & 0x8000000000LL) && spellProto->SpellIconID == 184)
            {
                CastingTime = 4025;
            }
            // Curse of Agony - 120% of Shadow Damage
            else if ((spellProto->SpellFamilyFlags & 0x0000000400LL) && spellProto->SpellIconID == 544)
            {
                DotFactor = 1.2f;
            }
            // Drain Soul 214.3%
            else if ((spellProto->SpellFamilyFlags & 0x4000LL) && spellProto->SpellIconID == 113)
            {
                CastingTime = 7500;
            }
            // Hellfire
            else if ((spellProto->SpellFamilyFlags & 0x40LL) && spellProto->SpellIconID == 937)
            {
                CastingTime = damagetype == DOT ? 5000 : 500; // self damage seems to be so
            }
            // Unstable Affliction - 180%
            else if (spellProto->Id == 31117)
            {
                CastingTime = 5400;
            }
            // Corruption 93.6%
            else if ((spellProto->SpellFamilyFlags & 0x2LL) && spellProto->SpellIconID == 313)
            {
                DotFactor = 0.936f;
            }
            // Seed of Corruption - 150% overall. 25% per tick. 22% on explosion
            else if ((spellProto->SpellFamilyFlags & 0x1000000000LL) && spellProto->SpellIconID == 1932)
            {
                CastingTime = damagetype == DOT ? 3500 : 770;
                DotFactor = damagetype == DOT ? 1.5f : 1.0f;
            }
            // Death Coil 22%
            else if ((spellProto->SpellFamilyFlags & 0x80000LL) && spellProto->SpellIconID == 88)
            {
                CastingTime = 770;
            }
            // immolate DoT damage - 13% per tick. 65% overall. 20% on direct
            else if ((spellProto->SpellFamilyFlags & 0x4LL) && spellProto->SpellIconID == 31)
            {
                CastingTime = damagetype == DOT ? 2275 : 700;
            }    
            // shadowfury 19.5%
            else if ((spellProto->SpellFamilyFlags & 0x100000000000LL) && spellProto->SpellIconID == 1988)
            {
                CastingTime = 683;
            }
            // Rain of fire triggered spell - 23.8% per tick
            else if ((spellProto->SpellFamilyFlags & 0x20LL) && spellProto->SpellIconID == 547)
            {
                CastingTime = 833;
            }
            break;
        case SPELLFAMILY_PALADIN:
            // Consecration - 95% of Holy Damage
            if ((spellProto->SpellFamilyFlags & 0x20LL) && spellProto->SpellIconID == 51)
            {
                DotFactor = 0.95f;
                CastingTime = 3500;
            }
            // Seal of Righteousness - 10.8%/9.2% (based on weapon type) of Holy Damage, multiplied by weapon speed
            else if ((spellProto->SpellFamilyFlags & 0x8000000LL) && spellProto->SpellIconID == 25)
            {
                Item *item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                float wspeed = GetAttackTime(BASE_ATTACK)/1000.0f;

                if (item && item->GetProto()->InventoryType == INVTYPE_2HWEAPON)
                   CastingTime = uint32(wspeed*3500*0.108f);
                else
                   CastingTime = uint32(wspeed*3500*0.092f);
                if (HasAura(43743)) // improved SoR
                    DoneAdvertisedBenefit += 94;
            }
            // Judgement of Righteousness - 71.43%
            else if ((spellProto->SpellFamilyFlags & 1024) && spellProto->SpellIconID == 25)
            {
                CastingTime = 2500;
                if (HasAura(43743)) // improved SoR
                    DoneAdvertisedBenefit += 94;
            }
            // Seal of Vengeance - DOT: 17% per Application, DIRECT: 1.1%
            else if ((spellProto->SpellFamilyFlags & 0x80000000000LL) && spellProto->SpellIconID == 2292)
            {
                DotFactor = damagetype == DOT ? 0.17f : 0.011f;
                CastingTime = 3500;
            }
            else if (spellProto->Id == 42463)
            {
                CastingTime = 160; // Gonna need test - it's seal of vengeance and it's worked already
            }
            // Holy shield - 5% of Holy Damage
            else if ((spellProto->SpellFamilyFlags & 0x4000000000LL) && spellProto->SpellIconID == 453)
            {
                CastingTime = 175;
            }
            // Avenger's Shield - 13.57%
            else if ((spellProto->SpellFamilyFlags & 0x400000000000LL) && spellProto->SpellIconID == 2172)
            {
                CastingTime = 475;
            }
            break;
        case  SPELLFAMILY_SHAMAN:
            // 10% Flametongue
            if (spellProto->SpellFamilyFlags & 0x200000)
                CastingTime = 350;
            // totem attack
            else if (spellProto->SpellFamilyFlags & 0x000040000000LL)
            {
                if (spellProto->SpellIconID == 33)          // Fire Nova totem attack must be 21.4%(untested)
                    CastingTime = 749;                      // ignore CastingTime and use as modifier
                else if (spellProto->SpellIconID == 680)    // Searing Totem attack 16.67%
                    CastingTime = 584;                      // ignore CastingTime and use as modifier
                else if (spellProto->SpellIconID == 37)     // Magma totem attack must be 6.67%(untested)
                    CastingTime = 234;                      // ignore CastingTimePenalty and use as modifier
            }
            // Lightning Shield (and proc shield from T2 8 pieces bonus) 33.33% per charge
            else if ((spellProto->SpellFamilyFlags & 0x00000000400LL) || spellProto->Id == 23552)
            {
                CastingTime = 1166;                         // ignore CastingTimePenalty and use as modifier
            }
            // Lightning Overload Lightning Bolts should have 35.4%
            else if (spellProto->Id == 45284 || spellProto->Id <= 45296 && spellProto->Id >= 45286)
            {
                CastingTime = 1240;
            }
            // Lightning Overload Chain Lightning should have 30%
            else if (spellProto->Id <= 45302 && spellProto->Id >= 45297)
            {
                CastingTime = 1050;
            }
            // Frost Shock 42.865
            else if ((spellProto->SpellFamilyFlags & 0x80000000LL) && spellProto->SpellIconID == 976)
            {
                CastingTime = 1500;
            }    
            // Flametongue Weapon - 0% per attack. 10% per attack implemented in the flametongue weapon function
            else if ((spellProto->SpellFamilyFlags & 0x200000LL) && spellProto->SpellIconID == 679)
            {
                CastingTime = 0;
            }    
            // Frostbrand weapon - 10% per attack
            else if ((spellProto->SpellFamilyFlags & 0x1000000LL) && spellProto->SpellIconID == 681)
            {
                CastingTime = 350;
            }
            break;
        case SPELLFAMILY_PRIEST:
            // Mind Flay - 57.1% of Shadow Damage
            if ((spellProto->SpellFamilyFlags & 0x800000LL) && spellProto->SpellIconID == 548)
            {
                CastingTime = 2000;
            }
            // Holy Fire - 86.71%, DoT - 16.5%
            else if ((spellProto->SpellFamilyFlags & 0x100000LL) && spellProto->SpellIconID == 156)
            {
                DotFactor = damagetype == DOT ? 0.165f : 1.0f;
                CastingTime = damagetype == DOT ? 3500 : 3000;
            }
            // Shadowguard - 28% per charge
            else if ((spellProto->SpellFamilyFlags & 0x2000000LL) && spellProto->SpellIconID == 19)
            {
                CastingTime = 980;
            }
            // Touch of Weakeness - 10%
            else if ((spellProto->SpellFamilyFlags & 0x80000LL) && spellProto->SpellIconID == 1591)
            {
                CastingTime = 350;
            }
            // Holy Nova - 16%
            else if ((spellProto->SpellFamilyFlags & 0x400000LL) && spellProto->SpellIconID == 1874)
            {
                CastingTime = 560;
            }
            // Starshards - 83.5$ overall
            else if ((spellProto->SpellFamilyFlags & 0x200000LL) && spellProto->SpellIconID == 1485)
            {
                DotFactor = 0.835;
            }
            // Chastise 14.3%
            else if ((spellProto->SpellFamilyFlags & 0x1000000000LL) && spellProto->SpellIconID == 2542)
            {
                CastingTime = 500;
            }
            // Shadow Word: Pain - 110% overall. 18.3% tick
            else if ((spellProto->SpellFamilyFlags & 0x8000LL) && spellProto->SpellIconID == 234)
            {
                DotFactor = 1.1f;
            }
            // Vampiric Touch 100% overall. 20% tick
            else if ((spellProto->SpellFamilyFlags & 0x40000000000LL) && spellProto->SpellIconID == 2213)
            {
                DotFactor = 1.0526f;
            }
            break;
        case SPELLFAMILY_DRUID:
            // Hurricane triggered spell
            if ((spellProto->SpellFamilyFlags & 0x400000LL) && spellProto->SpellIconID == 220)
            {
                CastingTime = 450;
            }
            // Entangling Roots 90% - 10% per tick
            else if ((spellProto->SpellFamilyFlags & 0x200LL) && spellProto->SpellIconID == 20)
            {
                DotFactor = 0.947f;
            }
            break;
        default:
            break;
    }

    if (spellProto->AttributesEx3 & SPELL_ATTR_EX3_NO_DONE_BONUS)
        DoneTotalMod = 1.0f;

    float LvlPenalty = CalculateLevelPenalty(spellProto);

    // Spellmod SpellDamage
    //float SpellModSpellDamage = 100.0f;
    float CoefficientPtc = DotFactor * 100.0f;
    if (spellProto->SchoolMask != SPELL_SCHOOL_MASK_NORMAL)
        CoefficientPtc *= ((float)CastingTime/3500.0f);

    if (Player* modOwner = GetSpellModOwner())
    {
        float oldCoeff = CoefficientPtc;
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_SPELL_BONUS_DAMAGE, CoefficientPtc);
        // DO IT IN BETTER WAY (read: rewrite auras system)
        if (damagetype == DOT && DotFactor != 0)
            CoefficientPtc += (CoefficientPtc-oldCoeff)*(DotTicks-1);
    }

    //SpellModSpellDamage /= 100.0f;
    CoefficientPtc /= 100.0f;

    if (casterModifiers)
    {
        if (casterModifiers->Apply)
            CoefficientPtc = casterModifiers->CoefficientPtc;
        else
            casterModifiers->CoefficientPtc = CoefficientPtc;
    }

    //float DoneActualBenefit = DoneAdvertisedBenefit * (CastingTime / 3500.0f) * DotFactor * SpellModSpellDamage * LvlPenalty;

    float DoneActualBenefit = DoneAdvertisedBenefit * CoefficientPtc * LvlPenalty;
    float TakenActualBenefit = TakenAdvertisedBenefit * DotFactor * LvlPenalty;
    if (spellProto->SpellFamilyName && spellProto->SchoolMask != SPELL_SCHOOL_MASK_NORMAL)
        TakenActualBenefit *= ((float)CastingTime / 3500.0f);

    // HACK for Felmyst's Noxious Fumes dmg calculation when under Berserk effect
    if (spellProto->Id == 47002)
        DoneTotalMod = 1.0;

    float tmpDamage = (float(pdamage)+DoneActualBenefit)*DoneTotalMod;

    // apply spellmod to Done damage
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, damagetype == DOT ? SPELLMOD_DOT : SPELLMOD_DAMAGE, tmpDamage);

    tmpDamage = (tmpDamage+TakenActualBenefit)*TakenTotalMod;

    if (GetObjectGuid().IsCreature())
        tmpDamage *= ((Creature*)this)->GetCreatureDamageMod();

    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

int32 Unit::SpellBaseDamageBonus(SpellSchoolMask schoolMask)
{
    int32 DoneAdvertisedBenefit = 0;

    // ..done
    AuraList const& mDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for (AuraList::const_iterator i = mDamageDone.begin();i != mDamageDone.end(); ++i)
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0 &&
        (*i)->GetSpellProto()->EquippedItemClass == -1 &&
                                                            // -1 == any item class (not wand then)
        (*i)->GetSpellProto()->EquippedItemInventoryTypeMask == 0)
                                                            // 0 == any inventory type (not wand then)
            DoneAdvertisedBenefit += (*i)->GetModifierValue();

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Damage bonus from stats
        AuraList const& mDamageDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT);
        for (AuraList::const_iterator i = mDamageDoneOfStatPercent.begin();i != mDamageDoneOfStatPercent.end(); ++i)
        {
            if ((*i)->GetModifier()->m_miscvalue & schoolMask)
            {
                SpellEntry const* iSpellProto = (*i)->GetSpellProto();
                uint8 eff = (*i)->GetEffIndex();

                // stat used dependent from next effect aura SPELL_AURA_MOD_SPELL_HEALING presence and misc value (stat index)
                Stats usedStat = STAT_INTELLECT;
                if (eff < 2 && iSpellProto->EffectApplyAuraName[eff+1]==SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT)
                    usedStat = Stats(iSpellProto->EffectMiscValue[eff+1]);

                DoneAdvertisedBenefit += int32(GetStat(usedStat) * (*i)->GetModifierValue() / 100.0f);
            }
        }
        // ... and attack power
        AuraList const& mDamageDonebyAP = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER);
        for (AuraList::const_iterator i =mDamageDonebyAP.begin();i != mDamageDonebyAP.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & schoolMask)
                DoneAdvertisedBenefit += int32(GetTotalAttackPowerValue(BASE_ATTACK) * (*i)->GetModifierValue() / 100.0f);

    }
    return DoneAdvertisedBenefit;
}

int32 Unit::SpellBaseDamageBonusForVictim(SpellSchoolMask schoolMask, Unit *pVictim)
{
    uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();

    int32 TakenAdvertisedBenefit = 0;
    // ..done (for creature type by mask) in taken
    AuraList const& mDamageDoneCreature = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for (AuraList::const_iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if (creatureTypeMask & uint32((*i)->GetModifier()->m_miscvalue))
            TakenAdvertisedBenefit += (*i)->GetModifierValue();

    // ..taken
    AuraList const& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for (AuraList::const_iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
            TakenAdvertisedBenefit += (*i)->GetModifierValue();

    return TakenAdvertisedBenefit;
}

bool Unit::isSpellCrit(Unit *pVictim, SpellEntry const *spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType, float spellExtraChance)
{
    if (ToCreature() && ToCreature()->isTotem())
        if (Unit* owner = GetOwner())
            return owner->isSpellCrit(pVictim, spellProto, schoolMask, attackType, spellExtraChance);

    if (!SpellMgr::CanSpellCrit(spellProto))
        return false;

    // creatures can't crit with spells
    if (IS_CREATURE_GUID(GetGUID()))
        return false;

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_DPS_TESTING)
        return true;

    float baseChance = 0.0f;
    float extraChance = 0.0f;
    switch (spellProto->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_NONE: // spells that should not be affected by crit are filtered in CanSpellCrit already
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            if (schoolMask & SPELL_SCHOOL_MASK_NORMAL)
                baseChance = 0.0f;
            // For other schools
            else if (GetTypeId() != TYPEID_PLAYER)
            {
                baseChance = m_baseSpellCritChance;
                baseChance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, schoolMask);
            }
            // taken
            if (pVictim && !SpellMgr::IsPositiveSpell(spellProto->Id))
            {
                // Modify critical chance by victim SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE
                extraChance += pVictim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE, schoolMask);
                // Modify critical chance by victim SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE
                extraChance += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE);
                // Modify by player victim resilience
                if (pVictim->GetTypeId() == TYPEID_PLAYER)
                    extraChance -= ((Player*)pVictim)->GetRatingBonusValue(CR_CRIT_TAKEN_SPELL);
                // scripted (increase crit chance ... against ... target by x%
                if (pVictim->isFrozen()) // Shatter
                {
                    AuraList const& mOverrideClassScript = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
                    {
                        switch ((*i)->GetModifier()->m_miscvalue)
                        {
                            case 849: extraChance+= 10.0f; break; //Shatter Rank 1
                            case 910: extraChance+= 20.0f; break; //Shatter Rank 2
                            case 911: extraChance+= 30.0f; break; //Shatter Rank 3
                            case 912: extraChance+= 40.0f; break; //Shatter Rank 4
                            case 913: extraChance+= 50.0f; break; //Shatter Rank 5
                        }
                    }
                }
            }
            break;
        }
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
        {
            if (pVictim)
            {
                baseChance = GetUnitCriticalChance(attackType, pVictim);
                extraChance += (int32(GetMaxSkillValueForLevel(pVictim)) - int32(pVictim->GetDefenseSkillValue(this))) * 0.04f;
                baseChance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, schoolMask);
                if (baseChance > 0 && !pVictim->IsStandState()) //Always crit against a sitting targets (except 0 crit chance)
                    return true;
            }
            break;
        }
        default:
            return false; // already done in CanSpellCrit. Just in case leave it here too
    }

    baseChance += spellExtraChance;
    SendCombatStats(1<<COMBAT_STATS_CRIT_CHANCE, "isSpellCrit (id=%d): baseChance = %f (spellExtraChance = %f) extraChance = %f totalChance = %f", pVictim, spellProto->Id, baseChance, spellExtraChance, extraChance, baseChance + extraChance);
    return RollPRD(baseChance/100, extraChance/100, spellProto->Id);
}

uint32 Unit::SpellCriticalBonus(SpellEntry const *spellProto, uint32 damage, Unit *pVictim)
{
    // Calculate critical bonus
    int32 crit_bonus;
    switch (spellProto->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:                      // for melee based spells is 100%
        case SPELL_DAMAGE_CLASS_RANGED:
            // TODO: write here full calculation for melee/ranged spells
            crit_bonus = damage;
            break;
        default:
            crit_bonus = damage / 2;                        // for spells is 50%
            break;
    }

    // adds additional damage to crit_bonus (from talents)
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);

    if (pVictim)
    {
        uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();
        crit_bonus = int32(crit_bonus * GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, creatureTypeMask));
    }

    if (crit_bonus > 0)
        damage += crit_bonus;

    return damage;
}

uint32 Unit::SpellHealingBonus(SpellEntry const *spellProto, uint32 healamount, DamageEffectType damagetype, Unit *pVictim, CasterModifiers *casterModifiers)
{
    // For totems get healing bonus from owner (statue isn't totem in fact)
    if (GetTypeId()==TYPEID_UNIT && ((Creature*)this)->isTotem() && ((Totem*)this)->GetTotemType()!=TOTEM_STATUE)
        if (Unit* owner = GetOwner())
            return owner->SpellHealingBonus(spellProto, healamount, damagetype, pVictim, casterModifiers);

    float TotalMod = 1.0f;
    // Healing taken percent
    float minval = pVictim->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_HEALING_PCT);
    if (minval)
        TotalMod *= (100.0f + minval) / 100.0f;

    float maxval = pVictim->GetMaxPositiveAuraModifier(SPELL_AURA_MOD_HEALING_PCT);
    if (maxval)
        TotalMod *= (100.0f + maxval) / 100.0f;

    // Some spells are not healing spells itself, but they just trigger other healing spell (with fixed amount of healing)
    // Don't apply SPELL_AURA_MOD_HEALING_PCT on them (it will be applied on triggered spell)
    if ((spellProto->Id == 33763 && damagetype != DOT) ||                                                       // lifebloom final heal
        (spellProto->SpellFamilyName == SPELLFAMILY_SHAMAN && spellProto->SpellFamilyFlags == 0x40000000000l) || // earth shield
        spellProto->Id == 41635)                                                                                  // prayer of mending
        TotalMod = 1.0f;

    // These Spells are doing fixed amount of healing (TODO found less hack-like check)
    if (spellProto->Id == 15290 || spellProto->Id == 39373 ||
        spellProto->Id == 33778 || spellProto->Id == 379   ||
        spellProto->Id == 38395 || spellProto->Id == 40972 ||
        spellProto->Id == 22845 || spellProto->Id == 33504 ||
        spellProto->Id == 34299 || spellProto->Id == 27813 ||
        spellProto->Id == 27817 || spellProto->Id == 27818 ||
        spellProto->Id == 5707  || spellProto->Id == 33110 ||
        spellProto->Id == 37382 || spellProto->Id == 25608)
        return healamount*TotalMod;

    int32 AdvertisedBenefit = SpellBaseHealingBonus(SpellMgr::GetSpellSchoolMask(spellProto));
    if (casterModifiers)
    {
        if (casterModifiers->Apply)
            AdvertisedBenefit = casterModifiers->AdvertisedBenefit;
        else
            casterModifiers->AdvertisedBenefit = AdvertisedBenefit;
    }
    uint32 CastingTime = !SpellMgr::IsChanneledSpell(spellProto) ? SpellMgr::GetSpellCastTime(spellProto) : SpellMgr::GetSpellDuration(spellProto);

    // Healing Taken
    AdvertisedBenefit += SpellBaseHealingBonusForVictim(SpellMgr::GetSpellSchoolMask(spellProto), pVictim);

    // Blessing of Light dummy effects healing taken from Holy Light and Flash of Light
    if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && (spellProto->SpellFamilyFlags & 0x00000000C0000000LL))
    {
        AuraList const& mDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
        for (AuraList::const_iterator i = mDummyAuras.begin();i != mDummyAuras.end(); ++i)
        {
            if ((*i)->GetSpellProto()->SpellVisual == 9180)
            {
                // Flash of Light
                if ((spellProto->SpellFamilyFlags & 0x0000000040000000LL) && (*i)->GetEffIndex() == 1)
                {
                    AdvertisedBenefit += (*i)->GetModifier()->m_amount;
                    if (HasAura(38320)) AdvertisedBenefit += 60; // libram of souls redeemed
                    break;
                }
                // Holy Light
                else if ((spellProto->SpellFamilyFlags & 0x0000000080000000LL) && (*i)->GetEffIndex() == 0)
                {
                    AdvertisedBenefit += (*i)->GetModifier()->m_amount;
                    if (HasAura(38320)) AdvertisedBenefit += 120; // libram of souls redeemed
                    break;
                }
            }
        }
    }

    // Flash of Light
    if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && (spellProto->SpellFamilyFlags & 0x0000000040000000LL))
    {
        AuraList const& dummyAuras = GetAurasByType(SPELL_AURA_DUMMY);
        for (AuraList::const_iterator i = dummyAuras.begin(); i != dummyAuras.end(); i++)
        {
            uint32 id = (*i)->GetSpellProto()->Id;
            if (id == 28851 || id == 28853 || id == 32403)   // bonuses from various librams
                AdvertisedBenefit += (*i)->GetModifierValue();
        }
    }

    // Holy Light
    if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && (spellProto->SpellFamilyFlags & 0x0000000080000000LL))
    {
        AuraList const& dummyAuras = GetAurasByType(SPELL_AURA_DUMMY);
        for (AuraList::const_iterator i = dummyAuras.begin(); i != dummyAuras.end(); i++)
        {
            uint32 id = (*i)->GetSpellProto()->Id;
            if (id == 34231)   // bonuses from various librams
                AdvertisedBenefit += (*i)->GetModifierValue();
        }
    }

    // Lesser Healing Wave
    if (spellProto->SpellFamilyName == SPELLFAMILY_SHAMAN && spellProto->SpellFamilyFlags & 0x80)
    {
        AuraList const& classScriptsAuras = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (AuraList::const_iterator i = classScriptsAuras.begin(); i != classScriptsAuras.end(); i++)
        {
            // Increased Lesser Healing Wave (few items has this effect)
            if ((*i)->GetMiscValue() == 3736)
                AdvertisedBenefit += (*i)->GetModifierValue();
        }
    }

    float ActualBenefit = 0.0f;

    if (AdvertisedBenefit != 0)
    {
        // Healing over Time spells
        float DotFactor = 1.0f;
        if (damagetype == DOT)
        {
            int32 DotDuration = SpellMgr::GetSpellDuration(spellProto);
            if (DotDuration > 0)
            {
                // 200% limit
                if (DotDuration > 30000) DotDuration = 30000;
                if (!SpellMgr::IsChanneledSpell(spellProto)) DotFactor = DotDuration / 15000.0f;
                int x = 0;
                for (int j = 0; j < 3; j++)
                {
                    if (spellProto->Effect[j] == SPELL_EFFECT_APPLY_AURA && (
                        spellProto->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_HEAL ||
                        spellProto->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_LEECH))
                    {
                        x = j;
                        break;
                    }
                }
                int DotTicks = 6;
                if (spellProto->EffectAmplitude[x] != 0)
                    DotTicks = DotDuration / spellProto->EffectAmplitude[x];
                if (DotTicks)
                    AdvertisedBenefit /= DotTicks;
            }
        }

        // distribute healing to all effects, reduce AoE damage
        CastingTime = GetCastingTimeForBonus(spellProto, damagetype, CastingTime);

        if (spellProto->AttributesCu & SPELL_ATTR_CU_NO_SPELL_DMG_COEFF)
            CastingTime = 0;

        // Exception
        switch (spellProto->SpellFamilyName)
        {
            case  SPELLFAMILY_SHAMAN:
                // Healing stream from totem (add 4.5% per tick from hill bonus owner)
                if (spellProto->SpellFamilyFlags & 0x000000002000LL)
                    CastingTime = 158;
                // Earth Shield 30% per charge
                else if (spellProto->SpellFamilyFlags & 0x40000000000LL)
                    CastingTime = 1000;
                break;
            case  SPELLFAMILY_DRUID:
                // Lifebloom
                if (spellProto->SpellFamilyFlags & 0x1000000000LL)
                {
                    CastingTime = damagetype == DOT ? 3500 : 1200;
                    DotFactor = damagetype == DOT ? 0.519f : 1.0f;
                }
                // Tranquility triggered spell
                else if (spellProto->SpellFamilyFlags & 0x80LL)
                    CastingTime = 667;
                // Rejuvenation
                else if (spellProto->SpellFamilyFlags & 0x10LL)
                    DotFactor = 0.845f;
                // Regrowth
                else if (spellProto->SpellFamilyFlags & 0x40LL)
                {
                    DotFactor = damagetype == DOT ? 0.705f : 1.0f;
                    CastingTime = damagetype == DOT ? 3500 : 1010;
                }
                // Improved Leader of the Pack
                else if (spellProto->AttributesEx2 == 536870912 && spellProto->SpellIconID == 312
                    && spellProto->AttributesEx3 == 33554432)
                {
                    CastingTime = 0;
                }
                break;
            case SPELLFAMILY_PRIEST:
                // Holy Nova - 14%
                if ((spellProto->SpellFamilyFlags & 0x8000000LL) && spellProto->SpellIconID == 1874)
                    CastingTime = 500;
                // Prayer of Mending - 42.86%
                if (spellProto->Id == 41635)
                    CastingTime = 1500;
                // Holy Nova - 16%
                else if ((spellProto->SpellFamilyFlags & 0x8000000LL) && spellProto->SpellIconID == 1874)
                {
                    CastingTime = 560;
                }
                // renew 100%
                else if ((spellProto->SpellFamilyFlags & 0x40LL) && spellProto->SpellIconID == 321)
                {
                    DotFactor = 1.0526f;
                }
                break;
            case SPELLFAMILY_WARLOCK:
               if (spellProto->SpellFamilyFlags & 0x1000000LL)
                    CastingTime = 10000;
                break;
            case SPELLFAMILY_POTION:
            case SPELLFAMILY_GENERIC: // not sure about generic
                CastingTime = 0;
                break;
        }

        float LvlPenalty = CalculateLevelPenalty(spellProto);

        // Spellmod SpellDamage
        //float SpellModSpellDamage = 100.0f;
        float CoefficientPtc = ((float)CastingTime/3500.0f)*DotFactor*100.0f;

        if (Player* modOwner = GetSpellModOwner())
            //modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_SPELL_BONUS_DAMAGE,SpellModSpellDamage);
            modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_SPELL_BONUS_DAMAGE,CoefficientPtc);

        //SpellModSpellDamage /= 100.0f;
        CoefficientPtc /= 100.0f;
        if (casterModifiers)
        {
            if (casterModifiers->Apply)
                CoefficientPtc = casterModifiers->CoefficientPtc;
            else
                casterModifiers->CoefficientPtc = CoefficientPtc;
        }

        //ActualBenefit = (float)AdvertisedBenefit * ((float)CastingTime / 3500.0f) * DotFactor * SpellModSpellDamage * LvlPenalty;
        ActualBenefit = (float)AdvertisedBenefit * CoefficientPtc * LvlPenalty;
    }
    else if (casterModifiers && !casterModifiers->Apply)
        casterModifiers->CoefficientPtc = 1.0f;

    // use float as more appropriate for negative values and percent applying
    float heal = healamount + ActualBenefit;

    // TODO: check for ALL/SPELLS type
    // Healing done percent
    float HealingDonePct = 1.0f;
    AuraList const& mHealingDonePct = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for (AuraList::const_iterator i = mHealingDonePct.begin();i != mHealingDonePct.end(); ++i)
        HealingDonePct *= (100.0f + (*i)->GetModifierValue()) / 100.0f;
    if (casterModifiers)
    {
        if (casterModifiers->Apply)
            HealingDonePct = casterModifiers->DamagePercentDone;
        else
            casterModifiers->DamagePercentDone = HealingDonePct;
    }

    heal *= HealingDonePct;

    // apply spellmod to Done amount
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, damagetype == DOT ? SPELLMOD_DOT : SPELLMOD_DAMAGE, heal);

    // Healing Wave cast
    if (spellProto->SpellFamilyName == SPELLFAMILY_SHAMAN && spellProto->SpellFamilyFlags & 0x0000000000000040LL)
    {
        // Search for Healing Way on Victim (stack up to 3 time)
        int32 pctMod = 0;
        Unit::AuraList const& auraDummy = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
        for (Unit::AuraList::const_iterator itr = auraDummy.begin(); itr!=auraDummy.end(); ++itr)
        {
            if ((*itr)->GetId() == 29203)
            {
                pctMod = (*itr)->GetModifier()->m_amount * (*itr)->GetStackAmount();
                break;
            }
        }
        // Apply bonus
        if (pctMod)
            heal = heal * (100 + pctMod) / 100;
    }

    heal = heal*TotalMod;
    if (heal < 0) heal = 0;

    if (GetObjectGuid().IsCreature())
        heal *= ((Creature*)this)->GetCreatureDamageMod();

    return uint32(heal);
}

int32 Unit::SpellBaseHealingBonus(SpellSchoolMask schoolMask)
{
    int32 AdvertisedBenefit = 0;

    AuraList const& mHealingDone = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE);
    for (AuraList::const_iterator i = mHealingDone.begin();i != mHealingDone.end(); ++i)
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
            AdvertisedBenefit += (*i)->GetModifierValue();

    // Healing bonus of spirit, intellect and strength
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Healing bonus from stats
        AuraList const& mHealingDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT);
        for (AuraList::const_iterator i = mHealingDoneOfStatPercent.begin();i != mHealingDoneOfStatPercent.end(); ++i)
        {
            // stat used dependent from misc value (stat index)
            Stats usedStat = Stats((*i)->GetSpellProto()->EffectMiscValue[(*i)->GetEffIndex()]);
            AdvertisedBenefit += int32(GetStat(usedStat) * (*i)->GetModifierValue() / 100.0f);
        }

        // ... and attack power
        AuraList const& mHealingDonebyAP = GetAurasByType(SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER);
        for (AuraList::const_iterator i = mHealingDonebyAP.begin();i != mHealingDonebyAP.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & schoolMask)
                AdvertisedBenefit += int32(GetTotalAttackPowerValue(BASE_ATTACK) * (*i)->GetModifierValue() / 100.0f);
    }
    return AdvertisedBenefit;
}

int32 Unit::SpellBaseHealingBonusForVictim(SpellSchoolMask schoolMask, Unit *pVictim)
{
    int32 AdvertisedBenefit = 0;
    AuraList const& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_HEALING);
    for (AuraList::const_iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
            AdvertisedBenefit += (*i)->GetModifierValue();
    return AdvertisedBenefit;
}

bool Unit::IsImmunedToDamage(SpellSchoolMask shoolMask, bool includeCharges)
{
    //If m_immuneToSchool type contain this school type, IMMUNE damage.
    SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
    for (SpellImmuneList::const_iterator itr = schoolList.begin(); itr != schoolList.end(); ++itr)
        if (itr->type & shoolMask)
            return true;

    //If m_immuneToDamage type contain magic, IMMUNE damage.
    // must be checked last
    SpellImmuneList const& damageList = m_spellImmune[IMMUNITY_DAMAGE];
    for (SpellImmuneList::const_iterator itr = damageList.begin(); itr != damageList.end(); ++itr)
    {
        if (itr->type & shoolMask)
        {
            if (!includeCharges)
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->spellId);
                if (spellInfo && spellInfo->procCharges > 0) // don't count it
                    continue;
            }
            return true;
        }
    }
    return false;
}

bool Unit::IsImmunedToSpellMechanic(uint32 mechanic, bool includeCharges)
{
    SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
    for (SpellImmuneList::const_iterator itr = mechanicList.begin(); itr != mechanicList.end(); ++itr)
    {
        if (itr->type == mechanic)
        {
            if (!includeCharges)
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->spellId);
                if (spellInfo && spellInfo->procCharges > 0) // don't count it
                    continue;
            }
            return true;
        }
    }
    return false;
}

bool Unit::IsImmunedToSpell(SpellEntry const* spellInfo, bool includeCharges)
{
    if (!spellInfo)
        return false;

    // If spell has this flag, it cannot be resisted/immuned/etc
    if (spellInfo->Attributes & SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY)
        return false;

    SpellImmuneList const& stateList = m_spellImmune[IMMUNITY_STATE];
    for (SpellImmuneList::const_iterator itr = stateList.begin(); itr != stateList.end(); ++itr)
            if ((itr->type == spellInfo->EffectApplyAuraName[0] || spellInfo->EffectApplyAuraName[0] == SPELL_AURA_NONE) &&
                (itr->type == spellInfo->EffectApplyAuraName[1] || spellInfo->EffectApplyAuraName[1] == SPELL_AURA_NONE) &&
                (itr->type == spellInfo->EffectApplyAuraName[2] || spellInfo->EffectApplyAuraName[2] == SPELL_AURA_NONE) &&
                spellInfo->HasApplyAura(itr->type)) // to be immune to spell unit needs to be immune to all auras
                return true;

    if (!(spellInfo->AttributesEx & SPELL_ATTR_EX_UNAFFECTED_BY_SCHOOL_IMMUNE) &&         // unaffected by school immunity
        !(spellInfo->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)               // can remove immune (by dispell or immune it)
        && (spellInfo->Id != 42292))
    {
        SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
        for (SpellImmuneList::const_iterator itr = schoolList.begin(); itr != schoolList.end(); ++itr)
            if (!(SpellMgr::IsPositiveSpell(itr->spellId) && SpellMgr::IsPositiveSpell(spellInfo->Id)) &&
                (itr->type & SpellMgr::GetSpellSchoolMask(spellInfo)))
                return true;
    }

    SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_DISPEL];
    for (SpellImmuneList::const_iterator itr = dispelList.begin(); itr != dispelList.end(); ++itr)
        if (itr->type == spellInfo->Dispel)
            return true;

    // must be checked last. In this case scripts are last, but all other checks must be before this one
    if (IsImmunedToSpellMechanic(spellInfo->Mechanic, includeCharges))
        return true;

    // used by scripts
    SpellImmuneList const& idList = m_spellImmune[IMMUNITY_ID];
    for (SpellImmuneList::const_iterator itr = idList.begin(); itr != idList.end(); ++itr)
        if (itr->type == spellInfo->Id)
            return true;

    return false;
}

bool Unit::IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const
{
    //If m_immuneToEffect type contain this effect type, IMMUNE effect.
    SpellImmuneList const& effectList = m_spellImmune[IMMUNITY_EFFECT];
    for (SpellImmuneList::const_iterator itr = effectList.begin(); itr != effectList.end(); ++itr)
        if (itr->type == effect)
            return true;

    SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
    for (SpellImmuneList::const_iterator itr = mechanicList.begin(); itr != mechanicList.end(); ++itr)
        if (itr->type == mechanic)
            return true;

    return false;
}

void Unit::MeleeDamageBonus(Unit *pVictim, uint32 *pdamage,WeaponAttackType attType, SpellEntry const *spellProto)
{
    if (!pVictim)
        return;

    if (*pdamage == 0)
        return;

    uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();

    // Taken/Done fixed damage bonus auras
    int32 DoneFlatBenefit = 0;
    int32 TakenFlatBenefit = 0;

    // ..done (for creature type by mask) in taken
    DoneFlatBenefit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE, creatureTypeMask);

    // ..done
    // SPELL_AURA_MOD_DAMAGE_DONE included in weapon damage

    // ..done (base at attack power for marked target and base at attack power for creature type)
    int32 APbonus = 0;
    if (attType == RANGED_ATTACK)
    {
        APbonus += pVictim->GetTotalAuraModifier(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
        APbonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS, creatureTypeMask);
    }
    else
    {
        APbonus += pVictim->GetMeleeApAttackerBonus(); //pVictim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);
        APbonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS, creatureTypeMask);
    }

    if (APbonus!=0)                                         // Can be negative
    {
        bool normalized = spellProto ? spellProto->HasEffect(SPELL_EFFECT_NORMALIZED_WEAPON_DMG) : false;
        DoneFlatBenefit += int32(APbonus/14.0f * GetAPMultiplier(attType,normalized));
    }

    // ..taken
    TakenFlatBenefit += pVictim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, this->GetMeleeDamageSchoolMask());

    if (attType!=RANGED_ATTACK)
        TakenFlatBenefit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);
    else
        TakenFlatBenefit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);

    // Done/Taken total percent damage auras
    float DoneTotalMod = 1;
    float TakenTotalMod = 1;

    // ..done
    // SPELL_AURA_MOD_DAMAGE_PERCENT_DONE included in weapon damage. BUT for other spellschools than physical - must be applied. (example - paladins seal of command + sanctity aura)
    // SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT  included in weapon damage

    if (spellProto && (SpellMgr::GetSpellSchoolMask(spellProto) & this->GetMeleeDamageSchoolMask()) == 0)
    {
        AuraList const& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        for (AuraList::const_iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
        {
            switch ((*i)->GetId())
            {
                case 6057:  // M Wand Spec 1
                case 6085:  // M Wand Spec 2
                case 14524: // P Wand Spec 1
                case 14525: // P Wand Spec 2
                case 14526: // P Wand Spec 3
                case 14527: // P Wand Spec 4
                case 14528: // P Wand Spec 5
                    //if (spellProto->Id != 5019) // Wand Shoot - Has spellschoolmask PHYSICAL - so always continue;
                        continue;
                default:
                    break;
            }

            if (((*i)->GetModifier()->m_miscvalue & SpellMgr::GetSpellSchoolMask(spellProto)) &&
                (GetTypeId() != TYPEID_PLAYER || ((Player*)this)->HasItemFitToSpellReqirements((*i)->GetSpellProto())))
            {
                DoneTotalMod *= ((*i)->GetModifierValue() +100.0f)/100.0f;
            }
        }
    }

    AuraList const& mDamageDoneVersus = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS);
    for (AuraList::const_iterator i = mDamageDoneVersus.begin();i != mDamageDoneVersus.end(); ++i)
        if (creatureTypeMask & uint32((*i)->GetModifier()->m_miscvalue))
            DoneTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;

    // ..taken
    AuraList const& mModDamagePercentTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for (AuraList::const_iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i)
        if ((*i)->GetModifier()->m_miscvalue & this->GetMeleeDamageSchoolMask())
            TakenTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;

    // .. taken pct: dummy auras
    bool hasmangle = false;         // apply mangle effect only once
    AuraList const& mDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
    for (AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
    {
        switch ((*i)->GetSpellProto()->SpellIconID)
        {
            //Mangle
            case 2312:
                if (spellProto==NULL)
                    break;
                if (hasmangle)
                    break;
                hasmangle = true;
                // Should increase Shred (initial Damage of Lacerate and Rake handled in Spell::EffectSchoolDMG)
                if (spellProto->SpellFamilyName==SPELLFAMILY_DRUID && (spellProto->SpellFamilyFlags==0x00008000LL))
                    TakenTotalMod *= (100.0f+(*i)->GetModifier()->m_amount)/100.0f;
                break;
        }
    }

    // .. taken pct: class scripts
    AuraList const& mclassScritAuras = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mclassScritAuras.begin(); i != mclassScritAuras.end(); ++i)
    {
        switch ((*i)->GetMiscValue())
        {
            case 6427: case 6428:                           // Dirty Deeds
                if (pVictim->HasAuraState(AURA_STATE_HEALTHLESS_35_PERCENT) && spellProto &&
                    spellProto->SpellFamilyName == SPELLFAMILY_ROGUE && spellProto->Attributes & SPELL_ATTR_ABILITY)
                {
                    Aura* eff0 = GetAura((*i)->GetId(), 0);
                    if (!eff0 || (*i)->GetEffIndex() != 1)
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Spell structure of DD (%u) changed.", (*i)->GetId());
                        continue;
                    }

                    // effect 0 have expected value but in negative state
                    TakenTotalMod *= (-eff0->GetModifier()->m_amount + 100.0f) / 100.0f;
                }
                break;
        }
    }

    if (attType != RANGED_ATTACK)
    {
        AuraList const& mModMeleeDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);
        for (AuraList::const_iterator i = mModMeleeDamageTakenPercent.begin(); i != mModMeleeDamageTakenPercent.end(); ++i)
            TakenTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;
    }
    else
    {
        AuraList const& mModRangedDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);
        for (AuraList::const_iterator i = mModRangedDamageTakenPercent.begin(); i != mModRangedDamageTakenPercent.end(); ++i)
            TakenTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;
    }

    float tmpDamage = float(int32(*pdamage) + DoneFlatBenefit) * DoneTotalMod;

    // apply spellmod to Done damage
    if (spellProto)
    {
        if (Player* modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_DAMAGE, tmpDamage);
    }

    tmpDamage = (tmpDamage + TakenFlatBenefit)*TakenTotalMod;

    // bonus result can be negative
    *pdamage =  tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

void Unit::ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply)
{
    for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(); itr != m_spellImmune[op].end();)
    {
        if (itr->spellId == spellId && itr->type == type)
            itr = m_spellImmune[op].erase(itr);
        else
            itr++;
    }

    if (apply)
    {
        SpellImmune Immune;
        Immune.spellId = spellId;
        Immune.type = type;
        m_spellImmune[op].push_back(Immune);
    }
}

void Unit::ApplySpellDispelImmunity(const SpellEntry * spellProto, DispelType type, bool apply)
{
    ApplySpellImmune(spellProto->Id,IMMUNITY_DISPEL, type, apply);

    if (apply && spellProto->AttributesEx & SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)
        RemoveAurasWithDispelType(type);
}

float Unit::GetWeaponProcChance() const // Trentone - this is hacky as fuck and i believe it is mostly unused/useless
{
    // normalized proc chance for weapon attack speed
    // (odd formula...)
    if (isAttackReady(BASE_ATTACK))
        return (GetAttackTime(BASE_ATTACK) * 1.8f / 1000.0f);
    else if (haveOffhandWeapon() && isAttackReady(OFF_ATTACK))
        return (GetAttackTime(OFF_ATTACK) * 1.6f / 1000.0f);
    return 0;
}

float Unit::GetPPMProcChance(uint32 WeaponSpeed, float PPM) const
{
    // proc per minute chance calculation
    if (PPM <= 0) return 0.0f;
    return float((WeaponSpeed * PPM) / 600.0f);   // result is chance in percents (probability = Speed_in_sec * (PPM / 60))
}

void Unit::Mount(uint32 mount)
{
    if (!mount)
        return;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MOUNT);

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, mount);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT);

    // unsummon pet
    if (GetTypeId() == TYPEID_PLAYER)
    {
        Pet* pet = GetPet();
        if (pet)
        {
            BattleGround *bg = ((Player *)this)->GetBattleGround();
            // don't unsummon pet in arena but SetFlag UNIT_FLAG_DISABLE_ROTATE to disable pet's interface
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(pet->GetUInt32Value(UNIT_CREATED_BY_SPELL));

            bool is_temporary_summoned = spellInfo && SpellMgr::GetSpellDuration(spellInfo) > 0;
            // Water Elemental should not disappear when mounted
            if (bg && bg->isArena() || is_temporary_summoned)
                pet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
            else
            {
                if (pet->isControlled())
                {
                    ((Player*)this)->SetTemporaryUnsummonedPetNumber(pet->GetCharmInfo()->GetPetNumber());
                    ((Player*)this)->SetOldPetSpell(pet->GetUInt32Value(UNIT_CREATED_BY_SPELL));
                }
                ((Player*)this)->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT);
                return;
            }
        }
        ((Player*)this)->SetTemporaryUnsummonedPetNumber(0);
    }
    if (GetDummyAura(54836))
    {
        CastSpell(this, 15473, true);
        RemoveAurasDueToSpell(54836);
    }
}

void Unit::Unmount()
{
    if (!IsMounted())
        return;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_MOUNTED);

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT);

    // only resummon old pet if the player is already added to a map
    // this prevents adding a pet to a not created map which would otherwise cause a crash
    // (it could probably happen when logging in after a previous crash)
    if (GetTypeId() == TYPEID_PLAYER && IsInWorld() && isAlive())
    {
        if (((Player*)this)->GetTemporaryUnsummonedPetNumber())
        {
            Pet* NewPet = new Pet;
            if (!NewPet->LoadPetFromDB(this, 0, ((Player*)this)->GetTemporaryUnsummonedPetNumber(), true))
                delete NewPet;
            ((Player*)this)->SetTemporaryUnsummonedPetNumber(0);
        }
        else
           if (Pet *pPet = GetPet())
               if (pPet->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE) && !pPet->HasUnitState(UNIT_STAT_STUNNED))
                   pPet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);

        // resummon golem guardian after using mount events
        if (sWorld.isEasyRealm() && IsLowHeroicDungeonOrNonactualRaid(GetMapId(), false) && ((Player*)this)->CountGuardianWithEntry(NPC_GOLEM_GUARDIAN) == 0)
            CastSpell(this, 55200, true);
    }
}

void Unit::SetInCombatWith(Unit* enemy)
{
    if(!enemy)
        return;

	if (Creature *creature = ToCreature())
	{
		// should do it on pull, because we need to set recipients on this stage to avoid loot-loss if someone gets disconnedted during fight
        creature->GenerateLoot(enemy, true);

        if (!creature->pulled)
        {
            creature->pulled = true;

            if (sWorld.IsShutdowning() && sWorld.GetShutdownTimer() < BOSS_AUTOEVADE_RESTART_TIME && creature->isWorldBoss())
            {
                creature->StopMoving();
                creature->AI()->EnterEvadeMode();
                enemy->ClearInCombat();
                creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_RESTART_PACIFIED);
                return;
            }
        }
	}

    Unit* eOwner = enemy->GetCharmerOrOwnerOrSelf();
    if (eOwner->IsPvP())
    {
		// involving cross faction parties in FFA PvP
		// need a better code
		//if (!sWorld.IsFFAPvPRealm())
		//{
		//	Player* p = ToPlayer();
		//	Player* pEnemy = eOwner->ToPlayer();
		//	if (p && pEnemy && !p->InBattleGroundOrArena() && !sWorld.isPvPArea(p->GetCachedArea()))
		//	{
		//		bool found = false;
		//		Group* group = p->GetGroup();
		//		if (group)
		//		{
		//			for (GroupReference *itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
		//			{
		//				Player* t = itr->getSource();
		//				if (t && t->IsInWorld() && t->GetTeam() != p->GetTeam() && t->GetCachedArea() == p->GetCachedArea())
		//				{
		//					found = true;
		//					t->UpdatePvP(true, true);
		//					t->SetFFAPvP(true);
		//				}
		//			}

		//			if (found)
		//			{
		//				p->SetFFAPvP(true);
		//				pEnemy->SetFFAPvP(true);
		//			}
		//		}
		//	}
		//}
		
		SetInCombatState(enemy, COMBAT_TIME_PVP_MIN);
        return;
    }

    //check for duel
    if (eOwner->GetTypeId() == TYPEID_PLAYER && ((Player*)eOwner)->duel)
    {
        Unit const* myOwner = GetCharmerOrOwnerOrSelf();
        if (((Player const*)eOwner)->duel->opponent == myOwner)
        {
            SetInCombatState(enemy, COMBAT_TIME_PVP_MIN);
            return;
        }
    }
    SetInCombatState(enemy);
}

void Unit::CombatStart(Unit* target, bool initialAggro)
{
    if (!isAlive() || !target->isAlive() || (target->GetTypeId() == TYPEID_UNIT && target->ToCreature()->IsInEvadeMode()))
        return;

    if (initialAggro)
    {
        if (!target->IsStandState()/* && !target->HasUnitState(UNIT_STAT_STUNNED)*/)
            target->addUnitState(UNIT_STAT_STAND_UP_PENDING);

        if (!target->IsInCombat() && target->GetTypeId() != TYPEID_PLAYER
            && !((Creature*)target)->HasReactState(REACT_PASSIVE) && ((Creature*)target)->IsAIEnabled)
        {
            ((Creature*)target)->AI()->AttackStart(this);
        }

        target->AddThreat(this, 0.0f);

        SetInCombatWith(target);
        target->SetInCombatWith(this);
    }

    Player *me = GetCharmerOrOwnerPlayerOrPlayerItself(); // player might not exist
    Unit *who = target->GetCharmerOrOwnerOrSelf(); // will exist for sure

    if (me)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            // SetContestedPvP itself finds player by Player* player = GetCharmerOrOwnerPlayerOrPlayerItself(); so only makes sense if "me" and "who" are players
            SetContestedPvP((Player*)who);
            if (who->IsPvP() && (!me->duel || me->duel->opponent != who))
                me->UpdatePvP(true);

            me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT | 
                (me->GetClass() == CLASS_HUNTER ? (AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP | AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_HUNTER_PVP) : AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP)); // remove auras in duels too
        }
        else if (who->IsPvP()) // definitely not player, only need to check by isPvP()
        {
            me->UpdatePvP(true);
            me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT |
                (me->GetClass() == CLASS_HUNTER ? (AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP | AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_HUNTER_PVP) : AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP));
        }
    }

	// need to test this I guess...
	//Unit *who = target->GetCharmerOrOwnerOrSelf();
	//if (who->GetTypeId() == TYPEID_PLAYER)
	//	SetContestedPvP((Player*)who);

	//Player *me = GetCharmerOrOwnerPlayerOrPlayerItself();
	//if (me && who->IsPvP()
	//	&& (who->GetTypeId() != TYPEID_PLAYER
	//		|| !me->duel || me->duel->opponent != who))
	//{
	//	me->UpdatePvP(true);
	//	me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT | 
	//		(me->GetClass() == CLASS_HUNTER ? (AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP | AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_HUNTER_PVP) : AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP));
	//}
}

void Unit::CombatStartOnCast(Unit* target, bool initialAggro, uint32 minCombatTime)
{
    CombatStart(target, false);

    if (initialAggro)
        SetInCombatState(target, target->isCharmedOwnedByPlayerOrPlayer() ? COMBAT_TIME_PVP_MIN : minCombatTime);
}

void Unit::SetInCombatState(Unit* enemy, uint32 minCombatTime)
{
    // only alive units can be in combat
    if (!isAlive())
        return;

    if (minCombatTime)
        SetCombatTimer(minCombatTime);

    if (GetTypeId() == TYPEID_UNIT && ToCreature()->isPet())
    {
        if (Unit* owner = GetCharmerOrOwner())
        {
            if (!minCombatTime)
                owner->SetInCombatState(enemy);
            // && owner->GetCombatTimer() - Gensen: i removed this because it returns 0 and will never set combat to owner
            else if (owner->GetTypeId() == TYPEID_PLAYER) 
                owner->SetInCombatState(enemy, minCombatTime);
                //owner->SetCombatTimer(minCombatTime);
        }
    }

    if (IsInCombat())
        return;

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    // hack fix for instantly combat ending in instances (2 players: 1 aggro mob, 2 use polymorph = 1 is in combat, 2 not in combat)
    //if (GetMap()->IsDungeon() && 
    //    GetTypeId() == TYPEID_PLAYER && 
    //    enemy->GetTypeId() == TYPEID_UNIT &&
    //    getHostileRefManager().isEmpty())
    //    enemy->AddThreat(this, 0.0f);

    if (Player* player = ToPlayer())
    {
        if (Spell *spell = GetCurrentSpell(CURRENT_GENERIC_SPELL))
            if (spell->getState() != SPELL_STATE_FINISHED &&
                spell->GetSpellEntry()->Attributes & SPELL_ATTR_CANT_USED_IN_COMBAT &&
                spell->GetSpellEntry()->SpellFamilyName != SPELLFAMILY_WARRIOR)
                InterruptSpell(CURRENT_GENERIC_SPELL);

        if (Pet* pet = GetPet())
            pet->SetInCombatState(enemy, minCombatTime);

		// remove non-self buffs on combat in instances
		Map* map = GetMap();
		if (map->IsDungeon())
		{
			Map::PlayerList const &plList = GetMap()->GetPlayers();
			std::set<uint64> member_list;
			for (Map::PlayerList::const_iterator i = plList.begin(); i != plList.end(); ++i)
			{
				if (Player* member = i->getSource())
				{
					if (member == player)
						continue;

					member_list.insert(member->GetGUID());
				}
			}

			//Unit::AuraMap& tAuras = GetAuras();
			//for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
			//{
			//	if (IS_PLAYER_GUID(itr->second->GetCasterGUID())
			//		&& itr->second->IsPositive()
			//		&& !itr->second->IsPassive()
			//		&& itr->second->GetCasterGUID() != GetGUID()
			//		&& member_list.find(itr->second->GetCasterGUID()) == member_list.end())
			//	{
			//		RemoveAurasDueToSpell(itr->second->GetId());
			//		itr = tAuras.begin();
			//	}
			//	else
			//	{
			//		++itr;
			//	}
			//}
		}

        if (!isCharmed())
            return;
    }
    else
    {
        Creature* creature = ToCreature();

        if ((IsAIEnabled && creature->AI()->IsEscorted()) ||
            GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE ||
            GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE && creature->GetFormation())
            creature->SetHomePosition(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());

        if (enemy)
        {
            if (creature->AI())
                creature->AI()->EnterCombat(enemy);

            if (creature->GetFormation())
                creature->GetFormation()->MemberAttackStart(creature, enemy);

            SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
        }
    }

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);
}

void Unit::ClearInCombat()
{
    m_CombatTimer = 0;

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    // Player's state will be cleared in Player::UpdateContestedPvP
    if (Creature *creature = ToCreature())
    {
        if (creature->GetCreatureInfo() && creature->GetCreatureInfo()->unit_flags & UNIT_FLAG_NOT_ATTACKABLE_2)
            SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);

        creature->setActive(false, ACTIVE_BY_COMBAT);
        ClearUnitState(UNIT_STAT_ATTACK_PLAYER);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);

		if (IsAIEnabled && creature->AI())
			creature->AI()->OnCombatStop();

		creature->RestoreDBEmote();
    }
    else if (!isCharmed())
        return;
    // else - charmed player
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);
}

void Unit::CombatTimerTick(uint32 update_diff)
{
	// update combat timer only for players and pets 
	if (IsInCombat())
	{
		// combat timer should still tick when in combat, even if there are creatures attacking us.
		if (isCharmedOwnedByPlayerOrPlayer())
		{
			if (m_CombatTimer <= 2000)
			{
				if (getHostileRefManager().isEmpty())
					ClearInCombat();
			}
			else
				m_CombatTimer -= 2000;
		}	
		//else if (GetTypeId() == TYPEID_UNIT && IsAIEnabled && ToCreature()->AI() && ToCreature()->AI()->IsEscorted())
		//{
		//	if (m_CombatTimer <= 2000)
		//	{
		//		HostileReference *ref = getHostileRefManager().getFirst();
		//		while (ref)
		//		{
		//			Unit* target = ref->getSource()->getOwner();
		//			HostileReference* nextRef = ref->next();

		//			// creature vs creature - tick timer
		//			if (target && target->IsInCombat())
		//				return;

		//			ref = nextRef;
		//		}

		//		ClearInCombat();
		//	}
		//	else
		//		m_CombatTimer -= 2000;
		//}
	}
}

bool Unit::isTargetableForAttack() const
{
    if (!isAlive())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS,
        UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NOT_ATTACKABLE_2))
        return false;

    if (GetTypeId()==TYPEID_PLAYER && ((Player *)this)->isGameMaster())
        return false;

    return !IsTaxiFlying();
}

bool Unit::canAttack(Unit const* target, bool force) const
{
    ASSERT(target);

    if (force)
    {
        if (IsFriendlyTo(target))
            return false;
    }
    else if (!IsHostileTo(target))
        return false;

    if (!target->isTargetableForAttack())
        return false;

    // feign dead case
    if (target->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
    {
        if (!GetCharmerOrOwnerPlayerOrPlayerItself() && !isGuard() && !IsContestedGuard())
            return false;
    }

    if ((m_invisibilityMask || target->m_invisibilityMask) && !canDetectInvisibilityOf(target))
    {
        if (GetEntry() == 21795 && (target->HasAura(37468) || target->HasAura(37495) || target->HasAura(39841))) { }
        else 
            return false;  
    } 

    if (target->GetVisibility() == VISIBILITY_GROUP_STEALTH && !canDetectStealthOf(target, this, GetExactDist(target)))
        return false;

    return true;
}

bool Unit::isAttackableByAOE() const
{
    // creatures that should not be damaged by AoE spells
    if(GetTypeId() == TYPEID_UNIT)
    {
        switch(GetEntry())
        {
            case 24745: // Pure Energy
            case 17248: // Demon Chains(Karazhan - Terestian Illhoof)
                return false;
            default:
                return true;
        }
    }
    // we may place here also conditions (if exist?) when Player should not be damaged by AoE spells
    return true;
}

int32 Unit::ModifyHealth(int32 dVal)
{
    if (dVal < 0 && GetTypeId() == TYPEID_UNIT && (((Creature *)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_DAMAGE_TAKEN))
        return 0;
    if (dVal > 0 && GetTypeId() == TYPEID_UNIT && (((Creature *)this)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_HEALING_TAKEN))
        return 0;

    int32 gain = 0;

    if (dVal==0)
        return 0;

    int32 curHealth = (int32)GetHealth();

    int32 val = dVal + curHealth;
    if (val <= 0)
    {
        SetHealth(0); // this however should never happen. All uses of this method make sure that health left is > 0
        return -curHealth;
    }

    int32 maxHealth = (int32)GetMaxHealth();

    if (val < maxHealth)
    {
        SetHealth(val);
        gain = val - curHealth;
    }
    else if (curHealth != maxHealth)
    {
        SetHealth(maxHealth);
        gain = maxHealth - curHealth;
    }

    return gain;
}

// used only to calculate channeling time
void Unit::ModSpellCastTime(SpellEntry const* spellProto, int32 & castTime, Spell * spell)
{
    if (!spellProto || castTime<0 || spellProto->AttributesEx & SPELL_ATTR_EX_CHANNELLED_HASTE_UNAFFECTED)
        return;
    //called from caster
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CASTING_TIME, castTime, spell);

     if (spellProto->Attributes & SPELL_ATTR_RANGED && !(spellProto->AttributesEx2 & SPELL_ATTR_EX2_AUTOREPEAT_FLAG))
            castTime = int32 (float(castTime) * m_modAttackSpeedPct[RANGED_ATTACK]);
     else if (!(spellProto->Attributes & (SPELL_ATTR_ABILITY | SPELL_ATTR_TRADESPELL))) // no bonus for spells such as fishing, first aid, trade spells
        castTime = int32(float(castTime) * GetFloatValue(UNIT_MOD_CAST_SPEED));
}

int32 Unit::ModifyPower(Powers power, int32 dVal)
{
    int32 gain = 0;

    if (dVal==0)
        return 0;

    int32 curPower = (int32)GetPower(power);

    int32 val = dVal + curPower;
    if (val <= 0)
    {
        SetPower(power,0);
        return -curPower;
    }

    int32 maxPower = (int32)GetMaxPower(power);

    if (val < maxPower)
    {
        SetPower(power,val);
        gain = val - curPower;
    }
    else if (curPower != maxPower)
    {
        SetPower(power,maxPower);
        gain = maxPower - curPower;
    }

    return gain;
}

bool Unit::isVisibleForOrDetect(Unit const* unit, WorldObject const* viewPoint, bool detect, bool inVisibleList, bool is3dDistance) const
{
    if (!IsInMap(unit))
        return false;

    // Arena Preparation hack
    if (!IsInRaidWith(unit))
    {
        Player *player = GetCharmerOrOwnerPlayerOrPlayerItself();
        if (player && player->InArena() && player->GetBattleGround()->GetStatus() != STATUS_IN_PROGRESS)
            return false;
    }

    return unit->canSeeOrDetect(this, viewPoint, detect, inVisibleList, is3dDistance);
}

bool Unit::canSeeOrDetect(Unit const*, WorldObject const*, bool, bool, bool) const
{
    return true;
}

bool Unit::canDetectInvisibilityOf(Unit const* u) const
{
    if (m_invisibilityMask & u->m_invisibilityMask) // same group
        return true;

    Unit* owner = GetCharmerOrOwner();
    if (owner && owner != this)
    {
        if (owner->m_invisibilityMask || u->m_invisibilityMask)
        {
            if (owner->canDetectInvisibilityOf(u)) // if master OR pet can see invisibility -> they are able to attack
                return true;
        }
        else
            return true;
    }

   if(m_invisibilityMask == 1) // normal invisibility
    {
        uint32 invLevel = 0;
        Unit::AuraList const& iAuras = GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for (Unit::AuraList::const_iterator itr = iAuras.begin(); itr != iAuras.end(); ++itr)
            if (invLevel < (*itr)->GetModifier()->m_amount)
                invLevel = (*itr)->GetModifier()->m_amount;

        // creatures with greater visibility can see other creatures
        if(GetTypeId() == TYPEID_UNIT && invLevel >= 300)
            return true;
    }

    if(u->m_invisibilityMask == 1) // normal invisibility of target
    {
        uint32 invLevel = 0;
        int32 invLevelPenalty = 0;  //some auras reduce invisibility level
        Unit::AuraList const& iAuras = u->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for (Unit::AuraList::const_iterator itr = iAuras.begin(); itr != iAuras.end(); ++itr)
        {
            if ((*itr)->GetModifier()->m_amount < invLevelPenalty)
            {
                invLevelPenalty = (*itr)->GetModifier()->m_amount;
                continue;
            }
            if (invLevel < (*itr)->GetModifier()->m_amount)
                invLevel = (*itr)->GetModifier()->m_amount;
        }
        // we can see creatures with invisibility mask when invisibility level gets below 0
        if (abs(invLevelPenalty) > invLevel)
            return true;
    }

    if (!u->HasAuraType(SPELL_AURA_UNTRACKABLE))
    {
        AuraList const& auras = u->GetAurasByType(SPELL_AURA_MOD_STALKED); // Hunter mark
        for (AuraList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
            if ((*iter)->GetCasterGUID()==GetGUID())
                return true;
    }

    if (uint32 mask = (m_detectInvisibilityMask & u->m_invisibilityMask))
    {
        for (uint32 i = 0; i < 11; ++i)
        {
            if (((1 << i) & mask)==0)
                continue;

            // find invisibility level
            uint32 invLevel = 0;
            Unit::AuraList const& iAuras = u->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
            for (Unit::AuraList::const_iterator itr = iAuras.begin(); itr != iAuras.end(); ++itr)
                if (((*itr)->GetModifier()->m_miscvalue)==i && invLevel < (*itr)->GetModifier()->m_amount)
                    invLevel = (*itr)->GetModifier()->m_amount;

            // find invisibility detect level
            uint32 detectLevel = 0;
            if (i==6 && GetTypeId()==TYPEID_PLAYER)          // special drunk detection case
            {
                detectLevel = ((Player*)this)->GetDrunkValue();
            }
            else
            {
                Unit::AuraList const& dAuras = GetAurasByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);
                for (Unit::AuraList::const_iterator itr = dAuras.begin(); itr != dAuras.end(); ++itr)
                    if (((*itr)->GetModifier()->m_miscvalue)==i && detectLevel < (*itr)->GetModifier()->m_amount)
                        detectLevel = (*itr)->GetModifier()->m_amount;
            }

            if (invLevel <= detectLevel)
                return true;
        }
    }

    return false;
}

bool Unit::canDetectStealthOf(Unit const* target, WorldObject const* viewPoint, float distance) const
{
    Unit* owner = GetCharmerOrOwner();
    if (owner && owner != this)
    {
        if (owner->canDetectStealthOf(target, owner, owner->GetExactDist(target))) // if master OR pet can see invisibility -> they are able to attack
            return true;
    }
    
    if (HasUnitState(UNIT_STAT_STUNNED))
        return false;

    if (distance < 0.24f) //collision
        return true;

    if (!viewPoint->HasInArc(M_PI, target)) //behind
        return false;

    if (HasAuraType(SPELL_AURA_DETECT_STEALTH))
        return true;

    if (!target->HasAuraType(SPELL_AURA_UNTRACKABLE))
    {
        AuraList const& auras = target->GetAurasByType(SPELL_AURA_MOD_STALKED); // Hunter mark
        for (AuraList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
            if ((*iter)->GetCasterGUID() == GetGUID())
                return true;
    }

    //Visible distance based on stealth value (stealth rank 4 300MOD, 10.5 - 3 = 7.5)
    float visibleDistance = 7.5f;
    //Visible distance is modified by -Level Diff (every level diff = 1.0f in visible distance)
    visibleDistance += float(getLevelForTarget(target)) - target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH)/5.0f;
    //-Stealth Mod(positive like Master of Deception) and Stealth Detection(negative like paranoia)
    //based on wowwiki every 5 mod we have 1 more level diff in calculation
    visibleDistance += (float)(GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_DETECT, 0) - target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_LEVEL)) / 5.0f;
    visibleDistance = visibleDistance > MAX_PLAYER_STEALTH_DETECT_RANGE ? MAX_PLAYER_STEALTH_DETECT_RANGE : visibleDistance;

    return distance < visibleDistance;
}

void Unit::DestroyForNearbyPlayers()
{
    if (!IsInWorld())
        return;

    std::list<Player*> targets;
    Hellground::AnyUnitInObjectRangeCheck check(this, GetMap()->GetVisibilityDistance() + World::GetVisibleObjectGreyDistance());
    Hellground::ObjectListSearcher<Player, Hellground::AnyUnitInObjectRangeCheck> searcher(targets, check);
    Cell::VisitWorldObjects(this, searcher, GetMap()->GetVisibilityDistance() + World::GetVisibleObjectGreyDistance());

    for (std::list<Player*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
    {
        Player* player = (*iter);
        if (player != this && player->HaveAtClient(this))
        {
            DestroyForPlayer(player);
            player->m_clientGUIDs.erase(GetGUID());
        }
    }
}

void Unit::SetVisibility(UnitVisibility x)
{
    m_Visibility = x;

    switch (x)
    {
        case VISIBILITY_OFF:
        case VISIBILITY_GROUP_STEALTH:
            DestroyForNearbyPlayers();
            break;
        default:
            break;
    }

    if (IsInWorld())
        UpdateVisibilityAndView();
}

void Unit::UpdateSpeed(UnitMoveType mtype, bool forced)
{
    // not in combat pet have same speed as owner @petspeed
    switch (mtype)
    {
        case MOVE_RUN:
        case MOVE_WALK:
        case MOVE_SWIM:
            if (GetTypeId() == TYPEID_UNIT && ToCreature()->isPet() && HasUnitState(UNIT_STAT_FOLLOW) && !IsInCombat())
            {
                if (Unit* owner = GetOwner())
                    if (!owner->IsInCombat())
                    {
                        float owner_speed = owner->GetSpeedRate(mtype);
                        int32 owner_slow = owner->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_DECREASE_SPEED);
                        int32 owner_slow_non_stack = owner->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_SPEED_NOT_STACK);
                        owner_slow = owner_slow < owner_slow_non_stack ? owner_slow : owner_slow_non_stack;
                        if (owner_slow)
                            owner_speed *=100.0f/(100.0f + owner_slow); // now we have owners speed without slow
                        SetSpeed(mtype, owner_speed, forced);
                        return;
                    }
            }
            break;
        default:
            break;
    }
    
    int32 main_speed_mod  = 0;
    float stack_bonus     = 1.0f;
    float non_stack_bonus = 1.0f;

    switch (mtype)
    {
        case MOVE_WALK:
            return;
        case MOVE_RUN:
        {
            if (IsMounted()) // Use on mount auras
            {
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED);
                stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS);
                non_stack_bonus = (100.0f + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK))/100.0f;
            }
            else
            {
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_SPEED);
                stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_SPEED_ALWAYS);
                non_stack_bonus = (100.0f + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_SPEED_NOT_STACK))/100.0f;
            }
            break;
        }
        case MOVE_RUN_BACK:
            return;
        case MOVE_SWIM:
        {
            main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_SWIM_SPEED);
            break;
        }
        case MOVE_SWIM_BACK:
            return;
        case MOVE_FLIGHT:
        {
            if(HasAuraType(SPELL_AURA_MOD_SPEED_MOUNTED)) // Use for Ragged Flying Carpet & MgT Kael'thas Gravity Lapse
                main_speed_mod  = GetTotalAuraModifier(SPELL_AURA_MOD_SPEED_MOUNTED);
            else if (IsMounted()) // Use on mount auras
            {
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED);
                stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_FLIGHT_SPEED_ALWAYS);
                non_stack_bonus = (100.0 + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK))/100.0f;
            }
            else             // Use not mount (only used in flight form, swift flight form, charm of swift flight); flight forms are not afected by any other effects.
                main_speed_mod  = GetTotalAuraModifier(SPELL_AURA_MOD_SPEED_FLIGHT);
            break;
        }
        case MOVE_FLIGHT_BACK:
            return;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: Unit::UpdateSpeed: Unsupported move type (%d)", mtype);
            return;
    }

    float bonus = non_stack_bonus > stack_bonus ? non_stack_bonus : stack_bonus;

    //apply creature's base speed
    if (GetTypeId() == TYPEID_UNIT)
    {       
        // make pets to follow owner with same speed (not working because of Bestial Swiftness... GENSENTODO
        //bool set_same_speed = (ToPet() && GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE && !IsInCombat());
        //if (!set_same_speed)
        bonus *= ((Creature*)this)->GetBaseSpeed();
    }

    // set bonus speed from Bestial Swiftness
    if (GetTypeId() == TYPEID_UNIT && ToCreature()->isPet())
    {
        if (Unit* owner = GetOwner())
        {
            if (owner->HasSpell(19596) && GetTerrain()->IsOutdoors(GetPositionX(), GetPositionY(), GetPositionZ()))
                main_speed_mod += 30;
        }
    }

    // now we ready for speed calculation
    float speed  = main_speed_mod ? bonus*(100.0f + main_speed_mod)/100.0f : bonus;

    switch (mtype)
    {
        case MOVE_RUN:
        case MOVE_SWIM:
        case MOVE_FLIGHT:
        {
            // Normalize speed by 191 aura SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED if need
            // TODO: possible affect only on MOVE_RUN
            if (int32 normalization = GetMaxPositiveAuraModifier(SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED))
            {
                // Use speed from aura
                float max_speed = normalization / baseMoveSpeed[mtype];
                if (speed > max_speed)
                    speed = max_speed;
            }
            break;
        }
        default:
            break;
    }

    // for creature case, we check explicit if mob searched for assistance
    if (GetTypeId() == TYPEID_UNIT)
    {
        if (((Creature*)this)->HasSearchedAssistance())
            speed *= 0.66f;                                 // best guessed value, so this will be 33% reduction. Based off initial speed, mob can then "run", "walk fast" or "walk".
    }

    // Apply strongest slow aura mod to speed
    int32 slow = GetMaxNegativeAuraModifier(SPELL_AURA_MOD_DECREASE_SPEED);
    int32 slow_non_stack = GetMaxNegativeAuraModifier(SPELL_AURA_MOD_SPEED_NOT_STACK);
    slow = slow < slow_non_stack ? slow : slow_non_stack;
    if (slow)
        speed *=(100.0f + slow)/100.0f;

    //store max possible speed (not used)
   // m_max_speed_rate[mtype] = speed;

    SetSpeed(mtype, speed, forced);

    // update speed of pets
    Unit* charmOrPet = GetPet() ? GetPet() : GetCharm();
    if (charmOrPet)
        charmOrPet->UpdateSpeed(mtype, forced);
}

float Unit::GetSpeed(UnitMoveType mtype) const
{
    // temp fix for teleprting pet in arenas?
    //Creature* pet = (Creature*)this;
    //if (GetTypeId() == TYPEID_UNIT && pet->isPet() && mtype == 1)
    //{
    //    float speed = m_speed_rate[mtype] * baseMoveSpeed[mtype];
    //    
    //    //char textBuf[300];
    //    //sprintf(textBuf, "speed %.2f, combat %d", speed, pet->IsInCombat());      
    //    //pet->MonsterYell(textBuf, 0);

    //    // resets speed if mounted, should not?
    //    //if (pet->IsInCombat() && speed > 8.06) //8.05
    //    //{
    //    //    sLog.outLog(LOG_SPECIAL, "GetSpeed() in combat %f for pet entry %u (player %s, map %u)", speed, pet->GetEntry(), ((Player*)this)->GetOwner()->GetName(), pet->GetMapId());
    //    //    //pet->SetSpeed(mtype, 1.14999998, true);
    //    //}            
    //}
    
    return m_speed_rate[mtype]*baseMoveSpeed[mtype];
}

const Opcodes SetSpeed2Opc_table[MAX_MOVE_TYPE][3]=
{
	{ SMSG_SPLINE_SET_WALK_SPEED,        SMSG_FORCE_WALK_SPEED_CHANGE,           MSG_MOVE_SET_WALK_SPEED },
	{ SMSG_SPLINE_SET_RUN_SPEED,         SMSG_FORCE_RUN_SPEED_CHANGE,            MSG_MOVE_SET_RUN_SPEED },
	{ SMSG_SPLINE_SET_RUN_BACK_SPEED,    SMSG_FORCE_RUN_BACK_SPEED_CHANGE,       MSG_MOVE_SET_RUN_BACK_SPEED },
	{ SMSG_SPLINE_SET_SWIM_SPEED,        SMSG_FORCE_SWIM_SPEED_CHANGE,           MSG_MOVE_SET_SWIM_SPEED },
	{ SMSG_SPLINE_SET_SWIM_BACK_SPEED,   SMSG_FORCE_SWIM_BACK_SPEED_CHANGE,      MSG_MOVE_SET_SWIM_BACK_SPEED },
	{ SMSG_SPLINE_SET_TURN_RATE,         SMSG_FORCE_TURN_RATE_CHANGE,            MSG_MOVE_SET_TURN_RATE },
	{ SMSG_SPLINE_SET_FLIGHT_SPEED,      SMSG_FORCE_FLIGHT_SPEED_CHANGE,         MSG_MOVE_SET_FLIGHT_SPEED },
	{ SMSG_SPLINE_SET_FLIGHT_BACK_SPEED, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE,    MSG_MOVE_SET_FLIGHT_BACK_SPEED },
};

// old, causing bugs with movements, for example Kiggler will wall
//void Unit::SetSpeed(UnitMoveType mtype, float rate, bool forced /* not used */)
//{
//    if (rate < 0)
//        rate = 0.0f;
//
//    // Update speed only on change
//    if (m_speed_rate[mtype] == rate)
//        return;
//
//    m_speed_rate[mtype] = rate;
//    propagateSpeedChange();
//
//    if (ToPlayer() && IsInWorld()) // ToPlayer() meaning IsMovedByPlayer()
//    {
//        SendSpeedChangeToMover(this, mtype, rate);
//    }       
//    
//    SendSpeedChangeToObservers(this, mtype, rate);
//}

//void Unit::SetSpeed(UnitMoveType mtype, float rate, bool forced)
//{
//    if (rate < 0)
//        rate = 0.0f;
//
//    // Update speed only on change
//    if (m_speed_rate[mtype] == rate)
//        return;
//
//    m_speed_rate[mtype] = rate;
//    propagateSpeedChange();
//
//    if (forced)
//    {
//        if (GetTypeId() == TYPEID_PLAYER && IsInWorld())
//            SendSpeedChangeToMover(this, mtype, rate);
//
//        SendSpeedChangeToObservers(this, mtype, rate);
//    }
//    else
//    {
//        SendSpeedChangeToObservers(this, mtype, rate);
//    }
//}

void Unit::SetSpeed(UnitMoveType mtype, float rate, bool forced)
{
    rate = std::max(rate, 0.01f);

    // Update speed only on change
    if (m_speed_rate[mtype] == rate)
        return;

    m_speed_rate[mtype] = rate;
    propagateSpeedChange();

    if (!IsInWorld())
        return;

    if (forced && GetTypeId() == TYPEID_PLAYER)
    {
        SendSpeedChangeToMover(this, mtype, rate);
        SendSpeedChangeToObservers(this, mtype, rate);
    }
    else
    {
        SendSpeedChangeToAll(this, mtype, GetSpeed(mtype));
    }
}

void Unit::SendSpeedChangeToMover(Unit* unit, UnitMoveType mtype, float rate)
{
    // register forced speed changes for WorldSession::HandleForceSpeedChangeAck
    // and do it only for real sent packets and use run for run/mounted as client expected

    ++((Player*)this)->m_forced_speed_changes[mtype];

    WorldPacket data(SetSpeed2Opc_table[mtype][1], 18);
    data << GetPackGUID();
    data << (uint32)0;                                  // moveEvent, NUM_PMOVE_EVTS = 0x39 (PushPendingMovementChange?)
    if (mtype == MOVE_RUN)
        data << uint8(0);                               // new 2.1.0
    data << float(GetSpeed(mtype));
    
    ((Player*)this)->SendPacketToSelf(&data);
}

void Unit::SendSpeedChangeToObservers(Unit* unit, UnitMoveType mtype, float rate)
{
    m_movementInfo.UpdateTime(WorldTimer::getMSTime());
    WorldPacket data(SetSpeed2Opc_table[mtype][2], 64);
    data << GetPackGUID();
    data << m_movementInfo;
    data << float(GetSpeed(mtype));
    BroadcastPacketExcept(&data, (Player*)this);
}

void Unit::SendSpeedChangeToAll(Unit* unit, UnitMoveType mtype, float rate)
{
    WorldPacket data;
    data.Initialize(SetSpeed2Opc_table[mtype][0], 8 + 4);
    data << unit->GetPackGUID();
    data << rate;
    BroadcastPacket(&data, true);
}

// fix random freezes? https://github.com/cmangos/mangos-tbc/commit/0b990d8297286047b3419459bc6f3e9eb4c9606f
void Unit::SendRootedToMover(Unit* unit, bool apply)
{
    WorldPacket data(apply ? SMSG_FORCE_MOVE_ROOT : SMSG_FORCE_MOVE_UNROOT, unit->GetPackGUID().size() + 4);
    data << unit->GetPackGUID();
    data << unit->GetUnitStateMgr().GetCounter(UNIT_ACTION_ROOT);
    // was sending only for ROOT, not STUN, but it is the same states O_O
    // i guess this should be send only is Unit is a player
    //data << uint32(0); 
    ((Player*)unit)->GetSession()->SendPacket(&data);
}

void Unit::SendRootedToObservers(Unit* unit, bool apply)
{
    // BUGGED!
    // using stun -> fear making target to teleporting instead of fleeing
    // look at WorldSession::HandleMoveRootAck mangos implementation... but it needs to implement packet ordering too...
    
    m_movementInfo.UpdateTime(WorldTimer::getMSTime());

    WorldPacket data(apply ? MSG_MOVE_ROOT : MSG_MOVE_UNROOT, unit->GetPackGUID().size() + 4);
    data << unit->GetPackGUID();
    data << m_movementInfo;
    //data << uint32(0);
    unit->BroadcastPacketExcept(&data, (Player*)unit);
}

void Unit::SendRootedToAll(Unit* unit, bool apply)
{
    WorldPacket data(apply ? SMSG_SPLINE_MOVE_ROOT : SMSG_SPLINE_MOVE_UNROOT, unit->GetPackGUID().size());
    data << unit->GetPackGUID();
    //data << uint32(0);
    unit->BroadcastPacket(&data, true);
}

void Unit::setDeathState(DeathState s)
{
	m_deathState = s;
	
	if (s != ALIVE && s != JUST_ALIVED)
    {
        CombatStop();
        DeleteThreatList();
        getHostileRefManager().deleteReferences();
        ClearComboPointHolders();                           // any combo points pointed to unit lost at it death

        if (IsNonMeleeSpellCast(false))
            InterruptNonMeleeSpells(false);
    }

    if (s == JUST_DIED)
    {
        RemoveAllAurasOnDeath();
        UnsummonAllTotems();

        ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        ModifyAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, false);

        // remove aura states allowing special moves
        ClearAllReactives();
        ClearDiminishings();

        GetUnitStateMgr().InitDefaults(false);

        StopMoving();

        //without this when removing IncreaseMaxHealth aura player may stuck with 1 hp
        //do not why since in IncreaseMaxHealth current health is checked
        SetHealth(0);
    }
    else if (s == JUST_ALIVED)
    {
        RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE); // clear skinnable for creature and player (at battleground)
    }
    else if (s == DEAD || s == CORPSE)
    {
        GetUnitStateMgr().DropAllStates();
    }
}

/*########################################
########                          ########
########       AGGRO SYSTEM       ########
########                          ########
########################################*/
bool Unit::CanHaveThreatList() const
{
    // only creatures can have threat list
    if (GetTypeId() != TYPEID_UNIT)
        return false;

    // only alive units can have threat list
    if (!isAlive() || isDying())
        return false;

    // totems can not have threat list
    if (isCharmedOwnedByPlayerOrPlayer())
        return false;

    return true;
}

//======================================================================

float Unit::ApplyTotalThreatModifier(float threat, SpellSchoolMask schoolMask)
{
    if (!HasAuraType(SPELL_AURA_MOD_THREAT))
        return threat;

    if (schoolMask == SPELL_SCHOOL_MASK_NONE)
        return threat;

    SpellSchools school = GetFirstSchoolInMask(schoolMask);

	// hack fix, need to look further
	if (GetTypeId() == TYPEID_PLAYER && GetClass() == CLASS_PALADIN && school == SPELL_SCHOOL_HOLY && m_threatModifier[school] > 2.0f)
	{
		// PALADIN_THREATBUG
		//sLog.outLog(LOG_SPECIAL, "Paladin %s threat bug #ApplyTotalThreatModifier! Threat %f, modifier %f", GetName(), threat, m_threatModifier[SPELL_SCHOOL_HOLY]);
		return threat * 2.0f;
	}

    return threat * m_threatModifier[school];
}

//======================================================================

void Unit::AddThreat(Unit* pVictim, float threat, SpellSchoolMask schoolMask, SpellEntry const *threatSpell)
{
    // Only mobs can manage threat lists
    if (CanHaveThreatList())
        getThreatManager().addThreat(pVictim, threat, schoolMask, threatSpell);
}

//======================================================================

void Unit::DeleteThreatList()
{
    getThreatManager().clearReferences();
}

//======================================================================

void Unit::TauntApply(Unit* taunter)
{
    ASSERT(GetTypeId() == TYPEID_UNIT);

    if (!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((Player*)taunter)->isGameMaster()))
        return;

    if (!CanHaveThreatList() && !((Creature*)this)->isPet())
        return;

    Unit *target = GetVictim();
    if (target && target == taunter)
        return;

    SetInFront(taunter);
    if (((Creature*)this)->IsAIEnabled)
        ((Creature*)this)->AI()->AttackStart(taunter);

    //m_ThreatManager.tauntApply(taunter);
}

//======================================================================

void Unit::TauntFadeOut(Unit *taunter)
{
    ASSERT(GetTypeId()== TYPEID_UNIT);

    if (!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((Player*)taunter)->isGameMaster()))
        return;

    if (!CanHaveThreatList())
        return;

    Unit *target = GetVictim();
    if (!target || target != taunter)
        return;

    if (getThreatManager().isThreatListEmpty())
    {
        if (((Creature*)this)->IsAIEnabled)
            ((Creature*)this)->AI()->EnterEvadeMode();
        
        if (InstanceData* mapInstance = GetInstanceData())
            mapInstance->OnCreatureEvade((Creature*)this);
        return;
    }

    //m_ThreatManager.tauntFadeOut(taunter);
    target = getThreatManager().getHostilTarget();

    if (target && target != taunter)
    {
        SetInFront(target);
        if (((Creature*)this)->IsAIEnabled)
            ((Creature*)this)->AI()->AttackStart(target);
    }
}

//======================================================================

Unit* Creature::SelectVictim()
{
    //function provides main threat functionality
    //next-victim-selection algorithm and evade mode are called
    //threat list sorting etc.

    //This should not be called by unit who does not have a threatlist
    //or who does not have threat (totem/pet/critter)
    //otherwise enterevademode every update


    if (IsInEvadeMode())
    {
        if (!m_attackers.empty())
            RemoveAllAttackers();

        if (!getThreatManager().isThreatListEmpty())
            DeleteThreatList();

        return NULL;
    }

    Unit* target = NULL;
    if (CanHaveThreatList())
    {
        if (!getThreatManager().isThreatListEmpty())
        {
            if (!HasAuraType(SPELL_AURA_MOD_TAUNT))
                target = getThreatManager().getHostilTarget();
            else
                return GetVictim();
        }
    }

    if (IsOutOfThreatArea(target))
        target = NULL;

    if (target)
    {
        if (!HasUnitState(UNIT_STAT_STUNNED | UNIT_STAT_CANNOT_TURN))
            SetInFront(target);

        return target;
    }

    for (AttackerSet::const_iterator itr = m_attackers.begin(); itr != m_attackers.end(); ++itr)
    {
        Unit* attacker = (*itr);
        if (!IsOutOfThreatArea(attacker))
            return attacker;
    }

    // search nearby enemy before enter evade mode
    if (HasReactState(REACT_AGGRESSIVE))
    {
        target = SelectNearestTarget(25.0f);
        if (target && !IsOutOfThreatArea(target))
            return target;
    }

    if (m_invisibilityMask)
    {
        Unit::AuraList const& iAuras = GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for (Unit::AuraList::const_iterator itr = iAuras.begin(); itr != iAuras.end(); ++itr)
        {
            if ((*itr)->IsPermanent())
            {
                if (m_attackers.size())
                    return NULL;

                AI()->EnterEvadeMode();
                break;
            }
        }
        return NULL;
    }

    // enter in evade mode in other case
    AI()->EnterEvadeMode();
    if (InstanceData* mapInstance = GetInstanceData())
        mapInstance->OnCreatureEvade((Creature*)this);

    return NULL;
}

//======================================================================
//======================================================================
//======================================================================

int32 Unit::CalculateSpellDamage(SpellEntry const* spellProto, uint8 effect_index, int32 effBasePoints, bool spendCharges)
{
    Player* unitPlayer = (GetTypeId() == TYPEID_PLAYER) ? (Player*)this : NULL;

    uint8 comboPoints = unitPlayer ? unitPlayer->GetComboPoints() : 0;

    int32 level = int32(GetLevel());
    if (level > (int32)spellProto->maxLevel && spellProto->maxLevel > 0)
        level = (int32)spellProto->maxLevel;
    else if (level < (int32)spellProto->baseLevel)
        level = (int32)spellProto->baseLevel;

    //if (!SpellMgr::IsPassiveSpell(spellProto))
    // oregon has that, but it also increases AP gained from druid passive from forms, which it shouldn't if compared bear form and dire bear form
    level-= (int32)spellProto->spellLevel;

    float basePointsPerLevel = spellProto->EffectRealPointsPerLevel[effect_index];
    int32 basePoints = int32(effBasePoints + level * basePointsPerLevel);
    float comboDamage = spellProto->EffectPointsPerComboPoint[effect_index];

    int32 baseRandomPoints = int32(spellProto->EffectBaseDice[effect_index]);
    int32 randomPoints = spellProto->EffectDieSides[effect_index];
    randomPoints += int32(spellProto->EffectDicePerLevel[effect_index] * float(level));

    // prevent random generator from getting confused by spells cast with Unit::CastCustomSpell
    int32 randvalue = baseRandomPoints >= randomPoints ? irand(randomPoints, baseRandomPoints) : irand(baseRandomPoints, randomPoints);

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_DPS_TESTING)
        randvalue = baseRandomPoints >= randomPoints ? round((randomPoints+baseRandomPoints)/2) : round((baseRandomPoints+randomPoints)/2);

    int32 value = basePoints + randvalue;

    // hacky formula for lowlvl spells with high spelldmg after spelllevel calc. Dmg restricted to (10+lvl)%
    /*if (GetLevel() <= 20 && GetObjectGuid().IsCreature() && value > int32(GetMaxHealth()*(GetLevel()*0.01f + 0.1f)) &&
        !ToCreature()->isTrigger() && GetEntry() != WORLD_TRIGGER && !(GetOwner() && GetOwner()->GetTypeId() == TYPEID_PLAYER))
    {
        sLog.outLog(LOG_DB_ERR, "ERROR: Error in CalculateSpellDamage. Lowlevelcreature's spell damage is too big. (HG fix logging). Value of spell damage: %u, creature ID: %u.", value, GetEntry());
        value = int32(GetMaxHealth()*(GetLevel()*0.01f + 0.1f));
    }*/ // no need after new formula implementation

    //random damage
    if (comboDamage != 0 && unitPlayer /*&& target && (target->GetGUID() == unitPlayer->GetComboTarget())*/)
        value += (int32)(comboDamage * comboPoints);

    if (Player* modOwner = GetSpellModOwner())
    {
        modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_ALL_EFFECTS, value, NULL, spendCharges);
        switch (effect_index)
        {
            case 0:
                modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT1, value, NULL, spendCharges);
                break;
            case 1:
                modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT2, value, NULL, spendCharges);
                break;
            case 2:
                modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT3, value, NULL, spendCharges);
                break;
            default:
                break;
        }
    }

    if (!basePointsPerLevel && SpellMgr::EffectCanScaleWithLevel(spellProto, effect_index))
    {
        float thisLevel = float(GetLevel());

        // because 60 lvl heroics used by 70 lvl players, it was scaled... but we don't really need this tho
        //if (GetObjectGuid().IsCreature())
        //{
        //    if (((Creature*)this)->ModIsInHeroicRaid && 
        //        ((Creature*)this)->GetHeroicRaid60DependentMod(((Creature*)this)->GetMapId()) > 0 && 
        //        thisLevel > 10/*just so it doesnt go negative*/) // this means its 60 heroic raid
        //        thisLevel -= 10;
        //}

        GtNPCManaCostScalerEntry const* spellScaler = sGtNPCManaCostScalerStore.LookupEntry(spellProto->spellLevel - 1);
        GtNPCManaCostScalerEntry const* casterScaler = sGtNPCManaCostScalerStore.LookupEntry(thisLevel - 1);
        if (spellScaler && casterScaler)
            value *= casterScaler->ratio / spellScaler->ratio;        
    }
    return value;

}

int32 Unit::CalculateSpellDuration(SpellEntry const* spellProto, uint8 effect_index, Unit const* target)
{
    Player* unitPlayer = (GetTypeId() == TYPEID_PLAYER) ? (Player*)this : NULL;

    uint8 comboPoints = unitPlayer ? unitPlayer->GetComboPoints() : 0;

    int32 minduration = SpellMgr::GetSpellDuration(spellProto);
    int32 maxduration = SpellMgr::GetSpellMaxDuration(spellProto);

    int32 duration;

    if (minduration != -1 && minduration != maxduration)
        duration = minduration + int32((maxduration - minduration) * comboPoints / 5);
    else
        duration = minduration;

    // apply overall bonus
    if (duration > 0)
    {
        // Duration of crowd control abilities on pvp target is limited by 10 sec. First do this then apply mods
        Player* targetPlayer = target->GetCharmerOrOwnerOrSelf()->ToPlayer();
        unitPlayer = GetCharmerOrOwnerOrSelf()->ToPlayer();
        if (unitPlayer && targetPlayer && duration > 10000 && !SpellMgr::IsPositiveSpell(spellProto->Id) &&
            (target !=this || !SpellMgr::IsChanneledSpell(spellProto) )&&
            SpellMgr::IsDiminishingReturnsGroupDurationLimited(SpellMgr::GetDiminishingReturnsGroupForSpell(spellProto,false)))
            // isDimnishing...(getdimnishing...(SpellEntry*,bool triggered)) does not depend on triggered value
        {
            duration = 10000;
            if (spellProto->SpellFamilyName == SPELLFAMILY_WARLOCK && spellProto->SpellFamilyFlags & 0x80000000LL) // Curse of Tongues
                duration = 12000;
        }

        if (int32 mechanic = SpellMgr::GetSpellMechanic(spellProto))
        {
            // Find total mod value (negative bonus)
            int32 durationMod_always = target->GetTotalAuraModifierByMiscValue(SPELL_AURA_MECHANIC_DURATION_MOD, mechanic);
            // Find max mod (negative bonus)
            int32 durationMod_not_stack = target->GetMaxNegativeAuraModifierByMiscValue(SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK, mechanic);

            int32 durationMod = 0;
            // Select strongest negative mod
            if (durationMod_always > durationMod_not_stack)
                durationMod = durationMod_not_stack;
            else
                durationMod = durationMod_always;

            if (durationMod != 0)
                duration = int32(int64(duration) * (100+durationMod) /100);

            if (duration < 0) duration = 0;
        }


        // THIS IS UNDER QUESTION - Should we apply second mechanic duration decrease? Such spells are very rare anyway.
        // apply effect bonus, if needed. (there is no double effect from same mechanic)
        if (duration > 0)
        {
            if (int32 mechanic = SpellMgr::GetEffectMechanic(spellProto, effect_index))
            {
                // Find total mod value (negative bonus)
                int32 durationMod_always = target->GetTotalAuraModifierByMiscValue(SPELL_AURA_MECHANIC_DURATION_MOD, mechanic);
                // Find max mod (negative bonus)
                int32 durationMod_not_stack = target->GetMaxNegativeAuraModifierByMiscValue(SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK, mechanic);

                int32 durationMod = 0;
                // Select strongest negative mod
                if (durationMod_always > durationMod_not_stack)
                    durationMod = durationMod_not_stack;
                else
                    durationMod = durationMod_always;

                if (durationMod != 0)
                    duration = int32(int64(duration) * (100+durationMod) /100);

                if (duration < 0) duration = 0;
            }
        }
    }

    return duration;
}

DiminishingLevels Unit::GetDiminishing(DiminishingGroup group)
{
    for (Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if (i->DRGroup != group)
            continue;

        if (!i->hitCount)
            return DIMINISHING_LEVEL_1;

        if (!i->hitTime)
            return DIMINISHING_LEVEL_1;

        if(group == DIMINISHING_ENSLAVE)
            return DiminishingLevels(i->hitCount);

        // If last spell was cast more than 15 seconds ago - reset the count.
        // blizzlike 15-20 second, gives more RNG to pvp, so it's bad
        // from 15 sec some classes benefits, so make it 17.5 sec
        if (i->stack==0 && WorldTimer::getMSTimeDiff(i->hitTime,WorldTimer::getMSTime()) > 17500)
        {
            i->hitCount = DIMINISHING_LEVEL_1;
            return DIMINISHING_LEVEL_1;
        }
        // or else increase the count.
        else
        {
            return DiminishingLevels(i->hitCount);
        }
    }
    return DIMINISHING_LEVEL_1;
}

void Unit::IncrDiminishing(DiminishingGroup group)
{
    // Checking for existing in the table
    for (Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if (i->DRGroup != group)
            continue;

        if (i->hitCount < DIMINISHING_LEVEL_IMMUNE)
            ++i->hitCount;

        return;
    }

    m_Diminishing.push_back(DiminishingReturn(group,WorldTimer::getMSTime(),DIMINISHING_LEVEL_2));
}

void Unit::DecrDiminishing(DiminishingGroup group)
{
    for (Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if (i->DRGroup != group)
            continue;

        if (i->hitCount)
            --i->hitCount;

        return;
    }
}

void Unit::ApplyDiminishingToDuration(DiminishingGroup group, int32 &duration,Unit* /*caster*/,DiminishingLevels Level, SpellEntry const *spellInfo)
{
    if (duration == -1 || group == DIMINISHING_NONE)/*(caster->IsFriendlyTo(this) && caster != this)*/
        return;

    // test pet/charm master
    Unit const* targetOwner = GetCharmerOrOwner();

    float mod = 1.0f;
                                                                                                                                    // Freezing trap exception, since it is cast by GO ?
    if ((SpellMgr::GetDiminishingReturnsGroupType(group) == DRTYPE_PLAYER &&
        (targetOwner ? targetOwner->GetTypeId() : GetTypeId())  == TYPEID_PLAYER) ||
        SpellMgr::GetDiminishingReturnsGroupType(group) == DRTYPE_ALL ||
        (spellInfo && spellInfo ->SpellFamilyName == SPELLFAMILY_HUNTER && spellInfo ->SpellFamilyFlags & 0x00000000008LL))
    {
        DiminishingLevels diminish = Level;
        switch (diminish)
        {
            case DIMINISHING_LEVEL_1: break;
            case DIMINISHING_LEVEL_2: mod = 0.5f; break;
            case DIMINISHING_LEVEL_3: mod = 0.25f; break;
            case DIMINISHING_LEVEL_IMMUNE: mod = 0.0f;break;
            default: break;
        }
    }

    duration = int32(duration * mod);
}

void Unit::ApplyDiminishingAura(DiminishingGroup group, bool apply)
{
    // Checking for existing in the table
    for (Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if (i->DRGroup != group)
            continue;

        i->hitTime = WorldTimer::getMSTime();

        if (apply)
            i->stack += 1;
        else if (i->stack)
            i->stack -= 1;

        break;
    }
}

Unit* Unit::GetUnit(WorldObject& object, uint64 guid)
{
    return object.GetMap()->GetUnit(guid);
}

Unit* Unit::GetUnit(const Unit& unit, uint64 guid)
{
    return unit.GetMap()->GetUnit(guid);
}

Unit* Unit::GetUnit(uint64 guid)
{
    return GetMap()->GetUnit(guid);
}

Player* Unit::GetPlayerInWorld(uint64 guid)
{
    return ObjectAccessor::GetPlayerInWorld(guid);
}

Creature* Unit::GetCreature(WorldObject& object, uint64 guid)
{
    return object.GetMap()->GetCreature(guid);
}

Creature* Unit::GetCreature(uint64 guid)
{
    return GetMap()->GetCreature(guid);
}

Player* Unit::GetPlayerByName(const char *name)
{
    return sObjectMgr.GetPlayerInWorld(name);
}

bool Unit::isVisibleForInState(Player const* player, WorldObject const* viewPoint, bool inVisibleList) const
{
    return isVisibleForOrDetect(player, viewPoint, false, inVisibleList, false);
}

uint32 Unit::GetCreatureType() const
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        SpellShapeshiftEntry const* ssEntry = sSpellShapeshiftStore.LookupEntry(m_form);
        if (ssEntry && ssEntry->creatureType > 0)
            return ssEntry->creatureType;
        else
            return CREATURE_TYPE_HUMANOID;
    }
    else
        return ((Creature*)this)->GetCreatureInfo()->type;
}

/*#######################################
########                         ########
########       STAT SYSTEM       ########
########                         ########
#######################################*/

bool Unit::HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply)
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ERROR in HandleStatModifier(): non existed UnitMods or wrong UnitModifierType!");
        return false;
    }

    float val = 1.0f;

    switch (modifierType)
    {
        case BASE_VALUE:
        case TOTAL_VALUE:
            m_auraModifiersGroup[unitMod][modifierType] += apply ? amount : -amount;
            break;
        case BASE_PCT:
        case TOTAL_PCT:
            if (amount <= -100.0f)                           //small hack-fix for -100% modifiers
                amount = -200.0f;

            val = (100.0f + amount) / 100.0f;
            m_auraModifiersGroup[unitMod][modifierType] *= apply ? val : (1.0f/val);
            break;

        default:
            break;
    }

    if (!CanModifyStats())
        return false;

    switch (unitMod)
    {
        case UNIT_MOD_STAT_STRENGTH:
        case UNIT_MOD_STAT_AGILITY:
        case UNIT_MOD_STAT_STAMINA:
        case UNIT_MOD_STAT_INTELLECT:
        case UNIT_MOD_STAT_SPIRIT:         UpdateStats(GetStatByAuraGroup(unitMod));  break;

        case UNIT_MOD_ARMOR:               UpdateArmor();           break;
        case UNIT_MOD_HEALTH:              UpdateMaxHealth();       break;

        case UNIT_MOD_MANA:
        case UNIT_MOD_RAGE:
        case UNIT_MOD_FOCUS:
        case UNIT_MOD_ENERGY:
        case UNIT_MOD_HAPPINESS:           UpdateMaxPower(GetPowerTypeByAuraGroup(unitMod));         break;

        case UNIT_MOD_RESISTANCE_HOLY:
        case UNIT_MOD_RESISTANCE_FIRE:
        case UNIT_MOD_RESISTANCE_NATURE:
        case UNIT_MOD_RESISTANCE_FROST:
        case UNIT_MOD_RESISTANCE_SHADOW:
        case UNIT_MOD_RESISTANCE_ARCANE:   UpdateResistances(GetSpellSchoolByAuraGroup(unitMod));      break;

        case UNIT_MOD_ATTACK_POWER:        UpdateAttackPowerAndDamage();         break;
        case UNIT_MOD_ATTACK_POWER_RANGED: UpdateAttackPowerAndDamage(true);     break;

        case UNIT_MOD_DAMAGE_MAINHAND:     UpdateDamagePhysical(BASE_ATTACK);    break;
        case UNIT_MOD_DAMAGE_OFFHAND:      UpdateDamagePhysical(OFF_ATTACK);     break;
        case UNIT_MOD_DAMAGE_RANGED:       UpdateDamagePhysical(RANGED_ATTACK);  break;

        default:
            break;
    }

    return true;
}

float Unit::GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: trial to access non existed modifier value from UnitMods!");
        return 0.0f;
    }

    if (modifierType == TOTAL_PCT && m_auraModifiersGroup[unitMod][modifierType] <= 0.0f)
        return 0.0f;

    return m_auraModifiersGroup[unitMod][modifierType];
}

float Unit::GetTotalStatValue(Stats stat) const
{
    UnitMods unitMod = UnitMods(UNIT_MOD_STAT_START + stat);

    if (m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = m_auraModifiersGroup[unitMod][BASE_VALUE] + GetCreateStat(stat);
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

float Unit::GetTotalAuraModValue(UnitMods unitMod) const
{
    if (unitMod >= UNIT_MOD_END)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: trial to access non existed UnitMods in GetTotalAuraModValue()!");
        return 0.0f;
    }

    if (m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    float value  = m_auraModifiersGroup[unitMod][BASE_VALUE];
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

SpellSchools Unit::GetSpellSchoolByAuraGroup(UnitMods unitMod) const
{
    SpellSchools school = SPELL_SCHOOL_NORMAL;

    switch (unitMod)
    {
        case UNIT_MOD_RESISTANCE_HOLY:     school = SPELL_SCHOOL_HOLY;          break;
        case UNIT_MOD_RESISTANCE_FIRE:     school = SPELL_SCHOOL_FIRE;          break;
        case UNIT_MOD_RESISTANCE_NATURE:   school = SPELL_SCHOOL_NATURE;        break;
        case UNIT_MOD_RESISTANCE_FROST:    school = SPELL_SCHOOL_FROST;         break;
        case UNIT_MOD_RESISTANCE_SHADOW:   school = SPELL_SCHOOL_SHADOW;        break;
        case UNIT_MOD_RESISTANCE_ARCANE:   school = SPELL_SCHOOL_ARCANE;        break;

        default:
            break;
    }

    return school;
}

Stats Unit::GetStatByAuraGroup(UnitMods unitMod) const
{
    Stats stat = STAT_STRENGTH;

    switch (unitMod)
    {
        case UNIT_MOD_STAT_STRENGTH:    stat = STAT_STRENGTH;      break;
        case UNIT_MOD_STAT_AGILITY:     stat = STAT_AGILITY;       break;
        case UNIT_MOD_STAT_STAMINA:     stat = STAT_STAMINA;       break;
        case UNIT_MOD_STAT_INTELLECT:   stat = STAT_INTELLECT;     break;
        case UNIT_MOD_STAT_SPIRIT:      stat = STAT_SPIRIT;        break;

        default:
            break;
    }

    return stat;
}

Powers Unit::GetPowerTypeByAuraGroup(UnitMods unitMod) const
{
    Powers power = POWER_MANA;

    switch (unitMod)
    {
        case UNIT_MOD_MANA:       power = POWER_MANA;       break;
        case UNIT_MOD_RAGE:       power = POWER_RAGE;       break;
        case UNIT_MOD_FOCUS:      power = POWER_FOCUS;      break;
        case UNIT_MOD_ENERGY:     power = POWER_ENERGY;     break;
        case UNIT_MOD_HAPPINESS:  power = POWER_HAPPINESS;  break;

        default:
            break;
    }

    return power;
}

float Unit::GetTotalAttackPowerValue(WeaponAttackType attType) const
{
    if (attType == RANGED_ATTACK)
    {
        float ap = (1.0f + GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER)) * float(GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) + GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS));
        return ap < 0.0f ? 0.0f : ap ;
    }
    else
    {
        float ap = (1.0f + GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER)) * float(GetInt32Value(UNIT_FIELD_ATTACK_POWER) + GetInt32Value(UNIT_FIELD_ATTACK_POWER_MODS));
        return ap < 0.0f ? 0.0f : ap ;
    }
}

float Unit::GetWeaponDamageRange(WeaponAttackType attType ,WeaponDamageRange type) const
{
    if (attType == OFF_ATTACK && !haveOffhandWeapon())
        return 0.0f;

    return m_weaponDamage[attType][type];
}

void Unit::SetLevel(uint32 lvl)
{
    SetUInt32Value(UNIT_FIELD_LEVEL, lvl);

    // group update
    if ((GetTypeId() == TYPEID_PLAYER) && ((Player*)this)->GetGroup())
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_LEVEL);
}

void Unit::SetHealth(uint32 val, bool ignoreAliveCheck)
{
    if (!ignoreAliveCheck && !isAlive())
        val = 0;
    else
    {
        uint32 maxHealth = GetMaxHealth();
        if (maxHealth < val)
            val = maxHealth;
    }

    SetUInt32Value(UNIT_FIELD_HEALTH, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_HP);
    }
    else if (((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit *owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_HP);
        }
    }
}

void Unit::SetMaxHealth(uint32 val)
{
    if (!val) val = 1;
    uint32 health = GetHealth();
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_HP);
    }
    else if (((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit *owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_HP);
        }
    }

    if (val < health)
        SetHealth(val);
}

void Unit::SetPower(Powers power, uint32 val)
{
    if (GetPower(power) == val)
        return;

    uint32 maxPower = GetMaxPower(power);
    if (maxPower < val)
        val = maxPower;

    SetStatInt32Value(UNIT_FIELD_POWER1 + power, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_POWER);
    }
    else if (((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit *owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
        }

        // Update the pet's character sheet with happiness damage bonus
        if (pet->getPetType() == HUNTER_PET && power == POWER_HAPPINESS)
        {
            if (sWorld.isEasyRealm())
            {
                SetPower(POWER_HAPPINESS, GetMaxPower(POWER_HAPPINESS)); // Pets are ALWAYS happy
            }
            
            pet->UpdateDamagePhysical(BASE_ATTACK);
        }
    }
}

void Unit::SetMaxPower(Powers power, uint32 val)
{
    uint32 cur_power = GetPower(power);
    SetStatInt32Value(UNIT_FIELD_MAXPOWER1 + power, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_POWER);
    }
    else if (((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit *owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_POWER);
        }
    }

    if (val < cur_power)
        SetPower(power, val);
}

void Unit::ApplyPowerMod(Powers power, uint32 val, bool apply)
{
    ApplyModUInt32Value(UNIT_FIELD_POWER1+power, val, apply);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_POWER);
    }
    else if (((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit *owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
        }
    }
}

void Unit::ApplyAuraProcTriggerDamage(Aura* aura, bool apply)
{
    AuraList& tAuraProcTriggerDamage = m_modAuras[SPELL_AURA_PROC_TRIGGER_DAMAGE];
    if (apply)
        tAuraProcTriggerDamage.push_back(aura);
    else
        tAuraProcTriggerDamage.remove(aura);
}

uint32 Unit::GetCreatePowers(Powers power) const
{
    // POWER_FOCUS and POWER_HAPPINESS only have hunter pet
    switch (power)
    {
        case POWER_MANA:      return GetCreateMana();
        case POWER_RAGE:      return 1000;
        case POWER_FOCUS:     return (GetTypeId()==TYPEID_PLAYER || !((Creature const*)this)->isPet() || ((Pet const*)this)->getPetType()!=HUNTER_PET ? 0 : 100);
        case POWER_ENERGY:    return 100;
        case POWER_HAPPINESS: return (GetTypeId()==TYPEID_PLAYER || !((Creature const*)this)->isPet() || ((Pet const*)this)->getPetType()!=HUNTER_PET ? 0 : 1050000);
    }

    return 0;
}

void Unit::AddToWorld()
{
    if (!IsInWorld())
    {
        WorldObject::AddToWorld();
        debug_info = { GetGUIDLow(), GetEntry(), GetName() };
    }       
}

void Unit::setHover(bool val)
{
    WorldPacket data;
    if (val)
        data.Initialize(SMSG_MOVE_SET_HOVER, 8+4);
    else
        data.Initialize(SMSG_MOVE_UNSET_HOVER, 8+4);

    data << GetPackGUID();
    data << uint32(0);
    BroadcastPacket(&data, true);
}

void Unit::RemoveFromWorld()
{
    // cleanup
    if (IsInWorld())
    {
        CombatStop(true);
        getHostileRefManager().deleteReferences();

        RemoveBindSightAuras();
        RemoveNotOwnSingleTargetAuras();

		// Remove non-guardian pet
		if (Pet* pet = GetPet())
			pet->Remove(PET_SAVE_AS_DELETED);

        GetViewPoint().Event_RemovedFromWorld();

        WorldObject::RemoveFromWorld();
    }
}

void Unit::CleanupsBeforeDelete()
{
    if (IsInWorld())
        RemoveFromWorld();

    if (m_uint32Values)
    {
        //A unit may be in remove list and not in world, but it is still in grid
        //and may have some references during delete
        if (isAlive() && HasAuraType(SPELL_AURA_AOE_CHARM))
            Kill(this,true);

        RemoveAllAuras();
        InterruptNonMeleeSpells(true);
        KillAllEvents(false);                      // non-delatable (currently cast spells) will not deleted now but it will deleted at call in Map::RemoveAllObjectsInRemoveList
        CombatStop();
        ClearComboPointHolders();
        DeleteThreatList();
        getHostileRefManager().setOnlineOfflineState(false);
        RemoveAllGameObjects();
        RemoveAllDynObjects();

        GetUnitStateMgr().InitDefaults(false);
    }
}

void Unit::UpdateCharmAI()
{
    if (GetTypeId() == TYPEID_PLAYER)
        return;

    if (i_disabledAI) // disabled AI must be primary AI
    {
        if (!isCharmed() || GetEntry() == 24922)    // allow Razorthorn Ravager to switch to CreatureAI when charmed
        {
            delete i_AI;

            i_AI = i_disabledAI;
            i_disabledAI = NULL;
        }
    }
    else
    {
        if (isCharmed())
        {
            i_disabledAI = i_AI;

            if (isPossessed())
                i_AI = new PossessedAI(ToCreature());
            else
                i_AI = new PetAI(ToCreature());
        }
    }
}

CharmInfo* Unit::InitCharmInfo()
{
    if (!m_charmInfo)
        m_charmInfo = new CharmInfo(this);

    return m_charmInfo;
}

void Unit::DeleteCharmInfo()
{
    if (!m_charmInfo)
        return;

    delete m_charmInfo;
    m_charmInfo = NULL;
}

bool Unit::isFrozen() const
{
    AuraList const& mRoot = GetAurasByType(SPELL_AURA_MOD_ROOT);
    for (AuraList::const_iterator i = mRoot.begin(); i != mRoot.end(); ++i)
        if (SpellMgr::GetSpellSchoolMask((*i)->GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
            return true;
    AuraList const& mStun = GetAurasByType(SPELL_AURA_MOD_STUN);
    for (AuraList::const_iterator i = mStun.begin(); i != mStun.end(); ++i)
        if (SpellMgr::GetSpellSchoolMask((*i)->GetSpellProto()) & SPELL_SCHOOL_MASK_FROST)
            return true;
    return false;
}

struct ProcTriggeredData
{
    ProcTriggeredData(SpellProcEventEntry const * _spellProcEvent, Aura* _triggeredByAura)
        : spellProcEvent(_spellProcEvent), triggeredByAura(_triggeredByAura),
        triggeredByAura_SpellPair(Unit::spellEffectPair(triggeredByAura->GetId(),triggeredByAura->GetEffIndex()))
        {}
    SpellProcEventEntry const *spellProcEvent;
    Aura* triggeredByAura;
    Unit::spellEffectPair triggeredByAura_SpellPair;
};

typedef std::list< ProcTriggeredData > ProcTriggeredList;
typedef std::list< uint32> RemoveSpellList;

// List of auras that CAN be trigger but may not exist in spell_proc_event
// in most case need for drop charges
// in some types of aura need do additional check
// for example SPELL_AURA_MECHANIC_IMMUNITY - need check for mechanic
static bool isTriggerAura[TOTAL_AURAS];
static bool isNonTriggerAura[TOTAL_AURAS];
void InitTriggerAuraData()
{
    for (int i=0;i<TOTAL_AURAS;i++)
    {
      isTriggerAura[i]=false;
      isNonTriggerAura[i] = false;
    }
    isTriggerAura[SPELL_AURA_DUMMY] = true;
    isTriggerAura[SPELL_AURA_MOD_CONFUSE] = true;
    isTriggerAura[SPELL_AURA_MOD_THREAT] = true;
    isTriggerAura[SPELL_AURA_MOD_STUN] = true; // Aura not have charges but need remove him on trigger
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_DONE] = true;
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_TAKEN] = true;
    isTriggerAura[SPELL_AURA_MOD_RESISTANCE] = true;
    isTriggerAura[SPELL_AURA_MOD_ROOT] = true;
    isTriggerAura[SPELL_AURA_REFLECT_SPELLS] = true;
    isTriggerAura[SPELL_AURA_DAMAGE_IMMUNITY] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_SPELL] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_DAMAGE] = true;
    isTriggerAura[SPELL_AURA_MOD_CASTING_SPEED] = true;
    isTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT] = true;
    isTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL] = true;
    isTriggerAura[SPELL_AURA_REFLECT_SPELLS_SCHOOL] = true;
    isTriggerAura[SPELL_AURA_MECHANIC_IMMUNITY] = true;
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN] = true;
    isTriggerAura[SPELL_AURA_SPELL_MAGNET] = true;
    isTriggerAura[SPELL_AURA_MOD_ATTACK_POWER] = true;
    isTriggerAura[SPELL_AURA_ADD_CASTER_HIT_TRIGGER] = true;
    isTriggerAura[SPELL_AURA_OVERRIDE_CLASS_SCRIPTS] = true;
    isTriggerAura[SPELL_AURA_MOD_MECHANIC_RESISTANCE] = true;
    isTriggerAura[SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS] = true;
    isTriggerAura[SPELL_AURA_MOD_HASTE] = true;
    isTriggerAura[SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE]=true;
    isTriggerAura[SPELL_AURA_PRAYER_OF_MENDING] = true;
    isTriggerAura[SPELL_AURA_PRAYER_OF_MENDING_NPC] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE] = true;

    isNonTriggerAura[SPELL_AURA_MOD_POWER_REGEN]=true;
    isNonTriggerAura[SPELL_AURA_RESIST_PUSHBACK]=true;
}

uint32 createProcExtendMask(SpellDamageLog *damageInfo, SpellMissInfo missCondition)
{
    uint32 procEx = PROC_EX_NONE;
    // Check victim state
    if (missCondition!=SPELL_MISS_NONE)
    switch (missCondition)
    {
        case SPELL_MISS_MISS:    procEx|=PROC_EX_MISS;   break;
        case SPELL_MISS_RESIST:  procEx|=PROC_EX_RESIST; break;
        case SPELL_MISS_DODGE:   procEx|=PROC_EX_DODGE;  break;
        case SPELL_MISS_PARRY:   procEx|=PROC_EX_PARRY;  break;
        case SPELL_MISS_BLOCK:   procEx|=PROC_EX_BLOCK;  break;
        case SPELL_MISS_EVADE:   procEx|=PROC_EX_EVADE;  break;
        case SPELL_MISS_IMMUNE:  procEx|=PROC_EX_IMMUNE; break;
        case SPELL_MISS_IMMUNE2: procEx|=PROC_EX_IMMUNE; break;
        case SPELL_MISS_DEFLECT: procEx|=PROC_EX_DEFLECT;break;
        case SPELL_MISS_ABSORB:  procEx|=PROC_EX_ABSORB;break;
        case SPELL_MISS_REFLECT: procEx|=PROC_EX_REFLECT;break;
        default:
            break;
    }
    else
    {
        // On block
        if (damageInfo->blocked)
            procEx |= PROC_EX_BLOCK;
        // On absorb
        if (damageInfo->absorb)
            procEx |= PROC_EX_ABSORB;
        // On crit
        if (damageInfo->hitInfo & SPELL_HIT_TYPE_CRIT)
            procEx |= PROC_EX_CRITICAL_HIT;
        else
            procEx |= PROC_EX_NORMAL_HIT;
    }
    return procEx;
}

void Unit::ProcDamageAndSpellfor (bool isVictim, Unit * pTarget, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellEntry const * procSpell, uint32 damage)
{
    ++m_procDeep;
    if (m_procDeep > 5)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Prevent possible stack owerflow in Unit::ProcDamageAndSpellFor");
        if (procSpell)
            sLog.outLog(LOG_DEFAULT, "ERROR:   Spell %u", procSpell->Id);
        --m_procDeep;
        return;
    }
    // For melee/ranged based attack need update skills and set some Aura states
    if (procFlag & MELEE_BASED_TRIGGER_MASK)
    {
        // Update skills here for players
        if (GetTypeId() == TYPEID_PLAYER)
        {
            // On melee based hit/miss/resist need update skill (for victim and attacker)
            if (procExtra & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT|PROC_EX_MISS|PROC_EX_RESIST|PROC_EX_DODGE
                            |PROC_EX_PARRY|PROC_EX_BLOCK/*|PROC_EX_IMMUNE*/)) // learning how to handle (weapon on immunity) or (defense when you are immune) is illogical and harmful, cause some creatures are banished (stunned and immune).
            {
                if (pTarget->GetTypeId() != TYPEID_PLAYER && !(pTarget->ToCreature()->GetCreatureInfo()->flags_extra & (CREATURE_FLAG_EXTRA_NO_DAMAGE_TAKEN | CREATURE_FLAG_EXTRA_NO_COMBAT_SKILL_UPDATE)))
                    ((Player*)this)->UpdateCombatSkills(pTarget, attType, isVictim);
            }
            // Update defence if player is victim and parry/dodge/block
            if (isVictim && procExtra&(PROC_EX_DODGE|PROC_EX_PARRY|PROC_EX_BLOCK))
                ((Player*)this)->UpdateCombatSkills(pTarget,attType,true);

            // @!tanks_boost Blessing of Sanctuary - regen 4% mana for Paladins
            if (sWorld.isEasyRealm())
            {
                if (procExtra & (PROC_EX_PARRY | PROC_EX_DODGE | PROC_EX_BLOCK))
                {
                    Player* player = (Player*)this;
                    if (player->GetClass() == CLASS_PALADIN && player->GetMap()->IsDungeon())
                    {
                        std::set<uint32> bs_auras = { 27168,20914,20913,20912,20911 };

                        for (auto bs_aura : bs_auras)
                        {
                            if (HasAura(bs_aura))
                            {
                                int32 customBP = round(float(player->GetMaxPower(POWER_MANA)) * 0.04f);
                                player->CastCustomSpell(player, 32848, &customBP, NULL, NULL, false);
                            }
                        }
                    }
                }
            }
        }
        // If exist crit/parry/dodge/block need update aura state (for victim and attacker)
        if (procExtra & (PROC_EX_CRITICAL_HIT|PROC_EX_PARRY|PROC_EX_DODGE|PROC_EX_BLOCK))
        {
            // for victim
            if (isVictim)
            {
                // if victim and dodge attack
                if (procExtra&PROC_EX_DODGE)
                {
                    //Update AURA_STATE on dodge
                    if (GetClass() != CLASS_ROGUE) // skip Rogue Riposte
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if victim and parry attack
                if (procExtra & PROC_EX_PARRY)
                {
                    // For Hunters only Counterattack (skip Mongoose bite)
                    if (GetClass() == CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, true);
                        StartReactiveTimer(REACTIVE_HUNTER_PARRY);
                    }
                    else
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if and victim block attack
                if (procExtra & PROC_EX_BLOCK)
                {
                    ModifyAuraState(AURA_STATE_DEFENSE,true);
                    StartReactiveTimer(REACTIVE_DEFENSE);
                }
            }
            else //For attacker
            {
                // Overpower on victim dodge
                if (procExtra&PROC_EX_DODGE && GetTypeId() == TYPEID_PLAYER && GetClass() == CLASS_WARRIOR)
                {
                    ((Player*)this)->AddComboPoints(pTarget, 1);
                    StartReactiveTimer(REACTIVE_OVERPOWER);
                }
                // Enable AURA_STATE_CRIT on crit
                if (procExtra & PROC_EX_CRITICAL_HIT)
                {
                    ModifyAuraState(AURA_STATE_CRIT, true);
                    StartReactiveTimer(REACTIVE_CRIT);
                    if (GetClass()==CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE, true);
                        StartReactiveTimer(REACTIVE_HUNTER_CRIT);
                    }
                }
            }
        }
    }

    RemoveSpellList removedSpells;
    ProcTriggeredList procTriggered;
    // Fill procTriggered list
    for (AuraMap::const_iterator itr = GetAuras().begin(); itr!= GetAuras().end(); ++itr)
    {

        SpellProcEventEntry const* spellProcEvent = NULL;
        bool active = (damage > 0) || (procExtra & PROC_EX_ABSORB/* && (isVictim && procSpell == NULL)*/); // this flag is only used at mana shield. Now things which should be procced on your absorb gonna proc from other absorbs too. As for now the only known absorb-proc is mana shield spell damage proc.
        if (!IsTriggeredAtSpellProcEvent(itr->second, procSpell, procFlag, procExtra, attType, isVictim, active, spellProcEvent))
           continue;

        procTriggered.push_back(ProcTriggeredData(spellProcEvent, itr->second));
        SendCombatStats(1 << COMBAT_STATS_PROC, "aura %u is procing from spell %u; %u %u %u", pTarget,
            itr->first.first, procSpell ? procSpell->Id : 0, procFlag, procExtra, isVictim);
    }

    // @!legendary_weapon - caster items should proc with CHANCE_ON_HIT
    if (!isVictim && GetTypeId() == TYPEID_PLAYER)
    {
        if (procFlag & (PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL | PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT) &&
            procExtra & PROC_EX_DAMAGE_OR_HEAL)
        {
            Item* item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            if (item)
            {
                ItemPrototype const* proto = item->GetProto();
                if (proto)
                    ((Player*)this)->CastItemCombatSpellFromCast(item, proto, procFlag);
            }
        }
    }

    // Handle effects proceed this time
    for (ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); ++i)
    {
        // Some auras can be deleted in function called in this loop (except first, ofc)
        // Until storing auars in std::multimap to hard check deleting by another way
        if (i != procTriggered.begin())
        {
            bool found = false;
            AuraMap::const_iterator lower = GetAuras().lower_bound(i->triggeredByAura_SpellPair);
            AuraMap::const_iterator upper = GetAuras().upper_bound(i->triggeredByAura_SpellPair);
            for (AuraMap::const_iterator itr = lower; itr!= upper; ++itr)
            {
                if (itr->second==i->triggeredByAura)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                continue;
        }

        SpellProcEventEntry const *spellProcEvent = i->spellProcEvent;
        Aura *triggeredByAura = i->triggeredByAura;
        Modifier *auraModifier = triggeredByAura->GetModifier();
        SpellEntry const *spellInfo = triggeredByAura->GetSpellProto();
        ASSERT(spellInfo);
        if (!spellInfo)
            continue; // it happens (for some reason) that triggeredByAura is not a valid Aura pointer, then everything fucks up
        uint32 effIndex = triggeredByAura->GetEffIndex();
        bool useCharges = triggeredByAura->m_procCharges > 0;
        // For players set spell cooldown if need
        uint32 cooldown = 0;
        if (GetTypeId() == TYPEID_PLAYER && spellProcEvent && spellProcEvent->cooldown)
            cooldown = spellProcEvent->cooldown;

        switch (auraModifier->m_auraname)
        {
            case SPELL_AURA_PROC_TRIGGER_SPELL:
            {
                sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered by %s aura of spell %u)", spellInfo->Id,(isVictim?"a victim's":"an attacker's"), triggeredByAura->GetId());
                // Don`t drop charge or add cooldown for not started trigger
                if (!HandleProcTriggerSpell(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown))
                    continue;
                break;
            }
            case SPELL_AURA_PROC_TRIGGER_DAMAGE:
            {
                sLog.outDebug("ProcDamageAndSpell: doing %u damage from spell id %u (triggered by %s aura of spell %u)", auraModifier->m_amount, spellInfo->Id, (isVictim?"a victim's":"an attacker's"), triggeredByAura->GetId());
                SpellMissInfo missInfo = SpellHitResult(pTarget, spellInfo, false, true);
                if (missInfo != SPELL_MISS_NONE) // yes it can miss, this will also prevent damaging immune targets
                {
                    SendSpellMiss(pTarget, spellInfo->Id, missInfo);
                    break;
                }
                SpellDamageLog damageInfo(sSpellMgr.GetSpellAnalog(spellInfo), this, pTarget, spellInfo->SchoolMask);
                uint32 damage = SpellDamageBonus(pTarget, spellInfo, auraModifier->m_amount, SPELL_DIRECT_DAMAGE);
                CalculateSpellDamageTaken(&damageInfo, damage, spellInfo);
                //SendSpellNonMeleeDamageLog(&damageInfo);
                DealSpellDamage(&damageInfo, true);

                break;
            }
            case SPELL_AURA_MANA_SHIELD:
            case SPELL_AURA_DUMMY:
            {
                sLog.outDebug("ProcDamageAndSpell: casting spell id %u (triggered by %s dummy aura of spell %u)", spellInfo->Id,(isVictim?"a victim's":"an attacker's"), triggeredByAura->GetId());
                if (!HandleDummyAuraProc(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown))
                    continue;
                break;
            }
            case SPELL_AURA_MOD_HASTE:
            {
                sLog.outDebug("ProcDamageAndSpell: casting spell id %u (triggered by %s haste aura of spell %u)", spellInfo->Id,(isVictim?"a victim's":"an attacker's"), triggeredByAura->GetId());
                if (!HandleHasteAuraProc(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown))
                    continue;
                break;
            }
            case SPELL_AURA_OVERRIDE_CLASS_SCRIPTS:
            {
                sLog.outDebug("ProcDamageAndSpell: casting spell id %u (triggered by %s aura of spell %u)", spellInfo->Id,(isVictim?"a victim's":"an attacker's"), triggeredByAura->GetId());
                if (!HandleOverrideClassScriptAuraProc(pTarget, triggeredByAura, procSpell, cooldown))
                    continue;
                break;
            }
            case SPELL_AURA_PRAYER_OF_MENDING:
            {
                sLog.outDebug("ProcDamageAndSpell: casting mending (triggered by %s dummy aura of spell %u)",
                    (isVictim?"a victim's":"an attacker's"),triggeredByAura->GetId());
                HandleMendingAuraProc(triggeredByAura);
                break;
            }
            case SPELL_AURA_PRAYER_OF_MENDING_NPC:
            {
                HandleMendingNPCAuraProc(triggeredByAura);
                break;
            }
            case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
            {
                sLog.outDebug("ProcDamageAndSpell: casting spell %u (triggered with value by %s aura of spell %u)", spellInfo->Id,(isVictim?"a victim's":"an attacker's"), triggeredByAura->GetId());

                if (!HandleProcTriggerSpell(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown))
                    continue;
                break;
            }
            case SPELL_AURA_MOD_STUN:
                // Remove by default, but if charge exist drop it
                if (triggeredByAura->m_procCharges == 0)
                   removedSpells.push_back(triggeredByAura->GetId());
                break;
            case SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS:
            case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
                // Hunter's Mark (1-4 Rangs)
                if (spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && (spellInfo->SpellFamilyFlags&0x0000000000000400LL))
                {
                    uint32 basevalue = triggeredByAura->GetBasePoints();
                    auraModifier->m_amount += basevalue / 10;
                    if (auraModifier->m_amount > basevalue * 4)
                        auraModifier->m_amount = basevalue * 4;
                }
                break;
            case SPELL_AURA_MOD_CASTING_SPEED:
                // Skip melee hits or instant cast spells
                if (procSpell == NULL || SpellMgr::GetSpellCastTime(procSpell) == 0)
                    continue;
                break;
            case SPELL_AURA_REFLECT_SPELLS_SCHOOL:
                // Skip Melee hits and spells ws wrong school
                if (procSpell == NULL || (auraModifier->m_miscvalue & procSpell->SchoolMask) == 0)
                    continue;
                break;
            case SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT:
            case SPELL_AURA_MOD_POWER_COST_SCHOOL:
                // Skip melee hits and spells ws wrong school or zero cost
                if (procSpell == NULL ||
                    (procSpell->manaCost == 0 && procSpell->ManaCostPercentage == 0) || // Cost check
                    (auraModifier->m_miscvalue & procSpell->SchoolMask) == 0)         // School check
                    continue;
                break;
            case SPELL_AURA_MECHANIC_IMMUNITY:
            {
                // Compare mechanic
                if (procSpell == NULL || procSpell->Mechanic != auraModifier->m_miscvalue)
                    continue;

                // if immunity comes not from charged proc aura -> don't remove it
                // cant make spellId exception because two charged auras will refer to each other as immunity
                if (useCharges && isVictim && IsImmunedToSpell(procSpell, false))
                    continue;
                break;
            }
            case SPELL_AURA_DAMAGE_IMMUNITY:
            {
                // if immunity comes not from charged proc aura -> don't remove it
                // cant make spellId exception because two charged auras will refer to each other as immunity
                if (useCharges && isVictim && IsImmunedToDamage(procSpell ? (SpellSchoolMask)procSpell->SchoolMask : SPELL_SCHOOL_MASK_NORMAL, false))
                    continue;
                break;
            }
            case SPELL_AURA_MOD_MECHANIC_RESISTANCE:
                // Compare mechanic
                if (procSpell==NULL || procSpell->Mechanic != auraModifier->m_miscvalue)
                    continue;
                break;
            default:
                // nothing do, just charges counter
                break;
        }
        // Remove charge (aura can be removed by triggers)
        if (useCharges)
        {
            // need found aura on drop (can be dropped by triggers)
            AuraMap::const_iterator lower = GetAuras().lower_bound(i->triggeredByAura_SpellPair);
            AuraMap::const_iterator upper = GetAuras().upper_bound(i->triggeredByAura_SpellPair);
            for (AuraMap::const_iterator itr = lower; itr!= upper; ++itr)
            {
                if (itr->second == triggeredByAura)
                {
                     triggeredByAura->m_procCharges -= 1;
                     triggeredByAura->UpdateAuraCharges();
                     if (triggeredByAura->m_procCharges <= 0)
                          removedSpells.push_back(triggeredByAura->GetId());
                    break;
                }
            }
        }
    }
    if (removedSpells.size())
    {
        // Sort spells and remove duplicates
        removedSpells.sort();
        removedSpells.unique();
        // Remove auras from removedAuras
        for (RemoveSpellList::iterator i = removedSpells.begin(); i != removedSpells.end();)
        {
            RemoveSpellList::iterator tmpItr = i;
            ++i;
            if (Aura *pTemp = GetAura(*tmpItr, 0)) // should we check all 3 effects ? I do not think so :p
                if (pTemp->m_procCharges > 0) // Aura has been refreshed after adding to removedSpells
                {
                    removedSpells.erase(tmpItr);
                    continue;
                }
            RemoveAurasDueToSpell(*tmpItr);
        }
    }
    --m_procDeep;
}

SpellSchoolMask Unit::GetMeleeDamageSchoolMask() const
{
    return SPELL_SCHOOL_MASK_NORMAL;
}

Player* Unit::GetSpellModOwner() const
{
    if (GetTypeId()==TYPEID_PLAYER)
        return (Player*)this;
    if (((Creature*)this)->isPet() || ((Creature*)this)->isTotem())
    {
        Unit* owner = GetOwner();
        if (owner && owner->GetTypeId()==TYPEID_PLAYER)
            return (Player*)owner;
    }
    return NULL;
}

///----------Pet responses methods-----------------
void Unit::SendPetCastFail(uint32 spellid, SpellCastResult msg)
{
    if(msg == SPELL_CAST_OK)
        return;

    Unit *owner = GetCharmerOrOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    SendCombatStats(1 << COMBAT_STATS_FAILED_CAST, "Pet cast %u failed, result %u", NULL,spellid, msg);
    WorldPacket data(SMSG_PET_CAST_FAILED, (4+1));
    data << uint32(spellid);
    data << uint8(msg);
    ((Player*)owner)->SendPacketToSelf(&data);
}

void Unit::SendPetActionFeedback (uint8 msg)
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_ACTION_FEEDBACK, 1);
    data << uint8(msg);
    ((Player*)owner)->SendPacketToSelf(&data);
}

void Unit::SendPetTalk (uint32 pettalk)
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_ACTION_SOUND, 8+4);
    data << uint64(GetGUID());
    data << uint32(pettalk);
    ((Player*)owner)->SendPacketToSelf(&data);
}

void Unit::SendPetClearCooldown(uint32 spellid)
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
    data << uint32(spellid);
    data << uint64(GetGUID());
    ((Player*)owner)->SendPacketToSelf(&data);
}

void Unit::SendPetAIReaction(uint64 guid)
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_AI_REACTION, 12);
    data << uint64(guid) << uint32(00000002);
    ((Player*)owner)->SendPacketToSelf(&data);
}

///----------End of Pet responses methods----------
bool Unit::SetPosition(float x, float y, float z, float orientation, bool teleport)
{
    // prevent crash when a bad coord is sent by the client
    if (!Hellground::IsValidMapCoord(x,y,z,orientation))
    {
        sLog.outDebug("Unit::SetPosition(%f, %f, %f) .. bad coordinates!",x,y,z);
        return false;
    }

    bool turn = (GetOrientation() != orientation);
    bool relocated = (teleport || GetPositionX() != x || GetPositionY() != y || GetPositionZ() != z);

    SpellAuraInterruptFlags interruptFlags = AURA_INTERRUPT_FLAG_NONE;
    if (relocated)
    {
        interruptFlags = SpellAuraInterruptFlags(interruptFlags | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_NOT_SEATED);

        // move and update visible state if need
        if (GetTypeId() == TYPEID_PLAYER)
            GetMap()->PlayerRelocation(ToPlayer(), x, y, z, orientation);
        else
            GetMap()->CreatureRelocation(ToCreature(), x, y, z, orientation);
    }

    if (turn)
    {
        interruptFlags = SpellAuraInterruptFlags(interruptFlags | AURA_INTERRUPT_FLAG_TURNING | AURA_INTERRUPT_FLAG_NOT_SEATED);
        SetOrientation(orientation);
    }

    if (interruptFlags)
        RemoveAurasWithInterruptFlags(interruptFlags);

    return (relocated || turn);
}

void Unit::StopMoving()
{
    ClearUnitState(UNIT_STAT_MOVING);

    if (!IsInWorld() || IsStopped())
        return;

    DisableSpline();

    Movement::MoveSplineInit init(*this);
    init.SetFacing(GetOrientation());
    init.Launch();
}

bool Unit::IsStopped() const
{
    return movespline->Finalized();
}

bool Unit::IsSitState() const
{
    uint8 s = getStandState();
    return s == UNIT_STAND_STATE_SIT_CHAIR || s == UNIT_STAND_STATE_SIT_LOW_CHAIR ||
        s == UNIT_STAND_STATE_SIT_MEDIUM_CHAIR || s == UNIT_STAND_STATE_SIT_HIGH_CHAIR ||
        s == UNIT_STAND_STATE_SIT;
}

bool Unit::IsStandState() const
{
	uint8 s = getStandState();
    return !IsSitState() && s != UNIT_STAND_STATE_SLEEP && s != UNIT_STAND_STATE_KNEEL;
}

void Unit::SetStandState(uint8 state)
{
    SetByteValue(UNIT_FIELD_BYTES_1, 0, state);

    if (IsStandState())
       RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_SEATED);

    if (GetTypeId()==TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_STANDSTATE_UPDATE, 1);
        data << (uint8)state;
        ((Player*)this)->SendPacketToSelf(&data);
    }
}

bool Unit::IsPolymorphed() const
{
    return SpellMgr::GetSpellSpecific(getTransForm()) == SPELL_MAGE_POLYMORPH;
}

void Unit::SetDisplayId(uint32 modelId)
{
    SetUInt32Value(UNIT_FIELD_DISPLAYID, modelId);

    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (!pet->isControlled())
            return;
        Unit *owner = GetOwner();
        if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
            ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MODEL_ID);
    }
}

void Unit::ClearComboPointHolders()
{
    while (!m_ComboPointHolders.empty())
    {
        uint32 lowguid = *m_ComboPointHolders.begin();

        Player* plr = sObjectMgr.GetPlayerInWorld(MAKE_NEW_GUID(lowguid, 0, HIGHGUID_PLAYER));
        if (plr && plr->GetComboTarget()==GetGUID())         // recheck for safe
            plr->ClearComboPoints();                        // remove also guid from m_ComboPointHolders;
        else
            m_ComboPointHolders.erase(lowguid);             // or remove manually
    }
}

void Unit::ClearAllReactives()
{

    for (int i=0; i < MAX_REACTIVE; ++i)
        m_reactiveTimer[i] = 0;

    if (HasAuraState(AURA_STATE_DEFENSE))
        ModifyAuraState(AURA_STATE_DEFENSE, false);
    if (GetClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_PARRY))
        ModifyAuraState(AURA_STATE_HUNTER_PARRY, false);
    if (HasAuraState(AURA_STATE_CRIT))
        ModifyAuraState(AURA_STATE_CRIT, false);
    if (GetClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_CRIT_STRIKE) )
        ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE, false);

    if (GetClass() == CLASS_WARRIOR && GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->ClearComboPoints();
}

void Unit::UpdateReactives(uint32 p_time)
{
    for (int i = 0; i < MAX_REACTIVE; ++i)
    {
        ReactiveType reactive = ReactiveType(i);

        if (!m_reactiveTimer[reactive])
            continue;

        if (m_reactiveTimer[reactive] <= p_time)
        {
            m_reactiveTimer[reactive] = 0;

            switch (reactive)
            {
                case REACTIVE_DEFENSE:
                    if (HasAuraState(AURA_STATE_DEFENSE))
                        ModifyAuraState(AURA_STATE_DEFENSE, false);
                    break;
                case REACTIVE_HUNTER_PARRY:
                    if (GetClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_PARRY))
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, false);
                    break;
                case REACTIVE_CRIT:
                    if (HasAuraState(AURA_STATE_CRIT))
                        ModifyAuraState(AURA_STATE_CRIT, false);
                    break;
                case REACTIVE_HUNTER_CRIT:
                    if (GetClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_CRIT_STRIKE))
                        ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE, false);
                    break;
                case REACTIVE_OVERPOWER:
                    if (GetClass() == CLASS_WARRIOR && GetTypeId() == TYPEID_PLAYER)
                        ((Player*)this)->ClearComboPoints();
                    break;
                default:
                    break;
            }
        }
        else
        {
            m_reactiveTimer[reactive] -= p_time;
        }
    }
}

Unit* Unit::SelectNearbyTarget(float dist, Unit* erase) const
{
    std::list<Unit *> targets;
    Hellground::AnyUnfriendlyUnitInObjectRangeCheck u_check(this, dist);
    Hellground::UnitListSearcher<Hellground::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);

    Cell::VisitAllObjects(this, searcher, dist);

    // remove current target
    if (!erase && GetVictim())
        targets.remove(GetVictim());
    else if (erase)
        targets.remove(erase);

    // remove not LoS targets
    for (std::list<Unit *>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        if (!IsWithinLOSInMap(*tIter) || (*tIter)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) ||
            (*tIter)->GetTypeId() == TYPEID_UNIT && (
                (((Creature*)(*tIter))->isCivilian() && !(*tIter)->IsInCombat()) ||
                ((Creature*)(*tIter))->isTrigger() || ((Creature*)(*tIter))->isTotem()))
        {
            std::list<Unit *>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }

    // no appropriate targets
    if (targets.empty())
        return NULL;

    // select random
    uint32 rIdx = urand(0,targets.size()-1);
    std::list<Unit *>::const_iterator tcIter = targets.begin();
    for (uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

void Unit::ApplyAttackTimePercentMod(WeaponAttackType att,float val, bool apply)
{
    float remainingTimePct = (float)m_attackTimer[att] / (GetAttackTime(att) * m_modAttackSpeedPct[att]);
    if (val > 0)
    {
        ApplyPercentModFloatVar(m_modAttackSpeedPct[att], val, !apply);
        ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val,!apply);
    }
    else
    {
        ApplyPercentModFloatVar(m_modAttackSpeedPct[att], -val, apply);
        ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME+att,-val,apply);
    }
    m_attackTimer[att] = uint32(GetAttackTime(att) * m_modAttackSpeedPct[att] * remainingTimePct);
}

void Unit::ApplyCastTimePercentMod(float val, bool apply)
{
    if (val > 0)
        ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED,val,!apply);
    else
        ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED,-val,apply);
}

uint32 Unit::GetCastingTimeForBonus(SpellEntry const *spellProto, DamageEffectType damagetype, uint32 CastingTime)
{
    // Not apply this to creature cast spells with casttime==0
    if (CastingTime==0 && GetTypeId()==TYPEID_UNIT && !((Creature*)this)->isPet())
        return 3500;

    if (CastingTime > 7000) CastingTime = 7000;
    if (CastingTime < 1500) CastingTime = 1500;

    if (damagetype == DOT && !SpellMgr::IsChanneledSpell(spellProto))
        CastingTime = 3500;

    int32 overTime    = 0;
    uint8 effects     = 0;
    bool DirectDamage = false;
    bool AreaEffect   = false;

    for (uint32 i=0; i<3;i++)
    {
        switch (spellProto->Effect[i])
        {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_POWER_DRAIN:
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_ENVIRONMENTAL_DAMAGE:
            case SPELL_EFFECT_POWER_BURN:
            case SPELL_EFFECT_HEAL:
                DirectDamage = true;
                break;
            case SPELL_EFFECT_APPLY_AURA:
                switch (spellProto->EffectApplyAuraName[i])
                {
                    case SPELL_AURA_PERIODIC_DAMAGE:
                    case SPELL_AURA_PERIODIC_HEAL:
                    case SPELL_AURA_PERIODIC_LEECH:
                        if (SpellMgr::GetSpellDuration(spellProto))
                            overTime = SpellMgr::GetSpellDuration(spellProto);
                        break;
                    default:
                        // -5% per additional effect
                        ++effects;
                        break;
                }
            default:
                break;
        }

        if (IsAreaEffectTarget[spellProto->EffectImplicitTargetA[i]] || IsAreaEffectTarget[spellProto->EffectImplicitTargetB[i]])
            AreaEffect = true;
    }

    // Combined Spells with Both Over Time and Direct Damage
    if (overTime > 0 && CastingTime > 0 && DirectDamage)
    {
        // mainly for DoTs which are 3500 here otherwise
        uint32 OriginalCastTime = SpellMgr::GetSpellCastTime(spellProto);
        if (OriginalCastTime > 7000) OriginalCastTime = 7000;
        if (OriginalCastTime < 1500) OriginalCastTime = 1500;
        // Portion to Over Time
        float PtOT = (overTime / 15000.f) / ((overTime / 15000.f) + (OriginalCastTime / 3500.f));

        if (damagetype == DOT)
            CastingTime = uint32(CastingTime * PtOT);
        else if (PtOT < 1.0f)
            CastingTime  = uint32(CastingTime * (1 - PtOT));
        else
            CastingTime = 0;
    }

    // Area Effect Spells receive only half of bonus
    if (AreaEffect)
        CastingTime /= 2;

    // -5% of total per any additional effect
    for (uint8 i=0; i<effects; ++i)
    {
        if (CastingTime > 175)
        {
            CastingTime -= 175;
        }
        else
        {
            CastingTime = 0;
            break;
        }
    }

    return CastingTime;
}

void Unit::UpdateAuraForGroup(uint8 slot)
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = (Player*)this;
        if (player->GetGroup())
        {
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_AURAS);
            player->SetAuraUpdateMask(slot);
        }
    }
    else if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->isPet())
    {
        Pet *pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit *owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
            {
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_AURAS);
                pet->SetAuraUpdateMask(slot);
            }
        }
    }
}

float Unit::GetAPMultiplier(WeaponAttackType attType, bool normalized)
{
    if (!normalized || GetTypeId() != TYPEID_PLAYER)
        return float(GetAttackTime(attType))/1000.0f;

    Item *Weapon = ((Player*)this)->GetWeaponForAttack(attType);
    if (!Weapon)
        return 2.4;                                         // fist attack

    switch (Weapon->GetProto()->InventoryType)
    {
        case INVTYPE_2HWEAPON:
            return 3.3;
        case INVTYPE_RANGED:
        case INVTYPE_RANGEDRIGHT:
        case INVTYPE_THROWN:
            return 2.8;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPONOFFHAND:
        default:
            return Weapon->GetProto()->SubClass==ITEM_SUBCLASS_WEAPON_DAGGER ? 1.7 : 2.4;
    }
}

Aura* Unit::GetDummyAura(uint32 spell_id) const
{
    Unit::AuraList const& mDummy = GetAurasByType(SPELL_AURA_DUMMY);
    for (Unit::AuraList::const_iterator itr = mDummy.begin(); itr != mDummy.end(); ++itr)
        if ((*itr)->GetId() == spell_id)
            return *itr;

    return NULL;
}

bool Unit::IsUnderLastManaUseEffect() const
{
    return  WorldTimer::getMSTimeDiff(m_lastManaUse,WorldTimer::getMSTime()) < 5000;
}

void Unit::SetContestedPvP(Player *attackedPlayer)
{
    Player* player = GetCharmerOrOwnerPlayerOrPlayerItself();

    if (!player || attackedPlayer && (attackedPlayer == player || player->duel && player->duel->opponent == attackedPlayer && player->duel->startTime != 0))
        return;

    player->SetContestedPvPTimer(30000);

    if (!player->HasUnitState(UNIT_STAT_ATTACK_PLAYER))
    {
        player->addUnitState(UNIT_STAT_ATTACK_PLAYER);
        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
        player->SetVisibility(player->GetVisibility());

        std::list<Unit*> targets;
        Hellground::ContestedGuardCheck check(player);
        Hellground::UnitListSearcher<Hellground::ContestedGuardCheck> searcher(targets, check);
        Cell::VisitAllObjects(player, searcher, 20.0f * sWorld.getConfig(RATE_CREATURE_GUARD_AGGRO));
        for (std::list<Unit *>::iterator itr = targets.begin(); itr != targets.end(); itr++)
            (*itr)->ToCreature()->AI()->MoveInLineOfSight_Safe(player);
    }
    if (!HasUnitState(UNIT_STAT_ATTACK_PLAYER))
    {
        addUnitState(UNIT_STAT_ATTACK_PLAYER);
        SetVisibility(GetVisibility());

        std::list<Unit*> targets;
        Hellground::ContestedGuardCheck check(this);
        Hellground::UnitListSearcher<Hellground::ContestedGuardCheck> searcher(targets, check);
        Cell::VisitAllObjects(this, searcher, 20.0f * sWorld.getConfig(RATE_CREATURE_GUARD_AGGRO));
        for (std::list<Unit *>::iterator itr = targets.begin(); itr != targets.end(); itr++)
            (*itr)->ToCreature()->AI()->MoveInLineOfSight_Safe(this);
    }
}

void Unit::AddPetAura(PetAura const* petSpell)
{
    m_petAuras.insert(petSpell);
    if (Pet* pet = GetPet())
        pet->CastPetAura(petSpell);
    else if (Unit* enslaved = GetEnslaved())
        enslaved->CastSpell(enslaved, petSpell->GetAura(0), true);
}

void Unit::RemovePetAura(PetAura const* petSpell)
{
    m_petAuras.erase(petSpell);
    if (Pet* pet = GetPet())
        pet->RemoveAurasDueToSpell(petSpell->GetAura(pet->GetEntry()));
    else if (Unit* enslaved = GetCharm())
        enslaved->RemoveAurasDueToSpell(petSpell->GetAura(0));
}

Pet* Unit::CreateTamedPetFrom(Creature* creatureTarget,uint32 spell_id)
{
    Pet* pet = new Pet(HUNTER_PET);

    if (!pet->CreateBaseAtCreature(creatureTarget))
    {
        delete pet;
        return NULL;
    }

    pet->SetOwnerGUID(GetGUID());
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, getFaction());
    pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, spell_id);

    if (!pet->InitStatsForLevel(creatureTarget->GetLevel()))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Pet::InitStatsForLevel() failed for creature (Entry: %u)!",creatureTarget->GetEntry());
        delete pet;
        return NULL;
    }

    pet->GetCharmInfo()->SetPetNumber(sObjectMgr.GeneratePetNumber(), true);
    // this enables pet details window (Shift+P)

    pet->InitPetCreateSpells();
    pet->SetHealth(pet->GetMaxHealth());

    return pet;
}

bool Unit::IsTriggeredAtSpellProcEvent(Aura* aura, SpellEntry const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active, SpellProcEventEntry const*& spellProcEvent)
{
    SpellEntry const* spellProto = aura->GetSpellProto ();

    // Get proc Event Entry
    spellProcEvent = sSpellMgr.GetSpellProcEvent(spellProto->Id);

    // Aura info stored here
    Modifier *mod = aura->GetModifier();
    // Skip this auras
    if (isNonTriggerAura[mod->m_auraname])
        return false;
    // If not trigger by default and spellProcEvent==NULL - skip
    if (!isTriggerAura[mod->m_auraname] && spellProcEvent==NULL)
        return false;

    // Get EventProcFlag
    uint32 EventProcFlag;
    if (spellProcEvent && spellProcEvent->procFlags) // if exist get custom spellProcEvent->procFlags
        EventProcFlag = spellProcEvent->procFlags;
    else
        EventProcFlag = spellProto->procFlags;       // else get from spell proto
    // Continue if no trigger exist
    if (!EventProcFlag)
        return false;
    // Check spellProcEvent data requirements
    if (!SpellMgr::IsSpellProcEventCanTriggeredBy(spellProcEvent, EventProcFlag, procSpell, procFlag, procExtra, active))
        return false;

    // Aura added by spell can`t trigger from self (prevent drop charges/do triggers)
    // But except periodic triggers (can triggered from self)
    if (procSpell && procSpell->Id == spellProto->Id && !(spellProto->procFlags & PROC_FLAG_ON_TAKE_PERIODIC) && !(spellProto->AttributesEx4 & SPELL_ATTR_EX4_CANT_PROC_FROM_SELFCAST))
        return false;

    // Check if current equipment allows aura to proc
    if (!isVictim && GetTypeId() == TYPEID_PLAYER)
    {
        if (spellProto->EquippedItemClass == ITEM_CLASS_WEAPON)
        {
            Item *item = NULL;
            if (attType == BASE_ATTACK)
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            else if (attType == OFF_ATTACK)
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            else
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);

            if (!((Player*)this)->IsUseEquipedWeapon(attType==BASE_ATTACK))
                return false;

            // @!legendary_weapon Legendary Axe case
            bool ignore_item_class = false;
            if (item && item->GetProto()->ItemId == ITEM_LEGENDARY_AXE && GetClass() == CLASS_WARRIOR)
            {
                //Sword Specialization
                //Mace Specialization
                const std::vector<uint32> specs = { 12281, 12284 };

                uint32 first = sSpellMgr.GetFirstSpellInChain(spellProto->Id);

                // check only specialization talents
                if (std::find(specs.begin(), specs.end(), first) != specs.end()) 
                {
                    for (auto spec : specs)
                    {
                        // get first active talent
                        if (HasSpellChained(spec))
                        {
                            if (HasSpellChained(12700)) //Poleaxe Specialization
                                return false;

                            // triggering spell is fit talent
                            if (first == spec)
                                ignore_item_class = true;

                            break;
                        }
                    }
                }
            }
            
            if (!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_WEAPON || (!ignore_item_class && !((1 << item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask)))
                    return false;
        }
        else if (spellProto->EquippedItemClass == ITEM_CLASS_ARMOR)
        {
            // Check if player is wearing shield
            Item *item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (!item || item->IsBroken() || item->GetProto()->Class != ITEM_CLASS_ARMOR || !((1<<item->GetProto()->SubClass) & spellProto->EquippedItemSubClassMask))
                return false;
        }
    }
    // Get chance from spell

    //procdebug (set, caster weapon)
    float chance = (float)spellProto->procChance;
    // If in spellProcEvent exist custom chance, chance = spellProcEvent->customChance;
    if (spellProcEvent && spellProcEvent->customChance)
        chance = spellProcEvent->customChance;
    // If PPM exist calculate chance from PPM
    if (!isVictim && spellProcEvent && spellProcEvent->ppmRate != 0)
    {
        if (procSpell && procSpell->Id == 20424/*Seal of Command*/ && spellProto->SpellFamilyFlags & 0x8000000/*Wisdom/Light/Justice. some others but they can't proc from Seal of Command*/)
            return true;

        uint32 WeaponSpeed = GetAttackTime(attType, spellProto->AttributesEx6 & SPELL_ATTR_EX6_REAL_PPM_SPEED);

        //procdebug
        // Gensen: procs were broken by this case @bugged_proc
        //chance = GetPPMProcChance((procFlag & (PROC_FLAG_SUCCESSFUL_MELEE_HIT | PROC_FLAG_SUCCESSFUL_RANGED_HIT)) ? WeaponSpeed : 2400, spellProcEvent->ppmRate);
        chance = GetPPMProcChance(WeaponSpeed, spellProcEvent->ppmRate);

        // @!legendary_weapon Legendary Dagger: 
        if (spellProto->Id == 27787)
        {
            Item* item = NULL;
            if (attType == BASE_ATTACK)
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            else if (attType == OFF_ATTACK)
                item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            
            if (item && item->GetProto()->ItemId == ITEM_LEGENDARY_DAGGER_ROGUE)
            {
                // each dagger applies proc aura that procs twice after each hit, so if we want to make it proc 1.2% on hit divide it by 2 = 0.6
                // lower proc chance if have only one dagger 
                if (!((Player*)this)->HasAura(41719))
                    chance /= 20;
            }
        }
            
    }

    // Apply chance modifer aura
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_CHANCE_OF_SUCCESS,chance);

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_ALWAYS_PROC)
        chance = 100.0f;

    SendCombatStats(1 << COMBAT_STATS_PROC, "item %u spell %u - proc spell: %u, chance: %.2f, procFlag: %u (at_spell_proc)", this, 0, spellProto->Id, procSpell ? procSpell->Id : 0, chance, procFlag);

    return roll_chance_f(chance);
}

bool Unit::HandleMendingAuraProc(Aura* triggeredByAura)
{
    // aura can be deleted at casts
    SpellEntry const* spellProto = triggeredByAura->GetSpellProto();
    uint32 effIdx = triggeredByAura->GetEffIndex();
    int32 heal = triggeredByAura->GetModifier()->m_amount;
    uint64 caster_guid = triggeredByAura->GetCasterGUID();

    // jumps
    int32 jumps = triggeredByAura->m_procCharges-1;

    // current aura expire
    triggeredByAura->m_procCharges = 1;             // will removed at next charges decrease

    if (Unit* caster = triggeredByAura->GetCaster())
    {
        // next target selection
        if (jumps > 0 && caster->GetTypeId()==TYPEID_PLAYER)
        {
            Player *PlayerCaster = (Player*)caster;
            float radius;
            if (spellProto->EffectRadiusIndex[effIdx])
                radius = SpellMgr::GetSpellRadius(spellProto,effIdx,false);
            else
                radius = SpellMgr::GetSpellMaxRange(sSpellRangeStore.LookupEntry(spellProto->rangeIndex));

            PlayerCaster->ApplySpellMod(spellProto->Id, SPELLMOD_RADIUS, radius,NULL);

            if (Unit* target = GetNextRandomRaidMember(radius, false, false))
            {
                Player* TargetMaster = target->GetCharmerOrOwnerPlayerOrPlayerItself();
                if (!TargetMaster || !TargetMaster->duel || TargetMaster->duel && TargetMaster == PlayerCaster)
                {
                    // aura will applied from caster, but spell casted from current aura holder
                    SpellModifier *mod = new SpellModifier;
                    mod->op = SPELLMOD_CHARGES;
                    mod->value = jumps-5;               // negative
                    mod->type = SPELLMOD_FLAT;
                    mod->spellId = spellProto->Id;
                    mod->effectId = effIdx;
                    mod->lastAffected = NULL;
                    mod->mask = spellProto->SpellFamilyFlags;
                    mod->charges = 0;

                    PlayerCaster->AddSpellMod(mod, true);
                    CastCustomSpell(target,spellProto->Id,&heal,NULL,NULL,true,NULL,triggeredByAura,caster->GetGUID());
                    PlayerCaster->AddSpellMod(mod, false);
                }
            }
        }
        heal = caster->SpellHealingBonus(spellProto, heal, HEAL, this);
    }

    CastCustomSpell(this,33110,&heal,NULL,NULL,true);
    return true;
}

bool Unit::HandleMendingNPCAuraProc(Aura* triggeredByAura)
{
    SpellEntry const* spellProto = triggeredByAura->GetSpellProto();
    uint32 effIdx = triggeredByAura->GetEffIndex();
    int32 heal = triggeredByAura->GetModifier()->m_amount;

    // jumps
    int32 jumps = triggeredByAura->m_procCharges-1;

    if (Unit* caster = triggeredByAura->GetCaster())
    {
        if(caster->GetTypeId() != TYPEID_UNIT)
            return false;

        Creature* CreatureCaster = (Creature*)caster;

        // next target selection
        if (jumps >= 0)
        {
            //triggeredByAura->m_procCharges = jumps;
            triggeredByAura->UpdateAuraCharges();
            float radius = 20.0;

            CreatureGroup * formation = (CreatureCaster->GetFormation());
            // only search for targets if having group formation !
            if(!formation)
            {
                RemoveAurasDueToSpell(spellProto->Id);
                CastCustomSpell(this,33110,&heal,NULL,NULL,true);
                return false;
            }

            if (Creature* target = formation->GetNextRandomCreatureGroupMember(CreatureCaster, radius))
            {
                if(jumps > 0)
                {
                    //remove aura on caster
                    RemoveAurasDueToSpell(spellProto->Id);
                    // manually apply aura on target, to update charges count
                    Aura* aura = CreateAura(spellProto, effIdx, NULL, this, NULL);
                    aura->SetLoadedState(target->GetGUID(), heal, triggeredByAura->GetAuraMaxDuration(), triggeredByAura->GetAuraMaxDuration(), jumps);
                    aura->SetTarget(target);
                    target->AddAura(aura);
                    // visual jump
                    CastSpell(target, 41637, true, NULL, triggeredByAura, caster->GetGUID());
                }
            }
            else
            {
                RemoveAurasDueToSpell(spellProto->Id);
                CastCustomSpell(this,33110,&heal,NULL,NULL,true);
                return false;
            }
        }
    }

    // heal
    CastCustomSpell(this,33110,&heal,NULL,NULL,true);
    return true;
}

void Unit::RemoveAurasAtChanneledTarget(SpellEntry const* spellInfo, Unit * caster)
{
/*    uint64 target_guid = GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT);
    if (target_guid == GetGUID())
        return;

    if (!IS_UNIT_GUID(target_guid))
        return;

    Unit* target = ObjectAccessor::GetUnit(*this, target_guid);*/
    if (!caster)
        return;

    for (AuraMap::iterator iter = GetAuras().begin(); iter != GetAuras().end();)
    {
        if (iter->second->GetId() == spellInfo->Id && iter->second->GetCasterGUID() == caster->GetGUID())
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::NearTeleportTo(float x, float y, float z, float orientation, bool casting /*= false*/ )
{
    DisableSpline();

    if (Player *pThis = ToPlayer())
        pThis->TeleportTo(GetMapId(), x, y, z, orientation, TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT/* | TELE_TO_NOT_UNSUMMON_PET*/ | (casting ? TELE_TO_SPELL : 0));
    else
    {
        Relocate(x, y, z);
        SendHeartBeat();
    }
}

/*-----------------------TRINITY-----------------------------*/
bool Unit::preventApplyPersistentAA(SpellEntry const *spellInfo, uint8 eff_index)
{
    bool unique = false;
    switch (spellInfo->Id)
    {
        case 38575: //Toxic Spores
        case 40253: //Molten Flame
        case 31943: //Doomfire
        case 33802: //Flame Wave
            unique = true;
            break;
    }

    if (unique && HasAura(spellInfo->Id, eff_index))
        return true;

    if (GetTypeId() == TYPEID_UNIT && GetEntry() == 23111)
        return true;

    return false;
}

class RelocationNotifyEvent : public BasicEvent
{
    public:
        RelocationNotifyEvent(Unit& owner) : BasicEvent(), _owner(owner)
        {
            _owner._SetAINotifyScheduled(true);
        }

        bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
        {
            float radius = _owner.GetMap()->GetVisibilityDistance(&_owner);
            if (_owner.GetObjectGuid().IsPlayer())
            {
                 Hellground::PlayerRelocationNotifier notify(*_owner.ToPlayer());
                 Cell::VisitAllObjects(&_owner,notify,radius);
            }
            else
            {
                Hellground::CreatureRelocationNotifier notify(*_owner.ToCreature());
                Cell::VisitAllObjects(&_owner,notify,radius);
            }

            //_owner.GetPosition(_owner._notifiedPosition);
            _owner._SetAINotifyScheduled(false);
            return true;
        }

        void Abort(uint64)
        {
            _owner._SetAINotifyScheduled(false);
        }

    private:
        Unit& _owner;
};

void Unit::ScheduleAINotify(uint32 delay)
{
    if (!IsAINotifyScheduled())
        AddEvent(new RelocationNotifyEvent(*this), delay);
}

void Unit::OnRelocated()
{
    Position delta;
    delta.x = _notifiedPosition.x - GetPositionX();
    delta.y = _notifiedPosition.y - GetPositionY();
    delta.z = _notifiedPosition.z - GetPositionZ();

    float distsq = delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
    if (distsq > GetTerrain()->GetSpecifics()->viewupdatedistance)
    {
        // This sends us to cameras(players) and deletes from them, if we're too far
        GetPosition(_notifiedPosition);
        UpdateVisibilityAndView();
        return; // ScheduleAINotify(0) is called within UpdateVisibilityAndView(), so we can "return" here.
    }

    // This is for creatures MoveInLineOfSight - it is actually updated each ainotifyperiod if Unit is moving
    ScheduleAINotify(GetTerrain()->GetSpecifics()->ainotifyperiod);
}

void Unit::UpdateVisibilityAndView()
{
	if (sWorld.getConfig(CONFIG_BOT_SKIP_UPDATES) && GetTypeId() == TYPEID_PLAYER && ((Player*)this)->GetSession()->isFakeBot())
		return;
	
	WorldObject::UpdateVisibilityAndView();
    ScheduleAINotify(0);
}

void Unit::Kill(Unit *pVictim, bool durabilityLoss)
{
	if (!pVictim)
		return;

	if (pVictim->isDead())
	{
		sLog.outLog(LOG_SPECIAL,"Unit %s (id %u guid %u) killing twice %s (guid %u)", GetName(), GetEntry(), GetGUIDLow(), pVictim->GetName(), pVictim->GetGUIDLow());
		//return;
	}

	// Prevent double kill the exact unit
	// Gensen: do we need this additional check?
	if (!pVictim->GetHealth() || pVictim->isDead() || isDead())
		AttackStop();

	pVictim->SetHealth(0);

	// find player: owner of controlled `this` or `this` itself maybe
	Player *player = GetCharmerOrOwnerPlayerOrPlayerItself();

	bool bRewardIsAllowed = true;
	if (pVictim->GetTypeId() == TYPEID_UNIT)
	{
		bRewardIsAllowed = ((Creature*)pVictim)->IsDamageEnoughForLootingAndReward();
		if (!bRewardIsAllowed)
			((Creature*)pVictim)->SetLootRecipient(NULL);
	}

	if (bRewardIsAllowed && pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->GetLootRecipient())
		player = ((Creature*)pVictim)->GetLootRecipient();

	// Reward player, his pets, and group/raid members
	// call kill spell proc event (before real die and combat stop to triggering auras removed at death/combat stop)
	if (bRewardIsAllowed && player && player != pVictim)
	{
		if (GetTypeId() == TYPEID_PLAYER && (IsInPartyWith(player) || IsInRaidWith(player)))
		{
			uint32 procFlag = 0;
			if (((Player*)this)->RewardPlayerAndGroupAtKill(pVictim))
			{
				procFlag = PROC_FLAG_KILL_AND_GET_XP;
				if (Pet* pet = ((Player*)this)->GetMiniPet())
				{
					if (pet->AI())
						pet->AI()->OwnerKilledAndGotXpOrHp(this, pVictim);
				}
			}
			else
				procFlag = PROC_FLAG_NONE;

			ProcDamageAndSpell(pVictim, procFlag, PROC_FLAG_KILLED, PROC_EX_NONE, 0);
		}
		else
		{
			uint32 procFlag = 0;
			if (player->RewardPlayerAndGroupAtKill(pVictim) && (IsInPartyWith(player) || IsInRaidWith(player)))
				procFlag = PROC_FLAG_KILL_AND_GET_XP;
			else
				procFlag = PROC_FLAG_NONE;

			if (Unit *owner = GetCharmerOrOwner())
				owner->ProcDamageAndSpell(pVictim, procFlag, PROC_FLAG_KILLED, PROC_EX_NONE, 0);
		}
	}

	// HACK: Skeleton Shot - summon skeleton on death when having aura
	if (pVictim->GetTypeId() == TYPEID_PLAYER && pVictim->HasAura(41171, 1))
		pVictim->CastSpell((Unit*)NULL, 41174, true, 0, 0, this->GetGUID());

	// if talent known but not triggered (check priest class for speedup check)
	bool SpiritOfRedemption = false;
	if (pVictim->GetTypeId() == TYPEID_PLAYER && pVictim->GetClass() == CLASS_PRIEST)
	{
		AuraList const& vDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
		for (AuraList::const_iterator itr = vDummyAuras.begin(); itr != vDummyAuras.end(); ++itr)
		{
			if ((*itr)->GetSpellProto()->SpellIconID == 1654)
			{
				// save value before aura remove
				uint32 ressSpellId = pVictim->GetUInt32Value(PLAYER_SELF_RES_SPELL);
				if (!ressSpellId)
					ressSpellId = ((Player*)pVictim)->GetResurrectionSpellId();
				//Remove all expected to remove at death auras (most important negative case like DoT or periodic triggers)
				pVictim->RemoveAllAurasOnDeath();
				// restore for use at real death
				pVictim->SetUInt32Value(PLAYER_SELF_RES_SPELL, ressSpellId);

				// FORM_SPIRITOFREDEMPTION and related auras
				pVictim->CastSpell(pVictim, 27827, true, NULL, *itr);
				SpiritOfRedemption = true;
				break;
			}
		}
	}

	bool VengeanceSpirit = false;
	if (pVictim->GetTypeId() == TYPEID_PLAYER)
	{
		AuraList const& vDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
		for (AuraList::const_iterator itr = vDummyAuras.begin(); itr != vDummyAuras.end(); ++itr)
		{
			if ((*itr)->GetSpellProto()->Id == 40251)
			{
				// save value before aura remove if not already saved by SoR
				uint32 ressSpellId = pVictim->GetUInt32Value(PLAYER_SELF_RES_SPELL);
				if (!SpiritOfRedemption)
				{
					if (!ressSpellId)
						ressSpellId = ((Player*)pVictim)->GetResurrectionSpellId();
				}

				//Remove all expected to remove at death auras (most important negative case like DoT or periodic triggers)
				pVictim->RemoveAllAurasOnDeath();
				// restore for use at real death if not already stored
				if (!SpiritOfRedemption)
					pVictim->SetUInt32Value(PLAYER_SELF_RES_SPELL, ressSpellId);

				pVictim->CastSpell(pVictim, 40282, true);   //Possess Spirit Immune
				VengeanceSpirit = true;
				break;
			}
		}
	}

	if (Creature *creatureVictim = pVictim->ToCreature())
	{
		if (!creatureVictim->GenerateLoot(this, false))
		{
			// TODO REMOVE ME!
			
			uint32 lootid = creatureVictim->GetCreatureInfo()->lootid;
			
			sLog.outLog(LOG_SPECIAL, "GenerateLoot FALSE CASE - Boss entry %u, lootid %u, empty loot %d", creatureVictim->GetEntry(), lootid, creatureVictim->loot.empty());

			if (creatureVictim->lootForPickPocketed)
			{
				creatureVictim->lootForPickPocketed = false;
				creatureVictim->loot.clear();
			}

			if (!creatureVictim->loot.LootLoadedFromDB())
			{
				creatureVictim->loot.clear();
				if (lootid) // only new loots are dropped in heroic raids
				{
					creatureVictim->loot.setCreatureGUID(creatureVictim);
					creatureVictim->loot.FillLoot(lootid, LootTemplates_Creature, creatureVictim->GetLootRecipient(), false, creatureVictim->GetEntry());
				}
				creatureVictim->loot.generateMoneyLoot(creatureVictim->GetCreatureInfo()->mingold, creatureVictim->GetCreatureInfo()->maxgold);
			}

			// set looterGUID for round robin loot
			if (creatureVictim->GetLootRecipient() && creatureVictim->GetLootRecipient()->GetGroup())
			{
				Group *group = creatureVictim->GetLootRecipient()->GetGroup();
				group->UpdateLooterGuid(this, true);            // select next looter if one is out of xp range
				creatureVictim->loot.looterGUID = group->GetLooterGuid();
				group->UpdateLooterGuid(this, false);           // select next looter
			}
		}
	}
 
    if (!SpiritOfRedemption && !VengeanceSpirit)
        pVictim->setDeathState(JUST_DIED);

    // 10% durability loss on death
    // clean InHateListOf
    if (Player* playerVictim = pVictim->ToPlayer())
    {
        // remember victim PvP death for corpse type and corpse reclaim delay
        // at original death (not at SpiritOfRedemtionTalent timeout)
        playerVictim->SetPvPDeath(player != NULL);

        if (sWorld.getConfig(CONFIG_DURABILITY_LOSS_ON_DEATH))
        {
            // only if not player and not controlled by player pet. And not at BG
            if (durabilityLoss && !player && !playerVictim->InBattleGroundOrArena())
            {
                playerVictim->DurabilityLossAll(0.10f, false);

                // durability lost message
                WorldPacket data(SMSG_DURABILITY_DAMAGE_DEATH, 0);
                playerVictim->SendPacketToSelf(&data);
            }
        }

        // Call KilledUnit for creatures
        if (GetTypeId() == TYPEID_UNIT && ToCreature()->IsAIEnabled)
            ToCreature()->AI()->KilledUnit(pVictim);

        // last damage from non duel opponent or opponent controlled creature
        if (playerVictim->duel)
        {
            playerVictim->duel->opponent->CombatStopWithPets(true);
            playerVictim->CombatStopWithPets(true);
            playerVictim->DuelComplete(DUEL_INTERUPTED);
        }
    }
    else                                                // creature died
    {
        Creature *creatureVictim = pVictim->ToCreature();

        if (!creatureVictim->isPet())
        {
            creatureVictim->DeleteThreatList();
            CreatureInfo const* cInfo = creatureVictim->GetCreatureInfo();
			if (cInfo && (cInfo->lootid || cInfo->mingold || cInfo->maxgold))
			{
				creatureVictim->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

				// simple way to detect goldfarmers - log & parse
				if (Player* p = ToPlayer())
				{				
					if (p->creature_last_killed < time(NULL) - MINUTE && !p->IsPlayerCustomFlagged(PL_CUSTOM_MARKED_BOT))
					{
						p->creature_last_killed = time(NULL);
						sLog.outLog(LOG_FARM, "Player %s guid %u killed creature id %u", p->GetName(), p->GetGUIDLow(), creatureVictim->GetEntry());
					}
				}
					
			}
        }

        // Call KilledUnit for creatures, this needs to be called after the lootable flag is set
        if (GetTypeId() == TYPEID_UNIT && ToCreature()->IsAIEnabled)
            ToCreature()->AI()->KilledUnit(pVictim);

        // Call creature just died function
        if (creatureVictim->IsAIEnabled)
            creatureVictim->AI()->JustDied(this);

        if (creatureVictim->IsTemporarySummon()) // Report summoner about summon death
        {
            TemporarySummon* pSummon = (TemporarySummon*)creatureVictim;
            if (Creature* pSummoner = creatureVictim->GetMap()->GetCreature(pSummon->GetSummonerGuid()))
                if (pSummoner->GetTypeId() == TYPEID_UNIT)
                { pSummoner->AI()->SummonedCreatureDies(creatureVictim, this); }
        }

        // Dungeon specific stuff, only applies to players killing creatures
        if (creatureVictim->GetInstanciableInstanceId())
        {
            Map* m = creatureVictim->GetMap();

            if (m->IsDungeon() && m->GetPlayers().getSize())
            {
                // log boss kills
				std::string rank_name = creatureVictim->GetCreatureInfo()->rank == CREATURE_ELITE_WORLDBOSS ? "BOSS KILL" : "elite kill";
				std::stringstream ss;
				ss << "#" << creatureVictim->GetInstanciableInstanceId() << " [" << rank_name.c_str() << "] name: " << creatureVictim->GetName() << " entry: " << creatureVictim->GetEntry() << " guid: " << creatureVictim->GetGUIDLow()
					<< " map: " << m->GetId() << " " << m->GetMapName() << " Players in instance: ";
				for (MapRefManager::const_iterator itr = m->GetPlayers().begin(); itr != m->GetPlayers().end(); ++itr)
				{
					if (Player* ininstance = itr->getSource())
						ss << ininstance->GetName() << ":(" << ininstance->GetGUIDLow() << ") ";
				}
				sLog.outLog(LOG_LOOT, "%s", ss.str().c_str());
				sLog.outLog(LOG_RAID_ACTIONS, "%s", ss.str().c_str());

                if (m->IsRaid() || m->IsHeroic())
                {
                    if (creatureVictim->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
                        ((InstanceMap *)m)->PermBindAllPlayers();
                }
                else
                {
                    // the reset time is set but not added to the scheduler
                    // until the players leave the instance
                    time_t resettime = creatureVictim->GetRespawnTimeEx() + 2 * HOUR;
                    if (InstanceSave *save = sInstanceSaveManager.GetInstanceSave(creatureVictim->GetInstanciableInstanceId()))
                    {
                        if (save->GetResetTime() < resettime)
                            save->SetResetTime(resettime);
                    }
                }
            }
        }
    }

    pVictim->UpdateObjectVisibility();

    // outdoor pvp things, do these after setting the death state, else the player activity notify won't work... doh...
    // handle player kill only if not suicide (spirit of redemption for example)
    if (player && this != pVictim)
        if (OutdoorPvP * pvp = player->GetOutdoorPvP())
            pvp->HandleKill(player, pVictim);

    if (pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        /*
        if (OutdoorPvP * pvp = ((Player*)pVictim)->GetOutdoorPvP())
            pvp->HandlePlayerActivityChanged((Player*)pVictim);
        */

        if (Map *pMap = pVictim->GetMap())       // call OnPlayerDeath function
        {
            if (pMap->IsRaid() || pMap->IsDungeon())
            {
                if (((InstanceMap*)pMap)->GetInstanceData())
                    ((InstanceMap*)pMap)->GetInstanceData()->OnPlayerDeath((Player*)pVictim);
            }
        }

        ((Player*)this)->RemoveCharmAuras();
    }

    // battleground things (do this at the end, so the death state flag will be properly set to handle in the bg->handlekill)
    if (player && player->InBattleGroundOrArena())
    {
        if (BattleGround *bg = player->GetBattleGround())
        {
            if (pVictim->GetTypeId() == TYPEID_PLAYER)
                bg->HandleKillPlayer((Player*)pVictim, player);
            else
                bg->HandleKillUnit((Creature*)pVictim, player);
        }
    }

	if (sWorld.isEasyRealm())
	{
		if (player && player->GetMap()->IsDungeon() && pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->isWorldBoss())
			if (InstanceData* iData = GetInstanceData())
			{
				if (!iData->IsEncounterInProgress())
				{
					if (Group* tmpG = player->GetGroup())
						for (GroupReference *itr = tmpG->GetFirstMember(); itr != NULL; itr = itr->next())
						{
							Player *pl = itr->getSource();
							if (!pl || pl->isAlive() || pl->GetMapId() != player->GetMapId())
								continue;

							// before GM
							float x, y, z;
							pVictim->GetNearPoint(x, y, z, pl->GetObjectSize());
							pl->TeleportTo(pVictim->GetMapId(), x, y, z, pl->GetOrientation());

							pl->ResurrectPlayer(1.0f);
							pl->SpawnCorpseBones();
							pl->SaveToDB();
						}
				}
			}
	}
}

void Unit::SendHeartBeat()
{
    m_movementInfo.UpdateTime(WorldTimer::getMSTime());
    WorldPacket data(MSG_MOVE_HEARTBEAT, 64);
    data << GetPackGUID();
    data << m_movementInfo;
    BroadcastPacket(&data, true);
}

void Unit::SetFlying(bool apply)
{
    if (apply)
    {
        SetByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
        AddUnitMovementFlag(MOVEFLAG_FLYING| MOVEFLAG_LEVITATING);
    }
    else
    {
        RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, 0x02);
        RemoveUnitMovementFlag(MOVEFLAG_FLYING | MOVEFLAG_LEVITATING);
    }

    WorldPacket data;
    if (apply)
        data.Initialize(SMSG_MOVE_SET_CAN_FLY, 12);
    else
        data.Initialize(SMSG_MOVE_UNSET_CAN_FLY, 12);

    data << GetPackGUID();
    data << uint32(0);
    BroadcastPacket(&data, true);
}

void Unit::SetCharmedOrPossessedBy(Unit* charmer, bool possess)
{
    if (this == charmer)
        return;

    if (IsTaxiFlying())
        return;

    if (GetTypeId() == TYPEID_PLAYER && ((Player*)this)->GetTransport())
        return;

    RemoveUnitMovementFlag(MOVEFLAG_WALK_MODE);

    CastStop();

    DeleteThreatList();

    // Charmer stop charming
    if (charmer->GetObjectGuid().IsPlayer())
        charmer->ToPlayer()->StopCastingCharm();

    // Charmed stop charming
    if (GetObjectGuid().IsPlayer())
        ToPlayer()->StopCastingCharm();

    // StopCastingCharm may remove a possessed pet?
    if (!IsInWorld())
        return;

    // Set charmed
    charmer->SetCharm(this);

    // delete charmed players for threat list
    if (charmer->CanHaveThreatList())
        charmer->getThreatManager().modifyThreatPercent(this, -101);

    SetCharmerGUID(charmer->GetGUID());

    if (!charmer->IsInCombat())
        CombatStop();

    setFaction(charmer->getFaction());

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

    bool initCharmInfo = false;
    if (Creature* thisCreature = ToCreature())
    {
        thisCreature->AI()->OnCharmed(true);
        GetMotionMaster()->StopControlledMovement();

        // pets already have initialized charm info
        initCharmInfo = !GetObjectGuid().IsPet();
    }
    else if (Player *thisPlayer = ToPlayer())
    {
        if (thisPlayer->isAFK())
            thisPlayer->ToggleAFK();

        thisPlayer->SetClientControl(this, false);

        if (charmer->GetObjectGuid().IsCreature())
            thisPlayer->CharmAI(true);

        initCharmInfo = true;
    }

    if (initCharmInfo)
    {
        CharmInfo *charmInfo = InitCharmInfo();
        if (possess)
            charmInfo->InitPossessCreateSpells();
        else
            charmInfo->InitCharmCreateSpells();
    }

    //Set possessed
    if (possess)
    {
        addUnitState(UNIT_STAT_POSSESSED);
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

        if (Player* charmerPlayer = charmer->ToPlayer())
        {
            float distance = charmerPlayer->GetDistance(this);
            charmerPlayer->SetAntiCheatJustTeleported(distance);
            if (GetTypeId() == TYPEID_PLAYER)
                ToPlayer()->SetAntiCheatJustTeleported(distance);

            charmerPlayer->SetClientControl(this, true);
            charmerPlayer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

            Camera& camera = charmerPlayer->GetCamera();
            camera.SetView(this);

            charmerPlayer->SetMover(this);
        }
    }
    // Charm demon
    else if (GetTypeId() == TYPEID_UNIT && charmer->GetTypeId() == TYPEID_PLAYER && charmer->GetClass() == CLASS_WARLOCK)
    {
        CreatureInfo const *cinfo = ToCreature()->GetCreatureInfo();
        if (cinfo && cinfo->type == CREATURE_TYPE_DEMON)
        {
            //to prevent client crash
            SetFlag(UNIT_FIELD_BYTES_0, 2048);

            //just to enable stat window
            if (GetCharmInfo())
                GetCharmInfo()->SetPetNumber(sObjectMgr.GeneratePetNumber(), true);

            //if charmed two demons the same session, the 2nd gets the 1st one's name
            SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));
        }
    }

    if (Player* charmerPlayer = charmer->ToPlayer())
    {
        if (possess)
            charmerPlayer->PossessSpellInitialize();
        else if (charmer->GetTypeId() == TYPEID_PLAYER)
            charmerPlayer->CharmSpellInitialize();
    }
}

void Unit::RemoveCharmedOrPossessedBy(Unit *charmer)
{
    if (!isCharmed())
        return;

    if (!charmer)
        charmer = GetCharmer();
    else if (charmer != GetCharmer()) // one aura overrides another?
        return;

    bool possess = HasUnitState(UNIT_STAT_POSSESSED);

    CastStop();
    CombatStop(); //TODO: CombatStop(true) may cause crash (interrupt spells)
    getHostileRefManager().deleteReferences();
    DeleteThreatList();
    SetCharmerGUID(0);
    RestoreFaction();

    GetUnitStateMgr().InitDefaults(true);

    if (possess)
    {
        if (charmer && charmer->GetTypeId() == TYPEID_PLAYER)
        {
            float distance = charmer->GetDistance(this);
            charmer->ToPlayer()->SetAntiCheatJustTeleported(distance);
            if (GetTypeId() == TYPEID_PLAYER)
                ToPlayer()->SetAntiCheatJustTeleported(distance);
        }
        ClearUnitState(UNIT_STAT_POSSESSED);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    }

    if (Creature* thisCreature = ToCreature())
    {
        if (!thisCreature->isPet())
        {
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

            if (GetCharmInfo())
                thisCreature->SetReactState(GetCharmInfo()->m_oldReactState);
        }

        thisCreature->AI()->OnCharmed(false);
        if (isAlive())
        {
            if (charmer && !IsFriendlyTo(charmer))
            {
                thisCreature->AddThreat(charmer, 10000.0f);
                thisCreature->AI()->AttackStart(charmer);
                charmer->CombatStart(thisCreature);
            }
            else if (!thisCreature->isPet())
            {
                if (Unit* new_victim = thisCreature->SelectVictim())
                    thisCreature->AI()->AttackStart(new_victim);
                else
                    thisCreature->AI()->EnterEvadeMode();
            }
        }
    }
    else if (Player* thisPlayer = ToPlayer())
    {
        if (IsAIEnabled)
            thisPlayer->CharmAI(false);

        thisPlayer->SetClientControl(this, true);
    }

    // If charmer still exists
    if (!charmer)
        return;

    ASSERT(!possess || charmer->GetTypeId() == TYPEID_PLAYER); // if possess then must be player

    charmer->SetCharm(0);
    if (possess)
    {
        if (!charmer->GetObjectGuid().IsPlayer())
        {
            sLog.outLog(LOG_SPECIAL, "BANG CRASH in RemoveCharmedOrPossessedBy. GUIDcharmer: %llu, GUIDcharmerObject %llu, mapCharmer %u", charmer->GetGUID(), charmer->GetObjectGuid().GetRawValue(), charmer->GetMapId());
        }

        Player* charmerPlayer = charmer->ToPlayer();

        charmerPlayer->SetClientControl(charmer, true);
        charmerPlayer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

        Camera& camera = charmerPlayer->GetCamera();
        camera.ResetView();

        charmerPlayer->SetMover(charmer);
    }
    // restore UNIT_FIELD_BYTES_0
    else if (GetTypeId() == TYPEID_UNIT && charmer->GetTypeId() == TYPEID_PLAYER && charmer->GetClass() == CLASS_WARLOCK)
    {
        CreatureInfo const *cinfo = ((Creature*)this)->GetCreatureInfo();
        if (cinfo && cinfo->type == CREATURE_TYPE_DEMON)
        {
            CreatureDataAddon const *cainfo = ((Creature*)this)->GetCreatureAddon();
            if (cainfo && cainfo->bytes0 != 0)
                SetUInt32Value(UNIT_FIELD_BYTES_0, cainfo->bytes0);
            else
                RemoveFlag(UNIT_FIELD_BYTES_0, 2048);

            if (GetCharmInfo())
                GetCharmInfo()->SetPetNumber(0, true);
            else
                sLog.outLog(LOG_DEFAULT, "ERROR: Aura::HandleModCharm: target=%llu with typeid=%d has a charm aura but no charm info!", GetGUID(), GetTypeId());
        }
    }

    if (GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_UNIT && !((Creature*)this)->isPet())
    {
        DeleteCharmInfo();
    }

    if (possess || charmer->GetTypeId() == TYPEID_PLAYER)
    {
        // Remove pet spell action bar
        WorldPacket data(SMSG_PET_SPELLS, 8);
        data << uint64(0);
        ((Player*)charmer)->SendPacketToSelf(&data);
    }
}

void Unit::RestoreFaction()
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        Player* pl = ((Player*)this);
        if (pl->InBattleGroundOrArena() && sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION))
        {
            if (pl->GetBGTeam() == HORDE && pl->GetTeam() != HORDE)
                pl->setFactionForRace(RACE_ORC);
            else if (pl->GetBGTeam() == ALLIANCE && pl->GetTeam() != ALLIANCE)
                pl->setFactionForRace(RACE_HUMAN);
            else
                pl->setFactionForRace(GetRace());
        }
        else
            pl->setFactionForRace(GetRace());
    }
    else
    {
        CreatureInfo const *cinfo = ((Creature*)this)->GetCreatureInfo();

        if (((Creature*)this)->isPet())
        {
            if (Unit* owner = GetOwner())
                setFaction(owner->getFaction());
            else if (cinfo)
                setFaction(cinfo->faction_A);
        }
        else if (cinfo)  // normal creature
            setFaction(cinfo->faction_A);
    }
}

bool Unit::IsInPartyWith(Unit const *unit) const
{
    if (this == unit)
        return true;

    const Unit *u1 = GetCharmerOrOwnerOrSelf();
    const Unit *u2 = unit->GetCharmerOrOwnerOrSelf();
    if (u1 == u2)
        return true;

    if (u1->GetTypeId() == TYPEID_PLAYER && u2->GetTypeId() == TYPEID_PLAYER)
        return ((Player*)u1)->IsInSameGroupWith((Player*)u2);
    else
        return false;
}

bool Unit::IsInRaidWith(Unit const *unit) const
{
    if (this == unit)
        return true;

    const Unit *u1 = GetCharmerOrOwnerOrSelf();
    const Unit *u2 = unit->GetCharmerOrOwnerOrSelf();
    if (u1 == u2)
        return true;

    if (u1->GetTypeId() == TYPEID_PLAYER && u2->GetTypeId() == TYPEID_PLAYER)
        return ((Player*)u1)->IsInSameRaidWith((Player*)u2);
    else
        return false;
}

void Unit::GetRaidMember(std::list<Unit*> &nearMembers, float radius)
{
    Player *owner = GetCharmerOrOwnerPlayerOrPlayerItself();
    if (!owner)
        return;

    Group *pGroup = owner->GetGroup();
    if (!pGroup)
        return;

    for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* Target = itr->getSource();

        // IsHostileTo check duel and controlled by enemy
        if (Target && Target != this && Target->isAlive()
            && IsWithinDistInMap(Target, radius) && !IsHostileTo(Target))
            nearMembers.push_back(Target);
    }
}

void Unit::GetPartyMember(std::list<Unit*> &TagUnitMap, float radius)
{
    Unit *owner = GetCharmerOrOwnerOrSelf();
    Group *pGroup = NULL;
    if (owner->GetTypeId() == TYPEID_PLAYER)
        pGroup = ((Player*)owner)->GetGroup();

    if (pGroup)
    {
        uint8 subgroup = ((Player*)owner)->GetSubGroup();

        for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* Target = itr->getSource();

            // IsHostileTo check duel and controlled by enemy
            if (Target && Target->GetSubGroup()==subgroup && !IsHostileTo(Target))
            {
                if (!Target->duel || Target->duel && Target == ((Player*)owner))
                {
                    if (Target->isAlive() && IsWithinDistInMap(Target, radius))
                        TagUnitMap.push_back(Target);

                    if (Pet* pet = Target->GetPet())
                    {
                        if (pet->isAlive() && IsWithinDistInMap(pet, radius))
                            TagUnitMap.push_back(pet);
                    }
                }
            }
        }
    }
    else
    {
        if (owner->isAlive() && (owner == this || IsWithinDistInMap(owner, radius)))
            TagUnitMap.push_back(owner);

        if (Pet* pet = owner->GetPet())
        {
            if (pet->isAlive() && IsWithinDistInMap(pet, radius))
                TagUnitMap.push_back(pet);
        }

        // Player cannot be in party with NPC's :)
        if (owner->GetTypeId() == TYPEID_UNIT && !owner->ToCreature()->isPet())
        {
            // for Creatures, grid search friendly units in radius
            std::list<Creature*> pList;
            Hellground::AllFriendlyCreaturesInGrid u_check(owner);
            Hellground::ObjectListSearcher<Creature, Hellground::AllFriendlyCreaturesInGrid> searcher(pList, u_check);
            Cell::VisitAllObjects(owner, searcher, radius);

            for (std::list<Creature*>::iterator i = pList.begin(); i != pList.end(); ++i)
            {
                if ((*i)->GetGUID() == owner->GetGUID() || (*i)->GetGUID() == owner->GetPetGUID())
                    continue;

                //sLog.outLog(LOG_TMP, "Unit::GetPartyMember owner guid: %u, owner entry: %u, p guid: %u, p entry: %u", owner->GetGUID(), owner->GetEntry(), (*i)->GetGUID(), (*i)->GetEntry());

                TagUnitMap.push_back(*i);
            }
        }
    }
}

void Unit::AddAura(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo || (!isAlive() && !SpellMgr::IsDeathPersistentSpell(spellInfo)))
        return;

    if (IsImmunedToSpell(spellInfo))
        return;

    for (uint32 i = 0; i < 3; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA)
        {
            if (IsImmunedToSpellEffect(spellInfo->Effect[i], spellInfo->EffectMechanic[i]))
                continue;

            {
                Aura* Aur = CreateAura(spellInfo, i, NULL, this, this);
                AddAura(Aur);
            }
        }
    }
}


void Unit::AddAura(uint32 spellId, Unit* target)
{
    if (!target)
        return;

    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo || (!target->isAlive() && !SpellMgr::IsDeathPersistentSpell(spellInfo)))
        return;

    if (target->IsImmunedToSpell(spellInfo))
        return;

    for (uint32 i = 0; i < 3; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA)
        {
            if (target->IsImmunedToSpellEffect(spellInfo->Effect[i], spellInfo->EffectMechanic[i]))
                continue;

            /*if(spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_CASTER)
            {
                Aura *Aur = CreateAura(spellInfo, i, NULL, this, this);
                AddAura(Aur);
            }
            else*/
            {
                Aura *Aur = CreateAura(spellInfo, i, NULL, target, this);
                target->AddAura(Aur);
            }
        }
    }
}

void Unit::AddAura(uint32 spellId, Unit *target, uint32 duration)
{
    target->AddAura(spellId, target);
    if (Aura* a = target->GetAura(spellId, 0))
    {
        a->SetAuraDuration(duration);
        a->UpdateAuraDuration();
    }
}

void Unit::ApplyMeleeAPAttackerBonus(int32 value, bool apply)
{
    m_meleeAPAttackerBonus += apply ? value : -value;
}

void Unit::KnockBackFrom(Unit* target, float horizontalSpeed, float verticalSpeed)
{
    float angle = this == target ? GetOrientation() + M_PI : target->GetOrientationTo(this);

    KnockBack(angle, horizontalSpeed, verticalSpeed);
}

void Unit::KnockBack(float angle, float horizontalSpeed, float verticalSpeed)
{
    float vsin = sin(angle);
    float vcos = cos(angle);

    // Effect propertly implemented only for players
    if (GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_MOVE_KNOCK_BACK, 8+4+4+4+4+4);
        data << GetPackGUID();
        data << uint32(0);                                  // Sequence
        data << float(vcos);                                // x direction
        data << float(vsin);                                // y direction
        data << float(horizontalSpeed);                     // Horizontal speed
        data << float(-verticalSpeed);                      // Z Movement speed (vertical)
        ((Player*)this)->SendPacketToSelf(&data);
    }
    else
    {
        if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
            return;

        float dis = horizontalSpeed;

        float ox, oy, oz;
        GetPosition(ox, oy, oz);

        float fx = ox + dis * vcos;
        float fy = oy + dis * vsin;
        float fz = oz;

        // getObjectHitPos overwrite last args in any result case
        if (VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), ox, oy, oz + 0.5, fx, fy, oz + 0.5, ox, oy, oz, -1.0f))
        {
            fx = ox;
            fy = oy;
            fz = oz;
            UpdateGroundPositionZ(fx, fy, fz);
        }

        //FIXME: this mostly hack, must exist some packet for proper creature move at client side
        //       with CreatureRelocation at server side
        GetMap()->CreatureRelocation(((Creature*)this), fx, fy, fz, GetOrientation());
    }
}

uint32 Unit::GetSpellRadiusForTarget(Unit* target,const SpellRadiusEntry * radiusEntry)
{
    if (!radiusEntry)
        return 0;
    if (radiusEntry->radiusHostile == radiusEntry->radiusFriend)
        return radiusEntry->radiusFriend;
    if (IsHostileTo(target))
        return radiusEntry->radiusHostile;
    return radiusEntry->radiusFriend;
};

bool Unit::HasEventAISummonedUnits()
{
    if (!IsAIEnabled || !i_AI)
        return false;

    return i_AI->HasEventAISummonedUnits();
}

Unit* Unit::GetNextRandomRaidMember(float radius, bool PlayerOnly, bool LosIgnore)
{
    Player *pPlayer = GetCharmerOrOwnerPlayerOrPlayerItself();
    if (!pPlayer)
        return NULL;

    Group *pGroup = pPlayer->GetGroup();
    if (!pGroup)
        return NULL;

    std::vector<Unit*> nearMembers;
    nearMembers.reserve(pGroup->GetMembersCount()*2);

    for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* Target = itr->getSource();

        if (Target)
        {
            // IsHostileTo check duel and controlled by enemy
            if (Target != this && IsWithinDistInMap(Target, radius) &&
                !Target->HasInvisibilityAura() && !IsHostileTo(Target) &&
                !Target->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && Target->isAlive() && (LosIgnore || IsWithinLOSInMap(Target)))
                    nearMembers.push_back(Target);

            if (Pet *pet = Target->GetPet())
                if (!PlayerOnly && pet != this && IsWithinDistInMap(pet, radius) &&
                    !pet->HasInvisibilityAura() && !IsHostileTo(pet) &&
                    !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && pet->isAlive() && (LosIgnore || IsWithinLOSInMap(pet)))
                    nearMembers.push_back(pet);
        }
    }

    if (nearMembers.empty())
        return NULL;

    uint32 randTarget = urand(0,nearMembers.size()-1);
    return nearMembers[randTarget];
}

float Unit::GetDeterminativeSize() const
{
    if (!IsInWorld() || GetTypeId() != TYPEID_UNIT)
        return 0.0f;

    CreatureDisplayInfoEntry const *info = sCreatureDisplayInfoStore.LookupEntry(((Creature*)this)->GetDisplayId());
    if (!info)
        return 0.0f;

    CreatureModelDataEntry const *model = sCreatureModelDataStore.LookupEntry(info->ModelId);
    if (!model)
        return 0.0f;

    float dx = model->maxX - model->minX;
    float dy = model->maxY - model->minY;
    float dz = model->maxZ - model->minZ;
    float _size = sqrt(dx*dx + dy*dy +dz*dz) * info->scale;

    return _size;
}

void Unit::SetInFront(Unit const* target)
{
    if (!HasUnitState(UNIT_STAT_CANNOT_TURN) && (GetTypeId() != TYPEID_UNIT || !ToCreature()->hasIgnoreVictimSelection()))
        SetOrientation(GetOrientationTo(target));
}

void Unit::SetFacingTo(float ori)
{
    Movement::MoveSplineInit init(*this);
    init.SetFacing(ori);
    init.Launch();
}

void Unit::SetFacingToObject(WorldObject* pObject)
{
    // never face when already moving
    if (!IsStopped())
        return;

    // TODO: figure out under what conditions creature will move towards object instead of facing it where it currently is.
    SetFacingTo(GetOrientationTo(pObject));
}

bool Unit::IsBehindTarget(Unit const* pTarget, bool strict) const
{
	if (strict)
	{
		if (Creature const* pCreature = pTarget->ToCreature())
		{
			// Mobs always face their current victim, unless incapacitated.
			if ((pCreature->GetVictim() == this) &&
				!pTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE | UNIT_FLAG_CONFUSED | UNIT_FLAG_FLEEING | UNIT_FLAG_PLAYER_CONTROLLED) &&
				!pTarget->HasUnitState(UNIT_STAT_CANNOT_TURN))
				return false;
		}
	}

	return !pTarget->HasInArc(M_PI_F, this);
}

void Unit::SendCombatStats(uint32 flag, const char* format, Unit *pVictim, ...) const
{
    Player *target = m_GMToSendCombatStats ? GetPlayerInWorld(m_GMToSendCombatStats) : NULL;
    if(!target)
        return;

    if ((flag & m_CombatStatsFlag) == 0)
        return;

    {
        if (sWorld.getConfig(CONFIG_ENABLE_CRASHTEST) && target->GetSession()->HasPermissions(PERM_DEVELOPER) && flag == (1 << COMBAT_STATS_CRASHTEST))
            *((uint32 volatile*)NULL) = 0;
    }


    va_list ap;
    char str[1024], message[1024];
    va_start(ap, pVictim);
    vsnprintf(str, 1024, format, ap);
    if (pVictim)
        snprintf(message, 1024, "%s (%u) vs %s (%u). %s ->", GetName(), GetGUIDLow(), pVictim->GetName(), pVictim->GetGUIDLow(), str);
    else
        snprintf(message, 1024, "%s (%u). %s ->", GetName(), GetGUIDLow(), str);

    va_end(ap);

    WorldPacket data;
    uint32 messageLength = (message ? strlen(message) : 0) + 1;

    data.Initialize(SMSG_MESSAGECHAT, 100);                // guess size
    data << uint8(CHAT_MSG_SYSTEM);
    data << uint32(LANG_UNIVERSAL);
    data << uint64(0);
    data << uint32(0);
    data << uint64(0);
    data << uint32(messageLength);
    data << message;
    data << uint8(0);

    target->SendPacketToSelf(&data);
}

// This constants can't be evaluated on runtime
const float PRDConstants[] = {
0,       0.00016, 0.00062, 0.00139, 0.00245, 0.0038,  0.00544, 0.00736, 0.00955, 0.01202,
0.01475, 0.01774, 0.02098, 0.02448, 0.02823, 0.03222, 0.03645, 0.04092, 0.04562, 0.05055,
0.0557,  0.06108, 0.06668, 0.07249, 0.07851, 0.08474, 0.09118, 0.09783, 0.10467, 0.11171,
0.11895, 0.12638, 0.134,   0.14181, 0.14981, 0.15798, 0.16633, 0.17491, 0.18362, 0.19249,
0.20155, 0.21092, 0.22036, 0.2299,  0.23954, 0.24931, 0.25987, 0.27045, 0.28101, 0.29155,
0.3021,  0.31268, 0.32329, 0.33412, 0.34737, 0.3604,  0.37322, 0.38584, 0.39828, 0.41054,
0.42265, 0.4346,  0.44642, 0.4581,  0.46967, 0.48113, 0.49248, 0.50746, 0.52941, 0.55072,
0.57143, 0.59155, 0.61111, 0.63014, 0.64865, 0.66667, 0.68421, 0.7013,  0.71795, 0.73418,
0.75,    0.76543, 0.78049, 0.79518, 0.80952, 0.82353, 0.83721, 0.85057, 0.86364, 0.8764,
0.88889, 0.9011,  0.91304, 0.92473, 0.93617, 0.94737, 0.95833, 0.96907, 0.97959, 0.9899,
1 };

// Pseudo-random distribution - each subsequent fail increases chance of success in next try
// chances are floats in range (0.0, 1.0)
bool Unit::RollPRD(float baseChance, float extraChance, uint32 spellId)
{
    if (baseChance < 0)
    {
        extraChance += baseChance;
        baseChance = 0;
    }
    if (baseChance > 1)
        baseChance = 1;

    uint32 indx = uint32(baseChance * 100);
    extraChance += baseChance - indx/100.0f;
    baseChance = indx/100.0f;

    if (m_PRDMap.find(spellId) == m_PRDMap.end())
        m_PRDMap[spellId] = 0;

    ++m_PRDMap[spellId];
    if (roll_chance_f(PRDConstants[indx] * m_PRDMap[spellId] * 100))
    {
        m_PRDMap[spellId] = 0; // Reset steps BEFORE rolling extra chance
        if (extraChance < 0 && roll_chance_f(-extraChance/baseChance * 100))
            return false;
        return true;
    }

    if (extraChance > 0 && roll_chance_f(extraChance / (1 - baseChance) * 100)) // No step reseting when rolling extra chance!
        return true;

    return false;
}

#define TIMED_FEAR_AURA 55324

void Unit::SetFeared(bool apply, Unit* target, uint32 time)
{
    if (apply)
    {
        if (time)
        {
            // Target must exist if time is used. Otherwise fear would not ever be removed from unit
            if (target)
            {
                target->AddAura(TIMED_FEAR_AURA, this);
                if (Aura* aur = GetAura(TIMED_FEAR_AURA, 0))
                    aur->SetAuraDuration(time); // not sending duration update cause spell is invisible for everyone
            }
            return;
        }

        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
        
        // ssk fleeing disabled because of vmaps (or bugged GetHeight()?)
        if (target && GetTypeId() == TYPEID_UNIT && (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE) || (target->GetMapId() == 548 && !target->GetOwner())))
            GetUnitStateMgr().PushAction(UNIT_ACTION_ROOT);
        else
            GetMotionMaster()->MoveFleeing(target);

        addUnitState(UNIT_STAT_FLEEING_NOT_MOVING); // cannot set it in the action, cause action might not start cause of action with higher priority
    }
    else
    {
        // ssk fear disable movements
        if (target && target->GetMapId() == 548 && GetTypeId() == TYPEID_UNIT && !target->GetOwner())
            GetUnitStateMgr().DropAction(UNIT_ACTION_ROOT);
        
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);
        GetUnitStateMgr().DropAction(UNIT_ACTION_FEARED);
        // when action is being dropped - it drops all states, no need to check for other fears - there are no fears
        ClearUnitState(UNIT_STAT_FLEEING_NOT_MOVING);

        // fix for HandlePreventFleeing
        if (target && target->HasAuraType(SPELL_AURA_MOD_FEAR))
            target->RemoveSpellsCausingAura(SPELL_AURA_MOD_FEAR);

        if (GetTypeId() == TYPEID_PLAYER)
            SetSelection(((Player*)this)->GetSavedSelection()); // restore target on fear remove
    }

    if (GetTypeId() == TYPEID_PLAYER)
    {
        ToPlayer()->SetClientControl(this, !apply);
        ToPlayer()->SetAntiCheatJustTeleported();
    }
}

void Unit::SetConfused(bool apply)
{
    if (apply)
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
        GetMotionMaster()->MoveConfused();
        addUnitState(UNIT_STAT_CONFUSED_NOT_MOVING);
    }
    else
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
        GetUnitStateMgr().DropAction(UNIT_ACTION_CONFUSED);
        ClearUnitState(UNIT_STAT_CONFUSED_NOT_MOVING);
        if (GetTypeId() == TYPEID_PLAYER)
            SetSelection(((Player*)this)->GetSavedSelection()); // restore target on confuse remove
    }

    if (GetTypeId() == TYPEID_PLAYER)
    {
        ToPlayer()->SetClientControl(this, !apply);
        ToPlayer()->SetAntiCheatJustTeleported();
    }
}

void Unit::SetStunned(bool apply)
{
    if (apply)
        GetUnitStateMgr().PushAction(UNIT_ACTION_STUN);
    else
        GetUnitStateMgr().DropAction(UNIT_ACTION_STUN);

    if (GetTypeId() == TYPEID_PLAYER)
        ToPlayer()->SetAntiCheatJustTeleported();
}

void Unit::SetRooted(bool apply)
{
    if (apply)
        GetUnitStateMgr().PushAction(UNIT_ACTION_ROOT);
	else
	{
		GetUnitStateMgr().DropAction(UNIT_ACTION_ROOT);
		RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);
	}
    if (GetTypeId() == TYPEID_PLAYER)
        ToPlayer()->SetAntiCheatJustTeleported();
}

void Unit::SendSpellVisual(uint32 kitID)
{
    // SpellVisual.dbc 4 column
    
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << uint64(GetGUID());
    data<< uint32(kitID);
    BroadcastPacket(&data, true);
}

bool Unit::isInSanctuary()
{
    const AreaTableEntry *area = GetAreaEntryByAreaID(GetAreaId());
    if (area && area->flags & AREA_FLAG_SANCTUARY)
        return true;

    return false;
}

// called from all movement in MovementHandler.cpp
void Unit::ActivateFoundTrapsIfNeeded(Position const & lineStart, Position const & lineEnd, float distance)
{
    if (!isAlive() || HasFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE)))
        return;

    std::list<GameObject*> tmpL;
    Hellground::AllReadyTrapsInRange go_check(this, distance);
    Hellground::ObjectListSearcher<GameObject, Hellground::AllReadyTrapsInRange> searcher(tmpL, go_check);

    Cell::VisitGridObjects(this, searcher, distance);

    float steps = distance/2.0f; // minimum 4 step. Last position is not counted cause it will be updated anyway in GameObject::Update, first too. We only need those inbetween
    // found all objects. - then we check if those objects are WithinDistInMap to a certain position - positions that we have gotten throught dividing start and end position of player
    float stepX = (lineEnd.x - lineStart.x) / steps;
    float stepY = (lineEnd.y - lineStart.y) / steps;
    float stepZ = (lineEnd.z - lineStart.z) / steps;
    uint8 intSteps = uint8(steps); 
    Position* test = new Position[uint8(intSteps)];
    for (uint8 i = 0; i < intSteps; ++i)
    {
        test[i].x = lineStart.x + stepX * (i+1);
        test[i].y = lineStart.y + stepY * (i+1);
        test[i].z = lineStart.z + stepZ * (i+1);
    }
    // if 10 yds - there will be two steps, on 2,4,6 and 8 yds from start. Hunter traps have radius 2.5 + 1.5 will be object boundary. So we check every 2 yds
    for (std::list<GameObject*>::const_iterator itr = tmpL.begin(); itr != tmpL.end(); ++itr)
    {
        GameObject * tmp = *itr;
        if (tmp)
        {
            GameObjectInfo const* goInfo = tmp->GetGOInfo();
            float radius = float(goInfo->trap.diameter)/2;

            if (!radius)
            {
                // try to read radius from trap spell
                if (const SpellEntry *spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(goInfo->trap.spellId))
                    radius = SpellMgr::GetSpellRadius(spellEntry,0,false);
            }
            for (uint8 j = 0; j < intSteps; ++j)
            {
                if (radius > tmp->GetDistance(test[j].x, test[j].y, test[j].z) + GetObjectSize())
                {
                    tmp->CastSpell(this, goInfo->trap.spellId);
                    tmp->m_cooldownTime = WorldTimer::getMSTime() + (goInfo->trap.cooldown ? goInfo->trap.cooldown : 4)*MILLISECONDS;  // default 4 sec cooldown??
                    tmp->SendCustomAnimation();

                    if (goInfo->trap.charges != 0)
                        tmp->SetLootState(GO_JUST_DEACTIVATED);  // can be despawned or destroyed
                    break; // from radius 'for' -> go back to traps
                }
            }
        }
    }
    delete []test;
}

void Unit::ApplyAdditionalSpell(Unit* caster, const SpellEntry* spellInfo)
{
    if (GetTypeId() == TYPEID_PLAYER || (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->isPet()))              // Negative buff should only be applied on players
    {
        if (!spellInfo)
            return;

        uint32 spellId = 0;
        if (spellInfo->CasterAuraStateNot == AURA_STATE_WEAKENED_SOUL || spellInfo->TargetAuraStateNot == AURA_STATE_WEAKENED_SOUL)
            spellId = 6788;                                 // Weakened Soul
        else if (spellInfo->CasterAuraStateNot == AURA_STATE_FORBEARANCE || spellInfo->TargetAuraStateNot == AURA_STATE_FORBEARANCE)
            spellId = 25771;                                // Forbearance
        else if (spellInfo->CasterAuraStateNot == AURA_STATE_HYPOTHERMIA)
            spellId = 41425;                                // Hypothermia
        else if (spellInfo->Mechanic == MECHANIC_BANDAGE) // Bandages
            spellId = 11196;                                // Recently Bandaged
        else if ((spellInfo->AttributesEx & 0x20) && (spellInfo->AttributesEx2 & 0x20000))
            spellId = 23230;                                // Blood Fury - Healing Reduction

        SpellEntry const *AdditionalSpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        if (AdditionalSpellEntry)
        {
            // applied at target by caster
            Aura* AdditionalAura = CreateAura(AdditionalSpellEntry, 0, NULL, this, caster, 0);
            AddAura(AdditionalAura);
            sLog.outDebug("Spell: Additional Aura is: %u", AdditionalSpellEntry->EffectApplyAuraName[0]);
        }
    }
}

bool Unit::InClassForm()
{
    // clean fix for drud forms + morph shirts?
    if (GetTypeId() != TYPEID_PLAYER)
        return false;

    switch (m_form)
    {
    case FORM_CAT:
    case FORM_TREE:
    case FORM_TRAVEL:
    case FORM_AQUA:
    case FORM_BEAR:
    case FORM_DIREBEAR:
    case FORM_CREATUREBEAR:
    case FORM_GHOSTWOLF:
    case FORM_FLIGHT:
    case FORM_FLIGHT_EPIC:
    case FORM_MOONKIN:
        return true;
    }

    return false;
}

// moonwell

float Unit::SpellSpecialMod(uint32 spell_id)
{
    // see @!tanks_boost
    
    float dmg_multiplier = 1.0f;

    // moonwell special damage multipliers to revive some classes like ambush rogues
    // muti boost
    switch (spell_id)
    {
        // Ambush
    case 8725:
    case 8724:
    case 8676:
    case 11269:
    case 11268:
    case 11267:
    case 27441:
        dmg_multiplier = 1.17f;
        break;
        // Mutilate
    case 1329:
    case 5374:
    case 27576:
    case 34411:
    case 34412:
    case 34413:
    case 34414:
    case 34415:
    case 34416:
    case 34417:
    case 34418:
    case 34419:
        dmg_multiplier = 1.27f;
        break;
        // Backstab
    case 53:
    case 2589:
    case 2590:
    case 2591:
    case 8721:
    case 11279:
    case 11280:
    case 11281:
    case 25300:
    case 26863:
        dmg_multiplier = 1.1f;
        break;
        // Envenom
    case 32645:
    case 32684:
        dmg_multiplier = 1.2f;
        break;
        // Warrior Revenge
    case 6572:
    case 6574:
    case 7379:
    case 11600:
    case 11601:
    case 25288:
    case 25269:
    case 30357:
        Map * map = GetMap();
        if (map && map->IsDungeon())
        {
            // check if Shield Block exists
            float val = HasAura(2565) ? 75.f : 0.f;
            dmg_multiplier = 1.f + (((GetFloatValue(PLAYER_BLOCK_PERCENTAGE) - val) + GetFloatValue(PLAYER_DODGE_PERCENTAGE) + GetFloatValue(PLAYER_PARRY_PERCENTAGE)) * (float(int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) / 100.f) * 2.f) / 100.f;
            if (dmg_multiplier < 1.0f)
                dmg_multiplier = 1.0f;
        }
        break;
    }

    return dmg_multiplier;
}