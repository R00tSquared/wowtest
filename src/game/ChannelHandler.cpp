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

#include "ObjectMgr.h"                                      // for normalizePlayerName
#include "ChannelMgr.h"
#include "Chat.h"
#include "ChatLog.h"

void WorldSession::HandleChannelJoin(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 4+1+1+1);

    uint32 channel_id;
    uint8 unknown1, unknown2;
    std::string channelname, pass;

    recvPacket >> channel_id >> unknown1 >> unknown2;
    recvPacket >> channelname;

    if (channelname.empty())
        return;

    if (!processChatmessageFurtherAfterSecurityChecks(channelname, LANG_UNIVERSAL))
        return;

    if (ChatHandler(_player).ContainsNotAllowedSigns(channelname))
        return;

    std::string to = "Join_Channel";
    sChatLog.WhisperMsg(GetPlayer(), to, channelname);

    if (channelname == "Lexics Cutter Detected Bad Words")
        return;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, 4+1+1+(channelname.size()+1)+1);

    recvPacket >> pass;
    if(channel_id == 2 && sWorld.getConfig(CONFIG_GLOBAL_TRADE_CHANNEL)) //magic number - trade channel id from DBC
        channelname = "Trade";
    
    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetJoinChannel(channelname, channel_id))
        {
			chn->Join(_player->GetGUID(), pass.c_str());
			
			if (channelname != "Global")
			{
				StringToUpper(channelname);
				Channel *chn2 = nullptr;

				// if russian or ruSSian...
				if (channelname == "GLOBAL")
					chn2 = cMgr->GetJoinChannel("Global", 0);

				if (chn2)
				{
					chn->Leave(_player->GetGUID(), true);
					chn2->Invite(_player->GetGUID(), _player->GetName(), false);
				}
			}

            //if (channelname != "Russian" && channelname != "English")
            //{
            //    // because it was renamed
            //    //if (channelname == "Global")
            //    //{
            //    //    chn->Leave(_player->GetGUID(), true);
            //    //    Channel *chn2 = cMgr->GetJoinChannel("Russian", 0);
            //    //    chn2->Invite(_player->GetGUID(), _player->GetName(), false);
            //    //}
            //
			//	StringToUpper(channelname);
			//	Channel *chn2 = nullptr;
            //
			//	//// if russian or ruSSian...
			//	if (channelname == "RUSSIAN")
			//		chn2 = cMgr->GetJoinChannel("Russian", 0);
			//	else if (channelname == "ENGLISH")
			//		chn2 = cMgr->GetJoinChannel("English", 0);
            //
			//	if (chn2)
			//	{
			//		chn->Leave(_player->GetGUID(), true);
			//		chn2->Invite(_player->GetGUID(), _player->GetName(), false);
			//	}
            //}
        }
    }
}

void WorldSession::HandleChannelLeave(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 4+1);

    uint32 channel_id;
    std::string channelname;
    recvPacket >> channel_id;
    recvPacket >> channelname;

    if (channelname.empty())
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->Leave(_player->GetGUID(), true);
        cMgr->LeftChannel(channelname);
    }
}

void WorldSession::HandleChannelList(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string channelname;
    recvPacket >> channelname;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->List(_player);
}

void WorldSession::HandleChannelPassword(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, pass;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> pass;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->Password(_player->GetGUID(), pass.c_str());
}

void WorldSession::HandleChannelSetOwner(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, newp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> newp;

    if (!normalizePlayerName(newp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->SetOwner(_player->GetGUID(), newp.c_str());
}

void WorldSession::HandleChannelOwner(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string channelname;
    recvPacket >> channelname;
    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->SendWhoOwner(_player->GetGUID());
}

void WorldSession::HandleChannelModerator(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->SetModerator(_player->GetGUID(), otp.c_str());
}

void WorldSession::HandleChannelUnmoderator(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->UnsetModerator(_player->GetGUID(), otp.c_str());
}

void WorldSession::HandleChannelMute(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->SetMute(_player->GetGUID(), otp.c_str());
}

void WorldSession::HandleChannelUnmute(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->UnsetMute(_player->GetGUID(), otp.c_str());
}

void WorldSession::HandleChannelInvite(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;
    
    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
        {
            if (!sObjectAccessor.GetPlayerByNameInWorld(otp) || !sObjectAccessor.GetPlayerByNameInWorld(otp)->isAcceptWhispers()) // player not found
                {
                    WorldPacket data;
                    data.Initialize(SMSG_CHANNEL_NOTIFY, 1+channelname.size()+1);
                    data << uint8(0x09); //CHAT_PLAYER_NOT_FOUND_NOTICE
                    data << channelname;
                    data << otp;
                    SendPacket(&data);
                    return;
                }
            chn->Invite(_player->GetGUID(), otp.c_str());
        }
}

void WorldSession::HandleChannelKick(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;
    if (!normalizePlayerName(otp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->Kick(_player->GetGUID(), otp.c_str());
}

void WorldSession::HandleChannelBan(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->Ban(_player->GetGUID(), otp.c_str());
}

void WorldSession::HandleChannelUnban(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1+1);

    std::string channelname, otp;
    recvPacket >> channelname;

    // recheck
    CHECK_PACKET_SIZE(recvPacket, (channelname.size()+1)+1);

    recvPacket >> otp;

    if (!normalizePlayerName(otp))
        return;

    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->UnBan(_player->GetGUID(), otp.c_str());
}

void WorldSession::HandleChannelAnnounce(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string channelname;
    recvPacket >> channelname;
    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->Announce(_player->GetGUID());
}

void WorldSession::HandleChannelModerate(WorldPacket& recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string channelname;
    recvPacket >> channelname;
    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->Moderate(_player->GetGUID());
}

void WorldSession::HandleChannelRosterQuery(WorldPacket &recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string channelname;
    recvPacket >> channelname;
    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->List(_player);
}

void WorldSession::HandleChannelInfoQuery(WorldPacket &recvPacket)
{
    return; // disable all channels!
    
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string channelname;
    recvPacket >> channelname;
    if (ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
    {
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
        {
            uint32 showCnt = chn->GetNumPlayers();
            if (sWorld.getConfig(CONFIG_MAX_CHANNEL_SHOW) && chn->IsConstant() && showCnt > sWorld.getConfig(CONFIG_MAX_CHANNEL_SHOW))
                showCnt = sWorld.getConfig(CONFIG_MAX_CHANNEL_SHOW);

            WorldPacket data(SMSG_CHANNEL_MEMBER_COUNT, chn->GetName().size()+1+1+4);
            data << chn->GetName();
            data << uint8(chn->GetFlags());
            data << uint32(showCnt);
            SendPacket(&data);
        }
    }
}

void WorldSession::HandleChannelJoinNotify(WorldPacket &recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
    //recvPacket.hexlike();
    CHECK_PACKET_SIZE(recvPacket, 1);

    std::string channelname;
    recvPacket >> channelname;
    /*if(ChannelMgr* cMgr = channelMgr(_player->GetTeam()))
        if (Channel *chn = cMgr->GetChannel(channelname, _player))
            chn->JoinNotify(_player->GetGUID());*/
}

