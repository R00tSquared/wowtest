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

#ifndef HELLGROUND_DBCSTRUCTURE_H
#define HELLGROUND_DBCSTRUCTURE_H

#include "DBCEnums.h"
#include "Platform/Define.h"
#include "Path.h"
#include "SharedDefines.h"
#include <map>
#include <set>
#include <vector>

// Structures using to access raw DBC data and required packing to portability

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

struct AreaTableEntry
{
    uint32    ID;                                           // 0
    uint32    mapid;                                        // 1
    uint32    zone;                                         // 2 if 0 then it's zone, else it's zone id of this area
    uint32    exploreFlag;                                  // 3, main index
    uint32    flags;                                        // 4, unknown value but 312 for all cities
                                                            // 5-9 unused
    int32     area_level;                                   // 10
    char*     area_name[16];                                // 11-26
                                                            // 27, string flags, unused
    uint32    team;                                         // 28
};

struct AreaTriggerEntry
{
    uint32    id;                                           // 0
    uint32    mapid;                                        // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    float     radius;                                       // 5
    float     box_x;                                        // 6 extent x edge
    float     box_y;                                        // 7 extent y edge
    float     box_z;                                        // 8 extent z edge
    float     box_orientation;                              // 9 extent rotation by about z axis
};

struct AuctionHouseEntry
{
    uint32    houseId;                                      // 0 index
    uint32    faction;                                      // 1 id of faction.dbc for player factions associated with city
    uint32    depositPercent;                               // 2 1/3 from real
    uint32    cutPercent;                                   // 3
    //char*     name[16];                                   // 4-19
                                                            // 20 string flag, unused
};

struct BankBagSlotPricesEntry
{
    uint32      ID;
    uint32      price;
};

struct BattlemasterListEntry
{
    uint32      id;                                         // 0
    uint32      mapid[3];                                   // 1-3 mapid
                                                            // 4-8 unused
    uint32      type;                                       // 9 (3 - BG, 4 - arena)
    uint32      minlvl;                                     // 10
    uint32      maxlvl;                                     // 11
    uint32      maxplayersperteam;                          // 12
                                                            // 13-14 unused
    char*       name[16];                                   // 15-30
                                                            // 31 string flag, unused
                                                            // 32 unused
};

#define MAX_OUTFIT_ITEMS 12
// #define MAX_OUTFIT_ITEMS 24                              // 12->24 in 3.0.x

struct CharStartOutfitEntry
{
    //uint32 Id;                                            // 0
    uint32 RaceClassGender;                                 // 1 (UNIT_FIELD_BYTES_0 & 0x00FFFFFF) comparable (0 byte = race, 1 byte = class, 2 byte = gender)
    int32 ItemId[MAX_OUTFIT_ITEMS];                         // 2-13
    //int32 ItemDisplayId[MAX_OUTFIT_ITEMS];                // 14-25 not required at server side
    //int32 ItemInventorySlot[MAX_OUTFIT_ITEMS];            // 26-37 not required at server side
    //uint32 Unknown1;                                      // 38, unique values (index-like with gaps ordered in other way as ids)
    //uint32 Unknown2;                                      // 39
    //uint32 Unknown3;                                      // 40
};

struct CharTitlesEntry
{
    uint32      ID;                                         // 0, title ids, for example in Quest::GetCharTitleId()
    //uint32      unk1;                                     // 1 flags?
    //char*       name[16];                                 // 2-17, unused
                                                            // 18 string flag, unused
    //char*       name2[16];                                // 19-34, unused
                                                            // 35 string flag, unused
    uint32      bit_index;                                  // 36 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER__FIELD_KNOWN_TITLES
};

struct ChatChannelsEntry
{
    uint32      ChannelID;                                  // 0
    uint32      flags;                                      // 1
    char*       pattern[16];                                // 3-18
                                                            // 19 string flags, unused
    //char*       name[16];                                 // 20-35 unused
                                                            // 36 string flag, unused
};

struct ChrClassesEntry
{
    uint32      ClassID;                                    // 0
                                                            // 1-2, unused
    uint32      powerType;                                  // 3
                                                            // 4, unused
    char*       name[16];                                   // 5-20 unused
                                                            // 21 string flag, unused
    //char*       nameFemale[16];                           // 21-36 unused, if different from base (male) case
                                                            // 37 string flag, unused
    //char*       nameNeutralGender[16];                    // 38-53 unused, if different from base (male) case
                                                            // 54 string flag, unused
                                                            // 55, unused
    uint32      spellfamily;                                // 56
                                                            // 57, unused
};

struct ChrRacesEntry
{
    uint32      RaceID;                                     // 0
                                                            // 1 unused
    uint32      FactionID;                                  // 2 facton template id
                                                            // 3 unused
    uint32      model_m;                                    // 4
    uint32      model_f;                                    // 5
                                                            // 6-7 unused
    uint32      TeamID;                                     // 8 (7-Alliance 1-Horde)
                                                            // 9-12 unused
    uint32      CinematicSequence;                          // 13 id from CinematicCamera.dbc
    char*       name[16];                                   // 14-29 used for DBC language detection/selection
                                                            // 30 string flags, unused
    //char*       nameFemale[16];                           // 31-46, if different from base (male) case
                                                            // 47 string flags, unused
    //char*       nameNeutralGender[16];                    // 48-63, if different from base (male) case
                                                            // 64 string flags, unused
                                                            // 65-67 unused
    uint32      addon;                                      // 68 (0 - original race, 1 - tbc addon, ...)
};

/* not used
struct CinematicCameraEntry
{
    uint32      id;                                         // 0 index
    char*       filename;                                   // 1
    uint32      soundid;                                    // 2 in SoundEntries.dbc or 0
    float       start_x;                                    // 3
    float       start_y;                                    // 4
    float       start_z;                                    // 5
    float       unk6;                                       // 6 speed?
};
*/

struct CinematicSequencesEntry
{
    uint32      Id;                                         // 0 index
    //uint32      unk1;                                     // 1 always 0
    //uint32      cinematicCamera;                          // 2 id in CinematicCamera.dbc
                                                            // 3-9 always 0
};

struct CreatureDisplayInfoEntry
{
    uint32      Displayid;                                  // 0
    uint32      ModelId;                                    // 1
                                                            // 2-3,unused
    float       scale;                                      // 4
                                                            // 5-13,unused
};

struct CreatureModelDataEntry
{
    uint32 ModelID;                                         // 0

    float CollisionWidth;                                   // 15
    float CollisionHeight;                                  // 16
    //float mountHeight                                     // 17
    float minX;                                             // 18 m_geoBoxMinX
    float minY;                                             // 19 m_geoBoxMinY
    float minZ;                                             // 20 m_geoBoxMinZ
    float maxX;                                             // 21 m_geoBoxMaxX
    float maxY;                                             // 22 m_geoBoxMaxY
    float maxZ;                                             // 23 m_geoBoxMaxZ
};

struct CreatureFamilyEntry
{
    uint32    ID;                                           // 0
    float     minScale;                                     // 1
    uint32    minScaleLevel;                                // 2 0/1
    float     maxScale;                                     // 3
    uint32    maxScaleLevel;                                // 4 0/60
    uint32    skillLine[2];                                 // 5-6
    uint32    petFoodMask;                                  // 7
    char*     Name[16];                                     // 8-23
                                                            // 24 string flags, unused
                                                            // 25 icon, unused
};

struct CreatureSpellDataEntry
{
    uint32    ID;                                           // 0
    //uint32    spellId[4];                                 // 1-4 hunter pet learned spell (for later use)
};

struct DurabilityCostsEntry
{
    uint32    Itemlvl;                                      // 0
    uint32    multiplier[29];                               // 1-29
};

struct DurabilityQualityEntry
{
    uint32    Id;                                           // 0
    float     quality_mod;                                  // 1
};

struct EmotesEntry
{
    uint32  Id;                                             // 0
    //char*   Name;                                         // 1, internal name
    //uint32  AnimationId;                                  // 2, ref to animationData
    uint32  Flags;                                          // 3, bitmask, may be unit_flags
    uint32  EmoteType;                                      // 4, Can be 0, 1 or 2 (determine how emote are shown)
    uint32  UnitStandState;                                 // 5, uncomfirmed, may be enum UnitStandStateType
    //uint32  SoundId;                                      // 6, ref to soundEntries
};

struct EmotesTextEntry
{
    uint32    Id;
    uint32    textid;
};

struct FactionEntry
{
    uint32      ID;                                         // 0        m_ID
    int32       reputationListID;                           // 1        m_reputationIndex
    uint32      BaseRepRaceMask[4];                         // 2-5      m_reputationRaceMask
    uint32      BaseRepClassMask[4];                        // 6-9      m_reputationClassMask
    int32       BaseRepValue[4];                            // 10-13    m_reputationBase
    uint32      ReputationFlags[4];                         // 14-17    m_reputationFlags
    uint32      team;                                       // 18       m_parentFactionID
    char*       name[16];                                   // 19-34    m_name_lang
                                                            // 35 string flags, unused
    //char*     description[16];                            // 36-51    m_description_lang
                                                            // 52 string flags, unused

    // helpers

    int GetIndexFitTo(uint32 raceMask, uint32 classMask) const
    {
        for (int i = 0; i < 4; ++i)
        {
            if ((BaseRepRaceMask[i] == 0 || (BaseRepRaceMask[i] & raceMask)) &&
                (BaseRepClassMask[i] == 0 || (BaseRepClassMask[i] & classMask)))
                return i;
        }

        return -1;
    }
};

struct FactionTemplateEntry
{
    uint32      ID;                                         // 0
    uint32      faction;                                    // 1
    uint32      factionFlags;                               // 2 specific flags for that faction
    uint32      ourMask;                                    // 3 if mask set (see FactionMasks) then faction included in masked team
    uint32      friendlyMask;                               // 4 if mask set (see FactionMasks) then faction friendly to masked team
    uint32      hostileMask;                                // 5 if mask set (see FactionMasks) then faction hostile to masked team
    uint32      enemyFaction[4];                            // 6-9
    uint32      friendFaction[4];                           // 10-13
    //-------------------------------------------------------  end structure

    // helpers
    bool IsFriendlyTo(FactionTemplateEntry const& entry) const
    {
        if (ID == entry.ID)
            return true;

        if (entry.faction)
        {
            for (int i = 0; i < 4; ++i)
                if (enemyFaction[i] == entry.faction)
                    return false;
            for (int i = 0; i < 4; ++i)
                if (friendFaction[i] == entry.faction)
                    return true;
        }

        return (friendlyMask & entry.ourMask) || (ourMask & entry.friendlyMask);
    }

    bool IsHostileTo(FactionTemplateEntry const& entry) const
    {
        if (ID == entry.ID)
            return false;

        if (entry.faction)
        {
            for (int i = 0; i < 4; ++i)
                if (enemyFaction[i]  == entry.faction)
                    return true;
            for (int i = 0; i < 4; ++i)
                if (friendFaction[i] == entry.faction)
                    return false;
        }

        return (hostileMask & entry.ourMask) != 0;
    }

    bool IsHostileToPlayers() const { return (hostileMask & FACTION_MASK_PLAYER) !=0; }

    bool IsNeutralToAll() const
    {
        for (int i = 0; i < 4; ++i)
            if (enemyFaction[i] != 0)
                return false;
        return hostileMask == 0 && friendlyMask == 0;
    }

    bool IsContestedGuardFaction() const { return (factionFlags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD)!=0; }
};

struct GameObjectDisplayInfoEntry
{
    uint32 Displayid; // 0
    // char* filename; // 1 m_modelName
    // uint32 unknown2[10]; // 2-11 m_Sound
    float minX; // 12 m_geoBoxMinX (use first value as interact dist, mostly in hacks way)
    float minY; // 13 m_geoBoxMinY
    float minZ; // 14 m_geoBoxMinZ
    float maxX; // 15 m_geoBoxMaxX
    float maxY; // 16 m_geoBoxMaxY
    float maxZ; // 17 m_geoBoxMaxZ
};

struct GemPropertiesEntry
{
    uint32      ID;
    uint32      spellitemenchantement;
    uint32      color;
};

// All Gt* DBC store data for 100 levels, some by 100 per class/race
#define GT_MAX_LEVEL    100

struct GtCombatRatingsEntry
{
    float    ratio;
};

struct GtChanceToMeleeCritBaseEntry
{
    float    base;
};

struct GtChanceToMeleeCritEntry
{
    float    ratio;
};

struct GtChanceToSpellCritBaseEntry
{
    float    base;
};

struct GtChanceToSpellCritEntry
{
    float    ratio;
};

struct GtNPCManaCostScalerEntry
{
     float    ratio;
};

struct GtOCTRegenHPEntry
{
    float    ratio;
};

//struct GtOCTRegenMPEntry
//{
//    float    ratio;
//};

struct GtRegenHPPerSptEntry
{
    float    ratio;
};

struct GtRegenMPPerSptEntry
{
    float    ratio;
};

struct ItemEntry
{
   uint32 ID;
   uint32 DisplayId;
   uint32 InventoryType;
   uint32 Sheath;
};

struct ItemBagFamilyEntry
{
    uint32   ID;                                            // 0
    //char*     name[16]                                    // 1-16     m_name_lang
    //                                                      // 17       name flags
};

struct ItemDisplayInfoEntry
{
    uint32      ID;
    uint32      randomPropertyChance;
};

//struct ItemCondExtCostsEntry
//{
//    uint32      ID;
//    uint32      condExtendedCost;                           // ItemPrototype::CondExtendedCost
//    uint32      itemextendedcostentry;                      // ItemPrototype::ExtendedCost
//    uint32      arenaseason;                                // arena season number(1-4)
//};

struct ItemExtendedCostEntry
{
    uint32      ID;                                         // 0 extended-cost entry id
    uint32      reqhonorpoints;                             // 1 required honor points
    uint32      reqarenapoints;                             // 2 required arena points
    uint32      reqitem[5];                                 // 3-7 required item id
    uint32      reqitemcount[5];                            // 8-12 required count of 1st item
    uint32      reqpersonalarenarating;                     // 13 required personal arena rating
};

struct ItemRandomPropertiesEntry
{
    uint32    ID;                                           // 0
    //char*     internalName                                // 1   unused
    uint32    enchant_id[3];                                // 2-4
                                                            // 5-6 unused, 0 only values, reserved for additional enchantments?
    char*     nameSuffix[16];                               // 7-22, unused
                                                            // 23 nameSufix flags, unused
};

struct ItemRandomSuffixEntry
{
    uint32    ID;                                           // 0
    char*     name[16];                                     // 1-16 unused
                                                            // 17, name flags, unused
                                                            // 18  unused
    uint32    enchant_id[3];                                // 19-21
    uint32    prefix[3];                                    // 22-24
};

struct ItemSetEntry
{
    //uint32    id                                          // 0 item set ID
    char*     name[16];                                     // 1-16
                                                            // 17 string flags, unused
                                                            // 18-28 items from set, but not have all items listed, use ItemPrototype::ItemSet instead
                                                            // 29-34 unused
    uint32    spells[8];                                    // 35-42
    uint32    items_to_triggerspell[8];                     // 43-50
    uint32    required_skill_id;                            // 51
    uint32    required_skill_value;                         // 52
};

struct LFGDungeons
{
    uint32      id;                                         // 0 LFG id
    char*       name[16];                                   // 1-16 name
    //uint32      unk17;                                    // 17 integer ? unknown - same for all
    uint32      minLevel;                                   // 18 minimum lvl
    uint32      maxLevel;                                   // 19 maximum lvl
    uint32      type;                                       // 20 type (1 - normal dung, 2 - raid, 4 - zone (for quests probably), 5 - hero dung)
    uint32      faction;                                    // 21 faction (-1 - all, 0 - horde, 1 - ally)
    char*       icon;                                       // 22 icon or category (for example: AQTemple)
    uint32      expansion;                                  // 23 expansion (0 pretbc, 1 tbc)
};

#define MAX_LOCK_CASE 8

struct LockEntry
{
    uint32      ID;                                         // 0        m_ID
    uint32      Type[MAX_LOCK_CASE];                        // 1-5      m_Type
    uint32      Index[MAX_LOCK_CASE];                       // 9-16     m_Index
    uint32      Skill[MAX_LOCK_CASE];                       // 17-24    m_Skill
    //uint32      Action[MAX_LOCK_CASE];                    // 25-32    m_Action
};

struct MailTemplateEntry
{
    uint32      ID;                                         // 0        m_ID
    //char*       subject[16];                              // 1-16     m_subject_lang
                                                            // 17 string flags
    char*       content[16];                                // 18-33    m_body_lang
};

struct MapEntry
{
    uint32      MapID;                                      // 0
    //char*       internalname;                             // 1 unused
    uint32      map_type;                                   // 2
                                                            // 3 unused
    char*       name[16];                                   // 4-19
                                                            // 20 name flags, unused
                                                            // 21-23 unused (something PvPZone related - levels?)
                                                            // 24-26
    uint32      linked_zone;                                // 27 common zone for instance and continent map
    //char*     hordeIntro                                  // 28-43 text for PvP Zones
                                                            // 44 intro text flags
    //char*     allianceIntro                               // 45-60 text for PvP Zones
                                                            // 46 intro text flags
                                                            // 47-61 not used
    uint32      multimap_id;                                // 62
                                                            // 63-65 not used
    //chat*     unknownText1                                // 66-81 unknown empty text fields, possible normal Intro text.
                                                            // 82 text flags
    //chat*     heroicIntroText                             // 83-98 heroic mode requirement text
                                                            // 99 text flags
    //chat*     unknownText2                                // 100-115 unknown empty text fields
                                                            // 116 text flags
    int32       entrance_map;                               // 117 map_id of entrance map
    float       entrance_x;                                 // 118 entrance x coordinate (if exist single entry)
    float       entrance_y;                                 // 119 entrance y coordinate (if exist single entry)
    uint32 resetTimeRaid;                                   // 120
    uint32 resetTimeHeroic;                                 // 121
                                                            // 122-123
    uint32      addon;                                      // 124 (0-original maps,1-tbc addon)

    // Helpers
    uint32 Expansion() const { return addon; }


    bool IsDungeon() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID; }
    bool Instanceable() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID || map_type == MAP_BATTLEGROUND || map_type == MAP_ARENA; }
    bool IsRaid() const { return map_type == MAP_RAID; }
    bool IsBattleGround() const { return map_type == MAP_BATTLEGROUND; }
    bool IsBattleArena() const { return map_type == MAP_ARENA; }
    bool IsBattleGroundOrArena() const { return map_type == MAP_BATTLEGROUND || map_type == MAP_ARENA; }
    bool SupportsHeroicMode() const { return resetTimeHeroic/* && !resetTimeRaid*/; }
    bool HasResetTime() const { return resetTimeHeroic || resetTimeRaid; }

    bool IsMountAllowed() const
    {
        return !IsDungeon() ||
            MapID==209 || MapID==269 || MapID==309 ||       // TanarisInstance, CavernsOfTime, Zul'gurub
            MapID==509 || MapID==534 || MapID==560 ||       // AhnQiraj, HyjalPast, HillsbradPast
            MapID==568 || MapID==580 || MapID==564;         // ZulAman, Sunwell Plateau, BT
    }

    bool IsContinent() const
    {
        return MapID == 0 || MapID == 1 || MapID == 530;
    }
};

struct QuestSortEntry
{
    uint32      id;                                         // 0, sort id
    //char*       name[16];                                 // 1-16, unused
                                                            // 17 name flags, unused
};

struct RandomPropertiesPointsEntry
{
    //uint32  Id;                                           // 0 hidden key
    uint32    itemLevel;                                    // 1
    uint32    EpicPropertiesPoints[5];                      // 2-6
    uint32    RarePropertiesPoints[5];                      // 7-11
    uint32    UncommonPropertiesPoints[5];                  // 12-16
};

//struct SkillLineCategoryEntry{
//    uint32    id;                                           // 0 hidden key
//    char*     name[16];                                     // 1 - 17 Category name
//                                                                  // 18 string flag
//    uint32    displayOrder;                                 // Display order in character tab
//};

//struct SkillRaceClassInfoEntry{
//    uint32    id;                                           // 0
//    uint32    skillId;                                      // 1 present some refrences to unknown skill
//    uint32    raceMask;                                     // 2
//    uint32    classMask;                                    // 3
//    uint32    flags;                                        // 4 mask for some thing
//    uint32    reqLevel;                                     // 5
//    uint32    skillTierId;                                  // 6
//    uint32    skillCostID;                                  // 7
//};

//struct SkillTiersEntry{
//    uint32    id;                                           // 0
//    uint32    skillValue[16];                               // 1-17 unknown possibly add value on learn?
//    uint32    maxSkillValue[16];                            // Max value for rank
//};

struct SkillLineEntry
{
    uint32    id;                                           // 0
    uint32    categoryId;                                   // 1 (index from SkillLineCategory.dbc)
    //uint32    skillCostID;                                // 2 not used
    char*     name[16];                                     // 3-18
                                                            // 19 string flags, not used
    //char*     description[16];                            // 20-35, not used
                                                            // 36 string flags, not used
    uint32    spellIcon;                                    // 37
};

struct SkillLineAbilityEntry
{
    uint32    id;                                           // 0, INDEX
    uint32    skillId;                                      // 1
    uint32    spellId;                                      // 2
    uint32    racemask;                                     // 3
    uint32    classmask;                                    // 4
    //uint32    racemaskNot;                                // 5 always 0 in 2.4.2
    //uint32    classmaskNot;                               // 6 always 0 in 2.4.2
    uint32    req_skill_value;                              // 7 for trade skill.not for training.
    uint32    forward_spellid;                              // 8
    uint32    learnOnGetSkill;                              // 9 can be 1 or 2 for spells learned on get skill
    uint32    max_value;                                    // 10
    uint32    min_value;                                    // 11
                                                            // 12-13, unknown, always 0
    uint32    reqtrainpoints;                               // 14
};

struct SoundEntriesEntry
{
    uint32    Id;                                           // 0, sound id
    //uint32    Type;                                       // 1, sound type (10 generally for creature, etc)
    //char*     InternalName;                               // 2, internal name, for use in lookup command for example
    //char*     FileName[10];                               // 3-12, file names
    //uint32    Unk13[10];                                  // 13-22, linked with file names?
    //char*     Path;                                       // 23
                                                            // 24-28, unknown
};

#define MAX_SPELL_REAGENTS 8
#define MAX_SPELL_TOTEMS 2
#define MAX_SPELL_TOTEM_CATEGORIES 2

struct SpellEntry
{
    uint32    Id;                                           // 0 normally counted from 0 field (but some tools start counting from 1, check this before tool use for data view!)
    uint32    Category;                                     // 1
    //uint32     castUI                                     // 2 not used
    uint32    Dispel;                                       // 3
    uint32    Mechanic;                                     // 4
    uint32    Attributes;                                   // 5
    uint32    AttributesEx;                                 // 6
    uint32    AttributesEx2;                                // 7
    uint32    AttributesEx3;                                // 8
    uint32    AttributesEx4;                                // 9
    uint32    AttributesEx5;                                // 10
    uint32    AttributesEx6;                                // 11
    uint32    Stances;                                      // 12
    uint32    StancesNot;                                   // 13
    uint32    Targets;                                      // 14
    uint32    TargetCreatureType;                           // 15
    uint32    RequiresSpellFocus;                           // 16
    uint32    FacingCasterFlags;                            // 17
    uint32    CasterAuraState;                              // 18
    uint32    TargetAuraState;                              // 19
    uint32    CasterAuraStateNot;                           // 20
    uint32    TargetAuraStateNot;                           // 21
    uint32    CastingTimeIndex;                             // 22
    uint32    RecoveryTime;                                 // 23
    uint32    CategoryRecoveryTime;                         // 24
    uint32    InterruptFlags;                               // 25
    uint32    AuraInterruptFlags;                           // 26
    uint32    ChannelInterruptFlags;                        // 27
    uint32    procFlags;                                    // 28
    uint32    procChance;                                   // 29
    uint32    procCharges;                                  // 30
    uint32    maxLevel;                                     // 31
    uint32    baseLevel;                                    // 32
    uint32    spellLevel;                                   // 33
    uint32    DurationIndex;                                // 34
    uint32    powerType;                                    // 35
    uint32    manaCost;                                     // 36
    uint32    manaCostPerlevel;                             // 37
    uint32    manaPerSecond;                                // 38
    uint32    manaPerSecondPerLevel;                        // 39
    uint32    rangeIndex;                                   // 40
    float     speed;                                        // 41
    //uint32    modalNextSpell;                             // 42
    uint32    StackAmount;                                  // 43
    uint32    Totem[2];                                     // 44-45
    int32     Reagent[8];                                   // 46-53
    uint32    ReagentCount[8];                              // 54-61
    int32     EquippedItemClass;                            // 62 (value)
    int32     EquippedItemSubClassMask;                     // 63 (mask)
    int32     EquippedItemInventoryTypeMask;                // 64 (mask)
    uint32    Effect[3];                                    // 65-67
    int32     EffectDieSides[3];                            // 68-70
    uint32    EffectBaseDice[3];                            // 71-73
    float     EffectDicePerLevel[3];                        // 74-76
    float     EffectRealPointsPerLevel[3];                  // 77-79
    int32     EffectBasePoints[3];                          // 80-82 (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
    uint32    EffectMechanic[3];                            // 83-85
    uint32    EffectImplicitTargetA[3];                     // 86-88
    uint32    EffectImplicitTargetB[3];                     // 89-91
    uint32    EffectRadiusIndex[3];                         // 92-94 - spellradius.dbc
    uint32    EffectApplyAuraName[3];                       // 95-97
    uint32    EffectAmplitude[3];                           // 98-100
    float     EffectMultipleValue[3];                       // 101-103
    uint32    EffectChainTarget[3];                         // 104-106
    uint32    EffectItemType[3];                            // 107-109
    int32     EffectMiscValue[3];                           // 110-112
    int32     EffectMiscValueB[3];                          // 113-115
    uint32    EffectTriggerSpell[3];                        // 116-118
    float     EffectPointsPerComboPoint[3];                 // 119-121
    uint32    SpellVisual;                                  // 122
    // 123 not used
    uint32    SpellIconID;                                  // 124
    uint32    activeIconID;                                 // 125
    //uint32    spellPriority;                              // 126
    char* SpellName[16];                                // 127-142
    //uint32    SpellNameFlag;                              // 143
    char* Rank[16];                                     // 144-159
    //uint32    RankFlags;                                  // 160
    //char*     Description[16];                            // 161-176 not used
    //uint32    DescriptionFlags;                           // 177     not used
    //char*     ToolTip[16];                                // 178-193 not used
    uint32    ToolTipFlags;                               // 194 not used
    uint32    ManaCostPercentage;                           // 195
    uint32    StartRecoveryCategory;                        // 196
    uint32    StartRecoveryTime;                            // 197
    uint32    MaxTargetLevel;                               // 198
    uint32    SpellFamilyName;                              // 199
    uint64    SpellFamilyFlags;                             // 200+201
    uint32    MaxAffectedTargets;                           // 202
    uint32    DmgClass;                                     // 203 defenseType
    uint32    PreventionType;                               // 204
    //uint32    StanceBarOrder;                             // 205 not used
    float     DmgMultiplier[3];                             // 206-208
    uint32    AttributesCu;                                 // 209 not used, and 0 in 2.4.2
    //uint32    MinReputation;                              // 210 not used, and 0 in 2.4.2
    //uint32    RequiredAuraVision;                         // 211 not used
    uint32    TotemCategory[2];                             // 212-213
    uint32    AreaId;                                       // 214
    uint32    SchoolMask;                                   // 215 school mask

    // helpers
    int32 CalculateSimpleValue(uint8 eff) const { return EffectBasePoints[eff]+int32(EffectBaseDice[eff]); }

    bool HasEffect(uint32 eff) const
    {
        for (uint8 i = 0; i < 3; ++i)
            if (Effect[i] == eff)
                return true;

        return false;
    }

    bool HasApplyAura(uint32 aur) const
    {
        for (uint8 i = 0; i < 3; ++i)
            if (EffectApplyAuraName[i] == aur)
                return true;

        return false;
    }

    bool IsDestTargetEffect(uint8 eff) const
    {
        switch (EffectImplicitTargetA[eff])
        {
            case TARGET_DST_HOME:
            case TARGET_DST_DB:
            case TARGET_DST_CASTER:
            case TARGET_DEST_DYNOBJ_ENEMY:
            case TARGET_DEST_DYNOBJ_ALLY:
            case TARGET_UNIT_AREA_ALLY_DST:
            case TARGET_UNIT_AREA_PARTY_DST:
            case TARGET_DEST_CASTER_RANDOM_UNKNOWN:
            case TARGET_DEST_CASTER_FRONT_LEFT:
            case TARGET_DEST_CASTER_BACK_LEFT:
            case TARGET_DEST_CASTER_BACK_RIGHT:
            case TARGET_DEST_CASTER_FRONT_RIGHT:
            case TARGET_DST_NEARBY_ENTRY:
            case TARGET_DEST_CASTER_FRONT:
            case TARGET_DEST_CASTER_BACK:
            case TARGET_DEST_CASTER_RIGHT:
            case TARGET_DEST_CASTER_LEFT:
            case TARGET_DST_TARGET_ENEMY:
            case TARGET_DEST_CASTER_FRONT_LEAP:
            case TARGET_DEST_TARGET_ANY:
            case TARGET_DEST_TARGET_FRONT:
            case TARGET_DEST_TARGET_BACK:
            case TARGET_DEST_TARGET_RIGHT:
            case TARGET_DEST_TARGET_LEFT:
            case TARGET_DEST_TARGET_FRONT_LEFT:
            case TARGET_DEST_TARGET_BACK_LEFT:
            case TARGET_DEST_TARGET_BACK_RIGHT:
            case TARGET_DEST_TARGET_FRONT_RIGHT:
            case TARGET_DEST_CASTER_RANDOM:
            case TARGET_DEST_CASTER_RADIUS:
            case TARGET_DEST_TARGET_RANDOM:
            case TARGET_DEST_TARGET_RADIUS:
            case TARGET_DEST_CHANNEL:
            case TARGET_DEST_DEST_FRONT:
            case TARGET_DEST_DEST_BACK:
            case TARGET_DEST_DEST_RIGHT:
            case TARGET_DEST_DEST_LEFT:
            case TARGET_DEST_DEST_FRONT_LEFT:
            case TARGET_DEST_DEST_BACK_LEFT:
            case TARGET_DEST_DEST_BACK_RIGHT:
            case TARGET_DEST_DEST_FRONT_RIGHT:
            case TARGET_DEST_DEST_RANDOM:
            case TARGET_DEST_DEST:
            case TARGET_DEST_DYNOBJ_NONE:
            case TARGET_DEST_TRAJ:
                return true;
        }

        return false;
    }

    bool NeedFillTargetMapForTargets(uint8 eff) const
    {
        // check combinations A <> B of implicit targets
        switch (EffectImplicitTargetA[eff])
        {
            case TARGET_UNIT_NEARBY_ENTRY:
            case TARGET_DST_NEARBY_ENTRY:
                return true;
            default:
                return false;
        }
        return false;
    }

};

typedef std::set<uint32> PetFamilySpellsSet;
typedef std::map<uint32,PetFamilySpellsSet > PetFamilySpellsStore;

struct SpellCastTimesEntry
{
    uint32    ID;                                           // 0
    int32     CastTime;                                     // 1
    //float     CastTimePerLevel;                           // 2 unsure / per skill?
    //int32     MinCastTime;                                // 3 unsure
};

struct SpellFocusObjectEntry
{
    uint32    ID;                                           // 0
    //char*     Name[16];                                   // 1-15 unused
                                                            // 16 string flags, unused
};

// stored in SQL table
struct SpellThreatEntry
{
    uint32      spellId;
    int32       threat;
    int32       multiplier;
};

struct SpellRadiusEntry
{
    uint32    ID;
    float     radiusHostile;
    float     radiusFriend;
};

enum SpellRangeType
{
    SPELL_RANGE_DEFAULT = 0,
    SPELL_RANGE_MELEE = 1,     //melee
    SPELL_RANGE_RANGED = 2,     //hunter range and ranged weapon
};

struct SpellRangeEntry
{
    uint32    ID;
    float     minRange;
    float     maxRange;
    SpellRangeType    type;
};

struct SpellShapeshiftEntry
{
    uint32 ID;                                              // 0
    //uint32 buttonPosition;                                // 1 unused
    //char*  Name[16];                                      // 2-17 unused
    //uint32 NameFlags;                                     // 18 unused
    uint32 flags1;                                          // 19
    int32  creatureType;                                    // 20 <=0 humanoid, other normal creature types
    //uint32 unk1;                                          // 21 unused
    uint32 attackSpeed;                                     // 22
    //uint32 modelID;                                       // 23 unused, alliance modelid (where horde case?)
    //uint32 unk2;                                          // 24 unused
    //uint32 unk3;                                          // 25 unused
    //uint32 unk4;                                          // 26 unused
    //uint32 unk5;                                          // 27 unused
    //uint32 unk6;                                          // 28 unused
    //uint32 unk7;                                          // 29 unused
    //uint32 unk8;                                          // 30 unused
    //uint32 unk9;                                          // 31 unused
    //uint32 unk10;                                         // 32 unused
    //uint32 unk11;                                         // 33 unused
    //uint32 unk12;                                         // 34 unused
};

struct SpellDurationEntry
{
    uint32    ID;
    int32     Duration[3];
};

struct SpellItemEnchantmentEntry
{
    uint32      ID;                                         // 0
    uint32      type[3];                                    // 1-3
    uint32      amount[3];                                  // 4-6
    //uint32    amount2[3]                                  // 7-9 always same as similar `amount` value
    uint32      spellid[3];                                 // 10-12
    char*       description[16];                            // 13-29
                                                            // 30 description flags
    uint32      aura_id;                                    // 31
    uint32      slot;                                       // 32
    uint32      GemID;                                      // 33
    uint32      EnchantmentCondition;                       // 34
};

struct SpellItemEnchantmentConditionEntry
{
    uint32  ID;
    uint8   Color[5];
    uint8   Comparator[5];
    uint8   CompareColor[5];
    uint32  Value[5];
};

struct StableSlotPricesEntry
{
    uint32 Slot;
    uint32 Price;
};

/* unused currently
struct SummonPropertiesEntry
{
    uint32  Id;                                             // 0
    uint32  Group;                                          // 1, enum SummonPropGroup,  0 - can't be controlled?, 1 - something guardian?, 2 - pet?, 3 - something controllable?, 4 - taxi/mount?
    uint32  Unk2;                                           // 2,                        14 rows > 0
    uint32  Type;                                           // 3, enum SummonPropType
    uint32  Slot;                                           // 4,                        0-6
    uint32  Flags;                                          // 5, enum SummonPropFlags
};
*/

struct TalentEntry
{
    uint32    TalentID;                                     // 0
    uint32    TalentTab;                                    // 1 index in TalentTab.dbc (TalentTabEntry)
    uint32    Row;                                          // 2
    uint32    Col;                                          // 3
    uint32    RankID[5];                                    // 4-8
                                                            // 9-12 not used, always 0, maybe not used high ranks
    uint32    DependsOn;                                    // 13 index in Talent.dbc (TalentEntry)
                                                            // 14-15 not used
    uint32    DependsOnRank;                                // 16
                                                            // 17-19 not used
    uint32    DependsOnSpell;                               // 20 req.spell
};

struct TalentTabEntry
{
    uint32    TalentTabID;                                  // 0
    //char*   name[16];                                     // 1-16, unused
    //uint32  nameFlags;                                    // 17, unused
    //unit32  spellicon;                                    // 18
                                                            // 19 not used
    uint32    ClassMask;                                    // 20
    uint32    tabpage;                                      // 21
    //char*   internalname;                                 // 22
};

struct TaxiNodesEntry
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    //char*     name[16];                                   // 5-21
                                                            // 22 string flags, unused
    uint32    horde_mount_type;                             // 23
    uint32    alliance_mount_type;                          // 24
};

struct TaxiPathEntry
{
    uint32    ID;
    uint32    from;
    uint32    to;
    uint32    price;
};

struct TaxiPathNodeEntry
{
    uint32    path;
    uint32    index;
    uint32    mapid;
    float     x;
    float     y;
    float     z;
    uint32    actionFlag;
    uint32    delay;

    uint32    arrivalEventID;
    uint32    departureEventID;
};

struct TotemCategoryEntry
{
    uint32    ID;                                           // 0
    //char*   name[16];                                     // 1-16
                                                            // 17 string flags, unused
    uint32    categoryType;                                 // 18 (one for specialization)
    uint32    categoryMask;                                 // 19 (compatibility mask for same type: different for totems, compatible from high to low for rods)
};

struct WMOAreaTableEntry
{
    uint32 Id;                                              // 0 index
    int32 rootId;                                           // 1 used in root WMO
    int32 adtId;                                            // 2 used in adt file
    int32 groupId;                                          // 3 used in group WMO
    //uint32 field4;
    //uint32 field5;
    //uint32 field6;
    //uint32 field7;
    //uint32 field8;
    uint32 Flags;                                           // 9 used for indoor/outdoor determination
    uint32 areaId;                                          // 10 link to AreaTableEntry.ID
    //char *Name[16];
    //uint32 nameFlags;
};

struct WorldMapAreaEntry
{
    //uint32    ID;                                         // 0
    uint32    map_id;                                       // 1
    uint32    area_id;                                      // 2 index (continent 0 areas ignored)
    //char*   internal_name                                 // 3
    float     y1;                                           // 4
    float     y2;                                           // 5
    float     x1;                                           // 6
    float     x2;                                           // 7
    int32   virtual_map_id;                                 // 8 -1 (map_id have correct map) other: virtual map where zone show (map_id - where zone in fact internally)
};

struct WorldSafeLocsEntry
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    //char*   name[16]                                      // 5-20 name, unused
                                                            // 21 name flags, unused
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

// Structures not used for casting to loaded DBC data and not required then packing
struct TalentSpellPos
{
    TalentSpellPos() : talent_id(0), rank(0) {}
    TalentSpellPos(uint16 _talent_id, uint8 _rank) : talent_id(_talent_id), rank(_rank) {}

    uint16 talent_id;
    uint8  rank;
};

typedef std::map<uint32,TalentSpellPos> TalentSpellPosMap;

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination() : ID(0),price(0) {}
    TaxiPathBySourceAndDestination(uint32 _id,uint32 _price) : ID(_id),price(_price) {}

    uint32    ID;
    uint32    price;
};
typedef std::map<uint32,TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32,TaxiPathSetForSource> TaxiPathSetBySource;

struct TaxiPathNodePtr
{
    TaxiPathNodePtr() : i_ptr(NULL) {}
    TaxiPathNodePtr(TaxiPathNodeEntry const* ptr) : i_ptr(ptr) {}

    TaxiPathNodeEntry const* i_ptr;

    operator TaxiPathNodeEntry const& () const { return *i_ptr; }
};

typedef Path<TaxiPathNodePtr,TaxiPathNodeEntry const> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

#define TaxiMaskSize 16
typedef uint32 TaxiMask[TaxiMaskSize];
#endif
