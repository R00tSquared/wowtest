/*
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

#ifndef HELLGROUND_OUTDOOR_PVP_MGR_H
#define HELLGROUND_OUTDOOR_PVP_MGR_H

#include "ace/Singleton.h"
#include "OutdoorPvP.h"

class Player;
class GameObject;
class Creature;
class ZoneScript;
struct GossipOption;

#define OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL 1000

// class to handle player enter / leave / areatrigger / GO use events
class OutdoorPvPMgr
{
    friend class ACE_Singleton<OutdoorPvPMgr, ACE_Null_Mutex>;
    OutdoorPvPMgr();

    public:
        // dtor
        ~OutdoorPvPMgr();

        // create outdoor pvp events
        void InitOutdoorPvP();
        // called when a player enters an outdoor pvp area
        void HandlePlayerEnterZone(Player * plr, uint32 areaflag);
        // called when player leaves an outdoor pvp area
        void HandlePlayerLeaveZone(Player * plr, uint32 areaflag);
        void HandlePlayerLeave(Player *plr);
        // return assigned outdoor pvp
        OutdoorPvP * GetOutdoorPvPToZoneId(uint32 zoneid);
        // handle custom (non-exist in dbc) spell if registered
        bool HandleCustomSpell(Player * plr, uint32 spellId, GameObject* go);
        // handle custom go if registered
        bool HandleOpenGo(Player * plr, uint64 guid);

        ZoneScript * GetZoneScript(uint32 zoneId);

        void AddZone(uint32 zoneid, OutdoorPvP * handle);

        void Update(uint32 diff);

        void HandleGossipOption(Player * player, uint64 guid, uint32 gossipid);

        bool CanTalkTo(Player * player, Creature * creature, GossipOption & gso);

        void HandleDropFlag(Player * plr, uint32 spellId);

        typedef std::vector<OutdoorPvP*> OutdoorPvPSet;
        typedef std::map<uint32 /* zoneid */, OutdoorPvP*> OutdoorPvPMap;

    private:
        // contains all initiated outdoor pvp events
        // used when initing / cleaning up
        OutdoorPvPSet  m_OutdoorPvPSet;
        // maps the zone ids to an outdoor pvp event
        // used in player event handling
        OutdoorPvPMap   m_OutdoorPvPMap;
        // update interval
        uint32 m_UpdateTimer;
};

#define sOutdoorPvPMgr (*ACE_Singleton<OutdoorPvPMgr, ACE_Null_Mutex>::instance())
#endif /*OUTDOOR_PVP_MGR_H_*/
