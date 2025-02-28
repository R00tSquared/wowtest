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
SDName: Instance_Blackrock_Depths
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"
#include "def_blackrock_depths.h"

#define ENCOUNTERS              9

#define C_EMPEROR               9019
#define C_PHALANX               9502
#define NPC_ANGERREL            9035
#define NPC_DOPEREL             9040
#define NPC_HATEREL             9034
#define NPC_VILEREL             9036
#define NPC_SEETHREL            9038
#define NPC_GLOOMREL            9037
#define NPC_DOOMREL             9039
#define NPC_PRINCESS            8929
#define NPC_PRIESTESS           10076
#define NPC_OGRABISI            9677
#define NPC_SHILL               9678
#define NPC_CREST               9680
#define NPC_JAZ                 9681
#define NPC_TOBIAS              9679
#define NPC_DUGHAL              9022
#define NPC_WINDSOR             9023
#define NPC_PLUGGER_SPAZZRING   9499
#define NPC_HURLEY_BLACKBREATH  9537
#define NPC_BLACKBREATH_CRONY   9541

#define GO_ARENA1               161525
#define GO_ARENA2               161522
#define GO_ARENA3               161524
#define GO_ARENA4               161523
#define GO_SHADOW_LOCK          161460
#define GO_SHADOW_MECHANISM     161461
#define GO_SHADOW_GIANT_DOOR    157923
#define GO_SHADOW_DUMMY         161516
#define GO_BAR_KEG_SHOT         170607
#define GO_BAR_KEG_TRAP         171941
#define GO_BAR_DOOR             170571
#define GO_TOMB_ENTER           170576
#define GO_TOMB_EXIT            170577
#define GO_LYCEUM               170558
#define GO_GOLEM_ROOM_N         170573
#define GO_GOLEM_ROOM_S         170574
#define GO_THONE_ROOM           170575
#define SPELL_STONED            10255
#define NPC_WATCHER_DOOMGRIP    9476
#define NPC_WARBRINGER_CONST    8905    // Four of them in Relict Vault are related to Doomgrip summon event
#define GO_SECRET_DOOR          174553
#define GO_SECRET_SAFE          161495

static const float aVaultPositions[4] = {821.905f, -338.382f, -50.134f, 3.78736f};

struct instance_blackrock_depths : public ScriptedInstance
{
    instance_blackrock_depths(Map *map) : ScriptedInstance(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];

    uint64 EmperorGUID;
    uint64 PhalanxGUID;
    uint64 DoomrelGUID;
    uint64 OgrabisiGUID;
    uint64 ShillGUID;
    uint64 ChestGUID;
    uint64 JazGUID;
    uint64 TobiasGUID;
    uint64 DughalGUID;
    uint64 WindsorGUID;
    uint64 ReginaldGUID;

    uint64 GoArena1GUID;
    uint64 GoArena2GUID;
    uint64 GoArena3GUID;
    uint64 GoArena4GUID;
    uint64 GoShadowLockGUID;
    uint64 GoShadowMechGUID;
    uint64 GoShadowGiantGUID;
    uint64 GoShadowDummyGUID;
    uint64 GoBarKegGUID;
    uint64 GoBarKegTrapGUID;
    uint64 GoBarDoorGUID;
    uint64 GoTombEnterGUID;
    uint64 GoTombExitGUID;
    uint64 GoLyceumGUID;
    uint64 GoGolemNGUID;
    uint64 GoGolemSGUID;
    uint64 GoThoneGUID;

    uint64 PlayerGUID;

    uint32 BarAleCount;
    uint32 DoomCount;
	uint32 BrokenKegs;

    uint64 TombBossGUIDs[6];
    uint8 CofferDoorsOpened;
    uint64 goSecretDoorGUID;
    uint64 goSecretSafeGUID;

    std::set<uint64> m_sVaultNpcGuids;

    void Initialize()
    {
        EmperorGUID = 0;
        PhalanxGUID = 0;
        DoomrelGUID = 0;
        OgrabisiGUID;
        ShillGUID = 0;
        ChestGUID = 0;
        JazGUID = 0;
        TobiasGUID = 0;
        DughalGUID = 0;
        WindsorGUID = 0;
        ReginaldGUID = 0;

        GoArena1GUID = 0;
        GoArena2GUID = 0;
        GoArena3GUID = 0;
        GoArena4GUID = 0;
        GoShadowLockGUID = 0;
        GoShadowMechGUID = 0;
        GoShadowGiantGUID = 0;
        GoShadowDummyGUID = 0;
        GoBarKegGUID = 0;
        GoBarKegTrapGUID = 0;
        GoBarDoorGUID = 0;
        GoTombEnterGUID = 0;
        GoTombExitGUID = 0;
        GoLyceumGUID = 0;
        GoGolemNGUID = 0;
        GoGolemSGUID = 0;
        GoThoneGUID = 0;

        BarAleCount = 0;
        DoomCount = 0;
        BrokenKegs = 0;

        CofferDoorsOpened = 0;
        m_sVaultNpcGuids.clear();

        goSecretDoorGUID = 0;
        goSecretSafeGUID = 0;

        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;
    }

    Player* GetPlayerInMap()
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

        debug_log("TSCR: Instance Blackrock Depths: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    // function that replaces the princess if requirements are met
    bool CanReplacePrincess() const
    {
        Map::PlayerList const& players = instance->GetPlayers();
        if (players.isEmpty())
            return false;

        for (const auto& player : players)
        {
            if (Player* pPlayer = player.getSource())
            {
                // if at least one player didn't complete the quest, return false
                if ((pPlayer->GetTeam() == ALLIANCE && !pPlayer->GetQuestRewardStatus(4362))
                        || (pPlayer->GetTeam() == HORDE && !pPlayer->GetQuestRewardStatus(4003)))
                    return false;
            }
        }

        return true;
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch(creature->GetEntry())
        {
            case NPC_PRINCESS: 
                if (CanReplacePrincess())
                    creature->UpdateEntry(NPC_PRIESTESS);
                break;
            case C_EMPEROR: EmperorGUID = creature->GetGUID(); break;
            case C_PHALANX: PhalanxGUID = creature->GetGUID(); break;
            case NPC_WINDSOR: WindsorGUID = creature->GetGUID(); break;
            case NPC_OGRABISI: OgrabisiGUID = creature->GetGUID(); break;
            case NPC_SHILL: ShillGUID = creature->GetGUID(); break;
            case NPC_CREST: ChestGUID = creature->GetGUID(); break;
            case NPC_JAZ: JazGUID = creature->GetGUID(); break;
            case NPC_TOBIAS: TobiasGUID = creature->GetGUID(); break;
            case NPC_DUGHAL: DughalGUID = creature->GetGUID(); break;
            case NPC_DOOMREL: DoomrelGUID = creature->GetGUID(); break;
            case NPC_DOPEREL: TombBossGUIDs[0] = creature->GetGUID(); break;
            case NPC_HATEREL: TombBossGUIDs[1] = creature->GetGUID(); break;
            case NPC_VILEREL: TombBossGUIDs[2] = creature->GetGUID(); break;
            case NPC_SEETHREL: TombBossGUIDs[3] = creature->GetGUID(); break;
            case NPC_GLOOMREL: TombBossGUIDs[4] = creature->GetGUID(); break;
            case NPC_ANGERREL: TombBossGUIDs[5] = creature->GetGUID(); break;
            case NPC_WARBRINGER_CONST:
                // Golems not in the Relict Vault?
                if (std::abs(creature->GetPositionZ() - aVaultPositions[2]) > 1.0f || !creature->IsWithinDist2d(aVaultPositions[0], aVaultPositions[1], 20.0f))
                    break;
                // Golems in Relict Vault need to have a stoned aura, set manually to prevent reapply when reached home
                creature->CastSpell(creature, SPELL_STONED, true);
                // Store the Relict Vault Golems into m_sVaultNpcGuids
            case NPC_WATCHER_DOOMGRIP:
                m_sVaultNpcGuids.insert(creature->GetObjectGuid());
            break;
        }
    }

    void OnObjectCreate(GameObject* go)
    {
        switch(go->GetEntry())
        {
        case GO_ARENA1: GoArena1GUID = go->GetGUID(); break;
        case GO_ARENA2: GoArena2GUID = go->GetGUID(); break;
        case GO_ARENA3: GoArena3GUID = go->GetGUID(); break;
        case GO_ARENA4: GoArena4GUID = go->GetGUID(); break;
        case GO_SHADOW_LOCK: GoShadowLockGUID = go->GetGUID(); break;
        case GO_SHADOW_MECHANISM: GoShadowMechGUID = go->GetGUID(); break;
        case GO_SHADOW_GIANT_DOOR: GoShadowGiantGUID = go->GetGUID(); break;
        case GO_SHADOW_DUMMY: GoShadowDummyGUID = go->GetGUID(); break;
        case GO_BAR_KEG_SHOT: GoBarKegGUID = go->GetGUID(); break;
        case GO_BAR_KEG_TRAP: GoBarKegTrapGUID = go->GetGUID(); break;
        case GO_BAR_DOOR: GoBarDoorGUID = go->GetGUID(); break;
        case GO_TOMB_ENTER: GoTombEnterGUID = go->GetGUID(); break;
        case GO_TOMB_EXIT: GoTombExitGUID = go->GetGUID();
            if (Encounter[3] == DONE)
                HandleGameObject(GoTombExitGUID, 0); break;
        case GO_LYCEUM: GoLyceumGUID = go->GetGUID(); break;
        case GO_GOLEM_ROOM_N: GoGolemNGUID = go->GetGUID(); break;
        case GO_GOLEM_ROOM_S: GoGolemSGUID = go->GetGUID(); break;
        case GO_THONE_ROOM: GoThoneGUID = go->GetGUID(); break;
        case GO_SECRET_DOOR: goSecretDoorGUID = go->GetGUID(); break;
        case GO_SECRET_SAFE: goSecretSafeGUID = go->GetGUID(); break;
        }
    }

    void OnCreatureDeath(Creature* creature)
    {
        switch (creature->GetEntry())
        {
            case NPC_WARBRINGER_CONST:
            case NPC_WATCHER_DOOMGRIP:
                if (GetData(TYPE_VAULT) == IN_PROGRESS)
                {
                    m_sVaultNpcGuids.erase(creature->GetObjectGuid());

                    // If all event npcs dead then set event to done
                    if (m_sVaultNpcGuids.empty())
                        SetData(TYPE_VAULT, DONE);
                }
                break;
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        Player *player = GetPlayerInMap();

        if (!player)
        {
            debug_log("TSCR: Instance Blackrock Depths: SetData (Type: %u Data %u) cannot find any player.", type, data);
            return;
        }

        debug_log("TSCR: Instance Blackrock Depths: SetData update (Type: %u Data %u)", type, data);

        switch(type)
        {
        case TYPE_RING_OF_LAW:
            Encounter[0] = data;
            break;
        case TYPE_VAULT:
            if (data == SPECIAL)
            {
                ++CofferDoorsOpened;

                if (CofferDoorsOpened == 12)
                {
                    SetData(TYPE_VAULT, IN_PROGRESS);

                    Creature* pConstruct = nullptr;

                    // Activate vault constructs
                    for (std::set<uint64>::iterator i = m_sVaultNpcGuids.begin(); i != m_sVaultNpcGuids.end(); ++i)
                    {
                        pConstruct = instance->GetCreature(*i);
                        if (pConstruct)
                            pConstruct->RemoveAurasDueToSpell(SPELL_STONED);
                    }

                    if (!pConstruct)
                        return;

                    // Summon doomgrip
                    pConstruct->SummonCreature(9476, aVaultPositions[0], aVaultPositions[1], aVaultPositions[2], aVaultPositions[3], TEMPSUMMON_DEAD_DESPAWN, 0);
                }
                // No need to store in this case
                return;
            }
            if (data == DONE)
            {
                if (GameObject* goSecretDoor = instance->GetGameObject(goSecretDoorGUID))
                    goSecretDoor->UseDoorOrButton();
                if (GameObject* goSecretDoor = instance->GetGameObject(goSecretSafeGUID))
                    goSecretDoor->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
            }
            Encounter[1] = data;
            break;
        case TYPE_BAR:
            if (data == SPECIAL)
                ++BarAleCount;
            else
                Encounter[2] = data;
            break;
        case TYPE_TOMB_OF_SEVEN:
            if (data == IN_PROGRESS)
            {
                ++DoomCount;
                if (Creature* Doomrel = instance->GetCreature(DoomrelGUID))
                    Doomrel->AI()->DoAction(DoomCount);

                HandleGameObject(GoTombEnterGUID, 1);
            }
            if (data == SPECIAL)
            {
                ++DoomCount;
                if (Creature* Doomrel = instance->GetCreature(DoomrelGUID))
                    Doomrel->AI()->DoAction(DoomCount);
            }
            if (data == FAIL)
            {
                for (uint8 i = 0; i < 6; ++i)
                {
                    if (Creature* boss = instance->GetCreature(TombBossGUIDs[i]))
                    {
                        if (!boss->isAlive())
                            boss->Respawn();
                    }
                }

                DoomCount = 0;
                
                HandleGameObject(GoTombEnterGUID, 0);
            }
            if (data == DONE)
            {
                HandleGameObject(GoTombExitGUID, 0);
                HandleGameObject(GoTombEnterGUID, 0);
            }

            Encounter[3] = data;
            break;
        case TYPE_LYCEUM:
            Encounter[4] = data;
            break;
        case TYPE_IRON_HALL:
            Encounter[5] = data;
            break;
        case DATA_QUEST_JAIL_BREAK:
            if (data == FAIL)
                Encounter[6] = NOT_STARTED;

            Encounter[6] = data;
            break;
        case TYPE_THELDREN:
            Encounter[7] = data;
            break;
		case TYPE_HURLEY:
            if (data == SPECIAL)
            {
                ++BrokenKegs;

                if (BrokenKegs == 3 && Encounter[8] != IN_PROGRESS)
                {
                    if (Creature* pPlugger = GetCreatureById(NPC_PLUGGER_SPAZZRING))
                    {
                        // Summon Hurley Blackbreath
                        Creature* pHurley = pPlugger->SummonCreature(NPC_HURLEY_BLACKBREATH, 856.0867, -149.7469, -49.6719, 0.05949629, TEMPSUMMON_DEAD_DESPAWN, 0);
                        if (!pHurley)
                            return;
						pHurley->SetSpeed(MOVE_RUN, 1.3f);
						DoScriptText(-2100033, pHurley);
                        pHurley->GetMotionMaster()->MovePoint(2, 908.96, -145.32, -49.75);
                        pHurley->SetHomePosition(908.96, -145.32, -49.75, 3.62);

                        // Summon cronies around Hurley
                        for (uint8 i = 0; i < 3; ++i)
                        {
                            float fX, fY, fZ;
                            pPlugger->GetRandomPoint(856.0867, -149.7469, -49.6719, 2.0f, fX, fY, fZ);
                            if (Creature* pSummoned = pPlugger->SummonCreature(NPC_BLACKBREATH_CRONY, fX, fY, fZ, 0.05949629, TEMPSUMMON_DEAD_DESPAWN, 0))
                            {
								pSummoned->SetSpeed(MOVE_RUN, 1.3f);
                                pSummoned->GetMotionMaster()->MoveFollow(pHurley, 1.0f, 0);

                                float fX2, fY2, fZ2;
                                pSummoned->GetRandomPoint(908.96, -145.32, -49.75, 3.62f, fX2, fY2, fZ2);
                                pSummoned->SetHomePosition(fX2, fY2, fZ2, 3.62);
                            }
                        }
                    }
                SetData(TYPE_HURLEY, IN_PROGRESS);
                }
            }
            else 
                Encounter[8] = data;
            break;
        }

        if (data == DONE)
            SaveToDB();
    }

    void SetData64(uint32 type, uint64 data)
    {
        switch (type)
        {
            case Q_STARTER:
                PlayerGUID = data; break;
            case DATA_REGINALD:
                ReginaldGUID = data; break;
        }
    }

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
        case TYPE_RING_OF_LAW:
            return Encounter[0];
        case TYPE_VAULT:
            return Encounter[1];
        case TYPE_BAR:
            if (Encounter[2] == IN_PROGRESS && BarAleCount == 3)
                return SPECIAL;
            else
                return Encounter[2];
        case TYPE_TOMB_OF_SEVEN:
            return Encounter[3];
        case TYPE_LYCEUM:
            return Encounter[4];
        case TYPE_IRON_HALL:
            return Encounter[5];
        case DATA_QUEST_JAIL_BREAK:
            return Encounter[6];
        case TYPE_THELDREN:
            return Encounter[7];
		case TYPE_HURLEY:
            return Encounter[8];
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch(data)
        {
        case DATA_EMPEROR:
            return EmperorGUID;
        case DATA_PHALANX:
            return PhalanxGUID;
        case DATA_ARENA1:
            return GoArena1GUID;
        case DATA_ARENA2:
            return GoArena2GUID;
        case DATA_ARENA3:
            return GoArena3GUID;
        case DATA_ARENA4:
            return GoArena4GUID;
        case DATA_GO_BAR_KEG:
            return GoBarKegGUID;
        case DATA_GO_BAR_KEG_TRAP:
            return GoBarKegTrapGUID;
        case DATA_GO_BAR_DOOR:
            return GoBarDoorGUID;
        case Q_STARTER:
            return PlayerGUID;
        case DATA_WINDSOR:
            return WindsorGUID;
        case DATA_REGINALD:
            return ReginaldGUID;
        case DATA_OGRABISI:
            return OgrabisiGUID;
        case DATA_SHILL:
            return ShillGUID;
        case DATA_CREST:
            return ChestGUID;
        case DATA_JAZ:
            return JazGUID;
        case DATA_TOBIAS:
            return TobiasGUID;
        case DATA_DUGHAL:
            return DughalGUID;
        case DATA_ANGERREL:
            return TombBossGUIDs[5];
        case DATA_DOPEREL:
            return TombBossGUIDs[0];
        case DATA_HATEREL:
            return TombBossGUIDs[1];
        case DATA_VILEREL:
            return TombBossGUIDs[2];
        case DATA_SEETHREL:
            return TombBossGUIDs[3];
        case DATA_GLOOMREL:
            return TombBossGUIDs[4];
        case DATA_DOOMREL:
            return DoomrelGUID;
        }
        return 0;
    }

    void HandleGameObject(uint64 guid, uint32 state)
    {
        Player *player = GetPlayerInMap();

        if (!player || !guid)
            return;

        if (GameObject *go = GameObject::GetGameObject(*player,guid))
            go->SetGoState(GOState(state));
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
        stream << Encounter[5];

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
        loadStream >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3]
        >> Encounter[4] >> Encounter[5];

        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if (Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;

        if (Encounter[3] == DONE)
            HandleGameObject(GoTombExitGUID, 0);

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_blackrock_depths(Map* map)
{
    return new instance_blackrock_depths(map);
}

   void AddSC_instance_blackrock_depths()
   {
       Script *newscript;
       newscript = new Script;
       newscript->Name = "instance_blackrock_depths";
       newscript->GetInstanceData = &GetInstanceData_instance_blackrock_depths;
       newscript->RegisterSelf();
   }

