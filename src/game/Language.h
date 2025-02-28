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

#ifndef HELLGROUND_LANGUAGE_H
#define HELLGROUND_LANGUAGE_H

enum HellgroundStrings
{
    // for chat commands
    LANG_SELECT_CHAR_OR_CREATURE        = 1,
    LANG_SELECT_CREATURE                = 2,

    // level 0 chat
    LANG_SYSTEMMESSAGE                  = 3,
    LANG_EVENTMESSAGE                   = 4,
    LANG_NO_HELP_CMD                    = 5,
    LANG_NO_CMD                         = 6,
    LANG_NO_SUBCMD                      = 7,
    LANG_SUBCMDS_LIST                   = 8,
    LANG_AVIABLE_CMD                    = 9,
    LANG_CMD_SYNTAX                     = 10,
    LANG_ACCOUNT_LEVEL                  = 11,
    LANG_CONNECTED_USERS                = 12,
    LANG_UPTIME                         = 13,
    LANG_PLAYER_SAVED                   = 14,
    LANG_PLAYERS_SAVED                  = 15,
    LANG_GMS_ON_SRV                     = 16,
    LANG_GMS_NOT_LOGGED                 = 17,
    LANG_YOU_IN_FLIGHT                  = 18,
    //LANG_YOU_IN_BATTLEGROUND            = 19, not used
    //LANG_TARGET_IN_FLIGHT               = 20, not used
    LANG_CHAR_IN_FLIGHT                 = 21,
    LANG_CHAR_NON_MOUNTED               = 22,
    LANG_YOU_IN_COMBAT                  = 23,
    LANG_YOU_USED_IT_RECENTLY           = 24,
    LANG_COMMAND_NOTCHANGEPASSWORD      = 25,
    LANG_COMMAND_PASSWORD               = 26,
    LANG_COMMAND_WRONGOLDPASSWORD       = 27,
    LANG_COMMAND_ACCLOCKLOCKED          = 28,
    LANG_COMMAND_ACCLOCKUNLOCKED        = 29,
    LANG_SPELL_RANK                     = 30,
    LANG_KNOWN                          = 31,
    LANG_LEARN                          = 32,
    LANG_PASSIVE                        = 33,
    LANG_TALENT                         = 34,
    LANG_ACTIVE                         = 35,
    LANG_COMPLETE                       = 36,
    LANG_OFFLINE                        = 37,
    LANG_ON                             = 38,
    LANG_OFF                            = 39,
    LANG_YOU_ARE                        = 40,
    LANG_VISIBLE                        = 41,
    LANG_INVISIBLE                      = 42,
    LANG_DONE                           = 43,
    LANG_YOU                            = 44,
    LANG_UNKNOWN                        = 45,
    LANG_ERROR                          = 46,
    LANG_NON_EXIST_CHARACTER            = 47,
    LANG_FRIEND_IGNORE_UNKNOWN          = 48,
    LANG_LEVEL_MINREQUIRED              = 49,
    LANG_LEVEL_MINREQUIRED_AND_ITEM     = 50,
    LANG_NPC_TAINER_HELLO               = 51,
    LANG_COMMAND_INVALID_ITEM_COUNT     = 52,
    LANG_COMMAND_MAIL_ITEMS_LIMIT       = 53,
    LANG_NEW_PASSWORDS_NOT_MATCH        = 54,
    LANG_PASSWORD_TOO_LONG              = 55,
    LANG_MOTD_CURRENT                   = 56,
    LANG_USING_WORLD_DB                 = 57,
    LANG_USING_SCRIPT_LIB               = 58,
    // Room for more level 0              59-99 not used

    // level 1 chat
    LANG_GLOBAL_NOTIFY                  = 100,
    LANG_MAP_POSITION                   = 101,
    LANG_IS_TELEPORTED                  = 102,
    LANG_CANNOT_SUMMON_TO_INST          = 103,
    LANG_CANNOT_GO_TO_INST_PARTY        = 104,
    LANG_CANNOT_GO_TO_INST_GM           = 105,
    LANG_CANNOT_GO_INST_INST            = 106,
    LANG_CANNOT_SUMMON_INST_INST        = 107,

    LANG_SUMMONING                      = 108,
    LANG_SUMMONED_BY                    = 109,
    LANG_TELEPORTING_TO                 = 110,
    LANG_TELEPORTED_TO_BY               = 111,
    LANG_NO_PLAYER                      = 112,
    LANG_APPEARING_AT                   = 113,
    LANG_APPEARING_TO                   = 114,

    LANG_BAD_VALUE                      = 115,
    LANG_NO_CHAR_SELECTED               = 116,
    LANG_NOT_IN_GROUP                   = 117,

    LANG_YOU_CHANGE_HP                  = 118,
    LANG_YOURS_HP_CHANGED               = 119,
    LANG_YOU_CHANGE_MANA                = 120,
    LANG_YOURS_MANA_CHANGED             = 121,
    LANG_YOU_CHANGE_ENERGY              = 122,
    LANG_YOURS_ENERGY_CHANGED           = 123,

    LANG_CURRENT_ENERGY                 = 124,              //log
    LANG_YOU_CHANGE_RAGE                = 125,
    LANG_YOURS_RAGE_CHANGED             = 126,
    LANG_YOU_CHANGE_LVL                 = 127,
    LANG_CURRENT_FACTION                = 128,
    LANG_WRONG_FACTION                  = 129,
    LANG_YOU_CHANGE_FACTION             = 130,
    LANG_YOU_CHANGE_SPELLFLATID         = 131,
    LANG_YOURS_SPELLFLATID_CHANGED      = 132,
    LANG_YOU_GIVE_TAXIS                 = 133,
    LANG_YOU_REMOVE_TAXIS               = 134,
    LANG_YOURS_TAXIS_ADDED              = 135,
    LANG_YOURS_TAXIS_REMOVED            = 136,

    LANG_YOU_CHANGE_ASPEED              = 137,
    LANG_YOURS_ASPEED_CHANGED           = 138,
    LANG_YOU_CHANGE_SPEED               = 139,
    LANG_YOURS_SPEED_CHANGED            = 140,
    LANG_YOU_CHANGE_SWIM_SPEED          = 141,
    LANG_YOURS_SWIM_SPEED_CHANGED       = 142,
    LANG_YOU_CHANGE_BACK_SPEED          = 143,
    LANG_YOURS_BACK_SPEED_CHANGED       = 144,
    LANG_YOU_CHANGE_FLY_SPEED           = 145,
    LANG_YOURS_FLY_SPEED_CHANGED        = 146,

    LANG_YOU_CHANGE_SIZE                = 147,
    LANG_YOURS_SIZE_CHANGED             = 148,
    LANG_NO_MOUNT                       = 149,
    LANG_YOU_GIVE_MOUNT                 = 150,
    LANG_MOUNT_GIVED                    = 151,

    LANG_CURRENT_MONEY                  = 152,
    LANG_YOU_TAKE_ALL_MONEY             = 153,
    LANG_YOURS_ALL_MONEY_GONE           = 154,
    LANG_YOU_TAKE_MONEY                 = 155,
    LANG_YOURS_MONEY_TAKEN              = 156,
    LANG_YOU_GIVE_MONEY                 = 157,
    LANG_YOURS_MONEY_GIVEN              = 158,
    LANG_YOU_HEAR_SOUND                 = 159,

    LANG_NEW_MONEY                      = 160,              // Log

    LANG_REMOVE_BIT                     = 161,
    LANG_SET_BIT                        = 162,
    LANG_COMMAND_TELE_TABLEEMPTY        = 163,
    LANG_COMMAND_TELE_NOTFOUND          = 164,
    LANG_COMMAND_TELE_PARAMETER         = 165,
    LANG_COMMAND_TELE_NOLOCATION        = 166,
    //                                    167               // not used
    LANG_COMMAND_TELE_LOCATION          = 168,

    LANG_MAIL_SENT                      = 169,
    LANG_SOUND_NOT_EXIST                = 170,
    LANG_TELEPORTED_TO_BY_CONSOLE       = 171,
    LANG_CONSOLE_COMMAND                = 172,
    // Room for more level 1              173-199 not used

    // level 2 chat
    LANG_NO_SELECTION                   = 200,
    LANG_OBJECT_GUID                    = 201,
    LANG_TOO_LONG_NAME                  = 202,
    LANG_CHARS_ONLY                     = 203,
    LANG_TOO_LONG_SUBNAME               = 204,
    LANG_NOT_IMPLEMENTED                = 205,

    LANG_ITEM_ADDED_TO_LIST             = 206,
    LANG_ITEM_NOT_FOUND                 = 207,
    LANG_ITEM_DELETED_FROM_LIST         = 208,
    LANG_ITEM_NOT_IN_LIST               = 209,
    LANG_ITEM_ALREADY_IN_LIST           = 210,

    LANG_RESET_SPELLS_ONLINE            = 211,
    LANG_RESET_SPELLS_OFFLINE           = 212,
    LANG_RESET_TALENTS_ONLINE           = 213,
    LANG_RESET_TALENTS_OFFLINE          = 214,
    LANG_RESET_SPELLS                   = 215,
    LANG_RESET_TALENTS                  = 216,

    LANG_RESETALL_UNKNOWN_CASE          = 217,
    LANG_RESETALL_SPELLS                = 218,
    LANG_RESETALL_TALENTS               = 219,

    LANG_WAYPOINT_NOTFOUND              = 220,
    LANG_WAYPOINT_NOTFOUNDLAST          = 221,
    LANG_WAYPOINT_NOTFOUNDSEARCH        = 222,
    LANG_WAYPOINT_NOTFOUNDDBPROBLEM     = 223,
    LANG_WAYPOINT_CREATSELECTED         = 224,
    LANG_WAYPOINT_CREATNOTFOUND         = 225,
    LANG_WAYPOINT_VP_SELECT             = 226,
    LANG_WAYPOINT_VP_NOTFOUND           = 227,
    LANG_WAYPOINT_VP_NOTCREATED         = 228,
    LANG_WAYPOINT_VP_ALLREMOVED         = 229,
    LANG_WAYPOINT_NOTCREATED            = 230,
    LANG_WAYPOINT_NOGUID                = 231,
    LANG_WAYPOINT_NOWAYPOINTGIVEN       = 232,
    LANG_WAYPOINT_ARGUMENTREQ           = 233,
    LANG_WAYPOINT_ADDED                 = 234,
    LANG_WAYPOINT_ADDED_NO              = 235,
    LANG_WAYPOINT_CHANGED               = 236,
    LANG_WAYPOINT_CHANGED_NO            = 237,
    LANG_WAYPOINT_EXPORTED              = 238,
    LANG_WAYPOINT_NOTHINGTOEXPORT       = 239,
    LANG_WAYPOINT_IMPORTED              = 240,
    LANG_WAYPOINT_REMOVED               = 241,
    LANG_WAYPOINT_NOTREMOVED            = 242,
    LANG_WAYPOINT_TOOFAR1               = 243,
    LANG_WAYPOINT_TOOFAR2               = 244,
    LANG_WAYPOINT_TOOFAR3               = 245,
    LANG_WAYPOINT_INFO_TITLE            = 246,
    LANG_WAYPOINT_INFO_WAITTIME         = 247,
    LANG_WAYPOINT_INFO_MODEL            = 248,
    LANG_WAYPOINT_INFO_EMOTE            = 249,
    LANG_WAYPOINT_INFO_SPELL            = 250,
    LANG_WAYPOINT_INFO_TEXT             = 251,
    LANG_WAYPOINT_INFO_AISCRIPT         = 252,

    LANG_RENAME_PLAYER                  = 253,
    LANG_RENAME_PLAYER_GUID             = 254,

    LANG_WAYPOINT_WPCREATNOTFOUND       = 255,
    LANG_WAYPOINT_NPCNOTFOUND           = 256,

    LANG_MOVE_TYPE_SET                  = 257,
    LANG_MOVE_TYPE_SET_NODEL            = 258,
    LANG_USE_BOL                        = 259,
    LANG_VALUE_SAVED                    = 260,
    LANG_VALUE_SAVED_REJOIN             = 261,

    LANG_COMMAND_GOAREATRNOTFOUND       = 262,
    LANG_INVALID_TARGET_COORD           = 263,
    LANG_INVALID_ZONE_COORD             = 264,
    LANG_INVALID_ZONE_MAP               = 265,
    LANG_COMMAND_TARGETOBJNOTFOUND      = 266,
    LANG_COMMAND_GOOBJNOTFOUND          = 267,
    LANG_COMMAND_GOCREATNOTFOUND        = 268,
    LANG_COMMAND_GOCREATMULTIPLE        = 269,
    LANG_COMMAND_DELCREATMESSAGE        = 270,
    LANG_COMMAND_CREATUREMOVED          = 271,
    LANG_COMMAND_CREATUREATSAMEMAP      = 272,
    LANG_COMMAND_OBJNOTFOUND            = 273,
    LANG_COMMAND_DELOBJREFERCREATURE    = 274,
    LANG_COMMAND_DELOBJMESSAGE          = 275,
    LANG_COMMAND_TURNOBJMESSAGE         = 276,
    LANG_COMMAND_MOVEOBJMESSAGE         = 277,
    LANG_COMMAND_VENDORSELECTION        = 278,
    LANG_COMMAND_NEEDITEMSEND           = 279,
    LANG_COMMAND_ADDVENDORITEMITEMS     = 280,
    LANG_COMMAND_KICKSELF               = 281,
    LANG_COMMAND_KICKMESSAGE            = 282,
    LANG_COMMAND_KICKNOTFOUNDPLAYER     = 283,
    LANG_COMMAND_WHISPERACCEPTING       = 284,
    LANG_YOU_CAN_WHISPER_TO_GM_ON       = 285,
    LANG_YOU_CAN_WHISPER_TO_GM_OFF      = 286,
    LANG_COMMAND_CREATGUIDNOTFOUND      = 287,
    LANG_COMMAND_CAN_WHISPER_GM_ON      = 288,
    LANG_COMMAND_CAN_WHISPER_GM_OFF     = 289,
    // TICKET STRINGS NEED REWRITE // 288-296 FREE

    // END
    LANG_COMMAND_SPAWNDIST              = 297,
    LANG_COMMAND_SPAWNTIME              = 298,
    LANG_COMMAND_MODIFY_HONOR           = 299,

    LANG_YOUR_CHAT_DISABLED             = 300,
    LANG_GM_DISABLE_CHAT                = 301,
    LANG_CHAT_ALREADY_ENABLED           = 302,
    LANG_YOUR_CHAT_ENABLED              = 303,
    LANG_GM_ENABLE_CHAT                 = 304,

    LANG_COMMAND_MODIFY_REP             = 305,
    LANG_COMMAND_MODIFY_ARENA           = 306,
    LANG_COMMAND_FACTION_NOTFOUND       = 307,
    LANG_COMMAND_FACTION_UNKNOWN        = 308,
    LANG_COMMAND_FACTION_INVPARAM       = 309,
    LANG_COMMAND_FACTION_DELTA          = 310,
    LANG_FACTION_LIST                   = 311,
    LANG_FACTION_VISIBLE                = 312,
    LANG_FACTION_ATWAR                  = 313,
    LANG_FACTION_PEACE_FORCED           = 314,
    LANG_FACTION_HIDDEN                 = 315,
    LANG_FACTION_INVISIBLE_FORCED       = 316,
    LANG_FACTION_INACTIVE               = 317,
    LANG_REP_HATED                      = 318,
    LANG_REP_HOSTILE                    = 319,
    LANG_REP_UNFRIENDLY                 = 320,
    LANG_REP_NEUTRAL                    = 321,
    LANG_REP_FRIENDLY                   = 322,
    LANG_REP_HONORED                    = 323,
    LANG_REP_REVERED                    = 324,
    LANG_REP_EXALTED                    = 325,
    LANG_COMMAND_FACTION_NOREP_ERROR    = 326,
    LANG_FACTION_NOREPUTATION           = 327,
    LANG_LOOKUP_PLAYER_ACCOUNT          = 328,
    LANG_LOOKUP_PLAYER_CHARACTER        = 329,
    LANG_NO_PLAYERS_FOUND               = 330,
    LANG_EXTENDED_COST_NOT_EXIST        = 331,
    LANG_GM_ON                          = 332,
    LANG_GM_OFF                         = 333,
    LANG_GM_CHAT_ON                     = 334,
    LANG_GM_CHAT_OFF                    = 335,
    LANG_YOU_REPAIR_ITEMS               = 336,
    LANG_YOUR_ITEMS_REPAIRED            = 337,
    LANG_YOU_SET_WATERWALK              = 338,
    LANG_YOUR_WATERWALK_SET             = 339,
    LANG_CREATURE_FOLLOW_YOU_NOW        = 340,
    LANG_CREATURE_NOT_FOLLOW_YOU        = 341,
    LANG_CREATURE_NOT_FOLLOW_YOU_NOW    = 342,
    LANG_CREATURE_NON_TAMEABLE          = 343,
    LANG_YOU_ALREADY_HAVE_PET           = 344,

    LANG_YOUR_CHAT_IS_DISABLED          = 345,
    LANG_MUTEINFO_NOCHARACTER           = 346,
    LANG_MUTEINFO_NOACCOUNTMUTE         = 347,
    LANG_MUTEINFO_MUTEHISTORY           = 348,
    LANG_MUTEINFO_HISTORYENTRY          = 349,
    LANG_YES                   = 350,
    LANG_JUST_NO                    = 351,
    LANG_MUTEINFO_TROLLMUTE_HISTORY     = 352,
    LANG_MUTEINFO_NOACCOUNT_TROLLMUTE   = 353,
    LANG_GM_TROLLMUTED_PLAYER           = 354,
    LANG_PLAYER_LOADING_WAIT            = 355,

    // Room for more level 2              346-399 not used

    // level 3 chat
    LANG_SCRIPTS_RELOADED               = 400,
    LANG_YOU_CHANGE_SECURITY            = 401,
    LANG_YOURS_SECURITY_CHANGED         = 402,
    LANG_YOURS_SECURITY_IS_LOW          = 403,
    LANG_CREATURE_MOVE_DISABLED         = 404,
    LANG_CREATURE_MOVE_ENABLED          = 405,
    LANG_NO_WEATHER                     = 406,
    LANG_WEATHER_DISABLED               = 407,

    LANG_BAN_YOUBANNED                  = 408,
    LANG_BAN_YOUPERMBANNED              = 409,
    LANG_BAN_NOTFOUND                   = 410,

    LANG_UNBAN_UNBANNED                 = 411,
    LANG_UNBAN_ERROR                    = 412,

    LANG_ACCOUNT_NOT_EXIST              = 413,

    LANG_BANINFO_NOCHARACTER            = 414,
    LANG_BANINFO_NOIP                   = 415,
    LANG_BANINFO_NOACCOUNTBAN           = 416,
    LANG_BANINFO_BANHISTORY             = 417,
    LANG_BANINFO_HISTORYENTRY           = 418,
    LANG_BANINFO_INFINITE               = 419,
    LANG_BANINFO_NEVER                  = 420,
    LANG_BANINFO_YES                    = 421,
    LANG_BANINFO_NO                     = 422,
    LANG_BANINFO_IPENTRY                = 423,

    LANG_BANLIST_NOIP                   = 424,
    LANG_BANLIST_NOACCOUNT              = 425,
    LANG_BANLIST_NOCHARACTER            = 426,
    LANG_BANLIST_MATCHINGIP             = 427,
    LANG_BANLIST_MATCHINGACCOUNT        = 428,

    LANG_COMMAND_LEARN_MANY_SPELLS      = 429,
    LANG_COMMAND_LEARN_CLASS_SPELLS     = 430,
    LANG_COMMAND_LEARN_CLASS_TALENTS    = 431,
    LANG_COMMAND_LEARN_ALL_LANG         = 432,
    LANG_COMMAND_LEARN_ALL_CRAFT        = 433,
    LANG_COMMAND_COULDNOTFIND           = 434,
    LANG_COMMAND_ITEMIDINVALID          = 435,
    LANG_COMMAND_NOITEMFOUND            = 436,
    LANG_COMMAND_LISTOBJINVALIDID       = 437,
    LANG_COMMAND_LISTITEMMESSAGE        = 438,
    LANG_COMMAND_LISTOBJMESSAGE         = 439,
    LANG_COMMAND_INVALIDCREATUREID      = 440,
    LANG_COMMAND_LISTCREATUREMESSAGE    = 441,
    LANG_COMMAND_NOAREAFOUND            = 442,
    LANG_COMMAND_NOITEMSETFOUND         = 443,
    LANG_COMMAND_NOSKILLFOUND           = 444,
    LANG_COMMAND_NOSPELLFOUND           = 445,
    LANG_COMMAND_NOQUESTFOUND           = 446,
    LANG_COMMAND_NOCREATUREFOUND        = 447,
    LANG_COMMAND_NOGAMEOBJECTFOUND      = 448,
    LANG_COMMAND_GRAVEYARDNOEXIST       = 449,
    LANG_COMMAND_GRAVEYARDALRLINKED     = 450,
    LANG_COMMAND_GRAVEYARDLINKED        = 451,
    LANG_COMMAND_GRAVEYARDWRONGZONE     = 452,
    //                                  = 453,
    LANG_COMMAND_GRAVEYARDERROR         = 454,
    LANG_COMMAND_GRAVEYARD_NOTEAM       = 455,
    LANG_COMMAND_GRAVEYARD_ANY          = 456,
    LANG_COMMAND_GRAVEYARD_ALLIANCE     = 457,
    LANG_COMMAND_GRAVEYARD_HORDE        = 458,
    LANG_COMMAND_GRAVEYARDNEAREST       = 459,
    LANG_COMMAND_ZONENOGRAVEYARDS       = 460,
    LANG_COMMAND_ZONENOGRAFACTION       = 461,
    LANG_COMMAND_TP_ALREADYEXIST        = 462,
    LANG_COMMAND_TP_ADDED               = 463,
    LANG_COMMAND_TP_ADDEDERR            = 464,
    LANG_COMMAND_TP_DELETED             = 465,
    //                                    466,              // not used

    LANG_COMMAND_TARGET_LISTAURAS       = 467,
    LANG_COMMAND_TARGET_AURADETAIL      = 468,
    LANG_COMMAND_TARGET_LISTAURATYPE    = 469,
    LANG_COMMAND_TARGET_AURASIMPLE      = 470,

    LANG_COMMAND_QUEST_NOTFOUND         = 471,
    LANG_COMMAND_QUEST_STARTFROMITEM    = 472,
    LANG_COMMAND_QUEST_REMOVED          = 473,
    LANG_COMMAND_QUEST_REWARDED         = 474,
    LANG_COMMAND_QUEST_COMPLETE         = 475,
    LANG_COMMAND_QUEST_ACTIVE           = 476,

    LANG_COMMAND_FLYMODE_STATUS         = 477,

    LANG_COMMAND_OPCODESENT             = 478,

    LANG_COMMAND_IMPORT_SUCCESS         = 479,
    LANG_COMMAND_IMPORT_FAILED          = 480,
    LANG_COMMAND_EXPORT_SUCCESS         = 481,
    LANG_COMMAND_EXPORT_FAILED          = 482,

    LANG_COMMAND_SPELL_BROKEN           = 483,

    LANG_SET_SKILL                      = 484,
    LANG_SET_SKILL_ERROR                = 485,

    LANG_INVALID_SKILL_ID               = 486,
    LANG_LEARNING_GM_SKILLS             = 487,
    LANG_YOU_KNOWN_SPELL                = 488,
    LANG_TARGET_KNOWN_SPELL             = 489,
    LANG_UNKNOWN_SPELL                  = 490,
    LANG_FORGET_SPELL                   = 491,
    LANG_REMOVEALL_COOLDOWN             = 492,
    LANG_REMOVE_COOLDOWN                = 493,

    LANG_ADDITEM                        = 494,              //log
    LANG_ADDITEMSET                     = 495,              //log
    LANG_REMOVEITEM                     = 496,
    LANG_ITEM_CANNOT_CREATE             = 497,
    LANG_INSERT_GUILD_NAME              = 498,
    LANG_PLAYER_NOT_FOUND               = 499,
    LANG_PLAYER_IN_GUILD                = 500,
    LANG_GUILD_NOT_CREATED              = 501,
    LANG_NO_ITEMS_FROM_ITEMSET_FOUND    = 502,

    LANG_DISTANCE                       = 503,

    LANG_ITEM_SLOT                      = 504,
    LANG_ITEM_SLOT_NOT_EXIST            = 505,
    LANG_ITEM_ADDED_TO_SLOT             = 506,
    LANG_ITEM_SAVE_FAILED               = 507,
    LANG_ITEMLIST_SLOT                  = 508,
    LANG_ITEMLIST_MAIL                  = 509,
    LANG_ITEMLIST_AUCTION               = 510,

    LANG_WRONG_LINK_TYPE                = 511,
    LANG_ITEM_LIST_CHAT                 = 512,
    LANG_QUEST_LIST_CHAT                = 513,
    LANG_CREATURE_ENTRY_LIST_CHAT       = 514,
    LANG_CREATURE_LIST_CHAT             = 515,
    LANG_GO_ENTRY_LIST_CHAT             = 516,
    LANG_GO_LIST_CHAT                   = 517,
    LANG_ITEMSET_LIST_CHAT              = 518,
    LANG_TELE_LIST                      = 519,
    LANG_SPELL_LIST                     = 520,
    LANG_SKILL_LIST_CHAT                = 521,

    LANG_GAMEOBJECT_NOT_EXIST           = 522,

    LANG_GAMEOBJECT_CURRENT             = 523,              //log
    LANG_GAMEOBJECT_DETAIL              = 524,
    LANG_GAMEOBJECT_ADD                 = 525,

    LANG_MOVEGENS_LIST                  = 526,
    LANG_MOVEGENS_IDLE                  = 527,
    LANG_MOVEGENS_RANDOM                = 528,
    LANG_MOVEGENS_WAYPOINT              = 529,
    LANG_MOVEGENS_ANIMAL_RANDOM         = 530,
    LANG_MOVEGENS_CONFUSED              = 531,
    LANG_MOVEGENS_CHASE_PLAYER          = 532,
    LANG_MOVEGENS_CHASE_CREATURE        = 533,
    LANG_MOVEGENS_CHASE_NULL            = 534,
    LANG_MOVEGENS_HOME_CREATURE         = 535,
    LANG_MOVEGENS_HOME_PLAYER           = 536,
    LANG_MOVEGENS_FLIGHT                = 537,
    LANG_MOVEGENS_UNKNOWN               = 538,

    LANG_NPCINFO_CHAR                   = 539,
    LANG_NPCINFO_LEVEL                  = 540,
    LANG_NPCINFO_HEALTH                 = 541,
    LANG_NPCINFO_FLAGS                  = 542,
    LANG_NPCINFO_LOOT                   = 543,
    LANG_NPCINFO_POSITION               = 544,
    LANG_NPCINFO_VENDOR                 = 545,
    LANG_NPCINFO_TRAINER                = 546,
    LANG_NPCINFO_DUNGEON_ID             = 547,

    LANG_PINFO_ACCOUNT                  = 548,
    LANG_PINFO_LEVEL                    = 549,
    LANG_PINFO_NO_REP                   = 550,

    LANG_YOU_SET_EXPLORE_ALL            = 551,
    LANG_YOU_SET_EXPLORE_NOTHING        = 552,
    LANG_YOURS_EXPLORE_SET_ALL          = 553,
    LANG_YOURS_EXPLORE_SET_NOTHING      = 554,

    LANG_HOVER_ENABLED                  = 555,
    LANG_HOVER_DISABLED                 = 556,
    LANG_YOURS_LEVEL_UP                 = 557,
    LANG_YOURS_LEVEL_DOWN               = 558,
    LANG_YOURS_LEVEL_PROGRESS_RESET     = 559,
    LANG_EXPLORE_AREA                   = 560,
    LANG_UNEXPLORE_AREA                 = 561,

    LANG_UPDATE                         = 562,
    LANG_UPDATE_CHANGE                  = 563,
    LANG_TOO_BIG_INDEX                  = 564,
    LANG_SET_UINT                       = 565,              //log
    LANG_SET_UINT_FIELD                 = 566,
    LANG_SET_FLOAT                      = 567,              //log
    LANG_SET_FLOAT_FIELD                = 568,
    LANG_GET_UINT                       = 569,              //log
    LANG_GET_UINT_FIELD                 = 570,
    LANG_GET_FLOAT                      = 571,              //log
    LANG_GET_FLOAT_FIELD                = 572,
    LANG_SET_32BIT                      = 573,              //log
    LANG_SET_32BIT_FIELD                = 574,
    LANG_CHANGE_32BIT                   = 575,              //log
    LANG_CHANGE_32BIT_FIELD             = 576,

    LANG_INVISIBLE_INVISIBLE            = 577,
    LANG_INVISIBLE_VISIBLE              = 578,
    LANG_SELECTED_TARGET_NOT_HAVE_VICTIM = 579,

    LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST = 580,
    LANG_COMMAND_NEAROBJMESSAGE         = 581,
    LANG_COMMAND_RAWPAWNTIMES           = 582,

    LANG_EVENT_ENTRY_LIST_CHAT          = 583,
    LANG_NOEVENTFOUND                   = 584,
    LANG_EVENT_NOT_EXIST                = 585,
    LANG_EVENT_INFO                     = 586,
    LANG_EVENT_ALREADY_ACTIVE           = 587,
    LANG_EVENT_NOT_ACTIVE               = 588,

    LANG_MOVEGENS_POINT                 = 589,
    LANG_MOVEGENS_FEAR                  = 590,
    LANG_MOVEGENS_DISTRACT              = 591,

    LANG_COMMAND_LEARN_ALL_RECIPES      = 592,
    LANG_BANLIST_ACCOUNTS               = 593,
    LANG_BANLIST_ACCOUNTS_HEADER        = 594,
    LANG_BANLIST_IPS                    = 595,
    LANG_BANLIST_IPS_HEADER             = 596,
    LANG_GMLIST                         = 597,
    LANG_GMLIST_HEADER                  = 598,
    LANG_GMLIST_EMPTY                   = 599,
    // End Level 3 list, continued at 1100

    // Battleground
    LANG_BG_A_WINS                      = 600,
    LANG_BG_H_WINS                      = 601,
    LANG_BG_WS_ONE_MINUTE               = 602,
    LANG_BG_WS_HALF_MINUTE              = 603,
    LANG_BG_WS_BEGIN                    = 604,

    LANG_BG_WS_CAPTURED_HF              = 605,
    LANG_BG_WS_CAPTURED_AF              = 606,
    LANG_BG_WS_DROPPED_HF               = 607,
    LANG_BG_WS_DROPPED_AF               = 608,
    LANG_BG_WS_RETURNED_AF              = 609,
    LANG_BG_WS_RETURNED_HF              = 610,
    LANG_BG_WS_PICKEDUP_HF              = 611,
    LANG_BG_WS_PICKEDUP_AF              = 612,
    LANG_BG_WS_F_PLACED                 = 613,
    LANG_BG_WS_ALLIANCE_FLAG_RESPAWNED  = 614,
    LANG_BG_WS_HORDE_FLAG_RESPAWNED     = 615,

    LANG_BG_EY_ONE_MINUTE               = 636,
    LANG_BG_EY_HALF_MINUTE              = 637,
    LANG_BG_EY_BEGIN                    = 638,

    LANG_BG_AB_ALLY                     = 650,
    LANG_BG_AB_HORDE                    = 651,
    LANG_BG_AB_NODE_STABLES             = 652,
    LANG_BG_AB_NODE_BLACKSMITH          = 653,
    LANG_BG_AB_NODE_FARM                = 654,
    LANG_BG_AB_NODE_LUMBER_MILL         = 655,
    LANG_BG_AB_NODE_GOLD_MINE           = 656,
    LANG_BG_AB_NODE_TAKEN               = 657,
    LANG_BG_AB_NODE_DEFENDED            = 658,
    LANG_BG_AB_NODE_ASSAULTED           = 659,
    LANG_BG_AB_NODE_CLAIMED             = 660,
    LANG_BG_AB_ONEMINTOSTART            = 661,
    LANG_BG_AB_HALFMINTOSTART           = 662,
    LANG_BG_AB_STARTED                  = 663,
    LANG_BG_AB_A_NEAR_VICTORY           = 664,
    LANG_BG_AB_H_NEAR_VICTORY           = 665,
    LANG_BG_MARK_BY_MAIL                = 666,

    LANG_BG_EY_HAS_TAKEN_A_M_TOWER      = 667,
    LANG_BG_EY_HAS_TAKEN_H_M_TOWER      = 668,
    LANG_BG_EY_HAS_TAKEN_A_D_RUINS      = 669,
    LANG_BG_EY_HAS_TAKEN_H_D_RUINS      = 670,
    LANG_BG_EY_HAS_TAKEN_A_B_TOWER      = 671,
    LANG_BG_EY_HAS_TAKEN_H_B_TOWER      = 672,
    LANG_BG_EY_HAS_TAKEN_A_F_RUINS      = 673,
    LANG_BG_EY_HAS_TAKEN_H_F_RUINS      = 674,
    LANG_BG_EY_HAS_LOST_A_M_TOWER       = 675,
    LANG_BG_EY_HAS_LOST_H_M_TOWER       = 676,
    LANG_BG_EY_HAS_LOST_A_D_RUINS       = 677,
    LANG_BG_EY_HAS_LOST_H_D_RUINS       = 678,
    LANG_BG_EY_HAS_LOST_A_B_TOWER       = 679,
    LANG_BG_EY_HAS_LOST_H_B_TOWER       = 680,
    LANG_BG_EY_HAS_LOST_A_F_RUINS       = 681,
    LANG_BG_EY_HAS_LOST_H_F_RUINS       = 682,
    LANG_BG_EY_HAS_TAKEN_FLAG           = 683,
    LANG_BG_EY_CAPTURED_FLAG_A          = 684,
    LANG_BG_EY_CAPTURED_FLAG_H          = 685,
    LANG_BG_EY_DROPPED_FLAG             = 686,
    LANG_BG_EY_RESETED_FLAG             = 687,

    LANG_ARENA_ONE_TOOLOW               = 700,
    LANG_ARENA_ONE_MINUTE               = 701,
    LANG_ARENA_THIRTY_SECONDS           = 702,
    LANG_ARENA_FIFTEEN_SECONDS          = 703,
    LANG_ARENA_BEGUN                    = 704,

    LANG_WAIT_BEFORE_SPEAKING           = 705,
    LANG_NOT_EQUIPPED_ITEM              = 706,
    LANG_PLAYER_DND                     = 707,
    LANG_PLAYER_AFK                     = 708,
    LANG_PLAYER_DND_DEFAULT             = 709,
    LANG_PLAYER_AFK_DEFAULT             = 710,

    LANG_BG_QUEUE_ANNOUNCE_SELF         = 711,
    LANG_BG_QUEUE_ANNOUNCE_WORLD        = 712,


    LANG_YOUR_ARENA_LEVEL_REQ_ERROR     = 713,
//    LANG_HIS_ARENA_LEVEL_REQ_ERROR      = 714, an opcode exists for this
    LANG_YOUR_BG_LEVEL_REQ_ERROR        = 715,
//    LANG_YOUR_ARENA_TEAM_FULL           = 716, an opcode exists for this

    LANG_BG_AV_ALLY                     = 717,
    LANG_BG_AV_HORDE                    = 718,
    LANG_BG_AV_TOWER_TAKEN              = 719,
    LANG_BG_AV_TOWER_ASSAULTED          = 720,
    LANG_BG_AV_TOWER_DEFENDED           = 721,
    LANG_BG_AV_GRAVE_TAKEN              = 722,
    LANG_BG_AV_GRAVE_DEFENDED           = 723,
    LANG_BG_AV_GRAVE_ASSAULTED          = 724,

    LANG_BG_AV_MINE_TAKEN               = 725,
    LANG_BG_AV_MINE_NORTH               = 726,
    LANG_BG_AV_MINE_SOUTH               = 727,

    LANG_BG_AV_NODE_GRAVE_STORM_AID     = 728,
    LANG_BG_AV_NODE_TOWER_DUN_S         = 729,
    LANG_BG_AV_NODE_TOWER_DUN_N         = 730,
    LANG_BG_AV_NODE_GRAVE_STORMPIKE     = 731,
    LANG_BG_AV_NODE_TOWER_ICEWING       = 732,
    LANG_BG_AV_NODE_GRAVE_STONE         = 733,
    LANG_BG_AV_NODE_TOWER_STONE         = 734,
    LANG_BG_AV_NODE_GRAVE_SNOW          = 735,
    LANG_BG_AV_NODE_TOWER_ICE           = 736,
    LANG_BG_AV_NODE_GRAVE_ICE           = 737,
    LANG_BG_AV_NODE_TOWER_POINT         = 738,
    LANG_BG_AV_NODE_GRAVE_FROST         = 739,
    LANG_BG_AV_NODE_TOWER_FROST_E       = 740,
    LANG_BG_AV_NODE_TOWER_FROST_W       = 741,
    LANG_BG_AV_NODE_GRAVE_FROST_HUT     = 742,

    LANG_BG_AV_ONEMINTOSTART            = 743,
    LANG_BG_AV_HALFMINTOSTART           = 744,
    LANG_BG_AV_STARTED                  = 745,
    LANG_BG_AV_A_NEAR_LOSE              = 746,
    LANG_BG_AV_H_NEAR_LOSE              = 747,
    LANG_BG_AV_H_CAPTAIN_DEAD           = 748,
    LANG_BG_AV_A_CAPTAIN_DEAD           = 749,
    LANG_NPCINFO_LINKGUID               = 750,

    // Room for BG/ARENA                  751-769 not used

    LANG_ARENA_TESTING                  = 785,

    LANG_ANNOUNCE_COLOR                 = 787,
    LANG_GUILD_ANNOUNCE                 = 788,

    LANG_MOVEGENS_FOLLOW_PLAYER         = 789,
    LANG_MOVEGENS_FOLLOW_CREATURE       = 790,
    LANG_MOVEGENS_FOLLOW_NULL           = 791,
    LANG_MOVEGENS_EFFECT                = 792,
    LANG_HDEV_ANNOUNCE_COLOR            = 793,

    LANG_BG_GROUP_TOO_LARGE             = 1122, // "Your group is too large for this battleground. Please regroup to join."
    LANG_ARENA_GROUP_TOO_LARGE          = 1123, // "Your group is too large for this arena. Please regroup to join."
    LANG_ARENA_YOUR_TEAM_ONLY           = 1124, // "Your group has members not in your arena team. Please regroup to join."
    LANG_ARENA_NOT_ENOUGH_PLAYERS       = 1125, // "Your group does not have enough players to join this match."
    LANG_ARENA_GOLD_WINS                = 1126, // "The Gold Team wins!"
    LANG_ARENA_GREEN_WINS               = 1127, // "The Green Team wins!"
    LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING = 1128,   // The battleground will end soon, because there aren't enough players. Get more ppl or win already!
    LANG_BG_GROUP_OFFLINE_MEMBER        = 1129, // "Your group has an offline member. Please remove him before joining."
    LANG_BG_GROUP_MIXED_FACTION         = 1130, // "Your group has players from the opposing faction. You can't join the battleground as a group."
    LANG_BG_GROUP_MIXED_LEVELS          = 1131, // "Your group has players from different battleground brakets. You can't join as group."
    LANG_BG_GROUP_MEMBER_ALREADY_IN_QUEUE = 1132, // "Someone in your party is already in this battleground queue. (S)he must leave it before joining as group."
    LANG_BG_GROUP_MEMBER_DESERTER       = 1133, // "Someone in your party is Deserter. You can't join as group."
    LANG_BG_GROUP_MEMBER_NO_FREE_QUEUE_SLOTS = 1134, // "Someone in your party is already in three battleground queues. You cannot join as group."

    LANG_CANNOT_TELE_TO_BG              = 1135, // "You cannot teleport to a battleground or arena map."
    LANG_CANNOT_SUMMON_TO_BG            = 1136, // "You cannot summon players to a battleground or arena map."
    LANG_CANNOT_GO_TO_BG_GM             = 1137, // "You must be in GM mode to teleport to a player in a battleground."
    LANG_CANNOT_GO_TO_BG_FROM_BG        = 1138, // "You cannot teleport to a battleground from another battleground. Please leave the current battleground first."

    // in game strings
    LANG_PET_INVALID_NAME               = 800,
    LANG_NOT_ENOUGH_GOLD                = 801,
    LANG_NOT_FREE_TRADE_SLOTS           = 802,
    LANG_NOT_PARTNER_FREE_TRADE_SLOTS   = 803,
    LANG_YOU_NOT_HAVE_PERMISSION        = 804,
    LANG_UNKNOWN_LANGUAGE               = 805,
    LANG_NOT_LEARNED_LANGUAGE           = 806,
    LANG_NEED_CHARACTER_NAME            = 807,
    LANG_PLAYER_NOT_EXIST_OR_OFFLINE    = 808,
    LANG_ACCOUNT_FOR_PLAYER_NOT_FOUND   = 809,

    LANG_FREE_RESPEC_NOT_ENABLED        = 816,
    LANG_FREE_RESPEC_ALREADY_ENABLED    = 817,
    LANG_FREE_RESPEC_NOT_ENOUGH_MONEY   = 818,
    LANG_FREE_RESPEC_SUCCESFUL          = 819,
    // Room for in-game strings           820-999 not used

    // Level 4 (CLI only commands)
    LANG_COMMAND_EXIT                   = 1000,
    LANG_ACCOUNT_DELETED                = 1001,
    LANG_ACCOUNT_NOT_DELETED_SQL_ERROR  = 1002,
    LANG_ACCOUNT_NOT_DELETED            = 1003,
    LANG_ACCOUNT_CREATED                = 1004,
    LANG_ACCOUNT_TOO_LONG               = 1005,
    LANG_ACCOUNT_ALREADY_EXIST          = 1006,
    LANG_ACCOUNT_NOT_CREATED_SQL_ERROR  = 1007,
    LANG_ACCOUNT_NOT_CREATED            = 1008,
    LANG_CHARACTER_DELETED              = 1009,
    LANG_ACCOUNT_LIST_HEADER            = 1010,
    LANG_ACCOUNT_LIST_ERROR             = 1011,
    // Room for more level 4              1012-1099 not used

    // Level 3 (continue)
    LANG_ACCOUNT_SETADDON               = 1100,
    LANG_MOTD_NEW                       = 1101,
    LANG_SENDMESSAGE                    = 1102,
    LANG_EVENT_ENTRY_LIST_CONSOLE       = 1103,
    LANG_CREATURE_ENTRY_LIST_CONSOLE    = 1104,
    LANG_ITEM_LIST_CONSOLE              = 1105,
    LANG_ITEMSET_LIST_CONSOLE           = 1106,
    LANG_GO_ENTRY_LIST_CONSOLE          = 1107,
    LANG_QUEST_LIST_CONSOLE             = 1108,
    LANG_SKILL_LIST_CONSOLE             = 1109,
    LANG_CREATURE_LIST_CONSOLE          = 1110,
    LANG_GO_LIST_CONSOLE                = 1111,
    LANG_FILE_OPEN_FAIL                 = 1112,
    LANG_ACCOUNT_CHARACTER_LIST_FULL    = 1113,
    LANG_DUMP_BROKEN                    = 1114,
    LANG_INVALID_CHARACTER_NAME         = 1115,
    LANG_INVALID_CHARACTER_GUID         = 1116,
    LANG_CHARACTER_GUID_IN_USE          = 1117,
    LANG_ITEMLIST_GUILD                 = 1118,
    LANG_MUST_MALE_OR_FEMALE            = 1119,
    LANG_YOU_CHANGE_GENDER              = 1120,
    LANG_YOUR_GENDER_CHANGED            = 1121,

    // Debug commands
    LANG_CINEMATIC_NOT_EXIST            = 1200,
    // Room for more debug                1201-1299 not used

    LANG_COMMAND_FRIEND                 = 1700,
    LANG_COMMAND_FRIEND_ERROR           = 1701,

    // Ticket Strings 2000-2029
    LANG_COMMAND_TICKETNEW              = 2000,
    LANG_COMMAND_TICKETUPDATED          = 2001,
    LANG_COMMAND_TICKETPLAYERABANDON    = 2002,
    LANG_COMMAND_TICKETCLOSED           = 2003,
    LANG_COMMAND_TICKETDELETED          = 2004,
    LANG_COMMAND_TICKETNOTEXIST         = 2005,
    LANG_COMMAND_TICKETCLOSEFIRST       = 2006,
    LANG_COMMAND_TICKETALREADYASSIGNED  = 2007,
    LANG_COMMAND_TICKETRELOAD           = 2008,
    LANG_COMMAND_TICKETSHOWLIST         = 2009,
    LANG_COMMAND_TICKETSHOWONLINELIST   = 2010,
    LANG_COMMAND_TICKETSHOWCLOSEDLIST   = 2011,
    LANG_COMMAND_TICKETASSIGNERROR_A    = 2012,
    LANG_COMMAND_TICKETASSIGNERROR_B    = 2013,
    LANG_COMMAND_TICKETNOTASSIGNED      = 2014,
    LANG_COMMAND_TICKETUNASSIGNSECURITY = 2015,
    LANG_COMMAND_TICKETCANNOTCLOSE      = 2016,
    LANG_COMMAND_TICKETLISTGUID         = 2017,
    LANG_COMMAND_TICKETLISTNAME         = 2018,
    LANG_COMMAND_TICKETLISTAGE          = 2019,
    LANG_COMMAND_TICKETLISTASSIGNEDTO   = 2020,
    LANG_COMMAND_TICKETLISTUNASSIGNED   = 2021,
    LANG_COMMAND_TICKETLISTMESSAGE      = 2022,
    LANG_COMMAND_TICKETLISTCOMMENT      = 2023,
    LANG_COMMAND_TICKETLISTADDCOMMENT   = 2024,
    LANG_COMMAND_TICKETLISTAGECREATE    = 2025,
    LANG_COMMAND_TICKETLISTRESPONSE     = 2026,

    // Trinity strings             5000-9999
    LANG_COMMAND_FREEZE                 = 5000,
    LANG_COMMAND_FREEZE_ERROR           = 5001,
    LANG_COMMAND_FREEZE_WRONG           = 5002,
    LANG_COMMAND_UNFREEZE               = 5003,
    LANG_COMMAND_NO_FROZEN_PLAYERS      = 5004,
    LANG_COMMAND_LIST_FREEZE            = 5005,
    LANG_COMMAND_FROZEN_PLAYERS         = 5006,
    LANG_INSTANCE_MUST_RAID_GRP         = 5007,
    LANG_INSTANCE_NOT_AS_GHOST          = 5008,
    LANG_COMMAND_PLAYED_TO_ALL          = 5009,
    // Room for more Trinity strings      5010-9999
    // Used for GM Announcements
    LANG_GM_BROADCAST                    = 6613,
    LANG_GM_NOTIFY                       = 6614,
    LANG_GM_ANNOUNCE_COLOR               = 6615,

    // Use for not-in-offcial-sources patches
    //                                    10000-10999
    // opvp hp
    LANG_OPVP_HP_CAPTURE_OVERLOOK_H     = 10001,
    LANG_OPVP_HP_CAPTURE_OVERLOOK_A     = 10002,
    LANG_OPVP_HP_CAPTURE_STADIUM_H      = 10003,
    LANG_OPVP_HP_CAPTURE_STADIUM_A      = 10004,
    LANG_OPVP_HP_CAPTURE_BROKENHILL_H   = 10005,
    LANG_OPVP_HP_CAPTURE_BROKENHILL_A   = 10006,
    LANG_OPVP_HP_LOOSE_OVERLOOK_H       = 10007,
    LANG_OPVP_HP_LOOSE_OVERLOOK_A       = 10008,
    LANG_OPVP_HP_LOOSE_STADIUM_H        = 10009,
    LANG_OPVP_HP_LOOSE_STADIUM_A        = 10010,
    LANG_OPVP_HP_LOOSE_BROKENHILL_H     = 10011,
    LANG_OPVP_HP_LOOSE_BROKENHILL_A     = 10012,
    // opvp zm
    LANG_OPVP_ZM_CAPTURE_WEST_H         = 10013,
    LANG_OPVP_ZM_CAPTURE_WEST_A         = 10014,
    LANG_OPVP_ZM_CAPTURE_EAST_H         = 10015,
    LANG_OPVP_ZM_CAPTURE_EAST_A         = 10016,
    LANG_OPVP_ZM_CAPTURE_GY_H           = 10017,
    LANG_OPVP_ZM_CAPTURE_GY_A           = 10018,
    LANG_OPVP_ZM_LOOSE_WEST_H           = 10019,
    LANG_OPVP_ZM_LOOSE_WEST_A           = 10020,
    LANG_OPVP_ZM_LOOSE_EAST_H           = 10021,
    LANG_OPVP_ZM_LOOSE_EAST_A           = 10022,
    LANG_OPVP_ZM_LOOSE_GY_H             = 10023,
    LANG_OPVP_ZM_LOOSE_GY_A             = 10024,
    // opvp na
    LANG_OPVP_NA_CAPTURE_H              = 10025,
    LANG_OPVP_NA_CAPTURE_A              = 10026,
    LANG_OPVP_NA_LOOSE_H                = 10027,
    LANG_OPVP_NA_LOOSE_A                = 10028,
    // opvp tf
    LANG_OPVP_TF_CAPTURE_H              = 10029,
    LANG_OPVP_TF_CAPTURE_A              = 10030,
    LANG_OPVP_TF_LOOSE_H                = 10031,
    LANG_OPVP_TF_LOOSE_A                = 10032,
    // opvp ep
    LANG_OPVP_EP_CAPTURE_NORTHPASS_H    = 10033,
    LANG_OPVP_EP_CAPTURE_NORTHPASS_A    = 10034,
    LANG_OPVP_EP_CAPTURE_EASTWALL_H     = 10035,
    LANG_OPVP_EP_CAPTURE_EASTWALL_A     = 10036,
    LANG_OPVP_EP_CAPTURE_CROWNGUARD_H   = 10037,
    LANG_OPVP_EP_CAPTURE_CROWNGUARD_A   = 10038,
    LANG_OPVP_EP_CAPTURE_PLAGUEWOOD_H   = 10039,
    LANG_OPVP_EP_CAPTURE_PLAGUEWOOD_A   = 10040,
    LANG_OPVP_EP_LOOSE_NORTHPASS_H      = 10041,
    LANG_OPVP_EP_LOOSE_NORTHPASS_A      = 10042,
    LANG_OPVP_EP_LOOSE_EASTWALL_H       = 10043,
    LANG_OPVP_EP_LOOSE_EASTWALL_A       = 10044,
    LANG_OPVP_EP_LOOSE_CROWNGUARD_H     = 10045,
    LANG_OPVP_EP_LOOSE_CROWNGUARD_A     = 10046,
    LANG_OPVP_EP_LOOSE_PLAGUEWOOD_H     = 10047,
    LANG_OPVP_EP_LOOSE_PLAGUEWOOD_A     = 10048,
    // opvp si
    LANG_OPVP_SI_CAPTURE_H              = 10049,
    LANG_OPVP_SI_CAPTURE_A              = 10050,
    // opvp gossips
    LANG_OPVP_EP_FLIGHT_NORTHPASS       = 10051,
    LANG_OPVP_EP_FLIGHT_EASTWALL        = 10052,
    LANG_OPVP_EP_FLIGHT_CROWNGUARD      = 10053,
    LANG_OPVP_ZM_GOSSIP_ALLIANCE        = 10054,
    LANG_OPVP_ZM_GOSSIP_HORDE           = 10055,
    LANG_NO_ENTER_HALL_OF_LEGENDS       = 10056,
    LANG_NO_ENTER_CHAMPIONS_HALL        = 10057,

    LANG_BG_START_ANNOUNCE              = 11000,
    LANG_ANTICHEAT_SPEEDHACK            = 11001,
    LANG_SET_WEATHER                    = 11002,
    LANG_BANINFO_NOEMAIL                = 11003,
    LANG_BANINFO_EMAILENTRY             = 11004,
    LANG_BANLIST_NOEMAIL                = 11005,
    LANG_BANLIST_MATCHINGEMAIL          = 11006,
    LANG_CHANGE_DIFFICULTY_INSIDE       = 11007,
    LANG_CHANGE_DIFFICULTY_OFFLINE      = 11008,
    LANG_CHANGE_DIFFICULTY_RAID         = 11009,
    LANG_GM_TICKETS_TABLE_EMPTY         = 11010,
    LANG_ANTICHEAT_FLY                  = 11011,
    LANG_ROLLSHUTDOWN                   = 11012,
    LANG_ANTICHEAT_WATERWALK            = 11013,
    LANG_ANTICHEAT_NOFALLDMG            = 11014,
    LANG_GM_BANNED_PLAYER               = 11015,
    LANG_POSSIBLE_CHEAT                 = 11016,
    LANG_INSTA_KILL_GUARDIAN            = 11017,

    LANG_INSTA_KILL_GUARDIAN_PET        = 11018,

    // DeathSide
    LANG_ARENA_HAS_BEGUN_WE_ARE_WATCHING_YOU = 11100, // When arena starts:
    LANG_ARENA_IS_CLOSED_TIMED          = 11103,
    LANG_ARENA_IS_CLOSED                = 11104,
    LANG_BG_START_REPORT_AFK            = 11122,
    LANG_BG_START_ANNOUNCE_LIMIT_INFO   = 11123, // old 11016

    LANG_GM_INGAME_BUSY                 = 11124, //" (busy)"
    LANG_TICKET_CREATE_NO_GM            = 11125,
    LANG_TICKET_CREATE_GM_FREE          = 11126,
    LANG_TICKET_CREATE_GM_BUSY          = 11127,
    LANG_TICKET_ASSIGNED                = 11128,

    LANG_SCRIPT_YOU_ARE_IN_COMBAT       = 12000, // You are in combat! // Вы находитесь в бою!);
    LANG_SCRIPT_NEXT                    = 12001, // Next -> // Дальше ->);
    LANG_SCRIPT_BACK                    = 12002, // <- Back // <- Назад);
    LANG_SCRIPT_MAIN_MENU               = 12003, // <- Main Menu // <- Главное Меню);

    LANG_SCRIPT_LOCATION_DARNASSUS      = 12100, // Darnassus //Дарнасс); -- 1001);
    LANG_SCRIPT_LOCATION_EXODAR         = 12101, // Exodar //Экзодар); -- 1005);
    LANG_SCRIPT_LOCATION_IRONFORGE      = 12102, // Ironforge //Стальгорн); -- 1010);
    LANG_SCRIPT_LOCATION_STORMWIND      = 12103, // Stormwind //Штормград); -- 1015);
    LANG_SCRIPT_LOCATION_ORGRIMMAR      = 12104, // Orgrimmar //Оргриммар); -- 2001);
    LANG_SCRIPT_LOCATION_SILVERMOON     = 12105, // Silvermoon //Луносвет); -- 2005);
    LANG_SCRIPT_LOCATION_THUNDER_BLUFF  = 12106, // Thunder Bluff //Громовой Утёс); -- 2010);
    LANG_SCRIPT_LOCATION_UNDERCITY      = 12107, // Undercity //Подгород); -- 2015);
    LANG_SCRIPT_LOCATION_BOOTY_BAY      = 12108, // Booty Bay //Пиратская Бухта); -- 3005);
    LANG_SCRIPT_LOCATION_EVERLOOK       = 12109, // Everlook //Круговзор); -- 3015);
    LANG_SCRIPT_LOCATION_GADGETZAN      = 12110, // Gadgetzan //Прибамбасск); -- 3020);
    LANG_SCRIPT_LOCATION_MUDSPOCKET     = 12111, // Mudsprocket //Шестермуть); -- 3025);
    LANG_SCRIPT_LOCATION_RACTHET        = 12112, // Ratchet //Кабестан); -- 3030);
    LANG_SCRIPT_LOCATION_SHATTRATH      = 12113, // Shattrath //Шаттрат); -- 3035);
    LANG_SCRIPT_LOCATION_ISLE_QUEL_DANAS = 12114, // Isle of Quel'Danas //Остров Кель'Данас); -- 3040);
    LANG_SCRIPT_LOCATION_AERIS_LANDING  = 12115, // Aeris Landing //Небесный лагерь); -- 3045);
    LANG_SCRIPT_LOCATION_THORIUM_POINT  = 12116, // Thorium Point //Тлеющее ущелье); -- 3195);
    LANG_SCRIPT_LOCATION_BRONZEBEARD_CAMP = 12117, // Bronzebeard Encampment //Лагерь Бронзоборода); -- 3065);
    LANG_SCRIPT_LOCATION_CENARION_HOLD  = 12118, // Cenarion Hold //Крепость Ценариона); -- 3070);
    LANG_SCRIPT_LOCATION_CENRION_REGUGE = 12119, // Cenarion Refuge //Ценарионский оплот); -- 3075);
    LANG_SCRIPT_LOCATION_COSMOWRENCH    = 12120, // Cosmowrench //Космоворот); -- 3080);
    LANG_SCRIPT_LOCATION_ALTAR_OF_SHATAR = 12121, // Altar of Sha'tar //Алтарь Ша'тара); -- 3055);
    LANG_SCRIPT_LOCATION_SOTS           = 12122, // Sanctum of the Stars //Святилище Звёзд); -- 3175);
    LANG_SCRIPT_LOCATION_EMERALD_SNACTUARY = 12123, // Emerald Sanctuary //Изумрудное святилище); -- 3085);
    LANG_SCRIPT_LOCATION_TIMBERMAW_HOLD = 12124, // Timbermaw Hold //Крепость Древобрюхов); -- 3200);
    LANG_SCRIPT_LOCATION_VALORS_REST    = 12125, // Valor's Rest //Погост Отважных); -- 3100);
    LANG_SCRIPT_LOCATION_HALAA          = 12126, // Halaa //Халаа); -- 3105);
    LANG_SCRIPT_LOCATION_HARBORAGE      = 12127, // The Harborage //Убежище); -- 3110);
    LANG_SCRIPT_LOCATION_WIZARD_ROW     = 12128, // Wizard Row //Путь Волшебника); -- 3115);
    LANG_SCRIPT_LOCATION_LIGHTS_HOPE_CHAPEL = 12129, // Light's Hope Chapel //Часовня Последней Надежды); -- 3120);
    LANG_SCRIPT_LOCATION_MARSHALS_REFUGE = 12130, // Marshal's Refuge //Укрытие Маршалла); -- 3125);
    LANG_SCRIPT_LOCATION_ECO_DOME_MIDREALM = 12131, // Eco-Dome Midrealm //Заповедник ''Срединные земли''); -- 3135);
    LANG_SCRIPT_LOCATION_MIRAGE_RACEWAY = 12132, // Mirage Raceway //Виражи на Миражах); -- 3140);
    LANG_SCRIPT_LOCATION_STORMSPIRE     = 12133, // The Stormspire //Штормовая Вершина); -- 3190);
    LANG_SCRIPT_LOCATION_NESINGWARE_EXPED = 12134, // Nesingwary's Expedition //Экспедиция Эрнестуэя); -- 3150);
    LANG_SCRIPT_LOCATION_NIGHTHAVEN     = 12135, // Nighthaven //Ночная Гавань); -- 3155);
    LANG_SCRIPT_LOCATION_OGRILA         = 12136, // Ogri'la //Огри'ла); -- 3160);
    LANG_SCRIPT_LOCATION_PROTECTORATE_W_P = 12137, // Protectorate Watch Post //Застава Стражей Протектората); -- 3165);
    LANG_SCRIPT_LOCATION_SPOREGGAR      = 12138, // Sporeggar //Спореггар); -- 3180);
    LANG_SCRIPT_LOCATION_STEAMWHEEDLE_PORT = 12139, // SteamWheedle Port //Порт Картеля); -- 3185);
    LANG_SCRIPT_KALIMDOR                = 12140, // Kalimdor -> //Калимдор ->); -- 5010);
    LANG_SCRIPT_EASTERN_KINGDOMS        = 12141, // Eastern Kingdoms ->//Восточные Королевства ->); -- 5015);
    LANG_SCRIPT_OUTLAND                 = 12142, // Outland -> //Запределье ->); -- 5025);
    LANG_SCRIPT_ALLIANCE_CITIES         = 12143, // Alliance Cities -> //Столицы Альянса ->); -- 1000);
    LANG_SCRIPT_HORDE_CITIES            = 12144, // Horde Cities ->//Столицы Орды ->); -- 2000);
    LANG_SCRIPT_NEUTRAL_TOWNS           = 12145, // Neutral Towns ->// Нейтральные Города ->); -- 3000);
    LANG_SCRIPT_DUNGEONS                = 12146, // Dungeons ->//Подземелья ->); -- 5000);
    LANG_SCRIPT_LOCATION_BLACKFATHOM_DEEPS = 12147, // Blackfathom Deeps //Непроглядная Пучина); -- 6001);
    LANG_SCRIPT_LOCATION_CAVERNS_OF_TIME = 12148, // Caverns of Time //Пещеры Времени); -- 6005);
    LANG_SCRIPT_LOCATION_DIRE_MAUL      = 12149, // Dire Maul //Забытый Город); -- 6010);
    LANG_SCRIPT_LOCATION_MARAUDON       = 12150, // Maraudon //Мародон); -- 6015);
    LANG_SCRIPT_LOCATION_ONYXIA_LAIR    = 12151, // Onyxia's Lair //Логово Ониксии); -- 6020);
    LANG_SCRIPT_LOCATION_RAGEFIRE_CHASM = 12152, // Ragefire Chasm //Огненная Пропасть); -- 6025);
    LANG_SCRIPT_LOCATION_RAZORFEN_DOWNS = 12153, // Razorfen Downs //Курганы Иглошкурых); -- 6030);
    LANG_SCRIPT_LOCATION_RAZORFEN_KRAUL = 12154, // Razorfen Kraul //Лабиринты Иглошкурых); -- 6035);
    LANG_SCRIPT_LOCATION_RUINS_AQ       = 12155, // Ruins of Ahn'Qiraj //Руины Ан'Киража); -- 6040);
    LANG_SCRIPT_LOCATION_TEMPLE_AQ      = 12156, // Temple of Ahn'Qiraj //Ан'Кираж); -- 6045);
    LANG_SCRIPT_LOCATION_WAILING_CAVERNS = 12157, // Wailing Caverns //Пещеры Стенаний); -- 6050);
    LANG_SCRIPT_LOCATION_ZULFARRAK      = 12158, // Zul'Farrak //Зул'Фаррак); -- 6055); 
    LANG_SCRIPT_LOCATION_BLACKROCK_DEPTHS = 12159, // Blackrock Depths //Глубины Черной горы); -- 7001);
    LANG_SCRIPT_LOCATION_BLACKROCK_SPIRE = 12160, // Blackrock Spire //Пик Черной горы); -- 7005);
    LANG_SCRIPT_LOCATION_BWL            = 12161, // Blackwing Lair //Логово Крыла Тьмы); -- 7010);
    LANG_SCRIPT_LOCATION_DEADMINES      = 12162, // The Deadmines //Мертвые копи); -- 7015);
    LANG_SCRIPT_LOCATION_GNOMEREGAN     = 12163, // Gnomeregan //Гномреган); -- 7020);
    LANG_SCRIPT_LOCATION_SUNWELL        = 12164, // Sunwell Plateau //Плато Солнечного Колодца); -- 7025);
    LANG_SCRIPT_LOCATION_KARAZHAN       = 12165, // Karazhan //Каражан); -- 7030);
    LANG_SCRIPT_LOCATION_MOLTEN_CORE    = 12166, // Molten Core //Огненные Недра); -- 7035);
    LANG_SCRIPT_LOCATION_SCARLET_MONASTERY = 12167, // Scarlet Crusade Monastery //Монастырь Алого ордена); -- 7040);
    LANG_SCRIPT_LOCATION_SCHOLOMANCE    = 12168, // Scholomance //Некроситет); -- 7045);
    LANG_SCRIPT_LOCATION_SHF_KEEP       = 12169, // Shadowfang Keep //Крепость Темного Клыка); -- 7050);
    LANG_SCRIPT_LOCATION_STRATHOLME     = 12170, // Stratholme //Стратхольм); -- 7055);
    LANG_SCRIPT_LOCATION_SUNKEN_TEMPLE  = 12171, // Sunken Temple //Затонувший храм); -- 7060);
    LANG_SCRIPT_LOCATION_STOCKADES      = 12172, // Stockades //Тюрьма); -- 7065);
    LANG_SCRIPT_LOCATION_ULDAMAN        = 12173, // Uldaman //Ульдаман); -- 7070);
    LANG_SCRIPT_LOCATION_ZULAMAN        = 12174, // Zul'Aman //Зул'Аман); -- 7075);
    LANG_SCRIPT_LOCATION_ZULGURUB       = 12175, // Zul'Gurub //Зул'Гуруб); -- 7080);
    LANG_SCRIPT_LOCATION_AUCHINDOUN     = 12176, // Auchindoun //Аукиндон); -- 8001);
    LANG_SCRIPT_LOCATION_BLACK_TEMPLE   = 12177, // Black Temple //Черный храм); -- 8005);
    LANG_SCRIPT_LOCATION_COIL_RESERVOIR = 12178, // Coilfang Reservoir //Резервуар Кривого Клыка); -- 8010);
    LANG_SCRIPT_LOCATION_GRUULS_LAIR    = 12179, // Gruul's Lair //Логово Груула); -- 8015);
    LANG_SCRIPT_LOCATION_HF_CITADEL     = 12180, // Hellfire Citadel //Цитадель Адского Пламени); -- 8020);
    LANG_SCRIPT_LOCATION_TEMPEST_KEEP   = 12181, // Tempest Keep //Крепость Бурь); -- 8025);
    LANG_SCRIPT_TELEPORT_BACK           = 12182, // Teleport Back ->// Телепортироваться Обратно ->);
    LANG_SCRIPT_LOCATION_GURUBASHI_ARENA = 12183, // Gurubashi Arena // Арена Гурубаши);
    LANG_SCRIPT_LOCATION_ARENA_RING     = 12184, // Arena ring // Ринг Арены);
    LANG_SCRIPT_LOCATION_RING_OF_BLOOD  = 12185, // Ring of Blood Arena // Арена в Круге Крови);
    LANG_SCRIPT_LOCATION_NAGRAND        = 12186, // Nagrand // Награнд);
    LANG_SCRIPT_LOCATION_TANARIS        = 12187, // Tanaris // Танарис);
    LANG_SCRIPT_LOCATION_DUEL_ZONES     = 12188, // Duel Zones // Дуэльные зоны);
    LANG_SCRIPT_LOCATION_PVP_ZONES      = 12189, // PvP Zones // PvP Зоны);
    LANG_SCRIPT_LOCATION_CRYPT_START_LOC = 12190, // Crypt (start location) // Склеп (старт. локация));
    LANG_SCRIPT_LOCATION_PET_PLACE      = 12191, // Pet Place // Питомник);
    LANG_SCRIPT_LOCATION_MAIN           = 12192, // Main // Основное);
    LANG_SCRIPT_LOCATION_INSTANCES      = 12193, // Instances // Инстансы);
    LANG_SCRIPT_LOCATION_ARENAS         = 12194, // Arenas // Арены);
    LANG_SCRIPT_LOCATION_NAXXRAMAS      = 12195, // Naxxramas// Наксрамас);
    LANG_SCRIPT_LOCATION_CORE_OF_HORROR = 12196, // Core of Horror (doesn't work) // Недра Ужаса (не работает));
    LANG_SCRIPT_LOCATION_TEMPLE_LOST_SOULS = 12197, // Temple of the Lost Souls (doesn't work) // Храм Потерянных Душ (не работает));
    LANG_SCRIPT_LOCATION_ARMORY         = 12198, // Armory // Арсенал);
    LANG_SCRIPT_LOCATION_MOON_MINE      = 12199, // Moon Mine // Лунная шахта);
    LANG_SCRIPT_LOCATION_HOLY_HOLT      = 12200, // Holy Holt // Святое Пристанище);

    LANG_SCRIPT_GEM_META                = 12201, // Meta gems// Мета сокеты); --, GOSSIP_SENDER_MAIN, 692020);
    LANG_SCRIPT_GEM_BLUE                = 12202, // Blue gems// Синие сокеты); --, GOSSIP_SENDER_MAIN, 692021);
    LANG_SCRIPT_GEM_RED                 = 12203, // Red gems// Красные сокеты); --, GOSSIP_SENDER_MAIN, 692022);
    LANG_SCRIPT_GEM_YELLOW              = 12204, // Yellow gems// Желтые сокеты); --, GOSSIP_SENDER_MAIN, 692023);
    LANG_SCRIPT_GEM_GREEN               = 12205, // Green gems// Зеленые сокеты); --, GOSSIP_SENDER_MAIN, 692024);
    LANG_SCRIPT_GEM_PURPLE              = 12206, // Purple gems// Пурпурные сокеты); --, GOSSIP_SENDER_MAIN, 692025);
    LANG_SCRIPT_GEM_ORANGE              = 12207, // Orange gems// Оранжевые сокеты); --, GOSSIP_SENDER_MAIN, 692026);

    LANG_SCRIPT_ARENA_START_LORD        = 12208, // Ruins of Lordaeron// Руины Лордерона);
    LANG_SCRIPT_ARENA_NAGRAND           = 12209, // Nagrand Arena// Арена Награнда);
    LANG_SCRIPT_ARENA_BLADE             = 12210, // Blade's Edge Arena// Арена Острогорья);

    LANG_SCRIPT_NO_ARENA_FIGHTS         = 12211, // There are no arena fights at the moment.//Сейчас на арене не происходит ни один бой.);
    LANG_SCRIPT_LEAVE_ARENA_QUEUE       = 12212, // You must leave arena queue to spectate arena fights.//Вы должны покинуть очередь на арену, чтобы наблюдать за боями.);
    LANG_SCRIPT_LEAVE_BG_QUEUE          = 12213, // You must leave one battleground queue to spectate arena fights.//Покиньте одну из очередей полей битв, чтобы наблюдать за боями.);
    LANG_SCRIPT_ARENA_SPECTATE_OFF      = 12214, // Arena spectation is forbidden.//Наблюдение за боями на аренах запрещено.);

    LANG_SCRIPT_REGISTRATION_ARENA      = 12215, // Arena//Арена);
    LANG_SCRIPT_ALTERAC_VALLEY          = 12216, // Alterac Valley//Альтеракская Долина);
    LANG_SCRIPT_EYE_OF_THE_STORM        = 12217, // Eye of the Storm//Око Бури);
    LANG_SCRIPT_ARATHI_BASIN            = 12218, // Arathi Basin//Низина Арати);
    LANG_SCRIPT_WARSONG_GULCH           = 12219, // Warsong Gulch//Ущелье Песни Войны);

    LANG_SCRIPT_ARENA_SPECTATING        = 12220, // Arena spectation//Наблюдение за боями на арене);
    LANG_SCRIPT_CREATE_REGISTER_ARENA_TEAM = 12221, // Creation/registration of arena teams//Создание/регистрация команд арены);
    LANG_SCRIPT_REGISTRATION_BG         = 12222, // Battleground/Arena registration//Регистрация на поля боя/арену);

    LANG_SCRIPT_ERROR                   = 12223, // An error %u occured, please, tell administration about it with the screenshot of this text.//Произошла ошибка %u, пожалуйста, сообщите об этом администрации вместе со скриншотом данного текста.);
    
    LANG_SCRIPT_ARENA_FIGHT_VERSUS      = 12224, // A fight on arena [%s] [%u vs %u] with team ratings [%s] and [%s]//Бой на арене [%s] [%u на %u] с рейтингами команд [%s] и [%s]);

    LANG_SCRIPT_LEAVE_1V1_QUEUE         = 12225, // Leave 1x1 queue //Покинуть очередь 1х1);
    LANG_SCRIPT_JOIN_1V1_QUEUE_SKIRM    = 12226, // Join 1x1 queue (skirmish)//Вступить в очередь 1х1 (стычка));
    LANG_SCRIPT_JOIN_1V1_QUEUE_RATING   = 12227, // Join 1v1 queue (rated)//Вступить в очередь 1х1 (рейтинговый бой));
    LANG_SCRIPT_CREATE_1V1_TEAM         = 12228, // Create new 1x1 team//Создать новую команду 1х1);
    LANG_SCRIPT_1V1_TEAM_STATS          = 12229, // Statistics of my 1x1 team//Статистика моей команды 1х1);
    LANG_SCRIPT_DELETE_1V1_TEAM         = 12230, // Delte 1x1 team//Удалить команду 1х1);
    LANG_SCRIPT_YOU_HAVE_LEFT_1V1_QUEUE = 12231, // You have left 1x1 queue//Вы покинули очередь 1х1);
    LANG_SCRIPT_YOUR_1V1_TEAM_DELETED   = 12232, // Your 1x1 team has been deleted//Ваша команда 1х1 удалена);
    LANG_SCRIPT_YOU_DONT_HAVE_1V1_TEAM  = 12233, // You don't have 1x1 team//У вас нет команды 1х1);
    LANG_SCRIPT_ARE_YOU_SURE            = 12234, // Are you sure?//Вы уверены?);

    LANG_SCRIPT_CODE_SELL_NPC_VENDOR    = 12235, //SELL //ПРОДАТЬ);
    LANG_SCRIPT_MSG_CODEBOX_SELL_NPC_VENDOR = 12236, //Вы действительно хотите обменять выбранный вами предмет на %u [Badge of Justice], что равно половине его изначальной стоимости? После совершения обмена вернуть предмет будет невозможно. Введите \"" %s "\" в следующем мини-окне, чтобы подтвердить действие. //Вы действительно хотите обменять выбранный вами предмет на %u [Badge of Justice], что равно половине его изначальной стоимости? После совершения обмена вернуть предмет будет невозможно. Введите \"" %s "\" в следующем мини-окне, чтобы подтвердить действие.);
    LANG_SCRIPT_NPC_VENDOR_TEXT         = 12237, //[%s]. Sell for %u [Badge of Justice] //[%s]. Продать за %u [Badge of Justice]);
    LANG_SCRIPT_INVENTORY_NO_SPACE      = 12238, //Not enough space in your inventory.//В инвентаре недостаточно места.);
    LANG_SCRIPT_NO_LEG_ITEM_EQUIPPED    = 12239, //You don't have any legendary items equipped.//На вас не надето ни одной легендарного предмета.);

    LANG_SCRIPT_YOU_ARE_NOT_HUNTER      = 12240,    //You are not a hunter!//Вы не охотник!);
    LANG_SCRIPT_ALREADY_HAVE_PET_OR_CHARM = 12241,  //You already have a pet or a charmed creature!//У вас уже есть питомец или зачарованное существо!);
    LANG_SCRIPT_PET_TRAINING            = 12242,    //Pet training ->//Обучение питомца ->);
    LANG_SCRIPT_PET_TAMING              = 12243,    //Pet tame ->//Приручение питомца ->);
    LANG_SCRIPT_PET_BEAR                = 12244,    //Bear//Медведь); --1188); //Grizzled Black Bear    |
    LANG_SCRIPT_PET_GORILLA             = 12245,    //Gorilla//Горилла); --2521); //Skymane Gorilla        |
    LANG_SCRIPT_PET_WOLF                = 12246,    //Wolf//Волк); --2753); //Barnabus               |
    LANG_SCRIPT_PET_TURTLE              = 12247,    //Turtle//Черепаха); --3653); //Kresh                  |
    LANG_SCRIPT_PET_BAT                 = 12248,    //Bat//Летучья мышь); --4425); //Blind Hunter           |
    LANG_SCRIPT_PET_SERPENT             = 12249,    //Serpent//Змей); --5762); //Deviate Moccasin       |
    LANG_SCRIPT_PET_HYENA               = 12250,    //Hyena//Гиена); --14228); //Giggler                |
    LANG_SCRIPT_PET_CROCOLISK           = 12251,    //Crocolisk//Кроколиск); --15043); //Zulian Crocolisk       |
    LANG_SCRIPT_PET_DRAGONHAWK          = 12252,    //Dragonhawk//Дракондор); --15650); //Crazed Dragonhawk      |
    LANG_SCRIPT_PET_RAVAGER             = 12253,    //Ravager//Опустошитель); --16933); //Razorfang Ravager      |
    LANG_SCRIPT_PET_VULTURE             = 12254,    //Vulture//Стервятник); --16973); //Bonestripper Vulture   |
    LANG_SCRIPT_PET_SPIDER              = 12255,    //Spider//Паук); --20998); //Ridgespine Horror      |
    LANG_SCRIPT_PET_L_SERPENT           = 12256,    //Lightning Serpent//Грозовой змей); --20749); //Scalewing Serpent      |
    LANG_SCRIPT_PET_RAPTOR              = 12257,    //Raptor//Раптор); --20728); //Bladespire Raptor      |
    LANG_SCRIPT_PET_OWL                 = 12258,    //Owl//Сова); --19055); //Windroc Matriarch      |
    LANG_SCRIPT_PET_SCORPID             = 12259,    //Scorpid//Скорпид); --22100); //Scorpid Bonecrawler    |
    LANG_SCRIPT_PET_BOAR                = 12260,    //Boar//Вепрь); --16117); //Plagued Swine          |
    LANG_SCRIPT_PET_WILD_CAT            = 12261,    //Wild Cat//Дикая кошка); --21723); //Blackwind Sabercat     |
    LANG_SCRIPT_PET_WARP_CHASER         = 12262,    //Warp Chaser//Прыгуана-ловец); --23219); //Blackwind Warp Chaser  |

    LANG_SCRIPT_ENCHANT_NAME_START_AP   = 12263, //Attack Power//Сила атаки);
    LANG_SCRIPT_ENCHANT_NAME_SPD        = 12264, //Spell Damage//Урон от закл.);
    LANG_SCRIPT_ENCHANT_NAME_HEALING    = 12265, //Spell Healing//Лечение от закл.);
    LANG_SCRIPT_ENCHANT_NAME_STRENGTH   = 12266, //Strength//Сила);
    LANG_SCRIPT_ENCHANT_NAME_AGILITY    = 12267, //Agility//Ловкость);
    LANG_SCRIPT_ENCHANT_NAME_INTELLECT  = 12268, //Intellect//Интеллект);
    LANG_SCRIPT_ENCHANT_NAME_SPIRIT     = 12269, //Spirit//Дух);
    LANG_SCRIPT_ENCHANT_NAME_STAMINA    = 12270, //Stamina//Выносливость);
    LANG_SCRIPT_ENCHANT_NAME_MANAREG    = 12271, //Mana per 5 sec.//Восп. маны);
    LANG_SCRIPT_ENCHANT_NAME_DEFENSE    = 12272, //Defense rating//Рейтинг защиты);

    LANG_SCRIPT_CODE_DESTROY            = 12273, //DESTROY//РАЗРУШИТЬ);
    LANG_SCRIPT_MSG_CODEBOX_REINFORCE   = 12274, //Вы действительно хотите использовать руны для улучшения предмета [%s]? Обратите внимание, что вероятность улучшения равна %u%%. Если вам не повезет, то все уровни усиления будут разрушены.//Вы действительно хотите использовать руны для улучшения предмета [%s]? Обратите внимание, что вероятность улучшения равна %u%%. Если вам не повезет, то все уровни усиления будут разрушены.);
    LANG_SCRIPT_MSG_CODEBOX_DESTROY     = 12275, //Вы действительно хотите разрушить рунные усиления на предмете [%s]? Введите \"" %s "\" в следующем мини-окне, чтобы подтвердить действие.//Вы действительно хотите разрушить рунные усиления на предмете [%s]? Введите \"" %s "\" в следующем мини-окне, чтобы подтвердить действие.);
    LANG_SCRIPT_NO_LEG_ARMOR_EQUIPPED   = 12277, //You don't have any legendary armor item equipped.//На вас не надето ни одного легендарного предмета брони.);
    LANG_SCRIPT_REINFORCEMENT_TABLE     = 12278, //Reinforcement table//Таблица усилений);
    LANG_SCRIPT_DO_REINFORCE            = 12279, //[%s] - Reinforce to %u level//[%s] - Усилить до %u уровня);
    LANG_SCRIPT_DESTROY_REINFORCE       = 12280, //Destroy rune reinforcements//Разрушить рунные усиления);
    LANG_SCRIPT_NOT_ENOUGH_RUNES        = 12281, //You don't have enough required runes.//У вас не хватает необходимых рун.);

    LANG_SCRIPT_EXCHANGE_MARKS_START    = 12282, //Exchange marks %s//Обменять марки %s);
    LANG_SCRIPT_ALTERAC_VALLEY_ENG      = 12283, // Alterac Valley//Alterac Valley);
    LANG_SCRIPT_EYE_OF_THE_STORM_ENG    = 12284, // Eye of the Storm//Eye of the Storm);
    LANG_SCRIPT_ARATHI_BASIN_ENG        = 12285, // Arathi Basin//Arathi Basin);
    LANG_SCRIPT_WARSONG_GULCH_ENG       = 12286, // Warsong Gulch//Warsong Gulch);
    LANG_SCRIPT_EXCHANGE_MARKS_FROM_5_FROM_5 = 12287, //Exchange %u %s marks to %u %s marks.//Обменять %u марок %s на %u марок %s.);
    LANG_SCRIPT_EXCHANGE_MARKS_FROM_5_FROM_2 = 12288, //Exchange %u %s marks to %u %s marks.//Обменять %u марок %s на %u марки %s.);
    LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_2 = 12289, //Exchange %u %s marks to %u %s marks.//Обменять %u марки %s на %u марки %s.);
    LANG_SCRIPT_EXCHANGE_MARKS_FROM_2_FROM_1 = 12290, //Exchange %u %s marks to 1 %s mark.//Обменять %u марки %s на 1 марку %s.);
    LANG_SCRIPT_EXCHANGE_MARKS_FROM_1_FROM_1 = 12291, //Exchange 1 %s mark to 1 %s mark.//Обменять 1 марку %s на 1 марку %s.);
    LANG_SCRIPT_NOT_ENOUGH_MARKS        = 12292, //You don't have enough marks.//У вас не хватает марок.);

    LANG_SCRIPT_SET_ARMOR               = 12293, //Set armor//Установить броню);
    LANG_SCRIPT_SET_RESIST_FIRE         = 12294, //Set fire resist//Установить сопр. к магии огня);
    LANG_SCRIPT_SET_RESIST_NATURE       = 12295, //Set nature resist//Установить сопр. к силам природы);
    LANG_SCRIPT_SET_RESIST_ICE          = 12296, //Set frost resist//Установить сопр. к магии льда);
    LANG_SCRIPT_SET_RESIST_SHADOW       = 12297, //Set shadow resist//Установить сопр. к темной магии);
    LANG_SCRIPT_SET_RESIST_ARCANE       = 12298, //Set arcane resist//Установить сопр. к тайной магии);
    LANG_SCRIPT_SET_CHANCE_DODGE        = 12299, //Set dodge chance//Установить шанс уклонения);
    LANG_SCRIPT_SET_CHANCE_PARRY        = 12300, //Set parry chance//Установить шанс парирования);
    LANG_SCRIPT_SET_CHANCE_BLOCK        = 12301, //Set block chance//Установить шанс блокировки);
    LANG_SCRIPT_SET_BLOCK_DAMAGE        = 12302, //Set block damage//Установить блокируемый урон);
    LANG_SCRIPT_SET_LEVEL               = 12303, //Set level//Установить уровень);
    LANG_SCRIPT_BATTLE_END_IF_PERSONAL  = 12304, //Statistics for a player %s. //Статистика для игрока %s. );
    LANG_SCRIPT_BATTLE_END_DD           = 12305, //%sDone %u damage (%u per sec.). Training fight time length %u seconds.//%sНанесено %u ед. урона (%u в сек.). Длительность тренировочного боя %u секунд.);
    LANG_SCRIPT_BATTLE_END_HEAL         = 12306, //%sHealed %u health (%u per sec.). Training fight time length %u seconds.//%sВылечено %u ед. здоровья (%u в сек.). Длительность тренировочного боя %u секунд.);
    LANG_SCRIPT_BATTLE_END_TANK         = 12307, //%sGenerated %u aggression (%u per sec.). Training fight time length %u seconds.//%sСгенерировано %u ед. агрессии (%u в сек.). Длительность тренировочного боя %u секунд.);
    LANG_SCRIPT_CHANGE_OPTIONS          = 12308, //Change options...//Изменить настройки...);
    LANG_SCRIPT_RESET_OPTIONS           = 12309, //Reset options//Сбросить установленные настройки);
    LANG_SCRIPT_SET_OFF_PERSONAL_FIGHT  = 12310, //Disable personal fight//Выключить личный бой);
    LANG_SCRIPT_SET_ON_PERSONAL_FIGHT   = 12311, //Enable personal fight//Включить личный бой);
    LANG_SCRIPT_TRAIN_AS_DD             = 12312, //Train as DD//Тренироваться как ДД);
    LANG_SCRIPT_TRAIN_AS_TANK           = 12313, //Train as tank//Тренироваться как танк);
    LANG_SCRIPT_TRAIN_AS_HEALER         = 12314, //Train as healer//Тренироваться как хиллер);
    LANG_SCRIPT_DEFAULT_OPTIONS         = 12315, //Resistances to magic and armor = 0. Dodge, parry, block chances = 5%%. Blocking damage = 35. Level 70. Personal fight Enabled.//Сопротивления к магиям и броня = 0. Шансы уклонения, парирования и блокировки = 5%%. Блокируемый урон = 35. Уровень 70. Включен личный бой.);
    LANG_SCRIPT_PERSONAL_FIGHT_NOW_OFF  = 12316, //Personal fight disabled//Личный бой выключен.);
    LANG_SCRIPT_PERSONAL_FIGHT_NOW_ON   = 12317, //Personal fight enabled//Личный бой включен.);
    LANG_SCRIPT_HAS_SET_PHYS_IMMUNITY   = 12318, //Immunity to physical types of attacks has been set.//Установлен иммунитет к физическим типам атак.);
    LANG_SCRIPT_HAS_SET_ARMOR           = 12319, //%u armor has been set.//Установлено %u брони.);
    LANG_SCRIPT_HAS_SET_LEVEL           = 12320, //%u level has been set.//Установлен %u уровень.);
    LANG_SCRIPT_HAS_SET_BLOCK_DAMAGE    = 12321, //%u block damage has been set.//Установлено %u блокируемого урона от атак.);

    LANG_SCRIPT_TRAINING_DUMMY_TEXT_1   = 12322, //Immunity has been set to //Установлен иммунитет к );
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_2   = 12323, //Has been set //Установлено );
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_3   = 12324, // resistance to // сопротивления к );
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_4   = 12325, //fire magic.//магии огня.);
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_5   = 12326, //nature magic.//силам природы.);
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_6   = 12327, //frost magic.//магии льда.);
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_7   = 12328, //shadow magic.//темной магии.);
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_8   = 12329, //arcane magic.//тайной магии.);
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_10  = 12331, //%% chance //%% шанс на );
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_11  = 12332, //to dodge an attack.//уклонение от атаки.);
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_12  = 12333, //to parry an attack.//парирование атаки.);
    LANG_SCRIPT_TRAINING_DUMMY_TEXT_13  = 12334, //to block an attack.//блокировку атаки.);
    
    LANG_SCRIPT_NO_ITEM                 = 12335, //No item//Нет предмета);
    LANG_SCRIPT_SLOT_HEAD_A_START       = 12336, //Slot: Head. Active: [%s]//Слот: Голова. Активно: [%s]);
    LANG_SCRIPT_SLOT_SHOULDERS_A        = 12337, //Slot: Shoulders. Active: [%s]//Слот: Плечи. Активно: [%s]);
    LANG_SCRIPT_SLOT_BODY_A             = 12338, //Slot: Body. Active: [%s]"//Слот: Тело. Активно: [%s]);
    LANG_SCRIPT_SLOT_CHEST_A            = 12339, //Slot: Chest. Active: [%s]//Слот: Грудь. Активно: [%s]);
    LANG_SCRIPT_SLOT_WAIST_A            = 12340, //Slot: Waist. Active: [%s]//Слот: Пояс. Активно: [%s]);
    LANG_SCRIPT_SLOT_LEGS_A             = 12341, //Slot: Legs. Active: [%s]//Слот: Ноги. Активно: [%s]);
    LANG_SCRIPT_SLOT_FEED_A             = 12342, //Slot: Feet. Active: [%s]//Слот: Ботинки. Активно: [%s]);
    LANG_SCRIPT_SLOT_WRISTS_A           = 12343, //Slot: Wrists. Active: [%s]//Слот: Наручи. Активно: [%s]);
    LANG_SCRIPT_SLOT_HANDS_A            = 12344, //Slot: Hands. Active: [%s]//Слот: Руки. Активно: [%s]);
    LANG_SCRIPT_SLOT_BACK_A             = 12345, //Slot: Back. Active: [%s]//Слот: Спина. Активно: [%s]);
    LANG_SCRIPT_SLOT_TABARD_A           = 12346, //Slot: Tabard. Active: [%s]//Слот: Накидка. Активно: [%s]);
    LANG_SCRIPT_TRANS_SET_ORIG_MODEL    = 12347, //Set original model: [%s]//Установить оригинальную модель: [%s]);
    LANG_SCRIPT_NO_ACTIVE_TRANS_IN_SLOT = 12348, //No active transmogrification in this slot//Нет активной трансмогрификации в этом слоте);
    LANG_SCRIPT_SLOT_HEAD_START         = 12349, //Slot: Head//Слот: Голова);
    LANG_SCRIPT_SLOT_SHOULDERS          = 12350, //Slot: Shoulders//Слот: Плечи);
    LANG_SCRIPT_SLOT_BODY               = 12351, //Slot: Body//Слот: Тело);
    LANG_SCRIPT_SLOT_CHEST              = 12352, //Slot: Chest//Слот: Грудь);
    LANG_SCRIPT_SLOT_WAIST              = 12353, //Slot: Waist//Слот: Пояс);
    LANG_SCRIPT_SLOT_LEGS               = 12354, //Slot: Legs//Слот: Ноги);
    LANG_SCRIPT_SLOT_FEED               = 12355, //Slot: Feet//Слот: Ботинки);
    LANG_SCRIPT_SLOT_WRISTS             = 12356, //Slot: Wrists//Слот: Наручи);
    LANG_SCRIPT_SLOT_HANDS              = 12357, //Slot: Hands//Слот: Руки);
    LANG_SCRIPT_SLOT_BACK               = 12358, //Slot: Back//Слот: Спина);
    LANG_SCRIPT_SLOT_TABARD             = 12359, //Slot: Tabard//Слот: Накидка);
    LANG_SCRIPT_TRANS_WRONG_ITEM        = 12360, //This item doesn't meet requirements for your class!//Этот предмет не подходит по требованиям вашего класса!);
    LANG_SCRIPT_TRANS_ALREADY_HAVE      = 12361, //You already have this item in transmogrification list//У вас уже есть этот предмет в списке трансмогрификаций);
    LANG_SCRIPT_ADD_ITEM_TO_TRANS       = 12362, //Add item [%s] to transmogrification list//Добавить предмет [%s] в список трансмогрификаций);
    LANG_SCRIPT_NO_ITEM_IN_SLOT         = 12363, //You have no item in this slot!//У вас нет предмета в этом слоте!);
    LANG_SCRIPT_REACHED_MAX_TRANS       = 12364, //Can't add more transmogrifications! Maximum: 10! Delete one of them to add another!//Нельзя добавить больше трансмогрификаций! Максимум: 10! Удалите одну из других, чтобы добавить эту!);
    LANG_SCRIPT_DELETE_TRANS            = 12365, //Delete active transmogrification: [%s]//Удалить активную трансмогрификацию: [%s]);
    LANG_SCRIPT_TRANS_ADDED             = 12366, //Transmogrification added//Трансмогрификация добавлена);

    LANG_SCRIPT_TRANS_CODE_SURE         = 12367, //SURE//УВЕРЕН);
    LANG_SCRIPT_TRANS_MSG_CODEBOX_DELETE = 12368, //Вы действительно траляляляляляля предмет [%s]? Введите \"" %s "\" в следующем мини-окне, чтобы подтвердить действие.//Вы действительно траляляляляляля предмет [%s]? Введите \"" %s "\" в следующем мини-окне, чтобы подтвердить действие.);
    LANG_SCRIPT_TRANS_MSG_CODEBOX_TRANSMOGRIFY = 12369, //Вы действительно хотите затрансмогить предмет [%s]?//Вы действительно хотите затрансмогить предмет [%s]?);
    
    LANG_SCRIPT_TRANS_GIVE_ITEM         = 12370, //Give me transmogrification item//Дай мне предмет-трансмогрификатор);
    LANG_SCRIPT_TRANS_ITEMS_SWITCHED    = 12371, //You have swapped items! You cannot trick me!//Вы сменили предмет! Вам меня не обмануть!

    LANG_SCRIPT_LUX_GADGETS             = 12732, // Gadgets // Безделушки
    LANG_SCRIPT_LUX_TRANSMOGRIFICATIONS = 12733, // Transmogrifications // Вещи для трансмогрификации
    LANG_SCRIPT_LUX_MOUNTS              = 12734, // Mounts // Ездовые животные
    LANG_SCRIPT_LUX_TABARDS             = 12735, // Tabards // Гербовые накидки
    LANG_SCRIPT_LUX_COMPANIONS          = 12736, // Companions // Спутники
    LANG_SCRIPT_TRAINER_RESET           = 12737,
    LANG_SCRIPT_TRAINER_BEASTMASTER     = 12738,
    LANG_SCRIPT_TRAINER_SURE            = 12739,
    // Enchanter NPC
    LANG_CANT_USE_NOW = 12740, // You can not use this book while in combat or in Invisible/Stealth!
    LANG_SCRIPT_ENCH_2H_WEAPON          = 12741, // Enchant 2H Weapons
    LANG_SCRIPT_ENCH_1H_WEAPON          = 12742, // Enchant 1H Weapons
    LANG_SCRIPT_ENCH_RANG_WEAPON        = 12743, // Enchant Ranged Weapons
    LANG_SCRIPT_ENCH_CHEST              = 12744, // Enchant Chest
    LANG_SCRIPT_ENCH_FEET               = 12745, // Enchant Feet
    LANG_SCRIPT_ENCH_HANDS              = 12746, // Enchant Hands
    LANG_SCRIPT_ENCH_WRIST              = 12747, // Enchant Wrist
    LANG_SCRIPT_ENCH_SHIELD             = 12748, // Enchant Shield
    LANG_SCRIPT_ENCH_RINGS              = 12749, // Enchant Rings
    LANG_SCRIPT_ENCH_CLOAK              = 12750, // Enchant Cloak
    LANG_SCRIPT_ENCH_HEAD               = 12751, // Enchant Head
    LANG_SCRIPT_ENCH_LEGS               = 12752, // Enchant Legs
    LANG_SCRIPT_ENCH_SHOULDER           = 12753, // Enchant Shoulder
    LANG_SCRIPT_ENCH_ENCHANTED          = 12754, // Your %s has now been enchanted
    LANG_SCRIPT_ENCH_70_AP              = 12755, // +70 Attack Power
    LANG_SCRIPT_ENCH_35_AGILITY         = 12756, // +35 Agility
    LANG_SCRIPT_ENCH_MAIN_MENU          = 12757, // [Main Menu]
    // 1 Hand Weapon Page 1
    LANG_SCRIPT_ENCH_DEATHFROST         = 12758, // Deathfrost // Смертельный мороз
    LANG_SCRIPT_ENCH_EXECUTIONER        = 12759, // Executioner // Палач
    LANG_SCRIPT_ENCH_20_AGILITY         = 12760, // +20 Agility
    LANG_SCRIPT_ENCH_81_HB_27_SPD       = 12761, // +81 Healing and +27 SPD
    LANG_SCRIPT_ENCH_SOULFROST          = 12762, // Soulfrost // Ледяная душа
    LANG_SCRIPT_ENCH_SUNFIRE            = 12763, // Sunfire // Солнечный огонь
    LANG_SCRIPT_ENCH_MONGOOSE           = 12764, // Mongoose // Мангуст
    LANG_SCRIPT_ENCH_SPELLSURGE         = 12765, // Spellsurge // Всплеск чар
    LANG_SCRIPT_ENCH_BATTLEMASTER       = 12766, // Battlemaster // Военачальник
    LANG_SCRIPT_ENCH_40_SPD             = 12767, // +40 SPD
    LANG_SCRIPT_ENCH_PAGE_2             = 12768, // [Page 2 -->] // [Страница 2 -->]
    // 1 Hand Weapon Page 2
    LANG_SCRIPT_ENCH_20_STRENGHT        = 12769, // +20 Strenght // +20 Силы
    LANG_SCRIPT_ENCH_7_DMG              = 12770, // +7 DMG
    LANG_SCRIPT_ENCH_30_INTELLECT       = 12771, // +30 Intellect
    LANG_SCRIPT_ENCH_20_SPIRIT          = 12772, // +20 Spirit
    LANG_SCRIPT_ENCH_LIFESTEALING       = 12773, // Lifestealing // Похищение жизни
    LANG_SCRIPT_ENCH_CRUSADER           = 12774, // Crusader // Рыцарь
    LANG_SCRIPT_ENCH_FIERY_WEAPON       = 12775, // Fiery Weapon // Огненное оружие
    LANG_SCRIPT_ENCH_ADAM_WEP_CHAIN     = 12776, // Adamantite Weapon Chain // Адамантитовая оружейная цепь
    LANG_SCRIPT_ENCH_ELEM_WEP_STONE     = 12777, // Elemental Sharpening Stone // Точило стихий
    LANG_SCRIPT_ENCH_ADAM_WEP_STONE     = 12778, // Adamantite Sharpening Stone // Адамантитовое точило
    LANG_SCRIPT_ENCH_PAGE_1             = 12779, // [<-- Page 1] // [<-- Страница 1]
    // Chest
    LANG_SCRIPT_ENCH_6_ALL_STATS        = 12780, // +6 All stats
    LANG_SCRIPT_ENCH_150_HEALTH         = 12781, // +150 Health
    LANG_SCRIPT_ENCH_8_MANA_SEC         = 12782, // 8 Mana per 5s
    LANG_SCRIPT_ENCH_15_RESILIENCE      = 12783, // +15 Resilience
    LANG_SCRIPT_ENCH_15_DEFENCE         = 12784, // +15 Defence
    LANG_SCRIPT_ENCH_150_MANA           = 12785, // +150 Mana
    LANG_SCRIPT_ENCH_15_SPIRIT          = 12786, // +15 Spirit
    // Boots(Feet)
    LANG_SCRIPT_ENCH_5_SNARE_ROOT       = 12787, // +5% snare and root resist+10HR
    LANG_SCRIPT_ENCH_4_HP_MP_SEC        = 12788, // 4 HP&MP every 5s.
    LANG_SCRIPT_ENCH_12_AGILITY         = 12789, // +12 Agility
    LANG_SCRIPT_ENCH_12_STAMINA         = 12790, // +12 Stamina
    LANG_SCRIPT_ENCH_9_STAM_MIN_SP      = 12791, // +9 stamina and minor speed
    LANG_SCRIPT_ENCH_6_AGIL_MIN_SP      = 12792, // +6 Agility and minor speed
    // Gloves
    LANG_SCRIPT_ENCH_10_SPELL_CRIT      = 12793, // +10 Spell Crit
    LANG_SCRIPT_ENCH_26_ATTACK_POW      = 12794, // +26 Attack Power
    LANG_SCRIPT_ENCH_35_HB_12_SPD       = 12795, // +35 Healing and 12 SPD
    LANG_SCRIPT_ENCH_20_SPD             = 12796, // +20 SPD
    LANG_SCRIPT_ENCH_15_STRENGHT        = 12797, // +15 Strenght
    LANG_SCRIPT_ENCH_15_AGILITY         = 12798, // +15 Agility
    LANG_SCRIPT_ENCH_15_SPELL_HIT       = 12799, // +15 Spell Hit
    // Bracers Page 1
    LANG_SCRIPT_ENCH_24_AP              = 12800, // +24 Attack Power
    LANG_SCRIPT_ENCH_5_SPIRIT           = 12801, // +5 Spirit
    LANG_SCRIPT_ENCH_30_HB_10_SPD       = 12802, // +30 Healing and +10 SPD
    LANG_SCRIPT_ENCH_4_ALL_STATS        = 12803, // +4 All Stats
    LANG_SCRIPT_ENCH_12_INTELLECT       = 12804, // +12 Intellect
    LANG_SCRIPT_ENCH_6_MANA_SEC         = 12805, // +6 mana every 5 sec.
    LANG_SCRIPT_ENCH_12_DEFENCE         = 12806, // +12 Defence rating
    LANG_SCRIPT_ENCH_12_STRENGHT        = 12807, // +12 Strenght
    // Bracers Page 2
    LANG_SCRIPT_ENCH_15_SPD             = 12808, // +15 SPD
    LANG_SCRIPT_ENCH_12_STAMINA_BRAC    = 12809, // +12 Stamina
    // Shield
    LANG_SCRIPT_ENCH_DEFENCE            = 12810, // Defence
    LANG_SCRIPT_ENCH_12_INTELLECT_SHI   = 12811, // +12 Intellect
    LANG_SCRIPT_ENCH_RESISTANCE         = 12812, // Resistance
    LANG_SCRIPT_ENCH_15_BLOCK           = 12813, // +15 Block
    LANG_SCRIPT_ENCH_12_RESILIENCE      = 12814, // +12 Resilience
    LANG_SCRIPT_ENCH_18_STAMINA         = 12815, // +18 Stamina
    LANG_SCRIPT_ENCH_9_SPIRIT           = 12816, // +9 Spirit
    LANG_SCRIPT_ENCH_FROST_RESISTANCE   = 12817, // Frost Resistance
    LANG_SCRIPT_ENCH_LESSER_PROTECTION  = 12818, // Lesser Protection
    // Rings
    LANG_SCRIPT_ENCH_20_HB_7_SPD        = 12819, // +20 Healing and +7 SPD
    LANG_SCRIPT_ENCH_12_SPD             = 12820, // +12 SPD
    LANG_SCRIPT_ENCH_4_ALL_STATS_RING   = 12821, // +4 All stats
    LANG_SCRIPT_ENCH_STRIKING           = 12822, // Striking
    // Cloak
    LANG_SCRIPT_ENCH_STEALTH            = 12823, // Stealth
    LANG_SCRIPT_ENCH_12_DODGE           = 12824, // +12 Dodge rating
    LANG_SCRIPT_ENCH_12_DEFENCE_CLOAK   = 12825, // +12 Defence rating
    LANG_SCRIPT_ENCH_120_ARMOR          = 12826, // +120 Armor
    LANG_SCRIPT_ENCH_12_AGILITY_CLOAK   = 12827, // +12 Agility
    LANG_SCRIPT_ENCH_15_FIRE_RES        = 12828, // +15 Fire Resistance
    LANG_SCRIPT_ENCH_15_ARCANE_RES      = 12829, // +15 Arcane Resistance
    LANG_SCRIPT_ENCH_15_SHADOW_RES      = 12830, // +15 Shadow Resistance
    LANG_SCRIPT_ENCH_15_NATURE_RES      = 12831, // +15 Nature Resistance
    LANG_SCRIPT_ENCH_7_ALL_RES          = 12832, // +7 All Resistance
    LANG_SCRIPT_ENCH_20_SPELL_PENETR    = 12833, // +20 Spell Penetration
    // Head
    LANG_SCRIPT_ENCH_17_STR_16_INT      = 12834, // +17 Strenght and +16 Intellect
    LANG_SCRIPT_ENCH_34_AP_16_HIT       = 12835, // +34 Attack Power and +16 Hit Rating
    LANG_SCRIPT_ENCH_22_SPD_14_SP_HIT   = 12836, // +22 SPD and 14 spell hit
    LANG_SCRIPT_ENCH_35_HB_12_SPD_7M    = 12837, // +35 Healing, 12 SPD, 7 mana per 5s
    LANG_SCRIPT_ENCH_16_DEF_17_DODGE    = 12838, // +16 def. and 17 dodge
    LANG_SCRIPT_ENCH_18_STM_20_RES      = 12839, // +18 Stamina and 20 resilience
    LANG_SCRIPT_ENCH_10_STM_18_SPD      = 12840, // +10 Stam and +18 SPD
    // Legs
    LANG_SCRIPT_ENCH_35_SPD_20_STM      = 12841, // +35 SPD and 20 Stamina
    LANG_SCRIPT_ENCH_50_AP_12_CRIT      = 12842, // +50 Attack Power and 12 Crit
    LANG_SCRIPT_ENCH_66_HB_22_SPD_20STM = 12843, // +66 Healing, 22 SPD and 20 stamina
    LANG_SCRIPT_ENCH_10_STAMINA         = 12844, // +10 Stamina
    LANG_SCRIPT_ENCH_40_STM_12_AGIL     = 12845, // +40 Stamina and +12 Agility
    // Shoulder
    LANG_SCRIPT_ENCH_15_SP_CRIT_12_SPD  = 12846, // +15 Spell crit and 12 SPD
    LANG_SCRIPT_ENCH_6MP_22_HB          = 12847, // +6MP per 5s. and +22 healing
    LANG_SCRIPT_ENCH_15_CRIT_20_AP      = 12848, // +15 Crit and +20 Attack Power
    LANG_SCRIPT_ENCH_15_DEF_10_DODGE    = 12849, // +15 def. and +10 dodge
    LANG_SCRIPT_ENCH_26_AP_14_CRIT      = 12850, // +26 Attack Power and +14 Crit
    LANG_SCRIPT_ENCH_15_SPD_14_SP_CRIT  = 12851, // +15 SPD and +14 Spell crit
    LANG_SCRIPT_ENCH_31_HB_11_SPD_5MP   = 12852, // +31 Healing, +11 SPD and +5MP per 5s.
    LANG_SCRIPT_ENCH_16_STM_100_ARMOR   = 12853, // +16 Stamina and +100 armor
    LANG_SCRIPT_ENCH_15_DODGE_10_DEF    = 12854, // +15 Dodge and +10 defence
    LANG_SCRIPT_ENCH_33_HB_11_SPD_4MP   = 12855, // +33 Healing,11 SPD and 4MP per 5s.
    LANG_SCRIPT_ENCH_18_SPD_10_SP_CR    = 12856, // +18 SPD and 10 Spell crit
    LANG_SCRIPT_ENCH_30_AP_10_CRIT      = 12857, // +30 Attack power and 10 crit
    // Ranged
    LANG_SCRIPT_ENCH_30_HIT             = 12858, // +30 Hit Rating
    LANG_SCRIPT_ENCH_12_DMG             = 12859, // +12 DMG
    LANG_SCRIPT_ENCH_28_CRIT            = 12860, // +28 Crit Rating
    
    LANG_SCRIPT_ARENAS_BLIZZLIKE        = 12861, //Original arenas//Оригинальные арены
    LANG_SCRIPT_ARENA_CRE_REG_BLIZZLIKE = 12862, //Original arena team creation//Создание команд оригинальных арен

    LANG_SCRIPT_TYPE_AND_ACTIVE         = 12863, //%s: Active [%s]
    LANG_SCRIPT_TYPE_AND_POSSIBLE       = 12864, //%s: [%s]
    LANG_SCRIPT_TYPE_MAINHAND_0_START   = 12865, //Axe
    LANG_SCRIPT_TYPE_MAINHAND_1         = 12866, //Axe2H
    LANG_SCRIPT_TYPE_MAINHAND_2         = 12867, //Mace
    LANG_SCRIPT_TYPE_MAINHAND_3         = 12868, //Mace2H
    LANG_SCRIPT_TYPE_MAINHAND_4         = 12869, //Polearm
    LANG_SCRIPT_TYPE_MAINHAND_5         = 12870, //Sword
    LANG_SCRIPT_TYPE_MAINHAND_6         = 12871, //Sword2H
    LANG_SCRIPT_TYPE_MAINHAND_7         = 12872, //Staff
    LANG_SCRIPT_TYPE_MAINHAND_8         = 12873, //Fist
    LANG_SCRIPT_TYPE_MAINHAND_9         = 12874, //Dagger
    LANG_SCRIPT_TYPE_OFFHAND_0_START    = 12875, //Axe
    LANG_SCRIPT_TYPE_OFFHAND_1          = 12876, //Mace
    LANG_SCRIPT_TYPE_OFFHAND_2          = 12877, //Sword
    LANG_SCRIPT_TYPE_OFFHAND_3          = 12878, //Fist
    LANG_SCRIPT_TYPE_OFFHAND_4          = 12879, //Dagger
    LANG_SCRIPT_TYPE_OFFHAND_5          = 12880, //Shield
    LANG_SCRIPT_TYPE_OFFHAND_6          = 12881, //Holdable

    LANG_SCRIPT_MAINHAND                = 12885, //Mainhand
    LANG_SCRIPT_OFFHAND                 = 12886, //Offhand
    LANG_SCRIPT_RANGED                  = 12887, //Ranged

    LANG_SCRIPT_ARMOR                   = 12888, //Armor
    LANG_SCRIPT_WEAPON                  = 12889, //Weapon
    LANG_SCRIPT_SAVED_MODELS            = 12890, //Saved models:
    LANG_SCRIPT_SELECT_TRANS_MODEL      = 12891, //Select model
    LANG_SCRIPT_TRANS_SET_ORIG_MODEL_WEAPON = 12892, //Set original model
    LANG_SCRIPT_ACTIVATE_MODEL          = 12893, //Activate [%s]
    LANG_SCRIPT_NO_TRANS_REAGENT        = 12894, // You don not have any [name of the reagent]
    LANG_SCRIPT_NO_SAVED_MODELS         = 12895, //
    LANG_SCRIPT_TRANS_SELECTED          = 12896, //
    LANG_SCRIPT_TRANS_DELETED           = 12897, //
    LANG_SCRIPT_TRANS_BETA              = 12898, //
    LANG_SCRIPT_TRANS_ORIG_SETTED       = 12899, //
    LANG_SCRIPT_TRANS_THEREIS_ITEM      = 12900, //: [%s]
    LANG_SCRIPT_TRANS_INFO_MAIN_START   = 12901,
    LANG_SCRIPT_TRANS_INFO_1            = 12902,
    LANG_SCRIPT_TRANS_INFO_2            = 12903,
    LANG_SCRIPT_TRANS_INFO_3            = 12904,
    LANG_SCRIPT_TRANS_INFO_4            = 12905,
    LANG_SCRIPT_TRANS_INFO_5            = 12906,
    LANG_SCRIPT_TRANS_INFO_6            = 12907,

    LANG_SCRIPT_TYPE_RANGED_0_START     = 12908, //Thrown
    LANG_SCRIPT_TYPE_RANGED_1           = 12909, //Wand
    LANG_SCRIPT_TYPE_RANGED_2           = 12910, //Bow
    LANG_SCRIPT_TYPE_RANGED_3           = 12911, //Gun
    LANG_SCRIPT_TYPE_RANGED_4           = 12912, //Crossbow

    LANG_WARDEN_CHEAT_DETECTED          = 12913,
    
    LANG_SCRIPT_CLOTH                   = 12914,
    LANG_SCRIPT_LEATHER                 = 12915,
    LANG_SCRIPT_MAIL                    = 12916,
    LANG_SCRIPT_PLATE                   = 12917,

    LANG_SCRIPT_LUX_D1D2                = 12918,
    LANG_SCRIPT_LUX_T1T5                = 12919,
    LANG_SCRIPT_LUX_A1A3                = 12920,
    LANG_SCRIPT_LUX_OFFSET              = 12921,
    LANG_SCRIPT_LUX_WEAPON              = 12922,
    LANG_SCRIPT_LUX_SHIRT               = 12923,

    LANG_GM_FREE_ANNOUNCE               = 12924,

    LANG_SCRIPT_2015_EVENT_KOBOLD       = 12925,
    LANG_SCRIPT_2015_EVENT_SANTA        = 12926,
    LANG_SCRIPT_DISABLE_OFF_PVP_FLAG    = 12927,
    LANG_SCRIPT_LUX_D3                  = 12928,

    // Race Changer
    LANG_RACECHANGER_HUMAN              = 12929,
    LANG_RACECHANGER_NIGHT_ELF          = 12930,
    LANG_RACECHANGER_DWARF              = 12931,
    LANG_RACECHANGER_GNOME              = 12932,
    LANG_RACECHANGER_DRAENEI            = 12933,
    LANG_RACECHANGER_ORC                = 12934,
    LANG_RACECHANGER_UNDEAD             = 12935,
    LANG_RACECHANGER_TAUREN             = 12936,
    LANG_RACECHANGER_TROLL              = 12937,
    LANG_RACECHANGER_BLOOD_ELF          = 12938,
    LANG_RACECHANGER_CANT_SAME_RACE     = 12939,
    LANG_RACECHANGER_ALREADY_CHANGING   = 12940,
    LANG_RACECHANGER_LEAVE_TEAM         = 12941,

    LANG_SCRIPT_CLOTH_BG                = 12942,
    LANG_SCRIPT_LEATHER_BG              = 12943,
    LANG_SCRIPT_MAIL_BG                 = 12944,
    LANG_SCRIPT_PLATE_BG                = 12945,
    LANG_SCRIPT_CLOTH_ARENA             = 12946,
    LANG_SCRIPT_LEATHER_ARENA           = 12947,   
    LANG_SCRIPT_MAIL_ARENA              = 12948,
    LANG_SCRIPT_PLATE_ARENA             = 12949,
    LANG_SCRIPT_WEAPON_F                = 12950,
    LANG_SCRIPT_WEAPON_BG               = 12951,
    LANG_SCRIPT_WEAPON_ARENA            = 12952,   
    LANG_SCRIPT_ACCESSORIES             = 12953,
    LANG_SCRIPT_ACCESSORIES_BG          = 12954,   
    LANG_SCRIPT_ACCESSORIES_ARENA       = 12955,       
    LANG_SCRIPT_RELIC                   = 12956,
    LANG_SCRIPT_RELIC_ARENA             = 12957,
    LANG_SCRIPT_ENCH_1H_MAIN_HAND       = 12958,
    LANG_SCRIPT_ENCH_OFF_HAND           = 12959,
    LANG_SCRIPT_WELCOME_ENGLISH         = 12960,
    LANG_SCRIPT_WELCOME_RUSSIAN         = 12961,
    LANG_SCRIPT_TELE_START              = 12962,
    LANG_SCRIPT_TELE_HIDDEN             = 12963,

    LANG_YOU_VANISHED_SPELL             = 12964, // only for some time
    LANG_YOUR_SPELL_WAS_VANISHED        = 12965,
    LANG_YOUR_VANISH_WAS_BROKEN         = 12966,
    LANG_YOU_HAVE_BROKEN_VANISH         = 12967,
    LANG_I_WANT_DEVHINTS_MESSAGES       = 12968,
    LANG_NO_MORE_DEVHINTS_MESSAGES      = 12969,

    LANG_SCRIPT_NO_ARENA_TEAM           = 12970,
    LANG_SCRIPT_CHANGE_BADGE_TO_AP      = 12971,

    LANG_NOT_MORE_OFTEN_THAN_10_SEC     = 12972,
    LANG_REVIISON                       = 12973,
    LANG_AP_DISTRIBUTION_IN             = 12974,
    LANG_SERVER_WORKS                   = 12975,
    LANG_SERVERINFO_0                   = 12976,
    LANG_RESTART                        = 12977,
    LANG_SHUTDOWN                       = 12978,
    LANG_REASON                         = 12979,

    LANG_RACECHANGER_RACE               = 12980,
    LANG_RACECHANGER_GENDER             = 12981,
    LANG_RACECHANGER_NAME               = 12982,
    LANG_RACECHANGER_ALREADY_NAMECHANGED = 12983,
    LANG_RACECHANGER_DO_NOT_HAVE_ITEM_1 = 12984,
    LANG_RACECHANGER_DO_NOT_HAVE_ITEM_2 = 12985,
    LANG_RACECHANGER_DO_NOT_HAVE_ITEM_3 = 12986,

    LANG_SCRIPT_MSG_CODEBOX_RACE_CHANGE = 12987,
    LANG_SCRIPT_MSG_CODEBOX_GENDER_CHANGE = 12988,
    LANG_SCRIPT_MSG_CODEBOX_NAME_CHANGE = 12989,
    LANG_RACECHANGER_AFTER_RELOG        = 12990,

    LANG_SCRIPT_TELEPORT_DUEL_ZONE      = 12992,

    LANG_ANNOUNCE_BAN_GM                   = 12993,
    LANG_ANNOUNCE_BAN_EM                  = 12994,
    LANG_ANNOUNCE_MUTE_GM                  = 12995,
    LANG_ANNOUNCE_MUTE_EM                  = 12996,
    LANG_ANNOUNCE_TROLLMUTE_GM             = 12997,
    LANG_ANNOUNCE_TROLLMUTE_EM             = 12998,

    LANG_SCRIPT_SET_DEFAULT_XP             = 12999,
    LANG_SCRIPT_FREEZE_XP                  = 13000,
    LANG_SCRIPT_XP_SETTED_DEFAULT          = 13001,
    LANG_SCRIPT_XP_FROZEN                  = 13002,

    LANG_SHOP_ARE_YOU_SURE_SINGLE          = 13003,
    
    // 13005 - 15000 is reserverd for SHOP. Next will be 15001

    LANG_ERR_BANK_FUN                      = 15001,
    LANG_GM_LIKE                           = 15002,

    LANG_SCRIPT_LOOK_TICKET                = 15003,
    LANG_SCRIPT_PROBLEM_SOLVED             = 15004,
    LANG_SCRIPT_DELETE_ITEM                = 15005,

    LANG_COMMAND_COINS_INFO                = 15006,

    LANG_SHOWING_COINS                     = 15007,
    LANG_SHOWING_GOLD                      = 15008,

    LANG_SHOP_ARE_YOU_SURE_MANY            = 15009,
    LANG_SHOWING_COINS_BIG                 = 15010,

    LANG_MAIL_SUBJECT_ITEM_REASSIGN        = 15011,
    LANG_MAIL_TEXT_ITEM_TAKEN              = 15012,
    LANG_MAIL_TEXT_ITEM_GIVEN              = 15013,

    LANG_INFO_LANGUAGE_RUSSIAN             = 15014,
    LANG_INFO_LANGUAGE_BLIZZ               = 15015,

    LANG_RAID_RULES                        = 15016,
    LANG_RAID_RULES_NEW                    = 15017,
    LANG_RAID_RULES_NOT_SET                = 15018,
    LANG_RAID_RULES_UNSET                  = 15019,
    LANG_RAID_RULES_NOT_IN_PROGRESS        = 15020,
    LANG_RAID_RULES_BOUND                  = 15021,
    LANG_RAID_RULES_NEW_LEADER             = 15022,
    LANG_RAID_RULES_MEM_ON_CONVERT         = 15023,
    LANG_RAID_ASSIST_CANT_KICK_WAIT        = 15024,

    LANG_NY_2017_BOSS_SUMM_WARN_SPAWN      = 15025,
    LANG_NY_2017_BOSS_SUMM_WARN_FIRE       = 15026,

    LANG_GBK_RAID_MSG                      = 15027,
    LANG_GBK_PERSONAL_MSG                  = 15028,

    LANG_GBK_INSTANCE_RAID_GUILD           = 15029,
    LANG_GBK_INSTANCE_RAID_NOT_GUILD       = 15030,

    LANG_BG_GROUP_MEMBER_ARENA_RESTRICTED  = 15031, // "You or someone in your party has free talents left or has restricted items equipped. You cannot join with free talents left or with restricted items equipped."
    
    LANG_SCRIPT_ITEM_RESTRICTED            = 15032,
    LANG_SCRIPT_RESTRICTION_ALL_SWAPPED    = 15033,

    LANG_RESTRICTION_NO_ITEM_OR_NOT_RESTRICTED = 15034,
    LANG_RESTRICTION_NO_ANALOGS            = 15035,
    LANG_RESTRICTION_NO_ANALOGS_FOR_YOU    = 15036,

    LANG_SCRIPT_RESTRICTION_SWAP_ITEMS     = 15037,

    LANG_EVENT_GURUBASHI_NO_RAID           = 15038,

    LANG_SCRIPT_ARENA_JOIN_2V2_RATED       = 15039,
    LANG_SCRIPT_REGISTRATION_ARENA_1V1     = 15040,
    LANG_SCRIPT_ARENA_LADDER               = 15041, // Arena Ladder(Arena Top isn't good)
    LANG_SCRIPT_ARENA_LADDER_1             = 15042, // 1v1 Ladder
    LANG_SCRIPT_ARENA_LADDER_2             = 15043, // 2v2 Ladder
    LANG_SCRIPT_ARENA_LADDER_1_STATS       = 15044, // %u. %s(%s, %s) - W: %u / L: %u - Rate: %u
    LANG_SCRIPT_ARENA_LADDER_2_TEAM_STATS  = 15045, // %u. %s - W: %u / L: %u - Rate: %u
    LANG_SCRIPT_ARENA_LADDER_2_PLAYER      = 15046, // %s(%s, %s) - W: %u / L: %u - PR: %u

    LANG_THIS_BG_CLOSED                    = 15047,

    LANG_TARGET_GM_WHISPER_OFF             = 15048,

    LANG_FREE_PREMIUM_CODE_SYSMSG          = 15049,
    LANG_FREE_PREMIUM_CODE_MAIL_TITLE      = 15050,
    LANG_FREE_PREMIUM_CODE_MAIL_TEXT       = 15051,

    LANG_FIRST_LEVEL_70_CLASS_MAIL_TITLE   = 15052,
    LANG_FIRST_LEVEL_70_CLASS_MAIL_TEXT    = 15053,
    LANG_FIRST_LEVEL_70_CLASS_SYSMSG       = 15054,

    LANG_RECRUIT_REACHED_70_MAIL_TITLE     = 15055,
    LANG_RECRUIT_REACHED_70_MAIL_TEXT      = 15056,

    LANG_5_RECRUITS_REACHED_70_MAIL_TITLE  = 15057,
    LANG_5_RECRUITS_REACHED_70_MAIL_TEXT   = 15058,

    LANG_SCRIPT_MSG_CODEBOX_APPEAR_CHANGE  = 15059,
    LANG_RACECHANGER_APPEAR                = 15060,
LANG_RACECHANGER_DO_NOT_HAVE_ITEM_4 = 15061,

LANG_VK_GIFT_ALREADY_RECEIVED = 15062,
LANG_VK_GIFT_SENT = 15063,
LANG_VK_GIFT_NEED_SUBSCRIPTION = 15064,
LANG_XP_RATE_NEED_CONFIRM = 15065,
LANG_XP_HARDCORE_INFO = 15066,

LANG_RAF_LVL_10_MINIPET_SYSMSG = 15067,
LANG_RAF_LVL_10_MINIPET_MAIL_TITLE = 15068,
LANG_RAF_LVL_10_MINIPET_MAIL_TEXT = 15069,

LANG_XP_X0_SYSMSG = 15070,
LANG_XP_X1_SYSMSG = 15071,
LANG_XP_DEFAULT_SYSMSG = 15072,

LANG_HARDCORE_70_CLASS_MAIL_TITLE = 15073,
LANG_HARDCORE_70_CLASS_MAIL_TEXT = 15074,
LANG_HARDCORE_70_CLASS_SYSMSG = 15075,

LANG_PREMIUM_ITEM_ADDED_SYSMSG = 15076,

LANG_PREMIUM_UNSUMMON = 15077,

LANG_BETA_PART_GIFT_ALREADY_RECEIVED = 15078,
LANG_BETA_PART_GIFT_SENT = 15079,
LANG_BETA_PART_GIFT_NEED_PARTICIPATION = 15080,
//LANG_BETA_PART_GIFT_MAIL_TITLE         = 15081,
//LANG_BETA_PART_GIFT_MAIL_TEXT          = 15082,

LANG_REFERRAL_QUEST_TAKE_QUEST = 15083,
LANG_REFERRAL_QUEST_CHANGE_RECEIVER = 15084,

LANG_SCRIPT_VENDOR_TRADE = 15085,
LANG_SCRIPT_SHOW_BANK = 15086,
LANG_SCRIPT_AUCTION_ALLIED = 15087,
LANG_SCRIPT_AUCTION_NEUTRAL = 15088,

LANG_PREMIUM_VISIBILITY = 15089,
LANG_PREMIUM_VISIBILITY_OFF = 15090,

LANG_SCRIPT_SUMMON_MAILBOX = 15091,
LANG_SCRIPT_SUMMON_MAILBOX_COOLDOWN = 15092,
LANG_SCRIPT_PREMIUM_DUR_LEFT = 15093,
LANG_SCRIPT_PREMIUM_NOT_OWNER = 15094,

LANG_PREMIUM_NOT_ACTIVE = 15095,

LANG_DONATE_UNSUMMOM = 15096,

LANG_WAIT_BEFORE_NEW_WHISPER = 15097,
LANG_WAIT_BEFORE_LFG_MSG = 15098,
LANG_CHAT_TYPE_DISABLED_UNTIL_LVL = 15099,

LANG_YOU_OFFERED_GRANT_LEVEL = 15100,

LANG_WAIT_BEFORE_GLOBAL_MSG = 15101,

LANG_QUEST_4881_COMPLETE = 15102,

LANG_SPEND_COINS_X100 = 15103,

LANG_AMNESIA_SPEC_CHANGE = 15104,

LANG_EMAIL_CHANGE_PENDING = 15105,

LANG_VALUABLE_XP_BONUS_ALREADY_ACTIVE = 15106,

LANG_CHEST_ANTIQUES_ANNOUNCE = 15107,

// id - name
LANG_ITEM_LINK = 15108,
LANG_IP_ADDR_ALT_SWITCH_ON = 15109,
LANG_IP_ADDR_ALT_SWITCH_OFF = 15110,
LANG_IP_ADDR_ALT_CURR_ON = 15111,

LANG_BOX_BONUS_ALREADY_RECEIVED = 15112,
LANG_BOX_BONUS_SENT = 15113,
LANG_BOX_BONUS_NEED_PARTICIPATION = 15114,

LANG_SERVER_TIME = 15115,
LANG_BG_WINS_CURR = 15116,
LANG_SERVER_REV = 15117,
LANG_SCRIPT_REFRESH_LIST = 15118,

LANG_SQ_AS_GROUP = 15119,
LANG_SQ_HAVE_DESERTER = 15120,
LANG_SQ_HAS_FREE_TALENT_POINTS = 15121,
LANG_SQ_NEED_MAX_LVL = 15122,
LANG_ALREADY_IN_QUEUE = 15123,
LANG_SQ_FEW_PLAYED_TIME = 15124,
LANG_SQ_REG = 15125,
LANG_SQ_TEAM_WILL_BE_CREATED = 15126,
LANG_SQ_PARTICIPANT_INFO = 15127,
LANG_SQ_NEED_PVP_TRINKET = 15128,
LANG_SQ_IS_REPORTED = 15129,

LANG_NO_ARENA_TEAM = 15130,
LANG_YOU_NOT_IN_GROUP = 15131,

LANG_ARENA_EARLY_READY = 15132,
LANG_ARENA_EARLY_READY_SELF = 15133,

LANG_SCRIPT_ARENA_LADDER_SOLO_3 = 15134,

LANG_SQ_ITEMLEVEL = 15136,
LANG_SQ_TANK_ITEMS = 15137,
LANG_SQ_CANT_DELETE_TEAM = 15138,
LANG_SCRIPT_ARENA_JOIN_2V2_SKIRMISH = 15139,

LANG_ONLY_FOR_19_AND_70 = 15140,
LANG_ARENA_2V2_ANNOUNCE = 15141,
LANG_ARENA_SOLO_3V3_ANNOUNCE = 15142,
LANG_SQ_NOT_IN_ARENA = 15143,
LANG_SQ_NOT_TEAMMATE = 15144,
LANG_SQ_ALREADY_REPORTED = 15145,
LANG_SQ_REPORTED = 15146,
LANG_SQ_YOU_WAS_REPORTED = 15147,
//LANG_SQ_USE_REPORTS = 15148,
LANG_SQ_ENTER_NAME = 15149,
LANG_SQ_REPORT_SELF = 15150,

LANG_BGANN_ENABLED = 15170,
LANG_BGANN_DISABLED = 15171,

LANG_FS_AGREE = 15172,
LANG_NO = 15173,
LANG_FS_REQ = 15174,
LANG_CHEST_ETHEREAL_ANNOUNCE = 15175,

LANG_BATTLEGROUND_WILL_END = 15176,
LANG_ARENA_WILL_END = 15177,
LANG_BATTLEGROUND_PREMATURE_FINISH = 15178,
LANG_BATTLEGROUND_AFK_MARK = 15179,
LANG_HONOR_EXCHANGER = 15180,
LANG_HONOR_EXCHANGER_AP = 15181,
LANG_BG_IS_CLOSED = 15182,
LANG_BG_IS_CLOSED_TIMED = 15183,


APPEARANCENPC_HAIR_COLOR = 15185, // Я хочу изменить цвет волос.
APPEARANCENPC_SKIN_COLOR = 15186, // Я хочу изменить цвет своей кожи
APPEARANCENPC_FACE = 15187, // Я хочу изменить лицо
APPEARANCENPC_DONE = 15188, // Оставить так!
APPEARANCENPC_NEXT = 15189, // Дальше!
APPEARANCENPC_BACK = 15190, // Назад!
APPEARANCENPC_ACCESSORIES = 15191,
APPEARANCENPC_HAIR_STYLE = 15192, // Я хочу изменить причёску.
LANG_HONOR_EXCHANGER_MARKS = 15193,
LANG_HONOR_EXCHANGER_NO_MARKS = 15194,
LANG_HONOR_EXCHANGER_NO_KILLS = 15195,
LANG_HONOR_EXCHANGER_REQ_KILLS = 15196,

LANG_TRANSMOG_ITEM_NOT_FOUND = 15197,
LANG_TRANSMOG_MUST_BE_DEACTIVATED = 15198,
LANG_TRANSMOG_REMOVED = 15199,

LANG_NOT_ENOUGH_SHARDS = 15200,
LANG_NOT_ENOUGH_FRAGMENTS = 15201,
LANG_SCRIPT_RESET_TALENTS = 15202,
LANG_SCRIPT_RESET_TALENTS_CONFIRM = 15203,
LANG_MOUNT_VENDOR = 15204,
LANG_MOUNT_VENDOR_CONFIRM = 15205,
LANG_SCRIPT_NOT_ENOUGH_ITEMS = 15206,
LANG_CONDITIONS_NOT_MET = 15207,

LANG_RESERVED = 15208,
LANG_ALREADY_RESERVED = 15209,

LANG_SIMPLE_ERROR = 15210,

LANG_NO_TRANS_REAGENT1 = 15211,
LANG_NO_TRANS_REAGENT2 = 15212,
LANG_NO_TRANS_REAGENT3 = 15213,
LANG_NO_TRANS_REAGENT4 = 15214,

LANG_CHANNEL_LEVEL_RESTRICTED = 15215,

LANG_ARENA_WIN = 15216,
LANG_ARENA_LOSE = 15217,
LANG_OUTLAND_LEVEL_REQUIRED = 15218,

LANG_TOO_MANY_SAME_KILLS = 15219,

LANG_BG_PLAYERS_QUEUED = 15220,

LANG_BG_START_WS = 15221,
LANG_BG_START_AB = 15222,
LANG_BG_START_EY = 15223,
LANG_BG_START_AV = 15224,

LANG_GUILD_ANN_1 = 15225,
LANG_GUILD_ANN_2 = 15226,
LANG_GUILD_ANN_3 = 15227,
LANG_GUILD_ANN_4 = 15228,
LANG_GUILD_ANN_5 = 15229,
LANG_GUILD_ANN_6 = 15230,
LANG_GUILD_ANN_7 = 15231,

LANG_ARENA_FLUSH = 15232,

LANG_GUILD_ANN_8 = 15233,
LANG_GUILD_ANN_ENABLED = 15234,
LANG_GUILD_ANN_DISABLED = 15235,

LANG_MOTD_1 = 15236,
LANG_MOTD_2 = 15237,
LANG_MOTD_3 = 15238,
LANG_MOTD_4 = 15239,

LANG_RAIDS = 15240,
LANG_INSTANCES = 15241,

LANG_SPECTATOR_BLOCKED = 15247,

LANG_USE_DISCORD = 15248,
LANG_TICKETS_DISABLED = 15249,

LANG_BONUS_CODE_THROWN = 15250,
LANG_BONUS_CODE_ACTIVATED = 15251,
LANG_BONUS_CODE_ERROR = 15252,
LANG_BG_STATS_NOT_COUNTED = 15253,
LANG_ENCH_TOO_HIGH_LVL_ITEM = 15254,

LANG_SPEND_ALL_TAL = 15255,
LANG_SAME_SPEC = 15256,
LANG_SPEC_ACTIVE = 15257,
LANG_SPEC_ABOUT = 15258,
LANG_SPEC = 15259,
LANG_BONUS_RATES = 15260,

LANG_INSTANCE_MODIFIERS = 15261,

LANG_SWP_HEROIC_IS_AVAILABLE = 15262,
LANG_BT_HEROIC_IS_AVAILABLE = 15263,
LANG_HS_HEROIC_IS_AVAILABLE = 15264,

LANG_INSTANCE_MODIFIERS_REC = 15265,
LANG_BG_REAL_STATS_TEAM = 15266,
LANG_BG_REAL_STATS_ME = 15267,

LANG_ARENA_PLAYERS_QUEUED = 15268,

LANG_BG_STATS = 15269,
LANG_BG_STATS_INNER = 15270,
LANG_MOON_COIN = 15271,
LANG_NO_REWARD = 15272,
LANG_BG_STATS_INNER_EXT = 15273,

LANG_MARK_EXCHANGE = 15400,
LANG_HONOR_EXCHANGE = 15401,
LANG_AP_EXCHANGE = 15402,

LANG_SQ_GAMES_NEEDED = 15403,

LANG_REPORT_ARENA = 15404,

LANG_REPORTED = 15405,
LANG_LIKED = 15406,
LANG_RATE_SUCCESS = 15407,
LANG_RATE_TEAMMATE = 15408,
LANG_LIKE_TEAMMATE = 15409,
LANG_REPORT_TEAMMATE = 15410,
LANG_RATING_MODIFIED = 15411,
LANG_QUEUE_NOT_ENOUGH_RESILIENCE = 15412,
LANG_ARENA_IS_CLOSED_TODAY = 15413,

LANG_USE_LFG = 15414,

LANG_VENDOR_MAX_ITEMS = 15416,
LANG_JOIN_ENOUGH_RESILIENCE = 15417,
LANG_NOT_ENOUGH_RESILIENCE = 15418,
LANG_REMOVE_SLEEP = 15419,

//15+15 is reserved for welcome 
LANG_WELCOME_1 = 15420,
LANG_WELCOME_2 = 15421,
LANG_WELCOME_3 = 15422,
LANG_WELCOME_4 = 15423,
LANG_WELCOME_5 = 15424,
LANG_WELCOME_6 = 15425,
LANG_WELCOME_7 = 15426,
LANG_WELCOME_8 = 15427,
LANG_WELCOME_9 = 15428,
LANG_WELCOME_10 = 15429,
LANG_WELCOME_11 = 15430,
LANG_WELCOME_12 = 15431,
LANG_WELCOME_13 = 15432,
LANG_WELCOME_14 = 15433,
LANG_WELCOME_15 = 15434,
LANG_WELCOME_CLICK_1 = 15435,
LANG_WELCOME_CLICK_2 = 15436,
LANG_WELCOME_CLICK_3 = 15437,
LANG_WELCOME_CLICK_4 = 15438,
LANG_WELCOME_CLICK_5 = 15439,
LANG_WELCOME_CLICK_6 = 15440,
LANG_WELCOME_CLICK_7 = 15441,
LANG_WELCOME_CLICK_8 = 15442,
LANG_WELCOME_CLICK_9 = 15443,
LANG_WELCOME_CLICK_10 = 15444,
LANG_WELCOME_CLICK_11 = 15445,
LANG_WELCOME_CLICK_12 = 15446,
LANG_WELCOME_CLICK_13 = 15447,
LANG_WELCOME_CLICK_14 = 15448,
LANG_WELCOME_CLICK_15 = 15449,
//15+15 is reserved for welcome 

LANG_WELCOME_SELECT_RU = 15450,
LANG_WELCOME_SELECT_EN = 15451,
LANG_WELCOME_TELEPORT = 15452,
LANG_ALTERAC_ENABLED = 15453,
LANG_WAKE_UP_FIRST = 15454,
LANG_CRAFT_NEED_RELOG = 15455,
    
    // select entry from mangos3.hellground_string where entry between 12000 and 49999 order by entry desc limit 50;
    // INSERT INTO `hellground_string` (`entry`, `content_default`, `content_loc8`) VALUES ('15119', 'QWE', 'ЙЦУ');

    // NOT RESERVED IDS                   12000-1999999999
    // `db_script_string` table index     2000000000-2000009999 (MIN_DB_SCRIPT_STRING_ID-MAX_DB_SCRIPT_STRING_ID)
    // For other tables maybe             2000010000-2147483647 (max index)
};
#endif

