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

#include "Language.h"
#include "WorldPacket.h"
#include "Common.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "TicketMgr.h"
#include "World.h"
#include "Chat.h"

void WorldSession::HandleGMTicketCreateOpcode(WorldPacket & recv_data)
{
    // ticket system is outdated, we use Discord instead
    ChatHandler(_player).PSendSysMessage(LANG_TICKETS_DISABLED);
    return;

    // always do a packet check
    CHECK_PACKET_SIZE(recv_data, 4*4+1+2*4);

    uint32 map;
    float x, y, z;
    std::string ticketText = "";
    std::string ticketText2 = "";
    GM_Ticket *ticket = new GM_Ticket;

    WorldPacket data(SMSG_GMTICKET_CREATE, 4);

    // recv Data
    //TODO: Add map coordinates to tickets.
    recv_data >> map;
    recv_data >> x;
    recv_data >> y;
    recv_data >> z;
    recv_data >> ticketText;

    // get additional data, rarely used
    recv_data >> ticketText2;

    // assign values
    ticket->name = GetPlayer()->GetName();
    ticket->guid = sTicketMgr.GenerateTicketID();
    ticket->playerGuid = GetPlayer()->GetGUID();
    ticket->message = ticketText;
    ticket->createtime = time(NULL);
    ticket->map = map;
    ticket->pos_x = x;
    ticket->pos_y = y;
    ticket->pos_z = z;
    ticket->updatetime = 0;
    ticket->assigntime = 0;
    ticket->closetime = 0;
    ticket->closedBy = 0;
    ticket->assignedToGM = 0;
    ticket->itemGUID = 0;
    ticket->approved = false;
    ticket->comment = "";

    // remove ticket by player, shouldn't happen
    sTicketMgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());

    // add ticket
    sTicketMgr.AddGMTicket(ticket, false);

    // Response - no errors
    data << uint32(2);

    // Send ticket creation
    SendPacket(&data);

    sWorld.SendGMText(LANG_COMMAND_TICKETNEW, ChatHandler::GetNameLink(ticket->name).c_str(), ticket->guid);

    ChatHandler(this).SendSysMessage(sTicketMgr.GetBestGmStatusMessage());
}

void WorldSession::HandleGMTicketUpdateOpcode(WorldPacket & recv_data)
{
    // always do a packet check
    CHECK_PACKET_SIZE(recv_data,1);

    std::string message = "";
    time_t t = time(NULL);

    WorldPacket data(SMSG_GMTICKET_UPDATETEXT, 4);

    // recv Data
    recv_data >> message;

    // Update Ticket
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpenByPlayer(GetPlayer()->GetGUID());

    // Check if player has a GM Ticket yet
    if (!ticket || ticket->assignedToGM != 0)
    {
        // Response - error couldnt find existing Ticket
        data << uint32(1);

        // Send packet
        SendPacket(&data);
        return;
    }

    ticket->message = message;
    ticket->updatetime = t;

    sTicketMgr.UpdateGMTicket(ticket);

    // Response - no errors
    data << uint32(2);

    // Send packet
    SendPacket(&data);

    sWorld.SendGMText(LANG_COMMAND_TICKETUPDATED, GetPlayer()->GetName(), ticket->guid);

}

void WorldSession::HandleGMTicketDeleteOpcode(WorldPacket & /*recv_data*/)
{
    // NO recv_data, NO packet check size

    GM_Ticket* ticket = sTicketMgr.GetGMTicketOpenByPlayer(GetPlayer()->GetGUID());

    // CHeck for Ticket
    if (ticket && ticket->assignedToGM == 0)
    {
        // Remove Tickets from Player

        // Response - no errors
        WorldPacket data(SMSG_GMTICKET_DELETETICKET, 4);
        data << uint32(9);
        // Send Packet
        SendPacket(&data);

        sWorld.SendGMText(LANG_COMMAND_TICKETPLAYERABANDON, GetPlayer()->GetName(), ticket->guid);
        sTicketMgr.RemoveGMTicketByPlayer(GetPlayer()->GetGUID());
    }
}

void WorldSession::HandleGMTicketGetTicketOpcode(WorldPacket & /*recv_data*/)
{
    // NO recv_data NO packet size check

    WorldPacket data(SMSG_GMTICKET_GETTICKET, 400);

    // get Current Ticket
    GM_Ticket *ticket = sTicketMgr.GetGMTicketOpenByPlayer(GetPlayer()->GetGUID());

    // check for existing ticket
    if (!ticket)
    {
        data << uint32(10);
        // send packet
        SendPacket(&data);
        return;
    }

    // Send current Ticket
    data << uint32(6); // unk ?
    data << ticket->message.c_str();

    SendPacket(&data);

}

void WorldSession::HandleGMTicketSystemStatusOpcode(WorldPacket & /*recv_data*/)
{
    // NO recv_data NO packet size check

    WorldPacket data(SMSG_GMTICKET_SYSTEMSTATUS, 4);

    // Response - System is working Fine

    // No need for checks, ticket system is active
    // in case of disactivity, this should be set to (0)

    data << uint32(1);


    // Send Packet
    SendPacket(&data);
}
