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
SDName: Terokkar_Forest
SD%Complete: 98
SDComment: Quest support: 9889, 10009, 10051, 10052, 10873, 10896, 10887, 11085, 11096. Skettis->Ogri'la Flight, 9978, 10852, 10898
SDCategory: Terokkar Forest
EndScriptData */

/* ContentData
mob_unkor_the_ruthless
mob_infested_root_walker
mob_rotting_forest_rager
mob_netherweb_victim
npc_floon
npc_skyguard_handler_deesak
npc_isla_starmane
go_skull_pile
go_ancient_skull_pile
mob_terokk
npc_skyguard_ace
npc_blackwing_warp_chaser
npc_skyguard_prisoner
npc_letoll
npc_sarthis
npc_sarthis_elemental
npc_minion_of_sarthis
npc_razorthorn_ravager
quest_the_vengeful_harbringer
mob_vengeful_draenei
npc_akuno
npc_empoor
npc_captive_child
go_veil_skith_cage
npc_skywing
npc_lady_dena_kennedy
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## mob_unkor_the_ruthless
######*/

#define SAY_SUBMIT                      -1000351

#define FACTION_HOSTILE                 45
#define FACTION_FRIENDLY                35
#define QUEST_DONTKILLTHEFATONE         9889

#define SPELL_PULVERIZE                 2676
//#define SPELL_QUID9889                32174

struct mob_unkor_the_ruthlessAI : public ScriptedAI
{
    mob_unkor_the_ruthlessAI(Creature* creature) : ScriptedAI(creature) {}

    bool CanDoQuest;
    Timer_UnCheked UnkorUnfriendly_Timer;
    Timer_UnCheked Pulverize_Timer;

    void Reset()
    {
        CanDoQuest = false;
        UnkorUnfriendly_Timer = 0;
        Pulverize_Timer.Reset(3000);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
        me->setFaction(FACTION_HOSTILE);
    }

    void EnterCombat(Unit *who) {}

    void DoNice()
    {
        DoScriptText(SAY_SUBMIT, me);
        me->setFaction(FACTION_FRIENDLY);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_SIT);
        me->RemoveAllAuras();
        me->DeleteThreatList();
        me->CombatStop();
        UnkorUnfriendly_Timer = 60000;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if( done_by->GetTypeId() == TYPEID_PLAYER )
            if( (me->GetHealth()-damage)*100 / me->GetMaxHealth() < 30 )
        {
            if( Group* pGroup = ((Player*)done_by)->GetGroup() )
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player *pGroupie = itr->getSource();
                    if( pGroupie &&
                        pGroupie->GetQuestStatus(QUEST_DONTKILLTHEFATONE) == QUEST_STATUS_INCOMPLETE &&
                        pGroupie->GetReqKillOrCastCurrentCount(QUEST_DONTKILLTHEFATONE, 18260) == 10 )
                    {
                        pGroupie->AreaExploredOrEventHappens(QUEST_DONTKILLTHEFATONE);
                        if( !CanDoQuest )
                            CanDoQuest = true;
                    }
                }
            } else
            if( ((Player*)done_by)->GetQuestStatus(QUEST_DONTKILLTHEFATONE) == QUEST_STATUS_INCOMPLETE &&
                ((Player*)done_by)->GetReqKillOrCastCurrentCount(QUEST_DONTKILLTHEFATONE, 18260) == 10 )
            {
                ((Player*)done_by)->AreaExploredOrEventHappens(QUEST_DONTKILLTHEFATONE);
                CanDoQuest = true;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if( CanDoQuest )
        {
            if( !UnkorUnfriendly_Timer.GetInterval() )
            {
                //DoCast(me,SPELL_QUID9889);        //not using spell for now
                DoNice();
            }
            else
            {
                if (UnkorUnfriendly_Timer.Expired(diff))
                {
                    EnterEvadeMode();
                    return;
                }
            }
        }

        if(!UpdateVictim())
            return;

        if (Pulverize_Timer.Expired(diff))
        {
            DoCast(me,SPELL_PULVERIZE);
            Pulverize_Timer = 9000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_unkor_the_ruthless(Creature *creature)
{
    return new mob_unkor_the_ruthlessAI (creature);
}

/*######
## mob_infested_root_walker
######*/

struct mob_infested_root_walkerAI : public ScriptedAI
{
    mob_infested_root_walkerAI(Creature *creature) : ScriptedAI(creature) {}

    void Reset() { }
    void EnterCombat(Unit *who) { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by && done_by->GetTypeId() == TYPEID_PLAYER)
            if (me->GetHealth() <= damage)
                if (rand()%100 < 75)
                    //Summon Wood Mites
                    me->CastSpell(me,39130,true);
    }
};
CreatureAI* GetAI_mob_infested_root_walker(Creature *creature)
{
    return new mob_infested_root_walkerAI (creature);
}

/*######
## mob_rotting_forest_rager
######*/

struct mob_rotting_forest_ragerAI : public ScriptedAI
{
    mob_rotting_forest_ragerAI(Creature *creature) : ScriptedAI(creature) {}

    void Reset() { }
    void EnterCombat(Unit *who) { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (done_by->GetTypeId() == TYPEID_PLAYER)
            if (me->GetHealth() <= damage)
                if (rand()%100 < 75)
                    //Summon Lots of Wood Mights
                    me->CastSpell(me,39134,true);
    }
};
CreatureAI* GetAI_mob_rotting_forest_rager(Creature *creature)
{
    return new mob_rotting_forest_ragerAI (creature);
}

/*######
## mob_netherweb_victim
######*/

#define QUEST_TARGET        22459
//#define SPELL_FREE_WEBBED   38950

const uint32 netherwebVictims[6] =
{
    18470, 16805, 21242, 18452, 22482, 21285
};
struct mob_netherweb_victimAI : public ScriptedAI
{
    mob_netherweb_victimAI(Creature *creature) : ScriptedAI(creature) {}

    void Reset() {}
    void EnterCombat(Unit *who) {}
    void MoveInLineOfSight(Unit *who) {}

    void JustDied(Unit* Killer)
    {
        if (Player* player = Killer->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            if (player->GetQuestStatus(10873) == QUEST_STATUS_INCOMPLETE)
            {
                if (rand() % 100 < 35)
                {
                    DoSpawnCreature(QUEST_TARGET, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                    player->KilledMonster(QUEST_TARGET, me->GetGUID());
                }
                else
                {
                    if(Creature* target = DoSpawnCreature(netherwebVictims[rand() % 6], 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
                        target->AI()->AttackStart(Killer);
                }
            }
            else
            {
                if(Creature* target = DoSpawnCreature(netherwebVictims[rand() % 6], 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
                    target->AI()->AttackStart(Killer);
            }
        }
    }
};
CreatureAI* GetAI_mob_netherweb_victim(Creature *creature)
{
    return new mob_netherweb_victimAI (creature);
}

/*######
## npc_floon
######*/

#define GOSSIP_FLOON1           16062
#define GOSSIP_FLOON2           16063

#define SAY_FLOON_ATTACK        -1000352

#define FACTION_HOSTILE_FL      1738
#define FACTION_FRIENDLY_FL     35

#define SPELL_SILENCE           6726
#define SPELL_FROSTBOLT         9672
#define SPELL_FROST_NOVA        11831

struct npc_floonAI : public ScriptedAI
{
    npc_floonAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked Silence_Timer;
    Timer_UnCheked Frostbolt_Timer;
    Timer_UnCheked FrostNova_Timer;

    void Reset()
    {
        Silence_Timer.Reset(2000);
        Frostbolt_Timer.Reset(4000);
        FrostNova_Timer.Reset(9000);
        me->setFaction(FACTION_FRIENDLY_FL);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (Silence_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SILENCE);
            Silence_Timer = 30000;
        }

        if (FrostNova_Timer.Expired(diff))
        {
            DoCast(me,SPELL_FROST_NOVA);
            FrostNova_Timer = 20000;
        }

        if (Frostbolt_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_FROSTBOLT);
            Frostbolt_Timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_floon(Creature *creature)
{
    return new npc_floonAI (creature);
}

bool GossipHello_npc_floon(Player *player, Creature *creature )
{
    if( player->GetQuestStatus(10009) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_FLOON1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_floon(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(GOSSIP_FLOON2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(9443, creature->GetGUID());
    }
    if( action == GOSSIP_ACTION_INFO_DEF+1 )
    {
        player->CLOSE_GOSSIP_MENU();
        creature->setFaction(FACTION_HOSTILE_FL);
        DoScriptText(SAY_FLOON_ATTACK, creature, player);
        ((npc_floonAI*)creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## npc_skyguard_handler_deesak
######*/

#define GOSSIP_SKYGUARD 16064

bool GossipHello_npc_skyguard_handler_deesak(Player *player, Creature *creature )
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu( creature->GetGUID() );

    if (player->GetReputationMgr().GetRank(1031) >= REP_HONORED)
        player->ADD_GOSSIP_ITEM( 2, player->GetSession()->GetHellgroundString(GOSSIP_SKYGUARD), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_skyguard_handler_deesak(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,41279,true);               //TaxiPath 705 (Taxi - Skettis to Skyguard Outpost)
    }
    return true;
}

/*######
## npc_isla_starmane
######*/

#define SAY_PROGRESS_1  -1000353
#define SAY_PROGRESS_2  -1000354
#define SAY_PROGRESS_3  -1000355
#define SAY_PROGRESS_4  -1000356

#define QUEST_EFTW_H    10052
#define QUEST_EFTW_A    10051
#define GO_CAGE         182794
#define SPELL_CAT       32447

struct npc_isla_starmaneAI : public npc_escortAI
{
    npc_isla_starmaneAI(Creature* creature) : npc_escortAI(creature) {}

    bool Completed;
    Timer RootsTimer;
    Timer MoonfireTimer;
    Timer WrathTimer;

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if(!player)
            return;

        switch (i)
        {
		case 0: {
			me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
			me->SetReactState(REACT_AGGRESSIVE);
			GameObject* Cage = FindGameObject(GO_CAGE, 10, me);
			if (Cage)
				Cage->SetGoState(GO_STATE_ACTIVE);
			}break;
            case 2: DoScriptText(SAY_PROGRESS_1, me, player); break;
            case 5: DoScriptText(SAY_PROGRESS_2, me, player); break;
            case 6: DoScriptText(SAY_PROGRESS_3, me, player); break;
            case 29:DoScriptText(SAY_PROGRESS_4, me, player);
                if (player)
                {
                    if (player->GetTeam() == ALLIANCE)
                        player->GroupEventHappens(QUEST_EFTW_A, me);
                    else if (player->GetTeam() == HORDE)
                        player->GroupEventHappens(QUEST_EFTW_H, me);
                } 
                Completed = true;
                me->SetInFront(player); break;
            case 30: me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE); break;
            case 31: DoCast(me, SPELL_CAT);
                me->SetWalk(false); break;
        }
    }

    void Reset()
    {
        Completed = false;
        if (HasEscortState(STATE_ESCORT_NONE))
            me->setFaction(1728);
        RootsTimer.Reset(500);
        MoonfireTimer.Reset(4000);
        WrathTimer.Reset(2000);

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void EnterCombat(Unit* who){}

    void JustDied(Unit* killer)
    {
        if (Player* player = GetPlayerForEscort())
        {
            if (player->GetTeam() == ALLIANCE)
            {
                if (player->GetQuestStatus(QUEST_EFTW_A) != QUEST_STATUS_COMPLETE)
                    player->FailQuest(QUEST_EFTW_A);
            }
            else if (player->GetTeam() == HORDE)
            {
                if (player->GetQuestStatus(QUEST_EFTW_H) != QUEST_STATUS_COMPLETE)
                    player->FailQuest(QUEST_EFTW_H);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        if(!UpdateVictim())
            return;
        
        if(RootsTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 33844, false);
            RootsTimer = 8000;
        }

        if(WrathTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 9739, false);
            WrathTimer = 2000;
        }

        if(MoonfireTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 15798, false);
            MoonfireTimer = 12000;
        }
    }
};

bool QuestAccept_npc_isla_starmane(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_EFTW_H || quest->GetQuestId() == QUEST_EFTW_A)
    {
		if (npc_escortAI* pEscortAI = CAST_AI(npc_isla_starmaneAI, creature->AI()))
		{
			pEscortAI->Start(true, true, player->GetGUID(), quest);
		}
    }
    return true;
}

CreatureAI* GetAI_npc_isla_starmane(Creature* creature)
{
    return new npc_isla_starmaneAI(creature);
}

/*######
## go_skull_pile
######*/
#define GOSSIP_S_DARKSCREECHER_AKKARAI          16065
#define GOSSIP_S_KARROG                         16066
#define GOSSIP_S_GEZZARAK_THE_HUNTRESS          16067
#define GOSSIP_S_VAKKIZ_THE_WINDRAGER           16068

bool GossipHello_go_skull_pile(Player *player, GameObject* go)
{
    player->PlayerTalkClass->ClearMenus();
    if ((player->GetQuestStatus(11885) == QUEST_STATUS_INCOMPLETE) || player->GetQuestRewardStatus(11885) || (player->GetQuestStatus(11074) == QUEST_STATUS_INCOMPLETE) || player->GetQuestRewardStatus(11074))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_S_DARKSCREECHER_AKKARAI), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_S_KARROG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_S_GEZZARAK_THE_HUNTRESS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_S_VAKKIZ_THE_WINDRAGER), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
    }

    player->SEND_GOSSIP_MENU(go->GetGOInfo()->questgiver.gossipID, go->GetGUID());
    return true;
}

void SendActionMenu_go_skull_pile(Player *player, GameObject* go, uint32 action)
{
    // GO should be despawned by spell cast below, but it's not working :(
    if(player->HasItemCount(32620, 10))
    {
        go->SetGoState(GO_STATE_ACTIVE);
        go->SetRespawnTime(600);
        player->CLOSE_GOSSIP_MENU();
    }

    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
              player->CastSpell(player,40642,false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
              player->CastSpell(player,40640,false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
              player->CastSpell(player,40632,false);
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
              player->CastSpell(player,40644,false);
            break;
    }
}

bool GossipSelect_go_skull_pile(Player *player, GameObject* go, uint32 sender, uint32 action )
{
    switch(sender)
    {
        case GOSSIP_SENDER_MAIN:    SendActionMenu_go_skull_pile(player, go, action); break;
    }
    return true;
}

/*######
## go_ancient_skull_pile
######*/
#define GOSSIP_S_TEROKK         16069

bool GossipHello_go_ancient_skull_pile(Player *player, GameObject* go)
{
    player->PlayerTalkClass->ClearMenus();
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_S_TEROKK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->SEND_GOSSIP_MENU(go->GetGOInfo()->questgiver.gossipID, go->GetGUID());
    return true;
}

bool GossipSelect_go_ancient_skull_pile(Player *player, GameObject* go, uint32 sender, uint32 action )
{
    switch(sender)
    {
        case GOSSIP_SENDER_MAIN:
            // player->CastSpell(player,41004,false);
            // terokk should be summoned by above spell, but it doesn't work; summoninng him in other way
            if(player->HasItemCount(32720, 1))
            {
                player->DestroyItemCount(32720, 1, true, false);
                player->SummonCreature(21838, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                player->CLOSE_GOSSIP_MENU();
                go->SetGoState(GO_STATE_ACTIVE);
                go->SetRespawnTime(600);
            }
            break;
    }
    return true;
}

/*######
## mob_terokk
######*/

#define SPELL_SHADOW_BOLT_VOLLEY    40721
#define SPELL_CLEAVE                15284
#define SPELL_DIVINE_SHIELD         40733
#define SPELL_ENRAGE                28747
#define SPELL_WILL_OF_ARRAKOA_GOD   40722
#define SPELL_CHOSEN_ONE            40726
#define SPELL_ANCIENT_FLAMES        40657
#define SPELL_SKYGUARD_FLARE_TARGET 40656
#define SPELL_SKYGUARD_FLARE        40655

#define SKYGUARD_WP_CIRCLE_MAX      6
#define SKYGUARD_WP_MIDDLE          7
#define SKYGUARD_WP_DESPAWN         8
#define SKYGUARD_WP_AFTER_SPAWN     9

float skyguardAltitude = 324;
float groundAltitiude = 286;

float skyguardSpawn[3][2] = {
    { -3637, 3572 },
    { -3645, 3574 },
    { -3641, 3564 }
};

float skyguardWPStart[3][2] = {
    { -3803, 3504 },
    { -3811, 3506 },
    { -3807, 3496 }
};

#define SKYGUARD_WP_MIDDLE_MAX 4
float skyguardWPMiddle[4][3] = {
    {-3785, 3507, 315},
    {-3798, 3500, 316},
    {-3798, 3515, 308},
    {-3786, 3515, 314}
};

float skyguardWPs[6][2] = {
    { -3830, 3513 },
    { -3795, 3549 },
    { -3761, 3538 },
    { -3759, 3498 },
    { -3783, 3467 },
    { -3811, 3494 }
};

struct mob_terokkAI : public ScriptedAI
{
    mob_terokkAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked ShadowBoltVolley_Timer;
    Timer_UnCheked Cleave_Timer;
    Timer_UnCheked ChosenOne_Timer;
    Timer_UnCheked ChosenOneActive_Timer;
    Timer_UnCheked SkyguardFlare_Timer;
    uint64 ChosenOneTarget;
    uint64 SkyguardGUIDs[3];
    Timer_UnCheked CheckTimer;
    uint8 phase;            // 0: 100% to 70% hp, 1: under 30% hp and shield up, 2: under 30% hp and shield down
    uint8 skyguardTurn;

    void Reset()
    {
        ResetSkyguards();
        ShadowBoltVolley_Timer.Reset(5000);
        Cleave_Timer.Reset(7000);
        ChosenOne_Timer.Reset(30000);
        ChosenOneActive_Timer = 0;
        ChosenOneTarget = 0;
        phase = 0;
        skyguardTurn = 0;
        for(int i = 0; i < 3; i++)
            SkyguardGUIDs[i] = 0;
    }

    void EnterCombat(Unit *who) {}

    void Despawn(Unit *unit)
    {
        unit->CombatStop();
        unit->AddObjectToRemoveList();
    }

    void ResetSkyguards()
    {
        for(int i = 0; i < 3; i++)
        {
            if(SkyguardGUIDs[i])
                if(Creature* skyguard = Creature::GetCreature(*me, SkyguardGUIDs[i]))
                    skyguard->GetMotionMaster()->MovePoint(SKYGUARD_WP_DESPAWN, skyguardSpawn[i][0], skyguardSpawn[i][1], skyguardAltitude);
            SkyguardGUIDs[i] = 0;
        }
    }

    void EnterEvadeMode()
    {
        ResetSkyguards();
        ScriptedAI::EnterEvadeMode();
    }

    void JustDied(Unit *)
    {
        ResetSkyguards();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ShadowBoltVolley_Timer.Expired(diff))
        {
            DoCast(me, SPELL_SHADOW_BOLT_VOLLEY);
            ShadowBoltVolley_Timer = 10000 + rand()%5000;
        }

        if (Cleave_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE);
            Cleave_Timer = 7000 + rand() % 2000;
        }

        if (ChosenOneTarget)
        {
            if (ChosenOneActive_Timer.Expired(diff))
            {
                if(me->getVictimGUID() == ChosenOneTarget)
                    me->AddThreat(me->GetVictim(), -500000.0f);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                ChosenOneActive_Timer = 0;
                ChosenOneTarget = 0;
            }
        }

        if(phase == 0)
        {
            if (ChosenOne_Timer.Expired(diff))
            {
                if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                {
                    DoCast(target,SPELL_CHOSEN_ONE);
                    me->AddThreat(target, 500000.0f);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    ChosenOneTarget = target->GetGUID();
                    ChosenOneActive_Timer = 12000;
                    ChosenOne_Timer = 30000 + rand() % 20000;
                }
            }
        }

        if(phase == 0 && me->GetHealth() / (float)me->GetMaxHealth() < 0.30f)
        {
            phase = 1;
            DoCast(me, SPELL_DIVINE_SHIELD, true);
            for(int i = 0; i < 3; i++)
                if(Creature *skyguard = me->SummonCreature(23377, skyguardSpawn[i][0], skyguardSpawn[i][1], skyguardAltitude, 3.30f, TEMPSUMMON_MANUAL_DESPAWN, 0))
                {
                    SkyguardGUIDs[i] = skyguard->GetGUID();
                    skyguard->setActive(true);
                    skyguard->GetMotionMaster()->MovePoint(SKYGUARD_WP_AFTER_SPAWN, skyguardWPStart[i][0], skyguardWPStart[i][1], skyguardAltitude );
                    skyguard->Mount(21158);
                }

            CheckTimer = 2000;
            SkyguardFlare_Timer = 15000;
        }

        if(phase > 0)
        {
            if (CheckTimer.Expired(diff))
            {
                std::list<Creature*> skyguardTargets = FindAllCreaturesWithEntry(23277, 5);
                if(phase == 1)
                {
                    if(!skyguardTargets.empty() && !(*skyguardTargets.begin())->HasAura(SPELL_SKYGUARD_FLARE, 0))
                    {
                        me->RemoveAurasDueToSpell(SPELL_DIVINE_SHIELD);
                        DoCast(me, SPELL_ENRAGE, true);
                        phase = 2;
                    }
                }
                else    // phase == 2
                {
                    if(skyguardTargets.empty())
                    {
                        me->RemoveAurasDueToSpell(SPELL_ENRAGE);
                        DoCast(me, SPELL_DIVINE_SHIELD, true);
                        phase = 1;
                    }
                }
                CheckTimer = 2000;
            }

            if (SkyguardFlare_Timer.Expired(diff))
            {
                if(Creature *skyguard = Creature::GetCreature(*me, SkyguardGUIDs[skyguardTurn++])){
                    uint32 i = rand() % SKYGUARD_WP_MIDDLE_MAX;
                    skyguard->GetMotionMaster()->MovePoint(SKYGUARD_WP_MIDDLE, skyguardWPMiddle[i][0], skyguardWPMiddle[i][1], skyguardWPMiddle[i][2]);
                }
                skyguardTurn %= 3;
                SkyguardFlare_Timer = 20000;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_terokk(Creature *creature)
{
    return new mob_terokkAI (creature);
}

/*
* npc_skyguard_ace
*/

//Trentone - seems ok, dont know why to rewrite it
//TODO: Rewrite with proper timers
struct npc_skyguard_aceAI : public ScriptedAI
{

    npc_skyguard_aceAI(Creature* creature) : ScriptedAI(creature) {}

    uint64 TargetGUID;
    Timer_UnCheked TargetLifetime;
    Timer_UnCheked AncientFlame_Timer;
    Timer_UnCheked Move_Timer;
    int NextWP;

    void Reset()
    {
        TargetGUID = 0;
        TargetLifetime = 0;
        Move_Timer = 0;
        AncientFlame_Timer = 0;
    }

    void EnterCombat(Unit *who) {}

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id < SKYGUARD_WP_CIRCLE_MAX)
        {
            NextWP = (id + 1) % SKYGUARD_WP_CIRCLE_MAX;
            Move_Timer = 1;
        }
        else if(id == SKYGUARD_WP_AFTER_SPAWN)
        {
            NextWP = rand() % SKYGUARD_WP_CIRCLE_MAX;
            Move_Timer = 1;
        }
        else if(id == SKYGUARD_WP_DESPAWN)
        {
            me->CombatStop();
            me->AddObjectToRemoveList();
        }
        else if(id == SKYGUARD_WP_MIDDLE)
        {
            DoCast(me, SPELL_SKYGUARD_FLARE, false);
            NextWP = rand() % SKYGUARD_WP_CIRCLE_MAX;
            Move_Timer = 9000;
        }

    }

    void JustSummoned(Creature *creature)
    {
        if(creature->GetEntry() == 23277)
        {
            float x, y, z;
            creature->GetPosition(x, y, z);
            z = groundAltitiude;
            creature->Relocate(x, y, z);
            creature->CastSpell(creature, SPELL_SKYGUARD_FLARE_TARGET, false);
            TargetGUID = creature->GetGUID();
            TargetLifetime = 20500;
            AncientFlame_Timer = 5500;
            DoCast(creature, SPELL_ANCIENT_FLAMES, false);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(TargetGUID && TargetLifetime.Expired(diff))
        {
            if(Unit* unit = me->GetMap()->GetCreature(TargetGUID))
            {
                unit->CombatStop();
                unit->AddObjectToRemoveList();
            }
            TargetGUID = 0;
        }

        
        if (Move_Timer.Expired(diff))
        {
            if (me->GetMotionMaster()->empty())
                me->GetMotionMaster()->MovePoint(NextWP, skyguardWPs[NextWP][0], skyguardWPs[NextWP][1], skyguardAltitude);
            Move_Timer = 0;
        }



        if (AncientFlame_Timer.Expired(diff))
        {
            if (Unit* target = me->GetMap()->GetCreature(TargetGUID))
            {
                target->CastStop();
                target->RemoveAurasDueToSpell(SPELL_SKYGUARD_FLARE_TARGET);
                // HACK: cast ancient flames so players can be damaged by it, we keep cast ancient flames by skyguard ace only for animation
                target->CastSpell(target, SPELL_ANCIENT_FLAMES, true);
            }
            AncientFlame_Timer = 0;
        }

        
    }
};

CreatureAI* GetAI_npc_skyguard_ace(Creature *creature)
{
    return new npc_skyguard_aceAI (creature);
}


/***
Script for Quest: Hungry Nether Rays (11093)
***/
struct npc_blackwing_warp_chaser : public ScriptedAI
{
    npc_blackwing_warp_chaser(Creature *creature) : ScriptedAI(creature) {}

    void JustDied(Unit* slayer)
    {
        if(slayer->GetTypeId()==TYPEID_PLAYER && ((Player*)(slayer))->GetQuestStatus(11093)==QUEST_STATUS_INCOMPLETE)
        {
            Unit* HungryNetherRay = FindCreature(23439, 50, me);
            // to avoid leeching by other players:
            if(HungryNetherRay && HungryNetherRay->GetOwner()==slayer)
            {
                // cast Lucille Feed Credit Trigger
                HungryNetherRay->CastSpell(HungryNetherRay, 41427, true);
            }
        }
    }

    void EnterCombat(Unit *who) {}

    void Reset()
    {

    }
};

CreatureAI* GetAI_npc_blackwing_warp_chaser(Creature *creature)
{
    return new npc_blackwing_warp_chaser(creature);
}
/*######
## npc_skyguard_prisoner
######*/

#define SAY_PROGRESS_1_SG_PRISONER  -1600004 //"Thanks for your help. Let's get out of here!"
#define SAY_PROGRESS_2_SG_PRISONER  -1600005 //"Let's keep moving. I don't like this place."
#define SAY_PROGRESS_3_SG_PRISONER  -1600006 //"Thanks again. Sergant Doryn will be glad to hear he has one less scout to replace this week."

#define GO_CAGE_SG_PRISONER 185952
#define QUEST_ESC   11085
#define SKETTIS_AMBUSH  21644

struct npc_skyguard_prisonerAI : public npc_escortAI
{
    npc_skyguard_prisonerAI(Creature* creature) : npc_escortAI(creature) {}

    bool Cpl;

    void Reset()
    {
        Cpl = false;
    }

    void EnterCombat(Unit* who) {}

    void JustSummoned(Creature *summoned)
    {
        summoned->AI()->AttackStart(me);
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        switch(i)
        {
        case 0:
            {
            GameObject* Cage = FindGameObject(GO_CAGE_SG_PRISONER, 10, me);
            if(Cage)
                Cage->SetGoState(GO_STATE_ACTIVE);
            DoScriptText(SAY_PROGRESS_1_SG_PRISONER, me, player);
            }
            break;
        case 10: me->SummonCreature(SKETTIS_AMBUSH, -4182.85, 3075.50, 333.15, 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            me->SummonCreature(SKETTIS_AMBUSH, -4180.54, 3075.50, 333.15, 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            break;
        case 11: DoScriptText(SAY_PROGRESS_2_SG_PRISONER, me, player);
            break;
        case 12: DoScriptText(SAY_PROGRESS_3_SG_PRISONER, me, player);
            Cpl=true;
            if(player)
                player->GroupEventHappens(QUEST_ESC, me);
            break;
        }
    }

    void JustDied(Unit* killer)
    {
        Player* player = GetPlayerForEscort();
        if (player && !Cpl)
        {
                player->FailQuest(QUEST_ESC);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_skyguard_prisoner(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ESC)
    {
        ((npc_escortAI*)(creature->AI()))->Start(true, false, player->GetGUID(), quest);
    }
    return true;
}

CreatureAI* GetAI_npc_skyguard_prisoner(Creature* creature)
{
    return new npc_skyguard_prisonerAI(creature);
}

/*######
## npc_letoll
######*/

enum
{
    SAY_LE_START                    = -1000510,
    SAY_LE_KEEP_SAFE                = -1000511,
    SAY_LE_NORTH                    = -1000512,
    SAY_LE_ARRIVE                   = -1000513,
    SAY_LE_BURIED                   = -1000514,
    SAY_LE_ALMOST                   = -1000515,
    SAY_LE_DRUM                     = -1000516,
    SAY_LE_DRUM_REPLY               = -1000517,
    SAY_LE_DISCOVERY                = -1000518,
    SAY_LE_DISCOVERY_REPLY          = -1000519,
    SAY_LE_NO_LEAVE                 = -1000520,
    SAY_LE_NO_LEAVE_REPLY1          = -1000521,
    SAY_LE_NO_LEAVE_REPLY2          = -1000522,
    SAY_LE_NO_LEAVE_REPLY3          = -1000523,
    SAY_LE_NO_LEAVE_REPLY4          = -1000524,
    SAY_LE_SHUT                     = -1000525,
    SAY_LE_REPLY_HEAR               = -1000526,
    SAY_LE_IN_YOUR_FACE             = -1000527,
    SAY_LE_HELP_HIM                 = -1000528,
    EMOTE_LE_PICK_UP                = -1000529,
    SAY_LE_THANKS                   = -1000530,

    QUEST_DIGGING_BONES             = 10922,

    NPC_RESEARCHER                  = 22464,
    NPC_BONE_SIFTER                 = 22466,

    MAX_RESEARCHER                  = 4
};

struct npc_letollAI : public npc_escortAI
{
    npc_letollAI(Creature* creature) : npc_escortAI(creature)
    {
        EventTimer.Reset(5000);
        EventCount = 0;
        Reset();
    }

    std::list<uint64> ResearchersList;

    Timer_UnCheked EventTimer;
    uint32 EventCount;

    void Reset() {}

    void SetFormation()
    {
        uint32 uiCount = 0;

        for (std::list<uint64>::iterator itr = ResearchersList.begin(); itr != ResearchersList.end(); ++itr)
        {
            if (Creature* it = me->GetCreature(*itr))
            {
                float fAngle = uiCount < MAX_RESEARCHER ? M_PI/MAX_RESEARCHER - (uiCount*2*M_PI/MAX_RESEARCHER) : 0.0f;

                if (it->isAlive() && !it->IsInCombat())
                    it->GetMotionMaster()->MoveFollow(me, 2.5f, fAngle);

                ++uiCount;
            }
        }
    }

    Creature* GetAvailableResearcher(uint8 ListNum)
    {
        if (!ResearchersList.empty())
        {
            uint8 Num = 1;

            for (std::list<uint64>::iterator itr = ResearchersList.begin(); itr != ResearchersList.end(); ++itr)
            {
                if (ListNum && ListNum != Num)
                {
                    ++Num;
                    continue;
                }

                if (Creature* it = me->GetCreature(*itr))
                {
                    if (it->isAlive() && it->IsWithinDistInMap(me, 20.0f))
                        return it;
                }
            }
        }

        return NULL;
    }

    void JustStarted()
    {
        EventTimer = 5000;
        EventCount = 0;

        ResearchersList.clear();

        float x, y, z;
        me->GetPosition(x, y, z);

        std::list<Creature*> ResearchersListPtr;
        Hellground::AllCreaturesOfEntryInRange check(me, NPC_RESEARCHER, 30.0f);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(ResearchersListPtr, check);
        Cell::VisitGridObjects(me, searcher, 30.0f);

        for (std::list<Creature*>::iterator itr = ResearchersListPtr.begin(); itr != ResearchersListPtr.end(); ++itr)
            ResearchersList.push_back((*itr)->GetGUID());

        if (!ResearchersList.empty())
            SetFormation();
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 0:
                JustStarted();
                if (Player* player = GetPlayerForEscort())
                    DoScriptText(SAY_LE_KEEP_SAFE, me, player);
                break;
            case 1:
                DoScriptText(SAY_LE_NORTH, me);
                SetFormation();
                break;
            case 10:
                DoScriptText(SAY_LE_ARRIVE, me);
                SetFormation();
                break;
            case 12:
                DoScriptText(SAY_LE_BURIED, me);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_NOSHEATHE_MINING);
                for (std::list<uint64>::iterator itr = ResearchersList.begin(); itr != ResearchersList.end(); ++itr)
                {
                    if (Creature* it = me->GetCreature(*itr))
                        it->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_NOSHEATHE_MINING);
                }
                SetEscortPaused(true);
                SetFormation();
                break;
            case 13:
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
                for (std::list<uint64>::iterator itr = ResearchersList.begin(); itr != ResearchersList.end(); ++itr)
                {
                    if (Creature* it = me->GetCreature(*itr))
                        it->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
                }
                SetRun();
                SetFormation();
                break;
            case 20:
                if (Player* player = GetPlayerForEscort())
                {
                    DoScriptText(SAY_LE_THANKS, me, player);
                    player->GroupEventHappens(QUEST_DIGGING_BONES, me);
                }
            default: SetFormation(); break;
                break;
        }
    }

    void EnterEvadeMode()
    {
        npc_escortAI::EnterEvadeMode();
        for (std::list<uint64>::iterator itr = ResearchersList.begin(); itr != ResearchersList.end(); ++itr)
        {
            if (Creature* it = me->GetCreature(*itr))
            {
                float x, y, z, o;
                it->GetHomePosition(x, y, z, o);
                it->GetMotionMaster()->MovePoint(100, x, y, z);
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if (who->IsInCombat() && who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == NPC_BONE_SIFTER)
            DoScriptText(SAY_LE_HELP_HIM, me);

        for (std::list<uint64>::iterator itr = ResearchersList.begin(); itr != ResearchersList.end(); ++itr)
        {
            if (Creature* it = me->GetCreature(*itr))
            {
                float x, y, z, o;
                me->GetHomePosition(x, y, z, o);
                it->SetHomePosition(x, y, z, o);
                it->AI()->AttackStart(who);
            }
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(me);
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED) && EventTimer.Expired(diff))
            {
                EventTimer = 7000;

                switch(EventCount)
                {
                    case 0:
                        DoScriptText(SAY_LE_ALMOST, me);
                        break;
                    case 1:
                        DoScriptText(SAY_LE_DRUM, me);
                        break;
                    case 2:
                        if (Creature* Researcher = GetAvailableResearcher(0))
                            DoScriptText(SAY_LE_DRUM_REPLY, Researcher);
                        break;
                    case 3:
                        DoScriptText(SAY_LE_DISCOVERY, me);
                        break;
                    case 4:
                        if (Creature* Researcher = GetAvailableResearcher(0))
                            DoScriptText(SAY_LE_DISCOVERY_REPLY, Researcher);
                        break;
                    case 5:
                        DoScriptText(SAY_LE_NO_LEAVE, me);
                        break;
                    case 6:
                        if (Creature* Researcher = GetAvailableResearcher(1))
                            DoScriptText(SAY_LE_NO_LEAVE_REPLY1, Researcher);
                        break;
                    case 7:
                        if (Creature* Researcher = GetAvailableResearcher(2))
                            DoScriptText(SAY_LE_NO_LEAVE_REPLY2, Researcher);
                        break;
                    case 8:
                        if (Creature* Researcher = GetAvailableResearcher(3))
                            DoScriptText(SAY_LE_NO_LEAVE_REPLY3, Researcher);
                        break;
                    case 9:
                        if (Creature* Researcher = GetAvailableResearcher(4))
                            DoScriptText(SAY_LE_NO_LEAVE_REPLY4, Researcher);
                        break;
                    case 10:
                        DoScriptText(SAY_LE_SHUT, me);
                        break;
                    case 11:
                        if (Creature* Researcher = GetAvailableResearcher(0))
                            DoScriptText(SAY_LE_REPLY_HEAR, Researcher);
                        break;
                    case 12:
                        DoScriptText(SAY_LE_IN_YOUR_FACE, me);
                        me->SummonCreature(NPC_BONE_SIFTER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                        break;
                    case 13:
                        DoScriptText(EMOTE_LE_PICK_UP, me);
                        SetEscortPaused(false);
                        break;
                }

                ++EventCount;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_letollAI(Creature* creature)
{
    return new npc_letollAI(creature);
}

bool QuestAccept_npc_letoll(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_DIGGING_BONES)
    {
        if (npc_letollAI* pEscortAI = dynamic_cast<npc_letollAI*>(creature->AI()))
        {
            DoScriptText(SAY_LE_START, creature);
            creature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);

            pEscortAI->Start(true, false, player->GetGUID(), quest, true);
        }
    }

    return true;
}

/*######
## npc_sarthis & his companions
######*/

#define GOSSIP_SARTHIS_SELECT       16070

static float SummonCoord[][3] =
{
    {-2469.45, 4696.66, 157.01}
};

static float MinionCoord[][3] =
{
    {-2459.4, 4689.6, 165.5},    //Air start
    {-2474.5, 4685.7, 156.7},    //Air final
    {-2485.7, 4678.7, 157.0},    //Water start
    {-2483.0, 4703.3, 154.3},    //Water final
    {-2480.9, 4676.1, 157.8},    //Earth start
    {-2479.0, 4689.4, 155.4},    //Earth final
    {-2478.8, 4720.4, 153.5},    //Fire start
    {-2476.0, 4707.2, 155.0}     //Fire final
};

enum eSarthis
{
    QUEST_THE_SOUL_CANNON_OF_RETHHEDRON     = 11089,

    SPELL_SUMMON_AIR_ELEMENTAL              = 40129,
    SPELL_SUMMON_WATER_ELEMENTAL            = 40130,
    SPELL_SUMMON_EARTH_ELEMENTAL            = 40132,
    SPELL_SUMMON_FIRE_ELEMENTAL             = 40133,
    SPELL_SUMMON_ARCANE_ELEMENTAL           = 40134,
    SPELL_RED_BEAM                          = 40228,
    SPELL_GREEN_BEAM                        = 40227,
    SPELL_BLUE_BEAM                         = 40225,

    SAY_SARTHIS_INTRO                       = -1600007, // "So my blood was not a sufficient payment, eh? Fine, let us recover your arcane essence. After this, I owe Balthas nothing."
    EMOTE_SARTHIS_FETISH                    = -1600008, // "Sar\'this places a fetish at the ritual pile."
    SAY_SARTHIS_START                       = -1600009, // "The process is arduous. We must first summon forth acolytes of the elements. you must then destroy these acolytes so that my minions can make preparations."
    SAY_SARTHIS_KILLED_ACOLYTE              = -1600010, // "Well done! Let\'s continue."
    SAY_SARTHIS_WATER                       = -1600011, // "Prepare yourself! The acolyte of water is soon to come..."
    SAY_SARTHIS_EARTH                       = -1600012, // "Come forth, acolyte of earth!"
    SAY_SARTHIS_FIRE                        = -1600013, // "Fire, show yourself!"
    SAY_SARTHIS_ARCANE                      = -1600014, // "Now we call forth the the arcane acolyte."
    SAY_ELEMENTAL_1                         = -1600015, // "I require your life essence to maintain my existence in this realm."
    SAY_SARTHIS_FINAL                       = -1600016, // "It is yours my lord!"

    GO_SARTHIS_FETISH                       = 185856,
    NPC_SARTHIS                             = 23093,
    NPC_MINION_OF_SARTHIS                   = 23094,
    NPC_ARCANE_ELEMENTAL                    = 23100
};

struct npc_sarthisAI : public npc_escortAI
{
    npc_sarthisAI(Creature* creature) : npc_escortAI(creature) {}

    uint64 fetishGUID;
    std::list<uint64> MinionGUID;
    bool speech;
    Timer_UnCheked SpeechTimer;
    Timer_UnCheked SpeechTimer2;
    Timer_UnCheked CastTimer;
    Timer_UnCheked ResetTimer;

    void Reset()
    {
        fetishGUID = 0;
        CastTimer = 0;
        speech = false;
        MinionGUID.clear();
        SpeechTimer = 0;
        SpeechTimer2 = 0;
        ResetTimer.Reset(300000);
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        switch(i)
        {
            case 2:
            {
                DoScriptText(EMOTE_SARTHIS_FETISH, me, player);
                GameObject* Fetish = me->SummonGameObject(GO_SARTHIS_FETISH, SummonCoord[0][0], SummonCoord[0][1], SummonCoord[0][2], 0, 0, 0, 0, 0, 180000);
                Unit* Trigger = me->SummonCreature(12999, SummonCoord[0][0], SummonCoord[0][1], SummonCoord[0][2], 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                if(Fetish)
                    fetishGUID = Fetish->GetGUID();
                DoScriptText(SAY_SARTHIS_START, me, player);
                break;
            }
            case 6:
            {
                SetEscortPaused(true);
                if(Unit* Minion = FindCreature(NPC_MINION_OF_SARTHIS, 20, me))
                    MinionGUID.push_front(Minion->GetGUID());
                DoCast(me, SPELL_SUMMON_AIR_ELEMENTAL);
                break;
            }
            case 11:
            {
                SetEscortPaused(true);
                if(Unit* Minion = FindCreature(NPC_MINION_OF_SARTHIS, 20, me))
                    MinionGUID.push_front(Minion->GetGUID());
                DoScriptText(SAY_SARTHIS_WATER, me, player);
                DoCast(me, SPELL_SUMMON_WATER_ELEMENTAL);
                break;
            }
            case 15:
            {
                SetEscortPaused(true);
                if(Unit* Minion = FindCreature(NPC_MINION_OF_SARTHIS, 30, me))
                    MinionGUID.push_front(Minion->GetGUID());
                DoScriptText(SAY_SARTHIS_EARTH, me, player);
                DoCast(me, SPELL_SUMMON_EARTH_ELEMENTAL);
                break;
            }
            case 21:
            {
                SetEscortPaused(true);
                if(Unit* Minion = FindCreature(NPC_MINION_OF_SARTHIS, 30, me))
                    MinionGUID.push_front(Minion->GetGUID());
                DoScriptText(SAY_SARTHIS_FIRE, me, player);
                DoCast(me, SPELL_SUMMON_FIRE_ELEMENTAL);
                break;
            }
            case 23:
            {
                uint8 j = 0;
                SetEscortPaused(true);
                for(std::list<uint64>::iterator i = MinionGUID.begin(); i != MinionGUID.end(); ++i)
                {
                    if(Unit* Minion = me->GetMap()->GetCreature(*i))
                    {
                        Minion->Relocate(MinionCoord[j][0], MinionCoord[j][1], MinionCoord[j][2], 0);
                        j++;
                        Minion->GetMotionMaster()->Clear();
                        Minion->GetMotionMaster()->MoveIdle();
                        Minion->GetMotionMaster()->MovePoint(1, MinionCoord[j][0], MinionCoord[j][1], MinionCoord[j][2]);
                        j++;
                    }
                }
                if(Unit* Fetish = FindCreature(12999, 40, me))
                    me->SetFacingToObject(Fetish);
                DoScriptText(SAY_SARTHIS_ARCANE, me, player);
                speech = true;
                SpeechTimer = 15000;
                CastTimer = 10000;
            }
        }
    }

    void JustDied(Unit* who)
    {
        float x, y, z, o;
        for(std::list<uint64>::iterator i = MinionGUID.begin(); i != MinionGUID.end(); ++i)
        {
            if(Unit* Minion = me->GetMap()->GetCreature(*i))
            {
                ((Creature*)Minion)->GetHomePosition(x, y, z, o);
                Minion->Kill(Minion, false);
                Minion->Relocate(x, y, z, o);
                ((Creature*)Minion)->Respawn();
            }
        }
        if(Unit* Fetish = FindCreature(12999, 40, me))
            Fetish->Kill(Fetish, false);
    }

    void DamageTaken(Unit* done_by, uint32& )
    {
        if(done_by->GetTypeId() == TYPEID_UNIT)
            done_by->Kill(me, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(speech && CastTimer.Expired(diff))
        {
            DoCast(me, SPELL_SUMMON_ARCANE_ELEMENTAL);
            CastTimer = 120000; //not let cast again;
        }

        if (speech && SpeechTimer.Expired(diff))
        {
            if(Unit* ArcaneAcolyte = FindCreature(NPC_ARCANE_ELEMENTAL, 30, me))
            {
                me->SetFacingToObject(ArcaneAcolyte);
                ArcaneAcolyte->MonsterSay(SAY_ELEMENTAL_1, 0, me->GetGUID());
            }
            SpeechTimer = 120000;   //not let speak again;
            SpeechTimer2 = 4000;
        }

        if(speech && SpeechTimer2.Expired(diff))
        {
            if(Unit* ArcaneAcolyte = FindCreature(NPC_ARCANE_ELEMENTAL, 30, me))
            {
                DoScriptText(SAY_SARTHIS_FINAL, me, ArcaneAcolyte);
                ArcaneAcolyte->setFaction(16);
                ((Creature*)ArcaneAcolyte)->AI()->AttackStart(me);
            }
            speech = false;
        }

        if (HasEscortState(STATE_ESCORT_PAUSED) && ResetTimer.Expired(diff))
        {
            me->Kill(me, false);
            me->Respawn();

            if(Unit* ArcaneAcolyte = FindCreature(NPC_ARCANE_ELEMENTAL, 100, me))
                ((Creature*)ArcaneAcolyte)->ForcedDespawn();

            ResetTimer = 300000;
        }

        npc_escortAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_npc_sarthisAI(Creature* creature)
{
    return new npc_sarthisAI(creature);
}

struct npc_sarthis_elementalAI : public ScriptedAI
{
    npc_sarthis_elementalAI(Creature* creature) : ScriptedAI(creature) {}

    void JustDied(Unit* killer)
    {
        if(Creature* Sarthis = (Creature*)FindCreature(NPC_SARTHIS, 30, me))
        {
            DoScriptText(SAY_SARTHIS_KILLED_ACOLYTE, Sarthis);
            CAST_AI(npc_sarthisAI,Sarthis->AI())->SetEscortPaused(false);
            CAST_AI(npc_sarthisAI,Sarthis->AI())->ResetTimer = 120000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_sarthis_elementalAI(Creature* creature)
{
    return new npc_sarthis_elementalAI(creature);
}

struct npc_minion_of_sarthisAI : public ScriptedAI
{
    npc_minion_of_sarthisAI(Creature* creature) : ScriptedAI(creature) {me->SetAggroRange(40.0f); me->SetIsDistanceToHomeEvadable(false);}

    uint32 delay;

    void Reset() { delay=5000; }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type != POINT_MOTION_TYPE)
            return;

        uint32 BeamId = RAND(SPELL_BLUE_BEAM, SPELL_GREEN_BEAM, SPELL_RED_BEAM);

        if (Id == 1)
        {
            if(Unit* Fetish = FindCreature(12999, 40, me))
            {
                me->CastSpell(Fetish, BeamId, false);
                me->SetFacingToObject(Fetish);
            }
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();
        }
    }

    void UpdateAI(const uint32 diff)
    {

    }
};

CreatureAI* GetAI_npc_minion_of_sarthisAI(Creature* creature)
{
    return new npc_minion_of_sarthisAI(creature);
}

bool GossipHello_npc_sarthis(Player* player, Creature *creature )
{
    player->PlayerTalkClass->ClearMenus();
    if( player->GetQuestStatus(QUEST_THE_SOUL_CANNON_OF_RETHHEDRON) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SARTHIS_SELECT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_sarthis(Player* player, Creature* creature, uint32 sender, uint32 action )
{
    if(action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        DoScriptText(SAY_SARTHIS_INTRO, creature, player);
        CAST_AI(npc_sarthisAI,creature->AI())->Start(false, false, player->GetGUID());
        CAST_AI(npc_sarthisAI,creature->AI())->SetMaxPlayerDistance(200.0f);
    }
    return true;
}

/*#####
# npc_razorthorn_ravager
#####*/

enum RazorthornRavager
{
    SPELL_RAVAGE                        = 38439,
    SPELL_REND                          = 13443,
    SPELL_RAVAGER_TAUNT                 = 46276,
    SPELL_SUMMON_RAZORTHORN_ROOT        = 44941,

    GAMEOBJECT_RAZORTHORN_DIRT_MOUND    = 187073
};

struct npc_razorthorn_ravagerAI : public ScriptedAI
{
    npc_razorthorn_ravagerAI(Creature* creature) : ScriptedAI(creature) { }

    Timer_UnCheked RavageTimer;
    Timer_UnCheked RendTimer;
    Timer_UnCheked RavageTauntTimer;
    Timer_UnCheked DiggingTimer;
    Timer_UnCheked CheckTimer;
    bool digging;
    bool checked;

    std::list<uint64> MoundList;

    void Reset()
    {
        RavageTimer.Reset(12000);
        RendTimer.Reset(urand(3000, 6000));
        RavageTauntTimer.Reset(urand(6000, 10000));
        DiggingTimer.Reset(5000);
        CheckTimer.Reset(2000);
        digging = false;
        checked = false;
    }

    void OnCharmed(bool apply)
    {
        if(apply)
            MoundList.clear();
    }

    void EnterCombat(Unit*)
    {
        me->NeedChangeAI = true;
        me->IsAIEnabled = false;
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(me->isCharmed())
        {
            if(!me->HasReactState(REACT_AGGRESSIVE))
                return;
            else
            {
                if (me->GetVictim())
                    return;
                if (me->canStartAttack(who))
                {
                    EnterCombat(who);
                    who->CombatStart(me);
                }
            }
        }
        ScriptedAI::MoveInLineOfSight(who);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id == 1)
        {
            me->GetMotionMaster()->MoveIdle();
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_ATTACKUNARMED);
            digging = true;
            DiggingTimer = 5000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(me->isCharmed() && me->GetCharmer())
        {
            if(me->HasReactState(REACT_DEFENSIVE) && me->GetCharmer()->IsInCombat())
            {
                me->NeedChangeAI = true;
                me->IsAIEnabled = false;
            }

            // remove charm when not in Razorthorn Rise
            if(CheckTimer.Expired(diff))
            {
                Unit* charmer = me->GetCharmer();

                if(charmer->GetAreaId() != 4078)
                    me->RemoveCharmedOrPossessedBy(charmer);

                CheckTimer = 2000;
            }
        }

        if(me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE && !checked)
        {
            if(GameObject* go = FindGameObject(GAMEOBJECT_RAZORTHORN_DIRT_MOUND, 15, me))
            {
                if(!MoundList.empty())
                {
                    for(std::list<uint64>::iterator itr = MoundList.begin(); itr != MoundList.end(); ++itr)
                    {
                        if((*itr) == go->GetGUID())
                        {
                            WorldPacket data(SMSG_CAST_FAILED, (4+1+1));
                            data << uint32(SPELL_SUMMON_RAZORTHORN_ROOT);
                            data << uint8(SPELL_FAILED_REQUIRES_SPELL_FOCUS);
                            data << uint8(1);
                            data << uint32(1482);
                            me->GetCharmerOrOwnerPlayerOrPlayerItself()->GetSession()->SendPacket(&data);
                            me->GetMotionMaster()->MoveFollow(me->GetCharmer(),PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
                            checked = false;
                            return;
                        }
                    }
                    checked = true;
                }
                else
                    checked = true;
            }
        }

        if(digging && DiggingTimer.Expired(diff))
        {
            digging = false;
            DoCast((Unit*)NULL, SPELL_SUMMON_RAZORTHORN_ROOT);
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE,EMOTE_ONESHOT_NONE);
            me->GetMotionMaster()->MoveFollow(me->GetCharmer(),PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);

            if(GameObject* go = FindGameObject(GAMEOBJECT_RAZORTHORN_DIRT_MOUND, 15, me))
            {
                if(me->GetCharmer() && me->GetCharmer()->GetTypeId() == TYPEID_PLAYER)
                {
                        go->SetLootState(GO_JUST_DEACTIVATED);
                    MoundList.push_front(go->GetGUID());
                    go->DestroyForPlayer(((Player*)me->GetCharmer()));
                    checked = false;
                }
            }
        }

        if(!UpdateVictim())
            return;

        if(RavageTimer.Expired(diff))
        {
            if(urand(0,3))
                AddSpellToCast(me->GetVictim(), SPELL_RAVAGE);
            RavageTimer = urand(17000, 20000);
        }

        if(RendTimer.Expired(diff))
        {
            if(urand(0,3))
                AddSpellToCast(me->GetVictim(), SPELL_REND);
            RendTimer = urand(15000, 20000);
        }

        if(RavageTauntTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_RAVAGER_TAUNT);
            RavageTauntTimer = urand(10000, 35000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_razorthorn_ravagerAI(Creature *creature)
{
    return new npc_razorthorn_ravagerAI(creature);
}

enum
{
    NPC_EVENT_TRIGGER_A = 21451,
    NPC_PORTAL_TRIGGER = 21433,
    NPC_VISUAL_ENERGY = 21429,
    NPC_GUARDIAN_PORTAL = 21463,
    NPC_GUARDIAN = 22285,
    NPC_HARBRINGER = 21638,
    NPC_VENGEFUL_DRAENEI = 21636,
    NPC_DEFENDER_TRIGGER = 21431,

    SPELL_SUMMON_VISUAL = 35510,
    SPELL_BALL_VISUAL = 46242,
    SPELL_BEAM_VISUAL = 36878,
    QUEST_VENGEFUL_HARBRINGER = 10842,
    GOB_ASCENDAND = 184830,
};

struct quest_the_vengeful_harbringerAI : public ScriptedAI
{
    quest_the_vengeful_harbringerAI(Creature* creature) : ScriptedAI(creature){}

    Timer_UnCheked visual_1_timer;
    Timer_UnCheked trash_timer;
    Timer_UnCheked checktimer;

    int trash_counter;

    uint64 owner;

    bool event_done;
    bool IS_ASCENDANT_ALREADY_SUMMONED;
    bool corpse_moved;

    void SpawnVengefulHarbringer()
    {
        Creature * Portal_Trigger  = GetClosestCreatureWithEntry(me, NPC_PORTAL_TRIGGER, 90.0f);
        Creature * Dranei_Guardian = GetClosestCreatureWithEntry(me, NPC_GUARDIAN,90.0f);

        if (!Portal_Trigger || !Dranei_Guardian)
            return;

        Portal_Trigger->CastSpell(me, SPELL_SUMMON_VISUAL, true);

        if (Creature * Boss = Portal_Trigger->SummonCreature(NPC_HARBRINGER, Portal_Trigger->GetPositionX(), Portal_Trigger->GetPositionY(), Portal_Trigger->GetPositionZ(), Portal_Trigger->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 180000))
        {
            Boss->SetAggroRange(100.0f);
            Boss->CombatStart(Dranei_Guardian);
        }
    }

    void SpawnVengefulDraenei()
    {
        Creature * Portal_Trigger  = GetClosestCreatureWithEntry(me, NPC_PORTAL_TRIGGER, 90.0f);
        Creature * Dranei_Guardian = GetClosestCreatureWithEntry(me, NPC_GUARDIAN,90.0f);

        if (!Portal_Trigger || !Dranei_Guardian)
            return;

        Portal_Trigger->CastSpell(me, SPELL_SUMMON_VISUAL, true);

        for (uint8 i = 0; i < 3; i++)
        {
            if (Creature* Trash = Portal_Trigger->SummonCreature(NPC_VENGEFUL_DRAENEI, Portal_Trigger->GetPositionX(), Portal_Trigger->GetPositionY(), Portal_Trigger->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000))
            {
                Trash->SetAggroRange(100.0f);
                Trash->CombatStart(Dranei_Guardian);
            }
        }
    }

    void IsSummonedBy(Unit *summoner)
    {
        owner = summoner->GetGUID();
    }

    void Reset()
    {
        event_done=false;
        trash_counter=0;
        trash_timer.Reset(15000);
        visual_1_timer.Reset(5000);
        IS_ASCENDANT_ALREADY_SUMMONED=false;
        corpse_moved=false;

        owner = 0;
        checktimer.Reset(2000);

        Creature *visual_energy  = GetClosestCreatureWithEntry(me, NPC_VISUAL_ENERGY, 60.0f);
        Creature *portal_trigger = GetClosestCreatureWithEntry(me, NPC_GUARDIAN_PORTAL, 60.0f);

        if (!visual_energy || !portal_trigger)
            return;

        visual_energy->CastSpell(visual_energy, SPELL_BALL_VISUAL, true);
        visual_energy->MonsterMoveWithSpeed(portal_trigger->GetPositionX(),portal_trigger->GetPositionY(),portal_trigger->GetPositionZ(),1500,true);

        std::list<Creature*> beam_visual_triggers = FindAllCreaturesWithEntry(NPC_EVENT_TRIGGER_A, 60.0f);
        for (std::list<Creature*>::iterator itr = beam_visual_triggers.begin(); itr != beam_visual_triggers.end(); ++itr)
            (*itr)->CastSpell(visual_energy, SPELL_BEAM_VISUAL, false);
    }

    void StopEventAndCleanUp()
    {
        if (Creature * visual_energy = GetClosestCreatureWithEntry(me, NPC_VISUAL_ENERGY, 60.0f))
            visual_energy->RemoveAurasDueToSpell(46242);

        std::list<Unit*> DeadList = FindAllDeadInRange(90);
        // Remove Guardians and Harbringer corpses
        for(std::list<Unit*>::iterator i = DeadList.begin(); i != DeadList.end(); ++i)
        {
            if ((*i)->GetTypeId()==TYPEID_UNIT && ((*i)->GetEntry()==NPC_GUARDIAN || (*i)->GetEntry()== NPC_HARBRINGER))
                ((Creature*)*i)->RemoveCorpse();
        }

        me->DisappearAndDie();
    }

    void UpdateAI(const uint32 diff)
    {
        if (visual_1_timer.Expired(diff))
        {
            if (Creature * visual_energy = GetClosestCreatureWithEntry(me, NPC_VISUAL_ENERGY, 60.0f))
            {
                visual_energy->CastSpell(visual_energy, SPELL_SUMMON_VISUAL, true);
                visual_1_timer=250000;

                Creature * Dranei_Guardian = me->SummonCreature(NPC_GUARDIAN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
                Creature * Defender_Trigger = GetClosestCreatureWithEntry(me, NPC_DEFENDER_TRIGGER, 40.0f);

                if (!Defender_Trigger || !Dranei_Guardian)
                    return;

                Dranei_Guardian->MonsterMoveWithSpeed(Defender_Trigger->GetPositionX(),Defender_Trigger->GetPositionY(),Defender_Trigger->GetPositionZ(),6000,true); //we ned this to visual movement with delay
            }
        }

        if (trash_timer.Expired(diff))
        {
            trash_timer=18000;

            if (trash_counter<=4)
            {
                if (trash_counter<4)
                    SpawnVengefulDraenei();
                else
                    SpawnVengefulHarbringer();

                trash_counter++;
            }
        }

        if (checktimer.Expired(diff))
        {
            Unit * player = me->GetUnit(owner);
            if (!player)
                return;

            std::list<Unit*> DeadList = FindAllDeadInRange(90);
            bool IS_DEFENDER_DEAD=false, IS_BOSS_DEAD=false;
            for(std::list<Unit*>::iterator i = DeadList.begin(); i != DeadList.end(); ++i)
            {
                if ((*i)->GetEntry() == NPC_GUARDIAN) IS_DEFENDER_DEAD = true;
                if ((*i)->GetEntry() == NPC_HARBRINGER) IS_BOSS_DEAD = true;
            }

            if (trash_counter>=4)
            {
                if (player->GetTypeId() == TYPEID_PLAYER)
                {
                    if(Group* pGroup = ((Player*)player)->GetGroup() )
                    {
                        for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                        {
                            Player *pGroupie = itr->getSource();
                            if(pGroupie && pGroupie->GetQuestStatus(QUEST_VENGEFUL_HARBRINGER) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IS_DEFENDER_DEAD)
                                {
                                    pGroupie->FailQuest(QUEST_VENGEFUL_HARBRINGER);
                                    StopEventAndCleanUp();
                                }
                                else
                                    if (IS_BOSS_DEAD)
                                        pGroupie->CompleteQuest(QUEST_VENGEFUL_HARBRINGER);
                            }
                        }
                    }
                    else
                    {
                        if (((Player*)player)->GetQuestStatus(QUEST_VENGEFUL_HARBRINGER) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (IS_DEFENDER_DEAD)
                            {
                                ((Player*)player)->FailQuest(QUEST_VENGEFUL_HARBRINGER);
                               StopEventAndCleanUp();
                            }
                            else
                                if (IS_BOSS_DEAD)
                                   ((Player*)player)->CompleteQuest(QUEST_VENGEFUL_HARBRINGER);
                        }
                    }
                }
            }

            if (IS_BOSS_DEAD && !IS_DEFENDER_DEAD)
            {
                //summon Draenei Ascendant
                Creature *portal_trigger = GetClosestCreatureWithEntry(me, NPC_GUARDIAN_PORTAL, 90.0f);

                if (!portal_trigger)
                    return;

                if (!IS_ASCENDANT_ALREADY_SUMMONED)
                    me->SummonGameObject(GOB_ASCENDAND, portal_trigger->GetPositionX(), portal_trigger->GetPositionY(), portal_trigger->GetPositionZ(),  0, 0, 0, 0.70959, 0.704615, 240000);

                IS_ASCENDANT_ALREADY_SUMMONED=true;

                if (Creature * Dranei_Guardian = GetClosestCreatureWithEntry(me, NPC_GUARDIAN,90.0f))
                    Dranei_Guardian->DisappearAndDie();
            }
            checktimer = 2000;
        }
    }
};

CreatureAI* GetAI_quest_the_vengeful_harbringer(Creature *creature)
{
    return new quest_the_vengeful_harbringerAI(creature);
}


/*#####
## npc_akuno
#####*/

#define SPELL_AKUNO_CHAIN_LIGHTNING 39945
#define QUEST_ESCAPING_THE_TOMB     10887

#define CABAL_SPAWN_1_1     -2890.478271, 5411.563477, -20.220009, 0.056583
#define CABAL_SPAWN_1_2     -2892.569336, 5415.295410, -18.824707, 6.280863
#define CABAL_SPAWN_2_1     -2891.779053, 5385.837402, -9.306866, 1.706707
#define CABAL_SPAWN_2_2     -2899.023926, 5385.787109, -9.301114, 1.577116

#define CABAL_ID_1  21661
#define CABAL_ID_2  21662
#define CABAL_ID_3  21660

struct npc_akunoAI : public npc_escortAI
{
    npc_akunoAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    Timer chainLightningTimer;

    void Reset()
    {
        chainLightningTimer.Reset(urand(6000, 12000));
        me->SetReactState(REACT_DEFENSIVE);
        me->SetAggroRange(0);
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 7:
                me->SummonCreature(RAND(CABAL_ID_1, CABAL_ID_2, CABAL_ID_3), CABAL_SPAWN_1_1, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                me->SummonCreature(RAND(CABAL_ID_1, CABAL_ID_2, CABAL_ID_3), CABAL_SPAWN_1_2, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);

                break;
            case 10:
                me->SummonCreature(RAND(CABAL_ID_1, CABAL_ID_2, CABAL_ID_3), CABAL_SPAWN_2_1, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                me->SummonCreature(RAND(CABAL_ID_1, CABAL_ID_2, CABAL_ID_3), CABAL_SPAWN_2_2, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);

                break;
            case 13:
                if (Player* player = GetPlayerForEscort())
                    player->GroupEventHappens(QUEST_ESCAPING_THE_TOMB, me);

                SetRun();
                break;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->SetWalk(false);
        summoned->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
        summoned->AI()->AttackStart(me);
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        if (!me->GetVictim())
            return;

        if (chainLightningTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_AKUNO_CHAIN_LIGHTNING);
            chainLightningTimer = urand(6000, 12000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_akuno(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_ESCAPING_THE_TOMB)
    {
        npc_akunoAI* pEscortAI = dynamic_cast<npc_akunoAI*>(creature->AI());
        if (pEscortAI)
        {
            creature->SetStandState(UNIT_STAND_STATE_STAND);
            creature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE); // faction escortee

            pEscortAI->Start(true, false, player->GetGUID(), quest);
        }
    }

    return true;
}

CreatureAI* GetAI_npc_akuno(Creature* creature)
{
    return new npc_akunoAI(creature);
}

/*######
# npc_empoor
######*/

enum
{
    FACTION_HOSTILE_A          = 90,
    FACTION_FRIENDLY_A         = 35,

    QUEST_BAMN                 = 9978,
    NPC_MY_BODYGUARD           = 18483,

    SPELL_SHIELD               = 12550,
    SPELL_FROST_SHOCK          = 12548
};

struct npc_empoorAI : public ScriptedAI
{
    npc_empoorAI(Creature* creature) : ScriptedAI(creature) {}

    Timer ShockTimer;

    void Reset()
    {
        ShockTimer.Reset(5000);
        me->setFaction(FACTION_FRIENDLY_A);
    }

    void EnterCombat(Unit *who)
    {
        DoCast(me ,SPELL_SHIELD);

        if (Creature* Guard = GetClosestCreatureWithEntry(me, NPC_MY_BODYGUARD, 10.5f))
        {
            Guard->setFaction(FACTION_HOSTILE_A);
            Guard->AI()->AttackStart(who);
        }
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if( done_by->GetTypeId() == TYPEID_PLAYER )
        {
            if( (me->GetHealth()-damage)*100 / me->GetMaxHealth() < 20 )
            {
                ((Player*)done_by)->AreaExploredOrEventHappens(QUEST_BAMN);
                damage = 0;

                if (Creature* Guard = GetClosestCreatureWithEntry(me, NPC_MY_BODYGUARD, 10.5f))
                {
                    Guard->setFaction(FACTION_FRIENDLY_A);
                    Guard->AI()->EnterEvadeMode();
                }

                EnterEvadeMode();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ShockTimer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_FROST_SHOCK);
            ShockTimer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_empoor(Creature *creature)
{
    return new npc_empoorAI (creature);
}

bool GossipHello_npc_empoor(Player *player, Creature *creature)
{
    player->PlayerTalkClass->ClearMenus();
    if( player->GetQuestStatus(QUEST_BAMN) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16071), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    }
    else
    {
        if( creature->isQuestGiver())
            player->PrepareQuestMenu( creature->GetGUID());
        player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_empoor(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        creature->setFaction(FACTION_HOSTILE_A);
        ((npc_empoorAI*)creature->AI())->AttackStart(player);
    }
    return true;
}

/*########
## npc_captive_child
########*/

enum
{
    QUEST_FRIENDS               = 10852,
    NPC_CHILD_CAPITIVE          = 22314,

    SAY_CHILD1                  = -1900174,
    SAY_CHILD2                  = -1900175,
    SAY_CHILD3                  = -1900176,
    SAY_CHILD4                  = -1900177,
    SAY_CHILD5                  = -1900178
};

struct WP
{
    float x, y, z;
};

static WP W[]=
{
    {-2464.39f, 5398.43f, 2.12f},
    {-2485.65f, 5382.77f, 0.11f},
    {-2542.65f, 5482.23f, 8.25f},
    {-2520.57f, 5447.79f, 0.12f},
    {-2582.34f, 5425.01f, 26.85f},
    {-2561.75f, 5439.09f, 27.16f},
    {-2528.63f, 5387.37f, 27.65f},
    {-2550.72f, 5404.97f, 20.00f}
};

struct npc_captive_childAI : public npc_escortAI
{
    npc_captive_childAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    void Reset() {}

    //if you add more children .add GUID here.
    uint32 WaypointID()
    {
        switch (me->GetGUIDLow())
        {
            case 78491:
                return 1;
                break;
            case 78494:
                return 2;
                break;
            case 78493:
                return 3;
                break;
            case 78492:
                return 3;
                break;
            case 78490:
                return 4;
                break;
            case 78488:
                return 4;
                break;
            case 78489:
                return 4;
                break;
            default:
                return 2;
                break;
        }
    }

    void StartRun(Player* player)
    {
        switch (WaypointID())
        {
            case 1:
                AddWaypoint(0, W[0].x+(rand()%4), W[0].y-(rand()%4), W[0].z, 3000);
                AddWaypoint(1, W[1].x, W[1].y, W[1].z);
                Start(false, true, player->GetGUID());
                break;
            case 2:
                AddWaypoint(0, W[2].x+(rand()%4), W[2].y-(rand()%4), W[2].z, 3000);
                AddWaypoint(1, W[3].x, W[3].y, W[3].z);
                Start(false, true, player->GetGUID());
                break;
            case 3:
                AddWaypoint(0, W[4].x+(rand()%4), W[4].y-(rand()%4), W[4].z, 3000);
                AddWaypoint(1, W[5].x, W[5].y, W[5].z);
                Start(false, true, player->GetGUID());
                break;
            case 4:
                AddWaypoint(0, W[6].x+(rand()%4), W[6].y-(rand()%4), W[6].z, 3000);
                AddWaypoint(1, W[7].x, W[7].y, W[7].z);
                Start(false, true, player->GetGUID());
                break;
        }
        return;
    }

    void WaypointReached(uint32 i)
    {
        switch (i)
        {
            case 0:
                switch (urand(0,4))
                {
                    case 0:
                        DoScriptText(SAY_CHILD1, me);
                        break;
                    case 1:
                        DoScriptText(SAY_CHILD2, me);
                        break;
                    case 2:
                        DoScriptText(SAY_CHILD3, me);
                        break;
                    case 3:
                        DoScriptText(SAY_CHILD4, me);
                        break;
                    case 4:
                        DoScriptText(SAY_CHILD5, me);
                        break;
                }
                break;
            case 1:
                me->ForcedDespawn();
                break;
        }
    }
};

CreatureAI* GetAI_npc_captive_child(Creature* creature)
{
    return new npc_captive_childAI(creature);
}

bool go_veil_skith_cage(Player* player, GameObject* go)
{
    std::list<Creature*> ChildrenList;
    ChildrenList.clear();

    if(player->GetQuestStatus(QUEST_FRIENDS) == QUEST_STATUS_INCOMPLETE)
    {
        Hellground::AllCreaturesOfEntryInRange check(player, NPC_CHILD_CAPITIVE, 5.0f);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(ChildrenList, check);
        Cell::VisitGridObjects(player, searcher, 5.0f);

        if(!ChildrenList.empty())
        {
            for (std::list<Creature*>::iterator itr = ChildrenList.begin(); itr != ChildrenList.end(); ++itr)
            {
                if ((*itr)->isAlive())
                {
                    if (npc_captive_childAI* escortAI = CAST_AI(npc_captive_childAI, (*itr)->AI()))
                    {
                        player->KilledMonster(NPC_CHILD_CAPITIVE, (*itr)->GetGUID());
                        escortAI->StartRun(player);
                    }
                }
            }
        }
        return false;
    }
    return true;
}

/*######
## npc_skywing
######*/

enum
{
    SAY_SKYWING_START            = -1900179,
    SAY_SKYWING_TREE_DOWN        = -1900180,
    SAY_SKYWING_TREE_UP          = -1900181,
    SAY_SKYWING_JUMP             = -1900182,
    SAY_SKYWING_SUMMON           = -1900183,
    SAY_SKYWING_END              = -1900184,

    SPELL_TRANSFORM              = 41301,

    NPC_LUANGA_THE_IMPRISONER    = 18533,

    QUEST_SKYWING                = 10898
};

static const float LuangaSpawnCoords[3] = { -3507.203f, 4084.619f, 92.947f};

struct npc_skywingAI : public npc_escortAI
{
    npc_skywingAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    void Reset() {}

    void WaypointReached(uint32 i)
    {
        switch (i)
        {
            case 6:
                DoScriptText(SAY_SKYWING_TREE_DOWN , me);
                break;
            case 36:
                DoScriptText(SAY_SKYWING_TREE_UP, me);
                break;
            case 60:
                DoScriptText(SAY_SKYWING_JUMP, me);
                me->SetLevitate(true);
                break;
            case 61:
                me->SetLevitate(false);
                break;
            case 80:
                DoScriptText(SAY_SKYWING_SUMMON, me);
                me->SummonCreature(NPC_LUANGA_THE_IMPRISONER, LuangaSpawnCoords[0], LuangaSpawnCoords[1], LuangaSpawnCoords[2], 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                break;
            case 82:
                DoCast(me, SPELL_TRANSFORM);
                DoScriptText(SAY_SKYWING_END, me);

                if (Player* player = GetPlayerForEscort())
                    player->GroupEventHappens(QUEST_SKYWING, me);
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(me);
    }
};

bool QuestAccept_npc_skywing(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_SKYWING)
    {
        if (npc_skywingAI* EscortAI = dynamic_cast<npc_skywingAI*>(creature->AI()))
        {
            creature->setFaction(FACTION_ESCORT_N_NEUTRAL_PASSIVE);
            DoScriptText(SAY_SKYWING_START, creature);

            EscortAI->Start(true, false, player->GetGUID(), quest);
        }
    }
    return true;
}

CreatureAI* GetAI_npc_skywing(Creature* creature)
{
    return new npc_skywingAI(creature);
}

/*######
## npc_cenarion_sparrowhawk
######*/

#define GAMEOBJECT_RAVEN_STONE  185541
#define EMOTE_FOUNDED           -1200151
#define EMOTE_CANT_FIND         -1200152

struct npc_cenarion_sparrowhawkAI : public ScriptedAI
{
    npc_cenarion_sparrowhawkAI(Creature *creature) : ScriptedAI(creature) {}

    uint32 DespawnTimer;
    uint32 CheckTimer;
    bool checked;
    void Reset()
    {
        DespawnTimer = 45000;
        CheckTimer = 3000;
        checked = false;
        me->SetSpeed(MOVE_WALK, 0.9);
        me->SetSpeed(MOVE_RUN, 0.9);
        me->SetSpeed(MOVE_FLIGHT, 0.9);
    }
    void UpdateAI(const uint32 diff)
    {
        if(!checked)
        {
            if(CheckTimer < diff)
            {
                std::list<GameObject*> RavenStoneList;
                Hellground::AllGameObjectsWithEntryInGrid go_check(GAMEOBJECT_RAVEN_STONE);
                Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher(RavenStoneList, go_check);
                Cell::VisitGridObjects(me, searcher, 30);
        
                if (!RavenStoneList.empty())
                {
                    for (std::list<GameObject*>::const_iterator i = RavenStoneList.begin(); i != RavenStoneList.end(); ++i)
                    {
                        if (GameObject* go = (*i)->ToGameObject())
                        {
                            if(go->getLootState() == GO_READY)
                            {
                                me->GetMotionMaster()->MovePoint(0, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                                me->MonsterTextEmote(-1200151,0);
                                checked = true;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    me->MonsterTextEmote(-1200152,0);
                    me->ForcedDespawn();
                }
                CheckTimer = 3000;
            }
            else
                CheckTimer -= diff;
        }
        if(DespawnTimer < diff)
        {
            me->ForcedDespawn();
        }
        else
            DespawnTimer -= diff;
    }
};
CreatureAI* GetAI_npc_cenarion_sparrowhawk(Creature *creature)
{
    return new npc_cenarion_sparrowhawkAI (creature);
}

bool ReceiveEmote_Dena_Kennedy(Player*, Creature* cr, uint32 emote)
{
    if (emote == TEXTEMOTE_FLEX)
        cr->TextEmote(-1200153, 0);
    return true;
}

enum who_are_they
{
    QUEST_HORDE     = 10041,
    QUEST_ALLIANCE  = 10040,
    SPELL_DISGUISE  = 32756,
    SPELL_MALE      = 38080,
    SPELL_FEMALE    = 38081
};

bool QuestAccept_npc_who_are_they(Player* player, Creature* creature, const Quest* quest)
{
    if (quest->GetQuestId() == QUEST_HORDE || quest->GetQuestId() == QUEST_ALLIANCE)
    {
        player->Unmount();
        player->CastSpell(player, SPELL_DISGUISE, true);
        player->CastSpell(player, player->GetGender() ? SPELL_FEMALE : SPELL_MALE, true);
    }
    return true;
}

bool QuestComplete_npc_who_are_they(Player* player, Creature* creature, const Quest* quest)
{
    player->RemoveAurasDueToSpell(SPELL_DISGUISE);
    player->RemoveAurasDueToSpell(player->GetGender() ? SPELL_FEMALE : SPELL_MALE);
    return true;
}

struct npc_18707AI : public ScriptedAI
{
    npc_18707AI(Creature* creature) : ScriptedAI(creature) {}

    Timer FeveredDiseaseTimer;
    Timer GutRipTimer;
    Timer GougeTimer;

    void Reset()
    {
        FeveredDiseaseTimer.Reset(8000);
        GutRipTimer.Reset(2000);
        GougeTimer.Reset(5000);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if(eventType == 5)
        {
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            me->GetMotionMaster()->MovePoint(100, invoker->GetPositionX(), invoker->GetPositionY(), invoker->GetPositionZ());
            me->AI()->AttackStart(invoker);
            me->SetReactState(REACT_AGGRESSIVE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
        
        if(FeveredDiseaseTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 32903, false);
            FeveredDiseaseTimer.Reset(urand(10000, 14000));
        }

        if(GutRipTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 32022, false);
            GutRipTimer.Reset(urand(12000, 20000));
        }

        if(GougeTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 13579, false);
            GougeTimer.Reset(urand(20000, 25000));
        }
        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18707(Creature* creature)
{
    return new npc_18707AI(creature);
}

struct npc_19606AI : public ScriptedAI
{
    npc_19606AI(Creature* creature) : ScriptedAI(creature) {}

    Timer Event1Timer;
    Timer Event2Timer;
    Timer RandomTextTimer;
    uint64 PlayerGUID;

    void Reset()
    {
        Event1Timer.Reset(0);
        Event2Timer.Reset(0);
        RandomTextTimer.Reset(1000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(Event1Timer.Expired(diff))
        {
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE,69);
            Event1Timer = 0;
            Event2Timer.Reset(5000);
            RandomTextTimer = 0;
        }

        if(Event2Timer.Expired(diff))
        {
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE,0);
            me->Say(-1200154, LANG_UNIVERSAL, 0);
            me->HandleEmoteCommand(15);
            Event2Timer = 0;
            if(PlayerGUID)
            {
                if(Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID))
                    pPlayer->KilledMonster(19606, 0);
            }
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            RandomTextTimer = 30000;
        }

        if(RandomTextTimer.Expired(diff))
        {
            switch (rand()%2)
            {
                case 0:
                    me->Say(-1200155, LANG_UNIVERSAL, 0);
                    me->HandleEmoteCommand(7);
                    break;
                case 1:
                    me->Say(-1200156, LANG_UNIVERSAL, 0);
                    me->HandleEmoteCommand(6);
                    break;
            }
            RandomTextTimer = 120000;
        }

        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_19606(Creature* creature)
{
    return new npc_19606AI(creature);
}

bool GossipHello_npc_19606(Player *player, Creature *creature )
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(10201) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16072), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_19606(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->Say(-1200157, LANG_UNIVERSAL, 0);
        ((npc_19606AI*)creature->AI())->Event1Timer = 1000;
        ((npc_19606AI*)creature->AI())->PlayerGUID = player->GetGUID();
    }
    return true;
}
// receive ai event
struct npc_18384AI : public ScriptedAI
{
    npc_18384AI(Creature* creature) : ScriptedAI(creature) {}

    Timer EventTimer;
    uint32 EventPhase;
    uint64 PlayerGUID;
    Timer ResetTimer;
    bool EventStarted;

    void Reset()
    {
        PlayerGUID = 0;
        EventPhase = 0;
        EventTimer.Reset(0);
        ResetTimer.Reset(0);
        EventStarted = false;
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if(!EventStarted)
        {
            PlayerGUID = invoker->GetGUID();
            EventPhase = 0;
            EventTimer = 2000;
            ResetTimer = 40000;
            EventStarted = true;
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiType, uint32 uiPointId)
    {
        if(pSummoned->GetEntry() == 19616)
        {
            if(uiPointId == 1)
            {
                pSummoned->HandleEmoteCommand(393);
                EventTimer = 4000;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventStarted && ResetTimer.Expired(diff))
        {
            ResetTimer = 0;
            Reset();
        }

        if(EventTimer.Expired(diff))
        {
            switch (EventPhase)
            {
                case 0:
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                    me->TextEmote(-1200158, 0);
                    me->CastSpell(me, 34317, false);
                    EventPhase++;
                    EventTimer = 3000;
                    break;
                case 1:
                    me->CastSpell(me, 28892, false);
                    EventTimer = 5000;
                    EventPhase++;
                    break;
                case 2:
                    me->Say(-1200159, LANG_UNIVERSAL, 0);
                    EventTimer = 2000;
                    EventPhase++;
                    break;
                case 3:
                    me->Say(-1200160, LANG_UNIVERSAL, 0);
                    EventTimer = 4000;
                    EventPhase++;
                    break;
                case 4:
                    me->Say(-1200161, LANG_UNIVERSAL, 0);
                    EventTimer = 5000;
                    EventPhase++;
                    break;
                case 5:
                    me->InterruptNonMeleeSpells(true);
                    me->CastSpell(me, 34312, false);
                    if(Creature* wolf = me->SummonCreature(19616, -2655.9,  4391.12, 36.1316, 4.062, TEMPSUMMON_MANUAL_DESPAWN, 0))
                        wolf->GetMotionMaster()->MovePoint(1, -2668.96, 4376.09, 35.9556);
                    EventTimer = 0;
                    EventPhase++;
                    break;
                case 6:
                    if(Unit* wolf = FindCreature(19616, 10, me))
                    {
                        ((Creature*)wolf)->HandleEmoteCommand(0);
                        ((Creature*)wolf)->TextEmote(-1200162, 0);
                        ((Creature*)wolf)->TextEmote(-1200163, 0);
                    }
                    EventTimer = 3000;
                    EventPhase++;
                    break;
                case 7:
                    me->Say(-1200164, LANG_UNIVERSAL, 0);
                    me->HandleEmoteCommand(2);
                    EventTimer = 3000;
                    EventPhase++;
                    break;
                case 8:
                    if(Unit* wolf = FindCreature(19616, 10, me))
                        ((Creature*)wolf)->DisappearAndDie();
                    if(PlayerGUID)
                    {
                        if(Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID))
                            pPlayer->KilledMonster(19616, 0);
                    }
                    me->Say(-1200165, LANG_UNIVERSAL, 0);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                    me->CastSpell(me, 34319, 0);
                    PlayerGUID = 0;
                    EventTimer = 0;
                    EventPhase = 0;
                    EventStarted = false;
                    break;
            }
        }

        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_18384(Creature* creature)
{
    return new npc_18384AI(creature);
}

bool GossipHello_npc_19718(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    bool canBuy;
    canBuy = false;

    if (player->GetReputationMgr().GetRank(1011) <= REP_FRIENDLY)
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    else canBuy = true;

    if (canBuy)
    {
        if (_Creature->isVendor())
            player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_19718(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

bool GossipHello_npc_23363(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    bool canBuy;
    canBuy = false;

    if (!player->HasAura(41181))
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    else canBuy = true;

    if (canBuy)
    {
        if (_Creature->isVendor())
            player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_23363(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

bool go_185211(Player* player, GameObject* go)
{
    if(player->GetQuestStatus(10861) == QUEST_STATUS_INCOMPLETE)
    {
        uint32 rand = urand(0, 1);

        switch(rand)
        {
            case 0:
                go->SummonCreature(22337,go->GetPositionX(),go->GetPositionY(),go->GetPositionZ(),go->GetOrientationTo(player),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
                go->SetLootState(GO_JUST_DEACTIVATED);
                break;
            case 1:
                Creature* summon = go->SummonCreature(22339,go->GetPositionX(),go->GetPositionY(),go->GetPositionZ(),go->GetOrientationTo(player),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 5000);
                if(summon)
                {
                    float x, y, z;
                    summon->GetNearPoint(x, y, z, 15.0f);
                    summon->GetMotionMaster()->MovePoint(100, x, y, z+15);
                    player->KilledMonster(22339, 0);
                }
                go->SetLootState(GO_JUST_DEACTIVATED);
                break;
        }
    }
    return true;
}

bool QuestAccept_npc_18714_18715(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 10040 || quest->GetQuestId() == 10041)
        player->CastSpell(player, player->GetGender() == GENDER_MALE ? 38080 : 38081, false);

    return true;
}

bool GossipHello_npc_18714_18715(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(10040) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10041) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16073), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_18714_18715(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 32756, false);
        player->CastSpell(player, player->GetGender() == GENDER_MALE ? 38080 : 38081, false);
    }
    return true;
}

struct npc_18719AI : public ScriptedAI
{
    npc_18719AI(Creature *creature) : ScriptedAI(creature) {}

    Timer RandomSayTimer;
    Timer ShadowBoltTimer;

    void Reset()
    {
        RandomSayTimer.Reset(1000);
        ShadowBoltTimer.Reset(1000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (RandomSayTimer.Expired(diff))
            {
                me->HandleEmote(1);
                RandomSayTimer = 5000;
            }
            return;
        }

        if (ShadowBoltTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 9613, false);
            ShadowBoltTimer = 3000;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_18719(Creature *creature)
{
    return new npc_18719AI(creature);
}

bool GossipHello_npc_18719(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if ((player->GetQuestStatus(10040) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10040, 18719) < 1) || (player->GetQuestStatus(10041) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10041, 18719) < 1))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16074), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(9505, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_18719(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->KilledMonster(18719, creature->GetGUID());
        player->SEND_GOSSIP_MENU(9506, creature->GetGUID());
    }
    return true;
}

bool GossipHello_npc_18717(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if ((player->GetQuestStatus(10040) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10040, 18717) < 1) || (player->GetQuestStatus(10041) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10041, 18717) < 1))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16075), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(9497, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_18717(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->KilledMonster(18717, creature->GetGUID());
        player->SEND_GOSSIP_MENU(9498, creature->GetGUID());
    }
    return true;
}

bool GossipHello_npc_18716(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if ((player->GetQuestStatus(10040) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10040, 18716) < 1) || (player->GetQuestStatus(10041) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10041, 18716) < 1))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16076), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(9502, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_18716(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->KilledMonster(18716, creature->GetGUID());
        player->SEND_GOSSIP_MENU(9503, creature->GetGUID());
    }
    return true;
}

struct npc_19847AI : public ScriptedAI
{
    npc_19847AI(Creature *creature) : ScriptedAI(creature), Summons(me) {}

    SummonList Summons;
    Timer CrippleTimer;
    Timer DominateMindTimer;
    Timer InfernoTimer;
    Timer RandomSayTimer;

    void Reset()
    {
        CrippleTimer.Reset(3000);
        DominateMindTimer.Reset(9000);
        InfernoTimer.Reset(13000);
        RandomSayTimer.Reset(5000);
        Summons.DespawnAll();
        me->SummonCreature(21870, -3586.91, 4960.24, -14.0263, 6.27009, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3592.4, 4960.32, -18.2386, 6.27009, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3598.61, 4960.4, -22.7091, 6.27009, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3610.52, 4960.55, -22.7091, 6.27009, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3586.9, 4952.79, -13.8297, 0.01046, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3592.62, 4952.73, -18.226, 0.01046, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3598.61, 4952.66, -22.7088, 0.01046, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3610.84, 4952.54, -22.678, 0.01046, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3587.27, 4944.94, -13.9305, 0.01833, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3593.16, 4944.83, -18.4467, 0.01833, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3598.93, 4944.73, -22.709, 0.01833, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3610.91, 4944.5, -22.6315, 6.28189, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3586.97, 4936.27, -13.8146, 0.01833, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3592.75, 4936.16, -18.2567, 0.01833, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3598.65, 4936.05, -22.7078, 0.01833, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3610.88, 4935.52, -22.4214, 6.28189, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3586.7, 4927.01, -13.8376, 0.00657, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3592.56, 4926.97, -18.337, 0.00657, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3599.38, 4926.93, -22.6147, 0.00657, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3610.76, 4926.85, -21.9033, 0.00264, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3565.14, 4938.02, -9.03127, 1.49099, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3568.04, 4939.09, -9.00204, 0.88230, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3570.31, 4943.01, -9.00207, 6.27799, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3569.09, 4947.12, -9.00207, 5.45332, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3561.54, 4939.43, -9.03505, 2.30388, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3560.04, 4942.64, -9.00438, 3.02251, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3561.53, 4947.27, -9.03883, 3.92572, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        me->SummonCreature(21870, -3565.48, 4949.34, -9.03578, 4.70718, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
    }

    void JustDied(Unit* killer)
    {
        Summons.DespawnAll();
    }

    void JustSummoned(Creature* sum)
    {
        Summons.Summon(sum);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (RandomSayTimer.Expired(diff))
            {
                me->HandleEmote(15);
                me->MonsterTextEmote(-1200166, LANG_UNIVERSAL, 0);
                switch (urand(0, 7))
                {
                case 0:
                    me->Say(-1200167, LANG_DEMONIC, 0);
                    break;
                case 1:
                    me->Say(-1200168, LANG_DEMONIC, 0);
                    break;
                case 2:
                    me->Say(-1200169, LANG_DEMONIC, 0);
                    break;
                case 3:
                    me->Say(-1200170, LANG_DEMONIC, 0);
                    break;
                case 4:
                    me->Say(-1200171, LANG_DEMONIC, 0);
                    break;
                case 5:
                    me->Say(-1200172, LANG_DEMONIC, 0);
                    break;
                case 6:
                    me->Say(-1200173, LANG_DEMONIC, 0);
                    break;
                case 7:
                    me->Say(-1200174, LANG_DEMONIC, 0);
                    break;
                default: break;
                }
                for (SummonList::const_iterator i = Summons.begin(); i != Summons.end(); i++)
                {
                    if (Creature* sum = me->GetCreature(*i))
                        sum->HandleEmote(18);
                }
                RandomSayTimer = 7000;
            }
            return;
        }

        if (CrippleTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 11443, false);
            CrippleTimer = 13000;
        }

        if (DominateMindTimer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1))
                me->CastSpell(target, 7645, false);
            DominateMindTimer = 17000;
        }

        if (InfernoTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 38939, false);
            InfernoTimer = 18000;
        }

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_19847(Creature *creature)
{
    return new npc_19847AI(creature);
}

struct npc_21285AI : public ScriptedAI
{
    npc_21285AI(Creature *creature) : ScriptedAI(creature) {}

    Timer RessurectionTimer;
    Timer ResetTimer;
    uint64 targetGUID;
    Timer MindBlastTimer;
    Timer MindFlayTimer;
    bool Heal;

    void Reset()
    {
        RessurectionTimer.Reset(urand(10000, 30000));
        ResetTimer.Reset(0);
        targetGUID = 0;
        MindBlastTimer.Reset(1);
        MindFlayTimer.Reset(6000);
        Heal = false;
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == 100)
            {
                if (targetGUID)
                {
                    if (Unit* target = me->GetUnit(targetGUID))
                        me->CastSpell(target, 37755, false);
                }
                targetGUID = 0;
                ResetTimer = 5000;
            }
        }
    }
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (ResetTimer.Expired(diff))
            {
                ResetTimer = 0;
                Reset();
            }

            if (RessurectionTimer.Expired(diff))
            {
                std::list<Creature*> vindicator = FindAllCreaturesWithEntry(21859, 25);
                std::list<Creature*> warrior = FindAllCreaturesWithEntry(21846, 25);
                std::vector<Creature*> resTarget;
                if (!vindicator.empty())
                {
                    for (std::list<Creature*>::iterator i = vindicator.begin(); i != vindicator.end(); i++)
                    {
                        if ((*i)->isAlive())
                            resTarget.push_back(*i);
                    }
                }
                if (!warrior.empty())
                {
                    for (std::list<Creature*>::iterator itr = warrior.begin(); itr != warrior.end(); itr++)
                    {
                        if ((*itr)->isAlive())
                            resTarget.push_back(*itr);
                    }
                }
                if (!resTarget.empty())
                    std::random_shuffle(resTarget.begin(), resTarget.end());
                if (!resTarget.empty())
                {
                    targetGUID = resTarget.front()->GetGUID();
                    me->GetMotionMaster()->MovePoint(100, resTarget.front()->GetPositionX(), resTarget.front()->GetPositionY(), resTarget.front()->GetPositionZ());
                }
                RessurectionTimer = 30000;
            }
            return;
        }

        if (MindBlastTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 31516, false);
            MindBlastTimer = 2000;
        }

        if (MindFlayTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 16568, false);
            MindFlayTimer = 7000;
        }

        if (!Heal)
        {
            if (me->GetHealthPercent() <= 40)
            {
                me->CastSpell(me, 11642, false);
                Heal = true;
            }
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_21285(Creature *creature)
{
    return new npc_21285AI(creature);
}

struct npc_21869AI : public ScriptedAI
{
    npc_21869AI(Creature *creature) : ScriptedAI(creature) {}

    Timer EventTimer;
    uint32 Phase;
    uint32 PathId;
    bool AlreadyUsed;
    bool PathDone;

    void Reset()
    {
        if (Unit* wpOne = GetClosestCreatureWithEntry(me, 21443, 100, true))
        {
            me->setActive(true);
            EventTimer.Reset(1);
            Phase = 0;
            PathId = 0;
            AlreadyUsed = false;
            PathDone = false;
        }
        else
        {
            EventTimer.Reset(0);
            Phase = 0;
            PathId = 0;
            AlreadyUsed = false;
            PathDone = false;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 misc)
    {
        if (!AlreadyUsed && PathDone && eventType == AI_EVENT_CUSTOM_EVENTAI_B)
        {
            EventTimer = 3000;
            Phase = 2;
            switch (sender->GetDBTableGUIDLow())
            {
            case 2382122:
                PathId = RAND(2186901, 2186902, 2186903, 2186904);
                break;
            case 2382123:
                PathId = RAND(2186905, 2186906, 2186907, 2186908);
                break;
            }
            AlreadyUsed = true;
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == 100)
            {
                if (Unit* wpTwo = GetClosestCreatureWithEntry(me, 41005, 100, true))
                    me->GetMotionMaster()->MovePoint(101, wpTwo->GetPositionX(), wpTwo->GetPositionY(), wpTwo->GetPositionZ(), true);
            }

            if (id == 101)
            {
                if (Unit* wpThree = GetClosestCreatureWithEntry(me, 41006, 100, true))
                    me->GetMotionMaster()->MovePoint(102, wpThree->GetPositionX(), wpThree->GetPositionY(), wpThree->GetPositionZ(), true);
            }

            if (id == 102)
            {
                std::list<Creature*> wpFour = FindAllCreaturesWithEntry(41007, 75);
                std::vector<Creature*> wpFourV;
                if (!wpFour.empty())
                {
                    for (std::list<Creature*>::iterator i = wpFour.begin(); i != wpFour.end(); i++)
                        wpFourV.push_back(*i);
                }
                if (!wpFourV.empty())
                    std::random_shuffle(wpFourV.begin(), wpFourV.end());
                if (!wpFourV.empty())
                    me->GetMotionMaster()->MovePoint(103, wpFourV.front()->GetPositionX(), wpFourV.front()->GetPositionY(), wpFourV.front()->GetPositionZ(), true);
            }

            if (id == 103)
            {
                PathDone = true;
                Phase = 1;
                EventTimer = 1000;
            }
        }

        if (type == WAYPOINT_MOTION_TYPE)
        {
            switch (PathId)
            {
            case 2186901:
                if (id == 40)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            case 2186902:
                if (id == 59)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            case 2186903:
                if (id == 44)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            case 2186904:
                if (id == 59)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            case 2186905:
                if (id == 29)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            case 2186906:
                if (id == 43)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            case 2186907:
                if (id == 25)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            case 2186908:
                if (id == 41)
                {
                    me->StopMoving();
                    me->SetRooted(true);
                    me->GetMotionMaster()->Clear();
                    EventTimer = 2000;
                    Phase = 4;
                }
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EventTimer.Expired(diff))
            {
                switch (Phase)
                {
                case 0:
                    me->HandleEmote(383);
                    me->CastSpell(me, 36545, false);
                    me->SetFlying(true);
                    if (Unit* wpOne = GetClosestCreatureWithEntry(me, 21443, 100, true))
                        me->GetMotionMaster()->MovePoint(100, wpOne->GetPositionX(), wpOne->GetPositionY(), wpOne->GetPositionZ(), true);
                    EventTimer = 0;
                    break;
                case 1:
                    EventTimer = 0;
                    if (Creature* lighter = GetClosestCreatureWithEntry(me, 21353, 100))
                        me->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, me, lighter, 0);
                    break;
                case 2:
                    me->HandleEmote(0);
                    me->SetFlying(false);
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    me->RemoveAurasDueToSpell(36545);
                    EventTimer = 2000;
                    Phase = 3;
                    break;
                case 3:
                    me->SetWalk(true);
                    me->setActive(true);
                    me->LoadPath(PathId);
                    me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                    me->GetMotionMaster()->Initialize();
                    EventTimer = 0;
                    Phase = 4;
                    break;
                case 4:
                    me->StopMoving();
                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                    if (Creature* trigger = me->SummonCreature(21903, -3362.04, 5140.009, -20.135, 0, TEMPSUMMON_TIMED_DESPAWN, 3000))
                        trigger->CastSpell(me, 36927, false);
                    EventTimer = 1900;
                    Phase = 5;
                    break;
                case 5:
                    me->CastSpell(me, 43759, false);
                    me->ForcedDespawn(1000);
                    Phase = 0;
                    EventTimer = 0;
                    break;
                }
            }
            return;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_21869(Creature *creature)
{
    return new npc_21869AI(creature);
}


struct npc_21353AI : public ScriptedAI
{
    npc_21353AI(Creature *creature) : ScriptedAI(creature) {}

    void Reset()
    {
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 misc)
    {
        if (eventType == AI_EVENT_CUSTOM_EVENTAI_A)
        {
            me->CastSpell(invoker, 37229, true);
            me->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_B, me, (Creature*)invoker, 0);            
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            return;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_21353(Creature *creature)
{
    return new npc_21353AI(creature);
}


struct npc_18713AI : public ScriptedAI
{
    npc_18713AI(Creature *creature) : ScriptedAI(creature) {}

    Timer EventTimer;
    uint32 Phase;

    void Reset()
    {
        EventTimer.Reset(10000);
        Phase = 0;
    }

    void EnterCombat(Unit* who)
    {
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            me->SetWalk(true);
            if (id == 100)
                me->GetMotionMaster()->MovePoint(101, -3011.903, 3982.82, 3.105);
            if (id == 101)
                me->GetMotionMaster()->MovePoint(102, -3010.23, 3995.06, 4.46);
            if (id == 102)
            {
                me->SetFacingTo(0.14);
                me->SetRooted(true);
                if (Creature* cap = GetClosestCreatureWithEntry(me, 18745, 10))
                    cap->SetFacingTo(3.2);
                me->HandleEmote(66);
                EventTimer = 4000;
                Phase = 3;
            }
            if (id == 103)
                me->GetMotionMaster()->MovePoint(104, -3002.51, 3988.99, 3.105);
            if (id == 104)
                me->GetMotionMaster()->MovePoint(105, -3003.82, 3981.51, 3.105);
            if (id == 105)
                me->GetMotionMaster()->MovePoint(0, -3007.89, 3978.21, 3.105);
        }
    }
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EventTimer.Expired(diff))
            {
                switch (Phase)
                {
                case 0:
                    DoScriptText(-19110210, 0);
                    EventTimer = 5000;
                    Phase = 1;
                    break;
                case 1:
                    DoScriptText(-1554029, me);
                    EventTimer = 5000;
                    Phase = 2;
                case 2:
                    me->GetMotionMaster()->MovePoint(100, -3009.292, 3981.76, 3.105);
                    EventTimer = 0;
                    Phase = 0;
                    break;
                case 3:
                    DoScriptText(-1554030, me);
                    EventTimer = 4000;
                    Phase = 4;
                    break;
                case 4:
                    if (Creature* cap = GetClosestCreatureWithEntry(me, 18745, 10))
                        DoScriptText(RAND(-1554031, -1554032), cap);
                    EventTimer = 4000;
                    Phase = 5;
                    break;
                case 5:
                    DoScriptText(RAND(-1554033, -1554034), me);
                    EventTimer = 8000;
                    Phase = 6;
                    break;
                case 6:
                    if (Creature* cap = GetClosestCreatureWithEntry(me, 18745, 10))
                        DoScriptText(RAND(-1554035, -1554036), cap);
                    EventTimer = 5000;
                    Phase = 7;
                    break;
                case 7:
                    DoScriptText(-1554037, me);
                    EventTimer = 4000;
                    Phase = 8;
                    break;
                case 8:
                    me->SetRooted(false);
                    me->GetMotionMaster()->MovePoint(103, -3010.54, 3990.09, 3.105);
                    EventTimer = 60000;
                    Phase = 0;
                    break;
                }
            }
            return;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_18713(Creature *creature)
{
    return new npc_18713AI(creature);
}


void AddSC_terokkar_forest()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_18713";
    newscript->GetAI = &GetAI_npc_18713;

    newscript->RegisterSelf();
    newscript = new Script;
    newscript->Name = "npc_21353";
    newscript->GetAI = &GetAI_npc_21353;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21869";
    newscript->GetAI = &GetAI_npc_21869;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21285";
    newscript->GetAI = &GetAI_npc_21285;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19847";
    newscript->GetAI = &GetAI_npc_19847;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_18716";
    newscript->pGossipHello =  &GossipHello_npc_18716;
    newscript->pGossipSelect = &GossipSelect_npc_18716;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_18717";
    newscript->pGossipHello =  &GossipHello_npc_18717;
    newscript->pGossipSelect = &GossipSelect_npc_18717;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_18719";
    newscript->GetAI = &GetAI_npc_18719;
    newscript->pGossipHello =  &GossipHello_npc_18719;
    newscript->pGossipSelect = &GossipSelect_npc_18719;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_18714_18715";
    newscript->pGossipHello =  &GossipHello_npc_18714_18715;
    newscript->pGossipSelect = &GossipSelect_npc_18714_18715;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_18714_18715;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_185211";
    newscript->pGOUse = &go_185211;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_23363";
    newscript->pGossipHello =  &GossipHello_npc_23363;
    newscript->pGossipSelect = &GossipSelect_npc_23363;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_19718";
    newscript->pGossipHello =  &GossipHello_npc_19718;
    newscript->pGossipSelect = &GossipSelect_npc_19718;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_unkor_the_ruthless";
    newscript->GetAI = &GetAI_mob_unkor_the_ruthless;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_infested_root_walker";
    newscript->GetAI = &GetAI_mob_infested_root_walker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_rotting_forest_rager";
    newscript->GetAI = &GetAI_mob_rotting_forest_rager;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_netherweb_victim";
    newscript->GetAI = &GetAI_mob_netherweb_victim;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_floon";
    newscript->GetAI = &GetAI_npc_floon;
    newscript->pGossipHello =  &GossipHello_npc_floon;
    newscript->pGossipSelect = &GossipSelect_npc_floon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_skyguard_handler_deesak";
    newscript->pGossipHello =  &GossipHello_npc_skyguard_handler_deesak;
    newscript->pGossipSelect = &GossipSelect_npc_skyguard_handler_deesak;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_isla_starmane";
    newscript->GetAI = &GetAI_npc_isla_starmane;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_isla_starmane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_skull_pile";
    newscript->pGOUse  = &GossipHello_go_skull_pile;
    newscript->pGossipSelectGO = &GossipSelect_go_skull_pile;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_ancient_skull_pile";
    newscript->pGOUse  = &GossipHello_go_ancient_skull_pile;
    newscript->pGossipSelectGO = &GossipSelect_go_ancient_skull_pile;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_terokk";
    newscript->GetAI = &GetAI_mob_terokk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_skyguard_ace";
    newscript->GetAI = &GetAI_npc_skyguard_ace;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_blackwing_warp_chaser";
    newscript->GetAI = &GetAI_npc_blackwing_warp_chaser;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_skyguard_prisoner";
    newscript->GetAI = &GetAI_npc_skyguard_prisoner;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_skyguard_prisoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_letoll";
    newscript->GetAI = &GetAI_npc_letollAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_letoll;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_sarthis";
    newscript->GetAI = &GetAI_npc_sarthisAI;
    newscript->pGossipHello  = &GossipHello_npc_sarthis;
    newscript->pGossipSelect = &GossipSelect_npc_sarthis;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_sarthis_elemental";
    newscript->GetAI = &GetAI_npc_sarthis_elementalAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_minion_of_sarthis";
    newscript->GetAI = &GetAI_npc_minion_of_sarthisAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_razorthorn_ravager";
    newscript->GetAI = &GetAI_npc_razorthorn_ravagerAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="quest_the_vengeful_harbringer";
    newscript->GetAI = &GetAI_quest_the_vengeful_harbringer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_akuno";
    newscript->GetAI = &GetAI_npc_akuno;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_akuno;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_empoor";
    newscript->GetAI = &GetAI_npc_empoor;
    newscript->pGossipHello =  &GossipHello_npc_empoor;
    newscript->pGossipSelect = &GossipSelect_npc_empoor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_captive_child";
    newscript->GetAI = &GetAI_npc_captive_child;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_veil_skith_cage";
    newscript->pGOUse = &go_veil_skith_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_skywing";
    newscript->GetAI = &GetAI_npc_skywing;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_skywing;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_cenarion_sparrowhawk";
    newscript->GetAI = &GetAI_npc_cenarion_sparrowhawk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lady_dena_kennedy";
    newscript->pReceiveEmote = &ReceiveEmote_Dena_Kennedy;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_who_are_they";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_who_are_they;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_who_are_they;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18707";
    newscript->GetAI = &GetAI_npc_18707;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_19606";
    newscript->GetAI = &GetAI_npc_19606;
    newscript->pGossipHello =  &GossipHello_npc_19606;
    newscript->pGossipSelect = &GossipSelect_npc_19606;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18384";
    newscript->GetAI = &GetAI_npc_18384;
    newscript->RegisterSelf();
}
