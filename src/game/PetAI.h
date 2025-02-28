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

#ifndef HELLGROUND_PETAI_H
#define HELLGROUND_PETAI_H

#include "CreatureAI.h"
#include "Timer.h"

class Creature;
class Spell;

/*Charmed creatures and some others can have PetAI. They're not necessary Pet*/
class HELLGROUND_IMPORT_EXPORT PetAI : public CreatureAI
{
    public:

        explicit PetAI(Creature *c);

        void EnterEvadeMode();
        void JustDied(Unit *who) { _stopAttack(); }
        void MovementInform(uint32 type, uint32 data);
        void DoCast(Unit* victim, uint32 spellId, bool triggered = false);

        void UpdateAI(const uint32);
        static int Permissible(const Creature *);

        void ForcedAttackStart(Unit* target);
        /*void ForcedAttackStart(Unit* target)
        {
            CreatureAI::AttackStart(target);
            if (Unit* victim = me->GetVictim())
            {
                if (victim == target)
                    _forced_action_unit_guid = victim->GetGUID();
            }
        }*/
        
        void AttackStart(Unit* target);
        virtual void PrepareSpellForAutocast(uint32 spellId);
        virtual void AddSpellForAutocast(uint32 spellId, Unit* target);
        virtual bool AutocastPreparedSpells();

        bool targetHasInterruptableAura(Unit *target) const;

        void ownerOrMeAttackedBy(uint64 enemy);
        void clearEnemySet() { m_EnemySet.clear(); };

        void MoveInLineOfSight(Unit*);

    protected:

        void UpdateMotionMaster();

        Unit* _getAttackerForHelper(Unit* from) const;
        bool _needToStop(void);
        bool _isVisible(Unit *) const;
        void _stopAttack(void);
        bool forced_attack;

        void UpdateAllies();
        Unit* FindValidTarget(); // for aggresive stance
        Unit* TargetSelectHelper();

        TimeTracker i_tracker;
        std::set<uint64> m_AllySet;
        std::set<uint64> m_EnemySet;
        
        TimeTrackerSmall updateAlliesTimer;

        typedef std::pair<Unit*, Spell*> TargetSpellPair;
        std::vector<TargetSpellPair> m_targetSpellStore;

        Unit* m_owner;              // pointer updated every UpdateAI call
};

class ImpAI : public PetAI
{
    public:
        ImpAI(Creature *c) : PetAI(c), m_chasing(false) {}
        void UpdateAI(const uint32);
        static int Permissible(const Creature *);
    protected:
        bool m_chasing;
};

class FelhunterAI : public PetAI
{
    public:
        FelhunterAI(Creature *c) : PetAI(c) {}
        static int Permissible(const Creature *);
        void PrepareSpellForAutocast(uint32 spellId);

};

class WaterElementalAI : public PetAI
{
    public:
        WaterElementalAI(Creature *c) : PetAI(c) {}
        static int Permissible(const Creature *);
        void UpdateAI(const uint32);
};

#endif

