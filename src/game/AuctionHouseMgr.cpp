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

#include "AuctionHouseMgr.h"
#include "Database/DatabaseEnv.h"
#include "SQLStorage.h"
#include "DBCStores.h"
//#include "ProgressBar.h"

#include "AccountMgr.h"
#include "Item.h"
#include "Language.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Mail.h"

AuctionHouseMgr::AuctionHouseMgr()
{
}

AuctionHouseMgr::~AuctionHouseMgr()
{
    for(ItemMap::const_iterator itr = mAitems.begin(); itr != mAitems.end(); ++itr)
        delete itr->second;
}

AuctionHouseObject * AuctionHouseMgr::GetAuctionsMap(AuctionHouseEntry const* house)
{
    if(sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
        return &mAuctions[AUCTION_HOUSE_NEUTRAL];

    // team have linked auction houses
    switch(GetAuctionHouseTeam(house))
    {
        case ALLIANCE: return &mAuctions[AUCTION_HOUSE_ALLIANCE];
        case HORDE:    return &mAuctions[AUCTION_HOUSE_HORDE];
        default:       return &mAuctions[AUCTION_HOUSE_NEUTRAL];
    }
}

uint32 AuctionHouseMgr::GetAuctionDeposit(AuctionHouseEntry const* entry, uint32 time, Item *pItem)
{
    float deposit = float(pItem->GetProto()->SellPrice * pItem->GetCount() * (time / MIN_AUCTION_TIME));

    deposit = deposit * entry->depositPercent * 3.0f / 100.0f;

    float min_deposit = 0;

    if (deposit < min_deposit)
        deposit = min_deposit;

    return uint32(deposit * sWorld.getConfig(RATE_AUCTION_DEPOSIT));
}
// does not clear ram
void AuctionHouseMgr::SendAuctionWonMail(AuctionEntry *auction)
{
    Item *pItem = GetAItem(auction->itemGuidLow);
    if (!pItem)
        return;

    ObjectGuid bidderGuid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    Player *bidder = sObjectMgr.GetPlayerInWorld(bidderGuid);
    uint32 bidderAccId = 0;
    std::string bidderName;
    uint64 bidderPermissions = PERM_PLAYER;

    ObjectGuid ownerGuid = ObjectGuid(HIGHGUID_PLAYER, auction->owner);

    if (bidder)
    {
        bidderAccId = bidder->GetSession()->GetAccountId();
        bidderName = bidder->GetName();
        bidderPermissions = bidder->GetSession()->GetPermissions();
    }
    else
    {
        bidderAccId = sObjectMgr.GetPlayerAccountIdByGUID(bidderGuid);
        bidderPermissions = bidderAccId ? AccountMgr::GetPermissions(bidderAccId) : PERM_PLAYER;
        if (!sObjectMgr.GetPlayerNameByGUID(bidderGuid, bidderName))
            bidderName = sObjectMgr.GetHellgroundStringForDBCLocale(LANG_UNKNOWN);
    }

    std::string ownerName;
    if (!sObjectMgr.GetPlayerNameByGUID(ownerGuid, ownerName))
        ownerName = sObjectMgr.GetHellgroundStringForDBCLocale(LANG_UNKNOWN);

    uint32 ownerAccId = sObjectMgr.GetPlayerAccountIdByGUID(ownerGuid);

    sLog.outLog(LOG_AUCTION, "Player %s (Account: %u) won item in auction: %s (Entry: %u Count: %u) and pay money: %u. Original owner %s (Account: %u)",
        bidderName.c_str(), bidderAccId, pItem->GetProto()->Name1, pItem->GetEntry(), pItem->GetCount(), auction->bid, ownerName.c_str(), ownerAccId);

    // gm.log
    if (sWorld.getConfig(CONFIG_GM_LOG_TRADE))
    {
        if (bidderPermissions & PERM_GMT)
        {
            sLog.outCommand(bidderAccId,"GM %s (Account: %u) won item in auction (Entry: %u Count: %u) and pay money: %u. Original owner %s (Account: %u)",
                bidderName.c_str(), bidderAccId, auction->itemTemplate, auction->itemCount, auction->bid, ownerName.c_str(), ownerAccId);
        }
    }

    // receiver exist
    if (bidder || bidderAccId)
    {
        std::ostringstream msgAuctionWonSubject;
        msgAuctionWonSubject << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_WON;

        std::ostringstream msgAuctionWonBody;
        msgAuctionWonBody.width(16);
        msgAuctionWonBody << std::right << std::hex << auction->owner;
        msgAuctionWonBody << std::dec << ":" << auction->bid << ":" << auction->buyout;
        debug_log("AuctionWon body string : %s", msgAuctionWonBody.str().c_str());

        // set owner to bidder (to prevent delete item with sender char deleting)
        // owner in `data` will set at mail receive and item extracting
        RealmDataDatabase.PExecute("UPDATE item_instance SET owner_guid = '%u' WHERE guid='%u'", auction->bidder, auction->itemGuidLow);

        if (bidder)
            bidder->GetSession()->SendAuctionBidderNotification(auction, true);

        RemoveAItem(auction->itemGuidLow);                  // we have to remove the item, before we delete it !!
        auction->itemGuidLow = 0;                           // pending list will not use guid data

        // will delete item or place to receiver mail list
        MailDraft(msgAuctionWonSubject.str(), msgAuctionWonBody.str())
            .AddItem(pItem)
            .SendMailTo(MailReceiver(bidder, bidderGuid), auction, MAIL_CHECK_MASK_COPIED);
    }
    // receiver not exist
    else
    {
        RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid='%u'", auction->itemGuidLow);
        RemoveAItem(auction->itemGuidLow);                  // we have to remove the item, before we delete it !!
        auction->itemGuidLow = 0;
        delete pItem;
    }
}

void AuctionHouseMgr::SendAuctionSalePendingMail(AuctionEntry * auction)
{
    ObjectGuid owner_guid = ObjectGuid(HIGHGUID_PLAYER, auction->owner);
    Player *owner = sObjectMgr.GetPlayerInWorld(owner_guid);

    // owner exist (online or offline)
    if (owner || owner_guid && sObjectMgr.GetPlayerAccountIdByGUID(owner_guid))
    {
        std::ostringstream msgAuctionSalePendingSubject;
        msgAuctionSalePendingSubject << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_SALE_PENDING;

        std::ostringstream msgAuctionSalePendingBody;
        uint32 auctionCut = auction->GetAuctionCut();

        time_t distrTime = time(NULL) + sWorld.getConfig(CONFIG_MAIL_DELIVERY_DELAY);

        msgAuctionSalePendingBody.width(16);
        msgAuctionSalePendingBody << std::right << std::hex << auction->bidder;
        msgAuctionSalePendingBody << std::dec << ":" << auction->bid << ":" << auction->buyout;
        msgAuctionSalePendingBody << ":" << auction->deposit << ":" << auctionCut << ":0:";
        msgAuctionSalePendingBody << secsToTimeBitFields(distrTime);

        debug_log("AuctionSalePending body string : %s", msgAuctionSalePendingBody.str().c_str());

        MailDraft(msgAuctionSalePendingSubject.str(), msgAuctionSalePendingBody.str())
            .SendMailTo(MailReceiver(owner, owner_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
}

// call this method to send mail to auction owner, when auction is successful, it does not clear ram
void AuctionHouseMgr::SendAuctionSuccessfulMail(AuctionEntry * auction)
{
    ObjectGuid owner_guid = ObjectGuid(HIGHGUID_PLAYER, auction->owner);
    Player *owner = sObjectMgr.GetPlayerInWorld(owner_guid);

    uint32 owner_accId = 0;
    if (!owner)
        owner_accId = sObjectMgr.GetPlayerAccountIdByGUID(owner_guid);

    // owner exist
    if (owner || owner_accId)
    {
        std::ostringstream msgAuctionSuccessfulSubject;
        msgAuctionSuccessfulSubject << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_SUCCESSFUL;

        std::ostringstream auctionSuccessfulBody;
        uint32 auctionCut = auction->GetAuctionCut();

        auctionSuccessfulBody.width(16);
        auctionSuccessfulBody << std::right << std::hex << auction->bidder;
        auctionSuccessfulBody << std::dec << ":" << auction->bid << ":" << auction->buyout;
        auctionSuccessfulBody << ":" << auction->deposit << ":" << auctionCut;

        debug_log("AuctionSuccessful body string : %s", auctionSuccessfulBody.str().c_str());

        uint32 profit = auction->bid + auction->deposit - auctionCut;

        if (owner/* && owner->GetGUIDLow() != sAuctionBot.GetAHBplayerGUID()*/) // no need to check for it, can just send a packet to bot, it's better than checking each time
        {
            //send auction owner notification, bidder must be current!
            owner->GetSession()->SendAuctionOwnerNotification( auction, true );
        }

        MailDraft(msgAuctionSuccessfulSubject.str(), auctionSuccessfulBody.str())
            .SetMoney(profit)
            .SendMailTo(MailReceiver(owner, owner_guid), auction, MAIL_CHECK_MASK_COPIED, 0);
    }
}

// does not clear ram
void AuctionHouseMgr::SendAuctionExpiredMail(AuctionEntry * auction)
{                                                           // return an item in auction to its owner by mail
    Item *pItem = GetAItem(auction->itemGuidLow);
    if (!pItem)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Auction item (GUID: %u) not found, and lost.", auction->itemGuidLow);
        return;
    }

    ObjectGuid owner_guid = ObjectGuid(HIGHGUID_PLAYER, auction->owner);
    Player *owner = sObjectMgr.GetPlayerInWorld(owner_guid);

    uint32 owner_accId = 0;
    if (!owner)
        owner_accId = sObjectMgr.GetPlayerAccountIdByGUID(owner_guid);

    // owner exist
    if (owner || owner_accId)
    {
        std::ostringstream subject;
        subject << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_EXPIRED;

        if (owner /*&& owner->GetGUIDLow() != sAuctionBot.GetAHBplayerGUID()*/) // no need to check for it, can just send a packet to bot, it's better than checking each time
            owner->GetSession()->SendAuctionOwnerNotification(auction, false);

        RemoveAItem(auction->itemGuidLow);                  // we have to remove the item, before we delete it !!
        auction->itemGuidLow = 0;

        // will delete item or place to receiver mail list
        MailDraft(subject.str())
            .AddItem(pItem)
            .SendMailTo(MailReceiver(owner, owner_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
    // owner not found
    else
    {
        RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid='%u'", auction->itemGuidLow);
        RemoveAItem(auction->itemGuidLow);                  // we have to remove the item, before we delete it !!
        auction->itemGuidLow = 0;
        delete pItem;
    }
}

void AuctionHouseMgr::LoadAuctionItems()
{
    // data needs to be at first place for Item::LoadFromDB       0      1        2
    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT data,itemguid,item_template FROM auctionhouse JOIN item_instance ON itemguid = guid");

    if (!result)
    {
        //BarGoLink bar(1);
        //bar.step();
        sLog.outString();
        sLog.outString(">> Loaded 0 auction items");
        return;
    }

    //BarGoLink bar(result->GetRowCount());

    uint32 count = 0;

    Field *fields;
    do
    {
        //bar.step();

        fields = result->Fetch();
        uint32 item_guid        = fields[1].GetUInt32();
        uint32 item_template    = fields[2].GetUInt32();

        ItemPrototype const *proto = ObjectMgr::GetItemPrototype(item_template);

        if (!proto)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: AuctionHouseMgr::LoadAuctionItems: Unknown item (GUID: %u id: #%u) in auction, skipped.", item_guid,item_template);
            continue;
        }

        Item *item = NewItemOrBag(proto);

        if (!item->LoadFromDB(item_guid,0, result))
        {
            delete item;
            continue;
        }
        AddAItem(item);

        ++count;
    }
    while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u auction items", count);
}

void AuctionHouseMgr::LoadAuctions()
{
    QueryResultAutoPtr result = RealmDataDatabase.Query("SELECT COUNT(*) FROM auctionhouse");
    if (!result)
    {
        //BarGoLink bar(1);
        //bar.step();
        sLog.outString();
        sLog.outString(">> Loaded 0 auctions. DB table `auctionhouse` is empty.");
        return;
    }

    Field *fields = result->Fetch();
    uint32 AuctionCount=fields[0].GetUInt32();

    if (!AuctionCount)
    {
        //BarGoLink bar(1);
        //bar.step();
        sLog.outString();
        sLog.outString(">> Loaded 0 auctions. DB table `auction` is empty.");
        return;
    }

    result = RealmDataDatabase.Query("SELECT id,houseid,itemguid,item_template,item_count,item_randompropertyid,itemowner,buyoutprice,time,buyguid,lastbid,startbid,deposit FROM auctionhouse");
    if (!result)
    {
        //BarGoLink bar(1);
        //bar.step();
        sLog.outString();
        sLog.outString(">> Loaded 0 auctions. DB table `auctionhouse` is empty.");
        return;
    }

    //BarGoLink bar(AuctionCount);

    typedef std::map<uint32, std::wstring> PlayerNames;
    PlayerNames playerNames;                                // caching for load time

    do
    {
        fields = result->Fetch();

        //bar.step();

        AuctionEntry *auction = new AuctionEntry;
        auction->Id = fields[0].GetUInt32();
        uint32 houseid  = fields[1].GetUInt32();
        auction->itemGuidLow = fields[2].GetUInt32();
        auction->itemTemplate = fields[3].GetUInt32();
        auction->itemCount = fields[4].GetUInt32();
        auction->itemRandomPropertyId = fields[5].GetUInt32();

        auction->owner = fields[6].GetUInt32();

        if (auction->owner)
        {
            std::wstring& plWName = playerNames[auction->owner];
            if (plWName.empty())
            {
                std::string plName;
                if (!sObjectMgr.GetPlayerNameByGUID(ObjectGuid(HIGHGUID_PLAYER, auction->owner), plName))
                    plName = sObjectMgr.GetHellgroundStringForDBCLocale(LANG_UNKNOWN);

                Utf8toWStr(plName, plWName);
            }

            auction->ownerName = plWName;
        }

        auction->buyout = fields[7].GetUInt32();
        auction->expireTime = time_t(fields[8].GetUInt64());
        auction->bidder = fields[9].GetUInt32();
        auction->bid = fields[10].GetUInt32();
        auction->startbid = fields[11].GetUInt32();
        auction->deposit = fields[12].GetUInt32();
        auction->auctionHouseEntry = NULL;                  // init later

        // check if sold item exists for guid
        // and item_template in fact (GetAItem will fail if problematic in result check in AuctionHouseMgr::LoadAuctionItems)
        Item* pItem = GetAItem(auction->itemGuidLow);
        if (!pItem)
        {
            auction->DeleteFromDB();
            sLog.outLog(LOG_DEFAULT, "ERROR: Auction %u has not a existing item : %u, deleted", auction->Id, auction->itemGuidLow);
            delete auction;
            continue;
        }

        // overwrite by real item data
        if ((auction->itemTemplate != pItem->GetEntry()) ||
            (auction->itemCount != pItem->GetCount()) ||
            (auction->itemRandomPropertyId != pItem->GetItemRandomPropertyId()))
        {
            auction->itemTemplate = pItem->GetEntry();
            auction->itemCount    = pItem->GetCount();
            auction->itemRandomPropertyId = pItem->GetItemRandomPropertyId();

            //No SQL injection (no strings)
            RealmDataDatabase.PExecute("UPDATE auctionhouse SET item_template = %u, item_count = %u, item_randompropertyid = %i WHERE itemguid = %u",
                auction->itemTemplate, auction->itemCount, auction->itemRandomPropertyId, auction->itemGuidLow);
        }

        auction->auctionHouseEntry = GetAuctionHouseEntryByHouseId(houseid);

        if (!auction->auctionHouseEntry)
        {
            // need for send mail, use goblin auctionhouse
            auction->auctionHouseEntry = GetAuctionHouseEntryByHouseId(7);

            // Attempt send item back to owner
            std::ostringstream msgAuctionCanceledOwner;
            msgAuctionCanceledOwner << auction->itemTemplate << ":" << auction->itemRandomPropertyId << ":" << AUCTION_CANCELED;

            if (auction->itemGuidLow)
            {
                RemoveAItem(auction->itemGuidLow);
                auction->itemGuidLow = 0;

                // item will deleted or added to received mail list
                MailDraft(msgAuctionCanceledOwner.str(), "")    // TODO: fix body
                    .AddItem(pItem)
                    .SendMailTo(MailReceiver(ObjectGuid(HIGHGUID_PLAYER, auction->owner)), auction, MAIL_CHECK_MASK_COPIED);
            }

            auction->DeleteFromDB();
            delete auction;

            continue;
        }

        GetAuctionsMap(auction->auctionHouseEntry)->AddAuction(auction);

    } while (result->NextRow());

    sLog.outString();
    sLog.outString(">> Loaded %u auctions", AuctionCount);
}

void AuctionHouseMgr::AddAItem(Item* it)
{
    ASSERT(it);
    ASSERT(mAitems.find(it->GetGUIDLow()) == mAitems.end());
    mAitems[it->GetGUIDLow()] = it;
}

bool AuctionHouseMgr::RemoveAItem(uint32 id)
{
    ItemMap::iterator i = mAitems.find(id);
    if (i == mAitems.end())
    {
        return false;
    }
    mAitems.erase(i);
    return true;
}

void AuctionHouseMgr::Update()
{
    for (int i = 0; i < MAX_AUCTION_HOUSE_TYPE; ++i)
        mAuctions[i].Update();
}

uint32 AuctionHouseMgr::GetAuctionHouseTeam(AuctionHouseEntry const* house)
{
    // auction houses have faction field pointing to PLAYER,* factions,
    // but player factions not have filled team field, and hard go from faction value to faction_template value,
    // so more easy just sort by auction house ids
    switch (house->houseId)
    {
        case 1: case 2: case 3:
            return ALLIANCE;
        case 4: case 5: case 6:
            return HORDE;
        case 7:
        default:
            return 0;                                       // neutral
    }
}

AuctionHouseEntry const* AuctionHouseMgr::GetAuctionHouseEntry(Unit* unit)
{
    uint32 houseid = 1;                                     // dwarf auction house (used for normal cut/etc percents)

    if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        if (unit->GetTypeId() == TYPEID_UNIT)
        {
            // FIXME: found way for proper auctionhouse selection by another way
            // AuctionHouse.dbc have faction field with _player_ factions associated with auction house races.
            // but no easy way convert creature faction to player race faction for specific city
            uint32 factionTemplateId = unit->getFaction();
            switch (factionTemplateId)
            {
                case   12: houseid = 1; break;              // human
                case   29: houseid = 6; break;              // orc, and generic for horde
                case   55: houseid = 2; break;              // dwarf/gnome, and generic for alliance
                case   68: houseid = 4; break;              // undead
                case   80: houseid = 3; break;              // n-elf
                case  104: houseid = 5; break;              // taurens
                case  120: houseid = 7; break;              // booty bay, neutral
                case  474: houseid = 7; break;              // gadgetzan, neutral
                case  534: houseid = 2; break;              // Alliance Generic
                case  855: houseid = 7; break;              // everlook, neutral
                case 1604: houseid = 6; break;              // b-elfs,
                case 1638: houseid = 2; break;              // exodar, alliance
                default:                                    // for unknown case
                {
                    FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(factionTemplateId);
                    if (!u_entry)
                        houseid = 7;                        // goblin auction house
                    else if (u_entry->ourMask & FACTION_MASK_ALLIANCE)
                        houseid = 1;                        // human auction house
                    else if (u_entry->ourMask & FACTION_MASK_HORDE)
                        houseid = 6;                        // orc auction house
                    else
                        houseid = 7;                        // goblin auction house
                    break;
                }
            }
        }
        else
            houseid = 7; //basicly it's an error, auctioneer MUST be a creature, but we gotta return real available auction house -> so return neutral one
    }

    return GetAuctionHouseEntryByHouseId(houseid);
}

AuctionHouseEntry const* AuctionHouseMgr::GetAuctionHouseEntryByHouseId(uint32 houseid)
{
    return sAuctionHouseStore.LookupEntry(houseid);
}

void AuctionHouseObject::Update()
{
    time_t curTime = sWorld.GetGameTime();
    ///- Handle expired auctions
    for (AuctionEntryMap::iterator itr = AuctionsMap.begin(); itr != AuctionsMap.end();)
    {
        if (curTime > itr->second->expireTime)
        {
            ///- perform the transaction if there was bidder
            if (itr->second->bid)
            {
                AuctionEntryMap::iterator current = itr++;
                current->second->AuctionBidWinning();
            }
            ///- cancel the auction if there was no bidder and clear the auction
            else
            {
                sAuctionMgr.SendAuctionExpiredMail(itr->second);

                itr->second->DeleteFromDB();
                sAuctionMgr.RemoveAItem(itr->second->itemGuidLow);
                delete itr->second;
                AuctionsMap.erase(itr++);
            }
        }
        else
            ++itr;
    }
}

void AuctionHouseObject::BuildListBidderItems(WorldPacket& data, Player* player, uint32& count, uint32& totalcount)
{
    for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin();itr != AuctionsMap.end();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if (Aentry->bidder == player->GetGUIDLow())
        {
            if (itr->second->BuildAuctionInfo(data))
                ++count;
            ++totalcount;
        }
    }
}

void AuctionHouseObject::BuildListOwnerItems(WorldPacket& data, Player* player, uint32& count, uint32& totalcount)
{
    for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin();itr != AuctionsMap.end();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if (Aentry->owner == player->GetGUIDLow())
        {
            if (count < 240 && Aentry->BuildAuctionInfo(data))
                ++count;
            ++totalcount;
        }
    }
}

int AuctionEntry::CompareAuctionEntry(uint32 column, const AuctionEntry *auc, Player* viewPlayer) const
{
    switch (column)
    {
        case 0:                                             // level = 0
        {
            ItemPrototype const* itemProto1 = ObjectMgr::GetItemPrototype(itemTemplate);
            ItemPrototype const* itemProto2 = ObjectMgr::GetItemPrototype(auc->itemTemplate);
            if (!itemProto2 || !itemProto1)
                return 0;
            if (itemProto1->RequiredLevel < itemProto2->RequiredLevel)
                return -1;
            else if (itemProto1->RequiredLevel > itemProto2->RequiredLevel)
                return +1;
            break;
        }
        case 1:                                             // quality = 1
        {
            ItemPrototype const* itemProto1 = ObjectMgr::GetItemPrototype(itemTemplate);
            ItemPrototype const* itemProto2 = ObjectMgr::GetItemPrototype(auc->itemTemplate);
            if (!itemProto2 || !itemProto1)
                return 0;
            if (itemProto1->Quality < itemProto2->Quality)
                return -1;
            else if (itemProto1->Quality > itemProto2->Quality)
                return +1;
            break;
        }
        case 2:                                             // buyoutthenbid = 2
            if (buyout != auc->buyout)
            {
                if (buyout < auc->buyout)
                    return -1;
                else if (buyout > auc->buyout)
                    return +1;
            }
            else
            {
                if (bid < auc->bid)
                    return -1;
                else if (bid > auc->bid)
                    return +1;
            }
            break;
        case 3:                                             // duration = 3
            if (expireTime < auc->expireTime)
                return -1;
            else if (expireTime > auc->expireTime)
                return +1;
            break;
        case 4:                                             // status = 4
            if (bidder < auc->bidder)
                return -1;
            else if (bidder > auc->bidder)
                return +1;
            break;
        case 5:                                             // name = 5
        {
            ItemPrototype const* itemProto1 = ObjectMgr::GetItemPrototype(itemTemplate);
            ItemPrototype const* itemProto2 = ObjectMgr::GetItemPrototype(auc->itemTemplate);
            if (!itemProto2 || !itemProto1)
                return 0;

            int32 loc_idx = viewPlayer->GetSession()->GetSessionDbLocaleIndex();

            std::string name1 = itemProto1->Name1;
            sObjectMgr.GetItemLocaleStrings(itemProto1->ItemId, loc_idx, &name1);

            std::string name2 = itemProto2->Name1;
            sObjectMgr.GetItemLocaleStrings(itemProto2->ItemId, loc_idx, &name2);

            std::wstring wname1, wname2;
            Utf8toWStr(name1, wname1);
            Utf8toWStr(name2, wname2);
            return wname1.compare(wname2);
        }
        case 6:                                             // minbidbuyout = 6
        {
            uint32 bid1 = bid ? bid : startbid;
            uint32 bid2 = auc->bid ? auc->bid : auc->startbid;

            if (bid1 != bid2)
            {
                if (bid1 < bid2)
                    return -1;
                else if (bid1 > bid2)
                    return +1;
            }
            else
            {
                if (buyout < auc->buyout)
                    return -1;
                else if (buyout > auc->buyout)
                    return +1;
            }

            break;
        }
        case 7:                                             // seller = 7
            return ownerName.compare(auc->ownerName);
        case 8:                                             // bid = 8
        {
            uint32 bid1 = bid ? bid : startbid;
            uint32 bid2 = auc->bid ? auc->bid : auc->startbid;

            if (bid1 < bid2)
                return -1;
            else if (bid1 > bid2)
                    return +1;
            break;
        }
        case 9:                                             // quantity = 9
        {
            if (itemCount < auc->itemCount)
                return -1;
            else if (itemCount > auc->itemCount)
                return +1;
            break;
        }
        case 10:                                            // buyout = 10
            if (buyout < auc->buyout)
                return -1;
            else if (buyout > auc->buyout)
                return +1;
            break;
        case 11:                                            // unused = 11
        default:
            break;
    }

    return 0;
}

bool AuctionSorter::operator()(const AuctionEntry *auc1, const AuctionEntry *auc2) const
{
    if (m_sort[0] == MAX_AUCTION_SORT)                      // not sorted
        return false;

    for (uint32 i = 0; i < MAX_AUCTION_SORT; ++i)
    {
        if (m_sort[i] == MAX_AUCTION_SORT)                  // end of sort
            return false;

        int res = auc1->CompareAuctionEntry(m_sort[i] & ~AUCTION_SORT_REVERSED, auc2, m_viewPlayer);
        // "equal" by used column
        if (res == 0)
            continue;
        // less/greater and normal/reversed ordered
        return (res < 0) == ((m_sort[i] & AUCTION_SORT_REVERSED) == 0);
    }

    return false;                                           // "equal" by all sorts
}

void WorldSession::BuildListAuctionItems(std::vector<AuctionEntry*> const& auctions, WorldPacket& data, std::wstring const& wsearchedname, uint32 listfrom, uint32 levelmin,
    uint32 levelmax, uint32 usable, uint32 inventoryType, uint32 itemClass, uint32 itemSubClass, uint32 quality, uint32& count, uint32& totalcount, bool isFull)
{
    int loc_idx = _player->GetSession()->GetSessionDbLocaleIndex();

    for (std::vector<AuctionEntry*>::const_iterator itr = auctions.begin(); itr != auctions.end(); ++itr)
    {
        AuctionEntry *Aentry = *itr;
        Item *item = sAuctionMgr.GetAItem(Aentry->itemGuidLow);
        if (!item)
            continue;

        if (isFull)
        {
            ++count;
            Aentry->BuildAuctionInfo(data);
        }
        else
        {
            ItemPrototype const *proto = item->GetProto();

            if (itemClass != 0xffffffff && proto->Class != itemClass)
                continue;

            if (itemSubClass != 0xffffffff && proto->SubClass != itemSubClass)
                continue;

            if (inventoryType != 0xffffffff && proto->InventoryType != inventoryType)
                continue;

            if (quality != 0xffffffff && proto->Quality != quality)
                continue;

            if (levelmin != 0x00 && (proto->RequiredLevel < levelmin || (levelmax != 0x00 && proto->RequiredLevel > levelmax)))
                continue;

            if (usable != 0x00 && !_player->CanUseItem(proto))
                continue;

            std::string name = proto->Name1;
            if (name.empty())
                continue;

            sObjectMgr.GetItemLocaleStrings(proto->ItemId, loc_idx, &name);

            if (!wsearchedname.empty() && !Utf8FitTo(name, wsearchedname))
                continue;

            if (count < 50 && totalcount >= listfrom)
            {
                ++count;
                Aentry->BuildAuctionInfo(data);
            }
        }

        ++totalcount;
    }
}

void WorldSession::BuildListAuctionItems(AuctionHouseObject::AuctionEntryMap const& auctions, WorldPacket& data, std::wstring const& wsearchedname, uint32 listfrom, uint32 levelmin,
    uint32 levelmax, uint32 usable, uint32 inventoryType, uint32 itemClass, uint32 itemSubClass, uint32 quality, uint32& count, uint32& totalcount, bool isFull)
{
    int loc_idx = _player->GetSession()->GetSessionDbLocaleIndex();

    for (AuctionHouseObject::AuctionEntryMap::const_iterator itr = auctions.begin(); itr != auctions.end(); ++itr)
    {
        AuctionEntry *Aentry = itr->second;
        Item *item = sAuctionMgr.GetAItem(Aentry->itemGuidLow);
        if (!item)
            continue;

        if (isFull)
        {
            ++count;
            Aentry->BuildAuctionInfo(data);
        }
        else
        {
            ItemPrototype const *proto = item->GetProto();

            if (itemClass != 0xffffffff && proto->Class != itemClass)
                continue;

            if (itemSubClass != 0xffffffff && proto->SubClass != itemSubClass)
                continue;

            if (inventoryType != 0xffffffff && proto->InventoryType != inventoryType)
                continue;

            if (quality != 0xffffffff && proto->Quality != quality)
                continue;

            if (levelmin != 0x00 && (proto->RequiredLevel < levelmin || (levelmax != 0x00 && proto->RequiredLevel > levelmax)))
                continue;

            if (usable != 0x00 && !_player->CanUseItem(proto))
                continue;

            std::string name = proto->Name1;
            if (name.empty())
                continue;

            sObjectMgr.GetItemLocaleStrings(proto->ItemId, loc_idx, &name);

            if (!wsearchedname.empty() && !Utf8FitTo(name, wsearchedname))
                continue;

            if (count < 50 && totalcount >= listfrom)
            {
                ++count;
                Aentry->BuildAuctionInfo(data);
            }
        }

        ++totalcount;
    }
}

void AuctionHouseObject::AddAuction(AuctionEntry *ah)
{
    ASSERT(ah);
    AuctionsMap[ah->Id] = ah;
}

bool AuctionHouseObject::RemoveAuction(AuctionEntry* ah)
{
    return AuctionsMap.erase(ah->Id);
}

AuctionEntry* AuctionHouseObject::AddAuction(AuctionHouseEntry const* auctionHouseEntry, Item* it, uint32 etime, uint32 bid, uint32 buyout, uint32 deposit, Player * pl /*= NULL*/)
{
    uint32 auction_time = uint32(etime * sWorld.getConfig(RATE_AUCTION_TIME));

    AuctionEntry *AH = new AuctionEntry;
    AH->Id = sObjectMgr.GenerateAuctionID();
    AH->itemGuidLow = it->GetObjectGuid().GetCounter();
    AH->itemTemplate = it->GetEntry();
    AH->itemCount = it->GetCount();
    AH->itemRandomPropertyId = it->GetItemRandomPropertyId();
    AH->owner = pl ? pl->GetGUIDLow() : 0;

    if (pl)
        Utf8toWStr(pl->GetName(), AH->ownerName);

    AH->startbid = bid;
    AH->bidder = 0;
    AH->bid = 0;
    AH->buyout = buyout;
    AH->expireTime = time(NULL) + auction_time;
    AH->deposit = deposit;
    AH->auctionHouseEntry = auctionHouseEntry;

    AddAuction(AH);

    sAuctionMgr.AddAItem(it);

    if (pl)
        pl->MoveItemFromInventory(it->GetBagSlot(), it->GetSlot(), true, "AUCTION");

    RealmDataDatabase.BeginTransaction();

    if (pl)
        it->DeleteFromInventoryDB();

    it->SaveToDB();
    AH->SaveToDB();

    if (pl)
        pl->SaveInventoryAndGoldToDB();

    RealmDataDatabase.CommitTransaction();

    return AH;
}

// this function inserts to WorldPacket auction's data
bool AuctionEntry::BuildAuctionInfo(WorldPacket & data) const
{
    Item *pItem = sAuctionMgr.GetAItem(itemGuidLow);
    if (!pItem)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: auction to item, that doesn't exist !!!!");
        return false;
    }
    data << uint32(Id);
    data << uint32(pItem->GetEntry());

    for (uint8 i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
    {
        data << uint32(pItem->GetEnchantmentId(EnchantmentSlot(i)));
        data << uint32(pItem->GetEnchantmentDuration(EnchantmentSlot(i)));
        data << uint32(pItem->GetEnchantmentCharges(EnchantmentSlot(i)));
    }

    data << uint32(pItem->GetItemRandomPropertyId());       // random item property id
    data << uint32(pItem->GetItemSuffixFactor());           // SuffixFactor
    data << uint32(pItem->GetCount());                      // item->count
    data << uint32(pItem->GetSpellCharges(0, false));       // item->charge FFFFFFF - item->charge + 1
    data << uint32(0);                                      // item flags (dynamic?) (0x04 no lockId?)
    data << ObjectGuid(HIGHGUID_PLAYER, owner);             // Auction->owner
    data << uint32(startbid);                               // Auction->startbid (not sure if useful)
    data << uint32(bid ? GetAuctionOutBid() : 0);           // minimal outbid
    data << uint32(buyout);                                 // auction->buyout
    data << uint32((expireTime-time(NULL))*MILLISECONDS); // time left
    data << ObjectGuid(HIGHGUID_PLAYER, bidder);            // auction->bidder current
    data << uint32(bid);                                    // current bid
    return true;
}

uint32 AuctionEntry::GetAuctionCut() const
{
    return uint32(auctionHouseEntry->cutPercent * bid * sWorld.getConfig(RATE_AUCTION_CUT) / 100.f);
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint32 AuctionEntry::GetAuctionOutBid() const
{
    uint32 outbid = (bid / 100) * 5;
    if (!outbid)
        outbid = 1;
    return outbid;
}

void AuctionEntry::DeleteFromDB() const
{
    //No SQL injection (Id is integer)
    RealmDataDatabase.PExecute("DELETE FROM auctionhouse WHERE id = '%u'",Id);
}

void AuctionEntry::SaveToDB() const
{
    static SqlStatementID saveAuction;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(saveAuction, "INSERT INTO auctionhouse (id,houseid,itemguid,item_template,item_count,item_randompropertyid,itemowner,buyoutprice,time,buyguid,lastbid,startbid,deposit) "
                                                                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    stmt.addUInt32(Id);
    stmt.addUInt32(auctionHouseEntry->houseId);
    stmt.addUInt32(itemGuidLow);
    stmt.addUInt32(itemTemplate);
    stmt.addUInt32(itemCount);
    stmt.addInt32(itemRandomPropertyId);
    stmt.addUInt32(owner);
    stmt.addUInt32(buyout);
    stmt.addUInt64((uint64)expireTime);
    stmt.addUInt32(bidder);
    stmt.addUInt32(bid);
    stmt.addUInt32(startbid);
    stmt.addUInt32(deposit);

    stmt.Execute();

    // SQL ERROR: Out of range value for column 'item_randompropertyid' at row 1
    //if (itemRandomPropertyId > 2164)
    //sLog.outLog(LOG_SPECIAL, "itemRandomPropertyId for item entry %u (guid %u) is %u (owner %u)", itemTemplate, itemGuidLow, itemRandomPropertyId, owner);
}

void AuctionEntry::AuctionBidWinning(Player* newbidder)
{
    //sAuctionMgr.SendAuctionSalePendingMail(this);
    sAuctionMgr.SendAuctionSuccessfulMail(this);
    sAuctionMgr.SendAuctionWonMail(this);

    sAuctionMgr.RemoveAItem(this->itemGuidLow);
    sAuctionMgr.GetAuctionsMap(this->auctionHouseEntry)->RemoveAuction(this);

    RealmDataDatabase.BeginTransaction();
    this->DeleteFromDB();
    if (newbidder)
        newbidder->SaveInventoryAndGoldToDB();
    RealmDataDatabase.CommitTransaction();

    delete this;
}

bool AuctionEntry::UpdateBid(uint32 newbid, Player* newbidder /*=NULL*/)
{
    Player* auction_owner = owner ? sObjectMgr.GetPlayerInWorld(ObjectGuid(HIGHGUID_PLAYER, owner)) : NULL;

    // bid can't be greater buyout
    if (buyout && newbid > buyout)
        newbid = buyout;

    if (newbidder && newbidder->GetGUIDLow() == bidder)
    {
        newbidder->ModifyMoney(-int32(newbid - bid));
    }
    else
    {
        if (newbidder)
            newbidder->ModifyMoney(-int32(newbid));

        if (bidder)                                     // return money to old bidder if present
            WorldSession::SendAuctionOutbiddedMail(this);
    }

    bidder = newbidder ? newbidder->GetGUIDLow() : 0;
    bid = newbid;

    if ((newbid < buyout) || (buyout == 0))                 // bid
    {
        if (auction_owner)
            auction_owner->GetSession()->SendAuctionOwnerNotification(this, false);

        // after this update we should save player's money ...
        RealmDataDatabase.BeginTransaction();
        RealmDataDatabase.PExecute("UPDATE auctionhouse SET buyguid = '%u', lastbid = '%u' WHERE id = '%u'", bidder, bid, Id);
        if (newbidder)
            newbidder->SaveInventoryAndGoldToDB();
        RealmDataDatabase.CommitTransaction();
        return true;
    }
    else                                                    // buyout
    {
        AuctionBidWinning(newbidder);
        return false;
    }
}
