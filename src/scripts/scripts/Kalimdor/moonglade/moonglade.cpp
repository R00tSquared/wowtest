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
SDName: Moonglade
SD%Complete: 100
SDComment: Quest support: 30, 272, 5929, 5930, 10965. Special Flight Paths for Druid class, 8736.
SDCategory: Moonglade
EndScriptData */

/* ContentData
npc_bunthen_plainswind
npc_great_bear_spirit
npc_silva_filnaveth
npc_clintar_spirit
npc_clintar_dreamwalker
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_bunthen_plainswind
######*/

#define GOSSIP_BP1 16313
#define GOSSIP_BP2 16314

bool GossipHello_npc_bunthen_plainswind(Player *player, Creature *_Creature)
{
    if(player->GetClass() != CLASS_DRUID)
        player->SEND_GOSSIP_MENU(4916,_Creature->GetGUID());
    else if(player->GetTeam() != HORDE)
    {
        if(player->GetQuestStatus(272) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_BP1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        player->SEND_GOSSIP_MENU(4917,_Creature->GetGUID());
    }
    else if(player->GetClass() == CLASS_DRUID && player->GetTeam() == HORDE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_BP2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if(player->GetQuestStatus(30) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_BP1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        player->SEND_GOSSIP_MENU(4918,_Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_bunthen_plainswind(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            player->CLOSE_GOSSIP_MENU();
            if (player->GetClass() == CLASS_DRUID && player->GetTeam() == HORDE)
            {
                std::vector<uint32> nodes;

                nodes.resize(2);
                nodes[0] = 63;                              // Nighthaven, Moonglade
                nodes[1] = 22;                              // Thunder Bluff, Mulgore
                player->ActivateTaxiPathTo(nodes);
            }
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->SEND_GOSSIP_MENU(5373,_Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->SEND_GOSSIP_MENU(5376,_Creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_great_bear_spirit
######*/

#define GOSSIP_BEAR1 16315
#define GOSSIP_BEAR2 16316
#define GOSSIP_BEAR3 16317
#define GOSSIP_BEAR4 16318

bool GossipHello_npc_great_bear_spirit(Player *player, Creature *_Creature)
{
    //ally or horde quest
    if (player->GetQuestStatus(5929) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(5930) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_BEAR1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(4719, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(4718, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_great_bear_spirit(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_BEAR2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(4721, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_BEAR3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(4733, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_BEAR4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(4734, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->SEND_GOSSIP_MENU(4735, _Creature->GetGUID());
            if (player->GetQuestStatus(5929)==QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(5929);
            if (player->GetQuestStatus(5930)==QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(5930);
            break;
    }
    return true;
}

/*######
## npc_silva_filnaveth
######*/

#define GOSSIP_SF1 16319
#define GOSSIP_SF2 16320
bool GossipHello_npc_silva_filnaveth(Player *player, Creature *_Creature)
{
    if(player->GetClass() != CLASS_DRUID)
        player->SEND_GOSSIP_MENU(4913,_Creature->GetGUID());
    else if(player->GetTeam() != ALLIANCE)
    {
        if(player->GetQuestStatus(30) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SF1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        player->SEND_GOSSIP_MENU(4915,_Creature->GetGUID());
    }
    else if(player->GetClass() == CLASS_DRUID && player->GetTeam() == ALLIANCE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SF2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if(player->GetQuestStatus(272) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SF1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        player->SEND_GOSSIP_MENU(4914,_Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_silva_filnaveth(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            player->CLOSE_GOSSIP_MENU();
            if (player->GetClass() == CLASS_DRUID && player->GetTeam() == ALLIANCE)
            {
                std::vector<uint32> nodes;

                nodes.resize(2);
                nodes[0] = 62;                              // Nighthaven, Moonglade
                nodes[1] = 27;                              // Rut'theran Village, Teldrassil
                player->ActivateTaxiPathTo(nodes);
            }
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->SEND_GOSSIP_MENU(5374,_Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->SEND_GOSSIP_MENU(5375,_Creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_clintar_spirit
######*/

float Clintar_spirit_WP[41][5] =
{
 //pos_x   pos_y    pos_z    orien waitTime
{7465.28, -3115.46, 439.327, 0.83, 4000},
{7476.49, -3101,    443.457, 0.89, 0},
{7486.57, -3085.59, 439.478, 1.07, 0},
{7472.19, -3085.06, 443.142, 3.07, 0},
{7456.92, -3085.91, 438.862, 3.24, 0},
{7446.68, -3083.43, 438.245, 2.40, 0},
{7446.17, -3080.21, 439.826, 1.10, 6000},
{7452.41, -3085.8,  438.984, 5.78, 0},
{7469.11, -3084.94, 443.048, 6.25, 0},
{7483.79, -3085.44, 439.607, 6.25, 0},
{7491.14, -3090.96, 439.983, 5.44, 0},
{7497.62, -3098.22, 436.854, 5.44, 0},
{7498.72, -3113.41, 434.596, 4.84, 0},
{7500.06, -3122.51, 434.749, 5.17, 0},
{7504.96, -3131.53, 434.475, 4.74, 0},
{7504.31, -3133.53, 435.693, 3.84, 6000},
{7504.55, -3133.27, 435.476, 0.68, 15000},
{7501.99, -3126.01, 434.93,  1.83, 0},
{7490.76, -3114.97, 434.431, 2.51, 0},
{7479.64, -3105.51, 431.123, 1.83, 0},
{7474.63, -3086.59, 428.994, 1.83, 2000},
{7472.96, -3074.18, 427.566, 1.57, 0},
{7472.25, -3063,    428.268, 1.55, 0},
{7473.46, -3054.22, 427.588, 0.36, 0},
{7475.08, -3053.6,  428.653, 0.36, 6000},
{7474.66, -3053.56, 428.433, 3.19, 4000},
{7471.81, -3058.84, 427.073, 4.29, 0},
{7472.16, -3064.91, 427.772, 4.95, 0},
{7471.56, -3085.36, 428.924, 4.72, 0},
{7473.56, -3093.48, 429.294, 5.04, 0},
{7478.94, -3104.29, 430.638, 5.23, 0},
{7484.46, -3109.61, 432.769, 5.79, 0},
{7490.23, -3111.08, 434.431, 0.02, 0},
{7496.29, -3108,    434.783, 1.15, 0},
{7497.46, -3100.66, 436.191, 1.50, 0},
{7495.64, -3093.39, 438.349, 2.10, 0},
{7492.44, -3086.01, 440.267, 1.38, 0},
{7498.26, -3076.44, 440.808, 0.71, 0},
{7506.4,  -3067.35, 443.64,  0.77, 0},
{7518.37, -3057.42, 445.584, 0.74, 0},
{7517.51, -3056.3,  444.568, 2.49, 4500}
};

#define ASPECT_RAVEN 22915

#define ASPECT_RAVEN_SUMMON_X 7472.96
#define ASPECT_RAVEN_SUMMON_Y -3074.18
#define ASPECT_RAVEN_SUMMON_Z 427.566
#define CLINTAR_SPIRIT_SUMMON_X 7459.2275
#define CLINTAR_SPIRIT_SUMMON_Y -3122.5632
#define CLINTAR_SPIRIT_SUMMON_Z 438.9842
#define CLINTAR_SPIRIT_SUMMON_O 0.8594

#define CLINTAR_SPIRIT_SAY_START -1000286
#define CLINTAR_SPIRIT_SAY_UNDER_ATTACK_1 -1000287
#define CLINTAR_SPIRIT_SAY_UNDER_ATTACK_2 -1000288
#define CLINTAR_SPIRIT_SAY_GET_ONE -1000289
#define CLINTAR_SPIRIT_SAY_GET_TWO -1000290
#define CLINTAR_SPIRIT_SAY_GET_THREE -1000291
#define CLINTAR_SPIRIT_SAY_GET_FINAL -1000292

struct npc_clintar_spiritAI : public npc_escortAI
{
    npc_clintar_spiritAI(Creature *c) : npc_escortAI(c) {}

    uint32 Step;
    uint32 CurrWP;
    uint32 Event_Timer;
    uint32 checkPlayer_Timer;

    uint64 PlayerGUID;

    bool Event_onWait;

    void Reset()
    {
        if(!PlayerGUID)
        {
            Step = 0;
            CurrWP = 0;
            Event_Timer = 0;
            PlayerGUID = 0;
            checkPlayer_Timer = 3000;
            Event_onWait = false;
        }
    }

    void JustDied(Unit *killer)
    {
        if(!PlayerGUID)
            return;

        Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID);
        if (pPlayer && pPlayer->GetQuestStatus(10965) == QUEST_STATUS_INCOMPLETE)
        {
            pPlayer->FailQuest(10965);
            PlayerGUID = 0;
            Reset();
        }
    }

    void EnterEvadeMode()
    {
        Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID);
        if (pPlayer && pPlayer->IsInCombat() && pPlayer->GetAttackerForHelper())
        {
            AttackStart(pPlayer->GetAttackerForHelper());
            return;
        }
        npc_escortAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* who)
    {
        uint32 rnd = rand()%2;
        switch(rnd)
        {
            case 0: DoScriptText(CLINTAR_SPIRIT_SAY_UNDER_ATTACK_1, m_creature, who); break;
            case 1: DoScriptText(CLINTAR_SPIRIT_SAY_UNDER_ATTACK_2, m_creature, who); break;
        }
    }

    void StartEvent(Player* pPlayer)
    {
        if(!pPlayer)
            return;

        if(pPlayer->GetQuestStatus(10965) == QUEST_STATUS_INCOMPLETE)
        {
            for(uint8 i = 0; i < 41; ++i)
            {
                AddWaypoint(i, Clintar_spirit_WP[i][0], Clintar_spirit_WP[i][1], Clintar_spirit_WP[i][2], (uint32)Clintar_spirit_WP[i][4]);
            }
            PlayerGUID = pPlayer->GetGUID();
            Start(true,false,PlayerGUID);
        }
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        /*if(!PlayerGUID)
        {
            m_creature->setDeathState(JUST_DIED);
            return;
        }*/

        if (!m_creature->IsInCombat() && !Event_onWait)
        {
            if (checkPlayer_Timer < diff)
            {
                Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID);
                if (pPlayer && pPlayer->IsInCombat() && pPlayer->GetAttackerForHelper())
                    AttackStart(pPlayer->GetAttackerForHelper());

                checkPlayer_Timer = 1000;
            }
            else
                checkPlayer_Timer -= diff;
        }

        if(Event_onWait && Event_Timer < diff)
        {
            Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID);
            if (!pPlayer || (pPlayer && pPlayer->GetQuestStatus(10965) == QUEST_STATUS_NONE))
            {
                m_creature->setDeathState(JUST_DIED);
                return;
            }

            switch(CurrWP)
            {
                case 0:
                    switch(Step)
                    {
                        case 0:
                            m_creature->Say(CLINTAR_SPIRIT_SAY_START,0,PlayerGUID);
                            Event_Timer = 8000;
                            Step = 1;
                            break;
                        case 1:
                            Event_onWait = false;
                            break;
                    }
                    break;
                case 6:
                    switch(Step)
                    {
                        case 0:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 133);
                            Event_Timer = 5000;
                            Step = 1;
                            break;
                        case 1:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                            DoScriptText(CLINTAR_SPIRIT_SAY_GET_ONE, m_creature, pPlayer);
                            Event_onWait = false;
                            break;
                    }
                    break;
                case 15:
                    switch(Step)
                    {
                        case 0:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 133);
                            Event_Timer = 5000;
                            Step = 1;
                            break;
                        case 1:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                            Event_onWait = false;
                            break;
                    }
                    break;
                case 16:
                    switch(Step)
                    {
                        case 0:
                            DoScriptText(CLINTAR_SPIRIT_SAY_GET_TWO, m_creature, pPlayer);
                            Event_Timer = 15000;
                            Step = 1;
                            break;
                        case 1:
                            Event_onWait = false;
                            break;
                    }
                    break;
                case 20:
                    switch(Step)
                    {
                        case 0:
                            {
                            Creature *mob = m_creature->SummonCreature(ASPECT_RAVEN, ASPECT_RAVEN_SUMMON_X, ASPECT_RAVEN_SUMMON_Y, ASPECT_RAVEN_SUMMON_Z, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000);
                            if(mob)
                            {
                                mob->AddThreat(m_creature,10000.0f);
                                mob->AI()->AttackStart(m_creature);
                            }
                            Event_Timer = 2000;
                            Step = 1;
                            break;
                            }
                        case 1:
                            Event_onWait = false;
                            break;
                    }
                    break;
                case 24:
                    switch(Step)
                    {
                        case 0:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 133);
                            Event_Timer = 5000;
                            Step = 1;
                            break;
                        case 1:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                            Event_onWait = false;
                            break;
                    }
                    break;
                case 25:
                    switch(Step)
                    {
                        case 0:
                            DoScriptText(CLINTAR_SPIRIT_SAY_GET_THREE, m_creature, pPlayer);
                            Event_Timer = 4000;
                            Step = 1;
                            break;
                        case 1:
                            Event_onWait = false;
                            break;
                    }
                    break;
                case 40:
                    switch(Step)
                    {
                        case 0:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 2);
                            DoScriptText(CLINTAR_SPIRIT_SAY_GET_FINAL, m_creature, pPlayer);
                            pPlayer->CompleteQuest(10965);
                            Event_Timer = 1500;
                            Step = 1;
                            break;
                        case 1:
                            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                            Event_Timer = 3000;
                            Step = 2;
                            break;
                        case 2:
                            pPlayer->TalkedToCreature(m_creature->GetEntry(), m_creature->GetGUID());
                            PlayerGUID = 0;
                            Reset();
                            m_creature->setDeathState(JUST_DIED);
                            break;
                    }
                    break;
                default:
                    Event_onWait = false;
                    break;
            }

        } else if(Event_onWait) Event_Timer -= diff;
    }

    void WaypointReached(uint32 id)
    {
        CurrWP = id;
        Event_Timer = 0;
        Step = 0;
        Event_onWait = true;
    }
};

CreatureAI* GetAI_npc_clintar_spirit(Creature *_Creature)
{
    return new npc_clintar_spiritAI (_Creature);
}

struct npc_22834AI : public ScriptedAI
{
    npc_22834AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer EventTimer;
    uint8 EventNumber;
    uint64 PlayerGUID;

    void Reset()
    {
        EventTimer.Reset(0);
        EventNumber = 0;
        PlayerGUID = 0;
        me->SetWalk(true);
    }

    void EventStart(Player* pPlayer)
    {
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
        DoTextEmote(-1200411, NULL);
        EventTimer = 3000;
        PlayerGUID = pPlayer->GetGUID();
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventTimer.Expired(diff))
        {
            switch(EventNumber)
            {
                case 0:
                    me->Say(-1200412, LANG_UNIVERSAL, 0);
                    me->HandleEmoteCommand(5);
                    EventTimer = 3000;
                    EventNumber++;
                    break;
                case 1:
                    me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
                    DoTextEmote(-1200413, NULL);
                    EventTimer = 5000;
                    EventNumber++;
                    break;
                case 2:
                    me->Say(-1200414, LANG_UNIVERSAL, 0);
                    EventTimer = 4000;
                    EventNumber++;
                    break;
                case 3:
                    me->GetMotionMaster()->MovePoint(1, 7453.25,-3116.29,439.604,5.534);
                    EventTimer = 4000;
                    EventNumber++;
                    break;
                case 4:
                    me->GetMotionMaster()->MovePoint(2, 7453.79,-3116.83,439.604,5.534);
                    EventTimer = 4000;
                    EventNumber++;
                    break;
                case 5:
                    me->Say(-1200415, LANG_UNIVERSAL, PlayerGUID);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                    EventTimer = 30000;
                    EventNumber++;
                    break;
                case 6:
                    me->GetMotionMaster()->MovePoint(0, 7459.47,-3122.79,439.485,5.84588);
                    EventTimer = 10000;
                    EventNumber++;
                    break;
                case 7:
                    me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_SLEEP);
                    EventTimer = 0;
                    EventNumber = 0;
                    PlayerGUID = 0;
                    break;
            }
        }
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_22834(Creature* creature)
{
    return new npc_22834AI(creature);
}

/*####
# npc_clintar_dreamwalker
####*/

#define CLINTAR_SPIRIT 22916

bool QuestAccept_npc_clintar_dreamwalker(Player *player, Creature *creature, Quest const *quest )
{
    if(quest->GetQuestId() == 10965)
    {
        Creature *clintar_spirit = creature->SummonCreature(CLINTAR_SPIRIT, CLINTAR_SPIRIT_SUMMON_X, CLINTAR_SPIRIT_SUMMON_Y, CLINTAR_SPIRIT_SUMMON_Z, CLINTAR_SPIRIT_SUMMON_O, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 100000);
        creature->DisappearAndDie();
        if(clintar_spirit)
            ((npc_clintar_spiritAI*)clintar_spirit->AI())->StartEvent(player);
    }
    return true;
}

bool QuestComplete_npc_22834(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 10964)
    {
        if (npc_22834AI* questnpc = CAST_AI(npc_22834AI, creature->AI()))
        {
            questnpc->EventStart(player);
        }
    }
    return true;
}

/* ##
# Quest Waking Legends: 8447
### */

enum quest8447
{
    QUEST_8447       = 8447,
    REMULOS_ID       = 2550014,
    REMULOS_O_ID     = 11832,
    MALFURION_ID     = 17949,
    SPELL_1          = 25004, // Drop object on lake
    SPELL_2          = 41232, // Visual teleport
    SPELL_3          = 24999, // Shadowform aura
    EVENT_8447_1     = 1,
    EVENT_8447_2     = 2,
    EVENT_8447_3     = 3,
    EVENT_8447_4     = 4,
    EVENT_8447_5     = 5,
    EVENT_8447_6     = 6,
    EVENT_8447_7     = 7,
    EVENT_8447_8     = 8,
    EVENT_8447_9     = 9,
    EVENT_8447_10    = 10,
    EVENT_8447_11    = 11,
    EVENT_8447_12    = 12,
    EVENT_8447_13    = 13,
    EVENT_8447_14    = 14,
    EVENT_8447_15    = 15,
    EVENT_8447_16    = 16,
    EVENT_8447_17    = 17,
    EVENT_8447_18    = 18,
    EVENT_8447_19    = 19,
    EVENT_8447_20    = 20,
    EVENT_8447_21    = 21,
    EVENT_8447_22    = 22,
    EVENT_8447_23    = 23,
    EVENT_8447_24    = 24
};

struct npc_remulos_q8447AI : public ScriptedAI
{
    npc_remulos_q8447AI(Creature *c) : ScriptedAI(c) {}

    EventMap events;
    uint64 SummonerGUID;

    void Reset()
    {
        events.Reset();
        events.ScheduleEvent(EVENT_8447_1, 1000);
        m_creature->SetSpeed(MOVE_RUN, 1.2);
        m_creature->SetSpeed(MOVE_WALK, 1.2);
    }
    
    void IsSummonedBy(Unit *summoner)
    {
        me->SetOwnerGUID(summoner->GetGUID());
        SummonerGUID = summoner->GetGUID();
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_8447_1:
                {
                    me->Say(-1200416, LANG_UNIVERSAL, SummonerGUID);
                    me->GetMotionMaster()->MovePoint(0, 7828.5752,-2246.8354,463.5159);
                    events.ScheduleEvent(EVENT_8447_2, 3000);
                    break;
                }
                case EVENT_8447_2:
                {
                    me->GetMotionMaster()->MovePoint(1, 7824.6440,-2279.0273,459.3173);
                    events.ScheduleEvent(EVENT_8447_3, 5300);
                    break;
                }
                case EVENT_8447_3:
                {
                    me->GetMotionMaster()->MovePoint(2, 7814.1699,-2302.2565,456.2227);
                    events.ScheduleEvent(EVENT_8447_4, 3300);
                    break;
                }
                case EVENT_8447_4:
                {
                    me->GetMotionMaster()->MovePoint(3, 7787.4604,-2320.9807,454.5470);
                    events.ScheduleEvent(EVENT_8447_5, 4000);
                    break;
                }
                case EVENT_8447_5:
                {
                    me->GetMotionMaster()->MovePoint(4, 7753.7495,-2319.0832,454.7066, 2.92);
                    events.ScheduleEvent(EVENT_8447_6, 5000);
                    break;
                }
                case EVENT_8447_6:
                {
                    me->Say(-1200417, LANG_UNIVERSAL, SummonerGUID);
                    AddSpellToCast(me, SPELL_1);
                    events.ScheduleEvent(EVENT_8447_7, 3000);
                    break;
                }
                case EVENT_8447_7:
                {
                    Creature* Malfurion = me->SummonCreature(MALFURION_ID, 7730.5288, -2318.8596, 453.8706, 6.14985, TEMPSUMMON_DEAD_DESPAWN, 100);
                    Malfurion->CastSpell(Malfurion, SPELL_2, false);
                    Malfurion->setHover(true);
                    Malfurion->SetLevitate(true);
                    Malfurion->CastSpell(Malfurion, SPELL_3, false);
                    DoSay("Malfurion!", LANG_UNIVERSAL, 0);
                    events.ScheduleEvent(EVENT_8447_8, 1500);
                    break;
                }
                case EVENT_8447_8:
                {
                    if (Creature* Malfurion = GetClosestCreatureWithEntry(me, MALFURION_ID, 30))
                    {
                        Malfurion->Say(-1200418, LANG_UNIVERSAL, 0);
                        events.ScheduleEvent(EVENT_8447_9, 7000);
                    }
                    break;
                }
                case EVENT_8447_9:
                {
                    DoSay(-1200419, LANG_UNIVERSAL, 0);
                    events.ScheduleEvent(EVENT_8447_10, 5000);
                    break;
                }
                case EVENT_8447_10:
                {
                    if (Creature* Malfurion = GetClosestCreatureWithEntry(me, MALFURION_ID, 30))
                    {
                        Malfurion->Say(-1200420, LANG_UNIVERSAL, 0);
                        events.ScheduleEvent(EVENT_8447_11, 11000);
                    }
                    break;
                }
                case EVENT_8447_11:
                {
                    DoSay(-1200421, LANG_UNIVERSAL, 0);
                    events.ScheduleEvent(EVENT_8447_12, 4000);
                    break;
                }
                case EVENT_8447_12:
                {
                    if (Creature* Malfurion = GetClosestCreatureWithEntry(me, MALFURION_ID, 30))
                    {
                        Malfurion->Say(-1200422, LANG_UNIVERSAL, 0);
                        events.ScheduleEvent(EVENT_8447_13, 4000);
                    }
                    break;
                }
                case EVENT_8447_13:
                {
                    DoSay(-1200423, LANG_UNIVERSAL, 0);
                    events.ScheduleEvent(EVENT_8447_14, 10000);
                    break;
                }
                case EVENT_8447_14:
                {
                    if (Creature* Malfurion = GetClosestCreatureWithEntry(me, MALFURION_ID, 30))
                    {
                        Malfurion->Say(-1200424, LANG_UNIVERSAL, 0);
                        events.ScheduleEvent(EVENT_8447_15, 21000);
                    }
                    break;
                }
                case EVENT_8447_15:
                {
                    if (Creature* Malfurion = GetClosestCreatureWithEntry(me, MALFURION_ID, 30))
                    {
                        Malfurion->Say(-1200425, LANG_UNIVERSAL, 0);
                        events.ScheduleEvent(EVENT_8447_16, 6500);
                    }
                    break;
                }
                case EVENT_8447_16:
                {
                    DoSay(-1200426, LANG_UNIVERSAL, 0);
                    events.ScheduleEvent(EVENT_8447_17, 2000);
                    break;
                }
                case EVENT_8447_17:
                {
                    DoSay(-1200427, LANG_UNIVERSAL, 0);
                    events.ScheduleEvent(EVENT_8447_18, 1000);
                    break;
                }
                case EVENT_8447_18:
                {
                    if (Creature* Malfurion = GetClosestCreatureWithEntry(me, MALFURION_ID, 30))
                        Malfurion->DisappearAndDie();
                    events.ScheduleEvent(EVENT_8447_19, 1000);
                    break;
                }
                case EVENT_8447_19:
                {
                    me->GetMotionMaster()->MovePoint(5, 7787.4604,-2320.9807,454.5470);
                    events.ScheduleEvent(EVENT_8447_20, 3000);
                    break;
                }
                case EVENT_8447_20:
                {
                    me->GetMotionMaster()->MovePoint(6, 7814.1699,-2302.2565,456.2227);
                    events.ScheduleEvent(EVENT_8447_21, 3000);
                    break;
                }
                case EVENT_8447_21:
                {
                    me->GetMotionMaster()->MovePoint(7, 7824.6440,-2279.0273,459.3173);
                    events.ScheduleEvent(EVENT_8447_22, 5000);
                    break;
                }
                case EVENT_8447_22:
                {
                    me->GetMotionMaster()->MovePoint(8, 7828.5752,-2246.8354,463.5159);
                    events.ScheduleEvent(EVENT_8447_23, 2500);
                    break;
                }
                case EVENT_8447_23:
                {
                    me->GetMotionMaster()->MovePoint(9, 7848.299,-2216.35,470.888, 3.9);
                    events.ScheduleEvent(EVENT_8447_24, 4000);
                    break;
                }
                case EVENT_8447_24:
                {
                    Unit* player = me->GetUnit(SummonerGUID);
                    if(player)
                        ((Player*)player)->GroupEventHappens(QUEST_8447, me);
                    if (Creature* Remulos = me->GetMap()->GetCreature(me->GetMap()->GetCreatureGUID(REMULOS_O_ID)))
                        if(!Remulos->isAlive())
                            Remulos->Respawn();
                    me->DisappearAndDie();
                    break;
                }
            }
        }
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_remulos_q8447(Creature *creature)
{
    return new npc_remulos_q8447AI (creature);
}

/*
##                      Quest: The Nightmare Manifests
## When we created this script. We tried to make it as much as possible blizzlike.
## But at the same time, we had some things replaced, for example, the coordinates
## of the Nightmares. The coordinates of the Nightmares had to do more clustered, because
## the current system scripting and muvemaps not allow to place Nightmares blizzlike.
 ####*/

enum TheNightmareManifests
{
    REMULOS_SAY_1                        = -1000672,
    REMULOS_SAY_2                        = -1000673,
    REMULOS_SAY_3                        = -1000674,
    REMULOS_SAY_4                        = -1000675,
    REMULOS_SAY_5                        = -1000676,
    ERANIKUS_ZONE_EMOTE_ENTER            = -1000677,
    ERANIKUS_YELL_1                      = -1000678,
    REMULOS_YELL_1                       = -1000679,
    ERANIKUS_EMOTE_LAUGHT                = -1000680,
    ERANIKUS_YELL_2                      = -1000681,
    REMULOS_YELL_2                       = -1000682,
    ERANIKUS_YELL_3                      = -1000683,
    ERANIKUS_EMOTE_ROAR                  = -1000684,
    REMULOS_SAY_6                        = -1000685,
    REMULOS_SAY_7                        = -1000686,
    ERANIKUS_YELL_4                      = -1000687,
    REMULOS_SAY_8                        = -1000688,
    ERANIKUS_YELL_5                      = -1000689,
    ERANIKUS_YELL_6                      = -1000690,
    ERANIKUS_YELL_7                      = -1000691,
    TYRANDE_YELL_1                       = -1000692,
    TYRANDE_SAY_1                        = -1000693,
    TYRANDE_YELL_2                       = -1000694,
    TYRANDE_YELL_3                       = -1000695,
    TYRANDE_SAY_2                        = -1000696,
    ERANIKUS_YELL_8                      = -1000697,
    ERANIKUS_YELL_9                      = -1000698,
    ERANIKUS_YELL_10                     = -1000699,
    ERANIKUS_ZONE_EMOTE_CONSUMED         = -1000700,
    TYRANDE_EMOTE_KNEE                   = -1000701,
    TYRANDE_YELL_4                       = -1000702,
    ERANIKUS_SAY_1                       = -1000703,
    ERANIKUS_SAY_2                       = -1000704,
    ERANIKUS_SAY_3                       = -1000705,
    ERANIKUS_SAY_4                       = -1000706,
    REMULOS_SAY_9                        = -1000707,
    REMULOS_SAY_10                       = -1000708,

    QUEST_NIGHTMARE_MANIFESTS            = 8736,

    NPC_REMULOS                          = 11832,
    NPC_ERANIKUS                         = 15491,
    NPC_TYRANDE                          = 15633,
    NPC_NIGHTMARE_PHANTASM               = 15629,
    NPC_MOONGLADE_WARDEN                 = 11822,
    NPC_ERANIKUS_THE_REDEEMED            = 15628,
    NPC_PRIESTESS_OF_THE_MOON            = 15634,

    NPC_NIGHTMARE_PHANTASMS_COUNT        = 15,
    NPC_MOONGLADE_WARDENS_COUNT          = 10,
    NPC_PRIESTESS_OF_THE_MOON_COUNT      = 7,


    SPELL_C0NJURE_DREAM_RIFT             = 25813,
    SPELL_STARFIRE                       = 21668,
    SPELL_HEALING_TOUCH                  = 23381,
    SPELL_REGROWTH                       = 20665,
    SPELL_REJUVENATION                   = 20664,
    SPELL_TRANQUILITY                    = 25817,

    SPELL_SPOTLIGHT                      = 35259,
    ERANIKUS_SPELL_ACID_BREATH           = 24839,
    ERANIKUS_SPELL_NOXIUS_BREATH         = 24818,
    ERANIKUS_SPELL_SHADOW_BOLT_VOLLEY    = 25586,

    POINT_COMBAT_START                   = 0xFFFFFF,
    POINT_NIGHTRAVEN                     = 0xFFFFF0
};

float PhantasmsSpawnpos[6][4] =
{
    {7865.17f, -2549.79f, 486.685f, 5.0f},
    {7889.45f, -2580.95f, 487.039f, 1.7f},
    {7916.65f, -2556.21f, 487.336f, 1.1f},
    {7926.25f, -2574.34f, 489.651f, 2.9f},
    {7855.32f, -2595.67f, 486.713f, 0.5f},
    {7890.27f, -2529.94f, 483.771f, 4.7f},
};

Position PristessHomePoint[NPC_PRIESTESS_OF_THE_MOON_COUNT] =
{
    {7883.12f, -2560.05f, 486.837f, 0.02f},
    {7882.61f, -2562.55f, 486.947f, 6.17f},
    {7882.21f, -2565.84f, 486.947f, 6.13f},
    {7882.12f, -2569.81f, 486.947f, 6.18f},
    {7878.62f, -2563.07f, 486.947f, 0.03f},
    {7878.14f, -2566.98f, 486.947f, 6.13f},
    {7880.23f, -2565.02f, 486.947f, 0.06f}
};

#define QUEST_WAKING_LEGENDS    8447
#define REMULOS_Q_8447          2550014
/*######
## npc_remulos
######*/

struct npc_remulosAI : public npc_escortAI
{
    npc_remulosAI(Creature *c) : npc_escortAI(c) {}

    uint64 EranikusGUID;
    uint32 uiPhase;
    uint32 EventTimer;
    uint32 FindVictimTimer;
    uint32 StarfireTimer;
    uint32 HealingTouchTimer;
    uint32 RegrowthTimer;
    uint32 RejuvenationTimer;
    uint32 TranquilityTimer;
    uint32 DeadPhantasmsCount;
    bool PhantasmPhase;
    bool EranikusPhase;

    std::list<uint64> PhantasmsList;
    std::list<uint64> MoongladeWardenList;

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            PhantasmPhase = false;
            EranikusPhase = false;
            EventTimer = 0;
            uiPhase = 0;
            FindVictimTimer = 2000;
            DeadPhantasmsCount = 0;
            StarfireTimer = urand(7000, 10000);
            HealingTouchTimer = 4000;
            RegrowthTimer = urand(5000, 10000);
            RejuvenationTimer = urand(5000, 10000);
            TranquilityTimer = urand(15000, 30000);
            EranikusGUID = 0;

            PhantasmsList.clear();
            MoongladeWardenList.clear();
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_NIGHTMARE_PHANTASM)
        {
            PhantasmsList.push_back(pSummoned->GetGUID());
            pSummoned->GetMotionMaster()->MoveChase(me);
            pSummoned->AI()->AttackStart(me);
        }

        if (pSummoned->GetEntry() == NPC_MOONGLADE_WARDEN)
            MoongladeWardenList.push_back(pSummoned->GetGUID());
    }

    void ClearSummonedCreatures()
    {
        if (!MoongladeWardenList.empty())
        {
            for (std::list<uint64>::iterator itr = MoongladeWardenList.begin(); itr != MoongladeWardenList.end(); ++itr)
            {
                if (Creature* pWarden = Unit::GetCreature(*me,*itr))
                    pWarden->ForcedDespawn();
            }
        }
        MoongladeWardenList.clear();

        if (!PhantasmsList.empty())
        {
            for (std::list<uint64>::iterator itr = PhantasmsList.begin(); itr != PhantasmsList.end(); ++itr)
            {
                if (Creature* pPhantasm = Unit::GetCreature(*me,*itr))
                    pPhantasm->ForcedDespawn();
            }
        }
        PhantasmsList.clear();
    }

    void JustDied(Unit* /*killer*/)
    {
        ClearSummonedCreatures();
        if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
            pEranikus->ForcedDespawn();
        me->Respawn();
    }

    void FindVictim(Creature* pCreature)
    {
        if (!pCreature->GetVictim())
        {
            if (PhantasmPhase)
            {
                if (Unit *pTarget = GetClosestCreatureWithEntry(pCreature, NPC_NIGHTMARE_PHANTASM, 50.0f))
                {
                    pCreature->GetMotionMaster()->MoveChase(pTarget);
                    pCreature->AI()->AttackStart(pTarget);
                    return;
                }
            }

            if (EranikusPhase)
            {
                if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                {
                    pCreature->GetMotionMaster()->MoveChase(pEranikus);
                    pCreature->AI()->AttackStart(pEranikus);
                    return;
                }
            }
            return;
        }
    }

    void CheckNightmare()
    {
        if (!PhantasmsList.empty())
        {
            for (std::list<uint64>::iterator itr = PhantasmsList.begin(); itr != PhantasmsList.end(); ++itr)
            {
                if (Creature *pPhantasm = Unit::GetCreature(*me,*itr))
                {
                    if (!pPhantasm->isAlive())
                    {
                        PhantasmsList.erase(itr);
                        DeadPhantasmsCount++;
                        break;
                    }
                }
            }
        }
        if (DeadPhantasmsCount >=15)
        {
            PhantasmsList.clear();
            EventTimer = 10000;
            uiPhase = 14;
        }
    }

    void WaypointReached(uint32 uiPointId)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (uiPointId)
        {
        case 18:
            SetEscortPaused(true);
            DoScriptText(REMULOS_SAY_3, me, pPlayer);
            EventTimer = 5000;
            uiPhase = 2;
            break;
        case 23:
            SetEscortPaused(true);
            me->SetOrientation(0.044f);
            me->SendHeartBeat();
            DoScriptText(REMULOS_SAY_7, me);
            EventTimer = 7000;
            uiPhase = 11;
            break;
        case 24:
            SetEscortPaused(true);
            break;
        case 26:
            SetEscortPaused(true);
            if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
            {
                me->SetOrientation(1.46);
                me->SendHeartBeat();
            }
            break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if (uiPhase)
        {
            if (EventTimer <= diff)
            {
                Player* pPlayer = GetPlayerForEscort();

                if (!pPlayer)
                    return;

                switch (uiPhase)
                {
                case 1:
                    DoScriptText(REMULOS_SAY_2, me);
                    uiPhase = 0;
                    SetEscortPaused(false);
                    break;
                case 2:
                    DoScriptText(REMULOS_SAY_4, me);
                    uiPhase = 3;
                    EventTimer = 10000;
                    break;
                case 3:
                    DoScriptText(REMULOS_SAY_5, me);
                    DoCast(me, SPELL_C0NJURE_DREAM_RIFT);
                    uiPhase = 4;
                    EventTimer = 10000;
                    break;
                case 4:
                    if (Creature* pEranikus = me->SummonCreature(NPC_ERANIKUS, 7867.44f, -2671.37f, 498.042f, 0.51f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000))
                    {
                        //TODO: now must be server-wide emote
                        DoScriptText(ERANIKUS_ZONE_EMOTE_ENTER, pEranikus);
                        pEranikus->SetReactState(REACT_PASSIVE);
                        pEranikus->SetFlying(true);
                        pEranikus->AI()->IsSummonedBy(me);
                        pEranikus->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        EranikusGUID = pEranikus->GetGUID();
                    }
                    else me->ForcedDespawn();
                    uiPhase = 5;
                    EventTimer = 10000;
                    break;
                case 5:
                    if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                        DoScriptText(ERANIKUS_YELL_1, pEranikus);
                    uiPhase = 6;
                    EventTimer = 10000;
                    break;
                case 6:
                    DoScriptText(REMULOS_YELL_1, me);
                    uiPhase = 7;
                    EventTimer = 5000;
                    break;
                case 7:
                    if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                    {
                        DoScriptText(ERANIKUS_EMOTE_LAUGHT, pEranikus);
                        DoScriptText(ERANIKUS_YELL_2, pEranikus);
                    }
                    uiPhase = 8;
                    EventTimer = 8000;
                    break;
                case 8:
                    DoScriptText(REMULOS_YELL_2, me);
                    uiPhase = 9;
                    EventTimer = 9000;
                    break;
                case 9:
                    if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                    {
                        DoScriptText(ERANIKUS_EMOTE_LAUGHT, pEranikus);
                        DoScriptText(ERANIKUS_YELL_3, pEranikus);
                        pEranikus->GetMotionMaster()->MovePoint(POINT_NIGHTRAVEN, 7926.28f, -2573.13f, 501.655f);
                        pEranikus->SetHomePosition(7926.28f, -2573.13f, 501.655f, 2.88f);
                    }
                    uiPhase = 10;
                    EventTimer = 5000;
                    break;
                case 10:
                    DoScriptText(REMULOS_SAY_6, me, pPlayer);
                    SetEscortPaused(false);
                    uiPhase = 0;
                    break;
                case 11:
                    SetEscortPaused(false);
                    if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                    {
                        DoScriptText(ERANIKUS_YELL_4, pEranikus);
                        DoScriptText(REMULOS_SAY_8, me);
                    }

                    for (int i = 0; i < NPC_NIGHTMARE_PHANTASMS_COUNT; i++)
                    {
                        uint32 r = urand(0, 5);
                        Position pos(PhantasmsSpawnpos[r][0], PhantasmsSpawnpos[r][1], PhantasmsSpawnpos[r][2]);
                        me->GetValidPointInAngle(pos, 4, frand(0, 2 * M_PI), false);

                        me->SummonCreature(NPC_NIGHTMARE_PHANTASM, pos.x, pos.y, pos.z, PhantasmsSpawnpos[r][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                    }
                    SetCanAttack(true);
                    PhantasmPhase = true;
                    me->SetSpeed(MOVE_RUN, 2.0f);
                    SetRun(true);
                    uiPhase = 12;
                    EventTimer = 5000;
                    break;
                case 12:
                    if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                        DoScriptText(ERANIKUS_YELL_5, pEranikus);
                    for (int i = 0; i < NPC_MOONGLADE_WARDENS_COUNT; i++)
                    {
                        uint32 r = urand(0, 1);
                        Position pos(PhantasmsSpawnpos[r][0], PhantasmsSpawnpos[r][1], PhantasmsSpawnpos[r][2]);
                        me->GetValidPointInAngle(pos, 4, frand(0, 2 * M_PI), false);
                                                
                        if (Creature* pWarden = me->SummonCreature(NPC_MOONGLADE_WARDEN, pos.x, pos.y, pos.z, PhantasmsSpawnpos[r][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                            FindVictim(pWarden);
                    }
                    uiPhase = 13;
                    EventTimer = 10000;
                    break;
                case 13:
                    CheckNightmare();
                    EventTimer = 500;
                    break;
                case 14:
                    if (DeadPhantasmsCount >= NPC_NIGHTMARE_PHANTASMS_COUNT)
                    {
                        EranikusPhase = true;
                        PhantasmPhase = false;
                        SetCanAttack(false);
                        SetEscortPaused(false);
                        SetRun(false);
                        uiPhase = 15;
                        if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                        {
                            DoScriptText(ERANIKUS_YELL_6, pEranikus);
                            pEranikus->GetMotionMaster()->MovePoint(POINT_COMBAT_START, 7901.51f, -2565.71f, 488.046f);
                            pEranikus->SetHomePosition(7901.51f, -2565.71f, 488.046f, 3.17f);
                        }
                    }
                    EventTimer = 10000;
                    break;
                case 15:
                    if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
                    {
                        for (int i = 0; i < NPC_MOONGLADE_WARDENS_COUNT; i++)
                            me->SummonCreature(NPC_MOONGLADE_WARDEN,
                                pEranikus->GetPositionX(), pEranikus->GetPositionY(),
                                pEranikus->GetPositionZ(), 0.02, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
                    }
                    EventTimer = 60000;
                    break;
                }
            }
            else EventTimer -= diff;
        }

        if (GetTargetForHeal())
        {
            if (RegrowthTimer <= diff)
            {
                DoCast(GetTargetForHeal(), SPELL_REGROWTH);
                RegrowthTimer = urand(5000, 10000);
            } else RegrowthTimer -= diff;

            if (RejuvenationTimer <= diff)
            {
                DoCast(GetTargetForHeal(), SPELL_REJUVENATION);
                RejuvenationTimer = urand(5000, 10000);
            } else RejuvenationTimer -= diff;

            if (TranquilityTimer <= diff)
            {
                DoCast(GetTargetForHeal(), SPELL_TRANQUILITY);
                TranquilityTimer = urand(20000, 30000);
            } else TranquilityTimer -= diff;

            if (HealingTouchTimer <= diff)
            {
                DoCast(GetTargetForHeal(), SPELL_HEALING_TOUCH);
                HealingTouchTimer = 3500;
            } else HealingTouchTimer -= diff;
        }

        if (!UpdateVictim())
        {
            if (FindVictimTimer <= diff)
            {
                FindVictim(me);
                FindVictimTimer = 2000;
                return;
            } else FindVictimTimer -= diff;
            return;
        }

        if (StarfireTimer <= diff)
        {
            DoCast(me->GetVictim(), SPELL_STARFIRE);
            StarfireTimer = urand(7000, 10000);
        } else StarfireTimer -= diff;

        DoMeleeAttackIfReady();
    }

    Unit* GetTargetForHeal()
    {
        if (HealthBelowPct(90))
            return me;

        if (PhantasmPhase)
        {
            if (Unit* pTarget = GetPlayerForEscort())
                if (pTarget->GetHealth()*100 < pTarget->GetMaxHealth()*90)
                    return pTarget;
            return NULL;
        }

        if (EranikusPhase)
        {
            if (Creature* pEranikus = Unit::GetCreature(*me, EranikusGUID))
            {
                if (Unit* pTarget = pEranikus->GetVictim())
                    if (pTarget->GetHealth()*100 < pTarget->GetMaxHealth()*90)
                        return pTarget;
                return NULL;
            }
        }
        return NULL;
    }

    void EventComplete()
    {
        if (Player* pPlayer = GetPlayerForEscort())
            pPlayer->GroupEventHappens(QUEST_NIGHTMARE_MANIFESTS, me);
    }
};

CreatureAI* GetAI_npc_remulos(Creature* pCreature)
{
    return new npc_remulosAI(pCreature);
}

bool QuestAccept_npc_remulos(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_NIGHTMARE_MANIFESTS)
    {
        DoScriptText(REMULOS_SAY_1, pCreature, pPlayer);

        if (npc_remulosAI* pEscortAI = CAST_AI(npc_remulosAI, pCreature->AI()))
        {
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest, true);
            pEscortAI->SetEscortPaused(true);
            pEscortAI->SetDespawnAtEnd(false);
            pEscortAI->SetDespawnAtFar(false);
            pEscortAI->uiPhase = 1;
            pEscortAI->EventTimer = 10000;
        }
    }
    else if (pQuest->GetQuestId() == QUEST_WAKING_LEGENDS)
    {
        //pCreature->Say("Come %s, the lake is right around the bend.", LANG_UNIVERSAL);
        Creature* summon = pPlayer->SummonCreature(REMULOS_Q_8447, pCreature->GetPositionX(),pCreature->GetPositionY(),pCreature->GetPositionZ(),pCreature->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 0);
        pCreature->DisappearAndDie();
    }
    return true;
}

struct npc_eranikusAI : public ScriptedAI
{
public:
    npc_eranikusAI(Creature *c) : ScriptedAI(c) {}

    uint32 ShadowBoltTimer;
    uint32 AcidBreathTimer;
    uint32 NoxiusBreathTimer;
    uint32 EventTimer;
    uint32 FindVictimTimer;
    uint32 uiPhase;
    uint64 TyrandeGUID;
    uint64 RemulosGUID;
    bool CanAttack;

    std::list<uint64> PriestessOfTheMoonList;

    void Reset()
    {
        ShadowBoltTimer = urand(5000, 12000);
        AcidBreathTimer = urand(5000, 15000);
        NoxiusBreathTimer = urand(10000, 20000);
        EventTimer = 0;
        FindVictimTimer = 2000;
        uiPhase = 0;
        TyrandeGUID = 0;
        RemulosGUID = 0;
        CanAttack = false;

        PriestessOfTheMoonList.clear();
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventTimer <= diff)
        {
            switch (uiPhase)
            {
            sLog.outLog(LOG_SPECIAL, "Moonglade event phase: %u", uiPhase);
            case 0:
                if (HealthBelowPct(80))
                {
                    DoScriptText(ERANIKUS_YELL_7, me);
                    if (Creature* pTyrande = me->SummonCreature(NPC_TYRANDE, 7927.01f, -2573.36f, 489.652f, 2.81f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000))
                    {
                        TyrandeGUID = pTyrande->GetGUID();
                        pTyrande->SetVisibility(VISIBILITY_OFF);
                        DoScriptText(TYRANDE_YELL_1, pTyrande);
                    }
                    uiPhase++;
                }
                EventTimer = 3000;
                break;
            case 1:
                if (HealthBelowPct(70))
                {
                    if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                        DoScriptText(TYRANDE_YELL_2, pTyrande);
                    uiPhase++;
                }
                EventTimer = 3000;
                break;
            case 2:
                if (HealthBelowPct(60))
                {
                    if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                        DoScriptText(TYRANDE_YELL_3, pTyrande);
                    uiPhase++;
                }
                EventTimer = 3000;
                break;
            case 3:
                if (HealthBelowPct(40))
                {
                    if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                        DoScriptText(TYRANDE_SAY_2, pTyrande);
                    uiPhase++;
                }
                EventTimer = 3000;
                break;
            case 4:
                if (HealthBelowPct(25))
                {
                    DoScriptText(ERANIKUS_YELL_8, me);
                    uiPhase++;
                }
                EventTimer = 3000;
                break;
            case 5:
                if (HealthBelowPct(22))
                {
                    DoScriptText(ERANIKUS_YELL_9, me);
                    uiPhase++;
                }
                EventTimer = 3000;
                break;
            case 6:
                if (HealthBelowPct(20))
                {
                    if (Creature* pRemulos = Unit::GetCreature(*me, RemulosGUID))
                    {
                        CAST_AI(npc_remulosAI, pRemulos->AI())->EranikusPhase = false;
                        CAST_AI(npc_remulosAI, pRemulos->AI())->uiPhase = 0;
                        CAST_AI(npc_remulosAI, pRemulos->AI())->SetEscortPaused(false);
                    }
                    else
                    {
                        Unit *pRemulosOtherWay = FindCreature(NPC_REMULOS, 50.0f, me);
                        if(pRemulosOtherWay)
                        {
                            CAST_AI(npc_remulosAI, ((Creature*)pRemulosOtherWay)->AI())->EranikusPhase = false;
                            CAST_AI(npc_remulosAI, ((Creature*)pRemulosOtherWay)->AI())->uiPhase = 0;
                            CAST_AI(npc_remulosAI, ((Creature*)pRemulosOtherWay)->AI())->SetEscortPaused(false);
                        }
                    }
                    DoScriptText(ERANIKUS_YELL_10, me);
                    for (std::set<Unit*>::const_iterator itr = me->GetAttackers().begin(); itr != me->GetAttackers().end(); ++itr)
                    {
                        Position pos(7877.72f, -2581.84f, 486.948f);
                        me->GetValidPointInAngle(pos, 8, frand(0, 2 * M_PI), false);

                        if ((*itr))
                            (*itr)->RemoveAllAuras();

                        if ((*itr)->GetEntry() == NPC_MOONGLADE_WARDEN)
                        {
                            if (Creature* pWarden = Unit::GetCreature(*me, (*itr)->GetGUID()))
                            {
                                pWarden->SetHomePosition(pos.x, pos.y, pos.z, 0.0f);
                                pWarden->GetMotionMaster()->MoveTargetedHome();
                                pWarden->SetStandState(UNIT_STAND_STATE_SIT);
                            }
                        }
                    }
                    CanAttack = false;
                    me->CombatStop(true);
                    me->setFaction(35);
                    me->DeleteThreatList();
                    me->RemoveAllAuras();
                    me->SetReactState(REACT_PASSIVE);
                    me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                    if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                    {
                        pTyrande->SetVisibility(VISIBILITY_ON);
                        pTyrande->SetHomePosition(7886.63f, -2565.8f, 486.965f, 6.16066f);
                        pTyrande->GetMotionMaster()->MoveTargetedHome();
                    }
                    
                    for (int i = 0; i < NPC_PRIESTESS_OF_THE_MOON_COUNT; ++i)
                    {
                        Position pos(7927.01f, -2573.36f, 489.652f);
                        me->GetValidPointInAngle(pos, 4, urand(0, 2*M_PI), false);
                        if (Creature* pPristess = me->SummonCreature(NPC_PRIESTESS_OF_THE_MOON, pos.x, pos.y, pos.z, 2.81f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000))
                        {
                            PriestessOfTheMoonList.push_back(pPristess->GetGUID());
                            pPristess->SetHomePosition(PristessHomePoint[i].x, PristessHomePoint[i].y,
                                PristessHomePoint[i].z, PristessHomePoint[i].o);
                            pPristess->GetMotionMaster()->MoveTargetedHome();
                        }
                    }
                    EventTimer = 3000;
                    uiPhase++;
                } else
                    EventTimer = 3000;
                break;
            case 7:
                me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                DoTeleportTo(7901.51f, -2565.71f, 488.046f);
                DoCast(me, SPELL_SPOTLIGHT, false);
                //TODO: now must be server-wide emote
                DoScriptText(ERANIKUS_ZONE_EMOTE_CONSUMED, me);
                if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                    me->SetFacingToObject(pTyrande);
                EventTimer = 3000;
                uiPhase++;
                break;
            case 8:
                if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                {
                    pTyrande->SetFacingToObject(me);
                    DoScriptText(TYRANDE_EMOTE_KNEE, pTyrande);
                    pTyrande->SetStandState(UNIT_STAND_STATE_KNEEL);
                    DoScriptText(TYRANDE_YELL_4, pTyrande);
                }
                EventTimer = 3000;
                uiPhase++;
                break;
            case 9:
                if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                {
                    pTyrande->SetStandState(UNIT_STAND_STATE_STAND);
                    me->SetFacingToObject(pTyrande);
                }
                if (Creature* pRemulos = Unit::GetCreature(*me, RemulosGUID))
                    pRemulos->SetFacingToObject(me);
                else
                {
                    Unit *pRemulosOtherWay = FindCreature(NPC_REMULOS, 50.0f, me);
                    if(pRemulosOtherWay)
                        pRemulosOtherWay->SetFacingToObject(me);
                }
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->UpdateEntry(NPC_ERANIKUS_THE_REDEEMED);
                me->SetDisplayId(6984);
                DoScriptText(ERANIKUS_SAY_1, me);
                EventTimer = 8000;
                uiPhase++;
                break;
            case 10:
                DoScriptText(ERANIKUS_SAY_2, me);
                EventTimer = 5000;
                uiPhase++;
                break;
            case 11:
                DoScriptText(ERANIKUS_SAY_3, me);
                EventTimer = 10000;
                uiPhase++;
                break;
            case 12:
                DoScriptText(ERANIKUS_SAY_4, me);
                EventTimer = 8000;
                uiPhase++;
                break;
            case 13:
                if (Creature* pRemulos = Unit::GetCreature(*me, RemulosGUID))
                {
                    DoScriptText(REMULOS_SAY_9, pRemulos);
                    CAST_AI(npc_remulosAI, pRemulos->AI())->EventComplete();
                }
                else
                {
                    Unit *pRemulosOtherWay = FindCreature(NPC_REMULOS, 50.0f, me);
                    if(pRemulosOtherWay)
                    {
                        DoScriptText(REMULOS_SAY_9, pRemulosOtherWay);
                        CAST_AI(npc_remulosAI, ((Creature*)pRemulosOtherWay)->AI())->EventComplete();
                    }
                }

                if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
                    pTyrande->ForcedDespawn();

                if (!PriestessOfTheMoonList.empty())
                {
                    for (std::list<uint64>::iterator itr = PriestessOfTheMoonList.begin(); itr != PriestessOfTheMoonList.end(); ++itr)
                    {
                        if (Creature* pPristess= Unit::GetCreature(*me,*itr))
                            pPristess->ForcedDespawn();
                    }
                }
                PriestessOfTheMoonList.clear();

                if (Creature* pRemulos = Unit::GetCreature(*me, RemulosGUID))
                {
                    DoScriptText(REMULOS_SAY_10, pRemulos);
                    CAST_AI(npc_remulosAI, pRemulos->AI())->ClearSummonedCreatures();
                    pRemulos->DisappearAndDie();
                }
                else
                {
                    Unit *pRemulosOtherWay = FindCreature(NPC_REMULOS, 50.0f, me);
                    if(pRemulosOtherWay)
                    {
                        DoScriptText(REMULOS_SAY_10, pRemulosOtherWay);
                        CAST_AI(npc_remulosAI, ((Creature*)pRemulosOtherWay)->AI())->ClearSummonedCreatures();
                        ((Creature*)pRemulosOtherWay)->DisappearAndDie();
                    }
                }

                Reset();
                me->ForcedDespawn();
                break;
            }
        } else EventTimer -= diff;

        if (CanAttack)
        {
            if (!UpdateVictim())
            {
                if (FindVictimTimer <= diff)
                {
                    FindVictim();
                    FindVictimTimer = 2000;
                    return;
                } else FindVictimTimer -= diff;
                return;
            }

            if (ShadowBoltTimer <= diff)
            {
                DoCast(me->GetVictim(), ERANIKUS_SPELL_SHADOW_BOLT_VOLLEY);
                ShadowBoltTimer = urand(7000, 12000);
            } else ShadowBoltTimer -= diff;

            if (AcidBreathTimer <= diff)
            {
                DoCast(me->GetVictim(), ERANIKUS_SPELL_ACID_BREATH);
                AcidBreathTimer = urand(5000, 15000);
            } else AcidBreathTimer -= diff;

            if (NoxiusBreathTimer <= diff)
            {
                DoCast(me->GetVictim(), ERANIKUS_SPELL_NOXIUS_BREATH);
                NoxiusBreathTimer = urand(10000, 20000);
            } else NoxiusBreathTimer -= diff;

            DoMeleeAttackIfReady();
        }
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        if (uiMotionType != POINT_MOTION_TYPE)
            return;

        if (uiPointId == POINT_NIGHTRAVEN)
        {
            me->SetOrientation(2.88f);
            me->SendHeartBeat();
        }

        if (uiPointId == POINT_COMBAT_START)
        {
            CanAttack = true;
            me->SetFlying(false);
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE, false);
        }
    }

    void JustDied()
    {
        if (Creature* pTyrande = Unit::GetCreature(*me, TyrandeGUID))
        {
            if (pTyrande->isAlive())
                pTyrande->ForcedDespawn();
        }

        if (!PriestessOfTheMoonList.empty())
        {
            for (std::list<uint64>::iterator itr = PriestessOfTheMoonList.begin(); itr != PriestessOfTheMoonList.end(); ++itr)
            {
                if (Creature* pPristess= Unit::GetCreature(*me,*itr))
                    pPristess->ForcedDespawn();
            }
        }
        PriestessOfTheMoonList.clear();
    }

    void IsSummonedBy(Unit* pSummoner)
    {
        if (pSummoner->GetEntry() == NPC_REMULOS)
        {
            RemulosGUID = pSummoner->GetGUID();
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveCharge(7867.44f, -2671.37f, 503.042f);
            return;
        }
    }

    void FindVictim()
    {
        if (!me->GetVictim())
        {
            if (Unit *pTarget = me->SelectNearestTarget(20))
            {
                me->GetMotionMaster()->MoveChase(pTarget);
                AttackStart(pTarget);
                return;
            }

            if (Creature* pRemulos = Unit::GetCreature(*me, RemulosGUID))
            {
                me->GetMotionMaster()->MoveChase(pRemulos);
                AttackStart(pRemulos);
                return;
            }
        }
    }
};

CreatureAI* GetAI_npc_eranikus(Creature* pCreature)
{
    return new npc_eranikusAI(pCreature);
}

void AddSC_moonglade()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_bunthen_plainswind";
    newscript->pGossipHello =  &GossipHello_npc_bunthen_plainswind;
    newscript->pGossipSelect = &GossipSelect_npc_bunthen_plainswind;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_great_bear_spirit";
    newscript->pGossipHello =  &GossipHello_npc_great_bear_spirit;
    newscript->pGossipSelect = &GossipSelect_npc_great_bear_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_silva_filnaveth";
    newscript->pGossipHello =  &GossipHello_npc_silva_filnaveth;
    newscript->pGossipSelect = &GossipSelect_npc_silva_filnaveth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_clintar_dreamwalker";
    newscript->GetAI = &GetAI_npc_22834;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_clintar_dreamwalker;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_22834;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_clintar_spirit";
    newscript->GetAI = &GetAI_npc_clintar_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_remulos";
    newscript->GetAI = &GetAI_npc_remulos;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_remulos;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_eranikus";
    newscript->GetAI = &GetAI_npc_eranikus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_remulos_8447";
    newscript->GetAI = &GetAI_npc_remulos_q8447;
    newscript->RegisterSelf();
}

