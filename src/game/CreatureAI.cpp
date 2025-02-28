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

#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "Creature.h"
#include "World.h"
#include "SpellMgr.h"
#include "Chat.h"
#include "GridNotifiersImpl.h"

void CreatureAI::OnCharmed(bool apply)
{
    if (!(m_creature->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_CHARM_AI))
    {
        me->NeedChangeAI = true;
        me->IsAIEnabled = false;
    }

    if (apply)
        m_creature->SetWalk(false);
}

AISpellEntryType * UnitAI::AISpellEntry;
HELLGROUND_EXPORT AISpellEntryType * GetAISpellEntry(uint32 i) { return &CreatureAI::AISpellEntry[i]; }

void CreatureAI::DoZoneInCombat(float max_dist)
{
    Unit *creature = me;

    if (!me->CanHaveThreatList() || me->IsInEvadeMode() || !me->isAlive())
        return;

    Map *pMap = me->GetMap();
    if (!pMap->IsDungeon())                                  //use IsDungeon instead of Instanceable, in case battlegrounds will be instantiated
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: DoZoneInCombat call for map that isn't an instance (creature entry = %d)", creature->GetTypeId() == TYPEID_UNIT ? ((Creature*)creature)->GetEntry() : 0);
        return;
    }

    Map::PlayerList const &plList = pMap->GetPlayers();
    if (plList.isEmpty())
        return;

    for (Map::PlayerList::const_iterator i = plList.begin(); i != plList.end(); ++i)
    {
        if (Player* pPlayer = i->getSource())
        {
            if (pPlayer->isGameMaster() || pPlayer->IsFriendlyTo(me))
                continue;

            if (pPlayer->isAlive() && me->IsWithinDistInMap(pPlayer, max_dist))
            {
                me->SetInCombatWith(pPlayer);
                pPlayer->SetInCombatWith(me);
                me->AddThreat(pPlayer, 0.0f);
            }
        }
    }
    me->SetIsDoZoneInCombatThreat();
}

// scripts does not take care about MoveInLineOfSight loops
// MoveInLineOfSight can be called inside another MoveInLineOfSight and cause stack overflow
void CreatureAI::MoveInLineOfSight_Safe(Unit *who)
{
    if (m_MoveInLineOfSight_locked == true)
        return;

    m_MoveInLineOfSight_locked = true;
    MoveInLineOfSight(who);
    m_MoveInLineOfSight_locked = false;
}

void CreatureAI::MoveInLineOfSight(Unit *who)
{  
    // Gensen: we don't need to update anything, right?
    // DANGEROUS
    if (me->isDead())
        return;

	if (!me->CanFly() && me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
		return;

	if (me->IsInEvadeMode())
		return;

	if (!me->canStartAttack(who))
		return;

    if (!me->GetVictim())
    {
		AttackStart(who);
		who->CombatStart(me);
    }
    else if (me->GetMap()->IsDungeon())
		who->CombatStart(me);
}

void CreatureAI::SelectNearestTarget(Unit *who)
{
    if (me->GetVictim() && me->GetDistanceOrder(who, me->GetVictim()) && me->canAttack(who))
    {
        float threat = me->getThreatManager().getThreat(me->GetVictim());
        me->getThreatManager().modifyThreatPercent(me->GetVictim(), -100);
        me->AddThreat(who, threat);
    }
}

void CreatureAI::EnterCombat(Unit* enemy)
{
    BasicEvent* event = new EvadeFromFarEvent(*me);
    if (me->m_Events.HasEventOfType(event))
    {
		me->m_Events.RemoveEventsOfType(event);
    }

    delete event;
}

bool EvadeFromFarEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    WorldLocation home = m_owner.GetHomePosition();
    //dima: use this mby? if (m_owner.IsWithinDistInMap(&home, 0.5))
    m_owner.NearTeleportTo(home.coord_x, home.coord_y, home.coord_z + 0.0001f, home.orientation);
    m_owner.GetMotionMaster()->MoveTargetedHome();
    return true;
}

bool CreatureAI::FindNextTargetExceptCurrent()
{
	// go evade if one attacker
	if (me->getThreatManager().getThreatList().size() == 1)
		return false;

	// or find another one
	Unit* current_target = me->GetVictim();
	{
		me->AttackStop();
		me->getThreatManager().modifyThreatPercent(current_target, -101);
		if (me->AI()->UpdateVictim())
			return true;
	}

	return false;
}
void CreatureAI::EnterEvadeModeAndRegen()
{		
	EnterEvadeMode();
	me->ModifyHealth(me->GetMaxHealth());
}

// used in escortAI
void CreatureAI::OnCombatStop()
{
}

void CreatureAI::EnterEvadeMode()
{
    if (!_EnterEvadeMode())
        return;

    sLog.outDebug("Creature %u enters evade mode.", me->GetEntry());

    me->GetMotionMaster()->MoveTargetedHome();

    // If our creature is out of CONFIG_EVADE_HOMEDIST then it'll run home 15s and after this Teleport to home pos.
    // to prevent stucking, when there are no players around, otherwise just run to start position
    //WorldLocation home = me->GetHomePosition();
    //if (!me->IsWithinDistInMap(&home, sWorld.getConfig(CONFIG_EVADE_HOMEDIST)))
    //{
    //    EvadeFromFarEvent *pEvent = new EvadeFromFarEvent(*me);
    //    me->m_Events.AddEvent(pEvent, me->m_Events.CalculateTime(15000));
    //}

    // dima: we need to check always if creature is not on thier spawn, cuz they can stuck like Kael in the roof
    EvadeFromFarEvent *pEvent = new EvadeFromFarEvent(*me);
    me->m_Events.AddEvent(pEvent, me->m_Events.CalculateTime(15000));    

    if (CreatureGroup *formation = me->GetFormation())
        formation->EvadeFormation(me);

    //me->UpdateSpeed(MOVE_RUN, false);
    Reset();
}

void CreatureAI::JustReachedHome()
{
    // me->GetMotionMaster()->Initialize();
}

void CreatureAI::GetDebugInfo(ChatHandler& reader)
{
    reader.SendSysMessage("This AI does not support debugging.");
}

void CreatureAI::SendDebug(const char* fmt, ...)
{
    if (!m_debugInfoReceiver)
        return;
    Player *target = sObjectAccessor.GetPlayerInWorldOrNot(m_debugInfoReceiver);
    if (!target)
    {
        m_debugInfoReceiver = 0;
        return;
    }

    va_list ap;
    char message[1024];
    va_start(ap, fmt);
    vsnprintf(message, 1024, fmt, ap);
    va_end(ap);

    WorldPacket data;
    uint32 messageLength = (message ? strlen(message) : 0) + 1;

    data.Initialize(SMSG_MESSAGECHAT, 100);                // guess size
    data << uint8(CHAT_MSG_SYSTEM);
    data << uint32(LANG_UNIVERSAL);
    data << uint64(0);
    data << uint32(0);
    data << uint64(0);
    data << uint32(messageLength);
    data << message;
    data << uint8(0);

    target->SendPacketToSelf(&data);
}

// ////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Event system
// ////////////////////////////////////////////////////////////////////////////////////////////////

class AiDelayEventAround : public BasicEvent
{
    public:
        AiDelayEventAround(AIEventType eventType, ObjectGuid invokerGuid, Creature& owner, std::list<Creature*> const& receivers, uint32 miscValue) :
            BasicEvent(),
            m_eventType(eventType),
            m_invokerGuid(invokerGuid),
            m_owner(owner),
            m_miscValue(miscValue)
        {
            // Pushing guids because in delay can happen some creature gets despawned => invalid pointer
            m_receiverGuids.reserve(receivers.size());
            for (std::list<Creature*>::const_iterator itr = receivers.begin(); itr != receivers.end(); ++itr)
                m_receiverGuids.push_back((*itr)->GetObjectGuid());
        }

        bool Execute(uint64 /*e_time*/, uint32 /*p_time*/) override
        {
            Unit* pInvoker = m_owner.GetMap()->GetUnit(m_invokerGuid);

            for (std::vector<ObjectGuid>::const_reverse_iterator itr = m_receiverGuids.rbegin(); itr != m_receiverGuids.rend(); ++itr)
            {
                if (Creature* pReceiver = m_owner.GetMap()->GetCreatureOrPet(*itr))
                {
                    pReceiver->AI()->ReceiveAIEvent(m_eventType, &m_owner, pInvoker, m_miscValue);
                    // Special case for type 0 (call-assistance)
                    if (m_eventType == AI_EVENT_CALL_ASSISTANCE && pInvoker && pReceiver->CanAssistTo(&m_owner, pInvoker))
                    {
                        pReceiver->SetNoCallAssistance(true);
                        pReceiver->AI()->AttackStart(pInvoker);
                    }
                }
            }
            m_receiverGuids.clear();

            return true;
        }

    private:
        AiDelayEventAround();

        AIEventType m_eventType;
        ObjectGuid m_invokerGuid;
        Creature& m_owner;
        uint32 m_miscValue;

        std::vector<ObjectGuid> m_receiverGuids;
};

void CreatureAI::SendAIEventAround(AIEventType eventType, Unit* invoker, uint32 delay, float radius, uint32 miscValue /*=0*/) const
{
    if (radius > 0)
    {
        std::list<Creature*> receiverList;

        // Allow sending custom AI events to all units in range
        if (eventType >= AI_EVENT_CUSTOM_EVENTAI_A && eventType <= AI_EVENT_CUSTOM_EVENTAI_F && eventType != AI_EVENT_GOT_CCED)
        {
            Hellground::AnyUnitInObjectRangeCheck u_check(me, radius);
            Hellground::ObjectListSearcher<Creature, Hellground::AnyUnitInObjectRangeCheck> searcher(receiverList, u_check);
            Cell::VisitGridObjects(me, searcher, radius);
        }
        /*else
        {
            // Use this check here to collect only assitable creatures in case of CALL_ASSISTANCE, else be less strict
            Hellground::AnyAssistCreatureInRangeCheck u_check(me, eventType == AI_EVENT_CALL_ASSISTANCE ? invoker : nullptr, radius);
            Hellground::ObjectListSearcher<Creature, Hellground::AnyAssistCreatureInRangeCheck> searcher(receiverList, u_check);
            Cell::VisitGridObjects(me, searcher, radius);
        }*/

        if (!receiverList.empty())
        {
            AiDelayEventAround* e = new AiDelayEventAround(eventType, invoker ? invoker->GetObjectGuid() : ObjectGuid(), *me, receiverList, miscValue);
            me->m_Events.AddEvent(e, me->m_Events.CalculateTime(delay));
        }
    }
}

void CreatureAI::SendAIEvent(AIEventType eventType, Unit* invoker, Creature* receiver, uint32 miscValue /*=0*/) const
{
    receiver->AI()->ReceiveAIEvent(eventType, me, invoker, miscValue);
}

void CreatureAI::GetRessurectPos(uint32 boss_entry, float& x, float& y, float& z)
{
    //std::map<uint32, std::vector<float>> map;
    //map = {
    //{10184, {-91.61, -214.699, -82.48}}
    //};

}