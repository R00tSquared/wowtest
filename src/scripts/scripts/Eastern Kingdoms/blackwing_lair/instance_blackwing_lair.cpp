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
SDName: Instance_Blackwing_Lair
SD%Complete: 0
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

#define ENCOUNTERS 8

/* Blackwing lair encounters:
0 - Razorgore the Untamed event
1 - Vaelastrasz the Corrupt Event
2 - Broodlord Lashlayer Event
3 - Firemaw Event
4 - Ebonroc Event
5 - Flamegor Event
6 - Chromaggus Event
7 - Nefarian Event
*/
static const uint32 RazorgoreSpawns[MAX_EGGS_DEFENDERS] = {NPC_BLACKWING_LEGIONNAIRE, NPC_BLACKWING_MAGE, NPC_DRAGONSPAWN, NPC_DRAGONSPAWN};

struct instance_blackwing_lair : public ScriptedInstance
{
    instance_blackwing_lair(Map *map) : ScriptedInstance(map) {Initialize();};

    uint32 Encounters[ENCOUNTERS];
    uint32 ResetTimer;
    uint32 DefenseTimer;
    uint64 OrbOfDominationGuid;
    std::list<uint64> DragonEggsGuids;
    std::list<uint64> UsedEggsGuids;
    std::list<uint64> DefendersGuids;
    std::vector<uint64> GeneratorGuids;

    void Initialize()
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounters[i] = NOT_STARTED;
        OrbOfDominationGuid = 0;
        ResetTimer = 0;
        DefenseTimer = 0;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            if(Encounters[i] != NOT_STARTED && Encounters[i] != DONE)
                return true;
        return false;
    }

    void OnObjectCreate(GameObject *pGo)
    {
        switch(pGo->GetEntry())
        {
            case GO_ORB_OF_DOMINATION:
                OrbOfDominationGuid = pGo->GetGUID();
                break;
            case GO_BLACK_DRAGON_EGG:
                DragonEggsGuids.push_back(pGo->GetGUID());
                return;
            default:
                return;
        }
    }

    uint32 GetEncounterForEntry(uint32 entry)
    {
        switch(entry)
        {
            case 12435:
                return DATA_RAZORGORE;
            case 13020:
                return DATA_VAELASTRASZ_THE_CORRUPT_EVENT;
            case 12017:
                return DATA_BROODLORD_LASHLAYER_EVENT;
            case 11983:
                return DATA_FIREMAW_EVENT;
            case 14601:
                return DATA_EBONROC_EVENT;
            case 11981:
                return DATA_FLAMEGOR_EVENT;
            case 14020:
                return DATA_CHROMAGGUS_EVENT;
            case 11583:
            case 10162:
                return DATA_NEFARIAN_EVENT;
            default:
                return 0;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch (creature->GetEntry())
        {
            case NPC_MONSTER_GENERATOR:
                GeneratorGuids.push_back(creature->GetGUID());
                break;
            case NPC_BLACKWING_LEGIONNAIRE:
            case NPC_BLACKWING_MAGE:
            case NPC_DRAGONSPAWN:
                DefendersGuids.push_back(creature->GetGUID());
                break;
            default: break;
        }

        const CreatureData *tmp = creature->GetLinkedRespawnCreatureData();
        if (!tmp)
            return;

        if (GetEncounterForEntry(tmp->id) && creature->isAlive() && GetData(GetEncounterForEntry(tmp->id)) == DONE)
        {
            creature->setDeathState(JUST_DIED);
            creature->RemoveCorpse();
        }
    }

    void OnCreatureDeath(Creature* pCreature)
    {
        if (pCreature->GetEntry() == NPC_GRETHOK_CONTROLLER)
        {
            // Allow orb to be used
            if (GameObject* pOrb = instance->GetGameObject(OrbOfDominationGuid))
                pOrb->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
    
            if (Creature* pOrbTrigger = instance->GetCreatureById(NPC_BLACKWING_ORB_TRIGGER))
                pOrbTrigger->InterruptNonMeleeSpells(false);
        }
    }

    void SetData64(uint32 type, uint64 data)
    {
        if (type == DATA_DRAGON_EGG)
        {
            if (GameObject* pEgg = instance->GetGameObject(data))
                UsedEggsGuids.push_back(pEgg->GetGUID());

            // If all eggs are destroyed, then allow Razorgore to be attacked
            if (UsedEggsGuids.size() == DragonEggsGuids.size())
            {
                SetData(DATA_RAZORGORE, SPECIAL);

                if (GameObject* pOrb = instance->GetGameObject(OrbOfDominationGuid))
                    pOrb->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);

                // Emote for the start of the second phase
                if (Creature* pTrigger = instance->GetCreatureById(NPC_NEFARIANS_TROOPS))
                {
                    DoScriptText(EMOTE_ORB_SHUT_OFF, pTrigger); // release maybe with some cd?
                    DoScriptText(EMOTE_TROOPS_FLEE, pTrigger);
                }
    
                // Break mind control and set max health
                if (Creature* pRazorgore = instance->GetCreatureById(NPC_RAZORGORE))
                {
                    pRazorgore->RemoveAllAurasNotCreatureAddon();
                    pRazorgore->SetHealth(pRazorgore->GetMaxHealth());
                }
    
                // All defenders evade and despawn
                for (std::list<uint64>::const_iterator itr = DefendersGuids.begin(); itr != DefendersGuids.end(); ++itr)
                {
                    if (Creature* pDefender = instance->GetCreature(*itr))
                    {
                        pDefender->AI()->EnterEvadeMode();
                        pDefender->ForcedDespawn(5000);
                    }
                }
            }
        }
    }

    uint64 GetData64(uint32 identifier)
    {
        switch(identifier)
        {
            case 0:
                return 0;
            default:
                return 0;
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case DATA_RAZORGORE:
                if(Encounters[0] != DONE)
                {
                    if(data == IN_PROGRESS)
                        DefenseTimer = 40000;
                    else if (data == FAIL)
                    {
                        ResetTimer = 2000;
                        // Reset the Orb of Domination and the eggs
                        if (GameObject* pOrb = instance->GetGameObject(OrbOfDominationGuid))
                            pOrb->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                        // Reset defenders
                        for (std::list<uint64>::const_iterator itr = DefendersGuids.begin(); itr != DefendersGuids.end(); ++itr)
                        {
                            if (Creature* pDefender = instance->GetCreature(*itr))
                                pDefender->ForcedDespawn();
                        }
                        DefenseTimer = 0;

                        UsedEggsGuids.clear();
                        DefendersGuids.clear();
                    }
                    Encounters[0] = data;
                }
                break;
            case DATA_VAELASTRASZ_THE_CORRUPT_EVENT:
                if(Encounters[1] != DONE)
                    Encounters[1] = data;
                break;
            case DATA_BROODLORD_LASHLAYER_EVENT:
                if(Encounters[2] != DONE)
                    Encounters[2] = data;
                break;
            case DATA_FIREMAW_EVENT:
                if(Encounters[3] != DONE)
                    Encounters[3] = data;
                break;
            case DATA_EBONROC_EVENT:
                if(Encounters[4] != DONE)
                    Encounters[4] = data;
                break;
            case DATA_FLAMEGOR_EVENT:
                if(Encounters[5] != DONE)
                    Encounters[5] = data;
                break;
            case DATA_CHROMAGGUS_EVENT:
                if(Encounters[6] != DONE)
                    Encounters[6] = data;
                break;
            case DATA_NEFARIAN_EVENT:
                if(Encounters[7] != DONE)
                    Encounters[7] = data;
                break;
        }

        if(data == DONE)
            SaveToDB();
    }

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
            case DATA_RAZORGORE:
                return Encounters[0];
            case DATA_VAELASTRASZ_THE_CORRUPT_EVENT:
                return Encounters[1];
            case DATA_BROODLORD_LASHLAYER_EVENT:
                return Encounters[2];
            case DATA_FIREMAW_EVENT:
                return Encounters[3];
            case DATA_EBONROC_EVENT:
                return Encounters[4];
            case DATA_FLAMEGOR_EVENT:
                return Encounters[5];
            case DATA_CHROMAGGUS_EVENT:
                return Encounters[6];
            case DATA_NEFARIAN_EVENT:
                return Encounters[7];
        }
        return 0;
    }

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
        stream << Encounters[7];

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
        stream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3]
        >> Encounters[4] >> Encounters[5] >> Encounters[6] >> Encounters[7];
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                Encounters[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void Update (uint32 diff)
    {
        // Reset Razorgore in case of wipe
        if (ResetTimer)
        {
            if (ResetTimer <= diff)
            {
                if (Creature* pRazorgore = instance->GetCreatureById(NPC_RAZORGORE))
                {
                    if (!pRazorgore->isAlive())
                        pRazorgore->Respawn();
                }

                if (Creature* pGrethok = instance->GetCreatureById(NPC_GRETHOK_CONTROLLER))
                {
                    if (!pGrethok->isAlive())
                        pGrethok->Respawn();
                }
    
                // Respawn the Dragon Eggs
                for (std::list<uint64>::const_iterator itr = DragonEggsGuids.begin(); itr != DragonEggsGuids.end(); ++itr)
                {
                    if (GameObject* pEgg = instance->GetGameObject(*itr))
                    {
                        if (!pEgg->isSpawned())
                            pEgg->Respawn();

                        if (pEgg->GetGoState() != GO_STATE_READY)
                            pEgg->SetGoState(GO_STATE_READY);

                        pEgg->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                    }
                }
                SetData(DATA_RAZORGORE, NOT_STARTED);
                ResetTimer = 0;
            }
            else
                ResetTimer -= diff;
        }
    
        if (GetData(DATA_RAZORGORE) != IN_PROGRESS)
            return;

        if(DefenseTimer)
        {
            if (DefenseTimer <= diff)
            {
                // Allow Razorgore to spawn the defenders
                Creature* pRazorgore = instance->GetCreatureById(NPC_RAZORGORE);
                if (!pRazorgore)
                    return;

                std::random_shuffle(GeneratorGuids.begin(), GeneratorGuids.end());
                // Spawn the defenders
                for (uint8 i = 0; i < MAX_EGGS_DEFENDERS; ++i)
                {
                    if(Creature* pGenerator = instance->GetCreature(GeneratorGuids[i]))
                        pRazorgore->SummonCreature(RazorgoreSpawns[i], pGenerator->GetPositionX(), pGenerator->GetPositionY(), pGenerator->GetPositionZ(), pGenerator->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                }
        
                DefenseTimer = 20000;
            }
            else
                DefenseTimer -= diff;
        }
    }
};

InstanceData* GetInstanceData_instance_blackwing_lair(Map* map)
{
    return new instance_blackwing_lair(map);
}

void AddSC_instance_blackwing_lair()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "instance_blackwing_lair";
    newscript->GetInstanceData = &GetInstanceData_instance_blackwing_lair;
    newscript->RegisterSelf();
}

