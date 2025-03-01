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

#ifndef HELLGROUND_CREATURE_EAI_MGR_H
#define HELLGROUND_CREATURE_EAI_MGR_H

#include "ace/Singleton.h"

#include "Common.h"
#include "CreatureEventAI.h"
#include "ScriptMgr.h"

 // Text Maps
typedef UNORDERED_MAP<int32, StringTextData> CreatureEventAI_TextMap;

class CreatureEventAIMgr
{
    friend class ACE_Singleton<CreatureEventAIMgr, ACE_Null_Mutex>;
    CreatureEventAIMgr(){};

    public:
        ~CreatureEventAIMgr(){};

        void LoadCreatureEventAI_Texts(bool check_entry_use);
        void LoadCreatureEventAI_Summons(bool check_entry_use);
        void LoadCreatureEventAI_Positions(bool check_entry_use);
        void LoadCreatureEventAI_Scripts(uint32 creatureId = 0);

        CreatureEventAI_Event_Map       const& GetCreatureEventAIMap()          const { return m_CreatureEventAI_Event_Map; }
        CreatureEventAI_Summon_Map      const& GetCreatureEventAISummonMap()    const { return m_CreatureEventAI_Summon_Map; }
        CreatureEventAI_Positions_Map   const& GetCreatureEventAIPositionsMap() const { return m_CreatureEventAI_Positions_Map; }
        CreatureEventAI_TextMap         const& GetCreatureEventAITextMap()      const { return m_CreatureEventAI_TextMap; }
        CreatureEventAI_EventComputedData_Map const& GetEAIComputedDataMap() const { return m_creatureEventAI_ComputedDataMap; }

    private:
        void CheckUnusedAITexts();
        void CheckUnusedAISummons();
        void CheckUnusedAIPositions();

        CreatureEventAI_Event_Map       m_CreatureEventAI_Event_Map;
        CreatureEventAI_Summon_Map      m_CreatureEventAI_Summon_Map;
        CreatureEventAI_Positions_Map   m_CreatureEventAI_Positions_Map;
        CreatureEventAI_TextMap         m_CreatureEventAI_TextMap;
        CreatureEventAI_EventComputedData_Map m_creatureEventAI_ComputedDataMap;
};

#define sCreatureEAIMgr (*ACE_Singleton<CreatureEventAIMgr, ACE_Null_Mutex>::instance())
#endif
