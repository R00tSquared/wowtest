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

#ifndef HELLGROUND_CUSTOMDEFINES_H
#define HELLGROUND_CUSTOMDEFINES_H

#include "Platform/Define.h"
#include <cassert>
#include <Common.h>

#define REALM_X3 0
#define REALM_X100 1

#define TWINK_LEVEL_MIN 60 // if you change this, you also need to change requiredlevel for items
#define TWINK_LEVEL_MAX 60 // if you change this, you also need to change requiredlevel for items

#define MAX_ARENA_REPORTS 4

enum DeserterDurations
{
    DESERTER_ARENA_DURATION_REPORT = 2 * HOUR * MILLISECONDS,
    DESERTER_ARENA_DURATION_AFK = 5 * MINUTE * MILLISECONDS,
    DESERTER_BG_DURATION_LEAVE = 30 * MINUTE * MILLISECONDS,
    DESERTER_BG_DURATION_AFK = 5 * MINUTE * MILLISECONDS,
};

#define MAX_TALENT_RANK 5
#define MAX_TALENT_SPECS 6
#define SOLO_3V3_REQUIRED_RESILIENCE 150

#define BOSS_AUTOEVADE_RESTART_TIME 5 * MINUTE

#define SPELL_CATEGORY_MORPH_SHIRT 5000
#define SPELL_CATEGORY_AURA_TABARD 5001

enum GearScore
{
    GS_EPIC_START = 1820, // full vendor = 1855
    GS_BADGE = 2080, // fully equipped = 2115
    GS_GOOD = 2150, //
    GS_SUN = 2400 // full char
};

enum SpellShared
{
    SPELL_BG_DESERTER = 26013, // not custom
    SPELL_ARENA_DESERTER = 55402,
    
    SPELL_BG_RATED = 55412,
    SPELL_BG_ALLIANCE_ICON = 55411,
    SPELL_BG_HORDE_ICON = 55410,
    //SPELL_BG_FACTION_SWITCHER = 54844,

    SPELL_AV_CROSSFAC_H_A = 55203, // Horde player on Alliance side
    SPELL_AV_CROSSFAC_A_H = 55204, // Alliance player on Horde side

    SPELL_GUILD_HOUSE_STATS_BUFF = 42741, // 10% for 24h
    SPELL_ROGUE_RAID_BUFF = 54863,

    SPELL_MORPHING_SHIRT_DRESSER = 55427,
    SPELL_AURA_TABARD_DRESSER = 55428,
};

// custom items
#define PREMIUM_REMOTE 31665
#define BOOK_OF_TELEPORTATION 1136

#define ITEM_SYMBOL_OF_VALOR 21787 //epic badge
#define ITEM_EMBLEM_OF_TRIUMPH 20880 //leg badge

#define GM_LIKE_ITEM 0 // NEED TO BE REMOVED FROM SOURCES
//#define BADGE_OF_WAR 37598
#define BOJ_FRAGMENT 23026

#define RARE_KEY_FRAGMENT 5696
#define EPIC_KEY_FRAGMENT 5697
#define LEGENDARY_KEY_FRAGMENT 5698

#define BUFF_SCROLL 875
//#define ENDLESS_BUFF_SCROLL 30526
#define LAMP_WITH_A_WISP 22455
#define REWARD_ORDER 20484
#define ELIXIR_OF_HARMONY 22258
#define MOON_COIN 34519
#define SECOND_BADGE 13215
#define ITEM_BADGE 29434
#define TALENT_BOOK 23689
#define SCROLL_OF_INDULGENCE 28877

#define ANTIQUE_SHARD 32372
#define AZERITE_SHARD 21141
#define GHOST_SHARD 10478

#define ITEM_SUMMONING_STONE 11228

// keys
#define ANTIQUE_KEY 2363
//#define RARE_KEY 6207
#define EPIC_KEY 6208
#define LEGENDARY_KEY 6209
#define FLAWLESS_LEGENDARY_KEY 4451
#define AZERITE_KEY 3580
#define GHOST_KEY 5384

// custom chests (for disable rates)
// don't forget IsCustomChest()
#define TEST_CHEST 1000
#define BONUS_CHEST 5857
#define ANTIQUE_CHEST 7246
#define REFERRAL_CHEST 10719
#define SUBSCRIBER_CHEST 23656
//#define RARE_RAID_CHEST 27316
#define EPIC_RAID_CHEST 27318
#define LEGENDARY_RAID_CHEST 27319
#define AZERITE_CHEST 21811
#define GHOST_CHEST 23861
#define OLD_MAN_CHEST 20020
#define VOTE_CHEST 20366

// transmog
#define TRANSMOGRIFICATOR 38234
#define TRANSMOG_BATTERY_1 16086 // weapon
#define TRANSMOG_BATTERY_2 16102 // head, shoulders
#define TRANSMOG_BATTERY_3 16103 // chest, legs
#define TRANSMOG_BATTERY_4 16104 // feet, waist, hands, wirst, cloak

// mounts
#define GLAD_DRAKE 30609
#define VENG_DRAKE 37676
#define MERC_DRAKE 34092

// custom mounts, don't forget to add fix at // fix for cancel moonkin aura when use custom mount
#define SNOW_BEAR 1698
#define LION 12440
#define BONE_GRYPHON 7388
#define SMOKY_PANTHER 23712
#define BRONZE_DRAGON 20221
#define ROTTEN_BEAR 1691
#define DRAGONHAWK 4966
#define FANTASTIC_TURTLE 21233
#define BENGAL_TIGER 8630
#define WHITE_DEER 1699
#define TEMPORAL_RIFT_DRAGON 5331

#define PINK_ELEKK 28482
#define LEOPARD 23940
#define UNICORN 2413

#define RED_QIRAJI 27782 // 2400 arena
#define BLUE_QIRAJI 27863 // guild house owners

// mount items
#define BONE_GRYPHON_SCALE 23961

// events
#define EVENT_GUILD_HOUSE 1016
#define EVENT_WORLDBOSS_TRIGGER 1012
#define EVENT_WORLDBOSS_START 1013
#define EVENT_WORLDBOSS_END 1015

#define BG_EVENT_DURATION_MINUTES 60

// legendary weapons
// if adding new DONT FORGET to update IsCustomLegendaryWeapon()
#define ITEM_LEGENDARY_AXE 693900
#define ITEM_LEGENDARY_DAGGER_ROGUE 693901
#define ITEM_LEGENDARY_FIST_MH 693902
#define ITEM_LEGENDARY_FIST_OH 693903
#define ITEM_LEGENDARY_FIST_OH_SLOW 693904
#define ITEM_LEGENDARY_DAGGER_SPD 693905
#define ITEM_LEGENDARY_MACE_SPD 693906
#define ITEM_LEGENDARY_STAFF_SPD 693907
#define ITEM_LEGENDARY_STAFF_HEAL 693908
#define ITEM_LEGENDARY_MACE_HEAL 693909
#define ITEM_LEGENDARY_STAFF_FERAL 693910
#define ITEM_LEGENDARY_SWORD_TANK 693911
#define ITEM_LEGENDARY_GLAIVE_RIGHT 32837
#define ITEM_LEGENDARY_GLAIVE_LEFT 32838
#define ITEM_LEGENDARY_BOW 34334

#define ITEM_TOKEN_NAME_CHANGE 33572
#define ITEM_TOKEN_APPEARANCE_CHANGE 33570
#define ITEM_TOKEN_GENDER_CHANGE 33573
#define ITEM_TOKEN_RACE_CHANGE_ALLIANCE 23700
#define ITEM_TOKEN_RACE_CHANGE_HORDE 23701

// key chest
struct ChestKeys
{
    std::vector<uint32> keys;
    uint32 chest;
};

const std::vector<ChestKeys> ChestsRequiredKeys = {
    { { ANTIQUE_KEY }, ANTIQUE_CHEST },
    { { EPIC_KEY }, EPIC_RAID_CHEST },
    { { FLAWLESS_LEGENDARY_KEY, LEGENDARY_KEY }, LEGENDARY_RAID_CHEST },
    { { AZERITE_KEY }, AZERITE_CHEST },
    { { GHOST_KEY }, GHOST_CHEST },
    { { 0 }, TEST_CHEST },
};

struct BossSouls {
    uint8 id;
    uint32 item_soul;
    uint32 reward_count;
};

const std::vector<BossSouls> boss_souls_template =
{
    // t4
    {1, 26562, 1},  // Soul of Gruul the Dragonkiller
    {2, 26572, 1},  // Soul of Magtheridon
    {3, 26564, 2}, // Soul of Prince Malchezaar
    //t5
    {4, 26563, 2},  // Soul of Kael'thas Sunstrider
    {5, 26561, 2},  // Soul of Lady Vashj
    {6, 26568, 2},  // Soul of Zul'jin
    //t6
    {7, 26567, 2},  // Soul of Archimonde
    {8, 26566, 3},  // Soul of Illidan Stormrage
    {9, 26565, 2},  // Soul of Kil'jaeden
    //classic
    {10, 26571, 1},  // Soul of Onyxia
    {11, 26573, 3},  // Soul of Kel'Thuzad
};

struct LegendaryWeapons {
    uint32 entry;
    double value;
};

const std::vector<LegendaryWeapons> legendary_weapons = {
    {ITEM_LEGENDARY_AXE, 1.0},
    {ITEM_LEGENDARY_DAGGER_ROGUE, 0.5},
    {ITEM_LEGENDARY_FIST_MH, 0.6},
    {ITEM_LEGENDARY_FIST_OH, 0.4},
    {ITEM_LEGENDARY_FIST_OH_SLOW, 0.4},
    {ITEM_LEGENDARY_DAGGER_SPD, 1.0},
    {ITEM_LEGENDARY_MACE_SPD, 1.0},
    {ITEM_LEGENDARY_STAFF_SPD, 1.0},
    {ITEM_LEGENDARY_STAFF_HEAL, 1.0},
    {ITEM_LEGENDARY_MACE_HEAL, 1.0},
    {ITEM_LEGENDARY_STAFF_FERAL, 1.0},
    {ITEM_LEGENDARY_SWORD_TANK, 1.0},
    {ITEM_LEGENDARY_GLAIVE_RIGHT, 0.6},
    {ITEM_LEGENDARY_GLAIVE_LEFT, 0.4},
    {ITEM_LEGENDARY_BOW, 1.0}
};

enum CustomEvents
{
    EVENT_BG_ARATHI = 150,
    EVENT_BG_EYE = 151,
    EVENT_BG_WARSONG = 152,
    EVENT_BG_ALTERAC = 153,
};

enum CustomNPCs
{
    NPC_GOLEM_GUARDIAN = 693116,
    NPC_VIOLET = 693133, // free gear vendor
    NPC_MAJOR_DUNTELO = 693125, // socket vendor
    NPC_NODALON = 693134, // BoJ vendor
    NPC_GRITHENA = 693017, // MC vendor
};

// reserved 100 -> 690900-690999 (should not exist)
enum MultivendorNPCs
{
    NPC_MULTIVENDOR_FIRST = 690900,
    NPC_MULTIVENDOR_LAST = 690999,
};

// reserved 200 -> 690700-690899
enum CreditMarkerNPCs
{
    NPC_CREDIT_MARKER_FIRST = 690700,
    NPC_CREDIT_MARKER_LAST = 690899,
};

// class, subclass, class_mask
//const uint32 item_types_classmask[26][3] =
//{
//    //{ 2, 0, 39, },  //axe_one_handed
//    //{ 2, 1, 39 },   //axe_two_handed
//    //{ 2, 2, 13 },   //bow
//    //{ 2, 3, 13 },   //gun
//    //{ 2, 4, 315 },  //mace_one_handed
//    //{ 2, 5, 291 },  //mace_two_handed
//    //{ 2, 6, 7 },    //polearm
//    //{ 2, 7, 207 },  //sword_one_handed
//    //{ 2, 8, 7 },    //sword_two_handed
//    //{ 2, 10, 500 }, //staff
//    //{ 2, 13, 511 }, //fist_weapon
//    //{ 2, 15, 509 }, //dagger
//    //{ 2, 16, 13 },  //thrown
//    //{ 2, 18, 13 },  //crossbow
//    //{ 2, 19, 208 }, //wand
//    { 4, 1, 208 },  //cloth
//    { 4, 2, 264 },  //leather
//    { 4, 3, 36 },   //mail
//    { 4, 4, 3 },    //plate
//    //{ 4, 6, 35 },   //shield
//    { 4, 0, 0 },    //ring_neck_trinket
//    //{ 4, 7, 2 },    //libram
//    //{ 4, 8, 256 },  //idol
//    //{ 4, 9, 32 },   //totem
//    //{ 2, 21, 464 }, //holdable
//    //{ 4, 10, 0 },   //cloak
//};

#endif

