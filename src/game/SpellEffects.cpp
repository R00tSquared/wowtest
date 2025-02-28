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
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "SkillExtraItems.h"
#include "Unit.h"
#include "Spell.h"
#include "DynamicObject.h"
#include "SpellAuras.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Pet.h"
#include "GameObject.h"
#include "GossipDef.h"
#include "Creature.h"
#include "Totem.h"
#include "CreatureAI.h"
#include "BattleGroundMgr.h"
#include "BattleGround.h"
#include "BattleGroundEY.h"
#include "BattleGroundWS.h"
#include "OutdoorPvPMgr.h"
#include "VMapFactory.h"
#include "Language.h"
#include "SocialMgr.h"
#include "Util.h"
#include "TemporarySummon.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "PathFinder.h"
#include "InstanceData.h"

#include "Chat.h"
#include "Guild.h"
#include "GuildMgr.h"

#include <sstream>

#define ITEM_NEW_YEAR_2016_ID_HAT_2 693153

#define ITEM_NEW_YEAR_2016_ELF_SUMMON_ITEM 693156

pEffect SpellEffects[TOTAL_SPELL_EFFECTS]=
{
    &Spell::EffectNULL,                                     //  0
    &Spell::EffectInstaKill,                                //  1 SPELL_EFFECT_INSTAKILL
    &Spell::EffectSchoolDMG,                                //  2 SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::EffectDummy,                                    //  3 SPELL_EFFECT_DUMMY
    &Spell::EffectUnused,                                   //  4 SPELL_EFFECT_PORTAL_TELEPORT          unused
    &Spell::EffectTeleportUnits,                            //  5 SPELL_EFFECT_TELEPORT_UNITS
    &Spell::EffectApplyAura,                                //  6 SPELL_EFFECT_APPLY_AURA
    &Spell::EffectEnvirinmentalDMG,                         //  7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::EffectPowerDrain,                               //  8 SPELL_EFFECT_POWER_DRAIN
    &Spell::EffectHealthLeech,                              //  9 SPELL_EFFECT_HEALTH_LEECH
    &Spell::EffectHeal,                                     // 10 SPELL_EFFECT_HEAL
    &Spell::EffectUnused,                                   // 11 SPELL_EFFECT_BIND
    &Spell::EffectNULL,                                     // 12 SPELL_EFFECT_PORTAL
    &Spell::EffectUnused,                                   // 13 SPELL_EFFECT_RITUAL_BASE              unused
    &Spell::EffectUnused,                                   // 14 SPELL_EFFECT_RITUAL_SPECIALIZE        unused
    &Spell::EffectUnused,                                   // 15 SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL   unused
    &Spell::EffectQuestComplete,                            // 16 SPELL_EFFECT_QUEST_COMPLETE
    &Spell::EffectWeaponDmg,                                // 17 SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::EffectResurrect,                                // 18 SPELL_EFFECT_RESURRECT
    &Spell::EffectAddExtraAttacks,                          // 19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::EffectUnused,                                   // 20 SPELL_EFFECT_DODGE                    one spell: Dodge
    &Spell::EffectUnused,                                   // 21 SPELL_EFFECT_EVADE                    one spell: Evade (DND)
    &Spell::EffectParry,                                    // 22 SPELL_EFFECT_PARRY
    &Spell::EffectBlock,                                    // 23 SPELL_EFFECT_BLOCK                    one spell: Block
    &Spell::EffectCreateItem,                               // 24 SPELL_EFFECT_CREATE_ITEM
    &Spell::EffectUnused,                                   // 25 SPELL_EFFECT_WEAPON
    &Spell::EffectUnused,                                   // 26 SPELL_EFFECT_DEFENSE                  one spell: Defense
    &Spell::EffectPersistentAA,                             // 27 SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::EffectSummonType,                               // 28 SPELL_EFFECT_SUMMON
    &Spell::EffectLeapForward,                              // 29 SPELL_EFFECT_LEAP
    &Spell::EffectEnergize,                                 // 30 SPELL_EFFECT_ENERGIZE
    &Spell::EffectWeaponDmg,                                // 31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::EffectTriggerMissileSpell,                      // 32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::EffectOpenLock,                                 // 33 SPELL_EFFECT_OPEN_LOCK
    &Spell::EffectSummonChangeItem,                         // 34 SPELL_EFFECT_SUMMON_CHANGE_ITEM
    &Spell::EffectApplyAreaAura,                            // 35 SPELL_EFFECT_APPLY_AREA_AURA_PARTY
    &Spell::EffectLearnSpell,                               // 36 SPELL_EFFECT_LEARN_SPELL
    &Spell::EffectUnused,                                   // 37 SPELL_EFFECT_SPELL_DEFENSE            one spell: SPELLDEFENSE (DND)
    &Spell::EffectDispel,                                   // 38 SPELL_EFFECT_DISPEL
    &Spell::EffectUnused,                                   // 39 SPELL_EFFECT_LANGUAGE
    &Spell::EffectDualWield,                                // 40 SPELL_EFFECT_DUAL_WIELD
    &Spell::EffectSummonWild,                               // 41 SPELL_EFFECT_SUMMON_WILD
    &Spell::EffectSummonGuardian,                           // 42 SPELL_EFFECT_SUMMON_GUARDIAN
    &Spell::EffectTeleUnitsFaceCaster,                      // 43 SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::EffectLearnSkill,                               // 44 SPELL_EFFECT_SKILL_STEP
    &Spell::EffectAddHonor,                                 // 45 SPELL_EFFECT_ADD_HONOR                honor/pvp related
    &Spell::EffectNULL,                                     // 46 SPELL_EFFECT_SPAWN                    we must spawn pet there
    &Spell::EffectTradeSkill,                               // 47 SPELL_EFFECT_TRADE_SKILL
    &Spell::EffectUnused,                                   // 48 SPELL_EFFECT_STEALTH                  one spell: Base Stealth
    &Spell::EffectUnused,                                   // 49 SPELL_EFFECT_DETECT                   one spell: Detect
    &Spell::EffectTransmitted,                              // 50 SPELL_EFFECT_TRANS_DOOR
    &Spell::EffectUnused,                                   // 51 SPELL_EFFECT_FORCE_CRITICAL_HIT       unused
    &Spell::EffectUnused,                                   // 52 SPELL_EFFECT_GUARANTEE_HIT            one spell: zzOLDCritical Shot
    &Spell::EffectEnchantItemPerm,                          // 53 SPELL_EFFECT_ENCHANT_ITEM
    &Spell::EffectEnchantItemTmp,                           // 54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::EffectTameCreature,                             // 55 SPELL_EFFECT_TAMECREATURE
    &Spell::EffectSummonPet,                                // 56 SPELL_EFFECT_SUMMON_PET
    &Spell::EffectLearnPetSpell,                            // 57 SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::EffectWeaponDmg,                                // 58 SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::EffectOpenSecretSafe,                           // 59 SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::EffectProficiency,                              // 60 SPELL_EFFECT_PROFICIENCY
    &Spell::EffectSendEvent,                                // 61 SPELL_EFFECT_SEND_EVENT
    &Spell::EffectPowerBurn,                                // 62 SPELL_EFFECT_POWER_BURN
    &Spell::EffectThreat,                                   // 63 SPELL_EFFECT_THREAT
    &Spell::EffectTriggerSpell,                             // 64 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::EffectUnused,                                   // 65 SPELL_EFFECT_HEALTH_FUNNEL            unused
    &Spell::EffectUnused,                                   // 66 SPELL_EFFECT_POWER_FUNNEL             unused
    &Spell::EffectHealMaxHealth,                            // 67 SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::EffectInterruptCast,                            // 68 SPELL_EFFECT_INTERRUPT_CAST
    &Spell::EffectDistract,                                 // 69 SPELL_EFFECT_DISTRACT
    &Spell::EffectPull,                                     // 70 SPELL_EFFECT_PULL                     one spell: Distract Move
    &Spell::EffectPickPocket,                               // 71 SPELL_EFFECT_PICKPOCKET
    &Spell::EffectAddFarsight,                              // 72 SPELL_EFFECT_ADD_FARSIGHT
    &Spell::EffectSummonPossessed,                          // 73 SPELL_EFFECT_SUMMON_POSSESSED
    &Spell::EffectSummonTotem,                              // 74 SPELL_EFFECT_SUMMON_TOTEM
    &Spell::EffectHealMechanical,                           // 75 SPELL_EFFECT_HEAL_MECHANICAL          one spell: Mechanical Patch Kit
    &Spell::EffectSummonObjectWild,                         // 76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::EffectScriptEffect,                             // 77 SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::EffectUnused,                                   // 78 SPELL_EFFECT_ATTACK
    &Spell::EffectSanctuary,                                // 79 SPELL_EFFECT_SANCTUARY
    &Spell::EffectAddComboPoints,                           // 80 SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::EffectUnused,                                   // 81 SPELL_EFFECT_CREATE_HOUSE             one spell: Create House (TEST)
    &Spell::EffectNULL,                                     // 82 SPELL_EFFECT_BIND_SIGHT
    &Spell::EffectDuel,                                     // 83 SPELL_EFFECT_DUEL
    &Spell::EffectStuck,                                    // 84 SPELL_EFFECT_STUCK
    &Spell::EffectSummonPlayer,                             // 85 SPELL_EFFECT_SUMMON_PLAYER
    &Spell::EffectActivateObject,                           // 86 SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::EffectSummonTotem,                              // 87 SPELL_EFFECT_SUMMON_TOTEM_SLOT1
    &Spell::EffectSummonTotem,                              // 88 SPELL_EFFECT_SUMMON_TOTEM_SLOT2
    &Spell::EffectSummonTotem,                              // 89 SPELL_EFFECT_SUMMON_TOTEM_SLOT3
    &Spell::EffectSummonTotem,                              // 90 SPELL_EFFECT_SUMMON_TOTEM_SLOT4
    &Spell::EffectUnused,                                   // 91 SPELL_EFFECT_THREAT_ALL               one spell: zzOLDBrainwash
    &Spell::EffectEnchantHeldItem,                          // 92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::EffectUnused,                                   // 93 SPELL_EFFECT_SUMMON_PHANTASM
    &Spell::EffectSelfResurrect,                            // 94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::EffectSkinning,                                 // 95 SPELL_EFFECT_SKINNING
    &Spell::EffectUnused,                                   // 96 SPELL_EFFECT_CHARGE
    &Spell::EffectSummonCritter,                            // 97 SPELL_EFFECT_SUMMON_CRITTER
    &Spell::EffectKnockBack,                                // 98 SPELL_EFFECT_KNOCK_BACK
    &Spell::EffectDisEnchant,                               // 99 SPELL_EFFECT_DISENCHANT
    &Spell::EffectInebriate,                                //100 SPELL_EFFECT_INEBRIATE
    &Spell::EffectFeedPet,                                  //101 SPELL_EFFECT_FEED_PET
    &Spell::EffectDismissPet,                               //102 SPELL_EFFECT_DISMISS_PET
    &Spell::EffectReputation,                               //103 SPELL_EFFECT_REPUTATION
    &Spell::EffectSummonObject,                             //104 SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    &Spell::EffectSummonObject,                             //105 SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    &Spell::EffectSummonObject,                             //106 SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    &Spell::EffectSummonObject,                             //107 SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    &Spell::EffectDispelMechanic,                           //108 SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::EffectResurrectPet,                             //109 SPELL_EFFECT_RESURRECT_PET
    &Spell::EffectDestroyAllTotems,                         //110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::EffectDurabilityDamage,                         //111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::EffectSummonDemon,                              //112 SPELL_EFFECT_SUMMON_DEMON
    &Spell::EffectResurrectNew,                             //113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::EffectTaunt,                                    //114 SPELL_EFFECT_ATTACK_ME
    &Spell::EffectDurabilityDamagePCT,                      //115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::EffectSkinPlayerCorpse,                         //116 SPELL_EFFECT_SKIN_PLAYER_CORPSE       one spell: Remove Insignia, bg usage, required special corpse flags...
    &Spell::EffectSpiritHeal,                               //117 SPELL_EFFECT_SPIRIT_HEAL              one spell: Spirit Heal
    &Spell::EffectSkill,                                    //118 SPELL_EFFECT_SKILL                    professions and more
    &Spell::EffectApplyAreaAura,                            //119 SPELL_EFFECT_APPLY_AREA_AURA_PET
    &Spell::EffectUnused,                                   //120 SPELL_EFFECT_TELEPORT_GRAVEYARD       one spell: Graveyard Teleport Test
    &Spell::EffectWeaponDmg,                                //121 SPELL_EFFECT_NORMALIZED_WEAPON_DMG
    &Spell::EffectUnused,                                   //122 SPELL_EFFECT_122                      unused
    &Spell::EffectSendTaxi,                                 //123 SPELL_EFFECT_SEND_TAXI                taxi/flight related (misc value is taxi path id)
    &Spell::EffectPlayerPull,                               //124 SPELL_EFFECT_PLAYER_PULL              opposite of knockback effect (pulls player twoard caster)
    &Spell::EffectModifyThreatPercent,                      //125 SPELL_EFFECT_MODIFY_THREAT_PERCENT
    &Spell::EffectStealBeneficialBuff,                      //126 SPELL_EFFECT_STEAL_BENEFICIAL_BUFF    spell steal effect?
    &Spell::EffectProspecting,                              //127 SPELL_EFFECT_PROSPECTING              Prospecting spell
    &Spell::EffectApplyAreaAura,                            //128 SPELL_EFFECT_APPLY_AREA_AURA_FRIEND
    &Spell::EffectApplyAreaAura,                            //129 SPELL_EFFECT_APPLY_AREA_AURA_ENEMY
    &Spell::EffectRedirectThreat,                           //130 SPELL_EFFECT_REDIRECT_THREAT
    &Spell::EffectUnused,                                   //131 SPELL_EFFECT_131                      used in some test spells
    &Spell::EffectPlayMusic,                                //132 SPELL_EFFECT_PLAY_MUSIC               sound id in misc value
    &Spell::EffectUnlearnSpecialization,                    //133 SPELL_EFFECT_UNLEARN_SPECIALIZATION   unlearn profession specialization
    &Spell::EffectKillCredit,                               //134 SPELL_EFFECT_KILL_CREDIT              misc value is creature entry
    &Spell::EffectNULL,                                     //135 SPELL_EFFECT_CALL_PET
    &Spell::EffectHealPct,                                  //136 SPELL_EFFECT_HEAL_PCT
    &Spell::EffectEnergisePct,                              //137 SPELL_EFFECT_ENERGIZE_PCT
    &Spell::EffectLeapBack,                                 //138 SPELL_EFFECT_LEAP_BACK                Leap Back
    &Spell::EffectUnused,                                   //139 SPELL_EFFECT_139                      unused
    &Spell::EffectForceCast,                                //140 SPELL_EFFECT_FORCE_CAST
    &Spell::EffectNULL,                                     //141 SPELL_EFFECT_141                      Only one spell, using SPELL_EFFECT_SCHOOL_DAMAGE, and there triggering spell
    &Spell::EffectTriggerSpellWithValue,                    //142 SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE
    &Spell::EffectApplyAreaAura,                            //143 SPELL_EFFECT_APPLY_AREA_AURA_OWNER
    &Spell::EffectKnockBack,                                //144 SPELL_EFFECT_KNOCK_BACK_2             Spectral Blast
    &Spell::EffectSuspendGravity,                           //145 SPELL_EFFECT_SUSPEND_GRAVITY          Black Hole Effect
    &Spell::EffectUnused,                                   //146 SPELL_EFFECT_146                      unused
    &Spell::EffectQuestFail,                                //147 SPELL_EFFECT_QUEST_FAIL               quest fail
    &Spell::EffectUnused,                                   //148 SPELL_EFFECT_148                      unused
    &Spell::EffectCharge2,                                  //149 SPELL_EFFECT_CHARGE2                  swoop
    &Spell::EffectUnused,                                   //150 SPELL_EFFECT_150                      unused
    &Spell::EffectTriggerRitualOfSummoning,                 //151 SPELL_EFFECT_TRIGGER_SPELL_2
    &Spell::EffectFriendSummon,                             //152 SPELL_EFFECT_FRIEND_SUMMON                     summon Refer-a-Friend
    &Spell::EffectNULL,                                     //153 SPELL_EFFECT_CREATE_PET               misc value is creature entry
};

void Spell::EffectNULL(uint32 /*i*/)
{
    sLog.outDebug("WORLD: Spell Effect DUMMY");
}

void Spell::EffectUnused(uint32 /*i*/)
{
    // NOT USED BY ANY SPELL OR USELESS OR IMPLEMENTED IN DIFFERENT WAY IN MANGOS
}

void Spell::EffectResurrectNew(uint32 i)
{
    if (!unitTarget || !unitTarget->IsInWorld() || unitTarget->isAlive())
        return;

    if (Player* pTarget = unitTarget->ToPlayer())
    {
        if (pTarget->isRessurectRequested())       // already have one active request
            return;

        uint32 health = damage;
        uint32 mana = GetSpellEntry()->EffectMiscValue[i];
        pTarget->setResurrectRequestData(m_caster->GetGUID(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
        SendResurrectRequest(pTarget);
    }
    else if (Pet* pet = unitTarget->ToPet())
    {
        if (pet->getPetType() != HUNTER_PET || pet->isAlive())
            return;

        float x, y, z;
        m_caster->GetPosition(x, y, z);
        pet->NearTeleportTo(x, y, z, m_caster->GetOrientation());

        pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
        pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        pet->setDeathState(ALIVE);
        pet->ClearUnitState(UNIT_STAT_ALL_STATE);
        pet->SetHealth(uint32(damage));

        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    }
}

void Spell::EffectInstaKill(uint32 /*i*/)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    // Demonic Sacrifice
    if (GetSpellEntry()->Id==18788 && unitTarget->GetTypeId()==TYPEID_UNIT)
    {
        uint32 entry = unitTarget->GetEntry();
        uint32 spellID;
        switch (entry)
        {
            case   416: spellID=18789; break;               //imp
            case   417: spellID=18792; break;               //fellhunter
            case  1860: spellID=18790; break;               //void
            case  1863: spellID=18791; break;               //succubus
            case 17252: spellID=35701; break;               //fellguard
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: EffectInstaKill: Unhandled creature entry (%u) case.",entry);
                return;
        }

        m_caster->CastSpell(m_caster,spellID,true);
    }

    if (m_caster==unitTarget)                                // prevent interrupt message
        finish();

    uint32 health = unitTarget->GetHealth();
    m_caster->DealDamage(unitTarget, health, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
}

void Spell::EffectEnvirinmentalDMG(uint32 i)
{
    if (!unitTarget)
        return;
    uint32 absorb = 0;
    uint32 resist = 0;

    // Note: this hack with damage replace required until GO casting not implemented
    // environment damage spells already have around enemies targeting but this not help in case not existed GO casting support
    // currently each enemy selected explicitly and self cast damage, we prevent apply self cast spell bonuses/etc
    damage = GetSpellEntry()->CalculateSimpleValue(i);

    m_caster->CalcAbsorb(m_caster, SpellMgr::GetSpellSchoolMask(GetSpellEntry()), damage, &absorb, &resist);

    //m_caster->SendSpellNonMeleeDamageLog(m_caster, GetSpellEntry()->Id, damage, SpellMgr::GetSpellSchoolMask(GetSpellEntry()), absorb, resist, false, 0, false);
    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
        unitTarget->ToPlayer()->EnvironmentalDamage(DAMAGE_FIRE, damage);
}

void Spell::EffectSchoolDMG(uint32 effect_idx)
{
}

void Spell::SpellDamageSchoolDmg(uint32 effect_idx)
{
    // what the fuck is done here? o.O
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(GetSpellEntry()->Id);
    if (!spellInfo)
        return;

    if (unitTarget && unitTarget->isAlive())
    {
        float totalDmgModPct = 1.0f;
        float attackPowerCoefficient = 0.0f;
        float rangedAttackPowerCoefficient = 0.0f;
        switch (spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_GENERIC:
            {
                //Gore
                if (spellInfo->SpellIconID == 2269)
                {
                     damage += (rand32()%2 ? damage : 0);
                }

                if (spellInfo->Id == 37841)
                {
                    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (unitTarget->HasAura(37830, 0))
                        {
                            damage += 500;
                            ((Player*)unitTarget)->KilledMonster(21910, 0);
                        }
                    }
                }

                // Self-damage
                if (spellInfo->Id == 44998)
                {
                    if (100*unitTarget->GetHealth()/unitTarget->GetMaxHealth() <= 50)
                    {
                        damage = 0;
                        unitTarget->RemoveAurasDueToSpell(44986);   //triggering self damage
                        unitTarget->CastSpell(unitTarget, 44994, true);   // cast self-repair
                    }
                    else
                        damage = 350;
                }

                // Meteor like spells (divided damage to targets)
                if (spellInfo->AttributesCu & SPELL_ATTR_CU_SHARE_DAMAGE)
                {
                    uint32 count = 0;
                    for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
                    {
                        if (ihit->deleted)
                            continue;

                        if (ihit->effectMask & (1<<effect_idx))
                            ++count;
                    }

                    damage /= count;                    // divide to all targets
                }

                switch (spellInfo->Id)                     // better way to check unknown
                {
                    // Priestess of Torment - whirlwind dmg should also mana burn
                    case 46271:
                    {
                        if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                            m_caster->CastSpell(unitTarget, 46266, true);
                        break;
                    }
                    case 35354: //Hand of Death
                    {
                        if (unitTarget && unitTarget->HasAura(38528,0)) //Protection of Elune
                        {
                            damage = 0;
                        }
                        break;
                    }
                    // percent from health with min
                    case 25599:                             // Thundercrash
                    {
                                                                                                            // gotta use GetCreatureDamageMod(), cause later this damage is multiplied by it
                        damage = unitTarget->GetHealth() / 2 / (m_caster->GetObjectGuid().IsCreature() ? ((Creature*)m_caster)->GetCreatureDamageMod() : 1.0f);
                        if (damage < 200)
                            damage = 200;
                        break;
                    }
                    // arcane charge. must only affect demons (also undead?)
                    case 45072:
                    {
                        if (unitTarget->GetCreatureType() != CREATURE_TYPE_DEMON
                            && unitTarget->GetCreatureType() != CREATURE_TYPE_UNDEAD)
                            return;
                        break;
                    }
                    // gruul's shatter
                    case 33671:
                    {
                        // don't damage self and only players
                        if (unitTarget->GetGUID() == m_caster->GetGUID() || unitTarget->GetTypeId() != TYPEID_PLAYER)
                            return;

                        float radius = SpellMgr::GetSpellRadius(spellInfo,0,false);
                        if (!radius) 
                            return;
                        float distance = m_caster->GetDistance2d(unitTarget);
                        damage = (distance > radius) ? 0 : (int32)(spellInfo->EffectBasePoints[0]*((radius - distance)/radius));
                        if (m_caster->GetObjectGuid().IsCreature())
                            damage *= ((Creature*)m_caster)->GetCreatureDamageMod();
                         // Set the damage directly without spell bonus damage
                        m_damage += damage;
                        damage = 0;
                        break;
                    }
                    case 19593: // Buru Egg Explosion
                    {
                        damage = 500;
                        if (unitTarget->GetTypeId() == TYPEID_UNIT && ((Creature*)unitTarget)->GetEntry() == 15370/*Buru the Gorger*/)
                            damage *= 120; // 60000 damage - it is reduced by armor for about 1/4 + 45k as said is not the most damage it can do

                        float radius = SpellMgr::GetSpellRadius(spellInfo,0,false);
                        if (!radius) 
                            return;

                        float distance = m_caster->GetDistance2d(unitTarget);
                        if (distance > radius)
                            damage = 0;
                        else if ((radius - distance)/radius > 0.2f)
                            damage = damage*((radius - distance)/radius);
                        else
                            damage = damage*0.2f; // 20% damage is minimum

                        if (m_caster->GetObjectGuid().IsCreature())
                            damage *= ((Creature*)m_caster)->GetCreatureDamageMod();

                        m_damage += damage;
                        damage = 0;
                        break;
                    }
                    // Thaddius' charges, don't deal dmg to units with the same charge but give them the buff:
                    // Positive Charge
                    case 28062:
                    case 39090:
                    {
                        // remove pet from damage and buff list
                        if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                        {
                              damage = 0;
                              break;
                        }
                        // If target is not (+) charged, then just deal dmg
                        if (!unitTarget->HasAura(28059, 0) && !unitTarget->HasAura(39088, 0))
                            break;

                        if (m_caster != unitTarget)
                            m_caster->CastSpell(m_caster, spellInfo->Id == 39090 ? 39089 : 29659, true);

                        damage = 0;
                        break;
                    }
                    // Negative Charge
                    case 28085:
                    case 39093:
                    {
                        // remove pet from damage and buff list
                        if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                        {
                              damage = 0;
                              break;
                        }
                        // If target is not (-) charged, then just deal dmg
                        if (!unitTarget->HasAura(28084, 0) && !unitTarget->HasAura(39091, 0))
                            break;

                        if (m_caster != unitTarget)
                            m_caster->CastSpell(m_caster, spellInfo->Id == 39093 ? 39092 : 29660, true);

                        damage = 0;
                        break;
                    }
                    // Cataclysmic Bolt
                    case 38441:
                        damage = unitTarget->GetMaxHealth() / 2;
                        break;
                    //sonic boom
                    case 33666:
                    case 38795:
                        damage = unitTarget->GetHealth()*90/100;
                        break;
                }
                break;
            }

            case SPELLFAMILY_MAGE:
            {
                // Arcane Blast
                if (spellInfo->SpellFamilyFlags & 0x20000000LL)
                {
                    m_caster->CastSpell(m_caster,36032,true);
                }
                break;
            }
            case SPELLFAMILY_WARRIOR:
            {
                // Bloodthirst
                if (spellInfo->SpellFamilyFlags & 0x40000000000LL)
                {
                    attackPowerCoefficient += float(damage) *0.01f; // Base damage shows us percentage of AP that need be added
                    damage = 0; // clear this, we have now how much AP should be taken
                }
                // Shield Slam
                else if (spellInfo->SpellFamilyFlags & 0x100000000LL)
                    damage += int32(m_caster->GetShieldBlockValue());
                // Victory Rush
                else if (spellInfo->SpellFamilyFlags & 0x10000000000LL)
                {
                    attackPowerCoefficient += float(damage) *0.01f; // Base damage shows us percentage of AP that need be added
                    damage = 0; // clear this, we have now how much AP should be taken
                    m_caster->ModifyAuraState(AURA_STATE_WARRIOR_VICTORY_RUSH, false);
                }
                break;
            }
            case SPELLFAMILY_WARLOCK:
            {
                // Incinerate Rank 1 & 2
                if ((spellInfo->SpellFamilyFlags & 0x00004000000000LL) && spellInfo->SpellIconID==2128)
                {
                    // Incinerate does more dmg (dmg*0.25) if the target is Immolated.
                    if (unitTarget->HasAuraState(AURA_STATE_IMMOLATE))
                        damage += int32(damage*0.25);
                    // T5 bonus - increase immolate damage on incinerate hit
                    if (m_caster->HasAura(37384, 0))
                    {
                        // look for immolate cast by m_caster
                        Unit::AuraList const &mPeriodic = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                        for (Unit::AuraList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
                        {
                            if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && ((*i)->GetSpellProto()->SpellFamilyFlags & 4) &&
                                (*i)->GetCasterGUID()==m_caster->GetGUID())
                            {
                                // store number of incinerate hits in m_miscvalue
                                (*i)->GetModifier()->m_miscvalue++;
                                break;
                            }
                        }
                    }
                }
                // Incinerate, Sunwell Warlock in MgT
                if (spellInfo->Id == 44519 || spellInfo->Id == 46043)
                {
                    // Incinerate does more dmg (dmg*0.25) if the target is Immolated.
                    if (unitTarget->HasAura(44518, 0) || unitTarget->HasAura(46042, 0))
                        damage += int32(damage*0.25);
                }
                // Shadow bolt
                if (spellInfo->SpellFamilyFlags & 1)
                {
                    // T5 bonus - increase corruption on shadow bolt hit
                    if (m_caster->HasAura(37384, 0))
                    {
                        // look for corruption cast by m_caster
                        Unit::AuraList const &mPeriodic = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                        for (Unit::AuraList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
                        {
                            if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && ((*i)->GetSpellProto()->SpellFamilyFlags & 2) &&
                                (*i)->GetCasterGUID()==m_caster->GetGUID())
                            {
                                // store number of shadow bolt hits in m_miscvalue
                                (*i)->GetModifier()->m_miscvalue++;
                                break;
                            }
                        }
                    }
                }
                // Conflagrate - consumes immolate
                if (spellInfo->TargetAuraState == AURA_STATE_IMMOLATE)
                {
                    // for caster applied auras only
                    Unit::AuraList const &mPeriodic = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraList::const_iterator i = mPeriodic.begin(); i != mPeriodic.end(); ++i)
                    {
                        if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_WARLOCK && ((*i)->GetSpellProto()->SpellFamilyFlags & 4) &&
                            (*i)->GetCasterGUID()==m_caster->GetGUID())
                        {
                            unitTarget->RemoveAurasByCasterSpell((*i)->GetId(), m_caster->GetGUID());
                            break;
                        }
                    }
                }
                break;
            }
            case SPELLFAMILY_DRUID:
            {
                // Ferocious Bite
                if ((spellInfo->SpellFamilyFlags & 0x000800000) && spellInfo->SpellVisual == 6587)
                {
                    // converts each extra point of energy into ($f1+$AP/630) additional damage
                    int extraEnergy = (m_caster->GetPower(POWER_ENERGY) - GetPowerCost());
                    damage += int32(extraEnergy * spellInfo->DmgMultiplier[effect_idx]);
                    m_caster->SetPower(POWER_ENERGY, GetPowerCost());
                    attackPowerCoefficient += extraEnergy / 630.0f + 0.1526f;
                }
                // Rake
                else if (spellInfo->SpellFamilyFlags & 0x0000000000001000LL && spellInfo->SpellIconID == 494)
                {
                    attackPowerCoefficient += 0.01f;
                }
                // Swipe
                else if (spellInfo->SpellFamilyFlags & 0x0010000000000000LL)
                {
                    attackPowerCoefficient += 0.08f;
                }
                // Starfire
                else if (spellInfo->SpellFamilyFlags & 0x0004LL)
                {
                    Unit::AuraList const& m_OverrideClassScript = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (Unit::AuraList::const_iterator i = m_OverrideClassScript.begin(); i != m_OverrideClassScript.end(); ++i)
                    {
                        // Starfire Bonus (caster)
                        switch ((*i)->GetModifier()->m_miscvalue)
                        {
                            case 5481:                      // Nordrassil Regalia - bonus
                            {
                                Unit::AuraList const& m_periodicDamageAuras = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                                for (Unit::AuraList::const_iterator itr = m_periodicDamageAuras.begin(); itr != m_periodicDamageAuras.end(); ++itr)
                                {
                                    // Moonfire or Insect Swarm (target debuff from any casters)
                                    if ((*itr)->GetSpellProto()->SpellFamilyFlags & 0x00200002LL)
                                    {
                                        totalDmgModPct *= 1 + ((*i)->GetModifierValue() / 100.0f);
                                        break;
                                    }
                                }
                                break;
                            }
                            case 5148:                      //Improved Starfire - Ivory Idol of the Moongoddes Aura
                            {
                                damage += (*i)->GetModifier()->m_amount;
                                break;
                            }
                        }
                    }
                }
                //Mangle Bonus for the initial damage of Lacerate and Rake
                if ((spellInfo->SpellFamilyFlags==0x0000000000001000LL && spellInfo->SpellIconID==494) ||
                    (spellInfo->SpellFamilyFlags==0x0000010000000000LL && spellInfo->SpellIconID==2246))
                {
                    Unit::AuraList const& mDummyAuras = unitTarget->GetAurasByType(SPELL_AURA_DUMMY);
                    for (Unit::AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
                        if ((*i)->GetSpellProto()->SpellFamilyFlags & 0x0000044000000000LL && (*i)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_DRUID)
                        {
                            totalDmgModPct *= 1 + (*i)->GetModifierValue() / 100.0f;
                            break;
                        }
                }
                // L5 Arcane Charge (its weird that it is claimed to be DRUID spell :D)
                if (spellInfo->Id == 41360)
                    damage = unitTarget->GetMaxHealth();
                break;
            }
            case SPELLFAMILY_ROGUE:
            {
                // Envenom
                if (m_caster->GetTypeId()==TYPEID_PLAYER && (spellInfo->SpellFamilyFlags & 0x800000000LL))
                {
                    // consume from stack dozes not more that have combo-points
                    if (uint32 combo = ((Player*)m_caster)->GetComboPoints())
                    {
                        // count consumed deadly poison doses at target
                        uint32 doses = 0;

                        // remove consumed poison doses
                        Unit::AuraList const& auras = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                        for (Unit::AuraList::const_iterator itr = auras.begin(); itr!=auras.end() && combo;)
                        {
                            // Deadly poison (only attacker applied)
                            if ((*itr)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_ROGUE && ((*itr)->GetSpellProto()->SpellFamilyFlags & 0x10000) &&
                                (*itr)->GetSpellProto()->SpellVisual==5100 && (*itr)->GetCasterGUID()==m_caster->GetGUID())
                            {
                                --combo;
                                ++doses;

                                unitTarget->RemoveSingleAuraFromStackByCaster((*itr)->GetId(), (*itr)->GetEffIndex(), m_caster->GetGUID());

                                itr = auras.begin();
                            }
                            else
                                ++itr;
                        }

                        damage *= doses;
                        attackPowerCoefficient += 0.03f * doses;

                        // Eviscerate and Envenom Bonus Damage (item set effect)
                        if (m_caster->GetDummyAura(37169))
                            damage += ((Player*)m_caster)->GetComboPoints()*40;
                    }
                }
                // Eviscerate
                else if ((spellInfo->SpellFamilyFlags & 0x00020000LL) && m_caster->GetTypeId()==TYPEID_PLAYER)
                {
                    if (uint32 combo = ((Player*)m_caster)->GetComboPoints())
                    {
                        attackPowerCoefficient += combo * 0.03f;

                        // Eviscerate and Envenom Bonus Damage (item set effect)
                        if (m_caster->GetDummyAura(37169))
                            damage += combo*40;
                    }
                }
                break;
            }
            case SPELLFAMILY_HUNTER:
            {
                // Mongoose Bite
                if ((spellInfo->SpellFamilyFlags & 0x000000002) && spellInfo->SpellVisual==342)
                {
                    attackPowerCoefficient += 0.2f;
                }
                // Arcane Shot
                else if ((spellInfo->SpellFamilyFlags & 0x00000800) && spellInfo->maxLevel > 0)
                {
                    rangedAttackPowerCoefficient += 0.15f;
                }
                // Steady Shot
                else if (spellInfo->SpellFamilyFlags & 0x100000000LL)
                {
                    int32 base = irand((int32)m_caster->GetWeaponDamageRange(RANGED_ATTACK, MINDAMAGE),(int32)m_caster->GetWeaponDamageRange(RANGED_ATTACK, MAXDAMAGE));
                    damage += int32(float(base)/m_caster->GetAttackTime(RANGED_ATTACK)*2800);
                    rangedAttackPowerCoefficient += 0.2f;

                    bool found = false;

                    // check dazed affect
                    Unit::AuraList const& decSpeedList = unitTarget->GetAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                    for (Unit::AuraList::const_iterator iter = decSpeedList.begin(); iter != decSpeedList.end(); ++iter)
                    {
                        if ((*iter)->GetSpellProto()->SpellIconID == 15)
                        {
                            found = true;
                            break;
                        }
                    }

                    //TODO: should this be put on taken but not done?
                    if (found)
                    {
                        damage += spellInfo->EffectBasePoints[1];
                    }
                }
                //Explosive Trap Effect
                else if (spellInfo->SpellFamilyFlags & 0x00000004)
                {
                    rangedAttackPowerCoefficient += 0.1f;
                }
                break;
            }
            case SPELLFAMILY_PALADIN:
            {
                //Judgement of Vengeance
                if ((spellInfo->SpellFamilyFlags & 0x800000000LL) && spellInfo->SpellIconID==2292 && spellInfo->Id != 42463)
                {
                    uint32 stacks = 0;
                    Unit::AuraList const& auras = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (Unit::AuraList::const_iterator itr = auras.begin(); itr!=auras.end(); ++itr)
                        if ((*itr)->GetId() == 31803 && (*itr)->GetCasterGUID()==m_caster->GetGUID())
                        {
                            stacks = (*itr)->GetStackAmount();
                            break;
                        }

                    if (!stacks)
                        //No damage if the target isn't affected by this
                        damage = -1;
                    else
                        damage *= stacks;
                }
                else if (spellInfo->SpellFamilyFlags & 0x4000000000000LL && m_originalCaster && m_originalCaster->GetDummyAura(54895)) // Avenger's Shield FUN
                {
                    int32 manaReg = m_originalCaster->GetMaxPower(POWER_MANA) * 0.2f;
                    m_originalCaster->CastCustomSpell(m_originalCaster, 8358, &manaReg, 0, 0, true);
                }
                break;
            }
            case SPELLFAMILY_SHAMAN:
            {
                // Lightning Bolt & Chain Lightning
                if (spellInfo->SpellFamilyFlags & 0x0003LL)
                {
                    bool stop = false;
                    Unit::AuraList const& auras = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                    {
                        switch ((*itr)->GetId())
                        {
                            case 28857:
                            case 34230:
                            case 41040:
                                damage += (*itr)->GetModifierValue();
                                stop = true;
                                break;
                        }

                        if (stop)
                            break;
                    }
                }
                break;
            }
        }

        if (attackPowerCoefficient)
        {
            float attackPower = m_caster->GetTotalAttackPowerValue(BASE_ATTACK) + unitTarget->GetMeleeApAttackerBonus();
            attackPower += m_caster->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS, unitTarget->GetCreatureTypeMask());

            damage += attackPowerCoefficient * attackPower;
        }

        if (rangedAttackPowerCoefficient)
        {
            float attackPower = m_caster->GetTotalAttackPowerValue(RANGED_ATTACK) + unitTarget->GetTotalAuraModifier(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
            attackPower += m_caster->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS, unitTarget->GetCreatureTypeMask());

            damage += rangedAttackPowerCoefficient * attackPower;
        }

        if (m_originalCaster && damage > 0)
            damage = m_originalCaster->SpellDamageBonus(unitTarget, spellInfo, (uint32)damage, SPELL_DIRECT_DAMAGE);

        damage *= totalDmgModPct;

        m_damage += damage;
    }
}

void Spell::EffectDummy(uint32 i)
{
    if (!unitTarget && !gameObjTarget && !itemTarget)
        return;

	Player* p = m_caster->ToPlayer();

    uint32 spell_id = 0;
    int32 bp = 0;

    // selection by spell family
    switch (GetSpellEntry()->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (GetSpellEntry()->Id)
            {
                case 11887:
                case 11885:
                case 11886:
                case 11888:
                case 11889:
                {
                    if(!unitTarget)
                        return;

                    if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->isDead())
                        ((Creature*)unitTarget)->RemoveCorpse();
                    return;
                }
                case 19395:
                {
                    if (!unitTarget)
                        return;

                    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
                        m_caster->SummonGameObject(urand(0, 1) ? 144064 : 177681, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 0, 0, 0, 0, 0, 60);

                    return;
                }
                case 17950:                                 // Shadow Portal
                {
                    if (!unitTarget)
                        return;
                    // Shadow Portal
                    const uint32 spell_list[6] = {17863, 17939, 17943, 17944, 17946, 17948};

                    m_caster->CastSpell(unitTarget, spell_list[urand(0, 5)], true);
                    return;
                }
                case 46292:                                 // Cataclysm Breath
                {
                    // Cataclysm Spells
                    const uint32 spell_list[8] =
                    {
                        46293,  // Corrosive Poison
                        46294,  // Fevered Fatigue
                        46295,  // Hex
                        46296,  // Necrotic Poison
                        46297,  // Piercing Shadow
                        46298,  // Shrink
                        46299,  // Wavering Will
                        46300   // Withered Touch
                    };

                    std::vector<uint32> debuff_list;
                    for (uint8 i = 0; i < 8; ++i)
                        debuff_list.push_back(spell_list[i]);
                    std::random_shuffle(debuff_list.begin(), debuff_list.end());
                    debuff_list.resize(urand(5, 6));
                    for (std::vector<uint32>::iterator itr = debuff_list.begin(); itr != debuff_list.end(); itr++)
                        m_caster->CastSpell((Unit*)NULL, *itr, true);
                    return;
                }
                case 46476:                                 // Sunblade Protector Activated
                {
                    if(!m_originalCaster)
                        return;

                    if(unitTarget->GetTypeId() == TYPEID_UNIT && m_originalCaster->GetVictim())
                    {
                        ((Creature*)unitTarget)->AI()->AttackStart(m_originalCaster->GetVictim());
                        m_originalCaster->GetMotionMaster()->MoveChase(m_originalCaster->GetVictim(), 0, 0);
                    }
                    return;
                }
                case 38782:
                {
                    if (i == 0)
                    {
                        float x,y,z;
                        m_caster->GetNearPoint(x,y,z, 0.0f, 10.0f, 0.0f);
                        if (Creature *pDruid = m_caster->SummonCreature(22423, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 40000))
                        {
                            pDruid->CastSpell(pDruid, 39158, true);
                            //pDruid->UpdateEntry(22425);
                        }
                    }
                    break;
                }
                case 39371:  
                {
					// need to port better scripts from OLD_HG
					if (unitTarget->GetEntry() == 22506 || unitTarget->GetEntry() == 22507 || unitTarget->GetEntry() == 22431)
					{
						if (unitTarget->GetEntry() != 22431)
							m_caster->CastSpell(m_caster, 39323, true);
						else
							m_caster->CastSpell(unitTarget, 39322, true);
					}

                    break;
                }
                case 32146:
                {
                    if (unitTarget->GetTypeId() == TYPEID_UNIT)
                        ((Creature*)unitTarget)->DisappearAndDie();

                    break;
                }
                case 41082:
                {
                    unitTarget->CastSpell(unitTarget, 41083, true, 0, 0, m_caster->GetGUID());
                    m_caster->CastSpell(unitTarget, 39123, true);

                    float x, y, z;
                    unitTarget->GetNearPoint(x,y,z, 0.0f, 0.0f, unitTarget->GetOrientationTo(m_caster));

                    m_caster->GetMotionMaster()->MovePoint(0, x, y, z);
                    break;
                }
                // Illidan Stormrage: Throw Glaive (Summon Glaive after throw;p
                case 39849:
                {
                    unitTarget->CastSpell(unitTarget, 41466, true);
                    if (unitTarget->GetTypeId() == TYPEID_UNIT)
                    {
                        ((Creature*)unitTarget)->Kill(unitTarget, false);
                        ((Creature*)unitTarget)->RemoveCorpse();
                    }
                    return;
                }
                // Illidan Stormrage: Return Glaive
                case 39873:
                {
                    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
                        return;

                    m_caster->CastSpell(unitTarget, 39635, true);
                }
                case 38002:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    m_caster->CastSpell((Unit*)NULL, 38003, false);
                }

                // Fatal Attraction
                case 40869:
                {
                    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    m_caster->CastSpell(unitTarget, 41001, true);
                }
                break;
                // Tag Subbued Talbuk (for Quest Creatures of the Eco-Domes - 10427)
                case 35771:
                {
                    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
                        return;

                    // 3 - after using target is forever-non-attackable
                    if (((Player*)m_caster)->GetQuestStatus(10427) == QUEST_STATUS_INCOMPLETE)
                    {
                        // Get Sleep Visual (34664)
                        SpellEntry const *sleepSpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(34664);
                        if (sleepSpellEntry) // Make the creature sleep in peace :)
                        {
                            m_caster->AttackStop();
                            unitTarget->RemoveAllAuras();
                            unitTarget->DeleteThreatList();
                            unitTarget->CombatStop();
                            Aura* sleepAura = CreateAura(sleepSpellEntry, 0, NULL, unitTarget,unitTarget, 0);

                            unitTarget->AddAura(sleepAura); // Apply Visual Sleep
                            unitTarget->addUnitState(UNIT_STAT_STUNNED);
                            // Cant use q item again on this target untill creature awakes
                            unitTarget->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                        }
                        // Add q objecive + 1
                        ((Player*)m_caster)->CastCreatureOrGO(20982, unitTarget->GetGUID(), 35771);
                    }
                return;
                }
                 // Skyguard Blasting Charge (for quest Fires Over Skettis - 11008)
                case 39844:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if (((Player*)m_caster)->GetQuestStatus(11008) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (unitTarget && unitTarget->GetEntry() == 22991) // trigger
                        {
                            // Handle associated GO - monstrous kaliri egg
                            GameObject* target = NULL;
                            Hellground::AllGameObjectsWithEntryInGrid go_check(185549);
                            Hellground::ObjectSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher(target, go_check);

                            // Find GO that matches this trigger:
                            Cell::VisitGridObjects(unitTarget, searcher, 3.0f);

                            // Add q objective and clean up
                            if (target)
                                m_caster->DealDamage(unitTarget, unitTarget->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                        }
                    }
                    return;
                }
                // Six Demon Bag, TODO: Search and add more spells to cast with normal dmg (100 ~ 200), Shadow bolt, Fireball, Summon Felhunter
                case 14537:
                {
                    int32 spellid = 0;
                    if (rand()%4)
                        switch (urand(1,3))
                        {
                            case 1: spellid = 45297; break;     // Chain Lightning
                            case 2: spellid = 23102; break;     // Frostbolt!
                            case 3: spellid = 9487;  break;     // Fireball !
                        }
                    else
                        spellid = (rand()%2) ? 29848 : 31718;     // Polymorph: Sheep : Enveloping Winds

                    uint8 backfire = rand()%5;
                    if (spellid == 29848 && !backfire)
                        m_caster->CastSpell(m_caster,spellid,true); // backfire with poly chance
                    else
                        m_caster->CastSpell(unitTarget,spellid,true);
                return;
                }
                 // Demon Broiled Surprise
                case 43723:
                {
                    m_caster->CastSpell(m_caster, 43753, false);
                    return;
                }
                // Wrath of the Astromancer
                case 42784:
                {
                    uint32 count = 0;
                    for (std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                    {
                        if (ihit->deleted)
                            continue;

                        if (ihit->effectMask & (1<<i))
                            ++count;
                    }

                    damage = 12000; // maybe wrong value
                    damage /= count;

                    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(42784);

                     // now deal the damage
                    for (std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                    {
                        if (ihit->deleted)
                            continue;

                        if (ihit->effectMask & (1<<i))
                        {
                            Unit* casttarget = Unit::GetUnit((*unitTarget), ihit->targetGUID);
                            if (casttarget)
                                m_caster->DealDamage(casttarget, damage, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_ARCANE, spellInfo, false);
                        }
                    }
                }
                // Encapsulate Voidwalker
                /*case 29364: // It's never used. The spell has other effects
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || ((Creature*)unitTarget)->isPet())
                        return;

                    Creature* creatureTarget = (Creature*)unitTarget;
                    GameObject* pGameObj = new GameObject;

                    if (!creatureTarget || !pGameObj)
                        return;

                    if (!pGameObj->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), 181574, creatureTarget->GetMap(),
                        creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(),
                        creatureTarget->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 100, GO_STATE_READY))
                    {
                        delete pGameObj;
                        return;
                    }

                    pGameObj->SetRespawnTime(0);
                    pGameObj->SetOwnerGUID(m_caster->GetGUID());
                    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->GetLevel());
                    pGameObj->SetSpellId(GetSpellEntry()->Id);

                    creatureTarget->GetMap()->Add(pGameObj);

                    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
                    data << uint64(pGameObj->GetGUID());
                    m_caster->BroadcastPacket(&data,true);

                    return;
                }*/
                case 8063:                                  // Deviate Fish
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;
                    switch (urand(1,5))
                    {
                        case 1: spell_id = 8064; break;     // Sleepy
                        case 2: spell_id = 8065; break;     // Invigorate
                        case 3: spell_id = 8066; break;     // Shrink
                        case 4: spell_id = 8067; break;     // Party Time!
                        case 5: spell_id = 8068; break;     // Healthy Spirit
                    }
                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 8213:                                  // Savory Deviate Delight
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;
                    switch (urand(1,2))
                    {
                        // Flip Out - ninja
                        case 1: spell_id = (m_caster->GetGender() == GENDER_MALE ? 8219 : 8220); break;
                        // Yaaarrrr - pirate
                        case 2: spell_id = (m_caster->GetGender() == GENDER_MALE ? 8221 : 8222); break;
                    }
                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                // case 8593:                                  // Symbol of life (restore creature to life)
                case 31225:                                 // Shimmering Vessel (restore creature to life)
                {
                    if (!unitTarget || unitTarget->GetTypeId()!=TYPEID_UNIT)
                        return;

                    ((Creature*)unitTarget)->setDeathState(JUST_ALIVED);
                    return;
                }
                case 12975:                                 //Last Stand
                {
                    int32 healthModSpellBasePoints0 = int32(m_caster->GetMaxHealth()*0.3);
                    m_caster->CastCustomSpell(m_caster, 12976, &healthModSpellBasePoints0, NULL, NULL, true, NULL);
                    return;
                }
                case 8344:                                  // Universal Remote
                {
                    if (!unitTarget)
                        return;

                    if (urand(0, 99) > 20)
                        m_caster->CastSpell(unitTarget, 8345, true);
                    else                                    // 20% (?guessed) chance for malfunction
                    {
                        switch (urand(0, 1))
                        {
                            case 0:
                                m_caster->CastSpell(unitTarget, 8346, true);
                                break;
                            case 1:
                                m_caster->CastSpell(unitTarget, 8347, true);
                                break;
                        }
                    }
                    return;
                }
                case 13280:                                 // Gnomish Death Ray direct damage
                {
                    int custdamage = m_originalCaster->GetMaxHealth() * 0.35f;
                    m_originalCaster->CastCustomSpell(roll_chance_i(5)? m_originalCaster: m_caster, 13279, &custdamage, NULL, NULL, true);
                    return;
                }
                case 13278:                                 // Gnomish Death Ray self DOT
                {
                    int custdamage = m_caster->GetMaxHealth() * frand(0.03f, 0.05f);
                    m_caster->CastCustomSpell(m_caster, 13493, &custdamage, NULL, NULL, true);
                    return;
                }
                case 13180:                                 // Gnomish Mind Control Cap
                {
                    if (!unitTarget)
                        return;

                    int32 failureChance = unitTarget->GetLevel() > 60 ? 20 : 5;            // guessed chance of failure
                    if (urand(0, 99) > failureChance)
                        m_caster->CastSpell(unitTarget, 13181, true);
                    else
                        unitTarget->CastSpell(m_caster, 13181, true);

                    return;
                }
                case 13006:                                 // gnomish shrink ray
                {
                    if (!unitTarget)
                        return;

                    if (urand(0, 99) > 10)
                        m_caster->CastSpell(unitTarget, 13003, true);
                    else                                    // 10% (?guessed) chance for malfunction
                    {
                        switch (urand(0, 3))
                        {
                            case 0:
                                m_caster->CastSpell(unitTarget, 13004, true);
                                break;
                            case 1:
                                m_caster->CastSpell(m_caster, 13003, true);
                                break;
                            case 2:
                                m_caster->CastSpell(m_caster, 13004, true);
                                break;
                            case 3:
                                m_caster->CastSpell(m_caster, 13010, true);
                                break;
                        }
                    }
                    return;
                }
                case 13120:                                 // net-o-matic
                {
                    if (!unitTarget)
                        return;

                    uint32 spell_id = 0;

                    uint32 roll = urand(0, 99);

                    if (roll < 2)                            // 2% for 30 sec self root (off-like chance unknown)
                        spell_id = 16566;
                    else if (roll < 10)                      // 8% for 20 sec root, charge to target (off-like chance unknown)
                        spell_id = 13119;
                    else
                        spell_id = 13099;

                    m_caster->CastSpell(unitTarget,spell_id,true,NULL);
                    return;
                }
                case 13567:                                 // Dummy Trigger
                {
                    // can be used for different aura triggering, so select by aura
                    if (!m_triggeredByAuraSpell || !unitTarget)
                        return;

                    switch (m_triggeredByAuraSpell->Id)
                    {
                        case 26467:                         // Persistent Shield
                            m_caster->CastCustomSpell(unitTarget, 26470, &damage, NULL, NULL, true);
                            break;
                        default:
                            sLog.outLog(LOG_DEFAULT, "ERROR: EffectDummy: Non-handled case for spell 13567 for triggered aura %u",m_triggeredByAuraSpell->Id);
                            break;
                    }
                    return;
                }
                case 14185:                                 // Preparation Rogue
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    //immediately finishes the cooldown on certain Rogue abilities
                    const SpellCooldowns& sp_list = ((Player *)m_caster)->GetSpellCooldowns();
                    time_t thetime = time(NULL);
                    SpellCooldowns::const_iterator itr, next;
                    for (itr = sp_list.begin(); itr != sp_list.end(); itr = next)
                    {
                        next = itr;
                        ++next;
                        if (itr->second < thetime) // cooldown gone already
                            continue;

                        uint32 classspell = itr->first;
                        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(classspell);

                        if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && (spellInfo->SpellFamilyFlags & 0x26000000860LL))
                        {
                            ((Player*)m_caster)->RemoveSpellCooldown(classspell, true);
                            ((Player*)m_caster)->GetCooldownMgr().CancelGlobalCooldown(spellInfo);
                        }
                    }
                    return;
                }
                case 21050:                                 // Melodious Rapture
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || unitTarget->GetEntry() != 13016)
                        return;

                    WorldLocation wLoc;
                    Creature* cTarget = (Creature*)unitTarget;
                    cTarget->GetPosition(wLoc);
                    float ang = cTarget->GetOrientationTo(wLoc.coord_x,wLoc.coord_y);

                    if (Creature * rat = m_caster->SummonCreature(13017,wLoc.coord_x,wLoc.coord_y,wLoc.coord_z,ang,TEMPSUMMON_TIMED_DESPAWN,600000))
                        rat->GetMotionMaster()->MoveFollow(m_caster, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

                    cTarget->setDeathState(JUST_DIED);
                    cTarget->RemoveCorpse();
                    cTarget->SetHealth(0);                  // just for nice GM-mode view
                    return;
                }
                case 15998:                                 // Capture Worg Pup
                case 29435:                                 // Capture Female Kaliri Hatchling
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    Creature* creatureTarget = (Creature*)unitTarget;
                    creatureTarget->DisappearAndDie();
                    return;
                }
                case 16589:                                 // Noggenfogger Elixir
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;
                    switch (urand(1,3))
                    {
                        case 1: spell_id = 16595; break;
                        case 2: spell_id = 16593; break;
                        default:spell_id = 16591; break;
                    }

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 17251:                                 // Spirit Healer Res
                {
                    if (!unitTarget || !m_originalCaster)
                        return;

                    if (m_originalCaster->GetTypeId() == TYPEID_PLAYER)
                    {
                        WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
                        data << unitTarget->GetGUID();
                        ((Player*)m_originalCaster)->SendPacketToSelf(&data);
                    }
                    return;
                }
                case 17271:                                 // Test Fetid Skull
                {
                    if (!itemTarget && m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 spell_id = roll_chance_i(50) ? 17269 : 17270;

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 19250:                                 // Placing Smokey's Explosives
                {
                    if(i == 0)
                    {
                        focusObject->SetLootState(GO_JUST_DEACTIVATED);
                        m_caster->CastSpell(m_caster, 19237, true);
                        ((Player*)m_caster)->KilledMonster(12247, m_caster->GetMap()->GetCreatureGUID(12247));
                    }
                    return;
                }
                case 19869:                                 // Dragon Orb
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER || unitTarget->HasAura(23958))
                        return;

                    unitTarget->CastSpell(unitTarget, 19832, true);
                    return;
                }
                case 20037:                                 // Explode Orb Effect
                {
                    if (!unitTarget)
                        return;

                    unitTarget->CastSpell(unitTarget, 20038, true);
                    return;
                }
                case 20577:                                 // Cannibalize
                    if (unitTarget)
                        m_caster->CastSpell(m_caster,20578,false,NULL);
                    return;
                case 21147:                                 // Arcane Vacuum
                {
                    if (!unitTarget)
                        return;

                    // Spell used by Azuregos to teleport all the players to him
                    // This also resets the target threat
                    if (m_caster->getThreatManager().getThreat(unitTarget))
                        m_caster->getThreatManager().modifyThreatPercent(unitTarget, -100);

                    // cast summon player
                    m_caster->CastSpell(unitTarget, 21150, true);
                    return;
                }
                case 23019:                                 // Crystal Prison Dummy DND
                {
                    if (!unitTarget || !unitTarget->isAlive() || unitTarget->GetTypeId() != TYPEID_UNIT || ((Creature*)unitTarget)->isPet())
                        return;

                    Creature* creatureTarget = (Creature*)unitTarget;
                    creatureTarget->setDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);                   // just for nice GM-mode view

                    GameObject* Crystal_Prison = m_caster->SummonGameObject(179644, creatureTarget->GetPositionX(), creatureTarget->GetPositionY(), creatureTarget->GetPositionZ(), creatureTarget->GetOrientation(), 0, 0, 0, 0, creatureTarget->GetRespawnTime()-time(NULL));
                    sLog.outDebug("SummonGameObject at SpellEfects.cpp EffectDummy for Spell 23019\n");
                    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
                    data << uint64(Crystal_Prison->GetGUID());
                    m_caster->BroadcastPacket(&data,true);

                    return;
                }
                case 23074:                                 // Arc. Dragonling
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,19804,true,m_CastItem);
                    return;
                case 23075:                                 // Mithril Mechanical Dragonling
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,12749,true,m_CastItem);
                    return;
                case 23076:                                 // Mechanical Dragonling
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,4073,true,m_CastItem);
                    return;
                case 23133:                                 // Gnomish Battle Chicken
                    if (!m_CastItem) return;
                    m_caster->CastSpell(m_caster,13166,true,m_CastItem);
                    return;
                case 23448:                                 // Ultrasafe Transporter: Gadgetzan - backfires
                {
                  int32 r = irand(0, 119);
                    if (r < 20)                           // 1/6 polymorph
                        m_caster->CastSpell(m_caster,23444,true);
                    else if (r < 100)                     // 4/6 evil twin
                        m_caster->CastSpell(m_caster,23445,true);
                    else                                    // 1/6 miss the target
                        m_caster->CastSpell(m_caster,36902,true);
                    return;
                }
                case 23453:                                 // Ultrasafe Transporter: Gadgetzan
                    if (roll_chance_i(50))                // success
                        m_caster->CastSpell(m_caster,23441,true);
                    else                                    // failure
                        m_caster->CastSpell(m_caster,23446,true);
                    return;
                case 23645:                                 // Hourglass Sand
                    m_caster->RemoveAurasDueToSpell(23170);
                    return;
                case 23725:                                 // Gift of Life (warrior bwl trinket)
                    m_caster->CastSpell(m_caster,23782,true);
                    m_caster->CastSpell(m_caster,23783,true);
                    return;
                case 24930:                                 // Hallow's End Candy
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    switch (irand(0,3))
                    {
                    case 0:
                        m_caster->CastSpell(m_caster,24927,true); // Ghost
                        break;
                    case 1:
                        m_caster->CastSpell(m_caster,24926,true); // Pirate
                        if (m_caster->GetGender() == GENDER_MALE)
                        {
                            m_caster->CastSpell(m_caster,44743,true);
                        }
                        else
                        {
                            m_caster->CastSpell(m_caster,44742,true);
                        }
                        break;
                    case 2:
                        m_caster->CastSpell(m_caster,24925,true); // Skeleton
                        break;
                    case 3:
                        m_caster->CastSpell(m_caster,24924,true); // Huge and Orange
                        break;
                    }
                    return;
                case 25860:                                 // Reindeer Transformation
                {
                    if (!m_caster->HasAuraType(SPELL_AURA_MOUNTED))
                        return;

                    float flyspeed = m_caster->GetSpeedRate(MOVE_FLIGHT);
                    float speed = m_caster->GetSpeedRate(MOVE_RUN);

                    m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

                    //5 different spells used depending on mounted speed and if mount can fly or not
                    if (flyspeed >= 4.1f)
                        m_caster->CastSpell(m_caster, 44827, true); //310% flying Reindeer
                    else if (flyspeed >= 3.8f)
                        m_caster->CastSpell(m_caster, 44825, true); //280% flying Reindeer
                    else if (flyspeed >= 1.6f)
                        m_caster->CastSpell(m_caster, 44824, true); //60% flying Reindeer
                    else if (speed >= 2.0f)
                        m_caster->CastSpell(m_caster, 25859, true); //100% ground Reindeer
                    else
                        m_caster->CastSpell(m_caster, 25858, true); //60% ground Reindeer

                    return;
                }
                case 52845:                                 // Items 39476, 39477
                case 49357:                                 // Item 37750, 37816
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if (!m_caster->HasAuraType(SPELL_AURA_MOUNTED))
                        return;

                    float speed = m_caster->GetSpeedRate(MOVE_RUN);
                    float flyspeed = m_caster->GetSpeedRate(MOVE_FLIGHT);

                    m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

                    if(((Player*)m_caster)->GetTeam() == HORDE)
                    {
                        if ((flyspeed >= 1.6f) || (flyspeed >= 3.8f) || (flyspeed >= 4.1f))
                            return;
                        else if (speed >= 2.0f)
                            m_caster->CastSpell(m_caster, 49379, true); //100% Great Brewfest Kodo
                        else if (speed >= 1.6f)
                            m_caster->CastSpell(m_caster, 49378, true); //60% Brewfest Kodo
                    }
                    else if(((Player*)m_caster)->GetTeam() == ALLIANCE)
                    {
                        if ((flyspeed >= 1.6f) || (flyspeed >= 3.8f) || (flyspeed >= 4.1f))
                            return;
                        else if (speed >= 2.0f)
                            m_caster->CastSpell(m_caster, 43900, true); //100% Swift Brewfest Ram
                        else if (speed >= 1.6f)
                            m_caster->CastSpell(m_caster, 43899, true); //60% Brewfest Ram
                    }

                    return;
                }
                //case 26074:                               // Holiday Cheer
                //    return; -- implemented at client side
                case 28006:                                 // Arcane Cloaking
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
                    {
                        unitTarget->ToPlayer()->RewardDNDQuest(9378);
                        m_caster->CastSpell(unitTarget,29294,true);
                    }
                    return;
                }
                /*case 28730:                                 // Arcane Torrent (Mana)
                {
                    Aura * dummy = m_caster->GetDummyAura(28734);
                    if (dummy)
                    {
                        int32 bp = damage * dummy->GetStackAmount();
                        m_caster->CastCustomSpell(m_caster, 28733, &bp, NULL, NULL, true);
                        m_caster->RemoveAurasDueToSpell(28734);
                    }
                    return;
                }*/
                // Polarity Shift (Thaddius)
                case 28089:
                    if (unitTarget)
                    {
                        // neutralize the target
                        if (unitTarget->HasAura(28059, 0) ) unitTarget->RemoveAurasDueToSpell(28059);
                        if (unitTarget->HasAura(29659, 0) ) unitTarget->RemoveAurasDueToSpell(29659);
                        if (unitTarget->HasAura(28084, 0) ) unitTarget->RemoveAurasDueToSpell(28084);
                        if (unitTarget->HasAura(29660, 0) ) unitTarget->RemoveAurasDueToSpell(29660);
                        unitTarget->CastSpell(unitTarget, roll_chance_i(50) ? 28059 : 28084, true, NULL, NULL, m_caster->GetGUID());
                    }
                    break;
                // Polarity Shift (Mechano-Lord Capacitus)
                case 28414:                                 // Call of the Ashbringer
                {
                    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    static uint32 AshbringerSounds[12] = { 8906,8907,8908,8920,8921,8922,8923,8924,8925,8926,8927,8928 };
                    m_caster->PlayDirectSound(AshbringerSounds[urand(0, 11)]);
                    return;
                }
                case 39096:
                    {
                    if (unitTarget->HasAura(28059, 0) ) unitTarget->RemoveAurasDueToSpell(39088);
                        if (unitTarget->HasAura(29659, 0) ) unitTarget->RemoveAurasDueToSpell(39089);
                        if (unitTarget->HasAura(28084, 0) ) unitTarget->RemoveAurasDueToSpell(39091);
                        if (unitTarget->HasAura(29660, 0) ) unitTarget->RemoveAurasDueToSpell(39092);

                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, roll_chance_i(50) ? 39088 : 39091, true, NULL, NULL, m_caster->GetGUID());
                    break;
                    }
                case 29200:                                 // Purify Helboar Meat
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spell_id = roll_chance_i(50) ? 29277 : 29278;

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 29858:                                 // Soulshatter
                    if (unitTarget && unitTarget->CanHaveThreatList()
                        && unitTarget->getThreatManager().getThreat(m_caster) > 0.0f)
                        m_caster->CastSpell(unitTarget,32835,true);
                    return;
                case 29883: // Blink (Karazhan, Arcane Anomaly)
                {
                    if (!unitTarget)
                        return;

                    m_caster->NearTeleportTo(unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), m_caster->GetOrientation());
                    // Drop all threat
                    std::list<HostileReference*> PlayerList = m_caster->getThreatManager().getThreatList();

                    if(PlayerList.empty())
                        return;

                    for(std::list<HostileReference*>::iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    {
                        Unit* pUnit = m_caster->GetMap()->GetUnit((*i)->getUnitGuid());
                        if (pUnit && m_caster->getThreatManager().getThreat(pUnit))
                            m_caster->getThreatManager().modifyThreatPercent(pUnit, -100);
                    }

                    // Give some base threat
                    m_caster->getThreatManager().addThreat(unitTarget, 1000.0f);
                    return;
                }
                case 30458:                                 // Nigh Invulnerability
                    
                    if (!m_CastItem) 
                        return;

                    if (roll_chance_i(86))                  // Nigh-Invulnerability   - success
                        m_caster->CastSpell(m_caster, 30456, true, m_CastItem);
                    else                                    // backfire in 14% casts
                        m_caster->CastSpell(m_caster, 30457, true, m_CastItem);

                    return;
                case 30507:                                 // Poultryizer
                    if (!m_CastItem) return;
                    if (roll_chance_i(80))                   // success
                        m_caster->CastSpell(unitTarget, 30501, true, m_CastItem);
                    else                                    // backfire 20%
                        m_caster->CastSpell(unitTarget, 30504, true, m_CastItem);
                    return;
                case 228:                                 // Poultryizer Polymorpher
                {
                    if (!m_CastItem)
                        return;

                    m_caster->CastSpell(unitTarget, 30501, true, m_CastItem);
                    return;
                }
                case 54759:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if (m_caster->ToPlayer()->InBattleGroundOrArena())
                        m_caster->ToPlayer()->LeaveBattleground();
                    return;
                }
                case 54653:                                    // Combat Dummy Combat Leave
                {
                    if (m_caster->GetTypeId() == TYPEID_UNIT && m_caster->GetDummyAura(54839))
                    {
                        m_caster->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, m_caster->GetVictim() ? m_caster->GetVictim()->GetMaxHealth() : 0);
                        m_caster->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1, m_caster->GetVictim() ? m_caster->GetVictim()->GetMaxHealth() : 0);
                        m_caster->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2, m_caster->GetVictim() ? m_caster->GetVictim()->GetMaxHealth() : 0);
                        return;
                    }
                    else if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    m_caster->CombatStop();
                    m_caster->getHostileRefManager().deleteReferences(); //leave combat
                    m_caster->SetHealth(m_caster->GetMaxHealth()); //restore hp
                    m_caster->SetPower(POWER_MANA, m_caster->GetMaxPower(POWER_MANA)); // restore mana
                    m_caster->SetPower(POWER_RAGE, 0); //delete rage
                    m_caster->SetPower(POWER_ENERGY, m_caster->GetMaxPower(POWER_ENERGY)); // restore energy
                    m_caster->ToPlayer()->RemoveArenaSpellCooldowns();

                    return;
                }
                //// respec
                //case 55404:
                //{
                //    Player* p = m_caster->ToPlayer();
                //    if (!p)
                //        return;
                //    
                //    uint32 spec = m_currentBasePoints[0];
                //    assert(spec > MAX_TALENT_SPECS);
                //    p->ActivateSpec(spec);
                //    return;
                //}
                case 55150:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    Player* Plr = m_caster->ToPlayer();

                    char chr[10];
                    sprintf(chr, "%u", m_currentBasePoints[0]);
                    bool Weapon = bool(chr[1]-48);
                    uint8 slot = (chr[2]-48) ? 10 : (chr[3]-48);
                    uint32 itemId = (chr[4]-48)*100000 + (chr[5]-48)*10000 + (chr[6]-48)*1000 + (chr[7]-48)*100 + (chr[8]-48)*10 + (chr[9]-48);
                    ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype(itemId);

                    if (!itemProto)
                    {
                        char chrErr[256];
                        sprintf(chrErr, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 2);
                        Plr->GetSession()->SendNotification(chrErr);
                        return;
                    }

                    uint32 battery;

                    // shoudlers, head
                    if (itemProto->InventoryType == INVTYPE_SHOULDERS || itemProto->InventoryType == INVTYPE_HEAD)
                    {
                        battery = TRANSMOG_BATTERY_2;
                        if (!Plr->HasItemCount(battery, 1))
                        {
                            Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_NO_TRANS_REAGENT2));
                            return;
                        }
                    }
                    // chest, legs
                    else if (itemProto->InventoryType == INVTYPE_ROBE || itemProto->InventoryType == INVTYPE_CHEST || itemProto->InventoryType == INVTYPE_LEGS)
                    {
                        battery = TRANSMOG_BATTERY_3;
                        if (!Plr->HasItemCount(battery, 1))
                        {
                            Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_NO_TRANS_REAGENT3));
                            return;
                        }
                    }
                    // offset
                    else if (itemProto->InventoryType == INVTYPE_CLOAK || itemProto->InventoryType == INVTYPE_HANDS || itemProto->InventoryType == INVTYPE_FEET || itemProto->InventoryType == INVTYPE_WAIST || itemProto->InventoryType == INVTYPE_WRISTS || itemProto->InventoryType == INVTYPE_TABARD)
                    {
                        battery = TRANSMOG_BATTERY_4;
                        if (!Plr->HasItemCount(battery, 1))
                        {
                            Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_NO_TRANS_REAGENT4));
                            return;
                        }
                    }
                    // weapon, don't care about hard check
                    else
                    {
                        battery = TRANSMOG_BATTERY_1;
                        if (!Plr->HasItemCount(battery, 1))
                        {
                            Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_NO_TRANS_REAGENT1));
                            return;
                        }
                    }

                    // item check is fine, continue
                    if (Plr->GetTransmogManager()->Add(Weapon, slot, itemId))
                    {
                        Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_TRANS_ADDED));
                        if (sWorld.getConfig(CONFIG_TRANSMOG_REQ_REAGENT))
                            Plr->DestroyItemCount(battery, 1, true, false, "CUSTOM_DESTROY");
                        Plr->CastSpell(Plr, 55166, true);
                    }
                    else
                    {
                        char chrErr[256];
                        sprintf(chrErr, Plr->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
                        Plr->GetSession()->SendNotification(chrErr);
                    }
                        
                    return;
                }
                case 55116: // Item teleporter
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    Player* Plr = m_caster->ToPlayer();

                    WorldLocation OldRecall = Plr->_recallPosition;
                    Plr->SaveRecallPosition();
                    switch (m_currentBasePoints[0])
                    {
                        case 1001: // Darnassus
                            Plr->TeleportTo(1, 9947.52f, 2482.73f, 1316.21f, 4.73f);
                            break;
                        case 1005: // Exodar
                            Plr->TeleportTo(530, -3954.20f, -11656.54f, -138.69f, 1.66f);
                            break;
                        case 1010: // Ironforge
                            Plr->TeleportTo(0, -4924.07f, -951.95f, 501.55f, 5.40f);
                            break;
                        case 1015: // Stormwind
                            Plr->TeleportTo(0, -8960.14f, 516.266f, 96.3568f, 0.68f);
                            break;
                        //////////////////////////////////////////////////HORDE///////////////////////////////////////////////////////////////
                        case 2001: // Orgrimmar
                            Plr->TeleportTo(1, 1664.23f, -4345.12f, 62.0102f, 3.867f);
                            break;
                        case 2005: // Silvermoon
                            Plr->TeleportTo(530, 9526.51f, -7300.27f, 15.2325f, 3.1f);
                            break;
                        case 2010: // Thunder Bluff
                            Plr->TeleportTo(1, -1197.04f, 30.9699f, 176.949f, 4.729000f);
                            break;
                        case 2015: // Undercity
                            Plr->TeleportTo(0, 1595.73f, 240.264f, 62.028f, 6.27f);
                            break;
                        //////////////////////////////////////////////////NEUTRAL///////////////////////////////////////////////////////////////
                        case 3035:// Shattrath City
                            Plr->TeleportTo(530, -1850.209961f, 5435.821777f, -10.961435f, 3.403913f);
                            break;
                        case 3005:// Booty Bay
                            Plr->TeleportTo(0, -14438.2f, 472.22f, 15.32f, 0.65);
                            break;
                        case 3015: //Everlook
                            Plr->TeleportTo(1, 6724.77f, -4610.68f, 720.78f, 4.78f);
                            break;
                        case 3020: //Gadgetzan
                            Plr->TeleportTo(1, -7173.26f, -3785.60f, 8.37f, 6.13f);
                            break;
                        case 3025: //Mudsprocket
                            Plr->TeleportTo(1, -4564.79f, -3172.38f, 33.93f, 3.21f);
                            break;
                        case 3030: //Ratchet
                            Plr->TeleportTo(1, -956.664f, -3754.71f, 5.33239f, 0.996637f);
                            break;
                        case 3040:// Isle Of Quel'Danas
                            Plr->TeleportTo(530, 12947.4f, -6893.31f, 5.68398f, 3.09154f);
                            break;
                        case 3045:// Aeris Landing
                            Plr->TeleportTo(530, -2105.47f, 8587.09f, 18.22f, 0.03f);
                            break;
                        case 3055: // Altar of Sha'tar
                            Plr->TeleportTo(530, -3045.61f, 821.313f, -10.5034f, 5.7f);
                            break;
                        case 3065: // Bronzebeard Encampment
                            Plr->TeleportTo(1, -8029.3818359375f, 1104.29541015625f, 6.27918004989624f, 1);
                            break;
                        case 3070: // Cenarion Hold
                            Plr->TeleportTo(1, -6812.36767578125f, 803.790771484375f, 51.4796485900879f, 3.0f);
                            break;
                        case 3075: // Cenarion Refuge
                            Plr->TeleportTo(530, -201.32048034668f, 5551.98828125f, 23.3083515167236f, 6);
                            break;
                        case 3080: // Cosmowrench
                            Plr->TeleportTo(530, 2955.91845703125f, 1782.75708007813f, 143.437622070313f, 1);
                            break;
                        case 3085: // Emerald Sanctuary
                            Plr->TeleportTo(1, 4000.341796875f, -1295.60144042969f, 254.221771240234f, 4);
                            break;
                        case 3100: // Valor's Rest
                            Plr->TeleportTo(1, -6387.18f, -297.97f, -2.388f, 4.81f);
                            break;
                        case 3105: // Halaa
                            Plr->TeleportTo(530, -1603.86f, 7765.25f, -21.9298f, 1.55f);
                            break;
                        case 3110: // Harborage
                            Plr->TeleportTo(0, -10106.6181640625f, -2817.39013671875f, 22.005012512207f, 5);
                            break;
                        case 3115: // Kirin'Var Village
                            Plr->TeleportTo(530, 2124.54931640625f, 2302.89208984375f, 73.6288986206055f, 0);
                            break;
                        case 3120: // Light's Hope Chapel
                            Plr->TeleportTo(0, 2287.22f, -5324.61f, 90.91f, 2.13f);
                            break;
                        case 3125: // Marshal's Refuge
                            Plr->TeleportTo(1, -6277.1f, -1126.65f, -224.935f, 3.97f);
                            break;
                        case 3135: // Midrealm Post
                            Plr->TeleportTo(530, 3401.38452148438f, 2922.79467773438f, 158.908935546875f, 6);
                            break;
                        case 3140: // Mirage Raceway
                            Plr->TeleportTo(1, -6202.0283203125f, -3902.23828125f, -60.2820510864258f, 0);
                            break;
                        case 3150: // Nesingwary's Expedition
                            Plr->TeleportTo(0, -11583.6962890625f, -38.9316635131836f, 11.0776624679565f, 5);
                            break;
                        case 3155: // Nighthaven
                            Plr->TeleportTo(1, 7796.24462890625f, -2574.78857421875f, 488.476989746094f, 0);
                            break;
                        case 3160: // Ogri'la
                            Plr->TeleportTo(530, 2310.12f, 7288.73f, 365.617f, 0.18f);
                            break;
                        case 3165: // Protectorate Watch Post
                            Plr->TeleportTo(530, 4247.81f, 2210.92f, 137.719f, 5.08f);
                            break;
                        case 3175: // Sanctum of the Stars
                            Plr->TeleportTo(530, -4076.57f, 1081.55f, 33.4521f, 1.93f);
                            break;
                        case 3180: // Sporeggar
                            Plr->TeleportTo(530, 224.467f, 8533.64f, 23.4459f, 2.64f);
                            break;
                        case 3185: // Steamwheedle Port
                            Plr->TeleportTo(1, -6933.19f, -4936.27f, 0.71469f, 1.47f);
                            break;
                        case 3190: // Stormspire
                            Plr->TeleportTo(530, 4200.9f, 3100.61f, 335.821f, 4.08f);
                            break;
                        case 3195: // Thorium Point
                            Plr->TeleportTo(0, -6501.11f, -1171.78f, 309.215f, 2.17f);
                            break;
                        case 3200: // Timbermaw Hold
                            Plr->TeleportTo(1, 6997.32f, -2106.95f, 588.566f, 4.93f);
                            break;
                        //////////////////////////////////////////////////KALIMDOR///////////////////////////////////////////////////////////////
                        case 6001:// Blackfathom Deeps
                            Plr->TeleportTo(1, 4248.72f, 744.35f, -24.67f, 1.34f);
                            break;
                        case 6005://Old Hillsbrad Foothills
                            Plr->TeleportTo(1, -8404.29f, -4070.62f, -208.58f, 4.94f);
                            break;
                        case 6006://The Black Morass
                            Plr->TeleportTo(1, -8761.95f, -4185.10f, -209.49f, 4.94f);
                            break;
                        case 6010:// Dire Maul
                            Plr->TeleportTo(1, -3960.95f, 1130.64f, 161.05f, 0.0f);
                            break;
                        case 6015:// Maraudon
                            Plr->TeleportTo(1, -1431.33f, 2962.34f, 98.23f, 4.74f);
                            break;
                        case 6020:// Onyxia's Lair
                            Plr->TeleportTo(1, -4707.44f, -3726.82f, 54.6723f, 3.8f);
                            break;
                        case 6021:// Hyjal
                            Plr->TeleportTo(1, -8177.89f, -4181.22f, -167.55f, 3.8f);
                            break;
                        case 6025:// Ragefire Chasm
                            Plr->TeleportTo(1, 1814.47f, -4419.46f, -18.78f, 5.28f);
                            break;
                        case 6030:// Razorfen Downs
                            Plr->TeleportTo(1, -4657.88f, -2525.59f, 81.4f, 4.16f);
                            break;
                        case 6035:// Razorfen Kraul
                            Plr->TeleportTo(1, -4463.6f, -1664.53f, 82.26f, 0.85f);
                            break;
                        case 6040:// Ruins of Ahn'Qiraj
                            Plr->TeleportTo(1, -8413.33f, 1501.27f, 29.64f, 2.61f);
                            break;
                        case 6045:// Temple of Ahn'Qiraj
                            Plr->TeleportTo(1, -8245.837891f, 1983.736206f, 129.071686f, 0.936195f);
                            break;
                        case 6050:// Wailing Caverns
                            Plr->TeleportTo(1, -722.53f,-2226.30f,16.94f,2.71f);
                            break;
                        case 6055:// Zul'Farrak
                            Plr->TeleportTo(1, -6801.9f, -2890.22f, 8.88f, 6.25f);
                            break;
                        //////////////////////////////////////////////////EASTERN KINGDOMS///////////////////////////////////////////////////////////////
                        case 7001:// Blackrock Depths
                            Plr->TeleportTo(0, -7180.57, -920.04f, 165.49f, 5.02f);
                            break;
                        case 7005:// Blackrock Spire
                            Plr->TeleportTo(0, -7526.77f, -1225.64f, 285.73f, 5.31f);
                            break;
                        case 7010:// Blackwing Lair
                            Plr->TeleportTo(469, -7672.61f, -1107.21f, 396.65f, 3.75f);
                            break;
                        case 7015:// Deadmines
                            Plr->TeleportTo(0, -11208.2f, 1675.92f, 24.57f, 1.48f);
                            break;
                        case 7020:// Gnomeregan
                            Plr->TeleportTo(0, -5163.32f, 927.18f, 257.158, 1.44f);
                            break;
                        case 7025:// Isle Of Quel'Danas
                            Plr->TeleportTo(530, 12589.5f, -6775.5f, 15.0916f, 3.02f);
                            break;
                        case 7030:// Karazhan
                            Plr->TeleportTo(0, -11119.6f, -2011.42f, 47.09f, 0.65f);
                            break;
                        case 7035:// Molten Core
                            Plr->TeleportTo(230, 1114.85f, -457.76f, -102.81f, 3.83f);
                            break;
                        case 7040:// Scarlet Monastery
                            Plr->TeleportTo(0, 2843.89f,-693.74f,139.32f,5.11f);
                            break;
                        case 7045:// Scholomance
                            Plr->TeleportTo(0, 1273.06f, -2574.01f, 92.66f, 2.06f);
                            break;
                        case 7050:// Shadowfang Keep
                            Plr->TeleportTo(0, -239.54f, 1550.8f, 76.89f, 1.18f);
                            break;
                        case 7055:// Stratholme
                            Plr->TeleportTo(0, 3370.76f, -3343.63f, 142.26f, 5.23f);
                            break;
                        case 7060:// Sunken Temple
                            Plr->TeleportTo(0, -10346.92f, -3851.90f, -43.41f, 6.09f);
                            break;
                        case 7065:// The Stockade
                            Plr->TeleportTo(0, -8766.89f, 844.6f, 88.43f, 0.69f);
                            break;
                        case 7070:// Uldaman
                            Plr->TeleportTo(0, -6070.72f, -2955.33f, 209.78f, 0.05f);
                            break;
                        case 7075:// Zul'Aman
                            Plr->TeleportTo(530, 6851.09f, -7979.71f, 183.54f, 4.72f);
                            break;
                        case 7080:// Zul'Gurub
                            Plr->TeleportTo(0, -11916.4f, -1216.23f, 92.28f, 4.75f);
                            break;
                        case 7081:// Naxxrama
                            Plr->TeleportTo(0, 3101.64f, -3704.07f, 131.41f, 4.75f);
                            break;

                        //////////////////////////////////////////////////OUTLAND///////////////////////////////////////////////////////////////
                        case 8001:// Auchindoun
                            Plr->TeleportTo(530, -3322.92f, 4931.02f, -100.56f, 1.86f);
                            break;
                        case 8005:// Black Temple
                            Plr->TeleportTo(530, -3649.1f, 317.33f, 35.19f, 2.97f);
                            break;
                        case 8010:// Coilfang: The Steamvault     
                            Plr->TeleportTo(530, 794.53f, 6927.81f, -80.47f, 0.34f);
                            break;
                        case 8033:// Coilfang: The Underbog
                            Plr->TeleportTo(530, 779.39f, 6758.12f, -72.53f, 0.34f);
                            break;
                        case 8034:// Coilfang: The Slave Pens
                            Plr->TeleportTo(530, 717.28f, 6979.87f, -73.02f, 0.34f);
                            break;
                        case 8037:// Serpentshrine Cavern
                            Plr->TeleportTo(530, 820.02f, 6864.93f, -66.75f, 0.34f);
                            break;
                        case 8015:// Gruul's Lair
                            Plr->TeleportTo(530, 3539.01f, 5082.36f, 1.69f, 0.0f);
                            break;
                        case 8020:// Hellfire Citadel: The Shattered Halls
                            Plr->TeleportTo(530, -305.79f, 3061.62f, -2.53f, 2.05f);
                            break;
                        case 8030:// Hellfire Citadel: The Blood Furnace
                            Plr->TeleportTo(530, -291.32f, 3149.10f, 31.55f, 2.05f);
                            break;
                        case 8031:// Hellfire Citadel: Ramparts
                            Plr->TeleportTo(530, -360.67f, 3071.89f, -15.09f, 2.05f);
                            break;
                        case 8021:// Magtheridon
                            Plr->TeleportTo(530, -316.32f, 3094.59f, -116.43f, -116.43f);
                            break;
                        case 8022:// Magister's Terrace
                            Plr->TeleportTo(530, 12884.59f, -7317.68f, 65.50f, -116.43f);
                            break;
                        case 8025:// Tempest Keep: The Arcatraz 
                            Plr->TeleportTo(530, 3308.91f, 1340.71f, 505.55f, 4.63f);
                            break;
                        case 8035:// Tempest Keep: The Botanica
                            Plr->TeleportTo(530, 3407.11f, 1488.47f, 182.83f, 4.63f);
                            break;
                        case 8036:// Tempest Keep: The Mechanar
                            Plr->TeleportTo(530, 2867.12f, 1549.42f, 252.15f, 4.63f);
                            break;
                        case 8042:// Tempest Keep: The Eye
                            Plr->TeleportTo(530, 3099.36f, 1518.72f, 190.30f, 4.75f);
                            break;
                        case 8038://Auchindoun: Shadow Labyrinth
                            Plr->TeleportTo(530, -3627.89f, 4941.97f, -101.04f, 4.63f);
                            break;
                        case 8039://Auchindoun: Sethekk Halls
                            Plr->TeleportTo(530, -3362.19f, 4664.12f, -101.04f, 4.63f);
                            break;
                        case 8040://Auchindoun: Mana-Tombs
                            Plr->TeleportTo(530, -3104.17f, 4945.52f, -101.50f, 4.63f);
                            Plr->KilledMonster(690712, 0, 693034);
                            break;
                        case 8041://Auchindoun: Auchenai Crypts
                            Plr->TeleportTo(530, -3362.04f, 5209.85f, -101.05f, 4.63f);
                            break;
                        case 8999://Guild House    
                            Plr->TeleportTo(1, 16224.19f, 16283.73f, 13.17f, 1.46f);
                            break;
                            ///////////////////////////////////////////////////RECALL////////////////////////////////
                        case 9000:
                            Plr->TeleportTo(OldRecall);
                            break;
                    }
                    return;
                }
                case 55163: // Race Change
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    Player* Plr = m_caster->ToPlayer();

                    if (i == 0)
                    {
                        if (uint8 new_race = m_currentBasePoints[0]) // race change
                        {
                            // Allowed races: 1-11 except 9
                            if (new_race < 1 || new_race > 11 || new_race == 9)
                            {
                                Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_ERROR));
                                return;
                            }

                            uint32 tokenId = Player::TeamForRace(new_race) == ALLIANCE ? 23700 : 23701;
                            // 23700 - alliance token, 23701 - horde token
                            if (!Plr->HasItemCount(tokenId, 1))
                                Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DO_NOT_HAVE_ITEM_1));
                            else
                            {
                                if (Plr->m_changeRaceTo)
                                    Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ALREADY_CHANGING));
                                else if (Plr->GetRace() == new_race)
                                    Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_CANT_SAME_RACE));
                                else
                                {
                                    uint32 oldTeamGuildCheck = Plr->GetTeam();
                                    uint32 newTeamGuildCheck = Player::TeamForRace(new_race);
                                    bool abortChange = false;
                                    // Prohibit and warn about faction change if Guild Master
                                    if (oldTeamGuildCheck != newTeamGuildCheck)
                                    {
                                        if (uint32 guildId = Plr->GetGuildId())
                                        {
                                            if (Guild *guild = sGuildMgr.GetGuildById(guildId))
                                            {
                                                if (Plr->GetGUID() == guild->GetLeader())
                                                {
                                                    // do error if not alone in guild. If alone -> simply allow faction change without guild leave
                                                    if (guild->GetMemberSize() > 1)
                                                    {
                                                        Plr->GetSession()->SendGuildCommandResult(GUILD_QUIT_S, "", GUILD_LEADER_LEAVE);
                                                        abortChange = true;
                                                    }
                                                }
                                                // guild leave will be done at next login
                                            }
                                        }
                                    }

                                    if (!abortChange)
                                    {
                                        if (m_CastItem && m_CastItem->GetEntry() == tokenId)
                                            m_CastItem = NULL;
                                        Plr->DestroyItemCount(tokenId, 1, true, false, "CUSTOM_DESTROY");
                                        Plr->m_changeRaceTo = new_race;
                                        Plr->ChangeRaceSwapItems(new_race);
                                        ChatHandler(Plr).SendSysMessage(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_AFTER_RELOG));
                                    }
                                }
                            }
                        }
                    }

                    else if (i == 1)
                    {
                        if (m_currentBasePoints[1]) // gender change
                        {
                            if (!Plr->HasItemCount(33573, 1))
                                Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DO_NOT_HAVE_ITEM_2));
                            else
                            {
                                if (m_CastItem && m_CastItem->GetEntry() == 33573)
                                    m_CastItem = NULL;
                                Plr->DestroyItemCount(33573, 1, true, false, "CUSTOM_DESTROY");
                                uint32 displayId = Plr->GetNativeDisplayId();
                                uint32 new_displayId = displayId;
                                Gender newGender = ((Plr->GetGender() == GENDER_MALE) ? GENDER_FEMALE : GENDER_MALE);

                                if (newGender == GENDER_MALE)
                                    new_displayId = (Plr->GetRace() == RACE_BLOODELF) ? (displayId+1) : (displayId-1);
                                else // gender is female
                                    new_displayId = (Plr->GetRace() == RACE_BLOODELF) ? (displayId-1) : (displayId+1);

                                // Set gender
                                Plr->SetByteValue(UNIT_FIELD_BYTES_0, 2, newGender);
                                Plr->SetByteValue(PLAYER_BYTES_3, 0, newGender);

                                // Change display ID
                                Plr->SetDisplayId(new_displayId);
                                Plr->SetNativeDisplayId(new_displayId);

                                uint32 pBytes = Plr->GetUInt32Value(PLAYER_BYTES);
                                uint32 pBytes2 = Plr->GetUInt32Value(PLAYER_BYTES_2);

                                Plr->AppearanceCheckAndFixIfNeeded(pBytes, pBytes2);

                                Plr->SetUInt32Value(PLAYER_BYTES, pBytes);
                                Plr->SetUInt32Value(PLAYER_BYTES_2, pBytes2);
                            }
                        }
                    }

                    else if (i == 2)
                    {
                        if (m_currentBasePoints[2]) // name change
                        {
                            if (!Plr->HasItemCount(33572, 1))
                                Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_DO_NOT_HAVE_ITEM_3));
                            else
                            {
                                if (!Plr->HasAtLoginFlag(AT_LOGIN_RENAME))
                                {
                                    if (m_CastItem && m_CastItem->GetEntry() == 33572)
                                        m_CastItem = NULL;
                                    Plr->DestroyItemCount(33572, 1, true, false, "CUSTOM_DESTROY");
                                    Plr->SetAtLoginFlag(AT_LOGIN_RENAME);
                                    ChatHandler(Plr).SendSysMessage(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_AFTER_RELOG));
                                }
                                else
                                    Plr->GetSession()->SendNotification(Plr->GetSession()->GetHellgroundString(LANG_RACECHANGER_ALREADY_NAMECHANGED));
                            }
                        }
                    }
                    return;
                }
                case 55191:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55192, true);
                    return;
                }
                case 55197:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55198, true);
                    return;
                }
                case 55250:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55251, true);
                    return;
                }
                case 55254:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55255, true);
                    return;
                }
                case 55256:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55257, true);
                    return;
                }
                case 55265: // Spectral Gryphon
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55266, true);
                    return;
                }
                case 55267: // Bone Gryphon
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55268, true);
                    return;
                }
                case 55269: // New Year 2018 Mount
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55270, true);
                    return;
                }
                case 55338: // New Year 2019 Mount
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    m_caster->CastSpell(m_caster, 55339, true);
                    return;
                }
                case 1206:
                {
                    // 1206 is NOT COUNTED AS SPELL CAST - does not call for remove aura on AURA_INTERRUPT_FLAG_CAST (for 55181 and 55182 right work)
                    if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_CastItem)
                        return;

                    uint32 SpellId = 0;
                    int32 BasePoints1 = 0;
                    int32 BasePoints2 = 0;
                    int32 BasePoints3 = 0;
                    switch (m_CastItem->GetEntry())
                    {
                        case ITEM_SUMMONING_STONE:
                            m_caster->CastSpell(m_caster, 55600, false);
                            break;
                        case 999016:
                        case 999017:
                        case 999018:
                            SpellId = 45049;
                            break;
                        case 993056: 
                            SpellId = 54871;
                            break;
                        case 999015:
                            SpellId = 54874;
                            break;
                        case 693010:
                            SpellId = 55155;
                            break;
                        case 199002: // Book of teleportation
                        case TRANSMOGRIFICATOR: // Transmog items. Transmogrificator
                            // if you have used an item and then spell -> the aura 55182 will be removed by a spellcast
                            // if you first used a spell and then used an item -> the aura 55181 will be removed by a spellcast
                            if (Aura* aur = m_caster->GetDummyAura(55181)) // Mount savior
                            {
                                int32 bp = aur->GetModifierValue(); // mount spell
                                int32 bp2 = 1; // slow fall
                                m_caster->CastCustomSpell(m_caster, 55182, &bp, &bp2, 0, true, m_CastItem);
                                m_caster->RemoveAurasDueToSpell(55181); // it is needed. Without that you can spam transmogrificator and bug out mount
                            }
                            break;
                        //case ENDLESS_BUFF_SCROLL:
                        case BUFF_SCROLL: // Buff Scroll
                        {
                            Unit* target = m_caster;

                            if (Creature* pet = m_caster->GetPet())
                            {
                                if (m_caster->GetSelection() == pet->GetGUID())
                                    target = pet;
                            }

                            uint32 del_auras[] = { 25389,25312,25433,27126,26990,26992,20217,27144,27140,1038,27168,27142 };
                            for (const uint32 del_aura : del_auras)
                            {
                                target->RemoveAurasDueToSpell(del_aura);
                            }

                            Creature* trigger = m_caster->SummonTrigger(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 0, 1);
                            if (!trigger)
                                return;
                            BasePoints1 = 102; // 79 base + 30% = 103
                            trigger->CastCustomSpell(target, 25389, &BasePoints1, NULL, NULL, true); // Power word: fortitude. +30% from talent

                            BasePoints1 = 50;
                            BasePoints2 = 10;
                            BasePoints3 = 10;
                            trigger->CastCustomSpell(target, 25312, &BasePoints1, &BasePoints2, &BasePoints3, true); // Divine Spirit. improved with +10% SPD from spirit

                            trigger->CastSpell(target, 25433, true); // Shadow protection.

                            trigger->CastSpell(target, 27126, true); // Arcane Intellect

                            BasePoints1 = 459;
                            BasePoints2 = 18;
                            BasePoints3 = 33;
                            trigger->CastCustomSpell(target, 26990, &BasePoints1, &BasePoints2, &BasePoints3, true);// Mark of the Wild. Improved with + 35%

                            BasePoints1 = 43;
                            trigger->CastCustomSpell(target, 26992, &BasePoints1, NULL, NULL, true);// Thorns improved with +75% talent

                            // PALADIN BLESSINGS. : Improved Blessing of Might, Improved Blessing of Wisdom
                            if (trigger)
                            {
                                trigger->CastSpell(target, 20217, true);// Blessing of Kings
                            }
                            trigger = m_caster->SummonTrigger(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 0, 1);
                            if (trigger)
                            {
                                trigger->CastSpell(target, 27144, true);// Blessing of Light
                            }
                            trigger = m_caster->SummonTrigger(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 0, 1);
                            if (trigger)
                            {
                                BasePoints1 = 264;
                                BasePoints2 = 264;
                                trigger->CastCustomSpell(target, 27140, &BasePoints1, &BasePoints2, NULL, true);// Blessing of Might + 20% from talent
                            }
                            trigger = m_caster->SummonTrigger(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 0, 1);
                            if (trigger)
                            {
                                trigger->CastSpell(target, 1038, true);// Blessing of Salvation
                            }
                            trigger = m_caster->SummonTrigger(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 0, 1);
                            if (trigger)
                            {
                                trigger->CastSpell(target, 27168, true);// Blessing of Sanctuary
                            }
                            trigger = m_caster->SummonTrigger(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 0, 1);
                            if (trigger)
                            {
                                BasePoints1 = 49;
                                trigger->CastCustomSpell(target, 27142, &BasePoints1, NULL, NULL, true);// Blessing of Wisdom + 20% talent
                            }

                            Unit::AuraMap& Auras = target->GetAuras();
                            for (Unit::AuraMap::iterator itr = Auras.begin(); itr != Auras.end(); ++itr)
                            {
                                switch (itr->second->GetSpellProto()->Id)
                                {
                                case 25389:
                                case 25312:
                                case 25433:
                                case 27126:
                                case 26990:
                                case 26992:
                                case 20217:
                                case 27144:
                                case 27140:
                                case 1038:
                                case 27168:
                                case 27142:
                                    (*itr).second->SetAuraDuration(60 * 60 * 24 * 1000);
                                    (*itr).second->UpdateAuraDuration();
                                }
                            }

                            m_caster->ToPlayer()->KilledMonster(690710, 0, 693033);

                            break;
                        }
                        case SCROLL_OF_INDULGENCE:
                        {
                            Player* plr = m_caster->ToPlayer();
                            if (plr->HasAura(SPELL_ARENA_DESERTER))
                            {
                                plr->RemoveAurasDueToSpell(SPELL_ARENA_DESERTER);
                            }

                            if (plr->HasAura(SPELL_BG_DESERTER))
                            {
                                plr->RemoveAurasDueToSpell(SPELL_BG_DESERTER);
                            }
                            
                            break;
                        }
                        // max level
                        case 1042:
                        {
                            Player* plr = m_caster->ToPlayer();
                            plr->SetDifficulty(DIFFICULTY_NORMAL);
                            plr->GiveLevel(70);
                            plr->InitTalentForLevel();
                            plr->SetUInt32Value(PLAYER_XP, 0);
                            ChatHandler(plr).PSendSysMessage(LANG_YOURS_LEVEL_UP, 70);
                            break;
                            // DONT FORGET BREAK!
                        }
                        case ITEM_NEW_YEAR_2016_ELF_SUMMON_ITEM:
                        {
                            Player* plr = m_caster->ToPlayer();
                            if ((plr->GetQuestStatus(693500) == QUEST_STATUS_COMPLETE && !plr->GetQuestRewardStatus(693500)) ||
                                (plr->GetQuestStatus(693501) == QUEST_STATUS_COMPLETE && !plr->GetQuestRewardStatus(693501)) ||
                                (plr->GetQuestStatus(693502) == QUEST_STATUS_COMPLETE && !plr->GetQuestRewardStatus(693502)) ||
                                (plr->GetQuestStatus(693503) == QUEST_STATUS_COMPLETE && !plr->GetQuestRewardStatus(693503)))
                            {
                                Position pos;
                                plr->GetValidPointInAngle(pos, 3, 0, true);
                                Creature* moroz = plr->SummonCreature(693101, pos.x, pos.y, pos.z, M_PI+plr->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 60000);
                                if (moroz)
                                    moroz->CastSpell(moroz, 16713, true);
                            }
                            m_caster->CastSpell(m_caster, 55190, true);
                            break;
                        }
                        case 693152:
                            m_caster->CastSpell(m_caster, 55191, false);
                            break;
                        case LION:
                            m_caster->CastSpell(m_caster, 55197, false);
                            break;
                        case ROTTEN_BEAR:
                            m_caster->CastSpell(m_caster, 55250, false);
                            break;
                        case SMOKY_PANTHER:
                            m_caster->CastSpell(m_caster, 55254, false);
                            break;
						case BENGAL_TIGER:
							m_caster->CastSpell(m_caster, 55413, false);
							break;
                        case 693007: // Mech Strider
                            m_caster->CastSpell(m_caster, 55256, false);
                            break;
                        case GLAD_DRAKE:
                        case MERC_DRAKE:
                        case VENG_DRAKE:
                        {
                            uint32 itemId = m_CastItem->GetEntry();
                            uint32 spellFly = 37015; // basic - nether drake
                            uint32 spellGround = 55258;
                            if (itemId == MERC_DRAKE)
                            {
                                spellFly = 44744;
                                spellGround = 55259;
                            }
                            else if (itemId == VENG_DRAKE)
                            {
                                spellFly = 49193;
                                spellGround = 55260;
                            }

                            // can fly or not?
                            if (GetVirtualMapForMapAndZone(m_caster->GetMapId(), m_caster->GetZoneId()) == 530)
                                m_caster->CastSpell(m_caster, spellFly, false);
                            else
                                m_caster->CastSpell(m_caster, spellGround, false);
                            break;
                        }
                        case 693013: // Spectral Gryphon
                            m_caster->CastSpell(m_caster, 55265, false);
                            break;
                        case BONE_GRYPHON: // Bone Gryphon
                            m_caster->CastSpell(m_caster, 55268, false);
                            break;
                        case 693015: // Yellow Qiraji Resonating Crystal
                            m_caster->CastSpell(m_caster, 55261, false);
                            break;
                        case 693016: // Green Qiraji Resonating Crystal
                            m_caster->CastSpell(m_caster, 55262, false);
                            break;
                        case RED_QIRAJI: // Red Qiraji Resonating Crystal
                            m_caster->CastSpell(m_caster, 55263, false);
                            break;
                        case BLUE_QIRAJI:
                        {
                            Player* player = (Player*)m_caster;
                            
                            if (!player->IsGuildHouseOwnerMember())
                            {
                                ChatHandler(player).PSendSysMessage(15514);
                                return;
                            }

                            m_caster->CastSpell(m_caster, 55264, false);
                            break;
                        }
                        case DRAGONHAWK:
                        {
                            Player* player = (Player*)m_caster;

                            if (!player->GetSession()->isPremium())
                            {
                                ChatHandler(player).PSendSysMessage(15095);
                                return;
                            }

                            m_caster->CastSpell(m_caster, 55407, false);
                            break;
                        }
						case WHITE_DEER:
							m_caster->CastSpell(m_caster, 55417, false);
							break;
                        case 693018: // Blue Qiraji Resonating Crystal
                            m_caster->CastSpell(m_caster, 55264, false);
                            break;
                        case 693019:
                            m_caster->CastSpell(m_caster, 55269, false);
                            break;
                        case SNOW_BEAR:
                            m_caster->CastSpell(m_caster, 55338, false);
                            break;
                        case BRONZE_DRAGON: // Bronze Dragon
                            m_caster->CastSpell(m_caster, 55400, false);
                            break;  
                        case PINK_ELEKK:
							m_caster->CastSpell(m_caster, 55601, false);
							break;
                        case LEOPARD:
                            m_caster->CastSpell(m_caster, 55602, false);
                            break;
                        case UNICORN:
                            m_caster->CastSpell(m_caster, 55603, false);
                            break;
						case TEMPORAL_RIFT_DRAGON:
							m_caster->CastSpell(m_caster, 55418, false);
							break;
                        // Premium Remote - Premium NPC call
                        case 31665:
                        {
                            Player* plr = ((Player*)m_caster);
                            if (!plr->GetSession()->isPremium())
                                ChatHandler(plr).SendSysMessage(LANG_PREMIUM_NOT_ACTIVE);
                            else
                            {
                                m_caster->CastSpell(m_caster, 55314, false);
                                m_caster->SendPlaySound(12182, true);
                            }
                            break;
                        }
                        case 30471: // 7d premium
                        {
                            if (WorldSession* s = ((Player*)m_caster)->GetSession())
                            {
                                if (s->IsAccountFlagged(ACC_PREMIUM_SCROLL_ACTIVATED))
                                {
                                    ChatHandler((Player*)m_caster).SendSysMessage(16654);
                                    break;
                                }

                                s->addPremiumTime(7 * DAY);
                                ChatHandler((Player*)m_caster).SendSysMessage(16652);
                                s->AddAccountFlag(ACC_PREMIUM_SCROLL_ACTIVATED);
                            }

                            break;
                        }
                        case 21930: // beta participation mecha-spider minipet
                        {
                            m_caster->CastSpell(m_caster, 55316, false);
                            break;
                        }
                        case LAMP_WITH_A_WISP: // Lamp with a wisp. referal lvl 10 minipet
                        {
                            m_caster->CastSpell(m_caster, 55317, false);
                            break;
                        }
                        case 21233: // Fast Riding Turtle
                        {
                            if (GetVirtualMapForMapAndZone(m_caster->GetMapId(), ((Player*)m_caster)->GetCachedZone()) != 530)
                            {
                                switch (((Player*)m_caster)->GetBaseSkillValue(SKILL_RIDING))
                                {
                                case 75: m_caster->CastSpell(m_caster, 55321, false); break;
                                case 150: case 225: case 300: m_caster->CastSpell(m_caster, 55318, false); break;
                                default: break;
                                }
                            }
                            else
                            {
                                switch (((Player*)m_caster)->GetBaseSkillValue(SKILL_RIDING))
                                {
                                case 75: m_caster->CastSpell(m_caster, 55321, false); break;
                                case 150: m_caster->CastSpell(m_caster, 55318, false); break;
                                case 225: m_caster->CastSpell(m_caster, 55320, false); break;
                                case 300: m_caster->CastSpell(m_caster, 55319, false); break;
                                default: break;
                                }
                            }
                            break;
                        }
                        //BUGGED(broom model is appearing when unmounted sometimes)
                        //case MAGIC_BROOM: // Magic Broom (x100)
                        //{                          
                        //    m_caster->CastSpell(m_caster, 42684, false);
                        //    break;
                        //}
                        case 23770: // Portable Rocket
                        {
                            m_caster->CastSpell(m_caster, 55322, false);
                            break;
                        }
                        case 35517: // Apprentice Riding (60% mount, 35g)
                        {
                            m_caster->CastSpell(m_caster, 33389, true);
                            break;
                        }
                        case 35519: // Journeyman Riding (100% mount, 600g)
                        {
                            m_caster->CastSpell(m_caster, 33392, true);
                            break;
                        }
                        case 35520: // Expert riding (flying 60%, 800g)
                        {
                            m_caster->CastSpell(m_caster, 34092, true);
                            break;
                        }
                        case 35521: // Artisan Riding (flying max, 5000g)
                        {
                            m_caster->CastSpell(m_caster, 34093, true);
                            break;
                        }
                        case 4420: // Scroll of Boundless Flights
                        {
                            // send level gain visual
                            WorldPacket data2(SMSG_PLAY_SPELL_VISUAL, 12); // visual effect on player
                            data2 << uint64(m_caster->GetGUID()) << uint32(0x016A);
                            m_caster->BroadcastPacket(&data2, true);
                            ((Player*)m_caster)->AddPlayerCustomFlag(PL_CUSTOM_TAXICHEAT);
                            break;
                        }
                        case 33573: // Token "Gender change"
                        {
                            int32 bpaction = 1;
                            m_caster->CastCustomSpell(m_caster, 55163, NULL, &bpaction, NULL, false);
                            break;
                        }
                        case 33572: // Token "Name change"
                        {
                            if (((Player*)m_caster)->HasAtLoginFlag(AT_LOGIN_RENAME))
                            {
                                ChatHandler((Player*)m_caster).SendSysMessage(LANG_RACECHANGER_ALREADY_NAMECHANGED);
                                break;
                            }
                            int32 bpaction = 1;
                            m_caster->CastCustomSpell(m_caster, 55163, NULL, NULL, &bpaction, false);
                            break;
                        }
                        case 23796: // 100% xp bottle
                        {
                            m_caster->CastSpell(m_caster, 55336, false, m_CastItem);
                            m_CastItem = NULL;
                            return;
                        }
						case 1924: // Magic Spyglass
						{
							((Player*)m_caster)->AddPlayerCustomFlag(PL_CUSTOM_TAXICHEAT);
							ChatHandler((Player*)m_caster).SendSysMessage(16571);
							return;
						}

						// Book of Knowledge case
						case 1085: // poisons
						{
							if (p->GetClass() == CLASS_ROGUE)
								m_caster->CastSpell(m_caster, 2995, false);
							return;
						}
						case 1086: // Voidwalker
						{
							if (p->GetClass() == CLASS_WARLOCK)
								m_caster->CastSpell(m_caster, 11520, false);
							return;
						}
						case 1087: // Succubus
						{
							if (p->GetClass() == CLASS_WARLOCK)
								m_caster->CastSpell(m_caster, 11519, false);
							return;
						}
						case 1088: // Felhunter
						{
							if (p->GetClass() == CLASS_WARLOCK)
								m_caster->CastSpell(m_caster, 1373, false);
							return;
						}
						case 1089: // Infernal
						{
							if (p->GetClass() == CLASS_WARLOCK)
								m_caster->CastSpell(m_caster, 1413, false);
							return;
						}
						case 1090: // Bear Form
						{
							if (p->GetClass() == CLASS_DRUID)
								m_caster->CastSpell(m_caster, 19179, false);
							return;
						}
						case 1091: // Cure Poison
						{
							if (p->GetClass() == CLASS_DRUID)
								m_caster->CastSpell(m_caster, 8946, false);
							return;
						}
						case 1092: // Aqua Form
						{
							if (p->GetClass() == CLASS_DRUID)
								m_caster->CastSpell(m_caster, 1446, false);
							return;
						}
						case 1093: // Berserker Stance and Intercept
						{
							if (p->GetClass() == CLASS_WARRIOR)
								m_caster->CastSpell(m_caster, 8616, true);
							return;
						}
						case 1095: // Defensive Stance, Sunder Armor and Taunt
						{
							if (p->GetClass() == CLASS_WARRIOR)
								m_caster->CastSpell(m_caster, 8121, false);
							return;
						}
						case 1096: // all spells needed for pets
						{
							if (p->GetClass() == CLASS_HUNTER)
							{
								p->CastSpell(m_caster, 5300, false);
								p->CastSpell(m_caster, 1579, false);
							}
							return;
						}
						case 1099: // Redemption
						{
							if (p->GetClass() == CLASS_PALADIN)
								m_caster->CastSpell(m_caster, 7329, false);
							return;
						}
						case 1100: // Stoneskin Totem
						{
							if (p->GetClass() == CLASS_SHAMAN)
								m_caster->CastSpell(m_caster, 8073, false);
							return;
						}
						case 1101: // Searing Totem
						{
							if (p->GetClass() == CLASS_SHAMAN)
								m_caster->CastSpell(m_caster, 2075, false);
							return;
						}
						case 1102: // Healing Stream Totem
						{
							if (p->GetClass() == CLASS_SHAMAN)
								m_caster->CastSpell(m_caster, 5396, false);
							return;
						}

                        case 23698: // 200% xp bottle
                        {
                            m_caster->CastSpell(m_caster, 55337, false, m_CastItem);
                            m_CastItem = NULL;
                            return;
                        }
                        case 1199: // 250 ap
                        {
                            Player* plr = ((Player*)m_caster);
                            if (plr->GetArenaPoints() + 250 > sWorld.getConfig(CONFIG_MAX_ARENA_POINTS))
                            {
                                plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW, NULL, NULL);
                                return;
                            }
                            uint32 cnt = 1;
                            plr->DestroyItemCount(m_CastItem, cnt, true, "CUSTOM_DESTROY");
                            m_CastItem = NULL;
                            plr->ModifyArenaPoints(int32(250));
                            return;
                        }
                        case 5172: // 50 ap
                        {
                            uint32 amount = 50;
                            
                            Player* plr = ((Player*)m_caster);
                            if (plr->GetArenaPoints() + amount > sWorld.getConfig(CONFIG_MAX_ARENA_POINTS))
                            {
                                plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW, NULL, NULL);
                                return;
                            }
                            uint32 cnt = 1;
                            plr->DestroyItemCount(m_CastItem, cnt, true, "CUSTOM_DESTROY");
                            m_CastItem = NULL;
                            plr->ModifyArenaPoints(int32(amount));
                            return;
                        }
                        case 2767: // 400 honor
                        {
                            uint32 amount = 400;
                            
                            Player* plr = ((Player*)m_caster);
                            if (plr->GetHonorPoints() + amount > sWorld.getConfig(CONFIG_MAX_HONOR_POINTS))
                            {
                                plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW, NULL, NULL);
                                return;
                            }
                            uint32 cnt = 1;
                            plr->DestroyItemCount(m_CastItem, cnt, true, "CUSTOM_DESTROY");
                            m_CastItem = NULL;
                            plr->ModifyHonorPoints(int32(amount));
                            WorldPacket data(SMSG_PVP_CREDIT, 4 + 8 + 4);
                            data << (uint32)(amount);
                            data << (uint64)0;
                            data << (uint32)0;
                            plr->SendPacketToSelf(&data);
                            return;
                        }
                        case 5564: // 2000 honor
                        {
                            Player* plr = ((Player*)m_caster);
                            if (plr->GetHonorPoints() + 2000 > sWorld.getConfig(CONFIG_MAX_HONOR_POINTS))
                            {
                                plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW, NULL, NULL);
                                return;
                            }
                            uint32 cnt = 1;
                            plr->DestroyItemCount(m_CastItem, cnt, true, "CUSTOM_DESTROY");
                            m_CastItem = NULL;
                            plr->ModifyHonorPoints(int32(2000));
                            WorldPacket data(SMSG_PVP_CREDIT, 4 + 8 + 4);
                            data << (uint32)(2000);
                            data << (uint64)0;
                            data << (uint32)0;
                            plr->SendPacketToSelf(&data);
                            return;
                        }
                        case ANTIQUE_SHARD:
                        case AZERITE_SHARD:
                        case GHOST_SHARD:
                        {
                            Player* plr = ((Player*)m_caster);
                            
                            // check required items
                            uint32 shard = m_CastItem->GetEntry();
                            uint32 shard_cnt = 3;

                            if (!plr->HasItemCount(shard, shard_cnt))
                            {
                                char chrErr[256];
                                sprintf(chrErr, plr->GetSession()->GetHellgroundString(LANG_NOT_ENOUGH_SHARDS), 2);
                                plr->GetSession()->SendNotification(chrErr);
                                return;
                            }

                            // check free space (maybe use _CanTakeMoreSimilarItems() ?)
                            uint32 key;

                            if (shard == ANTIQUE_SHARD)
                                key = ANTIQUE_KEY;
                            else if (shard == AZERITE_SHARD)
                                key = AZERITE_KEY;
                            else if (shard == GHOST_SHARD)
                                key = GHOST_KEY;
                            else
                                return;

                            ItemPosCountVec dest;
                            uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, key, 1);
                            if (msg != EQUIP_ERR_OK)
                            {
                                char chrErr[256];
                                sprintf(chrErr, plr->GetSession()->GetHellgroundString(LANG_SCRIPT_INVENTORY_NO_SPACE), 2);
                                plr->GetSession()->SendNotification(chrErr);
                                return;
                            }

                            // remove shards
                            plr->DestroyItemCount(shard, shard_cnt, true, false, "FRAGMENT_CONVERSION");
                            m_CastItem = NULL; // MUST HAVE TO PREVENT GetEntry() CRASH IN TakeCastItem()

                            // add key
                            Item* Item = plr->StoreNewItem(dest, key, true, 0, "FRAGMENT_CONVERSION");
                            plr->SendNewItem(Item, 1, true, true);

                            return;
                        }
                        //case RARE_KEY_FRAGMENT:
                        case EPIC_KEY_FRAGMENT:
                        case LEGENDARY_KEY_FRAGMENT:
                        {
                            Player* plr = ((Player*)m_caster);

                            // check required items
                            uint32 shard = m_CastItem->GetEntry();
                            int32 shard_cnt = 5;

                            if (!plr->HasItemCount(shard, shard_cnt))
                            {
                                plr->GetSession()->SendNotification(plr->GetSession()->GetHellgroundString(15523));
                                return;
                            }

                            uint32 key;

                            //if (shard == RARE_KEY_FRAGMENT)
                            //    key = RARE_KEY;
                            //else 
                            if (shard == EPIC_KEY_FRAGMENT)
                                key = EPIC_KEY;
                            else if (shard == LEGENDARY_KEY_FRAGMENT)
                                key = LEGENDARY_KEY;
                            else
                                return;

                            ItemPosCountVec dest;
                            uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, key, 1);
                            if (msg != EQUIP_ERR_OK)
                            {
                                char chrErr[256];
                                sprintf(chrErr, plr->GetSession()->GetHellgroundString(LANG_SCRIPT_INVENTORY_NO_SPACE), 2);
                                plr->GetSession()->SendNotification(chrErr);
                                return;
                            }

                            // remove shards
                            plr->DestroyItemCount(shard, shard_cnt, true, false, "FRAGMENT_CONVERSION");
                            m_CastItem = NULL; // MUST HAVE TO PREVENT GetEntry() CRASH IN TakeCastItem()

                            // add key
                            Item* Item = plr->StoreNewItem(dest, key, true, 0, "FRAGMENT_CONVERSION");
                            plr->SendNewItem(Item, 1, true, true);

                            return;
                        }
                        case BOJ_FRAGMENT:
                        {
                            Player* plr = ((Player*)m_caster);

                            // check required items
                            uint32 shard = BOJ_FRAGMENT;
                            uint32 shard_cnt = 5;

                            if (!plr->HasItemCount(shard, shard_cnt))
                            {
                                char chrErr[256];
                                sprintf(chrErr, plr->GetSession()->GetHellgroundString(LANG_NOT_ENOUGH_FRAGMENTS), 2);
                                plr->GetSession()->SendNotification(chrErr);
                                return;
                            }

                            // check free space (maybe use _CanTakeMoreSimilarItems() ?)
                            uint32 give_item = 29434;
                            ItemPosCountVec dest;
                            uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, give_item, 1);
                            if (msg != EQUIP_ERR_OK)
                            {
                                char chrErr[256];
                                sprintf(chrErr, plr->GetSession()->GetHellgroundString(LANG_SCRIPT_INVENTORY_NO_SPACE), 2);
                                plr->GetSession()->SendNotification(chrErr);
                                return;
                            }

                            // remove shards
                            plr->DestroyItemCount(shard, shard_cnt, true, false, "FRAGMENT_CONVERSION");
                            m_CastItem = NULL; // MUST HAVE TO PREVENT GetEntry() CRASH IN TakeCastItem()

                            // add key
                            Item* Item = plr->StoreNewItem(dest, give_item, true, 0, "FRAGMENT_CONVERSION");
                            plr->SendNewItem(Item, 1, true, true);

                            return;
                        }
                        // reputation books                        
                        case 8954:
                        {                            
                            //Cenarion Expedition (TBC)
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(942, 42999);
                            return;
                        }
                        case 8955:
                        {
                            //Keepers of Time (TBC Caverns of Time)
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(989, 42999);
                            return;
                        }
                        case 8958:
                        {
                            //Lower City (TBC Auchindoun)
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(1011, 42999);
                            return;
                        }
                        case 8960:
                        {
                            //The Consortium (TBC Mana Tombs)
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(933, 42999);
                            return;
                        }
                        case 8961:
                        {
                            //The Sha'tar (TBC Tempest Keep)
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(935, 42999);
                            return;
                        }
                        case 8962:
                        {
                            //The Scryers (TBC Tier & Vendors)
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(934, 42999);
                            plr->GetReputationMgr().ModifyReputation(932, -42999);
                            return;
                        }
                        case 8963:
                        {
                            //The Aldor (Tier & Vendors)
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(932, 42999);
                            plr->GetReputationMgr().ModifyReputation(934, -42999);
                            return;
                        }
                        case 8964:
                        {
                            //Thrallmar (TBC Hellfire Citadel)
                            Player* plr = ((Player*)m_caster);

                            if (plr->GetTeam() == HORDE)
                                plr->GetReputationMgr().ModifyReputation(947, 42999);
                            return;
                        }
                        case 8965:
                        {
                            //Honor Hold (Hellfire Citadel)
                            Player* plr = ((Player*)m_caster);

                            if (plr->GetTeam() == ALLIANCE)
                                plr->GetReputationMgr().ModifyReputation(946, 42999);

                            return;
                        }
                        case 8966:
                        {
                            //Shattered Sun
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(1077, 42999);
                            return;
                        }
                        case 8967:
                        {
                            //The Scale of the Sands
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(990, 42999);
                            return;
                        }
                        case 8968:
                        {
                            //Ashtongue Deathsworn
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(1012, 42999);
                            return;
                        }
                        case 8969:
                        {
                            //The Violet Eye
                            Player* plr = ((Player*)m_caster);
                            plr->GetReputationMgr().ModifyReputation(967, 42999);
                            return;
                        }
                        case EPIC_RAID_CHEST:
                        case LEGENDARY_RAID_CHEST:
                        {
                            if (uint32 id = sScriptMgr.GetScriptId("mw_player_raidchest"))
                                sScriptMgr.OnGossipHello(((Player*)m_caster), id, m_CastItem->GetEntry() == LEGENDARY_RAID_CHEST ? 1 : 0);

                            return;
                        }
                        default:
                            return;
                    }

                    if (SpellId && (BasePoints1 || BasePoints2 || BasePoints3))
                        m_caster->CastCustomSpell(m_caster, SpellId, &BasePoints1, &BasePoints2, &BasePoints3, true); // In this case unitTarget = caster, so use caster instead of unittarget
                    else if (SpellId)
                        m_caster->CastSpell(m_caster, SpellId, true); // In this case unitTarget = caster, so use caster instead of unittarget
                    
                    uint32 SpellId2 = 0;
                    int32 BasePoints1_2 = 0;
                    int32 BasePoints2_2 = 0;
                    int32 BasePoints3_3 = 0;
                    switch (m_CastItem->GetEntry())
                    {
                        case 999016:
                        case 999017:
                        case 999018:
                            SpellId2 = 33668;
                            break;
                        case 999015:
                            SpellId2 = 43620; // just for visual
                            BasePoints2_2 = 1;
                            break;
                        default:
                            return;
                    }

                    if (SpellId2 && (BasePoints1_2 || BasePoints2_2 || BasePoints3_3))
                        m_caster->CastCustomSpell(m_caster, SpellId2, &BasePoints1, &BasePoints2, &BasePoints3, true); // In this case unitTarget = caster, so use caster instead of unittarget
                    else if (SpellId2)
                        m_caster->CastSpell(m_caster, SpellId2, true); // In this case unitTarget = caster, so use caster instead of unittarget

                    return;
                }
                case 55336:
                case 55337:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_CastItem)
                        return;

                    uint32 cnt = 1;
                    ((Player*)m_caster)->DestroyItemCount(m_CastItem, cnt, true, "CUSTOM_DESTROY");
                    m_CastItem = NULL;
                    return;
                }
                case 54741:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    m_caster->CombatStop();
                    m_caster->getHostileRefManager().deleteReferences(); //leave combat

                    return;
                }
                case 55183:
                {
                    sLog.outLog(LOG_CHEAT, "Spell 55183 is called");
                    return;
                }
                //case 54761:
                //{
                //    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                //        return;

                //    if (Unit* PetOwner = m_caster->GetOwner())
                //    {
                //        if (PetOwner->GetTypeId() != TYPEID_PLAYER)
                //            return;

                //        if (PetOwner->GetClass() == CLASS_HUNTER) // Trentone - How about mages? - frost
                //        {
                //            if (PetOwner->HasAura(19596, 0))
                //                m_caster->CastSpell(m_caster, 54762, true);

                //            int32 CRIT = PetOwner->GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE)*0.3f;
                //            m_caster->CastCustomSpell(m_caster, 54763, &CRIT, &CRIT, NULL, true);
                //        }
                //        else if (PetOwner->GetClass() == CLASS_WARLOCK)
                //        {
                //            int32 CRIT = PetOwner->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_SHADOW)*0.3f; // select shadow  or fire, the one that has more crit
                //            m_caster->CastCustomSpell(m_caster, 54763, &CRIT, &CRIT, NULL, true);
                //        }                        
                //    }

                //    // resilience for pets may be added here too. NO resilience in Burning Crusade!

                //    return;
                //}
                case 54825: // Wings of Death
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_caster->isAlive())
                        return;
                    if (!m_caster->IsInCombat())
                    {
                        if (!m_caster->HasAura(24742, 0)) // krilya 50% damage and heal
                            m_caster->AddAura(24742, m_caster); 
                        if (!m_caster->HasAura(39943, 0)) // shadowform
                            m_caster->AddAura(39943, m_caster);
                        if (!m_caster->ToPlayer()->InBattleGroundOrArena()) 
                        {
                            if (!m_caster->HasAura(54824, 0))// polet
                                m_caster->AddAura(54824, m_caster);
                        }
                        else
                        {
                            if (m_caster->HasAura(54824, 0))
                                m_caster->RemoveAurasDueToSpell(54824);// esli na bg to net poleta
                            if (m_caster->HasAura(54826, 0))
                                m_caster->RemoveAurasDueToSpell(54826);// 210% skorost
                            return;
                        }
                        uint32 v_map = GetVirtualMapForMapAndZone(m_caster->GetMapId(), m_caster->GetZoneId());
                        if (v_map == 530 && !m_caster->HasAura(54826, 0))
                            m_caster->AddAura(54826, m_caster);
                        else if (v_map != 530 && m_caster->HasAura(54826, 0))
                            m_caster->RemoveAurasDueToSpell(54826);
                            
                    }
                    else
                    {
                        if (m_caster->HasAura(54826, 0))// +210% skorosti poleta
                            m_caster->RemoveAurasDueToSpell(54826);
                        if (m_caster->HasAura(54824, 0))// polet + 100%
                            m_caster->RemoveAurasDueToSpell(54824);
                    }
                    return;
                }
                case 54828://npc dresser undress
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_caster->isAlive())
                        return;

                    if (!m_caster->HasAura(54829, 0))
                    {
                        m_caster->RemoveAurasDueToSpell(54827);
                        m_caster->RemoveSpellsCausingAura(SPELL_AURA_TRANSFORM);
                        m_caster->RemoveSpellsCausingAura(SPELL_AURA_PERMANENT_TRANSFORM);
                    }
                    return;
                }
                case 54870:
                {
                    Player *Leaver = m_caster->ToPlayer();
                    if (!Leaver || !Leaver->IsSpectator() || !Leaver->InArena())
                        return;
                    Leaver->AddAura(54824, Leaver);
                    Leaver->AddFakeArenaQueue(Leaver, Leaver->GetMapId());
                    return;
                }
                case 55169: // arena preparation aura selector for special class. Gives GCD remove and Cast removal. Also heals.
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    m_caster->ToPlayer()->restorePowers();
                    return;
                }
                case 55201:
                {
                    if (m_caster->GetTypeId() != TYPEID_UNIT)
                        return;

                    if (!m_caster->ToCreature()->IsAIEnabled)
                        return;

                    m_caster->ToCreature()->AI()->EventPulse(m_caster, 0);
                    return;
                }
                // spec change spells
                case 55326:
                case 55327:
                case 55328:
                case 55329:
                case 55330:
                case 55331:
                case 55332:
                case 55333:
                case 55334:
                case 55335:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    ((Player*)m_caster)->resetTalents(true);
                    return;
                }
                // Shadow Labyrinth: Murmur - Sonic Boom trigger
                case 38796:
                case 33923:                    
                {
                    m_caster->CastSpell(m_caster, GetSpellEntry()->Id == 38796 ? 38795 : 33666, true);
                    return;
                }
                // Arcane Torrent (Mana)
                case 28730:                                 // Arcane Torrent (Mana)
                case 33390:                                 // Arcane Torrent (mana Wretched Devourer)
                {
                    Unit* caster = GetCaster();

                    if (!caster)
                        return;

                    if (caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                    
                    Aura* dummy = caster->GetDummyAura(28734);
                    if (dummy)
                    {
                        int32 bp = (2.17 * caster->GetLevel() + 9.136) * dummy->GetStackAmount();
                        caster->CastCustomSpell(caster, 28733, &bp, NULL, NULL, true);
                        caster->RemoveAurasDueToSpell(28734);
                    }
                    break;
                }
                // Arcane Torrent (Energy)
                case 25046:
                {
                    Unit * caster = GetCaster();

                    if (!caster)
                        return;

                    // Search Mana Tap auras on caster
                    Aura * dummy = caster->GetDummyAura(28734);
                    if (dummy)
                    {
                        int32 bp = dummy->GetStackAmount() * 10;
                        caster->CastCustomSpell(caster, 25048, &bp, NULL, NULL, true);
                        caster->RemoveAurasDueToSpell(28734);
                    }
                    return;
                }
                case 33060:                                         // Make a Wish
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 spell_id = 0;

                    switch (urand(1,5))
                    {
                        case 1: spell_id = 33053; break;
                        case 2: spell_id = 33057; break;
                        case 3: spell_id = 33059; break;
                        case 4: spell_id = 33062; break;
                        case 5: spell_id = 33064; break;
                    }

                    m_caster->CastSpell(m_caster,spell_id,true,NULL);
                    return;
                }
                case 35745:                                 // Socrethar's Stone
                {
                    uint32 spell_id;
                    uint32 areaid = m_caster->GetTypeId() == TYPEID_PLAYER ? ((Player*)m_caster)->GetCachedArea() : m_caster->GetAreaId();

                    switch (areaid)
                    {
                        case 3900: spell_id = 35743; break;
                        case 3742: spell_id = 35744; break;
                        default: return;
                    }

                    m_caster->CastSpell(m_caster,spell_id,true);
                    return;
                }
                case 37674:                                 // Chaos Blast
                {
                    if (!unitTarget)
                        return;

                    int32 basepoints0 = 100;
                    m_caster->CastCustomSpell(unitTarget,37675,&basepoints0,NULL,NULL,true);
                    return;
                }
                case 40109:                                 // Knockdown Fel Cannon: The Bolt
                {
                    if (unitTarget)
                        unitTarget->CastSpell(unitTarget, 40075, true);
                    return;
                }
                case 40802:                                 // Mingo's Fortune Generator (Mingo's Fortune Giblets)
                {
                    // selecting one from Bloodstained Fortune item
                    uint32 newitemid;
                    switch (urand(1,20))
                    {
                        case 1:  newitemid = 32688; break;
                        case 2:  newitemid = 32689; break;
                        case 3:  newitemid = 32690; break;
                        case 4:  newitemid = 32691; break;
                        case 5:  newitemid = 32692; break;
                        case 6:  newitemid = 32693; break;
                        case 7:  newitemid = 32700; break;
                        case 8:  newitemid = 32701; break;
                        case 9:  newitemid = 32702; break;
                        case 10: newitemid = 32703; break;
                        case 11: newitemid = 32704; break;
                        case 12: newitemid = 32705; break;
                        case 13: newitemid = 32706; break;
                        case 14: newitemid = 32707; break;
                        case 15: newitemid = 32708; break;
                        case 16: newitemid = 32709; break;
                        case 17: newitemid = 32710; break;
                        case 18: newitemid = 32711; break;
                        case 19: newitemid = 32712; break;
                        case 20: newitemid = 32713; break;
                        default:
                            return;
                    }

                    DoCreateItem(i,newitemid);
                    return;
                }
                case 40834: // Agonizing Flames
                {
                    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    m_caster->CastSpell(unitTarget,40932,true);
                    break;
                }
                case 44875:                                 // Complete Raptor Capture
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    Creature* creatureTarget = (Creature*)unitTarget;

                    creatureTarget->setDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);           // just for nice GM-mode view

                    //cast spell Raptor Capture Credit
                    m_caster->CastSpell(m_caster,42337,true,NULL);
                    return;
                }
                case 34665:                                 //Administer Antidote
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    Creature* pCreature = (Creature*)unitTarget;

                    pCreature->UpdateEntry(16992); // change into dreadtusk
                    pCreature->AIM_Initialize();

                    if (pCreature->IsAIEnabled)
                        pCreature->AI()->AttackStart(m_caster);

                    return;
                }
                case 44935:                                 // Expose Razorthorn Root
                {
                    if(!unitTarget)
                        return;

                    Player* plr = unitTarget->GetCharmerOrOwnerPlayerOrPlayerItself();

                    GameObject* ok = NULL;
                    Hellground::GameObjectFocusCheck go_check(plr,GetSpellEntry()->RequiresSpellFocus);
                    Hellground::ObjectSearcher<GameObject, Hellground::GameObjectFocusCheck> checker(ok,go_check);

                    Cell::VisitGridObjects(plr, checker, plr->GetMap()->GetVisibilityDistance());

                    if (!ok)
                    {
                        WorldPacket data(SMSG_CAST_FAILED, (4+1+1));
                        data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));
                        data << uint8(SPELL_FAILED_REQUIRES_SPELL_FOCUS);
                        data << uint8(m_cast_count);
                        data << uint32(GetSpellEntry()->RequiresSpellFocus);
                        plr->SendPacketToSelf(&data);
                        return;
                    }

                    if(unitTarget->GetTypeId() == TYPEID_UNIT)
                        unitTarget->GetMotionMaster()->MovePoint(1, ok->GetPositionX(), ok->GetPositionY(), ok->GetPositionZ());

                    return;
                }
                case 44997:                                 // Converting Sentry remove corpse
                {
                    if(unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
                        unitTarget->ToCreature()->DisappearAndDie();
                    return;
                }
                case 45030:                                 // Impale Emissary
                {
                    // Emissary of Hate Credit
                    m_caster->CastSpell(m_caster, 45088, true);
                    return;
                }
                case 45785:                                 // Sinister Reflection Clone
                {
                    if (!unitTarget)
                        return;

                    unitTarget->CastSpell(unitTarget, m_spellInfo->CalculateSimpleValue(i), true);
                    return;
                }
                case 45892:                                 // Sinister Reflection
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // Summon 4 clones of the same player
                    for (uint8 i = 0; i < 4; ++i)
                        unitTarget->CastSpell(unitTarget, 45891, true, NULL, NULL, m_caster->GetObjectGuid());
                    return;
                }
                case 46573:                                 // Blink (Sunblade Arch Mage)
                {
                    if (m_caster->GetTypeId() == TYPEID_UNIT)
                        m_caster->CastSpell(unitTarget, 41421, true);
                    return;
                }
                case 50243:                                 // Teach Language
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    // spell has a 1/3 chance to trigger one of the below
                    if (roll_chance_i(66))
                        return;
                    if (((Player*)m_caster)->GetTeam() == ALLIANCE)
                    {
                        // 1000001 - gnomish binary
                        m_caster->CastSpell(m_caster, 50242, true);
                    }
                    else
                    {
                        // 01001000 - goblin binary
                        m_caster->CastSpell(m_caster, 50246, true);
                    }

                    return;
                }
                case 51582:                                 //Rocket Boots Engaged (Rocket Boots Xtreme and Rocket Boots Xtreme Lite)
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if (BattleGround* bg = ((Player*)m_caster)->GetBattleGround())
                        bg->EventPlayerDroppedFlag((Player*)m_caster);

                    m_caster->CastSpell(m_caster, 30452, true, NULL);
                    return;
                }
                case 39992:                                 //Needle Spine Targeting
                {
                    m_caster->CastSpell(unitTarget,39835,true);
                    break;
                }
                case 39581:                                 // Storm Blink
                {
                    m_caster->CastSpell(m_caster, 39582, true);
                    break;
                }
                case 32225:                                 //Chess Event: Take Action (melee)
                {
                    switch (m_caster->GetEntry())
                    {
                        case 17211:     //alliance pawn (Human Footman)
                            m_caster->CastSpell((Unit*)NULL, 32227, true);
                            break;
                        case 17469:     //horde pawn (Orc Grunt)
                            m_caster->CastSpell((Unit*)NULL, 32228, true);
                            break;
                        case 21160:     //alliance rook (Conjured Water Elemental)
                            m_caster->CastSpell((Unit*)NULL, 37142, true);
                            break;
                        case 21726:     //horde rook (Summoned Daemon)
                            m_caster->CastSpell((Unit*)NULL, 37220, true);
                            break;
                        case 21664:     //alliance knight (Human Charger)
                            m_caster->CastSpell((Unit*)NULL, 37143, true);       //proper spell ??
                            break;
                        case 21748:     //horde knight (Orc Wolf)
                            m_caster->CastSpell((Unit*)NULL, 37339, true);
                            break;
                        case 21682:     //Alliance bishop (Human Cleric)
                            m_caster->CastSpell((Unit*)NULL, 37147, true);
                            break;
                        case 21747:     //Horde bishop (Orc Necrolyte)
                            m_caster->CastSpell((Unit*)NULL, 37337, true);
                            break;
                        case 21683:     //Alliance Queen (Human Conjurer)
                            m_caster->CastSpell((Unit*)NULL, 37149, true);
                            break;
                        case 21750:     //Horde Queen (Orc Warlock)
                            m_caster->CastSpell((Unit*)NULL, 37345, true);
                            break;
                        case 21684:     //Alliance King (King Llane)
                            m_caster->CastSpell((Unit*)NULL, 37150, true);
                            break;
                        case 21752:     //Horde King (Warchief Blackhand)
                            m_caster->CastSpell((Unit*)NULL, 37348, true);
                            break;
                        default:
                            m_caster->CastSpell((Unit*)NULL, 37348, true);
                            break;
                    }
                    break;
                }
                // Summon Thelrin DND (Banner of Provocation)
                case 27517:
                    m_caster->SummonCreature(16059, 590.6309, -181.061, -53.90, 5.33, TEMPSUMMON_DEAD_DESPAWN, 0);
                    break;
                case 43755:
                {
                    Aura * aur = unitTarget->GetAura(43880, 0);
                    if(aur)
                    {
                        aur->SetAuraDuration(aur->GetAuraDuration() + 30000);
                        aur->UpdateAuraDuration();
                    }
                    break;
                }
                case 19512: // Apply Salve - druid class quest 6124/6129
                {
                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if(unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    if((((Creature*)unitTarget)->GetEntry() == 12298) || (((Creature*)unitTarget)->GetEntry() == 12296))
                    {
                        ((Player*)m_caster)->CastCreatureOrGO(((Creature*)unitTarget)->GetEntry(), ((Creature*)unitTarget)->GetGUID(), 19512);

                        uint32 UpdateEntry = ((Creature*)unitTarget)->GetEntry() == 12298 ? 12299 : 12297;
                        ((Creature*)unitTarget)->RemoveAurasDueToSpell(19502);
                        ((Creature*)unitTarget)->UpdateEntry(UpdateEntry);
                        ((Creature*)unitTarget)->ForcedDespawn(20000);
                    }
                    return;
                }
                case 42339: // water bucket lands (hallow's end event)
                {
                    struct checker
                    {
                        checker(float x, float y, float z) : mx(x), my(y), mz(z) {};
                        bool operator()(WorldObject* object)
                        {
                            if (!object->IsWithinDist3d(mx, my, mz, 5.0f))
                                return false;

                            return (object->GetTypeId() == TYPEID_PLAYER) ||
                                (object->GetTypeId() == TYPEID_UNIT && object->GetEntry() == 23537); // Fire NPC
                        }
                    float mx, my, mz;
                    } check(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ);

                    std::list<Unit*> list;
                    Hellground::UnitListSearcher<checker> searcher(list, check);
                    Cell::VisitWorldObjects(m_targets.m_destX, m_targets.m_destY, m_caster->GetMap(), searcher, 5.0f);
                    if (list.empty())
                        return;

                    list.sort(Hellground::ObjectDistanceOrder(m_caster));
                    if (list.front()->GetTypeId() == TYPEID_PLAYER && list.front() != m_caster)
                    {
                        list.front()->CastSpell(list.front(), 42349, true);
                    }
                    else if (list.front()->GetTypeId() == TYPEID_UNIT)
                    {
                        m_caster->Kill(list.front());
                    }
                    break;
                }
                case 35686: // electro-shock therapy
                {
                    Creature* creatureTarget = unitTarget->ToCreature();
                    if (!creatureTarget)
                        return;

                    uint8 count = urand(5, 9);
                    if (creatureTarget->GetEntry() == 20501)
                    {
                        creatureTarget->UpdateEntry(20806, HORDE);    
                        for (uint8 i = 0; i < count; i++)
                            m_caster->SummonCreature(20806, creatureTarget->GetPositionX(), creatureTarget->GetPositionY(),
                                creatureTarget->GetPositionZ(), creatureTarget->GetOrientation(),
                                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
                    }
                    else if (creatureTarget->GetEntry() == 20778)
                    {
                        creatureTarget->UpdateEntry(20805, HORDE);
                        for (uint8 i = 0; i < count; i++)
                            m_caster->SummonCreature(20805, creatureTarget->GetPositionX(), creatureTarget->GetPositionY(),
                                creatureTarget->GetPositionZ(), creatureTarget->GetOrientation(),
                                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
                    }
                    return;
                }
                case 12938: // fel curse (blasted lands quest)
                {
                    if (!unitTarget || !unitTarget->ToCreature())
                        return;
                    unitTarget->CastSpell(unitTarget, 12941, true);
                    return;
                }
                case 37867: // frankly it makes no sense
                case 37892:
                case 37894:
                {
                    struct frankly_check
                    {
                        bool operator() (Unit*u)
                        {return (u->ToCreature() && u->GetEntry() == 21909 && u->isAlive() && !u->HasAuraType(SPELL_AURA_MOD_POSSESS));}
                    } my_check;
                    Unit* any = NULL;
                    Hellground::UnitSearcher<frankly_check> searcher(any, my_check);
                    Cell::VisitGridObjects(m_caster, searcher, 30.0f);
                    if (!any)
                        return;
                    m_caster->CastSpell(any, 37868, false);
                    return;
                }
            }

            //All IconID Check in there
            switch (GetSpellEntry()->SpellIconID)
            {
                // Berserking (troll racial traits)
                case 1661:
                {
                    uint32 healthPerc = uint32((float(m_caster->GetHealth())/m_caster->GetMaxHealth())*100);
                    int32 haste_mod = 10;
                    if (healthPerc <= 40)
                        haste_mod = 30;
                    if (healthPerc < 100 && healthPerc > 40)
                        haste_mod = 10+(100-healthPerc)/3;

                    // FIXME: custom spell required this aura state by some unknown reason, we not need remove it anyway
                    m_caster->ModifyAuraState(AURA_STATE_BERSERKING,true);
                    CustomSpellValues cvalues;
                    cvalues.AddSpellMod(SPELLVALUE_BASE_POINT0, haste_mod);
                    cvalues.AddSpellMod(SPELLVALUE_BASE_POINT1, 0);
                    cvalues.AddSpellMod(SPELLVALUE_BASE_POINT2, haste_mod);
                    m_caster->CastCustomSpell(26635, cvalues, m_caster, true, NULL);
                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_MAGE:
            switch (GetSpellEntry()->Id)
            {
                case 30610: // Wrath of the Titans
                {
                    
                    if (!m_caster->GetVictim())
                        return;

                    switch (rand()%5)
                    {
                        case 0: m_caster->CastSpell(m_caster->GetVictim(), 30605, true); break;  // Blast of Aman'Thul: Arcane
                        case 1: m_caster->CastSpell(m_caster->GetVictim(), 30606, true); break;  // Bolt of Eonar: Nature
                        case 2: m_caster->CastSpell(m_caster->GetVictim(), 30607, true); break;  // Flame of Khaz'goroth: Fire
                        case 3: m_caster->CastSpell(m_caster->GetVictim(), 30608, true); break;  // Spite of Sargeras: Shadow
                        case 4: m_caster->CastSpell(m_caster->GetVictim(), 30609, true); break;  // Chill of Norgannon: Frost
                    }
                    return;
                }
                case 11958:                                 // Cold Snap
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    const SpellCooldowns& sp_list = ((Player *)m_caster)->GetSpellCooldowns();
                    time_t thetime = time(NULL);
                    SpellCooldowns::const_iterator itr, next;
                    for (itr = sp_list.begin(); itr != sp_list.end(); itr = next)
                    {
                        next = itr;
                        ++next;
                        if (itr->second < thetime) // cooldown gone already
                            continue;

                        uint32 classspell = itr->first;
                        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(classspell);

                        if (spellInfo->SpellFamilyName == SPELLFAMILY_MAGE &&
                            ((SpellMgr::GetSpellSchoolMask(spellInfo) & SPELL_SCHOOL_MASK_FROST) || spellInfo->Category == 56/*FireWard*/) &&
                            spellInfo->Id != 11958)
                        {
                            ((Player*)m_caster)->RemoveSpellCooldown(classspell, true);
                            ((Player*)m_caster)->GetCooldownMgr().CancelGlobalCooldown(spellInfo);
                        }
                    }
                    return;
                }
                case 32826:
                {
                    if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
                    {
                        //Polymorph Cast Visual Rank 1
                        const uint32 spell_list[6] = {32813, 32816, 32817, 32818, 32819, 32820};
                        unitTarget->CastSpell(unitTarget, spell_list[urand(0, 5)], true);
                    }
                    return;
                }
                case 26373: // Lunar Invitation teleports
                {
                    static uint32 LunarEntry[6] =
                    {
                        15905, // Darnassus
                        15906, // Ironforge
                        15694, // Stormwind
                        15908, // Orgrimmar
                        15719, // Thunderbluff
                        15907 // Undercity
                    };

                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    if (((Player*)m_caster)->GetCachedZone() != 493)   // Moonglade
                        m_caster->CastSpell(m_caster, 26451, false);
                    else
                    {
                        uint32 LunarId = -1;
                        for (uint8 i=0;i<6;++i)
                        {
                            Creature *pCreature = NULL;
                            Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*m_caster, LunarEntry[i], true, 7.0, false);
                            Hellground::ObjectLastSearcher<Creature, Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);
                            Cell::VisitGridObjects(m_caster, searcher, 7.0);

                            if (pCreature)
                            {
                                LunarId = i;
                                break;
                            }
                        }

                        switch (((Player*)m_caster)->GetTeam())
                        {
                            case ALLIANCE:
                            {
                                switch (LunarId)
                                {
                                    case 0:
                                        m_caster->CastSpell(m_caster, 26450, false);  // Darnassus
                                        return;
                                    case 1:
                                        m_caster->CastSpell(m_caster, 26452, false); // Ironforge
                                        return;
                                    case 2:
                                        m_caster->CastSpell(m_caster, 26454, false); // Stormwind
                                        return;
                                    default:
                                        break;
                                }
                                break;
                            }
                            case HORDE:
                            {
                                switch (LunarId)
                                {
                                    case 3:
                                        m_caster->CastSpell(m_caster, 26453, false);  // Orgrimmar
                                        return;
                                    case 4:
                                        m_caster->CastSpell(m_caster, 26455, false); // Thunderbluff
                                        return;
                                    case 5:
                                        m_caster->CastSpell(m_caster, 26456, false); // Undercity
                                        return;
                                    default:
                                        break;
                                }
                                break;
                            }
                        }
                    }
                    return;
                }
                case 38642: // Blink
                {
                    if(!unitTarget)
                        return;
                    m_caster->CastSpell(unitTarget, 29884, true);
                    return;
                }
            }
            break;
        case SPELLFAMILY_WARRIOR:
            // Charge
            if (GetSpellEntry()->SpellFamilyFlags & 0x1 && GetSpellEntry()->SpellVisual == 867)
            {
                int32 chargeBasePoints0 = damage;
                m_caster->CastCustomSpell(m_caster,34846,&chargeBasePoints0,NULL,NULL,true);
                return;
            }
            if(GetSpellEntry()->Id == 35754)    // Charge
            {
                if (!unitTarget)
                    return;
                unitTarget->CastSpell(unitTarget, 35769, false); // Cast Felfire upon itself on hit
                return;
            }
            // Execute
            if (GetSpellEntry()->SpellFamilyFlags & 0x20000000)
            {
                if (!unitTarget)
                    return;

                spell_id = 20647;
                float MultiDMG = 1.0f;
                bp = damage + int32((m_caster->GetPower(POWER_RAGE) - GetPowerCost()) * GetSpellEntry()->DmgMultiplier[i] * MultiDMG);
                
                m_caster->SetPower(POWER_RAGE,0);
                break;
            }
            if (GetSpellEntry()->Id==21977)                      //Warrior's Wrath
            {
                if (!unitTarget)
                    return;

                m_caster->CastSpell(unitTarget,21887,true); // spell mod
                return;
            }
            break;
        case SPELLFAMILY_WARLOCK:
            //Life Tap (only it have this with dummy effect)
            if (GetSpellEntry()->SpellFamilyFlags == 0x40000)
            {
                float cost = damage;

                if (Player* modOwner = m_caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(GetSpellEntry()->Id, SPELLMOD_COST, cost,this);

                int32 dmg = m_caster->SpellDamageBonus(m_caster, GetSpellEntry(),uint32(cost > 0 ? cost : 0), SPELL_DIRECT_DAMAGE);

                if (int32(m_caster->GetHealth()) > dmg)
                {
                    // Shouldn't Appear in Combat Log
                    m_caster->ModifyHealth(-dmg);

                    int32 mana = dmg;

                    Unit::AuraList const& auraDummy = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for (Unit::AuraList::const_iterator itr = auraDummy.begin(); itr != auraDummy.end(); ++itr)
                    {
                        // only Imp. Life Tap have this in combination with dummy aura
                        if ((*itr)->GetSpellProto()->SpellFamilyName==SPELLFAMILY_WARLOCK && (*itr)->GetSpellProto()->SpellIconID == 208)
                            mana = ((*itr)->GetModifier()->m_amount + 100)* mana / 100;
                    }

                    m_caster->CastCustomSpell(m_caster,31818,&mana,NULL,NULL,true,NULL);

                    if (m_caster->GetPet())
                    {
                        // Mana Feed
                        int32 manaFeedVal = m_caster->CalculateSpellDamage(GetSpellEntry(),1, GetSpellEntry()->EffectBasePoints[1]);
                        manaFeedVal = manaFeedVal * mana / 100;
                        if (manaFeedVal > 0)
                            m_caster->CastCustomSpell(m_caster,32553,&manaFeedVal,NULL,NULL,true,NULL);
                    }
                }
                else
                    SendCastResult(SPELL_FAILED_FIZZLE);
                return;
            }
            break;
        case SPELLFAMILY_PRIEST:
            switch (GetSpellEntry()->Id)
            {
                case 36448:                                 // Focused Bursts
                case 36475:
                case 38986:
                case 38987:
                {
                    if (!unitTarget || i != 0)
                        return;

                    uint32 spellid = m_spellInfo->EffectBasePoints[urand(0, 3 - 1)] + 1;
                    m_caster->CastSpell(unitTarget, spellid, false);
                    return;
                }
                case 28598:                                 // Touch of Weakness triggered spell
                {
                    if (!unitTarget || !m_triggeredByAuraSpell)
                        return;

                    uint32 spellid = 0;
                    switch (m_triggeredByAuraSpell->Id)
                    {
                        case 2652:  spellid =  2943; break; // Rank 1
                        case 19261: spellid = 19249; break; // Rank 2
                        case 19262: spellid = 19251; break; // Rank 3
                        case 19264: spellid = 19252; break; // Rank 4
                        case 19265: spellid = 19253; break; // Rank 5
                        case 19266: spellid = 19254; break; // Rank 6
                        case 25461: spellid = 25460; break; // Rank 7
                        default:
                            sLog.outLog(LOG_DEFAULT, "ERROR: Spell::EffectDummy: Spell 28598 triggered by unhandled spell %u",m_triggeredByAuraSpell->Id);
                            return;
                    }
                    m_caster->CastSpell(unitTarget, spellid, true, NULL);
                    return;
                }
                case 34222: // Sunseeker Blessing (Botanica)
                {
                    std::list<Creature*> targetList1;
                    Hellground::AllCreaturesOfEntryInRange u_check(m_caster, 19511, 10.0f);
                    Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(targetList1, u_check);
                    Cell::VisitAllObjects(m_caster, searcher, 10.0f);
        
                    if (!targetList1.empty())
                    {
                        for(std::list<Creature*>::iterator i = targetList1.begin(); i != targetList1.end(); ++i)
                            (*i)->CastSpell((*i), 34173, false);
                    }

                    std::list<Creature*> targetList2;
                    Hellground::AllCreaturesOfEntryInRange u_check1(m_caster, 19512, 10.0f);
                    Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher1(targetList2, u_check1);
                    Cell::VisitAllObjects(m_caster, searcher1, 10.0f);
        
                    if (!targetList2.empty())
                    {
                        for(std::list<Creature*>::iterator i = targetList2.begin(); i != targetList2.end(); ++i)
                            (*i)->CastSpell((*i), 34173, false);
                    }
                }
            }
            break;
        case SPELLFAMILY_DRUID:
            switch (GetSpellEntry()->Id)
            {
                case 5420:                                  // Tree of Life passive
                {
                    // Tree of Life area effect
                    int32 health_mod = int32(m_caster->GetStat(STAT_SPIRIT)/4);
                    if (Aura *aur = m_caster->GetAura(39926, 0))         // Idol of the Raven Goddess
                        health_mod += aur->GetModifierValue();
                    m_caster->CastCustomSpell(m_caster,34123,&health_mod,NULL,NULL,true,NULL);
                    return;
                }
                case 29201:                                 // Loatheb Corrupted Mind triggered sub spells
                {
                    uint32 spellid = 0;
                    switch (unitTarget->GetClass())
                    {
                        case CLASS_PALADIN: spellid = 29196; break;
                        case CLASS_PRIEST: spellid = 29185; break;
                        case CLASS_SHAMAN: spellid = 29198; break;
                        case CLASS_DRUID: spellid = 29194; break;
                        default: 
                            break;
                    }
                    if (spellid != 0)
                        m_caster->CastSpell(unitTarget, spellid, true, NULL);
                    return;
                }
            }
            break;
        case SPELLFAMILY_ROGUE:
            switch (GetSpellEntry()->Id)
            {
                case 31231:                                 // Cheat Death
                {
                    m_caster->CastSpell(m_caster,45182,true);
                    return;
                }
                case 5938:                                  // Shiv
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    Player *pCaster = ((Player*)m_caster);

                    Item *item = pCaster->GetWeaponForAttack(OFF_ATTACK);
                    if (!item)
                        return;

                    // all poison enchantments is temporary
                    if (uint32 enchant_id = item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                    {
                        if (SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id))
                        {
                            for (int s=0;s<3;s++)
                            {
                                if (pEnchant->type[s]!=ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                    continue;

                                SpellEntry const* combatEntry = sSpellTemplate.LookupEntry<SpellEntry>(pEnchant->spellid[s]);
                                if (!combatEntry || combatEntry->Dispel != DISPEL_POISON)
                                    continue;

                                m_caster->CastSpell(unitTarget, combatEntry, true, item);
                            }
                        }
                    }

                    m_caster->CastSpell(unitTarget, 5940, true);
                    return;
                }
                // slice and dice
                case 5171:
                case 6774:
                {
                    Unit::AuraList procTriggerAuras = m_caster->GetAurasByType(SPELL_AURA_PROC_TRIGGER_SPELL);
                    for (Unit::AuraList::iterator i = procTriggerAuras.begin(); i != procTriggerAuras.end(); i++)
                    {
                        switch ((*i)->GetSpellProto()->Id)
                        {
                            // find weakness
                            case 31239:
                            case 31233:
                            case 31240:
                            case 31241:
                            case 31242:
                                m_caster->CastSpell(unitTarget, (*i)->GetSpellProto()->EffectTriggerSpell[(*i)->GetEffIndex()], true, NULL, (*i));
                                return;
                        }
                    }
                    return;
                }
            }
            break;
        case SPELLFAMILY_HUNTER:
            // Kill command
            if (GetSpellEntry()->SpellFamilyFlags & 0x00080000000000LL)
            {
                if (m_caster->GetClass()!=CLASS_HUNTER)
                    return;

                // clear hunter crit aura state
                m_caster->ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE,false);

                // additional damage from pet to pet target
                Pet* pet = m_caster->GetPet();
                if (!pet || !pet->GetVictim())
                    return;

                uint32 spell_id = 0;
                switch (GetSpellEntry()->Id)
                {
                case 34026: spell_id = 34027; break;        // rank 1
                default:
                    sLog.outLog(LOG_DEFAULT, "ERROR: Spell::EffectDummy: Spell %u not handled in KC",GetSpellEntry()->Id);
                    return;
                }

                // 4/5 set bonus
                if (m_caster->GetDummyAura(37483))
                    m_caster->CastSpell(m_caster, 37482, true);

                pet->CastSpell(pet->GetVictim(), spell_id, true);
                return;
            }

            switch (GetSpellEntry()->Id)
            {
                case 23989:                                 //Readiness talent
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    const SpellCooldowns& sp_list = ((Player *)m_caster)->GetSpellCooldowns();
                    time_t thetime = time(NULL);
                    SpellCooldowns::const_iterator itr, next;
                    for (itr = sp_list.begin(); itr != sp_list.end(); itr = next)
                    {
                        next = itr;
                        ++next;
                        if (itr->second < thetime) // cooldown gone already
                            continue;

                        uint32 classspell = itr->first;
                        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(classspell);

                        if (spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && spellInfo->Id != 23989)
                        {
                            ((Player*)m_caster)->RemoveSpellCooldown(classspell, true);
                            ((Player*)m_caster)->GetCooldownMgr().CancelGlobalCooldown(spellInfo);
                        }
                    }
                    return;
                }
                case 37506:                                 // Scatter Shot
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    // break Auto Shot and autohit
                    m_caster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
                    m_caster->AttackStop();
                    ((Player*)m_caster)->SendAttackSwingCancelAttack();
                    return;
                }
            }
            break;
        case SPELLFAMILY_PALADIN:
            switch (GetSpellEntry()->SpellIconID)
            {
                case  156:                                  // Holy Shock
                {
                    if (!unitTarget)
                        return;

                    int hurt = 0;
                    int heal = 0;

                    switch (GetSpellEntry()->Id)
                    {
                        case 20473: hurt = 25912; heal = 25914; break;
                        case 20929: hurt = 25911; heal = 25913; break;
                        case 20930: hurt = 25902; heal = 25903; break;
                        case 27174: hurt = 27176; heal = 27175; break;
                        case 33072: hurt = 33073; heal = 33074; break;
                        default:
                            sLog.outLog(LOG_DEFAULT, "ERROR: Spell::EffectDummy: Spell %u not handled in HS",GetSpellEntry()->Id);
                            return;
                    }

                    if (m_caster->IsFriendlyTo(unitTarget))
                    {
                        Player* TargetMaster = unitTarget->GetCharmerOrOwnerPlayerOrPlayerItself();
                        Player* CasterMaster = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself();
                        if (CasterMaster && TargetMaster && TargetMaster->duel && CasterMaster != TargetMaster)
                            return;
                    }
                    
                    if (m_caster->IsFriendlyTo(unitTarget))
                        m_caster->CastSpell(unitTarget, heal, true, 0);
                    else
                        m_caster->CastSpell(unitTarget, hurt, true, 0);

                    return;
                }
                case 561:                                   // Judgement of command
                {
                    if (!unitTarget)
                        return;

                    uint32 spell_id = GetSpellEntry()->EffectBasePoints[i]+1;//m_currentBasePoints[i]+1;
                    SpellEntry const* spell_proto = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
                    if (!spell_proto)
                        return;

                    if (!unitTarget->HasUnitState(UNIT_STAT_STUNNED) && m_caster->GetTypeId()==TYPEID_PLAYER)
                    {
                        // decreased damage (/2) for non-stunned target.
                        SpellModifier *mod = new SpellModifier;
                        mod->op = SPELLMOD_DAMAGE;
                        mod->value = -50;
                        mod->type = SPELLMOD_PCT;
                        mod->spellId = GetSpellEntry()->Id;
                        mod->effectId = i;
                        mod->lastAffected = NULL;
                        mod->mask = 0x0000020000000000LL;
                        mod->charges = 0;

                        ((Player*)m_caster)->AddSpellMod(mod, true);
                        m_caster->CastSpell(unitTarget,spell_proto,true,NULL);
                                                            // mod deleted
                        ((Player*)m_caster)->AddSpellMod(mod, false);
                    }
                    else
                        m_caster->CastSpell(unitTarget,spell_proto,true,NULL);

                    return;
                }
            }

            switch (GetSpellEntry()->Id)
            {
                case 31789:                                 // Righteous Defense (step 1)
                {
                    // 31989 -> dummy effect (step 1) + dummy effect (step 2) -> 31709 (taunt like spell for each target)

                    // non-standard cast requirement check
                    if (!unitTarget || unitTarget->GetAttackers().empty())
                    {
                        // clear cooldown at fail
                        if (m_caster->GetTypeId() == TYPEID_PLAYER)
                            ((Player*) m_caster)->RemoveSpellCooldown(GetSpellEntry()->Id, true);

                        SendCastResult(SPELL_FAILED_TARGET_AFFECTING_COMBAT);
                        return;
                    }

                    // Righteous Defense (step 2) (in old version 31980 dummy effect)
                    // Clear targets for eff 1
                    for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
                    {
                        if (ihit->deleted)
                            continue;

                        ihit->effectMask &= ~(1<<1);
                    }

                    // not empty (checked)
                    Unit::AttackerSet attackers(unitTarget->GetAttackers());

                    // chance to be selected from list
                    uint32 maxTargets = std::min<uint32>(3, attackers.size());
                    for (uint32 i = 0; i < maxTargets; ++i)
                    {
                        Unit::AttackerSet::const_iterator aItr = attackers.begin();
                        std::advance(aItr, urand(0, attackers.size() - 1));
                        AddUnitTarget(*aItr, 1);
                        attackers.erase(*aItr);
                    }

                    // now let next effect cast spell at each target.
                    return;
                }
                case 37877:                                 // Blessing of Faith
                {
                    if (!unitTarget)
                        return;

                    uint32 spell_id = 0;
                    switch (unitTarget->GetClass())
                    {
                        case CLASS_DRUID:   spell_id = 37878; break;
                        case CLASS_PALADIN: spell_id = 37879; break;
                        case CLASS_PRIEST:  spell_id = 37880; break;
                        case CLASS_SHAMAN:  spell_id = 37881; break;
                        default: return;                    // ignore for not healing classes
                    }

                    m_caster->CastSpell(m_caster,spell_id,true);
                    return;
                }
            }
            break;
        case SPELLFAMILY_SHAMAN:

            // Flametongue Weapon Proc
            if (GetSpellEntry()->SpellFamilyFlags & 0x200000)
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                bool mainhand = false;

                if (m_CastItem && m_CastItem->GetSlot() == EQUIPMENT_SLOT_MAINHAND)
                    mainhand = true;

                bp = m_caster->GetAttackTime(mainhand ? BASE_ATTACK : OFF_ATTACK) * (GetSpellEntry()->EffectBasePoints[0]+1) / 100000;
                spell_id = 10444;
                break;
            }

            // Flametongue Totem Proc
            if (GetSpellEntry()->SpellFamilyFlags & 0x400000000)
            {
                bp = m_caster->GetAttackTime(BASE_ATTACK) * (GetSpellEntry()->EffectBasePoints[0]+1) / 100000;
                spell_id = 16368;
                break;
            }

            //Shaman Rockbiter Weapon
            if (GetSpellEntry()->SpellFamilyFlags == 0x400000)
            {
                uint32 spell_id = 0;
                switch (GetSpellEntry()->Id)
                {
                    case  8017: spell_id = 36494; break;    // Rank 1
                    case  8018: spell_id = 36750; break;    // Rank 2
                    case  8019: spell_id = 36755; break;    // Rank 3
                    case 10399: spell_id = 36759; break;    // Rank 4
                    case 16314: spell_id = 36763; break;    // Rank 5
                    case 16315: spell_id = 36766; break;    // Rank 6
                    case 16316: spell_id = 36771; break;    // Rank 7
                    case 25479: spell_id = 36775; break;    // Rank 8
                    case 25485: spell_id = 36499; break;    // Rank 9
                    default:
                        sLog.outLog(LOG_DEFAULT, "ERROR: Spell::EffectDummy: Spell %u not handled in RW",GetSpellEntry()->Id);
                        return;
                }

                SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);

                if (!spellInfo)
                {
                    sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: unknown spell id %i\n", spell_id);
                    return;
                }

                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                for (int j = BASE_ATTACK; j <= OFF_ATTACK; ++j)
                {
                    if (Item* item = ((Player*)m_caster)->GetWeaponForAttack(WeaponAttackType(j)))
                    {
                        if (item->IsFitToSpellRequirements(GetSpellEntry()))
                        {
                            Spell *spell = new Spell(m_caster, spellInfo, true);

                            // enchanting spell selected by calculated damage-per-sec in enchanting effect
                            // at calculation applied affect from Elemental Weapons talent
                            // real enchantment damage-1
                            spell->m_currentBasePoints[1] = damage-1;

                            SpellCastTargets targets;
                            targets.setItemTarget(item);
                            spell->prepare(&targets);
                        }
                    }
                }
                return;
            }

            if (GetSpellEntry()->Id == 39610)                    // Mana-Tide Totem effect
            {
                if (!unitTarget || unitTarget->getPowerType() != POWER_MANA)
                    return;

                // Regenerate 6% of Total Mana Every 3 secs
                int32 EffectBasePoints0 = unitTarget->GetMaxPower(POWER_MANA) * damage / 100;
                m_caster->CastCustomSpell(unitTarget,39609,&EffectBasePoints0,NULL,NULL,true,NULL,NULL,m_originalCasterGUID);
                return;
            }

            break;
    }

    //spells triggered by dummy effect should not miss
    if (spell_id)
    {
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);

        if (!spellInfo)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: EffectDummy of spell %u: triggering unknown spell id %i\n", GetSpellEntry()->Id, spell_id);
            return;
        }

        Spell* spell = new Spell(m_caster, spellInfo, true, m_originalCasterGUID, NULL, true);
        if (bp) spell->m_currentBasePoints[0] = bp;
        SpellCastTargets targets;
        targets.setUnitTarget(unitTarget);
        spell->prepare(&targets);
    }

    // pet auras
    if (PetAura const* petSpell = sSpellMgr.GetPetAura(GetSpellEntry()->Id))
    {
        m_caster->AddPetAura(petSpell);
        return;
    }

    // Script based implementation. Must be used only for not good for implementation in core spell effects
    // So called only for not proccessed cases
    if (gameObjTarget)
        sScriptMgr.OnEffectDummy(m_caster, GetSpellEntry()->Id, i, gameObjTarget);
    else if (unitTarget && unitTarget->GetTypeId() == TYPEID_UNIT)
        sScriptMgr.OnEffectDummy(m_caster, GetSpellEntry()->Id, i, (Creature*)unitTarget);
    else if (itemTarget)
        sScriptMgr.OnEffectDummy(m_caster, GetSpellEntry()->Id, i, itemTarget);
}

void Spell::EffectTriggerSpellWithValue(uint32 i)
{
    uint32 triggered_spell_id = GetSpellEntry()->EffectTriggerSpell[i];

    // normal case
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: EffectTriggerSpellWithValue of spell %u: triggering unknown spell id %i\n", GetSpellEntry()->Id,triggered_spell_id);
        return;
    }

    int32 bp = damage;
    m_caster->CastCustomSpell(unitTarget,triggered_spell_id,&bp,&bp,&bp,true,NULL,NULL,m_originalCasterGUID);
}

void Spell::EffectTriggerRitualOfSummoning(uint32 i)
{
    uint32 triggered_spell_id = GetSpellEntry()->EffectTriggerSpell[i];
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: EffectTriggerRitualOfSummoning of spell %u: triggering unknown spell id %i", GetSpellEntry()->Id,triggered_spell_id);
        return;
    }

    finish();
    Spell *spell = new Spell(m_caster, spellInfo, true);

    SpellCastTargets targets;
    targets.setUnitTarget(unitTarget);
    spell->prepare(&targets);

    m_caster->SetCurrentCastSpell(spell);
    spell->m_selfContainer = &(m_caster->m_currentSpells[spell->GetCurrentContainer()]);

}

void Spell::EffectForceCast(uint32 i)
{
    if (!unitTarget)
        return;

    uint32 triggered_spell_id = GetSpellEntry()->EffectTriggerSpell[i];

    // normal case
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: EffectForceCast of spell %u: triggering unknown spell id %i", GetSpellEntry()->Id,triggered_spell_id);
        return;
    }

    unitTarget->CastSpell((Unit*)NULL,spellInfo,true,NULL,NULL,m_originalCasterGUID);
}

void Spell::EffectTriggerSpell(uint32 i)
{
    uint32 triggered_spell_id = GetSpellEntry()->EffectTriggerSpell[i];

    // special cases
    switch (triggered_spell_id)
    {
        //Explosives with self-kill when triggered
        case 3617:
        {
            if (m_caster->GetTypeId() == TYPEID_UNIT)
                m_caster->Kill(m_caster, false);
        }
        // Vanish
        case 18461:
        {
            m_caster->RemoveMovementImpairingAuras();
            m_caster->RemoveSpellsCausingAura(SPELL_AURA_MOD_STALKED);

            // if this spell is given to NPC it must handle rest by it's own AI
            if (m_caster->GetTypeId() != TYPEID_PLAYER)
                return;

            // get highest rank of the Stealth spell
            bool found = false;
            SpellEntry const *spellInfo;
            const PlayerSpellMap& sp_list = ((Player*)m_caster)->GetSpellMap();
            for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
            {
                // only highest rank is shown in spell book, so simply check if shown in spell book
                if (!itr->second.active || itr->second.disabled || itr->second.state == PLAYERSPELL_REMOVED)
                    continue;

                spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                if (!spellInfo)
                    continue;

                if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_STEALTH)
                {
                    found=true;
                    break;
                }
            }

            // no Stealth spell found
            if (!found)
                return;

            // reset cooldown on it if needed
            if (((Player*)m_caster)->HasSpellCooldown(spellInfo->Id, spellInfo->Category))
                ((Player*)m_caster)->RemoveSpellCooldown(spellInfo->Id);

            AddTriggeredSpell(spellInfo);
            return;
        }
        // just skip
        case 23770:                                         // Sayge's Dark Fortune of *
        case 47531:                                         // Dismiss Pet handled elsewhere
        case 32186:                                         // Fire Elemental Totem not known effect
        case 32184:                                         // Earth Elemental Totem not known effect
            // not exist, common cooldown can be implemented in scripts if need.
            return;
        // Brittle Armor - (need add max stack of 24575 Brittle Armor)
        case 29284:
        {
            const SpellEntry *spell = sSpellTemplate.LookupEntry<SpellEntry>(24575);
            if (!spell)
                return;

            for (int j = 0; j < spell->StackAmount; ++j)
                m_caster->CastSpell(unitTarget,spell->Id, true, m_CastItem, NULL, m_originalCasterGUID);
            return;
        }
        // Mercurial Shield - (need add max stack of 26464 Mercurial Shield)
        case 29286:
        {
            const SpellEntry *spell = sSpellTemplate.LookupEntry<SpellEntry>(26464);
            if (!spell)
                return;

            m_caster->CastSpell(unitTarget,spell->Id, true, m_CastItem, NULL, m_originalCasterGUID);
            if (Aura* aur = m_caster->GetAura(spell->Id, 0))
            {
                aur->ApplyModifier(false, true);
                aur->SetStackAmount(spell->StackAmount);
                aur->ApplyModifier(true, true);
                aur->UpdateSlotCounterAndDuration();
            }
            return;
        }
        // Righteous Defense
        case 31980:
        {
            m_caster->CastSpell(unitTarget, 31790, true,m_CastItem,NULL,m_originalCasterGUID);
            return;
        }
        // Unstable Mushroom Primer
        case 35256:
            triggered_spell_id = 35362;
            break;
        // Cloak of Shadows
        case 35729 :
        {
            uint32 dispelMask = SpellMgr::GetDispellMask(DISPEL_ALL);
            Unit::AuraMap& Auras = m_caster->GetAuras();
            for (Unit::AuraMap::iterator iter = Auras.begin(); iter != Auras.end(); ++iter)
            {
                // remove all harmful spells on you...
                SpellEntry const* spell = iter->second->GetSpellProto();
                if (((spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC && spell->SchoolMask != SPELL_SCHOOL_MASK_NORMAL) // only affect magic spells
                    || ((1<<spell->Dispel) & dispelMask))
                    // ignore positive and passive auras
                    && !iter->second->IsPositive() && !iter->second->IsPassive())
                {
                    m_caster->RemoveAurasDueToSpell(spell->Id);
                    iter = Auras.begin();
                }
            }
            return;
        }
        // Priest Shadowfiend (34433) need apply mana gain trigger aura on pet
        case 41967:
        {
            if (Unit *pet = m_caster->GetPet())
            {
                pet->CastSpell(pet, 28305, true); // Mana Leech
                pet->CastSpell(pet, 54742, true); // bonus dodge 70% +5% basic dodge
                ((Pet*)pet)->AI()->AttackStart(m_caster->GetUnit(m_caster->GetSelection()));
            }
            return;
        }
        // Support for quest To Legion Hold
        case 37492:
        {
            std::list<Creature*> pList;
            Hellground::AllCreaturesOfEntryInRange u_check(m_caster, 21633, 70.0f);
            Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(pList, u_check);

            Cell::VisitAllObjects(m_caster, searcher, 70.0f);

            if (pList.size() == 0)
            {
                if (Creature * summon = m_caster->SummonCreature(21633, -3361, 2962, 170, 5.83, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 90000))
                    summon->setActive(true);
            }
            return;
        }
        // Desperate Defense (self root)
        case 33897:
        {
            m_caster->CastSpell(m_caster, 33356, true, NULL, NULL, m_originalCasterGUID);
            return;
        }
        // Electrical Storm
        case 43657:
        {
            int32 damage = 800;
            if (m_caster->HasAura(43648, 1))
            {
                uint32 tick = 0;
                if (Aura* Aur = m_caster->GetAura(43648, 1))
                {
                    tick = Aur->GetTickNumber();
                    // FIXME: disable last tick damage until we find easy way to remove from
                    // target list players within eye of the storm;
                    // If not in eye of the storm all should be dead before anyway ;]
                    if (tick == 8)
                        damage = 0;
                    else
                        damage = urand(800, 1200)*tick;
                }
            }
            m_caster->CastCustomSpell((Unit*)NULL, triggered_spell_id, &damage, NULL, NULL, true, m_CastItem, NULL, m_originalCasterGUID);
            return;
        }
        // Activate Crystal Ward
        case 44969:
            unitTarget = m_caster;
            break;
        // Self Repair
        case 44994:
            if(100*unitTarget->GetHealth()/unitTarget->GetMaxHealth() > 70)
                return;
            break;
        // Explosion from Flame Wreath (Shade of Aran)
        case 29950:
            m_caster->RemoveAurasDueToSpell(29947);
            return;
			// diemetradon tag kill credit
		case 37907:
		{
			Player* plr = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself();
			if (plr)
				plr->CastSpell(plr, 37907, true);
			return;
		}
		case 29989: // dip in a moonwell control robot cast on robot (find by entry)
			m_caster->CastSpell((Unit*)(NULL), 29989, true);
			return;
    }

    // normal case
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: EffectTriggerSpell of spell %u: triggering unknown spell id %i", GetSpellEntry()->Id,triggered_spell_id);
        return;
    }

    // some triggered spells require specific equipment
    if (spellInfo->EquippedItemClass >=0 && m_caster->GetTypeId()==TYPEID_PLAYER)
    {
        // main hand weapon required
        if (spellInfo->AttributesEx3 & SPELL_ATTR_EX3_MAIN_HAND)
        {
            Item* item = ((Player*)m_caster)->GetWeaponForAttack(BASE_ATTACK);

            // skip spell if no weapon in slot or broken
            if (!item || item->IsBroken())
                return;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(spellInfo))
                return;
        }

        // offhand hand weapon required
        if (spellInfo->AttributesEx3 & SPELL_ATTR_EX3_REQ_OFFHAND)
        {
            Item* item = ((Player*)m_caster)->GetWeaponForAttack(OFF_ATTACK);

            // skip spell if no weapon in slot or broken
            if (!item || item->IsBroken())
                return;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(spellInfo))
                return;
        }
    }

    // some triggered spells must be cast instantly (for example, if next effect case instant kill caster)
    /*bool instant = false;
    for (uint32 j = i+1; j < 3; ++j)
    {
        if (GetSpellEntry()->EffectImplicitTargetA[j] == TARGET_UNIT_CASTER
            && (GetSpellEntry()->Effect[j]==SPELL_EFFECT_INSTAKILL))
        {
            instant = true;
            break;
        }
    }
    */

    if (unitTarget)
        m_caster->CastSpell(unitTarget, spellInfo, true, m_CastItem, NULL, m_originalCasterGUID);
}

void Spell::EffectTriggerMissileSpell(uint32 effect_idx)
{
    uint32 triggered_spell_id = GetSpellEntry()->EffectTriggerSpell[effect_idx];

    // normal case
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: EffectTriggerMissileSpell of spell %u (eff: %u): triggering unknown spell id %u",
            GetSpellEntry()->Id,effect_idx,triggered_spell_id);
        return;
    }

    if (m_CastItem)
        debug_log("WORLD: cast Item spellId - %i", spellInfo->Id);

    // some triggered spells require specific equipment
    if (spellInfo->EquippedItemClass >=0 && m_caster->GetTypeId()==TYPEID_PLAYER)
    {
        // main hand weapon required
        if (spellInfo->AttributesEx3 & SPELL_ATTR_EX3_MAIN_HAND)
        {
            Item* item = ((Player*)m_caster)->GetWeaponForAttack(BASE_ATTACK);

            // skip spell if no weapon in slot or broken
            if (!item || item->IsBroken())
                return;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(spellInfo))
                return;
        }

        // offhand hand weapon required
        if (spellInfo->AttributesEx3 & SPELL_ATTR_EX3_REQ_OFFHAND)
        {
            Item* item = ((Player*)m_caster)->GetWeaponForAttack(OFF_ATTACK);

            // skip spell if no weapon in slot or broken
            if (!item || item->IsBroken())
                return;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(spellInfo))
                return;
        }
    }

    Spell *spell = new Spell(m_caster, spellInfo, true, m_originalCasterGUID);

    SpellCastTargets targets;

    if (!spellInfo->IsDestTargetEffect(effect_idx) ||
        triggered_spell_id == 44008)     // Static Disruption needs direct targeting
        targets.setUnitTarget(unitTarget);
    else
        targets.setDestination(m_targets.m_destX,m_targets.m_destY,m_targets.m_destZ);

    spell->m_CastItem = m_CastItem;
    spell->prepare(&targets, NULL);
}

void Spell::EffectTeleportUnits(uint32 i)
{
    if (!unitTarget || unitTarget->IsTaxiFlying())
        return;

    // If not exist data for dest location - return
    if (!m_targets.HasDst())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell::EffectTeleportUnits - does not have destination for spell ID %u", GetSpellEntry()->Id);
        return;
    }
    // Init dest coordinates
    int32 mapid = m_targets.m_mapId;
    if (mapid < 0)
        mapid = (int32)unitTarget->GetMapId();

    Position dest;
    dest.x = m_targets.m_destX;
    dest.y = m_targets.m_destY;
    dest.z = m_targets.m_destZ;
    dest.o = m_targets.m_orientation;

    if(dest.o < 0)
        dest.o = m_targets.getUnitTarget() ? m_targets.getUnitTarget()->GetOrientation() : unitTarget->GetOrientation();
    sLog.outDebug("Spell::EffectTeleportUnits - teleport unit to %u %f %f %f\n", mapid, dest.x, dest.y, dest.z);
    // Teleport
    if (mapid == unitTarget->GetMapId())
        unitTarget->NearTeleportTo(dest.x, dest.y, dest.z, dest.o, unitTarget == m_caster);
    else if (unitTarget->GetTypeId() == TYPEID_PLAYER)
        ((Player*)unitTarget)->TeleportTo(mapid, dest.x, dest.y, dest.z, dest.o, unitTarget == m_caster ? TELE_TO_SPELL : 0);

    // post effects for TARGET_DST_DB
    switch (GetSpellEntry()->Id)
    {
        // Dimensional Ripper - Everlook
        case 23442:
        {
          int32 r = irand(0, 119);
            if (r >= 70)                                  // 7/12 success
            {
                if (r < 100)                              // 4/12 evil twin
                    m_caster->CastSpell(m_caster,23445,true);
                else                                        // 1/12 fire
                    m_caster->CastSpell(m_caster,23449,true);
            }
            return;
        }
        // Ultrasafe Transporter: Toshley's Station
        case 36941:
        {
            if (roll_chance_i(50))                        // 50% success
            {
              int32 rand_eff = urand(1,7);
                switch (rand_eff)
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster,36900,true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster,36901,true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster,36895,true);
                        break;
                    case 4:
                        // Decrease the size
                        m_caster->CastSpell(m_caster,36893,true);
                        break;
                    case 5:
                    // Transform
                    {
                        if (((Player*)m_caster)->GetTeam() == ALLIANCE)
                            m_caster->CastSpell(m_caster,36897,true);
                        else
                            m_caster->CastSpell(m_caster,36899,true);
                        break;
                    }
                    case 6:
                        // chicken
                        m_caster->CastSpell(m_caster,36940,true);
                        break;
                    case 7:
                        // evil twin
                        m_caster->CastSpell(m_caster,23445,true);
                        break;
                }
            }
            return;
        }
        // Dimensional Ripper - Area 52
        case 36890:
        {
            if (roll_chance_i(50))                        // 50% success
            {
              int32 rand_eff = urand(1,4);
                switch (rand_eff)
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster,36900,true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster,36901,true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster,36895,true);
                        break;
                    case 4:
                    // Transform
                    {
                        if (((Player*)m_caster)->GetTeam() == ALLIANCE)
                            m_caster->CastSpell(m_caster,36897,true);
                        else
                            m_caster->CastSpell(m_caster,36899,true);
                        break;
                    }
                }
            }
            return;
        }
        // Teleport: Spectral Realm - also teleport pets
        case 46019:
        {
            if(unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;
            if(Pet* targets_pet = unitTarget->GetPet())
                targets_pet->CastSpell(targets_pet, 46019, true);
            return;
        }
    }
}

void Spell::EffectApplyAura(uint32 i)
{
    if (!unitTarget)
        return;

    // what the fuck is done here? o.O
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(GetSpellEntry()->Id);
    if (!spellInfo)
        return;

    SpellImmuneList const& list = unitTarget->m_spellImmune[IMMUNITY_STATE];
    for (SpellImmuneList::const_iterator itr = list.begin(); itr != list.end(); ++itr)
        if (itr->type == spellInfo->EffectApplyAuraName[i])
            return;

    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if ((!unitTarget->isAlive() || (unitTarget->GetTypeId() == TYPEID_PLAYER && ((Player*)unitTarget)->IsSpectator())) && !(spellInfo->Attributes & (SPELL_ATTR_PASSIVE | SPELL_ATTR_CASTABLE_WHILE_DEAD)) && !SpellMgr::IsDeathPersistentSpell(spellInfo) &&
        (unitTarget->GetTypeId()!=TYPEID_PLAYER || !((Player*)unitTarget)->GetSession()->PlayerLoading()))
        return;

    // hacky GCD for Black Hole Effect dummy aura
    if (spellInfo->Id == 46230 && unitTarget->HasAura(46230, 2))
    {
        if(Aura* aur = unitTarget->GetAura(46230, 2))
            if(aur->GetAuraDuration() >= 3400)
                return;
    }

    Unit* caster = m_originalCasterGUID ? m_originalCaster : m_caster;
    if (!caster)
        return;

    sLog.outDebug("Spell: Aura is: %u", spellInfo->EffectApplyAuraName[i]);

    Aura* Aur = CreateAura(spellInfo, i, &damage, unitTarget, caster, m_CastItem);

    // Now Reduce spell duration using data received at spell hit
    int32 duration = Aur->GetAuraMaxDuration();
    if (!SpellMgr::IsPositiveSpell(spellInfo->Id))
    {
        if (unitTarget != caster || !SpellMgr::IsChanneledSpell(spellInfo))
        {
            unitTarget->ApplyDiminishingToDuration(m_diminishGroup,duration,caster,m_diminishLevel, spellInfo);
            Aur->setDiminishGroup(m_diminishGroup);
        }
    }

    //mod duration of channeled aura by spell haste
    if (SpellMgr::IsChanneledSpell(spellInfo))
    {
        caster->ModSpellCastTime(spellInfo, duration, this);
        if (initial_channell || duration > m_timer.GetInterval()) // we need to send the longest spell that we have
            SendChannelStart(duration, false);
    }

    // if Aura removed and deleted, do not continue.
    if (duration== 0 && !(Aur->IsPermanent()))
    {
        delete Aur;
        return;
    }

    if (Aur->GetSpellProto()->Category == SPELL_CATEGORY_MORPH_SHIRT && caster->InClassForm())
    {
        delete Aur;
        return;
    }

    if (duration != Aur->GetAuraMaxDuration())
    {
        Aur->SetAuraMaxDuration(duration);
        Aur->SetAuraDuration(duration);
    }

    bool added = unitTarget->AddAura(Aur);

    // Aura not added and deleted in AddAura call;
    if (!added)
        return;

    // found crash at character loading, broken pointer to Aur...
    // Aur was deleted in AddAura()...
    if (!Aur)
        return;
    
    unitTarget->ApplyAdditionalSpell(caster, spellInfo);

    // Prayer of Mending (jump animation), we need formal caster instead original for correct animation
    if (spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && (spellInfo->SpellFamilyFlags & 0x00002000000000LL))
        m_caster->CastSpell(unitTarget, 41637, true, NULL, Aur, m_originalCasterGUID);
}

void Spell::EffectUnlearnSpecialization(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)unitTarget;
    uint32 spellToUnlearn = GetSpellEntry()->EffectTriggerSpell[i];

    _player->removeSpell(spellToUnlearn);

    sLog.outDebug("Spell: Player %u have unlearned spell %u from NpcGUID: %u", _player->GetGUIDLow(), spellToUnlearn, m_caster->GetGUIDLow());
}

void Spell::EffectPowerDrain(uint32 i)
{
    if (GetSpellEntry()->EffectMiscValue[i] < 0 || GetSpellEntry()->EffectMiscValue[i] >= MAX_POWERS)
        return;

    Powers drain_power = Powers(GetSpellEntry()->EffectMiscValue[i]);

    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;
    if (unitTarget->getPowerType() != drain_power)
        return;
    if (damage < 0)
        return;

    uint32 curPower = unitTarget->GetPower(drain_power);

    //add spell damage bonus
    damage=m_caster->SpellDamageBonus(unitTarget,GetSpellEntry(),uint32(damage),SPELL_DIRECT_DAMAGE);

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    uint32 power = damage;
    if (drain_power == POWER_MANA && unitTarget->GetTypeId() == TYPEID_PLAYER)
        power -= ((Player*)unitTarget)->GetSpellCritDamageReduction(power);

    int32 new_damage;
    if (curPower < power)
        new_damage = curPower;
    else
        new_damage = power;

    unitTarget->ModifyPower(drain_power,-new_damage);

    if (GetSpellEntry()->Id == 28734) // Mana Tap is a pure mana drain (mana is lost)
        return;

    if (drain_power == POWER_MANA)
    {
        float manaMultiplier = GetSpellEntry()->EffectMultipleValue[i];
        if (manaMultiplier==0)
            manaMultiplier = 1;

        if (Player *modOwner = m_caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetSpellEntry()->Id, SPELLMOD_MULTIPLE_VALUE, manaMultiplier);

        int32 gain = int32(new_damage*manaMultiplier);

        m_caster->ModifyPower(POWER_MANA,gain);
        //send log
        m_caster->SendEnergizeSpellLog(m_caster, sSpellMgr.GetSpellAnalog(GetSpellEntry()),gain,POWER_MANA);
    }
}

void Spell::EffectSendEvent(uint32 EffectIndex)
{
    switch (GetSpellEntry()->Id)
    {
        // Summon Arcane Elemental
        case 40134:
        {
            if (m_caster->GetTypeId() == TYPEID_UNIT)
            {
                if (Unit* Arcane = m_caster->SummonCreature(23100, -2469.59, 4700.71, 155.86, 3.15, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300000))
                    Arcane->setFaction(35);
            }
            break;
        }
        // Place Belmara's Tome
        case 34140:
        {
            if (m_caster->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Creature *pBelmara = m_caster->SummonCreature(19546, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 3.15, TEMPSUMMON_TIMED_DESPAWN, 10000))
            {
                pBelmara->setFaction(35);
                pBelmara->MonsterSay("I can't sleep without a good bedtime story. Now I'm cerain to rest well.", LANG_UNIVERSAL, 0);

                ((Player*)m_caster)->CastCreatureOrGO(19547, pBelmara->GetGUID(), GetSpellEntry()->Id);
            }
            break;
        }
        // Place Luminrath's Mantle
        case 34142:
        {
            if (m_caster->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Creature *pLuminrath = m_caster->SummonCreature(19580, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 3.15, TEMPSUMMON_TIMED_DESPAWN, 10000))
            {
                pLuminrath->setFaction(35);
                pLuminrath->MonsterSay("I can't possibly go out without my cloak. I hope it's in here...", LANG_UNIVERSAL, 0);

                ((Player*)m_caster)->CastCreatureOrGO(19548, pLuminrath->GetGUID(), GetSpellEntry()->Id);
            }
            break;
        }
        // Place Cohlien's Hat
        case 34144:
        {
            if (m_caster->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Creature *pCohlien = m_caster->SummonCreature(19579, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 3.15, TEMPSUMMON_TIMED_DESPAWN, 10000))
            {
                pCohlien->setFaction(35);
                pCohlien->MonsterSay("Phew! There's my lucky hat. I've been looking for it everywhere.", LANG_UNIVERSAL, 0);

                ((Player*)m_caster)->CastCreatureOrGO(19550, pCohlien->GetGUID(), GetSpellEntry()->Id);
            }
            break;
        }
        // Place Dathric's Blade
        case 34141:
        {
            if (m_caster->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Creature *pDathric = m_caster->SummonCreature(19543, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), 3.15, TEMPSUMMON_TIMED_DESPAWN, 10000))
            {
                pDathric->setFaction(35);
                pDathric->MonsterSay("I don't know what I was thinking, going out without my sword. I would've put it on if I'd seen it here...", LANG_UNIVERSAL, 0);

                ((Player*)m_caster)->CastCreatureOrGO(19549, pDathric->GetGUID(), GetSpellEntry()->Id);
            }
            break;
        }
    }

    if (m_caster->GetTypeId() == TYPEID_PLAYER && ((Player*)m_caster)->InBattleGroundOrArena())
    {
        BattleGround* bg = ((Player *)m_caster)->GetBattleGround();
        if (bg && bg->GetStatus() == STATUS_IN_PROGRESS)
        {
            switch (GetSpellEntry()->Id)
            {
                case 23333:                                 // Pickup Horde Flag
                    /*do not uncomment .
                    if (bg->GetTypeID()==BATTLEGROUND_WS)
                        bg->EventPlayerClickedOnFlag((Player*)m_caster, gameObjTarget);
                    sLog.outDebug("Send Event Horde Flag Picked Up");
                    break;
                    /* not used :
                    case 23334:                                 // Drop Horde Flag
                        if (bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerDroppedFlag((Player*)m_caster);
                        sLog.outDebug("Drop Horde Flag");
                        break;
                    */
                case 23335:                                 // Pickup Alliance Flag
                    /*do not uncomment ... (it will cause crash, because of null targetobject!) anyway this is a bad way to call that event, because it would cause recursion
                    if (bg->GetTypeID()==BATTLEGROUND_WS)
                        bg->EventPlayerClickedOnFlag((Player*)m_caster, gameObjTarget);
                    sLog.outDebug("Send Event Alliance Flag Picked Up");
                    break;
                    /* not used :
                    case 23336:                                 // Drop Alliance Flag
                        if (bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerDroppedFlag((Player*)m_caster);
                        sLog.outDebug("Drop Alliance Flag");
                        break;
                    case 23385:                                 // Alliance Flag Returns
                        if (bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerClickedOnFlag((Player*)m_caster, gameObjTarget);
                        sLog.outDebug("Alliance Flag Returned");
                        break;
                    case 23386:                                   // Horde Flag Returns
                        if (bg->GetTypeID()==BATTLEGROUND_WS)
                            bg->EventPlayerClickedOnFlag((Player*)m_caster, gameObjTarget);
                        sLog.outDebug("Horde Flag Returned");
                        break;*/
                case 34976:
                    /*
                    if (bg->GetTypeID()==BATTLEGROUND_EY)
                        bg->EventPlayerClickedOnFlag((Player*)m_caster, gameObjTarget);
                    */
                    break;
                default:
                    sLog.outDebug("Unknown spellid %u in BG event", GetSpellEntry()->Id);
                    break;
            }
        }
    }
    sLog.outDebug("Spell ScriptStart %u for spellid %u in EffectSendEvent ", GetSpellEntry()->EffectMiscValue[EffectIndex], GetSpellEntry()->Id);
    if (!sScriptMgr.OnProcessEvent(GetSpellEntry()->EffectMiscValue[EffectIndex], m_caster, focusObject, true))
        m_caster->GetMap()->ScriptsStart(sEventScripts, GetSpellEntry()->EffectMiscValue[EffectIndex], m_caster, focusObject);
}

void Spell::EffectPowerBurn(uint32 i)
{
    if (GetSpellEntry()->EffectMiscValue[i] < 0 || GetSpellEntry()->EffectMiscValue[i] >= MAX_POWERS)
        return;

    Powers powertype = Powers(GetSpellEntry()->EffectMiscValue[i]);

    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;
    if (unitTarget->getPowerType()!=powertype)
        return;
    if (damage < 0)
        return;

    int32 curPower = int32(unitTarget->GetPower(powertype));

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    uint32 power = damage;
    if (powertype == POWER_MANA && unitTarget->GetTypeId() == TYPEID_PLAYER)
        power -= ((Player*)unitTarget)->GetSpellCritDamageReduction(power);

    int32 new_damage = (curPower < power) ? curPower : power;

    unitTarget->ModifyPower(powertype,-new_damage);
    float multiplier = GetSpellEntry()->EffectMultipleValue[i];

    if (Player *modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(GetSpellEntry()->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

    new_damage = int32(new_damage*multiplier);
    //m_damage+=new_damage; should not apply spell bonus
    //TODO: no log
    //unitTarget->ModifyHealth(-new_damage);
    if (m_originalCaster)
        m_damage = new_damage;
        //m_originalCaster->DealDamage(unitTarget, new_damage);
}

void Spell::EffectHeal(uint32 /*i*/)
{
}

void Spell::SpellDamageHeal(uint32 /*i*/)
{
    if (unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

        // Skip if m_originalCaster not available
        if (!caster)
            return;

        int32 addhealth = damage;

        // Vessel of the Naaru (Vial of the Sunwell trinket)
        if (GetSpellEntry()->Id == 45064)
        {
            // Amount of heal - depends from stacked Holy Energy
            Aura* healAmount = m_caster->GetDummyAura(45062);
            if (healAmount)
            {
                addhealth = caster->SpellHealingBonus(GetSpellEntry(), healAmount->GetModifierValue(), HEAL, unitTarget);
                m_caster->RemoveAurasDueToSpell(45062);
            }
            else
                addhealth = 0;
        }
        // Swiftmend - consumes Regrowth or Rejuvenation
        else if (GetSpellEntry()->TargetAuraState == AURA_STATE_SWIFTMEND && unitTarget->HasAuraState(AURA_STATE_SWIFTMEND))
        {
            Unit::AuraList const& RejorRegr = unitTarget->GetAurasByType(SPELL_AURA_PERIODIC_HEAL);
            // find most short by duration
            Aura *targetAura = NULL;
            for (Unit::AuraList::const_iterator i = RejorRegr.begin(); i != RejorRegr.end(); ++i)
            {
                if ((*i)->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID
                    && ((*i)->GetSpellProto()->SpellFamilyFlags == 0x40 || (*i)->GetSpellProto()->SpellFamilyFlags == 0x10))
                {
                    if (!targetAura || (*i)->GetAuraDuration() < targetAura->GetAuraDuration())
                        targetAura = *i;
                }
            }

            if (!targetAura)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Target(GUID:%llu) has aurastate AURA_STATE_SWIFTMEND but no matching aura.", unitTarget->GetGUID());
                return;
            }

            int32 tickheal = targetAura->GetModifierValuePerStack();
            if (Unit* auraCaster = targetAura->GetCaster())
                tickheal = auraCaster->SpellHealingBonus(targetAura->GetSpellProto(), tickheal, DOT, unitTarget);
            //int32 tickheal = targetAura->GetSpellProto()->EffectBasePoints[idx] + 1;
            //It is said that talent bonus should not be included
            //int32 tickheal = targetAura->GetModifierValue();
            int32 tickcount = 0;
            if (targetAura->GetSpellProto()->SpellFamilyName == SPELLFAMILY_DRUID)
            {
                switch (targetAura->GetSpellProto()->SpellFamilyFlags)//TODO: proper spellfamily for 3.0.x
                {
                    case 0x10:  tickcount = 4;  break; // Rejuvenation
                    case 0x40:  tickcount = 6;  break; // Regrowth
                }
            }
            addhealth += tickheal * tickcount;
            unitTarget->RemoveAurasByCasterSpell(targetAura->GetId(), targetAura->GetCasterGUID());

            //addhealth += tickheal * tickcount;
            //addhealth = caster->SpellHealingBonus(GetSpellEntry(), addhealth,HEAL, unitTarget);
        }
        else
            addhealth = caster->SpellHealingBonus(GetSpellEntry(), addhealth,HEAL, unitTarget);

        m_damage -= addhealth;
    }
}

void Spell::EffectHealPct(uint32 /*i*/)
{
    if (unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

        // Skip if m_originalCaster not available
        if (!caster)
            return;

        uint32 addhealth = unitTarget->GetMaxHealth() * damage / 100;
        caster->SendHealSpellLog(unitTarget, sSpellMgr.GetSpellAnalog(GetSpellEntry()), addhealth, false);

        int32 gain = unitTarget->ModifyHealth(int32(addhealth));
        unitTarget->getHostileRefManager().threatAssist(m_caster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(m_spellInfo), GetSpellEntry());

        if (caster->GetTypeId() == TYPEID_PLAYER)
        {
            if (BattleGround *bg = ((Player*)caster)->GetBattleGround())
                bg->UpdatePlayerScore(((Player*)caster), SCORE_HEALING_DONE, gain);

            if (caster->GetMap() && caster->GetMap()->IsDungeon() && ((InstanceMap*)caster->GetMap())->GetInstanceData())
                ((InstanceMap*)caster->GetMap())->GetInstanceData()->OnPlayerHealDamage(caster->ToPlayer(), gain);
        }
    }
}

void Spell::EffectHealMechanical(uint32 /*i*/)
{
    // Mechanic creature type should be correctly checked by targetCreatureType field
    if (unitTarget && unitTarget->isAlive() && damage >= 0)
    {
        // Try to get original caster
        Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

        // Skip if m_originalCaster not available
        if (!caster)
            return;

        uint32 addhealth = caster->SpellHealingBonus(GetSpellEntry(), uint32(damage), HEAL, unitTarget);
        caster->SendHealSpellLog(unitTarget, sSpellMgr.GetSpellAnalog(GetSpellEntry()), addhealth, false);
        unitTarget->ModifyHealth(int32(damage));
    }
}

void Spell::EffectHealthLeech(uint32 i)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    if (damage < 0)
        return;

    sLog.outDebug("HealthLeech :%i", damage);

    float multiplier = GetSpellEntry()->EffectMultipleValue[i];
    multiplier = multiplier ? multiplier : 1.0;

    if (Player *modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(GetSpellEntry()->Id, SPELLMOD_MULTIPLE_VALUE, multiplier);

    int32 new_damage = 0;
    uint32 curHealth = unitTarget->GetHealth();
    new_damage = m_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellEntry()->Id, damage, m_IsTriggeredSpell, true);
    if (curHealth < new_damage)
        new_damage = curHealth;

    // multipier only affects gains of HP!
    new_damage = int32(new_damage*multiplier);

    if (m_caster->isAlive())
    {
        new_damage = m_caster->SpellHealingBonus(GetSpellEntry(), new_damage, HEAL, m_caster);

        int32 gain = m_caster->ModifyHealth(new_damage);
        m_caster->getHostileRefManager().threatAssist(m_caster, gain * 0.5f * sSpellMgr.GetSpellThreatMultiplier(m_spellInfo), GetSpellEntry());

        m_caster->SendHealSpellLog(m_caster, sSpellMgr.GetSpellAnalog(GetSpellEntry()), uint32(new_damage));
    }
//    m_healthLeech+=tmpvalue;
//    m_damage+=new_damage;
}

void Spell::DoCreateItem(uint32 i, uint32 itemtype)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)unitTarget;

    uint32 newitemid = itemtype;
    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(newitemid);
    if (!pProto)
    {
        player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL);
        return;
    }

    uint32 num_to_add;

    // TODO: maybe all this can be replaced by using correct calculated `damage` value
    if (pProto->Class != ITEM_CLASS_CONSUMABLE || GetSpellEntry()->SpellFamilyName != SPELLFAMILY_MAGE)
    {
        num_to_add = damage;
        /*int32 basePoints = m_currentBasePoints[i];
        int32 randomPoints = GetSpellEntry()->EffectDieSides[i];
        if (randomPoints)
            num_to_add = basePoints + irand(1, randomPoints);
        else
            num_to_add = basePoints + 1;*/
    }
    else if (pProto->MaxCount == 1)
        num_to_add = 1;
    else if (player->GetLevel() >= GetSpellEntry()->spellLevel)
    {
        num_to_add = damage;
        /*int32 basePoints = m_currentBasePoints[i];
        float pointPerLevel = GetSpellEntry()->EffectRealPointsPerLevel[i];
        num_to_add = basePoints + 1 + uint32((player->GetLevel() - GetSpellEntry()->spellLevel)*pointPerLevel);*/
    }
    else
        num_to_add = 2;

    if (num_to_add < 1)
        num_to_add = 1;
    if (num_to_add > pProto->Stackable)
        num_to_add = pProto->Stackable;

    // init items_count to 1, since 1 item will be created regardless of specialization
    int items_count=1;
    // the chance to create additional items
    float additionalCreateChance=0.0f;
    // the maximum number of created additional items
    uint8 additionalMaxNum=0;
    // get the chance and maximum number for creating extra items

    if (canCreateExtraItems(player, GetSpellEntry()->Id, additionalCreateChance, additionalMaxNum))
    {
        // roll with this chance till we roll not to create or we create the max num
        while (roll_chance_f(additionalCreateChance) && items_count<=additionalMaxNum)
            ++items_count;
    }

    // really will be created more items
    num_to_add *= items_count;
    
    //// increase prof rate for x100
    //if (sWorld.isEasyRealm())
    //{
    //    switch (GetSpellEntry()->Id)
    //    {
    //    case 11479:
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
    //    case 28028:
    //    case 28027:
    //        num_to_add *= 2;
    //        break;
    //    case 36686:
    //    case 29688:
    //    case 32765:
    //    case 32766:
    //    case 26751:
    //    case 31373:
    //        num_to_add *= 2;
    //        break;
    //    }
    //}

    // can the player store the new item?
    ItemPosCountVec dest;
    uint32 no_space = 0;
    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, newitemid, num_to_add, &no_space);
    if (msg != EQUIP_ERR_OK)
    {
        // convert to possible store amount
        if (msg == EQUIP_ERR_INVENTORY_FULL || msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
            num_to_add -= no_space;
        else
        {
            // if not created by another reason from full inventory or unique items amount limitation
            player->SendEquipError(msg, NULL, NULL);
            return;
        }
    }

    if (num_to_add)
    {
        // create the new item and store it
        Item* pItem = player->StoreNewItem(dest, newitemid, true, Item::GenerateItemRandomPropertyId(newitemid), "SPELL_CREATE_ITEM");

        // was it successful? return error if not
        if (!pItem)
        {
            player->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL);
            return;
        }

        // set the "Crafted by ..." property of the item
        if (pItem->GetProto()->Class != ITEM_CLASS_CONSUMABLE && pItem->GetProto()->Class != ITEM_CLASS_QUEST)
            pItem->SetUInt32Value(ITEM_FIELD_CREATOR,player->GetGUIDLow());

        // send info to the client
        if (pItem)
            player->SendNewItem(pItem, num_to_add, true, true);

        // we succeeded in creating at least one item, so a levelup is possible
        player->UpdateCraftSkill(GetSpellEntry()->Id);
    }
}

void Spell::EffectCreateItem(uint32 i)
{
    DoCreateItem(i,GetSpellEntry()->EffectItemType[i]);
}

void Spell::EffectPersistentAA(uint32 i)
{
   float radius = SpellMgr::GetSpellRadius(GetSpellEntry(),i,false);
    if (Player* modOwner = m_originalCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(GetSpellEntry()->Id, SPELLMOD_RADIUS, radius);

    Unit *caster = m_caster->GetEntry() == WORLD_TRIGGER ? m_originalCaster : m_caster;
    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());
    if (Player* modOwner = m_originalCaster->GetSpellModOwner())
        modOwner->ApplySpellMod(GetSpellEntry()->Id, SPELLMOD_DURATION, duration);
    DynamicObject* dynObj = new DynamicObject;
    if (!dynObj->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), caster, GetSpellEntry()->Id, i, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, duration, radius))
    {
        delete dynObj;
        return;
    }
    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    //dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x01eeeeee);
    caster->AddDynObject(dynObj);
    dynObj->GetMap()->Add(dynObj);
}

void Spell::EffectEnergize(uint32 i)
{
    if (!unitTarget || !m_caster)
        return;

    if (!unitTarget->IsInWorld() || !m_caster->IsInWorld())
        return;

    if (!m_caster->IsInMap(unitTarget))
        return;

    if (!unitTarget->isAlive())
        return;

    if (GetSpellEntry()->EffectMiscValue[i] < 0 || GetSpellEntry()->EffectMiscValue[i] >= MAX_POWERS)
        return;

    // Don't energize targets with other power type
	// druids should regain mana
    //if (unitTarget->getPowerType() != GetSpellEntry()->EffectMiscValue[i])
    //    return;

    //Serpent Coil Braid
    if (GetSpellEntry()->SpellFamilyName == SPELLFAMILY_MAGE && GetSpellEntry()->SpellFamilyFlags == 0x10000000000LL)
        if (unitTarget->HasAura(37447, 0))
            unitTarget->CastSpell(unitTarget,37445,true);

    // Alchemist Stone
    if (GetSpellEntry()->SpellFamilyName == SPELLFAMILY_POTION)
        if (Aura *aura = unitTarget->GetAura(17619, 0))
        {
            int32 bp = damage * 4 / 10;
            unitTarget->CastCustomSpell(unitTarget,21400,&bp,NULL,NULL,true,NULL,aura);
        }

    // Some level depends spells
    int multiplier = 0;
    int level_diff = 0;
    switch (GetSpellEntry()->Id)
    {
        // Restore Energy
        case 9512:
            level_diff = m_caster->GetLevel() - 40;
            multiplier = 2;
            break;
        // Blood Fury
        case 24571:
            level_diff = m_caster->GetLevel() - 60;
            multiplier = 10;
            break;
        // Burst of Energy
        case 24532:
            level_diff = m_caster->GetLevel() - 60;
            multiplier = 4;
            break;
        default:
            break;
    }

    if (level_diff > 0)
        damage -= multiplier * level_diff;

    Powers power = Powers(GetSpellEntry()->EffectMiscValue[i]);

    if (unitTarget->GetMaxPower(power) == 0)
        return;

    int32 gain = unitTarget->ModifyPower(power,damage);

    //No threat from life tap
    if (GetSpellEntry()->Id != 31818)
        unitTarget->getHostileRefManager().threatAssist(m_caster, float(gain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(m_spellInfo), GetSpellEntry());

    m_caster->SendEnergizeSpellLog(unitTarget, sSpellMgr.GetSpellAnalog(GetSpellEntry()), damage, power);

    // Mad Alchemist's Potion
    if (GetSpellEntry()->Id == 45051)
    {
        // find elixirs on target
        uint32 elixir_mask = 0;
        Unit::AuraMap& Auras = unitTarget->GetAuras();
        for (Unit::AuraMap::iterator itr = Auras.begin(); itr != Auras.end(); ++itr)
        {
            uint32 spell_id = itr->second->GetId();
            if (uint32 mask = sSpellMgr.GetSpellElixirMask(spell_id))
                elixir_mask |= mask;
        }

        // get available elixir mask any not active type from battle/guardian (and flask if no any)
        elixir_mask = (elixir_mask & ELIXIR_FLASK_MASK) ^ ELIXIR_FLASK_MASK;

        // get all available elixirs by mask and spell level
        std::vector<uint32> elixirs;
        SpellElixirMap const& m_spellElixirs = sSpellMgr.GetSpellElixirMap();
        for (SpellElixirMap::const_iterator itr = m_spellElixirs.begin(); itr != m_spellElixirs.end(); ++itr)
        {
            if (itr->second & elixir_mask)
            {
                if (itr->second & (ELIXIR_UNSTABLE_MASK | ELIXIR_SHATTRATH_MASK))
                    continue;

                SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                if (spellInfo && (spellInfo->spellLevel < GetSpellEntry()->spellLevel || spellInfo->spellLevel > unitTarget->GetLevel()))
                    continue;

                elixirs.push_back(itr->first);
            }
        }

        if (!elixirs.empty())
        {
            // cast random elixir on target
            uint32 rand_spell = urand(0,elixirs.size()-1);
            m_caster->CastSpell(unitTarget,elixirs[rand_spell],true,m_CastItem);
        }
    }
}

void Spell::EffectEnergisePct(uint32 i)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    if (GetSpellEntry()->EffectMiscValue[i] < 0 || GetSpellEntry()->EffectMiscValue[i] >= MAX_POWERS)
        return;

    Powers power = Powers(GetSpellEntry()->EffectMiscValue[i]);

    uint32 maxPower = unitTarget->GetMaxPower(power);
    if (maxPower == 0)
        return;

    uint32 gain = damage * maxPower / 100;
    int32 realGain = unitTarget->ModifyPower(power, gain);
    unitTarget->getHostileRefManager().threatAssist(m_caster, float(realGain) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(m_spellInfo), GetSpellEntry());
    m_caster->SendEnergizeSpellLog(unitTarget, sSpellMgr.GetSpellAnalog(GetSpellEntry()), realGain, power);
}


void Spell::EffectOpenLock(uint32 effIndex)
{
    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        sLog.outDebug("WORLD: Open Lock - No Player Caster!");
        return;
    }

    Player* player = (Player*)m_caster;

    uint32 lockId = 0;
    uint64 guid = 0;

    // Get lockId
    if (gameObjTarget)
    {
        GameObjectInfo const* goInfo = gameObjTarget->GetGOInfo();
        // Arathi Basin banner opening !
        if (goInfo->type == GAMEOBJECT_TYPE_BUTTON && goInfo->button.noDamageImmune ||
            goInfo->type == GAMEOBJECT_TYPE_GOOBER && goInfo->goober.losOK)
        {
            //isAllowUseBattleGroundObject() already called in CheckCast()
            // in battleground check
            if (BattleGround *bg = player->GetBattleGround())
            {
                // check if it's correct bg
                if (bg->GetTypeID() == BATTLEGROUND_AB || bg->GetTypeID() == BATTLEGROUND_AV)
                    bg->EventPlayerClickedOnFlag(player, gameObjTarget);
                return;
            }
        }
        else if (goInfo->type == GAMEOBJECT_TYPE_FLAGSTAND)
        {
            //isAllowUseBattleGroundObject() already called in CheckCast()
            // in battleground check
            if (BattleGround *bg = player->GetBattleGround())
            {
                if (bg->GetTypeID() == BATTLEGROUND_EY)
                    bg->EventPlayerClickedOnFlag(player, gameObjTarget);
                return;
            }
        }
        // handle outdoor pvp object opening, return true if go was registered for handling
        // these objects must have been spawned by outdoorpvp!
        else if (gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_GOOBER && sOutdoorPvPMgr.HandleOpenGo(player, gameObjTarget->GetGUID()))
            return;
        lockId = gameObjTarget->GetLockId();
        guid = gameObjTarget->GetGUID();
    }
    else if (itemTarget)
    {
        lockId = itemTarget->GetProto()->LockID;
        guid = itemTarget->GetGUID();
    }
    else
    {
        sLog.outDebug("WORLD: Open Lock - No GameObject/Item Target!");
        return;
    }

    SkillType skillId = SKILL_NONE;
    int32 reqSkillValue = 0;
    int32 skillValue;

    SpellCastResult res = CanOpenLock(effIndex, lockId, skillId, reqSkillValue, skillValue);
    if (res != SPELL_CAST_OK)
    {
        SendCastResult(res);
        return;
    }

    if (gameObjTarget)
        gameObjTarget->Use(m_caster);
    else
        player->SendLoot(guid, LOOT_SKINNING, m_CastItem ? m_CastItem->GetEntry() : 0);

    // not allow use skill grow at item base open
    if(!m_CastItem && skillId != SKILL_NONE)
    {
        // update skill if really known
        if (uint32 pureSkillValue = player->GetPureSkillValue(skillId))
        {
            if (gameObjTarget)
            {
                // Allow one skill-up until respawned
                if (!gameObjTarget->IsInSkillupList(player->GetGUIDLow()) &&
                    player->UpdateGatherSkill(skillId, pureSkillValue, reqSkillValue))
                    gameObjTarget->AddToSkillupList(player->GetGUIDLow());
            }
            else if (itemTarget)
            {
                // Do one skill-up
                player->UpdateGatherSkill(skillId, pureSkillValue, reqSkillValue);
            }
        }
    }
}

void Spell::EffectSummonChangeItem(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = (Player*)m_caster;

    // applied only to using item
    if (!m_CastItem)
        return;

    // ... only to item in own inventory/bank/equip_slot
    if (m_CastItem->GetOwnerGUID()!=player->GetGUID())
        return;

    uint32 newitemid = GetSpellEntry()->EffectItemType[i];
    if (!newitemid)
        return;

    uint16 pos = m_CastItem->GetPos();

    Item *pNewItem = Item::CreateItem(newitemid, 1, player);
    if (!pNewItem)
        return;

    for (uint8 j = PERM_ENCHANTMENT_SLOT; j <= TEMP_ENCHANTMENT_SLOT; ++j)
    {
        if (m_CastItem->GetEnchantmentId(EnchantmentSlot(j)))
            pNewItem->SetEnchantment(EnchantmentSlot(j), m_CastItem->GetEnchantmentId(EnchantmentSlot(j)), m_CastItem->GetEnchantmentDuration(EnchantmentSlot(j)), m_CastItem->GetEnchantmentCharges(EnchantmentSlot(j)));
    }

    if (m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) < m_CastItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY))
    {
        double loosePercent = 1 - m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) / double(m_CastItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY));
        player->DurabilityLoss(pNewItem, loosePercent);
    }

    if (player->IsInventoryPos(pos))
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true);
        if (msg == EQUIP_ERR_OK)
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true, "SUMMON_CHANGE_DESTROY");

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if (m_CastItem==m_targets.getItemTarget())
                m_targets.setItemTarget(NULL);

            m_CastItem = NULL;

            player->StoreItem(dest, pNewItem, true);
            player->ItemAddedQuestCheck(newitemid, 1);
            return;
        }
    }
    else if (player->IsBankPos (pos))
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanBankItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true);
        if (msg == EQUIP_ERR_OK)
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true, "SUMMON_CHANGE_DESTROY");

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if (m_CastItem==m_targets.getItemTarget())
                m_targets.setItemTarget(NULL);

            m_CastItem = NULL;

            player->BankItem(dest, pNewItem, true);
            return;
        }
    }
    else if (player->IsEquipmentPos (pos))
    {
        uint16 dest;
        uint8 msg = player->CanEquipItem(m_CastItem->GetSlot(), dest, pNewItem, true);
        if (msg == EQUIP_ERR_OK)
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true, "SUMMON_CHANGE_DESTROY");

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if (m_CastItem==m_targets.getItemTarget())
                m_targets.setItemTarget(NULL);

            m_CastItem = NULL;

            player->EquipItem(dest, pNewItem, true);
            player->AutoUnequipOffhandIfNeed();
            return;
        }
    }

    // fail
    delete pNewItem;
}

void Spell::EffectOpenSecretSafe(uint32 i)
{
    EffectOpenLock(i);                                      //no difference for now
}

void Spell::EffectProficiency(uint32 /*i*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    Player *p_target = (Player*)unitTarget;

    uint32 subClassMask = GetSpellEntry()->EquippedItemSubClassMask;
    if (GetSpellEntry()->EquippedItemClass == 2 && !(p_target->GetWeaponProficiency() & subClassMask))
    {
        p_target->AddWeaponProficiency(subClassMask);
        p_target->SendProficiency(uint8(0x02),p_target->GetWeaponProficiency());
    }
    if (GetSpellEntry()->EquippedItemClass == 4 && !(p_target->GetArmorProficiency() & subClassMask))
    {
        p_target->AddArmorProficiency(subClassMask);
        p_target->SendProficiency(uint8(0x04),p_target->GetArmorProficiency());
    }
}

void Spell::EffectApplyAreaAura(uint32 i)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    AreaAura* Aur = new AreaAura(GetSpellEntry(), i, &damage, unitTarget, m_caster, m_CastItem);
    unitTarget->AddAura(Aur);
}

void Spell::EffectSummonType(uint32 i)
{
    switch (GetSpellEntry()->EffectMiscValueB[i])
    {
        case SUMMON_TYPE_GUARDIAN:
            EffectSummonGuardian(i);
            break;
        case SUMMON_TYPE_POSESSED:
        //case SUMMON_TYPE_POSESSED2:
        //case SUMMON_TYPE_POSESSED3:
            EffectSummonPossessed(i);
            break;
        case SUMMON_TYPE_WILD:
            DoEffectSummonWild(i);
            break;
        case SUMMON_TYPE_DEMON:
            EffectSummonDemon(i);
            break;
        case SUMMON_TYPE_SUMMON:
            EffectSummon(i);
            break;
        case SUMMON_TYPE_CRITTER:
        case SUMMON_TYPE_CRITTER2:
        case SUMMON_TYPE_CRITTER3:
            EffectSummonCritter(i);
            break;
        case SUMMON_TYPE_TOTEM_SLOT1:
        case SUMMON_TYPE_TOTEM_SLOT2:
        case SUMMON_TYPE_TOTEM_SLOT3:
        case SUMMON_TYPE_TOTEM_SLOT4:
        case SUMMON_TYPE_TOTEM:
            EffectSummonTotem(i);
            break;
        case SUMMON_TYPE_UNKNOWN1:
        case SUMMON_TYPE_UNKNOWN3:
        case SUMMON_TYPE_UNKNOWN4:
        case SUMMON_TYPE_UNKNOWN5:
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: EffectSummonType: Unhandled summon type %u", GetSpellEntry()->EffectMiscValueB[i]);
            break;
    }
}

void Spell::EffectSummon(uint32 i)
{
    uint32 pet_entry = GetSpellEntry()->EffectMiscValue[i];
    if (!pet_entry)
        return;

    if (!m_originalCaster || m_originalCaster->GetTypeId() != TYPEID_PLAYER)
    {
        DoEffectSummonWild(i, true);
        return;
    }

    Player *owner = (Player*)m_originalCaster;

    if (owner->GetPetGUID())
        return;

    // Summon in dest location
    float x,y,z;
    if (m_targets.HasDst())
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
    }
    else
        m_caster->GetNearPoint(x,y,z,owner->GetObjectSize());

    Pet *spawnCreature = owner->SummonPet(pet_entry, x, y, z, m_caster->GetOrientation(), SUMMON_PET, SpellMgr::GetSpellDuration(GetSpellEntry()));
    if (!spawnCreature)
        return;

    spawnCreature->SetUInt32Value(UNIT_NPC_FLAGS, 0);
    spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    spawnCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellEntry()->Id);

    std::string name = owner->GetName();
    name.append(petTypeSuffix[spawnCreature->getPetType()]);
    spawnCreature->SetName(name);

    spawnCreature->SetReactState(REACT_DEFENSIVE);
}

void Spell::EffectLearnSpell(uint32 i)
{
    if (!unitTarget)
        return;

    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
            EffectLearnPetSpell(i);

        return;
    }

    Player *player = (Player*)unitTarget;

    uint32 spellToLearn = (GetSpellEntry()->Id==SPELL_ID_GENERIC_LEARN) ? damage : GetSpellEntry()->EffectTriggerSpell[i];
    player->learnSpell(spellToLearn);

    sLog.outDebug("Spell: Player %u have learned spell %u from NpcGUID=%u", player->GetGUIDLow(), spellToLearn, m_caster->GetGUIDLow());
}

void Spell::EffectDispel(uint32 i)
{
    if (!unitTarget)
        return;

    if (unitTarget->IsHostileTo(m_caster))
    {
        // make this additional immunity check here for mass dispel
        if (unitTarget->IsImmunedToDamage(SpellMgr::GetSpellSchoolMask(GetSpellEntry())))
            return;
    }

    // Fill possible dispel list
    std::map <std::pair<uint32, uint64>, std::pair<Aura* , uint8>> dispel_map;

    // Create dispel mask by dispel type
    uint32 dispel_type = GetSpellEntry()->EffectMiscValue[i];
    uint32 dispelMask  = SpellMgr::GetDispellMask(DispelType(dispel_type));
    Unit::AuraMap const& auras = unitTarget->GetAuras();
    uint16 arr_count = 0;
    for (Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura *aur = (*itr).second;
        if (aur && (1<<aur->GetSpellProto()->Dispel) & dispelMask && !aur->IsPassive()/*there are some spells with Dispel that are passive*/)
        {
            if (aur->GetSpellProto()->Dispel == DISPEL_MAGIC)
            {
                // do not remove positive auras if friendly target
                //               negative auras if non-friendly target
                if (aur->IsPositive() == unitTarget->IsFriendlyTo(m_caster))
                    continue;
            }
            std::pair<uint32, uint64> index = std::make_pair(aur->GetId(), aur->GetCasterGUID());
            if (dispel_map.find(index) == dispel_map.end())
            {
                uint8 stack = aur->GetStackAmount();
                dispel_map[index] = std::make_pair(aur, stack);
                arr_count += stack;
            }
        }
    }

    if (arr_count)
    {
        Aura** dispel_list = new Aura*[arr_count];
        arr_count = 0;
        for (auto i = dispel_map.begin(); i != dispel_map.end(); ++i)
        {
            uint8 s = i->second.second;
            for (auto j = 0; j != s; ++j)
            {
                dispel_list[arr_count] = i->second.first;
                ++arr_count;
            }
        }

        std::list < std::pair<uint32,uint64> > success_list;// (spell_id,casterGuid)
        std::list < uint32 > fail_list;                     // spell_id;
        // dispel N = damage buffs (or while exist buffs for dispel)
        for (int32 count=0; count < damage && arr_count > 0; ++count)
        {
            uint16 rand = urand(0, arr_count-1);
            // Random select buff for dispel
            Aura *aur = dispel_list[rand];

            // Base dispel chance
            // TODO: possible chance depend from spell level??
            int32 miss_chance = 0;
            // Apply dispel mod from aura caster
            if (Unit *caster = aur->GetCaster())
            {
                if (Player* modOwner = caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(aur->GetId(), SPELLMOD_RESIST_DISPEL_CHANCE, miss_chance, this);
            }

            if (miss_chance < 100)
            {
                // Try dispel
                if (roll_chance_i(miss_chance))
                    fail_list.push_back(aur->GetId());
                else
                    success_list.push_back(std::pair<uint32,uint64>(aur->GetId(),aur->GetCasterGUID()));
            }
            // don't try to dispell effects with 100% dispell resistance (patch 2.4.3 notes)
            else
                count--;

            // remove used aura from access
            std::swap(dispel_list[rand], dispel_list[arr_count-1]);
            --arr_count;
        }

        // Send success log and really remove auras
        if (!success_list.empty())
        {
            int32 count = success_list.size();
            WorldPacket data(SMSG_SPELLDISPELLOG, 8+8+4+1+4+count*5);
            data << unitTarget->GetPackGUID();              // Victim GUID
            data << m_caster->GetPackGUID();                // Caster GUID
            data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));                // dispel spell id
            data << uint8(0);                               // not used
            data << uint32(count);                          // count
            for (std::list<std::pair<uint32,uint64> >::iterator j = success_list.begin(); j != success_list.end(); ++j)
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(j->first);
                data << uint32(sSpellMgr.GetSpellAnalog(spellInfo));              // Spell Id
                data << uint8(0);                           // 0 - dispelled !=0 cleansed
                if (spellInfo->StackAmount!= 0)
                    unitTarget->RemoveAuraFromStackByDispel(spellInfo->Id, j->second);
                else
                    unitTarget->RemoveAurasDueToSpellByDispel(spellInfo->Id, j->second, m_caster);
             }
            m_caster->BroadcastPacket(&data, true);

            // On succes dispel
            // Devour Magic
            if (GetSpellEntry()->SpellFamilyName == SPELLFAMILY_WARLOCK && GetSpellEntry()->Category == 12)
            {
                uint32 heal_spell = 0;
                switch (GetSpellEntry()->Id)
                {
                    case 19505: heal_spell = 19658; break;
                    case 19731: heal_spell = 19732; break;
                    case 19734: heal_spell = 19733; break;
                    case 19736: heal_spell = 19735; break;
                    case 27276: heal_spell = 27278; break;
                    case 27277: heal_spell = 27279; break;
                    default:
                        sLog.outDebug("Spell for Devour Magic %d not handled in Spell::EffectDispel", GetSpellEntry()->Id);
                        break;
                }
                if (heal_spell)
                    m_caster->CastSpell(m_caster, heal_spell, true);
            }
        }
        // Send fail log to client
        if (!fail_list.empty())
        {
            // Failed to dispell
            WorldPacket data(SMSG_DISPEL_FAILED, 8+8+4+4*fail_list.size());
            data << uint64(m_caster->GetGUID());            // Caster GUID
            data << uint64(unitTarget->GetGUID());          // Victim GUID
            data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));                // dispel spell id
            for (std::list< uint32 >::iterator j = fail_list.begin(); j != fail_list.end(); ++j)
                data << uint32(*j);                         // Spell Id
            m_caster->BroadcastPacket(&data, true);
        }
        delete []dispel_list;
    }    
}

void Spell::EffectDualWield(uint32 /*i*/)
{
    unitTarget->SetCanDualWield(true);
    if (unitTarget->GetTypeId() == TYPEID_UNIT)
        ((Creature*)unitTarget)->UpdateDamagePhysical(OFF_ATTACK);
}

void Spell::EffectPull(uint32 /*i*/)
{
    // TODO: create a proper pull towards distract spell center for distract
    sLog.outDebug("WORLD: Spell Effect DUMMY");
}

void Spell::EffectDistract(uint32 /*i*/)
{
    // Check for possible target
    if (!unitTarget || unitTarget->IsInCombat())
        return;

    // target must be OK to do this
    if (unitTarget->HasUnitState(UNIT_STAT_CONFUSED | UNIT_STAT_STUNNED | UNIT_STAT_FLEEING))
        return;

    unitTarget->SetStandState(UNIT_STAND_STATE_STAND);

    if (unitTarget->GetTypeId() == TYPEID_UNIT)
        unitTarget->GetMotionMaster()->MoveDistract(damage * MILLISECONDS);

    unitTarget->SetFacingTo(unitTarget->GetOrientationTo(m_targets.m_destX, m_targets.m_destY));
}

void Spell::EffectPickPocket(uint32 /*i*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    // victim must be creature and attackable
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->IsFriendlyTo(unitTarget))
        return;

    // victim have to be alive and humanoid or undead
    if (unitTarget->isAlive() && (unitTarget->GetCreatureTypeMask() &CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) != 0)
    {
        int32 chance = 10 + int32(m_caster->GetLevel()) - int32(unitTarget->GetLevel());

        if (chance > irand(0, 19))
        {
            // Stealing successful
            //sLog.outDebug("Sending loot from pickpocket");
            ((Player*)m_caster)->SendLoot(unitTarget->GetGUID(),LOOT_PICKPOCKETING);
        }
        else
        {
            // Reveal action + get attack
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
			if (((Creature*)unitTarget)->IsAIEnabled && !unitTarget->HasUnitState(UNIT_STAT_LOST_CONTROL))
                ((Creature*)unitTarget)->AI()->AttackStart(m_caster);
        }
    }
}

void Spell::EffectAddFarsight(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    float radius = SpellMgr::GetSpellRadius(GetSpellEntry(),i,false);
    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());
    DynamicObject* dynObj = new DynamicObject;
    if (!dynObj->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_DYNAMICOBJECT), m_caster, GetSpellEntry()->Id, 4, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, duration, radius))
    {
        delete dynObj;
        return;
    }

    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);
    m_caster->AddDynObject(dynObj);

    //dynObj->setActive(true);    //must before add to map to be put in world container
    m_caster->GetMap()->Add(dynObj); //grid will also be loaded

    m_caster->UpdateVisibilityAndView();

    ((Player*)m_caster)->GetCamera().SetView(dynObj, true);
}

void Spell::EffectSummonWild(uint32 i)
{
    DoEffectSummonWild(i);
}

void Spell::DoEffectSummonWild(uint32 i, bool DieWithSummoner)
{
    uint32 creature_entry = GetSpellEntry()->EffectMiscValue[i];
    if (!creature_entry)
        return;

    if (!(m_caster->GetTypeId() == TYPEID_UNIT && ((Creature*)m_caster)->GetEntry() == 18313) && m_caster->HasEventAISummonedUnits())
        return;

    uint32 level = m_caster->GetLevel();

    // level of creature summoned using engineering item based at engineering skill level
    if (m_caster->GetTypeId()==TYPEID_PLAYER && m_CastItem)
    {
        ItemPrototype const *proto = m_CastItem->GetProto();
        if (proto && proto->RequiredSkill == SKILL_ENGINERING)
        {
            uint16 skill202 = ((Player*)m_caster)->GetSkillValue(SKILL_ENGINERING);
            if (skill202)
            {
                level = skill202/5;
            }
        }
    }

    // select center of summon position
    float center_x = m_targets.m_destX;
    float center_y = m_targets.m_destY;
    float center_z = m_targets.m_destZ;

    float radius = SpellMgr::GetSpellRadius(GetSpellEntry(),i,false);

    int32 amount = damage > 0 ? damage : 1;

    for (int32 count = 0; count < amount; ++count)
    {
        float px, py, pz;
        // If dest location if present
        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            // Summon 1 unit in dest location
            if (count == 0)
            {
                px = m_targets.m_destX;
                py = m_targets.m_destY;
                pz = m_targets.m_destZ;
            }
            // Summon in random point all other units if location present
            else
                m_caster->GetRandomPoint(center_x,center_y,center_z,radius,px,py,pz);
        }
        // Summon if dest location not present near caster
        else
            m_caster->GetNearPoint(px,py,pz,3.0f);

        int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());

        TemporarySummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;

        Creature* summoned = NULL;
        if (m_originalCaster)
            summoned = m_originalCaster->SummonCreature(creature_entry, px, py, pz, m_caster->GetOrientation(), summonType, duration, DieWithSummoner);
        else
            summoned = m_caster->SummonCreature(creature_entry, px, py, pz, m_caster->GetOrientation(), summonType, duration, DieWithSummoner);

        if (summoned && (GetSpellEntry()->Effect[i] == SPELL_EFFECT_SUMMON_PET || summoned->GetEntry() == 12922))
            summoned->setFaction(m_caster->getFaction());

        if(summoned && (summoned->GetEntry() == 3950 || summoned->GetEntry() == 10928))
            summoned->SetOwnerGUID(m_caster->GetGUID());
    }
}

void Spell::EffectSummonGuardian(uint32 i)
{
    uint32 pet_entry = GetSpellEntry()->EffectMiscValue[i];
    if (!pet_entry)
        return;

    // Jewelery statue case (totem like)
    if (GetSpellEntry()->SpellIconID==2056)
    {
        EffectSummonTotem(i);
        return;
    }

    // set timer for unsummon
    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());

    Player *caster = NULL;
    if (m_originalCaster)
    {
        if (m_originalCaster->GetTypeId() == TYPEID_PLAYER)
            caster = (Player*)m_originalCaster;
        else if (((Creature*)m_originalCaster)->isTotem())
        {
            if (((Creature*)m_originalCaster)->GetEntry() == 15439 || ((Creature*)m_originalCaster)->GetEntry() == 15430 || ((Creature*)m_originalCaster)->GetEntry() == 22236)
            {
              float px, py, pz;
              m_caster->GetNearPoint(px,py,pz,m_caster->GetObjectSize());
              if (caster = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself())
                if (Pet *spawnCreature = caster->SummonPet(GetSpellEntry()->EffectMiscValue[i], px, py, pz, m_caster->GetOrientation(), GUARDIAN_PET, duration))
                {
                   spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
                   spawnCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellEntry()->Id);
                   spawnCreature->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_PVP_ATTACKABLE);
                   // spawnCreature->SetOwnerGUID(m_caster->GetGUID()); -> elemental despawns are done via their scripts.
                   spawnCreature->AI()->EventPulse(m_originalCaster, 0); // and here we let elemental know about the totem GUID
                   spawnCreature->SetReactState(REACT_DEFENSIVE);

                   float healthfactor = (frand(0.15, 0.30));
                   float manafactor = (frand(0.2, 0.8));
                   spawnCreature->SetMaxHealth(spawnCreature->GetMaxHealth() + caster->GetHealthBonusFromStamina() * healthfactor);
                   spawnCreature->SetHealth(spawnCreature->GetMaxHealth() + caster->GetHealthBonusFromStamina() * healthfactor);
                   if(spawnCreature->GetEntry() == 15438)
                   {
                      spawnCreature->SetMaxPower(POWER_MANA, (spawnCreature->GetMaxPower(POWER_MANA) + caster->GetManaBonusFromIntellect() * manafactor));
                      spawnCreature->SetPower(POWER_MANA,(spawnCreature->GetMaxPower(POWER_MANA) + caster->GetManaBonusFromIntellect() * manafactor));
                   }
                   //Precise values are impossible to find. This values (+ 10-30%) are the conclusion of all available proofs, lovered by 10%.
                   //After checking the impact on the game if may be lowered to 10%.
                   //Mana bonus must be much lover, due to conclusion of spells costs.
                   //Bonus spell damage is not confirmed, so it's NOT implemented.

                   return;
                }
            }
            else
                caster = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself();
        }
    }

    if (!caster)
    {
        DoEffectSummonWild(i, true);
        return;
    }
    
    // Search old Guardian only for players (if cast spell not have duration or cooldown)
    // FIXME: some guardians have control spell applied and controlled by player and anyway player can't summon in this time
    //        so this code hack in fact
    if ((duration <= 0 || SpellMgr::GetSpellRecoveryTime(GetSpellEntry())==0) && !m_CastItem || GetSpellEntry()->Id == 36904) //items has their own cd
        if (caster->CountGuardianWithEntry(pet_entry))
            return;                                        // find old guardian, ignore summon

    auto guardians = caster->GetGuardians();
    if (guardians.size() >= 30 && !caster->GetSession()->HasPermissions(PERM_GMT))
    {
        std::stringstream stream;
        stream << "Guardians summoning exploit - Spellid: " << GetSpellEntry()->Id << " count : " << guardians.size() << " cheater name: " << caster->GetName();
        
        sWorld.SendGMText(LANG_POSSIBLE_CHEAT, stream.str().c_str());
        sLog.outLog(LOG_CRITICAL, stream.str().c_str());
    }
    
    // in another case summon new
    uint32 level = caster->GetLevel();

    // level of pet summoned using engineering item based at engineering skill level
    if (m_CastItem)
    {
        ItemPrototype const *proto = m_CastItem->GetProto();
        if (proto && proto->RequiredSkill == SKILL_ENGINERING)
        {
            uint16 skill202 = caster->GetSkillValue(SKILL_ENGINERING);
            if (skill202)
            {
                level = skill202/5;
            }
        }
    }

    // select center of summon position
    float center_x = m_targets.m_destX;
    float center_y = m_targets.m_destY;
    float center_z = m_targets.m_destZ;

    float radius = SpellMgr::GetSpellRadius(GetSpellEntry(),i,false);

    int32 amount = damage > 0 ? damage : 1;

    if (GetSpellEntry()->Id == 45145) // Clever Traps - Snake Trap hackfix // TRENTONE check for need
    {
        if (caster->HasAura(19239, 0))
        {
            if (pet_entry == 19921)
                amount++;
        }
        else if (caster->HasAura(19245, 0))
            amount++;

    }
    for (int32 count = 0; count < amount; ++count)
    {
        float px, py, pz;
        // If dest location if present
        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            // Summon 1 unit in dest location
            if (count == 0)
            {
                px = m_targets.m_destX;
                py = m_targets.m_destY;
                pz = m_targets.m_destZ;
            }
            // Summon in random point all other units if location present
            else
                m_caster->GetRandomPoint(center_x,center_y,center_z,radius,px,py,pz);
        }
        // Summon if dest location not present near caster
        else
            m_caster->GetNearPoint(px,py,pz,m_caster->GetObjectSize());

        Pet *spawnCreature = caster->SummonPet(GetSpellEntry()->EffectMiscValue[i], px, py, pz, m_caster->GetOrientation(), GUARDIAN_PET, duration);
        if (!spawnCreature)
            return;

        spawnCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP,0);
        spawnCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellEntry()->Id);
        spawnCreature->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_PVP_ATTACKABLE);

        if (GetSpellEntry()->Id == 17166)
        {
            spawnCreature->MonsterTextEmote("%s marches around, roaring and making a ruckus.", 0, false);
            spawnCreature->MonsterSay("RAAAAAAAR!", 0, 0);
            if (spawnCreature->GetAreaId() == 541)
            {
                Creature *pCreature = NULL;
                Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*spawnCreature, 10977, true, 30.0, false);
                Hellground::ObjectLastSearcher<Creature, Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);
                Cell::VisitGridObjects(spawnCreature, searcher, 30.0);
                if (pCreature) // NPC_QUIXXIL
                {
                    spawnCreature->GetMotionMaster()->MoveFollow(pCreature, 0.6f, 3.14);
                    pCreature->MonsterSay("Oh!!! Get that thing away from me!", 0, 0);
                    pCreature->SetWalk(false);
                    pCreature->GetMotionMaster()->MovePath(1097700, false);
                }
            }
            else if (spawnCreature->GetAreaId() == 976) // Tanaris
            {
                Creature *pCreature1 = NULL;
                Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check1(*spawnCreature, 7583, true, 30.0, false);
                Hellground::ObjectLastSearcher<Creature, Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher1(pCreature1, creature_check1);
                Cell::VisitGridObjects(spawnCreature, searcher1, 30.0);
                if (pCreature1) // NPC_SPRINKLE
                {
                    spawnCreature->GetMotionMaster()->MoveFollow(pCreature1, 0.6f, 3.14);
                    pCreature1->MonsterTextEmote("%s jumps in fright!", 0, 0);
                    pCreature1->SetWalk(false);
                    pCreature1->GetMotionMaster()->MovePath(758300, false);
                }
            }
            if (spawnCreature->GetAreaId() == 2255) // Winterspring
            {
                Creature *pCreature2 = NULL;
                Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check2(*spawnCreature, 10978, true, 30.0, false);
                Hellground::ObjectLastSearcher<Creature, Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher2(pCreature2, creature_check2);
                Cell::VisitGridObjects(spawnCreature, searcher2, 30.0);
                if (pCreature2) // NPC_LEGACKI
                {
                    spawnCreature->GetMotionMaster()->MoveFollow(pCreature2, 0.6f, 3.14);
                    pCreature2->MonsterTextEmote("%s jumps in fright!", 0, 0);
                    pCreature2->SetWalk(false);
                    pCreature2->GetMotionMaster()->MovePath(1097800, false);
                }
            }
        }
    }
}

void Spell::EffectSummonPossessed(uint32 i)
{
	uint32 entry = GetSpellEntry()->EffectMiscValue[i];
	if (!entry)
		return;

	if (m_caster->GetTypeId() != TYPEID_PLAYER)
		return;

	uint32 level = m_caster->GetLevel();

	float x, y, z;
	m_caster->GetNearPoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE);

	int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());

	Pet* oldpet = m_caster->GetPet();
	if (oldpet)
	{
		if (oldpet->isControlled())
		{
			m_caster->ToPlayer()->SetTemporaryUnsummonedPetNumber(oldpet->GetCharmInfo()->GetPetNumber());
			m_caster->ToPlayer()->SetOldPetSpell(oldpet->GetUInt32Value(UNIT_CREATED_BY_SPELL));
		}
		m_caster->ToPlayer()->RemovePet(oldpet, PET_SAVE_NOT_IN_SLOT);
	}

	Pet* pet = m_caster->ToPlayer()->SummonPet(entry, x, y, z + 0.5f, m_caster->GetOrientation(), POSSESSED_PET, duration);
	if (!pet)
		return;

	if (entry == 4277)
		pet->CastSpell(pet, 2585, false);

	pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellEntry()->Id);
	pet->SetCharmedOrPossessedBy(m_caster, true);

	// hack for dream vision and eye of kilrog
	if (entry == 7863 || entry == 4277)
	{
		pet->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
		pet->addUnitState(UNIT_STAT_CANNOT_AUTOATTACK);		
	}       
}

void Spell::EffectTeleUnitsFaceCaster(uint32 i)
{
    if (!unitTarget)
        return;

    if (unitTarget->IsTaxiFlying())
        return;

    uint32 mapid = m_caster->GetMapId();

    if(!m_targets.HasDst())
    {
        float dis = SpellMgr::GetSpellRadius(GetSpellEntry(),i,false);

        float fx,fy,fz;
        m_caster->GetNearPoint(fx,fy,fz,unitTarget->GetObjectSize(),dis);

        if (mapid == unitTarget->GetMapId())
            unitTarget->NearTeleportTo(fx, fy, fz, -m_caster->GetOrientation(), unitTarget == m_caster);
        else if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            ((Player*)unitTarget)->TeleportTo(mapid, fx, fy, fz, -m_caster->GetOrientation(), unitTarget == m_caster ? TELE_TO_SPELL : 0);
    }
    else
    {
        if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            ((Player*)unitTarget)->TeleportTo(mapid, m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, unitTarget->GetOrientation());
    }

}

void Spell::EffectLearnSkill(uint32 i)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if (damage < 0)
        return;

    uint32 skillid =  GetSpellEntry()->EffectMiscValue[i];
    uint16 skillval = ((Player*)unitTarget)->GetPureSkillValue(skillid);
    ((Player*)unitTarget)->SetSkill(skillid, skillval?skillval:1, damage*75);
}

void Spell::EffectAddHonor(uint32 /*i*/)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    sLog.outDebug("SpellEffect::AddHonor called for spell_id %u , that rewards %d honor points to player: %u", GetSpellEntry()->Id, damage, ((Player*)unitTarget)->GetGUIDLow());

    // TODO: find formula for honor reward based on player's level!

    // now fixed only for level 70 players:
    if (((Player*)unitTarget)->GetLevel() == 70)
        ((Player*)unitTarget)->RewardHonor(NULL, 1, damage);
}

void Spell::EffectTradeSkill(uint32 /*i*/)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    // uint32 skillid =  GetSpellEntry()->EffectMiscValue[i];
    // uint16 skillmax = ((Player*)unitTarget)->(skillid);
    // ((Player*)unitTarget)->SetSkill(skillid,skillval?skillval:1,skillmax+75);
}

void Spell::EffectEnchantItemPerm(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!itemTarget)
        return;

    Player* p_caster = (Player*)m_caster;

    p_caster->UpdateCraftSkill(GetSpellEntry()->Id);

    if (GetSpellEntry()->EffectMiscValue[i])
    {
        uint32 enchant_id = GetSpellEntry()->EffectMiscValue[i];

        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!pEnchant)
            return;

        // item can be in trade slot and have owner diff. from caster
        Player* item_owner = itemTarget->GetOwner();
        if (!item_owner)
            return;

        if (item_owner!=p_caster)
        {
            if (p_caster->GetSession()->HasPermissions(PERM_GMT) && sWorld.getConfig(CONFIG_GM_LOG_TRADE))
                sLog.outCommand(p_caster->GetSession()->GetAccountId(),"GM %s (Account: %u) enchanting(perm): %s (Entry: %d) for player: %s (Account: %u)",
                    p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                    itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                    item_owner->GetName(),item_owner->GetSession()->GetAccountId());
            else
                sLog.outLog(LOG_TRADE, "Player %s (Account: %u) enchanting(perm): %s (Entry: %d) for player: %s (Account: %u)",
                    p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                    itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                    item_owner->GetName(),item_owner->GetSession()->GetAccountId());
        }

        // remove old enchanting before applying new if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,false);

        itemTarget->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchant_id, 0, 0);

        // add new enchanting if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,true);
    }
}

void Spell::EffectEnchantItemTmp(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;

    if (!itemTarget)
        return;

    uint32 enchant_id = GetSpellEntry()->EffectMiscValue[i];

    // Shaman Rockbiter Weapon
    if (i==0 && GetSpellEntry()->Effect[1]==SPELL_EFFECT_DUMMY)
    {
        int32 enchanting_damage = CalculateDamage(1);//+1;

        // enchanting id selected by calculated damage-per-sec stored in Effect[1] base value
        // with already applied percent bonus from Elemental Weapons talent
        // Note: damage calculated (correctly) with rounding int32(float(v)) but
        // RW enchantments applied damage int32(float(v)+0.5), this create  0..1 difference sometime
        switch (enchanting_damage)
        {
            // Rank 1
            case  2: enchant_id =   29; break;              //  0% [ 7% ==  2, 14% == 2, 20% == 2]
            // Rank 2
            case  4: enchant_id =    6; break;              //  0% [ 7% ==  4, 14% == 4]
            case  5: enchant_id = 3025; break;              // 20%
            // Rank 3
            case  6: enchant_id =    1; break;              //  0% [ 7% ==  6, 14% == 6]
            case  7: enchant_id = 3027; break;              // 20%
            // Rank 4
            case  9: enchant_id = 3032; break;              //  0% [ 7% ==  6]
            case 10: enchant_id =  503; break;              // 14%
            case 11: enchant_id = 3031; break;              // 20%
            // Rank 5
            case 15: enchant_id = 3035; break;              // 0%
            case 16: enchant_id = 1663; break;              // 7%
            case 17: enchant_id = 3033; break;              // 14%
            case 18: enchant_id = 3034; break;              // 20%
            // Rank 6
            case 28: enchant_id = 3038; break;              // 0%
            case 29: enchant_id =  683; break;              // 7%
            case 31: enchant_id = 3036; break;              // 14%
            case 33: enchant_id = 3037; break;              // 20%
            // Rank 7
            case 40: enchant_id = 3041; break;              // 0%
            case 42: enchant_id = 1664; break;              // 7%
            case 45: enchant_id = 3039; break;              // 14%
            case 48: enchant_id = 3040; break;              // 20%
            // Rank 8
            case 49: enchant_id = 3044; break;              // 0%
            case 52: enchant_id = 2632; break;              // 7%
            case 55: enchant_id = 3042; break;              // 14%
            case 58: enchant_id = 3043; break;              // 20%
            // Rank 9
            case 62: enchant_id = 2633; break;              // 0%
            case 66: enchant_id = 3018; break;              // 7%
            case 70: enchant_id = 3019; break;              // 14%
            case 74: enchant_id = 3020; break;              // 20%
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Spell::EffectEnchantItemTmp: Damage %u not handled in S'RW",enchanting_damage);
                return;
        }
    }

    if (!enchant_id)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have 0 as enchanting id",GetSpellEntry()->Id,i);
        return;
    }

    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if (!pEnchant)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have not existed enchanting id %u ",GetSpellEntry()->Id,i,enchant_id);
        return;
    }

    // select enchantment duration
    uint32 duration = 0;

    switch (GetSpellEntry()->Id)
    {
        case 38615: // rogue family enchantments exception by duration
            duration = 1800;
            break;
        //case 25122: // Brilliant Wizard Oil
        //case 28017: // Superior Wizard Oil
        //case 28013: // Superior Mana Oil
        //case 25123: // Brilliant Mana Oil
        //case 29453: // Sharpen Blade      ---   Adamantite
        //case 34340: // Weight Weapon      ---   Adamantite
        //case 22756: // Sharpen Weapon - Critical   --- Elemental sharpening stone
        //case 32282: // Greater Rune of Warding
        //    if (sWorld.isEasyRealm())
        //        duration = 86400;
        //    break;
        case 29702:
        case 37360:
            duration = 300;
            break;
        default:
            break;
    }
    // rogue family enchantments exception by duration
    if (!duration) 
    {
        if (GetSpellEntry()->SpellFamilyName==SPELLFAMILY_ROGUE)
        {
            duration = 3600;                                    // 1 hour
        }
        // shaman family enchantments
        else if (GetSpellEntry()->SpellFamilyName==SPELLFAMILY_SHAMAN)
        {
            duration = 1800;                                    // 30 mins
        }
        // other cases with this SpellVisual already selected
        else if (GetSpellEntry()->SpellVisual==215)
            duration = 1800;                                    // 30 mins
        // some fishing pole bonuses
        else if (GetSpellEntry()->SpellVisual==563)
            duration = 600;                                     // 10 mins
        // shaman rockbiter enchantments
        else if (GetSpellEntry()->SpellVisual==0)
        {
            duration = 1800;                                    // 30 mins
        }
        // default case
        else
            duration = 3600;                                    // 1 hour
    }

    // item can be in trade slot and have owner diff. from caster
    Player* item_owner = itemTarget->GetOwner();
    if (!item_owner)
        return;

    if (item_owner!=p_caster)
    {
        if  (p_caster->GetSession()->HasPermissions(PERM_GMT) && sWorld.getConfig(CONFIG_GM_LOG_TRADE))
            sLog.outCommand(p_caster->GetSession()->GetAccountId(),"GM %s (Account: %u) enchanting(temp): %s (Entry: %d) for player: %s (Account: %u)",
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());
        else
            sLog.outLog(LOG_TRADE, "Player %s (Account: %u) enchanting(temp): %s (Entry: %d) for player: %s (Account: %u)",
                p_caster->GetName(),p_caster->GetSession()->GetAccountId(),
                itemTarget->GetProto()->Name1,itemTarget->GetEntry(),
                item_owner->GetName(),item_owner->GetSession()->GetAccountId());
    }

    // remove old enchanting before applying new if equipped
    item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,false);

    itemTarget->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, duration*1000, 0);

    // add new enchanting if equipped
    item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,true);
}

void Spell::EffectTameCreature(uint32 /*i*/)
{
    if (m_caster->GetPetGUID())
        return;

    if (!unitTarget)
        return;

    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
        return;

    Creature* creatureTarget = (Creature*)unitTarget;

    if (creatureTarget->isPet())
        return;

    if (m_caster->GetClass() != CLASS_HUNTER)
        return;

    // cast finish successfully
    //SendChannelUpdate(0);
    finish();

    Pet* pet = m_caster->CreateTamedPetFrom(creatureTarget,GetSpellEntry()->Id);
    if (!pet) return;

    // kill original creature
    creatureTarget->setDeathState(JUST_DIED);
    creatureTarget->RemoveCorpse();
    creatureTarget->SetHealth(0);                       // just for nice GM-mode view

    // prepare visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL,creatureTarget->GetLevel()-1);

    // add to world
    pet->GetMap()->Add((Creature*)pet);

    // visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL,creatureTarget->GetLevel());

    // caster have pet now
    m_caster->SetPet(pet);

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        ((Player*)m_caster)->PetSpellInitialize();
        if (sWorld.isEasyRealm())
            pet->GivePetXP(1000000000); // just a really big number for the max level (stopped at owner's level))
    }
}

void Spell::EffectSummonPet(uint32 i)
{
    Player *owner = NULL;
    if (m_originalCaster)
    {
        if (m_originalCaster->GetTypeId() == TYPEID_PLAYER)
            owner = (Player*)m_originalCaster;
        else if (((Creature*)m_originalCaster)->isTotem())
            owner = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself();
    }

    if (!owner)
    {
        DoEffectSummonWild(i, true);
        return;
    }

    uint32 petentry = GetSpellEntry()->EffectMiscValue[i];

    Pet *OldSummon = owner->GetPet();

    // if pet requested type already exist
    if (OldSummon)
    {
        if (petentry == 0 || OldSummon->GetEntry() == petentry)
        {
            // pet in corpse state can't be summoned
            if (OldSummon->isDead())
                return;

            // hack fix - remove Summoning Disorientation to prevent pet teleporting after resummon
            OldSummon->RemoveAurasDueToSpell(32752);

            float px, py, pz;

            if (m_targets.HasDst())
            {
                px = m_targets.m_destX;
                py = m_targets.m_destY;
                pz = m_targets.m_destZ;
            }
            else
                owner->GetNearPoint(px, py, pz, OldSummon->GetObjectSize());

            if (!OldSummon->IsInMap(owner))
            {
                OldSummon->GetMap()->Remove((Creature*)OldSummon,false);
                OldSummon->SetMapId(owner->GetMapId());
                OldSummon->Relocate(px, py, pz, OldSummon->GetOrientation());
                owner->GetMap()->Add((Creature*)OldSummon);
            }
            else
                OldSummon->NearTeleportTo(px, py, pz, OldSummon->GetOrientation());

            if (owner->GetTypeId() == TYPEID_PLAYER && OldSummon->isControlled())
            {
                ((Player*)owner)->PetSpellInitialize();
            }

            if (OldSummon->getPetType() == SUMMON_PET)
             {
                 OldSummon->SetHealth(OldSummon->GetMaxHealth());
                 OldSummon->SetPower(POWER_MANA, OldSummon->GetMaxPower(POWER_MANA));
                 OldSummon->RemoveAllAurasButPermanent();
                 OldSummon->m_CreatureSpellCooldowns.clear();
                 OldSummon->m_CreatureCategoryCooldowns.clear();
                 CharmInfo* chinf = OldSummon->GetCharmInfo();
                 if (chinf) // well, it should exist, cause it's pet
                     chinf->GetCooldownMgr().ClearGlobalCooldowns();
            }
            return;
        }

        if (owner->GetTypeId() == TYPEID_PLAYER)
            ((Player*)owner)->RemovePet(OldSummon,(OldSummon->getPetType()==HUNTER_PET ? PET_SAVE_AS_DELETED : PET_SAVE_NOT_IN_SLOT),false);
        else
            return;
    }

    float x, y, z;
    if (m_targets.HasDst())
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
    }
    else
        owner->GetNearPoint(x, y, z, owner->GetObjectSize());

    Pet* pet = owner->SummonPet(petentry, x, y, z, owner->GetOrientation(), SUMMON_PET, 0);
    if (!pet)
        return;

    if (m_caster->GetTypeId() == TYPEID_UNIT)
    {
        if (((Creature*)m_caster)->isTotem())
            pet->SetReactState(REACT_AGGRESSIVE);
        else
            pet->SetReactState(REACT_DEFENSIVE);
    }

    pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellEntry()->Id);

    // this enables popup window (pet dismiss, cancel), hunter pet additional flags set later
    pet->SetUInt32Value(UNIT_FIELD_FLAGS,UNIT_FLAG_PVP_ATTACKABLE);
    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));

    // generate new name for summon pet
    std::string new_name=sObjectMgr.GeneratePetName(petentry);
    if (!new_name.empty())
        pet->SetName(new_name);
}

void Spell::EffectLearnPetSpell(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    Pet *pet = _player->GetPet();
    if (!pet)
        return;
    if (!pet->isAlive())
        return;

    SpellEntry const *learn_spellproto = sSpellTemplate.LookupEntry<SpellEntry>(GetSpellEntry()->EffectTriggerSpell[i]);
    if (!learn_spellproto)
        return;

    pet->SetTP(pet->m_TrainingPoints - pet->GetTPForSpell(learn_spellproto->Id));
    pet->learnSpell(learn_spellproto->Id);

    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    _player->PetSpellInitialize();
}

void Spell::EffectTaunt(uint32 /*i*/)
{
    if (!unitTarget)
        return;

    // this effect use before aura Taunt apply for prevent taunt already attacking target
    // for spell as marked "non effective at already attacking target"
    if (!unitTarget->CanHaveThreatList()
        || unitTarget->GetVictim() == m_caster)
    {
        SendCastResult(SPELL_FAILED_DONT_REPORT);
        return;
    }

    if (unitTarget->IsImmunedToSpellEffect(SPELL_EFFECT_ATTACK_ME,MECHANIC_NONE))
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    // Also use this effect to set the taunter's threat to the taunted creature's highest value
    if (unitTarget->getThreatManager().getCurrentVictim())
    {
        float myThreat = unitTarget->getThreatManager().getThreat(m_caster);
        float itsThreat = unitTarget->getThreatManager().getCurrentVictim()->getThreat();
        if (itsThreat > myThreat)
            unitTarget->AddThreat(m_caster, itsThreat - myThreat);
    }

    //Set aggro victim to caster
    if (!unitTarget->getThreatManager().getOnlineContainer().empty())
        if (HostileReference* forcedVictim = unitTarget->getThreatManager().getOnlineContainer().getReferenceByTarget(m_caster))
            unitTarget->getThreatManager().setCurrentVictim(forcedVictim);

    if (((Creature*)unitTarget)->IsAIEnabled)
        ((Creature*)unitTarget)->AI()->AttackStart(m_caster);
}

void Spell::EffectWeaponDmg(uint32 i)
{
}

void Spell::SpellDamageWeaponDmg(uint32 i)
{
    if (!unitTarget)
        return;
    if (!unitTarget->isAlive())
        return;

    // multiple weapon dmg effect workaround
    // execute only the last weapon damage
    // and handle all effects at once
    for (int j = 0; j < 3; j++)
    {
        switch (GetSpellEntry()->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                if (j < i)                                  // we must calculate only at last weapon effect
                    return;
            break;
        }
    }

    // some spell specific modifiers
    //float weaponDamagePercentMod = 1.0f;                    // applied to weapon damage (and to fixed effect damage bonus if customBonusDamagePercentMod not set
    float totalDamagePercentMod  = 1.0f;                    // applied to final bonus+weapon damage
    int32 fixed_bonus = 0;
    int32 spell_bonus = 0;                                  // bonus specific for spell

    switch (GetSpellEntry()->SpellFamilyName)
    {
        case SPELLFAMILY_WARRIOR:
        {
            // Heroic Strike
            if (GetSpellEntry()->SpellFamilyFlags & 0x40)
            {
                switch (GetSpellEntry()->Id)
                {
                    // Heroic Strike r10 + r11
                    case 29707:
                    case 30324:
                    {
                        Unit::AuraList const& decSpeedList = unitTarget->GetAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                        for (Unit::AuraList::const_iterator iter = decSpeedList.begin(); iter != decSpeedList.end(); ++iter)
                        {
                            if ((*iter)->GetSpellProto()->SpellIconID == 15 && (*iter)->GetSpellProto()->Dispel == 0)
                            {
                                fixed_bonus += (GetSpellEntry()->Id == 29707) ? 61 : 72;
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }
            // Devastate bonus and sunder armor refresh
            else if (GetSpellEntry()->SpellVisual == 671 && GetSpellEntry()->SpellIconID == 1508)
            {
                uint32 stack = 0;

                Unit::AuraList const& list = unitTarget->GetAurasByType(SPELL_AURA_MOD_RESISTANCE);
                for (Unit::AuraList::const_iterator itr=list.begin();itr!=list.end();++itr)
                {
                    SpellEntry const *proto = (*itr)->GetSpellProto();
                    if (proto->SpellFamilyName == SPELLFAMILY_WARRIOR
                        && proto->SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_SUNDERARMOR)
                    {
                        int32 duration = SpellMgr::GetSpellDuration(proto);
                        (*itr)->SetAuraDuration(duration);
                        (*itr)->UpdateAuraDuration();
                        stack = (*itr)->GetStackAmount();
                        break;
                    }
                }

                for (int j = 0; j < 3; j++)
                {
                    if (GetSpellEntry()->Effect[j] == SPELL_EFFECT_NORMALIZED_WEAPON_DMG)
                    {
                        fixed_bonus += (stack - 1) * CalculateDamage(j);
                        break;
                    }
                }

                if (stack < 5)
                {
                    // get highest rank of the Sunder Armor spell
                    const PlayerSpellMap& sp_list = ((Player*)m_caster)->GetSpellMap();
                    for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                    {
                        // only highest rank is shown in spell book, so simply check if shown in spell book
                        if (!itr->second.active || itr->second.disabled || itr->second.state == PLAYERSPELL_REMOVED)
                            continue;

                        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                        if (!spellInfo)
                            continue;

                        if (spellInfo->SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_SUNDERARMOR
                            && spellInfo->Id != GetSpellEntry()->Id
                            && spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR)
                        {
                            m_caster->CastSpell(unitTarget, spellInfo, true);
                            break;
                        }
                    }
                }
            }
            else if (GetSpellEntry()->Id == 29707 || GetSpellEntry()->Id == 30324)
            {
                bool found = false;

                // check dazed affect
                Unit::AuraList const& decSpeedList = unitTarget->GetAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                for (Unit::AuraList::const_iterator iter = decSpeedList.begin(); iter != decSpeedList.end(); ++iter)
                {
                    if ((*iter)->GetSpellProto()->SpellIconID == 15)
                    {
                        found = true;
                        break;
                    }
                }

                // TODO: should this be put on taken but not done?
                if (found)
                {
                    fixed_bonus += int32(GetSpellEntry()->EffectBasePoints[0] * 0.35f);
                }
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            // Hemorrhage
            if (GetSpellEntry()->SpellFamilyFlags & 0x2000000)
            {
                if (m_caster->GetTypeId()==TYPEID_PLAYER)
                    ((Player*)m_caster)->AddComboPoints(unitTarget, 1);
            }
            // Mutilate (for each hand)
            else if (GetSpellEntry()->SpellFamilyFlags & 0x600000000LL)
            {
                Unit::AuraMap const& auras = unitTarget->GetAuras();
                for (Unit::AuraMap::const_iterator itr = auras.begin(); itr!=auras.end(); ++itr)
                {
                    if (itr->second->GetSpellProto()->Dispel == DISPEL_POISON)
                    {
                        totalDamagePercentMod *= 1.5f;          // 150% if poisoned
                        break;
                    }
                }
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Seal of Command - receive benefit from Spell Damage and Healing
            if (GetSpellEntry()->SpellFamilyFlags & 0x00000002000000LL)
            {
                spell_bonus += int32(0.20f*m_caster->SpellBaseDamageBonus(SpellMgr::GetSpellSchoolMask(GetSpellEntry())));
                spell_bonus += int32(0.20f*m_caster->SpellBaseDamageBonusForVictim(SpellMgr::GetSpellSchoolMask(GetSpellEntry()), unitTarget));
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            // Skyshatter Harness item set bonus
            // Stormstrike
            if (GetSpellEntry()->SpellFamilyFlags & 0x001000000000LL)
            {
                Unit::AuraList const& m_OverrideClassScript = m_caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for (Unit::AuraList::const_iterator citr = m_OverrideClassScript.begin(); citr != m_OverrideClassScript.end(); ++citr)
                {
                    // Stormstrike AP Buff
                    if ((*citr)->GetModifier()->m_miscvalue == 5634)
                    {
                        m_caster->CastSpell(m_caster,38430, true, NULL, *citr);
                        break;
                    }
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Mangle (Cat): CP
            if (GetSpellEntry()->SpellFamilyFlags==0x0000040000000000LL)
            {
                if (m_caster->GetTypeId()==TYPEID_PLAYER)
                    ((Player*)m_caster)->AddComboPoints(unitTarget,1);
            }
            // Maim interrupt (s3, s4 gloves bonus)
            if (GetSpellEntry()->Id == 22570 && m_caster->HasAura(44835, 0))
                m_caster->CastSpell(unitTarget, 32747, true);
            break;
        }
    }

    bool normalized = false;
    float weaponDamagePercentMod = 1.0;
    for (int j = 0; j < 3; ++j)
    {
        switch (GetSpellEntry()->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                fixed_bonus += CalculateDamage(j);
                break;
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                fixed_bonus += CalculateDamage(j);
                normalized = true;
                break;
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                weaponDamagePercentMod *= float(CalculateDamage(j)) / 100.0f;
            default:
                break;                                      // not weapon damage effect, just skip
        }
    }

    // apply to non-weapon bonus weapon total pct effect, weapon total flat effect included in weapon damage
    if (fixed_bonus || spell_bonus)
    {
        UnitMods unitMod;
        switch (m_attackType)
        {
            default:
            case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
            case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
            case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
        }

        float weapon_total_pct  = m_caster->GetModifierValue(unitMod, TOTAL_PCT);

        // 50% offhand damage reduction should not affect bonus damage
        if(unitMod == UNIT_MOD_DAMAGE_OFFHAND)
            weapon_total_pct /= 0.5f;

        if (fixed_bonus)
            fixed_bonus = int32(fixed_bonus * weapon_total_pct);
        if (spell_bonus)
            spell_bonus = int32(spell_bonus * weapon_total_pct);
    }

    int32 weaponDamage = m_caster->CalculateDamage(m_attackType, normalized);

    // Sequence is important
    for (int j = 0; j < 3; ++j)
    {
        // We assume that a spell have at most one fixed_bonus
        // and at most one weaponDamagePercentMod
        switch (GetSpellEntry()->Effect[j])
        {
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                weaponDamage += fixed_bonus;
                break;
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                weaponDamage = int32(weaponDamage * weaponDamagePercentMod);
            default:
                break;                                      // not weapon damage effect, just skip
        }
    }

    // only for Seal of Command
    if (spell_bonus)
        weaponDamage += spell_bonus;

    // only for Mutilate
    if (totalDamagePercentMod != 1.0f)
        weaponDamage = int32(weaponDamage * totalDamagePercentMod);

    // prevent negative damage
    uint32 eff_damage = uint32(weaponDamage > 0 ? weaponDamage : 0);

    // Add melee damage bonuses (also check for negative)
    m_caster->MeleeDamageBonus(unitTarget, &eff_damage, m_attackType, GetSpellEntry());
    m_damage+= eff_damage;
}

void Spell::EffectThreat(uint32 /*i*/)
{
    if (!unitTarget || !unitTarget->isAlive() || !m_caster->isAlive())
        return;

    if (!unitTarget->CanHaveThreatList())
        return;

    unitTarget->AddThreat(m_caster, float(damage));
}

void Spell::EffectHealMaxHealth(uint32 /*i*/)
{
    if (!unitTarget)
        return;

    if (!unitTarget->isAlive())
        return;

    if (unitTarget->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_HEALING_PCT) <= -100)
        return;

    uint32 addhealth = unitTarget->GetMaxHealth() - unitTarget->GetHealth();

    // Lay on Hands
    if (GetSpellEntry()->SpellFamilyName == SPELLFAMILY_PALADIN && GetSpellEntry()->SpellFamilyFlags & 0x0000000000008000)
    {
        if (!m_originalCaster || m_originalCaster->IsHostileTo(unitTarget))
            return;

        addhealth = addhealth > m_originalCaster->GetMaxHealth() ? m_originalCaster->GetMaxHealth() : addhealth;
        uint32 LoHamount = unitTarget->GetHealth() + m_originalCaster->GetMaxHealth();
        LoHamount = LoHamount > unitTarget->GetMaxHealth() ? unitTarget->GetMaxHealth() : LoHamount;

        unitTarget->SetHealth(LoHamount);
    }
    else
        unitTarget->SetHealth(unitTarget->GetMaxHealth());

    if (m_originalCaster)
        m_originalCaster->SendHealSpellLog(unitTarget, sSpellMgr.GetSpellAnalog(GetSpellEntry()), addhealth, false);
}

void Spell::EffectInterruptCast(uint32 i)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    // TODO: not all spells that used this effect apply cooldown at school spells
    // also exist case: apply cooldown to interrupted cast only and to all spells
    for (uint32 k = CURRENT_FIRST_NON_MELEE_SPELL; k < CURRENT_MAX_SPELL; k++)
    {
        if (Spell* spell = unitTarget->m_currentSpells[k])
        {
            const SpellEntry* curSpellEntry = spell->GetSpellEntry();
            // check if we can interrupt spell
            if ((spell->getState() == SPELL_STATE_CASTING || (spell->getState() == SPELL_STATE_PREPARING && spell->GetCastTime() > 0.0f)) &&
                curSpellEntry->PreventionType == SPELL_PREVENTION_TYPE_SILENCE &&
                ((k == CURRENT_GENERIC_SPELL && curSpellEntry->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) ||
                (k == CURRENT_CHANNELED_SPELL && curSpellEntry->ChannelInterruptFlags & CHANNEL_INTERRUPT_FLAG_MOVEMENT)))
            {
                if (m_originalCaster)
                {
                    int32 duration = m_originalCaster->CalculateSpellDuration(GetSpellEntry(), i, unitTarget);
                    unitTarget->ProhibitSpellSchool(SpellMgr::GetSpellSchoolMask(curSpellEntry), duration /* GetSpellDuration(GetSpellEntry())? */);
                }

                // has to be sent before InterruptSpell call
                WorldPacket data(SMSG_SPELLLOGEXECUTE, (8+4+4+4+4+8+4));
                data << m_caster->GetPackGUID();
                data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));
                data << uint32(1); // effect count
                data << uint32(SPELL_EFFECT_INTERRUPT_CAST);
                data << uint32(1); // target count
                data << unitTarget->GetPackGUID();
                data << uint32(sSpellMgr.GetSpellAnalog(curSpellEntry));
                m_caster->BroadcastPacket(&data, true);

                unitTarget->InterruptSpell(k, false);
            }
        }
    }
}

void Spell::EffectSummonObjectWild(uint32 i)
{
    uint32 gameobject_id = GetSpellEntry()->EffectMiscValue[i];

    GameObject* pGameObj = new GameObject;

    WorldObject* target = focusObject;
    if (!target)
        target = m_caster;

    float x,y,z;
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
    }
    else
        m_caster->GetNearPoint(x,y,z,DEFAULT_WORLD_OBJECT_SIZE);

    Map *map = target->GetMap();

    if (!pGameObj->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id, map,
        x, y, z, target->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 100, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(GetSpellEntry()->Id);

    if (pGameObj->GetGoType() != GAMEOBJECT_TYPE_FLAGDROP)   // make dropped flag clickable for other players (not set owner guid (created by) for this)...
    {
        if (m_originalCaster)
            m_originalCaster->AddGameObject(pGameObj);
        else
            m_caster->AddGameObject(pGameObj);
    }
    map->Add(pGameObj);

    if (pGameObj->GetMapId() == 489 && pGameObj->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)  //WS
    {
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            Player *pl = (Player*)m_caster;
            BattleGround* bg = ((Player *)m_caster)->GetBattleGround();
            if (bg && bg->GetTypeID()==BATTLEGROUND_WS && bg->GetStatus() == STATUS_IN_PROGRESS)
            {
                PlayerTeam team = pl->HasAura(SPELL_BG_HORDE_ICON) ? ALLIANCE : HORDE;
                ((BattleGroundWS*)bg)->SetDroppedFlagGUID(pGameObj->GetGUID(),team);
            }
        }
    }

    if (pGameObj->GetMapId() == 566 && pGameObj->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)  //EY
    {
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            BattleGround* bg = ((Player *)m_caster)->GetBattleGround();
            if (bg && bg->GetTypeID()==BATTLEGROUND_EY && bg->GetStatus() == STATUS_IN_PROGRESS)
            {
                ((BattleGroundEY*)bg)->SetDroppedFlagGUID(pGameObj->GetGUID());
            }
        }
    }

    if (uint32 linkedEntry = pGameObj->GetLinkedGameObjectEntry())
    {
        GameObject* linkedGO = new GameObject;
        if (linkedGO->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), linkedEntry, map,
            x, y, z, target->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 100, GO_STATE_READY))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration/1000 : 0);
            linkedGO->SetSpellId(GetSpellEntry()->Id);

            m_caster->AddGameObject(linkedGO);
            map->Add(linkedGO);
        }
        else
        {
            delete linkedGO;
            linkedGO = NULL;
            return;
        }
    }
}

void Spell::EffectScriptEffect(uint32 effIndex)
{
    // TODO: we must implement hunter pet summon at login there (spell 6962)
    switch (GetSpellEntry()->Id)
    {
        case 34842:
        {
            if (m_caster->ToCreature())
            {
                m_caster->SetVisibility(VISIBILITY_OFF);
                m_caster->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                ((Creature*)m_caster)->SetReactState(REACT_PASSIVE);
            }
            break;
        }
        case 37666:
        {
            if(!m_caster->CanHaveThreatList())
                return;

            uint8 counter = 0;

            std::list<HostileReference*> PlayerList = m_caster->getThreatManager().getThreatList();

            if(PlayerList.empty() && m_caster->IsInCombat())
                ((Creature*)m_caster)->AI()->EnterEvadeMode();

            for(std::list<HostileReference*>::iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                Unit* pUnit = m_caster->GetMap()->GetUnit((*i)->getUnitGuid());
                if(!pUnit)
                    continue;
                if(pUnit->GetTypeId() != TYPEID_PLAYER)
                    continue;
                if(pUnit->IsInWorld() && pUnit->IsInMap(m_caster))
                {
                    if(counter < 5) // only 5 targets(group)
                        counter++;
                    else
                        break;
                    m_caster->CastSpell(pUnit, 37667, true);
                }
            }
            break;
        }
        case 32191:
        {
            if(!m_caster->CanHaveThreatList())
                return;

            uint8 counter = 0;

            std::list<HostileReference*> PlayerList = m_caster->getThreatManager().getThreatList();

            if(PlayerList.empty() && m_caster->IsInCombat())
                ((Creature*)m_caster)->AI()->EnterEvadeMode();

            for(std::list<HostileReference*>::iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                Unit* pUnit = m_caster->GetMap()->GetUnit((*i)->getUnitGuid());
                if(!pUnit)
                    continue;
                if(pUnit->GetTypeId() != TYPEID_PLAYER)
                    continue;
                if(pUnit->IsInWorld() && pUnit->IsInMap(m_caster))
                {
                    if(counter < 5) // only 5 targets(group)
                        counter++;
                    else
                        break;
                    m_caster->CastSpell(pUnit, 34672, true);
                }
            }
            break;
        }
        /*case 32314: // kil'sorrow banner
        {
            uint32 const CREDIT_MARKER = 18393;
            if (Player* caster = m_caster->ToPlayer())
            {
                caster->KilledMonster(CREDIT_MARKER, unitTarget->GetGUID());
                if (Creature* creature = unitTarget->ToCreature())
                    creature->RemoveCorpse();
            }
            return;
        }*/
        /*case 32307: // warmaul banner
        {
            uint32 const CREDIT_MARKER = 18388;
            if (Player* caster = m_caster->ToPlayer())
            {
                caster->KilledMonster(CREDIT_MARKER, unitTarget->GetGUID());
                if (Creature* creature = unitTarget->ToCreature())
                    creature->RemoveCorpse();
            }
            return;
        }*/
        case 28338:
        case 28339:
        {
            if (m_caster->ToCreature() && unitTarget->ToCreature())
            {
                if (Unit *pVictim = unitTarget->GetVictim())
                {
                    unitTarget->getThreatManager().modifyThreatPercent(pVictim, -100);
                    m_caster->CastSpell(pVictim, 28337, true);

                    // need to add threat to new victim, but if unitTarget need to cast MagneticPull on us we will bring same victim back to us :p
                }
            }
            break;
        }
        // Flame of Azzinoth Blaze
        case 40609:
        {
            unitTarget->CastSpell(unitTarget, 40637, true, 0, 0, m_caster->GetGUID());
            break;
        }
        case 38530:
        {
            m_caster->RemoveAurasDueToSpell(38495);
            break;
        }
        // Cage Trap Trigger (Maiev)
        case 40761:
        {
            if (unitTarget->GetTypeId() == TYPEID_UNIT)
                ((Creature*)unitTarget)->AI()->DoAction();

            break;
        }
        // Capacitor Overload visual
        case 45014:
        {
            for(uint8 i = 0; i < 8; ++i)
            {
                m_caster->CastSpell(m_caster, damage, true);
            }
            break;
        }
        // Electrical Overload & visual
        case 45336:
        {
            uint8 effect = urand(0, 1);
            // visual
            for(uint8 i = 0; i < 8; ++i)
            {
                m_caster->CastSpell(m_caster, 44993, true);
            }
            // effect
            switch(effect)
            {
                // self stun
                case 0:
                    m_caster->CastSpell(m_caster, 35856, true);
                    break;
                // cast Broken Capacitor
                case 1:
                {
                    int32 selfdamage = 350;
                    m_caster->CastCustomSpell(m_caster, 44986, 0, &selfdamage, 0, true);
                    break;
                }
            }
            break;
        }
        // Fog of Corruption (Felmyst)
        case 45714:
        {
            // unitTarget is here Felmyst, and affected player is a caster
            if(!m_caster->HasAura(45717))
            {
                m_caster->CastSpell(m_caster, 46411, true); // self-stun for a few sec
                unitTarget->CastSpell(m_caster, 45717, true);
            }
            break;
        }
        // Unbanish Azaloth
        case 37834:
        {
            if (unitTarget->HasAura(37833,0))
            {
                unitTarget->RemoveAurasDueToSpell(37833);

                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                {
                    if( Group* pGroup = ((Player*)m_caster)->GetGroup() )
                    {
                        for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                        {
                            Player *pGroupie = itr->getSource();
                            if( pGroupie && pGroupie->GetQuestStatus(10637) == QUEST_STATUS_INCOMPLETE)
                                pGroupie->AreaExploredOrEventHappens(10637);
                            if( pGroupie && pGroupie->GetQuestStatus(10688) == QUEST_STATUS_INCOMPLETE)
                                pGroupie->AreaExploredOrEventHappens(10688);
                        }
                    } else
                    {
                        if (((Player*)m_caster)->GetQuestStatus(10637) == QUEST_STATUS_INCOMPLETE)
                            ((Player*)m_caster)->AreaExploredOrEventHappens(10637);
                        if (((Player*)m_caster)->GetQuestStatus(10688) == QUEST_STATUS_INCOMPLETE)
                            ((Player*)m_caster)->AreaExploredOrEventHappens(10688);
                    }
                }
            }
            else
                 SendCastResult(SPELL_FAILED_BAD_TARGETS);
            break;
        }
        // Draenei Tomb Relic
        case 36867:
        {
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
                if (((Player*)m_caster)->GetQuestStatus(10842) == QUEST_STATUS_INCOMPLETE)
                    m_caster->SummonCreature(21445, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                else SendCastResult(SPELL_FAILED_NOT_READY);
                break;
        }
        // Destroy Deathforged Infernal
        case 38055:
        {
            m_caster->CastSpell(m_caster, 38054, true);
            return;
        }
        // Gathios the Shatterer: Judgement
        case 41467:
        {
            if (m_caster->GetTypeId() != TYPEID_UNIT)
                return;

            if (m_caster->HasAura(41459, 2)) // Seal of Blood
            {
                m_caster->CastSpell(m_caster->GetVictim(), 41461, true); // Judgement of Blood
                m_caster->RemoveAurasDueToSpell(41459);
                return;
            }
            if (m_caster->HasAura(41469, 2)) // Seal of Command
            {
                m_caster->CastSpell(m_caster->GetVictim(), 41470, true); // Judgement of Command
                m_caster->RemoveAurasDueToSpell(41469);
                return;
            }
            break;
        }
        // Gurtogg Bloodboil: Eject
        case 40486:
        {
            if (!m_caster->CanHaveThreatList())
                return;

            m_caster->getThreatManager().modifyThreatPercent(unitTarget, -40.0f);
            break;
        }
        // Bloodbolt & Blood Splash workaround
        case 41072:
        {
            const int32 damage = irand(3238, 3762);
            const int32 reduction = 0;
            m_caster->CastCustomSpell(unitTarget, 41229, &damage, &reduction, NULL, true);  // workaround not to implement spell effect 141 only for 1 spell
            unitTarget->CastCustomSpell(unitTarget, 41067, &damage, NULL, NULL, true, 0, 0, m_caster->GetGUID());
            break;
        }
        // Gravity Lapse teleports for Kael'thas in MgT
        case 44224:
        {
            if(!m_caster->CanHaveThreatList())
                return;

            uint32 GravityLapseSpellId = 44219; // Id for first teleport spell
            uint32 GravityLapseDOT = (m_caster->GetMap()->IsHeroic()?44226:49887); // knocback + damage
            uint32 GravityLapseChannel = 44251; // visual self-cast, triggers AoE beams
            uint8 counter = 0;

            std::list<HostileReference*> PlayerList = m_caster->getThreatManager().getThreatList();

            if(PlayerList.empty() && m_caster->IsInCombat())
                ((Creature*)m_caster)->AI()->EnterEvadeMode();

            for(std::list<HostileReference*>::iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                Unit* pUnit = m_caster->GetMap()->GetUnit((*i)->getUnitGuid());
                if(!pUnit)
                    continue;
                if(pUnit->GetTypeId() != TYPEID_PLAYER)
                    continue;
                if(((Player*)pUnit)->isGameMaster())
                    continue;
                if(pUnit->IsInWorld() && pUnit->IsInMap(m_caster))
                {
                    if(counter < 5) // safety counter for not to pass 5 teleporting spells
                        counter++;
                    else
                        break;
                    m_caster->CastSpell(pUnit, GravityLapseSpellId, true);
                    pUnit->CastSpell(pUnit, GravityLapseDOT, true, 0, 0, m_caster->GetGUID());
                    GravityLapseSpellId++;
                }
            }
            m_caster->CastSpell(m_caster, GravityLapseChannel, true);
            break;
        }
        // Void Reaver: Knock Back
        case 25778:
        {
            if (!m_caster->CanHaveThreatList())
                return;

            m_caster->getThreatManager().modifyThreatPercent(unitTarget, -40.0f);
            break;
        }
        // Incite Chaos
        case 33676:
        {
            if(!m_caster->CanHaveThreatList())
                return;

            uint32 SummonCreatureID = 19300;

            std::list<HostileReference*> PlayerList = m_caster->getThreatManager().getThreatList();
            std::list<uint64> SummonedTriggerList;

            for(std::list<HostileReference*>::iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                Unit* pUnit = m_caster->GetMap()->GetUnit((*i)->getUnitGuid());
                if(!pUnit)
                    continue;

                if(pUnit->GetTypeId() != TYPEID_PLAYER)
                    continue;

                if(((Player*)pUnit)->isGameMaster())
                    continue;

                if(pUnit->IsInWorld() && pUnit->IsInMap(m_caster))
                {
                    if(Creature* summon = m_caster->SummonCreature(SummonCreatureID, pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 20000))
                    {
                        summon->CastSpell(pUnit, 33684, false);
                        SummonCreatureID++;
                        SummonedTriggerList.push_back(summon->GetGUID());
                    }
                }
            }
            for(std::list<uint64>::iterator i = SummonedTriggerList.begin(); i != SummonedTriggerList.end(); ++i)
            for(std::list<uint64>::iterator j = SummonedTriggerList.begin(); j != SummonedTriggerList.end(); ++j)
            {
                Unit* pSelf = m_caster->GetMap()->GetUnit((*i));
                Unit* pTarget = m_caster->GetMap()->GetUnit((*j));
                if(pTarget && pSelf && pTarget->GetTypeId() == TYPEID_UNIT && pSelf->GetTypeId() == TYPEID_UNIT)
                {
                    if(pTarget && pSelf && (pTarget->GetGUID() != pSelf->GetGUID()))
                    {
                        ((Creature*)pSelf)->AI()->AttackStart(pTarget);
                        pSelf->SetInCombatWith(pTarget);
                        pSelf->AddThreat(pTarget, 2000000.0);
                        ((Creature*)pTarget)->AI()->AttackStart(pSelf);
                        pTarget->SetInCombatWith(pSelf);
                        pTarget->AddThreat(pSelf, 2000000.0);
                    }
                }
            }
            break;
        }

        // PX-238 Winter Wondervolt TRAP
        case 26275:
        {
            if (unitTarget->HasAura(26272,0)
             || unitTarget->HasAura(26157,0)
             || unitTarget->HasAura(26273,0)
             || unitTarget->HasAura(26274,0))
                return;

            uint32 iTmpSpellId;

            switch (urand(0,3))
            {
                case 0:
                    iTmpSpellId = 26272;
                    break;
                case 1:
                    iTmpSpellId = 26157;
                    break;
                case 2:
                    iTmpSpellId = 26273;
                    break;
                case 3:
                    iTmpSpellId = 26274;
                    break;
            }

            unitTarget->CastSpell(unitTarget, iTmpSpellId, true);
            return;
        }

        // Bending Shinbone
        case 8856:
        {
            if (!itemTarget && m_caster->GetTypeId()!=TYPEID_PLAYER)
                return;

            uint32 spell_id = 0;
            switch (urand(1,5))
            {
                case 1:  spell_id = 8854; break;
                default: spell_id = 8855; break;
            }

            m_caster->CastSpell(m_caster,spell_id,true,NULL);
            return;
        }

        case 27090:
        {
            if (m_caster->HasAura(54771, 0))
                DoCreateItem(effIndex, 993187);
            else
                DoCreateItem(effIndex, 22018);

        }
        // Healthstone creating spells
        case  6201:
        case  6202:
        case  5699:
        case 11729:
        case 11730:
        case 27230:
        {
            uint32 itemtype;
            uint32 rank = 0;

            // imp healthstone rank 1
            if (unitTarget->HasAura(18692))
                rank = 1;

            // imp healthstone rank 2
            if (unitTarget->HasAura(18693))
                rank = 2;

            static uint32 const itypes[6][3] = {
                { 5512,19004,19005},                        // Minor Healthstone
                { 5511,19006,19007},                        // Lesser Healthstone
                { 5509,19008,19009},                        // Healthstone
                { 5510,19010,19011},                        // Greater Healthstone
                { 9421,19012,19013},                        // Major Healthstone
                {22103,22104,22105},                         // Master Healthstone
            };

            switch (GetSpellEntry()->Id)
            {
            case  6201: itemtype = itypes[0][rank]; break; // Minor Healthstone
            case  6202: itemtype = itypes[1][rank]; break; // Lesser Healthstone
            case  5699: itemtype = itypes[2][rank]; break; // Healthstone
            case 11729: itemtype = itypes[3][rank]; break; // Greater Healthstone
            case 11730: itemtype = itypes[4][rank]; break; // Major Healthstone
            case 27230: itemtype = itypes[5][rank]; break; // Master Healthstone
            default:
                return;
            }
            DoCreateItem(effIndex, itemtype);
            return;
        }
        // Brittle Armor - need remove one 24575 Brittle Armor aura
        case 24590:
            unitTarget->RemoveSingleAuraFromStack(24575, 0);
            unitTarget->RemoveSingleAuraFromStack(24575, 1);
            return;
        // Mercurial Shield - need remove one 26464 Mercurial Shield aura
        case 26465:
            unitTarget->RemoveSingleAuraFromStack(26464, 0);
            return;
        // Orb teleport spells
        case 25140:
        case 25143:
        case 25650:
        case 25652:
        case 29128:
        case 29129:
        case 35376:
        case 35727:
        {
            if (!unitTarget)
                return;

            uint32 spellid = 0;
            switch (GetSpellEntry()->Id)
            {
                case 25140: spellid =  32571; break;
                case 25143: spellid =  32572; break;
                case 25650: spellid =  30140; break;
                case 25652: spellid =  30141; break;
                case 29128: spellid =  32568; break;
                case 29129: spellid =  32569; break;
                case 35376: spellid =  25649; break;
                case 35727: spellid =  35730; break;
                default:
                    return;
            }

            unitTarget->CastSpell(unitTarget, spellid, false);
            return;
        }

        // Shadow Flame (All script effects, not just end ones to prevent player from dodging the last triggered spell)
        case 22539:
        case 22972:
        case 22975:
        case 22976:
        case 22977:
        case 22978:
        case 22979:
        case 22980:
        case 22981:
        case 22982:
        case 22983:
        case 22984:
        case 22985:
        {
            if (!unitTarget || !unitTarget->isAlive())
                return;

            // Onyxia Scale Cloak
            if (unitTarget->GetDummyAura(22683) || (m_caster->GetTypeId() == TYPEID_UNIT && ((Creature*)m_caster)->ModIsInHeroicRaid))
                return;

            // Shadow Flame
            m_caster->CastSpell(unitTarget, 22682, true);
            return;
        }
        break;
        case 24194:                                 // Uther's Tribute
        case 24195:                                 // Grom's Tribute
        {
            if (m_caster->GetTypeId() != TYPEID_PLAYER)
                return;

            uint8 race = m_caster->GetRace();
            uint32 spellId = 0;

            switch (GetSpellEntry()->Id)
            {
                case 24194:
                    switch (race)
                    {
                        case RACE_HUMAN:            spellId = 24105; break;
                        case RACE_DWARF:            spellId = 24107; break;
                        case RACE_NIGHTELF:         spellId = 24108; break;
                        case RACE_GNOME:            spellId = 24106; break;
                    }
                break;
                case 24195:
                    switch (race)
                    {
                        case RACE_ORC:              spellId = 24104; break;
                        case RACE_UNDEAD_PLAYER:    spellId = 24103; break;
                        case RACE_TAUREN:           spellId = 24102; break;
                        case RACE_TROLL:            spellId = 24101; break;
                    }
                break;
            }

            if (spellId)
                m_caster->CastSpell(m_caster, spellId, true);

            return;
        }
        // Hallowed Wand
        // Random Costume
        case 24720:
        {
            if (!unitTarget || !unitTarget->isAlive())
                return;

            uint32 spellId;
            switch ((uint32)rand32()%7)
            {
                case 0: spellId = 24717; break; // Pirate Costume
                case 1: spellId = 24741; break; // Wisp Costume
                case 2: spellId = 24724; break; // Skeleton Costume
                case 3: spellId = 24719; break; // Leper Gnome Costume
                case 4: spellId = 24718; break; // Ninja Costume
                case 5: spellId = 24737; break; // Ghost Costume
                case 6: spellId = 24733; break; // Bat Costume
            }
            m_caster->CastSpell(unitTarget, spellId, true);
        }
        break;
        // Pirate Costume
        case 24717:
        {
            if (!unitTarget || !unitTarget->isAlive())
                return;

            if (unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24708,true);
            else
                m_caster->CastSpell(unitTarget,24709,true);
        }
        break;
        // Ninja Costume
        case 24718:
        {
            if (!unitTarget || !unitTarget->isAlive())
                return;

            if (unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24711,true);
            else
                m_caster->CastSpell(unitTarget,24710,true);
        }
        break;
        // Leper Gnome Costume
        case 24719:
        {
            if (!unitTarget || !unitTarget->isAlive())
                return;

            if (unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24712,true);
            else
                m_caster->CastSpell(unitTarget,24713,true);
        }
        break;
        // Ghost Costume
        case 24737:
        {
            if (!unitTarget || !unitTarget->isAlive())
                return;

            if (unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24735,true);
            else
                m_caster->CastSpell(unitTarget,24736,true);
        }
        break;
        case 26218:
        {
            if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            uint32 spells[3] = {26206, 26207, 45036};

            m_caster->CastSpell(unitTarget, spells[urand(0, 2)], true);
            return;
        }
        // Summon Black Qiraji Battle Tank
        case 26656:
        {
            if (!unitTarget)
                return;

            bool dismountOnly = false;

            if (unitTarget->HasAura(25863) || unitTarget->HasAura(26655))
                dismountOnly = true;

            // Prevent stacking of mounts
            unitTarget->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

            if (dismountOnly)
                return;

            // Two separate mounts depending on area id (allows use both in and out of specific instance)
            uint32 areaid = unitTarget->GetTypeId() == TYPEID_PLAYER ? ((Player*)unitTarget)->GetCachedArea() : unitTarget->GetAreaId();
            if (areaid == 3428)
                unitTarget->CastSpell(unitTarget, 25863, false);
            else
                unitTarget->CastSpell(unitTarget, 26655, false);
            break;
        }
        // Piccolo of the Flaming Fire
        case 17512:
        {
            if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;
            unitTarget->HandleEmoteCommand(EMOTE_STATE_DANCE);
            break;
        }
        // Netherbloom
        case 28702:
        {
            if (!unitTarget)
                return;
            // 25% chance of casting a random buff
            if (roll_chance_i(75))
                return;

            // triggered spells are 28703 to 28707
            // Note: some sources say, that there was the possibility of
            //       receiving a debuff. However, this seems to be removed by a patch.
            const uint32 spellid = 28703;

            // don't overwrite an existing aura
            for (uint8 i=0; i<5; i++)
                if (unitTarget->HasAura(spellid+i, 0))
                    return;
            unitTarget->CastSpell(unitTarget, spellid+urand(0, 4), true);
            break;
        }

        // Nightmare Vine
        case 28720:
        {
            if (!unitTarget)
                return;
            // 25% chance of casting Nightmare Pollen
            if (roll_chance_i(75))
                return;
            unitTarget->CastSpell(unitTarget, 28721, true);
            break;
        }

        // Mirren's Drinking Hat
        case 29830:
        {
            uint32 item = 0;
            switch (urand(1,6))
            {
                case 1: case 2: case 3: item = 23584; break;// Loch Modan Lager
                case 4: case 5:         item = 23585; break;// Stouthammer Lite
                case 6:                 item = 23586; break;// Aerie Peak Pale Ale
            }
            if (item)
                DoCreateItem(effIndex,item);
            break;
        }
        // Improved Sprint
        case 30918:
        {
            // Removes snares and roots.
            uint32 mechanic_mask = (1<<MECHANIC_ROOT) | (1<<MECHANIC_SNARE);
            Unit::AuraMap& Auras = unitTarget->GetAuras();
            for (Unit::AuraMap::iterator iter = Auras.begin(), next; iter != Auras.end(); iter = next)
            {
                next = iter;
                ++next;
                Aura *aur = iter->second;
                if (!aur->IsPositive())             //only remove negative spells
                {
                    // check for mechanic mask
                    if (SpellMgr::GetSpellMechanicMask(aur->GetSpellProto()) & mechanic_mask)
                    {
                        unitTarget->RemoveAurasDueToSpell(aur->GetId());
                        if (Auras.empty())
                            break;
                        else
                            next = Auras.begin();
                    }
                    else if (SpellMgr::GetEffectMechanicMask(aur->GetSpellProto(), aur->GetEffIndex()) & mechanic_mask)
                    {
                        unitTarget->RemoveAura(aur->GetId(), aur->GetEffIndex());
                        if (Auras.empty())
                            break;
                        else
                            next = Auras.begin();
                    }
                }
            }
            break;
        }
        // Gruul's Ground Slam
        case 33525:
        {
            if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            // knock back for random distance with random angle
            unitTarget->KnockBack(frand(0.0f, 2*M_PI), frand(2.0f, 40.0f), 15.0f);
            break;
        }
        // Gruul's shatter
        case 33654:
        {
            if (!unitTarget)
                return;

            //Remove Stoned
            unitTarget->RemoveAurasDueToSpell(33652);

            // Only player cast this
            if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            unitTarget->CastSpell(unitTarget, 33671, true, 0, 0, m_caster->GetGUID());
            break;
        }
        case 41055:                                 // Copy Weapon
        {
            if (m_caster->GetTypeId() != TYPEID_UNIT || !unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Item* pItem = ((Player*)unitTarget)->GetWeaponForAttack(BASE_ATTACK))
            {
                if (const ItemEntry *dbcitem = sItemStore.LookupEntry(pItem->GetProto()->ItemId))
                {
                    m_caster->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 0, dbcitem->ID);

                    // Unclear what this spell should do
                    unitTarget->CastSpell(m_caster, GetSpellEntry()->EffectBasePoints[effIndex], true);
                }
            }
            return;
        }
        // Goblin Weather Machine
        case 46203:
        {
            if (!unitTarget)
                return;

            uint32 spellId;
            switch (urand(0,3))
            {
                case 0:
                    spellId=46740;
                    break;
                case 1:
                    spellId=46739;
                    break;
                case 2:
                    spellId=46738;
                    break;
                case 3:
                    spellId=46736;
                    break;
            }
            unitTarget->CastSpell(unitTarget, spellId, true);
            break;
        }
        // Chilling Burst
        case 46541:
        {
            if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            int32 ChillDamage = urand(490, 670);
            Aura* ChillingAura = m_caster->GetAuraByCasterSpell(46542, m_caster->GetGUID());

            unitTarget->CastCustomSpell(unitTarget, 46576, &ChillDamage, NULL, NULL, true, 0, ChillingAura, m_caster->GetGUID());
            break;
        }
        // Headless Horseman's Mount
        case 48025:
        {
                if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                    return;

                //unitTarget->Mount(22653);

                if (GetVirtualMapForMapAndZone(unitTarget->GetMapId(), ((Player*)unitTarget)->GetCachedZone()) != 530)
                {
                    switch (((Player*)unitTarget)->GetBaseSkillValue(SKILL_RIDING))
                    {
                    case 75: unitTarget->CastSpell(unitTarget, 51621, true); break;
                    case 150: case 225: case 300: unitTarget->CastSpell(unitTarget, 48024, true); break;
                    default: break;
                    }
                }else
                {
                    switch (((Player*)unitTarget)->GetBaseSkillValue(SKILL_RIDING))
                    {
                    case 75: unitTarget->CastSpell(unitTarget, 51621, true); break;
                    case 150: unitTarget->CastSpell(unitTarget, 48024, true); break;
                    case 225: unitTarget->CastSpell(unitTarget, 51617, true); break;
                    case 300: unitTarget->CastSpell(unitTarget, 48023, true); break;
                    default: break;
                    }
                }
                break;
        }
        case 47977:                                     // Magic Broom
        {
            if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            if (GetVirtualMapForMapAndZone(unitTarget->GetMapId(), ((Player*)unitTarget)->GetCachedZone()) != 530)
            {
                switch (((Player*)unitTarget)->GetBaseSkillValue(SKILL_RIDING))
                {
                case 75: unitTarget->CastSpell(unitTarget, 42681, true); break;
                case 150: case 225: case 300: unitTarget->CastSpell(unitTarget, 42683, true); break;
                default: break;
                }
            }
            else
            {
                switch (((Player*)unitTarget)->GetBaseSkillValue(SKILL_RIDING))
                {
                case 75: unitTarget->CastSpell(unitTarget, 42681, true); break;
                case 150: unitTarget->CastSpell(unitTarget, 42684, true); break;
                case 225: unitTarget->CastSpell(unitTarget, 42673, true); break;
                case 300: unitTarget->CastSpell(unitTarget, 42679, true); break;
                default: break;
                }
            }
            break;
        }
        // Fixated
        case 40893:
        {
            m_caster->CastSpell(m_caster, 40893, false);
        }
        // Summon Shadowfiends
        case 39649:
        {
            uint8 random = urand(8, 12);
            for (uint8 i = 0; i < random; ++i)
            {
                m_caster->CastSpell(m_caster, 41159, true);
            }
            break;
        }
        // Green Helper Box, Red Helper Box, Snowman Kit, Jingling Bell
        case 26532:
        case 26541:
        case 26469:
        case 26528:
        {
            uint32 summon_spell_entry = 0;
            switch (GetSpellEntry()->Id)
            {
                case 26532: //green
                    summon_spell_entry = 26533;
                    break;
                case 26541: //red
                    summon_spell_entry = 26536;
                    break;
                case 26469: //snowman
                    summon_spell_entry = 26045;
                    break;
                case 26528: //reindeer
                    summon_spell_entry = 26529;
                    break;
            }

            if (!summon_spell_entry)
                return;

            m_caster->CastSpell(m_caster, summon_spell_entry, false);

            break;
        }
        case 42924:
        {
            Aura* aur = m_caster->GetAura(42924, 0);
            if(aur && aur->GetStackAmount() < 4)
                break;

            if(aur && aur->GetStackAmount() == 11)
            {
                m_caster->CastSpell(m_caster, 42936, true);
                break;
            }

            if(m_caster->HasAura(42993, 2))
            {
                m_caster->RemoveAurasDueToSpell(42993);
                m_caster->CastSpell(m_caster, 42994, true);
            }
            else if(m_caster->HasAura(42992, 2))
            {
                m_caster->RemoveAurasDueToSpell(42992);
                m_caster->CastSpell(m_caster, 42993, true);
            }
            else if(m_caster->HasAura(43310, 2))
            {
                m_caster->RemoveAurasDueToSpell(43310);
                m_caster->CastSpell(m_caster, 42992, true);
            }
            break;
        }
        case 51508:
        {
            m_caster->HandleEmoteCommand(EMOTE_STATE_DANCE);
            break;
        }
        case 42436:
        {
            switch(unitTarget->GetEntry())
            {
            case 24108:
                //((Player*)m_caster)->CastCreatureOrGO(unitTarget->GetEntry(), unitTarget->GetGUID(), 0);
                m_caster->CastSpell(m_caster, 47173, true);
                break;
            case 23709:
                m_caster->CastSpell(m_caster, 47171, true);
                m_caster->Kill(unitTarget, false);
                break;
            default:
                break;
            }
            break;
        }
        case 40638: //Willy Focus
        {
            m_caster->CastSpell(unitTarget, damage ,false);
            break;
        }
    }

    if (!unitTarget)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell %u in EffectScriptEffect does not have unitTarget", GetSpellEntry()->Id);
        return;
    }
    if (!unitTarget->isAlive())
    {
        if (GetSpellEntry()->Id == 20271) // player casted judgement, don't even log as this is caused by delays
            return;
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell %u in EffectScriptEffect has dead target", GetSpellEntry()->Id);
        return;
    }

    switch (GetSpellEntry()->Id)
    {
        // Dreaming Glory
        case 28698:
            unitTarget->CastSpell(unitTarget, 28694, true);
            return;
        // Needle Spine
        //case 39835: unitTarget->CastSpell(unitTarget, 39968, true); break;
        // Draw Soul
        case 40904:
            unitTarget->CastSpell(m_caster, 40903, true);
            return;
        // Flame Crash
        case 41126:
            unitTarget->CastSpell(unitTarget, 41131, true, NULL, NULL, m_originalCasterGUID);
            return;
        case 41931:
        {
            int bag=19;
            int slot=0;
            Item* item = NULL;

            while (bag < 256)
            {
                item = ((Player*)m_caster)->GetItemByPos(bag,slot);
                if (item && item->GetEntry() == 38587) break;
                slot++;
                if (slot == 39)
                {
                    slot = 0;
                    bag++;
                }
            }

            if (bag < 256)
            {
                if (((Player*)m_caster)->GetItemByPos(bag,slot)->GetCount() == 1) ((Player*)m_caster)->RemoveItem(bag,slot,true);
                else ((Player*)m_caster)->GetItemByPos(bag,slot)->SetCount(((Player*)m_caster)->GetItemByPos(bag,slot)->GetCount()-1);
                // Spell 42518 (Braufest - Gratisprobe des Braufest herstellen)
                m_caster->CastSpell(m_caster,42518,true);
            }

            return;
        }
        case 42578:                                 // Cannon Blast
        {
            if (!unitTarget)
                return;

            int32 basePoints = m_spellInfo->CalculateSimpleValue(effIndex);
            m_caster->CastCustomSpell(unitTarget, 42576, &basePoints, nullptr, nullptr, true);
            return;
        }
        // Force Cast - Portal Effect: Sunwell Isle
        case 44876:
        {
            unitTarget->CastSpell(unitTarget, 44870, true);
            return;
        }
        //Brutallus - Burn
        case 45141: case 45151:
        {
            //Workaround for Range ... should be global for every ScriptEffect
            float radius = SpellMgr::GetSpellRadius(GetSpellEntry(), effIndex, false);
            //if (unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER && unitTarget->GetDistance(m_caster) <= radius && !unitTarget->HasAura(46394,0) && unitTarget != m_caster)
            if(!unitTarget->HasAura(46394, 0) && unitTarget != m_caster)
                unitTarget->CastSpell(unitTarget,46394,true, 0, 0, m_originalCasterGUID);

            return;
        }
        // spell of Brutallus - Stomp
        case 45185:
        {
            if (unitTarget->HasAura(46394, 0)) // spell of Brutallus - Burn
                unitTarget->RemoveAurasDueToSpell(46394);
            return;
        }
        case 45206:                                 // Copy Off-hand Weapon
        {
            if (m_caster->GetTypeId() != TYPEID_UNIT || !unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            if (Item* pItem = ((Player*)unitTarget)->GetWeaponForAttack(OFF_ATTACK))
            {
                if (const ItemEntry *dbcitem = sItemStore.LookupEntry(pItem->GetProto()->ItemId))
                {
                    m_caster->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1, dbcitem->ID);

                    // Unclear what this spell should do
                    unitTarget->CastSpell(m_caster, GetSpellEntry()->EffectBasePoints[effIndex], true);
                }
            }
            return;
        }
        // Negative Energy - Entropius
        case 46289:
        {
            if(unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;
            int32 damage = irand(1885, 2115);
            m_caster->CastCustomSpell(unitTarget, 46285, &damage, 0, 0, true, 0, 0, m_caster->GetGUID());
            damage /= 2;
            if(Unit* target_2 = unitTarget->ToPlayer()->GetNextRandomRaidMember(10.0f, true))
            {
                unitTarget->CastCustomSpell(target_2, 46285, &damage, 0, 0, true, 0, 0, m_caster->GetGUID());
                damage /= 2;
                if(Unit* target_3 = target_2->ToPlayer()->GetNextRandomRaidMember(10.0f, true))
                    target_2->CastCustomSpell(target_3, 46285, &damage, 0, 0, true, 0, 0, m_caster->GetGUID());
            }
            return;
        }
        //5,000 Gold
        case 46642:
        {
            //if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            //    ((Player*)unitTarget)->ModifyMoney(50000000);
            return;
        }
        case 48917:
        {
            if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            // Male Shadowy Disguise / Female Shadowy Disguise
            unitTarget->CastSpell(unitTarget, unitTarget->GetGender() == GENDER_MALE ? 38080 : 38081, true);
            // Shadowy Disguise
            unitTarget->CastSpell(unitTarget, 32756, true);
            return;
        }
        // Listening to Music (Parent)
        case 50499:
        {
            if (unitTarget->GetTypeId() != TYPEID_PLAYER)
                return;

            unitTarget->CastSpell(unitTarget, 50493, true);
            return;
        }
        // Cleansing Flames
        case 29137: // Stormwind
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check horde only quest status: Stealing Stormwind's Flame
                if (tmpPl->GetTeam() == HORDE)
                {
                    if (tmpPl->GetQuestStatus(9330) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(9330))
                    {
                        if (!tmpPl->HasItemCount(23182, 1, true))
                            tmpPl->CastSpell(tmpPl, 29101, true);
                    }
                }
            }
            return;
        }
        case 29126: // Darnasus
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check horde only quest status: Stealing Darnasus's Flame
                if (tmpPl->GetTeam() == HORDE)
                    if (tmpPl->GetQuestStatus(9332) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(9332))
                        if (!tmpPl->HasItemCount(23184, 1, true))
                            tmpPl->CastSpell(tmpPl, 29099, true);
            }
            return;
        }
        case 29135: // Ironforge
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check horde only quest status: Stealing Ironforge's Flame
                if (tmpPl->GetTeam() == HORDE)
                    if (tmpPl->GetQuestStatus(9331) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(9331))
                        if (!tmpPl->HasItemCount(23183, 1, true))
                            tmpPl->CastSpell(tmpPl, 29102, true);
            }
            return;
        }
        case 29136: // Orgrimmar
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check alliance only quest status: Stealing Orgrimmar's Flame
                if (tmpPl->GetTeam() == ALLIANCE)
                    if (tmpPl->GetQuestStatus(9324) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(9324))
                        if (!tmpPl->HasItemCount(23179, 1, true))
                            tmpPl->CastSpell(tmpPl, 29130, true);
            }
            return;
        }
        case 46672: // Silvermoon
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check alliance only quest status: Stealing Silvermoon's Flame
                if (tmpPl->GetTeam() == ALLIANCE)
                    if (tmpPl->GetQuestStatus(11935) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(11935))
                        if (!tmpPl->HasItemCount(35568, 1, true))
                            tmpPl->CastSpell(tmpPl, 46689, true);
            }
            return;
        }
        case 29138: // Thunder Bluff
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check alliance only quest status: Stealing Thunder Bluff's Flame
                if (tmpPl->GetTeam() == ALLIANCE)
                    if (tmpPl->GetQuestStatus(9325) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(9325))
                        if (!tmpPl->HasItemCount(23180, 1, true))
                            tmpPl->CastSpell(tmpPl, 29132, true);
            }
            return;
        }
        case 46671: // Exodar
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check horde only quest status: Stealing the Exodar's Flame
                if (tmpPl->GetTeam() == HORDE)
                    if (tmpPl->GetQuestStatus(11933) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(11933))
                        if (!tmpPl->HasItemCount(35569, 1, true))
                            tmpPl->CastSpell(tmpPl, 46690, true);
            }
            return;
        }
        case 29139: // Undercity
        {
            if (unitTarget->GetTypeId() == TYPEID_PLAYER)
            {
                Player * tmpPl = (Player*)unitTarget;
                // check alliance only quest status: Stealing the Undercity's Flame
                if (tmpPl->GetTeam() == ALLIANCE)
                    if (tmpPl->GetQuestStatus(9326) != QUEST_STATUS_COMPLETE && !tmpPl->GetQuestRewardStatus(9326))
                        if (!tmpPl->HasItemCount(23181, 1, true))
                            tmpPl->CastSpell(tmpPl, 29133, true);
            }
            return;
        }
        case 38650: // Rancid Mushroom
            m_caster->SummonCreature(22250, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetOrientation(),
                    TEMPSUMMON_DEAD_DESPAWN, 30000);
            break;
        case 45235: // Eredar Twins: Blaze
            unitTarget->CastSpell(unitTarget, 45236, true, NULL, NULL, m_caster->GetGUID());
            break;
        case 30541: // Magtheridon's Blaze
            unitTarget->CastSpell(unitTarget, 30542, true, NULL, NULL, m_caster->GetGUID());
            break;
        case 36153: // Soul Bind
            unitTarget->CastSpell(m_caster, 36141, true);
            break;
    }

    if (GetSpellEntry()->SpellFamilyName == SPELLFAMILY_PALADIN)
    {
        switch (GetSpellEntry()->SpellFamilyFlags)
        {
            // Judgement
            case 0x800000:
            {
                uint32 spellId2 = 0;

                // all seals have aura dummy
                Unit::AuraList const& m_dummyAuras = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                for (Unit::AuraList::const_iterator itr = m_dummyAuras.begin(); itr != m_dummyAuras.end(); ++itr)
                {
                    SpellEntry const *spellInfo = (*itr)->GetSpellProto();

                    // search seal (all seals have judgement's aura dummy spell id in 2 effect
                    if (!spellInfo || !SpellMgr::IsSealSpell((*itr)->GetSpellProto()) || (*itr)->GetEffIndex() != 2)
                        continue;

                    // seal is delayed
                    if ((*itr)->special_flag == AURA_SPECIAL_SEALTWIST)
                        continue;

                    // must be calculated base at raw base points in spell proto, GetModifier()->m_value for S.Righteousness modified by SPELLMOD_DAMAGE
                    spellId2 = (*itr)->GetSpellProto()->EffectBasePoints[2]+1;

                    if (spellId2 <= 1)
                        continue;

                    // found, remove seal
                    m_caster->RemoveAurasDueToSpell((*itr)->GetId());

                    // Sanctified Judgement
                    Unit::AuraList const& m_auras = m_caster->GetAurasByType(SPELL_AURA_DUMMY);
                    for (Unit::AuraList::const_iterator i = m_auras.begin(); i != m_auras.end(); ++i)
                    {
                        if ((*i)->GetSpellProto()->SpellIconID == 205 && (*i)->GetSpellProto()->Attributes == 0x01D0LL)
                        {
                            int32 chance = (*i)->GetModifier()->m_amount;
                            if (roll_chance_i(chance))
                            {
                                int32 mana = SpellMgr::CalculatePowerCost(spellInfo, m_caster, SPELL_SCHOOL_MASK_NONE, this);
                                mana = int32(mana* 0.8f);
                                m_caster->CastCustomSpell(m_caster,31930,&mana,NULL,NULL,true,NULL,*i);
                            }
                            break;
                        }
                    }

                    break;
                }

                m_caster->CastSpell(unitTarget,spellId2,false);
                return;
            }
        }
    }

    // normal DB scripted effect
    sLog.outDebug("Spell ScriptStart spellid %u in EffectScriptEffect ", GetSpellEntry()->Id);
    m_caster->GetMap()->ScriptsStart(sSpellScripts, GetSpellEntry()->Id, m_caster, unitTarget);
}

void Spell::EffectSanctuary(uint32 /*i*/)
{
    if (!unitTarget)
        return;

    std::list<Unit*> targets;

    Hellground::AnyUnfriendlyUnitInObjectRangeCheck u_check(unitTarget, m_caster->GetMap()->GetVisibilityDistance());
    Hellground::UnitListSearcher<Hellground::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);

    Cell::VisitAllObjects(unitTarget, searcher, m_caster->GetMap()->GetVisibilityDistance());

    for (std::list<Unit*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
    {
        if (!(*iter)->HasUnitState(UNIT_STAT_CASTING))
            continue;

        for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        {
            if ((*iter)->m_currentSpells[i]
                && (*iter)->m_currentSpells[i]->m_targets.getUnitTargetGUID() == unitTarget->GetGUID())
            {
                (*iter)->InterruptSpell(i, false);
            }
        }
    }

    // comment from WoWHead on mage's Invisibility (32612 called by 66): 
    // "I've never been able to eat/drink while invisible on boss fights. It gives me the error message saying I'm still in combat."
    unitTarget->CombatStopExceptDoZoneInCombat();

    // Vanish allows to remove all threat and cast regular stealth so other spells can be used
    if (GetSpellEntry()->SpellFamilyName == SPELLFAMILY_ROGUE && (GetSpellEntry()->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_VANISH))
    {
        ((Player *)m_caster)->RemoveSpellsCausingAura(SPELL_AURA_MOD_ROOT);

        // makes spells cast before this time fizzle (only Vanish)
        unitTarget->m_lastSanctuaryTime = WorldTimer::getMSTime();
    }
}

void Spell::EffectAddComboPoints(uint32 /*i*/)
{
    if (!unitTarget)
        return;

    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (damage <= 0)
        return;

    ((Player*)m_caster)->AddComboPoints(unitTarget, damage);
}

void Spell::EffectDuel(uint32 i)
{
    if (sWorld.getConfig(CONFIG_DISABLE_DUEL))
        return;

    if (!m_caster || !unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *caster = (Player*)m_caster;
    Player *target = (Player*)unitTarget;

    // caster or target already have requested duel
    if (caster->duel || target->duel || !target->GetSocial() || target->GetSocial()->HasIgnore(caster->GetGUIDLow()))
        return;

    // Players can only fight a duel in zones with this flag
    if (!sWorld.isPvPArea(caster->GetCachedArea()))
    {
        AreaTableEntry const* casterAreaEntry = GetAreaEntryByAreaID(caster->GetCachedArea());
        if (casterAreaEntry && !(casterAreaEntry->flags & AREA_FLAG_ALLOW_DUELS))
        {
            SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
            return;
        }

        AreaTableEntry const* targetAreaEntry = GetAreaEntryByAreaID(target->GetCachedArea());
        if (targetAreaEntry && !(targetAreaEntry->flags & AREA_FLAG_ALLOW_DUELS))
        {
            SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
            return;
        }
    }

    //CREATE DUEL FLAG OBJECT
    GameObject* pGameObj = new GameObject;

    uint32 gameobject_id = GetSpellEntry()->EffectMiscValue[i];

    Map *map = m_caster->GetMap();
    if (!pGameObj->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), gameobject_id, map,
        m_caster->GetPositionX()+(unitTarget->GetPositionX()-m_caster->GetPositionX())/2 ,
        m_caster->GetPositionY()+(unitTarget->GetPositionY()-m_caster->GetPositionY())/2 ,
        m_caster->GetPositionZ(),
        m_caster->GetOrientation(), 0, 0, 0, 0, 0, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, m_caster->getFaction());
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->GetLevel()+1);
    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(GetSpellEntry()->Id);

    m_caster->AddGameObject(pGameObj);
    map->Add(pGameObj);
    //END

    // Send request
    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << pGameObj->GetGUID();
    data << caster->GetGUID();
    caster->SendPacketToSelf(&data);
    target->SendPacketToSelf(&data);

    // create duel-info
    DuelInfo *duel   = new DuelInfo;
    duel->initiator  = caster;
    duel->opponent   = target;
    duel->startTime  = 0;
    duel->startTimer = 0;
    caster->duel     = duel;

    DuelInfo *duel2   = new DuelInfo;
    duel2->initiator  = caster;
    duel2->opponent   = caster;
    duel2->startTime  = 0;
    duel2->startTimer = 0;
    target->duel      = duel2;

    caster->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
    target->SetUInt64Value(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
}

#define HEARTHSTONE_SPELL 8690
#define HEARTHSTONE_ITEM 6948

void Spell::EffectStuck(uint32 /*i*/)
{
    if (!unitTarget)
        return;

    Player* pTarget = unitTarget->ToPlayer();

    if (!pTarget || !sWorld.getConfig(CONFIG_CAST_UNSTUCK))
        return;

    sLog.outDetail("Player %s (guid %u) used auto-unstuck future at map %u (%f, %f, %f)", pTarget->GetName(), pTarget->GetGUIDLow(), m_caster->GetMapId(), m_caster->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ());

    // player stucked on arena or bg just should leave ;]
    if (pTarget->IsTaxiFlying() || pTarget->InBattleGroundOrArena())
        return;

    // if player isn't alive repop him on nearest graveyard (for stucking while in ghost)
    if (pTarget->isAlive())
    {
        // if player hasn't cooldown on HearthStone and have in bags then use him
        // otherwise teleport to player start location
        if (!pTarget->HasSpellCooldown(HEARTHSTONE_SPELL, 1176) && pTarget->HasItemCount(HEARTHSTONE_ITEM, 1))
        {
            pTarget->CastSpell(pTarget, HEARTHSTONE_SPELL, true);
            pTarget->AddSpellItemCooldown(HEARTHSTONE_SPELL, HEARTHSTONE_ITEM, 1176, time(NULL) + HOUR);
        }
        else
            if (PlayerInfo const * tmpPlInfo = sObjectMgr.GetPlayerInfo(pTarget->GetRace(), pTarget->GetClass()))
                pTarget->TeleportTo(tmpPlInfo->mapId, tmpPlInfo->positionX, tmpPlInfo->positionY, tmpPlInfo->positionZ, 0.0f);
    }
    else
    {
        if (!pTarget->GetCorpse())
            pTarget->BuildPlayerRepop();
        pTarget->TeleportToNearestGraveyard();
    }

    pTarget->SaveToDB(); // just in case
}

void Spell::EffectSummonPlayer(uint32 /*i*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    // Evil Twin (ignore player summon, but hide this for summoner)
    if (unitTarget->GetDummyAura(23445))
        return;

    if (!unitTarget->ToPlayer()->CanBeSummonedBy(m_caster->ToPlayer()))
        return;

    const Player * pCaster = m_caster->ToPlayer();
    float x,y,z;
    m_caster->GetPosition(x, y, z);

    unitTarget->ToPlayer()->SetSummonPoint(m_caster->GetMapId(),x,y,z);

    uint32 zoneid = pCaster ? pCaster->GetCachedZone() : m_caster->GetZoneId();

    WorldPacket data(SMSG_SUMMON_REQUEST, 8+4+4);
    data << uint64(m_caster->GetGUID());                    // summoner guid
    data << uint32(zoneid);                                 // summoner zone
    data << uint32(MAX_PLAYER_SUMMON_DELAY*1000);           // auto decline after msecs
    unitTarget->ToPlayer()->SendPacketToSelf(&data);
}

static ScriptInfo generateActivateCommand()
{
    ScriptInfo si;
    si.command = SCRIPT_COMMAND_ACTIVATE_OBJECT;
    return si;
}

void Spell::EffectActivateObject(uint32 effect_idx)
{
    if (!gameObjTarget)
        return;

    static ScriptInfo activateCommand = generateActivateCommand();

    int32 delay_secs = GetSpellEntry()->EffectMiscValue[effect_idx];

    gameObjTarget->GetMap()->ScriptCommandStart(activateCommand, delay_secs, m_caster, gameObjTarget);
}

void Spell::EffectSummonTotem(uint32 i)
{
    uint8 slot = 0;
    switch (GetSpellEntry()->EffectMiscValueB[i])
    {
        case SUMMON_TYPE_TOTEM_SLOT1: slot = 0; break;
        case SUMMON_TYPE_TOTEM_SLOT2: slot = 1; break;
        case SUMMON_TYPE_TOTEM_SLOT3: slot = 2; break;
        case SUMMON_TYPE_TOTEM_SLOT4: slot = 3; break;
        // Battle standard case
        case SUMMON_TYPE_TOTEM:       slot = 254; break;
        // jewelery statue case, like totem without slot
        case SUMMON_TYPE_GUARDIAN:    slot = 255; break;
        default: return;
    }

    if (slot < MAX_TOTEM)
    {
        uint64 guid = m_caster->m_TotemSlot[slot];
        if (guid != 0)
        {
            Creature *OldTotem = m_caster->GetMap()->GetCreature(guid);
            if (OldTotem && OldTotem->isTotem())
                ((Totem*)OldTotem)->UnSummon();
        }
    }

    PlayerTeam team = TEAM_NONE;
    if (m_caster->GetTypeId()==TYPEID_PLAYER)
        team = ((Player*)m_caster)->GetTeam();

    Position dest;
    dest.x = m_targets.m_destX;
    dest.y = m_targets.m_destY;
    dest.z = m_targets.m_destZ;

    Totem* pTotem = new Totem;
    if (!pTotem->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT), m_caster->GetMap(), GetSpellEntry()->EffectMiscValue[i], team, dest.x, dest.y, dest.z, m_caster->GetOrientation()))
    {
        delete pTotem;
        return;
    }

    if (slot < MAX_TOTEM)
        m_caster->m_TotemSlot[slot] = pTotem->GetGUID();

    pTotem->SetOwner(m_caster->GetGUID());
    pTotem->SetTypeBySummonSpell(GetSpellEntry());              // must be after Create call where m_spells initilized

    int32 duration=SpellMgr::GetSpellDuration(GetSpellEntry());
    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(GetSpellEntry()->Id,SPELLMOD_DURATION, duration);

    pTotem->SetDuration(duration);

    if (damage)                                             // if not spell info, DB values used
    {
        pTotem->SetMaxHealth(damage);
        pTotem->SetHealth(damage);
    }

    pTotem->SetUInt32Value(UNIT_CREATED_BY_SPELL,GetSpellEntry()->Id);
    pTotem->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_PVP_ATTACKABLE);

    pTotem->ApplySpellImmune(GetSpellEntry()->Id,IMMUNITY_STATE,SPELL_AURA_MOD_FEAR,true);
    pTotem->ApplySpellImmune(GetSpellEntry()->Id,IMMUNITY_STATE,SPELL_AURA_TRANSFORM,true);

    if (slot < MAX_TOTEM && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_TOTEM_CREATED, 17);
        data << uint8(slot);
        data << uint64(pTotem->GetGUID());
        data << uint32(duration);
        data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));
        ((Player*)m_caster)->SendPacketToSelf(&data);
    }

    pTotem->Summon(m_caster);
}

void Spell::EffectEnchantHeldItem(uint32 i)
{
    // this is only item spell effect applied to main-hand weapon of target player (players in area)
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* item_owner = (Player*)unitTarget;
    Item* item = item_owner->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    if (!item)
        return;

    // must be equipped
    if (!item ->IsEquipped())
        return;

    if (GetSpellEntry()->EffectMiscValue[i])
    {
        uint32 enchant_id = GetSpellEntry()->EffectMiscValue[i];
        int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());          //Try duration index first ..
        if (!duration)
            duration = damage;//+1;            //Base points after ..
        if (!duration)
            duration = 10;                                  //10 seconds for enchants which don't have listed duration

        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!pEnchant)
            return;

        // Always go to temp enchantment slot
        EnchantmentSlot slot = TEMP_ENCHANTMENT_SLOT;

        // Enchantment will not be applied if a different one already exists
        if (item->GetEnchantmentId(slot) && item->GetEnchantmentId(slot) != enchant_id)
            return;

        // Apply the temporary enchantment
        item->SetEnchantment(slot, enchant_id, duration*1000, 0);
        item_owner->ApplyEnchantment(item,slot,true);
    }
}

void Spell::EffectDisEnchant(uint32 /*i*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;

    if (!itemTarget)
        return;

    // raid chest loot case
	if (sWorld.isEasyRealm())
	{
		auto rcloot = p_caster->raidChestLoot.find(itemTarget->GetGUIDLow());
		if (rcloot != p_caster->raidChestLoot.end())
		{
			uint32 item = 0;

			switch (rcloot->second)
			{
				//case RARE_RAID_CHEST:
				//    item = RARE_KEY_FRAGMENT; break;
			case EPIC_RAID_CHEST:
				item = EPIC_KEY_FRAGMENT; break;
			case LEGENDARY_RAID_CHEST:
				item = LEGENDARY_KEY_FRAGMENT; break;
			}

			if (!item)
			{
				sLog.outLog(LOG_CRITICAL, "RaidChest disenchant NULL item Player %s (%u) from %u", p_caster->GetName(), p_caster->GetGUIDLow(), item);
				return;
			}

			if (p_caster->GiveItem(item, 1))
			{
				p_caster->raidChestLoot.erase(p_caster->raidChestLoot.find(itemTarget->GetGUIDLow()));
				RealmDataDatabase.PExecute("UPDATE character_raidchest SET removed = 1 WHERE item_guid = '%llu'", itemTarget->GetGUIDLow());

				uint32 count = 1;
				p_caster->DestroyItemCount(itemTarget, count, true, "RAID_CHEST_LOOT_DISENCHANT");
				m_CastItem = NULL;

				sLog.outLog(LOG_LOOT, "[RaidChestLoot]: disenchanted by %s (%u) from %u", p_caster->GetName(), p_caster->GetGUIDLow(), item);
			}

			return;
		}
	}

    if (!itemTarget->GetProto()->DisenchantID)
        return;

    p_caster->UpdateCraftSkill(GetSpellEntry()->Id);

    ((Player*)m_caster)->SendLoot(itemTarget->GetGUID(),LOOT_DISENCHANTING);

    // item will be removed at disenchanting end
}

void Spell::EffectInebriate(uint32 /*i*/)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = (Player*)unitTarget;
    uint16 currentDrunk = player->GetDrunkValue();
    uint16 drunkMod = damage * 256;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    player->SetDrunkValue(currentDrunk, m_CastItem?m_CastItem->GetEntry():0);
}

void Spell::EffectFeedPet(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    if (!itemTarget)
        return;

    Pet *pet = _player->GetPet();
    if (!pet)
        return;

    if (!pet->isAlive())
        return;

    int32 benefit = pet->GetCurrentFoodBenefitLevel(itemTarget->GetProto()->ItemLevel);
    if (benefit <= 0)
        return;

    uint32 count = 1;
	_player->DestroyItemCount(itemTarget, count, true, "");
    // TODO: fix crash when a spell has two effects, both pointed at the same item target

    m_caster->CastCustomSpell(m_caster,GetSpellEntry()->EffectTriggerSpell[i],&benefit,NULL,NULL,true);
}

void Spell::EffectDismissPet(uint32 /*i*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Pet* pet = m_caster->GetPet();

    // not let dismiss dead pet
    if (!pet||!pet->isAlive())
        return;

    ((Player*)m_caster)->RemovePet(pet,PET_SAVE_NOT_IN_SLOT);
}

void Spell::EffectSummonObject(uint32 i)
{
    uint32 go_id = GetSpellEntry()->EffectMiscValue[i];

    uint8 slot = 0;
    switch (GetSpellEntry()->Effect[i])
    {
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT1: slot = 0; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT2: slot = 1; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT3: slot = 2; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT4: slot = 3; break;
        default: return;
    }

    uint64 guid = m_caster->m_ObjectSlot[slot];
    if (guid)
    {
        if (GameObject* obj = m_caster ? m_caster->GetMap()->GetGameObject(guid) : NULL)
            obj->SetLootState(GO_JUST_DEACTIVATED);

        m_caster->m_ObjectSlot[slot] = 0;
    }

    GameObject* pGameObj = new GameObject;

    float x,y,z;
    // If dest location if present
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
    }
    // Summon in random point all other units if location present
    else
        m_caster->GetNearPoint(x,y,z,DEFAULT_WORLD_OBJECT_SIZE);

    Map *map = m_caster->GetMap();
    if (!pGameObj->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), go_id, map, x, y, z, m_caster->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->GetLevel());
    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(GetSpellEntry()->Id);
    m_caster->AddGameObject(pGameObj);
    if (pGameObj->GetGoType() == GAMEOBJECT_TYPE_TRAP)
    {
        pGameObj->Update(0, 0); // set arming time
    }

    map->Add(pGameObj);
    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << pGameObj->GetGUID();
    m_caster->BroadcastPacket(&data,true);

    m_caster->m_ObjectSlot[slot] = pGameObj->GetGUID();
}

void Spell::EffectResurrect(uint32 /*effIndex*/)
{
    if (!unitTarget || !unitTarget->IsInWorld() || unitTarget->isAlive())
        return;

    switch (GetSpellEntry()->Id)
    {
        // Defibrillate (Goblin Jumper Cables) have 33% chance on success
        case 8342:
            if (roll_chance_i(67))
            {
                m_caster->CastSpell(m_caster, 8338, true, m_CastItem);
                return;
            }
            break;
        // Defibrillate (Goblin Jumper Cables XL) have 50% chance on success
        case 22999:
            if (roll_chance_i(50))
            {
                m_caster->CastSpell(m_caster, 23055, true, m_CastItem);
                return;
            }
            break;
        default:
            break;
    }

    if (Player* pTarget = unitTarget->ToPlayer())
    {
        if (pTarget->isRessurectRequested())       // already have one active request
            return;

        uint32 health = pTarget->GetMaxHealth() * damage / 100;
        uint32 mana   = pTarget->GetMaxPower(POWER_MANA) * damage / 100;

        pTarget->setResurrectRequestData(m_caster->GetGUID(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
        SendResurrectRequest(pTarget);
    }
    else if (Pet* pet = unitTarget->ToPet())
    {
        if (pet->getPetType() != HUNTER_PET || pet->isAlive())
            return;

        float x, y, z;
        m_caster->GetPosition(x, y, z);
        pet->NearTeleportTo(x, y, z, m_caster->GetOrientation());

        pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
        pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        pet->setDeathState(ALIVE);
        pet->ClearUnitState(UNIT_STAT_ALL_STATE);
        pet->SetHealth(uint32(pet->GetMaxHealth() * damage / 100));

        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    }
}

void Spell::EffectAddExtraAttacks(uint32 /*i*/)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    if (unitTarget->m_extraAttacks)
        return;

    Unit *victim = unitTarget->GetVictim();

    // attack prevented
    // fixme, some attacks may not target current victim, this is right now not handled
    if (!victim || !unitTarget->IsWithinMeleeRange(victim) || !unitTarget->HasInArc(2*M_PI/3, victim))
        return;

    // Only for proc/log informations
    unitTarget->m_extraAttacks = damage;

    // Need to send log before attack is made
    SendLogExecute();
    m_needSpellLog = false;

    unitTarget->HandleProcExtraAttackFor(victim);
}

void Spell::EffectParry(uint32 /*i*/)
{
    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
    {
        ((Player*)unitTarget)->SetCanParry(true);
    }
}

void Spell::EffectBlock(uint32 /*i*/)
{
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)unitTarget)->SetCanBlock(true);
}

void Spell::EffectLeapForward(uint32 i)
{
    if (unitTarget->IsTaxiFlying())
        return;

    if (GetSpellEntry()->rangeIndex == 1)                        //self range
    {
        Position dest;
        dest.x = m_targets.m_destX;
        dest.y = m_targets.m_destY;
        dest.z = m_targets.m_destZ;

        unitTarget->NearTeleportTo(dest.x, dest.y, dest.z, unitTarget->GetOrientation(), unitTarget == m_caster);
    }
}

void Spell::EffectLeapBack(uint32 i)
{
    if (unitTarget->IsTaxiFlying())
        return;

    m_caster->KnockBackFrom(unitTarget,float(GetSpellEntry()->EffectMiscValue[i])/10,float(damage)/10);
}

void Spell::EffectReputation(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)unitTarget;

    int32  rep_change = damage;//+1;           // field store reputation change -1

    uint32 faction_id = GetSpellEntry()->EffectMiscValue[i];

    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
        return;

    rep_change = _player->CalculateReputationGain(REPUTATION_SOURCE_SPELL, rep_change, faction_id);

    _player->GetReputationMgr().ModifyReputation(factionEntry,rep_change);
}

void Spell::EffectQuestComplete(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    uint32 quest_id = GetSpellEntry()->EffectMiscValue[i];
    _player->AreaExploredOrEventHappens(quest_id);
}

void Spell::EffectSelfResurrect(uint32 i)
{
    if (!unitTarget || unitTarget->isAlive())
        return;
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!unitTarget->IsInWorld())
        return;

    uint32 health = 0;
    uint32 mana = 0;

    // flat case
    if (damage < 0)
    {
        health = uint32(-damage);
        mana = GetSpellEntry()->EffectMiscValue[i];
    }
    // percent case
    else
    {
        health = uint32(damage/100.0f*unitTarget->GetMaxHealth());
        if (unitTarget->GetMaxPower(POWER_MANA) > 0)
            mana = uint32(damage/100.0f*unitTarget->GetMaxPower(POWER_MANA));
    }

    Player *plr = ((Player*)unitTarget);
    plr->ResurrectPlayer(0.0f);

    plr->SetHealth(int32(health));
    plr->SetPower(POWER_MANA, int32(mana));
    plr->SetPower(POWER_RAGE, 0);
    plr->SetPower(POWER_ENERGY, plr->GetMaxPower(POWER_ENERGY));

    plr->SpawnCorpseBones();
}

void Spell::EffectSkinning(uint32 /*i*/)
{
    if (unitTarget->GetTypeId() != TYPEID_UNIT)
        return;
    if (!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Creature* creature = (Creature*) unitTarget;
    int32 targetLevel = creature->GetLevel();

    uint32 skill = creature->GetCreatureInfo()->GetRequiredLootSkill();

    ((Player*)m_caster)->SendLoot(creature->GetGUID(), LOOT_SKINNING);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

    int32 reqValue = targetLevel < 10 ? 0 : targetLevel < 20 ? (targetLevel-10)*10 : targetLevel*5;

    int32 skillValue = ((Player*)m_caster)->GetPureSkillValue(skill);

    // Double chances for elites
    ((Player*)m_caster)->UpdateGatherSkill(skill, skillValue, reqValue, creature->isElite() ? 2 : 1);
}

void Spell::EffectCharge(uint32 /*i*/)
{
    if (!m_caster)
        return;

    Unit *target = m_targets.getUnitTarget();
    if (!target)
        return;

    float speed = GetSpellEntry()->speed ? GetSpellEntry()->speed : SPEED_CHARGE;
    if (_path->getPathType() & PATHFIND_NOPATH)
    {
        Position dest;
        target->GetPosition(dest);

        float angle = m_caster->GetAngleTo(target) - M_PI;
        m_caster->GetValidPointInAngle(dest, 2.0f, angle, false);
        dest.z += target->GetPositionZ() > m_caster->GetPositionZ() ? ((target->GetPositionZ() - m_caster->GetPositionZ()) / 10.0f + 1) : 1;
        m_caster->GetMotionMaster()->MoveCharge(dest.x, dest.y, dest.z, speed);
    }
    else
        m_caster->GetMotionMaster()->MoveCharge(*_path, speed);

    // not all charge effects used in negative spells
    if (!SpellMgr::IsPositiveSpell(GetSpellEntry()->Id) && m_caster->GetTypeId() == TYPEID_PLAYER &&
        ((Player*)m_caster)->GetSelection() == target->GetGUID()) // attack only if we're charging our current target (focustarget is not current target, should not start attacking)
        m_caster->Attack(target, true);
}

void Spell::EffectCharge2(uint32 /*i*/)
{
    Unit *target = m_targets.getUnitTarget();
    if (!target && !(m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION))
        return;

    float speed = GetSpellEntry()->speed ? GetSpellEntry()->speed : SPEED_CHARGE;
    if (_path->getPathType() & PATHFIND_NOPATH)
    {
        Position dest;
        if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
        {
            dest.x = m_targets.m_destX;
            dest.y = m_targets.m_destY;
            dest.z = m_targets.m_destZ;
        }
        else
        {
            target->GetPosition(dest);

            float angle = m_caster->GetAngleTo(target) - M_PI;
            m_caster->GetValidPointInAngle(dest, 2.0f, angle, false);
        }

        m_caster->GetMotionMaster()->MoveCharge(dest.x, dest.y, dest.z, speed);
    }
    else
        m_caster->GetMotionMaster()->MoveCharge(*_path, speed);

    // not all charge effects used in negative spells
    if (target && !SpellMgr::IsPositiveSpell(GetSpellEntry()->Id))
        m_caster->Attack(target, true);
}

void Spell::EffectSummonCritter(uint32 i)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;
    Player* player = (Player*)m_caster;

    uint32 pet_entry = GetSpellEntry()->EffectMiscValue[i];
    if (!pet_entry)
        return;

    Pet* old_critter = player->GetMiniPet();

    // for same pet just despawn
    if (old_critter && old_critter->GetEntry() == pet_entry)
    {
        player->RemoveMiniPet();
        return;
    }

    // despawn old pet before summon new
    if (old_critter)
        player->RemoveMiniPet();

    // summon new pet
    Pet* critter = new Pet(MINI_PET);

    Map *map = m_caster->GetMap();
    uint32 pet_number = sObjectMgr.GeneratePetNumber();
    if (!critter->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_PET),
        map, pet_entry, pet_number))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Spell::EffectSummonCritter, spellid %u: no such creature entry %u", GetSpellEntry()->Id, pet_entry);
        delete critter;
        return;
    }

    float x,y,z;
    // If dest location if present
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
         x = m_targets.m_destX;
         y = m_targets.m_destY;
         z = m_targets.m_destZ;
     }
     // Summon if dest location not present near caster
     else
         m_caster->GetNearPoint(x,y,z,critter->GetObjectSize());

    critter->Relocate(x,y,z,m_caster->GetOrientation());

    if (!critter->IsPositionValid())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",
            critter->GetGUIDLow(), critter->GetEntry(), critter->GetPositionX(), critter->GetPositionY());
        delete critter;
        return;
    }

    critter->SetOwnerGUID(m_caster->GetGUID());
    critter->SetCreatorGUID(m_caster->GetGUID());
    critter->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE,m_caster->getFaction());
    critter->SetUInt32Value(UNIT_CREATED_BY_SPELL, GetSpellEntry()->Id);

    critter->InitPetCreateSpells();                         // e.g. disgusting oozeling has a create spell as critter...
    critter->SetMaxHealth(1);
    critter->SetHealth(1);
    critter->SelectLevel(critter->GetCreatureInfo());       // some summoned creaters have different from 1 DB data for level/hp
    critter->SetUInt32Value(UNIT_NPC_FLAGS, critter->GetCreatureInfo()->npcflag); // some mini-pets have quests

    // set timer for unsummon
    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());
    if (duration > 0)
        critter->SetDuration(duration);

    std::string name = player->GetName();
    name.append(petTypeSuffix[critter->getPetType()]);
    critter->SetName(name);
    player->SetMiniPet(critter);

    if (critter->GetEntry() != 27914)  // always non-attackable, except Soul-Trader Beacon
    {
        critter->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        if (critter->GetScriptName() == "DeathSide_npc_shop" || critter->GetScriptName() == "MW_bg_reg_anywhere" || 
            (critter->GetScriptName() == "MW_npc_premium" && m_caster->GetTypeId() == TYPEID_PLAYER && ((Player*)m_caster)->GetSession()->IsAccountFlagged(ACC_PREMIUM_NPC_INVISIBLE)))
        {
            critter->SetVisibility(VISIBILITY_OFF);
            // Invisibility visual
            critter->CastSpell(critter, 55315, true);
        }
    }

    map->Add((Creature*)critter);
}

void Spell::EffectKnockBack(uint32 i)
{
    if (!unitTarget)
        return;

    if (GetSpellEntry()->Id == 37852)    // Watery Grave Explosion;
        unitTarget->SetStunned(false);  // stunned state has to be removed manually here before aura expires to allow self knockback

    unitTarget->KnockBackFrom(m_caster,float(GetSpellEntry()->EffectMiscValue[i])/10,float(damage)/10);
}

void Spell::EffectSendTaxi(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(GetSpellEntry()->EffectMiscValue[i]);
    if (!entry)
        return;

    std::vector<uint32> nodes;

    nodes.resize(2);
    nodes[0] = entry->from;
    nodes[1] = entry->to;

    uint32 mountid = 0;
    switch (GetSpellEntry()->Id)
    {
        case 31606:       // Stormcrow Amulet
            mountid = 17447;
            break;
        case 45071:      // Quest - Sunwell Daily - Dead Scar Bombing Run
        case 45113:      // Quest - Sunwell Daily - Ship Bombing Run
        case 45353:      // Quest - Sunwell Daily - Ship Bombing Run Return
            mountid = 22840;
            break;
        case 34905:      //Stealth Flight
            mountid = 6851;
            break;
        case 41533:      // Fly of the Netherwing
        case 41540:      // Fly of the Netherwing
            mountid = 20811;
            break;
        case 42295:     // Alcaz Island Survey
            mountid = 17697;
            break;
    }

    ((Player*)unitTarget)->ActivateTaxiPathTo(nodes,mountid);
}

void Spell::EffectPlayerPull(uint32 i)
{
    if (!unitTarget)
        return;

    float dist = unitTarget->GetDistance2d(m_caster);
    if (damage && dist > damage)
        dist = damage;

    unitTarget->KnockBackFrom(m_caster, -dist, GetSpellEntry()->EffectMiscValue[i]/10.0);
}

void Spell::EffectSuspendGravity(uint32 i)
{
    if (!unitTarget)
        return;

    float dist = unitTarget->GetDistance2d(m_caster);
    WorldLocation wLoc;
    float diff_z;
    unitTarget->GetPosition(wLoc);
    float ground_z = m_caster->GetTerrain()->GetHeight(wLoc.coord_x, wLoc.coord_y, wLoc.coord_z, true);
    diff_z = unitTarget->GetPositionZ() - ground_z;

    // for now this has to only support one spell
    if(unitTarget->HasAura(46230, 2))
    {
        if(Aura* aur = unitTarget->GetAura(46230, 2))
            if(aur->GetAuraDuration() < 3400)
                unitTarget->KnockBackFrom(m_caster, dist-frand(7, 14), diff_z < 1.5 ? GetSpellEntry()->EffectMiscValue[i]/10.0 : 0);
    }
    else
        unitTarget->KnockBackFrom(m_caster, dist-frand(7, 14), diff_z < 1.5 ? GetSpellEntry()->EffectMiscValue[i]/10.0 : 0);
}

void Spell::EffectDispelMechanic(uint32 i)
{
   if (!unitTarget)
        return;

    if (unitTarget->IsHostileTo(m_caster))
    {
        m_caster->SetInCombatWith(unitTarget);
        unitTarget->SetInCombatWith(m_caster);
    }

    // Fill possible dispel list
    //                 spellId  casterGUID            aura    stackAmount EffectsToDispel(flag-type)
    std::map <std::pair<uint32, uint64>, std::pair<Aura*, std::pair<uint8 , uint8>>> dispel_map;

    // Create dispel mask by dispel type
    uint32 mechanic = GetSpellEntry()->EffectMiscValue[i];
    Unit::AuraMap const& auras = unitTarget->GetAuras();
    uint16 arr_count = 0;
    for (Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura* aur = itr->second;
        SpellEntry const *spell = aur->GetSpellProto();
            
        if ((spell->Mechanic == mechanic || spell->EffectMechanic[aur->GetEffIndex()] == mechanic) && !aur->IsPassive()/*there are some spells with Dispel that are passive*/)
        {
            std::pair<uint32, uint64> index = std::make_pair(aur->GetId(), aur->GetCasterGUID());
            if (dispel_map.find(index) == dispel_map.end())
            {
                uint8 stack = aur->GetStackAmount();
                dispel_map[index] = std::make_pair(aur, std::make_pair(stack, (spell->Mechanic == mechanic) ? 0x7/*whole spell*/ : 1<<aur->GetEffIndex()));
                arr_count += stack;
            }
            else
            {
                uint8 eff = 1<<aur->GetEffIndex();
                if (!(dispel_map[index].second.second & eff))
                    dispel_map[index].second.second += eff;
            }
        }
    }

    if (arr_count)
    {
        std::pair<Aura*, uint8>* dispel_list = new std::pair<Aura*, uint8>[arr_count];
        arr_count = 0;
        for (auto i = dispel_map.begin(); i != dispel_map.end(); ++i)
        {
            uint8 s = i->second.second.first;
            for (auto j = 0; j != s; ++j)
            {
                dispel_list[arr_count] = std::make_pair(i->second.first, i->second.second.second);
                ++arr_count;
            }
        }
        //                             spellId, casterGUID, effFlag
        std::list < std::pair<std::pair<uint32,uint64>, uint8> > success_list;// (spell_id,casterGuid)
        std::list < uint32 > fail_list;                     // spell_id;
        // dispel N = damage buffs (or while exist buffs for dispel)
        for (int32 count=0; count < damage && arr_count > 0; ++count)
        {
            uint16 rand = urand(0, arr_count-1);
            // Random select buff for dispel
            Aura *aur = dispel_list[rand].first;

            // Base dispel chance
            // TODO: possible chance depend from spell level??
            int32 miss_chance = 0;
            // Apply dispel mod from aura caster
            if (Unit *caster = aur->GetCaster())
            {
                if (Player* modOwner = caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(aur->GetId(), SPELLMOD_RESIST_DISPEL_CHANCE, miss_chance, this);
            }

            if (miss_chance < 100)
            {
                // Try dispel
                if (roll_chance_i(miss_chance))
                    fail_list.push_back(aur->GetId());
                else
                    success_list.push_back(std::make_pair(std::pair<uint32,uint64>(aur->GetId(),aur->GetCasterGUID()), dispel_list[rand].second));
            }
            // don't try to dispell effects with 100% dispell resistance (patch 2.4.3 notes)
            else
                count--;

            // remove used aura from access
            std::swap(dispel_list[rand], dispel_list[arr_count-1]);
            --arr_count;
        }

        // Send success log and really remove auras
        if (!success_list.empty())
        {
            int32 count = success_list.size();
            WorldPacket data(SMSG_SPELLDISPELLOG, 8+8+4+1+4+count*5);
            data << unitTarget->GetPackGUID();              // Victim GUID
            data << m_caster->GetPackGUID();                // Caster GUID
            data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));                // dispel spell id
            data << uint8(0);                               // not used
            data << uint32(count);                          // count
            for (std::list<std::pair<std::pair<uint32,uint64>, uint8>>::iterator j = success_list.begin(); j != success_list.end(); ++j)
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(j->first.first);
                data << uint32(sSpellMgr.GetSpellAnalog(spellInfo));              // Spell Id
                data << uint8(0);                           // 0 - dispelled !=0 cleansed
                if (j->second == 0x7/*whole spell remove*/)
                {
                    if (spellInfo->StackAmount!= 0)
                        unitTarget->RemoveAuraFromStackByDispel(spellInfo->Id, j->first.second);
                    else
                        unitTarget->RemoveAurasDueToSpellByDispel(spellInfo->Id, j->first.second, m_caster);
                }
                else
                {
                    for (uint8 o = 0; o < 3; ++o)
                    {
                        if ((j->second & (1<<o)))
                            unitTarget->RemoveSingleAuraFromStackByDispel(spellInfo->Id, j->first.second, o);
                    }
                }                
             }
            m_caster->BroadcastPacket(&data, true);
        }
        // Send fail log to client
        if (!fail_list.empty())
        {
            // Failed to dispell
            WorldPacket data(SMSG_DISPEL_FAILED, 8+8+4+4*fail_list.size());
            data << uint64(m_caster->GetGUID());            // Caster GUID
            data << uint64(unitTarget->GetGUID());          // Victim GUID
            data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));                // dispel spell id
            for (std::list< uint32 >::iterator j = fail_list.begin(); j != fail_list.end(); ++j)
                data << uint32(*j);                         // Spell Id
            m_caster->BroadcastPacket(&data, true);
        }
        delete []dispel_list;
    }
    return;
}

void Spell::EffectResurrectPet(uint32 /*i*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = (Player*)m_caster;

    Pet *pet = _player->GetPet();
    if (!pet)
        return;

    if (pet->isAlive())
        return;

    if (damage < 0)
        return;

    float x,y,z;
    _player->GetPosition(x, y, z);
    pet->NearTeleportTo(x, y, z, _player->GetOrientation());

    pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
    pet->RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    pet->setDeathState(ALIVE);
    pet->ClearUnitState(UNIT_STAT_ALL_STATE);
    pet->SetHealth(uint32(pet->GetMaxHealth()*(float(damage)/100)));

    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    _player->DelayedPetSpellInitialize();
}

void Spell::EffectDestroyAllTotems(uint32 /*i*/)
{
    float mana = 0;
    for (int slot = 0; slot < MAX_TOTEM; ++slot)
    {
        if (!m_caster->m_TotemSlot[slot])
            continue;

        Creature* totem = m_caster->GetMap()->GetCreature(m_caster->m_TotemSlot[slot]);
        if (totem && totem->isTotem())
        {
            uint32 spell_id = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
            if (spellInfo)
            {
                mana += spellInfo->manaCost * damage / 100;
                if (spellInfo->ManaCostPercentage)
                    mana += spellInfo->ManaCostPercentage * m_caster->GetCreateMana() / 100 * damage / 100;
            }
            ((Totem*)totem)->UnSummon();
        }
    }

    int32 gain = m_caster->ModifyPower(POWER_MANA, int32(mana));
    m_caster->SendEnergizeSpellLog(m_caster, GetSpellEntry()->Id, gain, POWER_MANA);
}

void Spell::EffectDurabilityDamage(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = GetSpellEntry()->EffectMiscValue[i];

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        ((Player*)unitTarget)->DurabilityPointsLossAll(damage,(slot < -1));
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (Item* item = ((Player*)unitTarget)->GetItemByPos(INVENTORY_SLOT_BAG_0,slot))
        ((Player*)unitTarget)->DurabilityPointsLoss(item,damage);
}

void Spell::EffectDurabilityDamagePCT(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = GetSpellEntry()->EffectMiscValue[i];

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        ((Player*)unitTarget)->DurabilityLossAll(double(damage)/100.0f,(slot < -1));
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (damage <= 0)
        return;

    if (Item* item = ((Player*)unitTarget)->GetItemByPos(INVENTORY_SLOT_BAG_0,slot))
        ((Player*)unitTarget)->DurabilityLoss(item,double(damage)/100.0f);
}

void Spell::EffectModifyThreatPercent(uint32 /*effIndex*/)
{
    if (!unitTarget)
        return;

    unitTarget->getThreatManager().modifyThreatPercent(m_caster, damage);
}

void Spell::EffectTransmitted(uint32 effIndex)
{
    uint32 name_id = GetSpellEntry()->EffectMiscValue[effIndex];

    // Create Soulwell hack
   if (GetSpellEntry()->Id == 29886)
    {
        if (m_caster->HasAura(18693, 0))    //imp healthstone rank 2
            name_id = 183511;
        else
            if (m_caster->HasAura(18692, 0))//imp healthstone rank 1
                name_id = 183510;
    }

    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(name_id);

    if (!goinfo)
    {
        sLog.outLog(LOG_DB_ERR, "Gameobject (Entry: %u) not exist and not created at spell (ID: %u) cast",name_id, GetSpellEntry()->Id);
        return;
    }

    float fx,fy,fz;

    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        fx = m_targets.m_destX;
        fy = m_targets.m_destY;
        fz = m_targets.m_destZ;
    }
    //FIXME: this can be better check for most objects but still hack
    else if (GetSpellEntry()->EffectRadiusIndex[effIndex] && GetSpellEntry()->speed==0)
    {
        float dis = SpellMgr::GetSpellRadius(GetSpellEntry(),effIndex,false);
        m_caster->GetNearPoint(fx,fy,fz,DEFAULT_WORLD_OBJECT_SIZE, dis);
    }
    else
    {
        float min_dis = SpellMgr::GetSpellMinRange(sSpellRangeStore.LookupEntry(GetSpellEntry()->rangeIndex));
        float max_dis = SpellMgr::GetSpellMaxRange(sSpellRangeStore.LookupEntry(GetSpellEntry()->rangeIndex));
        float dis = rand_norm() * (max_dis - min_dis) + min_dis;

        m_caster->GetNearPoint(fx,fy,fz,DEFAULT_WORLD_OBJECT_SIZE, dis);
    }

    Map *cMap = m_caster->GetMap();

    if (goinfo->type==GAMEOBJECT_TYPE_SUMMONING_RITUAL)
    {
        m_caster->GetPosition(fx,fy,fz);
    }

    GameObject* pGameObj = new GameObject;

    if (!pGameObj->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), name_id, cMap,
        fx, fy, fz, m_caster->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 100, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    int32 duration = SpellMgr::GetSpellDuration(GetSpellEntry());

    switch (goinfo->type)
    {
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            m_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT,pGameObj->GetGUID());
            m_caster->AddGameObject(pGameObj);              // will removed at spell cancel

            // end time of range when possible catch fish (FISHING_BOBBER_READY_TIME..GetDuration(GetSpellEntry()))
            // start time == fish-FISHING_BOBBER_READY_TIME (0..GetDuration(GetSpellEntry())-FISHING_BOBBER_READY_TIME)
            int32 lastSec = 0;
            switch (urand(0, 3))
            {
                case 0: lastSec =  3; break;
                case 1: lastSec =  7; break;
                case 2: lastSec = 13; break;
                case 3: lastSec = 17; break;
            }

            duration = duration - lastSec*1000 + FISHING_BOBBER_READY_TIME*1000;
            break;
        }
        case GAMEOBJECT_TYPE_SUMMONING_RITUAL:
        {
            if (m_caster->GetTypeId()==TYPEID_PLAYER)
            {
                pGameObj->AddUniqueUse((Player*)m_caster);
                m_caster->AddGameObject(pGameObj);          // will removed at spell cancel
                if (pGameObj->GetEntry() != 177193) // ritual of doom
                    pGameObj->SetLootState(GO_ACTIVATED);
                pGameObj->SetTarget(((Player*)m_caster)->GetSelection());
            }
            break;
        }
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        case GAMEOBJECT_TYPE_CHEST:
        default:
        {
            break;
        }
    }

    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);

    pGameObj->SetOwnerGUID(m_caster->GetGUID());

    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->GetLevel());
    pGameObj->SetSpellId(GetSpellEntry()->Id);

    debug_log("AddObject at SpellEfects.cpp EffectTransmitted\n");
    //m_caster->AddGameObject(pGameObj);
    //m_ObjToDel.push_back(pGameObj);

    cMap->Add(pGameObj);

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << uint64(pGameObj->GetGUID());
    m_caster->BroadcastPacket(&data,true);

    if (uint32 linkedEntry = pGameObj->GetLinkedGameObjectEntry())
    {
        GameObject* linkedGO = new GameObject;
        if (linkedGO->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), linkedEntry, cMap,
            fx, fy, fz, m_caster->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 100, GO_STATE_READY))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration/1000 : 0);
            //linkedGO->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->GetLevel());
            linkedGO->SetSpellId(GetSpellEntry()->Id);
            linkedGO->SetOwnerGUID(m_caster->GetGUID());

            linkedGO->GetMap()->Add(linkedGO);
        }
        else
        {
            delete linkedGO;
            linkedGO = NULL;
            return;
        }
    }
}

void Spell::EffectProspecting(uint32 /*i*/)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = (Player*)m_caster;
    if (!itemTarget || !(itemTarget->GetProto()->BagFamily & BAG_FAMILY_MASK_MINING_SUPP))
        return;

    if (itemTarget->GetCount() < 5)
        return;

    if (sWorld.getConfig(CONFIG_SKILL_PROSPECTING))
    {
        uint32 SkillValue = p_caster->GetPureSkillValue(SKILL_JEWELCRAFTING);
        uint32 reqSkillValue = itemTarget->GetProto()->RequiredSkillRank;
        p_caster->UpdateGatherSkill(SKILL_JEWELCRAFTING, SkillValue, reqSkillValue);
    }

    ((Player*)m_caster)->SendLoot(itemTarget->GetGUID(), LOOT_PROSPECTING);
}

void Spell::EffectSkill(uint32 /*i*/)
{
    sLog.outDebug("WORLD: SkillEFFECT");
}

void Spell::EffectSummonDemon(uint32 i)
{
    float px = m_targets.m_destX;
    float py = m_targets.m_destY;
    float pz = m_targets.m_destZ;
    int32 creature_ID = GetSpellEntry()->EffectMiscValue[i];

    Creature* Charmed = m_caster->SummonCreature(creature_ID, px, py, pz, m_caster->GetOrientation(),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,3600000);
    if (!Charmed)
        return;

    // might not always work correctly, maybe the creature that dies from CoD casts the effect on itself and is therefore the caster?
    Charmed->SetLevel(m_caster->GetLevel());

    // Add damage/mana/hp according to level
    //PetLevelInfo const* pInfo = sObjectMgr.GetPetLevelInfo(creature_ID, m_caster->GetLevel());
    if (creature_ID == 89 || creature_ID == 11859)
    {
        int32 CasterLevel = m_caster->GetLevel();
        int lvldiff = (CasterLevel - 52);
        if (lvldiff > 0)
        {
            if (creature_ID == 89)
            {
                int32 hp, armor, newdam;
                if (CasterLevel >= 60)
                {
                    hp = (2690 + 641 * (CasterLevel - 60));
                    armor = 4500 + 350 * (CasterLevel - 60);
                    newdam = (299 + 26.25f * (CasterLevel - 60)) * 1.4f; // Mid damage for level 70 is 637.5, for 60 level - 375
                }
                else
                {
                    hp = int32(CasterLevel * 44.3f);
                    armor = int32(71.83f * CasterLevel);
                    newdam = 5 * CasterLevel * 1.4f;
                }

                int32 SPDval;
                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    SPDval = m_caster->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + SPELL_SCHOOL_FIRE)*0.44f; // Receives 44% spell dam (fire) from the caster
                else
                    SPDval = 0;

                Charmed->CastCustomSpell(Charmed, 9392, &SPDval, NULL, NULL, true);
                Charmed->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, hp);
                Charmed->SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, 0);
                Charmed->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, newdam);
                Charmed->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, newdam + 150);
                Charmed->SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, armor);//around 44% armor
                Charmed->SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, 0);
                if (CasterLevel < 58)
                    m_caster->CastSpell(Charmed, 11725, true); // Enslave
                else
                    m_caster->CastSpell(Charmed, 11726, true); // Enslave
                int32 StunBonusDamage = 200;
                Charmed->CastCustomSpell(Charmed, 22703, &StunBonusDamage, NULL, NULL, true);        // Inferno effect
            }
            else if (creature_ID == 11859)
            {
                int32 hp, armor, newdam, mana, SPDval;
                if (CasterLevel >= 60)
                {
                    hp = (4900 + 210 * (CasterLevel - 60)); // 7000 health on 70 level
                    armor = 4500 + 350 * (CasterLevel - 60);
                    newdam = (182.5f + 15.5f * (CasterLevel - 60)) * 1.4f; // Mid damage for level 70 is 375, for 60 level - 220
                    mana = (1874 + 55.5f * (CasterLevel - 60)); // 2429 mana for 70 level. 1874 mana for 60 level
                }
                else
                {
                    hp = int32(CasterLevel * 81.66f);
                    armor = int32(71.83f * CasterLevel);
                    newdam = 3.04f * CasterLevel * 1.4f;
                    mana = 31.23f * CasterLevel;
                }
                SPDval = 0;
                Charmed->CastCustomSpell(Charmed, 9392, &SPDval, NULL, NULL, true);
                Charmed->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, hp);
                Charmed->SetModifierValue(UNIT_MOD_MANA, BASE_VALUE, mana);
                Charmed->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, newdam);
                Charmed->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, newdam + 75);
                Charmed->SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, armor);
                Charmed->SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, 0);
            }

            Charmed->UpdateAllStats();
            Charmed->SetHealth(Charmed->GetMaxHealth());
            Charmed->SetPower(POWER_MANA, Charmed->GetMaxPower(POWER_MANA));
        }
    }

}

/* There is currently no need for this effect. We handle it in BattleGround.cpp
   If we would handle the resurrection here, the spiritguide would instantly disappear as the
   player revives, and so we wouldn't see the spirit heal visual effect on the npc.
   This is why we use a half sec delay between the visual effect and the resurrection itself */
void Spell::EffectSpiritHeal(uint32 /*i*/)
{
    /*
    if (!unitTarget || unitTarget->isAlive())
        return;
    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    if (!unitTarget->IsInWorld())
        return;

    //GetSpellEntry()->EffectBasePoints[i]; == 99 (percent?)
    //((Player*)unitTarget)->setResurrect(m_caster->GetGUID(), unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetMaxHealth(), unitTarget->GetMaxPower(POWER_MANA));
    ((Player*)unitTarget)->ResurrectPlayer(1.0f);
    ((Player*)unitTarget)->SpawnCorpseBones();
    */
}

// remove insignia spell effect
void Spell::EffectSkinPlayerCorpse(uint32 /*i*/)
{
    sLog.outDebug("Effect: SkinPlayerCorpse");
    if ((m_caster->GetTypeId() != TYPEID_PLAYER) || (unitTarget->GetTypeId() != TYPEID_PLAYER) || (unitTarget->isAlive()))
        return;

    ((Player*)unitTarget)->RemovedInsignia((Player*)m_caster);
}

void Spell::EffectStealBeneficialBuff(uint32 i)
{
    sLog.outDebug("Effect: StealBeneficialBuff");
    if (!unitTarget || unitTarget == m_caster) // cannot steal from self
        return;

    if (unitTarget->IsHostileTo(m_caster))
    {
        // make this additional immunity check here for mass dispel
        if (unitTarget->IsImmunedToDamage(SpellMgr::GetSpellSchoolMask(GetSpellEntry())))
            return;
        m_caster->SetInCombatWith(unitTarget);
        unitTarget->SetInCombatWith(m_caster);
    }

    // Fill possible dispel list
    std::map <std::pair<uint32, uint64>, std::pair<Aura* , uint8>> dispel_map;

    // Create dispel mask by dispel type
    uint32 dispelMask  = SpellMgr::GetDispellMask(DispelType(GetSpellEntry()->EffectMiscValue[i]));
    Unit::AuraMap const& auras = unitTarget->GetAuras();
    uint16 arr_count = 0;
    for (Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura *aur = (*itr).second;
        if (aur && (1<<aur->GetSpellProto()->Dispel) & dispelMask && !aur->IsPassive()/*there are some spells with Dispel that are passive*/)
        {
            // Need check for passive? this
            if (aur->IsPositive() &&
                !(aur->GetSpellProto()->AttributesEx4 & SPELL_ATTR_EX4_NOT_STEALABLE))
            {
                std::pair<uint32, uint64> index = std::make_pair(aur->GetId(), aur->GetCasterGUID());
                if (dispel_map.find(index) == dispel_map.end())
                {
                    uint8 stack = aur->GetStackAmount();
                    dispel_map[index] = std::make_pair(aur, stack);
                    arr_count += stack;
                }
            }
        }
    }

    if (arr_count)
    {
        Aura** dispel_list = new Aura*[arr_count];
        arr_count = 0;
        for (auto i = dispel_map.begin(); i != dispel_map.end(); ++i)
        {
            uint8 s = i->second.second;
            for (auto j = 0; j != s; ++j)
            {
                dispel_list[arr_count] = i->second.first;
                ++arr_count;
            }
        }

        std::list < std::pair<uint32,uint64> > success_list;// (spell_id,casterGuid)
        std::list < uint32 > fail_list;                     // spell_id;
        // dispel N = damage buffs (or while exist buffs for dispel)
        for (int32 count=0; count < damage && arr_count > 0; ++count)
        {
            uint16 rand = urand(0, arr_count-1);
            // Random select buff for dispel
            Aura *aur = dispel_list[rand];

            // Base dispel chance
            // TODO: possible chance depend from spell level??
            int32 miss_chance = 0;
            // Apply dispel mod from aura caster
            if (Unit *caster = aur->GetCaster())
            {
                if (Player* modOwner = caster->GetSpellModOwner())
                    modOwner->ApplySpellMod(aur->GetId(), SPELLMOD_RESIST_DISPEL_CHANCE, miss_chance, this);
            }

            if (miss_chance < 100)
            {
                // Try dispel
                if (roll_chance_i(miss_chance))
                    fail_list.push_back(aur->GetId());
                else
                    success_list.push_back(std::pair<uint32,uint64>(aur->GetId(),aur->GetCasterGUID()));
            }
            // don't try to dispell effects with 100% dispell resistance (patch 2.4.3 notes)
            else
                count--;

            // remove used aura from access
            std::swap(dispel_list[rand], dispel_list[arr_count-1]);
            --arr_count;
        }

        // Send success log and really remove auras
        if (!success_list.empty())
        {
            int32 count = success_list.size();
            WorldPacket data(SMSG_SPELLSTEALLOG, 8+8+4+1+4+count*5);
            data << unitTarget->GetPackGUID();       // Victim GUID
            data << m_caster->GetPackGUID();         // Caster GUID
            data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));         // dispel spell id
            data << uint8(0);                        // not used
            data << uint32(count);                   // count
            for (std::list<std::pair<uint32,uint64> >::iterator j = success_list.begin(); j != success_list.end(); ++j)
            {
                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(j->first);
                data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));       // Spell Id
                data << uint8(0);                    // 0 - steals !=0 transfers
                unitTarget->RemoveAurasDueToSpellBySteal(spellInfo->Id, j->second, m_caster);
            }
            m_caster->BroadcastPacket(&data, true);
        }
        // Is there other way to send spellsteal resists?
        // Send fail log to client
        if (!fail_list.empty())
        {
            // Failed to dispell
            WorldPacket data(SMSG_DISPEL_FAILED, 8+8+4+4*fail_list.size());
            data << uint64(m_caster->GetGUID());            // Caster GUID
            data << uint64(unitTarget->GetGUID());          // Victim GUID
            data << uint32(sSpellMgr.GetSpellAnalog(GetSpellEntry()));                // dispel spell id
            for (std::list< uint32 >::iterator j = fail_list.begin(); j != fail_list.end(); ++j)
                data << uint32(*j);                         // Spell Id
            m_caster->BroadcastPacket(&data, true);
        }
        delete []dispel_list;
    }    
}

void Spell::EffectKillCredit(uint32 i)
{
    unitTarget->SendCombatStats(1 << COMBAT_STATS_TEST, "Kill credit %u", NULL, GetSpellEntry()->EffectMiscValue[i]);
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)unitTarget)->RewardPlayerAndGroupAtEvent(GetSpellEntry()->EffectMiscValue[i], unitTarget);
}

void Spell::EffectQuestFail(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    ((Player*)unitTarget)->FailQuest(GetSpellEntry()->EffectMiscValue[i]);
}

void Spell::EffectRedirectThreat(uint32 /*i*/)
{
    if (unitTarget)
        m_caster->SetReducedThreatPercent(100, unitTarget->GetGUID());
}

void Spell::EffectPlayMusic(uint32 i)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 soundid = GetSpellEntry()->EffectMiscValue[i];

    if (!sSoundEntriesStore.LookupEntry(soundid))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: EffectPlayMusic: Sound (Id: %u) not exist in spell %u.",soundid,GetSpellEntry()->Id);
        return;
    }

    WorldPacket data(SMSG_PLAY_MUSIC, 4);
    data << uint32(soundid);
    ((Player*)unitTarget)->SendPacketToSelf(&data);
}

void Spell::EffectFriendSummon(uint32 i)
{
    if (!unitTarget)
        return;

    // cast triggerSpell without the trigger at our target
    uint32 triggered_spell_id = GetSpellEntry()->EffectTriggerSpell[i];
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(triggered_spell_id);

    if (!spellInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: EffectFriendSummon of spell %u: triggering unknown spell id %i", GetSpellEntry()->Id, triggered_spell_id);
        return;
    }

    m_caster->CastSpell(unitTarget, spellInfo, true);
}
