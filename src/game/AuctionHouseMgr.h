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

#ifndef HELLGROUND_AUCTION_HOUSE_MGR_H
#define HELLGROUND_AUCTION_HOUSE_MGR_H

#include "ace/Singleton.h"

#include "Common.h"
#include "SharedDefines.h"
#include "DBCStructure.h"
#include "Log.h"

class Item;
class Player;
class Unit;
class WorldPacket;

#define MIN_AUCTION_TIME (12*HOUR)
#define MAX_AUCTION_SORT 12
#define AUCTION_SORT_REVERSED 0x10

enum AuctionError
{
    AUCTION_OK                          = 0,                // depends on enum AuctionAction
    AUCTION_ERR_INVENTORY               = 1,                // depends on enum InventoryChangeResult
    AUCTION_ERR_DATABASE                = 2,                // ERR_AUCTION_DATABASE_ERROR (default)
    AUCTION_ERR_NOT_ENOUGH_MONEY        = 3,                // ERR_NOT_ENOUGH_MONEY
    AUCTION_ERR_ITEM_NOT_FOUND          = 4,                // ERR_ITEM_NOT_FOUND
    AUCTION_ERR_HIGHER_BID              = 5,                // ERR_AUCTION_HIGHER_BID
    AUCTION_ERR_BID_INCREMENT           = 7,                // ERR_AUCTION_BID_INCREMENT
    AUCTION_ERR_BID_OWN                 = 10,               // ERR_AUCTION_BID_OWN
    AUCTION_ERR_RESTRICTED_ACCOUNT      = 13                // ERR_RESTRICTED_ACCOUNT
};

enum AuctionAction
{
    AUCTION_STARTED     = 0,                                // ERR_AUCTION_STARTED
    AUCTION_REMOVED     = 1,                                // ERR_AUCTION_REMOVED
    AUCTION_BID_PLACED  = 2                                 // ERR_AUCTION_BID_PLACED
};

struct AuctionEntry
{
    uint32 Id;
    uint32 itemGuidLow;                                     // can be 0 after send won mail with item
    uint32 itemTemplate;
    uint32 itemCount;
    int32 itemRandomPropertyId;
    uint32 owner;                                           // player low guid, can be 0 for server generated auction
    std::wstring ownerName;                                 // cache name for sorting
    uint32 startbid;                                        // start minimal bid value
    uint32 bid;                                             // current bid, =0 meaning no bids
    uint32 buyout;
    time_t expireTime;
    uint32 bidder;                                          // current bidder player lowguid, can be 0 if bid generated by server, use 'bid'!=0 for check bid existance
    uint32 deposit;                                         // deposit can be calculated only when creating auction
    AuctionHouseEntry const* auctionHouseEntry;             // in AuctionHouse.dbc

    // helpers
    uint32 GetHouseId() const { return auctionHouseEntry->houseId; }
    uint32 GetHouseFaction() const { return auctionHouseEntry->faction; }
    uint32 GetAuctionCut() const;
    uint32 GetAuctionOutBid() const;
    bool BuildAuctionInfo(WorldPacket & data) const;
    void DeleteFromDB() const;
    void SaveToDB() const;
    void AuctionBidWinning(Player* bidder = NULL);

    // -1,0,+1 order result
    int CompareAuctionEntry(uint32 column, const AuctionEntry *auc, Player* viewPlayer) const;

    bool UpdateBid(uint32 newbid, Player* newbidder = NULL);// true if normal bid, false if buyout, bidder==NULL for generated bid
};

//this class is used as auctionhouse instance
class AuctionHouseObject
{
    public:
        AuctionHouseObject() {}
        ~AuctionHouseObject()
        {
            for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin(); itr != AuctionsMap.end(); ++itr)
                delete itr->second;
        }

        typedef std::map<uint32, AuctionEntry*> AuctionEntryMap;
        typedef std::pair<AuctionEntryMap::const_iterator, AuctionEntryMap::const_iterator> AuctionEntryMapBounds;

        uint32 GetCount() { return AuctionsMap.size(); }

        AuctionEntryMap const& GetAuctions() const { return AuctionsMap; }

        AuctionEntryMapBounds GetAuctionsBounds() const {return AuctionEntryMapBounds(AuctionsMap.begin(), AuctionsMap.end()); }

        void AddAuction(AuctionEntry *ah);

        AuctionEntry* GetAuction(uint32 id) const
        {
            AuctionEntryMap::const_iterator itr = AuctionsMap.find( id );
            return itr != AuctionsMap.end() ? itr->second : NULL;
        }

        bool RemoveAuction(AuctionEntry* ah);

        void Update();

        void BuildListBidderItems(WorldPacket& data, Player* player, uint32& count, uint32& totalcount);
        void BuildListOwnerItems(WorldPacket& data, Player* player, uint32& count, uint32& totalcount);

        AuctionEntry* AddAuction(AuctionHouseEntry const* auctionHouseEntry, Item* newItem, uint32 etime, uint32 bid, uint32 buyout = 0, uint32 deposit = 0, Player * pl = NULL);
    private:
        AuctionEntryMap AuctionsMap;
};

class AuctionSorter
{
    public:
        AuctionSorter(AuctionSorter const& sorter) : m_sort(sorter.m_sort), m_viewPlayer(sorter.m_viewPlayer) {}
        AuctionSorter(uint8 *sort, Player* viewPlayer) : m_sort(sort), m_viewPlayer(viewPlayer) {}
        bool operator()(const AuctionEntry *auc1, const AuctionEntry *auc2) const;

    private:
        uint8* m_sort;
        Player* m_viewPlayer;
};

enum AuctionHouseType
{
    AUCTION_HOUSE_ALLIANCE  = 0,
    AUCTION_HOUSE_HORDE     = 1,
    AUCTION_HOUSE_NEUTRAL   = 2
};

#define MAX_AUCTION_HOUSE_TYPE 3

class AuctionHouseMgr
{
    friend class ACE_Singleton<AuctionHouseMgr, ACE_Null_Mutex>;
    AuctionHouseMgr();

    public:
        ~AuctionHouseMgr();

        typedef UNORDERED_MAP<uint32, Item*> ItemMap;

        AuctionHouseObject* GetAuctionsMap(AuctionHouseType houseType) { return &mAuctions[houseType]; }
        AuctionHouseObject* GetAuctionsMap(AuctionHouseEntry const* house);

        Item* GetAItem(uint32 id)
        {
            ItemMap::const_iterator itr = mAitems.find(id);
            if (itr != mAitems.end())
            {
                return itr->second;
            }
            return NULL;
        }

        //auction messages
        void SendAuctionWonMail( AuctionEntry * auction );
        void SendAuctionSalePendingMail( AuctionEntry * auction );
        void SendAuctionSuccessfulMail( AuctionEntry * auction );
        void SendAuctionExpiredMail( AuctionEntry * auction );
        static uint32 GetAuctionDeposit(AuctionHouseEntry const* entry, uint32 time, Item *pItem);

        static uint32 GetAuctionHouseTeam(AuctionHouseEntry const* house);
        static AuctionHouseEntry const* GetAuctionHouseEntry(Unit* unit);
        static AuctionHouseEntry const* GetAuctionHouseEntryByHouseId(uint32 houseid);

    public:
        //load first auction items, because of check if item exists, when loading
        void LoadAuctionItems();
        void LoadAuctions();

        void AddAItem(Item* it);
        bool RemoveAItem(uint32 id);

        void Update();

    private:
        AuctionHouseObject  mAuctions[MAX_AUCTION_HOUSE_TYPE];

        ItemMap             mAitems;
};

#define sAuctionMgr (*ACE_Singleton<AuctionHouseMgr, ACE_Null_Mutex>::instance())
#endif
