// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "ArenaTeam.h"
#include "World.h"
#include "Chat.h"
#include "BattleGroundMgr.h"
#include "Language.h"

#ifndef UNORDERED_MAP
#define UNORDERED_MAP std::unordered_map
#endif
#include <GameEvent.h>

ArenaTeam::ArenaTeam()
{
    Id                  = 0;
    Type                = 0;
    Name                = "";
    CaptainGuid         = 0;
    BackgroundColor     = 0;                                // background
    EmblemStyle         = 0;                                // icon
    EmblemColor         = 0;                                // icon color
    BorderStyle         = 0;                                // border
    BorderColor         = 0;                                // border color
    stats.games_week    = 0;
    stats.games_season  = 0;
    stats.rank          = 0;
    stats.rating        = 1500;
    stats.wins_week     = 0;
    stats.wins_season   = 0;
}

ArenaTeam::~ArenaTeam()
{
}

bool ArenaTeam::Create(uint64 captainGuid, uint32 type, std::string ArenaTeamName, bool ignore_same_name)
{
    if (!sObjectMgr.GetPlayerInWorld(captainGuid))                      // player not exist
        return false;
    if (!ignore_same_name && sObjectMgr.GetArenaTeamByName(ArenaTeamName))            // arena team with this name already exist
        return false;

    sLog.outDebug("GUILD: creating arena team %s to leader: %u", ArenaTeamName.c_str(), GUID_LOPART(captainGuid));

    CaptainGuid = captainGuid;
    Name = ArenaTeamName;
    Type = type;

    Id = sObjectMgr.GenerateArenaTeamId();

    // ArenaTeamName already assigned to ArenaTeam::name, use it to encode string for DB
    RealmDataDatabase.escape_string(ArenaTeamName);

    RealmDataDatabase.BeginTransaction();
    // CharacterDatabase.PExecute("DELETE FROM arena_team WHERE arenateamid='%u'", Id); - MAX(arenateam)+1 not exist
    RealmDataDatabase.PExecute("DELETE FROM arena_team_member WHERE arenateamid='%u'", Id);
    RealmDataDatabase.PExecute("INSERT INTO arena_team (arenateamid,name,captainguid,type,BackgroundColor,EmblemStyle,EmblemColor,BorderStyle,BorderColor) "
        "VALUES('%u','%s','%u','%u','%u','%u','%u','%u','%u')",
        Id, ArenaTeamName.c_str(), GUID_LOPART(CaptainGuid), Type, BackgroundColor, EmblemStyle, EmblemColor, BorderStyle, BorderColor);
    RealmDataDatabase.PExecute("INSERT INTO arena_team_stats (arenateamid, rating, games, wins, played, wins2, rank) VALUES "
        "('%u', '%u', '%u', '%u', '%u', '%u', '%u')", Id, stats.rating, stats.games_week, stats.wins_week, stats.games_season, stats.wins_season, stats.rank);

    RealmDataDatabase.CommitTransaction();

    AddMember(CaptainGuid);

    sLog.outLog(LOG_ARENA, "New ArenaTeam created [Name: %s] [Id: %u] [Type: %u] [Captain GUID: %llu]", ArenaTeamName.c_str(), GetId(), GetType(), GetCaptain());
    return true;
}

bool ArenaTeam::CreateTempForSolo3v3(std::vector<Player*> team_members, uint8 team)
{
    if (team_members.size() < SOLO_3v3_MIN_PLAYERS)
    {
        sLog.outLog(LOG_CRITICAL, "CreateTempForSolo3v3 team_members.size() < SOLO_3v3_MIN_PLAYERS - %u %u", team_members.size(), team);
        return false;
    }
    
    // Generate new arena team id
    Id = sObjectMgr.GenerateTempArenaTeamId();
    // Assign member variables
    CaptainGuid = team_members[0]->GetGUID();
    Type = ARENA_TEAM_3v3;

    std::stringstream ssTeamName;
    ssTeamName << "Solo Team " << (team + 1);
    Name = ssTeamName.str();
    BackgroundColor = 0;
    EmblemStyle = 0;
    EmblemColor = 0;
    BorderStyle = 0;
    BorderColor = 0;

    stats.rating = 0;
    stats.games_week = 0;
    stats.wins_week = 0;
    stats.games_season = 0;
    stats.wins_season = 0;
    //stats.rank =

    for (int i = 0; i < SOLO_3v3_MIN_PLAYERS; i++)
    {
        if (!team_members[i])
        {
            sLog.outLog(LOG_CRITICAL, "CreateTempForSolo3v3() !team_members[i]");
            return false;
        }

        if (!team_members[i]->GetArenaTeamId(GetSlotByType(ARENA_TEAM_3v3)))
        {
            sLog.outLog(LOG_CRITICAL, "CreateTempForSolo3v3() !GetArenaTeamId()");
            return false;
        }
        
        ArenaTeam* team = sObjectMgr.GetArenaTeamById(team_members[i]->GetArenaTeamId(GetSlotByType(ARENA_TEAM_3v3)));

        if (!team)
            continue;

        ArenaTeamMember newMember;
        for (MemberList::const_iterator itr = team->members.begin(); itr != team->members.end(); ++itr)
        {
            newMember = *itr;
        }

        stats.games_week += team->stats.games_week;
        stats.games_season += team->stats.games_season;
        stats.rating += team->GetRating();
        stats.wins_week += team->stats.wins_week;
        stats.wins_season += team->stats.wins_season;

        members.push_back(newMember);
    }

    if (members.size() < SOLO_3v3_MIN_PLAYERS)
    {
        sLog.outLog(LOG_CRITICAL, "members.size() < SOLO_3v3_MIN_PLAYERS");
        return false;
    }

    stats.rating /= SOLO_3v3_MIN_PLAYERS;
    stats.games_week /= SOLO_3v3_MIN_PLAYERS;
    stats.wins_week /= SOLO_3v3_MIN_PLAYERS;
    stats.games_season /= SOLO_3v3_MIN_PLAYERS;
    stats.wins_season /= SOLO_3v3_MIN_PLAYERS;

    return true;
}

bool ArenaTeam::AddMember(const uint64& PlayerGuid)
{
    std::string plName;
    uint8 plClass;
    uint8 plRace;

    // arena team is full (can't have more than type * 2 players!)
    if (GetMembersSize() >= GetType() * 2)
        return false;

    Player *pl = sObjectMgr.GetPlayerInWorld(PlayerGuid);
    if (pl)
    {
        if (pl->GetArenaTeamId(GetSlot()))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Arena::AddMember() : player already in this sized team");
            return false;
        }

        plClass = (uint8)pl->GetClass();
        plRace = (uint8)pl->GetRace();
        plName = pl->GetName();
    }
    else
    {
        //                                                     0     1
        QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT name, class, race FROM characters WHERE guid='%u'", GUID_LOPART(PlayerGuid));
        if (!result)
            return false;

        plName = (*result)[0].GetCppString();
        plClass = (*result)[1].GetUInt8();
        plRace = (*result)[2].GetUInt8();

        // check if player already in arenateam of that size
        if (Player::GetArenaTeamIdFromDB(PlayerGuid, GetType()) != 0)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Arena::AddMember() : player already in this sized team");
            return false;
        }
    }

    // remove all player signs from another petitions
    // this will be prevent attempt joining player to many arenateams and corrupt arena team data integrity
    Player::RemovePetitionsAndSigns(PlayerGuid, GetType());

    ArenaTeamMember newmember;
    newmember.name              = plName;
    newmember.guid              = PlayerGuid;
    newmember.Class             = plClass;
    newmember.race              = plRace;
    newmember.games_season      = 0;
    newmember.games_week        = 0;
    newmember.wins_season       = 0;
    newmember.wins_week         = 0;
    newmember.personal_rating   = 1500;

    members.push_back(newmember);

    RealmDataDatabase.PExecute("INSERT INTO arena_team_member (arenateamid, guid, personal_rating) VALUES ('%u', '%u', '%u')", Id, GUID_LOPART(newmember.guid), newmember.personal_rating);

    if (pl)
    {
        pl->SetInArenaTeam(Id, GetSlot());
        pl->SetArenaTeamIdInvited(0);
        // pl->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot()*6) + 5, newmember.personal_rating);
        pl->SetArenaTeamInfoField(GetSlot(), 5, newmember.personal_rating);
        // hide promote/remove buttons
        if (CaptainGuid != PlayerGuid)
            pl->SetArenaTeamInfoField(GetSlot(), 1, 1);
            // pl->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot() * 6) + 1, 1);
        sLog.outLog(LOG_ARENA, "Player: %s [GUID: %u] joined arena team type: %u [Id: %u], IP: %s.", pl->GetName(), pl->GetGUIDLow(), GetType(), GetId(), pl->GetSession()->GetRemoteAddress().c_str());
    }
    return true;
}

bool ArenaTeam::LoadArenaTeamFromDB(uint32 ArenaTeamId)
{
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT arenateamid,name,captainguid,type,BackgroundColor,EmblemStyle,EmblemColor,BorderStyle,BorderColor FROM arena_team WHERE arenateamid = '%u'", ArenaTeamId);

    if (!result)
        return false;

    Field *fields = result->Fetch();

    Id = fields[0].GetUInt32();
    Name = fields[1].GetCppString();
    CaptainGuid  = MAKE_NEW_GUID(fields[2].GetUInt32(), 0, HIGHGUID_PLAYER);
    Type = fields[3].GetUInt32();
    BackgroundColor = fields[4].GetUInt32();
    EmblemStyle = fields[5].GetUInt32();
    EmblemColor = fields[6].GetUInt32();
    BorderStyle = fields[7].GetUInt32();
    BorderColor = fields[8].GetUInt32();

    // only load here, so additional checks can be made
    LoadStatsFromDB(ArenaTeamId);
    LoadMembersFromDB(ArenaTeamId);

    if (Empty())
    {
        // arena team is empty, delete from db
        RealmDataDatabase.BeginTransaction();
        if (sWorld.getConfig(CONFIG_ARENA_KEEP_TEAMS))
            RealmDataDatabase.PExecute("UPDATE arena_team SET captainguid = 0 where arenateamid = '%u'", ArenaTeamId);
        else
            RealmDataDatabase.PExecute("DELETE FROM arena_team WHERE arenateamid = '%u'", ArenaTeamId);
        
        RealmDataDatabase.PExecute("DELETE FROM arena_team_member WHERE arenateamid = '%u'", ArenaTeamId);
        RealmDataDatabase.PExecute("DELETE FROM arena_team_stats WHERE arenateamid = '%u'", ArenaTeamId);
        RealmDataDatabase.CommitTransaction();
        return false;
    }

    return true;
}

void ArenaTeam::LoadStatsFromDB(uint32 ArenaTeamId)
{
    //                                                     0      1     2    3      4     5
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT rating,games,wins,played,wins2,rank FROM arena_team_stats WHERE arenateamid = '%u'", ArenaTeamId);

    if (!result)
        return;

    Field *fields = result->Fetch();

    stats.rating        = fields[0].GetUInt32();
    stats.games_week    = fields[1].GetUInt32();
    stats.wins_week     = fields[2].GetUInt32();
    stats.games_season  = fields[3].GetUInt32();
    stats.wins_season   = fields[4].GetUInt32();
    stats.rank          = fields[5].GetUInt32();
}

void ArenaTeam::LoadMembersFromDB(uint32 ArenaTeamId)
{
    //                                                                 0           1          2            3              4             5           6      7
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT member.guid, played_week, wons_week, played_season, wons_season, personal_rating, name, class, race "
                                                   "FROM arena_team_member member "
                                                   "INNER JOIN characters chars on member.guid = chars.guid "
                                                   "WHERE member.arenateamid = '%u'", ArenaTeamId);
    if (!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        ArenaTeamMember newmember;
        newmember.guid          = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        newmember.games_week    = fields[1].GetUInt32();
        newmember.wins_week     = fields[2].GetUInt32();
        newmember.games_season  = fields[3].GetUInt32();
        newmember.wins_season   = fields[4].GetUInt32();
        newmember.personal_rating = fields[5].GetUInt32();
        newmember.name          = fields[6].GetCppString();
        newmember.Class         = fields[7].GetUInt8();
        newmember.race          = fields[8].GetUInt8();

        members.push_back(newmember);
    }
    while (result->NextRow());
}

void ArenaTeam::SetCaptain(const uint64& guid)
{
    // disable remove/promote buttons
    Player *oldcaptain = sObjectMgr.GetPlayerInWorld(GetCaptain());
    if (oldcaptain)
        oldcaptain->SetArenaTeamInfoField(GetSlot(), 1, 1);
        // oldcaptain->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 1 + (GetSlot() * 6), 1);

    // set new captain
    CaptainGuid = guid;

    // update database
    RealmDataDatabase.PExecute("UPDATE arena_team SET captainguid = '%u' WHERE arenateamid = '%u'", GUID_LOPART(guid), Id);

    // enable remove/promote buttons
    Player *newcaptain = sObjectMgr.GetPlayerInWorld(guid);
    if (newcaptain)
    {        
        // newcaptain->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 1 + (GetSlot() * 6), 0);
        newcaptain->SetArenaTeamInfoField(GetSlot(), 1, 0);
        sLog.outLog(LOG_ARENA, "Player: %s [GUID: %u] promoted player: %s [GUID: %llu] to leader of arena team [Id: %u] [Type: %u].", oldcaptain->GetName(), oldcaptain->GetGUIDLow(), newcaptain->GetName(), newcaptain->GetGUID(), GetId(), GetType());
    }
}

void ArenaTeam::DelMember(uint64 guid)
{
    for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (itr->guid == guid)
        {
            members.erase(itr);
            break;
        }
    }

    Player *player = sObjectMgr.GetPlayerInWorld(guid);

    if (player)
    {
        player->SetInArenaTeam(0, GetSlot());
        player->GetSession()->SendArenaTeamCommandResult(ERR_ARENA_TEAM_QUIT_S, GetName(), "", 0);
        // delete all info regarding this team
        for (int i = 0; i < 6; ++i)
        {
            //player->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (GetSlot() * 6) + i, 0);
            player->SetArenaTeamInfoField(GetSlot(), i, 0);
        }
        sLog.outLog(LOG_ARENA, "Player: %s [GUID: %u] left arena team type: %u [Id: %u].", player->GetName(), player->GetGUIDLow(), GetType(), GetId());
    }
    RealmDataDatabase.PExecute("DELETE FROM arena_team_member WHERE arenateamid = '%u' AND guid = '%u'", GetId(), GUID_LOPART(guid));
}

void ArenaTeam::Disband(WorldSession *session)
{
    // event
    WorldPacket data;
    session->BuildArenaTeamEventPacket(&data, ERR_ARENA_TEAM_DISBANDED_S, 2, session->GetPlayerName(), GetName(), "");
    BroadcastPacket(&data);

    while (!members.empty())
    {
        // Removing from members is done in DelMember.
        DelMember(members.front().guid);
    }

    if (Player *player = session->GetPlayer())
        sLog.outLog(LOG_ARENA, "Player: %s [GUID: %u] disbanded arena team type: %u [Id: %u].", player->GetName(), player->GetGUIDLow(), GetType(), GetId());

    RealmDataDatabase.BeginTransaction();
    if (sWorld.getConfig(CONFIG_ARENA_KEEP_TEAMS))
        RealmDataDatabase.PExecute("UPDATE arena_team SET captainguid = 0 where arenateamid = '%u'", Id);
    else
        RealmDataDatabase.PExecute("DELETE FROM arena_team WHERE arenateamid = '%u'", Id);
    RealmDataDatabase.PExecute("DELETE FROM arena_team_member WHERE arenateamid = '%u'", Id); //< this should be alredy done by calling DelMember(memberGuids[j]); for each member
    RealmDataDatabase.PExecute("DELETE FROM arena_team_stats WHERE arenateamid = '%u'", Id);
    RealmDataDatabase.CommitTransaction();
    sObjectMgr.RemoveArenaTeam(Id);
}

void ArenaTeam::Roster(WorldSession *session)
{
    Player *pl = NULL;

    WorldPacket data(SMSG_ARENA_TEAM_ROSTER, 100);
    data << uint32(GetId());                                // arena team id
    data << uint32(GetMembersSize());                       // members count
    data << uint32(GetType());                              // arena team type?

    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        pl = sObjectMgr.GetPlayerInWorld(itr->guid);

        data << uint64(itr->guid);                          // guid
        data << uint8((pl ? 1 : 0));                        // online flag
        data << itr->name;                                  // member name
        data << uint32((itr->guid == GetCaptain() ? 0 : 1));// captain flag 0 captain 1 member
        data << uint8((pl ? pl->GetLevel() : 0));           // unknown, level?
        data << uint8(itr->Class);                          // class
        data << uint32(itr->games_week);                    // played this week
        data << uint32(itr->wins_week);                     // wins this week
        data << uint32(itr->games_season);                  // played this season
        data << uint32(itr->wins_season);                   // wins this season
        data << uint32(itr->personal_rating);               // personal rating
    }

    session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent SMSG_ARENA_TEAM_ROSTER");
}

void ArenaTeam::Query(WorldSession *session)
{
    WorldPacket data(SMSG_ARENA_TEAM_QUERY_RESPONSE, 4*7+GetName().size()+1);
    data << uint32(GetId());                                // team id
    data << GetName();                                      // team name
    data << uint32(GetType());                              // arena team type (2=2x2, 3=3x3 or 5=5x5)
    data << uint32(BackgroundColor);                        // background color
    data << uint32(EmblemStyle);                            // emblem style
    data << uint32(EmblemColor);                            // emblem color
    data << uint32(BorderStyle);                            // border style
    data << uint32(BorderColor);                            // border color
    session->SendPacket(&data);
    sLog.outDebug("WORLD: Sent SMSG_ARENA_TEAM_QUERY_RESPONSE");
}

void ArenaTeam::Stats(WorldSession *session)
{
    WorldPacket data(SMSG_ARENA_TEAM_STATS, 4*7);
    data << uint32(GetId());                                // arena team id
    data << uint32(stats.rating);                           // rating
    data << uint32(stats.games_week);                       // games this week
    data << uint32(stats.wins_week);                        // wins this week
    data << uint32(stats.games_season);                     // played this season
    data << uint32(stats.wins_season);                      // wins this season
    data << uint32(stats.rank);                             // rank
    session->SendPacket(&data);
}

void ArenaTeam::NotifyStatsChanged()
{
    // this is called after a rated match ended
    // updates arena team stats for every member of the team (not only the ones who participated!)
    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        Player * plr = sObjectMgr.GetPlayerInWorld(itr->guid);
        if (plr)
            Stats(plr->GetSession());
    }
}

void ArenaTeam::InspectStats(WorldSession *session, uint64 guid)
{
    ArenaTeamMember* member = GetMember(guid);
    if (!member)
        return;

    WorldPacket data(MSG_INSPECT_ARENA_TEAMS, 8+1+4*6);
    data << uint64(guid);                                   // player guid
    data << uint8(GetSlot());                               // slot (0...2)
    data << uint32(GetId());                                // arena team id
    data << uint32(stats.rating);                           // rating
    data << uint32(stats.games_season);                     // season played
    data << uint32(stats.wins_season);                      // season wins
    data << uint32(member->games_season);                   // played (count of all games, that the inspected member participated...)
    data << uint32(member->personal_rating);                // personal rating
    session->SendPacket(&data);
}

void ArenaTeam::SetEmblem(uint32 backgroundColor, uint32 emblemStyle, uint32 emblemColor, uint32 borderStyle, uint32 borderColor)
{
    BackgroundColor = backgroundColor;
    EmblemStyle = emblemStyle;
    EmblemColor = emblemColor;
    BorderStyle = borderStyle;
    BorderColor = borderColor;

    RealmDataDatabase.PExecute("UPDATE arena_team SET BackgroundColor='%u', EmblemStyle='%u', EmblemColor='%u', BorderStyle='%u', BorderColor='%u' WHERE arenateamid='%u'", BackgroundColor, EmblemStyle, EmblemColor, BorderStyle, BorderColor, Id);
}

void ArenaTeam::SetStats(uint32 stat_type, uint32 value)
{
    switch (stat_type)
    {
        case STAT_TYPE_RATING:
            stats.rating = value;
            RealmDataDatabase.PExecute("UPDATE arena_team_stats SET rating = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_GAMES_WEEK:
            stats.games_week = value;
            RealmDataDatabase.PExecute("UPDATE arena_team_stats SET games = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_WINS_WEEK:
            stats.wins_week = value;
            RealmDataDatabase.PExecute("UPDATE arena_team_stats SET wins = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_GAMES_SEASON:
            stats.games_season = value;
            RealmDataDatabase.PExecute("UPDATE arena_team_stats SET played = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_WINS_SEASON:
            stats.wins_season = value;
            RealmDataDatabase.PExecute("UPDATE arena_team_stats SET wins2 = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        case STAT_TYPE_RANK:
            stats.rank = value;
            RealmDataDatabase.PExecute("UPDATE arena_team_stats SET rank = '%u' WHERE arenateamid = '%u'", value, GetId());
            break;
        default:
            sLog.outDebug("unknown stat type in ArenaTeam::SetStats() %u", stat_type);
            break;
    }
}

void ArenaTeam::BroadcastPacket(WorldPacket *packet)
{
    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        Player *player = sObjectMgr.GetPlayerInWorld(itr->guid);
        if (player)
            player->SendPacketToSelf(packet);
    }
}

uint8 ArenaTeam::GetSlotByType(uint32 type)
{
    switch (type)
    {
        case ARENA_TEAM_2v2: return 0;
        case ARENA_TEAM_3v3: return 1;
        case ARENA_TEAM_5v5: return 2;
        //case ARENA_TEAM_1v1: return 3;
        default:
            break;
    }
    sLog.outLog(LOG_DEFAULT, "ERROR: FATAL: Unknown arena team type %u for some arena team", type);
    return 0xFF;
}

bool ArenaTeam::HaveMember(const uint64& guid) const
{
    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
        if (itr->guid == guid)
            return true;

    return false;
}

uint32 ArenaTeam::GetPoints(uint32 MemberRating)
{
    // returns how many points would be awarded with this team type with this rating
    float points;

    uint32 rating = MemberRating + 150 < stats.rating ? MemberRating : stats.rating;

    if (rating<=1500)
        points = (float)rating * 0.22f + 14.0f;
    else
        points = 1511.26f / (1.0f + 1639.28f * exp(-0.00412f * (float)rating));

    // type penalties for <5v5 teams
    if (Type == ARENA_TEAM_2v2)
        points *= 0.76f;
    else if (Type == ARENA_TEAM_3v3)
        points *= 0.88f; // default is 0.88, but modified for solo 3v3

    return (uint32) points;
}

int32 ArenaTeam::GetChanceAgainstMod(uint32 own_rating, uint32 enemy_rating, bool Won)
{
    // want to change something? see info/Gensen/calc_arenarating.php

    float difference = (float)enemy_rating - (float)own_rating;

    // we using this because low online (reteams, sniping, instant discard, same teams)
    // if we're the losing team and we have much more rating than our enemies - consider their rating only 100 less than ours
    // for all arenas
    //if (!Won && GetType() == ARENA_TEAM_3v3)
    //{
    //    //-((float)sBattleGroundMgr.GetMaxRatingDifference()/3*2)
    //    float maxdiff = 0; // make 3v3 easy, because it will be always 1500-1700 enemy rating
    //    if (difference < maxdiff) // if enemy rating is more than 100 lower than ours and we lost - consider enemy rating only 100 less.
    //        difference = maxdiff; // difference is minus, cause enemy rating is lower
    //}

    float modToUse;

    //if (sWorld.isEasyRealm() && GetType() == ARENA_TEAM_2v2)
    //{
    //    float modRight = ((Won ? 36.0f : 32.0f) * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(10.0f)*difference / 400.0f)))); // improved gain
    //    float modLeft = ((Won ? 36.0f : 32.0f) * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(10.0f)*difference / 160.0f / sqrt(sqrt(fabs(difference) ? fabs(difference) : 1)))))); // changed - increased rating gain
    //    modToUse = modRight > modLeft ? modRight : modLeft;
    //} else if (GetType() == ARENA_TEAM_3v3)
    //    modToUse = (18.0f * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(10.0f)*(difference) / 600.0f))));
    //else

	// BG rating and 3v3
    if (GetType() == ARENA_TEAM_3v3 || GetType() == ARENA_TEAM_5v5)
    {
        //modToUse = ((32.0f * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(5.0f)*(difference) / 400.0f)))) / 2) + (Won ? 1 : 0); // OLD modified for 3v3
        //modToUse = 32.0f * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(5.0f)*(difference) / 400.0f))); // modified for 3v3
        if (!Won)
            modToUse = -10; //lose is always -10
        else
        {
            // arena_3v3_calc.php
            if (own_rating >= 2200)
                modToUse = 2;
            else if (own_rating >= 2100)
                modToUse = 3;
            else
            {
                modToUse = 10 - floor((own_rating - 1500) / 55);

                if (modToUse < 4)
                    modToUse = 4;
            }
        }
    }
	// 2v2
    else
    {
		if (sWorld.isEasyRealm())
		{
			float modRight = ((Won ? 30.0f : 26.0f) * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(10.0f)*difference / 400.0f)))); // improved gain
			float modLeft = ((Won ? 30.0f : 26.0f) * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(10.0f)*difference / 200.0f / sqrt(sqrt(fabs(difference) ? fabs(difference) : 1))))));
			modToUse = modRight > modLeft ? modRight : modLeft;
		}
		else
		{
			//modToUse = 32.0f * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(10.0f)*(difference) / 400.0f))); // original arena rating formula

			modToUse = 32.0f * ((Won ? 1.0f : 0.0f) - 1.0f / (1.0f + exp(log(10.0f)*(difference) / 400.0f)));

			// simplify
			if (!Won && modToUse < -15)
				modToUse = -15;
		}
    }

    int32 modReturn = (int32)(Won ? floor(modToUse) : ceil(modToUse));

    // do not lower 1500 teams less than 1500
    if (!Won && own_rating + modReturn < 1500) // mod is minus number here
        return int32(1500 - own_rating);

    return modReturn ? modReturn : (Won ? 1 : -1);
}

int32 ArenaTeam::WonAgainst(uint32 againstRating)
{
    if (GetId() >= 0xFFF00000)
    {
        return 0;
    }
    
    // called when the team has won
    //'chance' calculation - to beat the opponent
    // calculate the rating modification (ELO system with k=32)
    int32 mod = GetChanceAgainstMod(stats.rating, againstRating, true);
    // modify the team stats accordingly
    stats.rating += mod;
    stats.games_week += 1;
    stats.wins_week += 1;
    stats.games_season += 1;
    stats.wins_season += 1;

    //update team's rank
    stats.rank = 1;
    ObjectMgr::ArenaTeamMap::iterator i = sObjectMgr.GetArenaTeamMapBegin();
    for (; i != sObjectMgr.GetArenaTeamMapEnd(); ++i)
    {
        if (i->second->GetType() == this->Type && i->second->GetStats().rating > stats.rating)
            ++stats.rank;
    }

    // return the rating change, used to display it on the results screen
    return mod;
}

int32 ArenaTeam::LostAgainst(uint32 againstRating)
{
    if (GetId() >= 0xFFF00000)
    {
        return 0;
    }
    
    // called when the team has lost
    //'chance' calculation - to loose to the opponent
    // calculate the rating modification (ELO system with k=32)
    int32 mod = GetChanceAgainstMod(stats.rating, againstRating, false);
    // modify the team stats accordingly
    stats.rating += mod;
    stats.games_week += 1;
    stats.games_season += 1;
    //update team's rank

    stats.rank = 1;
    ObjectMgr::ArenaTeamMap::iterator i = sObjectMgr.GetArenaTeamMapBegin();
    for (; i != sObjectMgr.GetArenaTeamMapEnd(); ++i)
    {
        if (i->second->GetType() == this->Type && i->second->GetStats().rating > stats.rating)
            ++stats.rank;
    }

    // return the rating change, used to display it on the results screen
    return mod;
}

void ArenaTeam::RewardArenaPoints(Player* plr, uint32 my_rating, bool win)
{
    float points_calc = (float)my_rating / 10;

    if (win)
    {
        if (points_calc > 300)
			points_calc = 300;
    }
    else
    {
		points_calc *= 0.25;

        if (points_calc > 75)
			points_calc = 75;
    }

    if (GetType() == ARENA_TEAM_3v3)
		points_calc *= 0.75;

	uint32 points_to_add = uint32(round(points_calc));
	uint32 bonus = plr->CalculateBonus(points_to_add);
	points_to_add += bonus;

    plr->ModifyArenaPoints((uint32)points_to_add);
    sLog.outLog(LOG_ARENA_FLUSH, "Player %s received arena points: %u (rating %u, bonus %u)", plr->GetName(), points_to_add, my_rating, bonus);

    ChatHandler(plr).PSendSysMessage(15574, points_to_add);
}

//void ArenaTeam::BGModStats(Player* plr, bool won)
//{
//	if (CaptainGuid != plr->GetGUID() || Type != ARENA_TEAM_5v5)
//		return;
//
//	for (MemberList::iterator itrTeamMember = members.begin(); itrTeamMember != members.end(); ++itrTeamMember)
//	{
//		if (itrTeamMember->guid != plr->GetGUID())
//			continue;
//
//		int32 mod = GetChanceAgainstMod(itrTeamMember->personal_rating, itrTeamMember->personal_rating, won);
//
//		uint32 rating_before = itrTeamMember->personal_rating;
//		sObjectMgr.ModifyPersonalRating(*itrTeamMember, plr, mod, GetSlot());
//
//		if (won)
//		{
//			itrTeamMember->wins_season += 1;
//			itrTeamMember->wins_week += 1;
//		}
//		itrTeamMember->games_season += 1;
//		itrTeamMember->games_week += 1;
//
//		ModStats().rating = itrTeamMember->personal_rating;
//
//		if (won)
//		{
//			ChatHandler(plr).PSendSysMessage(15649, mod, rating_before, itrTeamMember->personal_rating);
//			sLog.outLog(LOG_ARENA, "Battleground Win for %s (GUID: %u), bgtype: %u, amount: %d (%u -> %u)", plr->GetName(), plr->GetGUIDLow(), GetType(), mod, rating_before, itrTeamMember->personal_rating);
//		}
//		else
//		{
//			ChatHandler(plr).PSendSysMessage(15650, mod, rating_before, itrTeamMember->personal_rating);
//			sLog.outLog(LOG_ARENA, "Battleground Lose for %s (GUID: %u), bgtype: %u, amount: %d (%u -> %u)", plr->GetName(), plr->GetGUIDLow(), GetType(), mod, rating_before, itrTeamMember->personal_rating);
//		}
//
//		if (won)
//		{
//			stats.wins_season += 1;
//			stats.wins_week += 1;
//		}
//		stats.games_week += 1;
//		stats.games_season += 1;
//
//		plr->SetArenaTeamInfoField(GetSlot(), 2, stats.games_week);
//		plr->SetArenaTeamInfoField(GetSlot(), 3, stats.games_season);
//
//		SaveToDB();
//		NotifyStatsChanged();
//
//		return;
//	}
//}

void ArenaTeam::Solo3v3ModStats(Player* plr, bool won)
{
    // get real teams
    for (MemberList::const_iterator itrTempTeamMember = members.begin(); itrTempTeamMember != members.end(); ++itrTempTeamMember)
    {
        if (itrTempTeamMember->guid != plr->GetGUID())
            continue;
        
        // find real arena team for player
        for (UNORDERED_MAP<uint32, ArenaTeam*>::iterator itrArenaTeam = sObjectMgr.GetArenaTeamMapBegin(); itrArenaTeam != sObjectMgr.GetArenaTeamMapEnd(); itrArenaTeam++)
        {
            if (itrArenaTeam->first < 0xFFF00000 && itrArenaTeam->second->CaptainGuid == plr->GetGUID() && itrArenaTeam->second->Type == ARENA_TEAM_3v3)
            {
                // find real arena member
                for (MemberList::iterator itrTeamMember = itrArenaTeam->second->members.begin(); itrTeamMember != itrArenaTeam->second->members.end(); ++itrTeamMember)
                {
                    if (itrTeamMember->guid == itrArenaTeam->second->GetCaptain())
                    {
                        int32 mod = GetChanceAgainstMod(itrTeamMember->personal_rating, itrTeamMember->personal_rating, won);

                        uint32 rating_before = itrTeamMember->personal_rating;
                        sObjectMgr.ModifyPersonalRating(*itrTeamMember, plr, mod, GetSlot());

                        if (won)
                        {
                            itrTeamMember->wins_season += 1;
                            itrTeamMember->wins_week += 1;
                        }
						itrTeamMember->games_season += 1;
						itrTeamMember->games_week += 1;

                        // team rating is always == personal rating
                        itrArenaTeam->second->ModStats().rating = itrTeamMember->personal_rating;

                        if (won)
                        {
                            ChatHandler(plr).PSendSysMessage(LANG_ARENA_WIN, mod, rating_before, itrTeamMember->personal_rating);
                            sLog.outLog(LOG_ARENA, "Win for %s (GUID: %u), arenatype: %u, amount: %d (%u -> %u)", plr->GetName(), plr->GetGUIDLow(), GetType(), mod, rating_before, itrTeamMember->personal_rating);
                        }
                        else
                        {
                            ChatHandler(plr).PSendSysMessage(LANG_ARENA_LOSE, mod, rating_before, itrTeamMember->personal_rating);
                            sLog.outLog(LOG_ARENA, "Lose for %s (GUID: %u), arenatype: %u, amount: %d (%u -> %u)", plr->GetName(), plr->GetGUIDLow(), GetType(), mod, rating_before, itrTeamMember->personal_rating);
                        }

                        // reward ap
                        RewardArenaPoints(plr, itrTeamMember->personal_rating, won);

						if (won)
						{
							itrArenaTeam->second->stats.wins_season += 1;
							itrArenaTeam->second->stats.wins_week += 1;
						}
						itrArenaTeam->second->stats.games_week += 1;
						itrArenaTeam->second->stats.games_season += 1;

						plr->SetArenaTeamInfoField(GetSlot(), 2, itrArenaTeam->second->stats.games_week);
						plr->SetArenaTeamInfoField(GetSlot(), 3, itrArenaTeam->second->stats.games_season);

						if (won)
						{
                            // 3v3 arena quest
							plr->KilledMonster(695008, 0);

                            if (sGameEventMgr.IsActiveEvent(1009))
                            {
                                if (roll_chance_f(10.91f))
                                {
                                    plr->GiveItem(FLAWLESS_LEGENDARY_KEY, 1);
                                }
                                else
                                {
                                    ChatHandler(plr).SendSysMessage(16758);
                                }
                            }

							// clear reports
                            sLog.outLog(LOG_ARENA, "ArenaReport: Player %s (%u) won arena, reports was cleared", plr->GetName(), plr->GetGUIDLow());
							sWorld.m_arenareports.erase(plr->GetGUID());
						}

						itrArenaTeam->second->SaveToDB();
						itrArenaTeam->second->NotifyStatsChanged();

						return;
                    }
                }
            }
        }
    }
}

void ArenaTeam::MemberLost(Player * plr, uint32 againstRating)
{
    if (GetId() >= 0xFFF00000)
    {
        Solo3v3ModStats(plr, false);
        return;
    }   
    
    // called for each participant of a match after losing
    for (MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        if (itr->guid == plr->GetGUID())
        {
            // update personal rating
            int32 mod = GetChanceAgainstMod(itr->personal_rating, againstRating, false);

            uint32 rating_before = itr->personal_rating;
            sObjectMgr.ModifyPersonalRating(*itr, plr, mod, GetSlot());
            sLog.outLog(LOG_ARENA, "Lose for GUID: %u, arenatype: %u, amount: %d (%u -> %u)", GUID_LOPART(itr->guid), GetType(), mod, rating_before, itr->personal_rating);

            ChatHandler(plr).PSendSysMessage(LANG_ARENA_LOSE, mod, rating_before, itr->personal_rating);
            // update personal played stats
            itr->games_week +=1;
            itr->games_season +=1;

            // update the unit fields
            // plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 2, itr->games_week);
            plr->SetArenaTeamInfoField(GetSlot(), 2, itr->games_week);
            // plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + 6 * GetSlot() + 3, itr->games_season);
            plr->SetArenaTeamInfoField(GetSlot(), 3, itr->games_season);

            // add arena points
            // ap_calc.php
			if (sWorld.isEasyRealm())
				RewardArenaPoints(plr, itr->personal_rating, false);
        }
    }
}

void ArenaTeam::MemberWon(Player * plr, uint32 againstRating)
{
    if (GetId() >= 0xFFF00000)
    {
        Solo3v3ModStats(plr, true);
        return;
    }
    
    // called for each participant after winning a match
    for (MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        if (itr->guid == plr->GetGUID())
        {
            // update personal rating
            int32 mod = GetChanceAgainstMod(itr->personal_rating, againstRating, true);

            uint32 rating_before = itr->personal_rating;
            sObjectMgr.ModifyPersonalRating(*itr, plr, mod, GetSlot());
            sLog.outLog(LOG_ARENA, "Win for GUID: %u, arenatype: %u, amount: %d (%u -> %u)", GUID_LOPART(itr->guid), GetType(), mod, rating_before, itr->personal_rating);

            ChatHandler(plr).PSendSysMessage(LANG_ARENA_WIN, mod, rating_before, itr->personal_rating);
            
            // update personal stats
            itr->games_week +=1;
            itr->games_season +=1;
            itr->wins_season += 1;
            itr->wins_week += 1;
            // update unit fields
            plr->SetArenaTeamInfoField(GetSlot(), 2, itr->games_week);
            plr->SetArenaTeamInfoField(GetSlot(), 3, itr->games_season);

            // add arena points
            // ap_calc.php
			if (sWorld.isEasyRealm())
				RewardArenaPoints(plr, itr->personal_rating, true);

            // quest 2v2
            if (Type == ARENA_TYPE_2v2)
                plr->KilledMonster(695007, 0);
        }
    }
}

void ArenaTeam::UpdateArenaPointsHelper(std::map<uint32, uint32>& PlayerPoints)
{
	// called after a match has ended and the stats are already modified
	// helper function for arena point distribution (this way, when distributing, no actual calculation is required, just a few comparisons)

	sLog.outLog(LOG_ARENA_FLUSH, "Team ID: %u has games_week: %u", GetId(), stats.games_week);

	// 10 played games per week is a minimum
	// 20 for solo 3v3
	if (Type == ARENA_TYPE_3v3 && stats.games_week < 15)
		return;
	else if (stats.games_week < 10)
		return;

	// to get points, a player has to participate in at least 30% of the matches
	uint32 min_plays = (uint32)ceil(stats.games_week * 0.3);
	for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
	{
		// the player participated in enough games, update his points
		uint32 points_to_add = 0;
		if (itr->games_week >= min_plays)
		{
			points_to_add = (GetPoints(itr->personal_rating) * sWorld.getConfig(CONFIG_ARENA_POINTS_RATE));
			uint32 init_ponts = points_to_add;

			// bonus is only for x100
			if (sWorld.isEasyRealm())
			{
				if (sWorld.getConfig(BONUS_RATES))
					points_to_add += init_ponts * 0.5;

				// this is insanely bad, but there's no other way to do this :O
				bool premium = false;
				Player* player = sObjectAccessor.GetPlayerOnline(itr->guid);
				if (player)
				{
					if (player->GetSession()->isPremium())
						premium = true;
				}
				else
				{
					QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT account FROM characters WHERE guid = %u", GUID_LOPART(itr->guid));
					if (result)
					{
						uint32 account = (*result)[0].GetUInt32();
						if (account)
						{
							QueryResultAutoPtr result1 = AccountsDatabase.PQuery("SELECT premium_until FROM account WHERE account_id = %u", account);
							if (result1)
							{
								uint32 until = (*result1)[0].GetUInt32();
								time_t curr = time(NULL);

								if (until && curr < until)
									premium = true;
							}
						}
					}
				}

				// CalculateBonus
				if (premium)
					points_to_add += init_ponts * 0.5;
			}
		}

		std::map<uint32, uint32>::iterator plr_itr = PlayerPoints.find(GUID_LOPART(itr->guid));
		if (plr_itr != PlayerPoints.end())
		{
			//check if there is already more points
			if (plr_itr->second < points_to_add)
				PlayerPoints[GUID_LOPART(itr->guid)] = points_to_add;
		}
		else
			PlayerPoints[GUID_LOPART(itr->guid)] = points_to_add;

		sLog.outLog(LOG_ARENA_FLUSH, "Player guid: %u added arena points (flush): %u", GUID_LOPART(itr->guid), points_to_add);
	}
}

void ArenaTeam::SaveToDB()
{
    // If a temp arena team return
    if (Id >= 0xFFF00000)
        return;

    static SqlStatementID updateATeam;
    static SqlStatementID updateAMembers;
    static SqlStatementID updateH2Rating;
    static SqlStatementID updateH3Rating;
    static SqlStatementID updateH5Rating;

    RealmDataDatabase.BeginTransaction();

    SqlStatement stmt = RealmDataDatabase.CreateStatement(updateATeam, "UPDATE arena_team_stats SET rating = ?, games = ?, played = ?, rank = ?, wins = ?, wins2 = ? WHERE arenateamid = ?");
    stmt.addUInt32(stats.rating);
    stmt.addUInt32(stats.games_week);
    stmt.addUInt32(stats.games_season);
    stmt.addUInt32(stats.rank);
    stmt.addUInt32(stats.wins_week);
    stmt.addUInt32(stats.wins_season);
    stmt.addUInt32(GetId());

    stmt.Execute();

    // save team and member stats to db
    // called after a match has ended, or when calculating arena_points
    for (MemberList::iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        stmt = RealmDataDatabase.CreateStatement(updateAMembers, "UPDATE arena_team_member SET played_week = ?, wons_week = ?, played_season = ?, wons_season = ?, personal_rating = ? WHERE arenateamid = ? AND guid = ?");

        stmt.addUInt32(itr->games_week);
        stmt.addUInt32(itr->wins_week);
        stmt.addUInt32(itr->games_season);
        stmt.addUInt32(itr->wins_season);
        stmt.addUInt32(itr->personal_rating);
        stmt.addUInt32(Id);
        stmt.addUInt32(itr->guid);

        stmt.Execute();
    }

    RealmDataDatabase.CommitTransaction();

    //// else it's a temp team, so we have to save the real one for each player

    //// Init some variables for speedup the programm
    //ArenaTeam* realTeams[SOLO_3v3_MIN_PLAYERS];
    //uint32 itrRealTeam = 0;
    //for (; itrRealTeam < SOLO_3v3_MIN_PLAYERS; itrRealTeam++)
    //    realTeams[itrRealTeam] = NULL;
    //itrRealTeam = 0;

    //uint32 oldRating = 0;

    //// get real teams
    //for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    //{
    //    ArenaTeam* plrArenaTeam = NULL;

    //    // Find real arena team for player
    //    for (UNORDERED_MAP<uint32, ArenaTeam*>::iterator itrMgr = sObjectMgr.GetArenaTeamMapBegin(); itrMgr != sObjectMgr.GetArenaTeamMapEnd(); itrMgr++)
    //    {
    //        if (itrMgr->first < 0xFFF00000 && itrMgr->second->CaptainGuid == itr->guid && itrMgr->second->Type == ARENA_TEAM_3v3)
    //        {
    //            plrArenaTeam = itrMgr->second; // found!
    //            break;
    //        }
    //    }

    //    if (!plrArenaTeam)
    //        continue; // Not found? Maybe player has left the game and deleted it before the arena game ends.

    //    ASSERT(itrRealTeam < SOLO_3v3_MIN_PLAYERS);
    //    realTeams[itrRealTeam++] = plrArenaTeam;
    //}

    //itrRealTeam = 0;

    //// Let's loop again through temp arena team and add the new rating
    //for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    //{
    //    ArenaTeam* plrArenaTeam = realTeams[itrRealTeam++];

    //    if (!plrArenaTeam)
    //        continue;

    //    for (MemberList::iterator realMemberItr = plrArenaTeam->members.begin(); realMemberItr != plrArenaTeam->members.end(); ++realMemberItr)
    //    {
    //        if (realMemberItr->guid == plrArenaTeam->GetCaptain())
    //        {
    //            realMemberItr->personal_rating = itr->personal_rating;
    //            realMemberItr->games_season = itr->games_season;
    //            realMemberItr->wins_season = itr->wins_season;
    //            realMemberItr->games_week = itr->games_week;
    //            realMemberItr->wins_week = itr->wins_week;

    //            // team rating == personal rating
    //            plrArenaTeam->stats.rating = itr->personal_rating;
    //        }
    //    }

    //    plrArenaTeam->stats.games_season = itr->games_season;
    //    plrArenaTeam->stats.wins_season = itr->wins_season;
    //    plrArenaTeam->stats.games_week = itr->games_week;
    //    plrArenaTeam->stats.wins_week = itr->wins_week;

    //    plrArenaTeam->SaveToDBHelper();
    //    plrArenaTeam->NotifyStatsChanged();
    //}
}

void ArenaTeam::FinishWeek()
{
    stats.games_week = 0;                                   // played this week
    stats.wins_week = 0;                                    // wins this week
    for (MemberList::iterator itr = members.begin(); itr !=  members.end(); ++itr)
    {
        itr->games_week = 0;
        itr->wins_week = 0;
    }
}

bool ArenaTeam::CantLeave() const
{
    for (MemberList::const_iterator itr = members.begin(); itr != members.end(); ++itr)
    {
        if (Player *p = sObjectAccessor.GetPlayerInWorld(itr->guid))
        {
            BattleGround* bg = p->GetBattleGround();            
            if (bg && bg->isArena() && bg->GetArenaType() == ARENA_TYPE_2v2)
                return true;

            // removed DANGEROUS?
            //|| p->InBattleGroundQueueForBattleGroundQueueType(BATTLEGROUND_QUEUE_2v2)
            //    || p->InBattleGroundQueueForBattleGroundQueueType(BATTLEGROUND_QUEUE_3v3)
            //    || p->InBattleGroundQueueForBattleGroundQueueType(BATTLEGROUND_QUEUE_5v5)
        }
    }
    return false;
}

/*
arenateam fields (id from 2.3.3 client):
1414 - arena team id 2v2
1415 - 0=captain, 1=member
1416 - played this week
1417 - played this season
1418 - unk - rank?
1419 - personal arena rating
1420 - arena team id 3v3
1421 - 0=captain, 1=member
1422 - played this week
1423 - played this season
1424 - unk - rank?
1425 - personal arena rating
1426 - arena team id 5v5
1427 - 0=captain, 1=member
1428 - played this week
1429 - played this season
1430 - unk - rank?
1431 - personal arena rating
*/
