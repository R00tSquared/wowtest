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
SDName: Orgrimmar
SD%Complete: 100
SDComment: Quest support: 2460, 4941, 5727, 6566
SDCategory: Orgrimmar
EndScriptData */

/* ContentData
npc_neeru_fireblade     npc_text + gossip options text missing
npc_shenthul
npc_thrall_warchief
npc_eitrigg
EndContentData */

#include "precompiled.h"

/*######
## npc_neeru_fireblade
######*/

#define QUEST_5727  5727

#define GOSSIP_HNF 16322
#define GOSSIP_SNF 16323
bool GossipHello_npc_neeru_fireblade(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(QUEST_5727) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HNF), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(4513, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_neeru_fireblade(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SNF), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(4513, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(QUEST_5727);
            break;
    }
    return true;
}

/*######
## npc_shenthul
######*/

#define RESTORE_ITEM_WHISTLE 16324
#define ITEM_WHISTLE 8066
#define QUEST_2460  2460

struct npc_shenthulAI : public ScriptedAI
{
    npc_shenthulAI(Creature* c) : ScriptedAI(c) {}

    bool CanTalk;
    bool CanEmote;
    uint32 Salute_Timer;
    uint32 Reset_Timer;
    uint64 playerGUID;

    void Reset()
    {
        CanTalk = false;
        CanEmote = false;
        Salute_Timer = 6000;
        Reset_Timer = 0;
        playerGUID = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (CanEmote)
        {
            
            if (Reset_Timer <= diff)
            {
                if (Player* temp = Unit::GetPlayerInWorld(playerGUID))
                    temp->FailQuest(QUEST_2460);
                Reset();
            }
            else
                Reset_Timer -= diff;
        }

        if (CanTalk && !CanEmote)
        {
            
            if (Salute_Timer <= diff)
            {
                m_creature->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                CanEmote = true;
                Reset_Timer = 60000;
            }
            else
                Salute_Timer -= diff;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_shenthul(Creature *_Creature)
{
    return new npc_shenthulAI (_Creature);
}

bool QuestAccept_npc_shenthul(Player* player, Creature* creature, Quest const* quest)
{
    if( quest->GetQuestId() == QUEST_2460 )
    {
        ((npc_shenthulAI*)creature->AI())->CanTalk = true;
        ((npc_shenthulAI*)creature->AI())->playerGUID = player->GetGUID();
    }
    return true;
}

bool ReciveEmote_npc_shenthul(Player *player, Creature *_Creature, uint32 emote)
{
    if( emote == TEXTEMOTE_SALUTE && player->GetQuestStatus(QUEST_2460) == QUEST_STATUS_INCOMPLETE )
        if( ((npc_shenthulAI*)_Creature->AI())->CanEmote )
    {
        player->AreaExploredOrEventHappens(QUEST_2460);
        ((npc_shenthulAI*)_Creature->AI())->Reset();
    }
    return true;
}

bool GossipHello_npc_shenthul(Player *player, Creature *_Creature)
{
    _Creature->prepareGossipMenu(player);

    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    if (player->GetQuestStatus(2458) == QUEST_STATUS_COMPLETE && !player->HasItemCount(ITEM_WHISTLE,1,true))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(RESTORE_ITEM_WHISTLE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_shenthul(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            if(player->GetQuestStatus(2458) == QUEST_STATUS_COMPLETE && !player->HasItemCount(ITEM_WHISTLE, 1, true))
            {
                ItemPosCountVec dest;
                if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_WHISTLE, 1) == EQUIP_ERR_OK)
                {
                    Item* item = player->StoreNewItem(dest, ITEM_WHISTLE, true);
                    player->SendNewItem(item, 1, true, false);
                }
            }
            break;
        }
        case GOSSIP_OPTION_TRAINER:
            player->SEND_TRAINERLIST(_Creature->GetGUID());
            break;
        case GOSSIP_OPTION_UNLEARNTALENTS:
            player->CLOSE_GOSSIP_MENU();
            player->SendTalentWipeConfirm(_Creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_thrall_warchief
######*/

#define QUEST_6566              6566

#define SPELL_CHAIN_LIGHTNING   16033
#define SPELL_SHOCK             16034

#define GOSSIP_HTW  16325
#define GOSSIP_STW1 16326
#define GOSSIP_STW2 16327
#define GOSSIP_STW3 16328
#define GOSSIP_STW4 16329
#define GOSSIP_STW5 16330
#define GOSSIP_STW6 16331

struct npc_thrall_warchiefAI : public ScriptedAI
{
    npc_thrall_warchiefAI(Creature* c) : ScriptedAI(c) {}

    Timer ChainLightning_Timer;
    Timer Shock_Timer;
    Timer EventMessengerTimer;
    uint64 PlayerGUID;
    uint8 EventMessengerPhase;
    bool EventMessenger;

    void Reset()
    {
        ChainLightning_Timer.Reset(2000);
        Shock_Timer.Reset(8000);
        EventMessengerTimer.Reset(0);
        PlayerGUID = 0;
        EventMessengerPhase = 0;
        EventMessenger = false;
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_SIT);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void EnterCombat(Unit *who) {}

    void StartEvent(Player* pPlayer)
    {
        PlayerGUID = pPlayer->GetGUID();
        EventMessenger = true;
        EventMessengerTimer = 2000;
        EventMessengerPhase = 0;
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        if (PointId == 100)
        {
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
            EventMessengerTimer = 1000;
            if(Creature* Eitrigg = GetClosestCreatureWithEntry(me, 3144, 100, true))
            {
                Eitrigg->GetMotionMaster()->MovePoint(100, 1924.421, -4140.29, 40.408);
                Eitrigg->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                Eitrigg->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }
            me->SetOrientation(4.81);
            me->SetFacingTo(4.81);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventMessenger)
        {
            if(EventMessengerTimer.Expired(diff))
            {
                switch(EventMessengerPhase)
                {
                    case 0:
                        me->Say(-1200428, LANG_UNIVERSAL, 0);
                        me->SetWalk(true);
                        me->GetMotionMaster()->MovePoint(100, 1923.987, -4135.793, 40.346);
                        EventMessengerPhase++;
                        EventMessengerTimer = 0;
                        break;
                    case 1:
                        me->Say(-1200429, LANG_UNIVERSAL, 0);
                        EventMessengerPhase++;
                        EventMessengerTimer = 1500;
                        break;
                    case 2:
                        me->Say(-1200430, LANG_UNIVERSAL, 0);
                        EventMessengerPhase++;
                        EventMessengerTimer = 1000;
                        break;
                    case 3:
                        if(Creature* Eitrigg = GetClosestCreatureWithEntry(me, 3144, 100, true))
                        {
                            Eitrigg->SetOrientation(Eitrigg->GetOrientationTo(me));
                            Eitrigg->SetFacingTo(Eitrigg->GetOrientationTo(me));
                        }
                        EventMessengerPhase++;
                        EventMessengerTimer = 500;
                        break;
                    case 4:
                        if(Creature* Eitrigg = GetClosestCreatureWithEntry(me, 3144, 100, true))
                            Eitrigg->Say(-1200431, LANG_UNIVERSAL, 0);
                        EventMessengerPhase++;
                        EventMessengerTimer = 3000;
                        break;
                    case 5:
                        me->Say(-1200432, LANG_UNIVERSAL, 0);
                        EventMessengerPhase++;
                        EventMessengerTimer = 5000;
                        break;
                    case 6:
                        if(Creature* Eitrigg = GetClosestCreatureWithEntry(me, 3144, 100, true))
                        {
                            Eitrigg->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
                            Eitrigg->Say(-1200433, LANG_UNIVERSAL, 0);
                        }
                        EventMessengerPhase++;
                        EventMessengerTimer = 5000;
                        break;
                    case 7:
                        me->Say(-1200434, LANG_UNIVERSAL, 0);
                        EventMessengerPhase++;
                        EventMessengerTimer = 5000;
                        break;
                    case 8:
                        if(Creature* Eitrigg = GetClosestCreatureWithEntry(me, 3144, 100, true))
                        {
                            Eitrigg->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
                            Eitrigg->GetMotionMaster()->MoveTargetedHome();
                            Eitrigg->Say(-1200435, LANG_UNIVERSAL, 0);
                        }
                        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
                        me->GetMotionMaster()->MoveTargetedHome();
                        EventMessengerPhase++;
                        EventMessengerTimer = 5000;
                        break;
                    case 9:
                        if(Creature* Eitrigg = GetClosestCreatureWithEntry(me, 3144, 100, true))
                        {
                            Eitrigg->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                            Eitrigg->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        }
                        me->SetOrientation(4.692);
                        me->SetFacingTo(4.692);
                        me->Say(-1200436, LANG_UNIVERSAL, 0);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        Reset();
                        break;
                }
            }
        }

        if(!UpdateVictim())
            return;

        if(ChainLightning_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CHAIN_LIGHTNING);
            ChainLightning_Timer = 9000;
        }

        if(Shock_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHOCK);
            Shock_Timer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_thrall_warchief(Creature *_Creature)
{
    return new npc_thrall_warchiefAI (_Creature);
}

bool QuestComplete_npc_thrall_warchief(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 9438)
    {
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
        ((npc_thrall_warchiefAI*)creature->AI())->StartEvent(player);
    }
    return true;
}

bool GossipHello_npc_thrall_warchief(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    if (player->GetQuestStatus(QUEST_6566) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HTW), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_thrall_warchief(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_STW1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(5733, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_STW2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(5734, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_STW3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(5735, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_STW4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(5736, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_STW5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(5737, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_STW6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
            player->SEND_GOSSIP_MENU(5738, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(QUEST_6566);
            break;
    }
    return true;
}

/*######
## npc_eitrigg
######*/

#define GOSSIP_HN  16332
#define GOSSIP_SN1 16333
#define GOSSIP_SN2 16334
#define GOSSIP_SN3 16335
#define GOSSIP_SN4 16336
#define GOSSIP_SN5 16337
#define GOSSIP_SN6 16338
#define GOSSIP_SN7 16339

#define QUEST_EITRIGGS_WISDOM    4941

bool GossipHello_npc_eitrigg(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(QUEST_EITRIGGS_WISDOM) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(3573, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_eitrigg(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(3574, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(3575, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(3576, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(3577, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(3578, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
            player->SEND_GOSSIP_MENU(3579, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+8);
            player->SEND_GOSSIP_MENU(3580, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+8:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(QUEST_EITRIGGS_WISDOM);
            break;
    }
    return true;
}

void AddSC_orgrimmar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_neeru_fireblade";
    newscript->pGossipHello =  &GossipHello_npc_neeru_fireblade;
    newscript->pGossipSelect = &GossipSelect_npc_neeru_fireblade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_shenthul";
    newscript->GetAI = &GetAI_npc_shenthul;
    newscript->pQuestAcceptNPC =  &QuestAccept_npc_shenthul;
    newscript->pReceiveEmote = &ReciveEmote_npc_shenthul;
    newscript->pGossipHello = &GossipHello_npc_shenthul;
    newscript->pGossipSelect = &GossipSelect_npc_shenthul;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_thrall_warchief";
    newscript->GetAI = &GetAI_npc_thrall_warchief;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_thrall_warchief;
    newscript->pGossipHello =  &GossipHello_npc_thrall_warchief;
    newscript->pGossipSelect = &GossipSelect_npc_thrall_warchief;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_eitrigg";
    newscript->pGossipHello =  &GossipHello_npc_eitrigg;
    newscript->pGossipSelect = &GossipSelect_npc_eitrigg;
    newscript->RegisterSelf();
}

