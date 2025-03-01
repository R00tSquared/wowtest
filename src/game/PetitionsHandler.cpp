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

#include "Common.h"
#include "Language.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Guild.h"
#include "ArenaTeam.h"
#include "MapManager.h"
#include "GossipDef.h"
#include "SocialMgr.h"
#include "GuildMgr.h"
#include "PetitionsHandler.h"
#include "BattleGroundMgr.h"

void WorldSession::HandlePetitionBuyOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+8+4+1+5*8+2+1+4+4);

    sLog.outDebug("Received opcode CMSG_PETITION_BUY");
    //recv_data.hexlike();

    uint64 guidNPC;
    uint64 unk1, unk3, unk4, unk5, unk6, unk7;
    uint32 unk2;
    std::string name;
    uint16 unk8;
    uint8  unk9;
    uint32 unk10;                                           // selected index
    uint32 unk11;
    recv_data >> guidNPC;                                   // NPC GUID
    recv_data >> unk1;                                      // 0
    recv_data >> unk2;                                      // 0
    recv_data >> name;                                      // name

    // recheck
    CHECK_PACKET_SIZE(recv_data, 8+8+4+(name.size()+1)+5*8+2+1+4+4);

    recv_data >> unk3;                                      // 0
    recv_data >> unk4;                                      // 0
    recv_data >> unk5;                                      // 0
    recv_data >> unk6;                                      // 0
    recv_data >> unk7;                                      // 0
    recv_data >> unk8;                                      // 0
    recv_data >> unk9;                                      // 0
    recv_data >> unk10;                                     // index
    recv_data >> unk11;                                     // 0
    sLog.outDebug("Petitioner with GUID %u tried sell petition: name %s", GUID_LOPART(guidNPC), name.c_str());

    // prevent cheating
    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(guidNPC, UNIT_NPC_FLAG_PETITIONER);
    if (!pCreature)
    {
        sLog.outDebug("WORLD: HandlePetitionBuyOpcode - Unit (GUID: %u) not found or you can't interact with him.", GUID_LOPART(guidNPC));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    uint32 charterid = 0;
    uint32 cost = 0;
    uint32 type = PETITION_TYPE_NONE;
    if (pCreature->isTabardDesigner())
    {
        // if tabard designer, then trying to buy a guild charter.
        // do not let if already in guild.
        if (_player->GetGuildId())
            return;

        charterid = GUILD_CHARTER;
        cost = 1000; // 10 silver
        type = PETITION_TYPE_GUILD;
    }
    else
    {
        // TODO: find correct opcode
        if (_player->GetLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            SendNotification(LANG_ARENA_ONE_TOOLOW, sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL));
            return;
        }

        // allow to create only 2v2
        if (unk10 != 1)
            return;

        switch (unk10)
        {
            case 1:
                charterid = ARENA_TEAM_CHARTER_2v2;
                cost = 800000; // 80 g
                type = PETITION_TYPE_2v2;                                   // 2v2
                break;
            case 2:
                charterid = ARENA_TEAM_CHARTER_3v3;
                cost = 1200000; // 120 g
                type = PETITION_TYPE_3v3;                                   // 3v3
                break;
            case 3:
                charterid = ARENA_TEAM_CHARTER_5v5;
                cost = 2000000; // 200 g
                type = PETITION_TYPE_5v5;                                   // 5v5
                break;
            default:
                sLog.outDebug("unknown selection at buy petition: %u", unk10);
                return;
        }

        if (_player->GetArenaTeamId(unk10-1))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ALREADY_IN_ARENA_TEAM);
            return;
        }
    }

    if (type == PETITION_TYPE_GUILD)
    {
        if (sGuildMgr.GetGuildByName(name))
        {
            SendGuildCommandResult(GUILD_CREATE_S, name, GUILD_NAME_EXISTS);
            return;
        }
        if (!ObjectMgr::IsValidCharterName(name))
        {
            SendGuildCommandResult(GUILD_CREATE_S, name, GUILD_NAME_INVALID);
            return;
        }
    }
    else
    {
        if (sObjectMgr.GetArenaTeamByName(name))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ARENA_TEAM_NAME_EXISTS_S);
            return;
        }
        if (!ObjectMgr::IsValidCharterName(name))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ARENA_TEAM_NAME_INVALID);
            return;
        }
    }

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(charterid);
    if (!pProto)
    {
        _player->SendBuyError(BUY_ERR_CANT_FIND_ITEM, NULL, charterid, 0);
        return;
    }

    if (_player->GetMoney() < cost)
    {                                                       //player hasn't got enough money
        _player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, charterid, 0);
        return;
    }

    ItemPosCountVec dest;
    uint8 msg = _player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, charterid, pProto->BuyCount);
    if (msg != EQUIP_ERR_OK)
    {
        _player->SendBuyError(msg, pCreature, charterid, 0);
        return;
    }

    _player->ModifyMoney(-(int32)cost);
    Item *charter = _player->StoreNewItem(dest, charterid, true, 0, "PETITION");
    if (!charter)
        return;

    charter->SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1, charter->GetGUIDLow());
    // ITEM_FIELD_ENCHANTMENT_1_1 is guild/arenateam id
    // ITEM_FIELD_ENCHANTMENT_1_1+1 is current signatures count (showed on item)
    charter->SetState(ITEM_CHANGED, _player);
    _player->SendNewItem(charter, 1, true, false);

    // a petition is invalid, if both the owner and the type matches
    // we checked above, if this player is in an arenateam, so this must be
    // datacorruption
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT petitionguid FROM petition WHERE ownerguid = '%u'  AND type = '%u'", _player->GetGUIDLow(), type);

    std::ostringstream ssInvalidPetitionGUIDs;

    if (result)
    {

        do
        {
            Field *fields = result->Fetch();
            ssInvalidPetitionGUIDs << "'" << fields[0].GetUInt32() << "' , ";
        }
        while (result->NextRow());
    }

    // delete petitions with the same guid as this one
    ssInvalidPetitionGUIDs << "'" << charter->GetGUIDLow() << "'";

    sLog.outDebug("Invalid petition GUIDs: %s", ssInvalidPetitionGUIDs.str().c_str());
    RealmDataDatabase.escape_string(name);
    RealmDataDatabase.BeginTransaction();
    RealmDataDatabase.PExecute("DELETE FROM petition WHERE petitionguid IN (%s)",  ssInvalidPetitionGUIDs.str().c_str());
    RealmDataDatabase.PExecute("DELETE FROM petition_sign WHERE petitionguid IN (%s)", ssInvalidPetitionGUIDs.str().c_str());
    RealmDataDatabase.PExecute("INSERT INTO petition (ownerguid, petitionguid, name, type) VALUES ('%u', '%u', '%s', '%u')",
        _player->GetGUIDLow(), charter->GetGUIDLow(), name.c_str(), type);
    RealmDataDatabase.CommitTransaction();
}

void WorldSession::HandlePetitionShowSignOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

                                                            // ok
    sLog.outDebug("Received opcode CMSG_PETITION_SHOW_SIGNATURES");
    //recv_data.hexlike();

    uint8 signs = 0;
    uint64 petitionguid;
    recv_data >> petitionguid;                              // petition guid

    // solve (possible) some strange compile problems with explicit use GUID_LOPART(petitionguid) at some GCC versions (wrong code optimization in compiler?)
    uint32 petitionguid_low = GUID_LOPART(petitionguid);

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT type FROM petition WHERE petitionguid = '%u'", petitionguid_low);
    if (!result)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: any petition on server...");
        return;
    }
    Field *fields = result->Fetch();
    uint32 type = fields[0].GetUInt32();

    // if guild petition and has guild => error, return;
    if (type == PETITION_TYPE_GUILD && _player->GetGuildId())
        return;

    result = RealmDataDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitionguid_low);

    // result==NULL also correct in case no sign yet
    if (result)
        signs = result->GetRowCount();

    sLog.outDebug("CMSG_PETITION_SHOW_SIGNATURES petition entry: '%u'", petitionguid_low);

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+1+signs*12));
    data << petitionguid;                                   // petition guid
    data << _player->GetGUID();                             // owner guid
    data << petitionguid_low;                               // guild guid (in Trinity always same as GUID_LOPART(petitionguid)
    data << signs;                                          // sign's count

    for (uint8 i = 1; i <= signs; i++)
    {
        Field *fields2 = result->Fetch();
        uint64 plguid = fields2[0].GetUInt64();

        data << plguid;                                     // Player GUID
        data << (uint32)0;                                  // there 0 ...

        result->NextRow();
    }
    SendPacket(&data);
}

void WorldSession::HandlePetitionQueryOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4+8);

    sLog.outDebug("Received opcode CMSG_PETITION_QUERY");   // ok
    //recv_data.hexlike();

    uint32 guildguid;
    uint64 petitionguid;
    recv_data >> guildguid;                                 // in Trinity always same as GUID_LOPART(petitionguid)
    recv_data >> petitionguid;                              // petition guid
    sLog.outDebug("CMSG_PETITION_QUERY Petition GUID %u Guild GUID %u", GUID_LOPART(petitionguid), guildguid);

    SendPetitionQueryOpcode(petitionguid);
}

void WorldSession::SendPetitionQueryOpcode(uint64 petitionguid)
{
    uint64 ownerguid = 0;
    uint32 type;
    std::string name = "NO_NAME_FOR_GUID";
    uint8 signs = 0;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery(
        "SELECT ownerguid, name, "
        "  (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = '%u') AS signs, "
        "  type "
        "FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid), GUID_LOPART(petitionguid));

    if (result)
    {
        Field* fields = result->Fetch();
        ownerguid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        name      = fields[1].GetCppString();
        signs     = fields[2].GetUInt8();
        type      = fields[3].GetUInt32();
    }
    else
    {
        sLog.outDebug("CMSG_PETITION_QUERY failed for petition (GUID: %u)", GUID_LOPART(petitionguid));
        return;
    }

    WorldPacket data(SMSG_PETITION_QUERY_RESPONSE, (4+8+name.size()+1+1+4*13));
    data << GUID_LOPART(petitionguid);                      // guild/team guid (in Trinity always same as GUID_LOPART(petition guid)
    data << ownerguid;                                      // charter owner guid
    data << name;                                           // name (guild/arena team)
    data << uint8(0);                                       // 1
    if (type == PETITION_TYPE_GUILD)
    {
        data << uint32(9);
        data << uint32(9);
        data << uint32(0);                                  // bypass client - side limitation, a different value is needed here for each petition
    }
    else
    {
        data << type-1;
        data << type-1;
        data << type;                                       // bypass client - side limitation, a different value is needed here for each petition
    }
    data << uint32(0);                                      // 5
    data << uint32(0);                                      // 6
    data << uint32(0);                                      // 7
    data << uint32(0);                                      // 8
    data << uint16(0);                                      // 9 2 bytes field
    data << uint32(0);                                      // 10
    data << uint32(0);                                      // 11
    data << uint32(0);                                      // 13 count of next strings?
    data << uint32(0);                                      // 14
    if (type == 9)
        data << uint32(0);                                  // 15 0 - guild, 1 - arena team
    else
        data << uint32(1);
    SendPacket(&data);
}

void WorldSession::HandlePetitionRenameOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+1);

    sLog.outDebug("Received opcode MSG_PETITION_RENAME");   // ok
    //recv_data.hexlike();

    uint64 petitionguid;
    uint32 type;
    std::string newname;

    recv_data >> petitionguid;                              // guid
    recv_data >> newname;                                   // new name

    Item *item = _player->GetItemByGuid(petitionguid);
    if (!item)
        return;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT type FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));

    if (result)
    {
        Field* fields = result->Fetch();
        type = fields[0].GetUInt32();
    }
    else
    {
        sLog.outDebug("CMSG_PETITION_QUERY failed for petition (GUID: %u)", GUID_LOPART(petitionguid));
        return;
    }

    if (type == PETITION_TYPE_GUILD)
    {
        if (sGuildMgr.GetGuildByName(newname))
        {
            SendGuildCommandResult(GUILD_CREATE_S, newname, GUILD_NAME_EXISTS);
            return;
        }
        if (!ObjectMgr::IsValidCharterName(newname))
        {
            SendGuildCommandResult(GUILD_CREATE_S, newname, GUILD_NAME_INVALID);
            return;
        }
    }
    else
    {
        if (sObjectMgr.GetArenaTeamByName(newname))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, newname, "", ERR_ARENA_TEAM_NAME_EXISTS_S);
            return;
        }
        if (!ObjectMgr::IsValidCharterName(newname))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, newname, "", ERR_ARENA_TEAM_NAME_INVALID);
            return;
        }
    }

    std::string db_newname = newname;
    RealmDataDatabase.escape_string(db_newname);
    RealmDataDatabase.PExecute("UPDATE petition SET name = '%s' WHERE petitionguid = '%u'",
        db_newname.c_str(), GUID_LOPART(petitionguid));

    sLog.outDebug("Petition (GUID: %u) renamed to '%s'", GUID_LOPART(petitionguid), newname.c_str());
    WorldPacket data(MSG_PETITION_RENAME, (8+newname.size()+1));
    data << petitionguid;
    data << newname;
    SendPacket(&data);
}

void WorldSession::HandlePetitionSignOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+1);

    sLog.outDebug("Received opcode CMSG_PETITION_SIGN");    // ok
    //recv_data.hexlike();

    Field *fields;
    uint64 petitionguid;
    uint8 unk;
    recv_data >> petitionguid;                              // petition guid
    recv_data >> unk;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery(
        "SELECT ownerguid, "
        "  (SELECT COUNT(playerguid) FROM petition_sign WHERE petition_sign.petitionguid = '%u') AS signs, "
        "  type "
        "FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid), GUID_LOPART(petitionguid));

    if (!result)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: any petition on server...");
        return;
    }

    fields = result->Fetch();
    uint64 ownerguid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
    uint8 signs = fields[1].GetUInt8();
    uint32 type = fields[2].GetUInt32();

    uint32 plguidlo = _player->GetGUIDLow();
    if (GUID_LOPART(ownerguid) == plguidlo)
        return;

    bool AllowTwoSideArena = sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_ARENA) && !BattleGroundMgr::IsArenaTypeRestrictedPVE(type);

    // check guild
    if (((type != PETITION_TYPE_GUILD && !AllowTwoSideArena) || ((type == PETITION_TYPE_GUILD) && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD)))
        && GetPlayer()->GetTeam() != sObjectMgr.GetPlayerTeamByGUID(ownerguid))
    {
        if (type != PETITION_TYPE_GUILD)
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", "", ERR_ARENA_TEAM_NOT_ALLIED);
        else
            SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_NOT_ALLIED);
        return;
    }    

    if (((type != PETITION_TYPE_GUILD && !AllowTwoSideArena) || ((type == PETITION_TYPE_GUILD) && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD)))
        && ((sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION) && GetPlayer()->InBattleGroundOrArena())))
    {
        if (GetPlayer()->TeamForRace(GetPlayer()->GetRace()) != sObjectMgr.GetPlayerTeamByGUID(ownerguid))
        {
            if (type != PETITION_TYPE_GUILD)
                SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", "", ERR_ARENA_TEAM_NOT_ALLIED);
            else
                SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_NOT_ALLIED);
        }
    }

    if (type != PETITION_TYPE_GUILD)
    {
        if (_player->GetLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, "", _player->GetName(), ERR_ARENA_TEAM_PLAYER_TO_LOW);
            return;
        }

        uint8 slot = ArenaTeam::GetSlotByType(type);
        if (slot >= MAX_ARENA_SLOT)
            return;

        if (_player->GetArenaTeamId(slot))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", _player->GetName(), ERR_ALREADY_IN_ARENA_TEAM_S);
            return;
        }

        if (_player->GetArenaTeamIdInvited())
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", _player->GetName(), ERR_ALREADY_INVITED_TO_ARENA_TEAM_S);
            return;
        }
    }
    else
    {
        if (_player->GetGuildId())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_IN_GUILD);
            return;
        }
        if (_player->GetGuildIdInvited())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_INVITED_TO_GUILD);
            return;
        }
    }

    if (++signs > type)                                        // client signs maximum
        return;

    //client doesn't allow to sign petition two times by one character, but not check sign by another character from same account
    //not allow sign another player from already sign player account
    result = RealmDataDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE player_account = '%u' AND petitionguid = '%u'", GetAccountId(), GUID_LOPART(petitionguid));

    if (result)
    {
        WorldPacket data(SMSG_PETITION_SIGN_RESULTS, (8+8+4));
        data << petitionguid;
        data << _player->GetGUID();
        data << (uint32)PETITION_SIGN_ALREADY_SIGNED;

        // close at signer side
        SendPacket(&data);

        // update for owner if online
        if (Player *owner = sObjectMgr.GetPlayerInWorld(ownerguid))
            owner->SendPacketToSelf(&data);
        return;
    }

    RealmDataDatabase.PExecute("INSERT IGNORE INTO petition_sign (ownerguid,petitionguid, playerguid, player_account, type) VALUES ('%u', '%u', '%u', '%u', '%u')", GUID_LOPART(ownerguid),GUID_LOPART(petitionguid), plguidlo, GetAccountId(), type);

    sLog.outDebug("PETITION SIGN: GUID %u by player: %s (GUID: %u Account: %u)", GUID_LOPART(petitionguid), _player->GetName(),plguidlo,GetAccountId());

    WorldPacket data(SMSG_PETITION_SIGN_RESULTS, (8+8+4));
    data << petitionguid;
    data << _player->GetGUID();
    data << (uint32)PETITION_SIGN_OK;

    // close at signer side
    SendPacket(&data);

    // update signs count on charter, required testing...
    //Item *item = _player->GetItemByGuid(petitionguid));
    //if(item)
    //    item->SetUInt32Value(ITEM_FIELD_ENCHANTMENT_1_1+1, signs);

    // update for owner if online
    if (Player *owner = sObjectMgr.GetPlayerInWorld(ownerguid))
        owner->SendPacketToSelf(&data);
}

void WorldSession::HandlePetitionDeclineOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    sLog.outDebug("Received opcode MSG_PETITION_DECLINE");  // ok
    //recv_data.hexlike();

    uint64 petitionguid;
    uint64 ownerguid;
    recv_data >> petitionguid;                              // petition guid
    sLog.outDebug("Petition %u declined by %u", GUID_LOPART(petitionguid), _player->GetGUIDLow());

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT ownerguid FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    if (!result)
        return;

    Field *fields = result->Fetch();
    ownerguid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);

    Player *owner = sObjectMgr.GetPlayerInWorld(ownerguid);
    if (owner)                                               // petition owner online
    {
        WorldPacket data(MSG_PETITION_DECLINE, 8);
        data << _player->GetGUID();
        owner->SendPacketToSelf(&data);
    }
}

void WorldSession::HandleOfferPetitionOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 4+8+8);

    sLog.outDebug("Received opcode CMSG_OFFER_PETITION");   // ok
    //recv_data.hexlike();

    uint8 signs = 0;
    uint64 petitionguid, plguid;
    uint32 type, junk;
    Player *player;
    recv_data >> junk;                                      // this is not petition type!
    recv_data >> petitionguid;                              // petition guid
    recv_data >> plguid;                                    // player guid

    player = ObjectAccessor::GetPlayerInWorld(plguid);
    if (!player)
        return;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT type FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    if (!result)
        return;

    Field *fields = result->Fetch();
    type = fields[0].GetUInt32();

    sLog.outDebug("OFFER PETITION: type %u, GUID1 %u, to player id: %u", type, GUID_LOPART(petitionguid), GUID_LOPART(plguid));

    bool AllowTwoSideArena = sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_ARENA) && !BattleGroundMgr::IsArenaTypeRestrictedPVE(type);

    if (((type != PETITION_TYPE_GUILD && !AllowTwoSideArena) || ((type == PETITION_TYPE_GUILD) && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD)))
        && GetPlayer()->GetTeam() != player->GetTeam())
    {
        if (type != PETITION_TYPE_GUILD)
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", "", ERR_ARENA_TEAM_NOT_ALLIED);
        else
            SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_NOT_ALLIED);
        return;
    }

    if (((type != PETITION_TYPE_GUILD && !AllowTwoSideArena) || ((type == PETITION_TYPE_GUILD) && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD)))
        && ((sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION) && (GetPlayer()->InBattleGroundOrArena() || player->InBattleGroundOrArena()))))
    {
        if (GetPlayer()->TeamForRace(GetPlayer()->GetRace()) != player->TeamForRace(player->GetRace()))
        {
            if (type != PETITION_TYPE_GUILD)
                SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", "", ERR_ARENA_TEAM_NOT_ALLIED);
            else
                SendGuildCommandResult(GUILD_CREATE_S, "", GUILD_NOT_ALLIED);
        }
    }

    if (type != PETITION_TYPE_GUILD)
    {
        if (player->GetLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            // player is too low level to join an arena team
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, player->GetName(), "", ERR_ARENA_TEAM_PLAYER_TO_LOW);
            return;
        }

        uint8 slot = ArenaTeam::GetSlotByType(type);
        if (slot >= MAX_ARENA_SLOT)
            return;

        if (player->GetArenaTeamId(slot))
        {
            // player is already in an arena team
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, player->GetName(), "", ERR_ALREADY_IN_ARENA_TEAM_S);
            return;
        }

        if (player->GetArenaTeamIdInvited())
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_INVITE_SS, "", _player->GetName(), ERR_ALREADY_INVITED_TO_ARENA_TEAM_S);
            return;
        }
    }
    else
    {
        if (player->GetGuildId())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_IN_GUILD);
            return;
        }

        if (player->GetGuildIdInvited())
        {
            SendGuildCommandResult(GUILD_INVITE_S, _player->GetName(), ALREADY_INVITED_TO_GUILD);
            return;
        }
    }

    result = RealmDataDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    // result==NULL also correct charter without signs
    if (result)
        signs = result->GetRowCount();

    WorldPacket data(SMSG_PETITION_SHOW_SIGNATURES, (8+8+4+signs+signs*12));
    data << petitionguid;                                   // petition guid
    data << _player->GetGUID();                             // owner guid
    data << GUID_LOPART(petitionguid);                      // guild guid (in Trinity always same as GUID_LOPART(petition guid)
    data << signs;                                          // sign's count

    for (uint8 i = 1; i <= signs; i++)
    {
        Field *fields2 = result->Fetch();
        uint64 plguid = fields2[0].GetUInt64();

        data << plguid;                                     // Player GUID
        data << (uint32)0;                                  // there 0 ...

        result->NextRow();
    }

    player->SendPacketToSelf(&data);
}

void WorldSession::HandleTurnInPetitionOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    sLog.outDebug("Received opcode CMSG_TURN_IN_PETITION"); // ok
    //recv_data.hexlike();

    WorldPacket data;
    uint64 petitionguid;

    uint32 ownerguidlo;
    uint32 type;
    std::string name;

    recv_data >> petitionguid;

    sLog.outDebug("Petition %u turned in by %u", GUID_LOPART(petitionguid), _player->GetGUIDLow());

    // data
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT ownerguid, name, type FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    if (result)
    {
        Field *fields = result->Fetch();
        ownerguidlo = fields[0].GetUInt32();
        name = fields[1].GetCppString();
        type = fields[2].GetUInt32();
    }
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: petition table has broken data!");
        return;
    }

    if (type == PETITION_TYPE_GUILD)
    {
        if (_player->GetGuildId())
        {
            data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
            data << (uint32)PETITION_TURN_ALREADY_IN_GUILD; // already in guild
            _player->SendPacketToSelf(&data);
            return;
        }
    }
    else
    {
        uint8 slot = ArenaTeam::GetSlotByType(type);
        if (slot >= MAX_ARENA_SLOT)
            return;

        if (_player->GetArenaTeamId(slot))
        {
            //data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
            //data << (uint32)PETITION_TURN_ALREADY_IN_GUILD;                          // already in guild
            //_player->BroadcastPacketToSelf(&data);
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ALREADY_IN_ARENA_TEAM);
            return;
        }
    }

    if (_player->GetGUIDLow() != ownerguidlo)
        return;

    // signs
    uint8 signs;
    result = RealmDataDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    if (result)
        signs = result->GetRowCount();
    else
        signs = 0;

    uint8 count;
    if (type == PETITION_TYPE_GUILD)
        count = sWorld.getConfig(CONFIG_MIN_PETITION_SIGNS);
    else
        count = type - 1;
    if (signs < count)
    {
        data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
        data << (uint32)PETITION_TURN_NEED_MORE_SIGNATURES; // need more signatures...
        SendPacket(&data);
        return;
    }

    if (type == PETITION_TYPE_GUILD)
    {
        if (sGuildMgr.GetGuildByName(name))
        {
            SendGuildCommandResult(GUILD_CREATE_S, name, GUILD_NAME_EXISTS);
            return;
        }
    }
    else
    {
        if (sObjectMgr.GetArenaTeamByName(name))
        {
            SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, name, "", ERR_ARENA_TEAM_NAME_EXISTS_S);
            return;
        }
    }

    // and at last charter item check
    Item *item = _player->GetItemByGuid(petitionguid);
    if (!item)
        return;

    // OK!

    // delete charter item
    _player->DestroyItem(item->GetBagSlot(),item->GetSlot(), true);

    if (type == PETITION_TYPE_GUILD)                                           // create guild
    {
        Guild* guild = new Guild;
        if (!guild->create(_player->GetGUID(), name))
        {
            delete guild;
            return;
        }

        // register guild and add guildmaster
        sGuildMgr.AddGuild(guild);

        // add members
        for (uint8 i = 0; i < signs; ++i)
        {
            Field* fields = result->Fetch();
            guild->AddMember(fields[0].GetUInt64(), guild->GetLowestRank());
            result->NextRow();
        }
    }
    else                                                    // or arena team
    {
        ArenaTeam* at = new ArenaTeam;
        if (!at->Create(_player->GetGUID(), type, name))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: PetitionsHandler: arena team create failed.");
            delete at;
            return;
        }

        CHECK_PACKET_SIZE(recv_data, 8+5*4);
        uint32 icon, iconcolor, border, bordercolor, backgroud;
        recv_data >> backgroud >> icon >> iconcolor >> border >> bordercolor;

        at->SetEmblem(backgroud, icon, iconcolor, border, bordercolor);

        // register team and add captain
        sObjectMgr.AddArenaTeam(at);
        sLog.outDebug("PetitonsHandler: arena team added to objmrg");

        // add members
        for (uint8 i = 0; i < signs; ++i)
        {
            Field* fields = result->Fetch();
            uint64 memberGUID = fields[0].GetUInt64();
            sLog.outDebug("PetitionsHandler: adding arena member %u", GUID_LOPART(memberGUID));
            at->AddMember(memberGUID);
            result->NextRow();
        }
    }

    RealmDataDatabase.BeginTransaction();
    RealmDataDatabase.PExecute("DELETE FROM petition WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    RealmDataDatabase.PExecute("DELETE FROM petition_sign WHERE petitionguid = '%u'", GUID_LOPART(petitionguid));
    RealmDataDatabase.CommitTransaction();

    // created
    sLog.outDebug("TURN IN PETITION GUID %u", GUID_LOPART(petitionguid));

    data.Initialize(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data << (uint32)PETITION_TURN_OK;
    SendPacket(&data);
}

void WorldSession::HandlePetitionShowListOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    sLog.outDebug("Received CMSG_PETITION_SHOWLIST");       // ok
    //recv_data.hexlike();

    uint64 guid;
    recv_data >> guid;

    SendPetitionShowList(guid);
}

void WorldSession::SendPetitionShowList(uint64 guid)
{
    Creature *pCreature = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_PETITIONER);
    if (!pCreature)
    {
        sLog.outDebug("WORLD: HandlePetitionShowListOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    uint8 count = 0;
    if (pCreature->isTabardDesigner())
        count = 1;
    else
        count = 3;

    WorldPacket data(SMSG_PETITION_SHOWLIST, 8+1+4*6);
    data << guid;                                           // npc guid
    data << count;                                          // count
    if (count == 1)
    {
        data << uint32(1);                                  // index
        data << uint32(GUILD_CHARTER);                      // charter entry
        data << uint32(16161);                              // charter display id
        uint32 GUILD_CHARTER_COST;
        GUILD_CHARTER_COST = 1000; // 10 silver
        data << uint32(GUILD_CHARTER_COST);                 // charter cost
        data << uint32(0);                                  // unknown
        data << uint32(9);                                  // required signs?
    }
    else
    {
        // 2v2
        data << uint32(1);                                  // index
        data << uint32(ARENA_TEAM_CHARTER_2v2);             // charter entry
        data << uint32(16161);                              // charter display id
        data << uint32(800000);                             // charter cost
        data << uint32(2);                                  // unknown
        data << uint32(2);                                  // required signs?
        // 3v3
        data << uint32(2);                                  // index
        data << uint32(ARENA_TEAM_CHARTER_3v3);             // charter entry
        data << uint32(16161);                              // charter display id
        data << uint32(1200000);                            // charter cost
        data << uint32(3);                                  // unknown
        data << uint32(3);                                  // required signs?
        // 5v5
        data << uint32(3);                                  // index
        data << uint32(ARENA_TEAM_CHARTER_5v5);             // charter entry
        data << uint32(16161);                              // charter display id
        data << uint32(2000000);                            // charter cost
        data << uint32(5);                                  // unknown
        data << uint32(5);                                  // required signs?
    }
    //for (uint8 i = 0; i < count; i++)
    //{
    //    data << uint32(i);                      // index
    //    data << uint32(GUILD_CHARTER);          // charter entry
    //    data << uint32(16161);                  // charter display id
    //    data << uint32(GUILD_CHARTER_COST+i);   // charter cost
    //    data << uint32(0);                      // unknown
    //    data << uint32(9);                      // required signs?
    //}
    SendPacket(&data);
    sLog.outDebug("Sent SMSG_PETITION_SHOWLIST");
}

