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

#include <algorithm>

#include "InstanceData.h"
#include "Database/DatabaseEnv.h"
#include "Map.h"
#include "GameObject.h"
#include "Creature.h"
#include "MapRefManager.h"
#include "MapReference.h"
#include "Language.h"
#include "Player.h"
#include "GuildMgr.h"
#include "Chat.h"
#include "GameEvent.h"
#include "ObjectMgr.h"

void InstanceData::SaveToDB()
{
    std::string data = GetSaveData();
    if (data.empty())
        return;

    static SqlStatementID updateInstance;

    SqlStatement stmt = RealmDataDatabase.CreateStatement(updateInstance, "UPDATE instance SET data = ? WHERE id = ?");
    stmt.addString(data);
    stmt.addUInt32(instance->GetInstanciableInstanceId());
    stmt.Execute();
}

void InstanceData::HandleGameObject(uint64 GUID, bool open, GameObject *go)
{
    if (!go)
        go = instance->GetGameObject(GUID);

    if (go)
        go->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
    else
        debug_log("TSCR: InstanceData: HandleGameObject failed");
}

bool InstanceData::IsEncounterInProgress() const
{
    for (std::vector<BossInfo>::const_iterator itr = bosses.begin(); itr != bosses.end(); ++itr)
        if (itr->state == IN_PROGRESS)
            return true;

    return false;
}

//This will be removed in the future, just compitiable with Mangos
void InstanceData::OnCreatureCreate(Creature *creature, bool add)
{
   OnCreatureCreate(creature, creature->GetEntry());
}

void InstanceData::LoadDoorData(const DoorData *data)
{
    while (data->entry)
    {
        if (data->bossId < bosses.size())
            doors.insert(std::make_pair(data->entry, DoorInfo(&bosses[data->bossId], data->type)));

        ++data;
    }
    sLog.outDebug("InstanceData::LoadDoorData: %llu doors loaded.", doors.size());
}

void InstanceData::UpdateDoorState(GameObject *door)
{
    DoorInfoMap::iterator lower = doors.lower_bound(door->GetEntry());
    DoorInfoMap::iterator upper = doors.upper_bound(door->GetEntry());
    if (lower == upper)
        return;

    bool open = true;
    for (DoorInfoMap::iterator itr = lower; itr != upper; ++itr)
    {
        if (itr->second.type == DOOR_TYPE_ROOM)
        {
            if (itr->second.bossInfo->state == IN_PROGRESS)
            {
                open = false;
                break;
            }
        }
        else if (itr->second.type == DOOR_TYPE_PASSAGE)
        {
            if (itr->second.bossInfo->state != DONE)
            {
                open = false;
                break;
            }
        }
    }

    door->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
}

void InstanceData::AddDoor(GameObject *door, bool add)
{
    DoorInfoMap::iterator lower = doors.lower_bound(door->GetEntry());
    DoorInfoMap::iterator upper = doors.upper_bound(door->GetEntry());
    if (lower == upper)
        return;

    for (DoorInfoMap::iterator itr = lower; itr != upper; ++itr)
    {
        if (add)
            itr->second.bossInfo->door[itr->second.type].insert(door);
        else
            itr->second.bossInfo->door[itr->second.type].erase(door);
    }

    if (add)
        UpdateDoorState(door);
}

bool InstanceData::SetBossState(uint32 id, EncounterState state)
{
    if (id < bosses.size())
    {
        BossInfo *bossInfo = &bosses[id];
        if (bossInfo->state == TO_BE_DECIDED) // loading
        {
            bossInfo->state = state;
            return false;
        }
        else
        {
            if (bossInfo->state == state)
                return false;
            bossInfo->state = state;
            SaveToDB();
        }

        for (uint32 type = 0; type < MAX_DOOR_TYPES; ++type)
            for (DoorSet::iterator i = bossInfo->door[type].begin(); i != bossInfo->door[type].end(); ++i)
                UpdateDoorState(*i);

        return true;
    }
    return false;
}

std::string InstanceData::LoadBossState(const char * data)
{
    if (!data) return NULL;
    std::istringstream loadStream(data);
    uint32 buff;
    uint32 bossId = 0;
    for (std::vector<BossInfo>::iterator i = bosses.begin(); i != bosses.end(); ++i, ++bossId)
    {
        loadStream >> buff;
        if (buff < TO_BE_DECIDED)
            SetBossState(bossId, (EncounterState)buff);
    }
    return loadStream.str();
}

std::string InstanceData::GetBossSaveData()
{
    std::ostringstream saveStream;
    for (std::vector<BossInfo>::iterator i = bosses.begin(); i != bosses.end(); ++i)
        saveStream << (uint32)i->state << " ";
    return saveStream.str();
}

void InstanceData::ResetEncounterInProgress()
{
    // this will only reset encounter state to NOT_STARTED, it won't evade creatures inside of map, better option is to override it in instance script
    Load(GetSaveData().c_str());
}

/* used with creature_linked_respawn table to make creatures non-attackable until certain event is done.
may also be used to make script-special changes or sets them dead, so don't use it to make bosses non-attackable!*/
void InstanceData::HandleInitCreatureState(Creature * mob)
{
    if (mob == nullptr)
        return;

    const CreatureData *tmp = mob->GetLinkedRespawnCreatureData();
    if (!tmp)
        return;

    uint32 encounter = GetEncounterForEntry(tmp->id);

    if (encounter && mob->isAlive() && GetData(encounter) == DONE)
    {
        mob->setDeathState(JUST_DIED);
        mob->RemoveCorpse();
        return;
    }

    encounter = GetRequiredEncounterForEntry(tmp->id);

    if (encounter && mob->isAlive() && GetData(encounter) != DONE)
    {
        mob->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        requiredEncounterToMobs[encounter].push_back(mob->GetGUID());
    }
}

/* used with creature_linked_respawn table to make creatures non-attackable until certain event is done.
may also be used to make script-special changes or sets them dead, so don't use it to make bosses non-attackable!*/
void InstanceData::HandleRequiredEncounter(uint32 encounter)
{
    if (GetData(encounter) != DONE)
        return;

    auto itr = requiredEncounterToMobs.find(encounter);
    if (itr != requiredEncounterToMobs.end())
    {
        std::vector<uint64> & tmpVec = itr->second;

        std::for_each(tmpVec.begin(), tmpVec.end(), [this] (uint64& var)
                                                    {
                                                        Creature * tmp = GetCreature(var);
                                                        if (tmp != nullptr)
                                                            tmp->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                                                    });
    }
}

void InstanceData::LogPossibleCheaters(const char* cheatName)
{
    std::string playerlist="";
    Map::PlayerList players = instance->GetPlayers();
    if (Player* pPlayer = players.getFirst()->getSource())
    {
        for (MapRefManager::iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            playerlist += itr->getSource()->GetName();
            playerlist += " ";
        }
        sLog.outLog(LOG_CHEAT,"Possible cheaters(%s): %s, instance_id %u",cheatName,playerlist.c_str(), instance->GetInstanciableInstanceId());
    
        sWorld.SendGMText(LANG_POSSIBLE_CHEAT, cheatName, pPlayer->GetName(),playerlist.c_str());
    }
}

//bool GBK_handler::FirstBoss(GBK_Encounters encounter)
//{
//	switch (encounter)
//	{
//	case GBK_MAGTHERIDON:
//	case GBK_HIGH_KING_MAULGAR:
//	case GBK_KARA_ATTUNEMENT:
//	case GBK_HYDROSS_THE_UNSTABLE:
//	case GBK_ALAR:
//	case GBK_RAGE_WINTERCHILL:
//	case GBK_HIGH_WARLORD_NAJENTUS:
//	case GBK_KALECGOS:
//	case GBK_ZA_AKILZON:
//		return true;
//	}
//
//	return false;
//}

bool GBK_handler::LastBoss(GBK_Encounters encounter)
{
	switch (encounter)
	{
	case GBK_MAGTHERIDON:
	case GBK_GRUUL:
	case GBK_KARA_MALCHEZAR:
	case GBK_LADY_VASHJ:
	case GBK_KAELTHAS_SUNSTRIDER:
	case GBK_ARCHIMONDE:
	case GBK_ILLIDAN_STORMRAGE:
	case GBK_KILJAEDEN:
	case GBK_ZA_ZULJIN:
	case GBK_ONYXIA:
	case GBK_KEL_THUZAD:
		return true;
	}

	return false;
}

void GBK_handler::StartCombat(GBK_Encounters encounter)
{
    // dungeons disabled
    if (encounter >= GBK_HR_GARGOLMAR && encounter <= GBK_OHF_LEUTENANT)
        return;

    DebugLog("StartCombat START", encounter);

    if (m_encounter != GBK_NONE && m_encounter != encounter)
    {
        if (m_encounter != GBK_ANTISPAMINLOGSINATOR)
        {
            DebugLog("StartCombat ERROR while already in combat", encounter);
            m_encounter = GBK_ANTISPAMINLOGSINATOR;
        }

        DebugLog("StartCombat return", encounter);
        return;
    }
    else if (m_encounter == encounter)
    {
        DebugLog("StartCombat ERROR (NO) m_encounter == encounter", encounter);
        return; // combat in progress anyway, just dance
    }

    if (encounter != m_last_encounter) // need to check m_last_encounter, cause m_encounter is set to 0 on stopCombat()
    {
        QueryResultAutoPtr result = RealmDataDatabase.PQuery(
            "SELECT `evades` FROM `boss_fights_evades` WHERE `instance_id`='%u' AND `boss_id`='%u' LIMIT 1",
            m_map->GetInstanciableInstanceId(), encounter);
        if (result)
            m_evade_count = (*result)[0].GetUInt32();
        else
            m_evade_count = 0;

        m_last_encounter = encounter; // m_last_encounter is not getting reset on stopCombat()
    }

    m_encounter = encounter;
    m_timer = WorldTimer::getMSTime();
    DebugLog("StartCombat END", encounter);
}

void GBK_handler::StopCombat(GBK_Encounters encounter, bool win)
{
    // dungeons disabled
    if (encounter >= GBK_HR_GARGOLMAR && encounter <= GBK_OHF_LEUTENANT)
        return;
    
    DebugLog("StopCombat START", encounter);

	if (m_encounter == GBK_ANTISPAMINLOGSINATOR)
	{
        DebugLog("StopCombat return", encounter);
		return;
	}

	if (!m_map->GetInstanciableInstanceId())
	{
        DebugLog("StopCombat ERROR null instance id", encounter);
		return;
	}

    if (win)
    {
        if (m_encounter == GBK_NONE || m_encounter != encounter)
        {
            DebugLog("StopCombat ERROR m_encounter", encounter);
            m_timer = 0;
            stats.clear();
            m_encounter = GBK_ANTISPAMINLOGSINATOR;
            return;
        }

        // need to recalculate raid-guild-id, can't just take it from instance, cause raid-group could have changed
        uint32 leaderLowGuid = 0; // initialized via Calculate_GBD_Guild()
        uint32 guild_id = m_map->IsDungeon() ? ((InstanceMap*)m_map)->Calculate_GBK_Guild(&leaderLowGuid) : 0;
        uint32 kill_id = sGuildMgr.BossKilled(encounter, guild_id, WorldTimer::getMSTimeDiffToNow(m_timer));
        uint32 kill_time = WorldTimer::getMSTimeDiffToNow(m_timer);
        std::string boss_name = sGuildMgr.GBK_GetBossNameForEvent(uint32(m_encounter));

        std::multimap<uint32, RaidPrintStats, std::greater<uint32>> raid_stats_set;
        uint32 damage_total = 0;
        uint32 healed_total = 0;

        Map::PlayerList const& list = m_map->GetPlayers();
        for (Map::PlayerList::const_iterator i = list.begin(); i != list.end(); ++i)
        {
            if (Player* plr = i->getSource())
            {
                if (plr->isGameMaster() && stats[plr->GetGUIDLow()].damage == 0 && stats[plr->GetGUIDLow()].healing == 0)
                    continue;

                uint32 value = 0;
                bool is_healer = false;

                if (stats[plr->GetGUIDLow()].damage == 0 && stats[plr->GetGUIDLow()].healing == 0 || stats[plr->GetGUIDLow()].healing > stats[plr->GetGUIDLow()].damage)
                {
                    value = stats[plr->GetGUIDLow()].healing;
                    is_healer = true;
                    healed_total += value;
                }
                else
                {
                    value = stats[plr->GetGUIDLow()].damage;
                    is_healer = false;
                    damage_total += value;
                }

                RaidPrintStats rps;
                rps.name = plr->GetName();
                rps.is_healer = is_healer;

                raid_stats_set.insert({ value, rps });

                ChatHandler(plr).PSendSysMessage(LANG_GBK_RAID_MSG, boss_name.c_str(), plr->GetSession()->secondsToTimeString(kill_time / 1000, true).c_str());
                //ChatHandler(plr).PSendSysMessage(LANG_GBK_PERSONAL_MSG, 
                //    stats[plr->GetGUIDLow()].damage, stats[plr->GetGUIDLow()].healing, stats[plr->GetGUIDLow()].deaths);
            }
        }

        if (!raid_stats_set.empty() && raid_stats_set.size() > 1)
        {
            std::stringstream ss_ru;
            std::stringstream ss_en;

            uint32 healer_pos = 1;
            uint32 damager_pos = 1;

            const char* strings[4] = {
                sObjectMgr.GetHellgroundString(16648, 0), // ru
                sObjectMgr.GetHellgroundString(16648, 1), // en
                sObjectMgr.GetHellgroundString(16649, 0),
                sObjectMgr.GetHellgroundString(16649, 1)
            };

            for (int x = 0; x < 2; x++)
            {
                bool header_printed = false;

                for (const auto& rs : raid_stats_set) {
                    // dps first
                    if ((x == 0 && rs.second.is_healer) || (x == 1 && !rs.second.is_healer))
                        continue;

                    if (!header_printed) {
                        ss_ru << strings[rs.second.is_healer ? 2 : 0] << "\n";
                        ss_en << strings[rs.second.is_healer ? 3 : 1] << "\n";
                        header_printed = true;
                    }

                    uint32 pos = (rs.second.is_healer) ? healer_pos++ : damager_pos++;
                    uint32 percentage = std::round(float(rs.first) / ((rs.second.is_healer) ? healed_total : damage_total) * 100);

                    std::stringstream formatted;
                    formatted << "|cfffcfcfc" << pos << ") " << rs.second.name << " - " << rs.first << " (" << percentage << "%)|r\n";

                    // Запись в ss_ru и ss_en
                    ss_ru << formatted.str();
                    ss_en << formatted.str();
                }
            }

            for (Map::PlayerList::const_iterator i = list.begin(); i != list.end(); ++i)
            {
                if (Player* plr = i->getSource())
                {
                    if (WorldSession* sess = plr->GetSession())
                    {
                        if (sess->IsAccountFlagged(ACC_DISABLED_RAIDSTATS))
                            continue;

                        ChatHandler(plr).SendSysMessage(sess->isRussian() ? ss_ru.str().c_str() : ss_en.str().c_str());                  
                        ChatHandler(plr).SendSysMessage(16650);
                    }
                }
            }
        }

        RealmDataDatabase.BeginTransaction();

        static SqlStatementID bossFightsInsert;
        static SqlStatementID bossFightsDetailedInsert;
        static SqlStatementID bossFightsEvadesDelete;

        SqlStatement stmt = RealmDataDatabase.CreateStatement(bossFightsInsert, "INSERT INTO boss_fights VALUES (?,?,?,?,?,?,?,NOW())");
        stmt.addUInt32(kill_id);
        stmt.addUInt32(uint32(m_encounter));
        stmt.addUInt32(m_map->GetInstanciableInstanceId());
        stmt.addUInt32(leaderLowGuid);
        stmt.addUInt32(guild_id);
        stmt.addUInt32(kill_time);
        stmt.addUInt32(m_evade_count);
        stmt.Execute();

        for (Map::PlayerList::const_iterator i = list.begin(); i != list.end(); ++i)
        {
            if (Player* plr = i->getSource())
            {
                if (plr->isGameMaster() && stats[plr->GetGUIDLow()].damage == 0 && stats[plr->GetGUIDLow()].healing == 0)
                    continue;

                stmt = RealmDataDatabase.CreateStatement(bossFightsDetailedInsert, "INSERT INTO boss_fights_detailed VALUES (?,?,?,?,?)");
                stmt.addUInt32(kill_id);
                stmt.addUInt32(plr->GetGUIDLow());
                stmt.addUInt32(stats[plr->GetGUIDLow()].damage);
                stmt.addUInt32(stats[plr->GetGUIDLow()].healing);
                stmt.addUInt32(stats[plr->GetGUIDLow()].deaths);
                stmt.Execute();
            }
        }

        if (m_evade_count)
        {
            stmt = RealmDataDatabase.CreateStatement(bossFightsEvadesDelete, "DELETE FROM boss_fights_evades WHERE instance_id=? AND boss_id=?");
            stmt.addUInt32(m_map->GetInstanciableInstanceId());
            stmt.addUInt32(uint32(m_encounter));
            stmt.Execute();
            m_evade_count = 0;
        }

		if (LastBoss(encounter))
		{
			static SqlStatementID durationUpdate;
			SqlStatement stmt1 = RealmDataDatabase.CreateStatement(durationUpdate, "UPDATE boss_fights_instance SET end_time = unix_timestamp() WHERE instance_id = ?");
			stmt1.addUInt32(m_map->GetInstanciableInstanceId());
			stmt1.Execute();

			static SqlStatementID updateBossFightsInstance;
			SqlStatement stmt2 = RealmDataDatabase.CreateStatement(updateBossFightsInstance, "UPDATE boss_fights_instance SET guild_id = ? WHERE instance_id = ?");
			stmt2.addUInt32(guild_id);
			stmt2.addUInt32(m_map->GetInstanciableInstanceId());
			stmt2.Execute();
		}

        RealmDataDatabase.CommitTransaction();
    }

    if (m_encounter == encounter) //do not reset timers when some boss is just spamming not_started
    { 
        // NOT winning, EVADING!
        if (!win) 
        {
            // ressurect dead players
            RessurectAfterEvade();

            // if the battle was longer than 15 sec.
            if (m_timer > 30000)
            {
                ++m_evade_count;
                if (m_evade_count == 1) // need to insert row
                {
                    static SqlStatementID insertBossFightsEvade;

                    SqlStatement stmt = RealmDataDatabase.CreateStatement(insertBossFightsEvade, "INSERT INTO boss_fights_evades VALUES (?,?,1)");
                    stmt.addUInt32(m_map->GetInstanciableInstanceId());
                    stmt.addUInt32(uint32(m_encounter));
                    stmt.Execute();
                }
                else // need to update old row
                {
                    static SqlStatementID updateBossFightsEvade;

                    SqlStatement stmt = RealmDataDatabase.CreateStatement(updateBossFightsEvade,
                        "UPDATE boss_fights_evades SET evades = ? WHERE instance_id = ? AND boss_id = ?");
                    stmt.addUInt32(m_evade_count);
                    stmt.addUInt32(m_map->GetInstanciableInstanceId());
                    stmt.addUInt32(uint32(m_encounter));
                    stmt.Execute();
                }
            }
        }

        m_encounter = GBK_NONE;
        m_timer = 0;
        stats.clear();
    }

    DebugLog("StopCombat END", encounter);
}

void GBK_handler::RessurectAfterEvade()
{
    auto it = sWorld.encounter_ressurect_pos.find(m_encounter);
    if (it == sWorld.encounter_ressurect_pos.end())
        return;

    const std::vector<float>& resurrectionPos = it->second;

    Map::PlayerList const& PlayerList = ((InstanceMap*)m_map)->GetPlayers();
    for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
    {
        Player* plr = i->getSource();
        if (plr && !plr->isAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->TeleportTo(m_map->GetId(), resurrectionPos[0], resurrectionPos[1], resurrectionPos[2], 0.f);
        }
    }
}

void GBK_handler::DebugLog(std::string msg, GBK_Encounters encounter)
{
    std::string log = "GBKDebugLog: " + msg + " - encounter %u, m_encounter %u, m_map %u, instance %u";
    sLog.outLog(LOG_SPECIAL, log.c_str(), uint32(encounter), uint32(m_encounter), m_map->GetId(), m_map->GetInstanciableInstanceId());
}