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
SDName: Darkshore
SD%Complete: 0
SDComment: Placeholder Quest support 731, 945
SDCategory: Darkshore
EndScriptData */

/* ContentData
npc_prospector_remtravel
npc_therylune
npc_grimclaw
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"

/*####
# npc_kerlonian
####*/

enum
{
    SAY_KER_START               = -1000434,

    EMOTE_KER_SLEEP_1           = -1000435,
    EMOTE_KER_SLEEP_2           = -1000436,
    EMOTE_KER_SLEEP_3           = -1000437,

    SAY_KER_SLEEP_1             = -1000438,
    SAY_KER_SLEEP_2             = -1000439,
    SAY_KER_SLEEP_3             = -1000440,
    SAY_KER_SLEEP_4             = -1000441,

    EMOTE_KER_AWAKEN            = -1000445,

    SAY_KER_ALERT_1             = -1000442,
    SAY_KER_ALERT_2             = -1000443,

    SAY_KER_END                 = -1000444,

    SPELL_BEAR_FORM             = 18309,
    SPELL_SLEEP_VISUAL          = 25148,
    SPELL_AWAKEN                = 17536,
    QUEST_SLEEPER_AWAKENED      = 5321,
    NPC_LILADRIS                = 11219                     //attackers entries unknown
};

struct npc_kerlonianAI : public FollowerAI
{
    npc_kerlonianAI(Creature* pCreature) : FollowerAI(pCreature)
    {
        Reset();
    }

    Timer FallAsleepTimer;
    uint8 Phase;
    bool Phase1Done;
    bool Phase2Done;

    void Reset()
    {
        FallAsleepTimer.Reset(urand(10000, 45000));
        if (!HasFollowState(STATE_FOLLOW_INPROGRESS))
        {
            me->CastSpell(me, SPELL_BEAR_FORM, false);
            Phase = 0;
            Phase1Done = false;
            Phase2Done = false;
            me->addUnitState(UNIT_STAT_LOST_CONTROL);
        }
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!me->GetVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && pWho->GetEntry() == NPC_LILADRIS)
        {
            if (me->IsWithinDistInMap(pWho, INTERACTION_DISTANCE * 5))
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_SLEEPER_AWAKENED) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->GroupEventHappens(QUEST_SLEEPER_AWAKENED, me);

                    DoScriptText(SAY_KER_END, me);
                }

                SetFollowComplete();
            }
        }

        if(!me->GetVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE) && pWho->GetEntry() == 14495)
        {
            if (me->IsWithinDistInMap(pWho, INTERACTION_DISTANCE * 5))
            {
                switch(Phase)
                {
                    case 0:
                    {
                        if(!Phase1Done)
                        {
                            Phase1Done = true;
                            Phase++;
                            if(Creature* summon1 = me->SummonCreature(2169, 4480.79, 248.848, 60.5918, 1.06, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
                                summon1->AI()->AttackStart(me);
                            if(Creature* summon2 = me->SummonCreature(2169, 4470.68, 260.508, 59.0824, 0.59, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
                                summon2->AI()->AttackStart(me);
                            if(Creature* summon3 = me->SummonCreature(2169, 4461.42, 271.358, 61.7843, 6.22188, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
                                summon3->AI()->AttackStart(me);
                            DoScriptText(SAY_KER_ALERT_1, me);
                        }
                        break;
                    }
                    case 1:
                    {
                        if(!Phase2Done)
                        {
                            if(Creature* summon1 = me->SummonCreature(3727, 3619.87, 202.682, 2.825, 1.101, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
                                summon1->AI()->AttackStart(me);
                            if(Creature* summon2 = me->SummonCreature(3725, 3624.79, 226.158, 1.831, 4.731, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
                                summon2->AI()->AttackStart(me);
                            if(Creature* summon3 = me->SummonCreature(3728, 3603.32, 217.795, 1.201, 6.168, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000))
                                summon3->AI()->AttackStart(me);
                            Phase = 0;
                            Phase2Done = true;
                            DoScriptText(SAY_KER_ALERT_2, me);
                        }
                        break;
                    }
                }
            }
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (HasFollowState(STATE_FOLLOW_INPROGRESS | STATE_FOLLOW_PAUSED) && pSpell->Id == SPELL_AWAKEN)
            ClearSleeping();
    }

    void SetSleeping()
    {
        SetFollowPaused(true);

        switch (urand(0, 2))
        {
            case 0:
                DoScriptText(EMOTE_KER_SLEEP_1, me);
                break;
            case 1:
                DoScriptText(EMOTE_KER_SLEEP_2, me);
                break;
            case 2:
                DoScriptText(EMOTE_KER_SLEEP_3, me);
                break;
        }

        switch (urand(0, 3))
        {
            case 0:
                DoScriptText(SAY_KER_SLEEP_1, me);
                break;
            case 1:
                DoScriptText(SAY_KER_SLEEP_2, me);
                break;
            case 2:
                DoScriptText(SAY_KER_SLEEP_3, me);
                break;
            case 3:
                DoScriptText(SAY_KER_SLEEP_4, me);
                break;
        }

        me->SetStandState(UNIT_STAND_STATE_SLEEP);
        me->CastSpell(me, SPELL_SLEEP_VISUAL, false);
        me->addUnitState(UNIT_STAT_LOST_CONTROL);
    }

    void ClearSleeping()
    {
        me->RemoveAurasDueToSpell(SPELL_SLEEP_VISUAL);
        me->SetStandState(UNIT_STAND_STATE_STAND);

        DoScriptText(EMOTE_KER_AWAKEN, me);
        me->ClearUnitState(UNIT_STAT_LOST_CONTROL);

        SetFollowPaused(false);
    }

    void UpdateFollowerAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (!HasFollowState(STATE_FOLLOW_INPROGRESS))
                return;

            if (!HasFollowState(STATE_FOLLOW_PAUSED))
            {
                if (FallAsleepTimer.Expired(diff))
                {
                    SetSleeping();
                    FallAsleepTimer = urand(25000, 90000);
                }
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kerlonian(Creature* pCreature)
{
    return new npc_kerlonianAI(pCreature);
}

bool QuestAccept_npc_kerlonian(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_SLEEPER_AWAKENED)
    {
        if (npc_kerlonianAI* pKerlonianAI = dynamic_cast<npc_kerlonianAI*>(pCreature->AI()))
        {
            pPlayer->SendPlaySound(6209, true);
            pCreature->RemoveAurasDueToSpell(SPELL_BEAR_FORM);
            pCreature->SetStandState(UNIT_STAND_STATE_STAND);
            DoScriptText(SAY_KER_START, pCreature, pPlayer);
            pKerlonianAI->StartFollow(pPlayer, FACTION_ESCORT_N_FRIEND_PASSIVE, pQuest);
        }
    }

    return true;
}

/*######
## npc_prospector_remtravel
######*/

#define Q_Absent_Minded_Prospector 731
#define SAY_prospector_ACC     -1581001
#define SAY_prospector_AGGRO_1 -1581011
#define SAY_prospector_AGGRO_2 -1581012
#define SAY_prospector_AGGRO_3 -1581013
#define SAY_prospector_COMP    -1581010

struct npc_prospector_remtravel : public npc_escortAI
{
    npc_prospector_remtravel(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch(i) {
        case 10: DoScriptText(-1581002, me);break;
        case 12: DoScriptText(-1581003, me);break;
        case 14: DoScriptText(-1581004, me);break;
        case 16: DoScriptText(-1581005, me);break;
        case 17:
            DoScriptText(-1581006, me);
            me->SummonCreature(2158, 4628.38, 638.456, 6.402, 6.20, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            me->SummonCreature(2158, 4625.13, 645.962, 6.73182, 6.27, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            break;
        case 25: DoScriptText(-1581007, me);break;
        case 32: DoScriptText(-1581008, me);break;
        case 35:
            me->SummonCreature(2158, 4570.04, 557.292, 1.989, 6.20, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            me->SummonCreature(2159, 4573.17, 557.583, 3.328, 6.27, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            me->SummonCreature(2160, 4564.94, 551.357, 5.91, 6.27, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            break;
        case 42: DoScriptText(-1581009, me);break;
        case 56:
            DoScriptText(SAY_prospector_COMP, me, player);
            player->GroupEventHappens(Q_Absent_Minded_Prospector, me);
            break;
        }
    }

    void Reset(){}

    void EnterCombat(Unit* who)
    {
        DoScriptText(RAND(SAY_prospector_AGGRO_1, SAY_prospector_AGGRO_2, SAY_prospector_AGGRO_3), me);

        me->Attack(who, true);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(me);
    }
};

CreatureAI* GetAI_npc_prospector_remtravel(Creature *_Creature)
{
    npc_prospector_remtravel* prospector_remtravel = new npc_prospector_remtravel(_Creature);

    prospector_remtravel->AddWaypoint(0, 4677.35, 582.882, 21.3481);
    prospector_remtravel->AddWaypoint(1, 4673.13, 582.741, 20.7995);
    prospector_remtravel->AddWaypoint(2, 4672.99, 593.263, 17.5914);
    prospector_remtravel->AddWaypoint(3, 4668.94, 600.76, 14.6225);
    prospector_remtravel->AddWaypoint(4, 4659.59, 609.162, 9.36938);
    prospector_remtravel->AddWaypoint(5, 4651.6, 615.518, 8.56175);
    prospector_remtravel->AddWaypoint(6, 4644, 621.083, 8.57906);
    prospector_remtravel->AddWaypoint(7, 4634.77, 624.85, 7.57635);
    prospector_remtravel->AddWaypoint(8, 4633.5, 631.041, 6.61543);
    prospector_remtravel->AddWaypoint(9, 4634.23, 633.058, 7.01277);
    prospector_remtravel->AddWaypoint(10, 4640.83, 638.327, 13.4057);
    prospector_remtravel->AddWaypoint(11, 4645.78, 633.204, 13.4303);
    prospector_remtravel->AddWaypoint(12, 4639.92, 637.669, 13.3612);
    prospector_remtravel->AddWaypoint(13, 4636.84, 635.289, 10.3009);
    prospector_remtravel->AddWaypoint(14, 4630.55, 631.307, 6.32709);
    prospector_remtravel->AddWaypoint(15, 4627.73, 637.741, 6.36486);
    prospector_remtravel->AddWaypoint(16, 4622.89, 637.517, 6.31533);
    prospector_remtravel->AddWaypoint(17, 4625.13, 645.962, 6.73182);
    prospector_remtravel->AddWaypoint(18, 4628.38, 638.456, 6.402);
    prospector_remtravel->AddWaypoint(19, 4615.94, 640.499, 6.67037);
    prospector_remtravel->AddWaypoint(20, 4624.88, 635.098, 6.30605);
    prospector_remtravel->AddWaypoint(21, 4623.39, 631.595, 6.24625);
    prospector_remtravel->AddWaypoint(22, 4617.66, 631.987, 6.25943);
    prospector_remtravel->AddWaypoint(23, 4614.22, 619.818, 5.84416);
    prospector_remtravel->AddWaypoint(24, 4609.45, 613.739, 5.24473);
    prospector_remtravel->AddWaypoint(25, 4599.65, 607.006, 1.94703);
    prospector_remtravel->AddWaypoint(26, 4589.98, 599.73, 1.16939);
    prospector_remtravel->AddWaypoint(27, 4581.11, 593.133, 1.01014);
    prospector_remtravel->AddWaypoint(28, 4565.69, 582.105, 1.04814);
    prospector_remtravel->AddWaypoint(29, 4558.03, 571.718, 1.28869);
    prospector_remtravel->AddWaypoint(30, 4553.65, 566.054, 5.31751);
    prospector_remtravel->AddWaypoint(31, 4550.6, 562.106, 7.30321);
    prospector_remtravel->AddWaypoint(32, 4544.58, 568.095, 7.27242);
    prospector_remtravel->AddWaypoint(33, 4551.83, 564.466, 7.19855);
    prospector_remtravel->AddWaypoint(34, 4557.01, 571.599, 1.27336);
    prospector_remtravel->AddWaypoint(35, 4565.46, 557.54, 3.03739);
    prospector_remtravel->AddWaypoint(36, 4564.94, 551.357, 5.91);
    prospector_remtravel->AddWaypoint(37, 4573.17, 557.583, 3.328);
    prospector_remtravel->AddWaypoint(39, 4570.04, 557.292, 1.989);
    prospector_remtravel->AddWaypoint(40, 4578.1, 565.285, 1.02112);
    prospector_remtravel->AddWaypoint(41, 4589.62, 564.402, 0.923398);
    prospector_remtravel->AddWaypoint(42, 4595.54, 572.613, 1.10837);
    prospector_remtravel->AddWaypoint(43, 4600.8, 572.295, 1.23612);
    prospector_remtravel->AddWaypoint(45, 4607.06, 566.704, 1.26906);
    prospector_remtravel->AddWaypoint(46, 4590.98, 571.303, 1.13302);
    prospector_remtravel->AddWaypoint(47, 4574.5, 571.384, 1.10307);
    prospector_remtravel->AddWaypoint(48, 4575.81, 581.597, 0.979237);
    prospector_remtravel->AddWaypoint(49, 4600.36, 604.279, 2.03134);
    prospector_remtravel->AddWaypoint(50, 4614.11, 613.762, 5.50871);
    prospector_remtravel->AddWaypoint(51, 4629.06, 620.1, 6.59015);
    prospector_remtravel->AddWaypoint(52, 4636.06, 624.593, 7.78406);
    prospector_remtravel->AddWaypoint(53, 4640.59, 625.475, 8.21139);
    prospector_remtravel->AddWaypoint(54, 4656.98, 613.055, 8.56197);
    prospector_remtravel->AddWaypoint(55, 4675.34, 598.581, 17.3818);
    prospector_remtravel->AddWaypoint(56, 4687.96, 590.182, 23.839,5000);

    return (CreatureAI*)prospector_remtravel;
}

bool QuestAccept_npc_prospector_remtravel(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == Q_Absent_Minded_Prospector)
    {
        creature->setFaction(113);
        creature->SetHealth(creature->GetMaxHealth());
        creature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        creature->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
        DoScriptText(SAY_prospector_ACC, creature, player);
        ((npc_escortAI*)creature->AI())->Start(true, false, player->GetGUID(), quest);
    }
    return true;
}

/*######
## npc_therylune
######*/

#define Q_Therylune_Escape 945
#define SAY_therylune_ACC     -1581014
#define SAY_therylune_COMP    -1581015

struct npc_therylune : public npc_escortAI
{
    npc_therylune(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch(i) {
        case 20:
            DoScriptText(SAY_therylune_COMP, me, player);
            player->GroupEventHappens(Q_Therylune_Escape, me);
            break;
        }
    }

    void Reset(){}

    void EnterCombat(Unit* who)
    {
        me->Attack(who, true);
    }

    void JustDied(Unit* killer)
    {
        if (Player* player = GetPlayerForEscort())
            player->FailQuest(Q_Therylune_Escape);
    }


    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        if (!UpdateVictim())
            return;
    }
};

bool QuestAccept_npc_therylune(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == Q_Therylune_Escape)
    {
        creature->setFaction(113);
        creature->SetHealth(creature->GetMaxHealth());
        creature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        creature->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
        DoScriptText(SAY_therylune_ACC, creature, player);
        ((npc_escortAI*)creature->AI())->Start(true, true, player->GetGUID(), quest);

    }
    return true;
}

CreatureAI* GetAI_npc_therylune(Creature *_Creature)
{
    npc_therylune* therylune = new npc_therylune(_Creature);

    therylune->AddWaypoint(0, 4520.4, 420.235, 33.5284);
    therylune->AddWaypoint(1,  4512.26, 408.881, 32.9308);
    therylune->AddWaypoint(2,  4507.94, 396.47, 32.9476);
    therylune->AddWaypoint(3,  4507.53, 383.781, 32.995);
    therylune->AddWaypoint(4,  4512.1, 374.02, 33.166);
    therylune->AddWaypoint(5,  4519.75, 373.241, 33.1574);
    therylune->AddWaypoint(6, 4592.41, 369.127, 31.4893);
    therylune->AddWaypoint(7,  4598.55, 364.801, 31.4947);
    therylune->AddWaypoint(8,  4602.76, 357.649, 32.9265);
    therylune->AddWaypoint(9,  4597.88, 352.629, 34.0317);
    therylune->AddWaypoint(10,  4590.23, 350.9, 36.2977);
    therylune->AddWaypoint(11,  4581.5, 348.254, 38.3878);
    therylune->AddWaypoint(12,  4572.05, 348.059, 42.3539);
    therylune->AddWaypoint(13,  4564.75, 344.041, 44.2463);
    therylune->AddWaypoint(14,  4556.63, 341.003, 47.6755);
    therylune->AddWaypoint(15,  4554.38, 334.968, 48.8003);
    therylune->AddWaypoint(16,  4557.63, 329.783, 49.9532);
    therylune->AddWaypoint(17,  4563.32, 316.829, 53.2409);
    therylune->AddWaypoint(18,  4566.09, 303.127, 55.0396);
    therylune->AddWaypoint(19, 4561.65, 295.456, 57.0984);
    therylune->AddWaypoint(20,  4551.03, 293.333, 57.1534);
    return (CreatureAI*)therylune;
}

/*####
# npc_sentinel_aynasha
####*/
 
 
enum OneShotOneKill
{
    QUEST_ONESHOT_ONEKILL       = 5713,

    SAY_START                   = -1789654,
    SAY_OUT_OF_ARROWS           = -1789655,
    SAY_END                     = -1789656,
    SAY_END2                    = -1789657,

    SPELL_SHOOT                 = 19767,

    NPC_BLACKWOOD_TRACKER       = 11713,
    NPC_BOSS_MAROSH             = 11714
};

struct npc_sentinel_aynashaAI : public npc_escortAI
{
    npc_sentinel_aynashaAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        ArrowSaid = 0;
        WaitController = 0;
    }

    uint32 WaitController;
    uint32 WaitTimer;

    uint32 ShootTimer;

    uint32 ArrowTimer;
    bool ArrowSaid;

    uint64 PlayerGUID;

    void Reset()
    {
        ShootTimer = 1000;
    }

    void WaypointReached(uint32 PointId) {}

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(me);
    }

    void UpdateAI(const uint32 diff)
    {
        if(WaitController)
        {
            if(WaitTimer <= diff)
            {
                switch(WaitController)
                {
                    case 1:
                        me->SummonCreature(NPC_BLACKWOOD_TRACKER, 4371.19, -33.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        me->SummonCreature(NPC_BLACKWOOD_TRACKER, 4376.19, -33.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        me->SummonCreature(NPC_BLACKWOOD_TRACKER, 4371.19, -38.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        ++WaitController;
                        WaitTimer = 45000;
                        break;

                    case 2:
                        me->SummonCreature(NPC_BLACKWOOD_TRACKER, 4371.19, -33.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        me->SummonCreature(NPC_BLACKWOOD_TRACKER, 4376.19, -33.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        me->SummonCreature(NPC_BLACKWOOD_TRACKER, 4371.19, -38.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        me->SummonCreature(NPC_BLACKWOOD_TRACKER, 4376.19, -38.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        ++WaitController;
                        WaitTimer = 45000;
                        break;
                    case 3:
                        me->SummonCreature(NPC_BOSS_MAROSH, 4376.19, -38.71, 73.53, 5.44982, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                        ++WaitController;
                        WaitTimer = 20000;
                        break;
                    case 4:
                        if (!UpdateVictim())            //wait untill combat ends
                        {
                            DoScriptText(SAY_END, me);
                            WaitTimer = 10000;
                            ++WaitController;
                        }
                        break;
                    case 5:
                        if (Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID))
                        {
                            pPlayer->GroupEventHappens(QUEST_ONESHOT_ONEKILL,me);
                        }
                        DoScriptText(SAY_END2, me);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE); //not really neccesary, only removing what i addedfor it to be as it in DB is
                        WaitController = 0;
                        ArrowSaid = 0;
                        break;
                    default:
                        break;
                }
            }
            else
                WaitTimer -= diff;
        }

        if(!UpdateVictim())
            return;

        if(ArrowTimer <= diff && !ArrowSaid && WaitController)
        {
            DoScriptText(SAY_OUT_OF_ARROWS , me);
            ArrowSaid = 1;
        } else ArrowTimer -= diff;

        if(ShootTimer <= diff && !ArrowSaid)
        {
            DoCast(me->GetVictim(), SPELL_SHOOT);
            ShootTimer = 1000;
        } else ShootTimer -= diff;

    }

    void StartEvent()
    {
        WaitController = 1;
        WaitTimer = 5000;
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        DoScriptText(SAY_START, me);
        ArrowTimer = 60000;
    }
};

CreatureAI* GetAI_npc_sentinel_aynasha(Creature* pCreature)
{
    return new npc_sentinel_aynashaAI(pCreature);
}

bool QuestAccept_npc_sentinel_aynasha(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ONESHOT_ONEKILL)
    {
        CAST_AI(npc_sentinel_aynashaAI, pCreature->AI())->StartEvent();
        CAST_AI(npc_sentinel_aynashaAI, pCreature->AI())->PlayerGUID = (pPlayer->GetGUID());

        if (npc_escortAI* pEscortAI = CAST_AI(npc_sentinel_aynashaAI, pCreature->AI()))
            pEscortAI->Start(true, true, pPlayer->GetGUID());
    }
    return true;
}

/*######
## npc_rabid_bear
######*/

enum
{
    QUEST_PLAGUED_LANDS             = 2118,

    NPC_CAPTURED_RABID_THISTLE_BEAR = 11836,
    NPC_THARNARIUN_TREETENDER       = 3701,                 // Npc related to quest-outro
    GO_NIGHT_ELVEN_BEAR_TRAP        = 111148,               // This is actually the (visual) spell-focus GO

    SPELL_RABIES                    = 3150,                 // Spell used in comabt
};

struct npc_rabid_bearAI : public ScriptedAI
{
    npc_rabid_bearAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
        JustRespawned();
    }

    uint32 CheckTimer;
    uint32 RabiesTimer;
    uint32 DespawnTimer;

    void Reset()
    {
        RabiesTimer = urand(12000, 18000);
    }

    void JustRespawned()
    {
        CheckTimer = 1000;
        DespawnTimer = 0;
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!CheckTimer && pWho->GetTypeId() == TYPEID_UNIT && pWho->GetEntry() == NPC_THARNARIUN_TREETENDER &&
              pWho->IsWithinDist(me, 2 * INTERACTION_DISTANCE, false))
        {
            // Possible related spell: 9455 9372
            me->ForcedDespawn(1000);
            me->GetMotionMaster()->MoveIdle();

            return;
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (CheckTimer && me->IsInCombat())
        {
            if (CheckTimer <= uiDiff)
            {
                // Trap nearby?
                if (GameObject* pTrap = GetClosestGameObjectWithEntry(me, GO_NIGHT_ELVEN_BEAR_TRAP, 0.5f))
                {
                    // Despawn trap
                    pTrap->Use(me);
                    // "Evade"
                    me->RemoveAllAuras();
                    me->DeleteThreatList();
                    me->CombatStop(true);
                    me->SetLootRecipient(NULL);
                    Reset();
                    // Update Entry and start following player
                    me->UpdateEntry(NPC_CAPTURED_RABID_THISTLE_BEAR);
                    // get player
                    Unit* pTrapOwner = pTrap->GetOwner();
                    if (pTrapOwner && pTrapOwner->GetTypeId() == TYPEID_PLAYER &&
                            ((Player*)pTrapOwner)->GetQuestStatus(QUEST_PLAGUED_LANDS) == QUEST_STATUS_INCOMPLETE)
                    {
                        ((Player*)pTrapOwner)->KilledMonster(me->GetEntry(), me->GetObjectGuid());
                        me->GetMotionMaster()->MoveFollow(pTrapOwner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
                    }
                    else                                // Something unexpected happened
                        me->ForcedDespawn(1000);

                    // No need to check any more
                    CheckTimer = 0;
                    // Despawn after a while (delay guesswork)
                    DespawnTimer = 3 * MINUTE * MILLISECONDS;

                    return;
                }
                else
                    CheckTimer = 1000;
            }
            else
                CheckTimer -= uiDiff;
        }

        if (DespawnTimer && !me->IsInCombat())
        {
            if (DespawnTimer <= uiDiff)
            {
                me->ForcedDespawn();
                return;
            }
            else
                DespawnTimer -= uiDiff;
        }

        if (!UpdateVictim())
            return;

        if (RabiesTimer < uiDiff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, SPELL_RABIES, true))
                DoCast(pTarget, SPELL_RABIES);
            RabiesTimer = urand(12000, 18000);
        }
        else
            RabiesTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_rabid_bear(Creature* pCreature)
{
    return new npc_rabid_bearAI(pCreature);
}

/*####
# npc_threshwackonator
####*/

enum
{
    EMOTE_START             = -1000413,
    SAY_AT_CLOSE            = -1000414,
    QUEST_GYROMAST_REV      = 2078,
    NPC_GELKAK              = 6667,
    FACTION_HOSTILE         = 14,
    FACTION_NEUTRAL         = 35
};

#define GOSSIP_ITEM_INSERT_KEY  16303

struct npc_threshwackonatorAI : public ScriptedAI
{
    npc_threshwackonatorAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    bool Said;

    void Reset()
    {
        me->SetIsDistanceToHomeEvadable(false);
        Said = false;
    }

    void EnterEvadeMode()
    {
        me->setFaction(FACTION_NEUTRAL);
        me->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!m_creature->GetVictim() && pWho->GetEntry() == NPC_GELKAK && !Said)
        {
            if (m_creature->IsWithinDistInMap(pWho, 10.0f))
            {
                DoScriptText(SAY_AT_CLOSE, pWho);
                DoAtEnd();
                Said = true;
            }
        }
    }

    void DoAtEnd()
    {
        m_creature->setFaction(FACTION_HOSTILE);
        if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0))
            me->AI()->AttackStart(target);
    }

};

CreatureAI* GetAI_npc_threshwackonator(Creature* pCreature)
{
    return new npc_threshwackonatorAI(pCreature);
}

bool GossipHello_npc_threshwackonator(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_GYROMAST_REV) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_INSERT_KEY), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_threshwackonator(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        DoScriptText(EMOTE_START, pCreature);
        pCreature->GetMotionMaster()->MoveFollow(pPlayer, PET_FOLLOW_DIST, PET_FOLLOW_DIST);
        pCreature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    }

    return true;
}

/*######
## npc_volcor
######*/
enum
{
    QUEST_ESCAPE_THROUGH_FORCE      = 994,
    QUEST_ESCAPE_THROUGH_STEALTH    = 995,
    VOLCOR_SAY_START                = -1000007,
    VOLCOR_SAY_END                  = -1000008,
    VOLCOR_SAY_FIRST_AMBUSH         = -1000009,
    VOLCOR_SAY_AGGRO_1              = -1000010,
    VOLCOR_SAY_AGGRO_2              = -1000011,
    VOLCOR_SAY_AGGRO_3              = -1000012,
    VOLCOR_SAY_ESCAPE               = -1000013,

    NPC_BLACKWOOD_SHAMAN            = 2171,
    NPC_BLACKWOOD_URSA              = 2170,

    SPELL_MOONSTALKER_FORM          = 10849,
};

struct SummonLocation
{
    float m_fX, m_fY, m_fZ, m_fO;
};

// Spawn locations
static const SummonLocation VolcorSpawnLocs[] =
{
    {4630.2f, 22.6f, 70.1f, 2.4f},
    {4603.8f, 53.5f, 70.4f, 5.4f},
    {4627.5f, 100.4f, 62.7f, 5.8f},
    {4692.8f, 75.8f, 56.7f, 3.1f},
    {4747.8f, 152.8f, 54.6f, 2.4f},
    {4711.7f, 109.1f, 53.5f, 2.4f},
};

struct npc_volcor : public npc_escortAI
{
    npc_volcor(Creature *c) : npc_escortAI(c) {}

    uint32 QuestId;

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            QuestId = 0;
    }

    void EnterCombat(Unit* who)
    {
        // shouldn't always use text on agro
        switch (urand(0, 4))
        {
            case 0: DoScriptText(VOLCOR_SAY_AGGRO_1, me); break;
            case 1: DoScriptText(VOLCOR_SAY_AGGRO_2, me); break;
            case 2: DoScriptText(VOLCOR_SAY_AGGRO_3, me); break;
        }
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        // No combat for this quest
        if (QuestId == QUEST_ESCAPE_THROUGH_STEALTH)
            return;

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(me);
    }

    // Wrapper to handle start function for both quests
    void StartEscort(Player* pPlayer, const Quest* pQuest)
    {
        me->SetStandState(UNIT_STAND_STATE_STAND);
        me->SetFacingToObject(pPlayer);
        QuestId = pQuest->GetQuestId();

        if (pQuest->GetQuestId() == QUEST_ESCAPE_THROUGH_STEALTH)
        {
            DoScriptText(VOLCOR_SAY_ESCAPE, me, pPlayer);
            ((npc_escortAI*)(me->AI()))->SetClearWaypoints(true);
            AddWaypoint(0, 4607.23,-5.78216,69.5633);
            AddWaypoint(1, 4622.35,29.6909,69.693);
            AddWaypoint(2, 4639.32,32.764,68.597);
            ((npc_escortAI*)(me->AI()))->SetDespawnAtEnd(false);
            Start(false, true, pPlayer->GetGUID(), pQuest);
            me->setFaction(35);
        }
        else if(pQuest->GetQuestId() == QUEST_ESCAPE_THROUGH_FORCE)
        {
            ((npc_escortAI*)(me->AI()))->SetClearWaypoints(true);
            AddWaypoint(0, 4606.61,2.96905,69.909);
            AddWaypoint(1, 4615.56,19.7957,70.7951);
            AddWaypoint(2, 4626.26,38.1105,69.0281);
            AddWaypoint(3, 4633.11,48.379,67.5631);
            AddWaypoint(4, 4637.2,71.7156,63.3412);
            AddWaypoint(5, 4645.39,88.5048,60.3851);
            AddWaypoint(6, 4660.2,104.218,58.4603);
            AddWaypoint(7, 4675.33,115.93,56.8969);
            AddWaypoint(8, 4688.21,131.397,55.5033);
            AddWaypoint(9, 4701.45,145.562,53.2203);
            AddWaypoint(10, 4709.21,155.279,52.0846);
            AddWaypoint(11, 4716.9,169.528,53.5005);
            AddWaypoint(12, 4725.44,180.07,54.7346);
            AddWaypoint(13, 4734.01,194.431,55.3888);
            AddWaypoint(14, 4747.92,209.436,53.1076);
            ((npc_escortAI*)(me->AI()))->SetDespawnAtEnd(false);
            Start(true, true, pPlayer->GetGUID(), pQuest);
            DoScriptText(VOLCOR_SAY_START, me);
        }
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch(i)
        {
            case 4:
                if(QuestId == QUEST_ESCAPE_THROUGH_FORCE)
                {
                    DoScriptText(VOLCOR_SAY_FIRST_AMBUSH, me);
                    me->SummonCreature(NPC_BLACKWOOD_SHAMAN, VolcorSpawnLocs[0].m_fX, VolcorSpawnLocs[0].m_fY, VolcorSpawnLocs[0].m_fZ, VolcorSpawnLocs[0].m_fO, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                    me->SummonCreature(NPC_BLACKWOOD_URSA, VolcorSpawnLocs[1].m_fX, VolcorSpawnLocs[1].m_fY, VolcorSpawnLocs[1].m_fZ, VolcorSpawnLocs[1].m_fO, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                }
                break;
            case 10:
                if(QuestId == QUEST_ESCAPE_THROUGH_FORCE)
                {
                    me->SummonCreature(NPC_BLACKWOOD_SHAMAN, VolcorSpawnLocs[2].m_fX, VolcorSpawnLocs[2].m_fY, VolcorSpawnLocs[2].m_fZ, VolcorSpawnLocs[2].m_fO, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                    me->SummonCreature(NPC_BLACKWOOD_URSA, VolcorSpawnLocs[3].m_fX, VolcorSpawnLocs[3].m_fY, VolcorSpawnLocs[3].m_fZ, VolcorSpawnLocs[3].m_fO, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                }
                break;
            case 12:
                if(QuestId == QUEST_ESCAPE_THROUGH_FORCE)
                {
                    me->SummonCreature(NPC_BLACKWOOD_URSA, VolcorSpawnLocs[4].m_fX, VolcorSpawnLocs[4].m_fY, VolcorSpawnLocs[4].m_fZ, VolcorSpawnLocs[4].m_fO, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                    me->SummonCreature(NPC_BLACKWOOD_URSA, VolcorSpawnLocs[5].m_fX, VolcorSpawnLocs[5].m_fY, VolcorSpawnLocs[5].m_fZ, VolcorSpawnLocs[5].m_fO, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                }
                break;
            case 14:
                if(QuestId == QUEST_ESCAPE_THROUGH_FORCE)
                {
                    DoScriptText(VOLCOR_SAY_END, me);
                    if (Player* pPlayer = GetPlayerForEscort())
                        pPlayer->GroupEventHappens(QUEST_ESCAPE_THROUGH_FORCE, me);
                    SetEscortPaused(true);
                    me->ForcedDespawn(10000);
                }
                break;
            // Quest 995 waypoints
            case 0:
                if(QuestId == QUEST_ESCAPE_THROUGH_STEALTH)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                    DoCast(me, SPELL_MOONSTALKER_FORM, false);
                }
                break;
            case 2:
                if(QuestId == QUEST_ESCAPE_THROUGH_STEALTH)
                {
                    if (Player* pPlayer = GetPlayerForEscort())
                        pPlayer->GroupEventHappens(QUEST_ESCAPE_THROUGH_STEALTH, me);
                    me->ForcedDespawn(1000);
                }
                break;
            default: break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        if (!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_volcor(Creature *_Creature)
{
    npc_volcor* npcvolcor = new npc_volcor(_Creature);
    return (CreatureAI*)npcvolcor;
}

bool QuestAccept_npc_volcor(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ESCAPE_THROUGH_FORCE || quest->GetQuestId() == QUEST_ESCAPE_THROUGH_STEALTH)
    {
        if (npc_volcor* pEscortAI = dynamic_cast<npc_volcor*>(creature->AI()))
            pEscortAI->StartEscort(player, quest);
    }
    return true;
}

/*#####
# npc_grimclaw
######*/
#define QUEST_A_LOST_MASTER 993

struct npc_grimclawAI : public ScriptedAI
{
    npc_grimclawAI(Creature* creature) : ScriptedAI(creature) {}
    
    Timer TalkEmoteTimer;
    uint32 emoteTarget;
    bool emoteEvent;
    int emoteCount;

    void Reset()
    {
        TalkEmoteTimer.Reset(1000);
        emoteEvent = false;
        emoteCount = 1;
        emoteTarget = 0;
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if (eventType == 5) 
        {
            emoteEvent = true;
            emoteTarget = invoker->GetGUID();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (emoteEvent && TalkEmoteTimer.Expired(diff))
        {
            switch(emoteCount)
            {
                case 1:
                    me->TextEmote(-1200384, emoteTarget);
                    TalkEmoteTimer = 2000;
                    break;
                case 2:
                    me->TextEmote(-1200385, emoteTarget);
                    TalkEmoteTimer = 1000;
                    break;
                default:
                    emoteEvent = false;
                    emoteCount = 0;
                    emoteTarget = 0;
                    TalkEmoteTimer.Reset(1000);
                    break;
            }

            emoteCount++;
        }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_grimclaw(Creature* creature)
{
    return new npc_grimclawAI(creature);
}

bool ReceiveEmote_npc_grimclaw(Player *player, Creature* pCreature, uint32 emote)
{
    if (emote == TEXTEMOTE_WAVE 
        && player->GetQuestStatus(QUEST_A_LOST_MASTER) == QUEST_STATUS_COMPLETE
        && player->getQuestStatusMap()[QUEST_A_LOST_MASTER].m_rewarded == false)
        pCreature->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, player, pCreature);

    return true;
}

/*######
## AddSC
######*/

void AddSC_darkshore()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_kerlonian";
    newscript->GetAI = &GetAI_npc_kerlonian;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kerlonian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_prospector_remtravel";
    newscript->GetAI = &GetAI_npc_prospector_remtravel;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_prospector_remtravel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_therylune";
    newscript->GetAI = &GetAI_npc_therylune;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_therylune;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_sentinel_aynasha";
    newscript->GetAI = &GetAI_npc_sentinel_aynasha;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_sentinel_aynasha;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_rabid_bear";
    newscript->GetAI = &GetAI_npc_rabid_bear;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_threshwackonator";
    newscript->GetAI = &GetAI_npc_threshwackonator;
    newscript->pGossipHello = &GossipHello_npc_threshwackonator;
    newscript->pGossipSelect = &GossipSelect_npc_threshwackonator;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_volcor";
    newscript->GetAI = &GetAI_npc_volcor;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_volcor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_grimclaw";
    newscript->GetAI = &GetAI_npc_grimclaw;
    newscript->pReceiveEmote =  &ReceiveEmote_npc_grimclaw;
    newscript->RegisterSelf();
}
