// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Boss_Victor_Nefarius
SD%Complete: 90
SDComment: Fix Shadow Command. Out of BWL missing part
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

enum VictorNefarius
{
    SAY_GAMESBEGIN_1                = -1469004,
    SAY_GAMESBEGIN_2                = -1469005,
    // SAY_VAEL_INTRO                  = -1469006,
    
    MAX_DRAKES                      = 5,
    MAX_DRAKE_SUMMONS               = 42,
    CREATURE_BRONZE_DRAKANOID       = 14263,
    CREATURE_BLUE_DRAKANOID         = 14261,
    CREATURE_RED_DRAKANOID          = 14264,
    CREATURE_GREEN_DRAKANOID        = 14262,
    CREATURE_BLACK_DRAKANOID        = 14265,
    CREATURE_CHROMATIC_DRAKANOID    = 14302,
    CREATURE_NEFARIAN               = 11583,
    CREATURE_BONE_CONSTRUCT         = 14605,

    SPELL_NEFARIUS_BARRIER          = 22663,
    SPELL_SHADOWBLINK_INTRO         = 22664,
    SPELL_SHADOWBOLT_VOLLEY         = 22665,
    SPELL_SILENCE                   = 22666,
    SPELL_SHADOW_COMMAND            = 22667,
    SPELL_SHADOWBOLT                = 22677,
    SPELL_FEAR                      = 22678,
    SPELL_SHADOWBLINK               = 22681, // triggers a random from spells (22668 - 22676)

    MAP_ID_BWL                      = 469,
    FACTION_BLACK_DRAGON            = 103,
    FACTION_NEUTRAL                 = 35
};

static const uint32 aPossibleDrake[MAX_DRAKES] = {CREATURE_BRONZE_DRAKANOID, CREATURE_BLUE_DRAKANOID, CREATURE_RED_DRAKANOID, CREATURE_GREEN_DRAKANOID, CREATURE_BLACK_DRAKANOID};

struct SpawnLocation
{
    float m_fX, m_fY, m_fZ;
};

static const SpawnLocation aNefarianLocs[3] =
{
    { -7599.32f, -1191.72f, 475.545f},                      // opening where red/blue/black darknid spawner appear (ori 3.05433)
    { -7526.27f, -1135.04f, 473.445f},                      // same as above, closest to door (ori 5.75959)
    { -7498.177f, -1273.277f, 481.649f}                    // nefarian spawn location (ori 1.798)
};

#define GOSSIP_ITEM_1           16115
#define GOSSIP_ITEM_2           16116
#define GOSSIP_ITEM_3           16117

struct boss_victor_nefariusAI : public ScriptedAI
{
    boss_victor_nefariusAI(Creature *c) : ScriptedAI(c), Summons(me)
    {
        NefarianGUID = 0;
        srand(time(NULL));
        uint8 Pos1 = urand(0, MAX_DRAKES - 1);
        uint8 Pos2 = (Pos1 + urand(1, MAX_DRAKES - 1)) % MAX_DRAKES;
        DrakeTypeOne = aPossibleDrake[Pos1];
        DrakeTypeTwo = aPossibleDrake[Pos2];
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    SummonList Summons;

    uint32 SpawnedAdds;
    Timer AddSpawnTimer;
    Timer _ChangedNameShadowBoltTimer;
    Timer _ChangedNameFearTimer;
    Timer _ChangedNameShadowboltVolleyTimer;
    Timer _ChangedNameSilenceTimer;
    Timer _ChangedNameShadowBlinkTimer;
    uint32 ResetTimer;
    uint32 DrakeTypeOne;
    uint32 DrakeTypeTwo;

    uint64 NefarianGUID;
    Timer NefCheckTime;

    void Reset()
    {
        if (me->GetMapId() != MAP_ID_BWL)
            return;

        SpawnedAdds = 0;
        AddSpawnTimer.Reset(10000);
        _ChangedNameShadowBoltTimer.Reset(3000);
        _ChangedNameFearTimer.Reset(8000);
        _ChangedNameShadowboltVolleyTimer.Reset(13000);
        _ChangedNameSilenceTimer.Reset(23000);
        _ChangedNameShadowBlinkTimer.Reset(40000);
        ResetTimer = HeroicMode ? 5000 : 900000;
        NefarianGUID = 0;
        NefCheckTime = 2000;

        me->SetUInt32Value(UNIT_NPC_FLAGS,1); // Set gossip
        me->setFaction(FACTION_NEUTRAL);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetStandState(UNIT_STAND_STATE_SIT_HIGH_CHAIR);
        me->RemoveAurasDueToSpell(SPELL_NEFARIUS_BARRIER);

        Summons.DespawnAll();
    }

    void BeginEvent(Player* target)
    {
        if (me->GetMapId() != MAP_ID_BWL)
            return;

        DoZoneInCombat();
        DoScriptText(SAY_GAMESBEGIN_2, me);
        me->SetUInt32Value(UNIT_NPC_FLAGS, 0);
        me->setFaction(FACTION_BLACK_DRAGON);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetStandState(UNIT_STAND_STATE_STAND);
        DoCast(me, SPELL_NEFARIUS_BARRIER);
        AttackStart(target);
    }

    void EnterEvadeMode()
    {
        if (me->GetMapId() != MAP_ID_BWL)
            return;

        if(pInstance)
            pInstance->SetData(DATA_NEFARIAN_EVENT, NOT_STARTED);
        ScriptedAI::EnterEvadeMode();
    }

    void EnterCombat(Unit *who)
    {
        if (me->GetMapId() != MAP_ID_BWL)
            return;

        if (pInstance)
            pInstance->SetData(DATA_NEFARIAN_EVENT, IN_PROGRESS);

        me->AddThreat(who, 0.0f);
        me->GetMotionMaster()->MoveChase(who, 15);
    }
    
    void JustReachedHome()
    {
        if (me->GetMapId() != MAP_ID_BWL)
            return;

        if (pInstance)
            pInstance->SetData(DATA_NEFARIAN_EVENT, NOT_STARTED);
        Reset();
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
    {
        if (me->GetMapId() != MAP_ID_BWL)
            return;

        if (summon->GetEntry() != CREATURE_NEFARIAN)
        {
            summon->Respawn();
            summon->UpdateEntry(CREATURE_BONE_CONSTRUCT);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summon->SetReactState(REACT_PASSIVE);
            summon->SetStandState(UNIT_STAND_STATE_DEAD);
        }
    }

    void JustSummoned(Creature* summoned)
    {
        Unit* target = NULL;
        target = SelectUnit(SELECT_TARGET_RANDOM,0);
        if (target && summoned)
        {
            summoned->AI()->AttackStart(target);
            if (me->GetMapId() == MAP_ID_BWL)
                summoned->setFaction(FACTION_BLACK_DRAGON);
        }
        if (me->GetMapId() == MAP_ID_BWL)
        {
            if(summoned->GetEntry() != CREATURE_NEFARIAN)
                ++SpawnedAdds;
            Summons.Summon(summoned);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetMapId() != MAP_ID_BWL)
            return;

        if (!UpdateVictim())
        {
            if (ResetTimer < diff)
            {
                me->SetVisibility(VISIBILITY_ON);
                ResetTimer = HeroicMode ? 5000 : 900000;
            }
            else 
                ResetTimer -= diff; // count only OUT OF COMBAT
            return;
        }

        if (SpawnedAdds < MAX_DRAKE_SUMMONS) // Only do this if we haven't spawned Nefarian yet
        {
            // ShadowBoltTimer
            if (_ChangedNameShadowBoltTimer.Expired(diff))
            {
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                if (target)
                    DoCast(target, SPELL_SHADOWBOLT);
                _ChangedNameShadowBoltTimer = urand(2000, 4000);
            }

            // FearTimer
            if (_ChangedNameFearTimer.Expired(diff))
            {
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                if (target)
                    DoCast(target, SPELL_FEAR);

                _ChangedNameFearTimer = urand(10000, 20000);
            }

            // Shadowbolt Volley
            if (_ChangedNameShadowboltVolleyTimer.Expired(diff))
            {
                DoCast(me, SPELL_SHADOWBOLT_VOLLEY);
                _ChangedNameShadowboltVolleyTimer = urand(19000, 28000);
            }

            // Silence
            if (_ChangedNameSilenceTimer.Expired(diff))
            {
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                if (target)
                    DoCast(target, SPELL_SILENCE);
                _ChangedNameSilenceTimer = urand(14000, 23000);
            }

            // ShadowBlink
            if (_ChangedNameShadowBlinkTimer.Expired(diff))
            {
                DoCast(me, SPELL_SHADOWBLINK);
                _ChangedNameShadowBlinkTimer = urand(30000, 40000);
            }

            if (AddSpawnTimer.Expired(diff)) // Add spawning mechanism
            {
                // Spawn 2 random types of creatures at the 2 locations
                uint32 CreatureID;
                // 1 in 3 chance it will be a chromatic
                CreatureID = urand(0, 2) ? DrakeTypeOne : uint32(CREATURE_CHROMATIC_DRAKANOID);
                // Spawn creature and force it to start attacking a random target
                me->SummonCreature(CreatureID, aNefarianLocs[0].m_fX, aNefarianLocs[0].m_fY, aNefarianLocs[0].m_fZ, 5, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1800000);

                // 1 in 3 chance it will be a chromatic
                CreatureID = urand(0, 2) ? DrakeTypeTwo : uint32(CREATURE_CHROMATIC_DRAKANOID);
                me->SummonCreature(CreatureID, aNefarianLocs[1].m_fX, aNefarianLocs[1].m_fY, aNefarianLocs[1].m_fZ, 5, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1800000);

                if (SpawnedAdds >= 42) // Begin Phase 2
                {
                    me->InterruptNonMeleeSpells(false);
                    me->SetVisibility(VISIBILITY_OFF);
                    // Spawn Nefarian and have him attack a random target
                    Creature* Nefarian = NULL;
                    Nefarian = me->SummonCreature(CREATURE_NEFARIAN, aNefarianLocs[2].m_fX, aNefarianLocs[2].m_fY, aNefarianLocs[2].m_fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0, false);
                    if (Nefarian)
                        NefarianGUID = Nefarian->GetGUID();
                    else error_log("TSCR: Blackwing Lair: Unable to spawn nefarian properly.");
                }
                AddSpawnTimer = 4000;
            }
        }
        else if (NefarianGUID)
        {
            if (NefCheckTime.Expired(diff))
            {
                Unit* Nefarian = NULL;
                Nefarian = Unit::GetUnit((*me),NefarianGUID);

                // If nef is dead then we die to so the players get out of combat
                // and cannot repeat the event
                if (Nefarian && !Nefarian->isAlive())
                {
                    NefarianGUID = 0;
                    me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                }

                NefCheckTime = 2000;
            }
        }
    }
};

CreatureAI* GetAI_boss_victor_nefarius(Creature *_Creature)
{
    return new boss_victor_nefariusAI (_Creature);
}

bool GossipHello_boss_victor_nefarius(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(7134,_Creature->GetGUID());
    return true;
}

bool GossipSelect_boss_victor_nefarius(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(7198, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(7199, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_GAMESBEGIN_1, _Creature);
            ((boss_victor_nefariusAI*)_Creature->AI())->BeginEvent(player);
            break;
    }
    return true;
}

void AddSC_boss_victor_nefarius()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_victor_nefarius";
    newscript->GetAI = &GetAI_boss_victor_nefarius;
    newscript->pGossipHello = &GossipHello_boss_victor_nefarius;
    newscript->pGossipSelect = &GossipSelect_boss_victor_nefarius;
    newscript->RegisterSelf();
}


