// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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
SDName: Tanaris
SD%Complete: 80
SDComment: Quest support: 648, 1560, 2882, 2954, 4005, 10277, 10279(Special flight path). Noggenfogger vendor
SDCategory: Tanaris
EndScriptData */

/* ContentData
mob_aquementas
npc_custodian_of_time
npc_marin_noggenfogger
npc_steward_of_time
npc_stone_watcher_of_norgannon
npc_OOX17
go_landmark_treasure
npc_tooga
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"

/*######
## mob_aquementas
######*/

#define AGGRO_YELL_AQUE     -1000350

#define SPELL_AQUA_JET      13586
#define SPELL_FROST_SHOCK   15089

struct mob_aquementasAI : public ScriptedAI
{
    mob_aquementasAI(Creature *c) : ScriptedAI(c) {}

    Timer_UnCheked SendItem_Timer;
    Timer_UnCheked SwitchFaction_Timer;
    bool isFriendly;

    Timer_UnCheked FrostShock_Timer;
    Timer_UnCheked AquaJet_Timer;

    void Reset()
    {
        SendItem_Timer.Reset(1000);
        SwitchFaction_Timer.Reset(10000);
        me->setFaction(35);
        isFriendly = true;

        AquaJet_Timer.Reset(5000);
        FrostShock_Timer.Reset(1000);
    }

    void SendItem(Unit* receiver)
    {
        if (!receiver)
            return;
        Player* plr = receiver->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!plr)
            return;

        if (!plr->HasItemCount(11522, 1, true) &&
            plr->HasItemCount(11169, 1, false) &&
            plr->HasItemCount(11172, 11, false) &&
            plr->HasItemCount(11173, 1, false))
        {
            ItemPosCountVec dest;
            if (plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 11522, 1) == EQUIP_ERR_OK)
                plr->StoreNewItem(dest, 11522, 1, true);
        }
    }

    void JustDied(Unit* pKiller)
    {
        SendItem(pKiller);
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(AGGRO_YELL_AQUE, me, who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (isFriendly)
        {
            if (SwitchFaction_Timer.Expired(diff))
            {
                me->setFaction(91);
                isFriendly = false;
            }
        }

        if (!UpdateVictim())
            return;

        if (!isFriendly)
        {
            if (SendItem_Timer.Expired(diff))
            {
                SendItem(me->GetVictim());
                SendItem_Timer = 1000;
            }
        }

        if (FrostShock_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_FROST_SHOCK);
            FrostShock_Timer = 15000;
        }

        if (AquaJet_Timer.Expired(diff))
        {
            DoCast(me, SPELL_AQUA_JET);
            AquaJet_Timer = 15000;
        } 

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_aquementas(Creature* pCreature)
{
    return new mob_aquementasAI (pCreature);
}

/*######
## npc_custodian_of_time
######*/

#define WHISPER_CUSTODIAN_1     -1000150
#define WHISPER_CUSTODIAN_2     -1000151
#define WHISPER_CUSTODIAN_3     -1000152
#define WHISPER_CUSTODIAN_4     -1000153
#define WHISPER_CUSTODIAN_5     -1000154
#define WHISPER_CUSTODIAN_6     -1000155
#define WHISPER_CUSTODIAN_7     -1000156
#define WHISPER_CUSTODIAN_8     -1000157
#define WHISPER_CUSTODIAN_9     -1000158
#define WHISPER_CUSTODIAN_10    -1000159
#define WHISPER_CUSTODIAN_11    -1000160
#define WHISPER_CUSTODIAN_12    -1000161
#define WHISPER_CUSTODIAN_13    -1000162
#define WHISPER_CUSTODIAN_14    -1000163

struct npc_custodian_of_timeAI : public npc_escortAI
{
    npc_custodian_of_timeAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player *pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch(i)
        {
            case 0: DoScriptText(WHISPER_CUSTODIAN_1, me, pPlayer); break;
            case 1: DoScriptText(WHISPER_CUSTODIAN_2, me, pPlayer); break;
            case 2: DoScriptText(WHISPER_CUSTODIAN_3, me, pPlayer); break;
            case 3: DoScriptText(WHISPER_CUSTODIAN_4, me, pPlayer); break;
            case 5: DoScriptText(WHISPER_CUSTODIAN_5, me, pPlayer); break;
            case 6: DoScriptText(WHISPER_CUSTODIAN_6, me, pPlayer); break;
            case 7: DoScriptText(WHISPER_CUSTODIAN_7, me, pPlayer); break;
            case 8: DoScriptText(WHISPER_CUSTODIAN_8, me, pPlayer); break;
            case 9: DoScriptText(WHISPER_CUSTODIAN_9, me, pPlayer); break;
            case 10: DoScriptText(WHISPER_CUSTODIAN_4, me, pPlayer); break;
            case 13: DoScriptText(WHISPER_CUSTODIAN_10, me, pPlayer); break;
            case 14: DoScriptText(WHISPER_CUSTODIAN_4, me, pPlayer); break;
            case 16: DoScriptText(WHISPER_CUSTODIAN_11, me, pPlayer); break;
            case 17: DoScriptText(WHISPER_CUSTODIAN_12, me, pPlayer); break;
            case 18: DoScriptText(WHISPER_CUSTODIAN_4, me, pPlayer); break;
            case 22: DoScriptText(WHISPER_CUSTODIAN_13, me, pPlayer); break;
            case 23: DoScriptText(WHISPER_CUSTODIAN_4, me, pPlayer); break;
            case 24:
                DoScriptText(WHISPER_CUSTODIAN_14, me, pPlayer);
                DoCast(pPlayer, 34883);
                // below here is temporary workaround, to be removed when spell works properly
                pPlayer->GroupEventHappens(10277, me);
                break;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (who->HasAura(34877,1) && CAST_PLR(who)->GetQuestStatus(10277) == QUEST_STATUS_INCOMPLETE)
            {
                float Radius = 10.0;
                if (me->IsWithinDistInMap(who, Radius))
                {
                    Start(false, false, who->GetGUID());
                }
            }
        }
    }

    void EnterCombat(Unit* /*who*/) {}
    void Reset() { }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_npc_custodian_of_time(Creature* pCreature)
{
    return new npc_custodian_of_timeAI(pCreature);
}

/*######
## npc_marin_noggenfogger
######*/

bool GossipHello_npc_marin_noggenfogger(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pCreature->isVendor() && pPlayer->GetQuestRewardStatus(2662))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, pPlayer->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_marin_noggenfogger(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_TRADE)
        pPlayer->SEND_VENDORLIST(pCreature->GetGUID());

    return true;
}

/*######
## npc_steward_of_time
######*/

#define GOSSIP_ITEM_FLIGHT  16379

bool GossipHello_npc_steward_of_time(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(10279) == QUEST_STATUS_INCOMPLETE || pPlayer->GetQuestRewardStatus(10279))
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_FLIGHT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        pPlayer->SEND_GOSSIP_MENU(9978, pCreature->GetGUID());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(9977, pCreature->GetGUID());

    return true;
}

bool QuestAccept_npc_steward_of_time(Player* pPlayer, Creature* /*pCreature*/, Quest const *quest)
{
    if (quest->GetQuestId() == 10279)                      //Quest: To The Master's Lair
        pPlayer->CastSpell(pPlayer,34891,true);               //(Flight through Caverns)

    return false;
}

bool GossipSelect_npc_steward_of_time(Player* pPlayer, Creature* /*pCreature*/, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        pPlayer->CastSpell(pPlayer,34891,true);               //(Flight through Caverns)

    return true;
}

/*######
## npc_stone_watcher_of_norgannon
######*/

#define GOSSIP_ITEM_NORGANNON_1     16380
#define GOSSIP_ITEM_NORGANNON_2     16381
#define GOSSIP_ITEM_NORGANNON_3     16382
#define GOSSIP_ITEM_NORGANNON_4     16383
#define GOSSIP_ITEM_NORGANNON_5     16384
#define GOSSIP_ITEM_NORGANNON_6     16385

bool GossipHello_npc_stone_watcher_of_norgannon(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(2954) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NORGANNON_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(1674, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_stone_watcher_of_norgannon(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NORGANNON_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->SEND_GOSSIP_MENU(1675, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NORGANNON_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(1676, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NORGANNON_4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            pPlayer->SEND_GOSSIP_MENU(1677, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NORGANNON_5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            pPlayer->SEND_GOSSIP_MENU(1678, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NORGANNON_6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            pPlayer->SEND_GOSSIP_MENU(1679, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(2954);
            break;
    }
    return true;
}

/*######
## npc_OOX17
######*/

enum e00X17
{
    //texts are signed for 7806
    SAY_OOX_START           = -1060000,
    SAY_OOX_AGGRO1          = -1060001,
    SAY_OOX_AGGRO2          = -1060002,
    SAY_OOX_AMBUSH          = -1060003,
    SAY_OOX17_AMBUSH_REPLY  = -1060004,
    SAY_OOX_END             = -1060005,

    Q_OOX17                 = 648,
    SPAWN_FIRST             = 7803,
    SPAWN_SECOND_1          = 5617,
    SPAWN_SECOND_2          = 7805
};

struct npc_OOX17AI : public npc_escortAI
{
    npc_OOX17AI(Creature *c) : npc_escortAI(c) {}

    void Reset() {}

    void EnterCombat(Unit* /*who*/)
    {
        DoScriptText(RAND(SAY_OOX_AGGRO1,SAY_OOX_AGGRO2), me);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == SPAWN_SECOND_1)
            DoScriptText(SAY_OOX17_AMBUSH_REPLY, summoned);

        summoned->AI()->AttackStart(me);
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch(i)
        {
            case 23:
                DoScriptText(SAY_OOX_AMBUSH, me);
                break;
            case 24:
                for (uint8 i = 0; i < 3; ++i)
                {
                    float x, y, z;
                    switch (i)
                    {
                        case 0:
                            me->GetNearPoint(x, y, z, 0.0f, 15.0f, 0.0f);
                            me->SummonCreature(SPAWN_FIRST, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                            break;
                        case 1:
                            me->GetNearPoint(x, y, z, 0.0f, 15.0f, 2.0f);
                            me->SummonCreature(SPAWN_FIRST, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                            break;
                        case 2:
                            me->GetNearPoint(x, y, z, 0.0f, 15.0f, 4.0f);
                            me->SummonCreature(SPAWN_FIRST, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                            break;
                    }
                }
                break;
            case 57:
                DoScriptText(SAY_OOX_AMBUSH, me);
                break;
            case 58:
                for (uint8 i = 0; i < 3; ++i)
                {
                    float x, y, z;
                    switch (i)
                    {
                        case 0:
                            me->GetNearPoint(x, y, z, 0.0f, 15.0f, 0.0f);
                            me->SummonCreature(SPAWN_SECOND_1, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                            break;
                        case 1:
                            me->GetNearPoint(x, y, z, 0.0f, 15.0f, 2.0f);
                            me->SummonCreature(SPAWN_SECOND_2, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                            break;
                        case 2:
                            me->GetNearPoint(x, y, z, 0.0f, 15.0f, 4.0f);
                            me->SummonCreature(SPAWN_SECOND_2, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
                            break;
                    }
                }
                break;
            case 88:
                if (pPlayer)
                {
                    DoScriptText(SAY_OOX_END, me);
                    pPlayer->GroupEventHappens(Q_OOX17, me);
                }
                break;
        }
    }
};

bool QuestAccept_npc_OOX17(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == Q_OOX17)
    {
        pCreature->SetStandState(UNIT_STAND_STATE_STAND);
        DoScriptText(SAY_OOX_START, pCreature);
        pCreature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
        if (npc_escortAI* pEscortAI = CAST_AI(npc_OOX17AI, pCreature->AI()))
            pEscortAI->Start(true, true, pPlayer->GetGUID());
    }
    return true;
}

CreatureAI* GetAI_npc_OOX17(Creature* pCreature)
{
    return new npc_OOX17AI(pCreature);
}

/*######
## go_landmark_treasure
######*/

#define QUEST_CUERGOS_GOLD 2882
#define GO_TREASURE        142194
#define NPC_PIRATES_1      7899
#define NPC_PIRATES_2       8901
#define NPC_PIRATES_3       7902

bool GOUse_go_landmark_treasure(Player *player, GameObject* _GO)
{
    if (player->GetQuestStatus(QUEST_CUERGOS_GOLD) != QUEST_STATUS_INCOMPLETE)
        return false;

    Creature* pirate1 = NULL;
    Creature* pirate2 = NULL;
    Creature* pirate3 = NULL;
    Creature* pirate4 = NULL;
    Creature* pirate5 = NULL;
    GameObject* pTreasure = NULL;
    int extraPirateType[2];
    extraPirateType[0] = NPC_PIRATES_1;
    extraPirateType[1] = NPC_PIRATES_2;

    if (pirate1 = _GO->SummonCreature(NPC_PIRATES_1, -10119.85f, -4068.36f, 4.55f, 1.35f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 310000))
    {
        pirate1->AI()->AttackStart(player);
        pirate1->SetRespawnDelay(350000);
    }
    if (pirate2 = _GO->SummonCreature(NPC_PIRATES_2, -10109.80f, -4054.45f, 5.64f, 3.17f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 310000))
    {
        pirate2->AI()->AttackStart(player);
        pirate2->SetRespawnDelay(350000);
    }
    if (pirate3 = _GO->SummonCreature(NPC_PIRATES_3, -10127.80f, -4047.04f, 4.50f, 5.07f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 310000))
    {
        pirate3->AI()->AttackStart(player);
        pirate3->SetRespawnDelay(350000);
    }

    for (int i = 0; i < 2; i++)
    {
        switch (urand(0, 2))
        {
        case 0:
            extraPirateType[i] = NPC_PIRATES_1;
            break;
        case 1:
            extraPirateType[i] = NPC_PIRATES_2;
            break;
        case 2:
            extraPirateType[i] = NPC_PIRATES_3;
            break;
        }
    }
    if (pirate4 = _GO->SummonCreature(extraPirateType[0], -10113.952148f, -4040.484375f, 5.174251f, 4.300828f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 310000))
    {
        pirate4->AI()->AttackStart(player);
        pirate4->SetRespawnDelay(350000);
    }
    if (pirate5 = _GO->SummonCreature(extraPirateType[1], -10136.779297f, -4063.175049f, 4.787039f, 0.526417f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 310000))
    {
        pirate5->AI()->AttackStart(player);
        pirate5->SetRespawnDelay(350000);
    }
    _GO->SetGoState(GO_STATE_ACTIVE);
    _GO->SetRespawnTime(900);
    player->SummonGameObject(GO_TREASURE, -10119.70, -4050.45, 5.33, 0, 0, 0, 0, 0, 240);
    return true;
};

/*####
# npc_tooga
####*/

enum eTooga
{
    SAY_TOOG_THIRST             = -1600391,
    SAY_TOOG_WORRIED            = -1600392,
    SAY_TOOG_POST_1             = -1600393,
    SAY_TORT_POST_2             = -1600394,
    SAY_TOOG_POST_3             = -1600395,
    SAY_TORT_POST_4             = -1600396,
    SAY_TOOG_POST_5             = -1600397,
    SAY_TORT_POST_6             = -1600398,

    QUEST_TOOGA                 = 1560,
    NPC_TORTA                   = 6015,

    POINT_ID_TO_WATER           = 1,
    FACTION_TOOG_ESCORTEE       = 113
};

const float m_afToWaterLoc[] = {-7032.664551f, -4906.199219f, -1.606446f};

struct npc_toogaAI : public FollowerAI
{
    npc_toogaAI(Creature* pCreature) : FollowerAI(pCreature) { }

    uint32 m_uiCheckSpeechTimer;
    uint32 m_uiPostEventTimer;
    uint32 m_uiPhasePostEvent;

    uint64 TortaGUID;

    void Reset()
    {
        m_uiCheckSpeechTimer = 2500;
        m_uiPostEventTimer = 1000;
        m_uiPhasePostEvent = 0;

        TortaGUID = 0;
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        FollowerAI::MoveInLineOfSight(pWho);

        if (!me->GetVictim() && !HasFollowState(STATE_FOLLOW_COMPLETE | STATE_FOLLOW_POSTEVENT) && pWho->GetEntry() == NPC_TORTA)
        {
            if (me->IsWithinDistInMap(pWho, INTERACTION_DISTANCE))
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_TOOGA) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->GroupEventHappens(QUEST_TOOGA, me);
                }

                TortaGUID = pWho->GetGUID();
                SetFollowComplete(true);
            }
        }
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        FollowerAI::MovementInform(uiMotionType, uiPointId);

        if (uiMotionType != POINT_MOTION_TYPE)
            return;

        if (uiPointId == POINT_ID_TO_WATER)
            SetFollowComplete();
    }

    void UpdateFollowerAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
        {
            //we are doing the post-event, or...
            if (HasFollowState(STATE_FOLLOW_POSTEVENT))
            {
                if (m_uiPostEventTimer <= uiDiff)
                {
                    m_uiPostEventTimer = 5000;

                    Unit *pTorta = Unit::GetUnit(*me, TortaGUID);
                    if (!pTorta || !pTorta->isAlive())
                    {
                        //something happened, so just complete
                        SetFollowComplete();
                        return;
                    }

                    switch(m_uiPhasePostEvent)
                    {
                        case 1:
                            DoScriptText(SAY_TOOG_POST_1, me);
                            break;
                        case 2:
                            DoScriptText(SAY_TORT_POST_2, pTorta);
                            break;
                        case 3:
                            DoScriptText(SAY_TOOG_POST_3, me);
                            break;
                        case 4:
                            DoScriptText(SAY_TORT_POST_4, pTorta);
                            break;
                        case 5:
                            DoScriptText(SAY_TOOG_POST_5, me);
                            break;
                        case 6:
                            DoScriptText(SAY_TORT_POST_6, pTorta);
                            me->GetMotionMaster()->MovePoint(POINT_ID_TO_WATER, m_afToWaterLoc[0], m_afToWaterLoc[1], m_afToWaterLoc[2]);
                            break;
                    }

                    ++m_uiPhasePostEvent;
                }
                else
                    m_uiPostEventTimer -= uiDiff;
            }
            //...we are doing regular speech check
            else if (HasFollowState(STATE_FOLLOW_INPROGRESS))
            {
                if (m_uiCheckSpeechTimer <= uiDiff)
                {
                    m_uiCheckSpeechTimer = 5000;

                    if (urand(0,9) > 8)
                        DoScriptText(RAND(SAY_TOOG_THIRST,SAY_TOOG_WORRIED), me);
                }
                else
                    m_uiCheckSpeechTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_tooga(Creature* pCreature)
{
    return new npc_toogaAI(pCreature);
}

bool QuestAccept_npc_tooga(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOOGA)
    {
        if (npc_toogaAI* pToogaAI = CAST_AI(npc_toogaAI, pCreature->AI()))
            pToogaAI->StartFollow(pPlayer, FACTION_TOOG_ESCORTEE, pQuest);
    }

    return true;
}

struct npc_anachronosAI : public ScriptedAI
{
    npc_anachronosAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    Timer CheckTimer;
    Timer VisibilityTimer;

    void Reset()
    {
        me->SetVisibility(VISIBILITY_ON);
        CheckTimer.Reset(1000);
        VisibilityTimer.Reset(1);
    }

    void UpdateAI(const uint32 diff)
    {
        if (VisibilityTimer.Expired(diff))
        {
            if(me->GetVisibility() == VISIBILITY_OFF)
                me->SetVisibility(VISIBILITY_ON);
            
            VisibilityTimer = 0;
        }

        if (!UpdateVictim())
            return;

        if (CheckTimer.Expired(diff))
        {
            if (HealthBelowPct(20))
            {
                me->Yell(-1200440, LANG_UNIVERSAL, 0);
                me->SetVisibility(VISIBILITY_OFF);
                me->DestroyForNearbyPlayers();
                me->Kill(me, false);
                return;
            }

            CheckTimer = 1000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_anachronos(Creature* pCreature)
{
    return new npc_anachronosAI(pCreature);
}

#define SPELL_AV_VISUALTRANSFORM 24085

struct npc_8579AI : public npc_escortAI
{
    npc_8579AI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
    }
    
    uint32 Event_Timer;
    bool isEventStarted;
    uint32 Point;   
 
    void Reset()
    {
        isEventStarted = false;
        me->LoadEquipment(1315, true);
        me->SetDisplayId(7902);
        Event_Timer = 0;
        me->SetFlying(false);
        me->SetWalk(false);
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;
    }

    void WaypointReached(uint32 i)
    {
        switch (i)
        {
            case 1:
                me->SetWalk(false);
                isEventStarted = true;
                me->LoadEquipment(0, true);
                Event_Timer = 3000;
                DoCast(me, SPELL_AV_VISUALTRANSFORM);
                me->SetDisplayId(1336);
                me->SetFlying(true);
                SetEscortPaused(true);
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (Event_Timer <= diff)
        {
            if(isEventStarted)
            {
                SetEscortPaused(false);
                me->MonsterYell(-1200441, LANG_UNIVERSAL, 0);
                me->SetWalk(false);
                Event_Timer = 15000;
            }
        }
        else
            Event_Timer -= diff;
    }
};

CreatureAI* GetAI_npc_8579(Creature* pCreature)
{
    npc_8579AI* npc_8579 = new npc_8579AI(pCreature);
    npc_8579->AddWaypoint(1, -6909.799, -4840.913, 8.273);
    npc_8579->AddWaypoint(2, -6907.971, -4865.743, 19.445);
    return (CreatureAI*)npc_8579;
}

#define QUEST_HAKKAR_EVENT 8181

bool QuestRewarded_npc_8579(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_HAKKAR_EVENT)
    {
        pCreature->Say(-1200442, LANG_UNIVERSAL, 0);

        if (npc_8579AI* pEscortAI = dynamic_cast<npc_8579AI*>(pCreature->AI()))
        {
            pEscortAI->Start(true, true, 0, NULL, true);
            pCreature->SetWalk(false);
        }
    }
    return true;
}

bool GossipHello_npc_8579(Player *player, Creature *_Creature)
{
    if(_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(player->GetQuestRewardStatus(3528))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16386), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(2139, _Creature->GetGUID());
    }else
        player->SEND_GOSSIP_MENU(2140, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_8579(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CastSpell(player, 12998, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        default: break;
    }
    return true;
}

void AddSC_tanaris()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_8579";
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_8579;
    newscript->pGossipHello =  &GossipHello_npc_8579;
    newscript->pGossipSelect = &GossipSelect_npc_8579;
    newscript->GetAI = &GetAI_npc_8579;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_aquementas";
    newscript->GetAI = &GetAI_mob_aquementas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_custodian_of_time";
    newscript->GetAI = &GetAI_npc_custodian_of_time;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_marin_noggenfogger";
    newscript->pGossipHello =  &GossipHello_npc_marin_noggenfogger;
    newscript->pGossipSelect = &GossipSelect_npc_marin_noggenfogger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_steward_of_time";
    newscript->pGossipHello =  &GossipHello_npc_steward_of_time;
    newscript->pGossipSelect = &GossipSelect_npc_steward_of_time;
    newscript->pQuestAcceptNPC =  &QuestAccept_npc_steward_of_time;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stone_watcher_of_norgannon";
    newscript->pGossipHello =  &GossipHello_npc_stone_watcher_of_norgannon;
    newscript->pGossipSelect = &GossipSelect_npc_stone_watcher_of_norgannon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_OOX17";
    newscript->GetAI = &GetAI_npc_OOX17;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_OOX17;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_landmark_treasure";
    newscript->pGOUse = &GOUse_go_landmark_treasure;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tooga";
    newscript->GetAI = &GetAI_npc_tooga;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_tooga;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_anachronos";
    newscript->GetAI = &GetAI_npc_anachronos;
    newscript->RegisterSelf();
}
