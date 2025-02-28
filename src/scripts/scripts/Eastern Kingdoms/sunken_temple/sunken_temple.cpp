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

#include "precompiled.h"

enum
{
    // atalalarion event
    GOB_STATUE_START                = 148830, // to 148835, should be used in same order as their ids
    GOB_ALTAR                       = 148836,
    GOB_IDOL                        = 148838,
    // hakkar event
    GOB_EVIL_CIRCLE                 = 148998, // Objects used at the avatar event. they are spawned when the event starts, and the mobs are summon atop of them
    GOB_HAKKAR_DOOR_1               = 149432, // Combat doors
    GOB_HAKKAR_DOOR_2               = 149433,
    GOB_ETERNAL_FLAME_1             = 148418,
    GOB_ETERNAL_FLAME_2             = 148419,
    GOB_ETERNAL_FLAME_3             = 148420,
    GOB_ETERNAL_FLAME_4             = 148421,
    GOB_JAMMALAN_BARRIER            = 149431,

    CREATURE_ATALALARION            = 8580,
    CREATURE_AVATAR_OF_HAKKAR       = 8443,
    // Avatar of hakkar mobs
    CREATURE_SHADE_OF_HAKKAR        = 8440, // Shade of Hakkar appears when the event starts; will despawn when avatar of hakkar is summoned
    CREATURE_BLOODKEEPER            = 8438, // Spawned rarely and contains the hakkari blood -> used to extinguish the flames
    CREATURE_HAKKARI_MINION         = 8437, // Npc randomly spawned during the event = trash
    CREATURE_SUPPRESSOR             = 8497, // Npc summoned at one of the two doors and moves to the boss;
    // Jammalain
    NPC_JAMMALAN                    = 5710,
    NPC_ZOLO                        = 5712,
    NPC_GASHER                      = 5713,
    NPC_LORO                        = 5714,
    NPC_HUKKU                       = 5715,
    NPC_ZULLOR                      = 5716,
    NPC_MIJAN                       = 5717,
    // eranikus
    NPC_SHADE_OF_ERANIKUS           = 5709,
    NPC_DREAMSCYTH                  = 5721,
    NPC_WEAVER                      = 5720,

    DATA_STATUES                    = 1,
    DATA_AVATAR                     = 2,
    TYPE_PROTECTORS                 = 3,
    TYPE_JAMMALAN                   = 4,
    TYPE_ERANIKUS                   = 5,
    MAX_ENCOUNTER                   = 4,

    MAX_FLAMES                      = 4,

    SPELL_SUMMON_AVATAR             = 12639, // Cast by the shade of hakkar, updates entry to avatar
    SPELL_AVATAR_SUMMONED           = 12948,
    SPELL_SPIRIT_SPAWN_IN           = 17321,

    SAY_AVATAR_BRAZIER_1  = -1109006,
    SAY_AVATAR_BRAZIER_2  = -1109007,
    SAY_AVATAR_BRAZIER_3  = -1109008,
    SAY_AVATAR_BRAZIER_4  = -1109009,
    SAY_AVATAR_SPAWN      = -1109010,
};

float spawnpos[4] = { -479.936, 95.06, -189.729, 4.607 };
float spawnposmob[4] = { -448.935, 95.171, -189.729, 3.102 };

struct instance_sunken_temple : public ScriptedInstance
{
    instance_sunken_temple(Map *map) : ScriptedInstance(map)
    {
        Statues = 0;
        FlameCounter = 0;
        AvatarSummonTimer = 0;
        SupressorTimer = 0;
        ProtectorsRemaining = 0;
        IsFirstHakkarWave = false;
        CanSummonBloodkeeper = false;
        for(uint8 i = 0; i < MAX_ENCOUNTER; i++)
            Encounter[i] = NOT_STARTED;
    };

    uint32 Encounter[MAX_ENCOUNTER];
    // Atalalarion event
    uint8 Statues; // 0-5 clicking, 6-all clicked boss and obj spawned, 7-boss dead
    uint64 altarguid;
    uint64 smallLightGUID[6];
    std::list<uint64> BigLightGUIDList;
    std::list<uint64> SmallLightGUIDList;
    // Avatar of Hakkar Event
    uint8 FlameCounter;
    uint32 AvatarSummonTimer;
    uint32 SupressorTimer;
    uint64 Door1Guid;
    uint64 Door2Guid;
    bool IsFirstHakkarWave;
    bool CanSummonBloodkeeper;
    std::list<uint64> FlameGUIDs;
    std::vector<uint64> CircleGUIDs;
    // Jamallan
    uint64 JammalanGUID;
    uint64 DreamscythGUID;
    uint64 WeaverGUID;
    uint32 ProtectorsRemaining;
    uint64 BarrierGUID;
    // Eranikus
    uint64 ShadeEranikusGUID;

    void OnGameObjectCreate(GameObject *go, bool add)
    {
        switch(go->GetDBTableGUIDLow())
        {
            case 112831:
                smallLightGUID[0] = go->GetGUID();
                SmallLightGUIDList.push_back(go->GetGUID());
                break;
            case 112832:
                smallLightGUID[1] = go->GetGUID();
                SmallLightGUIDList.push_back(go->GetGUID());
                break;
            case 112833:
                smallLightGUID[2] = go->GetGUID();
                SmallLightGUIDList.push_back(go->GetGUID());
                break;
            case 112834:
                smallLightGUID[3] = go->GetGUID();
                SmallLightGUIDList.push_back(go->GetGUID());
                break;
            case 112835:
                smallLightGUID[4] = go->GetGUID();
                SmallLightGUIDList.push_back(go->GetGUID());
                break;
            case 112836:
                smallLightGUID[5] = go->GetGUID();
                SmallLightGUIDList.push_back(go->GetGUID());
                break;
            default: break;
        }

        switch(go->GetEntry())
        {
            case 148937:
                BigLightGUIDList.push_back(go->GetGUID());
                break;
            case GOB_ALTAR:
            {
                if (add)
                {
                    altarguid = go->GetGUID();
                    if (Statues > 5)
                    {
                        go->SummonGameObject(GOB_IDOL, spawnpos[0], spawnpos[1], spawnpos[2], spawnpos[3], 0, 0, 0, 0, 5000);
                        for (std::list<uint64>::iterator itr = BigLightGUIDList.begin(); itr != BigLightGUIDList.end(); itr++)
                        {
                            if (GameObject* goBigLight = instance->GetGameObject(*itr))
                            {
                                goBigLight->SetRespawnTime(30000);
                                goBigLight->Refresh();
                            }
                        }
                    }
                    if (Statues == 6)
                    {
                        if(Creature* atalalarion = go->SummonCreature(CREATURE_ATALALARION, spawnposmob[0], spawnposmob[1], spawnposmob[2], spawnposmob[3], TEMPSUMMON_DEAD_DESPAWN, 60000))
                            atalalarion->GetMotionMaster()->MovePoint(1, -475.704, 95.131, -179.729);
                    }
                }
                break;
            }
            case GOB_ETERNAL_FLAME_1:
            case GOB_ETERNAL_FLAME_2:
            case GOB_ETERNAL_FLAME_3:
            case GOB_ETERNAL_FLAME_4:
                FlameGUIDs.push_back(go->GetGUID());
                break;
            case GOB_EVIL_CIRCLE:
                CircleGUIDs.push_back(go->GetGUID());
                break;
            case GOB_HAKKAR_DOOR_1:
                Door1Guid = go->GetGUID();
                break;
            case GOB_HAKKAR_DOOR_2:
                Door2Guid = go->GetGUID();
                break;
            case GOB_JAMMALAN_BARRIER:
                if (Encounter[TYPE_PROTECTORS] == DONE)
                    go->SetGoState(GO_STATE_ACTIVE);
                BarrierGUID = go->GetGUID();
            break;
            default: break;
        }
    }

    void OnCreatureEvade(Creature* pCreature)
    {
        switch(pCreature->GetEntry())
        {
            // Hakkar Event Mobs: On Wipe set as failed!
            case CREATURE_BLOODKEEPER:
            case CREATURE_HAKKARI_MINION:
            case CREATURE_SUPPRESSOR:
            case CREATURE_AVATAR_OF_HAKKAR:
            case CREATURE_SHADE_OF_HAKKAR:
                SetData(DATA_AVATAR, FAIL);
                if (GameObject* goone = instance->GetGameObject(Door1Guid))
                    goone->SetGoState(GO_STATE_ACTIVE);
                if (GameObject* gotwo = instance->GetGameObject(Door2Guid))
                    gotwo->SetGoState(GO_STATE_ACTIVE);
                break;
        }
    }

    void OnCreatureDeath(Creature* pCreature)
    {
        switch (pCreature->GetEntry())
        {
            case CREATURE_AVATAR_OF_HAKKAR:
                SetData(DATA_AVATAR, DONE);
                if (GameObject* goone = instance->GetGameObject(Door1Guid))
                    goone->SetGoState(GO_STATE_ACTIVE);
                if (GameObject* gotwo = instance->GetGameObject(Door2Guid))
                    gotwo->SetGoState(GO_STATE_ACTIVE);
                break;

            case CREATURE_SUPPRESSOR:
                CanSummonBloodkeeper = true;
                break;
            // Jammalain mini-bosses
            case NPC_ZOLO:
            case NPC_GASHER:
            case NPC_LORO:
            case NPC_HUKKU:
            case NPC_ZULLOR:
            case NPC_MIJAN:
                SetData(TYPE_PROTECTORS, DONE);
                break;
            case NPC_JAMMALAN:
                SetData(TYPE_JAMMALAN, DONE);
                break;
        }
    }

    void OnCreatureCreate(Creature* pCreature, uint32 entry)
    {
        switch(pCreature->GetEntry())
        {
            case NPC_ZOLO:
            case NPC_GASHER:
            case NPC_LORO:
            case NPC_HUKKU:
            case NPC_ZULLOR:
            case NPC_MIJAN:
                ++ProtectorsRemaining;
                break;
            case NPC_JAMMALAN:
                JammalanGUID = pCreature->GetGUID();
                break;
            case NPC_SHADE_OF_ERANIKUS:
                ShadeEranikusGUID = pCreature->GetGUID();
                if (Encounter[2] != DONE)      // TYPE_JAMMALAN
                {
                    pCreature->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, true);
                    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                    pCreature->SetStandState(UNIT_STAND_STATE_SLEEP);
                }
                break;
            case NPC_DREAMSCYTH:
                DreamscythGUID = pCreature->GetGUID();
                if (Encounter[2] != DONE)      // TYPE_JAMMALAN
                {
                    pCreature->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, true);
                    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                    pCreature->SetStandState(UNIT_STAND_STATE_SLEEP);
                }
                break;
            case NPC_WEAVER:
                WeaverGUID = pCreature->GetGUID();
                if (Encounter[2] != DONE)      // TYPE_JAMMALAN
                {
                    pCreature->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, true);
                    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                    pCreature->SetStandState(UNIT_STAND_STATE_SLEEP);
                }
                break;
        }
    }

    void DoUpdateFlamesFlags(bool Restore)
    {
        for (std::list<uint64>::iterator itr = FlameGUIDs.begin(); itr != FlameGUIDs.end(); itr++)
        {
            if (GameObject* goFlame = instance->GetGameObject(*itr))
            {
                if (Restore)
                    goFlame->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                else
                {
                    if(GetData(DATA_AVATAR) != DONE)
                        goFlame->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                }
            }
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

        debug_log("Instance Sunken Temple: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case DATA_STATUES:
            {
                if (data == IN_PROGRESS)
                {
                    Statues++;
                    if(Statues == 1)
                    {
                        if (GameObject* goSmallLight = instance->GetGameObject(smallLightGUID[0]))
                        {
                            goSmallLight->SetRespawnTime(30000);
                            goSmallLight->Refresh();
                            // goSmallLight->Respawn();
                        }
                    }
                    if(Statues == 2)
                    {
                        if (GameObject* goSmallLight = instance->GetGameObject(smallLightGUID[1]))
                        {
                            goSmallLight->SetRespawnTime(30000);
                            goSmallLight->Refresh();
                            // goSmallLight->Respawn();
                        }
                    }
                    if(Statues == 3)
                    {
                        if (GameObject* goSmallLight = instance->GetGameObject(smallLightGUID[2]))
                        {
                            goSmallLight->SetRespawnTime(30000);
                            goSmallLight->Refresh();
                            // goSmallLight->Respawn();
                        }
                    }
                    if(Statues == 4)
                    {
                        if (GameObject* goSmallLight = instance->GetGameObject(smallLightGUID[3]))
                        {
                            goSmallLight->SetRespawnTime(30000);
                            goSmallLight->Refresh();
                            // goSmallLight->Respawn();
                        }
                    }
                    if(Statues == 5)
                    {
                        if (GameObject* goSmallLight = instance->GetGameObject(smallLightGUID[4]))
                        {
                            goSmallLight->SetRespawnTime(30000);
                            goSmallLight->Refresh();
                            // goSmallLight->Respawn();
                        }
                    }
                    if (Statues == 6)
                    {
                        if (GameObject* goSmallLight = instance->GetGameObject(smallLightGUID[5]))
                        {
                            goSmallLight->SetRespawnTime(30000);
                            goSmallLight->Refresh();
                            // goSmallLight->Respawn();
                        }

                        if (GameObject* go = instance->GetGameObject(altarguid))
                        {
                            if(Creature* atalalarion = go->SummonCreature(CREATURE_ATALALARION, spawnposmob[0], spawnposmob[1], spawnposmob[2], spawnposmob[3], TEMPSUMMON_DEAD_DESPAWN, 60000))
                                atalalarion->GetMotionMaster()->MovePoint(1, -475.704, 95.131, -179.729);

                            go->SummonGameObject(GOB_IDOL, spawnpos[0], spawnpos[1], spawnpos[2], spawnpos[3], 0, 0, 0, 0, 5000);

                            for (std::list<uint64>::iterator itr = BigLightGUIDList.begin(); itr != BigLightGUIDList.end(); itr++)
                            {
                                if (GameObject* goBigLight = instance->GetGameObject(*itr))
                                {
                                    goBigLight->SetRespawnTime(30000);
                                    goBigLight->Refresh();
                                }
                            }
                        }
                    }
                }
                else if (data == FAIL) // wrong statue used
                {
                    Statues = 0;
                    for (std::list<uint64>::iterator itr = SmallLightGUIDList.begin(); itr != SmallLightGUIDList.end(); itr++)
                    {
                        if (GameObject* goSmallLight = instance->GetGameObject(*itr))
                        {
                            goSmallLight->SetRespawnTime(1);
                            goSmallLight->Refresh();
                            // goSmallLight->SetLootState(GO_JUST_DEACTIVATED);
                        }
                    }
                }
                else if (data == DONE) // atalalalala killed
                    Statues = 7;
                break;
            }
            case DATA_AVATAR:
            {
                if (data == SPECIAL)
                {
                    ++FlameCounter;

                    Creature* pShade = instance->GetCreatureById(CREATURE_SHADE_OF_HAKKAR);
                    if (!pShade)
                        return;

                    switch (FlameCounter)
                    {
                        // Yells on each flame
                        // TODO It might be possible that these yells should be ordered randomly, however this is the seen state
                        case 1: DoScriptText(SAY_AVATAR_BRAZIER_1, pShade); break;
                        case 2: DoScriptText(SAY_AVATAR_BRAZIER_2, pShade); break;
                        case 3: DoScriptText(SAY_AVATAR_BRAZIER_3, pShade); break;
                        // Summon the avatar of all flames are used
                        case MAX_FLAMES:
                            DoScriptText(SAY_AVATAR_BRAZIER_4, pShade);
                            pShade->CastSpell(pShade, SPELL_SUMMON_AVATAR, true);
                            AvatarSummonTimer = 0;
                            SupressorTimer = 0;
                            break;
                    }

                    // Summon the suppressors only after the flames are doused
                    // Summon timer is confusing random; timers were: 13, 39 and 52 secs;
                    if (FlameCounter != MAX_FLAMES)
                        SupressorTimer = urand(15000, 45000);

                    return;
                }

                // Prevent double processing
                if (Encounter[type] == data)
                    return;

                if (data == IN_PROGRESS)
                {
                    // Use combat doors
                    if (GameObject* goone = instance->GetGameObject(Door1Guid))
                        goone->UseDoorOrButton();
                    if (GameObject* gotwo = instance->GetGameObject(Door2Guid))
                        gotwo->UseDoorOrButton();

                    SupressorTimer = 0;
                    DoUpdateFlamesFlags(false);

                    // Summon timer; use a small delay
                    AvatarSummonTimer = 3000;
                    IsFirstHakkarWave = true;

                    // Summon the shade
                    Player* pPlayer = GetPlayerInMap();
                    if (!pPlayer)
                        return;

                    if (Creature* pShade = pPlayer->SummonCreature(CREATURE_SHADE_OF_HAKKAR, -466.8673, 272.31204, -90.7441, 3.5255, TEMPSUMMON_MANUAL_DESPAWN, 0))
                    {
                        pShade->SetRespawnDelay(DAY);
                    }

                    // Respawn circles
                    for (std::vector<uint64>::iterator itr = CircleGUIDs.begin(); itr != CircleGUIDs.end(); itr++)
                    {
                        if (GameObject* goCircle = instance->GetGameObject(*itr))
                        {
                            goCircle->SetRespawnTime(30 * MINUTE);
                            goCircle->Refresh();
                        }
                    }
                }
                else if (data == FAIL)
                {
                    // In case of wipe during the summoning ritual the shade is despawned
                    // The trash mobs stay in place, they are not despawned; the avatar is not sure if it's despawned or not but most likely he'll stay in place

                    // Despawn the shade and the avatar if needed -- TODO, avatar really?
                    if (Creature* pShade = instance->GetCreatureById(CREATURE_SHADE_OF_HAKKAR))
                        pShade->ForcedDespawn();

                    // Reset flames
                    DoUpdateFlamesFlags(true);
                }

                Encounter[type] = data;

                break;
            }
            case TYPE_PROTECTORS:
            {
                if (data == DONE)
                {
                    --ProtectorsRemaining;
                    if (!ProtectorsRemaining)
                    {
                        Encounter[type] = data;
                        if (GameObject* goBarrier = instance->GetGameObject(BarrierGUID))
                            goBarrier->SetGoState(GO_STATE_ACTIVE);
                    }
                }
                break;
            }
            case TYPE_JAMMALAN:
            {
                Encounter[2] = data;
                if (data == DONE)
                {
                    if (Creature* pEranikus = instance->GetCreature(ShadeEranikusGUID))
                    {
                        pEranikus->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, false);
                        pEranikus->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                    }
                    if (Creature* pDream = instance->GetCreature(DreamscythGUID))
                    {
                        pDream->SetVisibility(VISIBILITY_ON);
                        pDream->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, false);
                        pDream->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                        //pDream->GetMotionMaster()->MoveWaypoint();
                        // DoScriptText(SAY_DREAMSCYTHE_INTRO, pDream);
                    }
                    if (Creature* pWeav = instance->GetCreature(WeaverGUID))
                    {
                        pWeav->SetVisibility(VISIBILITY_ON);
                        pWeav->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, false);
                        pWeav->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                        //pWeav->GetMotionMaster()->MoveWaypoint();
                    }
                }
                else if (data == IN_PROGRESS)
                {
                    Creature* pJam = instance->GetCreature(JammalanGUID);
                    if (!pJam)
                        break;

                    std::vector<uint32> mobsEntries;
                    std::vector<uint32>::iterator entriesIt;
                    mobsEntries.push_back(5263);    // Mummified Atal'ai
                    mobsEntries.push_back(5271);    // Atal'ai Deathwalker
                    mobsEntries.push_back(5273);    // Atal'ai High Priest

                    for (entriesIt = mobsEntries.begin(); entriesIt != mobsEntries.end(); ++entriesIt)
                    {
                        std::list<Creature*> tmpMobsList;
                        Hellground::AllCreaturesOfEntryInRange check(pJam, (*entriesIt), 150.0f);
                        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(tmpMobsList, check);
                        Cell::VisitGridObjects(pJam, searcher, 150.0f);
                        while (!tmpMobsList.empty())
                        {
                            Creature* curr = tmpMobsList.front();
                            tmpMobsList.pop_front();
                            // Creature invoquee
                            if (!curr->GetDBTableGUIDLow())
                                continue;

                            if (curr->isAlive())
                            {
                                curr->SetInCombatWithZone();
                                if(pJam->GetVictim())
                                    curr->AI()->AttackStart(pJam->GetVictim());
                            }
                        }
                    }
                    mobsEntries.clear();
                }
                break;
            }
            case TYPE_ERANIKUS:
            {
                Encounter[3] = data;
                if (data == IN_PROGRESS)
                {
                    Creature* pEranikus = instance->GetCreature(ShadeEranikusGUID);
                    if (!pEranikus)
                        break;

                    std::vector<uint32> mobsEntries;
                    std::vector<uint32>::iterator entriesIt;
                    mobsEntries.push_back(5277);    // Nightmare Scalebane
                    mobsEntries.push_back(5280);    // Nightmare Wyrmkin
                    mobsEntries.push_back(8319);    // Nightmare Whelp
                    mobsEntries.push_back(5283);    // Nightmare Wanderer

                    for (entriesIt = mobsEntries.begin(); entriesIt != mobsEntries.end(); ++entriesIt)
                    {
                        std::list<Creature*> tmpMobsList;
                        Hellground::AllCreaturesOfEntryInRange check(pEranikus, (*entriesIt), 300.0f);
                        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(tmpMobsList, check);
                        Cell::VisitGridObjects(pEranikus, searcher, 300.0f);

                        while (!tmpMobsList.empty())
                        {
                            Creature* curr = tmpMobsList.front();
                            tmpMobsList.pop_front();
                            // Creature invoquee
                            if (!curr->GetDBTableGUIDLow())
                                continue;

                            if (curr->isAlive())
                            {
                                curr->SetInCombatWithZone();
                                if(pEranikus->GetVictim())
                                    curr->AI()->AttackStart(pEranikus->GetVictim());
                            }
                        }
                    }
                    mobsEntries.clear();
                }
                break;
            }
            default: break;
        }
        if(data == DONE)
            SaveToDB();
    }

    uint32 GetData(uint32 data)
    {
        if (data == DATA_STATUES)
            return Statues;
        if (data == DATA_AVATAR)
            return Encounter[0];
        if (data == TYPE_PROTECTORS)
            return Encounter[1];
        if (data  == TYPE_JAMMALAN)
            return Encounter[2];
        if (data == TYPE_ERANIKUS)
            return Encounter[3];
        return 0;
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Statues << " ";
        stream << Encounter[0] << " ";
        stream << Encounter[1] << " ";
        stream << Encounter[2] << " ";
        stream << Encounter[3];

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

        std::istringstream stream(in);
        stream >> Statues >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3];

        for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (Encounter[i] == IN_PROGRESS) // Do not load an encounter as "In Progress" - reset it instead.
            Encounter[i] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void Update(uint32 diff)
    {
        if (Encounter[DATA_AVATAR] != IN_PROGRESS)
            return;
        
        // Summon random mobs around the circles
        if (AvatarSummonTimer)
        {
            if (AvatarSummonTimer <= diff)
            {
                Creature* pShade = instance->GetCreatureById(CREATURE_SHADE_OF_HAKKAR);
                if (!pShade)
                    return;

                // If no summon circles are spawned then return
                if (CircleGUIDs.empty())
                    return;

                if (IsFirstHakkarWave)                       // First wave summoned
                {
                    // Summon at all circles
                    for (std::vector<uint64>::iterator itr = CircleGUIDs.begin(); itr != CircleGUIDs.end(); itr++)
                    {
                        if (GameObject* goCircle = instance->GetGameObject(*itr))
                            pShade->SummonCreature(CREATURE_HAKKARI_MINION, goCircle->GetPositionX(), goCircle->GetPositionY(), goCircle->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                    }

                    // Summon Bloodkeeper at random circle
                    if (GameObject* goCircle = instance->GetGameObject(CircleGUIDs[urand(0, CircleGUIDs.size() - 1)]))
                        pShade->SummonCreature(CREATURE_BLOODKEEPER, goCircle->GetPositionX(), goCircle->GetPositionY(), goCircle->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);

                    CanSummonBloodkeeper = false;
                    IsFirstHakkarWave = false;
                    AvatarSummonTimer = 50000;
                }
                else                                            // Later wave
                {
                    uint32 Roll = urand(0, 99);
                    uint8 MaxSummons = Roll < 75 ? 1 : Roll < 95 ? 2 : 3;

                    if (CanSummonBloodkeeper && roll_chance_i(30))
                    {
                        // Summon a Bloodkeeper
                        if (GameObject* goCircle = instance->GetGameObject(CircleGUIDs[urand(0, CircleGUIDs.size() - 1)]))
                            pShade->SummonCreature(CREATURE_BLOODKEEPER, goCircle->GetPositionX(), goCircle->GetPositionY(), goCircle->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);

                        CanSummonBloodkeeper = false;
                        --MaxSummons;
                    }

                    for (uint8 i = 0; i < MaxSummons; ++i)
                    {
                        if (GameObject* goCircle = instance->GetGameObject(CircleGUIDs[urand(0, CircleGUIDs.size() - 1)]))
                            pShade->SummonCreature(CREATURE_HAKKARI_MINION, goCircle->GetPositionX(), goCircle->GetPositionY(), goCircle->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                    }
                    AvatarSummonTimer = urand(3000, 15000);
                }
            }
            else
                AvatarSummonTimer -= diff;
        }

        // Summon nightmare suppressor after flame used
        if (SupressorTimer)
        {
            if (SupressorTimer <= diff)
            {
                Creature* pShade = instance->GetCreatureById(CREATURE_SHADE_OF_HAKKAR);
                if (!pShade)
                {
                    // Something went very wrong!
                    return;
                }

                // Summon npc at random door; movement and script handled in DB
                uint8 SummonLoc = urand(0, 1);
                switch(SummonLoc)
                {
                    case 0:
                        if(Creature* suppressor = pShade->SummonCreature(CREATURE_SUPPRESSOR, -420.629, 276.682, -90.827, 0, TEMPSUMMON_DEAD_DESPAWN, 0))
                        {
                            suppressor->CastSpell(suppressor, 7741, true);
                            suppressor->GetMotionMaster()->MovePoint(2, -452.48, 275.606, -90.526);
                        }
                        break;
                    case 1:
                        if(Creature* suppressor = pShade->SummonCreature(CREATURE_SUPPRESSOR, -512.015, 276.134, -90.827, 0, TEMPSUMMON_DEAD_DESPAWN, 0))
                        {
                            suppressor->CastSpell(suppressor, 7741, true);
                            suppressor->GetMotionMaster()->MovePoint(2, -481.589, 275.7, -90.61);
                        }
                        break;
                    default: break;
                }

                // This timer is finished now
                SupressorTimer = 0;
            }
            else
                SupressorTimer -= diff;
        }
    }
};


InstanceData* GetInstanceData_instance_sunken_temple(Map* map)
{
    return new instance_sunken_temple(map);
}

bool GO_use_atalai_statue(Player* plr, GameObject* gob)
{
    InstanceData* inst = gob->GetInstanceData();
    if (!inst)
        return true;

    if (inst->GetData(DATA_STATUES) == (gob->GetEntry() - GOB_STATUE_START))
        inst->SetData(DATA_STATUES, IN_PROGRESS);
    else
    {
        inst->SetData(DATA_STATUES, FAIL);
        uint32 Roll = urand(0, 99);
        ((Unit*)plr)->AddAura(Roll < 50 ? 18949 : 18948, ((Unit*)plr));
    }

    return true;
}

bool GOUse_go_eternal_flame(Player* pPlayer, GameObject* pGo)
{
    InstanceData* inst = pGo->GetInstanceData();
    if (!inst)
        return true;

    /*if (inst->GetData(DATA_AVATAR) != IN_PROGRESS)
        return true;*/

    // Set data to special when flame is used
    inst->SetData(DATA_AVATAR, SPECIAL);
    pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);

    return true;
}

bool ProcessEventId_event_avatar_of_hakkar(uint32 /*uiEventId*/, Object* pSource, Object* /*pTarget*/, bool /*bIsStart*/)
{
    if (pSource->GetTypeId() == TYPEID_PLAYER)
    {
        if (InstanceData* pInstance = (InstanceData*)((Player*)pSource)->GetInstanceData())
        {
            // return if not NOT_STARTED
            if (pInstance->GetData(DATA_AVATAR) != NOT_STARTED)
                return true;

            if (pInstance->GetData(DATA_AVATAR) != DONE)
                pInstance->SetData(DATA_AVATAR, IN_PROGRESS);

            return true;
        }
    }
    return false;
}

bool EffectDummyCreature_summon_hakkar(Unit* pCaster, uint32 uiSpellId, uint32 uiEffIndex, Creature* /*pCreatureTarget*/)
{
    // Always check spellid and effectindex
    if (uiSpellId == SPELL_SUMMON_AVATAR && uiEffIndex == 0)
    {
        if (!pCaster || pCaster->GetTypeId() != TYPEID_UNIT)
            return true;

        // Update entry to avatar of Hakkar and cast some visuals
        ((Creature*)pCaster)->UpdateEntry(CREATURE_AVATAR_OF_HAKKAR);
        pCaster->CastSpell(pCaster, SPELL_AVATAR_SUMMONED, true);
        DoScriptText(SAY_AVATAR_SPAWN, pCaster);

        // Always return true when we are handling this spell and effect
        return true;
    }

    return false;
}

void AddSC_sunken_temple()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "instance_sunken_temple";
    newscript->GetInstanceData = &GetInstanceData_instance_sunken_temple;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "gob_atalai_statue";
    newscript->pGOUse = &GO_use_atalai_statue;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_eternal_flame";
    newscript->pGOUse = &GOUse_go_eternal_flame;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "event_avatar_of_hakkar";
    newscript->pProcessEventId = &ProcessEventId_event_avatar_of_hakkar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_shade_of_hakkar";
    newscript->pEffectDummyNPC = &EffectDummyCreature_summon_hakkar;
    newscript->RegisterSelf();
}