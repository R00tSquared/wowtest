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
SDName: Blasted_Lands
SD%Complete: 90
SDComment: Quest support: 2784, 2801, 3628. Missing some texts for Fallen Hero. Teleporter to Rise of the Defiler missing group support.
SDCategory: Blasted Lands
EndScriptData */

/* ContentData
npc_deathly_usher
npc_fallen_hero_of_horde
EndContentData */

#include "precompiled.h"

/*######
## npc_deathly_usher
######*/

#define GOSSIP_ITEM_USHER               16077

#define SPELL_TELEPORT_SINGLE           12885
#define SPELL_TELEPORT_SINGLE_IN_GROUP  13142
#define SPELL_TELEPORT_GROUP            27686

bool GossipHello_npc_deathly_usher(Player *player, Creature *_Creature)
{
    if(player->GetQuestStatus(3628) == QUEST_STATUS_INCOMPLETE && player->HasItemCount(10757, 1))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_USHER), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_deathly_usher(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if(action = GOSSIP_ACTION_INFO_DEF)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, SPELL_TELEPORT_SINGLE, true);
    }

    return true;
}

/*######
## npc_fallen_hero_of_horde
######*/
#define GOSSIP_H_F1         16078
#define GOSSIP_H_F2         16079

#define GOSSIP_ITEM_FALLEN  16080

#define GOSSIP_ITEM_FALLEN1 16081
#define GOSSIP_ITEM_FALLEN2 16082
#define GOSSIP_ITEM_FALLEN3 16083
#define GOSSIP_ITEM_FALLEN4 16084
#define GOSSIP_ITEM_FALLEN5 16085

bool GossipHello_npc_fallen_hero_of_horde(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(2784) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_H_F1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    if (player->GetQuestStatus(2801) == QUEST_STATUS_INCOMPLETE && player->GetTeam() == HORDE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_H_F2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    if (player->GetQuestStatus(2801) == QUEST_STATUS_INCOMPLETE && player->GetTeam() == ALLIANCE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_H_F1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_fallen_hero_of_horde(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            player->SEND_GOSSIP_MENU(1392, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            player->SEND_GOSSIP_MENU(1411, _Creature->GetGUID());
            if (player->GetQuestStatus(2784) == QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(2784);
            if (player->GetTeam() == ALLIANCE)
            {
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(1411, _Creature->GetGUID());
            }
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            player->SEND_GOSSIP_MENU(1451, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            player->SEND_GOSSIP_MENU(1452, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
            player->SEND_GOSSIP_MENU(1453, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+23:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
            player->SEND_GOSSIP_MENU(1454, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+24:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
            player->SEND_GOSSIP_MENU(1455, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+25:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_FALLEN5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 26);
            player->SEND_GOSSIP_MENU(1456, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+26:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(2801);
            break;
    }
    return true;
}

struct npc_8022AI : public ScriptedAI
{
    npc_8022AI(Creature* pCreature) : ScriptedAI(pCreature)
    {
    }

    bool underEvent;
    bool underTrance;
    bool underSpeak;
    bool sentence;
    bool createObject;
    bool isStun;
    uint32 SpeakTimer;
    uint32 BeginTimer;
    uint32 TranceTimer;
    uint64 grimshadeGUID;
    uint64 playerGUID;

    void Reset()
    {
        underEvent = false;
        BeginTimer = 3000;
        TranceTimer = 8000;
        grimshadeGUID = 0;
        underTrance = false;
        underSpeak = false;
        SpeakTimer = 8000;
        sentence = false;
        createObject = false;
        isStun = false;
        playerGUID = 0;
    }

    void QuestCompleted(Player* pPlayer, Quest const* pQuest)
    {
        if (!underEvent)
        {
            playerGUID = pPlayer->GetGUID();
            underEvent = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (underEvent)
        {
            if (!createObject)
            {
                if (GameObject* pGo = me->SummonGameObject(144069, -10999.166992f, -3484.187012f, 103.127243f, 2.592681f, 0, 0, 0, 0, 100000))
                    grimshadeGUID = pGo->GetGUID();
                createObject = true;
            }
            if (BeginTimer < diff)
                underTrance = true;
            else
                BeginTimer -= diff;
        }

        if (underTrance)
        {
            if (!isStun)
            {
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STUN);
                isStun = true;
            }
            if (TranceTimer < diff)
                underSpeak = true;
            else
                TranceTimer -= diff;

        }

        if (underSpeak)
        {
            if (!sentence)
            {
                me->MonsterSay(-1200175, 0, 0);
                sentence = true;
            }
            if (SpeakTimer < diff)
            {
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
                if (GameObject* pVision = me->GetMap()->GetGameObject(grimshadeGUID))
                    pVision->Delete();

                underEvent = false;
                BeginTimer = 3000;
                TranceTimer = 3000;
                grimshadeGUID = 0;
                underTrance = false;
                underSpeak = false;
                SpeakTimer = 3000;
                sentence = false;
                createObject = false;
                isStun = false;
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                if (Player* player = me->GetPlayerInWorld(playerGUID))
                {
                    if (player->GetQuestStatus(2992) == QUEST_STATUS_INCOMPLETE)
                        player->AreaExploredOrEventHappens(2992);
                }
            }
            else
                SpeakTimer -= diff;
        }
    }
};

CreatureAI* GetAI_npc_8022(Creature* pCreature)
{
    return new npc_8022AI(pCreature);
}

bool QuestAccept_npc_8022(Player* pPlayer, Creature* pQuestGiver, Quest const* pQuest)
{
    if (pQuest->GetQuestId() != 2992)
        return false;

    if (npc_8022AI* pThadius = dynamic_cast<npc_8022AI*>(pQuestGiver->AI()))
    {
        pQuestGiver->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        pThadius->QuestCompleted(pPlayer, pQuest);
    }
    return true;
}

void AddSC_blasted_lands()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_8022";
    newscript->GetAI = &GetAI_npc_8022;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_8022;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_deathly_usher";
    newscript->pGossipHello =  &GossipHello_npc_deathly_usher;
    newscript->pGossipSelect = &GossipSelect_npc_deathly_usher;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_fallen_hero_of_horde";
    newscript->pGossipHello =  &GossipHello_npc_fallen_hero_of_horde;
    newscript->pGossipSelect = &GossipSelect_npc_fallen_hero_of_horde;
    newscript->RegisterSelf();
}

