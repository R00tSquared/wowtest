// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005,2006,2007 MaNGOS <http://www.mangosproject.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "ChatLog.h"
#include "Chat.h"
#include "Group.h"
#include "Guild.h"
#include "ObjectMgr.h"
#include "SpellAuras.h"
//#include "ace/SingletonImp.h"
#include "Config/Config.h"
#include "Language.h"
#include "GuildMgr.h"

//INSTANTIATE_SINGLETON_1( ChatLog );

ChatLog::ChatLog()
{
    for (int i = 0; i <= CHATLOG_CHAT_TYPES_COUNT - 1; i++)
    {
        names[i] = "";
        files[i] = NULL;
    }

    Initialize();
}

ChatLog::~ChatLog()
{
    // close all files (avoiding double-close)
    CloseAllFiles();
}

void ChatLog::Initialize()
{
    // determine, if the chat logs are enabled
    ChatLogEnable = sConfig.GetBoolDefault("ChatLogEnable", false);
    ChatLogDateSplit = sConfig.GetBoolDefault("ChatLogDateSplit", false);
    ChatLogUTFHeader = sConfig.GetBoolDefault("ChatLogUTFHeader", false);
    ChatLogIgnoreUnprintable = sConfig.GetBoolDefault("ChatLogIgnoreUnprintable", false);

    if (ChatLogEnable)
    {
        // read chat log file names
        names[CHAT_LOG_CHAT] = sConfig.GetStringDefault("ChatLogChatFile", "");

        // read screen log flags
        screenflag[CHAT_LOG_CHAT] = sConfig.GetBoolDefault("ChatLogChatScreen", false);
    }

    // open all files (with aliasing)
    OpenAllFiles();

    // write timestamps (init)
    WriteInitStamps();
}

bool ChatLog::_ChatCommon(int ChatType, Player *player, std::string &msg)
{
    if (!ChatLogEnable) return(false);

    if (ChatLogIgnoreUnprintable)
    {
        // have to ignore unprintables, verify string by UTF8 here
        unsigned int pos = 0;
        std::string lchar;
        while (LexicsCutter::ReadUTF8(msg, lchar, pos))
        {
            if (lchar.size() == 1)
            {
                if (lchar[0] < ' ') return(false); // unprintable detected
            }
        }
    }

    return(true);
}

void ChatLog::ChatMsg(Player *player, std::string &msg, uint32 type)
{
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg)) return;

    CheckDateSwitch();

    std::string log_str = "";

    switch (type)
    {
        case CHAT_MSG_EMOTE:
        log_str.append("{EMOTE} ");
        break;

        case CHAT_MSG_YELL:
        log_str.append("{YELL} ");
        break;
    }

    log_str.append("[");
    log_str.append(player->GetName());
    log_str.append("] ");

    log_str.append(msg);

    if (uint32 instId = player->GetInstanciableInstanceId()) // if instance -> copy to raid_actions log
        sLog.outLog(LOG_RAID_ACTIONS, "#%u %s", instId, log_str.c_str());

    log_str.append("\n");

    if (screenflag[CHAT_LOG_CHAT]) printf("%s", log_str.c_str());
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", log_str.c_str());
        fflush(files[CHAT_LOG_CHAT]);
    }
}

void ChatLog::PartyMsg(Player *player, std::string &msg)
{
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg)) return;

    CheckDateSwitch();

    std::string log_str = "";

    log_str.append("[");
    log_str.append(player->GetName());
    log_str.append("]->GROUP:");

    Group *group = player->GetGroup();
    if (!group)
    {
        log_str.append("[unknown group] ");
    }
    else
    {
        // obtain group information
        log_str.append("[");

        uint8 gm_count = group->GetMembersCount();
        uint8 gm_count_m1 = gm_count - 1;
        uint64 gm_leader_GUID = group->GetLeaderGUID();
        Player *gm_member;

        gm_member = sObjectMgr.GetPlayerInWorld(gm_leader_GUID);
        if (gm_member)
        {
            log_str.append(gm_member->GetName());
            log_str.append(",");
        }

        Group::MemberSlotList g_members = group->GetMemberSlots();

        for (Group::member_citerator itr = g_members.begin(); itr != g_members.end(); itr++)
        {
            if (itr->guid == gm_leader_GUID) continue;

            gm_member = sObjectMgr.GetPlayerInWorld(itr->guid);
            if (gm_member)
            {
                log_str.append(itr->name);
                log_str.append(",");
            }
        }

        log_str.erase(log_str.length() - 1);
        log_str.append("] ");
    }

    log_str.append(msg);

    if (uint32 instId = player->GetInstanciableInstanceId()) // if instance -> copy to raid_actions log
        sLog.outLog(LOG_RAID_ACTIONS, "#%u %s", instId, log_str.c_str());

    log_str.append("\n");

    if (screenflag[CHAT_LOG_CHAT]) printf("%s", log_str.c_str());
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", log_str.c_str());
        fflush(files[CHAT_LOG_CHAT]);
    }
}

void ChatLog::GuildMsg(Player *player, std::string &msg, bool officer)
{
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg)) return;

    CheckDateSwitch();

    std::string log_str = "";

    log_str.append("[");
    log_str.append(player->GetName());
    log_str.append((officer ? "]->GUILD_OFF:" : "]->GUILD:"));

    if (!player->GetGuildId())
    {
        log_str.append("[unknown guild] ");
    }
    else
    {
        Guild *guild = sGuildMgr.GetGuildById(player->GetGuildId());
        if (!guild)
        {
            log_str.append("[unknown guild] ");
        }
        else
        {
            // obtain guild information
            log_str.append("(");
            log_str.append(guild->GetName());
            log_str.append(") ");
        }
    }

    log_str.append(msg);

    log_str.append("\n");

    if (screenflag[CHAT_LOG_CHAT]) printf("%s", log_str.c_str());
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", log_str.c_str());
        fflush(files[CHAT_LOG_CHAT]);
    }
}

void ChatLog::WhisperMsg(Player *player, std::string &to, std::string &msg)
{
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg)) return;

    CheckDateSwitch();

    std::string log_str = "";

    log_str.append("[");
    log_str.append(player->GetName());
    log_str.append("]->");

    if (to.size() == 0)
    {
        log_str.append("[???] ");
    }
    else
    {
        normalizePlayerName(to);
        log_str.append("[");
        log_str.append(to);
        log_str.append("] ");
    }

    log_str.append(msg);

    log_str.append("\n");

	if (to != "Join_channel" && to != "Afk")
	{
		if (screenflag[CHAT_LOG_CHAT]) printf("%s", log_str.c_str());
		if (files[CHAT_LOG_CHAT])
		{
			OutTimestamp(files[CHAT_LOG_CHAT]);
			fprintf(files[CHAT_LOG_CHAT], "%s", log_str.c_str());
			fflush(files[CHAT_LOG_CHAT]);
		}
	}
}

void ChatLog::ChannelMsg(Player *player, std::string &channel, std::string &msg, bool bad_lexics)
{
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg)) return;

    CheckDateSwitch();

    std::string log_str = "";

    log_str.append("[");
    log_str.append(player->GetName());
    log_str.append("]->CHANNEL:");

    if (channel.size() == 0)
    {
        log_str.append("[unknown channel] ");
    }
    else
    {
        log_str.append("[");
        log_str.append(channel);
        log_str.append("] ");
    }

    log_str.append(msg);

    if (!bad_lexics)
	    sLog.outLog(LOG_DISCORD, "%s", log_str.c_str());

    if (uint32 instId = player->GetInstanciableInstanceId()) // if instance -> copy to raid_actions log
        sLog.outLog(LOG_RAID_ACTIONS, "#%u %s", instId, log_str.c_str());

    log_str.append("\n");

    if (screenflag[CHAT_LOG_CHAT]) printf("%s", log_str.c_str());
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", log_str.c_str());
        fflush(files[CHAT_LOG_CHAT]);
    }
}

void ChatLog::RaidMsg(Player *player, std::string &msg, uint32 type)
{
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg)) return;

    CheckDateSwitch();

    std::string log_str = "";

    log_str.append("[");
    log_str.append(player->GetName());

    switch (type)
    {
        case CHAT_MSG_RAID:
        log_str.append("]->RAID:");
        break;

        case CHAT_MSG_RAID_LEADER:
        log_str.append("]->RAID_LEADER:");
        break;

        case CHAT_MSG_RAID_WARNING:
        log_str.append("]->RAID_WARN:");
        break;

        default:
        log_str.append("]->RAID_UNKNOWN:");
    }

    Group *group = player->GetGroup();
    if (!group)
    {
        log_str.append("[unknown raid] ");
    }
    else
    {
        // obtain group information
        log_str.append("[");

        uint8 gm_count = group->GetMembersCount();
        uint8 gm_count_m1 = gm_count - 1;
        uint64 gm_leader_GUID = group->GetLeaderGUID();
        Player *gm_member;

        gm_member = sObjectMgr.GetPlayerInWorld(gm_leader_GUID);
        if (gm_member)
        {
            log_str.append(gm_member->GetName());
            log_str.append(",");
        }

        Group::MemberSlotList g_members = group->GetMemberSlots();

        for (Group::member_citerator itr = g_members.begin(); itr != g_members.end(); itr++)
        {
            if (itr->guid == gm_leader_GUID) continue;

            gm_member = sObjectMgr.GetPlayerInWorld(itr->guid);
            if (gm_member)
            {
                log_str.append(itr->name);
                log_str.append(",");
            }
        }

        log_str.erase(log_str.length() - 1);
        log_str.append("] ");
    }

    log_str.append(msg);

    if (uint32 instId = player->GetInstanciableInstanceId()) // if instance -> copy to raid_actions log
        sLog.outLog(LOG_RAID_ACTIONS, "#%u %s", instId, log_str.c_str());

    log_str.append("\n");

    if (screenflag[CHAT_LOG_CHAT]) printf("%s", log_str.c_str());
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", log_str.c_str());
        fflush(files[CHAT_LOG_CHAT]);
    }
}

void ChatLog::BattleGroundMsg(Player *player, std::string &msg, uint32 type)
{
    if (!_ChatCommon(CHAT_LOG_CHAT, player, msg)) return;

    CheckDateSwitch();

    std::string log_str = "";

    log_str.append("[");
    log_str.append(player->GetName());

    switch (type)
    {
        case CHAT_MSG_BATTLEGROUND:
        log_str.append("]->BG:");
        break;

        case CHAT_MSG_BATTLEGROUND_LEADER:
        log_str.append("]->BG_LEADER:");
        break;

        default:
        log_str.append("]->BG_UNKNOWN:");
    }

    Group *group = player->GetGroup();
    if (!group)
    {
        log_str.append("[unknown group] ");
    }
    else
    {
        // obtain group information
        log_str.append("[");

        uint8 gm_count = group->GetMembersCount();
        uint8 gm_count_m1 = gm_count - 1;
        uint64 gm_leader_GUID = group->GetLeaderGUID();
        Player *gm_member;

        gm_member = sObjectMgr.GetPlayerInWorld(gm_leader_GUID);
        if (gm_member)
        {
            log_str.append(gm_member->GetName());
            log_str.append(",");
        }

        Group::MemberSlotList g_members = group->GetMemberSlots();

        for (Group::member_citerator itr = g_members.begin(); itr != g_members.end(); itr++)
        {
            if (itr->guid == gm_leader_GUID) continue;

            gm_member = sObjectMgr.GetPlayerInWorld(itr->guid);
            if (gm_member)
            {
                log_str.append(itr->name);
                log_str.append(",");
            }
        }

        log_str.erase(log_str.length() - 1);
        log_str.append("] ");
    }

    log_str.append(msg);

    log_str.append("\n");

    if (screenflag[CHAT_LOG_CHAT]) printf("%s", log_str.c_str());
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", log_str.c_str());
        fflush(files[CHAT_LOG_CHAT]);
    }
}

void ChatLog::OpenAllFiles()
{
    std::string tempname;
    char dstr[12];

    if (ChatLogDateSplit)
    {
        time_t t = time(NULL);
        tm* aTm = localtime(&t);
        sprintf(dstr, "%-4d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday);
    }

    if (ChatLogEnable)
    {
        for (int i = 0; i <= CHATLOG_CHAT_TYPES_COUNT - 1; i++)
        {
            if (names[i] != "")
            {
                for (int j = i - 1; j >= 0; j--)
                {
                    if (names[i] == names[j])
                    {
                        files[i] = files[j];
                        break;
                    }
                }
                if (!files[i])
                {
                    tempname = names[i];
                    if (ChatLogDateSplit)
                    {
                        // append date instead of $d if applicable
                        int dpos = tempname.find("$d");
                        if (dpos != tempname.npos)
                        {
                            tempname.replace(dpos, 2, &dstr[0], 10);
                        }
                    }
                    files[i] = fopen(tempname.c_str(), "a+b");
                    if (ChatLogUTFHeader && (ftell(files[i]) == 0)) fputs("\xEF\xBB\xBF", files[i]);
                }
            }
        }
    }
}

void ChatLog::CloseAllFiles()
{
    for (int i = 0; i <= CHATLOG_CHAT_TYPES_COUNT - 1; i++)
    {
        if (files[i])
        {
            for (int j = i + 1; j <= CHATLOG_CHAT_TYPES_COUNT - 1; j++)
            {
                if (files[j] == files[i]) files[j] = NULL;
            }

            fclose(files[i]);
            files[i] = NULL;
        }
    }
}

void ChatLog::CheckDateSwitch()
{
    if (ChatLogDateSplit)
    {
        time_t t = time(NULL);
        tm* aTm = localtime(&t);
        if (lastday != aTm->tm_mday)
        {
            // date switched
            CloseAllFiles();
            OpenAllFiles();
            WriteInitStamps();
        }
    }
}

void ChatLog::WriteInitStamps()
{
    // remember date
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    lastday = aTm->tm_mday;

    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", "[SYSTEM] Chat Log Initialized\n");
    }
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", "[SYSTEM] Party Chat Log Initialized\n");
    }
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", "[SYSTEM] Guild Chat Log Initialized\n");
    }
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", "[SYSTEM] Whisper Log Initialized\n");
    }
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", "[SYSTEM] Chat Channels Log Initialized\n");
    }
    if (files[CHAT_LOG_CHAT])
    {
        OutTimestamp(files[CHAT_LOG_CHAT]);
        fprintf(files[CHAT_LOG_CHAT], "%s", "[SYSTEM] Raid Party Chat Log Initialized\n");
    }
}

void ChatLog::OutTimestamp(FILE* file)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    fprintf(file, "%-4d-%02d-%02d %02d:%02d:%02d ", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
}