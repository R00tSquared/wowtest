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
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "RealmList.h"
#include "AuthSocket.h"
#include "AuthCodes.h"
#include "TOTP.h"
#include "PatchHandler.h"

#include <openssl/md5.h>
//#include "Util.h" -- for commented utf8ToUpperOnlyLatin

#include <ace/OS_NS_unistd.h>
#include <ace/OS_NS_fcntl.h>
#include <ace/OS_NS_sys_stat.h>

#include <sstream>

extern DatabaseType AccountsDatabase;

enum AccountFlags
{
    ACCOUNT_FLAG_GM         = 0x00000001,
    ACCOUNT_FLAG_TRIAL      = 0x00000008,
    ACCOUNT_FLAG_PROPASS    = 0x00800000,
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

typedef struct AUTH_LOGON_CHALLENGE_C
{
    uint8   cmd;
    uint8   error;
    uint16  size;
    uint8   gamename[4];
    uint8   version1;
    uint8   version2;
    uint8   version3;
    uint16  build;
    uint8   platform[4];
    uint8   os[4];
    uint8   country[4];
    uint32  timezone_bias;
    uint8   ip[4];
    uint8   I_len;
    uint8   I[1];
} sAuthLogonChallenge_C;

//typedef sAuthLogonChallenge_C sAuthReconnectChallenge_C;
/*
typedef struct
{
    uint8   cmd;
    uint8   error;
    uint8   unk2;
    uint8   B[32];
    uint8   g_len;
    uint8   g[1];
    uint8   N_len;
    uint8   N[32];
    uint8   s[32];
    uint8   unk3[16];
} sAuthLogonChallenge_S;
*/

typedef struct AUTH_LOGON_PROOF_C
{
    uint8   cmd;
    uint8   A[32];
    uint8   M1[20];
    uint8   crc_hash[20];
    uint8   number_of_keys;
    uint8   securityFlags;                                  // 0x00-0x04
} sAuthLogonProof_C;
/*
typedef struct
{
    uint16  unk1;
    uint32  unk2;
    uint8   unk3[4];
    uint16  unk4[20];
}  sAuthLogonProofKey_C;
*/
typedef struct AUTH_LOGON_PROOF_S
{
    uint8   cmd;
    uint8   error;
    uint8   M2[20];
    uint32  accountFlags;                                   // see enum AccountFlags
    uint32  surveyId;                                       // SurveyId
    uint16  unkFlags;                                       // some flags (AccountMsgAvailable = 0x01)
} sAuthLogonProof_S;

typedef struct AUTH_LOGON_PROOF_S_BUILD_6005
{
    uint8   cmd;
    uint8   error;
    uint8   M2[20];
    //uint32  unk1;
    uint32  unk2;
    //uint16  unk3;
} sAuthLogonProof_S_BUILD_6005;

typedef struct AUTH_RECONNECT_PROOF_C
{
    uint8   cmd;
    uint8   R1[16];
    uint8   R2[20];
    uint8   R3[20];
    uint8   number_of_keys;
} sAuthReconnectProof_C;

typedef struct XFER_INIT
{
    uint8 cmd;                                              // XFER_INITIATE
    uint8 fileNameLen;                                      // strlen(fileName);
    uint8 fileName[5];                                      // fileName[fileNameLen]
    uint64 file_size;                                       // file size (bytes)
    uint8 md5[MD5_DIGEST_LENGTH];                           // MD5
}XFER_INIT;

typedef struct AuthHandler
{
    eAuthCmd cmd;
    eStatus status;
    bool (AuthSocket::*handler)(void);
}AuthHandler;

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

const AuthHandler table[] =
{
    { CMD_AUTH_LOGON_CHALLENGE,         STATUS_CHALLENGE,   &AuthSocket::_HandleLogonChallenge    },
    { CMD_AUTH_LOGON_PROOF,             STATUS_LOGON_PROOF, &AuthSocket::_HandleLogonProof        },
    { CMD_AUTH_RECONNECT_CHALLENGE,     STATUS_CHALLENGE,   &AuthSocket::_HandleReconnectChallenge},
    { CMD_AUTH_RECONNECT_PROOF,         STATUS_RECON_PROOF, &AuthSocket::_HandleReconnectProof    },
    { CMD_REALM_LIST,                   STATUS_AUTHED,      &AuthSocket::_HandleRealmList         },
    { CMD_XFER_ACCEPT,                  STATUS_PATCH,       &AuthSocket::_HandleXferAccept        },
    { CMD_XFER_RESUME,                  STATUS_PATCH,       &AuthSocket::_HandleXferResume        },
    { CMD_XFER_CANCEL,                  STATUS_PATCH,       &AuthSocket::_HandleXferCancel        }
};

#define AUTH_TOTAL_COMMANDS sizeof(table)/sizeof(AuthHandler)

/// Constructor - set the N and g values for SRP6
AuthSocket::AuthSocket()
{
    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
    _status = STATUS_CHALLENGE;

    accountPermissionMask_ = PERM_PLAYER;

    _build = 0;
    patch_ = ACE_INVALID_HANDLE;
}

/// Close patch file descriptor before leaving
AuthSocket::~AuthSocket()
{
    if(patch_ != ACE_INVALID_HANDLE)
        ACE_OS::close(patch_);
}

/// Accept the connection and set the s random value for SRP6
void AuthSocket::OnAccept()
{
    sLog.outBasic("Accepting connection from '%s'", get_remote_address().c_str());
}

#define MAX_AUTH_LOGON_CHALLENGES_IN_A_ROW 3
#define MAX_AUTH_GET_REALM_LIST 10
/// Read the packet from the client
void AuthSocket::OnRead()
{
    uint32 challengesInARow = 0;
    uint32 challengesInARowRealmList = 0;
    uint8 _cmd;
    while (1)
    {
        if(!recv_soft((char *)&_cmd, 1))
            return;

        if (_cmd == CMD_AUTH_LOGON_CHALLENGE)
        {
            ++challengesInARow;
            if (challengesInARow == MAX_AUTH_LOGON_CHALLENGES_IN_A_ROW)
            {
                sLog.outBasic("Got %u CMD_AUTH_LOGON_CHALLENGE in a row from '%s', possible ongoing DoS", challengesInARow, get_remote_address().c_str());
                close_connection();
                return;
            }
        }
        else if (_cmd == CMD_REALM_LIST) {
            challengesInARowRealmList++;
            if (challengesInARowRealmList == MAX_AUTH_GET_REALM_LIST)
            {
                sLog.outBasic("Got %u REALM_LIST in a row from '%s', possible ongoing DoS", challengesInARowRealmList, get_remote_address().c_str());
                close_connection();
                return;
            }
        }

        size_t i;

        ///- Circle through known commands and call the correct command handler
        for (i = 0; i < AUTH_TOTAL_COMMANDS; ++i)
        {
            if ((uint8)table[i].cmd == _cmd && table[i].status == _status)
            {
                debug_log("[Auth] got data for cmd %u recv length %u",
                        (uint32)_cmd, (uint32)recv_len());

                if (!(*this.*table[i].handler)())
                {
                    debug_log("Command handler failed for cmd %u recv length %u",
                            (uint32)_cmd, (uint32)recv_len());

                    return;
                }
                break;
            }
        }

        ///- Report unknown commands in the debug log
        if (i == AUTH_TOTAL_COMMANDS)
        {
            sLog.outLog(LOG_DEFAULT, "[Auth] got unknown packet %u from '%s'. Kicking.", (uint32)_cmd, get_remote_address().c_str());
            sLog.outLog(LOG_WARDEN, "[Auth] got unknown packet %u from '%s'. Kicking.", (uint32)_cmd, get_remote_address().c_str());
            return;
        }
    }
}

/// Make the SRP6 calculation from hash in dB
void AuthSocket::_SetVSFields(const std::string& rI)
{
    s.SetRand(s_BYTE_SIZE * 8);

    BigNumber I;
    I.SetHexStr(rI.c_str());

    // In case of leading zeros in the rI hash, restore them
    uint8 mDigest[SHA_DIGEST_LENGTH];
    memset(mDigest, 0, SHA_DIGEST_LENGTH);
    if (I.GetNumBytes() <= SHA_DIGEST_LENGTH)
        memcpy(mDigest, I.AsByteArray(), I.GetNumBytes());

    std::reverse(mDigest, mDigest + SHA_DIGEST_LENGTH);

    Sha1Hash sha;
    sha.UpdateData(s.AsByteArray(), s.GetNumBytes());
    sha.UpdateData(mDigest, SHA_DIGEST_LENGTH);
    sha.Finalize();
    BigNumber x;
    x.SetBinary(sha.GetDigest(), sha.GetLength());
    v = g.ModExp(x, N);
    // No SQL injection (username escaped)
    const char *v_hex, *s_hex;
    v_hex = v.AsHexStr();
    s_hex = s.AsHexStr();

    AccountsDatabase.DirectPExecute("UPDATE account_session SET v = '%s', s = '%s' WHERE username = '%s'", v_hex, s_hex, _safelogin.c_str() );

    OPENSSL_free((void*)v_hex);
    OPENSSL_free((void*)s_hex);
}

void AuthSocket::SendProof(Sha1Hash sha)
{
    switch(_build)
    {
        case 5875:                                          // 1.12.1
        case 6005:                                          // 1.12.2
        {
            sAuthLogonProof_S_BUILD_6005 proof;
            memcpy(proof.M2, sha.GetDigest(), 20);
            proof.cmd = CMD_AUTH_LOGON_PROOF;
            proof.error = 0;
            proof.unk2 = 0x00;

            send((char *)&proof, sizeof(proof));
            break;
        }
        case 8606:                                          // 2.4.3
        case 10505:                                         // 3.2.2a
        case 11159:                                         // 3.3.0a
        case 11403:                                         // 3.3.2
        case 11723:                                         // 3.3.3a
        case 12340:                                         // 3.3.5a
        default:                                            // or later
        {
            sAuthLogonProof_S proof;
            memcpy(proof.M2, sha.GetDigest(), 20);
            proof.cmd = CMD_AUTH_LOGON_PROOF;
            proof.error = 0;
            proof.accountFlags = ACCOUNT_FLAG_PROPASS;
            proof.surveyId = 0x00000000;
            proof.unkFlags = 0x0000;

            send((char *)&proof, sizeof(proof));
            break;
        }
    }
}

#ifdef REGEX_NAMESPACE
PatternList AuthSocket::pattern_banned = PatternList();
#endif

/// Logon Challenge command handler
bool AuthSocket::_HandleLogonChallenge()
{
    debug_log("Entering _HandleLogonChallenge");
    if (recv_len() < sizeof(sAuthLogonChallenge_C))
        return false;

    ///- Read the first 4 bytes (header) to get the length of the remaining of the packet
    std::vector<uint8> buf;
    buf.resize(4);

    recv((char *)&buf[0], 4);

    EndianConvert(*((uint16*)(buf[0])));
    uint16 remaining = ((sAuthLogonChallenge_C *)&buf[0])->size;
    debug_log("[AuthChallenge] got header, body is %#04x bytes", remaining);

    if ((remaining < sizeof(sAuthLogonChallenge_C) - buf.size()) || (recv_len() < remaining))
        return false;

    //- Session is closed unless overriden
    _status = STATUS_CLOSED;

    //No big fear of memory outage (size is int16, i.e. < 65536)
    buf.resize(remaining + buf.size() + 1);
    buf[buf.size() - 1] = 0;
    sAuthLogonChallenge_C *ch = (sAuthLogonChallenge_C*)&buf[0];

    ///- Read the remaining of the packet
    recv((char *)&buf[4], remaining);
    debug_log("[AuthChallenge] got full packet, %#04x bytes", ch->size);
    debug_log("[AuthChallenge] name(%d): '%s'", ch->I_len, ch->I);

    // BigEndian code, nop in little endian case
    // size already converted
    EndianConvert(*((uint32*)(&ch->gamename[0])));
    EndianConvert(ch->build);
    EndianConvert(*((uint32*)(&ch->platform[0])));
    EndianConvert(*((uint32*)(&ch->os[0])));
    EndianConvert(*((uint32*)(&ch->country[0])));
    EndianConvert(ch->timezone_bias);
    EndianConvert(*((uint32*)(&ch->ip[0])));

    std::stringstream tmpLocalIp;
    tmpLocalIp << (uint32)ch->ip[0] << "." << (uint32)ch->ip[1] << "." << (uint32)ch->ip[2] << "." << (uint32)ch->ip[3];

    localIp_ = tmpLocalIp.str();

    ByteBuffer pkt;

    _login = (const char*)ch->I;
    _build = ch->build;
    std::string operatingSystem_ = (const char*)ch->os;

    // Restore string order as its byte order is reversed
    std::reverse(operatingSystem_.begin(), operatingSystem_.end());

    if (operatingSystem_ == "Win")
        OS = CLIENT_OS_WIN;
    else if (operatingSystem_ == "OSX")
        OS = CLIENT_OS_OSX;
    else if (sRealmList.GetChatboxOsName() != "" && operatingSystem_ == sRealmList.GetChatboxOsName())
        OS = CLIENT_OS_CHAT;
    else {
        sLog.outLog(LOG_WARDEN, "Client %s got unsupported operating system (%s)", _login.c_str(), operatingSystem_.c_str());
        return false;
    }

    ///- Normalize account name
    //utf8ToUpperOnlyLatin(_login); -- client already send account in expected form

    //Escape the user login to avoid further SQL injection
    //Memory will be freed on AuthSocket object destruction
    _safelogin = _login;
    AccountsDatabase.escape_string(_safelogin);

    pkt << (uint8) CMD_AUTH_LOGON_CHALLENGE;
    pkt << (uint8) 0x00;

    std::string address = get_remote_address();

#ifdef REGEX_NAMESPACE
    for (PatternList::const_iterator i = pattern_banned.begin(); i != pattern_banned.end(); ++i)
    {
        if (REGEX_NAMESPACE::regex_match(address.c_str(), i->first) && REGEX_NAMESPACE::regex_match(localIp_.c_str(), i->second))
        {
            pkt<< (uint8) WOW_FAIL_UNKNOWN_ACCOUNT;
            send((char const*)pkt.contents(), pkt.size());
            return true;
        }
    }
#endif

    ///- Verify that this IP is not in the ip_banned table
    // No SQL injection possible (paste the IP address as passed by the socket)
    AccountsDatabase.Execute("DELETE FROM ip_banned WHERE expiration_date<=UNIX_TIMESTAMP() AND expiration_date<>punishment_date");
    AccountsDatabase.escape_string(address);
	QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT * FROM ip_banned WHERE ip = '%s'", address.c_str());

	if (result) // ip banned
	{
		//if punished_by == "Realm", then it's IP BRUTE ban, otherwise IP was banned by GM
		if ((*result)[3].GetCppString() == "Realm") {
			pkt << uint8(WOW_FAIL_DB_BUSY);
			send((char const*)pkt.contents(), pkt.size());
		}
		else {
			sLog.outBasic("[AuthChallenge] Banned ip %s tries to login!", get_remote_address().c_str());
			pkt << uint8(WOW_FAIL_BANNED);
			send((char const*)pkt.contents(), pkt.size());
		}

		return true;
	}


    ///- Get the account details from the account table
    // No SQL injection (escaped user name)

    result = AccountsDatabase.PQuery("SELECT pass_hash, account.account_id, account_state_id, token_key, last_ip, mask, email "
                                     "FROM account LEFT JOIN account_gm ON account.account_id = account_gm.account_id "
                                     "WHERE username = '%s' ORDER BY mask DESC", _safelogin.c_str());

    if (!result)    // account not exists
    {
        pkt<< uint8(WOW_FAIL_UNKNOWN_ACCOUNT);
        send((char const*)pkt.contents(), pkt.size());
        return true;
    }

    Field * fields = result->Fetch();

    ///- If the IP is 'locked', check that the player comes indeed from the correct IP address
    switch (fields[2].GetUInt8())
    {
        case ACCOUNT_STATE_IP_LOCKED:
        {
            debug_log("[AuthChallenge] Account '%s' is locked to IP - '%s'", _login.c_str(), (*result)[3].GetString());
            debug_log("[AuthChallenge] Player address is '%s'", get_remote_address().c_str());
            if (strcmp(fields[4].GetString(), get_remote_address().c_str()))
            {
                debug_log("[AuthChallenge] Account IP differs");
                    pkt << (uint8) WOW_FAIL_LOCKED_ENFORCED;
                send((char const*)pkt.contents(), pkt.size());
                return true;
            }
            else
                debug_log("[AuthChallenge] Account IP matches");

            break;
        }
        case ACCOUNT_STATE_FROZEN:
        {
            pkt << uint8(WOW_FAIL_SUSPENDED);
            send((char const*)pkt.contents(), pkt.size());
            return true;
        }
        default:
            debug_log("[AuthChallenge] Account '%s' is not locked to ip or frozen", _login.c_str());
            break;
    }
    _status = STATUS_LOGON_PROOF;

    // update account punishments
    AccountsDatabase.PQuery("UPDATE account_punishment SET active = 0 WHERE account_id = '%u' "
        "AND active = 1 AND expiration_date <= UNIX_TIMESTAMP() AND expiration_date <> punishment_date AND punishment_type_id = '%u'", (*result)[1].GetUInt32(), PUNISHMENT_BAN);
    ///- If the account is banned, reject the logon attempt
    QueryResultAutoPtr  banresult = AccountsDatabase.PQuery("SELECT punishment_date, expiration_date "
                                                            "FROM account_punishment WHERE account_id = '%u' "
                                                            "AND punishment_type_id = '%u' AND active = 1 ",(*result)[1].GetUInt32(), PUNISHMENT_BAN);

    if (banresult)
    {
        if((*banresult)[0].GetUInt64() == (*banresult)[1].GetUInt64())
        {
            pkt << uint8(WOW_FAIL_BANNED);
            sLog.outBasic("[AuthChallenge] Banned account %s tries to login!", _login.c_str ());
        }
        else
        {
            pkt << uint8(WOW_FAIL_SUSPENDED);
            sLog.outBasic("[AuthChallenge] Temporarily banned account %s tries to login!", _login.c_str ());
        }

        send((char const*)pkt.contents(), pkt.size());
        return true;
    }

    QueryResultAutoPtr  emailbanresult = AccountsDatabase.PQuery("SELECT email FROM email_banned WHERE email = '%s'", (*result)[5].GetString());
    if (emailbanresult)
    {
        pkt << uint8(WOW_FAIL_BANNED);
        sLog.outBasic("[AuthChallenge] Account %s with banned email %s tries to login!", _login.c_str (), (*emailbanresult)[0].GetString());

        send((char const*)pkt.contents(), pkt.size());
        return true;
    }

    ///- Get the password from the account table, upper it, and make the SRP6 calculation
    std::string rI = fields[0].GetCppString();

    _SetVSFields(rI);

    b.SetRand(19 * 8);
    BigNumber gmod = g.ModExp(b, N);
    B = ((v * 3) + gmod) % N;

    ASSERT(gmod.GetNumBytes() <= 32);

    BigNumber unk3;
    unk3.SetRand(16 * 8);

    ///- Fill the response packet with the result
    pkt << uint8(WOW_SUCCESS);

    // B may be calculated < 32B so we force minimal length to 32B
    pkt.append(B.AsByteArray(32), 32);      // 32 bytes
    pkt << uint8(1);
    pkt.append(g.AsByteArray(), 1);
    pkt << uint8(32);
    pkt.append(N.AsByteArray(32), 32);
    pkt.append(s.AsByteArray(), s.GetNumBytes());// 32 bytes
    pkt.append(unk3.AsByteArray(16), 16);
    uint8 securityFlags = 0;
    // Check if token is used
    _tokenKey = fields[3].GetString();
        if (!_tokenKey.empty())
            securityFlags = 4;

    pkt << uint8(securityFlags);            // security flags (0x0...0x04)

    if (securityFlags & 0x01)                // PIN input
    {
        pkt << uint32(0);
        pkt << uint64(0) << uint64(0);      // 16 bytes hash?
    }

    if (securityFlags & 0x02)                // Matrix input
    {
        pkt << uint8(0);
        pkt << uint8(0);
        pkt << uint8(0);
        pkt << uint8(0);
        pkt << uint64(0);
    }

    if (securityFlags & 0x04)                // Security token input
        pkt << uint8(1);

    accountPermissionMask_ = fields[5].GetUInt64();
    if (!accountPermissionMask_)
        accountPermissionMask_ = PERM_PLAYER;

    _localizationName.resize(4);
    for (int i = 0; i < 4; ++i)
        _localizationName[i] = ch->country[4-i-1];

    sLog.outBasic("[AuthChallenge] account %s is using '%c%c%c%c' locale (%u)", _login.c_str (), ch->country[3], ch->country[2], ch->country[1], ch->country[0], GetLocaleByName(_localizationName));

    send((char const*)pkt.contents(), pkt.size());
    return true;
}

/// Logon Proof command handler
bool AuthSocket::_HandleLogonProof()
{
    debug_log("Entering _HandleLogonProof");
    ///- Read the packet
    sAuthLogonProof_C lp;
    if(!recv((char *)&lp, sizeof(sAuthLogonProof_C)))
        return false;

    //- Session is closed unless overriden
    _status = STATUS_CLOSED;

    ///- Check if the client has one of the expected version numbers
    bool valid_version = FindBuildInfo(_build) != NULL;

    /// <ul><li> If the client has no valid version
    if(!valid_version)
    {
        if (this->patch_ != ACE_INVALID_HANDLE)
            return false;

        ///- Check if we have the apropriate patch on the disk
        // file looks like: 65535enGB.mpq
        char tmp[64];

        snprintf(tmp, 24, "./patches/%d%s.mpq", _build, _localizationName.c_str());

        char filename[PATH_MAX];
        if (ACE_OS::realpath(tmp, filename) != NULL)
        {
            patch_ = ACE_OS::open(filename, GENERIC_READ | FILE_FLAG_SEQUENTIAL_SCAN);
        }

        if (patch_ == ACE_INVALID_HANDLE)
        {
            // no patch found
            ByteBuffer pkt;
            pkt << (uint8) CMD_AUTH_LOGON_CHALLENGE;
            pkt << (uint8) 0x00;
            pkt << (uint8) WOW_FAIL_VERSION_INVALID;
            debug_log("[AuthChallenge] %u is not a valid client version!", _build);
            debug_log("[AuthChallenge] Patch %s not found", tmp);
            send((char const*)pkt.contents(), pkt.size());
            return true;
        }

        XFER_INIT xferh;

        ACE_OFF_T file_size = ACE_OS::filesize(this->patch_);

        if (file_size == -1)
        {
            close_connection();
            return false;
        }

        if (!PatchCache::instance()->GetHash(tmp, (uint8*)&xferh.md5))
        {
            // calculate patch md5, happens if patch was added while realmd was running
            PatchCache::instance()->LoadPatchMD5(tmp);
            PatchCache::instance()->GetHash(tmp, (uint8*)&xferh.md5);
        }

        uint8 data[2] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_VERSION_UPDATE};
        send((const char*)data, sizeof(data));

        memcpy(&xferh, "0\x05Patch", 7);
        xferh.cmd = CMD_XFER_INITIATE;
        xferh.file_size = file_size;

        send((const char*)&xferh, sizeof(xferh));
        return true;
    }
    /// </ul>

    ///- Continue the SRP6 calculation based on data received from the client
    BigNumber A;

    A.SetBinary(lp.A, 32);

    // SRP safeguard: abort if A==0 or A % N == 0
    if (A.isZero() || (A % N).isZero())
        return false;

    Sha1Hash sha;
    sha.UpdateBigNumbers(&A, &B, NULL);
    sha.Finalize();
    BigNumber u;
    u.SetBinary(sha.GetDigest(), 20);
    BigNumber S = (A * (v.ModExp(u, N))).ModExp(b, N);

    uint8 t[32];
    uint8 t1[16];
    uint8 vK[40];
    memcpy(t, S.AsByteArray(32), 32);
    for (int i = 0; i < 16; ++i)
    {
        t1[i] = t[i * 2];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
        
    // Check auth token
        if ((lp.securityFlags & 0x04) || !_tokenKey.empty())
        {
            uint8 size;
            recv((char*)&size, 1);
            char* token = new char[size + 1];
            token[size] = '\0';
            recv(token, size);
            unsigned int validToken = TOTP::GenerateToken(_tokenKey.c_str());
            unsigned int incomingToken = atoi(token);
            delete[] token;
            if (validToken != incomingToken)
            {
                char data[4] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT, 3, 0};
                send(data, sizeof(data));
                return false;
            }
        }
    for (int i = 0; i < 20; ++i)
    {
        vK[i * 2] = sha.GetDigest()[i];
    }
    for (int i = 0; i < 16; ++i)
    {
        t1[i] = t[i * 2 + 1];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
    {
        vK[i * 2 + 1] = sha.GetDigest()[i];
    }
    K.SetBinary(vK, 40);

    uint8 hash[20];

    sha.Initialize();
    sha.UpdateBigNumbers(&N, NULL);
    sha.Finalize();
    memcpy(hash, sha.GetDigest(), 20);
    sha.Initialize();
    sha.UpdateBigNumbers(&g, NULL);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
    {
        hash[i] ^= sha.GetDigest()[i];
    }
    BigNumber t3;
    t3.SetBinary(hash, 20);

    sha.Initialize();
    sha.UpdateData(_login);
    sha.Finalize();
    uint8 t4[SHA_DIGEST_LENGTH];
    memcpy(t4, sha.GetDigest(), SHA_DIGEST_LENGTH);

    sha.Initialize();
    sha.UpdateBigNumbers(&t3, NULL);
    sha.UpdateData(t4, SHA_DIGEST_LENGTH);
    sha.UpdateBigNumbers(&s, &A, &B, &K, NULL);
    sha.Finalize();
    BigNumber M;
    M.SetBinary(sha.GetDigest(), 20);

    ///- Check if SRP6 results match (password is correct), else send an error
    if (!memcmp(M.AsByteArray(), lp.M1, 20))
    {
        sLog.outBasic("User '%s' successfully authenticated", _login.c_str());

        ///- Update the sessionkey, last_ip, last login time and reset number of failed logins in the account table for this account
        // No SQL injection (escaped user name) and IP address as received by socket
        const char* K_hex = K.AsHexStr();

        QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT account_id FROM account WHERE username = '%s'", _safelogin.c_str());

        if (!result)
        {
            if (_build > 6005)                                  // > 1.12.2
            {
                char data[4] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT, 3, 0};
                send(data, sizeof(data));
            }
            else
            {
                // 1.x not react incorrectly at 4-byte message use 3 as real error
                char data[2] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT};
                send(data, sizeof(data));
            }
            return true;
        }

        uint32 accId = result->Fetch()->GetUInt32();

        // direct to be sure that values will be set before character choose, this will slow down logging in a bit ;p
        AccountsDatabase.DirectPExecute("UPDATE account_session SET session_key = '%s' WHERE account_id = '%u'", K_hex, accId);

        static SqlStatementID updateAccount;
        SqlStatement stmt = AccountsDatabase.CreateStatement(updateAccount, "UPDATE account SET last_ip = ?, last_local_ip = ?, last_login = NOW(), locale_id = ?, failed_logins = 0, client_os_version_id = ? WHERE account_id = ?");
        std::string tmpIp = get_remote_address();
        stmt.addString(tmpIp.c_str());
        stmt.addString(localIp_.c_str());
        stmt.addUInt8(uint8(GetLocaleByName(_localizationName)));
        stmt.addUInt8(OS);
        stmt.addUInt32(accId);
        stmt.DirectExecute();

        OPENSSL_free((void*)K_hex);

        ///- Finish SRP6 and send the final result to the client
        sha.Initialize();
        sha.UpdateBigNumbers(&A, &M, &K, NULL);
        sha.Finalize();

        SendProof(sha);

        // Set _status to authed
        _status = STATUS_AUTHED;
    }
    else
    {
        if (_build > 6005)                                  // > 1.12.2
        {
            char data[4] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT, 3, 0 };
            send(data, sizeof(data));
        }
        else
        {
            // 1.x not react incorrectly at 4-byte message use 3 as real error
            char data[2] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT };
            send(data, sizeof(data));
        }
        sLog.outBasic("[AuthChallenge] account %s tried to login with wrong password!",_login.c_str ());

        
        if (sRealmList.GetWrongPassCount())
        {
            static SqlStatementID updateAccountFailedLogins;
            //Increment number of failed logins by one and if it reaches the limit temporarily ban that account or IP
            SqlStatement stmt = AccountsDatabase.CreateStatement(updateAccountFailedLogins, "UPDATE account SET failed_logins = failed_logins + 1 WHERE username = ?");
            stmt.addString(_login);
            stmt.Execute();

            if (QueryResultAutoPtr loginfail = AccountsDatabase.PQuery("SELECT account_id, failed_logins FROM account WHERE username = '%s'", _safelogin.c_str()))
            {
                Field* fields = loginfail->Fetch();
                uint32 failed_logins = fields[1].GetUInt32();

                if (failed_logins >= sRealmList.GetWrongPassCount())
                {
                    if (sRealmList.GetWrongPassBanType())
                    {
                        uint32 acc_id = fields[0].GetUInt32();
                        AccountsDatabase.PExecute("INSERT INTO account_punishment VALUES ('%u', '%u', UNIX_TIMESTAMP(), UNIX_TIMESTAMP()+%u, 'Realm', 'Incorrect password for: %u times. Ban for: %u seconds', '1', '0')",
                                                acc_id, PUNISHMENT_BAN, sRealmList.GetWrongPassBanTime(), failed_logins, sRealmList.GetWrongPassBanTime());
                        sLog.outBasic("[AuthChallenge] account %s got banned for '%u' seconds because it failed to authenticate '%u' times",
                            _login.c_str(), sRealmList.GetWrongPassBanTime(), failed_logins);
                    }
                    else
                    {
                        std::string current_ip = get_remote_address();
                        AccountsDatabase.escape_string(current_ip);
                        AccountsDatabase.PExecute("INSERT INTO ip_banned VALUES ('%s',UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+'%u','Realm','Incorrect password for: %u times. Ban for: %u seconds')",
                            current_ip.c_str(), sRealmList.GetWrongPassBanTime(), failed_logins, sRealmList.GetWrongPassBanTime());
                        sLog.outBasic("[AuthChallenge] IP %s got banned for '%u' seconds because account %s failed to authenticate '%u' times",
                            current_ip.c_str(), sRealmList.GetWrongPassBanTime(), _login.c_str(), failed_logins);
                    }
                }
            }
        }
    }
    return true;
}

/// Reconnect Challenge command handler
bool AuthSocket::_HandleReconnectChallenge()
{
    debug_log("Entering _HandleReconnectChallenge");
    if (recv_len() < sizeof(sAuthLogonChallenge_C))
        return false;

    ///- Read the first 4 bytes (header) to get the length of the remaining of the packet
    std::vector<uint8> buf;
    buf.resize(4);

    recv((char *)&buf[0], 4);

    EndianConvert(*((uint16*)(buf[0])));
    uint16 remaining = ((sAuthLogonChallenge_C *)&buf[0])->size;
    debug_log("[ReconnectChallenge] got header, body is %#04x bytes", remaining);

    if ((remaining < sizeof(sAuthLogonChallenge_C) - buf.size()) || (recv_len() < remaining))
        return false;

    //- Session is closed unless overriden
    _status = STATUS_CLOSED;

    //No big fear of memory outage (size is int16, i.e. < 65536)
    buf.resize(remaining + buf.size() + 1);
    buf[buf.size() - 1] = 0;
    sAuthLogonChallenge_C *ch = (sAuthLogonChallenge_C*)&buf[0];

    ///- Read the remaining of the packet
    recv((char *)&buf[4], remaining);
    debug_log("[ReconnectChallenge] got full packet, %#04x bytes", ch->size);
    debug_log("[ReconnectChallenge] name(%d): '%s'", ch->I_len, ch->I);

    _login = (const char*)ch->I;

    _safelogin = _login;
    AccountsDatabase.escape_string(_safelogin);

    EndianConvert(ch->build);
    _build = ch->build;

    QueryResultAutoPtr  result = AccountsDatabase.PQuery("SELECT session_key FROM account JOIN account_session ON account.account_id = account_session.account_id WHERE username = '%s'", _safelogin.c_str());

    // Stop if the account is not found
    if (!result)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: [ERROR] user %s tried to login and we cannot find his session key in the database.", _login.c_str());
        close_connection();
        return false;
    }

    Field* fields = result->Fetch ();
    K.SetHexStr (fields[0].GetString ());

    _status = STATUS_RECON_PROOF;

    ///- Sending response
    ByteBuffer pkt;
    pkt << uint8(CMD_AUTH_RECONNECT_CHALLENGE);
    pkt << uint8(0x00);
    _reconnectProof.SetRand(16 * 8);
    pkt.append(_reconnectProof.AsByteArray(16),16);         // 16 bytes random
    pkt << uint64(0x00) << uint64(0x00);                    // 16 bytes zeros
    send((char const*)pkt.contents(), pkt.size());
    return true;
}

/// Reconnect Proof command handler
bool AuthSocket::_HandleReconnectProof()
{
    debug_log("Entering _HandleReconnectProof");
    ///- Read the packet
    sAuthReconnectProof_C lp;
    if(!recv((char *)&lp, sizeof(sAuthReconnectProof_C)))
        return false;

    //- Session is closed unless overriden
    _status = STATUS_CLOSED;

    if (_login.empty() || !_reconnectProof.GetNumBytes() || !K.GetNumBytes())
        return false;

    BigNumber t1;
    t1.SetBinary(lp.R1, 16);

    Sha1Hash sha;
    sha.Initialize();
    sha.UpdateData(_login);
    sha.UpdateBigNumbers(&t1, &_reconnectProof, &K, NULL);
    sha.Finalize();

    if (!memcmp(sha.GetDigest(), lp.R2, SHA_DIGEST_LENGTH))
    {
        ///- Sending response
        ByteBuffer pkt;
        pkt << uint8(CMD_AUTH_RECONNECT_PROOF);
        pkt << uint8(0x00);
        pkt << uint16(0x00);                                // 2 bytes zeros
        send((char const*)pkt.contents(), pkt.size());

        // Set _status to authed
        _status = STATUS_AUTHED;

        return true;
    }
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: [ERROR] user %s tried to login, but session invalid.", _login.c_str());
        close_connection();
        return false;
    }
}

/// %Realm List command handler
bool AuthSocket::_HandleRealmList()
{
    debug_log("Entering _HandleRealmList");
    if (recv_len() < 5)
        return false;

    recv_skip(5);

    ///- Get the user id (else close the connection)
    // No SQL injection (escaped user name)

    QueryResultAutoPtr  result = AccountsDatabase.PQuery("SELECT account_id, pass_hash FROM account WHERE username = '%s'", _safelogin.c_str());
    if (!result)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: [ERROR] user %s tried to login and we cannot find him in the database.", _login.c_str());
        close_connection();
        return false;
    }

    uint32 id = (*result)[0].GetUInt32();
    std::string rI = (*result)[1].GetCppString();

    ///- Update realm list if need
    sRealmList.UpdateIfNeed();

    ///- Circle through realms in the RealmList and construct the return packet (including # of user characters in each realm)
    ByteBuffer pkt;
    LoadRealmlist(pkt, id);

    ByteBuffer hdr;
    hdr << uint8(CMD_REALM_LIST);
    hdr << uint16(pkt.size());
    hdr.append(pkt);

    send((char const*)hdr.contents(), hdr.size());

    return true;
}

void AuthSocket::LoadRealmlist(ByteBuffer& pkt, uint32 acctid)
{
    switch (_build)
    {
    case 5875:                                          // 1.12.1
    case 6005:                                          // 1.12.2
    {
        pkt << uint32(0);                               // unused value
        pkt << uint8(sRealmList.size());

        for (RealmList::RealmMap::const_iterator i = sRealmList.begin(); i != sRealmList.end(); ++i)
        {
            uint8 AmountOfCharacters;

            // No SQL injection. id of realm is controlled by the database.
            QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT characters_count FROM realm_characters WHERE realm_id = '%u' AND account_id = '%u'", i->second.m_ID, acctid);
            if (result)
            {
                Field* fields = result->Fetch();
                AmountOfCharacters = fields[0].GetUInt8();
            }
            else
                AmountOfCharacters = 0;

            bool ok_build = std::find(i->second.realmbuilds.begin(), i->second.realmbuilds.end(), _build) != i->second.realmbuilds.end();

            RealmBuildInfo const* buildInfo = ok_build ? FindBuildInfo(_build) : NULL;
            if (!buildInfo)
                buildInfo = &i->second.realmBuildInfo;

            RealmFlags realmflags = i->second.realmflags;

            // 1.x clients not support explicitly REALM_FLAG_SPECIFYBUILD, so manually form similar name as show in more recent clients
            std::string name = i->first;
            if (realmflags & REALM_FLAG_SPECIFYBUILD)
            {
                char buf[20];
                snprintf(buf, 20, " (%u,%u,%u)", buildInfo->major_version, buildInfo->minor_version, buildInfo->bugfix_version);
                name += buf;
            }

            // Show offline state for unsupported client builds and locked realms (1.x clients not support locked state show)
            if (!ok_build || !(i->second.requiredPermissionMask & accountPermissionMask_))
                realmflags = RealmFlags(realmflags | REALM_FLAG_OFFLINE);

            pkt << uint32(i->second.icon);              // realm type
            pkt << uint8(realmflags);                   // realmflags
            pkt << name;                                // name
            pkt << i->second.address;                   // address
            pkt << float(i->second.populationLevel);
            pkt << uint8(AmountOfCharacters);
            pkt << uint8(i->second.timezone);           // realm category
            pkt << uint8(0x00);                         // unk, may be realm number/id?
        }

        pkt << uint16(0x0002);                          // unused value (why 2?)
        break;
    }

    case 8606:                                          // 2.4.3
    case 10505:                                         // 3.2.2a
    case 11159:                                         // 3.3.0a
    case 11403:                                         // 3.3.2
    case 11723:                                         // 3.3.3a
    case 12340:                                         // 3.3.5a
    default:                                            // and later
    {
        pkt << uint32(0);                               // unused value
        pkt << uint16(sRealmList.size());

        for (RealmList::RealmMap::const_iterator i = sRealmList.begin(); i != sRealmList.end(); ++i)
        {
            uint8 AmountOfCharacters;

            // No SQL injection. id of realm is controlled by the database.
            QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT characters_count FROM realm_characters WHERE realm_id = '%u' AND account_id = '%u'", i->second.m_ID, acctid);
            if (result)
            {
                Field* fields = result->Fetch();
                AmountOfCharacters = fields[0].GetUInt8();
            }
            else
                AmountOfCharacters = 0;

            bool ok_build = std::find(i->second.realmbuilds.begin(), i->second.realmbuilds.end(), _build) != i->second.realmbuilds.end();

            RealmBuildInfo const* buildInfo = ok_build ? FindBuildInfo(_build) : NULL;
            if (!buildInfo)
                buildInfo = &i->second.realmBuildInfo;

            uint8 lock = (i->second.requiredPermissionMask & accountPermissionMask_) ? 0 : 1;

            RealmFlags realmFlags = i->second.realmflags;

            // Show offline state for unsupported client builds
            if (!ok_build)
                realmFlags = RealmFlags(realmFlags | REALM_FLAG_OFFLINE);

            if (!buildInfo)
                realmFlags = RealmFlags(realmFlags & ~REALM_FLAG_SPECIFYBUILD);

            pkt << uint8(i->second.icon);               // realm type (this is second column in Cfg_Configs.dbc)
            pkt << uint8(lock);                         // flags, if 0x01, then realm locked
            pkt << uint8(realmFlags);                   // see enum RealmFlags
            pkt << i->first;                            // name
            pkt << i->second.address;                   // address
            pkt << float(i->second.populationLevel);
            pkt << uint8(AmountOfCharacters);
            pkt << uint8(i->second.timezone);           // realm category (Cfg_Categories.dbc)
            pkt << uint8(0x2C);                         // unk, may be realm number/id?

            if (realmFlags & REALM_FLAG_SPECIFYBUILD)
            {
                pkt << uint8(buildInfo->major_version);
                pkt << uint8(buildInfo->minor_version);
                pkt << uint8(buildInfo->bugfix_version);
                pkt << uint16(_build);
            }
        }

        pkt << uint16(0x0010);                          // unused value (why 10?)
        break;
    }
    }
}

/// Resume patch transfer
bool AuthSocket::_HandleXferResume()
{
    debug_log("Entering _HandleXferResume");

    if (recv_len() < 9)
        return false;

    recv_skip(1);

    uint64 start_pos;
    recv((char *)&start_pos, 8);

    if (patch_ == ACE_INVALID_HANDLE)
    {
        close_connection();
        return false;
    }

    ACE_OFF_T file_size = ACE_OS::filesize(patch_);

    if (file_size == -1 || start_pos >= (uint64)file_size)
    {
        close_connection();
        return false;
    }

    if (ACE_OS::lseek(patch_, start_pos, SEEK_SET) == -1)
    {
        close_connection();
        return false;
    }

    InitPatch();

    return true;
}

/// Cancel patch transfer
bool AuthSocket::_HandleXferCancel()
{
    debug_log("Entering _HandleXferCancel");

    recv_skip(1);
    close_connection();

    return true;
}

/// Accept patch transfer
bool AuthSocket::_HandleXferAccept()
{
    debug_log("Entering _HandleXferAccept");

    recv_skip(1);

    InitPatch();

    return true;
}

void AuthSocket::InitPatch()
{
    PatchHandler* handler = new PatchHandler(ACE_OS::dup(get_handle()), patch_);

    patch_ = ACE_INVALID_HANDLE;

    if (handler->open() == -1)
    {
        handler->close();
        close_connection();
    }
}
