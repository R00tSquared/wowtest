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

#include "ThreatManager.h"
#include "Unit.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Map.h"
#include "MapManager.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "UnitEvents.h"
#include "Spell.h"

//==============================================================
//================= ThreatCalcHelper ===========================
//==============================================================

// The pHatingUnit is not used yet
float ThreatCalcHelper::calcThreat(Unit* pHatedUnit, Unit* pHatingUnit, float pThreat, SpellSchoolMask schoolMask, SpellEntry const *pThreatSpell)
{
    if (pThreatSpell)
    {
        /*if (pThreatSpell->AttributesEx & SPELL_ATTR_EX_NO_THREAT)
            return 0.0f;*/

        if (Player* modOwner = pHatedUnit->GetSpellModOwner())
            modOwner->ApplySpellMod(pThreatSpell->Id, SPELLMOD_THREAT, pThreat);
    }

    float threat = pHatedUnit->ApplyTotalThreatModifier(pThreat, schoolMask);
    return threat;
}

//============================================================
//================= HostileReference =========================
//============================================================

HostileReference::HostileReference(Unit* pUnit, ThreatManager *pThreatManager, float pThreat)
{
    iThreat = pThreat;
    iTempThreatModifyer = 0.0f;
    link(pUnit, pThreatManager);
    iUnitGuid = pUnit->GetGUID();
    iOnline = true;
    iAccessible = true;
}

//============================================================
// Tell our refTo (target) object that we have a link
void HostileReference::targetObjectBuildLink()
{
    getTarget()->addHatedBy(this);
}

//============================================================
// Tell our refTo (taget) object, that the link is cut
void HostileReference::targetObjectDestroyLink()
{
    getTarget()->removeHatedBy(this);
}

//============================================================
// Tell our refFrom (source) object, that the link is cut (Target destroyed)

void HostileReference::sourceObjectDestroyLink()
{
    setOnlineOfflineState(false);
}

//============================================================
// Inform the source, that the status of the reference changed

void HostileReference::fireStatusChanged(ThreatRefStatusChangeEvent& pThreatRefStatusChangeEvent)
{
    if (getSource())
        getSource()->processThreatEvent(&pThreatRefStatusChangeEvent);
}

//============================================================

void HostileReference::addThreat(float pMod)
{
    iThreat += pMod;
    // the threat is changed. Source and target unit have to be availabe
    // if the link was cut before relink it again
    if (!isOnline())
        updateOnlineStatus();

    if (pMod != 0.0f)
    {
        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_THREAT_CHANGE, this, pMod);
        fireStatusChanged(event);
    }

    if (isValid() && pMod >= 0 && !((Creature*)getTarget())->isTotem()) // all
    {
        Unit* victim_owner = getTarget()->GetCharmerOrOwner();
        if (victim_owner && victim_owner->isAlive())
            getSource()->addThreat(victim_owner, 0.0f);     // create a threat to the owner of a pet, if the pet attacks
    }
}

//============================================================
// check, if source can reach target and set the status

void HostileReference::updateOnlineStatus()
{
    bool online = false;
    bool accessible = false;

    if (!isValid())
    {
        Unit* target = getSourceUnit()->GetMap()->GetUnit(getUnitGuid());
        if (target)
            link(target, getSource());
    }
    // only check for online status if
    // ref is valid
    // target is no player or not gamemaster
    // target is not in flight
    if (isValid()
        && (getTarget()->GetTypeId() != TYPEID_PLAYER || !((Player*)getTarget())->isGameMaster())
        && !getTarget()->HasUnitState(UNIT_STAT_TAXI_FLIGHT))
    {
        Creature* creature = (Creature*) getSourceUnit();
        online = getTarget()->isInAccessiblePlacefor (creature);
        if (!online)
        {
            if (creature->IsWithinCombatRange(getTarget(),MELEE_RANGE))
                online = true;                              // not accessible but stays online
        }
        else
            accessible = true;

    }
    setAccessibleState(accessible);
    setOnlineOfflineState(online);
}

//============================================================
// set the status and fire the event on status change

void HostileReference::setOnlineOfflineState(bool pIsOnline)
{
    if (iOnline != pIsOnline)
    {
        iOnline = pIsOnline;
        if (!iOnline)
            setAccessibleState(false);                      // if not online that not accessible as well

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ONLINE_STATUS, this);
        fireStatusChanged(event);
    }
}

//============================================================

void HostileReference::setAccessibleState(bool pIsAccessible)
{
    if (iAccessible != pIsAccessible)
    {
        iAccessible = pIsAccessible;

        ThreatRefStatusChangeEvent event(UEV_THREAT_REF_ASSECCIBLE_STATUS, this);
        fireStatusChanged(event);
    }
}

//============================================================
// prepare the reference for deleting
// this is called be the target

void HostileReference::removeReference()
{
    invalidate();

    ThreatRefStatusChangeEvent event(UEV_THREAT_REF_REMOVE_FROM_LIST, this);
    fireStatusChanged(event);
}

//============================================================

Unit* HostileReference::getSourceUnit()
{
    return (getSource()->getOwner());
}

//============================================================
//================ ThreatContainer ===========================
//============================================================

void ThreatContainer::clearReferences()
{
    for (std::list<HostileReference*>::iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
    {
        (*i)->unlink();
        delete (*i);
    }
    iThreatList.clear();
}

//============================================================
// Return the HostileReference of NULL, if not found
HostileReference* ThreatContainer::getReferenceByTarget(Unit* pVictim)
{
    HostileReference* result = NULL;
    if (!pVictim)
        return NULL;

    uint64 guid = pVictim->GetGUID();
    for (std::list<HostileReference*>::iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
    {
        if ((*i)->getUnitGuid() == guid)
        {
            result = (*i);
            break;
        }
    }

    return result;
}

//============================================================
// Add the threat, if we find the reference

HostileReference* ThreatContainer::addThreat(Unit* pVictim, float pThreat)
{
    HostileReference* ref = getReferenceByTarget(pVictim);
    if (ref)
        ref->addThreat(pThreat);
    return ref;
}

//============================================================

void ThreatContainer::modifyThreatPercent(Unit *pVictim, int32 pPercent)
{
    if (HostileReference* ref = getReferenceByTarget(pVictim))
    {
        if (pPercent < -100)
        {
            ref->removeReference();
            delete ref;
        }
        else
            ref->addThreatPercent(pPercent);
    }
}

//============================================================

bool HostileReferenceSortPredicate(const HostileReference* lhs, const HostileReference* rhs)
{
    // std::list::sort ordering predicate must be: (Pred(x,y)&&Pred(y,x))==false
    return lhs->getThreat() > rhs->getThreat();             // reverse sorting
}

//============================================================
// Check if the list is dirty and sort if necessary

void ThreatContainer::update()
{
    if (iDirty && iThreatList.size() >1)
    {
        iThreatList.sort(HostileReferenceSortPredicate);
    }
    iDirty = false;
}

bool DropAggro(Creature* pAttacker, Unit * target)
{
    if (!target)
        return false;

    // if target is immuned to melee dmg
    if (target->IsImmunedToDamage(pAttacker->GetMeleeDamageSchoolMask()))
        return true;

    // if attacker is rooted and can't attack in melee range his target
    if (!pAttacker->IsWithinMeleeRange(target) && pAttacker->IsInRoots())
        return true;

    // if target is immune to actually cast spell - i think it's not good check because we don't switch spell target to proper one
    if (pAttacker->IsNonMeleeSpellCast(false))
    {
        SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NONE;
        if (Spell* pSpell = pAttacker->m_currentSpells[CURRENT_GENERIC_SPELL])
            schoolMask = SpellSchoolMask(pSpell->GetSpellEntry()->SchoolMask);
        else if (Spell* pSpell = pAttacker->m_currentSpells[CURRENT_CHANNELED_SPELL])
            schoolMask = SpellSchoolMask(pSpell->GetSpellEntry()->SchoolMask);

        if (target->IsImmunedToDamage(schoolMask))
            return true;
    }

    // disorient and confuse effects
    if (target->HasUnitState(UNIT_STAT_CONFUSED))
        return true;

    // is this needed ? Oo if not then next check if also useless ;)
    // check if target is charmed by friendly player
    if (target->isCharmed() && pAttacker->IsFriendlyTo(target))
        return true;

    // check if target is friendly because of faction or forced reactions
    FactionTemplateEntry const* faction = pAttacker->getFactionTemplateEntry();
    if (faction && target->GetTypeId() == TYPEID_PLAYER)
    {
        ReputationRank const * rank = ((Player const*)target)->GetReputationMgr().GetForcedRankIfAny(faction);
        if (rank && (*rank) >= REP_FRIENDLY)
            return true;
    }

    // target has Spirit of Redemption aura (shapeshift effect) or should be ignored
    if (target->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION) || target->HasAuraType(SPELL_AURA_IGNORED))
        return true;

    // special cases
	if (target->HasAura(24698, 1) || // Gouge
		target->HasAura(29425, 1) || // Gouge
		target->HasAura(41086, 0) || // Ice Trap
		target->HasAura(41197, 2) || // Shield Bash
		target->HasAura(38509, 0) || // vashj stun
		target->HasAura(33130, 0) // olm death coil
		)
		return true;

    // Vengeful Spirit can't be attacked
    if (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == 23109)
        return true;

    return false;
}

//============================================================
// return the next best victim
// could be the current victim

HostileReference* ThreatContainer::selectNextVictim(Creature* pAttacker, HostileReference* pCurrentVictim)
{
    HostileReference* currentRef = NULL;
    bool found = false;
    bool noPriorityTargetFound = false;

    std::list<HostileReference*>::iterator lastRef = iThreatList.end();
    lastRef--;

    for (std::list<HostileReference*>::iterator iter = iThreatList.begin(); iter != iThreatList.end() && !found;)
    {
        currentRef = (*iter);

        Unit* target = currentRef->getTarget();
        ASSERT(target);                                     // if the ref has status online the target must be there !

        // some units are preferred in comparison to others
        if (!noPriorityTargetFound && DropAggro(pAttacker, target))
        {
            if (iter != lastRef)
            {
                // current victim is a second choice target, so don't compare threat with it below
                if (currentRef == pCurrentVictim)
                    pCurrentVictim = NULL;
                ++iter;
                continue;
            }
            else
            {
                // if we reached to this point, everyone in the threatlist is a second choice target. In such a situation the target with the highest threat should be attacked.
                noPriorityTargetFound = true;
                iter = iThreatList.begin();
                continue;
            }
        }

        if (!pAttacker->IsOutOfThreatArea(target))           // skip non attackable currently targets
        {
            if (pCurrentVictim)                              // select 1.3/1.1 better target in comparison current target
            {
                // list sorted and and we check current target, then this is best case
                if (pCurrentVictim == currentRef || currentRef->getThreat() <= 1.1f * pCurrentVictim->getThreat())
                {
                    currentRef = pCurrentVictim;            // for second case
                    found = true;
                    break;
                }

                if (currentRef->getThreat() > 1.3f * pCurrentVictim->getThreat() ||
                    currentRef->getThreat() > 1.1f * pCurrentVictim->getThreat() && pAttacker->IsWithinMeleeRange(target))
                {                                           //implement 110% threat rule for targets in melee range
                    found = true;                           //and 130% rule for targets in ranged distances
                    break;                                  //for selecting alive targets
                }
            }
            else                                            // select any
            {
                found = true;
                break;
            }
        }
        ++iter;
    }
    if (!found)
        currentRef = NULL;

    return currentRef;
}

//============================================================
//=================== ThreatManager ==========================
//============================================================

ThreatManager::ThreatManager(Unit* owner) : iCurrentVictim(NULL), iOwner(owner)
{
}

//============================================================

void ThreatManager::clearReferences()
{
    iThreatContainer.clearReferences();
    iThreatOfflineContainer.clearReferences();
    iCurrentVictim = NULL;
}

//============================================================

#define MISDIRECTION_TRIGGER_SPELL 35079

void ThreatManager::addThreat(Unit* pVictim, float pThreat, SpellSchoolMask schoolMask, SpellEntry const *pThreatSpell)
{
    //function deals with adding threat and adding players and pets into ThreatList
    //mobs, NPCs, guards have ThreatList and HateOfflineList
    //players and pets have only InHateListOf
    //HateOfflineList is used co contain unattackable victims (in-flight, in-water, GM etc.)

    // not to self
    if (pVictim == getOwner())
        return;

    // not to GM
    if (!pVictim || (pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->isGameMaster()))
        return;


    // not to dead and not for dead
    if (!pVictim->isAlive() || !getOwner()->isAlive())
        return;

    if (pThreatSpell && (pThreatSpell->AttributesEx3 & SPELL_ATTR_EX3_NO_INITIAL_AGGRO || pThreatSpell->AttributesEx & SPELL_ATTR_EX_NO_THREAT))
        return;

    ASSERT(getOwner()->GetTypeId() == TYPEID_UNIT);

    float threat = ThreatCalcHelper::calcThreat(pVictim, iOwner, pThreat, schoolMask, pThreatSpell);

    // must check > 0.0f, otherwise dead loop
    if (threat > 0.0f && pVictim->GetReducedThreatPercent())
        if (Unit *unit = pVictim->GetMisdirectionTarget())
            if (unit->GetAuraByCasterSpell(MISDIRECTION_TRIGGER_SPELL, pVictim->GetGUID()))
            {
                float reducedThreat = threat * pVictim->GetReducedThreatPercent() / 100;
                threat -= reducedThreat;

                _addThreat(unit, reducedThreat);
            }

    _addThreat(pVictim, threat);
}

void ThreatManager::_addThreat(Unit *pVictim, float threat)
{
    HostileReference* ref = iThreatContainer.addThreat(pVictim, threat);
    // Ref is not in the online refs, search the offline refs next
    if (!ref)
        ref = iThreatOfflineContainer.addThreat(pVictim, threat);

    if (!ref)                                                // there was no ref => create a new one
    {
                                                            // threat has to be 0 here
        HostileReference* hostileRef = new HostileReference(pVictim, this, 0);
        iThreatContainer.addReference(hostileRef);
        hostileRef->addThreat(threat);                 // now we add the real threat
        if (pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->isGameMaster())
            hostileRef->setOnlineOfflineState(false);  // GM is always offline
    }
}

//============================================================

void ThreatManager::modifyThreatPercent(Unit *pVictim, int32 pPercent)
{
    iThreatContainer.modifyThreatPercent(pVictim, pPercent);
}

//============================================================

Unit* ThreatManager::getHostilTarget()
{
    iThreatContainer.update();
    HostileReference* nextVictim = iThreatContainer.selectNextVictim((Creature*) getOwner(), getCurrentVictim());
    setCurrentVictim(nextVictim);
    return getCurrentVictim() != NULL ? getCurrentVictim()->getTarget() : NULL;
}

//============================================================

float ThreatManager::getThreat(Unit *pVictim, bool pAlsoSearchOfflineList)
{
    if (!pVictim)
        return 0;

    float threat = 0.0f;
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pVictim);
    if (!ref && pAlsoSearchOfflineList)
        ref = iThreatOfflineContainer.getReferenceByTarget(pVictim);
    if (ref)
        threat = ref->getThreat();
    return threat;
}

//============================================================

void ThreatManager::tauntApply(Unit* pTaunter)
{
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pTaunter);
    if (getCurrentVictim() && ref && (ref->getThreat() < getCurrentVictim()->getThreat()))
    {
        if (ref->getTempThreatModifyer() == 0.0f)
                                                            // Ok, temp threat is unused
            ref->setTempThreat(getCurrentVictim()->getThreat());
    }
}

//============================================================

void ThreatManager::tauntFadeOut(Unit *pTaunter)
{
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pTaunter);
    if (ref)
        ref->resetTempThreat();
}

//============================================================

void ThreatManager::setCurrentVictim(HostileReference* pHostileReference)
{
    iCurrentVictim = pHostileReference;
}

//============================================================
// The hated unit is gone, dead or deleted
// return true, if the event is consumed

void ThreatManager::processThreatEvent(ThreatRefStatusChangeEvent* threatRefStatusChangeEvent)
{
    threatRefStatusChangeEvent->setThreatManager(this);     // now we can set the threat manager

    HostileReference* hostileRef = threatRefStatusChangeEvent->getReference();

    switch(threatRefStatusChangeEvent->getType())
    {
        case UEV_THREAT_REF_THREAT_CHANGE:
            if ((getCurrentVictim() == hostileRef && threatRefStatusChangeEvent->getFValue()<0.0f) ||
                (getCurrentVictim() != hostileRef && threatRefStatusChangeEvent->getFValue()>0.0f))
                setDirty(true);                             // the order in the threat list might have changed
            break;
        case UEV_THREAT_REF_ONLINE_STATUS:
            if (!hostileRef->isOnline())
            {
                if (hostileRef == getCurrentVictim())
                {
                    setCurrentVictim(NULL);
                    setDirty(true);
                }
                iThreatContainer.remove(hostileRef);
                iThreatOfflineContainer.addReference(hostileRef);
            }
            else
            {
                if (getCurrentVictim() && hostileRef->getThreat() > (1.1f * getCurrentVictim()->getThreat()))
                    setDirty(true);
                iThreatContainer.addReference(hostileRef);
                iThreatOfflineContainer.remove(hostileRef);
            }
            break;
        case UEV_THREAT_REF_REMOVE_FROM_LIST:
            if (hostileRef == getCurrentVictim())
            {
                setCurrentVictim(NULL);
                setDirty(true);
            }
            if (hostileRef ->isOnline())
                iThreatContainer.remove(hostileRef);
            else
                iThreatOfflineContainer.remove(hostileRef);
            break;
    }
}
