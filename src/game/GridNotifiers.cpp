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

#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Item.h"
#include "Map.h"
#include "MapManager.h"
#include "Transports.h"
#include "ObjectAccessor.h"
#include "CellImpl.h"
#include "SpellAuras.h"

using namespace Hellground;

void VisibleNotifier::SendToSelf()
{
    Player& player = *_camera.GetOwner();
    // at this moment i_clientGUIDs have guids that not iterate at grid level checks
    // but exist one case when this possible and object not out of range: transports
    if (Transport* transport = player.GetTransport())
    {
        for (Transport::PlayerSet::const_iterator itr = transport->GetPassengers().begin(); itr != transport->GetPassengers().end(); ++itr)
        {
            if (vis_guids.find((*itr)->GetGUID()) != vis_guids.end())
            {
                vis_guids.erase((*itr)->GetGUID());

                (*itr)->UpdateVisibilityOf(*itr, &player);
                player.UpdateVisibilityOf(&player, *itr, i_data, i_visibleNow);
            }
        }
    }

    for (Player::ClientGUIDs::const_iterator it = vis_guids.begin(); it != vis_guids.end(); ++it)
    {
        player.m_clientGUIDs.erase(*it);
        i_data.AddOutOfRangeGUID(*it);
        if (IS_PLAYER_GUID(*it))
        {
            Player* plr = ObjectAccessor::GetPlayerInWorld(*it);
            if (plr && plr->IsInWorld())
                plr->UpdateVisibilityOf(plr->GetCamera().GetBody(), &player);
        }
    }

    if (!i_data.HasData())
        return;

    WorldPacket packet;
    i_data.BuildPacket(&packet);
    player.SendPacketToSelf(&packet);

    for (std::set<WorldObject*>::const_iterator it = i_visibleNow.begin(); it != i_visibleNow.end(); ++it)
    {
        if ((*it)->GetObjectGuid().IsUnit())
            player.SendInitialVisiblePackets((*it)->ToUnit());
    }
}

void VisibleChangesNotifier::Visit(CameraMapType& m)
{
    for (CameraMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        iter->getSource()->UpdateVisibilityOf(&_object);
}

void DynamicObjectUpdater::VisitHelper(Unit* target)
{
    if (!target->isAlive() || target->IsTaxiFlying())
        return;

    if (target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->isTotem())
        return;

    if (!i_dynobject.IsInMap(target))
        return;

    if (fabs(i_dynobject.GetPositionZ() - target->GetPositionZ()) > DST_AREA_AURA_REACH_Z)
        return;

    if (i_dynobject.GetExactDistSq(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ()) > i_dynobject.GetRadiusSq())
        return; // it is offlike to be able to Sap a hunter (though still tricky) when he's standing in the middle of Flare. See http://eu.battle.net/wow/en/forum/topic/2722997223 and http://www.skill-capped.com/forums/showthread.php?2045-How-to-counter-flare

    //Check targets for not_selectable unit flag and remove
    if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        return;

    // Evade target
    if (target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsInEvadeMode())
        return;

    //Check player targets and remove if in GM mode or GM invisibility (for not self casting case)
    if (target->GetTypeId() == TYPEID_PLAYER && target != i_check && (((Player*)target)->isGameMaster() || ((Player*)target)->GetVisibility() == VISIBILITY_OFF))
        return;

    if (i_dynobject.IsAffecting(target))
        return;

    SpellEntry const *spellInfo = GetSpellStore()->LookupEntry<SpellEntry>(i_dynobject.GetSpellId());

    uint32 eff_index  = i_dynobject.GetEffIndex();
    if (spellInfo->EffectImplicitTargetB[eff_index] == TARGET_DEST_DYNOBJ_ALLY
        || spellInfo->EffectImplicitTargetB[eff_index] == TARGET_UNIT_AREA_ALLY_DST) // There are no other possibilities
        // for friendly targets. ImplicitTargetA has no such types of targets.
    {
        if (!i_check->IsFriendlyTo(target))
            return;
    }
    else if (i_check->GetTypeId() == TYPEID_PLAYER)
    {
        if (i_check->IsFriendlyTo(target))
            return;
		
		if (i_check->GetDistance(target) <= CUSTOM_COMBAT_DISTANCE)
			i_check->CombatStart(target, !(spellInfo->AttributesEx3 & SPELL_ATTR_EX3_NO_INITIAL_AGGRO || spellInfo->AttributesEx & SPELL_ATTR_EX_NO_THREAT));
    }
    else
    {
        if (!i_check->IsHostileTo(target))
            return;

		if (i_check->GetDistance(target) <= CUSTOM_COMBAT_DISTANCE)
			i_check->CombatStart(target, !(spellInfo->AttributesEx3 & SPELL_ATTR_EX3_NO_INITIAL_AGGRO || spellInfo->AttributesEx & SPELL_ATTR_EX_NO_THREAT));
    }

    // Check target immune to spell or aura
    if (target->IsImmunedToSpell(spellInfo) || target->IsImmunedToSpellEffect(spellInfo->Effect[eff_index], spellInfo->EffectMechanic[eff_index]))
        return;

    if (target->preventApplyPersistentAA(spellInfo, eff_index))
        return;

	// looks like there's no "target not a player" realisation for AREA aura effects, so make a hacky one
	if (target->GetTypeId() == TYPEID_PLAYER)
	{
		switch (spellInfo->Id)
			case 37469: //Chess AOE spell
			case 37465: //Chess AOE spell
		return;
	}
		

    // Apply PersistentAreaAura on target
    PersistentAreaAura* Aur = new PersistentAreaAura(spellInfo, eff_index, NULL, target, i_dynobject.GetCaster(), NULL, i_dynobject.GetGUID());

    target->AddAura(Aur);
    i_dynobject.AddAffected(target);
}

void DynamicObjectUpdater::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        VisitHelper(itr->getSource());
}

void DynamicObjectUpdater::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        VisitHelper(itr->getSource());
}

PacketBroadcaster::PacketBroadcaster(WorldObject& src, WorldPacket* msg, Player* except /*= NULL*/, float dist /*= 0.0f*/, bool ownTeam /*= false*/ ) : _source(src), _message(msg), _dist(dist)
{
    if (except)
        playerGUIDS.insert(except->GetGUID());

   _ownTeam = ownTeam && _source.GetObjectGuid().IsPlayer();
}

void PacketBroadcaster::Visit(CameraMapType& m)
{
    for (CameraMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (_dist && !_source.IsWithinDist(iter->getSource()->GetBody(), _dist))
            continue;

        BroadcastPacketTo(iter->getSource()->GetOwner());
    }
}

void PacketBroadcaster::BroadcastPacketTo(Player* player)
{
    if (_ownTeam && _source.ToPlayer()->GetTeam() != player->GetTeam())
        return;

    if (!player->HaveAtClient(&_source))
        return;

    if (playerGUIDS.find(player->GetGUID()) == playerGUIDS.end())
    {
        if (WorldSession* session = player->GetSession())
            session->SendPacket(_message);

        playerGUIDS.insert(player->GetGUID());
    }
}

template<class T>
void ObjectUpdater::Visit(GridRefManager<T> &m)
{
    for (typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (iter->getSource()->IsInWorld())
        {
            WorldObject::UpdateHelper helper(iter->getSource());
            helper.Update(i_timeDiff); 
        }
    }
}

bool CannibalizeObjectCheck::operator()(Player* u)
{
    if (i_funit->IsFriendlyTo(u) || u->isAlive() || u->IsTaxiFlying())
        return false;

    if (i_funit->IsWithinDistInMap(u, i_range))
    {
        // If player hasn't left body -> there's only player, but no corpse
        // If player has left body -> there is player AND corpse, so we need to check for corpse, NOT THE PLAYER
        if (!u->GetCorpse()) // if player is target -> he should have no corpse
            return true;
    }
    return false;
}

bool CannibalizeObjectCheck::operator()(Creature* u)
{
    if (i_funit->IsFriendlyTo(u) || u->isAlive() || u->IsTaxiFlying() ||
        (u->GetCreatureTypeMask() & CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) == 0)
        return false;

    if (i_funit->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool CannibalizeObjectCheck::operator()(Corpse* u)
{
    // ignore bones
    if (u->GetType()==CORPSE_BONES)
        return false;

    Player* owner = ObjectAccessor::GetPlayerInWorld(u->GetOwnerGUID());

    if (!owner || i_funit->IsFriendlyTo(owner))
        return false;

    if (i_funit->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool AnyUnfriendlyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (Player* owner = sObjectAccessor.GetPlayerInWorldOrNot(i_unit->GetCharmerOrOwnerGUID()))
    {
        if (!owner->HaveAtClient(u))
            return false; // pets should be able to attack stealthed unit if only player detected them
    }
    else if (u->m_invisibilityMask && u->m_invisibilityMask & (1 << 10) &&
        !u->canDetectInvisibilityOf(i_unit))
        return false;

    if (u->isAlive() && i_unit->IsWithinDistInMap(u, i_range) && !i_unit->IsFriendlyTo(u))
        return true;
    else
        return false;
}

template void ObjectUpdater::Visit<GameObject>(GameObjectMapType &);
template void ObjectUpdater::Visit<DynamicObject>(DynamicObjectMapType &);
