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

/** \file
    \ingroup world
*/

#include "Common.h"
//#include "WorldSocket.h"
#include "Database/DatabaseEnv.h"
#include "Config/Config.h"
#include "SystemConfig.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "Weather.h"
#include "Player.h"
#include "SkillExtraItems.h"
#include "SkillDiscovery.h"
#include "World.h"
#include "AccountMgr.h"
#include "AuctionHouseMgr.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "LootMgr.h"
#include "ItemEnchantmentMgr.h"
#include "MapManager.h"
#include "ScriptMgr.h"
#include "CreatureAIRegistry.h"
#include "BattleGroundMgr.h"
#include "OutdoorPvPMgr.h"
#include "TemporarySummon.h"
#include "WaypointMovementGenerator.h"
#include "VMapFactory.h"
#include "movemap/MoveMap.h"
#include "GameEvent.h"
#include "PoolManager.h"
#include "Database/DatabaseImpl.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "InstanceSaveMgr.h"
#include "TicketMgr.h"
#include "Util.h"
#include "Language.h"
#include "CreatureGroups.h"
#include "Transports.h"
#include "CreatureEventAIMgr.h"
#include "WardenDataStorage.h"
#include "WorldEventProcessor.h"
#include "ArenaTeam.h"
#include "Chat.h"
#include "Shop.h"
#include "SocialMgr.h"

#include <thread>
#include <chrono>

//#include "Timer.h"
#include "GuildMgr.h"
#include <unordered_set>

volatile bool World::m_stopEvent = false;
uint8 World::m_ExitCode = SHUTDOWN_EXIT_CODE;
volatile uint32 World::m_worldLoopCounter = 0;

float World::m_VisibleObjectGreyDistance      = 0;

int32 World::m_activeObjectUpdateDistanceOnContinents = DEFAULT_VISIBILITY_DISTANCE;
int32 World::m_activeObjectUpdateDistanceInInstances = DEFAULT_VISIBILITY_DISTANCE;

void MapUpdateDiffInfo::InitializeMapData()
{
    for(MapManager::MapMapType::const_iterator map = sMapMgr.Maps().begin(); map != sMapMgr.Maps().end(); ++map)
    {
        if (_cumulativeDiffInfo.find(map->first.nMapId) == _cumulativeDiffInfo.end())
        {
            _cumulativeDiffInfo[map->first.nMapId] = new atomic_uint[DIFF_MAX_CUMULATIVE_INFO];
            for (int i = DIFF_SESSION_UPDATE; i < DIFF_MAX_CUMULATIVE_INFO; i++)
                _cumulativeDiffInfo[map->first.nMapId][i] = 0;
        }
    }

}

void MapUpdateDiffInfo::PrintCumulativeMapUpdateDiff()
{
    for (CumulativeDiffMap::iterator itr = _cumulativeDiffInfo.begin(); itr != _cumulativeDiffInfo.end(); ++itr)
    {
        for (int i = DIFF_SESSION_UPDATE; i < DIFF_MAX_CUMULATIVE_INFO; i++)
        {
            uint32 diff = itr->second[i].value();
            if (diff >= (sWorld.getConfig(CONFIG_MIN_LOG_UPDATE)/10))
                sLog.outLog(LOG_DIFF, "Map[%u] diff for: %i - %u", itr->first, i, diff);
        }
    }


}

/// World constructor
World::World()
{
	m_loading = true;
	
	m_playerLimit = 0;
    m_requiredPermissionMask = PERM_PLAYER;
    m_allowMovement = true;
    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_gameTime=time(NULL);
    m_startTime=m_gameTime;
    m_maxActiveSessionCount = 0;
    m_maxQueuedSessionCount = 0;
    m_NextDailyQuestReset = 0;
    m_scheduledScripts = 0;

    m_defaultDbcLocale = LOCALE_enUS;
    m_availableDbcLocaleMask = 0;

    m_updateTimeSum = 0;
    m_updateTimeCount = 0;

    m_massMuteTime = 0;

    loggedInAlliances = 0;
    loggedInHordes = 0;

    // TODO: move to config
    m_honorRanks[0] = 20;
    m_honorRanks[1] = 100;
    m_honorRanks[2] = 250;
    m_honorRanks[3] = 500;
    m_honorRanks[4] = 1000;
    m_honorRanks[5] = 1800;
    m_honorRanks[6] = 3000;
    m_honorRanks[7] = 5000;
    m_honorRanks[8] = 8000;
    m_honorRanks[9] = 12000;
    m_honorRanks[10] = 18000;
    m_honorRanks[11] = 25000;
    m_honorRanks[12] = 36000;
    m_honorRanks[13] = 50000;

    m_guild_house_owner = 0;

    /*WSGBattles = 0;
    ArathiBattles = 0;
    EyeOfTheStormBattles = 0;*/

    //noModCreatures.clear();
    LogOnlyOnce.clear();
    inGameShop = new Shop();

    memset(_maxLvlReachedPerClass, 0, sizeof(_maxLvlReachedPerClass));
    _minorityPropagandaPkt = NULL;

    //CanEnableAlterac[0] = true; // afternoon
    //CanEnableAlterac[1] = true; // evening

    warningTimer = 0;

    online.fake = 0;
    online.weekly_max = 0;

    fakebot_account_number = 0x0FFFFFFF;
}

/// World destructor
World::~World()
{
    ///- Empty the kicked session set
    while (!m_sessions.empty())
    {
        // not remove from queue, prevent loading new sessions
        WorldSession *temp = m_sessions.begin()->second;
        m_sessions.erase(m_sessions.begin());
        delete temp;
    }

    // Here event list SHOULD be already empty, but who knows what can happen :p
    sWorldEventProcessor.DestroyEvents();

    ///- Empty the WeatherMap
    for (WeatherMap::iterator itr = m_weathers.begin(); itr != m_weathers.end(); ++itr)
        delete itr->second;

    m_weathers.clear();

    CliCommandHolder* command;
    while (cliCmdQueue.next(command))
        delete command;


    VMAP::VMapFactory::clear();
    MMAP::MMapFactory::clear();

    delete inGameShop;
    //TODO free addSessQueue
}

/// Find a player in a specified zone
Player* World::FindPlayerInZone(uint32 zone)
{
    ///- circle through active sessions and return the first player found in the zone
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second)
            continue;
        Player *player = itr->second->GetPlayer();
        if (!player)
            continue;
        if (player->IsInWorld() && player->GetCachedZone() == zone)
        {
            // Used by the weather system. We return the player to broadcast the change weather message to him and all players in the zone.
            return player;
        }
    }
    return NULL;
}

/// Find a session by its id
WorldSession* World::FindSession(uint32 id) const
{
    SessionMap::const_iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end())
        return itr->second;                                 // also can return NULL for kicked session
    else
        return NULL;
}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
    ///- Find the session, kick the user, but we can't delete session at this moment to prevent iterator invalidation
    SessionMap::iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end() && itr->second)
    {
        if (itr->second->PlayerLoading())
            return false;
        itr->second->KickPlayer();
    }

    return true;
}

void World::AddSession(WorldSession* s)
{
    addSessQueue.add(s);
}

void World::AddSession_ (WorldSession* s)
{
    if (!s)
        return;

    //NOTE - Still there is race condition in WorldSession* being used in the Sockets

    ///- kick already loaded player with same account (if any) and remove session
    ///- if player is in loading and want to load again, return
    if (!RemoveSession (s->GetAccountId ()))
    {
        s->KickPlayer ();
        delete s;                                           // session not added yet in session list, so not listed in queue
        return;
    }

    //m_ac_auth[s->GetRemoteAddress()] = 15000;

    // decrease session counts only at not reconnection case
    bool decrease_session = true;

    // if session already exist, prepare to it deleting at next world update
    // NOTE - KickPlayer() should be called on "old" in RemoveSession()
    {
        SessionMap::const_iterator old = m_sessions.find(s->GetAccountId ());

        if (old != m_sessions.end())
        {
            // prevent decrease sessions count if session queued
            if (RemoveQueuedPlayer(old->second))
                decrease_session = false;
            // not remove replaced session form queue if listed
            delete old->second;
        }
    }

    uint32 accId = s->GetAccountId();

    m_sessions[accId] = s;

    QueryResultAutoPtr externalAction = AccountsDatabase.PQuery("SELECT ea.`id`, ea.`action`, ea.`data` FROM `external_action` ea WHERE ea.`target`='%u'", accId);
    if (externalAction)
    {
        do
        {
            Field* fields = externalAction->Fetch();
            uint32 action_id_del = fields[0].GetUInt32();
            uint32 action = fields[1].GetUInt32();
            uint32 data = fields[2].GetUInt32();

            switch (action)
            {
                case EXT_ACT_ACC_ADD_FLAG:
                {
                    s->AddAccountFlag((AccountFlags)data);
                    break;
                }
                case EXT_ACT_ACC_DEL_MUTE:
                {
                    s->m_muteRemain = 0;
                    s->m_muteReason = "";
                    AccountsDatabase.PExecute("UPDATE account_punishment SET muteRemain ='0', active = '0' WHERE account_id = '%u' AND punishment_type_id = '%u' AND active = 1 AND muteRemain != '0'", accId, PUNISHMENT_MUTE);
                    break;
                }
                case EXT_ACT_ACC_DEL_TMUTE:
                {
                    s->m_trollMuteRemain = 0;
                    s->m_trollmuteReason = "";
                    AccountsDatabase.PExecute("UPDATE account_punishment SET muteRemain ='0', active = '0' WHERE account_id = '%u' AND punishment_type_id = '%u' AND active = 1 AND muteRemain != '0'", accId, PUNISHMENT_TROLLMUTE);
                    break;
                }
                case EXT_ACT_ACC_ADD_PREM:
                {
                    s->addPremiumTime(data);
                    break;
                }
                default:
                {
                    sLog.outLog(LOG_CRITICAL, "external_action unhandled action %u (target %u, data %u), nothing done and deleted", action, accId, data);
                    break;
                }
            }
            AccountsDatabase.PExecute("DELETE FROM `external_action` WHERE `id`='%u'", action_id_del);
        } while (externalAction->NextRow());
    }

    uint32 Sessions = GetActiveAndQueuedSessionCount ();
    uint32 QueueSize = GetQueueSize (); //number of players in the queue

    //so we don't count the user trying to
    //login as a session and queue the socket that we are using
    if (decrease_session)
        --Sessions;

    if (m_playerLimit > 0 && Sessions >= m_playerLimit && !s->HasPermissions(PERM_GMT_HDEV))
    {
        if (!sObjectMgr.IsUnqueuedAccount(s->GetAccountId()) && !HasRecentlyDisconnected(s))
        {
            AddQueuedPlayer (s);
            UpdateMaxSessionCounters ();
            sLog.outDetail ("PlayerQueue: Account id %u is in Queue Position (%u).", s->GetAccountId (), ++QueueSize);
            return;
        }
    }

    WorldPacket packet(SMSG_AUTH_RESPONSE, 1 + 4 + 1 + 4 + 1);
    packet << uint8(AUTH_OK);
    packet << uint32(0);                        // unknown random value...
    packet << uint8(0);
    packet << uint32(0);
    packet << uint8(s->Expansion());            // 0 - normal, 1 - TBC, must be set in database manually for each account
    s->SendPacket (&packet);

    UpdateMaxSessionCounters();

    // Updates the population
    //if (m_playerLimit > 0)
    //{
    //    float popu = GetActiveSessionCount (); //updated number of users on the server
    //    popu /= m_playerLimit;
    //    popu *= 2;
    //    AccountsDatabase.PExecute ("UPDATE realms SET population = '%f' WHERE realm_id = '%u'", popu, realmID);
    //    sLog.outDetail ("Server Population (%f).", popu);
    //}
}

bool World::HasRecentlyDisconnected(WorldSession* session)
{
    if (!session)
        return false;

    if (uint32 tolerance = getConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
    {
        for (DisconnectMap::iterator next, i = m_disconnects.begin(); i != m_disconnects.end(); i = next)
        {
            next = i;
            next++;

            if (i->first == session->GetAccountId())
            {
                if (difftime(i->second, time(NULL)) <= tolerance)
                    return true;
                else
                    m_disconnects.erase(i);
            }
        }
    }
    return false;
 }

int32 World::GetQueuePos(WorldSession* sess)
{
    uint32 position = 1;

    for (Queue::iterator iter = m_QueuedPlayer.begin(); iter != m_QueuedPlayer.end(); ++iter, ++position)
        if ((*iter) == sess)
            return position;

    return 0;
}

void World::AddQueuedPlayer(WorldSession* sess)
{
    sess->SetInQueue(true);
    m_QueuedPlayer.push_back (sess);

    // The 1st SMSG_AUTH_RESPONSE needs to contain other info too.
    WorldPacket packet (SMSG_AUTH_RESPONSE, 1 + 4 + 1 + 4 + 1);
    packet << uint8 (AUTH_WAIT_QUEUE);
    packet << uint32 (0); // unknown random value...
    packet << uint8 (0);
    packet << uint32 (0);
    packet << uint8 (sess->Expansion () ? 1 : 0); // 0 - normal, 1 - TBC, must be set in database manually for each account
    packet << uint32(GetQueuePos (sess));
    sess->SendPacket (&packet);

    //sess->SendAuthWaitQue (GetQueuePos (sess));
}

bool World::RemoveQueuedPlayer(WorldSession* sess)
{
    // sessions count including queued to remove (if removed_session set)
    uint32 sessions = GetActiveSessionCount();

    uint32 position = 1;
    Queue::iterator iter = m_QueuedPlayer.begin();

    // search to remove and count skipped positions
    bool found = false;

    for (;iter != m_QueuedPlayer.end(); ++iter, ++position)
    {
        if (*iter == sess)
        {
            sess->SetInQueue(false);
            iter = m_QueuedPlayer.erase(iter);
            found = true;                                   // removing queued session
            break;
        }
    }

    // iter point to next socked after removed or end()
    // position store position of removed socket and then new position next socket after removed

    // if session not queued then we need decrease sessions count
    if (!found && sessions)
        --sessions;

    // accept first in queue
    if ((!m_playerLimit || (sessions < m_playerLimit)) && !m_QueuedPlayer.empty())
    {
        WorldSession* pop_sess = m_QueuedPlayer.front();
        pop_sess->SetInQueue(false);
        pop_sess->SendAuthWaitQue(0);
        m_QueuedPlayer.pop_front();

        // update iter to point first queued socket or end() if queue is empty now
        iter = m_QueuedPlayer.begin();
        position = 1;
    }

    // update position from iter to end()
    // iter point to first not updated socket, position store new position
    for (; iter != m_QueuedPlayer.end(); ++iter, ++position)
        (*iter)->SendAuthWaitQue(position);

    if (!found && getConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
    {
        uint32 accountId = sess->GetAccountId();
        time_t currentTime = time(NULL);

        std::pair<uint32, time_t> tPair = std::make_pair(accountId, currentTime);

        addDisconnectTime(tPair);
    }
    return found;
}

/// Find a Weather object by the given zoneid
Weather* World::FindWeather(uint32 id) const
{
    WeatherMap::const_iterator itr = m_weathers.find(id);

    if (itr != m_weathers.end())
        return itr->second;
    else
        return 0;
}

/// Remove a Weather object for the given zoneid
void World::RemoveWeather(uint32 id)
{
    // not called at the moment. Kept for completeness
    WeatherMap::iterator itr = m_weathers.find(id);

    if (itr != m_weathers.end())
    {
        Weather *temp = itr->second;
        m_weathers.erase(itr);
        delete temp;
    }
}

/// Add a Weather object to the list
Weather* World::AddWeather(uint32 zone_id)
{
    WeatherZoneChances const* weatherChances = sObjectMgr.GetWeatherChances(zone_id);

    // zone not have weather, ignore
    if (!weatherChances)
        return NULL;

    Weather* w = new Weather(zone_id,weatherChances);
    m_weathers[w->GetZone()] = w;
    w->ReGenerate();
    w->UpdateWeather();
    return w;
}

/// Initialize config values
void World::LoadConfigSettings(bool reload)
{
    if (reload && !sConfig.Reload())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: World settings reload fail: can't read settings from %s.",sConfig.GetFilename().c_str());
        return;
    }

    std::string dataPath = sConfig.GetStringDefault("DataDir", "./");
    if (dataPath.at(dataPath.length()-1)!='/' && dataPath.at(dataPath.length()-1)!='\\')
        dataPath.append("/");

    if (reload)
    {
        if (dataPath!=m_dataPath)
            sLog.outLog(LOG_DEFAULT, "ERROR: DataDir option can't be changed at .conf file reload, using current value (%s).",m_dataPath.c_str());
    }
    else
    {
        m_dataPath = dataPath;
        sLog.outString("Using DataDir %s",m_dataPath.c_str());
    }

    // === Load section ===
    // Performance settings
    SetPlayerLimit(sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT));
    loadConfig(CONFIG_ADDON_CHANNEL, "AddonChannel", false);
    loadConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY, "SaveRespawnTimeImmediately", true);
    loadConfig(CONFIG_GRID_UNLOAD, "GridUnload", true);

    loadConfig(CONFIG_INTERVAL_CHANGEWEATHER, "ChangeWeatherInterval", 600000);
    loadConfig(CONFIG_INTERVAL_SAVE, "PlayerSaveInterval", 900000);
    loadConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE, "DisconnectToleranceInterval", 0);

    loadConfig(CONFIG_NUMTHREADS, "MapUpdate.Threads", 1);
    if (m_configs[CONFIG_NUMTHREADS] < 1)
        m_configs[CONFIG_NUMTHREADS] = 1;
    loadConfig(CONFIG_MAPUPDATE_MAXVISITORS, "MapUpdate.UpdateVisitorsMax", 0);
    loadConfig(CONFIG_CUMULATIVE_LOG_METHOD, "MapUpdate.CumulativeLogMethod", 0);

    loadConfig(CONFIG_SESSION_UPDATE_MAX_TIME, "SessionUpdate.MaxTime", 1000);
    loadConfig(CONFIG_SESSION_UPDATE_OVERTIME_METHOD, "SessionUpdate.Method", 3);
    loadConfig(CONFIG_SESSION_UPDATE_VERBOSE_LOG, "SessionUpdate.VerboseLog", 0);
    loadConfig(CONFIG_SESSION_UPDATE_IDLE_KICK, "SessionUpdate.IdleKickTimer", 15*MINUTE*MILLISECONDS);
    loadConfig(CONFIG_SESSION_UPDATE_MIN_LOG_DIFF, "SessionUpdate.MinLogDiff", 25);
    loadConfig(CONFIG_INTERVAL_LOG_UPDATE, "RecordUpdateTimeDiffInterval", 60000);
    loadConfig(CONFIG_MIN_LOG_UPDATE, "MinRecordUpdateTimeDiff", 10);
    loadConfig(CONFIG_FASTBOOT, "Accounts.Fastboot", false);

    // Server settings
    /*if (m_configs[CONFIG_REALM_ZONE] == REALM_ZONE_RUSSIAN)
        m_configs[CONFIG_DECLINED_NAMES_USED] = true;
    else*/
        loadConfig(CONFIG_DECLINED_NAMES_USED, "DeclinedNames", false);
    loadConfig(CONFIG_LANG_PLAYER_NAMES, "LangPlayerNames",  LT_CYRILLIC);
    loadConfig(CONFIG_LANG_CHARTER_NAMES, "LangCharterNames", LT_CYRILLIC);
    loadConfig(CONFIG_LANG_PET_NAMES, "LangPetNames", LT_CYRILLIC);

    // Server customization basic
    loadConfig(CONFIG_CHARACTERS_CREATING_DISABLED, "CharactersCreatingDisabled", 0);
    loadConfig(CONFIG_START_ALL_TAXI_PATHS, "PlayerStart.AllFlightPaths", false);
    loadConfig(CONFIG_START_ALL_REP, "PlayerStart.AllReputation", false);
    loadConfig(CONFIG_START_ALL_SPELLS, "PlayerStart.AllSpells", false);
    loadConfig(CONFIG_START_ALL_EXPLORED, "PlayerStart.MapsExplored", false);
    SetNewCharString(sConfig.GetStringDefault("PlayerStart.String", ""));

    loadConfig(CONFIG_ALWAYS_MAX_WEAPON_SKILL, "AlwaysMaxWeaponSkill", false);
    loadConfig(CONFIG_CAST_UNSTUCK, "CastUnstuck", true);
    loadConfig(CONFIG_DAILY_BLIZZLIKE, "DailyQuest.Blizzlike", true);
    //loadConfig(CONFIG_DAILY_MAX_PER_DAY, "DailyQuest.MaxPerDay", 25);
    Season season = sWorld.getSeasonFromDB();
    switch (season)
    {
        case SEASON_1:
            m_configs[CONFIG_DAILY_MAX_PER_DAY] = 0;
            break;
		case SEASON_2:
        case SEASON_3:
            m_configs[CONFIG_DAILY_MAX_PER_DAY] = 10;
            break;
        case SEASON_4:
            m_configs[CONFIG_DAILY_MAX_PER_DAY] = 15;
            break;
        case SEASON_5:
            m_configs[CONFIG_DAILY_MAX_PER_DAY] = 25;
            break;
        default:
            m_configs[CONFIG_DAILY_MAX_PER_DAY] = 0;
            break;
    }

    loadConfig(CONFIG_DISABLE_DUEL, "DisableDuel", false);
    loadConfig(CONFIG_DISABLE_PVP, "DisablePVP", false);
    loadConfig(CONFIG_EVENT_ANNOUNCE, "EventAnnounce", 0);
    loadConfig(CONFIG_FFA_DISALLOWGROUP, "FFA.DisallowGroup", false);
    loadConfig(CONFIG_HONOR_AFTER_DUEL, "HonorPointsAfterDuel", 0);
    if (m_configs[CONFIG_HONOR_AFTER_DUEL] < 0)
        m_configs[CONFIG_HONOR_AFTER_DUEL]= 0;
    loadConfig(CONFIG_INSTANCE_IGNORE_LEVEL, "Instance.IgnoreLevel", false);
    loadConfig(CONFIG_INSTANCE_IGNORE_RAID, "Instance.IgnoreRaid", false);
    loadConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL, "MaxPrimaryTradeSkill", 2);
    SetMotd(sConfig.GetStringDefault("Motd", "Welcome to Moonwell server."));
    loadConfig(CONFIG_PVP_TOKEN_ENABLE, "PvPToken.Enable", false);
    loadConfig(CONFIG_PVP_TOKEN_ID, "PvPToken.ItemID", 29434);
    loadConfig(CONFIG_PVP_TOKEN_COUNT, "PvPToken.ItemCount", 1);
    if (m_configs[CONFIG_PVP_TOKEN_COUNT] < 1)
        m_configs[CONFIG_PVP_TOKEN_COUNT] = 1;
    loadConfig(CONFIG_PVP_TOKEN_MAP_TYPE, "PvPToken.MapAllowType", 4);
    loadConfig(CONFIG_ENABLE_SINFO_LOGIN, "Server.LoginInfo", 0);
    loadConfig(CONFIG_SHOW_KICK_IN_WORLD, "ShowKickInWorld", false);

    loadConfig(CONFIG_DONT_DELETE_CHARS, "DontDeleteChars", false);
    loadConfig(CONFIG_DONT_DELETE_CHARS_LVL, "DontDeleteCharsLvl", 40);
    loadConfig(CONFIG_KEEP_DELETED_CHARS_TIME, "KeepDeletedCharsTime", 31);

    // Server customization advanced
    loadConfig(CONFIG_WEATHER, "ActivateWeather",true);
    loadConfig(CONFIG_ENABLE_SORT_AUCTIONS, "Auction.EnableSort", true);
    loadConfig(CONFIG_AUTOBROADCAST_INTERVAL, "AutoBroadcast.Timer", 35*MINUTE*1000);
    loadConfig(CONFIG_GROUPLEADER_RECONNECT_PERIOD, "GroupLeaderReconnectPeriod", 180);
    loadConfig(CONFIG_INSTANCE_RESET_TIME_HOUR, "Instance.ResetTimeHour", 4);
    loadConfig(CONFIG_INSTANCE_UNLOAD_DELAY, "Instance.UnloadDelay", 1800000);
    loadConfig(CONFIG_MAIL_DELIVERY_DELAY, "Mail.DeliveryDelay", HOUR);
    loadConfig(CONFIG_EXTERNAL_MAIL, "Mail.External", 0);
    loadConfig(CONFIG_EXTERNAL_MAIL_INTERVAL, "Mail.ExternalInterval", 1);
    loadConfig(CONFIG_GM_MAIL, "Mail.GmInstantSend", 1);
    loadConfig(CONFIG_RETURNOLDMAILS_MODE, "Mail.OldReturnMode", 0);
    loadConfig(CONFIG_RETURNOLDMAILS_INTERVAL, "Mail.OldReturnTimer", 60);
    loadConfig(CONFIG_GROUP_XP_DISTANCE, "MaxGroupXPDistance", 74);
    loadConfig(CONFIG_NO_RESET_TALENT_COST, "NoResetTalentsCost", false);
    loadConfig(CONFIG_RABBIT_DAY, "Rabbit.Day", 954547200);
    loadConfig(CONFIG_FREE_RESPEC_COST, "FreeRespec.Cost", 0); // 0 -> disabled. By default is disabled
    loadConfig(CONFIG_FREE_RESPEC_DURATION, "FreeRespec.Duration", 6 * MONTH);
    loadConfig(CONFIG_SKILL_PROSPECTING, "SkillChance.Prospecting",false);

    // note: disable value (-1) will assigned as 0xFFFFFFF, to prevent overflow at calculations limit it to max possible player level MAX_LEVEL(100)
    loadConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF, "Quests.LowLevelHideDiff", 4);
    if (m_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] = MAX_LEVEL;
    loadConfig(CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF, "Quests.HighLevelHideDiff", 7);
    if (m_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] = MAX_LEVEL;

    m_forbiddenMapIds.clear();
    std::string forbiddenmaps = sConfig.GetStringDefault("ForbiddenMaps", "");
    char * forbiddenMaps = new char[forbiddenmaps.length() + 1];
    forbiddenMaps[forbiddenmaps.length()] = 0;
    strncpy(forbiddenMaps, forbiddenmaps.c_str(), forbiddenmaps.length());
    const char * delim = ", ";
    char * token = strtok(forbiddenMaps, delim);
    while (token != NULL)
    {
        int32 mapid = strtol(token, NULL, 10);
        m_forbiddenMapIds.insert(mapid);
        token = strtok(NULL,delim);
    }
    delete[] forbiddenMaps;

    loadConfig(CONFIG_GUILD_ANN_INTERVAL, "GuildAnnounce.Timer", 1*MINUTE*1000);
    loadConfig(CONFIG_GUILD_ANN_COOLDOWN, "GuildAnnounce.Cooldown", 60*MINUTE);
    loadConfig(CONFIG_GUILD_ANN_LENGTH, "GuildAnnounce.Length", 60);

    loadConfig(CONFIG_XP_RATE_MODIFY_ITEM_ENTRY, "XPRateModifyItem.Entry",0);
    loadConfig(CONFIG_XP_RATE_MODIFY_ITEM_PCT, "XPRateModifyItem.Pct",5);

    // Player interaction
    loadConfig(CONFIG_ALLOW_TWO_SIDE_ACCOUNTS, "AllowTwoSide.Accounts", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND, "AllowTwoSide.AddFriend", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT, "AllowTwoSide.Interaction.Chat", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL, "AllowTwoSide.Interaction.Channel", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP, "AllowTwoSide.Interaction.Group", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD, "AllowTwoSide.Interaction.Guild", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_ARENA, "AllowTwoSide.Interaction.Arena", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION, "AllowTwoSide.Interaction.Auction", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_MAIL, "AllowTwoSide.Interaction.Mail", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_TRADE, "AllowTwoSide.Trade", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_WHO_LIST, "AllowTwoSide.WhoList", false);
    loadConfig(CONFIG_TALENTS_INSPECTING, "TalentsInspecting", true);

    // Chat settings
    loadConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING, "ChatFakeMessagePreventing", false);
    loadConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY, "ChatStrictLinkChecking.Severity", 0);
    loadConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK, "ChatStrictLinkChecking.Kick", 0);
    loadConfig(CONFIG_GLOBAL_TRADE_CHANNEL, "Channel.GlobalTradeChannel",false);
    loadConfig(CONFIG_PRIVATE_CHANNEL_LIMIT, "Channel.PrivateLimitCount", 20);
    loadConfig(CONFIG_RESTRICTED_LFG_CHANNEL, "Channel.RestrictedLfg", true);
    loadConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL, "Channel.SilentlyGMJoin", false);
    loadConfig(CONFIG_CHAT_DENY_MASK, "Chat.DenyMask", 0);
    loadConfig(CONFIG_CHAT_MINIMUM_LVL, "Chat.MinimumLevel", 5);
    loadConfig(CONFIG_CHATFLOOD_MESSAGE_COUNT, "ChatFlood.MessageCount",10);
    loadConfig(CONFIG_CHATFLOOD_MESSAGE_DELAY, "ChatFlood.MessageDelay",1);
    loadConfig(CONFIG_CHATFLOOD_MUTE_TIME, "ChatFlood.MuteTime",10);

    // Game master settings
    loadConfig(CONFIG_GM_LOGIN_STATE, "GM.LoginState",2);
    loadConfig(CONFIG_GM_VISIBLE_STATE, "GM.Visible", 2);
    loadConfig(CONFIG_GM_CHAT, "GM.Chat",2);
    loadConfig(CONFIG_GM_WISPERING_TO, "GM.WhisperingTo",2);
    loadConfig(CONFIG_GM_IN_GM_LIST, "GM.InGMList",false);
    loadConfig(CONFIG_GM_IN_WHO_LIST, "GM.InWhoList",false);
    loadConfig(CONFIG_GM_LOG_TRADE, "GM.LogTrade", false);
    loadConfig(CONFIG_ALLOW_GM_GROUP, "GM.AllowInvite", false);
    loadConfig(CONFIG_ALLOW_GM_FRIEND, "GM.AllowFriend", false);
    loadConfig(CONFIG_GM_TRUSTED_LEVEL, "GM.TrustedLevel", PERM_HIGH_GMT);
    /*Only load 7 days of closed tickets*/
    loadConfig(CONFIG_TICKET_CLOSED_COUNT, "TicketClosedCount", 1000);
    loadConfig(CONFIG_ENABLE_CRASHTEST, "EnableCrashtest", false);

    loadConfig(CONFIG_COMMAND_LOG_PERMISSION, "CommandLogPermission", PERM_GMT_DEV);
    loadConfig(CONFIG_INSTANT_LOGOUT, "InstantLogout", PERM_GMT_DEV);
    loadConfig(CONFIG_MIN_GM_TEXT_LVL, "MinGMTextLevel", PERM_GMT_HDEV);
    loadConfig(CONFIG_DISABLE_BREATHING, "DisableWaterBreath", PERM_CONSOLE);
    loadConfig(CONFIG_HIDE_GAMEMASTER_ACCOUNTS, "HideGameMasterAccounts", true);

    // Server rates
    loadConfig(BONUS_RATES, "BonusRates", false);

    loadConfig(RATE_HEALTH, "Rate.Health", 1.0f);
    loadConfig(RATE_POWER_MANA, "Rate.Mana", 1.0f);
    loadConfig(RATE_POWER_RAGE_INCOME, "Rate.Rage.Income", 1.0f);
    loadConfig(RATE_POWER_RAGE_LOSS, "Rate.Rage.Loss", 1.0f);
    loadConfig(RATE_POWER_FOCUS, "Rate.Focus", 1.0f);
    loadConfig(RATE_LOYALTY, "Rate.Loyalty", 1.0f);
    loadConfig(RATE_SKILL_DISCOVERY, "Rate.Skill.Discovery", 1.0f);
    loadConfig(RATE_DROP_ITEM_POOR, "Rate.Drop.Item.Poor", 1.0f);
    loadConfig(RATE_DROP_ITEM_NORMAL, "Rate.Drop.Item.Normal", 1.0f);
    loadConfig(RATE_DROP_ITEM_UNCOMMON, "Rate.Drop.Item.Uncommon", 1.0f);
    loadConfig(RATE_DROP_ITEM_RARE, "Rate.Drop.Item.Rare", 1.0f);
    loadConfig(RATE_DROP_ITEM_EPIC, "Rate.Drop.Item.Epic", 1.0f);
    loadConfig(RATE_DROP_ITEM_LEGENDARY, "Rate.Drop.Item.Legendary", 1.0f);
    loadConfig(RATE_DROP_ITEM_ARTIFACT, "Rate.Drop.Item.Artifact", 1.0f);
    loadConfig(RATE_DROP_ITEM_REFERENCED, "Rate.Drop.Item.Referenced", 1.0f);
    loadConfig(RATE_DROP_MONEY, "Rate.Drop.Money", 1.0f);
    loadConfig(RATE_XP_KILL, "Rate.XP.Kill", 1.0f);
    loadConfig(RATE_XP_QUEST, "Rate.XP.Quest", 1.0f);
    loadConfig(RATE_XP_EXPLORE, "Rate.XP.Explore", 1.0f);
    loadConfig(RATE_XP_PAST_70, "Rate.XP.PastLevel70", 1.0f);

    loadConfig(RATE_REST_INGAME, "Rate.Rest.InGame", 1.0f);
    loadConfig(RATE_REST_OFFLINE_IN_TAVERN_OR_CITY, "Rate.Rest.Offline.InTavernOrCity", 1.0f);
    loadConfig(RATE_REST_OFFLINE_IN_WILDERNESS, "Rate.Rest.Offline.InWilderness", 1.0f);
    loadConfig(RATE_REST_LIMIT, "Rate.Rest.Limit", 1.0f);

    loadConfig(RATE_DAMAGE_FALL, "Rate.Damage.Fall", 1.0f);
    loadConfig(RATE_AUCTION_TIME, "Rate.Auction.Time", 1.0f);
    loadConfig(RATE_AUCTION_DEPOSIT, "Rate.Auction.Deposit", 1.0f);
    loadConfig(RATE_AUCTION_CUT, "Rate.Auction.Cut", 1.0f);
    loadConfig(RATE_HONOR_A, "Rate.Honor.Alliance",1.0f);
    loadConfig(RATE_HONOR_H, "Rate.Honor.Horde", 1.0f);
    loadConfig(RATE_MINING_NEXT, "Rate.Mining.Next",1.0f);
    loadConfig(RATE_TALENT, "Rate.Talent",1.0f);
    loadConfig(RATE_REPUTATION_GAIN, "Rate.Reputation.Gain", 1.0f);
    loadConfig(RATE_REPUTATION_LOWLEVEL_KILL, "Rate.Reputation.LowLevel.Kill", 0.2f);
    loadConfig(RATE_REPUTATION_LOWLEVEL_QUEST, "Rate.Reputation.LowLevel.Quest", 1.0f);
    loadConfig(RATE_INSTANCE_RESET_TIME, "Rate.InstanceResetTime", 1.0f);
    loadConfig(RATE_DURABILITY_LOSS_DAMAGE, "DurabilityLossChance.Damage", 0.5f);
    loadConfig(RATE_DURABILITY_LOSS_ABSORB, "DurabilityLossChance.Absorb", 0.5f);
    loadConfig(RATE_DURABILITY_LOSS_BLOCK, "DurabilityLossChance.Block", 0.05f);
    loadConfig(RATE_DURABILITY_LOSS_PARRY, "DurabilityLossChance.Parry", 0.05f);
    loadConfig(CONFIG_SKILL_GAIN_CRAFTING, "SkillGain.Crafting", 1);
    loadConfig(CONFIG_SKILL_GAIN_DEFENSE, "SkillGain.Defense", 1);
    loadConfig(CONFIG_SKILL_GAIN_GATHERING, "SkillGain.Gathering", 1);
    loadConfig(CONFIG_SKILL_GAIN_WEAPON, "SkillGain.Weapon", 1);
    loadConfig(CONFIG_SKILL_CHANCE_ORANGE, "SkillChance.Orange",100);
    loadConfig(CONFIG_SKILL_CHANCE_YELLOW, "SkillChance.Yellow",75);
    loadConfig(CONFIG_SKILL_CHANCE_GREEN, "SkillChance.Green",25);
    loadConfig(CONFIG_SKILL_CHANCE_GREY, "SkillChance.Grey",0);
    loadConfig(CONFIG_SKILL_CHANCE_MINING_STEPS, "SkillChance.MiningSteps",75);
    loadConfig(CONFIG_SKILL_CHANCE_SKINNING_STEPS, "SkillChance.SkinningSteps",75);

    loadConfig(CONFIG_DEATH_SICKNESS_LEVEL, "Death.SicknessLevel", 11);
    loadConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP, "Death.CorpseReclaimDelay.PvP", true);
    loadConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE, "Death.CorpseReclaimDelay.PvE", true);
    loadConfig(CONFIG_DEATH_BONES_WORLD, "Death.Bones.World", true);
    loadConfig(CONFIG_DEATH_BONES_BG_OR_ARENA, "Death.Bones.BattlegroundOrArena", true);

    // Creature settings
    loadConfig(RATE_CREATURE_AGGRO, "Rate.Creature.Aggro", 1.0f);
    loadConfig(RATE_CREATURE_GUARD_AGGRO, "Rate.Creature.Guard.Aggro", 1.5f);
    loadConfig(RATE_CORPSE_DECAY_LOOTED, "Rate.Corpse.Decay.Looted",0.5f);
    loadConfig(RATE_CREATURE_NORMAL_DAMAGE, "Rate.Creature.Normal.Damage", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_ELITE_DAMAGE, "Rate.Creature.Elite.Elite.Damage", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_RAREELITE_DAMAGE, "Rate.Creature.Elite.RAREELITE.Damage", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE, "Rate.Creature.Elite.WORLDBOSS.Damage", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_RARE_DAMAGE, "Rate.Creature.Elite.RARE.Damage", 1.0f);
    loadConfig(RATE_CREATURE_NORMAL_HP, "Rate.Creature.Normal.HP", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_ELITE_HP, "Rate.Creature.Elite.Elite.HP", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_RAREELITE_HP, "Rate.Creature.Elite.RAREELITE.HP", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_WORLDBOSS_HP, "Rate.Creature.Elite.WORLDBOSS.HP", 1.0f);
    loadConfig(RATE_CREATURE_ELITE_RARE_HP, "Rate.Creature.Elite.RARE.HP", 1.0f);

    loadConfig(CONFIG_CORPSE_DECAY_NORMAL, "Corpse.Decay.NORMAL", 60);
    loadConfig(CONFIG_CORPSE_DECAY_RARE, "Corpse.Decay.RARE", 300);
    loadConfig(CONFIG_CORPSE_DECAY_ELITE, "Corpse.Decay.ELITE", 300);
    loadConfig(CONFIG_CORPSE_DECAY_RAREELITE, "Corpse.Decay.RAREELITE", 300);
    loadConfig(CONFIG_CORPSE_DECAY_WORLDBOSS, "Corpse.Decay.WORLDBOSS", 3600);
    loadConfig(CONFIG_LISTEN_RANGE_SAY, "ListenRange.Say", 25);
    loadConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE, "ListenRange.TextEmote", 25);
    loadConfig(CONFIG_LISTEN_RANGE_YELL, "ListenRange.Yell", 300);

    loadConfig(CONFIG_WAYPOINT_MOVEMENT_ACTIVE_ON_CONTINENTS, "AutoActive.WaypointMovement.Continents", true);
    loadConfig(CONFIG_WAYPOINT_MOVEMENT_ACTIVE_IN_INSTANCES, "AutoActive.WaypointMovement.Instances", true);
    loadConfig(CONFIG_COMBAT_ACTIVE_ON_CONTINENTS, "AutoActive.Combat.Continents", true);
    loadConfig(CONFIG_COMBAT_ACTIVE_IN_INSTANCES, "AutoActive.Combat.Instances", true);
    loadConfig(CONFIG_COMBAT_ACTIVE_FOR_PLAYERS_ONLY, "AutoActive.Combat.PlayersOnly", false);

    loadConfig(CONFIG_SIGHT_GUARD, "GuarderSight", 50);
    loadConfig(CONFIG_SIGHT_MONSTER, "MonsterSight", 50);
    loadConfig(CONFIG_EVADE_HOMEDIST, "Creature.Evade.DistanceToHome", 60);
    loadConfig(CONFIG_EVADE_TARGETDIST, "Creature.Evade.DistanceToTarget", 50);

    loadConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS, "CreatureFamilyAssistanceRadius", 10);
    loadConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY, "CreatureFamilyAssistanceDelay", 1500);
    loadConfig(CONFIG_CREATURE_FAMILY_FLEE_RADIUS, "CreatureFamilyFleeAssistanceRadius", 30);
    loadConfig(CONFIG_CREATURE_FAMILY_FLEE_DELAY, "CreatureFamilyFleeDelay", 7000);
    loadConfig(CONFIG_WORLD_BOSS_LEVEL_DIFF, "WorldBossLevelDiff",3);

    // Arena settings
    loadConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE, "Arena.MaxRatingDifference", 150); // initial difference is 150
    loadConfig(CONFIG_ARENA_RATING_DISCARD_TIMER, "Arena.RatingDiscardTimer",10000); // check every 10 sec - should be same as Arena.StepByStep.Time
    loadConfig(CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS, "Arena.AutoDistributePoints", true); // autodistribute it
    loadConfig(CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS, "Arena.AutoDistributeInterval", 7); // depends on realm
    loadConfig(CONFIG_ENABLE_FAKE_WHO_ON_ARENA, "Arena.EnableFakeWho", true); // enable
    loadConfig(CONFIG_ENABLE_FAKE_WHO_IN_GUILD, "Arena.EnableFakeWho.ForGuild", true); // enable
    loadConfig(CONFIG_ARENA_LOG_EXTENDED_INFO, "Arena.LogExtendedInfo", false);
    loadConfig(CONFIG_ARENA_EXPORT_RESULTS, "Arena.ExportResults", false);

    // MMR is deleted in our core
    loadConfig(CONFIG_ENABLE_HIDDEN_RATING, "Arena.EnableMMR", false);
    loadConfig(CONFIG_ENABLE_HIDDEN_RATING_PENALTY, "Arena.EnableMMRPenalty", false);
    loadConfig(CONFIG_HIDDEN_RATING_PENALTY, "Arena.MMRPenalty", 150);
    loadConfig(CONFIG_ENABLE_HIDDEN_RATING_LOWER_LOSS, "Arena.MMRSpecialLossCalc", false);

    loadConfig(CONFIG_ENABLE_ARENA_STEP_BY_STEP_MATCHING, "Arena.StepByStep.Enable",true); // enabled - all realms
    loadConfig(CONFIG_ARENA_STEP_BY_STEP_TIME, "Arena.StepByStep.Time",10000); // 10 seconds all realms. Should be same as Arena.RatingDiscardTimer
    loadConfig(CONFIG_ARENA_STEP_BY_STEP_VALUE, "Arena.StepByStep.Value",75); // 75 rating all realms
    loadConfig(CONFIG_ARENA_END_AFTER_TIME, "Arena.EndAfter.Time",2400000); // 40 min at max
    loadConfig(CONFIG_ARENA_STATUS_INFO, "Arena.StatusInfo");
    loadConfig(CONFIG_ARENA_ELO_COEFFICIENT, "Arena.ELOCoefficient",32);
    loadConfig(CONFIG_ARENA_DAILY_REQUIREMENT, "Arena.DailyRequirement",0);
    loadConfig(CONFIG_ARENA_DAILY_AP_REWARD, "Arena.DailyAPReward",0);
    loadConfig(CONFIG_ARENA_READY_START_TIMER, "Arena.ReadyStartTimer", 15000);
    loadConfig(CONFIG_ARENA_KEEP_TEAMS, "Arena.KeepTeams", 0);

    // Battleground settings
    loadConfig(CONFIG_BATTLEGROUND_ANNOUNCE_START, "BattleGround.AnnounceStart", 0);
    loadConfig(CONFIG_BATTLEGROUND_DESERTER_ON_INACTIVE, "BattleGround.DeserterOnInactive", true);
    loadConfig(CONFIG_BATTLEGROUND_DESERTER_REALTIME,"BattleGround.DeserterRealtime",true);
    loadConfig(CONFIG_BATTLEGROUND_INVITATION_TYPE, "Battleground.InvitationType", 1);
    loadConfig(CONFIG_BATTLEGROUND_KICK_AFTER_INACTIVE_TIME, "BattleGround.KickAfterInactiveTime", 60000);
    loadConfig(CONFIG_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH, "BattleGround.PremadeGroupWaitForMatch", 10 * MINUTE * MILLISECONDS);
    loadConfig(CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER, "BattleGround.PrematureFinishTimer", 0);
    loadConfig(CONFIG_BATTLEGROUND_TIMER_INFO, "BattleGround.TimerInfo", false); // by default players don't know when BG will prematurely end
    loadConfig(CONFIG_PREMATURE_BG_REWARD, "Battleground.PrematureReward", true);
    loadConfig(CONFIG_BATTLEGROUND_QUEUE_INFO, "BattleGround.QueueInfo", false); // by default players don't know how many players are queued for BGs
    loadConfig(CONFIG_BG_START_MUSIC, "Battleground.StartMusic", false);
    loadConfig(CONFIG_BATTLEGROUND_WSG_END_AFTER_ENABLED, "BattleGround.WSGEndAfter.Enabled",true);
    loadConfig(CONFIG_BATTLEGROUND_WSG_END_AFTER_TIME, "BattleGround.WSGEndAfter.Time",2400000); // 40 min at max
    loadConfig(CONFIG_BATTLEGROUND_AV_END_AFTER_ENABLED, "BattleGround.AVEndAfter.Enabled", true);
    loadConfig(CONFIG_BATTLEGROUND_AV_END_AFTER_TIME, "BattleGround.AVEndAfter.Time", 3000000); // 60 min at max

    // Vmaps/mmaps
    loadConfig(CONFIG_VMAP_LOS_ENABLED, "vmap.enableLOS", true);
    sLog.outString("WORLD: vmap los %sabled", getConfig(CONFIG_VMAP_LOS_ENABLED) ? "en" : "dis");

    loadConfig(CONFIG_VMAP_INDOOR_CHECK, "vmap.enableIndoorCheck", true);
    loadConfig(CONFIG_PET_LOS, "vmap.petLOS", false);
    loadConfig(CONFIG_VMAP_GROUND, "vmap.ground.enable", 0);
    loadConfig(CONFIG_VMAP_GROUND_TOLERANCE, "vmap.ground.tolerance", 5.0f);

    loadConfig(CONFIG_MMAP_ENABLED, "mmap.enabled", true);
    sLog.outString("WORLD: mmap pathfinding %sabled", getConfig(CONFIG_MMAP_ENABLED) ? "en" : "dis");

    // visibility and radiuses
    loadConfig(CONFIG_GROUP_VISIBILITY, "Visibility.GroupMode", 0);
    m_activeObjectUpdateDistanceOnContinents = sConfig.GetIntDefault("Visibility.Distance.ActiveObjectUpdate.Continents", DEFAULT_VISIBILITY_DISTANCE);
    m_activeObjectUpdateDistanceInInstances = sConfig.GetIntDefault("Visibility.Distance.ActiveObjectUpdate.Instances", DEFAULT_VISIBILITY_DISTANCE);

    // movement
    loadConfig(CONFIG_TARGET_POS_RECALCULATION_RANGE, "Movement.RecalculateRange", 2);
    if (m_configs[CONFIG_TARGET_POS_RECALCULATION_RANGE] < 0)
        m_configs[CONFIG_TARGET_POS_RECALCULATION_RANGE] = 0;
    if (m_configs[CONFIG_TARGET_POS_RECALCULATION_RANGE] > 5)
        m_configs[CONFIG_TARGET_POS_RECALCULATION_RANGE] = 5;
    loadConfig(CONFIG_TARGET_POS_RECHECK_TIMER, "Movement.RecheckTimer", 100);
    loadConfig(CONFIG_WAYPOINT_MOVEMENT_PATHFINDING_ON_CONTINENTS, "Movement.WaypointPathfinding.Continents", true);
    loadConfig(CONFIG_WAYPOINT_MOVEMENT_PATHFINDING_IN_INSTANCES, "Movement.WaypointPathfinding.Instances", true);
    loadConfig(CONFIG_MOVEMENT_ENABLE_LONG_CHARGE, "Movement.LongCharge", false);

    // CoreBalancer
    loadConfig(CONFIG_COREBALANCER_ENABLED, "CoreBalancer.Enable", false);
    loadConfig(CONFIG_COREBALANCER_PLAYABLE_DIFF, "CoreBalancer.PlayableDiff", 200);
    loadConfig(CONFIG_COREBALANCER_INTERVAL, "CoreBalancer.BalanceInterval", 300000);
    loadConfig(CONFIG_COREBALANCER_VISIBILITY_PENALTY, "CoreBalancer.VisibilityPenalty", 25);

    // VMSS system
    loadConfig(CONFIG_VMSS_ENABLE, "VMSS.Enable", false);
    loadConfig(CONFIG_VMSS_MAPFREEMETHOD, "VMSS.MapFreeMethod", 0);
    loadConfig(CONFIG_VMSS_FREEZECHECKPERIOD, "VMSS.FreezeCheckPeriod", 1000);
    loadConfig(CONFIG_VMSS_FREEZEDETECTTIME, "VMSS.MapFreezeDetectTime", 1000);

    // Warden/anticheat
    loadConfig(CONFIG_WARDEN_ENABLED, "Warden.Enabled", true);
    loadConfig(CONFIG_WARDEN_KICK, "Warden.Kick", true);
    loadConfig(CONFIG_WARDEN_BAN, "Warden.Ban", true);
    loadConfig(CONFIG_WARDEN_LOG_ONLY_CHECK, "Warden.LogOnlyCheck",0);
    loadConfig(CONFIG_WARDEN_CHECK_INTERVAL_MIN, "Warden.CheckIntervalMin",25000);
    loadConfig(CONFIG_WARDEN_CHECK_INTERVAL_MAX, "Warden.CheckIntervalMax",35000);
    loadConfig(CONFIG_WARDEN_MEM_CHECK_MAX, "Warden.MemCheckMax",3);
    loadConfig(CONFIG_WARDEN_RANDOM_CHECK_MAX, "Warden.RandomCheckMax",5);
    loadConfig(CONFIG_ENABLE_PASSIVE_ANTICHEAT, "AntiCheat.Enable", 1); 
    loadConfig(CONFIG_ANTICHEAT_CUMULATIVE_DELAY, "AntiCheat.CumulativeDelay", 5);
    loadConfig(CONFIG_ANTICHEAT_SPEEDHACK_TOLERANCE, "AntiCheat.SpeedhackTolerance",1.00f);
    loadConfig(CONFIG_GOBJECT_USE_EXPLOIT_RANGE, "AntiCheat.GobjectUseExploitRange", 3 * INTERACTION_DISTANCE);

    // RaF
    loadConfig(CONFIG_UINT32_RAF_MAXGRANTLEVEL, "RAF.MaxGrantLevel", 60); // Level at which target can no longer be given grantable levels OR summonned
    //loadConfig(CONFIG_UINT32_RAF_MAXREFERALS, "RAF.MaxReferals", 5);
    //loadConfig(CONFIG_UINT32_RAF_MAXREFERERS, "RAF.MaxReferers", 5);
    loadConfig(CONFIG_FLOAT_RATE_RAF_XP, "Rate.RAF.XP", 3.0f);
    loadConfig(CONFIG_FLOAT_RATE_RAF_LEVELPERLEVEL, "Rate.RAF.LevelPerLevel", 0.5f);
    
    // Ganking penalty
    loadConfig(CONFIG_ENABLE_GANKING_PENALTY, "PVP.EnableGankingPenalty", false);
    loadConfig(CONFIG_GANKING_PENALTY_EXPIRE, "PVP.GankingPenaltyExpireTime", 600000);
    loadConfig(CONFIG_GANKING_KILLS_ALERT, "PVP.GankingPenaltyKillsAlert", 10);
    loadConfig(CONFIG_GANKING_PENALTY_PER_KILL , "PVP.GankingPenaltyPerKill", 0.1);

    // Network
    loadConfig(CONFIG_KICK_PLAYER_ON_BAD_PACKET, "Network.KickOnBadPacket", true);

        // === Reload only section === 
    if (reload)
    {
        if (sConfig.GetIntDefault("WorldServerPort", DEFAULT_WORLDSERVER_PORT) != m_configs[CONFIG_PORT_WORLD])
            sLog.outLog(LOG_DEFAULT, "ERROR: WorldServerPort option can't be changed at .conf file reload, using current value (%u).",m_configs[CONFIG_PORT_WORLD]);

        sMapMgr.SetGridCleanUpDelay(m_configs[CONFIG_INTERVAL_GRIDCLEAN]);
        m_timers[WUPDATE_UPTIME].SetInterval(m_configs[CONFIG_UPTIME_UPDATE]*MINUTE*1000);
        m_timers[WUPDATE_UPTIME].Reset();

        // Server settings
        if (sConfig.GetIntDefault("GameType", 0) != m_configs[CONFIG_GAME_TYPE])
            sLog.outLog(LOG_DEFAULT, "ERROR: GameType option can't be changed at .conf file reload, using current value (%u).",m_configs[CONFIG_GAME_TYPE]);
        if (sConfig.GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT) != m_configs[CONFIG_REALM_ZONE])
            sLog.outLog(LOG_DEFAULT, "ERROR: RealmZone option can't be changed at .conf file reload, using current value (%u).",m_configs[CONFIG_REALM_ZONE]);
        if ( sConfig.GetIntDefault("Expansion",1) != m_configs[CONFIG_EXPANSION])
            sLog.outLog(LOG_DEFAULT, "ERROR: Expansion option can't be changed at .conf file reload, using current value (%u).",m_configs[CONFIG_EXPANSION]);
        
        // Server customization basic
        if (sConfig.GetIntDefault("MaxPlayerLevel", 70) != m_configs[CONFIG_MAX_PLAYER_LEVEL])
            sLog.outLog(LOG_DEFAULT, "ERROR: MaxPlayerLevel option can't be changed at config reload, using current value (%u).",m_configs[CONFIG_MAX_PLAYER_LEVEL]);

    }
    // === Not-reload only section ===
    else
    {
        loadConfig(CONFIG_PORT_WORLD, "WorldServerPort", DEFAULT_WORLDSERVER_PORT);

        // Server settings
        loadConfig(CONFIG_GAME_TYPE, "GameType", 0);
        loadConfig(CONFIG_REALM_ZONE, "RealmZone", REALM_ZONE_DEVELOPMENT);
        loadConfig(CONFIG_EXPANSION, "Expansion",1);

        // Server customization basic
        loadConfig(CONFIG_MAX_PLAYER_LEVEL, "MaxPlayerLevel", 70);
        if (m_configs[CONFIG_MAX_PLAYER_LEVEL] > MAX_LEVEL)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: MaxPlayerLevel (%i) must be in range 1..%u. Set to %u.",m_configs[CONFIG_MAX_PLAYER_LEVEL],MAX_LEVEL,MAX_LEVEL);
            m_configs[CONFIG_MAX_PLAYER_LEVEL] = MAX_LEVEL;
        }
    }

    // === Warns section ===
    // Performance settings
    loadConfig(CONFIG_COMPRESSION, "Compression", 1);
    if (m_configs[CONFIG_COMPRESSION] < 1 || m_configs[CONFIG_COMPRESSION] > 9)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Compression level (%i) must be in range 1..9. Using default compression level (1).",m_configs[CONFIG_COMPRESSION]);
        m_configs[CONFIG_COMPRESSION] = 1;
    }
        
    loadConfig(CONFIG_MAX_OVERSPEED_PINGS, "MaxOverspeedPings",2);
    if (m_configs[CONFIG_MAX_OVERSPEED_PINGS] != 0 && m_configs[CONFIG_MAX_OVERSPEED_PINGS] < 2)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MaxOverspeedPings (%i) must be in range 2..infinity (or 0 to disable check. Set to 2.",m_configs[CONFIG_MAX_OVERSPEED_PINGS]);
        m_configs[CONFIG_MAX_OVERSPEED_PINGS] = 2;
    }

    loadConfig(CONFIG_INTERVAL_GRIDCLEAN, "GridCleanUpDelay", 300000);
    if (m_configs[CONFIG_INTERVAL_GRIDCLEAN] < MIN_GRID_DELAY)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: GridCleanUpDelay (%i) must be greater %u. Use this minimal value.",m_configs[CONFIG_INTERVAL_GRIDCLEAN],MIN_GRID_DELAY);
        m_configs[CONFIG_INTERVAL_GRIDCLEAN] = MIN_GRID_DELAY;
    }

    loadConfig(CONFIG_INTERVAL_MAPUPDATE, "MapUpdateInterval", 100);
    if (m_configs[CONFIG_INTERVAL_MAPUPDATE] < MIN_MAP_UPDATE_DELAY)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MapUpdateInterval (%i) must be greater %u. Use this minimal value.",m_configs[CONFIG_INTERVAL_MAPUPDATE],MIN_MAP_UPDATE_DELAY);
        m_configs[CONFIG_INTERVAL_MAPUPDATE] = MIN_MAP_UPDATE_DELAY;
    }

    loadConfig(CONFIG_UPTIME_UPDATE, "UpdateUptimeInterval", 10);
    if (m_configs[CONFIG_UPTIME_UPDATE]<=0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: UpdateUptimeInterval (%i) must be > 0, set to default 10.",m_configs[CONFIG_UPTIME_UPDATE]);
        m_configs[CONFIG_UPTIME_UPDATE] = 10;
    }
    // Server settings    
    loadConfig(CONFIG_CHARACTERS_PER_REALM, "CharactersPerRealm", 10);
    if (m_configs[CONFIG_CHARACTERS_PER_REALM] < 1 || m_configs[CONFIG_CHARACTERS_PER_REALM] > 10)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CharactersPerRealm (%i) must be in range 1..10. Set to 10.",m_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_configs[CONFIG_CHARACTERS_PER_REALM] = 10;
    }

    loadConfig(CONFIG_CHARACTERS_PER_ACCOUNT, "CharactersPerAccount", 50);
    if (m_configs[CONFIG_CHARACTERS_PER_ACCOUNT] < m_configs[CONFIG_CHARACTERS_PER_REALM])
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CharactersPerAccount (%i) can't be less than CharactersPerRealm (%i).",m_configs[CONFIG_CHARACTERS_PER_ACCOUNT],m_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_configs[CONFIG_CHARACTERS_PER_ACCOUNT] = m_configs[CONFIG_CHARACTERS_PER_REALM];
    }
    // Server customization basic
    loadConfig(CONFIG_START_PLAYER_LEVEL, "StartPlayerLevel", 1);
    if (m_configs[CONFIG_START_PLAYER_LEVEL] < 1)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to 1.",m_configs[CONFIG_START_PLAYER_LEVEL],m_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_configs[CONFIG_START_PLAYER_LEVEL] = 1;
    }
    else if (m_configs[CONFIG_START_PLAYER_LEVEL] > m_configs[CONFIG_MAX_PLAYER_LEVEL])
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to %u.",m_configs[CONFIG_START_PLAYER_LEVEL],m_configs[CONFIG_MAX_PLAYER_LEVEL],m_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_configs[CONFIG_START_PLAYER_LEVEL] = m_configs[CONFIG_MAX_PLAYER_LEVEL];
    }

    loadConfig(CONFIG_START_PLAYER_MONEY,"StartPlayerMoney", 0);
    if (m_configs[CONFIG_START_PLAYER_MONEY] < 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartPlayerMoney (%i) must be in range 0..%u. Set to %u.",m_configs[CONFIG_START_PLAYER_MONEY],MAX_MONEY_AMOUNT,0);
        m_configs[CONFIG_START_PLAYER_MONEY] = 0;
    }
    else if (m_configs[CONFIG_START_PLAYER_MONEY] > MAX_MONEY_AMOUNT)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartPlayerMoney (%i) must be in range 0..%u. Set to %u.",
            m_configs[CONFIG_START_PLAYER_MONEY],MAX_MONEY_AMOUNT,MAX_MONEY_AMOUNT);
        m_configs[CONFIG_START_PLAYER_MONEY] = MAX_MONEY_AMOUNT;
    }

    loadConfig(CONFIG_MAX_HONOR_POINTS, "MaxHonorPoints", 75000);
    if (m_configs[CONFIG_MAX_HONOR_POINTS] < 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MaxHonorPoints (%i) can't be negative. Set to 0.",m_configs[CONFIG_MAX_HONOR_POINTS]);
        m_configs[CONFIG_MAX_HONOR_POINTS] = 0;
    }

    loadConfig(CONFIG_START_HONOR_POINTS, "StartHonorPoints", 0);
    if (m_configs[CONFIG_START_HONOR_POINTS] < 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartHonorPoints (%i) must be in range 0..MaxHonorPoints(%u). Set to %u.",
            m_configs[CONFIG_START_HONOR_POINTS],m_configs[CONFIG_MAX_HONOR_POINTS],0);
        m_configs[CONFIG_START_HONOR_POINTS] = 0;
    }
    else if (m_configs[CONFIG_START_HONOR_POINTS] > m_configs[CONFIG_MAX_HONOR_POINTS])
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartHonorPoints (%i) must be in range 0..MaxHonorPoints(%u). Set to %u.",
            m_configs[CONFIG_START_HONOR_POINTS],m_configs[CONFIG_MAX_HONOR_POINTS],m_configs[CONFIG_MAX_HONOR_POINTS]);
        m_configs[CONFIG_START_HONOR_POINTS] = m_configs[CONFIG_MAX_HONOR_POINTS];
    }

    loadConfig(CONFIG_MAX_ARENA_POINTS, "MaxArenaPoints", 5000);
    if (m_configs[CONFIG_MAX_ARENA_POINTS] < 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MaxArenaPoints (%i) can't be negative. Set to 0.",m_configs[CONFIG_MAX_ARENA_POINTS]);
        m_configs[CONFIG_MAX_ARENA_POINTS] = 0;
    }

    loadConfig(CONFIG_START_ARENA_POINTS, "StartArenaPoints", 0);
    if (m_configs[CONFIG_START_ARENA_POINTS] < 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartArenaPoints (%i) must be in range 0..MaxArenaPoints(%u). Set to %u.",
            m_configs[CONFIG_START_ARENA_POINTS],m_configs[CONFIG_MAX_ARENA_POINTS],0);
        m_configs[CONFIG_START_ARENA_POINTS] = 0;
    }
    else if (m_configs[CONFIG_START_ARENA_POINTS] > m_configs[CONFIG_MAX_ARENA_POINTS])
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: StartArenaPoints (%i) must be in range 0..MaxArenaPoints(%u). Set to %u.",
            m_configs[CONFIG_START_ARENA_POINTS],m_configs[CONFIG_MAX_ARENA_POINTS],m_configs[CONFIG_MAX_ARENA_POINTS]);
        m_configs[CONFIG_START_ARENA_POINTS] = m_configs[CONFIG_MAX_ARENA_POINTS];
    }

    // Server customization advanced
    loadConfig(CONFIG_MIN_PETITION_SIGNS, "MinPetitionSigns", 9);
    if (m_configs[CONFIG_MIN_PETITION_SIGNS] > 9)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MinPetitionSigns (%i) must be in range 0..9. Set to 9.",m_configs[CONFIG_MIN_PETITION_SIGNS]);
        m_configs[CONFIG_MIN_PETITION_SIGNS] = 9;
    }

    loadConfig(CONFIG_SKIP_CINEMATICS, "SkipCinematics", 0);
    if (m_configs[CONFIG_SKIP_CINEMATICS] < 0 || m_configs[CONFIG_SKIP_CINEMATICS] > 2)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: SkipCinematics (%i) must be in range 0..2. Set to 0.",m_configs[CONFIG_SKIP_CINEMATICS]);
        m_configs[CONFIG_SKIP_CINEMATICS] = 0;
    }

    // Game master settings
    loadConfig(CONFIG_START_GM_LEVEL, "GM.StartLevel", 1);
    if (m_configs[CONFIG_START_GM_LEVEL] < m_configs[CONFIG_START_PLAYER_LEVEL])
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: GM.StartLevel (%i) must be in range StartPlayerLevel(%u)..%u. Set to %u.",
            m_configs[CONFIG_START_GM_LEVEL],m_configs[CONFIG_START_PLAYER_LEVEL], MAX_LEVEL, m_configs[CONFIG_START_PLAYER_LEVEL]);
        m_configs[CONFIG_START_GM_LEVEL] = m_configs[CONFIG_START_PLAYER_LEVEL];
    }
    else if (m_configs[CONFIG_START_GM_LEVEL] > MAX_LEVEL)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: GM.StartLevel (%i) must be in range 1..%u. Set to %u.", m_configs[CONFIG_START_GM_LEVEL], MAX_LEVEL, MAX_LEVEL);
        m_configs[CONFIG_START_GM_LEVEL] = MAX_LEVEL;
    }

    // visibility and radiuses
    m_VisibleObjectGreyDistance = sConfig.GetIntDefault("Visibility.Distance.Grey.Object", 10);
    if (m_VisibleObjectGreyDistance >  MAX_VISIBILITY_DISTANCE)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Visibility.Distance.Grey.Object can't be greater %f",MAX_VISIBILITY_DISTANCE);
        m_VisibleObjectGreyDistance = MAX_VISIBILITY_DISTANCE;
    }

    loadConfig(CONFIG_SHOW_ACTIVEEVENTS_ON_LOGIN, "Player.ShowActiveEventsOnLogin", false);
    //DeathSide
    loadConfig(CONFIG_ARENA_POINTS_RATE, "Arena.Points.Rate", 1); // arena points rate
    loadConfig(CONFIG_REALM_TYPE, "RealmType", 0); // 0 - x1. 1 - ffa. 2 - pvp. 3 - fun. 4 - x30
    loadConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION, "AllowTwoSide.Battlegrounds", false);
    loadConfig(RATE_BG_WIN, "Rate.BG.Win", 3); 
    loadConfig(RATE_BG_LOSS, "Rate.BG.Loss", 1);
    loadConfig(CONFIG_ARENA_SPECTATORS_ENABLE, "Arena.Spectators.Enable", false);
    loadConfig(TIME_ARENA_CLOSEDFOR1v1, "Time.Arena1v1.ClosedFor", 0);
    loadConfig(TIME_ARENA_CLOSING1v1, "Time.Arena1v1.Closing", 1);
    loadConfig(TIME_ARENA_CLOSEDFOR2v2, "Time.Arena2v2.ClosedFor", 0);
    loadConfig(TIME_ARENA_CLOSING2v2, "Time.Arena2v2.Closing", 1);
    loadConfig(TIME_ARENA_CLOSEDFOR3v3, "Time.Arena3v3.ClosedFor", 0);
    loadConfig(TIME_ARENA_CLOSING3v3, "Time.Arena3v3.Closing", 1);
    loadConfig(TIME_ARENA_CLOSEDFOR5v5, "Time.Arena5v5.ClosedFor", 0);
    loadConfig(TIME_ARENA_CLOSING5v5, "Time.Arena5v5.Closing", 1);
    loadConfig(TIME_BG_CLOSEDFOR, "Time.BG.ClosedFor", 0);
    loadConfig(TIME_BG_CLOSING, "Time.BG.Closing", 1);
    loadConfig(CONFIG_TRANSMOG_REQ_REAGENT, "Transmogrification.RequireReagent", true);
    loadConfig(SKIRMISH_ARENA_WIN_HONOR, "SkirmishArenaWinHonor", 0);
    loadConfig(CONFIG_LOGIN_IPSET, "AccountLoginIpSet", 0);
    loadConfig(CONFIG_DURABILITY_LOSS_ON_DEATH, "DurabilityLossOnDeath", true);
    loadConfig(CONFIG_ARENA_PVE_RESTRICTED, "Arena.PVE_Restricted", false);
    loadConfig(CONFIG_BG_EVENTS_ENABLED, "BG.Events", true); // change BG events every 60 min

    loadConfig(CONFIG_FACTION_MINORITY, "FactionMinority", 0);

    loadConfig(CONFIG_DYN_RESPAWN_CHECK_RANGE, "DynamicRespawn.CheckRange", 0); // 0 by default -> disabled. (integer, 0-500), Cell::Visit limits max to 500 yds
    if (m_configs[CONFIG_DYN_RESPAWN_CHECK_RANGE] >  MAX_VISIBILITY_DISTANCE)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: DynamicRespawn.CheckRange can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_configs[CONFIG_DYN_RESPAWN_CHECK_RANGE] = MAX_VISIBILITY_DISTANCE;
    }
    loadConfig(CONFIG_DYN_RESPAWN_PLAYERS_THRESHOLD, "DynamicRespawn.PlayersThreshold", 0); // how many players need to be found in order to apply reduction
    loadConfig(CONFIG_DYN_RESPAWN_PLAYERS_LEVELDIFF, "DynamicRespawn.PlayersMaxLevelDiff", 0); // max level diff between player/creature to be counted by search
    loadConfig(CONFIG_DYN_RESPAWN_MIN_RESULTING_RESPAWN, "DynamicRespawn.MinResultingRespawn", 0); // minimum time for respawn after the calculations (seconds)
    loadConfig(CONFIG_DYN_RESPAWN_MAX_AFFECTED_RESPAWN, "DynamicRespawn.MaxAffectedRespawn", 0); // maximum time for respawn to be affected (seconds)
    loadConfig(CONFIG_DYN_RESPAWN_MIN_REDUCED_PCT, "DynamicRespawn.MinReducedPct", 0); // min percent of respawntime after respawn reduction apply (integer, 0-100)
    if (m_configs[CONFIG_DYN_RESPAWN_MIN_REDUCED_PCT] > 100)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: DynamicRespawn.MinReducedPct must be within range 0 to 100");
        m_configs[CONFIG_DYN_RESPAWN_MIN_REDUCED_PCT] = 100;
    }
    loadConfig(CONFIG_DYN_RESPAWN_MAX_AFFECTED_LEVEL, "DynamicRespawn.MaxAffectedLevel", 0); // max creature level to be affected by dyn respawn
    loadConfig(CONFIG_DYN_RESPAWN_PERCENT_PER_10_PLAYERS, "DynamicRespawn.PercentPer10Players", 0); // percent of respawn reduction per 10 found players over threshold (integer, 0-1000)
    if (m_configs[CONFIG_DYN_RESPAWN_PERCENT_PER_10_PLAYERS] > 1000)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: DynamicRespawn.PercentPer10Players must be within range 0 to 1000. 1000 means 100% reduction for 1 player over threshold");
        m_configs[CONFIG_DYN_RESPAWN_PERCENT_PER_10_PLAYERS] = 1000;
    }

    loadConfig(CONFIG_CHARGES_COUNT_GM_LOOT_DEL, "Gm.LootDeleteCharges", 10);
    loadConfig(CONFIG_CHARGES_COOLDOWN_GM_LOOT_DEL, "Gm.LootDeleteCooldown", 86400);
    m_configs[CONFIG_CHARGES_COOLDOWN_GM_LOOT_DEL] *= MILLISECONDS;
    if (m_configs[CONFIG_CHARGES_COOLDOWN_GM_LOOT_DEL] && !m_configs[CONFIG_CHARGES_COUNT_GM_LOOT_DEL])
        m_configs[CONFIG_CHARGES_COUNT_GM_LOOT_DEL] = 1; // has to have atleast one charge if enabled

    loadConfig(CONFIG_CHARGES_COUNT_ASSIST_KICK, "Group.AssistKickCharges", 3);
    loadConfig(CONFIG_CHARGES_COOLDOWN_ASSIST_KICK, "Group.AssistKickCooldown", 60); // 3 charges per minute
    m_configs[CONFIG_CHARGES_COOLDOWN_ASSIST_KICK] *= MILLISECONDS;
    if (m_configs[CONFIG_CHARGES_COOLDOWN_ASSIST_KICK] && !m_configs[CONFIG_CHARGES_COUNT_ASSIST_KICK])
        m_configs[CONFIG_CHARGES_COUNT_ASSIST_KICK] = 1; // has to have atleast one charge if enabled

    loadConfig(CONFIG_CHARGES_COUNT_LFG_MSG, "Chat.LfgMsgCharges", 3);
    loadConfig(CONFIG_CHARGES_COOLDOWN_LFG_MSG, "Chat.LfgMsgCooldown", 60); // 3 charges per minute
    m_configs[CONFIG_CHARGES_COOLDOWN_LFG_MSG] *= MILLISECONDS;
    if (m_configs[CONFIG_CHARGES_COOLDOWN_LFG_MSG] && !m_configs[CONFIG_CHARGES_COUNT_LFG_MSG])
        m_configs[CONFIG_CHARGES_COUNT_LFG_MSG] = 1; // has to have atleast one charge if enabled

    loadConfig(CONFIG_CHARGES_COUNT_GLOBAL_MSG, "Chat.GlobalMsgCharges", 3);
    loadConfig(CONFIG_CHARGES_COOLDOWN_GLOBAL_MSG, "Chat.GlobalMsgCooldown", 60); // 3 charges per minute
    m_configs[CONFIG_CHARGES_COOLDOWN_GLOBAL_MSG] *= MILLISECONDS;
    if (m_configs[CONFIG_CHARGES_COOLDOWN_GLOBAL_MSG] && !m_configs[CONFIG_CHARGES_COUNT_GLOBAL_MSG])
        m_configs[CONFIG_CHARGES_COUNT_GLOBAL_MSG] = 1; // has to have atleast one charge if enabled

    loadConfig(CONFIG_CHARGES_COUNT_WHISPER_MSG, "Chat.WhisperMsgCharges", 1);
    loadConfig(CONFIG_CHARGES_COOLDOWN_WHISPER_MSG, "Chat.WhisperMsgCooldown", 0); // no cooldown atm
    m_configs[CONFIG_CHARGES_COOLDOWN_WHISPER_MSG] *= MILLISECONDS;
    if (m_configs[CONFIG_CHARGES_COOLDOWN_WHISPER_MSG] && !m_configs[CONFIG_CHARGES_COUNT_WHISPER_MSG])
        m_configs[CONFIG_CHARGES_COUNT_WHISPER_MSG] = 1; // has to have atleast one charge if enabled

    loadConfig(CONFIG_SPELL_TRAINER_START_DISCOUNT, "SpellTrainerStartDiscount", 60); // 60% discount until 6 lvl 70s of different classess appear
    if (m_configs[CONFIG_SPELL_TRAINER_START_DISCOUNT] > 100)
        m_configs[CONFIG_SPELL_TRAINER_START_DISCOUNT] = 100;

    loadConfig(CONFIG_MINORITY_PROPAGANDA_CHAR, "MinorityPropagandaChar", 0);

    //loadConfig(CONFIG_BOT_STRESS_AI, "BotStressAI", 0); // 0 by default - not to stress the realm
    loadConfig(CONFIG_BOT_UNLOAD_AFTER, "Bot.UnloadAfter", 30 * MINUTE * MILLISECONDS); // 30 min in miliseconds by default
    loadConfig(CONFIG_WHO_SHOW_OVERALL, "Who.ShowOverall", false); // to show or not the full online count
    loadConfig(CONFIG_MAX_CHANNEL_SHOW, "MaxChannelReturns", 49); // default -> show only 49 people in channel

    loadConfig(CONFIG_BG_QUEUE_FROM_ANYWHERE, "BG.QueueFromAnywhere", false);
    loadConfig(CONFIG_ALLOW_TWO_SIDE_GLOBAL_CHAT, "AllowTwoSide.GlobalChat", false);

    loadConfig(CONFIG_SOLO_3V3_CAST_DESERTER, "Solo.3v3.CastDeserter", true);
    loadConfig(CONFIG_19_LVL_ADAPTATIONS, "AdaptationsFor19Lvl", false);
    loadConfig(CONFIG_ARENA_ANNOUNCE, "ArenaAnnounce", false);   

    loadConfig(CONFIG_BG_MAX_PREMADE_COUNT, "BG.MaxPremadeCount", 5);
    loadConfig(CONFIG_BOJ_FRAGMENTS_AFTER_KILL, "BOJAfterKill", 0);
    loadConfig(CONFIG_DEBUG_MASK, "Debug.Mask", DEBUG_MASK_NONE);
    loadConfig(CONFIG_ENGLISH_GUILD_ID, "AutoJoin.EnglishGuildID", 0);
    loadConfig(CONFIG_RUSSIAN_GUILD_ID, "AutoJoin.RussianGuildID", 0);
    loadConfig(CONFIG_HONOR_IN_WORLD, "HonorInWorld", true);
    loadConfig(CONFIG_LFG_FROM_LEVEL, "LFG.AvailableFromLevel", 20);
    loadConfig(CONFIG_SPECIAL_COMMAND_USERS_ENABLED, "SpecialCommandUsersEnabled", false);
    loadConfig(CONFIG_IS_LOCAL, "IsLocal", false);
    loadConfig(CONFIG_ARENA_SEASON_END, "ArenaSeasonEndTimestamp", 0);
    loadConfig(CONFIG_BOT_SKIP_UPDATES, "BotSkipUpdates", true);
    loadConfig(CONFIG_BOT_LOAD, "BotLoad", true);
    loadConfig(CONFIG_LEGENDARY_CHEST_BONUS_DROP_RATE, "LegendaryChestBonusDropRate", 0.f);
    loadConfig(CONFIG_IS_BETA, "IsBeta", false);
    loadConfig(CONFIG_FAKE_DELAY, "FakeDelay", 0);
    loadConfig(CONFIG_TMP, "TmpValue", 0); // just for temporary purposes
    loadConfig(CONFIG_RATED_BG_ENABLED, "RatedBGEnabled", false); // 0 - only stats, 1 - enabled
	loadConfig(CONFIG_LOAD_ONLY_CREATURE_GUID, "LoadOnlyCreatureGUID", 0);
	loadConfig(CONFIG_CAPTCHA_ENABLED, "CaptchaEnabled", 0);
	loadConfig(CONFIG_FAKE_MULTIPLIER, "FakeMultiplier", 0.f); // 0 - disabled
	loadConfig(CONFIG_HASHIP_DISABLED, "HashIPDisabled", false); // 0 - disabled
	loadConfig(CONFIG_RAID_DISABLE_MASK, "RaidDisableMask", 0);

    loadConfig(CONFIG_ALTERAC_ENABLED, "Alterac.Enabled", false);
	loadConfig(CONFIG_ALTERAC_EVENING_START_HOUR, "Alterac.EveningStartHour", 20);

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("select max(maxplayers) from uptime where startstring>now() - interval 7 day;");
    if (result)
        sWorld.online.weekly_max = (*result)[0].GetUInt32();
}

/// Initialize the World
void World::SetInitialWorldSettings()
{
    ///- Initialize the random number generator
    srand((unsigned int)time(NULL));

    dtAllocSetCustom(dtCustomAlloc, dtCustomFree);

    ///- Initialize config settings
    LoadConfigSettings();

    sObjectMgr.LoadRevision();


    ///- Init highest guids before any table loading to prevent using not initialized guids in some code.
    sLog.outString("SetHighestGuids...");
    sObjectMgr.SetHighestGuids();

    sLog.outString("MapManager...");
    ///- Check the existence of the map files for all races' startup areas.
    if ( !MapManager::ExistMap(0,-6240.32f, 331.033f)
        ||!MapManager::ExistMap(0,-8949.95f,-132.493f)
        ||!MapManager::ExistMap(0,-8949.95f,-132.493f)
        ||!MapManager::ExistMap(1,-618.518f,-4251.67f)
        ||!MapManager::ExistMap(0, 1676.35f, 1677.45f)
        ||!MapManager::ExistMap(1, 10311.3f, 832.463f)
        ||!MapManager::ExistMap(1,-2917.58f,-257.98f)
        ||m_configs[CONFIG_EXPANSION] && (
        !MapManager::ExistMap(530,10349.6f,-6357.29f) || !MapManager::ExistMap(530,-3961.64f,-13931.2f)))
    {
        sLog.outString("ERROR: Correct *.map files for startup zones not found in path '%smaps'. Please place *.map files in appropriate directories or correct the DataDir value in the .conf file.",m_dataPath.c_str());
        exit(1);
    }

    ///- Loading strings. Getting no records means core load has to be canceled because no error message can be output.
    sLog.outString();
    sLog.outString("Loading Hellground strings...");
    if (!sObjectMgr.LoadHellgroundStrings())
        exit(1);                                            // Error message displayed in function already

    ///- Update the realm entry in the database with the realm type from the config file
    //No SQL injection as values are treated as integers

    // not send custom type REALM_FFA_PVP to realm list
    uint32 server_type = IsFFAPvPRealm() ? REALM_TYPE_PVP : getConfig(CONFIG_GAME_TYPE);
    uint32 realm_zone = getConfig(CONFIG_REALM_ZONE);
    AccountsDatabase.PExecute("UPDATE realms SET icon = %u, timezone = %u WHERE realm_id = '%u'", server_type, realm_zone, realmID);

	if (getConfig(CONFIG_IS_BETA) && isEasyRealm())
	{
		exit(1);
		sLog.outString("x100 beta fatal error!");
	}

    ///- Remove the bones after a restart
    RealmDataDatabase.PExecute("DELETE FROM corpse WHERE corpse_type = '0'");

    ///- Load the DBC files
    sLog.outString("Initialize data stores...");

    sLog.outString("Loading spell_template...");
    sObjectMgr.LoadSpellTemplate();

    LoadDBCStores(m_dataPath);
    DetectDBCLang();

    sLog.outString("Loading Terrain specific data...");
    sTerrainMgr.LoadTerrainSpecifics();

    sLog.outString("Loading Script Names...");
    sScriptMgr.LoadScriptNames();
       
    sLog.outString("Loading CreatureMapMod...");
    LoadCreatureMapMod();

    sLog.outString("Loading InstanceTemplate");
    sObjectMgr.LoadInstanceTemplate();

    sLog.outString("Loading SkillLineAbilityMultiMap Data...");
    sSpellMgr.LoadSkillLineAbilityMap();

    ///- Clean up and pack instances
    sLog.outString("Cleaning up instances...");
    sInstanceSaveManager.CleanupInstances();                              // must be called before `creature_respawn`/`gameobject_respawn` tables

    sLog.outString("Cleaning up Warden Warnings...");
    AccountsDatabase.Execute("DELETE FROM warden_warnings WHERE DATEDIFF(now(), warning_date) >= 14");

    sLog.outString("Cleaning up account_premium_codes...");
    AccountsDatabase.Execute("DELETE FROM account_premium_codes WHERE `expire`<=UNIX_TIMESTAMP()");

    //sLog.outString("Packing instances...");
    //sInstanceSaveManager.PackInstances();

    sLog.outString("Loading Localization strings...");
    sObjectMgr.LoadCreatureLocales();
    sObjectMgr.LoadGameObjectLocales();
    sObjectMgr.LoadItemLocales();
    sObjectMgr.LoadQuestLocales();
    sObjectMgr.LoadNpcTextLocales();
    sObjectMgr.LoadPageTextLocales();
    sObjectMgr.LoadNpcOptionLocales();
    sObjectMgr.SetDBCLocaleIndex(GetDefaultDbcLocale());        // Get once for all the locale index of DBC language (console/broadcasts)

    sLog.outString("Loading Page Texts...");
    sObjectMgr.LoadPageTexts();

    sLog.outString("Loading Game Object Templates...");   // must be after LoadPageTexts
    sObjectMgr.LoadGameobjectInfo();

    sLog.outString("Loading Spell Chain Data...");
    sSpellMgr.LoadSpellChains();

    sLog.outString("Loading Spell Required Data...");
    sSpellMgr.LoadSpellRequired();

    sLog.outString("Loading Spell Elixir types...");
    sSpellMgr.LoadSpellElixirs();

    sLog.outString("Loading Spell Learn Skills...");
    sSpellMgr.LoadSpellLearnSkills();                        // must be after LoadSpellChains

    sLog.outString("Loading Spell Learn Spells...");
    sSpellMgr.LoadSpellLearnSpells();

    sLog.outString("Loading Spell Proc Event conditions...");
    sSpellMgr.LoadSpellProcEvents();

    sLog.outString("Loading Aggro Spells Definitions...");
    sSpellMgr.LoadSpellThreats();

    //sLog.outString("Loading Unqueued Account List...");
    //sObjectMgr.LoadUnqueuedAccountList();

    sLog.outString("Loading NPC Texts...");
    sObjectMgr.LoadGossipText();

    sLog.outString("Loading Enchant Spells Proc datas...");
    sSpellMgr.LoadSpellEnchantProcData();

    sLog.outString("Loading Item Random Enchantments Table...");
    LoadRandomEnchantmentsTable();

    sLog.outString("Loading Items...");                   // must be after LoadRandomEnchantmentsTable and LoadPageTexts
    sObjectMgr.LoadItemPrototypes();

    sLog.outString("Loading Item Texts...");
    sObjectMgr.LoadItemTexts();

    sLog.outString("Loading Creature Model Based Info Data...");
    sObjectMgr.LoadCreatureModelInfo();

    sLog.outString("Loading Equipment templates...");
    sObjectMgr.LoadEquipmentTemplates();

    sLog.outString("Loading Creature templates...");
    sObjectMgr.LoadCreatureTemplates();

    sLog.outString("Loading SpellsScriptTarget...");
    sSpellMgr.LoadSpellScriptTarget();                       // must be after LoadCreatureTemplates and LoadGameobjectInfo

    sLog.outString( "Loading Reputation Reward Rates...");
    sObjectMgr.LoadReputationRewardRate();

    sLog.outString("Loading Creature Reputation OnKill Data...");
    sObjectMgr.LoadReputationOnKill();

    sLog.outString( "Loading Reputation Spillover Data..." );
    sObjectMgr.LoadReputationSpilloverTemplate();

    sLog.outString("Loading Pet Create Spells...");
    sObjectMgr.LoadPetCreateSpells();

    sLog.outString("Loading Creature Data...");
    sObjectMgr.LoadCreatures();

    sLog.outString("Loading Creature Linked Respawn...");
    sObjectMgr.LoadCreatureLinkedRespawn();                     // must be after LoadCreatures()

    sLog.outString("Loading Creature Addon Data...");
    sObjectMgr.LoadCreatureAddons();                            // must be after LoadCreatureTemplates() and LoadCreatures()

    sLog.outString("Loading Creature Respawn Data...");   // must be after PackInstances()
    sObjectMgr.LoadCreatureRespawnTimes();

    sLog.outString("Loading Gameobject Data...");
    sObjectMgr.LoadGameobjects();

    sLog.outString("Loading Gameobject Respawn Data..."); // must be after PackInstances()
    sObjectMgr.LoadGameobjectRespawnTimes();

    sLog.outString("Loading Objects Pooling Data...");
    sPoolMgr.LoadFromDB();

    sLog.outString("Loading Game Event Data...");
    sGameEventMgr.LoadFromDB();

    sLog.outString("Loading Weather Data...");
    sObjectMgr.LoadWeatherZoneChances();

    sLog.outString("Loading Quests...");
    sObjectMgr.LoadQuests();                                    // must be loaded after DBCs, creature_template, item_template, gameobject tables

    sLog.outString("Loading Quests Relations...");
    sObjectMgr.LoadQuestRelations();                            // must be after quest load

    sLog.outString("Loading AreaTrigger definitions...");
    sObjectMgr.LoadAreaTriggerTeleports();

    sLog.outString("Loading Access Requirements...");
    sObjectMgr.LoadAccessRequirements();                        // must be after item template load

    sLog.outString("Loading Quest Area Triggers...");
    sObjectMgr.LoadQuestAreaTriggers();                         // must be after LoadQuests

    sLog.outString("Loading Tavern Area Triggers...");
    sObjectMgr.LoadTavernAreaTriggers();

    sLog.outString("Loading AreaTrigger script names...");
    sScriptMgr.LoadAreaTriggerScripts();

    sLog.outString("Loading CompletedCinematic script names...");
    sScriptMgr.LoadCompletedCinematicScripts();

    sLog.outString("Loading event id script names...");
    sScriptMgr.LoadEventIdScripts();

    sLog.outString("Loading spell id script names...");
    sScriptMgr.LoadSpellIdScripts();

    sLog.outString("Loading Graveyard-zone links...");
    sObjectMgr.LoadGraveyardZones();

    sLog.outString("Loading Spell target coordinates...");
    sSpellMgr.LoadSpellTargetPositions();

    sLog.outString("Loading SpellAffect definitions...");
    sSpellMgr.LoadSpellAffects();

    sLog.outString("Loading spell pet auras...");
    sSpellMgr.LoadSpellPetAuras();

    sLog.outString("Loading spell analogs...");
    sSpellMgr.LoadSpellAnalogs();

    sLog.outString("Loading custom spell item enchantments...");
    sSpellMgr.LoadCustomSpellItemEnchantments();

    sLog.outString("Loading spell extra attributes...");
    sSpellMgr.LoadSpellCustomAttr();

    sLog.outString("Loading linked spells...");
    sSpellMgr.LoadSpellLinked();

    sLog.outString("Loading player Create Info & Level Stats...");
    sObjectMgr.LoadPlayerInfo();

    sLog.outString("Loading Exploration BaseXP Data...");
    sObjectMgr.LoadExplorationBaseXP();

    sLog.outString("Loading Pet Name Parts...");
    sObjectMgr.LoadPetNames();

    sLog.outString("Loading the max pet number...");
    sObjectMgr.LoadPetNumber();

    sLog.outString("Loading pet level stats...");
    sObjectMgr.LoadPetLevelInfo();

    sLog.outString("Loading Player Corpses...");
    sObjectMgr.LoadCorpses();

    sLog.outString("Loading Disabled Spells...");
    sObjectMgr.LoadSpellDisabledEntrys();

    sLog.outString("Loading Loot Tables...");
    LoadLootTables();

    sLog.outString("Loading Skill Discovery Table...");
    LoadSkillDiscoveryTable();

    sLog.outString("Loading Skill Extra Item Table...");
    LoadSkillExtraItemTable();

    sLog.outString("Loading Skill Fishing base level requirements...");
    sObjectMgr.LoadFishingBaseSkillLevel();

    ///- Load dynamic data tables from the database
    sLog.outString("Loading Auctions...");
    sAuctionMgr.LoadAuctionItems();
    sAuctionMgr.LoadAuctions();

    sLog.outString("Loading Guilds...");
    sGuildMgr.LoadGuilds();

	sLog.outString("Loading Guild House...");
    LoadGuildHouse();

    sLog.outString("Loading ArenaTeams...");
    sObjectMgr.LoadArenaTeams();

    sLog.outString("Loading Groups...");
    sObjectMgr.LoadGroups();

    sLog.outString("Loading ReservedNames...");
    sObjectMgr.LoadReservedPlayersNames();

    //sLog.outString("Loading GameObject for quests...");
    //sObjectMgr.LoadGameObjectForQuests();

    sLog.outString("Loading BattleMasters...");
    sBattleGroundMgr.LoadBattleMastersEntry();

    sLog.outString("Loading GameTeleports...");
    sObjectMgr.LoadGameTele();

    sLog.outString("Loading Npc Text Id...");
    sObjectMgr.LoadNpcTextId();                                 // must be after load Creature and NpcText

    sLog.outString("Loading Npc Options...");
    sObjectMgr.LoadNpcOptions();

    sLog.outString("Loading vendors...");
    sObjectMgr.LoadVendors();                                   // must be after load CreatureTemplate and ItemPrototype

    sLog.outString("Loading trainers...");
    sObjectMgr.LoadTrainerSpell();                              // must be after load CreatureTemplate

    sLog.outString("Loading opcodes cooldown...");
    sObjectMgr.LoadOpcodesCooldown();

    sLog.outString("Loading Waypoints...");
    sWaypointMgr.Load();

    sLog.outString("Loading Creature Formations...");
    CreatureGroupManager::LoadCreatureFormations();

    //sLog.outString("Loading GM tickets...");
    //sTicketMgr.LoadGMTickets();

    ///- Handle outdated emails (delete/return)
    sLog.outString("Returning old mails...");
    sObjectMgr.ReturnOrDeleteOldMails(false);

    sLog.outString("Loading Autobroadcasts...");
    LoadAutobroadcasts();

    ///- Load and initialize scripts
    sLog.outString("Loading Scripts...");
    sScriptMgr.LoadQuestStartScripts();                         // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sScriptMgr.LoadQuestEndScripts();                           // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sScriptMgr.LoadSpellScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sScriptMgr.LoadGameObjectScripts();                         // must be after load Creature/Gameobject(Template/Data)
    sScriptMgr.LoadEventScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sScriptMgr.LoadWaypointScripts();

    sLog.outString("Loading Scripts text locales...");    // must be after Load*Scripts calls
    sScriptMgr.LoadDbScriptStrings();

    sLog.outString("Loading CreatureEventAI Texts...");
    sCreatureEAIMgr.LoadCreatureEventAI_Texts(false);       // false, will checked in LoadCreatureEventAI_Scripts

    sLog.outString("Loading CreatureEventAI Summons...");
    sCreatureEAIMgr.LoadCreatureEventAI_Summons(false);     // false, will checked in LoadCreatureEventAI_Scripts

    sLog.outString("Loading CreatureEventAI Positions...");
    sCreatureEAIMgr.LoadCreatureEventAI_Positions(false);     // false, will checked in LoadCreatureEventAI_Scripts

    sLog.outString("Loading CreatureEventAI Scripts...");
    sCreatureEAIMgr.LoadCreatureEventAI_Scripts();

    sLog.outString("Initializing Scripts...");
    sScriptMgr.LoadScriptLibrary();

    ///- Initialize game time and timers
    sLog.outDebug("DEBUG:: Initialize game time and timers");
    m_gameTime = time(NULL);
    m_startTime=m_gameTime;

    tm local;
    time_t curr;
    time(&curr);
    local=*(localtime(&curr));                              // dereference and assign
    char isoDate[128];
    sprintf(isoDate, "%04d-%02d-%02d %02d:%02d:%02d",
        local.tm_year+1900, local.tm_mon+1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);

    RealmDataDatabase.PExecute("INSERT INTO uptime (startstring, starttime, uptime) VALUES('%s', %llu, 0)",
        isoDate, uint64(m_startTime));

    m_timers[WUPDATE_OBJECTS].SetInterval(0); // updated every world tick
    m_timers[WUPDATE_SESSIONS].SetInterval(0); // updated every world tick
    m_timers[WUPDATE_WEATHERS].SetInterval(1000);
    m_timers[WUPDATE_AUCTIONS].SetInterval(MINUTE*MILLISECONDS);    //set auction update interval to 1 minute
    m_timers[WUPDATE_UPTIME].SetInterval(m_configs[CONFIG_UPTIME_UPDATE]*MINUTE*MILLISECONDS);
                                                            //Update "uptime" table based on configuration entry in minutes.
    m_timers[WUPDATE_CORPSES].SetInterval(20*MINUTE*MILLISECONDS);  //erase corpses every 20 minutes. Also calls CleanOldCharges() from various classess

    m_timers[WUPDATE_AUTOBROADCAST].SetInterval(getConfig(CONFIG_AUTOBROADCAST_INTERVAL));
    m_timers[WUPDATE_GUILD_ANNOUNCES].SetInterval(getConfig(CONFIG_GUILD_ANN_INTERVAL));
    m_timers[WUPDATE_DELETECHARS].SetInterval(DAY*MILLISECONDS); // check for chars to delete every day
    m_timers[WUPDATE_OLDMAILS].SetInterval(getConfig(CONFIG_RETURNOLDMAILS_INTERVAL)*1000);
    m_timers[WUPDATE_EXTERNALMAILS].SetInterval(5*MILLISECONDS);
    m_timers[WUPDATE_ONLINE].SetInterval(10*MILLISECONDS);

    //to set mailtimer to return mails every day between 4 and 5 am
    //mailtimer is increased when updating auctions
    //one second is 1000 -(tested on win system)
    mail_timer = ((((localtime(&m_gameTime)->tm_hour + 20) % 24)* HOUR * 1000) / m_timers[WUPDATE_OLDMAILS].GetInterval());
                                                            //1440
    mail_timer_expires = ((DAY * 1000) / (m_timers[WUPDATE_OLDMAILS].GetInterval()));
    sLog.outDebug("Mail timer set to: %u, mail return is called every %u minutes", mail_timer, mail_timer_expires);

    ///- Initilize static helper structures
    AIRegistry::Initialize();
    Player::InitVisibleBits();

    ///- Initialize MapManager
    sLog.outString("Starting Map System");
    sMapMgr.Initialize();

    ///- Initialize Battlegrounds
    sLog.outString("Starting BattleGround System");
    sBattleGroundMgr.CreateInitialBattleGrounds();
    sBattleGroundMgr.InitAutomaticArenaPointDistribution();

    //Not sure if this can be moved up in the sequence (with static data loading) as it uses MapManager
    sLog.outString("Loading Transports...");
    sMapMgr.LoadTransports();

    sLog.outString("Loading Transports Events...");
    sObjectMgr.LoadTransportEvents();

    sLog.outString("Creating continents...");
    if (!CreateContinents())
        exit(1);

    ///- Initialize outdoor pvp
    sLog.outString("Starting Outdoor PvP System");
    sOutdoorPvPMgr.InitOutdoorPvP();

    sLog.outString("Deleting expired IP bans...");
    AccountsDatabase.Execute("DELETE FROM ip_banned WHERE expiration_date <= UNIX_TIMESTAMP() AND expiration_date <> punishment_date");

    sLog.outString("Starting objects Pooling system...");
    sPoolMgr.Initialize();

    sLog.outString("Calculate next daily quest reset time...");
    InitDailyQuestResetTime();

    //sLog.outString("Selecting level caps per class...");
    //LoadMaxLvlReachedPerClass();

    sLog.outString("Starting Game Event system...");
    uint32 nextGameEvent = sGameEventMgr.Initialize();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);    //depend on next event

    sLog.outString("Loading Warden Data..." );
    sWardenDataStorage.Init();

    sLog.outString("Cleanup deleted characters");
    CleanupDeletedChars();

    sLog.outString("Loading Shop...");
    GetShop()->LoadShop();

    //sLog.outString("Loading arena spec/item restrictions...");
    //sBattleGroundMgr.LoadArenaRestrictions();

    sLog.outString("Loading arena items to log...");
    sBattleGroundMgr.LoadArenaItemsLogging();

    if (getConfig(CONFIG_COREBALANCER_ENABLED))
        _coreBalancer.Initialize();

    if (sConfig.GetIntDefault("Server.AutorestartPeriod", 0) != 0)
        ShutdownServ(sConfig.GetIntDefault("Server.AutorestartPeriod", 0), SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, "Autorestart");

    LoadMinorityPropagandaChar();

    if (sWorld.getConfig(CONFIG_BOT_LOAD))
    {
        sLog.outString("Loading bots...");
        LoadFakeBotInfo();
    }

    if (sWorld.getConfig(CONFIG_IS_LOCAL))
    {
        sBattleGroundMgr.SetDebugArenaId(BATTLEGROUND_RL);
        sBattleGroundMgr.ToggleTesting();
    }

    // loadadditional

    LoadChestAnnounce();
    LoadArena3v3Banned();
    LoadLexicsCutter();
    LoadCustomVendor();

    LoadOtherStuff();

    //checks
    CriticalChecks();

    sLog.outString("WORLD: World initialized");
}

void World::DetectDBCLang()
{
    uint32 m_lang_confid = sConfig.GetIntDefault("Locale", 255);

    if (m_lang_confid != 255 && m_lang_confid >= MAX_LOCALE)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Incorrect DBC.Locale! Must be >= 0 and < %d (set to 0)",MAX_LOCALE);
        m_lang_confid = LOCALE_enUS;
    }

    ChrRacesEntry const* race = sChrRacesStore.LookupEntry(1);

    std::string availableLocalsStr;

    int default_locale = MAX_LOCALE;
    for (int i = MAX_LOCALE-1; i >= 0; --i)
    {
        if (strlen(race->name[i]) > 0)                     // check by race names
        {
            default_locale = i;
            m_availableDbcLocaleMask |= (1 << i);
            availableLocalsStr += localeNames[i];
            availableLocalsStr += " ";
        }
    }

    if (default_locale != m_lang_confid && m_lang_confid < MAX_LOCALE &&
        (m_availableDbcLocaleMask & (1 << m_lang_confid)))
    {
        default_locale = m_lang_confid;
    }

    if (default_locale >= MAX_LOCALE)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Unable to determine your DBC Locale! (corrupt DBC?)");
        exit(1);
    }

    m_defaultDbcLocale = LocaleConstant(default_locale);

    sLog.outString("Using %s DBC Locale as default. All available DBC locales: %s",localeNames[m_defaultDbcLocale],availableLocalsStr.empty() ? "<none>" : availableLocalsStr.c_str());
}

void World::LoadAutobroadcasts()
{
    m_Autobroadcasts.clear();
    m_Autobroadcast_advance = 0;

    LoadHellgroundStrings(GameDataDatabase, "autobroadcast", MIN_DB_AUTOBROADCAST_STRING_ID, MAX_DB_AUTOBROADCAST_STRING_ID);

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT `entry` FROM autobroadcast WHERE `show`=1");

    if (!result)
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 autobroadcasts definitions: database empty");
        return;
    }

    uint32 count = 0;
    do
    {
        int32 entry = (*result)[0].GetInt32();

        if (entry < MIN_DB_AUTOBROADCAST_STRING_ID || entry >= MAX_DB_AUTOBROADCAST_STRING_ID)
        {
            sLog.outLog(LOG_DB_ERR, "ERROR: Entry %i in table `autobroadcasts` is out of accepted entry range for table.", entry);
            continue;
        }

        m_Autobroadcasts.push_back(uint32(entry));
        count++;
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u autobroadcast definitions", count);
}

void World::LoadAnnounces()
{
    LoadHellgroundStrings(GameDataDatabase, "announces", MIN_DB_ANNOUNCES_STRING_ID, MAX_DB_ANNOUNCES_STRING_ID);

    sLog.outString();
    sLog.outString(">> Reloaded 'announces' table");
}

/// @world
void World::Update(uint32 diff)
{
	if (getConfig(CONFIG_FAKE_DELAY))
		std::this_thread::sleep_for(std::chrono::milliseconds(getConfig(CONFIG_FAKE_DELAY)));

	m_updateTime = uint32(diff);

    if (getConfig(CONFIG_COREBALANCER_ENABLED))
        _coreBalancer.Update(diff);

    bool accumulateMapDiff = getConfig(CONFIG_CUMULATIVE_LOG_METHOD) == 1 ? true : false;
    if (getConfig(CONFIG_INTERVAL_LOG_UPDATE))
    {
        if (m_updateTimeSum > getConfig(CONFIG_INTERVAL_LOG_UPDATE))
        {
            accumulateMapDiff = true;

            float curAvgUpdateTime = (float)m_updateTimeSum / (float)m_updateTimeCount;   // from last log time
            m_serverUpdateTimeSum += m_updateTimeSum;
            m_serverUpdateTimeCount += m_updateTimeCount;

            float avgUpdateTime = (float)m_serverUpdateTimeSum / (float)m_serverUpdateTimeCount; // from server start

            sLog.outLog(LOG_DEFAULT, "[Diff]: Update time diff: %.3f, avg: %.3f. Players online: %u. Uptime %s.", curAvgUpdateTime, avgUpdateTime, GetActiveSessionCount(), (secondsToTimeStringEn(sWorld.GetUptime()).c_str()));
            sLog.outLog(LOG_DIFF, "Update time diff: %.3f, avg: %.3f. Players online: %u. Uptime %s.", curAvgUpdateTime, avgUpdateTime, GetActiveSessionCount(), (secondsToTimeStringEn(sWorld.GetUptime()).c_str()));
            sLog.outLog(LOG_STATUS, "%u %u %u %u %u %u %s %.3f %.3f %u %u %ld",
                        GetUptime(), GetActiveSessionCount(), GetMaxActiveSessionCount(), GetQueuedSessionCount(), GetMaxQueuedSessionCount(),
                        m_playerLimit, REVISION, curAvgUpdateTime, avgUpdateTime, loggedInAlliances.value(), loggedInHordes.value(), m_gameTime);

            m_updateTimeSum = m_updateTime;
            m_updateTimeCount = 1;
        }
        else
        {
            m_updateTimeSum += m_updateTime;
            ++m_updateTimeCount;
        }
    }

    DiffRecorder diffRecorder(__FUNCTION__, getConfig(CONFIG_MIN_LOG_UPDATE));

    ///- Update the different timers
    for (int i = 0; i < WUPDATE_COUNT; i++)
    {
        if (m_timers[i].GetCurrent()>=0)
            m_timers[i].Update(diff);
        else
            m_timers[i].SetCurrent(0);
    }

    diffRecorder.RecordTimeFor("UpdateTimers");

    ///- Update the game time and check for shutdown time
    _UpdateGameTime();

    diffRecorder.RecordTimeFor("UpdateGameTime");

    /// Handle daily quests reset time
    if (m_gameTime > m_NextDailyQuestReset)
    {
        ResetDailyQuests();
        m_NextDailyQuestReset += DAY;

        tm localTm = *localtime(&m_gameTime);
        if (localTm.tm_wday == 1) // 1 is monday
            sGuildMgr.UpdateWeek();

        diffRecorder.RecordTimeFor("ResetDailyQuests");
    }
    /// <ul><li> Handle auctions when the timer has passed
    if (m_timers[WUPDATE_OLDMAILS].Passed())
    {
        m_timers[WUPDATE_OLDMAILS].Reset();

        ///- Update mails (return old mails with item, or delete them)
        switch (m_configs[CONFIG_RETURNOLDMAILS_MODE])
        {
            case 1:
                sObjectMgr.ReturnOrDeleteOldMails(true);
                break;
            case 0:
            default:
                if (++mail_timer > mail_timer_expires)
                {
                    mail_timer = 0;
                    sObjectMgr.ReturnOrDeleteOldMails(true);
                }
                break;
        }

        diffRecorder.RecordTimeFor("ReturnOldMails");
    }

    /// <ul><li> Handle auctions when the timer has passed
    if (m_timers[WUPDATE_EXTERNALMAILS].Passed())
    {
        m_timers[WUPDATE_EXTERNALMAILS].Reset();
        SendExternalMails();
    }

    if (m_timers[WUPDATE_ONLINE].Passed())
    {
        // update weekly max
        if (GetMaxActiveSessionCount() > online.weekly_max)
            online.weekly_max = GetMaxActiveSessionCount();

        // add fakebots
        float fake_multiplier = getConfig(CONFIG_FAKE_MULTIPLIER);
        if (fake_multiplier)
        {
            if (fakebot_load_queue >= 10)
            {
                sLog.outLog(LOG_CRITICAL, "Fatal! fakebot_load_queue is %u", fakebot_load_queue);
            }
            else
            {
                uint32 current_online = GetRealSessionCount();

                double calc = 1.0 / (1.0 + exp(-0.1 * (current_online - 10.0)));
                int32 to_load = std::round(std::sqrt(current_online) * std::sqrt(current_online) * calc * fake_multiplier - online.fake);

                if (to_load > 0)
                {
                    if (online.fake + to_load > GetRealSessionCount() * 2)
                    {
                        sLog.outLog(LOG_CRITICAL, "Fatal! online.fake - %u > GetRealSessionCount() * 2 - %u", online.fake, GetRealSessionCount() * 2);
                    }
                    else
                    {
                        if (to_load > 3)
                            to_load = 3;

                        if (to_load > 1)
                            to_load = urand(1, to_load);

                        AddFakeBots(to_load);
                    }
                }
            }
        }

        RealmDataDatabase.PExecute("UPDATE online SET total = %u, fake = %u", GetActiveSessionCount(), online.fake);
        m_timers[WUPDATE_ONLINE].Reset();
    }

    /// <ul><li> Handle auctions when the timer has passed
    if (m_timers[WUPDATE_AUCTIONS].Passed())
    {
        m_timers[WUPDATE_AUCTIONS].Reset();
        ///-Handle expired auctions
        sAuctionMgr.Update();
        diffRecorder.RecordTimeFor("UpdateAuctions");
    }

    /// <li> Handle session updates when the timer has passed
    if (m_timers[WUPDATE_SESSIONS].Passed())
    {
        m_timers[WUPDATE_SESSIONS].Reset();

        UpdateSessions(diff);

        diffRecorder.RecordTimeFor("UpdateSessions");

        // Update groups
        for (ObjectMgr::GroupSet::iterator itr = sObjectMgr.GetGroupSetBegin(); itr != sObjectMgr.GetGroupSetEnd(); ++itr)
            (*itr)->Update(diff);

        diffRecorder.RecordTimeFor("UpdateGroups");

        sObjectMgr.UpdateRolls(diff);
        diffRecorder.RecordTimeFor("UpdateRolls");
    }

    sWorldEventProcessor.ExecuteEvents();
    diffRecorder.RecordTimeFor("ExecuteWorldEvents");

    /// <li> Handle weather updates when the timer has passed
    if (m_timers[WUPDATE_WEATHERS].Passed())
    {
        m_timers[WUPDATE_WEATHERS].Reset();

        ///- Send an update signal to Weather objects
        WeatherMap::iterator itr, next;
        for (itr = m_weathers.begin(); itr != m_weathers.end(); itr = next)
        {
            next = itr;
            ++next;

            ///- and remove Weather objects for zones with no player as interval > WorldTick
            if (!itr->second->Update(m_timers[WUPDATE_WEATHERS].GetInterval()))
            {
                Weather *temp = itr->second;
                m_weathers.erase(itr);
                delete temp;
            }
        }

        diffRecorder.RecordTimeFor("UpdateWeathers");
    }

    /// <li> Update uptime table
    if (m_timers[WUPDATE_UPTIME].Passed())
    {
        uint32 tmpDiff = (m_gameTime - m_startTime);
        uint32 maxClientsNum = GetMaxActiveSessionCount();

        m_timers[WUPDATE_UPTIME].Reset();
        RealmDataDatabase.PExecute("UPDATE uptime SET uptime = %d, maxplayers = %d WHERE starttime = %llu", tmpDiff, maxClientsNum, uint64(m_startTime));
    }

    diffRecorder.ResetDiff();

    if (getConfig(CONFIG_AUTOBROADCAST_INTERVAL))
    {
        if (m_timers[WUPDATE_AUTOBROADCAST].Passed())
        {
            m_timers[WUPDATE_AUTOBROADCAST].Reset();
            if (!m_Autobroadcasts.empty())
            {
                m_Autobroadcast_advance = m_Autobroadcast_advance % m_Autobroadcasts.size();

                std::list<uint32>::const_iterator itr = m_Autobroadcasts.begin();
                std::advance(itr, m_Autobroadcast_advance);
                ++m_Autobroadcast_advance;

                SendWorldText(*itr, ACC_DISABLED_BROADCAST);
            }
        }

        diffRecorder.RecordTimeFor("Send Autobroadcast");
    }

    ///- send guild announces every one minute
    if (m_timers[WUPDATE_GUILD_ANNOUNCES].Passed())
    {
        m_timers[WUPDATE_GUILD_ANNOUNCES].Reset();
        if (!m_GuildAnnounces.empty())
        {
            std::list<std::pair<uint64, std::string> >::iterator itr = m_GuildAnnounces.begin();
            std::string guildName = sGuildMgr.GetGuildNameById(PAIR64_LOPART(itr->first));

            sWorld.SendWorldText(LANG_GUILD_ANNOUNCE, ACC_DISABLED_GANN, guildName.c_str(), itr->second.c_str());
            m_GuildAnnounces.pop_front();
        }

        diffRecorder.RecordTimeFor("Send Guild announce");
    }

    /// <li> Handle all other objects
    ///- Update objects when the timer has passed (maps, transport, creatures,...)
    MAP_UPDATE_DIFF(MapUpdateDiff().InitializeMapData())

    sMapMgr.Update(diff);                // As interval = 0
    
    sObjectMgr.SingleInstanceChangeApply();

    uint32 mapDiff = diffRecorder.RecordTimeFor("MapManager::update");

    if (accumulateMapDiff)
    {
        if (mapDiff > sWorld.getConfig(CONFIG_MIN_LOG_UPDATE))
            MAP_UPDATE_DIFF(MapUpdateDiff().PrintCumulativeMapUpdateDiff());
        MapUpdateDiff().ClearDiffInfo();
    }

    sBattleGroundMgr.Update(diff);
    diffRecorder.RecordTimeFor("UpdateBattleGroundMgr");

    sOutdoorPvPMgr.Update(diff);
    diffRecorder.RecordTimeFor("UpdateOutdoorPvPMgr");

    ///- Delete all characters which have been deleted X days before
    if (m_timers[WUPDATE_DELETECHARS].Passed())
    {
        m_timers[WUPDATE_DELETECHARS].Reset();
        CleanupDeletedChars();
        diffRecorder.RecordTimeFor("CleanupDeletedChars");
    }

    // execute callbacks from sql queries that were queued recently
    UpdateResultQueue();
    diffRecorder.RecordTimeFor("UpdateResultQueue");

    ///- Erase corpses once every 20 minutes
    if (m_timers[WUPDATE_CORPSES].Passed())
    {
        m_timers[WUPDATE_CORPSES].Reset();

        sSocialMgr.CleanOldCharges();
        CleanOldCharges();

        sObjectAccessor.RemoveOldCorpses();
        diffRecorder.RecordTimeFor("RemoveOldCorpses");
    }

    ///- Process Game events when necessary
    if (m_timers[WUPDATE_EVENTS].Passed())
    {
        m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
        uint32 nextGameEvent = sGameEventMgr.Update();
        m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
        m_timers[WUPDATE_EVENTS].Reset();
        diffRecorder.RecordTimeFor("UpdateGameEvents");
    }
    /// </ul>

    // update the instance reset times
    sInstanceSaveManager.Update();
    diffRecorder.RecordTimeFor("UpdateSaveMGR");

    // And last, but not least handle the issued cli commands
    ProcessCliCommands();
    diffRecorder.RecordTimeFor("UpdateProcessCLI");

    //cleanup unused GridMap objects as well as VMaps
    sTerrainMgr.Update(diff);
    diffRecorder.RecordTimeFor("UpdateTerrainMGR");

    sTicketMgr.Update(diff);
    diffRecorder.RecordTimeFor("UpdateTicketMGR");
}

void World::UpdateSessions(const uint32 & diff)
{
    ///- Add new sessions
    WorldSession* sess = nullptr;
    while (addSessQueue.next(sess))
        AddSession_ (sess);

    ///- Then send an update signal to remaining ones
    for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
    {
        next = itr;
        ++next;

        if (!itr->second)
            continue;

        ///- and remove not active sessions from the list
        WorldSession * pSession = itr->second;
        WorldSessionFilter updater(pSession);

        if (pSession->isFakeBot())
        {
            if (!pSession->UpdateFakeBot(diff, updater))   // As interval = 0
            {
                RemoveQueuedPlayer(pSession);
                AddSessionToRemove(itr);
            }
        }
        else
        {
            if (!pSession->Update(diff, updater))   // As interval = 0
            {
                RemoveQueuedPlayer(pSession);
                AddSessionToRemove(itr);
            }
        }
    }

    for (std::list<SessionMap::iterator>::iterator itr = removedSessions.begin(); itr != removedSessions.end(); ++itr)
    {
        sess = (*itr)->second;
        m_sessions.erase(*itr);
        delete sess;
        sess = NULL;
    }

    removedSessions.clear();
}

void World::ForceGameEventUpdate()
{
    m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
    uint32 nextGameEvent = sGameEventMgr.Update();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
    m_timers[WUPDATE_EVENTS].Reset();
}

/// Send a packet to all players (except self if mentioned)
void World::SendGlobalMessage(WorldPacket *packet, WorldSession *self, PlayerTeam team)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second != self &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team))
        {
            itr->second->SendPacket(packet);
            //itr->second->SendAreaTriggerMessage("hello");
        }
    }
}

void World::QueueGuildAnnounce(uint32 guildid, uint32 team, std::string &msg)
{
    std::pair<uint64, std::string> temp;
    //                           low, high
    temp.first = MAKE_PAIR64(guildid, team);
    temp.second = msg;
    m_GuildAnnounces.push_back(temp);
}

void World::SendGlobalGMMessage(WorldPacket *packet, WorldSession *self, PlayerTeam team)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); itr++)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second != self &&
            itr->second->HasPermissions(sWorld.getConfig(CONFIG_MIN_GM_TEXT_LVL)) &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team))
        {
            itr->second->SendPacket(packet);
        }
    }
}

/// Send a System Message to all players (except self if mentioned)
void World::SendWorldText(int32 string_id, uint32 preventFlags, ...)
{
    if (sWorld.ServerLoading())
        return;
    
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld() || itr->second->IsAccountFlagged(AccountFlags(preventFlags)))
            continue;

        int loc_idx = itr->second->GetSessionDbLocaleIndexCheckRussian();
        uint32 cache_idx = loc_idx + 1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if (data_cache.size() < cache_idx + 1 || data_cache[cache_idx].empty())
        {
            if (data_cache.size() < cache_idx + 1)
                data_cache.resize(cache_idx + 1);

            data_list = &data_cache[cache_idx];

            const char* text = sObjectMgr.GetHellgroundString(string_id, loc_idx);
            
            // autobroadcast
            if (string_id >= 2000000000)
            {
                char result[1000];
                sprintf(result, "|cff6ec4f9%s|r", text);
                text = result;
            }

            char buf[1000];

            va_list argptr;
            va_start(argptr, preventFlags);
            vsnprintf(buf, 1000, text, argptr);
            va_end(argptr);

            char* pos = &buf[0];

            while (char* line = ChatHandler::LineFromMessage(pos))
            {
                WorldPacket* data = new WorldPacket();
                ChatHandler::FillMessageData(data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, line, NULL);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for (int i = 0; i < data_list->size(); ++i)
            itr->second->SendPacket((*data_list)[i]);
    }

    // free memory
    for (int i = 0; i < data_cache.size(); ++i)
        for (int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

// send global message for players in range <minLevel, maxLevel>
// send global message for players in range <minLevel, maxLevel> which don't have account flags
// setted in 'preventFlags'
void World::SendWorldTextForLevels(uint32 minLevel, uint32 maxLevel, uint32 preventFlags, int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld())
            continue;

        if (itr->second->GetPlayer()->GetLevel() < minLevel || itr->second->GetPlayer()->GetLevel() > maxLevel)
            continue;

        if (itr->second->IsAccountFlagged(AccountFlags(preventFlags)))
            continue;

        int loc_idx = itr->second->GetSessionDbLocaleIndexCheckRussian();
        uint32 cache_idx = loc_idx+1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if (data_cache.size() < cache_idx+1 || data_cache[cache_idx].empty())
        {
            if (data_cache.size() < cache_idx+1)
                data_cache.resize(cache_idx+1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr.GetHellgroundString(string_id,loc_idx);

            char buf[1000];

            va_list argptr;
            va_start(argptr, string_id);
            vsnprintf(buf,1000, text, argptr);
            va_end(argptr);

            char* pos = &buf[0];

            while (char* line = ChatHandler::LineFromMessage(pos))
            {
                WorldPacket* data = new WorldPacket();
                ChatHandler::FillMessageData(data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, line, NULL);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for (int i = 0; i < data_list->size(); ++i)
            itr->second->SendPacket((*data_list)[i]);
    }

    // free memory
    for (int i = 0; i < data_cache.size(); ++i)
        for (int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

void World::SendGMText(int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld())
            continue;

        int loc_idx = itr->second->GetSessionDbLocaleIndexCheckRussian();
        uint32 cache_idx = loc_idx+1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if (data_cache.size() < cache_idx+1 || data_cache[cache_idx].empty())
        {
            if (data_cache.size() < cache_idx+1)
                data_cache.resize(cache_idx+1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr.GetHellgroundString(string_id,loc_idx);

            char buf[1000];

            va_list argptr;
            va_start(argptr, string_id);
            vsnprintf(buf,1000, text, argptr);
            va_end(argptr);

            char* pos = &buf[0];

            while (char* line = ChatHandler::LineFromMessage(pos))
            {
                WorldPacket* data = new WorldPacket();
                ChatHandler::FillMessageData(data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, line, NULL);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for (int i = 0; i < data_list->size(); ++i)
            if (itr->second->HasPermissions(sWorld.getConfig(CONFIG_MIN_GM_TEXT_LVL)))
                itr->second->SendPacket((*data_list)[i]);
    }

    // free memory
    for (int i = 0; i < data_cache.size(); ++i)
        for (int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

/// Send a System Message to all players (except self if mentioned)
void World::SendGlobalText(const char* text, WorldSession *self)
{
    WorldPacket data;
    sLog.outBasic("%s", text);
    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(text);
    char* pos = buf;

    while (char* line = ChatHandler::LineFromMessage(pos))
    {
        ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, line, NULL);
        SendGlobalMessage(&data, self);
    }

    delete [] buf;
}

/// Send a packet to all players (or players selected team) in the zone (except self if mentioned)
void World::SendZoneMessage(uint32 zone, WorldPacket *packet, WorldSession *self, PlayerTeam team)
{
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second->GetPlayer()->GetCachedZone() == zone &&
            itr->second != self &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team))
        {
            itr->second->SendPacket(packet);
        }
    }
}

/// Send a System Message to all players in the zone (except self if mentioned)
void World::SendZoneText(uint32 zone, const char* text, WorldSession *self, PlayerTeam team)
{
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendZoneMessage(zone, &data, self,team);
}

/// Kick (and save) all players
void World::KickAll()
{
    m_QueuedPlayer.clear();                                 // prevent send queue update packet and login queued sessions

    // session not removed at kick and will removed in next update tick
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        itr->second->KickPlayer();
}

/// Kick (and save) all players with security level less `sec`
void World::KickAllWithoutPermissions(uint64 perms)
{
    // session not removed at kick and will removed in next update tick
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (!itr->second->HasPermissions(perms))
            itr->second->KickPlayer();
}

/// Kick (and save) the designated player
bool World::KickPlayer(const std::string& playerName)
{
    SessionMap::iterator itr;

    // session not removed at kick and will removed in next update tick
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second)
            continue;
        Player *player = itr->second->GetPlayer();
        if (!player)
            continue;
        if (player->IsInWorld())
        {
            if (playerName == player->GetName())
            {
                itr->second->KickPlayer();
                return true;
            }
        }
    }
    return false;
}

/// Ban an account or ban an IP address, duration will be parsed using TimeStringToSecs if it is positive, otherwise permban
BanReturn World::BanAccount(BanMode mode, std::string nameIPOrMail, std::string duration, std::string reason, std::string author)
{
    AccountsDatabase.escape_string(nameIPOrMail);
    AccountsDatabase.escape_string(reason);
    std::string safe_author=author;
    if (safe_author.length() <= 1)
        safe_author = "[CONSOLE]";
    AccountsDatabase.escape_string(safe_author);

    uint32 duration_secs = 0;
    duration_secs = TimeStringToSecs(duration);

    QueryResultAutoPtr resultAccounts = QueryResultAutoPtr(NULL);   //used for kicking

    ///- Update the database with ban information
    switch (mode)
    {
        case BAN_IP:
            //No SQL injection as strings are escaped
            resultAccounts = AccountsDatabase.PQuery("SELECT account_id FROM account WHERE last_ip = '%s'", nameIPOrMail.c_str());
            AccountsDatabase.PExecute("INSERT INTO ip_banned VALUES ('%s', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, '%s', '%s')", nameIPOrMail.c_str(), duration_secs, safe_author.c_str(), reason.c_str());
            break;
        case BAN_ACCOUNT:
            //No SQL injection as string is escaped
            resultAccounts = AccountsDatabase.PQuery("SELECT account_id FROM account WHERE username = '%s'", nameIPOrMail.c_str());
            break;
        case BAN_ARENA3V3:
        case BAN_CHARACTER:
            //No SQL injection as string is escaped
            resultAccounts = RealmDataDatabase.PQuery("SELECT account FROM characters WHERE name = '%s'", nameIPOrMail.c_str());
            break;
        default:
            return BAN_SYNTAX_ERROR;
    }

    if (!resultAccounts)
    {
        switch (mode)
        {
            case BAN_IP:
                return BAN_SUCCESS;
            default:
                return BAN_NOTFOUND;                            // Nobody to ban
        }
    }

    ///- Disconnect all affected players (for IP it can be several)
    do
    {
        Field* fieldsAccount = resultAccounts->Fetch();
        uint32 account = fieldsAccount[0].GetUInt32();
        uint32 permissions = PERM_PLAYER;

        // bot
        if (isBotAccount(account))
            continue;

        QueryResultAutoPtr resultAccPerm = AccountsDatabase.PQuery("SELECT mask FROM account_gm WHERE account_id = '%u' AND realm_id = '%u'", account, realmID);
        if (resultAccPerm)
        {
            Field* fieldsAccId = resultAccPerm->Fetch();
            if (fieldsAccId)
                permissions = fieldsAccId[0].GetUInt32();
        }

        if (permissions & PERM_GMT)
            continue;

        uint32 ban_type = mode == BAN_ARENA3V3 ? PUNISHMENT_ARENA3V3 : PUNISHMENT_BAN;

        if (mode != BAN_IP)
        {
            //No SQL injection as strings are escaped
            AccountsDatabase.PExecute("INSERT IGNORE INTO account_punishment VALUES ('%u', '%u', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, '%s', '%s', '1', '0')",
                                    account, ban_type, duration_secs, safe_author.c_str(), reason.c_str());
        }

        if (mode == BAN_ARENA3V3)
        {
            m_arena_3v3_banned[account] = time(NULL) + duration_secs;
            continue;
        }

        if (WorldSession* sess = FindSession(account))
            if (std::string(sess->GetPlayerName()) != author)
                sess->KickPlayer();
    }
    while (resultAccounts->NextRow());

    return BAN_SUCCESS;
}

/// Remove a ban from an account or IP address
bool World::RemoveBanAccount(BanMode mode, std::string nameIPOrMail)
{
    switch (mode)
    {
        case BAN_IP:
            AccountsDatabase.escape_string(nameIPOrMail);
            AccountsDatabase.PExecute("DELETE FROM ip_banned WHERE ip = '%s'",nameIPOrMail.c_str());
            break;
        case BAN_ACCOUNT:
        case BAN_CHARACTER:
        case BAN_ARENA3V3:
            uint32 account = 0;
            if (mode == BAN_ACCOUNT)
                account = AccountMgr::GetId (nameIPOrMail);
            else if (mode == BAN_CHARACTER || mode == BAN_ARENA3V3)
                account = sObjectMgr.GetPlayerAccountIdByPlayerName (nameIPOrMail);

            if (!account)
                return false;

            //NO SQL injection as account is uint32
            uint32 ban_type = mode == BAN_ARENA3V3 ? PUNISHMENT_ARENA3V3 : PUNISHMENT_BAN;

            if (mode == BAN_ARENA3V3)
            {
                m_arena_3v3_banned.erase(account);
            }

            AccountsDatabase.PExecute("UPDATE account_punishment SET active = '0' WHERE account_id = '%u' AND punishment_type_id = '%u'", account, ban_type);
            break;
    }
    return true;
}

/// Update the game time
void World::_UpdateGameTime()
{
    ///- update the time
    time_t thisTime = time(NULL);
    uint32 elapsed = uint32(thisTime - m_gameTime);
    m_gameTime = thisTime;

    ///- if there is a shutdown timer
    if (!m_stopEvent && m_ShutdownTimer > 0 && elapsed > 0)
    {
        ///- ... and it is overdue, stop the world (set m_stopEvent)
        if (m_ShutdownTimer <= elapsed)
        {
            if (!(m_ShutdownMask & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount() == 0)
            {
                sObjectAccessor.SaveAllPlayersWithSleep();

                m_stopEvent = true;                         // exist code already set
            }
            else
                m_ShutdownTimer = 1;                        // minimum timer value to wait idle state
        }
        ///- ... else decrease it and if necessary display a shutdown countdown to the users
        else
        {
            m_ShutdownTimer -= elapsed;

            ShutdownMsg();
        }
    }
}

/// Shutdown the server
void World::ShutdownServ(uint32 time, uint32 options, uint8 exitcode, char const* exitmsg)
{
    // ignore if server shutdown at next tick
    if (m_stopEvent)
        return;

    m_ShutdownMask = options;
    m_ExitCode = exitcode;

    m_ShutdownReason = exitmsg;

    m_ShutdownTimer = time;
    ShutdownMsg(true);
}

/// Display a shutdown message to the user(s)
void World::ShutdownMsg(bool show, Player* player)
{
    // not show messages for idle shutdown mode
    if (m_ShutdownMask & SHUTDOWN_MASK_IDLE)
        return;

    ///- Display a message every 12 hours, hours, 5 minutes, minute, 5 seconds and finally seconds
    if (show ||
        (m_ShutdownTimer < 10) ||
                                                            // < 30 sec; every 5 sec
        (m_ShutdownTimer<30        && (m_ShutdownTimer % 5        )==0) ||
                                                            // < 5 min ; every 1 min
        (m_ShutdownTimer<5*MINUTE  && (m_ShutdownTimer % MINUTE   )==0) ||
                                                            // < 30 min ; every 5 min
        (m_ShutdownTimer<30*MINUTE && (m_ShutdownTimer % (5*MINUTE))==0) ||
                                                            // < 12 h ; every 1 h
        (m_ShutdownTimer<12*HOUR   && (m_ShutdownTimer % HOUR     )==0) ||
                                                            // > 12 h ; every 12 h
        (m_ShutdownTimer>12*HOUR   && (m_ShutdownTimer % (12*HOUR))==0))
    {
        std::string str = WorldSession::secondsToTimeString(player ? player->GetSession() : NULL, m_ShutdownTimer);
        str += " (" + m_ShutdownReason + ")";

		ServerMessageType msgid;
		uint32 stringid;

		if (m_ShutdownMask & SHUTDOWN_MASK_RESTART)
		{
			msgid = SERVER_MSG_RESTART_TIME;
			stringid = 16598;
		}
		else
		{
			msgid = SERVER_MSG_SHUTDOWN_TIME;
			stringid = 16599;
		};

		char chr_ru[255];
		sprintf(chr_ru, sObjectMgr.GetHellgroundString(stringid, 0), str.c_str());

		char chr_en[255];
		sprintf(chr_en, sObjectMgr.GetHellgroundString(stringid, 1), str.c_str());

        SendServerMessage(chr_en, chr_ru, player);

        debug_log("Server is %s in %s",(m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"),str.c_str());
    }
}

/// Cancel a planned server shutdown
void World::ShutdownCancel()
{
    // nothing cancel or too later
    if (!m_ShutdownTimer || m_stopEvent)
        return;

    uint32 stringid = (m_ShutdownMask & SHUTDOWN_MASK_RESTART) ? 16600 : 16601;

    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_ShutdownReason = "no reason set";

    m_ExitCode = SHUTDOWN_EXIT_CODE;                       // to default value

    SendServerMessage(sObjectMgr.GetHellgroundString(stringid, 1), sObjectMgr.GetHellgroundString(stringid, 0));

    debug_log("Server %s cancelled.",(m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"));
}

/// Send a server message to the user(s)
void World::SendServerMessage(const char *text_en, const char *text_ru, Player* player)
{
	//WorldPacket data(SMSG_SERVER_MESSAGE, 50);              // guess size
    //data << uint32(type);
    //if (type <= SERVER_MSG_STRING)
    //    data << text;

    //if (player)
    //    player->SendPacketToSelf(&data);
    //else
    //    SendGlobalMessage(&data);

	// better to do it with direct messages
	bool ru;

	if (player)
	{
		ru = player->GetSession()->isRussian();
		ChatHandler(player).PSendSysMessage(ru ? text_ru : text_en);
		player->GetSession()->SendAreaTriggerMessage(ru ? text_ru : text_en);
	}
	else
	{
		SessionMap::iterator itr;
		for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
		{
			if (itr->second &&
				itr->second->GetPlayer() &&
				itr->second->GetPlayer()->IsInWorld())
			{
				ru = itr->second->isRussian();
				ChatHandler(itr->second->GetPlayer()).PSendSysMessage(ru ? text_ru : text_en);
				itr->second->SendAreaTriggerMessage(ru ? text_ru : text_en);
			}
		}
	}
}

// This handles the issued and queued CLI commands
void World::ProcessCliCommands()
{
    CliCommandHolder::Print* zprint = NULL;

    CliCommandHolder* command;
    while (cliCmdQueue.next(command))
    {
        sLog.outDebug("CLI command under processing...");

        zprint = command->m_print;

        CliHandler(zprint).ParseCommands(command->m_command);

        delete command;
    }

    // print the console message here so it looks right
    if (zprint)
        zprint("TC> ");
}

void World::InitResultQueue()
{

}

void World::UpdateResultQueue()
{
    //process async result queues
    RealmDataDatabase.ProcessResultQueue();
    GameDataDatabase.ProcessResultQueue();
    AccountsDatabase.ProcessResultQueue();
}

void World::UpdateRealmCharCount(uint32 accountId)
{
    QueryResultAutoPtr resultCharCount = RealmDataDatabase.PQuery("SELECT COUNT(guid) FROM characters WHERE account = '%u'", accountId);

    if (!resultCharCount)
        return;

    Field *fields = resultCharCount->Fetch();

    static SqlStatementID updateRealmChars;
    SqlStatement stmt = AccountsDatabase.CreateStatement(updateRealmChars, "UPDATE realm_characters SET characters_count = ? WHERE account_id = ? AND realm_id = ?");
    stmt.addUInt8(fields[0].GetUInt8());
    stmt.addUInt32(accountId);
    stmt.addUInt32(realmID);
    stmt.DirectExecute();
}

void World::InitDailyQuestResetTime()
{
    time_t mostRecentQuestTime;

    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT MAX(time) FROM character_queststatus_daily");
    if (result)
        mostRecentQuestTime = (time_t)(*result)[0].GetUInt64();
    else
        mostRecentQuestTime = 0;

    // client built-in time for reset is 6:00 AM
    // FIX ME: client not show day start time
    time_t curTime = time(NULL);
    tm localTm6AM = *localtime(&curTime);
    localTm6AM.tm_hour = 6;
    localTm6AM.tm_min = 0;
    localTm6AM.tm_sec = 0;

    // current day reset time
    time_t curDayResetTime = mktime(&localTm6AM);

    // last reset time before current moment
    time_t previousResetTime = (curTime < curDayResetTime) ? curDayResetTime - DAY : curDayResetTime;

    // need reset (if we have quest time before last reset time (not processed by some reason)
    if (mostRecentQuestTime && mostRecentQuestTime <= previousResetTime)
        // reset time is 6 AM, for example, if at the moment is 7 AM. mostRecentQuestTime could be 5 AM for example.
        // do reset on next world tick
        m_NextDailyQuestReset = previousResetTime;
    else
        m_NextDailyQuestReset = previousResetTime + DAY;
}

void World::UpdateRequiredPermissions()
{
     QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT required_permission_mask from realms WHERE realm_id = '%u'", realmID);
     if (result)
     {
        m_requiredPermissionMask = result->Fetch()->GetUInt64();
        sLog.outDebug("Required permission mask: %llu", m_requiredPermissionMask);
     }
}

void World::SelectRandomHeroicDungeonDaily()
{
    if (sGameEventMgr.GetEventMap().empty())
        return;

    if (sWorld.getSeason() < SEASON_4)
        return;

    const uint32 HeroicEventStart = 100;
    const uint32 HeroicEventEnd   = sWorld.getSeason() == SEASON_5 ? 115 : 114;

    uint8 currentId = 0;
    for (uint8 eventId = HeroicEventStart; eventId <= HeroicEventEnd; ++eventId)
    {
        if (sGameEventMgr.IsActiveEvent(eventId))
        {
            currentId = eventId;
            sGameEventMgr.StopEvent(eventId, true);
        }
        if (sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5184000 WHERE entry = %u", eventId);
        else
        {
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", eventId);
            sGameEventMgr.StartEvent(eventId, true);
        }
    }

    if (!sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
        return;

    uint8 random = urand(HeroicEventStart, HeroicEventEnd);
    while (random == currentId)
        random = urand(HeroicEventStart, HeroicEventEnd);

    if (currentId)
        sGameEventMgr.GetEventMap()[currentId].occurence = 5184000;
    sGameEventMgr.GetEventMap()[random].occurence = 1400;

    sGameEventMgr.StartEvent(random, true);
    GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", random);
}

void World::SelectRandomDungeonDaily()
{
    if (sGameEventMgr.GetEventMap().empty())
        return;

    const uint32 DungeonEventStart = 116;
    const uint32 DungeonEventEnd   = sWorld.getSeason() == SEASON_5 ? 123 : 122;

    uint8 currentId = 0;
    for (uint8 eventId = DungeonEventStart; eventId <= DungeonEventEnd; ++eventId)
    {
        if (sGameEventMgr.IsActiveEvent(eventId))
        {
            currentId = eventId;
            sGameEventMgr.StopEvent(eventId, true);
        }
        if (sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5184000 WHERE entry = %u", eventId);
        else{
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", eventId);
            sGameEventMgr.StartEvent(eventId, true);
        }
    }

    if (!sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
        return;

    uint8 random = urand(DungeonEventStart, DungeonEventEnd);
    while (random == currentId)
        random = urand(DungeonEventStart, DungeonEventEnd);

    if (currentId)
        sGameEventMgr.GetEventMap()[currentId].occurence = 5184000;
    sGameEventMgr.GetEventMap()[random].occurence = 1400;

    sGameEventMgr.StartEvent(random, true);
    GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", random);
}

//void World::SelectRandomInstanceDaily()
//{
//    if (!sWorld.isEasyRealm())
//        return;
//    
//    if (sGameEventMgr.GetEventMap().empty())
//        return;
//    
//    // 300 Kara
//    // 301 Gruul
//    // 302 TK
//    // 303 SSK
//
//    const uint32 EventStart = 300;
//    const uint32 EventEnd = 303;
//
//    uint32 currentId = 0;
//    for (uint32 eventId = EventStart; eventId <= EventEnd; ++eventId)
//    {
//        if (sGameEventMgr.IsActiveEvent(eventId))
//        {
//            currentId = eventId;
//            sGameEventMgr.StopEvent(eventId, true);
//        }
//        if (sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
//            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5184000 WHERE entry = %u", eventId);
//        else{
//            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", eventId);
//            sGameEventMgr.StartEvent(eventId, true);
//        }
//    }
//
//    uint32 random = urand(EventStart, EventEnd);
//    while (random == currentId)
//        random = urand(EventStart, EventEnd);
//
//    if (currentId)
//        sGameEventMgr.GetEventMap()[currentId].occurence = 5184000;
//    sGameEventMgr.GetEventMap()[random].occurence = 1400;
//
//    sGameEventMgr.StartEvent(random, true);
//    GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", random);
//}

void World::SelectRandomCookingDaily()
{
    if (sGameEventMgr.GetEventMap().empty())
        return;

    const uint32 CookingEventStart = 124;
    const uint32 CookingEventEnd   = 127;

    uint8 currentId = 0;
    for (uint8 eventId = CookingEventStart; eventId <= CookingEventEnd; ++eventId)
    {
        if (sGameEventMgr.IsActiveEvent(eventId))
        {
            currentId = eventId;
            sGameEventMgr.StopEvent(eventId, true);
        }
        if (sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5184000 WHERE entry = %u", eventId);
        else{
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", eventId);
            sGameEventMgr.StartEvent(eventId, true);
        }
    }

    if (!sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
        return;

    uint8 random = urand(CookingEventStart, CookingEventEnd);
    while (random == currentId)
        random = urand(CookingEventStart, CookingEventEnd);

    if (currentId)
        sGameEventMgr.GetEventMap()[currentId].occurence = 5184000;
    sGameEventMgr.GetEventMap()[random].occurence = 1400;

    sGameEventMgr.StartEvent(random, true);
    GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", random);
}

void World::SelectRandomFishingDaily()
{
    if (sGameEventMgr.GetEventMap().empty())
        return;

    const uint32 FishingEventStart = 128;
    const uint32 FishingEventEnd   = 132;

    uint8 currentId = 0;
    for (uint8 eventId = FishingEventStart; eventId <= FishingEventEnd; ++eventId)
    {
        if (sGameEventMgr.IsActiveEvent(eventId))
        {
            currentId = eventId;
            sGameEventMgr.StopEvent(eventId, true);
        }
        if (sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5184000 WHERE entry = %u", eventId);
        else{
            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", eventId);
            sGameEventMgr.StartEvent(eventId, true);
        }
    }

    if (!sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
        return;

    uint8 random = urand(FishingEventStart, FishingEventEnd);
    while (random == currentId)
        random = urand(FishingEventStart, FishingEventEnd);

    if (currentId)
        sGameEventMgr.GetEventMap()[currentId].occurence = 5184000;
    sGameEventMgr.GetEventMap()[random].occurence = 1400;

    sGameEventMgr.StartEvent(random, true);
    GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", random);
}

//void World::SelectRandomDailyPvP()
//{
//    if (sGameEventMgr.GetEventMap().empty())
//        return;
//
//    const uint32 PvPEventStart = 133; // blizz events
//    const uint32 PvPEventEnd = 136;
//
//    uint16 currentId = 0;
//    for (uint16 eventId = PvPEventStart; eventId <= PvPEventEnd; ++eventId)
//    {
//        if (sGameEventMgr.IsActiveEvent(eventId))
//        {
//            currentId = eventId;
//            sGameEventMgr.StopEvent(eventId);
//        }
//        if (sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
//            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5184000 WHERE entry = %u", eventId);
//        else{
//            GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", eventId);
//            sGameEventMgr.StartEvent(eventId, true);
//        }
//    }
//
//    if (!sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
//        return;
//
//    uint16 next = urand(PvPEventStart, PvPEventEnd);
//    while (next == currentId)
//        next = urand(PvPEventStart, PvPEventEnd);
//
//    if (currentId)
//        sGameEventMgr.GetEventMap()[currentId].occurence = 5184000;
//    sGameEventMgr.GetEventMap()[next].occurence = 1400;
//
//    sGameEventMgr.StartEvent(next);
//    GameDataDatabase.PExecute("UPDATE game_event SET occurence = 1400 WHERE entry = %u", next);
//}

void World::SelectNextBGEvent(int hour, uint16 forceEvent)
{
    if (sGameEventMgr.GetEventMap().empty())
        return;

    if (sWorld.getConfig(CONFIG_ALTERAC_ENABLED) && hour == sWorld.getConfig(CONFIG_ALTERAC_EVENING_START_HOUR) && forceEvent != EVENT_BG_ALTERAC)
    {
        sWorld.SendWorldText(15453, 0);

        // end all active BGs
        for (uint32 i = BATTLEGROUND_TYPE_NONE; i < MAX_BATTLEGROUND_TYPE_ID; i++)
        {
            for (BattleGroundSet::iterator itr = sBattleGroundMgr.m_BattleGrounds[i].begin(); itr != sBattleGroundMgr.m_BattleGrounds[i].end(); ++itr)
            {
                auto bg = itr->second;
                if (bg->isBattleGround() && bg->GetTypeID() != BATTLEGROUND_AV)
                {
                    if (bg->GetStatus() == STATUS_IN_PROGRESS || bg->GetStatus() == STATUS_WAIT_JOIN)
                        bg->EndBattleGround(bg->GetWinningTeam());
                }

            }
        }

        SelectNextBGEvent(hour, EVENT_BG_ALTERAC);
        return;
    }

	std::set<uint32> currentIds;

	// on x5, don't disable current BG when AV starts
	//if (sWorld.isEasyRealm() || (!sWorld.isEasyRealm() && forceEvent != EVENT_BG_ALTERAC))
	//{
	//}

    // stop all events
    for (uint32 eventId = EVENT_BG_ARATHI; eventId <= EVENT_BG_ALTERAC; ++eventId)
    {
        if (sGameEventMgr.IsActiveEvent(eventId))
        {
            currentIds.insert(eventId);
            sGameEventMgr.StopEvent(eventId);
        }

        GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5184000 WHERE entry = %u", eventId);
    }

    bool always_wsg = false;

    uint16 next;
    if (forceEvent && forceEvent >= EVENT_BG_ARATHI && forceEvent <= EVENT_BG_ALTERAC)
    {
        next = forceEvent;
    }
    else
    {
        time_t t = time(NULL);
        tm* aTm = localtime(&t);

        // always wsg from 02:00 until 14:00       
        if (aTm->tm_hour > 2 && aTm->tm_hour < 14)
        {
            next = EVENT_BG_WARSONG;
            always_wsg = true;
        }
        else
        {
            //0 1 0 2 0 1 0 2 0 1....
            if (totalBGEvents % 4 == 0)
                next = EVENT_BG_ARATHI;
            else if ((int(totalBGEvents) - 2) % 4 == 0)
                next = EVENT_BG_EYE;
            else
                next = EVENT_BG_WARSONG;
        }
    }

    sLog.outLog(LOG_BG, "BG event started: %u (total: %u, forced %u)", next, totalBGEvents, forceEvent);

	for (auto id : currentIds)
	{
		if (next != id)
			sBattleGroundMgr.SetBGEventEnding(id);

		sGameEventMgr.GetEventMap()[id].occurence = 5184000;
	}

    sGameEventMgr.GetEventMap()[next].occurence = 5000;
    sGameEventMgr.StartEvent(next);

    if (next != EVENT_BG_ALTERAC)
    {
        if (always_wsg)
            totalBGEvents = 0; // next is always aratch 0 % 4 == 0
        else
            ++totalBGEvents;
    }

    GameDataDatabase.PExecute("UPDATE game_event SET occurence = 5000 WHERE entry = %u", next);
    sBattleGroundMgr.last_bgevent_hour = hour;
}

void World::ResetDailyQuests()
{
    sLog.outDetail("Daily quests reset for all characters.");
    RealmDataDatabase.Execute("DELETE FROM character_queststatus_daily");
    //RealmDataDatabase.Execute("UPDATE character_stats_ro SET dailyarenawins = 0");
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetPlayer())
            itr->second->GetPlayer()->ResetDailyQuestStatus();

    SelectRandomHeroicDungeonDaily();
    SelectRandomDungeonDaily();
    SelectRandomCookingDaily();
    SelectRandomFishingDaily();
    //SelectRandomDailyPvP();
    //SelectRandomInstanceDaily();
    //sGameEventMgr.LoadFromDB();
}

void World::SetPlayerLimit(int32 limit)
{
    if(limit >= 0)
    {
        m_playerLimit = limit;
        m_requiredPermissionMask = PERM_PLAYER;
        return;
    }
    
    if(limit == -1)
        m_requiredPermissionMask = PERM_GMT_DEV;
    if(limit == -2)
        m_requiredPermissionMask = PERM_HIGH_GMT | PERM_HEAD_DEVELOPER;
    if(limit == -3)
        m_requiredPermissionMask = PERM_ADM;
}

void World::UpdateMaxSessionCounters()
{
    m_maxActiveSessionCount = std::max(m_maxActiveSessionCount,uint32(m_sessions.size()-m_QueuedPlayer.size()));
    m_maxQueuedSessionCount = std::max(m_maxQueuedSessionCount,uint32(m_QueuedPlayer.size()));
}

void World::CleanupDeletedChars()
{
    int keepDays = getConfig(CONFIG_KEEP_DELETED_CHARS_TIME);

    if (keepDays < 1)
        return;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT char_guid FROM deleted_chars WHERE DATEDIFF(now(), date) >= %u", keepDays);
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            Player::DeleteCharacterInfoFromDB(guid);
        }
        while (result->NextRow());
    }
}

uint32 World::GetLoggedInCharsCount(TeamId team)
{
    switch (team)
    {
        case TEAM_HORDE:
            return loggedInHordes.value();
        case TEAM_ALLIANCE:
            return loggedInAlliances.value();
        default:
            return loggedInAlliances.value() + loggedInHordes.value();
    }
}

uint32 World::ModifyLoggedInCharsCount(TeamId team, int val)
{
    switch (team)
    {
        case TEAM_HORDE:
            return loggedInHordes += val;
        case TEAM_ALLIANCE:
            return loggedInAlliances += val;
        default:
            break;
    }

    return 0;
}

void World::SetLoggedInCharsCount(TeamId team, uint32 val)
{
    switch (team)
    {
        case TEAM_HORDE:
            loggedInHordes = val;
            break;
        case TEAM_ALLIANCE:
            loggedInAlliances = val;
            break;
        default:
            break;
    }
}

void World::loadConfig(WorldConfigs index, const char* name, int32 def)
{
    if (index < CONFIG_VALUE_COUNT)
        m_configs[index] = sConfig.GetIntDefault(name, def);
}

void World::loadConfig(Rates index, const char* name, float def)
{
    if (index < MAX_RATES)
        rate_values[index] = sConfig.GetFloatDefault(name, def);
}

void World::loadConfig(WorldConfigs index, const char* name, bool def)
{
    if (index < CONFIG_VALUE_COUNT)
        m_configs[index] = sConfig.GetBoolDefault(name, def);
}

CBTresholds World::GetCoreBalancerTreshold()
{
    return _coreBalancer.GetTreshold();
}

//bool World::RaidForceDefaultModifiers(uint32 map_id)
//{
//	// used only for x5
//	if (sWorld.isEasyRealm())
//		return false;
//
//	uint32 season;
//	
//	switch (map_id)
//	{
//	case MAP_KARA:
//		season = SEASON_1;
//		break;
//	case MAP_GRUUL:
//		season = SEASON_1;
//		break;
//	case MAP_MAGTH:
//		season = SEASON_1;
//		break;
//	case MAP_TK:
//		season = SEASON_2;
//		break;
//	case MAP_SSK:
//		season = SEASON_2;
//		break;
//	case MAP_ZA:
//		season = SEASON_3;
//		break;
//	case MAP_HS:
//		season = SEASON_4;
//		break;
//	case MAP_BT:
//		season = SEASON_4;
//		break;
//	case MAP_SWP:
//		season = SEASON_5;
//		break;
//	default:
//		return false;
//	}
//
//	return sWorld.getSeason() > season;
//}

bool World::RaidNerfed(uint32 map_id)
{
	if (sWorld.isEasyRealm())
		return true;
	
	switch (map_id)
	{
	case MAP_SSK:
	case MAP_TK:
	{
		if (getSeason() < SEASON_5)
			return false;
	}
	}

	return true;
}

Season World::getSeason()
{
    for (uint16 ev = 200; ev < 205; ++ev)
    {
        if (sGameEventMgr.IsActiveEvent(ev))
            return Season(ev - 200);
    }

    return sWorld.isEasyRealm() ? SEASON_5 : SEASON_1;
}

Season World::getSeasonFromDB()
{
    QueryResultAutoPtr resultSeason = GameDataDatabase.Query("SELECT entry FROM game_event WHERE `occurence`<`length` AND entry IN (200, 201, 202, 203, 204) LIMIT 1");
    if (!resultSeason)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: DB table `game_event` does not have season determination (200-204) events or none of them are active.");
        exit(1);
    }
    else
    {
        Field *fields = resultSeason->Fetch();
        return (Season)(fields[0].GetUInt16()-200);
    }
    
    // Never gets here
    return SEASON_1;
}

//void World::LoadMaxLvlReachedPerClass()
//{
//    QueryResultAutoPtr resultMaxLvlReached = RealmDataDatabase.Query("SELECT DISTINCT `class` from `characters` where `level` = 70");
//    if (resultMaxLvlReached)
//    {
//        uint32 count = 0;
//        do
//        {
//            uint8 maxLvlClass = (*resultMaxLvlReached)[0].GetUInt8();
//
//            if (maxLvlClass >= MAX_CLASSES)
//            {
//                sLog.outLog(LOG_DB_ERR, "ERROR: Max lvl player class out of bound, class number: %u.", maxLvlClass);
//                continue;
//            }
//
//            sWorld.getMaxLvlReachedPerClass()[maxLvlClass] = true;
//            ++count;
//        } while (resultMaxLvlReached->NextRow());
//
//        sLog.outString("%u classess out of 9 have reached max lvl...", count);
//    }
//    else
//        sLog.outString("There are 0 max lvl players or could not load count of them...");
//}

void World::LoadMinorityPropagandaChar()
{
    if (_minorityPropagandaPkt)
    {
        delete _minorityPropagandaPkt;
        _minorityPropagandaPkt = NULL;
    }

    loadConfig(CONFIG_MINORITY_PROPAGANDA_CHAR, "MinorityPropagandaChar", 0);

    uint32 propagandaChar = getConfig(CONFIG_MINORITY_PROPAGANDA_CHAR);
    // just clear loaded packet if needed
    if (!propagandaChar)
    {
        sLog.outString("Propaganda character not set...");
        return;
    }

    /*SQL fully copied from WorldSession::HandleCharEnumOpcode to use with Player::BuildEnumData later*/
    QueryResultAutoPtr result = RealmDataDatabase.PQuery(!sWorld.getConfig(CONFIG_DECLINED_NAMES_USED) ?
        //   ------- Query Without Declined Names --------
        //          0                1                2                3                 4                  5                       6                        7
        "SELECT characters.guid, '%s', characters.race, characters.class, characters.gender, characters.playerBytes, characters.playerBytes2, characters.level, "
        //  8                 9               10                     11                     12                     13                    14
        "characters.zone, characters.map, characters.position_x, characters.position_y, characters.position_z, guild_member.guildid, characters.playerFlags | %u, "
        //  15                    16                   17                     18                   19
        "characters.at_login, character_pet.entry, character_pet.modelid, character_pet.level, characters.data "
        "FROM characters LEFT JOIN character_pet ON characters.guid=character_pet.owner AND character_pet.slot='%u' "
        "LEFT JOIN guild_member ON characters.guid = guild_member.guid "
        "WHERE characters.guid = '%u'"
        :
    //   --------- Query With Declined Names ---------
    //          0                1                2                3                 4                  5                       6                        7
    "SELECT characters.guid, '%s', characters.race, characters.class, characters.gender, characters.playerBytes, characters.playerBytes2, characters.level, "
        //  8                 9               10                     11                     12                     13                    14
        "characters.zone, characters.map, characters.position_x, characters.position_y, characters.position_z, guild_member.guildid, characters.playerFlags | 0x80000000, "
        //  15                    16                   17                     18                   19               20
        "characters.at_login, character_pet.entry, character_pet.modelid, character_pet.level, characters.data, character_declinedname.genitive "
        "FROM characters LEFT JOIN character_pet ON characters.guid = character_pet.owner AND character_pet.slot='%u' "
        "LEFT JOIN character_declinedname ON characters.guid = character_declinedname.guid "
        "LEFT JOIN guild_member ON characters.guid = guild_member.guid "
        "WHERE characters.guid = '%u'",
        sConfig.GetStringDefault("MinorityPropagandaCharNewName", "The Alliance/Horde needs you!").c_str(), PET_SAVE_AS_CURRENT, propagandaChar);

    if (result)
    {
        _minorityPropagandaPkt = new WorldPacket();
        // just call BuildEnumData with this SQL result and we get the ready worldPacket
        Player::BuildEnumData(result, _minorityPropagandaPkt);
        sLog.outString("Propaganda character loaded...");
    }
    else
    {
        sLog.outString("Propaganda character not loaded (not found)...");
    }
}

bool World::AddCharacterMinorityPropaganda(WorldPacket * p_data)
{
    if (_minorityPropagandaPkt)
    {
        p_data->append((char*)_minorityPropagandaPkt->contents(), _minorityPropagandaPkt->size());
        return true;
    }
    return false;
}

bool World::CheckInstanceCount(uint32 accountId, uint32 instanceId, uint32 maxCount, bool notMaxLevel)
{
    AccountInstanceEnterTimesMap::iterator it = _instanceEnterTimes[notMaxLevel].find(accountId);
    if (it == _instanceEnterTimes[notMaxLevel].end())
        return true;
    InstanceEnterTimesMap& enterTimes = it->second;
    InstanceEnterTimesMap::iterator it2 = enterTimes.find(instanceId);
    if (it2 != enterTimes.end())
        return true;
    if (enterTimes.size() < maxCount)
        return true;
    time_t now = time(NULL);
    for (it2 = enterTimes.begin(); it2 != enterTimes.end(); ++it2)
        if (it2->second + 3600 < now)
        {
            enterTimes.erase(it2);
            return true;
        }
    return false;
}

void World::AddInstanceEnterTime(uint32 accountId, uint32 instanceId, time_t enterTime, bool notMaxLevel)
{
    AccountInstanceEnterTimesMap::iterator it = _instanceEnterTimes[notMaxLevel].find(accountId);
    if (it == _instanceEnterTimes[notMaxLevel].end())
    {
        InstanceEnterTimesMap resetTimes;
        resetTimes[instanceId] = enterTime;
        _instanceEnterTimes[notMaxLevel][accountId] = resetTimes;
        return;
    }
    it->second[instanceId] = enterTime;
}

bool World::CreateContinents()
{
    Map *pKingdom = sMapMgr.CreateMap(0, NULL);       // Create Eastern Kingdom
    if (!pKingdom)
        sLog.outLog(LOG_DEFAULT, "ERROR: Couldn't create map with id: 0");

    Map *pKalimdor = sMapMgr.CreateMap(1, NULL);       // Create Kalimdor
    if (!pKalimdor)
        sLog.outLog(LOG_DEFAULT, "ERROR: Couldn't create map with id: 1");

    Map *pOutland = sMapMgr.CreateMap(530, NULL);     // Create Outland
    if (!pOutland)
        sLog.outLog(LOG_DEFAULT, "ERROR: Couldn't create map with id: 530");

    return pKingdom && pKalimdor && pOutland;
}

void World::UnloadAllFakeBots()
{
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->isFakeBot())
            itr->second->FakeBotUnload();
}

uint64 World::GenerateFakeBotCharacterGuid()
{
    // should never iterate twice, but anyway... :D
    while (true)
    {
        uint64 rand_guid = ObjectGuid(HIGHGUID_PLAYER, urand(0x0FFFFFFF, 0xFFFFFFFF));

        if (!sObjectMgr.GetPlayerInWorld(rand_guid))
            return rand_guid;

        sLog.outLog(LOG_CRITICAL, "GenerateFakeBotCharacterGuid() -> wow it did match!");
    }
}

std::string World::GetRandomFakeBotName()
{
    uint32 max = fakebot_names.size();
    uint32 i = 0;

    while (true)
    {
        std::string name = fakebot_names[urand(0, max - 1)];

        if (!sObjectMgr.GetPlayerInWorld(name.c_str()))
            return name;

        ++i;

        if (i >= max)
            return std::string();
    }
}

void World::LoadFakeBotInfo()
{
    fakebot_locations.clear();
    fakebot_names.clear();
    fakebot_levelcount.clear();

    // cleanup after inserting new values
    // delete from fakebot_locations where map in (489,529,13,25,29,30,449,450,451,559,562,527,566);
    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT level, race, zone FROM fakebot_locations");
    if (!result)
    {
        sLog.outString("No bot locations found... die");
        return;
    }

    uint32 cnt = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 level = fields[0].GetUInt32();
        uint8 race = fields[1].GetUInt8();
        uint32 zone = fields[2].GetUInt32();
        fakebot_locations[level].insert(std::make_pair(race, zone));
        ++cnt;
    } while (result->NextRow());
    sLog.outString("Loaded %u bot locations...", cnt);

    // load names
    result = GameDataDatabase.PQuery("SELECT name FROM fakebot_names");
    if (!result)
    {
        sLog.outString("No bot names found... die");
        return;
    }

    cnt = 0;
    do
    {
        Field* fields = result->Fetch();
        std::string name = fields[0].GetString();

        fakebot_names.push_back(name);
        ++cnt;
    } while (result->NextRow());
    sLog.outString("Loaded %u bot names...", cnt);

    // load levelcount
    result = GameDataDatabase.PQuery("SELECT level, count FROM fakebot_levelcount");
    if (!result)
    {
        sLog.outString("No bot levelcount found... die");
        return;
    }

    cnt = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 level = fields[0].GetUInt32();
        uint32 count = fields[1].GetUInt32();

        fakebot_levelcount.insert(std::make_pair(level, count));
        ++cnt;
    } while (result->NextRow());
    sLog.outString("Loaded %u bot levelcount...", cnt);
}

uint32 World::GetAssistKickCooldown(uint32 lowGuid)
{
    uint32 curTime = WorldTimer::getMSTime();
    ChargesMap::iterator itr = m_assistKickCharges.find(lowGuid);
    if (itr == m_assistKickCharges.end())
    {
        m_assistKickCharges[lowGuid].charges = sWorld.getConfig(CONFIG_CHARGES_COUNT_ASSIST_KICK);
        m_assistKickCharges[lowGuid].nextReset = curTime + sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_ASSIST_KICK);
        return 0;
    }

    if (curTime > itr->second.nextReset)
    {
        itr->second.charges = sWorld.getConfig(CONFIG_CHARGES_COUNT_ASSIST_KICK);
        itr->second.nextReset = curTime + sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_ASSIST_KICK);
        return 0;
    }

    if (!itr->second.charges)
        return ((itr->second.nextReset - curTime) / MILLISECONDS) + 1;

    return 0;
}

bool World::HasLootDeleteCharge(uint32 accId)
{
    uint32 curTime = WorldTimer::getMSTime();
    ChargesMap::iterator itr = m_lootDelCommandCharges.find(accId);
    if (itr == m_lootDelCommandCharges.end())
    {
        m_lootDelCommandCharges[accId].charges = sWorld.getConfig(CONFIG_CHARGES_COUNT_GM_LOOT_DEL);
        m_lootDelCommandCharges[accId].nextReset = curTime + sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_GM_LOOT_DEL);
        return true;
    }

    if (curTime > itr->second.nextReset)
    {
        itr->second.charges = sWorld.getConfig(CONFIG_CHARGES_COUNT_GM_LOOT_DEL);
        itr->second.nextReset = curTime + sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_GM_LOOT_DEL);
        return true;
    }

    if (!itr->second.charges)
        return false;

    return true;
}

void World::CleanOldCharges()
{
    {
        uint32 curTime = WorldTimer::getMSTime();
        ChargesMap::iterator itr = m_assistKickCharges.begin();
        ChargesMap::iterator next = itr;
        uint32 minCleanupTime = sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_ASSIST_KICK) * 10;
        for (itr = m_assistKickCharges.begin(); itr != m_assistKickCharges.end(); itr = next)
        {
            next = itr;
            ++next;

            if (curTime > itr->second.nextReset + minCleanupTime)
                m_assistKickCharges.erase(itr);
        }
    }

    {
        uint32 curTime = WorldTimer::getMSTime();
        ChargesMap::iterator itr = m_lootDelCommandCharges.begin();
        ChargesMap::iterator next = itr;
        uint32 minCleanupTime = sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_GM_LOOT_DEL) * 10;
        for (itr = m_lootDelCommandCharges.begin(); itr != m_lootDelCommandCharges.end(); itr = next)
        {
            next = itr;
            ++next;

            if (curTime > itr->second.nextReset + minCleanupTime)
                m_lootDelCommandCharges.erase(itr);
        }
    }
}


CoreBalancer::CoreBalancer() : _diffSum(0), _diffCount(0), _balanceTimer(0)
{
    _treshold = CB_DISABLE_NONE;
}

void CoreBalancer::Initialize()
{
    _balanceTimer.Reset(sWorld.getConfig(CONFIG_COREBALANCER_INTERVAL));
}

void CoreBalancer::Update(const uint32 diff)
{
    _diffSum += diff;
    ++_diffCount;

    if (_balanceTimer.Expired(diff))
    {
        uint32 diffAvg = _diffSum / _diffCount;
        if (diffAvg > sWorld.getConfig(CONFIG_COREBALANCER_PLAYABLE_DIFF))
            IncreaseTreshold();
        else
        {
            if (diffAvg + 20 < sWorld.getConfig(CONFIG_COREBALANCER_PLAYABLE_DIFF))
                DecreaseTreshold();
        }

        _diffSum = diff;
        _diffCount = 1;

        _balanceTimer.Reset(sWorld.getConfig(CONFIG_COREBALANCER_INTERVAL));
    }
}

void CoreBalancer::IncreaseTreshold()
{
    uint32 t = _treshold;
    if (t >= CB_TRESHOLD_MAX)
        return;

    _treshold = CBTresholds(++t);
}

void CoreBalancer::DecreaseTreshold()
{
    uint32 t = _treshold;
    if (t <= CB_DISABLE_NONE)
        return;

    _treshold = CBTresholds(--t);
}

void World::LoadChestAnnounce()
{
    m_chest_announce.clear();

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT `item`,`chest` FROM `chest_announce`");

    if (!result)
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 chest_announce entries: table is empty or non-existent");
        return;
    }

    uint32 count = 0;

    do
    {
        Field *fields = result->Fetch();
        uint32 item = fields[0].GetUInt32();
        uint32 chest = fields[1].GetUInt32();

        m_chest_announce[item] = chest;
        ++count;

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u chest_announce entries", count);
}

void World::UpdateGuildHouse(uint32 guild, GuildHouseDonators ghd, bool sql)
{
    guild_house_info.insert(std::make_pair(guild, ghd));

    bool found = false;
    for (auto& ghr : guild_house_ranks)
    {
        if (ghr.second == guild)
        {
            ghr.first += ghd.amount;
            found = true;
        }
    }

    if (!found)
        guild_house_ranks.push_back(std::make_pair(ghd.amount, guild));

    if (sql)
        RealmDataDatabase.PExecute("INSERT INTO guild_house (`guild`,`name`,`amount`) VALUES ('%u', '%s', '%u')", guild, ghd.char_name.c_str(), ghd.amount);
}

void World::LoadGuildHouse()
{
    uint32 owner = 0;
    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT GuildHouseOwner FROM saved_variables");
    if (result)
    {
        if (Guild *go = sGuildMgr.GetGuildById((*result)[0].GetUInt32()))
            owner = (*result)[0].GetUInt32();
    }

    SetGuildHouseOwner(owner);
    
    result = RealmDataDatabase.Query("SELECT `guild`, `name`, `amount`, `date` from guild_house ORDER BY `guild`,`date`");

    if (!result)
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 guild house info");
        return;
    }

    do
    {
        Field *fields = result->Fetch();
        uint32 guild = fields[0].GetUInt32();
        const char* name = fields[1].GetString();
        uint32 amount = fields[2].GetUInt32();
        std::string date = fields[3].GetString();

        if (Guild *g = sGuildMgr.GetGuildById(guild))
        {
            GuildHouseDonators ghd;
            ghd.char_name = name;
            ghd.amount = amount;
            ghd.date = date;

            UpdateGuildHouse(guild, ghd, false);
        }
    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded guild house info");
}

bool World::timerMessage(uint8 delay) 
{
    time_t t = time(NULL);

    if (t % delay == 0 && t > warningTimer)
    {
        warningTimer = t;
        return true;
    }

    return false;
};

//bool World::CanStartAlterac()
//{
//    // this code can be better :)
//
//    if (!getConfig(CONFIG_ALTERAC_ENABLED))
//        return false;
//
//    time_t t = time(NULL);
//    tm* aTm = localtime(&t);
//
//    if (CanEnableAlterac[0] && aTm->tm_hour == 15)
//    {
//        CanEnableAlterac[0] = false;
//        return true;
//    }
//    else if (CanEnableAlterac[1] && aTm->tm_hour == 21)
//    {
//        CanEnableAlterac[1] = false;
//        return true;
//    }
//
//    return false;
//};

// old PvpAreaid
bool World::isPvPArea(uint32 area)
{ 
	// 3703 Shattrath City
	// 1037 Grim Batol
	return (isEasyRealm() && area == 3703) || area == 1037;
}

void World::SendExternalMails()
{
    if (!sWorld.getConfig(CONFIG_EXTERNAL_MAIL))
        return;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT me.id, me.receiver, me.subject, me.message, me.money, me.item, me.item_count FROM mail_external me, characters c WHERE me.receiver=c.guid AND c.online=1 limit 100");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 id = fields[0].GetUInt32();
            uint32 receiver = fields[1].GetUInt32();
            std::string subject = fields[2].GetString();
            std::string message = fields[3].GetString();
            uint32 money = fields[4].GetUInt32();
            uint32 item_id = fields[5].GetUInt32();
            uint32 item_count = fields[6].GetUInt32();

            Player* player = sObjectMgr.GetPlayerInWorld(receiver);
            if (player)
            {
                MailDraft draft;
                draft.SetSubjectAndBody(subject, message).SetMoney(money);

                if (item_id != 0)
                {
                    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(item_id);
                    if (!proto)
                    {
                        sLog.outLog(LOG_CRITICAL, "ExternalMails: item not exist! id: %u (count: %u), player %s (guid: %u)", item_id, item_count, player->GetName(), player->GetGUIDLow());
                        continue;
                    }

                    uint32 max_stack_size = proto->GetMaxStackSize();

                    Item* item = nullptr;

                    // Unstackable items case (like Chests)
                    if (proto->GetMaxStackSize() == 1 && item_count > 1)
                    {       
                        if (item_count > 12)
                        {
                            sLog.outLog(LOG_CRITICAL, "ExternalMails: can't send more than 12 items! id: %u (count: %u), player %s (guid: %u)", item_id, item_count, player->GetName(), player->GetGUIDLow());
                            continue;
                        }
                        
                        for (uint32 i = 0; i < item_count; ++i) {
                            item = Item::CreateItem(item_id, 1, player);
                            if (!item)
                            {
                                sLog.outLog(LOG_CRITICAL, "ExternalMails: cannot create item! id: %u (count: %u), player %s (guid: %u)", item_id, item_count, player->GetName(), player->GetGUIDLow());
                                continue;
                            }

                            item->SetOwnerGUID(receiver);
                            if (max_stack_size == 1)
                                item->SetBinding(true);
                            item->SaveToDB();

                            draft.AddItem(item);
                        }
                    }
                    // Stackable items case
                    else
                    {
                        int32 count = item_count;

                        uint32 i = 1;
                        while (count > 0)
                        {
                            if (i > 16)
                            {
                                sLog.outLog(LOG_MAIL_EXTERNAL, "ExternalMails: too much items! id: %u (count: %u), player %s (guid: %u)", item_id, item_count, player->GetName(), player->GetGUIDLow());
                                break;
                            }
                            
                            uint32 current_count = count > max_stack_size ? max_stack_size : count;
                            
                            item = Item::CreateItem(item_id, current_count, player);
                            if (!item)
                            {
                                sLog.outLog(LOG_MAIL_EXTERNAL, "ExternalMails: cannot create item! id: %u (current_count: %u), player %s (guid: %u)", item_id, current_count, player->GetName(), player->GetGUIDLow());
                                continue;
                            }

                            item->SetOwnerGUID(receiver);
                            if (max_stack_size == 1)
                                item->SetBinding(true);
                            item->SaveToDB();

                            draft.AddItem(item);

                            count -= current_count;
                            ++i;
                        }
                    }
                }

                draft.SendMailTo(player, MailSender(player, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_RETURNED);
                RealmDataDatabase.PExecute("DELETE FROM mail_external WHERE id=%u", id);

                sLog.outLog(LOG_MAIL_EXTERNAL, "Player: %s (guid: %u), Item: %u (count: %u), Money: %u, Subject: %s", player->GetName(), player->GetGUIDLow(), item_id, item_count, money, subject.c_str());
            }
        } while (result->NextRow());
    }
}

void World::LoadArena3v3Banned()
{
    m_arena_3v3_banned.clear();

    QueryResultAutoPtr result = AccountsDatabase.Query("SELECT account_id, expiration_date from account_punishment where active = 1 and punishment_type_id = 4");

    if (!result)
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 arena_3v3_banned entries: table is empty or non-existent");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 guid = fields[0].GetUInt32();
        time_t expires = fields[1].GetUInt32();

        m_arena_3v3_banned[guid] = expires;
        ++count;

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u arena_3v3_banned entries", count);
}

void World::LoadLexicsCutter()
{
    // lexics cutter
    Lexics = NULL;

    // lexics cutter
    std::string fn_analogsfile = GetDataPath() + "letter_analogs.txt";
    std::string fn_wordsfile = GetDataPath() + "innormative_words.txt";

    // initialize lexics cutter
    Lexics = new LexicsCutter;
    if (!Lexics)
        return;

    if (!Lexics->ReadLetterAnalogs(fn_analogsfile) || !Lexics->ReadInnormativeWords(fn_wordsfile))
    {
        sLog.outString("Cannot initialize LexicsCutter because files is missing!");
        return;
    }

    Lexics->MapInnormativeWords();

    // read additional parameters
    Lexics->IgnoreLetterRepeat = true;
    Lexics->IgnoreMiddleSpaces = true;
}

std::vector<CustomVendor> const* World::GetCustomVendorItems(uint32 entry) const
{
    auto iter = custom_vendor.find(entry);
    if (iter == custom_vendor.end())
        return NULL;

    return &iter->second;
}

void World::LoadCreatureMapMod()
{
    creature_map_mod.clear();

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT map,heroic,damage,hp,eot_count,plus_drop_pre,plus_drop_final,legendary_weapons FROM `creature_map_mod`");

    if (!result)
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 creature_map_mod entries: table is empty or non-existent");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        int32 map = fields[0].GetInt32();
        bool heroic = fields[1].GetBool();
        float damage = fields[2].GetFloat();
        float hp = fields[3].GetFloat();
        uint8 eot_count = fields[4].GetUInt8();
        uint8 plus_drop_pre = fields[5].GetUInt8();
        uint8 plus_drop_final = fields[6].GetUInt8();
        std::string legendary_weapons = fields[7].GetString();

        ModDamageHP mod;
        mod.hp = hp;
        mod.damage = damage;

        AdditionalInfo info;
		info.mod = mod;
        info.eot_count = heroic ? eot_count : 0;
        info.plus_drop_pre = plus_drop_pre;
        info.plus_drop_final = plus_drop_final;
        info.legendary_weapons = legendary_weapons;

        if (heroic)
            map = -map;

        creature_map_mod.insert(std::pair<int32, AdditionalInfo>(map, info));
        ++count;

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u creature_map_mod entries", count);
}

void World::LoadCustomVendor()
{
    custom_vendor.clear();

    QueryResultAutoPtr result = GameDataDatabase.Query("SELECT * FROM `custom_vendor` order by need_count1 desc,need_count2 desc");

    if (!result)
    {
        sLog.outString();
        sLog.outString(">> Loaded 0 custom_vendor entries: table is empty or non-existent");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        uint32 item = fields[1].GetUInt32();
        uint32 need_item1 = fields[2].GetUInt32();
        uint32 need_count1 = fields[3].GetUInt32();
        uint32 need_item2 = fields[4].GetUInt32();
        uint32 need_count2 = fields[5].GetUInt32();

        if (need_count2 > 10000 || need_item1 == 0 || need_count1 == 0) // need_count2 > 1 gold, we suppose cost1 is always gold
        {
            sLog.outLog(LOG_CRITICAL, "Custom_vendor item %u has problems", item);
            continue;
        }

        CustomVendor cv;
        cv.item = item;
        cv.need_item1 = need_item1;
        cv.need_count1 = need_count1;
        cv.need_item2 = need_item2;
        cv.need_count2 = need_count2;

        std::vector<CustomVendor>& cv_vector = custom_vendor[entry];
        cv_vector.push_back(cv);

        ++count;

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u custom_vendor entries", count);
}

ModDamageHP World::GetModDamageHP(uint32 mapid, bool heroic_mode)
{
    int32 map_id = heroic_mode && sWorld.isEasyRealm() ? -static_cast<int32>(mapid) : mapid;

    auto it = sWorld.creature_map_mod.find(map_id);
    if (it != sWorld.creature_map_mod.end())
    {
        ModDamageHP mod;
        mod.damage = it->second.mod.damage;
        mod.hp = it->second.mod.hp;
        return mod;
    }

    return ModDamageHP();
}


void World::CriticalChecks()
{
    if (!isEasyRealm())
        return;

    // boss souls check
    bool is_boss_soul = false;
    for (const auto& bst : boss_souls_template) {
        if (!sObjectMgr.GetItemPrototype(bst.item_soul))
        {
            sLog.outLog(LOG_CRITICAL, "CRITICAL! Item %u doesn't exist", bst.item_soul);
            return;
        }
    }
}

void World::AddFakeBots(uint32 count)
{
    if (count == 0)
    {
        sWorld.UnloadAllFakeBots();
        return;
    }

    uint32 added = 0;
    while (count > 0)
    {
        uint64 guid = GenerateFakeBotCharacterGuid();
        WorldSession::LoginFakeBot(guid);
        --count;
        ++added;
        ++fakebot_load_queue;
    }

    sLog.DEBUG("AddFakeBots - %u bots loaded", added);
}

//void World::ArenaTeamLosesRowAdd(uint32 winner, uint32 loser)
//{
//    m_ArenaLoses[loser][winner]++;
//
//    auto it = m_ArenaLoses[winner].find(loser);
//    if (it != m_ArenaLoses[winner].end())
//        m_ArenaLoses[winner].erase(it);
//}
//
//bool World::ArenaTeamLosesRowSkip(uint32 team1, uint32 team2)
//{
//    for (const auto& team : { std::make_pair(team1, team2), std::make_pair(team2, team1) }) {
//        auto it = m_ArenaLoses[team.first].find(team.second);
//
//        // seems unfair, so never face teams again
//        if (it != m_ArenaLoses[team.first].end() && it->second >= 2)
//            return true;
//    }
//
//    return false;
//}

// @loadother
void World::LoadOtherStuff()
{
    // other
    encounter_ressurect_pos.clear();

    encounter_ressurect_pos[GBK_GRUUL] = { 126.997177f, 360.827148f, 6.194636f }; // Gruul
    encounter_ressurect_pos[GBK_HIGH_KING_MAULGAR] = { 69.434120f, 118.224892f, -6.661665f }; // High King Maulgar

    encounter_ressurect_pos[GBK_ONYXIA] = { -140.579895f, -213.908600f, -69.117973f }; // Onyxia

    encounter_ressurect_pos[GBK_RAGE_WINTERCHILL] = { 5032.214f, -1783.016f, 1321.68f }; // Rage Winterchill
    encounter_ressurect_pos[GBK_ANETHERON] = { 5032.214f, -1783.016f, 1321.68f }; // Anetheron
    encounter_ressurect_pos[GBK_KAZROGAL] = { 5484.005f, -2715.907f, 1482.417f }; // Kaz'rogal
    encounter_ressurect_pos[GBK_AZGALOR] = { 5484.005f, -2715.907f, 1482.417f }; // Azgalor
    encounter_ressurect_pos[GBK_ARCHIMONDE] = { 5334.793945f, -3506.185303f, 1572.067627f }; // Archimonde

    encounter_ressurect_pos[GBK_MAGTHERIDON] = { -69.808922f, 50.681293f, -0.380804f }; // Magtheridon

    encounter_ressurect_pos[GBK_VOID_REAVER] = { 492.903f, 267.152f, 20.153f }; // Void Reaver
    encounter_ressurect_pos[GBK_ALAR] = { 240.625f, -0.8177f, -2.42f }; // Al'ar
    encounter_ressurect_pos[GBK_HIGH_ASTROMANCER_SOLARIAN] = { 492.991f, -268.473f, 20.153f }; // High Astromancer Solarian
    encounter_ressurect_pos[GBK_KAELTHAS_SUNSTRIDER] = { 643.308533f, -0.727690f, 46.778000f }; // Kael'thas Sunstrider

    encounter_ressurect_pos[GBK_ZA_AKILZON] = { 308.166412f, 1387.108398f, 57.138882f }; // Akil'zon (Eagle)
    encounter_ressurect_pos[GBK_ZA_NALORAK] = { -80.116592f, 1402.202393f, 27.205109f }; // Nalorakk (Bear)
    encounter_ressurect_pos[GBK_ZA_JANALAI] = { -83.945557f, 1150.523193f, 5.594103f }; // Jan'alai (Dragonhawk)
    encounter_ressurect_pos[GBK_ZA_HALAZZI] = { 370.088165f, 1046.042969f, 9.520288f }; // Halazzi (Lynx)
    encounter_ressurect_pos[GBK_ZA_HEXLORD] = { 117.971764f, 1013.406067f, 33.888145f }; // Hex Lord Malacrass
    encounter_ressurect_pos[GBK_ZA_ZULJIN] = { 119.501068f, 802.356812f, 33.377167f }; // Zul'jin

    encounter_ressurect_pos[GBK_HYDROSS_THE_UNSTABLE] = { -193.157501f, -229.835022f, -5.407284f }; // Hydross the Unstable
    encounter_ressurect_pos[GBK_LURKER_BELOW] = { 4.155289f, -470.298462f, -19.793459f }; // The Lurker Below
    encounter_ressurect_pos[GBK_LEOTHERAS_THE_BLIND] = { 384.674683f, -327.208801f, 20.146763f }; // Leotheras the Blind
    encounter_ressurect_pos[GBK_FATHOMLORD_KARATHRESS] = { 499.653107f, -453.251740f, -13.158179f }; // Fathom-Lord Karathress
    encounter_ressurect_pos[GBK_MOROGRIM_TIDEWALKER] = { 466.827606f, -724.003784f, -7.145666f }; // Morogrim Tidewalker
    encounter_ressurect_pos[GBK_LADY_VASHJ] = { 40.573616f, -789.799377f, 22.140081f }; // Lady Vashj

    encounter_ressurect_pos[GBK_KALECGOS] = { 1706.859863f, 1067.945801f, 52.859608f }; // Kalecgos
    encounter_ressurect_pos[GBK_BRUTALLUS] = { 1534.357300f, 544.296570f, 32.050411f }; // Brutallus
    encounter_ressurect_pos[GBK_FELMYST] = { 1534.357300f, 544.296570f, 32.050411f }; // Felmyst
    encounter_ressurect_pos[GBK_HOT_EREDAR_CHICKS] = { 1770.951172f, 540.642639f, 62.093216f }; // Lady Sacrolash
    encounter_ressurect_pos[GBK_MURU] = { 1907.299561f, 550.258118f, 71.300102f }; // Mu'ru
    encounter_ressurect_pos[GBK_KILJAEDEN] = { 1622.844727f, 653.591064f, 37.971733f }; // Kil'jaeden

    encounter_ressurect_pos[GBK_HIGH_WARLORD_NAJENTUS] = { 395.816650f, 819.193665f, 13.182721f }; // High Warlord Naj'entus
    encounter_ressurect_pos[GBK_SUPREMUS] = { 705.250000f, 790.904175f, 63.411430f }; // Supremus
    encounter_ressurect_pos[GBK_SHADE_OF_AKAMA] = { 556.948975f, 400.597137f, 112.783722f }; // Shade of Akama
    encounter_ressurect_pos[GBK_TERON_GOREFIEND] = { 547.020203f, 402.575043f, 193.201218f }; // Teron Gorefiend
    encounter_ressurect_pos[GBK_GURTOG_BLOODBOIL] = { 842.943665f, 276.752991f, 76.620300f }; // Gurtogg Bloodboil
    encounter_ressurect_pos[GBK_REQUILARY_OF_SOULS] = { 496.241180f, 53.135502f, 112.681595f }; // Reliquary of the Lost (no loot)
    encounter_ressurect_pos[GBK_MOTHER_SHARAZ] = { 945.787720f, 244.744751f, 191.208069f }; // Mother Shahraz
    encounter_ressurect_pos[GBK_ILLIDARI_COUNCIL] = { 584.193665f, 305.654236f, 272.120087f }; // Council
    encounter_ressurect_pos[GBK_ILLIDAN_STORMRAGE] = { 745.685120f, 249.245636f, 352.996429f }; // Illidan Stormrage

    encounter_ressurect_pos[GBK_KARA_ATTUNEMENT] = { -11084.754883f, -1954.507568f, 49.726944f }; // Kara
    encounter_ressurect_pos[GBK_KARA_MOROES] = { -11020.126953f, -1936.220581f, 78.867661f }; // Kara
    encounter_ressurect_pos[GBK_KARA_OPERA] = { -10864.916992f, -1774.789551f, 90.466827f }; // Kara
    encounter_ressurect_pos[GBK_KARA_MAIDEN] = { -10887.545898f, -2014.140869f, 92.173409f }; // Kara
    encounter_ressurect_pos[GBK_KARA_CURATOR] = { -11023.489258f, -1855.625854f, 165.765549f }; // Kara
    encounter_ressurect_pos[GBK_KARA_ARAN] = { -11241.839844f, -1815.734497f, 223.944061f }; // Kara
    encounter_ressurect_pos[GBK_KARA_TERESTIAN] = { -11140.956055f, -1745.496094f, 201.155487f }; // Kara
    encounter_ressurect_pos[GBK_KARA_NETHERSPITE] = { -11217.606445f, -1690.167725f, 290.348206f }; // Kara
    encounter_ressurect_pos[GBK_KARA_MALCHEZAR] = { -11006.440430f, -1981.083252f, 274.977020f }; // Kara
    encounter_ressurect_pos[GBK_KARA_NIGHTBANE] = { -11107.615234f, -1966.454346f, 91.440254f }; // Kara

    encounter_ressurect_pos[GBK_THADDIUS] = { 3410.283691f, -3029.052002f, 296.568054f }; // Naxx
    encounter_ressurect_pos[GBK_GROBBULUS] = { 3318.648193f, -3268.958984f, 292.595459f }; // Naxx
    encounter_ressurect_pos[GBK_GLUTH] = { 3239.069824f, -3171.934326f, 297.804077f }; // Naxx
    encounter_ressurect_pos[GBK_HEIGAN_THE_UNCLEAN] = { 2844.562012f, -3682.638184f, 277.421204f }; // Naxx
    encounter_ressurect_pos[GBK_MAEXXNA] = { 3427.977295f, -3846.381348f, 308.225494f }; // Naxx
    encounter_ressurect_pos[GBK_GRAND_WIDOW_FAERLINA] = { 3314.394043f, -3711.715576f, 265.777924f }; // Naxx
    encounter_ressurect_pos[GBK_NOTH_THE_PLAGUEBRINGER] = { 2778.564453f, -3489.783691f, 274.044037f }; // Naxx
    encounter_ressurect_pos[GBK_ANUB_REKHAN] = { 3209.689697f, -3476.269043f, 287.065002f }; // Naxx
    encounter_ressurect_pos[GBK_LOATHEB] = { 2909.525879f, -3882.385986f, 272.894745f }; // Naxx
    encounter_ressurect_pos[GBK_PATCHWERK] = { 3076.002441f, -3169.671143f, 294.062897f }; // Naxx
    encounter_ressurect_pos[GBK_GOTHIK_THE_HARVESTER] = { 2736.500244f, -3423.298096f, 267.686432f }; // Naxx
    encounter_ressurect_pos[GBK_INSTRUCTOR_RAZUVIOUS] = { 2836.620361f, -3147.920654f, 273.787445f }; // Naxx
    encounter_ressurect_pos[GBK_FOUR_HORSEMEN] = { 2632.196533f, -3061.866211f, 240.526718f }; // Naxx
    encounter_ressurect_pos[GBK_SAPPHIRON] = { 3504.857422f, -5322.881348f, 139.913712f }; // Naxx
    encounter_ressurect_pos[GBK_KEL_THUZAD] = { 3647.873047f, -5093.005859f, 143.298035f }; // Naxx


    // bg event
    time_t t = time(NULL);
    tm* tm = localtime(&t);

    bool exist = false;
    for (uint32 eventId = EVENT_BG_ARATHI; eventId <= EVENT_BG_ALTERAC; ++eventId)
    {
        if (sGameEventMgr.IsActiveEvent(eventId))
        {
            sBattleGroundMgr.last_bgevent_hour = tm->tm_hour;

            if (eventId == EVENT_BG_WARSONG)
                totalBGEvents = 4; // arathi next

            exist = true;
            break;
        }
    }

    if (!exist)
    {
        SelectNextBGEvent(tm->tm_hour, EVENT_BG_WARSONG);
        sLog.outLog(LOG_CRITICAL, "BG Event not exist! So start warsong...");
    }

    //multivendor
    // npc_entry, {vendor_id, string_id}

    // Arena Gear Vendor
    multivendors.insert({ 693132, {
        {690900, 16684}, // A4
        {690901, 16685}, // A3
        {690902, 16686}, // A2
        {690903, 16687}, // A1
        {690904, 16688}  // PvP trinkets
    } });

    // Starting PvE Gear Vendor
    multivendors.insert({ 693133, {
        {690915, 16697}, // Weapon
        {690916, 16696}, // Jewelry
        {690917, 16692}, // Cloth Armor
        {690918, 16693}, // Leather Armor
        {690919, 16694}, // Mail Armor
        {690920, 16695}  // Plate Armor
    } });

    // some defines
    raidZones = { 3457, 3836, 3923, 3607, 3845, 3805, 3606, 3959, 4075 };
    heroicDungeonZones = { 3792,3714,3713,3562,3715,3716,3717,3848,3847,3849,3789,3791,3790,267,4131,2366 };

    if (sWorld.isEasyRealm())
    {
        {
            QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT entry,spellid_1 FROM item_template WHERE name like 'Morphing Shirt %%' and spellid_1 > 0 and quality < 6 order by quality desc");
            if (result)
            {
                std::vector<uint32> groups1;
                std::vector<uint32> groups2;
                
                do
                {
                    groups1.push_back((*result)[0].GetUInt32());
                    groups2.push_back((*result)[1].GetUInt32());
                } while (result->NextRow());

                entryGroups.insert({ ENTRY_GROUP_MORPH_SHIRT, groups1 });
                entryGroups.insert({ ENTRY_GROUP_MORPH_SHIRT_AURAS, groups2 });
            }
        }

        {
            QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT entry,spellid_1 FROM item_template WHERE name like 'Aura Tabard %%' and spellid_1 > 0 order by quality desc");
            if (result)
            {
                std::vector<uint32> groups1;
                std::vector<uint32> groups2;
                
                do
                {
                    groups1.push_back((*result)[0].GetUInt32());
                    groups2.push_back((*result)[1].GetUInt32());
                } while (result->NextRow());

                entryGroups.insert({ ENTRY_GROUP_AURA_TABARD, groups1 });
                entryGroups.insert({ ENTRY_GROUP_AURA_TABARD_AURAS, groups2 });
            }
        }
    }
}

void World::RemoveQuestFromEveryone(uint32 entry)
{
    RealmDataDatabase.PExecute("DELETE FROM character_queststatus WHERE quest = %u", entry);
    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::iterator pitr = m.begin(); pitr != m.end(); ++pitr)
        if (pitr->second->GetQuestStatus(entry) != QUEST_STATUS_NONE)
        {
            // remove all quest entries for 'entry' from quest log
            for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            {
                uint32 quest = pitr->second->GetQuestSlotQuestId(slot);
                if (quest == entry)
                {
                    pitr->second->SetQuestSlot(slot, 0);

                    // we ignore unequippable quest items in this case, its' still be equipped
                    pitr->second->TakeQuestSourceItem(quest, false);
                }
            }

            // set quest status to not started (will updated in DB at next save)
            pitr->second->SetQuestStatus(entry, QUEST_STATUS_NONE);

            // reset rewarded for restart repeatable quest
            pitr->second->getQuestStatusMap()[entry].m_rewarded = false;
        }   
}