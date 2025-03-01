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

#ifndef HELLGROUND_ACCOUNTMGR_H
#define HELLGROUND_ACCOUNTMGR_H

#include "Common.h"
#include <string>

enum AccountOpResult
{
    AOR_OK,
    AOR_NAME_TOO_LONG,
    AOR_PASS_TOO_LONG,
    AOR_NAME_ALREDY_EXIST,
    AOR_NAME_NOT_EXIST,
    AOR_DB_INTERNAL_ERROR
};

#define MAX_ACCOUNT_STR 16

namespace AccountMgr
{
    AccountOpResult CreateAccount(std::string username, std::string password);
    AccountOpResult DeleteAccount(uint32 accid);
    AccountOpResult ChangeUsername(uint32 accid, std::string new_uname, std::string new_passwd);
    AccountOpResult ChangePassword(uint32 accid, std::string new_passwd);
    bool CheckPassword(uint32 accid, std::string passwd);

    uint32 GetId(std::string username);

    uint64 GetPermissions(uint32 acc_id);
    bool HasPermissions(uint32 accId, uint64 perms);

    bool GetName(uint32 acc_id, std::string &name);

    bool normalizeString(std::string& utf8str);

    // HG RAF
    //std::vector<uint32> GetRAFAccounts(uint32 accid, bool referred = true);
    //AccountOpResult AddRAFLink(uint32 accid, uint32 friendid);
    //AccountOpResult DeleteRAFLink(uint32 accid, uint32 friendid);
};

#endif
