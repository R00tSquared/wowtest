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

#ifndef HELLGROUND_CREATUREAIIMPL_H
#define HELLGROUND_CREATUREAIIMPL_H

#include "Common.h"
#include "Platform/Define.h"
#include "TemporarySummon.h"
#include "CreatureAI.h"

#define HEROIC(n,h) (HeroicMode ? h : n)

template<class T>
inline const T& RAND(const T& v1, const T& v2)
{
    return (rand()%2) ? v1 : v2;
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3)
{
    switch (rand()%3)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4)
{
    switch (rand()%4)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5)
{
    switch (rand()%5)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6)
{
    switch (rand()%6)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7)
{
    switch (rand()%7)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8)
{
    switch (rand()%8)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9)
{
    switch (rand()%9)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9, const T& v10)
{
    switch (rand()%10)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
        case 9: return v10;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9, const T& v10, const T& v11)
{
    switch (rand()%11)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
        case 9: return v10;
        case 10: return v11;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9, const T& v10, const T& v11, const T& v12)
{
    switch (rand()%12)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
        case 9: return v10;
        case 10: return v11;
        case 11: return v12;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9, const T& v10, const T& v11, const T& v12, const T& v13)
{
    switch (rand()%13)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
        case 9: return v10;
        case 10: return v11;
        case 11: return v12;
        case 12: return v13;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9, const T& v10, const T& v11, const T& v12, const T& v13, const T& v14)
{
    switch (rand()%14)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
        case 9: return v10;
        case 10: return v11;
        case 11: return v12;
        case 12: return v13;
        case 13: return v14;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9, const T& v10, const T& v11, const T& v12, const T& v13, const T& v14, const T& v15)
{
    switch (rand()%15)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
        case 9: return v10;
        case 10: return v11;
        case 11: return v12;
        case 12: return v13;
        case 13: return v14;
        case 14: return v15;
    }
}

template<class T>
inline
const T& RAND(const T& v1, const T& v2, const T& v3, const T& v4, const T& v5, const T& v6, const T& v7, const T& v8,
              const T& v9, const T& v10, const T& v11, const T& v12, const T& v13, const T& v14, const T& v15, const T& v16)
{
    switch (rand()%16)
    {
        default:
        case 0: return v1;
        case 1: return v2;
        case 2: return v3;
        case 3: return v4;
        case 4: return v5;
        case 5: return v6;
        case 6: return v7;
        case 7: return v8;
        case 8: return v9;
        case 9: return v10;
        case 10: return v11;
        case 11: return v12;
        case 12: return v13;
        case 13: return v14;
        case 14: return v15;
        case 15: return v16;
    }
}

class EventMap : private std::map<uint32, uint32>
{
    private:
        uint32 m_time, m_phase;
    public:
        EventMap() : m_phase(0), m_time(0) {}

        uint32 GetTimer() const
        {
            return m_time;
        }

        EventMap& Reset() {
            clear();
            m_time = 0;
            m_phase = 0;
            return *this;
        }

        EventMap& Update(uint32 time)
        {
            m_time += time;
            return *this;
        }

        EventMap& SetPhase(uint32 phase)
        {
            if (phase && phase < 9)
                m_phase = (1 << (phase + 24));
            return *this;
        }

        uint32 GetPhase() const
        {
            return m_phase;
        }

        EventMap& ScheduleEvent(uint32 eventId, uint32 time, uint32 gcd = 0, uint32 phase = 0)
        {
            time += m_time;
            if (gcd && gcd < 9)
                eventId |= (1 << (gcd + 16));
            if (phase && phase < 9)
                eventId |= (1 << (phase + 24));
            iterator itr = find(time);
            while (itr != end())
            {
                ++time;
                itr = find(time);
            }
            insert(std::make_pair(time, eventId));
            return *this;
        }

        EventMap& RescheduleEvent(uint32 eventId, uint32 time, uint32 gcd = 0, uint32 phase = 0)
        {
            CancelEvent(eventId);
            ScheduleEvent(eventId, time, gcd, phase);
            return *this;
        }

        EventMap& RepeatEvent(uint32 time)
        {
            if (empty())
                return *this;
            uint32 eventId = begin()->second;
            erase(begin());
            time += m_time;
            iterator itr = find(time);
            while (itr != end())
            {
                ++time;
                itr = find(time);
            }
            insert(std::make_pair(time, eventId));
            return *this;
        }

        void PopEvent()
        {
            erase(begin());
        }

        uint32 ExecuteEvent()
        {
            while (!empty())
            {
                if (begin()->first > m_time)
                    return 0;
                else if (m_phase && (begin()->second & 0xFF000000) && !(begin()->second & m_phase))
                    erase(begin());
                else
                {
                    uint32 eventId = (begin()->second & 0x0000FFFF);
                    erase(begin());
                    return eventId;
                }
            }
            return 0;
        }

        uint32 GetEvent()
        {
            while (!empty())
            {
                if (begin()->first > m_time)
                    return 0;
                else if (m_phase && (begin()->second & 0xFF000000) && !(begin()->second & m_phase))
                    erase(begin());
                else
                {
                    return (begin()->second & 0x0000FFFF);
                }
            }
            return 0;
        }

        EventMap& DelayEvents(uint32 time, uint32 gcd)
        {
            time += m_time;
            gcd = (1 << (gcd + 16));
            for (iterator itr = begin(); itr != end();)
            {
                if (itr->first >= time)
                    break;
                if (itr->second & gcd)
                {
                    ScheduleEvent(time, itr->second);
                    erase(itr++);
                }
                else
                    ++itr;
            }
            return *this;
        }

        EventMap& CancelEvent(uint32 eventId)
        {
            for (iterator itr = begin(); itr != end();)
            {
                if (eventId == (itr->second & 0x0000FFFF))
                    erase(itr++);
                else
                    ++itr;
            }
            return *this;
        }

        EventMap& CancelEventsByGCD(uint32 gcd)
        {
            for (iterator itr = begin(); itr != end();)
            {
                if (itr->second & gcd)
                    erase(itr++);
                else
                    ++itr;
            }
            return *this;
        }
};

enum AITarget
{
    AITARGET_SELF,
    AITARGET_VICTIM,
    AITARGET_ENEMY,
    AITARGET_ALLY,
    AITARGET_BUFF,
    AITARGET_DEBUFF,
};

enum AICondition
{
    AICOND_AGGRO,
    AICOND_COMBAT,
    AICOND_DIE,
};

#define AI_DEFAULT_COOLDOWN 5000

struct AISpellEntryType
{
    AISpellEntryType() : target(AITARGET_SELF), condition(AICOND_COMBAT)
        , cooldown(AI_DEFAULT_COOLDOWN), realCooldown(0), maxRange(0.0f){}
    AITarget target;
    AICondition condition;
    uint32 cooldown;
    uint32 realCooldown;
    float maxRange;
};

HELLGROUND_IMPORT_EXPORT AISpellEntryType * GetAISpellEntry(uint32 i);


inline void CreatureAI::SetGazeOn(Unit *target)
{
    if (me->canAttack(target))
    {
        AttackStart(target);
        me->SetReactState(REACT_AGGRESSIVE);
    }
}

inline bool CreatureAI::UpdateVictimWithGaze()
{
    if (!me->IsInCombat())
        return false;

    if (me->HasReactState(REACT_PASSIVE))
    {
        if (me->GetVictim())
            return true;
        else
            me->SetReactState(REACT_AGGRESSIVE);
    }

    if (Unit *victim = me->SelectVictim())
        AttackStart(victim);
    return me->GetVictim();
}

inline bool CreatureAI::UpdateCombatState()
{
    if (!me->IsInCombat())
        return false;

    if (!me->HasReactState(REACT_PASSIVE))
    {
        if (Unit *victim = me->SelectVictim())
            AttackStart(victim);
        return me->GetVictim();
    }
    else if (me->getThreatManager().isThreatListEmpty())
    {
        EnterEvadeMode();
        me->SetReactState(REACT_PASSIVE);
        return false;
    }

    return true;
}

inline bool CreatureAI::UpdateVictim()
{
    if (!me->IsInCombat() || !me->isAlive())
        return false;

    bool outofthreat = me->IsOutOfThreatArea(me->GetVictim());

    if (!me->HasReactState(REACT_PASSIVE))
    {
        if (Unit *pVictim = me->SelectVictim())
            AttackStart(pVictim);
    }
    else if (me->getThreatManager().isThreatListEmpty() || outofthreat)
    {
        EnterEvadeMode();
        return false;
    }
    else if (me->IsInEvadeMode())
        return false;

    if (!me->IsWithinMeleeRange(m_creature->GetVictim()) && me->IsInRoots() && !me->IsNonMeleeSpellCast(true) &&
        ((me->GetMap()->Instanceable() && me->GetCreatureInfo()->rank != CREATURE_ELITE_WORLDBOSS) || (!me->GetMap()->Instanceable())))
    {
        {
            if (!SelectUnit(SELECT_TARGET_RANDOM, 0, NOMINAL_MELEE_RANGE))
            {
                me->SetSelection(0);
                me->SetFacingTo(me->GetOrientationTo(me->GetVictim()));
                return false;
            }
        }
    }

    if (me->HasUnitState(UNIT_STAT_LOST_CONTROL))
    {
        me->SetSelection(0);
        return false;
    }

    if (me->GetVictim() && !outofthreat)
    {
        if (me->IsNonMeleeSpellCast(false))
            return true;
        else
        {
            if (!me->HasUnitState(UNIT_STAT_CANNOT_TURN) && !me->HasReactState(REACT_PASSIVE) && me->GetSelection() != me->getVictimGUID() && !me->hasIgnoreVictimSelection())
                me->SetSelection(me->getVictimGUID());
        }
    }

    return me->GetVictim();
}

/*
inline bool CreatureAI::UpdateVictim()
{
    if (!me->IsInCombat())
        return false;
    if (Unit *victim = me->SelectVictim())
        AttackStart(victim);
    return me->GetVictim();
}
*/

inline bool CreatureAI::_EnterEvadeMode()
{
    if (!me->isAlive())
        return false;

    // sometimes bosses stuck in combat?
    me->DeleteThreatList();
    me->CombatStop(true);
    me->SetLastDamagedTime(0);
    //me->ResetPlayerDamageReq();

    if (me->IsInEvadeMode())
        return false;

    me->RemoveAllAuras();
    me->LoadCreaturesAddon();
    me->SetLootRecipient(NULL);
    me->SetReactState(REACT_AGGRESSIVE);

    return true;
}

inline void UnitAI::DoCast(Unit* victim, uint32 spellId, bool triggered)
{
    if (!victim || me->HasUnitState(UNIT_STAT_CASTING) && !triggered)
        return;

    me->CastSpell(victim, spellId, triggered);
}

inline void UnitAI::DoCastVictim(uint32 spellId, bool triggered)
{
    me->CastSpell(me->GetVictim(), spellId, triggered);
}

inline void UnitAI::DoCastAOE(uint32 spellId, bool triggered)
{
    if (!triggered && me->HasUnitState(UNIT_STAT_CASTING))
        return;

    me->CastSpell((Unit*)NULL, spellId, triggered);
}

inline Creature *CreatureAI::DoSummon(uint32 uiEntry, const WorldLocation &pos, uint32 uiDespawntime, TemporarySummonType uiType)
{
    return me->SummonCreature(uiEntry, pos.coord_x, pos.coord_y, pos.coord_z, pos.orientation, uiType, uiDespawntime);
}

inline Creature *CreatureAI::DoSummon(uint32 uiEntry, WorldObject* obj, float fRadius, uint32 uiDespawntime, TemporarySummonType uiType)
{
    WorldLocation pos;
    obj->GetNearPoint(pos.coord_x,pos.coord_y,pos.coord_z,obj->GetObjectSize(), fRadius);
    return me->SummonCreature(uiEntry, pos.coord_x, pos.coord_y, pos.coord_z, pos.orientation, uiType, uiDespawntime);
}

inline Creature *CreatureAI::DoSummonFlyer(uint32 uiEntry, WorldObject *obj, float _fZ, float fRadius, uint32 uiDespawntime, TemporarySummonType uiType)
{
    WorldLocation pos;
    obj->GetNearPoint(pos.coord_x,pos.coord_y,pos.coord_z,obj->GetObjectSize(), fRadius);
    pos.coord_z += _fZ;
    return me->SummonCreature(uiEntry, pos.coord_x, pos.coord_y, pos.coord_z, pos.orientation, uiType, uiDespawntime);
}

#endif

