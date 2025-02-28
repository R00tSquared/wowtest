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

#ifndef HELLGROUND_CREATUREAI_H
#define HELLGROUND_CREATUREAI_H

#include "UnitAI.h"
#include "Common.h"

class Unit;
class Creature;
class Player;
class Aura;
struct SpellEntry;
class WorldObject;
class ChatHandler;

#define TIME_INTERVAL_LOOK   5000
#define VISIBILITY_RANGE    10000

//Spell targets used by SelectSpell
enum SelectTargetType
{
    SELECT_TARGET_DONTCARE = 0,                             //All target types allowed

    SELECT_TARGET_SELF,                                     //Only Self casting

    SELECT_TARGET_SINGLE_ENEMY,                             //Only Single Enemy
    SELECT_TARGET_AOE_ENEMY,                                //Only AoE Enemy
    SELECT_TARGET_ANY_ENEMY,                                //AoE or Single Enemy

    SELECT_TARGET_SINGLE_FRIEND,                            //Only Single Friend
    SELECT_TARGET_AOE_FRIEND,                               //Only AoE Friend
    SELECT_TARGET_ANY_FRIEND,                               //AoE or Single Friend
};

//Spell Effects used by SelectSpell
enum SelectEffect
{
    SELECT_EFFECT_DONTCARE = 0,                             //All spell effects allowed
    SELECT_EFFECT_DAMAGE,                                   //Spell does damage
    SELECT_EFFECT_HEALING,                                  //Spell does healing
    SELECT_EFFECT_AURA,                                     //Spell applies an aura
};

//Selection method used by SelectTarget
enum SCEquip
{
    EQUIP_NO_CHANGE = -1,
    EQUIP_UNEQUIP   = 0
};

enum AIEventType
{
    // Usable with Event AI
    AI_EVENT_JUST_DIED          = 0,                        // Sender = Killed Npc, Invoker = Killer
    AI_EVENT_CRITICAL_HEALTH    = 1,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 10% health
    AI_EVENT_LOST_HEALTH        = 2,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 50% health
    AI_EVENT_LOST_SOME_HEALTH   = 3,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 90% health
    AI_EVENT_GOT_FULL_HEALTH    = 4,                        // Sender = Healed Npc, Invoker = Healer
    AI_EVENT_CUSTOM_EVENTAI_A   = 5,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_B   = 6,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_GOT_CCED           = 7,                        // Sender = CCed Npc, Invoker = Caster that CCed
    AI_EVENT_CUSTOM_EVENTAI_C   = 8,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_D   = 9,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_E   = 10,                       // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_F   = 11,                       // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    MAXIMAL_AI_EVENT_EVENTAI    = 12,

    // Internal Use
    AI_EVENT_CALL_ASSISTANCE    = 13,                       // Sender = Attacked Npc, Invoker = Enemy

    // Predefined for SD2
    AI_EVENT_START_ESCORT       = 100,                      // Invoker = Escorting Player
    AI_EVENT_START_ESCORT_B     = 101,                      // Invoker = Escorting Player
    AI_EVENT_START_EVENT        = 102,                      // Invoker = EventStarter
    AI_EVENT_START_EVENT_A      = 103,                      // Invoker = EventStarter
    AI_EVENT_START_EVENT_B      = 104,                      // Invoker = EventStarter

    // Some IDs for special cases in SD2
    AI_EVENT_CUSTOM_A           = 1000,
    AI_EVENT_CUSTOM_B           = 1001,
    AI_EVENT_CUSTOM_C           = 1002,
    AI_EVENT_CUSTOM_D           = 1003,
    AI_EVENT_CUSTOM_E           = 1004,
    AI_EVENT_CUSTOM_F           = 1005,
};

class HELLGROUND_IMPORT_EXPORT CreatureAI : public UnitAI
{
    protected:
        Creature * const me;
        Creature * const m_creature;

        bool UpdateVictim();
        bool UpdateVictimWithGaze();
        bool UpdateCombatState();

        void SelectNearestTarget(Unit *who);

        void SetGazeOn(Unit *target);

        Creature *DoSummon(uint32 uiEntry, const WorldLocation &pos, uint32 uiDespawntime = 30000, TemporarySummonType uiType = TEMPSUMMON_CORPSE_TIMED_DESPAWN);
        Creature *DoSummon(uint32 uiEntry, WorldObject *obj, float fRadius = 5.0f, uint32 uiDespawntime = 30000, TemporarySummonType uiType = TEMPSUMMON_CORPSE_TIMED_DESPAWN);
        Creature *DoSummonFlyer(uint32 uiEntry, WorldObject *obj, float fZ, float fRadius = 5.0f, uint32 uiDespawntime = 30000, TemporarySummonType uiType = TEMPSUMMON_CORPSE_TIMED_DESPAWN);

    public:
        explicit CreatureAI(Creature *c) : UnitAI((Unit*)c), me(c), m_creature(c), m_MoveInLineOfSight_locked(false), m_debugInfoReceiver(0) {}

        virtual ~CreatureAI() {}

        ///== Reactions At =================================

        // Called if IsVisible(Unit *who) is true at each *who move, reaction at visibility zone enter
        void MoveInLineOfSight_Safe(Unit *who);

        virtual void OnSpellInterrupt(SpellEntry const* spellInfo) {}
        // Called for reaction at stopping attack at no attackers or targets
		bool FindNextTargetExceptCurrent();
        void EnterEvadeModeAndRegen();

		virtual void OnCombatStop();

        virtual void EnterEvadeMode();

        // Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
        virtual void EnterCombat(Unit* /*enemy*/);

        // Called at any Damage from any attacker (before damage apply)
        // Note: it for recalculation damage or special reaction at damage
        // for attack reaction use AttackedBy called for not DOT damage in Unit::DealDamage also
        virtual void DamageTaken(Unit *done_by, uint32 & /*damage*/) {}

        // Called when the creature is killed
        virtual void JustDied(Unit *) {}

        // Called when the creature kills a unit
        virtual void KilledUnit(Unit *) {}

        // Called when the creature summon successfully other creature
        virtual void JustSummoned(Creature*) {}
        virtual void IsSummonedBy(Unit *summoner) {}

        virtual void SummonedCreatureDespawn(Creature* /*unit*/) {}
        virtual void SummonedCreatureDies(Creature* /*unit*/, Unit* /*killer*/) {}

        /*virtual void OnQuestAccept(Player* pPlayer, Quest const* pQuest);*/
        // Called when hit by a spell
        virtual void SpellHit(Unit*, const SpellEntry*) {}

        // Called when aura is applied
        virtual void OnAuraApply(Aura* aura, Unit* caster, bool addStack) {}

        // Called when aura is removed
        virtual void OnAuraRemove(Aura* aur, bool removeStack) {}

        // Called when spell hits a target
        virtual void SpellHitTarget(Unit* target, const SpellEntry*) {}

        //Called when creature deals damage to player
        virtual void DamageMade(Unit* target, uint32 & , bool direct_damage, uint8 school_mask) {}

        // Called when the creature is target of hostile action: swing, hostile spell landed, fear/etc)
        //virtual void AttackedBy(Unit* attacker);
        virtual bool IsEscorted() { return false; }

        // Called when creature is spawned or respawned (for reseting variables)
        virtual void JustRespawned() {}

        // Called at waypoint reached or point movement finished
        virtual void MovementInform(uint32 /*MovementType*/, uint32 /*Data*/) {}

        /*
         * Called if a temporary summoned of m_creature reach a move point
         * @param pSummoned Summoned Creature that finished some movement
         * @param uiMotionType Type of the movement (enum MovementGeneratorType)
         * @param uiData Data related to the finished movement (ie point-id)
         */
        virtual void SummonedMovementInform(Creature* /*pSummoned*/, uint32 /*uiMotionType*/, uint32 /*uiData*/) {}

        virtual void OnCharmed(bool apply);

        //virtual void SpellClick(Player *player) {}
        virtual void ownerOrMeAttackedBy(uint64 guid) {};
        // Called at reaching home after evade
        virtual void JustReachedHome();
 
        void DoZoneInCombat(float max_dist = 200.0f);
 
        // Called at text emote receive from player 
        virtual void ReceiveEmote(Player* pPlayer, uint32 text_emote) {}
        virtual void ReceiveScriptText(WorldObject *pSource, int32 iTextEntry) {}

        // For debugging AI
        virtual void GetDebugInfo(ChatHandler& reader);

        void ToggleDebug(uint64 target) { m_debugInfoReceiver = target; };
        void SendDebug(const char* fmt, ...);

        ///== Triggered Actions Requested ==================
 
        // Called when creature attack expected (if creature can and no have current victim)
        // Note: for reaction at hostile action must be called AttackedBy function.
        //virtual void AttackStart(Unit *) {}

        // Called at World update tick
        //virtual void UpdateAI(const uint32 diff) {}

        ///== State checks =================================

        // Is unit visible for MoveInLineOfSight
        //virtual bool IsVisible(Unit *) const { return false; }

        // Called when victim entered water and creature can not enter water
        //virtual bool canReachByRangeAttack(Unit*) { return false; }

        ///== Event Handling ===============================
        /**
         * Send an AI Event to nearby Creatures around
         * @param uiType number to specify the event, default cases listed in enum AIEventType
         * @param pInvoker Unit that triggered this event (like an attacker)
         * @param uiDelay  delay time until the Event will be triggered
         * @param fRadius  range in which for receiver is searched
         */
        void SendAIEventAround(AIEventType eventType, Unit* invoker, uint32 delay, float radius, uint32 miscValue = 0) const;
        /**
         * Send an AI Event to a Creature
         * @param eventType to specify the event, default cases listed in enum AIEventType
         * @param pInvoker Unit that triggered this event (like an attacker)
         * @param pReceiver Creature to receive this event
         */
        void SendAIEvent(AIEventType eventType, Unit* invoker, Creature* receiver, uint32 miscValue = 0) const;

        /**
         * Called when an AI Event is received
         * @param eventType to specify the event, default cases listed in enum AIEventType
         * @param pSender Creature that sent this event
         * @param pInvoker Unit that triggered this event (like an attacker)
         */
        virtual void ReceiveAIEvent(AIEventType /*eventType*/, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) {}

        ///== Fields =======================================

        // Pointer to controlled by AI creature
        //Creature* const m_creature;

        virtual uint32 GetData(uint32 /*id = 0*/) { return 0; }
        virtual void SetData(uint32 /*id*/, uint32 /*value*/) {}
        virtual void SetGUID(uint64 /*guid*/, int32 /*id*/ = 0) {}
        virtual uint64 GetGUID(int32 /*id*/ = 0) { return 0; }

        virtual void PassengerBoarded(Unit *who, int8 seatId, bool apply) {}
        bool _EnterEvadeMode();

        /**This used to start private event from another creature script.
        *  Different Events can be started from different cretures
        *  So we use pSender to detect who started an event and
        *  PulseEventNumber to detect what event we should start
        */
        virtual void EventPulse(Unit* /*pSender*/, uint32 /*PulseEventNumber*/) {}
        
        // Used to detect if owner of creature have killed someone and got Experience or Honor Points
        virtual void OwnerKilledAndGotXpOrHp(Unit* /*pOwner*/, Unit* /*pTarget*/) {} 

        std::string m_AIName;

        //public
        void GetRessurectPos(uint32 boss_entry, float& x, float& y, float& z);
    protected:
        virtual void MoveInLineOfSight(Unit *);


    private:
        bool m_MoveInLineOfSight_locked;
        uint64 m_debugInfoReceiver;
};

enum Permitions
{
    PERMIT_BASE_NO                 = -1,
    PERMIT_BASE_IDLE               = 1,
    PERMIT_BASE_REACTIVE           = 100,
    PERMIT_BASE_PROACTIVE          = 200,
    PERMIT_BASE_FACTION_SPECIFIC   = 400,
    PERMIT_BASE_SPECIAL            = 800
};

class EvadeFromFarEvent : public BasicEvent
{
    public:
        EvadeFromFarEvent(Creature& owner) : BasicEvent(), m_owner(owner) { }
        bool Execute(uint64 e_time, uint32 p_time);

    private:
        Creature& m_owner;
};

#endif
