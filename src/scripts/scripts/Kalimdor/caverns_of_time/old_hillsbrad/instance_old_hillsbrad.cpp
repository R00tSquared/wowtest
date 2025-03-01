// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Instance_Old_Hillsbrad
SD%Complete: 99
SDComment:
SDCategory: Caverns of Time, Old Hillsbrad Foothills
EndScriptData */

#include "precompiled.h"
#include "def_old_hillsbrad.h"

#define ENCOUNTERS               15

#define THRALL_ENTRY             17876
#define TARETHA_ENTRY            18887
#define EPOCH_ENTRY              18096
#define ORC_PRISONER_ENTRY       18598
#define DRAKE_ENTRY              17848

#define QUEST_ENTRY_DIVERSION    10283
#define LODGE_QUEST_TRIGGER      20155

#define GO_ROARING_FLAME         182592

#define SAY_DRAKE_ENTER          -1560006

static const float OrcLoc[][4] =
{
    {2104.51f, 91.96f, 53.14f, 0},
    {2192.58f, 238.44f, 52.44f, 0}
};

enum Summon
{
    NOT_SUMMONED    = 0,
    WAIT_FOR_SUMMON = 1,
    SUMMONED        = 2
};

struct instance_old_hillsbrad : public ScriptedInstance
{
    instance_old_hillsbrad(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    GBK_handler m_gbk;
    uint32 Encounter[ENCOUNTERS];
    uint32 BarrelCount;
    uint32 ThrallEventCount;

    uint64 ThrallGUID;
    uint64 TarethaGUID;
    uint64 EpochGUID;
    uint64 BarrelGUID;

    std::list<GameObject*> RoaringFlamesList;
    std::list<uint64> LeftPrisonersList;
    std::list<uint64> RightPrisonersList;
    std::list<uint64> UsedBarrelGUIDList;

    Summon summon;

    void Initialize()
    {
        BarrelCount         = 0;
        ThrallEventCount    = 0;
        ThrallGUID          = 0;
        TarethaGUID         = 0;
        EpochGUID           = 0;
        BarrelGUID          = 0;

        summon = NOT_SUMMONED;

        for (uint8 i = 0; i < ENCOUNTERS; ++i)
            Encounter[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for (uint8 i = 7; i < ENCOUNTERS; ++i)
            if (Encounter[i] == IN_PROGRESS)
                return true;
        return false;
    }

    void OnPlayerEnter(Player* player)
    {
        if (summon == NOT_SUMMONED)
            summon = WAIT_FOR_SUMMON;
    }

    Player* GetPlayerInMap()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* plr = itr->getSource())
                    return plr;
            }
        }

        debug_log("TSCR: Instance Old Hillsbrad: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    void UpdateOHWorldState()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->getSource())
                {
                    player->SendUpdateWorldState(WORLD_STATE_OH, BarrelCount);

                    if (BarrelCount == 5)
                        player->KilledMonster(LODGE_QUEST_TRIGGER, 0);
                }
            }
        }
        else
            debug_log("TSCR: Instance Old Hillsbrad: UpdateOHWorldState, but PlayerList is empty!");
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch (creature_entry)
        {
            case THRALL_ENTRY:
                ThrallGUID = creature->GetGUID();
                break;
            case TARETHA_ENTRY:
                TarethaGUID = creature->GetGUID();
                break;
            case EPOCH_ENTRY:
                EpochGUID = creature->GetGUID();
                break;
            case ORC_PRISONER_ENTRY:
            if (creature->GetPositionZ() > 53.4f)
            {
                if (creature->GetPositionY() > 150.0f)
                    LeftPrisonersList.push_back(creature->GetGUID());
                else
                    RightPrisonersList.push_back(creature->GetGUID());
            }
            break;
        }
    }

    void OnObjectCreate(GameObject* go)
    {
        if (go->GetEntry() == GO_ROARING_FLAME)
            RoaringFlamesList.push_back(go);
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case DATA_SKARLOC:          return GBK_OHF_SKARLOC;
            case DATA_EPOCH_HUNTER:     return GBK_OHF_EPOCH;
            case DATA_LEUTENANT_DRAKE:  return GBK_OHF_LEUTENANT;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        Player *player = GetPlayerInMap();

        if (!player)
        {
            debug_log("TSCR: Instance Old Hillsbrad: SetData (Type: %u Data %u) cannot find any player.", type, data);
            return;
        }

        switch(type)
        {
            case TYPE_BARREL_DIVERSION:
            {
                if (data == IN_PROGRESS)
                {
                    if (BarrelCount >= 5)
                        return;

                    for (std::list<uint64>::iterator it = UsedBarrelGUIDList.begin(); it != UsedBarrelGUIDList.end(); ++it)
                    {
                        if ((*it) == BarrelGUID)
                            return;
                    }

                    ++BarrelCount;

                    UpdateOHWorldState();

                    debug_log("TSCR: Instance Old Hillsbrad: go_barrel_old_hillsbrad count %u", BarrelCount);

                    if (BarrelCount == 5)
                    {
                        if (Creature* drake = player->SummonCreature(DRAKE_ENTRY, 2128.43f, 71.01f, 64.42, 1.74f, TEMPSUMMON_DEAD_DESPAWN, 15000))
                            DoScriptText(SAY_DRAKE_ENTER, drake, player);

                        Encounter[0] = DONE;

                        for (std::list<GameObject*>::iterator itr = RoaringFlamesList.begin(); itr != RoaringFlamesList.end(); ++itr)
                        {
                            // move the orcs outside the houses
                            float x, y, z;
                            for (std::list<uint64>::iterator it = RightPrisonersList.begin(); it != RightPrisonersList.end(); ++it)
                            {
                                if (Creature* Orc = instance->GetCreature(*it))
                                {
                                    Orc->GetRandomPoint(OrcLoc[0][0], OrcLoc[0][1], OrcLoc[0][2], 10.0f, x, y, z);
                                    Orc->SetWalk(false);
                                    Orc->GetMotionMaster()->MovePoint(0, x, y, z);
                                }
                            }

                            for (std::list<uint64>::iterator il = LeftPrisonersList.begin(); il != LeftPrisonersList.end(); ++il)
                            {
                                if (Creature* Orc = instance->GetCreature(*il))
                                {
                                    Orc->GetRandomPoint(OrcLoc[1][0], OrcLoc[1][1], OrcLoc[1][2], 10.0f, x, y, z);
                                    Orc->SetWalk(false);
                                    Orc->GetMotionMaster()->MovePoint(0, x, y, z);
                                }
                            }

                            (*itr)->SetRespawnTime(1800);
                            (*itr)->UpdateObjectVisibility();
                        }
                    }
                }
                break;
            }
            case TYPE_THRALL_EVENT:
            {
                if (data == FAIL)
                {
                    if (ThrallEventCount <= 20)
                    {
                        ThrallEventCount++;
                        debug_log("TSCR: Instance Old Hillsbrad: Thrall event failed %u times.", ThrallEventCount);

                        Encounter[1] = NOT_STARTED;

                        if (Encounter[2] == IN_PROGRESS)
                            Encounter[2] = NOT_STARTED;

                        if (Encounter[3] == IN_PROGRESS)
                            Encounter[3] = NOT_STARTED;

                        if (Encounter[4] == IN_PROGRESS)
                            Encounter[4] = NOT_STARTED;

                        if (Encounter[5] == IN_PROGRESS)
                            Encounter[5] = NOT_STARTED;
                    }
                    else if (ThrallEventCount > 20)
                    {
                        Encounter[0] = DONE;
                        Encounter[1] = DONE;
                        Encounter[2] = DONE;
                        Encounter[3] = DONE;
                        Encounter[4] = DONE;
                        Encounter[5] = DONE;
                        Encounter[6] = DONE;
                        Encounter[7] = DONE;
                        Encounter[8] = DONE;
                        debug_log("TSCR: Instance Old Hillsbrad: Thrall event failed %u times. This is the end.", ThrallEventCount);
                    }
                }
                else
                    Encounter[1] = data;
                debug_log("TSCR: Instance Old Hillsbrad: Thrall escort event adjusted to data %u.", data);
                break;
            }
            case TYPE_THRALL_PART1:
                Encounter[2] = data;
                debug_log("TSCR: Instance Old Hillsbrad: Thrall event part I adjusted to data %u.", data);
                break;
            case TYPE_THRALL_PART2:
                Encounter[3] = data;
                debug_log("TSCR: Instance Old Hillsbrad: Thrall event part II adjusted to data %u.", data);
                break;
            case TYPE_THRALL_PART3:
                Encounter[4] = data;
                debug_log("TSCR: Instance Old Hillsbrad: Thrall event part III adjusted to data %u.", data);
                break;
            case TYPE_THRALL_PART4:
                Encounter[5] = data;
                 debug_log("TSCR: Instance Old Hillsbrad: Thrall event part IV adjusted to data %u.", data);
                break;
            case DATA_SKARLOC_DEATH:
                if (Encounter[6] != DONE)
                    Encounter[6] = data;
                break;
            case DATA_DRAKE_DEATH:
                if (Encounter[7] != DONE)
                    Encounter[7] = data;
                break;
            case DATA_EPOCH_DEATH:
                if (Encounter[8] != DONE)
                    Encounter[8] = data;
                break;
            case DATA_SKARLOC:
                if (data == DONE)
                    Encounter[9] = data;
                break;
            case DATA_EPOCH_HUNTER:
                if(data == DONE)
                    Encounter[10] = data;
                break;
            case DATA_LEUTENANT_DRAKE:
                if(data == DONE)
                    Encounter[11] = data;
                break;
        }

        if (instance->IsHeroic())
        {
            GBK_Encounters gbkEnc = EncounterForGBK(type);
            if (gbkEnc != GBK_NONE)
            {
                if (data == DONE)
                    m_gbk.StopCombat(gbkEnc, true);
                else if (data == NOT_STARTED)
                    m_gbk.StopCombat(gbkEnc, false);
                else if (data == IN_PROGRESS)
                    m_gbk.StartCombat(gbkEnc);
            }
        }

        if (data == DONE)
            SaveToDB();
    }

    void OnPlayerDealDamage(Player* plr, uint32 amount)
    {
        m_gbk.DamageDone(plr->GetGUIDLow(), amount);
    }

    void OnPlayerHealDamage(Player* plr, uint32 amount)
    {
        m_gbk.HealingDone(plr->GetGUIDLow(), amount);
    }

    void OnPlayerDeath(Player* plr)
    {
        m_gbk.PlayerDied(plr->GetGUIDLow());
    }

    uint32 GetData(uint32 data)
    {
        switch (data)
        {
            case TYPE_BARREL_DIVERSION:
                return Encounter[0];
            case TYPE_THRALL_EVENT:
                return Encounter[1];
            case TYPE_THRALL_PART1:
                return Encounter[2];
            case TYPE_THRALL_PART2:
                return Encounter[3];
            case TYPE_THRALL_PART3:
                return Encounter[4];
            case TYPE_THRALL_PART4:
                return Encounter[5];
            case DATA_SKARLOC_DEATH:
                return Encounter[6];
            case DATA_DRAKE_DEATH:
                return Encounter[7];
            case DATA_EPOCH_DEATH:
                return Encounter[8];
            case DATA_SKARLOC:
                return Encounter[9];
            case DATA_EPOCH_HUNTER:
                return Encounter[10];
            case DATA_LEUTENANT_DRAKE:
                return Encounter[11];
        }

        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch (data)
        {
            case DATA_THRALL:
                return ThrallGUID;
            case DATA_TARETHA:
                return TarethaGUID;
            case DATA_EPOCH:
                return EpochGUID;
        }
        return 0;
    }

    void Update(uint32 diff)
    {
        if (summon == WAIT_FOR_SUMMON)
        {
            if (instance->GetPlayers().isEmpty())
                return;

            Player* player = instance->GetPlayers().begin()->getSource();

            if (GetData(TYPE_THRALL_PART1) == NOT_STARTED)
                player->SummonCreature(THRALL_ENTRY, 2231.51f, 119.84f, 82.297f, 4.15f,TEMPSUMMON_DEAD_DESPAWN,15000);

            if (GetData(TYPE_THRALL_PART1) == DONE && GetData(TYPE_THRALL_PART2) == NOT_STARTED)
            {
                player->SummonCreature(THRALL_ENTRY, 2063.40f, 229.512f, 64.488f, 2.18f,TEMPSUMMON_DEAD_DESPAWN,15000);
                player->SummonCreature(18798, 2047.90f, 254.85f, 62.822f, 5.94f, TEMPSUMMON_DEAD_DESPAWN, 15000);
            }

            if (GetData(TYPE_THRALL_PART2) == DONE && GetData(TYPE_THRALL_PART3) == NOT_STARTED)
                player->SummonCreature(THRALL_ENTRY, 2486.91f, 626.357f, 58.076f, 4.66f,TEMPSUMMON_DEAD_DESPAWN,15000);

            if (GetData(TYPE_THRALL_PART3) == DONE && GetData(TYPE_THRALL_PART4) == NOT_STARTED)
                player->SummonCreature(THRALL_ENTRY, 2660.48f, 659.409f, 61.937f, 5.83f,TEMPSUMMON_DEAD_DESPAWN,15000);

            summon = SUMMONED;
        }
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " ";
        stream << Encounter[1] << " ";
        stream << Encounter[2] << " ";
        stream << Encounter[3] << " ";
        stream << Encounter[4] << " ";
        stream << Encounter[5] << " ";
        stream << Encounter[6] << " ";
        stream << Encounter[7] << " ";
        stream << Encounter[8] << " ";
        stream << Encounter[9] << " ";
        stream << Encounter[10] << " ";
        stream << Encounter[11];

        OUT_SAVE_INST_DATA_COMPLETE;

        return stream.str();
    }

    void Load(const char* in)
    {
        if(!in)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(in);

        std::istringstream stream(in);
        stream  >> Encounter[0] >> Encounter[1] >> Encounter[2]
                >> Encounter[3] >> Encounter[4] >> Encounter[5]
                >> Encounter[6] >> Encounter[7] >> Encounter[8]
                >> Encounter[9] >> Encounter[10] >> Encounter[11];

        for (uint8 i = 0; i < ENCOUNTERS; ++i)
            if (Encounter[i] == IN_PROGRESS)        // Do not load an encounter as "In Progress" - reset it instead.
                Encounter[i] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_old_hillsbrad(Map* map)
{
    return new instance_old_hillsbrad(map);
}

/*######
## go_barrel_old_hillsbrad
######*/

bool GOUse_go_barrel_old_hillsbrad(Player *player, GameObject* go)
{
    instance_old_hillsbrad* pInstance;
    pInstance = (instance_old_hillsbrad*)go->GetInstanceData();

    if (!pInstance)
        return false;

    if (pInstance->GetData(TYPE_BARREL_DIVERSION) == DONE)
        return false;

    pInstance->BarrelGUID = go->GetGUID();
    pInstance->SetData(TYPE_BARREL_DIVERSION, IN_PROGRESS);
    pInstance->UsedBarrelGUIDList.push_back(go->GetGUID());

    go->UseDoorOrButton(1800);

    return false;
}

void AddSC_instance_old_hillsbrad()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_old_hillsbrad";
    newscript->GetInstanceData = &GetInstanceData_instance_old_hillsbrad;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_barrel_old_hillsbrad";
    newscript->pGOUse = &GOUse_go_barrel_old_hillsbrad;
    newscript->RegisterSelf();
}
