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

#ifndef BOSS_KILJAEDEN_H
#define BOSS_KILJAEDEN_H

#include "precompiled.h"

/*** Speech and sounds***/
enum Speeches
{
    // Felmyst outro
    YELL_KALECGOS       = -1580043, //after felmyst's death spawned and say this

    // These are used throughout Sunwell and Magisters(?). Players can hear this while running through the instances.
    SAY_KJ_OFFCOMBAT1   = -1580066,
    SAY_KJ_OFFCOMBAT2   = -1580067,
    SAY_KJ_OFFCOMBAT3   = -1580068,
    SAY_KJ_OFFCOMBAT4   = -1580069,
    SAY_KJ_OFFCOMBAT5   = -1580070,

    // Encounter speech and sounds
    SAY_KJ_EMERGE       = -1580071,
    SAY_KJ_SLAY1        = -1580072,
    SAY_KJ_SLAY2        = -1580073,
    SAY_KJ_REFLECTION1  = -1580074,
    SAY_KJ_REFLECTION2  = -1580075,
    SAY_KJ_DARKNESS1    = -1580076,
    SAY_KJ_DARKNESS2    = -1580077,
    SAY_KJ_DARKNESS3    = -1580078,
    SAY_KJ_PHASE3       = -1580079,
    SAY_KJ_PHASE4       = -1580080,
    SAY_KJ_PHASE5       = -1580081,
    SAY_KJ_DEATH        = -1580093,
    EMOTE_KJ_DARKNESS   = -1580094,

    /*** Kalecgos - Anveena speech at the beginning of Phase 5; Anveena's sacrifice ***/
    SAY_KALECGOS_AWAKEN     = -1580082,
    SAY_ANVEENA_IMPRISONED  = -1580083,
    SAY_KALECGOS_LETGO      = -1580084,
    SAY_ANVEENA_LOST        = -1580085,
    SAY_KALECGOS_FOCUS      = -1580086,
    SAY_ANVEENA_KALEC       = -1580087,
    SAY_KALECGOS_FATE       = -1580088,
    SAY_ANVEENA_GOODBYE     = -1580089,
    SAY_KALECGOS_GOODBYE    = -1580090,
    SAY_KALECGOS_ENCOURAGE  = -1580091,

    /*** Kalecgos says throughout the fight ***/
    SAY_KALECGOS_JOIN       = -1580092,
    SAY_KALEC_ORB_READY1    = -1580095,
    SAY_KALEC_ORB_READY2    = -1580096,
    SAY_KALEC_ORB_READY3    = -1580097,
    SAY_KALEC_ORB_READY4    = -1580098,
    // outro
    SAY_OUTRO_1                 = -1580099,         // Velen
    SAY_OUTRO_2                 = -1580100,
    SAY_OUTRO_3                 = -1900257,
    SAY_OUTRO_4                 = -1580101,
    SAY_OUTRO_5                 = -1580107,         // Liadrin
    SAY_OUTRO_6                 = -1580102,         // Velen
    SAY_OUTRO_7                 = -1580108,         // Liadrin
    SAY_OUTRO_8                 = -1580103,         // Velen
    SAY_OUTRO_9                 = -1580104,
    SAY_OUTRO_10                = -1580109,         // Liadrin
    SAY_OUTRO_11                = -1580105,         // Velen
    SAY_OUTRO_12                = -1580106
};

/*** Spells used during the encounter ***/
enum SpellIds
{
    /* Hand of the Deceiver's spells and cosmetics */
    SPELL_SHADOW_BOLT_VOLLEY                            = 45770, // ~30 yard range Shadow Bolt Volley for ~2k(?) damage
    SPELL_SHADOW_INFUSION                               = 45772, // They gain this at 20% - Immunity to Stun/Silence and makes them look angry!
    SPELL_FELFIRE_PORTAL                                = 46875, // Creates a portal that spawns Felfire Fiends (LIVE FOR THE SWARM!1 FOR THE OVERMIND!)
    SPELL_SHADOW_CHANNELING                             = 46757, // Channeling animation out of combat

    /* Volatile Felfire Fiend's spells */
    SPELL_FELFIRE_FISSION                               = 45779, // Felfire Fiends explode when they die or get close to target.
    
    /* Shield Orb Spells*/
    SPELL_SHADOW_BOLT                                   = 45680, //45679 would be correct but triggers to often //TODO fix console error

    /* Kil'Jaeden's spells and cosmetics */
    SPELL_TRANS                                         = 23188, // Surprisingly, this seems to be the right spell.. (Where is it used?)
    SPELL_REBIRTH                                       = 44200, // Emerge from the Sunwell
    SPELL_SOUL_FLAY                                     = 45442, // 9k Shadow damage over 3 seconds. Spammed throughout all the fight.
    SPELL_SOUL_FLAY_SLOW                                = 47106,
    SPELL_LEGION_LIGHTNING                              = 45664, // Chain Lightning, 4 targets, ~3k Shadow damage, 1.5k mana burn. 1.5 sec cast time
    SPELL_FIRE_BLOOM                                    = 45641, // Places a debuff on 5 raid members, which causes them to deal 2k Fire damage to nearby allies and selves. MIGHT NOT WORK. 1 sec cast time
    SPELL_SUNWELL_KNOCKBACK                             = 40191,
    SPELL_SINISTER_REFLECTION                           = 45785, // Summon shadow copies of 5 raid members that fight against KJ's enemies
    SPELL_SINISTER_REFLECTION_ENLARGE                   = 45893,
    SPELL_COPY_WEAPON                                   = 41055, // }
    SPELL_COPY_WEAPON2                                  = 41054, // }
    SPELL_COPY_OFFHAND                                  = 45206, // }- Spells used in Sinister Reflection creation
    SPELL_COPY_OFFHAND_WEAPON                           = 45205, // }

    SPELL_SHADOW_SPIKE                                  = 46680, // Bombard random raid members with Shadow Spikes (Very similar to Void Reaver orbs). For 28 sec every 3 sec.
    SPELL_FLAME_DART                                    = 45737, // Bombards the raid with flames every 3(?) seconds. 1 sec cast time
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS                  = 46605, // Begins a 8-second channeling, after which he will deal 50'000 damage to the raid
    SPELL_DARKNESS_OF_A_THOUSAND_SOULS_DAMAGE           = 45657, // called from periodic aura
    
    /* Armageddon spells wrong visual */
    SPELL_ARMAGEDDON_TRIGGER                            = 45909, // Meteor spell trigger missile should cast creature on himself
    SPELL_ARMAGEDDON_VISUAL                             = 45911, // Does the hellfire visual to indicate where the meteor missle lands
    SPELL_ARMAGEDDON_VISUAL2                            = 45914, // Does the light visual to indicate where the meteor missle lands
    SPELL_ARMAGEDDON_VISUAL3                            = 24207, // This shouldn't correct but same as seen on the movie
    SPELL_ARMAGEDDON_SUMMON_TRIGGER                     = 45921, // Summons the triggers that cast the spells on himself need random target select
    SPELL_ARMAGEDDON_DAMAGE                             = 45915, // This does the area damage

    /* Anveena's spells and cosmetics (Or, generally, everything that has "Anveena" in name) */
    SPELL_ANVEENA_PRISON                                = 46367, // She hovers locked within a bubble
    SPELL_ANVEENA_ENERGY_DRAIN                          = 46410, // Sunwell energy glow animation (Control mob uses this)
    SPELL_SACRIFICE_OF_ANVEENA                          = 46474, // This is cast on Kil'Jaeden when Anveena sacrifices herself into the Sunwell

    /* Sinister Reflection Spells */
    SPELL_SR_CURSE_OF_AGONY                             = 46190, // Warlock
    SPELL_SR_SHADOW_BOLT                                = 47076,
    SPELL_SR_EARTH_SHOCK                                = 47071, // Shaman
    SPELL_SR_FIREBALL                                   = 47074, // Mage
    SPELL_SR_HEMORRHAGE                                 = 45897, // Rogue
    SPELL_SR_HOLY_SHOCK                                 = 38921, // Paladin
    SPELL_SR_HAMMER_OF_JUSTICE                          = 37369,
    SPELL_SR_HOLY_SMITE                                 = 47077, // Priest
    SPELL_SR_RENEW                                      = 47079,
    SPELL_SR_SHOOT                                      = 16496, // Hunter
    SPELL_SR_MULTI_SHOT                                 = 48098,
    SPELL_SR_WING_CLIP                                  = 40652, // Reduce movement speed by 30%
    SPELL_SR_WHIRLWIND                                  = 17207, // Warrior
    SPELL_SR_MOONFIRE                                   = 47072, // Druid

    /*** Other Spells (used by players, etc) ***/
    SPELL_ARCANE_BOLT                                   = 45670, // used by Kalec
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT                  = 45839, // Possess the blue dragon from the orb to help the raid.
    SPELL_RING_OF_BLUE_FLAMES                           = 45825, // Cast this spell when the go is activated
    SPELL_POSSESS_DRAKE_IMMUNE                          = 45838, // Gives immunity to dragon controller
    SPELL_KILL_DRAKES                                   = 46707, // Used on wipe to kill remaining drakes
    // Outro
    SPELL_TELEPORT_VISUAL       = 41232,
    SPELL_KALEC_TELEPORT        = 46473, // Teleport and transforms Kalec in human form
    SPELL_CALL_ENTROPIUS        = 46818,
    SPELL_ENTROPIUS_BODY        = 46819,
    SPELL_BLAZE_TO_LIGHT        = 46821,
    SPELL_SUNWELL_IGNITION      = 46822,
    SPELL_SHADOW_PORTAL_STATE   = 42047,
    SPELL_PORTAL_OPENING        = 46801
};

enum CreatureIds
{
    CREATURE_ANVEENA                        = 26046, // Embodiment of the Sunwell
    CREATURE_KALECGOS                       = 25319, // Helps the raid throughout the fight
    CREATURE_KILJAEDEN                      = 25315, // Give it to 'em KJ!
    CREATURE_HAND_OF_THE_DECEIVER           = 25588, // Adds found before KJ emerges
    CREATURE_FELFIRE_PORTAL                 = 25603, // Portal spawned be Hand of the Deceivers
    CREATURE_VOLATILE_FELFIRE_FIEND         = 25598, // Fiends spawned by the above portal
    CREATURE_ARMAGEDDON_TARGET              = 25735, // This mob casts meteor on itself.. I think
    CREATURE_SHIELD_ORB                     = 25502, // Shield orbs circle the room raining shadow bolts on raid
    CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT = 25653, // NPC that players possess when using the Orb of the Blue Dragonflight
    CREATURE_SPIKE_TARGET1                  = 30598, // Should summon these under Shadow Spike Channel on targets place
    CREATURE_SINISTER_REFLECTION            = 25708, // Sinister Relection spawnd on Phase swichtes
    CREATURE_BOSS_PORTAL                    = 24925, // Portal for Outro
    CREATURE_KILJAEDEN_CONTROLLER           = 25608,

    // unused
    CREATURE_SPIKE_TARGET2                  = 30614, // unused
    
    // outro
    CREATURE_CORE_ENTROPIUS                 = 26262, // NPC in the Center(summon purple crystal, light)
    CREATURE_SOLDIER                        = 26259, // Summoned in 2 waves before Velen. Should move into 2 circle formations
    CREATURE_RIFTWALKER                     = 26289, // They call portal visually
    CREATURE_VELEN                          = 26246, // Come from portal
    CREATURE_LIADRIN                        = 26247, // Come from portal
    CREATURE_SOLDIER_TARGET                 = 99989, // To make our soldiers look into the center of circle
};

/*** GameObjects ***/
#define GAMEOBJECT_ORB_OF_THE_BLUE_DRAGONFLIGHT 188415

/*** Others ***/
#define FLOOR_Z 28.050388
#define SHIELD_ORB_Z 45.000

enum Phase
{
    PHASE_DECEIVERS     = 1, // Fight 3 adds
    PHASE_NORMAL        = 2, // Kil'Jaeden emerges from the sunwell
    PHASE_DARKNESS      = 3, // At 85%, he gains few abilities; Kalecgos joins the fight
    PHASE_ARMAGEDDON    = 4, // At 55%, he gains even more abilities
    PHASE_SACRIFICE     = 5, // At 25%, Anveena sacrifices herself into the Sunwell; at this point he becomes enraged and has *significally* shorter cooldowns.
};

// Timers are set by priority. 0 is the most prioritized.
enum KilJaedenTimers
{
    P_2_TIMER_KALEC_JOIN, // instant, does not break cast
    P_3_TIMER_ORBS_EMPOWER, // instant, does not break cast
    P_3_TIMER_DARKNESS, // starts darkness, stops timers below from being updated for 8 sec
    P_2_TIMER_SUMMON_SHILEDORB, // instant, does not break cast,
    P_3_TIMER_SHADOW_SPIKE, // starts shadow spike, stops timers below from being updated for 28 sec.
    P_4_TIMER_ARMAGEDDON, // instant, does not break cast. Not going to be updated when shadow-spiking the raid
    P_2_TIMER_LEGION_LIGHTNING, // 1.5 sec cast time
    P_2_TIMER_FIRE_BLOOM, // 1 sec cast time
    P_3_TIMER_FLAME_DART, // 1 sec cast time

    TIMER_KJ_MAX
};

// Locations of the Hand of Deceiver adds
float DeceiverLocations[3][3]=
{
    {1682.045, 631.299, 5.936},
    {1684.099, 618.848, 0.589},
    {1694.170, 612.272, 1.416},
};

// Locations, where Shield Orbs will spawn
float ShieldOrbLocations[4][2] =
{
    {1698.900, 627.870},  //middle pont of Sunwell
    {(3.14f * 0.75f), 17.0f }, 
    {(3.14f * 1.25f), 17.0f },
    {(3.14f * 1.75f), 17.0f }
};

struct Speech
{
    int32 textid;
    uint32 creature, timer;
};

struct Locations
{
    float x, y, z, o;
};

static Locations aOutroLocations[] =
{
    {1727.854, 656.060, 28.31, 3.86}, // portal summon loc
    {1716.969, 646.407, 28.05, 3.91}, // velen summon loc
    {1718.862, 644.528, 28.05, 3.87}, // liadrin summon loc
    {1712.110, 641.044, 27.80},       // velen move forward
    {1711.537, 637.600, 27.34}        // liadrin move forward
};

static Locations SpawnPosition[]=
{
    {1691.760132, 656.543701, 28.051085, 4.878134},
    {1698.773682, 661.876953, 28.051085, 4.697074},
    {1706.504272, 656.923035, 28.051085, 4.47324},
    {1715.569092, 659.07019 , 28.051085, 4.253326},
    {1717.812744, 651.117615, 28.051085, 4.09232},
    {1726.173096, 650.366638, 28.051085, 3.821357},
    {1724.794556, 643.699463, 28.051085, 3.734962},
    {1731.064697, 640.627319, 28.051085, 3.475781},
    {1729.090332, 633.627747, 28.051085, 3.140416},
    {1735.229126, 627.13678, 28.051085 ,3.082167}
};

static Locations MovePositionRight[]=
{
    {1719.091309, 607.821106, 28.050314, 5.283991},
    {1721.384033, 608.608337, 28.050314, 4.396486},
    {1723.667725, 607.596558, 28.050314, 3.870272},
    {1725.043701, 605.320557, 28.050314, 3.36369 },
    {1724.521973, 602.580566, 28.050314, 2.476189},
    {1722.180908, 601.124878, 28.050314, 1.769331},
    {1719.926025, 601.365051, 28.050314, 1.203843},
    {1718.003418, 602.554321, 28.050314, 0.838634},
    {1717.534546, 604.116455, 28.050314, 0.084646},
    {1717.50647, 605.871887, 28.050314, 5.955501}
};

static Locations MovePositionLeft[]=
{
    {1679.601807, 651.970642, 28.050264, 3.645072},
    {1679.978394, 650.11438, 28.050264, 3.185613},
    {1678.957642, 648.234253, 28.050264, 2.235281},
    {1677.399048, 647.723083, 28.05026, 3.36369 },
    {1675.833374, 648.145142, 28.050264, 1.155358},
    {1674.618896, 649.505127, 28.050264, 1.769331},
    {1674.443481, 651.406128, 28.050264, 6.079807},
    {1675.299072, 652.49884, 28.050264, 0.838634},
    {1676.444824, 653.164429, 28.050264, 0.084646},
    {1678.224365, 653.184753, 28.050264, 5.955501}
};

static Speech Sacrifice[] =
{
    {SAY_KALECGOS_AWAKEN,       CREATURE_KALECGOS,  5000},
    {SAY_ANVEENA_IMPRISONED,    CREATURE_ANVEENA,   4000},
    {SAY_KALECGOS_LETGO,        CREATURE_KALECGOS,  7000},
    {SAY_ANVEENA_LOST,          CREATURE_ANVEENA,   5000},
    {SAY_KALECGOS_FOCUS,        CREATURE_KALECGOS,  7000},
    {SAY_ANVEENA_KALEC,         CREATURE_ANVEENA,   2000},
    {SAY_KALECGOS_FATE,         CREATURE_KALECGOS,  3000},
    {SAY_ANVEENA_GOODBYE,       CREATURE_ANVEENA,   6000},
    {SAY_KALECGOS_GOODBYE,      CREATURE_KALECGOS,  12000},
    {SAY_KJ_PHASE5,             CREATURE_KILJAEDEN, 8000},
    {SAY_KALECGOS_ENCOURAGE,    CREATURE_KALECGOS,  0}
};

#endif