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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Database/DatabaseEnv.h"
#include "ChannelMgr.h"
#include "Group.h"
#include "Guild.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "SpellAuras.h"
#include "Language.h"
#include "Util.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ChatLog.h"
#include "GuildMgr.h"
#include "SocialMgr.h"

enum ChatDenyMask
{
    DENY_NONE       = 0,
    DENY_SAY        = 1,
    DENY_EMOTE      = 2,
    DENY_PARTY      = 4,
    DENY_GUILD      = 8,
    DENY_WHISP      = 16,
    DENY_CHANNEL    = 32,
    DENY_ADDON      = 64,
};

bool WorldSession::processChatmessageFurtherAfterSecurityChecks(std::string& msg, uint32 lang)
{
    if (lang != LANG_ADDON)
    {
        // strip invisible characters for non-addon messages
        if (sWorld.getConfig(CONFIG_CHAT_FAKE_MESSAGE_PREVENTING))
        {
            stripLineInvisibleChars(msg);
            if (msg.empty())
                return false;
        }

        if (sWorld.getConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) && !ChatHandler(this).isValidChatMessage(msg.c_str()))
        {
            sLog.outLog(LOG_DEFAULT, "Player %s (GUID: %u) sent a chatmessage with an invalid link: %s", GetPlayer()->GetName(),
                          GetPlayer()->GetGUIDLow(), msg.c_str());
            if (sWorld.getConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_KICK))
                KickPlayer();
            return false;
        }
    }

    return true;
}

void WorldSession::HandleMessagechatOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,4+4+1);

    uint32 type;
    uint32 lang;

    PlayerTeam team = _player->GetTeam();
    recv_data >> type;
    recv_data >> lang;

    if (type >= MAX_CHAT_MSG_TYPE)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CHAT: Wrong message type received: %u", type);
        return;
    }

    // check if addon channel is disabled
    if (lang == LANG_ADDON && !sWorld.getConfig(CONFIG_ADDON_CHANNEL))
        return;

    std::string channel = "";
    std::string msg = "";
    std::string to = "";

    if (type == CHAT_MSG_WHISPER)
    {
        recv_data >> to;
        if (!normalizePlayerName(to))
        {
            WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size() + 1));
            data << to;
            SendPacket(&data);
            return;
        }
    }
    else if(type == CHAT_MSG_CHANNEL)
        recv_data >> channel;

    recv_data >> msg;
    if (type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
    {
        if (msg.empty())
            return;

        if (ChatHandler(this).ParseCommands(msg.c_str()) > 0)
            return;

        if (!processChatmessageFurtherAfterSecurityChecks(msg, lang))
            return;
    }
    else if (!msg.empty()) // message is CHAT_MSG_AFK or CHAT_MSG_DND, still need to check msg for adequacy if there is message
    {
        if (!processChatmessageFurtherAfterSecurityChecks(msg, lang))
            return;

        if (ChatHandler(this).ContainsNotAllowedSigns(msg))
            return;
    }

    // prevent talking at unknown language (cheating)
    LanguageDesc const* langDesc = GetLanguageDescByID(lang);
    if (!langDesc)
    {
        SendNotification(LANG_UNKNOWN_LANGUAGE);
        return;
    }

    if (langDesc->skill_id != 0 && !_player->HasSkill(langDesc->skill_id))
    {
        // also check SPELL_AURA_COMPREHEND_LANGUAGE (client offers option to speak in that language)
        Unit::AuraList const& langAuras = _player->GetAurasByType(SPELL_AURA_COMPREHEND_LANGUAGE);
        bool foundAura = false;
        for (Unit::AuraList::const_iterator i = langAuras.begin();i != langAuras.end(); ++i)
        {
            if ((*i)->GetModifier()->m_miscvalue == lang)
            {
                foundAura = true;
                break;
            }
        }

        if (!foundAura)
        {
            SendNotification(LANG_NOT_LEARNED_LANGUAGE);
            return;
        }
    }

    // mass mute for players check
    //if (!HasPermissions(PERM_GMT) && sWorld.GetMassMuteTime() && sWorld.GetMassMuteTime() > time(NULL))
    //{
    //    if (sWorld.GetMassMuteReason())
    //        ChatHandler(_player).PSendSysMessage("Server has been muted. Mass mute reason: %s", sWorld.GetMassMuteReason());

    //    return;
    //}

    // LANG_ADDON should not be changed nor be affected by flood control
    if (lang != LANG_ADDON)
    {
        // send in universal language if player in .gmon mode (ignore spell effects)
        if (_player->isGameMaster())
            lang = LANG_UNIVERSAL;
        else
        {
            // send in universal language in two side iteration allowed mode
            if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT))
                lang = LANG_UNIVERSAL;
            else
            {
                switch (type)
                {
                    case CHAT_MSG_PARTY:
                    case CHAT_MSG_RAID:
                    case CHAT_MSG_RAID_LEADER:
                    case CHAT_MSG_RAID_WARNING:
                        // allow two side chat at group channel if two side group allowed
                        if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
                            lang = LANG_UNIVERSAL;
                        break;
                    case CHAT_MSG_GUILD:
                    case CHAT_MSG_OFFICER:
                        // allow two side chat at guild channel if two side guild allowed
                        if (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
                            lang = LANG_UNIVERSAL;
                        break;
                    case CHAT_MSG_BATTLEGROUND:
                    case CHAT_MSG_BATTLEGROUND_LEADER:
                        lang = LANG_UNIVERSAL;
                        break;
                    case CHAT_MSG_CHANNEL:
                    {
                        //if ((channel == "Russian" || channel == "English") && sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_GLOBAL_CHAT))
                        if (channel == "Global" && sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_GLOBAL_CHAT))
                            lang = LANG_UNIVERSAL;
                        break;
                    }
                }
            }

            // but overwrite it by SPELL_AURA_MOD_LANGUAGE auras (only single case used)
            Unit::AuraList const& ModLangAuras = _player->GetAurasByType(SPELL_AURA_MOD_LANGUAGE);
            if (!ModLangAuras.empty())
                lang = ModLangAuras.front()->GetModifier()->m_miscvalue;
        }

        if (type != CHAT_MSG_AFK && type != CHAT_MSG_DND)
            GetPlayer()->UpdateSpeakTime();
    }

    if (!_player->CanSpeak())
    {
        if (lang != LANG_ADDON)
        {
            std::string timeStr = secondsToTimeString(m_muteRemain/1000);
            SendNotification(GetHellgroundString(LANG_WAIT_BEFORE_SPEAKING),timeStr.c_str());
            ChatHandler(_player).PSendSysMessage(LANG_YOUR_CHAT_IS_DISABLED, timeStr.c_str(), m_muteReason.c_str());
        }

        return;
    }

    if (GetPlayer()->GetLevel() < sWorld.getConfig(CONFIG_CHAT_MINIMUM_LVL))
    {
        int mask = 0;
        switch (type)
        {
            case CHAT_MSG_SAY:
            case CHAT_MSG_YELL:
                mask = DENY_SAY;
                break;
            case CHAT_MSG_EMOTE:
            case CHAT_MSG_TEXT_EMOTE:
                mask = DENY_EMOTE;
                break;
            case CHAT_MSG_PARTY:
            case CHAT_MSG_RAID:
            case CHAT_MSG_RAID_LEADER:
            case CHAT_MSG_RAID_WARNING:
            case CHAT_MSG_BATTLEGROUND:
            case CHAT_MSG_BATTLEGROUND_LEADER:
                mask = DENY_PARTY;
                break;
            case CHAT_MSG_GUILD:
            case CHAT_MSG_OFFICER:
                mask = DENY_GUILD;
                break;
            case CHAT_MSG_WHISPER:
            case CHAT_MSG_WHISPER_INFORM:
            case CHAT_MSG_REPLY:
            {
                Player *target = sObjectMgr.GetPlayerInWorld(to.c_str());

                if (target && !target->isGameMaster())
                    mask = DENY_WHISP;
                break;
            }
            case CHAT_MSG_CHANNEL:
            case CHAT_MSG_CHANNEL_JOIN:
            case CHAT_MSG_CHANNEL_LEAVE:
            case CHAT_MSG_CHANNEL_NOTICE:
            case CHAT_MSG_CHANNEL_NOTICE_USER:
                mask = DENY_CHANNEL;
                break;
            case CHAT_MSG_ADDON:
                mask = DENY_ADDON;
                break;
            default:
                break;
        }

        if (lang == LANG_ADDON)
            mask |= DENY_ADDON;

        if (sWorld.getConfig(CONFIG_CHAT_DENY_MASK) & mask)
        {
            SendNotification(LANG_CHAT_TYPE_DISABLED_UNTIL_LVL, sWorld.getConfig(CONFIG_CHAT_MINIMUM_LVL));
            return;
        }
    }

    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_EMOTE:
        case CHAT_MSG_YELL:
        {
            if (!_player->isAlive())
                return;

            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (lang != LANG_ADDON)
                sChatLog.ChatMsg(GetPlayer(), msg, type);

            bool bad_lexics = _player->CheckBadLexics(msg);

            switch (type)
            {
                case CHAT_MSG_SAY:
                    GetPlayer()->Say(msg, lang, bad_lexics);
                    break;
                case CHAT_MSG_EMOTE:
                    GetPlayer()->TextEmote(msg, bad_lexics);
                    break;
                case CHAT_MSG_YELL:
                    GetPlayer()->Yell(msg, lang, bad_lexics);
                    break;
                default:
                    break;
            }

            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_SAY_A, team,_player->GetName(), msg.c_str());
        } 
        break;

        case CHAT_MSG_WHISPER:
        {
            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (lang != LANG_ADDON)
                sChatLog.WhisperMsg(GetPlayer(), to, msg);

            bool bad_lexics = _player->CheckBadLexics(msg);

            Player *player = sObjectMgr.GetPlayerInWorld(to.c_str());
            if (!player || (player->IsHidden() && !_player->isGameMaster()))
            {
                if (player && _player)
                    ChatHandler(player).PSendSysMessage(15580, _player->GetName());
                
                WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size()+1));
                data<<to;
                SendPacket(&data);
                return;
            }

            // can whisper if whispers are open OR we are high gm OR whispers are partially open just for us
            bool canWhisper = player->isAcceptWhispers() ||
                HasPermissions(PERM_GMT_HDEV)/*High GMs can whisper to other GMs*/ ||
                (player->isPartialWhispers() && sSocialMgr.canWhisperToPartialWhisperGM(player->GetGUIDLow(), GetPlayer()->GetGUIDLow()));
            if (!canWhisper)
            {
                ChatHandler(_player).SendSysMessage(_player->GetSession()->GetHellgroundString(LANG_TARGET_GM_WHISPER_OFF));
                /*WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size() + 1));
                data << to;
                SendPacket(&data);*/
                return;
            }

            if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT) && !HasPermissions(PERM_GMT_HDEV) && !player->GetSession()->HasPermissions(PERM_GMT_HDEV))
            {
                uint32 sidea = GetPlayer()->GetTeam();
                uint32 sideb = player->GetTeam();
                if (sidea != sideb)
                {
                    WorldPacket data(SMSG_CHAT_PLAYER_NOT_FOUND, (to.size()+1));
                    data<<to;
                    SendPacket(&data);
                    return;
                }
            }

            if (_player->GetLevel() < 70)
            {
                if (sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_WHISPER_MSG) && lang != LANG_ADDON)
                {
                    uint32 accId = _player->GetSession()->GetAccountId();
                    if (!sSocialMgr.HasInList(accId, player->GetGUIDLow()))
                    {
                        if (uint32 cooldown = sSocialMgr.GetWhisperMsgCooldown(accId, player->GetGUIDLow()))
                        {
                            // if we got here - then both times are greater than thetime
                            std::string timeStr = secondsToTimeString(cooldown);
                            SendNotification(GetHellgroundString(LANG_WAIT_BEFORE_NEW_WHISPER), timeStr.c_str());
                            ChatHandler(_player).PSendSysMessage(LANG_WAIT_BEFORE_NEW_WHISPER, timeStr.c_str());
                            return;
                        }

                        sSocialMgr.WhisperMsgUsed(accId, player->GetGUIDLow());
                    }
                }
            }

            GetPlayer()->Whisper(msg, lang,player->GetGUID(), bad_lexics);
            if (!player->CanSpeak())
                ChatHandler(_player).SendSysMessage("The player you are whispering to is muted and can not reply.");
        } 
        break;

        case CHAT_MSG_PARTY:
        {
            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (lang != LANG_ADDON)
                sChatLog.PartyMsg(GetPlayer(), msg);

            Group *group = GetPlayer()->GetOriginalGroup();
            if (!group && (!(group = GetPlayer()->GetGroup()) || group->isBGGroup()))
                return;            

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_PARTY, lang, NULL, 0, msg.c_str(),NULL);

            if (_player->CheckBadLexics(msg))
                _player->SendPacketToSelf(&data);
            else
                group->BroadcastPacket(&data, false, group->GetMemberGroup(GetPlayer()->GetGUID()));

            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_PARTY_A, team, _player->GetName(), msg.c_str());
        }
        break;
        case CHAT_MSG_GUILD:
        {
            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (lang != LANG_ADDON)
                sChatLog.GuildMsg(GetPlayer(), msg, true);

            if (GetPlayer()->GetGuildId())
            {
                Guild *guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
                if (guild)
                    guild->BroadcastToGuild(this, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL, _player->CheckBadLexics(msg));
                    
                if (lang != LANG_ADDON)
                {
                    std::string widename = std::string(_player->GetName()) + " " + guild->GetName();
                    sLog.outChat(LOG_CHAT_GUILD_A, team, widename.c_str(), msg.c_str());
                }
            }
            
            break;
        }
        case CHAT_MSG_OFFICER:
        {
            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (lang != LANG_ADDON)
                sChatLog.GuildMsg(GetPlayer(), msg, type);

            if (msg == "Lexics Cutter Detected Bad Words")
                return;

            if (GetPlayer()->GetGuildId())
            {
                Guild *guild = sGuildMgr.GetGuildById(GetPlayer()->GetGuildId());
                if (guild)
                    guild->BroadcastToOfficers(this, msg, lang == LANG_ADDON ? LANG_ADDON : LANG_UNIVERSAL, _player->CheckBadLexics(msg));

                if (lang != LANG_ADDON)
                {
                    std::string widename = std::string(_player->GetName()) + " oficer " + guild->GetName();
                    sLog.outChat(LOG_CHAT_GUILD_A, team, widename.c_str(), msg.c_str());
                }
            }
            break;
        }
        case CHAT_MSG_RAID:
        {
            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (lang != LANG_ADDON)
                sChatLog.RaidMsg(GetPlayer(), msg, type);

            Group *group = GetPlayer()->GetOriginalGroup();
            if (!group && (!(group = GetPlayer()->GetGroup()) || group->isBGGroup()))
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_RAID, lang, "", 0, msg.c_str(),NULL);

            if (_player->CheckBadLexics(msg))
                _player->SendPacketToSelf(&data);
            else
                group->BroadcastPacket(&data, false);

            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_RAID_A, team, _player->GetName(), msg.c_str());
        } break;
        case CHAT_MSG_RAID_LEADER:
        {
            if (msg.empty())
                break;

            if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            if (lang != LANG_ADDON)
                sChatLog.RaidMsg(GetPlayer(), msg, type);

            Group *group = GetPlayer()->GetOriginalGroup();
            if (!group && !(group = GetPlayer()->GetGroup()) || group->isBGGroup() || !group->isRaidGroup() || !group->IsLeader(GetPlayer()->GetGUID()))
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_RAID_LEADER, lang, "", 0, msg.c_str(),NULL);

            if (_player->CheckBadLexics(msg))
                _player->SendPacketToSelf(&data);
            else
                group->BroadcastPacket(&data, false);

            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_RAID_A, team, _player->GetName(), msg.c_str());
        } break;
        case CHAT_MSG_RAID_WARNING:
        {
            if (lang != LANG_ADDON)
                sChatLog.RaidMsg(GetPlayer(), msg, type);

            Group *group = GetPlayer()->GetGroup();
            if (!group || !group->isRaidGroup() || !(group->IsLeader(GetPlayer()->GetGUID()) || group->IsAssistant(GetPlayer()->GetGUID())) || group->isBGGroup())
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_RAID_WARNING, lang, "", 0, msg.c_str(),NULL);

            if (_player->CheckBadLexics(msg))
                _player->SendPacketToSelf(&data);
            else
                group->BroadcastPacket(&data, false);

            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_RAID_A, team, _player->GetName(), msg.c_str());
        } break;

        case CHAT_MSG_BATTLEGROUND:
        {
            if (lang != LANG_ADDON)
                sChatLog.BattleGroundMsg(GetPlayer(), msg, type);

            Group *group = GetPlayer()->GetGroup();
            if (!group || !group->isBGGroup())
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_BATTLEGROUND, lang, "", 0, msg.c_str(),NULL);

            if (_player->CheckBadLexics(msg))
                _player->SendPacketToSelf(&data);
            else
                group->BroadcastPacket(&data, false);

            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_BG_A, team, _player->GetName(), msg.c_str());
        } break;

        case CHAT_MSG_BATTLEGROUND_LEADER:
        {
            if (lang != LANG_ADDON)
                sChatLog.BattleGroundMsg(GetPlayer(), msg, type);

            Group *group = GetPlayer()->GetGroup();
            if (!group || !group->isBGGroup() || !group->IsLeader(GetPlayer()->GetGUID()))
                return;

            WorldPacket data;
            ChatHandler::FillMessageData(&data, this, CHAT_MSG_BATTLEGROUND_LEADER, lang, "", 0, msg.c_str(),NULL);

            if (_player->CheckBadLexics(msg))
                _player->SendPacketToSelf(&data);
            else
                group->BroadcastPacket(&data, false);

            if (lang != LANG_ADDON)
                sLog.outChat(LOG_CHAT_BG_A, team, _player->GetName(), msg.c_str());
        } break;

        case CHAT_MSG_CHANNEL:
        {
			if (_player->IsTrollmuted())
			{
				if (WorldSession* ses = _player->GetSession())
				{
					std::string timeStr = ses->secondsToTimeString(ses->m_trollMuteRemain / 1000);
					ses->SendNotification(ses->GetHellgroundString(LANG_WAIT_BEFORE_SPEAKING), timeStr.c_str());
					ChatHandler(_player).PSendSysMessage(LANG_YOUR_CHAT_IS_DISABLED, timeStr.c_str(), ses->m_trollmuteReason.c_str());
					return;
				}
			}

			if (msg.empty())
				break;

			if (ChatHandler(this).ContainsNotAllowedSigns(msg))
                return;

            //if (channel != "Russian" && channel != "English")
            if (channel != "Global")
            {
                ChatHandler(_player).PSendSysMessage(LANG_USE_LFG);
                return;
            }

            // LFG from 20 lvl
            if (sWorld.isEasyRealm() && GetPlayer()->GetLevel() < sWorld.getConfig(CONFIG_LFG_FROM_LEVEL)) {
                ChatHandler(_player).PSendSysMessage(LANG_CHANNEL_LEVEL_RESTRICTED, sWorld.getConfig(CONFIG_LFG_FROM_LEVEL));
                return;
            }

            // colored messages! !msg.rfind("|cff", 0) == 0 && 
            // not coloded, adding a dot
			//std::string original_msg = msg;

            bool bad_lexics = _player->CheckBadLexics(msg);
            
            // replace with ICONS!
			//if (!_player->IsPlayerCustomFlagged(PL_CUSTOM_DISABLE_GH_DOTS) && _player->IsGuildHouseOwnerMember())
			//{	
			//	if (Guild* guild = sGuildMgr.GetGuildById(_player->GetGuildId()))
			//	{
			//		if (guild->GetRank(_player->GetGUIDLow()) <= 1)
			//			msg = "|cff00ff00¤|r " + msg;
			//		else
			//			msg = "|cff00ff00@!|r " + msg;
			//	}				
			//}

            if (ChannelMgr* cMgr = channelMgr(team))
            {
                if (Channel *chn = cMgr->GetChannel(channel, _player))
                {
                    if (chn->IsLFG())
                    {
                        if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_LFG_MSG))
                        {
                            uint32 accId = _player->GetSession()->GetAccountId();
                            if (uint32 cooldown = sSocialMgr.GetLfgMsgCooldown(accId))
                            {
                                // if we got here - then both times are greater than thetime
                                std::string timeStr = secondsToTimeString(cooldown);
                                SendNotification(GetHellgroundString(LANG_WAIT_BEFORE_LFG_MSG),timeStr.c_str());
                                ChatHandler(_player).PSendSysMessage(LANG_WAIT_BEFORE_LFG_MSG, timeStr.c_str());
                                return;
                            }
                    
                            sSocialMgr.LfgMsgUsed(accId);
                        }

                        // copy message to other-locale-channel of the same faction
                        channel = channel == "LookingForGroup" ? "Поиск спутников" : "LookingForGroup";
                        if (Channel *chn2 = cMgr->GetChannel(channel, _player))
                            chn2->Say(_player->GetGUID(), msg.c_str(), lang, bad_lexics);
                    }
                    else if (chn->HasFlag(CHANNEL_FLAG_GLOBAL))
                    {
                        if (lang != LANG_ADDON && sWorld.getConfig(CONFIG_CHARGES_COOLDOWN_GLOBAL_MSG))
                        {
                            uint32 accId = _player->GetSession()->GetAccountId();
                            if (uint32 cooldown = sSocialMgr.GetGlobalMsgCooldown(accId))
                            {
                                // if we got here - then both times are greater than thetime
                                std::string timeStr = secondsToTimeString(cooldown);
                                SendNotification(GetHellgroundString(LANG_WAIT_BEFORE_GLOBAL_MSG), timeStr.c_str());
                                ChatHandler(_player).PSendSysMessage(LANG_WAIT_BEFORE_GLOBAL_MSG, timeStr.c_str());
                                return;
                            }

                            sSocialMgr.GlobalMsgUsed(accId);
                        }

                        if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL) && sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_GLOBAL_CHAT))
                        {
                            // copy message to other-locale-channel of the opposite faction
                            if (ChannelMgr* cMgr2 = channelMgr(team == ALLIANCE ? HORDE : ALLIANCE))
                            {
                                if (Channel *chn2 = cMgr2->GetJoinChannel(channel, 0))
                                {
                                    chn2->Say(_player->GetGUID(), msg.c_str(), lang, bad_lexics);
                                }
                            }
                        }
                    }

                    chn->Say(_player->GetGUID(), msg.c_str(), lang, bad_lexics);
                }
            }

            if (lang != LANG_ADDON)
                sChatLog.ChannelMsg(GetPlayer(), channel, msg, bad_lexics);

            // print "Contact via Discord" message
            std::wstring wmsg;

            if (!Utf8toWStr(msg, wmsg))
                return;

            wstrToLower(wmsg);

            std::map<std::wstring, int> keywords = {
                {L"gm", LANG_USE_DISCORD},
                {L"gms", LANG_USE_DISCORD},
                {L"admin", LANG_USE_DISCORD},
                {L"admins", LANG_USE_DISCORD},
                {L"гм", LANG_USE_DISCORD},
                {L"гмы", LANG_USE_DISCORD},
                {L"админ", LANG_USE_DISCORD},
                {L"админы", LANG_USE_DISCORD},
                {L"ninja", 15586},
                {L"кидала", 15586},
                {L"кидок", 15586}
            };

            for (const auto& pair : keywords) {
                if (wmsg.find(pair.first) != std::wstring::npos) {
                    ChatHandler(_player).PSendSysMessage(pair.second);
                    break;
                }
            }
        } break;

        case CHAT_MSG_AFK:
        {
            if ((msg.empty() || !_player->isAFK()) && !_player->IsInCombat())
            {
                if (!msg.empty())
                {
                    std::string to = "AFK";
                    if (lang != LANG_ADDON)
                        sChatLog.WhisperMsg(GetPlayer(), to, msg);

                    if (_player->CheckBadLexics(msg))
                        return;
                }

                if (!_player->isAFK())
                {
                    if (msg.empty())
                        msg  = GetHellgroundString(LANG_PLAYER_AFK_DEFAULT);
                    _player->afkMsg = msg;
                }
                _player->ToggleAFK();
                //if (_player->isAFK() && _player->isDND())
                //    _player->ToggleDND();
            }
        } break;

        case CHAT_MSG_DND:
        {
            if (msg.empty() || !_player->isDND())
            {
                if (!msg.empty())
                {
                    std::string to = "DND";
                    if (lang != LANG_ADDON)
                        sChatLog.WhisperMsg(GetPlayer(), to, msg);

                    if (_player->CheckBadLexics(msg))
                        return;
                }

                if (!_player->isDND())
                {
                    if (msg.empty())
                        msg  = GetHellgroundString(LANG_PLAYER_DND_DEFAULT);
                    _player->dndMsg = msg;
                }
                _player->ToggleDND();
                if (_player->isDND() && _player->isAFK())
                    _player->ToggleAFK();
            }
        } break;

        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: CHAT: unknown message type %u, lang: %u", type, lang);
            break;
    }
}

void WorldSession::HandleEmoteOpcode(WorldPacket & recv_data)
{
    if (!GetPlayer()->isAlive() || GetPlayer()->isPossessed() || GetPlayer()->isCharmed())
        return;

    // arena spectator
    if (GetPlayer()->HasAura(55194))
        return;

    CHECK_PACKET_SIZE(recv_data,4);

    uint32 emote;
    recv_data >> emote;
    GetPlayer()->HandleEmoteCommand(emote);
}

namespace Hellground
{
    class EmoteChatBuilder
    {
        public:
            EmoteChatBuilder(Player const& pl, uint32 text_emote, uint32 emote_num, Unit const* target)
                : i_player(pl), i_text_emote(text_emote), i_emote_num(emote_num), i_target(target) {}

            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* nam = i_target ? i_target->GetNameForLocaleIdx(loc_idx) : NULL;
                uint32 namlen = (nam ? strlen(nam) : 0) + 1;

                data.Initialize(SMSG_TEXT_EMOTE, (20+namlen));
                data << i_player.GetGUID();
                data << uint32(i_text_emote);
                data << i_emote_num;
                data << uint32(namlen);
                if( namlen > 1 )
                    data.append(nam, namlen);
                else
                    data << (uint8)0x00;
            }

        private:
            Player const& i_player;
            uint32        i_text_emote;
            uint32        i_emote_num;
            Unit const*   i_target;
    };
}                                                           // namespace Hellground

void WorldSession::HandleTextEmoteOpcode(WorldPacket & recv_data)
{
    Player * player = GetPlayer();
    if (!player->isAlive() || player->isPossessed() || player->isCharmed())
        return;

    // arena spectator
    if (player->HasAura(55194))
        return;

    if (!player->CanSpeak())
    {
        std::string timeStr = secondsToTimeString(m_muteRemain/1000);
        SendNotification(GetHellgroundString(LANG_WAIT_BEFORE_SPEAKING),timeStr.c_str());
        ChatHandler(player).PSendSysMessage(LANG_YOUR_CHAT_IS_DISABLED, timeStr.c_str(), m_muteReason.c_str());
        return;
    }

    CHECK_PACKET_SIZE(recv_data, 4+4+8);

    uint32 text_emote, emoteNum;
    uint64 guid;

    recv_data >> text_emote;
    recv_data >> emoteNum;
    recv_data >> guid;

    EmotesTextEntry const *em = sEmotesTextStore.LookupEntry(text_emote);
    if (!em)
        return;

    uint32 emote_anim = em->textid;

    switch (emote_anim)
    {
        case EMOTE_STATE_SLEEP:
        case EMOTE_STATE_SIT:
        case EMOTE_STATE_KNEEL:
        case EMOTE_ONESHOT_NONE:
            break;
        default:
        {
            // in feign death state allowed only text emotes.
            if (!player->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
                player->HandleEmoteCommand(emote_anim);
            break;
        }
    }

    Unit* unit = player->GetMap()->GetUnit(guid);

    Hellground::EmoteChatBuilder emote_builder(*player, text_emote, emoteNum, unit);
    Hellground::LocalizedPacketDo<Hellground::EmoteChatBuilder > emote_do(emote_builder);
    Hellground::CameraDistWorker<Hellground::LocalizedPacketDo<Hellground::EmoteChatBuilder > > emote_worker(player, sWorld.getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), emote_do);
    Cell::VisitWorldObjects(player, emote_worker,  sWorld.getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE));

    //Send scripted event call
    if (unit && unit->GetTypeId() == TYPEID_UNIT)
    {
        if (((Creature*)unit)->AI())
            ((Creature*)unit)->AI()->ReceiveEmote(player, text_emote);

        sScriptMgr.OnReceiveEmote(GetPlayer(), (Creature*)unit, text_emote);
    }
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+1);

    uint64 iguid;
    uint8 unk;
    //sLog.outDebug("WORLD: Received CMSG_CHAT_IGNORED");

    recv_data >> iguid;
    recv_data >> unk;                                       // probably related to spam reporting

    Player *player = sObjectMgr.GetPlayerInWorld(iguid);
    if (!player || !player->GetSession())
        return;

    WorldPacket data;
    ChatHandler::FillMessageData(&data, this, CHAT_MSG_IGNORED, LANG_UNIVERSAL, NULL, GetPlayer()->GetGUID(), GetPlayer()->GetName(),NULL);
    player->SendPacketToSelf(&data);
}

void WorldSession::HandleChannelDeclineInvite(WorldPacket &recvPacket)
{
    sLog.outDebug("Opcode %u", recvPacket.GetOpcode());
}
