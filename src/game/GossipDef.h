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

#ifndef HELLGROUND_GOSSIP_H
#define HELLGROUND_GOSSIP_H

#include "Common.h"
#include "QuestDef.h"
#include "NPCHandler.h"

class WorldSession;

#define GOSSIP_MAX_MENU_ITEMS 64                            // client supported items unknown, but provided number must be enough
#define DEFAULT_GOSSIP_MESSAGE              1

enum GossipOptionIcon
{
    GOSSIP_ICON_CHAT                = 0,                    //white chat bubble
    GOSSIP_ICON_VENDOR              = 1,                    //brown bag
    GOSSIP_ICON_TAXI                = 2,                    //flight
    GOSSIP_ICON_TRAINER             = 3,                    //book
    GOSSIP_ICON_INTERACT_1          = 4,                    //interaction wheel
    GOSSIP_ICON_INTERACT_2          = 5,                    //interaction wheel
    GOSSIP_ICON_MONEY_BAG           = 6,                    //brown bag with yellow dot
    GOSSIP_ICON_TALK                = 7,                    //white chat bubble with black dots
    GOSSIP_ICON_TABARD              = 8,                    //tabard
    GOSSIP_ICON_BATTLE              = 9,                    //two swords
    GOSSIP_ICON_DOT                 = 10,                    //yellow dot --- BUGGED. Instead of DOT client gets icon from previous page. BEWARE!
    GOSSIP_ICON_MAX
};

//POI defines
enum Poi_Icon
{
    ICON_POI_0                  =   0,                      // Grey ?
    ICON_POI_1                  =   1,                      // Red ?
    ICON_POI_2                  =   2,                      // Blue ?
    ICON_POI_BWTOMB             =   3,                      // Blue and White Tomb Stone
    ICON_POI_HOUSE              =   4,                      // House
    ICON_POI_TOWER              =   5,                      // Tower
    ICON_POI_REDFLAG            =   6,                      // Red Flag with Yellow !
    ICON_POI_TOMB               =   7,                      // Tomb Stone
    ICON_POI_BWTOWER            =   8,                      // Blue and White Tower
    ICON_POI_REDTOWER           =   9,                      // Red Tower
    ICON_POI_BLUETOWER          =   10,                     // Blue Tower
    ICON_POI_RWTOWER            =   11,                     // Red and White Tower
    ICON_POI_REDTOMB            =   12,                     // Red Tomb Stone
    ICON_POI_RWTOMB             =   13,                     // Red and White Tomb Stone
    ICON_POI_BLUETOMB           =   14,                     // Blue Tomb Stone
    ICON_POI_NOTHING            =   15,                     // NOTHING
    ICON_POI_16                 =   16,                     // Red ?
    ICON_POI_17                 =   17,                     // Grey ?
    ICON_POI_18                 =   18,                     // Blue ?
    ICON_POI_19                 =   19,                     // Red and White ?
    ICON_POI_20                 =   20,                     // Red ?
    ICON_POI_GREYLOGS           =   21,                     // Grey Wood Logs
    ICON_POI_BWLOGS             =   22,                     // Blue and White Wood Logs
    ICON_POI_BLUELOGS           =   23,                     // Blue Wood Logs
    ICON_POI_RWLOGS             =   24,                     // Red and White Wood Logs
    ICON_POI_REDLOGS            =   25,                     // Red Wood Logs
    ICON_POI_26                 =   26,                     // Grey ?
    ICON_POI_27                 =   27,                     // Blue and White ?
    ICON_POI_28                 =   28,                     // Blue ?
    ICON_POI_29                 =   29,                     // Red and White ?
    ICON_POI_30                 =   30,                     // Red ?
    ICON_POI_GREYHOUSE          =   31,                     // Grey House
    ICON_POI_BWHOUSE            =   32,                     // Blue and White House
    ICON_POI_BLUEHOUSE          =   33,                     // Blue House
    ICON_POI_RWHOUSE            =   34,                     // Red and White House
    ICON_POI_REDHOUSE           =   35,                     // Red House
    ICON_POI_GREYHORSE          =   36,                     // Grey Horse
    ICON_POI_BWHORSE            =   37,                     // Blue and White Horse
    ICON_POI_BLUEHORSE          =   38,                     // Blue Horse
    ICON_POI_RWHORSE            =   39,                     // Red and White Horse
    ICON_POI_REDHORSE           =   40                      // Red Horse
};

struct GossipMenuItem
{
    uint8       m_gIcon;
    bool        m_gCoded;
    std::string m_gMessage;
    uint32      m_gSender;
    uint32      m_gAction;
    std::string m_gBoxMessage;
    uint32      m_gBoxMoney;
};

typedef std::vector<GossipMenuItem> GossipMenuItemList;

struct QuestMenuItem
{
    uint32      m_qId;
    uint8       m_qIcon;
};

typedef std::vector<QuestMenuItem> QuestMenuItemList;

class HELLGROUND_IMPORT_EXPORT GossipMenu
{
    public:
        GossipMenu();
        ~GossipMenu();

        void AddMenuItem(uint8 Icon, const std::string& Message, bool Coded = false);
        void AddMenuItem(uint8 Icon, const std::string& Message, uint32 dtSender, uint32 dtAction, const std::string& BoxMessage, uint32 BoxMoney, bool Coded = false);

        // for using from scripts, don't must be inlined
        void AddMenuItem(uint8 Icon, char const* Message, bool Coded = false);
        void AddMenuItem(uint8 Icon, char const* Message, uint32 dtSender, uint32 dtAction, char const* BoxMessage, uint32 BoxMoney, bool Coded = false);

        unsigned int MenuItemCount() const
        {
            return m_gItems.size();
        }

        bool Empty() const
        {
            return m_gItems.empty();
        }

        GossipMenuItem const& GetItem(unsigned int Id)
        {
            return m_gItems[ Id ];
        }

        uint32 MenuItemSender(unsigned int ItemId);
        uint32 MenuItemAction(unsigned int ItemId);
        bool MenuItemCoded(unsigned int ItemId);

        void ClearMenu();

    protected:
        GossipMenuItemList m_gItems;
};

class QuestMenu
{
    public:
        QuestMenu();
        ~QuestMenu();

        void AddMenuItem(uint32 QuestId, uint8 Icon);
        void ClearMenu();

        uint8 MenuItemCount() const
        {
            return m_qItems.size();
        }

        bool Empty() const
        {
            return m_qItems.empty();
        }

        bool HasItem(uint32 questid);

        QuestMenuItem const& GetItem(uint16 Id)
        {
            return m_qItems[ Id ];
        }

    protected:
        QuestMenuItemList m_qItems;
};

class HELLGROUND_IMPORT_EXPORT PlayerMenu
{
    private:
        GossipMenu mGossipMenu;
        QuestMenu  mQuestMenu;
        WorldSession* pSession;

    public:
        PlayerMenu(WorldSession *Session);
        ~PlayerMenu();

        GossipMenu& GetGossipMenu() { return mGossipMenu; }
        QuestMenu& GetQuestMenu() { return mQuestMenu; }

        bool Empty() const { return mGossipMenu.Empty() && mQuestMenu.Empty(); }

        void ClearMenus();
        uint32 GossipOptionSender(unsigned int Selection);
        uint32 GossipOptionAction(unsigned int Selection);
        bool GossipOptionCoded(unsigned int Selection);

        void SendGossipMenu(uint32 TitleTextId, uint64 npcGUID);
        void CloseGossip();
        void SendPointOfInterest(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char * locName);
        void SendTalking(uint32 textID);
        void SendTalking(char const * title, char const * text);

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/
        void SendQuestGiverStatus(uint8 questStatus, uint64 npcGUID);

        void SendQuestGiverQuestList(QEmote eEmote, const std::string& Title, uint64 npcGUID);

        void SendQuestQueryResponse (Quest const *pQuest);
        void SendQuestGiverQuestDetails(Quest const *pQuest, uint64 npcGUID, bool ActivateAccept);

        void SendQuestGiverOfferReward(Quest const* pQuest, uint64 npcGUID, bool EnbleNext);
        void SendQuestGiverRequestItems(Quest const *pQuest, uint64 npcGUID, bool Completable, bool CloseOnCancel);
};
#endif
