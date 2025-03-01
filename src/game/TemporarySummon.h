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

#ifndef HELLGROUND_TEMPSUMMON_H
#define HELLGROUND_TEMPSUMMON_H

#include "Creature.h"
#include "ObjectAccessor.h"

class TemporarySummon : public Creature
{
    public:
        explicit TemporarySummon(uint64 summoner = 0);
        virtual ~TemporarySummon()
        {
            if (IsInWorld())
                Creature::RemoveFromWorld();
        }
        void Update(uint32 update_diff, uint32 time); 
        void Summon(TemporarySummonType type, uint32 lifetime);
        void UnSummon();

        uint64 GetSummonerGuid() const { return m_summoner ; }
        Unit* GetSummoner() const { return m_summoner ? GetMap()->GetUnit(m_summoner) : NULL; }
        void SetDieWithSummoner(bool Die) { DieWithSummoner = Die;}
    private:
        void SaveUpdateToDB()                        // overwrite of Creature::SaveUpdateToDB     - must not be called
        {
            ASSERT(false);
        }
        void SaveUpdateOrNewToDB_npc_add(uint32, uint8)                        // overwrite of Creature::SaveToDB     - must not be called
        {
            ASSERT(false);
        }
        void DeleteFromDB_npc_del()                                 // overwrite of Creature::DeleteFromDB - must not be called
        {
            ASSERT(false);
        }

    private:
        TemporarySummonType m_type;
        uint32 m_timer;
        uint32 m_lifetime;
        uint64 m_summoner;
        bool DieWithSummoner;
};

#endif
