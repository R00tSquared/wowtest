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

#include "HostileRefManager.h"
#include "ThreatManager.h"
#include "Unit.h"
#include "DBCStructure.h"
#include "SpellMgr.h"

HostileRefManager::~HostileRefManager()
{
    deleteReferences();
}

//=================================================
// send threat to all my haters for the pVictim
// The pVictim is hated than by them as well
// use for buffs and healing threat functionality

void HostileRefManager::threatAssist(Unit *pVictim, float pThreat, SpellEntry const *pThreatSpell, bool pSingleTarget)
{
    if (iOwner->HasUnitState(UNIT_STAT_IGNORE_ATTACKERS))
        return;

	// don't add threat if unit in Sheep, Gouge and etc.
	if (iOwner->HasUnitState(UNIT_STAT_FLEEING) || iOwner->HasUnitState(UNIT_STAT_CONFUSED))
		return;

    if (pThreatSpell && (pThreatSpell->AttributesEx3 & SPELL_ATTR_EX3_NO_INITIAL_AGGRO || pThreatSpell->AttributesEx & SPELL_ATTR_EX_NO_THREAT))
        return;

    HostileReference* ref = nullptr;

    uint32 size = pSingleTarget ? 1 : getSize();            // if pSingleTarget do not divide threat
    ref = getFirst();
    while (ref != nullptr)
    {
        float threat = ThreatCalcHelper::calcThreat(pVictim, iOwner, pThreat, (pThreatSpell ? SpellMgr::GetSpellSchoolMask(pThreatSpell) : SPELL_SCHOOL_MASK_NORMAL), pThreatSpell);
        if (pVictim == getOwner())
            ref->addThreat(float (threat) / size);          // It is faster to modify the threat directly if possible
        else
            ref->getSource()->addThreat(pVictim, float (threat) / size);
        ref = ref->next();
    }
}

//=================================================

void HostileRefManager::addThreatPercent(int32 pValue)
{
    for (HostileReference* ref = getFirst(); ref != nullptr; ref = ref->next())
        ref->addThreatPercent(pValue);
}

//=================================================
// The online / offline status is given to the method. The calculation has to be done before

void HostileRefManager::setOnlineOfflineState(bool pIsOnline)
{
    for (HostileReference* ref = getFirst(); ref != nullptr; ref = ref->next())
        ref->setOnlineOfflineState(pIsOnline);
}

//=================================================
// The online / offline status is calculated and set

void HostileRefManager::updateThreatTables()
{
    for (HostileReference* ref = getFirst(); ref != nullptr; ref = ref->next())
        ref->updateOnlineStatus();
}

//=================================================
// The references are not needed anymore
// tell the source to remove them from the list and free the mem

void HostileRefManager::deleteReferences()
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        ref->removeReference();
        delete ref;
        ref = nextRef;
    }
}

//=================================================
// delete one reference, defined by Unit

void HostileRefManager::deleteReference(Unit *pCreature)
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner() == pCreature)
        {
            ref->removeReference();
            delete ref;
            break;
        }
        ref = nextRef;
    }
}

//=================================================
// set state for one reference, defined by Unit

void HostileRefManager::setOnlineOfflineState(Unit *pCreature,bool pIsOnline)
{
    HostileReference* ref = getFirst();
    while (ref)
    {
        HostileReference* nextRef = ref->next();
        if (ref->getSource()->getOwner() == pCreature)
        {
            ref->setOnlineOfflineState(pIsOnline);
            break;
        }
        ref = nextRef;
    }
}

//=================================================

