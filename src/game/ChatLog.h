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

#ifndef HELLGROUND_CHATLOG_H
#define HELLGROUND_CHATLOG_H

#include "SharedDefines.h"
#include "ObjectMgr.h"
#include "ace/Singleton.h"

#define CHATLOG_CHAT_TYPES_COUNT 7

#define CHAT_LOG_CHAT 0

enum LexicsActions
{
    LEXICS_ACTION_LOG = 0,
    LEXICS_ACTION_SHEEP = 1,
    LEXICS_ACTION_STUN = 2,
    LEXICS_ACTION_DIE = 3,
    LEXICS_ACTION_DRAIN = 4,
    LEXICS_ACTION_SILENCE = 5,
    LEXICS_ACTION_STUCK = 6,
    LEXICS_ACTION_SICKNESS = 7,
    LEXICS_ACTION_SHEAR = 8,
};

class ChatLog
{
    friend class ACE_Singleton<ChatLog, ACE_Thread_Mutex>;
    public:
        ChatLog();
        ~ChatLog();

        void Initialize();

        void ChatMsg(Player *player, std::string &msg, uint32 type);
        void PartyMsg(Player *player, std::string &msg);
        void GuildMsg(Player *player, std::string &msg, bool officer);
        void WhisperMsg(Player *player, std::string &to, std::string &msg);
        void ChannelMsg(Player *player, std::string &channel, std::string &msg, bool bad_lexics);
        void RaidMsg(Player *player, std::string &msg, uint32 type);
        void BattleGroundMsg(Player *player, std::string &msg, uint32 type);

    private:
        bool _ChatCommon(int ChatType, Player *player, std::string &msg);

        bool ChatLogEnable;
        bool ChatLogDateSplit;
        bool ChatLogUTFHeader;
        bool ChatLogIgnoreUnprintable;

        int lastday;

        FILE* files[CHATLOG_CHAT_TYPES_COUNT];
        std::string names[CHATLOG_CHAT_TYPES_COUNT];
        bool screenflag[CHATLOG_CHAT_TYPES_COUNT];

        void OpenAllFiles();
        void CloseAllFiles();
        void CheckDateSwitch();

        void WriteInitStamps();
        void OutTimestamp(FILE *file);
};

#define sChatLog (*ACE_Singleton<ChatLog, ACE_Thread_Mutex>::instance())
#endif