// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2009 TrinityCore <http://www.trinitycore.org/>
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

#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "UpdateMask.h"
#include "AuctionHouseMgr.h"
#include "Mail.h"
#include "Util.h"
#include "Chat.h"

//please DO NOT use iterator++, because it is slower than ++iterator!!!
//post-incrementation is always slower than pre-incrementation !

//void called when player click on auctioneer npc
void WorldSession::HandleAuctionHelloOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;                                            //NPC guid
    recv_data >> guid;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid ,UNIT_NPC_FLAG_AUCTIONEER);
    if (!unit)
    {
        debug_log("WORLD: HandleAuctionHelloOpcode - %u not found or you can't interact with him.", GUID_LOPART(guid));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    SendAuctionHello(unit);
}

//this void causes that auction window is opened
void WorldSession::SendAuctionHello(Unit* unit)
{
    // always return pointer
    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntry(unit);

    WorldPacket data(MSG_AUCTION_HELLO, 12);
    data << unit->GetObjectGuid();
    data << uint32(ahEntry->houseId);
    SendPacket(&data);
}

//call this method when player bids, creates, or deletes auction
void WorldSession::SendAuctionCommandResult(AuctionEntry *auc, AuctionAction Action, AuctionError ErrorCode, InventoryResult invError)
{
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 16);
    data << uint32(auc ? auc->Id : 0);
    data << uint32(Action);
    data << uint32(ErrorCode);

    switch (ErrorCode)
    {
        case AUCTION_OK:
            if (Action == AUCTION_BID_PLACED)
                data << uint32(auc->GetAuctionOutBid());    // new AuctionOutBid?
            break;
        case AUCTION_ERR_INVENTORY:
            data << uint32(invError);
            break;
        case AUCTION_ERR_HIGHER_BID:
            data << ObjectGuid(HIGHGUID_PLAYER,auc->bidder);// new bidder guid
            data << uint32(auc->bid);                       // new bid
            data << uint32(auc->GetAuctionOutBid());        // new AuctionOutBid?
            break;
        default:
            break;
    }

    SendPacket(&data);
}

// this function sends notification, if bidder is online
void WorldSession::SendAuctionBidderNotification(AuctionEntry* auction, bool won)
{
    WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, (8*4));
    data << uint32(auction->GetHouseId());
    data << uint32(auction->Id);
    //if(auction->bidder)
        data << ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    /*else // Bot sets own guid to bidder, so it's never 0
        data << ObjectGuid(HIGHGUID_PLAYER, sAuctionBot.GetAHBplayerGUID());*/

    // if 0, client shows ERR_AUCTION_WON_S, else ERR_AUCTION_OUTBID_S
    data << uint32(won ? 0 : auction->bid);
    data << uint32(auction->GetAuctionOutBid());            // AuctionOutBid?
    data << uint32(auction->itemTemplate);
    data << int32(auction->itemRandomPropertyId);

    SendPacket(&data);
}

// this void causes on client to display: "Your auction sold"
void WorldSession::SendAuctionOwnerNotification(AuctionEntry* auction, bool sold)
{
    WorldPacket data(SMSG_AUCTION_OWNER_NOTIFICATION, (7*4));
    data << uint32(auction->Id);
    data << uint32(auction->bid);                           // if 0, client shows ERR_AUCTION_EXPIRED_S, else ERR_AUCTION_SOLD_S (works only when guid==0)
    data << uint32(auction->GetAuctionOutBid());            // AuctionOutBid?

    ObjectGuid bidder_guid = ObjectGuid();
    if (!sold)                                               // not sold yet
        bidder_guid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);

    // bidder==0 and moneyDeliveryTime==0 for expired auctions, and client shows error messages as described above
    // if bidder!=0 client updates auctions with new bid, outbid and bidderGuid
    data << bidder_guid;                                    // bidder guid
    data << uint32(auction->itemTemplate);                  // item entry
    data << uint32(auction->itemRandomPropertyId);

    SendPacket(&data);
}

// shows ERR_AUCTION_REMOVED_S
void WorldSession::SendAuctionRemovedNotification(AuctionEntry* auction)
{
    WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, (3*4));
    data << uint32(auction->Id);
    data << uint32(auction->itemTemplate);
    data << uint32(auction->itemRandomPropertyId);

    SendPacket(&data);
}

// this function sends mail to old bidder
void WorldSession::SendAuctionOutbiddedMail(AuctionEntry *auction)
{
    ObjectGuid oldBidder_guid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    Player *oldBidder = sObjectMgr.GetPlayerInWorld(oldBidder_guid);

    uint32 oldBidder_accId = 0;
    if(!oldBidder)
        oldBidder_accId = sObjectMgr.GetPlayerAccountIdByGUID(oldBidder_guid);

    // old bidder exist
    if (oldBidder || oldBidder_accId)
    {
        std::ostringstream msgAuctionOutbiddedSubject;
        msgAuctionOutbiddedSubject << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_OUTBIDDED;

        if (oldBidder)
            oldBidder->GetSession()->SendAuctionBidderNotification(auction, false);

        MailDraft(msgAuctionOutbiddedSubject.str())
            .SetMoney(auction->bid)
            .SendMailTo(MailReceiver(oldBidder, oldBidder_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
}

// this function sends mail, when auction is cancelled to old bidder
void WorldSession::SendAuctionCancelledToBidderMail(AuctionEntry* auction)
{
    ObjectGuid bidder_guid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    Player *bidder = sObjectMgr.GetPlayerInWorld(bidder_guid);

    uint32 bidder_accId = 0;
    if (!bidder)
        bidder_accId = sObjectMgr.GetPlayerAccountIdByGUID(bidder_guid);

    // bidder exist
    if (bidder || bidder_accId)
    {
        std::ostringstream msgAuctionCancelledSubject;
        msgAuctionCancelledSubject << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_CANCELLED_TO_BIDDER;

        if (bidder)
            bidder->GetSession()->SendAuctionRemovedNotification(auction);

        MailDraft(msgAuctionCancelledSubject.str())
            .SetMoney(auction->bid)
            .SendMailTo(MailReceiver(bidder, bidder_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
}


AuctionHouseEntry const* WorldSession::GetCheckedAuctionHouseForAuctioneer(ObjectGuid guid)
{
    Unit* auctioneer = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!auctioneer)
        return NULL;

    // always return pointer
    return AuctionHouseMgr::GetAuctionHouseEntry(auctioneer);
}

//this void creates new auction and adds auction to some auctionhouse
void WorldSession::HandleAuctionSellItem(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+8+4+4+4);

    ObjectGuid auctioneerGuid;
    ObjectGuid itemGuid;
    uint32 etime, bid, buyout;

    recv_data >> auctioneerGuid;
    recv_data >> itemGuid;
    recv_data >> bid;
    recv_data >> buyout;
    recv_data >> etime;
    
    Player *pl = GetPlayer();

    if (!itemGuid || !bid || !etime || bid >= 2000000000/*200k gold is client restricted*/ || buyout >= MAX_MONEY_AMOUNT)
        return;                                             //check for cheaters

	if (pl->IsPlayerCustomFlagged(PL_CUSTOM_MARKED_BOT))
	{
		ChatHandler(pl).PSendSysMessage(16579);
		return;
	}

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // client send time in minutes, convert to common used sec time
    etime *= MINUTE;

    // client understand only 3 auction time
    switch (etime)
    {
        case 1*MIN_AUCTION_TIME:
        case 2*MIN_AUCTION_TIME:
        case 4*MIN_AUCTION_TIME:
            break;
        default:
            return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    if (!itemGuid)
        return;

    Item *it = pl->GetItemByGuid(itemGuid);

    // do not allow to sell already auctioned items
    if (sAuctionMgr.GetAItem(itemGuid.GetCounter()))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: AuctionError, Player %s (guid: %u) is sending %s, but item is already in another auction", pl->GetName(), pl->GetGUIDLow(), itemGuid.GetString().c_str());
        SendAuctionCommandResult(NULL, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    // prevent sending bag with items (cheat: can be placed in bag after adding equipped empty bag to auction)
    if (!it)
    {
        SendAuctionCommandResult(NULL, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (!it->CanBeTraded())
    {
        SendAuctionCommandResult(NULL, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_CANNOT_TRADE_THAT);
        return;
    }

    if (it->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_CONJURED) || it->GetUInt32Value(ITEM_FIELD_DURATION))
    {
        SendAuctionCommandResult(NULL, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_CANNOT_TRADE_THAT);
        return;
    }

    if (it->IsBag() && !((Bag*)it)->IsEmpty())
    {
        SendAuctionCommandResult(0, AUCTION_STARTED, AUCTION_ERR_INVENTORY, EQUIP_ERR_CANNOT_TRADE_THAT);
        return;
    }

    // check money for deposit
    uint32 deposit = AuctionHouseMgr::GetAuctionDeposit(auctionHouseEntry, etime, it);
    if (pl->GetMoney() < deposit)
    {
        SendAuctionCommandResult(NULL, AUCTION_STARTED, AUCTION_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    if (HasPermissions(PERM_GMT) && sWorld.getConfig(CONFIG_GM_LOG_TRADE))
    {
        sLog.outCommand(GetAccountId(),"GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
            GetPlayerName(), GetAccountId(), it->GetProto()->Name1, it->GetEntry(), it->GetCount());
    }

    
    sLog.outLog(LOG_AUCTION, "Player %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
        GetPlayerName(),GetAccountId(),it->GetProto()->Name1,it->GetEntry(),it->GetCount());

    pl->ModifyMoney(-int32(deposit));

    AuctionEntry* AH = auctionHouse->AddAuction(auctionHouseEntry, it, etime, bid, buyout, deposit, pl);

    SendAuctionCommandResult(AH, AUCTION_STARTED, AUCTION_OK);
}

//this function is called when client bids or buys out auction
void WorldSession::HandleAuctionPlaceBid(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4+4);

    ObjectGuid auctioneerGuid;
    uint32 auctionId;
    uint32 price;
    recv_data >> auctioneerGuid;
    recv_data >> auctionId >> price;

    if (!auctionId || !price || price >= MAX_MONEY_AMOUNT)
        return;                                             // check for cheaters

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

	if (GetPlayer()->IsPlayerCustomFlagged(PL_CUSTOM_MARKED_BOT))
	{
		ChatHandler(GetPlayer()).PSendSysMessage(16579);
		return;
	}

    AuctionEntry *auction = auctionHouse->GetAuction(auctionId);
    Player *pl = GetPlayer();

    if (!auction || auction->owner == pl->GetGUIDLow())
    {
        // you cannot bid your own auction:
        SendAuctionCommandResult(NULL, AUCTION_BID_PLACED, AUCTION_ERR_BID_OWN);
        return;
    }

    ObjectGuid ownerGuid = ObjectGuid(HIGHGUID_PLAYER, auction->owner);

    // impossible have online own another character (use this for speedup check in case online owner)
    Player* auction_owner = sObjectMgr.GetPlayerInWorld(ownerGuid);
    if (!auction_owner && sObjectMgr.GetPlayerAccountIdByGUID(ownerGuid) == pl->GetSession()->GetAccountId())
    {
        // you cannot bid your another character auction:
        SendAuctionCommandResult(NULL, AUCTION_BID_PLACED, AUCTION_ERR_BID_OWN);
        return;
    }

    // cheating or client lags
    if (price <= auction->bid)
    {
        // client test but possible in result lags
        SendAuctionCommandResult(auction, AUCTION_BID_PLACED, AUCTION_ERR_HIGHER_BID);
        return;
    }

    // price too low for next bid if not buyout
    if ((price < auction->buyout || auction->buyout == 0) &&
        price < auction->bid + auction->GetAuctionOutBid())
    {
        // client test but possible in result lags
        SendAuctionCommandResult(auction, AUCTION_BID_PLACED, AUCTION_ERR_BID_INCREMENT);
        return;
    }

    if (price > pl->GetMoney())
    {
        // you don't have enough money!, client tests!
        // SendAuctionCommandResult(auction->auctionId, AUCTION_ERR_INVENTORY, EQUIP_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    // cheating
    if (price < auction->startbid)
        return;

    SendAuctionCommandResult(auction, AUCTION_BID_PLACED, AUCTION_OK);

    auction->UpdateBid(price, pl);
}

//this void is called when auction_owner cancels his auction
void WorldSession::HandleAuctionRemoveItem(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    ObjectGuid auctioneerGuid;
    uint32 auctionId;
    recv_data >> auctioneerGuid;
    recv_data >> auctionId;
    //debug_log("Cancel AUCTION AuctionID: %u", auctionId);

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    AuctionEntry *auction = auctionHouse->GetAuction(auctionId);
    Player *pl = GetPlayer();

    if (!auction || auction->owner != pl->GetGUIDLow())
    {
        SendAuctionCommandResult(NULL, AUCTION_REMOVED, AUCTION_ERR_DATABASE);
        sLog.outLog(LOG_DEFAULT, "ERROR: CHEATER : %u, he tried to cancel auction (id: %u) of another player, or auction is NULL", pl->GetGUIDLow(), auctionId);
        return;
    }

    Item *pItem = sAuctionMgr.GetAItem(auction->itemGuidLow);
    if (!pItem)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Auction id: %u has nonexistent item (item guid : %u)!!!", auction->Id, auction->itemGuidLow);
        SendAuctionCommandResult(NULL, AUCTION_REMOVED, AUCTION_ERR_INVENTORY, EQUIP_ERR_ITEM_NOT_FOUND);
        return;
    }

    if (auction->bid)                                       // If we have a bid, we have to send him the money he paid
    {
        uint32 auctionCut = auction->GetAuctionCut();
        if (pl->GetMoney() < auctionCut)                    // player doesn't have enough money, maybe message needed
            return;

        if (auction->bidder)                                // if auction have real existed bidder send mail
            SendAuctionCancelledToBidderMail(auction);

        pl->ModifyMoney(-int32(auctionCut));
    }

    // Return the item by mail
    std::ostringstream msgAuctionCanceledOwner;
    msgAuctionCanceledOwner << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_CANCELED;

    // item will deleted or added to received mail list
    MailDraft(msgAuctionCanceledOwner.str())
        .AddItem(pItem)
        .SendMailTo(pl, auction, MAIL_CHECK_MASK_COPIED);

    // inform player, that auction is removed
    SendAuctionCommandResult(auction, AUCTION_REMOVED, AUCTION_OK);
    // Now remove the auction
    RealmDataDatabase.BeginTransaction();
    auction->DeleteFromDB();
    pl->SaveInventoryAndGoldToDB();
    RealmDataDatabase.CommitTransaction();
    sAuctionMgr.RemoveAItem(auction->itemGuidLow);
    auctionHouse->RemoveAuction(auction);
    delete auction;
}

//called when player lists his bids
void WorldSession::HandleAuctionListBidderItems(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4+4);

    ObjectGuid auctioneerGuid;                              // NPC guid
    uint32 listfrom;                                        // page of auctions
    uint32 outbiddedCount;                                  // count of outbidded auctions

    recv_data >> auctioneerGuid;
    recv_data >> listfrom;                                  // not used in fact (this list not have page control in client)
    recv_data >> outbiddedCount;
    if (recv_data.size() != (16 + outbiddedCount * 4))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Client sent bad opcode!!! with count: %u and size : %u (must be: %u)", outbiddedCount, (uint32)recv_data.size(), (16 + outbiddedCount * 4));
        outbiddedCount = 0;
    }

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, (4+4+4));
    Player *pl = GetPlayer();
    data << uint32(0);                                      // add 0 as count
    uint32 count = 0;
    uint32 totalcount = 0;
    while (outbiddedCount > 0)                              // add all data, which client requires
    {
        --outbiddedCount;
        uint32 outbiddedAuctionId;
        recv_data >> outbiddedAuctionId;
        AuctionEntry *auction = auctionHouse->GetAuction(outbiddedAuctionId);
        if (auction && auction->BuildAuctionInfo(data))
        {
            ++totalcount;
            ++count;
        }
    }

    auctionHouse->BuildListBidderItems(data, pl, count, totalcount);
    data.put<uint32>(0, count);                             // add count to placeholder
    data << uint32(totalcount);
    data << uint32(300);                                    // unk 2.3.0 delay for next isFull request?
    SendPacket(&data);
}

//this void sends player info about his auctions
void WorldSession::HandleAuctionListOwnerItems(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    ObjectGuid auctioneerGuid;
    uint32 listfrom;

    recv_data >> auctioneerGuid;
    recv_data >> listfrom;                                  // not used in fact (this list not have page control in client)

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, (4+4+4));
    data << uint32(0);                                      // amount place holder

    uint32 count = 0;
    uint32 totalcount = 0;

    auctionHouse->BuildListOwnerItems(data, _player, count, totalcount);
    data.put<uint32>(0, count);
    data << uint32(totalcount);
    // Gensen: 300 items is a limit, idk how to fix it
    data << uint32(300);                                    // 2.3.0 delay for next isFull request?
    SendPacket(&data);
}

//this void is called when player clicks on search button
void WorldSession::HandleAuctionListItems(WorldPacket & recv_data)
{
    ObjectGuid auctioneerGuid;
    std::string searchedname;
    uint8 levelmin, levelmax, usable, isFull, sortCount;
    uint32 listfrom, auctionSlotID, auctionMainCategory, auctionSubCategory, quality;

    recv_data >> auctioneerGuid;
    recv_data >> listfrom;                                  // start, used for page control listing by 50 elements
    recv_data >> searchedname;

    recv_data >> levelmin >> levelmax;
    recv_data >> auctionSlotID >> auctionMainCategory >> auctionSubCategory >> quality;
    recv_data >> usable >> isFull >> sortCount;

    if (sortCount >= MAX_AUCTION_SORT)
        return;

    uint8 Sort[MAX_AUCTION_SORT];
    memset(Sort, MAX_AUCTION_SORT, MAX_AUCTION_SORT);

    // auction columns sorting
    for (uint32 i = 0; i < sortCount; ++i)
    {
        uint8 column, reversed;
        recv_data >> column;

        if (column >= MAX_AUCTION_SORT)
            return;

        recv_data >> reversed;
        Sort[i] = (reversed > 0) ? (column |= AUCTION_SORT_REVERSED) : column;
    }

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // Sort
    AuctionHouseObject::AuctionEntryMap const& aucs = auctionHouse->GetAuctions();
    std::vector<AuctionEntry*> auctions;

    bool sortEnabled = sWorld.getConfig(CONFIG_ENABLE_SORT_AUCTIONS);

    if (sortEnabled)
    {
        auctions.reserve(aucs.size());
        for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = aucs.begin(); itr != aucs.end(); ++itr)
            auctions.push_back(itr->second);

        AuctionSorter sorter(Sort, GetPlayer());
        std::sort(auctions.begin(), auctions.end(), sorter);
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    //debug_log("Auctionhouse search %s list from: %u, searchedname: %s, levelmin: %u, levelmax: %u, auctionSlotID: %u, auctionMainCategory: %u, auctionSubCategory: %u, quality: %u, usable: %u",
    //  auctioneerGuid.GetString().c_str(), listfrom, searchedname.c_str(), levelmin, levelmax, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, usable);

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, (4+4+4));
    uint32 count = 0;
    uint32 totalcount = 0;
    data << uint32(0);

    // converting string that we try to find to lower case
    std::wstring wsearchedname;
    if (!Utf8toWStr(searchedname, wsearchedname))
        return;

    wstrToLower(wsearchedname);

    if (sortEnabled)
    {
        BuildListAuctionItems(auctions, data, wsearchedname, listfrom, levelmin, levelmax, usable,
            auctionSlotID, auctionMainCategory, auctionSubCategory, quality, count, totalcount, isFull);
    }
    else
    {
        BuildListAuctionItems(aucs, data, wsearchedname, listfrom, levelmin, levelmax, usable,
            auctionSlotID, auctionMainCategory, auctionSubCategory, quality, count, totalcount, isFull);
    }

    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(300);                                    // 2.3.0 delay for next isFull request?
    SendPacket(&data);
}
