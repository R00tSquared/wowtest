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

#ifndef HELLGROUND_PETITIONSHANDLER_H
#define HELLGROUND_PETITIONSHANDLER_H

enum PetitionType
{
    PETITION_TYPE_NONE = 0,
    //PETITION_TYPE_1v1 = ARENA_TYPE_1v1,
    PETITION_TYPE_2v2 = ARENA_TYPE_2v2,
    PETITION_TYPE_3v3 = ARENA_TYPE_3v3,
    PETITION_TYPE_5v5 = ARENA_TYPE_5v5,
    PETITION_TYPE_GUILD = 9,
    PETITION_TYPE_REMOVE_ALL = 10
};

// Charters ID in item_template
#define GUILD_CHARTER               5863
#define ARENA_TEAM_CHARTER_2v2      23560
#define ARENA_TEAM_CHARTER_3v3      23561
#define ARENA_TEAM_CHARTER_5v5      23562

#endif

