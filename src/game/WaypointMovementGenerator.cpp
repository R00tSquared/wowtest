// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
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
//Basic headers
#include "WaypointMovementGenerator.h"
//Extended headers
#include "ObjectMgr.h"
#include "ScriptMgr.h"
//Creature-specific headers
#include "Creature.h"
#include "CreatureAI.h"
//Player-specific
#include "Player.h"
#include "TemporarySummon.h"

#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

void WaypointMovementGenerator<Creature>::LoadPath(Creature &creature)
{
    if (!_pathId)
        _pathId = creature.GetWaypointPath();

    if (!_path)
        _path = sWaypointMgr.GetPath(_pathId);
}

void WaypointMovementGenerator<Creature>::Initialize(Creature &creature)
{
    LoadPath(creature);

    _nextMoveTime.Reset(0);
    creature.StopMoving();

    creature.addUnitState(UNIT_STAT_ROAMING);

    _pathFinding = !creature.HasUnitState(UNIT_STAT_IGNORE_PATHFINDING) && creature.GetMap()->WaypointMovementPathfinding();

    if (creature.IsFormationLeader())
        creature.GetFormation()->ClearMovingUnits();

    if (creature.GetMap()->WaypointMovementAutoActive())
        creature.setActive(true, ACTIVE_BY_WAYPOINT_MOVEMENT);
}

void WaypointMovementGenerator<Creature>::Finalize(Creature &creature)
{
    creature.StopMoving();

    creature.ClearUnitState(UNIT_STAT_ROAMING);
    creature.SetWalk(false);
    creature.setActive(false, ACTIVE_BY_WAYPOINT_MOVEMENT);
}

bool WaypointMovementGenerator<Creature>::atNode(Creature& creature)
{
    if (_isArrivalDone)
        return true;

    const WaypointData *node = _path->at(_currentNode);
    if (node->event_id && urand(0, 99) < node->event_chance)
        creature.GetMap()->ScriptsStart(sWaypointScripts, node->event_id, &creature, NULL/*, false*/);

    _nextMoveTime.Reset(node->delay);

    if(node->o != 0.0f)
    {
        creature.SetOrientation(node->o);
        creature.SetFacingTo(node->o);
    }

    if(node->emote != 0)
        creature.HandleEmote(node->emote);
    else
        creature.HandleEmote(0);

    // Inform script
    if (creature.IsAIEnabled)
        creature.AI()->MovementInform(WAYPOINT_MOTION_TYPE, _currentNode);

    if (_repeating)
        creature.SetHomePosition(node->x, node->y, node->z, node->o != 0.0f ? node->o : creature.GetOrientation());

    if (creature.IsTemporarySummon())
    {
        TemporarySummon* pSummon = (TemporarySummon*)&creature;
        if (Creature* pSummoner = creature.GetMap()->GetCreature(pSummon->GetSummonerGuid()))
            if (pSummoner->GetTypeId() == TYPEID_UNIT)
                if (pSummoner->AI())
                    { pSummoner->AI()->SummonedMovementInform(pSummon, WAYPOINT_MOTION_TYPE, _currentNode); }
    }

    if (WPCheck)
        WPCheck = false;
    else
        _isArrivalDone = true;

    return true;
}

bool WaypointMovementGenerator<Creature>::tryToMove(Creature &creature)
{
    if (!_nextMoveTime.Passed())
        return true;

    bool pathFinding = _pathFinding;

    // for debug
    volatile int debug_creature_guid = creature.GetGUIDLow();
    volatile int debug_creature_entry = creature.GetEntry();

    // @!path_crash - disabled WP to stop crashes, ugly fix
    //if (creature.GetMapId() == 556 && pathFinding)
    //    pathFinding = false;

    if (creature.IsFormationLeader() && !creature.GetFormation()->AllUnitsReachedWaypoint())
    {
        _nextMoveTime.Reset(500);
        return true;
    }

    if(_isArrivalDone)
    {
        if (!_repeating)
        {
            // We have arrived at destination and we are NOT allowed to repeat whole path so it is time to disable movegen :P
            if (_currentNode == _path->size() - 1)
                return false;
        }
        _currentNode = (_currentNode + 1) % _path->size();
        // _currentNode %= _path->size();
    }
    // ++_currentNode;

    const WaypointData *node = _path->at(_currentNode);

    _isArrivalDone = false;

    creature.addUnitState(UNIT_STAT_ROAMING);

    //if (creature.GetMapId() == 556)
    //sLog.outLog(LOG_TMP, "WaypointMovementGenerator: Creature %s (guid %u) moved to XYZ %f %f %f, pathfinding %u, type %u, creature map %u, leader %u",
    //    creature.GetName(), creature.GetGUIDLow(), node->x, node->y, node->z, pathFinding, node->moveType, creature.GetMapId(), creature.IsFormationLeader());

    //Call for creature group update
    if (creature.IsFormationLeader())
    {
        bool turnAround = !creature.HasInArc(M_PI, node->x, node->y); // more than 180 degrees

        Movement::MoveSplineInit init(creature);

        init.MoveTo(node->x, node->y, node->z, pathFinding && node->moveType != M_FLY);

        if (node->moveType == M_FLY)
            init.SetFly();
        else
            init.SetWalk(node->moveType != M_RUN);

        init.Launch();

        creature.GetFormation()->LeaderMoveTo(node->x, node->y, node->z, turnAround); // gotta call that before leader moves so we calculate orientation diff
    }
    else
    {
        Movement::MoveSplineInit init(creature);
        init.MoveTo(node->x, node->y, node->z, pathFinding && node->moveType != M_FLY);

        if (node->moveType == M_FLY)
            init.SetFly();
        else
            init.SetWalk(node->moveType != M_RUN);

        init.Launch();
    }
    return true;
}

bool WaypointMovementGenerator<Creature>::Update(Creature &creature, const uint32 &diff)
{
    // way point movement can be switched on/off
    // This is quite handy for escort quests and other stuff
    if (creature.HasUnitState(UNIT_STAT_NOT_MOVE))
        return true;

    // prevent a crash at empty way point path.
    if (!_path || _path->empty())
        return false;

    if(Stopped())
    {
        if (CanMove(diff))
        {
            _nextMoveTime.Update(diff);
            return tryToMove(creature);
        }
    }
    else
    {
        if (!creature.HasUnitState(UNIT_STAT_MOVING))
        {
            _nextMoveTime.Update(diff);
            _nextMoveTime.Reset(30000);
        }
        else if (creature.IsStopped())
        {
            atNode(creature);

            _nextMoveTime.Update(diff);
            return tryToMove(creature);
        }
    }

    return true;
}

bool WaypointMovementGenerator<Creature>::GetResetPosition(Creature&, float& x, float& y, float& z)
{
    // prevent a crash at empty way point path.
    if (!_path || _path->empty())
        return false;

    const WaypointData* node = _path->at(_currentNode);
    x = node->x;
    y = node->y;
    z = node->z;
    return true;
}

//----------------------------------------------------//
uint32 FlightPathMovementGenerator::GetPathAtMapEnd() const
{
    if (_currentNode >= _path->size())
        return _path->size();

    uint32 curMapId = (*_path)[_currentNode].mapid;

    for(uint32 i = _currentNode; i < _path->size(); ++i)
    {
        if ((*_path)[i].mapid != curMapId)
            return i;
    }

    return _path->size();
}

void FlightPathMovementGenerator::_Initialize(Player &player)
{
    _Reset(player);
}

void FlightPathMovementGenerator::_Finalize(Player & player)
{
    if (player.m_taxi.empty())
    {
        // update z position to ground and orientation for landing point
        // this prevent cheating with landing  point at lags
        // when client side flight end early in comparison server side
        player.StopMoving();
    }
}

void FlightPathMovementGenerator::_Reset(Player & player)
{
    Movement::MoveSplineInit init(player);
    uint32 end = GetPathAtMapEnd();
    for (uint32 i = GetCurrentNode(); i != end; ++i)
    {
        G3D::Vector3 vertice((*_path)[i].x,(*_path)[i].y,(*_path)[i].z);
        init.Path().push_back(vertice);
    }
    init.SetFirstPointId(GetCurrentNode());
    init.SetFly();
    init.SetVelocity(32.0f);
    init.Launch();
}

void FlightPathMovementGenerator::_Interrupt(Player & player)
{
}

bool FlightPathMovementGenerator::Update(Player &player, const uint32 &diff)
{
    uint32 pointId = (uint32)player.movespline->currentPathIdx();


    /*char buf[64];
    sprintf(buf, "PointID: %u", pointId);
    std::string pointidstr = buf;
    player.Say(pointidstr.c_str(), LANG_UNIVERSAL);*/


    if (pointId > _currentNode)
    {
        bool departureEvent = true;
        do
        {
            DoEventIfAny(player,(*_path)[_currentNode],departureEvent);
            if (pointId == _currentNode)
                break;

            _currentNode += (uint32)departureEvent;
            departureEvent = !departureEvent;
        }
        while(true);
    }

    return _currentNode < _path->size()-1;
}

void FlightPathMovementGenerator::SetCurrentNodeAfterTeleport()
{
    if (_path->empty())
        return;

    uint32 map0 = (*_path)[0].mapid;

    for (size_t i = 1; i < _path->size(); ++i)
    {
        if ((*_path)[i].mapid != map0)
        {
            _currentNode = i;
            return;
        }
    }
}

void FlightPathMovementGenerator::DoEventIfAny(Player& player, TaxiPathNodeEntry const& node, bool departure)
{
    if (uint32 eventid = departure ? node.departureEventID : node.arrivalEventID)
    {
        //DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "Taxi %s event %u of node %u of path %u for player %s", departure ? "departure" : "arrival", eventid, node.index, node.path, player.GetName());

        if (!sScriptMgr.OnProcessEvent(eventid, &player, &player, departure))
            player.GetMap()->ScriptsStart(sEventScripts, eventid, &player, &player);
    }
}

bool FlightPathMovementGenerator::GetResetPosition(Player&, float& x, float& y, float& z)
{
    const TaxiPathNodeEntry& node = (*_path)[_currentNode];
    x = node.x;
    y = node.y;
    z = node.z;
    return true;
}
