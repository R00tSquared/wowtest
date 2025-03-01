// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "ReputationMgr.h"
#include "DBCStores.h"
#include "Player.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"

const int32 ReputationMgr::PointsInRank[MAX_REPUTATION_RANK] = {36000, 3000, 3000, 3000, 6000, 12000, 21000, 1000};

ReputationRank ReputationMgr::ReputationToRank(int32 standing)
{
    int32 limit = Reputation_Cap + 1;
    for (int i = MAX_REPUTATION_RANK-1; i >= MIN_REPUTATION_RANK; --i)
    {
        limit -= PointsInRank[i];
        if (standing >= limit)
            return ReputationRank(i);
    }
    return MIN_REPUTATION_RANK;
}

int32 ReputationMgr::GetReputation(uint32 faction_id) const
{
    FactionEntry const *factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ReputationMgr::GetReputation: Can't get reputation of %s for unknown faction (faction id) #%u.",m_player->GetName(), faction_id);
        return 0;
    }

    return GetReputation(factionEntry);
}

int32 ReputationMgr::GetBaseReputation(FactionEntry const* factionEntry) const
{
    if (!factionEntry)
        return 0;

    uint32 raceMask = m_player->GetRaceMask();
    uint32 classMask = m_player->GetClassMask();

    int idx = factionEntry->GetIndexFitTo(raceMask, classMask);

    return idx >= 0 ? factionEntry->BaseRepValue[idx] : 0;
}

int32 ReputationMgr::GetReputation(FactionEntry const* factionEntry) const
{
    // Faction without recorded reputation. Just ignore.
    if (!factionEntry)
        return 0;

    if (FactionState const* state = GetState(factionEntry))
        return GetBaseReputation(factionEntry) + state->Standing;

    return 0;
}

ReputationRank ReputationMgr::GetRank(FactionEntry const* factionEntry) const
{
    int32 reputation = GetReputation(factionEntry);
    return ReputationToRank(reputation);
}

ReputationRank ReputationMgr::GetRank(uint32 faction_id) const
{
    FactionEntry const *factionEntry = sFactionStore.LookupEntry(faction_id);
    return GetRank(factionEntry);
}

ReputationRank ReputationMgr::GetBaseRank(FactionEntry const* factionEntry) const
{
    int32 reputation = GetBaseReputation(factionEntry);
    return ReputationToRank(reputation);
}

void ReputationMgr::ApplyForceReaction( uint32 faction_id,ReputationRank rank,bool apply )
{
    if (apply)
        m_forcedReactions[faction_id] = rank;
    else
        m_forcedReactions.erase(faction_id);
}

uint32 ReputationMgr::GetDefaultStateFlags(FactionEntry const* factionEntry) const
{
    if (!factionEntry)
        return 0;

    uint32 raceMask = m_player->GetRaceMask();
    uint32 classMask = m_player->GetClassMask();

    int idx = factionEntry->GetIndexFitTo(raceMask, classMask);

    return idx >= 0 ? factionEntry->ReputationFlags[idx] : 0;
}

void ReputationMgr::SendForceReactions()
{
    WorldPacket data;
    data.Initialize(SMSG_SET_FORCED_REACTIONS, 4+m_forcedReactions.size()*(4+4));
    data << uint32(m_forcedReactions.size());
    for (ForcedReactions::const_iterator itr = m_forcedReactions.begin(); itr != m_forcedReactions.end(); ++itr)
    {
        data << uint32(itr->first);                         // faction_id (Faction.dbc)
        data << uint32(itr->second);                        // reputation rank
    }
    m_player->SendPacketToSelf(&data);
}

void ReputationMgr::SendState(FactionState const* faction)
{
    uint32 count = 1;

    WorldPacket data(SMSG_SET_FACTION_STANDING, (16));      // last check 2.4.0
    data << float(0);                                      // unk 2.4.0

    size_t p_count = data.wpos();
    data << uint32(count);                                 // placeholder

    data << uint32(faction->ReputationListID);
    data << uint32(faction->Standing);

    for (FactionStateList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        if (itr->second.needSend)
        {
            itr->second.needSend = false;
            if (itr->second.ReputationListID != faction->ReputationListID)
            {
                data << uint32(itr->second.ReputationListID);
                data << uint32(itr->second.Standing);
                ++count;
            }
        }
    }

    data.put<uint32>(p_count, count);
    m_player->SendPacketToSelf(&data);
}

void ReputationMgr::SendInitialReputations()
{
    WorldPacket data(SMSG_INITIALIZE_FACTIONS, (4+128*5));
    data << uint32(0x00000080);

    RepListID a = 0;

    for (FactionStateList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        // fill in absent fields
        for (; a != itr->first; a++)
        {
            data << uint8(0x00);
            data << uint32(0x00000000);
        }

        // fill in encountered data
        data << uint8(itr->second.Flags);
        data << uint32(itr->second.Standing);

        itr->second.needSend = false;

        ++a;
    }

    // fill in absent fields
    for (; a != 128; a++)
    {
        data << uint8(0x00);
        data << uint32(0x00000000);
    }

    m_player->SendPacketToSelf(&data);
}

void ReputationMgr::SendVisible(FactionState const* faction) const
{
    if (m_player->GetSession()->PlayerLoading())
        return;

    // make faction visible in reputation list at client
    WorldPacket data(SMSG_SET_FACTION_VISIBLE, 4);
    data << faction->ReputationListID;
    m_player->SendPacketToSelf(&data);
}

void ReputationMgr::Initialize()
{
    m_factions.clear();

    for (unsigned int i = 1; i < sFactionStore.GetNumRows(); i++)
    {
        FactionEntry const *factionEntry = sFactionStore.LookupEntry(i);

        if (factionEntry && (factionEntry->reputationListID >= 0))
        {
            FactionState newFaction;
            newFaction.ID = factionEntry->ID;
            newFaction.ReputationListID = factionEntry->reputationListID;
            newFaction.Standing = 0;
            newFaction.Flags = GetDefaultStateFlags(factionEntry);
            newFaction.needSend = true;
            newFaction.needSave = true;

            m_factions[newFaction.ReputationListID] = newFaction;
        }
    }
}

bool ReputationMgr::SetReputation(FactionEntry const* factionEntry, int32 standing, bool incremental)
{
    bool res = false;
    // if spillover definition exists in DB
    if (const RepSpilloverTemplate *repTemplate = sObjectMgr.GetRepSpilloverTemplate(factionEntry->ID))
    {
        for (uint32 i = 0; i < MAX_SPILLOVER_FACTIONS; ++i)
        {
            if (repTemplate->faction[i])
            {
                if (GetRank(repTemplate->faction[i]) <= ReputationRank(repTemplate->faction_rank[i]))
                {
                    // bonuses are already given, so just modify standing by rate
                    int32 spilloverRep = standing * repTemplate->faction_rate[i];
                    SetOneFactionReputation(sFactionStore.LookupEntry(repTemplate->faction[i]), spilloverRep, incremental);
                }
            }
        }
    }

    // spillover done, update faction itself
    FactionStateList::iterator faction = m_factions.find(factionEntry->reputationListID);
    if (faction != m_factions.end())
    {
        res = SetOneFactionReputation(factionEntry, standing, incremental);
        // only this faction gets reported to client, even if it has no own visible standing
        SendState(&faction->second);
    }

    return res;
}

bool ReputationMgr::SetOneFactionReputation(FactionEntry const* factionEntry, int32 standing, bool incremental)
{
    FactionStateList::iterator itr = m_factions.find(factionEntry->reputationListID);
    if (itr != m_factions.end())
    {
        int32 BaseRep = GetBaseReputation(factionEntry);

        if (incremental)
            standing += itr->second.Standing + BaseRep;

        if (standing > Reputation_Cap)
            standing = Reputation_Cap;
        else if (standing < Reputation_Bottom)
            standing = Reputation_Bottom;

        itr->second.Standing = standing - BaseRep;
        itr->second.needSend = true;
        itr->second.needSave = true;

        SetVisible(&itr->second);

        if (ReputationToRank(standing) <= REP_HOSTILE)
            SetAtWar(&itr->second, true);

        m_player->ReputationChanged(factionEntry);
        return true;
    }
    return false;
}

bool ReputationMgr::SetReputation(uint32 factionId, int32 standing)
{
    FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionId);

    if (!factionEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ReputationMgr::SetReputation: Can't get reputation of %s for unknown faction (faction id) #%u.", m_player->GetName(), factionId);
        return false;
    }

    return SetReputation(factionEntry, standing, false);
}

bool ReputationMgr::ModifyReputation(uint32 factionId, int32 standing)
{
    FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionId);

    if (!factionEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ReputationMgr::SetReputation: Can't get reputation of %s for unknown faction (faction id) #%u.", m_player->GetName(), factionId);
        return false;
    }

    return SetReputation(factionEntry, standing, true);
}

void ReputationMgr::SetVisible(FactionTemplateEntry const*factionTemplateEntry)
{
    if (!factionTemplateEntry->faction)
        return;

    if (FactionEntry const *factionEntry = sFactionStore.LookupEntry(factionTemplateEntry->faction))
        SetVisible(factionEntry);
}

void ReputationMgr::SetVisible(FactionEntry const *factionEntry)
{
    if (factionEntry->reputationListID < 0)
        return;

    FactionStateList::iterator itr = m_factions.find(factionEntry->reputationListID);
    if (itr == m_factions.end())
        return;

    SetVisible(&itr->second);
}

void ReputationMgr::SetVisible(FactionState* faction)
{
    // always invisible or hidden faction can't be make visible
    if (faction->Flags & (FACTION_FLAG_INVISIBLE_FORCED|FACTION_FLAG_HIDDEN))
        return;

    // already set
    if (faction->Flags & FACTION_FLAG_VISIBLE)
        return;

    faction->Flags |= FACTION_FLAG_VISIBLE;
    faction->needSend = true;
    faction->needSave = true;

    SendVisible(faction);
}

void ReputationMgr::SetAtWar(RepListID repListID, bool on)
{
    FactionStateList::iterator itr = m_factions.find(repListID);
    if (itr == m_factions.end())
        return;

    // always invisible or hidden faction can't change war state
    if (itr->second.Flags & (FACTION_FLAG_INVISIBLE_FORCED|FACTION_FLAG_HIDDEN))
        return;

    SetAtWar(&itr->second,on);
}

void ReputationMgr::SetAtWar(FactionState* faction, bool atWar)
{
    // not allow declare war to faction unless already hated or less
    if (atWar && (faction->Flags & FACTION_FLAG_PEACE_FORCED) && ReputationToRank(faction->Standing) > REP_HATED)
        return;

    // already set
    if (((faction->Flags & FACTION_FLAG_AT_WAR) != 0) == atWar)
        return;

    if (atWar)
        faction->Flags |= FACTION_FLAG_AT_WAR;
    else
        faction->Flags &= ~FACTION_FLAG_AT_WAR;

    faction->needSend = true;
    faction->needSave = true;
}

void ReputationMgr::SetInactive(RepListID repListID, bool on)
{
    FactionStateList::iterator itr = m_factions.find(repListID);
    if (itr == m_factions.end())
        return;

    SetInactive(&itr->second,on);
}

void ReputationMgr::SetInactive(FactionState* faction, bool inactive)
{
    // always invisible or hidden faction can't be inactive
    if (inactive && ((faction->Flags & (FACTION_FLAG_INVISIBLE_FORCED|FACTION_FLAG_HIDDEN)) || !(faction->Flags & FACTION_FLAG_VISIBLE) ) )
        return;

    // already set
    if (((faction->Flags & FACTION_FLAG_INACTIVE) != 0) == inactive)
        return;

    if (inactive)
        faction->Flags |= FACTION_FLAG_INACTIVE;
    else
        faction->Flags &= ~FACTION_FLAG_INACTIVE;

    faction->needSend = true;
    faction->needSave = true;
}

void ReputationMgr::LoadFromDB(QueryResultAutoPtr result, uint8 old_race)
{
    // Set initial reputations (so everything is nifty before DB data load)
    Initialize();

    //QueryResult *result = CharacterDatabase.PQuery("SELECT faction,standing,flags FROM character_reputation WHERE guid = '%u'",GetGUIDLow());

    // oldFacId, newFacId
    std::map<uint32, uint32> repSwaps;
    if (old_race)
    {
        uint32 new_race = m_player->GetRace();
        uint32 new_team = Player::TeamForRace(m_player->GetRace());
        bool teamChanged = Player::TeamForRace(old_race) != new_team;

        // on race change - swap innate reputation (Orgrimmar -> Darkspear for example)
        QueryResultAutoPtr resFromRace = GameDataDatabase.PQuery("SELECT faction FROM race_change_swap_innate_rep WHERE race = '%u'", old_race);
        QueryResultAutoPtr resToRace = GameDataDatabase.PQuery("SELECT faction FROM race_change_swap_innate_rep WHERE race = '%u'", new_race);
        uint32 innateOldFac = 0;
        uint32 innateNewFac = 0;
        if (resFromRace && resToRace)
        {
            innateOldFac = (*resFromRace)[0].GetUInt32();
            innateNewFac = (*resToRace)[0].GetUInt32();
        }

        if (innateOldFac && innateNewFac)
        {
            // on faction change - swap all faction-dependent reputations
            if (teamChanged)
            {
                QueryResultAutoPtr resRep = GameDataDatabase.Query("SELECT old_id, new_id FROM race_change_swap_faction_rep");
                if (resRep)
                {
                    uint32 newRepFrom = 0;
                    do
                    {
                        Field *fields = resRep->Fetch();
                        uint32 fac = fields[0].GetUInt32();
                        uint32 swappedTo = fields[1].GetUInt32();
                        if (swappedTo == innateNewFac)
                            newRepFrom = fac;

                        repSwaps[fac] = swappedTo;
                    } while (resRep->NextRow());

                    if (newRepFrom)
                        std::swap(repSwaps[innateOldFac], repSwaps[newRepFrom]);
                }
            }
            // else just add ONE faction to change -> ONLY innate one
            else
            {
                repSwaps[innateOldFac] = innateNewFac;
                repSwaps[innateNewFac] = innateOldFac;
            }
        }
        else
            sLog.outLog(LOG_RACE_CHANGE, "ERROR: Player: %s [%u] innate faction reputations not found", m_player->GetName(), m_player->GetGUIDLow());
    }

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 facId = fields[0].GetUInt32();
            uint32 oldFacId = 0;
            if (old_race)
            {
                if (repSwaps.find(facId) != repSwaps.end())
                {
                    oldFacId = facId;
                    facId = repSwaps[facId];
                }
            }

            FactionEntry const *factionEntry = sFactionStore.LookupEntry(facId);
            if (factionEntry && (factionEntry->reputationListID >= 0))
            {
                FactionState* faction = &m_factions[factionEntry->reputationListID];

                // update standing to current
                faction->Standing = int32(fields[1].GetUInt32());

                uint32 dbFactionFlags = fields[2].GetUInt32();

                if (dbFactionFlags & FACTION_FLAG_VISIBLE)
                    SetVisible(faction);                    // have internal checks for forced invisibility

                if (dbFactionFlags & FACTION_FLAG_INACTIVE)
                    SetInactive(faction, true);             // have internal checks for visibility requirement

                if (dbFactionFlags & FACTION_FLAG_AT_WAR)   // DB at war
                    SetAtWar(faction, true);                // have internal checks for FACTION_FLAG_PEACE_FORCED
                else                                        // DB not at war
                {
                    // allow remove if visible (and then not FACTION_FLAG_INVISIBLE_FORCED or FACTION_FLAG_HIDDEN)
                    if (faction->Flags & FACTION_FLAG_VISIBLE)
                        SetAtWar(faction, false);           // have internal checks for FACTION_FLAG_PEACE_FORCED
                }

                // set atWar for hostile
                if (GetRank(factionEntry) <= REP_HOSTILE)
                    SetAtWar(faction,true);

                // reset changed flag if values similar to saved in DB
                if (faction->Flags==dbFactionFlags)
                {
                    faction->needSend = false;
                    faction->needSave = false;
                }

                if (oldFacId)
                {
                    FactionEntry const *oldFactionEntry = sFactionStore.LookupEntry(oldFacId);
                    if (oldFactionEntry)
                        m_player->ReputationChanged(oldFactionEntry);

                    m_player->ReputationChanged(factionEntry);

                    faction->needSend = true;
                    faction->needSave = true;
                }
            }
        }
        while(result->NextRow());
    }
}

void ReputationMgr::SaveToDB(bool transaction)
{
    static SqlStatementID delRep;
    static SqlStatementID insRep;

    if (transaction)
        RealmDataDatabase.BeginTransaction();

    for (FactionStateList::iterator itr = m_factions.begin(); itr != m_factions.end(); ++itr)
    {
        if (itr->second.needSave)
        {
            SqlStatement stmt = RealmDataDatabase.CreateStatement(delRep, "DELETE FROM character_reputation WHERE guid = ? AND faction = ?");
            stmt.PExecute(m_player->GetGUIDLow(), itr->second.ID);

            stmt = RealmDataDatabase.CreateStatement(insRep, "INSERT INTO character_reputation (guid,faction,standing,flags) VALUES (?, ?, ?, ?)");
            stmt.PExecute(m_player->GetGUIDLow(), itr->second.ID, itr->second.Standing, itr->second.Flags);

            itr->second.needSave = false;
        }
    }

    if (transaction)
        RealmDataDatabase.CommitTransaction();
}
