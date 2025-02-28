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
SDName: Azuremyst_Isle
SD%Complete: 75
SDComment: Quest support: 9283, 9528, 9537, 9544, 9582, 9554, 9531, 9303(special flight path, proper model for mount missing). Injured Draenei cosmetic only, 9582
SDCategory: Azuremyst Isle
EndScriptData */

/* ContentData
npc_draenei_survivor
npc_engineer_spark_overgrind
npc_injured_draenei
npc_magwin
npc_susurrus
npc_geezle
mob_nestlewood_owlkin
mob_siltfin_murloc
npc_stillpine_capitive
go_bristlelimb_cage
go_ravager_cage
npc_death_ravager
npc_17311
mob_surveyor_candress
npc_17058
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include <cmath>

/*######
## npc_draenei_survivor
######*/

#define SAY_HEAL1    -1000248
#define SAY_HEAL2    -1000249
#define SAY_HEAL3    -1000250
#define SAY_HEAL4    -1000251

#define HELP1        -1000252
#define HELP2        -1000253
#define HELP3        -1000254
#define HELP4        -1000255

#define SPELL_IRRIDATION    35046
#define SPELL_STUNNED       28630

struct npc_draenei_survivorAI : public ScriptedAI
{
    npc_draenei_survivorAI(Creature *c) : ScriptedAI(c) {}

    uint64 pCaster;
    Timer SayThanksTimer;
    Timer RunAwayTimer;
    Timer SayHelpTimer;

    bool CanSayHelp;
    bool CanSayHelp2;

    void Reset()
    {
        pCaster = 0;
        SayThanksTimer.Reset(0);
        RunAwayTimer.Reset(0);
        SayHelpTimer.Reset(10000);

        CanSayHelp = true;
        CanSayHelp2 = true;

        me->CastSpell(me, SPELL_IRRIDATION, true);
        me->SetHealth(int(me->GetMaxHealth()*.1));
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_SLEEP);
    }

    void EnterCombat(Unit *who) {}

    void MoveInLineOfSight(Unit *who)
    {
        if (CanSayHelp && CanSayHelp2 && who->GetTypeId() == TYPEID_PLAYER && me->IsFriendlyTo(who) && me->IsWithinDistInMap(who, 25.0f))
        {
            DoScriptText(RAND(HELP1, HELP2, HELP3, HELP4), me);

            SayHelpTimer = 20000;
            CanSayHelp = false;
        }
    }

    void SpellHit(Unit *Caster, const SpellEntry *Spell)
    {
        if(Spell->Id == 28880)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
            me->CastSpell(me, SPELL_STUNNED, true);

            pCaster = Caster->GetGUID();

            CanSayHelp2 = false;
            SayThanksTimer = 5000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(SayThanksTimer.Expired(diff))
        {
            me->RemoveAurasDueToSpell(SPELL_IRRIDATION);

            if (Player *pPlayer = (Player*)Unit::GetUnit(*me,pCaster))
            {
                if (pPlayer->GetTypeId() != TYPEID_PLAYER)
                    return;

                DoScriptText(RAND(SAY_HEAL1, SAY_HEAL2, SAY_HEAL3, SAY_HEAL4), me, pPlayer);

                pPlayer->TalkedToCreature(me->GetEntry(),me->GetGUID());
            }

            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(0, -4115.053711f, -13754.831055f, 73.508949f);
            RunAwayTimer = 10000;
            SayThanksTimer = 0;
            return;
        }

        if(RunAwayTimer.Expired(diff))
        {
            me->RemoveAllAuras();
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();
            me->setDeathState(JUST_DIED);
            me->SetHealth(0);
            me->CombatStop();
            me->DeleteThreatList();
            me->RemoveCorpse();
            return;
        }

        if (SayHelpTimer.Expired(diff))
        {
            CanSayHelp = true;
            SayHelpTimer = 20000;
        }
        
    }
};

CreatureAI* GetAI_npc_draenei_survivor(Creature *_Creature)
{
    return new npc_draenei_survivorAI (_Creature);
}

/*######
## npc_sethir_the_ancient
######*/

#define EMOTE_SOUND_DIE                  -1069090

struct npc_sethir_the_ancientAI : public ScriptedAI
{
    npc_sethir_the_ancientAI(Creature *c) : ScriptedAI(c) {}

    Timer SpawnTimer;       // Do not spawn all mobs immediately
    Timer temp;
    bool pause_say;     // wait some time until say sentence again

    void Reset()
    {
       SpawnTimer.Reset(1000);
       pause_say = false;
       temp.Reset(0);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(EMOTE_SOUND_DIE, me);
    }

    void SummonedCreatureDespawn(Creature*)
    {
        EnterEvadeMode();   // evade after 3 seconds if summons do not aggro someone
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!me->IsInCombat() && !pause_say && me->IsWithinDistInMap(who, 30) && me->IsHostileTo(who) && who->HasAuraType(SPELL_AURA_MOD_STEALTH))
        {
            DoScriptText(-1230073, me, 0);
            pause_say = true;
            temp = 60000;
        }
        //if (!me->IsInCombat() && me->IsWithinDistInMap(who, 30) && me->IsHostileTo(who)) AttackStart(who);
        ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(temp.Expired(diff))      // after 1 minute he can say it again
        {
            pause_say = false;
            temp = 60000;
        }
        

        if (SpawnTimer.Expired(diff))
        {
            Position pos;
            me->GetPosition(pos);

            for (int i = 1; i <= 6; i++)
            {
                Creature * tmpC = me->SummonCreature(6911, pos.x, pos.y, pos.z, pos.o, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 3000);
                tmpC->AI()->AttackStart(me->GetVictim());
            }

            SpawnTimer = 0;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_sethir_the_ancient(Creature *_Creature)
{
    return new npc_sethir_the_ancientAI (_Creature);
}

/*######
## npc_engineer_spark_overgrind
######*/

#define SAY_TEXT        -1000256
#define SAY_EMOTE       -1000257
#define ATTACK_YELL     -1000258

#define GOSSIP_FIGHT    16267

#define SPELL_DYNAMITE  7978

struct npc_engineer_spark_overgrindAI : public ScriptedAI
{
    npc_engineer_spark_overgrindAI(Creature *c) : ScriptedAI(c) {}

    Timer Dynamite_Timer;
    Timer Emote_Timer;

    void Reset()
    {
        Dynamite_Timer.Reset(8000);
        Emote_Timer.Reset(120000 + rand()%30000);
        me->setFaction(875);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if(!me->IsInCombat())
        {
            if (Emote_Timer.Expired(diff))
            {
                DoScriptText(SAY_TEXT, me);
                DoScriptText(SAY_EMOTE, me);
                Emote_Timer = 120000 + rand()%30000;
            }
        }

        if(!UpdateVictim())
            return;

        if (Dynamite_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_DYNAMITE);
            Dynamite_Timer = 8000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_engineer_spark_overgrind(Creature *_Creature)
{
    return new npc_engineer_spark_overgrindAI (_Creature);
}

bool GossipHello_npc_engineer_spark_overgrind(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(9537) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_FIGHT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_engineer_spark_overgrind(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->setFaction(14);
        DoScriptText(ATTACK_YELL, _Creature, player);
        ((npc_engineer_spark_overgrindAI*)_Creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## npc_injured_draenei
######*/

struct npc_injured_draeneiAI : public ScriptedAI
{
    npc_injured_draeneiAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        me->SetHealth(int(me->GetMaxHealth()*.15));

        me->SetUInt32Value(UNIT_FIELD_BYTES_1, RAND(UNIT_STAND_STATE_SIT, UNIT_STAND_STATE_SLEEP));
    }

    void EnterCombat(Unit *who) {}

    void MoveInLineOfSight(Unit *who)
    {
        return;                                             //ignore everyone around them (won't aggro anything)
    }

    void UpdateAI(const uint32 diff)
    {
        return;
    }

};
CreatureAI* GetAI_npc_injured_draenei(Creature *_Creature)
{
    return new npc_injured_draeneiAI (_Creature);
}

/*######
## npc_magwin
######*/

enum eMagwin
{
    SAY_START                   = -1000111,
    SAY_AGGRO                   = -1000112,
    SAY_PROGRESS                = -1000113,
    SAY_END1                    = -1000114,
    SAY_END2                    = -1000115,
    EMOTE_HUG                   = -1000116,

    QUEST_A_CRY_FOR_SAY_HELP    = 9528
};

struct npc_magwinAI : public npc_escortAI
{
    npc_magwinAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

         if (!pPlayer)
            return;

        switch(i)
        {
        case 0:
            DoScriptText(SAY_START, me, pPlayer);
            break;
        case 17:
            DoScriptText(SAY_PROGRESS, me, pPlayer);
            break;
        case 28:
            DoScriptText(SAY_END1, me, pPlayer);
            break;
        case 29:
            DoScriptText(EMOTE_HUG, me, pPlayer);
            DoScriptText(SAY_END2, me, pPlayer);
            pPlayer->GroupEventHappens(QUEST_A_CRY_FOR_SAY_HELP,me);
            break;
        }
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_AGGRO, me, who);
    }

    void Reset() { }
};

bool QuestAccept_npc_magwin(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_A_CRY_FOR_SAY_HELP)
    {
        pCreature->setFaction(113);
        if (npc_escortAI* pEscortAI = CAST_AI(npc_escortAI, pCreature->AI()))
            pEscortAI->Start(true, false, pPlayer->GetGUID());
    }
    return true;
}

CreatureAI* GetAI_npc_magwinAI(Creature *_Creature)
{
    npc_magwinAI* magwinAI = new npc_magwinAI(_Creature);

    magwinAI->AddWaypoint(0, -4784.532227, -11051.060547, 3.484263);
    magwinAI->AddWaypoint(1, -4805.509277, -11037.293945, 3.043942);
    magwinAI->AddWaypoint(2, -4827.826172, -11034.398438, 1.741959);
    magwinAI->AddWaypoint(3, -4852.630859, -11033.695313, 2.208656);
    magwinAI->AddWaypoint(4, -4876.791992, -11034.517578, 3.175228);
    magwinAI->AddWaypoint(5, -4895.486816, -11038.306641, 9.390890);
    magwinAI->AddWaypoint(6, -4915.464844, -11048.402344, 12.369793);
    magwinAI->AddWaypoint(7, -4937.288086, -11067.041992, 13.857983);
    magwinAI->AddWaypoint(8, -4966.577637, -11067.507813, 15.754786);
    magwinAI->AddWaypoint(9, -4993.799805, -11056.544922, 19.175295);
    magwinAI->AddWaypoint(10, -5017.836426, -11052.569336, 22.476587);
    magwinAI->AddWaypoint(11, -5039.706543, -11058.459961, 25.831593);
    magwinAI->AddWaypoint(12, -5057.289063, -11045.474609, 26.972496);
    magwinAI->AddWaypoint(13, -5078.828125, -11037.601563, 29.053417);
    magwinAI->AddWaypoint(14, -5104.158691, -11039.195313, 29.440195);
    magwinAI->AddWaypoint(15, -5120.780273, -11039.518555, 30.142139);
    magwinAI->AddWaypoint(16, -5140.833008, -11039.810547, 28.788074);
    magwinAI->AddWaypoint(17, -5161.201660, -11040.050781, 27.879545, 4000);
    magwinAI->AddWaypoint(18, -5171.842285, -11046.803711, 27.183821);
    magwinAI->AddWaypoint(19, -5185.995117, -11056.359375, 20.234867);
    magwinAI->AddWaypoint(20, -5198.485840, -11065.065430, 18.872593);
    magwinAI->AddWaypoint(21, -5214.062500, -11074.653320, 19.215731);
    magwinAI->AddWaypoint(22, -5220.157227, -11088.377930, 19.818476);
    magwinAI->AddWaypoint(23, -5233.652832, -11098.846680, 18.349432);
    magwinAI->AddWaypoint(24, -5250.163086, -11111.653320, 16.438959);
    magwinAI->AddWaypoint(25, -5268.194336, -11125.639648, 12.668313);
    magwinAI->AddWaypoint(26, -5286.270508, -11130.669922, 6.912246);
    magwinAI->AddWaypoint(27, -5317.449707, -11137.392578, 4.963446);
    magwinAI->AddWaypoint(28, -5334.854492, -11154.384766, 6.742664);
    magwinAI->AddWaypoint(29, -5353.874512, -11171.595703, 6.903912, 20000);
    magwinAI->AddWaypoint(30, -5354.240000, -11171.940000, 6.890000);

    return (CreatureAI*)magwinAI;
}

/*######
## npc_susurrus
######*/

bool GossipHello_npc_susurrus(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->HasItemCount(23843,1,true))
        player->ADD_GOSSIP_ITEM(0, "I am ready.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_susurrus(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,32474,true);               //apparently correct spell, possible not correct place to cast, or correct caster

        std::vector<uint32> nodes;

        nodes.resize(2);
        nodes[0] = 92;                                      //from susurrus
        nodes[1] = 91;                                      //end at exodar
        player->ActivateTaxiPathTo(nodes,11686);            //TaxiPath 506. Using invisible model, possible Trinity must allow 0(from dbc) for cases like this.
    }
    return true;
}

/*######
## npc_geezle
######*/

#define GEEZLE_SAY_1    -1000259
#define SPARK_SAY_2     -1000260
#define SPARK_SAY_3     -1000261
#define GEEZLE_SAY_4    -1000262
#define SPARK_SAY_5     -1000263
#define SPARK_SAY_6     -1000264
#define GEEZLE_SAY_7    -1000265

#define EMOTE_SPARK     -1000266

#define MOB_SPARK       17243
#define GO_NAGA_FLAG    181694

static float SparkPos[3] = {-5030.95, -11291.99, 7.97};

struct npc_geezleAI : public ScriptedAI
{
    npc_geezleAI(Creature *c) : ScriptedAI(c) {}

    std::list<GameObject*> FlagList;

    uint64 SparkGUID;

    uint32 Step;
    Timer SayTimer;

    bool EventStarted;

    void Reset()
    {
        SparkGUID = 0;
        Step = 0;
        SayTimer.Reset(0);
        StartEvent();
    }

    void StartEvent()
    {
        Step = 1;
        EventStarted = true;
        Creature* Spark = me->SummonCreature(MOB_SPARK, SparkPos[0], SparkPos[1], SparkPos[2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
        if(Spark)
        {
            SparkGUID = Spark->GetGUID();
            Spark->setActive(true);
            Spark->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            Spark->SetWalk(true);
            Spark->GetMotionMaster()->MovePoint(0, -5080.70, -11253.61, 0.56);
        }
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(0, -5092.26, -11252, 0.71);
        SayTimer = 20000;
    }

    uint32 NextStep(uint32 Step)
    {
        Unit* Spark = Unit::GetUnit((*me), SparkGUID);

        switch(Step)
        {
        case 0: return 99999;
        case 1:
            //DespawnNagaFlag(true);
            DoScriptText(EMOTE_SPARK, Spark);
            return 1000;
        case 2:
            DoScriptText(GEEZLE_SAY_1, me, Spark);
            if(Spark)
            {
                Spark->SetInFront(me);
                me->SetInFront(Spark);
            }
            return 5000;
        case 3: DoScriptText(SPARK_SAY_2, Spark); return 7000;
        case 4: DoScriptText(SPARK_SAY_3, Spark); return 8000;
        case 5: DoScriptText(GEEZLE_SAY_4, me, Spark); return 8000;
        case 6: DoScriptText(SPARK_SAY_5, Spark); return 9000;
        case 7: DoScriptText(SPARK_SAY_6, Spark); return 8000;
        case 8: DoScriptText(GEEZLE_SAY_7, me, Spark); return 2000;
        case 9:
            me->GetMotionMaster()->MovePoint(0, -5134.3, -11250.3, 5.29568);
            if(Spark)
                Spark->GetMotionMaster()->MovePoint(0, -5030.95, -11291.99, 7.97);
            return 15000;
        case 10:
            if(Spark)
                Spark->DealDamage(Spark,Spark->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            //DespawnNagaFlag(false);
            me->SetVisibility(VISIBILITY_OFF);
            me->GetMotionMaster()->MoveTargetedHome();
        default: return 99999999;
        }
    }

    void DespawnNagaFlag(bool despawn)
    {
        Hellground::AllGameObjectsWithEntryInGrid go_check(GO_NAGA_FLAG);
        Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> go_search(FlagList, go_check);
        Cell::VisitGridObjects(me, go_search, me->GetMap()->GetVisibilityDistance());

        Player* player = NULL;
        if (!FlagList.empty())
        {
            for(std::list<GameObject*>::iterator itr = FlagList.begin(); itr != FlagList.end(); ++itr)
            {
                //TODO: Found how to despawn and respawn
                if(despawn)
                    (*itr)->RemoveFromWorld();
                else
                    (*itr)->Respawn();
            }
        }
        else
            error_log("SD2 ERROR: FlagList is empty!");
    }

    void UpdateAI(const uint32 diff)
    {
        if(SayTimer.Expired(diff))
        {
            if(EventStarted)
            {
                SayTimer = NextStep(++Step);
            }
        }
    }
};

CreatureAI* GetAI_npc_geezleAI(Creature *_Creature)
{
    return new npc_geezleAI(_Creature);
}

/*######
## mob_nestlewood_owlkin
######*/

#define INOCULATION_QUEST 9303
#define INOCULATION_CHANNEL 29528
#define INOCULATED_OWLKIN   16534
#define OWLKIN              16518

struct mob_nestlewood_owlkinAI : public ScriptedAI
{
    mob_nestlewood_owlkinAI(Creature *c) : ScriptedAI(c) {}

    Timer ChannelTimer;
    bool Channeled;
    bool Hitted;
    uint64 PlayerGUID;

    void Reset()
    {
        ChannelTimer.Reset(0);
        Channeled = false;
        Hitted = false;
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        PlayerGUID = 0;
    }

    void EnterCombat(Unit *who){}

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(!caster)
            return;

        if(caster->GetTypeId() == TYPEID_PLAYER && spell->Id == INOCULATION_CHANNEL)
        {
            ChannelTimer = 3000;
            Hitted = true;
            PlayerGUID = ((Player*)caster)->GetGUID();
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
            // ((Player*)caster)->CastCreatureOrGO(OWLKIN, me->GetGUID(), INOCULATION_CHANNEL);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(ChannelTimer.Expired(diff) && !Channeled && Hitted)
        {
            if(Player* target = Unit::GetPlayerInWorld(PlayerGUID))
                target->CastCreatureOrGO(OWLKIN, 0, INOCULATION_CHANNEL);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            me->DisappearAndDie();
            me->SummonCreature(INOCULATED_OWLKIN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 180000);
            Channeled = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_nestlewood_owlkinAI(Creature *_Creature)
{
    return new mob_nestlewood_owlkinAI (_Creature);
}

/*######
## mob_siltfin_murloc
######*/

struct mob_siltfin_murlocAI : public ScriptedAI
{
    mob_siltfin_murlocAI(Creature *c) : ScriptedAI(c) {}

    void EnterCombat(Unit *who){}

    void JustDied(Unit *player)
    {
        player = player->GetCharmerOrOwnerPlayerOrPlayerItself();

        if(roll_chance_i(20) && player)
        {
            if(((Player*)player)->GetQuestStatus(9595) == QUEST_STATUS_INCOMPLETE)
            {
                 Unit* summon = me->SummonCreature(17612, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                 player->CastSpell(summon, 30790, false);
            }
        }
     }

};

CreatureAI* GetAI_mob_siltfin_murlocAI(Creature *_Creature)
{
    return new mob_siltfin_murlocAI (_Creature);
}

/*######
## npc_death_ravager
######*/

enum eRavegerCage
{
    NPC_DEATH_RAVAGER       = 17556,

    SPELL_REND              = 13443,
    SPELL_ENRAGING_BITE     = 30736,

    QUEST_STRENGTH_ONE      = 9582
};

bool go_ravager_cage(Player* pPlayer, GameObject* pGo)
{

    if (pPlayer->GetQuestStatus(QUEST_STRENGTH_ONE) == QUEST_STATUS_INCOMPLETE)
    {
        if (Creature* ravager = GetClosestCreatureWithEntry(pGo, NPC_DEATH_RAVAGER, 5.0f))
        {
            ravager->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            ravager->SetReactState(REACT_AGGRESSIVE);
            ravager->AI()->AttackStart(pPlayer);
        }
    }
    return true ;
}

struct npc_death_ravagerAI : public ScriptedAI
{
    npc_death_ravagerAI(Creature *c) : ScriptedAI(c){}

    Timer RendTimer;
    Timer EnragingBiteTimer;

    void Reset()
    {
        RendTimer.Reset(30000);
        EnragingBiteTimer.Reset(20000);

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (RendTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_REND);
            RendTimer = 30000;
        }

        if (EnragingBiteTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_ENRAGING_BITE);
            EnragingBiteTimer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_death_ravagerAI(Creature* pCreature)
{
    return new npc_death_ravagerAI(pCreature);
}

/*########
## Quest: The Prophecy of Akida
########*/

enum BristlelimbCage
{
    CAPITIVE_SAY_1                      = -1600474,
    CAPITIVE_SAY_2                      = -1600475,
    CAPITIVE_SAY_3                      = -1600476,

    QUEST_THE_PROPHECY_OF_AKIDA         = 9544,
    NPC_STILLPINE_CAPITIVE              = 17375,
    GO_BRISTELIMB_CAGE                  = 181714

};


struct npc_stillpine_capitiveAI : public ScriptedAI
{
    npc_stillpine_capitiveAI(Creature *c) : ScriptedAI(c){}

    uint32 FleeTimer;

    void Reset()
    {
        FleeTimer = 0;
        GameObject* cage = FindGameObject(GO_BRISTELIMB_CAGE, 5.0f, me);
        if(cage)
            cage->ResetDoorOrButton();
    }

    void UpdateAI(const uint32 diff)
    {
    }
};

CreatureAI* GetAI_npc_stillpine_capitiveAI(Creature* pCreature)
{
    return new npc_stillpine_capitiveAI(pCreature);
}

bool go_bristlelimb_cage(Player* pPlayer, GameObject* pGo)
{
    if(pPlayer->GetQuestStatus(QUEST_THE_PROPHECY_OF_AKIDA) == QUEST_STATUS_INCOMPLETE)
    {
        Creature* pCreature = GetClosestCreatureWithEntry(pGo, NPC_STILLPINE_CAPITIVE, 5.0f);
        if(pCreature)
        {
            DoScriptText(RAND(CAPITIVE_SAY_1, CAPITIVE_SAY_2, CAPITIVE_SAY_3), pCreature, pPlayer);
            pCreature->GetMotionMaster()->MoveFleeing(pPlayer, 3500);
            pPlayer->KilledMonster(pCreature->GetEntry(), pCreature->GetGUID());
            pCreature->ForcedDespawn(3500);
            return false;
        }
    }
    return true;
}

/*######
## Quest: Matis the Cruel
######*/

#define NPC_MATIS    17664
#define SAY_1        -1900255
#define SAY_2        -1900256

struct npc_trackerAI : public ScriptedAI
{
    npc_trackerAI(Creature* creature) : ScriptedAI(creature) {}

    uint64 matisguid;
    bool Checked;

    void Reset()
    {
        DoScriptText(SAY_1, me);
        me->setFaction(1700);
        if (Creature* Matis = GetClosestCreatureWithEntry(me, NPC_MATIS, 35.0f))
        {
            me->AI()->AttackStart(Matis);
            matisguid = Matis->GetGUID();
            Matis->Unmount();
        }
        Checked = false;
    }

    void Credit()
    { 
        Map* map = me->GetMap();
        Map::PlayerList const &PlayerList = map->GetPlayers();

        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            if (Player* player = itr->getSource())
            {
                if (me->IsWithinDistInMap(player, 35.0f) && (player->GetQuestStatus(9711) == QUEST_STATUS_INCOMPLETE))
                    player->GroupEventHappens(9711, me);
            }
        }
    }

    void EnterEvadeMode()
    {
        me->DeleteThreatList();
        me->CombatStop(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Creature* Matis = me->GetCreature(matisguid))
        {
            if (!Checked && Matis->HealthBelowPct(10))
            {
                me->AI()->EnterEvadeMode();
                Matis->setFaction(35);
                Matis->CombatStop();
                Matis->DeleteThreatList();
                Matis->SetHealth(Matis->GetMaxHealth());
                DoScriptText(SAY_2, me);
                Credit();
                Matis->ForcedDespawn(10000);
                me->ForcedDespawn(10000);
                Checked = true;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_tracker(Creature* creature)
{
    return new npc_trackerAI (creature);
}

struct npc_stillpine_ancestor_akidaAI : public npc_escortAI
{
    npc_stillpine_ancestor_akidaAI(Creature *c) : npc_escortAI(c){}

    void Reset()
    {
    }

    void WaypointReached(uint32 wp)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(wp)
        {
            case 10:
                me->MonsterTextEmote(-1200314,0);
                me->CastSpell(me, 30428, true);
                me->ForcedDespawn(3000);
                break;
        }
    }
    
    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_totem_of_akida(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 9539)
    {
        Unit* akida = FindCreature(17379, 20, pCreature);
        if(!akida)
        {
            if(Creature* spirit = pCreature->SummonCreature(17379, -4178.82861, -12508.79394, 44.362156, 4.622056, TEMPSUMMON_TIMED_DESPAWN, 240000))
            {
                spirit->MonsterTextEmote(-1200315, 0);
                spirit->CastSpell(spirit, 30428, true);
                if (npc_escortAI* pEscortAI = CAST_AI(npc_stillpine_ancestor_akidaAI, spirit->AI()))
                    pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);
            }
        }
    }
    return true;
}

CreatureAI* GetAI_npc_stillpine_ancestor_akida(Creature* pCreature)
{
    npc_stillpine_ancestor_akidaAI* thisAI = new npc_stillpine_ancestor_akidaAI(pCreature);

    thisAI->AddWaypoint(0, -4154.224, -12514.75, 45.35527);
    thisAI->AddWaypoint(1, -4123.563, -12517.23, 44.9127);
    thisAI->AddWaypoint(2, -4091.881, -12524, 42.37354);
    thisAI->AddWaypoint(3, -4058.039, -12538.57, 43.96096);
    thisAI->AddWaypoint(4, -4026.534, -12568.4, 45.82222);
    thisAI->AddWaypoint(5, -4000.155, -12598.55, 54.19722);
    thisAI->AddWaypoint(6, -3977.5, -12627.22, 63.1442);
    thisAI->AddWaypoint(7, -3952.254, -12660.37, 74.23783);
    thisAI->AddWaypoint(8, -3933.183, -12698.27, 85.65151);
    thisAI->AddWaypoint(9, -3925.843, -12718.81, 89.94553);
    thisAI->AddWaypoint(10, -3915.914, -12743.41, 98.56779);

    return (CreatureAI*)thisAI;
}

struct npc_stillpine_ancestor_cooAI : public npc_escortAI
{
    npc_stillpine_ancestor_cooAI(Creature *c) : npc_escortAI(c){}

    void Reset()
    {
    }

    void WaypointReached(uint32 wp)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(wp)
        {
            case 2:
                DoScriptText(-1230074, me, 0);
                break;
            case 3:
                DoScriptText(-1230075, me, 0);
                me->CastSpell(pPlayer, 30424, false);
                me->SetDisplayId(17019);
                me->CastSpell(me, 30428, true);
                me->SetFlying(true);
                break;
            case 6:
                me->CastSpell(me, 30428, true);
                me->ForcedDespawn(3000);
                break;
        }
    }
    
    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_stillpine_ancestor_coo(Creature* pCreature)
{
    npc_stillpine_ancestor_cooAI* thisAI = new npc_stillpine_ancestor_cooAI(pCreature);

    thisAI->AddWaypoint(0, -3926.039,-12746.71,96.06779);
    thisAI->AddWaypoint(1, -3926.326,-12753.71,98.44279);
    thisAI->AddWaypoint(2, -3924.268,-12761.45,101.6928, 6000);
    thisAI->AddWaypoint(3, -3924.268,-12761.46,101.6928, 4000);
    thisAI->AddWaypoint(4, -3926.068,-12767.33,104.3799);
    thisAI->AddWaypoint(5, -3923.497,-12794.98,98.46323);
    thisAI->AddWaypoint(6, -3922.687,-12832.4,89.24097);

    return (CreatureAI*)thisAI;
}

bool QuestAccept_npc_totem_of_coo(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 9540)
    {
        Unit* coo = FindCreature(17391, 20, pCreature);
        if(!coo)
        {
            if(Creature* spirit = pCreature->SummonCreature(17391, -3920.650391, -12743.317383, 96.568825, 3.27981, TEMPSUMMON_TIMED_DESPAWN, 240000))
            {
                spirit->MonsterTextEmote(-1200315, 0);
                spirit->CastSpell(spirit, 30428, true);
                if (npc_escortAI* pEscortAI = CAST_AI(npc_stillpine_ancestor_cooAI, spirit->AI()))
                    pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);
            }
        }
    }
    return true;
}

struct npc_stillpine_ancestor_tiktiAI : public npc_escortAI
{
    npc_stillpine_ancestor_tiktiAI(Creature *c) : npc_escortAI(c){}

    void Reset()
    {
    }

    void WaypointReached(uint32 wp)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(wp)
        {
            case 1:
                DoScriptText(-1230076, me, 0);
                me->CastSpell(pPlayer, 30430, false);
                break;
            case 2:
                DoScriptText(-1230077, me, 0);
                me->SetSpeed(MOVE_SWIM, 2.0, true);
                me->CastSpell(me, 30431, false);
                break;
            case 13:
                me->CastSpell(me, 30428, true);
                me->ForcedDespawn(2000);
                break;
        }
    }
    
    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_stillpine_ancestor_tikti(Creature* pCreature)
{
    npc_stillpine_ancestor_tiktiAI* thisAI = new npc_stillpine_ancestor_tiktiAI(pCreature);

    thisAI->AddWaypoint(0, -3879.668, -13111.49, 5.632213);
    thisAI->AddWaypoint(1, -3885.67, -13096.5, 3.56423, 3000);
    thisAI->AddWaypoint(2, -3905.81, -13079.04, 0.28126, 1000);
    thisAI->AddWaypoint(3, -3913.36, -13055.2, -4.82595);
    thisAI->AddWaypoint(4, -3951.4, -13033.9, -8.45508);
    thisAI->AddWaypoint(5, -4001.27, -13004.5, -6.786);
    thisAI->AddWaypoint(6, -4083.9, -12990.5, -3.210524);
    thisAI->AddWaypoint(7, -4159.045, -12992.1, -1.9385);
    thisAI->AddWaypoint(8, -4202.63, -13022.8, -0.860907);
    thisAI->AddWaypoint(9, -4290.32, -13055.33, -1.896845);
    thisAI->AddWaypoint(10, -4363.34, -13017.06, -1.917168);
    thisAI->AddWaypoint(11, -4430.19, -12998.4, -1.98677);
    thisAI->AddWaypoint(12, -4563.03, -13067.091, -12.903808);
    thisAI->AddWaypoint(13, -4635.5, -13068.6, -13.281);

    return (CreatureAI*)thisAI;
}

bool QuestAccept_npc_totem_of_tikti(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 9541)
    {
        Unit* coo = FindCreature(17392, 20, pCreature);
        if(!coo)
        {
            if(Creature* spirit = pCreature->SummonCreature(17392, -3872.551, -13129.15, 7.134583, 1.665, TEMPSUMMON_TIMED_DESPAWN, 240000))
            {
                spirit->MonsterTextEmote(-1200316, 0);
                 spirit->CastSpell(spirit, 30428, true);
                if (npc_escortAI* pEscortAI = CAST_AI(npc_stillpine_ancestor_tiktiAI, spirit->AI()))
                    pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);
            }
        }
    }
    return true;
}

struct npc_stillpine_ancestor_yorAI : public npc_escortAI
{
    npc_stillpine_ancestor_yorAI(Creature *c) : npc_escortAI(c){}

    void Reset()
    {
    }

    void WaypointReached(uint32 wp)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(wp)
        {
            case 1:
                me->SetDisplayId(16995);
                break;
            case 2:
                DoScriptText(-1230078, me, 0);
                break;
            case 3:
                DoScriptText(-1230079, me, 0);
                SetRun(true);
                me->CastSpell(me, 30447, false);
                me->CastSpell(pPlayer, 30448, false);
                break;
            case 65:
                DoScriptText(-1230080, me, pPlayer);
                pPlayer->RemoveAurasDueToSpell(30448);
                me->ForcedDespawn(2000);
                break;
        }
    }
    
    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_stillpine_ancestor_yor(Creature* pCreature)
{
    npc_stillpine_ancestor_yorAI* thisAI = new npc_stillpine_ancestor_yorAI(pCreature);

    thisAI->AddWaypoint(0, -4637.45, -13065.41, -11.657);
    thisAI->AddWaypoint(1, -4646.63, -13015.4, -1.38673);
    thisAI->AddWaypoint(2, -4679.14,-12985,0.5803, 3000);
    thisAI->AddWaypoint(3, -4679.14, -12985, 0.5803, 2000);
    thisAI->AddWaypoint(4, -4680.61,-12963.3,2.66436);
    thisAI->AddWaypoint(5, -4681.2,-12949.7,5.54);
    thisAI->AddWaypoint(6, -4682.73,-12929.5,3.47035);
    thisAI->AddWaypoint(7, -4667.73,-12909.8,1.50962);
    thisAI->AddWaypoint(8, -4647.96,-12887.9,2.97161);
    thisAI->AddWaypoint(9, -4639.09,-12877.2,4.9471);
    thisAI->AddWaypoint(10, -4630.71,-12855.1,4.28118);
    thisAI->AddWaypoint(11, -4620.17,-12829.9,6.06746);
    thisAI->AddWaypoint(12, -4612,-12814,7.12509);
    thisAI->AddWaypoint(13, -4601.96,-12795.7,3.27105);
    thisAI->AddWaypoint(14, -4588.53,-12774.2,7.44323);
    thisAI->AddWaypoint(15, -4575.77,-12760.8,6.10979);
    thisAI->AddWaypoint(16, -4563.73,-12744.2,9.99685);
    thisAI->AddWaypoint(17,-4554.96,-12729,12.2105);
    thisAI->AddWaypoint(18, -4547.41,-12712.1,9.81412);
    thisAI->AddWaypoint(19, -4538.2,-12689.2,12.5838);
    thisAI->AddWaypoint(20, -4533.22,-12668.1,16.8651);
    thisAI->AddWaypoint(21, -4532.96,-12656.8,15.2664);
    thisAI->AddWaypoint(22, -4532.78,-12637.9,16.5789);
    thisAI->AddWaypoint(23, -4533.02,-12619.1,12.0263);
    thisAI->AddWaypoint(24, -4532.77,-12592.5,14.642);
    thisAI->AddWaypoint(25, -4535.16,-12572.5,11.9603);
    thisAI->AddWaypoint(26, -4521.38,-12547.2,8.20642);
    thisAI->AddWaypoint(27, -4506.74,-12520.5,2.71515);
    thisAI->AddWaypoint(28, -4492.57,-12495.4,4.36968);
    thisAI->AddWaypoint(29, -4476.92,-12469.2,2.26036);
    thisAI->AddWaypoint(30, -4462.38,-12438.4,2.68436);
    thisAI->AddWaypoint(31, -4430.42,-12442,2.38524);
    thisAI->AddWaypoint(32, -4419.91,-12429.3,3.2091);
    thisAI->AddWaypoint(33, -4408.61,-12400.1,5.02559);
    thisAI->AddWaypoint(34, -4414.42,-12370.4,5.99229);
    thisAI->AddWaypoint(35, -4423.44,-12345,8.29463);
    thisAI->AddWaypoint(36, -4435.58,-12315.7,10.3162);
    thisAI->AddWaypoint(37, -4457.19,-12304.2,12.3064);
    thisAI->AddWaypoint(38, -4475.24,-12294,13.9122);
    thisAI->AddWaypoint(39, -4490.89,-12279.7,15.2792);
    thisAI->AddWaypoint(40, -4503.42,-12249.6,16.3871);
    thisAI->AddWaypoint(41, -4510.15,-12229.8,17.1362);
    thisAI->AddWaypoint(42, -4513.27,-12209.5,16.9777);
    thisAI->AddWaypoint(43, -4513.64,-12189.1,16.9593);
    thisAI->AddWaypoint(44, -4502.99,-12158,16.1291, 2000);
    thisAI->AddWaypoint(45, -4508.94,-12122.7,16.9898);
    thisAI->AddWaypoint(46, -4505.31,-12095.6,18.8873);
    thisAI->AddWaypoint(47, -4500.36,-12071.2,21.4168);
    thisAI->AddWaypoint(48, -4516.81,-12050.5,24.2492);
    thisAI->AddWaypoint(49, -4528.22,-12037.5,26.0347);
    thisAI->AddWaypoint(50, -4531.9,-12008.6,28.4037);
    thisAI->AddWaypoint(51, -4538.61,-11984.4,29.7635);
    thisAI->AddWaypoint(52, -4543.79,-11963.5,29.1954);
    thisAI->AddWaypoint(53, -4537.48,-11934.2,27.0094);
    thisAI->AddWaypoint(54, -4533.18,-11906.3,22.686);
    thisAI->AddWaypoint(55, -4504.71,-11879,17.5661);
    thisAI->AddWaypoint(56, -4500.36,-11845.7,15.0063);
    thisAI->AddWaypoint(57, -4510.63,-11816.8,13.8306);
    thisAI->AddWaypoint(58, -4531.7,-11786.6,15.5384);
    thisAI->AddWaypoint(59, -4555.94,-11756.9,17.6289);
    thisAI->AddWaypoint(60, -4546.7,-11735.9,19.533);
    thisAI->AddWaypoint(61, -4535.96,-11712.4,18.2368);
    thisAI->AddWaypoint(62, -4519.88,-11702.4,17.8815);
    thisAI->AddWaypoint(63, -4507,-11694.5,13.2184);
    thisAI->AddWaypoint(64, -4490.14,-11673,10.8723);
    thisAI->AddWaypoint(65, -4486.34,-11658,10.6353);

    return (CreatureAI*)thisAI;
}

bool QuestAccept_npc_totem_of_yor(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 9542)
    {
        Unit* coo = FindCreature(17393, 20, pCreature);
        if(!coo)
        {
            if(Creature* spirit = pCreature->SummonCreature(17393, -4637.05,-13067.3,-13.5973, 0.26767, TEMPSUMMON_TIMED_DESPAWN, 360000))
            {
                DoScriptText(-1230081, spirit, pPlayer);
                spirit->CastSpell(spirit, 30428, true);
                if (npc_escortAI* pEscortAI = CAST_AI(npc_stillpine_ancestor_yorAI, spirit->AI()))
                    pEscortAI->Start(true, false, pPlayer->GetGUID(), quest);
            }
        }
    }
    return true;
}

/*######
## npc_17311
######*/

bool GossipHello_npc_17311(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );


    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16268), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    player->SEND_GOSSIP_MENU(8870, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_17311(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16269), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(8871, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16270), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(8872, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}

struct npc_17311AI : public ScriptedAI
{
    npc_17311AI(Creature *c) : ScriptedAI(c) {}

    Timer EmotionTimer;

    void Reset()
    {
        EmotionTimer.Reset(60000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(EmotionTimer.Expired(diff))
        {
            me->HandleEmoteCommand(20);
            EmotionTimer = 60000;
        }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_17311(Creature *_Creature)
{
    return new npc_17311AI (_Creature);
}

/*######
## mob_surveyor_candress
######*/

#define SPELL_FIREBOLT  9053
#define SPELL_RED_BEAM  30944
#define SAY_ATTACK      -1910206

struct mob_surveyor_candressAI : public ScriptedAI
{
    mob_surveyor_candressAI(Creature *c) : ScriptedAI(c) {}

    Timer FireboltTimer;
    Timer RedBeamTimer;

    void Reset()
    {
        FireboltTimer.Reset(1000); 
        RedBeamTimer.Reset(1000);
    }

    void EnterCombat(Unit *who) 
    {
        if(Unit *redCrystalBunny = (Unit*)FindCreature(17947, 15.0f, m_creature))
        {
            if (redCrystalBunny->HasAura(SPELL_RED_BEAM, 0))
                redCrystalBunny->RemoveAurasDueToSpell(SPELL_RED_BEAM);
        }

        m_creature->InterruptNonMeleeSpells(true);
        DoScriptText(SAY_ATTACK, m_creature);
    }


    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if (RedBeamTimer.Expired(diff))
            {
                if(Unit *redCrystalBunny = (Unit*)FindCreature(17947, 15.0f, m_creature))
                    m_creature->CastSpell(redCrystalBunny, SPELL_RED_BEAM, false);
                
                RedBeamTimer = urand(30000, 60000);
            }

            return;
        }

        if (FireboltTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FIREBOLT);
            FireboltTimer = urand(3000, 4000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};
CreatureAI* GetAI_mob_surveyor_candress(Creature *_Creature)
{
    return new mob_surveyor_candressAI (_Creature);
}

/*######
## npc_stillpine_the_younger
######*/

#define QUEST_CHIEFTAIN_OOMOOROO    9573
#define MOB_STILLPINE_RAIDER        17495
#define SAY_START_EVENT             -1001168
#define WHISPER_START_EVENT         -1200317


struct npc_stillpine_the_youngerAI : public ScriptedAI
{
    npc_stillpine_the_youngerAI(Creature* creature) : ScriptedAI(creature),  summons(creature) {}
    
    SummonList summons;

    Timer EventTimer;
    uint32 PlayerGUID;
    uint32 MorphNumber;
    uint32 Phase;
    bool EventStarted;

    void Reset()
    {
        EventTimer.Reset(2000);
        PlayerGUID = 0;
        MorphNumber = 0;
        Phase = 0;
        EventStarted = false;
    }

    void DoSummon()
    {
        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3368.74, -12420.82, 25.22, 2.09,  
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);
        
        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3360.43, -12426.46, 27.18, 3.09,
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);

        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3362.11, -12434.49, 25.63, 1.09,
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);
        
        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3353.78, -12434.55, 30.55, 3.09,
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);
        
        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3355.85, -12441.45, 30.11, 1.09,
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);

        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3349.57, -12439.4, 32.89, 3.09,
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);
        
        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3352.96, -12444.0, 32.06, 1.09,
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);
        
        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3346.05, -12441.06, 35.16, 3.09, 
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);

        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3349.05, -12449.31, 36.48, 1.09, 
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);
        
        m_creature->SummonCreature(MOB_STILLPINE_RAIDER, -3341.72, -12449.42, 40.46, 2.09, 
                                    TEMPSUMMON_TIMED_DESPAWN, 2*MINUTE*MILLISECONDS);                                                                                                       
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if (eventType == 5) 
        {
            PlayerGUID = invoker->GetGUID();

            if (!EventStarted)
            {
                DoSummon();
                DoScriptText(SAY_START_EVENT, m_creature);

                EventStarted = true;
            }
            else 
                m_creature->Whisper(-1200317, PlayerGUID); 
        }
    }

    void JustSummoned(Creature* pSummoned) 
    {
        summons.Summon(pSummoned);

        switch(MorphNumber)
        {
            case 0:
                pSummoned->SetDisplayId(6819);
                break;
            case 2:
                pSummoned->SetDisplayId(6819);
                break;
            case 4:
                pSummoned->SetDisplayId(3024);
                break;
             case 5:
                pSummoned->SetDisplayId(6819);
                break;   
            case 7:
                pSummoned->SetDisplayId(6819);
                break;
            default:
                break;                             
        }
        MorphNumber++;

    }
     void SummonedCreatureDies(Creature* pSummoned, Unit* /*killer*/) 
     {
        if (!pSummoned)
            return;
        else
            summons.Despawn(pSummoned);
     }

    void UpdateAI(const uint32 diff)
    {   
        if(EventStarted)
        {
            if(EventTimer.Expired(diff))
            {
				Player* pInvoker = me->GetPlayerInWorld(PlayerGUID);
				
				if (Phase == 0)
                {
                    for (SummonList::const_iterator i = summons.begin(); i != summons.end(); i++)
                    {
                        if (Creature* sum = m_creature->GetCreature(*i))
                            m_creature->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, (Unit*)pInvoker, sum);
                    }

                    Phase++;
                    EventTimer.Reset(2*MINUTE*MILLISECONDS);
                }
                else
                {
                    if (!summons.empty())
                    {
                        for (SummonList::const_iterator i = summons.begin(); i != summons.end(); i++)
                        {
                            if (Creature* sum = m_creature->GetCreature(*i))
                                m_creature->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_B, (Unit*)pInvoker, sum);
                        }
                    }
                    
                    Reset();
                }
            }
        }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_stillpine_the_younger(Creature* creature)
{
    return new npc_stillpine_the_youngerAI(creature);
}

bool QuestRewarded_npc_stillpine_the_younger(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CHIEFTAIN_OOMOOROO)
        pCreature->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, pPlayer, pCreature);

    return true;
}


/*######
## mob_stillpine_raider
######*/

#define SPELL_HEROIC_STRIKE 25710

struct mob_stillpine_raiderAI : public npc_escortAI
{
   mob_stillpine_raiderAI(Creature *c) : npc_escortAI(c) {}

    Timer HeroicStrikeTimer;

    void Reset()
    {
        HeroicStrikeTimer.Reset(1000);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if (eventType == AI_EVENT_CUSTOM_EVENTAI_A) 
        {
            if (npc_escortAI* pEscortAI = CAST_AI(mob_stillpine_raiderAI, (m_creature->AI())))
            {
                pEscortAI->Start(true, true);
                pEscortAI->SetDespawnAtEnd(true);
                pEscortAI->SetDespawnAtFar(false);
            }
        }

        if (eventType == AI_EVENT_CUSTOM_EVENTAI_B)
        {
            m_creature->ForcedDespawn(1000);
        }
    }

    void WaypointReached(uint32 i) {}

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        if (HeroicStrikeTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_HEROIC_STRIKE, true);

            HeroicStrikeTimer = urand(9000, 14000);
        }

        DoMeleeAttackIfReady();
    }

};
CreatureAI* GetAI_mob_stillpine_raider(Creature *_Creature)
{
    mob_stillpine_raiderAI* stillpine_raiderAI = new mob_stillpine_raiderAI(_Creature);

    stillpine_raiderAI->AddWaypoint(0, -3395.55, -12390.18, 18.9);
    stillpine_raiderAI->AddWaypoint(1, -3398.76, -12359.09, 18.27);
    stillpine_raiderAI->AddWaypoint(2, -3381.20, -12349.01, 21.93);
    stillpine_raiderAI->AddWaypoint(3, -3319.78, -12345.76, 24.18);
    stillpine_raiderAI->AddWaypoint(4, -3273.41, -12350.01, 17.66);
    stillpine_raiderAI->AddWaypoint(5, -3257.68, -12360.50, 13.09);
    stillpine_raiderAI->AddWaypoint(6, -3238.29, -12374.42, 9.65);
    stillpine_raiderAI->AddWaypoint(7, -3238.29, -12374.42, 9.65);
    stillpine_raiderAI->AddWaypoint(8, -3232.61, -12392.33, 9.7);
    stillpine_raiderAI->AddWaypoint(9, -3232.61, -12392.33, 9.7);
    stillpine_raiderAI->AddWaypoint(10, -3210.19, -12404.44, 6.57);
    stillpine_raiderAI->AddWaypoint(11, -3200.67, -12416.13, 2.59);
    stillpine_raiderAI->AddWaypoint(12, -3197.73, -12435.87, 1.18);
    stillpine_raiderAI->AddWaypoint(13, -3179.38, -12454.87, -0.61);
    stillpine_raiderAI->AddWaypoint(14, -3179.38, -12454.87, -0.61);
    stillpine_raiderAI->AddWaypoint(15, -3156.88, -12477.03, -2.25);
    stillpine_raiderAI->AddWaypoint(16, -3156.88, -12477.03, -2.25);
    stillpine_raiderAI->AddWaypoint(17, -3151.2, -12499.85, -0.52);
    stillpine_raiderAI->AddWaypoint(18, -3151.2, -12499.85, -0.52);
    
    return (CreatureAI*)stillpine_raiderAI;
}


/*#####
# npc_17058
######*/
#define NPC_GALAEN  17426

struct npc_17058AI : public ScriptedAI
{
    npc_17058AI(Creature* creature) : ScriptedAI(creature) {}
    
    Timer EventTimer;
    uint32 playerGUID;
    uint8 phase;

    void Reset()
    {
        EventTimer.Reset(0);
        playerGUID = 0;
        phase = 0;
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if (eventType == AI_EVENT_CUSTOM_EVENTAI_A) 
        {
            EventTimer = 1000;
            playerGUID = invoker->GetGUID();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventTimer.Expired(diff))
        {
            switch(phase)
            {
                case 0:
                    m_creature->SummonCreature(NPC_GALAEN, -2090.63, -11298.43, 63.36, 3.76, TEMPSUMMON_TIMED_DESPAWN, 10000);
                    phase++;
                    EventTimer = 1000;
                    break;
                case 1:
                    if (Creature *sum = (Creature*)FindCreature(NPC_GALAEN, 20, m_creature))
                        if (Player *plr = Player::GetPlayerInWorld(playerGUID))
                            sum->SetFacingTo(sum->GetOrientationTo(plr));

                    Reset();       
                    return;
            }
        }

        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_17058(Creature* creature)
{
    return new npc_17058AI(creature);
}

bool QuestAccept_npc_17058(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 9579)
         pCreature->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, pPlayer, pCreature);

    return true;
}

void AddSC_azuremyst_isle()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mob_stillpine_raider";
    newscript->GetAI = &GetAI_mob_stillpine_raider;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_stillpine_the_younger";
    newscript->GetAI = &GetAI_npc_stillpine_the_younger;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_stillpine_the_younger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_sethir_the_ancient";
    newscript->GetAI = &GetAI_npc_sethir_the_ancient;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_draenei_survivor";
    newscript->GetAI = &GetAI_npc_draenei_survivor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_engineer_spark_overgrind";
    newscript->GetAI = &GetAI_npc_engineer_spark_overgrind;
    newscript->pGossipHello =  &GossipHello_npc_engineer_spark_overgrind;
    newscript->pGossipSelect = &GossipSelect_npc_engineer_spark_overgrind;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_injured_draenei";
    newscript->GetAI = &GetAI_npc_injured_draenei;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_magwin";
    newscript->GetAI = &GetAI_npc_magwinAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_magwin;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_susurrus";
    newscript->pGossipHello =  &GossipHello_npc_susurrus;
    newscript->pGossipSelect = &GossipSelect_npc_susurrus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_geezle";
    newscript->GetAI = &GetAI_npc_geezleAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_nestlewood_owlkin";
    newscript->GetAI = &GetAI_mob_nestlewood_owlkinAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_siltfin_murloc";
    newscript->GetAI = &GetAI_mob_siltfin_murlocAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_death_ravager";
    newscript->GetAI = &GetAI_npc_death_ravagerAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_ravager_cage";
    newscript->pGOUse = &go_ravager_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_stillpine_capitive";
    newscript->GetAI = &GetAI_npc_stillpine_capitiveAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_bristlelimb_cage";
    newscript->pGOUse = &go_bristlelimb_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_tracker";
    newscript->GetAI = &GetAI_npc_tracker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_stillpine_ancestor_akida";
    newscript->GetAI = &GetAI_npc_stillpine_ancestor_akida;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_totem_of_akida";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_totem_of_akida;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_stillpine_ancestor_coo";
    newscript->GetAI = &GetAI_npc_stillpine_ancestor_coo;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_totem_of_coo";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_totem_of_coo;
    newscript->RegisterSelf();


    newscript = new Script;
    newscript->Name="npc_stillpine_ancestor_tikti";
    newscript->GetAI = &GetAI_npc_stillpine_ancestor_tikti;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_totem_of_tikti";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_totem_of_tikti;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_stillpine_ancestor_yor";
    newscript->GetAI = &GetAI_npc_stillpine_ancestor_yor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_totem_of_yor";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_totem_of_yor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_17311";
    newscript->pGossipHello =  &GossipHello_npc_17311;
    newscript->pGossipSelect = &GossipSelect_npc_17311;
    newscript->GetAI = &GetAI_npc_17311;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_surveyor_candress";
    newscript->GetAI = &GetAI_mob_surveyor_candress;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_17058";
    newscript->GetAI = &GetAI_npc_17058;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_17058;
    newscript->RegisterSelf();
}
