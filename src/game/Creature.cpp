// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "ScriptMgr.h"
#include "Creature.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "Player.h"
#include "PoolManager.h"
#include "Opcodes.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapManager.h"
#include "CreatureAI.h"
#include "PlayerAI.h"
#include "CreatureAISelector.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "WaypointMovementGenerator.h"
#include "InstanceData.h"
#include "BattleGroundMgr.h"
#include "Util.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "OutdoorPvPMgr.h"
#include "GameEvent.h"
#include "CreatureGroups.h"

#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

// apply implementation of the singletons
#include "Map.h"

std::map<uint32, uint32> CreatureAIReInitialize;

TrainerSpell const* TrainerSpellData::Find(uint32 spell_id) const
{
    TrainerSpellMap::const_iterator itr = spellList.find(spell_id);
    if (itr != spellList.end())
        return &itr->second;

    return NULL;
}

bool VendorItemData::RemoveItem(uint32 item_id)
{
    for (VendorItemList::iterator i = m_items.begin(); i != m_items.end(); ++i)
    {
        if ((*i)->item==item_id)
        {
            m_items.erase(i);
            return true;
        }
    }
    return false;
}

size_t VendorItemData::FindItemSlot(uint32 item_id) const
{
    for (size_t i = 0; i < m_items.size(); ++i)
        if (m_items[i]->item==item_id)
            return i;
    return m_items.size();
}

VendorItem const* VendorItemData::FindItem(uint32 item_id) const
{
    for (VendorItemList::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
        if ((*i)->item==item_id)
            return *i;
    return NULL;
}

uint32 CreatureInfo::GetRandomValidModelId() const
{
    uint32 c = 0;
    uint32 modelIDs[4];

    if (Modelid_A1) modelIDs[c++] = Modelid_A1;
    if (Modelid_A2) modelIDs[c++] = Modelid_A2;
    if (Modelid_H1) modelIDs[c++] = Modelid_H1;
    if (Modelid_H2) modelIDs[c++] = Modelid_H2;

    return ((c>0) ? modelIDs[urand(0,c-1)] : 0);
}

uint32 CreatureInfo::GetFirstValidModelId() const
{
    if (Modelid_A1) return Modelid_A1;
    if (Modelid_A2) return Modelid_A2;
    if (Modelid_H1) return Modelid_H1;
    if (Modelid_H2) return Modelid_H2;
    return 0;
}

bool AssistDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Unit* victim = Unit::GetUnit(m_owner, m_victim);
    if (victim)
    {
        while (!m_assistants.empty())
        {
            Creature* assistant = Unit::GetCreature(m_owner, *m_assistants.begin());
            m_assistants.pop_front();

            if (assistant && assistant->CanAssistTo(&m_owner, victim))
            {
                assistant->SetNoCallAssistance(true);
                assistant->CombatStart(victim);
                if (assistant->IsAIEnabled)
                    assistant->AI()->AttackStart(victim);
            }
        }
    }
    return true;
}

bool ForcedDespawnDelayEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    m_owner.ForcedDespawn();
    return true;
}

Creature::Creature() :
Unit(), m_aggroRange(0.0), m_ignoreSelection (false), IsDistancToHomeEvadable(true),
lootForPickPocketed(false), lootForBody(false), m_lootMoney(0), m_lootRecipient(0),
m_deathTimer(0), m_respawnTime(0), m_respawnDelay(300), m_corpseDelay(60), m_respawnradius(0.0f),
m_gossipOptionLoaded(false), m_isPet(false), m_isTotem(false), m_reactState(REACT_AGGRESSIVE),
m_defaultMovementType(IDLE_MOTION_TYPE), m_equipmentId(0), m_AlreadyCallAssistance(false),
m_regenHealth(true), m_isDeadByDefault(false), m_AlreadySearchedAssistance(false), m_creatureData(NULL),
m_meleeDamageSchoolMask(SPELL_SCHOOL_MASK_NORMAL),m_creatureInfo(NULL), m_DBTableGuid(0), m_formation(NULL), m_PlayerDamageReq(0),
m_tempSummon(false), ModIsInHeroicRaid(false), _IsDoZoneInCombatThreat(false), pulled(false)
{
    m_regenTimer = 0;
    m_valuesCount = UNIT_END;
    m_xpMod = 0;
    m_hpMod = 0;
    m_dmgMod = 0;
    
    for (int i =0; i < CREATURE_MAX_SPELLS; ++i)
        m_spells[i] = 0;

    m_CreatureSpellCooldowns.clear();
    m_CreatureCategoryCooldowns.clear();
    m_PlayersAllowedToLoot.clear();
    //m_MotherAccsAllowedToLoot.clear();

    DisableReputationGain = false;
    m_path_id = 0;
    db_emote = 0;
}

Creature::~Creature()
{
    m_vendorItemCounts.clear();

    if (i_AI)
    {
        delete i_AI;
        i_AI = NULL;
    }

    if (m_uint32Values)
        sLog.outDetail("Deconstruct Creature Entry = %u", GetEntry());
}

void Creature::AddToWorld()
{
    ///- Register the creature for guid lookup
    if (!IsInWorld())
    {
        GetMap()->InsertIntoObjMap(this);
        Unit::AddToWorld();
        SearchFormation();
        AIM_Initialize();

        if (m_zoneScript)
            m_zoneScript->OnCreatureCreate(this, true);
    }
}

void Creature::RemoveFromWorld()
{
    ///- Remove the creature from the accessor
    if (IsInWorld())
    {
        if (m_zoneScript)
            m_zoneScript->OnCreatureCreate(this, false);

        CreatureGroupManager::LeaveGroupIfHas(this);

        Unit::RemoveFromWorld();
        GetMap()->RemoveFromObjMap(this);
    }
}

void Creature::SearchFormation()
{
    if (isPet())
        return;

    CreatureGroupManager::AddCreatureToGroupIfNeeded(this);
}

void Creature::RemoveCorpse()
{
    if (getDeathState()!=CORPSE && !m_isDeadByDefault || getDeathState()!=ALIVE && m_isDeadByDefault)
        return;

    setDeathState(DEAD);
    m_deathTimer = 0; // must be before UpdateObjectVisibility() so it will destroy the creature's corpse for players.
    UpdateObjectVisibility();
    loot.clear();

	if (!isPet())
	{
		float x, y, z, o;
		GetRespawnCoord(x, y, z, &o);
		SetHomePosition(x, y, z, o);
		Relocate(x, y, z, o);
	}
}

/**
 * change the entry of creature until respawn
 */
bool Creature::InitEntry(uint32 Entry, PlayerTeam team, const CreatureData *data, GameEventCreatureData const* eventData)
{
    // use game event entry if any instead default suggested
    if (eventData && eventData->entry_id)
        Entry = eventData->entry_id;

    CreatureInfo const *normalInfo = ObjectMgr::GetCreatureTemplate(Entry);
    if (!normalInfo)
    {
        sLog.outLog(LOG_DB_ERR, "Creature::UpdateEntry creature entry %u does not exist.", Entry);
        return false;
    }

    // get heroic mode entry
    uint32 actualEntry = Entry;
    CreatureInfo const *cinfo = normalInfo;

    if (normalInfo->HeroicEntry)
    {
        Map *map = GetMap();
        if(map)
        {
            if (map->IsHeroic())
                cinfo = ObjectMgr::GetCreatureTemplate(normalInfo->HeroicEntry);

            if (!cinfo)
            {
                sLog.outLog(LOG_NOTIFY, "Creature::UpdateEntry creature heroic entry %u does not exist.", actualEntry);
                return false;
            }
        }
    }

	//if (GetMap()->IsHeroicRaid() && !normalInfo->HeroicEntry)
	//	sLog.outLog(LOG_NOTIFY, "Creature::UpdateEntry no heroic entry for %u in IsHeroicRaid()", actualEntry);

    SetEntry(Entry);                                        // normal entry always
    m_creatureInfo = cinfo;                                 // map mode related always

    // Cancel load if no model defined
    if (!(cinfo->GetFirstValidModelId()))
    {
        sLog.outLog(LOG_DB_ERR, "Creature (Entry: %u) has no model defined in table `creature_template`, can't load. ",Entry);
        return false;
    }

    uint32 display_id = sObjectMgr.ChooseDisplayId(team, GetCreatureInfo(), data, eventData);
    CreatureModelInfo const *minfo = sObjectMgr.GetCreatureModelRandomGender(display_id);
    if (!minfo)
    {
        sLog.outLog(LOG_DB_ERR, "Creature (Entry: %u) has model %u not found in table `creature_model_info`, can't load. ", Entry, display_id);
        return false;
    }
    else
        display_id = minfo->modelid;                        // it can be different (for another gender)

    SetDisplayId(display_id);
    SetNativeDisplayId(display_id);
    SetByteValue(UNIT_FIELD_BYTES_0, 2, minfo->gender);

    // Load creature equipment
    if (eventData && eventData->equipment_id)
    {
        LoadEquipment(eventData->equipment_id);             // use event equipment if any for active event
    }
    else if (!data || data->equipmentId == 0)
    {                                                       // use default from the template
        if (cinfo->equipmentId == 0)
            LoadEquipment(normalInfo->equipmentId);         // use default from normal template if diff does not have any
        else
            LoadEquipment(cinfo->equipmentId);              // else use from diff template
    }
    else if (data && data->equipmentId != -1)
    {                                                       // override, -1 means no equipment
        LoadEquipment(data->equipmentId);
    }

    SetName(normalInfo->Name);                              // at normal entry always
	m_guid = GetGUIDLow();
	m_entry = Entry;

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS,minfo->bounding_radius);
    SetFloatValue(UNIT_FIELD_COMBATREACH,minfo->combat_reach);

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    float m_baseSpeed = GetBaseSpeed();
    SetSpeed(MOVE_WALK, m_baseSpeed);
    SetSpeed(MOVE_RUN,  m_baseSpeed);
    SetSpeed(MOVE_SWIM, m_baseSpeed);

    if (IsPet())
    {
        UpdateSpeed(MOVE_RUN, true);
        UpdateSpeed(MOVE_SWIM, true);
        UpdateSpeed(MOVE_FLIGHT, true);
    }

    SetFloatValue(OBJECT_FIELD_SCALE_X, cinfo->scale);
    SetLevitate(CanFly());

    // checked at loading
    m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
    if (!m_respawnradius && m_defaultMovementType == RANDOM_MOTION_TYPE)
        m_defaultMovementType = IDLE_MOTION_TYPE;

    m_spells[0] = GetCreatureInfo()->spell1;
    m_spells[1] = GetCreatureInfo()->spell2;
    m_spells[2] = GetCreatureInfo()->spell3;
    m_spells[3] = GetCreatureInfo()->spell4;
    m_spells[4] = GetCreatureInfo()->spell5;
    m_spells[5] = GetCreatureInfo()->spell6;

    return true;
}

bool Creature::UpdateEntry(uint32 Entry, PlayerTeam team, const CreatureData *data, GameEventCreatureData const* eventData)
{
    if (!InitEntry(Entry,team,data,eventData))
        return false;

    ModIsInHeroicRaid = GetInstanciableInstanceId() ? GetMap()->IsHeroicRaid() : false;

    m_regenHealth = GetCreatureInfo()->RegenHealth;

    // creatures always have melee weapon ready if any
    SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
    SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_AURAS);

    SelectLevel(GetCreatureInfo());
    if (team == HORDE)
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetCreatureInfo()->faction_H);
    else
        SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetCreatureInfo()->faction_A);

    if (GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT)
        SetUInt32Value(UNIT_NPC_FLAGS,GetCreatureInfo()->npcflag | sGameEventMgr.GetNPCFlag(this));
    else
        SetUInt32Value(UNIT_NPC_FLAGS,GetCreatureInfo()->npcflag);

    SetAttackTime(BASE_ATTACK,  GetCreatureInfo()->baseattacktime);
    SetAttackTime(OFF_ATTACK,   GetCreatureInfo()->baseattacktime);
    SetAttackTime(RANGED_ATTACK,GetCreatureInfo()->rangeattacktime);

    SetUInt32Value(UNIT_FIELD_FLAGS,GetCreatureInfo()->unit_flags);
    SetUInt32Value(UNIT_DYNAMIC_FLAGS,GetCreatureInfo()->dynamicflags);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    SetMeleeDamageSchool(SpellSchools(GetCreatureInfo()->dmgschool));
    SetModifierValue(UNIT_MOD_ARMOR,             BASE_VALUE, float(GetCreatureInfo()->armor));
    if (GetCreatureInfo()->resistance1 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_HOLY, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_HOLY, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_HOLY, BASE_VALUE, float(GetCreatureInfo()->resistance1));

    if (GetCreatureInfo()->resistance2 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_FIRE, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_FIRE, BASE_VALUE, float(GetCreatureInfo()->resistance2));

    if (GetCreatureInfo()->resistance3 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(GetCreatureInfo()->resistance3));

    if (GetCreatureInfo()->resistance4 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_FROST, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_FROST, BASE_VALUE, float(GetCreatureInfo()->resistance4));

    if (GetCreatureInfo()->resistance5 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(GetCreatureInfo()->resistance5));

    if (GetCreatureInfo()->resistance6 < 0)
    {
        ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
        SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, 0);
    }
    else
        SetModifierValue(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(GetCreatureInfo()->resistance6));

    SetCanModifyStats(true);
    UpdateAllStats();

    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(GetCreatureInfo()->faction_A);
    if (factionTemplate)                                    // check and error show at loading templates
    {
        FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction);
        if (factionEntry)
            if (!(GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN) &&
                (factionEntry->team == ALLIANCE || factionEntry->team == HORDE))
                SetPvP(true);
    }

    // HACK: trigger creature is always not selectable
    if (isTrigger())
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    if (isTotem() || isTrigger()
        || GetCreatureType() == CREATURE_TYPE_CRITTER)
        SetReactState(REACT_PASSIVE);
    /*else if (isCivilian())
        SetReactState(REACT_DEFENSIVE);*/
    else
        SetReactState(REACT_AGGRESSIVE);

    if (GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_TAUNT)
    {
        ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        ApplySpellImmune(0, IMMUNITY_EFFECT,SPELL_EFFECT_ATTACK_ME, true);
    }

    // if eventData set then event active and need apply spell_start
    if (eventData)
        ApplyGameEventSpells(eventData, true);

    return true;
}

void Creature::Update(uint32 update_diff, uint32 diff)
{
    if (IsAIEnabled && GetAIName() == "RadiusAI")
        return;
    
    switch (m_deathState)
    {
        case JUST_ALIVED:
            // Don't must be called, see Creature::setDeathState JUST_ALIVED -> ALIVE promoting.
            sLog.outLog(LOG_DEFAULT, "ERROR: Creature (GUIDLow: %u Entry: %u) in wrong state: JUST_ALIVED (4)",GetGUIDLow(),GetEntry());
            break;
        case JUST_DIED:
            // Don't must be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
            sLog.outLog(LOG_DEFAULT, "ERROR: Creature (GUIDLow: %u Entry: %u) in wrong state: JUST_DEAD (1)",GetGUIDLow(),GetEntry());
            break;
        case DEAD:
        {
            if (m_respawnTime <= time(NULL))
            {
                // encounter in progress - don't respawn
                if (GetMap()->EncounterInProgress(NULL))
                {
                    SetRespawnTime(MINUTE);
                    SaveRespawnTime();
                }
                else if (!GetLinkedCreatureRespawnTime()) // Can respawn
                    Respawn();
                else // the master is dead
                {
                    if (uint32 targetGuid = sObjectMgr.GetLinkedRespawnGuid(m_DBTableGuid))
                    {
                        if (targetGuid == m_DBTableGuid) // if linking self, never respawn (check delayed to next day)
                            SetRespawnTime(DAY);
                        else
                            m_respawnTime = (time(NULL)>GetLinkedCreatureRespawnTime()? time(NULL):GetLinkedCreatureRespawnTime())+urand(5,MINUTE); // else copy time from master and add a little
                        SaveRespawnTime(); // also save to DB immediately
                    }
                    else
                        Respawn();
                }
            }
            break;
        }
        case CORPSE:
        {
            Unit::Update(update_diff, diff);

            if (m_isDeadByDefault)
                break;

			if (m_deathTimer <= time(NULL))
            {
                RemoveCorpse();
                debug_log("Removing corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));

                // Not dead by default for sure. Go through pool update if needed
                if (uint16 poolid = sPoolMgr.IsPartOfAPool<Creature>(GetGUIDLow()))
                    sPoolMgr.UpdatePool<Creature>(poolid, GetGUIDLow());
            }

            if (loot.looterTimer && loot.looterTimer < time(NULL))
            {
                loot.looterTimer = 0;
                loot.looterGUID = 0;
                if (GetLootRecipient() && GetLootRecipient()->GetGroup())
                    GetLootRecipient()->GetGroup()->SendRoundRobin(&loot, this);
            }
            if (loot.looterGUID && loot.looterCheckTimer < time(NULL))
            {
                Player* player = GetPlayerInWorld(loot.looterGUID);
                if (!player || !IsWithinDist(player, sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE), false))
                {
                    loot.looterTimer = 0;
                    loot.looterGUID = 0;
                    if (GetLootRecipient() && GetLootRecipient()->GetGroup())
                        GetLootRecipient()->GetGroup()->SendRoundRobin(&loot, this);
                }
                else
                    loot.looterCheckTimer = time(NULL) + 1; // 1 second
            }

            break;
        }
        case ALIVE:
        {
            if (m_isDeadByDefault)
            {
				if (m_deathTimer <= time(NULL))
                {
                    RemoveCorpse();
                    debug_log("Removing alive corpse... %u ", GetUInt32Value(OBJECT_FIELD_ENTRY));
                }
            }

            Unit::Update(update_diff, diff);

            // creature can be dead after Unit::Update call
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!isAlive())
                break;

            // if creature is charmed, switch to charmed AI
            if (NeedChangeAI)
            {
                UpdateCharmAI();
                NeedChangeAI = false;
                IsAIEnabled = true;
            }

            if (!(isTotem() || isPet() || IsTemporarySummon() || IS_CREATURE_GUID(GetOwnerGUID()) && GetOwner()->ToCreature()->isTotem()) && GetMap() && !GetMap()->IsDungeon() && !AI()->IsEscorted() && 
                GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE &&
                GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE && IsDistancToHomeEvadable) // IsDistanceEvadable USE ONLY for timing mobs or event purposes.
            {
                uint32 distToHome = sWorld.getConfig(CONFIG_EVADE_HOMEDIST) * 3; // Don't know for sure why do we even have this function - anyway atleast let it be x3 of the normal evade distance while out of combat
                if (!IsWithinDistInMap(&homeLocation, distToHome))
                    if (isWorldBoss() || (time(NULL) - GetLastDamagedTime() >= 10)) // can't kite world bosses
                        AI()->EnterEvadeMode(); // Trentone do we really need this? try to remove or log or do something with it. it bugs-out long-distance moving creatures.
            }

            if ((!sWorld.IsShutdowning() || sWorld.GetShutdownTimer() >= BOSS_AUTOEVADE_RESTART_TIME) && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_RESTART_PACIFIED))
                ToggleFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_RESTART_PACIFIED);

            if (!IsInEvadeMode() && IsAIEnabled)
            {
                // do not allow the AI to be changed during update
                m_AI_locked = true;
                i_AI->UpdateAI(diff);
                m_AI_locked = false;
            }

            // Trentone says: Some scripts make creatures kill themself - and then they're not in combat - thus dynamicflags are set to normal - which should not happen
            // creature can be dead after UpdateAI call (example: Kalecgos / Illidan)
            // CORPSE/DEAD state will processed at next tick (in other case death timer will be updated unexpectedly)
            if (!isAlive())
                break;

            m_regenTimer += update_diff;
            if (m_regenTimer < 2000)
                break;

            CombatTimerTick(update_diff);

            if (!IsInCombat())
            {
                
                if (HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER))
                    SetUInt32Value(UNIT_DYNAMIC_FLAGS, GetCreatureInfo()->dynamicflags);

                RegenerateHealth();
            }
            else if (IsPolymorphed())
                RegenerateHealth();

            RegenerateMana();

            m_regenTimer -= 2000;
            break;
        }
        default:
            break;
    }

    if (m_aiReinitializeCheckTimer <= update_diff)
    {
        if (!IsInCombat() && !IsInEvadeMode() && !isCharmed() && m_aiInitializeTime < CreatureAIReInitialize[GetEntry()])
            AIM_Initialize();

        m_aiReinitializeCheckTimer = 10000;
    }
    else
        m_aiReinitializeCheckTimer -= update_diff;

    for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
        m_schoolCooldowns[i].Update(diff);
}

void Creature::RegenerateMana()
{
    uint32 curValue = GetPower(POWER_MANA);
    uint32 maxValue = GetMaxPower(POWER_MANA);

    if (curValue >= maxValue)
        return;

    if(GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NOT_REGEN_MANA)
        return;

    uint32 addvalue = 0;

    // Combat and any controlled creature
    if (IsInCombat() || (GetCharmerOrOwnerGUID() && GetEntry() != 10928))
    {
        if (!IsUnderLastManaUseEffect())
        {
            float ManaIncreaseRate = sWorld.getConfig(RATE_POWER_MANA);
            float Spirit = GetStat(STAT_SPIRIT);

            addvalue = uint32((Spirit/5.0f + 17.0f) * ManaIncreaseRate);
        }
    }
    else
        addvalue = maxValue/3;

    ModifyPower(POWER_MANA, addvalue);
}

void Creature::RegenerateHealth()
{
    if (!isRegeneratingHealth())
        return;

    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue)
        return;

    if(GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NOT_REGEN_HEALTH)
        return;

    uint32 addvalue = 0;

    // Not only pet, but any controlled creature
    if (GetCharmerOrOwnerGUID() && GetEntry() != 10928)
    {
        float HealthIncreaseRate = sWorld.getConfig(RATE_HEALTH);
        float Spirit = GetStat(STAT_SPIRIT);

        if (GetPower(POWER_MANA) > 0)
            addvalue = uint32(Spirit * 0.25 * HealthIncreaseRate);
        else
            addvalue = uint32(Spirit * 0.80 * HealthIncreaseRate);
    }
    else
        addvalue = maxValue/3;

    ModifyHealth(addvalue);
}

bool Creature::AIM_Initialize(CreatureAI* ai)
{
    // make sure nothing can change the AI during AI update
    if (m_AI_locked)
    {
        sLog.outDebug("AIM_Initialize: failed to init, locked.");
        return false;
    }

    UnitAI * oldAI = i_AI;

    GetMotionMaster()->Initialize();
    i_AI = FactorySelector::selectAI(this);

    if (oldAI)
        delete oldAI;

    IsAIEnabled = true;
    i_AI->InitializeAI();
    m_aiInitializeTime = WorldTimer::getMSTime();

    return true;
}

bool Creature::Create(uint32 guidlow, Map *map, uint32 Entry, PlayerTeam team, float x, float y, float z, float ang, const CreatureData *data, GameEventCreatureData const* eventData)
{
    Relocate(x, y, z, ang);
    if (!IsPositionValid())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Creature (guidlow %d, entry %d) not created. Suggested coordinates isn't valid (X: %f Y: %f)",guidlow,Entry,x,y);
        return false;
    }

    SetMapId(map->GetId());
    SetInstanceId(map->GetAnyInstanceId());

    SetMap(map);

    //oX = x;     oY = y;    dX = x;    dY = y;    m_moveTime = 0;    m_startMove = 0;
    const bool bResult = CreateFromProto(guidlow, Entry, team, data, eventData);

    if (bResult)
    {
        switch (GetCreatureInfo()->rank)
        {
            case CREATURE_ELITE_RARE:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_RARE);
                break;
            case CREATURE_ELITE_ELITE:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_ELITE);
                break;
            case CREATURE_ELITE_RAREELITE:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_RAREELITE);
                break;
            case CREATURE_ELITE_WORLDBOSS:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_WORLDBOSS);
                break;
            default:
                m_corpseDelay = sWorld.getConfig(CONFIG_CORPSE_DECAY_NORMAL);
                break;
        }

        Map *map = GetMap();
        if (map->IsDungeon() && GetCreatureInfo()->rank != 0 && GetCreatureInfo()->rank != 3) { // not bosses and normals
            m_corpseDelay = m_corpseDelay * 4; // for example, bosses in MGT has Rare type, so make their corpses lasts longer
        }

        LoadCreaturesAddon();
        if (GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_HASTE_IMMUNE)
            ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
    }
    return bResult;
}

bool Creature::isCanTrainingOf(Player* pPlayer, bool msg) const
{
    if (!isTrainer())
        return false;

    TrainerSpellData const* trainer_spells = GetTrainerSpells();

    if (!trainer_spells || trainer_spells->spellList.empty())
    {
        sLog.outLog(LOG_DB_ERR, "Creature %u (Entry: %u) have UNIT_NPC_FLAG_TRAINER but have empty trainer spell list.",
            GetGUIDLow(),GetEntry());
        return false;
    }

    switch (GetCreatureInfo()->trainer_type)
    {
        case TRAINER_TYPE_CLASS:
            if (pPlayer->GetClass()!=GetCreatureInfo()->classNum)
            {
                if (msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureInfo()->classNum)
                    {
                        case CLASS_DRUID:  pPlayer->PlayerTalkClass->SendGossipMenu(4913,GetGUID()); break;
                        case CLASS_HUNTER: pPlayer->PlayerTalkClass->SendGossipMenu(10090,GetGUID()); break;
                        case CLASS_MAGE:   pPlayer->PlayerTalkClass->SendGossipMenu( 328,GetGUID()); break;
                        case CLASS_PALADIN:pPlayer->PlayerTalkClass->SendGossipMenu(1635,GetGUID()); break;
                        case CLASS_PRIEST: pPlayer->PlayerTalkClass->SendGossipMenu(4436,GetGUID()); break;
                        case CLASS_ROGUE:  pPlayer->PlayerTalkClass->SendGossipMenu(4797,GetGUID()); break;
                        case CLASS_SHAMAN: pPlayer->PlayerTalkClass->SendGossipMenu(5003,GetGUID()); break;
                        case CLASS_WARLOCK:pPlayer->PlayerTalkClass->SendGossipMenu(5836,GetGUID()); break;
                        case CLASS_WARRIOR:pPlayer->PlayerTalkClass->SendGossipMenu(4985,GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_PETS:
            if (pPlayer->GetClass()!=CLASS_HUNTER)
            {
                pPlayer->PlayerTalkClass->ClearMenus();
                pPlayer->PlayerTalkClass->SendGossipMenu(3620,GetGUID());
                return false;
            }
            break;
        case TRAINER_TYPE_MOUNTS:
            if (GetCreatureInfo()->race && pPlayer->GetRace() != GetCreatureInfo()->race)
            {
                if (msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    switch (GetCreatureInfo()->classNum)
                    {
                        case RACE_DWARF:        pPlayer->PlayerTalkClass->SendGossipMenu(5865,GetGUID()); break;
                        case RACE_GNOME:        pPlayer->PlayerTalkClass->SendGossipMenu(4881,GetGUID()); break;
                        case RACE_HUMAN:        pPlayer->PlayerTalkClass->SendGossipMenu(5861,GetGUID()); break;
                        case RACE_NIGHTELF:     pPlayer->PlayerTalkClass->SendGossipMenu(5862,GetGUID()); break;
                        case RACE_ORC:          pPlayer->PlayerTalkClass->SendGossipMenu(5863,GetGUID()); break;
                        case RACE_TAUREN:       pPlayer->PlayerTalkClass->SendGossipMenu(5864,GetGUID()); break;
                        case RACE_TROLL:        pPlayer->PlayerTalkClass->SendGossipMenu(5816,GetGUID()); break;
                        case RACE_UNDEAD_PLAYER:pPlayer->PlayerTalkClass->SendGossipMenu(624,GetGUID()); break;
                        case RACE_BLOODELF:     pPlayer->PlayerTalkClass->SendGossipMenu(5862,GetGUID()); break;
                        case RACE_DRAENEI:      pPlayer->PlayerTalkClass->SendGossipMenu(5864,GetGUID()); break;
                    }
                }
                return false;
            }
            break;
        case TRAINER_TYPE_TRADESKILLS:
            if (GetCreatureInfo()->trainer_spell && !pPlayer->HasSpell(GetCreatureInfo()->trainer_spell))
            {
                if (msg)
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    pPlayer->PlayerTalkClass->SendGossipMenu(11031,GetGUID());
                }
                return false;
            }
            break;
        default:
            return false;                                   // checked and error output at creature_template loading
    }
    return true;
}

bool Creature::isCanInteractWithBattleMaster(Player* pPlayer, bool msg) const
{
    if (!isBattleMaster())
        return false;

    BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(GetEntry());
    if (!msg)
        return pPlayer->GetBGAccessByLevel(bgTypeId);

    if (sendBgNotAvailableByLevel(pPlayer, bgTypeId))
        return false;

    return true;
}

bool Creature::sendBgNotAvailableByLevel(Player* pPlayer, BattleGroundTypeId bgTypeId) const
{
    if (!pPlayer->GetBGAccessByLevel(bgTypeId))
    {
        pPlayer->PlayerTalkClass->ClearMenus();
        switch (bgTypeId)
        {
            case BATTLEGROUND_AV:  pPlayer->PlayerTalkClass->SendGossipMenu(7616, GetGUID()); break;
            case BATTLEGROUND_WS:  pPlayer->PlayerTalkClass->SendGossipMenu(7599, GetGUID()); break;
            case BATTLEGROUND_AB:  pPlayer->PlayerTalkClass->SendGossipMenu(7642, GetGUID()); break;
            case BATTLEGROUND_EY:
            case BATTLEGROUND_NA:
            case BATTLEGROUND_BE:
            case BATTLEGROUND_AA:
            case BATTLEGROUND_RL:  pPlayer->PlayerTalkClass->SendGossipMenu(10024, GetGUID()); break;
        }
        return true;
    }

    return false;
}

bool Creature::isCanTrainingAndResetTalentsOf(Player* pPlayer) const
{
    return pPlayer->GetLevel() >= 10
        && GetCreatureInfo()->trainer_type == TRAINER_TYPE_CLASS
        && pPlayer->GetClass() == GetCreatureInfo()->classNum;
}

void Creature::prepareGossipMenu(Player *pPlayer,uint32 gossipid)
{
    //Prevent gossip from NPCs that are possessed.
    Unit* Charmed = Unit::GetCharmer();
    if (Charmed)
        return;

    // don't prepere menu for dueling players
    if (pPlayer->duel)
        return;

    PlayerMenu* pm=pPlayer->PlayerTalkClass;
    pm->ClearMenus();

    // lazy loading single time at use
    LoadGossipOptions();

    for (GossipOptionList::iterator i = m_goptions.begin(); i != m_goptions.end(); ++i)
    {
        GossipOption* gso=&*i;
        if (gso->GossipId == gossipid)
        {
            bool cantalking=true;
            if (gso->Id==1)
            {
                uint32 textid=GetNpcTextId();
                GossipText const * gossiptext=sObjectMgr.GetGossipText(textid);
                if (!gossiptext)
                    cantalking=false;
            }
            else
            {
                switch (gso->Action)
                {
                    case GOSSIP_OPTION_QUESTGIVER:
                        pPlayer->PrepareQuestMenu(GetGUID());
                        //if (pm->GetQuestMenu()->MenuItemCount() == 0)
                        cantalking=false;
                        //pm->GetQuestMenu()->ClearMenu();
                        break;
                    case GOSSIP_OPTION_ARMORER:
                        cantalking=false;                   // added in special mode
                        break;
                    case GOSSIP_OPTION_SPIRITHEALER:
                        if (!pPlayer->isDead())
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_VENDOR:
                    {                     
                        VendorItemData const* vItems = GetVendorItems();
                        if (!vItems || vItems->Empty())
                        {
                            if (!sWorld.custom_vendor[GetEntry()].empty())
                                break;
                            
                            sLog.outLog(LOG_DB_ERR, "Creature %u (Entry: %u) have UNIT_NPC_FLAG_VENDOR but have empty trading item list.",
                                GetGUIDLow(),GetEntry());
                            cantalking=false;
                        }
                        break;
                    }
                    case GOSSIP_OPTION_TRAINER:
                        if (!isCanTrainingOf(pPlayer,false))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_UNLEARNTALENTS:
                    case GOSSIP_OPTION_BUY_FREE_RESPEC:
                        if (!isCanTrainingAndResetTalentsOf(pPlayer))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_UNLEARNPETSKILLS:
                        if (!pPlayer->GetPet() || pPlayer->GetPet()->getPetType() != HUNTER_PET || pPlayer->GetPet()->m_spells.size() <= 1 || GetCreatureInfo()->trainer_type != TRAINER_TYPE_PETS || GetCreatureInfo()->classNum != CLASS_HUNTER)
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_TAXIVENDOR:
                        if (pPlayer->GetSession()->SendLearnNewTaxiNode(this))
                            return;
                        break;
                    case GOSSIP_OPTION_BATTLEFIELD:
                        if (!isCanInteractWithBattleMaster(pPlayer,false))
                            cantalking=false;
                        break;
                    case GOSSIP_OPTION_SPIRITGUIDE:
                    case GOSSIP_OPTION_INNKEEPER:
                    case GOSSIP_OPTION_BANKER:
                    case GOSSIP_OPTION_PETITIONER:
                    case GOSSIP_OPTION_STABLEPET:
                    case GOSSIP_OPTION_TABARDDESIGNER:
                    case GOSSIP_OPTION_AUCTIONEER:
                        break;                              // no checks
                    case GOSSIP_OPTION_OUTDOORPVP:
                        if (!sOutdoorPvPMgr.CanTalkTo(pPlayer,this,(*gso)))
                            cantalking = false;
                        break;
                    default:
                        sLog.outLog(LOG_DB_ERR, "Creature %u (entry: %u) have unknown gossip option %u",GetDBTableGUIDLow(),GetEntry(),gso->Action);
                        break;
                }
            }

            //note for future dev: should have database fields for BoxMessage & BoxMoney
            if (!gso->OptionText.empty() && cantalking)
            {
                //std::string OptionText = gso->OptionText;
                //std::string BoxText = gso->BoxText;
                //int loc_idx = pPlayer->GetSession()->GetSessionDbLocaleIndex();
                //if (loc_idx >= 0)
                //{
                //    NpcOptionLocale const *no = sObjectMgr.GetNpcOptionLocale(gso->Id);
                //    if (no)
                //    {
                //        if (no->OptionText.size() > loc_idx && !no->OptionText[loc_idx].empty())
                //            OptionText=no->OptionText[loc_idx];
                //        if (no->BoxText.size() > loc_idx && !no->BoxText[loc_idx].empty())
                //            BoxText=no->BoxText[loc_idx];
                //    }
                //}

                //std::string boxtext = gso->BoxText;
                //sObjectMgr.GetNpcOptionLocaleStrings(gso->Id, loc_idx, &boxtext);

                pm->GetGossipMenu().AddMenuItem((uint8)gso->Icon, pPlayer->GetSession()->GetNpcOptionLocaleString(gso->Id), gossipid, gso->Action, NULL/*boxtext*/, gso->BoxMoney, gso->Coded);
            }
        }
    }

    ///some gossips aren't handled in normal way ... so we need to do it this way .. TODO: handle it in normal way ;-)
    if (pm->Empty())
    {
        if (HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER))
        {
            isCanTrainingOf(pPlayer,true);                  // output error message if need
        }
        if (HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_BATTLEMASTER))
        {
            isCanInteractWithBattleMaster(pPlayer,true);     // output error message if need
        }
    }
}

void Creature::sendPreparedGossip(Player* player)
{
    if (!player)
        return;

    if (GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT) // if world event npc then
        sGameEventMgr.HandleWorldEventGossip(player, this);      // update world state with progress

    // in case no gossip flag and quest menu not empty, open quest menu (client expect gossip menu with this flag)
    if (!HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_GOSSIP) && !player->PlayerTalkClass->GetQuestMenu().Empty())
    {
        player->SendPreparedQuest(GetGUID());
        return;
    }

    // in case non empty gossip menu (that not included quests list size) show it
    // (quest entries from quest menu will be included in list)
                                              // error is sent if (trainer cant train you) or (bg regger cant reg you). In this case not sending gossip menu
    if (!player->PlayerTalkClass->Empty() || !((isTrainer() && !isCanTrainingOf(player, true)) || (isBattleMaster() && !isCanInteractWithBattleMaster(player, true))))
        player->PlayerTalkClass->SendGossipMenu(GetNpcTextId(), GetGUID());
}

void Creature::OnGossipSelect(Player* player, uint32 option)
{
    // player can't select gossip if in duel
    if (player->duel)
        return;

    GossipMenu& gossipmenu = player->PlayerTalkClass->GetGossipMenu();

    if (option >= gossipmenu.MenuItemCount())
        return;

    uint32 action=gossipmenu.GetItem(option).m_gAction;
    uint32 zoneid=GetZoneId();
    uint64 guid=GetGUID();

    GossipOption const *gossip=GetGossipOption(action);
    if (!gossip)
    {
        zoneid=0;
        gossip=GetGossipOption(action);
        if (!gossip)
            return;
    }

    switch (gossip->Action)
    {
        case GOSSIP_OPTION_GOSSIP:
        {
            uint32 textid = GetGossipTextId(action, zoneid);
            if (textid == 0)
                textid=GetNpcTextId();

            player->PlayerTalkClass->CloseGossip();
            player->PlayerTalkClass->SendTalking(textid);
            break;
        }
        case GOSSIP_OPTION_OUTDOORPVP:
            sOutdoorPvPMgr.HandleGossipOption(player, GetGUID(), option);
            break;
        case GOSSIP_OPTION_SPIRITHEALER:
            if (player->isDead())
                CastSpell(this,17251,true,NULL,NULL,player->GetGUID());
            break;
        case GOSSIP_OPTION_QUESTGIVER:
            player->PrepareQuestMenu(guid);
            player->SendPreparedQuest(guid);
            break;
        case GOSSIP_OPTION_VENDOR:
        case GOSSIP_OPTION_ARMORER:
            player->GetSession()->SendListInventory(guid);
            break;
        case GOSSIP_OPTION_STABLEPET:
            player->GetSession()->SendStablePet(guid);
            break;
        case GOSSIP_OPTION_TRAINER:
            player->GetSession()->SendTrainerList(guid);
            break;
        case GOSSIP_OPTION_UNLEARNTALENTS:
            player->PlayerTalkClass->CloseGossip();
            player->SendTalentWipeConfirm(guid);
            break;
        case GOSSIP_OPTION_UNLEARNPETSKILLS:
            player->PlayerTalkClass->CloseGossip();
            player->SendPetSkillWipeConfirm();
            break;
        case GOSSIP_OPTION_TAXIVENDOR:
            player->GetSession()->SendTaxiMenu(this);
            break;
        case GOSSIP_OPTION_INNKEEPER:
            player->PlayerTalkClass->CloseGossip();
            player->SetBindPoint(guid);
            break;
        case GOSSIP_OPTION_BANKER:
            player->GetSession()->SendShowBank(guid);
            break;
        case GOSSIP_OPTION_PETITIONER:
            player->PlayerTalkClass->CloseGossip();
            player->GetSession()->SendPetitionShowList(guid);
            break;
        case GOSSIP_OPTION_TABARDDESIGNER:
            player->PlayerTalkClass->CloseGossip();
            player->GetSession()->SendTabardVendorActivate(guid);
            break;
        case GOSSIP_OPTION_AUCTIONEER:
            player->GetSession()->SendAuctionHello(this);
            break;
        case GOSSIP_OPTION_SPIRITGUIDE:
        case GOSSIP_GUARD_SPELLTRAINER:
        case GOSSIP_GUARD_SKILLTRAINER:
            prepareGossipMenu(player,gossip->Id);
            sendPreparedGossip(player);
            break;
        case GOSSIP_OPTION_BATTLEFIELD:
        {
            BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(GetEntry());
            player->GetSession()->SendBattlegGroundList(ObjectGuid(GetGUID()), bgTypeId);
            break;
        }
        default:
            OnPoiSelect(player, gossip);
            break;
    }

}

void Creature::OnPoiSelect(Player* player, GossipOption const *gossip)
{
    if (gossip->GossipId==GOSSIP_GUARD_SPELLTRAINER || gossip->GossipId==GOSSIP_GUARD_SKILLTRAINER)
    {
        Poi_Icon icon = ICON_POI_0;
        //need add more case.
        switch (gossip->Action)
        {
            case GOSSIP_GUARD_BANK:
                icon=ICON_POI_HOUSE;
                break;
            case GOSSIP_GUARD_RIDE:
                icon=ICON_POI_RWHORSE;
                break;
            case GOSSIP_GUARD_GUILD:
                icon=ICON_POI_BLUETOWER;
                break;
            default:
                icon=ICON_POI_TOWER;
                break;
        }
        uint32 textid = GetGossipTextId(gossip->Action, GetZoneId());
        player->PlayerTalkClass->SendTalking(textid);
        // std::string areaname= gossip->OptionText;
        // how this could worked player->PlayerTalkClass->SendPointOfInterest(x, y, icon, 2, 15, areaname.c_str());
    }
}

uint32 Creature::GetGossipTextId(uint32 action, uint32 zoneid)
{
    QueryResultAutoPtr result= GameDataDatabase.PQuery("SELECT textid FROM npc_gossip_textid WHERE action = '%u' AND zoneid ='%u'", action, zoneid);

    if (!result)
        return 0;

    Field *fields = result->Fetch();
    uint32 id = fields[0].GetUInt32();

    return id;
}

uint32 Creature::GetNpcTextId()
{
    // don't cache / use cache in case it's a world event announcer
    if (GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_WORLDEVENT)
        if (uint32 textid = sGameEventMgr.GetNpcTextId(m_DBTableGuid))
            return textid;

    if (!m_DBTableGuid)
        return DEFAULT_GOSSIP_MESSAGE;

    if (uint32 pos = sObjectMgr.GetNpcGossip(m_DBTableGuid))
        return pos;

    return DEFAULT_GOSSIP_MESSAGE;
}

GossipOption const* Creature::GetGossipOption(uint32 id) const
{
    for (GossipOptionList::const_iterator i = m_goptions.begin(); i != m_goptions.end(); ++i)
    {
        if (i->Action==id)
            return &*i;
    }
    return NULL;
}

void Creature::ResetGossipOptions()
{
    m_gossipOptionLoaded = false;
    m_goptions.clear();
}

void Creature::LoadGossipOptions()
{
    if (m_gossipOptionLoaded)
        return;

    uint32 npcflags=GetUInt32Value(UNIT_NPC_FLAGS);

    CacheNpcOptionList const& noList = sObjectMgr.GetNpcOptions ();
    for (CacheNpcOptionList::const_iterator i = noList.begin (); i != noList.end (); ++i)
        if (i->NpcFlag & npcflags)
            addGossipOption(*i);

    m_gossipOptionLoaded = true;
}

bool Creature::IsPlayerAllowedToLoot(Player *player) const 
{
    if (m_PlayersAllowedToLoot.empty())
        return true;

    return m_PlayersAllowedToLoot.find(player->GetGUID()) != m_PlayersAllowedToLoot.end();
}

Player *Creature::GetLootRecipient()
{
    if (!m_lootRecipient)
        return NULL;
    else
    {
        Player* plr = sObjectAccessor.GetPlayerInWorld(m_lootRecipient);
        if (!plr && HasPlayersAllowedToLoot())
        {
            Player* test;
            for (std::set<uint64>::const_iterator i = m_PlayersAllowedToLoot.begin(); i != m_PlayersAllowedToLoot.end(); ++i)
            {
                test = sObjectAccessor.GetPlayerInWorld(*i);
                if (test && test->GetAnyInstanceId() == GetAnyInstanceId() && test->GetGroup()) // this is the player we need, he is in instance with us and in group
                {
                    m_lootRecipient = test->GetGUID();
                    plr = test;
                    break;
                }
            }
        }
        return plr;
    }
}

void Creature::SetLootRecipient(Unit *unit)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to NULL after the loot disappears

    if (!unit)
    {
        loot.RemoveSavedLootFromDB();
		loot.clear();

        m_lootRecipient = 0;
        m_PlayersAllowedToLoot.clear();
		pulled = false;

        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);

        return;
    }

    Player* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
    if (!player)                                             // normal creature, no player involved
        return;

    m_lootRecipient = player->GetGUID();

    // special case for world bosses
    Group* group = player->GetGroup();

	// isWorldBoss() replaced, because we need to save m_PlayersAllowedToLoot for elite bosses in dungeons
	Map* map = GetMap();

    if (group && (map->IsDungeon() || isWorldBoss()))
    {
        if (GetUnit(group->GetLeaderGUID()))
            m_lootRecipient = group->GetLeaderGUID();
        else if (GetUnit(group->GetLooterGuid()))
            m_lootRecipient = group->GetLooterGuid();

        Map* map = GetMap();
        if (map && map->IsDungeon())
        {
            //bool accLoot = sWorld.isEasyRealm();
            Map::PlayerList const &PlayerList = map->GetPlayers();
            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                if (Player* i_pl = i->getSource())
                    if (i_pl->GetGroup() == player->GetGroup())
                    {
                        m_PlayersAllowedToLoot.insert(i_pl->GetGUID());
                        //if (accLoot)
                        //    m_MotherAccsAllowedToLoot.insert(i_pl->GetSession()->GetMotherAccId());
                    }
        }
    }

    SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_OTHER_TAGGER);
}

void Creature::SaveUpdateToDB()
{
    // this should only be used when the creature has already been loaded
    // preferably after adding to map, because mapid may not be valid otherwise
    CreatureData const *data = sObjectMgr.GetCreatureData(m_DBTableGuid);
    if (!data)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Creature::SaveToDB failed, cannot get creature data!");
        return;
    }

    SaveUpdateOrNewToDB_npc_add(GetMapId(), data->spawnMask);
}

void Creature::SaveUpdateOrNewToDB_npc_add(uint32 mapid, uint8 spawnMask)
{
    // update in loaded data
    if (!m_DBTableGuid)
        m_DBTableGuid = GetGUIDLow();

    CreatureData& data = sObjectMgr.NewOrExistCreatureData(m_DBTableGuid);

    uint32 displayId = GetNativeDisplayId();

    // check if it's a custom model and if not, use 0 for displayId
    CreatureInfo const *cinfo = GetCreatureInfo();
    if (cinfo)
    {
        if (displayId == cinfo->Modelid_A1 || displayId == cinfo->Modelid_A2 ||
            displayId == cinfo->Modelid_H1 || displayId == cinfo->Modelid_H2) displayId = 0;
    }

    // data->guid = guid don't must be update at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.displayid = displayId;
    data.equipmentId = GetEquipmentId();
    data.posX = GetPositionX();
    data.posY = GetPositionY();
    data.posZ = GetPositionZ();
    data.orientation = GetOrientation();
    data.spawntimesecs = m_respawnDelay;
    // prevent add data integrity problems
    data.spawndist = GetDefaultMovementType() == IDLE_MOTION_TYPE ? 0 : m_respawnradius;
    data.currentwaypoint = 0;
    data.curhealth = GetHealth();
    data.curmana = GetPower(POWER_MANA);
    data.is_dead = m_isDeadByDefault;
    // prevent add data integrity problems
    data.movementType = !m_respawnradius && GetDefaultMovementType() == RANDOM_MOTION_TYPE ? IDLE_MOTION_TYPE : GetDefaultMovementType();
    data.spawnMask = spawnMask;
    data.sInstId = sObjectMgr.GetSingleInstance(mapid, data.posX, data.posY);

    // updated in DB
    GameDataDatabase.BeginTransaction();

    static SqlStatementID saveCreature;
    static SqlStatementID deleteCreature;

    SqlStatement stmt = GameDataDatabase.CreateStatement(deleteCreature, "DELETE FROM creature WHERE guid = ?");
    stmt.PExecute(m_DBTableGuid);

    stmt = GameDataDatabase.CreateStatement(saveCreature, "INSERT INTO creature VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    stmt.addUInt64(m_DBTableGuid);
    stmt.addUInt32(data.id);
    stmt.addUInt32(data.mapid);
    stmt.addUInt32(data.spawnMask);
    stmt.addUInt32(data.displayid);
    stmt.addUInt32(data.equipmentId);
    stmt.addFloat(data.posX);
    stmt.addFloat(data.posY);
    stmt.addFloat(data.posZ);
    stmt.addFloat(data.orientation);
    stmt.addUInt32(data.spawntimesecs);
    stmt.addFloat(m_respawnradius);
    stmt.addUInt32(data.currentwaypoint);
    stmt.addUInt32(data.curhealth);
    stmt.addUInt32(data.curmana);
    stmt.addUInt32(data.is_dead ? 1 : 0);
    stmt.addUInt32(GetDefaultMovementType());

    stmt.Execute();

    GameDataDatabase.CommitTransaction();
}

void Creature::SelectLevel(const CreatureInfo *cinfo)
{
    uint32 rank = isPet()? 0 : cinfo->rank;

    // disabled because premium robot and etc.
    //if (isPet())
    //    sLog.outLog(LOG_CHEAT, "Pet creature WTF? id %u", GetEntry());

    // level
    uint32 minlevel = std::min(cinfo->maxlevel, cinfo->minlevel);
    uint32 maxlevel = std::max(cinfo->maxlevel, cinfo->minlevel);
    uint32 level = minlevel == maxlevel ? minlevel : urand(minlevel, maxlevel);
    SetLevel(level);

    float rellevel = maxlevel == minlevel ? 0 : (float(level - minlevel))/(maxlevel - minlevel);
    
    m_hpMod = cinfo->hpMod;
    m_dmgMod = cinfo->dmgMod;

    // health
    float healthmod = GetCreatureHealthMod();
    float damagemod = GetCreatureDamageMod();

    // pet health/damage reinitializes later, i suppose

    uint32 minhealth = std::min(cinfo->maxhealth, cinfo->minhealth);
    uint32 maxhealth = std::max(cinfo->maxhealth, cinfo->minhealth);
	uint32 health = (rellevel == 0 ? uint32(healthmod * urand(minhealth, maxhealth)) : uint32(healthmod * (minhealth + uint32(rellevel*(maxhealth - minhealth)))));
    if (health < 1)
        health = 1;

    SetCreateHealth(health);
    SetMaxHealth(health);
    SetHealth(health);
    ResetPlayerDamageReq();

    // mana
    uint32 minmana = std::min(cinfo->maxmana, cinfo->minmana);
    uint32 maxmana = std::max(cinfo->maxmana, cinfo->minmana);
    uint32 mana = minmana + uint32(rellevel*(maxmana - minmana));

    SetCreateMana(mana);
    SetMaxPower(POWER_MANA, mana);                          //MAX Mana
    SetPower(POWER_MANA, mana);

    SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, health);
    SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, mana);

	SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg * damagemod);
	SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg * damagemod);
	SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, cinfo->mindmg * damagemod);
	SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, cinfo->maxdmg * damagemod);
	SetBaseWeaponDamage(RANGED_ATTACK, MINDAMAGE, cinfo->minrangedmg * damagemod);
	SetBaseWeaponDamage(RANGED_ATTACK, MAXDAMAGE, cinfo->maxrangedmg * damagemod);

    // this value is not accurate, but should be close to the real value
    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, level * 5);
    SetModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, level * 5);
    //SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, cinfo->attackpower * damagemod);
    //SetModifierValue(UNIT_MOD_ATTACK_POWER_RANGED, BASE_VALUE, cinfo->rangedattackpower * damagemod);
}

bool Creature::CreateFromProto(uint32 guidlow, uint32 Entry, PlayerTeam team, const CreatureData *data, GameEventCreatureData const* eventData)
{
    SetZoneScript();
    if (m_zoneScript && data)
    {
        Entry = m_zoneScript->GetCreatureEntry(guidlow, data);
        if (!Entry)
            return false;
    }

    CreatureInfo const *cinfo = ObjectMgr::GetCreatureTemplate(Entry);
    if (!cinfo)
    {
        sLog.outLog(LOG_DB_ERR, "Error: creature entry %u does not exist.", Entry);
        return false;
    }
    m_originalEntry = Entry;

    Object::_Create(guidlow, Entry, HIGHGUID_UNIT);

    if (!UpdateEntry(Entry, team, data, eventData))
        return false;

    return true;
}

bool Creature::LoadFromDB(uint32 guid, Map *map)
{
    CreatureData const* data = sObjectMgr.GetCreatureData(guid);

    if (!data)
    {
        sLog.outLog(LOG_DB_ERR, "Creature (GUID: %u) not found in table `creature`, can't load. ",guid);
        return false;
    }

    if (!map->GetInstanciableInstanceId() && data->sInstId != map->GetAnyInstanceId())
        return false;

    GameEventCreatureData const* eventData = sGameEventMgr.GetCreatureUpdateDataForActiveEvent(guid);
    m_DBTableGuid = guid;
    if (map->GetInstanciableInstanceId() != 0)
        guid = sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT);

    if (!Create(guid,map,data->id,TEAM_NONE,data->posX,data->posY,data->posZ,data->orientation,data, eventData))
        return false;

    //We should set first home position, because then AI calls home movement
    SetHomePosition(data->posX, data->posY, data->posZ, data->orientation);

    m_respawnradius = data->spawndist;

    m_respawnDelay = data->spawntimesecs;
    m_isDeadByDefault = data->is_dead;
    m_deathState = m_isDeadByDefault ? DEAD : ALIVE;

    m_respawnTime = sObjectMgr.GetCreatureRespawnTime(m_DBTableGuid, GetInstanciableInstanceId());
    if (m_respawnTime)                          // respawn on Update
    {
        m_deathState = DEAD;
        if (isWorldBoss() && map->IsDungeon())
            loot.FillLootFromDB(this, NULL);

        if (CanFly())
        {
            float tz = GetTerrain()->GetHeight(data->posX,data->posY,data->posZ,false);
            if (data->posZ - tz > 0.1)
                Relocate(data->posX,data->posY,tz);
        }
    }

    uint32 curhealth = data->curhealth;
    if (curhealth)
    {
        curhealth = uint32(curhealth) * GetCreatureHealthMod();
        if (curhealth < 1)
            curhealth = 1;
    }

    SetHealth(m_deathState == ALIVE ? curhealth : 0);
    SetPower(POWER_MANA,data->curmana);

    // checked at creature_template loading
    m_defaultMovementType = MovementGeneratorType(data->movementType);

    if (CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(data->id))
        m_xpMod = info->xpMod;
    else
        m_xpMod = 0.0f;

    m_creatureData = data;

    // check if it is rabbit day
    //if (isAlive() && sWorld.getConfig(CONFIG_RABBIT_DAY))
    //{
    //    time_t rabbit_day = time_t(sWorld.getConfig(CONFIG_RABBIT_DAY));
    //    tm rabbit_day_tm = *localtime(&rabbit_day);
    //    tm now_tm = *localtime(&sWorld.GetGameTime());

    //    if (now_tm.tm_mon == rabbit_day_tm.tm_mon && now_tm.tm_mday == rabbit_day_tm.tm_mday)
    //        CastSpell(this, 10710 + urand(0, 2), true);
    //}

    return true;
}

void Creature::LoadEquipment(uint32 equip_entry, bool force)
{
    if (equip_entry == 0)
    {
        if (force)
        {
            for (uint8 i = 0; i < 3; i++)
            {
                SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, 0);
                SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2), 0);
                SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, 0);
            }
            m_equipmentId = 0;
        }
        return;
    }

    EquipmentInfo const *einfo = sObjectMgr.GetEquipmentInfo(equip_entry);
    if (!einfo)
        return;

    m_equipmentId = equip_entry;
    for (uint8 i = 0; i < 3; i++)
    {
        SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + i, einfo->equipmodel[i]);
        SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2), einfo->equipinfo[i]);
        SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + (i * 2) + 1, einfo->equipslot[i]);
    }
}

bool Creature::hasQuest(uint32 quest_id) const
{
    QuestRelations const& qr = sObjectMgr.mCreatureQuestRelations;
    for (QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if (itr->second == quest_id)
            return true;
    }
    return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id) const
{
    QuestRelations const& qr = sObjectMgr.mCreatureQuestInvolvedRelations;
    for (QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
    {
        if (itr->second==quest_id)
            return true;
    }
    return false;
}

void Creature::DeleteFromDB_npc_del()
{
    if (!m_DBTableGuid)
    {
        sLog.outDebug("Trying to delete not saved creature!");
        return;
    }
	
    sObjectMgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanciableInstanceId(), GetMapId(), GetMap()->IsRaid(), 0);
    sObjectMgr.DeleteCreatureData(m_DBTableGuid);

    static SqlStatementID deleteCreature;
    static SqlStatementID deleteCreatureAddon;
    static SqlStatementID deleteGECreature;
    static SqlStatementID deleteGEModelEquip;

    GameDataDatabase.BeginTransaction();
    SqlStatement stmt = GameDataDatabase.CreateStatement(deleteCreature, "DELETE FROM creature WHERE guid = ?");
    stmt.PExecute(m_DBTableGuid);

    stmt = GameDataDatabase.CreateStatement(deleteCreatureAddon, "DELETE FROM creature_addon WHERE guid = ?");
    stmt.PExecute(m_DBTableGuid);

    stmt = GameDataDatabase.CreateStatement(deleteGECreature, "DELETE FROM game_event_creature WHERE guid = ?");
    stmt.PExecute(m_DBTableGuid);

    stmt = GameDataDatabase.CreateStatement(deleteGEModelEquip, "DELETE FROM game_event_creature_data WHERE guid = ?");
    stmt.PExecute(m_DBTableGuid);
    GameDataDatabase.CommitTransaction();
}

bool Creature::canSeeOrDetect(Unit const* u, WorldObject const* viewPoint, bool detect, bool inVisibleList, bool is3dDistance) const
{
    // not in world
    if (!IsInWorld() || !u->IsInWorld())
        return false;

    // all dead creatures/players not visible for any creatures
    if (!u->isAlive() || !isAlive())
        return false;

    // Always can see self
    if (u == this)
        return true;

    // always seen by owner
    if (GetGUID() == u->GetCharmerOrOwnerGUID())
        return true;

    if (u->GetVisibility() == VISIBILITY_OFF) //GM
        return false;

    if (u->GetObjectGuid().IsAnyTypeCreature() && u->ToCreature()->IsAIEnabled && !u->ToCreature()->AI()->IsVisible())
        return false;

    // invisible aura
    if ((m_invisibilityMask || u->m_invisibilityMask) && !canDetectInvisibilityOf(u))
        return false;

    // unit got in stealth in this moment and must ignore old detected state
    //if (m_Visibility == VISIBILITY_GROUP_NO_DETECT)
    //    return false;

    // GM invisibility checks early, invisibility if any detectable, so if not stealth then visible
    if (u->GetVisibility() == VISIBILITY_GROUP_STEALTH)
    {
        //do not know what is the use of this detect
        if (!detect || !canDetectStealthOf(u, viewPoint, viewPoint->GetExactDist(u)))
            return false;
    }

    // Now check is target visible with LoS
    //return u->IsWithinLOS(GetPositionX(),GetPositionY(),GetPositionZ());
    return true;
}

bool Creature::IsWithinSightDist(Unit const* u) const
{
    if (!IsWithinDistInMap(u, sWorld.getConfig(CONFIG_SIGHT_MONSTER)))
        return false;

    return IsWithinLOSInMap(u);
}

bool Creature::canStartAttack(Unit const* who) const
{
	// add  || who->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE) ?
	if (isCivilian() || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE) || !who->isAlive() || IsInEvadeMode())
		return false;

	if (!IsWithinDistInMap(who, GetAttackDistance(who)))
		return false;	

	if (!canAttack(who, false))
		return false;

	//if (!CanFly() && who->isFlying() && GetDistanceZ(who) > (!((Unit*)this)->inRangedAttackMode() ? 15.0f : CREATURE_Z_ATTACK_RANGE))
	//if (!CanFly() && who->isFlying() && GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
	//	return false;

	//PathFinder path(who);
	//bool result = path.calculate(who->GetPositionX(), who->GetPositionY(), who->GetPositionZ());
	//if (!result || !(path.getPathType() & PATHFIND_NORMAL))
	//	return false;

    if (!who->isInAccessiblePlacefor(this)) // CPU-HUNGRY
		return false;

    if(!IsWithinLOSInMap(who))
		return false;

	return true;
}

uint8 Creature::ReachForAutoAttackErrorCode(Unit* who, bool isReachable)
{
	if (!isReachable)
	{
		sLog.DEBUG("!isReachable() %s (%u, %u) -> %s (%u, %u)", GetName(), GetEntry(), GetGUIDLow(), who->GetName(), who->GetEntry(), who->GetGUIDLow());
		return 1;
	}

	// getThreatManager().getThreatList().size() == 1 && 
	if (!((Unit*)this)->inRangedAttackMode() && !isFlying() && GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
	{
		sLog.DEBUG("!CREATURE_Z_ATTACK_RANGE %s (%u, %u) -> %s (%u, %u)", GetName(), GetEntry(), GetGUIDLow(), who->GetName(), who->GetEntry(), who->GetGUIDLow());
		return 2;
	}

	//if (!canAttack(who, false))
	//{
	//	sLog.DEBUG("!canAttack()");
	//	return 3;
	//}

	if (!who->isInAccessiblePlacefor(this)) // CPU-HUNGRY
	{
		sLog.DEBUG("!isInAccessiblePlacefor() %s (%u, %u) -> %s (%u, %u)", GetName(), GetEntry(), GetGUIDLow(), who->GetName(), who->GetEntry(), who->GetGUIDLow());
		return 3;
	}

	if (!IsWithinLOSInMap(who))
	{
		sLog.DEBUG("!IsWithinLOSInMap() %s (%u, %u) -> %s (%u, %u)", GetName(), GetEntry(), GetGUIDLow(), who->GetName(), who->GetEntry(), who->GetGUIDLow());
		return 4;
	}

	return 0;
}

float Creature::GetAttackDistance(Unit const* pl) const
{
    float aggroRate = sWorld.getConfig(isGuard() ? RATE_CREATURE_GUARD_AGGRO : RATE_CREATURE_AGGRO);

    if (aggroRate==0)
        return 0.0f;

    int32 playerlevel   = pl->getLevelForTarget(this);
    int32 creaturelevel = getLevelForTarget(pl);

    int32 leveldif       = playerlevel - creaturelevel;

    // "The maximum Aggro Radius has a cap of 25 levels under. Example: A level 30 char has the same Aggro Radius of a level 5 char on a level 60 mob."
    if (leveldif < - 25)
        leveldif = -25;

    // "The aggro radius of a mob having the same level as the player is roughly 20 yards"
    float RetDistance = m_aggroRange ? m_aggroRange : 20.0;

    // "Aggro Radius varies with level difference at a rate of roughly 1 yard/level"
    // radius grow if playlevel < creaturelevel
    RetDistance -= (float)leveldif;

    int32 CreatureMod = 0;
    int32 PlayerMod = 0;
    //get detect range aura modifiers on creatures
    AuraList const& mTotalAuraList = GetAurasByType(SPELL_AURA_MOD_DETECT_RANGE);
    for (AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if(creaturelevel <= (*i)->GetSpellProto()->MaxTargetLevel)
            CreatureMod += (*i)->GetModifierValue();
    }
    //get detect range aura modifiers on players
    AuraList const& mTotalPlayerAuraList = pl->GetAurasByType(SPELL_AURA_MOD_DETECT_RANGE);
    for (AuraList::const_iterator i = mTotalPlayerAuraList.begin();i != mTotalPlayerAuraList.end(); ++i)
    {
        if(playerlevel <= (*i)->GetSpellProto()->MaxTargetLevel)
            PlayerMod += (*i)->GetModifierValue();
    }

    RetDistance += (CreatureMod + PlayerMod);

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if (RetDistance < 5)
        RetDistance = 5;

	if (pl->ToCreature() && pl->isPossessedByPlayer()) // player pets do not aggro from so afar
		RetDistance = RetDistance * 0.65f;

    return RetDistance*aggroRate;
}

void Creature::setDeathState(DeathState s)
{
    if ((s == JUST_DIED && !m_isDeadByDefault)||(s == JUST_ALIVED && m_isDeadByDefault))
    {
		m_deathTimer = time(NULL) + m_corpseDelay;

        // if no loot in DB, remove corpse quickly (20 sec ?)
        CreatureInfo const* cInfo = GetCreatureInfo();
        if (cInfo && !cInfo->lootid && !(cInfo->mingold || cInfo->maxgold) && !isWorldBoss())
			m_deathTimer = time(NULL) + 20;

        /* Respawn speed should not depend on if we looted the creature.
         http://et.worldofwarcraft.wikia.com/wiki/Spawn "Looting or not looting creatures and npcs does not matter, it all depends on the different spawn timers."
         "while actions like skinning cause corpses to disappear, there is no evidence that this makes the mob respawn faster."*/
        /*Creature will never respawn faster than m_deathTimer, but will be able to respawn right after corpse removal (this is ensured by creature states CORPSE and DEAD)*/
        uint32 respTime = m_respawnDelay;
        DynamicRespawnMod(respTime);
        m_respawnTime = time(NULL) + std::max(respTime, m_corpseDelay);

        // always save boss respawn time at death to prevent crash cheating
        if (sWorld.getConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY) || isWorldBoss())
            SaveRespawnTime();

        SetNoSearchAssistance(false);

        //Dismiss group if is leader
        if (m_formation && m_formation->getLeader() == this)
            m_formation->FormationReset();

        if (m_zoneScript)
            m_zoneScript->OnCreatureDeath(this);
    }

    Unit::setDeathState(s);

    if (s == JUST_DIED)
    {
        SetSelection(0);                       // remove target selection in any cases (can be set at aura remove in Unit::setDeathState)
        SetUInt32Value(UNIT_NPC_FLAGS, 0);

        setActive(false, ACTIVE_BY_ALL);

        if (!isPet() && GetCreatureInfo()->SkinLootId)
            if (LootTemplates_Skinning.HaveLootfor (GetCreatureInfo()->SkinLootId))
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        if (CanFly())
            i_motionMaster.MoveFall();

        if (!loot.empty())
        {
            // Loot is filled in Unit:Kill, here we just do some additional work
			loot.looterTimer = time(NULL) / 4 + m_deathTimer * 3 / 4;
            loot.looterCheckTimer = time(NULL) + 1;
            if (GetLootRecipient() && GetLootRecipient()->GetGroup())
                GetLootRecipient()->GetGroup()->SendRoundRobin(&loot, this);
        }

        Unit::setDeathState(CORPSE);
    }

    if (s == JUST_ALIVED)
    {
        SetHealth(GetMaxHealth(), true);
        SetLootRecipient(NULL);
        ResetPlayerDamageReq();

        Unit::setDeathState(ALIVE);

        CreatureInfo const *cinfo = GetCreatureInfo();
        RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

        SetWalk(true);

        SetUInt32Value(UNIT_NPC_FLAGS, cinfo->npcflag);
        SetUInt32Value(UNIT_FIELD_FLAGS, cinfo->unit_flags);

        ClearUnitState(UNIT_STAT_ALL_STATE);
        GetMotionMaster()->Initialize();

        SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));
        LoadCreaturesAddon(true);

        //reset map changes
        GetMap()->CreatureRespawnRelocation(this);
    }
}

void Creature::Respawn()
{
    RemoveCorpse();

    // forced recreate creature object at clients
    UnitVisibility currentVis = GetVisibility();
    SetVisibility(VISIBILITY_RESPAWN);

    if (getDeathState()==DEAD)
    {
        if (m_DBTableGuid)
            sObjectMgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanciableInstanceId(), GetMapId(), GetMap()->IsRaid(), 0);

        debug_log("Respawning...");
        m_respawnTime = 0;
        lootForPickPocketed = false;
        lootForBody         = false;

        if (m_originalEntry != GetEntry())
        {
            // need preserver gameevent state
            GameEventCreatureData const* eventData = sGameEventMgr.GetCreatureUpdateDataForActiveEvent(GetDBTableGUIDLow());
            UpdateEntry(m_originalEntry, TEAM_NONE, NULL, eventData);
        }

        CreatureInfo const *cinfo = GetCreatureInfo();
        SelectLevel(cinfo);

        if (m_isDeadByDefault)
        {
            setDeathState(JUST_DIED);
            SetHealth(0);
            GetUnitStateMgr().InitDefaults(true);
            ClearUnitState(UNIT_STAT_ALL_STATE);
            LoadCreaturesAddon(true);
        }
        else
            setDeathState(JUST_ALIVED);

        //Call AI respawn virtual function
        AI()->Reset();
        AI()->JustRespawned();
        UpdateSpeed(MOVE_RUN, false);
    }

    SetVisibility(currentVis); // restore visibility state AFTER RESURRECTION -> so respawned creatures will be seen by players!
    SendHeartBeat();
    SendMonsterStop();
}

void Creature::DisappearAndDie()
{
    DestroyForNearbyPlayers();
    if (isAlive())
        setDeathState(JUST_DIED);
    RemoveCorpse();
    SetHealth(0);                                           // just for nice GM-mode view
}

void Creature::ForcedDespawn(uint32 timeMSToDespawn)
{
    if (timeMSToDespawn)
    {
        ForcedDespawnDelayEvent *pEvent = new ForcedDespawnDelayEvent(*this);

        m_Events.AddEvent(pEvent, m_Events.CalculateTime(timeMSToDespawn));
        return;
    }

    if (isPet()) // Force Despawn for pet's
    {
        switch (((Pet*)this)->getPetType())
        {
            case GUARDIAN_PET:
            case POSSESSED_PET:
                ((Pet*)this)->Remove(PET_SAVE_AS_DELETED);
                break;
            default:
                DisappearAndDie();
                break;
        }
        return;
    }

    DisappearAndDie(); // ForcedDespawn didn't have DestroyForNearbyPlayers(), but with it it is essentially DisappearAndDie(), so call it
    /*if (isAlive())
        setDeathState(JUST_DIED);

    RemoveCorpse();
    SetHealth(0);                                           // just for nice GM-mode view*/
}

bool Creature::IsImmunedToSpell(SpellEntry const* spellInfo, bool includeCharges)
{
    if (!spellInfo)
        return false;

    if (GetCreatureInfo()->MechanicImmuneMask & (1 << (spellInfo->Mechanic - 1)))
        if(!(isPet() && ((Pet*)this)->getPetType() == HUNTER_PET))
            return true;

    return Unit::IsImmunedToSpell(spellInfo, includeCharges);
}

bool Creature::IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const
{
    if (GetCreatureInfo()->MechanicImmuneMask & (1 << (mechanic-1)))
        if(!(isPet() && ((Pet*)this)->getPetType() == HUNTER_PET))
            return true;

    return Unit::IsImmunedToSpellEffect(effect, mechanic);
}

void Creature::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if (idSchoolMask & (1 << i) && m_schoolCooldowns[i].GetExpiry() < unTimeMs)
            m_schoolCooldowns[i].Reset(unTimeMs);
    }
}

bool Creature::isSchoolProhibited(SpellSchoolMask idSchoolMask) const
{
    if (idSchoolMask == SPELL_SCHOOL_MASK_NONE)
        return false;
    for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; i++)
    {
        if (idSchoolMask & (1 << i) && !m_schoolCooldowns[i].Passed())
            return true;
    }
    return false;
}

SpellEntry const *Creature::reachWithSpellAttack(Unit *pVictim)
{
    if (!pVictim)
        return NULL;

    for (uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
    {
        if (!m_spells[i])
            continue;
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(m_spells[i]);
        if (!spellInfo)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j=0;j<3;j++)
        {
            if ((spellInfo->Effect[j] == SPELL_EFFECT_SCHOOL_DAMAGE)       ||
                (spellInfo->Effect[j] == SPELL_EFFECT_INSTAKILL)            ||
                (spellInfo->Effect[j] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE) ||
                (spellInfo->Effect[j] == SPELL_EFFECT_HEALTH_LEECH)
               )
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue) continue;

        if (spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = SpellMgr::GetSpellMaxRange(srange);
        float minrange = SpellMgr::GetSpellMinRange(srange);

        float dist = GetCombatDistance(pVictim);

        //if(!isInFront(pVictim, range) && spellInfo->AttributesEx)
        //    continue;
        if (dist > range || dist < minrange)
            continue;
        if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

SpellEntry const *Creature::reachWithSpellCure(Unit *pVictim)
{
    if (!pVictim)
        return NULL;

    for (uint32 i=0; i < CREATURE_MAX_SPELLS; i++)
    {
        if (!m_spells[i])
            continue;
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(m_spells[i]);
        if (!spellInfo)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: unknown spell id %i\n", m_spells[i]);
            continue;
        }

        bool bcontinue = true;
        for (uint32 j=0;j<3;j++)
        {
            if ((spellInfo->Effect[j] == SPELL_EFFECT_HEAL))
            {
                bcontinue = false;
                break;
            }
        }
        if (bcontinue) continue;

        if (spellInfo->manaCost > GetPower(POWER_MANA))
            continue;
        SpellRangeEntry const* srange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);
        float range = SpellMgr::GetSpellMaxRange(srange);
        float minrange = SpellMgr::GetSpellMinRange(srange);

        float dist = GetCombatDistance(pVictim);

        //if(!isInFront(pVictim, range) && spellInfo->AttributesEx)
        //    continue;
        if (dist > range || dist < minrange)
            continue;
        if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED))
            continue;
        return spellInfo;
    }
    return NULL;
}

bool Creature::IsVisibleInGridForPlayer(Player const* pl) const
{
    // gamemaster in GM mode see all, including ghosts
    if (pl->isGameMaster())
        return true;

    // Live player (or with not release body see live creatures or death creatures with corpse disappearing time > 0
    if (pl->isAlive() || pl->GetDeathTimer() > 0)
    {
        if (GetEntry() == VISUAL_WAYPOINT ||
            // We should be able to see that creature only if completed quest 10252 or 10045
            ((GetEntry() == 19698 || GetEntry() == 19879) && pl->GetQuestStatus(10252) != QUEST_STATUS_COMPLETE))
            return false;

        if (GetEntry() == 7750  && pl->GetQuestStatus(2681) != QUEST_STATUS_COMPLETE)
            return false;

		return isAlive() || (m_deathTimer > time(NULL)) || m_isDeadByDefault && m_deathState == CORPSE;
    }

    // Dead player see creatures near own corpse
    Corpse *corpse = pl->GetCorpse();
    if (corpse)
    {
        // 20 - aggro distance for same level, 25 - max additional distance if player level less that creature level
        if (corpse->IsWithinDistInMap(this, (20 + 25) * sWorld.getConfig(RATE_CREATURE_AGGRO)))
            return true;
    }

    // Dead player see Spirit Healer or Spirit Guide
    if (isSpiritService())
        return true;

    // Dead player can see ghost creatures
    if (GetCreatureInfo()->type_flags & CREATURE_TYPEFLAGS_GHOST)
        return true;

    // and not see any other
    return false;
}

void Creature::DoFleeToGetAssistance()
{
    if (!GetVictim())
        return;

    if (HasAuraType(SPELL_AURA_PREVENTS_FLEEING) || HasUnitState(UNIT_STAT_NOT_MOVE))
        return;

    float radius = sWorld.getConfig(CONFIG_CREATURE_FAMILY_FLEE_RADIUS);
    if (radius > 0)
    {
        Creature* pCreature = NULL;

        Hellground::NearestAssistCreatureInCreatureRangeCheck u_check(this, GetVictim(), radius);
        Hellground::ObjectLastSearcher<Creature, Hellground::NearestAssistCreatureInCreatureRangeCheck> searcher(pCreature, u_check);

        Cell::VisitGridObjects(this, searcher, radius);

        SetNoSearchAssistance(true);
        UpdateSpeed(MOVE_RUN, false);

        if (!pCreature)
            SetFeared(true, GetVictim(), sWorld.getConfig(CONFIG_CREATURE_FAMILY_FLEE_DELAY));
        else
            GetMotionMaster()->MoveSeekAssistance(pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ(), radius*1.5f);
    }
}

Unit* Creature::SelectNearestTarget(float dist) const
{
    Unit *target = NULL;
    {
        Hellground::NearestHostileUnitInAttackDistanceCheck u_check(this, dist);
        Hellground::UnitLastSearcher<Hellground::NearestHostileUnitInAttackDistanceCheck> searcher(target, u_check);

        Cell::VisitAllObjects(this, searcher, dist);
    }

    return target;
}

void Creature::CallAssistance()
{
    if (!GetVictim() || isPet() || isCharmed())
        return;

    if (CreatureGroup *formation = GetFormation())
    {
        formation->MemberAttackStart(this, GetVictim());
    }
    else
    {
        if (m_AlreadyCallAssistance)
            return;

        SetNoCallAssistance(true);

        float radius = sWorld.getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS);
        if (radius > 0)
        {
            std::list<Creature*> assistList;
            {
                Hellground::AnyAssistCreatureInRangeCheck u_check(this, GetVictim(), radius);
                Hellground::ObjectListSearcher<Creature, Hellground::AnyAssistCreatureInRangeCheck> searcher(assistList, u_check);

                Cell::VisitGridObjects(this, searcher, radius);
            }

            if (!assistList.empty())
            {
                AssistDelayEvent *e = new AssistDelayEvent(getVictimGUID(), *this);
                while (!assistList.empty())
                {
                    // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
                    e->AddAssistant((*assistList.begin())->GetGUID());
                    assistList.pop_front();
                }
                m_Events.AddEvent(e, m_Events.CalculateTime(sWorld.getConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY)));
            }
        }
    }
}

void Creature::CallForHelp(float fRadius)
{
    if (fRadius <= 0.0f || !GetVictim() || isPet() || isCharmed())
        return;

    Hellground::CallOfHelpCreatureInRangeDo u_do(this, GetVictim(), fRadius);
    Hellground::ObjectWorker<Creature, Hellground::CallOfHelpCreatureInRangeDo> worker(u_do);

    Cell::VisitGridObjects(this, worker, fRadius);
}

bool Creature::CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction /*= true*/) const
{
    // is it true?
    if (!HasReactState(REACT_AGGRESSIVE))
        return false;

    // we don't need help from zombies :)
    if (!isAlive())
        return false;

    // skip fighting creature
    if (IsInCombat())
        return false;

    // only from same creature faction
    if (checkfaction)
    {
        if (getFaction() != u->getFaction())
            return false;
    }
    else
    {
        if (!IsFriendlyTo(u))
            return false;
    }

    // only free creature
    if (GetCharmerOrOwnerGUID())
        return false;

    // skip non hostile to caster enemy creatures
    if (!IsHostileTo(enemy))
        return false;

    return true;
}

class DynamicRespawnPlayerCounter
{
public:
    DynamicRespawnPlayerCounter(const Creature* const crea, uint32 lvlDiff) : count(0), me(crea), _lvlDiff(lvlDiff) {}
    void operator()(Player* u)
    {
        // not checking by isWithinDist, cause we don't need very precise result. Without it the range may differ by one cell range (which is 33.333 yds)
        if (uint32(abs(int32(u->GetLevel() - me->GetLevel()))) <= _lvlDiff)
            ++count;
    }
    uint32 count;
    const Creature* const me;
    uint32 _lvlDiff;
};

void Creature::DynamicRespawnMod(uint32 &baseDelay) const
{
    float checkRange = sWorld.getConfig(CONFIG_DYN_RESPAWN_CHECK_RANGE);
    if (checkRange <= 0)
        return;

    if (!IsInWorld())
        return;

    // Only affects continents
    if (GetMapId() > 1 && GetMapId() != 530)
        return;

    if (GetLevel() >= sWorld.getConfig(CONFIG_DYN_RESPAWN_MAX_AFFECTED_LEVEL))
        return;

    if (baseDelay > sWorld.getConfig(CONFIG_DYN_RESPAWN_MAX_AFFECTED_RESPAWN))
        return;

    if (baseDelay < sWorld.getConfig(CONFIG_DYN_RESPAWN_MIN_RESULTING_RESPAWN))
        return;

    if (GetCreatureInfo()->rank)
        return;

    uint32 originalDelay = baseDelay;

    DynamicRespawnPlayerCounter checker(this, sWorld.getConfig(CONFIG_DYN_RESPAWN_PLAYERS_LEVELDIFF));
    Hellground::ObjectWorker<Player, DynamicRespawnPlayerCounter> searcher(checker);
    Cell::VisitWorldObjects(this, searcher, checkRange);

    if (checker.count <= sWorld.getConfig(CONFIG_DYN_RESPAWN_PLAYERS_THRESHOLD))
        return;
    checker.count -= sWorld.getConfig(CONFIG_DYN_RESPAWN_PLAYERS_THRESHOLD);

    uint32 reduction = checker.count * sWorld.getConfig(CONFIG_DYN_RESPAWN_PERCENT_PER_10_PLAYERS) * baseDelay / 1000;
    if (reduction > baseDelay)
        baseDelay = 0;
    else
        baseDelay -= reduction;

    if (baseDelay < sWorld.getConfig(CONFIG_DYN_RESPAWN_MIN_RESULTING_RESPAWN))
        baseDelay = sWorld.getConfig(CONFIG_DYN_RESPAWN_MIN_RESULTING_RESPAWN);
    if (baseDelay < originalDelay * sWorld.getConfig(CONFIG_DYN_RESPAWN_MIN_REDUCED_PCT) / 100)
        baseDelay = originalDelay * sWorld.getConfig(CONFIG_DYN_RESPAWN_MIN_REDUCED_PCT) / 100;
}

void Creature::SaveRespawnTime()
{
    if (isPet() || !m_DBTableGuid || m_creatureData && !m_creatureData->dbData)
        return;

    if (m_respawnTime > time(NULL))                          // dead (no corpse)
        sObjectMgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanciableInstanceId(), GetMapId(), GetMap()->IsRaid(), m_respawnTime);
	else if (m_deathTimer > time(NULL))                               // dead (corpse)
		sObjectMgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanciableInstanceId(), GetMapId(), GetMap()->IsRaid(), m_respawnDelay + m_deathTimer);
}

bool Creature::IsOutOfThreatArea(Unit* pVictim) const
{
    if (!pVictim)
        return true;

    if (!pVictim->IsInMap(this))
        return true;

    if (!canAttack(pVictim))
        return true;

    if (!pVictim->isInAccessiblePlacefor(this)) // Called in update, CPU-HUNGRY
        return true;

    if (GetMap()->IsDungeon())
        return false;

    uint32 AttackDist = GetAttackDistance(pVictim);

    uint32 distToHome = std::max(AttackDist, sWorld.getConfig(CONFIG_EVADE_HOMEDIST));
    uint32 distToTarget = std::max(AttackDist, sWorld.getConfig(CONFIG_EVADE_TARGETDIST));

    /*// If you're staying away and you pull aggro from your pet -> don't let creature go evade.
    if (pVictim->GetPet() && !IsOutOfThreatArea(pVictim->GetPet()))
        return false;

    // If you're staying away and you pull aggro from your charmed -> don't let creature go evade.
    if (pVictim->GetCharm() && !IsOutOfThreatArea(pVictim->GetCharm()))
        return false;*/ // Trentone -> I don't really like it. I will leave it here for a moment if someone will bug report this.

    if (!IsWithinDistInMap(pVictim, distToTarget))
        return true;

    if (IsDistancToHomeEvadable /* USE ONLY for timing mobs or event purposes*/ && (isWorldBoss() /*can't kite bosses*/ || (time(NULL) - GetLastDamagedTime() >= 10))) 
    {
        if (!IsWithinDistInMap(&homeLocation, distToHome))
            return true;

        if (!pVictim->IsWithinDistInMap(&homeLocation, distToHome))
            return true;
    }

    return false;
}

#define HEROIC_GUID_ADDON 20000000

CreatureDataAddon const* Creature::GetCreatureAddon() const
{
    if (m_DBTableGuid)
    {
        if (ModIsInHeroicRaid) // try to find heroic creature_addon
            if (CreatureDataAddon const* addon = ObjectMgr::GetCreatureAddon(m_DBTableGuid + HEROIC_GUID_ADDON))
                return addon;

        if (CreatureDataAddon const* addon = ObjectMgr::GetCreatureAddon(m_DBTableGuid))
            return addon;
    }

    // dependent from heroic mode entry
    return ObjectMgr::GetCreatureTemplateAddon(GetCreatureInfo()->Entry);
}

//creature_addon table
bool Creature::LoadCreaturesAddon(bool reload)
{
    CreatureDataAddon const *cainfo = GetCreatureAddon();
    if (!cainfo)
        return false;

    if (cainfo->mount != 0)
        Mount(cainfo->mount);

    if (cainfo->bytes0 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_0, cainfo->bytes0);

    if (cainfo->bytes1 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_1, cainfo->bytes1);

    if (cainfo->bytes2 != 0)
        SetUInt32Value(UNIT_FIELD_BYTES_2, cainfo->bytes2);

	if (cainfo->emote != 0)
	{
		SetUInt32Value(UNIT_NPC_EMOTESTATE, cainfo->emote);
		db_emote = cainfo->emote;
	}

    if (cainfo->move_flags != 0)
        SetUnitMovementFlags(cainfo->move_flags);

    //Load Path
    if (cainfo->path_id != 0)
        m_path_id = cainfo->path_id;

    if (cainfo->auras)
    {
        for (CreatureDataAddonAura const* cAura = cainfo->auras; cAura->spell_id; ++cAura)
        {
            SpellEntry const *AdditionalSpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(cAura->spell_id);
            if (!AdditionalSpellEntry)
            {
                sLog.outLog(LOG_DB_ERR, "Creature (GUIDLow: %u Entry: %u) has wrong spell %u defined in `auras` field.",GetGUIDLow(),GetEntry(),cAura->spell_id);
                continue;
            }

            // skip already applied aura
            if (HasAura(cAura->spell_id,cAura->effect_idx))
            {
                if (!reload)
                    sLog.outDebug("Creature (GUIDLow: %u Entry: %u) has duplicate aura (spell %u effect %u) in `auras` field.",GetGUIDLow(),GetEntry(),cAura->spell_id,cAura->effect_idx);

                continue;
            }

            Aura* AdditionalAura = CreateAura(AdditionalSpellEntry, cAura->effect_idx, NULL, this, this, 0);
            AddAura(AdditionalAura);
            sLog.outDebug("Spell: %u with Aura %u added to creature (GUIDLow: %u Entry: %u)", cAura->spell_id, AdditionalSpellEntry->EffectApplyAuraName[0],GetGUIDLow(),GetEntry());
        }
    }
    return true;
}

/// Send a message to LocalDefense channel for players opposition team in the zone
void Creature::SendZoneUnderAttackMessage(Player* attacker)
{
    uint32 enemy_team = attacker->GetTeam();

    WorldPacket data(SMSG_ZONE_UNDER_ATTACK,4);
    data << (uint32)GetZoneId();
    sWorld.SendGlobalMessage(&data,NULL,(enemy_team==ALLIANCE ? HORDE : ALLIANCE));
}

void Creature::SetInCombatWithZone()
{
    if (!CanHaveThreatList())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Creature entry %u call SetInCombatWithZone but creature cannot have threat list.", GetEntry());
        return;
    }

    Map* pMap = GetMap();

    if (!pMap->IsDungeon())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Creature entry %u call SetInCombatWithZone for map (id: %u) that isn't an instance.", GetEntry(), pMap->GetId());
        return;
    }

    Map::PlayerList const &PlList = pMap->GetPlayers();

    if (PlList.isEmpty())
        return;

    for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
    {
        if (Player* pPlayer = i->getSource())
        {
            if (pPlayer->isGameMaster())
                continue;

            if (pPlayer->isAlive())
            {
                pPlayer->SetInCombatWith(this);
                AddThreat(pPlayer, 0.0f);
            }
        }
    }
}

void Creature::_AddCreatureSpellCooldown(uint32 spell_id, time_t end_time)
{
    m_CreatureSpellCooldowns[spell_id] = end_time;
}

void Creature::_AddCreatureCategoryCooldown(uint32 category, time_t end_time)
{
    m_CreatureCategoryCooldowns[category] = end_time;
}

void Creature::AddCreatureSpellCooldown(uint32 spellid)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
    if (!spellInfo)
        return;

    uint32 cooldown = SpellMgr::GetSpellRecoveryTime(spellInfo);
    uint32 CategoryCooldown = spellInfo->CategoryRecoveryTime;

    if (isPet())
    {
        if (Unit* Owner = GetOwner())
        {
            if (Owner->GetTypeId() == TYPEID_PLAYER)
            {
                Owner->ToPlayer()->ApplySpellMod(spellid, SPELLMOD_COOLDOWN, CategoryCooldown);
                if (CategoryCooldown < 0)
                    CategoryCooldown = 0;
                // what the fuck was that for? Disabling this fixes spells like Shell Shield
                cooldown = CategoryCooldown;
            }
        }
    }

    if (cooldown)
        _AddCreatureSpellCooldown(spellid, time(NULL) + cooldown/1000);

    if (spellInfo->Category && CategoryCooldown)
        _AddCreatureCategoryCooldown(spellInfo->Category, time(NULL) + CategoryCooldown/1000);
}

bool Creature::HasCategoryCooldown(uint32 spell_id) const
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
    if (!spellInfo)
        return false;

    CreatureSpellCooldowns::const_iterator itr = m_CreatureCategoryCooldowns.find(spellInfo->Category);
    return(itr != m_CreatureCategoryCooldowns.end() && itr->second > time(NULL));
}

bool Creature::HasSpellCooldown(uint32 spell_id) const
{
    CreatureSpellCooldowns::const_iterator itr = m_CreatureSpellCooldowns.find(spell_id);
    return (itr != m_CreatureSpellCooldowns.end() && itr->second > time(NULL)) || HasCategoryCooldown(spell_id);
}

bool Creature::IsInEvadeMode() const
{
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_RESTART_PACIFIED))
        return true;
    
    return /*!i_motionMaster.empty() &&*/ i_motionMaster.GetCurrentMovementGeneratorType() == HOME_MOTION_TYPE;
}

float Creature::GetBaseSpeed() const
{
    if (isPet())
    {
        switch (((Pet*)this)->getPetType())
        {
            case SUMMON_PET:
            case HUNTER_PET:
            {
                return 1.15;  // Blizzlike ;p
            }
            case GUARDIAN_PET:
            case MINI_PET:
            {
                break;
            }
        }
    }
    return m_creatureInfo->speed;
}

bool Creature::HasSpell(uint32 spellID) const
{
    for (uint8 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        if (spellID == m_spells[i])
            return true;

    return false;
}

time_t Creature::GetRespawnTimeEx() const
{
    time_t now = time(NULL);
    if (m_respawnTime > now)                                 // dead (no corpse)
        return m_respawnTime;
	else if (m_deathTimer > now)                               // dead (corpse)
		return m_respawnDelay + m_deathTimer;
    else
        return now;
}

void Creature::GetRespawnCoord(float &x, float &y, float &z, float* ori, float* dist) const
{
    if (m_DBTableGuid)
    {
        if (CreatureData const* data = sObjectMgr.GetCreatureData(GetDBTableGUIDLow()))
        {
            x = data->posX;
            y = data->posY;
            z = data->posZ;
            if (ori)
                *ori = data->orientation;
            if (dist)
                *dist = data->spawndist;
        }
    }
    else
    {
        x = GetPositionX();
        y = GetPositionY();
        z = GetPositionZ();
        if (ori)
            *ori = GetOrientation();
        if (dist)
            *dist = 0;
    }
    //lets check if our creatures have valid spawn coordinates
    if(!Hellground::IsValidMapCoord(x, y, z))
    {
        sLog.outLog(LOG_CRASH, "ERROR: Creature with invalid respawn coordinates: cre id %u, mapid = %u, guid = %u, x = %f, y = %f, z = %f. Cur pos x = %f, y = %f, z = %f",
            GetEntry(), GetMapId(), GetGUIDLow(), x, y, z, GetPositionX(), GetPositionY(), GetPositionZ());
        ASSERT(false);
    }
}

void Creature::AllLootRemovedFromCorpse()
{
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
    {
        uint32 nDeathTimer;

        CreatureInfo const *cinfo = GetCreatureInfo();

        // corpse was not skinnable -> apply corpse looted timer
        if (!cinfo || !cinfo->SkinLootId)
            nDeathTimer = std::max<uint32>((uint32)(m_corpseDelay * sWorld.getConfig(RATE_CORPSE_DECAY_LOOTED)), uint32(20));
        // corpse skinnable, but without skinning flag, and then skinned, corpse will despawn next update
        else
            nDeathTimer = 0;

        // update death timer only if looted timer is shorter
		if ((m_deathTimer - time(NULL)) > nDeathTimer)
			m_deathTimer = time(NULL) + nDeathTimer;
    }
}

uint32 Creature::getLevelForTarget(Unit const* target) const
{
    if (!isWorldBoss())
        return Unit::getLevelForTarget(target);

    uint32 level = target->GetLevel()+sWorld.getConfig(CONFIG_WORLD_BOSS_LEVEL_DIFF);
    if (level < 1)
        return 1;
    if (level > 255)
        return 255;
    return level;
}

std::string Creature::GetAIName() const
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->AIName;
}

std::string Creature::GetScriptName()
{
    return sScriptMgr.GetScriptName(GetScriptId());
}

uint32 Creature::GetScriptId()
{
    return ObjectMgr::GetCreatureTemplate(GetEntry())->ScriptID;
}

VendorItemData const* Creature::GetVendorItems() const
{
    return sObjectMgr.GetNpcVendorItemList(GetEntry());
}

uint32 Creature::GetVendorItemCurrentCount(VendorItem const* vItem)
{
    if (!vItem->maxcount)
        return vItem->maxcount;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId==vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
        return vItem->maxcount;

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if (vCount->lastIncrementTime + vItem->incrtime <= ptime)
    {
        ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) >= vItem->maxcount)
        {
            m_vendorItemCounts.erase(itr);
            return vItem->maxcount;
        }

        vCount->count += diff * pProto->BuyCount;
        vCount->lastIncrementTime = ptime;
    }

    return vCount->count;
}

uint32 Creature::UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count)
{
    if (!vItem->maxcount)
        return 0;

    VendorItemCounts::iterator itr = m_vendorItemCounts.begin();
    for (; itr != m_vendorItemCounts.end(); ++itr)
        if (itr->itemId==vItem->item)
            break;

    if (itr == m_vendorItemCounts.end())
    {
        uint32 new_count = vItem->maxcount > used_count ? vItem->maxcount-used_count : 0;
        m_vendorItemCounts.push_back(VendorItemCount(vItem->item,new_count));
        return new_count;
    }

    VendorItemCount* vCount = &*itr;

    time_t ptime = time(NULL);

    if (vCount->lastIncrementTime + vItem->incrtime <= ptime)
    {
        ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(vItem->item);

        uint32 diff = uint32((ptime - vCount->lastIncrementTime)/vItem->incrtime);
        if ((vCount->count + diff * pProto->BuyCount) < vItem->maxcount)
            vCount->count += diff * pProto->BuyCount;
        else
            vCount->count = vItem->maxcount;
    }

    vCount->count = vCount->count > used_count ? vCount->count-used_count : 0;
    vCount->lastIncrementTime = ptime;
    return vCount->count;
}

TrainerSpellData const* Creature::GetTrainerSpells() const
{
    return sObjectMgr.GetNpcTrainerSpells(GetEntry());
}

// overwrite WorldObject function for proper name localization
const char* Creature::GetNameForLocaleIdx(int32 loc_idx) const
{
    char const* name = GetName();
    sObjectMgr.GetCreatureLocaleStrings(GetEntry(), loc_idx, &name);
    return name;
}

const CreatureData* Creature::GetLinkedRespawnCreatureData() const
{
    if (!m_DBTableGuid) // only hard-spawned creatures from DB can have a linked master
        return NULL;

    if (uint32 targetGuid = sObjectMgr.GetLinkedRespawnGuid(m_DBTableGuid))
        return sObjectMgr.GetCreatureData(targetGuid);

    return NULL;
}

// returns master's remaining respawn time if any
time_t Creature::GetLinkedCreatureRespawnTime() const
{
    if (!m_DBTableGuid) // only hard-spawned creatures from DB can have a linked master
        return 0;

    if (uint32 targetGuid = sObjectMgr.GetLinkedRespawnGuid(m_DBTableGuid))
    {
        Map* targetMap = NULL;
        if (const CreatureData* data = sObjectMgr.GetCreatureData(targetGuid))
        {
            if (data->mapid == GetMapId())   // look up on the same map
                targetMap = GetMap();
            else                            // it shouldn't be instanceable map here
                targetMap = sMapMgr.FindMap(data->mapid, INSTANCEID_NULL);
        }
        if (targetMap)
            return sObjectMgr.GetCreatureRespawnTime(targetGuid,targetMap->GetInstanciableInstanceId());
    }

    return 0;
}

void Creature::SetWalk(bool enable)
{
    if (enable)
        AddUnitMovementFlag(MOVEFLAG_WALK_MODE);
    else
        RemoveUnitMovementFlag(MOVEFLAG_WALK_MODE);

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_WALK_MODE : SMSG_SPLINE_MOVE_SET_RUN_MODE, 9);
    data << GetPackGUID();
    BroadcastPacket(&data, true);
}

void Creature::SetLevitate(bool enable)
{
    if (enable)
    {
        AddUnitMovementFlag(MOVEFLAG_LEVITATING);
        addUnitState(UNIT_STAT_IGNORE_PATHFINDING);
    }
    else
    {
        RemoveUnitMovementFlag(MOVEFLAG_LEVITATING);
        ClearUnitState(UNIT_STAT_IGNORE_PATHFINDING);
    }

    WorldPacket data(enable ? SMSG_SPLINE_MOVE_SET_FLYING : SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
    data << GetPackGUID();
    BroadcastPacket(&data, true);
}

bool Creature::CanReactToPlayerOnTaxi()
{
    // hacky exception for Sunblade Lookout, Shattered Sun Bombardier and Brutallus
    switch (GetEntry())
    {
        case 25132:
        case 25144:
        case 25158:
        case 17973:
            return true;
        default:
            return false;
    }
}

bool Creature::CanFly() const
{
    return GetCreatureInfo()->InhabitType & INHABIT_AIR || IsLevitating();
}


bool AttackResumeEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    if (!m_owner.isAlive())
        return true;

    if (m_owner.HasUnitState(UNIT_STAT_CAN_NOT_REACT) || m_owner.HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
        return true;

    Unit* victim = m_owner.GetVictim();
    if (!victim || !victim->IsInMap(&m_owner))
        return true;

    switch(m_owner.GetObjectGuid().GetHigh())
    {
        case HIGHGUID_UNIT:
        {
            m_owner.AttackStop(/*!b_force*/);
            CreatureAI* ai = m_owner.ToCreature()->AI();
            if (ai)
                ai->AttackStart(victim);
            break;
        }
        case HIGHGUID_PET:
        {
            m_owner.AttackStop(/*!b_force*/);
            m_owner.ToPet()->AI()->AttackStart(victim);
            break;
        }
        case HIGHGUID_PLAYER:
            if (m_owner.IsAIEnabled)
                m_owner.ToPlayer()->AI()->AttackStart(victim);
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: AttackResumeEvent::Execute try execute for unsupported owner %s!", m_owner.GetObjectGuid().GetString().c_str());
            break;
    }
    return true;
}

void Creature::ApplyGameEventSpells(GameEventCreatureData const* eventData, bool activated)
{
    uint32 cast_spell = activated ? eventData->spell_id_start : eventData->spell_id_end;
    uint32 remove_spell = activated ? eventData->spell_id_end : eventData->spell_id_start;

    if (remove_spell)
        if (SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(remove_spell))
            RemoveAurasDueToSpell(remove_spell);

    if (cast_spell)
        CastSpell(this, cast_spell, true);
}

//float Creature::GetHeroicRaid70DependentMod(uint32 mapId)
//{
//    switch (mapId)
//    {
//        /*case 532: //| 532 | instance_karazhan          |
//        case 534: //| 534 | instance_hyjal             |
//        case 544: //| 544 | instance_magtheridons_lair |
//        case 550: //| 550 | instance_the_eye           |
//        case 565: //| 565 | instance_gruuls_lair       |
//        case 568: //| 568 | instance_zulaman           |*/ // NYI
//
//        case 548: //| 548 | instance_serpent_shrine    |
//			return 1.3f;
//        case 564: //| 564 | instance_black_temple      |
//            return 0.7f;
//        case 580: //| 580 | instance_sunwell_plateau   |
//            return 0.6f;
//    }
//    return 0;
//}
//
//float Creature::GetHeroicRaid60DependentMod(uint32 mapId)
//{
//    //sWorld.noModCreatures[GetEntry()] = GetCreatureInfo()->Name;
//    switch (mapId)
//    {
//        case 533: // Naxxramas // 40 man HARDMODE raid
//            return 0.45f;
//        case 531: // Temple of AQ // 40 man raid
//        case 409: // Molten Core // 40 man raid
//        case 469: // BWL // 40 man raid
//            return 0.75f;
//        case 509: // Ruins of AQ // 20 man raid - 1.5
//        case 309: // Zul'Gurub // 20 man raid - 1.5
//            return 1.5f; // 20 man raid. 1.5 from 60 level is JUST ENOUGH to make it t4-t5 level instance
//        case 249: // Onyxia // 40 man raid - 1.5 is enough i think for her
//            return 1.35f;
//    }
//    return 0;
//}

//float Creature::GetMapDependentMod(uint32 mapId, bool IsHeroic)
//{
//    //float mod = 1.0f;
//    if (IsHeroic)
//    {
//        float heroic60 = GetHeroicRaid60DependentMod(mapId); // this means this map is raid 60
//        if (heroic60 > 0)
//            return heroic60;
//        float heroic70 = GetHeroicRaid70DependentMod(mapId);
//        if (heroic70 > 0)
//            return heroic70;
//    }
//
//    return 0;
//}

float Creature::GetCreatureHealthMod()
{
	//if (!ModIsInHeroicRaid && sWorld.RaidForceDefaultModifiers(GetMapId()))
	//	return GetConfigHealthMod();
	
    ModDamageHP mod;

    // `creature_map_mod` table modifiers
    mod = Map::GetMapCreatureMod(GetMapId(), ModIsInHeroicRaid);

    if (mod.hp == 0)
        mod.hp = GetConfigHealthMod();

    if (GetMap()->IsRaid())
    {
        if (sWorld.isEasyRealm())
        {
            // make trash easier
            if (GetCreatureInfo()->rank != CREATURE_ELITE_WORLDBOSS && !GetMap()->IsHeroic())
                mod.hp /= 2;
        }
        else
        {
            // @lowrate_tweaks 25 raid instances are easier on lowrate realm
            uint32 maxPlayers = GetMap()->GetMaxPlayers(GetMap()->GetId());
            if (maxPlayers == 25)
                mod.hp /= 2;
        }
    }
       
    // we need to make horde stronger in Alterac Valley
    // last update 17.05.24 - 69/21 A/H
    if (GetMap()->GetId() == MAP_ALTERAC)
    {
        if (GetCreatureInfo()->faction_H == 1214)
			mod.hp *= 1.25f;
    }

    // `dmg_modifiers` in `creature_template` table
    float hpMod = GetHPMod();
    if (hpMod != 0)
        mod.hp *= hpMod;

    return mod.hp;
}

float Creature::GetCreatureDamageMod()
{
	//if (!ModIsInHeroicRaid && sWorld.RaidForceDefaultModifiers(GetMapId()))
	//	return GetConfigDamageMod();
	
    ModDamageHP mod;

    // `creature_map_mod` table modifiers
    mod = Map::GetMapCreatureMod(GetMapId(), ModIsInHeroicRaid);

    if (mod.damage == 0)
        mod.damage = GetConfigDamageMod();

    // @lowrate_tweaks 25 raid instances are easier on lowrate realm
    if (!sWorld.isEasyRealm() && GetMap()->IsRaid() && GetMap()->GetMaxPlayers(GetMap()->GetId()) == 25)
       mod.damage /= 2;

    // we need to make horde stronger in Alterac Valley
    // last update 17.05.24 - 69/21 A/H
    if (GetMap()->GetId() == MAP_ALTERAC)
    {
        if (GetCreatureInfo()->faction_H == 1214)
            mod.damage *= 1.25f;
    }

    // `dmg_modifiers` in `creature_template` table
    float dmgMod = GetDMGMod();
    if (dmgMod != 0)
        mod.damage *= dmgMod;

    return mod.damage;
}

//float Creature::GetPercentageDamageMod()
//{   
//	if (!ModIsInHeroicRaid && sWorld.RaidForceDefaultModifiers(GetMapId()))
//		return GetConfigDamageMod();
//
//    ModDamageHP mod;
//
//    // MODED by command or DB
//    float dmgMod = GetDMGMod();
//    if (dmgMod > 0)
//    {
//        if (!sWorld.isEasyRealm())
//            return dmgMod;
//        
//        mod.damage = dmgMod;
//        mod = sWorld.ChallengeModeModDamageHP(GetMapId(), mod);
//        return mod.damage;
//    }
//
//    mod = Map::GetMapCreatureMod(GetMapId(), ModIsInHeroicRaid);
//    if (mod.damage > 0)
//        return mod.damage;
//
//    return GetConfigDamageMod();
//}

float Creature::GetConfigHealthMod()
{
    switch (GetCreatureInfo()->rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_NORMAL_HP);
        case CREATURE_ELITE_ELITE:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_ELITE_HP);
        case CREATURE_ELITE_RAREELITE:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_RAREELITE_HP);
        case CREATURE_ELITE_WORLDBOSS:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_WORLDBOSS_HP);
        case CREATURE_ELITE_RARE:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_RARE_HP);
        default:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_ELITE_HP);
    }
}

float Creature::GetConfigDamageMod()
{
    switch (GetCreatureInfo()->rank)                                           // define rates for each elite rank
    {
        case CREATURE_ELITE_NORMAL:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_NORMAL_DAMAGE);
        case CREATURE_ELITE_ELITE:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_ELITE_DAMAGE);
        case CREATURE_ELITE_RAREELITE:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_RAREELITE_DAMAGE);
        case CREATURE_ELITE_WORLDBOSS:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE);
        case CREATURE_ELITE_RARE:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_RARE_DAMAGE);
        default:
            return /*mod**/sWorld.getConfig(RATE_CREATURE_ELITE_ELITE_DAMAGE);
    }
}

void Creature::RemoveAllAurasNotCreatureAddon()
{
    CreatureDataAddon const *cainfo = GetCreatureAddon();
    if (!cainfo || !cainfo->auras) // simply remove all auras
    {
        RemoveAllAuras();
        return;
    }

    std::set<uint32> aurasAllowed;
    for (CreatureDataAddonAura const* cAura = cainfo->auras; cAura->spell_id; ++cAura)
        aurasAllowed.insert(cAura->spell_id);

    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if (aurasAllowed.find(iter->first.first) == aurasAllowed.end()) // not addon aura -> remove
            RemoveAura(iter);
        else
            ++iter;
    }
}

bool Creature::GenerateLoot(Unit* loot_recipient, bool alive)
{
	if (alive && pulled)
		return true;
		
	bool isBoss = GetCreatureInfo()->rank == CREATURE_ELITE_WORLDBOSS;

	if (alive && !isBoss)
		return false;

	if (!alive && lootForPickPocketed)
	{
		lootForPickPocketed = false;
		loot.clear();
	}

	uint32 lootid = GetCreatureInfo()->lootid;
	if (!lootid)
		return true;

	if (isBoss)
	{
		// boss pulled - set recipient and generate loot
		if (alive)
		{
			// already generated, just continue
			if (!loot.empty())
			{
				if (GetLootRecipient())
					return true;
			
				sLog.outLog(LOG_SPECIAL, "GenerateLoot - entry %u has not empty loot and null recipient", GetEntry());
				return false;
			}

			Player* player = loot_recipient->GetCharmerOrOwnerPlayerOrPlayerItself();
			if (!player)
			{
				sLog.outLog(LOG_SPECIAL, "GenerateLoot - entry %u has no player", GetEntry());
				return false;
			}

			SetLootRecipient(player);
		}
		// boss killed - give loot and return
		else
		{
			if (!pulled)
			{
				sLog.outLog(LOG_SPECIAL, "GenerateLoot - entry %u boss_loot_generated is false", GetEntry());
				return false;
			}

			if (loot.empty())
			{
				sLog.outLog(LOG_SPECIAL, "GenerateLoot - entry %u has FURTHER empty loot", GetEntry());
				return false;
			}
			
			if (!GetLootRecipient())
			{
				sLog.outLog(LOG_SPECIAL, "GenerateLoot - entry %u has null GetLootRecipient(), loot cleared", GetEntry());
				loot.clear();
				return false;
			}

			loot.saveLootToDB(GetLootRecipient());
			return true;
		}
	}

	if (!loot.LootLoadedFromDB())
	{
		loot.clear();
		if (lootid) // only new loots are dropped in heroic raids
		{
			loot.setCreatureGUID(this);
			// don't save loot in DB at this step
			loot.FillLoot(lootid, LootTemplates_Creature, GetLootRecipient(), false, GetCreatureInfo()->Entry, !alive);
		}
		loot.generateMoneyLoot(GetCreatureInfo()->mingold, GetCreatureInfo()->maxgold);
	}

	// set looterGUID for round robin loot
	if (GetLootRecipient() && GetLootRecipient()->GetGroup())
	{
		Group *group = GetLootRecipient()->GetGroup();
		group->UpdateLooterGuid(this, true);            // select next looter if one is out of xp range
		loot.looterGUID = group->GetLooterGuid();
		group->UpdateLooterGuid(this, false);           // select next looter
	}

	return true;
}