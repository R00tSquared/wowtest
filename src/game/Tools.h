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

#include "Common.h"
#include "WorldPacket.h"

bool readGUID(WorldPacket & data, uint64& guid);
void    writeGUID(WorldPacket & data, uint64 & guid);

//bool HELLGROUND_IMPORT_EXPORT IsCustomHeroicMap(uint32 map);
bool IsLowHeroicDungeonOrNonactualRaid(uint32 map, bool only_solo_dungeons);
bool IsCustomChest(uint32 entry);
bool IsRaidChest(uint32 entry);
bool IsCustomChestRequiredKeys(uint32 entry);
const char* GetBGTeamName(PlayerTeam team);
//bool IsBonusQuestAndRewardItem(uint32 quest, uint32 item);