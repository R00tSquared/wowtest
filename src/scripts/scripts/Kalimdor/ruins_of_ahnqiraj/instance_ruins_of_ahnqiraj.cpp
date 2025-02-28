// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
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
SDName: Instance_Ruins_of_Ahnqiraj
SD%Complete:
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_ruins_of_ahnqiraj.h"

instance_ruins_of_ahnqiraj::instance_ruins_of_ahnqiraj(Map *map) : ScriptedInstance(map), ArmyDelayTimer(0), CurrentArmyWave(0)
{ Initialize(); };

void instance_ruins_of_ahnqiraj::Initialize()
{
    KurinaxxGUID = 0;
    BuruGUID = 0;
    GeneralRajaxGUID = 0;
    for(uint8 i = 0; i < MAX_ENCOUNTERS; i++)
        Encounters[i] = NOT_STARTED;
}

bool instance_ruins_of_ahnqiraj::IsEncounterInProgress() const
{
    for(uint8 i = 0; i < MAX_ENCOUNTERS; i++)
        if(Encounters[i] == IN_PROGRESS)
            return true;

    return false;
}

uint32 instance_ruins_of_ahnqiraj::GetEncounterForEntry(uint32 entry)
{
    switch(entry)
    {
        case 15348:
            return DATA_KURINNAXX;
        case 15341:
            return DATA_GENERAL_RAJAXX;
        case 15340:
            return DATA_MOAM;
        case 15370:
            return DATA_BURU_THE_GORGER;
        case 15369:
            return DATA_AYAMISS_THE_HUNTER;
        case 15339:
            return DATA_OSSIRIAN_THE_UNSCARRED;
        default:
            return 0; // Kurinax :]
    }
}

void instance_ruins_of_ahnqiraj::OnCreatureCreate(Creature *creature, uint32 creature_entry)
{
    switch(creature_entry)
    {
        case 15348:
            KurinaxxGUID = creature->GetGUID();
            break;
        case 15370:
            BuruGUID = creature->GetGUID();
            break;
        case NPC_GENERAL_ANDOROV: // General Andorov
            if(GetData(DATA_KURINNAXX) != DONE)
            {
                if(creature->GetVisibility() != VISIBILITY_OFF)
                    creature->SetVisibility(VISIBILITY_OFF);
            }
            else if ((GetData(DATA_KURINNAXX) == DONE) && GetData(DATA_GENERAL_RAJAXX) != DONE)
            {
                Player* pPlayer = GetPlayerInMap();
                creature->AI()->EventPulse(pPlayer, 0);
            }
            break;
        case 15341:
            GeneralRajaxGUID = creature->GetGUID();
            /*if(!creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            {
                creature->SetReactState(REACT_PASSIVE);
                creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            }*/ // turned off due to bad scripts. doesnt activate a lot of the times. part 2/2
            break;          
        case NPC_KALDOREI_ELITE:
            KaldoreiGuidList.push_back(creature->GetGUID());
            break;
        default:
            break;
    }

    HandleInitCreatureState(creature);
}

void instance_ruins_of_ahnqiraj::OnCreatureDeath(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_COLONEL_ZERRAN:
        case NPC_MAJOR_PAKKON:
        case NPC_MAJOR_YEGGETH:
        case NPC_CAPTAIN_XURREM:
        case NPC_CAPTAIN_DRENN:
        case NPC_CAPTAIN_TUUBID:
        case NPC_CAPTAIN_QEEZ:
        case NPC_QIRAJI_WARRIOR:
        case NPC_SWARMGUARD_NEEDLER:
        {
            // If event isn't started by Andorov, return
            if (GetData(DATA_GENERAL_RAJAXX) != IN_PROGRESS)
                return;

                ArmyWavesGuids.erase(creature->GetGUID());

                // If all the soldiers from the current wave are dead, then send the next one
                if (ArmyWavesGuids.empty())
                {
                    if(Creature* pAndorov = instance->GetCreatureById(NPC_GENERAL_ANDOROV))
                        pAndorov->SetHealth(pAndorov->GetMaxHealth()); // we should reset his health on every way, otherwise he'll die fast and event will failed
                    DoSendNextArmyWave();
                }
            break;
        }
        case NPC_GENERAL_ANDOROV:
            CurrentArmyWave = 0;
            break;
    }
}

uint64 instance_ruins_of_ahnqiraj::GetData64(uint32 identifier)
{
    switch(identifier)
    {
        case DATA_KURINNAXX:
            return KurinaxxGUID;
        case DATA_BURU_THE_GORGER:
            return BuruGUID;
        case DATA_GENERAL_RAJAXX:
            return GeneralRajaxGUID;
        default:
            return 0;
    }
}

void instance_ruins_of_ahnqiraj::SetData(uint32 type, uint32 data)
{
    switch(type)
    {
        case DATA_KURINNAXX:
            if(data == DONE)
                DoSpawnAndorovIfCan();
            if(Encounters[0] != DONE)
                Encounters[0] = data;
            break;
        case DATA_GENERAL_RAJAXX:
            if (data == DONE)
            {
                if (Creature* pAndorov = instance->GetCreatureById(NPC_GENERAL_ANDOROV))
                {
                    if (pAndorov->isAlive())
                        pAndorov->SetFlag(UNIT_NPC_FLAGS, 0x00000082);
                }
            }
            if(Encounters[1] != DONE)
                Encounters[1] = data;
            break;
        case DATA_MOAM:
            if(Encounters[2] != DONE)
                Encounters[2] = data;
            break;
        case DATA_BURU_THE_GORGER:
            if(Encounters[3] != DONE)
                Encounters[3] = data;
            break;
        case DATA_AYAMISS_THE_HUNTER:
            if(Encounters[4] != DONE)
                Encounters[4] = data;
            break;
        case DATA_OSSIRIAN_THE_UNSCARRED:
            if(Encounters[5] != DONE)
                Encounters[5] = data;
            break;
    }

    if(data == DONE)
        SaveToDB();
}

uint32 instance_ruins_of_ahnqiraj::GetData(uint32 type)
{
    switch(type)
    {
        case DATA_KURINNAXX:
            return Encounters[0];
        case DATA_GENERAL_RAJAXX:
            return Encounters[1];
        case DATA_MOAM:
            return Encounters[2];
        case DATA_BURU_THE_GORGER:
            return Encounters[3];
        case DATA_AYAMISS_THE_HUNTER:
            return Encounters[4];
        case DATA_OSSIRIAN_THE_UNSCARRED:
            return Encounters[5];
    }
    return 0;
}

std::string instance_ruins_of_ahnqiraj::GetSaveData()
{
    OUT_SAVE_INST_DATA;

    std::ostringstream stream;
    stream << Encounters[0] << " ";
    stream << Encounters[1] << " ";
    stream << Encounters[2] << " ";
    stream << Encounters[3] << " ";
    stream << Encounters[4] << " ";
    stream << Encounters[5];

    OUT_SAVE_INST_DATA_COMPLETE;

    return stream.str();
}

Player* instance_ruins_of_ahnqiraj::GetPlayerInMap()
{
    Map::PlayerList const& players = instance->GetPlayers();

    if (!players.isEmpty())
    {
        for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            if (Player* plr = itr->getSource())
                return plr;
        }
    }

    debug_log("HSCR: Instance Ruins of AhnQiraj: GetPlayerInMap, but PlayerList is empty!");
    return NULL;
}
    
void instance_ruins_of_ahnqiraj::DoSpawnAndorovIfCan()
{
    Creature* pAndorov = instance->GetCreatureById(NPC_GENERAL_ANDOROV);

    if(!pAndorov)
        return;

    if(pAndorov->GetVisibility() == VISIBILITY_ON)
        return;

    Player* pPlayer = GetPlayerInMap();
    if (!pPlayer)
        return;

    pAndorov->AI()->EventPulse(pPlayer, 1);
}

void instance_ruins_of_ahnqiraj::Load(const char* in)
{
    if(!in)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(in);
    std::istringstream stream(in);
    stream >> Encounters[0] >> Encounters[1] >> Encounters[2]
         >> Encounters[3] >> Encounters[4] >> Encounters[5];
    for(uint8 i = 0; i < MAX_ENCOUNTERS; ++i)
        if(Encounters[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
            Encounters[i] = NOT_STARTED;
    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_ruins_of_ahnqiraj::Update(uint32 diff)
{
    if (GetData(DATA_GENERAL_RAJAXX) == IN_PROGRESS)
    {
        if (ArmyDelayTimer)
        {
            if (ArmyDelayTimer <= diff)
            {
                DoSendNextArmyWave();
                ArmyDelayTimer = 2*MINUTE*MILLISECONDS;
            }
            else
                ArmyDelayTimer -= diff;
        }
    }
}

void instance_ruins_of_ahnqiraj::InsertGuidInArmyList(uint64 CreatureGuid)
{
    ArmyWavesGuids.insert(CreatureGuid);
}

void instance_ruins_of_ahnqiraj::DoSendNextArmyWave()
{
    // The next army wave is sent into battle after 2 min or after the previous wave is finished
    if (GetData(DATA_GENERAL_RAJAXX) != IN_PROGRESS)
        return;

    Creature* pTemp = NULL;

    switch(CurrentArmyWave) // quez, tuubid, drenn, xurrem, yeggeth, pakkon, zerran
    {
        case 0:
        {
            pTemp = instance->GetCreatureById(NPC_CAPTAIN_TUUBID);
            break;
        }
        case 1:
        {
            pTemp = instance->GetCreatureById(NPC_CAPTAIN_DRENN);
            break;
        }
        case 2:
        {
            pTemp = instance->GetCreatureById(NPC_CAPTAIN_XURREM);
            break;
        }
        case 3:
        {
            pTemp = instance->GetCreatureById(NPC_MAJOR_YEGGETH);
            break;
        }
        case 4:
        {
            pTemp = instance->GetCreatureById(NPC_MAJOR_PAKKON);
            break;
        }
        case 5:
        {
            pTemp = instance->GetCreatureById(NPC_COLONEL_ZERRAN);
            break;
        }
        case 6:
        {
            pTemp = instance->GetCreatureById(NPC_RAJAXX);
            DoScriptText(SAY_INTRO, pTemp);
            break;
        }
        default:
            break;
    }

    if(pTemp)
    {
        if (!pTemp->isAlive())
            return;

        if(pTemp->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            pTemp->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

        pTemp->SetWalk(false);

        if(Map *map = pTemp->GetMap())
        {
            Map::PlayerList const &PlayerList = map->GetPlayers();
            for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* i_pl = i->getSource())
                {
                    if(pTemp->GetDistance2d(i_pl) <= 300 && !i_pl->isAlive())
                        if(pTemp->IsInCombat())
                            pTemp->SetInCombatWith(i_pl);
                        pTemp->AI()->AttackStart(i_pl);
                    if(Creature* pAndorov = instance->GetCreatureById(NPC_GENERAL_ANDOROV))
                    {
                        if(!pAndorov->IsInCombat())
                            pAndorov->SetInCombatWith(pTemp);
                        pAndorov->AI()->AttackStart(pTemp);
                    }
                }
            }
        }
        /*if(Unit* pTarget = pTemp->AI()->SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
        {
            if(!pTemp->IsInCombat())
                pTemp->SetInCombatWith(pTarget);
            pTemp->AI()->AttackStart(pTarget);
        }
        else
        {
            if(Unit* pAndorov = instance->GetCreatureById(NPC_GENERAL_ANDOROV))
            {
                if(!pTemp->IsInCombat())
                    pTemp->SetInCombatWith(pAndorov);
                pTemp->AI()->AttackStart(pAndorov);
            }
        }*/

        if (aArmySortingParameters[CurrentArmyWave])
        {
            if (Creature* pRajaxx = instance->GetCreatureById(NPC_RAJAXX))
                DoScriptText(aArmySortingParameters[CurrentArmyWave], pRajaxx);
        }
        ++CurrentArmyWave;
    }

    // on wowwiki it states that there were 3 min between the waves, but this was reduced in later patches
    ArmyDelayTimer = 2*MINUTE*MILLISECONDS;
}

InstanceData* GetInstanceData_instance_ruins_of_ahnqiraj(Map* map)
{
    return new instance_ruins_of_ahnqiraj(map);
}

void AddSC_instance_ruins_of_ahnqiraj()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_ruins_of_ahnqiraj";
    newscript->GetInstanceData = &GetInstanceData_instance_ruins_of_ahnqiraj;
    newscript->RegisterSelf();
}

