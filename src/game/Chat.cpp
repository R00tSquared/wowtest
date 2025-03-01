﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Chat.h"
#include "MapManager.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "TicketMgr.h"

bool ChatHandler::load_command_table = true;

ChatCommand * ChatHandler::getCommandTable()
{
    //static ChatCommand accountSetCommandTable[] =
    //{
    //    { "addon",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleAccountSetAddonCommand,     "", NULL },
    //    { "permissions",    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleAccountSetPermissionsCommand,"", NULL },
    //    { "password",       PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleAccountSetPasswordCommand,  "", NULL },
    //    { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    //};

    static ChatCommand accountAnnounceCommandTable[] =
    {
        { "battleground",   PERM_GMT,        PERM_CONSOLE, false, &ChatHandler::HandleAccountBattleGroundAnnCommand,   "", NULL },
        { "bg",             PERM_GMT,        PERM_CONSOLE, false, &ChatHandler::HandleAccountBattleGroundAnnCommand,   "", NULL },
        //{ "broadcast",      PERM_GMT,        PERM_CONSOLE, false, &ChatHandler::HandleAccountAnnounceBroadcastCommand, "", NULL },
        { "guild",          PERM_GMT,        PERM_CONSOLE, false, &ChatHandler::HandleAccountGuildAnnToggleCommand,    "", NULL },
        { NULL,             0,                  0,            false,  NULL,                                               "", NULL }
    };

    /*static ChatCommand accountFriendCommandTable[] =
    {
        { "add",            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleAccountFriendAddCommand,    "", NULL },
        { "delete",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleAccountFriendDeleteCommand, "", NULL },
        { "list",           PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleAccountFriendListCommand,   "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };*/

    static ChatCommand accountCommandTable[] =
    {
        { "announce",       PERM_GMT,    PERM_CONSOLE, false,    NULL,                                                     "", accountAnnounceCommandTable },
        { "create",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleAccountCreateCommand,                 "", NULL },        
        { "delete",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleAccountDeleteCommand,                 "", NULL },
        //{ "friend",         PERM_ADM,       PERM_CONSOLE, true,   NULL,                                                     "", accountFriendCommandTable },
        { "gann",           PERM_GMT,    PERM_CONSOLE, false,  &ChatHandler::HandleAccountGuildAnnToggleCommand,         "", NULL },
        //{ "bgmarks",        PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleAccountBGMarksCommand,                "", NULL },
        { "bones",          PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleAccountBonesHideCommand,              "", NULL },
        //{ "log",            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleAccountSpecialLogCommand,             "", NULL },
        { "onlinelist",     PERM_CONSOLE,   PERM_CONSOLE, true,   &ChatHandler::HandleAccountOnlineListCommand,             "", NULL },
        //{ "set",            PERM_ADM,       PERM_CONSOLE, true,   NULL,                                                     "", accountSetCommandTable },
        { "whisp",          PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleAccountWhispLogCommand,               "", NULL },
        { "",               PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleAccountCommand,                       "", NULL },
        { NULL,             0,              0,            false,  NULL,                                                     "", NULL }
    };

    static ChatCommand serverSetCommandTable[] =
    {
        { "difftime",       PERM_CONSOLE,   PERM_CONSOLE, true,   &ChatHandler::HandleServerSetDiffTimeCommand,   "", NULL },
        { "motd",           PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerSetMotdCommand,       "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand sendCommandTable[] =
    {
        { "items",          PERM_ADM,         PERM_CONSOLE, true,   &ChatHandler::HandleSendItemsCommand,           "", NULL },
        { "mail",           PERM_HIGH_GMT,   PERM_CONSOLE, true,   &ChatHandler::HandleSendMailCommand,            "", NULL },
        { "message",        PERM_ADM,         PERM_CONSOLE, true,   &ChatHandler::HandleSendMessageCommand,         "", NULL },
        { "money",          PERM_ADM,         PERM_CONSOLE, true,   &ChatHandler::HandleSendMoneyCommand,           "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand serverIdleRestartCommandTable[] =
    {
        { "cancel",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerIdleRestartCommand,   "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand serverIdleShutdownCommandTable[] =
    {
        { "cancel",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerIdleShutDownCommand,  "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand serverRestartCommandTable[] =
    {
        { "cancel",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerRestartCommand,       "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand serverShutdownCommandTable[] =
    {
        { "cancel",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerShutDownCommand,      "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand serverCommandTable[] =
    {
        { "corpses",        PERM_ADM,        PERM_CONSOLE, true,   &ChatHandler::HandleServerCorpsesCommand,       "", NULL },
        { "exit",           PERM_CONSOLE,   PERM_CONSOLE, true,   &ChatHandler::HandleServerExitCommand,          "", NULL },
        { "idlerestart",    PERM_ADM,        PERM_CONSOLE, true,   NULL,                                           "", serverIdleRestartCommandTable },
        { "idleshutdown",   PERM_ADM,        PERM_CONSOLE, true,   NULL,                                           "", serverShutdownCommandTable },
        { "info",           PERM_PLAYER,    PERM_CONSOLE, true,   &ChatHandler::HandleServerInfoCommand,          "", NULL },
        //{ "events",         PERM_PLAYER,    PERM_CONSOLE, true,   &ChatHandler::HandleServerEventsCommand,        "", NULL },
        { "kickall",        PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleServerKickallCommand,       "", NULL },
        //{ "motd",           PERM_PLAYER,    PERM_CONSOLE, true,   &ChatHandler::HandleServerMotdCommand,          "", NULL },
        { "mute",           PERM_ADM,        PERM_CONSOLE, true,   &ChatHandler::HandleServerMuteCommand,          "", NULL },
        //{ "pvp",            PERM_GMT,    PERM_CONSOLE, false,  &ChatHandler::HandleServerPVPCommand,           "", NULL },
        { "restart",        PERM_ADM,        PERM_CONSOLE, true,   NULL,                                           "", serverRestartCommandTable },
        { "rollshutdown",   PERM_ADM,        PERM_CONSOLE, true,   &ChatHandler::HandleServerRollShutDownCommand,  "", NULL},
        { "set",            PERM_ADM,        PERM_CONSOLE, true,   NULL,                                           "", serverSetCommandTable },
        { "shutdown",       PERM_ADM,        PERM_CONSOLE, true,   NULL,                                           "", serverShutdownCommandTable },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand mmapCommandTable[] =
    {
        { "path",           PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmapPathCommand,            "", NULL },
        { "loc",            PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmapLocCommand,             "", NULL },
        { "loadedtiles",    PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmapLoadedTilesCommand,     "", NULL },
        { "stats",          PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmapStatsCommand,           "", NULL },
        { "testarea",       PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmapTestArea,               "", NULL },
        { "follow",         PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmapFollowArea,             "", NULL },
        { "offmesh",        PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmapOffsetCreateCommand,    "", NULL },
        { "",               PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleMmap,                       "", NULL },
        { NULL,             0,              0,            false, NULL,                                           "", NULL }
    };

    static ChatCommand modifyCommandTable[] =
    {
        { "addtitle",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyAddTitleCommand,      "", NULL },
        { "arena",          PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyArenaCommand,         "", NULL },
        { "aspeed",         PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyASpeedCommand,        "", NULL },
        { "awardtitle",     PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyAwardTitleCommand,    "", NULL },
        { "bit",            PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyBitCommand,           "", NULL },
        { "bwalk",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyBWalkCommand,         "", NULL },
        { "drunk",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyDrunkCommand,         "", NULL },
        { "energy",         PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyEnergyCommand,        "", NULL },
        { "faction",        PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyFactionCommand,       "", NULL },
        { "fly",            PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyFlyCommand,           "", NULL },
        { "gender",         PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyGenderCommand,        "", NULL },
        { "honor",          PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyHonorCommand,         "", NULL },
        { "hp",             PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyHPCommand,            "", NULL },
        { "mana",           PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyManaCommand,          "", NULL },
        { "money",          PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyMoneyCommand,         "", NULL },
        { "morph",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyMorphCommand,         "", NULL },

        { "truemorph",      PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleTrueMorphCommand,           "", NULL },

        { "mount",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyMountCommand,         "", NULL },
        { "rage",           PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyRageCommand,          "", NULL },
        { "removetitle",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyRemoveTitleCommand,   "", NULL },
        { "rep",            PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyRepCommand,           "", NULL },
        { "scale",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyScaleCommand,         "", NULL },
        { "speed",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifySpeedCommand,         "", NULL },
        { "spell",          PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifySpellCommand,         "", NULL },
        { "standstate",     PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifyStandStateCommand,    "", NULL },
        { "swim",           PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleModifySwimCommand,          "", NULL },

        { "strength",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyStrengthCommand,      "", NULL },
        { "agility",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyAgilityCommand,       "", NULL },
        { "stamina",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyStaminaCommand,       "", NULL },
        { "intellect",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyIntellectCommand,     "", NULL },
        { "spirit",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifySpiritCommand,        "", NULL },
        { "armor",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyArmorCommand,         "", NULL },
        { "holy",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyHolyCommand,          "", NULL },
        { "fire",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyFireCommand,          "", NULL },
        { "nature",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyNatureCommand,        "", NULL },
        { "frost",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyFrostCommand,         "", NULL },
        { "shadow",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyShadowCommand,        "", NULL },
        { "arcane",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyArcaneCommand,        "", NULL },
        { "ap",             PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyMeleeApCommand,       "", NULL },
        { "rangeap",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyRangedApCommand,      "", NULL },
        { "spellpower",     PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifySpellPowerCommand,    "", NULL },
        { "crit",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyMeleeCritCommand,     "", NULL },
        { "rangecrit",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyRangedCritCommand,    "", NULL },
        { "spellcrit",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifySpellCritCommand,     "", NULL },
        { "mainspeed",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyMainSpeedCommand,     "", NULL },
        { "offspeed",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyOffSpeedCommand,      "", NULL },
        { "rangespeed",     PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyRangedSpeedCommand,   "", NULL },
        { "castspeed",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyCastSpeedCommand,     "", NULL },
        { "block",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyBlockCommand,         "", NULL },
        { "dodge",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyDodgeCommand,         "", NULL },
        { "parry",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyParryCommand,         "", NULL },
        { "combreach",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyCrCommand,            "", NULL },
        { "boundrad",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleModifyBrCommand,            "", NULL },

        { "titles",         PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyKnownTitlesCommand,   "", NULL },
        { "tp",             PERM_ADM,        PERM_CONSOLE, false,  &ChatHandler::HandleModifyTalentCommand,        "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand wpCommandTable[] =
    {
        { "add",            PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWpAddCommand,                "", NULL },
        { "event",          PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWpEventCommand,              "", NULL },
        { "load",           PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWpLoadPathCommand,           "", NULL },
        { "modify",         PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWpModifyCommand,             "", NULL },
        { "reloadpath",     PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWpReloadPath,                "", NULL },
        { "show",           PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWpShowCommand,               "", NULL },
        { "tofile",         PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWPToFileCommand,             "", NULL },
        { "unload",         PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleWpUnLoadPathCommand,         "", NULL },
        { NULL,             0,              0,            false, NULL,                                            "", NULL }
    };


    static ChatCommand banCommandTable[] =
    {
        { "account",        PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanAccountCommand,           "", NULL },
        { "character",      PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanCharacterCommand,         "", NULL },
        { "ip",             PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanIPCommand,                "", NULL },
        { "arena3v3",       PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanArena3v3Command,                "", NULL },
        { NULL,             0,              0,            false, NULL,                                            "", NULL }
    };

    static ChatCommand baninfoCommandTable[] =
    {
        { "account",        PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanInfoAccountCommand,       "", NULL },
        { "character",      PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanInfoCharacterCommand,     "", NULL },
        { "ip",             PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanInfoIPCommand,            "", NULL },
        { "arena3v3",       PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanInfoArena3v3Command,            "", NULL },
        { NULL,             0,              0,            false, NULL,                                            "", NULL }
    };

    static ChatCommand banlistCommandTable[] =
    {
        { "account",        PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanListAccountCommand,       "", NULL },
        { "character",      PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanListCharacterCommand,     "", NULL },
        { "ip",             PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanListIPCommand,            "", NULL },
        { "arena3v3",       PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleBanListArena3v3Command,            "", NULL },
        { NULL,             0,              0,            false, NULL,                                            "", NULL }
    };

    static ChatCommand unbanCommandTable[] =
    {
        { "account",        PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleUnBanAccountCommand,         "", NULL },
        { "character",      PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleUnBanCharacterCommand,       "", NULL },
        { "ip",             PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleUnBanIPCommand,              "", NULL },
        { "arena3v3",       PERM_GMT,       PERM_CONSOLE, true,  &ChatHandler::HandleUnBanArena3v3Command,              "", NULL },
        { NULL,             0,              0,            false, NULL,                                            "", NULL }
    };

    static ChatCommand debugPlayCommandTable[] =
    {
        { "cinematic",      PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleDebugPlayCinematicCommand,   "", NULL },
        { "sound",          PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleDebugPlaySoundCommand,       "", NULL },
        { "rangesound",     PERM_ADM,  PERM_CONSOLE, false, &ChatHandler::HandleDebugPlayRangeSound,         "", NULL },
        { NULL,             0,              0,            false, NULL,                                            "", NULL }
    };

    static ChatCommand debugSendCommandTable[] =
    {
        { "bgopcode",       PERM_GMT_DEV,   PERM_CONSOLE, false, &ChatHandler::HandleDebugSendBattlegroundOpcodes,    "", NULL },
        { "buyerror",       PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendBuyErrorCommand,        "", NULL },
        { "channelnotify",  PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendChannelNotifyCommand,   "", NULL },
        { "chatmessage",    PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendChatMsgCommand,         "", NULL },
        { "equiperror",     PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendEquipErrorCommand,      "", NULL },
        { "opcode",         PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendOpcodeCommand,          "", NULL },
        { "petspellinit",   PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendPetSpellInitCommand,    "", NULL },
        { "poi",            PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendPoiCommand,             "", NULL },
        { "qpartymsg",      PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendQuestPartyMsgCommand,   "", NULL },
        { "qinvalidmsg",    PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendQuestInvalidMsgCommand, "", NULL },
        { "sellerror",      PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendSellErrorCommand,       "", NULL },
        { "spellfail",      PERM_ADM,       PERM_CONSOLE, false, &ChatHandler::HandleDebugSendSpellFailCommand,       "", NULL },
        { NULL,             0,              0,            false, NULL,                                                "", NULL }
    };

    static ChatCommand debugCommandTable[] =
    {
        { "addformation",   PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugAddFormationToFileCommand, "", NULL },
        { "anim",           PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugAnimCommand,               "", NULL },
        { "arena",          PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugArenaCommand,              "", NULL },
        { "bg",             PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugBattleGroundCommand,       "", NULL },
        { "bossemote",      PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugBossEmoteCommand,          "", NULL },
        { "formation",      PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugFormationCommand,          "", NULL },
        { "getitemstate",   PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugGetItemState,              "", NULL },
        { "getinstdata",    PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugGetInstanceDataCommand,    "", NULL },
        { "getinstdata64",  PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugGetInstanceData64Command,  "", NULL },
        { "getvalue",       PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugGetValue,                  "", NULL },
        { "hostilelist",    PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugHostileRefList,            "", NULL },
        { "lootrecipient",  PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugGetLootRecipient,          "", NULL },
        { "joinbg",         PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugJoinBG,                    "", NULL },
        { "Mod32Value",     PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugMod32Value,                "", NULL },
        { "play",           PERM_ADM,PERM_CONSOLE, false,  NULL,                                               "", debugPlayCommandTable },
        { "poolstats",      PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleGetPoolObjectStatsCommand,      "", NULL },
        { "rel",            PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleRelocateCreatureCommand,        "", NULL },
        { "send",           PERM_ADM,PERM_CONSOLE, false,  NULL,                                               "", debugSendCommandTable },
        { "setinstdata",    PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugSetInstanceDataCommand,    "", NULL },
        { "setinstdata64",  PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugSetInstanceData64Command,  "", NULL },
        { "setitemflag",    PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugSetItemFlagCommand,        "", NULL },
        { "setspellspeed",  PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugSetSpellSpeed,             "", NULL },
        { "setvalue",       PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugSetValue,                  "", NULL },
        { "combatstats",    PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugShowCombatStats,           "", NULL },
        { "threatlist",     PERM_ADM,PERM_CONSOLE, false,  &ChatHandler::HandleDebugThreatList,                "", NULL },
        { "printstate",     PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleDebugUnitState,                 "", NULL },
        { "update",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleDebugUpdate,                    "", NULL },
        { "uws",            PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleDebugUpdateWorldStateCommand,   "", NULL },
        { "cooldowns",      PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleDebugCooldownsCommand,   "", NULL },
        { "vmap",           PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleDebugVmapsCommand,              "", NULL },
        { "water",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleTestGridNeedsVMapWaterCommand,  "", NULL },
        { "creaturecheck",  PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCreatureCheckCommand,           "", NULL },
        { "gocheck",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleGameobjectCheckCommand,         "", NULL },
        { "newpos",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleDebugCheckNewPosCommand,         "", NULL },
        { "goduplicates",   PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleGameobjectDuplicateCommand,     "", NULL },
        { "creduplicates",  PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCreatureDuplicateCommand,       "", NULL },
        { "gopool",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleGameobjectPoolCommand,          "", NULL },
        { NULL,             0,              0,            false,  NULL,                                               "", NULL }
    };

    static ChatCommand eventCommandTable[] =
    {
        { "activelist",     PERM_ADM,   PERM_CONSOLE, true,   &ChatHandler::HandleEventActiveListCommand,     "", NULL },
        { "award",          PERM_ADM,  PERM_CONSOLE, true, &ChatHandler::HandleEventAwardCommand, "", NULL },
        { "start",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleEventStartCommand,          "", NULL },
        { "stop",           PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleEventStopCommand,           "", NULL },
        { "",               PERM_ADM,   PERM_CONSOLE, true,   &ChatHandler::HandleEventInfoCommand,           "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand learnCommandTable[] =
    {
        { "all",            PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllCommand,            "", NULL },
        { "all_crafts",     PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllCraftsCommand,      "", NULL },
        { "all_default",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllDefaultCommand,     "", NULL },
        { "all_gm",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllGMCommand,          "", NULL },
        { "all_lang",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllLangCommand,        "", NULL },
        { "all_myclass",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllMyClassCommand,     "", NULL },
        { "all_myspells",   PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllMySpellsCommand,    "", NULL },
        { "all_mytalents",  PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllMyTalentsCommand,   "", NULL },
        { "all_recipes",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnAllRecipesCommand,     "", NULL },
        { "",               PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleLearnCommand,               "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand reloadCommandTable[] =
    {
        { "all",            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllCommand,           "", NULL },
        { "all_item",       PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllItemCommand,       "", NULL },
        { "all_locales",    PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllLocalesCommand,    "", NULL },
        { "all_loot",       PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllLootCommand,       "", NULL },
        { "all_npc",        PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllNpcCommand,        "", NULL },
        { "all_quest",      PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllQuestCommand,      "", NULL },
        { "all_scripts",    PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllScriptsCommand,    "", NULL },
        { "all_spell",      PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadAllSpellCommand,      "", NULL },
        { "config",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleReloadConfigCommand,        "", NULL },
        { "areatrigger_tavern",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadAreaTriggerTavernCommand,         "", NULL },
        { "areatrigger_teleport",        PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadAreaTriggerTeleportCommand,       "", NULL },
        { "access_requirement",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadAccessRequirementCommand,         "", NULL },
        { "areatrigger_involvedrelation",PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadQuestAreaTriggersCommand,         "", NULL },
        { "auctions",                    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadAuctionsCommand,                  "", NULL },
        { "autobroadcast",               PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadAutobroadcastCommand,             "", NULL },
        { "command",                     PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadCommandCommand,                   "", NULL },
        { "creature_involvedrelation",   PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadCreatureQuestInvRelationsCommand, "", NULL },
        { "creature_linked_respawn",     PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadCreatureLinkedRespawnCommand,     "", NULL },
        { "creature_loot_template",      PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesCreatureCommand,     "", NULL },
        { "creature_questrelation",      PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadCreatureQuestRelationsCommand,    "", NULL },
        { "creature_ai_scripts",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadEventAIScriptsCommand,            "", NULL },
        { "disenchant_loot_template",    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesDisenchantCommand,   "", NULL },
        { "eventai",                     PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadEventAICommand,                   "", NULL },
        { "event_scripts",               PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadEventScriptsCommand,              "", NULL },
        { "fishing_loot_template",       PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesFishingCommand,      "", NULL },
        { "game_graveyard_zone",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadGameGraveyardZoneCommand,         "", NULL },
        { "game_tele",                   PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadGameTeleCommand,                  "", NULL },
        { "gameobject_involvedrelation", PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadGOQuestInvRelationsCommand,       "", NULL },
        { "gameobject_loot_template",    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesGameobjectCommand,   "", NULL },
        { "gameobject_questrelation",    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadGOQuestRelationsCommand,          "", NULL },
        { "gameobject_scripts",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadGameObjectScriptsCommand,         "", NULL },
        { "gm_tickets",                  PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadGMTicketCommand,                  "", NULL },
        { "item_enchantment_template",   PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadItemEnchantementsCommand,         "", NULL },
        { "item_loot_template",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesItemCommand,         "", NULL },
        { "locales_creature",            PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLocalesCreatureCommand,           "", NULL },
        { "locales_gameobject",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLocalesGameobjectCommand,         "", NULL },
        { "locales_item",                PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLocalesItemCommand,               "", NULL },
        { "locales_npc_text",            PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLocalesNpcTextCommand,            "", NULL },
        { "locales_page_text",           PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLocalesPageTextCommand,           "", NULL },
        { "locales_quest",               PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLocalesQuestCommand,              "", NULL },
        { "npc_gossip",                  PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadNpcGossipCommand,                 "", NULL },
        { "npc_option",                  PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadNpcOptionCommand,                 "", NULL },
        { "npc_trainer",                 PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadNpcTrainerCommand,                "", NULL },
        { "npc_vendor",                  PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadNpcVendorCommand,                 "", NULL },
        { "page_text",                   PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadPageTextsCommand,                 "", NULL },
        { "pickpocketing_loot_template", PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesPickpocketingCommand,"", NULL},
        { "prospecting_loot_template",   PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesProspectingCommand,  "", NULL },
        { "quest_mail_loot_template",    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesQuestMailCommand,    "", NULL },
        { "quest_end_scripts",           PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadQuestEndScriptsCommand,           "", NULL },
        { "quest_start_scripts",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadQuestStartScriptsCommand,         "", NULL },
        { "quest_template",              PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadQuestTemplateCommand,             "", NULL },
        { "reference_loot_template",     PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesReferenceCommand,    "", NULL },
        { "reserved_name",               PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadReservedNameCommand,              "", NULL },
        { "reputation_reward_rate",      PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadReputationRewardRateCommand,      "", NULL },
        { "reputation_spillover_template", PERM_ADM, PERM_CONSOLE, true, &ChatHandler::HandleReloadReputationSpilloverTemplateCommand, "", NULL },
        { "shop",                        PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadShopCommand,                       "", NULL },
        { "arena_restrictions",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadArenaRestrictionsCommand,         "", NULL },
        { "warden",                      PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadWardenCommand,                    "", NULL },
        { "propaganda",                  PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadMinorityPropagandaChar,           "", NULL },
        { "skill_discovery_template",    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSkillDiscoveryTemplateCommand,    "", NULL },
        { "skill_extra_item_template",   PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSkillExtraItemPrototypeCommand,    "", NULL },
        { "skill_fishing_base_level",    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSkillFishingBaseLevelCommand,     "", NULL },
        { "skinning_loot_template",      PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadLootTemplatesSkinningCommand,     "", NULL },
        { "spell_affect",                PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellAffectCommand,               "", NULL },
        { "spell_required",              PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellRequiredCommand,             "", NULL },
        { "spell_elixir",                PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellElixirCommand,               "", NULL },
        { "spell_learn_spell",           PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellLearnSpellCommand,           "", NULL },
        { "spell_linked_spell",          PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellLinkedSpellCommand,          "", NULL },
        { "spell_pet_auras",             PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellPetAurasCommand,             "", NULL },
        { "spell_proc_event",            PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellProcEventCommand,            "", NULL },
        { "spell_enchant_proc_data",     PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellEnchantDataCommand,          "", NULL },
        { "spell_script_target",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellScriptTargetCommand,         "", NULL },
        { "spell_scripts",               PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellScriptsCommand,              "", NULL },
        { "spell_target_position",       PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellTargetPositionCommand,       "", NULL },
        { "spell_threats",               PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellThreatsCommand,              "", NULL },
        { "spell_disabled",              PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadSpellDisabledCommand,             "", NULL },
        { "spell_analog",                PERM_ADM,    PERM_CONSOLE,         true,   &ChatHandler::HandleReloadSpellAnalogsCommand,    "", NULL },
        { "hellground_string",           PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadHellgroundStringCommand,          "", NULL },
        { "unqueue_account",             PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadUnqueuedAccountListCommand,       "", NULL },
        { "waypoint_scripts",            PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadWpScriptsCommand,                 "", NULL },
        { "revision",                    PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadRevisionCommand,                  "", NULL },
        { "bots",                        PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadFakeBots,                            "", NULL },
        { "",                            PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleReloadCommand,                          "", NULL },
        { NULL,                          0,         0,            false,  NULL,                                                       "", NULL }
    };

    static ChatCommand honorCommandTable[] =
    {
        { "add",            PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleHonorAddCommand,            "", NULL },
        { "addkill",        PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleHonorAddKillCommand,        "", NULL },
        { "update",         PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleHonorUpdateCommand,         "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand guildDisableCommandTable[] =
    {
        { "announce",       PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleGuildDisableAnnounceCommand,    "", NULL},
        { NULL,             0,              0,            false,  NULL,                                               "", NULL}
    };

    static ChatCommand guildEnableCommandTable[] =
    {
        { "announce",       PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleGuildEnableAnnounceCommand,     "", NULL},
        { NULL,             0,              0,            false,  NULL,                                               "", NULL}
    };

    static ChatCommand guildCommandTable[] =
    {
        //{ "advert",         PERM_GMT,    PERM_CONSOLE, false,  &ChatHandler::HandleGuildAdvertCommand,         "", NULL },
        { "ann",            PERM_GMT,    PERM_CONSOLE, false,  &ChatHandler::HandleGuildAnnounceCommand,       "", NULL },
        { "create",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleGuildCreateCommand,         "", NULL },
        { "delete",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleGuildDeleteCommand,         "", NULL },
        { "disable",        PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", guildDisableCommandTable },
        { "enable",         PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", guildEnableCommandTable },
        { "invite",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleGuildInviteCommand,         "", NULL },
        { "rank",           PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleGuildRankCommand,           "", NULL },
        { "uninvite",       PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleGuildUninviteCommand,       "", NULL },
        { "rename",         PERM_ADM,  PERM_CONSOLE, true,   &ChatHandler::HandleGuildRenameCommand,       "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand petCommandTable[] =
    {
        { "create",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCreatePetCommand,           "", NULL },
        { "learn",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandlePetLearnCommand,            "", NULL },
        { "tp",             PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandlePetTpCommand,               "", NULL },
        { "unlearn",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandlePetUnlearnCommand,          "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };


    static ChatCommand groupCommandTable[] =
    {
        { "disband",        PERM_ADM,       PERM_CONSOLE, false,   &ChatHandler::HandleGroupDisbandCommand,    "", NULL },
        { "leader",         PERM_ADM,       PERM_CONSOLE, false,   &ChatHandler::HandleGroupLeaderCommand,     "", NULL },
        { "remove",         PERM_ADM,       PERM_CONSOLE, false,   &ChatHandler::HandleGroupRemoveCommand,     "", NULL },
        { NULL,             0,              0,            false,   NULL,                                       "", NULL }
    };

    static ChatCommand lookupPlayerCommandTable[] =
    {
        { "account",        PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupPlayerAccountCommand, "", NULL },
        { "email",          PERM_HIGH_GMT,  PERM_CONSOLE, true,   &ChatHandler::HandleLookupPlayerEmailCommand,   "", NULL },
        { "ip",             PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupPlayerIpCommand,      "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand lookupCommandTable[] =
    {
        { "area",           PERM_HIGH_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupAreaCommand,          "", NULL },
        { "creature",       PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupCreatureCommand,      "", NULL },
        { "event",          PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupEventCommand,         "", NULL },
        { "faction",        PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupFactionCommand,       "", NULL },
        { "item",           PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupItemCommand,          "", NULL },
        { "itemset",        PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupItemSetCommand,       "", NULL },
        { "object",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupObjectCommand,        "", NULL },
        { "player",         PERM_GMT,        PERM_CONSOLE, true,   NULL,                                           "", lookupPlayerCommandTable },
        { "quest",          PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupQuestCommand,         "", NULL },
        { "skill",          PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupSkillCommand,         "", NULL },
        { "spell",          PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupSpellCommand,         "", NULL },
        { "tele",           PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleLookupTeleCommand,          "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand resetCommandTable[] =
    {
        { "all",            PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleResetAllCommand,            "", NULL },
        { "honor",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleResetHonorCommand,          "", NULL },
        { "level",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleResetLevelCommand,          "", NULL },
        { "spells",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleResetSpellsCommand,         "", NULL },
        { "stats",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleResetStatsCommand,          "", NULL },
        { "talents",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleResetTalentsCommand,        "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand castCommandTable[] =
    {
        { "back",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCastBackCommand,            "", NULL },
        { "dist",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCastDistCommand,            "", NULL },
        { "null",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCastNullCommand,            "", NULL },
        { "self",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCastSelfCommand,            "", NULL },
        { "target",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCastTargetCommand,          "", NULL },
        { "",               PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCastCommand,                "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand listCommandTable[] =
    {
        { "auras",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleListAurasCommand,           "", NULL },
        { "creature",       PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleListCreatureCommand,        "", NULL },
        { "item",           PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleListItemCommand,            "", NULL },
        { "object",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleListObjectCommand,          "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand teleCommandTable[] =
    {
        { "add",            PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleTeleAddCommand,             "", NULL },
        { "del",            PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleTeleDelCommand,             "", NULL },
        { "group",          PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleTeleGroupCommand,           "", NULL },
        { "name",           PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleTeleNameCommand,            "", NULL },
        { "",               PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleTeleCommand,                "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand npcCommandTable[] =
    {
        { "add",            PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleNpcAddCommand,              "", NULL },
        { "additem",        PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleNpcAddItemCommand,          "", NULL },
        { "addmove",        PERM_ADM, PERM_CONSOLE, false,  &ChatHandler::HandleNpcAddMoveCommand,          "", NULL },
        { "addtemp",        PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleNpcAddTempCommand,          "", NULL },
        { "changeentry",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleNpcChangeEntryCommand,      "", NULL },
        { "changelevel",    PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleNpcChangeLevelCommand,      "", NULL },
        { "debugai",        PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcDebugAICommand,          "", NULL },
        { "delete",         PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleNpcDeleteCommand,           "", NULL },
        { "delitem",        PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcDelItemCommand,          "", NULL },
        { "doaction",       PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcDoActionCommand,         "", NULL },
        { "enterevademode", PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcEnterEvadeModeCommand,   "", NULL },
        { "extraflag",      PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcExtraFlagCommand,        "", NULL },
        { "factionid",      PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcFactionIdCommand,        "", NULL },
        { "fieldflag",      PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcFieldFlagCommand,        "", NULL },
        { "flag",           PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcFlagCommand,             "", NULL },
        { "follow",         PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcFollowCommand,           "", NULL },
        { "info",           PERM_ADM,    PERM_CONSOLE, false,  &ChatHandler::HandleNpcInfoCommand,             "", NULL },
        { "move",           PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcMoveCommand,             "", NULL },
        { "playemote",      PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcPlayEmoteCommand,        "", NULL },
        { "resetai",        PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcResetAICommand,          "", NULL },
        { "say",            PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleNpcSayCommand,              "", NULL },
        { "setlink",        PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcSetLinkCommand,          "", NULL },
        { "setmodel",       PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcSetModelCommand,         "", NULL },
        { "setmovetype",    PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcSetMoveTypeCommand,      "", NULL },
        { "spawndist",      PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcSpawnDistCommand,        "", NULL },
        { "spawntime",      PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcSpawnTimeCommand,        "", NULL },
        { "standstate",     PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcStandState,              "", NULL },
        { "textemote",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleNpcTextEmoteCommand,        "", NULL },
        { "unfollow",       PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleNpcUnFollowCommand,         "", NULL },
        { "whisper",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleNpcWhisperCommand,          "", NULL },
        { "yell",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleNpcYellCommand,             "", NULL },
        //{ TODO: fix or remove this commands
        { "name",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleNameCommand,                "", NULL },
        { "subname",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleSubNameCommand,             "", NULL },
        { "addweapon",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleAddWeaponCommand,           "", NULL },
        { "setmodifier",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleNpcSetModifierCommand,      "", NULL },
        //}
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand goCommandTable[] =
    {
        { "creature",       PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoCreatureCommand,          "", NULL },
        { "direct",         PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoCreatureDirectCommand,    "", NULL },
        { "crinmap",        PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoCreatureInMapCommand,     "", NULL },
        { "graveyard",      PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoGraveyardCommand,         "", NULL },
        { "grid",           PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoGridCommand,              "", NULL },
        { "object",         PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoObjectCommand,            "", NULL },
        { "ticket",         PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGoTicketCommand,            "", NULL },
        { "trigger",        PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoTriggerCommand,           "", NULL },
        { "zonexy",         PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoZoneXYCommand,            "", NULL },
        { "xy",             PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoXYCommand,                "", NULL },
        { "xyz",            PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoXYZCommand,               "", NULL },
        { "",               PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGoXYZCommand,               "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand gobjectCommandTable[] =
    {
        { "activate",       PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectActivateCommand,  "", NULL },
        { "add",            PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectAddCommand,       "", NULL },
        { "addtemp",        PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectAddTempCommand,   "", NULL },
        { "delete",         PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectDeleteCommand,    "", NULL },
        { "grid",           PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectGridCommand,      "", NULL },
        { "move",           PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectMoveCommand,      "", NULL },
        { "near",           PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectNearCommand,      "", NULL },
        { "reset",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectResetCommand,      "", NULL },
        { "target",         PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectTargetCommand,    "", NULL },
        { "turn",           PERM_ADM,   PERM_CONSOLE, false,  &ChatHandler::HandleGameObjectTurnCommand,      "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand questCommandTable[] =
    {
        { "add",            PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleQuestAdd,                   "", NULL },
        { "complete",       PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleQuestComplete,              "", NULL },
        { "remove",         PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleQuestRemove,                "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand gmCommandTable[] =
    {
        { "announce",       PERM_HIGH_GMT,  PERM_CONSOLE, true,   &ChatHandler::HandleGMAnnounceCommand,          "", NULL },
        { "chat",           PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMChatCommand,              "", NULL },
        { "fly",            PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleGMFlyCommand,               "", NULL },
        { "list",           PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleGMListFullCommand,          "", NULL },
        { "nameannounce",   PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMNameAnnounceCommand,      "", NULL },
        { "notify",         PERM_HIGH_GMT,  PERM_CONSOLE, true,   &ChatHandler::HandleGMNotifyCommand,            "", NULL },
        { "visible",        PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMVisibleCommand,           "", NULL },
        { "ingame",         PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMIngameCommand,            "", NULL },
        { "free",           PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMFreeCommand,              "", NULL },
        { "",               PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMCommand,                  "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand instanceCommandTable[] =
    {
        { "listbinds",       PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleInstanceListBindsCommand,        "", NULL },
        { "savedata",        PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleInstanceSaveDataCommand,         "", NULL },
        { "selfunbind",      PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleInstanceSelfUnbindCommand,       "", NULL },
        { "stats",           PERM_GMT_DEV,   PERM_CONSOLE, true,   &ChatHandler::HandleInstanceStatsCommand,            "", NULL },
        { "unbind",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleInstanceUnbindCommand,           "", NULL },
        { "bind",            PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleInstanceBindCommand,             "", NULL },
        { "resetencounters", PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleInstanceResetEncountersCommand,  "", NULL },
        { "insideplayers",   PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleInstanceInsidePlayersCommand,    "", NULL },
        { "getdata",	     PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleInstanceGetData,				    "", NULL },
        { "setdata",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleInstanceSetData,				    "", NULL },
        { NULL,              0,              0,            false,  NULL,                                                "", NULL }
    };

    /*static ChatCommand ticketCommandTable[] =
    {
        { "assign",         PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketAssignToCommand,    "", NULL },
        { "selfassign",     PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketSelfAssignCommand,  "", NULL },
        { "deny",           PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketDenyCommand,        "", NULL },
        { "close",          PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketCloseByIdCommand,   "", NULL },
        { "closedlist",     PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketListClosedCommand,  "", NULL },
        { "comment",        PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketCommentCommand,     "", NULL },
        { "delete",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketDeleteByIdCommand,  "", NULL },
        { "history",        PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketHistoryCommand,     "", NULL },
        { "list",           PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketListCommand,        "", NULL },
        { "onlinelist",     PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketListOnlineCommand,  "", NULL },
        { "response",       PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketResponseCommand,    "", NULL },
        { "unassign",       PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketUnAssignCommand,    "", NULL },
        { "viewid",         PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketGetByIdCommand,     "", NULL },
        { "viewname",       PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGMTicketGetByNameCommand,   "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };*/

    static ChatCommand channelCommandTable[] =
    {
        { "kick",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleChannelKickCommand,         "", NULL },
        { "list",           PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleChannelListCommand,         "", NULL },
        { "masskick",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleChannelMassKickCommand,     "", NULL },
        { "pass",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleChannelPassCommand,         "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand crashCommandTable[] =
    {
        { "map",            PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleCrashMapCommand,            "", NULL },
        { "server",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleCrashServerCommand,         "", NULL },
        { "freeze",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleCrashFreezeCommand,         "", NULL },
        { "memleak",        PERM_CONSOLE,   PERM_CONSOLE, true,   &ChatHandler::HandleCrashMemleakCommand,        "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    static ChatCommand cacheCommandTable[] =
    {
        { "item",           PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleCacheItemCommand,           "", NULL },
        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    //static ChatCommand lootCommandTable[] =
    //{
    //    { "iteminfo",        PERM_ADM,      PERM_CONSOLE, false,  &ChatHandler::HandleLootItemInfoCommand,        "", NULL },
    //    { "playerinfo",      PERM_ADM,      PERM_CONSOLE, false,  &ChatHandler::HandleLootPlayerInfoCommand,      "", NULL },
    //    { "delete",          PERM_ADM,      PERM_CONSOLE, false,  &ChatHandler::HandleLootDeleteCommand,          "", NULL },
    //    { "move",            PERM_ADM,      PERM_CONSOLE, false,  &ChatHandler::HandleLootMoveCommand,            "", NULL },
    //    { NULL, 0, 0, false, NULL, "", NULL }
    //};

    static ChatCommand wardenCommandTable[] =
    {
        { "reload",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleReloadWardenCommand,       "", NULL },
        { "force",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleWardenForceCheckCommand,   "", NULL },
        { NULL, 0, 0, false, NULL, "", NULL }
    };

    static ChatCommand commandTable[] =
    {
        { "account",        PERM_PLAYER,    PERM_CONSOLE, true,   NULL,                                           "", accountCommandTable },
        { "ban",            PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", banCommandTable },
        { "baninfo",        PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", baninfoCommandTable },
        { "banlist",        PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", banlistCommandTable },
        { "cast",           PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", castCommandTable },
        { "channel",        PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", channelCommandTable},
        { "crash",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  NULL,                                           "", crashCommandTable },
        { "cache",          PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", cacheCommandTable },
        //{ "loot",           PERM_ADM,       PERM_CONSOLE, false,  NULL,                                           "", lootCommandTable },
        { "debug",          PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", debugCommandTable },
        { "event",          PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", eventCommandTable },
        { "gm",             PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", gmCommandTable },
        { "go",             PERM_GMT_DEV,   PERM_CONSOLE, false,  NULL,                                           "", goCommandTable },
        { "gobject",        PERM_GMT_DEV,   PERM_CONSOLE, false,  NULL,                                           "", gobjectCommandTable },
        { "guild",          PERM_HIGH_GMT,  PERM_CONSOLE, true,   NULL,                                           "", guildCommandTable },
        { "honor",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  NULL,                                           "", honorCommandTable },
        { "instance",       PERM_GMT_DEV,   PERM_CONSOLE, true,   NULL,                                           "", instanceCommandTable },
        { "learn",          PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", learnCommandTable },
        { "list",           PERM_ADM,       PERM_CONSOLE, true,   NULL,                                           "", listCommandTable },
        { "lookup",         PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", lookupCommandTable },
        { "modify",         PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", modifyCommandTable },
        { "mmap",           PERM_GMT_DEV,   PERM_CONSOLE, false,  NULL,                                           "", mmapCommandTable },
        { "npc",            PERM_GMT_DEV,   PERM_CONSOLE, false,  NULL,                                           "", npcCommandTable },
        { "path",           PERM_GMT_DEV,   PERM_CONSOLE, false,  NULL,                                           "", wpCommandTable },
        { "pet",            PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", petCommandTable },
        { "quest",          PERM_HIGH_GMT,  PERM_CONSOLE, false,  NULL,                                           "", questCommandTable },
        { "reload",         PERM_ADM,       PERM_CONSOLE, true,   NULL,                                           "", reloadCommandTable },
        { "reset",          PERM_ADM,       PERM_CONSOLE, false,  NULL,                                           "", resetCommandTable },
        { "send",           PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", sendCommandTable },
        { "server",         PERM_ADM,       PERM_CONSOLE, true,   NULL,                                           "", serverCommandTable },
        { "tele",           PERM_GMT_DEV,   PERM_CONSOLE, true,   NULL,                                           "", teleCommandTable },
        //{ "ticket",         PERM_GMT,       PERM_CONSOLE, false,  NULL,                                           "", ticketCommandTable },
        { "unban",          PERM_GMT,       PERM_CONSOLE, true,   NULL,                                           "", unbanCommandTable },
        { "warden",         PERM_ADM,       PERM_CONSOLE, false,  NULL,                                           "", wardenCommandTable },
        { "wp",             PERM_GMT_DEV,   PERM_CONSOLE, false,  NULL,                                           "", wpCommandTable },
        { "additem",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleAddItemCommand,             "", NULL },
        { "additemset",     PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleAddItemSetCommand,          "", NULL },
        { "allowmove",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleAllowMovementCommand,       "", NULL },
        { "announce",       PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleAnnounceCommand,            "", NULL },
        { "annid",          PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleAnnIdCommand,               "", NULL },
        { "aura",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleAuraCommand,                "", NULL },
        { "bank",           PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleBankCommand,                "", NULL },
        { "bindfollow",     PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleBindFollowCommand,          "", NULL },
        { "bindsight",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleBindSightCommand,           "", NULL },
        { "cd",             PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCooldownCommand,            "", NULL },
        { "chardelete",     PERM_CONSOLE,   PERM_CONSOLE, true,   &ChatHandler::HandleCharacterDeleteCommand,     "", NULL },
        { "combatstop",     PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleCombatStopCommand,          "", NULL },
        { "cometome",       PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleComeToMeCommand,            "", NULL },
        { "commands",       PERM_PLAYER,    PERM_CONSOLE, true,   &ChatHandler::HandleCommandsCommand,            "", NULL },
        { "cooldown",       PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleCooldownCommand,            "", NULL },
        { "damage",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleDamageCommand,              "", NULL },
        { "demorph",        PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleDeMorphCommand,             "", NULL },

        { "truedemorph",    PERM_ADM,           PERM_CONSOLE,   false,  &ChatHandler::HandleTrueDeMorphCommand,         "", NULL },
        { "spellaffect",    PERM_ADM,           PERM_CONSOLE,   false,  &ChatHandler::HandleSpellAffectCheckCommand,    "", NULL },
        { "spellknown",     PERM_ADM,           PERM_CONSOLE,   false,  &ChatHandler::HandleSpellKnownCheckCommand,     "", NULL },
        { "spellinfo",      PERM_ADM,           PERM_CONSOLE,   false,  &ChatHandler::HandleSpellInfoCheckCommand,      "", NULL },
        //{ "itemlevel",      PERM_ADM,           PERM_CONSOLE,   false,  &ChatHandler::HandleItemLevelCheckCommand,      "", NULL },
        //{ "charinfo",       PERM_PLAYER,        PERM_CONSOLE,   true,   &ChatHandler::HandleCharInfoCommand,            "", NULL },
        //{ "gp",             PERM_PLAYER,        PERM_CONSOLE,   true,   &ChatHandler::HandleGuildPointsInfoCommand,     "", NULL },
        { "coins",          PERM_PLAYER,        PERM_CONSOLE,   true,   &ChatHandler::HandleCoinsInfoCommand,           "", NULL },
        { "lostest",        PERM_ADM,           PERM_CONSOLE,   true,   &ChatHandler::HandleLosTestCommand,             "", NULL },
        { "finditem",       PERM_ADM,           PERM_CONSOLE,   true,   &ChatHandler::HandleLookGMItemCommand,          "", NULL },
        { "delitem",        PERM_ADM,           PERM_CONSOLE,   true,   &ChatHandler::HandleDeleteGMItemCommand,        "", NULL },

        { "die",            PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleDieCommand,                 "", NULL },
        { "dismount",       PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleDismountCommand,            "", NULL },
        { "distance",       PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGetDistanceCommand,         "", NULL },
        { "explorecheat",   PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleExploreCheatCommand,        "", NULL },
        { "flusharenapoints",PERM_ADM,      PERM_CONSOLE, false,  &ChatHandler::HandleFlushArenaPointsCommand,    "", NULL },
        { "freeze",         PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleFreezeCommand,              "", NULL },
        { "goname",         PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGonameCommand,              "", NULL },
        { "gps",            PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGPSCommand,                 "", NULL },
        { "posinfo",           PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandlePosInfoCommand,                "", NULL },
        { "groupgo",        PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleGroupgoCommand,             "", NULL },
        { "guid",           PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleGUIDCommand,                "", NULL },
        { "hdevannounce",   PERM_HEAD_DEVELOPER,PERM_CONSOLE, false,&ChatHandler::HandleHDevAnnounceCommand,      "", NULL },
        { "help",           PERM_PLAYER,    PERM_CONSOLE, true,   &ChatHandler::HandleHelpCommand,                "", NULL },
        //{ "hidearea",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleHideAreaCommand,            "", NULL },
        { "hover",          PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleHoverCommand,               "", NULL },
        { "itemmove",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleItemMoveCommand,            "", NULL },
        { "kick",           PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleKickPlayerCommand,          "", NULL },
        { "levelup",        PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleLevelUpCommand,             "", NULL },
        { "linkgrave",      PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleLinkGraveCommand,           "", NULL },
        { "listfreeze",     PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleListFreezeCommand,          "", NULL },
        { "loadscripts",    PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleLoadScriptsCommand,         "", NULL },
        //{ "lockaccount",    PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleLockAccountCommand,         "", NULL },
        { "maxskill",       PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleMaxSkillCommand,            "", NULL },
        { "movegens",       PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleMovegensCommand,            "", NULL },
        { "mute",           PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleMuteCommand,                "", NULL },
        { "muteinfo",       PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleMuteInfoCommand,            "", NULL },
        { "nameannounce",   PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleNameAnnounceCommand,        "", NULL },
        { "namego",         PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleNamegoCommand,              "", NULL },
        { "neargrave",      PERM_GMT_DEV,   PERM_CONSOLE, false,  &ChatHandler::HandleNearGraveCommand,           "", NULL },
        { "notify",         PERM_HIGH_GMT,  PERM_CONSOLE, true,   &ChatHandler::HandleNotifyCommand,              "", NULL },
        //{ "password",       PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandlePasswordCommand,            "", NULL },
        { "pinfo",          PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandlePInfoCommand,               "", NULL },
        { "playall",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandlePlayAllCommand,             "", NULL },

        { "playself",       PERM_ADM,  PERM_CONSOLE, false,  &ChatHandler::HandlePlaySelfCommand,            "", NULL },

        { "plimit",         PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandlePLimitCommand,              "", NULL },
        { "possess",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandlePossessCommand,             "", NULL },
        //{ "raidrules",      PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleRaidRulesCommand,           "", NULL },
        { "recall",         PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleRecallCommand,              "", NULL },
        { "rename",         PERM_HIGH_GMT,  PERM_CONSOLE, true,   &ChatHandler::HandleRenameCommand,              "", NULL },

        { "nameban",        PERM_GMT,     PERM_CONSOLE, true,   &ChatHandler::HandleNameBanCommand,              "", NULL },

        { "repairitems",    PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleRepairitemsCommand,         "", NULL },
        { "respawn",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleRespawnCommand,             "", NULL },
        { "revive",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleReviveCommand,              "", NULL },
        { "revivegroup",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleReviveGroupCommand,         "", NULL },
        { "save",           PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleSaveCommand,                "", NULL },
        { "setskill",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleSetSkillCommand,            "", NULL },
        { "showarea",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleShowAreaCommand,            "", NULL },
        { "start",          PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleStartCommand,               "", NULL },
        { "taxicheat",      PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleTaxiCheatCommand,           "", NULL },
        { "trollmute",      PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleTrollmuteCommand,           "", NULL },
        { "trollmuteinfo",  PERM_HIGH_GMT,  PERM_CONSOLE, false,  &ChatHandler::HandleTrollmuteInfoCommand,       "", NULL },
        { "unaura",         PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleUnAuraCommand,              "", NULL },
        { "unbindfollow",   PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleUnbindFollowCommand,        "", NULL },
        { "unbindsight",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleUnbindSightCommand,         "", NULL },
        { "unfreeze",       PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleUnFreezeCommand,            "", NULL },
        { "unlearn",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleUnLearnCommand,             "", NULL },
        { "unmute",         PERM_GMT,       PERM_CONSOLE, true,   &ChatHandler::HandleUnmuteCommand,              "", NULL },
        { "unpossess",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleUnPossessCommand,           "", NULL },
        { "waterwalk",      PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleWaterwalkCommand,           "", NULL },
        { "wchange",        PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleChangeWeather,              "", NULL },
        { "weather",        PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleAccountWeatherCommand,      "", NULL },
        { "whispers",       PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleWhispersCommand,            "", NULL },
        //{ "bgkick",         PERM_GMT, PERM_CONSOLE, false, &ChatHandler::HandleBGKick, "", NULL },

        { "diff",           PERM_ADM,       PERM_CONSOLE, true,   &ChatHandler::HandleDiffCommand,                "", NULL },
        { "sendspellvisual",PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleSendSpellVisualCommand,     "", NULL },
        { "deletedchars",   PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleDeletedCharactersCommand,   "", NULL },
        { "charrestore",    PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleCharacterRestoreCommand,    "", NULL },
        { "fakepacket",     PERM_ADM,       PERM_CONSOLE, false,  &ChatHandler::HandleFakePacketCommand,          "", NULL },
        //{ "teaminfo",       PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleTeamInfoCommand,            "", NULL },

        { "gift",           PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleGiftCommand,              "", NULL },
        { "bonus",          PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleBonusCommand,               "", NULL },
        { "captcha",          PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleCaptcha,               "", NULL },
        { "getchest",            PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleBoxCommand,                 "", NULL },
        { "raidtest",       PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleRaidtest,                   "", NULL },
        //{ "raidchest", PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleRaidChest, "", NULL },
        { "guildmarker",          PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleGuildMarker,                      "", NULL },
        
		{ "golem",          PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleGolem,                      "", NULL },

		{ "renamepet",          PERM_ADM,    PERM_CONSOLE, false,  &ChatHandler::HandleRenamePet,                      "", NULL },

        //{ "beta",           PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleBetaParticipationGiftCommand,"", NULL },
        //{ "pvp_bonus",      PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandlePvpBonusCommand,            "", NULL },

        { "shop",           PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleShopCommand,                "", NULL },
        { "premium",        PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandlePremiumCommand,             "", NULL },
        { "xp",             PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleXpRatesCommand,             "", NULL },
        { "language",       PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleLanguageCommand,            "", NULL },
        //{ "altip",          PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleAccountAltRealmCommand,     "", NULL },
        { "bg",             PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleBgRegSummonCommand,         "", NULL },
        //{ "bgstat",         PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleBgWinsCommand,              "", NULL },
        { "arenabgann",     PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleAccountBattleGroundAnnCommand, "", NULL },
        { "lootann",        PERM_PLAYER,    PERM_CONSOLE, false,  &ChatHandler::HandleLootAnnounces, "", NULL },

        { "botadd",          PERM_CONSOLE,   PERM_CONSOLE, true,   &ChatHandler::HandleFakeBotAdd,          "", NULL },

        //{ "specialization", PERM_GMT,       PERM_CONSOLE, false,  &ChatHandler::HandleSpecializationCommand,      "", NULL },
        { "deltransmog", PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleDeleteTransmog, "", NULL },
        //{ "namereserve", PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleNicknameReservation, "", NULL },
        //{ "betafree", PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleBetaFree, "", NULL },

        { "code", PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleActivateCode, "", NULL },
        { "throwcode", PERM_ADM, PERM_CONSOLE, true, &ChatHandler::HandleThrowCode, "", NULL },

        { "test", PERM_ADM, PERM_CONSOLE, true, &ChatHandler::HandleTest, "", NULL },

        { "hide", PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleHide, "", NULL },

        { "removebanhistory", PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleRemoveBanHistoryCommand, "", NULL },
        { "bgevent", PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleBgEventCommand, "", NULL },
        { "mw", PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleMW, "", NULL },
        { "info", PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleInfo, "", NULL },

		{ "markbot",        PERM_ADM,       PERM_CONSOLE, true,  &ChatHandler::HandleMarkBot,           "", NULL },
		{ "thisbg",			    PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleBG, "", NULL },
		{ "setmapmod",			    PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleSetMapMod, "", NULL },
		{ "getmapmod",			    PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleGetMapMod, "", NULL },

		{ "raidstats",			    PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleRaidStats, "", NULL },
		{ "unitinfo",			    PERM_ADM, PERM_CONSOLE, false, &ChatHandler::HandleUnitInfoCommand, "", NULL },
        //{ "berserk", PERM_GMT, PERM_CONSOLE, false, &ChatHandler::HandleBerserk, "", NULL },

        { "enchant_legweapon", PERM_PLAYER, PERM_CONSOLE, false, &ChatHandler::HandleEnchantLegweapon, "", NULL },

        { NULL,             0,              0,            false,  NULL,                                           "", NULL }
    };

    if (load_command_table)
    {
        load_command_table = false;

        QueryResultAutoPtr result = GameDataDatabase.Query("SELECT name, permission_mask, self_mask, help FROM command");
        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                std::string name = fields[0].GetCppString();
                for (uint32 i = 0; commandTable[i].Name != NULL; i++)
                {
                    if (name == commandTable[i].Name)
                    {
                        commandTable[i].RequiredPermissions = fields[1].GetUInt64();
                        commandTable[i].SelfPermissions = fields[2].GetUInt64();
                        commandTable[i].Help = fields[3].GetCppString();
                    }
                    if (commandTable[i].ChildCommands != NULL)
                    {
                        ChatCommand *ptable = commandTable[i].ChildCommands;
                        for (uint32 j = 0; ptable[j].Name != NULL; j++)
                        {
                            // first case for "" named subcommand
                            if (ptable[j].Name[0]=='\0' && name == commandTable[i].Name ||
                                name == fmtstring("%s %s", commandTable[i].Name, ptable[j].Name))
                            {
                                ptable[j].RequiredPermissions = fields[1].GetUInt64();
                                ptable[j].SelfPermissions = fields[2].GetUInt64();
                                ptable[j].Help = fields[3].GetCppString();
                            }
                        }
                    }
                }
            }
            while (result->NextRow());
        }
    }

    return commandTable;
}

const char *ChatHandler::GetHellgroundString(int32 entry) const
{
    return m_session->GetHellgroundString(entry);
}

bool ChatHandler::isAvailable(ChatCommand const& cmd, bool self) const
{
    // check security level only for simple  command (without child commands)
    return m_session->HasPermissions(self ? cmd.SelfPermissions : cmd.RequiredPermissions);
}

bool ChatHandler::hasStringAbbr(const char* name, const char* part)
{
    // non "" command
    if (*name)
    {
        // "" part from non-"" command
        if (!*part)
            return false;

        for (;;)
        {
            if (!*part)
                return true;
            else if (!*name)
                return false;
            else if (tolower(*name) != tolower(*part))
                return false;
            ++name; ++part;
        }
    }
    // allow with any for ""

    return true;
}

void ChatHandler::SendSysMessage(const char *str)
{
    if (!m_session)
    {
        puts(str);
        return;
    }

    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        FillSystemMessageData(&data, line);
        m_session->SendPacket(&data);
    }

    delete [] buf;
}

void ChatHandler::SendGlobalSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        FillSystemMessageData(&data, line);
        sWorld.SendGlobalMessage(&data);
    }

    delete [] buf;
}

void ChatHandler::SendGlobalGMSysMessage(int32 entry, ...)
{
    const char *format = GetHellgroundString(entry);
    va_list ap;
    char str [1024];
    va_start(ap, entry);
    vsnprintf(str,1024,format, ap);
    va_end(ap);
    SendGlobalGMSysMessage(str);
}

void ChatHandler::SendGlobalGMSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        FillSystemMessageData(&data, line);
        sWorld.SendGlobalGMMessage(&data);
     }
    delete [] buf;
}

void ChatHandler::SendSysMessage(int32 entry)
{
    SendSysMessage(GetHellgroundString(entry));
}

void ChatHandler::SendNotification(int32 entry)
{
    std::string str = GetHellgroundString(entry);
    WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
    data << str;
    m_session->SendPacket(&data);
}

void ChatHandler::PSendSysMessage(int32 entry, ...)
{
    const char *format = GetHellgroundString(entry);
    va_list ap;
    char str [1024];
    va_start(ap, entry);
    vsnprintf(str,1024,format, ap);
    va_end(ap);
    SendSysMessage(str);
}

void ChatHandler::PSendSysMessage(const char *format, ...)
{
    va_list ap;
    char str [1024];
    va_start(ap, format);
    vsnprintf(str,1024,format, ap);
    va_end(ap);
    SendSysMessage(str);
}

bool ChatHandler::ExecuteCommandInTable(ChatCommand *table, const char* text, std::string& fullcmd)
{
    char const* oldtext = text;
    std::string cmd = "";

    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    for (uint32 i = 0; table[i].Name != NULL; i++)
    {
        if (!hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;
        fullcmd += table[i].Name;
        // select subcommand from child commands list
        if (table[i].ChildCommands != NULL)
        {
            fullcmd += " ";
            if (!ExecuteCommandInTable(table[i].ChildCommands, text, fullcmd))
            {
                if (text && text[0] != '\0')
                    SendSysMessage(LANG_NO_SUBCMD);
                else
                    SendSysMessage(LANG_CMD_SYNTAX);

                ShowHelpForCommand(table[i].ChildCommands,text);
            }

            return true;
        }

        Player* plr = m_session ? m_session->GetPlayer() : NULL;
        // must be available and have handler
        if (!isAvailable(table[i],false))
        {
            if (plr && isAvailable(table[i], true))
                plr->SetSelection(plr->GetGUID());
            else
                continue;
        }
        if (!table[i].Handler)
            continue;

        SetSentErrorMessage(false);
        // table[i].Name == "" is special case: send original command to handler
        std::string args = strlen(table[i].Name )!=0 ? std::string(" ") + text : oldtext;
        if ((this->*(table[i].Handler))(strlen(table[i].Name)!=0 ? text : oldtext))
        {
        if (m_session && (m_session->GetPermissions() & sWorld.getConfig(CONFIG_COMMAND_LOG_PERMISSION)) && table[i].Name != "password")
            {
                Player* p = m_session->GetPlayer();
                uint64 sel_guid = p->GetSelection();
                Unit* unit = p->GetUnit(sel_guid);
                char sel_string[100];
                if(sel_guid && unit)
                    sprintf(sel_string,"%s (GUID:%u)",unit->GetName(), GUID_LOPART(sel_guid));
                else if (sel_guid)
                    sprintf(sel_string,"(GUID:%u)", GUID_LOPART(sel_guid));
                else
                    sprintf(sel_string,"NONE");

                sLog.outCommand(m_session->GetAccountId(),"Command: %s%s [Player: %s (Account: %u) X: %f Y: %f Z: %f Map: %u Selected: %s]",
                    fullcmd.c_str(),args .c_str(),
                    p->GetName(),m_session->GetAccountId(),p->GetPositionX(),p->GetPositionY(),p->GetPositionZ(),p->GetMapId(),
                    sel_string);
            }
        }
        // some commands have custom error messages. Don't send the default one in these cases.
        else if (!sentErrorMessage)
        {
            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());
            else
                SendSysMessage(LANG_CMD_SYNTAX);
        }

        // restore target if it was changed
        if (plr)
            plr->SetSelection(plr->GetSavedSelection());
        return true;
    }

    return false;
}

bool ChatHandler::ContainsNotAllowedSigns(std::string text,bool strict)
{
    for (uint32 i = 0; i < text.length(); ++i)
        text[i] = tolower(text[i]);

    if ((text.find(".blp") != text.npos) || (text.find("t|t") != text.npos))
        return true;

    if (!strict)
        return false;

    /*for (std::string::iterator itr = text.begin(); itr != text.end(); itr++)
    {
        if (char(*itr) < 0x20)
            return true;
    }*/ // Trentone - this is a restriction to ASCII. Might help restricting characters to english/russian later.
    return false;
}

int ChatHandler::ParseCommands(const char* text)
{
    ASSERT(text);
    ASSERT(*text);

    std::string fullcmd = "";
    /// chat case (.command format)
    if (m_session)
    {
        if (text[0] != '.')
            return 0;
    }

    /// ignore single . and ! in line
    if (strlen(text) < 2)
        return 0;
    // original `text` can't be used. It content destroyed in command code processing.

    /// ignore messages staring from many dots.
    if (text[0] == '.' && text[1] == '.')
        return 0;

    /// skip first . (in console allowed use command with . and without its)
    if (text[0] == '.')
        ++text;

    if (!ExecuteCommandInTable(getCommandTable(), text, fullcmd))
    {
        if (m_session && !m_session->HasPermissions(PERM_GMT))
            return 0;
        SendSysMessage(LANG_NO_CMD);
    }
    return 1;
}

bool ChatHandler::ShowHelpForSubCommands(ChatCommand *table, char const* cmd, char const* subcmd)
{
    std::string list;
    for (uint32 i = 0; table[i].Name != NULL; ++i)
    {
        // must be available (ignore handler existence for show command with possibe avalable subcomands
        if (!isAvailable(table[i],false) && !isAvailable(table[i],true))
            continue;

        /// for empty subcmd show all available
        if (*subcmd && !hasStringAbbr(table[i].Name, subcmd))
            continue;

        if (m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += table[i].Name;

        if (table[i].ChildCommands)
            list += " ...";
    }

    if (list.empty())
        return false;

    if (table==getCommandTable())
    {
        SendSysMessage(LANG_AVIABLE_CMD);
        PSendSysMessage("%s",list.c_str());
    }
    else
        PSendSysMessage(LANG_SUBCMDS_LIST,cmd,list.c_str());

    return true;
}

bool ChatHandler::ShowHelpForCommand(ChatCommand *table, const char* cmd)
{
    if (*cmd)
    {
        for (uint32 i = 0; table[i].Name != NULL; ++i)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if (!isAvailable(table[i],false) && !isAvailable(table[i],true))
                continue;

            if (!hasStringAbbr(table[i].Name, cmd))
                continue;

            // have subcommand
            char const* subcmd = (*cmd) ? strtok(NULL, " ") : "";

            if (table[i].ChildCommands && subcmd && *subcmd)
            {
                if (ShowHelpForCommand(table[i].ChildCommands, subcmd))
                    return true;
            }

            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if (table[i].ChildCommands)
                if (ShowHelpForSubCommands(table[i].ChildCommands,table[i].Name,subcmd ? subcmd : ""))
                    return true;

            return !table[i].Help.empty();
        }
    }
    else
    {
        for (uint32 i = 0; table[i].Name != NULL; ++i)
        {
            // must be available (ignore handler existence for show command with possibe avalable subcomands
            if (!isAvailable(table[i],false) && !isAvailable(table[i],true))
                continue;

            if (strlen(table[i].Name))
                continue;

            if (!table[i].Help.empty())
                SendSysMessage(table[i].Help.c_str());

            if (table[i].ChildCommands)
                if (ShowHelpForSubCommands(table[i].ChildCommands,"",""))
                    return true;

            return !table[i].Help.empty();
        }
    }

    return ShowHelpForSubCommands(table,"",cmd);
}

//Note: target_guid used only in CHAT_MSG_WHISPER_INFORM mode (in this case channelName ignored)
void ChatHandler::FillMessageData(WorldPacket *data, WorldSession* session, uint8 type, uint32 language, const char *channelName, uint64 target_guid, const char *message, Unit *speaker)
{
    uint32 messageLength = (message ? strlen(message) : 0) + 1;

    data->Initialize(SMSG_MESSAGECHAT, 100);                // guess size
    *data << uint8(type);
    if ((type != CHAT_MSG_CHANNEL && type != CHAT_MSG_WHISPER) || language == LANG_ADDON)
        *data << uint32(language);
    else
        *data << uint32(LANG_UNIVERSAL);

    switch (type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_RAID:
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        case CHAT_MSG_YELL:
        case CHAT_MSG_WHISPER:
        case CHAT_MSG_CHANNEL:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        case CHAT_MSG_BG_SYSTEM_NEUTRAL:
        case CHAT_MSG_BG_SYSTEM_ALLIANCE:
        case CHAT_MSG_BG_SYSTEM_HORDE:
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
            target_guid = session ? session->GetPlayer()->GetGUID() : 0;
            break;
        case CHAT_MSG_MONSTER_SAY:
        case CHAT_MSG_MONSTER_PARTY:
        case CHAT_MSG_MONSTER_YELL:
        case CHAT_MSG_MONSTER_WHISPER:
        case CHAT_MSG_MONSTER_EMOTE:
        case CHAT_MSG_RAID_BOSS_WHISPER:
        case CHAT_MSG_RAID_BOSS_EMOTE:
        {
            *data << uint64(speaker->GetGUID());
            *data << uint32(0);                             // 2.1.0
            *data << uint32(strlen(speaker->GetName()) + 1);
            *data << speaker->GetName();
            uint64 listener_guid = 0;
            *data << uint64(listener_guid);
            if (listener_guid && !IS_PLAYER_GUID(listener_guid))
            {
                *data << uint32(1);                         // string listener_name_length
                *data << uint8(0);                          // string listener_name
            }
            *data << uint32(messageLength);
            *data << message;
            *data << uint8(0);
            return;
        }
        default:
            if (type != CHAT_MSG_REPLY && type != CHAT_MSG_IGNORED && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
                target_guid = 0;                            // only for CHAT_MSG_WHISPER_INFORM used original value target_guid
            break;
    }

    *data << uint64(target_guid);                           // there 0 for BG messages
    *data << uint32(0);                                     // can be chat msg group or something

    if (type == CHAT_MSG_CHANNEL)
    {
        ASSERT(channelName);
        *data << channelName;
    }

    *data << uint64(target_guid);
    *data << uint32(messageLength);
    *data << message;
    if (session != 0 && type != CHAT_MSG_REPLY && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
        *data << uint8(session->GetPlayer() ? session->GetPlayer()->chatTag() : 0);
    else
        *data << uint8(0);
}

Player * ChatHandler::getSelectedPlayer()
{
    if (!m_session)
        return NULL;

    uint64 guid  = m_session->GetPlayer()->GetSelection();

    if (guid == 0)
        return m_session->GetPlayer();

    return sObjectMgr.GetPlayerInWorld(guid);
}

Unit* ChatHandler::getSelectedUnit()
{
    if (!m_session)
        return NULL;

    uint64 guid = m_session->GetPlayer()->GetSelection();

    if (guid == 0)
        return m_session->GetPlayer();

    return m_session->GetPlayer()->GetMap()->GetUnit(guid);
}

Creature* ChatHandler::getSelectedCreature()
{
    if (!m_session)
        return NULL;

    Player * tmp = m_session->GetPlayer();

    return tmp->GetMap()->GetCreatureOrPet(tmp->GetSelection());
}

char* ChatHandler::extractKeyFromLink(char* text, char const* linkType, char** something1)
{
    // skip empty
    if (!text)
        return NULL;

    // skip spaces
    while (*text==' '||*text=='\t'||*text=='\b')
        ++text;

    if (!*text)
        return NULL;

    // return non link case
    if (text[0]!='|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if (!check)
        return NULL;                                        // end of data

    char* cLinkType = strtok(NULL, ":");                    // linktype
    if (!cLinkType)
        return NULL;                                        // end of data

    if (strcmp(cLinkType,linkType) != 0)
    {
        strtok(NULL, " ");                                  // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
        SendSysMessage(LANG_WRONG_LINK_TYPE);
        return NULL;
    }

    char* cKeys = strtok(NULL, "|");                        // extract keys and values
    char* cKeysTail = strtok(NULL, "");

    char* cKey = strtok(cKeys, ":|");                       // extract key
    if (something1)
        *something1 = strtok(NULL, ":|");                   // extract something

    strtok(cKeysTail, "]");                                 // restart scan tail and skip name with possible spaces
    strtok(NULL, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after retturn from function
    return cKey;
}

char* ChatHandler::extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1)
{
    // skip empty
    if (!text)
        return NULL;

    // skip spaces
    while (*text==' '||*text=='\t'||*text=='\b')
        ++text;

    if (!*text)
        return NULL;

    // return non link case
    if (text[0]!='|')
        return strtok(text, " ");

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r

    char* check = strtok(text, "|");                        // skip color
    if (!check)
        return NULL;                                        // end of data

    char* cLinkType = strtok(NULL, ":");                    // linktype
    if (!cLinkType)
        return NULL;                                        // end of data

    for (int i = 0; linkTypes[i]; ++i)
    {
        if (strcmp(cLinkType,linkTypes[i]) == 0)
        {
            char* cKeys = strtok(NULL, "|");                // extract keys and values
            char* cKeysTail = strtok(NULL, "");

            char* cKey = strtok(cKeys, ":|");               // extract key
            if (something1)
                *something1 = strtok(NULL, ":|");           // extract something

            strtok(cKeysTail, "]");                         // restart scan tail and skip name with possible spaces
            strtok(NULL, " ");                              // skip link tail (to allow continue strtok(NULL,s) use after return from function
            if (found_idx)
                *found_idx = i;
            return cKey;
        }
    }

    strtok(NULL, " ");                                      // skip link tail (to allow continue strtok(NULL,s) use after return from function
    SendSysMessage(LANG_WRONG_LINK_TYPE);
    return NULL;
}

char const *fmtstring(char const *format, ...)
{
    va_list        argptr;
    #define    MAX_FMT_STRING    32000
    static char        temp_buffer[MAX_FMT_STRING];
    static char        string[MAX_FMT_STRING];
    static int        index = 0;
    char    *buf;
    int len;

    va_start(argptr, format);
    vsnprintf(temp_buffer,MAX_FMT_STRING, format, argptr);
    va_end(argptr);

    len = strlen(temp_buffer);

    if (len >= MAX_FMT_STRING)
        return "ERROR";

    if (len + index >= MAX_FMT_STRING-1)
    {
        index = 0;
    }

    buf = &string[index];
    memcpy(buf, temp_buffer, len+1);

    index += len + 1;

    return buf;
}

GameObject* ChatHandler::GetObjectGlobalyWithGuidOrNearWithDbGuid(uint32 lowguid,uint32 entry)
{
    if (!m_session)
        return NULL;

    Player* pl = m_session->GetPlayer();

    GameObject* obj = pl->GetMap()->GetGameObject(MAKE_NEW_GUID(lowguid, entry, HIGHGUID_GAMEOBJECT));

    if (!obj && sObjectMgr.GetGOData(lowguid))                   // guid is DB guid of object
    {
        // search near player then
        Hellground::GameObjectWithDbGUIDCheck go_check(*pl,lowguid);
        Hellground::ObjectSearcher<GameObject, Hellground::GameObjectWithDbGUIDCheck> checker(obj,go_check);

        Cell::VisitGridObjects(pl,checker, pl->GetMap()->GetVisibilityDistance());
    }

    return obj;
}

static char const* const spellTalentKeys[] = {
    "Hspell",
    "Htalent",
    0
};

uint32 ChatHandler::extractSpellIdFromLink(char* text)
{
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id,rank|h[name]|h|r
    int type = 0;
    char* rankS = NULL;
    char* idS = extractKeyFromLink(text,spellTalentKeys,&type,&rankS);
    if (!idS)
        return 0;

    uint32 id = (uint32)atol(idS);

    // spell
    if (type==0)
        return id;

    // talent
    TalentEntry const* talentEntry = sTalentStore.LookupEntry(id);
    if (!talentEntry)
        return 0;

    int32 rank = rankS ? (uint32)atol(rankS) : 0;
    if (rank >= 5)
        return 0;

    if (rank < 0)
        rank = 0;

    return talentEntry->RankID[rank];
}

GameTele const* ChatHandler::extractGameTeleFromLink(char* text)
{
    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    char* cId = extractKeyFromLink(text,"Htele");
    if (!cId)
        return NULL;

    // id case (explicit or from shift link)
    if (cId[0] >= '0' || cId[0] >= '9')
        if (uint32 id = atoi(cId))
            return sObjectMgr.GetGameTele(id);

    return sObjectMgr.GetGameTele(cId);
}

const char *ChatHandler::GetName() const
{
    return m_session->GetPlayer()->GetName();
}

bool ChatHandler::needReportToTarget(Player* chr) const
{
    Player* pl = m_session->GetPlayer();
    return pl != chr && pl->IsVisibleGloballyfor (chr);
}

const char *CliHandler::GetHellgroundString(int32 entry) const
{
    return sObjectMgr.GetHellgroundStringForDBCLocale(entry);
}

bool CliHandler::isAvailable(ChatCommand const& cmd, bool) const
{
    // skip non-console commands in console case
    return cmd.AllowConsole;
}

void CliHandler::SendSysMessage(const char *str)
{
    m_print(str);
    m_print("\r\n");
}

const char *CliHandler::GetName() const
{
    return GetHellgroundString(LANG_CONSOLE_COMMAND);
}

bool CliHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

bool ChatHandler::GetPlayerGroupAndGUIDByName(const char* cname, Player* &plr, Group* &group, uint64 &guid, bool offline)
{
    plr  = NULL;
    guid = 0;

    if (cname)
    {
        std::string name = cname;
        if (!name.empty())
        {
            if (!normalizePlayerName(name))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }

            plr = sObjectMgr.GetPlayerInWorld(name.c_str());
            if (offline)
                guid = sObjectMgr.GetPlayerGUIDByName(name.c_str());
        }
    }

    if (plr)
    {
        group = plr->GetGroup();
        if (!guid || !offline)
            guid = plr->GetGUID();
    }
    else
    {
        if (getSelectedPlayer())
            plr = getSelectedPlayer();
        else
            plr = m_session->GetPlayer();

        if (!guid || !offline)
            guid  = plr->GetGUID();
        group = plr->GetGroup();
    }

    return true;
}

std::string ChatHandler::GetNameLink(const std::string & name)
{
    return "|Hplayer:" + name + "|h[" + name + "]|h";
}

bool ChatHandler::isValidChatMessage(const char* message)
{
    /*

    valid examples:
    |cffa335ee|Hitem:812:0:0:0:0:0:0:0:70|h[Glowing Brightwood Staff]|h|r
    |cff808080|Hquest:2278:47|h[The Platinum Discs]|h|r
    |cff4e96f7|Htalent:2232:-1|h[Taste for Blood]|h|r
    |cff71d5ff|Hspell:21563|h[Command]|h|r
    |cffffd000|Henchant:3919|h[Engineering: Rough Dynamite]|h|r

    | will be escaped to ||
    */

    if (strlen(message) > 255)
        return false;

    const char validSequence[6] = "cHhhr";
    const char* validSequenceIterator = validSequence;

    // more simple checks
    if (sWorld.getConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) < 3)
    {
        const std::string validCommands = "cHhr|";

        while (*message)
        {
            // find next pipe command
            message = strchr(message, '|');

            if (!message)
                return true;

            ++message;
            char commandChar = *message;
            if (validCommands.find(commandChar) == std::string::npos)
                return false;

            ++message;
            // validate sequence
            if (sWorld.getConfig(CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY) == 2)
            {
                if (commandChar == *validSequenceIterator)
                {
                    if (validSequenceIterator == validSequence + 4)
                        validSequenceIterator = validSequence;
                    else
                        ++validSequenceIterator;
                }
                else
                    return false;
            }
        }
        return true;
    }

    std::istringstream reader(message);
    char buffer[256];

    uint32 color = 0;

    ItemPrototype const* linkedItem;
    Quest const* linkedQuest;
    SpellEntry const* linkedSpell = NULL;

    while (!reader.eof())
    {
        if (validSequence == validSequenceIterator)
        {
            linkedItem = NULL;
            linkedQuest = NULL;
            linkedSpell = NULL;

            reader.ignore(255, '|');
        }
        else if (reader.get() != '|')
        {
            #ifdef HELLGROUND_DEBUG
            sLog.outBasic("ChatHandler::isValidChatMessage sequence aborted unexpectedly");
            #endif
            return false;
        }

        // pipe has always to be followed by at least one char
        if (reader.peek() == '\0')
        {
            #ifdef HELLGROUND_DEBUG
            sLog.outBasic("ChatHandler::isValidChatMessage pipe followed by \\0");
            #endif
            return false;
        }

        // no further pipe commands
        if (reader.eof())
            break;

        char commandChar;
        reader >> commandChar;

        // | in normal messages is escaped by ||
        if (commandChar != '|')
        {
            if (commandChar == *validSequenceIterator)
            {
                if (validSequenceIterator == validSequence + 4)
                    validSequenceIterator = validSequence;
                else
                    ++validSequenceIterator;
            }
            else
            {
                #ifdef HELLGROUND_DEBUG
                sLog.outBasic("ChatHandler::isValidChatMessage invalid sequence, expected %c but got %c", *validSequenceIterator, commandChar);
                #endif
                return false;
            }
        }
        else if (validSequence != validSequenceIterator)
        {
            // no escaped pipes in sequences
            #ifdef HELLGROUND_DEBUG
            sLog.outBasic("ChatHandler::isValidChatMessage got escaped pipe in sequence");
            #endif
            return false;
        }

        switch (commandChar)
        {
        case 'c':
            color = 0;
            // validate color, expect 8 hex chars
            for (int i = 0; i < 8; i++)
            {
                char c;
                reader >> c;
                if (!c)
                {
                    #ifdef HELLGROUND_DEBUG
                    sLog.outBasic("ChatHandler::isValidChatMessage got \\0 while reading color in |c command");
                    #endif
                    return false;
                }

                color <<= 4;
                // check for hex char
                if (c >= '0' && c <= '9')
                {
                    color |= c - '0';
                    continue;
                }
                if (c >= 'a' && c <= 'f')
                {
                    color |= 10 + c - 'a';
                    continue;
                }
                #ifdef HELLGROUND_DEBUG
                sLog.outBasic("ChatHandler::isValidChatMessage got non hex char '%c' while reading color", c);
                #endif
                return false;
            }
            break;
        case 'H':
            // read chars up to colon  = link type
            reader.getline(buffer, 256, ':');

            if (strcmp(buffer, "item") == 0)
            {
                // read item entry
                reader.getline(buffer, 256, ':');

                linkedItem = sObjectMgr.GetItemPrototype(atoi(buffer));
                if (!linkedItem)
                {
                    #ifdef HELLGROUND_DEBUG
                    sLog.outBasic("ChatHandler::isValidChatMessage got invalid itemID %u in |item command", atoi(buffer));
                    #endif
                    return false;
                }

                if (color != ItemQualityColors[linkedItem->Quality])
                {
                    #ifdef HELLGROUND_DEBUG
                    sLog.outBasic("ChatHandler::isValidChatMessage linked item has color %u, but user claims %u", ItemQualityColors[linkedItem->Quality],
                                  color);
                    #endif
                    return false;
                }

                char c = reader.peek();

                // ignore enchants etc.
                while ((c >= '0' && c <= '9') || (c == ':' || c == '-'))
                {
                    reader.ignore(1);
                    c = reader.peek();
                }
            }
            else if (strcmp(buffer, "quest") == 0)
            {
                // no color check for questlinks, each client will adapt it anyway
                uint32 questid = 0;
                // read questid
                char c = reader.peek();
                while (c >= '0' && c <= '9')
                {
                    reader.ignore(1);
                    questid *= 10;
                    questid += c - '0';
                    c = reader.peek();
                }

                linkedQuest = sObjectMgr.GetQuestTemplate(questid);

                if (!linkedQuest)
                {
                    #ifdef HELLGROUND_DEBUG
                    sLog.outBasic("ChatHandler::isValidChatMessage Questtemplate %u not found", questid);
                    #endif
                    return false;
                }
                c = reader.peek();
                // level
                while (c != '|' && c != '\0')
                {
                    reader.ignore(1);
                    c = reader.peek();
                }
            }
            else if (strcmp(buffer, "talent") == 0)
            {
                // talent links are always supposed to be blue
                if (color != CHAT_LINK_COLOR_TALENT)
                    return false;

                // read talent entry
                reader.getline(buffer, 256, ':');
                TalentEntry const* talentInfo = sTalentStore.LookupEntry(atoi(buffer));
                if (!talentInfo)
                    return false;

                linkedSpell = sSpellTemplate.LookupEntry<SpellEntry>(talentInfo->RankID[0]);
                if (!linkedSpell)
                    return false;

                char c = reader.peek();
                // skillpoints? whatever, drop it
                while (c != '|' && c != '\0')
                {
                    reader.ignore(1);
                    c = reader.peek();
                }
            }
            else if (strcmp(buffer, "spell") == 0)
            {
                if (color != CHAT_LINK_COLOR_SPELL)
                    return false;

                uint32 spellid = 0;
                // read spell entry
                char c = reader.peek();
                while (c >= '0' && c <= '9')
                {
                    reader.ignore(1);
                    spellid *= 10;
                    spellid += c - '0';
                    c = reader.peek();
                }
                linkedSpell = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
                if (!linkedSpell)
                    return false;
            }
            else if (strcmp(buffer, "enchant") == 0)
            {
                if (color != CHAT_LINK_COLOR_ENCHANT)
                    return false;

                uint32 spellid = 0;
                // read spell entry
                char c = reader.peek();
                while (c >= '0' && c <= '9')
                {
                    reader.ignore(1);
                    spellid *= 10;
                    spellid += c - '0';
                    c = reader.peek();
                }
                linkedSpell = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
                if (!linkedSpell)
                    return false;
            }
            else
            {
                #ifdef HELLGROUND_DEBUG
                sLog.outBasic("ChatHandler::isValidChatMessage user sent unsupported link type '%s'", buffer);
                #endif
                return false;
            }
            break;
        case 'h':
            // if h is next element in sequence, this one must contain the linked text :)
            if (*validSequenceIterator == 'h')
            {
                // links start with '['
                if (reader.get() != '[')
                {
                    #ifdef HELLGROUND_DEBUG
                    sLog.outBasic("ChatHandler::isValidChatMessage link caption doesn't start with '['");
                    #endif
                    return false;
                }
                reader.getline(buffer, 256, ']');

                // verify the link name
                if (linkedSpell)
                {
                    // spells with that flag have a prefix of "$PROFESSION: "
                    if (linkedSpell->Attributes & SPELL_ATTR_TRADESPELL)
                    {
                        // lookup skillid
                        SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(linkedSpell->Id);
                        SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(linkedSpell->Id);

                        if (lower == upper)
                            return false;

                        SkillLineAbilityEntry const* skillInfo = lower->second;

                        if (!skillInfo)
                            return false;

                        SkillLineEntry const* skillLine = sSkillLineStore.LookupEntry(skillInfo->skillId);
                        if (!skillLine)
                            return false;

                        // Gensen: disabled, because we have many locales
                        //for (uint8 i = 0; i < MAX_LOCALE; ++i)
                        //{
                        //    uint32 skillLineNameLength = strlen(skillLine->name[i]);
                        //    if (skillLineNameLength > 0 && strncmp(skillLine->name[i], buffer, skillLineNameLength) == 0)
                        //    {
                        //        // found the prefix, remove it to perform spellname validation below
                        //        // -2 = strlen(": ")
                        //        uint32 spellNameLength = strlen(buffer) - skillLineNameLength - 2;
                        //        memmove(buffer, buffer + skillLineNameLength + 2, spellNameLength + 1);
                        //    }
                        //}
                    }
                    // Gensen: disabled, because we have many locales
                    //bool foundName = false;
                    //for (uint8 i = 0; i < MAX_LOCALE; ++i)
                    //{
                    //    if (*linkedSpell->SpellName[i] && strcmp(linkedSpell->SpellName[i], buffer) == 0)
                    //    {
                    //        foundName = true;
                    //        break;
                    //    }
                    //}
                    //if (!foundName)
                    //    return false;
                }
                else if (linkedQuest)
                {
                    if (linkedQuest->GetName() != buffer)
                    {
                        QuestLocale const* ql = sObjectMgr.GetQuestLocale(linkedQuest->GetQuestId());

                        if (!ql)
                        {
                            #ifdef HELLGROUND_DEBUG
                            sLog.outBasic("ChatHandler::isValidChatMessage default questname didn't match and there is no locale");
                            #endif
                            return false;
                        }

                        bool foundName = false;
                        for (uint8 i = 0; i < ql->Title.size(); i++)
                        {
                            if (ql->Title[i] == buffer)
                            {
                                foundName = true;
                                break;
                            }
                        }
                        if (!foundName)
                        {
                            #ifdef HELLGROUND_DEBUG
                            sLog.outBasic("ChatHandler::isValidChatMessage no quest locale title matched");
                            #endif
                            return false;
                        }
                    }
                }
                else if (linkedItem)
                {
                    if (strcmp(linkedItem->Name1, buffer) != 0)
                    {
                        ItemLocale const* il = sObjectMgr.GetItemLocale(linkedItem->ItemId);

                        if (!il)
                        {
                            #ifdef HELLGROUND_DEBUG
                            sLog.outBasic("ChatHandler::isValidChatMessage linked item name doesn't is wrong and there is no localization");
                            #endif
                            return false;
                        }

						// cause error to link some items like entry 9779
                        //bool foundName = false;
                        //for (uint8 i = 0; i < il->Name.size(); ++i)
                        //{
                        //    if (il->Name[i] == buffer)
                        //    {
                        //        foundName = true;
                        //        break;
                        //    }
                        //}
                        //if (!foundName)
                        //{
                        //    #ifdef HELLGROUND_DEBUG
                        //    sLog.outBasic("ChatHandler::isValidChatMessage linked item name wasn't found in any localization");
                        //    #endif
                        //    return false;
                        //}
                    }
                }
                // that place should never be reached - if nothing linked has been set in |H
                // it will return false before
                else
                    return false;
            }
            break;
        case 'r':
        case '|':
            // no further payload
            break;
        default:
            #ifdef HELLGROUND_DEBUG
            sLog.outBasic("ChatHandler::isValidChatMessage got invalid command |%c", commandChar);
            #endif
            return false;
        }
    }

    // check if every opened sequence was also closed properly
    #ifdef HELLGROUND_DEBUG
    if (validSequence != validSequenceIterator)
        sLog.outBasic("ChatHandler::isValidChatMessage EOF in active sequence");
    #endif
    return validSequence == validSequenceIterator;
}