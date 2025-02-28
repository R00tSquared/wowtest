// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include <sstream>

#include "Common.h"
#include "SharedDefines.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Object.h"
#include "Creature.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "UpdateMask.h"
#include "Util.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "Transports.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "VMapFactory.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "InstanceData.h"

#include "TemporarySummon.h"
#include "OutdoorPvPMgr.h"

#include "Spell.h"

#include "Shop.h"

#include "movement/packet_builder.h"

uint32 GuidHigh2TypeId(uint32 guid_hi)
{
    switch (guid_hi)
    {
        case HIGHGUID_ITEM:         return TYPEID_ITEM;
        //case HIGHGUID_CONTAINER:    return TYPEID_CONTAINER; HIGHGUID_CONTAINER==HIGHGUID_ITEM currently
        case HIGHGUID_UNIT:         return TYPEID_UNIT;
        case HIGHGUID_PET:          return TYPEID_UNIT;
        case HIGHGUID_PLAYER:       return TYPEID_PLAYER;
        case HIGHGUID_GAMEOBJECT:   return TYPEID_GAMEOBJECT;
        case HIGHGUID_DYNAMICOBJECT:return TYPEID_DYNAMICOBJECT;
        case HIGHGUID_CORPSE:       return TYPEID_CORPSE;
        case HIGHGUID_MO_TRANSPORT: return TYPEID_GAMEOBJECT;
    }
    return TYPEID_OBJECT;                                              // unknown
}

Object::Object()
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPEMASK_OBJECT;

    m_uint32Values      = 0;
    m_uint32Values_mirror = 0;
    m_valuesCount       = 0;

    m_inWorld           = false;
    m_objectUpdated     = false;

    m_PackGUID.Set(0);
}

Object::~Object()
{
    if (m_uint32Values)
    {
        if (IsInWorld())
        {
            ///- Do NOT call RemoveFromWorld here, if the object is a player it will crash
            sLog.outLog(LOG_CRASH, "ERROR: Object::~Object - guid=%llu, typeid=%d deleted but still in world!!", GetGUID(), GetTypeId());
            ASSERT(false);
        }

        ASSERT(!m_objectUpdated);

        delete [] m_uint32Values;
        delete [] m_uint32Values_mirror;

        m_uint32Values = NULL;
        m_uint32Values_mirror = NULL;
    }
}

void Object::_InitValues()
{
    m_uint32Values = new uint32[ m_valuesCount ];
    memset(m_uint32Values, 0, m_valuesCount*sizeof(uint32));

    m_uint32Values_mirror = new uint32[ m_valuesCount ];
    memset(m_uint32Values_mirror, 0, m_valuesCount*sizeof(uint32));

    m_objectUpdated = false;
}

void Object::_Create(uint32 guidlow, uint32 entry, HighGuid guidhigh)
{
    if (!m_uint32Values)
        _InitValues();

    uint64 guid = MAKE_NEW_GUID(guidlow, entry, guidhigh);  // required more changes to make it working

    SetUInt64Value(OBJECT_FIELD_GUID, guid);
    SetUInt32Value(OBJECT_FIELD_TYPE, m_objectType);

    debug_info = { guidlow, entry, "" };

    m_PackGUID.Set(guid);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if (!target)
        return;

    uint8 updatetype   = UPDATETYPE_CREATE_OBJECT;
    uint8 updateFlags  = m_updateFlag;

    /** lower flag1 **/
    if (target == this)                                      // building packet for yourself
        updateFlags |= UPDATEFLAG_SELF;

    if (updateFlags & UPDATEFLAG_HAS_POSITION)
    {
        // UPDATETYPE_CREATE_OBJECT2 dynamic objects, corpses...
        if (isType(TYPEMASK_DYNAMICOBJECT) || isType(TYPEMASK_CORPSE) || isType(TYPEMASK_PLAYER))
            updatetype = UPDATETYPE_CREATE_OBJECT2;

        // UPDATETYPE_CREATE_OBJECT2 for pets...
        if (target->GetPetGUID() == GetGUID())
            updatetype = UPDATETYPE_CREATE_OBJECT2;

        // UPDATETYPE_CREATE_OBJECT2 for some gameobject types...
        if (isType(TYPEMASK_GAMEOBJECT))
        {
            switch (((GameObject*)this)->GetGoType())
            {
                case GAMEOBJECT_TYPE_TRAP:
                case GAMEOBJECT_TYPE_DUEL_ARBITER:
                case GAMEOBJECT_TYPE_FLAGSTAND:
                case GAMEOBJECT_TYPE_FLAGDROP:
                    updatetype = UPDATETYPE_CREATE_OBJECT2;
                    break;
                case GAMEOBJECT_TYPE_TRANSPORT:
                    updateFlags |= UPDATEFLAG_TRANSPORT;
                    break;
                default:
                    break;
            }
        }

        if (isType(TYPEMASK_UNIT))
        {
            if (((Unit*)this)->GetVictim())
                updateFlags |= UPDATEFLAG_HAS_ATTACKING_TARGET;
        }
    }

    ByteBuffer buf(500);
    buf << uint8(updatetype);
    //buf.append(GetPackGUID());    //client crashes when using this
    buf << uint8(0xFF) << GetGUID();
    buf << uint8(m_objectTypeId);

    BuildMovementUpdate(&buf, updateFlags);

    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);
    _SetCreateBits(&updateMask, target);
    BuildValuesUpdate(updatetype, &buf, &updateMask, target);
    data->AddUpdateBlock(buf);
}

void Object::SendCreateUpdateToPlayer(Player* player)
{
    // send create update to player
    UpdateData upd;
    WorldPacket packet;

    BuildCreateUpdateBlockForPlayer(&upd, player);
    upd.BuildPacket(&packet);

    player->SendPacketToSelf(&packet);
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    ByteBuffer buf(500);

    buf << uint8(UPDATETYPE_VALUES);
    //buf.append(GetPackGUID());    //client crashes when using this. but not have crash in debug mode
    buf << uint8(0xFF);
    buf << GetGUID();

    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);

    _SetUpdateBits(&updateMask, target);
    BuildValuesUpdate(UPDATETYPE_VALUES, &buf, &updateMask, target);

    data->AddUpdateBlock(buf);
}

void Object::BuildFieldsUpdate(Player *pl, UpdateDataMapType &data_map) const
{
    UpdateDataMapType::iterator iter = data_map.find(pl);
    if (iter == data_map.end())
    {
        std::pair<UpdateDataMapType::iterator, bool> p = data_map.insert(UpdateDataMapType::value_type(pl, UpdateData()));
        ASSERT(p.second);
        iter = p.first;
    }
    BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);
}

void Object::BuildOutOfRangeUpdateBlock(UpdateData * data) const
{
    data->AddOutOfRangeGUID(GetGUID());
}

void Object::DestroyForPlayer(Player *target) const
{
    ASSERT(target);

    WorldPacket data(SMSG_DESTROY_OBJECT, 8);
    data << uint64(GetGUID());
    target->SendPacketToSelf(&data);
}

void Object::BuildMovementUpdate(ByteBuffer * data, uint8 updateFlags) const
{
    *data << uint8(updateFlags);                            // update flags

    // 0x20
    if (updateFlags & UPDATEFLAG_LIVING)
    {
        Unit *unit = ((Unit*)this);

        if (GetTypeId() == TYPEID_PLAYER)
        {
            Player *player = ((Player*)unit);
            if(player->GetTransport())
                player->m_movementInfo.AddMovementFlag(MOVEFLAG_ONTRANSPORT);
            else
                player->m_movementInfo.RemoveMovementFlag(MOVEFLAG_ONTRANSPORT);
        }

        if (unit->IsStopped() && unit->m_movementInfo.HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
            unit->m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEFLAG_SPLINE_ENABLED|MOVEFLAG_FORWARD));

        // Update movement info time
        unit->m_movementInfo.UpdateTime(WorldTimer::getMSTime());
        // Write movement info
        unit->m_movementInfo.Write(*data);

        // Unit speeds
        *data << float(unit->GetSpeed(MOVE_WALK));
        *data << float(unit->GetSpeed(MOVE_RUN));
        *data << float(unit->GetSpeed(MOVE_RUN_BACK));
        *data << float(unit->GetSpeed(MOVE_SWIM));
        *data << float(unit->GetSpeed(MOVE_SWIM_BACK));
        *data << float(unit->GetSpeed(MOVE_FLIGHT));
        *data << float(unit->GetSpeed(MOVE_FLIGHT_BACK));
        *data << float(unit->GetSpeed(MOVE_TURN_RATE));

        // 0x08000000
        if (unit->m_movementInfo.GetMovementFlags() & MOVEFLAG_SPLINE_ENABLED)
            Movement::PacketBuilder::WriteCreate(*unit->movespline, *data);
    }
    // 0x40
    else if (updateFlags & UPDATEFLAG_HAS_POSITION)
    {
        // 0x02
        if (updateFlags & UPDATEFLAG_TRANSPORT && ((GameObject*)this)->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            *data << float(0);
            *data << float(0);
            *data << float(0);
            *data << float(((WorldObject*)this)->GetOrientation());
        }
        else
        {
            *data << float(((WorldObject*)this)->GetPositionX());
            *data << float(((WorldObject*)this)->GetPositionY());
            *data << float(((WorldObject*)this)->GetPositionZ());
            *data << float(((WorldObject*)this)->GetOrientation());
        }
    }

    // 0x8
    if (updateFlags & UPDATEFLAG_LOWGUID)
    {
        switch (GetTypeId())
        {
            case TYPEID_OBJECT:
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
            case TYPEID_GAMEOBJECT:
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_CORPSE:
                *data << uint32(GetGUIDLow());              // GetGUIDLow()
                break;
            case TYPEID_UNIT:
                *data << uint32(0x0000000B);                // unk, can be 0xB or 0xC
                break;
            case TYPEID_PLAYER:
                if(updateFlags & UPDATEFLAG_SELF)
                    *data << uint32(0x00000015);            // unk, can be 0x15 or 0x22
                else
                    *data << uint32(0x00000008);            // unk, can be 0x7 or 0x8
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }

    // 0x10
    if (updateFlags & UPDATEFLAG_HIGHGUID)
    {
        switch(GetTypeId())
        {
            case TYPEID_OBJECT:
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
            case TYPEID_GAMEOBJECT:
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_CORPSE:
                *data << uint32(GetObjectGuid().GetHigh()); // GetGUIDHigh()
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }

    // 0x4
    if (updateFlags & UPDATEFLAG_HAS_ATTACKING_TARGET)       // packed guid (current target guid)
    {
        if (((Unit*)this)->GetVictim())
            *data << ((Unit*)this)->GetVictim()->GetPackGUID();
        else
            data->appendPackGUID(0);
    }

    // 0x2
    if (updateFlags & UPDATEFLAG_TRANSPORT)
    {
        *data << uint32(WorldTimer::getMSTime());                       // ms time
    }
}

void Object::BuildValuesUpdate(uint8 updatetype, ByteBuffer * data, UpdateMask *updateMask, Player *target) const
{
    if (!target)
        return;

    bool IsActivateToQuest = false;
    if (updatetype == UPDATETYPE_CREATE_OBJECT || updatetype == UPDATETYPE_CREATE_OBJECT2)
    {
        if (isType(TYPEMASK_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {
            if (((GameObject*)this)->ActivateToQuest(target) || target->isGameMaster())
                IsActivateToQuest = true;

            updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);

            if (GetUInt32Value(GAMEOBJECT_ARTKIT))
                updateMask->SetBit(GAMEOBJECT_ARTKIT);
        }
    }
    else                                                    //case UPDATETYPE_VALUES
    {
        if (isType(TYPEMASK_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {
            if (((GameObject*)this)->ActivateToQuest(target) || target->isGameMaster())
            {
                IsActivateToQuest = true;
            }
            updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);
            updateMask->SetBit(GAMEOBJECT_ANIMPROGRESS);
        }
    }

    ASSERT(updateMask && updateMask->GetCount() == m_valuesCount);

    *data << (uint8)updateMask->GetBlockCount();
    data->append(updateMask->GetMask(), updateMask->GetLength());

    // 2 specialized loops for speed optimization in non-unit case
    if (isType(TYPEMASK_UNIT))                               // unit (creature/player) case
    {
        for (uint16 index = 0; index < m_valuesCount; index ++)
        {
            if (updateMask->GetBit(index))
            {
                // remove custom flag before send
                if (index == UNIT_NPC_FLAGS)
                    *data << uint32(m_uint32Values[ index ] & ~(UNIT_NPC_FLAG_GUARD | UNIT_NPC_FLAG_OUTDOORPVP));
                // FIXME: Some values at server stored in float format but must be sent to client in uint32 format
                else if (index >= UNIT_FIELD_BASEATTACKTIME && index <= UNIT_FIELD_RANGEDATTACKTIME)
                {
                    // convert from float to uint32 and send
                    *data << uint32(m_floatValues[ index ] < 0 ? 0 : m_floatValues[ index ]);
                }
                // there are some float values which may be negative or can't get negative due to other checks
                else if (index >= UNIT_FIELD_NEGSTAT0   && index <= UNIT_FIELD_NEGSTAT4 ||
                    index >= UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE  && index <= (UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 6) ||
                    index >= UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE  && index <= (UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6) ||
                    index >= UNIT_FIELD_POSSTAT0   && index <= UNIT_FIELD_POSSTAT4)
                {
                    *data << uint32(m_floatValues[ index ]);
                }
                // Gamemasters should be always able to select units - remove not selectable flag
                else if (index == UNIT_FIELD_FLAGS && target->isGameMaster())
                {
                    *data << (m_uint32Values[ index ] & ~UNIT_FLAG_NOT_SELECTABLE);
                }

				// START HP_PERCENT
				// everything as % until in party
				//else if (!sWorld.isEasyRealm() && index == UNIT_FIELD_MAXHEALTH && !target->IsInRaidWith((Unit*)this) && !target->IsInPartyWith((Unit*)this))
				//{
				//	*data << uint32(100);
				//}
				//else if (!sWorld.isEasyRealm() && index == UNIT_FIELD_HEALTH && !target->IsInRaidWith((Unit*)this) && !target->IsInPartyWith((Unit*)this))
				//{
				//	*data << uint32(ceil(float(m_uint32Values[index])*100.f / float(m_uint32Values[index + 6])));
				//}
				// END HP_PERCENT

                // use modelid_a if not gm, _h if gm for CREATURE_FLAG_EXTRA_TRIGGER creatures
                else if (index == UNIT_FIELD_DISPLAYID && GetTypeId() == TYPEID_UNIT)
                {
                    const CreatureInfo* cinfo = ((Creature*)this)->GetCreatureInfo();
                    if (cinfo->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER)
                    {
                        if (target->isGameMaster())
                        {
                            if (cinfo->Modelid_A2)
                                *data << cinfo->Modelid_A1;
                            else
                                *data << 17519; // world invisible trigger's model
                        }
                        else
                        {
                            if (cinfo->Modelid_A2)
                                *data << cinfo->Modelid_A2;
                            else
                                *data << 11686; // world invisible trigger's model
                        }
                    }
                    else
                        *data << m_uint32Values[ index ];
                }
                // hide lootable animation for unallowed players
                else if (index == UNIT_DYNAMIC_FLAGS)
                {
                    if (GetTypeId() == TYPEID_UNIT)
                    {
                        if (!target->isAllowedToLoot((Creature*)this))
                            *data << (m_uint32Values[ index ] & ~UNIT_DYNFLAG_LOOTABLE);
                        else
                            *data << (m_uint32Values[ index ] & ~UNIT_DYNFLAG_OTHER_TAGGER);
                    }
                    // hide RAF flag if need
					else if ((m_uint32Values[index] & UNIT_DYNFLAG_REFER_A_FRIEND) && GetTypeId() == TYPEID_PLAYER && !((Player*)this)->GetSession()->isRAFConnectedWith(target->GetSession()))
					{
						*data << (m_uint32Values[index] & ~UNIT_DYNFLAG_REFER_A_FRIEND);
					}
					else
					{
						*data << m_uint32Values[index];
					}
                }
                // FG: pretend that OTHER players in own group are friendly ("blue")
                else if (index == UNIT_FIELD_BYTES_2 || index == UNIT_FIELD_FACTIONTEMPLATE)
                {
                bool ch = false;
                    if (target->GetTypeId() == TYPEID_PLAYER && GetTypeId() == TYPEID_PLAYER && target != this)
                    {
                    if (target->IsInSameGroupWith((Player*)this) || target->IsInSameRaidWith((Player*)this) || (sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) && target->IsFriendlyTo((Player*)this)))
                    {
                        if (index == UNIT_FIELD_BYTES_2)
                        {
                            debug_log("-- VALUES_UPDATE: Sending '%s' the blue-group-fix from '%s' (flag)", target->GetName(), ((Player*)this)->GetName());
                            *data << (m_uint32Values[ index ] & ((UNIT_BYTE2_FLAG_SANCTUARY | UNIT_BYTE2_FLAG_AURAS | UNIT_BYTE2_FLAG_UNK5) << 8)); // this flag is at uint8 offset 1 !!

                            ch = true;
                        }
                        else if (index == UNIT_FIELD_FACTIONTEMPLATE)
                        {
                            FactionTemplateEntry const *ft1, *ft2;
                            ft1 = ((Player*)this)->getFactionTemplateEntry();
                            ft2 = ((Player*)target)->getFactionTemplateEntry();
                            if (ft1 && ft2 && !ft1->IsFriendlyTo(*ft2))
                            {
                                uint32 faction = ((Player*)target)->getFaction(); // pretend that all other HOSTILE players have own faction, to allow follow, heal, rezz (trade wont work)
                                debug_log("-- VALUES_UPDATE: Sending '%s' the blue-group-fix from '%s' (faction %u)", target->GetName(), ((Player*)this)->GetName(), faction);
                                *data << uint32(faction);
                                ch = true;
                            }
                        }
                    }
                    }
                    if (!ch)
                        *data << m_uint32Values[ index ];
                }
                else if (this != target && (index == PLAYER_VISIBLE_ITEM_16_0 || index == PLAYER_VISIBLE_ITEM_17_0 || index == PLAYER_VISIBLE_ITEM_18_0) &&
                    target->GetTypeId() == TYPEID_PLAYER && GetTypeId() == TYPEID_PLAYER) // mainhand, offhand, ranged
                {
                    uint8 slot = 0;
                    if (index == PLAYER_VISIBLE_ITEM_17_0)
                        slot = 1;
                    else if (index == PLAYER_VISIBLE_ITEM_18_0)
                        slot = 2;

                    Item* item = ((Player*)this)->GetItemByPos(255, EQUIPMENT_SLOT_MAINHAND + slot);

                    if (!item || item->GetEntry() == m_uint32Values[index])
                        *data << m_uint32Values[ index ];
                    else
                    {
                        bool sendReal = false;
                        
                        // arena case
                        if (((Player*)this)->InArena() && !((Player*)this)->IsInSameRaidWith(target))
                            sendReal = true;
                        // inspect case
                        else if (Aura* aurThis = ((Player*)this)->GetAura(55148, 0))// player being inspected and has transmog
                        {
                            Aura* aurTarget = target->GetAura(55149, 0); // on aura remove should updateforcevalue at indexes where transmog exists
                            if (aurTarget && aurThis->GetModifierValue() == aurTarget->GetModifierValue())
                                sendReal = true;
                        }

                        if (sendReal)
                            *data << item->GetEntry();
                        else 
                            *data << m_uint32Values[ index ];
                    }
                }
                else if (this == target && index == PLAYER_FIELD_COINAGE && ((Player*)this)->HasAura(55193))
                {
                    ACE_GUARD(ACE_Thread_Mutex, guard, sWorld.GetShop()->plrVariableMutex);
                    auto itr = sWorld.GetShop()->plrVariable.find(((Player*)this)->GetGUID());
                    *data << uint32(itr != sWorld.GetShop()->plrVariable.end() ? uint32(itr->second.goldVisual) * 10000 : 0); // in gold
                }
                else
                {
                    // send in current format (float as float, uint32 as uint32)
                    *data << m_uint32Values[ index ];
                }
            }
        }
    }
    else if (isType(TYPEMASK_GAMEOBJECT))                    // gameobject case
    {
        for (uint16 index = 0; index < m_valuesCount; index ++)
        {
            if (updateMask->GetBit(index))
            {
                // send in current format (float as float, uint32 as uint32)
                if (index == GAMEOBJECT_DYN_FLAGS)
                {
                    if (IsActivateToQuest)
                    {
                        switch (((GameObject*)this)->GetGoType())
                        {
                            case GAMEOBJECT_TYPE_CHEST:
                            case GAMEOBJECT_TYPE_GOOBER:
                                *data << uint16(GO_DYNFLAG_LO_ACTIVATE | GO_DYNFLAG_LO_SPARKLE);
                                *data << uint16(-1);
                                break;
                            default:
                                *data << uint32(0);         // unknown. not happen.
                                break;
                        }
                    }
                    else
                        *data << uint32(0);                 // disable quest object
                }
                // Private Mailbox is only selectable by its owner
                else if (index == GAMEOBJECT_FLAGS && ((GameObject*)this)->GetEntry() == 181751 && ((GameObject*)this)->GetOwnerGUID() == target->GetGUID())
                {
                    *data << (m_uint32Values[index] & ~GO_FLAG_NOTSELECTABLE);
                }
                else
                    *data << m_uint32Values[ index ];       // other cases
            }
        }
    }
    else                                                    // other objects case (no special index checks)
    {
        for (uint16 index = 0; index < m_valuesCount; index ++)
        {
            if (updateMask->GetBit(index))
            {
                // send in current format (float as float, uint32 as uint32)
                *data << m_uint32Values[ index ];
            }
        }
    }
}

void Object::ClearUpdateMask(bool remove)
{
    for (uint16 index = 0; index < m_valuesCount; index ++)
    {
        if (m_uint32Values_mirror[index]!= m_uint32Values[index])
            m_uint32Values_mirror[index] = m_uint32Values[index];
    }
    if (m_objectUpdated)
    {
        if (remove)
            RemoveFromClientUpdateList();
        m_objectUpdated = false;
    }
}

bool Object::LoadValues(const char* data)
{
    if (!m_uint32Values) _InitValues();

    Tokens tokens = StrSplit(data, " ");

    if (tokens.size() != m_valuesCount)
        return false;

    Tokens::iterator iter;
    int index;
    for (iter = tokens.begin(), index = 0; index < m_valuesCount; ++iter, ++index)
    {
        m_uint32Values[index] = atol((*iter).c_str());
    }

    return true;
}

void Object::_SetUpdateBits(UpdateMask *updateMask, Player* /*target*/) const
{
    for (uint16 index = 0; index < m_valuesCount; index ++)
    {
        if (m_uint32Values_mirror[index]!= m_uint32Values[index])
            updateMask->SetBit(index);
    }

    // Trentone: This is supposedly done to evade the possibility of no-change-BuildValuesUpdate,
    // because, for example, item-move on player "A" will cause an update to be called, but it wouldn't add anything to send to "B", so this adds one bit to an update
    if (GetTypeId() == TYPEID_PLAYER)
        updateMask->SetBit(UNIT_FIELD_BYTES_2); 
}

void Object::_SetCreateBits(UpdateMask *updateMask, Player* /*target*/) const
{
    for (uint16 index = 0; index < m_valuesCount; index++)
    {
        if (GetUInt32Value(index) != 0)
            updateMask->SetBit(index);
    }
}

void Object::SetInt32Value(uint16 index, int32 value)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));

    if (m_int32Values[ index ] != value)
    {
        m_int32Values[ index ] = value;

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetUInt32Value(uint16 index, uint32 value)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));

    if (m_uint32Values[ index ] != value)
    {
        m_uint32Values[ index ] = value;

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetUInt64Value(uint16 index, const uint64 &value)
{
    ASSERT(index + 1 < m_valuesCount || PrintIndexError(index , true));
    if (*((uint64*)&(m_uint32Values[ index ])) != value)
    {
        m_uint32Values[ index ] = *((uint32*)&value);
        m_uint32Values[ index + 1 ] = *(((uint32*)&value) + 1);

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetFloatValue(uint16 index, float value)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));

    if (m_floatValues[ index ] != value)
    {
        m_floatValues[ index ] = value;

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetByteValue(uint16 index, uint8 offset, uint8 value)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));

    if (offset > 4)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[ index ] >> (offset * 8)) != value)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(0xFF) << (offset * 8));
        m_uint32Values[ index ] |= uint32(uint32(value) << (offset * 8));

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));

    if (offset > 2)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[ index ] >> (offset * 16)) != value)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(0xFFFF) << (offset * 16));
        m_uint32Values[ index ] |= uint32(uint32(value) << (offset * 16));

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetStatFloatValue(uint16 index, float value)
{
    if (value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value(uint16 index, int32 value)
{
    if (value < 0)
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetUInt32Value(index,cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index,cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index,cur);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetFloatValue(index,cur);
}

void Object::SetFlag(uint16 index, uint32 newFlag)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval | newFlag;

    if (oldval != newval)
    {
        m_uint32Values[ index ] = newval;

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetNonAttackableFlag(uint32 newFlag, bool BecameNonAttackable)
{
    if (BecameNonAttackable)
        SetFlag(UNIT_FIELD_FLAGS, newFlag);
    else
        RemoveFlag(UNIT_FIELD_FLAGS, newFlag);

    if (!BecameNonAttackable)
        return;

    std::list<Unit*> targets;
    Hellground::AnyUnfriendlyUnitInObjectRangeCheck u_check((Unit*)this, ((Unit*)this)->GetMap()->GetVisibilityDistance());
    Hellground::UnitListSearcher<Hellground::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);

    Cell::VisitAllObjects((Unit*)this, searcher, ((Unit*)this)->GetMap()->GetVisibilityDistance());
    for (std::list<Unit*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
    {
        if((*iter)->CanHaveThreatList())
            continue;

        if((*iter)->getVictimGUID() == this->GetGUID())
            ((Unit*)*iter)->AttackStop(); // stop attacking, clears target inside method for units, should not clear target for players!

        if (!(*iter)->HasUnitState(UNIT_STAT_CASTING))
            continue;

        for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        {
            if ((*iter)->m_currentSpells[i] && (*iter)->m_currentSpells[i]->m_targets.getUnitTargetGUID() == this->GetGUID())
            {
                (*iter)->InterruptSpell(i, false);
            }
        }
    }
    HostileReference *ref = ((Unit*)this)->getHostileRefManager().getFirst();
    while(ref)
    {
        Unit* target = ref->getSource()->getOwner();
        ref = ref->next();

        if(!target)
            continue;
            
        if (target->HasUnitState(UNIT_STAT_CASTING))
            for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
                if (target->m_currentSpells[i] && target->m_currentSpells[i]->m_targets.getUnitTargetGUID() == this->GetGUID())
                    target->InterruptSpell(i, false);

        if (target->CanHaveThreatList())
            continue;

        // ((Unit*)this)->getHostileRefManager().deleteReference(target); // should not be here, we're might still wanna be in combat
        if(target->getVictimGUID() == this->GetGUID())
            target->AttackStop(); // clears target, though we shouldn't do it here, or should we? 
    }
}

void Object::RemoveFlag(uint16 index, uint32 oldFlag)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval & ~oldFlag;

    if (oldval != newval)
    {
        m_uint32Values[ index ] = newval;

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetByteFlag(uint16 index, uint8 offset, uint8 newFlag)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));

    if (offset > 4)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if (!(uint8(m_uint32Values[ index ] >> (offset * 8)) & newFlag))
    {
        m_uint32Values[ index ] |= uint32(uint32(newFlag) << (offset * 8));

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

void Object::RemoveByteFlag(uint16 index, uint8 offset, uint8 oldFlag)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index , true));

    if (offset > 4)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[ index ] >> (offset * 8)) & oldFlag)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(oldFlag) << (offset * 8));

        if (m_inWorld)
        {
            if (!m_objectUpdated)
            {
                AddToClientUpdateList();
                m_objectUpdated = true;
            }
        }
    }
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    sLog.outLog(LOG_CRITICAL, "ERROR: Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u",(set ? "set value to" : "get value from"),index,m_valuesCount,GetTypeId(),m_objectType);

    // assert must fail after function call
    return false;
}

std::string Object::GetUInt32ValuesString() const
{
    std::ostringstream ss;
    for (uint16 i = 0; i < m_valuesCount; ++i)
        ss << GetUInt32Value(i) << " ";

    return ss.str();
}

WorldObject::WorldObject()
    : m_mapId(0), m_InstanceId(0),
    m_positionX(0.0f), m_positionY(0.0f), m_positionZ(0.0f), m_orientation(0.0f),
    mSemaphoreTeleport(false)
    , m_map(NULL), m_zoneScript(NULL)
    , m_activeBy(0)/*, IsTempWorldObject(false)*/
{
    mSemaphoreTeleport  = false;
}

/*void WorldObject::SetWorldObject(bool on)
{
    if (!IsInWorld())
        return;

    GetMap()->AddObjectToSwitchList(this, on);
}*/

void WorldObject::setActive(bool on, ActiveObject activeBy)
{
    if (GetTypeId() == TYPEID_PLAYER)
        return;

    bool wasActive = m_activeBy;

    if(on)
        m_activeBy |= activeBy;
    else
        m_activeBy &= ~activeBy;

    bool isActive = m_activeBy;

    if (wasActive == isActive)
        return;

    if (!IsInWorld())
        return;

    Map *map = GetMap();
    if (!map)
        return;

    if (on)
        map->AddToActive(this);
    else
        map->RemoveFromActive(this);
}

void WorldObject::CleanupsBeforeDelete()
{

}

void WorldObject::_Create(uint32 guidlow, HighGuid guidhigh, uint32 mapid)
{
    Object::_Create(guidlow, 0, guidhigh);

    m_mapId = mapid;
}

void WorldObject::Relocate(Position pos)
{
    m_positionX = pos.x;
    m_positionY = pos.y;
    m_positionZ = pos.z;
    m_orientation = pos.o;

    if(isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangePosition(pos.x, pos.y, pos.z, pos.o);
}

void WorldObject::Relocate(float x, float y, float z, float orientation)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    m_orientation = orientation;

    if(isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangePosition(x, y, z, orientation);
}

void WorldObject::Relocate(float x, float y, float z)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;

    if(isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangePosition(x, y, z, GetOrientation());
}

void WorldObject::SetOrientation(float orientation)
{
    m_orientation = orientation;

    if(isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangeOrientation(orientation);
}

uint32 WorldObject::GetZoneId() const
{
    if (!Hellground::IsValidMapCoord(m_positionX, m_positionY, m_positionZ))
    {
        sLog.outDebug("Unit::GetZoneId()(%f, %f, %f) .. bad coordinates!",m_positionX, m_positionY, m_positionZ);
        return 0;
    }

    return GetTerrain()->GetZoneId(m_positionX, m_positionY, m_positionZ);
}

uint32 WorldObject::GetAreaId() const
{
    if (!Hellground::IsValidMapCoord(m_positionX, m_positionY, m_positionZ))
    {
        sLog.outDebug("Unit::GetAreaId()(%f, %f, %f) .. bad coordinates!",m_positionX, m_positionY, m_positionZ);
        return 0;
    }

    return GetTerrain()->GetAreaId(m_positionX, m_positionY, m_positionZ);
}

InstanceData* WorldObject::GetInstanceData()
{
    Map *map = GetMap();
    return map->IsDungeon() ? ((InstanceMap*)map)->GetInstanceData() : NULL;
}
                                                            //slow
float WorldObject::GetDistance(WorldObject const* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

bool WorldObject::GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D /* = true */) const
{
    float dx1 = GetPositionX() - obj1->GetPositionX();
    float dy1 = GetPositionY() - obj1->GetPositionY();
    float distsq1 = dx1*dx1 + dy1*dy1;
    if (is3D)
    {
        float dz1 = GetPositionZ() - obj1->GetPositionZ();
        distsq1 += dz1*dz1;
    }

    float dx2 = GetPositionX() - obj2->GetPositionX();
    float dy2 = GetPositionY() - obj2->GetPositionY();
    float distsq2 = dx2*dx2 + dy2*dy2;
    if (is3D)
    {
        float dz2 = GetPositionZ() - obj2->GetPositionZ();
        distsq2 += dz2*dz2;
    }

    return distsq1 < distsq2;
}

float WorldObject::GetDistance2d(float x, float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetExactDistance2d(const float x, const float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    return sqrt((dx * dx) + (dy * dy));
}

float WorldObject::GetDistance(const float x, const float y, const float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceSq(const float& x, const float& y, const float& z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    return dx * dx + dy * dy + dz * dz;
}

float WorldObject::GetDistanceSq(WorldObject const* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    return dx * dx + dy * dy + dz * dz;
}

float WorldObject::GetDistance2d(WorldObject const* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceZ(WorldObject const* obj) const
{
    float dz = fabs(GetPositionZ() - obj->GetPositionZ());
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = dz - sizefactor;
    return (dist > 0 ? dist : 0);
}

// example:
// dist2compare = 30
// sizefactor=boundingradius (how big mob is) = 100
// distsq (real distance) = 40
// maxdist = 30 + 15
// 40 < 130
// ingame spells count COMBATREACH, NOT BOUNDINGRADIUS
bool WorldObject::IsWithinDist3d(float x, float y, float z, float dist2compare) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetObjectBoundingRadius();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinDist2d(float x, float y, float dist2compare) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float distsq = dx*dx + dy*dy;

    float sizefactor = GetObjectBoundingRadius();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if (is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz*dz;
    }
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::_IsWithinDist(WorldLocation const* wLoc, float dist2compare, bool is3D) const
{
    float dx = GetPositionX() - wLoc->coord_x;
    float dy = GetPositionY() - wLoc->coord_y;
    float distsq = dx*dx + dy*dy;
    if (is3D)
    {
        float dz = GetPositionZ() - wLoc->coord_z;
        distsq += dz*dz;
    }
    float maxdist = dist2compare + GetObjectSize();

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinLOSInMap(WorldObject const* obj) const
{
    if (!IsInMap(obj))
        return false;

    float ox,oy,oz;
    obj->GetPosition(ox,oy,oz);
    return(IsWithinLOS(ox, oy, oz));
}

bool WorldObject::IsWithinDistInMapForSrcCaster(WorldObject const* obj, float dist2compare) const
{
    if (!obj || !IsInMap(obj))
        return false;
    
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx * dx + dy * dy + dz * dz;

    float sizefactor = obj->GetObjectSize();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinLOS(const float ox, const float oy, const float oz) const
{
    if (!GetTerrain()->IsLineOfSightEnabled())
        return true;

    float x,y,z;
    GetPosition(x,y,z);
    VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
    bool result = vMapManager->isInLineOfSight(GetMapId(), x, y, z +2.0f, ox, oy, oz +2.0f);
    

    uint8 prec = sWorld.getConfig(CONFIG_VMAP_GROUND); // this is bad - Trentone
    if (prec && result) // if not result then no reason to check
    {
        const TerrainInfo* ti = GetTerrain();
        if (!ti)
            return true;
        float beginh = ti->GetHeight(x, y, z, false);
        if (beginh == VMAP_INVALID_HEIGHT_VALUE)
            return true;
        float endh = ti->GetHeight(ox, oy, oz, false);
        if (endh == VMAP_INVALID_HEIGHT_VALUE)
            return true;

        float tolerance = sWorld.getConfig(CONFIG_VMAP_GROUND_TOLERANCE);
        for (uint8 i = 1; i < prec; i++)
        {
            float height = ti->GetHeight(x + (i*(ox - x)) / prec, y + (i*(ox - y)) / prec, std::max(z, oz), false);
            if (height == VMAP_INVALID_HEIGHT_VALUE || height > tolerance + z + (i*(oz - z)) / prec)
                return false;
        }
    }
    return result;
}

bool WorldObject::IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D /* = true */) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if (is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz*dz;
    }

    float sizefactor = GetObjectSize() + obj->GetObjectSize();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange2d(float x, float y, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float distsq = dx*dx + dy*dy;

    float sizefactor = GetObjectSize();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange3d(float x, float y, float z, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetObjectSize();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

float WorldObject::GetOrientationTo(WorldObject const* obj) const
{
    if (!obj) return 0;
    return GetOrientationTo(obj->GetPositionX(), obj->GetPositionY());
}

// Return angle in range 0..2*pi
float WorldObject::GetOrientationTo(const float x, const float y) const
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();

    float ang = atan2(dy, dx);
    ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    return ang;
}

float WorldObject::GetAngleTo(WorldObject const* obj) const
{
    if (!obj) return 0;
    return GetAngleTo(obj->GetPositionX(), obj->GetPositionY());
}

// Return angle in range 0..2*pi
float WorldObject::GetAngleTo(const float x, const float y) const
{
    return GetOrientationTo(x, y) - GetOrientation();
}

Position WorldObject::GetPoint2d(float rel_ori, float dist) const
{
    rel_ori += GetOrientation();

    Position pos;
    pos.x = GetPositionX() + dist * cos(rel_ori);
    pos.y = GetPositionY() + dist * sin(rel_ori);

    return pos;
}

bool WorldObject::HasInArc(const float arcangle, WorldObject const* obj) const
{
    // always have self in arc
    if (obj == this)
        return true;

    float arc = arcangle;
    arc = MapManager::NormalizeOrientation(arc);

    float angle = GetOrientationTo(obj);
    angle -= m_orientation;

    // move angle to range -pi ... +pi
    angle = MapManager::NormalizeOrientation(angle);
    if (angle > M_PI)
        angle -= 2.0f * M_PI;

    float lborder =  -1 * (arc/2.0f);                       // in range -pi..0
    float rborder = (arc/2.0f);                             // in range 0..pi
    return ((angle >= lborder) && (angle <= rborder));
}

bool WorldObject::HasInArc(const float arcangle, float x, float y) const
{
    float arc = arcangle;
    arc = MapManager::NormalizeOrientation(arc);

    float angle = GetOrientationTo(x, y);
    angle -= m_orientation;

    // move angle to range -pi ... +pi
    angle = MapManager::NormalizeOrientation(angle);
    if (angle > M_PI)
        angle -= 2.0f * M_PI;

    float lborder = -1 * (arc / 2.0f);                       // in range -pi..0
    float rborder = (arc / 2.0f);                             // in range 0..pi
    return ((angle >= lborder) && (angle <= rborder));
}

bool WorldObject::IsInBetween(const WorldObject* obj1, const WorldObject* obj2, float size) const
{
    if (!obj1 || !obj2)
        return false;

    float dist = GetExactDist2d(obj1->GetPositionX(), obj1->GetPositionY());

    // not using sqrt() for performance
    if ((dist * dist) >= obj1->GetExactDist2dSq(obj2->GetPositionX(), obj2->GetPositionY()))
        return false;

    if (!size)
        size = GetObjectSize() / 2;

    float angle = obj1->GetOrientationTo(obj2);

    // not using sqrt() for performance
    return (size * size) >= GetExactDist2dSq(obj1->GetPositionX() + std::cos(angle) * dist, obj1->GetPositionY() + std::sin(angle) * dist);
}

    //float ground = _map->GetHeight(dest.x, dest.y, MAX_HEIGHT, true);
    //float floor = _map->GetHeight(dest.x, dest.y, pos.z, true);

    //dest.z = 1.0f +fabs(ground - pos.z) <= fabs(floor - pos.z) ? ground : floor;

            //ground = _map->GetHeight(dest.x, dest.y, MAX_HEIGHT, true);
            //floor = _map->GetHeight(dest.x, dest.y, pos.z +2.0f, true);
            //dest.z = 1.0f +fabs(ground - pos.z) <= fabs(floor - pos.z) ? ground : floor;

    //ground = _map->GetHeight(pos.x, pos.y, MAX_HEIGHT, true);
    //floor = _map->GetHeight(pos.x, pos.y, pos.z +2.0f, true);
    //pos.z = 0.5f +fabs(ground - pos.z) <= fabs(floor - pos.z) ? ground : floor;
bool WorldObject::IsPositionValid() const
{
    return Hellground::IsValidMapCoord(m_positionX,m_positionY,m_positionZ,m_orientation);
}

void WorldObject::MonsterSay(const char* text, uint32 language, uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,CHAT_MSG_MONSTER_SAY,text,language,GetName(),TargetGuid);
    BroadcastPacketInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_SAY),true);
}

void WorldObject::MonsterYell(const char* text, uint32 language, uint64 TargetGuid)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,CHAT_MSG_MONSTER_YELL,text,language,GetName(),TargetGuid);
    BroadcastPacketInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_YELL),true);
}

void WorldObject::MonsterTextEmote(const char* text, uint64 TargetGuid, bool IsBossEmote)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, text, LANG_UNIVERSAL, GetName(), TargetGuid);
    BroadcastPacketInRange(&data, sWorld.getConfig(IsBossEmote ? CONFIG_LISTEN_RANGE_YELL : CONFIG_LISTEN_RANGE_TEXTEMOTE), true);
}

void WorldObject::MonsterWhisper(const char* text, uint64 receiver, bool IsBossWhisper)
{
    Player *player = sObjectMgr.GetPlayerInWorld(receiver);
    if (!player || !player->GetSession())
        return;

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER,text,LANG_UNIVERSAL,GetName(),receiver);

    player->SendPacketToSelf(&data);
}

void WorldObject::SendPlaySound(uint32 Sound, bool OnlySelf)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << Sound;
    if (OnlySelf && GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SendPacketToSelf(&data);
    else
        BroadcastPacket(&data, true); // ToSelf ignored in this case
}

void Object::ForceValuesUpdateAtIndex(uint32 i)
{
    m_uint32Values_mirror[i] = ~GetUInt32Value(i); // makes server think the field changed
    if (m_inWorld)
    {
        if (!m_objectUpdated)
        {
            AddToClientUpdateList();
            m_objectUpdated = true;
        }
    }
}

namespace Hellground
{
    class MonsterChatBuilder
    {
        public:
            MonsterChatBuilder(WorldObject const& obj, ChatMsg msgtype, int32 textId, uint32 language, uint64 targetGUID, bool withoutPrename = false)
                : i_object(obj), i_msgtype(msgtype), i_textId(textId), i_language(language), i_targetGUID(targetGUID), i_withoutPrename(withoutPrename) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text = sObjectMgr.GetHellgroundString(i_textId, loc_idx);
                // TODO: i_object.GetName() also must be localized?
                i_object.BuildMonsterChat(&data, i_msgtype, text, i_language, i_object.GetNameForLocaleIdx(loc_idx), i_targetGUID, i_withoutPrename);
            }

        private:
            WorldObject const& i_object;
            ChatMsg i_msgtype;
            int32 i_textId;
            uint32 i_language;
            uint64 i_targetGUID;
            bool i_withoutPrename;
    };
}                                                           // namespace Hellground

void WorldObject::MonsterSay(int32 textId, uint32 language, uint64 TargetGuid)
{
    float range = sWorld.getConfig(CONFIG_LISTEN_RANGE_SAY);
    Hellground::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_SAY, textId, language, TargetGuid);
    Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> say_do(say_build);
    Hellground::CameraDistWorker<Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> > say_worker(this, range, say_do);
    TypeContainerVisitor<Hellground::CameraDistWorker<Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> >, WorldTypeMapContainer > message(say_worker);
    //cell_lock->Visit(cell_lock, message, *GetMap());
    Cell::VisitWorldObjects(this, say_worker, range);
}

void WorldObject::MonsterYell(int32 textId, uint32 language, uint64 TargetGuid)
{
    float range = sWorld.getConfig(CONFIG_LISTEN_RANGE_YELL);
    Hellground::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_YELL, textId, language, TargetGuid);
    Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> say_do(say_build);
    Hellground::CameraDistWorker<Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> > say_worker(this, range, say_do);
    Cell::VisitWorldObjects(this, say_worker, range);
}

void WorldObject::MonsterYellToZone(int32 textId, uint32 language, uint64 TargetGuid)
{
    Hellground::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_YELL, textId, language, TargetGuid);
    Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> say_do(say_build);

    uint32 zoneid = GetZoneId();

    Map::PlayerList const& pList = GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = pList.begin(); itr != pList.end(); ++itr)
        if (itr->getSource()->GetCachedZone()==zoneid)
            say_do(itr->getSource());
}

void WorldObject::MonsterTextEmote(int32 textId, uint64 TargetGuid, bool IsBossEmote, bool withoutPrename)
{
    float range = sWorld.getConfig(IsBossEmote ? CONFIG_LISTEN_RANGE_YELL : CONFIG_LISTEN_RANGE_TEXTEMOTE);
    Hellground::MonsterChatBuilder say_build(*this, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, textId, LANG_UNIVERSAL, TargetGuid, withoutPrename);
    Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> say_do(say_build);
    Hellground::CameraDistWorker<Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> > say_worker(this, range, say_do);
    Cell::VisitWorldObjects(this, say_worker, range);
}

void WorldObject::MonsterTextEmoteToZone(int32 textId, uint64 TargetGuid, bool IsBossEmote, bool withoutPrename)
{
    Hellground::MonsterChatBuilder say_build(*this, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, textId, LANG_UNIVERSAL, TargetGuid, withoutPrename);
    Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> say_do(say_build);

    uint32 zoneid = GetZoneId();

    Map::PlayerList const& pList = GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator itr = pList.begin(); itr != pList.end(); ++itr)
        if (itr->getSource()->GetCachedZone()==zoneid)
            say_do(itr->getSource());
}

void WorldObject::MonsterWhisper(int32 textId, uint64 receiver, bool IsBossWhisper)
{
    Player *player = sObjectMgr.GetPlayerInWorld(receiver);
    if (!player || !player->GetSession())
        return;

    int loc_idx = player->GetSession()->GetSessionDbLocaleIndex();
    char const* text = sObjectMgr.GetHellgroundString(textId, loc_idx);

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data,IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, text, LANG_UNIVERSAL, GetNameForLocaleIdx(loc_idx), receiver);

    player->SendPacketToSelf(&data);
}

void WorldObject::MonsterMessageToList(int32 textId, std::list<Player*> & receivers, ChatMsg chastMsg, uint64 TargetGuid)
{
    Hellground::MonsterChatBuilder say_build(*this, chastMsg, textId, LANG_UNIVERSAL, TargetGuid);
    Hellground::LocalizedPacketDo<Hellground::MonsterChatBuilder> say_do(say_build);

    for (std::list<Player*>::iterator itr = receivers.begin(); itr != receivers.end(); ++itr)
        say_do(*itr);
}

void WorldObject::BuildMonsterChat(WorldPacket *data, uint8 msgtype, int32 iTextEntry, uint32 language, char const* name, uint64 targetGuid, bool withoutPrename) const
{
    char const* text = 0;
    if(GetTypeId() == TYPEID_PLAYER)
    {
        int loc_idx = ((Player*)this)->GetSession()->GetSessionDbLocaleIndex();
        text = sObjectMgr.GetHellgroundString(iTextEntry,loc_idx);
    } else
        text = sObjectMgr.GetHellgroundStringForDBCLocale(iTextEntry);
    BuildMonsterChat(data, msgtype, text, language, name, targetGuid, withoutPrename);
    if(GetTypeId() == TYPEID_PLAYER)
        data->put(5, (uint64)0);  // BAD HACK
}

void WorldObject::BuildMonsterChat(WorldPacket *data, uint8 msgtype, char const* text, uint32 language, char const* name, uint64 targetGuid, bool withoutPrename) const
{
    bool pre = withoutPrename ? false : (msgtype==CHAT_MSG_MONSTER_EMOTE || msgtype==CHAT_MSG_RAID_BOSS_EMOTE);

    *data << (uint8)msgtype;
    *data << (uint32)language;
    *data << (uint64)GetGUID();
    *data << (uint32)0;                                     //2.1.0
    *data << (uint32)(strlen(name)+1);
    *data << name;
    *data << (uint64)targetGuid;                            //Unit Target
    if (targetGuid && !IS_PLAYER_GUID(targetGuid))
    {
        *data << (uint32)1;                                 // target name length
        *data << (uint8)0;                                  // target name
    }
    *data << (uint32)(strlen(text)+1+(pre?3:0));
    if (pre)
        data->append("%s ",3);
    *data << text;
    *data << (uint8)0;                                      // ChatTag
}

void WorldObject::BroadcastPacket(WorldPacket *data, bool toSelf)
{
    GetMap()->BroadcastPacket(this, data, toSelf);
}

void WorldObject::BroadcastPacketInRange(WorldPacket *data, float dist, bool toSelf, bool ownTeamOnly)
{
    GetMap()->BroadcastPacketInRange(this, data, dist, toSelf, ownTeamOnly);
}

void WorldObject::BroadcastPacketExcept(WorldPacket* data, Player* except)
{
    GetMap()->BroadcastPacketExcept(this, data, except);
}

void WorldObject::SendObjectDeSpawnAnim(uint64 guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << uint64(guid);
    BroadcastPacket(&data, true);
}

void WorldObject::SendGameObjectCustomAnim(uint64 guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_CUSTOM_ANIM, 8+4);
    data << uint64(guid);
    data << uint32(0);
    BroadcastPacket(&data, true);
}

Map* WorldObject::_getMap()
{
    return m_map = sMapMgr.FindMap(GetMapId(), GetAnyInstanceId());
}

TerrainInfo const* WorldObject::GetTerrain() const
{
    return GetMap()->GetTerrain();
}

void WorldObject::AddObjectToRemoveList()
{
    ASSERT(m_uint32Values);

    GetMap()->AddObjectToRemoveList(this);
}

Creature* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang,TemporarySummonType spwtype,uint32 despwtime, bool DieWithSummoner)
{
    TemporarySummon* pCreature = new TemporarySummon(GetGUID());

    pCreature->SetDieWithSummoner(DieWithSummoner);

    PlayerTeam team = TEAM_NONE;
    if (GetTypeId()==TYPEID_PLAYER)
        team = ((Player*)this)->GetTeam();

    if (!pCreature->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT), GetMap(), id, team, x, y, z, ang))
    {
        delete pCreature;
        return NULL;
    }

    pCreature->SetHomePosition(x, y, z, ang);
    pCreature->Summon(spwtype, despwtime);

    if (GetTypeId()==TYPEID_UNIT && ((Creature*)this)->IsAIEnabled)
        ((Creature*)this)->AI()->JustSummoned(pCreature);

    if (pCreature->IsAIEnabled)
    {
        pCreature->AI()->JustRespawned();

        if (GetTypeId() == TYPEID_UNIT || GetTypeId() == TYPEID_PLAYER)
            pCreature->AI()->IsSummonedBy((Unit*)this);
    }

    if (pCreature->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER && pCreature->m_spells[0])
    {
        if (GetTypeId() == TYPEID_UNIT || GetTypeId() == TYPEID_PLAYER)
            pCreature->setFaction(((Unit*)this)->getFaction());

        pCreature->CastSpell(pCreature, pCreature->m_spells[0], false, 0, 0, GetGUID());
    }

    //return the creature therewith the summoner has access to it
    return pCreature;
}

void WorldObject::SetZoneScript()
{
    if (Map *map = GetMap())
    {
        if (map->IsDungeon())
            m_zoneScript = (ZoneScript*)((InstanceMap*)map)->GetInstanceData();
        else if (!map->IsBattleGroundOrArena())
            m_zoneScript = sOutdoorPvPMgr.GetZoneScript(GetZoneId());
    }
}

Pet* Player::SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 duration)
{
    Pet* pet = new Pet(petType);

    if (petType == SUMMON_PET && pet->LoadPetFromDB(this, entry, 0, false, x, y, z, ang))
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (Unit::AuraList::const_iterator itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
        {
            if ((*itr)->GetModifier()->m_miscvalue==2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }

        if (duration > 0)
            pet->SetDuration(duration);

        if (entry == 19668)
        {
            pet->SetReactState(REACT_AGGRESSIVE);
            pet->ClearUnitState(UNIT_STAT_FOLLOW);
            pet->SendPetAIReaction(pet->GetGUID());
        }
        else if (entry == 510)
        {
            pet->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);
            pet->ApplySpellImmune(0, IMMUNITY_DISPEL, DISPEL_POISON, true);
        }

		// attack nearest target after summon
		if (pet->HasReactState(REACT_AGGRESSIVE))
			if (Unit* target = pet->SelectNearbyTarget(40.0f))
				if (pet->canStartAttack(target))
					pet->CombatStart(target);

        return NULL;
    }

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if (!entry)
    {
        delete pet;
        return NULL;
    }

    Map *map = GetMap();
    uint32 pet_number = sObjectMgr.GeneratePetNumber();
    if (!pet->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_PET), map, entry, pet_number))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: no such creature entry %u", entry);
        delete pet;
        return NULL;
    }

    pet->Relocate(x, y, z, ang);

    if (!pet->IsPositionValid())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pet->GetGUIDLow(),pet->GetEntry(),pet->GetPositionX(),pet->GetPositionY());
        delete pet;
        return NULL;
    }

    pet->SetOwnerGUID(GetGUID());
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, getFaction());

    // this enables pet details window (Shift+P)
    CreatureInfo const *cinfo = pet->GetCreatureInfo();
    if (petType==HUNTER_PET || (petType==SUMMON_PET && cinfo->type == CREATURE_TYPE_DEMON && GetClass() == CLASS_WARLOCK))
        pet->GetCharmInfo()->SetPetNumber(pet_number, true);
    else
        pet->GetCharmInfo()->SetPetNumber(pet_number, false);
    //pet->GetCharmInfo()->SetPetNumber(pet_number, false);

    map->Add((Creature*)pet);

    pet->setPowerType(POWER_MANA);
    pet->SetUInt32Value(UNIT_NPC_FLAGS, 0);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    pet->InitStatsForLevel(GetLevel());

    if(pet->GetEntry() == 21867)
    {
        pet->setPowerType(POWER_MANA);
        pet->SetMaxPower(POWER_MANA,19854);
        pet->SetPower(POWER_MANA,19854);
    }

    switch (petType)
    {
        case GUARDIAN_PET:
        case POSSESSED_PET:
            //if (pet->IsAIEnabled) //  Fix respawn AIEvent (11) for Pet's with type - guardian
            //    pet->AI()->JustRespawned();
            //    
            pet->SetUInt32Value(UNIT_FIELD_FLAGS,0);
            AddGuardian(pet);
            break;
        case SUMMON_PET:
            pet->SetUInt32Value(UNIT_FIELD_BYTES_0, 2048);
            pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 1000);
            pet->SetHealth(pet->GetMaxHealth());
            pet->SetPower(POWER_MANA, pet->GetMaxPower(POWER_MANA));
            pet->InitPetCreateSpells();
            pet->SavePetToDB(PET_SAVE_AS_CURRENT);
            SetPet(pet);
            DelayedPetSpellInitialize();
            break;
    }
    if (GetTypeId() == TYPEID_PLAYER && (GetClass() == CLASS_HUNTER || GetClass() == CLASS_WARLOCK) && pet->isControlled() && !pet->isTemporarySummoned() && (petType == SUMMON_PET || petType == HUNTER_PET))
        SetLastPetNumber(pet_number);

    if (petType == SUMMON_PET)
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (Unit::AuraList::const_iterator itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
        {
            if ((*itr)->GetModifier()->m_miscvalue==2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }
    }

    if (petType == POSSESSED_PET)
    {
        pet->GetCharmInfo()->InitEmptyActionBar(pet->GetEntry() != 21867 ? false : true);
        pet->GetCharmInfo()->InitPossessCreateSpells();
    }

    if (pet->GetEntry() == NPC_GOLEM_GUARDIAN)
    {
        pet->SetUInt32Value(UNIT_NPC_FLAGS, 1);
        pet->SetUInt32Value(UNIT_FIELD_FLAGS, 130);
    }

    if (duration > 0)
        pet->SetDuration(duration);

    return pet;
}

GameObject* WorldObject::SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime)
{
    if (!IsInWorld())
        return NULL;

    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(entry);
    if (!goinfo)
    {
        sLog.outLog(LOG_DB_ERR, "Gameobject template %u not found in database!", entry);
        return NULL;
    }
    Map *map = GetMap();
    GameObject *go = new GameObject();
    if (!go->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),entry,map,x,y,z,ang,rotation0,rotation1,rotation2,rotation3,100, GO_STATE_READY))
    {
        delete go;
        return NULL;
    }
    go->SetRespawnTime(respawnTime);
    if (GetTypeId()==TYPEID_PLAYER || GetTypeId()==TYPEID_UNIT) //not sure how to handle this
        ((Unit*)this)->AddGameObject(go);
    else
        go->SetSpawnedByDefault(false);
    map->Add(go);

    return go;
}

Creature* WorldObject::SummonTrigger(float x, float y, float z, float ang, uint32 duration, CreatureAI* (*GetAI)(Creature*), bool ignore_faction)
{
    TemporarySummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Creature* summon = SummonCreature(WORLD_TRIGGER, x, y, z, ang, summonType, duration);
    if (!summon)
        return NULL;

    //summon->SetName(GetName());
    if (GetTypeId()==TYPEID_PLAYER || GetTypeId()==TYPEID_UNIT)
    {
        summon->setFaction(ignore_faction ? 35 : ((Unit*)this)->getFaction());
        summon->SetLevel(((Unit*)this)->GetLevel());
    }

    if (GetAI)
        summon->AIM_Initialize(GetAI(summon));
    return summon;
}

void WorldObject::GetNearPoint(float &x, float &y, float &z, float searcher_bounding_radius, float distance2d, float absAngle) const
{
    absAngle -= GetOrientation();

    Position pos;
    GetValidPointInAngle(pos, distance2d+searcher_bounding_radius, absAngle, true);
    x = pos.x;
    y = pos.y;
    z = pos.z;
}

void WorldObject::UpdateVisibilityAndView()
{
    GetViewPoint().Call_UpdateVisibilityForOwner();
    UpdateObjectVisibility();
    GetViewPoint().Event_ViewPointVisibilityChanged();
}

void WorldObject::UpdateObjectVisibility(bool /*forced*/)
{
    //updates object's visibility for nearby players
    Hellground::VisibleChangesNotifier notifier(*this);
    float radius = World::GetVisibleObjectGreyDistance();

    if ( ToCorpse() != nullptr || !IsInWorld() )
        radius = MAX_VISIBILITY_DISTANCE;
    else if ( Map* map = GetMap() )
        radius += map->GetVisibilityDistance();

    Cell::VisitWorldObjects(this, notifier, radius);
}

void WorldObject::AddToClientUpdateList()
{
    GetMap()->AddUpdateObject(this);
}

void WorldObject::RemoveFromClientUpdateList()
{
    GetMap()->RemoveUpdateObject(this);
}

struct WorldObjectChangeAccumulator
{
    UpdateDataMapType &i_updateDatas;
    WorldObject &i_object;
    std::set<uint64> plr_list;

    WorldObjectChangeAccumulator(WorldObject &obj, UpdateDataMapType &d) : i_updateDatas(d), i_object(obj)
    {
        if (i_object.isType(TYPEMASK_PLAYER))
            i_object.BuildFieldsUpdate(i_object.ToPlayer(), i_updateDatas);
    }

    void Visit(CameraMapType &m)
    {
        for (CameraMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        {
            Player* owner = iter->getSource()->GetOwner();
            if (owner != &i_object && owner->HaveAtClient(&i_object))
                i_object.BuildFieldsUpdate(owner, i_updateDatas);
        }
    }

    template<class SKIP>
    void Visit(GridRefManager<SKIP> &) {}
};

void WorldObject::BuildUpdate(UpdateDataMapType& data_map)
{
     WorldObjectChangeAccumulator notifier(*this, data_map);
     Cell::VisitWorldObjects(this, notifier, GetMap()->GetVisibilityDistance(this));

     ClearUpdateMask(false);
}

void WorldObject::PlayDistanceSound( uint32 sound_id, Player* target /*= NULL*/ )
{
    WorldPacket data(SMSG_PLAY_OBJECT_SOUND,4+8);
    data << uint32(sound_id);
    data << GetGUID();
    if (target)
        target->SendPacketToSelf( &data );
    else
        BroadcastPacket( &data, true );
}

void WorldObject::PlayDirectSound( uint32 sound_id, Player* target /*= NULL*/ )
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(sound_id);
    if (target)
        target->SendPacketToSelf( &data );
    else
        BroadcastPacket( &data, true );
}

Totem* WorldObject::ToTotem()
{
    if (ToCreature() && ToCreature()->isTotem())
        return reinterpret_cast<Totem*>(this);

    return NULL;
}

const Totem* WorldObject::ToTotem() const
{
    if (ToCreature() && ToCreature()->isTotem())
        return (const Totem*)((Totem*)this);

    return NULL;
}

Pet* WorldObject::ToPet()
{
    if (ToCreature() && ToCreature()->isPet())
        return reinterpret_cast<Pet*>(this);

    return NULL;
}

const Pet* WorldObject::ToPet() const
{
    if (ToCreature() && ToCreature()->isPet())
        return (const Pet*)((Pet*)this);

    return NULL;
}

bool WorldObject::UpdateHelper::ProcessUpdate(Creature* creature)
{
    if (!creature->IsInWorld() || creature->isSpiritService())
        return false;

    Map* map = creature->GetMap();
    uint32 minUpdateTime = (map && map->IsBattleArena()) ? MIN_MAP_UPDATE_DELAY : sWorld.getConfig(CONFIG_INTERVAL_MAPUPDATE);

    return creature->m_updateTracker.timeElapsed() >= minUpdateTime;
}

bool WorldObject::UpdateHelper::ProcessUpdate(WorldObject* obj)
{
    return obj->IsInWorld();
}

//

void WorldObject::GetRandomPoint(float x, float y, float z, float distance, float &rand_x, float &rand_y, float &rand_z) const
{
    if (distance == 0)
    {
        rand_x = x;
        rand_y = y;
        rand_z = z;
        return;
    }

    Position pos;
    pos.x = x;
    pos.y = y;
    pos.z = z;

    GetValidPointInAngle(pos, frand(0.0f, distance), frand(0.0f, 2*M_PI), false);

    rand_x = pos.x;
    rand_y = pos.y;
    rand_z = pos.z;
}

void WorldObject::GetValidPointInAngle(Position &pos, float dist, float angle, bool meAsSourcePos, float allowHeightDifference) const
{
    angle += GetOrientation();

    if (meAsSourcePos)
        GetPosition(pos);

    float cosinus = cos(angle);
    float sinus = sin(angle);

    Position dest;
    dest.x = pos.x + dist * cosinus;
    dest.y = pos.y + dist * sinus;
    dest.z = pos.z;
    float tempZ;
    if (!UpdateAllowedPositionZ_WithUndermap(dest.x, dest.y, dest.z)) // if pos.z wasn't near floor -> then gotta check getObjectHitPos to this position as well, not only to dest
    {
        tempZ = pos.z; // we're trying to check objectHitPos to initial pos.z, not dest.z (which may have become lower)
        if (VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), pos.x, pos.y, pos.z + 2.0f, dest.x, dest.y, pos.z + 2.0f, dest.x, dest.y, tempZ, -0.75f))
        {
            dest.z = tempZ; // update dest z only if changed x / y
            dist = sqrt((pos.x - dest.x)*(pos.x - dest.x) + (pos.y - dest.y)*(pos.y - dest.y));
            UpdateAllowedPositionZ_WithUndermap(dest.x, dest.y, dest.z);
        }
    }

    // collision occurred, only finds objects. This does not find simple ground (finds floor(vmap) ground, however)
    tempZ = dest.z;
    if (VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), pos.x, pos.y, pos.z + 2.0f, dest.x, dest.y, dest.z + 2.0f, dest.x, dest.y, tempZ, -0.75f))
    {
        dest.z = tempZ; // update dest z only if changed x / y
        dist = sqrt((pos.x - dest.x)*(pos.x - dest.x) + (pos.y - dest.y)*(pos.y - dest.y));
        UpdateAllowedPositionZ_WithUndermap(dest.x, dest.y, dest.z);
    }

    TerrainInfo const* _map = GetTerrain();
    // dest.z does not have any plus cause we want to know for sure if it is above or below ground
    // pos.z + 2.0 because player/target might actually be under ground a little. This ensures we get the right (above/below)-ground info
    // increased z pos (checkForHeight) must correspond to one used for getObjectHitPos(), so if there's a wall on standing on ground -> it should use the same vector
    if (_map->getGridGroundHitPos(pos.x, pos.y, pos.z, dest.x, dest.y, dest.z, cosinus, sinus, dist, 2.0f))
    {
        dist = sqrt((pos.x - dest.x)*(pos.x - dest.x) + (pos.y - dest.y)*(pos.y - dest.y));
        UpdateAllowedPositionZ_WithUndermap(dest.x, dest.y, dest.z);
    }

    uint32 stepCount = ceil(dist); // 1 step for <= 1 yd. 2 steps for <= 2 yds, etc.
    float step = dist / (float)stepCount;
    for (uint32 j = 0; j < stepCount; ++j)
    {
        // do not allow too big z changes
        if (fabs(pos.z - dest.z) > allowHeightDifference)
        {
            dest.x -= step * cosinus;
            dest.y -= step * sinus;
            dest.z = pos.z;
            UpdateAllowedPositionZ_WithUndermap(dest.x, dest.y, dest.z);
            if (j == stepCount-1) // we must check last modification in here
            {
                if (fabs(pos.z - dest.z) > allowHeightDifference) // even our initial pos is damn bad - this means we're standing in "almost air" near the edge
                // Try finding allowed position near the initial pos
                {
                    Position test = dest;
                    float FourAngles = 0;
                    for (uint32 k = 0; k < 8; ++k) // it goes for 0, pi/2, pi, 3*pi/2, fifth just for check if fabs is already ok.
                    {
                        if (fabs(pos.z - test.z) > allowHeightDifference)
                        {
                            test = dest;
                            test.x += 0.4f * cos(FourAngles);
                            test.y += 0.4f * sin(FourAngles);
                            dest.z = pos.z;
                            UpdateAllowedPositionZ_WithUndermap(dest.x, dest.y, dest.z);
                            FourAngles += M_PI/4;
                        }
                        else
                        {
                            pos = test;
                            break;
                        }
                    }
                }
                else
                {
                    pos = dest;
                    break;
                }
            }
        }
        else
        {
            UpdateAllowedPositionZ_WithUndermap(dest.x, dest.y, dest.z);
            pos = dest;
            break;
        }
    }

    float x = pos.x;
    float y = pos.y;
    Hellground::NormalizeMapCoord(pos.x);
    Hellground::NormalizeMapCoord(pos.y);
    // already at allowed, cause was taken via this same function. Only update if x or y is broken OR 
    // if getObjectHitPos moved us back and z is not currently generated by UpdateAllowedPositionZ_WithUndermap
    if (x != pos.x || y != pos.y)
        UpdateAllowedPositionZ_WithUndermap(pos.x, pos.y, pos.z); 
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float &z) const
{
    float new_z = GetTerrain()->GetHeight(x,y,z,true);
    if (new_z > INVALID_HEIGHT)
        z = new_z + 0.06f;                                  // just to be sure that we are not a few pixel under the surface
}

bool WorldObject::UpdateAllowedPositionZ(float x, float y, float &z) const
{
    switch (GetTypeId())
    {
    case TYPEID_UNIT:
        {
            // non fly unit don't must be in air
            // non swim unit must be at ground (mostly speedup, because it don't must be in water and water level check less fast
            if (!((Creature const*)this)->CanFly())
            {
                bool CanSwim = ((Creature const*)this)->CanSwim();
                float ground_z = z;
                float max_z = CanSwim
                    ? GetTerrain()->GetWaterOrGroundLevel(x, y, z, &ground_z, !((Unit const*)this)->HasAuraType(SPELL_AURA_WATER_WALK))
                    : ((ground_z = GetTerrain()->GetHeight(x, y, z, true)));
                if (max_z > INVALID_HEIGHT)
                {
                    if (z > max_z)
                        z = max_z;
                    else if (z < ground_z) // this will work only if we're just 2 yds under ground (GetHeight tolerance)
                        z = ground_z;

                    return true;
                }
            }
            else
            {
                float ground_z = GetTerrain()->GetHeight(x, y, z, true);
                if (ground_z > INVALID_HEIGHT)
                {
                    if (z < ground_z) // this will work only if we're just 2 yds under ground (GetHeight tolerance)
                        z = ground_z;

                    return true;
                }
            }
            break;
        }
    case TYPEID_PLAYER:
        {
            // for server controlled moves playr work same as creature (but it can always swim)
            if (!((Player const*)this)->IsFlying())
            {
                float ground_z = z;
                float max_z = GetTerrain()->GetWaterOrGroundLevel(x, y, z, &ground_z, !((Unit const*)this)->HasAuraType(SPELL_AURA_WATER_WALK));
                if (max_z > INVALID_HEIGHT)
                {
                    if (z > max_z)
                        z = max_z;
                    else if (z < ground_z) // this will work only if we're just 2 yds under ground (GetHeight tolerance) 
                        z = ground_z;

                    return true;
                }
            }
            else
            {
                float ground_z = GetTerrain()->GetHeight(x, y, z, true);
                if (ground_z > INVALID_HEIGHT)
                {
                    if (z < ground_z) // this will work only if we're just 2 yds under ground (GetHeight tolerance)
                        z = ground_z;

                    return true;
                }
            }
            break;
        }
    default:
        {
            float ground_z = GetTerrain()->GetHeight(x, y, z, true);
            if (ground_z > INVALID_HEIGHT)
            {
                z = ground_z;

                return true;
            }
            break;
        }
    }
    return false;
}

bool WorldObject::UpdateAllowedPositionZ_WithUndermap(float x, float y, float &z, bool undermapOnly) const
{
    float floor = z;
    bool ok = UpdateAllowedPositionZ(x, y, floor);
    if (z - floor > 2.0f || !ok)
        // this might be not our floor or floor not found at all. 
        // z is allowed to be 2 yds over floor or 2 yds under it. GetHeight will not find floor above us if we're more than 2 yds under it.
        // So max "upping" possible of "floor" compared to "z" is 2, that's why not using fabs, but a simple minus. We can only get lower by more than 2 yds
    {
        // need to compare with top map height
        float ground = MAX_HEIGHT;
        if (UpdateAllowedPositionZ(x, y, ground))
        {
            if (!ok)
                z = ground;
            else if (!undermapOnly)
                z = fabs(ground - z) <= fabs(floor - z) ? ground : floor; // get whichever is closer to searched z pos
        }
        return false;
    }

    z = floor; // we're in range of 2 to the floor -> then it should be right. Return it.
    return true;
}
