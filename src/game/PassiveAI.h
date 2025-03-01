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

#ifndef HELLGROUND_PASSIVEAI_H
#define HELLGROUND_PASSIVEAI_H

#include "CreatureAI.h"
//#include "CreatureAIImpl.h"

class HELLGROUND_IMPORT_EXPORT PassiveAI : public CreatureAI
{
    public:
        explicit PassiveAI(Creature *c);

        void MoveInLineOfSight(Unit *) {}
        void AttackStart(Unit *) {}
        void UpdateAI(const uint32);

        static int Permissible(const Creature *) { return PERMIT_BASE_IDLE;  }
};

class PossessedAI : public CreatureAI
{
    public:
        explicit PossessedAI(Creature *c);

        void MoveInLineOfSight(Unit *) {}
        void AttackStart(Unit *target);
        void UpdateAI(const uint32);

        void JustDied(Unit*);
        void KilledUnit(Unit* victim);

        static int Permissible(const Creature *) { return PERMIT_BASE_IDLE;  }
};

class HELLGROUND_IMPORT_EXPORT NullCreatureAI : public CreatureAI
{
    public:
        explicit NullCreatureAI(Creature *c);

        void MoveInLineOfSight(Unit *) {}
        void AttackStart(Unit *) {}
        void UpdateAI(const uint32) {}
        void EnterEvadeMode() {}
        void OnCharmed(bool apply) {}

        static int Permissible(const Creature *) { return PERMIT_BASE_IDLE;  }
};

class CritterAI : public PassiveAI
{
    public:
        explicit CritterAI(Creature *c) : PassiveAI(c) {}

        void DamageTaken(Unit *done_by, uint32 & /*damage*/);
        void EnterEvadeMode();
};

class HELLGROUND_IMPORT_EXPORT TriggerAI : public NullCreatureAI
{
    public:
        explicit TriggerAI(Creature *c) : NullCreatureAI(c) {}
        void IsSummonedBy(Unit *summoner);
        virtual void MoveInLineOfSight(Unit *) {};
};
#endif
