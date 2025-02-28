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
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "SpellAuras.h"
#include "UpdateMask.h"
#include "ScriptMgr.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "MapManager.h"
#include "Pet.h"
#include "BattleGroundMgr.h"
#include "BattleGround.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Spell.h"

void WorldSession::HandleTabardVendorActivateOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;
    recv_data >> guid;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TABARDDESIGNER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleTabardVendorActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    SendTabardVendorActivate(guid);
}

void WorldSession::SendTabardVendorActivate(uint64 guid)
{
    WorldPacket data(MSG_TABARDVENDOR_ACTIVATE, 8);
    data << guid;
    SendPacket(&data);
}

void WorldSession::HandleBankerActivateOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;

    sLog.outDebug( "WORLD: Received CMSG_BANKER_ACTIVATE");

    recv_data >> guid;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_BANKER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleBankerActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    SendShowBank(guid);
}

void WorldSession::SendShowBank(uint64 guid)
{
    WorldPacket data(SMSG_SHOW_BANK, 8);
    data << guid;
    SendPacket(&data);
}

void WorldSession::HandleTrainerListOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 guid;

    recv_data >> guid;
    SendTrainerList(guid);
}

void WorldSession::SendTrainerList(uint64 guid)
{
    std::string str = GetHellgroundString(LANG_NPC_TAINER_HELLO);
    SendTrainerList(guid, str);
}

void WorldSession::SendTrainerList(uint64 guid, const std::string& strTitle)
{
    sLog.outDebug("WORLD: SendTrainerList");

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog.outDebug("WORLD: SendTrainerList - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // trainer list loaded at check;
    if (!unit->isCanTrainingOf(_player,true))
        return;

    CreatureInfo const *ci = unit->GetCreatureInfo();

    if (!ci)
    {
        sLog.outDebug("WORLD: SendTrainerList - (GUID: %u) NO CREATUREINFO!",GUID_LOPART(guid));
        return;
    }

    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
    {
        sLog.outDebug("WORLD: SendTrainerList - Training spells not found for creature (GUID: %u Entry: %u)",
            GUID_LOPART(guid), unit->GetEntry());
        return;
    }

    WorldPacket data(SMSG_TRAINER_LIST, 8+4+4+trainer_spells->spellList.size()*38 + strTitle.size()+1);
    data << guid;
    data << uint32(trainer_spells->trainerType);

    size_t count_pos = data.wpos();
    data << uint32(trainer_spells->spellList.size());

    // reputation discount
    float fDiscountMod = _player->GetReputationPriceDiscount(unit);
    _player->ApplyTrainerPriceDiscount((TrainerType)ci->trainer_type, fDiscountMod);

    uint32 count = 0;
    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
    {
        TrainerSpell const* tSpell = &itr->second;

        if (!_player->IsSpellFitByClassAndRace(tSpell->spell))
            continue;

        ++count;

        bool primary_prof_first_rank = sSpellMgr.IsPrimaryProfessionFirstRankSpell(tSpell->spell);

        SpellChainNode const* chain_node = sSpellMgr.GetSpellChainNode(tSpell->spell);
        uint32 req_spell = sSpellMgr.GetSpellRequired(tSpell->spell);

        data << uint32(tSpell->spell);
        data << uint8(_player->GetTrainerSpellState(tSpell));
        data << uint32(floor(tSpell->spellCost * fDiscountMod));

        data << uint32(primary_prof_first_rank ? 1 : 0);    // primary prof. learn confirmation dialog
        data << uint32(primary_prof_first_rank ? 1 : 0);    // must be equal prev. field to have learn button in enabled state
        data << uint8(tSpell->reqLevel);
        data << uint32(tSpell->reqSkill);
        data << uint32(tSpell->reqSkillValue);
        data << uint32(chain_node && chain_node->prev ? chain_node->prev : req_spell);
        data << uint32(chain_node && chain_node->prev ? req_spell : 0);
        data << uint32(0);
    }

    data << strTitle;

    data.put<uint32>(count_pos,count);
    SendPacket(&data);
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4);

    uint64 guid;
    uint32 spellId = 0;

    recv_data >> guid >> spellId;
    sLog.outDebug("WORLD: Received CMSG_TRAINER_BUY_SPELL NpcGUID=%u, learn spell id is: %u",uint32(GUID_LOPART(guid)), spellId);

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_TRAINER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleTrainerBuySpellOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    if (!unit->isCanTrainingOf(_player,true))
        return;

    // check present spell in trainer spell list
    TrainerSpellData const* trainer_spells = unit->GetTrainerSpells();
    if (!trainer_spells)
        return;

    // not found, cheat?
    TrainerSpell const* trainer_spell = trainer_spells->Find(spellId);
    if (!trainer_spell)
        return;

    // can't be learn, cheat? Or double learn with lags...
    if (_player->GetTrainerSpellState(trainer_spell) != TRAINER_SPELL_GREEN)
        return;

    CreatureInfo const *ci = unit->GetCreatureInfo();

    if (!ci)
    {
        sLog.outDebug("WORLD: SendTrainerList - (GUID: %u) NO CREATUREINFO!", GUID_LOPART(guid));
        return;
    }

    float discount = _player->GetReputationPriceDiscount(unit);
    _player->ApplyTrainerPriceDiscount((TrainerType)ci->trainer_type, discount);

    // apply reputation discount
    uint32 nSpellCost = uint32(floor(trainer_spell->spellCost * discount));

    // check money requirement
    if (_player->GetMoney() < nSpellCost)
        return;

    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);           // visual effect on trainer
    data << uint64(guid) << uint32(0xB3);
    _player->BroadcastPacket(&data, true);

    data.Initialize(SMSG_PLAY_SPELL_VISUAL, 12);            // visual effect on player
    data << uint64(_player->GetGUID()) << uint32(0x016A);
    _player->BroadcastPacket(&data, true);

    _player->ModifyMoney(-int32(nSpellCost));

    // learn explicitly to prevent lost money at lags, learning spell will be only show spell animation
    _player->learnSpell(trainer_spell->spell);

    data.Initialize(SMSG_TRAINER_BUY_SUCCEEDED, 12);
    data << uint64(guid) << uint32(spellId);
    SendPacket(&data);
}

void WorldSession::HandleGossipHelloOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug( "WORLD: Received CMSG_GOSSIP_HELLO");

    uint64 guid;
    recv_data >> guid;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    GetPlayer()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
    // remove fake death
    //if(GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
    //    GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    if (unit->isArmorer() || unit->isCivilian() || unit->isQuestGiver() || unit->isServiceProvider() || unit->isGuard())
    {
        unit->StopMoving();
    }

    // If spirit guide, no need for gossip menu, just put player into resurrect queue
    if (unit->isSpiritGuide())
    {
        BattleGround *bg = _player->GetBattleGround();
        if (bg)
        {
            bg->AddPlayerToResurrectQueue(unit->GetGUID(), _player->GetGUID());
            sBattleGroundMgr.SendAreaSpiritHealerQueryOpcode(_player, bg, unit->GetGUID());
            return;
        }
    }

    if (!sScriptMgr.OnGossipHello(_player, unit))
    {
        unit->prepareGossipMenu(_player);
        unit->sendPreparedGossip(_player);
    }

    _player->TalkedToCreature(unit->GetEntry(), unit->GetGUID());
}

void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8+4+4);

    sLog.outDebug("WORLD: CMSG_GOSSIP_SELECT_OPTION");

    uint32 option;
    uint32 unk;
    uint64 guid;
    std::string code = "";
    SpellCastTargets targets;

    recv_data >> guid >> unk >> option;

    if (_player->PlayerTalkClass->GossipOptionCoded(option))
    {
        // recheck
        CHECK_PACKET_SIZE(recv_data,8+4+1);
        sLog.outBasic("reading string");
        recv_data >> code;
        sLog.outBasic("string read: %s", code.c_str());
    }

    Creature *unit = NULL;
    GameObject *go = NULL;
    Item *pItem = NULL;

    uint32 sender = _player->PlayerTalkClass->GossipOptionSender(option);
    uint32 action = _player->PlayerTalkClass->GossipOptionAction(option);

    if (IS_CREATURE_GUID(guid) || IS_PET_GUID(guid))
    {
        unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE);
        if (!unit)
        {
            sLog.outDebug("WORLD: HandleGossipSelectOptionOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else if (IS_GAMEOBJECT_GUID(guid))
    {
        go = _player->GetMap()->GetGameObject(guid);
        if (!go)
        {
            sLog.outDebug("WORLD: HandleGossipSelectOptionOpcode - GameObject (GUID: %u) not found.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else if (IS_ITEM_GUID(guid))
    {
        pItem = _player->GetItemByGuid(guid);
        if (!pItem)
        {
            sLog.outDebug("WORLD: HandleGossipSelectOptionOpcode - Item (GUID: %u) not found.", uint32(GUID_LOPART(guid)));
            return;
        }
    }
    else if (IS_PLAYER_GUID(guid))
    {
        if (!_player->last_script_id)
            return;

        sScriptMgr.OnGossipSelect(_player, _player->last_script_id, sender, action, code.empty() ? nullptr : code.c_str(), _player->last_script_option_id);
        return;
    }
    else
    {
        sLog.outDebug("WORLD: HandleGossipSelectOptionOpcode - unsupported GUID type for highguid %u. lowpart %u.", uint32(GUID_HIPART(guid)), uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    if (unit)
    {
        if (!sScriptMgr.OnGossipSelect(_player, unit, sender, action, code.empty() ? NULL : code.c_str()))
            unit->OnGossipSelect(_player, option);
    }
    else if (pItem)
    {
        sScriptMgr.OnGossipSelectItem(_player, pItem, sender, action, targets, code.empty() ? NULL : code.c_str());
    }
    else
        sScriptMgr.OnGossipSelect(_player, go, sender, action, code.empty() ? NULL : code.c_str());
}

void WorldSession::HandleSpiritHealerActivateOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("WORLD: CMSG_SPIRIT_HEALER_ACTIVATE");

    uint64 guid;

    recv_data >> guid;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_SPIRITHEALER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleSpiritHealerActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    SendSpiritResurrect();
}

void WorldSession::SendSpiritResurrect()
{
    _player->ResurrectPlayer(0.5f, true);

    if (sWorld.getConfig(CONFIG_DURABILITY_LOSS_ON_DEATH))
        _player->DurabilityLossAll(0.25f,true);

    // get corpse nearest graveyard
    WorldSafeLocsEntry const *corpseGrave = NULL;
    Corpse *corpse = _player->GetCorpse();
    if (corpse)
        corpseGrave = sObjectMgr.GetClosestGraveYard(
            corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetMapId(), _player->GetTeam());

    // now can spawn bones
    _player->SpawnCorpseBones();

    // teleport to nearest from corpse graveyard, if different from nearest to player ghost
    if (corpseGrave)
    {
        WorldSafeLocsEntry const *ghostGrave = sObjectMgr.GetClosestGraveYard(
            _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId(), _player->GetTeam());

        if (corpseGrave != ghostGrave)
            _player->TeleportTo(corpseGrave->map_id, corpseGrave->x, corpseGrave->y, corpseGrave->z, _player->GetOrientation());
    }

    _player->SaveToDB();
    _player->UpdateVisibilityAndView();
}

void WorldSession::HandleBinderActivateOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    uint64 npcGUID;
    recv_data >> npcGUID;

    if (!GetPlayer()->IsInWorld() || !GetPlayer()->isAlive())
        return;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_INNKEEPER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleBinderActivateOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    SendBindPoint(unit);
}

void WorldSession::SendBindPoint(Creature* npc, float pos_x, float pos_y, float pos_z, uint16 zone, uint32 map)
{
    if (GetPlayer()->GetMap()->Instanceable())
        return;

    uint32 bindspell = 3286;

    float x = pos_x ? pos_x : _player->GetPositionX();
    float y = pos_y ? pos_y : _player->GetPositionY();
    float z = pos_z ? pos_z : _player->GetPositionZ();
    uint16 set_zone = zone ? zone : _player->GetCachedZone();
    uint32 set_map = map ? map : _player->GetMapId();

    // update sql homebind
    RealmDataDatabase.PExecute("UPDATE character_homebind SET map = '%u', zone = '%u', position_x = '%f', position_y = '%f', position_z = '%f' WHERE guid = '%u'", set_map, set_zone, x, y, z, _player->GetGUIDLow());
    _player->m_homebindMapId = set_map;
    _player->m_homebindZoneId = set_zone;
    _player->m_homebindX = x;
    _player->m_homebindY = y;
    _player->m_homebindZ = z;

    // send spell for bind 3286 bind magic
    npc->CastSpell(_player, bindspell, true);

    WorldPacket data(SMSG_TRAINER_BUY_SUCCEEDED, (8 + 4));
    data << npc->GetGUID();
    data << bindspell;
    SendPacket(&data);

    // binding
    data.Initialize(SMSG_BINDPOINTUPDATE, (4 + 4 + 4 + 4 + 4));
    data << float(x);
    data << float(y);
    data << float(z);
    data << uint32(set_map);
    data << uint32(set_zone);
    SendPacket(&data);

    debug_log("New Home Position X is %f", x);
    debug_log("New Home Position Y is %f", y);
    debug_log("New Home Position Z is %f", z);
    debug_log("New Home MapId is %u", set_map);
    debug_log("New Home ZoneId is %u", set_zone);

    // zone update
    data.Initialize(SMSG_PLAYERBOUND, 8 + 4);
    data << uint64(_player->GetGUID());
    data << uint32(set_zone);
    SendPacket(&data);

    _player->PlayerTalkClass->CloseGossip();
}

void WorldSession::HandleListStabledPetsOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data,8);

    sLog.outDebug("WORLD: Recv MSG_LIST_STABLED_PETS");
    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size
    uint64 npcGUID;

    recv_data >> npcGUID;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_STABLEMASTER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleListStabledPetsOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)));
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    //sLog.outLog(LOG_SPECIAL,"StablePetLog: HandleListStabledPetsOpcode: PlayerGUID %u, NPCGuid %u", GetPlayer()->GetGUIDLow(), unit->GetGUIDLow());
    SendStablePet(npcGUID);
}

void WorldSession::SendStablePet(uint64 guid)
{
    sLog.outDebug("WORLD: Recv MSG_LIST_STABLED_PETS Send.");

    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    WorldPacket data(MSG_LIST_STABLED_PETS, 200);           // guess size
    data << uint64 (guid);

    Pet *pet = _player->GetPet();

    size_t wpos = data.wpos();
    data << uint8(0);                                       // place holder for slot show number

    data << uint8(GetPlayer()->m_stableSlots);

    uint8 num = 0;                                          // counter for place holder
    PetSaveMode firstSlot = PET_SAVE_IN_STABLE_SLOT_1;      // have to be changed to PET_SAVE_AS_CURRENT if pet is currently temp unsummoned

    // not let move dead pet in slot
    if (pet && pet->isAlive() && pet->getPetType() == HUNTER_PET)
    {
        //sLog.outLog(LOG_SPECIAL,"StablePetLog: SendStablePet: 1, PlayerGUID %u, pet %u", GetPlayer()->GetGUIDLow(), pet->GetGUIDLow());
        data << uint32(pet->GetCharmInfo()->GetPetNumber());
        data << uint32(pet->GetEntry());
        data << uint32(pet->GetLevel());
        data << pet->GetName();                             // petname
        data << uint32(pet->GetLoyaltyLevel());             // loyalty
        data << uint8(0x01);                                // client slot 1 == current pet (0)
        ++num;
    }
    else
    {
        if(_player->GetTemporaryUnsummonedPetNumber()) // temporary unsummon - mount
            firstSlot = PET_SAVE_AS_CURRENT;
        else
        {
            QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT owner, id, entry, level, name, loyalty FROM character_pet WHERE owner = '%u' AND slot = '%u' ORDER BY slot",
                _player->GetGUIDLow(), uint32(PET_SAVE_NOT_IN_SLOT));

            if (result) // dismissed pet
            {
                do
                {
                    Field* fields = result->Fetch();

                    data << uint32(fields[1].GetUInt32());          // petnumber
                    data << uint32(fields[2].GetUInt32());          // creature entry
                    data << uint32(fields[3].GetUInt32());          // level
                    data << fields[4].GetString();                  // name
                    data << uint32(fields[5].GetUInt32());          // loyalty
                    data << uint8(0x01);       // slot

                    ++num;
                } while (result->NextRow());
            }
        }
    }

    //                                                            0      1     2   3      4      5        6
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT owner, slot, id, entry, level, loyalty, name FROM character_pet WHERE owner = '%u' AND slot > 0 AND slot < 3 ORDER BY slot",_player->GetGUIDLow());

    if (result)
    {
        // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
        if (GetPlayer()->IsMounted())
            GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        do
        {
            Field *fields = result->Fetch();

            data << uint32(fields[2].GetUInt32());          // petnumber
            data << uint32(fields[3].GetUInt32());          // creature entry
            data << uint32(fields[4].GetUInt32());          // level
            data << fields[6].GetString();                  // name
            data << uint32(fields[5].GetUInt32());          // loyalty
            data << uint8(fields[1].GetUInt32()+1);         // slot

            ++num;
        }while (result->NextRow());
    }
    //sLog.outLog(LOG_SPECIAL,"StablePetLog: SendStablePet: 2, PlayerGUID %u", GetPlayer()->GetGUIDLow());

    data.put<uint8>(wpos, num);                                // set real data to placeholder
    SendPacket(&data);
}

void WorldSession::HandleStablePet(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    sLog.outDebug("WORLD: Recv CMSG_STABLE_PET");
    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    uint64 npcGUID;

    recv_data >> npcGUID;

    if (!GetPlayer()->isAlive())
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_STABLEMASTER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleStablePet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)));
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    Pet *pet = _player->GetPet();

    // can't place in stable dead pet
   /* if (!pet || !pet->isAlive() || pet->getPetType() != HUNTER_PET)
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }*/
    if(pet)
    {
        bool stop = false;
        if (!pet->isAlive())
        {
            _player->SendPetTameFailure(PETTAME_DEAD);
            stop = true;
        }

        if (!stop && pet->getPetType() != HUNTER_PET)
        {
            _player->SendPetTameFailure(PETTAME_INVALIDCREATURE);
            stop = true;
        }

        if (stop)
        {
            data << uint8(0x06);
            SendPacket(&data);
            return;
        }
    }
    else
    {
        SpellCastResult loadResult = Pet::TryLoadFromDB(_player, 0, 0, _player->GetTemporaryUnsummonedPetNumber() != 0, HUNTER_PET);
        if (loadResult != SPELL_CAST_OK)
        {
            if (loadResult == SPELL_FAILED_TARGETS_DEAD)
                _player->SendPetTameFailure(PETTAME_DEAD);

            if (loadResult == SPELL_FAILED_BAD_TARGETS)
                _player->SendPetTameFailure(PETTAME_INVALIDCREATURE);

            data << uint8(0x06);
            SendPacket(&data);
            return;
        }
    }

    uint32 free_slot = 1;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT owner,slot,id FROM character_pet WHERE owner = '%u'  AND slot > 0 AND slot < 3 ORDER BY slot ",_player->GetGUIDLow());
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 slot = fields[1].GetUInt32();

            // slots ordered in query, and if not equal then free
            if (slot != free_slot)
                break;

            // this slot not free, skip
            ++free_slot;
        }while (result->NextRow());
    }

    if (free_slot > 0 && free_slot <= GetPlayer()->m_stableSlots)
    {
        if(pet)
            _player->RemovePet(pet, PetSaveMode(free_slot));
        else
        {
            RealmDataDatabase.BeginTransaction();
            static SqlStatementID ChangePetSlot_ID;
            SqlStatement ChangePetSlot = RealmDataDatabase.CreateStatement(ChangePetSlot_ID, "UPDATE character_pet SET slot = ? WHERE owner = ? AND slot = ? ");
            ChangePetSlot.PExecute(free_slot, _player->GetObjectGuid().GetCounter(), uint32(_player->GetTemporaryUnsummonedPetNumber() ? PET_SAVE_AS_CURRENT : PET_SAVE_NOT_IN_SLOT));
            RealmDataDatabase.CommitTransaction();
        }
        data << uint8(0x08);
        _player->SetTemporaryUnsummonedPetNumber(0);
    }
    else
        data << uint8(0x06);

    SendPacket(&data);
}

void WorldSession::HandleUnstablePet(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+4);

    sLog.outDebug("WORLD: Recv CMSG_UNSTABLE_PET.");
    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size
    uint64 npcGUID;
    uint32 petnumber;

    recv_data >> npcGUID >> petnumber;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_STABLEMASTER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleUnstablePet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)));
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    /*Pet* pet = _player->GetPet();
    if (pet && pet->isAlive())
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    // delete dead pet
    if (pet)
        _player->RemovePet(pet,PET_SAVE_AS_DELETED);
    */
    uint32 creature_id = 0;
    uint32 slot = 0;
    // Pet *newpet = NULL;

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT entry, slot FROM character_pet WHERE owner = '%u' AND id = '%u' AND slot > 0 AND slot < 3",_player->GetGUIDLow(),petnumber);

    if (result)
    {
        Field *fields = result->Fetch();
        creature_id = fields[0].GetUInt32();
        slot = fields[1].GetUInt32();

        /*newpet = new Pet(HUNTER_PET);
        if (!newpet->LoadPetFromDB(_player,petentry,petnumber))
        {
            delete newpet;
            newpet = NULL;
        }*/
    }

    if (!creature_id)
    {
        sLog.outLog(LOG_DB_ERR, "HandleUnstablePet: %s trying to unstable a pet with invalid creature id!", _player->GetGuidStr().c_str());
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    if (slot == uint32(PET_SAVE_AS_CURRENT))
    {
        sLog.outLog(LOG_DB_ERR, "HandleUnstablePet: %s trying to unstable current pet!", _player->GetGuidStr().c_str());
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    CreatureInfo const* creatureInfo = ObjectMgr::GetCreatureTemplate(creature_id);
    if (!creatureInfo || !creatureInfo->isTameable())
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }
    
    Pet* pet = _player->GetPet();
    if(pet)
    {
        bool stop = false;

        if (!pet->isAlive())
        {
            _player->SendPetTameFailure(PETTAME_DEAD);
            stop = true;
        }

        if (!stop && pet->getPetType() != HUNTER_PET)
        {
            _player->SendPetTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
            stop = true;
        }

        if (stop)
        {
            data << uint8(0x06);
            SendPacket(&data);
            return;
        }

        _player->RemovePet(pet, PetSaveMode(slot));
    }
    else
    {
        // try to find if pet is actually temporary unsummoned
        SpellCastResult loadResult = Pet::TryLoadFromDB(_player, 0, 0, _player->GetTemporaryUnsummonedPetNumber() != 0, HUNTER_PET);
        if (loadResult != SPELL_CAST_OK)
        {
            if (loadResult == SPELL_FAILED_TARGETS_DEAD)
            {
                _player->SendPetTameFailure(PETTAME_DEAD);

                sLog.outLog(LOG_DB_ERR, "UnStablePet: trying to unstable a dead pet. Should not be possible!");
                return;
            }

            if (loadResult == SPELL_FAILED_BAD_TARGETS)
            {
                _player->SendPetTameFailure(PETTAME_ANOTHERSUMMONACTIVE);
                data << uint8(0x06);
                SendPacket(&data);
                return;
            }
        }
        else
        {
            // change pet slot directly in database
            RealmDataDatabase.BeginTransaction();
            static SqlStatementID ChangePetSlot_ID;
            SqlStatement ChangePetSlot = RealmDataDatabase.CreateStatement(ChangePetSlot_ID, "UPDATE character_pet SET slot = ? WHERE owner = ? AND slot = ? ");
            ChangePetSlot.PExecute(slot, _player->GetObjectGuid().GetCounter(), uint32(_player->GetTemporaryUnsummonedPetNumber() ? PET_SAVE_AS_CURRENT : PET_SAVE_NOT_IN_SLOT));
            RealmDataDatabase.CommitTransaction();
            _player->SetTemporaryUnsummonedPetNumber(0);
        }
    }

    Pet* newpet = new Pet(HUNTER_PET);
    if (!newpet->LoadPetFromDB(_player, creature_id, petnumber))
    {
        delete newpet;
        newpet = nullptr;

        // load failed but it may be normal if the pet is set as temporary unsummoned by any reason
        if (!_player->GetTemporaryUnsummonedPetNumber())
        {
            // load really failed
            data << uint8(0x06);
            SendPacket(&data);
            return;
        }
    }
    data << uint8(0x09);
    /*if (newpet)
        data << uint8(0x09);
    else
        data << uint8(0x06);
    */
    SendPacket(&data);
}

void WorldSession::HandleBuyStableSlot(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);

    sLog.outDebug("WORLD: Recv CMSG_BUY_STABLE_SLOT.");
    uint64 npcGUID;

    recv_data >> npcGUID;

    WorldPacket data(SMSG_STABLE_RESULT, 200);

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_STABLEMASTER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleBuyStableSlot - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)));
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    if (GetPlayer()->m_stableSlots < 2)
    {
        StableSlotPricesEntry const *SlotPrice = sStableSlotPricesStore.LookupEntry(GetPlayer()->m_stableSlots+1);
        if (_player->GetMoney() >= SlotPrice->Price)
        {
            ++GetPlayer()->m_stableSlots;
            _player->ModifyMoney(-int32(SlotPrice->Price));
            data << uint8(0x0A);                            // success buy
        }
        else
            data << uint8(0x06);
    }
    else
        data << uint8(0x06);

    SendPacket(&data);
}

void WorldSession::HandleStableRevivePet(WorldPacket &/* recv_data */)
{
    sLog.outDebug("HandleStableRevivePet: Not implemented");
}

void WorldSession::HandleStableSwapPet(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+4);

    sLog.outDebug("WORLD: Recv CMSG_STABLE_SWAP_PET.");
    WorldPacket data(SMSG_STABLE_RESULT, 200);              // guess size

    uint64 npcGUID;
    uint32 pet_number;

    recv_data >> npcGUID >> pet_number;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_STABLEMASTER);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleStableSwapPet - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)));
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // remove mounts this fix bug where getting pet from stable while mounted deletes pet.
    if (GetPlayer()->IsMounted())
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    Pet* pet = _player->GetPet();

    /*if (!pet || pet->getPetType() != HUNTER_PET)
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }*/

    if (pet)
    {
        bool stop = false;
        if (!pet->isAlive())
        {
            _player->SendPetTameFailure(PETTAME_DEAD);
            stop = true;
        }

        if (!stop && pet->getPetType() != HUNTER_PET)
        {
            _player->SendPetTameFailure(PETTAME_INVALIDCREATURE);
            stop = true;
        }

        if (stop)
        {
            data << uint8(0x06);
            SendPacket(&data);
            return;
        }
    }
    else
    {
        SpellCastResult loadResult = Pet::TryLoadFromDB(_player, 0, 0, _player->GetTemporaryUnsummonedPetNumber() != 0, HUNTER_PET);
        if (loadResult != SPELL_CAST_OK)
        {
            if (loadResult == SPELL_FAILED_TARGETS_DEAD)
                _player->SendPetTameFailure(PETTAME_DEAD);

            if (loadResult == SPELL_FAILED_BAD_TARGETS)
                _player->SendPetTameFailure(PETTAME_INVALIDCREATURE);

            data << uint8(0x06);
            SendPacket(&data);
            return;
        }
    }

    // find swapped pet slot in stable
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT slot, entry FROM character_pet WHERE owner = '%u' AND id = '%u'",
                                                         _player->GetGUIDLow(), pet_number);
    if (!result)
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    Field *fields       = result->Fetch();

    uint32 slot         = fields[0].GetUInt32();
    uint32 creature_id  = fields[1].GetUInt32();

    if (!creature_id)
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    CreatureInfo const* creatureInfo = ObjectMgr::GetCreatureTemplate(creature_id);
    if (!creatureInfo || !creatureInfo->isTameable())
    {
        data << uint8(0x06);
        SendPacket(&data);
        return;
    }

    if(pet)
        _player->RemovePet(pet, PetSaveMode(slot));
    else
    {
        // change pet slot directly in memory
        RealmDataDatabase.BeginTransaction();
        static SqlStatementID ChangePetSlot_ID;
        SqlStatement ChangePetSlot = RealmDataDatabase.CreateStatement(ChangePetSlot_ID, "UPDATE character_pet SET slot = ? WHERE owner = ? AND slot = ? ");
        ChangePetSlot.PExecute(slot, _player->GetObjectGuid().GetCounter(), uint32(_player->GetTemporaryUnsummonedPetNumber() ? PET_SAVE_AS_CURRENT : PET_SAVE_NOT_IN_SLOT));
        RealmDataDatabase.CommitTransaction();
    }

    // summon unstabled pet
    Pet* newpet = new Pet;
    if (!newpet->LoadPetFromDB(_player, creature_id, pet_number))
    {
        delete newpet;

        // load failed but it may be normal if the pet is set as temporary unsummoned by any reason
        if (!_player->GetTemporaryUnsummonedPetNumber())
        {
            // load really failed
            data << uint8(0x06);
            SendPacket(&data);
            return;
        }
    }
    data << uint8(0x09);

    // move alive pet to slot or delele dead pet
    /* _player->RemovePet(pet, pet->isAlive() ? PetSaveMode(slot) : PET_SAVE_AS_DELETED);

    // summon unstabled pet
    Pet *newpet = new Pet;
    if (!newpet->LoadPetFromDB(_player,petentry,pet_number))
    {
        delete newpet;
        data << uint8(0x06);
    }
    else
        data << uint8(0x09);
    */

    SendPacket(&data);
}

void WorldSession::HandleRepairItemOpcode(WorldPacket & recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8+8+1);

    sLog.outDebug("WORLD: CMSG_REPAIR_ITEM");

    uint64 npcGUID, itemGUID;
    uint8 guildBank;                                        // new in 2.3.2, bool that means from guild bank money

    recv_data >> npcGUID >> itemGUID >> guildBank;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(npcGUID, UNIT_NPC_FLAG_REPAIR);
    if (!unit)
    {
        sLog.outDebug("WORLD: HandleRepairItemOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(npcGUID)));
        return;
    }

    // remove fake death
    if (GetPlayer()->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    // reputation discount
    float discountMod = _player->GetReputationPriceDiscount(unit);

    uint32 TotalCost = 0;
    if (itemGUID)
    {
        sLog.outDebug("ITEM: Repair item, itemGUID = %u, npcGUID = %u", GUID_LOPART(itemGUID), GUID_LOPART(npcGUID));

        Item* item = _player->GetItemByGuid(itemGUID);

        if (item)
            TotalCost= _player->DurabilityRepair(item->GetPos(),true,discountMod,guildBank>0?true:false);
    }
    else
    {
        sLog.outDebug("ITEM: Repair all items, npcGUID = %u", GUID_LOPART(npcGUID));

        TotalCost = _player->DurabilityRepairAll(true,discountMod,guildBank>0?true:false);
    }
    if (guildBank)
    {
        uint32 GuildId = _player->GetGuildId();
        if (!GuildId)
            return;
        Guild *pGuild = sGuildMgr.GetGuildById(GuildId);
        if (!pGuild)
            return;
        pGuild->LogBankEvent(GUILD_BANK_LOG_REPAIR_MONEY, 0, _player, TotalCost);
        pGuild->SendMoneyInfo(this, _player->GetGUIDLow());
    }
}

