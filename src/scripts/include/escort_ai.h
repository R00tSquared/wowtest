/*
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

#define DEFAULT_MAX_PLAYER_DISTANCE 50

struct Escort_Waypoint
{
    Escort_Waypoint(uint32 _id, float _x, float _y, float _z, uint32 _w)
    {
        id = _id;
        x = _x;
        y = _y;
        z = _z;
        WaitTimeMs = _w;
    }

    uint32 id;
    float x;
    float y;
    float z;
    uint32 WaitTimeMs;
};

enum eEscortState
{
    STATE_ESCORT_NONE       = 0x000,                        //nothing in progress
    STATE_ESCORT_ESCORTING  = 0x001,                        //escort are in progress
    STATE_ESCORT_INCOMBAT   = 0x002,                        //escort is in combat
    STATE_ESCORT_PAUSED     = 0x004,                        //will not proceed with waypoints before state is removed
};

struct npc_escortAI : public ScriptedAI
{
    public:
        explicit npc_escortAI(Creature* pCreature);
        ~npc_escortAI() {}

        // CreatureAI functions
        void AttackStart(Unit* who);

        void MoveInLineOfSight(Unit* who);

        void JustDied(Unit*);

        void JustRespawned();

		void DamageTaken(Unit* pDone_by, uint32& uiDamage);

        void ReturnToLastPoint();

        void EnterEvadeMode();

		void OnCombatStop();

        void UpdateAI(const uint32);                        //the "internal" update, calls UpdateEscortAI()
        virtual void UpdateEscortAI(const uint32);          //used when it's needed to add code in update (abilities, scripted events, etc)

        void MovementInform(uint32, uint32);

        // EscortAI functions
        void AddWaypoint(uint32 id, float x, float y, float z, uint32 WaitTimeMs = 0);

        virtual void WaypointReached(uint32 uiPointId) = 0;
        virtual void WaypointStart(uint32 uiPointId) {}

        void Start(bool bIsActiveAttacker = true, bool bRun = false, uint64 uiPlayerGUID = 0, const Quest* pQuest = NULL, bool bInstantRespawn = false, bool bCanLoopPath = false);

        void SetRun(bool bRun = true);
        void SetEscortPaused(bool uPaused);

        bool HasEscortState(uint32 uiEscortState) { return (EscortState & uiEscortState); }
        virtual bool IsEscorted() { return (EscortState & STATE_ESCORT_ESCORTING); }

        void SetMaxPlayerDistance(float newMax) { MaxPlayerDistance = newMax; }
        float GetMaxPlayerDistance() { return MaxPlayerDistance; }

        void SetDespawnAtEnd(bool despawn) { DespawnAtEnd = despawn; }
        void SetDespawnAtFar(bool despawn) { DespawnAtFar = despawn; }
        bool GetAttack() { return IsActiveAttacker; }//used in EnterEvadeMode override
        void SetCanAttack(bool attack) { IsActiveAttacker = attack; }
        uint64 GetEventStarterGUID() { return PlayerGUID; }
        void SetClearWaypoints(bool clear) { ClearWaypoints = clear; }
        virtual void GetDebugInfo(ChatHandler& reader);

    protected:
        Player* GetPlayerForEscort() { return (Player*)Unit::GetUnit(*m_creature, PlayerGUID); }

    private:
        bool AssistPlayerInCombat(Unit* pWho);
        bool IsPlayerOrGroupInRange();
        void FillPointMovementListForCreature();

        void AddEscortState(uint32 uiEscortState) { EscortState |= uiEscortState; }
        void RemoveEscortState(uint32 uiEscortState) { EscortState &= ~uiEscortState; }

        uint64 PlayerGUID;
        uint32 WPWaitTimer;
        uint32 PlayerCheckTimer;
        uint32 EscortState;
        float MaxPlayerDistance;

        const Quest* QuestForEscort;                     //generally passed in Start() when regular escort script.

        std::list<Escort_Waypoint> WaypointList;
        std::list<Escort_Waypoint>::iterator CurrentWP;

        bool IsActiveAttacker;                           //obsolete, determined by faction.
        bool IsRunning;                                  //all creatures are walking by default (has flag MOVEMENTFLAG_WALK)
        bool CanInstantRespawn;                          //if creature should respawn instantly after escort over (if not, database respawntime are used)
        bool CanReturnToStart;                           //if creature can walk same path (loop) without despawn. Not for regular escort quests.
        bool DespawnAtEnd;
        bool DespawnAtFar;
        bool ScriptWP;
        bool ClearWaypoints;                             //clear WaypointList and drop escort if DespawnAtEnd is off
        bool WPCheck;
};
#endif
