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
SDName: instance_uldaman
SD%Complete: 99%
SDComment: Need some cosmetics updates when archeadas door are closing (Guardians Waypoints).
SDCategory: Uldaman
EndScriptData */

#include "precompiled.h"
#include "def_uldaman.h"

#define SPELL_ARCHAEDAS_AWAKEN        10347
#define SPELL_AWAKEN_VAULT_WALKER     10258

#define ARCHAEDAS_TEMPLE_DOOR           141869
#define ALTAR_OF_ARCHAEDAS              133234

#define ALTAR_OF_THE_KEEPER_TEMPLE_DOOR 124367
#define ALTAR_OF_THE_KEEPER_TEMPLE      130511

#define ANCIENT_VAULT_DOOR              124369
#define IRONAYA_SEAL_DOOR               124372

#define KEYSTONE_GO                     124371

#define ENCOUNTERS 3

struct instance_uldaman : public ScriptedInstance
{
    instance_uldaman(Map *map) : ScriptedInstance(map) {};

    void Initialize()
    {
        archaedasGUID = 0;
        ironayaGUID = 0;

        altarOfTheKeeperTempleDoor = 0;
        altarOfArcheadas = 0;
        archaedasTempleDoor = 0;
        ancientVaultDoor = 0;
        ironayaSealDoor = 0;

        keystoneGUID = 0;

        _ChangedNameironayaSealDoorTimer.Reset(26000);
        keystoneCheck = false;

        for (uint8 i = 0; i < ENCOUNTERS; ++i)
            Encounters[i] = NOT_STARTED;

    }

    uint64 archaedasGUID;
    uint64 ironayaGUID;

    uint64 altarOfTheKeeperTempleDoor;
    uint64 altarOfArcheadas;
    uint64 archaedasTempleDoor;
    uint64 ancientVaultDoor;
    uint64 ironayaSealDoor;

    uint64 keystoneGUID;

    Timer _ChangedNameironayaSealDoorTimer;
    bool keystoneCheck;

    std::vector<uint64> stoneKeeper;
    std::vector<uint64> altarOfTheKeeperCount;
    std::vector<uint64> vaultWalker;
    std::vector<uint64> earthenGuardian;
    std::vector<uint64> archaedasWallMinions;    // minions lined up around the wall

    uint32 Encounters[ENCOUNTERS];

    void OnObjectCreate(GameObject* go)
    {
        switch (go->GetEntry())
        {
            case ALTAR_OF_THE_KEEPER_TEMPLE_DOOR:         // lock the door
                altarOfTheKeeperTempleDoor = go->GetGUID();

                if (Encounters[0] == DONE)
                    HandleGameObject(0, true, go);
                break;

            case ALTAR_OF_ARCHAEDAS:         // lock the door
                altarOfArcheadas = go->GetGUID();
                if (Encounters[2] == DONE)
                    go->SetUInt32Value(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                break;

            case ARCHAEDAS_TEMPLE_DOOR:
                archaedasTempleDoor = go->GetGUID();

                if (Encounters[0] == DONE)
                    HandleGameObject(0, true, go);
                break;

            case ANCIENT_VAULT_DOOR:
                go->SetUInt32Value(GAMEOBJECT_STATE, 1);
                go->SetUInt32Value(GAMEOBJECT_FLAGS, 33);
                ancientVaultDoor = go->GetGUID();

                if (Encounters[1] == DONE)
                    HandleGameObject(0, true, go);
                break;

            case IRONAYA_SEAL_DOOR:
                ironayaSealDoor = go->GetGUID();

                if (Encounters[2] == DONE)
                    HandleGameObject(0, true, go);
                break;

            case KEYSTONE_GO:
                keystoneGUID = go->GetGUID();

                if (Encounters[2] == DONE)
                {
                    HandleGameObject(0, true, go);
                    go->SetUInt32Value(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                }
                break;
        }
    }

    void SetDoor(uint64 guid, bool open)
    {
        GameObject *go = instance->GetGameObject(guid);
        if (!go)
            return;

        HandleGameObject(0, open, go);
    }

    void BlockGO(uint64 guid)
    {
        GameObject *go = instance->GetGameObject(guid);
        if (!go)
            return;
        go->SetUInt32Value(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);

    }

    void ActivateStoneKeepers()
    {
        for (std::vector<uint64>::iterator i = stoneKeeper.begin(); i != stoneKeeper.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (!target || !target->isAlive() || target->getFaction() == 14)
                continue;
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            target->setFaction(14);
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            return;        // only want the first one we find
        }
        // if we get this far than all four are dead so open the door
        SetData(DATA_ALTAR_DOORS, DONE);
        SetDoor(archaedasTempleDoor, true); //open next the door too
    }

    void ActivateWallMinions()
    {
        Creature *archaedas = instance->GetCreature(archaedasGUID);
        if (!archaedas)
            return;

        for (std::vector<uint64>::iterator i = archaedasWallMinions.begin(); i != archaedasWallMinions.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (!target || !target->isAlive() || target->getFaction() == 14)
                continue;
            archaedas->CastSpell(target, SPELL_AWAKEN_VAULT_WALKER, true);
            target->CastSpell(target, SPELL_ARCHAEDAS_AWAKEN, true);
            return;        // only want the first one we find
        }
    }

    // used when Archaedas dies.  All active minions must be despawned.
    void DeActivateMinions()
    {
        // first despawn any aggroed wall minions
        for (std::vector<uint64>::iterator i = archaedasWallMinions.begin(); i != archaedasWallMinions.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (!target || target->isDead() || target->getFaction() != 14)
                continue;
            target->setDeathState(JUST_DIED);
            target->RemoveCorpse();
        }

        // Vault Walkers
        for (std::vector<uint64>::iterator i = vaultWalker.begin(); i != vaultWalker.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (!target || target->isDead() || target->getFaction() != 14)
                continue;
            target->setDeathState(JUST_DIED);
            target->RemoveCorpse();
        }

        // Earthen Guardians
        for (std::vector<uint64>::iterator i = earthenGuardian.begin(); i != earthenGuardian.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (!target || target->isDead() || target->getFaction() != 14)
                continue;
            target->setDeathState(JUST_DIED);
            target->RemoveCorpse();
        }
    }

    void ActivateArchaedas()
    {
        Creature *archaedas = instance->GetCreature(archaedasGUID);
        if (!archaedas)
            return;

        archaedas->CastSpell(archaedas, SPELL_ARCHAEDAS_AWAKEN, false);
    }

    void ActivateIronaya()
    {
        Creature *ironaya = instance->GetCreature(ironayaGUID);
        if (!ironaya)
            return;

        ironaya->setFaction(415);
        ironaya->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        ironaya->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void RespawnMinions()
    {
        // first respawn any aggroed wall minions
        for (std::vector<uint64>::iterator i = archaedasWallMinions.begin(); i != archaedasWallMinions.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (target && target->isDead())
            {
                target->Respawn();
                target->GetMotionMaster()->MoveTargetedHome();
            }
        }

        // Vault Walkers
        for (std::vector<uint64>::iterator i = vaultWalker.begin(); i != vaultWalker.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (target && target->isDead())
            {
                target->Respawn();
                target->GetMotionMaster()->MoveTargetedHome();
            }
        }

        // Earthen Guardians
        for (std::vector<uint64>::iterator i = earthenGuardian.begin(); i != earthenGuardian.end(); ++i)
        {
            Creature *target = instance->GetCreature(*i);
            if (target && target->isDead())
            {
                target->Respawn();
                target->GetMotionMaster()->MoveTargetedHome();
            }
        }
    }

    void Update(uint32 diff)
    {
        if (!keystoneCheck)
        return;

        if (_ChangedNameironayaSealDoorTimer.Expired(diff))
        {
            ActivateIronaya();

            SetDoor(ironayaSealDoor, true);
            BlockGO(keystoneGUID);

            SetData(DATA_IRONAYA_DOOR, DONE); //save state
            keystoneCheck = false;
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        switch (type)
        {
            case DATA_ALTAR_DOORS:
                Encounters[0] = data;
                if (data == DONE)
                    SetDoor(altarOfTheKeeperTempleDoor, true);
                break;

            case DATA_ANCIENT_DOOR:
                if (Encounters[1] == DONE)
                    return;

                if (data == NOT_STARTED)
                {
                    instance->GetGameObject(altarOfArcheadas)->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                }
                if (data == IN_PROGRESS && Encounters[1] != IN_PROGRESS)
                {
                    ActivateArchaedas();
                }
                if (data == DONE) //archeadas defeat
                {
                    SetDoor(archaedasTempleDoor, true); //re open enter door
                    SetDoor(ancientVaultDoor, true);
                }
                Encounters[1] = data;
                break;

            case DATA_IRONAYA_DOOR:
                Encounters[2] = data;
                break;

            case DATA_STONE_KEEPERS:
                ActivateStoneKeepers();
                break;

            case DATA_MINIONS:
                switch (data)
                {
                    case NOT_STARTED:
                        if (Encounters[0] == DONE) //if players opened the doors 
                            SetDoor(archaedasTempleDoor, true);

                        RespawnMinions();
                        break;
                    case IN_PROGRESS:
                        ActivateWallMinions();
                        break;
                    case SPECIAL:
                        DeActivateMinions();
                        break;
                }
                break;

            case DATA_IRONAYA_SEAL:
                keystoneCheck = true;
                break;
        }

        if (data == DONE)
            SaveToDB();
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch (creature_entry)
        {
            case 4857:    // Stone Keeper
                stoneKeeper.push_back(creature->GetGUID());
                break;

            case 7309:    // Earthen Custodian
            case 7077:    // Earthen Hallshaper
                archaedasWallMinions.push_back(creature->GetGUID());
                break;

            case 7076:    // Earthen Guardian
                earthenGuardian.push_back(creature->GetGUID());
                break;

            case 7228:   // Ironaya
                ironayaGUID = creature->GetGUID();
                break;

            case 10120:    // Vault Walker
                vaultWalker.push_back(creature->GetGUID());
                break;

            case 2748:    // Archaedas
                archaedasGUID = creature->GetGUID();
                break;

        } // end switch
    } // end OnCreatureCreate


    uint64 GetData64(uint32 identifier)
    {
        if (identifier == 0) return archaedasGUID;
        if (identifier == 1) return vaultWalker[0];    // VaultWalker1
        if (identifier == 2) return vaultWalker[1];    // VaultWalker2
        if (identifier == 3) return vaultWalker[2];    // VaultWalker3
        if (identifier == 4) return vaultWalker[3];    // VaultWalker4

        if (identifier == 5) return earthenGuardian[0];
        if (identifier == 6) return earthenGuardian[1];
        if (identifier == 7) return earthenGuardian[2];
        if (identifier == 8) return earthenGuardian[3];
        if (identifier == 9) return earthenGuardian[4];
        if (identifier == 10) return earthenGuardian[5];

        return 0;
    } // end GetData64

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounters[0] << " ";
        stream << Encounters[1] << " ";
        stream << Encounters[2];

        OUT_SAVE_INST_DATA_COMPLETE;

        return stream.str();
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
        loadStream >> Encounters[0] >> Encounters[1] >> Encounters[2];

        for (uint8 i = 0; i < ENCOUNTERS; ++i)
            if (Encounters[i] == IN_PROGRESS)
                Encounters[i] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;

    }

};



InstanceData* GetInstanceData_instance_uldaman(Map* map)
{
    return new instance_uldaman(map);
}

void AddSC_instance_uldaman()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_uldaman";
    newscript->GetInstanceData = &GetInstanceData_instance_uldaman;
    newscript->RegisterSelf();
}


