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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "CellImpl.h"
#include "InstanceSaveMgr.h"
#include "Util.h"
#include "TicketMgr.h"
#include "GridMap.h"
#include "Guild.h"
#include "AccountMgr.h"
#include "SocialMgr.h"
#include "GuildMgr.h"

bool ChatHandler::HandleNpcSayCommand(const char* args)
{
    if (!*args)
        return false;

    Creature* pCreature = getSelectedCreature();
    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->Say(args, LANG_UNIVERSAL, 0);

    // make some emotes
    char lastchar = args[strlen(args) - 1];
    switch (lastchar)
    {
        case '?':   pCreature->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);      break;
        case '!':   pCreature->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);   break;
        default:    pCreature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);          break;
    }

    return true;
}

bool ChatHandler::HandleNpcYellCommand(const char* args)
{
    if (!*args)
        return false;

    Creature* pCreature = getSelectedCreature();
    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->Yell(args, LANG_UNIVERSAL, 0);

    // make an emote
    pCreature->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);

    return true;
}

//show text emote by creature in chat
bool ChatHandler::HandleNpcTextEmoteCommand(const char* args)
{
    if (!*args)
        return false;

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->TextEmote(args, 0);

    return true;
}

// make npc whisper to player
bool ChatHandler::HandleNpcWhisperCommand(const char* args)
{
    if (!*args)
        return false;

    char* receiver_str = strtok((char*)args, " ");
    char* text = strtok(NULL, "");

    uint64 guid = m_session->GetPlayer()->GetSelection();
    Creature* pCreature = m_session->GetPlayer()->GetMap()->GetCreature(guid);

    if (!pCreature || !receiver_str || !text)
    {
        return false;
    }

    uint64 receiver_guid= atol(receiver_str);

    pCreature->Whisper(text,receiver_guid);

    return true;
}

bool ChatHandler::HandleNameAnnounceCommand(const char* args)
{
    WorldPacket data;
    if (!*args)
        return false;
    //char str[1024];
    //sprintf(str, GetHellgroundString(LANG_ANNOUNCE_COLOR), m_session->GetPlayer()->GetName(), args);
    sWorld.SendWorldText(LANG_ANNOUNCE_COLOR, 0, m_session->GetPlayer()->GetName(), args);
    return true;
}

bool ChatHandler::HandleHDevAnnounceCommand(const char* args)
{
    WorldPacket data;
    if (!*args)
        return false;

    sWorld.SendWorldText(LANG_HDEV_ANNOUNCE_COLOR, 0, m_session->GetPlayer()->GetName(), args);
    return true;
}

bool ChatHandler::HandleGMNameAnnounceCommand(const char* args)
{
    WorldPacket data;
    if (!*args)
        return false;

    sWorld.SendGMText(LANG_GM_ANNOUNCE_COLOR, m_session->GetPlayer()->GetName(), args);
    return true;
}

// global announce
bool ChatHandler::HandleAnnounceCommand(const char* args)
{
    if (!*args)
        return false;

    sWorld.SendWorldText(LANG_SYSTEMMESSAGE, 0, args);
    return true;
}

// global announce
bool ChatHandler::HandleAnnIdCommand(const char* args)
{
    if (!*args)
        return false;

    int32 id = atoi((char*)args);
    if (!id)
        return false;

    sWorld.LoadAnnounces(); // reload announces table

    if (sObjectMgr.GetHellgroundString(id, sObjectMgr.GetIndexForLocale(LOCALE_enUS)) == "<error>") // nothing found
        return false;

    sWorld.SendWorldText(id, 0);
    return true;
}

// announce to logged in GMs
bool ChatHandler::HandleGMAnnounceCommand(const char* args)
{
    if (!*args)
        return false;

    sWorld.SendGMText(LANG_GM_BROADCAST,args);
    return true;
}

//notification player at the screen
bool ChatHandler::HandleNotifyCommand(const char* args)
{
    if (!*args)
        return false;

    std::string str = GetHellgroundString(LANG_GLOBAL_NOTIFY);
    str += args;

    WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
    data << str;
    sWorld.SendGlobalMessage(&data);

    return true;
}

//notification GM at the screen
bool ChatHandler::HandleGMNotifyCommand(const char* args)
{
    if (!*args)
        return false;

    std::string str = GetHellgroundString(LANG_GM_NOTIFY);
    str += args;

    WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
    data << str;
    sWorld.SendGlobalGMMessage(&data);

    return true;
}

//Enable\Dissable GM Mode
bool ChatHandler::HandleGMCommand(const char* args)
{
    if (!*args)
    {
        if (m_session->GetPlayer()->isGameMaster())
            m_session->SendNotification(LANG_GM_ON);
        else
            m_session->SendNotification(LANG_GM_OFF);
        return true;
    }

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        m_session->GetPlayer()->SetGameMaster(true);
        m_session->SendNotification(LANG_GM_ON);
        return true;
    }

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetGameMaster(false);
        m_session->SendNotification(LANG_GM_OFF);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

// Enables or disables hiding of the staff badge
bool ChatHandler::HandleGMChatCommand(const char* args)
{
    if (!*args)
    {
        if (m_session->GetPlayer()->isGMChat())
            m_session->SendNotification(LANG_GM_CHAT_ON);
        else
            m_session->SendNotification(LANG_GM_CHAT_OFF);
        return true;
    }

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        m_session->GetPlayer()->SetGMChat(true);
        m_session->SendNotification(LANG_GM_CHAT_ON);
        return true;
    }

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetGMChat(false);
        m_session->SendNotification(LANG_GM_CHAT_OFF);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

const char* ChatHandler::PGetSysMessage(int32 entry, ...)
{
    const char* format = GetHellgroundString(entry);
    va_list ap;
    char* str = new char[1024];
    va_start(ap, entry);
    vsnprintf(str, 1024, format, ap);
    va_end(ap);
    return str;
}

bool ChatHandler::HandleGMTicketListCommand(const char* args)
{
    SendSysMessage(LANG_COMMAND_TICKETSHOWLIST);
    for (GmTicketList::iterator itr = sTicketMgr.GM_TicketOpenList.begin(); itr != sTicketMgr.GM_TicketOpenList.end(); ++itr)
    {
        std::string gmname;
        std::stringstream ss;

        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, (*itr)->guid);
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, ((*itr)->name).c_str());
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGECREATE, WorldSession::secondsToTimeString(m_session, time(NULL) - (*itr)->createtime, true, false).c_str());
        if ((*itr)->updatetime)
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGE, WorldSession::secondsToTimeString(m_session, time(NULL) - (*itr)->updatetime, true, false).c_str());
        if (sObjectMgr.GetPlayerNameByGUID((*itr)->assignedToGM, gmname))
        {
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
        }
        SendSysMessage(ss.str().c_str());
    }
    return true;
}


bool ChatHandler::HandleGMTicketListOnlineCommand(const char* args)
{
    SendSysMessage(LANG_COMMAND_TICKETSHOWONLINELIST);
    for (GmTicketList::iterator itr = sTicketMgr.GM_TicketOpenList.begin(); itr != sTicketMgr.GM_TicketOpenList.end(); ++itr)
    {
        if (!sObjectAccessor.GetPlayerInWorldOrNot((*itr)->playerGuid))
            continue;

        std::string gmname;
        std::stringstream ss;

        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, (*itr)->guid);
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, ((*itr)->name).c_str());
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGECREATE, (WorldSession::secondsToTimeString(m_session, time(NULL) - (*itr)->createtime, true, false)).c_str());
        if ((*itr)->updatetime)
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGE, (WorldSession::secondsToTimeString(m_session, time(NULL) - (*itr)->updatetime, true, false)).c_str());
        if (sObjectMgr.GetPlayerNameByGUID((*itr)->assignedToGM, gmname))
        {
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
        }
        SendSysMessage(ss.str().c_str());
    }
    return true;
}
bool ChatHandler::HandleGMTicketListClosedCommand(const char* /*args*/)
{
    SendSysMessage(LANG_COMMAND_TICKETSHOWCLOSEDLIST);
    for (GmTicketList::iterator itr = sTicketMgr.GM_TicketClosedList.begin(); itr != sTicketMgr.GM_TicketClosedList.end(); ++itr)
    {
        std::string gmname;
        std::stringstream ss;

        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, (*itr)->guid);
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, ((*itr)->name).c_str());
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGECREATE, (WorldSession::secondsToTimeString(m_session, time(NULL) - (*itr)->createtime, true, false)).c_str());
        if ((*itr)->updatetime)
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGE, (WorldSession::secondsToTimeString(m_session, time(NULL) - (*itr)->updatetime, true, false)).c_str());
        if (sObjectMgr.GetPlayerNameByGUID((*itr)->assignedToGM, gmname))
        {
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
        }
        SendSysMessage(ss.str().c_str());
    }
    return true;
}

//bool ChatHandler::HandleGMTicketGetByIdCommand(const char* args)
//{
//    if (!*args)
//        return false;
//
//    uint64 tguid = atoi(args);
//    GM_Ticket *ticket = sTicketMgr.GetGMTicketAny(tguid);
//    if (!ticket)
//    {
//        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
//        return true;
//    }
//
//    std::string gmname;
//    std::stringstream ss;
//
//    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
//    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
//    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGECREATE, (WorldSession::secondsToTimeString(m_session, time(NULL) - ticket->createtime, true, false)).c_str());
//    if (ticket->updatetime)
//        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGE, (WorldSession::secondsToTimeString(m_session, time(NULL) - ticket->updatetime, true, false)).c_str());
//    if (sObjectMgr.GetPlayerNameByGUID(ticket->assignedToGM, gmname))
//    {
//        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
//    }
//    ss <<  PGetSysMessage(LANG_COMMAND_TICKETLISTMESSAGE, ticket->message.c_str());
//    if (ticket->comment != "")
//        ss <<  PGetSysMessage(LANG_COMMAND_TICKETLISTCOMMENT, ticket->comment.c_str());
//    
//    if (ticket->response != "")
//        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTRESPONSE, ticket->response.c_str());
//
//    SendSysMessage(ss.str().c_str());
//    return true;
//}

bool ChatHandler::HandleGMTicketGetByNameCommand(const char* args)
{
    if (!*args)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpenByName(args);
    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    std::string gmname;
    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGECREATE, (WorldSession::secondsToTimeString(m_session, time(NULL) - ticket->createtime, true, false)).c_str());
    if (ticket->updatetime)
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGE, (WorldSession::secondsToTimeString(m_session, time(NULL) - ticket->updatetime, true, false)).c_str());
    if (sObjectMgr.GetPlayerNameByGUID(ticket->assignedToGM, gmname))
    {
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
    }
    ss <<  PGetSysMessage(LANG_COMMAND_TICKETLISTMESSAGE, ticket->message.c_str());
    if (ticket->comment != "")
    {
        ss <<  PGetSysMessage(LANG_COMMAND_TICKETLISTCOMMENT, ticket->comment.c_str());
    }
    SendSysMessage(ss.str().c_str());
    return true;
}

bool ChatHandler::HandleGMTicketHistoryCommand(const char* args)
{
    if (!*args)
        return false;

    GmTicketList tickets = sTicketMgr.GetGMTicketsAllByName(args);
    if (tickets.empty())
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    std::stringstream ss;

    for (GmTicketList::const_iterator itr = tickets.begin(); itr != tickets.end(); ++itr)
    {
        GM_Ticket* tmpTicket = *itr;
        if (tmpTicket)
        {
            std::string gmname;

            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, tmpTicket->guid);
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(tmpTicket->name).c_str());
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGECREATE, (WorldSession::secondsToTimeString(m_session, time(NULL) - tmpTicket->createtime, true, false)).c_str());
            if (tmpTicket->updatetime)
                ss << PGetSysMessage(LANG_COMMAND_TICKETLISTAGE, (WorldSession::secondsToTimeString(m_session, time(NULL) - tmpTicket->updatetime, true, false)).c_str());

            if (sObjectMgr.GetPlayerNameByGUID(tmpTicket->assignedToGM, gmname))
                ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());

            ss <<  PGetSysMessage(LANG_COMMAND_TICKETLISTMESSAGE, tmpTicket->message.c_str());

            if (tmpTicket->comment != "")
                ss <<  PGetSysMessage(LANG_COMMAND_TICKETLISTCOMMENT, tmpTicket->comment.c_str());

            if (sObjectMgr.GetPlayerNameByGUID(tmpTicket->closedBy, gmname))
                ss << PGetSysMessage(LANG_COMMAND_TICKETCLOSED, gmname.c_str());

            ss << "\n";
        }
    }

    SendSysMessage(ss.str().c_str());
    return true;
}

bool ChatHandler::HandleGMTicketCloseByIdCommand(const char* args)
{
    if (!*args)
        return false;

    uint64 tguid = atoi(args);
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpen(tguid);
    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }
    if (ticket && ticket->assignedToGM != 0 && ticket->assignedToGM != m_session->GetPlayer()->GetGUID())
    {
        PSendSysMessage(LANG_COMMAND_TICKETCANNOTCLOSE, ticket->guid);
        return true;
    }
    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETCLOSED, m_session->GetPlayer()->GetName());
    SendGlobalGMSysMessage(ss.str().c_str());
    uint64 itemGUID = 0;
    Player *plr = sObjectMgr.GetPlayerInWorld(ticket->playerGuid);

    ItemPrototype const* itemProto = NULL;//ObjectMgr::GetItemPrototype(GM_LIKE_ITEM);
    if (!plr || !plr->IsInWorld())
    {
        uint32 accId = sObjectMgr.GetPlayerAccountIdByGUID(ticket->playerGuid);
        if (accId && itemProto)
        {
            QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT `locale_id` FROM `account` WHERE account_id='%u'", accId);
            if (result)
            {
                Field* fields = result->Fetch();
                uint8 loc_idx = sObjectMgr.GetIndexForLocale(LocaleConstant(fields[0].GetUInt8()));
                uint64 receiver_guid = ticket->playerGuid;

                if (Item* gmLikeItem = Item::CreateItem(GM_LIKE_ITEM,1,m_session ? m_session->GetPlayer() : 0))
                {
                    itemGUID = gmLikeItem->GetGUIDLow();
                    // save new item before send
                    gmLikeItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

                    // subject: item name
                    std::string subject = itemProto->Name1;
                    sObjectMgr.GetItemLocaleStrings(itemProto->ItemId, loc_idx, &subject);

                    // text
                    char textBuf[300];
                    sprintf(textBuf, "%s", sObjectMgr.GetHellgroundString(LANG_GM_LIKE, loc_idx));
                    uint32 itemTextId = sObjectMgr.CreateItemText(textBuf);

                    MailDraft(subject, itemTextId)
                        .AddItem(gmLikeItem)
                        .SendMailTo(MailReceiver(NULL, ObjectGuid(receiver_guid)), MailSender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM));
                }
            }
        }
    }
    else // plr exists
    {
        // send abandon ticket
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        plr->SendPacketToSelf(&data);
        if (itemProto)
        {
            ItemPosCountVec dest;
            uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, GM_LIKE_ITEM, 1);
            if (msg != EQUIP_ERR_OK)
            {
                if (Item* gmLikeItem = Item::CreateItem(GM_LIKE_ITEM,1,plr))
                {
                    itemGUID = gmLikeItem->GetGUIDLow();
                    // save new item before send
                    gmLikeItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

                    // subject: item name
                    std::string subject = itemProto->Name1;
                    sObjectMgr.GetItemLocaleStrings(itemProto->ItemId, plr->GetSession()->GetSessionDbLocaleIndex(), &subject);

                    // text
                    char textBuf[300];
                    sprintf(textBuf, "%s", plr->GetSession()->GetHellgroundString(LANG_GM_LIKE));
                    uint32 itemTextId = sObjectMgr.CreateItemText(textBuf);

                    MailDraft(subject, itemTextId)
                        .AddItem(gmLikeItem)
                        .SendMailTo(plr, MailSender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM));
                }
            }
            else if (Item *it = plr->StoreNewItem(dest, GM_LIKE_ITEM, true, 0, "GM_COMMAND"))
            {
                itemGUID = it->GetGUIDLow();
                plr->SendNewItem(it, 1, true, false);
            }
        }
    }
    sTicketMgr.RemoveGMTicket(ticket->guid, m_session->GetPlayer()->GetGUID(), itemGUID);
    return true;
}

bool ChatHandler::HandleGMTicketResponseCommand(const char* args)
{
    if (!*args)
        return false;

    // format: id "mail text"

    char* guid = strtok((char*)args, " ");

    if (!guid)
        return false;

    uint64 tguid = atoi(guid);
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpen(tguid);
    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    if (ticket && ticket->assignedToGM != 0 && ticket->assignedToGM != m_session->GetPlayer()->GetGUID())
    {
        PSendSysMessage(LANG_COMMAND_TICKETCANNOTCLOSE, ticket->guid);
        return true;
    }

    char* tail = strtok(NULL, "");
    if (!tail)
        return false;

    char* msgText;
    if (*tail == '"')
        msgText = strtok(tail+1, "\"");
    else
    {
        char* space = strtok(tail, "\"");
        if (!space)
            return false;
        msgText = strtok(NULL, "\"");
    }

    if (!msgText)
        return false;

    ticket->response = msgText;
    sTicketMgr.UpdateGMTicket(ticket);

    if (!SendGMMail(ticket->name.c_str(), "Ticket", msgText))
        return false;

    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETCLOSED, m_session->GetPlayer()->GetName());
    SendGlobalGMSysMessage(ss.str().c_str());
    uint64 itemGUID = 0;
    Player *plr = sObjectMgr.GetPlayerInWorld(ticket->playerGuid);

    ItemPrototype const* itemProto = NULL; //ObjectMgr::GetItemPrototype(GM_LIKE_ITEM);
    if (!plr || !plr->IsInWorld())
    {
        uint32 accId = sObjectMgr.GetPlayerAccountIdByGUID(ticket->playerGuid);
        if (accId && itemProto)
        {
            QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT `locale_id` FROM `account` WHERE account_id='%u'", accId);
            if (result)
            {
                Field* fields = result->Fetch();
                uint8 loc_idx = sObjectMgr.GetIndexForLocale(LocaleConstant(fields[0].GetUInt8()));
                uint64 receiver_guid = ticket->playerGuid;

                if (Item* gmLikeItem = Item::CreateItem(GM_LIKE_ITEM,1,m_session ? m_session->GetPlayer() : 0))
                {
                    itemGUID = gmLikeItem->GetGUIDLow();
                    // save new item before send
                    gmLikeItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

                    // subject: item name
                    std::string subject = itemProto->Name1;
                    sObjectMgr.GetItemLocaleStrings(itemProto->ItemId, loc_idx, &subject);

                    // text
                    char textBuf[300];
                    sprintf(textBuf, "%s", sObjectMgr.GetHellgroundString(LANG_GM_LIKE, loc_idx));
                    uint32 itemTextId = sObjectMgr.CreateItemText(textBuf);

                    MailDraft(subject, itemTextId)
                        .AddItem(gmLikeItem)
                        .SendMailTo(MailReceiver(NULL, ObjectGuid(receiver_guid)), MailSender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM));
                }
            }
        }
    }
    else // plr exists
    {
        // send abandon ticket
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        plr->SendPacketToSelf(&data);
        if (itemProto)
        {
            ItemPosCountVec dest;
            uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, GM_LIKE_ITEM, 1);
            if (msg != EQUIP_ERR_OK)
            {
                if (Item* gmLikeItem = Item::CreateItem(GM_LIKE_ITEM,1,plr))
                {
                    itemGUID = gmLikeItem->GetGUIDLow();
                    // save new item before send
                    gmLikeItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

                    // subject: item name
                    std::string subject = itemProto->Name1;
                    sObjectMgr.GetItemLocaleStrings(itemProto->ItemId, plr->GetSession()->GetSessionDbLocaleIndex(), &subject);

                    // text
                    char textBuf[300];
                    sprintf(textBuf, "%s", plr->GetSession()->GetHellgroundString(LANG_GM_LIKE));
                    uint32 itemTextId = sObjectMgr.CreateItemText(textBuf);

                    MailDraft(subject, itemTextId)
                        .AddItem(gmLikeItem)
                        .SendMailTo(plr, MailSender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM));
                }
            }
            else if (Item *it = plr->StoreNewItem(dest, GM_LIKE_ITEM, true, 0, "GM_COMMAND"))
            {
                itemGUID = it->GetGUIDLow();
                plr->SendNewItem(it, 1, true, false);
            }
        }
    }
    sTicketMgr.RemoveGMTicket(ticket->guid, m_session->GetPlayer()->GetGUID(), itemGUID);
    return true;
}

bool ChatHandler::HandleGMTicketAssignToCommand(const char* args)
{
    if (!*args)
        return false;

    char* tguid = strtok((char*)args, " ");
    uint64 ticketGuid = atoi(tguid);
    char* targetgm = strtok(NULL, " ");

    if (!targetgm)
        return false;

    std::string targm = targetgm;

    if (!normalizePlayerName(targm))
        return true;

    std::string gmname;
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpen(ticketGuid);

    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }
    uint64 tarGUID = sObjectMgr.GetPlayerGUIDByName(targm.c_str());
    uint64 accid = sObjectMgr.GetPlayerAccountIdByGUID(tarGUID);

    if (!tarGUID || !AccountMgr::HasPermissions(accid, PERM_GMT))
    {
        SendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_A);
        return true;
    }
    if (ticket->assignedToGM == tarGUID)
    {
        PSendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_B, ticket->guid);
        return true;
    }
    sObjectMgr.GetPlayerNameByGUID(tarGUID, gmname);

    ticket->assignedToGM = tarGUID;
    ticket->assigntime = time(NULL);
    sTicketMgr.CheckUnassignedExist();
    sTicketMgr.UpdateGMTicket(ticket);
    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
    SendGlobalGMSysMessage(ss.str().c_str());

    Player *plrGM = sObjectMgr.GetPlayerInWorld(tarGUID);
    if (plrGM)
        sTicketMgr.ModifyGmBusy(plrGM->GetSession(), true);

    if (sTicketMgr.IsGMTicketDenied(tarGUID, ticket))
    {
        sTicketMgr.RemoveGMTicketDeny(tarGUID, ticket);
        if (plrGM)
            ChatHandler(plrGM).PSendSysMessage("Ticket %llu removed from your denied list", ticket->guid);
    }

    Player *plr = sObjectMgr.GetPlayerInWorld(ticket->playerGuid);
    if (plr)
        ChatHandler(plr).PSendSysMessage(LANG_TICKET_ASSIGNED, gmname.c_str());

    return true;
}

bool ChatHandler::HandleGMTicketSelfAssignCommand(const char* args)
{
    if (!*args)
        return false;

    char* tguid = strtok((char*)args, " ");
    uint64 ticketGuid = atoi(tguid);

    Player *cplr = m_session->GetPlayer();
    std::string gmname;
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpen(ticketGuid);

    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }
    uint64 tarGUID = cplr->GetGUID();

    if (ticket->assignedToGM == tarGUID)
    {
        PSendSysMessage(LANG_COMMAND_TICKETASSIGNERROR_B, ticket->guid);
        return true;
    }
    sObjectMgr.GetPlayerNameByGUID(tarGUID, gmname);
    if (ticket->assignedToGM != 0)
    {
        PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->guid, gmname.c_str());
        return true;
    }

    if (sTicketMgr.IsGMTicketDenied(tarGUID, ticket))
    {
        sTicketMgr.RemoveGMTicketDeny(tarGUID, ticket);
        PSendSysMessage("Ticket %llu removed from your denied list", ticket->guid);
    }

    ticket->assignedToGM = tarGUID;
    ticket->assigntime = time(NULL);
    sTicketMgr.CheckUnassignedExist();
    sTicketMgr.UpdateGMTicket(ticket);
    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
    SendGlobalGMSysMessage(ss.str().c_str());

    sTicketMgr.ModifyGmBusy(m_session, true);

    Player *plr = sObjectMgr.GetPlayerInWorld(ticket->playerGuid);
    if (plr)
        ChatHandler(plr).PSendSysMessage(LANG_TICKET_ASSIGNED, gmname.c_str());
    return true;
}

bool ChatHandler::HandleGMTicketDenyCommand(const char* args)
{
    if (!*args)
        return false;

    uint64 ticketGuid = atoi(args);
    Player *cplr = m_session->GetPlayer();
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpen(ticketGuid);

    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    if (sTicketMgr.IsGMTicketDenied(cplr->GetGUID(), ticket))
    {
        PSendSysMessage("Ticket %llu already in your denied list", ticket->guid);
        return true;
    }

    if (ticket->assignedToGM != 0)
    {
        if (ticket->assignedToGM != cplr->GetGUID())
        {
            SendSysMessage("Ticket is assigned to another GM");
            return true;
        }
        else // do unassign self before denying
        {
            // Copy of the code from HandleGMTicketUnAssignCommand()
            std::stringstream ss;

            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, cplr->GetName());
            ss << PGetSysMessage(LANG_COMMAND_TICKETLISTUNASSIGNED, cplr->GetName());
            SendGlobalGMSysMessage(ss.str().c_str());
            ticket->assignedToGM = 0;
            ticket->assigntime = time(NULL);
            sTicketMgr.CheckUnassignedExist();
            sTicketMgr.UpdateGMTicket(ticket);

            if (cplr)
                sTicketMgr.ModifyGmBusy(cplr->GetSession(), false);
        }
    }

    sTicketMgr.AddGMTicketDeny(cplr->GetGUID(), ticket);
    PSendSysMessage("Ticket %llu added to your denied list", ticket->guid);

    return true;
}

bool ChatHandler::HandleGMTicketUnAssignCommand(const char* args)
{
    if (!*args)
        return false;

    uint64 ticketGuid = atoi(args);
    Player *cplr = m_session->GetPlayer();
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpen(ticketGuid);

    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }
    if (ticket->assignedToGM == 0)
    {
        PSendSysMessage(LANG_COMMAND_TICKETNOTASSIGNED, ticket->guid);
        return true;
    }

    std::string gmname;
    sObjectMgr.GetPlayerNameByGUID(ticket->assignedToGM, gmname);
    Player *plr = sObjectMgr.GetPlayerInWorld(ticket->assignedToGM);
    if (plr && plr->IsInWorld() && plr->GetSession()->GetPermissions() > cplr->GetSession()->GetPermissions())
    {
        SendSysMessage(LANG_COMMAND_TICKETUNASSIGNSECURITY);
        return true;
    }

    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTUNASSIGNED, cplr->GetName());
    SendGlobalGMSysMessage(ss.str().c_str());
    ticket->assignedToGM = 0;
    ticket->assigntime = time(NULL);
    sTicketMgr.CheckUnassignedExist();
    sTicketMgr.UpdateGMTicket(ticket);

    if (plr)
        sTicketMgr.ModifyGmBusy(plr->GetSession(), false);
    return true;
}

bool ChatHandler::HandleGMTicketCommentCommand(const char* args)
{
    if (!*args)
        return false;

    char* tguid = strtok((char*)args, " ");
    uint64 ticketGuid = atoi(tguid);
    char* comment = strtok(NULL, "\n");

    if (!comment)
        return false;

    Player *cplr = m_session->GetPlayer();
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpen(ticketGuid);

    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }
    if (ticket->assignedToGM != 0 && ticket->assignedToGM != cplr->GetGUID())
    {
        PSendSysMessage(LANG_COMMAND_TICKETALREADYASSIGNED, ticket->guid);
        return true;
    }

    std::string gmname;
    sObjectMgr.GetPlayerNameByGUID(ticket->assignedToGM, gmname);
    ticket->comment = comment;
    sTicketMgr.UpdateGMTicket(ticket);
    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    if (sObjectMgr.GetPlayerNameByGUID(ticket->assignedToGM, gmname))
    {
        ss << PGetSysMessage(LANG_COMMAND_TICKETLISTASSIGNEDTO, gmname.c_str());
    }
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTADDCOMMENT, cplr->GetName(), ticket->comment.c_str());
    SendGlobalGMSysMessage(ss.str().c_str());
    return true;
}

bool ChatHandler::HandleGMTicketDeleteByIdCommand(const char* args)
{
    if (!*args)
        return false;
    uint64 ticketGuid = atoi(args);
    GM_Ticket *ticket = sTicketMgr.GetGMTicketAny(ticketGuid);

    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }
    if (ticket->closedBy == 0)
    {
        SendSysMessage(LANG_COMMAND_TICKETCLOSEFIRST);
        return true;
    }

    std::stringstream ss;

    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTGUID, ticket->guid);
    ss << PGetSysMessage(LANG_COMMAND_TICKETLISTNAME, GetNameLink(ticket->name).c_str());
    ss << PGetSysMessage(LANG_COMMAND_TICKETDELETED, m_session->GetPlayer()->GetName());
    SendGlobalGMSysMessage(ss.str().c_str());
    Player *plr = sObjectMgr.GetPlayerInWorld(ticket->playerGuid);
    sTicketMgr.DeleteGMTicketPermanently(ticket->guid);
    if (plr && plr->IsInWorld())
    {
        // Force abandon ticket
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        plr->SendPacketToSelf(&data);
    }

    ticket = NULL;
    return true;
}

bool ChatHandler::HandleReloadGMTicketCommand(const char*)
{
    sTicketMgr.LoadGMTickets();
    return true;
}

bool ChatHandler::HandleReloadFakeBots(const char*)
{
	sWorld.LoadFakeBotInfo();
	return true;
}

//Enable\Dissable Invisible mode
bool ChatHandler::HandleGMVisibleCommand(const char* args)
{
    if (!*args)
    {
        PSendSysMessage(LANG_YOU_ARE, m_session->GetPlayer()->isGMVisible() ?  GetHellgroundString(LANG_VISIBLE) : GetHellgroundString(LANG_INVISIBLE));
        return true;
    }

    const uint32 VISUAL_AURA = 37800;
    std::string argstr = (char*)args;
    Player* player = m_session->GetPlayer();

    if (argstr == "on")
    {
        if (player->HasAura(VISUAL_AURA, 0))
            player->RemoveAurasDueToSpell(VISUAL_AURA);

        player->SetGMVisible(true);
        m_session->SendNotification(LANG_INVISIBLE_VISIBLE);
        return true;
    }

    if (argstr == "off")
    {
        m_session->SendNotification(LANG_INVISIBLE_INVISIBLE);
        m_session->GetPlayer()->SetGMVisible(false);

        player->AddAura(VISUAL_AURA, player);

        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleGPSCommand(const char* args)
{
    Unit *obj = NULL;
    if (*args)
    {
        std::string name = args;
        if (normalizePlayerName(name))
            obj = sObjectMgr.GetPlayerInWorld(name.c_str());

        if (!obj)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        obj = getSelectedUnit();

        if (!obj)
        {
            SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }
    }
    CellPair cell_val = Hellground::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell(cell_val);

    uint32 zone_id, area_id;
    if (obj->GetTypeId() == TYPEID_PLAYER)
    {
        zone_id = ((Player*)obj)->GetCachedZone();
        area_id = ((Player*)obj)->GetCachedArea();
    }
    else
    {
        zone_id = obj->GetZoneId();
        area_id = obj->GetAreaId();
    }

    MapEntry const* mapEntry = sMapStore.LookupEntry(obj->GetMapId());
    AreaTableEntry const* zoneEntry = GetAreaEntryByAreaID(zone_id);
    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(area_id);

    float zone_x = obj->GetPositionX();
    float zone_y = obj->GetPositionY();

    Map2ZoneCoordinates(zone_x,zone_y,zone_id);

    TerrainInfo const *map = obj->GetTerrain();

    float ground_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), MAX_HEIGHT);
    float floor_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());
    float grid_z_cur = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), false);
    float grid_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), MAX_HEIGHT, false);

    GridPair p = Hellground::ComputeGridPair(obj->GetPositionX(), obj->GetPositionY());

    int gx=63-p.x_coord;
    int gy=63-p.y_coord;

    uint32 have_map = GridMap::ExistMap(obj->GetMapId(),gx,gy) ? 1 : 0;
    uint32 have_vmap = GridMap::ExistVMap(obj->GetMapId(),gx,gy) ? 1 : 0;

    if (have_vmap)
    {
        if (map->IsOutdoors(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ()))
            SendSysMessage("Object is outdoors");
        else
            SendSysMessage("Object is indoor");

        if (obj->GetMap()->GetTerrain()->IsInWater(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ()))
            SendSysMessage("Object is in water");
        else
            SendSysMessage("Object is not in water");
    }
    else
        SendSysMessage("no VMAP available for area info");

    PSendSysMessage(LANG_MAP_POSITION,
        obj->GetMapId(), (mapEntry ? mapEntry->name[m_session->GetSessionDbcLocale()] : "<unknown>"),
        zone_id, (zoneEntry ? zoneEntry->area_name[m_session->GetSessionDbcLocale()] : "<unknown>"),
        area_id, (areaEntry ? areaEntry->area_name[m_session->GetSessionDbcLocale()] : "<unknown>"),
        obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
        cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetAnyInstanceId(),
        zone_x, zone_y, ground_z, floor_z, have_map, have_vmap, have_vmap);
    PSendSysMessage("GridGroundCurr height: %f. Gridground max: %f", grid_z_cur, grid_z);

    sLog.outDebug("Player %s GPS call for %s '%s' (%s: %u):",
        GetName(),
        (obj->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), obj->GetName(),
        (obj->GetTypeId() == TYPEID_PLAYER ? "GUID" : "Entry"), (obj->GetTypeId() == TYPEID_PLAYER ? obj->GetGUIDLow(): obj->GetEntry()));
    sLog.outDebug(GetHellgroundString(LANG_MAP_POSITION),
        obj->GetMapId(), (mapEntry ? mapEntry->name[sWorld.GetDefaultDbcLocale()] : "<unknown>"),
        zone_id, (zoneEntry ? zoneEntry->area_name[sWorld.GetDefaultDbcLocale()] : "<unknown>"),
        area_id, (areaEntry ? areaEntry->area_name[sWorld.GetDefaultDbcLocale()] : "<unknown>"),
        obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
        cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetAnyInstanceId(),
        zone_x, zone_y, ground_z, floor_z, have_map, have_vmap, have_vmap);

    if (Player* pl = m_session->GetPlayer())
    {
        if (pl->GetDummyAura(54839))
        {
            char chr[255];
            sprintf(chr, "position_x='%f', position_y='%f', position_z='%f'", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());
            pl->Say(chr, LANG_UNIVERSAL);
        }
    }

    PSendSysMessage("X: %f, Y: %f, Z: %f", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());
    sLog.outString("%ff, %ff, %ff (.go xyz %f %f %f %u)", obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetMapId());

    return true;
}

bool ChatHandler::HandlePosInfoCommand(const char* args)
{
    Player* _player = m_session->GetPlayer();

    MapEntry const* mapEntry = sMapStore.LookupEntry(_player->GetMapId());
    PSendSysMessage("MapId: %u, Name: /", _player->GetMapId() /*, mapEntry->name*/);
    SendSysMessage("- cached data -");

    const AreaTableEntry* zEntry = GetAreaEntryByAreaID(_player->GetCachedZone());
    const AreaTableEntry* aEntry = GetAreaEntryByAreaID(_player->GetCachedArea());
    if (!aEntry || !zEntry)
        return false;

    PSendSysMessage("*zone: / [%u]",/* zEntry->area_name,*/ _player->GetCachedZone());
    PSendSysMessage("*area: / [%u]", /*aEntry->area_name,*/ _player->GetCachedArea());

    const AreaTableEntry* zEntry2 = GetAreaEntryByAreaID(_player->GetZoneId());
    const AreaTableEntry* aEntry2 = GetAreaEntryByAreaID(_player->GetAreaId());
    if (!aEntry2 || !zEntry2)
        return false;

    SendSysMessage("- real data -");
    PSendSysMessage("*zone: / [%u]", /*zEntry2->area_name,*/ _player->GetZoneId());
    PSendSysMessage("*area: / [%u]", /*aEntry2->area_name,*/ _player->GetAreaId());

    TerrainInfo const *terrain = _player->GetTerrain();
    SendSysMessage("- terrain data -");

    PSendSysMessage("*ground Z: %f", terrain->GetHeight(_player->GetPositionX(), _player->GetPositionY(), MAX_HEIGHT));
    PSendSysMessage("*floor Z: %f", terrain->GetHeight(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ()));
    PSendSysMessage("*los: %s", terrain->IsLineOfSightEnabled() ? "enabled" : "disabled");
    PSendSysMessage("*mmaps: %s", terrain->IsPathFindingEnabled() ? "enabled" : "disabled");
    PSendSysMessage("*outdoors: %s", terrain->IsOutdoors(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ()) ? "yes" : "no");
    PSendSysMessage("*visibility: %f", terrain->GetSpecifics()->visibility);
    PSendSysMessage("*ainotify: %u", terrain->GetSpecifics()->ainotifyperiod);
    PSendSysMessage("*viewupdateafter: %f", sqrt(float(terrain->GetSpecifics()->viewupdatedistance)));
    return true;
}

//Summon Player
bool ChatHandler::HandleNamegoCommand(const char* args)
{
    if (!*args)
        return false;

    std::string name = args;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (Player *target = sObjectMgr.GetPlayerInWorld(name.c_str()))
    {
        if (target->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, target->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        Map* pMap = m_session->GetPlayer()->GetMap();

        if (pMap->IsBattleGroundOrArena())
        {
            // only allow if gm mode is on
            if (!target->isGameMaster())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM,target->GetName());
                SetSentErrorMessage(true);
                return false;
            }
            else if (target->GetBattleGroundId())
            {
                // only allow to teleport within the same BG, when in BG
                if (m_session->GetPlayer()->GetBattleGroundId() != target->GetBattleGroundId())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_BG_FROM_BG, target->GetName());
                    SetSentErrorMessage(true);
                    return false;
                }
            }
            else // only save entry point and set bg if target is NOT in a BG at the moment
            {
                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                target->SetBattleGroundId(m_session->GetPlayer()->GetBattleGroundId(), m_session->GetPlayer()->GetBattleGroundTypeId());
                target->SaveOwnBattleGroundEntryPoint();
            }
        }
        else if (pMap->IsDungeon())
        {
            Map* cMap = target->GetMap();
            if (cMap->Instanceable() && cMap->GetInstanciableInstanceId() != pMap->GetInstanciableInstanceId())
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,target->GetName());
                SetSentErrorMessage(true);
                return false;
            }

            // we are in instance, and can summon only player in our group with us as lead
            if (!m_session->GetPlayer()->GetGroup() || !target->GetGroup() ||
                (target->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ||
                (m_session->GetPlayer()->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()))
                // the last check is a bit excessive, but let it be, just in case
            {
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,target->GetName());
                SetSentErrorMessage(true);
                return false;
            }
        }

        PSendSysMessage(LANG_SUMMONING, target->GetName(),"");
        if (needReportToTarget(target))
            ChatHandler(target).PSendSysMessage(LANG_SUMMONED_BY, GetName());

        // stop flight if need
        target->InterruptTaxiFlying();

        // before GM
        float x,y,z;
        m_session->GetPlayer()->GetNearPoint(x,y,z,target->GetObjectSize());
        target->TeleportTo(m_session->GetPlayer()->GetMapId(),x,y,z,target->GetOrientation());
    }
    else if (uint64 guid = sObjectMgr.GetPlayerGUIDByName(name))
    {
        PSendSysMessage(LANG_SUMMONING, name.c_str(),GetHellgroundString(LANG_OFFLINE));

        // in point where GM stay
        Player::SavePositionInDB(m_session->GetPlayer()->GetMapId(),
            m_session->GetPlayer()->GetPositionX(),
            m_session->GetPlayer()->GetPositionY(),
            m_session->GetPlayer()->GetPositionZ(),
            m_session->GetPlayer()->GetOrientation(),
            m_session->GetPlayer()->GetCachedZone(),
            guid);
    }
    else
    {
        PSendSysMessage(LANG_NO_PLAYER, args);
        SetSentErrorMessage(true);
    }

    return true;
}

//Teleport to Player
bool ChatHandler::HandleGonameCommand(const char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    std::string name = args;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

	Player *target = sObjectMgr.GetPlayerInWorld(name.c_str());

	if (m_session->GetPermissions() < PERM_ADM && target && target->GetSession() && isBotAccount(target->GetSession()->GetAccountId()))
	{
		SendSysMessage("Error");
		SetSentErrorMessage(true);
		return false;
	}

    if (target)
    {
        Map* cMap = target->GetMap();
        if (cMap->IsBattleGroundOrArena())
        {
            // only allow if gm mode is on
            if (!_player->isGameMaster())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM,target->GetName());
                SetSentErrorMessage(true);
                return false;
            }
            // if already in a bg, don't let port to other
            else if (_player->GetBattleGroundId())
            {
                // only allow to teleport within the same BG, when in BG
                if (_player->GetBattleGroundId() != target->GetBattleGroundId())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_BG_FROM_BG, target->GetName());
                    SetSentErrorMessage(true);
                    return false;
                }
            }
            else // only save entry point and set bg if we are NOT in a BG at the moment
            {
                // all's well, set bg id
                // when porting out from the bg, it will be reset to 0
                _player->SetBattleGroundId(target->GetBattleGroundId(), target->GetBattleGroundTypeId());
                _player->SaveOwnBattleGroundEntryPoint();
            }
        }
        else if (cMap->IsDungeon())
        {
            Map* pMap = _player->GetMap();

            // we have to go to instance, and can go to player only if:
            //   1) we are in his group (either as leader or as member)
            //   2) we are not bound to any group and have GM mode on
            if (_player->GetGroup())
            {
                // we are in group, we can go only if we are in the player group
                if (_player->GetGroup() != target->GetGroup())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY,target->GetName());
                    SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                // we are not in group, let's verify our GM mode
                if (!_player->isGameMaster())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM,target->GetName());
                    SetSentErrorMessage(true);
                    return false;
                }
            }

            // if the player or the player's group is bound to another instance
            // the player will not be bound to another one
            InstancePlayerBind *pBind = _player->GetBoundInstance(target->GetMapId(), target->GetDifficulty());
            if (!pBind)
            {
                Group *group = _player->GetGroup();
                InstanceGroupBind *gBind = group ? group->GetBoundInstance(target->GetMapId(), target->GetDifficulty()) : NULL;
                if (!gBind)
                {
                    // if no bind exists, create a solo bind
                    if (InstanceSave *save = sInstanceSaveManager.GetInstanceSave(target->GetInstanciableInstanceId()))
                        _player->BindToInstance(save, !save->CanReset());
                }
            }

            _player->SetDifficulty(target->GetDifficulty());
        }

        PSendSysMessage(LANG_APPEARING_AT, target->GetName());

        if (_player->IsVisibleGloballyfor (target))
            ChatHandler(target).PSendSysMessage(LANG_APPEARING_TO, _player->GetName());

        // stop flight if need
        _player->InterruptTaxiFlying();

        // to point to see at target with same orientation
        float x,y,z;
        target->GetPosition(x, y, z);
        _player->TeleportTo(target->GetMapId(), x, y, z, _player->GetOrientationTo(target), TELE_TO_GM_MODE);

        return true;
    }

    if (uint64 guid = sObjectMgr.GetPlayerGUIDByName(name))
    {
        PSendSysMessage(LANG_APPEARING_AT, name.c_str());

        // to point where player stay (if loaded)
        float x,y,z,o;
        uint32 map;
        bool in_flight;
        if (Player::LoadPositionFromDB(map,x,y,z,o,in_flight,guid))
        {
            // stop flight if need
            _player->InterruptTaxiFlying();

            _player->TeleportTo(map, x, y, z,_player->GetOrientation());
            return true;
        }
    }

    PSendSysMessage(LANG_NO_PLAYER, args);

    SetSentErrorMessage(true);
    return false;
}

// Teleport player to last position
bool ChatHandler::HandleRecallCommand(const char* args)
{
    Player* chr = NULL;

    if (!*args)
    {
        chr = getSelectedPlayer();
        if (!chr)
            chr = m_session->GetPlayer();
    }
    else
    {
        std::string name = args;

        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        chr = sObjectMgr.GetPlayerInWorld(name.c_str());

        if (!chr)
        {
            PSendSysMessage(LANG_NO_PLAYER, args);
            SetSentErrorMessage(true);
            return false;
        }
    }

    if (chr->IsBeingTeleported())
    {
        PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if (chr->IsTaxiFlying())
    {
        chr->GetUnitStateMgr().DropAction(UNIT_ACTION_TAXI);
        chr->m_taxi.ClearTaxiDestinations();
        chr->GetUnitStateMgr().InitDefaults(false);
    }

    if (!chr->TeleportTo(chr->_recallPosition))
        SendSysMessage("Error on recall");
    else
        SendSysMessage("Recalled successfully");

    return true;
}

//Edit Player KnownTitles
bool ChatHandler::HandleModifyKnownTitlesCommand(const char* args)
{
    if (!*args)
        return false;

    uint64 titles = 0;

    sscanf((char*)args, "%llu", &titles);

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 existingTitles;

    for (int i = 1; i < sCharTitlesStore.GetNumRows(); ++i)
        if (CharTitlesEntry const* tEntry = sCharTitlesStore.LookupEntry(i))
            existingTitles |= (uint64(1) << tEntry->bit_index);

    titles &= existingTitles;

    chr->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles);
    SendSysMessage(LANG_DONE);

    return true;
}

//Edit Player HP
bool ChatHandler::HandleModifyHPCommand(const char* args)
{
    if (!*args)
        return false;

    //    char* pHp = strtok((char*)args, " ");
    //    if (!pHp)
    //        return false;

    //    char* pHpMax = strtok(NULL, " ");
    //    if (!pHpMax)
    //        return false;

    //    int32 hpm = atoi(pHpMax);
    //    int32 hp = atoi(pHp);

    int32 hp = atoi((char*)args);
    int32 hpm = atoi((char*)args);

    if (hp <= 0 || hpm <= 0 || hpm < hp)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_HP, chr->GetName(), hp, hpm);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_HP_CHANGED, GetName(), hp, hpm);

    chr->SetMaxHealth(hpm);
    chr->SetHealth(hp);

    return true;
}

//Edit Player Mana
bool ChatHandler::HandleModifyManaCommand(const char* args)
{
    if (!*args)
        return false;

    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);
    int32 mana = atoi((char*)args);
    int32 manam = atoi((char*)args);

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_MANA, chr->GetName(), mana, manam);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_MANA_CHANGED, GetName(), mana, manam);

    chr->SetMaxPower(POWER_MANA,manam);
    chr->SetPower(POWER_MANA, mana);

    return true;
}

//Edit Player Energy
bool ChatHandler::HandleModifyEnergyCommand(const char* args)
{
    if (!*args)
        return false;

    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);

    int32 energy = atoi((char*)args)*10;
    int32 energym = atoi((char*)args)*10;

    if (energy <= 0 || energym <= 0 || energym < energy)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_ENERGY, chr->GetName(), energy/10, energym/10);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_ENERGY_CHANGED, GetName(), energy/10, energym/10);

    chr->SetMaxPower(POWER_ENERGY,energym);
    chr->SetPower(POWER_ENERGY, energy);

    sLog.outDetail(GetHellgroundString(LANG_CURRENT_ENERGY),chr->GetMaxPower(POWER_ENERGY));

    return true;
}

//Edit Player Rage
bool ChatHandler::HandleModifyRageCommand(const char* args)
{
    if (!*args)
        return false;

    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);

    int32 rage = atoi((char*)args)*10;
    int32 ragem = atoi((char*)args)*10;

    if (rage <= 0 || ragem <= 0 || ragem < rage)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_RAGE, chr->GetName(), rage/10, ragem/10);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_RAGE_CHANGED, GetName(), rage/10, ragem/10);

    chr->SetMaxPower(POWER_RAGE,ragem);
    chr->SetPower(POWER_RAGE, rage);

    return true;
}

//Edit Player Faction
bool ChatHandler::HandleModifyFactionCommand(const char* args)
{
    if (!*args)
        return false;

    char* pfactionid = extractKeyFromLink((char*)args,"Hfaction");

    Creature* chr = getSelectedCreature();
    if (!chr)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!pfactionid)
    {
        if (chr)
        {
            uint32 factionid = chr->getFaction();
            uint32 flag      = chr->GetUInt32Value(UNIT_FIELD_FLAGS);
            uint32 npcflag   = chr->GetUInt32Value(UNIT_NPC_FLAGS);
            uint32 dyflag    = chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
            PSendSysMessage(LANG_CURRENT_FACTION,chr->GetGUIDLow(),factionid,flag,npcflag,dyflag);
        }
        return true;
    }

    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 factionid = atoi(pfactionid);
    uint32 flag;

    char *pflag = strtok(NULL, " ");
    if (!pflag)
        flag = chr->GetUInt32Value(UNIT_FIELD_FLAGS);
    else
        flag = atoi(pflag);

    char* pnpcflag = strtok(NULL, " ");

    uint32 npcflag;
    if (!pnpcflag)
        npcflag   = chr->GetUInt32Value(UNIT_NPC_FLAGS);
    else
        npcflag = atoi(pnpcflag);

    char* pdyflag = strtok(NULL, " ");

    uint32  dyflag;
    if (!pdyflag)
        dyflag   = chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
    else
        dyflag = atoi(pdyflag);

    if (!sFactionTemplateStore.LookupEntry(factionid))
    {
        PSendSysMessage(LANG_WRONG_FACTION, factionid);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_FACTION, chr->GetGUIDLow(),factionid,flag,npcflag,dyflag);

    chr->setFaction(factionid);
    chr->SetUInt32Value(UNIT_FIELD_FLAGS,flag);
    chr->SetUInt32Value(UNIT_NPC_FLAGS,npcflag);
    chr->SetUInt32Value(UNIT_DYNAMIC_FLAGS,dyflag);

    return true;
}

//Edit Player Spell
bool ChatHandler::HandleModifySpellCommand(const char* args)
{
    if (!*args) return false;
    char* pspellflatid = strtok((char*)args, " ");
    if (!pspellflatid)
        return false;

    char* pop = strtok(NULL, " ");
    if (!pop)
        return false;

    char* pval = strtok(NULL, " ");
    if (!pval)
        return false;

    uint16 mark;

    char* pmark = strtok(NULL, " ");

    uint8 spellflatid = atoi(pspellflatid);
    uint8 op   = atoi(pop);
    uint16 val = atoi(pval);
    if (!pmark)
        mark = 65535;
    else
        mark = atoi(pmark);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SPELLFLATID, spellflatid, val, mark, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SPELLFLATID_CHANGED, GetName(), spellflatid, val, mark);

    WorldPacket data(SMSG_SET_FLAT_SPELL_MODIFIER, (1+1+2+2));
    data << uint8(spellflatid);
    data << uint8(op);
    data << uint16(val);
    data << uint16(mark);
    chr->SendPacketToSelf(&data);

    return true;
}

//Edit Player TP
bool ChatHandler::HandleModifyTalentCommand (const char* args)
{
    if (!*args)
        return false;

    int tp = atoi((char*)args);
    if (tp>0)
    {
        Player* player = getSelectedPlayer();
        if (!player)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            SetSentErrorMessage(true);
            return false;
        }
        player->SetFreeTalentPoints(tp);
        return true;
    }
    return false;
}

//Enable On\OFF all taxi paths
bool ChatHandler::HandleTaxiCheatCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    std::string argstr = (char*)args;

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        chr=m_session->GetPlayer();
    }

    if (argstr == "on")
    {
        if (!chr->IsPlayerCustomFlagged(PL_CUSTOM_TAXICHEAT))
        {
            chr->AddPlayerCustomFlag(PL_CUSTOM_TAXICHEAT);
            PSendSysMessage(LANG_YOU_GIVE_TAXIS, chr->GetName());
            if (needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_ADDED, GetName());
        }
        else
            PSendSysMessage("%s already has taxicheat on", chr->GetName());
        return true;
    }

    if (argstr == "off")
    {
        if (chr->IsPlayerCustomFlagged(PL_CUSTOM_TAXICHEAT))
        {
            chr->RemovePlayerCustomFlag(PL_CUSTOM_TAXICHEAT);
            PSendSysMessage(LANG_YOU_REMOVE_TAXIS, chr->GetName());
            if (needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_REMOVED, GetName());
        }
        else
            PSendSysMessage("%s already has taxicheat off", chr->GetName());
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

//Edit Player Aspeed
bool ChatHandler::HandleModifyASpeedCommand(const char* args)
{
    if (!*args)
        return false;

    float ASpeed = (float)atof((char*)args);

    if (ASpeed > 20 || ASpeed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (m_session->GetPermissions() < PERM_ADM)
        chr = m_session->GetPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_ASPEED, ASpeed, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_ASPEED_CHANGED, GetName(), ASpeed);

    chr->SetSpeed(MOVE_WALK,    ASpeed,true);
    chr->SetSpeed(MOVE_RUN,     ASpeed,true);
    chr->SetSpeed(MOVE_SWIM,    ASpeed,true);
    //chr->SetSpeed(MOVE_TURN,    ASpeed,true);
    chr->SetSpeed(MOVE_FLIGHT,     ASpeed,true);
    return true;
}

//Edit Player Speed
bool ChatHandler::HandleModifySpeedCommand(const char* args)
{
    if (!*args)
        return false;

    float Speed = (float)atof((char*)args);

    if (Speed > 20 || Speed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (m_session->GetPermissions() < PERM_ADM)
        chr = m_session->GetPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SPEED, Speed, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SPEED_CHANGED, GetName(), Speed);

    chr->SetSpeed(MOVE_RUN,Speed,true);

    return true;
}

//Edit Player Swim Speed
bool ChatHandler::HandleModifySwimCommand(const char* args)
{
    if (!*args)
        return false;

    float Swim = (float)atof((char*)args);

    if (Swim > 20.0f || Swim < 0.01f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (m_session->GetPermissions() < PERM_ADM)
        chr = m_session->GetPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SWIM_SPEED, Swim, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SWIM_SPEED_CHANGED, GetName(), Swim);

    chr->SetSpeed(MOVE_SWIM,Swim,true);

    return true;
}

//Edit Player Walk Speed
bool ChatHandler::HandleModifyBWalkCommand(const char* args)
{
    if (!*args)
        return false;

    float BSpeed = (float)atof((char*)args);

    if (BSpeed > 20.0f || BSpeed < 0.1f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (m_session->GetPermissions() < PERM_ADM)
        chr = m_session->GetPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_BACK_SPEED, BSpeed, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_BACK_SPEED_CHANGED, GetName(), BSpeed);

    chr->SetSpeed(MOVE_RUN_BACK,BSpeed,true);

    return true;
}

//Edit Player Fly
bool ChatHandler::HandleModifyFlyCommand(const char* args)
{
    if (!*args)
        return false;

    float FSpeed = (float)atof((char*)args);

    if (FSpeed > 20.0f || FSpeed < 0.1f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (m_session->GetPermissions() < PERM_ADM)
        chr = m_session->GetPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_FLY_SPEED, FSpeed, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_FLY_SPEED_CHANGED, GetName(), FSpeed);

    chr->SetSpeed(MOVE_FLIGHT,FSpeed,true);

    return true;
}

//Edit Player Scale
bool ChatHandler::HandleModifyScaleCommand(const char* args)
{
    if (!*args)
        return false;

    float Scale = (float)atof((char*)args);
    if (Scale > 10.0f || Scale <= 0.0f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SIZE, Scale, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SIZE_CHANGED, GetName(), Scale);

    chr->SetFloatValue(OBJECT_FIELD_SCALE_X, Scale);

    return true;
}

//Enable Player mount
bool ChatHandler::HandleModifyMountCommand(const char* args)
{
    if (!*args)
        return false;

    uint16 mId = 1147;
    float speed = (float)15;
    uint32 num = 0;

    num = atoi((char*)args);
    switch (num)
    {
        case 1:
            mId=14340;
            break;
        case 2:
            mId=4806;
            break;
        case 3:
            mId=6471;
            break;
        case 4:
            mId=12345;
            break;
        case 5:
            mId=6472;
            break;
        case 6:
            mId=6473;
            break;
        case 7:
            mId=10670;
            break;
        case 8:
            mId=10719;
            break;
        case 9:
            mId=10671;
            break;
        case 10:
            mId=10672;
            break;
        case 11:
            mId=10720;
            break;
        case 12:
            mId=14349;
            break;
        case 13:
            mId=11641;
            break;
        case 14:
            mId=12244;
            break;
        case 15:
            mId=12242;
            break;
        case 16:
            mId=14578;
            break;
        case 17:
            mId=14579;
            break;
        case 18:
            mId=14349;
            break;
        case 19:
            mId=12245;
            break;
        case 20:
            mId=14335;
            break;
        case 21:
            mId=207;
            break;
        case 22:
            mId=2328;
            break;
        case 23:
            mId=2327;
            break;
        case 24:
            mId=2326;
            break;
        case 25:
            mId=14573;
            break;
        case 26:
            mId=14574;
            break;
        case 27:
            mId=14575;
            break;
        case 28:
            mId=604;
            break;
        case 29:
            mId=1166;
            break;
        case 30:
            mId=2402;
            break;
        case 31:
            mId=2410;
            break;
        case 32:
            mId=2409;
            break;
        case 33:
            mId=2408;
            break;
        case 34:
            mId=2405;
            break;
        case 35:
            mId=14337;
            break;
        case 36:
            mId=6569;
            break;
        case 37:
            mId=10661;
            break;
        case 38:
            mId=10666;
            break;
        case 39:
            mId=9473;
            break;
        case 40:
            mId=9476;
            break;
        case 41:
            mId=9474;
            break;
        case 42:
            mId=14374;
            break;
        case 43:
            mId=14376;
            break;
        case 44:
            mId=14377;
            break;
        case 45:
            mId=2404;
            break;
        case 46:
            mId=2784;
            break;
        case 47:
            mId=2787;
            break;
        case 48:
            mId=2785;
            break;
        case 49:
            mId=2736;
            break;
        case 50:
            mId=2786;
            break;
        case 51:
            mId=14347;
            break;
        case 52:
            mId=14346;
            break;
        case 53:
            mId=14576;
            break;
        case 54:
            mId=9695;
            break;
        case 55:
            mId=9991;
            break;
        case 56:
            mId=6448;
            break;
        case 57:
            mId=6444;
            break;
        case 58:
            mId=6080;
            break;
        case 59:
            mId=6447;
            break;
        case 60:
            mId=4805;
            break;
        case 61:
            mId=9714;
            break;
        case 62:
            mId=6448;
            break;
        case 63:
            mId=6442;
            break;
        case 64:
            mId=14632;
            break;
        case 65:
            mId=14332;
            break;
        case 66:
            mId=14331;
            break;
        case 67:
            mId=8469;
            break;
        case 68:
            mId=2830;
            break;
        case 69:
            mId=2346;
            break;
        default:
            SendSysMessage(LANG_NO_MOUNT);
            SetSentErrorMessage(true);
            return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_GIVE_MOUNT, chr->GetName());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_MOUNT_GIVED, GetName());

    chr->SetUInt32Value(UNIT_FIELD_FLAGS , 0x001000);
    chr->Mount(mId);

    WorldPacket data(SMSG_FORCE_RUN_SPEED_CHANGE, (8+4+1+4));
    data << chr->GetPackGUID();
    data << (uint32)0;
    data << (uint8)0;                                       //new 2.1.0
    data << float(speed);
    chr->BroadcastPacket(&data, true);

    data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE, (8+4+4));
    data << chr->GetPackGUID();
    data << (uint32)0;
    data << float(speed);
    chr->BroadcastPacket(&data, true);

    return true;
}

//Edit Player money
bool ChatHandler::HandleModifyMoneyCommand(const char* args)
{
    if (!*args)
        return false;

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int32 addmoney = atoi((char*)args);

    uint32 moneyuser = chr->GetMoney();

    if (addmoney < 0)
    {
        int32 newmoney = moneyuser + addmoney;

        sLog.outDetail(GetHellgroundString(LANG_CURRENT_MONEY), moneyuser, addmoney, newmoney);
        if (newmoney <= 0)
        {
            PSendSysMessage(LANG_YOU_TAKE_ALL_MONEY, chr->GetName());
            if (needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_ALL_MONEY_GONE, GetName());

            chr->SetMoney(0);
        }
        else
        {
            PSendSysMessage(LANG_YOU_TAKE_MONEY, abs(addmoney), chr->GetName());
            if (needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_MONEY_TAKEN, GetName(), abs(addmoney));
            chr->SetMoney(newmoney);
        }
    }
    else
    {
        PSendSysMessage(LANG_YOU_GIVE_MONEY, addmoney, chr->GetName());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_MONEY_GIVEN, GetName(), addmoney);
        chr->ModifyMoney(addmoney);
    }

    sLog.outDetail(GetHellgroundString(LANG_NEW_MONEY), moneyuser, addmoney, chr->GetMoney());

    return true;
}

//Edit Player field
bool ChatHandler::HandleModifyBitCommand(const char* args)
{
    if (!*args)
        return false;

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pBit = strtok(NULL, " ");
    if (!pBit)
        return false;

    uint16 field = atoi(pField);
    uint32 bit   = atoi(pBit);

    if (field < 1 || field >= PLAYER_END)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    if (bit < 1 || bit > 32)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->HasFlag(field, (1<<(bit-1))))
    {
        chr->RemoveFlag(field, (1<<(bit-1)));
        PSendSysMessage(LANG_REMOVE_BIT, bit, field);
    }
    else
    {
        chr->SetFlag(field, (1<<(bit-1)));
        PSendSysMessage(LANG_SET_BIT, bit, field);
    }

    return true;
}

bool ChatHandler::HandleModifyHonorCommand (const char* args)
{
    if (!*args)
        return false;

    Player *target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = (uint32)atoi(args);

    target->ModifyHonorPoints(amount);

    PSendSysMessage(LANG_COMMAND_MODIFY_HONOR, target->GetName(), target->GetHonorPoints());

    return true;
}

bool ChatHandler::HandleTeleCommand(const char * args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = extractGameTeleFromLink((char*)args);

    if (!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    MapEntry const * me = sMapStore.LookupEntry(tele->mapId);
    if (!me)
    {
        SendSysMessage("Map error");
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    _player->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
    return true;
}

bool ChatHandler::HandleLookupAreaCommand(const char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // converting string that we try to find to lower case
    wstrToLower (wnamepart);

    // Search in AreaTable.dbc
    for (uint32 areaflag = 0; areaflag < sAreaStore.GetNumRows (); ++areaflag)
    {
        AreaTableEntry const *areaEntry = sAreaStore.LookupEntry (areaflag);
        if (areaEntry)
        {
            int loc = m_session ? m_session->GetSessionDbcLocale () : sWorld.GetDefaultDbcLocale();
            std::string name = areaEntry->area_name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo (name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (m_session && loc==m_session->GetSessionDbcLocale ())
                        continue;

                    name = areaEntry->area_name[loc];
                    if (name.empty ())
                        continue;

                    if (Utf8FitTo (name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                // send area in "id - [name]" format
                std::ostringstream ss;
                if (m_session)
                    ss << areaEntry->ID << " - |cffffffff|Harea:" << areaEntry->ID << "|h[" << name << " " << localeNames[loc]<< "]|h|r";
                else
                    ss << areaEntry->ID << " - " << name << " " << localeNames[loc];

                SendSysMessage (ss.str ().c_str());

                ++counter;
            }
        }
    }
    if (counter == 0)                                      // if counter == 0 then we found nth
        SendSysMessage (LANG_COMMAND_NOAREAFOUND);
    return true;
}

//Find tele in game_tele order by name
bool ChatHandler::HandleLookupTeleCommand(const char * args)
{
    if (!*args)
    {
        SendSysMessage(LANG_COMMAND_TELE_PARAMETER);
        SetSentErrorMessage(true);
        return false;
    }

    char const* str = strtok((char*)args, " ");
    if (!str)
        return false;

    std::string namepart = str;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    std::ostringstream reply;

    GameTeleMap const & teleMap = sObjectMgr.GetGameTeleMap();
    for (GameTeleMap::const_iterator itr = teleMap.begin(); itr != teleMap.end(); ++itr)
    {
        GameTele const* tele = &itr->second;

        if (tele->wnameLow.find(wnamepart) == std::wstring::npos)
            continue;

        if (m_session)
            reply << "  |cffffffff|Htele:" << itr->first << "|h[" << tele->name << "]|h|r\n";
        else
            reply << "  " << itr->first << " " << tele->name << "\n";
    }

    if (reply.str().empty())
        SendSysMessage(LANG_COMMAND_TELE_NOLOCATION);
    else
        PSendSysMessage(LANG_COMMAND_TELE_LOCATION,reply.str().c_str());

    return true;
}

//Enable\Disable GM accept whispers and players ability to whisper to gm
bool ChatHandler::HandleWhispersCommand(const char* args)
{
    if (!*args)
    {
        PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING, m_session->GetPlayer()->isAcceptWhispers() ?  GetHellgroundString(LANG_ON) : GetHellgroundString(LANG_OFF));
        return true;
    }

    std::string firstpart = strtok((char*)args, " ");
    if (firstpart.empty())
        return false;

    // whisper on
    if (firstpart == "on")
    {
        m_session->GetPlayer()->SetAcceptWhispers(true);
        PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING,GetHellgroundString(LANG_ON));
        return true;
    }
    // whisper off
    else if (firstpart == "off")
    {
        m_session->GetPlayer()->SetAcceptWhispers(false);
        PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING,GetHellgroundString(LANG_OFF));
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::SendGMMail(const char* pName, const char* msgSubject, const char* msgText)
{
    if (!pName || !msgSubject || !msgText)
        return false;

    // pName, msgSubject, msgText isn't NUL after prev. check
    std::string name    = pName;
    std::string subject = msgSubject;
    std::string text    = msgText;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 receiver_guid = sObjectMgr.GetPlayerGUIDByName(name);
    if (!receiver_guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // from console show not existed sender
    MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM);

    uint32 itemTextId = !text.empty() ? sObjectMgr.CreateItemText(text) : 0;

    Player *receiver = sObjectMgr.GetPlayerInWorld(receiver_guid);

    MailDraft(subject, itemTextId)
        .SendMailTo(MailReceiver(receiver, ObjectGuid(receiver_guid)), sender);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());

    return true;
}

//Send mail by command
bool ChatHandler::HandleSendMailCommand(const char* args)
{
    if (!*args)
        return false;

    // format: name "subject text" "mail text"

    char* pName = strtok((char*)args, " ");
    if (!pName)
        return false;

    char* tail1 = strtok(NULL, "");
    if (!tail1)
        return false;

    char* msgSubject;
    if (*tail1=='"')
        msgSubject = strtok(tail1+1, "\"");
    else
    {
        char* space = strtok(tail1, "\"");
        if (!space)
            return false;
        msgSubject = strtok(NULL, "\"");
    }

    if (!msgSubject)
        return false;

    char* tail2 = strtok(NULL, "");
    if (!tail2)
        return false;

    char* msgText;
    if (*tail2=='"')
        msgText = strtok(tail2+1, "\"");
    else
    {
        char* space = strtok(tail2, "\"");
        if (!space)
            return false;
        msgText = strtok(NULL, "\"");
    }

    if (!msgText)
        return false;

    return SendGMMail(pName, msgSubject, msgText);
}

// teleport player to given game_tele.entry
bool ChatHandler::HandleTeleNameCommand(const char * args)
{
    if (!*args)
        return false;

    char* pName = strtok((char*)args, " ");

    if (!pName)
        return false;

    std::string name = pName;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    char* tail = strtok(NULL, "");
    if (!tail)
        return false;

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = extractGameTeleFromLink(tail);
    if (!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    MapEntry const * me = sMapStore.LookupEntry(tele->mapId);
    if (!me || me->IsBattleGroundOrArena())
    {
        SendSysMessage(LANG_CANNOT_TELE_TO_BG);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = sObjectMgr.GetPlayerInWorld(name.c_str());
    if (chr)
    {

        if (chr->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, chr->GetName(),"", tele->name.c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_TELEPORTED_TO_BY, GetName());

        // stop flight if need
        chr->InterruptTaxiFlying();

        chr->TeleportTo(tele->mapId,tele->position_x,tele->position_y,tele->position_z,tele->orientation);
    }
    else if (uint64 guid = sObjectMgr.GetPlayerGUIDByName(name.c_str()))
    {
        PSendSysMessage(LANG_TELEPORTING_TO, name.c_str(), GetHellgroundString(LANG_OFFLINE), tele->name.c_str());
        Player::SavePositionInDB(tele->mapId,tele->position_x,tele->position_y,tele->position_z,tele->orientation,sTerrainMgr.GetZoneId(tele->mapId,tele->position_x,tele->position_y,tele->position_z),guid);
    }
    else
        PSendSysMessage(LANG_NO_PLAYER, name.c_str());

    return true;
}

//Teleport group to given game_tele.entry
bool ChatHandler::HandleTeleGroupCommand(const char * args)
{
    if (!*args)
        return false;

    Player *player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = extractGameTeleFromLink((char*)args);
    if (!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    MapEntry const * me = sMapStore.LookupEntry(tele->mapId);
    if (!me || me->IsBattleGroundOrArena())
    {
        SendSysMessage(LANG_CANNOT_TELE_TO_BG);
        SetSentErrorMessage(true);
        return false;
    }
    Group *grp = player->GetGroup();
    if (!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP,player->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();

        if (!pl || !pl->GetSession())
            continue;

        if (pl->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName());
            continue;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, pl->GetName(),"", tele->name.c_str());
        if (needReportToTarget(pl))
            ChatHandler(pl).PSendSysMessage(LANG_TELEPORTED_TO_BY, GetName());

        // stop flight if need
        pl->InterruptTaxiFlying();

        pl->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
    }

    return true;
}

//Summon group of player
bool ChatHandler::HandleGroupgoCommand(const char* args)
{
    if (!*args)
        return false;

    std::string name = args;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *player = sObjectMgr.GetPlayerInWorld(name.c_str());
    if (!player)
    {
        PSendSysMessage(LANG_NO_PLAYER, args);
        SetSentErrorMessage(true);
        return false;
    }

    Group *grp = player->GetGroup();

    if (!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP,player->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    Map* gmMap = m_session->GetPlayer()->GetMap();
    bool to_instance =  gmMap->Instanceable();

    // we are in instance, and can summon only player in our group with us as lead
    if (to_instance && (
        !m_session->GetPlayer()->GetGroup() || (grp->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ||
        (m_session->GetPlayer()->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID())))
        // the last check is a bit excessive, but let it be, just in case
    {
        SendSysMessage(LANG_CANNOT_SUMMON_TO_INST);
        SetSentErrorMessage(true);
        return false;
    }

    for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();

        if (!pl || pl==m_session->GetPlayer() || !pl->GetSession())
            continue;

        if (pl->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        if (to_instance)
        {
            Map* plMap = pl->GetMap();

            if (plMap->Instanceable() && plMap->GetInstanciableInstanceId() != gmMap->GetInstanciableInstanceId())
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,pl->GetName());
                SetSentErrorMessage(true);
                return false;
            }
        }

        PSendSysMessage(LANG_SUMMONING, pl->GetName(),"");
        if (needReportToTarget(pl))
            ChatHandler(pl).PSendSysMessage(LANG_SUMMONED_BY, GetName());

        // stop flight if need
        pl->InterruptTaxiFlying();

        // before GM
        float x,y,z;
        m_session->GetPlayer()->GetNearPoint(x,y,z,pl->GetObjectSize());
        pl->TeleportTo(m_session->GetPlayer()->GetMapId(),x,y,z,pl->GetOrientation());
    }

    return true;
}

//teleport at coordinates
bool ChatHandler::HandleGoXYCommand(const char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pmapid = strtok(NULL, " ");

    if (!px || !py)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else mapid = _player->GetMapId();

    if (!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    TerrainInfo const *map = sTerrainMgr.LoadTerrain(mapid);
    float z = map->GetWaterOrGroundLevel(x, y, MAX_HEIGHT);

    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport at coordinates, including Z
bool ChatHandler::HandleGoXYZCommand(const char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");
    char* pmapid = strtok(NULL, " ");

    if (!px || !py || !pz)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else
        mapid = _player->GetMapId();

    if (!MapManager::IsValidMapCoord(mapid,x,y,z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport at coordinates
bool ChatHandler::HandleGoZoneXYCommand(const char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* tail = strtok(NULL,"");

    char* cAreaId = extractKeyFromLink(tail,"Harea");       // string or [name] Shift-click form |color|Harea:area_id|h[name]|h|r

    if (!px || !py)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);

    // prevent accept wrong numeric args
    if (x==0.0f && *px!='0' || y==0.0f && *py!='0')
        return false;

    uint32 areaid = cAreaId ? (uint32)atoi(cAreaId) : _player->GetCachedZone();

    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(areaid);

    if (x<0 || x>100 || y<0 || y>100 || !areaEntry)
    {
        PSendSysMessage(LANG_INVALID_ZONE_COORD,x,y,areaid);
        SetSentErrorMessage(true);
        return false;
    }

    // update to parent zone if exist (client map show only zones without parents)
    AreaTableEntry const* zoneEntry = areaEntry->zone ? GetAreaEntryByAreaID(areaEntry->zone) : areaEntry;

    MapEntry const *mapEntry = sMapStore.LookupEntry(zoneEntry->mapid);

    if (mapEntry->Instanceable())
    {
        PSendSysMessage(LANG_INVALID_ZONE_MAP,areaEntry->ID,areaEntry->area_name[m_session->GetSessionDbcLocale()],mapEntry->MapID,mapEntry->name);
        SetSentErrorMessage(true);
        return false;
    }

    Map *map = sMapMgr.FindMap(zoneEntry->mapid, sObjectMgr.GetSingleInstance(zoneEntry->mapid, x, y));

    Zone2MapCoordinates(x,y,zoneEntry->ID);

    if (!MapManager::IsValidMapCoord(zoneEntry->mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,zoneEntry->mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    float z = map->GetTerrain()->GetWaterOrGroundLevel(x, y, MAX_HEIGHT);
    _player->TeleportTo(zoneEntry->mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport to grid
bool ChatHandler::HandleGoGridCommand(const char* args)
{
    if (!*args)    return false;
    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pmapid = strtok(NULL, " ");

    if (!px || !py)
        return false;

    float grid_x = (float)atof(px);
    float grid_y = (float)atof(py);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else mapid = _player->GetMapId();

    // center of grid
    float x = (grid_x-CENTER_GRID_ID+0.5f)*SIZE_OF_GRIDS;
    float y = (grid_y-CENTER_GRID_ID+0.5f)*SIZE_OF_GRIDS;

    if (!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    MapEntry const *mapEntry = sMapStore.LookupEntry(mapid);

    if (mapEntry->Instanceable())
    {
        SetSentErrorMessage(true);
        return false;
    }

    if (Map *map = sMapMgr.FindMap(mapid, sObjectMgr.GetSingleInstance(mapid, x, y)))
    {
        float z = map->GetTerrain()->GetWaterOrGroundLevel(x, y, MAX_HEIGHT);
        _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());
        return true;
    }

    return false;
}

bool ChatHandler::HandleModifyDrunkCommand(const char* args)
{
    if (!*args)    return false;

    uint32 drunklevel = (uint32)atoi(args);
    if (drunklevel > 100)
        drunklevel = 100;

    uint16 drunkMod = drunklevel * 0xFFFF / 100;

    m_session->GetPlayer()->SetDrunkValue(drunkMod);

    return true;
}

bool ChatHandler::HandleNpcStandState(const char* args)
{
    uint32 state = atoi((char*)args);

    Creature* target = getSelectedCreature();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->SetStandState(state);

    return true;
}

bool ChatHandler::HandleGMFreeCommand(const char* args)
{
    Player* p = m_session->GetPlayer();
    sWorld.SendWorldText(m_session->GetPermissions() == 3 ? 15499 : LANG_GM_FREE_ANNOUNCE, 0, m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName());
    return true;
}

bool ChatHandler::HandleCacheItemCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 item = (uint32)atoi((char*)args);

    if (!item)
        return false;

    Player* plr = getSelectedPlayer();
    if (!plr)
        plr = m_session->GetPlayer();

    WorldPacket data(CMSG_ITEM_QUERY_SINGLE, 4);
    data << item;
    plr->GetSession()->HandleItemQuerySingleOpcode(data);
    return true;
}

//bool ChatHandler::HandleLootItemInfoCommand(const char* args)
//{
//    return false; // disabled
//    
//    if (!*args)
//        return false;
//
//    char* cId = extractKeyFromLink((char*)args, "Hitem");
//    if (!cId)
//        return false;
//
//    uint32 itemId = atol(cId);
//
//    if (!itemId)
//        return false;
//
//    char* instId = strtok(NULL, " "); // instance Id may be NULL
//    uint32 instance_id = 0;
//    if (instId)
//        instance_id = strtol(instId, NULL, 10);
//    else
//        instance_id = m_session->GetPlayer()->GetInstanciableInstanceId();
//
//    QueryResultAutoPtr resultLog = RealmDataDatabase.PQuery("SELECT `id`, `looterguid`, `receiverguid`, `count`, `boss`, `method`, `date`, `fixed_by` FROM `loot_items_log` WHERE `item_template`='%u' AND `instance_id`='%u' AND `method` < 5", itemId, instance_id);
//    if (!resultLog)
//    {
//        SendSysMessage("No one has got such itemid from such instanceid");
//        return true;
//    }
//
//    // result exists
//    do
//    {
//        Field* fieldsLog = resultLog->Fetch();
//        uint32 receiverGUID = fieldsLog[2].GetUInt32();
//        std::string receiverName = "NOT_FOUND";
//        sObjectMgr.GetPlayerNameByGUID(MAKE_NEW_GUID(receiverGUID, 0, HIGHGUID_PLAYER), receiverName);
//
//        PSendSysMessage("A player \'%s\' (guid %u) received this item:", receiverName.c_str(), receiverGUID);
//
//        // info from loot_items_log
//        uint32 looterGUID = fieldsLog[1].GetUInt32();
//        std::string looterName = "NOT_FOUND";
//        sObjectMgr.GetPlayerNameByGUID(MAKE_NEW_GUID(looterGUID, 0, HIGHGUID_PLAYER), looterName);
//        uint32 bossEntry = fieldsLog[4].GetUInt32();
//        char const* bossName = "NOT_FOUND";
//
//        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(bossEntry);
//        if (cInfo)
//            bossName = cInfo->Name;
//
//        std::string lootMethodName;
//        switch (fieldsLog[5].GetUInt8())
//        {
//            case FREE_FOR_ALL:
//                lootMethodName = "FreeForAll";
//                break;
//            case MASTER_LOOT:
//                lootMethodName = "MasterLoot";
//                break;
//            default:
//                lootMethodName = "NotSupported";
//                break;
//        }
//
//        uint32 fixed_by = fieldsLog[7].GetUInt32();
//        PSendSysMessage("Loot ID: %u, Looter: \'%s\' (GUID %u), Count: %u, Creature \'%s\' (entry %u), Method: \'%s\', Date: %s, Fixed by Loot ID: %u", fieldsLog[0].GetUInt32(),
//            looterName.c_str(), looterGUID, fieldsLog[3].GetUInt8(), bossName, bossEntry, lootMethodName.c_str(), fieldsLog[6].GetString(), fixed_by);
//
//        SendSysMessage("Who could loot from this boss:");
//        QueryResultAutoPtr whoCanLoot = RealmDataDatabase.PQuery("SELECT `playerGuids`, `playerAccs`, `raidRules` FROM `loot_loot_log` WHERE `creatureId`='%u' AND `instanceId`='%u'", bossEntry, instance_id);
//        if (!whoCanLoot)
//            SendSysMessage("Players who could loot this creature not found. (it is probably not a boss)");
//        else if ((*whoCanLoot)[2].GetUInt8() == 0) // no Raid Rules were set
//            SendSysMessage("There were no Raid Rules for this boss!");
//        else if ((*whoCanLoot)[2].GetUInt8() == 2)
//            SendSysMessage("Group on this boss was not found - raid rules not found!");
//        else
//        {
//            // for characters
//            std::set<uint64> allowedToLoot;
//            allowedToLoot.clear();
//            std::stringstream guids;
//            uint32 playerscount;
//            uint64 playerguid;
//
//            guids << (*whoCanLoot)[0].GetString();
//            guids >> playerscount;
//            if (!playerscount)
//                SendSysMessage("Info for players not found");
//            for (uint32 i = 0; i < playerscount; ++i)
//            {
//                guids >> playerguid;
//                allowedToLoot.insert(playerguid);
//            }
//
//            std::string playername;
//            std::string names = "";
//            uint32 sendEvery5 = 0;
//            for (std::set<uint64>::const_iterator itr = allowedToLoot.begin(); itr != allowedToLoot.end(); itr++)
//            {
//                playername = "";
//                if (sObjectMgr.GetPlayerNameByGUID(*itr, playername))
//                {
//                    names += playername;
//                    names += " (";
//                    char guid[16];
//                    sprintf(guid, "%u", GUID_LOPART(*itr));
//                    names += guid;
//                    names += "), ";
//                }
//                if (sendEvery5++ > 4)
//                {
//                    PSendSysMessage("%s", names.c_str());
//                    names = "";
//                    sendEvery5 = 0;
//                }
//            }
//            if (names != "")
//                PSendSysMessage("%s", names.c_str());
//
//            // for accounts
//            std::set<uint32> allowedToLootAccs;
//            allowedToLootAccs.clear();
//            std::stringstream accs;
//            uint32 accsCount;
//            uint32 accId;
//
//            accs << (*whoCanLoot)[1].GetString();
//            accs >> accsCount;
//            if (!accsCount)
//                SendSysMessage("Info for accounts not found");
//            for (uint32 i = 0; i < accsCount; ++i)
//            {
//                accs >> accId;
//                allowedToLootAccs.insert(accId);
//            }
//
//            names = "";
//            sendEvery5 = 0;
//            for (std::set<uint32>::const_iterator itr = allowedToLootAccs.begin(); itr != allowedToLootAccs.end(); itr++)
//            {
//                char acc[16];
//                sprintf(acc, "%u", *itr);
//                names += acc;
//                names += ", ";
//
//                if (sendEvery5++ > 4)
//                {
//                    PSendSysMessage("%s", names.c_str());
//                    names = "";
//                    sendEvery5 = 0;
//                }
//            }
//            if (names != "")
//                PSendSysMessage("%s", names.c_str());
//        }
//
//        while (fixed_by)
//        {
//            SendSysMessage("FIXED BY:");
//            QueryResultAutoPtr fixedByRes = RealmDataDatabase.PQuery("SELECT `id`, `looterguid`, `receiverguid`, `date`, `forum_id`, `fixed_by` FROM `loot_items_log` WHERE `id`='%u' AND `method` IN (5,6)", fixed_by);
//            if (!fixedByRes)
//            {
//                SendSysMessage("Fixed_by not found!");
//                break;
//            }
//
//            uint32 receiverGUID = (*fixedByRes)[2].GetUInt32();
//            std::string receiverName = "NOT_FOUND";
//            sObjectMgr.GetPlayerNameByGUID(MAKE_NEW_GUID(receiverGUID, 0, HIGHGUID_PLAYER), receiverName);
//
//            PSendSysMessage("A player \'%s\' (guid %u) received this item:", receiverName.c_str(), receiverGUID);
//
//            // info from loot_items_log
//            uint32 looterGUID = (*fixedByRes)[1].GetUInt32();
//            std::string looterName = "NOT_FOUND";
//            sObjectMgr.GetPlayerNameByGUID(MAKE_NEW_GUID(looterGUID, 0, HIGHGUID_PLAYER), looterName);
//
//            fixed_by = (*fixedByRes)[5].GetUInt32();
//            PSendSysMessage("Loot ID: %u, GM_Fixer: \'%s\' (GUID %u), Date: %s, Forum topic ID: %u, Fixed by Loot ID: %u", (*fixedByRes)[0].GetUInt32(),
//                looterName.c_str(), looterGUID, (*fixedByRes)[3].GetString(), (*fixedByRes)[4].GetUInt32(), fixed_by);
//
//        }
//
//    } while (resultLog->NextRow());
//
//    return true;
//}
//
//bool ChatHandler::HandleLootPlayerInfoCommand(const char* args)
//{
//    return false; // disabled
//    
//    if (!*args)
//        return false;
//
//    char* cinstance_id = strtok((char*)args, " ");
//    
//    // Instead of kill id's use instance id and player name (select by GUID).
//    if (!cinstance_id)
//        return false;
//
//    uint32 instance_id = atol(cinstance_id);
//    uint64 playerGUID = 0;
//    char* cplrName = strtok(NULL, "");
//
//    std::string plrName = "";
//    if (cplrName)
//    {
//        plrName = cplrName;
//        if (!normalizePlayerName(plrName))
//            return true;
//
//        playerGUID = sObjectMgr.GetPlayerGUIDByName(plrName.c_str());
//    }
//    else
//    {
//        if (Player* plr = getSelectedPlayer())
//        {
//            playerGUID = plr->GetGUID();
//            plrName = plr->GetName();
//        }
//        else
//            return false;
//    }
//    if (!playerGUID)
//    {
//        SendSysMessage("Player not found");
//        return true;
//    }
//        
//    PSendSysMessage("Information from GBK about instance_id %u for a player \'%s\' (guid %u):", instance_id, plrName.c_str(), GUID_LOPART(playerGUID));
//
//    QueryResultAutoPtr plrInfo = RealmDataDatabase.PQuery(
//        "SELECT `boss_id`, `damage`, `healing`, `deaths` FROM `boss_fights_detailed` bfd "
//        "INNER JOIN `boss_fights` bf ON bfd.`kill_id`=bf.`kill_id` WHERE `instance_id`='%u' AND `player_guid`='%u'",
//        instance_id, GUID_LOPART(playerGUID));
//
//    if (!plrInfo)
//    {
//        PSendSysMessage("Player info for instance %u not found", instance_id);
//        return true;
//    }
//
//    // result exists
//    do
//    {
//        Field* fieldsIfo = plrInfo->Fetch();
//        uint32 boss_id = fieldsIfo[0].GetUInt32();
//
//        PSendSysMessage("Player participated in fight with '%s': damage: %u, healing: %u, deaths: %u", 
//            sGuildMgr.GBK_GetBossNameForEvent(boss_id).c_str(), fieldsIfo[1].GetUInt32() , fieldsIfo[2].GetUInt32(), fieldsIfo[3].GetUInt32());
//
//    } while (plrInfo->NextRow());
//
//    return true;
//}
//
//bool ChatHandler::HandleLootDeleteCommand(const char* args)
//{
//    return false; // disabled
//    
//    uint32 gmAccId = m_session->GetAccountId();
//    if (sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_GM_LOOT_DEL))
//    {
//        if (!sWorld.HasLootDeleteCharge(gmAccId))
//        {
//            SendSysMessage("You have no charges left for this command");
//            return true;
//        }
//    }
//
//    if (!*args)
//        return false;
//
//    char* cloot_id = strtok((char*)args, " ");
//    if (!cloot_id)
//        return false;
//
//    uint32 loot_id = atol(cloot_id);
//
//    char* cforum_id = strtok(NULL, "");
//    if (!cforum_id)
//        return false;
//
//    uint32 forum_id = atol(cforum_id);
//
//    if (!loot_id || !forum_id)
//        return false;
//
//    QueryResultAutoPtr resultLog = RealmDataDatabase.PQuery("SELECT `receiverguid`, `item`, `count`, `item_template`, `fixed_by` FROM `loot_items_log` WHERE `id`='%u'", loot_id);
//    if (!resultLog)
//    {
//        PSendSysMessage("No loot with id %u found", loot_id);
//        return false;
//    }
//    else
//    {
//        if ((*resultLog)[4].GetUInt32() != 0) // already deleted
//        {
//            SendSysMessage("This loot id is already deleted");
//            return true;
//        }
//
//        if ((*resultLog)[0].GetUInt32() == 0) // no receiver found - no one to delete from!
//        {
//            SendSysMessage("There is no receiver on this loot_id");
//            return true;
//        }
//
//        QueryResultAutoPtr maxLog = RealmDataDatabase.Query("SELECT MAX(`id`) FROM `loot_items_log`");
//        if (!maxLog)
//        {
//            SendSysMessage("Max loot id not found");
//            return true;
//        }
//
//        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype>((*resultLog)[3].GetUInt32());
//        if (!pProto)
//        {
//            PSendSysMessage("Cannot find item_template for entry %u", (*resultLog)[3].GetUInt32());
//            return true;
//        }
//
//        Player* plr = sObjectAccessor.GetPlayerInWorldOrNot(MAKE_NEW_GUID((*resultLog)[0].GetUInt32(), 0, HIGHGUID_PLAYER));
//        if (plr && !plr->IsInWorld())
//        {
//            SendSysMessage(LANG_PLAYER_LOADING_WAIT);
//            return true;
//        }
//
//        if (plr) // easy - he is online
//        {
//            if (!plr->IsInWorld())
//            {
//                SendSysMessage("Player is loading at the moment, he must be in world to get an update of item removal. Wait some time, please.");
//                return true;
//            }
//            if (!plr->HasItemCount((*resultLog)[3].GetUInt32(), (*resultLog)[2].GetUInt8(), true))
//            {
//                PSendSysMessage("Player does not have %u of itemid %u!", (*resultLog)[2].GetUInt8(), (*resultLog)[3].GetUInt32());
//                return true;
//            }
//            char toMailSubj[128];
//            sprintf(toMailSubj, "%s", plr->GetSession()->GetHellgroundString(LANG_MAIL_SUBJECT_ITEM_REASSIGN));
//
//            char toMailText[1024];
//            std::string itemName;
//            sprintf(toMailText, plr->GetSession()->GetHellgroundString(LANG_MAIL_TEXT_ITEM_TAKEN), 
//                plr->GetItemNameLocale(pProto, &itemName), (*resultLog)[2].GetUInt8());
//
//            if (!SendGMMail(plr->GetName(), toMailSubj, toMailText))
//            {
//                SendSysMessage("Could not send mail -> nothing is taken!");
//                return true;
//            }
//            plr->DestroyItemCount((*resultLog)[3].GetUInt32(), (*resultLog)[2].GetUInt8(), true, false);
//        }
//        else // offline case
//        {
//            uint32 accId = sObjectMgr.GetPlayerAccountIdByGUID((*resultLog)[0].GetUInt32());
//            if (!accId)
//            {
//                SendSysMessage("Could not find account id for the receiver!");
//                return true;
//            }
//            QueryResultAutoPtr resultLoc = AccountsDatabase.PQuery("SELECT `locale_id`, `account_flags` FROM `account` WHERE account_id='%u'", accId);
//            if (!resultLoc)
//            {
//                SendSysMessage("Could not find locale id for the receiver!");
//                return true;
//            }
//            uint8 loc_idx = sObjectMgr.GetIndexForLocale(((*resultLoc)[1].GetUInt64() & ACC_INFO_LANG_RU) ? LOCALE_ruRU : LocaleConstant((*resultLoc)[0].GetUInt8()));
//
//            char toMailSubj[128];
//            sprintf(toMailSubj, "%s", sObjectMgr.GetHellgroundString(LANG_MAIL_SUBJECT_ITEM_REASSIGN, loc_idx));
//
//            char toMailText[1024];
//            std::string itemName = pProto->Name1;
//            sObjectMgr.GetItemLocaleStrings(pProto->ItemId, loc_idx, &itemName);
//            sprintf(toMailText, sObjectMgr.GetHellgroundString(LANG_MAIL_TEXT_ITEM_TAKEN, loc_idx), itemName.c_str(), (*resultLog)[2].GetUInt8());
//
//            std::string plrName = "";
//            sObjectMgr.GetPlayerNameByGUID(MAKE_NEW_GUID((*resultLog)[0].GetUInt32(), 0, HIGHGUID_PLAYER), plrName);
//            if (pProto->Stackable > 1) // cannot remove stackable items via just deletion, it would delete whole stack
//            {
//                QueryResultAutoPtr itemGuids = RealmDataDatabase.PQuery("SELECT a.`guid`, a.`data` "
//                    "FROM `item_instance` a INNER JOIN character_inventory b ON a.`guid`=b.`item` AND b.`guid`='%u' AND b.item_template='%u'"
//                    , (*resultLog)[0].GetUInt32(), (*resultLog)[3].GetUInt32());
//
//                if (!itemGuids)
//                {
//                    SendSysMessage("No stackable items found on a player");
//                    return true;
//                }
//
//                std::map<uint32, uint32> guidAndCount;
//                std::map<uint32, std::string> datas;
//                uint32 needToTake = (*resultLog)[2].GetUInt8();
//                uint32 alreadyFound = 0;
//                uint32 bestSuitedGUID = 0;
//                // count how many items he has
//                do
//                {
//                    Field* fieldsItems = itemGuids->Fetch();
//                    std::string str = fieldsItems[1].GetCppString();
//                    datas[fieldsItems[0].GetUInt32()] = str;
//                    std::string subStr = "";
//                    int32 leftToGo = 14;
//                    for (auto i = str.begin(); i != str.end(); ++i) // count is on 15th pos - after 14 of ' '
//                    {
//                        if (*i == ' ')
//                        {
//                            if (--leftToGo < 0)
//                                break;
//                        }
//                        else if (leftToGo == 0)
//                            subStr += *i;
//                    }
//                    if (subStr == "")
//                        continue;
//                    uint32 cntFound = atoi(subStr.c_str());
//                    guidAndCount[fieldsItems[0].GetUInt32()] = cntFound;
//                    if (cntFound >= needToTake)
//                    {
//                        bestSuitedGUID = fieldsItems[0].GetUInt32();
//                        break;
//                    }
//                    else
//                    {
//                        alreadyFound += cntFound;
//                        if (alreadyFound >= needToTake)
//                            break;
//                    }
//                } while (itemGuids->NextRow());
//
//                if (!bestSuitedGUID && alreadyFound < needToTake)
//                {
//                    PSendSysMessage("Player does not have %u of itemid %u!", (*resultLog)[2].GetUInt8(), (*resultLog)[3].GetUInt32());
//                    return true;
//                }
//
//                if (!SendGMMail(plrName.c_str(), toMailSubj, toMailText))
//                {
//                    SendSysMessage("Could not send mail -> nothing is taken!");
//                    return true;
//                }
//
//                if (bestSuitedGUID) // need to update only 1 item, it has same or more count already
//                {
//                    if (guidAndCount[bestSuitedGUID] == needToTake) // need to delete pack
//                    {
//                        RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", bestSuitedGUID);
//                        RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", bestSuitedGUID);
//                    }
//                    else
//                    {
//                        guidAndCount[bestSuitedGUID] -= needToTake; // need to update pack
//                        uint32 startFrom = 0;
//                        uint32 countOfChars = 0;
//                        int32 leftToGo = 14;
//                        for (auto i = datas[bestSuitedGUID].begin(); i != datas[bestSuitedGUID].end(); ++i)
//                        {
//                            ++startFrom;
//                            if (*i == ' ')
//                            {
//                                if (--leftToGo < 0)
//                                    break;
//                            }
//                            else if (leftToGo == 0)
//                                ++countOfChars;
//                        }
//                        --startFrom;
//                        char newcnt[8];
//                        sprintf(newcnt, "%u", guidAndCount[bestSuitedGUID]);
//                        datas[bestSuitedGUID].replace(startFrom - countOfChars, countOfChars, newcnt);
//                        RealmDataDatabase.PExecute("UPDATE item_instance SET `data`='%s' WHERE guid = '%u'", datas[bestSuitedGUID].c_str(), bestSuitedGUID);
//                    }
//                }
//                else
//                {
//                    for (std::map<uint32, uint32>::iterator itr = guidAndCount.begin(); itr != guidAndCount.end() && needToTake > 0; ++itr)
//                    {
//                        if (itr->second <= needToTake) // need to delete pack
//                        {
//                            needToTake -= itr->second;
//                            RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", itr->first);
//                            RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", itr->first);
//                        }
//                        else // if gone to else -> this is the last then
//                        {
//                            itr->second -= needToTake; // need to update pack
//                            uint32 startFrom = 0;
//                            uint32 countOfChars = 0;
//                            int32 leftToGo = 14;
//                            for (auto i = datas[itr->first].begin(); i != datas[itr->first].end(); ++i)
//                            {
//                                ++startFrom;
//                                if (*i == ' ')
//                                {
//                                    if (--leftToGo < 0)
//                                        break;
//                                }
//                                else if (leftToGo == 0)
//                                    ++countOfChars;
//                            }
//                            --startFrom;
//                            char newcnt[8];
//                            sprintf(newcnt, "%u", itr->second);
//                            datas[itr->first].replace(startFrom - countOfChars, countOfChars, newcnt);
//                            RealmDataDatabase.PExecute("UPDATE item_instance SET `data`='%s' WHERE guid = '%u'", datas[itr->first].c_str(), itr->first);
//                            break;
//                        }
//                    }
//                }
//            }
//            else
//            {
//                if (!(*resultLog)[1].GetUInt32())
//                {
//                    SendSysMessage("Item GUID not found for this loot_id");
//                    return true;
//                }
//                QueryResultAutoPtr hasItem = RealmDataDatabase.PQuery("SELECT 1 FROM character_inventory WHERE `item`='%u'", (*resultLog)[1].GetUInt32());
//                if (!hasItem)
//                {
//                    PSendSysMessage("Player has no itemguid %u, itemid %u", (*resultLog)[1].GetUInt32(), (*resultLog)[3].GetUInt32());
//                    return true;
//                }
//
//                if (!SendGMMail(plrName.c_str(), toMailSubj, toMailText))
//                {
//                    SendSysMessage("Could not send mail -> nothing is taken!");
//                    return true;
//                }
//
//                RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", (*resultLog)[1].GetUInt32());
//                RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", (*resultLog)[1].GetUInt32());
//            }
//        }
//
//        uint32 newId = (*maxLog)[0].GetUInt32() + 20;
//
//        RealmDataDatabase.PExecute("UPDATE `loot_items_log` SET `fixed_by`='%u' WHERE `id`='%u'", newId, loot_id);
//        static SqlStatementID lootDeleteCommandInsert;
//
//        SqlStatement stmt = RealmDataDatabase.CreateStatement(lootDeleteCommandInsert, "INSERT INTO loot_items_log VALUES (?, ?, 0, 0, 0, 0, 0, 0, '5', NOW(), ?, 0)");
//        stmt.addUInt32(newId); // new ID of loot_log
//        stmt.addUInt32(m_session->GetPlayer()->GetGUIDLow()); // looterguid - GM lowGuid here
//        stmt.addUInt32(forum_id); // topic id
//        stmt.Execute();
//
//        sWorld.LootDeleteUsed(gmAccId);
//    }
//    return true;
//}
//
//bool ChatHandler::HandleLootMoveCommand(const char* args)
//{
//    return false; // disabled
//    
//    if (!*args)
//        return false;
//
//    char* cloot_id = strtok((char*)args, " ");
//
//    if (!cloot_id)
//        return false;
//
//    uint32 loot_id = atol(cloot_id);
//
//    uint64 playerGUID = 0;
//    char* cplrName = strtok(NULL, "");
//
//    std::string plrName = "";
//    uint32 motherAcc = 0;
//    if (cplrName)
//    {
//        plrName = cplrName;
//        if (!normalizePlayerName(plrName))
//            return true;
//
//        playerGUID = sObjectMgr.GetPlayerGUIDByName(plrName.c_str());
//        /*Account-shared loot only for REALM_X100*/
//        if (sWorld.isEasyRealm())
//        {
//            uint32 plrAcc = sObjectMgr.GetPlayerAccountIdByGUID(playerGUID);
//            if (!plrAcc)
//            {
//                SendSysMessage("Player account not found");
//                return true;
//            }
//
//            std::string email = "";
//            QueryResultAutoPtr emailResult =
//                AccountsDatabase.PQuery("SELECT `email` FROM account WHERE `account_id`='%u'", plrAcc);
//            if (emailResult)
//                email = (*emailResult)[0].GetCppString(); // email of this account
//
//            if (!email.empty())
//            {
//                QueryResultAutoPtr motherAccResult =
//                    AccountsDatabase.PQuery("SELECT MIN(`account_id`) FROM `account` WHERE `email` = '%s'", email.c_str());
//
//                if (motherAccResult) // mother acc ID, selected by e-mail
//                    motherAcc = (*motherAccResult)[0].GetUInt32();
//                else
//                    motherAcc = plrAcc; // nothing found - should never happen, but anyway, set this accounts id for mother then
//            }
//            else
//                motherAcc = plrAcc; // no email on this account
//        }
//    }
//    else
//    {
//        if (Player* plr = getSelectedPlayer())
//        {
//            playerGUID = plr->GetGUID();
//            plrName = plr->GetName();
//            /*Account-shared loot only for REALM_X100*/
//            if (sWorld.isEasyRealm())
//                motherAcc = plr->GetSession()->GetMotherAccId();
//        }
//        else
//            return false;
//    }
//
//    if (!playerGUID)
//    {
//        SendSysMessage("Player not found");
//        return true;
//    }
//
//    /*Account-shared loot only for REALM_X100*/
//    if (!motherAcc && sWorld.isEasyRealm())
//    {
//        SendSysMessage("Motheracc not found");
//        return true;
//    }
//
//    QueryResultAutoPtr resultLog = RealmDataDatabase.PQuery("SELECT `count`, `item_template`, `instance_id`, `boss`, `fixed_by` FROM `loot_items_log` WHERE `id`='%u'", loot_id);
//    if (!resultLog)
//    {
//        PSendSysMessage("No loot with id %u found", loot_id);
//        return false;
//    }
//    else
//    {
//        uint32 itemCount = (*resultLog)[0].GetUInt32();
//        if (itemCount <= 0) // count check
//        {
//            SendSysMessage("Count is lower or is 0. Error.");
//            return true;
//        }
//        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype>((*resultLog)[1].GetUInt32()); // item_id check
//        if (!pProto)
//        {
//            PSendSysMessage("Cannot find item_template for entry %u", (*resultLog)[1].GetUInt32());
//            return true;
//        }
//        // check if our new player was able to loot this item - check by instance_id + boss_id
//        QueryResultAutoPtr whoCanLoot = RealmDataDatabase.PQuery("SELECT `playerGuids`, `playerAccs`, `raidRules` FROM `loot_loot_log` WHERE `creatureId`='%u' AND `instanceId`='%u'", (*resultLog)[3].GetUInt32(), (*resultLog)[2].GetUInt32());
//        if (!whoCanLoot)
//        {
//            SendSysMessage("Players who could loot this creature not found. (it is probably not a boss)");
//            return true;
//        }
//        else if ((*whoCanLoot)[2].GetUInt8() == 0) // no Raid Rules were set
//        {
//            SendSysMessage("There were no Raid Rules for this boss!");
//            return true;
//        }
//        else if ((*whoCanLoot)[2].GetUInt8() == 2)
//        {
//            SendSysMessage("Group on this boss was not found - raid rules not found!");
//            return true;
//        }
//        else
//        {
//            // check by character
//            std::stringstream guids;
//            uint32 playerscount;
//            uint64 playerguid;
//            bool ok = false;
//
//            guids << (*whoCanLoot)[0].GetString();
//            guids >> playerscount;
//            if (!playerscount)
//                SendSysMessage("Info about who could loot by guid is not found.");
//            for (uint32 i = 0; i < playerscount; ++i)
//            {
//                guids >> playerguid;
//                if (playerguid == playerGUID) // player was able to loot -> ok!
//                {
//                    ok = true;
//                    break;
//                }
//            }
//
//            /*Account-shared loot only for REALM_X100*/
//            if (!ok && sWorld.isEasyRealm()) // check by account
//            {
//                std::stringstream accs;
//                uint32 accsCount;
//                uint32 accId;
//
//                accs << (*whoCanLoot)[1].GetString();
//                accs >> accsCount;
//                if (!accsCount)
//                    SendSysMessage("Info about who could loot by accounts is not found.");
//                for (uint32 i = 0; i < accsCount; ++i)
//                {
//                    accs >> accId;
//                    if (accId == motherAcc) // player was able to loot -> ok!
//                    {
//                        ok = true;
//                        break;
//                    }
//                }
//            }
//
//            if (!ok)
//            {
//                PSendSysMessage("Player %s could not loot this creature!", plrName.c_str());
//                return true;
//            }
//        }
//
//        if ((*resultLog)[4].GetUInt32() == 0) // loot should be deleted first
//        {
//            SendSysMessage("This loot id is not deleted yet from the receiver. You must delete the item from the receiver first, using command '.loot delete'");
//            return true;
//        }
//
//        QueryResultAutoPtr fixedLog = RealmDataDatabase.PQuery("SELECT `receiverguid`, `looterguid` FROM `loot_items_log` WHERE `id`='%u'", (*resultLog)[4].GetUInt32());
//        if (!fixedLog)
//        {
//            PSendSysMessage("No fixed_loot %u found, but there is id %u that points to it!", (*resultLog)[4].GetUInt32(), loot_id);
//            return true;
//        }
//        if ((*fixedLog)[0].GetUInt32()) // there is receiverGUID
//        {
//            PSendSysMessage("This loot was already reassigned(fixed) to playerLowGUID %u", (*fixedLog)[0].GetUInt32());
//            return true;
//        }
//        if ((*fixedLog)[1].GetUInt32() != m_session->GetPlayer()->GetGUIDLow()) // we are not the one who deleted item
//        {
//            PSendSysMessage("Only GM_Fixer %u can reassign loot", (*fixedLog)[1].GetUInt32());
//            return true;
//        }
//
//        // Now everything is checked - we can add item to a player!
//
//        uint8 loc_idx = 0;
//        Player* plr = sObjectAccessor.GetPlayerInWorldOrNot(playerGUID);
//        if (plr && !plr->IsInWorld())
//        {
//            SendSysMessage(LANG_PLAYER_LOADING_WAIT);
//            return true;
//        }
//
//        if (plr) // easy - he is online
//        {
//            loc_idx = plr->GetSession()->IsAccountFlagged(ACC_INFO_LANG_RU) ? sObjectMgr.GetIndexForLocale(LOCALE_ruRU) : plr->GetSession()->GetSessionDbLocaleIndex();
//        }
//        else
//        {
//            uint32 accId = sObjectMgr.GetPlayerAccountIdByGUID(GUID_LOPART(playerGUID));
//            if (!accId)
//            {
//                SendSysMessage("Could not find account id for new receiver!");
//                return true;
//            }
//            QueryResultAutoPtr resultLoc = AccountsDatabase.PQuery("SELECT `locale_id`, `account_flags` FROM `account` WHERE account_id='%u'", accId);
//            if (!resultLoc)
//            {
//                SendSysMessage("Could not find locale id for the receiver!");
//                return true;
//            }
//            loc_idx = sObjectMgr.GetIndexForLocale(((*resultLoc)[1].GetUInt64() & ACC_INFO_LANG_RU) ? LOCALE_ruRU : LocaleConstant((*resultLoc)[0].GetUInt8()));
//        }
//
//        char toMailSubj[128];
//        sprintf(toMailSubj, "%s", sObjectMgr.GetHellgroundString(LANG_MAIL_SUBJECT_ITEM_REASSIGN, loc_idx));
//
//        char toMailText[1024];
//        std::string itemName = pProto->Name1;
//        sObjectMgr.GetItemLocaleStrings(pProto->ItemId, loc_idx, &itemName);
//        sprintf(toMailText, sObjectMgr.GetHellgroundString(LANG_MAIL_TEXT_ITEM_GIVEN, loc_idx), itemName.c_str(), itemCount);
//
//        // pName, msgSubject, msgText isn't NUL after prev. check
//        std::string subject = toMailSubj;
//        std::string text = toMailText;
//
//        // extract items
//        typedef std::pair<uint32, uint32> ItemPair;
//        typedef std::list< ItemPair > ItemPairs;
//        ItemPairs items;
//
//        uint32 item_count_mail = itemCount; // it is 1 or more 100% here
//        if (pProto->MaxCount && item_count_mail > pProto->MaxCount)
//        {
//            PSendSysMessage(LANG_COMMAND_INVALID_ITEM_COUNT, item_count_mail, pProto->ItemId);
//            SetSentErrorMessage(true);
//            return true;
//        }
//
//        while (item_count_mail > pProto->GetMaxStackSize())
//        {
//            items.push_back(ItemPair(pProto->ItemId, pProto->GetMaxStackSize()));
//            item_count_mail -= pProto->GetMaxStackSize();
//        }
//        items.push_back(ItemPair(pProto->ItemId, item_count_mail));
//
//        if (items.size() > MAX_MAIL_ITEMS)
//        {
//            PSendSysMessage(LANG_COMMAND_MAIL_ITEMS_LIMIT, MAX_MAIL_ITEMS);
//            SetSentErrorMessage(true);
//            return true;
//        }
//
//        if (!normalizePlayerName(plrName))
//        {
//            SendSysMessage(LANG_PLAYER_NOT_FOUND);
//            SetSentErrorMessage(true);
//            return true;
//        }
//
//        // from console show not existed sender
//        MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM);
//
//        uint32 itemTextId = !text.empty() ? sObjectMgr.CreateItemText(text) : 0;
//
//        Player *receiver = sObjectMgr.GetPlayerInWorld(playerGUID);
//
//        // fill mail
//        MailDraft draft(subject, itemTextId);
//
//        uint32 itemLowGuid = 0;
//        for (ItemPairs::const_iterator itr = items.begin(); itr != items.end(); ++itr)
//        {
//            if (Item* item = Item::CreateItem(itr->first, itr->second, m_session ? m_session->GetPlayer() : 0))
//            {
//                item->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted
//                draft.AddItem(item);
//                if (items.size() == 1) // only 1 item
//                    itemLowGuid = item->GetGUIDLow();
//            }
//        }
//
//        draft.SendMailTo(MailReceiver(receiver, ObjectGuid(playerGUID)), sender);
//
//        PSendSysMessage(LANG_MAIL_SENT, plrName.c_str());
//
//        uint32 fixedLootId = (*resultLog)[4].GetUInt32();
//
//        RealmDataDatabase.PExecute("UPDATE `loot_items_log` SET `receiverguid`='%u', `item`='%u', `count`='%u', `item_template`='%u', `instance_id`='%u', "
//            "`boss`='%u', `method`='6', `date`=NOW() WHERE `id`='%u'", 
//            GUID_LOPART(playerGUID), itemLowGuid, itemCount, pProto->ItemId, (*resultLog)[2].GetUInt32(), (*resultLog)[3].GetUInt32(), fixedLootId);
//    }
//    return true;
//}

void AwardTitleCallback(QueryResultAutoPtr result, uint32 gamemaster)
{
    Player* gm = sObjectAccessor.GetPlayerInWorldOrNot(gamemaster);
    if (!gm)
        return;

    if (!result)
    {
        ChatHandler(gm).SendSysMessage("Cannot find player in titles_to_award");
        return;
    }
    Field* fields = result->Fetch();
    Player* target = sObjectAccessor.GetPlayerInWorldOrNot(fields[0].GetUInt32());
    if (!target)
    {
        ChatHandler(gm).SendSysMessage("WTF, player not found, just went offline?");
        return;
    }
    
    uint64 titles = target->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES);
    titles |= fields[1].GetUInt64();
    target->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles);
    ChatHandler(gm).SendSysMessage("Player awarded");
}

bool ChatHandler::HandleModifyAwardTitleCommand(const char* args)
{
    return false;
    
    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    RealmDataDatabase.AsyncPQuery(&AwardTitleCallback, m_session->GetPlayer()->GetGUIDLow(),
        "SELECT guid, mask FROM titles_to_award WHERE guid = %u",chr->GetGUIDLow());
    SendSysMessage("Command accepted");
    return true;
}

//bool ChatHandler::HandleBerserk(const char * args)
//{
//    Player *p = getSelectedPlayer();
//    if (!p)
//    {
//        SendSysMessage(LANG_NO_CHAR_SELECTED);
//        SetSentErrorMessage(true);
//        return false;
//    }
//
//    if (!p->GetMap()->IsDungeon())
//    {
//        PSendSysMessage("Can be only used in instances.");
//        SetSentErrorMessage(true);
//        return false;
//    }
//
//    if (p->GetMap()->IsHeroicRaid())
//    {
//        PSendSysMessage("Can not be used in heroic raids.");
//        SetSentErrorMessage(true);
//        return false;
//    }
//
//    if (p->GetMap()->GetId() != 534 && p->GetMap()->GetId() != 564 && p->GetMap()->GetId() != 580)
//    {
//        PSendSysMessage("Can only be used in Sunwell / HS / BT.");
//        SetSentErrorMessage(true);
//        return false;
//    }
//    
//    uint32 duration = 60 * MINUTE * MILLISECONDS * 24;
//    std::string argstr = (char*)args;
//    if (argstr == "group")
//    {
//        Group *grp = p->GetGroup();
//        if (!grp)
//        {
//            PSendSysMessage("Player have no group.");
//            SetSentErrorMessage(true);
//            return false;
//        }
//
//        for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
//        {
//            Player *pl = itr->getSource();
//
//            if (!pl->GetMap()->IsDungeon())
//                continue;
//
//            if (pl->GetMap()->IsHeroicRaid())
//                continue;
//
//            pl->AddAura(41924, pl, duration);
//        }
//    }
//    else
//        p->AddAura(41924, p, duration);
//
//    return true;
//}