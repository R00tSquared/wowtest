// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

#include "OutdoorPvPMgr.h"
#include "OutdoorPvPHP.h"
#include "OutdoorPvPNA.h"
#include "OutdoorPvPTF.h"
#include "OutdoorPvPZM.h"
#include "OutdoorPvPSI.h"
#include "OutdoorPvPEP.h"
#include "Player.h"

#include "MapManager.h"
#include "ObjectMgr.h"

OutdoorPvPMgr::OutdoorPvPMgr()
{
    m_UpdateTimer = 0;
    //sLog.outDebug("Instantiating OutdoorPvPMgr");
}

OutdoorPvPMgr::~OutdoorPvPMgr()
{
    //sLog.outDebug("Deleting OutdoorPvPMgr");
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
        delete *itr;
}

#define MAP_EASTERN_KINGDOM 0
#define MAP_KALIMDOR 1
#define MAP_OUTLAND 530

void OutdoorPvPMgr::InitOutdoorPvP()
{
    Map* HP_map = sMapMgr.FindMap(MAP_OUTLAND, sObjectMgr.GetSingleInstance(MAP_OUTLAND, -211.0f, 4278.0f));
    Map* NA_map = sMapMgr.FindMap(MAP_OUTLAND, sObjectMgr.GetSingleInstance(MAP_OUTLAND, -1145.0f, 8182.0f));
    Map* TF_map = sMapMgr.FindMap(MAP_OUTLAND, sObjectMgr.GetSingleInstance(MAP_OUTLAND, -2000.0f, 4451.0f ));
    Map* ZM_map = sMapMgr.FindMap(MAP_OUTLAND, sObjectMgr.GetSingleInstance(MAP_OUTLAND, -54.0f, 5813.0f));
    Map* EP_map = sMapMgr.FindMap(MAP_EASTERN_KINGDOM, sObjectMgr.GetSingleInstance(MAP_EASTERN_KINGDOM, 2300.0f, -4613.0f));
    Map* SI_map = sMapMgr.FindMap(MAP_KALIMDOR, sObjectMgr.GetSingleInstance(MAP_KALIMDOR, -7426.0f, 1005.0f));

    // create new opvp
    OutdoorPvP * pOP = new OutdoorPvPHP;
    // respawn, init variables
    if (!pOP->SetupOutdoorPvP() || !HP_map)
    {
        sLog.outLog(LOG_CRASH, "OutdoorPvP : HP init failed.");
        delete pOP;
    }
    else
    {
        pOP->SetMap(HP_map);

        m_OutdoorPvPSet.push_back(pOP);
        sLog.outDebug("OutdoorPvP : HP successfully initiated.");
    }

    pOP = new OutdoorPvPNA;
    // respawn, init variables
    if (!pOP->SetupOutdoorPvP() || !NA_map)
    {
        sLog.outLog(LOG_CRASH, "OutdoorPvP : NA init failed.");
        delete pOP;
    }
    else
    {
        pOP->SetMap(NA_map);

        m_OutdoorPvPSet.push_back(pOP);
        sLog.outDebug("OutdoorPvP : NA successfully initiated.");
    }

    pOP = new OutdoorPvPTF;
    // respawn, init variables
    if (!pOP->SetupOutdoorPvP() || !TF_map)
    {
        sLog.outLog(LOG_CRASH, "OutdoorPvP : TF init failed.");
        delete pOP;
    }
    else
    {
        pOP->SetMap(TF_map);

        m_OutdoorPvPSet.push_back(pOP);
        sLog.outDebug("OutdoorPvP : TF successfully initiated.");
    }

    pOP = new OutdoorPvPZM;
    // respawn, init variables
    if (!pOP->SetupOutdoorPvP() || !ZM_map)
    {
        sLog.outLog(LOG_CRASH, "OutdoorPvP : ZM init failed.");
        delete pOP;
    }
    else
    {
        pOP->SetMap(ZM_map);

        m_OutdoorPvPSet.push_back(pOP);
        sLog.outDebug("OutdoorPvP : ZM successfully initiated.");
    }

    pOP = new OutdoorPvPSI;
    // respawn, init variables
    if (!pOP->SetupOutdoorPvP() || !SI_map)
    {
        sLog.outLog(LOG_CRASH, "OutdoorPvP : SI init failed.");
        delete pOP;
    }
    else
    {
        pOP->SetMap(SI_map);

        m_OutdoorPvPSet.push_back(pOP);
        sLog.outDebug("OutdoorPvP : SI successfully initiated.");
    }

    pOP = new OutdoorPvPEP;
    // respawn, init variables
    if (!pOP->SetupOutdoorPvP() || !EP_map)
    {
        sLog.outLog(LOG_CRASH, "OutdoorPvP : EP init failed.");
        delete pOP;
    }
    else
    {
        pOP->SetMap(EP_map);

        m_OutdoorPvPSet.push_back(pOP);
        sLog.outDebug("OutdoorPvP : EP successfully initiated.");
    }
}

void OutdoorPvPMgr::AddZone(uint32 zoneid, OutdoorPvP *handle)
{
    m_OutdoorPvPMap[zoneid] = handle;
}

void OutdoorPvPMgr::HandlePlayerEnterZone(Player *plr, uint32 zoneid)
{
    // always send init world states at zone change, not just for pvp zones
    plr->SendInitWorldStates();

    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneid);
    if (itr == m_OutdoorPvPMap.end())
        return;

    if (itr->second->HasPlayer(plr))
        return;

    itr->second->HandlePlayerEnterZone(plr, zoneid);
    sLog.outDebug("Player %u entered outdoorpvp id %u", plr->GetGUIDLow(), itr->second->GetTypeId());
}

void OutdoorPvPMgr::HandlePlayerLeaveZone(Player *plr, uint32 zoneid)
{
    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneid);
    if (itr == m_OutdoorPvPMap.end())
        return;

    // teleport: remove once in removefromworld, once in updatezone
    if (!itr->second->HasPlayer(plr))
        return;

    itr->second->HandlePlayerLeaveZone(plr, zoneid);
    sLog.outDebug("Player %u left outdoorpvp id %u",plr->GetGUIDLow(), itr->second->GetTypeId());
}

void OutdoorPvPMgr::HandlePlayerLeave(Player *plr)
{
    for (OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.begin(); itr != m_OutdoorPvPMap.end(); ++itr)
    {
        // teleport: remove once in removefromworld, once in updatezone
        if (!itr->second->HasPlayer(plr))
            continue;
        else
        {
            itr->second->HandlePlayerLeaveZone(plr, itr->first);
            sLog.outDebug("Player %u left outdoorpvp id %u",plr->GetGUIDLow(), itr->second->GetTypeId());
        }
    }
}

OutdoorPvP * OutdoorPvPMgr::GetOutdoorPvPToZoneId(uint32 zoneid)
{
    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneid);
    if (itr == m_OutdoorPvPMap.end())
    {
        // no handle for this zone, return
        return NULL;
    }
    return itr->second;
}

void OutdoorPvPMgr::Update(uint32 diff)
{
    m_UpdateTimer += diff;
    if (m_UpdateTimer > OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL)
    {
        for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
            (*itr)->Update(m_UpdateTimer);
        m_UpdateTimer = 0;
    }
}

bool OutdoorPvPMgr::HandleCustomSpell(Player *plr, uint32 spellId, GameObject * go)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleCustomSpell(plr,spellId,go))
            return true;
    }
    return false;
}

ZoneScript * OutdoorPvPMgr::GetZoneScript(uint32 zoneId)
{
    OutdoorPvPMap::iterator itr = m_OutdoorPvPMap.find(zoneId);
    if (itr != m_OutdoorPvPMap.end())
        return itr->second;
    else
        return NULL;
}

bool OutdoorPvPMgr::HandleOpenGo(Player *plr, uint64 guid)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleOpenGo(plr,guid))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleGossipOption(Player *plr, uint64 guid, uint32 gossipid)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleGossipOption(plr,guid,gossipid))
            return;
    }
}

bool OutdoorPvPMgr::CanTalkTo(Player * plr, Creature * c, GossipOption & gso)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->CanTalkTo(plr,c,gso))
            return true;
    }
    return false;
}

void OutdoorPvPMgr::HandleDropFlag(Player *plr, uint32 spellId)
{
    for (OutdoorPvPSet::iterator itr = m_OutdoorPvPSet.begin(); itr != m_OutdoorPvPSet.end(); ++itr)
    {
        if ((*itr)->HandleDropFlag(plr,spellId))
            return;
    }
}
