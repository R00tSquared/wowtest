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
#include "WorldPacket.h"
#include "Log.h"
#include "Corpse.h"
#include "GameObject.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "LootMgr.h"
#include "Object.h"
#include "Group.h"
#include "World.h"
#include "Util.h"
#include "Chat.h"

void WorldSession::HandleAutostoreLootItemOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,1);

    sLog.outDebug("WORLD: CMSG_AUTOSTORE_LOOT_ITEM");
    Player  *player =   GetPlayer();
    uint64   lguid =    player->GetLootGUID();
    Loot    *loot;
    uint8    lootSlot;

    recv_data >> lootSlot;
    
    uint32 logLootBossEntry = 0;
    if (IS_GAMEOBJECT_GUID(lguid))
    {
        GameObject *go = player->GetMap()->GetGameObject(lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
        if (!go || (go->GetOwnerGUID() != _player->GetGUID() && go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE) && !go->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
        {
            player->SendLootRelease(lguid);
            return;
        }

        loot = &go->loot;
    }
    else if (IS_ITEM_GUID(lguid))
    {
        Item *pItem = player->GetItemByGuid(lguid);

        if (!pItem)
        {
            player->SendLootRelease(lguid);
            return;
        }

        loot = &pItem->loot;
    }
    else if (IS_CORPSE_GUID(lguid))
    {
        Corpse *bones = ObjectAccessor::GetCorpse(*player, lguid);
        if (!bones)
        {
            player->SendLootRelease(lguid);
            return;
        }
        loot = &bones->loot;
    }
    else
    {
        Creature* pCreature = GetPlayer()->GetMap()->GetCreature(lguid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->GetClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);

        if (!ok_loot || !pCreature->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
        {
            player->SendLootRelease(lguid);
            return;
        }

        logLootBossEntry = pCreature->GetEntry();
        loot = &pCreature->loot;
    }

    if (!loot->IsPlayerAllowedToLoot(player, NULL))
    {
        std::ostringstream str;
        str << "HandleAutostoreLootItem - player " << player->GetName() << " (GUID: " << player->GetGUIDLow();
        if (lootSlot < loot->items.size())
            str << ") is trying to loot item " << loot->items[lootSlot].itemid << " from ";
        else
            str << ") is trying to loot quest item (O_o) from ";
        str << (IS_GAMEOBJECT_GUID(lguid) ? "gobject" : "creature");
        str << " Entry " << GUID_ENPART(lguid) << " LowGUID " << GUID_LOPART(lguid) << "but he is not alowed to do so.\n";
        str << "MapID: " << player->GetMapId() << " InstanceID: " << player->GetInstanciableInstanceId() << " position X: ";
        str << player->GetPositionX() << " Y: " << player->GetPositionY() << " Z: " << player->GetPositionZ();
        sLog.outLog(LOG_CHEAT, "%s", str.str().c_str());
        player->SendLootRelease(lguid);
        return;
    }

    QuestItem *qitem = NULL;
    QuestItem *ffaitem = NULL;

    LootItem *item = loot->LootItemInSlot(lootSlot,player,&qitem,&ffaitem);

    if (!item)
    {
        player->SendEquipError(EQUIP_ERR_ALREADY_LOOTED, NULL, NULL);
        return;
    }

    // questitems use the blocked field for other purposes
    if (!qitem && (item->is_blocked || !item->AllowedForPlayer(player, 3)))
    {
        if (!(loot->m_from_entry == EPIC_RAID_CHEST || loot->m_from_entry == LEGENDARY_RAID_CHEST))
        {
            // Trentone - when already looking into loot, DoLootRelease should be called, not SendLootRelease, cause SendLootRelease only sends it to client, but server-side he's still looking into loot.
            player->SendLootRelease(lguid);
            return;
        }
    }

	uint32 itemid = item->itemid;
	uint8 count = item->count;

    // BOJ case
    std::string hash = _player->GetSession()->m_IPHash;

    bool is_boss_soul = false;
    for (const auto& bst : boss_souls_template) {
        if (bst.item_soul == itemid) {
            is_boss_soul = true;
            break;
        }
    }

    bool boj_replaced = false;
    if (sWorld.isEasyRealm() && (IS_CREATURE_GUID(lguid) || IS_GAMEOBJECT_GUID(lguid)))
    {
        if (is_boss_soul && loot->ItemTakersHashIPMatch(itemid, hash))
        {
            ChatHandler(_player).SendSysMessage(16638);
            player->SendEquipError(EQUIP_ERR_DONT_OWN_THAT_ITEM, NULL, NULL);
            return;
        }
        else if (itemid == ITEM_EMBLEM_OF_TRIUMPH)
        {
            if (loot->ItemTakersHashIPMatch(ITEM_EMBLEM_OF_TRIUMPH, hash))
            {
                ChatHandler(_player).SendSysMessage(16638);
                player->SendEquipError(EQUIP_ERR_DONT_OWN_THAT_ITEM, NULL, NULL);
                return;
            }

            uint32 bonus = _player->CalculateBonus(count);
            count += bonus;

            // send ad message
            if (!bonus)
                ChatHandler(_player).SendSysMessage(15623);
        }
        else if (itemid == ITEM_BADGE || itemid == SECOND_BADGE)
        {
            // multiply BOJ count on heroic for PREM and rates
            uint32 bonus = _player->CalculateBonus(count);
            count += bonus;

            if (count > 100 || count == 0)
                sLog.outLog(LOG_CRITICAL, "BOJ error! Player %s, count %u, new_count %u, bonus %u, itemid %u, from entry %u", _player->GetName(), item->count, count, bonus, itemid, loot->m_from_entry);

            if (count > 200)
                count = 200;

            // change badge to analog
            if (itemid == ITEM_BADGE && loot->ItemTakersHashIPMatch(ITEM_BADGE, hash))
            {
                boj_replaced = true;
                itemid = SECOND_BADGE;
            }

            // send ad message
            if (!bonus)
                ChatHandler(_player).SendSysMessage(15626);
        }
    }

    ItemPosCountVec dest;
    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemid, count);
    if (msg == EQUIP_ERR_OK)
    {
        Item * newitem = player->StoreNewItem(dest, itemid, true, item->randomPropertyId, "LOOT");

        if (qitem)
        {
            qitem->is_looted = true;
            //freeforall is 1 if everyone's supposed to get the quest item.
            if (item->freeforall || loot->GetPlayerQuestItems().size() == 1)
                player->SendNotifyLootItemRemoved(lootSlot);
            else
                loot->NotifyQuestItemRemoved(qitem->index);
        }
        else
        {
            if (ffaitem)
            {
                //freeforall case, notify only one player of the removal
                ffaitem->is_looted = true;

                player->SendNotifyLootItemRemoved(lootSlot);
            }
            else
            {
                loot->NotifyItemRemoved(lootSlot);
            }
        }

        //if only one person is supposed to loot the item, then set it to looted
        if (!item->freeforall)
        {
            loot->setItemLooted(item, player); // there is log inside setItemLooted function for all types of item loots - master loot, group loot, etc.
            loot->LogLooted(item, player, false);
        }
        else // and there is only one way to loot "party loot" (like badges) - it is here - so work it out here
            loot->LogLooted(item, player, true);

        --loot->unlootedCount;

        // LootItem is being removed (looted) from the container, delete it from the DB.
        if (loot->containerID > 0)
            loot->DeleteLootItemFromContainerItemDB(itemid);

        player->SendNewItem(newitem, uint32(count), false, false, true);

		if (sWorld.isEasyRealm())
		{
			// BOJ case
            if (itemid == SECOND_BADGE)
            {
                if (boj_replaced)
                    ChatHandler(_player).SendSysMessage(15492);
            }
            else if (itemid == 29434)
                loot->AddItemTakerHash(ITEM_BADGE, hash);
            else if (is_boss_soul)
                loot->AddItemTakerHash(itemid, hash);
            else if (itemid == ITEM_EMBLEM_OF_TRIUMPH)
                loot->AddItemTakerHash(ITEM_EMBLEM_OF_TRIUMPH, hash);

			// save raidchest loot for disenchant
			if (newitem && newitem->GetGUIDLow() && loot->m_source == SOURCE_ITEM && IsRaidChest(loot->m_from_entry))
			{
				static SqlStatementID raidchestloot;

				SqlStatement stmt = RealmDataDatabase.CreateStatement(raidchestloot, "INSERT INTO character_raidchest VALUES (?, ?, ?, ?, 0)");
				stmt.addUInt32(newitem->GetGUIDLow()); // item_guid
				stmt.addUInt32(player->GetGUIDLow()); // owner_guid
				stmt.addUInt32(itemid); // item_entry
				stmt.addUInt32(loot->m_from_entry); // chest
				stmt.Execute();

				player->raidChestLoot.insert({ newitem->GetGUIDLow(), loot->m_from_entry });
			}
		}
    }
    else
        player->SendEquipError(msg, NULL, NULL);
}

void WorldSession::HandleLootMoneyOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: CMSG_LOOT_MONEY");

    Player *player = GetPlayer();
    uint64 guid = player->GetLootGUID();
    if (!guid)
        return;

    Loot *pLoot = NULL;

    switch (GUID_HIPART(guid))
    {
        case HIGHGUID_GAMEOBJECT:
        {
            GameObject *pGameObject = GetPlayer()->GetMap()->GetGameObject(guid);

            // not check distance for GO in case owned GO (fishing bobber case, for example)
            if (pGameObject && (pGameObject->GetOwnerGUID()==_player->GetGUID() || pGameObject->IsWithinDistInMap(_player,INTERACTION_DISTANCE)))
                pLoot = &pGameObject->loot;

            break;
        }
        case HIGHGUID_CORPSE:                               // remove insignia ONLY in BG
        {
            Corpse *bones = ObjectAccessor::GetCorpse(*GetPlayer(), guid);

            if (bones && bones->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
                pLoot = &bones->loot;

            break;
        }
        case HIGHGUID_ITEM:
        {
            if (Item *item = GetPlayer()->GetItemByGuid(guid))
                pLoot = &item->loot;
            break;
        }
        case HIGHGUID_UNIT:
        {
            Creature* pCreature = GetPlayer()->GetMap()->GetCreature(guid);

            bool ok_loot = pCreature && pCreature->isAlive() == (player->GetClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);

            if (ok_loot && pCreature->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
                pLoot = &pCreature->loot ;

            break;
        }
        default:
            return;                                         // unlootable type
    }

    if (pLoot)
    {
        if (!pLoot->IsPlayerAllowedToLoot(player, NULL))
        {
            sLog.outLog(LOG_CHEAT, "HandleLootMoneyOpcode - player %s(%u) is trying to loot money (Hi %04X En %u Lo %u) but he is not allowed to",
                player->GetName(), player->GetGUIDLow(), GUID_HIPART(guid), GUID_ENPART(guid), GUID_LOPART(guid));
            player->SendLootRelease(guid);
            return;
        }

        if (!IS_ITEM_GUID(guid) && player->GetGroup())      //item can be looted only single player
        {
            Group *group = player->GetGroup();

            std::vector<Player*> playersNear;
            for (GroupReference *itr = group->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* playerGroup = itr->getSource();
                if (!playerGroup)
                    continue;
                if (player->GetDistance(playerGroup) < sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE))
                    playersNear.push_back(playerGroup);
            }

            uint32 money_per_player = uint32((pLoot->gold)/(playersNear.size()));

            for (std::vector<Player*>::iterator i = playersNear.begin(); i != playersNear.end(); ++i)
            {
                (*i)->ModifyMoney(money_per_player);
                //Offset surely incorrect, but works
                WorldPacket data(SMSG_LOOT_MONEY_NOTIFY, 4);
                data << uint32(money_per_player);
                (*i)->SendPacketToSelf(&data);
            }
        }
        else
            player->ModifyMoney(pLoot->gold);

        pLoot->gold = 0;
        pLoot->NotifyMoneyRemoved();

        // Delete the money loot record from the DB
        if (pLoot->containerID > 0)
            pLoot->DeleteLootMoneyFromContainerItemDB();

        // Delete container if empty
        if (pLoot->isLooted() && IS_ITEM_GUID(guid))
            player->GetSession()->DoLootRelease(guid);
    }
}

void WorldSession::HandleLootOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("WORLD: CMSG_LOOT");

    uint64 guid;
    recv_data >> guid;

    GetPlayer()->SendLoot(guid, LOOT_CORPSE);
}

void WorldSession::HandleLootReleaseOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("WORLD: CMSG_LOOT_RELEASE");

    // cheaters can modify lguid to prevent correct apply loot release code and re-loot
    // use internal stored guid
    //uint64   lguid;
    //recv_data >> lguid;
    GetPlayer()->SendCombatStats(1 << COMBAT_STATS_LOOTING, "received loot release opcode", NULL);


    if (uint64 lguid = GetPlayer()->GetLootGUID())
        DoLootRelease(lguid);
}

void WorldSession::DoLootRelease(uint64 lguid)
{
    Player  *player = GetPlayer();
    Loot    *loot;
    
    player->SetLootGUID(0);
    player->SendLootRelease(lguid);

    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);

    if (!player->IsInWorld())
        return;

    if (IS_GAMEOBJECT_GUID(lguid))
    {
        GameObject *go = GetPlayer()->GetMap()->GetGameObject(lguid);

        // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
        if (!go || (go->GetOwnerGUID() != _player->GetGUID() && go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE) && !go->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
            return;

        loot = &go->loot;

        if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR)
        {
            // locked doors are opened with spelleffect openlock, prevent remove its as looted
            go->UseDoorOrButton();
        }
        else if (loot->isLooted() || go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
        {
            // GO is mineral vein? so it is not removed after its looted
            if (go->GetGoType() == GAMEOBJECT_TYPE_CHEST)
            {
                uint32 go_min = go->GetGOInfo()->chest.minSuccessOpens;
                uint32 go_max = go->GetGOInfo()->chest.maxSuccessOpens;

                // only vein pass this check
                if (go_min != 0 && go_max > go_min)
                {
                    float amount_rate = 1;
                    float min_amount = go_min * amount_rate;
                    float max_amount = go_max * amount_rate;

                    float uses = float(go->GetUseCount());

                    if (uses < max_amount)
                    {
                        if (uses >= min_amount)
                        {
                            float chance_rate = 1;

                            int32 ReqValue = 175;
                            LockEntry const* lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->chest.lockId);
                            if (lockInfo)
                                ReqValue = lockInfo->Skill[3]; // mining
                            float skill = float(player->GetSkillValue(SKILL_MINING)) / (ReqValue + 25);
                            double chance = pow(0.8 * chance_rate, 4 * (1 / double(max_amount)) * double(uses));
                            if (roll_chance_f(100 * chance + skill))
                                go->SetLootState(GO_READY);
                            else                            // not have more uses
                                go->SetLootState(GO_JUST_DEACTIVATED);
                        }
                        else                                // 100% chance until min uses
                            go->SetLootState(GO_READY);
                    }
                    else                                    // max uses already
                        go->SetLootState(GO_JUST_DEACTIVATED);
                }
                else                                        // not vein
                    go->SetLootState(GO_JUST_DEACTIVATED);
            }
            else if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
            {                                               // The fishing hole used once more
                go->AddUse();                               // if the max usage is reached, will be despawned in next tick
                if (go->GetUseCount() >= irand(go->GetGOInfo()->fishinghole.minSuccessOpens, go->GetGOInfo()->fishinghole.maxSuccessOpens))
                {
                    go->SetLootState(GO_JUST_DEACTIVATED);
                }
                else
                    go->SetLootState(GO_READY);
            }
            else
                go->SetLootState(GO_JUST_DEACTIVATED);

            loot->clear();
        }
        else
            go->SetLootState(GO_ACTIVATED);

        if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE) // not sure if this should exist
        {
            go->SetLootState(GO_JUST_DEACTIVATED);
            loot->clear();
        }
    }
    else if (IS_CORPSE_GUID(lguid))        // ONLY remove insignia at BG
    {
        Corpse *corpse = ObjectAccessor::GetCorpse(*player, lguid);
        if (!corpse || !corpse->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
            return;

        loot = &corpse->loot;

        if (loot->isLooted())
        {
            loot->clear();
            corpse->RemoveFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);
        }
    }
    else if (IS_ITEM_GUID(lguid))
    {
        Item *pItem = player->GetItemByGuid(lguid);
        if (!pItem)
            return;
        loot = &pItem->loot;

        ItemPrototype const* proto = pItem ? pItem->GetProto() : NULL;

		// save custom chest loot
		// save disenchanted fragments
		if (sWorld.isEasyRealm() && proto)
		{
			bool autostore = false;
			
			if (proto->Flags & ITEM_FLAGS_CUSTOM)
				autostore = true;
			else
			{
                for (const auto& chestKeys : ChestsRequiredKeys)
                {
                    if (chestKeys.chest == pItem->GetEntry())
                    {
                        autostore = true;
                        break;
                    }
                }
			}

			if (autostore)
			{
				player->SetLootGUID(lguid);
				for (uint8 lootSlot = 0; lootSlot < pItem->loot.items.size(); lootSlot++)
				{
					QuestItem *qitem = NULL;
					QuestItem *ffaitem = NULL;

					LootItem *item = pItem->loot.LootItemInSlot(lootSlot, player, &qitem, &ffaitem);

					if (!item)
						continue;

					WorldPacket lootPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
					lootPacket << lootSlot;
					HandleAutostoreLootItemOpcode(lootPacket);
				}
				player->SetLootGUID(0);
			}
		}

        // destroy only 5 items from stack in case prospecting and milling
        if (proto && (proto->BagFamily & BAG_FAMILY_MASK_MINING_SUPP) && proto->Class == ITEM_CLASS_TRADE_GOODS)
        {
            pItem->m_lootGenerated = false;
            loot->clear();

            uint32 count = 5;
            player->DestroyItemCount(pItem, count, true, "LOOT_RELEASE_DESTROY");
        }
        else
        {
            // Only delete item if no loot or money or player closed loot menu.
            if (pItem->loot.isLooted() || !(pItem->GetProto()->Flags & ITEM_FLAGS_OPENABLE))
                player->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true, "LOOT_RELEASE_DESTROY");
            /*if (!loot->isLooted())
            {
                LootItem* li = loot->LootItemInSlot(0);
                if (li && ObjectMgr::GetItemPrototype(li->itemid)->Class == ITEM_CLASS_QUEST)
                {
                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, li->itemid, li->count);
                    if (!li->is_looted && !li->is_blocked && msg == EQUIP_ERR_OK)
                        player->StoreNewItem(dest, li->itemid, true, li->randomPropertyId);
                }
            }
            player->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
            */
        }
        return;                                             // item can be looted only single player
    }
    else
    {
        Creature* pCreature = GetPlayer()->GetMap()->GetCreature(lguid);

        bool ok_loot = pCreature && pCreature->isAlive() == (player->GetClass()==CLASS_ROGUE && pCreature->lootForPickPocketed);
        if (!ok_loot || !pCreature->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
            return;

        loot = &pCreature->loot;

        if (player->GetGUID() == loot->looterGUID)
        {
            loot->looterGUID = 0;
            if (Group *group = player->GetGroup())
                group->SendRoundRobin(loot, pCreature);
        }

        if (loot->isLooted())
        {
            // skip pickpocketing loot for speed, skinning timer redunction is no-op in fact
            if (!pCreature->isAlive())
                pCreature->AllLootRemovedFromCorpse();

            pCreature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            loot->clear();
        }
    }

    //Player is not looking at loot list, he doesn't need to see updates on the loot list
    loot->RemoveLooter(player->GetGUID());

    // should be fixed after some research in loot implementation etc
//    loot->RemoveQuestLoot(player);
}

void WorldSession::HandleLootMasterGiveOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+1+8);

    uint8 slotid;
    uint64 lootguid, target_playerguid;

    recv_data >> lootguid >> slotid >> target_playerguid;

    if (!_player->GetGroup() || _player->GetGroup()->GetLooterGuid() != _player->GetGUID())
    {
        _player->SendLootRelease(lootguid);
        return;
    }

    Player *target = ObjectAccessor::GetPlayerInWorld(MAKE_NEW_GUID(target_playerguid, 0, HIGHGUID_PLAYER));
    if (!target)
        return;

    sLog.outDebug("WorldSession::HandleLootMasterGiveOpcode (CMSG_LOOT_MASTER_GIVE, 0x02A3) Target = [%s].", target->GetName());

    if (_player->GetLootGUID() != lootguid)
        return;

    if (_player->GetInstanciableInstanceId() != target->GetInstanciableInstanceId())
        return;

    Loot *pLoot = NULL;

    uint32 logLootBossEntry = 0;
    if (IS_CREATURE_GUID(GetPlayer()->GetLootGUID()))
    {
        Creature *pCreature = GetPlayer()->GetMap()->GetCreature(lootguid);
        if (!pCreature)
            return;

        logLootBossEntry = pCreature->GetEntry();
        pLoot = &pCreature->loot;
    }
    else if (IS_GAMEOBJECT_GUID(GetPlayer()->GetLootGUID()))
    {
        GameObject *pGO = GetPlayer()->GetMap()->GetGameObject(lootguid);
        if (!pGO)
            return;

        pLoot = &pGO->loot;
    }

    if (!pLoot)
        return;

    if (slotid > pLoot->items.size())
    {
        sLog.outDebug("AutoLootItem: Player %s might be using a hack! (slot %d, size %llu)",GetPlayer()->GetName(), slotid, pLoot->items.size());
        return;
    }

    if (!pLoot->IsPlayerAllowedToLoot(target, NULL))
    {
        std::ostringstream str;
        str << "HandleLootMasterGiveOpcode - player " << _player->GetName() << " (GUID: " << _player->GetGUIDLow();
        if (slotid < pLoot->items.size())
            str << ") is trying to give item " << pLoot->items[slotid].itemid << " from ";
        else
            str << ") is trying to give quest item (O_o) from ";
        str << (IS_GAMEOBJECT_GUID(lootguid) ? "gobject" : "creature");
        str << " Entry " << GUID_ENPART(lootguid) << " LowGUID " << GUID_LOPART(lootguid) << "but he is not alowed to do so.\n";
        str << "MapID: " << _player->GetMapId() << " InstanceID: " << _player->GetInstanciableInstanceId() << " position X: ";
        str << _player->GetPositionX() << " Y: " << _player->GetPositionY() << " Z: " << _player->GetPositionZ() << " To ";
        str << target->GetName() << " (GUID: " << target->GetGUIDLow() << ")";
        sLog.outLog(LOG_CHEAT, "%s", str.str().c_str());
        _player->SendLootRelease(lootguid);
        return;
    }

    LootItem& item = pLoot->items[slotid];
    if (!item.AllowedForPlayer(target, 4))
    {
        _player->SendEquipError(EQUIP_ERR_ITEM_LOCKED,NULL,NULL); // cannot give this to him, sorry
        return;
    }
    ItemPosCountVec dest;
    uint8 msg = target->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item.itemid, item.count);
    if (msg != EQUIP_ERR_OK)
    {
        target->SendEquipError(msg, NULL, NULL);
        _player->SendEquipError(msg, NULL, NULL);         // send duplicate of error massage to master looter
        return;
    }

    // not move item from loot to target inventory
    Item * newitem = target->StoreNewItem(dest, item.itemid, true, item.randomPropertyId, "LOOT");
    target->SendNewItem(newitem, uint32(item.count), false, false, true);

    target->SaveToDB();

    //log
	_player->LogLootMasterGive(target, item.itemid);

    // mark as looted
    item.count=0;

    pLoot->setItemLooted(&item,target);
    pLoot->NotifyItemRemoved(slotid);
    --pLoot->unlootedCount;
}

