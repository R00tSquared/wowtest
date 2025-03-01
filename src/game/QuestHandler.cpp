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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "QuestDef.h"
#include "ObjectAccessor.h"
#include "ScriptMgr.h"
#include "Group.h"
#include "BattleGround.h"
#include "BattleGroundAV.h"
#include "CreatureAI.h"
#include "Chat.h"

// Release HACK
#define NPC_FRANCLORN_FORGEWRIGHT 8888
#define NPC_GAERIYAN 9299

void WorldSession::HandleQuestgiverStatusQueryOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;
    uint8 questStatus = DIALOG_STATUS_NONE;
    uint8 defstatus = DIALOG_STATUS_NONE;

    Object* questgiver = _player->GetMap()->GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if (!questgiver)
    {
        sLog.outDetail("Error in CMSG_QUESTGIVER_STATUS_QUERY, called for not found questgiver (Typeid: %u GUID: %u)",GuidHigh2TypeId(GUID_HIPART(guid)),GUID_LOPART(guid));
        return;
    }

    switch (questgiver->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY for npc, guid = %u",uint32(GUID_LOPART(guid)));
            Creature* cr_questgiver=(Creature*)questgiver;
            if (!cr_questgiver->IsHostileTo(_player))       // not show quest status to enemies
            {
                questStatus = sScriptMgr.GetDialogStatus(_player, cr_questgiver);
                if (questStatus == DIALOG_STATUS_SCRIPTED_NO_STATUS)
                    questStatus = getDialogStatus(_player, cr_questgiver, defstatus);
            }
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_STATUS_QUERY for GameObject guid = %u",uint32(GUID_LOPART(guid)));
            GameObject* go_questgiver=(GameObject*)questgiver;
            questStatus = sScriptMgr.GetDialogStatus(_player, go_questgiver);
            if (questStatus == DIALOG_STATUS_SCRIPTED_NO_STATUS)
                questStatus = getDialogStatus(_player, go_questgiver, defstatus);
            break;
        }
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: QuestGiver called for unexpected type %u", questgiver->GetTypeId());
            break;
    }

    //inform client about status of quest
    _player->PlayerTalkClass->SendQuestGiverStatus(questStatus, guid);
}

void WorldSession::HandleQuestgiverHelloOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    sLog.outDebug ("WORLD: Received CMSG_QUESTGIVER_HELLO npc = %u", GUID_LOPART(guid));

    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!pCreature)
    {
        sLog.outDebug ("WORLD: HandleQuestgiverHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.",
            GUID_LOPART(guid));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // Stop the npc if moving

    // new year kobolds
    if (pCreature->GetEntry() >= 690000 && pCreature->GetEntry() <= 690003)
    {
        if (!pCreature->HasAura(55154))
        {
            ChatHandler(GetPlayer()).SendSysMessage(16651);
            return;
        }
    }
    else
        pCreature->StopMoving();

    if (sScriptMgr.OnGossipHello(_player, pCreature))
        return;

    pCreature->prepareGossipMenu(_player);
    pCreature->sendPreparedGossip(_player);
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 guid;
    uint32 quest;
    recv_data >> guid >> quest;

    if (GetPlayer()->isAlive() == ((GUID_ENPART(guid) == NPC_GAERIYAN) || (GUID_ENPART(guid) == NPC_FRANCLORN_FORGEWRIGHT)))
        return;

    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest);

    Object* pObject = _player->GetMap()->GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT|TYPEMASK_ITEM|TYPEMASK_PLAYER);

    // no or incorrect quest giver
    if (!pObject
        || (pObject->GetTypeId()!=TYPEID_PLAYER && !pObject->hasQuest(quest))
        || (pObject->GetTypeId()==TYPEID_PLAYER && !((Player*)pObject)->CanShareQuest(quest))
       )
    {
        _player->PlayerTalkClass->CloseGossip();
        _player->SetDivider(0);
        return;
    }

    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest);
    if (qInfo)
    {
        // prevent cheating
        if (!GetPlayer()->CanTakeQuest(qInfo,true))
        {
            _player->PlayerTalkClass->CloseGossip();
            _player->SetDivider(0);
            return;
        }

        if (_player->GetDivider() != 0)
        {
            Player *pPlayer = ObjectAccessor::GetPlayerInWorld(_player->GetDivider());
            if (pPlayer)
            {
                pPlayer->SendPushToPartyResponse(_player, QUEST_PARTY_MSG_ACCEPT_QUEST);
                _player->SetDivider(0);
            }
        }

        if (_player->CanAddQuest(qInfo, true))
        {
            _player->AddQuest(qInfo, pObject);

            if (qInfo->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            {
                if (Group* pGroup = _player->GetGroup())
                {
                    for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                    {
                        Player* pPlayer = itr->getSource();

                        if (!pPlayer || pPlayer == _player)     // not self
                            continue;

                        if (pPlayer->CanTakeQuest(qInfo, true))
                        {
                            pPlayer->SetDivider(_player->GetGUID());

                            //need confirmation that any gossip window will close
                            pPlayer->PlayerTalkClass->CloseGossip();

                            _player->SendQuestConfirmAccept(qInfo, pPlayer);
                        }
                    }
                }
            }

            if (_player->CanCompleteQuest(quest))
                _player->CompleteQuest(quest);

            switch (pObject->GetTypeId())
            {
                case TYPEID_UNIT:
                    sScriptMgr.OnQuestAccept(_player, ((Creature*)pObject), qInfo);
                    //((Creature*)pObject)->AI()->OnQuestAccept(_player, qInfo);
                    break;
                case TYPEID_ITEM:
                case TYPEID_CONTAINER:
                {
                    sScriptMgr.OnQuestAccept(_player, ((Item*)pObject), qInfo);

                    // destroy not required for quest finish quest starting item

					ItemPrototype const* proto = ((Item*)pObject)->GetProto();
					if (proto && proto->InventoryType == 0)
					{
						bool destroyItem = true;
						for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
						{
							if ((qInfo->ReqItemId[i] == ((Item*)pObject)->GetEntry()) && proto->MaxCount > 0)
							{
								destroyItem = false;
								break;
							}
						}

						if (destroyItem)
							_player->DestroyItem(((Item*)pObject)->GetBagSlot(), ((Item*)pObject)->GetSlot(), true, "ACCEPT_QUEST_DESTROY");
					}

                    break;
                }
                case TYPEID_GAMEOBJECT:
                    sScriptMgr.OnQuestAccept(_player, ((GameObject*)pObject), qInfo);
                    break;
            }
            _player->PlayerTalkClass->CloseGossip();

            if (qInfo->GetSrcSpell() > 0)
                _player->CastSpell(_player, qInfo->GetSrcSpell(), true);

            return;
        }
    }

    _player->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleQuestgiverQuestQueryOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 guid;
    uint32 quest;
    recv_data >> guid >> quest;
    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_QUERY_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest);

    // Verify that the guid is valid and is a questgiver or involved in the requested quest
    Object* pObject = _player->GetMap()->GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT|TYPEMASK_ITEM);
    if (!pObject||!pObject->hasQuest(quest) && !pObject->hasInvolvedQuest(quest))
    {
        _player->PlayerTalkClass->CloseGossip();
        return;
    }

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest);
    if (pQuest)
    {
        _player->PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, pObject->GetGUID(), true);
    }
}

void WorldSession::HandleQuestQueryOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4);

    uint32 quest;
    recv_data >> quest;
    sLog.outDebug("WORLD: Received CMSG_QUEST_QUERY quest = %u",quest);

    Quest const *pQuest = sObjectMgr.GetQuestTemplate(quest);
    if (pQuest)
    {
        _player->PlayerTalkClass->SendQuestQueryResponse(pQuest);
    }
}

void WorldSession::HandleQuestgiverChooseRewardOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4+4);

    uint32 quest, reward;
    uint64 guid;
    recv_data >> guid >> quest >> reward;

    if (reward >= QUEST_REWARD_CHOICES_COUNT)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Error in CMSG_QUESTGIVER_CHOOSE_REWARD: player %s (guid %d) tried to get invalid reward (%u) (probably packet hacking)", _player->GetName(), _player->GetGUIDLow(), reward);
        return;
    }

    if (GetPlayer()->isAlive() == ((GUID_ENPART(guid) == NPC_GAERIYAN) || (GUID_ENPART(guid) == NPC_FRANCLORN_FORGEWRIGHT)))
        return;

    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_CHOOSE_REWARD npc = %u, quest = %u, reward = %u",uint32(GUID_LOPART(guid)),quest,reward);

    Object* pObject = _player->GetMap()->GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if (!pObject)
        return;

    if (!pObject->hasInvolvedQuest(quest))
        return;

    if (Quest const *pQuest = sObjectMgr.GetQuestTemplate(quest))
    {
        if ((!_player->CanSeeStartQuest(pQuest) &&  _player->GetQuestStatus(quest) == QUEST_STATUS_NONE) ||
            (_player->GetQuestStatus(quest) != QUEST_STATUS_COMPLETE && !pQuest->IsAutoComplete()))
        {
            sLog.outLog(LOG_DEFAULT,"HACK ALERT: Player %s (guid: %u) is trying to complete quest (id: %u) but he has no right to do it!",
                            _player->GetName(), _player->GetGUIDLow(), quest);
            return;
        }

        if (_player->CanRewardQuest(pQuest, reward, true))
        {
            _player->RewardQuest(pQuest, reward, pObject);

            switch (pObject->GetTypeId())
            {
                case TYPEID_UNIT:
                    // Send next quest
                    if (Quest const* nextquest = _player->GetNextQuest(guid ,pQuest))
                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextquest,guid,true);
                    break;
                case TYPEID_GAMEOBJECT:
                    // Send next quest
                    if (Quest const* nextquest = _player->GetNextQuest(guid ,pQuest))
                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextquest,guid,true);
                    break;
            }
        }
        else
            _player->PlayerTalkClass->SendQuestGiverOfferReward(pQuest, guid, true);
    }
}

void WorldSession::HandleQuestgiverRequestRewardOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint32 quest;
    uint64 guid;
    recv_data >> guid >> quest;

    if (GetPlayer()->isAlive() == ((GUID_ENPART(guid) == NPC_GAERIYAN) || (GUID_ENPART(guid) == NPC_FRANCLORN_FORGEWRIGHT)))
        return;

    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_REQUEST_REWARD npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest);

    Object* pObject = _player->GetMap()->GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if (!pObject||!pObject->hasInvolvedQuest(quest))
        return;

    if (_player->CanCompleteQuest(quest))
        _player->CompleteQuest(quest);

    if (_player->GetQuestStatus(quest) != QUEST_STATUS_COMPLETE)
        return;

    if (Quest const *pQuest = sObjectMgr.GetQuestTemplate(quest))
        _player->PlayerTalkClass->SendQuestGiverOfferReward(pQuest, guid, true);
}

void WorldSession::HandleQuestgiverCancel(WorldPacket& /*recv_data*/)
{
    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_CANCEL");

    _player->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleQuestLogSwapQuest(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1+1);

    uint8 slot1, slot2;
    recv_data >> slot1 >> slot2;

    if (slot1 == slot2 || slot1 >= MAX_QUEST_LOG_SIZE || slot2 >= MAX_QUEST_LOG_SIZE)
        return;

    sLog.outDebug("WORLD: Received CMSG_QUESTLOG_SWAP_QUEST slot 1 = %u, slot 2 = %u", slot1, slot2);

    GetPlayer()->SwapQuestSlot(slot1,slot2);
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    uint8 slot;
    recv_data >> slot;

    sLog.outDebug("WORLD: Received CMSG_QUESTLOG_REMOVE_QUEST slot = %u",slot);

    if (slot < MAX_QUEST_LOG_SIZE)
    {
        if (uint32 quest = _player->GetQuestSlotQuestId(slot))
        {
            if (!_player->TakeQuestSourceItem(quest, true))
                return;                                     // can't un-equip some items, reject quest cancel

            _player->SetQuestStatus(quest, QUEST_STATUS_NONE);
        }

        _player->SetQuestSlot(slot, 0);
    }
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recv_data)
{
    uint32 quest;
    recv_data >> quest;

    sLog.outDebug("WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT quest = %u", quest);

    if (const Quest* pQuest = sObjectMgr.GetQuestTemplate(quest))
    {
        if (!pQuest->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            return;

        Player* pOriginalPlayer = ObjectAccessor::GetPlayerInWorld(_player->GetDivider());

        if (!pOriginalPlayer)
            return;

        if (pQuest->GetType() == QUEST_TYPE_RAID)
        {
            if (!_player->IsInSameRaidWith(pOriginalPlayer))
                return;
        }
        else
        {
            if (!_player->IsInSameGroupWith(pOriginalPlayer))
                return;
        }

        if (_player->CanAddQuest(pQuest, true))
            _player->AddQuest(pQuest, NULL);                // NULL, this prevent DB script from duplicate running

        _player->SetDivider(0);
    }
}

void WorldSession::HandleQuestComplete(WorldPacket& recv_data)
{
    uint32 quest;
    uint64 guid;
    recv_data >> guid >> quest;

    if (GetPlayer()->isAlive() == ((GUID_ENPART(guid) == NPC_GAERIYAN) || (GUID_ENPART(guid) == NPC_FRANCLORN_FORGEWRIGHT)))
        return;

    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_COMPLETE_QUEST npc = %u, quest = %u",uint32(GUID_LOPART(guid)),quest);

    Quest const *pQuest = sObjectMgr.GetQuestTemplate(quest);
    if (pQuest)
    {
        if (_player->GetQuestStatus(quest) != QUEST_STATUS_COMPLETE)
        {
            if (pQuest->IsRepeatable())
                _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, _player->CanCompleteRepeatableQuest(pQuest), false);
            else
                _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, _player->CanRewardQuest(pQuest,false), false);
        }
        else
            _player->PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, _player->CanRewardQuest(pQuest,false), false);
    }
}

void WorldSession::HandleQuestAutoLaunch(WorldPacket& /*recvPacket*/)
{
    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_QUEST_AUTOLAUNCH (Send your log to anakin if you see this message)");
}

void WorldSession::HandleQuestPushToParty(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,4);

    uint32 quest;
    recvPacket >> quest;

    sLog.outDebug("WORLD: Received CMSG_PUSHQUESTTOPARTY quest = %u", quest);

    Quest const *pQuest = sObjectMgr.GetQuestTemplate(quest);
    if (pQuest)
    {
        if (_player->GetGroup())
        {
            Group *pGroup = _player->GetGroup();
            if (pGroup)
            {
                for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                {
                    Player *pPlayer = itr->getSource();
                    if (!pPlayer || pPlayer == _player)     // skip self
                        continue;

                    _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_SHARING_QUEST);

                    if (_player->GetDistance(pPlayer) > 10)
                    {
                        _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_TOO_FAR);
                        continue;
                    }

                    if (!pPlayer->SatisfyQuestStatus(pQuest, false))
                    {
                        _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_HAVE_QUEST);
                        continue;
                    }

                    if (pPlayer->GetQuestStatus(quest) == QUEST_STATUS_COMPLETE)
                    {
                        _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_FINISH_QUEST);
                        continue;
                    }

                    if (!pPlayer->CanTakeQuest(pQuest, false))
                    {
                        _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_CANT_TAKE_QUEST);
                        continue;
                    }

                    if (!pPlayer->SatisfyQuestLog(false))
                    {
                        _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_LOG_FULL);
                        continue;
                    }

                    if (pPlayer->GetDivider() != 0 )
                    {
                        _player->SendPushToPartyResponse(pPlayer, QUEST_PARTY_MSG_BUSY);
                        continue;
                    }

                    pPlayer->PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, _player->GetGUID(), true);
                    pPlayer->SetDivider(_player->GetGUID());
                }
            }
        }
    }
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket,8+1);

    uint64 guid;
    uint8 msg;
    recvPacket >> guid >> msg;

    sLog.outDebug("WORLD: Received MSG_QUEST_PUSH_RESULT");

    if (_player->GetDivider() != 0)
    {
        Player *pPlayer = ObjectAccessor::GetPlayerInWorld(_player->GetDivider());
        if (pPlayer)
        {
            WorldPacket data(MSG_QUEST_PUSH_RESULT, (8+1));
            data << uint64(guid);
            data << uint8(msg);                             // valid values: 0-8
            pPlayer->SendPacketToSelf(&data);
            _player->SetDivider(0);
        }
    }
}

uint32 WorldSession::getDialogStatus(Player *pPlayer, Object* questgiver, uint32 defstatus)
{
    uint32 result = defstatus;

    QuestRelations const* qir = nullptr;
    QuestRelations const* qr = nullptr;

    switch (questgiver->GetTypeId())
    {
        case TYPEID_GAMEOBJECT:
        {
            qir = &sObjectMgr.mGOQuestInvolvedRelations;
            qr  = &sObjectMgr.mGOQuestRelations;
            break;
        }
        case TYPEID_UNIT:
        {
            switch (((Unit*)questgiver)->GetEntry()) 
            {
            case 693020: // Izuar
            case 693130: // Thaldrion
                return DIALOG_STATUS_REWARD;
            }
            
            qir = &sObjectMgr.mCreatureQuestInvolvedRelations;
            qr  = &sObjectMgr.mCreatureQuestRelations;
            break;
        }
        default:
            //its imposible, but check ^)
            sLog.outLog(LOG_DEFAULT, "ERROR: Warning: GetDialogStatus called for unexpected type %u", questgiver->GetTypeId());
            return DIALOG_STATUS_NONE;
    }

    for (QuestRelations::const_iterator i = qir->lower_bound(questgiver->GetEntry()); i != qir->upper_bound(questgiver->GetEntry()); ++i)
    {
        uint32 result2 = 0;
        uint32 quest_id = i->second;
        Quest const *pQuest = sObjectMgr.GetQuestTemplate(quest_id);
        if (!pQuest) continue;

        QuestStatus status = pPlayer->GetQuestStatus(quest_id);
        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(quest_id)) ||
            (pQuest->IsAutoComplete() && pPlayer->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
                result2 = DIALOG_STATUS_REWARD_REP;
            else
                result2 = DIALOG_STATUS_REWARD;
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
            result2 = DIALOG_STATUS_INCOMPLETE;

        if (result2 > result)
            result = result2;
    }

    for (QuestRelations::const_iterator i = qr->lower_bound(questgiver->GetEntry()); i != qr->upper_bound(questgiver->GetEntry()); ++i)
    {
        uint32 result2 = 0;
        uint32 quest_id = i->second;
        Quest const *pQuest = sObjectMgr.GetQuestTemplate(quest_id);
        if (!pQuest)
            continue;

        QuestStatus status = pPlayer->GetQuestStatus(quest_id);
        if (status == QUEST_STATUS_NONE)
        {
            if (pPlayer->CanSeeStartQuest(pQuest))
            {
                if (pPlayer->SatisfyQuestLevel(pQuest, false))
                {
                    if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && pPlayer->getQuestStatusMap()[quest_id].m_rewarded))
                        result2 = DIALOG_STATUS_REWARD_REP;
                    else if (pPlayer->GetLevel() <= pPlayer->GetQuestOrPlayerLevel(pQuest) + sWorld.getConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF))
                    {
                        if (pQuest->HasFlag(QUEST_FLAGS_DAILY))
                            result2 = DIALOG_STATUS_AVAILABLE_REP;
                        else
                            result2 = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                        result2 = DIALOG_STATUS_CHAT;
                }
                else
                    result2 = DIALOG_STATUS_UNAVAILABLE;
            }
        }

        if (result2 > result)
            result = result2;
    }

    return result;
}

void WorldSession::HandleQuestgiverStatusQueryMultipleOpcode(WorldPacket& /*recvPacket*/)
{
    sLog.outDebug("WORLD: Received CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY");

    _player->SendQuestGiverStatusMultiple();
}

