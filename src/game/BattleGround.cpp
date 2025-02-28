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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "Creature.h"
#include "MapManager.h"
#include "Language.h"
#include "Chat.h"
#include "SpellAuras.h"
#include "ArenaTeam.h"
#include "World.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "Object.h"
#include "WorldPacket.h"
#include "Util.h"
#include "BattleGroundNA.h"
#include "BattleGroundBE.h"
#include "BattleGroundRL.h"

#include "BattleGroundAV.h"
#include "BattleGroundWS.h"
#include "BattleGroundAB.h"
#include "BattleGroundEY.h"
#include <GameEvent.h>

BattleGround::BattleGround()
{
    m_TypeID            = BattleGroundTypeId(0);
    m_BracketId         = MAX_BATTLEGROUND_BRACKETS;        // use as mark bg template
    m_InstanceID        = 0;
    m_Status            = STATUS_NONE;
    m_EndTime           = 0;
    m_LastResurrectTime = 0;
    m_Queue_type        = MAX_BATTLEGROUND_QUEUES;
    m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;
    m_ArenaType         = 0;
    m_IsArena           = false;
    m_Winner            = 2;
    m_StartTime         = 0;
    m_Events            = 0;
    m_IsRated           = false;
	m_RatedBG		    = false;
    m_BuffChange        = false;
    m_Name              = "";
    m_LevelMin          = 0;
    m_LevelMax          = 0;
    m_InBGFreeSlotQueue = false;
    m_SetDeleteThis     = false;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers        = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers        = 0;

    m_MapId             = 0;
    m_Map               = NULL;

    m_TeamStartLocX[TEAM_ALLIANCE]   = 0;
    m_TeamStartLocX[TEAM_HORDE]      = 0;

    m_TeamStartLocY[TEAM_ALLIANCE]   = 0;
    m_TeamStartLocY[TEAM_HORDE]      = 0;

    m_TeamStartLocZ[TEAM_ALLIANCE]   = 0;
    m_TeamStartLocZ[TEAM_HORDE]      = 0;

    m_TeamStartLocO[TEAM_ALLIANCE]   = 0;
    m_TeamStartLocO[TEAM_HORDE]      = 0;

    m_ArenaTeamIds[TEAM_ALLIANCE]   = 0;
    m_ArenaTeamIds[TEAM_HORDE]      = 0;

    m_ArenaTeamRatingChanges[TEAM_ALLIANCE]   = 0;
    m_ArenaTeamRatingChanges[TEAM_HORDE]      = 0;

    m_BgRaids[TEAM_ALLIANCE]         = NULL;
    m_BgRaids[TEAM_HORDE]            = NULL;

    m_PlayersCount[TEAM_ALLIANCE]    = 0;
    m_PlayersCount[TEAM_HORDE]       = 0;

    m_guidsReady[TEAM_ALLIANCE].clear();
    m_guidsReady[TEAM_HORDE].clear();

    m_PrematureCountDown = false;
    m_PrematureCountDown = 0;
    m_TimeElapsedSinceBeggining = 0;
    m_ShouldEndTime = 0;
    m_forceClose = false;

    m_HonorMode = BG_NORMAL;

    m_progressStart = 0;

    m_ValidStartPositionTimer = 0;

    for (uint32 key = 1; key <= 2048; key *= 2) 
    {
        teamClassMaskCount[TEAM_ALLIANCE][key] = 0;
        teamClassMaskCount[TEAM_HORDE][key] = 0;
    }

    m_playerMaxKillCount[TEAM_ALLIANCE] = 0;
    m_playerMaxKillCount[TEAM_HORDE] = 0;

    m_doubleLoginCheckTimer.Reset(15000);
    m_alteracAnnounced = false;
}

BattleGround::~BattleGround()
{
    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)
    int size = m_BgCreatures.size();
    for (int i = 0; i < size; ++i)
    {
        DelCreature(i);
    }
    size = m_BgObjects.size();
    for (int i = 0; i < size; ++i)
    {
        DelObject(i);
    }

    if (GetBgInstanceId())
    {
        // delete creature and go respawn times
        RealmDataDatabase.PExecute("DELETE FROM creature_respawn WHERE instance = '%u'",GetBgInstanceId());
        RealmDataDatabase.PExecute("DELETE FROM gameobject_respawn WHERE instance = '%u'",GetBgInstanceId());
        // delete instance from db
        RealmDataDatabase.PExecute("DELETE FROM instance WHERE id = '%u'",GetBgInstanceId());
    }
    // remove from battlegrounds
    sBattleGroundMgr.RemoveBattleGround(GetBgInstanceId(), GetTypeID());
    // unload map
    Map * map = GetMap();
    if (map && map->IsBattleGroundOrArena())
    {
        ((BattleGroundMap*)map)->SetBattleGround(NULL);
        ((BattleGroundMap*)map)->SetUnload();
    }

    // remove from bg free slot queue
    this->RemoveFromBGFreeSlotQueue();

    for (BattleGroundScoreMap::iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); itr++)
    {
        delete itr->second;
    }

    // Cleanup temp arena teams for solo 3v3
    if (isArena() && isRated() && GetArenaType() == ARENA_TYPE_3v3)
    {
        ArenaTeam *tempAlliArenaTeam = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
        ArenaTeam *tempHordeArenaTeam = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));

        if (tempAlliArenaTeam && tempAlliArenaTeam->GetId() >= 0xFFF00000)
        {
            sObjectMgr.RemoveArenaTeam(tempAlliArenaTeam->GetId());
            delete tempAlliArenaTeam;
        }

        if (tempHordeArenaTeam && tempHordeArenaTeam->GetId() >= 0xFFF00000)
        {
            sObjectMgr.RemoveArenaTeam(tempHordeArenaTeam->GetId());
            delete tempHordeArenaTeam;
        }
    }
}

void BattleGround::Update(uint32 diff)
{
    if (!GetPlayersSize() && !GetRemovedPlayersSize() && !GetReviveQueueSize())
        //BG is empty
        return;

    if (GetStatus() == STATUS_IN_PROGRESS)
        m_InProgressDuration += diff;

    m_StartTime += diff;
    m_TimeElapsedSinceBeggining += diff;
    m_ValidStartPositionTimer += diff;

    bool doubleLoginCheck = false;
    m_doubleLoginCheckTimer.Update(diff);
    if (m_doubleLoginCheckTimer.Passed())
    {
        doubleLoginCheck = true;
        m_doubleLoginCheckTimer.Reset(15000);
    }

    if (GetRemovedPlayersSize())
    {
        bool punish = GetStatus() == STATUS_IN_PROGRESS || GetStatus() == STATUS_WAIT_JOIN;
        
        for (std::map<uint64, uint8>::iterator itr = m_RemovedPlayers.begin(); itr != m_RemovedPlayers.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

            if (plr)
            {
                RemovePlayerAtLeave(itr->first, true, true, punish);
                if (itr->second == 2)
                    ChatHandler(plr).SendSysMessage(16754);
            }
            else
                RemovePlayerAtLeave(itr->first, false, false, punish);
        }
        m_RemovedPlayers.clear();
    }

	if (GetPlayersSize() && isBattleGround())
	{
		// remove offline players from bg after 1 minute
		for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
		{
			Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
			itr->second.LastOnlineTime += diff;

			if (GetStatus() == STATUS_IN_PROGRESS)
				itr->second.PlayedTime += diff;

            // PlayedTime was 10 min
			if (sWorld.getConfig(CONFIG_RATED_BG_ENABLED) && m_RatedBG && itr->second.PlayedTime > 3 * MINUTE * MILLISECONDS)
			{
				if (!itr->second.Rated)
					itr->second.Rated = true;

				if (plr && !plr->HasAura(SPELL_BG_RATED))
				{
					ChatHandler(plr).SendSysMessage(15655);
					plr->AddAura(SPELL_BG_RATED, plr);
				}
			}

            // remove double login players
            if (doubleLoginCheck && plr && Player::IsDoubleLogin(itr->second.haship) && !plr->isGameMaster() && !sBattleGroundMgr.isTesting())
            {
                m_RemovedPlayers[itr->first] = 2;
            }
            else
            {
                // remove AFK players
                if (plr)
                {
                    itr->second.LastOnlineTime = 0;                 // update last online time 

                    if (!plr->IsPvP())                       // force all players PvP on
                        plr->UpdatePvP(true, true);
                }
                else
                {
                    if (itr->second.LastOnlineTime >= MAX_OFFLINE_TIME)
                        m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
                }
            }
		}
	}

    m_LastResurrectTime += diff;
    if (m_LastResurrectTime >= RESURRECTION_INTERVAL)
    {
        if (GetReviveQueueSize())
        {
            for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
            {
                Creature *sh = NULL;
                for (std::vector<uint64>::iterator itr2 = (itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
                {
                    Player *plr = sObjectMgr.GetPlayerInWorld(*itr2);
                    if (!plr)
                        continue;

                    if (!sh && plr->IsInWorld())
                    {
                        sh = plr->GetMap()->GetCreature(itr->first);
                        // only for visual effect
                        if (sh)
                            sh->CastSpell(sh, SPELL_SPIRIT_HEAL, true);   // Spirit Heal, effect 117
                    }

                    plr->CastSpell(plr, SPELL_RESURRECTION_VISUAL, true);   // Resurrection visual

                    RestorePet(plr);

                    m_ResurrectQueue.push_back(*itr2);
                }
                (itr->second).clear();
            }

            m_ReviveQueue.clear();
            m_LastResurrectTime = 0;
        }
        else
            // queue is clear and time passed, just update last resurrection time
            m_LastResurrectTime = 0;
    }
    else if (m_LastResurrectTime > 500)    // Resurrect players only half a second later, to see spirit heal effect on NPC
    {
        for (std::vector<uint64>::iterator itr = m_ResurrectQueue.begin(); itr != m_ResurrectQueue.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayerInWorld(*itr);
            if (!plr)
                continue;
            plr->ResurrectPlayer(1.0f);

            RestorePet(plr);
            plr->CastSpell(plr, SPELL_SPIRIT_HEAL_MANA, true);
            plr->CastSpell(plr, SPELL_SPIRIT_HEAL_PET_WARLOCK, true);
            sObjectAccessor.ConvertCorpseForPlayer(*itr);
        }
        m_ResurrectQueue.clear();
    }

    if (sWorld.getConfig(CONFIG_ALTERAC_ENABLED) && isBattleGround() && !m_alteracAnnounced && (GetStatus() == STATUS_IN_PROGRESS || GetStatus() == STATUS_WAIT_JOIN))
    {
        time_t t = time(NULL);
        tm* aTm = localtime(&t);
        if (aTm->tm_hour == sWorld.getConfig(CONFIG_ALTERAC_EVENING_START_HOUR) - 1 && aTm->tm_min == 55)
        {
            SendNotifyToTeam(HORDE, 16753);
            SendNotifyToTeam(ALLIANCE, 16753);
            m_alteracAnnounced = true;
        }
    }

    // end arena and BGs (but give marks) on restart
    if (GetStatus() == STATUS_IN_PROGRESS && sWorld.IsShutdowning() && sWorld.GetShutdownTimer() < 15 && !m_forceClose)
    {
        // give marks for BGs
        //if (isBattleGround())
        //{
        //    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        //    {
        //        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
        //        if (!plr)
        //            continue;

        //        BGReward(plr, sWorld.getConfig(RATE_BG_WIN));
        //    }
        //}

		m_RatedBG = false;
		SetRated(false);
        EndBattleGround(TEAM_NONE);      
        m_forceClose = true;
        return;
    }

    // if less then minimum players are in on one side, then start premature finish timer
    if (GetStatus() == STATUS_IN_PROGRESS && !isArena() && sBattleGroundMgr.GetPrematureFinishTime() && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()) && !sBattleGroundMgr.isTesting())
    {
        if (!m_PrematureCountDown)
        {
            m_PrematureCountDown = true;
            m_PrematureCountDownTimer = sBattleGroundMgr.GetPrematureFinishTime();
            if( sBattleGroundMgr.IsPrematureFinishTimerEnabled())
                SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH, m_PrematureCountDownTimer / 60000);
            else
                SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
        }
        else if (m_PrematureCountDownTimer <= diff)
        {
            // time's up!
            EndBattleGround(TEAM_NONE); // noone wins
            m_PrematureCountDown = false;
        }
        else
        {
            uint32 newtime = m_PrematureCountDownTimer - diff;
            // announce every minute
            if (m_PrematureCountDownTimer != sBattleGroundMgr.GetPrematureFinishTime() && newtime / 60000 != m_PrematureCountDownTimer / 60000)
                if( sBattleGroundMgr.IsPrematureFinishTimerEnabled())
                    SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH, m_PrematureCountDownTimer / 60000);
                else
                    SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
            m_PrematureCountDownTimer = newtime;
        }
    }
    else if (m_PrematureCountDown)
        m_PrematureCountDown = false;

    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        if (isArena() && sWorld.timerMessage(5) && GetStartDelayTime() > sWorld.getConfig(CONFIG_ARENA_READY_START_TIMER))
        {
            for (std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                if (Player* plr = sObjectMgr.GetPlayerInWorld(itr->first))
                {
                    uint8 team = plr->GetBGTeam() == ALLIANCE ? 0 : 1;
                    if (m_guidsReady[team].find(plr->GetGUID()) == m_guidsReady[team].end())
                        ChatHandler(plr).SendSysMessage(15575);
                }
            }
        }

        if (sBattleGroundMgr.isTesting() && isBattleGround())
        {
            SetStartDelayTime(1);
        }
        
        //WSG/AB/EY/ALTERAC/NAGRAND/RUINS/BLADE_EDGE
        uint8 index;
        switch (GetMapId())
        {
            case 489: // WSG
                index = 0;
                break;
            case 529: // AB
                index = 1;
                break;
            case 566: // EOTS
                index = 2;
                break;
            case 30: // AV
                index = 3;
                break;
            case 559: //Nagrand
                index = 4;
                break;
            case 572: // lordaeron
                index = 5;
                break;
            case 562: // blade_edge
                index = 6;
                break;
            default:
                index = 10; // unidentified
                break;
        }
        if (index != 10)
        {
            // Find if the player left our start zone; if so, teleport it back
            if (m_ValidStartPositionTimer > 1000)
            {
                m_ValidStartPositionTimer = 0;
                            
                for (std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
                {
                    if (Player* plr = sObjectMgr.GetPlayerInWorld(itr->first))
                    {
                        if (plr->isGameMaster())
                            continue;
                        
                        uint8 team = plr->GetBGTeam() == ALLIANCE ? 0 : 1;

                        if (plr->GetDistance(insidePos[team][index][0], insidePos[team][index][1], insidePos[team][index][2]) > 
                            plr->GetDistance(outsidePos[team][index][0], outsidePos[team][index][1], outsidePos[team][index][2]))
                            plr->TeleportTo(GetMapId(), m_TeamStartLocX[team], m_TeamStartLocY[team], m_TeamStartLocZ[team], m_TeamStartLocO[team]);
                    }
                }
            }
        }
    }

    if (GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        m_EndTime += diff;
        if (m_EndTime >= TIME_TO_AUTOREMOVE)                 // 2 minutes
        {
            for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
            }
            // do not change any battleground's private variables
        }
    }

    if (isArena() && GetStatus() == STATUS_IN_PROGRESS)
    {
        if (GetMapId() == 559)
        {
            if(BG_NA_DOOR_DESPAWN_TIMER < (time(NULL) - m_progressStart))
            {
                for (uint32 i = BG_NA_OBJECT_DOOR_1; i <= BG_NA_OBJECT_DOOR_2; i++)
                    DelObject(i, false);
            }
            for (uint32 j = BG_NA_OBJECT_T_CR_1; j <= BG_NA_OBJECT_T_CR_2; j++)
                DelObject(j, false);
        }
        else if (GetMapId() == 562)
        {
            for (uint32 i = BG_BE_OBJECT_T_CR_1; i <= BG_BE_OBJECT_T_CR_2; i++)
                DelObject(i, false);
        }
        else if (GetMapId() == 572)
        {
            for (uint32 i = BG_RL_OBJECT_T_CR_1; i <= BG_RL_OBJECT_T_CR_2; i++)
                DelObject(i, false);
        }
    }

    if (isArena() && sBattleGroundMgr.GetArenaEndAfterTime())
    {
        if (m_TimeElapsedSinceBeggining > sBattleGroundMgr.GetArenaEndAfterTime() && GetStatus() == STATUS_IN_PROGRESS)
        {
            if (GetAlivePlayersCountByTeam(HORDE) > GetAlivePlayersCountByTeam(ALLIANCE))
                EndBattleGround(HORDE);
            else if (GetAlivePlayersCountByTeam(HORDE) < GetAlivePlayersCountByTeam(ALLIANCE))
                EndBattleGround(ALLIANCE);
            else // impossible to define a true winner
                EndBattleGround(TEAM_NONE); 
   
            return;
        }
        if(m_TimeElapsedSinceBeggining > sBattleGroundMgr.GetArenaEndAfterTime()/2 &&
            m_TimeElapsedSinceBeggining / (!isRated() ? 60000 : 120000) > (m_TimeElapsedSinceBeggining - diff) / (!isRated() ? 60000 : 120000)) //warning every 2 mins, or 1 min for 1v1
            SendMessageToAll(LANG_ARENA_WILL_END,
            (sBattleGroundMgr.GetArenaEndAfterTime() - m_TimeElapsedSinceBeggining + diff) / 60000);
        // BONE AURA +5% DAMAGE on 5th minute for 1v1
        // 1v1:
            /*300 - 1
            330 2
            360 3
            390 4
            420 5
            450 6
            480 7
            510 8
            540 9 
            570 10
            600 arena end*/
        //if((GetArenaType() == ARENA_TYPE_1v1/* || !isRated()*/) && m_TimeElapsedSinceBeggining > sBattleGroundMgr.GetArenaEndAfterTime()/8/*5 min*/ &&
        //    m_TimeElapsedSinceBeggining/30000 > (m_TimeElapsedSinceBeggining - diff)/30000) // aura every 30 sec
        //{
        //    if (SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(55117))
        //    {
        //        if (GetPlayersSize())
        //        {
        //            for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        //            {
        //                Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

        //                if (plr && plr->isAlive() && !plr->IsSpectator())
        //                {
        //                    Aura *Aur = CreateAura(spellInfo, 0, NULL, plr);
        //                    plr->AddAura(Aur);
        //                    plr->SendSpellVisual(6756);
        //                }
        //            }
        //        }
        //    }
        //}
    }
    if (isArena() && m_ShouldEndTime) // if exists means should end
    {
        if (m_ShouldEndTime <= diff)
        {
            m_ShouldEndTime = 0;
            if(GetAlivePlayersCountByTeam(HORDE) && !GetAlivePlayersCountByTeam(ALLIANCE))
            {
                EndBattleGround(HORDE); // horde wins
                return;
            }
            else if (!GetAlivePlayersCountByTeam(HORDE) && GetAlivePlayersCountByTeam(ALLIANCE))
            {
                EndBattleGround(ALLIANCE); // alliance wins
                return;
            }
            EndBattleGround(TEAM_NONE); // both teams are dead
            return;
        }
        else
            m_ShouldEndTime -= diff;
    }
}

void BattleGround::RestorePet(Player* plr)
{
    if ((plr->GetClass() != CLASS_HUNTER && plr->GetClass() != CLASS_WARLOCK) || !plr->isAlive())
        return;

    Pet* ThePet;
    if (plr->GetClass() == CLASS_HUNTER)
    {
        ThePet = new Pet();
        if (!ThePet->LoadPetFromDB(plr,0,0,false))
            return;

        if (ThePet->isDead())
        {
            ThePet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
            ThePet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            ThePet->setDeathState(ALIVE);
            ThePet->ClearUnitState(UNIT_STAT_ALL_STATE);
        }

        ThePet->SetHealth(ThePet->GetMaxHealth());
        ThePet->SetPower(POWER_HAPPINESS, ThePet->GetMaxPower(POWER_HAPPINESS));
        ThePet->RemoveAllAurasButPermanent();
        ThePet->m_CreatureSpellCooldowns.clear();
        ThePet->m_CreatureCategoryCooldowns.clear();
        plr->DelayedPetSpellInitialize();
    }
    else if (plr->GetLastPetNumber()) // we're sure it is warlock
    {
        ThePet = new Pet();
        if (!ThePet->LoadPetFromDB(plr, 0, plr->GetLastPetNumber(), true))
            delete ThePet;
        else
        {
            // nerf doomguard/infernal
            if (ThePet->GetEntry() == 11859 || ThePet->GetEntry() == 89)
                ThePet->SetEntry(416);

            ThePet->SetHealth(ThePet->GetMaxHealth());
            ThePet->SetPower(POWER_MANA, ThePet->GetMaxPower(POWER_MANA));
        }
    }
}

bool BattleGround::Solo3v3ArenaFinishIfNotFull()
{
    if (GetArenaType() != ARENA_TYPE_3v3 || GetStatus() != STATUS_IN_PROGRESS)
        return false;

    bool someoneNotInArena = false;

    ArenaTeam* team[2];
    team[0] = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
    team[1] = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));

    ASSERT(team[0] && team[1]);

    uint32 spell_id = SPELL_ARENA_DESERTER;
    uint32 duration = DESERTER_ARENA_DURATION_AFK;

    for (int i = 0; i < 2; i++)
    {
        if (!team[i])
            return true;
        
        for (ArenaTeam::MemberList::iterator itr = team[i]->membersBegin(); itr != team[i]->membersEnd(); itr++)
        {
            Player* plr = sObjectAccessor.GetPlayerOnline(itr->guid);

            if (!plr)
            {              
                // if not joined - add deserter
                // if joined and did logout - add deserter and take rating (//take rating on logout in 3v3 arena)
                RealmDataDatabase.PExecute("DELETE FROM character_aura WHERE guid='%u' and spell='%u'", GUID_LOPART(itr->guid), spell_id);
                RealmDataDatabase.PExecute("INSERT INTO character_aura (guid, caster_guid, item_guid, spell, maxduration, remaintime, remaincharges) VALUES ('%u', '%u', '0', '%u', '%u', '%u', '-1')", GUID_LOPART(itr->guid), GUID_LOPART(itr->guid), spell_id, duration, duration);

                getObjectMgr()->ModifyArenaRating(itr->guid, -10, ARENA_TEAM_3v3);
                someoneNotInArena = true;
            } else if (plr && (plr->GetAnyInstanceId() != GetBgInstanceId())) {
                plr->AddAura(spell_id, plr, duration);
                ChatHandler(plr).PSendSysMessage(LANG_RATING_MODIFIED);

                getObjectMgr()->ModifyArenaRating(itr->guid, -10, ARENA_TEAM_3v3);
                someoneNotInArena = true;
            }
        }
    }

    if (someoneNotInArena)
        return true;

    // loop again (can't do this in first loop)      
    for (int i = 0; i < 2; i++)
    {
        //uint32 my_team_rating = team[i]->GetRating();         
        //uint32 enemy_team_rating = team[OTHER_TEAM(i)]->GetRating();
        //int32 change_on_win = team[0]->GetChanceAgainstMod(my_team_rating, enemy_team_rating, 1);
        //int32 change_on_lose = team[0]->GetChanceAgainstMod(my_team_rating, enemy_team_rating, 0);

        for (ArenaTeam::MemberList::iterator itr = team[i]->membersBegin(); itr != team[i]->membersEnd(); itr++)
        {
            Player* plr = sObjectAccessor.GetPlayerOnline(itr->guid);
            if (plr)
            {
                // send messages
                ChatHandler(plr).PSendSysMessage(LANG_SQ_PARTICIPANT_INFO);
                //ChatHandler(plr).PSendSysMessage(LANG_SQ_GAMES_NEEDED);

                // Arena points are awarded immediately after match. For a lose, 75% less points are awarded.
                //ChatHandler(plr).SendSysMessage(15578);

                // clear solo 3v3 teammates info
                plr->Solo3v3LastTeammates.clear();

                // add info
                for (ArenaTeam::MemberList::iterator itr1 = team[i]->membersBegin(); itr1 != team[i]->membersEnd(); itr1++)
                {
                    if (itr == itr1)
                        continue;

                    plr->Solo3v3LastTeammates.push_back(std::make_pair(0, (*itr1)));
                }
            }

        }
    }

    return false;
}

Map* BattleGround::GetMap()
{
    return m_Map ? m_Map : m_Map = sMapMgr.FindMap(GetMapId(),GetBgInstanceId());
}

void BattleGround::SetTeamStartLoc(PlayerTeam TeamID, float X, float Y, float Z, float O)
{
    TeamId idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
}

void BattleGround::SendPacketToAll(WorldPacket *packet)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
        if (plr)
        {
            if (GetTypeID() == BATTLEGROUND_AV)
            {
                OpcodeHandler& opHandle = opcodeTable[packet->GetOpcode()];
                //sLog.outLog(LOG_DEFAULT, "SendPacketToAll - %s sent to %s", opHandle.name, plr->GetName());
            }

            plr->SendPacketToSelf(packet);
        }
        else
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
    }
}

void BattleGround::SendPacketToTeam(PlayerTeam TeamID, WorldPacket *packet, Player *sender, bool self)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
            continue;
        }

        if (!self && sender == plr)
            continue;

        PlayerTeam team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetBGTeam();

        if (team == TeamID)
            plr->SendPacketToSelf(packet);
    }
}

void BattleGround::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void BattleGround::PlaySoundToTeam(uint32 SoundID, PlayerTeam TeamID)
{
    WorldPacket data;

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
            continue;
        }

        PlayerTeam team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetBGTeam();

        if (team == TeamID)
        {
            sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
            plr->SendPacketToSelf(&data);
        }
    }
}

void BattleGround::CastSpellOnTeam(uint32 SpellID, PlayerTeam TeamID)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
            continue;
        }

        PlayerTeam team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team)
            team = plr->GetBGTeam();

        if (team == TeamID)
            plr->CastSpell(plr, SpellID, true);
    }
}

void BattleGround::YellToAll(Creature* creature, const char* text, uint32 language)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        WorldPacket data(SMSG_MESSAGECHAT, 200);
        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
        if (!plr)
        {
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
            continue;
        }
        creature->BuildMonsterChat(&data,CHAT_MSG_MONSTER_YELL,text,language,creature->GetName(),itr->first);
        plr->SendPacketToSelf(&data);
    }
}

void BattleGround::RewardHonorToTeam(uint32 Honor, PlayerTeam TeamID)
{
     if (!Honor)
        return;

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
            continue;
        }

        PlayerTeam team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team) team = plr->GetBGTeam();

        if (team == TeamID)
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, Honor);
    }
}

void BattleGround::RewardReputationToTeam(uint32 faction_id, uint32 Reputation, PlayerTeam TeamID)
{
    uint32 opposeFactionId = 0;
    switch (faction_id)
    {
    case 509: opposeFactionId = 510; break;
    case 730: opposeFactionId = 729; break;
    case 889: opposeFactionId = 890; break;
    case 510: opposeFactionId = 509; break;
    case 729: opposeFactionId = 730; break;
    case 890: opposeFactionId = 889; break;
    default: break;
    }

    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);
    FactionEntry const* opposeFactionEntry = sFactionStore.LookupEntry(opposeFactionId);

    if (!factionEntry || (opposeFactionId && !opposeFactionEntry))
        return;

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

        if (!plr)
        {
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
            continue;
        }

        PlayerTeam team = itr->second.Team;
        if (!team) team = plr->GetBGTeam();

        if (team == TeamID)
        {
            //sLog.outLog(LOG_DEFAULT, "BattleGround.cpp L721: %u, %u", plr->TeamForRace(plr->GetRace()), team);
            int32 rep = plr->CalculateReputationGain(REPUTATION_SOURCE_BG, Reputation, ((plr->TeamForRace(plr->GetRace()) == team || !opposeFactionId) ? faction_id : opposeFactionId));
            plr->GetReputationMgr().ModifyReputation(((plr->TeamForRace(plr->GetRace()) == team || !opposeFactionEntry) ? factionEntry : opposeFactionEntry), Reputation);
            plr->UpdateBgTitle();
        }
    }
}

void BattleGround::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void BattleGround::UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player *Source)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    Source->SendPacketToSelf(&data);
}

void BattleGround::EndBattleGround(PlayerTeam winner)
{
    this->RemoveFromBGFreeSlotQueue();
    PlayerTeam almost_winning_team = HORDE;
    ArenaTeam * winner_arena_team = NULL;
    ArenaTeam * loser_arena_team = NULL;
    uint32 loser_rating = 0;
    uint32 winner_rating = 0;
    WorldPacket data;
    Player *Source = NULL;
    const char *winmsg = "";

    if (winner == ALLIANCE)
    {
        if (isBattleGround())
            winmsg = GetHellgroundString(LANG_BG_A_WINS);
        else
            winmsg = GetHellgroundString(LANG_ARENA_GOLD_WINS);

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        SetWinner(WINNER_ALLIANCE);
    }
    else if (winner == HORDE)
    {
        if (isBattleGround())
            winmsg = GetHellgroundString(LANG_BG_H_WINS);
        else
            winmsg = GetHellgroundString(LANG_ARENA_GREEN_WINS);

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        SetWinner(WINNER_HORDE);
    }
    else
    {
        SetWinner(3);
    }

    if (!isArena())
        sLog.outLog(LOG_BG,"ID %u: Ended bgtype %u, winner %s, time_elapsed %s", GetBgInstanceId(), GetTypeID(), GetBGTeamName(winner), GetTimeString(GetTimeElapsed()/1000).c_str());

    SetStatus(STATUS_WAIT_LEAVE);
    m_EndTime = 0;

    // arena rating calculation
    if (isArena() && isRated() && !sWorld.IsStopped())
    {
        if (winner == ALLIANCE)
        {
            winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
            loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
        }
        else if (winner == HORDE)
        {
            winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(HORDE));
            loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(ALLIANCE));
        }
        if (winner_arena_team && loser_arena_team)
        {
            loser_rating = loser_arena_team->GetStats().rating;
            winner_rating = winner_arena_team->GetStats().rating;

            int32 winner_change = winner_arena_team->WonAgainst(loser_rating);
            int32 loser_change = loser_arena_team->LostAgainst(winner_rating);

            sLog.outDebug("--- Winner rating: %u, Loser rating: %u, Winner change: %u, Losser change: %u ---", winner_rating, loser_rating, winner_change, loser_change);

            if (winner == ALLIANCE)
            {
                SetArenaTeamRatingChangeForTeam(ALLIANCE, winner_change);
                SetArenaTeamRatingChangeForTeam(HORDE, loser_change);
            }
            else
            {
                SetArenaTeamRatingChangeForTeam(HORDE, winner_change);
                SetArenaTeamRatingChangeForTeam(ALLIANCE, loser_change);
            }

            sLog.outLog(LOG_ARENA, "Arena match %u Type: %u for Team1Id: %u - Team2Id: %u ended. WinnerTeamId: %u. Winner rating: %u, Loser rating: %u. RatingChange: %i.", GetTempArenaGuid(), m_ArenaType, m_ArenaTeamIds[TEAM_ALLIANCE], m_ArenaTeamIds[TEAM_HORDE], winner_arena_team->GetId(), winner_rating, loser_rating, winner_change);
            //if (sWorld.getConfig(CONFIG_ARENA_LOG_EXTENDED_INFO))
            for (std::map<uint64, BattleGroundScore*>::const_iterator itr = GetPlayerScoresBegin(); itr != GetPlayerScoresEnd(); ++itr)
            {
                if (Player* player = sObjectMgr.GetPlayerInWorld(itr->first)) 
                {
                    sLog.outLog(LOG_ARENA, "Statistics %u for %s (GUID: %llu, Team: %d, IP: %s): %u damage, %u healing, %u killing blows", GetTempArenaGuid(), player->GetName(), itr->first, player->GetArenaTeamId(m_ArenaType == 5 ? 2 : m_ArenaType == 3), player->GetSession()->GetRemoteAddress().c_str(), itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
                }
                else
                {
                    std::string lookName;
                    sObjectMgr.GetPlayerNameByGUID(itr->first, lookName);
                    sLog.outLog(LOG_ARENA, "Statistics %u for %s (GUID: %llu, Arena match: %u vs %u): %u damage, %u healing, %u killing blows (OFFLINE)", GetTempArenaGuid(), lookName.c_str(), itr->first, m_ArenaType, m_ArenaType, itr->second->DamageDone, itr->second->HealingDone, itr->second->KillingBlows);
                }
            }

            if (sWorld.getConfig(CONFIG_ARENA_EXPORT_RESULTS))
            {
                RealmDataDatabase.PExecute("INSERT INTO arena_fights VALUES (%u, '%01u', %u, %u, %u, %u, %i, NOW(), %u);",
                    GetBgInstanceId(), m_ArenaType, winner_arena_team->GetId(),loser_arena_team->GetId(), winner_rating, loser_rating, winner_change,uint32(m_TimeElapsedSinceBeggining/1000));
            }
        }
        else
        {
            SetArenaTeamRatingChangeForTeam(ALLIANCE, 0);
            SetArenaTeamRatingChangeForTeam(HORDE, 0);
        }
    }

    if (isArena() && sWorld.getConfig(CONFIG_ARENA_ANNOUNCE))
    {
        if (isRated())
        {
            if (m_ArenaType == ARENA_TYPE_3v3 || m_ArenaType == ARENA_TYPE_2v2)
            {
                uint32 string = m_ArenaType == ARENA_TYPE_2v2 ? LANG_ARENA_2V2_ANNOUNCE : LANG_ARENA_SOLO_3V3_ANNOUNCE;

                sWorld.SendWorldText(string, ACC_DISABLED_BGANN);

                if (m_ArenaType == ARENA_TYPE_3v3)
                    sLog.outLog(LOG_DISCORD, "#ARENA3 %s", sObjectMgr.GetHellgroundString(string, 1));
                else
                    sLog.outLog(LOG_DISCORD, "#ARENA2 %s", sObjectMgr.GetHellgroundString(string, 1));
            }
        }
        else
        {
            if (m_ArenaType == ARENA_TYPE_2v2)
                sWorld.SendWorldText(16756, ACC_DISABLED_BGANN);
        }
    }

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        PlayerTeam team = itr->second.Team;
		Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

		if (!team)
		{
			if (plr)
				team = plr->GetBGTeam();
			else
			{
				sLog.outLog(LOG_CRITICAL, "Player guid %u have no BG team!", GUID_LOPART(itr->first));
				continue;
			}
		}

        if (!plr)
        {
			if (isBattleGround())
			{
                getObjectMgr()->BGModStats(itr->first, team == winner, m_RatedBG&& itr->second.Rated);
				RemovePlayerAtLeave(itr->first, false, false, false);
			}

            continue;
        }

        // should remove spirit of redemption
        if (plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        if (isArena() && !plr->isGameMaster())
        {
            plr->SetVisibility(VISIBILITY_ON);
            plr->SetFlying(false);

            plr->GetCamera().ResetView(true);
        }

        if (!plr->isAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }
        else
        {
            HostileReference* ref = plr->getHostileRefManager().getFirst();
            while (ref)
            {
                if (Unit* pUnit = ref->getSource()->getOwner())
                {
                    if (!pUnit || !pUnit->GetObjectGuid().IsCreature())
                    {
                        ref = ref->next();
                        continue;
                    }

                    pUnit->SetNonAttackableFlag(UNIT_FLAG_PASSIVE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_ATTACKABLE_2, true);
                }
                ref = ref->next();
            }
            // needed cause else in av some creatures will kill the players at the end
            plr->CombatStop();
            plr->getHostileRefManager().deleteReferences();
        }

        // per player calculation
        if (isArena() && isRated() && winner_arena_team && loser_arena_team && !sWorld.IsStopped())
        {
            if (team == winner)
            {
                winner_arena_team->MemberWon(plr, loser_rating);
            }
            else
                loser_arena_team->MemberLost(plr, winner_rating);

            //// arena 2v2 daily quest condition
            //if (sWorld.isEasyRealm())
            //{
            //    if(GetArenaType() == ARENA_TYPE_2v2)
            //        plr->KilledMonster(695007, 0);
            //    else if (GetArenaType() == ARENA_TYPE_3v3)
            //        plr->KilledMonster(695008, 0);
            //}

            /*if (sWorld.getConfig(CONFIG_ARENA_EXPORT_RESULTS))
            {
                BattleGroundScore* score = m_PlayerScores[itr->first];
                RealmDataDatabase.PExecute("INSERT INTO arena_fights_detailed VALUES (%u, " UI64FMTD", %u, %u, %u, %u, %u, %i);",
                    GetInstanceID(), itr->first, (team == winner ? winner_arena_team->GetId() : loser_arena_team->GetId()),
                    score->DamageDone, score->HealingDone, score->KillingBlows,persRating,persDiff);
            }*/
        }

        if (team == winner)
        {
            if (!Source)
                Source = plr;

            // blizzlike quest
            RewardQuest(plr);

			if (isBattleGround())
			{
				BGReward(plr, true);
                getObjectMgr()->BGModStats(itr->first, true, m_RatedBG && itr->second.Rated);
			}
        }
        else if (winner != 0)
        {
			if (isBattleGround())
			{
				BGReward(plr, false);
                getObjectMgr()->BGModStats(itr->first, false, m_RatedBG && itr->second.Rated);
			}
        }
        else // winner == 0
        {
            if (isBattleGround())
            {
                if (sWorld.getConfig(CONFIG_PREMATURE_BG_REWARD))
                {
                    if (!almost_winning_team)
                        almost_winning_team = GetWinningTeam();
                    
                    if (almost_winning_team == team)                  // player's team had more points
                        BGReward(plr, true);
                    else
                        BGReward(plr, false);            // if scores were the same, each team gets 1 mark.
                }

                // there is always a winner!       
                //if (sWorld.getConfig(CONFIG_RATED_BG_ENABLED))
                //    ChatHandler(plr).PSendSysMessage(15648);
            }
        }

        plr->CombatStopWithPets(true);

        BlockMovement(plr);

        sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
        plr->SendPacketToSelf(&data);

        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, team, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime());
        plr->SendPacketToSelf(&data);
    }

    if (isArena() && isRated() && winner_arena_team && loser_arena_team)
    {
        // send updated arena team stats to players
        // this way all arena team members will get notified, not only the ones who participated in this match
        if (winner_arena_team->GetId() <= 0xFFF00000 && loser_arena_team->GetId() <= 0xFFF00000) {
            winner_arena_team->SaveToDB();
            loser_arena_team->SaveToDB();

            winner_arena_team->NotifyStatsChanged();
            loser_arena_team->NotifyStatsChanged();
        }

        //if (GetArenaType() == ARENA_TYPE_2v2)
        //    sWorld.ArenaTeamLosesRowAdd(winner_arena_team->GetId(), loser_arena_team->GetId());

        sLog.outDebug("Rated arena match between %s and %s finished, winner: %s", loser_arena_team->GetName().c_str(), winner_arena_team->GetName().c_str(), winner_arena_team->GetName().c_str());    
    }

    // inform invited players about the removal
    sBattleGroundMgr.m_BattleGroundQueues[BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType())].BGEndedRemoveInvites(this);

    if (Source)
    {
        ChatHandler(Source).FillMessageData(&data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, Source->GetGUID(), winmsg);
        SendPacketToAll(&data);
    }

    // teleport spectators to recall position and remove spectator state
    Map::PlayerList const &PlList = m_Map->GetPlayers();

    if (!PlList.isEmpty())
        for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
            if (Player* pPlayer = i->getSource())
                if (pPlayer->IsSpectator())
                {
                    pPlayer->SetSpectator(false);
                    pPlayer->TeleportTo(pPlayer->_recallPosition.mapid, pPlayer->_recallPosition.coord_x, pPlayer->_recallPosition.coord_y, pPlayer->_recallPosition.coord_z, pPlayer->_recallPosition.orientation);
                }

    if (sBattleGroundMgr.IsBGEventEnding(GetTypeID()))
        sBattleGroundMgr.CheckBGEventAllEnded();
}

uint32 BattleGround::GetBattlemasterEntry() const
{
    switch (GetTypeID())
    {
        case BATTLEGROUND_AV: return 15972;
        case BATTLEGROUND_WS: return 14623;
        case BATTLEGROUND_AB: return 14879;
        case BATTLEGROUND_EY: return 22516;
        case BATTLEGROUND_NA: return 20200;
        default:              return 0;
    }
}

void BattleGround::RewardItem(Player *plr, uint32 itemId, uint32 count)
{
    if (ObjectMgr::GetItemPrototype(itemId))
    {
        ItemPosCountVec dest;
        uint32 no_space_count = 0;
        uint8 msgLimit = plr->_CanTakeMoreSimilarItems(itemId, count, NULL, &no_space_count);
        if (msgLimit != EQUIP_ERR_OK)                       // convert to possible store amount
        {
            // no_space_count cannot be more than thisCount
            count -= no_space_count;
            if (!count)
                return; // limit reached, items are in bags
        }
        no_space_count = 0;

        // Check if limit is reached by mail items, and if yes -> don't try to add/send
        uint32 cntInMail = 0;
        const ItemMap& mailItems = plr->GetMailItems();
        for (ItemMap::const_iterator i = mailItems.begin(); i != mailItems.end(); ++i)
        {
            if (i->second->GetEntry() == itemId)
                cntInMail += i->second->GetCount();
        }
        // now check for limit including items found in mail
        if (cntInMail)
        {
            uint8 msgLimit = plr->_CanTakeMoreSimilarItems(itemId, count + cntInMail, NULL, &no_space_count);
            if (msgLimit != EQUIP_ERR_OK)                       // convert to possible store amount
            {
                // no_space_count can be more than thisCount, cause of cntInMail, so check like this:
                if (count <= no_space_count)
                    return; // limit reached, items are in bags/mail
                else
                    count -= no_space_count; // reduce count to reach limit with new item
            }
            no_space_count = 0;
        }

        // At this stage we are sure that with current count we won't break the limit -> so can send by mail if errors are found
        uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &no_space_count);
        if (msg != EQUIP_ERR_OK)                       // convert to possible store amount
            count -= no_space_count;

        if (!dest.empty())                // can add some
            if (Item* item = plr->StoreNewItem(dest, itemId, true, 0, "BATTLEGROUND"))
                plr->SendNewItem(item, count, true, false);

        if (no_space_count > 0)
            SendRewardMarkByMail(plr, itemId, no_space_count);
    }
}

void BattleGround::BGReward(Player *plr,bool win)
{
    if (!isBattleGround())
        return;

    if (!plr)
        return;

    if (InProgressDuration() < 5 * MILLISECONDS)
    {
        ChatHandler(plr).PSendSysMessage(16755);
        return;
    }

    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    //if (plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
    //    return;

    uint32 mark;

    switch (GetTypeID())
    {
    case BATTLEGROUND_AV:
        mark = 20560; // Alterac Valley
        break;
    case BATTLEGROUND_WS:
        mark = 20558; // Warsong Gulch
        break;
    case BATTLEGROUND_AB:
        mark = 20559; // Arathi Basin
        break;
    case BATTLEGROUND_EY:
        mark = 29024; // Eye of the Storm
        break;
    default:
        return;
    }

    uint32 mark_count = win ? sWorld.getConfig(RATE_BG_WIN) : sWorld.getConfig(RATE_BG_LOSS);

    RewardItem(plr, mark, mark_count);

    sLog.outLog(LOG_SPECIAL, "Honorlog: Player %s left BG %u type %u with honor amount %u (is prem %u) won %u", plr->GetName(), GetBgInstanceId(), GetTypeID(), plr->GetHonorPoints(), plr->GetSession()->isPremium(), win);

    //// reward BOJ on x3
    //if (sWorld.getConfig(CONFIG_REALM_TYPE) == REALM_X3)
    //{
    //    uint32 bojcount = won ? 3 : 1;
    //    RewardItem(plr, 29434, bojcount);
    //}

    // reward chest on x100
    if (sWorld.isEasyRealm())
    {
        if (GetTypeID() == BATTLEGROUND_AV)
        {
            uint8 emblem_count = win ? 3 : 1;
            uint32 bonus = plr->CalculateBonus(emblem_count);

            plr->GiveItem(ITEM_EMBLEM_OF_TRIUMPH, emblem_count + bonus);

            // send ad message
            if (bonus)
                plr->AddEvent(new PSendSysMessageEvent(*plr, 16653, plr->GetItemLink(ITEM_EMBLEM_OF_TRIUMPH).c_str(), bonus), 2000);
        }
        
        // won any BG
        if (win) 
        {
            plr->KilledMonster(695006, 0);

            if (sGameEventMgr.IsActiveEvent(1009))
            {
                if (roll_chance_f(20.45f))
                {
                    plr->GiveItem(FLAWLESS_LEGENDARY_KEY, 1);
                }
                else
                {
                    ChatHandler(plr).SendSysMessage(16758);
                }
            }
        }
    }
}

void BattleGround::SendRewardMarkByMail(Player *plr,uint32 mark, uint32 count)
{
    uint32 bmEntry = GetBattlemasterEntry();
    if (!bmEntry)
        return;

    ItemPrototype const* markProto = ObjectMgr::GetItemPrototype(mark);
    if (!markProto)
        return;

    if (Item* markItem = Item::CreateItem(mark,count,plr))
    {
        // save new item before send
        markItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

        int loc_idx = plr->GetSession()->GetSessionDbLocaleIndex();

        // subject: item name
        std::string subject = markProto->Name1;
        sObjectMgr.GetItemLocaleStrings(markProto->ItemId, loc_idx, &subject);

        // text
        std::string textFormat = plr->GetSession()->GetHellgroundString(LANG_BG_MARK_BY_MAIL);
        char textBuf[500];
        snprintf(textBuf,500,textFormat.c_str(),GetName(),GetName());
        uint32 itemTextId = sObjectMgr.CreateItemText(textBuf);

        MailDraft(subject, itemTextId)
            .AddItem(markItem)
            .SendMailTo(plr, MailSender(MAIL_CREATURE, bmEntry));
    }
}

void BattleGround::RewardQuest(Player *plr)
{
    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    //if (plr->GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
    //    return;

    uint32 quest;
    switch (GetTypeID())
    {
        case BATTLEGROUND_AV:
            quest = SPELL_AV_QUEST_REWARD;
            break;
        case BATTLEGROUND_WS:
            quest = SPELL_WS_QUEST_REWARD;
            break;
        case BATTLEGROUND_AB:
            quest = SPELL_AB_QUEST_REWARD;
            break;
        case BATTLEGROUND_EY:
            quest = SPELL_EY_QUEST_REWARD;
            break;
        default:
            return;
    }

    plr->CastSpell(plr, quest, true);
}

void BattleGround::BlockMovement(Player *plr)
{
    plr->SetClientControl(plr, false);                          // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

void BattleGround::RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket, bool punish)
{
    PlayerTeam team = GetPlayerTeam(guid);
    bool participant = false;
    // Remove from lists/maps

    BattleGroundPlayer bp;

    BattleGroundPlayerMap::iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        // @!bg_balancer
        --teamClassMaskCount[GetTeamIndexByTeamId(team)][itr->second.ClassMask];
        bp = itr->second;

        UpdatePlayersCountByTeam(team, true);   // -1 player
        m_Players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;

        if (isArena())
        {
            switch (GetArenaType())
            {
            case ARENA_TYPE_2v2:
                sBattleGroundMgr.inArenasCount[0]--; break;
            case ARENA_TYPE_3v3:
                sBattleGroundMgr.inArenasCount[1]--; break;
            case ARENA_TYPE_5v5:
                sBattleGroundMgr.inArenasCount[2]--; break;
            }
        }
    }

    if (isBattleGround())
    {
        std::map<uint64, BattleGroundScore*>::iterator itr2 = m_PlayerScores.find(guid);
        if (itr2 != m_PlayerScores.end())
        {
            BattleGroundScore *temp = itr2->second;  // delete player's score
            m_PlayerScores.erase(itr2);
            delete temp;			
        }
    }

    RemovePlayerFromResurrectQueue(guid);

    Player *plr = sObjectMgr.GetPlayerInWorld(guid);

    if (!plr)
    {
        if (Group * BgGroup = GetBgRaid(team))
        {
            if (!BgGroup->RemoveMember(guid, 0))               // group was disbanded
            {
                SetBgRaid(team, NULL);
                delete BgGroup;
            }
        }
    }

    // Cast Deserter for players that is logout > 1 minute and take rating
    if (isBattleGround() && punish)
    {
        uint32 spell_id = SPELL_BG_DESERTER;
        uint32 duration = DESERTER_BG_DURATION_LEAVE;

        std::string player_name;

        if (plr)
        {
            plr->AddAura(spell_id, plr, duration);
            player_name = plr->GetName();
        }
        else
        {
            RealmDataDatabase.PExecute("INSERT INTO character_aura (guid, caster_guid, item_guid, spell, maxduration, remaintime, remaincharges) VALUES ('%u', '%u', '0', '%u', '%u', '%u', '-1')", GUID_LOPART(guid), GUID_LOPART(guid), spell_id, duration, duration);
            sObjectMgr.GetPlayerNameByGUID(guid, player_name);
        }

        sLog.outLog(LOG_BG, "ID %u: Player %s left from BG (%s)", GetBgInstanceId(), player_name.c_str(), GetPlayerCountInfo().c_str());
        getObjectMgr()->BGModStats(guid, false, (bool)sWorld.getConfig(CONFIG_RATED_BG_ENABLED)); // consider as loss
    }

    // should remove spirit of redemption
    if (plr && plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

    // should remove SPELL_AURA_MOD_STUN
    if (plr && plr->isAlive() && plr->HasAuraType(SPELL_AURA_MOD_STUN))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_STUN);            //testfixbg

    if (plr && !plr->isAlive())                              // resurrect on exit
    {
        plr->ResurrectPlayer(1.0f);
        plr->SpawnCorpseBones();
    }

    if (plr && participant && isArena() && isRated() && 
        (GetStatus() == STATUS_IN_PROGRESS || (m_ArenaType == ARENA_TYPE_2v2 && GetStatus() == STATUS_WAIT_JOIN)) && 
        !sWorld.IsStopped())
    {
        //left a rated match while the encounter was in progress, consider as loser
        //need to be done before RemovePlayer which can cause EndBattleground and last removed player rating wont be updated
        if (!team) 
            team = plr->GetBGTeam();
        ArenaTeam* winner_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(team == HORDE? ALLIANCE : HORDE));
        ArenaTeam* loser_arena_team = sObjectMgr.GetArenaTeamById(GetArenaTeamIdForTeam(team));

        if (winner_arena_team && loser_arena_team)
        {
            loser_arena_team->MemberLost(plr, winner_arena_team->GetRating());

            if (m_ArenaType == ARENA_TYPE_2v2)
                sLog.outLog(LOG_ARENA, "Player %s left arena %u before it ended.", plr->GetName(), GetTempArenaGuid());

            loser_arena_team->SaveToDB();
            loser_arena_team->Stats(plr->GetSession());
        }

        //BattleGroundScore* score = m_PlayerScores[guid];
            //" Statistics for %s (GUID: %u, Team: %d, IP: %s): %u damage, %u healing, %u killing blows",
            //plr->GetName(), GetTempArenaGuid(),
            //plr->GetName(), guid, loser_arena_team ? loser_arena_team->GetId() : 0,
            //plr->GetSession()->GetRemoteAddress().c_str(), score->DamageDone, score->HealingDone, score->KillingBlows);
        /*if (sWorld.getConfig(CONFIG_ARENA_EXPORT_RESULTS))
        {
            RealmDataDatabase.PExecute("INSERT INTO arena_fights_detailed VALUES (%u, " UI64FMTD", %u, %u, %u, %u, %u, %i);",
                GetInstanceID(), guid, loser_arena_team->GetId(),
                score->DamageDone, score->HealingDone, score->KillingBlows, persRating, persDiff);
        }*/
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code
    DecreaseInvitedCount(team);

    if (plr)
    {
        plr->ClearAfkReports();
        plr->RemoveArenaAuras(true);    // removes only debuffs / dots etc., we don't want the player to die after porting out. Was only for arena.
        if (sWorld.getConfig(CONFIG_BATTLEGROUND_ALLOW_DIFFERENT_FACTION))
            plr->setFactionForRace(plr->GetRace()); // cant call restoreFaction, cause player is not yet removed from BG and BGteam

        if (participant) // if the player was a match participant, remove auras, calc rating, update queue
        {
            if (!team) team = plr->GetBGTeam();

            BattleGroundTypeId bgTypeId = GetTypeID();
            BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());
            // if arena, remove the specific arena auras
            if (isArena())
            {
                plr->GetMotionMaster()->MovementExpired();

                bgTypeId=BATTLEGROUND_AA;       // set the bg type to all arenas (it will be used for queue refreshing)

                // summon old pet if there was one and there isn't a current pet
                if (!plr->GetPet() && plr->GetTemporaryUnsummonedPetNumber())
                {
                    Pet* NewPet = new Pet;
                    if (!NewPet->LoadPetFromDB(plr, 0, (plr)->GetTemporaryUnsummonedPetNumber(), true))
                        delete NewPet;

                    (plr)->SetTemporaryUnsummonedPetNumber(0);
                }
            }

            WorldPacket data;
            if (SendPacket)
            {
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, team, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0);
                plr->SendPacketToSelf(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            plr->RemoveBattleGroundQueueId(bgQueueTypeId);

            //we should update battleground queue, but only if bg isn't ending
            if (isBattleGround() && GetStatus() < STATUS_WAIT_LEAVE)
                sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, GetBracketId());

            Group * group = plr->GetGroup();
            // remove from raid group if exist
            if (group && group == GetBgRaid(team))
            {
                if (!group->RemoveMember(guid, 0))               // group was disbanded
                {
                    SetBgRaid(team, NULL);
                    delete group;
                }
            }

            // Let others know
            sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(&data, guid);
            SendPacketToTeam(team, &data, plr, false);
        }

        // Do next only if found in battleground
        plr->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);                          // We're not in BG.

        // reset destination bg team
        plr->SetBGTeam(TEAM_NONE);
        plr->GetMotionMaster()->MovementExpired();

        if (plr->IsSpectator())
            plr->SetSpectator(false);

        if (Transport)
            plr->TeleportTo(plr->GetBattleGroundEntryPoint());

        // remove pet
        Pet* pet = plr->GetPet();
        if (pet)
        {            
            // DANGEROUNS? do i need this line?
            plr->SetTemporaryUnsummonedPetNumber(pet->isControlled() && !pet->isTemporarySummoned() ? pet->GetCharmInfo()->GetPetNumber() : 0);
            plr->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT);
        }
    }

    if (plr)
        sLog.outLog(isBattleGround() ? LOG_BG : LOG_ARENA, "ID %u: Player %s left %s, classmask %u (%s)", GetBgInstanceId(), plr->GetName(), GetBGTeamName(team), bp.ClassMask, GetPlayerCountInfo().c_str());
    else
    {
        std::string name = "UNKNOWN";
        sObjectMgr.GetPlayerNameByGUID(guid, name);

        sLog.outLog(isBattleGround() ? LOG_BG : LOG_ARENA, "ID %u: Player %s left %s, classmask %u (%s)", GetBgInstanceId(), name.c_str(), GetBGTeamName(team), bp.ClassMask, GetPlayerCountInfo().c_str());
    }

    if (!GetPlayersSize() && !GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
    {
        // if no players left AND no invitees left, set this bg to delete in next update
        // direct deletion could cause crashes
        SetDeleteThis();
        // return to prevent addition to freeslotqueue
        return;
    }

    // a player exited the battleground, so there are free slots. add to queue
    this->AddToBGFreeSlotQueue();
}

// this method is called when no players remains in battleground
void BattleGround::Reset()
{
    SetQueueType(MAX_BATTLEGROUND_QUEUES);
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);
    SetLastResurrectTime(0);
    SetArenaType(0);
    SetRated(false);
    m_Map = NULL;
    m_TempArenaRatedGUID = 0;

    m_Events = 0;

    if (m_InvitedAlliance > 0 || m_InvitedHorde > 0)
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround system ERROR: bad counter, m_InvitedAlliance: %d, m_InvitedHorde: %d", m_InvitedAlliance, m_InvitedHorde);

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    for (BattleGroundScoreMap::iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); itr++)
    {
        delete itr->second;
    }
    m_Players.clear();
    m_PlayerScores.clear();

    // reset BGSubclass
    ResetBGSubclass();
}

void BattleGround::StartBattleGround()
{
    ///this method should spawn spirit guides and so on
    SetStartTime(0);
    SetLastResurrectTime(0);
    AnnounceBGStart();
    if (m_IsRated)
    {
        m_TempArenaRatedGUID = sObjectMgr.GenerateTempGuid(TEMPGUID_ARENA);
        sLog.outLog(LOG_ARENA, "Arena match %u type: %u for Team1Id: %u - Team2Id: %u started.", GetTempArenaGuid(), m_ArenaType, m_ArenaTeamIds[TEAM_ALLIANCE], m_ArenaTeamIds[TEAM_HORDE]);
    }
}

void BattleGround::AnnounceBGStart()
{
    if (!sWorld.getConfig(CONFIG_BATTLEGROUND_ANNOUNCE_START))
        return;

    int32 string_id;
    switch (m_TypeID)
    {
        case BATTLEGROUND_WS:
            string_id = 15221; break;
        case BATTLEGROUND_AB:
            string_id = 15222; break;
        case BATTLEGROUND_EY:
            string_id = 15223; break;
        case BATTLEGROUND_AV:
            string_id = 15224; break;
        default: return;
    }

    sLog.outLog(LOG_DEFAULT, "BG %u started", m_TypeID);

	std::stringstream ss;
	ss << m_BracketLevelMin;
	if (m_BracketLevelMin != sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
		ss << "-" << m_BracketLevelMax;

    sWorld.SendWorldText(string_id, ACC_DISABLED_BGANN, ss.str().c_str());

	sLog.outLog(LOG_DISCORD, "#BG %s", sObjectMgr.GetHellgroundString(string_id, 1));
}

void BattleGround::AddPlayer(Player *plr)
{
    // remove afk from player
    if (plr->isAFK())
        plr->ToggleAFK();
    
    PlayerTeam team = plr->GetBGTeam();
    uint64 guid = plr->GetGUID();
    plr->ClearAfkReports();

    BattleGroundPlayer bp;
    bp.LastOnlineTime = 0;
    bp.Team = team;
	bp.Rated = false;
	bp.PlayedTime = 0;
	bp.ClassMask = plr->GetBGClassMask();
	bp.haship = plr->GetHashIP();

    if (isBattleGround() && !plr->bg_premade_leader_guid)
    {
        if (GetFreeBGSlots() == 0)
        {
            DecreaseInvitedCount(team);

            BattleGroundTypeId bgTypeId = GetTypeID();
            BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType());

            WorldPacket data;
            sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, team, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0);
            plr->SendPacketToSelf(&data);
            plr->RemoveBattleGroundQueueId(bgQueueTypeId);

            plr->GetMotionMaster()->MovementExpired();
            plr->TeleportTo(plr->GetBattleGroundEntryPoint());

            ChatHandler(plr).PSendSysMessage(16746);
            sLog.outLog(LOG_BG, "ID %u: Player %s kicked just after teleport due to no slots", GetBgInstanceId(), plr->GetName());
            return;
        }
    }

    // Add to list/maps
    m_Players[guid] = bp;

    if (GetFreeBGSlots())
        AddToBGFreeSlotQueue();

    UpdatePlayersCountByTeam(team, false); // +1 player
    ++teamClassMaskCount[GetTeamIndexByTeamId(team)][plr->GetBGClassMask()];

    if (sWorld.getConfig(CONFIG_RATED_BG_ENABLED) && isBattleGround() && !m_RatedBG && GetPlayersSize() >= 4)
        m_RatedBG = true;

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);
    SendPacketToTeam(team, &data, plr, false);

    plr->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    plr->CombatStop();
    plr->getHostileRefManager().deleteReferences();
    plr->RemoveArenaSpellCooldowns();
    if (plr->IsSpectator())
        plr->SetSpectator(false);

    // should remove SPELL_AURA_MOD_STUN
    if (plr && plr->isAlive() && plr->HasAuraType(SPELL_AURA_MOD_STUN))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_STUN);            //testfixbg

    plr->restorePowers();

    // add arena specific auras
    if (isArena())
    {
        //plr->RemoveArenaSpellCooldowns(); already done above
        plr->RemoveArenaAuras();
        plr->GetMotionMaster()->MoveIdle();
        plr->RemoveAllEnchantments();
        if (team == ALLIANCE)                                // gold
        {
            if (plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GOLD_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GOLD_FLAG,true);
        }
        else                                                // green
        {
            if (plr->GetTeam() == HORDE)
                plr->CastSpell(plr, SPELL_HORDE_GREEN_FLAG,true);
            else
                plr->CastSpell(plr, SPELL_ALLIANCE_GREEN_FLAG,true);
        }

        plr->DestroyConjuredItems(true);

        Pet* pet = plr->GetPet();
        if (pet) // never has pet - cause this is called after TeleportTo - and in that function pet is removed
        {
            if (pet->getPetType() == SUMMON_PET || pet->getPetType() == HUNTER_PET)
            {
                (plr)->SetTemporaryUnsummonedPetNumber(pet->GetCharmInfo()->GetPetNumber());
                (plr)->SetOldPetSpell(pet->GetUInt32Value(UNIT_CREATED_BY_SPELL));
            }
            pet->RemoveArenaAuras();
            plr->RemovePet(NULL,PET_SAVE_NOT_IN_SLOT);
        }
        else
        {
            // Remove auras off unsummoned pet
            if (plr->GetTemporaryUnsummonedPetNumber())
                RealmDataDatabase.PExecute("DELETE FROM pet_aura WHERE guid = '%u'", plr->GetTemporaryUnsummonedPetNumber());
            else if (plr->GetClass() == CLASS_HUNTER)
            {
                QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT id FROM character_pet WHERE owner = '%u' AND (slot = '0' OR slot = '3') ", plr->GetGUIDLow());
                if (result)
                {
                    Field *fields = result->Fetch();
                    uint32 petNumber = fields[0].GetUInt32();
                    if (petNumber)
                        RealmDataDatabase.PExecute("DELETE FROM pet_aura WHERE guid = '%u'", petNumber);
                }

            }

            plr->SetTemporaryUnsummonedPetNumber(0);
        }

        if (GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
        {
            plr->CastSpell(plr, SPELL_ARENA_PREPARATION, true);

            /*plr->SetHealth(plr->GetMaxHealth());
            plr->SetPower(POWER_MANA, plr->GetMaxPower(POWER_MANA));

            if (plr->GetPower(POWER_RAGE))
                plr->SetPower(POWER_RAGE, 0);*/ // already done above
        }

        switch (GetArenaType())
        {
        case ARENA_TYPE_2v2:
        sBattleGroundMgr.inArenasCount[0]++; break;
        case ARENA_TYPE_3v3:
        sBattleGroundMgr.inArenasCount[1]++; break;
        case ARENA_TYPE_5v5:
        sBattleGroundMgr.inArenasCount[2]++; break;
        }
        //ChatHandler(plr).SendSysMessage("NOTICE: If you are ready, click on crystal near you. So, when everyone are ready arena preparation can end earlier.");
    }
    else
    {
        if (GetStatus() == STATUS_WAIT_JOIN)                 // not started yet
            plr->CastSpell(plr, SPELL_PREPARATION, true);   // reduces all mana cost of spells.
    }

    plr->RemoveCharmAuras();
    plr->RemoveMovementImpairingAuras();

    // vial stacks
    if (plr->GetClass() == CLASS_DRUID || plr->GetClass() == CLASS_PALADIN || plr->GetClass() == CLASS_PRIEST || plr->GetClass() == CLASS_SHAMAN)
    {
        for (int i = EQUIPMENT_SLOT_TRINKET1; i <= EQUIPMENT_SLOT_TRINKET2; i++)
        {
            Item* itemTarget = plr->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (itemTarget && itemTarget->GetEntry() == 34471)
            {
                Aura* aura = plr->GetAura(45062, 0);
                uint32 stack = (!aura) ? 0 : aura->GetStackAmount();

                do
                {
                    plr->AddAura(45062, plr);
                    stack++;
                } while (stack < 20);
            }
        }
    }

    AddOrSetPlayerToCorrectBgGroup(plr, guid, team);
    BGRealStats(plr);

    // in arena, autochange leader to first connected warrior
    // this is allows him to manage group to buff with shouts
    if (isArena() && plr->GetClass() == CLASS_WARRIOR)
    {    
        if (Group* grp = plr->GetGroup())
        {
            uint64 leader_guid = grp->GetLeaderGUID();
            Player* leader = sObjectMgr.GetPlayerOnline(leader_guid);
            if (leader && leader->GetClass() != CLASS_WARRIOR)
                grp->ChangeLeader(plr->GetGUID());
        }
    }
    else if (isBattleGround() && GetStatus() == STATUS_WAIT_JOIN)
    {
        if (m_playerMaxKillCount[GetTeamIndexByTeamId(team)] < plr->GetTotalKills())
        {
            Group* grp = plr->GetGroup();
            if (grp && grp->GetLeaderGUID() != plr->GetGUID())
                grp->ChangeLeader(plr->GetGUID());

            m_playerMaxKillCount[GetTeamIndexByTeamId(team)] = plr->GetTotalKills();
        } 
    }

    sLog.outLog(isBattleGround() ? LOG_BG : LOG_ARENA, "ID %u: Player %s joined to %s, classmask %u (%s)", GetBgInstanceId(), plr->GetName(), GetBGTeamName(team), bp.ClassMask, GetPlayerCountInfo().c_str());
}

void BattleGround::AddOrSetPlayerToCorrectBgGroup(Player *plr, uint64 plr_guid, PlayerTeam team)
{
    if (Group* group = GetBgRaid(team))                     // raid already exist
    {
        if (group->IsMember(plr_guid))
        {
            uint8 subgroup = group->GetMemberGroup(plr_guid);
            plr->SetBattleGroundRaid(group, subgroup);
        }
        else
        {
            group->AddMember(plr_guid, plr->GetName());
            if (Group* originalGroup = plr->GetOriginalGroup())
                if (originalGroup->IsLeader(plr_guid))
                    group->ChangeLeader(plr_guid);
        }
    }
    else                                                    // first player joined
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(plr_guid, plr->GetName());
    }
}
/* This method should be called only once ... it adds pointer to queue */
void BattleGround::AddToBGFreeSlotQueue()
{
    // make sure to add only once
    if (!m_InBGFreeSlotQueue && isBattleGround())
    {
        sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

/* This method removes this battleground from free queue - it must be called when deleting battleground - not used now*/
void BattleGround::RemoveFromBGFreeSlotQueue()
{
    // set to be able to re-add if needed
    m_InBGFreeSlotQueue = false;
    // uncomment this code when battlegrounds will work like instances
    for (BGFreeSlotQueueType::iterator itr = sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].end(); ++itr)
    {
        if ((*itr)->GetBgInstanceId() == GetBgInstanceId())
        {
            sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].erase(itr);
            return;
        }
    }
}

uint32 BattleGround::GetFreeBGSlotsForTeam(PlayerTeam team, bool over_limit) const
{
    // Return free slot count to MaxPlayerPerTeam
    if (GetStatus() == STATUS_WAIT_JOIN || GetStatus() == STATUS_IN_PROGRESS)
    {
        uint32 playersCount = GetPlayersCountByTeam(team);
        uint32 otherTeamPlayersCount = GetPlayersCountByTeam(OTHER_PLAYER_TEAM(team));
        uint32 maxPlayersPerTeam = GetMaxPlayersPerTeam();

        if (over_limit)
        {
            if (otherTeamPlayersCount > maxPlayersPerTeam)
            {
                // with premade limit = 3 can be 1 or 2
                uint32 bonusSlots = otherTeamPlayersCount - maxPlayersPerTeam;
                return bonusSlots;
            }
        }
        else 
        {
            return maxPlayersPerTeam - playersCount;
        }  
    }

    return 0;
}

uint32 BattleGround::GetFreeBGSlots() const
{
    if (GetPlayersSize() > GetMaxPlayers())
    {
        uint32 horde = GetPlayersCountByTeam(HORDE);
        uint32 alliance = GetPlayersCountByTeam(ALLIANCE);

        if (horde == alliance)
            return 0; // no slots

        return GetPlayersSize() - GetMaxPlayers();
    }
	else
		return GetMaxPlayers() - GetPlayersSize();
}

void BattleGround::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{
    //this procedure is called from virtual function implemented in bg subclass
    std::map<uint64, BattleGroundScore*>::iterator itr = m_PlayerScores.find(Source->GetGUID());

    if (itr == m_PlayerScores.end())                         // player not found...
        return;

    switch (type)
    {
        case SCORE_KILLING_BLOWS:                           // Killing blows
            itr->second->KillingBlows += value;
            break;
        case SCORE_DEATHS:                                  // Deaths
            itr->second->Deaths += value;
            if (itr->second->Deaths >= 30)
                Source->ToUnit()->WorthHonor = true;
            break;
        case SCORE_HONORABLE_KILLS:                         // Honorable kills
            itr->second->HonorableKills += value;
            break;
        case SCORE_BONUS_HONOR:                             // Honor bonus
            // do not add honor in arenas
            if (isBattleGround())
            {
                // reward honor instantly
                if (Source->RewardHonor(NULL, 1, value))
                    itr->second->BonusHonor += value;
            }
            break;
            //used only in EY, but in MSG_PVP_LOG_DATA opcode
        case SCORE_DAMAGE_DONE:                             // Damage Done
            itr->second->DamageDone += value;
            break;
        case SCORE_HEALING_DONE:                            // Healing Done
            itr->second->HealingDone += value;
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: Unknown player score type %u", type);
            break;
    }
}

void BattleGround::AddPlayerToResurrectQueue(uint64 npc_guid, uint64 player_guid)
{
    m_ReviveQueue[npc_guid].push_back(player_guid);

    Player *plr = sObjectMgr.GetPlayerInWorld(player_guid);
    if (!plr)
        return;

    plr->CastSpell(plr, SPELL_WAITING_FOR_RESURRECT, true);
}

void BattleGround::RemovePlayerFromResurrectQueue(uint64 player_guid)
{
    for (std::map<uint64, std::vector<uint64> >::iterator itr = m_ReviveQueue.begin(); itr != m_ReviveQueue.end(); ++itr)
    {
        for (std::vector<uint64>::iterator itr2 =(itr->second).begin(); itr2 != (itr->second).end(); ++itr2)
        {
            if (*itr2 == player_guid)
            {
                (itr->second).erase(itr2);

                Player *plr = sObjectMgr.GetPlayerInWorld(player_guid);
                if (!plr)
                    return;

                plr->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);

                return;
            }
        }
    }
}

bool BattleGround::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime)
{
    Map * map = GetMap();
    if (!map)
        return false;

    // must be created this way, adding to godatamap would add it to the base map of the instance
    // and when loading it (in go::LoadFromDB()), a new guid would be assigned to the object, and a new object would be created
    // so we must create it specific for this instance
    GameObject * go = new GameObject;
    if (!go->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),entry, map,x,y,z,o,rotation0,rotation1,rotation2,rotation3,100,GO_STATE_READY))
    {
        sLog.outLog(LOG_DB_ERR, "Gameobject template %u not found in database! BattleGround not created!", entry);
        sLog.outLog(LOG_DEFAULT, "ERROR: Cannot create gameobject template %u! BattleGround not created!", entry);
        delete go;
        return false;
    }
/*
    uint32 guid = go->GetGUIDLow();

    // without this, UseButtonOrDoor caused the crash, since it tried to get go info from godata
    // iirc that was changed, so adding to go data map is no longer required if that was the only function using godata from GameObject without checking if it existed
    GameObjectData& data = sObjectMgr.NewGOData(guid);

    data.id             = entry;
    data.mapid          = GetMapId();
    data.posX           = x;
    data.posY           = y;
    data.posZ           = z;
    data.orientation    = o;
    data.rotation0      = rotation0;
    data.rotation1      = rotation1;
    data.rotation2      = rotation2;
    data.rotation3      = rotation3;
    data.spawntimesecs  = respawnTime;
    data.spawnMask      = 1;
    data.animprogress   = 100;
    data.go_state       = 1;
*/
    // add to world, so it can be later looked up from HashMapHolder
    map->Add(go);
    m_BgObjects[type] = go->GetGUID();
    return true;
}

//some doors aren't despawned so we cannot handle their closing in gameobject::update()
//it would be nice to correctly implement GO_ACTIVATED state and open/close doors in gameobject code
void BattleGround::DoorClose(uint32 type)
{
    Map * tmpMap = GetMap();

    if (!tmpMap)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround::DoorClose: map not found");
        return;
    }

    GameObject *obj = tmpMap->GetGameObject(m_BgObjects[type]);
    if (obj)
    {
        //if doors are open, close it
        if (obj->getLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            //change state to allow door to be closed
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
    }
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: Door object not found (cannot close doors)");
    }
}

void BattleGround::DoorOpen(uint32 type)
{
    Map * tmpMap = GetMap();

    if (!tmpMap)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround::DoorClose: map not found");
        return;
    }

    GameObject *obj = tmpMap->GetGameObject(m_BgObjects[type]);
    if (obj)
    {
        //change state to be sure they will be opened
        if (obj->GetGoState() == GO_STATE_READY)
        {
            // this is kinda hack for doors which were not activated by active object in grid
            if (obj->getLootState() == GO_NOT_READY)
                obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
        else
        {
            obj->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
            obj->SetLootState(GO_ACTIVATED);
        }
    }
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: Door object not found! - doors will be closed.");
    }
}

GameObject* BattleGround::GetBGObject(uint32 type)
{
    Map * tmpMap = GetMap();

    if (!tmpMap)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround::GetBGObject: map not found");
        return NULL;
    }

    GameObject *obj = tmpMap->GetGameObject(m_BgObjects[type]);
    if (!obj)
        sLog.outLog(LOG_DEFAULT, "ERROR: couldn't get gameobject %i",type);
    return obj;
}

Creature* BattleGround::GetBGCreature(uint32 type)
{
    Map * tmp = GetMap();
    Creature *creature = NULL;

    if (tmp)
        creature = tmp->GetCreature(m_BgCreatures[type]);
    if (!creature)
        sLog.outLog(LOG_DEFAULT, "ERROR: couldn't get creature %i",type);
    return creature;
}

void BattleGround::SpawnBGObject(uint32 type, uint32 respawntime)
{
    Map * map = GetMap();
    if (!map)
        return;
    if (respawntime == 0)
    {
        GameObject *obj = map->GetGameObject(m_BgObjects[type]);
        if (obj)
        {
            //we need to change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
            if (obj->getLootState() == GO_JUST_DEACTIVATED)
                obj->SetLootState(GO_READY);
            obj->SetRespawnTime(0);
            map->Add(obj);
        }
    }
    else
    {
        GameObject *obj = map->GetGameObject(m_BgObjects[type]);
        if (obj)
        {
            map->Add(obj);
            obj->SetRespawnTime(respawntime);
            obj->SetLootState(GO_JUST_DEACTIVATED);
            obj->UpdateObjectVisibility();
        }
    }
}

Creature* BattleGround::AddCreature(uint32 entry, uint32 type, PlayerTeam teamval, float x, float y, float z, float o, uint32 respawntime)
{
    Map * map = GetMap();
    if (!map)
        return NULL;

    Creature* pCreature = new Creature;
    if (!pCreature->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT), map, entry, teamval, x, y, z, o))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't create creature entry: %u",entry);
        delete pCreature;
        return NULL;
    }

    pCreature->SetHomePosition(x, y, z, o);

    map->Add(pCreature);
    m_BgCreatures[type] = pCreature->GetGUID();

    return  pCreature;
}
/*
void BattleGround::SpawnBGCreature(uint32 type, uint32 respawntime)
{
    Map * map = sMapMgr.FindMap(GetMapId(),GetInstanciableInstanceId());
    if (!map)
        return false;

    if (respawntime == 0)
    {
        Creature *obj = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
        if (obj)
        {
            //obj->Respawn();                               // bugged
            obj->SetRespawnTime(0);
            sObjectMgr.SaveCreatureRespawnTime(obj->GetGUIDLow(), GetInstanceID(), 0);
            map->Add(obj);
        }
    }
    else
    {
        Creature *obj = HashMapHolder<Creature>::Find(m_BgCreatures[type]);
        if (obj)
        {
            obj->setDeathState(DEAD);
            obj->SetRespawnTime(respawntime);
            map->Add(obj);
        }
    }
}
*/
bool BattleGround::DelCreature(uint32 type)
{
    Map * map = GetMap();
    if (!map)
        return false;

    if (!m_BgCreatures[type])
        return true;

    Creature *cr = map->GetCreature(m_BgCreatures[type]);
    if (!cr)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't find creature guid: %u",GUID_LOPART(m_BgCreatures[type]));
        return false;
    }
    //TODO: only delete creature after not in combat
    cr->AddObjectToRemoveList();
    m_BgCreatures[type] = 0;
    return true;
}

bool BattleGround::DelObject(uint32 type, bool setGoState)
{
    Map * map = GetMap();
    if (!map)
        return false;

    if (!m_BgObjects[type])
        return true;

    GameObject *obj = map->GetGameObject(m_BgObjects[type]);
    if (!obj)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't find gobject guid: %u",GUID_LOPART(m_BgObjects[type]));
        return false;
    }
    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete(setGoState);
    m_BgObjects[type] = 0;
    return true;
}

bool BattleGround::AddSpiritGuide(uint32 type, float x, float y, float z, float o, PlayerTeam team)
{
    uint32 entry = 0;

    if (team == ALLIANCE)
        entry = 13116;
    else
        entry = 13117;

    Creature* pCreature = AddCreature(entry,type,team,x,y,z,o);
    if (!pCreature)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Can't create Spirit guide. BattleGround not created!");
        EndNow();
        return false;
    }

    pCreature->setDeathState(DEAD);

    pCreature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, pCreature->GetGUID());
    // aura
    pCreature->SetUInt32Value(UNIT_FIELD_AURA, SPELL_SPIRIT_HEAL_CHANNEL);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAFLAGS, 0x00000009);
    pCreature->SetUInt32Value(UNIT_FIELD_AURALEVELS, 0x0000003C);
    pCreature->SetUInt32Value(UNIT_FIELD_AURAAPPLICATIONS, 0x000000FF);
    // casting visual effect
    pCreature->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_SPIRIT_HEAL_CHANNEL);
    // correct cast speed
    pCreature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    //pCreature->CastSpell(pCreature, SPELL_SPIRIT_HEAL_CHANNEL, true);

    return true;
}

//void BattleGround::SendMessageToAll(char const* text)
//{
//    WorldPacket data;
//    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
//    SendPacketToAll(&data);
//}

//void BattleGround::SendMessageToTeam(uint32 team, char const* text)
//{
//    WorldPacket data;
//    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
//    SendPacketToTeam(team, &data);
//}

void BattleGround::SendNotifyToTeam(PlayerTeam team, int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player* plr = sObjectMgr.GetPlayerInWorld(itr->first);
        if (!plr)
            continue;

        uint32 _team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!_team)
            _team = plr->GetBGTeam();

        if (_team != team)
            continue;

        int loc_idx = plr->GetSession()->GetSessionDbLocaleIndexCheckRussian();
        uint32 cache_idx = loc_idx + 1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if (data_cache.size() < cache_idx + 1 || data_cache[cache_idx].empty())
        {
            if (data_cache.size() < cache_idx + 1)
                data_cache.resize(cache_idx + 1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr.GetHellgroundString(string_id, loc_idx);

            char buf[1000];

            va_list argptr;
            va_start(argptr, string_id);
            vsnprintf(buf, 1000, text, argptr);
            va_end(argptr);

            char* pos = &buf[0];

            while (char* line = ChatHandler::LineFromMessage(pos))
            {
                WorldPacket* data = new WorldPacket();
                ChatHandler::FillMessageData(data, NULL, CHAT_MSG_RAID_BOSS_WHISPER, LANG_UNIVERSAL, NULL, 0, line, plr);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for (int i = 0; i < data_list->size(); ++i)
            plr->SendPacketToSelf((*data_list)[i]);
    }

    // free memory
    for (int i = 0; i < data_cache.size(); ++i)
        for (int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

void BattleGround::SendMessageToTeam(PlayerTeam team, int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
        if (!plr)
            continue;

        uint32 _team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!_team)
            _team = plr->GetBGTeam();

        if (_team != team)
            continue;

        int loc_idx = plr->GetSession()->GetSessionDbLocaleIndexCheckRussian();
        uint32 cache_idx = loc_idx + 1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if (data_cache.size() < cache_idx + 1 || data_cache[cache_idx].empty())
        {
            if (data_cache.size() < cache_idx + 1)
                data_cache.resize(cache_idx + 1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr.GetHellgroundString(string_id, loc_idx);

            char buf[1000];

            va_list argptr;
            va_start(argptr, string_id);
            vsnprintf(buf, 1000, text, argptr);
            va_end(argptr);

            char* pos = &buf[0];

            while (char* line = ChatHandler::LineFromMessage(pos))
            {
                WorldPacket* data = new WorldPacket();
                ChatHandler::FillMessageData(data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, line, NULL);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for (int i = 0; i < data_list->size(); ++i)
            plr->SendPacketToSelf((*data_list)[i]);
    }

    // free memory
    for (int i = 0; i < data_cache.size(); ++i)
        for (int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

//void BattleGround::PrepareMessageToAll(char const *format, ...)
//{
    //va_list ap;
    //char str [1024];
    //va_start(ap, format);
    //vsnprintf(str,1024,format, ap);
    //va_end(ap);
    //SendMessageToAll(str);
//}

//void BattleGround::SendMessageToAll(int32 entry)
//{
//    char const* text = GetHellgroundString(entry);
//    WorldPacket data;
//    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
//    SendPacketToAll(&data);
//}

void BattleGround::SendMessageToAll(int32 string_id, uint8 type, WorldSession* session, const uint64 guid, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
        if (!plr)
            continue;

        int loc_idx = plr->GetSession()->GetSessionDbLocaleIndexCheckRussian();
        uint32 cache_idx = loc_idx + 1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if (data_cache.size() < cache_idx + 1 || data_cache[cache_idx].empty())
        {
            if (data_cache.size() < cache_idx + 1)
                data_cache.resize(cache_idx + 1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr.GetHellgroundString(string_id, loc_idx);

            char buf[1000];

            va_list argptr;
            va_start(argptr, guid);
            vsnprintf(buf, 1000, text, argptr);
            va_end(argptr);

            char* pos = &buf[0];

            while (char* line = ChatHandler::LineFromMessage(pos))
            {
                WorldPacket* data = new WorldPacket();
                ChatHandler::FillMessageData(data, session, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, guid, line, NULL);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for (int i = 0; i < data_list->size(); ++i)
            plr->GetSession()->SendPacket((*data_list)[i]);
    }

    // free memory
    for (int i = 0; i < data_cache.size(); ++i)
        for (int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

void BattleGround::SendMessageToAll(int32 string_id, ...)
{
    std::vector<std::vector<WorldPacket*> > data_cache;     // 0 = default, i => i-1 locale index

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.LastOnlineTime)
            continue;

        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
        if (!plr)
            continue;

        int loc_idx = plr->GetSession()->GetSessionDbLocaleIndexCheckRussian();
        uint32 cache_idx = loc_idx + 1;

        std::vector<WorldPacket*>* data_list;

        // create if not cached yet
        if (data_cache.size() < cache_idx + 1 || data_cache[cache_idx].empty())
        {
            if (data_cache.size() < cache_idx + 1)
                data_cache.resize(cache_idx + 1);

            data_list = &data_cache[cache_idx];

            char const* text = sObjectMgr.GetHellgroundString(string_id, loc_idx);

            char buf[1000];

            va_list argptr;
            va_start(argptr, string_id);
            vsnprintf(buf, 1000, text, argptr);
            va_end(argptr);

            char* pos = &buf[0];

            while (char* line = ChatHandler::LineFromMessage(pos))
            {
                WorldPacket* data = new WorldPacket();
                ChatHandler::FillMessageData(data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, line, NULL);
                data_list->push_back(data);
            }
        }
        else
            data_list = &data_cache[cache_idx];

        for (int i = 0; i < data_list->size(); ++i)
            plr->GetSession()->SendPacket((*data_list)[i]);
    }

    // free memory
    for (int i = 0; i < data_cache.size(); ++i)
        for (int j = 0; j < data_cache[i].size(); ++j)
            delete data_cache[i][j];
}

void BattleGround::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(TIME_TO_AUTOREMOVE);
    // inform invited players about the removal
    sBattleGroundMgr.m_BattleGroundQueues[BattleGroundMgr::BGQueueTypeId(GetTypeID(), GetArenaType())].BGEndedRemoveInvites(this);
}

// Battleground messages are localized using the dbc lang, they are not client language dependent
const char *BattleGround::GetHellgroundString(int32 entry)
{
    // FIXME: now we have different DBC locales and need localized message for each target client
    return sObjectMgr.GetHellgroundStringForDBCLocale(entry);
}

bool BattleGround::HandlePlayerUnderMap(Player * plr, float z)
{
    EventPlayerDroppedFlag(plr);

    WorldSafeLocsEntry const *graveyard = GetClosestGraveYard(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetBGTeam());
    if (graveyard)
    {
        plr->NearTeleportTo(graveyard->x, graveyard->y, graveyard->z, plr->GetOrientation());
        if (plr->isDead())                                        // not send if alive, because it used in TeleportTo()
        {
            WorldPacket data(SMSG_DEATH_RELEASE_LOC, 4*4);  // show spirit healer position on minimap
            data << graveyard->map_id;
            data << graveyard->x;
            data << graveyard->y;
            data << graveyard->z;
            plr->SendPacketToSelf(&data);
        }
        return true;
    }
    return false;
}

/*
important notice:
buffs aren't spawned/despawned when players captures anything
buffs are in their positions when battleground starts
*/
void BattleGround::HandleTriggerBuff(uint64 const& go_guid)
{
    Map * map = GetMap();
    if (!map)
        return;

    GameObject *obj = map->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;

    //change buff type, when buff is used:
    int32 index = m_BgObjects.size() - 1;
    while (index >= 0 && m_BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround (Type: %u) has buff gameobject (Guid: %u Entry: %u Type:%u) but it hasn't that object in its internal data",GetTypeID(),GUID_LOPART(go_guid),obj->GetEntry(),obj->GetGoType());
        return;
    }

    //randomly select new buff
    uint8 buff = urand(0, 2);
    uint32 entry = obj->GetEntry();
    if (m_BuffChange && entry != Buff_Entries[buff])
    {
        //despawn current buff
        SpawnBGObject(index, RESPAWN_ONE_DAY);
        //set index for new one
        for (uint8 currBuffTypeIndex = 0; currBuffTypeIndex < 3; ++currBuffTypeIndex)
            if (entry == Buff_Entries[currBuffTypeIndex])
            {
                index -= currBuffTypeIndex;
                index += buff;
            }
    }

    SpawnBGObject(index, BUFF_RESPAWN_TIME);
}

void BattleGround::HandleKillPlayer(Player *player, Player *killer)
{
    //keep in mind that for arena this will have to be changed a bit

    // add +1 deaths
    UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    //Aura * dummy = player->GetDummyAura(55115); // Left for Dead
    //uint8 DummyStack = dummy ? dummy->GetStackAmount() : 0;

    // && DummyStack < 5
    if (killer && killer != player && !player->ToUnit()->WorthHonor)
    {
        //uint32 token_count = sWorld.getConfig(CONFIG_BOJ_FRAGMENTS_AFTER_KILL) * sWorld.getConfig(BONUS_RATES);

        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        //if (token_count > 0 && sWorld.isEasyRealm() && !isArena() && (killer->isAlive() || killer->HasAura(35571)))
        //    RewardItem(killer, BOJ_FRAGMENT, token_count);

        for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);

            if (!plr || plr == killer)
                continue;

            if (plr->GetBGTeam() == killer->GetBGTeam() && plr->IsAtGroupRewardDistance(player))
            {
                UpdatePlayerScore(plr, SCORE_HONORABLE_KILLS, 1);
                
                //if (token_count > 0 && sWorld.isEasyRealm() && !isArena() && (plr->isAlive() || plr->HasAura(35571)))
                //    RewardItem(plr, BOJ_FRAGMENT, token_count);
            }                
        }
    }

    // to be able to remove insignia -- ONLY IN BattleGrounds
    //if (!isArena()) 
    //{
    //    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    //    player->CastSpell(player, 35571, true);
    //}
}

// return the player's team based on battlegroundplayer info
// used in same faction arena matches mainly
PlayerTeam BattleGround::GetPlayerTeam(uint64 guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr!=m_Players.end())
        return itr->second.Team;
    return TEAM_NONE;
}

bool BattleGround::IsPlayerInBattleGround(uint64 guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr!=m_Players.end())
        return true;
    return false;
}

uint32 BattleGround::GetAlivePlayersCountByTeam(PlayerTeam Team) const
{
    int count = 0;
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.Team == Team)
        {
            Player * pl = sObjectMgr.GetPlayerInWorld(itr->first);
            if (pl && pl->isAlive() && !pl->IsSpectator() && !pl->HasByteFlag(UNIT_FIELD_BYTES_2, 3, FORM_SPIRITOFREDEMPTION))
                ++count;
        }
    }
    return count;
}

void BattleGround::SetBonus(bool is_bonus)
{
    if (is_bonus)
        m_HonorMode = BG_BONUS;
    else
        m_HonorMode = BG_NORMAL;
}

int32 BattleGround::GetObjectType(uint64 guid)
{
    for (uint32 i = 0;i <= m_BgObjects.size(); i++)
        if (m_BgObjects[i] == guid)
            return i;
    sLog.outLog(LOG_DEFAULT, "ERROR: BattleGround: cheating? a player used a gameobject which isnt supposed to be a usable object!");
    return -1;
}

void BattleGround::HandleKillUnit(Creature *creature, Player *killer)
{
}

// This method should be called when player logs out from running battleground
void BattleGround::EventPlayerLoggedOut(Player* player)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (isBattleGround())
            EventPlayerDroppedFlag(player);
    }

    if (isArena())
        RemovePlayerAtLeave(player->GetGUID(), true, true, true);
}

void BattleGround::SetBgRaid(PlayerTeam TeamID, Group *bg_raid )
{
    Group* &old_raid = TeamID == ALLIANCE ? m_BgRaids[TEAM_ALLIANCE] : m_BgRaids[TEAM_HORDE];

    if (old_raid)
        old_raid->SetBattlegroundGroup(NULL);

    if (bg_raid)
        bg_raid->SetBattlegroundGroup(this);

    old_raid = bg_raid;
}

void BattleGround::SetStatus(BattleGroundStatus Status)
{
    m_Status = Status;
    if (Status == STATUS_IN_PROGRESS)
    {
        m_progressStart = time(NULL);
        m_TimeElapsedSinceBeggining = 0;
    }
}

void BattleGround::SendObjectiveComplete(uint32 id, PlayerTeam TeamID/*, float x, float y*/)
{
    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayerInWorld(itr->first);
        if (!plr)
        {
            sLog.outDebug("BattleGround: Player %llu not found!", itr->first);
            continue;
        }
        PlayerTeam team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if (!team)
            team = plr->GetBGTeam();

        if (team == TeamID && plr->IsInWorld())
        {
            plr->KilledMonster(id, 0);
            // sLog.outLog(LOG_DEFAULT, "Battleground::SendObjectiveComplete: CreatureID: %u, PlayerGuid: %u, team: %u, TeamID: %u", id, plr->GetGUID(), team, TeamID);
        }
    }
}

uint8 BattleGround::SetPlayerReady(uint64 playerGUID)
{
    if (!isArena())
        return 1;

    if (sBattleGroundMgr.GetDebugArenaId())
    {
        SetStartDelayTime(1);
        return 0;
    }

    if (GetStatus() != STATUS_WAIT_JOIN)
        return 4;

    PlayerTeam team = GetPlayerTeam(playerGUID);
    if (team == TEAM_NONE)
        return 3;

    uint8 idx = team == ALLIANCE ? 0 : 1;
    if (m_guidsReady[idx].find(playerGUID) != m_guidsReady[idx].end())
        return 10;

    if (GetStartDelayTime() <= sWorld.getConfig(CONFIG_ARENA_READY_START_TIMER))
        return 10;

    m_guidsReady[idx].insert(playerGUID);
    
    uint32 readyCount = m_guidsReady[0].size() + m_guidsReady[1].size();
    if (readyCount == m_ArenaType * 2)
    {
        SetStartDelayTime(sWorld.getConfig(CONFIG_ARENA_READY_START_TIMER));
    }
    else if (m_guidsReady[idx].size() == m_ArenaType)
    {
        SendMessageToTeam(team == ALLIANCE ? HORDE : ALLIANCE, LANG_ARENA_EARLY_READY);
    }
    return 0;
}

uint8 BattleGround::GetSamePlayerKillCount(uint32 killer, uint32 victim)
{
    std::map<uint32, std::map<uint32, uint8>>::iterator itr = m_SamePlayerKillCount.find(killer);
    
    if (itr != m_SamePlayerKillCount.end())
    {
        std::map<uint32, uint8>::iterator itr2 = itr->second.find(victim);
        
        if (itr2 != itr->second.end())
            return itr2->second;
    }

    return 0;
}

void BattleGround::BGRealStats(Player* p)
{
    // show real team stats
    if (!isArena())
    {
        uint32 real_horde = 0;
        uint32 real_alliance = 0;
        std::string msg;

		for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            if (Player *plr = sObjectMgr.GetPlayerInWorld(itr->first))
            {
                bool ishorde = (plr->GetTeam() == HORDE) ? true : false;

                // invert
                if (GetPlayerTeam(plr->GetGUID()) == ALLIANCE && plr->GetTeam() == HORDE || GetPlayerTeam(plr->GetGUID()) == HORDE && plr->GetTeam() == ALLIANCE)
                    ishorde = !ishorde;

                if (ishorde)
                    ++real_horde;
                else
                    ++real_alliance;
            }
        }

		auto SendJoinInfo = [&](Player* plr)
		{
			ChatHandler(plr).SendSysMessage(11122);

			// After spending 10 minutes in BG, players receive the "Eye of Divinity" debuff...
			if (sWorld.getConfig(CONFIG_RATED_BG_ENABLED))
				ChatHandler(plr).PSendSysMessage(15656);

			ChatHandler(plr).PSendSysMessage(LANG_BG_REAL_STATS_TEAM, real_horde, real_alliance);
		};

        // send to all
        if (!p)
        {
			for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                if (Player *plr = sObjectMgr.GetPlayerInWorld(itr->first))
                {
                    //bool ishorde = (plr->GetTeam() == HORDE) ? true : false;
                    //if (GetPlayerTeam(plr->GetGUID()) == ALLIANCE && plr->GetTeam() == HORDE || GetPlayerTeam(plr->GetGUID()) == HORDE && plr->GetTeam() == ALLIANCE)
                    //    ishorde = !ishorde;

					SendJoinInfo(plr);
                }
            }
        }
        // send only to those who joined
        else
        {
            //bool ishorde = (p->GetTeam() == HORDE) ? true : false;
            //if (GetPlayerTeam(p->GetGUID()) == ALLIANCE && p->GetTeam() == HORDE || GetPlayerTeam(p->GetGUID()) == HORDE && p->GetTeam() == ALLIANCE)
            //    ishorde = !ishorde;

			SendJoinInfo(p);
        }
    }
}

void BattleGround::BGStarted()
{
    if (isArena())
    {
        SendMessageToAll(LANG_ARENA_BEGUN);
        DelCreature(0);
        DelCreature(1);
    }
    else
    {
        switch (GetTypeID())
        {
        case BATTLEGROUND_AV:
            SendMessageToAll(LANG_BG_AV_STARTED);
            //DelCreature(302);
            //DelCreature(303);
            break;
        case BATTLEGROUND_WS:
            SendMessageToAll(LANG_BG_WS_BEGIN);
            DelCreature(2);
            DelCreature(3);
            break;
        case BATTLEGROUND_AB:
            SendMessageToAll(LANG_BG_AB_STARTED);
            DelCreature(7);
            DelCreature(8);
            break;
        case BATTLEGROUND_EY:
            SendMessageToAll(LANG_BG_EY_BEGIN);
            DelCreature(6);
            DelCreature(7);
            break;
        }

        PlaySoundToAll(SOUND_BG_START);
        BGRealStats();
    }

    SetStatus(STATUS_IN_PROGRESS);

    for (BattleGroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
    {
        if (Player* plr = sObjectMgr.GetPlayerInWorld(itr->first))
        {
            if (isArena())
            {
                plr->RemoveAurasDueToSpell(SPELL_ARENA_PREPARATION);

                if (GetArenaType() == ARENA_TYPE_2v2 && isRated())
                {
                    std::string ip = plr->GetSession()->GetRemoteAddress().c_str();
                    time_t t = time(NULL) + 10 * MINUTE;
                    sWorld.m_ArenaPlayersIPs[ip] = t;
                }
            }
            else
                plr->RemoveAurasDueToSpell(SPELL_PREPARATION);
        }
    }
}

bool BattleGround::IsPlayerBGRated(uint64 guid)
{
	if (!m_RatedBG)
		return false;
	
	for (std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
	{
		if (itr->first == guid)
			return itr->second.Rated;
	}

	return false;
}

void BattleGround::AddRadiusNpc()
{
    AddCreature(690100, 2, TEAM_NONE, 1223.676636f, 1631.550415f, 321.875763f, 0.f);
    AddCreature(690100, 2, TEAM_NONE, 1233.958740f, 1624.452881f, 336.612549f, 0.f);
    AddCreature(690101, 2, TEAM_NONE, 1270.393311f, 1605.711548f, 312.390564f, 0.f);
    AddCreature(690101, 2, TEAM_NONE, 1266.195068f, 1611.077881f, 323.876831f, 0.f);
    AddCreature(690101, 2, TEAM_NONE, 1248.344116f, 1622.360229f, 335.921875f, 0.f);
    // BG_CREATURES_MAX_WS
}

std::string BattleGround::GetPlayerCountInfo()
{
    std::string allianceCount = std::to_string(GetPlayersCountByTeam(ALLIANCE));
    std::string hordeCount = std::to_string(GetPlayersCountByTeam(HORDE));

    return "in A/H " + allianceCount + "/" + hordeCount;
}

PlayerTeam BattleGround::GetBestBGTeamOnDistribute(GroupQueueInfo* gqi, bool only_check)
{
    // A: dru dru
    // H: rog dru rog
    // +dru -> A (wait, class majority)
    // +rog -> A (dru H, rog A)
    // A: dru dru rog
    // H: rog dru rog dru
    // +rog -> A (less players)

    uint32 group_size = gqi->Players.size();

    uint32 hordeFreeOverLimit = GetFreeBGSlotsForTeam(HORDE, true);
    uint32 allianceFreeOverLimit = GetFreeBGSlotsForTeam(ALLIANCE, true);

    if (hordeFreeOverLimit > 0)
    {
        if (group_size > 1)
            return TEAM_NONE;
        
        return HORDE;
    }
    else if (allianceFreeOverLimit > 0)
    {
        if (group_size > 1)
            return TEAM_NONE;
        
        return ALLIANCE;
    }

    // premade members are always joining to the same team, regardless slots
    // note: when member joining he leaves from gqi->Players
    if (gqi->CurrentBGId == GetBgInstanceId())
    {
        sLog.outLog(LOG_BG, "ID %u: --- check #2, only_check %u", GetBgInstanceId(), int(only_check));
        return gqi->Team;
    }

    uint32 hordeFree = GetFreeBGSlotsForTeam(HORDE);
    uint32 allianceFree = GetFreeBGSlotsForTeam(ALLIANCE);

    if (hordeFree + allianceFree == 0)
    {
        sLog.outLog(LOG_BG, "ID %u: --- check #1, only_check %u", GetBgInstanceId(), int(only_check));
        return TEAM_NONE;
    }

    auto HaveClassMajority = [&](PlayerTeam team, uint32 class_mask, bool only_healer)
    {
        if (only_healer && !(class_mask & CLASS_HEALER_MASK))
            return false;

        TeamId index = GetTeamIndexByTeamId(team);

        if (teamClassMaskCount[index][class_mask] > teamClassMaskCount[GetOtherTeam(index)][class_mask])
            return true;

        return false;
    };

    // premade is always going to less-populated team ignoring class balance
    if (group_size > 1)
    {
        // designed for BG.MaxPremadeCount = 2
        // 9-9 WSG -> 2 free slots, premade can't join
        if (hordeFree < group_size && allianceFree < group_size)
            return TEAM_NONE;

        // 8-8 random
        if (hordeFree == allianceFree)
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #3, only_check %u", GetBgInstanceId(), int(only_check));
            return urand(0, 1) ? ALLIANCE : HORDE;
        }
        // 8-6 WSG, join team with less players
        else if (hordeFree > allianceFree)
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #4, only_check %u", GetBgInstanceId(), int(only_check));
            return HORDE;
        }
        else if (hordeFree < allianceFree)
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #5, only_check %u", GetBgInstanceId(), int(only_check));
            return ALLIANCE;
        }
    }

    // always join team with less players if there are no free slots for the other team
    if (hordeFree == 0)
    {
        sLog.outLog(LOG_BG, "ID %u: --- check #6, only_check %u", GetBgInstanceId(), int(only_check));
        return ALLIANCE;
    }
    else if (allianceFree == 0)
    {
        sLog.outLog(LOG_BG, "ID %u: --- check #7, only_check %u", GetBgInstanceId(), int(only_check));
        return HORDE;
    }
    uint32 diff = std::abs((int32)hordeFree - (int32)allianceFree);

    // A: heal
    // H: rog rog
    // +heal -> wait
    if (diff >= 1 && gqi->LeaderClassMask == CLASS_HEALER_MASK)
    {
        if (allianceFree > hordeFree && !HaveClassMajority(ALLIANCE, gqi->LeaderClassMask, true))
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #8, only_check %u", GetBgInstanceId(), int(only_check));
            return ALLIANCE;
        }
        else if (allianceFree < hordeFree && !HaveClassMajority(HORDE, gqi->LeaderClassMask, true))
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #9, only_check %u", GetBgInstanceId(), int(only_check));
            return HORDE;
        }
        else
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #10, only_check %u", GetBgInstanceId(), int(only_check));
            return TEAM_NONE;
        }
    }

    // A: heal
    // H: rog
    // +heal -> H
    if (hordeFree == allianceFree)
    {
        if (HaveClassMajority(HORDE, gqi->LeaderClassMask, false))
        { 
            sLog.outLog(LOG_BG, "ID %u: --- check #11, only_check %u", GetBgInstanceId(), int(only_check));
            return ALLIANCE;
        }
        else if (HaveClassMajority(ALLIANCE, gqi->LeaderClassMask, false))
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #12, only_check %u", GetBgInstanceId(), int(only_check));
            return HORDE;
        }
        else
        {
            sLog.outLog(LOG_BG, "ID %u: --- check #13, only_check %u", GetBgInstanceId(), int(only_check));
            return urand(0, 1) ? ALLIANCE : HORDE;
        }
    }
    else if (hordeFree < allianceFree)
    {
        sLog.outLog(LOG_BG, "ID %u: --- check #14, only_check %u", GetBgInstanceId(), int(only_check));
        return ALLIANCE;
    }
    else if (hordeFree > allianceFree)
    {
        sLog.outLog(LOG_BG, "ID %u: --- check #15, only_check %u", GetBgInstanceId(), int(only_check));
        return HORDE;
    }

    sLog.outLog(LOG_BG, "ID %u: --- check #16, only_check %u", GetBgInstanceId(), int(only_check));
    return urand(0, 1) ? ALLIANCE : HORDE;
}

PlayerTeam BattleGround::GetWinningTeam()
{
    uint32 score_horde = GetTeamScore(HORDE);
    uint32 score_alliance = GetTeamScore(ALLIANCE);

    if (score_horde > score_alliance)
        return HORDE;
    else if (score_horde < score_alliance)
        return ALLIANCE;

    uint32 horde_kills = 0;
    uint32 alliance_kills = 0;

    for (auto& itr : m_Players)
    {
        BattleGroundPlayer bplayer = itr.second;
        auto scoreItr = m_PlayerScores.find(itr.first);
        if (scoreItr != m_PlayerScores.end())
        {
            if (bplayer.Team == HORDE)
                horde_kills += scoreItr->second->KillingBlows;
            else
                alliance_kills += scoreItr->second->KillingBlows;
        }
    }

    if (horde_kills > alliance_kills)
        return HORDE;
    else if (horde_kills < alliance_kills)
        return ALLIANCE;

    return urand(0, 1) ? HORDE : ALLIANCE;
}

uint32 BattleGround::GetTeamScore(PlayerTeam team) const
{
    sLog.outLog(LOG_CRITICAL, "Should never be called");
    return 0;
}