// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2006-2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
#include "def_blackrock_spire.h"

#define EMBERSEER_OUT               175153
#define BLACKROCK_ALTAR             175706
#define BLACKHAND_INCARCERATOR      10316
#define PYROGUARD_EMBERSEER         9816

static const DialogueEntry aStadiumDialogue[] =
{
    {NPC_LORD_VICTOR_NEFARIUS,  0,                          1000},
    {SAY_NEFARIUS_INTRO_1,      NPC_LORD_VICTOR_NEFARIUS,   7000},
    {SAY_NEFARIUS_INTRO_2,      NPC_LORD_VICTOR_NEFARIUS,   5000},
    {NPC_BLACKHAND_HANDLER,     0,                          0},
    {SAY_NEFARIUS_LOSE_4,       NPC_LORD_VICTOR_NEFARIUS,   3000},
    {SAY_REND_ATTACK,           NPC_REND_BLACKHAND,         2000},
    {SAY_NEFARIUS_WARCHIEF,     NPC_LORD_VICTOR_NEFARIUS,   0},
    {SAY_NEFARIUS_VICTORY,      NPC_LORD_VICTOR_NEFARIUS,   5000},
    {NPC_REND_BLACKHAND,        0,                          0},
    {0, 0, 0},
};

instance_blackrock_spire::instance_blackrock_spire(Map* map) : ScriptedInstance(map), DialogueHelper(aStadiumDialogue) {Initialize();}


void instance_blackrock_spire::Initialize()
{
    InitializeDialogueHelper(this);
    for(uint8 i=0; i < MAX_ENCOUNTERS; ++i)
        Encounters[i] = NOT_STARTED;

    // UBRS
    // 1.
    EmberseerInDoorGUID.clear();
    for (uint8 i = 0; i < MAX_ROOMS; ++i)
        RoomRuneGUID[i] = 0;
    RoomEventMobGUIDList.clear();
    for (uint8 i = 0; i < MAX_ROOMS; ++i)
        RoomEventMobGUIDSorted[i].clear();
    // 2.
    emberseerOut = 0;
    pyroguard_emberseerGUID = 0;
    runesEmberGUID.clear();
    channelersGUID.clear();
    // 3.
    FlamewreathEventTimer = 0;
    FlamewreathWaveCount = 0;
    GoFatherFlameGUID = 0;
    // 4.
    NefariusGUID = 0;
    GythGUID = 0;
    GythEntryDoorGUID = 0;
    GythCombatDoorGUID = 0;
    GythExitDoorGUID = 0;
    StadiumEventTimer = 0;
    StadiumWaves = 0;
    StadiumMobsAlive = 0;
}

void instance_blackrock_spire::OnObjectCreate(GameObject* go)
{
    switch(go->GetEntry())
    {
        case GO_ROOM_1_RUNE: RoomRuneGUID[0] = go->GetGUID(); break;
        case GO_ROOM_2_RUNE: RoomRuneGUID[1] = go->GetGUID(); break;
        case GO_ROOM_3_RUNE: RoomRuneGUID[2] = go->GetGUID(); break;
        case GO_ROOM_4_RUNE: RoomRuneGUID[3] = go->GetGUID(); break;
        case GO_ROOM_5_RUNE: RoomRuneGUID[4] = go->GetGUID(); break;
        case GO_ROOM_6_RUNE: RoomRuneGUID[5] = go->GetGUID(); break;
        case GO_ROOM_7_RUNE: RoomRuneGUID[6] = go->GetGUID(); break;

        case 175187:
        case 175267:
        case 175268:
        case 175269:
        case 175270:
        case 175271:
        case 175272:
            runesEmberGUID.insert(go->GetGUID());
            break;

        case GO_EMBERSEER_IN_DOOR:
            EmberseerInDoorGUID.insert(go->GetGUID());
            if(GetData(TYPE_ROOM_EVENT) == DONE)
                HandleGameObject(go->GetGUID(), true);
            break;

        case EMBERSEER_OUT:
            emberseerOut = go->GetGUID();
            if(GetData(DATA_EMBERSEER) == DONE)
                HandleGameObject(go->GetGUID(), true);
            break;
        case BLACKROCK_ALTAR:
            if(GetData(DATA_EMBERSEER) != NOT_STARTED)
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
            break;

        case GO_FATHER_FLAME:
            GoFatherFlameGUID = go->GetGUID();
            if (Encounters[TYPE_FLAMEWREATH] == DONE || Encounters[TYPE_FLAMEWREATH] == IN_PROGRESS)
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
            else
                go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
            break;

        case GO_GYTH_ENTRY_DOOR:
            GythEntryDoorGUID = go->GetGUID();
            if (GetData(TYPE_GYTH) != IN_PROGRESS)
                HandleGameObject(GythEntryDoorGUID, true);
            break;
        case GO_GYTH_COMBAT_DOOR:
            GythCombatDoorGUID = go->GetGUID();
            break;
        case GO_GYTH_EXIT_DOOR:
            GythExitDoorGUID = go->GetGUID();
            if (GetData(TYPE_GYTH) == DONE)
                HandleGameObject(GythExitDoorGUID, true);
            break;
        default:
            break;
    }
}

void instance_blackrock_spire::OnCreatureCreate(Creature *creature, uint32 entry)
{
    switch (entry)
    {
        case NPC_BLACKHAND_SUMMONER:
        case NPC_BLACKHAND_VETERAN:
            RoomEventMobGUIDList.push_back(creature->GetGUID());
            break;
        case PYROGUARD_EMBERSEER:
            pyroguard_emberseerGUID = creature->GetGUID();
            break;
        case BLACKHAND_INCARCERATOR:
            channelersGUID.insert(creature->GetGUID());
            break;
        case NPC_LORD_VICTOR_NEFARIUS:
            NefariusGUID = creature->GetGUID();
            break;
        case NPC_GYTH:
            GythGUID = creature->GetGUID();
            break;
        default:
            break;
    }
}

uint32 instance_blackrock_spire::GetData(uint32 type)
{
    switch(type)
    {
        case TYPE_ROOM_EVENT:
            return Encounters[0];
        case DATA_EMBERSEER:
            return Encounters[1];
        case DATA_THE_BEAST:
            return Encounters[2];
        case TYPE_FLAMEWREATH:
            return Encounters[3];
        case TYPE_GYTH:
            return Encounters[4];
        case TYPE_STADIUM:
            return Encounters[5];
    }

    return 0;
}

void instance_blackrock_spire::DoSortRoomEventMobs()
{
    if (GetData(TYPE_ROOM_EVENT) != NOT_STARTED)
        return;

    for (uint8 i = 0; i < MAX_ROOMS; ++i)
    {
        if (GameObject* pRune = instance->GetGameObject(RoomRuneGUID[i]))
        {
            for (std::list<uint64>::const_iterator itr = RoomEventMobGUIDList.begin(); itr != RoomEventMobGUIDList.end(); ++itr)
            {
                Creature* pCreature = instance->GetCreature(*itr);
                if (pCreature && pCreature->isAlive() && pCreature->GetDistance(pRune) < 10.0f)
                    RoomEventMobGUIDSorted[i].push_back(*itr);
            }
        }
    }
    SetData(TYPE_ROOM_EVENT, IN_PROGRESS);
}

void instance_blackrock_spire::OnCreatureDeath(Creature* creature)
{
    switch(creature->GetEntry())
    {
        case NPC_BLACKHAND_SUMMONER:
        case NPC_BLACKHAND_VETERAN:
            // Handle Runes
            if (Encounters[TYPE_ROOM_EVENT] == IN_PROGRESS)
            {
                uint8 NotEmptyRoomsCount = 0;
                for (uint8 i = 0; i < MAX_ROOMS; ++i)
                {
                    if (RoomRuneGUID[i])                 // This check is used, to ensure which runes still need processing
                    {
                        RoomEventMobGUIDSorted[i].remove(creature->GetGUID());
                        if (RoomEventMobGUIDSorted[i].empty())
                        {
                            if(GameObject* pRune = instance->GetGameObject(RoomRuneGUID[i]))
                                pRune->UseDoorOrButton();
                            RoomRuneGUID[i] = 0;
                        }
                        else
                            ++NotEmptyRoomsCount;         // found an not empty room
                    }
                }
                if (!NotEmptyRoomsCount)
                    SetData(TYPE_ROOM_EVENT, DONE);
            }
            break;
        case NPC_SOLAKAR_FLAMEWREATH:
            SetData(TYPE_FLAMEWREATH, DONE);
            break;
        case NPC_CHROMATIC_WHELP:
        case NPC_CHROMATIC_DRAGON:
        case NPC_BLACKHAND_HANDLER:
            // check if it's summoned - some npcs with the same entry are already spawned in the instance
            if (!creature->IsTemporarySummon())
                break;
            --StadiumMobsAlive;
            if (StadiumMobsAlive <= 0)
                DoSendNextStadiumWave();
            break;
        case NPC_GYTH:
        case NPC_REND_BLACKHAND:
            --StadiumMobsAlive;
            if (StadiumMobsAlive == 0)
                StartNextDialogueText(SAY_NEFARIUS_VICTORY);
            break;
    }
}

void instance_blackrock_spire::OnCreatureEvade(Creature* creature)
{
    switch(creature->GetEntry())
    {
        case NPC_SOLAKAR_FLAMEWREATH:
        case NPC_ROOKERY_GUARDIAN:
        case NPC_ROOKERY_HATCHER:
            SetData(TYPE_FLAMEWREATH, FAIL);
            break;
        case NPC_CHROMATIC_WHELP:
        case NPC_CHROMATIC_DRAGON:
        case NPC_BLACKHAND_HANDLER:
        case NPC_GYTH:
        case NPC_REND_BLACKHAND:
            // check if it's summoned - some npcs with the same entry are already spawned in the instance
            if (!creature->IsTemporarySummon())
                break;
            SetData(TYPE_STADIUM, FAIL);
            creature->ForcedDespawn();
            break;
    }
}
void instance_blackrock_spire::SetData(uint32 type, uint32 data)
{
    switch(type)
    {
        case TYPE_ROOM_EVENT:
        {
            if(Encounters[0] != DONE)
                Encounters[0] = data;

            if(data == DONE)
                for(std::set<uint64>::iterator i = EmberseerInDoorGUID.begin(); i != EmberseerInDoorGUID.end(); ++i)
                    HandleGameObject(*i, true);

            break;
        }
        case DATA_EMBERSEER:
        {
            if(Encounters[1] == DONE) return;
            Encounters[1] = data;

            /*if(data == IN_PROGRESS && Encounters[0] != DONE)
                LogPossibleCheaters("UBRS-Emberseer without runes");*/
            switch(data)
            {
                case IN_PROGRESS:
                {
                    if(Creature *ember = GetCreature(pyroguard_emberseerGUID))
                    {
                        ember->AI()->DoAction(1);
                    }
                    for(std::set<uint64>::iterator i = channelersGUID.begin(); i != channelersGUID.end(); ++i)
                    {
                        if(Creature * channeler = instance->GetCreature(*i))
                        {
                            channeler->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                            channeler->FinishSpell(CURRENT_CHANNELED_SPELL);
                            channeler->AI()->DoZoneInCombat();
                        }
                    }
                    for(std::set<uint64>::iterator i = runesEmberGUID.begin(); i != runesEmberGUID.end(); ++i)
                    {
                        HandleGameObject(*i, true);
                    }
                    break;
                }
                case DONE:
                {
                    HandleGameObject(emberseerOut, true);
                }
                case FAIL:
                {
                    for(std::set<uint64>::iterator i = channelersGUID.begin(); i != channelersGUID.end(); ++i)
                    {
                        if(Creature * channeler = instance->GetCreature(*i))
                        {
                            channeler->ForcedDespawn();
                        }
                    }
                    for(std::set<uint64>::iterator i = runesEmberGUID.begin(); i != runesEmberGUID.end(); ++i)
                    {
                        HandleGameObject(*i, false);
                    }
                    break;
                }
            }
            break;
        }
        case DATA_THE_BEAST:
        {
            if(Encounters[2] != DONE)
                Encounters[2] = data;

            /*if(data == IN_PROGRESS && Encounters[1] != DONE)
                LogPossibleCheaters("UBRS-Beast without emberseer");*/
        }
        case TYPE_FLAMEWREATH:
        {
            if (data == FAIL)
            {
                FlamewreathEventTimer = 0;
                FlamewreathWaveCount = 0;
            }
            Encounters[3] = data;
            break;
        }
        case TYPE_GYTH:
        {
            if (data == IN_PROGRESS)
                HandleGameObject(GythEntryDoorGUID, false);
            else if (data == DONE)
            {
                HandleGameObject(GythEntryDoorGUID, true);
                HandleGameObject(GythExitDoorGUID, true);
            }
            Encounters[4] = data;
            break;
        }
        case TYPE_STADIUM:
            // Don't set the same data twice
            if (Encounters[5] == data)
                break;
            // Start event
            if (data == IN_PROGRESS)
            {
                HandleGameObject(GythEntryDoorGUID, false);
                StartNextDialogueText(SAY_NEFARIUS_INTRO_1);
            }
            else if (data == DONE)
            {
                HandleGameObject(GythEntryDoorGUID, true);
                HandleGameObject(GythExitDoorGUID, true);
            }
            else if (data == FAIL)
            {
                HandleGameObject(GythEntryDoorGUID, true);
                // Despawn Nefarius and Rend on fail (the others are despawned OnCreatureEvade())
                if (Creature* pNefarius = instance->GetCreatureById(NPC_LORD_VICTOR_NEFARIUS))
                    pNefarius->ForcedDespawn();
                if (Creature* pRend = instance->GetCreatureById(NPC_REND_BLACKHAND))
                    pRend->ForcedDespawn();
                if (Creature* pGyth = instance->GetCreatureById(NPC_GYTH))
                    pGyth->ForcedDespawn();

                StadiumEventTimer = 0;
                StadiumMobsAlive = 0;
                StadiumWaves = 0;
            }
            Encounters[5] = data;
            break;
    }

    if (data == DONE || data == FAIL)
        SaveToDB();
}

std::string instance_blackrock_spire::GetSaveData()
{
    OUT_SAVE_INST_DATA;

    std::ostringstream stream;
    stream << Encounters[0] << " ";
    stream << Encounters[1] << " ";
    stream << Encounters[2];

    OUT_SAVE_INST_DATA_COMPLETE;

    return stream.str();
}

void instance_blackrock_spire::Load(const char* in)
{
    if (!in)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(in);

    std::istringstream loadStream(in);
    loadStream >> Encounters[0] >> Encounters[1] >> Encounters[2];

    for(uint8 i = 0; i < MAX_ENCOUNTERS; ++i)
        if (Encounters[i] == IN_PROGRESS)
            Encounters[i] = NOT_STARTED;

    OUT_LOAD_INST_DATA_COMPLETE;

}

void instance_blackrock_spire::DoSendNextFlamewreathWave()
{
    GameObject* pSummoner = instance->GetGameObject(GoFatherFlameGUID);
    if (!pSummoner)
        return;

    // TODO - The npcs would move nicer if they had DB waypoints, so i suggest to change their default movement to DB waypoints, and random movement when they reached their goal

    if (FlamewreathWaveCount < 6)                       // Send two adds (6 waves, then boss)
    {
        Creature* pSummoned = NULL;
        for (uint8 i = 0; i < 2; ++i)
        {
            float fX, fY, fZ;
            pSummoner->GetRandomPoint(rookeryEventSpawnPos[0], rookeryEventSpawnPos[1], rookeryEventSpawnPos[2], 2.5f, fX, fY, fZ);
            // Summon Rookery Hatchers in first wave, else random
            if (pSummoned = pSummoner->SummonCreature(urand(0, 1) && FlamewreathWaveCount ? NPC_ROOKERY_GUARDIAN : NPC_ROOKERY_HATCHER, fX, fY, fZ, 0.0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000))
            {
                if(Unit *target = pSummoned->AI()->SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
                    pSummoned->AI()->AttackStart(target);
            }
        }
        if (pSummoned && FlamewreathWaveCount == 0)
            DoScriptText(SAY_ROOKERY_EVENT_START, pSummoned);

        if (FlamewreathWaveCount < 4)
            FlamewreathEventTimer = 30000;
        else if (FlamewreathWaveCount < 6)
            FlamewreathEventTimer = 40000;
        else
            FlamewreathEventTimer = 10000;

        ++FlamewreathWaveCount;
    }
    else                                                    // Send Flamewreath
    {
        if (Creature* pSolakar = pSummoner->SummonCreature(NPC_SOLAKAR_FLAMEWREATH, rookeryEventSpawnPos[0], rookeryEventSpawnPos[1], rookeryEventSpawnPos[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, HOUR * MILLISECONDS))
            pSolakar->GetMotionMaster()->MovePoint(1, pSummoner->GetPositionX(), pSummoner->GetPositionY(), pSummoner->GetPositionZ());
        SetData(TYPE_FLAMEWREATH, SPECIAL);
        FlamewreathEventTimer = 0;
    }
}

Creature* instance_blackrock_spire::GetSpeakerByEntry(uint32 uiEntry)
{
    switch (uiEntry)
    {
        case NPC_LORD_VICTOR_NEFARIUS:  return instance->GetCreatureById(NPC_LORD_VICTOR_NEFARIUS);
        case NPC_REND_BLACKHAND:        return instance->GetCreatureById(NPC_REND_BLACKHAND);
        default: return NULL;
    }

}

void instance_blackrock_spire::Update(uint32 diff)
{
    DialogueUpdate(diff);

    if (StadiumEventTimer)
    {
        if (StadiumEventTimer <= diff)
            DoSendNextStadiumWave();
        else
            StadiumEventTimer -= diff;
    }

    if (FlamewreathEventTimer)
    {
        if (FlamewreathEventTimer <= diff)
            DoSendNextFlamewreathWave();
        else
            FlamewreathEventTimer -= diff;
    }
}

void instance_blackrock_spire::StartflamewreathEventIfCan()
{
    // Already done or currently in progress - or endboss done
    if (Encounters[TYPE_FLAMEWREATH] == DONE || Encounters[TYPE_FLAMEWREATH] == IN_PROGRESS)
        return;

    // Boss still around
    if (Creature* Solakar = instance->GetCreatureById(NPC_SOLAKAR_FLAMEWREATH))
    {
        if(Solakar->isAlive())
            return;
    }
    // Start summoning of mobs
    FlamewreathEventTimer = 1;
    FlamewreathWaveCount = 0;
}

void instance_blackrock_spire::JustDidDialogueStep(int32 iEntry)
{
    switch (iEntry)
    {
        case NPC_BLACKHAND_HANDLER:
            StadiumEventTimer = 1000;
            // Move the two near the balcony
            if (Creature* pRend = instance->GetCreatureById(NPC_REND_BLACKHAND))
                pRend->SetFacingTo(aStadiumLocs[5].m_fO);
            if (Creature* pNefarius = instance->GetCreatureById(NPC_LORD_VICTOR_NEFARIUS))
                pNefarius->GetMotionMaster()->MovePoint(0, aStadiumLocs[5].m_fX, aStadiumLocs[5].m_fY, aStadiumLocs[5].m_fZ);
            break;
        case SAY_NEFARIUS_WARCHIEF:
            // Prepare for Gyth - note: Nefarius should be moving around the balcony
            if (Creature* pRend = instance->GetCreatureById(NPC_REND_BLACKHAND))
            {
                pRend->ForcedDespawn(5000);
                pRend->SetWalk(false);
                pRend->GetMotionMaster()->MovePoint(0, aStadiumLocs[6].m_fX, aStadiumLocs[6].m_fY, aStadiumLocs[6].m_fZ);
            }
            StadiumEventTimer = 30000;
            break;
        case SAY_NEFARIUS_VICTORY:
            SetData(TYPE_STADIUM, DONE);
            break;
        case NPC_REND_BLACKHAND:
            // Despawn Nefarius
            if (Creature* pNefarius = instance->GetCreatureById(NPC_LORD_VICTOR_NEFARIUS))
            {
                pNefarius->ForcedDespawn(5000);
                pNefarius->GetMotionMaster()->MovePoint(0, aStadiumLocs[6].m_fX, aStadiumLocs[6].m_fY, aStadiumLocs[6].m_fZ);
            }
            break;
    }
}

void instance_blackrock_spire::DoSendNextStadiumWave()
{
    if (StadiumWaves < MAX_STADIUM_WAVES)
    {
        // Send current wave mobs
        if (Creature* pNefarius = instance->GetCreatureById(NPC_LORD_VICTOR_NEFARIUS))
        {
            float fX, fY, fZ;
            for (uint8 i = 0; i < MAX_STADIUM_MOBS_PER_WAVE; ++i)
            {
                if (aStadiumEventNpcs[StadiumWaves][i] == 0)
                    continue;

                pNefarius->GetRandomPoint(aStadiumLocs[0].m_fX, aStadiumLocs[0].m_fY, aStadiumLocs[0].m_fZ, 7.0f, fX, fY, fZ);
                fX = std::min(aStadiumLocs[0].m_fX, fX);    // Halfcircle - suits better the rectangular form
                if (Creature* pTemp = pNefarius->SummonCreature(aStadiumEventNpcs[StadiumWaves][i], fX, fY, fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0))
                {
                    // Get some point in the center of the stadium
                    pTemp->GetRandomPoint(aStadiumLocs[2].m_fX, aStadiumLocs[2].m_fY, aStadiumLocs[2].m_fZ, 5.0f, fX, fY, fZ);
                    fX = std::min(aStadiumLocs[2].m_fX, fX);// Halfcircle - suits better the rectangular form

                    pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
                }
            }
        }
        switch(StadiumWaves)
        {
            case 0: case 1: case 2: case 3: StadiumMobsAlive = 4; break;
            case 4: case 5: case 6: StadiumMobsAlive = 5; break;
            default: break;
        }
        HandleGameObject(GythCombatDoorGUID, true);
    }
    // All waves are cleared - start Gyth intro
    else if (StadiumWaves == MAX_STADIUM_WAVES)
        StartNextDialogueText(SAY_NEFARIUS_LOSE_4);
    else
    {
        // Send Gyth
        if (Creature* pNefarius = instance->GetCreatureById(NPC_LORD_VICTOR_NEFARIUS))
        {
            if (Creature* pTemp = pNefarius->SummonCreature(NPC_GYTH, aStadiumLocs[1].m_fX, aStadiumLocs[1].m_fY, aStadiumLocs[1].m_fZ, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0))
                pTemp->GetMotionMaster()->MovePoint(0, aStadiumLocs[2].m_fX, aStadiumLocs[2].m_fY, aStadiumLocs[2].m_fZ);
        }

        // Set this to 2, because Rend will be summoned later during the fight
        StadiumMobsAlive = 2;

        HandleGameObject(GythCombatDoorGUID, true);
    }

    ++StadiumWaves;

    // Stop the timer when all the waves have been sent
    if (StadiumWaves >= MAX_STADIUM_WAVES)
        StadiumEventTimer = 0;
    else
        StadiumEventTimer = 60000;
}

InstanceData* GetInstanceData_instance_blackrock_spire(Map* map)
{
    return new instance_blackrock_spire(map);
}

bool AreaTrigger_at_blackrock_spire(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (!pPlayer->isAlive() || pPlayer->isGameMaster())
        return false;

    switch (pAt->id)
    {
        case AREATRIGGER_ENTER_UBRS:
            if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*)pPlayer->GetInstanceData())
                pInstance->DoSortRoomEventMobs();
            break;
        case AREATRIGGER_STADIUM:
            if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*)pPlayer->GetInstanceData())
            {
                if (pInstance->GetData(TYPE_STADIUM) == IN_PROGRESS || pInstance->GetData(TYPE_STADIUM) == DONE)
                    return false;

                // Summon Nefarius and Rend for the dialogue event
                // Note: Nefarius and Rend need to be hostile and not attackable
                if (Creature* pNefarius = pPlayer->SummonCreature(NPC_LORD_VICTOR_NEFARIUS, aStadiumLocs[3].m_fX, aStadiumLocs[3].m_fY, aStadiumLocs[3].m_fZ, aStadiumLocs[3].m_fO, TEMPSUMMON_CORPSE_DESPAWN, 0))
                {
                    pNefarius->setFaction(FACTION_BLACK_DRAGON);
                    pNefarius->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                }
                if (Creature* pRend = pPlayer->SummonCreature(NPC_REND_BLACKHAND, aStadiumLocs[4].m_fX, aStadiumLocs[4].m_fY, aStadiumLocs[4].m_fZ, aStadiumLocs[4].m_fO, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    pRend->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);

                pInstance->SetData(TYPE_STADIUM, IN_PROGRESS);
            }
            break;
    }
    return false;
}

bool GOUse_go_father_flame(Player* pPlayer, GameObject* pGo)
{
    if (instance_blackrock_spire* pInstance = (instance_blackrock_spire*)pGo->GetInstanceData())
        pInstance->StartflamewreathEventIfCan();
    pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
    return true;
}

void AddSC_instance_blackrock_spire()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_blackrock_spire";
    newscript->GetInstanceData = &GetInstanceData_instance_blackrock_spire;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_blackrock_spire";
    newscript->pAreaTrigger = &AreaTrigger_at_blackrock_spire;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_father_flame";
    newscript->pGOUse = &GOUse_go_father_flame;
    newscript->RegisterSelf();
}
