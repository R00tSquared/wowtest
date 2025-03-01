// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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
    \ingroup realmd
*/

#include "Common.h"
#include "RealmList.h"
#include "AuthCodes.h"
#include "Util.h"                                           // for Tokens typedef
#include "Database/DatabaseEnv.h"
#include "Config/Config.h"

extern DatabaseType AccountsDatabase;

// will only support WoW 1.12.1/1.12.2 , WoW:TBC 2.4.3 and official release for WoW:WotLK and later, client builds 10505, 8606, 6005, 5875
// if you need more from old build then add it in cases in realmd sources code
// list sorted from high to low build and first build used as low bound for accepted by default range (any > it will accepted by realmd at least)

static RealmBuildInfo ExpectedRealmdClientBuilds[] = {
    {12340, 3, 3, 5, 'a'},                                  // highest supported build, also auto accept all above for simplify future supported builds testing
    {11723, 3, 3, 3, 'a'},
    {11403, 3, 3, 2, ' '},
    {11159, 3, 3, 0, 'a'},
    {10505, 3, 2, 2, 'a'},
    {8606,  2, 4, 3, ' '},
    {6005,  1,12, 2, ' '},
    {5875,  1,12, 1, ' '},
    {0,     0, 0, 0, ' '}                                   // terminator
};

RealmBuildInfo const* FindBuildInfo(uint16 _build)
{
    // first build is low bound of always accepted range
    if (_build >= ExpectedRealmdClientBuilds[0].build)
        return &ExpectedRealmdClientBuilds[0];

    // continue from 1 with explicit equal check
    for(int i = 1; ExpectedRealmdClientBuilds[i].build; ++i)
        if(_build == ExpectedRealmdClientBuilds[i].build)
            return &ExpectedRealmdClientBuilds[i];

    // none appropriate build
    return NULL;
}

RealmList::RealmList( ) : m_UpdateInterval(0), m_NextUpdateTime(time(NULL))
{
}

RealmList& RealmList::Instance()
{
    static RealmList realmlist;
    return realmlist;
}

void RealmList::UpdateRealm(uint32 ID, const std::string& name, const std::string& address, uint32 port, uint8 icon, RealmFlags realmflags, uint8 timezone, uint64 requiredPermissionMask, float popu, const std::string& builds)
{
    ///- Create new if not exist or update existed
    Realm& realm = m_realms[name];

    realm.m_ID = ID;
    realm.icon = icon;
    realm.realmflags = realmflags;
    realm.timezone = timezone;
    realm.requiredPermissionMask = requiredPermissionMask;
    realm.populationLevel = popu;

    Tokens tokens = StrSplit(builds, " ");
    Tokens::iterator iter;

    for (iter = tokens.begin(); iter != tokens.end(); ++iter)
    {
        uint32 build = atol((*iter).c_str());
        realm.realmbuilds.insert(build);
    }

    uint16 first_build = !realm.realmbuilds.empty() ? *realm.realmbuilds.begin() : 0;

    realm.realmBuildInfo.build = first_build;
    realm.realmBuildInfo.major_version = 0;
    realm.realmBuildInfo.minor_version = 0;
    realm.realmBuildInfo.bugfix_version = 0;
    realm.realmBuildInfo.hotfix_version = ' ';

    if (first_build)
        if (RealmBuildInfo const* bInfo = FindBuildInfo(first_build))
            if (bInfo->build == first_build)
                realm.realmBuildInfo = *bInfo;

    ///- Append port to IP address.
    std::ostringstream ss;
    ss << address << ":" << port;
    realm.address = ss.str();
}

void RealmList::UpdateIfNeed()
{
    // maybe disabled or updated recently
    if(m_UpdateInterval == 0 || m_NextUpdateTime > time(NULL))
        return;

    m_NextUpdateTime = time(NULL) + m_UpdateInterval;

    // Clears Realm list
    m_realms.clear();

    // Get the content of the realmlist table in the database
    UpdateRealms(false);
}

void RealmList::UpdateRealms(bool init)
{
    sLog.outDetail("Updating Realm List...");

    ////                                                          0       1         2       3     4      5       6                 7                  8           9
    QueryResultAutoPtr result = AccountsDatabase.Query("SELECT realm_id, name, ip_address, port, icon, flags, timezone, required_permission_mask, population, allowed_builds "
        "FROM realms WHERE (flags & 1) = 0 ORDER BY name");

    ///- Circle through results and add them to the realm map
    if(result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint64 requiredPermissionMask = fields[7].GetUInt64();

            uint8 realmflags = fields[5].GetUInt8();

            if (realmflags & ~(REALM_FLAG_OFFLINE|REALM_FLAG_NEW_PLAYERS|REALM_FLAG_RECOMMENDED|REALM_FLAG_SPECIFYBUILD))
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Realm allowed have only OFFLINE Mask 0x2), or NEWPLAYERS (mask 0x20), or RECOMENDED (mask 0x40), or SPECIFICBUILD (mask 0x04) flags in DB");
                realmflags &= (REALM_FLAG_OFFLINE|REALM_FLAG_NEW_PLAYERS|REALM_FLAG_RECOMMENDED|REALM_FLAG_SPECIFYBUILD);
            }

            UpdateRealm(
                fields[0].GetUInt32(), fields[1].GetCppString(), fields[2].GetCppString(), fields[3].GetUInt32(),
                fields[4].GetUInt8(), RealmFlags(realmflags), fields[6].GetUInt8(),
                requiredPermissionMask, fields[8].GetFloat(), fields[9].GetCppString());

            if(init)
                sLog.outString("Added realm \"%s\"", fields[1].GetString());
        } while( result->NextRow() );
    }
}
