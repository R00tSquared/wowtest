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

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Corpse.h"
#include "Player.h"
#include "MapManager.h"
#include "Transports.h"
#include "BattleGroundMgr.h"
#include "WaypointMovementGenerator.h"
#include "InstanceSaveMgr.h"
#include "ObjectMgr.h"
#include "Language.h"

void WorldSession::HandleMoveWorldportAckOpcode(WorldPacket & /*recv_data*/)
{
    sLog.outDebug("WORLD: got MSG_MOVE_WORLDPORT_ACK.");
    HandleMoveWorldportAckOpcode();
}

void WorldSession::HandleMoveWorldportAckOpcode()
{
    // get start teleport coordinates (will used later in fail case)
    WorldLocation old_loc;
    GetPlayer()->GetPosition(old_loc);

    // get the teleport destination
    WorldLocation &loc = GetPlayer()->GetTeleportDest();

    // possible errors in the coordinate validity check
    if (!MapManager::IsValidMapCoord(loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation))
    {
        // stop teleportation else we would try this again and again in LogoutPlayer...
        GetPlayer()->SetSemaphoreTeleport(false);
        // player don't gets saved - so his coords will stay at the point where
        // he was last saved
        LogoutPlayer(false);
        return;
    }

    // get the destination map entry, not the current one, this will fix homebind and reset greeting
    MapEntry const* mEntry = sMapStore.LookupEntry(loc.mapid);

    Map *map = NULL;

    // prevent crash at attempt landing to not existed battleground instance
    if(mEntry->IsBattleGroundOrArena())
    {
        if (GetPlayer()->GetBattleGroundId())
            map = sMapMgr.FindMap(loc.mapid, GetPlayer()->GetBattleGroundId());

        if (!map)
        {
            GetPlayer()->SetSemaphoreTeleport(false);

            // Teleport to previous place, if cannot be ported back TP to homebind place
            if (!GetPlayer()->TeleportTo(old_loc))
                GetPlayer()->TeleportToHomebind();

            return;
        }
    }

    InstanceTemplate const* mInstance = ObjectMgr::GetInstanceTemplate(loc.mapid);

    // reset instance validity, except if going to an instance inside an instance
    if (GetPlayer()->m_InstanceValid == false && !mInstance)
        GetPlayer()->m_InstanceValid = true;

    GetPlayer()->SetSemaphoreTeleport(false);

    GetPlayer()->Relocate(loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation);

    // relocate the player to the teleport destination
    if (!map)
        map = sMapMgr.CreateMap(loc.mapid, GetPlayer());

    GetPlayer()->SetMapId(loc.mapid);
    GetPlayer()->SetMap(map);

    // since the MapId is set before the GetInstance call, the InstanceId must be set to 0
    // to let GetInstance() determine the proper InstanceId based on the player's binds
    GetPlayer()->SetInstanceId(map->GetAnyInstanceId());

    // check this before Map::Add(player), because that will create the instance save!
    bool reset_notify = (GetPlayer()->GetBoundInstance(GetPlayer()->GetMapId(), GetPlayer()->GetDifficulty()) == NULL);

    GetPlayer()->SendInitialPacketsBeforeAddToMap();
    // the CanEnter checks are done in TeleporTo but conditions may change
    // while the player is in transit, for example the map may get full
    if (!GetPlayer()->GetMap()->Add(GetPlayer()))
    {
        // Teleport to previous place, if cannot be ported back TP to homebind place
        if (!GetPlayer()->TeleportTo(old_loc))
            GetPlayer()->TeleportToHomebind();

        return;
    }

    //this will set player's team ... so IT MUST BE CALLED BEFORE SendInitialPacketsAfterAddToMap()
    // battleground state prepare (in case join to BG), at relogin/tele player not invited
    // only add to bg group and object, if the player was invited (else he entered through command)
    if (_player->InBattleGroundOrArena())
    {
        // cleanup seting if outdated
        if (!mEntry->IsBattleGroundOrArena())
        {
            // Do next only if found in battleground
            _player->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);                          // We're not in BG.
            // reset destination bg team
            _player->SetBGTeam(TEAM_NONE);
        }
        // join to bg case
        else if (BattleGround *bg = _player->GetBattleGround())
        {
            if (_player->IsInvitedForBattleGroundInstance(_player->GetBattleGroundId()))
                bg->AddPlayer(_player);
            else if (_player->isGameMaster()) // add pvp minimap button
            {
                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, _player->GetTeam(), 0, STATUS_IN_PROGRESS, 0, bg->GetStartTime());
                SendPacket(&data);
            }
        }
    }

    GetPlayer()->SendInitialPacketsAfterAddToMap();

    // flight fast teleport case
    if (GetPlayer()->GetMotionMaster()->GetCurrentMovementGeneratorType()==FLIGHT_MOTION_TYPE)
    {
        if (!_player->InBattleGroundOrArena())
        {
            // short preparations to continue flight
            FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(GetPlayer()->GetMotionMaster()->top());
            flight->Reset(*GetPlayer());
            return;
        }

        // battleground state prepare, stop flight
        GetPlayer()->GetUnitStateMgr().InitDefaults(true);
        GetPlayer()->CleanupAfterTaxiFlight();
    }

    // resurrect character at enter into instance where his corpse exist after add to map
    Corpse *corpse = GetPlayer()->GetCorpse();
    if (corpse && corpse->GetType() != CORPSE_BONES && corpse->GetMapId() == GetPlayer()->GetMapId())
    {
        if (mEntry->IsDungeon())
        {
            GetPlayer()->ResurrectPlayer(0.5f,false);
            GetPlayer()->SpawnCorpseBones();
        }
    }

    if (mEntry->IsRaid() && mInstance)
    {
        if (reset_notify)
        {
            uint32 timeleft = sInstanceSaveManager.GetResetTimefor (GetPlayer()->GetMapId(), GetPlayer()->GetDifficulty()) - time(NULL);
            GetPlayer()->SendInstanceResetWarning(GetPlayer()->GetMapId(), timeleft); // greeting at the entrance of the resort raid instance
        }
    }
    
    if(GetPlayer()->GetGroup())
        GetPlayer()->GetGroup()->SendPlayerUpdate(_player);

    // need to be in this place, because message should be after raid initial message
    if (mEntry->IsDungeon())
    {
        Map* map = GetPlayer()->GetMap();
        GetPlayer()->SendCustomRaidInfo(map->GetId(), map->IsHeroicRaid());

        if (!GetPlayer()->HasItemCount(ITEM_SUMMONING_STONE, 1))
            GetPlayer()->GiveItem(ITEM_SUMMONING_STONE, 1);
    }

    // mount allow check
    if (!mEntry->IsMountAllowed())
        _player->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

    // honorless target
    if (GetPlayer()->pvpInfo.inHostileArea)
        GetPlayer()->CastSpell(GetPlayer(), 2479, true);
    else if (GetPlayer()->IsPvP() && !GetPlayer()->HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_IN_PVP))
        GetPlayer()->UpdatePvP(false);

    // resummon pet
    if (GetPlayer()->m_temporaryUnsummonedPetNumber)
    {
        Pet* NewPet = new Pet;
        if (!NewPet->LoadPetFromDB(GetPlayer(), 0, GetPlayer()->m_temporaryUnsummonedPetNumber, true))
            delete NewPet;

        GetPlayer()->m_temporaryUnsummonedPetNumber = 0;
    }

    _player->ClearTeleportOptions();

	// prevent to hack-teleport to Isle of Quel'Danas before it allowed
	if (GetPlayer()->GetZoneId() == 4080 && GetPlayer()->GetSession()->GetPermissions() == PERM_PLAYER && sWorld.getSeason() < SEASON_5)
	{
		GetPlayer()->TeleportToHomebind();
		return;
	}

    //// teleport back after respec
    //if (!GetPlayer()->isGameMaster() && GetPlayer()->GetMapId() == 13)
    //    GetPlayer()->TeleportTo(GetPlayer()->_recallPosition);
}

void WorldSession::HandleMoveTeleportAck(WorldPacket& recv_data)
{
    sLog.outDebug("MSG_MOVE_TELEPORT_ACK");
        /*uint64 guid;
    uint32 flags, time;

    recv_data >> guid;
    recv_data >> flags >> time;
    debug_log("Guid " UI64FMTD, guid);
    debug_log("Flags %u, time %u", flags, time/IN_MILLISECONDS);

    Unit *mover = _player->m_mover;
    Player *plMover = mover->GetTypeId() == TYPEID_PLAYER ? mover->ToPlayer() : NULL;

    if (!plMover || !plMover->IsBeingTeleportedNear())
        return;

    if (guid != plMover->GetGUID())
        return;

        //reset falltimer at teleport
    plMover->m_anti_justteleported = true;

    plMover->SetSemaphoreTeleportNear(false);

    uint32 old_zone = plMover->GetZoneId();

    WorldLocation const& dest = plMover->GetTeleportDest();

    plMover->SetPosition(dest, true);

    uint32 newzone = plMover->GetZoneId();

    plMover->UpdateZone(newzone);

    // new zone
    if (old_zone != newzone)
    {
        // honorless target
        if (plMover->pvpInfo.inHostileArea)
            plMover->CastSpell(plMover, 2479, true);
    }

    // resummon pet
    if (plMover->m_temporaryUnsummonedPetNumber)
    {
        Pet* NewPet = new Pet(plMover);
        if (!NewPet->LoadPetFromDB(plMover, 0, plMover->m_temporaryUnsummonedPetNumber, true))
            delete NewPet;

        plMover->m_temporaryUnsummonedPetNumber = 0;
    }

    //lets process all delayed operations on successful teleport
    plMover->ProcessDelayedOperations();*/ // Was in oregon, wasn't in HellGround
}

void WorldSession::HandleMovementOpcodes(WorldPacket & recv_data)
{
    Unit *mover = _player->GetMover();

	// crashfix
	if (!mover)
	{
		sLog.outLog(LOG_CRITICAL, "CRASH? mover is null for player %s (guid %u)", _player->GetName(), _player->GetGUIDLow());
		sLog.outLog(LOG_CRASH, "CRASH? mover is null for player %s (guid %u)", _player->GetName(), _player->GetGUIDLow());
		ASSERT(false);
		return;
	}

    Player *plMover = mover->ToPlayer();

    Opcodes opcode = recv_data.GetOpcode();
    // ignore, waiting processing in WorldSession::HandleMoveWorldportAckOpcode and WorldSession::HandleMoveTeleportAck
    if (plMover && plMover->IsBeingTeleported())
    {
        plMover->m_anti_justteleported_timer = 1000;
        recv_data.rpos(recv_data.wpos());                   // prevent warnings spam
        return;
    }

    MovementInfo movementInfo;
    recv_data >> movementInfo;
    /*----------------*/
    if (recv_data.size() != recv_data.rpos())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: MovementHandler: player %s (guid %d, account %u) sent a packet (opcode %u) that is %llu bytes larger than it should be. Kicked as cheater.", _player->GetName(), _player->GetGUIDLow(), _player->GetSession()->GetAccountId(), recv_data.GetOpcode(), recv_data.size() - recv_data.rpos());
        KickPlayer();
        return;
    }

    if (!Hellground::IsValidMapCoord(movementInfo.GetPos()->x, movementInfo.GetPos()->y, movementInfo.GetPos()->z, movementInfo.GetPos()->o))
        return;

    // fall damage generation (ignore in flight case that can be triggered also at lags in moment teleportation to another map).
    if (opcode == MSG_MOVE_FALL_LAND && plMover && !plMover->IsTaxiFlying())
        plMover->HandleFallDamage(movementInfo);

    // we dont want to switch to walk when MC other player
    if (!_player->IsSelfMover() && plMover)
        movementInfo.RemoveMovementFlag(MOVEFLAG_WALK_MODE);

    bool justStarted = !(bool(mover->GetUnitMovementFlags()));

    if (plMover)
    {
        bool updateOrientationOnly = false;
        bool skipAnticheat = false;
        bool useFallingFlag = false;
        bool forcemovement = false;
        bool check_passed = true;
        bool skip_passed = false;
        bool rooted = false;
        std::string m_anti_lastcheat;
        Opcodes opcode = recv_data.GetOpcode();
        uint32 TimeBetweenMove = movementInfo.time - plMover->m_anti_lastmovetime;
        bool Teleported = false;

        float Diff[3] = { fabs(plMover->GetPositionX() - movementInfo.GetPos()->x), fabs(plMover->GetPositionY() - movementInfo.GetPos()->y), fabs(plMover->GetPositionZ() - movementInfo.GetPos()->z) };

        if (movementInfo.HasMovementFlag(MOVEFLAG_ROOT) && (opcode == MSG_MOVE_STOP || opcode == MSG_MOVE_STOP_STRAFE || opcode == MSG_MOVE_STOP_PITCH || MSG_MOVE_STOP_SWIM))
        {
            rooted = true;
            movementInfo.moveFlags = 0;
        }

        if ((!movementInfo.HasMovementFlag(MOVEFLAG_MOVING) && !plMover->HasUnitMovementFlag(MOVEFLAG_MOVING) && 
            (opcode != MSG_MOVE_FALL_LAND || /*if opcode == MSG_MOVE_FALL_LAND -> allow to fall and not update position for 0.02f*/ 
            (Diff[0] == 0 && Diff[1] == 0 && (plMover->GetPositionZ() - movementInfo.GetPos()->z) < 0.02f )) &&
            !rooted)
            || plMover->GetDummyAura(55153)) // warden freeze - disable all moving
        {
            skipAnticheat = true;
            updateOrientationOnly = true;
        }

        if (opcode == MSG_MOVE_FALL_LAND && !plMover->IsTaxiFlying())
        {
            plMover->m_anti_ontaxipath = false;
            plMover->m_anti_isknockedback = false;
        }

        // Fix KnockBack
        if (plMover->m_anti_isknockedback)
            forcemovement = true;

        /*----------------------*/

        if (plMover->HasUnitState(UNIT_STAT_CASTING_NOT_MOVE) && /* means that the current cast has interrupt flags by moving + this means that player DOES CASTS some spell*/ 
            !plMover->HasUnitMovementFlag(MOVEFLAG_MOVING) && movementInfo.HasMovementFlag(MOVEFLAG_MOVING)) // You can't cast spell that need you to be standing if u are moving without stopping. Check only when you are moving at the moment
                plMover->InterruptSpellsOnMove();

        if (plMover->HasAura(54839) && plMover->GetLevel() == 71)
        {
            char chr[255];
            sprintf(chr, "OldX: %f, OldY: %f, OldZ: %f", plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ());
            plMover->Say(chr, LANG_UNIVERSAL);
            sprintf(chr, "NewX: %f, NewY: %f, NewZ: %f", movementInfo.GetPos()->x, movementInfo.GetPos()->y, movementInfo.GetPos()->z);
            plMover->Say(chr, LANG_UNIVERSAL);
            sprintf(chr, "Opcode == %s, NewMovementflags %u, OldMovementFlags %u", LookupOpcodeName(opcode), movementInfo.GetMovementFlags(), plMover->GetUnitMovementFlags()); // For testing
            plMover->Say(chr, LANG_UNIVERSAL);
            sprintf(chr, "movementInfo.time = %u", movementInfo.time);
            plMover->Say(chr, LANG_UNIVERSAL);
        }

        if ((!plMover->HasUnitMovementFlag(MOVEFLAG_MOVING) &&
        ((opcode == MSG_MOVE_JUMP && movementInfo.GetMovementFlags() == MOVEFLAG_FALLING) // movementInfo.GetMovementFlags() == MOVEFLAG_FALLING - Why is it used - i don't remember 
        || opcode == MSG_MOVE_START_FORWARD || opcode == MSG_MOVE_START_BACKWARD || opcode == MSG_MOVE_START_STRAFE_LEFT || opcode == MSG_MOVE_START_STRAFE_RIGHT
                || opcode == MSG_MOVE_START_PITCH_UP || opcode == MSG_MOVE_START_PITCH_DOWN || opcode == MSG_MOVE_START_SWIM) || updateOrientationOnly)
                && plMover->m_anti_transportGUID == 0 && plMover->m_anti_justteleported_timer == 0)
        {
            if (Diff[0] > 0.89f || Diff[1] > 0.89f || Diff[2] > 0.89f) // else of dungeon check only blinks
            {
                float BiggestFloat = Diff[0] > Diff[1] && Diff[0] > Diff[2] ? Diff[0] : Diff[2] > Diff[1] ? Diff[2] : Diff[1];
                uint8 DistanceNudge = BiggestFloat / 0.89f + 1;
                uint8 DistanceBlink = BiggestFloat + 1;
                if (DistanceBlink < 71) // ~70 yds - if more - tele hack will detect it
                {
                    for (uint8 DiffIndex = 0; DiffIndex < 3; DiffIndex++)
                    {
                        if (Diff[DiffIndex] > 0.89f)
                        {
                            m_anti_lastcheat = "Passiv Movement Hack (Blink Hack).";
                            for (uint8 i = 1; i < DistanceBlink; i++)
                            {
                                if (fabs(Diff[DiffIndex] - 1.0f*i) < 0.00001f * i)
                                {
                                    skip_passed = true;
                                    m_anti_lastcheat += "Blink";
                                    break;
                                }
                            }
                            if (!skip_passed)
                                for (uint8 i = 1; i < DistanceNudge; i++)
                                {
                                    if (fabs(Diff[DiffIndex] - 0.9f*i) < 0.0002f * i)
                                    {
                                        skip_passed = true;
                                        m_anti_lastcheat += "Nudge";
                                        break;
                                    }
                                }
                        }
                    }
                    if (skip_passed)
                        check_passed = false;
                }
            }
            else if (Diff[0] > 0.05f || Diff[1] > 0.05f || Diff[2] > 0.05f)
                sLog.outLog(LOG_WARDEN, "Movement anticheat: %s produces a anticheat alarm for LOW TELEPORT (blink analog - if there's someone so smart). DiffX: %.10f, DiffY: %.10f, DiffZ: %.10f", plMover->GetName(), Diff[0], Diff[1], Diff[2]);  
        }

        float real_delta = Diff[0] * Diff[0] + Diff[1] * Diff[1];
        if (plMover->m_anti_transportGUID == 0 && !plMover->m_anti_ontaxipath && !skipAnticheat)
        {
            UnitMoveType move_type;

            if (movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                move_type = movementInfo.HasMovementFlag(MOVEFLAG_BACKWARD) ? MOVE_FLIGHT_BACK : MOVE_FLIGHT;
            else if (movementInfo.HasMovementFlag(MOVEFLAG_SWIMMING))
                move_type = movementInfo.HasMovementFlag(MOVEFLAG_BACKWARD) ? MOVE_SWIM_BACK : MOVE_SWIM;
            else if (movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE))
                move_type = MOVE_WALK;
            else
                move_type = movementInfo.HasMovementFlag(MOVEFLAG_BACKWARD) ? MOVE_RUN_BACK : MOVE_RUN;

            float allowed_delta = 0;
            float current_speed = plMover->GetSpeed(move_type);

            if (move_type == MOVE_FLIGHT)
                if (plMover->GetSpeed(MOVE_RUN) > current_speed)
                    current_speed = plMover->GetSpeed(MOVE_RUN);

            float time_delta = TimeBetweenMove;

            if (time_delta > 0)
                plMover->m_anti_lastmovetime = movementInfo.time;
            else
                time_delta = 0;

            time_delta = (time_delta < 3000) ? time_delta / 1000 : 3.0f; // normalize time - 3.0 second allowed for heavy loaded server
            if (rooted)
                time_delta = (time_delta < 1.0f) ? time_delta : 1.0f; // maximum "running in stun" is one second

            if (current_speed < plMover->m_anti_last_hspeed)
            {
                allowed_delta = plMover->m_anti_last_hspeed;
                if (plMover->m_anti_lastspeed_changetime == 0)
                    plMover->m_anti_lastspeed_changetime = movementInfo.time + (uint32)floor(((plMover->m_anti_last_hspeed / current_speed) * 1000)) + 100; //100ms above for random fluctuating =)))
            }
            else
                allowed_delta = current_speed;

            allowed_delta = allowed_delta * time_delta;
            allowed_delta = allowed_delta * allowed_delta + 2;

            if (real_delta > 2500.0f || (real_delta > allowed_delta) && (Diff[2] < 1))
                sLog.outLog(LOG_WARDEN, "BEFORE allowed delta %f, real delta %f, justteleported timer %u, justteleported distance %f ", allowed_delta, real_delta, plMover->m_anti_justteleported_timer, plMover->m_anti_justteleported_distance);

            if (plMover->m_anti_justteleported_distance != 0 && plMover->m_anti_justteleported_timer != 0)
            {
                uint32 justTeleportedYds = plMover->m_anti_justteleported_distance * 1.2f;
                if (allowed_delta < justTeleportedYds * justTeleportedYds)
                    allowed_delta = justTeleportedYds * justTeleportedYds;
            }
            else
                plMover->m_anti_justteleported_distance = 0;

            if (real_delta > 2500.0f || (real_delta > allowed_delta) && (Diff[2] < 1))
                sLog.outLog(LOG_WARDEN, "AFTER allowed delta %f, real delta %f, justteleported timer %u, justteleported distance %f ", allowed_delta, real_delta, plMover->m_anti_justteleported_timer, plMover->m_anti_justteleported_distance);

            // Anti-Speedhack
            if ((real_delta > allowed_delta) && (Diff[2] < 1))
            {
                m_anti_lastcheat = "Speed Hack";
                check_passed = false;
                if (real_delta/17 > allowed_delta) // 414% speed max
                    skip_passed = true;
            }
            if (plMover->m_anti_justteleported_timer != 0)
            {
                Teleported = true;
                if (plMover->m_anti_justteleported_timer < 250)
                    plMover->m_anti_justteleported_timer = 0;
                else
                    plMover->m_anti_justteleported_timer -= (time_delta * 1000) >= 250 ? 250 : time_delta * 1000;
            }
            // Anti-Teleport
            // Disabled to revert: if ((real_delta > allowed_delta) && (real_delta > (time_delta * 100)))
            if (real_delta > 4900.0f && real_delta > allowed_delta)
            {
                sLog.outLog(LOG_WARDEN, "Movement anticheat: %s produces a anticheat alarm for teleport hack. real_delta %f, allowed_delta %f, justteleported timer %u, justteleported distance %f, map %u, old xyz: %f %f %f ,new xyz: %f %f %f", plMover->GetName(), real_delta,
                    allowed_delta, plMover->m_anti_justteleported_timer, plMover->m_anti_justteleported_distance, plMover->GetMapId(), plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), movementInfo.GetPos()->x, movementInfo.GetPos()->y, movementInfo.GetPos()->z);  
                m_anti_lastcheat = "Teleport Hack";
                check_passed = false;
                skip_passed = true;
            }

            if (current_speed > plMover->m_anti_last_hspeed || plMover->m_anti_last_hspeed != current_speed && movementInfo.time > plMover->m_anti_lastspeed_changetime)
            {
                plMover->m_anti_last_hspeed = current_speed; // store current speed
                plMover->m_anti_lastspeed_changetime = 0;
            }

            // Anti-Flyhack
            if (((movementInfo.GetMovementFlags() & (MOVEFLAG_CAN_FLY | MOVEFLAG_FLYING | SPLINEFLAG_FLYINGING)) != 0) && !(plMover->HasAuraType(SPELL_AURA_FLY) || plMover->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED)) && !(plMover->IsSwimming() && movementInfo.GetPos()->z < plMover->GetPositionZ())) // fixed descending down in water.)
            {
                m_anti_lastcheat = "Fly Hack";
                useFallingFlag = true;
                check_passed = false;
            }

            // Anti-Featherfall
            if (movementInfo.HasMovementFlag(MOVEFLAG_SAFE_FALL) && !(plMover->HasAuraType(SPELL_AURA_FEATHER_FALL)))
            {
                m_anti_lastcheat = "Feather Fall Hack";
                check_passed = false;
            }

            if (uint32 timediff = WorldTimer::getMSTime())
            {
                int64 catchUpTime = int64(movementInfo.time) - int64(timediff);
                // example
                // cycle one
                //    -7000             3000          -        10000
                //     7000             10000         -        3000

                // cycle two (after we cheat engine enabled and added x6 speed)
                //    -3000(4000 added)         8000(added 5000)           -      11000 (1 sec passed)
                //     11000(4000 added)        15000(added 5000)          -        4000 (1 sec passed)

                // when increasing client time catchUpTime goes up 

                // Anti Cheat Engine speedhack - detects speed process after player is moved by synchronyzing the client and server ms
                if (_player->m_anti_ping_time > catchUpTime || _player->m_anti_ping_time == 0) // before that you could firstly slow WoW and then speed hack. Now when slowing down ping_time will be real server time.
                    _player->m_anti_ping_time = catchUpTime;
                else if (_player->m_anti_ping_time + 3000 < catchUpTime)    // giving players a 3000 ms lag(acceleration, not lag) opportunity
                {
                    sLog.outLog(LOG_WARDEN, "Movement anticheat: cheat engine for player %s caught: m_anti_ping_time = %li, movementInfo.time = %u, timediff = %u", _player->GetName(), _player->m_anti_ping_time, movementInfo.time, timediff);
                    check_passed = false;
                    skip_passed = true;
                    m_anti_lastcheat = "Cheat Engine speedhack";
                    _player->m_anti_ping_time = catchUpTime;    // renew the timer
                }
            }
            
            if (((movementInfo.GetMovementFlags() & (MOVEFLAG_CAN_FLY | MOVEFLAG_FLYING | SPLINEFLAG_FLYINGING)) == 0) && !(plMover->HasAuraType(SPELL_AURA_FLY) || plMover->HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED)) && !plMover->IsSwimming() && !plMover->m_anti_justteleported_distance && 
                !(opcode == MSG_MOVE_START_FORWARD || opcode == MSG_MOVE_START_BACKWARD || opcode == MSG_MOVE_START_STRAFE_LEFT || opcode == MSG_MOVE_START_STRAFE_RIGHT)
                /*if we just start movement - there will be no diff but we can still cheat, so do not increase and do not reset counter. On msg stop there is some difference so it's ok*/)
            {
                bool SameX = !Diff[0];
                bool SameY = !Diff[1];
                bool SameZ = !Diff[2];
                // Anti air-ctrl brakes
                if (SameZ && (!SameX || !SameY))
                {
                    // anti waterwalk hack
                    if (movementInfo.HasMovementFlag(MOVEFLAG_WATERWALKING) && !(plMover->HasAuraType(SPELL_AURA_WATER_WALK) || plMover->HasAuraType(SPELL_AURA_GHOST)))
                    {
                        m_anti_lastcheat = "Water Walk Hack";
                        check_passed = false;
                    }
                    else if (_player->m_anti_airbrakes_alarmcount > 5)
                    {
                        float destx = movementInfo.GetPos()->x;
                        float desty = movementInfo.GetPos()->y;
                        float z = movementInfo.GetPos()->z;

                        float ground = plMover->GetTerrain()->GetHeight(destx, desty, MAX_HEIGHT, true);
                        float floor = plMover->GetTerrain()->GetHeight(destx, desty, z, true);
                        float destz = fabs(ground - z) <= fabs(floor - z) ? ground : floor;
                        if (plMover->HasAuraType(SPELL_AURA_WATER_WALK) || plMover->HasAuraType(SPELL_AURA_GHOST))
                        {
                            float water = plMover->GetTerrain()->GetWaterLevel(destx, desty, z);
                            if(water > destz)
                                destz = water;
                        }
                            
                        if (fabs(z - destz) > COMMON_ALLOW_HEIGHT_DIFF)
                        {
                            check_passed = false;
                            m_anti_lastcheat = "Air-Ctrl Brakes Hack";
                        }
                        else
                            _player->m_anti_airbrakes_alarmcount = 0;
                    }
                    else 
                        _player->m_anti_airbrakes_alarmcount++;
                }
                else
                    _player->m_anti_airbrakes_alarmcount = 0;
            }

            // adminpanel - jump frequency
            // wowmaelstrom - jump height
            // if opcode == jump --- start assessing opcodes by Z coordinate until we lose FALLING flag
            // falling flag is present during whole maelstrom air jump height and during all continuing jumps of adminpanel hack
            if (movementInfo.HasMovementFlag(MOVEFLAG_FALLING) && !plMover->m_anti_justteleported_timer)
            {
                if (opcode == MSG_MOVE_JUMP && plMover->m_anti_jump_start_z == 0)
                    plMover->m_anti_jump_start_z = movementInfo.GetPos()->z; // when we have just started jumping (and until we lose the fall-flag)

                if (plMover->m_anti_jump_start_z && (movementInfo.GetPos()->z - plMover->m_anti_jump_start_z > COMMON_ALLOW_HEIGHT_DIFF))
                    // no fabs(), only count higher than norm
                    // recheck existence of plMover->m_anti_jump_start_z cause we don't want to spam with WardenChecks
                {
                    check_passed = false;
                    skip_passed = true;
                    m_anti_lastcheat = "Air-Jump Hack";
                    // jumping and using some sort of "push up" movement produces same changes (example: spell 37896), 
                    // but "forcemovement" variable checks on that
                    plMover->m_anti_jump_start_z = 0;
                }
            }
            else if (plMover->m_anti_jump_start_z && (plMover->HasUnitMovementFlag(MOVEFLAG_FALLING) || plMover->m_anti_justteleported_timer)) // player doesn't have falling flag anymore -> clear the JUMP_Z variable
                plMover->m_anti_jump_start_z = 0;
        }
        else if (movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
        {
            // antiwarp
            if (plMover->m_transport)
            {
                float trans_rad = movementInfo.GetTransportPos()->x * movementInfo.GetTransportPos()->x + movementInfo.GetTransportPos()->y * movementInfo.GetTransportPos()->y + movementInfo.GetTransportPos()->z * movementInfo.GetTransportPos()->z;
                if (opcode != CMSG_MOVE_CHNG_TRANSPORT && trans_rad > 3600.0f)
                {
                    check_passed = false;
                    m_anti_lastcheat = "In Transport warp.";
                }
            }
            else
            {
                if (GameObjectData const* go_data = sObjectMgr.GetGOData(plMover->m_anti_transportGUID))
                {
                    int mapid = go_data->mapid;
                    if (plMover->GetMapId() != mapid)
                    {
                        m_anti_lastcheat = "Transport wrong mapid";
                        check_passed = false;
                    }
                    else if (mapid != 369 && plMover->GetMapId() != 554)
                    {
                        float delta_gox = go_data->posX - movementInfo.GetPos()->x;
                        float delta_goy = go_data->posY - movementInfo.GetPos()->y;
                        float delta_go = delta_gox*delta_gox + delta_goy*delta_goy;
                        if (delta_go > 3600.0f)
                        {
                            check_passed = false;
                            m_anti_lastcheat = "Out Transport warp.";
                        }
                    }
                }
                else if (plMover->GetMapId() != 554)
                {
                    check_passed = false;
                    m_anti_lastcheat = "Transport doesn't exist as GameObject";
                }
            }

            if (!check_passed)
            {
                if (plMover->m_transport)
                {
                    plMover->m_transport->RemovePassenger(plMover);
                    plMover->m_transport = NULL;
                }
                movementInfo.ClearTransportData();
                plMover->m_anti_transportGUID = 0;
            }
        }

        if (!check_passed)
        {
            if (_player->GetDummyAura(54839))
                _player->Say(m_anti_lastcheat.c_str(), LANG_UNIVERSAL);

            if (_player->m_anti_alarmcount_skip_timer >= 250)
                _player->m_anti_alarmcount_skip_timer -= TimeBetweenMove >= 250 ? 250 : TimeBetweenMove;
            else
                _player->m_anti_alarmcount_skip_timer = 0;

            if (_player->m_anti_alarmcount_skip_timer > 0 && !skip_passed)
                check_passed = true;    

            if (_player->GetDummyAura(54839))
            {
                if (check_passed)
                    _player->Say("check is passed", LANG_UNIVERSAL);
                if (forcemovement)
                    _player->Say("forcemovement", LANG_UNIVERSAL);
                _player->Say(LookupOpcodeName(opcode), LANG_UNIVERSAL);
                uint64 moveInfo = movementInfo.GetMovementFlags();
                char buf[64];
                sprintf(buf, "%u", moveInfo);
                std::string moveInfoString = buf;
                _player->Say(moveInfoString.c_str(), LANG_UNIVERSAL);
            }
        }

        //Save movement flags
        if (!rooted)
            plMover->SetUnitMovementFlags(movementInfo.GetMovementFlags());

        if (check_passed || plMover->isGameMaster() || forcemovement)
        {
            // player model itself is 1.5 yds
            // on speed 1 there is 3.5 yds per 0.5 sec (heartbeat)
            // lowest trap radius (players) is 2.5 - therefor distance is 5 + 3 yds (player model radius*2)
            // so we're checking from speed 228% lowest - all lower speeds should activate trap just fine themself
            if (!plMover->IsSpectator() && !Teleported)
            {
                real_delta = sqrt(real_delta + (Diff[2] * Diff[2]));
                if (real_delta > 8.0f && real_delta < 35.0f) // using real_delta as a parameter here, not counting speeds faster than 1000%
                {
                    Position start, end;
                    mover->GetPosition(start);
                    end = movementInfo.pos;
                    plMover->ActivateFoundTrapsIfNeeded(start, end, real_delta);
                }
            }
            /* process position-change */
            HandleMoverRelocation(movementInfo, updateOrientationOnly, justStarted);

            if (plMover)
                plMover->UpdateFallInformationIfNeed(movementInfo, opcode);

            WorldPacket data(rooted ? MSG_MOVE_FALL_LAND : opcode, recv_data.size());
            data << mover->GetPackGUID();                 // write guid
            movementInfo.Write(data);                     // write data
            mover->BroadcastPacketExcept(&data, _player);
        
            if (_player->m_anti_alarmcount > 0 && !updateOrientationOnly && !(opcode == MSG_MOVE_START_FORWARD || opcode == MSG_MOVE_START_BACKWARD || opcode == MSG_MOVE_START_STRAFE_LEFT || opcode == MSG_MOVE_START_STRAFE_RIGHT))
                // Opcodes - right to check START movement, cause start movement has no changes in coordinates. STOP opcode HAS difference between client and server - so anticheat is checking something there.
                _player->m_anti_alarmcount -= 1;
        }
        else
        {
            if (!rooted)
                _player->m_anti_alarmcount += 10; // 50 in config. if got 5 times - bye bye then.
            else
                sLog.outLog(LOG_WARDEN, "Movement anticheat: %s produces a anticheat alarm for %s AND is rooted.", _player->GetName(), m_anti_lastcheat.c_str());
            if (_player->m_anti_alarmcount > 0 && _player->GetBattleGround())
                sLog.outLog(LOG_WARDEN, "Movement anticheat: %s produces a anticheat alarm for %s AND is in battleground.", _player->GetName(), m_anti_lastcheat.c_str());
            WorldPacket data;
            //plMover->SetUnitMovementFlags(MOVEFLAG_NONE);
            plMover->BuildTeleportAckMsg(data, plMover->GetPositionX(), plMover->GetPositionY(), plMover->GetPositionZ(), plMover->GetOrientation());
            plMover->SendPacketToSelf(&data);
            plMover->SendHeartBeat();
            if (_player->m_anti_alarmcount > 0)
            {
                if (_player->m_anti_alarmcount > 50)
                {
                    sLog.outLog(LOG_WARDEN, "Movement anticheat: %s kicked: %d anticheat alarms!", _player->GetName(), _player->m_anti_alarmcount);
                    KickPlayer(); // kick player who SENDS packets, not the one who moves
                    return; // after kickplayer there's no session, can't continue;
                }
                else
                    sLog.outLog(LOG_WARDEN, "Movement anticheat: %s produces a anticheat alarm for %s.", _player->GetName(), m_anti_lastcheat.c_str());
            }

            // Here we send additional Warden Checks if needed
            if (m_anti_lastcheat == "Air-Jump Hack")
            {
                // 1003 and 1004 checks in Warden are emuhack(adminpanel) airjump and maelstrom airjump
                WardenRequestAdditionalMemCheck(1003);
                WardenRequestAdditionalMemCheck(1004);
                WardenForceRequest();
            }
        }
    }
    else
    {
        /* process position-change */
        HandleMoverRelocation(movementInfo, false, justStarted);
        
        WorldPacket data(opcode, recv_data.size());
        data << mover->GetPackGUID();                 // write guid
        movementInfo.Write(data);                     // write data
        mover->BroadcastPacketExcept(&data, _player);
    }
}

void WorldSession::HandleMoverRelocation(MovementInfo& movementInfo, bool updateOrientationOnly, bool JustStarted)
{
    if (JustStarted && m_clientTimeDelayReset)
    {
        m_clientTimeDelayReset = false;
        uint32 oldTime = _player->GetMover()->m_movementInfo.time + m_clientTimeDelay;
        uint32 newDelay = WorldTimer::getMSTime() - movementInfo.time;
        if ((movementInfo.time + newDelay) > oldTime) // only change if new time is above old time
            m_clientTimeDelay = newDelay;
    }
    
    movementInfo.UpdateTime( movementInfo.time + m_clientTimeDelay );

    Unit *mover = _player->GetMover();

    if (Player *plMover = mover->ToPlayer())
    {
        if (movementInfo.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
        {
            if (!plMover->GetTransport())
            {
                // elevators also cause the client to send MOVEFLAG_ONTRANSPORT - just unmount if the guid can be found in the transport list
                for (MapManager::TransportSet::const_iterator iter = sMapMgr.m_Transports.begin(); iter != sMapMgr.m_Transports.end(); ++iter)
                {
                    if ((*iter)->GetObjectGuid() == movementInfo.GetTransportGuid())
                    {
                        plMover->m_transport = (*iter);
                        (*iter)->AddPassenger(plMover);
                        break;
                    }
                }
            }
            if (GameObject *obj = plMover->GetMap()->GetGameObject(movementInfo.t_guid))
                plMover->m_anti_transportGUID = obj->GetDBTableGUIDLow();
            else
                plMover->m_anti_transportGUID = GUID_LOPART(movementInfo.t_guid);
        }
        else if (plMover->GetTransport())               // if we were on a transport, leave
        {
            plMover->GetTransport()->RemovePassenger(plMover);
            plMover->SetTransport(NULL);
            movementInfo.ClearTransportData();
        }
        else if (plMover->m_anti_transportGUID != 0)
            plMover->m_anti_transportGUID = 0;

        if (movementInfo.HasMovementFlag(MOVEFLAG_SWIMMING) != plMover->IsSwimming())
        {
            // now client not include swimming flag in case jumping under water
            plMover->SetSwimming(!plMover->IsSwimming() || plMover->GetTerrain()->IsInWater(movementInfo.GetPos()->x, movementInfo.GetPos()->y, movementInfo.GetPos()->z));
        }
    }

    mover->m_movementInfo = movementInfo;
    if (updateOrientationOnly)
        mover->SetPosition(mover->GetPositionX(), mover->GetPositionY(), mover->GetPositionZ(), movementInfo.GetPos()->o);
    else
        mover->SetPosition(movementInfo.GetPos()->x, movementInfo.GetPos()->y, movementInfo.GetPos()->z, movementInfo.GetPos()->o);

    if (mover->GetObjectGuid().IsPlayer())
        mover->ToPlayer()->HandleFallUnderMap(movementInfo.GetPos()->z);
}

void WorldSession::HandleForceSpeedChangeAck(WorldPacket &recv_data)
{
    /* extract packet */
    ObjectGuid guid;
    MovementInfo movementInfo;
    float newspeed;

    recv_data >> guid;
    recv_data >> Unused<uint32>();                          // counter or moveEvent
    recv_data >> movementInfo;
    recv_data >> newspeed;

    // now can skip not our packet
    if (_player->GetGUID() != guid.GetRawValue())
        return;

    // continue parse packet
    /*----------------*/

    // skip not personal message;
    GetPlayer()->m_movementInfo = movementInfo;
    if (movementInfo.j_xyspeed > GetPlayer()->m_anti_last_hspeed)
        GetPlayer()->m_anti_last_hspeed = movementInfo.j_xyspeed;
    GetPlayer()->m_anti_lastspeed_changetime = movementInfo.time + 1750;
    if (GetPlayer()->m_anti_alarmcount_skip_timer < 2000)
        GetPlayer()->m_anti_alarmcount_skip_timer += 1000;

    // client ACK send one packet for mounted/run case and need skip all except last from its
    // in other cases anti-cheat check can be fail in false case
    UnitMoveType move_type;
    UnitMoveType force_move_type;

    static char const* move_type_name[MAX_MOVE_TYPE] = {  "Walk", "Run", "RunBack", "Swim", "SwimBack", "TurnRate", "Flight", "FlightBack" };

    uint16 opcode = recv_data.GetOpcode();
    switch (opcode)
    {
        case CMSG_FORCE_WALK_SPEED_CHANGE_ACK:          move_type = MOVE_WALK;          force_move_type = MOVE_WALK;        break;
        case CMSG_FORCE_RUN_SPEED_CHANGE_ACK:           move_type = MOVE_RUN;           force_move_type = MOVE_RUN;         break;
        case CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK:      move_type = MOVE_RUN_BACK;      force_move_type = MOVE_RUN_BACK;    break;
        case CMSG_FORCE_SWIM_SPEED_CHANGE_ACK:          move_type = MOVE_SWIM;          force_move_type = MOVE_SWIM;        break;
        case CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK:     move_type = MOVE_SWIM_BACK;     force_move_type = MOVE_SWIM_BACK;   break;
        case CMSG_FORCE_TURN_RATE_CHANGE_ACK:           move_type = MOVE_TURN_RATE;     force_move_type = MOVE_TURN_RATE;   break;
        case CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK:        move_type = MOVE_FLIGHT;        force_move_type = MOVE_FLIGHT;      break;
        case CMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE_ACK:   move_type = MOVE_FLIGHT_BACK;   force_move_type = MOVE_FLIGHT_BACK; break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: WorldSession::HandleForceSpeedChangeAck: Unknown move type opcode: %u", opcode);
            return;
    }

    // skip all forced speed changes except last and unexpected
    // in run/mounted case used one ACK and it must be skipped.m_forced_speed_changes[MOVE_RUN} store both.
    if (_player->m_forced_speed_changes[force_move_type] > 0)
    {
        --_player->m_forced_speed_changes[force_move_type];
        if (_player->m_forced_speed_changes[force_move_type] > 0)
            return;
    }

    if (!_player->GetTransport() && fabs(_player->GetSpeed(move_type) - newspeed) > 0.01f)
    {
        if (_player->GetSpeed(move_type) > newspeed)         // must be greater - just correct
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: %sSpeedChange player %s is NOT correct (must be %f instead %f), force set to correct value",
                move_type_name[move_type], _player->GetName(), _player->GetSpeed(move_type), newspeed);
            _player->SetSpeed(move_type,_player->GetSpeedRate(move_type),true);
        }
        else                                                // must be lesser - cheating
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Player %s from account id %u kicked for incorrect speed (must be %f instead %f)",
                _player->GetName(),_player->GetSession()->GetAccountId(),_player->GetSpeed(move_type), newspeed);
            _player->GetSession()->KickPlayer();
        }
    }
}

void WorldSession::HandleSetActiveMoverOpcode(WorldPacket &recv_data)
{
    sLog.outDebug("WORLD: Recvd CMSG_SET_ACTIVE_MOVER");

    recv_data >> Unused<uint64>();                          // guid
}

void WorldSession::HandleMoveNotActiveMoverOpcode(WorldPacket &recv_data)
{
    sLog.outDebug("WORLD: Recvd CMSG_MOVE_NOT_ACTIVE_MOVER");
    recv_data.hexlike();

    uint64 old_mover_guid;
    MovementInfo mi;

    old_mover_guid = recv_data.readPackGUID();
    recv_data >> mi;

    if (!old_mover_guid)
        return;

    _player->m_movementInfo = mi;
}

void WorldSession::HandleMountSpecialAnimOpcode(WorldPacket& /*recvdata*/)
{
    //sLog.outDebug("WORLD: Recvd CMSG_MOUNTSPECIAL_ANIM");

    WorldPacket data(SMSG_MOUNTSPECIAL_ANIM, 8);
    data << uint64(GetPlayer()->GetGUID());

    GetPlayer()->BroadcastPacket(&data, false);
}

void WorldSession::HandleMoveKnockBackAck(WorldPacket & recv_data)
{
    sLog.outDebug("CMSG_MOVE_KNOCK_BACK_ACK");
    // Currently not used but maybe use later for recheck final player position
    // (must be at call same as into "recv_data >> x >> y >> z >> orientation;"

    MovementInfo movementInfo;
    uint64 guid;

    recv_data >> guid;                                      // guid
    recv_data >> Unused<uint32>();                          // unk
    recv_data >> movementInfo;

    if (GetPlayer()->GetGUID() != guid)
        return;

    // Save movement flags
    _player->SetUnitMovementFlags(movementInfo.GetMovementFlags());

    GetPlayer()->m_anti_isknockedback = true;

    // skip not personal message;
    GetPlayer()->m_movementInfo = movementInfo;
    if (movementInfo.j_xyspeed > GetPlayer()->m_anti_last_hspeed)
        GetPlayer()->m_anti_last_hspeed = movementInfo.j_xyspeed;
    GetPlayer()->m_anti_lastspeed_changetime = movementInfo.time + 1750;
    if (GetPlayer()->m_anti_alarmcount_skip_timer < 2000)
        GetPlayer()->m_anti_alarmcount_skip_timer += 1000;
}

void WorldSession::HandleMoveHoverAck(WorldPacket& recv_data)
{
    sLog.outDebug("CMSG_MOVE_HOVER_ACK");

    MovementInfo movementInfo;

    recv_data >> Unused<uint64>();                          // guid
    recv_data >> Unused<uint32>();                          // unk
    recv_data >> movementInfo;
    recv_data >> Unused<uint32>();                          // unk2
}

void WorldSession::HandleMoveWaterWalkAck(WorldPacket& recv_data)
{
    sLog.outDebug("CMSG_MOVE_WATER_WALK_ACK");

    MovementInfo movementInfo;                         // unk2

    recv_data >> Unused<uint64>();                          // guid
    recv_data >> Unused<uint32>();                          // unk
    recv_data >> movementInfo;
    recv_data >> Unused<uint32>();                          // unk2
}

void WorldSession::HandleSummonResponseOpcode(WorldPacket& recv_data)
{
    if (!_player->isAlive() || _player->IsInCombat())
        return;

    uint64 summoner_guid;
    bool agree;
    recv_data >> summoner_guid;
    recv_data >> agree;

    _player->SummonIfPossible(agree, summoner_guid);
}
