/*
* Copyright (C) 2009 TrinityCore <http://www.trinitycore.org/>
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

#ifndef BOSS_ILLIDAN_H
#define BOSS_ILLIDAN_H

#include "precompiled.h"



/**************** ILLIDAN DEFINES *******************/



/************* Quotes and Sounds ***********************/
// Gossip for when a player clicks Akama
#define GOSSIP_ITEM           "We are ready to face Illidan"


// Yells for/by Akama
#define SAY_AKAMA_BEWARE      -1999988

// I think I'll fly now and let my subordinates take you on
#define SAY_SUMMONFLAMES      -1999994

#define SPELL_SHADOWFIEND_PASSIVE       41913 // Passive aura for shadowfiends

// Other defines
#define CENTER_X            676.740
#define CENTER_Y            305.297
#define CENTER_Z            354.519

#define SPELL_CAGED                     40695 // Caged Trap triggers will cast this on Illidan if he is within 3 yards

struct Locations
{
    float x, y, z;
};

static Locations HoverPosition[] =
{
    { 657, 340, 354.519f },
    { 657, 275, 354.519f },
    { 705, 275, 354.519f },
    { 705, 340, 354.519f }
};

static Locations GlaivePosition[] =
{
    { 676.226f, 325.230f, 354.319f },
    { 678.059f, 285.220f, 354.319f }
};

static Locations EyeBlast[] =
{
    { 640.839f, 276.951f, 354.519f },
    { 638.110f, 306.876f, 354.519f },
    { 710.786f, 266.433f, 354.519f },
    { 711.203f, 343.015f, 354.519f },
    { 711.649f, 267.606f, 354.519f }
};

enum IllidanPhase
{
    PHASE_NULL = 0,
    PHASE_ONE = 1,
    PHASE_TWO = 2,
    PHASE_THREE = 3,
    PHASE_FOUR = 4,
    PHASE_FIVE = 5,
    PHASE_MAIEV = 6,
    PHASE_DEATH = 7
};

enum IllidanSpell
{
    SPELL_ILLIDAN_DUAL_WIELD = 42459,
    SPELL_ILLIDAN_KNEEL_INTRO = 39656,

    // Phase 1 spells
    SPELL_ILLIDAN_SHEAR = 41032, // proper
    SPELL_ILLIDAN_DRAW_SOUL = 40904, // need to implement scripteffect
    SPELL_ILLIDAN_FLAME_CRASH = 40832,
    SPELL_ILLIDAN_PARASITIC_SHADOWFIEND = 41917,

    // Phase 3 spells
    SPELL_ILLIDAN_AGONIZING_FLAMES = 40932,

    // Phase 5 spells
    SPELL_ILLIDAN_ENRAGE = 40683,

    // Phase 2 spells
    SPELL_ILLIDAN_THROW_GLAIVE = 39849,
    SPELL_ILLIDAN_GLAIVE_RETURN = 39873,
    SPELL_ILLIDAN_FIREBALL = 40598,
    SPELL_ILLIDAN_DARK_BARRAGE = 40585,
    SPELL_ILLIDAN_EYE_BLAST = 39908,

    // Phase 4 spells
    SPELL_ILLIDAN_DEMON_TRANSFORM_1 = 40511,
    SPELL_ILLIDAN_DEMON_TRANSFORM_2 = 40398,
    SPELL_ILLIDAN_DEMON_TRANSFORM_3 = 40510,
    SPELL_ILLIDAN_DEMON_FORM = 40506,
    SPELL_ILLIDAN_SHADOW_BLAST = 41078,
    SPELL_ILLIDAN_FLAME_BURST = 41126,
    SPELL_ILLIDAN_SHADOW_DEMON = 41120,
    SPELL_ILLIDAN_SHADOW_DEMON_CAST = 41117,

    SPELL_ILLIDAN_INPRISON_RAID = 40647,
    SPELL_ILLIDAN_SUMMON_MAIEV = 40403,

    SPELL_ILLIDAN_HARD_ENRAGE = 45078,

    SPELL_ILLIDAN_DEATH_OUTRO = 41220
};

enum IllidanEvent
{
    EVENT_ILLIDAN_START = 1,
    EVENT_ILLIDAN_CHANGE_PHASE = 2,
    EVENT_ILLIDAN_SUMMON_MINIONS = 3,

    // Phase 1,3,5 events
    EVENT_ILLIDAN_SHEAR = 5,
    EVENT_ILLIDAN_DRAW_SOUL = 6,
    EVENT_ILLIDAN_FLAME_CRASH = 7,
    EVENT_ILLIDAN_PARASITIC_SHADOWFIEND = 8,

    EVENT_ILLIDAN_AGONIZING_FLAMES = 9,
    EVENT_ILLIDAN_SOFT_ENRAGE = 10,

    // Phase 2 events
    EVENT_ILLIDAN_THROW_GLAIVE = 11,
    EVENT_ILLIDAN_EYE_BLAST = 12,
    EVENT_ILLIDAN_DARK_BARRAGE = 13,
    EVENT_ILLIDAN_SUMMON_TEAR = 14,
    EVENT_ILLIDAN_RETURN_GLAIVE = 15,
    EVENT_ILLIDAN_LAND = 16,

    EVENT_ILLIDAN_TRANSFORM_NO1 = 17,
    EVENT_ILLIDAN_TRANSFORM_NO2 = 18,
    EVENT_ILLIDAN_TRANSFORM_NO3 = 19,
    EVENT_ILLIDAN_TRANSFORM_NO4 = 20,
    EVENT_ILLIDAN_TRANSFORM_NO5 = 21,
    EVENT_ILLIDAN_FLAME_BURST = 22,
    EVENT_ILLIDAN_SHADOW_DEMON = 23,
    EVENT_ILLIDAN_TRANSFORM_BACKNO1 = 24,
    EVENT_ILLIDAN_TRANSFORM_BACKNO2 = 25,
    EVENT_ILLIDAN_TRANSFORM_BACKNO3 = 26,
    EVENT_ILLIDAN_TRANSFORM_BACKNO4 = 27,

    // Phase: Maiev summon
    EVENT_ILLIDAN_SUMMON_MAIEV = 28,
    EVENT_ILLIDAN_INPRISON_RAID = 29,
    EVENT_ILLIDAN_CAGE_TRAP = 30,

    EVENT_ILLIDAN_KILL = 31,
    EVENT_ILLIDAN_DEATH_SPEECH = 32,

    EVENT_ILLIDAN_FLAME_DEATH = 33,

    EVENT_ILLIDAN_RANDOM_YELL
};

enum IllidanTexts
{
    YELL_ILLIDAN_AGGRO = -1529002,
    YELL_ILLIDAN_SUMMON_MINIONS = -1999989,

    YELL_ILLIDAN_SLAIN = -1999991,
    YELL_ILLIDAN_SLAIN2 = -1999992,

    YELL_ILLIDAN_PHASE_TWO = -1999993,
    YELL_ILLIDAN_EYE_BLAST = -1999995,
    YELL_ILLIDAN_DEMON_FORM = -1999996,

    YELL_ILLIDAN_TAUNT_NO1 = -1529016,
    YELL_ILLIDAN_TAUNT_NO2 = -1529017,
    YELL_ILLIDAN_TAUNT_NO3 = -1529018,
    YELL_ILLIDAN_TAUNT_NO4 = -1529019,

    YELL_ILLIDAN_INPRISON_RAID = -1529003,
    YELL_ILLIDAN_DEATH_SPEECH = -1529009, // You have won... Maiev...but the huntress... is nothing...without the hunt... you... are nothing... without me..
    YELL_ILLIDAN_HARD_ENRAGE = -1999997
};

enum CreatureEntries
{
    BLADE_OF_AZZINOTH = 22996,
    FLAME_OF_AZZINOTH = 22997,
    GLAIVE_TARGET = 23448,
    ILLIDARI_ELITE = 23226,
    PARASITIC_SHADOWFIEND = 23498,
    CAGE_TRAP_TRIGGER = 23292
};

class GlaiveTargetRespawner
{
public:
    GlaiveTargetRespawner() {}

    void operator()(Creature* u) const
    {
        if (u->GetEntry() == GLAIVE_TARGET)
            u->Respawn();
    }
    void operator()(GameObject* u) const { }
    void operator()(WorldObject*) const {}
    void operator()(Corpse*) const {}
};



/**************** AKAMA DEFINES *******************/




static float SpiritSpawns[][4] =
{
    { 23411, 755.5426, 309.9156, 312.2129 },
    { 23410, 755.5426, 298.7923, 312.0834 }
};

//Akama spells
enum AkamaSpells
{
    SPELL_AKAMA_DOOR_CAST_SUCCESS = 41268,
    SPELL_AKAMA_DOOR_CAST_FAIL = 41271,
    SPELL_DEATHSWORN_DOOR_CHANNEL = 41269,
    SPELL_AKAMA_POTION = 40535,
    SPELL_AKAMA_CHAIN_LIGHTNING = 40536  // 6938 to 8062 for 5 targets
};

enum ConversationText
{
    SAY_ILLIDAN_NO1 = -1999998,
    SAY_AKAMA_NO1 = -1529099,
    SAY_ILLIDAN_NO2 = -1529000,
    SAY_AKAMA_NO2 = -1529001,
};

enum AkamaEvents
{
    EVENT_AKAMA_START = 1,
    EVENT_AKAMA_TALK_SEQUENCE_NO1 = 2,
    EVENT_AKAMA_TALK_SEQUENCE_NO2 = 3,
    EVENT_AKAMA_TALK_SEQUENCE_NO3 = 4,
    EVENT_AKAMA_TALK_SEQUENCE_NO4 = 5,

    EVENT_AKAMA_SET_DOOR_EVENT = 6,

    EVENT_AKAMA_ILLIDAN_FIGHT = 7,
    EVENT_AKAMA_MINIONS_FIGHT = 8,
    EVENT_AKAMA_SUMMON_ELITE,

    EVENT_AKAMA_DOOR_CAST_FAIL,
    EVENT_AKAMA_SUMMON_SPIRITS,
    EVENT_AKAMA_DOOR_CAST_SUCCESS,
    EVENT_AKAMA_DOOR_OPEN,
    EVENT_AKAMA_DOOR_MOVE_PATH,
    EVENT_AKAMA_RETURN_ILLIDAN,
    EVENT_AKAMA_END_ILLIDAN,
};

enum AkamaTexts
{
    SAY_AKAMA_DOOR_SPEECH_NO1 = -1309025,
    SAY_AKAMA_DOOR_SPEECH_NO2 = -1309024,
    SAY_AKAMA_DOOR_SPEECH_NO3 = -1309028,

    YELL_AKAMA_FIGHT_MINIONS = -1999990,

    SAY_SPIRIT_DOOR_SPEECH_NO1 = -1309026,
    SAY_SPIRIT_DOOR_SPEECH_NO2 = -1309027,
    SAY_AKAMA_ILLIDAN_END      = -1529011, // The Light will fill these dismal halls once again. I swear it.
};

enum AkamaPath
{
    PATH_AKAMA_MINION_EVENT = 2111,
    PATH_AKAMA_DOOR_EVENT_AFTER = 2109,
    PATH_AKAMA_DOOR_EVENT_BEFORE = 2110
};



/************************ MAIEV DEFINES **********************/




enum MaievTaunts
{
    MAIEV_TAUNT_NO1 = -1529020,
    MAIEV_TAUNT_NO2 = -1529021,
    MAIEV_TAUNT_NO3 = -1529022,
    MAIEV_TAUNT_NO4 = -1529023
};

enum MaievTexts
{
    YELL_MAIEV_TALK_SEQUENCE_NO1 = -1529004,
    YELL_MAIEV_TALK_SEQUENCE_NO2 = -1529006,
    YELL_ILLIDAN_TALK_SQUENCE_NO1 = -1529005,
    YELL_MAIEV_ILLIDAN_END = -1529008, // It is finished. You are beaten.
    YELL_MAIEV_ILLIDAN_END_2 = -1529010, // He is right. I feel nothing... I am nothing...
    YELL_MAIEV_ILLIDAN_END_3 = -1999980, // Farewell, champions.
};

enum MaievEvents
{
    EVENT_MAIEV_TALK_SEQUENCE_NO1 = 1,
    EVENT_MAIEV_TALK_SEQUENCE_NO2 = 2,
    EVENT_MAIEV_TALK_SEQUENCE_NO3 = 3,
    EVENT_MAIEV_RANGE_ATTACK = 4,
    EVENT_MAIEV_CAGE_TRAP = 5,
    EVENT_MAIEV_BEGIN_FIGHT = 6,
    EVENT_MAIEV_END_FIGHT_SPEECH = 7,
    EVENT_MAIEV_RANDOM_TAUNT = 8,
    EVENT_MAIEV_END_SPEECH_2 = 9,
    EVENT_MAIEV_END_SPEECH_3 = 10,
    EVENT_MAIEV_DESPAWN      = 11,
};

enum MaievSpells
{
    SPELL_MAIEV_TELEPORT_VISUAL = 41232,
    SPELL_MAIEV_THROW_DAGGER = 41152,
    SPELL_MAIEV_SUMMON_CAGE_TRAP = 40694,
    SPELL_MAIEV_CAGE_TRAP_TRIGGER = 40761
};

#define SPELL_SHADOW_STRIKE             40685 // 4375 to 5625 every 3 seconds for 12 seconds
#define SPELL_FAN_BLADES                39954 // bugged visual



/*********************** GLAIVE DEFINES ***********************/



enum GlaiveSpells
{
    SPELL_GLAIVE_SUMMON_TEAR = 39855,
    SPELL_GLAIVE_CHANNEL = 39857
};



/*********************** FLAME OF AZZINOTH DEFINES ***********************/



enum FlameEvents
{
    EVENT_FLAME_RANGE_CHECK = 1,
    EVENT_FLAME_FLAME_BLAST = 2
};

enum FlameSpells
{
    SPELL_FLAME_FLAME_BLAST = 40631,
    SPELL_FLAME_BLAZE = 40609,
    SPELL_FLAME_CHARGE = 40602,
    SPELL_FLAME_ENRAGE = 45078
};



/*********************** SHADOW DEMON DEFINES ***********************/



enum ShadowDemonSpells
{
    SPELL_SHADOW_DEMON_PASSIVE = 41079,
    SPELL_SHADOW_DEMON_CONSUME_SOUL = 41080,
    SPELL_SHADOW_DEMON_FOUND_TARGET = 41082,
    SPELL_SHADOW_DEMON_BEAM = 39123,
    SPELL_SHADOW_DEMON_PARALYZE = 41083
};

#endif