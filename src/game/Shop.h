#ifndef HELLGROUND_SHOP_H
#define HELLGROUND_SHOP_H

#include "Common.h"
#include <list>
#include <map>
#include <ace/Thread_Mutex.h>

#define GO_BACK_ITEM_SHOP 693103
#define GOSSIP_CATEGORIES_NPC_TEXT_X100 990023
#define GOSSIP_CATEGORIES_NPC_TEXT_SUB 990024
#define GOSSIP_CATEGORIES_NPC_TEXT_LOWRATE 990025

class Player;
class Creature;
struct ItemPrototype;

struct shopItem
{
    uint32 itemId;
    uint16 price;
};

struct catData
{
    std::list<uint32> subcategories;
    std::list<shopItem> items;
    uint32 string_id;
    uint16 subCategoryOf;
    uint8 icon;
};

struct catCoins
{
    uint16 prevCat;
    uint16 thisCat;
    uint32 goldVisual;
};

class HELLGROUND_IMPORT_EXPORT Shop
{
    public:

        Shop();
        ~Shop();

        void LoadShop();
        
        // prevCategory 0 is Main Menu
        void SendShopList(Creature* sender, Player* plr, uint16 category);
        
        void HandleShopDoBuy(Creature* sender, Player*plr, uint32 itemid, uint8 count);
        
        void HandleBuyShopPacket(Creature* sender, Player*plr, uint32 itemid, ItemPrototype const *pProto, uint8 count);
        
        //void SendPlayerHaveMoney(Player* plr); // emulates money visually for player until he goes off the vendor

        uint32 GetMainStringId() { return catMap[0].string_id; };

        bool NeedsSlot(uint32 itemId);
        
        ACE_Thread_Mutex plrVariableMutex;
        std::map<uint64, catCoins> plrVariable;
    private:

        // category -> subcategories list
        std::map<uint16, catData> catMap;
};
#endif