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

#include "TicketMgr.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Language.h"
#include "Player.h"
#include "Common.h"
#include "ObjectAccessor.h"
#include "Chat.h"

GM_Ticket* TicketMgr::GetGMTicketOpen(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = GM_TicketOpenList.begin(); i != GM_TicketOpenList.end();)
    {
        if ((*i)->guid == ticketGuid)
            return (*i);

        ++i;
    }
    return NULL;
}

GM_Ticket* TicketMgr::GetGMTicketClosed(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = GM_TicketClosedList.begin(); i != GM_TicketClosedList.end();)
    {
        if ((*i)->guid == ticketGuid)
            return (*i);

        ++i;
    }
    return NULL;
}

GM_Ticket* TicketMgr::GetGMTicketAny(uint64 ticketGuid)
{
    GM_Ticket* t = GetGMTicketOpen(ticketGuid);
    if (!t)
        t = GetGMTicketClosed(ticketGuid);

    return t;
}

GM_Ticket* TicketMgr::GetGMTicketOpenByPlayer(uint64 playerGuid)
{
    for (GmTicketList::iterator i = GM_TicketOpenList.begin(); i != GM_TicketOpenList.end();)
    {
        if ((*i)->playerGuid == playerGuid)
            return (*i);

        ++i;
    }
    return NULL;
}

GM_Ticket* TicketMgr::GetGMTicketOpenByName(const char* name)
{
    std::string pname = name;
    if (!normalizePlayerName(pname))
        return NULL;

    uint64 playerGuid = sObjectMgr.GetPlayerGUIDByName(pname.c_str());
    if (!playerGuid)
        return NULL;

    return GetGMTicketOpenByPlayer(playerGuid);
}

GmTicketList TicketMgr::GetGMTicketsAllByName(const char* name)
{
    std::string pname = name;

    GmTicketList tmpL;

    if (!normalizePlayerName(pname))
        return tmpL;

    uint64 playerGuid = sObjectMgr.GetPlayerGUIDByName(pname.c_str());
    if (!playerGuid)
        return tmpL;

    // first closed -> then open ones

    for (GmTicketList::iterator i = GM_TicketClosedList.begin(); i != GM_TicketClosedList.end(); ++i)
        if ((*i)->playerGuid == playerGuid)
            tmpL.push_back(*i);

    if (GM_Ticket* t = GetGMTicketOpenByPlayer(playerGuid))
        tmpL.push_back(t);

    return tmpL;
}

void TicketMgr::AddGMTicket(GM_Ticket *ticket, bool startup)
{
    ASSERT(ticket);
    if (ticket->closedBy)
    {
        if (m_ticketPlacesLeftCount)
            --m_ticketPlacesLeftCount; // have place
        else
        {
            delete GM_TicketClosedList.front();
            GM_TicketClosedList.pop_front(); // remove first
        }

        GM_TicketClosedList.push_back(ticket);

        // Remove all current denies (only from memory, they're already saved in DB if needed)
        for (DeniedTicketsMap::iterator i = m_deniedTickets.begin(); i != m_deniedTickets.end(); ++i)
            (*i).second.erase(ticket->guid);
        
        if (!startup)
        {
            if (Player* gm = (ticket->assignedToGM ? sObjectMgr.GetPlayerInWorld(ticket->assignedToGM) : NULL))
                sTicketMgr.ModifyGmBusy(gm->GetSession(), false);
        }
    }
    else
    {
        // ticket is not closed. Send sound to active GMs (there are no GMs active on startup anyway, dont check it)
        if (!m_gms.empty())
        {
            for (GmActivityMap::iterator i = m_gms.begin(); i != m_gms.end(); ++i)
            {
                GM_activity& act = (*i).second;
                if (act.busyCount)
                    continue;

                Player* gm = (*i).first->GetPlayer();
                if (!gm || !gm->IsInWorld())
                    continue;

                gm->SendPlaySound(12867, true);
            }
        }

        GM_TicketOpenList.push_back(ticket);
    }

    CheckUnassignedExist(); // this might be last unassigned ticket that was closed by a player, or a new ticket created by a player

    // save
    if (!startup)
        SaveGMTicket(ticket);
}

void TicketMgr::DeleteGMTicketPermanently(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = GM_TicketClosedList.begin(); i != GM_TicketClosedList.end();)
    {
        if ((*i)->guid == ticketGuid)
        {
            delete *i;
            GM_TicketClosedList.erase(i);
            ++m_ticketPlacesLeftCount;
            break;
        }
        else
            ++i;
    }

    // delete database record
    RealmDataDatabase.PExecute("DELETE FROM `gm_tickets` WHERE guid= '%llu'", ticketGuid);
}


void TicketMgr::LoadGMTickets()
{
    // Delete all out of object holder
    GM_TicketOpenList.clear();
    GM_TicketClosedList.clear();
    m_unassignedExist = false;
    m_ticketPlacesLeftCount = sWorld.getConfig(CONFIG_TICKET_CLOSED_COUNT);
                                                        //        0           1         2         3           4          5      6       7       8
    QueryResultAutoPtr resultClosed = RealmDataDatabase.PQuery("SELECT `guid`, `playerGuid`, `name`, `message`, `createtime`, `map`, `posX`, `posY`, `posZ`,"
                                                        //    9          10           11           12        13            14          15         16          17
                                                        "`updatetime`, `assigntime`, `closetime`, `closed`, `assignedto`, `itemGUID`, `approved`, `comment`, `response` FROM `gm_tickets` "
                                                        "WHERE closed != 0 ORDER BY createtime DESC LIMIT %u", m_ticketPlacesLeftCount);

    QueryResultAutoPtr resultOpen = RealmDataDatabase.Query("SELECT `guid`, `playerGuid`, `name`, `message`, `createtime`, `map`, `posX`, `posY`, `posZ`,"
                                                        //    9          10           11           12        13            14          15         16          17
                                                        "`updatetime`, `assigntime`, `closetime`, `closed`, `assignedto`, `itemGUID`, `approved`, `comment`, `response` FROM `gm_tickets` "
                                                        "WHERE closed = 0");

    GM_Ticket *ticket;

    if (!resultClosed && !resultOpen)
    {
        sTicketMgr.InitTicketID();
        sWorld.SendGMText(LANG_GM_TICKETS_TABLE_EMPTY);
        //sLog.outString(">> GM Tickets table is empty, no tickets were loaded.");
        return;
    }

    if (resultClosed)
    {
        GmTicketList t_closed;

        // Assign values from SQL to the object holder
        do
        {
            Field *fields = resultClosed->Fetch();
            ticket = new GM_Ticket;
            ticket->guid = fields[0].GetUInt64();
            ticket->playerGuid = fields[1].GetUInt64();
            ticket->name = fields[2].GetString();
            ticket->message = fields[3].GetString();
            ticket->createtime = fields[4].GetUInt64();
            ticket->map = fields[5].GetUInt32();
            ticket->pos_x = fields[6].GetFloat();
            ticket->pos_y = fields[7].GetFloat();
            ticket->pos_z = fields[8].GetFloat();
            ticket->updatetime = fields[9].GetUInt64();
            ticket->assigntime = fields[10].GetUInt64();
            ticket->closetime = fields[11].GetUInt64();
            ticket->closedBy = fields[12].GetUInt64();
            ticket->assignedToGM = fields[13].GetUInt64();
            ticket->itemGUID = fields[14].GetUInt32();
            ticket->approved = fields[15].GetBool();
            ticket->comment = fields[16].GetString();
            ticket->response = fields[17].GetString();

            t_closed.push_back(ticket);
        } while (resultClosed->NextRow());

        t_closed.reverse();
        for (GmTicketList::const_iterator i = t_closed.begin(); i != t_closed.end(); ++i)
            AddGMTicket(*i, true);
    }

    if (resultOpen)
    {
        // Assign values from SQL to the object holder
        do
        {
            Field *fields = resultOpen->Fetch();
            ticket = new GM_Ticket;
            ticket->guid = fields[0].GetUInt64();
            ticket->playerGuid = fields[1].GetUInt64();
            ticket->name = fields[2].GetString();
            ticket->message = fields[3].GetString();
            ticket->createtime = fields[4].GetUInt64();
            ticket->map = fields[5].GetUInt32();
            ticket->pos_x = fields[6].GetFloat();
            ticket->pos_y = fields[7].GetFloat();
            ticket->pos_z = fields[8].GetFloat();
            ticket->updatetime = fields[9].GetUInt64();
            ticket->assigntime = fields[10].GetUInt64();
            ticket->closetime = fields[11].GetUInt64();
            ticket->closedBy = fields[12].GetUInt64();
            ticket->assignedToGM = fields[13].GetUInt64();
            ticket->itemGUID = fields[14].GetUInt32();
            ticket->approved = fields[15].GetBool();
            ticket->comment = fields[16].GetString();
            ticket->response = fields[17].GetString();

            AddGMTicket(ticket, true);

        } while (resultOpen->NextRow());

        CheckUnassignedExist();
    }

    uint32 cnt = resultClosed ? resultClosed->GetRowCount() : 0;
    sWorld.SendGMText(LANG_COMMAND_TICKETRELOAD, cnt + (resultOpen ? resultOpen->GetRowCount() : 0));
}

void TicketMgr::RemoveGMTicket(uint64 ticketGuid, uint64 GMguid, uint64 itemGUID)
{
    GM_Ticket* t = GetGMTicketOpen(ticketGuid);
    if (t)
    {
        GM_TicketOpenList.remove(t);
        t->closedBy = GMguid;
        t->closetime = time(NULL);
        t->itemGUID = itemGUID;
        AddGMTicket(t, false);
    }
}


void TicketMgr::RemoveGMTicketByPlayer(uint64 playerGuid)
{
    GM_Ticket* t = GetGMTicketOpenByPlayer(playerGuid);
    if (t)
    {
        GM_TicketOpenList.remove(t);
        t->closedBy = playerGuid;
        t->closetime = time(NULL);
        AddGMTicket(t, false);
    }
}

void TicketMgr::SaveGMTicket(GM_Ticket* ticket)
{
    std::string msg = ticket->message, comment = ticket->comment, response = ticket->response;
    RealmDataDatabase.escape_string(msg);
    RealmDataDatabase.escape_string(comment);
    RealmDataDatabase.escape_string(response);

    static SqlStatementID replaceTicket;

    RealmDataDatabase.BeginTransaction();
    SqlStatement stmt = RealmDataDatabase.CreateStatement(replaceTicket, "REPLACE INTO `gm_tickets` (`guid`, `playerGuid`, `name`, `message`, `createtime`, `map`, `posX`, `posY`, `posZ`, `updatetime`, `assigntime`, `closetime`, `closed`, `assignedto`, `itemguid`, `approved`, `comment`, `response`) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    stmt.addUInt64(ticket->guid);
    stmt.addUInt64(ticket->playerGuid);
    stmt.addString(ticket->name);
    stmt.addString(msg);
    stmt.addUInt64(ticket->createtime);
    stmt.addUInt32(ticket->map);
    stmt.addFloat(ticket->pos_x);
    stmt.addFloat(ticket->pos_y);
    stmt.addFloat(ticket->pos_z);
    stmt.addUInt64(ticket->updatetime);
    stmt.addUInt64(ticket->assigntime);
    stmt.addUInt64(ticket->closetime);
    stmt.addUInt64(ticket->closedBy);
    stmt.addUInt64(ticket->assignedToGM);
    stmt.addUInt32(ticket->itemGUID);
    stmt.addUInt8(uint8(ticket->approved));
    stmt.addString(comment);
    stmt.addString(response);

    stmt.Execute();
    RealmDataDatabase.CommitTransaction();
}

void TicketMgr::UpdateGMTicket(GM_Ticket *ticket)
{
    SaveGMTicket(ticket);
}

void TicketMgr::InitTicketID()
{
    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT MAX(guid) FROM gm_tickets");
    if (result)
        m_ticketid = result->Fetch()[0].GetUInt64();
}

uint64 TicketMgr::GenerateTicketID()
{
    return ++m_ticketid;
}

GM_Ticket* TicketMgr::GetClosedTicketByItemGUID(uint32 itemGUID)
{
    for (GmTicketList::iterator i = GM_TicketClosedList.begin(); i != GM_TicketClosedList.end();)
    {
        if ((*i)->itemGUID == itemGUID)
            return (*i);

        ++i;
    }
    return NULL;
}

void TicketMgr::Update(uint32 diff)
{
    // no GMs online
    if (m_gms.empty())
        return;

    // no open unassigned tickets
    if (!m_unassignedExist)
        return;

    // at this point we have a ticket open and a gm presumably online (might be not ingame at the moment, so check on GetPlayer() && isinworld())
    for (GmActivityMap::iterator i = m_gms.begin(); i != m_gms.end(); ++i)
    {
        GM_activity& act = (*i).second;
        if (act.busyCount)
            continue;

        Player* gm = (*i).first->GetPlayer();
        if (!gm || !gm->IsInWorld())
            continue;

        // If not empty -> then some open tickets are denied, thus need to find out if there are any open non-denied tickets
        if (!m_deniedTickets[gm->GetGUID()].empty())
        {
            bool allDenied = true;
            std::set<uint64> &ticketDeniedGuids = m_deniedTickets[gm->GetGUID()];
            for (GmTicketList::iterator i = GM_TicketOpenList.begin(); i != GM_TicketOpenList.end(); ++i)
            {
                if (ticketDeniedGuids.find((*i)->guid) == ticketDeniedGuids.end())
                {
                    //not denied - break and continue with AFK time
                    allDenied = false;
                    break;
                }
            }

            if (allDenied) // all open tickets are denied by this GM, skip him
                continue;
        }

        if (act.responseDelay >= MAX_RESPONSE_DELAY)
        {
            act.totalAfkTime += diff;
            // save every minute
            if ((act.totalAfkTime / (MINUTE*MILLISECONDS)) != ((act.totalAfkTime - diff) / (MINUTE*MILLISECONDS)))
                SaveTotalAFK((*i).first);
        }

        // GM is not busy -> count the timer.
        act.responseDelay += diff;

        // every minute send message/sound to a gm if they need to answer an open ticket
        // save every 1 min after MAX_RESPONSE_DELAY mark with the offset of MAX_RESPONSE_DELAY.
        if ((act.responseDelay / (MINUTE*MILLISECONDS)) != ((act.responseDelay - diff) / (MINUTE*MILLISECONDS)))
        {
            gm->SendPlaySound(12867, true);
            ChatHandler(gm).PSendSysMessage("You haven't answered a ticket for %u minute(s).", (act.responseDelay / (MINUTE*MILLISECONDS)));
        }
    }
}
void TicketMgr::AddOrUpdateGm(WorldSession* s, uint64 plrGuid)
{
    // player must exist here, don't check for it
    uint32 busyCount = 0;
    for (GmTicketList::iterator i = GM_TicketOpenList.begin(); i != GM_TicketOpenList.end();)
    {
        if ((*i)->assignedToGM == plrGuid)
            ++busyCount;

        ++i;
    }

    GmActivityMap::iterator i = m_gms.find(s);
    if (i == m_gms.end()) // add new
        m_gms[s] = GM_activity{ 0, busyCount, 0 };
    else // update busy, but not time
        m_gms[s].busyCount = busyCount;

    ChatHandler(s).PSendSysMessage("Your busy count now is %u.", busyCount);
};

void TicketMgr::RemoveGm(WorldSession* s) 
{ 
    SaveTotalAFK(s);
    // do a save with login id here
    m_gms.erase(s); 
};

void TicketMgr::ModifyGmBusy(WorldSession* s, bool busy)
{
    GmActivityMap::iterator i = m_gms.find(s);
    if (i != m_gms.end())
    {
        if ((*i).second.busyCount == 0 && !busy)
        {
            ChatHandler(s).SendSysMessage("Error: You were not busy.");
            return; // well, err, already non-busy
        }

        (*i).second.busyCount += busy ? 1 : -1;
        if (busy)
            (*i).second.responseDelay = 0; // reset the delay

        ChatHandler(s).PSendSysMessage("Your busy count now is %u.", (*i).second.busyCount);
    }
    else
        ChatHandler(s).SendSysMessage("Error: GM not found.");
}

void TicketMgr::SaveTotalAFK(WorldSession* s)
{
    uint32 totalAfk = (m_gms.find(s) != m_gms.end() ? m_gms[s].totalAfkTime : 0) / MILLISECONDS; // save in seconds
    if (!totalAfk) // not found or is 0 -> don't save
        return;

    static SqlStatementID replaceTotalAFK;

    SqlStatement stmt = AccountsDatabase.CreateStatement(replaceTotalAFK, "REPLACE INTO `gm_idle` (`account_id`, `login_id`, `afk_time`) VALUES (?, ?, ?)");
    stmt.addUInt32(s->GetAccountId());
    stmt.addUInt32(s->GetLoginId());
    stmt.addUInt32(totalAfk);
    stmt.Execute();
}

void TicketMgr::CheckUnassignedExist()
{
    for (GmTicketList::iterator i = GM_TicketOpenList.begin(); i != GM_TicketOpenList.end(); ++i)
    {
        if (!(*i)->assignedToGM)
        {
            m_unassignedExist = true;
            return;
        }
    }

    m_unassignedExist = false;
}

uint32 TicketMgr::GetBestGmStatusMessage()
{
    if (m_gms.empty())
        return LANG_TICKET_CREATE_NO_GM;

    for (GmActivityMap::iterator i = m_gms.begin(); i != m_gms.end(); ++i)
    {
        GM_activity& act = (*i).second;
        if (act.busyCount)
            continue;

        Player* gm = (*i).first->GetPlayer();
        if (!gm || !gm->IsInWorld())
            continue;

        // afk, count as busy
        if (act.responseDelay >= MAX_RESPONSE_DELAY)
            continue;

        return LANG_TICKET_CREATE_GM_FREE;
    }
    
    // There are GMs, but everyone is busy/afk
    return LANG_TICKET_CREATE_GM_BUSY;
}

void TicketMgr::AddGMTicketDeny(uint64 denierGuid, GM_Ticket* t)
{
    if (IsGMTicketDenied(denierGuid, t))
        return;

    m_deniedTickets[denierGuid].insert(t->guid);

    static SqlStatementID addGMTicketDeny;

    RealmDataDatabase.BeginTransaction();
    SqlStatement stmt = RealmDataDatabase.CreateStatement(addGMTicketDeny, "REPLACE INTO `gm_tickets_denied` (`ticket`, `gm`) VALUES (?, ?)");
    stmt.addUInt64(t->guid);
    stmt.addUInt64(denierGuid);

    stmt.Execute();
    RealmDataDatabase.CommitTransaction();
}

void TicketMgr::RemoveGMTicketDeny(uint64 denierGuid, GM_Ticket* t)
{
    if (!IsGMTicketDenied(denierGuid, t))
        return;

    m_deniedTickets[denierGuid].erase(t->guid);

    static SqlStatementID removeGMTicketDeny;

    RealmDataDatabase.BeginTransaction();
    SqlStatement stmt = RealmDataDatabase.CreateStatement(removeGMTicketDeny, "DELETE FROM `gm_tickets_denied` WHERE `ticket`=? AND `gm`=?");
    stmt.addUInt64(t->guid);
    stmt.addUInt64(denierGuid);

    stmt.Execute();
    RealmDataDatabase.CommitTransaction();
}