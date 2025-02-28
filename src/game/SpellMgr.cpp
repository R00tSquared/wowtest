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

// #include "Unit.h"
#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
//#include "ProgressBar.h"
#include "DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"
#include "CreatureAI.h"
#include "BattleGroundMgr.h"

bool IsAreaEffectTarget[TOTAL_SPELL_TARGETS];

std::initializer_list<int> all_Flamestrike_ranks = { 2120, 2121, 8422, 8423, 10215, 10216, 27086 };


SpellMgr::SpellMgr()
{
    for (int i = 0; i < TOTAL_SPELL_EFFECTS; ++i)
    {
        switch (i)
        {
            case SPELL_EFFECT_PERSISTENT_AREA_AURA: //27
            case SPELL_EFFECT_SUMMON:               //28
            case SPELL_EFFECT_TRIGGER_MISSILE:      //32
            case SPELL_EFFECT_SUMMON_WILD:          //41
            case SPELL_EFFECT_SUMMON_GUARDIAN:      //42
            case SPELL_EFFECT_TRANS_DOOR:           //50 summon object
            case SPELL_EFFECT_SUMMON_PET:           //56
            case SPELL_EFFECT_ADD_FARSIGHT:         //72
            case SPELL_EFFECT_SUMMON_POSSESSED:     //73
            case SPELL_EFFECT_SUMMON_TOTEM:         //74
            case SPELL_EFFECT_SUMMON_OBJECT_WILD:   //76
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT1:   //87
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT2:   //88
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT3:   //89
            case SPELL_EFFECT_SUMMON_TOTEM_SLOT4:   //90
            case SPELL_EFFECT_SUMMON_CRITTER:       //97
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:  //104
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:  //105
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:  //106
            case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:  //107
            case SPELL_EFFECT_RESURRECT_PET:      //109
            case SPELL_EFFECT_SUMMON_DEMON:         //112
            case SPELL_EFFECT_TRIGGER_SPELL_2:      //151 ritual of summon
                EffectTargetType[i] = SPELL_REQUIRE_DEST;
                break;
            case SPELL_EFFECT_PARRY: // 0
            case SPELL_EFFECT_BLOCK: // 0
            case SPELL_EFFECT_SKILL: // always with dummy 3 as A
            //case SPELL_EFFECT_LEARN_SPELL: // 0 may be 5 pet
            case SPELL_EFFECT_TRADE_SKILL: // 0 or 1
            case SPELL_EFFECT_PROFICIENCY: // 0
                EffectTargetType[i] = SPELL_REQUIRE_NONE;
                break;
            case SPELL_EFFECT_ENCHANT_ITEM:
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            case SPELL_EFFECT_DISENCHANT:
            case SPELL_EFFECT_FEED_PET:
            case SPELL_EFFECT_PROSPECTING:
                EffectTargetType[i] = SPELL_REQUIRE_ITEM;
                break;
            //caster must be pushed otherwise no sound
            case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
            case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
            case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
            case SPELL_EFFECT_APPLY_AREA_AURA_PET:
            case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
                EffectTargetType[i] = SPELL_REQUIRE_CASTER;
                break;
            default:
                EffectTargetType[i] = SPELL_REQUIRE_UNIT;
                break;
        }
    }

    for (int i = 0; i < TOTAL_SPELL_TARGETS; ++i)
    {
        switch (i)
        {
            case TARGET_UNIT_CASTER:
            case TARGET_UNIT_CASTER_FISHING:
            case TARGET_UNIT_MASTER:
            case TARGET_UNIT_PET:
            case TARGET_UNIT_PARTY_CASTER:
            case TARGET_UNIT_RAID_CASTER:
                SpellTargetType[i] = TARGET_TYPE_UNIT_CASTER;
                break;
            case TARGET_UNIT_MINIPET:
            case TARGET_UNIT_TARGET_ALLY:
            case TARGET_UNIT_TARGET_RAID:
            case TARGET_UNIT_TARGET_ANY:
            case TARGET_UNIT_TARGET_ENEMY:
            case TARGET_UNIT_TARGET_PARTY:
            case TARGET_UNIT_PARTY_TARGET:
            case TARGET_UNIT_CLASS_TARGET:
            case TARGET_UNIT_CHAINHEAL:
                SpellTargetType[i] = TARGET_TYPE_UNIT_TARGET;
                break;
            case TARGET_UNIT_NEARBY_ENEMY:
            case TARGET_UNIT_NEARBY_ALLY:
            case TARGET_UNIT_NEARBY_ALLY_UNK:
            case TARGET_UNIT_NEARBY_ENTRY:
            case TARGET_UNIT_NEARBY_RAID:
            case TARGET_OBJECT_USE:
                SpellTargetType[i] = TARGET_TYPE_UNIT_NEARBY;
                break;
            case TARGET_UNIT_AREA_ENEMY_SRC:
            case TARGET_UNIT_AREA_ALLY_SRC:
            case TARGET_UNIT_AREA_ENTRY_SRC:
            case TARGET_UNIT_AREA_PARTY_SRC:
            case TARGET_OBJECT_AREA_SRC:
                SpellTargetType[i] = TARGET_TYPE_AREA_SRC;
                break;
            case TARGET_UNIT_AREA_ENEMY_DST:
            case TARGET_UNIT_AREA_ALLY_DST:
            case TARGET_UNIT_AREA_ENTRY_DST:
            case TARGET_UNIT_AREA_PARTY_DST:
            case TARGET_OBJECT_AREA_DST:
                SpellTargetType[i] = TARGET_TYPE_AREA_DST;
                break;
            case TARGET_UNIT_CONE_ENEMY:
            case TARGET_UNIT_CONE_ALLY:
            case TARGET_UNIT_CONE_ENTRY:
            case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
                SpellTargetType[i] = TARGET_TYPE_AREA_CONE;
                break;
            case TARGET_DST_CASTER:
            case TARGET_SRC_CASTER:
            case TARGET_MINION:
            case TARGET_DEST_CASTER_FRONT_LEAP:
            case TARGET_DEST_CASTER_FRONT:
            case TARGET_DEST_CASTER_BACK:
            case TARGET_DEST_CASTER_RIGHT:
            case TARGET_DEST_CASTER_LEFT:
            case TARGET_DEST_CASTER_FRONT_LEFT:
            case TARGET_DEST_CASTER_BACK_LEFT:
            case TARGET_DEST_CASTER_BACK_RIGHT:
            case TARGET_DEST_CASTER_FRONT_RIGHT:
            case TARGET_DEST_CASTER_RANDOM:
            case TARGET_DEST_CASTER_RADIUS:
                SpellTargetType[i] = TARGET_TYPE_DEST_CASTER;
                break;
            case TARGET_DST_TARGET_ENEMY:
            case TARGET_DEST_TARGET_ANY:
            case TARGET_DEST_TARGET_FRONT:
            case TARGET_DEST_TARGET_BACK:
            case TARGET_DEST_TARGET_RIGHT:
            case TARGET_DEST_TARGET_LEFT:
            case TARGET_DEST_TARGET_FRONT_LEFT:
            case TARGET_DEST_TARGET_BACK_LEFT:
            case TARGET_DEST_TARGET_BACK_RIGHT:
            case TARGET_DEST_TARGET_FRONT_RIGHT:
            case TARGET_DEST_TARGET_RANDOM:
            case TARGET_DEST_TARGET_RADIUS:
                SpellTargetType[i] = TARGET_TYPE_DEST_TARGET;
                break;
            case TARGET_DEST_DYNOBJ_ENEMY:
            case TARGET_DEST_DYNOBJ_ALLY:
            case TARGET_DEST_DYNOBJ_NONE:
            case TARGET_DEST_DEST:
            case TARGET_DEST_TRAJ:
            case TARGET_DEST_DEST_FRONT_LEFT:
            case TARGET_DEST_DEST_BACK_LEFT:
            case TARGET_DEST_DEST_BACK_RIGHT:
            case TARGET_DEST_DEST_FRONT_RIGHT:
            case TARGET_DEST_DEST_FRONT:
            case TARGET_DEST_DEST_BACK:
            case TARGET_DEST_DEST_RIGHT:
            case TARGET_DEST_DEST_LEFT:
            case TARGET_DEST_DEST_RANDOM:
                SpellTargetType[i] = TARGET_TYPE_DEST_DEST;
                break;
            case TARGET_DST_DB:
            case TARGET_DST_HOME:
            case TARGET_DST_NEARBY_ENTRY:
                SpellTargetType[i] = TARGET_TYPE_DEST_SPECIAL;
                break;
            case TARGET_UNIT_CHANNEL:
            case TARGET_DEST_CHANNEL:
                SpellTargetType[i] = TARGET_TYPE_CHANNEL;
                break;
            default:
                SpellTargetType[i] = TARGET_TYPE_DEFAULT;
        }
    }

    for (int i = 0; i < TOTAL_SPELL_TARGETS; ++i)
    {
        switch (i)
        {
            case TARGET_UNIT_AREA_ENEMY_DST:
            case TARGET_UNIT_AREA_ENEMY_SRC:
            case TARGET_UNIT_AREA_ALLY_DST:
            case TARGET_UNIT_AREA_ALLY_SRC:
            case TARGET_UNIT_AREA_ENTRY_DST:
            case TARGET_UNIT_AREA_ENTRY_SRC:
            case TARGET_UNIT_AREA_PARTY_DST:
            case TARGET_UNIT_AREA_PARTY_SRC:
            case TARGET_UNIT_PARTY_TARGET:
            case TARGET_UNIT_PARTY_CASTER:
            case TARGET_UNIT_CONE_ENEMY:
            case TARGET_UNIT_CONE_ALLY:
            case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
            case TARGET_UNIT_RAID_CASTER:
                IsAreaEffectTarget[i] = true;
                break;
            default:
                IsAreaEffectTarget[i] = false;
                break;
        }
    }
}

SpellMgr::~SpellMgr()
{
}

int32 SpellMgr::GetSpellDuration(SpellEntry const *spellInfo)
{
    if (!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if (!du)
        return 0;
    return (du->Duration[0] == -1) ? -1 : abs(du->Duration[0]);
}

int32 SpellMgr::GetSpellMaxDuration(SpellEntry const *spellInfo)
{
    if (!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if (!du)
        return 0;
    return (du->Duration[2] == -1) ? -1 : abs(du->Duration[2]);
}

int32 SpellMgr::GetSpellBaseCastTime(SpellEntry const *spellInfo)
{
    SpellCastTimesEntry const *spellCastTimeEntry = sSpellCastTimesStore.LookupEntry(spellInfo->CastingTimeIndex);
    if (!spellCastTimeEntry)
        return 0;
    return spellCastTimeEntry->CastTime;
}

uint32 SpellMgr::GetSpellBaseCastTimeNotNegative(SpellEntry const *spellInfo)
{
    SpellCastTimesEntry const *spellCastTimeEntry = sSpellCastTimesStore.LookupEntry(spellInfo->CastingTimeIndex);
    if (!spellCastTimeEntry)
        return 0;
    return spellCastTimeEntry->CastTime > 0 ? spellCastTimeEntry->CastTime : 0;
}

uint32 SpellMgr::GetSpellCastTime(SpellEntry const* spellInfo, Spell const* spell)
{
    int32 castTime = SpellMgr::GetSpellBaseCastTime(spellInfo);

    if (spellInfo->Attributes & SPELL_ATTR_RANGED && (!spell || !(spell->IsAutoRepeatStart())))
        castTime += 500;

    // if castTime == 0 no sense to apply modifiers - except ranged ability spells
    if (castTime <= 0) // not found or instant - return instant
        return 0;

    if (spell)
    {
        if (Player* modOwner = spell->GetCaster()->GetSpellModOwner())
            modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CASTING_TIME, castTime, spell);

        if (!(spellInfo->Attributes & (SPELL_ATTR_ABILITY|SPELL_ATTR_TRADESPELL))) // no bonus for spells such as fishing, first aid, trade spells
            castTime = int32(castTime * spell->GetCaster()->GetFloatValue(UNIT_MOD_CAST_SPEED));
        else
        {
            if (spell->IsRangedSpell() && !spell->IsAutoRepeatStart() && !(spell->GetCaster()->GetClassMask() & CLASSMASK_WAND_USERS))
                castTime = int32(castTime * spell->GetCaster()->m_modAttackSpeedPct[RANGED_ATTACK]);
        }
    }

    return (castTime > 0) ? uint32(castTime) : 0;
}

bool SpellMgr::IsPassiveSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return false;
    return (spellInfo->Attributes & SPELL_ATTR_PASSIVE) != 0;
}

bool SpellMgr::IsPassiveSpell(SpellEntry const *spellInfo)
{
    return (spellInfo->Attributes & SPELL_ATTR_PASSIVE) != 0;
}

//void SpellMgr::ApplySpellThreatModifiers(SpellEntry const *spellInfo, float &threat)
//{
//    // moonwell disabled
//
//    if (!spellInfo)
//        return;
//
//    if (spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && spellInfo->SpellFamilyFlags & 0x100LL) // Searing Pain
//        threat *= 2.0f;
//
//    else if (spellInfo->SpellFamilyName == SPELLFAMILY_SHAMAN && spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_SHAMAN_FROST_SHOCK)
//        threat *= 2.0f;
//
//    else if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo->SpellFamilyFlags & 0x4000000000LL) // Holy shield
//        threat *= 1.35f;
//
//    else if (spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags & 0x10000000000LL) // Lacerate
//        threat *= 0.20f;
//
//    else if (spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && spellInfo->SpellFamilyFlags & 0x8400000LL)    // Holy Nova
//        threat = 1.0f;
//
//    else if (spellInfo->Id == 33619) // Reflective shield
//        threat = 1.0f;
//
//    else if (spellInfo->Id == 31616) // Nature's Guardian - shaman talent
//        threat *= 0.9f;
//
//    else if (spellInfo->Id == 33763) // Lifebloom HOT, Last instant heal counts as zero threat (at least should count as zero)
//        threat *= 0.47f;
//
//    else if (spellInfo->Id == 32546) // Binging heal
//        threat *= 0.5f;
//
//    else if (spellInfo->Id == 20647) // Execute
//        threat *= 1.25f;
//
//    else if (spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && spellInfo->SpellFamilyFlags & 0x1000000000LL) // Chastise
//        threat *= 0.5f;
//
//    else if (spellInfo->Id == 37661) // Lightning Capacitor
//        threat *= 0.5f;
//
//    else if (spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo->SpellFamilyFlags & 0x80LL) // Thunder Clap
//        threat *= 1.75f;
//}

uint32 SpellMgr::CalculatePowerCost(SpellEntry const * spellInfo, Unit const * caster, SpellSchoolMask schoolMask, Spell* spell, bool finalUse)
{
    // Spell drain all exist power on cast (Only paladin lay of Hands)
    if (spellInfo->AttributesEx & SPELL_ATTR_EX_DRAIN_ALL_POWER)
    {
        // If power type - health drain all
        if (spellInfo->powerType == POWER_HEALTH)
            return caster->GetHealth();
        // Else drain all power
        if (spellInfo->powerType < MAX_POWERS)
            return caster->GetPower(Powers(spellInfo->powerType));
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell::CalculateManaCost: Unknown power type '%d' in spell %d", spellInfo->powerType, spellInfo->Id);
        return 0;
    }

    // Base powerCost
    int32 powerCost = spellInfo->manaCost;
    // PCT cost from total amount
    if (spellInfo->ManaCostPercentage)
    {
        switch (spellInfo->powerType)
        {
            // health as power used
            case POWER_HEALTH:
                powerCost += spellInfo->ManaCostPercentage * caster->GetCreateHealth() / 100;
                break;
            case POWER_MANA:
                powerCost += spellInfo->ManaCostPercentage * caster->GetCreateMana() / 100;
                break;
            case POWER_RAGE:
            case POWER_FOCUS:
            case POWER_ENERGY:
            case POWER_HAPPINESS:
                //            case POWER_RUNES:
                powerCost += spellInfo->ManaCostPercentage * caster->GetMaxPower(Powers(spellInfo->powerType)) / 100;
            break;
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Spell::CalculateManaCost: Unknown power type '%d' in spell %d", spellInfo->powerType, spellInfo->Id);
                return 0;
        }
    }
    SpellSchools school = GetFirstSchoolInMask(schoolMask);
    // Flat mod from caster auras by spell school
    powerCost += caster->GetInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + school);
    // Shiv - costs 20 + weaponSpeed*10 energy (apply only to non-triggered spell with energy cost)
    if (spellInfo->AttributesEx4 & SPELL_ATTR_EX4_SPELL_VS_EXTEND_COST)
        powerCost += caster->GetAttackTime(OFF_ATTACK)/100;
    // Apply cost mod by spell
    if (Player* modOwner = caster->GetSpellModOwner())
        modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_COST, powerCost, spell, finalUse);

    if (spellInfo->Attributes & SPELL_ATTR_LEVEL_DAMAGE_CALCULATION)
    {
        GtNPCManaCostScalerEntry const* spellScaler = sGtNPCManaCostScalerStore.LookupEntry(spellInfo->spellLevel - 1);
        GtNPCManaCostScalerEntry const* casterScaler = sGtNPCManaCostScalerStore.LookupEntry(caster->GetLevel() - 1);
        if (spellScaler && casterScaler)
            powerCost *= casterScaler->ratio / spellScaler->ratio;
    }

    // PCT mod from user auras by school
    powerCost = int32(powerCost * (1.0f+caster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+school)));
    if (powerCost < 0)
        powerCost = 0;
    return powerCost;
}

int32 SpellMgr::CompareAuraRanks(uint32 spellId_1, uint32 effIndex_1, uint32 spellId_2, uint32 effIndex_2)
{
    SpellEntry const*spellInfo_1 = sSpellTemplate.LookupEntry<SpellEntry>(spellId_1);
    SpellEntry const*spellInfo_2 = sSpellTemplate.LookupEntry<SpellEntry>(spellId_2);
    if (!spellInfo_1 || !spellInfo_2) return 0;
    if (spellId_1 == spellId_2) return 0;

    int32 diff = spellInfo_1->EffectBasePoints[effIndex_1] - spellInfo_2->EffectBasePoints[effIndex_2];
    if (spellInfo_1->CalculateSimpleValue(effIndex_1) < 0 && spellInfo_2->CalculateSimpleValue(effIndex_2) < 0) return -diff;
    else return diff;
}

SpellSpecific SpellMgr::GetSpellSpecific(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return SPELL_NORMAL;

    if (spellInfo->AttributesCu & SPELL_ATTR_CU_TREAT_AS_WELL_FED)
        return SPELL_WELL_FED;

    switch (spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            //food/drink
            if (spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            {
                for (int i = 0; i < 3; i++)
                {
                    if (spellInfo->EffectApplyAuraName[i]==SPELL_AURA_MOD_POWER_REGEN)
                        return SPELL_DRINK;
                    else if (spellInfo->EffectApplyAuraName[i]==SPELL_AURA_MOD_REGEN)
                        return SPELL_FOOD;
                }
            }
            // this may be a hack
            else
            {
                if (spellInfo->AttributesEx2 & SPELL_ATTR_EX2_FOOD && !spellInfo->Category)
                    return SPELL_WELL_FED;
            }

            switch (spellInfo->Id)
            {
                case 12880: // warrior's Enrage rank 1
                case 14201: //           Enrage rank 2
                case 14202: //           Enrage rank 3
                case 14203: //           Enrage rank 4
                case 14204: //           Enrage rank 5
                case 12292: //             Death Wish
                    return SPELL_WARRIOR_ENRAGE;
                case 38245: //             Polymorph
                case 43309: //             Polymorph
                    return SPELL_MAGE_POLYMORPH;
                break;
                default: break;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // family flags 18(Molten), 25(Frost/Ice), 28(Mage)
            if (spellInfo->SpellFamilyFlags & 0x12040000)
                return SPELL_MAGE_ARMOR;

            if ((spellInfo->SpellFamilyFlags & 0x1000000) && spellInfo->EffectApplyAuraName[0]==SPELL_AURA_MOD_CONFUSE)
                return SPELL_MAGE_POLYMORPH;

            switch(spellInfo->Id)
            {
                case 604:
                case 1008:
                case 8450:
                case 8451:
                case 8455:
                case 10169:
                case 10170:
                case 10173:
                case 10174:
                case 27130:
                case 33944:
                case 33946:
                    return SPELL_AMPLIFY_DAMPEN;
                default: break;
            }

            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (spellInfo->SpellFamilyFlags & 0x00008000010000LL)
                return SPELL_POSITIVE_SHOUT;

            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // only warlock curses have this
            if (spellInfo->Dispel == DISPEL_CURSE)
                return SPELL_CURSE;

            // family flag 37 (only part spells have family name)
            if (spellInfo->SpellFamilyFlags & 0x2000000000LL)
                return SPELL_WARLOCK_ARMOR;

            //seed of corruption and corruption
            if (spellInfo->SpellFamilyFlags & 0x1000000002LL)
                return SPELL_WARLOCK_CORRUPTION;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // only hunter stings have this
            if (spellInfo->Dispel == DISPEL_POISON)
                return SPELL_STING;

            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            if (SpellMgr::IsSealSpell(spellInfo))
                return SPELL_SEAL;

            if (spellInfo->SpellFamilyFlags & 0x10000180LL)
                return SPELL_BLESSING;

            if ((spellInfo->SpellFamilyFlags & 0x00000820180400LL) && (spellInfo->AttributesEx3 & 0x200))
                return SPELL_JUDGEMENT;

            for (int i = 0; i < 3; i++)
            {
                // only paladin auras have this, + imp blood pact, + trueshot aura hunter
                if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    return SPELL_AURA;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (SpellMgr::IsElementalShield(spellInfo))
                return SPELL_ELEMENTAL_SHIELD;

            break;
        }

        case SPELLFAMILY_POTION:
            return sSpellMgr.GetSpellElixirSpecific(spellInfo->Id);
    }

    switch(spellInfo->Id)
    {
        case 770:
        case 778:
        case 6950:
        case 9749:
        case 9806:
        case 9907:
        case 13424:
        case 13752:
        case 16498:
        case 16857:
        case 17390:
        case 17391:
        case 17392:
        case 20656:
        case 21670:
        case 25602:
        case 26993:
        case 27011:
        case 32129:
            return SPELL_FAERIE_FIRE;
        case 33876:
        case 33878:
        case 33982:
        case 33983:
        case 33986:
        case 33987:
            return SPELL_MANGLE;
        case 30550:
        case 30557:
        case 30562:
        case 30567:
            return SPELL_KARAZHAN_BOOKS;
        case 24423:
        case 24577:
        case 24578:
        case 24579:
        case 27051:
            return SPELL_SCREECH;
        default: break;
    }

    // only warlock armor/skin have this (in additional to family cases)
    if (spellInfo->SpellVisual == 130 && spellInfo->SpellIconID == 89)
    {
        return SPELL_WARLOCK_ARMOR;
    }

    // only hunter aspects have this (but not all aspects in hunter family)
    if (spellInfo->activeIconID == 122 && (GetSpellSchoolMask(spellInfo) & SPELL_SCHOOL_MASK_NATURE) &&
        (spellInfo->Attributes & 0x50000) != 0 && (spellInfo->Attributes & 0x9000010) == 0)
    {
        return SPELL_ASPECT;
    }

    for (int i = 0; i < 3; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA)
        {
            switch (spellInfo->EffectApplyAuraName[i])
            {
                case SPELL_AURA_TRACK_CREATURES:
                case SPELL_AURA_TRACK_RESOURCES:
                case SPELL_AURA_TRACK_STEALTHED:
                    return SPELL_TRACKER;
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_MOD_POSSESS_PET:
                case SPELL_AURA_MOD_POSSESS:
                    return SPELL_CHARM;
            }
        }
    }

    // elixirs can have different families, but potion most ofc.
    if (SpellSpecific sp = sSpellMgr.GetSpellElixirSpecific(spellInfo->Id))
        return sp;

    return SPELL_NORMAL;
}

bool SpellMgr::IsSingleFromSpellSpecificPerCaster(SpellSpecific spellSpec1,SpellSpecific spellSpec2)
{
    switch (spellSpec1)
    {
        case SPELL_SEAL:
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_STING:
        case SPELL_CURSE:
        case SPELL_ASPECT:
        case SPELL_POSITIVE_SHOUT:
        case SPELL_JUDGEMENT:
        case SPELL_WARLOCK_CORRUPTION:
            return spellSpec1 == spellSpec2;
        default:
            return false;
    }
}

bool SpellMgr::IsSingleFromSpellSpecificPerTarget(SpellSpecific spellSpec1, SpellSpecific spellSpec2)
{
    switch (spellSpec1)
    {
        case SPELL_TRACKER:
        case SPELL_WARLOCK_ARMOR:
        case SPELL_MAGE_ARMOR:
        case SPELL_ELEMENTAL_SHIELD:
        case SPELL_MAGE_POLYMORPH:
        case SPELL_WELL_FED:
        case SPELL_DRINK:
        case SPELL_FOOD:
        case SPELL_CHARM:
        case SPELL_WARRIOR_ENRAGE:
            return spellSpec1 == spellSpec2;
        case SPELL_BATTLE_ELIXIR:
            return spellSpec2 == SPELL_BATTLE_ELIXIR
                || spellSpec2 == SPELL_FLASK_ELIXIR;
        case SPELL_GUARDIAN_ELIXIR:
            return spellSpec2 == SPELL_GUARDIAN_ELIXIR
                || spellSpec2 == SPELL_FLASK_ELIXIR;
        case SPELL_FLASK_ELIXIR:
            return spellSpec2 == SPELL_BATTLE_ELIXIR
                || spellSpec2 == SPELL_GUARDIAN_ELIXIR
                || spellSpec2 == SPELL_FLASK_ELIXIR;
        default:
            return false;
    }
}

bool SpellMgr::IsSingleFromSpellSpecificRanksPerTarget(SpellSpecific spellId_spec, SpellSpecific i_spellId_spec)
{
    switch (spellId_spec)
    {
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_CURSE:
            return spellId_spec == i_spellId_spec;
        default:
            return false;
    }
}

bool SpellMgr::IsPositiveTarget(uint32 targetA, uint32 targetB)
{
    // non-positive targets
    switch (targetA)
    {
        case TARGET_UNIT_TARGET_ENEMY:
        case TARGET_UNIT_AREA_ENEMY_SRC:
        case TARGET_UNIT_AREA_ENEMY_DST:
        case TARGET_UNIT_CONE_ENEMY:
        case TARGET_DEST_DYNOBJ_ENEMY:
        case TARGET_DST_TARGET_ENEMY:
        case TARGET_UNIT_CHANNEL:
        case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
            return false;
        case TARGET_SRC_CASTER:
            return (targetB == TARGET_UNIT_AREA_PARTY_SRC || targetB == TARGET_UNIT_AREA_ALLY_SRC);
        default:
            break;
    }
    if (targetB)
        return SpellMgr::IsPositiveTarget(targetB, 0);
    return true;
}

bool SpellMgr::IsPositiveEffect(uint32 spellId, uint32 effIndex)
{
    SpellEntry const *spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellproto)
        return false;

    // talents
    if (SpellMgr::IsPassiveSpell(spellId) && GetTalentSpellCost(spellId))
        return true;

    /*
    // explicit targeting set positiveness independent from real effect
    // Note: IsExplicitNegativeTarget can't be used symmetric (look some TARGET_SINGLE_ENEMY spells for example)
    if (IsExplicitPositiveTarget(spellproto->EffectImplicitTargetA[effIndex]) ||
        IsExplicitPositiveTarget(spellproto->EffectImplicitTargetB[effIndex]))
        return true;
    */

    // should this work fine?
    if (spellproto->Attributes & SPELL_ATTR_NEGATIVE_1)
        return false;

    // SPELL_CHECK_POSITIVE
    switch (spellId)
    {
        case 23333:                                         // BG spell
        case 23335:                                         // BG spell
        case 24732:                                         // Bat Costume
        case 24740:                                         // Wisp Costume
        case 34976:                                         // BG spell
        case 31579:                                         // Arcane Empowerment Rank1 talent aura with one positive and one negative (check not needed in wotlk)
        case 31582:                                         // Arcane Empowerment Rank2
        case 31583:                                         // Arcane Empowerment Rank3
        case 37441:                                         // Improved Arcane Blast
        case 12042:                                         // Arcane Power
        case 40268:                                         // Spiritual Vengeance
        case 40322:                                         // Spirit Shield
        case 41151:                                         // Lightning Shield
        case 34970:                                         // Frenzy
        case 43550:                                         // Mind Control (Hex Lord Malacrass)
        case 35336:                                         // Energizing Spores
        case 40604:                                         // Fel Rage 1
        case 40616:                                         // Fel Rage 2
        case 41625:                                         // Fel Rage 3
        case 46787:                                         // Fel Rage scale
        case 38318:                                         // Orb of Blackwhelp
		case 37851:                                         // tag greater felfire diemetradon
		case 37907:                                         // kill credit felfire diemetradon
		case 40825:                                         // blade edge banish the demons quest (dont get in combat with trigger)
        case 5024:                                          // Flee by Skull of Impending Doom
            return true;
        case 46392:                                         // Focused Assault
        case 46393:                                         // Brutal Assault
        case 43437:                                         // Paralyzed
        case 28441:                                         // not positive dummy spell
        case 37675:                                         // Chaos Blast
        case 41519:                                         // Mark of Stormrage
        case 34877:                                         // Custodian of Time
        case 34700:                                         // Allergic Reaction
        case 31719:                                         // Suspension
        case 41406:                                         // Dementia +
        case 41409:                                         // Dementia -
        case 30529:                                         // Chess event: Recently In Game
        case 37469:
        case 37465:
        case 37128:                                         // Doomwalker - Mark of Death
        case 30421:                                         // Neterspite - Player buffs(3)
        case 30422:
        case 30423:
        case 30457:                                         // Complete Vulnerability
        case 47002:                                         // Noxious Fumes (not sure if needed, just in case)
        case 41350:                                         // Aura of Desire
        case 43501:                                         // Siphon Soul (Hexlord Spell)
        case 41292:                                         // Aura of Suffering
        case 42017:                                         // Aura of Suffering
        case 45350:                                         // Gamemaster Whisper visual aura
        case 41070:                                         // Death Coil - BT Trash
            return false;
        case 32375:
            if(effIndex == 0)                               // Mass Dispel on friendly targets
                return true;
            else                                            // Mass Dispel on enemy targets
                return false;
    }

    switch (spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_MAGE:
            // Amplify Magic, Dampen Magic
            if (spellproto->SpellFamilyFlags & 0x20000000000)
                return true;
            break;
        case SPELLFAMILY_HUNTER:
            // Aspect of the Viper
            if (spellproto->Id == 34074)
                return true;
            break;
        default:
            break;
    }

    switch (spellproto->Mechanic)
    {
        case MECHANIC_IMMUNE_SHIELD:
            return true;
        default:
            break;
    }

    switch (spellproto->Effect[effIndex])
    {
        // always positive effects (check before target checks that provided non-positive result in some case for positive effects)
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_LEARN_SPELL:
        case SPELL_EFFECT_SKILL_STEP:
        case SPELL_EFFECT_HEAL_PCT:
        case SPELL_EFFECT_ENERGIZE_PCT:
            return true;

            // non-positive aura use
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
        {
            switch (spellproto->EffectApplyAuraName[effIndex])
            {
                case SPELL_AURA_DUMMY:
                {
                    // dummy aura can be positive or negative dependent from cast spell
                    switch (spellproto->Id)
                    {
                        case 13139:                         // net-o-matic special effect
                        case 23445:                         // evil twin
                        case 35679:                         // Protectorate Demolitionist
                        case 38637:                         // Nether Exhaustion (red)
                        case 38638:                         // Nether Exhaustion (green)
                        case 38639:                         // Nether Exhaustion (blue)
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_STAT:
                case SPELL_AURA_MOD_DAMAGE_DONE:            // dependent from bas point sign (negative -> negative)
                case SPELL_AURA_MOD_HEALING_DONE:
                    if (spellproto->CalculateSimpleValue(effIndex) < 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
                    if (spellproto->EffectBasePoints[effIndex] > 0)
                        return true;
                    break;
                case SPELL_AURA_ADD_TARGET_TRIGGER:
                    return true;
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    if (spellId != spellproto->EffectTriggerSpell[effIndex])
                    {
                        uint32 spellTriggeredId = spellproto->EffectTriggerSpell[effIndex];
                        SpellEntry const *spellTriggeredProto = sSpellTemplate.LookupEntry<SpellEntry>(spellTriggeredId);

                        if (spellTriggeredProto)
                        {
                            // non-positive targets of main spell return early
                            for (int i = 0; i < 3; ++i)
                            {
                                // if non-positive trigger cast targeted to positive target this main cast is non-positive
                                // this will place this spell auras as debuffs
                                if (SpellMgr::IsPositiveTarget(spellTriggeredProto->EffectImplicitTargetA[effIndex],spellTriggeredProto->EffectImplicitTargetB[effIndex]) && !IsPositiveEffect(spellTriggeredId,i))
                                    return false;
                            }
                        }
                    }
                    break;
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                    // many positive auras have negative triggered spells at damage for example and this not make it negative (it can be canceled for example)
                    break;
                case SPELL_AURA_MOD_STUN:                   //have positive and negative spells, we can't sort its correctly at this moment.
                    if (effIndex==0 && spellproto->Effect[1]==0 && spellproto->Effect[2]==0)
                        return false;                       // but all single stun aura spells is negative

                    // Petrification
                    if (spellproto->Id == 17624)
                        return false;
                    break;
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_SILENCE:
                case SPELL_AURA_GHOST:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_MOD_PACIFY_SILENCE:
                case SPELL_AURA_MOD_STALKED:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                    return false;
                case SPELL_AURA_PERIODIC_DAMAGE:            // used in positive spells also.
                    // part of negative spell if cast at self (prevent cancel)
                    if (spellproto->EffectImplicitTargetA[effIndex] == TARGET_UNIT_TARGET_ANY)
                        return false;
                    // part of negative spell if cast at self (prevent cancel)
                    else if (spellproto->EffectImplicitTargetA[effIndex] == TARGET_UNIT_CASTER)
                        return false;
                    break;
                case SPELL_AURA_MOD_DECREASE_SPEED:         // used in positive spells also
                    // part of positive spell if cast at self
                    if (spellproto->EffectImplicitTargetA[effIndex] != TARGET_UNIT_CASTER)
                        return false;
                    // but not this if this first effect (don't found batter check)
                    if (spellproto->Attributes & 0x4000000 && effIndex==0)
                        return false;
                    break;
                case SPELL_AURA_TRANSFORM:
                    // some spells negative
                    switch (spellproto->Id)
                    {
                        case 36897:                         // Transporter Malfunction (race mutation to horde)
                        case 36899:                         // Transporter Malfunction (race mutation to alliance)
                            return false;
                    }
                    break;
                case SPELL_AURA_MOD_SCALE:
                    // some spells negative
                    switch (spellproto->Id)
                    {
                        case 36900:                         // Soul Split: Evil!
                        case 36901:                         // Soul Split: Good
                        case 36893:                         // Transporter Malfunction (decrease size case)
                        case 36895:                         // Transporter Malfunction (increase size case)
                            return false;
                    }
                    break;
                case SPELL_AURA_MECHANIC_IMMUNITY:
                {
                    // non-positive immunities
                    switch (spellproto->EffectMiscValue[effIndex])
                    {
                        case MECHANIC_BANDAGE:
                        case MECHANIC_SHIELD:
                        case MECHANIC_MOUNT:
                        case MECHANIC_INVULNERABILITY:
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_ADD_FLAT_MODIFIER:          // mods
                case SPELL_AURA_ADD_PCT_MODIFIER:
                {
                    // non-positive mods
                    switch (spellproto->EffectMiscValue[effIndex])
                    {
                        case SPELLMOD_COST:                 // dependent from bas point sign (negative -> positive)
                            if (spellproto->CalculateSimpleValue(effIndex) > 0)
                            {
                                if (spellproto->Id > 54769)
                                    return true;
                                return false;
                            }
                            break;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_HEALING_PCT:
                    if (spellproto->CalculateSimpleValue(effIndex) < 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_SKILL:
                    if (spellproto->CalculateSimpleValue(effIndex) < 0)
                        return false;
                    break;
                case SPELL_AURA_FORCE_REACTION:
                    if (spellproto->Id==42792)               // Recently Dropped Flag (prevent cancel)
                        return false;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    // non-positive targets
    if (!SpellMgr::IsPositiveTarget(spellproto->EffectImplicitTargetA[effIndex],spellproto->EffectImplicitTargetB[effIndex]))
        return false;

    // ok, positive
    return true;
}

bool SpellMgr::IsPositiveSpell(uint32 spellId)
{
    SpellEntry const *spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellproto) return false;

    // talents
    if (SpellMgr::IsPassiveSpell(spellId) && GetTalentSpellCost(spellId))
        return true;

    // spells with at least one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < 3; i++)
    {
        if (!SpellMgr::IsPositiveEffect(spellId, i))
            return false;
    }

    return true;
}

bool SpellMgr::IsSingleTargetSpell(SpellEntry const *spellInfo)
{
    // all other single target spells have if it has AttributesEx5
    if (spellInfo->AttributesEx5 & SPELL_ATTR_EX5_SINGLE_TARGET_SPELL)
        return true;

    // TODO - need found Judgements rule
    switch (SpellMgr::GetSpellSpecific(spellInfo->Id))
    {
        case SPELL_JUDGEMENT:
            return true;
    }

    // single target triggered spell.
    // Not real client side single target spell, but it' not triggered until prev. aura expired.
    // This is allow store it in single target spells list for caster for spell proc checking
    if (spellInfo->Id==38324)                                // Regeneration (triggered by 38299 (HoTs on Heals))
        return true;

    return false;
}

bool SpellMgr::IsSingleTargetSpells(SpellEntry const *spellInfo1, SpellEntry const *spellInfo2)
{
    // TODO - need better check
    // Equal icon and spellfamily
    if (spellInfo1->SpellFamilyName == spellInfo2->SpellFamilyName &&
        spellInfo1->SpellIconID == spellInfo2->SpellIconID)
        return true;

    // TODO - need found Judgements rule
    SpellSpecific spec1 = SpellMgr::GetSpellSpecific(spellInfo1->Id);
    // spell with single target specific types
    switch (spec1)
    {
        case SPELL_JUDGEMENT:
        case SPELL_MAGE_POLYMORPH:
            if (SpellMgr::GetSpellSpecific(spellInfo2->Id) == spec1)
                return true;
            break;
    }

    return false;
}

bool SpellMgr::IsAuraAddedBySpell(uint32 auraType, uint32 spellId)
{
    SpellEntry const *spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellproto) return false;

    for (int i = 0; i < 3; i++)
        if (spellproto->EffectApplyAuraName[i] == auraType)
            return true;
    return false;
}

SpellCastResult SpellMgr::GetErrorAtShapeshiftedCast (SpellEntry const *spellInfo, uint32 form)
{
    // talents that learn spells can have stance requirements that need ignore
    // (this requirement only for client-side stance show in talent description)
    if (GetTalentSpellCost(spellInfo->Id) > 0 &&
        (spellInfo->Effect[0]==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[1]==SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[2]==SPELL_EFFECT_LEARN_SPELL))
        return SPELL_CAST_OK;

    uint32 stanceMask = (form ? 1 << (form - 1) : 0);

    if (stanceMask & spellInfo->StancesNot)                 // can explicitly not be cast in this stance
        return SPELL_FAILED_NOT_SHAPESHIFT;

    if (stanceMask & spellInfo->Stances)                    // can explicitly be cast in this stance
        return SPELL_CAST_OK;

    bool actAsShifted = false;
    if (form > 0)
    {
        SpellShapeshiftEntry const *shapeInfo = sSpellShapeshiftStore.LookupEntry(form);
        if (!shapeInfo)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: GetErrorAtShapeshiftedCast: unknown shapeshift %u", form);
            return SPELL_CAST_OK;
        }
        actAsShifted = !(shapeInfo->flags1 & 1);            // shapeshift acts as normal form for spells
    }

    if (actAsShifted)
    {
        if (spellInfo->Attributes & SPELL_ATTR_NOT_SHAPESHIFT) // not while shapeshifted
            return SPELL_FAILED_NOT_SHAPESHIFT;
        else if (spellInfo->Stances != 0)                   // needs other shapeshift
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    else
    {
        // needs shapeshift
        if (!(spellInfo->AttributesEx2 & SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) && spellInfo->Stances != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }

    return SPELL_CAST_OK;
}

bool SpellMgr::IsBinaryResistable(SpellEntry const* spellInfo)
{
    if(spellInfo->SchoolMask & SPELL_SCHOOL_MASK_HOLY)                  // can't resist holy spells
        return false;

    if (spellInfo->SpellFamilyName)         // only player's spells, bosses don't follow that simple rule
    {
        for (int eff = 0; eff < 3; eff++)
        {
            if (!spellInfo->Effect[eff])
                continue;

            if (SpellMgr::IsPositiveEffect(spellInfo->Id, eff))
                continue;

            switch (spellInfo->Effect[eff])
            {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
                break;
            case SPELL_EFFECT_APPLY_AURA:
            case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
                if (spellInfo->EffectApplyAuraName[eff] != SPELL_AURA_PERIODIC_DAMAGE)       // spells that apply aura other then DOT are binary resistable
                    return true;
                break;
            default:
                return true;                                                                // spells that have other effects then damage or apply aura are binary resistable
            }
        }
    }

    switch (spellInfo->Id)
    {
        case 31306:     // Anetheron - Carrion Swarm
        case 31344:     // Howl of Azgalor
        case 31447:     // Mark of Kaz'Rogal
        case 34190:     // Void - Arcane Orb
        case 37730:     // Morogrim - Tidal Wave
        case 38441:     // Fathom - Cataclysm bolt
        case 38509:     // Vashj - Shock Blast
        case 37675:     // Leotheras - Chaos Blast
            return true;
    }
    return false;
}

bool SpellMgr::IsPartialyResistable(SpellEntry const* spellInfo)
{
    if (spellInfo->AttributesEx4 & SPELL_ATTR_EX4_IGNORE_RESISTANCES)
        return false;

    // check which of them have above attribute and remove it from switch :]
    switch (spellInfo->Id)
    {
        case 30115:     // Terestian - Sacrifice
        case 33051:     // Krosh Firehand - Greater Fireball
        case 36805:     // Kael'thas - Fireball
        case 36819:     // Kael'thas - Pyroblast
        case 31944:     // Archimond - Doomfire
        case 31972:     // Archimond - Grip of the Legion
        case 32053:     // Archimond - Soul Charge, red
        case 32054:     // Archimond - Soul Charge, yellow
        case 32057:     // Archimond - Soul Charge, green
        case 41545:     // RoS: Soul Scream
        case 41376:     // RoS: Spite
        case 41352:     // RoS: Aura of Desire dmg back
        case 41337:     // RoS: Aura of Anger
        case 40239:     // Teron: Incinerate
        case 40325:     // Teron: Spirit Strike
        case 40157:     // Teron: Spirit Lance
        case 40175:     // Teron: Spirit Chains
        case 41483:     // High Nethermancer Zerevor: Arcane Bolt
        case 44335:     // Vexallus: Energy Feedback
        case 47002:     // Felmyst: Noxious Fumes
        case 45866:     // Felmyst: Corrosion
        case 45855:     // Felmyst: Gas Nova
            return false;
    }

	if (spellInfo->SchoolMask & SPELL_SCHOOL_MASK_HOLY)                  // can't resist holy spells
		return false;

    if (SpellMgr::IsBinaryResistable(spellInfo))
        return false;
    else
        return true;
}

void SpellMgr::LoadSpellTargetPositions()
{
    mSpellTargetPositions.clear();                                // need for reload case

    uint32 count = 0;

    //                                                       0   1           2                  3                  4                  5
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position");
    if (!result)
    {

        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell target coordinates", count);
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        ++count;

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.target_mapId       = fields[1].GetUInt32();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(Spell_ID);
        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (ID:%u) listed in `spell_target_position` does not exist.",Spell_ID);
            continue;
        }

        bool found = false;
        for (int i = 0; i < 3; ++i)
        {
            if (spellInfo->EffectImplicitTargetA[i]==TARGET_DST_DB || spellInfo->EffectImplicitTargetB[i]==TARGET_DST_DB)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_DST_DB (17).",Spell_ID);
            continue;
        }

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if (!mapEntry)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.",Spell_ID,st.target_mapId);
            continue;
        }

        if (st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            sLog.outLog(LOG_DB_ERR, "Spell (ID:%u) target coordinates not provided.",Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell teleport coordinates", count);
}

void SpellMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                                       0      1         2
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, effectId, SpellFamilyMask FROM spell_affect");
    if (!result)
    {

        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell affect definitions", count);
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 effectId = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(entry);

        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= 3)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have invalid effect index (%u)", entry,effectId);
            continue;
        }

        if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry,SPELL_AURA_ADD_FLAT_MODIFIER,SPELL_AURA_ADD_PCT_MODIFIER,SPELL_AURA_ADD_TARGET_TRIGGER,effectId);
            continue;
        }

        uint64 spellAffectMask = fields[2].GetUInt64();

        // Spell.dbc have own data for low part of SpellFamilyMask
        if (spellInfo->EffectItemType[effectId])
        {
            if (spellInfo->EffectItemType[effectId] == spellAffectMask)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have redundant (same with EffectItemType%d) data for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }

            // 24429 have wrong data in EffectItemType and overwrites by DB, possible bug in client
            if (spellInfo->Id!=24429 && spellInfo->EffectItemType[effectId] != spellAffectMask)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_affect` have different low part from EffectItemType%d for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }
        }

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,spellAffectMask));

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell affect definitions", count);

    for (uint32 id = 0; id < sSpellTemplate.GetMaxEntry(); ++id)
    {
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(id);
        if (!spellInfo)
            continue;

        for (int effectId = 0; effectId < 3; ++effectId)
        {
            if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA ||
                (spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER))
                continue;

            if (spellInfo->EffectItemType[effectId] != 0)
                continue;

            if (mSpellAffectMap.find((id<<8) + effectId) !=  mSpellAffectMap.end())
                continue;

            sLog.outLog(LOG_DB_ERR, "Spell %u (%s) misses spell_affect for effect %u",id,spellInfo->SpellName[sWorld.GetDefaultDbcLocale()], effectId);
        }
    }
}

bool SpellMgr::IsAffectedBySpell(SpellEntry const *spellInfo, uint32 spellId, uint8 effectId, uint64 familyFlags) const
{
    // false for spellInfo == NULL
    if (!spellInfo)
        return false;

    SpellEntry const *affect_spell = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    // false for affect_spell == NULL
    if (!affect_spell)
        return false;
    
    if ((spellId == 37706 || spellId == 24546) && (spellInfo->Effect[0] == SPELL_EFFECT_HEAL || spellInfo->EffectApplyAuraName[0] == SPELL_AURA_PERIODIC_HEAL))
        return true;

    // False if spellFamily not equal
    if (affect_spell->SpellFamilyName != spellInfo->SpellFamilyName)
        return false;

    // If familyFlags == 0
    if (!familyFlags)
    {
        // Get it from spellAffect table
        familyFlags = GetSpellAffectMask(spellId,effectId);
        // false if familyFlags == 0
        if (!familyFlags)
            return false;
    }

    // true
    if (familyFlags & spellInfo->SpellFamilyFlags)
        return true;

    // mage conjured spells (dangerous?)
    //if (spellInfo->SpellFamilyName == SPELLFAMILY_MAGE && spellInfo->SpellFamilyFlags == uint64(0x40000000))
    //    return true;

    return false;
}

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                                       0      1           2                3                4          5       6        7             8
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, SchoolMask, SpellFamilyName, SpellFamilyMask, procFlags, procEx, ppmRate, CustomChance, Cooldown FROM spell_proc_event");
    if (!result)
    {

        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell proc event conditions", count );
        return;
    }

    //BarGoLink bar(result->GetRowCount());
    uint32 customProc = 0;
    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        uint16 entry = fields[0].GetUInt16();

        const SpellEntry *spell = sSpellTemplate.LookupEntry<SpellEntry>(entry);
        if (!spell)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_proc_event` does not exist", entry);
            continue;
        }

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.spellFamilyName = fields[2].GetUInt32();
        spe.spellFamilyMask = fields[3].GetUInt64();
        spe.procFlags       = fields[4].GetUInt32();
        spe.procEx          = fields[5].GetUInt32();
        spe.ppmRate         = fields[6].GetFloat();
        spe.customChance    = fields[7].GetFloat();
        spe.cooldown        = fields[8].GetUInt32();

        mSpellProcEventMap[entry] = spe;

        if (spell->procFlags==0)
        {
            if (spe.procFlags == 0)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_proc_event` probally not triggered spell", entry);
                continue;
            }
            customProc++;
        }
        ++count;
    } while (result->NextRow());

    sLog.outString();
    if (customProc)
        sLog.outString(">> Loaded %u custom spell proc event conditions +%u custom",  count, customProc);
    else
        sLog.outString(">> Loaded %u spell proc event conditions", count);

    /*
    // Commented for now, as it still produces many errors (still quite many spells miss spell_proc_event)
    for (uint32 id = 0; id < sSpellTemplate.GetMaxEntry(); ++id)
    {
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(id);
        if (!spellInfo)
            continue;

        bool found = false;
        for (int effectId = 0; effectId < 3; ++effectId)
        {
            // at this moment check only SPELL_AURA_PROC_TRIGGER_SPELL
            if (spellInfo->EffectApplyAuraName[effectId] == SPELL_AURA_PROC_TRIGGER_SPELL)
            {
                found = true;
                break;
            }
        }

        if (!found)
            continue;

        if (GetSpellProcEvent(id))
            continue;

        sLog.outLog(LOG_DB_ERR, "Spell %u (%s) misses spell_proc_event",id,spellInfo->SpellName[sWorld.GetDBClang()]);
    }
    */
}

/*
bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const * spellProcEvent, SpellEntry const * procSpell, uint32 procFlags)
{
    if ((procFlags & spellProcEvent->procFlags) == 0)
        return false;

    // Additional checks in case spell cast/hit/crit is the event
    // Check (if set) school, category, skill line, spell talent mask
    if (spellProcEvent->schoolMask && (!procSpell || (GetSpellSchoolMask(procSpell) & spellProcEvent->schoolMask) == 0))
        return false;
    if (spellProcEvent->category && (!procSpell || procSpell->Category != spellProcEvent->category))
        return false;
    if (spellProcEvent->skillId)
    {
        if (!procSpell)
            return false;

        SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(procSpell->Id);
        SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(procSpell->Id);

        bool found = false;
        for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
        {
            if (_spell_idx->second->skillId == spellProcEvent->skillId)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    if (spellProcEvent->spellFamilyName && (!procSpell || spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
        return false;
    if (spellProcEvent->spellFamilyMask && (!procSpell || (spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0))
        return false;

    return true;
}
*/

bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const * spellProcEvent, uint32 EventProcFlag, SpellEntry const * procSpell, uint32 procFlags, uint32 procExtra, bool active)
{
    // No extra req need
    uint32 procEvent_procEx = PROC_EX_NONE;

    // check prockFlags for condition
    if ((procFlags & EventProcFlag) == 0)
        return false;

    /* Check Periodic Auras

    * Both hots and dots can trigger if spell has no PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL
        nor PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT

    *Only Hots can trigger if spell has PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL

    *Only dots can trigger if spell has both positivity flags or PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT

    */

    /*if (EventProcFlag & PROC_FLAG_ON_DO_PERIODIC) // rare case, only 7 spells that should trigger ONLY from tick
    {
        if (!(procFlags & PROC_FLAG_ON_DO_PERIODIC))
            return false;
    }*/ // Trentone says: this is stupid.
    // With this any spell that have DOT_PROC and any other procFlag - will not be able to proc from other types
    // just an example of different-flagged spell: 28744
    // Also 37381: pet healing

    if (procFlags & PROC_FLAG_ON_DO_PERIODIC)
    {
        if (EventProcFlag & PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT)
        {
            if (!(procExtra & PROC_EX_INTERNAL_DOT))
                return false;
        }
        else if (EventProcFlag & PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL
            && !(procExtra & PROC_EX_INTERNAL_HOT))
            return false;
    }

    if (procFlags & PROC_FLAG_ON_TAKE_PERIODIC)
    {
        if (EventProcFlag & PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT)
        {
            if (!(procExtra & PROC_EX_INTERNAL_DOT))
                return false;
        }
        else if (EventProcFlag & PROC_FLAG_TAKEN_POSITIVE_SPELL
            && !(procExtra & PROC_EX_INTERNAL_HOT))
            return false;
    }

    // Always trigger for this
    if (EventProcFlag & (PROC_FLAG_KILLED | PROC_FLAG_KILL_AND_GET_XP))
        return true;

    if (spellProcEvent)     // Exist event data
    {
        // Store extra req
        procEvent_procEx = spellProcEvent->procEx;

        // For melee triggers
        if (procSpell == NULL)
        {
            // Check (if set) for school (melee attack have Normal school)
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Potions can trigger only if spellfamily given
            if (procSpell->SpellFamilyName == SPELLFAMILY_POTION)
            {
                if (procSpell->SpellFamilyName == spellProcEvent->spellFamilyName)
                    return true;
                return false;
            }

            // Check (if set) for school
            if (spellProcEvent->schoolMask && (spellProcEvent->schoolMask & procSpell->SchoolMask) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if (spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                return false;

            // spellFamilyName is Ok need check for spellFamilyMask if present
            if (spellProcEvent->spellFamilyMask)
            {
                if ((spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0)
                    return false;
                active = true; // Spell added manualy -> so its active spell
            }
        }
    }
    // potions can trigger only if have spell_proc entry
    else if (procSpell && procSpell->SpellFamilyName==SPELLFAMILY_POTION)
        return false;

    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // No extra req, so can trigger only for active (damage/healing present) and hit/crit
        if ((procExtra & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)) && (active || (procSpell && procSpell->SpellFamilyName == SPELLFAMILY_MAGE && (procSpell->SpellFamilyFlags & 0x800) && (EventProcFlag & PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT)))) // Check near active is for Arcane Missiles for initial hit - cause it's not active
            return true;
    }
    else // Passive spells hits here only if resist/reflect/immune/evade
    {
        if (procEvent_procEx & PROC_EX_SHAMAN_SHIELD)
        {
            if(procSpell && procSpell->AttributesEx3 & SPELL_ATTR_EX3_NO_INITIAL_AGGRO)
                return false;
            else
                return true;
        }
        if (procEvent_procEx & PROC_EX_NETHER_PROTECTION)
        {
            if (procSpell == NULL)
                return false;
            if (procSpell->Effect[0] == SPELL_EFFECT_ENVIRONMENTAL_DAMAGE)
                return false;
            if (procExtra & PROC_EX_NORMAL_HIT)
                return true;

            return false;
        }
        // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
        if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
            return true;
        // Passive spells can`t trigger if need hit
        if ((procEvent_procEx & PROC_EX_NORMAL_HIT & procExtra) && !active)
            return false;
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    return false;
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                       0      1
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, mask FROM spell_elixir");
    if (!result)
    {

        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell elixir definitions", count);
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        uint16 entry = fields[0].GetUInt16();
        uint8 mask = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(entry);

        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_elixir` does not exist", entry);
            continue;
        }

        mSpellElixirs[entry] = mask;

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell elixir definitions", count);
}

void SpellMgr::LoadSpellThreats()
{
    mSpellThreatMap.clear();
    uint32 count = 0;

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, Threat, multiplier FROM spell_threat");
    if (!result)
    {
        sLog.outString();
        sLog.outString(">> Loaded %u spell threat definitions", count);
        return;
    }

    // @!tanks_boost warrior Thunder Clap generates more rage
    const std::set<uint32> specialSpells = { 6343, 8198, 8204, 8205, 11580, 11581, 25264 };
    const bool isEasyRealm = sWorld.isEasyRealm();

    auto ApplyThreatModifierIfNecessary = [&](uint32 spell_id, SpellThreatEntry& ste)
    {
        if (isEasyRealm && specialSpells.count(spell_id))
            ste.multiplier = 5;
    };

    std::function<void(uint32, SpellThreatEntry)> FillHigherRanks = [&](uint32 spell_id, SpellThreatEntry data)
    {
        uint32 next_spell = sSpellMgr.GetNextSpellInChain(spell_id);
        if (!next_spell || GetSpellThreat(next_spell)) 
            return;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(next_spell);
        if (!spellInfo)
        {
            sLog.outLog(LOG_CRITICAL, "LoadSpellThreats - Spell %u does not exist", next_spell);
            return;
        }

        data.spellId = next_spell;
        ApplyThreatModifierIfNecessary(next_spell, data);
        mSpellThreatMap[next_spell] = data;
        FillHigherRanks(next_spell, data);
    };

    while (result->NextRow())
    {
        Field* fields = result->Fetch();
        uint32 spell_id = fields[0].GetUInt32();

        SpellThreatEntry ste;
        ste.spellId = spell_id;
        ste.threat = fields[1].GetUInt16();
        ste.multiplier = fields[2].GetFloat();

        if (!sSpellTemplate.LookupEntry<SpellEntry>(spell_id))
        {
            sLog.outLog(LOG_DB_ERR, "LoadSpellThreats - Spell %u does not exist", spell_id);
            continue;
        }

        ApplyThreatModifierIfNecessary(spell_id, ste);
        mSpellThreatMap[spell_id] = ste;
        FillHigherRanks(spell_id, ste);
        ++count;
    }

    sLog.outString(">> Loaded %u spell threat definitions", count);
    sLog.outString();
}

void SpellMgr::LoadSpellBonusData()
{
    mSpellBonusDataMap.clear();
    uint32 count = 0;

    //                                                0      1          2       3             4
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, direct_co, dot_co, direct_ap_co, dot_ap_co, FROM spell_bonus_data");
    if (!result)
    {

        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell bonus data info", count);
        return;
    }

    //BarGoLink bar(result->GetRowCount());
    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        uint32 entry        = fields[0].GetUInt32();

        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(entry);
        if (!spellInfo)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_bonus_data` does not exist", entry);
            continue;
        }

        SpellBonusData bd;
        bd.direct_co    = fields[1].GetFloat();
        bd.dot_co       = fields[2].GetFloat();
        bd.direct_ap_co = fields[3].GetFloat();
        bd.dot_ap_co    = fields[4].GetFloat();

        mSpellBonusDataMap[entry] = bd;

        ++count;
    }
    while (result->NextRow());

    sLog.outString(">> Loaded %u spell bonus data definitions", count);
}

void SpellMgr::LoadSpellEnchantProcData()
{
    mSpellEnchantProcEventMap.clear();                             // need for reload case

    uint32 count = 0;

    //                                                       0      1             2          3          4
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, customChance, PPMChance, procFlags, procEx FROM spell_enchant_proc_data");
    if (!result)
    {

        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell enchant proc event conditions", count);
        return;
    }

    //BarGoLink bar(result->GetRowCount());
    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        uint32 enchantId = fields[0].GetUInt32();

        SpellItemEnchantmentEntry const *ench = sSpellItemEnchantmentStore.LookupEntry(enchantId);
        if (!ench)
        {
            sLog.outLog(LOG_DB_ERR, "Enchancment %u listed in `spell_enchant_proc_data` does not exist", enchantId);
            continue;
        }

        SpellEnchantProcEntry spe;

        spe.customChance = fields[1].GetUInt32();
        spe.PPMChance = fields[2].GetFloat();
        spe.procFlags = fields[3].GetUInt32();
        spe.procEx = fields[4].GetUInt32();

        mSpellEnchantProcEventMap[enchantId] = spe;

        ++count;
    } while (result->NextRow());

    sLog.outString(">> Loaded %u enchant proc data definitions", count);
}

void SpellMgr::LoadSpellAnalogs()
{
    mSpellAnalogViceVersaMap.clear();                                  // for reload case

    uint32 count = 0;

    //                                                            0          1
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT old_entry, new_entry FROM spell_analog_vice_versa");
    if (!result)
    {
        //BarGoLink bar(1);

        //bar.step();

        sLog.outString(">> Loaded 0 spell_analog_vice_versa");
    }
    else
    {
        //BarGoLink bar(result->GetRowCount());
        do
        {
            Field *fields = result->Fetch();

            //bar.step();

            uint32 oldEntry = fields[0].GetUInt32();
            uint32 newEntry = fields[1].GetUInt32();

            SpellAnalogViceVersaEntries const* entries = GetSpellAnalogViceVersa(oldEntry);
            if (entries)
            {
                ((SpellAnalogViceVersaEntries*)entries)->push_back(newEntry);
            }
            else
            {
                SpellAnalogViceVersaEntries entries;
                entries.push_back(newEntry);
                mSpellAnalogViceVersaMap[oldEntry] = entries;
            }

            ++count;
        } while (result->NextRow());

        sLog.outString(">> Loaded %u spell_analog_vice_versa", count);
    }

    for (SpellAnalogViceVersaMap::iterator itr = mSpellAnalogViceVersaMap.begin(); itr != mSpellAnalogViceVersaMap.end(); ++itr)
        itr->second.shrink_to_fit();

    mSpellAnalogMap.clear();                                  // for reload case

    count = 0;

    //                                        0          1                2                3                    4                 5
    result = GameDataDatabase.Query("SELECT new_entry, old_entry, visual_caster_start, visual_target_start, visual_caster_end, visual_target_end FROM spell_analog");
    if (!result)
    {
        //BarGoLink bar(1);

        //bar.step();

        sLog.outString(">> Loaded 0 spell_analog_vice_versa", count);
    }
    else
    {
        //BarGoLink bar(result->GetRowCount());
        do
        {
            Field *fields = result->Fetch();

            //bar.step();

            uint32 newEntry = fields[0].GetUInt32();
            uint32 oldEntry = fields[1].GetUInt32();
            uint32 visCasterStart = fields[2].GetUInt32();
            uint32 visTargetStart = fields[3].GetUInt32();
            uint32 visCasterEnd = fields[4].GetUInt32();
            uint32 visTargetEnd = fields[5].GetUInt32();

            if (!oldEntry && !visCasterStart && !visTargetStart && !visCasterEnd && !visTargetEnd)
            {
                sLog.outLog(LOG_DB_ERR, "Spell Analog (Id: %u) has nothing but new_entry inside - it's useless!)", newEntry);
                continue;
            }

            SpellAnalog analog;
            analog.old_entry = oldEntry;
            analog.visual_caster_end = visCasterEnd;
            analog.visual_caster_start = visCasterStart;
            analog.visual_target_end = visTargetEnd;
            analog.visual_target_start = visTargetStart;

            mSpellAnalogMap[newEntry] = analog;

            ++count;
        } while (result->NextRow());

        sLog.outString(">> Loaded %u spell_analog", count);
    }
}

bool SpellMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2) const
{
    SpellEntry const *spellInfo_2 = sSpellTemplate.LookupEntry<SpellEntry>(spellId_2);
    if (!spellInfo_1 || !spellInfo_2) return false;
    if (spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::canStackSpellRanks(SpellEntry const *spellInfo)
{
    // exception: faerie fire (feral)
    if (spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags & 0x400)
        return true;

    if (spellInfo->powerType != POWER_MANA && spellInfo->powerType != POWER_HEALTH)
        return false;

    if (SpellMgr::IsProfessionSpell(spellInfo->Id))
        return false;

    // All stance spells. if any better way, change it.
    for (int i = 0; i < 3; i++)
    {
        // Paladin aura Spell
        if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
            return false;
        // Druid form Spell
        if (spellInfo->SpellFamilyName == SPELLFAMILY_DRUID
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA
            && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
        // Rogue Stealth
        if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE
            && spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA
            && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
            return false;
    }
    return true;
}

bool SpellMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2, bool sameCaster, bool procTrigger)
{
    //if(spellId_1 == spellId_2) // auras due to the same spell
    //    return false;
    SpellEntry const *spellInfo_1 = sSpellTemplate.LookupEntry<SpellEntry>(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellTemplate.LookupEntry<SpellEntry>(spellId_2);

    if (!spellInfo_1 || !spellInfo_2)
        return false;

    if (SpellMgr::IsSpecialStackCase(spellInfo_1, spellInfo_2, sameCaster))
        return false;

    if (SpellMgr::IsSpecialNoStackCase(spellInfo_1, spellInfo_2, sameCaster))
        return true;

    SpellSpecific spellId_spec_1 = SpellMgr::GetSpellSpecific(spellId_1);
    SpellSpecific spellId_spec_2 = SpellMgr::GetSpellSpecific(spellId_2);

    // Seal of Command case
    if (procTrigger && spellId_spec_1 == SPELL_SEAL && spellId_spec_2 == SPELL_SEAL && spellInfo_1->SpellIconID == 561)
        return false;

    if (spellId_spec_1 && spellId_spec_2)
        if (SpellMgr::IsSingleFromSpellSpecificPerTarget(spellId_spec_1, spellId_spec_2)
            ||(SpellMgr::IsSingleFromSpellSpecificPerCaster(spellId_spec_1, spellId_spec_2) && sameCaster) ||
            (SpellMgr::IsSingleFromSpellSpecificRanksPerTarget(spellId_spec_1, spellId_spec_2) && sSpellMgr.IsRankSpellDueToSpell(spellInfo_1, spellId_spec_2)))
            return true;

    // spells with different specific always stack
	if (spellId_spec_1 != spellId_spec_2)
		return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    // generic spells
    if (!spellInfo_1->SpellFamilyName)
    {
        if (!spellInfo_1->SpellIconID
            || spellInfo_1->SpellIconID == 1
            || spellInfo_1->SpellIconID != spellInfo_2->SpellIconID)
            return false;
    }

    // check for class spells
    else
    {
        if (spellInfo_1->SpellFamilyFlags != spellInfo_2->SpellFamilyFlags)
            return false;
    }

    if (!sameCaster)
    {
        for (uint32 i = 0; i < 3; ++i)
            if (spellInfo_1->Effect[i] == SPELL_EFFECT_APPLY_AURA
                || spellInfo_1->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                // not area auras (shaman totem)
                switch (spellInfo_1->EffectApplyAuraName[i])
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
                        return false;
                    default:
                        break;
                }
    }

    //use data of highest rank spell(needed for spells which ranks have different effects)
    spellInfo_1=sSpellTemplate.LookupEntry<SpellEntry>(sSpellMgr.GetLastSpellInChain(spellId_1));
    spellInfo_2=sSpellTemplate.LookupEntry<SpellEntry>(sSpellMgr.GetLastSpellInChain(spellId_2));

    //if spells have exactly the same effect they cannot stack
    for (uint32 i = 0; i < 3; ++i)
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i]
            || spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i]
            || spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i]) // paladin resist aura
            return false; // need itemtype check? need an example to add that check

    // Specific spell family spells
    switch (spellInfo_1->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch (spellInfo_2->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:                   // same family case
                {
                    // Possess visual and Possess
                    if ((spellInfo_1->Id == 23014 && spellInfo_2->Id == 19832) ||
                            (spellInfo_2->Id == 23014 && spellInfo_1->Id == 19832))
                        return false;

                    break;
                }
                default: break;
            }
            break;
            default: break;
    }

    return true;
}

bool SpellMgr::IsCorrectFlameStrikeSpells(uint32 const spellId_1, uint32 const spellId_2) {

    //If Flamestrike has same ranks don't stack
    if (spellId_1 == spellId_2) {
        return false;
    }

    bool isFirstTrue = false;
    bool isSecondTrue = false;

    // Check that both spells are Flamestrike
    for (auto rank : all_Flamestrike_ranks) {
        if (spellId_1 == rank) {
            isFirstTrue = true;
        }
        if (spellId_2 == rank) {
            isSecondTrue = true;
        }
    }
    return (isFirstTrue && isSecondTrue);
}

bool SpellMgr::IsSpecialStackCase(SpellEntry const *spellInfo_1, SpellEntry const *spellInfo_2, bool sameCaster, bool recur)
{
    // put here all spells that should stack, but accoriding to rules in method IsNoStackSpellDueToSpell don't stack
    uint32 spellId_1 = spellInfo_1->Id;
    uint32 spellId_2 = spellInfo_2->Id;

    // Check on different ranks of Flamestrike stacking (if both spells is flamestrike)
    if (sameCaster && IsCorrectFlameStrikeSpells(spellId_1, spellId_2)) {
        return true;
    }

    // judgement of light stacks with judgement of wisdom
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo_1->SpellFamilyFlags & 0x80000 && spellInfo_1->SpellIconID == 299 // light
            && spellInfo_2->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo_2->SpellFamilyFlags & 0x80000 && spellInfo_2->SpellIconID == 206) // wisdom
        return !sameCaster;

    // Dragonmaw Illusion - should stack with everything ?
    if (spellId_1 == 40214 || spellId_2 == 40214)
        return true;

    // hourglass of unraveller stacks with blood fury
    if (spellId_1 == 33649 && spellId_2 == 20572)
        return true;

    if (spellId_1 == 22620 && spellId_2 == 22618)
        return true; // force reactive disc

    // Sextant of Unstable Currents, Shiffar's Nexus-Hornand, Scryer's Bloodgem and Band of the Ethernal Sage stacks with each other
    if ((spellId_1 == 38348 || spellId_1 == 34321 || spellId_1 == 35084 || spellId_1 == 35337) &&
            (spellId_2 == 38348 || spellId_2 == 34321 || spellId_2 == 35084 || spellId_2 == 35337) &&
            (spellId_1 != spellId_2))
        return true;

    // Warlord's Rage for Warlord Kalithresh event in Steamvault
    if (spellId_1 == 36453 && spellId_2 == 37076)
        return true;

    // Scroll of Agility and Idol of Terror
    if (spellId_1 == 43738 && (spellId_2 == 8115 || spellId_2 == 8116 || spellId_2 == 8117 || spellId_2 == 12174 || spellId_2 == 33077))
        return true;

    // Enh shaman t6 bonus proc and t6 trinket proc
    if(spellId_1 == 40466 && spellId_2 == 38430)
        return true;

    // wound poison & blood fury
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_ROGUE && spellInfo_1->SpellFamilyFlags == 268435456 &&
        spellInfo_2->SpellVisual == 47 && spellInfo_2->SpellIconID == 1662)
        return true;

    if (recur)
        return SpellMgr::IsSpecialStackCase(spellInfo_2, spellInfo_1, sameCaster, false);

    return false;
}

bool SpellMgr::IsSpecialNoStackCase(SpellEntry const *spellInfo_1, SpellEntry const *spellInfo_2, bool sameCaster, bool recur)
{
    // put here all spells that should NOT stack, but accoriding to rules in method IsNoStackSpellDueToSpell stack

    // Sunder Armor effect doesn't stack with Expose Armor
    if (spellInfo_1->SpellFamilyName == SPELLFAMILY_WARRIOR && spellInfo_1->SpellFamilyFlags & 0x4000L
        && spellInfo_2->SpellFamilyName == SPELLFAMILY_ROGUE && spellInfo_2->SpellFamilyFlags & 0x80000L)
        return true;

    // Power Infusion
    if (spellInfo_1->Id == 10060)
    {
        switch (spellInfo_2->Id)
        {
            // Icy Veins
            case 12472:
            // Heroism
            case 32182:
            // Bloodlust
            case 2825:
                return true;
        }
    }

    // Mangle Bear-Cat
    if (spellInfo_1->SpellIconID == 2312 && spellInfo_2->SpellIconID == 2312 && spellInfo_1->SpellFamilyFlags && spellInfo_2->SpellFamilyFlags)
        return true;

    // Scrolls no stack case
    if (spellInfo_1->AttributesCu & SPELL_ATTR_CU_NO_SCROLL_STACK && spellInfo_2->AttributesCu & SPELL_ATTR_CU_NO_SCROLL_STACK)
    {
        // if it's same stat
        if (spellInfo_1->EffectMiscValue[0] == spellInfo_2->EffectMiscValue[0])
            return true;
    }

    if (spellInfo_1->Id == 45848 && (spellInfo_2->Id == 45737 || spellInfo_2->Id == 45641))
        return true; // shield of the blue removes fire bloom and flame dart

    if (recur)
        return SpellMgr::IsSpecialNoStackCase(spellInfo_2, spellInfo_1, sameCaster, false);

    return false;
}

bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return SpellMgr::IsProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return false;

    if (spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return SpellMgr::IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return SpellMgr::IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

bool SpellMgr::IsSplashBuffAura(SpellEntry const* spellInfo)
{
    for (uint8 i = 0; i < 3; i++)
    {
        if (SpellMgr::IsPositiveEffect(spellInfo->Id, i))
        {
            if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
               return true;

            if (spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_PARTY_CASTER)
                return true;

            if (spellInfo->EffectImplicitTargetB[i] == TARGET_UNIT_AREA_PARTY_SRC)
                return true;

            if (spellInfo->EffectImplicitTargetB[i] == TARGET_UNIT_AREA_PARTY_DST)
                return true;
        }

    }
    return false;
}

SpellEntry const* SpellMgr::SelectAuraRankForPlayerLevel(SpellEntry const* spellInfo, uint32 playerLevel) const
{
    // ignore passive spells
    if (SpellMgr::IsPassiveSpell(spellInfo->Id))
        return spellInfo;

    bool needRankSelection = false;
    for (int i=0;i<3;i++)
    {
        if (SpellMgr::IsPositiveEffect(spellInfo->Id, i) && (
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA ||
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY
           ))
        {
            needRankSelection = true;
            break;
        }
    }

    // not required
    if (!needRankSelection)
        return spellInfo;

    for (uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellEntry const *nextSpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(nextSpellId);
        if (!nextSpellEntry)
            break;

        // if found appropriate level
        if (playerLevel + 10 >= nextSpellEntry->spellLevel)
            return nextSpellEntry;

        // one rank less then
    }

    // not found
    return NULL;
}

void SpellMgr::LoadSpellRequired()
{
    mSpellsReqSpell.clear();                                   // need for reload case
    mSpellReq.clear();                                         // need for reload case

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT spell_id, req_spell from spell_required");

    if ( ! result )
    {
        //BarGoLink bar(1);
        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 spell required records");
        sLog.outLog(LOG_DB_ERR, "`spell_required` table is empty!");
        return;
    }
    uint32 rows = 0;

    //BarGoLink bar(result->GetRowCount());
    do
    {
        //bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();
        uint32 spell_req = fields[1].GetUInt32();

        mSpellsReqSpell.insert (std::pair<uint32, uint32>(spell_req, spell_id));
        mSpellReq[spell_id] = spell_req;
        ++rows;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell required records", rows);
}

struct SpellRankEntry
{
    uint32 SkillId;
    char const *SpellName;
    uint32 DurationIndex;
    uint32 RangeIndex;
    uint32 SpellVisual;
    uint32 ProcFlags;
    uint64 SpellFamilyFlags;
    uint32 TargetAuraState;
    uint32 ManaCost;

    bool operator()(const SpellRankEntry& _Left, const SpellRankEntry& _Right) const
    {
        if (_Left.SkillId != _Right.SkillId)
            return _Left.SkillId < _Right.SkillId;

        if (std::strcmp(_Left.SpellName, _Right.SpellName) != 0)
            return std::strcmp(_Left.SpellName, _Right.SpellName) < 0;

        if (_Left.ProcFlags != _Right.ProcFlags)
            return _Left.ProcFlags < _Right.ProcFlags;

        if (_Left.SpellFamilyFlags != _Right.SpellFamilyFlags)
            return _Left.SpellFamilyFlags < _Right.SpellFamilyFlags;

        if ((_Left.SpellVisual != _Right.SpellVisual) && (!_Left.SpellVisual || !_Right.SpellVisual))
            return _Left.SpellVisual < _Right.SpellVisual;

        if ((_Left.ManaCost != _Right.ManaCost) && (!_Left.ManaCost || !_Right.ManaCost))
            return _Left.ManaCost < _Right.ManaCost;

        if ((_Left.DurationIndex != _Right.DurationIndex) && (!_Left.DurationIndex || !_Right.DurationIndex))
            return _Left.DurationIndex < _Right.DurationIndex;

        if ((_Left.RangeIndex != _Right.RangeIndex) && (!_Left.RangeIndex || !_Right.RangeIndex || _Left.RangeIndex == 1 || _Right.RangeIndex == 1))
            return _Left.RangeIndex < _Right.RangeIndex;

        return _Left.TargetAuraState < _Right.TargetAuraState;
    }
};

struct SpellRankValue
{
    uint32 Id;
    char const *Rank;
};

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case

    std::vector<uint32> ChainedSpells;
    for (uint32 ability_id=0;ability_id<sSkillLineAbilityStore.GetNumRows();ability_id++)
    {
        SkillLineAbilityEntry const *AbilityInfo=sSkillLineAbilityStore.LookupEntry(ability_id);
        if (!AbilityInfo)
            continue;
        if (AbilityInfo->spellId==20154) //exception to these rules (not needed in 3.0.3)
            continue;
        if (!AbilityInfo->forward_spellid)
            continue;
        ChainedSpells.push_back(AbilityInfo->forward_spellid);
    }

    std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry> RankMap;

    for (uint32 ability_id=0;ability_id<sSkillLineAbilityStore.GetNumRows();ability_id++)
    {
        SkillLineAbilityEntry const *AbilityInfo=sSkillLineAbilityStore.LookupEntry(ability_id);
        if (!AbilityInfo)
            continue;

        //get only spell with lowest ability_id to prevent doubles
        uint32 spell_id=AbilityInfo->spellId;
        if (spell_id == 20154) //exception to these rules (not needed in 3.0.3)
            continue;

        bool found=false;
        for (uint32 i=0; i<ChainedSpells.size(); i++)
        {
           if (ChainedSpells.at(i)==spell_id)
               found=true;
        }
        if (found)
            continue;

        if (mSkillLineAbilityMap.lower_bound(spell_id)->second->id!=ability_id)
            continue;
        SpellEntry const *spellinfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
        if (!spellinfo)
            continue;
        std::string sRank = spellinfo->Rank[sWorld.GetDefaultDbcLocale()];
        if (sRank.empty())
            continue;
        //exception to polymorph spells-make pig and turtle other chain than sheep
        if ((spellinfo->SpellFamilyName==SPELLFAMILY_MAGE) && (spellinfo->SpellFamilyFlags & 0x1000000) && (spellinfo->SpellIconID!=82))
             continue;

        //if (spellinfo->Id != 1513 && spellinfo->Id != 1528 && spellinfo->Id != 14326)
        //    continue;

        SpellRankEntry entry;
        SpellRankValue value;
        entry.SkillId=AbilityInfo->skillId;
        entry.SpellName= spellinfo->SpellName[sWorld.GetDefaultDbcLocale()];
        entry.DurationIndex= spellinfo->DurationIndex;
        entry.RangeIndex= spellinfo->rangeIndex;
        entry.ProcFlags= spellinfo->procFlags;
        entry.SpellFamilyFlags= spellinfo->SpellFamilyFlags;
        entry.TargetAuraState= spellinfo->TargetAuraState;
        entry.SpellVisual= spellinfo->SpellVisual;
        entry.ManaCost= spellinfo->manaCost;

        for (;;)
        {
            AbilityInfo=mSkillLineAbilityMap.lower_bound(spell_id)->second;
            value.Id=spell_id;
            value.Rank= spellinfo->Rank[sWorld.GetDefaultDbcLocale()];
            RankMap.insert(std::pair<SpellRankEntry, SpellRankValue>(entry,value));
            spell_id=AbilityInfo->forward_spellid;
            spellinfo =sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
            if (!spellinfo)
                break;
        }
    }

    //BarGoLink bar(RankMap.size());

    uint32 count=0;

    for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr = RankMap.begin();itr!=RankMap.end();)
    {
        SpellRankEntry entry = itr->first;

        std::multimap<std::string, std::multimap<SpellRankEntry, SpellRankValue, SpellRankEntry>::iterator> RankErrorMap;

        for (std::multimap<SpellRankEntry, SpellRankValue, SpellRankEntry>::iterator itr2 = RankMap.lower_bound(entry); itr2 != RankMap.upper_bound(entry); itr2++)
        {
            RankErrorMap.insert(std::pair<std::string, std::multimap<SpellRankEntry, SpellRankValue, SpellRankEntry>::iterator>(itr2->second.Rank, itr2));
        }

        for (std::multimap<std::string, std::multimap<SpellRankEntry, SpellRankValue, SpellRankEntry>::iterator>::iterator itr2 = RankErrorMap.begin(); itr2 != RankErrorMap.end(); )
        {
            std::string err_entry = itr2->first;
            uint32 rank_count = RankErrorMap.count(itr2->first);

            if (rank_count > 1)
            {
                for (itr2 = RankErrorMap.lower_bound(err_entry); itr2 != RankErrorMap.upper_bound(err_entry); itr2++)
                {
                    sLog.outDebug("There is a duplicate rank entry (%s) for spell: %u", itr2->first.c_str(), itr2->second->second.Id);
                    sLog.outDebug("Spell %u removed from chain data.", itr2->second->second.Id);
                    RankMap.erase(itr2->second);
                    itr = RankMap.lower_bound(entry);
                }
            }
            else
            {
                itr2++;
            }
        }
        //do not proceed for spells with less than 2 ranks
        uint32 spell_max_rank=RankMap.count(entry);
        if (spell_max_rank<2)
        {
            itr=RankMap.upper_bound(entry);
            continue;
        }

        itr=RankMap.upper_bound(entry);

        //order spells by spells by spellLevel
        std::list<uint32> RankedSpells;
        uint32 min_spell_lvl=0;
        std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator min_itr;
        for (;RankMap.count(entry);)
        {
            for (std::multimap<SpellRankEntry, SpellRankValue,SpellRankEntry>::iterator itr2 = RankMap.lower_bound(entry);itr2!=RankMap.upper_bound(entry);itr2++)
            {
                SpellEntry const * spellinfo =sSpellTemplate.LookupEntry<SpellEntry>(itr2->second.Id);
                if (spellinfo->spellLevel<min_spell_lvl || itr2==RankMap.lower_bound(entry))
                {
                    min_spell_lvl= spellinfo->spellLevel;
                    min_itr=itr2;
                }
            }
            RankedSpells.push_back(min_itr->second.Id);
            RankMap.erase(min_itr);
        }

        //use data from talent.dbc
        uint16 talent_id=0;
        for (std::list<uint32>::iterator itr2 = RankedSpells.begin();itr2!=RankedSpells.end();)
        {
            if (TalentSpellPos const* TalentPos=GetTalentSpellPos(*itr2))
            {
                talent_id=TalentPos->talent_id;
                RankedSpells.erase(itr2);
                itr2 = RankedSpells.begin();
            }
            else
                itr2++;
        }
        if (talent_id)
        {
            TalentEntry const *TalentInfo = sTalentStore.LookupEntry(talent_id);
            for (uint8 rank=5;rank;rank--)
            {
                if (TalentInfo->RankID[rank-1])
                    RankedSpells.push_front(TalentInfo->RankID[rank-1]);
            }
        }

        count++;

        itr=RankMap.upper_bound(entry);
        uint32 spell_rank=1;
        for (std::list<uint32>::iterator itr2 = RankedSpells.begin(); itr2 != RankedSpells.end(); spell_rank++)
        {
            uint32 spell_id = *itr2;
            mSpellChains[spell_id].rank = spell_rank;
            mSpellChains[spell_id].first = RankedSpells.front();
            mSpellChains[spell_id].last = RankedSpells.back();
            mSpellChains[spell_id].cur = *itr2;

            itr2++;
            if (spell_rank < 2)
                mSpellChains[spell_id].prev = 0;

            if (spell_id == RankedSpells.back())
                mSpellChains[spell_id].next = 0;
            else
            {
                mSpellChains[*itr2].prev = spell_id;
                mSpellChains[spell_id].next = *itr2;
            }
        }
    }

    // BIG UGLY PIECE OF CODE ! BUT WORKS :p
    // Bear Form
    uint32 spell_id = 5487;
    mSpellChains[spell_id].prev = 0;
    mSpellChains[spell_id].next = 9634;
    mSpellChains[spell_id].first = 5487;
    mSpellChains[spell_id].last = 9634;
    mSpellChains[spell_id].rank = 1;

    spell_id = 9634;
    mSpellChains[spell_id].prev = 5487;
    mSpellChains[spell_id].next = 0;
    mSpellChains[spell_id].first = 5487;
    mSpellChains[spell_id].last = 9634;
    mSpellChains[spell_id].rank = 2;

    // Flight form
    spell_id = 33943;
    mSpellChains[spell_id].prev = 0;
    mSpellChains[spell_id].next = 40120;
    mSpellChains[spell_id].first = 33943;
    mSpellChains[spell_id].last = 40120;
    mSpellChains[spell_id].rank = 1;

    spell_id = 40120;
    mSpellChains[spell_id].prev = 33943;
    mSpellChains[spell_id].next = 0;
    mSpellChains[spell_id].first = 33943;
    mSpellChains[spell_id].last = 40120;
    mSpellChains[spell_id].rank = 2;

    // Blessing of Kings
    spell_id = 20217;
    mSpellChains[spell_id].prev = 0;
    mSpellChains[spell_id].next = 25898;
    mSpellChains[spell_id].first = 20217;
    mSpellChains[spell_id].last = 25898;
    mSpellChains[spell_id].rank = 1;

    // Greater Blessing of Kings
    spell_id = 25898;
    mSpellChains[spell_id].prev = 20217;
    mSpellChains[spell_id].next = 0;
    mSpellChains[spell_id].first = 20217;
    mSpellChains[spell_id].last = 25898;
    mSpellChains[spell_id].rank = 2;

    // Greater Blessing of Sanctuary I
    spell_id = 25899;
    mSpellChains[spell_id].prev = 27168;    // BoS V
    mSpellChains[spell_id].next = 27169;    // GBoS II
    mSpellChains[spell_id].first = 20911;   // BoS I
    mSpellChains[spell_id].last = 27169;    // GBoS II
    mSpellChains[spell_id].rank = 6;

    // link BoS V with GBoS I
    mSpellChains[27168].next = spell_id;

    // Greater Blessing of Sanctuary II
    spell_id = 27169;
    mSpellChains[spell_id].prev = 25899;    // GBoS I
    mSpellChains[spell_id].next = 0;        // none
    mSpellChains[spell_id].first = 20911;   // BoS I
    mSpellChains[spell_id].last = 27169;    // GBoS II
    mSpellChains[spell_id].rank = 7;

    // set GBoS II as last for all BoS ranks
    mSpellChains[20911].last = spell_id;
    mSpellChains[20912].last = spell_id;
    mSpellChains[20913].last = spell_id;
    mSpellChains[20914].last = spell_id;
    mSpellChains[27168].last = spell_id;

    // Primal Fury talent -> Itself and its triggered spells must not be in chains
    mSpellChains.erase(37116);
    mSpellChains.erase(37117);
    mSpellChains.erase(16958);
    mSpellChains.erase(16961);
    mSpellChains.erase(16952);
    mSpellChains.erase(16954);
    // Primal Fury fix end

	// Druid Mangle
	spell_id = 33876;
	mSpellChains[spell_id].prev = 0;
	mSpellChains[spell_id].next = 33982;
	mSpellChains[spell_id].first = 33876;
	mSpellChains[spell_id].last = 33983;
	mSpellChains[spell_id].rank = 1;

	spell_id = 33982;
	mSpellChains[spell_id].prev = 33876;
	mSpellChains[spell_id].next = 33983;
	mSpellChains[spell_id].first = 33876;
	mSpellChains[spell_id].last = 33983;
	mSpellChains[spell_id].rank = 2;

	spell_id = 33983;
	mSpellChains[spell_id].prev = 33982;
	mSpellChains[spell_id].next = 0;
	mSpellChains[spell_id].first = 33876;
	mSpellChains[spell_id].last = 33983;
	mSpellChains[spell_id].rank = 3;

	spell_id = 33878;
	mSpellChains[spell_id].prev = 0;
	mSpellChains[spell_id].next = 33986;
	mSpellChains[spell_id].first = 33878;
	mSpellChains[spell_id].last = 33987;
	mSpellChains[spell_id].rank = 1;

	spell_id = 33986;
	mSpellChains[spell_id].prev = 33878;
	mSpellChains[spell_id].next = 33987;
	mSpellChains[spell_id].first = 33878;
	mSpellChains[spell_id].last = 33987;
	mSpellChains[spell_id].rank = 2;

	spell_id = 33987;
	mSpellChains[spell_id].prev = 33986;
	mSpellChains[spell_id].next = 0;
	mSpellChains[spell_id].first = 33878;
	mSpellChains[spell_id].last = 33987;
	mSpellChains[spell_id].rank = 3;

//uncomment these two lines to print yourself list of spell_chains on startup
//    for (UNORDERED_MAP<uint32, SpellChainNode>::iterator itr=mSpellChains.begin();itr!=mSpellChains.end();itr++)
//       sLog.outString("Id: %u, Rank: %d , %s",itr->first,itr->second.rank, sSpellTemplate.LookupEntry<SpellEntry>(itr->first)->Rank[sWorld.GetDefaultDbcLocale()]);

    sLog.outString();
    sLog.outString(">> Loaded %u spell chains",count);
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < sSpellTemplate.GetMaxEntry(); ++spell)
    {
        SpellEntry const* entry = sSpellTemplate.LookupEntry<SpellEntry>(spell);

        if (!entry)
            continue;

        for (int i = 0; i < 3; ++i)
        {
            if (entry->Effect[i] == SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill = entry->EffectMiscValue[i];
                if (dbc_node.skill != SKILL_RIDING)
                    dbc_node.value = 1;
                else
                    dbc_node.value = entry->CalculateSimpleValue(i)*75;
                dbc_node.maxvalue = entry->CalculateSimpleValue(i)*75;

                SpellLearnSkillNode const* db_node = GetSpellLearnSkill(spell);

                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Learn Skills from DBC", dbc_count);
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry, SpellID FROM spell_learn_spell");
    if (!result)
    {
        //BarGoLink bar(1);
        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 spell learn spells");
        sLog.outLog(LOG_DB_ERR, "`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    //BarGoLink bar(result->GetRowCount());
    do
    {
        //bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.autoLearned= false;

        if (!sSpellTemplate.LookupEntry<SpellEntry>(spell_id))
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_learn_spell` does not exist",spell_id);
            continue;
        }

        if (!sSpellTemplate.LookupEntry<SpellEntry>(node.spell))
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_learn_spell` does not exist",node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while (result->NextRow());

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for (uint32 spell = 0; spell < sSpellTemplate.GetMaxEntry(); ++spell)
    {
        SpellEntry const* entry = sSpellTemplate.LookupEntry<SpellEntry>(spell);

        if (!entry)
            continue;

        for (int i = 0; i < 3; ++i)
        {
            if (entry->Effect[i]==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.autoLearned = true;

                SpellLearnSpellMap::const_iterator db_node_begin = GetBeginSpellLearnSpell(spell);
                SpellLearnSpellMap::const_iterator db_node_end   = GetEndSpellLearnSpell(spell);

                bool found = false;
                for (SpellLearnSpellMap::const_iterator itr = db_node_begin; itr != db_node_end; ++itr)
                {
                    if (itr->second.spell == dbc_node.spell)
                    {
                        sLog.outLog(LOG_DB_ERR, "Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spell,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if (!found)                                  // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.outString();
    sLog.outString(">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count);
}

void SpellMgr::LoadSpellScriptTarget()
{
    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT entry,type,targetEntry FROM spell_script_target");

    if (!result)
    {
        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded 0 spell script target");
        sLog.outLog(LOG_DB_ERR, "`spell_script_target` table is empty!");
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        //bar.step();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();

        SpellEntry const* spellProto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

        if (!spellProto)
        {
            sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.",spellId,targetEntry);
            continue;
        }

        if (type >= MAX_SPELL_TARGET_TYPE)
        {
            sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.",type,targetEntry);
            continue;
        }

        switch (type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if (targetEntry==0)
                    break;

                if (!sGOStorage.LookupEntry<GameObjectInfo>(targetEntry))
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: gameobject template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
            default:
            {
                //players
                /*if(targetEntry==0)
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: target entry == 0 for not GO target type (%u).",type);
                    continue;
                }*/
                if (targetEntry && !sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry))
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target`: creature template entry %u does not exist.",targetEntry);
                    continue;
                }
                const CreatureInfo* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry);

                if (spellId == 30427 && !cInfo->SkinLootId)
                {
                    sLog.outLog(LOG_DB_ERR, "Table `spell_script_target` has creature %u as a target of spellid 30427, but this creature has no skinlootid. Gas extraction will not work!", cInfo->Entry);
                    continue;
                }
                break;
            }
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId,SpellTargetEntry(SpellScriptTargetType(type),targetEntry)));

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Script Targets", count);
}

void SpellMgr::LoadSpellPetAuras()
{
    mSpellPetAuraMap.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                       0      1    2
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT spell, pet, aura FROM spell_pet_auras");
    if (!result)
    {

        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u spell pet auras", count);
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        uint16 spell = fields[0].GetUInt16();
        uint16 pet = fields[1].GetUInt16();
        uint16 aura = fields[2].GetUInt16();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find(spell);
        if (itr != mSpellPetAuraMap.end())
        {
            itr->second.AddAura(pet, aura);
        }
        else
        {
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
            if (!spellInfo)
            {
                sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            int i = 0;
            for (; i < 3; ++i)
                if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA &&
                    spellInfo->EffectApplyAuraName[i] == SPELL_AURA_DUMMY) ||
                    spellInfo->Effect[i] == SPELL_EFFECT_DUMMY)
                    break;

            if (i == 3)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellEntry const* spellInfo2 = sSpellTemplate.LookupEntry<SpellEntry>(aura);
            if (!spellInfo2)
            {
                sLog.outLog(LOG_DB_ERR, "Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_PET, spellInfo->CalculateSimpleValue(i));
            mSpellPetAuraMap[spell] = pa;
        }

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u spell pet auras", count);
}

void SpellMgr::LoadCustomSpellItemEnchantments()
{
    SpellItemEnchantmentEntry* enchant;
    for (uint32 i = 0; i < sSpellItemEnchantmentStore.GetNumRows(); ++i)
    {
        enchant = (SpellItemEnchantmentEntry*)sSpellItemEnchantmentStore.LookupEntry(i);
        if (!enchant)
            continue;

        switch (i)
        {
            case 324:
                enchant->type[0] = ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL;
                break;
            default:
                break;
        }
    }
}

// set data in core for now
void SpellMgr::LoadSpellCustomAttr()
{
    SpellEntry *spellInfo;
    Season season = sWorld.getSeasonFromDB();
    for (uint32 i = 0; i < GetSpellStore()->GetMaxEntry(); ++i)
    {
        spellInfo = (SpellEntry*)GetSpellStore()->LookupEntry<SpellEntry>(i);
        if (!spellInfo)
            continue;
        spellInfo->AttributesCu = 0;

        bool auraSpell = true;
        for (uint32 j = 0; j < 3; ++j)
        {
            if (spellInfo->Effect[j])
                if (spellInfo->Effect[j] != SPELL_EFFECT_APPLY_AURA
                || SpellTargetType[spellInfo->EffectImplicitTargetA[j]] != TARGET_TYPE_UNIT_TARGET)
                //ignore target party for now
                {
                    auraSpell = false;
                    break;
                }
        }
        if (auraSpell)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_SPELL;

        for (uint32 j = 0; j < 3; ++j)
        {
            switch (spellInfo->EffectApplyAuraName[j])
            {
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                case SPELL_AURA_PERIODIC_LEECH:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_DOT;
                    break;
                case SPELL_AURA_PERIODIC_HEAL:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_HOT;
                    if (spellInfo->Mechanic == MECHANIC_BANDAGE) // bandages
                        spellInfo->Attributes |= SPELL_ATTR_NOT_SHAPESHIFT;
                    break;
                case SPELL_AURA_OBS_MOD_HEALTH:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_HOT;
                    /*if (spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) // food and drink
                        spellInfo->Attributes |= SPELL_ATTR_NOT_SHAPESHIFT;*/ // Maybe gonna need - Forbidden cast bandage and food in forms
                    break;
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_DECREASE_SPEED:
                    // Creature daze exception
                    if (spellInfo->Id == 1604)
                        break;
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                    break;
                case SPELL_AURA_MOD_POSSESS:
                case SPELL_AURA_MOD_CONFUSE:
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_MOD_FEAR:
                case SPELL_AURA_MOD_STUN:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_AURA_CC;
                    spellInfo->AttributesCu &= ~SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                    break;
                /*case SPELL_AURA_MOD_REGEN:
                case SPELL_AURA_MOD_POWER_REGEN:
                    if (spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) // food and drink
                        spellInfo->Attributes |= SPELL_ATTR_NOT_SHAPESHIFT;*/ // Maybe gonna need - Forbidden cast bandage and food in forms
                case SPELL_AURA_DAMAGE_SHIELD:
                    spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_DAMAGE_DOESNT_BREAK_AURAS;
                    break;
                default:
                    break;
            }

            switch (spellInfo->Effect[j])
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_DIRECT_DAMAGE;
                    break;
                case SPELL_EFFECT_CHARGE:
                case SPELL_EFFECT_CHARGE2:
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_CHARGE;
                    break;
                case SPELL_EFFECT_TRIGGER_SPELL:
                    if (spellInfo->Id == 39897)   // Mass Dispel should not have effect_trigger_missile? to be verified now
                        break;
                    if (IsPositionTarget(spellInfo->EffectImplicitTargetA[j]) ||
                        spellInfo->Targets & (TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION))
                        spellInfo->Effect[j] = SPELL_EFFECT_TRIGGER_MISSILE;
                    break;
                case SPELL_EFFECT_TELEPORT_UNITS:
                    if (spellInfo->EffectImplicitTargetA[j] == 17 && spellInfo->EffectImplicitTargetB[j] == 0)
                    {
                        spellInfo->EffectImplicitTargetA[j] = 1;
                        spellInfo->EffectImplicitTargetB[j] = 17;
                    }
                    break;
                case SPELL_EFFECT_HEAL:
                    if (spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && spellInfo->SpellFamilyFlags & 0x0000000000010000LL) // healthstone debug for 2 healthstones
                    {
                        spellInfo->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                        spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                        spellInfo->EffectTriggerSpell[1] = 54789;
                    }
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_DIRECT_DAMAGE;
                    break;
            }
        }

        if (spellInfo->SpellVisual == 3879)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_BACK;

        if ((spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags & 0x1000LL && spellInfo->SpellIconID == 494) || spellInfo->Id == 33745 /* Lacerate */)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;

        // Modify SchoolMask to allow them critically heal
        // Healthstones
        if (spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && spellInfo->SpellFamilyFlags & 0x10000LL)
            spellInfo->SchoolMask = SPELL_SCHOOL_MASK_SHADOW;

        // so channeled  spell can NOT be interrupted by movement :p
        if (spellInfo->Effect[0] == SPELL_EFFECT_STUCK)
        {
            if (IsChanneledSpell(spellInfo))
                spellInfo->ChannelInterruptFlags &= ~CHANNEL_INTERRUPT_FLAG_MOVEMENT;
            else
                spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_MOVEMENT;
        }

		// test, in most cases turning is caused by channeling, so spells just dont work
		spellInfo->ChannelInterruptFlags &= ~AURA_INTERRUPT_FLAG_TURNING;

        LoadCustomSpellCooldowns(spellInfo);

        if (spellInfo->HasApplyAura(SPELL_AURA_DAMAGE_SHIELD) ||
            spellInfo->HasApplyAura(SPELL_AURA_PERIODIC_LEECH) ||
            spellInfo->HasEffect(SPELL_EFFECT_HEALTH_LEECH))
            spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;

        // Soulshatter 32835 is triggered by a dummy AoE effect, cant check for AoE for percent effect
        if (spellInfo->HasEffect(SPELL_EFFECT_MODIFY_THREAT_PERCENT) ||
            (spellInfo->HasEffect(SPELL_EFFECT_THREAT) && sSpellMgr.IsAreaOfEffectSpell(spellInfo)))
            spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_NOT_CONSIDERED_HIT;

        // if spellInfo has mechanic - then its effects SHOULD NOT have same mechanic.
        // override spell effects mechanic to none if spell has same overall mechanic
        // This is tested - for sure this code is needed
        if (spellInfo->Mechanic)
        {
            for (uint32 j = 0; j < 3; ++j)
            {
                if (spellInfo->EffectMechanic[j] == spellInfo->Mechanic)
                    spellInfo->EffectMechanic[j] = MECHANIC_NONE;
            }
        }
        // this is done for the reason that mechanic is already worked out as SPELL mechanic. Leaving effect mechanic the same will cause double-effect on spell

        if (spellInfo->SpellIconID == 109 && spellInfo->SpellVisual == 192)
            spellInfo->AttributesCu |= SPELL_ATTR_CU_BLOCK_STEALTH;

        switch (spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                switch (spellInfo->Id)
                {
					case 38511: // vashj persuasion
						spellInfo->MaxAffectedTargets = 1;
						spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
						spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_TARGET_ENEMY;
						spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_TARGET_ENEMY;
						spellInfo->EffectImplicitTargetB[0] = 0;
						spellInfo->EffectImplicitTargetB[1] = 0;
						spellInfo->EffectImplicitTargetB[2] = 0;
					case 38442:
                        spellInfo->Attributes = 0;
                        spellInfo->AttributesEx = 0;
                        spellInfo->AttributesEx2 = 0;
                        spellInfo->AttributesEx3 = 0;
                        break;
                    case 25117:
                        spellInfo->spellLevel = 5;
                        break;
                    case 12699:
                        spellInfo->EffectImplicitTargetA[0] = TARGET_DST_TARGET_ENEMY;
                        spellInfo->EffectImplicitTargetB[0] = 0;
                        break;
                    case 19588:
                        spellInfo->Effect[1] = SPELL_EFFECT_SEND_EVENT;
                        spellInfo->EffectBasePoints[1] = 1;
                        spellInfo->EffectImplicitTargetA[1] = 0;
                        spellInfo->EffectImplicitTargetB[1] = 0;
                        spellInfo->EffectMiscValue[1] = 185500;
                        break;
                    case 34623:
                    case 34621:
                        spellInfo->rangeIndex = 14;
                        break;
                    case 36555:
                        spellInfo->DurationIndex = 3;
                        break;
                    case 33924:
                        spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                        spellInfo->EffectImplicitTargetB[0] = 0;
                        break;
                    case 38736:
                        spellInfo->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
                        spellInfo->EffectImplicitTargetA[0] = 38;
                        spellInfo->EffectImplicitTargetB[0] = 0;
                        spellInfo->EffectAmplitude[0] = 0;
                        spellInfo->EffectTriggerSpell[0] = 0;
                        spellInfo->Effect[1] = 6;
                        spellInfo->EffectBasePoints[1] = 1;
                        spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                        spellInfo->EffectImplicitTargetB[1] = 0;
                        spellInfo->EffectApplyAuraName[1] = 23;
                        spellInfo->EffectAmplitude[1] = 10000;
                        spellInfo->EffectTriggerSpell[1] = 38729;
                        break;
                    case 38729:
                        spellInfo->EffectMiscValue[0] = 1;
                        break;
                    case 38722:
                        spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                        break;
                    case 36652:
                        spellInfo->EffectRadiusIndex[0] = 20;
                        spellInfo->MaxAffectedTargets = 1;
                        break;
                    case 37462: // Chess Queen Alliance - Elemental Blast
                    case 37463: // Chess Queen Horde - Fireball
                        spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                        break;
                    case 30024: // remove interrupt flags for this spell
                        spellInfo->InterruptFlags = 0;
                        spellInfo->AuraInterruptFlags = 0;
                        spellInfo->Attributes |= SPELL_ATTR_CASTABLE_WHILE_SITTING;
                        break;
                    case 29969: // remove interrupt flags for this spell
                        spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_INTERRUPT;
                        break;
                        break;
                    case 785:
                        spellInfo->MaxAffectedTargets = 1;
                        break;
                    case 32349:
                    case 32353:
                        spellInfo->RecoveryTime = 1000;
                        break;
                    case 30815:
                        spellInfo->MaxAffectedTargets = 1;
                        break;
                    case 30952:
                    case 34536:
                        spellInfo->EffectImplicitTargetA[0] = 25;
                        spellInfo->EffectImplicitTargetB[0] = 38;
                        break;
                    case 6714:
                        spellInfo->rangeIndex = 1;
                        break;
                    case 15582:
                        spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_FROM_BEHIND;
                        break;
                    case 34156:
                        spellInfo->EffectImplicitTargetA[0] = 38;
                        spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                        spellInfo->AuraInterruptFlags = 0;
                        spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_NOT_VICTIM;
                        spellInfo->ChannelInterruptFlags = 0;
                        break;
                    case 34173:
                        spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                        break;
                    case 30846:
                    case 32784:
                        spellInfo->Effect[0] = SPELL_EFFECT_SUMMON_OBJECT_SLOT1;
                        spellInfo->EffectBasePoints[0] = 1;
                        spellInfo->EffectImplicitTargetA[0] = 47;
                        spellInfo->EffectImplicitTargetB[0] = 0;
                        break;
                    case 39089: // Polarity Shift - charges. Fix alt+f4 abuse.
                    case 29659:
                    case 39092:
                    case 29660:
                        spellInfo->DurationIndex = 3;
                        break;
                    case 45716: // Torch Tossing Training
                        spellInfo->DurationIndex = 22; // 45 sec
                        spellInfo->Effect[2] = SPELL_EFFECT_APPLY_AURA;
                        spellInfo->EffectApplyAuraName[2] = SPELL_AURA_DUMMY;
                        break;
                    case 46630: // Torch Tossing Practice
                        spellInfo->Effect[2] = SPELL_EFFECT_APPLY_AURA;
                        spellInfo->EffectApplyAuraName[2] = SPELL_AURA_DUMMY;
                        break;
                    case 37666: // FixMe: Spell with SPELL_EFFECT_TRIGGER_MISSILE
                    case 32191: // FixMe: Spell with SPELL_EFFECT_TRIGGER_MISSILE
                        spellInfo->Effect[1] = SPELL_EFFECT_SCRIPT_EFFECT;
                        spellInfo->EffectImplicitTargetA[1] = 22;
                        spellInfo->EffectImplicitTargetB[1] = 7;
                        break;
                    case 45732: // FixMe: Spell with SPELL_EFFECT_TRIGGER_MISSILE
                        spellInfo->Effect[0] = SPELL_EFFECT_SCRIPT_EFFECT;
                        spellInfo->EffectRadiusIndex[0] = 7;
                        break;
                    case 42821:
                    case 42818:
                        spellInfo->rangeIndex = 6;
                        break;
                    case 42380:
                        spellInfo->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE_PERCENT;
                        spellInfo->EffectBasePoints[0] = 10;
                        spellInfo->EffectBaseDice[0] = 10;
                        spellInfo->DmgMultiplier[0] = 1;
                        break;
                    case 43301:
                        spellInfo->CastingTimeIndex = 5;
                        break;
                    case 43550:
                        spellInfo->MaxAffectedTargets = 1;
                        break;
                    case 43734:
                        spellInfo->EffectImplicitTargetA[0] = 1;
                        spellInfo->EffectImplicitTargetB[0] = 0;
                        break;
                    case 42389:
                        spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_TARGET_ENEMY;
                        break;
                    case 31344:
                        spellInfo->EffectRadiusIndex[0] = 12; // 100yards instead of 50000?!
                        break;
                    case 31480:
                        spellInfo->Effect[1] = 2;
                        break;
                    case 40090:
                        spellInfo->AttributesEx |= SPELL_ATTR_EX_CHANNELED_1;
                        break;
                    case 41296:
                        spellInfo->EffectTriggerSpell[1] = 0;
                        break;
                    case 40243:
                        spellInfo->MaxAffectedTargets = 5;
                        break;
                    case 38575:
                        spellInfo->EffectBasePoints[0] = 1500;
                        break;
                    case 36819:
                        spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                        spellInfo->EffectImplicitTargetB[0] = 0;
                        break;
                    case 36797:
                    {
                        spellInfo->MaxAffectedTargets = 1;
                        for(uint8 i = 0; i<3; i++)
                        {
                            spellInfo->EffectImplicitTargetA[i] = TARGET_UNIT_TARGET_ENEMY;
                            spellInfo->EffectImplicitTargetB[i] = 0;
                        }
                        break;
                    }
                    case 37036:
                        spellInfo->Effect[0] = 2;
                        break;
                    case 26681: // Love is in the Air: Cologne
                    {
                        spellInfo->EffectApplyAuraName[0] = 23; // periodic trigger spell
                        spellInfo->EffectAmplitude[0] = 5000; // 5 sec
                        spellInfo->EffectTriggerSpell[0] = 27741; // Love is in the Air
                        break;
                    }
                    case 26682: // Love is in the Air: Perfume
                    {
                        spellInfo->EffectApplyAuraName[0] = 23; // periodic trigger spell
                        spellInfo->EffectAmplitude[0] = 5000; // 5 sec
                        spellInfo->EffectTriggerSpell[0] = 27741; // Love is in the Air
                        break;
                    }
                    case 27741: // Love is in the Air
                    {
                        spellInfo->EffectImplicitTargetA[0] = 31; // All Friendly units in area
                        spellInfo->EffectRadiusIndex[0] = 29; // 6 yd
                        spellInfo->DurationIndex = 32; // 6 sec.
                        spellInfo->EffectApplyAuraName[0] = 0;
                        break;
                    }
                    case 52009: // Goblin Rocket Launcher
                        spellInfo->EffectMiscValue[0] = 20865;
                        break;
                    case 15852:
                        spellInfo->Dispel = DISPEL_NONE;
                        break;
                    case 46337: // Crab disguise
                        spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_CAST;
                        break;
                    case 34171: // Underbat - Tentacle Lash
                    case 37956:
                        spellInfo->AttributesEx |= SPELL_ATTR_EX_UNK9;
                        spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_FROM_BEHIND;
                        break;
                    case 16613: // some quest spell spamming with non-existing triggered
                        spellInfo->Effect[2] = 0;
                        break;
                    case 39280: // same here
                        spellInfo->Effect[1] = 0;
                        break;
                    case 38829: // arcatraz sentinels prevent spam
                        spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                        break;
                    case 26635: // berserking troll racial
                        spellInfo->EffectDieSides[0] = 1;
                        break;
                    case 40669: // egbert egg- run away spell
                    case 36200: // doomsaw 12 sec
                        spellInfo->DurationIndex = 29; // 12 secs, guess
                        break;
                    case 37674: // leotheras chaos blast
                        spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_BE_REFLECTED;
                        break;
                    case 37675: // leotheras the blind - chaos blast should be spell not ability
                        spellInfo->Attributes &= ~SPELL_ATTR_ABILITY;
                        break;
                    case 8690: // hearthstone - no haste bonus
                        spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                        break;
                    case 7620: // fishing - no haste bonus
                    case 7731:
                    case 7732:
                    case 18248:
                    case 33095:
                        spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                        break;
                    case 44586: // NPC prayer of mending
                        spellInfo->procFlags &= ~PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL;
                        break;
                    case 40176: // simon game spells
                    case 40177:
                    case 40178:
                    case 40179:
                        spellInfo->Targets |= TARGET_FLAG_GAMEOBJECT;
                        break;
                    case 30532: // karazhan chess teleport
                        spellInfo->rangeIndex = 6;
                        break;
                    case 18818: // skullflame shield flamestrike
                    case 7712:  // blazefury medalion
                    case 46365: // Fire Festival Fury
                        spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                        break;
                    case 9806: // faerie-fire-alikes
                    case 9991:
                    case 16432:
                    case 35325:
                    case 35328:
                    case 35329:
                    case 35331:
                        spellInfo->AttributesCu |= SPELL_ATTR_CU_BLOCK_STEALTH;
                        break;
                    case 45275:
                    case 42336: // bucket/torch visuals duration 60sec
                        spellInfo->DurationIndex = 3;
                        break;
                    case 44969: // energize crystal ward
                        spellInfo->RequiresSpellFocus = 1483;
                        break;
                    case 44999: // conversions on ioqd, set guardians duration to 12 sec and let them spawn multiple times
                        spellInfo->DurationIndex = 9;
                        spellInfo->RecoveryTime = 1;
                        break;
                    case 26194: // FIXME: pretbc spell scaling does not work with resistance correctly
                        spellInfo->Attributes &= ~SPELL_ATTR_LEVEL_DAMAGE_CALCULATION;
                        break;
                    case 46648: // no pull for cosmetic spell
                        spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                        break;
                    /*case 19832: // Possess - Razorgore the Untamed
                        spellInfo->DmgClass = 1;
                        spellInfo->PreventionType = SPELL_PREVENTION_TYPE_SILENCE;
                        spellInfo->Attributes = SPELL_ATTR_UNK18 | SPELL_ATTR_STOP_ATTACK_TARGET;
                        spellInfo->AttributesEx = SPELL_ATTR_EX_SUMMON_PET | SPELL_ATTR_EX_CHANNELED_1 | SPELL_ATTR_EX_UNK13 | SPELL_ATTR_EX_UNAUTOCASTABLE_BY_PET | SPELL_ATTR_EX_UNK26;
                        spellInfo->AttributesEx2 = SPELL_ATTR_EX2_CANT_TARGET_TAPPED | SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT;
                        spellInfo->AttributesEx4 = SPELL_ATTR_EX4_UNK11 | SPELL_ATTR_EX4_UNK29;
                        spellInfo->Dispel = 1;
                        spellInfo->Mechanic = MECHANIC_CHARM;
                        spellInfo->ChannelInterruptFlags = 0;
                        spellInfo->EffectBasePoints[0] = 73;
                        break;*/
                    case 23014:
                        spellInfo->ChannelInterruptFlags = 0;
                        break;
                    case 45885: // KJ shadow spike
                        spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                        spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_AREA_ENEMY_DST;
                        break;
                    case 46589: // Shadow spike, target destination set in aura::triggerspell
                        spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                        spellInfo->speed = 6;
                        break;
                    case 38112: // vashj shield generator
                        spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                        break;
                    case 45839: // KJ drake control, no haste
                    case 45838:
                        spellInfo->Attributes |= SPELL_ATTR_TRADESPELL;
                        break;
                    case 45072: // Arcane charges ignore los
                        spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                        break;
                    case 38544: // coax mamrot
                        spellInfo->EffectMiscValueB[0] = SUMMON_TYPE_POSESSED;
                        break;
                    case 7720: // summon effect
                        spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                        break;
                    case 45927: // Summon Friend
                        spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                        spellInfo->rangeIndex = 1;
                        break;
                }
                if (spellInfo->SpellIconID == 184 && spellInfo->Attributes == 4259840)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                else if (spellInfo->SpellIconID == 2367) // remove flag from steam tonk & crashin trashin racers
                    spellInfo->AttributesEx4 &= ~SPELL_ATTR_EX4_FORCE_TRIGGERED;

                break;
            }
            case SPELLFAMILY_DRUID:
            {
                if ((spellInfo->SpellFamilyFlags & 128) &&
                   ((spellInfo->EffectImplicitTargetA[0] == TARGET_DEST_DYNOBJ_ALLY) ||
                   (spellInfo->EffectImplicitTargetA[0] == TARGET_DEST_CHANNEL)))
                        spellInfo->EffectImplicitTargetA[0] = 0; // tranquility log spam

                switch (spellInfo->Id)
                {
                    case 16998: // Savage Fury
                    case 16999:
                        spellInfo->Effect[2] = 0;
                        break;
                    case 17768: //Wolfshead Helm - Wolfshead Helm Energy in cat form
                        spellInfo->EffectTriggerSpell[1] = 29940;
                        break;
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
            {
                if (spellInfo->Id == 16368)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                else if (spellInfo->SpellFamilyFlags & 0x800000LL) // wf attack
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                // Judgement & seal of Light
                if (spellInfo->SpellFamilyFlags & 0x100040000LL)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Seal of Righteousness trigger - already computed for parent spell
                else if (spellInfo->SpellIconID==25 && spellInfo->AttributesEx4 & 0x00800000LL)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_FIXED_DAMAGE;
                // Blessing of Sanctuary. normal and greater
                else if (spellInfo->SpellIconID == 1804 || spellInfo->SpellIconID == 29)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Devotion Aura
                else if (spellInfo->SpellFamilyFlags & 0x40 && spellInfo->SpellIconID == 291)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                else if (spellInfo->Id == 25997) // Eye for an eye
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                else if (spellInfo->Id == 43743) // improved Seal of Righteousness
                    spellInfo->Effect[1] = 0;

                if (spellInfo->SpellFamilyFlags & 0x0001040002200000LL) // seal of blood/command/holy shock
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                break;
            }
            case SPELLFAMILY_PRIEST:
            {
                // Mana Burn
                if (spellInfo->SpellFamilyFlags & 0x10LL && spellInfo->SpellIconID == 212)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Health Link T5 Hunter/Warlock bonus
                else if (spellInfo->Id == 37382)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Reflective Shield
                else if (spellInfo->Id == 33619) 
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                else if (!spellInfo->SpellFamilyFlags && spellInfo->SpellIconID == 237)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;

                // Divine Spirit/Prayer of Spirit
                else if (spellInfo->SpellFamilyFlags & 0x20)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                // Power Word: Fortitude/Prayer of Fortitude
                else if (spellInfo->SpellFamilyFlags & 0x08 && spellInfo->SpellVisual == 278)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                // mass dispel simplification - combine 3 spells limiting triggering
                else if (spellInfo->Id == 32375)
                {
                    spellInfo->Effect[1] = SPELL_EFFECT_DISPEL;
                    spellInfo->EffectRadiusIndex[1] = 18;
                    spellInfo->EffectMiscValue[1] = 1;
                    spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
                    spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_AREA_ENEMY_DST;
                    spellInfo->EffectRadiusIndex[2] = 13;
                    spellInfo->EffectTriggerSpell[2] = 39897;
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                }
                else if (spellInfo->Id == 32409) // shadow word: death back damage, unaffected by anything
                {
                    spellInfo->SpellFamilyFlags = 0;
                }
                else if(spellInfo->Id == 34303)
                {
                    spellInfo->MaxAffectedTargets = 1;
                }
                break;
            }
            case SPELLFAMILY_MAGE:
            {
                // Molten Armor
                if (spellInfo->SpellFamilyFlags & 0x800000000LL)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Arcane Intellect/Brilliance
                else if (spellInfo->SpellFamilyFlags & 0x0400)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                // some quest spell spamming with non-existing triggered
                else if (spellInfo->Id == 39280)
                    spellInfo->Effect[1] = 0;
                else if (spellInfo->Id == 33395) // elementals' freeze (blocks frost nova)
                    spellInfo->Category = 0;

                if (spellInfo->SpellFamilyFlags & 0x0000000800200080LL) // Arcane Missles / Blizzard / Molten Armor proc
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                break;
            }
            case SPELLFAMILY_WARLOCK:
            {
                // Drain Mana
                if (spellInfo->SpellFamilyFlags & 0x10LL && spellInfo->SpellIconID == 548)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Healthstone
                else if (spellInfo->SpellFamilyFlags & 0x10000LL)
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                // Death Coil
                else if (spellInfo->SpellVisual == 9152)
                    spellInfo->Attributes |= SPELL_ATTR_CANT_CANCEL;
                else if(spellInfo->Id == 30741)
                {
                    spellInfo->Effect[0] = 0;
                    /*spellInfo->EffectImplicitTargetA[0] = 22;
                    spellInfo->EffectImplicitTargetB[0] = 31;*/
                    spellInfo->EffectImplicitTargetA[1] = 22;
                    spellInfo->EffectImplicitTargetB[1] = 31;
                    spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                    spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT;
                }

                if (spellInfo->SpellIconID == 937 && spellInfo->SpellVisual == 781) // Hellfire Effect - patch 2.3.0 This spell no longer cause enemy spells to increase casting time
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_PUSHBACK;

                if (spellInfo->SpellFamilyFlags & 0x0000800000000000LL) // Seed of corruption (proc one from another)
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                if (spellInfo->Id == 45172) // BE guards shooting at flying players on isle, prevent spam
                    spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_AREA_ENEMY_DST;
                else if (spellInfo->Id == 34026) // kill command, all handled by first dummy effect
                    spellInfo->Effect[1] = 0;
                else if (spellInfo->Id == 13810) // frost trap effect ignore los
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                else if (spellInfo->Id == 1543 || spellInfo->Id == 28822) // flares
                {
                    spellInfo->AttributesCu |= SPELL_ATTR_CU_BLOCK_STEALTH;
                    spellInfo->speed = 15;
                    spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                }
                else if (spellInfo->Id == 3045 || spellInfo->Id == 36828) // Rapid Fire
                    spellInfo->speed = 0;
                if (spellInfo->SpellFamilyFlags & 0x0000200000000014LL) // trap effects
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            }
            case SPELLFAMILY_WARRIOR:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            case SPELLFAMILY_ROGUE:
                if (spellInfo->SpellFamilyFlags & 0x600000000LL) // mutilate
                    spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;

                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
        }

        // spellmodifiers spellattributes
        switch (i)
        {
            // Leotheras Chaos Blast
            case 37675:
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_BE_REFLECTED;
                break;
            // Leotheras Whirlwind debuff should stack
            case 37641:
                spellInfo->StackAmount = 20; 
                break;
            // Bloodwarder Vindicator
            case 37249:
                spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT | SPELL_INTERRUPT_FLAG_MOVEMENT;
                break;
			// bonfiers don't break stealth
			case 7902:
				spellInfo->AttributesEx |= SPELL_ATTR_EX_NOT_BREAK_CASTER_STEALTH;
				break;
			// Shade of Aran
			case 29954:
			case 29953:
			case 29991:
			case 29964:
			case 30035:
			case 29973:
			case 29963:
			case 29967:
			case 29975:
			case 30024:
			case 32453:
			case 29978:
			case 29969:
			case 29979:
			case 39567:
			case 29952:
			case 37054:
			case 38837:
			case 29960:
				spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
				break;
			case 30004: // Aran: Flame Wreath
				spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
				spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
				spellInfo->MaxAffectedTargets = 3;
				break;
			case 29962: // Summon Elemental (Shade of Aran)
			case 37053:
			case 37051:
			case 37052:
				spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
				spellInfo->rangeIndex = 6;
				break;
			case 29955: // Arcane Missiles (Shade of Aran)
				spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
				spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
				spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_TARGET_ENEMY;
				spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
				break;
			case 29961:
				spellInfo->EffectRadiusIndex[0] = 8;
				spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
				break;

            case 40953:
                spellInfo->EffectBasePoints[0] = 1500;
                break;
            case 31538:
                spellInfo->Effect[0] = SPELL_EFFECT_HEAL_PCT;
                spellInfo->EffectBasePoints[1] = 7;
                break;
            case 46008:
                spellInfo->MaxAffectedTargets = 5;
                break;
            case 46009:
            case 46284:
            case 46289:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_PUSHBACK;
                break;
            // case 408:
            case 1953:
            case 5232:
            case 5234:
            case 6756:
            case 7353:
            case 7358:
            // case 8643:
            case 8907:
            case 9884:
            case 9885:
            case 11426:
            case 12880:
            case 13022:
            case 13031:
            case 13032:
            case 13033:
            case 14201:
            case 14202:
            case 14203:
            case 14204:
            case 15271:
            case 19595:
            case 20050:
            case 20052:
            case 20053:
            case 20054:
            case 20055:
            case 20128:
            case 20131:
            case 20132:
            case 20133:
            case 20134:
            case 20572:
            case 23415:
            //case 23505:
            case 24752:
            case 24899:
            case 24900:
            case 26990:
            case 27134:
            case 27615:
            case 31661:
            case 32108:
            case 32548:
            case 33041:
            case 33042:
            case 33043:
            case 33405:
            case 33697:
            case 33702:
            case 33786:
            // case 34121:
            // case 35383:
            case 35480:
            case 35481:
            case 35482:
            case 35483:
            case 36817:
            case 37163:
            case 37204:
            case 37205:
            // case 37206:
            case 37444:
            case 37612:
            case 38099:
            case 38100:
            case 38101:
            case 38102:
            // case 38103:
            // case 38104:
            case 38105:
            case 38106:
            case 43837:
            case 24086:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_BUFF_STACKING;
                break;
            case 2096:
            case 10909:
            case 33045:
            case 38112:
            case 8326:
            case 9036:
            case 20584:
            case 55194:
            case 55169:
            case 55170:
            case 55171:
            case 55172:
            case 55173:
            case 55174:
            case 55175:
            case 55176:
            case 55177:
            case 55178:
            case 55180:
            case 44218:
            case 44224:
            case 44226:
            case 49887:
            case 44227:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGONE_ALL_BUFF_STACK;
                break;
            case 1604:
            case 5116:
            case 11113:
            case 12323:
            case 13018:
            case 13019:
            case 13020:
            case 13021:
            case 18118:
            case 27133:
            case 31125:
            case 31935:
            case 32699:
            case 32700:
            case 33933:
            case 35101:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_DAZE_EFFECT;
                break;
            case 33763:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_REFRESH_MODIFIERS;
                break;
            case 589:
            case 594:
            case 970:
            case 992:
            case 2767:
            case 10892:
            case 10893:
            case 10894:
            case 17364:
            case 25367:
            case 25368:
            case 27605:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_ALWAYS_CASTABLE;
                break;
            case 20625:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_PARTY_CASTER;
                break;
            case 32148:
                spellInfo->Effect[2] = 0;
                spellInfo->EffectBasePoints[2] = 0;
                break;
				// Dawnblade Hawkrider - Cast Dawnblade Attack (prevent pathing bugs)
			case 45189:
				spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT;
				spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
				break;
            case 36481:
                spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            case 9256:
                spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_IGNORE_RESISTANCES;
                break;
            case 7106:
            case 29067:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_NEARBY_ENTRY;
                break;
            case 45248:
                spellInfo->Attributes |= SPELL_ATTR_RANGED;
                break;
            case 20911:
            case 20912:
            case 20913:
            case 20914:
            case 27168:
            case 25899:
            case 27169:
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            case 32785:
                spellInfo->Effect[2] = 0;
                spellInfo->EffectBasePoints[2] = 0;
            case 33637:
                spellInfo->AttributesEx |= SPELL_ATTR_EX_NOT_IN_COMBAT_TARGET | SPELL_ATTR_EX_NO_THREAT;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            case 37229:
                spellInfo->EffectImplicitTargetA[0] = 6;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            case 31902:
                spellInfo->AttributesEx2 &= SPELL_ATTR_EX2_IGNORE_LOS;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            case 37206:
                spellInfo->AttributesEx = SPELL_ATTR_EX_CHANNELED_1;
            case 38103:
            case 38104:
                spellInfo->InterruptFlags = 15;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_BUFF_STACKING;
                break;
            // Dragonmaw Race: All parts
            case 40905:
            case 40900:
            case 40929:
            case 40931:
            case 41064:
                spellInfo->EffectRadiusIndex[0] = 18; // 15 yd, because 7ydd is not enough for real challenge
                spellInfo->EffectRadiusIndex[1] = 18; // 15 yd, because 7ydd is not enough for real challenge
                break;
            case 40890: // Oldie's Rotten Pumpkin
                spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectTriggerSpell[0] = 40905;
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                break;
            case 40909: // Trope's Slime Cannon
                spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectTriggerSpell[0] = 40905;
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                break;
            case 40894: // Corlok's Skull Barrage
                spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectTriggerSpell[0] = 40900;
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                break;
            case 40928: // Ichman's Blazing Fireball
                spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectTriggerSpell[0] = 40929;
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                break;
            case 40930: // Mulverick's Great Balls of Lightning
                spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectTriggerSpell[0] = 40931;
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                break;
            case 40945: // Sky Shatter
                spellInfo->Targets |= TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectTriggerSpell[0] = 41064;
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_MISSILE;
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_DEST;
                break;
            case 18987:
                spellInfo->Effect[1] = 77;
                spellInfo->EffectBasePoints[1] = 1;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                break;
            case 34367:
                spellInfo->ChannelInterruptFlags = 0;
                break;
            case 39401: // GAME OVER (DND)
                spellInfo->DurationIndex = 8; // 15 sec
                break;
            case 3287:
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_DEATH_PERSISTENT;
                break;
            case 34447: // Arcane Missiles. Mechanic bug. Should be removed when fixed. TRENTONE
                spellInfo->EffectImplicitTargetA[0] = 77;
                break;
            case 35460:
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_NEARBY_ENTRY;
                break;
            case 40080:
                spellInfo->EffectRealPointsPerLevel[0] = 0;
                break;
            case 32251:
                spellInfo->EffectImplicitTargetA[0] = 18;
                spellInfo->EffectImplicitTargetB[0] = 16;
                break;
            case 31623:
                spellInfo->EffectBasePoints[0] = 200;
                break;
            case 34290:
                spellInfo->EffectImplicitTargetA[0] = 24;
                spellInfo->EffectImplicitTargetB[0] = 0;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_WIDE;
                break;
            case 29951:
                spellInfo->EffectRadiusIndex[0] = 42;
                spellInfo->EffectRadiusIndex[1] = 42;
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[1] = 0;
                break;
				// ashbringer
			case 28441:
				spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT;
				spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
				break;
            case 30625:
                spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            case 35846:
                spellInfo->EffectImplicitTargetA[0] = 25;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 33572:
                spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC; // dispelable by cloak
                break;
            case 29947:
                spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                break;
            case 29448:
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_DAMAGE;
                break;
            case 29883:
                spellInfo->SpellFamilyName = 0;
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 30039:
                spellInfo->SpellFamilyName = 0;
                break;
            case 31515:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_NEARBY_ENTRY;
                break;
            case 34168:
            case 38652:
                spellInfo->Effect[0] = 6;
                spellInfo->EffectImplicitTargetA[0] = 1;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 33332:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_AREA_ENTRY_SRC;
                spellInfo->EffectImplicitTargetB[0] = 0;
                spellInfo->MaxAffectedTargets = 4;
                break;
            case 33335:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_NEARBY_ENTRY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 31718: // Enveloping Winds
                spellInfo->Attributes &= ~SPELL_ATTR_BREAKABLE_BY_DAMAGE;
                break;
            case 31673: // Foul Spores
            case 28333: // Whirldwind - Naxxramas
                spellInfo->InterruptFlags = 0;
                break;
            case 29989:
                spellInfo->EffectBasePoints[0] = 70;
                break;
            case 39810:
                spellInfo->Effect[2] = 24;
                spellInfo->EffectItemType[2] = 32320;
                spellInfo->EffectBasePoints[2] = 1;
                spellInfo->EffectImplicitTargetA[2] = 1;
                break;
            case 17775:
                spellInfo->EffectMultipleValue[0] = 0;
                break;
            case 2479: // Honorless Target
                spellInfo->DurationIndex = 3; // 1 min.
                break;
            case 46705: // Honorless Target aura 80 yd
                spellInfo->Effect[0] = 128;
                spellInfo->EffectRadiusIndex[0] = 31;
                spellInfo->DurationIndex = 21;
                break;
            /* FIXED DAMAGE SPELLS */
            case 12654: // Ignite
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FIXED_DAMAGE;
                break;
            case 20532: // Intense Heat (Majordomo Executus lava pit)
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_CANT_CRIT;
                // no break here
            /* NO SPELL DMG COEFF */
            case 7714:  // Fiery Plate Gauntlets
            case 40471: // Enduring Light - T6 proc
            case 40472: // Enduring Judgement - T6 proc
            case 28715: // Flame Cap
            case 37284: // Scalding Water
            case 6297:  // Fiery Blaze
            // Six Demon Bag spells
            case 45297:   // Chain Lightning
            case 23102:    // Frostbolt!
            case 9487:    // Fireball !
            case 45429: // Shattered Sun Pendant of Acumen: Scryers ex proc
            case 37661: // The Lightning Capacitor, lightning bolt spell
            case 38324: // Regeneration (Fel Reaver's Piston)
            case 28733: // Arcane Torrent
            case 43731: // Lightning Zap on critters (Stormchops)
            case 43733: // Lightning Zap on others (Stormchops)
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            /* WELL FEED */
            case 18191:
            case 46687:
            /* RUMS */
            case 5257:
            case 5021:
            case 5020:
            case 22789:
            case 37058:
            case 25804:
            case 25722:
            case 25037:
            case 20875:
            /* DIFF FOOD */
            case 18193:
            case 18125:
            case 18192:
            case 18141:
            case 18194:
            case 18222:
            case 22730:
            case 23697:
            case 25661: // Dirge's Kickin' Chimaerok Chops
                spellInfo->AttributesCu |= SPELL_ATTR_CU_TREAT_AS_WELL_FED;
                break;
            /* Scrolls - no stack */
            case 8112:  // Spirit I
            case 8113:  // Spirit II
            case 8114:  // Spirit III
            case 12177: // Spirit IV
            case 33080: // Spirit V
            case 8099:  // Stamina I
            case 8100:  // Stamina II
            case 8101:  // Stamina III
            case 12178: // Stamina IV
            case 33081: // Stamina V
            case 8096:  // Intellect I
            case 8097:  // Intellect II
            case 8098:  // Intellect III
            case 12176: // Intellect IV
            case 33078: // Intellect V
            case 8091:  // Protection I
            case 8094:  // Protection II
            case 8095:  // Protection III
            case 12175: // Protection IV
            case 33079: // Protection V
            /* Other to not stack with scrolls */
            case 35078: // Band of the Eternal Defender
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SCROLL_STACK;
                break;
            /* DRUID CUSTOM ATTRIBUTES */
            case 5215: // Prowl (Rank 1)
            case 6783: // Prowl (Rank 2)
            case 9913: // Prowl (Rank 3)
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_HITBYSPELL;
                spellInfo->AuraInterruptFlags &= ~AURA_INTERRUPT_FLAG_DAMAGE;
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_DIRECT_DAMAGE_OR_NON_ABSORBED_DOT;
                break;
            /* ROGUE CUSTOM ATTRIBUTES */
            case 2094:                     // Blind
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY; // add const fake delay
                break;
            case 6538: // Dig Trap
            case 8822: // Stealth
            case 31621: // Stealth
            case 10032: // Uber Stealth
            case 1784: // Stealth (Rank 1)
            case 1785: // Stealth (Rank 2)
            case 1786: // Stealth (Rank 3)
            case 1787: // Stealth (Rank 4)
            case 1856: // Vanish (Rank 1)
            case 11327: // Vanish (Rank 1)
            case 1857: // Vanish (Rank 2)
            case 11329: // Vanish (Rank 2)
            case 27617: // Vanish (Rank 2)
            case 26888: // Vanish (Rank 3)
            case 26889: // Vanish (Rank 3)
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_HITBYSPELL;
                spellInfo->AuraInterruptFlags &= ~AURA_INTERRUPT_FLAG_DAMAGE;
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_DIRECT_DAMAGE_OR_NON_ABSORBED_DOT;
                break;
            case 55405: //arena like effect
            case 55406: //arena report effect
            case 23451: // BG buffs
            case 23493:
            case 23505:
            case 24379:
            case 23978:
            case 24378:
            case 40571:
                spellInfo->AttributesEx |= SPELL_ATTR_EX_NOT_BREAK_CASTER_STEALTH;
                //spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO; // Do not put caster in combat after use
                break;
            /* SHAMAN CUSTOM ATTRIBUTES */
            case 2895:                      // Wrath of Air Totem - disallow weird stacking
                spellInfo->EffectImplicitTargetA[0] = spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetB[0] = spellInfo->EffectImplicitTargetB[1] = 0;
                break;
            /* WARLOCK CUSTOM ATTRIBUTES */
            case 27285:                     // Seed of Corruption - final boom damage
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_CANT_TRIGGER_PROC;
                break;
            /* PRIEST CUSTOM ATTRIBUTES */
            case 15290: // Vampiric Embrace
            case 34919: // Vampiric Touch
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            /* WARRIOR CUSTOM ATTRIBUTES */
            case 12721: // Deep Wounds
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_DONE_BONUS;
                break;
            // Triggered spells that should be delayed
            case 32848:                     // Mana Restore
            case 14189:                     // Seal Fate
            case 14181:                     // Relentless Strikes
            case 17794:                     // Improved Shadow Bolt ranks 1-5
            case 17797:
            case 17798:
            case 17799:
            case 17800:
            case 20272:                     // Illumination
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                break;
            /* UNSORTED */
            /* Damage Corrections */
            case 16785: // Flamebreak
                spellInfo->EffectBasePoints[0] = 24;
                break;
            case 17289: // Shadow Shock
                spellInfo->EffectBasePoints[0] = 74;
                break;
            case 36920: // Fireball (Vazruden)
                spellInfo->EffectBasePoints[0] = 151;
                break;
            case 34934: // Shadow Bolt Volley
                spellInfo->EffectBasePoints[0] = 124;
                break;
            case 40317: // Throw
                spellInfo->EffectBasePoints[0] = 199;
                break;
            case 40103: // Sludge Nova
                spellInfo->EffectBasePoints[0] = urand(24, 29);
                break;
            case 40076://Electric Spur (used by Coilskar Wrangler 22877 in BlackTemple)
               spellInfo->EffectBasePoints[1] = 2;
               break;
            /****************/
            case 40447: // BT: Akama - Soul Channel
                spellInfo->Effect[0] = 0;
                break;
            case 29538:
                spellInfo->EffectApplyAuraName[0] = 0; // there should not be feather fall effect
                spellInfo->DurationIndex = 39; // 4s -> 2s
                spellInfo->EffectBasePoints[1] = 188; // max height
                break;
            case 24311: // Powerful Healing Ward
                spellInfo->CastingTimeIndex = 14;
                break;
            case 24178: // Will of Hakkar
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CHANNELED_1;
                break;
            case 28282: // Ashbringer
                spellInfo->Effect[2] = SPELL_EFFECT_APPLY_AURA;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                spellInfo->EffectApplyAuraName[2] = SPELL_AURA_FORCE_REACTION;
                spellInfo->EffectMiscValue[2] = 56; // Scarlet Crusade
                spellInfo->EffectBasePoints[2] = 4; // Friendly
                break;
            case 48025: // Headless Horseman's Mount
                spellInfo->Attributes |= SPELL_ATTR_CANT_USED_IN_COMBAT;
                spellInfo->Attributes |= SPELL_ATTR_ABILITY;
                break;
            case 42489: // Case Ooze Zap When Energized
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->EffectTriggerSpell[0] = 42483;
                break;
            case 42492: // Cast Energized
                spellInfo->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                spellInfo->EffectTriggerSpell[0] = 42490;
                break;
            case 38297: // Leggins of BeastMastery
                spellInfo->Effect[0] = 0;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
                break;
            case 41350:
            case 41337:
                spellInfo->Attributes |= SPELL_ATTR_CANT_CANCEL;
                break;
            // do NOT remove encapsulate on druid shapeshift, attribute is added higher, so is safe to remove it here
            case 45665:
                spellInfo->AttributesCu &= ~SPELL_ATTR_CU_MOVEMENT_IMPAIR;
                break;
            case 45680: // Kil'jaeden orbs shadowbolts
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 39042: // Rampant Infection
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 45785: // Sinister Reflection
                spellInfo->SpellVisual = 0;
                break;
            case 46105:
                spellInfo->EffectImplicitTargetA[0] = 0;
                break;
            case 35346:
                spellInfo->SpellFamilyName = 0;
                spellInfo->Effect[1] = SPELL_EFFECT_TELEPORT_UNITS;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetB[1] = TARGET_DEST_TARGET_BACK;
                spellInfo->EffectRadiusIndex[1] = 15;
                break;
            case 40017: // If we can't adjust speed :P we spawn it in bigger periods
                spellInfo->EffectAmplitude[1] = 1900;
                break;
            case 40841:
                spellInfo->EffectRadiusIndex[0] = 15;
                break;
            case 41120:
                spellInfo->EffectImplicitTargetA[0] = TARGET_DEST_TARGET_LEFT;
                break;
            case 37851: // Tag Greater Felfire Diemetradon
                spellInfo->EffectImplicitTargetA[1] = 1;
                break;
            case 41117:
                spellInfo->Effect[0] = 0;
                spellInfo->Effect[1] = 0;
                break;
            case 38054:
                spellInfo->MaxAffectedTargets = 10;
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_AREA_ENTRY_SRC;
                spellInfo->Targets = TARGET_FLAG_GAMEOBJECT & TARGET_FLAG_DEST_LOCATION;
                spellInfo->EffectMiscValue[0] = 0;
                break;
            case 33824:
                spellInfo->Effect[2] = 0;
                break;
            case 34121: // Al'ar Flame Buffet
                spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_MOVEMENT;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_ON_MOVE | SPELL_ATTR_CU_IGNORE_BUFF_STACKING;;
            case 26029: // dark glare
            case 43140: case 43215: // flame breath
                spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_LINE;
                break;
            case 24340: case 26558: case 28884:     // Meteor
            case 36837: case 38903:                 // Meteor
            case 26789:                             // Shard of the Fallen Star
            case 31436:                             // Malevolent Cleave
            case 35181:                             // Dive Bomb
            case 40810: case 43267: case 43268:     // Saber Lash
            case 42384:                             // Brutal Swipe
            case 45150:                             // Meteor Slash
                spellInfo->AttributesCu |= SPELL_ATTR_CU_SHARE_DAMAGE;
                switch (i) // Saber Lash Targets
                {
                case 40810:
                        spellInfo->MaxAffectedTargets = 3;
                        break;
                    case 43267:
                    case 43268:
                        spellInfo->MaxAffectedTargets = 2;
                        break;
                    case 45150:
                        spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                        break;
                }
                break;
            case 44978:
            case 45001:
            case 45002:     // Wild Magic
            case 45004:
            case 45006:
            case 45010:     // Wild Magic
            //case 31347: // Doom
            case 41635: // Prayer of Mending
            case 44869: // Spectral Blast
            case 45976: // Muru Portal Channel
            case 39365: // Thundering Storm
            case 41071: // Raise Dead
            case 41172: // Rapid Shot
            case 40834: // Agonizing Flames
            case 45032: // Curse of Boundless Agony
            case 42357: // Axe Throw, triggered by 42359
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 45034:
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_TARGET_SELF;
                break;
            case 38281: // Static Charge (LV)
            case 39992: // Najentus: Needle Spine
            case 46019: // Teleport: Spectral Realm
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_PLAYERS_ONLY;
            case 41357: // L1 Acane Charge
            case 41376: // Spite
            case 29576: // Multi-Shot
            case 37790: // Spread Shot
            case 41303: // Soul Drain
            case 31298: // Anetheron: Sleep
                spellInfo->MaxAffectedTargets = 3;
                break;
            case 37697: // Top Bunny Beam Test Visual
            case 38310: // Multi-Shot
                spellInfo->MaxAffectedTargets = 4;
                break;
            case 42005: // Bloodboil
            case 31347: // Doom
            case 39594: // Cyclone
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                if (i == 42005)
                    spellInfo->rangeIndex = 6;
                break;
           /* case 37408: // Oscillation Field
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_STACK_FOR_DIFF_CASTERS;
                break;*/
            case 41625: // Fel Rage 3
                spellInfo->Stances = 0;
                break;
            case 45641: // Fire Bloom
                spellInfo->MaxAffectedTargets = 5;
                spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC; // dispelable by cloak
                break;
            case 38296: // Spitfire Totem
            case 37676: // Insidious Whisper
            case 46771: // Flame Sear
                spellInfo->MaxAffectedTargets = 5;
                break;
            case 40827: // Sinful Beam
            case 40859: // Sinister Beam
            case 40860: // Vile Beam
            case 40861: // Wicked Beam
                spellInfo->MaxAffectedTargets = 10;
                break;
            case 8122:
            case 8124:
            case 10888:
            case 10890: // Psychic Scream
                spellInfo->Attributes |= SPELL_ATTR_BREAKABLE_BY_DAMAGE;
                break;
            case 44949: // Whirlwind's offhand attack - TODO: remove this (50% weapon damage effect)
                spellInfo->Effect[1] = 0;
                break;
            case 24905: // Moonkin form -> elune's touch
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                break;
            case 27066: // Trueshot r4 - poprzednie ranki nie maja dispel type: MAGIC o.O
                spellInfo->Dispel = DISPEL_NONE;
                break;
            case 31117: // UA dispell effect
                spellInfo->SpellFamilyFlags = 0x010000000000LL;
                break;
            case 32045: // Archimonde: Soul Charge - yellow
            case 32051: // Archimonde: Soul Charge - green
            case 32052: // Archimonde: Soul Charge - red
                spellInfo->procCharges = 0;
                spellInfo->procChance = 101;
                spellInfo->procFlags = 0;
                break;
            case 40105:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->Effect[2] = 0;
            case 40106:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectTriggerSpell[0] = 0;
            case 41001: // Fatal Attraction Aura
                spellInfo->EffectTriggerSpell[1] = 0;
                break;
            case 40869: // Fatal Attraction
                spellInfo->EffectRadiusIndex[0] = 12;
                spellInfo->MaxAffectedTargets = 3;
                spellInfo->Effect[1] = 0;
                break;
            case 40870: // Fatal Attraction Trigger
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ALLY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 40594: // Fel Rage
                spellInfo->EffectBasePoints[1] = 99;
                break;
            case 40855: // Akama Soul Expel
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 40401: // Shade of Akama Channeling
                spellInfo->Effect[2] = spellInfo->Effect[0];
                spellInfo->EffectApplyAuraName[2] = spellInfo->EffectApplyAuraName[0];
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                break;
            case 40251:
                spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_IGNORE_RESISTANCES;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
                break;
            case 36819: // Kael Pyroblast
                spellInfo->rangeIndex = 6;  // from 40yd to 100yd to avoid running from dmg
                break;
            case 40334:
                spellInfo->procFlags = PROC_FLAG_SUCCESSFUL_MELEE_HIT;
                break;
            case 30015: // Summon Naias cooldown
                spellInfo->RecoveryTime = 300000;
                break;
            case 35413: // Summon Goliathon cooldown
                spellInfo->RecoveryTime = 300000;
                break;
            case 13278: // Gnomish Death Ray
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 6947:  // Curse of the Bleakheart
                spellInfo->procFlags = 65876;      //any succesfull melee, ranged or negative spell hit
                break;
            case 34580: // Impale(Despair item)
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_ARMOR;
                break;
            case 66:    // Invisibility (fading) - break on casting spell
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_CAST;
                break;
            case 37363: // set 5y radius instead of 25y
                spellInfo->EffectRadiusIndex[0] = 8;
                spellInfo->EffectRadiusIndex[1] = 8;
                spellInfo->EffectMiscValue[1] = 50;
                break;
            case 42835: // set visual only
                spellInfo->Effect[0] = 0;
                break;
            case 46037:
            case 46040:
                spellInfo->EffectBasePoints[1] = 1;
            case 46038:
            case 46039:
            case 45661:
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            case 21358: // Aqual Quintessence / Eternal Quintessence
            case 47977: // Broom Broom
            case 42679:
            case 42673:
            case 42680:
            case 42681:
            case 42683:
            case 42684:
                spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA;
                break;
            case 43730: // Stormchops effect
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectImplicitTargetB[1] = 0;
                std::swap(spellInfo->EffectBasePoints[0], spellInfo->EffectBasePoints[1]);
                std::swap(spellInfo->EffectAmplitude[0], spellInfo->EffectAmplitude[1]);
                std::swap(spellInfo->EffectRadiusIndex[0], spellInfo->EffectRadiusIndex[1]);
                std::swap(spellInfo->EffectApplyAuraName[0], spellInfo->EffectApplyAuraName[1]);
                std::swap(spellInfo->EffectTriggerSpell[0], spellInfo->EffectTriggerSpell[1]);
                spellInfo->AttributesCu |= SPELL_ATTR_CU_TREAT_AS_WELL_FED;
                break;
            case 41470: //Judgement of Command should be reflectable
                spellInfo->AttributesEx2 = 0;
                break;
            case 26373: // Lunar Invitation
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetB[1] = 0;
                break;
            case 31532: // Repair from Mekgineer event in Steamvault
            case 37936:
                spellInfo->Attributes &= ~SPELL_ATTR_BREAKABLE_BY_DAMAGE;
                break;
            case 39095: // karazhan prince malchezar: Amplify Damage
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 39842: // Chess event: rain of fire damage
            case 37465: // Chess event: rain of fire
                spellInfo->EffectImplicitTargetA[0] = TARGET_DST_TARGET_ENEMY;
                spellInfo->EffectRadiusIndex[0] = 14; // 6 yd to 8 yd
                spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                break;
            case 37469: // Chess event: poison cloud
                spellInfo->EffectRadiusIndex[0] = 14; // 6 yd to 8 yd
                break;
            case 37498: // Chess event: Stomp
            case 37502: // Chess event: Howl
                spellInfo->EffectRadiusIndex[0] = 29; // 10 yd to 6 yd
                spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_WIDE;
                break;
            case 37476: // Chess event: Cleave
            case 37474: // Chess event: Sweep
                spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_WIDE;
                break;
            case 37454: // Chess event: Bite
            case 37453: // Chess event: Smash
            case 37413: // Chess event: Visious Strike
            case 37406: // Chess event: Heroic Blow
                spellInfo->EffectRadiusIndex[0] = 15;    // effect radius from 8 to 3 yd
                break;
            case 37834: spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY; //Unbanish Azaloth
                break;
            case 37461: // Chess event: Shadow Spear
            case 37459: // Chess event: Holy Lance
                spellInfo->AttributesCu |= SPELL_ATTR_CU_CONE_LINE;
                spellInfo->EffectRadiusIndex[0] = 18;   // effect radius from 18 to 15 yd
                break;
            case 41363: // Shared Bonds
                spellInfo->AttributesEx &= ~SPELL_ATTR_EX_CHANNELED_1;
            case 16007: // DRACO_INCARCINATRIX_900
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                break;
            case 39331: // In Session
                spellInfo->DurationIndex = 21;  // infinity
                spellInfo->Effect[0] = SPELL_EFFECT_APPLY_AREA_AURA_FRIEND;
                spellInfo->EffectRadiusIndex[0] = 27;   // effect radius from 65 to 50 yd
                break;
            case 19937: //Illusion: Black Dragonkin
                spellInfo->AreaId = 15;
                break;
            case 28062: // Positive Charge
            case 28085: // Negative Charge
            case 39090: // Positive Charge
            case 39093: // Negative Charge
            case 39968: // Needle Spine Explosion
            case 39692: // Cannon
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_TARGET_SELF;
                break;
            case 42992: //ram - neutral
            case 43310: //ram - trot
                spellInfo->EffectImplicitTargetA[1] = 1;
                break;
            case 37370: // Kelidan the breaker - vortex
                spellInfo->EffectMiscValue[0] /= 3;
                break;
            case 41345: // Infatuation (BT Trash)
                spellInfo->AttributesEx2 &= ~SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            case 40222: // Smash Hit
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            case 43383: // Spirit Bolts (HexLord)
                spellInfo->ChannelInterruptFlags = 0;
                spellInfo->InterruptFlags &= ~SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
			case 27661: //Love Fool
				spellInfo->DurationIndex = 8;
				break;
            case 30889:
                spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
            case 40214: // Dragonmaw illusion
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_DEATH_PERSISTENT;
                break;
            case 39288:
            case 39289:
            case 39290:
                spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_DEATH_PERSISTENT;
                break;
            case 30541: // Magtheridon's Blaze
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ENEMY;
                spellInfo->EffectImplicitTargetB[0] = 0;
                break;
            case 36449: // Magtheridon's Debris (30% hp)
                spellInfo->EffectImplicitTargetA[0] = TARGET_SRC_CASTER;
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_SRC;
                spellInfo->EffectImplicitTargetA[1] = TARGET_SRC_CASTER;
                spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_AREA_ENEMY_SRC;
                break;
            case 30631: // Magtheridon's Debris damage
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_DST;
                break;
            case 30207: // Magtheridon's creatures Shadow Grasp
                spellInfo->StackAmount = 5;
                break;
            case 20814: // Collect Dire Water
                spellInfo->InterruptFlags = SPELL_INTERRUPT_FLAG_MOVEMENT | SPELL_INTERRUPT_FLAG_DAMAGE | SPELL_INTERRUPT_FLAG_AUTOATTACK | SPELL_INTERRUPT_FLAG_PUSH_BACK | SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
            case 31059:
            case 35766:
                spellInfo->InterruptFlags = SPELL_INTERRUPT_FLAG_MOVEMENT | SPELL_INTERRUPT_FLAG_DAMAGE | SPELL_INTERRUPT_FLAG_AUTOATTACK | SPELL_INTERRUPT_FLAG_PUSH_BACK | SPELL_INTERRUPT_FLAG_INTERRUPT;
                spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_CAST | AURA_INTERRUPT_FLAG_JUMP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_HITBYSPELL | AURA_INTERRUPT_FLAG_ATTACK | AURA_INTERRUPT_FLAG_TURNING | AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT | AURA_INTERRUPT_FLAG_DIRECT_DAMAGE;
                break;
            case 29200: // Purify Helboar Meat - for quest
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_CASTER;
                break;
            case 31790: // Righteous Defense taunt
                spellInfo->Attributes |= SPELL_ATTR_ABILITY;
                break;
            case 30346: // Fel Iron Shells duplicated effect, weird
            case 30254: // Curator Evocation
                spellInfo->Effect[1] = 0;   // remove self stun
                break;
            case 10722: // Silithid Swarm - silithids in barrens were forever...
                spellInfo->DurationIndex = 3;
                break;
            case 12051: // Evocation - Now can kick/pummel interrupt.
                spellInfo->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
                break;
            case 28722: // dreamwalker set 2 bonus - rejuvenation ticks gives mana/energy/rage
            case 28723: // dreamwalker set 2 bonus - rejuvenation ticks gives mana/energy/rage
            case 28724: // dreamwalker set 2 bonus - rejuvenation ticks gives mana/energy/rage
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO; // Not blizzlike
                break;
            case 25112: // summon voidwalker trinket now not usable in arena.
                spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA;
                break;
            case 18119: // Aftermath (Rank 1)
            {
                spellInfo->procChance = 100;
                spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_OVERRIDE_CLASS_SCRIPTS;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
                spellInfo->EffectMiscValue[0] = 2189;
                spellInfo->EffectTriggerSpell[0] = 0;
                spellInfo->EffectTriggerSpell[2] = 54643;
                break;
            }
            case 18120: // Aftermath (Rank 2)
            {
                spellInfo->procChance = 100;
                spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_OVERRIDE_CLASS_SCRIPTS;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
                spellInfo->EffectMiscValue[0] = 2189;
                spellInfo->EffectTriggerSpell[0] = 0;
                spellInfo->EffectTriggerSpell[2] = 54644;
                break;
            }
            case 18121: // Aftermath (Rank 3)
            {
                spellInfo->procChance = 100;
                spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_OVERRIDE_CLASS_SCRIPTS;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
                spellInfo->EffectMiscValue[0] = 2189;
                spellInfo->EffectTriggerSpell[0] = 0;
                spellInfo->EffectTriggerSpell[2] = 54645;
                break;
            }
            case 18122: // Aftermath (Rank 4)
            {
                spellInfo->procChance = 100;
                spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_OVERRIDE_CLASS_SCRIPTS;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
                spellInfo->EffectMiscValue[0] = 2189;
                spellInfo->EffectTriggerSpell[0] = 0;
                spellInfo->EffectTriggerSpell[2] = 54646;
                break;
            }
            case 18123: // Aftermath (Rank 5)
            {
                spellInfo->procChance = 100;
                spellInfo->Effect[1] = SPELL_EFFECT_APPLY_AURA;
                spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
                spellInfo->EffectImplicitTargetA[1] = TARGET_UNIT_CASTER;
                spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_OVERRIDE_CLASS_SCRIPTS;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
                spellInfo->EffectMiscValue[0] = 2189;
                spellInfo->EffectTriggerSpell[0] = 0;
                spellInfo->EffectTriggerSpell[2] = 54647;
                break;
            }
            //case 18073: // Pyroclasm (Rank 2)
            //{
            //    spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
            //    spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
            //    spellInfo->EffectTriggerSpell[2] = 54642;
            //    break;
            //}
            //case 18096: // Pyroclasm (Rank 1)
            //{
            //    spellInfo->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
            //    spellInfo->EffectImplicitTargetA[2] = TARGET_UNIT_CASTER;
            //    spellInfo->EffectTriggerSpell[2] = 54641;
            //    break;
            //}
            case 35383: // Flame Patch
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_BUFF_STACKING;
            case 1050: // Sacrifice
            case 17471: // Death Pact
            case 23861: // Poison Cloud
            case 25516: // Aura of Command
            case 28241: // Poison
            case 28299: // Ball Lightning
            case 28522: // Icebolt
            case 28547: // Chill
            case 29609: // Ill Gift
            case 30122: // Plague Cloud
            case 30216: // Fel Iron Bomb
            case 30217: // Adamantite Grenade
            case 30461: // The Bigger One
            case 30486: // Super Sapper Charge
            case 30613: // Blast Nova
            case 30925: // Exploding Beaker
            case 31250: // Frost Nova
            case 35276: // Throw Dynamite
            case 37118: // Shell Shock
            case 37120: // Fragmentation Bomb
            case 38819: // Death Blast
            case 38893: // Fire Shield
            case 40736: // Death Blast
            case 41068: // Blood Siphon
            case 41578: // Pyroblast
            case 41579: //  Iceblast
            case 41958: // Immolate
            case 43093: // Grievous Throw
            case 43299: // Flame Buffet
            case 44459: // Living Bomb (Rank 1)
            case 44461: // Living Bomb (Rank 1)
            case 45996: // Darkness
            case 46184: // Fel Iron Bomb
            case 46186: // Goblin Dragon Gun
            case 22482: // Blade Flurry Damage Proc
            case 12723: // Sweeping Strikes
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_DONE_BONUS;
                break;
            case 29949: // Rocket
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_DONE_BONUS;
                spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                break;
            case 39965: // Frost Grenade (Rank 5)
                spellInfo->SpellFamilyName = 0;
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_DONE_BONUS;
                break;
            case 20623: // Fire Blast
                spellInfo->SpellFamilyName = 0;
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
            break;
            case 37563: // Renewal (Rank 1)
            case 33554: // Judgement of Command
            case 45055: // Shadow Bolt
            case 21889: // Judgement Smite
            case 27655: // Flame Lash
            case 29496: //  Jealousy
            case 34587: // Romulo's Poison
            case 36829: // Hell Rain
            case 36841: // Sonic Boom
            case 37252: // Water Bolt
            case 41192: // Deadly Poison
            case 26415: // Shock
            case 35250: // Dragon's Breath
            case 16614: //Lightning Bolt from gloves mail
            case 7712: //Fire Blast from neck
            case 41990: //fire blast from fists of fury from hyjal
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                break;
            case 19260: // Frost Blast
                spellInfo->SpellFamilyName = 0;
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                break;
            case 41276: // Meteor
            {
                spellInfo->AttributesCu |= SPELL_ATTR_CU_SHARE_DAMAGE;
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_DONE_BONUS;
                break;
            }
            case 408: // Kidney Shot (Rank 1)
            case 8643: // Kidney Shot (Rank 2)
                spellInfo->Effect[1] = 0;
                spellInfo->EffectImplicitTargetA[1] = 0;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGNORE_BUFF_STACKING;
                break;
            //case 18727: // Tamed Pet Passive (DND)
            //case 18728: // Tamed Pet Passive (DND)
            //case 18729: // Tamed Pet Passive (DND)
            //case 18730: // Tamed Pet Passive (DND)
            //case 19591: // Tamed Pet Passive (DND)
            //    spellInfo->Effect[2] = 6;
            //    spellInfo->EffectBaseDice[2] = 1;
            //    spellInfo->EffectBasePoints[2] = -1;
            //    spellInfo->EffectImplicitTargetA[2] = 1;
            //    spellInfo->EffectApplyAuraName[2] = 23;
            //    spellInfo->EffectAmplitude[2] = 3000;
            //    spellInfo->EffectTriggerSpell[2] = 54761;
            //    break;
            /*case 18803: // Focus -- 2.4.0 patch undocumented change made it be 4 seconds, so it is right in dbc
                spellInfo->DurationIndex = 32;
                break;*/
            case 19239: // Clever Traps (Rank 1)
                spellInfo->Effect[2] = 6;
                spellInfo->EffectBaseDice[2] = 1;
                spellInfo->EffectBasePoints[2] = 14;
                spellInfo->EffectImplicitTargetA[2] = 1;
                spellInfo->EffectApplyAuraName[2] = 108;
                spellInfo->EffectMiscValue[1] = SPELLMOD_DAMAGE;
                spellInfo->EffectMiscValue[2] = 22;
                spellInfo->EffectItemType[2] = 0x4LL;
                break;
            case 19245: // Clever Traps (Rank 2)
                spellInfo->Effect[2] = 6;
                spellInfo->EffectBaseDice[2] = 1;
                spellInfo->EffectBasePoints[2] = 29;
                spellInfo->EffectImplicitTargetA[2] = 1;
                spellInfo->EffectApplyAuraName[2] = 108;
                spellInfo->EffectMiscValue[1] = SPELLMOD_DAMAGE;
                spellInfo->EffectMiscValue[2] = 22;
                spellInfo->EffectItemType[2] = 0x4LL;
                break;
            case 19596: // Bestial Swiftness
                spellInfo->EffectBasePoints[0] = -1;
                spellInfo->EffectApplyAuraName[0] = 34;
                spellInfo->EffectItemType[0] = 0x0LL;
                spellInfo->EffectMiscValue[0] = 0;
                break;
            case 20253: // Intercept Stun (Rank 1)
            case 20614: // Intercept Stun (Rank 2)
            case 20615: // Intercept Stun (Rank 3)
            case 25273: // Intercept Stun (Rank 4)
            case 25274: // Intercept Stun (Rank 5)
                spellInfo->SpellFamilyName = 4;
                spellInfo->SpellFamilyFlags = 0x0000200000000000LL;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                break;
            case 7922: // Charge Stun
            case 30153: // Felguard Intercept Stun rank 1
            case 30195: // Felguard Intercept Stun rank 2
            case 30197: // Felguard Intercept Stun rank 3
            case 25999: // Boar Charge Immobilize
                // Feral charge?
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                break;
            case 21079: // Shard of the Defiler - remove crit and haste effects from the item
                spellInfo->Effect[1] = 0;
                spellInfo->Effect[2] = 0;
                break;
            case 32727: // Arena Preparation - remove invisibility aura and make even increased spell casts no cost
            case 44521: // preparation - battlegrounds
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGONE_ALL_BUFF_STACK;
                // 1 - no costs
                spellInfo->EffectBasePoints[0] = -201;

                // 2 - periodic heal and remove gcd and set insane cast speed
                spellInfo->Effect[1] = 6;
                spellInfo->EffectBasePoints[1] = 1;
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectApplyAuraName[1] = 23;
                spellInfo->EffectAmplitude[1] = 5000;
                spellInfo->EffectTriggerSpell[1] = 55169; // power restore
                // 55169 triggers spells one per each class:
                // 55170-55180
                spellInfo->EffectMiscValue[1] = 0;

                // 3 - arena prep - no reagent needed
                spellInfo->Effect[2] = 6;
                spellInfo->EffectBasePoints[2] = 1;
                spellInfo->EffectApplyAuraName[2] = 215;
                spellInfo->EffectImplicitTargetA[2] = 1;
                break;
            case 32728: // Arena Preparation make even increased spell casts no cost
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGONE_ALL_BUFF_STACK;
            case 44535: // Spirit Heal make even increased spell casts no cost
                spellInfo->AttributesCu |= SPELL_ATTR_CU_IGONE_ALL_BUFF_STACK;
                spellInfo->EffectBasePoints[0] = -201;
                break;
            case 31893: // Seal of Blood damage proc
                spellInfo->SpellVisual = 39; // taken from Seal of Command - i want this spell to be told in combat log.
                break;
            case 32220: // Judgement of Blood backlash
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                spellInfo->DurationIndex = 36;
                spellInfo->Effect[0] = 6;
                spellInfo->EffectImplicitTargetA[0] = 1;
                spellInfo->EffectBaseDice[0] = 1;
                spellInfo->EffectBasePoints[0] = -1;
                spellInfo->EffectApplyAuraName[0] = 3;
                spellInfo->EffectAmplitude[0] = 1000;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_CANT_TRIGGER_PROC;
                break;
            case 31983: // Earthgrab
                spellInfo->RecoveryTime = 30000;
                break;
            case 19185: // Entrapment
                spellInfo->SpellFamilyName = 9;
                break;
            case 25028: // Fire Blast
                spellInfo->manaCost = 400;
                spellInfo->StartRecoveryTime = 3000;
                spellInfo->StartRecoveryCategory = 0x85LL;
                break;
            case 23462: // Fire Nova
                spellInfo->manaCost = 800;
                spellInfo->StartRecoveryTime = 3000;
                spellInfo->StartRecoveryCategory = 0x85LL;
                break;
            case 36213: // Angered Earth
                spellInfo->StartRecoveryTime = 3000;
                spellInfo->StartRecoveryCategory = 0x85LL;
                break;
            case 25046: // Arcane Torrent (Racial)
                spellInfo->Effect[1] = 3;
                spellInfo->EffectBaseDice[1] = 1;
                spellInfo->EffectImplicitTargetA[1] = 1;
                break;
            case SPELL_BG_DESERTER: // Deserter
                // 6 - 10 min
                // 30  - 30 min
                spellInfo->DurationIndex = 30;
                break;
            case 7266: // Duel
                spellInfo->AttributesEx3 = 0x20000LL;
                break;
            case 28730: // Arcane Torrent (Racial)
                spellInfo->EffectRealPointsPerLevel[1] = 0;
                spellInfo->EffectBasePoints[1] = -1;
                spellInfo->EffectImplicitTargetA[1] = 1;
                break;
            case 23394: // Shadow of Ebonroc
                 spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                 break;
            /*case 28733: // Arcane Torrent (Racial)
                spellInfo->EffectRealPointsPerLevel[0] = 0;
                spellInfo->EffectBasePoints[0] = -1;
                break;*/
            case 28734: // Mana Tap (Racial)
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                spellInfo->EffectBasePoints[0] = 50;
                break;
            case 29192: // Improved Weapon Totems (Rank 1)
            case 29193: // Improved Weapon Totems (Rank 2)
                spellInfo->Effect[0] = 35;
                spellInfo->Effect[1] = 35;
                spellInfo->EffectRadiusIndex[0] = 11;
                spellInfo->EffectRadiusIndex[1] = 11;
                spellInfo->EffectMiscValue[0] = 127;
                break;
            case 29759: // Improved Berserker Stance (Rank 1)
            case 29760: // Improved Berserker Stance (Rank 2)
            case 29761: // Improved Berserker Stance (Rank 3)
            case 29762: // Improved Berserker Stance (Rank 4)
            case 29763: // Improved Berserker Stance (Rank 5)
                spellInfo->EffectApplyAuraName[1] = 10;
                spellInfo->EffectMiscValue[1] = 127;
                break;
            case 36563: // Shadowstep
                spellInfo->EffectBasePoints[1] = -1;
                break;
            case 33152: // Prayer of Healing
                spellInfo->SchoolMask = 0x1LL;
                break;
            case 19641: // Pyroclast Barrage
                spellInfo->RecoveryTime = 60000;
                break;
            case 7001: // Lightwell Renew (Rank 1)
            case 28276: // Lightwell Renew (Rank 4)
                spellInfo->SpellFamilyName = 6;
                break;
            case 12494: // Frostbite
                spellInfo->Attributes |= SPELL_ATTR_BREAKABLE_BY_DAMAGE; // it already has that flag in dbc. lol
                spellInfo->SpellFamilyName = 3;
                break;
            case 14157: // Ruthlessness (Rank 1)
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                spellInfo->AttributesEx3 = 0x20000LL;
                break;
            case 22703: // Inferno Effect
                spellInfo->procChance = 101;
                break;
            case 25686: // Snowball
                spellInfo->EffectImplicitTargetA[0] = 6;
                spellInfo->EffectImplicitTargetA[1] = 6;
                break;
            case 27873: // Lightwell Renew (Rank 2)
            case 27874: // Lightwell Renew (Rank 3)
                spellInfo->SpellFamilyName = 6;
                break;
            //case 30147: // Tamed Pet Passive (DND)
            //    spellInfo->Effect[2] = 6;
            //    spellInfo->EffectBaseDice[2] = 1;
            //    spellInfo->EffectBasePoints[2] = -1;
            //    spellInfo->EffectImplicitTargetA[2] = 1;
            //    spellInfo->EffectApplyAuraName[2] = 23;
            //    spellInfo->EffectAmplitude[2] = 3000;
            //    spellInfo->EffectTriggerSpell[2] = 54761;
            //    break;
            case 33111:
                spellInfo->MaxAffectedTargets = 5;
                break;
            case 10052: // Replenish Mana (Rank 2)
            case 10057: // Replenish Mana (Rank 3)
            case 10058: // Replenish Mana (Rank 4)
            case 5405: // Replenish Mana (Rank 1)
            case 27103: // Replenish Mana (Rank 5)
                spellInfo->Effect[1] = 64;
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectTriggerSpell[1] = 54790;
                break;
            /*case 43681: //  Inactive
                spellInfo->Effect[1] = 6;
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectApplyAuraName[1] = 23;
                spellInfo->EffectAmplitude[1] = 60000;
                spellInfo->EffectTriggerSpell[1] = 54759;
                break;*/ // config implemented - Trentone delete spell 54759 too
            case 43922: // Increase Spell Dam 473
                spellInfo->Effect[1] = 6;
                spellInfo->EffectDieSides[1] = 0;
                spellInfo->EffectBasePoints[0] = 472;
                spellInfo->EffectBasePoints[1] = 472;
                spellInfo->EffectApplyAuraName[1] = 135;
                spellInfo->EffectMiscValue[1] = 126;
                break;
            case 45027: // Revitalize
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->SchoolMask = 0x1LL;
                break;
            case 45145: // Snake Trap Effect (Rank 1)
                spellInfo->Effect[2] = 2;
                spellInfo->EffectBasePoints[2] = 0;
                spellInfo->EffectImplicitTargetA[2] = 6;
                break;
            case 15473: // Shadowform
                spellInfo->StartRecoveryCategory = 0;
                spellInfo->CategoryRecoveryTime = 0;
                spellInfo->StartRecoveryTime = 0;
                break;
            case 1714: // Curse of Tongues rank 1
            case 11719: // Curse of Tongues rank 2
                spellInfo->SpellFamilyFlags = 0x0000800000000000LL; // new Curse of Tongues Flag.
                break;
            case 31834: // Light's Grace
                spellInfo->SpellFamilyFlags = 0;
                break;
            case 28509: // Greater Mana Regeneration - Elixir of Major Mageblood
            case 24363: // Mana Regeneration - Mageblood Potion
            case 31462: // Moonwell Restoration
            case 36746: // Shadowy Fortitude
            case 36749: // Arcane Might
            case 42965: // Tricky Treat
                spellInfo->Attributes = 0x28000000LL; // Remove when entering arena
                break;
            case 30077: // Carinda's Retribution
                spellInfo->EffectMiscValue[0] = 6271;
                break;
            case 32756: // Shadowy Disguise
                spellInfo->Effect[1] = spellInfo->Effect[0];
                spellInfo->EffectBasePoints[1] = spellInfo->EffectBasePoints[0];
                spellInfo->EffectImplicitTargetA[1] = spellInfo->EffectImplicitTargetA[0];
                spellInfo->EffectImplicitTargetB[1] = spellInfo->EffectImplicitTargetB[0];
                spellInfo->EffectApplyAuraName[1] = spellInfo->EffectApplyAuraName[0];
                spellInfo->EffectMiscValue[1] = 87;// Bloodsail buccaneers - for succubuses
                break;
            case 11213: // Arcane Concentration
            case 12574: // Arcane Concentration
            case 12575: // Arcane Concentration
            case 12576: // Arcane Concentration
            case 12577: // Arcane Concentration
                spellInfo->procFlags = 0x11000;
                break;
            case 19474: // Rain of Fire of Pit Lord (DoomGuard) - was triggering itself
                spellInfo->EffectAmplitude[0] = 2000;
                spellInfo->EffectApplyAuraName[0] = 3;
                spellInfo->Effect[1] = 0;
                spellInfo->EffectBasePoints[1] = 0;
                spellInfo->EffectImplicitTargetA[1] = 0;
                spellInfo->EffectApplyAuraName[1] = 0;
                spellInfo->EffectAmplitude[1] = 0;
                spellInfo->EffectTriggerSpell[1] = 0;
                break;
            case 29992: // Quest spell - needs cooldown to be able to add to possessed unit
                spellInfo->RecoveryTime = 120000;
                break;
            case 44935: // Expose Razorthorn Root - quest discovering your roots
                spellInfo->RecoveryTime = 10000;
                break;
            case 20271: // Paladins Judgement
                //spellInfo->AttributesEx3 &= ~SPELL_ATTR_EX3_CANT_TRIGGER_PROC; // old

                // Judgment spell should not trigger procs
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_CANT_TRIGGER_PROC;
                break;
            case 30502: // Dark Spin
                spellInfo->Effect[0] = 0;
                spellInfo->Effect[1] = 0;
                spellInfo->Effect[2] = 0;
                break;
            case 38629: // Poison Keg
                spellInfo->RecoveryTime = 1080;
                break;
            case 25678:
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ANY;
                break;
            case 40129: // Summon Air Elemental (by quest  [The Soul Cannon of Reth'hedron], fix cast freeze - fix quest)
                spellInfo->AttributesEx &= ~SPELL_ATTR_EX_CHANNELED_1;
                break;
            case 33151: // Surge of light - priest talent, makes smite incapatible of crit . Had wrong applyauraname(was pct_mod instead of flat_mod).
                spellInfo->EffectApplyAuraName[2] = 107;
                break;
            case 32409: // Shadow Word: Death (Rank 1) backlash
            case 32221: // Seal of Blood backlash
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                spellInfo->SpellFamilyFlags = 0x0LL;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_CANT_TRIGGER_PROC;
                break;
            case 18060: // Attack Power 200
                spellInfo->EffectBasePoints[1] = 199;
                break;
            case 6495: // Sentry totem
                spellInfo->EffectRadiusIndex[0] = 0;
                break;
            case 33666: ///sonic boom. the radius is less than 34 yards, because the tank and melee also can avoid it.
            case 38795:
                spellInfo->EffectRadiusIndex[0] = 9;
                spellInfo->EffectRadiusIndex[1] = 9;
                spellInfo->EffectRadiusIndex[2] = 9;
                break;
            case 33711: // Murmur's Touch
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->EffectTriggerSpell[0] = 33760;
                break;
            case 36717: //energy discharge
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_NEARBY_ENEMY;
                break;
            case 38829: // energy discharge hc
                spellInfo->MaxAffectedTargets = 1;
                break;
            case 38794:
                spellInfo->Effect[1] = 0;
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->EffectTriggerSpell[0] = 33760;
                break;
            case 29838: //Second Wind (Rank 2)
                spellInfo->procFlags &= ~PROC_FLAG_ON_TAKE_PERIODIC;
                break;
            case 32096: // Thrallmar's Favor
                spellInfo->EffectBasePoints[0] = 24;
                spellInfo->EffectApplyAuraName[0] = 190;
                spellInfo->EffectMiscValue[0] = 947;
                spellInfo->Effect[1] = 6;
                spellInfo->EffectBasePoints[1] = 4;
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectApplyAuraName[1] = 200;
                spellInfo->EffectBaseDice[1] = 1;
                break;
            case 45342: // Conflagration Eredar Twins
                spellInfo->Mechanic = MECHANIC_CONFUSED;
                spellInfo->EffectMechanic[1] = MECHANIC_NONE;
                break;
            case 32098: // Honor Hold's Favor
                spellInfo->EffectBasePoints[0] = 24;
                spellInfo->EffectApplyAuraName[0] = 190;
                spellInfo->EffectMiscValue[0] = 946;
                spellInfo->Effect[1] = 6;
                spellInfo->EffectBasePoints[1] = 4;
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectApplyAuraName[1] = 200;
                spellInfo->EffectBaseDice[1] = 1;
                break;
            case 42805: // new year 2015 event spell - Dirty Trick
                spellInfo->CastingTimeIndex = 14;
                spellInfo->Effect[1] = 0;
                spellInfo->Effect[0] = 3;
                spellInfo->EffectImplicitTargetA[0] = 31;
                break;
            //case 20424: // Seal of Command proc
            //    spellInfo->speed = 10; // from 5 yds (melee range proc, doh) it will have 0.5 delay. 8 yds - 0.8 delay... but it doesn't matter
            //    break;
            case 31892: // Seal of Blood (Rank 1)
                spellInfo->Effect[1] = 0; // No effect
                spellInfo->EffectApplyAuraName[1] = 0;
                spellInfo->EffectBasePoints[1] = 0;
                spellInfo->EffectImplicitTargetA[1] = 0;
                break;
            case 46713: // Transmute Metals (Alchemy Passive)
            case 46714: // Transmute Elements (Alchemy Passive)
            case 46715: // Transmute Gems (Alchemy Passive)
            case 20608: // Reincarnation (Passive)
                spellInfo->CategoryRecoveryTime = 1000;
                break;
            case 800: // Twin Teleport
                spellInfo->DurationIndex = 39; // 2 sec.
                spellInfo->SchoolMask = 0;
                break;
            // Nefarian's class spells
            case 23410: // mage
                spellInfo->EffectTriggerSpell[0] = 23603;
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            case 23603: // poly
                spellInfo->MaxAffectedTargets = 1;
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS;
                break;
            case 46648: // ugly hack for puling whole SWP
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            case 25671: // Drain Mana - Moam AQ20
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FIXED_DAMAGE;
                break;
            case 19593: // Buru Egg Explosion
                spellInfo->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;
                spellInfo->Effect[1] = SPELL_EFFECT_SCHOOL_DAMAGE;
                spellInfo->DurationIndex = 0;
                spellInfo->EffectBasePoints[0] = 10;
                spellInfo->EffectBasePoints[1] = 10;
                spellInfo->AttributesEx |= SPELL_ATTR_EX_CANT_BE_REFLECTED | SPELL_ATTR_EX_CANT_BE_REDIRECTED;
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_IGNORE_LOS | SPELL_ATTR_EX2_CANT_CRIT;
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_DONE_BONUS;
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ENEMY_SRC;
                spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_AREA_ALLY_SRC;
                spellInfo->EffectImplicitTargetA[1] = TARGET_SRC_CASTER;
                spellInfo->EffectRadiusIndex[1] = 20; // 25 yd
                spellInfo->EffectMiscValue[1] = 0;
                spellInfo->EffectMiscValueB[1] = 0;
                spellInfo->AttributesCu |= SPELL_ATTR_CU_DIRECT_DAMAGE; // it is assigned to SPELL_EFFECT_SCHOOL_DAMAGE higher. Need to reassign here, cause spell had no such effect
                break;
            case 5246: // Intimidating Shout
                spellInfo->Attributes |= SPELL_ATTR_BREAKABLE_BY_DAMAGE;
                break;
            case 29214: // Wrath of the Plaguebringer // Noth the PlagueBringer
                spellInfo->EffectImplicitTargetB[0] = TARGET_UNIT_AREA_ALLY_SRC;
                spellInfo->EffectImplicitTargetB[1] = TARGET_UNIT_AREA_ALLY_SRC;
                break;
            case 1206:
                spellInfo->Attributes |= SPELL_ATTR_CASTABLE_WHILE_MOUNTED;
                spellInfo->AttributesEx |= SPELL_ATTR_EX_NOT_BREAK_CASTER_STEALTH;
                break;
            case 30834: // Infernal Relay - Malchezaar spell from Karazhan - casted on Infernals
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ANY;
                break;
            // found by target == 1 and speed = 40, other spells might also be instant, but they're hard to confirm cause they're not player-based
            case 3045: // Rapid Fire
            case 36828: // Rapid Fire
            // exploited quiver/ammo pouch spells for incredible ranged attack speed bug
            case 14824:
            case 14825:
            case 14826:
            case 14827:
            case 14828:
            case 14829:
            case 29413:
            case 29414:
            case 29415:
            case 29416:
            case 29417:
            case 29418:
            case 44972:
                spellInfo->speed = 0;
                break;
            case 38325: // Regeneration - trinket Spyglass of the Hidden Fleet
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            // Blizzard ranks 1-7
            case 10:
            case 6141:
            case 8427:
            case 10185:
            case 10186:
            case 10187:
            case 27085:
            // Hurricane
            case 16914:
            case 17401:
            case 17402:
            case 27012:
            // Rain of Fire
            case 5740:
            case 6219:
            case 11677:
            case 11678:
            case 27212:
            // Volley
            case 1510:
            case 14294:
            case 14295:
            case 27022:
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_NO_INITIAL_AGGRO;
                break;
            case 32375: // Mass dispel
            case 32592: // Mass dispel
            case 39897: // Mass dispel
            case 1725: // Distract
            case 14183: // Premeditation
            case 921: // Pick pocket
            case 29858: // Soulshatter AoE dummy effect
            case 32835: // Soulshatter target triggered effect
            case 33619: // Reflective Shield
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_NOT_CONSIDERED_HIT;
                break;
            case 37868: // Arcano-Scorp Control - quest Frankly it makes no sense
                spellInfo->EffectImplicitTargetA[0] = TARGET_UNIT_TARGET_ANY;
                break;
            case 24394: // Intimidation - hunter talent stun. Set spellfamily for right diminishing return
                spellInfo->SpellFamilyName = SPELLFAMILY_HUNTER;
                break;
            case 34510: // Stormherald stun. Set family for right diminishing return;
                spellInfo->SpellFamilyName = SPELLFAMILY_WARRIOR;
                break;
            case 50247: // Path of Illidan
            case 46736: // Personalized Weather
            case 46738: // Personalized Weather
            case 46739: // Personalized Weather
            case 46740: // Personalized Weather
                spellInfo->DurationIndex = 42;
                break;
            /*case 25677: // SnowBall for NY 2017 event - switch knockback effect to dummy effect
                spellInfo->Effect[1] = 0;
                spellInfo->Effect[0] = 3;
                spellInfo->EffectBasePoints[0] = 0;
                break;*/
            case 45828: // AV Marshal's HP/DMG auras
            case 45829:
            case 45831:
            case 45830:
            case 45822: // AV Warmaster's HP/DMG auras
            case 45823:
            case 45824:
            case 45826:
                spellInfo->DurationIndex = 21; // Forever
                spellInfo->Attributes &= ~SPELL_ATTR_PASSIVE; // make them non-passive
                break;
            case 34709: // Shadow Sight
                spellInfo->Attributes |= SPELL_ATTR_NEGATIVE_1;
                spellInfo->Attributes |= SPELL_ATTR_CASTABLE_WHILE_MOUNTED;
                spellInfo->Attributes |= SPELL_ATTR_CASTABLE_WHILE_SITTING;
                spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT;
                break;
            case 23693: // Stormpike's Salvation
            case 22751: // Fury of the Frostwolf
            // on buff-gain make them heal to full health (heal for 17% --- to make up for 20% HP increase)
                spellInfo->Effect[1] = SPELL_EFFECT_HEAL_PCT;
                spellInfo->EffectBasePoints[1] = 17;
                spellInfo->EffectImplicitTargetA[1] = 21; // same as first effect
                spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY; // just to make sure that heal is not reduced by some auras
                break;
            case 7393: // Heal Brother
                spellInfo->EffectBasePoints[1] = 0; // don't heal self. Heal is done by our target via script
                break;
            case 46579: // Deathfrost
                spellInfo->SpellFamilyFlags = 0x0000000100000000LL;
                spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT; // does not put in combat
                break;
			case 46629: // Deathfrost
				spellInfo->AttributesEx |= SPELL_ATTR_EX_NO_THREAT; // does not put in combat
				break;
            case 46605: // Darkness of a Thousand Souls
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DUMMY;
                spellInfo->EffectAmplitude[0] = 8000;

                spellInfo->Effect[2] = 0; // remove effect. It breaks effect [0]
                break;
            case 40303: // Spell Bomb - used by Anzu - make it behave as Energy Storm (43983)
                spellInfo->procFlags = 0x14000;
                break;
            case 43983: // Energy Storm
                spellInfo->EffectTriggerSpell[0] = 43137;
                break;
            case 37116: // Primal Fury rank 1
            case 37117: // Primal Fury rank 2
                spellInfo->Attributes |= SPELL_ATTR_PASSIVE; // must be passive in order to be unlearned on talent reset and not just set 'disabled'
                break;
            case 5024: // Flee - Skull of Impending Doom
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PERIODIC_DAMAGE_PERCENT;
                spellInfo->EffectBasePoints[1] = 12;
                spellInfo->EffectBaseDice[1] = 0;
                spellInfo->EffectDieSides[1] = 0;

                spellInfo->EffectBasePoints[2] = 1;
                spellInfo->EffectBaseDice[2] = 0;
                spellInfo->EffectDieSides[2] = 0;
                break;
            case 31616: // Nature's Guardian talent spell trigger
                spellInfo->AttributesEx2 |= SPELL_ATTR_EX2_TRIGGERED_CAN_TRIGGER;
                break;
            case 45064: // Vessel of the Naaru - Vial of the Sunwell heal - should be able to trigger
                spellInfo->AttributesCu |= SPELL_ATTR_CU_NO_SPELL_DMG_COEFF;
                break;
            case 39476: // Reputation: +500 Sporeggar
                spellInfo->EffectBasePoints[0] = 499;
                break;
            case 22745: // Chains of Ice (mana burn debuff from spell Chains of Ice(22744))
                spellInfo->EffectImplicitTargetA[0] = 1;
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_POWER_BURN_MANA;
                break;
            case 44432:
                spellInfo->EffectApplyAuraName[0] = SPELL_AURA_NONE;
                break;
            case 38383: //  Multi-Shot (Durnholde Rifleman)
                spellInfo->RecoveryTime = 10000;
                break;
            case 39445: //  Vengeance
                spellInfo->DmgClass = SPELL_DAMAGE_CLASS_MAGIC;
                break;
            case 36513: //  Intangible Presence
            case 36631: //  Netherbreath
            case 17012: //  Devour Magic
			case 38932: //  Blink
                spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                break;
			case 12544: // Frost Armor
			    spellInfo->Attributes |= SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY;
                break;
            case 36534: //  Energy Surge
            case 41265: //  Energy Surge
                spellInfo->EffectImplicitTargetA[0] = 1;
                break;
            case 38145: // Vashj: Forked Lightning
                spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_IGNORE_RESISTANCES;
                break;
            case 31287: // Entangling Roots (mob spell)
                spellInfo->EffectAmplitude[1] = 1000;
                break;
            case 6608:  //  Dropped Weapon
                spellInfo->EffectImplicitTargetA[0] = 6;
                break;
            case 24869: // Bobbing Apple and Winter Veil Cookie
                spellInfo->Effect[1] = 6;
                spellInfo->EffectDieSides[1] = 1;
                spellInfo->EffectImplicitTargetA[1] = 1;
                spellInfo->EffectApplyAuraName[1] = SPELL_AURA_PERIODIC_TRIGGER_SPELL;
                spellInfo->EffectAmplitude[1] = 10000;
                spellInfo->EffectTriggerSpell[1] = 24870;
                break;
            case 24870: // Well Fed (Stamina and Spirit increased by 25% of level)
                spellInfo->EffectBasePoints[1] = 0;
                spellInfo->EffectRealPointsPerLevel[0] = 0.249f;
                spellInfo->EffectRealPointsPerLevel[1] = 0.249f;
                break;
            // Arena Drink
            case 430:
            case 431:
            case 432:
            case 1133:
            case 1135:
            case 1137:
            case 10250:
            case 22734:
            case 27089:
            case 34291:
            case 43706:
            case 46755:
                spellInfo->EffectAmplitude[1] = 2000;
                break;
            case 35247: // Choking Wound
                // just add some delay
                spellInfo->AttributesCu |= SPELL_ATTR_CU_FAKE_DELAY;
                break;
            case 35571:
                // fake death aura for BG
                spellInfo->EffectApplyAuraName[0] = 0;
                spellInfo->Attributes |= 0x24800100;
                spellInfo->AttributesEx |= 0x00000088;
                spellInfo->AttributesEx2 |= 0x00000005;
                spellInfo->AttributesEx3 |= 0x10100000;
                spellInfo->AttributesEx4 |= 0x00000004;        
                break;
            case SPELL_GUILD_HOUSE_STATS_BUFF:
                spellInfo->AttributesEx3 |= SPELL_ATTR_EX3_DEATH_PERSISTENT;
                spellInfo->AttributesEx4 |= SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA;
                spellInfo->DurationIndex = 581; // 24h
                break;
			case 36549: //Summon Cauldron Stuff
				spellInfo->EffectMiscValueB[0] = 121;
				spellInfo->Effect[1] = 0;
				spellInfo->Effect[2] = 0;

            // moonwell spell custom attr
            //case 24546:
            //    spellInfo->SpellFamilyName = SPELLFAMILY_GENERIC;
			// spellqsw
            default:
                break;
        }
        // END DEFAULT CASE

        //if (sWorld.isEasyRealm())
        //switch(i)
        //{
        //case 15007: // Resurrection Sickness
        //    spellInfo->DurationIndex = 9;
        //    break;
        //case 1122: // Inferno (Summon)
        //case 712: // Summon Succubus (Summon)
        //case 698: // Ritual of Summoning
        //case 697: // Summon Voidwalker (Summon)
        //case 693: // Create Soulstone (Rank 1)
        //case 691: // Summon Felhunter (Summon)
        //case 130: // Slow Fall
        //case 5699: // Create Healthstone (Rank 3)
        //case 6201: // Create Healthstone (Rank 1)
        //case 6202: // Create Healthstone (Rank 2)
        //case 18867: // Shadowburn (Rank 2)
        //case 18868: // Shadowburn (Rank 3)
        //case 18869: // Shadowburn (Rank 4)
        //case 18870: //  Shadowburn (Rank 5)
        //case 18871: // Shadowburn (Rank 6)
        //case 20608: // Reincarnation (Passive)
        //case 20739: // Rebirth (Rank 2)
        //case 20742: // Rebirth (Rank 3)
        //case 20747: // Rebirth (Rank 4)
        //case 20748: // Rebirth (Rank 5)
        //case 20755: // Create Soulstone (Rank 3)
        //case 20756: // Create Soulstone (Rank 4)
        //case 20757: // Create Soulstone (Rank 5)
        //case 26889: // Vanish (Rank 3)
        //case 26994: // Rebirth (Rank 6)
        //case 46546: // Ritual of Summoning
        //case 1706: // Levitate
        //case 2362: // Create Spellstone (Rank 1)
        //case 30545: // Soul Fire (Rank 4)
        //case 30546: // Shadowburn (Rank 8)
        //case 18540: // Ritual of Doom
        //case 6353: // Soul Fire (Rank 1)
        //case 6366: // Create Firestone (Rank 1)
        //case 27263: // Shadowburn (Rank 7)
        //case 29858: // Soulshatter
        //case 29893: // Ritual of Souls (Rank 1)
        //case 28172: //  Create Spellstone (Rank 4)
        //case 27250: // Create Firestone (Rank 5)
        //case 17727: // Create Spellstone (Rank 2)
        //case 17728: // Create Spellstone (Rank 3)
        //case 17877: //  Shadowburn (Rank 1)
        //case 17924: // Soul Fire (Rank 2)
        //case 17951: // Create Firestone (Rank 2)
        //case 17952: // Create Firestone (Rank 3)
        //case 17953: // Create Firestone (Rank 4)
        //case 19752: // Divine Intervention
        //case 20484: // Rebirth (Rank 1)
        //case 21169: // Reincarnation
        //case 21562: // Prayer of Fortitude (Rank 1)
        //case 21564: // Prayer of Fortitude (Rank 2)
        //case 21849: // Gift of the Wild (Rank 1)
        //case 21850: // Gift of the Wild (Rank 2)
        //case 23028: // Arcane Brilliance (Rank 1)
        //case 27211: // Soul Fire (Rank 3)
        //case 27230: // Create Healthstone (Rank 6)
        //case 27238: // Create Soulstone (Rank 6)
        //case 27681: // Prayer of Spirit (Rank 1)
        //case 27683: // Prayer of Shadow Protection (Rank 1)
        //case 27740: // Reincarnation
        //case 30146: // Summon Felguard (Summon)
        //case 43987: // Ritual of Refreshment (Rank 1)
        //case 11725: // Enslave Demon (Rank 2)
        //case 11726: // Enslave Demon (Rank 3)
        //case 11729: // Create Healthstone (Rank 4)
        //case 11730: // Create Healthstone (Rank 5)
        //    spellInfo->Reagent[0] = 0;
        //    break;
        //case 546: // Water Walking
        //case 131: // Water Breathing
        //case 26991: // Gift of the Wild (Rank 3)
        //case 39374: // Prayer of Shadow Protection (Rank 2)
        //case 25392: // Prayer of Fortitude (Rank 3)
        //case 25895: // Greater Blessing of Salvation
        //case 25898: // Greater Blessing of Kings
        //case 32999: // Prayer of Spirit (Rank 2)
        //case 27127: // Arcane Brilliance (Rank 2)
        //case 27141: // Greater Blessing of Might (Rank 3)
        //case 27143: // Greater Blessing of Wisdom (Rank 3)
        //case 27145: // Greater Blessing of Light (Rank 2)
        //case 27169: // Greater Blessing of Sanctuary (Rank 2)
        //    spellInfo->DurationIndex = 581;
        //    spellInfo->Reagent[0] = 0;
        //    break;
        //case 132: // Detect Invisibility
        //case 5697: // Unending Breath
        //case 18789: // Burning Wish
        //case 18790: // Fel Stamina
        //case 18791: // Touch of Shadow
        //case 18792: // Fel Energy
        //case 20217: // Blessing of Kings
        //case 26990: // Mark of the Wild (Rank 8)
        //case 26992: // Thorns (Rank 7)
        //case 33944: // Dampen Magic (Rank 6)
        //case 33946: // Amplify Magic (Rank 6)
        //case 33736: // Water Shield (Rank 2)
        //case 33721: // Adept's Elixir
        //case 33726: //  Elixir of Mastery
        //case 32594: // Earth Shield (Rank 3)
        //case 39625: // Elixir of Major Fortitude
        //case 39626: //  Earthen Elixir
        //case 39627: // Elixir of Draenic Wisdom
        //case 39628: // Elixir of Ironskin
        //case 25312: // Divine Spirit (Rank 5)
        //case 25389: // Power Word: Fortitude (Rank 7)
        //case 25431: // Inner Fire (Rank 7)
        //case 25433: // Shadow Protection (Rank 4)
        //case 25461: // Touch of Weakness (Rank 7)
        //case 25472: //  Lightning Shield (Rank 9)
        //case 25477: // Shadowguard (Rank 7)
        //case 25780: // Righteous Fury
        //case 1038: // Blessing of Salvation
        //case 16864: // Omen of Clarity
        //case 17538: //  Elixir of the Mongoose
        //case 17627: // Distilled Wisdom
        //case 17628: // Supreme Power
        //case 23735: // Sayge's Dark Fortune of Strength
        //case 23736: // Sayge's Dark Fortune of Agility
        //case 23737: // Sayge's Dark Fortune of Stamina
        //case 23738: // Sayge's Dark Fortune of Spirit
        //case 23766: // Sayge's Dark Fortune of Intelligence
        //case 23767: //  Sayge's Dark Fortune of Armor
        //case 23768: // Sayge's Dark Fortune of Damage
        //case 23769: //  Sayge's Dark Fortune of Resistance
        //case 27124: //  Ice Armor (Rank 5)
        //case 27125: // Mage Armor (Rank 4)
        //case 27126: // Arcane Intellect (Rank 6)
        //case 27140: // Blessing of Might (Rank 8)
        //case 27142: // Blessing of Wisdom (Rank 7)
        //case 27144: // Blessing of Light (Rank 4)
        //case 27168: // Blessing of Sanctuary (Rank 5)
        //case 27260: // Demon Armor (Rank 6)
        //case 28189: // Fel Armor (Rank 2)
        //case 28489: // Camouflage
        //case 28490: // Major Strength
        //case 28491: // Healing Power
        //case 28493: // Major Frost Power
        //case 28496: // Greater Stealth Detection
        //case 28497: // Major Agility
        //case 28501: // Major Firepower
        //case 28502: // Major Armor
        //case 28503: // Major Shadow Power
        //case 28514: // Empowerment
        //case 28518: // Flask of Fortification
        //case 28519: // Flask of Mighty Restoration
        //case 28520: // Flask of Relentless Assault
        //case 28521: // Flask of Blinding Light
        //case 28540: // Flask of Pure Death
        //case 29534: // Traces of Silithyst
        //case 30482: // Molten Armor (Rank 1)
        //case 42735: // Chromatic Wonder
        //case 22807: // Elixir of Greater Water Breathing
        //case 11389: // Elixir of Detect Undead
        //case 11407: // Elixir of Detect Demon
        //case 28509: // Elixir of Major Mageblood
        //    spellInfo->DurationIndex = 581;
        //    break;
        //case 29341: // Shadowburn
        //    spellInfo->EffectItemType[0] = 0;
        //    break;
        //default:
        //    break;
        //}

        //if (sWorld.isEasyRealm())
        //    switch (i)
        //{
        //    case 11479: // all cooldowned-profession things
        //    case 11480:
        //    case 17559:
        //    case 17560:
        //    case 17561:
        //    case 17562:
        //    case 17563:
        //    case 17564:
        //    case 17565:
        //    case 17566:
        //    case 19566:
        //    case 21935:
        //    case 28566:
        //    case 28567:
        //    case 28568:
        //    case 28569:
        //    case 28580:
        //    case 28581:
        //    case 28582:
        //    case 28583:
        //    case 28584:
        //    case 28585:
        //    case 29688:
        //    case 32765:
        //    case 32766:
        //    case 28028:
        //    case 28027:
        //    case 26751:
        //    case 31373:
        //    case 36686:
        //        spellInfo->RecoveryTime = 0;
        //        spellInfo->StartRecoveryTime = 0;
        //        spellInfo->CategoryRecoveryTime = 0;
        //        break;
        //}
        //    case 8690: // Hearthstone
        //        spellInfo->Category = 0;
        //        spellInfo->CategoryRecoveryTime = 0;
        //        spellInfo->Effect[2] = 64;
        //        spellInfo->EffectImplicitTargetA[2] = 1;
        //        spellInfo->EffectTriggerSpell[2] = 54741;
        //        break;
        //    case 29720: // Greater Ward of Shielding
        //    case 17624: // Flask of Petrification
        //    case 6615:  // Free Action Potion
        //    case 24364: // Living Action Potion
        //    case 43730: // Stormchops
        //    case 11350: // Oil of Immolation
        //        spellInfo->AttributesEx6 |= SPELL_ATTR_EX6_BG_ONLY_CUSTOM;
        //        break;
        //    case 13241: // Goblin Sapper Charge
        //    case 13180: // Gnomish Mind Control Cap
        //    case 19821: // Arcane Bomb
        //    case 30461: // The Bigger One
        //    case 4100:  // Goblin Land Mine
        //    case 30486: // Super Sapper Charge
        //    case 30526: // Gnomish Flame Turret
        //    case 19769: // Thorium Grenade
        //    case 30217: // Adamantite Grenade
        //    case 30216: // Fel Iron Bomb
        //    case 19784: // Dark Iron Bomb
        //    case 12562: // The Big One
        //    case 30458: // Nigh Invulnerability Belt
        //    case 35476: // Drums of Battle
        //    case 29528: // Drums of War
        //    case 35478: // Drums of Restoration
        //    case 40724: // Crystalforged Trinket
        //        spellInfo->AttributesEx6 |= SPELL_ATTR_EX6_BG_DISALLOW_19LVL;
        //        break;
        //    /* Release: i can't imagine the purpose of this.
        //    case 1122: // Inferno (Summon)
        //        spellInfo->EffectMiscValue[0] = 40089;
        //        break;
        //    */
        //    default:
        //        break;
        //}

        switch (season)
        {
            case SEASON_1:
            case SEASON_2:
            case SEASON_3:
            case SEASON_4:
            {
                switch (i)
                {
                    case 18560: // Mooncloth 92 hours cooldown
                    {
                        spellInfo->RecoveryTime = 92 * HOUR * MILLISECONDS;
                        break;
                    }
                    case 17187: // Transmute: Arcanite 20 hours cooldown
                    {
                        spellInfo->RecoveryTime = 20 * HOUR * MILLISECONDS;
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            /*case SEASON_FOUR_2_4:
            {
                switch (i)
                {
                    default:
                        break;
                }
                break;
            }*/
            default:
                break;
        }

        if (mSpellAnalogMap.find(spellInfo->Id) != mSpellAnalogMap.end()) // can't use GetSpellAnalog here, cause it checks on attribute
            spellInfo->AttributesEx6 |= SPELL_ATTR_EX6_HAS_ANALOG;

        if (spellInfo->AttributesEx6 & SPELL_ATTR_EX6_NOT_IN_RAID_INSTANCE)
            spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_IN_RAID_INSTANCE;
        if (spellInfo->AttributesEx6 & SPELL_ATTR_EX6_NOT_PVP)
            spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP;
        if (spellInfo->AttributesEx6 & SPELL_ATTR_EX6_NOT_HUNTER_PVP)
            spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_HUNTER_PVP;
        if (spellInfo->AttributesEx6 & SPELL_ATTR_EX6_ONLY_IN_RAID)
            spellInfo->AuraInterruptFlags |= AURA_INTERRUPT_FLAG_ONLY_IN_RAID;
    }
    CreatureAI::FillAISpellEntry();
}

// TODO: move this to database along with slot position in cast bar
void SpellMgr::LoadCustomSpellCooldowns(SpellEntry* spellInfo)
{
    if (!spellInfo)
        return;

    switch (spellInfo->Id)
    {
        // 2 sec cooldown
        case 22907:     // Shoot
        case 35946:     // Shoot
        case 44533:     // Wretched Stab
            spellInfo->RecoveryTime = 2000;
            break;
        // 6 sec cooldown
        case 44639:     // Frost Arrow
        case 46082:     // Shadow Bolt Volley
            spellInfo->RecoveryTime = 6000;
            break;
        // 8 sec cooldown
        case 44534:     // Wretched Strike
        case 44640:     // Lash of Pain
            spellInfo->RecoveryTime = 8000;
            break;
        // 10 sec cooldown
        case 44518:     // Immolate
        case 46042:     // Immolate
        case 44479:     // holy Light
        case 46029:     // Holy Light
            spellInfo->RecoveryTime = 10000;
            break;
        // 12 sec cooldown
        case 44547:     // Deadly Embrace
        case 44599:     // Inject Poison
        case 46046:     // Inject Poison
            spellInfo->RecoveryTime = 12000;
            break;
        // 15 sec cooldown
        case 44478:     // Glaive Throw
        case 46028:     // Glaive Throw
        case 20299:     // Forked Lightning
        case 46150:     // Forked Lightning
        case 17741:     // Mana Shield
        case 46151:     // Mana Shield
            spellInfo->RecoveryTime = 15000;
            break;
        // 20 sec cooldown
        case 44480:     // Seal of Wrath
        case 46030:     // Seal of Wrath
            spellInfo->RecoveryTime = 20000;
            break;
        // 24 sec cooldown
        case 44505:     // Drink Fel Infusion
            spellInfo->RecoveryTime = 24000;
            break;
        // 30 sec cooldown
        case 44475:
            spellInfo->RecoveryTime = 30000;
            break;
        // 3 min cooldown
        case 30015: // Summon Naias
        case 35413: // Summon Goliathon
            spellInfo->RecoveryTime = 300000;
            break;
        // 30 min cooldown
        case 44520:
            spellInfo->RecoveryTime = 1800000;
            break;
        // 3h cooldown
        case 16054: // Flames of the Black Flight
            spellInfo->RecoveryTime = 5000;
            break;
        case 44935: //Expose Razorthorn Root
        case 29992: //Quest spell - needs cooldown to be able to add to possessed unit
            spellInfo->RecoveryTime = 1080;
            break;
        case 32096: // Thrallmar's Favor
            spellInfo->EffectBasePoints[0] = 24;
            spellInfo->EffectApplyAuraName[0] = 190;
            spellInfo->EffectMiscValue[0] = 947;
            spellInfo->Effect[1] = 6;
            spellInfo->EffectBasePoints[1] = 4;
            spellInfo->EffectImplicitTargetA[1] = 1;
            spellInfo->EffectApplyAuraName[1] = 200;
            spellInfo->EffectBaseDice[1] = 1;
            break;
        case 32098: // Honor Hold's Favor
            spellInfo->EffectBasePoints[0] = 24;
            spellInfo->EffectApplyAuraName[0] = 190;
            spellInfo->EffectMiscValue[0] = 946;
            spellInfo->Effect[1] = 6;
            spellInfo->EffectBasePoints[1] = 4;
            spellInfo->EffectImplicitTargetA[1] = 1;
            spellInfo->EffectApplyAuraName[1] = 200;
            spellInfo->EffectBaseDice[1] = 1;
            break;
        default:
            break;
    }
}

void SpellMgr::LoadSpellLinked()
{
    mSpellLinkedMap.clear();    // need for reload case
    uint32 count = 0;

    //                                                       0              1             2
    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT spell_trigger, spell_effect, type FROM spell_linked_spell");
    if (!result)
    {
        //BarGoLink bar(1);
        //bar.step();
        sLog.outString();
        sLog.outString(">> Loaded %u linked spells", count);
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        //bar.step();

        int32 trigger = fields[0].GetInt32();
        int32 effect = fields[1].GetInt32();
        int32 type = fields[2].GetInt32();

        SpellEntry *strigger = (SpellEntry *)sSpellTemplate.LookupEntry<SpellEntry>(abs(trigger));
        if (!strigger)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_linked_spell` does not exist", abs(trigger));
            continue;
        }
        SpellEntry *seffect = (SpellEntry *)sSpellTemplate.LookupEntry<SpellEntry>(abs(effect));
        if (!seffect)
        {
            sLog.outLog(LOG_DB_ERR, "Spell %u listed in `spell_linked_spell` does not exist", abs(effect));
            continue;
        }

        if (trigger > 0)
        {
            switch (type)
            {
                case 0: strigger->AttributesCu |= SPELL_ATTR_CU_LINK_CAST; break;
                case 1: strigger->AttributesCu |= SPELL_ATTR_CU_LINK_HIT;  break;
                case 2: strigger->AttributesCu |= SPELL_ATTR_CU_LINK_AURA; break;
                case 3: strigger->AttributesCu |= SPELL_ATRR_CU_LINK_PRECAST; break;
            }
        }
        else
        {
            strigger->AttributesCu |= SPELL_ATTR_CU_LINK_REMOVE;
        }

        if (type) //we will find a better way when more types are needed
        {
            if (trigger > 0)
                trigger += SPELL_LINKED_MAX_SPELLS * type;
            else
                trigger -= SPELL_LINKED_MAX_SPELLS * type;
        }
        mSpellLinkedMap[trigger].push_back(effect);

        ++count;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u linked spells", count);
}

/// Some checks for spells, to prevent adding depricated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if (!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for (int i=0; i<3; ++i)
    {
        switch (spellInfo->Effect[i])
        {
            case 0:
                continue;

                // craft spell for crafting non-existed item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (!ObjectMgr::GetItemPrototype(spellInfo->EffectItemType[i]))
                {
                    if (msg)
                    {
                        if (pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                        else
                            sLog.outLog(LOG_DB_ERR, "Craft spell %u create not-exist in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellTemplate.LookupEntry<SpellEntry>(spellInfo->EffectTriggerSpell[i]);
                if (!IsSpellValid(spellInfo2,pl,msg))
                {
                    if (msg)
                    {
                        if (pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[i]);
                        else
                            sLog.outLog(LOG_DB_ERR, "Spell %u learn to invalid spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[i]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if (need_check_reagents)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (spellInfo->Reagent[j] > 0 && !ObjectMgr::GetItemPrototype(spellInfo->Reagent[j]))
            {
                if (msg)
                {
                    if (pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                    else
                        sLog.outLog(LOG_DB_ERR, "Craft spell %u have not-exist reagent in DB item (Entry: %u) and then...",spellInfo->Id,spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

bool SpellMgr::IsSpellAllowedInLocation(SpellEntry const *spellInfo,uint32 map_id,uint32 zone_id,uint32 area_id)
{
    // hack moved from Player::UpdateAreaDependentAuras <--- is still needed ? Oo i don't think so ...
    if (spellInfo->Id == 38157)
        if (area_id == 3522 || area_id == 3785)
            return true;

    // normal case
    if (spellInfo->AreaId && spellInfo->AreaId != zone_id && spellInfo->AreaId != area_id)
        return false;

    // elixirs (all area dependent elixirs have family SPELLFAMILY_POTION, use this for speedup)
    if (spellInfo->SpellFamilyName==SPELLFAMILY_POTION)
    {
        if (uint32 mask = sSpellMgr.GetSpellElixirMask(spellInfo->Id))
        {
            if (mask & ELIXIR_BATTLE_MASK)
            {
                if (spellInfo->Id==45373)                    // Bloodberry Elixir
                    return zone_id==4075;
            }
            if (mask & ELIXIR_UNSTABLE_MASK)
            {
                // in the Blade's Edge Mountains Plateaus and Gruul's Lair.
                return zone_id ==3522 || map_id==565;
            }
            if (mask & ELIXIR_SHATTRATH_MASK)
            {
                // in Tempest Keep, Serpentshrine Cavern, Caverns of Time: Mount Hyjal, Black Temple, Sunwell Plateau
                if (zone_id ==3607 || map_id==534 || map_id==564 || zone_id==4075)
                    return true;

                MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
                if (!mapEntry)
                    return false;

                return mapEntry->multimap_id==206;
            }

            // elixirs not have another limitations
            return true;
        }
    }

    // special cases zone check (maps checked by multimap common id) SPELL_CHECK_ARENA_AND_OR_ZONE
    switch (spellInfo->Id)
    {
        case 45403:
        case 45401:
        {
            if (map_id == 580 || map_id == 585)
                return true;

            if (map_id == 530 && zone_id == 4080)
                return true;

            return false;
        }
        case 48025: // Headless Horseman's Mount
        case 42684: // Swift Magic Broom
        case 42683:
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsMountAllowed())
                return false;

            return true;
        }
        case 23333:                                         // Warsong Flag
        case 23335:                                         // Silverwing Flag
        case 46392:                                         // Focused Assault
        case 46393:                                         // Brutal Assault
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsBattleGround())
                return false;

            if (zone_id == 3277)
                return true;

            return false;
        }
        // BG only
        case SPELL_BG_RATED:
        case SPELL_BG_ALLIANCE_ICON:
        case SPELL_BG_HORDE_ICON:
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (mapEntry->IsBattleGround())
                return true;

            return false;
        }
        case 34976:                                         // Netherstorm Flag
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsBattleGround())
                return false;

            if (zone_id == 3820)
                return true;

            return false;
        }
        case SPELL_AV_CROSSFAC_A_H:
        case SPELL_AV_CROSSFAC_H_A:
            return map_id == 30/*Alterac Valley*/;
        case 32307:                                         // Warmaul Ogre Banner
            return area_id == 3637;
        case 32724:                                         // Gold Team (Alliance)
        case 32725:                                         // Green Team (Alliance)
        case SPELL_ARENA_PREPARATION:                       // Arena Preparation
        case 35774:                                         // Gold Team (Horde)
        case 35775:                                         // Green Team (Horde)
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            //the follow code doesn't work.
            //if(!mapEntry->IsBattleArena())
            //    return false;

            //this is the working code, HACK
            if (zone_id == 3702 || zone_id == 3968 || zone_id == 3698)
                return true;

            return false;
        }
        case 41618:                                         // Bottled Nethergon Energy
        case 41620:                                         // Bottled Nethergon Vapor
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            return mapEntry->multimap_id==206;
        }
        case 41617:                                         // Cenarion Mana Salve
        case 41619:                                         // Cenarion Healing Salve
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            return mapEntry->multimap_id==207;
        }
        case 40216:                                         // Dragonmaw Illusion
        case 42016:                                         // Dragonmaw Illusion
            return area_id == 3759 || area_id == 3966 || area_id == 3939 || area_id == 3965 || area_id == 3967;
        case 2584:                                          // Waiting to Resurrect
        case 22011:                                         // Spirit Heal Channel
        case 22012:                                         // Spirit Heal
        case 24171:                                         // Resurrection Impact Visual
        case 42792:                                         // Recently Dropped Flag
        case SPELL_AURA_BG_PLAYER_IDLE:                     // Inactive
        case 44535:                                         // Spirit Heal (mana)
        case 44521:                                         // Preparation
        case 55162:                                         // BG: Heal Self
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (!mapEntry->IsBattleGround())
                return false;
            break; // goes to return true;
        }
        case 55170: // Warrior GCD and cast time preparation
        case 55171: // Paladin GCD and cast time preparation
        case 55172: // Hunter GCD and cast time preparation
        case 55173: // Rogue GCD and cast time preparation
        case 55174: // Priest GCD and cast time preparation
        case 55176: // Shaman GCD and cast time preparation
        case 55177: // Mage GCD and cast time preparation
        case 55178: // Warlock GCD and cast time preparation
        case 55180: // Druid GCD and cast time preparation
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (mapEntry->IsBattleGround())
                return true;

            //the follow code doesn't work.
            //if(!mapEntry->IsBattleArena())
            //    return false;

            //this is the working code, HACK
            if (zone_id == 3702 || zone_id == 3968 || zone_id == 3698)
                return true;

            return false;
        }
        // karazhan books
        case 30562:
        case 30550:
        case 30567:
        case 30557:
            return map_id == 532;
        // Zul'Aman Amani Charms
        case 43818:
        case 43816:
        case 43822:
        case 43820:
            return map_id == 568;
        // Ritual of summoning in Zul'Aman
        //case 698:
        //    return map_id != 568;
        case 32096:
        case 32098:
        case 39911:
        case 39913:
        {
            if (zone_id == 3483)
                return true;

            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            return mapEntry->multimap_id == 199 || mapEntry->multimap_id == 208;
        }
        case 39953:
        {
            if (zone_id == 3703) // zone shatt
                return true;

            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            return mapEntry->multimap_id == 206;
        }
        case 40817: // summon banishing portal
            return (area_id == 3785) || (area_id == 3784);
        case 42766: // Fishing Chair is not usable in instances
            return (map_id == 530 || map_id == 0 || map_id == 1);

        case 1122: // infernal
        case 18540: // doom
            return (zone_id != 3703 && zone_id != 1637 && zone_id != 1519);

            // BATTLEGROUND case
        case 49844: // Personal Mole Machine (190022)
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return false;

            if (mapEntry->IsBattleGround())
                return false;
        }

    }

    return true;
}

void SpellMgr::LoadSkillLineAbilityMap()
{
    mSkillLineAbilityMap.clear();

    uint32 count = 0;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); i++)
    {
        SkillLineAbilityEntry const *SkillInfo = sSkillLineAbilityStore.LookupEntry(i);
        if (!SkillInfo)
            continue;

        mSkillLineAbilityMap.insert(SkillLineAbilityMap::value_type(SkillInfo->spellId,SkillInfo));
        ++count;
    }

    sLog.outString();
    sLog.outString(">> Loaded %u SkillLineAbility MultiMap", count);
}

SpellEntry const* SpellMgr::GetHighestSpellRankForPlayer(uint32 spellId, Player* player)
{
    SpellEntry const* highest_rank = NULL;

    SpellChainNode const* node = sSpellMgr.GetSpellChainNode(sSpellMgr.GetLastSpellInChain(spellId));

    // should work for spells with multiple ranks
    if (node)
    {
        while (node)
        {
            if (player->HasSpell(node->cur))
                if (highest_rank = sSpellTemplate.LookupEntry<SpellEntry>(node->cur))
                    break;

            node = sSpellMgr.GetSpellChainNode(node->prev);
        }
    }
    // if spell don't have ranks check spell itself
    else if (player->HasSpell(spellId))
        highest_rank = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    return highest_rank;
}

float SpellMgr::GetSpellRadiusForHostile(SpellRadiusEntry const *radius)
{
    return (radius ? radius->radiusHostile : 0);
}

float SpellMgr::GetSpellRadiusForFriend(SpellRadiusEntry const *radius)
{
    return (radius ? radius->radiusFriend : 0);
}

float SpellMgr::GetSpellMaxRange( SpellEntry const *spellInfo )
{
    return SpellMgr::GetSpellMaxRange(sSpellRangeStore.LookupEntry(spellInfo->rangeIndex));
}

float SpellMgr::GetSpellMaxRange( uint32 id )
{
    SpellEntry const *spellInfo = GetSpellStore()->LookupEntry<SpellEntry>(id);
    if (!spellInfo) return 0;
    return GetSpellMaxRange(spellInfo);
}

float SpellMgr::GetSpellMaxRange( SpellRangeEntry const *range )
{
    return (range ? range->maxRange : 0);
}

float SpellMgr::GetSpellMinRange( SpellEntry const *spellInfo )
{
    return SpellMgr::GetSpellMinRange(sSpellRangeStore.LookupEntry(spellInfo->rangeIndex));
}

float SpellMgr::GetSpellMinRange( uint32 id )
{
    SpellEntry const *spellInfo = GetSpellStore()->LookupEntry<SpellEntry>(id);
    if (!spellInfo) return 0;
    return SpellMgr::GetSpellMinRange(spellInfo);
}

float SpellMgr::GetSpellMinRange( SpellRangeEntry const *range )
{
    return (range ? range->minRange : 0);
}

SpellRangeType SpellMgr::GetSpellRangeType( SpellRangeEntry const *range )
{
    return (range ? range->type : SPELL_RANGE_DEFAULT);
}

uint32 SpellMgr::GetSpellRecoveryTime( SpellEntry const *spellInfo )
{
    return spellInfo->RecoveryTime > spellInfo->CategoryRecoveryTime ? spellInfo->RecoveryTime : spellInfo->CategoryRecoveryTime;
}

float SpellMgr::GetSpellRadius( SpellEntry const *spellInfo, uint32 effectIdx, bool positive )
{
    return positive
        ? SpellMgr::GetSpellRadiusForFriend(sSpellRadiusStore.LookupEntry(spellInfo->EffectRadiusIndex[effectIdx]))
        : SpellMgr::GetSpellRadiusForHostile(sSpellRadiusStore.LookupEntry(spellInfo->EffectRadiusIndex[effectIdx]));
}

bool SpellMgr::IsSealSpell( SpellEntry const *spellInfo )
{
    // Seal of Command should meet requirements, but mysteriously is not
    if (spellInfo->Id == 41469)
        return true;
    //Collection of all the seal family flags. No other paladin spell has any of those.
    return spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN &&
        (spellInfo->SpellFamilyFlags & 0x4000A000200LL);
}

bool SpellMgr::IsElementalShield( SpellEntry const *spellInfo )
{
    // family flags 10 (Lightning), 42 (Earth), 37 (Water), proc shield from T2 8 pieces bonus
    return (spellInfo->SpellFamilyFlags & 0x42000000400LL) || spellInfo->Id == 23552;
}

bool SpellMgr::IsPassiveSpellStackableWithRanks( SpellEntry const* spellProto )
{
    if (!SpellMgr::IsPassiveSpell(spellProto))
        return false;

    return !spellProto->HasEffect(SPELL_EFFECT_APPLY_AURA);
}

bool SpellMgr::IsDeathOnlySpell( SpellEntry const *spellInfo )
{
    return spellInfo->AttributesEx3 & SPELL_ATTR_EX3_CAST_ON_DEAD
        || spellInfo->Id == 2584;
}

bool SpellMgr::IsDeathPersistentSpell( SpellEntry const *spellInfo )
{
    if (!spellInfo)
        return false;

    return spellInfo->AttributesEx3 & SPELL_ATTR_EX3_DEATH_PERSISTENT;
}

bool SpellMgr::IsNonCombatSpell( SpellEntry const *spellInfo )
{
    return ((spellInfo->Attributes & SPELL_ATTR_CANT_USED_IN_COMBAT) != 0);
}

bool SpellMgr::IsAreaOfEffectSpell( SpellEntry const *spellInfo )
{
    if (IsAreaEffectTarget[spellInfo->EffectImplicitTargetA[0]] || IsAreaEffectTarget[spellInfo->EffectImplicitTargetB[0]])
        return true;
    if (IsAreaEffectTarget[spellInfo->EffectImplicitTargetA[1]] || IsAreaEffectTarget[spellInfo->EffectImplicitTargetB[1]])
        return true;
    if (IsAreaEffectTarget[spellInfo->EffectImplicitTargetA[2]] || IsAreaEffectTarget[spellInfo->EffectImplicitTargetB[2]])
        return true;
    return false;
}

bool SpellMgr::IsAreaAuraEffect( uint32 effect )
{
    if (effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY    ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND   ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY    ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_PET      ||
        effect == SPELL_EFFECT_APPLY_AREA_AURA_OWNER)
        return true;

    return false;
}

bool SpellMgr::IsDispel( SpellEntry const *spellInfo )
{
    //spellsteal is also dispel
    if (spellInfo->Effect[0] == SPELL_EFFECT_DISPEL ||
        spellInfo->Effect[1] == SPELL_EFFECT_DISPEL ||
        spellInfo->Effect[2] == SPELL_EFFECT_DISPEL)
        return true;
    return false;
}

bool SpellMgr::IsDispelSpell( SpellEntry const *spellInfo )
{
    //spellsteal is also dispel
    if (spellInfo->Effect[0] == SPELL_EFFECT_STEAL_BENEFICIAL_BUFF ||
        spellInfo->Effect[1] == SPELL_EFFECT_STEAL_BENEFICIAL_BUFF ||
        spellInfo->Effect[2] == SPELL_EFFECT_STEAL_BENEFICIAL_BUFF
        || SpellMgr::IsDispel(spellInfo))
        return true;

    return false;
}

bool SpellMgr::IsCritterSummonSpell(SpellEntry const* spellInfo)
{
    if ((spellInfo->Effect[0] == SPELL_EFFECT_SUMMON && spellInfo->EffectMiscValueB[0] == 407) ||
        (spellInfo->Effect[1] == SPELL_EFFECT_SUMMON && spellInfo->EffectMiscValueB[1] == 407) ||
        (spellInfo->Effect[2] == SPELL_EFFECT_SUMMON && spellInfo->EffectMiscValueB[2] == 407))
        return true;

    return false;
}

bool SpellMgr::isSpellBreakCasterStealth( SpellEntry const* spellInfo )
{
    return spellInfo && !(spellInfo->AttributesEx & SPELL_ATTR_EX_NOT_BREAK_CASTER_STEALTH);
}

bool SpellMgr::IsChanneledSpell( SpellEntry const* spellInfo )
{
    return spellInfo && spellInfo->AttributesEx & (SPELL_ATTR_EX_CHANNELED_1 | SPELL_ATTR_EX_CHANNELED_2);
}

bool SpellMgr::NeedsComboPoints( SpellEntry const* spellInfo )
{
    return spellInfo && spellInfo->AttributesEx & (SPELL_ATTR_EX_REQ_COMBO_POINTS1 | SPELL_ATTR_EX_REQ_COMBO_POINTS2);
}

SpellSchoolMask SpellMgr::GetSpellSchoolMask( SpellEntry const* spellInfo )
{
    return SpellSchoolMask(spellInfo->SchoolMask);
}

uint32 SpellMgr::GetSpellMechanicMask( SpellEntry const* spellInfo)
{
    uint32 mask = 0;
    if (spellInfo->Mechanic)
        mask |= 1<<spellInfo->Mechanic;
    return mask;
}

uint32 SpellMgr::GetEffectMechanicMask( SpellEntry const* spellInfo, int32 effect )
{
    uint32 mask = 0;
    if (spellInfo->EffectMechanic[effect])
        mask |= 1<<spellInfo->EffectMechanic[effect];
    return mask;
}

Mechanics SpellMgr::GetSpellMechanic( SpellEntry const* spellInfo)
{
    if (spellInfo->Mechanic)
        return Mechanics(spellInfo->Mechanic);
    return MECHANIC_NONE;
}

Mechanics SpellMgr::GetEffectMechanic( SpellEntry const* spellInfo, int32 effect )
{
    if (spellInfo->EffectMechanic[effect])
        return Mechanics(spellInfo->EffectMechanic[effect]);
    return MECHANIC_NONE;
}

uint32 SpellMgr::GetDispellMask( DispelType dispel )
{
    // If dispel all
    if (dispel == DISPEL_ALL)
        return DISPEL_ALL_MASK;
    else
        return (1 << dispel);
}

bool SpellMgr::IsPrimaryProfessionSkill( uint32 skill )
{
    SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(skill);
    if (!pSkill)
        return false;

    if (pSkill->categoryId != SKILL_CATEGORY_PROFESSION)
        return false;

    return true;
}

bool SpellMgr::IsProfessionSkill( uint32 skill )
{
    return SpellMgr::IsPrimaryProfessionSkill(skill) || skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
}

DiminishingGroup SpellMgr::GetDiminishingReturnsGroupForSpell(SpellEntry const* spellproto, bool triggered)
{
    if (!spellproto)
        return DIMINISHING_NONE;

    // Explicit Diminishing Groups
    switch (spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_MAGE:
        {
            // Polymorph
            if ((spellproto->SpellFamilyFlags & 0x00001000000LL) && spellproto->EffectApplyAuraName[0]==SPELL_AURA_MOD_CONFUSE)
                return DIMINISHING_POLYMORPH;
            else if (spellproto->Id == 33395) // water element freeze
                return DIMINISHING_CONTROL_ROOT;
            else if (spellproto->Id == 12494) // frostbite
                return DIMINISHING_TRIGGER_ROOT;
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Kidney Shot
            if (spellproto->SpellFamilyFlags & 0x00000200000LL)
                return DIMINISHING_KIDNEYSHOT;
            // Sap
            else if (spellproto->SpellFamilyFlags & 0x00000000080LL)
                return DIMINISHING_POLYMORPH;
            // Gouge
            else if (spellproto->SpellFamilyFlags & 0x00000000008LL)
                return DIMINISHING_POLYMORPH;
            // Blind
            else if (spellproto->SpellFamilyFlags & 0x00001000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Freezing trap
            if (spellproto->SpellFamilyFlags & 0x00000000008LL)
                return DIMINISHING_FREEZE;
            // Intimidation
            else if (spellproto->Id == 24394)
                return DIMINISHING_CONTROL_STUN;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Death Coil
            if (spellproto->SpellFamilyFlags & 0x00000080000LL)
                return DIMINISHING_DEATHCOIL;
            // Seduction
            else if (spellproto->Id == 6358)
                return DIMINISHING_FEAR;
            // Fear
            // else if (spellproto->SpellFamilyFlags & 0x40840000000LL)
            //    return DIMINISHING_WARLOCK_FEAR;
            // Curse of Tongues
            else if ((spellproto->SpellFamilyFlags & 0x00080000000LL) && (spellproto->SpellIconID == 692))
                return DIMINISHING_LIMITONLY;
            else if (spellproto->SpellFamilyFlags & 0x800000000000LL) // curse of tongues with new flag
                return DIMINISHING_LIMITONLY;
            // Unstable Affliction dispel silence
            else if (spellproto->Id == 31117)
                return DIMINISHING_UNSTABLE_AFFLICTION;
            // Enslave deamon
            else if(spellproto->SpellFamilyFlags & 0x800LL)
                return DIMINISHING_ENSLAVE;
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Cyclone
            if (spellproto->SpellFamilyFlags & 0x02000000000LL)
                return DIMINISHING_BLIND_CYCLONE;
            // Nature's Grasp trigger
            else if (spellproto->SpellFamilyFlags & 0x00000000200LL && spellproto->Attributes == 0x49010000)
                return DIMINISHING_CONTROL_ROOT;
            // feral charge effect should not be in any dr
            if (spellproto->Id == 45334)
                return DIMINISHING_NONE;
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Hamstring - limit duration to 10s in PvP
            if (spellproto->SpellFamilyFlags & 0x00000000002LL)
                return DIMINISHING_LIMITONLY;
            else if (spellproto->Id == 34510) // Stormherald stun
                return DIMINISHING_TRIGGER_STUN;
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Turn Evil - share group with fear, seduction
            if (spellproto->Id == 10326)
                return DIMINISHING_FEAR;
            else if (spellproto->SpellIconID == 316) // Repentance
                return DIMINISHING_POLYMORPH;
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            if (spellproto->SpellFamilyFlags & 0x00000000200LL) // Chastise
                return DIMINISHING_NONE;
            break;
        }
        case SPELLFAMILY_POTION:
            return DIMINISHING_NONE;
        default:
            break;
    }

	// special cases
	switch (spellproto->Id)
	{
		case 37029: // Remote Toy stun (TK)
			return DIMINISHING_NONE;
	}

    Mechanics mech = SpellMgr::GetSpellMechanic(spellproto);
    if (mech == MECHANIC_NONE) // try to find it in effects
    {
    // Get by mechanic
        for (uint8 i=0;i<3;++i)
        {
            if (mech = SpellMgr::GetEffectMechanic(spellproto, i))
                break; // found something
        }
    }

    switch (mech)
    {
        case MECHANIC_STUN:
            return triggered ? DIMINISHING_TRIGGER_STUN : DIMINISHING_CONTROL_STUN;
        case MECHANIC_SLEEP:
            return DIMINISHING_SLEEP;
        case MECHANIC_ROOT:
            return triggered ? DIMINISHING_TRIGGER_ROOT : DIMINISHING_CONTROL_ROOT;
        case MECHANIC_FEAR:
            return DIMINISHING_FEAR;
        case MECHANIC_CHARM:
            return DIMINISHING_CHARM;
        case MECHANIC_DISARM:
            return DIMINISHING_DISARM;
        case MECHANIC_FREEZE:
            return DIMINISHING_FREEZE;
        case MECHANIC_KNOCKOUT:
        case MECHANIC_SAPPED:
            return DIMINISHING_KNOCKOUT;
        case MECHANIC_BANISH:
            return DIMINISHING_BANISH;
    }

    return DIMINISHING_NONE;
}

bool SpellMgr::IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_FREEZE:
        case DIMINISHING_KNOCKOUT:
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_BANISH:
        case DIMINISHING_LIMITONLY:
            return true;
    }
    return false;
}

DiminishingReturnsType SpellMgr::GetDiminishingReturnsGroupType(DiminishingGroup group)
{
    switch (group)
    {
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
            return DRTYPE_ALL;
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_UNSTABLE_AFFLICTION:
        case DIMINISHING_DISARM:
        case DIMINISHING_DEATHCOIL:
        case DIMINISHING_FREEZE:
        case DIMINISHING_BANISH:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_KNOCKOUT:
            return DRTYPE_PLAYER;
    }

    return DRTYPE_NONE;
}

bool SpellMgr::SpellIgnoreLOS(SpellEntry const* spellproto, uint8 effIdx)
{
    if (spellproto->AttributesEx2 & SPELL_ATTR_EX2_IGNORE_LOS)
        return true;

    if (SpellMgr::IsSplashBuffAura(spellproto))
        return true;

    // Most QuestItems should ommit los ;]
    if (spellproto->Effect[effIdx] == SPELL_EFFECT_DUMMY && spellproto->NeedFillTargetMapForTargets(effIdx))
        return true;

    if (spellproto->EffectImplicitTargetA[effIdx] == TARGET_DEST_CHANNEL && spellproto->AttributesEx3 & SPELL_ATTR_EX3_DYNOBJ_TRIGGERED_LOS_IGNORE)
        return true;

    return false;
}

bool SpellMgr::EffectCanScaleWithLevel(const SpellEntry * spellInfo, uint8 eff)
{
    if (!(spellInfo->Attributes & SPELL_ATTR_LEVEL_DAMAGE_CALCULATION) || !spellInfo->spellLevel)
        return false;

    if (spellInfo->Effect[eff] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE ||
        spellInfo->Effect[eff] == SPELL_EFFECT_KNOCK_BACK ||
        spellInfo->Effect[eff] == SPELL_EFFECT_ADD_EXTRA_ATTACKS)
        return false;

    if (spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_SPEED_ALWAYS ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_SPEED_NOT_STACK ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_INCREASE_SPEED ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_DECREASE_SPEED ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_HASTE_MELEE ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_HASTE ||
        spellInfo->EffectApplyAuraName[eff] == SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN)
        return false;

    //there are many more than above: slow speed, -healing pct

    return true;
}

bool SpellMgr::CanSpellCrit(const SpellEntry* spellInfo)
{
    if (spellInfo->AttributesEx2 & SPELL_ATTR_EX2_CANT_CRIT)
        return false;

    switch (spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_NONE:
            switch (spellInfo->Id)// We need more spells to find a general way (if there is any)
            {
                case 379:   // Earth Shield
                case 33778: // Lifebloom
                case 45064: // Vessel of the Naaru
                    break;
                default:
                    return false;
            } // fall through to break
        case SPELL_DAMAGE_CLASS_MAGIC:
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            break;
        default:
            return false;
    }

    for (uint8 eff=0; eff<3; ++eff)
        switch(spellInfo->Effect[eff])
        {
            case SPELL_EFFECT_HEAL:
            case SPELL_EFFECT_HEALTH_LEECH:
            case SPELL_EFFECT_HEAL_MECHANICAL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                return true;
        }

    return false;
}

uint64 SpellMgr::GetSpellAffectMask(uint16 spellId, uint8 effectId) const
{
    SpellAffectMap::const_iterator itr = mSpellAffectMap.find((spellId<<8) + effectId);
    if (itr != mSpellAffectMap.end())
        return itr->second;
    return 0;
}

bool SpellMgr::IsPositionTarget(uint32 target)
{
    switch (SpellTargetType[target])
    {
    case TARGET_TYPE_DEST_CASTER:
    case TARGET_TYPE_DEST_TARGET:
    case TARGET_TYPE_DEST_DEST:
        return true;
    default:
        break;
    }
    return false;
}

bool SpellMgr::IsTauntSpell(SpellEntry const* spellInfo)
{
    if (!spellInfo)
        return false;

    for (uint8 i = 0; i < 3; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_ATTACK_ME)
            return true;
        else if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_TAUNT)
            return true;
    }
    return false;
}

uint32 SpellMgr::GetSpellAnalog(SpellEntry const* spellInfo, Unit* const caster, Unit* target, bool IsSpellStart)
{
    if (!spellInfo)
        return 0;

    if (!(spellInfo->AttributesEx6 & SPELL_ATTR_EX6_HAS_ANALOG))
        return spellInfo->Id;

    SpellAnalogMap::const_iterator analog = mSpellAnalogMap.find(spellInfo->Id);
    if (analog != mSpellAnalogMap.end())
    {
        if (IsSpellStart)
        {
            if (caster && analog->second.visual_caster_start)
                caster->SendSpellVisual(analog->second.visual_caster_start);

            if (target && analog->second.visual_target_start)
                target->SendSpellVisual(analog->second.visual_target_start);
        }
        else
        {
            if (caster && analog->second.visual_caster_end)
                caster->SendSpellVisual(analog->second.visual_caster_end);

            if (target && analog->second.visual_target_end)
                target->SendSpellVisual(analog->second.visual_target_end);
        }

        if (analog->second.old_entry)
            return analog->second.old_entry;
    }
    return spellInfo->Id;
}

SpellAnalogViceVersaEntries const* SpellMgr::GetSpellAnalogViceVersa(uint32 spellId) // need to change this one. Many new can point to one old. Also make an attribute for it as well
{
    SpellAnalogViceVersaMap::const_iterator analogViceVersa = mSpellAnalogViceVersaMap.find(spellId);
    if (analogViceVersa != mSpellAnalogViceVersaMap.end())
        return &(analogViceVersa->second);
    return NULL;
}

bool SpellMgr::IsAuraCountdownContinueOffline(uint32 spellID, uint32 effIndex)
{
    if (IsPositiveEffect(spellID, effIndex))
        return false;

    //sWorld.getConfig(CONFIG_BATTLEGROUND_DESERTER_REALTIME)
    switch (spellID)
    {
    case SPELL_BG_DESERTER:
    case SPELL_ARENA_DESERTER:
          return false;
    }

    return true;
}

bool SpellMgr::CheckVictimAppropriate(SpellEntry const* spellInfo, Unit* target, bool checkInWorld)
{
    //check unit target but only for spells with direct targeting effect
    for (int i = 0; i < 3; ++i)
    {
        if (sSpellMgr.SpellTargetType[spellInfo->EffectImplicitTargetA[i]] == TARGET_TYPE_UNIT_TARGET)
        {
            if (!target || (checkInWorld && !target->IsInWorld()))
                return false;
            else
                break;
        }
    }

    //check destination
    if (spellInfo->Targets & (TARGET_FLAG_SOURCE_LOCATION | TARGET_FLAG_DEST_LOCATION))
    {
        if (!target)
            return false;
    }

    return true;
}