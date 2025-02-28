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

#include "Tools.h"
#include "SharedDefines.h"

// THIS CAN BE A LOT FASTER
bool readGUID(WorldPacket & data, uint64& guid)
{
    if (data.rpos()+1 > data.size())
        return false;

    uint8 guidmark=0;
    uint8 bit;
    uint8 shiftdata=0x1;
    uint64 Temp=0;

    guid = 0;

    data >> guidmark;
    for (int i=0;i<8;i++)
    {
        if (guidmark & shiftdata)
        {
            Temp = 0;

            if (data.rpos()+1 > data.size())
                return false;

            data >> bit;
            Temp = bit;
            Temp <<= i*8;
            guid |= Temp;
        }
        shiftdata=shiftdata<<1;
    }

    return true;
}

void  writeGUID(WorldPacket & data, uint64 & guid)
{
    uint8 RAWmask = 0;
    uint8 PackedGuid[8] = {0,0,0,0,0,0,0,0};

    int j = 1;
    uint8 * test = (uint8*)&guid;

    if (*test)
    {
        PackedGuid[j] = *test;
        RAWmask |= 1;
        ++j;
    }
    if (*(test+1))
    {
        PackedGuid[j] = *(test+1);
        RAWmask |= 2;
        ++j;
    }
    if (*(test+2))
    {
        PackedGuid[j] = *(test+2);
        RAWmask |= 4;
        ++j;
    }
    if (*(test+3))
    {
        PackedGuid[j] = *(test+3);
        RAWmask |= 8;
        ++j;
    }
    if (*(test+4))
    {
        PackedGuid[j] = *(test+4);
        RAWmask |= 16;
        ++j;
    }
    if (*(test+5))
    {
        PackedGuid[j] = *(test+5);
        RAWmask |= 32;
        ++j;
    }
    if (*(test+6))
    {
        PackedGuid[j] = *(test+6);
        RAWmask |= 64;
        ++j;
    }
    if (*(test+7))
    {
        PackedGuid[j] = *(test+7);
        RAWmask |= 128;
        ++j;
    }
    PackedGuid[0] = RAWmask;

    data.append(PackedGuid,j);
}

bool IsLowHeroicDungeonOrNonactualRaid(uint32 map, bool only_solo_dungeons)
{
    switch (map)
    {
    // solo dungeons
    case 540: // Hellfire Citadel: The Shattered Halls
    case 542: // Hellfire Citadel: The Blood Furnace
    case 543: // Hellfire Citadel: Ramparts
    case 545: // Coilfang: The Steamvault
    case 546: // Coilfang: The Underbog
    case 547: // Coilfang: The Slave Pens
    case 552: // Tempest Keep: The Arcatraz
    case 553: // Tempest Keep: The Botanica
    case 554: // Tempest Keep: The Mechanar
    case 555: // Auchindoun: Shadow Labyrinth
    case 556: // Auchindoun: Sethekk Halls
    case 557: // Auchindoun: Mana-Tombs
    case 558: // Auchindoun: Auchenai Crypts
    case 560: // Old Hillsbrad Foothills
    case 269: // The Black Morass
    case 585: // Magister's Terrace
        return true;

    // non-actual classic raids
    case MAP_AQ_TEMPLE:
    case MAP_AQ_RUINS:
    case MAP_ZG:
    case MAP_MK:
    case MAP_BWL:
        return only_solo_dungeons ? false : true;
    }

    return false;
}

//bool HELLGROUND_IMPORT_EXPORT IsCustomHeroicMap(uint32 map)
//{
//	switch (map)
//    {
//	case 532: // Karazhan (10)
//	case 544: // Magtheridon's Lair (25)
//	case 565: // Gruul's Lair (25)
//	case 548: // Serpentshrine Cavern (25)
//	case 550: // Tempest Keep (25)
//	case 568: // Zul'Aman (10)
//	case 534: // Hyjal (25)
//	case 564: // Black Temple (25)
//	case 580: // The Sunwell (25)
//        return true;
//    }
//
//    return false;
//}

bool IsRaidChest(uint32 entry)
{
    switch (entry)
    {
    case TEST_CHEST:
    //case RARE_RAID_CHEST:
    case EPIC_RAID_CHEST:
    case LEGENDARY_RAID_CHEST:
        return true;
    }

    return false;
}

bool IsCustomChestRequiredKeys(uint32 entry)
{
    for (const auto& chestKeys : ChestsRequiredKeys)
    {
        if (chestKeys.chest == entry)
            return true;
    }

    return false;
}

// item rates disabled
bool IsCustomChest(uint32 entry)
{
    switch (entry)
    {
    case TEST_CHEST:
    case BONUS_CHEST:
    case ANTIQUE_CHEST:
    case REFERRAL_CHEST:
    case SUBSCRIBER_CHEST:
    //case RARE_RAID_CHEST:
    case EPIC_RAID_CHEST:
    case LEGENDARY_RAID_CHEST:
    case AZERITE_CHEST:
    case GHOST_CHEST:
    case OLD_MAN_CHEST:
    case VOTE_CHEST:
        return true;
    }

    return false;
};

const char* GetBGTeamName(PlayerTeam team)
{
    if (team == ALLIANCE)
        return "ALLIANCE";
    else if (team == HORDE)
        return "HORDE";
    else
        return "NONE";

    return std::to_string(team).c_str();
}

//bool IsBonusQuestAndRewardItem(uint32 quest, uint32 item)
//{
//    uint8 i = 0;
//    
//    switch (quest)
//    {
//    case 693008: // Naxx
//    case 693006: // T5
//    case 693007: // T6 
//    case 693010: // Heroic dungeons
//        ++i;
//    }
//
//    switch (item)
//    {
//    case EPIC_KEY:
//    //case RARE_KEY:
//    case LEGENDARY_KEY:
//    case BUFF_SCROLL:
//    case GHOST_KEY:
//        ++i;
//    }
//
//    return i == 2;
//}