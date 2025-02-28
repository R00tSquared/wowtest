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

#include "GuildMgr.h"

#include "Database/DatabaseEnv.h"
#include "Database/SQLStorage.h"
#include "Database/SQLStorageImpl.h"

#include "Common.h"
#include "SharedDefines.h"
#include "Guild.h"
//#include "ProgressBar.h"
#include "World.h"
#include "Log.h"

GuildMgr::GuildMgr()
{
    m_guildId = 1;

    m_guildBankTabPrices.resize( GUILD_BANK_MAX_TABS );
    m_guildBankTabPrices[0] = 100;
    m_guildBankTabPrices[1] = 250;
    m_guildBankTabPrices[2] = 500;
    m_guildBankTabPrices[3] = 1000;
    m_guildBankTabPrices[4] = 2500;
    m_guildBankTabPrices[5] = 5000;
}

GuildMgr::~GuildMgr()
{
    for (GuildMap::iterator itr = m_guildsMap.begin(); itr != m_guildsMap.end(); ++itr)
        delete itr->second;
    
    m_guildsMap.clear();
}

GuildMgr* GuildMgr::getGuildMgrFromScripts()
{
    return &sGuildMgr;
}

Guild * GuildMgr::GetGuildById(const uint32 & GuildId) const
{
    GuildMap::const_iterator itr = m_guildsMap.find(GuildId);
    if (itr != m_guildsMap.end())
        return itr->second;

    return nullptr;
}

Guild * GuildMgr::GetGuildByName(const std::string& guildname) const
{
    std::string search = guildname;
    std::transform(search.begin(), search.end(), search.begin(), toupper);
    for (GuildMap::const_iterator itr = m_guildsMap.begin(); itr != m_guildsMap.end(); ++itr)
    {
        std::string gname = itr->second->GetName();
        std::transform(gname.begin(), gname.end(), gname.begin(), toupper);
        if (search == gname)
            return itr->second;
    }
    return nullptr;
}

std::string GuildMgr::GetGuildNameById(const uint32 & GuildId) const
{
    GuildMap::const_iterator itr = m_guildsMap.find(GuildId);
    if (itr != m_guildsMap.end())
        return itr->second->GetName();

    return "";
}

Guild* GuildMgr::GetGuildByLeader(const uint64 &guid) const
{
    for (GuildMap::const_iterator itr = m_guildsMap.begin(); itr != m_guildsMap.end(); ++itr)
        if (itr->second->GetLeader() == guid)
            return itr->second;

    return nullptr;
}

void GuildMgr::AddGuild(Guild* guild)
{
    m_guildsMap[guild->GetId()] = guild;
}

void GuildMgr::RemoveGuild(const uint32 & Id)
{
    m_guildsMap.erase(Id);
}

uint32 GuildMgr::GetGuildBankTabPrice( const uint8 & Index ) const
{
    return Index < GUILD_BANK_MAX_TABS ? m_guildBankTabPrices[Index] : 0;
}

void GuildMgr::LoadGuilds()
{
    Guild *newguild;
    uint32 count = 0;

    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT guildid FROM guild");

    if (!result)
    {
        //BarGoLink bar(1);

        //bar.step();

        sLog.outString();
        sLog.outString(">> Loaded %u guild definitions", count);
    }
    else
    {
        //BarGoLink bar(result->GetRowCount());

        do
        {
            Field *fields = result->Fetch();

            //bar.step();
            ++count;

            newguild = new Guild;
            if (!newguild->LoadGuildFromDB(fields[0].GetUInt32()))
            {
                newguild->Disband();
                delete newguild;
                continue;
            }
            AddGuild(newguild);

        } while (result->NextRow());

        result = RealmDataDatabase.Query("SELECT MAX(guildid) FROM guild");
        if (result)
            m_guildId = (*result)[0].GetUInt32() + 1;
    }

    sLog.outString();
    sLog.outString(">> Loaded %u guild definitions, next guild ID: %u", count, m_guildId);

    result = RealmDataDatabase.Query("SELECT max(kill_id) FROM boss_fights");
    if (result)
        m_bosskill = (*result)[0].GetUInt32() + 1;
    else
        m_bosskill = 0;

    // first we must load boss names, cause there might be no records at all at the moment
    result = RealmDataDatabase.Query("SELECT boss_id, boss_name, boss_points FROM boss_id_names");

    // clear everything
    m_bossrecords.clear();

    if (result)
    {
        m_bossrecords.resize(result->GetRowCount()+1); // there is no boss with id 0, so we add 0 as "+1" to vector size
        for (uint32 i = 0; i < m_bossrecords.size(); ++i)
        {
            m_bossrecords[i].name = ""; // not set
            m_bossrecords[i].record = 0;
            m_bossrecords[i].points = 0;
        }

        // set names which do exist
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if (entry < m_bossrecords.size())
            {
                m_bossrecords[entry].name = fields[1].GetCppString();
                m_bossrecords[entry].points = fields[2].GetUInt32();
            }
            else
                sLog.outLog(LOG_CRITICAL, "The table `boss_id_names` ids should be ascending one by one. Entry %u not loaded", entry);
        } while (result->NextRow());
    }
    else
        sLog.outLog(LOG_CRITICAL, "The table `boss_id_names` is empty or corrupt.");

    result = RealmDataDatabase.Query("SELECT boss_id, min(length) FROM boss_fights GROUP BY boss_id");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if (entry < m_bossrecords.size())
                m_bossrecords[entry].record = fields[1].GetUInt32();
            else
                sLog.outLog(LOG_CRITICAL, "The table `boss_fights` contains boss_id greater than we have info on. boss_id %u not loaded", entry);
        } while (result->NextRow());
    }
}

void GuildMgr::UpdateWeek()
{
    RealmDataDatabase.Execute("UPDATE guild SET LastPoints = (LastPoints + CurrentPoints)/2");
    RealmDataDatabase.Execute("UPDATE guild SET CurrentPoints = 0");
    RealmDataDatabase.Execute("UPDATE guild SET LastPoints = 0 WHERE LastPoints < 20");
}

uint32 GuildMgr::BossKilled(uint32 boss, uint32 guildid, uint32 mstime)
{
    //if (!guildid || boss >= m_bossrecords.size())
    //    return m_bosskill++;

    //bossrecord& br = m_bossrecords[boss];
    //if (br.name != "" && mstime < br.record)
    //{
    //    std::string message = "New server record: " + WorldSession::msToTimeString(NULL, mstime) + " (last record: "
    //        + WorldSession::msToTimeString(NULL, br.record) + ") for boss " + br.name
    //        + " by guild <|cffffffff" + GetGuildNameById(guildid) + "|r>";

    //    sLog.outLog(LOG_SERVER_RECORDS, "%s", message.c_str());
    //    br.record = mstime;
    //}

    //RealmDataDatabase.PExecute("UPDATE guild SET CurrentPoints = CurrentPoints + %u WHERE guildid = %u", br.points, guildid);

    return m_bosskill++;
}

std::string GuildMgr::GBK_GetBossNameForEvent(uint32 enc)
{
    if (enc >= m_bossrecords.size())
        return "";

    return m_bossrecords[enc].name;
}


uint32 GuildMgr::GenerateGuildId()
{
    if (m_guildId >= 0xFFFFFFFE)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Guild ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return m_guildId++;
}

void GuildMgr::SaveGuildAnnCooldown(uint32 guild_id)
{
    time_t tmpTime = time_t(time(NULL) + sWorld.getConfig(CONFIG_GUILD_ANN_COOLDOWN));
    m_guildCooldownTimes[guild_id] = tmpTime;
}
