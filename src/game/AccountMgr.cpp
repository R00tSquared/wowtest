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

#include "Database/DatabaseEnv.h"

#include "AccountMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Util.h"

extern DatabaseType AccountsDatabase;

AccountOpResult AccountMgr::CreateAccount(std::string username, std::string password)
{
    return AOR_DB_INTERNAL_ERROR;

    // not needed at all
    
    if (utf8length(username) > MAX_ACCOUNT_STR)
        return AOR_NAME_TOO_LONG;                           // username's too long

    normalizeString(username);
    normalizeString(password);

    AccountsDatabase.escape_string(username);
    AccountsDatabase.escape_string(password);

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT 1 FROM account WHERE username = '%s'", username.c_str());
    if (result)
        return AOR_NAME_ALREDY_EXIST;                       // username does already exist

    if (!AccountsDatabase.PExecute("INSERT INTO account(username, pass_hash, join_date) "
                                   "VALUES ('%s', SHA1(CONCAT('%s', ':', '%s')), NOW())", username.c_str(), username.c_str(), password.c_str()))
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error

    return AOR_OK;                                          // everything's fine
}

AccountOpResult AccountMgr::DeleteAccount(uint32 accid)
{
    return AOR_DB_INTERNAL_ERROR;

    // not needed at all
    
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT 1 FROM account WHERE account_id = '%u'", accid);
    if (!result)
        return AOR_NAME_NOT_EXIST;                          // account doesn't exist

    result = RealmDataDatabase.PQuery("SELECT guid FROM characters WHERE account = '%u'",accid);
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guidlo = fields[0].GetUInt32();
            uint64 guid = MAKE_NEW_GUID(guidlo, 0, HIGHGUID_PLAYER);

            // kick if player currently
            if (Player* p = ObjectAccessor::GetPlayerInWorld(guid))
            {
                WorldSession* s = p->GetSession();
                s->KickPlayer();                            // mark session to remove at next session list update
                s->LogoutPlayer(false);                     // logout player without waiting next session list update
            }

            Player::DeleteFromDB(guid, accid, false);       // no need to update realm characters
        }
        while (result->NextRow());
    }

    // table realm specific but common for all characters of account for realm
    RealmDataDatabase.PExecute("DELETE FROM character_tutorial WHERE account = '%u'",accid);

    if (!AccountsDatabase.PExecute("DELETE FROM account WHERE account_id = '%u'", accid))
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error;

    return AOR_OK;
}

AccountOpResult AccountMgr::ChangeUsername(uint32 accid, std::string new_uname, std::string new_passwd)
{
    return AOR_DB_INTERNAL_ERROR;

    // not needed at all
    
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT 1 FROM account WHERE account_id = '%u'", accid);
    if (!result)
        return AOR_NAME_NOT_EXIST;                          // account doesn't exist

    if (utf8length(new_uname) > MAX_ACCOUNT_STR)
        return AOR_NAME_TOO_LONG;

    if (utf8length(new_passwd) > MAX_ACCOUNT_STR)
        return AOR_PASS_TOO_LONG;

    normalizeString(new_uname);
    normalizeString(new_passwd);

    AccountsDatabase.escape_string(new_uname);
    AccountsDatabase.escape_string(new_passwd);
    if (!AccountsDatabase.PExecute("UPDATE account SET username = '%s', pass_hash = SHA1(CONCAT('%s', ':', '%s')) WHERE account_id = '%u'",
                                   new_uname.c_str(), new_uname.c_str(), new_passwd.c_str(), accid))
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error

    return AOR_OK;
}

AccountOpResult AccountMgr::ChangePassword(uint32 accid, std::string new_passwd)
{
    return AOR_DB_INTERNAL_ERROR;

    // not needed at all
    
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT 1 FROM account WHERE account_id = '%u'", accid);
    if (!result)
        return AOR_NAME_NOT_EXIST;                          // account doesn't exist

    if (utf8length(new_passwd) > MAX_ACCOUNT_STR)
        return AOR_PASS_TOO_LONG;

    normalizeString(new_passwd);

    AccountsDatabase.escape_string(new_passwd);
    if (!AccountsDatabase.PExecute("UPDATE account SET pass_hash = SHA1(CONCAT(username, ':', '%s')) WHERE account_id = '%u'", new_passwd.c_str(), accid))
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error

    return AOR_OK;
}

uint32 AccountMgr::GetId(std::string username)
{
    AccountsDatabase.escape_string(username);
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT account_id FROM account WHERE username = '%s'", username.c_str());
    if (result)
        return (*result)[0].GetUInt32();

    return 0;
}

uint64 AccountMgr::GetPermissions(uint32 acc_id)
{
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT mask FROM account_gm WHERE account_id = '%u' AND realm_id = '%u'", acc_id, realmID);
    if (result)
        return (*result)[0].GetUInt64();

    return PERM_PLAYER;
}

bool AccountMgr::HasPermissions(uint32 accId, uint64 perms)
{
    return GetPermissions(accId) & perms;
}

bool AccountMgr::GetName(uint32 acc_id, std::string &name)
{
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT username FROM account WHERE account_id = '%u'", acc_id);
    if (result)
    {
        name = (*result)[0].GetCppString();
        return true;
    }

    return false;
}

bool AccountMgr::CheckPassword(uint32 accid, std::string passwd)
{
    normalizeString(passwd);
    AccountsDatabase.escape_string(passwd);

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT 1 FROM account WHERE account_id ='%u' AND pass_hash=SHA1(CONCAT(username, ':', '%s'))", accid, passwd.c_str());
    if (result)
        return true;

    return false;
}

bool AccountMgr::normalizeString(std::string& utf8str)
{
    wchar_t wstr_buf[MAX_ACCOUNT_STR+1];

    size_t wstr_len = MAX_ACCOUNT_STR;
    if (!Utf8toWStr(utf8str,wstr_buf,wstr_len))
        return false;

    std::transform(&wstr_buf[0], wstr_buf+wstr_len, &wstr_buf[0], wcharToUpperOnlyLatin);

    return WStrToUtf8(wstr_buf,wstr_len,utf8str);
}

/*std::vector<uint32> AccountMgr::GetRAFAccounts(uint32 accid, bool referred)
{

    QueryResultAutoPtr result;

    if (referred)
        result = AccountsDatabase.PQuery("SELECT `friend_id` FROM `account_friends` WHERE `id` = %u AND `expire_date` > NOW() LIMIT %u", accid, sWorld.getConfig(CONFIG_UINT32_RAF_MAXREFERERS));
    else
        result = AccountsDatabase.PQuery("SELECT `id` FROM `account_friends` WHERE `friend_id` = %u AND `expire_date` > NOW() LIMIT %u", accid, sWorld.getConfig(CONFIG_UINT32_RAF_MAXREFERALS));

    std::vector<uint32> acclist;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 refaccid = fields[0].GetUInt32();
            acclist.push_back(refaccid);
        }
        while (result->NextRow());
    }

    return acclist;
}*/

/*AccountOpResult AccountMgr::AddRAFLink(uint32 accid, uint32 friendid)
{
    if (!AccountsDatabase.PExecute("INSERT INTO `account_friends`  (`id`, `friend_id`, `expire_date`) VALUES (%u,%u,NOW() + INTERVAL 3 MONTH)", accid, friendid))
        return AOR_DB_INTERNAL_ERROR;

    return AOR_OK;
}*/

/*AccountOpResult AccountMgr::DeleteRAFLink(uint32 accid, uint32 friendid)
{
    if (!AccountsDatabase.PExecute("DELETE FROM `account_friends` WHERE `id` = %u AND `friend_id` = %u", accid, friendid))
        return AOR_DB_INTERNAL_ERROR;

    return AOR_OK;
}*/
