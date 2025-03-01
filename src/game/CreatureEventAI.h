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

#ifndef HELLGROUND_CREATURE_EAI_H
#define HELLGROUND_CREATURE_EAI_H

#include "Common.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Unit.h"

class Player;
class WorldObject;

#define EVENT_UPDATE_TIME               500
#define MAX_ACTIONS                     3
#define MAX_PHASE                       32

enum EventAI_Type
{
    EVENT_T_TIMER                   = 0,                    // InitialMin, InitialMax, RepeatMin, RepeatMax
    EVENT_T_TIMER_OOC               = 1,                    // InitialMin, InitialMax, RepeatMin, RepeatMax
    EVENT_T_HP                      = 2,                    // HPMax%, HPMin%, RepeatMin, RepeatMax
    EVENT_T_MANA                    = 3,                    // ManaMax%,ManaMin% RepeatMin, RepeatMax
    EVENT_T_AGGRO                   = 4,                    // NONE
    EVENT_T_KILL                    = 5,                    // RepeatMin, RepeatMax
    EVENT_T_DEATH                   = 6,                    // NONE
    EVENT_T_EVADE                   = 7,                    // NONE
    EVENT_T_SPELLHIT                = 8,                    // SpellID, School, RepeatMin, RepeatMax
    EVENT_T_RANGE                   = 9,                    // MinDist, MaxDist, RepeatMin, RepeatMax
    EVENT_T_OOC_LOS                 = 10,                   // NoHostile, MaxRnage, RepeatMin, RepeatMax
    EVENT_T_SPAWNED                 = 11,                   // Condition, CondValue1
    EVENT_T_TARGET_HP               = 12,                   // HPMax%, HPMin%, RepeatMin, RepeatMax
    EVENT_T_TARGET_CASTING          = 13,                   // RepeatMin, RepeatMax
    EVENT_T_FRIENDLY_HP             = 14,                   // HPDeficit, Radius, RepeatMin, RepeatMax
    EVENT_T_FRIENDLY_IS_CC          = 15,                   // DispelType, Radius, RepeatMin, RepeatMax
    EVENT_T_FRIENDLY_MISSING_BUFF   = 16,                   // SpellId, Radius, RepeatMin, RepeatMax
    EVENT_T_SUMMONED_UNIT           = 17,                   // CreatureId, RepeatMin, RepeatMax
    EVENT_T_TARGET_MANA             = 18,                   // ManaMax%, ManaMin%, RepeatMin, RepeatMax
    EVENT_T_QUEST_ACCEPT            = 19,                   // QuestID
    EVENT_T_QUEST_COMPLETE          = 20,                   //
    EVENT_T_REACHED_HOME            = 21,                   // NONE
    EVENT_T_RECEIVE_EMOTE           = 22,                   // EmoteId, Condition, CondValue1, CondValue2
    EVENT_T_BUFFED                  = 23,                   // Param1 = SpellID, Param2 = Number of Time STacked, Param3/4 Repeat Min/Max
    EVENT_T_TARGET_BUFFED           = 24,                   // Param1 = SpellID, Param2 = Number of Time STacked, Param3/4 Repeat Min/Max
    EVENT_T_SUMMONED_JUST_DIED      = 25,                   // creatureEntry, RepeatMin, RepeatMax
    EVENT_T_SUMMONED_JUST_DESPAWN   = 26,                   // creatureEntry, RepeatMin, RepeatMax
    EVENT_T_MISSING_AURA            = 27,                   // Param1 = SpellID, Param2 = Number of time stacked expected, Param3/4 Repeat Min/Max
    EVENT_T_TARGET_MISSING_AURA     = 28,                   // Param1 = SpellID, Param2 = Number of time stacked expected, Param3/4 Repeat Min/Max
    EVENT_T_TIMER_GENERIC           = 29,                   // InitialMin, InitialMax, RepeatMin, RepeatMax
    EVENT_T_RECEIVE_AI_EVENT        = 30,                   // AIEventType, Sender-Entry, unused, unused
    // EVENT_T_ENERGY                  = 31,                // NOT USABLE IN OUR CORE
    EVENT_T_SELECT_ATTACKING_TARGET = 32,                   // minrange, maxrange, minrepeat, maxrepeat
    // EVENT_T_FACING_TARGET           = 33,                // NOT USABLE IN OUR CORE
    EVENT_T_RESET                   = 35,                   // Is it called after combat, when the creature respawn and spawn. -- TRINITY ONLY
    EVENT_T_TARGET_HAS_AURA_TYPE    = 36,                   // param1 = auratype, param2 = 0, param3/4 = repeat min/max
    EVENT_T_REACH_POINT             = 37,                   // pointID
    EVENT_T_OOC_LOS_SPECIAL         = 38,                   // maxrange, questId, repeatMin, repeatMax

    EVENT_T_END,
};

enum EventAI_ActionType
{
    ACTION_T_NONE                       = 0,                // No action
    ACTION_T_TEXT                       = 1,                // TextId1, optionally -TextId2, optionally -TextId3(if -TextId2 exist). If more than just -TextId1 is defined, randomize. Negative values.
    ACTION_T_SET_FACTION                = 2,                // FactionId (or 0 for default)
    ACTION_T_MORPH_TO_ENTRY_OR_MODEL    = 3,                // Creature_template entry(param1) OR ModelId (param2) (or 0 for both to demorph)
    ACTION_T_SOUND                      = 4,                // SoundId
    ACTION_T_EMOTE                      = 5,                // EmoteId
    ACTION_T_RANDOM_SAY                 = 6,                // TextId1 begin, TextId2 Last. Random between this two (included)
    ACTION_T_RANDOM_YELL                = 7,                // UNUSED
    ACTION_T_RANDOM_TEXTEMOTE           = 8,                // UNUSED
    ACTION_T_RANDOM_SOUND               = 9,                // SoundId1, SoundId2, SoundId3 (-1 in any field means no output if randomed that field)
    ACTION_T_RANDOM_EMOTE               = 10,               // EmoteId1, EmoteId2, EmoteId3 (-1 in any field means no output if randomed that field)
    ACTION_T_CAST                       = 11,               // SpellId, Target, CastFlags
    ACTION_T_SUMMON                     = 12,               // CreatureID, Target, Duration in ms
    ACTION_T_THREAT_SINGLE_PCT          = 13,               // Threat%, Target
    ACTION_T_THREAT_ALL_PCT             = 14,               // Threat%
    ACTION_T_QUEST_EVENT                = 15,               // QuestID, Target
    ACTION_T_CAST_EVENT                 = 16,               // QuestID, SpellId, Target - must be removed as hack?
    ACTION_T_SET_UNIT_FIELD             = 17,               // Field_Number, Value, Target
    ACTION_T_SET_UNIT_FLAG              = 18,               // Flags (may be more than one field OR'd together), Target
    ACTION_T_REMOVE_UNIT_FLAG           = 19,               // Flags (may be more than one field OR'd together), Target
    ACTION_T_AUTO_ATTACK                = 20,               // AllowAttackState (0 = stop attack, anything else means continue attacking)
    ACTION_T_COMBAT_MOVEMENT            = 21,               // AllowCombatMovement (0 = stop combat based movement, anything else continue attacking)
    ACTION_T_SET_PHASE                  = 22,               // Phase
    ACTION_T_INC_PHASE                  = 23,               // Value (may be negative to decrement phase, should not be 0)
    ACTION_T_EVADE                      = 24,               // No Params
    ACTION_T_FLEE_FOR_ASSIST            = 25,               // No Params
    ACTION_T_QUEST_EVENT_ALL            = 26,               // QuestID
    ACTION_T_CAST_EVENT_ALL             = 27,               // CreatureId, SpellId
    ACTION_T_REMOVEAURASFROMSPELL       = 28,               // Target, Spellid
    ACTION_T_RANGED_MOVEMENT            = 29,               // Distance, Angle
    ACTION_T_RANDOM_PHASE               = 30,               // PhaseId1, PhaseId2, PhaseId3
    ACTION_T_RANDOM_PHASE_RANGE         = 31,               // PhaseMin, PhaseMax
    ACTION_T_SUMMON_ID                  = 32,               // CreatureId, Target, SpawnId
    ACTION_T_KILLED_MONSTER             = 33,               // CreatureId, Target
    ACTION_T_SET_INST_DATA              = 34,               // Field, Data
    ACTION_T_SET_INST_DATA64            = 35,               // Field, Target
    ACTION_T_UPDATE_TEMPLATE            = 36,               // Entry, Team
    ACTION_T_DIE                        = 37,               // No Params
    ACTION_T_ZONE_COMBAT_PULSE          = 38,               // No Params
    ACTION_T_CALL_FOR_HELP              = 39,               // Radius
    ACTION_T_SET_SHEATH                 = 40,               // Sheath (0-passive,1-melee,2-ranged)
    ACTION_T_FORCE_DESPAWN              = 41,               // No Params
    ACTION_T_SET_INVINCIBILITY_HP_LEVEL = 42,               // MinHpValue, format(0-flat,1-percent from max health)
    ACTION_T_MOUNT_TO_ENTRY_OR_MODEL    = 43,               // CreatureEntry (param1) or modelId (param2) or 0 for both to dismount
    // ACTION_T_CHANCED_TEXT            = 44,               // NOT USED
    ACTION_T_THROW_AI_EVENT             = 45,               // EventType, Radius, Target
    // ACTION_T_SET_THROW_MASK          = 46,               // NOT USED
    // ACTION_T_SET_STAND_STATE            = 47,               // StandState. Our action is 98
    ACTION_T_DYNAMIC_MOVEMENT           = 49,               // EnableDynamicMovement (1 = on; 0 = off)
    ACTION_T_LOAD_PATH                  = 70,               // WayPointEntry, Repeatable (if >0)
    ACTION_T_COMBAT_STOP                = 71,               // No Params // UPDATE `creature_ai_scripts` SET `action1_type`='71' WHERE `id` IN (2493803, 2506303)
    ACTION_T_CHECK_OUT_OF_THREAT        = 72,               // NOT USED
    ACTION_T_REMOVE_CORPSE              = 73,               // No Params - NUT USED
    ACTION_T_CAST_GUID                  = 74,               // SpellId, TargetGUID, CastFlags // UPDATE `creature_ai_scripts` SET `action1_type`='74' WHERE `id` IN (2499904, 2499905)

    ACTION_T_SET_PHASE_MASK             = 97,
    ACTION_T_SET_STAND_STATE            = 98,
    ACTION_T_MOVE_RANDOM_POINT          = 99,
    ACTION_T_SET_VISIBILITY             = 100,
    ACTION_T_SET_ACTIVE                 = 101,              // Apply
    ACTION_T_SET_AGGRESSIVE             = 102,              // Apply
    ACTION_T_ATTACK_START_PULSE         = 103,              // Distance
    ACTION_T_SUMMON_GO                  = 104,              // GameObjectID, DespawnTime in msz
    ACTION_T_MOVE_POINT                 = 105,              // positionId
    ACTION_T_SET_HOME_POSITION          = 106,              // positionId
    ACTION_T_SET_ORIENTATION            = 107,              // positionId
    ACTION_T_MOVE_RANDOM_AROUND         = 108,              // radius
    ACTION_T_SET_MOVE_SPEED             = 109,              // type, speed
    ACTION_T_SET_WALK                   = 110,              // true or false
    ACTION_T_SET_FLYING                 = 111,              // true or false
    ACTION_T_TEXT_WITH_TARGET           = 112,              // text, target
    ACTION_T_MOUNT                      = 113,              // 2 mount/ 1 unmount, model
    ACTION_T_HANDLE_GAMEOBJECT          = 114,              // Getting nearest gameobject with entry in range. (param1 = entry, param2 = range, param3 = 0 - close, 1 - open.)
    ACTION_T_DISABLE_MMAPS              = 115,              // 0 - enable, 1 - disable
    ACTION_T_NEAR_TELEPORT              = 116,              // positionId
    ACTION_T_SET_EVADABLE_AT_DIST       = 117,              // 0 - evadable, 1 - not evadable
    ACTION_T_AUTO_EMOTE                 = 118,              // emoteid
    ACTION_T_LOAD_EQUIPMENT             = 119,              // id
    ACTION_T_SET_HOVER_LEVITATE         = 120,              // 0 - disable both, 1 - enable hover, 2 - enable levitate, 3 - enable both
    ACTION_T_SET_DEFAULT_MOVEMENT_TYPE  = 121,              // param1= 1 - Idle; 2 - random, param2=radius in his current position; 3 - WP, param2=pathID
    ACTION_T_FORCE_REACTION             = 122,              // targetType, factionTemplate, ReputationRank
    ACTION_T_INTERRUPT_SPELLS           = 123,              // 1 - withdelayed
    ACTION_T_CAST_DEFAULT               = 124,              // spellid, target, Triggered = 1 (0 - not)
    ACTION_T_MOVE_FOLLOW                = 125,              // target, dist, angle
    ACTION_T_SET_GO_GUID_LOOT_STATE     = 126,              // guid, dist, state
    ACTION_T_APPLY_IMMUNITY             = 127,              // id, SpellImmunity, id
    ACTION_T_REMOVE_IMMUNITY            = 128,              // id, SpellImmunity, id

    ACTION_T_END
};

enum Target
{
    //Self (m_creature)
    TARGET_T_SELF = 0,                                      //Self cast

    //Hostile targets (if pet then returns pet owner)
    TARGET_T_HOSTILE,                                       //Our current target (ie: highest aggro)
    TARGET_T_HOSTILE_SECOND_AGGRO,                          //Second highest aggro (generaly used for cleaves and some special attacks)
    TARGET_T_HOSTILE_LAST_AGGRO,                            //Dead last on aggro (no idea what this could be used for)
    TARGET_T_HOSTILE_RANDOM,                                //Just any random target on our threat list
    TARGET_T_HOSTILE_RANDOM_NOT_TOP,                        //Any random target except top threat
//6
    //Invoker targets (if pet then returns pet owner)
    TARGET_T_ACTION_INVOKER,                                //Unit who caused this Event to occur (only works for EVENT_T_AGGRO, EVENT_T_KILL, EVENT_T_DEATH, EVENT_T_SPELLHIT, EVENT_T_OOC_LOS, EVENT_T_FRIENDLY_HP, EVENT_T_FRIENDLY_IS_CC, EVENT_T_FRIENDLY_MISSING_BUFF)

    //Hostile targets (including pets)
    TARGET_T_HOSTILE_WPET,                                  //Current target (can be a pet)
    TARGET_T_HOSTILE_WPET_SECOND_AGGRO,                     //Second highest aggro (generaly used for cleaves and some special attacks)
    TARGET_T_HOSTILE_WPET_LAST_AGGRO,                       //Dead last on aggro (no idea what this could be used for)
    TARGET_T_HOSTILE_WPET_RANDOM,                           //Just any random target on our threat list
    TARGET_T_HOSTILE_WPET_RANDOM_NOT_TOP,                   //Any random target except top threat
//11
    TARGET_T_ACTION_INVOKER_NOT_PLAYER,

    // use (Unit*)NULL as target
    TARGET_T_NULL,
    
    TARGET_T_ACTION_INVOKER_OWNER,                          // Unit who is responsible for Event to occur
    TARGET_T_EVENT_SENDER,                                  // Unit who sent an AIEvent that was received with EVENT_T_RECEIVE_AI_EVENT
    TARGET_T_SUMMONER,                                      // Unit who summoned creature

    TARGET_T_EVENT_SPECIFIC,

    TARGET_T_HOSTILE_RANDOM_PLAYER_NOT_TOP,

    TARGET_T_EVENT_TARGET,

    TARGET_T_END
};

enum CastFlags
{
    CAST_INTURRUPT_PREVIOUS     = 0x01,                     //Interrupt any spell casting
    CAST_TRIGGERED              = 0x02,                     //Triggered (this makes spell cost zero mana and have no cast time)
    CAST_FORCE_CAST             = 0x04,                     //Forces cast even if creature is out of mana or out of range
    CAST_NO_MELEE_IF_OOM        = 0x08,                     //Prevents creature from entering melee if out of mana or out of range
    CAST_FORCE_TARGET_SELF      = 0x10,                     //Forces the target to cast this spell on itself
    CAST_AURA_NOT_PRESENT       = 0x20,                     //Only casts the spell if the target does not have an aura from the spell
};

enum EventFlags
{
    EFLAG_REPEATABLE            = 0x01,                     //Event repeats
    EFLAG_NORMAL                = 0x02,                     //Event only occurs in Normal instance difficulty
    EFLAG_HEROIC                = 0x04,                     //Event only occurs in Heroic instance difficulty
    EFLAG_NOT_CCED              = 0x08,                     //if set then creature wouldn't process any event while CCed
    EFLAG_RESERVED_4            = 0x10,
    EFLAG_RESERVED_5            = 0x20,
    EFLAG_RESERVED_6            = 0x40,
    EFLAG_DEBUG_ONLY            = 0x80,                     //Event only occurs in debug build
};

enum SpawnedEventMode
{
    SPAWNED_EVENT_ALWAY = 0,
    SPAWNED_EVENT_MAP   = 1,
    SPAWNED_EVENT_ZONE  = 2
};

struct CreatureEventAI_Action
{
    EventAI_ActionType type: 16;
    union
    {
        // ACTION_T_TEXT                                    = 1
        struct
        {
           int32 TextId[3];
        } text;
        // ACTION_T_SET_FACTION                             = 2
        struct
        {
            uint32 factionId;                               // faction or 0 for default)
        } set_faction;
        // ACTION_T_MORPH_TO_ENTRY_OR_MODEL                 = 3
        struct
        {
            uint32 creatureId;                              // set one from fields (or 0 for both to demorph)
            uint32 modelId;
        } morph;
        // ACTION_T_SOUND                                   = 4
        struct
        {
            uint32 soundId;
        } sound;
        // ACTION_T_EMOTE                                   = 5
        struct
        {
            uint32 emoteId;
        } emote;
        // ACTION_T_RANDOM_SOUND                            = 9
        struct
        {
            int32 soundId1;                                 // (-1 in any field means no output if randomed that field)
            int32 soundId2;
            int32 soundId3;
        } random_sound;
        // ACTION_T_RANDOM_EMOTE                            = 10
        struct
        {
            int32 emoteId1;                                 // (-1 in any field means no output if randomed that field)
            int32 emoteId2;
            int32 emoteId3;
        } random_emote;
        // ACTION_T_CAST                                    = 11
        struct
        {
            uint32 spellId;
            uint32 target;
            uint32 castFlags;
        } cast;
        // ACTION_T_SUMMON                                  = 12
        struct
        {
            uint32 creatureId;
            uint32 target;
            uint32 duration;
        } summon;
        // ACTION_T_THREAT_SINGLE_PCT                       = 13
        struct
        {
            int32 percent;
            uint32 target;
        } threat_single_pct;
        // ACTION_T_THREAT_ALL_PCT                          = 14
        struct
        {
            int32 percent;
        } threat_all_pct;
        // ACTION_T_QUEST_EVENT                             = 15
        struct
        {
            uint32 questId;
            uint32 target;
        } quest_event;
        // ACTION_T_CAST_EVENT                              = 16
        struct
        {
            uint32 creatureId;
            uint32 spellId;
            uint32 target;
        } cast_event;
        // ACTION_T_SET_UNIT_FIELD                          = 17
        struct
        {
            uint32 field;
            uint32 value;
            uint32 target;
        } set_unit_field;
        // ACTION_T_SET_UNIT_FLAG                           = 18,  // value provided mask bits that will be set
        // ACTION_T_REMOVE_UNIT_FLAG                        = 19,  // value provided mask bits that will be clear
        struct
        {
            uint32 value;
            uint32 target;
        } unit_flag;
        // ACTION_T_AUTO_ATTACK                             = 20
        struct
        {
            uint32 state;                                   // 0 = stop attack, anything else means continue attacking
        } auto_attack;
        // ACTION_T_COMBAT_MOVEMENT                         = 21
        struct
        {
            uint32 state;                                   // 0 = stop combat based movement, anything else continue attacking
            uint32 melee;                                   // if set: at stop send melee combat stop if in combat, use for terminate melee fighting state for switch to ranged
        } combat_movement;
        // ACTION_T_SET_PHASE                               = 22
        struct
        {
            uint32 phase;
        } set_phase;
        // ACTION_T_INC_PHASE                               = 23
        struct
        {
            int32 step;
        } set_inc_phase;
        // ACTION_T_QUEST_EVENT_ALL                         = 26
        struct
        {
            uint32 questId;
        } quest_event_all;
        // ACTION_T_CAST_EVENT_ALL                          = 27
        struct
        {
            uint32 creatureId;
            uint32 spellId;
        } cast_event_all;
        // ACTION_T_REMOVEAURASFROMSPELL                    = 28
        struct
        {
            uint32 target;
            uint32 spellId;
        } remove_aura;
        // ACTION_T_RANGED_MOVEMENT                         = 29
        struct
        {
            uint32 distance;
            int32  angle;
        } ranged_movement;
        // ACTION_T_RANDOM_PHASE                            = 30
        struct
        {
            uint32 phase1;
            uint32 phase2;
            uint32 phase3;
        } random_phase;
        // ACTION_T_RANDOM_PHASE_RANGE                      = 31
        struct
        {
            uint32 phaseMin;
            uint32 phaseMax;
        } random_phase_range;
        // ACTION_T_SUMMON_ID                               = 32
        struct
        {
            uint32 creatureId;
            uint32 target;
            uint32 spawnId;
        } summon_id;
        // ACTION_T_KILLED_MONSTER                          = 33
        struct
        {
            uint32 creatureId;
            uint32 target;
        } killed_monster;
        // ACTION_T_SET_INST_DATA                           = 34
        struct
        {
            uint32 field;
            uint32 value;
        } set_inst_data;
        // ACTION_T_SET_INST_DATA64                         = 35
        struct
        {
            uint32 field;
            uint32 target;
        } set_inst_data64;
        // ACTION_T_UPDATE_TEMPLATE                         = 36
        struct
        {
            uint32 creatureId;
            uint32 team;
        } update_template;
        // ACTION_T_CALL_FOR_HELP                           = 39
        struct
        {
            uint32 radius;
        } call_for_help;
        // ACTION_T_SET_SHEATH                              = 40
        struct
        {
            uint32 sheath;
        } set_sheath;
        // ACTION_T_FORCE_DESPAWN                           = 41
        struct
        {
            uint32 msDelay;
        } forced_despawn;
        // ACTION_T_SET_INVINCIBILITY_HP_LEVEL              = 42
        struct
        {
            uint32 hp_level;
            uint32 is_percent;
        } invincibility_hp_level;
        // ACTION_T_MOUNT_TO_ENTRY_OR_MODEL                 = 43
        struct
        {
            uint32 creatureId;                              // set one from fields (or 0 for both to dismount)
            uint32 modelId;
        } mount;
        // ACTION_T_DYNAMIC_MOVEMENT                        = 49
        struct
        {
            uint32 state;                                   // bool: 1 = on; 0 = off
            uint32 unused1;
            uint32 unused2;
        } dynamicMovement;
        // RAW
        // ACTION_T_REMOVE_CORPSE                           = 43
        struct
        {

        } remove_corpse;
        // ACTION_T_CAST_GUID                               = 44
        struct
        {
            uint32 spellId;
            uint64 targetGUID;
            uint32 castFlags;
        } castguid;
        // ACTION_T_THROW_AI_EVENT                          = 45
        struct
        {
            uint32 eventType;
            uint32 radius;
            uint32 target;
        } throwEvent;
        // ACTION_T_CHECK_OUT_OF_THREAT                     = 45
        struct
        {
        } outofarea;
        // ACTION_T_LOAD_PATH                               = 70
        struct
        {
            uint32 PathId;
            uint32 Repeatable;
        } loadpath;
        // ACTION_T_MOVE_POINT                              = 105
        struct
        {
            uint32 positionId;
        } movepoint;
        // ACTION_T_SET_HOME_POSITION                       = 106
        struct
        {
            uint32 positionId;
        } sethomepos;
        // ACTION_T_SET_ORIENTATION                         = 107
        struct
        {
            uint32 positionId;
        } setorientation;
        // RAW
        struct
        {
            uint32 param1;
            uint32 param2;
            uint32 param3;
        } raw;
    };
};

struct CreatureEventAI_Event
{
    uint32 event_id;
    int64 entryOrGUID; // needs to be int64 for correct work
    uint32 event_inverse_phase_mask;

    EventAI_Type event_type : 16;
    uint8 event_chance : 8;
    uint8 event_flags  : 8;

    union
    {
        // EVENT_T_TIMER                                    = 0
        // EVENT_T_TIMER_OOC                                = 1
        // EVENT_T_TIMER_GENERIC                            = 29
        struct
        {
            uint32 initialMin;
            uint32 initialMax;
            uint32 repeatMin;
            uint32 repeatMax;
        } timer;
        // EVENT_T_HP                                       = 2
        // EVENT_T_MANA                                     = 3
        // EVENT_T_TARGET_HP                                = 12
        // EVENT_T_TARGET_MANA                              = 18
        struct
        {
            uint32 percentMax;
            uint32 percentMin;
            uint32 repeatMin;
            uint32 repeatMax;
        } percent_range;
        // EVENT_T_KILL                                     = 5
        struct
        {
            uint32 repeatMin;
            uint32 repeatMax;
        } kill;
        // EVENT_T_SPELLHIT                                 = 8
        struct
        {
            uint32 spellId;
            uint32 schoolMask;                              // -1 (==0xffffffff) is ok value for full mask, or must be more limited mask like (0 < 1) = 1 for normal/physical school
            uint32 repeatMin;
            uint32 repeatMax;
        } spell_hit;
        // EVENT_T_RANGE                                    = 9
        struct
        {
            uint32 minDist;
            uint32 maxDist;
            uint32 repeatMin;
            uint32 repeatMax;
        } range;
        // EVENT_T_OOC_LOS                                  = 10
        struct
        {
            uint32 noHostile;
            uint32 maxRange;
            uint32 repeatMin;
            uint32 repeatMax;
        } ooc_los;
        // EVENT_T_SPAWNED                                  = 11
        struct
        {
            uint32 condition;
            uint32 conditionValue1;
        } spawned;
        // EVENT_T_TARGET_CASTING                           = 13
        struct
        {
            uint32 repeatMin;
            uint32 repeatMax;
        } target_casting;
        // EVENT_T_FRIENDLY_HP                              = 14
        struct
        {
            uint32 hpDeficit;
            uint32 radius;
            uint32 repeatMin;
            uint32 repeatMax;
        } friendly_hp;
        // EVENT_T_FRIENDLY_IS_CC                           = 15
        struct
        {
            uint32 dispelType;                              // unused ?
            uint32 radius;
            uint32 repeatMin;
            uint32 repeatMax;
        } friendly_is_cc;
        // EVENT_T_FRIENDLY_MISSING_BUFF                    = 16
        struct
        {
            uint32 spellId;
            uint32 radius;
            uint32 repeatMin;
            uint32 repeatMax;
        } friendly_buff;
        // EVENT_T_SUMMONED_UNIT                            = 17
        // EVENT_T_SUMMONED_JUST_DIED
        // EVENT_T_SUMMONED_JUST_DESPAWN
        struct
        {
            uint32 creatureId;
            uint32 repeatMin;
            uint32 repeatMax;
        } summon_unit;
        // EVENT_T_QUEST_ACCEPT                             = 19
        // EVENT_T_QUEST_COMPLETE                           = 20
        struct
        {
            uint32 questId;
        } quest;
        // EVENT_T_RECEIVE_EMOTE                            = 22
        struct
        {
            uint32 emoteId;
            uint32 condition;
            uint32 conditionValue1;
            uint32 conditionValue2;
        } receive_emote;
        // EVENT_T_BUFFED                                   = 23
        // EVENT_T_TARGET_BUFFED                            = 24
        // EVENT_T_MISSING_AURA
        // EVENT_T_TARGET_MISSING_AURA
        struct
        {
            uint32 spellId;
            uint32 amount;
            uint32 repeatMin;
            uint32 repeatMax;
        } buffed;
        // EVENT_T_RECEIVE_AI_EVENT                         = 30
        struct
        {
            uint32 eventType;                               // See CreatureAI.h enum AIEventType - Receive only events of this type
            uint32 senderEntry;                             // Optional npc from only whom this event can be received
            uint32 unused1;
            uint32 unused2;
        } receiveAIEvent;
        // EVENT_T_SELECT_ATTACKING_TARGET                  = 32
        struct
        {
            uint32 minRange;
            uint32 maxRange;
            uint32 repeatMin;
            uint32 repeatMax;
        } selectTarget;
        // EVENT_T_TARGET_HAS_AURA_TYPE                            = 36
        struct
        {
            uint32 eAuraType;
            uint32 unused;
            uint32 repeatMin;
            uint32 repeatMax;
        } hasauratype;
        // RAW
        struct
        {
            uint32 param1;
            uint32 param2;
            uint32 param3;
            uint32 param4;
        } raw;
    };

    CreatureEventAI_Action action[MAX_ACTIONS];
};

struct CreatureEventAI_EventComputedData
{
    union
    {
        // EVENT_T_FRIENDLY_HP
        struct
        {
            bool targetSelf;
        } friendlyHp;
    };
};

//Event_Map
typedef std::vector<CreatureEventAI_Event> CreatureEventAI_Event_Vec;
typedef UNORDERED_MAP<int64, CreatureEventAI_Event_Vec> CreatureEventAI_Event_Map;
typedef UNORDERED_MAP<int64, CreatureEventAI_EventComputedData> CreatureEventAI_EventComputedData_Map;

struct CreatureEventAI_Summon
{
    uint32 id;

    float position_x;
    float position_y;
    float position_z;
    float orientation;
    uint32 SpawnTimeSecs;
};

//EventSummon_Map
typedef UNORDERED_MAP<uint32, CreatureEventAI_Summon> CreatureEventAI_Summon_Map;

struct CreatureEventAI_Positions
{
    uint32 id;

    float position_x;
    float position_y;
    float position_z;
    float orientation;
};
//EventPositions_Map
typedef UNORDERED_MAP<uint32, CreatureEventAI_Positions> CreatureEventAI_Positions_Map;

struct CreatureEventAIHolder
{
    CreatureEventAIHolder(CreatureEventAI_Event p) : Event(p), Time(0), Enabled(true){}

    CreatureEventAI_Event Event;
    uint32 Time;
    bool Enabled;

    // helper
    bool UpdateRepeatTimer(Creature* creature, uint32 repeatMin, uint32 repeatMax);
};

class HELLGROUND_IMPORT_EXPORT CreatureEventAI : public CreatureAI
{

    public:
        explicit CreatureEventAI(Creature *c);
        ~CreatureEventAI()
        {
            CreatureEventAIList.clear();
        }
        void JustRespawned();
        void Reset();
        void JustReachedHome();
        void MovementInform(uint32 /*MovementType*/, uint32 /*Data*/);
        void EnterCombat(Unit *enemy);
        void EnterEvadeMode();
        void JustDied(Unit* killer);
        void KilledUnit(Unit* victim);
        void JustSummoned(Creature* pUnit);
        void SummonedCreatureDies(Creature* summoned);
        void SummonedCreatureDespawn(Creature* summoned);
        // void OnQuestAccept(Player* pPlayer, Quest const* pQuest);
        void AttackStart(Unit *who);
        void MoveInLineOfSight(Unit *who);
        void SpellHit(Unit* pUnit, const SpellEntry* pSpell);
        void DamageTaken(Unit* done_by, uint32& damage);
        void UpdateAI(const uint32 diff);
        void ReceiveEmote(Player* pPlayer, uint32 text_emote);
        static int Permissible(const Creature *);
        void GetDebugInfo(ChatHandler& reader);

        bool ProcessEvent(CreatureEventAIHolder& pHolder, Unit* pActionInvoker = NULL);
        void ProcessAction(CreatureEventAI_Action const& action, uint32 rnd, uint32 EventId, Unit* pActionInvoker);
        inline uint32 GetRandActionParam(uint32 rnd, uint32 param1, uint32 param2, uint32 param3);
        inline int32 GetRandActionParam(uint32 rnd, int32 param1, int32 param2, int32 param3);
        inline Unit* GetTargetByType(uint32 Target, Unit* pActionInvoker);

        void DoScriptText(int32 textEntry, WorldObject* pSource, Unit* target);
        bool CanCast(Unit* Target, SpellEntry const *Spell, bool Triggered);

        bool SpawnedEventConditionsCheck(CreatureEventAI_Event const& event);

        Unit* SelectLowestHpFriendly(float range, uint32 MinHPDiff, bool targetSelf);
        void FindFriendlyMissingBuff(std::list<Creature*>& _list, float range, uint32 spellid);
        void FindFriendlyCC(std::list<Creature*>& _list, float range);
        void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 miscValue) override;

        void SetCombatMove(bool enable, bool stopOrStartMovement /*=false*/);

        void OnSpellInterrupt(SpellEntry const* spellInfo) override;

                                                            //Holder for events (stores enabled, time, and eventid)
        std::list<CreatureEventAIHolder> CreatureEventAIList;
        Timer EventUpdateTime;                             //Time between event updates
        uint32 EventDiff;                                   //Time between the last event call
        bool bEmptyList;

        //Variables used by Events themselves
        uint8 Phase;                                        // Current phase, max 32 phases
        bool CombatMovementEnabled;                         // If we allow targeted movment gen (movement twoards top threat)
        bool MeleeEnabled;                                  // If we allow melee auto attack
        float AttackDistance;                               // Distance to attack from
        float AttackAngle;                                  // Angle of attack
        uint32 InvinceabilityHpLevel;                       // Minimal health level allowed at damage apply
        float LastSpellMaxRange;                            // Maximum spell range that was cast during dynamic movement
        Unit* m_eventTarget;                                // Target filled on specific event to be used in action
        Unit* summoned;
        uint32 MoveChaseOnAttackTimer;
};

#endif
