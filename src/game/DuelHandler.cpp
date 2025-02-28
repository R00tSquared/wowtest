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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Log.h"
#include "Opcodes.h"
#include "UpdateData.h"
#include "Player.h"

void WorldSession::HandleDuelAcceptedOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,8);

    uint64 guid;
    Player *pl;
    Player *plTarget;

    if (!GetPlayer()->duel)                                  // ignore accept from duel-sender
        return;

    recvPacket >> guid;

    pl       = GetPlayer();
    plTarget = pl->duel->opponent;

    if (pl == pl->duel->initiator || !plTarget || pl == plTarget || pl->duel->startTime != 0 || plTarget->duel->startTime != 0)
        return;

    //sLog.outDebug("WORLD: received CMSG_DUEL_ACCEPTED");
    debug_log("Player 1 is: %u (%s)", pl->GetGUIDLow(),pl->GetName());
    debug_log("Player 2 is: %u (%s)", plTarget->GetGUIDLow(),plTarget->GetName());

    time_t now = time(NULL);
    pl->duel->startTimer = now;
    plTarget->duel->startTimer = now;
    pl->RemoveAurasWithInterruptFlags(pl->GetClass() == CLASS_HUNTER ? (AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP | AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_HUNTER_PVP) : AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP);
    plTarget->RemoveAurasWithInterruptFlags(plTarget->GetClass() == CLASS_HUNTER ? (AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP | AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_HUNTER_PVP) : AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP);

	// reset cooldown only on x100 or in Duel Zone on x5
    if (sWorld.isPvPArea(pl->GetCachedArea()))
    {
        if (!pl->IsInCombat() && !plTarget->IsInCombat())
        {
            pl->RemoveAurasDueToDurationAndRecoveryTime();                // DUEL UNBUFF
            plTarget->RemoveAurasDueToDurationAndRecoveryTime();        // DUEL UNBUFF
            pl->ClearDiminishings();
            plTarget->ClearDiminishings();
    
            GuardianPetList const& plguardians = pl->GetGuardians();
                for (GuardianPetList::const_iterator itr = plguardians.begin(); itr != plguardians.end(); ++itr)
                    if (Unit* guardian = Unit::GetUnit(*pl,*itr))
                        guardian->RemoveFromWorld();

            GuardianPetList const& plTargetguardians = plTarget->GetGuardians();
                for (GuardianPetList::const_iterator itr = plTargetguardians.begin(); itr != plTargetguardians.end(); ++itr)
                    if (Unit* guardian = Unit::GetUnit(*plTarget,*itr))
                        guardian->RemoveFromWorld();

            Pet *petpl = pl->GetPet();
            if (petpl)
            {
                //petpl->RemoveAllAurasButPermanent();
                petpl->RemoveAurasDueToDurationAndRecoveryTime();
                petpl->RemoveArenaAuras(true);
                petpl->m_CreatureSpellCooldowns.clear();
                petpl->m_CreatureCategoryCooldowns.clear(); 
                petpl->SetHealth(petpl->GetMaxHealth());
                if (petpl->getPowerType() == POWER_MANA)
                    petpl->SetPower(POWER_MANA, petpl->GetMaxPower(POWER_MANA));
                if (petpl->HasAura(11196, 0))
                     petpl->RemoveAurasDueToSpell(11196);    // Recently Bandaged (for everyone and pet)
                if (petpl->HasAura(30803, 0))
                     petpl->RemoveAurasDueToSpell(30803);    // Unleashed Rage proc rank 1 (only for melee classes and pet)
                if (petpl->HasAura(30804, 0))
                     petpl->RemoveAurasDueToSpell(30804);    // Unleashed Rage proc rank 2 (only for melee classes and pet)
                if (petpl->HasAura(30805, 0))
                     petpl->RemoveAurasDueToSpell(30805);    // Unleashed Rage proc rank 3 (only for melee classes and pet)
                if (petpl->HasAura(30806, 0))
                     petpl->RemoveAurasDueToSpell(30806);    // Unleashed Rage proc rank 4 (only for melee classes and pet)
                if (petpl->HasAura(30807, 0))
                     petpl->RemoveAurasDueToSpell(30807);    // Unleashed Rage proc rank 5 (only for melee classes and pet)
            }

            Pet *petplTarget = plTarget->GetPet();
            if (petplTarget)
            {
                //petplTarget->RemoveAllAurasButPermanent();
                petplTarget->RemoveAurasDueToDurationAndRecoveryTime();
                petplTarget->RemoveArenaAuras(true);
                petplTarget->m_CreatureSpellCooldowns.clear();
                petplTarget->m_CreatureCategoryCooldowns.clear(); 
                petplTarget->SetHealth(petplTarget->GetMaxHealth());
                if (petplTarget->getPowerType() == POWER_MANA)
                    petplTarget->SetPower(POWER_MANA, petplTarget->GetMaxPower(POWER_MANA));
                if (petplTarget->HasAura(11196, 0))
                     petplTarget->RemoveAurasDueToSpell(11196);    // Recently Bandaged (for everyone and pet)
                if (petplTarget->HasAura(30803, 0))
                     petplTarget->RemoveAurasDueToSpell(30803);    // Unleashed Rage proc rank 1 (only for melee classes and pet)
                if (petplTarget->HasAura(30804, 0))
                     petplTarget->RemoveAurasDueToSpell(30804);    // Unleashed Rage proc rank 2 (only for melee classes and pet)
                if (petplTarget->HasAura(30805, 0))
                     petplTarget->RemoveAurasDueToSpell(30805);    // Unleashed Rage proc rank 3 (only for melee classes and pet)
                if (petplTarget->HasAura(30806, 0))
                     petplTarget->RemoveAurasDueToSpell(30806);    // Unleashed Rage proc rank 4 (only for melee classes and pet)
                if (petplTarget->HasAura(30807, 0))
                     petplTarget->RemoveAurasDueToSpell(30807);    // Unleashed Rage proc rank 5 (only for melee classes and pet)
            }

            pl->RemoveArenaSpellCooldowns();
            plTarget->RemoveArenaSpellCooldowns();
            pl->restorePowers();
            plTarget->restorePowers();

            switch (pl->GetClass())
            {
                case CLASS_PALADIN:
                {
                    if (pl->HasAura(25771, 0))
                        pl->RemoveAurasDueToSpell(25771);    // Forbearance
                    if (pl->HasAura(20216, 0))
                        pl->RemoveAurasDueToSpell(20216);    // Divine Favor
                    if (pl->HasAura(20050, 0))
                        pl->RemoveAurasDueToSpell(20050);    // Vengeance proc rank 1
                    if (pl->HasAura(20052, 0))
                        pl->RemoveAurasDueToSpell(20052);     // Vengeance proc rank 2
                    if (pl->HasAura(20053, 0))
                        pl->RemoveAurasDueToSpell(20053);     // Vengeance proc rank 3
                    if (pl->HasAura(20054, 0))
                        pl->RemoveAurasDueToSpell(20054);     // Vengeance proc rank 4
                    if (pl->HasAura(20055, 0))
                        pl->RemoveAurasDueToSpell(20055);     // Vengeance proc rank 5
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(37658, 0))
                        pl->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_ROGUE:
                {
                    if (pl->HasAura(41425, 0))
                        pl->RemoveAurasDueToSpell(41425);     // Cold Blood
                    if (pl->HasAura(6774, 0))
                        pl->RemoveAurasDueToSpell(6774);     // Slice and Dice rank 2
                    if (pl->HasAura(5171, 0))
                        pl->RemoveAurasDueToSpell(5171);     // Slice and Dice rank 1
                    if (pl->HasAura(26888, 0))
                        pl->RemoveAurasDueToSpell(26888);     // Vanish rank 3 buff
                    if (pl->HasAura(11329, 0))
                        pl->RemoveAurasDueToSpell(11329);     // Vanish rank 2 buff
                    if (pl->HasAura(11327, 0))
                        pl->RemoveAurasDueToSpell(11327);     // Vanish rank 1 buff
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(30803, 0))
                        pl->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (pl->HasAura(30804, 0))
                        pl->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (pl->HasAura(30805, 0))
                        pl->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (pl->HasAura(30806, 0))
                        pl->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (pl->HasAura(30807, 0))
                        pl->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_WARRIOR:
                {
                    if (pl->HasAura(12970, 0))
                        pl->RemoveAurasDueToSpell(12970);     // Flurry rank 5
                    if (pl->HasAura(12969, 0))
                        pl->RemoveAurasDueToSpell(12969);     // Flurry rank 4
                    if (pl->HasAura(12968, 0))
                        pl->RemoveAurasDueToSpell(12968);     // Flurry rank 3
                    if (pl->HasAura(12967, 0))
                        pl->RemoveAurasDueToSpell(12967);     // Flurry rank 2
                    if (pl->HasAura(12966, 0))
                        pl->RemoveAurasDueToSpell(12966);     // Flurry rank 1
                    if (pl->HasAura(12880, 0))
                        pl->RemoveAurasDueToSpell(12880);     // Enrage rank 1
                    if (pl->HasAura(14201, 0))
                        pl->RemoveAurasDueToSpell(14201);     // Enrage rank 2
                    if (pl->HasAura(14202, 0))
                        pl->RemoveAurasDueToSpell(14202);     // Enrage rank 3
                    if (pl->HasAura(14203, 0))
                        pl->RemoveAurasDueToSpell(14203);     // Enrage rank 4
                    if (pl->HasAura(14204, 0))
                        pl->RemoveAurasDueToSpell(14204);     // Enrage rank 5
                    if (pl->HasAura(29801, 0))
                        pl->RemoveAurasDueToSpell(29801);     // Rampage rank 1
                    if (pl->HasAura(30029, 0))
                        pl->RemoveAurasDueToSpell(30029);     // Rampage proc rank 1
                    if (pl->HasAura(30030, 0))
                        pl->RemoveAurasDueToSpell(30030);     // Rampage rank 2
                    if (pl->HasAura(30031, 0))
                        pl->RemoveAurasDueToSpell(30031);     // Rampage proc rank 2
                    if (pl->HasAura(30032, 0))
                        pl->RemoveAurasDueToSpell(30032);     // Rampage rank 3
                    if (pl->HasAura(30033, 0))
                        pl->RemoveAurasDueToSpell(30033);     // Rampage proc rank 3
                    if (pl->HasAura(29131, 0))
                        pl->RemoveAurasDueToSpell(29131);     // Bloodrage buff
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(30803, 0))
                        pl->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (pl->HasAura(30804, 0))
                        pl->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (pl->HasAura(30805, 0))
                        pl->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (pl->HasAura(30806, 0))
                        pl->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (pl->HasAura(30807, 0))
                        pl->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_PRIEST:
                {
                    if (pl->HasAura(14751, 0))
                        pl->RemoveAurasDueToSpell(14751);     // Inner Focus
                    if (pl->HasAura(33151, 0))
                        pl->RemoveAurasDueToSpell(33151);     // Surge of Light proc
                    if (pl->HasAura(34754, 0))
                        pl->RemoveAurasDueToSpell(34754);     // Holy Concentration proc
                    if (pl->HasAura(33143, 0))
                        pl->RemoveAurasDueToSpell(33143);     // Blessed Resilience proc
                    if (pl->HasAura(15271, 0))
                        pl->RemoveAurasDueToSpell(15271);     // Spirit Tap proc
                    if (pl->HasAura(45237, 0))
                        pl->RemoveAurasDueToSpell(45237);     // Focused Will proc rank 1
                    if (pl->HasAura(45241, 0))
                        pl->RemoveAurasDueToSpell(45241);     // Focused Will proc rank 2
                    if (pl->HasAura(45242, 0))
                        pl->RemoveAurasDueToSpell(45242);     // Focused Will proc rank 3
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(37658, 0))
                        pl->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_SHAMAN:
                {
                    if (pl->HasAura(16166, 0))
                        pl->RemoveAurasDueToSpell(16166);     // Elemental Mastery
                    if (pl->HasAura(16168, 0))
                        pl->RemoveAurasDueToSpell(16168);     // Nature's Swiftness
                    if (pl->HasAura(16246, 0))
                        pl->RemoveAurasDueToSpell(16246);     // Elemental Focus
                    if (pl->HasAura(29063, 0))
                        pl->RemoveAurasDueToSpell(29063);     // Eye of the Storm proc
                    if (pl->HasAura(29177, 0))
                        pl->RemoveAurasDueToSpell(29177);     // Elemental devastation proc rank 1
                    if (pl->HasAura(29178, 0))
                        pl->RemoveAurasDueToSpell(29178);     // Elemental devastation proc rank 2
                    if (pl->HasAura(30165, 0))
                        pl->RemoveAurasDueToSpell(30165);     // Elemental devastation proc rank 3
                    if (pl->HasAura(43339, 0))
                        pl->RemoveAurasDueToSpell(43339);     // Shamanistic Focus proc
                    if (pl->HasAura(16257, 0))
                        pl->RemoveAurasDueToSpell(16257);     // Flurry proc rank 1
                    if (pl->HasAura(16277, 0))
                        pl->RemoveAurasDueToSpell(16277);     // Flurry proc rank 2
                    if (pl->HasAura(16278, 0))
                        pl->RemoveAurasDueToSpell(16278);     // Flurry proc rank 3
                    if (pl->HasAura(16279, 0))
                        pl->RemoveAurasDueToSpell(16279);     // Flurry proc rank 4
                    if (pl->HasAura(16280, 0))
                        pl->RemoveAurasDueToSpell(16280);     // Flurry proc rank 5
                    if (pl->HasAura(37658, 0))
                        pl->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(30803, 0))
                        pl->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (pl->HasAura(30804, 0))
                        pl->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (pl->HasAura(30805, 0))
                        pl->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (pl->HasAura(30806, 0))
                        pl->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (pl->HasAura(30807, 0))
                        pl->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_MAGE:
                {
                    if (pl->HasAura(28682, 0))
                        pl->RemoveAurasDueToSpell(28682);     // Combustion
                    if (pl->HasAura(41425, 0))
                        pl->RemoveAurasDueToSpell(41425);     // Hypothermia
                    if (pl->HasAura(32612, 0))
                        pl->RemoveAurasDueToSpell(32612);     // Invisibility
                    if (pl->HasAura(12043, 0))
                        pl->RemoveAurasDueToSpell(12043);     // Presence of Mind
                    if (pl->HasAura(12536, 0))
                        pl->RemoveAurasDueToSpell(12536);     // Clearcasting
                    if (pl->HasAura(31643, 0))
                        pl->RemoveAurasDueToSpell(31643);     // Blazing Speed
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(37658, 0))
                        pl->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_WARLOCK:
                {
                    if (pl->HasAura(17941, 0))
                        pl->RemoveAurasDueToSpell(17941);     // Nightfall proc
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(37658, 0))
                        pl->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_HUNTER:
                {
                    if (pl->HasAura(34471, 0))
                        pl->RemoveAurasDueToSpell(34471);     // The Beast Within
                    if (pl->HasAura(6150, 0))
                        pl->RemoveAurasDueToSpell(6150);         // Improved Aspect of the Hawk proc
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(30803, 0))
                        pl->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (pl->HasAura(30804, 0))
                        pl->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (pl->HasAura(30805, 0))
                        pl->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (pl->HasAura(30806, 0))
                        pl->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (pl->HasAura(30807, 0))
                        pl->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_DRUID:
                {
                    if (pl->HasAura(42581, 0))
                        pl->RemoveAurasDueToSpell(45281);     // Natural Perfection Proc
                    if (pl->HasAura(45282, 0))
                        pl->RemoveAurasDueToSpell(45282);     // Natural Perfection Proc
                    if (pl->HasAura(45283, 0))
                        pl->RemoveAurasDueToSpell(45283);     // Natural Perfection Proc
                    if (pl->HasAura(16886, 0))
                        pl->RemoveAurasDueToSpell(16886);     // Nature's Grace
                    if (pl->HasAura(17116, 0))
                        pl->RemoveAurasDueToSpell(17116);     // Nature's Swiftnes
                    if (pl->HasAura(37658, 0))
                        pl->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    if (pl->HasAura(11196, 0))
                        pl->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (pl->HasAura(30803, 0))
                        pl->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (pl->HasAura(30804, 0))
                        pl->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (pl->HasAura(30805, 0))
                        pl->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (pl->HasAura(30806, 0))
                        pl->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (pl->HasAura(30807, 0))
                        pl->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
            }

            switch (plTarget->GetClass())
            {
                case CLASS_PALADIN:
                {
                    if (plTarget->HasAura(25771, 0))
                        plTarget->RemoveAurasDueToSpell(25771);     // Forbearance
                    if (plTarget->HasAura(20216, 0))
                        plTarget->RemoveAurasDueToSpell(20216);     // Divine Favor
                    if (plTarget->HasAura(20050, 0))
                        plTarget->RemoveAurasDueToSpell(20050);     // Vengeance proc rank 1
                    if (plTarget->HasAura(20052, 0))
                        plTarget->RemoveAurasDueToSpell(20052);     // Vengeance proc rank 2
                    if (plTarget->HasAura(20053, 0))
                        plTarget->RemoveAurasDueToSpell(20053);     // Vengeance proc rank 3
                    if (plTarget->HasAura(20054, 0))
                        plTarget->RemoveAurasDueToSpell(20054);     // Vengeance proc rank 4
                    if (plTarget->HasAura(20055, 0))
                        plTarget->RemoveAurasDueToSpell(20055);     // Vengeance proc rank 5
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(37658, 0))
                        plTarget->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_ROGUE:
                {
                    if (plTarget->HasAura(41425, 0))
                        plTarget->RemoveAurasDueToSpell(41425);     // Cold Blood
                    if (plTarget->HasAura(6774, 0))
                        plTarget->RemoveAurasDueToSpell(6774);     // Slice and Dice rank 2
                    if (plTarget->HasAura(5171, 0))
                        plTarget->RemoveAurasDueToSpell(5171);     // Slice and Dice rank 1
                    if (plTarget->HasAura(26888, 0))
                        plTarget->RemoveAurasDueToSpell(26888);     // Vanish rank 3 buff
                    if (plTarget->HasAura(11329, 0))
                        plTarget->RemoveAurasDueToSpell(11329);     // Vanish rank 2 buff
                    if (plTarget->HasAura(11327, 0))
                        plTarget->RemoveAurasDueToSpell(11327);     // Vanish rank 1 buff
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(30803, 0))
                        plTarget->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (plTarget->HasAura(30804, 0))
                        plTarget->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (plTarget->HasAura(30805, 0))
                        plTarget->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (plTarget->HasAura(30806, 0))
                        plTarget->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (plTarget->HasAura(30807, 0))
                        plTarget->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_WARRIOR:
                {
                    if (plTarget->HasAura(12970, 0))
                        plTarget->RemoveAurasDueToSpell(12970);     // Flurry rank 5
                    if (plTarget->HasAura(12969, 0))
                        plTarget->RemoveAurasDueToSpell(12969);     // Flurry rank 4
                    if (plTarget->HasAura(12968, 0))
                        plTarget->RemoveAurasDueToSpell(12968);     // Flurry rank 3
                    if (plTarget->HasAura(12967, 0))
                        plTarget->RemoveAurasDueToSpell(12967);     // Flurry rank 2
                    if (plTarget->HasAura(12966, 0))
                        plTarget->RemoveAurasDueToSpell(12966);     // Flurry rank 1
                    if (plTarget->HasAura(12880, 0))
                        plTarget->RemoveAurasDueToSpell(12880);     // Enrage rank 1
                    if (plTarget->HasAura(14201, 0))
                        plTarget->RemoveAurasDueToSpell(14201);     // Enrage rank 2
                    if (plTarget->HasAura(14202, 0))
                        plTarget->RemoveAurasDueToSpell(14202);     // Enrage rank 3
                    if (plTarget->HasAura(14203, 0))
                        plTarget->RemoveAurasDueToSpell(14203);     // Enrage rank 4
                    if (plTarget->HasAura(14204, 0))
                        plTarget->RemoveAurasDueToSpell(14204);     // Enrage rank 5
                    if (plTarget->HasAura(29801, 0))
                        plTarget->RemoveAurasDueToSpell(29801);     // Rampage rank 1
                    if (plTarget->HasAura(30029, 0))
                        plTarget->RemoveAurasDueToSpell(30029);     // Rampage proc rank 1
                    if (plTarget->HasAura(30030, 0))
                        plTarget->RemoveAurasDueToSpell(30030);     // Rampage rank 2
                    if (plTarget->HasAura(30031, 0))
                        plTarget->RemoveAurasDueToSpell(30031);     // Rampage proc rank 2
                    if (plTarget->HasAura(30032, 0))
                        plTarget->RemoveAurasDueToSpell(30032);     // Rampage rank 3
                    if (plTarget->HasAura(30033, 0))
                        plTarget->RemoveAurasDueToSpell(30033);     // Rampage proc rank 3
                    if (plTarget->HasAura(29131, 0))
                        plTarget->RemoveAurasDueToSpell(29131);     // Bloodrage buff
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(30803, 0))
                        plTarget->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (plTarget->HasAura(30804, 0))
                        plTarget->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (plTarget->HasAura(30805, 0))
                        plTarget->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (plTarget->HasAura(30806, 0))
                        plTarget->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (plTarget->HasAura(30807, 0))
                        plTarget->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_PRIEST:
                {
                    if (plTarget->HasAura(14751, 0))
                        plTarget->RemoveAurasDueToSpell(14751);     // Inner Focus
                    if (plTarget->HasAura(33151, 0))
                        plTarget->RemoveAurasDueToSpell(33151);     // Surge of Light proc
                    if (plTarget->HasAura(34754, 0))
                        plTarget->RemoveAurasDueToSpell(34754);     // Holy Concentration proc
                    if (plTarget->HasAura(33143, 0))
                        plTarget->RemoveAurasDueToSpell(33143);     // Blessed Resilience proc
                    if (plTarget->HasAura(15271, 0))
                        plTarget->RemoveAurasDueToSpell(15271);     // Spirit Tap proc
                    if (plTarget->HasAura(45237, 0))
                        plTarget->RemoveAurasDueToSpell(45237);     // Focused Will proc rank 1
                    if (plTarget->HasAura(45241, 0))
                        plTarget->RemoveAurasDueToSpell(45241);     // Focused Will proc rank 2
                    if (plTarget->HasAura(45242, 0))
                        plTarget->RemoveAurasDueToSpell(45242);     // Focused Will proc rank 3
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(37658, 0))
                        plTarget->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_SHAMAN:
                {
                    if (plTarget->HasAura(16166, 0))
                        plTarget->RemoveAurasDueToSpell(16166);     // Elemental Mastery
                    if (plTarget->HasAura(16168, 0))
                        plTarget->RemoveAurasDueToSpell(16168);     // Nature's Swiftness
                    if (plTarget->HasAura(16246, 0))
                        plTarget->RemoveAurasDueToSpell(16246);     // Elemental Focus
                    if (plTarget->HasAura(29063, 0))
                        plTarget->RemoveAurasDueToSpell(29063);     // Eye of the Storm proc
                    if (plTarget->HasAura(29177, 0))
                        plTarget->RemoveAurasDueToSpell(29177);     // Elemental devastation proc rank 1
                    if (plTarget->HasAura(29178, 0))
                        plTarget->RemoveAurasDueToSpell(29178);     // Elemental devastation proc rank 2
                    if (plTarget->HasAura(30165, 0))
                        plTarget->RemoveAurasDueToSpell(30165);     // Elemental devastation proc rank 3
                    if (plTarget->HasAura(43339, 0))
                        plTarget->RemoveAurasDueToSpell(43339);     // Shamanistic Focus proc
                    if (plTarget->HasAura(16257, 0))
                        plTarget->RemoveAurasDueToSpell(16257);     // Flurry proc rank 1
                    if (plTarget->HasAura(16277, 0))
                        plTarget->RemoveAurasDueToSpell(16277);     // Flurry proc rank 2
                    if (plTarget->HasAura(16278, 0))
                        plTarget->RemoveAurasDueToSpell(16278);     // Flurry proc rank 3
                    if (plTarget->HasAura(16279, 0))
                        plTarget->RemoveAurasDueToSpell(16279);     // Flurry proc rank 4
                    if (plTarget->HasAura(16280, 0))
                        plTarget->RemoveAurasDueToSpell(16280);     // Flurry proc rank 5
                    if (plTarget->HasAura(37658, 0))
                        plTarget->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(30803, 0))
                        plTarget->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (plTarget->HasAura(30804, 0))
                        plTarget->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (plTarget->HasAura(30805, 0))
                        plTarget->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (plTarget->HasAura(30806, 0))
                        plTarget->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (plTarget->HasAura(30807, 0))
                        plTarget->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_MAGE:
                {
                    if (plTarget->HasAura(28682, 0))
                        plTarget->RemoveAurasDueToSpell(28682);     // Combustion
                    if (plTarget->HasAura(41425, 0))
                        plTarget->RemoveAurasDueToSpell(41425);     // Hypothermia
                    if (plTarget->HasAura(32612, 0))
                        plTarget->RemoveAurasDueToSpell(32612);     // Invisibility
                    if (plTarget->HasAura(12043, 0))
                        plTarget->RemoveAurasDueToSpell(12043);     // Presence of Mind
                    if (plTarget->HasAura(12536, 0))
                        plTarget->RemoveAurasDueToSpell(12536);     // Clearcasting
                    if (plTarget->HasAura(31643, 0))
                        plTarget->RemoveAurasDueToSpell(31643);     // Blazing Speed
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(37658, 0))
                        plTarget->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_WARLOCK:
                {
                    if (plTarget->HasAura(17941, 0))
                        plTarget->RemoveAurasDueToSpell(17941);     // Nightfall proc
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(37658, 0))
                        plTarget->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    break;
                }
                case CLASS_HUNTER:
                {
                    if (plTarget->HasAura(34471, 0))
                        plTarget->RemoveAurasDueToSpell(34471);     // The Beast Within
                    if (plTarget->HasAura(6150, 0))
                        plTarget->RemoveAurasDueToSpell(6150);         // Improved Aspect of the Hawk proc
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(30803, 0))
                        plTarget->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (plTarget->HasAura(30804, 0))
                        plTarget->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (plTarget->HasAura(30805, 0))
                        plTarget->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (plTarget->HasAura(30806, 0))
                        plTarget->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (plTarget->HasAura(30807, 0))
                        plTarget->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
                case CLASS_DRUID:
                {
                    if (plTarget->HasAura(42581, 0))
                        plTarget->RemoveAurasDueToSpell(45281);     // Natural Perfection Proc
                    if (plTarget->HasAura(45282, 0))
                        plTarget->RemoveAurasDueToSpell(45282);     // Natural Perfection Proc
                    if (plTarget->HasAura(45283, 0))
                        plTarget->RemoveAurasDueToSpell(45283);     // Natural Perfection Proc
                    if (plTarget->HasAura(16886, 0))
                        plTarget->RemoveAurasDueToSpell(16886);     // Nature's Grace
                    if (plTarget->HasAura(17116, 0))
                        plTarget->RemoveAurasDueToSpell(17116);     // Nature's Swiftnes
                    if (plTarget->HasAura(37658, 0))
                        plTarget->RemoveAurasDueToSpell(37658);     // Lightning Capacitor Charges (only for spell classes)
                    if (plTarget->HasAura(11196, 0))
                        plTarget->RemoveAurasDueToSpell(11196);     // Recently Bandaged (for everyone and pet)
                    if (plTarget->HasAura(30803, 0))
                        plTarget->RemoveAurasDueToSpell(30803);     // Unleashed Rage proc rank 1 (only for melee classes and pet)
                    if (plTarget->HasAura(30804, 0))
                        plTarget->RemoveAurasDueToSpell(30804);     // Unleashed Rage proc rank 2 (only for melee classes and pet)
                    if (plTarget->HasAura(30805, 0))
                        plTarget->RemoveAurasDueToSpell(30805);     // Unleashed Rage proc rank 3 (only for melee classes and pet)
                    if (plTarget->HasAura(30806, 0))
                        plTarget->RemoveAurasDueToSpell(30806);     // Unleashed Rage proc rank 4 (only for melee classes and pet)
                    if (plTarget->HasAura(30807, 0))
                        plTarget->RemoveAurasDueToSpell(30807);     // Unleashed Rage proc rank 5 (only for melee classes and pet)
                    break;
                }
            }

            if (pl->GetRace() == RACE_TROLL)
            {
                if (pl->HasAura(26635, 0))
                    pl->RemoveAurasDueToSpell(26635);    // Berserking
            }
            else if (pl->GetRace() == RACE_BLOODELF)
            {
                if (pl->HasAura(28734, 0))
                    pl->RemoveAurasDueToSpell(28734);    // Mana Tap
            }
            if (plTarget->GetRace() == RACE_TROLL)
            {
                if (plTarget->HasAura(26635, 0))
                    plTarget->RemoveAurasDueToSpell(26635);    // Berserking
            }
            else if (plTarget->GetRace() == RACE_BLOODELF)
            {
                if (plTarget->HasAura(28734, 0))
                    plTarget->RemoveAurasDueToSpell(28734);    // Mana Tap
            }
        }
    }

    WorldPacket data(SMSG_DUEL_COUNTDOWN, 4);
    data << (uint32)3000;                                   // 3 seconds
    pl->SendPacketToSelf(&data);
    plTarget->SendPacketToSelf(&data);
}

void WorldSession::HandleDuelCancelledOpcode(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,8);

    //sLog.outDebug("WORLD: received CMSG_DUEL_CANCELLED");

    // no duel requested
    if (!GetPlayer()->duel)
        return;

    // player surrendered in a duel using /forfeit
    if (GetPlayer()->duel->startTime != 0)
    {
        if (Player* temp = GetPlayer()->duel->opponent)
        {
            temp->CombatStopWithPets(true);
            if (sWorld.isEasyRealm())
            {
                temp->CastSpell(GetPlayer()->duel->opponent, 54648, true);    // Duel won heal
                GetPlayer()->CastSpell(GetPlayer(), 54648, true);    // heal
            }
            GetPlayer()->CombatStopWithPets(true);
            GetPlayer()->CastSpell(GetPlayer(), 7267, true);    // beg
            GetPlayer()->DuelComplete(DUEL_WON);
            GetPlayer()->GetCamera().UpdateVisibilityForOwner(); // should remove bug with no-tablet duel
            temp->GetCamera().UpdateVisibilityForOwner(); // should remove bug with no-tablet duel
            return;
        }
    }

    // player either discarded the duel using the "discard button"
    // or used "/forfeit" before countdown reached 0
    uint64 guid;
    recvPacket >> guid;

    GetPlayer()->DuelComplete(DUEL_INTERUPTED);
}
