// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
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

#include "CharmInfo.h"

#include "WorldPacket.h"
#include "Player.h"
#include "Creature.h"
#include "PetAI.h"
#include "Spell.h"
#include "SpellMgr.h"

#include "MovementGenerator.h"

#include "ObjectMgr.h" //delete this?

////////////////////////////////////////////////////////////
// Methods of class GlobalCooldownMgr
bool CooldownMgr::HasGlobalCooldown(SpellEntry const* spellInfo) const
{
    CooldownList::const_iterator itr = m_GlobalCooldowns.find(spellInfo->StartRecoveryCategory);
    uint32 msNow = WorldTimer::getMSTime();
    return itr != m_GlobalCooldowns.end() && itr->second.duration && WorldTimer::getMSTimeDiff(itr->second.cast_time, msNow + WorldTimer::getMSTimeDiff(WorldTimer::tickPrevTime(), WorldTimer::tickTime()) * 2) < itr->second.duration;
}

bool CooldownMgr::HasGlobalCooldown(uint32 category) const
{
    CooldownList::const_iterator itr = m_GlobalCooldowns.find(category);
    uint32 msNow = WorldTimer::getMSTime();
    return itr != m_GlobalCooldowns.end() && itr->second.duration && WorldTimer::getMSTimeDiff(itr->second.cast_time, msNow + WorldTimer::getMSTimeDiff(WorldTimer::tickPrevTime(), WorldTimer::tickTime()) * 2) < itr->second.duration;
}

void CooldownMgr::AddGlobalCooldown(SpellEntry const* spellInfo, uint32 gcd)
{
    m_GlobalCooldowns[spellInfo->StartRecoveryCategory] = Cooldown(gcd, WorldTimer::getMSTime());
}

void CooldownMgr::CancelGlobalCooldown(SpellEntry const* spellInfo)
{
    m_GlobalCooldowns[spellInfo->StartRecoveryCategory].duration = 0;
}

void CooldownMgr::ClearGlobalCooldowns()
{
    m_GlobalCooldowns.clear();
}

bool CooldownMgr::HasSpellCategoryCooldown(SpellEntry const* spellInfo) const // UNUSED AT ALL. For players category check uses HasCategorySpellCooldown(uint32 category);
{
    CooldownList::const_iterator itr = m_CategoryCooldowns.find(spellInfo->Category);
    uint32 msNow = WorldTimer::getMSTime();
    return itr != m_CategoryCooldowns.end() && itr->second.duration && WorldTimer::getMSTimeDiff(itr->second.cast_time, msNow + WorldTimer::getMSTimeDiff(WorldTimer::tickPrevTime(), WorldTimer::tickTime()) * 2) < itr->second.duration;
}

void CooldownMgr::AddSpellCategoryCooldown(SpellEntry const* spellInfo, uint32 gcd) // UNUSED
{
    m_CategoryCooldowns[spellInfo->Category] = Cooldown(gcd, WorldTimer::getMSTime());
}

void CooldownMgr::CancelSpellCategoryCooldown(SpellEntry const* spellInfo)
{
    m_CategoryCooldowns[spellInfo->Category].duration = 0;
}

bool CooldownMgr::HasSpellIdCooldown(SpellEntry const* spellInfo) const  // ALMOST UNUSED - TRENTONE - need to move the cooldown fix (by worldtimer) to another place where it is REALLY USED.
{
    CooldownList::const_iterator itr = m_SpellCooldowns.find(spellInfo->Id);
    uint32 msNow = WorldTimer::getMSTime();
    return itr != m_SpellCooldowns.end() && itr->second.duration && WorldTimer::getMSTimeDiff(itr->second.cast_time, msNow + WorldTimer::getMSTimeDiff(WorldTimer::tickPrevTime(), WorldTimer::tickTime()) * 2) < itr->second.duration;
}

void CooldownMgr::AddSpellIdCooldown(SpellEntry const* spellInfo, uint32 gcd)
{
    m_SpellCooldowns[spellInfo->Id] = Cooldown(gcd, WorldTimer::getMSTime());
}

void CooldownMgr::CancelSpellIdCooldown(SpellEntry const* spellInfo)
{
    m_SpellCooldowns[spellInfo->Id].duration = 0;
}

CharmInfo::CharmInfo(Unit* unit)
: m_unit(unit), m_CommandState(COMMAND_FOLLOW), m_petnumber(0), m_barInit(false)
{
    for (int i =0; i<4; ++i)
    {
        m_charmspells[i].spellId = 0;
        m_charmspells[i].active = ACT_DISABLED;
    }

    if (m_unit->GetTypeId() == TYPEID_UNIT)
    {
        m_oldReactState = ((Creature*)m_unit)->GetReactState();
        ((Creature*)m_unit)->SetReactState(REACT_PASSIVE);
    }
}

CharmInfo::~CharmInfo()

{
}

void CharmInfo::InitPetActionBar()
{
    if (m_barInit)
        return;

    // the first 3 SpellOrActions are attack, follow and stay
    for (uint32 i = 0; i < 3; i++)
    {
        PetActionBar[i].Type = ACT_COMMAND;
        PetActionBar[i].SpellOrAction = COMMAND_ATTACK - i;

        PetActionBar[i + 7].Type = ACT_REACTION;
        PetActionBar[i + 7].SpellOrAction = COMMAND_ATTACK - i;
    }
    for (uint32 i=0; i < 4; i++)
    {
        PetActionBar[i + 3].Type = ACT_DISABLED;
        PetActionBar[i + 3].SpellOrAction = 0;
    }
    m_barInit = true;
}

void CharmInfo::InitEmptyActionBar(bool withAttack)
{
    if (m_barInit)
        return;

    for (uint32 x = 0; x < 10; ++x)
    {
        PetActionBar[x].Type = ACT_CAST;
        PetActionBar[x].SpellOrAction = 0;
    }
    if (withAttack)
    {
        PetActionBar[0].Type = ACT_COMMAND;
        PetActionBar[0].SpellOrAction = COMMAND_ATTACK;
    }
    m_barInit = true;
}

void CharmInfo::InitPossessCreateSpells()
{
    uint32 TeronIAmSpellID[7] =   // Teron Gorefiend
    {
        60000,  //to make empty slot
        60000,  //to make empty slot
        60000,  //to make empty slot
        37729,
        37727,
        37788,
        37728
    };

    uint32 SpiritSpellID[7] =   //Vengeful Spirit's spells
    {
        40325,
        60000,  //to make empty slot
        40157,
        40175,
        40314,
        60000,  //to make empty slot
        40322
    };

    uint32 BlueDrakeID[5] =   //Power of the Blue Flight spells (Kij'jaeden fight)
    {
        45862,
        45856,
        45860,
        60000,  //to make empty slot
        45848
    };

    uint32 ArcanoScorp[4] =   // Arcano-Scorp, quest 10672
    {
        37851,
        37917,
        37919,
        37918
    };

    uint32 FelguardDegrader[9] =   // Felguard Degrader, Shartuul's Transporter Event
    {
        40220,
        40219,
        60000,  //to make empty slot
        40221,
        40497,
        60000,  //to make empty slot
        40222,
        60000,  //to make empty slot
        40658
    };

    uint32 DoomguardPunisher[9] =   // Doomguard Punisher, Shartuul's Transporter Event
    {
        40560,
        40561,
        60000,  //to make empty slot
        40563,
        40565,
        60000,  //to make empty slot
        40493,
        60000,  //to make empty slot
        35181
    };
    
    uint32 RazorgoreTheUntamed[4] =   // Doomguard Punisher, Shartuul's Transporter Event
    {
        19873, // destroy egg
        19872, // calm dragonkin
        19632, // cleave
        22425  // fireball valley
    };

    uint32 FelReaverSentinel[4] =   // Fel Reaver Sentinel
    {
        38052,
        38006,
        38055,
        37920 
    };

    /*uint32 ShivanAssasinShadow[9] =   // Shavan Assasin Shadow Aspect, Shartuul's Transporter Event
    {
        40736,
        41597,
        40737,
        60000,  //to make empty slot
        41593,
        41594,
        60000,  //to make empty slot
        40741
    };

    uint32 ShivanAssasinFlame[9] =   // Shavan Assasin Flame Aspect, Shartuul's Transporter Event
    {
        40560,
        40561,
        60000,  //to make empty slot
        40563,
        40565,
        60000,  //to make empty slot
        40493,
        60000,  //to make empty slot
        35181
    };

    uint32 ShivanAssasinFrost[9] =   // Shavan Assasin Frost Aspect, Shartuul's Transporter Event
    {
        40560,
        40561,
        60000,  //to make empty slot
        40563,
        40565,
        60000,  //to make empty slot
        40493,
        60000,  //to make empty slot
        35181
    };*/

    /*uint32 BloodwarderMender[3] =   // Botanica: Bloodwarder Mender
    {
        35096, // Greater Heal
        34809, // Holy Fury
        17287 // Mind Blast - Heroic Only
    };
    
    uint32 SunseekerGeneSplicer[1] =   // Botanica: Sunseeker Gene-Splicer
    {
        34642 // Death & Decay
    };

    uint32 EtherealCryptRaider[2] =   // Mana Tombs: Ethereal Crypt Raider
    {
        31403, // Battle Shout
        32315  // Soul Strike
    };

    uint32 EtherealScavenger[3] =   // Mana Tombs: Ethereal Scavenger
    {
        33871, // Shield Bash
        33865, // Singe
        34920  // Strike
    };

    uint32 EtherealPriest[3] =   // Mana Tombs: Ethereal Priest
    {
        34945, // Heal
        34944, // Holy Nova
        17139  // Power Word: Shield
    };

    uint32 EtherealDarkcaster[3] =   // Mana Tombs: Ethereal Darkcaster
    {
        34931, // Mana Burn
        34942, // SW: Pain
        16592  // Shadowform
    };

    uint32 NexusStalker[1] =   // Mana Tombs: Nexus Stalker
    {
        33925 // Phantom Strike
    };

    uint32 EtherealSpellbinder[2] =   // Mana Tombs: Ethereal Spellbinder
    {
        37470, // Counterspell
        17883  // Immolate
    };

    uint32 SethekkGuard[1] =   // Sethekk Halls: Sethekk Guard
    {
        33967 // Thunderclap
    };

    uint32 SethekkInitiate[2] =   // Sethekk Halls: Sethekk Initiate
    {
        33961, // Spell Reflection
        16145  // Sunder Armor
    };

    uint32 SethekkOracle[1] =   // Sethekk Halls: Sethekk Oracle
    {
        32129 // Faerie Fire
    };

    uint32 SethekkRavenguard[2] =   // Sethekk Halls: Sethekk Ravenguard
    {
        33964, // Bloodthirst
        32651  // Howling Screech
    };

    uint32 SethekkTalonLord[1] =   // Sethekk Halls: Sethekk Talon Lord
    {
        32654 // Talon Of Justice
    };

    if ((m_unit->GetEntry() == 18321) || (m_unit->GetEntry() == 20701))     //HACK to allow proper spells for Sethekk Halls: Sethekk Talon Lord
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 1; ++i)
        {
            uint32 spellid = EtherealSpellbinder[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18322) || (m_unit->GetEntry() == 20696))     //HACK to allow proper spells for Sethekk Halls: Sethekk Ravenguard
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 2; ++i)
        {
            uint32 spellid = EtherealSpellbinder[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18328) || (m_unit->GetEntry() == 20694))     //HACK to allow proper spells for Sethekk Halls: Sethekk Oracle
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 1; ++i)
        {
            uint32 spellid = EtherealSpellbinder[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18318) || (m_unit->GetEntry() == 20693))     //HACK to allow proper spells for Sethekk Halls: Sethekk Initiate
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 2; ++i)
        {
            uint32 spellid = EtherealSpellbinder[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18323) || (m_unit->GetEntry() == 20692))     //HACK to allow proper spells for Sethekk Halls: Sethekk Guard
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 1; ++i)
        {
            uint32 spellid = EtherealSpellbinder[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18312) || (m_unit->GetEntry() == 20260))     //HACK to allow proper spells for Ethereal Spellbinder
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 2; ++i)
        {
            uint32 spellid = EtherealSpellbinder[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18314) || (m_unit->GetEntry() == 20264))     //HACK to allow proper spells for Nexus Stalker
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 1; ++i)
        {
            uint32 spellid = NexusStalker[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18331) || (m_unit->GetEntry() == 20256))     //HACK to allow proper spells for Ethereal Darkcaster
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 3; ++i)
        {
            uint32 spellid = EtherealDarkcaster[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18311) || (m_unit->GetEntry() == 20257))     //HACK to allow proper spells for Ethereal Priest
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 3; ++i)
        {
            uint32 spellid = EtherealPriest[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18309) || (m_unit->GetEntry() == 20258))     //HACK to allow proper spells for Ethereal Scavenger
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 3; ++i)
        {
            uint32 spellid = EtherealScavenger[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 18311) || (m_unit->GetEntry() == 20255))     //HACK to allow proper spells for Ethereal Crypt Raider
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 2; ++i)
        {
            uint32 spellid = EtherealCryptRaider[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if ((m_unit->GetEntry() == 19507) || (m_unit->GetEntry() == 21573))     //HACK to allow proper spells for Sunseeker Gene-Splicer
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 1; ++i)
        {
            uint32 spellid = SunseekerGeneSplicer[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 19633)     //HACK to allow proper spells for Bloodwarder Mender(N)
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 2; ++i)
        {
            uint32 spellid = BloodwarderMender[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }
    
    if (m_unit->GetEntry() == 21547)     //HACK to allow proper spells for Bloodwarder Mender(H)
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 3; ++i)
        {
            uint32 spellid = BloodwarderMender[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }*/

    if (m_unit->GetEntry() == 21867)     //HACK to allow proper spells for Teron Gorefiend
    {
        InitEmptyActionBar(true);

        for (uint32 i = 0; i < 7; ++i)
        {
            uint32 spellid = TeronIAmSpellID[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 23109)     //HACK to allow proper spells for Vengeful Spirit
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 7; ++i)
        {
            uint32 spellid = SpiritSpellID[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 25653)     //HACK to allow proper spells for the Power of the Blue Flight
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 5; ++i)
        {
            uint32 spellid = BlueDrakeID[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 21909)     //HACK to allow proper spells for Arcano-Scorp
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 4; ++i)
        {
            uint32 spellid = ArcanoScorp[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 16506)     //HACK to allow proper spells for Naxxramas Worshipper(Faerlina event)
    {
        InitEmptyActionBar(true);
        AddSpellToActionBar(0, 28732, ACT_CAST);
        return;
    }

    if (m_unit->GetEntry() == 23055)     //HACK to allow proper spells for Felguard Degrader
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 9; ++i)
        {
            uint32 spellid = FelguardDegrader[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 23113)     //HACK to allow proper spells for Doomguard Punisher
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 9; ++i)
        {
            uint32 spellid = DoomguardPunisher[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }
    
    if (m_unit->GetEntry() == 12435)     //HACK to allow proper spells for Razorgore The Untamed
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 4; ++i)
        {
            uint32 spellid = RazorgoreTheUntamed[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 21949)     //HACK to allow proper spells for Fel Reaver Sentinel
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 4; ++i)
        {
            uint32 spellid = FelReaverSentinel[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    /*if (m_unit->GetEntry() == 23476)
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 9; ++i)
        {
            uint32 spellid = ShivanAssasinShadow[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 23474)
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 9; ++i)
        {
            uint32 spellid = ShivanAssasinFlame[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }

    if (m_unit->GetEntry() == 23475)
    {
        InitEmptyActionBar(false);

        for (uint32 i = 0; i < 9; ++i)
        {
            uint32 spellid = ShivanAssasinFrost[i];
            AddSpellToActionBar(0, spellid, ACT_CAST);
        }
        return;
    }*/

    InitEmptyActionBar();

    if (m_unit->GetTypeId() == TYPEID_UNIT)
    {
        for (uint32 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        {
            uint32 spellid = ((Creature*)m_unit)->m_spells[i];
            if (SpellMgr::IsPassiveSpell(spellid))
                m_unit->CastSpell(m_unit, spellid, true);
            else
            {
                // add spell only if there are cooldown or global cooldown // TODO: find proper solution
                const SpellEntry * tmpSpellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
                if (tmpSpellEntry)
                {
                    if (tmpSpellEntry->RecoveryTime || tmpSpellEntry->StartRecoveryTime || tmpSpellEntry->CategoryRecoveryTime)
                        AddSpellToActionBar(0, spellid, ACT_CAST);
                    else
                        sLog.outLog(LOG_DEFAULT, "ERROR: Didn't add spell to action bar on possess. SpellID: %u, creature ID: %u.", spellid, ((Creature*)m_unit)->GetEntry());
                }
            }
        }
    }
}

void CharmInfo::InitCharmCreateSpells()
{
    if (m_unit->GetTypeId() == TYPEID_PLAYER)                //charmed players don't have spells
    {
        InitEmptyActionBar();
        return;
    }

    InitPetActionBar();

    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        uint32 spellId = ((Creature*)m_unit)->m_spells[x];
        m_charmspells[x].spellId = spellId;

        if (!spellId)
            continue;

        if (SpellMgr::IsPassiveSpell(spellId))
        {
            m_unit->CastSpell(m_unit, spellId, true);
            m_charmspells[x].active = ACT_PASSIVE;
        }
        else
        {
            ActiveStates newstate;
            bool onlyselfcast = true;
            SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

            if (spellInfo)
            {
                for (uint32 i = 0; i < 3; ++i)       //non existent spell will not make any problems as onlyselfcast would be false -> break right away
                {
                    if (spellInfo->EffectImplicitTargetA[i] != TARGET_UNIT_CASTER && spellInfo->EffectImplicitTargetA[i] != 0)
                    {
                        onlyselfcast = false;
                        break;
                    }
                }
            }
            else
                onlyselfcast = false;

            if (onlyselfcast || !SpellMgr::IsPositiveSpell(spellId))   //only self cast and spells versus enemies are autocastable
                newstate = ACT_DISABLED;
            else
                newstate = ACT_CAST;

            // add spell only if there are cooldown or global cooldown // TODO: find proper solution
            if (spellInfo)
            {
                if (spellInfo->RecoveryTime || spellInfo->StartRecoveryTime || spellInfo->CategoryRecoveryTime)
                    AddSpellToActionBar(0, spellId, newstate);
                else
                    sLog.outLog(LOG_DEFAULT, "ERROR: Didn't add spell to action bar on charm. SpellID: %u, creature ID: %u.", spellId, ((Creature*)m_unit)->GetEntry());
            }
        }
    }
}

bool CharmInfo::AddSpellToActionBar(uint32 oldid, uint32 newid, ActiveStates newstate)
{
    for (uint8 i = 0; i < 10; i++)
    {
        if ((PetActionBar[i].Type == ACT_DISABLED || PetActionBar[i].Type == ACT_ENABLED || PetActionBar[i].Type == ACT_CAST) && PetActionBar[i].SpellOrAction == oldid)
        {
            PetActionBar[i].SpellOrAction = newid;
            if (!oldid)
            {
                if (newstate == ACT_DECIDE)
                    PetActionBar[i].Type = ACT_DISABLED;
                else
                    PetActionBar[i].Type = newstate;
            }

            return true;
        }
    }
    return false;
}

void CharmInfo::ToggleCreatureAutocast(uint32 spellid, bool apply)
{
    if (SpellMgr::IsPassiveSpell(spellid))
        return;

    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        if (spellid == m_charmspells[x].spellId)
        {
            m_charmspells[x].active = apply ? ACT_ENABLED : ACT_DISABLED;
        }
    }
}

void CharmInfo::SetPetNumber(uint32 petnumber, bool statwindow)
{
    m_petnumber = petnumber;
    if (statwindow)
        m_unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, m_petnumber);
    else
        m_unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, 0);
}

void CharmInfo::HandleStayCommand()
{
    SetCommandState(COMMAND_STAY);

    m_unit->AttackStop();
    m_unit->InterruptNonMeleeSpells(false);

    m_unit->GetMotionMaster()->StopControlledMovement();
}

void CharmInfo::HandleFollowCommand()
{
    if (m_unit->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
        return;

    SetCommandState(COMMAND_FOLLOW);

    m_unit->AttackStop();
    m_unit->InterruptNonMeleeSpells(false);

    m_unit->GetMotionMaster()->MoveFollow(m_unit->GetCharmerOrOwner(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

    if (Creature* pCharm = m_unit->ToCreature())
    {
        if (PetAI* petai = dynamic_cast<PetAI*>(pCharm->AI()))
            petai->clearEnemySet();
    }
}

void CharmInfo::HandleAttackCommand(uint64 targetGUID)
{
    Unit* pOwner = m_unit->GetCharmerOrOwner();
    // Can't attack if owner is pacified
    if (pOwner->HasAuraType(SPELL_AURA_MOD_PACIFY))
         return;

    // only place where pet can be player
    Unit *pTarget = pOwner->GetUnit(targetGUID);
    if (!pTarget)
        return;

    if (!m_unit->canAttack(pTarget, true))
         return;

    // Not let attack through obstructions
    if (sWorld.getConfig(CONFIG_PET_LOS) && !m_unit->IsWithinLOSInMap(pTarget))
        return;

    if (Creature* pCharm = m_unit->ToCreature())
    {
        if (PetAI* petai = dynamic_cast<PetAI*>(pCharm->AI()))
        {
            petai->ForcedAttackStart(pTarget);
        }
        else
            pCharm->AI()->AttackStart(pTarget);

        Pet *pPet = m_unit->ToPet();
        if (pPet)
        {
            if (pPet->getPetType() == HUNTER_PET)
            {
                pPet->UpdateSpeed(MOVE_RUN, true); // @petspeed
            }
            else if (pPet->getPetType() == SUMMON_PET && roll_chance_i(10))
            {
                // 10% chance for special talk
                pPet->SendPetTalk(uint32(PET_TALK_ATTACK));
                return;
            }
        }
    }
    else
    {
        if (m_unit->getVictimGUID() != targetGUID)
             m_unit->AttackStop();

        m_unit->Attack(pTarget, true);
    }

    m_unit->SendPetAIReaction(targetGUID);
}

void CharmInfo::HandleSpellActCommand(uint64 targetGUID, uint32 spellId)
{
    // do not cast unknown spells
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return;

    // Global Cooldown, stop cast
    if (spellInfo->StartRecoveryCategory > 0 && GetCooldownMgr().HasGlobalCooldown(spellInfo))
        return;

    for (uint32 i = 0; i < 3;i++)
         if (spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_AREA_ENEMY_SRC || spellInfo->EffectImplicitTargetA[i] == TARGET_UNIT_AREA_ENEMY_DST || spellInfo->EffectImplicitTargetA[i] == TARGET_DEST_DYNOBJ_ENEMY)
             return;

    // do not cast not learned spells
    if (!m_unit->HasSpell(spellId) || SpellMgr::IsPassiveSpell(spellId))
        return;

    uint64 charmerGUID = m_unit->GetCharmerGUID();

    Spell *spell = new Spell(m_unit, spellInfo, spellId == 33395, charmerGUID);

    Unit* pTarget = m_unit->GetUnit(targetGUID);

    SpellCastResult result = spell->CheckPetCast(pTarget);

    // auto turn to target unless possessed
    if (result == SPELL_FAILED_UNIT_NOT_INFRONT && !m_unit->isPossessed())
    {
        if (Unit *pTarget2 = targetGUID ? pTarget : spell->m_targets.getUnitTarget())
            m_unit->SetFacingToObject(pTarget2);

        result = SPELL_CAST_OK;
    }

    if (result == SPELL_CAST_OK)
    {
        Creature* pCreature = m_unit->ToCreature();

        pCreature->AddCreatureSpellCooldown(spellId);
        if (Pet* pPet = m_unit->ToPet())
        {
            pPet->CheckLearning(spellId);
            if (pPet->getPetType() == SUMMON_PET && roll_chance_i(10))
                pPet->SendPetTalk(uint32(PET_TALK_SPECIAL_SPELL));
            else
                pPet->SendPetAIReaction(charmerGUID);
        }
        else
            m_unit->SendPetAIReaction(charmerGUID);

        Unit *pSpellTarget = spell->m_targets.getUnitTarget();

        if (pSpellTarget && !m_unit->isPossessed() && !m_unit->GetCharmerOrOwner()->IsFriendlyTo(pSpellTarget))
        {
            if (m_unit->GetVictim())
                m_unit->AttackStop();

            if (pCreature->IsAIEnabled)
            {
                if (PetAI* petai = dynamic_cast<PetAI*>(pCreature->AI()))
                    petai->ForcedAttackStart(pSpellTarget);
                else
                    pCreature->AI()->AttackStart(pSpellTarget);
            }
        }

        //m_unit->GetMotionMaster()->StopMovement();
        //m_unit->GetMotionMaster()->MovementExpired(false);

        spell->prepare(&(spell->m_targets));
    }
    else
    {
        if (m_unit->isPossessed())
        {
            WorldPacket data(SMSG_CAST_FAILED, (4+1+1));
            data << uint32(spellId);
            data << uint8(2);
            data << uint8(result);

            switch (result)
            {
                case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
                    data << uint32(spellInfo->RequiresSpellFocus);
                    break;
                case SPELL_FAILED_REQUIRES_AREA:
                    data << uint32(spellInfo->AreaId);
                    break;
            }

            Player *pPlayer = m_unit->GetCharmer()->ToPlayer();
            pPlayer->SendPacketToSelf(&data);
        }
        else
            m_unit->SendPetCastFail(spellId, result);

        if (!m_unit->ToCreature()->HasSpellCooldown(spellId))
            m_unit->SendPetClearCooldown(spellId);

        spell->finish(false);
        delete spell;
    }
}
