// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2013 ScriptDev2 <http://www.scriptdev2.com/>
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
SDName: Instance_Hellfire_Ramparts
SD%Complete: 95
SDComment:
SDCategory: Hellfire Ramparts
EndScriptData */

#include "precompiled.h"
#include "hellfire_ramparts.h"

#define ENCOUNTERS                           3
#define ENTRY_VAZRUDEN_HERALD                17307
#define ENTRY_NAZAN                          17536
#define ENTRY_HELLFIRE_SENTRY                17517
#define ENTRY_REINFORCED_FEL_IRON_CHEST_H    185169
#define ENTRY_REINFORCED_FEL_IRON_CHEST      185168

const float VazrudenMiddle[3] = {-1435.27, 1769.33, 82.079};

struct instance_ramparts : public ScriptedInstance
{
    instance_ramparts(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();}

    GBK_handler m_gbk;
    uint32 Encounter[ENCOUNTERS];
    std::string str_data;
    std::list<uint64> SentryList;
    uint64 VazHeraldGUID;
    uint64 ChestNGUID;
    uint64 ChestHGUID;

    void Initialize()
    {
        for (uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;

        VazHeraldGUID = 0;
        ChestNGUID = 0;
        ChestHGUID = 0;
    }

    bool IsEncounterInProgress() const
    {
        for (uint8 i = 0; i < ENCOUNTERS; i++)
            if (Encounter[i] == IN_PROGRESS) return true;

        return false;
    }

    void OnObjectCreate(GameObject* pGo)
    {
        switch(pGo->GetEntry())
        {
            case ENTRY_REINFORCED_FEL_IRON_CHEST:
                ChestNGUID = pGo->GetGUID();
                break;
            case ENTRY_REINFORCED_FEL_IRON_CHEST_H:
                ChestHGUID = pGo->GetGUID();
                break;
            default: break;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch (creature_entry)
        {
            case ENTRY_VAZRUDEN_HERALD:
                VazHeraldGUID = creature->GetGUID();
                break;
            case ENTRY_HELLFIRE_SENTRY:
                SentryList.push_back(creature->GetGUID());
                break;
        }
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

        debug_log("OSCR: Instance Hellfire Ramparts: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case DATA_GARGOLMAR:    return GBK_HR_GARGOLMAR;
            case DATA_OMOR:         return GBK_HR_OMOR;
            case DATA_NAZAN:        return GBK_HR_NAZAN;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case DATA_GARGOLMAR:
                if (Encounter[0] != DONE)
                    Encounter[0] = data;
                break;
            case DATA_OMOR:
                if (Encounter[1] != DONE)
                    Encounter[1] = data;
                break;
            case DATA_NAZAN:
                if (data == FAIL)
                {
                    for (std::list<uint64>::iterator it = SentryList.begin(); it != SentryList.end(); ++it)
                    {
                        if (Creature* sentry = instance->GetCreature(*it))
                        {
                            if (sentry->isDead())
                                sentry->Respawn();
                        }
                    }
                    Encounter[2] = NOT_STARTED;
                }

                if (data == DONE)
                {
                    Player *player = GetPlayerInMap();

                    if (instance->IsHeroic())
                    {
                        if (GameObject* pGo = instance->GetGameObject(ChestHGUID))
                            pGo->RemoveFlag(GAMEOBJECT_FLAGS,GO_FLAG_NOTSELECTABLE);
                    }
                    else
                    {
                        if (GameObject* pGo = instance->GetGameObject(ChestNGUID))
                            pGo->RemoveFlag(GAMEOBJECT_FLAGS,GO_FLAG_NOTSELECTABLE);
                    }
                }

                if (Encounter[2] != DONE)
                Encounter[2] = data;
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
        {
            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;
        }
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

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
            case DATA_GARGOLMAR: return Encounter[0];
            case DATA_OMOR: return Encounter[1];
            case DATA_NAZAN: return Encounter[2];
        }
        return false;
    }

    uint64 GetData64(uint32 data)
    {
        switch (data)
        {
            case DATA_VAZHERALD:
                return VazHeraldGUID;
        }
        return 0;
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;
        std::ostringstream saveStream;

        saveStream << Encounter[0] << " " << Encounter[1] << " " << Encounter[2];

        char* out = new char[saveStream.str().length() + 1];
        strcpy(out, saveStream.str().c_str());
        if (out)
        {
            OUT_SAVE_INST_DATA_COMPLETE;
            return out;
        }

        return str_data.c_str();
    }

    void Load(const char* in)
    {
        if (!in)
        {
            OUT_LOAD_INST_DATA_FAIL;
            return;
        }

        OUT_LOAD_INST_DATA(in);

        std::istringstream loadStream(in);
        loadStream >> Encounter[0] >> Encounter[1] >> Encounter[2];

        for (uint8 i = 0; i < ENCOUNTERS; ++i)
            if (Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};
InstanceData* GetInstanceData_instance_ramparts(Map* pMap)
{
    return new instance_ramparts(pMap);
}

void AddSC_instance_ramparts()
{
    Script* pNewScript;
    pNewScript = new Script;
    pNewScript->Name = "instance_ramparts";
    pNewScript->GetInstanceData = &GetInstanceData_instance_ramparts;
    pNewScript->RegisterSelf();
}