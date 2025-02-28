// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Shop.h"
#include "Language.h"
#include "GossipDef.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "Chat.h"

Shop::Shop()
{
}

Shop::~Shop()
{
    for (auto i = catMap.begin(); i != catMap.end(); ++i)
    {
        i->second.items.clear();
        i->second.subcategories.clear();
    }
    catMap.clear();
}

void Shop::LoadShop()
{
    for (auto i = catMap.begin(); i != catMap.end(); ++i)
    {
        i->second.items.clear();
        i->second.subcategories.clear();
    }
    catMap.clear();

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT `id`, `category`, `text`, `icon` FROM `shop` order by `id`");

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint16 category = fields[0].GetUInt16();
            
            catMap[category].subCategoryOf = fields[1].GetUInt16();
            catMap[category].string_id = fields[2].GetUInt32();
            catMap[category].icon = fields[3].GetUInt8();

            if (category) // basic category should not be subcategory of anything
                catMap[catMap[category].subCategoryOf].subcategories.push_back(category);
        }
        while (result->NextRow());
    }

    // categories are set - now add items to them
    result = RealmDataDatabase.PQuery("SELECT `item`, `id`, `price`, `discount` FROM `shop_items` ORDER BY `price` DESC");
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 item_id = fields[0].GetUInt32();
            uint16 category = fields[1].GetUInt16();
            uint16 price = fields[2].GetUInt16();
            // apply discount
            uint8 discount = fields[3].GetUInt8(); // 0 - 100
            if (discount > 100)
                continue;
            // if price is 8 and discount is 30% -> it should become 6
            // if price is 8 and discount is 25% -> it should become 6

            price = ceil(float(price) * (float(100-discount) / 100.0f));

            shopItem newItem = {item_id, price};

            catMap[category].items.push_back(newItem);
        }
        while (result->NextRow());
    }
}

// prevCategory 0 is Main Menu
void Shop::SendShopList(Creature* sender, Player* plr, uint16 category)
{
    if (category == 1999) // Unsummon mini-pet
    {
        plr->RemoveMiniPet();
        return;
    }
    // MAIN MENU
    else if (category == 2000) // To main menu!
    {
        sScriptMgr.OnGossipHello(plr, sender);
        return;
    }
    else
        category -= 2001;

    // Restrict using shop from lowrate when still has coins on x100
    //if (sWorld.getConfig(CONFIG_REALM_TYPE) == REALM_X3)
    //{
    //    WorldSession* ses = plr->GetSession();
    //    // Should never happen
    //    if (!ses)
    //        return;

    //    uint32 accId = ses->GetAccountId();
    //    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT `coins_x100` FROM `account` WHERE `account_id`='%u' AND `coins_x100` > 0", accId);
    //    if (result)
    //    {
    //        ChatHandler(plr).PSendSysMessage(LANG_SPEND_COINS_X100, (*result)[0].GetInt32());
    //        return;
    //    }
    //}

    auto tryFind = catMap.find(category);
    if (tryFind == catMap.end())
        return;

    if (plrVariable.find(plr->GetGUID()) == plrVariable.end())
    {
        if (WorldSession* ses = plr->GetSession())
        {
            uint32 accId = ses->GetAccountId();
            QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT `coins` FROM `account` WHERE `account_id`='%u'", accId);
            if (result) // need to update player gold coins
            {
                Field *fields = result->Fetch();
                int32 coins = (int32)(fields[0].GetFloat());
                plrVariable[plr->GetGUID()].goldVisual = coins > 0 ? (uint32)coins : 0;

                if (!plr->HasAura(55193)) // force update values after updating gold coins
                {
                    int32 thisentry = sender->GetEntry();
                    plr->CastCustomSpell(plr, 55193, &thisentry, NULL, NULL, true);
                }
                else
                    plr->ForceValuesUpdateAtIndex(PLAYER_FIELD_COINAGE);
            }
            else
            {
                plr->PlayerTalkClass->CloseGossip();
                return;
            }
        }
        else
        {
            plr->PlayerTalkClass->CloseGossip();
            return;
        }
    }

    if (!plr->HasAura(55193))
    {
        sLog.outLog(LOG_SPECIAL, "DONATE SHOP IS BUGGED! Player %s shop aura bug (thisentry %i)", plr->GetName(), sender->GetEntry());

		int32 thisentry = sender->GetEntry();
		plr->CastCustomSpell(plr, 55193, &thisentry, NULL, NULL, true);
		if (!plr->HasAura(55193))
		{
			sLog.outLog(LOG_SPECIAL, "DONATE SHOP IS BUGGED AGAIN... CANT REAPPLY! Player %s shop aura bug (thisentry %i)", plr->GetName(), sender->GetEntry());
		}
    }

    if (plrVariable.find(plr->GetGUID()) == plrVariable.end())
    {
        ChatHandler(plr).PSendSysMessage("DONATE SHOP IS BUGGED! Please, contact administration. Debug info %s %u", plr->GetName(), sender->GetEntry());
        sLog.outLog(LOG_SPECIAL, "DONATE SHOP IS BUGGED! Player %s shop plrVariable.end() (thisentry %i)", plr->GetName(), sender->GetEntry());
        plr->PlayerTalkClass->CloseGossip();
        return;
    }

    // SUBCATEGORY SENDING
    if (!catMap[category].subcategories.empty())
    {
        plr->PlayerTalkClass->ClearMenus();
        for (auto i = catMap[category].subcategories.begin(); i!= catMap[category].subcategories.end(); ++i)
            plr->PlayerTalkClass->GetGossipMenu().AddMenuItem((catMap[*i].icon),plr->GetSession()->GetHellgroundString(catMap[*i].string_id),1/*GOSSIP_SENDER_MAIN*/,2001+(*i),"",0);

        // go back
        if ((category && sender->GetEntry() != 693016) || catMap[category].subCategoryOf) // no need to back if back is main menu
            plr->PlayerTalkClass->GetGossipMenu().AddMenuItem(7,plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK),1/*GOSSIP_SENDER_MAIN*/,2001+catMap[category].subCategoryOf,"",0);

        if (category || sender->GetEntry() != 693016)
            plr->PlayerTalkClass->GetGossipMenu().AddMenuItem(7,plr->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU),1/*GOSSIP_SENDER_MAIN*/,2000,"",0);

        // This is the first, MAIN menu. And the Donate Shop creature is our mini-pet
        if (category == 0 && sender->GetOwnerGUID() == plr->GetGUID() && sender->isPet() && ((Pet*)sender)->getPetType() == MINI_PET) 
            plr->PlayerTalkClass->GetGossipMenu().AddMenuItem(7, plr->GetSession()->GetHellgroundString(LANG_DONATE_UNSUMMOM), 1/*GOSSIP_SENDER_MAIN*/, 1999, "", 0);

        plr->PlayerTalkClass->SendGossipMenu(category ? GOSSIP_CATEGORIES_NPC_TEXT_SUB : (!sWorld.isEasyRealm() ? GOSSIP_CATEGORIES_NPC_TEXT_LOWRATE : GOSSIP_CATEGORIES_NPC_TEXT_X100), sender->GetGUID());
    }
    else
    {
        plrVariable[plr->GetGUID()].prevCat = 2001+catMap[category].subCategoryOf;
        plrVariable[plr->GetGUID()].thisCat = 2001+category;

        // ITEM SENDING
        ItemPrototype const* lastItemProto = ObjectMgr::GetItemPrototype(GO_BACK_ITEM_SHOP);
        if (!lastItemProto)
        {
            // just go to main menu
            sScriptMgr.OnGossipHello(plr, sender);
            return;
        }
        ItemPrototype const* itemProto = NULL;
        uint8 numitems = catMap[category].items.size();
    
        //example numitems 16
        uint8 go_back_buttons = (numitems-1)/9 + 1; // 6 ? 10. 9 ? 10. 10 ? 2. 17 ? 2. 18 ? 2. 19 ? 3.
    
        WorldPacket data(SMSG_LIST_INVENTORY, (8+1+(go_back_buttons * 10)*8*4));

        data << uint64(sender->GetGUID());
        data << uint8(go_back_buttons * 10);

        auto ptr = catMap[category].items.begin();
        auto end = catMap[category].items.end();
        for (uint8 count = 0; count < (go_back_buttons * 10);)
        {
            // count must be increased before all other
            ++count;
            if ((count % 10) == 0) // each page tenth item is main_menu
            {
                data << uint32(count);
                data << uint32(lastItemProto->ItemId);
                data << uint32(lastItemProto->DisplayInfoID);
                data << uint32(0xFFFFFFFF);
                data << uint32(0);
                data << uint32(lastItemProto->MaxDurability);
                data << uint32(1); // buyCount - should never be more than 1
                data << uint32(0);
            }
            else if (ptr != end)
            {
                itemProto = ObjectMgr::GetItemPrototype(ptr->itemId);
                if (itemProto)
                {
                    data << uint32(count);
                    data << uint32(itemProto->ItemId);
                    data << uint32(itemProto->DisplayInfoID);
                    data << uint32(0xFFFFFFFF);
                    data << uint32(ptr->price * 10000); // in gold
                    data << uint32(itemProto->MaxDurability);
                    data << uint32(1); // buyCount - should never be more than 1
                    data << uint32(0);
                }
                else // empty space
                {
                    data << uint32(count);
                    data << uint32(0);
                    data << uint32(0);
                    data << uint32(0xFFFFFFFF);
                    data << uint32(0);
                    data << uint32(0);
                    data << uint32(1); // buyCount - should never be more than 1
                    data << uint32(0);
                }
                ++ptr;
            }
            else // empty space
            {
                data << uint32(count);
                data << uint32(0);
                data << uint32(0);
                data << uint32(0xFFFFFFFF);
                data << uint32(0);
                data << uint32(0);
                data << uint32(1); // buyCount - should never be more than 1
                data << uint32(0);
            }
        }

        plr->GetSession()->SendPacket(&data);

        // @!newbie_quest
        switch (category)
        {
            case 1: plr->KilledMonster(690703, 0, 693032); break;
            case 4: plr->KilledMonster(690704, 0, 693032); break;
            case 31: plr->KilledMonster(690705, 0, 693032); break;
            case 12: plr->KilledMonster(690706, 0, 693032); break;
        }
    }
}

void Shop::HandleShopDoBuy(Creature* sender, Player* plr, uint32 itemid, uint8 count)
{
    if (!itemid)
        return;

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemid);
    if (!pProto)
        return;

    uint32 accId = plr->GetSession() ? plr->GetSession()->GetAccountId() : 0;
    if (!accId)
        return; // should never happen

    uint16 price = 0;
    for (auto i = catMap.begin(); i != catMap.end() && !price; ++i)
    {
        for (auto j = i->second.items.begin(); j != i->second.items.end() && !price; ++j)
            if (j->itemId == itemid)
                price = j->price;
    }

    if(price) 
    { 
        uint16 maxCount = 0xFFFF / price;
        if((uint16)count > maxCount) 
        { 
            sLog.outLog(LOG_CHEAT, "Player %s tried to buy %u item id %u, causing overflow", plr->GetName(), (uint32)count, pProto->ItemId); 
            count = (uint8)maxCount; // set count to max possible to buy without overflow of price
        } 
    }

    if (!price) // if no such item in Shop - return
    {
        plr->SendBuyError(BUY_ERR_CANT_FIND_ITEM, sender, itemid, 0);
        SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
        return;
    }
    
    uint16 haveGoldCoins = 0;
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT `coins` FROM `account` WHERE `account_id`='%u'", accId);
    if (result) // need to update player gold coins
    {
        Field *fields = result->Fetch();
        int32 coins = (int32)(fields[0].GetFloat());
        haveGoldCoins = coins > 0 ? (uint32)coins : 0;
    }
        
    if (haveGoldCoins < (price*count) || plrVariable[plr->GetGUID()].goldVisual < (price*count))
    {
        plr->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, sender, itemid, 0);
        SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
        return;
    }
    
    if (NeedsSlot(itemid))
    {
        ItemPosCountVec dest;
        
        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemid, 1*count);
        if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
        {
            plr->SendEquipError(msg, NULL, NULL);
            SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
            return;
        }

        if (Item *it = plr->StoreNewItem(dest, itemid, true, 0, "DONATE_SHOP"))
        {
            WorldPacket data(SMSG_BUY_ITEM, (8+4+4+4));
            data << sender->GetGUID();
            data << (uint32)(/*vendor_slot*/1);                // numbered from 1 at client
            data << (uint32)(0xFFFFFFFF);
            data << (uint32)count;
            plr->SendPacketToSelf(&data);

            plr->SendNewItem(it, 1*count, true, false, false);
        }
    }
    else // Arena Points, Gold, Honor
    {
        switch (itemid)
        {
            case 695005: // 10 GOld
            {
                uint64 goldToGive = count * 100000;
                if ((uint64)plr->GetMoney() + goldToGive >= MAX_MONEY_AMOUNT)
                {
                    plr->SendEquipError(EQUIP_ERR_TOO_MUCH_GOLD,NULL,NULL);
                    SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
                    return;
                }
                plr->ModifyMoney(int32(goldToGive));
				ChatHandler(plr).SendSysMessage(16573);
                break;
            }
            //case 693106: // 750 ap
            //case 691008: // 220 ap
            //{
            //    uint32 apToGive = count * (itemid == 693106 ? 750 : 220);
            //    if (plr->GetArenaPoints() + apToGive > sWorld.getConfig(CONFIG_MAX_ARENA_POINTS))
            //    {
            //        plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW,NULL,NULL);
            //        SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
            //        return;
            //    }
            //    plr->ModifyArenaPoints(int32(apToGive));
            //    break;
            //}
            //case 693107: // 10000 honor
            //{
            //    if (plr->GetHonorPoints() + 10000*count > sWorld.getConfig(CONFIG_MAX_HONOR_POINTS))
            //    {
            //        plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW,NULL,NULL);
            //        SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
            //        return;
            //    }
            //    plr->ModifyHonorPoints(int32(10000*count));
            //    WorldPacket data(SMSG_PVP_CREDIT,4+8+4);
            //    data << (uint32)(10000*count);
            //    data << (uint64)0;
            //    data << (uint32)0;
            //    plr->SendPacketToSelf(&data);
            //    break;
            //}
            default:
                plr->SendBuyError(BUY_ERR_CANT_FIND_ITEM, sender, itemid, 0);
                SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
                return; // not handled case
        }
    }

    AccountsDatabase.PExecute("UPDATE `account` SET `coins`=`coins`-'%u' WHERE `account_id`='%u'", price*count, accId);
    if (sWorld.isEasyRealm())
        AccountsDatabase.PExecute("UPDATE `account` SET `coins_x100`=`coins_x100`-'%u' WHERE `account_id`='%u'", price*count, accId);

    plrVariable[plr->GetGUID()].goldVisual -= price*count;
    if (!plr->HasAura(55193)) // force update values after updating gold coins
    {
        int32 thisentry = sender->GetEntry();
        plr->CastCustomSpell(plr, 55193, &thisentry, NULL, NULL, true);
    }
    else
        plr->ForceValuesUpdateAtIndex(PLAYER_FIELD_COINAGE);

    AccountsDatabase.PExecute("INSERT INTO `shop_log` (`id`, `account`, `realm`, `guid`, `name`, `item`, `price`, `count`, `date`) VALUES (NULL, '%u', '%u', '%u', '%s', '%u', '%u', '%u', NOW())", accId, realmID, plr->GetGUIDLow(), plr->GetName(), itemid, price, count);
    plr->SaveToDB();
    //after buy - send window of shop again
    SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000);
}

void Shop::HandleBuyShopPacket(Creature* sender, Player*plr, uint32 itemid, ItemPrototype const *pProto, uint8 count)
{
    if (itemid == GO_BACK_ITEM_SHOP)
        SendShopList(sender, plr, (plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.prevCat : 2000);
    // buying something
    else
    {
        if (!itemid)
            return;

        /*ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemid);
        if (!pProto)
            return;*/ // already checked in method from which this method is called

        uint32 accId = plr->GetSession() ? plr->GetSession()->GetAccountId() : 0;
        if (!accId)
            return; // should never happen

        uint16 price = 0;
        for (auto i = catMap.begin(); i != catMap.end() && !price; ++i)
        {
            for (auto j = i->second.items.begin(); j != i->second.items.end() && !price; ++j)
                if (j->itemId == itemid)
                    price = j->price;
        }

        if (!price) // if no such item in Shop - return
        {
            plr->SendBuyError(BUY_ERR_CANT_FIND_ITEM, sender, itemid, 0);
            return;
        }

        if (plrVariable[plr->GetGUID()].goldVisual < (price*count)) // no need to select new coins -> cause we're not updating them here
        {
            plr->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, sender, itemid, 0);
            return;
        }
    
        if (NeedsSlot(itemid))
        {
            ItemPosCountVec dest;
        
            uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemid, 1*count);
            if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
            {
                plr->SendEquipError(msg, NULL, NULL);
                return;
            }
        }
        //else // Arena Points, Gold, Honor
        //{
        //    switch (itemid)
        //    {
        //        case 693104: // 50000 gold
        //        case 691007: // 1000 gold
        //        {
        //            uint64 havemoney = plr->GetMoney();
        //            uint64 goldToGive = count * (itemid == 693104 ? 500000000 : 10000000);
        //            if (havemoney + goldToGive >= MAX_MONEY_AMOUNT)
        //            {
        //                plr->SendEquipError(EQUIP_ERR_TOO_MUCH_GOLD,NULL,NULL);
        //                return;
        //            }
        //            break;
        //        }
        //        case 693106: // 750 ap
        //        case 691008: // 220 ap
        //        {
        //            uint32 apToGive = count * (itemid == 693106 ? 750 : 220);
        //            if (plr->GetArenaPoints() + apToGive > sWorld.getConfig(CONFIG_MAX_ARENA_POINTS))
        //            {
        //                plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW,NULL,NULL);
        //                return;
        //            }
        //            break;
        //        }
        //        case 693107: // 10000 honor
        //        {
        //            if (plr->GetHonorPoints() + 10000*count > sWorld.getConfig(CONFIG_MAX_HONOR_POINTS))
        //            {
        //                plr->SendEquipError(EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW,NULL,NULL);
        //                return;
        //            }
        //            break;
        //        }
        //        default:
        //            plr->SendBuyError(BUY_ERR_CANT_FIND_ITEM, sender, itemid, 0);
        //            return; // not handled case
        //    }
        //}

        char ItemName[256];
        std::string ItemNameLocale;
        // are you sure you want to buy [%s]?
        if (count == 1)
            sprintf(ItemName, plr->GetSession()->GetHellgroundString(LANG_SHOP_ARE_YOU_SURE_SINGLE), plr->GetItemNameLocale(pProto, &ItemNameLocale), price);
        else
            sprintf(ItemName, plr->GetSession()->GetHellgroundString(LANG_SHOP_ARE_YOU_SURE_MANY), plr->GetItemNameLocale(pProto, &ItemNameLocale), count, price*count);

        plr->PlayerTalkClass->ClearMenus();
        plr->PlayerTalkClass->GetGossipMenu().AddMenuItem(GOSSIP_ICON_VENDOR,ItemName,count,100000+itemid,"", 0);
        plr->PlayerTalkClass->GetGossipMenu().AddMenuItem(GOSSIP_ICON_TALK,plr->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK),1/*GOSSIP_SENDER_MAIN*/,(plrVariable.find(plr->GetGUID()) != plrVariable.end()) ? plrVariable.find(plr->GetGUID())->second.thisCat : 2000,"",0);
        plr->PlayerTalkClass->SendGossipMenu(GOSSIP_CATEGORIES_NPC_TEXT_SUB, sender->GetGUID());
    }
}

bool Shop::NeedsSlot(uint32 itemId)
{
    switch (itemId)
    {
        case 695005: // 10 Gold
            return false;
        default:
            return true;
    }
}