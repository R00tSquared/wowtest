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
#include <regex>

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Item.h"
#include "GameObject.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Guild.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "Language.h"
#include "World.h"
#include "GameEvent.h"
#include "SpellMgr.h"
#include "PoolManager.h"
#include "AccountMgr.h"
#include "WaypointMgr.h"
#include "Util.h"
#include <cctype>
#include <iostream>
#include <fstream>
#include <map>
#include "TicketMgr.h"
#include "CreatureAI.h"
#include "ChannelMgr.h"
#include "GuildMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

#include "TargetedMovementGenerator.h"                      // for HandleNpcUnFollowCommand
#include "MoveMap.h"                                        // for mmap manager
#include "PathFinder.h"                                     // for mmap commands

static uint32 ReputationRankStrIndex[MAX_REPUTATION_RANK] =
{
    LANG_REP_HATED,    LANG_REP_HOSTILE, LANG_REP_UNFRIENDLY, LANG_REP_NEUTRAL,
    LANG_REP_FRIENDLY, LANG_REP_HONORED, LANG_REP_REVERED,    LANG_REP_EXALTED
};

//mute player for some times
bool ChatHandler::HandleMuteCommand(const char* args)
{
    if (!*args)
        return false;
    
    char *charname = strtok((char*)args, " ");
    if (!charname)
        return false;

    std::string cname = charname;

    char *timetonotspeak = strtok(NULL, " ");
    if (!timetonotspeak || !atoi(timetonotspeak))
        return false;

    char *mutereason = strtok(NULL, "");
    if (!mutereason)
    {
        SendSysMessage("Reason must be set");
        SetSentErrorMessage(true);
        return false;
    }

    std::string mutereasonstr = mutereason;

    //if (!std::regex_search(mutereasonstr, std::regex("^\\d+\\.\\d+ \\(\\d+\\)$")))
    //{
    //    SendSysMessage("Specify violated rule and the number of repetitions. Example: 1.4 (1), 2.4 (22), etc...");
    //    SetSentErrorMessage(true);
    //    return false;
    //}

    // check if there is specificator. there should be none
    char* testSp = timetonotspeak;
    while (*testSp/*not end of the line*/)
    {
        if (!isdigit(*testSp))
        {
            SendSysMessage("Do not use specificator. Mute commands take minutes as arguements.");
            SetSentErrorMessage(true);
            return false;
        }

        ++testSp;
    }

    uint32 notspeaktime = (uint32) atoi(timetonotspeak);

    if (notspeaktime == 0)
        return false;

    if (!normalizePlayerName(cname))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = sObjectMgr.GetPlayerGUIDByName(cname.c_str());
    if (!guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = sObjectAccessor.GetPlayerInWorldOrNot(guid);
    if (chr && !chr->IsInWorld())
    {
        SendSysMessage(LANG_PLAYER_LOADING_WAIT);
        return true;
    }

    // check security
    uint32 account_id = 0;
    uint32 permissions = 0;

    if (chr)
    {
        account_id = chr->GetSession()->GetAccountId();
        permissions = chr->GetSession()->GetPermissions();
    }
    else
    {
        account_id = sObjectMgr.GetPlayerAccountIdByGUID(guid);
        permissions = AccountMgr::GetPermissions(account_id);
    }

    if (m_session && permissions >= m_session->GetPermissions())
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return false;
    }

    AccountsDatabase.escape_string(mutereasonstr);

    WorldSession* targSess = chr ? chr->GetSession() : sWorld.FindSession(account_id);
    // notspeaktime - minutes
    if (targSess)
    {
        if (targSess->m_muteRemain > notspeaktime * 60000) // cannot set mutetime lower than there already is.
                                                           // When selecting from DB at login it ALWAYS selects the highest mutetime
        {
            SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage(true);
            return false;
        }
        targSess->m_muteRemain = notspeaktime * 60000; // m_muteRemain - miliseconds
        targSess->m_muteReason = mutereasonstr;
        if (chr)
            ChatHandler(chr).PSendSysMessage(LANG_YOUR_CHAT_DISABLED, notspeaktime, mutereasonstr.c_str());
    }

    std::string author;

    if (m_session)
        author = m_session->GetPlayerName();
    else
        author = "[CONSOLE]";

    AccountsDatabase.escape_string(author);

    AccountsDatabase.PExecute("INSERT IGNORE INTO account_punishment VALUES ('%u', '%u', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, '%s', '%s', '1', '%u')",
                              account_id, PUNISHMENT_MUTE, notspeaktime*60/*seconds*/, author.c_str(), mutereasonstr.c_str(), uint32(notspeaktime*60)/*seconds*/);

    // todo
    if (!m_session->HasPermissions(PERM_ADM) && sWorld.getConfig(CONFIG_SHOW_KICK_IN_WORLD) == 1)
    {
        sWorld.SendWorldText(m_session->GetPermissions() == 3 ? LANG_ANNOUNCE_MUTE_EM : LANG_ANNOUNCE_MUTE_GM,
            0, author.c_str(), cname.c_str(), timetonotspeak, mutereasonstr.c_str());
    }

    SendGlobalGMSysMessage(LANG_GM_DISABLE_CHAT, author.c_str(), cname.c_str(), notspeaktime, mutereasonstr.c_str());

    return true;
}

//unmute player
bool ChatHandler::HandleUnmuteCommand(const char* args)
{
    if (!*args)
        return false;

    char *charname = strtok((char*)args, " ");
    if (!charname)
        return false;

    std::string cname = charname;

    if (!normalizePlayerName(cname))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = sObjectMgr.GetPlayerGUIDByName(cname.c_str());
    if (!guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = sObjectAccessor.GetPlayerInWorldOrNot(guid);
    if (chr && !chr->IsInWorld())
    {
        SendSysMessage(LANG_PLAYER_LOADING_WAIT);
        return true;
    }

    // check security
    uint32 account_id = 0;
    uint32 permissions = 0;

    if (chr)
    {
        account_id = chr->GetSession()->GetAccountId();
        permissions = chr->GetSession()->GetPermissions();
    }
    else
    {
        account_id = sObjectMgr.GetPlayerAccountIdByGUID(guid);
        permissions = AccountMgr::GetPermissions(account_id);
    }

    //if (m_session && permissions >= m_session->GetPermissions())
    //{
    //    SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
    //    SetSentErrorMessage(true);
    //    return false;
    //}

    if (chr)
    {
        if (chr->CanSpeak() && !chr->IsTrollmuted())
        {
            SendSysMessage(LANG_CHAT_ALREADY_ENABLED);
            SetSentErrorMessage(true);
            return false;
        }

        chr->GetSession()->m_trollMuteRemain = 0;
        chr->GetSession()->m_trollmuteReason = "";
        chr->GetSession()->m_muteRemain = 0;
        chr->GetSession()->m_muteReason = "";
        ChatHandler(chr).SendSysMessage(LANG_YOUR_CHAT_ENABLED);
    }

    AccountsDatabase.PExecute("UPDATE account_punishment SET active ='0', muteRemain = '0' WHERE account_id = '%u' AND punishment_type_id IN ('%u','%u')", account_id, PUNISHMENT_MUTE, PUNISHMENT_TROLLMUTE);

    std::string author;

    if (m_session)
        author = m_session->GetPlayerName();
    else
        author = "[CONSOLE]";

    SendGlobalGMSysMessage(LANG_GM_ENABLE_CHAT, author.c_str(), cname.c_str());

    return true;
}

bool ChatHandler::HandleMuteInfoCommand(const char* args)
{
    if (!args)
        return false;

    char* cname = strtok ((char*)args, "");
    if (!cname)
        return false;

    std::string name = cname;
    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sObjectMgr.GetPlayerAccountIdByPlayerName(name);
    if (!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if (!AccountMgr::GetName(accountid,accountname))
    {
        SendSysMessage(LANG_MUTEINFO_NOCHARACTER);
        return true;
    }

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT FROM_UNIXTIME(punishment_date), expiration_date-punishment_date, expiration_date, reason, punished_by, active, muteRemain, punishment_date "
                                                        "FROM account_punishment "
                                                        "WHERE account_id = '%u' AND punishment_type_id = '%u' "
                                                        "ORDER BY punishment_date ASC", accountid, PUNISHMENT_MUTE);

    if (!result)
    {
        PSendSysMessage(LANG_MUTEINFO_NOACCOUNTMUTE, accountname.c_str());
        return true;
    }

    PSendSysMessage(LANG_MUTEINFO_MUTEHISTORY, accountname.c_str());
    do
    {
        Field* fields = result->Fetch();

        time_t unmutedate = time_t(fields[2].GetUInt64());
        uint64 muteLength = fields[1].GetUInt64();

        bool active = false;
        if ((muteLength == 0 || unmutedate >= time(NULL) || fields[6].GetUInt32() != 0) && fields[5].GetBool())
            active = true;

        uint32 date_id = fields[7].GetUInt32();

        std::string mutetime = WorldSession::secondsToTimeString(m_session, muteLength, true);
        PSendSysMessage(LANG_MUTEINFO_HISTORYENTRY,
            date_id, fields[0].GetString(), mutetime.c_str(), active ? GetHellgroundString(LANG_YES) : GetHellgroundString(LANG_JUST_NO), fields[3].GetString(), fields[4].GetString());
        PSendSysMessage("This mute left duration: %s", (WorldSession::secondsToTimeString(m_session, fields[6].GetUInt32(), true)).c_str());
    }
    while (result->NextRow());

    return true;
}

bool ChatHandler::HandleGameObjectTargetCommand(const char* args)
{
    Player* pl = m_session->GetPlayer();
    QueryResultAutoPtr result;
    GameEventMgr::ActiveEvents const& activeEventsList = sGameEventMgr.GetActiveEventList();
    if (*args)
    {
        // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
        char* cId = extractKeyFromLink((char*)args, "Hgameobject_entry");
        if(!cId)
            return false;

        uint32 id = atol(cId);

        if (id)
            result = GameDataDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, orientation, map, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE map = '%i' AND id = '%u' ORDER BY order_ ASC LIMIT 1",
                pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(),id);
        else
        {
            std::string name = cId;
            GameDataDatabase.escape_string(name);
            result = GameDataDatabase.PQuery(
                "SELECT guid, id, position_x, position_y, position_z, orientation, map, (POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ "
                "FROM gameobject,gameobject_template WHERE gameobject_template.entry = gameobject.id AND map = %i AND name " _LIKE_ " " _CONCAT3_("'%%'","'%s'","'%%'") " ORDER BY order_ ASC LIMIT 1",
                pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetMapId(),name.c_str());
        }
    }
    else
    {
        std::ostringstream eventFilter;
        eventFilter << " AND (event IS NULL ";
        bool initString = true;

        for (GameEventMgr::ActiveEvents::const_iterator itr = activeEventsList.begin(); itr != activeEventsList.end(); ++itr)
        {
            if (initString)
            {
                eventFilter  <<  "OR event IN (" <<*itr;
                initString =false;
            }
            else
                eventFilter << "," << *itr;
        }

        if (!initString)
            eventFilter << "))";
        else
            eventFilter << ")";

        result = GameDataDatabase.PQuery("SELECT gameobject.guid, id, position_x, position_y, position_z, orientation, map, "
            "(POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ FROM gameobject "
            "LEFT OUTER JOIN game_event_gameobject on gameobject.guid=game_event_gameobject.guid WHERE map = '%i' %s ORDER BY order_ ASC LIMIT 10",
            m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ(), m_session->GetPlayer()->GetMapId(),eventFilter.str().c_str());
    }

    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TARGETOBJNOTFOUND);
        return true;
    }

    bool found = false;
    float x, y, z, o;
    uint32 lowguid, id;
    uint16 mapid;

    do
    {
        Field *fields = result->Fetch();
        lowguid = fields[0].GetUInt32();
        id =      fields[1].GetUInt32();
        x =       fields[2].GetFloat();
        y =       fields[3].GetFloat();
        z =       fields[4].GetFloat();
        o =       fields[5].GetFloat();
        mapid =   fields[6].GetUInt16();
        if (sPoolMgr.IsSpawnedOrNotInPoolGameobject(lowguid))
            found = true;
    } while (result->NextRow() && (!found));

    if (!found)
    {
        PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST,id);
        return false;
    }


    GameObjectInfo const* goI = ObjectMgr::GetGameObjectInfo(id);

    if (!goI)
    {
        PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST,id);
        return false;
    }

    GameObject* target = m_session->GetPlayer()->GetMap()->GetGameObject(MAKE_NEW_GUID(lowguid,id,HIGHGUID_GAMEOBJECT));

    PSendSysMessage(LANG_GAMEOBJECT_DETAIL, lowguid, goI->name, lowguid, id, x, y, z, mapid, o);

    if (target)
    {
        int32 curRespawnDelay = target->GetRespawnTimeEx()-time(NULL);
        if (curRespawnDelay < 0)
            curRespawnDelay = 0;

        std::string curRespawnDelayStr = m_session->secondsToTimeString(curRespawnDelay,true);
        std::string defRespawnDelayStr = m_session->secondsToTimeString(target->GetRespawnDelay(),true);

        PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(),curRespawnDelayStr.c_str());
        const uint32 GOflags = target->GetUInt32Value(GAMEOBJECT_FLAGS);
        PSendSysMessage("Selected GameObject flags: %u, Loot state: %u, HasLooters: %u, isLooted %u, internal timer %i", GOflags, target->getLootState(), target->loot.HasLooters(), target->loot.isLooted(), target->GetCooldownTimeLeft());
    }
    return true;
}

//teleport to gameobject
bool ChatHandler::HandleGoObjectCommand(const char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if (!cId)
        return false;

    int32 guid = atoi(cId);
    if (!guid)
        return false;

    float x, y, z, ort;
    int mapid;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr.GetGOData(guid))
    {
        x = go_data->posX;
        y = go_data->posY;
        z = go_data->posZ;
        ort = go_data->orientation;
        mapid = go_data->mapid;
    }
    else
    {
        SendSysMessage(LANG_COMMAND_GOOBJNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (!MapManager::IsValidMapCoord(mapid,x,y,z,ort))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    _player->TeleportTo(mapid, x, y, z, ort);
    return true;
}

bool ChatHandler::HandleGoTicketCommand(const char * args)
{
     if (!*args)
        return false;

    char *cstrticket_id = strtok((char*)args, " ");

    if (!cstrticket_id)
        return false;

    uint64 ticket_id = atoi(cstrticket_id);
    if (!ticket_id)
        return false;

    GM_Ticket *ticket = sTicketMgr.GetGMTicketAny(ticket_id);
    if (!ticket)
    {
        SendSysMessage(LANG_COMMAND_TICKETNOTEXIST);
        return true;
    }

    float x, y, z;
    int mapid;

    x = ticket->pos_x;
    y = ticket->pos_y;
    z = ticket->pos_z;
    mapid = ticket->map;

    Player* _player = m_session->GetPlayer();
    _player->InterruptTaxiFlying();

    _player->TeleportTo(mapid, x, y, z, 1, 0);
    return true;
}

bool ChatHandler::HandleGoTriggerCommand(const char* args)
{
    Player* _player = m_session->GetPlayer();

    if (!*args)
        return false;

    char *atId = strtok((char*)args, " ");
    if (!atId)
        return false;

    int32 i_atId = atoi(atId);

    if (!i_atId)
        return false;

    AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(i_atId);
    if (!at)
    {
        PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND,i_atId);
        SetSentErrorMessage(true);
        return false;
    }

    if (!MapManager::IsValidMapCoord(at->mapid,at->x,at->y,at->z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,at->x,at->y,at->mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    _player->TeleportTo(at->mapid, at->x, at->y, at->z, _player->GetOrientation());
    return true;
}

bool ChatHandler::HandleGoGraveyardCommand(const char* args)
{
    Player* _player = m_session->GetPlayer();

    if (!*args)
        return false;

    char *gyId = strtok((char*)args, " ");
    if (!gyId)
        return false;

    int32 i_gyId = atoi(gyId);

    if (!i_gyId)
        return false;

    WorldSafeLocsEntry const* gy = sWorldSafeLocsStore.LookupEntry(i_gyId);
    if (!gy)
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST,i_gyId);
        SetSentErrorMessage(true);
        return false;
    }

    if (!MapManager::IsValidMapCoord(gy->map_id,gy->x,gy->y,gy->z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,gy->x,gy->y,gy->map_id);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    _player->TeleportTo(gy->map_id, gy->x, gy->y, gy->z, _player->GetOrientation());
    return true;
}

/** \brief Teleport the GM to the specified creature
 *
 * .gocreature <GUID>      --> TP using creature.guid
 * .gocreature azuregos    --> TP player to the mob with this name
 *                             Warning: If there is more than one mob with this name
 *                                      you will be teleported to the first one that is found.
 * .gocreature id 6109     --> TP player to the mob, that has this creature_template.entry
 *                             Warning: If there is more than one mob with this "id"
 *                                      you will be teleported to the first one that is found.
 */
//teleport to creature
bool ChatHandler::HandleGoCreatureCommand(const char* args)
{
    if (!*args)
        return false;
    Player* _player = m_session->GetPlayer();

    // "id" or number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
    char* pParam1 = extractKeyFromLink((char*)args,"Hcreature");
    if (!pParam1)
        return false;

    std::ostringstream whereClause;

    // User wants to teleport to the NPC's template entry
    if (strcmp(pParam1, "id") == 0)
    {
        //sLog.outLog(LOG_DEFAULT, "DEBUG: ID found");

        // Get the "creature_template.entry"
        // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
        char* tail = strtok(NULL,"");
        if (!tail)
            return false;
        char* cId = extractKeyFromLink(tail,"Hcreature_entry");
        if (!cId)
            return false;

        int32 tEntry = atoi(cId);
        //sLog.outLog(LOG_DEFAULT, "DEBUG: ID value: %d", tEntry);
        if (!tEntry)
            return false;

        whereClause << "WHERE id = '" << tEntry << "'";
    }
    else
    {
        //sLog.outLog(LOG_DEFAULT, "DEBUG: ID *not found*");

        int32 guid = atoi(pParam1);

        // Number is invalid - maybe the user specified the mob's name
        if (!guid)
        {
            std::string name = pParam1;
            GameDataDatabase.escape_string(name);
            whereClause << ", creature_template WHERE creature.id = creature_template.entry AND creature_template.name " _LIKE_ " '%" << name << "%' ORDER BY creature_template.rank desc,creature_template.maxhealth desc LIMIT 1";
        }
        else
        {
            whereClause <<  "WHERE guid = '" << guid << "'";
        }
    }
    //sLog.outLog(LOG_DEFAULT, "DEBUG: %s", whereClause.c_str());

    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map FROM creature %s ", whereClause.str().c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }
    if (result->GetRowCount() > 1)
    {
        SendSysMessage(LANG_COMMAND_GOCREATMULTIPLE);
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();

    if (!MapManager::IsValidMapCoord(mapid,x,y,z,ort))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    _player->InterruptTaxiFlying();

    _player->TeleportTo(mapid, x, y, z, ort);
    return true;
}

bool ChatHandler::HandleGoCreatureDirectCommand(const char* args)
{
    if (!*args || !m_session)
        return false;
    uint32 lowguid = atoi(args);
    CreatureData const* data = sObjectMgr.GetCreatureData(lowguid);
    if (!data)
    {
        PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT `map`, `position_x`, `position_y` FROM creature WHERE guid = '%u'",lowguid);
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }
    uint16 mapid = result->Fetch()[0].GetUInt16();
    float x = result->Fetch()[1].GetFloat();
    float y = result->Fetch()[2].GetFloat();
    Map* map = sMapMgr.FindMap(mapid, sObjectMgr.GetSingleInstance(mapid, x, y));

    if (!map)
    {
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Creature* creature = map->GetCreature(MAKE_NEW_GUID(lowguid,data->id,HIGHGUID_UNIT));
    if (!creature)
        {
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->InterruptTaxiFlying();
    
    WorldLocation loc;
    creature->GetPosition(loc);
    m_session->GetPlayer()->TeleportTo(loc);
    return true;
}

bool ChatHandler::HandleGoCreatureInMapCommand(const char* args)
{
    if (!*args || !m_session)
        return false;

    uint64 guid = std::stoull(args, NULL, 10);

    Map* map = m_session->GetPlayer()->GetMap();

    if (!map)
    {
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Unit* found = map->GetUnit(guid);
    if (!found)
    {
        SendSysMessage(LANG_COMMAND_GOCREATNOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->InterruptTaxiFlying();

    WorldLocation loc;
    found->GetPosition(loc);
    m_session->GetPlayer()->TeleportTo(loc);
    return true;
}

bool ChatHandler::HandleGUIDCommand(const char* /*args*/)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();

    if (guid == 0)
    {
        SendSysMessage(LANG_NO_SELECTION);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_OBJECT_GUID, GUID_LOPART(guid), GUID_HIPART(guid));
    return true;
}

bool ChatHandler::HandleLookupFactionCommand(const char* args)
{
    if (!*args)
        return false;

    // Can be NULL at console call
    Player *target = getSelectedPlayer ();

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower (wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    for (uint32 id = 0; id < sFactionStore.GetNumRows(); ++id)
    {
        FactionEntry const *factionEntry = sFactionStore.LookupEntry (id);
        if (factionEntry)
        {
            FactionState const* repState = target ? target->GetReputationMgr().GetState(factionEntry) : NULL;

            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = factionEntry->name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (m_session && loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = factionEntry->name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                // send faction in "id - [faction] rank reputation [visible] [at war] [own team] [unknown] [invisible] [inactive]" format
                // or              "id - [faction] [no reputation]" format
                std::ostringstream ss;
                if (m_session)
                    ss << id << " - |cffffffff|Hfaction:" << id << "|h[" << name << " " << localeNames[loc] << "]|h|r";
                else
                    ss << id << " - " << name << " " << localeNames[loc];

                if (repState)                               // and then target!=NULL also
                {
                    ReputationRank rank = target->GetReputationMgr().GetRank(factionEntry);
                    std::string rankName = GetHellgroundString(ReputationRankStrIndex[rank]);

                    ss << " " << rankName << "|h|r (" << target->GetReputationMgr().GetReputation(factionEntry) << ")";

                    if (repState->Flags & FACTION_FLAG_VISIBLE)
                        ss << GetHellgroundString(LANG_FACTION_VISIBLE);
                    if (repState->Flags & FACTION_FLAG_AT_WAR)
                        ss << GetHellgroundString(LANG_FACTION_ATWAR);
                    if (repState->Flags & FACTION_FLAG_PEACE_FORCED)
                        ss << GetHellgroundString(LANG_FACTION_PEACE_FORCED);
                    if (repState->Flags & FACTION_FLAG_HIDDEN)
                        ss << GetHellgroundString(LANG_FACTION_HIDDEN);
                    if (repState->Flags & FACTION_FLAG_INVISIBLE_FORCED)
                        ss << GetHellgroundString(LANG_FACTION_INVISIBLE_FORCED);
                    if (repState->Flags & FACTION_FLAG_INACTIVE)
                        ss << GetHellgroundString(LANG_FACTION_INACTIVE);
                }
                else
                    ss << GetHellgroundString(LANG_FACTION_NOREPUTATION);

                SendSysMessage(ss.str().c_str());
                counter++;
            }
        }
    }

    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_FACTION_NOTFOUND);
    return true;
}

bool ChatHandler::HandleModifyRepCommand(const char * args)
{
    if (!*args) return false;

    Player* target = NULL;
    target = getSelectedPlayer();

    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    char* factionTxt = extractKeyFromLink((char*)args,"Hfaction");
    if (!factionTxt)
        return false;

    uint32 factionId = atoi(factionTxt);

    int32 amount = 0;
    char *rankTxt = strtok(NULL, " ");
    if (!factionTxt || !rankTxt)
        return false;

    amount = atoi(rankTxt);
    if ((amount == 0) && (rankTxt[0] != '-') && !isdigit(rankTxt[0]))
    {
        std::string rankStr = rankTxt;
        std::wstring wrankStr;
        if (!Utf8toWStr(rankStr,wrankStr))
            return false;
        wstrToLower(wrankStr);

        int r = 0;
        amount = -42000;
        for (; r < MAX_REPUTATION_RANK; ++r)
        {
            std::string rank = GetHellgroundString(ReputationRankStrIndex[r]);
            if (rank.empty())
                continue;

            std::wstring wrank;
            if (!Utf8toWStr(rank,wrank))
                continue;

            wstrToLower(wrank);

            if (wrank.substr(0,wrankStr.size())==wrankStr)
            {
                char *deltaTxt = strtok(NULL, " ");
                if (deltaTxt)
                {
                    int32 delta = atoi(deltaTxt);
                    if ((delta < 0) || (delta > ReputationMgr::PointsInRank[r] -1))
                    {
                        PSendSysMessage(LANG_COMMAND_FACTION_DELTA, (ReputationMgr::PointsInRank[r]-1));
                        SetSentErrorMessage(true);
                        return false;
                    }
                    amount += delta;
                }
                break;
            }
            amount += ReputationMgr::PointsInRank[r];
        }
        if (r >= MAX_REPUTATION_RANK)
        {
            PSendSysMessage(LANG_COMMAND_FACTION_INVPARAM, rankTxt);
            SetSentErrorMessage(true);
            return false;
        }
    }

    FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionId);

    if (!factionEntry)
    {
        PSendSysMessage(LANG_COMMAND_FACTION_UNKNOWN, factionId);
        SetSentErrorMessage(true);
        return false;
    }

    if (factionEntry->reputationListID < 0)
    {
        PSendSysMessage(LANG_COMMAND_FACTION_NOREP_ERROR, factionEntry->name[m_session->GetSessionDbcLocale()], factionId);
        SetSentErrorMessage(true);
        return false;
    }

    target->GetReputationMgr().SetReputation(factionEntry,amount);
    PSendSysMessage(LANG_COMMAND_MODIFY_REP, factionEntry->name[m_session->GetSessionDbcLocale()], factionId,
                   target->GetName(), target->GetReputationMgr().GetReputation(factionEntry));
    return true;
}

bool ChatHandler::HandleNameCommand(const char* args)
{
    return true;
}

bool ChatHandler::HandleSubNameCommand(const char* /*args*/)
{
    return true;
}

//move item to other slot
bool ChatHandler::HandleItemMoveCommand(const char* args)
{
    if (!*args)
        return false;
    uint8 srcslot, dstslot;

    char* pParam1 = strtok((char*)args, " ");
    if (!pParam1)
        return false;

    char* pParam2 = strtok(NULL, " ");
    if (!pParam2)
        return false;

    srcslot = (uint8)atoi(pParam1);
    dstslot = (uint8)atoi(pParam2);

    if (srcslot==dstslot)
        return true;

    if (!m_session->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0,srcslot))
        return false;

    if (!m_session->GetPlayer()->IsValidPos(INVENTORY_SLOT_BAG_0,dstslot))
        return false;

    uint16 src = ((INVENTORY_SLOT_BAG_0 << 8) | srcslot);
    uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | dstslot);

    m_session->GetPlayer()->SwapItem(src, dst);

    return true;
}

//add spawn of creature
bool ChatHandler::HandleNpcAddCommand(const char* args)
{
    if (!*args)
        return false;
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    char* team = strtok(NULL, " ");
    int32 teamval = 0;
    if (team) { teamval = atoi(team); }
    if (teamval < 0) { teamval = 0; }

    uint32 id  = atoi(charID);

    Player *chr = m_session->GetPlayer();
    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float o = chr->GetOrientation();
    Map *map = chr->GetMap();

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT MAX(guid) FROM creature");
    if (!result)
        return false;

    uint32 newCreatureGuid = (*result)[0].GetUInt32()+1;

    Creature* pCreature = new Creature;
    if (!pCreature->Create(newCreatureGuid, map, id, PlayerTeam(teamval), x, y, z, o))
    {
        delete pCreature;
        return false;
    }

    pCreature->SaveUpdateOrNewToDB_npc_add(map->GetId(), (1 << map->GetSpawnMode()));

    uint32 db_guid = pCreature->GetDBTableGUIDLow();

    // To call _LoadGoods(); _LoadQuests(); CreateTrainerSpells();
    pCreature->LoadFromDB(db_guid, map);

    map->Add(pCreature);
    sObjectMgr.AddCreatureToGrid(db_guid, sObjectMgr.GetCreatureData(db_guid));
    return true;
}

bool ChatHandler::HandleNpcDeleteCommand(const char* args)
{
    Creature* unit = NULL;

    if (*args)
    {
        // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
        char* cId = extractKeyFromLink((char*)args,"Hcreature");
        if (!cId)
            return false;

        uint32 lowguid = atoi(cId);
        if (!lowguid)
            return false;

        if (CreatureData const* cr_data = sObjectMgr.GetCreatureData(lowguid))
            unit = m_session->GetPlayer()->GetMap()->GetCreature(MAKE_NEW_GUID(lowguid, cr_data->id, HIGHGUID_UNIT));
    }
    else
        unit = getSelectedCreature();

    if (!unit || unit->isPet() || unit->isTotem())
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // Delete the creature
    unit->CombatStop();
    unit->DeleteFromDB_npc_del();
    unit->AddObjectToRemoveList();

    SendSysMessage(LANG_COMMAND_DELCREATMESSAGE);

    return true;
}

//delete object by selection or guid
bool ChatHandler::HandleGameObjectDeleteCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if (!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if (!lowguid)
        return false;

    GameObject* obj = NULL;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr.GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithDbGuid(lowguid,go_data->id);

    if (!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 owner_guid = obj->GetOwnerGUID();
    if (owner_guid)
    {
        Unit* owner = m_session->GetPlayer()->GetMap()->GetUnit(owner_guid);
        if (!owner || !IS_PLAYER_GUID(owner_guid))
        {
            PSendSysMessage(LANG_COMMAND_DELOBJREFERCREATURE, GUID_LOPART(owner_guid), obj->GetGUIDLow());
            SetSentErrorMessage(true);
            return false;
        }

        owner->RemoveGameObject(obj,false);
    }

    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    obj->DeleteFromDB_gob_del();

    PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, obj->GetGUIDLow());

    return true;
}

//turn selected object
bool ChatHandler::HandleGameObjectTurnCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args, "Hgameobject");
    if (!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if (!lowguid)
        return false;

    GameObject* obj = NULL;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr.GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithDbGuid(lowguid,go_data->id);

    if (!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    char* po = strtok(NULL, " ");
    float o;

    if (po)
    {
        o = (float)atof(po);
    }
    else
    {
        Player *chr = m_session->GetPlayer();
        o = chr->GetOrientation();
    }

    Map* map = obj->GetMap();
    map->Remove(obj,false);

    obj->Relocate(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), o);

    obj->UpdateRotationFields();

    map->Add(obj);

    obj->SaveUpdateToDB();
    obj->Refresh();

    PSendSysMessage(LANG_COMMAND_TURNOBJMESSAGE, obj->GetGUIDLow(), obj->GetGOInfo()->name, obj->GetGUIDLow(), o);

    return true;
}

//move selected creature
bool ChatHandler::HandleNpcMoveCommand(const char* args)
{
    uint32 lowguid = 0;

    Creature* pCreature = getSelectedCreature();

    float o = m_session->GetPlayer()->GetOrientation();

    if (!pCreature)
    {
        // number or [name] Shift-click form |color|Hcreature:creature_guid|h[name]|h|r
        char* cId = extractKeyFromLink((char*)args,"Hcreature");
        if (!cId)
            return false;

        lowguid = atoi(cId);

        // Attempting creature load from DB data
        CreatureData const* data = sObjectMgr.GetCreatureData(lowguid);
        if (!data)
        {
            PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
            SetSentErrorMessage(true);
            return false;
        }

        o = data->orientation;// .npc move guid should not change orientation

        uint32 map_id = data->mapid;
        if (m_session->GetPlayer()->GetMapId()!=map_id)
        {
            PSendSysMessage(LANG_COMMAND_CREATUREATSAMEMAP, lowguid);
            SetSentErrorMessage(true);
            return false;
        }
        else
        {
            Map* pMap = sMapMgr.FindMap(map_id,m_session->GetPlayer()->GetAnyInstanceId());
            if (pMap)
                pCreature = pMap->GetCreature(MAKE_NEW_GUID(lowguid,data->id,HIGHGUID_UNIT));
            //not sure how MAKE_NEW_GUID does what it does, and FIXME: does not work in instances
        }
    }
    else
    {
        lowguid = pCreature->GetDBTableGUIDLow();
    }
    
    float x = m_session->GetPlayer()->GetPositionX();
    float y = m_session->GetPlayer()->GetPositionY();
    float z = m_session->GetPlayer()->GetPositionZ();

    if (CreatureData const* data = sObjectMgr.GetCreatureData(lowguid))
    {
        const_cast<CreatureData*>(data)->posX = x;
        const_cast<CreatureData*>(data)->posY = y;
        const_cast<CreatureData*>(data)->posZ = z;
        const_cast<CreatureData*>(data)->orientation = o;
    }
    if (pCreature)
    {
        Map *pMap = pCreature->GetMap();
        pMap->CreatureRelocation(pCreature,x, y, z,o);
        pCreature->GetMotionMaster()->Initialize();
        if (pCreature->isAlive())                            // dead creature will reset movement generator at respawn
        {
            pCreature->setDeathState(JUST_DIED);
            pCreature->Respawn();
        }
    }

    GameDataDatabase.PExecuteLog("UPDATE creature SET position_x = '%f', position_y = '%f', position_z = '%f', orientation = '%f' WHERE guid = '%u'", x, y, z, o, lowguid);
    SendSysMessage(LANG_COMMAND_CREATUREMOVED);
    return true;
}

//move selected object
bool ChatHandler::HandleGameObjectMoveCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args, "Hgameobject");
    if (!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if (!lowguid)
        return false;

    GameObject* obj = NULL;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr.GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithDbGuid(lowguid,go_data->id);

    if (!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    char* px = strtok(NULL, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px)
    {
        Player *chr = m_session->GetPlayer();

        Map* map = obj->GetMap();
        map->Remove(obj,false);

        obj->Relocate(chr->GetPositionX(), chr->GetPositionY(), chr->GetPositionZ(), obj->GetOrientation());
        obj->SetFloatValue(GAMEOBJECT_POS_X, chr->GetPositionX());
        obj->SetFloatValue(GAMEOBJECT_POS_Y, chr->GetPositionY());
        obj->SetFloatValue(GAMEOBJECT_POS_Z, chr->GetPositionZ());

        map->Add(obj);
    }
    else
    {
        if (!py || !pz)
            return false;

        float x = (float)atof(px);
        float y = (float)atof(py);
        float z = (float)atof(pz);

        if (!MapManager::IsValidMapCoord(obj->GetMapId(),x,y,z))
        {
            PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,obj->GetMapId());
            SetSentErrorMessage(true);
            return false;
        }

        Map* map = obj->GetMap();
        map->Remove(obj,false);

        obj->Relocate(x, y, z, obj->GetOrientation());
        obj->SetFloatValue(GAMEOBJECT_POS_X, x);
        obj->SetFloatValue(GAMEOBJECT_POS_Y, y);
        obj->SetFloatValue(GAMEOBJECT_POS_Z, z);

        map->Add(obj);
    }

    obj->SaveUpdateToDB();
    obj->Refresh();

    PSendSysMessage(LANG_COMMAND_MOVEOBJMESSAGE, obj->GetGUIDLow(), obj->GetGOInfo()->name, obj->GetGUIDLow());

    return true;
}

//demorph player or unit
bool ChatHandler::HandleDeMorphCommand(const char* /*args*/)
{
    Unit *target = getSelectedUnit();
    if (!target)
        target = m_session->GetPlayer();

    target->DeMorph();

    return true;
}

bool ChatHandler::HandleTrueDeMorphCommand(const char* /*args*/)
{
    Unit *target = getSelectedUnit();
    if (!target)
        target = m_session->GetPlayer();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return false;

    uint32 OriginalModelId;
    Gender gender;

    switch (target->GetGender())
    {
        case GENDER_MALE:
            {
            gender = GENDER_MALE;
            switch (target->GetRace())
            {
                case RACE_HUMAN:            OriginalModelId = 49; break;
                case RACE_ORC:                OriginalModelId = 51; break;
                case RACE_DWARF:            OriginalModelId = 53; break;
                case RACE_NIGHTELF:            OriginalModelId = 55; break;
                case RACE_UNDEAD_PLAYER:    OriginalModelId = 57; break;
                case RACE_TAUREN:            OriginalModelId = 59; break;
                case RACE_GNOME:            OriginalModelId = 1563; break;
                case RACE_TROLL:            OriginalModelId = 1478; break;
                case RACE_BLOODELF:            OriginalModelId = 15476; break;
                case RACE_DRAENEI:            OriginalModelId = 16125; break;
                default:
                    break;
            }
            break;
            }
        case GENDER_FEMALE:
            {
            gender = GENDER_FEMALE;
            switch (target->GetRace())
            {
                case RACE_HUMAN:            OriginalModelId = 50; break;
                case RACE_ORC:                OriginalModelId = 52; break;
                case RACE_DWARF:            OriginalModelId = 54; break;
                case RACE_NIGHTELF:            OriginalModelId = 56; break;
                case RACE_UNDEAD_PLAYER:    OriginalModelId = 58; break;
                case RACE_TAUREN:            OriginalModelId = 60; break;
                case RACE_GNOME:            OriginalModelId = 1564; break;
                case RACE_TROLL:            OriginalModelId = 1479; break;
                case RACE_BLOODELF:            OriginalModelId = 15475; break;
                case RACE_DRAENEI:            OriginalModelId = 16126; break;
                default:
                    break;
            }
            break;
            }
        default:
            break;
    }
    target->SetByteValue(UNIT_FIELD_BYTES_0, 2, gender);
    target->SetByteValue(PLAYER_BYTES_3, 0, gender);
    target->SetNativeDisplayId(OriginalModelId);
    target->SetDisplayId(OriginalModelId);
    target->setTransForm(0);
    return true;
}

//add item in vendorlist
bool ChatHandler::HandleNpcAddItemCommand(const char* args)
{
    if (!*args)
        return false;

    char* pitem  = extractKeyFromLink((char*)args,"Hitem");
    if (!pitem)
    {
        SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 itemId = atol(pitem);

    char* fmaxcount = strtok(NULL, " ");                    //add maxcount, default: 0
    uint32 maxcount = 0;
    if (fmaxcount)
        maxcount = atol(fmaxcount);

    char* fincrtime = strtok(NULL, " ");                    //add incrtime, default: 0
    uint32 incrtime = 0;
    if (fincrtime)
        incrtime = atol(fincrtime);

    char* fextendedcost = strtok(NULL, " ");                //add ExtendedCost, default: 0
    uint32 extendedcost = fextendedcost ? atol(fextendedcost) : 0;

    Creature* vendor = getSelectedCreature();

    uint32 vendor_entry = vendor ? vendor->GetEntry() : 0;

    if (!sObjectMgr.IsVendorItemValid(vendor_entry,itemId,maxcount,incrtime,extendedcost,m_session->GetPlayer()))
    {
        SetSentErrorMessage(true);
        return false;
    }

    sObjectMgr.AddVendorItem(vendor_entry,itemId,maxcount,incrtime,extendedcost);

    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(itemId);

    PSendSysMessage(LANG_ITEM_ADDED_TO_LIST,itemId,pProto->Name1,maxcount,incrtime,extendedcost);
    return true;
}

//del item from vendor list
bool ChatHandler::HandleNpcDelItemCommand(const char* args)
{
    if (!*args)
        return false;

    Creature* vendor = getSelectedCreature();
    if (!vendor || !vendor->isVendor())
    {
        SendSysMessage(LANG_COMMAND_VENDORSELECTION);
        SetSentErrorMessage(true);
        return false;
    }

    char* pitem  = extractKeyFromLink((char*)args,"Hitem");
    if (!pitem)
    {
        SendSysMessage(LANG_COMMAND_NEEDITEMSEND);
        SetSentErrorMessage(true);
        return false;
    }
    uint32 itemId = atol(pitem);


    if (!sObjectMgr.RemoveVendorItem(vendor->GetEntry(),itemId))
    {
        PSendSysMessage(LANG_ITEM_NOT_IN_LIST,itemId);
        SetSentErrorMessage(true);
        return false;
    }

    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(itemId);

    PSendSysMessage(LANG_ITEM_DELETED_FROM_LIST,itemId,pProto->Name1);
    return true;
}

//add move for creature
bool ChatHandler::HandleNpcAddMoveCommand(const char* args)
{
    if (!*args)
        return false;

    char* guid_str = strtok((char*)args, " ");
    char* wait_str = strtok((char*)NULL, " ");

    uint32 lowguid = atoi((char*)guid_str);

    Creature* pCreature = NULL;

    /* FIXME: impossible without entry
    if (lowguid)
        pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));
    */

    // attempt check creature existence by DB data
    if (!pCreature)
    {
        CreatureData const* data = sObjectMgr.GetCreatureData(lowguid);
        if (!data)
        {
            PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        // obtain real GUID for DB operations
        lowguid = pCreature->GetDBTableGUIDLow();
    }

    int wait = wait_str ? atoi(wait_str) : 0;

    if (wait < 0)
        wait = 0;

    Player* player = m_session->GetPlayer();

    // update movement type
    GameDataDatabase.PExecuteLog("UPDATE creature SET MovementType = '%u' WHERE guid = '%u'", WAYPOINT_MOTION_TYPE,lowguid);
    if (pCreature && pCreature->GetWaypointPath())
    {
        pCreature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        pCreature->GetMotionMaster()->Initialize();
        if (pCreature->isAlive())                            // dead creature will reset movement generator at respawn
        {
            pCreature->setDeathState(JUST_DIED);
            pCreature->Respawn();
        }
        pCreature->SaveUpdateToDB();
    }

    return true;
}

/**
 * Set the movement type for an NPC.<br/>
 * <br/>
 * Valid movement types are:
 * <ul>
 * <li> stay - NPC wont move </li>
 * <li> random - NPC will move randomly according to the spawndist </li>
 * <li> way - NPC will move with given waypoints set </li>
 * </ul>
 * additional parameter: NODEL - so no waypoints are deleted, if you
 *                       change the movement type
 */
bool ChatHandler::HandleNpcSetMoveTypeCommand(const char* args)
{
    if (!*args)
        return false;

    // 3 arguments:
    // GUID (optional - you can also select the creature)
    // stay|random|way (determines the kind of movement)
    // NODEL (optional - tells the system NOT to delete any waypoints)
    //        this is very handy if you want to do waypoints, that are
    //        later switched on/off according to special events (like escort
    //        quests, etc)
    char* guid_str = strtok((char*)args, " ");
    char* type_str = strtok((char*)NULL, " ");
    char* dontdel_str = strtok((char*)NULL, " ");

    bool doNotDelete = false;

    if (!guid_str)
        return false;

    uint32 lowguid = 0;
    Creature* pCreature = NULL;

    if (dontdel_str)
    {
        //sLog.outLog(LOG_DEFAULT, "DEBUG: All 3 params are set");

        // All 3 params are set
        // GUID
        // type
        // doNotDEL
        if (stricmp(dontdel_str, "NODEL") == 0)
        {
            //sLog.outLog(LOG_DEFAULT, "DEBUG: doNotDelete = true;");
            doNotDelete = true;
        }
    }
    else
    {
        // Only 2 params - but maybe NODEL is set
        if (type_str)
        {
            sLog.outLog(LOG_DEFAULT, "DEBUG: Only 2 params ");
            if (stricmp(type_str, "NODEL") == 0)
            {
                //sLog.outLog(LOG_DEFAULT, "DEBUG: type_str, NODEL ");
                doNotDelete = true;
                type_str = NULL;
            }
        }
    }

    if (!type_str)                                           // case .setmovetype $move_type (with selected creature)
    {
        type_str = guid_str;
        pCreature = getSelectedCreature();
        if (!pCreature || pCreature->isPet())
            return false;
        lowguid = pCreature->GetDBTableGUIDLow();
    }
    else                                                    // case .setmovetype #creature_guid $move_type (with selected creature)
    {
        lowguid = atoi((char*)guid_str);

        /* impossible without entry
        if (lowguid)
            pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(),MAKE_GUID(lowguid,HIGHGUID_UNIT));
        */

        // attempt check creature existence by DB data
        if (!pCreature)
        {
            CreatureData const* data = sObjectMgr.GetCreatureData(lowguid);
            if (!data)
            {
                PSendSysMessage(LANG_COMMAND_CREATGUIDNOTFOUND, lowguid);
                SetSentErrorMessage(true);
                return false;
            }
        }
        else
        {
            lowguid = pCreature->GetDBTableGUIDLow();
        }
    }

    // now lowguid is low guid really existed creature
    // and pCreature point (maybe) to this creature or NULL

    MovementGeneratorType move_type;

    std::string type = type_str;

    if (type == "stay")
        move_type = IDLE_MOTION_TYPE;
    else if (type == "random")
        move_type = RANDOM_MOTION_TYPE;
    else if (type == "way")
        move_type = WAYPOINT_MOTION_TYPE;
    else
        return false;

    if (pCreature)
    {
        // update movement type
        if (doNotDelete == false)
            pCreature->LoadPath(0);

        pCreature->SetDefaultMovementType(move_type);
        pCreature->GetMotionMaster()->Initialize();
        if (pCreature->isAlive())                            // dead creature will reset movement generator at respawn
        {
            pCreature->setDeathState(JUST_DIED);
            pCreature->Respawn();
        }
        pCreature->SaveUpdateToDB();
    }
    if (doNotDelete == false)
    {
        PSendSysMessage(LANG_MOVE_TYPE_SET,type_str);
    }
    else
    {
        PSendSysMessage(LANG_MOVE_TYPE_SET_NODEL,type_str);
    }

    return true;
}                                                           // HandleNpcSetMoveTypeCommand

//change level of creature or pet
bool ChatHandler::HandleNpcChangeLevelCommand(const char* args)
{
    if (!*args)
        return false;

    uint8 lvl = (uint8) atoi((char*)args);
    if (lvl < 1 || lvl > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) + 3)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Creature* pCreature = getSelectedCreature();
    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (pCreature->isPet())
    {
        ((Pet*)pCreature)->GivePetLevel(lvl);
    }
    else
    {
        pCreature->SetMaxHealth(100 + 30*lvl);
        pCreature->SetHealth(100 + 30*lvl);
        pCreature->SetLevel(lvl);
        pCreature->SaveUpdateToDB();
    }

    return true;
}

//set npcflag of creature
bool ChatHandler::HandleNpcFlagCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 npcFlags = (uint32) atoi((char*)args);

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->SetUInt32Value(UNIT_NPC_FLAGS, npcFlags);

    GameDataDatabase.PExecuteLog("UPDATE creature_template SET npcflag = '%u' WHERE entry = '%u'", npcFlags, pCreature->GetEntry());

    SendSysMessage(LANG_VALUE_SAVED_REJOIN);

    return true;
}

//set field flags of creature
bool ChatHandler::HandleNpcFieldFlagCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 Flags = (uint32) atoi((char*)args);

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->SetUInt32Value(UNIT_FIELD_FLAGS, Flags);

    GameDataDatabase.PExecuteLog("UPDATE creature_template SET unit_flags = '%u' WHERE entry = '%u'", Flags, pCreature->GetEntry());

    SendSysMessage(LANG_VALUE_SAVED_REJOIN);

    return true;
}

bool ChatHandler::HandleNpcExtraFlagCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 Flags = (uint32) atoi((char*)args);

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (CreatureInfo const *cinfo = pCreature->GetCreatureInfo())
    {
        const_cast<CreatureInfo*>(cinfo)->flags_extra = Flags;
    }

    GameDataDatabase.PExecuteLog("UPDATE creature_template SET flags_extra = '%u' WHERE entry = '%u'", Flags, pCreature->GetEntry());

    SendSysMessage(LANG_VALUE_SAVED_REJOIN);

    return true;
}

//set model of creature
bool ChatHandler::HandleNpcSetModelCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 displayId = (uint32) atoi((char*)args);

    Creature *pCreature = getSelectedCreature();

    if (!pCreature || pCreature->isPet())
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->SetDisplayId(displayId);
    pCreature->SetNativeDisplayId(displayId);

    pCreature->SaveUpdateToDB();

    return true;
}

//morph creature or player
bool ChatHandler::HandleModifyMorphCommand(const char* args)
{
    if (!*args)
        return false;

    uint16 display_id = (uint16)atoi((char*)args);

    Unit *target = getSelectedUnit();
    if (!target)
        target = m_session->GetPlayer();

    target->SetDisplayId(display_id);

    return true;
}

bool ChatHandler::HandleTrueMorphCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 display_id = (uint32)atoi((char*)args);

    Unit *target = getSelectedUnit();
    if (!target)
        target = m_session->GetPlayer();

    if (target->GetTypeId() != TYPEID_PLAYER)
        return false;

    target->SetNativeDisplayId(display_id);

    return true;
}

bool ChatHandler::HandleSpellAffectCheckCommand(const char *args)
{
    if (!*args)
        return false;

    uint32 entry = (uint32)atoi((char*)args);

    Player* target = m_session->GetPlayer();

    uint8 atoieffectId = 0;
    if (target->GetLevel() == 71)
        atoieffectId = 1;
    else if (target->GetLevel() == 72)
        atoieffectId = 2;

    SpellEntry const *spellInfoCommand = sSpellTemplate.LookupEntry<SpellEntry>(entry);
    if (!spellInfoCommand)
        return false;

    uint64 LookSpellFamilyFlags = sSpellMgr.GetSpellAffectMask(entry,atoieffectId);
    if (!LookSpellFamilyFlags)
        LookSpellFamilyFlags = spellInfoCommand->EffectItemType[atoieffectId];

    bool found = false;

    for (uint32 id = 0; id < sSpellTemplate.GetMaxEntry(); ++id)
    {
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(id);
        if (spellInfo)
        {
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = spellInfo->SpellName[loc];
            uint64 SpellFamilyFlags = spellInfo->SpellFamilyFlags;

            if (SpellFamilyFlags == 0)
                continue;

            if (spellInfo->SpellFamilyName != spellInfoCommand->SpellFamilyName)
                continue;

            if (!(spellInfo->SpellFamilyFlags & LookSpellFamilyFlags))
                continue;
            
            if (loc < MAX_LOCALE)
            {
                bool known = target && target->HasSpell(id);
                bool learn = (spellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL);

                uint32 talentCost = GetTalentSpellCost(id);

                bool talent = (talentCost > 0);
                bool passive = SpellMgr::IsPassiveSpell(id);
                bool active = target && (target->HasAura(id,0) || target->HasAura(id,1) || target->HasAura(id,2));

                uint32 rank = talentCost ? talentCost : sSpellMgr.GetSpellRank(learn ? spellInfo->EffectTriggerSpell[0] : id);

                // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
                std::ostringstream ss;
                if (m_session)
                    ss << id << " - |cffffffff|Hspell:" << id << "|h[" << name;
                else
                    ss << id << " - " << name;

                if (rank)
                    ss << GetHellgroundString(LANG_SPELL_RANK) << rank;

                if (m_session)
                    ss << " " << localeNames[loc] << "]|h|r";
                else
                    ss << " " << localeNames[loc];

                if (talent)
                    ss << GetHellgroundString(LANG_TALENT);
                if (passive)
                    ss << GetHellgroundString(LANG_PASSIVE);
                if (learn)
                    ss << GetHellgroundString(LANG_LEARN);
                if (known)
                    ss << GetHellgroundString(LANG_KNOWN);
                if (active)
                    ss << GetHellgroundString(LANG_ACTIVE);

                target->Say(ss.str(), LANG_UNIVERSAL);
                if (!found)
                    found = true;
            }
        }
    }
    return true;
}

bool ChatHandler::HandleSpellKnownCheckCommand(const char *args)
{
    Player* target = getSelectedPlayer();

    bool found = false;

    for (uint32 id = 0; id < sSpellTemplate.GetMaxEntry(); ++id)
    {
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(id);
        if (spellInfo)
        {
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = spellInfo->SpellName[loc];
            
            if (loc < MAX_LOCALE)
            {
                bool known = target && target->HasSpell(id);
                bool learn = (spellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL);

                uint32 talentCost = GetTalentSpellCost(id);

                bool talent = (talentCost > 0);
                bool passive = SpellMgr::IsPassiveSpell(id);
                bool active = target && (target->HasAura(id,0) || target->HasAura(id,1) || target->HasAura(id,2));

                uint32 rank = talentCost ? talentCost : sSpellMgr.GetSpellRank(learn ? spellInfo->EffectTriggerSpell[0] : id);

                // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
                std::ostringstream ss;
                if (m_session)
                    ss << id << " - |cffffffff|Hspell:" << id << "|h[" << name;
                else
                    ss << id << " - " << name;

                if (rank)
                    ss << GetHellgroundString(LANG_SPELL_RANK) << rank;

                if (m_session)
                    ss << " " << localeNames[loc] << "]|h|r";
                else
                    ss << " " << localeNames[loc];

                if (talent)
                    ss << GetHellgroundString(LANG_TALENT);
                if (passive)
                    ss << GetHellgroundString(LANG_PASSIVE);
                if (learn)
                    ss << GetHellgroundString(LANG_LEARN);
                if (known)
                    ss << GetHellgroundString(LANG_KNOWN);
                if (active)
                    ss << GetHellgroundString(LANG_ACTIVE);

                if (known)
                    SendSysMessage(ss.str().c_str());
                if (!found)
                    found = true;
            }
        }
    }
    return true;
}

bool ChatHandler::HandleSpellInfoCheckCommand(const char *args)
{
    if (!*args)
        return false;

    uint32 spellId = (uint32)atoi((char*)args);

    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return false;

    PSendSysMessage("Id: %u", spellId);
    if (spellInfo->Category != 0)
        PSendSysMessage("Category: %u", spellInfo->Category);
    if (spellInfo->Dispel != 0)
        PSendSysMessage("Dispel: %u", spellInfo->Dispel);
    if (spellInfo->Mechanic != 0)
        PSendSysMessage("Mechanic: %u", spellInfo->Mechanic);
    if (spellInfo->Attributes != 0)
        PSendSysMessage("Attributes: %u", spellInfo->Attributes);
    if (spellInfo->AttributesEx != 0)
        PSendSysMessage("AttributesEx: %u", spellInfo->AttributesEx);
    if (spellInfo->AttributesEx2 != 0)
        PSendSysMessage("AttributesEx2: %u", spellInfo->AttributesEx2);
    if (spellInfo->AttributesEx3 != 0)
        PSendSysMessage("AttributesEx3: %u", spellInfo->AttributesEx3);
    if (spellInfo->AttributesEx4 != 0)
        PSendSysMessage("AttributesEx4: %u", spellInfo->AttributesEx4);
    if (spellInfo->AttributesEx5 != 0)
        PSendSysMessage("AttributesEx5: %u", spellInfo->AttributesEx5);
    if (spellInfo->AttributesEx6 != 0)
        PSendSysMessage("AttributesEx6: %u", spellInfo->AttributesEx6);
    if (spellInfo->AttributesCu != 0)
        PSendSysMessage("AttributesCu %u", spellInfo->AttributesCu);
    if (spellInfo->Stances != 0)
        PSendSysMessage("Stances %u", spellInfo->Stances);
    if (spellInfo->StancesNot != 0)
        PSendSysMessage("StancesNot %u", spellInfo->StancesNot);
    if (spellInfo->Targets != 0)
        PSendSysMessage("Targets (mask flag. Ex: unit) %u", spellInfo->Targets);
    if (spellInfo->TargetCreatureType != 0)
        PSendSysMessage("TargetCreatureType %u", spellInfo->TargetCreatureType);
    if (spellInfo->RequiresSpellFocus != 0)
        PSendSysMessage("RequiresSpellFocus %u", spellInfo->RequiresSpellFocus);
    if (spellInfo->FacingCasterFlags != 0)
        PSendSysMessage("FacingCasterFlags %u", spellInfo->FacingCasterFlags);
    if (spellInfo->CasterAuraState != 0)
        PSendSysMessage("CasterAuraState %u", spellInfo->CasterAuraState);
    if (spellInfo->TargetAuraState != 0)
        PSendSysMessage("TargetAuraState %u", spellInfo->TargetAuraState);
    if (spellInfo->CasterAuraStateNot != 0)
        PSendSysMessage("CasterAuraStateNot %u", spellInfo->CasterAuraStateNot);
    if (spellInfo->TargetAuraStateNot != 0)
        PSendSysMessage("TargetAuraStateNot %u", spellInfo->TargetAuraStateNot);
    if (spellInfo->CastingTimeIndex != 0)
        PSendSysMessage("CastingTimeIndex %u", spellInfo->CastingTimeIndex);
    if (spellInfo->RecoveryTime != 0)
        PSendSysMessage("RecoveryTime %u", spellInfo->RecoveryTime);
    if (spellInfo->CategoryRecoveryTime != 0)
        PSendSysMessage("CategoryRecoveryTime %u", spellInfo->CategoryRecoveryTime);
    if (spellInfo->InterruptFlags != 0)
        PSendSysMessage("InterruptFlags %u", spellInfo->InterruptFlags);
    if (spellInfo->AuraInterruptFlags != 0)
        PSendSysMessage("AuraInterruptFlags %u", spellInfo->AuraInterruptFlags);
    if (spellInfo->ChannelInterruptFlags != 0)
        PSendSysMessage("ChannelInterruptFlags %u", spellInfo->ChannelInterruptFlags);
    if (spellInfo->procFlags != 0)
        PSendSysMessage("procFlags %u", spellInfo->procFlags);
    if (spellInfo->procChance != 0 && spellInfo->procChance != 101)
        PSendSysMessage("procChance %u", spellInfo->procChance);
    if (spellInfo->procCharges != 0)
        PSendSysMessage("procCharges %u", spellInfo->procCharges);
    if (spellInfo->maxLevel != 0)
        PSendSysMessage("maxLevel %u", spellInfo->maxLevel);
    if (spellInfo->baseLevel != 0)
        PSendSysMessage("baseLevel %u", spellInfo->baseLevel);
    if (spellInfo->spellLevel != 0)
        PSendSysMessage("spellLevel %u", spellInfo->spellLevel);
    if (spellInfo->DurationIndex != 0)
        PSendSysMessage("DurationIndex %u", spellInfo->DurationIndex);
    PSendSysMessage("powerType %u", spellInfo->powerType);
    if (spellInfo->manaCost != 0)
        PSendSysMessage("manaCost %u", spellInfo->manaCost);
    if (spellInfo->manaCostPerlevel != 0)
        PSendSysMessage("manaCostPerlevel %u", spellInfo->manaCostPerlevel);
    if (spellInfo->manaPerSecond != 0)
        PSendSysMessage("manaPerSecond %u", spellInfo->manaPerSecond);
    if (spellInfo->manaPerSecondPerLevel != 0)
        PSendSysMessage("manaPerSecondPerLevel %u", spellInfo->manaPerSecondPerLevel);
    if (spellInfo->rangeIndex != 0)
        PSendSysMessage("rangeIndex %u", spellInfo->rangeIndex);
    if (spellInfo->speed != 0)
        PSendSysMessage("speed %f", spellInfo->speed);
    if (spellInfo->StackAmount != 0)
        PSendSysMessage("StackAmount %u", spellInfo->StackAmount);
    for (int i = 0; i < 2; ++i)
        if (spellInfo->Totem[i] != 0)
            PSendSysMessage("Totem %u", spellInfo->Totem[i]);
    for (int i = 0; i < 8; ++i)
        if (spellInfo->Reagent[i] != 0)
            PSendSysMessage("Reagent %u", spellInfo->Reagent[i]);
    for (int i = 0; i < 8; ++i)
        if (spellInfo->ReagentCount[i] != 0)
            PSendSysMessage("ReagentCount %u", spellInfo->ReagentCount[i]);
    if (spellInfo->EquippedItemClass != 0)
        PSendSysMessage("EquippedItemClass %u", spellInfo->EquippedItemClass);
    if (spellInfo->EquippedItemSubClassMask != 0)
        PSendSysMessage("EquippedItemSubClassMask %u", spellInfo->EquippedItemSubClassMask);
    if (spellInfo->EquippedItemInventoryTypeMask != 0)
        PSendSysMessage("EquippedItemInventoryTypeMask %u", spellInfo->EquippedItemInventoryTypeMask);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->Effect[i] != 0)
            PSendSysMessage("Effect %u", spellInfo->Effect[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectDieSides[i] != 0)
            PSendSysMessage("EffectDieSides %u", spellInfo->EffectDieSides[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectBaseDice[i] != 0)
            PSendSysMessage("EffectBaseDice %u", spellInfo->EffectBaseDice[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectDicePerLevel[i] != 0)
        {
            if (spellInfo->EffectDicePerLevel[i] < 0)
                PSendSysMessage("EffectDicePerLevel -%f", -(spellInfo->EffectDicePerLevel[i]));
            else
                PSendSysMessage("EffectDicePerLevel %f", spellInfo->EffectDicePerLevel[i]);
        }
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectRealPointsPerLevel[i] != 0)
        {
            if (spellInfo->EffectRealPointsPerLevel[i] < 0)
                PSendSysMessage("EffectRealPointsPerLevel -%f", -(spellInfo->EffectRealPointsPerLevel[i]));
            else
                PSendSysMessage("EffectRealPointsPerLevel %f", spellInfo->EffectRealPointsPerLevel[i]);
        }
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectBasePoints[i] != 0)
        {
            if (spellInfo->EffectBasePoints[i] < 0)
                PSendSysMessage("EffectBasePoints -%u", -(spellInfo->EffectBasePoints[i]));
            else
                PSendSysMessage("EffectBasePoints %u", spellInfo->EffectBasePoints[i]);
        }
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectMechanic[i] != 0)
            PSendSysMessage("EffectMechanic %u", spellInfo->EffectMechanic[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectImplicitTargetA[i] != 0)
            PSendSysMessage("EffectImplicitTargetA %u", spellInfo->EffectImplicitTargetA[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectImplicitTargetB[i] != 0)
            PSendSysMessage("EffectImplicitTargetB %u", spellInfo->EffectImplicitTargetB[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectRadiusIndex[i] != 0)
            PSendSysMessage("EffectRadiusIndex %u", spellInfo->EffectRadiusIndex[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectApplyAuraName[i] != 0)
            PSendSysMessage("EffectApplyAuraName %u", spellInfo->EffectApplyAuraName[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectAmplitude[i] != 0)
            PSendSysMessage("EffectAmplitude %u", spellInfo->EffectAmplitude[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectMultipleValue[i] != 0)
            PSendSysMessage("EffectMultipleValue %f", spellInfo->EffectMultipleValue[i]); // Should only be positive
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectChainTarget[i] != 0)
            PSendSysMessage("EffectChainTarget %u", spellInfo->EffectChainTarget[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectItemType[i] != 0)
            PSendSysMessage("EffectItemType %u", spellInfo->EffectItemType[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectMiscValue[i] != 0)
        {
            if (spellInfo->EffectMiscValue[i] < 0)
                PSendSysMessage("EffectMiscValue -%u", -(spellInfo->EffectMiscValue[i]));
            else
                PSendSysMessage("EffectMiscValue %u", spellInfo->EffectMiscValue[i]);
        }
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectMiscValueB[i] != 0)
        {
            if (spellInfo->EffectMiscValueB[i] < 0)
                PSendSysMessage("EffectMiscValueB -%u", -(spellInfo->EffectMiscValueB[i]));
            else
                PSendSysMessage("EffectMiscValueB %u", spellInfo->EffectMiscValueB[i]);
        }
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectTriggerSpell[i] != 0)
            PSendSysMessage("EffectTriggerSpell %u", spellInfo->EffectTriggerSpell[i]);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->EffectPointsPerComboPoint[i] != 0)
            PSendSysMessage("EffectPointsPerComboPoint %f", spellInfo->EffectPointsPerComboPoint[i]); // Should only be positive
    if (spellInfo->SpellVisual != 0)
        PSendSysMessage("SpellVisual %u", spellInfo->SpellVisual);
    if (spellInfo->SpellIconID != 0)
        PSendSysMessage("SpellIconID %u", spellInfo->SpellIconID);
    if (spellInfo->activeIconID != 0)
        PSendSysMessage("activeIconID %u", spellInfo->activeIconID);
    if (spellInfo->SpellName != 0)
        PSendSysMessage("SpellName %s", spellInfo->SpellName[0]);
    if (spellInfo->Rank[0] != 0)
        PSendSysMessage("Rank %s", spellInfo->Rank[0]);
    if (spellInfo->ManaCostPercentage != 0)
        PSendSysMessage("ManaCostPercentage %u", spellInfo->ManaCostPercentage);
    if (spellInfo->StartRecoveryCategory != 0)
        PSendSysMessage("StartRecoveryCategory %u", spellInfo->StartRecoveryCategory);
    if (spellInfo->StartRecoveryTime != 0)
        PSendSysMessage("StartRecoveryTime %u", spellInfo->StartRecoveryTime);
    if (spellInfo->MaxTargetLevel != 0)
        PSendSysMessage("MaxTargetLevel %u", spellInfo->MaxTargetLevel);
    if (spellInfo->SpellFamilyName != 0)
        PSendSysMessage("SpellFamilyName %u", spellInfo->SpellFamilyName);
    if (spellInfo->SpellFamilyFlags != 0)
    {
        uint64 SpellFamilyFlags = spellInfo->SpellFamilyFlags;
        uint64 itrStepen10 = 0x1LL;
        for (int itrStepen = 0; itrStepen < 16; ++itrStepen)
        {
            for (int itr = 1; itr <= 8; itr+=itr)
            {
                if (SpellFamilyFlags & (itr * itrStepen10))
                    PSendSysMessage("SpellFamilyFlags: %u * 10^%u", itr, itrStepen);
            }
            itrStepen10 *= 0x10LL;
        }
    }
    if (spellInfo->MaxAffectedTargets != 0)
        PSendSysMessage("MaxAffectedTargets %u", spellInfo->MaxAffectedTargets);
    PSendSysMessage("DmgClass %u", spellInfo->DmgClass);
    if (spellInfo->PreventionType != 0)
        PSendSysMessage("PreventionType %u", spellInfo->PreventionType);
    for (int i = 0; i < 3; ++i)
        if (spellInfo->DmgMultiplier[i] != 0)
            PSendSysMessage("DmgMultiplier %f", spellInfo->DmgMultiplier[i]); // Should only be positive
    for (int i = 0; i < 2; ++i)
        if (spellInfo->TotemCategory[i] != 0)
            PSendSysMessage("TotemCategory %u", spellInfo->TotemCategory[i]);
    if (spellInfo->AreaId != 0)
        PSendSysMessage("AreaId %u", spellInfo->AreaId);
    if (spellInfo->SchoolMask != 0)
        PSendSysMessage("SchoolMask %u", spellInfo->SchoolMask);
    if (SpellThreatEntry const *threatSpell = sSpellMgr.GetSpellThreat(spellId)) // Spell_threat
        PSendSysMessage("Spell Threat: %u", threatSpell->threat);
    for (int i = 0; i < 3; ++i)
        if (uint64 LookSpellFamilyFlags = sSpellMgr.GetSpellAffectMask(spellId, i)) // Spell_affect
        {
            uint64 itrStepen10 = 0x1LL;
            for (int itrStepen = 0; itrStepen < 16; ++itrStepen)
            {
                for (int itr = 1; itr <= 8; itr+=itr)
                {
                    if (LookSpellFamilyFlags & (itr * itrStepen10))
                        PSendSysMessage("Effect %u Affects Spellfamilyflags: %u * 10^%u", i, itr, itrStepen);
                }
                itrStepen10 *= 0x10LL;
            }
        }
    if (spellInfo->EffectImplicitTargetA[0]==TARGET_DST_DB || spellInfo->EffectImplicitTargetB[0]==TARGET_DST_DB ||
        spellInfo->EffectImplicitTargetA[1]==TARGET_DST_DB || spellInfo->EffectImplicitTargetB[1]==TARGET_DST_DB ||
        spellInfo->EffectImplicitTargetA[2]==TARGET_DST_DB || spellInfo->EffectImplicitTargetB[2]==TARGET_DST_DB)
        if (SpellTargetPosition const* SpellTargetPosition = sSpellMgr.GetSpellTargetPosition(spellId)) // spell_target_position
        {
            PSendSysMessage("Spell Target Position target_mapId: %u, Coordinates: X %f, Y %f, Z %f", SpellTargetPosition->target_mapId, SpellTargetPosition->target_X, SpellTargetPosition->target_Y, SpellTargetPosition->target_Z);
            if (SpellTargetPosition->target_Orientation != 0)
                PSendSysMessage("Spell Target Position target_Orientation: %f", SpellTargetPosition->target_Orientation);
        }

    SpellScriptTarget::const_iterator lower = sSpellMgr.GetBeginSpellScriptTarget(spellId); // spell_script_target
    SpellScriptTarget::const_iterator upper = sSpellMgr.GetEndSpellScriptTarget(spellId);
    if (lower != upper)
        for (SpellScriptTarget::const_iterator i_spellST = lower; i_spellST != upper; ++i_spellST)
        {
            PSendSysMessage("Spell Script Target Type: %u, Entry: %u",i_spellST->second.type, i_spellST->second.targetEntry);
        }

    if (SpellProcEventEntry const* spellProcEvent = sSpellMgr.GetSpellProcEvent(spellId)) // spell_proc_event
    {
        if (spellProcEvent->schoolMask != 0)
            PSendSysMessage("Spell Proc Event schoolMask: %u", spellProcEvent->schoolMask);
        if (spellProcEvent->spellFamilyName != 0)
            PSendSysMessage("Spell Proc Event spellFamilyName: %u", spellProcEvent->spellFamilyName);
        if (uint64 SpellFamilyMask = spellProcEvent->spellFamilyMask) // spell_proc_event mask
        {
            uint64 itrStepen10 = 0x1LL;
            for (int itrStepen = 0; itrStepen < 16; ++itrStepen)
            {
                for (int itr = 1; itr <= 8; itr+=itr)
                {
                    if (SpellFamilyMask & (itr * itrStepen10))
                        PSendSysMessage("Spell Proc Event spellFamilyMask: %u * 10^%u", itr, itrStepen);
                }
                itrStepen10 *= 0x10LL;
            }
        }
        if (spellProcEvent->procFlags != 0)
            PSendSysMessage("Spell Proc Event procFlags: %u", spellProcEvent->procFlags);
        if (spellProcEvent->procEx != 0)
            PSendSysMessage("Spell Proc Event procEx: %u", spellProcEvent->procEx);
        if (spellProcEvent->ppmRate != 0)
            PSendSysMessage("Spell Proc Event ppmRate: %f", spellProcEvent->ppmRate);
        if (spellProcEvent->customChance != 0)
            PSendSysMessage("Spell Proc Event customChance: %f", spellProcEvent->customChance);
        if (spellProcEvent->cooldown != 0)
            PSendSysMessage("Spell Proc Event cooldown: %u", spellProcEvent->cooldown);        
    }

    if (uint32 ElixirMask = sSpellMgr.GetSpellElixirMask(spellId)) // Spell_elixir
        PSendSysMessage("Spell Elixir Mask: %u", ElixirMask);

    if (SkillLineAbilityEntry const *pAbility = sSkillLineAbilityStore.LookupEntry(spellId))
        PSendSysMessage("Skill Id: %u", pAbility->skillId);

    SpellLearnSpellMap::const_iterator spell_begin = sSpellMgr.GetBeginSpellLearnSpell(spellId); // spell_learn_spell
    SpellLearnSpellMap::const_iterator spell_end   = sSpellMgr.GetEndSpellLearnSpell(spellId);
    for (SpellLearnSpellMap::const_iterator itr2 = spell_begin; itr2 != spell_end; ++itr2)
        PSendSysMessage("Spell Learned Id: %u", itr2->second.spell);
    
    if (spellInfo->AttributesCu & SPELL_ATTR_CU_LINK_HIT)
    {
        if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(spellInfo->Id + SPELL_LINK_HIT))
            for (std::vector<int32>::const_iterator i = spell_triggered->begin(); i != spell_triggered->end(); ++i)
                if (*i < 0)
                    PSendSysMessage("Removes %u on spell hit", (-(*i)));
                else
                    PSendSysMessage("Casts %u on spell hit", *i);
    }
    if (spellInfo->AttributesCu & SPELL_ATTR_CU_LINK_CAST)
    {
        if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(spellInfo->Id))
            for (std::vector<int32>::const_iterator i = spell_triggered->begin(); i != spell_triggered->end(); ++i)
                if (*i < 0)
                    PSendSysMessage("Removes %u on spell cast end", (-(*i)));
                else
                    PSendSysMessage("Casts %u on spell cast end", *i);
    }
    if (spellInfo->AttributesCu & SPELL_ATTR_CU_LINK_AURA)
    {
        if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(spellId + SPELL_LINK_AURA))
            for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                if (*itr < 0)
                    PSendSysMessage("Applies Immunity to %u on aura affect", (-(*itr)));
                else
                    PSendSysMessage("Removes %u on aura affect", *itr);
    }
    if (spellInfo->AttributesCu & SPELL_ATRR_CU_LINK_PRECAST)
        if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(spellInfo->Id + SPELL_LINK_PRECAST))
            for (std::vector<int32>::const_iterator i = spell_triggered->begin(); i != spell_triggered->end(); ++i)
                PSendSysMessage("Casted %u on spell cast begin", *i);
    if (spellInfo->AttributesCu & SPELL_ATTR_CU_LINK_REMOVE)
        {
            if (const std::vector<int32> *spell_triggered = sSpellMgr.GetSpellLinked(-(int32)spellId))
                for (std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    if (*itr < 0)
                        PSendSysMessage("Removes %u on aura remove", (-(*itr)));
                    else
                        PSendSysMessage("Casts %u on aura remove", *itr);
        }
    return true;
}

//bool ChatHandler::HandleItemLevelCheckCommand(const char *args)
//{
//    if (!*args)
//        return false;
//
//    uint32 itemId = 0;
//
//    char* cId = extractKeyFromLink((char*)args,"Hitem");
//    if (!cId)
//        return false;
//    itemId = atol(cId);
//
//    ItemPrototype const *ItemProto = ObjectMgr::GetItemPrototype(itemId);
//    if (!ItemProto)
//    {
//        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
//        return false;
//    }
//    if (ItemProto->Quality != 5 && ItemProto->Quality != 6)
//    {
//        SendSysMessage("This command only supports legendary items.");
//        return false;
//    }
//
//    float LegItemLevelHas = 0;
//    uint16 LegItemLevelNeeded = 0; // What level should have this item
//    bool PhysItem = false;
//    switch (ItemProto->InventoryType)
//    {
//        case INVTYPE_HOLDABLE:
//        case INVTYPE_SHIELD:
//            LegItemLevelNeeded = 190;
//            break;
//        case INVTYPE_HEAD:
//            LegItemLevelNeeded = 312;
//            break;
//        case INVTYPE_NECK:
//            LegItemLevelNeeded = 203;
//            break;
//        case INVTYPE_SHOULDERS:
//            LegItemLevelNeeded = 227;
//            break;
//        case INVTYPE_CHEST:
//        case INVTYPE_ROBE:
//            LegItemLevelNeeded = 303;
//            break;
//        case INVTYPE_WAIST:
//        case INVTYPE_FEET:
//        case INVTYPE_HANDS:
//            LegItemLevelNeeded = 231;
//            break;
//        case INVTYPE_LEGS:
//            LegItemLevelNeeded = 292;
//            break;
//        case INVTYPE_WRISTS:
//            LegItemLevelNeeded = 161;
//            break;
//        case INVTYPE_FINGER:
//            LegItemLevelNeeded = 184;
//            break;
//        case INVTYPE_TRINKET:
//            LegItemLevelNeeded = 160;
//            break;
//        case INVTYPE_WEAPON:
//        case INVTYPE_WEAPONMAINHAND:
//        {
//            LegItemLevelNeeded = 524;
//            for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
//                if (((ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin) * 500 / ItemProto->Delay) > 100) // Phys weapons 1h have more than 100 DPS - really ~150
//                {
//                    PhysItem = true;
//                    break;
//                }
//            if (PhysItem)
//                LegItemLevelNeeded += 173;
//            break;
//        }
//        case INVTYPE_CLOAK:
//            LegItemLevelNeeded = 196;
//            break;
//        case INVTYPE_2HWEAPON: // Feral staff counted as SPD
//        {
//            LegItemLevelNeeded = 747;
//            for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
//                if (((ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin) * 500 / ItemProto->Delay) > 150) // Phys weapons 2h have more than 150 DPS - really ~220, most SPD DPS staff is feral staff - 115 dps
//                {
//                    PhysItem = true;
//                    break;
//                }
//            if (PhysItem)
//                LegItemLevelNeeded += 317;
//            break;
//        }
//        case INVTYPE_RELIC:
//            LegItemLevelNeeded = 120;
//            break;
//        case INVTYPE_RANGEDRIGHT:
//        case INVTYPE_RANGED:
//        case INVTYPE_THROWN:
//            LegItemLevelNeeded = 573;
//            if (ItemProto->SubClass == ITEM_SUBCLASS_WEAPON_WAND)
//                LegItemLevelNeeded += 477;
//            PhysItem = true;
//            break;
//        default:
//            PSendSysMessage("Unknown itemInventoryType: %u", ItemProto->InventoryType);
//            return false;
//    }
//    PSendSysMessage("Legendary item of this type should have %u ItemLevel.", LegItemLevelNeeded);
//    // All item stats
//    for (int i = 0; i < MAX_ITEM_PROTO_STATS; i++)
//    {   
//        float Value = float(ItemProto->ItemStat[i].ItemStatValue);
//        switch (ItemProto->ItemStat[i].ItemStatType)
//        {
//            case ITEM_MOD_MANA:
//            case ITEM_MOD_HEALTH:
//                if (Value != 0)
//                    SendSysMessage("Stats health and mana are not counted as stats cause they're used as base stat bonuses");
//                continue;
//            case ITEM_MOD_AGILITY:
//            case ITEM_MOD_STRENGTH:
//            case ITEM_MOD_INTELLECT:
//            case ITEM_MOD_SPIRIT:
//            case ITEM_MOD_DEFENSE_SKILL_RATING:
//            case ITEM_MOD_DODGE_RATING:
//            case ITEM_MOD_PARRY_RATING:
//            case ITEM_MOD_BLOCK_RATING:
//            case ITEM_MOD_CRIT_MELEE_RATING: // Almost same as crit_rating, used to make item more unique-used
//            case ITEM_MOD_CRIT_RANGED_RATING: // Almost same as crit_rating, used to make item more unique-used
//            case ITEM_MOD_CRIT_SPELL_RATING: // spell_Crit
//            case ITEM_MOD_CRIT_RATING: // Melee + Ranged crit rating
//            case ITEM_MOD_RESILIENCE_RATING: // Resilience
//            case ITEM_MOD_HASTE_MELEE_RATING: // Almost same as normal haste, used to make item more unique-used
//            case ITEM_MOD_HASTE_RANGED_RATING: // Almost same as normal haste, used to make item more unique-used
//            case ITEM_MOD_HASTE_SPELL_RATING: // Spell_Haste
//            case ITEM_MOD_HASTE_RATING: // Melee + Ranged haste rating
//            case ITEM_MOD_HIT_MELEE_RATING: // Almost same as hit_rating, used to make item more unique-used
//            case ITEM_MOD_HIT_RANGED_RATING: // Almost same as hit_rating, used to make item more unique-used
//            case ITEM_MOD_HIT_SPELL_RATING: // spell_hit
//            case ITEM_MOD_HIT_RATING: // Melee + Ranged hit rating
//                LegItemLevelHas += Value; // These 4 stats are 1 to 1 the same
//                continue;
//            case ITEM_MOD_STAMINA:
//                LegItemLevelHas += Value/1.5f; // Stamina is /1.5
//                continue;
//            case ITEM_MOD_EXPERTISE_RATING: // Melee expertise rating
//                LegItemLevelHas += Value * 1.6f; // These stats are legendary-non-reducable - gained by % - so increase their "cost"
//                continue;
//            case ITEM_MOD_HIT_TAKEN_MELEE_RATING: // ???
//            case ITEM_MOD_HIT_TAKEN_RANGED_RATING: // ???
//            case ITEM_MOD_HIT_TAKEN_SPELL_RATING: // ???
//            case ITEM_MOD_CRIT_TAKEN_MELEE_RATING: // ???
//            case ITEM_MOD_CRIT_TAKEN_RANGED_RATING: // ???
//            case ITEM_MOD_CRIT_TAKEN_SPELL_RATING: // ???
//            case ITEM_MOD_HIT_TAKEN_RATING: // ???
//            case ITEM_MOD_CRIT_TAKEN_RATING: // ???
//                PSendSysMessage("Unused stat %u.", ItemProto->ItemStat[i].ItemStatType);
//                continue;
//            default:
//                PSendSysMessage("Unknown stat %u.", ItemProto->ItemStat[i].ItemStatType);
//                continue;
//        }        
//    }
//    PSendSysMessage("Item Stats included. (%f)", LegItemLevelHas);
//    // Sockets
//    for (int i = 0; i < MAX_ITEM_PROTO_SOCKETS; i++)
//    {
//        switch (ItemProto->Socket[i].Color)
//        {
//            case SOCKET_COLOR_META:
//                LegItemLevelHas += 32.0f;
//                continue;
//            case SOCKET_COLOR_RED:
//            case SOCKET_COLOR_YELLOW:
//            case SOCKET_COLOR_BLUE:
//                LegItemLevelHas += 10.0f;
//                continue;
//        }
//    }
//    PSendSysMessage("Sockets included. (%f)", LegItemLevelHas);
//    // Socket bonus
//    if (ItemProto->socketBonus)
//    {
//        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(ItemProto->socketBonus);
//        if (enchantEntry)
//        {
//            for (int i = 0; i < 3; i++)
//            {
//                switch (enchantEntry->type[i])
//                {
//                    case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
//                    {
//                        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(enchantEntry->spellid[i]);
//                        if (!spellInfo)
//                        {
//                            if (enchantEntry->spellid[i] != 0)
//                                SendSysMessage("Socket bonus has spell but database has no such spell.");
//                            continue;
//                        }
//                        else
//                        {
//                            PSendSysMessage("Socket Bonus spellID %u, SpellName %s.", spellInfo->Id, spellInfo->SpellName[0]);
//                            for (int j = 0; j < 3; j++)
//                            {
//                                if (spellInfo->Effect[j] == SPELL_EFFECT_APPLY_AURA)
//                                {
//                                    switch (spellInfo->EffectApplyAuraName[j])
//                                    {
//                                        case SPELL_AURA_MOD_HEALING_DONE: // normal heal + 1/3 damage
//                                        {
//                                            if (j == 0)
//                                            {
//                                                LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 2.2f;
//                                                SendSysMessage("Socket bonus has healing done spell.");
//                                            }
//                                            continue;
//                                        }
//                                        case SPELL_AURA_MOD_DAMAGE_DONE: // Both SPD and like Frost_SPD
//                                        {
//                                            if (j == 0)
//                                            {
//                                                if (spellInfo->EffectMiscValue[j] == 126)
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 1.2f;
//                                                else // special ONE type only SPD
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 1.44f;
//                                                SendSysMessage("Socket bonus has spell damage done spell.");
//                                            }
//                                            continue;
//                                        }
//                                        // If only melee or only ranged - /1.1f
//                                        case SPELL_AURA_MOD_ATTACK_POWER:
//                                        {
//                                            if (spellInfo->HasApplyAura(SPELL_AURA_MOD_RANGED_ATTACK_POWER))
//                                                LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 4.0f; 
//                                            else
//                                            {
//                                                if (spellInfo->Stances)
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 5.3f; // feral / 5.3
//                                                else
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 2.2f;
//                                            }
//                                            continue;
//                                        }
//                                        // If only melee or only ranged - /1.1f
//                                        case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
//                                        {
//                                            if (spellInfo->HasApplyAura(SPELL_AURA_MOD_ATTACK_POWER))
//                                                LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 4.0f;
//                                            else
//                                                LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 2.2f;
//                                            continue;
//                                        }
//                                        case SPELL_AURA_MOD_TARGET_RESISTANCE:
//                                        {
//                                            if (spellInfo->EffectBasePoints[j] + 1 < 0)
//                                            {
//                                                if (spellInfo->EffectMiscValue[j] == 1) // armor ignore
//                                                    LegItemLevelHas += abs(float(spellInfo->EffectBasePoints[j] + 1) / 3.08f);
//                                                else if (spellInfo->EffectMiscValue[j] == 124) // spell penetration
//                                                    LegItemLevelHas += abs(float(spellInfo->EffectBasePoints[j] + 1) / 0.8125f);
//                                            }
//                                            else
//                                                SendSysMessage("Unsupported SPELL_AURA_MOD_RESISTANCE case.");
//                                            continue;                           
//                                        }
//                                        case SPELL_AURA_MOD_POWER_REGEN:
//                                        {
//                                            if (spellInfo->EffectMiscValue[j] == 0)
//                                                LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 0.4f;
//                                            else
//                                                SendSysMessage("Unsupported SPELL_AURA_MOD_POWER_REGEN case.");
//                                            continue;
//                                        }
//                                        case SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT:
//                                        {
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 0.6f;
//                                            continue;
//                                        }
//                                        case SPELL_AURA_MOD_RESISTANCE:
//                                        {
//                                            if (spellInfo->EffectMiscValue[j] == 1) // armor
//                                                LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 8.75f;
//                                            else
//                                                SendSysMessage("Socket Bonus Spell has applyaura SPELL_AURA_MOD_RESISTANCE (resistance)");
//                                            continue;
//                                        }
//                                        case SPELL_AURA_MOD_SHIELD_BLOCKVALUE:
//                                        {
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 1.5f;
//                                            continue;
//                                        }
//                                        case SPELL_AURA_MOD_RATING:
//                                        {
//                                            switch (spellInfo->EffectMiscValue[j])
//                                            {
//                                                case CR_DEFENSE_SKILL:
//                                                case CR_DODGE:
//                                                case CR_PARRY:
//                                                case CR_BLOCK:
//                                                case CR_CRIT_SPELL:
//                                                case CR_HASTE_SPELL:
//                                                case CR_HIT_MELEE:
//                                                case CR_HIT_RANGED:
//                                                case CR_HIT_SPELL:
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1);
//                                                    continue;
//                                                case CR_EXPERTISE:
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) * 1.6f;
//                                                    continue;                                    
//                                                default:
//                                                    PSendSysMessage("Spell has aura SPELL_AURA_MOD_RATING with unsupported rating %u", spellInfo->EffectMiscValue[j]);
//                                                    continue;
//                                            }
//                                            continue;
//                                        }
//                                        case SPELL_AURA_MOD_STAT:
//                                        {
//                                            switch (spellInfo->EffectMiscValue[j])
//                                            {
//                                                case UNIT_MOD_STAT_STRENGTH:
//                                                case UNIT_MOD_STAT_AGILITY:
//                                                case UNIT_MOD_STAT_INTELLECT:
//                                                case UNIT_MOD_STAT_SPIRIT:
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1);
//                                                    continue;
//                                                case UNIT_MOD_STAT_STAMINA:
//                                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 1.5f;
//                                                    continue;
//                                                default:
//                                                    PSendSysMessage("Socket Bonus: Unknown stat %u for SPELL_AURA_MOD_STAT", spellInfo->EffectMiscValue[j]);
//                                                    continue;
//                                            }     
//                                            continue;
//                                        }
//                                        case SPELL_AURA_NONE:
//                                        {
//                                            SendSysMessage("Socket Bonus Spell has effect applyaura but there's no aura to apply");
//                                            continue;
//                                        }
//                                        default:
//                                        {
//                                            SendSysMessage("Socket bonus has unsupported spell.");
//                                            continue;
//                                        }
//                                    }
//                                }
//                                else if (spellInfo->Effect[j] != 0)
//                                {
//                                    SendSysMessage("Socket bonus has spell but it's not aura.");
//                                    continue;
//                                }
//                            }
//                        }
//                        continue;
//                    }
//                    case ITEM_ENCHANTMENT_TYPE_STAT:
//                    {
//                        float BonusValue = float(enchantEntry->amount[i]);
//                        switch (enchantEntry->spellid[i]) // it is stat type
//                        {
//                            case ITEM_MOD_MANA:
//                            case ITEM_MOD_HEALTH:
//                                if (BonusValue != 0)
//                                    SendSysMessage("Stats health and mana are not counted as stats cause they're used as base stat bonuses(Socket Bonus).");
//                                continue;
//                            case ITEM_MOD_AGILITY:
//                            case ITEM_MOD_STRENGTH:
//                            case ITEM_MOD_INTELLECT:
//                            case ITEM_MOD_SPIRIT:
//                            case ITEM_MOD_DEFENSE_SKILL_RATING:
//                            case ITEM_MOD_DODGE_RATING:
//                            case ITEM_MOD_PARRY_RATING:
//                            case ITEM_MOD_BLOCK_RATING:
//                            case ITEM_MOD_CRIT_SPELL_RATING: // spell_Crit
//                            case ITEM_MOD_CRIT_RATING: // Melee + Ranged crit rating
//                            case ITEM_MOD_RESILIENCE_RATING: // Resilience
//                            case ITEM_MOD_HASTE_MELEE_RATING: // Almost same as normal haste, used to make item more unique-used
//                            case ITEM_MOD_HASTE_RANGED_RATING: // Almost same as normal haste, used to make item more unique-used
//                            case ITEM_MOD_HASTE_SPELL_RATING: // Spell_Haste
//                            case ITEM_MOD_HASTE_RATING: // Melee + Ranged haste rating
//                            case ITEM_MOD_HIT_MELEE_RATING: // Almost same as hit_rating, used to make item more unique-used
//                            case ITEM_MOD_HIT_RANGED_RATING: // Almost same as hit_rating, used to make item more unique-used
//                            case ITEM_MOD_HIT_SPELL_RATING: // spell_hit
//                            case ITEM_MOD_HIT_RATING: // Melee + Ranged hit rating
//                                LegItemLevelHas += BonusValue; // These 4 stats are 1 to 1 the same
//                                continue;
//                            case ITEM_MOD_CRIT_MELEE_RATING: // Almost same as crit_rating, used to make item more unique-used
//                            {
//                                bool BothCrit = false;
//                                for (int k = 0; k < 3; k ++)
//                                {
//                                    if (enchantEntry->spellid[i] == ITEM_MOD_CRIT_RANGED_RATING)
//                                    {
//                                        BothCrit = true;
//                                        break;
//                                    }
//                                }
//                                if (BothCrit)
//                                    LegItemLevelHas += BonusValue / 2.0f;
//                                else
//                                    LegItemLevelHas += BonusValue / 1.1f;
//                                continue;
//                            }
//                            case ITEM_MOD_CRIT_RANGED_RATING: // Almost same as crit_rating, used to make item more unique-used
//                            {
//                                bool BothCrit = false;
//                                for (int k = 0; k < 3; k ++)
//                                {
//                                    if (enchantEntry->spellid[i] == ITEM_MOD_CRIT_MELEE_RATING)
//                                    {
//                                        BothCrit = true;
//                                        break;
//                                    }
//                                }
//                                if (BothCrit)
//                                    LegItemLevelHas += BonusValue / 2.0f;
//                                else
//                                    LegItemLevelHas += BonusValue / 1.1f;
//                                continue;
//                            }
//                            case ITEM_MOD_STAMINA:
//                                LegItemLevelHas += BonusValue/1.5f; // Stamina is /1.5
//                                continue;
//                            case ITEM_MOD_EXPERTISE_RATING: // Melee expertise rating
//                                LegItemLevelHas += BonusValue * 1.6f; // These stats are legendary-non-reducable - gained by % - so increase their "cost"
//                                continue;
//                            case ITEM_MOD_HIT_TAKEN_MELEE_RATING: // ???
//                            case ITEM_MOD_HIT_TAKEN_RANGED_RATING: // ???
//                            case ITEM_MOD_HIT_TAKEN_SPELL_RATING: // ???
//                            case ITEM_MOD_CRIT_TAKEN_MELEE_RATING: // ???
//                            case ITEM_MOD_CRIT_TAKEN_RANGED_RATING: // ???
//                            case ITEM_MOD_CRIT_TAKEN_SPELL_RATING: // ???
//                            case ITEM_MOD_HIT_TAKEN_RATING: // ???
//                            case ITEM_MOD_CRIT_TAKEN_RATING: // ???
//                                PSendSysMessage("Unused stat for Socket Bonus %u.", enchantEntry->spellid[i]);
//                                continue;
//                            default:
//                                PSendSysMessage("Unknown stat for Socket Bonus %u.", enchantEntry->spellid[i]);
//                                continue;
//                        }
//                        continue;
//                    }
//                    case ITEM_ENCHANTMENT_TYPE_NONE: // no enchant
//                        continue;
//                    default:
//                        SendSysMessage("Socket bonus not supported to calculate.");
//                        continue;
//                }
//            }
//        }
//        else
//            SendSysMessage("Socket bonus doesn't exist.");    
//    }
//    else
//        SendSysMessage("Item has no socket bonus.");     
//    PSendSysMessage("Socket bonus Included. (%f)", LegItemLevelHas);
//    // Resistances // 1.6f * 0.5f = 0.8f
//    float Resistance = float(ItemProto->FireRes);
//    LegItemLevelHas += 0.8f * Resistance;
//    Resistance = float (ItemProto->ArcaneRes);
//    LegItemLevelHas += 0.8f * Resistance;
//    Resistance = float (ItemProto->ShadowRes);
//    LegItemLevelHas += 0.8f * Resistance;
//    Resistance = float (ItemProto->FrostRes);
//    LegItemLevelHas += 0.8f * Resistance;
//    Resistance = float (ItemProto->NatureRes);
//    LegItemLevelHas += 0.8f * Resistance;
//    PSendSysMessage("Resistances included. (%f)", LegItemLevelHas);
//    // end of resistances
//    if (PhysItem)
//    {
//        for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
//        {
//            if (ItemProto->Damage[i].DamageMax != 0 && ItemProto->Damage[i].DamageMin != 0 && ItemProto->Damage[i].DamageMax > ItemProto->Damage[i].DamageMin)
//                LegItemLevelHas += float((ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin) * 1492.5f / float(ItemProto->Delay));
//                // LegItemLevelHas += (ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin)  / 2 / 0.335f / ItemProto->Delay;
//        }
//        if (ItemProto->AmmoType == 2 || ItemProto->AmmoType == 3)
//            LegItemLevelHas -= 31.8f / 0.335f;
//        PSendSysMessage("Damage included. (%f)", LegItemLevelHas);
//    }
//    switch (ItemProto->SubClass)
//    {
//        case ITEM_SUBCLASS_ARMOR_CLOTH:
//        case ITEM_SUBCLASS_ARMOR_LEATHER:
//        case ITEM_SUBCLASS_ARMOR_MAIL:
//        case ITEM_SUBCLASS_ARMOR_PLATE:
//        case ITEM_SUBCLASS_ARMOR_SHIELD:
//            SendSysMessage("Armor NOT included. (Armor for these items should be static and not changed).");
//            break;
//        default:
//        {
//            if (ItemProto->Armor != 0)
//            {
//                LegItemLevelHas += float(ItemProto->Armor)/8.75f;
//                PSendSysMessage("Armor included. (%f)", LegItemLevelHas);
//            }
//            break;
//        }
//    }
//    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
//    {
//        if (ItemProto->Spells[i].SpellId != 0)
//        {
//            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(ItemProto->Spells[i].SpellId);
//            if (spellInfo)
//            {
//                PSendSysMessage("Spell #%u spellID %u, SpellName %s.", i+1, spellInfo->Id, spellInfo->SpellName[0]);
//                if (ItemProto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_EQUIP)
//                {
//                    for (int j = 0; j < 3; j++)
//                    {
//                        if (spellInfo->Effect[j] == SPELL_EFFECT_APPLY_AURA)
//                        {
//                            switch (spellInfo->EffectApplyAuraName[j])
//                            {
//                                case SPELL_AURA_MOD_HEALING_DONE: // normal heal + 1/3 damage
//                                {
//                                    if (j == 0)
//                                    {
//                                        LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 2.2f;
//                                        SendSysMessage("Spell has healing done spell.");
//                                    }
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_DAMAGE_DONE: // Both SPD and like Frost_SPD
//                                {
//                                    if (j == 0)
//                                    {
//                                        if (spellInfo->EffectMiscValue[j] == 126)
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 1.2f;
//                                        else // special ONE type only SPD
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 1.44f;
//                                        SendSysMessage("Spell has spell damage done spell.");
//                                    }
//                                    continue;
//                                }
//                                // If only melee or only ranged - /1.1f
//                                case SPELL_AURA_MOD_ATTACK_POWER:
//                                {
//                                    if (spellInfo->HasApplyAura(SPELL_AURA_MOD_RANGED_ATTACK_POWER))
//                                        LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 4.0f;
//                                    else
//                                        LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 2.2f;
//                                    continue;
//                                }
//                                // If only melee or only ranged - /1.1f
//                                case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
//                                {
//                                    if (spellInfo->HasApplyAura(SPELL_AURA_MOD_ATTACK_POWER))
//                                        LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 4.0f;
//                                    else
//                                    {
//                                        if (spellInfo->Stances)
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 5.3f; // feral / 5.3
//                                        else
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 2.2f;
//                                    }
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_TARGET_RESISTANCE:
//                                {
//                                    if (spellInfo->EffectBasePoints[j] + 1 < 0)
//                                    {
//                                        if (spellInfo->EffectMiscValue[j] == 1) // armor ignore
//                                            LegItemLevelHas += abs(float(spellInfo->EffectBasePoints[j] + 1) / 3.08f);
//                                        else if (spellInfo->EffectMiscValue[j] == 124) // spell penetration
//                                            LegItemLevelHas += abs(float(spellInfo->EffectBasePoints[j] + 1) / 0.8125f);
//                                    }
//                                    else
//                                        SendSysMessage("Unsupported SPELL_AURA_MOD_RESISTANCE case.");
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_POWER_REGEN:
//                                {
//                                    if (spellInfo->EffectMiscValue[j] == 0)
//                                        LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 0.4f;
//                                    else
//                                        SendSysMessage("Unsupported SPELL_AURA_MOD_POWER_REGEN case.");
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT:
//                                {
//                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 0.6f;
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_RESISTANCE:
//                                {
//                                    SendSysMessage("Spell has applyaura SPELL_AURA_MOD_RESISTANCE, but it shoule be added to item itself(armor/resistance).");
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_SHIELD_BLOCKVALUE:
//                                {
//                                    LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) / 1.5f;
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_RATING:
//                                {
//                                    switch (spellInfo->EffectMiscValue[j])
//                                    {
//                                        case CR_DEFENSE_SKILL:
//                                        case CR_DODGE:
//                                        case CR_PARRY:
//                                        case CR_BLOCK:
//                                        case CR_CRIT_SPELL:
//                                        case CR_HASTE_SPELL:
//                                        case CR_HIT_MELEE:
//                                        case CR_HIT_RANGED:
//                                        case CR_HIT_SPELL:
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1);
//                                            continue;
//                                        case CR_EXPERTISE:
//                                            LegItemLevelHas += float(spellInfo->EffectBasePoints[j] + 1) * 1.6f;
//                                            continue;                                    
//                                        default:
//                                            PSendSysMessage("Spell has aura SPELL_AURA_MOD_RATING with unsupported rating %u.", spellInfo->EffectMiscValue[j]);
//                                            continue;
//                                    }
//                                    continue;
//                                }
//                                case SPELL_AURA_MOD_STAT:
//                                {
//                                    SendSysMessage("Spell has applyaura SPELL_AURA_MOD_STAT(used for normal stats only) - they should be added to item itself, not spell.");
//                                    continue;
//                                }
//                                case SPELL_AURA_NONE:
//                                {
//                                    SendSysMessage("Spell has effect applyaura but there's no aura to apply.");
//                                    continue;
//                                }
//                                default:
//                                {
//                                    PSendSysMessage("Spell has unsupported aura. SpellID: %u, Name: %s.", spellInfo->Id, spellInfo->SpellName[0]);
//                                    continue;
//                                }
//                            }
//                        }
//                        else if (spellInfo->Effect[j] != 0)
//                        {
//                            SendSysMessage("Spell has spell but it's not aura.");
//                            continue;
//                        }
//                    }
//                    continue;
//                }
//                else if (ItemProto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
//                {
//                    SendSysMessage("Spells on use not yet supported.");
//                    continue;
//                }
//                else if (ItemProto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_CHANCE_ON_HIT)
//                {
//                    if (ItemProto->Spells[i].SpellPPMRate > 0.02f)
//                        SendSysMessage("Spells chance on hit are NOT supported (And won't be supported).");
//                    continue;
//                }
//            }
//            else
//            {
//                PSendSysMessage("Spell %u doesn't exist.", i+1);
//                continue;
//            }   
//        }
//        else
//            continue;
//    }
//    PSendSysMessage("Spells included. (%f)", LegItemLevelHas);
//    PSendSysMessage("ItemLevel: %f.", LegItemLevelHas);
//    return true;
//}

bool ChatHandler::HandleLosTestCommand(const char* args)
{
    static const uint32 WAYPOINT_NPC_ENTRY = 1;
    float everydist = 2.0f;
    float xStart = m_session->GetPlayer()->GetPositionX();
    float yStart = m_session->GetPlayer()->GetPositionY();
    float thisZ = m_session->GetPlayer()->GetPositionZ();
    float maxdistX = xStart + everydist*10;
    float maxdistY = yStart + everydist*10;
    for (; xStart <= maxdistX; xStart+=everydist)
    {
        for (float ytest = yStart; ytest <= maxdistY; ytest+=everydist)
        {
            float ground = m_session->GetPlayer()->GetTerrain()->GetHeight(xStart, ytest, MAX_HEIGHT, true);
            float floor = m_session->GetPlayer()->GetTerrain()->GetHeight(xStart, ytest, thisZ, true);
            float destz = fabs(ground - thisZ) <= fabs(floor - thisZ) ? ground : floor;
                 
            if (Unit* wp = m_session->GetPlayer()->SummonCreature(WAYPOINT_NPC_ENTRY, xStart, ytest, destz, 0, TEMPSUMMON_TIMED_DESPAWN, 9000))
                wp->CastSpell(wp, 23126, true);
        }
    }
    return true;
}

//set faction of creature
bool ChatHandler::HandleNpcFactionIdCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 factionId = (uint32) atoi((char*)args);

    if (!sFactionTemplateStore.LookupEntry(factionId))
    {
        PSendSysMessage(LANG_WRONG_FACTION, factionId);
        SetSentErrorMessage(true);
        return false;
    }

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->setFaction(factionId);

    // faction is set in creature_template - not inside creature

    // update in memory
    if (CreatureInfo const *cinfo = pCreature->GetCreatureInfo())
    {
        const_cast<CreatureInfo*>(cinfo)->faction_A = factionId;
        const_cast<CreatureInfo*>(cinfo)->faction_H = factionId;
    }

    // and DB
    GameDataDatabase.PExecuteLog("UPDATE creature_template SET faction_A = '%u', faction_H = '%u' WHERE entry = '%u'", factionId, factionId, pCreature->GetEntry());

    return true;
}

//kick player
bool ChatHandler::HandleKickPlayerCommand(const char *args)
{
    const char* kickName = strtok((char*)args, " ");
    char* kickReason = strtok(NULL, "\n");
    std::string reason = "No Reason";
    std::string kicker = "Console";
    if (kickReason)
        reason = kickReason;
    if (m_session)
        kicker = m_session->GetPlayer()->GetName();

    if (!kickName)
     {
        Player* player = getSelectedPlayer();
        if (!player)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            SetSentErrorMessage(true);
            return false;
        }

        if (m_session && (player == m_session->GetPlayer() || player->GetSession()->GetPermissions() > m_session->GetPermissions()))
        {
            SendSysMessage(LANG_COMMAND_KICKSELF);
            SetSentErrorMessage(true);
            return false;
        }

        if (sWorld.getConfig(CONFIG_SHOW_KICK_IN_WORLD))
            sWorld.SendWorldText(LANG_COMMAND_KICKMESSAGE, 0, player->GetName(), kicker.c_str(), reason.c_str());
        else
            PSendSysMessage(LANG_COMMAND_KICKMESSAGE, player->GetName(), kicker.c_str(), reason.c_str());

        player->GetSession()->KickPlayer();
    }
    else
    {
        std::string name = kickName;
        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        if (m_session && name==m_session->GetPlayer()->GetName())
        {
            SendSysMessage(LANG_COMMAND_KICKSELF);
            SetSentErrorMessage(true);
            return false;
        }

        Player* player = sObjectMgr.GetPlayerInWorld(name.c_str());
        if (!player)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        if (m_session && player->GetSession()->GetPermissions() > m_session->GetPermissions())
        {
            SendSysMessage(LANG_YOURS_SECURITY_IS_LOW); //maybe replacement string for this later on
            SetSentErrorMessage(true);
            return false;
        }

        if (sWorld.KickPlayer(name.c_str()))
        {
            if (sWorld.getConfig(CONFIG_SHOW_KICK_IN_WORLD))
                sWorld.SendWorldText(LANG_COMMAND_KICKMESSAGE, 0, name.c_str(), kicker.c_str(), reason.c_str());
            else
                PSendSysMessage(LANG_COMMAND_KICKMESSAGE, name.c_str(), kicker.c_str(), reason.c_str());
        }
        else
        {
            PSendSysMessage(LANG_COMMAND_KICKNOTFOUNDPLAYER, name.c_str());
            return false;
        }
    }
    return true;
}

//show info of player
//show info of player
bool ChatHandler::HandlePInfoCommand(const char* args)
{
    Player* target = NULL;
    uint64 targetGUID = 0;

    char* px = strtok((char*)args, " ");
    char* py = NULL;

    std::string name;

    if (px)
    {
        name = px;

        if (name.empty())
            return false;

        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        target = sObjectMgr.GetPlayerInWorld(name.c_str());
        if (target)
            py = strtok(NULL, " ");
        else
        {
            targetGUID = sObjectMgr.GetPlayerGUIDByName(name);
            if (targetGUID)
                py = strtok(NULL, " ");
            else
                py = px;
        }
    }

    if (!target && !targetGUID)
    {
        target = getSelectedPlayer();
    }

    if (!target && !targetGUID)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accId = 0;
    uint32 money = 0;
    uint32 total_player_time = 0;
    uint32 level = 0;
    uint32 latency = 0;
    uint8 race;
    uint8 Class;
    bool isRafAcc = false;
    bool hasDynRafFlag = false;
    uint32 grantableLevels = 0;

    // get additional information from Player object
    if (target && target->GetSession()->HasPermissions(PERM_GMT) &&
        (m_session && !m_session->HasPermissions(sWorld.getConfig(CONFIG_GM_TRUSTED_LEVEL))))
        return false;
    if (target)
    {
        targetGUID = target->GetGUID();
        name = target->GetName();                           // re-read for case getSelectedPlayer() target
        accId = target->GetSession()->GetAccountId();
        money = target->GetMoney();
        total_player_time = target->GetTotalPlayedTime();
        level = target->GetLevel();
        latency = target->GetSession()->GetLatency();
        race = target->GetRace();
        Class = target->GetClass();
        hasDynRafFlag = target->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_REFER_A_FRIEND);
        grantableLevels = target->GetGrantableLevels();
    }
    // get additional information from DB
    else
    {
        QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT totaltime, level, money, account, race, class, grantableLevels FROM characters WHERE guid = '%u'", GUID_LOPART(targetGUID));
        if (!result)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
        Field *fields = result->Fetch();
        total_player_time = fields[0].GetUInt32();
        level = fields[1].GetUInt32();
        money = fields[2].GetUInt32();
        accId = fields[3].GetUInt32();
        race = fields[4].GetUInt8();
        Class = fields[5].GetUInt8();
        grantableLevels = fields[6].GetUInt8();
    }

    std::string username = GetHellgroundString(LANG_ERROR);
    std::string email = GetHellgroundString(LANG_ERROR);
    std::string last_ip = GetHellgroundString(LANG_ERROR);
    uint32 permissions = 0;
    std::string last_login = GetHellgroundString(LANG_ERROR);

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT a.username,ap.mask,a.email,a.last_ip,a.last_login,a.account_flags "
                                                        "FROM account a "
                                                        "LEFT JOIN account_gm ap "
                                                        "ON (a.account_id = ap.account_id) "
                                                        "WHERE a.account_id = '%u'",accId);
    

    if (result)
    {
        Field* fields = result->Fetch();
        username = fields[0].GetCppString();
        permissions = fields[1].GetUInt32() ? fields[1].GetUInt32() : 1;
        if ((permissions & PERM_GMT) && m_session && !m_session->HasPermissions(sWorld.getConfig(CONFIG_GM_TRUSTED_LEVEL)))
            return false;

        if (email.empty())
            email = "-";
        
        last_ip = fields[3].GetCppString();
        last_login = fields[4].GetCppString();
        
        if (!m_session || m_session->HasPermissions(sWorld.getConfig(CONFIG_GM_TRUSTED_LEVEL)))
            email = fields[2].GetCppString();
        else
            email = "NO_PERMISSION";

        isRafAcc = fields[5].GetUInt32() & ACC_WEB_RAF_CONNECTED;
    }

    // get guild info
    uint32 guild_id = 0;
    const char* guild_name = "";

    QueryResultAutoPtr result1 = RealmDataDatabase.PQuery("select a.guildid,a.name from guild a, guild_member b where a.guildid=b.guildid and b.guid = '%u'", GUID_LOPART(targetGUID));
    if (result1)
    {
        Field* fields = result1->Fetch();
        guild_id = fields[0].GetUInt32();
        guild_name = fields[1].GetString();
    }

    PSendSysMessage(LANG_PINFO_ACCOUNT, (target ? "" : GetHellgroundString(LANG_OFFLINE)), GetNameLink(name).c_str(), GUID_LOPART(targetGUID), username.c_str(), accId, email.c_str(), permissions, last_ip.c_str(), last_login.c_str(), latency, guild_name, guild_id);

    if (target)
    {
        PSendSysMessage("IsHealer %u (semi %u)", target->IsHealer(), target->IsSemiHealer());
    }

    std::string race_s, Class_s;
    switch(race)
    {
        case RACE_HUMAN:            race_s = "Human";       break;
        case RACE_ORC:              race_s = "Orc";         break;
        case RACE_DWARF:            race_s = "Dwarf";       break;
        case RACE_NIGHTELF:         race_s = "Night Elf";   break;
        case RACE_UNDEAD_PLAYER:    race_s = "Undead";      break;
        case RACE_TAUREN:           race_s = "Tauren";      break;
        case RACE_GNOME:            race_s = "Gnome";       break;
        case RACE_TROLL:            race_s = "Troll";       break;
        case RACE_BLOODELF:         race_s = "Blood Elf";   break;
        case RACE_DRAENEI:          race_s = "Draenei";     break;
    }

    switch(Class)
    {
        case CLASS_WARRIOR:         Class_s = "Warrior";        break;
        case CLASS_PALADIN:         Class_s = "Paladin";        break;
        case CLASS_HUNTER:          Class_s = "Hunter";         break;
        case CLASS_ROGUE:           Class_s = "Rogue";          break;
        case CLASS_PRIEST:          Class_s = "Priest";         break;
        case CLASS_SHAMAN:          Class_s = "Shaman";         break;
        case CLASS_MAGE:            Class_s = "Mage";           break;
        case CLASS_WARLOCK:         Class_s = "Warlock";        break;
        case CLASS_DRUID:           Class_s = "Druid";          break;
    }

    std::string timeStr = WorldSession::secondsToTimeString(m_session, total_player_time,true,true);
    uint32 gold = money /GOLD;
    uint32 silv = (money % GOLD) / SILVER;
    uint32 copp = (money % GOLD) % SILVER;
    PSendSysMessage(LANG_PINFO_LEVEL,  race_s.c_str(), Class_s.c_str(), timeStr.c_str(), level, gold, silv, copp);
    PSendSysMessage("Account is considered refer-a-friend connected: %u. Can grant %u levels.", isRafAcc, grantableLevels);
    if (target)
        PSendSysMessage("Character has flag of refer-a-friend applied: %u.", hasDynRafFlag);

    if (py && strncmp(py, "rep", 3) == 0)
    {
        if (!target)
        {
            // rep option not implemented for offline case
            SendSysMessage(LANG_PINFO_NO_REP);
            SetSentErrorMessage(true);
            return false;
        }

        FactionStateList const& targetFSL = target->GetReputationMgr().GetStateList();
        for (FactionStateList::const_iterator itr = targetFSL.begin(); itr != targetFSL.end(); ++itr)
        {
            FactionEntry const *factionEntry = sFactionStore.LookupEntry(itr->second.ID);
            char const* factionName = factionEntry ? factionEntry->name[m_session->GetSessionDbcLocale()] : "#Not found#";
            ReputationRank rank = target->GetReputationMgr().GetRank(factionEntry);
            std::string rankName = GetHellgroundString(ReputationRankStrIndex[rank]);
            std::ostringstream ss;
            ss << itr->second.ID << ": |cffffffff|Hfaction:" << itr->second.ID << "|h[" << factionName << "]|h|r " << rankName << "|h|r ("
                << target->GetReputationMgr().GetReputation(factionEntry) << ")";

            if (itr->second.Flags & FACTION_FLAG_VISIBLE)
                ss << GetHellgroundString(LANG_FACTION_VISIBLE);
            if (itr->second.Flags & FACTION_FLAG_AT_WAR)
                ss << GetHellgroundString(LANG_FACTION_ATWAR);
            if (itr->second.Flags & FACTION_FLAG_PEACE_FORCED)
                ss << GetHellgroundString(LANG_FACTION_PEACE_FORCED);
            if (itr->second.Flags & FACTION_FLAG_HIDDEN)
                ss << GetHellgroundString(LANG_FACTION_HIDDEN);
            if (itr->second.Flags & FACTION_FLAG_INVISIBLE_FORCED)
                ss << GetHellgroundString(LANG_FACTION_INVISIBLE_FORCED);
            if (itr->second.Flags & FACTION_FLAG_INACTIVE)
                ss << GetHellgroundString(LANG_FACTION_INACTIVE);

            SendSysMessage(ss.str().c_str());
        }
    }
    return true;
}

//set spawn dist of creature
bool ChatHandler::HandleNpcSpawnDistCommand(const char* args)
{
    if (!*args)
        return false;

    float option = atof((char*)args);
    if (option < 0.0f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        return false;
    }

    MovementGeneratorType mtype = IDLE_MOTION_TYPE;
    if (option >0.0f)
        mtype = RANDOM_MOTION_TYPE;

    Creature *pCreature = getSelectedCreature();
    uint32 u_guidlow = 0;

    if (pCreature)
        u_guidlow = pCreature->GetDBTableGUIDLow();
    else
        return false;

    pCreature->SetRespawnRadius((float)option);
    pCreature->SetDefaultMovementType(mtype);
    pCreature->GetMotionMaster()->Initialize();
    if (pCreature->isAlive())                                // dead creature will reset movement generator at respawn
    {
        pCreature->setDeathState(JUST_DIED);
        pCreature->Respawn();
    }

    GameDataDatabase.PExecuteLog("UPDATE creature SET spawndist=%f, MovementType=%i WHERE guid=%u",option,mtype,u_guidlow);
    PSendSysMessage(LANG_COMMAND_SPAWNDIST,option);
    return true;
}

bool ChatHandler::HandleNpcSpawnTimeCommand(const char* args)
{
    if (!*args)
        return false;

    char* stime = strtok((char*)args, " ");

    if (!stime)
        return false;

    int i_stime = atoi((char*)stime);

    if (i_stime < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Creature *pCreature = getSelectedCreature();
    uint32 u_guidlow = 0;

    if (pCreature)
        u_guidlow = pCreature->GetDBTableGUIDLow();
    else
        return false;

    GameDataDatabase.PExecuteLog("UPDATE creature SET spawntimesecs=%i WHERE guid=%u",i_stime,u_guidlow);
    pCreature->SetRespawnDelay((uint32)i_stime);
    PSendSysMessage(LANG_COMMAND_SPAWNTIME,i_stime);

    return true;
}

/////WAYPOINT COMMANDS

bool ChatHandler::HandleWpAddCommand(const char* args)
{
    sLog.outDebug("DEBUG: HandleWpAddCommand");

    // optional
    char* str = NULL;
    uint32 pathid = 0;
    uint32 delay = 0;
    uint32 moveflag = 0;

    uint32 point = 0;
    Creature* target = getSelectedCreature();

    if (target)
        pathid = target->GetWaypointPath();
    else if(*args)
    {
        str = strtok((char*)args, " ");
        pathid = atoi(str);
    }
    else
    {
        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT MAX(id) FROM waypoint_data");
        uint32 maxpathid = result->Fetch()->GetInt32();
        pathid = maxpathid+1;
        sLog.outDebug("DEBUG: HandleWpAddCommand - New path started.");
        PSendSysMessage("%s%s|r", "|cff00ff00", "New path started.");
    }

    // path_id -> ID of the Path
    // point   -> number of the waypoint (if not 0)

    if (!pathid)
    {
        sLog.outDebug("DEBUG: HandleWpAddCommand - Current creature haven't loaded path.");
        PSendSysMessage("%s%s|r", "|cffff33ff", "Current creature haven't loaded path.");
        return true;
    }

    if (str = strtok(str ? NULL : (char*)args, " "))
        moveflag = atoi(str);
    if (str = strtok(NULL, " "))
        delay = atoi(str);

    sLog.outDebug("DEBUG: HandleWpAddCommand - point == 0");

    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT MAX(point) FROM waypoint_data WHERE id = '%u'",pathid);

    if (result)
        point = (*result)[0].GetUInt32();

    Player* player = m_session->GetPlayer();
    Map *map = player->GetMap();

    GameDataDatabase.PExecuteLog("INSERT INTO waypoint_data (id, point, position_x, position_y, position_z, orientation, delay, move_type, action, action_chance, emote) VALUES ('%u','%u','%f', '%f', '%f', '%f', '%u', '%u', 0, 100, 0)",
		pathid, point + 1, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetOrientation(), delay, moveflag);

    PSendSysMessage("%s%s%u%s%u%s|r", "|cff00ff00", "PathID: |r|cff00ffff", pathid, "|r|cff00ff00: Waypoint |r|cff00ffff", point,"|r|cff00ff00 created. ");

    return true;
}                                                           // HandleWpAddCommand

bool ChatHandler::HandleWpLoadPathCommand(const char *args)
{
    if (!*args)
        return false;

    // optional
    char* path_number = NULL;

    if (*args)
        path_number = strtok((char*)args, " ");


    uint32 pathid = 0;
    uint32 guidlow = 0;
    Creature* target = getSelectedCreature();

    // Did player provide a path_id?
    if (!path_number)
        sLog.outDebug("DEBUG: HandleWpLoadPathCommand - No path number provided");

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (target->GetEntry() == 1)
    {
        PSendSysMessage("%s%s|r", "|cffff33ff", "You want to load path to a waypoint? Aren't you?");
        SetSentErrorMessage(true);
        return false;
    }

    pathid = atoi(path_number);

    if (!pathid)
    {
        PSendSysMessage("%s%s|r", "|cffff33ff", "No vallid path number provided.");
        return true;
    }

    guidlow = target->GetDBTableGUIDLow();
    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT guid FROM creature_addon WHERE guid = '%u'",guidlow);

    if (result)
        GameDataDatabase.PExecute("UPDATE creature_addon SET path_id = '%u' WHERE guid = '%u'", pathid, guidlow);
    else
        GameDataDatabase.PExecute("INSERT INTO creature_addon(guid,path_id) VALUES ('%u','%u')", guidlow, pathid);

    GameDataDatabase.PExecute("UPDATE creature SET MovementType = '%u' WHERE guid = '%u'", WAYPOINT_MOTION_TYPE, guidlow);

    target->LoadPath(pathid);
    target->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
    target->GetMotionMaster()->Initialize();
    target->Say("Path loaded.",0,0);

    return true;
}


bool ChatHandler::HandleWpReloadPath(const char* args)
{
    if (!*args)
        return false;

    uint32 id = atoi(args);

    if (!id)
        return false;

    PSendSysMessage("%s%s|r|cff00ffff%u|r", "|cff00ff00", "Loading Path: ", id);
    sWaypointMgr.UpdatePath(id);

    return true;
}

bool ChatHandler::HandleWpUnLoadPathCommand(const char* /*args*/)
{
    uint32 guidlow = 0;
    Creature* target = getSelectedCreature();

    if (!target)
    {
        PSendSysMessage("%s%s|r", "|cff33ffff", "You must select target.");
        return true;
    }

    if (target->GetCreatureAddon())
    {
        if (target->GetCreatureAddon()->path_id != 0)
        {
            GameDataDatabase.PExecute("DELETE FROM creature_addon WHERE guid = %u", target->GetGUIDLow());
            target->UpdateWaypointID(0);
            GameDataDatabase.PExecute("UPDATE creature SET MovementType = '%u' WHERE guid = '%u'", IDLE_MOTION_TYPE, guidlow);
            target->LoadPath(0);
            target->SetDefaultMovementType(IDLE_MOTION_TYPE);
            target->GetMotionMaster()->MoveTargetedHome();
            target->GetMotionMaster()->Initialize();
            target->Say("Path unloaded.",0,0);
            return true;
        }
        PSendSysMessage("%s%s|r", "|cffff33ff", "Target have no loaded path.");
        return true;
    }
    PSendSysMessage("%s%s|r", "|cffff33ff", "Target have no loaded path.");
    return true;
}

bool ChatHandler::HandleWpEventCommand(const char* args)
{
if(!*args)
     return false;

    char* show_str = strtok((char*)args, " ");

    std::string show = show_str;

    // Check
    if ((show != "add") && (show != "mod") && (show != "del") && (show != "listid")) return false;


    if (show == "add")
    {
    uint32 id = 0;
    char* arg_id = strtok(NULL, " ");

    if (arg_id)
        uint32 id = atoi(arg_id);

    if (id)
    {
        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT `id` FROM waypoint_scripts WHERE guid = %u", id);

        if (!result)
        {
        GameDataDatabase.PExecute("INSERT INTO waypoint_scripts(guid)VALUES(%u)", id);
        PSendSysMessage("%s%s%u|r", "|cff00ff00", "Wp Event: New waypoint event added: ", id);
        }
        else
        PSendSysMessage("|cff00ff00Wp Event: You have choosed an existing waypoint script guid: %u|r", id);
    }
    else
    {
        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT MAX(guid) FROM waypoint_scripts");
        id = result->Fetch()->GetUInt32();
        GameDataDatabase.PExecute("INSERT INTO waypoint_scripts(guid)VALUES(%u)", id+1);
        PSendSysMessage("%s%s%u|r", "|cff00ff00","Wp Event: New waypoint event added: |r|cff00ffff", id+1);
    }

    return true;
    }


    if (show == "listid")
    {
    uint32 id;
    char* arg_id = strtok(NULL, " ");

    if (!arg_id)
    {
    PSendSysMessage("%s%s|r", "|cff33ffff","Wp Event: You must provide waypoint script id.");
    return true;
    }

    id = atoi(arg_id);

    uint32 a2, a3, a4, a5, a6;
    float a8, a9, a10, a11;
    char const* a7;

    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT `guid`, `delay`, `command`, `datalong`, `datalong2`, `dataint`, `x`, `y`, `z`, `o` FROM waypoint_scripts WHERE id = %u", id);

    if (!result)
    {
        PSendSysMessage("%s%s%u|r", "|cff33ffff", "Wp Event: No waypoint scripts found on id: ", id);
        return true;
    }

    Field *fields;

    do
    {
        fields = result->Fetch();
        a2 = fields[0].GetUInt32();
        a3 = fields[1].GetUInt32();
        a4 = fields[2].GetUInt32();
        a5 = fields[3].GetUInt32();
        a6 = fields[4].GetUInt32();
        a7 = fields[5].GetString();
        a8 = fields[6].GetFloat();
        a9 = fields[7].GetFloat();
        a10 = fields[8].GetFloat();
        a11 = fields[9].GetFloat();

        PSendSysMessage("|cffff33ffid:|r|cff00ffff %u|r|cff00ff00, guid: |r|cff00ffff%u|r|cff00ff00, delay: |r|cff00ffff%u|r|cff00ff00, command: |r|cff00ffff%u|r|cff00ff00, datalong: |r|cff00ffff%u|r|cff00ff00, datalong2: |r|cff00ffff%u|r|cff00ff00, datatext: |r|cff00ffff%s|r|cff00ff00, posx: |r|cff00ffff%f|r|cff00ff00, posy: |r|cff00ffff%f|r|cff00ff00, posz: |r|cff00ffff%f|r|cff00ff00, orientation: |r|cff00ffff%f|r", id, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
    }while (result->NextRow());
    }

    if (show == "del")
    {

    char* arg_id = strtok(NULL, " ");
    uint32 id = atoi(arg_id);

    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT `guid` FROM waypoint_scripts WHERE guid = %u", id);

    if (result)
    {

       GameDataDatabase.PExecuteLog("DELETE FROM waypoint_scripts WHERE guid = %u", id);
       PSendSysMessage("%s%s%u|r","|cff00ff00","Wp Event: Waypoint script removed: ", id);
    }
    else
        PSendSysMessage("|cffff33ffWp Event: ERROR: you have selected a non existing script: %u|r", id);

    return true;
    }


    if (show == "mod")
    {
    char* arg_1 = strtok(NULL," ");

    if (!arg_1)
    {
        SendSysMessage("|cffff33ffERROR: Waypoint script guid not present.|r");
        return true;
    }

    uint32 id = atoi(arg_1);

    if (!id)
    {
        SendSysMessage("|cffff33ffERROR: No vallid waypoint script id not present.|r");
        return true;
    }

    char* arg_2 = strtok(NULL," ");

    if (!arg_2)
        {   SendSysMessage("|cffff33ffERROR: No argument present.|r");
    return true;}


    std::string arg_string  = arg_2;


if((arg_string != "setid") && (arg_string != "delay") && (arg_string != "command")
&& (arg_string != "datalong") && (arg_string != "datalong2") && (arg_string != "dataint") && (arg_string != "posx")
&& (arg_string != "posy") && (arg_string != "posz") && (arg_string != "orientation")
) { SendSysMessage("|cffff33ffERROR: No valid argument present.|r");
    return true;}


char* arg_3;
std::string arg_str_2 = arg_2;
arg_3 = strtok(NULL," ");

if(!arg_3)
{SendSysMessage("|cffff33ffERROR: No additional argument present.|r");
    return true;}

float coord;

    if (arg_str_2 == "setid")
    {
        uint32 newid = atoi(arg_3);
        PSendSysMessage("%s%s|r|cff00ffff%u|r|cff00ff00%s|r|cff00ffff%u|r","|cff00ff00","Wp Event: Wypoint scipt guid: ", newid," id changed: ", id);
        GameDataDatabase.PExecuteLog("UPDATE waypoint_scripts SET id='%u' WHERE guid='%u'",
        newid, id); return true;
    }
    else
    {

    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT id FROM waypoint_scripts WHERE guid='%u'",id);

    if (!result)
    {
        SendSysMessage("|cffff33ffERROR: You have selected an non existing waypoint script guid.|r");
        return true;
    }

if(arg_str_2 == "posx")
{
    coord = atof(arg_3);
    GameDataDatabase.PExecuteLog("UPDATE waypoint_scripts SET x='%f' WHERE guid='%u'",
        coord, id);
    PSendSysMessage("|cff00ff00Waypoint script:|r|cff00ffff %u|r|cff00ff00 position_x updated.|r", id);
    return true;
}else if (arg_str_2 == "posy")
{
    coord = atof(arg_3);
    GameDataDatabase.PExecuteLog("UPDATE waypoint_scripts SET y='%f' WHERE guid='%u'",
        coord, id);
    PSendSysMessage("|cff00ff00Waypoint script: %u position_y updated.|r", id);
    return true;
} else if (arg_str_2 == "posz")
{
    coord = atof(arg_3);
    GameDataDatabase.PExecuteLog("UPDATE waypoint_scripts SET z='%f' WHERE guid='%u'",
        coord, id);
    PSendSysMessage("|cff00ff00Waypoint script: |r|cff00ffff%u|r|cff00ff00 position_z updated.|r", id);
    return true;
} else if (arg_str_2 == "orientation")
{
    coord = atof(arg_3);
    GameDataDatabase.PExecuteLog("UPDATE waypoint_scripts SET o='%f' WHERE guid='%u'",
        coord, id);
    PSendSysMessage("|cff00ff00Waypoint script: |r|cff00ffff%u|r|cff00ff00 orientation updated.|r", id);
    return true;
} else if (arg_str_2 == "dataint")
{
        GameDataDatabase.PExecuteLog("UPDATE waypoint_scripts SET %s='%u' WHERE guid='%u'",
        arg_2, atoi(arg_3), id);
        PSendSysMessage("|cff00ff00Waypoint script: |r|cff00ffff%u|r|cff00ff00 dataint updated.|r", id);
        return true;
}else
{
        std::string arg_str_3 = arg_3;
        GameDataDatabase.escape_string(arg_str_3);
        GameDataDatabase.PExecuteLog("UPDATE waypoint_scripts SET %s='%s' WHERE guid='%u'",
        arg_2, arg_str_3.c_str(), id);
}
}
    PSendSysMessage("%s%s|r|cff00ffff%u:|r|cff00ff00 %s %s|r","|cff00ff00","Waypoint script:", id, arg_2,"updated.");
}
return true;
}

bool ChatHandler::HandleWpModifyCommand(const char* args)
{
    sLog.outDebug("DEBUG: HandleWpModifyCommand");

    if (!*args)
        return false;

    // first arg: add del text emote spell waittime move
    char* show_str = strtok((char*)args, " ");
    if (!show_str)
    {
        return false;
    }

    std::string show = show_str;
    // Check
    // Remember: "show" must also be the name of a column!
    if ((show != "delay") && (show != "action") && (show != "action_chance")
        && (show != "move_type") && (show != "del") && (show != "move") && (show != "wpadd")
      )
    {
        return false;
    }

    // Next arg is: <PATHID> <WPNUM> <ARGUMENT>
    char* arg_str = NULL;

    // Did user provide a GUID
    // or did the user select a creature?
    // -> variable lowguid is filled with the GUID of the NPC
    uint32 pathid = 0;
    uint32 point = 0;
    uint32 wpGuid = 0;
    Creature* wpCreature = getSelectedCreature();

    if (!wpCreature || wpCreature->GetEntry() != VISUAL_WAYPOINT)
    {
        SendSysMessage("|cffff33ffERROR: You must select a waypoint.|r");
        return false;
    }

    wpGuid = wpCreature->GetGUIDLow();


    VisualWaypoint* wp = sWaypointMgr.GetVisualWaypoint(wpGuid);
    if (!wp)
    {
        PSendSysMessage(LANG_WAYPOINT_NOTFOUNDSEARCH, wpGuid);
        return true;
    }

    pathid = wp->pathId;
    point  = wp->pathPoint;

    // We have the waypoint number and the GUID of the "visual npc"
    // Text is enclosed in "<>", all other arguments not
    arg_str = strtok((char*)NULL, " ");

    // Check for argument
    if (show != "del" && show != "move" && arg_str == NULL)
    {
        PSendSysMessage(LANG_WAYPOINT_ARGUMENTREQ, show_str);
        return false;
    }

    if (show == "del")
    {
        PSendSysMessage("|cff00ff00DEBUG: wp modify del, PathID: |r|cff00ffff%u|r", pathid);

        if (wpCreature)
            wpCreature->Kill(wpCreature);

        GameDataDatabase.PExecuteLog("DELETE FROM waypoint_data WHERE id='%u' AND point='%u'",
            pathid, point);
        GameDataDatabase.PExecuteLog("UPDATE waypoint_data SET point=point-1 WHERE id='%u' AND point>'%u'",
            pathid, point);

        SendSysMessage(LANG_WAYPOINT_REMOVED);
        return true;
    }                                                       // del

    if (show == "move")
    {
        PSendSysMessage("|cff00ff00DEBUG: wp move, PathID: |r|cff00ffff%u|r", pathid);

        Player *chr = m_session->GetPlayer();
        Map *map = chr->GetMap();

        // What to do:
        // Move the visual spawnpoint
        // Respawn the owner of the waypoints
        float x, y, z, o;
        chr->GetPosition(x, y, z);
        o = chr->GetOrientation();

        map->CreatureRelocation(wpCreature,x, y, z,o);

        GameDataDatabase.PExecuteLog("UPDATE waypoint_data SET position_x = '%f',position_y = '%f',position_z = '%f' where id = '%u' AND point='%u'",
            x, y, z, pathid, point);

        SendSysMessage(LANG_WAYPOINT_CHANGED);

        return true;
    }                                                       // move


    const char *text = arg_str;

    if (text == 0)
    {
        // show_str check for present in list of correct values, no sql injection possible
        GameDataDatabase.PExecuteLog("UPDATE waypoint_data SET %s=NULL WHERE id='%u' AND point='%u'",
            show_str, pathid, point);
    }
    else
    {
        // show_str check for present in list of correct values, no sql injection possible
        std::string text2 = text;
        GameDataDatabase.escape_string(text2);
        GameDataDatabase.PExecuteLog("UPDATE waypoint_data SET %s='%s' WHERE id='%u' AND point='%u'",
            show_str, text2.c_str(), pathid, point);
    }

    PSendSysMessage(LANG_WAYPOINT_CHANGED_NO, show_str);

    return true;
}


bool ChatHandler::HandleWpShowCommand(const char* args)
{
    sLog.outDebug("DEBUG: HandleWpShowCommand");

    if (!*args)
        return false;

    // first arg: on, off, first, last
    char* show_str = strtok((char*)args, " ");
    if (!show_str)
    {
        return false;
    }
    // second arg: GUID (optional, if a creature is selected)
    char* guid_str = strtok((char*)NULL, " ");
    sLog.outDebug("DEBUG: HandleWpShowCommand: show_str: %s guid_str: %s", show_str, guid_str);

    uint32 pathid = 0;
    Creature* target = getSelectedCreature();

    // Did player provide a PathID?

    if (!guid_str)
    {
        // No PathID provided
        // -> Player must have selected a creature

        if (!target)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }

        pathid = target->GetWaypointPath();
    }

    else
    {
        // PathID provided
        // Warn if player also selected a creature
        // -> Creature selection is ignored <-
        if (target)
            SendSysMessage(LANG_WAYPOINT_CREATSELECTED);

        pathid = atoi((char*)guid_str);
    }

    std::string show = show_str;
    uint32 Maxpoint;

    // Show info for the selected waypoint
    if (show == "info")
    {
        // Check if the user did specify a visual waypoint
        if (target->GetEntry() != VISUAL_WAYPOINT)
        {
            SendSysMessage(LANG_WAYPOINT_VP_SELECT);
            SetSentErrorMessage(true);
            return false;
        }

        VisualWaypoint* wp = sWaypointMgr.GetVisualWaypoint(target->GetGUIDLow());
        if (!wp)
        {
            SendSysMessage(LANG_WAYPOINT_NOTFOUNDDBPROBLEM);
            return true;
        }

        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT id, point, delay, move_type, action, action_chance FROM waypoint_data WHERE id = '%u' AND point = '%u'", wp->pathId, wp->pathPoint);

        if (!result)
        {
            SendSysMessage(LANG_WAYPOINT_NOTFOUNDDBPROBLEM);
            return true;
        }

        SendSysMessage("|cff00ffffDEBUG: wp show info:|r");

        Field *fields = result->Fetch();
        pathid                  = fields[0].GetUInt32();
        uint32 point            = fields[1].GetUInt32();
        uint32 delay            = fields[2].GetUInt32();
        uint32 flag             = fields[3].GetUInt32();
        uint32 ev_id            = fields[4].GetUInt32();
        uint32 ev_chance        = fields[5].GetUInt32();

        PSendSysMessage("|cff00ff00Show info: for current point: |r|cff00ffff%u|r|cff00ff00, Path ID: |r|cff00ffff%u|r", point, pathid);
        PSendSysMessage("|cff00ff00Show info: delay: |r|cff00ffff%u|r", delay);
        PSendSysMessage("|cff00ff00Show info: Move flag: |r|cff00ffff%u|r", flag);
        PSendSysMessage("|cff00ff00Show info: Waypoint event: |r|cff00ffff%u|r", ev_id);
        PSendSysMessage("|cff00ff00Show info: Event chance: |r|cff00ffff%u|r", ev_chance);
        return true;
    }

    if (show == "on")
    {
        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT point, position_x,position_y,position_z FROM waypoint_data WHERE id = '%u'", pathid);

        if (!result)
        {
            SendSysMessage("|cffff33ffPath no found.|r");
            SetSentErrorMessage(true);
            return false;
        }

        PSendSysMessage("|cff00ff00DEBUG: wp on, PathID: |cff00ffff%u|r", pathid);

        do
        {
            Field *fields = result->Fetch();
            uint32 point    = fields[0].GetUInt32();
            float x         = fields[1].GetFloat();
            float y         = fields[2].GetFloat();
            float z         = fields[3].GetFloat();

            uint32 id = VISUAL_WAYPOINT;

            Player *chr = m_session->GetPlayer();
            Map *map = chr->GetMap();
            float o = chr->GetOrientation();

            Creature* wpCreature = chr->SummonCreature(id, x, y, z, 0, TEMPSUMMON_CORPSE_DESPAWN, 600);
            if (!wpCreature)
            {
                PSendSysMessage(LANG_WAYPOINT_VP_NOTCREATED, id);
                return false;
            }

            sWaypointMgr.AddVisualWaypoint(wpCreature->GetGUIDLow(), pathid, point);

            wpCreature->SetLevel(point > 70 ? 70 : point);
            wpCreature->SetMaxHealth(point);
            wpCreature->SetHealth(point);
            wpCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);

            //if (target)
            //    wpCreature->SetDisplayId(target->GetDisplayId()); // changing display id does not work for some reason
        }
        while (result->NextRow());

        SendSysMessage("|cff00ff00Showing the current creature's path.|r");
        return true;
    }

    if (show == "first")
    {
        PSendSysMessage("|cff00ff00DEBUG: wp first, GUID: %u|r", pathid);

        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT position_x,position_y,position_z FROM waypoint_data WHERE point='1' AND id = '%u'",pathid);
        if (!result)
        {
            PSendSysMessage(LANG_WAYPOINT_NOTFOUND, pathid);
            SetSentErrorMessage(true);
            return false;
        }

        Field *fields = result->Fetch();
        float x         = fields[0].GetFloat();
        float y         = fields[1].GetFloat();
        float z         = fields[2].GetFloat();
        uint32 id = VISUAL_WAYPOINT;

        Player *chr = m_session->GetPlayer();
        float o = chr->GetOrientation();
        Map *map = chr->GetMap();

        Creature* wpCreature = chr->SummonCreature(id, x, y, z, 0, TEMPSUMMON_CORPSE_DESPAWN, 600);
        if (!wpCreature)
        {
            PSendSysMessage(LANG_WAYPOINT_VP_NOTCREATED, id);
            return false;
        }

        wpCreature->SetLevel(1);
        wpCreature->SetMaxHealth(1);
        wpCreature->SetHealth(1);
        wpCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);

        //if (target)
        //    wpCreature->SetDisplayId(target->GetDisplayId()); // changing display id does not work for some reason
        return true;
    }

    if (show == "last")
    {
        PSendSysMessage("|cff00ff00DEBUG: wp last, PathID: |r|cff00ffff%u|r", pathid);

        QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT MAX(point) FROM waypoint_data WHERE id = '%u'",pathid);
        if (!result)
        {
            PSendSysMessage(LANG_WAYPOINT_NOTFOUNDLAST, pathid);
            SetSentErrorMessage(true);
            return false;
        }
            
        Maxpoint = (*result)[0].GetUInt32();

        result = GameDataDatabase.PQuery("SELECT position_x,position_y,position_z FROM waypoint_data WHERE point ='%u' AND id = '%u'",Maxpoint, pathid);
        if (!result)
        {
            PSendSysMessage(LANG_WAYPOINT_NOTFOUNDLAST, pathid);
            SetSentErrorMessage(true);
            return false;
        }
        Field *fields = result->Fetch();
        float x         = fields[0].GetFloat();
        float y         = fields[1].GetFloat();
        float z         = fields[2].GetFloat();
        uint32 id = VISUAL_WAYPOINT;

        Player *chr = m_session->GetPlayer();
        float o = chr->GetOrientation();
        Map *map = chr->GetMap();

        Creature* wpCreature = chr->SummonCreature(id, x, y, z, 0, TEMPSUMMON_CORPSE_DESPAWN, 600);
        if (!wpCreature)
        {
            PSendSysMessage(LANG_WAYPOINT_NOTCREATED, id);
            return false;
        }

        wpCreature->SetLevel(Maxpoint > 70 ? 70 : Maxpoint);
        wpCreature->SetMaxHealth(Maxpoint);
        wpCreature->SetHealth(Maxpoint);
        wpCreature->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5);

        //if (target)
        //    wpCreature->SetDisplayId(target->GetDisplayId()); // changing display id does not work for some reason
        return true;
    }

    if (show == "off")
    {
        Map* map = m_session->GetPlayer()->GetMap();

        std::list<uint64> list = map->GetCreaturesGUIDList(VISUAL_WAYPOINT);

        for (std::list<uint64>::const_iterator i = list.begin(); i != list.end(); ++i)
        {
            if (Creature* c = map->GetCreature(*i))
                c->Kill(c);
        }

        sWaypointMgr.ClearVisualWaypoints();
        SendSysMessage(LANG_WAYPOINT_VP_ALLREMOVED);

        return true;
    }

    SendSysMessage("|cffff33ffDEBUG: wpshow - no valid command found|r");

    return true;
}

//////////// WAYPOINT COMMANDS //

//rename characters
bool ChatHandler::HandleRenameCommand(const char* args)
{
    Player* target = NULL;
    uint64 targetGUID = 0;
    std::string oldname;

    char* px = strtok((char*)args, " ");

    if (px)
    {
        oldname = px;

        if (!normalizePlayerName(oldname))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        target = sObjectMgr.GetPlayerInWorld(oldname.c_str());

        if (!target)
            targetGUID = sObjectMgr.GetPlayerGUIDByName(oldname);
    }

    if (!target && !targetGUID)
    {
        target = getSelectedPlayer();
    }

    if (!target && !targetGUID)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (target)
    {
        PSendSysMessage(LANG_RENAME_PLAYER, target->GetName());
        target->SetAtLoginFlag(AT_LOGIN_RENAME);
    }
    else
    {
        PSendSysMessage(LANG_RENAME_PLAYER_GUID, oldname.c_str(), GUID_LOPART(targetGUID));
        RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login | '1' WHERE guid = '%u'", GUID_LOPART(targetGUID));
    }

    return true;
}

bool ChatHandler::HandleRenamePet(const char* args)
{
	Player* target = NULL;
	uint64 targetGUID = 0;
	std::string oldname;

	char* px = strtok((char*)args, " ");

	if (px)
	{
		oldname = px;

		if (!normalizePlayerName(oldname))
		{
			SendSysMessage("Pet name is incorrect");
			SetSentErrorMessage(true);
			return false;
		}

		QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT owner FROM character_pet WHERE name = '%s'", oldname.c_str());
		if (result)
		{
			do
			{
				Field *fields = result->Fetch();
				uint32 owner = fields[0].GetUInt32();
				if (Player* p = sObjectMgr.GetPlayerInWorld(owner))
				{
					Pet* pet = p->GetPet();
					if (pet)
					{
						pet->SetName("Rename");
						pet->SetByteValue(UNIT_FIELD_BYTES_2, 2, UNIT_RENAME_ALLOWED);
					}
				}

			} while (result->NextRow());
		}

		RealmDataDatabase.PExecute("UPDATE character_pet SET name = 'Rename', renamed = 0 where name = '%s'", oldname.c_str());

		SendSysMessage("Player will be able to change pet name after next login.");	
	}

	return true;
}

bool ChatHandler::HandleNameBanCommand(const char* args)
{
    Player* target = NULL;
    uint64 targetGUID = 0;
    std::string oldname = "";
    bool Selected = false;
    bool WasBanned = false;
    std::string HadReservedName = "";

    char* px = strtok((char*)args, " ");

    if (px)
    {
        oldname = px;

        if (!normalizePlayerName(oldname))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        target = sObjectMgr.GetPlayerInWorld(oldname.c_str());

        if (!target)
            targetGUID = sObjectMgr.GetPlayerGUIDByName(oldname);
    }

    if (!target && !targetGUID)
    {
        target = getSelectedPlayer();
        if (target == m_session->GetPlayer())
            target = NULL;
        Selected = true;
    }

    if (target || targetGUID)
    {
        QueryResultAutoPtr ReservedName = RealmDataDatabase.PQuery("SELECT name FROM reserved_name WHERE guid = '%u'", target ? target->GetGUID() : targetGUID);
        if (ReservedName)
        {
            Field* ResNameFields = ReservedName->Fetch();
            HadReservedName = ResNameFields[0].GetCppString(); // Has been banned once already
            PSendSysMessage("Player %s has already been banned for name %s.", (Selected && target) ? target->GetName() : oldname.c_str(), HadReservedName.c_str());
        }
    }

    if ((Selected && target) || !sObjectMgr.IsReservedName(oldname))
    {
        PSendSysMessage("Name %s added to banned names.", (Selected && target) ? target->GetName() : oldname.c_str());
        if (target)
            RealmDataDatabase.PExecute("INSERT IGNORE INTO reserved_name (`guid`, `name`) VALUES ('%u', '%s')", target->GetGUID(), Selected ? target->GetName() : oldname.c_str());
        else if (targetGUID)
            RealmDataDatabase.PExecute("INSERT IGNORE INTO reserved_name (`guid`, `name`) VALUES ('%u', '%s')", targetGUID, oldname.c_str());
        else
            RealmDataDatabase.PExecute("INSERT IGNORE INTO reserved_name (`name`) VALUES ('%s')", oldname.c_str());

        HandleReloadReservedNameCommand("");
    }
    else
        PSendSysMessage("Name %s already exists in banned names.", oldname.c_str());

    if (!target && !targetGUID)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (target)
    {
        PSendSysMessage(LANG_RENAME_PLAYER, target->GetName());
        target->SetAtLoginFlag(AT_LOGIN_RENAME);
        target->GetSession()->KickPlayer();
        PSendSysMessage("Player %s kicked.", (Selected && target) ? target->GetName() : oldname.c_str());
    }
    else
    {
        PSendSysMessage(LANG_RENAME_PLAYER_GUID, oldname.c_str(), GUID_LOPART(targetGUID));
        RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login | '1' WHERE guid = '%u'", GUID_LOPART(targetGUID));
    }

    return true;
}

//spawn go
bool ChatHandler::HandleGameObjectAddCommand(const char* args)
{
    if (!*args)
        return false;

    // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args, "Hgameobject_entry");
    if (!cId)
        return false;

    uint32 id = atol(cId);
    if (!id)
        return false;

    char* spawntimeSecs = strtok(NULL, " ");

    const GameObjectInfo *goI = ObjectMgr::GetGameObjectInfo(id);

    if (!goI)
    {
        PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST,id);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = m_session->GetPlayer();
    float x = float(chr->GetPositionX());
    float y = float(chr->GetPositionY());
    float z = float(chr->GetPositionZ());
    float o = float(chr->GetOrientation());
    Map *map = chr->GetMap();

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT MAX(guid) FROM gameobject");
    if (!result)
        return false;

    uint32 newGameObjectGuid = (*result)[0].GetUInt32()+1;

    GameObject* pGameObj = new GameObject;
    uint32 db_lowGUID = newGameObjectGuid;

    if (!pGameObj->Create(db_lowGUID, goI->id, map, x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
    {
        delete pGameObj;
        return false;
    }

    if (spawntimeSecs)
    {
        uint32 value = atoi((char*)spawntimeSecs);
        pGameObj->SetRespawnTime(value);
        //sLog.outDebug("*** spawntimeSecs: %d", value);
    }

    // fill the gameobject data and save to the db
    pGameObj->SaveUpdateOrNewToDB_gob_add(map->GetId(), (1 << map->GetSpawnMode()));

    // this will generate a new guid if the object is in an instance
    if (!pGameObj->LoadFromDB(db_lowGUID, map))
    {
        delete pGameObj;
        return false;
    }

    sLog.outDebug(GetHellgroundString(LANG_GAMEOBJECT_CURRENT), goI->name, db_lowGUID, x, y, z, o);

    map->Add(pGameObj);

    // TODO: is it really necessary to add both the real and DB table guid here ?
    sObjectMgr.AddGameobjectToGrid(db_lowGUID, sObjectMgr.GetGOData(db_lowGUID));

    PSendSysMessage(LANG_GAMEOBJECT_ADD,id,goI->name,db_lowGUID,x,y,z);
    return true;
}

//change standstate
bool ChatHandler::HandleModifyStandStateCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 anim_id = atoi((char*)args);
    m_session->GetPlayer()->SetUInt32Value(UNIT_NPC_EMOTESTATE , anim_id);

    return true;
}

bool ChatHandler::HandleHonorAddCommand(const char* args)
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

    uint32 amount = (uint32)atoi(args);
    target->RewardHonor(NULL, 1, amount);
    return true;
}

bool ChatHandler::HandleHonorAddKillCommand(const char* /*args*/)
{
    Unit *target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->RewardHonor(target, 1);
    return true;
}

bool ChatHandler::HandleHonorUpdateCommand(const char* /*args*/)
{
    Player *target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    target->UpdateHonorFields();
    return true;
}

bool ChatHandler::HandleLookupEventCommand(const char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();
    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr.GetActiveEventList();

    for (uint32 id = 0; id < events.size(); ++id)
    {
        GameEventData const& eventData = events[id];

        std::string descr = eventData.name_loc1;
        if (descr.empty())
            continue;

        if (Utf8FitTo(descr, wnamepart))
        {
            char const* active = activeEvents.find(id) != activeEvents.end() ? GetHellgroundString(LANG_ACTIVE) : "";

            if (m_session)
                PSendSysMessage(LANG_EVENT_ENTRY_LIST_CHAT, id, id, eventData.name_loc1.c_str(), active);
            else
                PSendSysMessage(LANG_EVENT_ENTRY_LIST_CONSOLE, id, eventData.name_loc1.c_str(), active);

            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_NOEVENTFOUND);

    return true;
}

bool ChatHandler::HandleEventActiveListCommand(const char* args)
{
    uint32 counter = 0;

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();
    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr.GetActiveEventList();

    char const* active = GetHellgroundString(LANG_ACTIVE);

    for (GameEventMgr::ActiveEvents::const_iterator itr = activeEvents.begin(); itr != activeEvents.end(); ++itr)
    {
        uint32 event_id = *itr;
        GameEventData const& eventData = events[event_id];

        if (m_session)
            PSendSysMessage(LANG_EVENT_ENTRY_LIST_CHAT, event_id, event_id, eventData.name_loc1.c_str(), active);
        else
            PSendSysMessage(LANG_EVENT_ENTRY_LIST_CONSOLE, event_id, eventData.name_loc1.c_str(), active);

        ++counter;
    }

    if (counter==0)
        SendSysMessage(LANG_NOEVENTFOUND);

    return true;
}

bool ChatHandler::HandleEventAwardCommand(const char* args)
{
    uint32 entry = 0;
    uint32 count = 0;
    if (strcmp(args, "bojs!") == 0)
    {
        entry = 29434;
        count = 2;
    }
    std::ostringstream ostr;
    ostr << "List of players that get award: ";
    Unit* me = m_session->GetPlayer();
    std::list<Player*> targets;
    Hellground::AnyPlayerInObjectRangeCheck check(me, 50);
    Hellground::ObjectListSearcher<Player, Hellground::AnyPlayerInObjectRangeCheck> searcher(targets, check);
    Cell::VisitAllObjects(me, searcher, 50);
    for (std::list<Player*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
    {
        ostr << (*itr)->GetName() << " ";
        if (entry && count)
        {
            ItemPosCountVec dest;
            uint8 msg = (*itr)->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, count);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = (*itr)->StoreNewItem(dest, entry, true, 0, "GM_COMMAND");
                (*itr)->SendNewItem(item, count, true, false, true);
            }
        }
    }

    SendSysMessage(ostr.str().c_str());
    return true;
}

bool ChatHandler::HandleEventInfoCommand(const char* args)
{
    if (!*args)
        return false;

    // id or [name] Shift-click form |color|Hgameevent:id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameevent");
    if (!cId)
        return false;

    uint32 event_id = atoi(cId);

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();

    if (event_id >=events.size())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventData const& eventData = events[event_id];
    if (!eventData.isValid())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr.GetActiveEventList();
    bool active = activeEvents.find(event_id) != activeEvents.end();
    char const* activeStr = active ? GetHellgroundString(LANG_ACTIVE) : "";

    std::string startTimeStr = TimeToTimestampStr(eventData.start);
    std::string endTimeStr = TimeToTimestampStr(eventData.end);

    uint32 delay = sGameEventMgr.NextCheck(event_id);
    time_t nextTime = time(NULL)+delay;
    std::string nextStr = nextTime >= eventData.start && nextTime < eventData.end ? TimeToTimestampStr(time(NULL)+delay) : "-";

    std::string occurenceStr = WorldSession::secondsToTimeString(m_session, eventData.occurence * MINUTE);
    std::string lengthStr = WorldSession::secondsToTimeString(m_session, eventData.length * MINUTE);

    PSendSysMessage(LANG_EVENT_INFO, event_id, eventData.name_loc1.c_str(), activeStr,
        startTimeStr.c_str(),endTimeStr.c_str(),occurenceStr.c_str(),lengthStr.c_str(),
        nextStr.c_str());
    return true;
}

bool ChatHandler::HandleEventStartCommand(const char* args)
{
    if (!*args)
        return false;

    // id or [name] Shift-click form |color|Hgameevent:id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameevent");
    if (!cId)
        return false;

    int32 event_id = atoi(cId);

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();

    if (event_id < 1 || event_id >=events.size())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    if (event_id >= 200 && event_id <= 204)
    {
        SendSysMessage("Seasons cannot be started ingame");
        SetSentErrorMessage(true);
        return false;
    }

    GameEventData const& eventData = events[event_id];
    if (!eventData.isValid())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr.GetActiveEventList();
    if (activeEvents.find(event_id) != activeEvents.end())
    {
        PSendSysMessage(LANG_EVENT_ALREADY_ACTIVE,event_id);
        SetSentErrorMessage(true);
        return false;
    }

    sGameEventMgr.StartEvent(event_id,true);
    PSendSysMessage("Event %u activated", event_id);
    return true;
}

bool ChatHandler::HandleEventStopCommand(const char* args)
{
    if (!*args)
        return false;

    // id or [name] Shift-click form |color|Hgameevent:id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameevent");
    if (!cId)
        return false;

    int32 event_id = atoi(cId);

    GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();

    if (event_id < 1 || event_id >=events.size())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    if (event_id >= 200 && event_id <= 204)
    {
        SendSysMessage("Seasons cannot be stopped ingame");
        SetSentErrorMessage(true);
        return false;
    }

    GameEventData const& eventData = events[event_id];
    if (!eventData.isValid())
    {
        SendSysMessage(LANG_EVENT_NOT_EXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameEventMgr::ActiveEvents const& activeEvents = sGameEventMgr.GetActiveEventList();

    if (activeEvents.find(event_id) == activeEvents.end())
    {
        PSendSysMessage(LANG_EVENT_NOT_ACTIVE,event_id);
        SetSentErrorMessage(true);
        return false;
    }

    sGameEventMgr.StopEvent(event_id,true);
    return true;
}

bool ChatHandler::HandleCombatStopCommand(const char* args)
{
    Player *player;

    if (*args)
    {
        std::string playername = args;

        if (!normalizePlayerName(playername))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectMgr.GetPlayerInWorld(playername.c_str());

        if (!player)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        player = getSelectedPlayer();

        if (!player)
            player = m_session->GetPlayer();
    }

    player->CombatStop();
    player->getHostileRefManager().deleteReferences();
    return true;
}

bool ChatHandler::HandleLearnAllCraftsCommand(const char* /*args*/)
{
    uint32 classmask = m_session->GetPlayer()->GetClassMask();

    for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
    {
        SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(i);
        if (!skillInfo)
            continue;

        if (skillInfo->categoryId == SKILL_CATEGORY_PROFESSION || skillInfo->categoryId == SKILL_CATEGORY_SECONDARY)
        {
            for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
            {
                SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
                if (!skillLine)
                    continue;

                // skip racial skills
                if (skillLine->racemask != 0)
                    continue;

                // skip wrong class skills
                if (skillLine->classmask && (skillLine->classmask & classmask) == 0)
                    continue;

                if (skillLine->skillId != i || skillLine->forward_spellid)
                    continue;

                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(skillLine->spellId);
                if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
                    continue;

                m_session->GetPlayer()->learnSpell(skillLine->spellId);
            }
        }
    }

    SendSysMessage(LANG_COMMAND_LEARN_ALL_CRAFT);
    return true;
}

bool ChatHandler::HandleLearnAllRecipesCommand(const char* args)
{
    //  Learns all recipes of specified profession and sets skill to max
    //  Example: .learn all_recipes enchanting

    Player* target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        return false;
    }

    if (!*args)
        return false;

    std::wstring wnamepart;

    if (!Utf8toWStr(args,wnamepart))
        return false;

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 classmask = m_session->GetPlayer()->GetClassMask();

    for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
    {
        SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(i);
        if (!skillInfo)
            continue;

        if (skillInfo->categoryId != SKILL_CATEGORY_PROFESSION &&
            skillInfo->categoryId != SKILL_CATEGORY_SECONDARY)
            continue;

        int loc = m_session->GetSessionDbcLocale();
        std::string name = skillInfo->name[loc];

        if (Utf8FitTo(name, wnamepart))
        {
            for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
            {
                SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
                if (!skillLine)
                    continue;

                if (skillLine->skillId != i || skillLine->forward_spellid)
                    continue;

                // skip racial skills
                if (skillLine->racemask != 0)
                    continue;

                // skip wrong class skills
                if (skillLine->classmask && (skillLine->classmask & classmask) == 0)
                    continue;

                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(skillLine->spellId);
                if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
                    continue;

                if (!target->HasSpell(spellInfo->Id))
                    m_session->GetPlayer()->learnSpell(skillLine->spellId);
            }

            uint16 maxLevel = target->GetPureMaxSkillValue(skillInfo->id);
            target->SetSkill(skillInfo->id, maxLevel, maxLevel);
            PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, name.c_str());
            return true;
        }
    }

    return false;
}

bool ChatHandler::HandleLookupPlayerIpCommand(const char* args)
{
    if (!*args)
        return false;

    std::string ip = strtok((char*)args, " ");
    char* limit_str = strtok(NULL, " ");
    int32 limit = limit_str ? atoi(limit_str) : -1;

    AccountsDatabase.escape_string(ip);
    if (sWorld.getConfig(CONFIG_HIDE_GAMEMASTER_ACCOUNTS))
    {
        QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT account_id, username FROM account WHERE last_ip = '%s' AND account_id NOT IN (SELECT account_id FROM account_gm)", ip.c_str());
        return LookupPlayerSearchCommand(result, limit);
    }
    else
    {
        QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT account_id, username FROM account WHERE last_ip = '%s'", ip.c_str());
        return LookupPlayerSearchCommand(result, limit);
    }

}

bool ChatHandler::HandleLookupPlayerAccountCommand(const char* args)
{
    if (!*args)
        return false;

    std::string account = strtok((char*)args, " ");
    char* limit_str = strtok(NULL, " ");
    int32 limit = limit_str ? atoi(limit_str) : -1;

    if (!AccountMgr::normalizeString(account))
        return false;

    AccountsDatabase.escape_string(account);

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT account_id, username FROM account WHERE username = '%s'", account.c_str());

    return LookupPlayerSearchCommand(result, limit);
}

bool ChatHandler::HandleLookupPlayerEmailCommand(const char* args)
{
    if (!*args)
        return false;

    std::string email = strtok((char*)args, " ");
    char* limit_str = strtok(NULL, " ");
    int32 limit = limit_str ? atoi(limit_str) : -1;

    AccountsDatabase.escape_string(email);

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT account_id, username FROM account WHERE email = '%s'", email.c_str());

    return LookupPlayerSearchCommand(result, limit);
}

bool ChatHandler::LookupPlayerSearchCommand(QueryResultAutoPtr result, int32 limit)
{
    if (!result)
    {
        SendSysMessage(LANG_NO_PLAYERS_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    int i =0;
    do
    {
        Field* fields = result->Fetch();
        uint32 acc_id = fields[0].GetUInt32();
        std::string acc_name = fields[1].GetCppString();

        QueryResultAutoPtr chars = RealmDataDatabase.PQuery("SELECT guid,name FROM characters WHERE account = '%u'", acc_id);
        if (chars)
        {
            PSendSysMessage(LANG_LOOKUP_PLAYER_ACCOUNT,acc_name.c_str(),acc_id);

            uint64 guid = 0;
            std::string name;
            Player* Player = nullptr;

            do
            {
                Field* charfields = chars->Fetch();
                guid = charfields[0].GetUInt64();
                name = charfields[1].GetCppString();
                Player = sObjectMgr.GetPlayerInWorld(sObjectMgr.GetPlayerGUIDByName(name));
                if (Player && Player->IsInWorld())
                    name += "    ONLINE ";

                PSendSysMessage(LANG_LOOKUP_PLAYER_CHARACTER, GetNameLink(name).c_str(), guid);
                ++i;

            } while (chars->NextRow() && (limit == -1 || i < limit));
        }
    } while (result->NextRow());

    if (i == 0)                                                // empty accounts only
    {
        SendSysMessage(LANG_NO_PLAYERS_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

/// Triggering corpses expire check in world
bool ChatHandler::HandleServerCorpsesCommand(const char* /*args*/)
{
    sObjectAccessor.RemoveOldCorpses();
    return true;
}

bool ChatHandler::HandleRepairitemsCommand(const char* /*args*/)
{
    Player *target = getSelectedPlayer();

    if (!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (target->GetDummyAura(54839))
        target->DurabilityLossAll(1.0f, false);
    else
    // Repair items
        target->DurabilityRepairAll(false, 0, false);

    PSendSysMessage(LANG_YOU_REPAIR_ITEMS, target->GetName());
    if (needReportToTarget(target))
        ChatHandler(target).PSendSysMessage(LANG_YOUR_ITEMS_REPAIRED, GetName());
    return true;
}

bool ChatHandler::HandleNpcFollowCommand(const char* /*args*/)
{
    Player *player = m_session->GetPlayer();
    Creature *creature = getSelectedCreature();

    if (!creature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // Follow player - Using pet's default dist and angle
    creature->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

    PSendSysMessage(LANG_CREATURE_FOLLOW_YOU_NOW, creature->GetName());
    return true;
}

bool ChatHandler::HandleNpcUnFollowCommand(const char* /*args*/)
{
    Player *player = m_session->GetPlayer();
    Creature *creature = getSelectedCreature();

    if (!creature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (/*creature->GetMotionMaster()->empty() ||*/
        creature->GetMotionMaster()->GetCurrentMovementGeneratorType ()!=FOLLOW_MOTION_TYPE)
    {
        PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    FollowMovementGenerator<Creature> const* mgen
        = static_cast<FollowMovementGenerator<Creature> const*>((creature->GetMotionMaster()->top()));

    if (mgen->GetTarget()!=player)
    {
        PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU, creature->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    // reset movement
    creature->GetUnitStateMgr().InitDefaults(false);

    PSendSysMessage(LANG_CREATURE_NOT_FOLLOW_YOU_NOW, creature->GetName());
    return true;
}

bool ChatHandler::HandleCreatePetCommand(const char* args)
{
    Player *player = m_session->GetPlayer();
    Creature *creatureTarget = getSelectedCreature();

    if (!creatureTarget || creatureTarget->isPet() || creatureTarget->GetTypeId() == TYPEID_PLAYER)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creatureTarget->GetEntry());
    // Creatures with family 0 crashes the server
    if (cInfo->family == 0)
    {
        SendSysMessage("This creature cannot be tamed. (family id: 0).");
        SetSentErrorMessage(true);
        return false;
    }

    if (player->GetPetGUID())
    {
        SendSysMessage("You already have a pet");
        SetSentErrorMessage(true);
        return false;
    }

    // Everything looks OK, create new pet
    Pet* pet = new Pet(HUNTER_PET);

    if (!pet)
      return false;

    if (!pet->CreateBaseAtCreature(creatureTarget))
    {
        delete pet;
        SendSysMessage("Error 1");
        return false;
    }

    creatureTarget->setDeathState(JUST_DIED);
    creatureTarget->RemoveCorpse();
    creatureTarget->SetHealth(0); // just for nice GM-mode view

    pet->SetUInt64Value(UNIT_FIELD_SUMMONEDBY, player->GetGUID());
    pet->SetUInt64Value(UNIT_FIELD_CREATEDBY, player->GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, player->getFaction());

    if (!pet->InitStatsForLevel(creatureTarget->GetLevel()))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: InitStatsForLevel() in EffectTameCreature failed! Pet deleted.");
        SendSysMessage("Error 2");
        delete pet;
        return false;
    }

    // prepare visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL,creatureTarget->GetLevel()-1);

     pet->GetCharmInfo()->SetPetNumber(sObjectMgr.GeneratePetNumber(), true);
     // this enables pet details window (Shift+P)

     pet->InitPetCreateSpells();
     pet->SetHealth(pet->GetMaxHealth());

     Map * pMap = pet->GetMap();
     pMap->Add((Creature*)pet);

     // visual effect for levelup
     pet->SetUInt32Value(UNIT_FIELD_LEVEL,creatureTarget->GetLevel());

     player->SetPet(pet);
     pet->SavePetToDB(PET_SAVE_AS_CURRENT);
     player->DelayedPetSpellInitialize();

    return true;
}

bool ChatHandler::HandlePetLearnCommand(const char* args)
{
    if (!*args)
        return false;

    Pet *pet = ObjectAccessor::GetPet(m_session->GetPlayer()->GetSelection());

    if (!pet)
    {
        SendSysMessage("You must select a pet");
        SetSentErrorMessage(true);
        return false;
    }

    uint32 spellId = extractSpellIdFromLink((char*)args);

    if (!spellId || !sSpellTemplate.LookupEntry<SpellEntry>(spellId))
        return false;

    // Check if pet already has it
    if (pet->HasSpell(spellId))
    {
        PSendSysMessage("Pet already has spell: %u", spellId);
        SetSentErrorMessage(true);
        return false;
    }

    // Check if spell is valid
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spellId);
        SetSentErrorMessage(true);
        return false;
    }

    pet->learnSpell(spellId);

    PSendSysMessage("Pet has learned spell %u", spellId);
    return true;
}

bool ChatHandler::HandlePetUnlearnCommand(const char *args)
{
    if (!*args)
        return false;

    Pet *pet = ObjectAccessor::GetPet(m_session->GetPlayer()->GetSelection());

    if (!pet)
    {
        SendSysMessage("You must select a pet");
        SetSentErrorMessage(true);
        return false;
    }

    uint32 spellId = extractSpellIdFromLink((char*)args);

    if (pet->HasSpell(spellId))
        pet->removeSpell(spellId);
    else
        SendSysMessage("Pet doesn't have that spell");

    return true;
}

bool ChatHandler::HandlePetTpCommand(const char *args)
{
    if (!*args)
        return false;

    Player *plr = m_session->GetPlayer();
    Pet *pet = plr->GetPet();

    if (!pet)
    {
        SendSysMessage("You have no pet");
        SetSentErrorMessage(true);
        return false;
    }

    uint32 tp = atol(args);

    pet->SetTP(tp);

    PSendSysMessage("Pet's tp changed to %u", tp);
    return true;
}

bool ChatHandler::HandleGameObjectActivateCommand(const char *args)
{
    if (!*args)
        return false;

    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if (!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if (!lowguid)
        return false;

    GameObject* obj = NULL;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr.GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithDbGuid(lowguid,go_data->id);

    if (!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    // Activate
    obj->SetLootState(GO_READY);
    obj->UseDoorOrButton(10000);

    SendSysMessage("Object activated!");

    return true;
}

bool ChatHandler::HandleGameObjectResetCommand(const char *args)
{
    if (!*args)
        return false;

    char* cId = extractKeyFromLink((char*)args,"Hgameobject");
    if (!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if (!lowguid)
        return false;

    GameObject* obj = NULL;

    // by DB guid
    if (GameObjectData const* go_data = sObjectMgr.GetGOData(lowguid))
        obj = GetObjectGlobalyWithGuidOrNearWithDbGuid(lowguid,go_data->id);

    if (!obj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    // Activate
    obj->Reset();

    SendSysMessage("Object reset!");

    return true;
}

// add creature, temp only
bool ChatHandler::HandleNpcAddTempCommand(const char* args)
{
    if (!*args)
        return false;
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    Player *chr = m_session->GetPlayer();

    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float ang = chr->GetOrientation();

    uint32 id = atoi(charID);

    chr->SummonCreature(id,x,y,z,ang,TEMPSUMMON_CORPSE_DESPAWN,120);

    return true;
}

// add go, temp only
bool ChatHandler::HandleGameObjectAddTempCommand(const char* args)
{
    if (!*args)
        return false;
    char* charID = strtok((char*)args, " ");
    if (!charID)
        return false;

    Player *chr = m_session->GetPlayer();

    char* spawntime = strtok(NULL, " ");
    uint32 spawntm = 60;

    if (spawntime)
        spawntm = atoi((char*)spawntime);

    float x = chr->GetPositionX();
    float y = chr->GetPositionY();
    float z = chr->GetPositionZ();
    float ang = chr->GetOrientation();

    float rot2 = sin(ang/2);
    float rot3 = cos(ang/2);

    uint32 id = atoi(charID);

    chr->SummonGameObject(id,x,y,z,ang,0,0,rot2,rot3,spawntm);

    return true;
}

bool ChatHandler::HandleNpcSetLinkCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 linkguid = (uint32) atoi((char*)args);

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!pCreature->GetDBTableGUIDLow())
    {
        PSendSysMessage("Selected creature %u isn't in `creature` table", pCreature->GetGUIDLow());
        SetSentErrorMessage(true);
        return false;
    }

    if (!sObjectMgr.SetCreatureLinkedRespawn(pCreature->GetDBTableGUIDLow(), linkguid))
    {
        PSendSysMessage("Selected creature can't link with guid '%u'", linkguid);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage("LinkGUID '%u' added to creature with DBTableGUID: '%u'", linkguid, pCreature->GetDBTableGUIDLow());
    return true;
}

bool ChatHandler::HandleNpcResetAICommand(const char* args)
{
    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->AIM_Initialize();
    SendSysMessage("CreatureAI re-created.");
    return true;
}

bool ChatHandler::HandleNpcDoActionCommand(const char* args)
{
    if (!*args)
        return false;

    int32 param = (uint32) atoi((char*)args);

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->AI()->DoAction(param);

    SendSysMessage("Action send to creature.");
    return true;
}

bool ChatHandler::HandleNpcEnterEvadeModeCommand(const char* args)
{
    Creature * pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage("You should select creature");
        return false;
    }

    if (m_session->GetPlayer()->GetMapId() != pCreature->GetMapId())
    {
        SendSysMessage("Creature must be in same map !");
        return false;
    }

    if (!pCreature->IsAIEnabled)
    {
        SendSysMessage("Creature AI is disabled !");
        return false;
    }

    pCreature->AI()->EnterEvadeMode();

    return true;
}

bool ChatHandler::HandleChannelListCommand(const char * args)
{
    ChannelMgr* cMgr = channelMgr(m_session->GetPlayer()->GetTeam());

    if (!cMgr)
        return false;

    std::list<std::string> tmpList = cMgr->GetCustomChannelNames();

    SendSysMessage("Channel list:");
    for (std::list<std::string>::iterator itr = tmpList.begin(); itr != tmpList.end(); ++itr)
        PSendSysMessage("%s", (*itr).c_str());

    SendSysMessage("Channel list end.");

    return true;
}

bool ChatHandler::HandleMmapPathCommand(const char* args)
{
    if (!MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(m_session->GetPlayer()->GetMapId()))
    {
        SendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    SendSysMessage("mmap path:");

    // units
    Player* player = m_session->GetPlayer();
    Unit* startUnit = player;
    Unit* endUnit = getSelectedUnit();
    if (!startUnit || !endUnit)
    {
        SendSysMessage("Invalid target/source selection.");
        return true;
    }

    char* para = strtok((char*)args, " ");
    bool useStraightPath = false;
    if (para && strcmp(para, "true") == 0)
        useStraightPath = true;
    else if (para && strcmp(para, "reverse") == 0)
        std::swap(startUnit, endUnit);

    // unit locations
    float x, y, z;
    startUnit->GetPosition(x, y, z);

    // path
    PathFinder path(endUnit);
    path.setUseStrightPath(useStraightPath);
    path.calculate(x, y, z);

    PointsArray pointPath = path.getPath();
    PSendSysMessage("%s's path to %s:", endUnit->GetName(), startUnit->GetName());
    PSendSysMessage("Building %s", useStraightPath ? "StraightPath" : "SmoothPath");
    PSendSysMessage("length %llu type %u", pointPath.size(), path.getPathType());

    Vector3 start = path.getStartPosition();
    Vector3 end = path.getEndPosition();
    Vector3 actualEnd = path.getActualEndPosition();

    PSendSysMessage("start      (%.3f, %.3f, %.3f)", start.x, start.y, start.z);
    PSendSysMessage("end        (%.3f, %.3f, %.3f)", end.x, end.y, end.z);

    PSendSysMessage("actual end (%.3f, %.3f, %.3f)", actualEnd.x, actualEnd.y, actualEnd.z);

    if (!player->isGameMaster())
        SendSysMessage("Enable GM mode to see the path points.");

    // this entry visible only to GM's with "gm on"
    static const uint32 WAYPOINT_NPC_ENTRY = 1;
    Creature* wp = NULL;
    for (uint32 i = 0; i < pointPath.size(); ++i)
    {
        wp = player->SummonCreature(WAYPOINT_NPC_ENTRY, pointPath[i].x, pointPath[i].y, pointPath[i].z, 0, TEMPSUMMON_TIMED_DESPAWN, 9000);
        // TODO: make creature not sink/fall
    }

    return true;
}

bool ChatHandler::HandleMmapLocCommand(const char* /*args*/)
{
    SendSysMessage("mmap tileloc:");

    // grid tile location
    Player* player = m_session->GetPlayer();

    int32 gx = 32 - player->GetPositionX() / SIZE_OF_GRIDS;
    int32 gy = 32 - player->GetPositionY() / SIZE_OF_GRIDS;

    PSendSysMessage("%03u%02i%02i.mmtile", player->GetMapId(), gy, gx);
    PSendSysMessage("gridloc [%i,%i]", gx, gy);

    // calculate navmesh tile location
    const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(player->GetMapId());
    const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(player->GetMapId(), player->GetAnyInstanceId());
    if (!navmesh || !navmeshquery)
    {
        SendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    const float* min = navmesh->getParams()->orig;

    float x, y, z;
    player->GetPosition(x, y, z);
    float location[VERTEX_SIZE] = {y, z, x};
    float extents[VERTEX_SIZE] = {3.0f, 5.0f, 3.0f};

    int32 tilex = int32((y - min[0]) / SIZE_OF_GRIDS);
    int32 tiley = int32((x - min[2]) / SIZE_OF_GRIDS);

    PSendSysMessage("Calc   [%02i,%02i]", tilex, tiley);

    // navmesh poly -> navmesh tile location
    dtQueryFilter filter = dtQueryFilter();
    dtPolyRef polyRef = INVALID_POLYREF;
    navmeshquery->findNearestPoly(location, extents, &filter, &polyRef, NULL);

    if (polyRef == INVALID_POLYREF)
        SendSysMessage("Dt     [??,??] (invalid poly, probably no tile loaded)");
    else
    {
        const dtMeshTile* tile;
        const dtPoly* poly;
        navmesh->getTileAndPolyByRef(polyRef, &tile, &poly);
        if (tile)
            PSendSysMessage("Dt     [%02i,%02i]", tile->header->x, tile->header->y);
        else
            SendSysMessage("Dt     [??,??] (no tile loaded)");
    }

    return true;
}

bool ChatHandler::HandleMmapLoadedTilesCommand(const char* /*args*/)
{
    uint32 mapid = m_session->GetPlayer()->GetMapId();

    const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(mapid);
    const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(mapid, m_session->GetPlayer()->GetAnyInstanceId());
    if (!navmesh || !navmeshquery)
    {
        SendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    SendSysMessage("mmap loadedtiles:");

    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        PSendSysMessage("[%02i,%02i]", tile->header->x, tile->header->y);
    }

    return true;
}

bool ChatHandler::HandleMmapStatsCommand(const char* /*args*/)
{
    SendSysMessage("mmap stats:");
    PSendSysMessage("  global mmap pathfinding is %sabled", sWorld.getConfig(CONFIG_MMAP_ENABLED) ? "en" : "dis");

    MMAP::MMapManager *manager = MMAP::MMapFactory::createOrGetMMapManager();
    PSendSysMessage(" %u maps loaded with %u tiles overall", manager->getLoadedMapsCount(), manager->getLoadedTilesCount());

    const dtNavMesh* navmesh = manager->GetNavMesh(m_session->GetPlayer()->GetMapId());
    if (!navmesh)
    {
        SendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    uint32 tileCount = 0;
    uint32 nodeCount = 0;
    uint32 polyCount = 0;
    uint32 vertCount = 0;
    uint32 triCount = 0;
    uint32 triVertCount = 0;
    uint32 dataSize = 0;
    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        tileCount ++;
        nodeCount += tile->header->bvNodeCount;
        polyCount += tile->header->polyCount;
        vertCount += tile->header->vertCount;
        triCount += tile->header->detailTriCount;
        triVertCount += tile->header->detailVertCount;
        dataSize += tile->dataSize;
    }

    SendSysMessage("Navmesh stats on current map:");
    PSendSysMessage(" %u tiles loaded", tileCount);
    PSendSysMessage(" %u BVTree nodes", nodeCount);
    PSendSysMessage(" %u polygons (%u vertices)", polyCount, vertCount);
    PSendSysMessage(" %u triangles (%u vertices)", triCount, triVertCount);
    PSendSysMessage(" %.2f MB of data (not including pointers)", ((float)dataSize / sizeof(unsigned char)) / 1048576);

    return true;
}

bool ChatHandler::HandleMmapOffsetCreateCommand(const char* /*args*/)
{
    Unit* target = getSelectedUnit();
    if (target == NULL)
        return false;

    Player* player = m_session->GetPlayer();
    int32 gx = 32 - player->GetPositionX() / SIZE_OF_GRIDS;
    int32 gy = 32 - player->GetPositionY() / SIZE_OF_GRIDS;

    player->Say("we have target", LANG_UNIVERSAL);

    std::ofstream file;
    file.open("mmaps.offmesh", std::ios_base::out | std::ios_base::app);
    if (file.fail())
    {
        player->Say("no file found", LANG_UNIVERSAL);
        return false;
    }

    // YEP THEY are swapped itnernally, mmap filename is mapidgxgy.mmap offmesh use gygx order
    file << player->GetMapId() << " " << gy << "," << gx << " ("
         << player->GetPositionX() << " "
         << player->GetPositionY() << " "
         << player->GetPositionZ() << ")" << " " << "("
         << target->GetPositionX() << " "
         << target->GetPositionY() << " "
         << target->GetPositionZ() << ") 2.5" << std::endl;

    return true;
}

bool ChatHandler::HandleGuildDisableAnnounceCommand(const char *args)
{
    if (!args)
        return false;

    std::string guildName = args;

    Guild* guild = sGuildMgr.GetGuildByName(guildName);
    if (!guild)
        return false;

    guild->AddFlag(GUILD_FLAG_DISABLE_ANN);
    guild->BroadcastToGuild(m_session, "Guild announce system has been disabled for that guild", LANG_UNIVERSAL);
    PSendSysMessage("Guild announce system has been disabled for guild %s", guildName.c_str());

    return true;
}

bool ChatHandler::HandleGuildEnableAnnounceCommand(const char *args)
{
    if (!args)
        return false;

    std::string guildName = args;

    Guild* guild = sGuildMgr.GetGuildByName(guildName);
    if (!guild)
        return false;

    guild->RemoveFlag(GUILD_FLAG_DISABLE_ANN);
    guild->BroadcastToGuild(m_session, "Guild announce system has been enabled for that guild", LANG_UNIVERSAL);
    PSendSysMessage("Guild announce system has been enabled for guild %s", guildName.c_str());

    return true;
}

bool ChatHandler::HandleTrollmuteCommand(const char* args)
{
    if (!*args)
        return false;

    char *charname = strtok((char*)args, " ");
    if (!charname)
        return false;

    std::string cname = charname;

    char *timetonotspeak = strtok(NULL, " ");
    if (!timetonotspeak || !atoi(timetonotspeak))
        return false;

    // check if there is specificator. there should be none
    char* testSp = timetonotspeak;
    while (*testSp/*not end of the line*/)
    {
        if (!isdigit(*testSp))
        {
            SendSysMessage("Do not use specificator. Mute commands take minutes as arguements.");
            SetSentErrorMessage(true);
            return false;
        }

        ++testSp;
    }

    char *mutereason = strtok(NULL, "");
    if (!mutereason)
    {
        SendSysMessage("Reason must be set");
        SetSentErrorMessage(true);
        return false;
    }

    std::string mutereasonstr = mutereason;

    if (!std::regex_search(mutereasonstr, std::regex("^\\d+\\.\\d+ \\(\\d+\\)$")))
    {
        SendSysMessage("Specify violated rule and the number of repetitions. Example: 1.4 (1), 2.4 (22), etc...");
        SetSentErrorMessage(true);
        return false;
    }

    uint32 notspeaktime = (uint32) atoi(timetonotspeak);

    if (notspeaktime == 0)
        return false;

    if (!normalizePlayerName(cname))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = sObjectMgr.GetPlayerGUIDByName(cname.c_str());
    if (!guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = sObjectAccessor.GetPlayerInWorldOrNot(guid);
    if (chr && !chr->IsInWorld())
    {
        SendSysMessage(LANG_PLAYER_LOADING_WAIT);
        return true;
    }

    // check security
    uint32 account_id = 0;
    uint32 permissions = 0;

    if (chr)
    {
        account_id = chr->GetSession()->GetAccountId();
        permissions = chr->GetSession()->GetPermissions();
    }
    else
    {
        account_id = sObjectMgr.GetPlayerAccountIdByGUID(guid);
        permissions = AccountMgr::GetPermissions(account_id);
    }

    if (m_session && permissions >= m_session->GetPermissions())
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return false;
    }

    AccountsDatabase.escape_string(mutereasonstr);

    WorldSession* targSess = chr ? chr->GetSession() : sWorld.FindSession(account_id);
    // notspeaktime - minutes
    if (targSess)
    {
        if (targSess->m_trollMuteRemain > notspeaktime * 60000) // cannot set mutetime lower than there already is.
                                                           // When selecting from DB at login it ALWAYS selects the highest mutetime
        {
            SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
            SetSentErrorMessage(true);
            return false;
        }
        targSess->m_trollMuteRemain = notspeaktime * 60000; // m_trollMuteRemain - miliseconds
        targSess->m_trollmuteReason = mutereasonstr;
        if (chr)
            ChatHandler(chr).PSendSysMessage(LANG_YOUR_CHAT_DISABLED, notspeaktime, mutereasonstr.c_str());
    }

    std::string author;

    if (m_session)
        author = m_session->GetPlayerName();
    else
        author = "[CONSOLE]";

    AccountsDatabase.escape_string(author);

    AccountsDatabase.PExecute("INSERT IGNORE INTO account_punishment VALUES ('%u', '%u', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, '%s', '%s', '1', '%u')",
                              account_id, PUNISHMENT_TROLLMUTE, notspeaktime*60, author.c_str(), mutereasonstr.c_str(), uint32(notspeaktime*60));

    SendGlobalGMSysMessage(LANG_GM_TROLLMUTED_PLAYER, author.c_str(), cname.c_str(), notspeaktime, mutereasonstr.c_str());

    if (!(m_session->GetPermissions() & 0x2000/*admin*/ || m_session->GetPermissions() == 0x9/*moderator*/) && sWorld.getConfig(CONFIG_SHOW_KICK_IN_WORLD) == 1)
    {
        sWorld.SendWorldText((m_session->GetPermissions() & 0x400/*GM*/) ? LANG_ANNOUNCE_TROLLMUTE_GM : LANG_ANNOUNCE_TROLLMUTE_EM,
            0, author.c_str(), cname.c_str(), timetonotspeak, mutereasonstr.c_str());
    }

    return true;
}

bool ChatHandler::HandleTrollmuteInfoCommand(const char* args)
{
    if (!args)
        return false;

    char* cname = strtok ((char*)args, "");
    if (!cname)
        return false;

    std::string name = cname;
    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sObjectMgr.GetPlayerAccountIdByPlayerName(name);
    if (!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if (!AccountMgr::GetName(accountid,accountname))
    {
        SendSysMessage(LANG_MUTEINFO_NOCHARACTER);
        return true;
    }

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT FROM_UNIXTIME(punishment_date), expiration_date-punishment_date, expiration_date, reason, punished_by, active, muteRemain, punishment_date "
                                    "FROM account_punishment "
                                    "WHERE account_id = '%u' AND punishment_type_id = '%u' "
                                    "ORDER BY punishment_date ASC", accountid, PUNISHMENT_TROLLMUTE);

    if (!result)
    {
        PSendSysMessage(LANG_MUTEINFO_NOACCOUNT_TROLLMUTE, accountname.c_str());
        return true;
    }

    PSendSysMessage(LANG_MUTEINFO_TROLLMUTE_HISTORY, accountname.c_str());
    do
    {
        Field* fields = result->Fetch();

        time_t unmutedate = time_t(fields[2].GetUInt64());
        uint64 muteLength = fields[1].GetUInt64();

        bool active = false;
        if ((muteLength == 0 || unmutedate >= time(NULL) || fields[6].GetUInt32() != 0) && fields[5].GetBool())
            active = true;

        uint32 date_id = fields[7].GetUInt32();

        std::string mutetime = WorldSession::secondsToTimeString(m_session, muteLength, true);
        PSendSysMessage(LANG_MUTEINFO_HISTORYENTRY,
            date_id, fields[0].GetString(), mutetime.c_str(), active ? GetHellgroundString(LANG_YES) : GetHellgroundString(LANG_JUST_NO), fields[3].GetString(), fields[4].GetString());
        PSendSysMessage("This mute left duration: %s", (WorldSession::secondsToTimeString(m_session, fields[6].GetUInt32(), true)).c_str());
    }
    while (result->NextRow());

    return true;
}

bool ChatHandler::HandleNpcDebugAICommand(const char* args)
{
    Creature* pCreature = getSelectedCreature();
    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    CreatureAI* ai = pCreature->AI();
    if (!ai)
    {
        SendSysMessage("no AI detected");
        SetSentErrorMessage(true);
        return false;
    }
    
    if (strcmp(args, "on") == 0)
    {
        ai->ToggleDebug(m_session->GetPlayer()->GetGUID());
        SendSysMessage(LANG_DONE);
    }
    else if (strcmp(args, "off") == 0)
    {
        ai->ToggleDebug(0);
        SendSysMessage(LANG_DONE);
    }
    else
        ai->GetDebugInfo(*this);
    return true;
}

bool ChatHandler::HandleBgEventCommand(const char* args)
{
    char *new_event_c = strtok((char*)args, " ");
    if (!new_event_c || !atoi(new_event_c))
        return false;

    uint16 new_event = (uint16)atoi(new_event_c);

    time_t t = time(NULL);
    tm* aTm = localtime(&t);

    sWorld.SelectNextBGEvent(aTm->tm_hour, new_event);
    PSendSysMessage("Tried to change bg event to %u (AB 150, ES 151, WS 152, AV 153)", new_event);
    return true;
}

//finditem delitem command
std::string GetItemLink(Player* player, Item* item)
{
    // LOCALE_enUS = 0,
    // LOCALE_koKR = 1,
    // LOCALE_frFR = 2,
    // LOCALE_deDE = 3,
    // LOCALE_zhCN = 4,
    // LOCALE_zhTW = 5,
    // LOCALE_esES = 6,
    // LOCALE_esMX = 7,
    // LOCALE_ruRU = 8

    int loc_idx = player ? player->GetSession()->GetSessionDbLocaleIndex() : 0; // if player is offline use 0
    const ItemPrototype* temp = item->GetProto();
    std::string name = temp->Name1;
    if (ItemLocale const* il = sObjectMgr.GetItemLocale(temp->ItemId))
        sObjectMgr.GetItemLocaleStrings(temp->ItemId, loc_idx, &name);

    if (int32 itemRandPropId = item->GetItemRandomPropertyId())
    {
        char* const* suffix = NULL;
        if (itemRandPropId < 0)
        {
            const ItemRandomSuffixEntry* itemRandEntry = sItemRandomSuffixStore.LookupEntry(-item->GetItemRandomPropertyId());
            if (itemRandEntry)
                suffix = itemRandEntry->name;
        }
        else
        {
            const ItemRandomPropertiesEntry* itemRandEntry = sItemRandomPropertiesStore.LookupEntry(item->GetItemRandomPropertyId());
            if (itemRandEntry)
                suffix = itemRandEntry->nameSuffix;
        }
        if (suffix)
        {
            //std::string test(suffix[(name != temp->Name1) ? loc_idx : DEFAULT_LOCALE]);
            //if (!test.empty())
            //{
            name += ' ';
            name += suffix[LOCALE_enUS];
            /*}*/
        }
    }

    std::ostringstream oss;
    oss << "|c" << std::hex << ItemQualityColors[temp->Quality] << std::dec <<
        "|Hitem:" << temp->ItemId << ":" <<
        item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) << ":" <<
        item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT) << ":" <<
        item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_2) << ":" <<
        item->GetEnchantmentId(SOCK_ENCHANTMENT_SLOT_3) << ":" <<
        item->GetEnchantmentId(BONUS_ENCHANTMENT_SLOT) << ":" <<
        item->GetItemRandomPropertyId() << ":" << item->GetItemSuffixFactor() << "|h[" << name << "]|h|r";

    return oss.str();
}

bool ChatHandler::HandleLookGMItemCommand(const char* args)
{
    if (!*args)
        return false;

    char *Name = strtok((char*)args, " ");
    char *ArgItemID = strtok(NULL, " ");

    if (!ArgItemID || !Name)
    {
        PSendSysMessage("ERROR! USE: .finditem PlayerName ItemID");
        SetSentErrorMessage(true);
        return false;
    }

    std::string PlayerName = Name;
    uint64 PlayerGUID = sObjectMgr.GetPlayerGUIDByName(PlayerName);
    if (!PlayerGUID || PlayerGUID == 0 || !normalizePlayerName(PlayerName))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 ItemId = uint32(atoi(ArgItemID));

    Player* target = NULL;
    target = ObjectAccessor::GetPlayerInWorldOrNot(PlayerGUID);

	if (target)
		target->SaveToDB();

    std::vector<Item*> itemList;

	QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT item FROM character_inventory WHERE guid=%u AND item_template=%u", (uint32)PlayerGUID, ItemId);
	if (!result)
	{
		PSendSysMessage("Player has no such items");
		SetSentErrorMessage(true);
		return false;
	}

	std::list<uint64> ItemGuidList;
	do
	{
		Field* fields = result->Fetch();
		ItemGuidList.push_back(fields[0].GetUInt64());
	} while (result->NextRow());

	for (std::list<uint64>::iterator itr = ItemGuidList.begin(); itr != ItemGuidList.end(); ++itr)
	{
		ItemPrototype const* proto = ObjectMgr::GetItemPrototype(ItemId);
		Item* item = NewItemOrBag(proto);

		if (item->LoadFromDB(*itr, PlayerGUID))
			itemList.push_back(item);
	}

    if (itemList.empty())
    {
        PSendSysMessage("Player has no such items");
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage("Player %s has items with id %u:", PlayerName.c_str(), ItemId);
    for (std::vector<Item*>::iterator itr = itemList.begin(); itr != itemList.end(); ++itr)
    {
        Item* item = (*itr);
        PSendSysMessage("%s - GUID: %u (count %u)", GetItemLink(0, item).c_str(), item->GetGUIDLow(), item->GetCount());
    }
    //PSendSysMessage("End of item(id %u) list for player %s", ItemId, PlayerName.c_str());

    return true;
}

bool ChatHandler::HandleDeleteGMItemCommand(const char* args)
{
    if (!*args)
        return false;

    char *ArgName = strtok((char*)args, " ");
    char *ArgItemGUID = strtok(NULL, " ");
	char *ArgItemCount = strtok(NULL, " ");

    if (!ArgItemGUID || !ArgName)
    {
        PSendSysMessage("ERROR! USE: .delitem PlayerName ItemGUID ItemCount(default: 1)");
        SetSentErrorMessage(true);
        return false;
    }

	uint32 count = 1;
	if (ArgItemCount)
		count = uint32(atoi(ArgItemCount));

    std::string PlayerName = ArgName;
    uint64 targetGUID = sObjectMgr.GetPlayerGUIDByName(PlayerName);
    if (!targetGUID || targetGUID == 0 || !normalizePlayerName(PlayerName))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

	uint32 itemGUID = uint32(atoi(ArgItemGUID));

	m_session->GetPlayer()->RemoveItemCount(targetGUID, itemGUID, count);
    return true;
}

bool ChatHandler::HandleModifyStrengthCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetModifierValue(UNIT_MOD_STAT_STRENGTH, BASE_VALUE, (float)amount);
    pTarget->UpdateAllStats();

    PSendSysMessage("You set strength of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyAgilityCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetModifierValue(UNIT_MOD_STAT_AGILITY, BASE_VALUE, (float)amount);
    pTarget->UpdateAllStats();

    PSendSysMessage("You set agility of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyStaminaCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetModifierValue(UNIT_MOD_STAT_STAMINA, BASE_VALUE, (float)amount);
    pTarget->UpdateAllStats();

    if (pTarget->isAlive())
        pTarget->SetHealth(pTarget->GetMaxHealth());

    PSendSysMessage("You set stamina of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyIntellectCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetModifierValue(UNIT_MOD_STAT_INTELLECT, BASE_VALUE, (float)amount);
    pTarget->UpdateAllStats();

    PSendSysMessage("You set intellect of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifySpiritCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetModifierValue(UNIT_MOD_STAT_SPIRIT, BASE_VALUE, (float)amount);
    pTarget->UpdateAllStats();

    PSendSysMessage("You set spirit of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyArmorCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetInt32Value(UNIT_FIELD_RESISTANCES, amount);

    PSendSysMessage("You set armor of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyHolyCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    pTarget->SetInt32Value(UNIT_FIELD_RESISTANCES + 1, amount);

    PSendSysMessage("You set holy resist of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyFireCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    pTarget->SetInt32Value(UNIT_FIELD_RESISTANCES + 2, amount);

    PSendSysMessage("You set fire resist of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyNatureCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    pTarget->SetInt32Value(UNIT_FIELD_RESISTANCES + 3, amount);

    PSendSysMessage("You set nature resist of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyFrostCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    pTarget->SetInt32Value(UNIT_FIELD_RESISTANCES + 4, amount);

    PSendSysMessage("You set frost resist of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyShadowCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    pTarget->SetInt32Value(UNIT_FIELD_RESISTANCES + 5, amount);

    PSendSysMessage("You set shadow resist of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyArcaneCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    pTarget->SetInt32Value(UNIT_FIELD_RESISTANCES + 6, amount);

    PSendSysMessage("You set arcane resist of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyMeleeApCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = pTarget->GetInt32Value(UNIT_FIELD_ATTACK_POWER) + atoi((char*)args);

    pTarget->SetInt32Value(UNIT_FIELD_ATTACK_POWER, amount);
    pTarget->UpdateDamagePhysical(BASE_ATTACK);
    pTarget->UpdateDamagePhysical(OFF_ATTACK);

    PSendSysMessage("You set attack power of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyRangedApCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount <= 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, amount);
    pTarget->UpdateDamagePhysical(RANGED_ATTACK);

    PSendSysMessage("You set ranged attack power of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifySpellPowerCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);
    if (amount < 0)
    {
        pTarget->RemoveAurasDueToSpell(18058);
        PSendSysMessage("All modified spell power removed");
        return true;
    }

    // dunno where spell power is stored so using a custom spell
    pTarget->RemoveAurasDueToSpell(18058);
    pTarget->CastCustomSpell(pTarget, 18058, &amount, &amount, &amount, true);

    PSendSysMessage("You set spell power of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyMeleeCritCommand(const char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();

    if (!player)
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    float amount = (float)atof((char*)args);

    if (amount < 0 || amount > 100)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    player->SetStatFloatValue(PLAYER_CRIT_PERCENTAGE, amount);

    PSendSysMessage("You set melee crit chance of %s to %g.", player->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyRangedCritCommand(const char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();

    if (!player)
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    float amount = (float)atof((char*)args);

    if (amount < 0 || amount > 100)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    player->SetStatFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE, amount);

    PSendSysMessage("You set ranged crit chance of %s to %g.", player->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifySpellCritCommand(const char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();

    if (!player)
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    float amount = (float)atof((char*)args);

    if (amount < 0 || amount > 100)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    player->SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_NORMAL, amount);
    player->SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_HOLY, amount);
    player->SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FIRE, amount);
    player->SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_NATURE, amount);
    player->SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_FROST, amount);
    player->SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_SHADOW, amount);
    player->SetStatFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1 + SPELL_SCHOOL_ARCANE, amount);

    PSendSysMessage("You set spell crit chance of %s to %g.", player->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyMainSpeedCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount <= 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetFloatValue(UNIT_FIELD_BASEATTACKTIME, (float)amount);

    PSendSysMessage("You set main hand attack speed of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyOffSpeedCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount <= 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetFloatValue(UNIT_FIELD_BASEATTACKTIME + 1, (float)amount);

    PSendSysMessage("You set off hand attack speed of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyRangedSpeedCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = atoi((char*)args);

    if (amount <= 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetFloatValue(UNIT_FIELD_RANGEDATTACKTIME, (float)amount);

    PSendSysMessage("You set ranged attack speed of %s to %i.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyCastSpeedCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* pTarget = getSelectedUnit();

    if (!pTarget)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    float amount = (float)atof((char*)args);

    if (amount < 0)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    pTarget->SetFloatValue(UNIT_MOD_CAST_SPEED, amount);

    PSendSysMessage("You set cast speed of %s to %g times normal.", pTarget->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyBlockCommand(const char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();

    if (!player)
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    float amount = (float)atof((char*)args);

    if (amount < 0 || amount > 100)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    player->SetStatFloatValue(PLAYER_BLOCK_PERCENTAGE, amount);

    PSendSysMessage("You set block chance of %s to %g.", player->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyDodgeCommand(const char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();

    if (!player)
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    float amount = (float)atof((char*)args);

    if (amount < 0 || amount > 100)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    player->SetStatFloatValue(PLAYER_DODGE_PERCENTAGE, amount);

    PSendSysMessage("You set dodge chance of %s to %g.", player->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyParryCommand(const char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();

    if (!player)
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    float amount = (float)atof((char*)args);

    if (amount < 0 || amount > 100)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    player->SetStatFloatValue(PLAYER_PARRY_PERCENTAGE, amount);

    PSendSysMessage("You set parry chance of %s to %g.", player->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyCrCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();
    if (!target) return false;
    float f = (float)atof((char*)args);

    target->SetFloatValue(UNIT_FIELD_COMBATREACH, f);
    return true;
}

bool ChatHandler::HandleModifyBrCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();
    if (!target) return false;
    float f = (float)atof((char*)args);

    target->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, f);
    return true;
}