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

#ifndef HELLGROUND_BATTLEGROUNDRL_H
#define HELLGROUND_BATTLEGROUNDRL_H

class BattleGround;

enum BattleGroundRLObjectTypes
{
    BG_RL_OBJECT_DOOR_1         = 0,
    BG_RL_OBJECT_DOOR_2         = 1,
    BG_RL_OBJECT_BUFF_1         = 2,
    BG_RL_OBJECT_BUFF_2         = 3,
    BG_RL_OBJECT_T_CR_1         = 4,
    BG_RL_OBJECT_T_CR_2         = 5,
    BG_RL_OBJECT_MAX            = 6
};

enum BattleGroundRLObjects
{
    BG_RL_OBJECT_TYPE_DOOR_1    = 185918,
    BG_RL_OBJECT_TYPE_DOOR_2    = 185917,
    BG_RL_OBJECT_TYPE_BUFF_1    = 184663,
    BG_RL_OBJECT_TYPE_BUFF_2    = 184664,
    BG_RL_OBJECT_TYPE_CRYS_1    = 900000
};

class BattleGroundRLScore : public BattleGroundScore
{
    public:
        BattleGroundRLScore() {};
        virtual ~BattleGroundRLScore() {};
        //TODO fix me
};

class BattleGroundRL : public BattleGround
{
    friend class BattleGroundMgr;

    public:
        BattleGroundRL();
        ~BattleGroundRL();
        void Update(uint32 diff);

        /* inherited from BattlegroundClass */
        virtual void AddPlayer(Player *plr);

        void RemovePlayer(Player *plr, uint64 guid);
        void HandleAreaTrigger(Player *Source, uint32 Trigger);
        bool SetupBattleGround();
        virtual void ResetBGSubclass();
        virtual void FillInitialWorldStates(WorldPacket &d);
        void HandleKillPlayer(Player* player, Player *killer);
        bool HandlePlayerUnderMap(Player * plr, float z);
};
#endif

