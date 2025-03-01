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
SDName: Instance_Temple_of_Ahnqiraj
SD%Complete: 80
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_temple_of_ahnqiraj.h"

#define ENCOUNTERS 9

struct instance_temple_of_ahnqiraj : public ScriptedInstance
{
    instance_temple_of_ahnqiraj(Map *map) : ScriptedInstance(map) {Initialize();};

    //Storing Skeram, Vem and Kri.
    uint64 SkeramGUID;
    uint64 VemGUID;
    uint64 KriGUID;
    uint64 VeklorGUID;
    uint64 VeknilashGUID;
    uint64 OuroSpawnerGUID;
    uint64 SandWormBaseGUID;

    uint32 BugTrioDeathCount;

    uint32 CthunPhase;

    bool Vem;

    uint32 Encounters[ENCOUNTERS];

    void Initialize()
    {
        SkeramGUID = 0;
        VemGUID = 0;
        KriGUID = 0;
        VeklorGUID = 0;
        VeknilashGUID = 0;
        OuroSpawnerGUID = 0;
        SandWormBaseGUID = 0;
        BugTrioDeathCount = 0;

        CthunPhase = 0;

        Vem = false;

        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounters[i] = NOT_STARTED;
    }

    uint32 GetEncounterForEntry(uint32 entry)
    {
        switch(entry)
        {
            case 15263:
                return DATA_THE_PROPHET_SKERAM;
            case 15516:
                return DATA_BATTLEGUARD_SARTURA;
            case 15510:
                return DATA_FANKRISS_THE_UNYIELDING;
            case 15509:
                return DATA_PRINCESS_HUHURAN;
            case 15276:
            case 15275:
                return DATA_TWIN_EMPERORS;
            case 15727:
                return DATA_C_THUN;
            case 15543:
            case 15544:
            case 15511:
                return DATA_BUG_TRIO;
            case 15299:
                return DATA_VISCIDUS;
            case 15517:
                return DATA_OURO;
            default:
                return 0;
        }
    }

    void OnObjectCreate (GameObject* go)
    {
        switch(go->GetEntry())
        {
            case 180795:
                SandWormBaseGUID = go->GetGUID();
                break;
            default:
                break;
        }
    }

    void OnCreatureCreate (Creature *creature, uint32 creature_entry)
    {
        switch (creature_entry)
        {
            case 15263:
            if (GetData(DATA_THE_PROPHET_SKERAM) != IN_PROGRESS)
                SkeramGUID = creature->GetGUID();
            break;
            case 15544: VemGUID = creature->GetGUID(); break;
            case 15511: KriGUID = creature->GetGUID(); break;
            case 15276: VeklorGUID = creature->GetGUID(); break;
            case 15275: VeknilashGUID = creature->GetGUID(); break;
            case 15957: OuroSpawnerGUID = creature->GetGUID(); break;
        }

        HandleInitCreatureState(creature);
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            if(Encounters[i] == IN_PROGRESS) return true;

        return false;
    }

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
            case DATA_THE_PROPHET_SKERAM:
                return Encounters[0];
            case DATA_BATTLEGUARD_SARTURA:
                return Encounters[1];
            case DATA_FANKRISS_THE_UNYIELDING:
                return Encounters[2];
            case DATA_PRINCESS_HUHURAN:
                return Encounters[3];
            case DATA_TWIN_EMPERORS:
                return Encounters[4];
            case DATA_C_THUN:
                return Encounters[5];
            case DATA_BUG_TRIO:
                return Encounters[6];
            case DATA_VISCIDUS:
                return Encounters[7];
            case DATA_OURO:
                return Encounters[8];
            case DATA_BUG_TRIO_DEATH:
                return BugTrioDeathCount;
            case DATA_CTHUN_PHASE:
                return CthunPhase;
            case DATA_VEM:
                return Vem;
        }
        return 0;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case DATA_THE_PROPHET_SKERAM:
                if(Encounters[0] != DONE)
                    Encounters[0] = data;
                break;
            case DATA_BATTLEGUARD_SARTURA:
                if(Encounters[1] != DONE)
                    Encounters[1] = data;
                break;
            case DATA_FANKRISS_THE_UNYIELDING:
                if(Encounters[2] != DONE)
                    Encounters[2] = data;
                break;
            case DATA_PRINCESS_HUHURAN:
                if(Encounters[3] != DONE)
                    Encounters[3] = data;
                break;
            case DATA_TWIN_EMPERORS:
                if(Encounters[4] != DONE)
                    Encounters[4] = data;
                break;
            case DATA_C_THUN:
                if(Encounters[5] != DONE)
                    Encounters[5] = data;
                break;
            case DATA_BUG_TRIO:
                if(data == DONE)
                    BugTrioDeathCount++;
                if(Encounters[6] != DONE && (data != DONE || BugTrioDeathCount == 3))
                    Encounters[6] = data;
                break;
            case DATA_VISCIDUS:
                if(Encounters[7] != DONE)
                    Encounters[7] = data;
                break;
            case DATA_OURO:
                // Respawn the Ouro spawner on fail
                switch(data)
                {
                    case FAIL:
                        if (Creature* pSpawner = instance->GetCreature(OuroSpawnerGUID))
                            pSpawner->Respawn();
                    case DONE:
                        // Despawn the sandworm base on Done or Fail
                        if (GameObject* pBase = instance->GetGameObject(SandWormBaseGUID))
                            pBase->SetLootState(GO_JUST_DEACTIVATED);
                    break;
                }
                Encounters[8] = data;
                break;
            case DATA_VEM:
                Vem = data;
                break;
            case DATA_CTHUN_PHASE:
                CthunPhase = data;
                break;
        }

        if(data == DONE)
            SaveToDB();
    }

    uint64 GetData64 (uint32 identifier)
    {
        switch(identifier)
        {
            case DATA_SKERAM:
                return SkeramGUID;
            case DATA_VEM:
                return VemGUID;
            case DATA_KRI:
                return KriGUID;
            case DATA_VEKLOR:
                return VeklorGUID;
            case DATA_VEKNILASH:
                return VeknilashGUID;
        }
        return 0;
    }                                                       // end GetData64

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounters[0] << " ";
        stream << Encounters[1] << " ";
        stream << Encounters[2] << " ";
        stream << Encounters[3] << " ";
        stream << Encounters[4] << " ";
        stream << Encounters[5] << " ";
        stream << Encounters[6] << " ";
        stream << Encounters[7] << " ";
        stream << Encounters[8];

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
        stream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3] >>
            Encounters[4] >> Encounters[5] >> Encounters[6] >> Encounters[7] >> Encounters[8];
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                Encounters[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_temple_of_ahnqiraj(Map* map)
{
    return new instance_temple_of_ahnqiraj(map);
}

void AddSC_instance_temple_of_ahnqiraj()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_temple_of_ahnqiraj";
    newscript->GetInstanceData = &GetInstanceData_instance_temple_of_ahnqiraj;
    newscript->RegisterSelf();
}

