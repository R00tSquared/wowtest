/*
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2008-2015 Hellground <http://hellground.net/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#ifndef SC_GOSSIP_H
#define SC_GOSSIP_H

#include "Player.h"
#include "GossipDef.h"
#include "QuestDef.h"

// Gossip Item Text
#define GOSSIP_TEXT_BROWSE_GOODS        3
#define GOSSIP_TEXT_TRAIN               5

#define GOSSIP_TEXT_BANK                16
#define GOSSIP_TEXT_WINDRIDER           17
#define GOSSIP_TEXT_GRYPHON             17
#define GOSSIP_TEXT_BATHANDLER          17
#define GOSSIP_TEXT_HIPPOGRYPH          17
#define GOSSIP_TEXT_FLIGHTMASTER        17
#define GOSSIP_TEXT_AUCTIONHOUSE        21
#define GOSSIP_TEXT_GUILDMASTER         18
#define GOSSIP_TEXT_INN                 19
#define GOSSIP_TEXT_MAILBOX             20
#define GOSSIP_TEXT_STABLEMASTER        23
#define GOSSIP_TEXT_WEAPONMASTER        22
#define GOSSIP_TEXT_BATTLEMASTER        24
#define GOSSIP_TEXT_CLASSTRAINER        25
#define GOSSIP_TEXT_PROFTRAINER         26
#define GOSSIP_TEXT_OFFICERS            53

#define GOSSIP_TEXT_ALTERACVALLEY       54
#define GOSSIP_TEXT_ARATHIBASIN         55
#define GOSSIP_TEXT_WARSONGULCH         56
#define GOSSIP_TEXT_ARENA               57
#define GOSSIP_TEXT_EYEOFTHESTORM       58

#define GOSSIP_TEXT_DRUID               37
#define GOSSIP_TEXT_HUNTER              29
#define GOSSIP_TEXT_PRIEST              31
#define GOSSIP_TEXT_ROGUE               30
#define GOSSIP_TEXT_WARRIOR             27
#define GOSSIP_TEXT_PALADIN             28
#define GOSSIP_TEXT_SHAMAN              33
#define GOSSIP_TEXT_MAGE                34
#define GOSSIP_TEXT_WARLOCK             35

#define GOSSIP_TEXT_ALCHEMY             38
#define GOSSIP_TEXT_BLACKSMITHING       39
#define GOSSIP_TEXT_COOKING             40
#define GOSSIP_TEXT_ENCHANTING          41
#define GOSSIP_TEXT_ENGINEERING         59
#define GOSSIP_TEXT_FIRSTAID            42
#define GOSSIP_TEXT_HERBALISM           44
#define GOSSIP_TEXT_LEATHERWORKING      45
#define GOSSIP_TEXT_POISONS             60
#define GOSSIP_TEXT_TAILORING           48
#define GOSSIP_TEXT_MINING              46
#define GOSSIP_TEXT_FISHING             43
#define GOSSIP_TEXT_SKINNING            47
#define GOSSIP_TEXT_JEWELCRAFTING       61

#define GOSSIP_TEXT_IRONFORGE_BANK      62
#define GOSSIP_TEXT_STORMWIND_BANK      63
#define GOSSIP_TEXT_DEEPRUNTRAM         64
#define GOSSIP_TEXT_ZEPPLINMASTER       65
#define GOSSIP_TEXT_FERRY               66

// Skill defines

#define TRADESKILL_ALCHEMY                  1
#define TRADESKILL_BLACKSMITHING            2
#define TRADESKILL_COOKING                  3
#define TRADESKILL_ENCHANTING               4
#define TRADESKILL_ENGINEERING              5
#define TRADESKILL_FIRSTAID                 6
#define TRADESKILL_HERBALISM                7
#define TRADESKILL_LEATHERWORKING           8
#define TRADESKILL_POISONS                  9
#define TRADESKILL_TAILORING                10
#define TRADESKILL_MINING                   11
#define TRADESKILL_FISHING                  12
#define TRADESKILL_SKINNING                 13
#define TRADESKILL_JEWLCRAFTING             14

#define TRADESKILL_LEVEL_NONE               0
#define TRADESKILL_LEVEL_APPRENTICE         1
#define TRADESKILL_LEVEL_JOURNEYMAN         2
#define TRADESKILL_LEVEL_EXPERT             3
#define TRADESKILL_LEVEL_ARTISAN            4
#define TRADESKILL_LEVEL_MASTER             5

// Gossip defines

#define GOSSIP_ACTION_TRADE                 1
#define GOSSIP_ACTION_TRAIN                 2
#define GOSSIP_ACTION_TAXI                  3
#define GOSSIP_ACTION_GUILD                 4
#define GOSSIP_ACTION_BATTLE                5
#define GOSSIP_ACTION_BANK                  6
#define GOSSIP_ACTION_INN                   7
#define GOSSIP_ACTION_HEAL                  8
#define GOSSIP_ACTION_TABARD                9
#define GOSSIP_ACTION_AUCTION               10
#define GOSSIP_ACTION_INN_INFO              11
#define GOSSIP_ACTION_UNLEARN               12
#define GOSSIP_ACTION_INFO_DEF              1000
#define GOSSIP_ACTION_1					    1000000

#define GOSSIP_SENDER_MAIN                  1
#define GOSSIP_SENDER_INN_INFO              2
#define GOSSIP_SENDER_INFO                  3
#define GOSSIP_SENDER_SEC_PROFTRAIN         4
#define GOSSIP_SENDER_SEC_CLASSTRAIN        5
#define GOSSIP_SENDER_SEC_BATTLEINFO        6
#define GOSSIP_SENDER_SEC_BANK              7
#define GOSSIP_SENDER_SEC_INN               8
#define GOSSIP_SENDER_SEC_MAILBOX           9
#define GOSSIP_SENDER_SEC_STABLEMASTER      10

#define DEFAULT_GOSSIP_MESSAGE              1

extern uint32 GetSkillLevel(Player *player,uint32 skill);

// Defined fuctions to use with player.

// This fuction add's a menu item,
// a - Icon Id
// b - Text
// c - Sender(this is to identify the current Menu with this item)
// d - Action (identifys this Menu Item)
// e - Text to be displayed in pop up box
// f - Money value in pop up box
// max 15 gossips per window
#define ADD_GOSSIP_ITEM(a,b,c,d)   PlayerTalkClass->GetGossipMenu().AddMenuItem(a,b,c,d,"",0)
#define ADD_GOSSIP_ITEM_EXTENDED(a,b,c,d,e,f,g)   PlayerTalkClass->GetGossipMenu().AddMenuItem(a,b,c,d,e,f,g)

// This fuction Sends the current menu to show to client, a - NPCTEXTID(uint32) , b - npc guid(uint64)
#define SEND_GOSSIP_MENU(a,b)      PlayerTalkClass->SendGossipMenu(a,b)

// This fuction shows POI(point of interest) to client.
// a - position X
// b - position Y
// c - Icon Id
// d - Flags
// e - Data
// f - Location Name
#define SEND_POI(a,b,c,d,e,f)      PlayerTalkClass->SendPointOfInterest(a,b,c,d,e,f)

// Closes the Menu
#define CLOSE_GOSSIP_MENU()        PlayerTalkClass->CloseGossip()

// Fuction to tell to client the details
// a - quest object
// b - npc guid(uint64)
// c - Activate accept(bool)
#define SEND_QUEST_DETAILS(a,b,c)  PlayerTalkClass->SendQuestDetails(a,b,c)

// Fuction to tell to client the requested items to complete quest
// a - quest object
// b - npc guid(uint64)
// c - Iscompletable(bool)
// d - close at cancel(bool) - in case single incomplite ques
#define SEND_REQUESTEDITEMS(a,b,c,d) PlayerTalkClass->SendRequestedItems(a,b,c,d)

// Fuctions to send NPC lists, a - is always the npc guid(uint64)
#define SEND_VENDORLIST(a)         GetSession()->SendListInventory(a)
#define SEND_TRAINERLIST(a)        GetSession()->SendTrainerList(a)
#define SEND_BANKERLIST(a)         GetSession()->SendShowBank(a)
#define SEND_TABARDLIST(a)         GetSession()->SendTabardVendorActivate(a)
#define SEND_AUCTIONLIST(a)        GetSession()->SendAuctionHello(a)
#define SEND_TAXILIST(a)           GetSession()->SendTaxiStatus(a)

// Ressurect's the player if is dead.
#define SEND_SPRESURRECT()         GetSession()->SendSpiritResurrect()

// Get the player's honor rank.
#define GET_HONORRANK()            GetHonorRank()
// -----------------------------------

// defined fuctions to use with Creature

#define QUEST_DIALOG_STATUS(a,b,c)   GetSession()->getDialogStatus(a,b,c)
#endif

