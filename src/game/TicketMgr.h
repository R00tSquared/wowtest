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

#ifndef HELLGROUND_TICKETMGR_H
#define HELLGROUND_TICKETMGR_H

#include "ace/Singleton.h"

#include "Database/DatabaseEnv.h"
#include "Util.h"
#include <map>
#include "WorldSession.h"

#define MAX_RESPONSE_DELAY 3*MINUTE*MILLISECONDS

struct GM_Ticket
{
    uint64 guid;
    uint64 playerGuid;
    std::string name;
    float pos_x;
    float pos_y;
    float pos_z;
    uint32 map;
    std::string message;
    uint64 createtime;
    uint64 updatetime;
    uint64 assigntime;
    uint64 closetime;
    uint64 closedBy;
    uint64 assignedToGM;
    uint32 itemGUID;
    bool approved;
    std::string comment;
    std::string response;
};

// Map Typedef
typedef std::list<GM_Ticket*> GmTicketList;

struct GM_activity
{
    uint32 responseDelay;
    uint32 busyCount;
    uint32 totalAfkTime;
};

typedef std::map<WorldSession*, GM_activity> GmActivityMap;

/*uint64 denier guid, set of denied ticket guids*/
typedef std::map<uint64, std::set<uint64>> DeniedTicketsMap;

class TicketMgr
{
    friend class ACE_Singleton<TicketMgr, ACE_Null_Mutex>;
    TicketMgr(){ InitTicketID();}    //constructor

    public:
        ~TicketMgr(){}  //destructor

        // Object Holder
        GmTicketList         GM_TicketOpenList;
        GmTicketList         GM_TicketClosedList;

        /*Save into DB is called if no startup*/
        void AddGMTicket(GM_Ticket *ticket, bool startup);
        /*Can only be called for closed tickets*/
        void DeleteGMTicketPermanently(uint64 ticketGuid);
        void LoadGMTickets();
        void RemoveGMTicketByPlayer(uint64 playerGuid); // when player closes ticket
        void RemoveGMTicket(uint64 ticketGuid, uint64 GMguid, uint64 itemGUID); // when GM closes ticket
        void UpdateGMTicket(GM_Ticket *ticket);
        void SaveGMTicket(GM_Ticket* ticket);

        uint64 GenerateTicketID();
        void InitTicketID();
        GM_Ticket* GetGMTicketOpen(uint64 ticketGuid);
        GM_Ticket* GetGMTicketClosed(uint64 ticketGuid);
        GM_Ticket* GetGMTicketAny(uint64 ticketGuid);
        GM_Ticket* GetGMTicketOpenByPlayer(uint64 playerGuid);
        GM_Ticket* GetGMTicketOpenByName(const char *name);
        GmTicketList GetGMTicketsAllByName(const char *name);
        GM_Ticket* GetClosedTicketByItemGUID(uint32 itemGUID);

        void Update(uint32 diff);
        /*plrGuid is used, cause at LoadFromDB there isn't player on the session yet*/
        void AddOrUpdateGm(WorldSession* s, uint64 plrGuid);
        void RemoveGm(WorldSession* s);
        GmActivityMap const & GetGms() { return m_gms; };
        void ModifyGmBusy(WorldSession* s, bool busy);
        void CheckUnassignedExist();
        uint32 GetBestGmStatusMessage();

        bool IsGMTicketDenied(uint64 denierGuid, GM_Ticket* t) { return m_deniedTickets[denierGuid].find(t->guid) != m_deniedTickets[denierGuid].end(); };
        /*Receives denier GUID and an open ticket as t*/
        void AddGMTicketDeny(uint64 denierGuid, GM_Ticket* t);
        /*Receives denier GUID and an open ticket as t*/
        void RemoveGMTicketDeny(uint64 denierGuid, GM_Ticket* t);

    private: 
        void SaveTotalAFK(WorldSession* s);

    private:
        uint64 m_ticketid;
        uint32 m_ticketPlacesLeftCount;
        GmActivityMap m_gms;
        bool m_unassignedExist;
        DeniedTicketsMap m_deniedTickets;
};

#define sTicketMgr (*ACE_Singleton<TicketMgr, ACE_Null_Mutex>::instance())
#endif
