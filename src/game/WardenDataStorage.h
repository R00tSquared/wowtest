/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
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

#ifndef HELLGROUND_WARDEN_DATA_STORAGE_H
#define HELLGROUND_WARDEN_DATA_STORAGE_H

#include "ace/Singleton.h"

#include <map>
#include "Auth/BigNumber.h"

struct WardenData
{
    uint8 Type;
    BigNumber i;
    uint32 Address;                                         // PROC_CHECK, MEM_CHECK, PAGE_CHECK
    uint8 Length;                                           // PROC_CHECK, MEM_CHECK, PAGE_CHECK
    std::string str;                                        // LUA, MPQ, DRIVER
};

struct WardenDataResult
{
    BigNumber res;                                          // MEM_CHECK
};

class WardenDataStorage
{
    friend class ACE_Singleton<WardenDataStorage, ACE_Null_Mutex>;
    WardenDataStorage();

    public:
        ~WardenDataStorage();

    private:
        std::map<uint32, WardenData*> data_map;
        uint32* data_map_ids; // count is data_map.size()
        std::map<uint32, WardenDataResult*> result_map;
        std::vector<uint32> memCheckIds;

    public:
        WardenData * GetWardenDataById(uint32 Id) const;
        const uint32* GetWardenDataIds() const { return data_map_ids; }; // returns only for read
        const uint32 GetWardenDataIdsCnt() const { return data_map.size(); };
        WardenDataResult * GetWardenResultById(uint32 Id) const;
        std::vector<uint32> GetMemCheckIds() { return memCheckIds; } // makes copy
        void Init();
        void LoadWardenDataResult(bool reload = false);

    private:
        void Cleanup();
};

#define sWardenDataStorage (*ACE_Singleton<WardenDataStorage, ACE_Null_Mutex>::instance())
#endif
