/*
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

#ifndef HELLGROUND_ZONE_SCRIPT_H
#define HELLGROUND_ZONE_SCRIPT_H

#include "Common.h"
#include "Creature.h"

//struct CreatureData;
class Creature;
class GameObject;

class HELLGROUND_IMPORT_EXPORT ZoneScript
{
    public:
        explicit ZoneScript() {}

        virtual uint32 GetCreatureEntry(uint32 guidlow, const CreatureData *data) { return data->id; }
        virtual uint32 GetGameObjectEntry(uint32 guidlow, uint32 entry) { return entry; }

        virtual void OnCreatureCreate(Creature *, bool add) {}
        virtual void OnCreatureDeath(Creature* /*creature*/) {}
        virtual void OnGameObjectCreate(GameObject *go, bool add) {}

        //All-purpose data storage 64 bit
        virtual uint64 GetData64(uint32 /*DataId*/) { return 0; }
        virtual void SetData64(uint32 /*DataId*/, uint64 /*Value*/) {}

        //All-purpose data storage 32 bit
        virtual uint32 GetData(uint32 /*DataId*/) { return 0; }
        virtual void SetData(uint32 /*DataId*/, uint32 /*Value*/) {}

        virtual void ProcessEvent(GameObject *obj, uint32 eventId) {}
};

#endif
