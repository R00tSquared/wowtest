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
SDName: Instance_Shadow_Labyrinth
SD%Complete: 85
SDComment: Some cleanup left along with save
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "precompiled.h"
#include "def_shadow_labyrinth.h"

#define ENCOUNTERS 5

#define REFECTORY_DOOR          183296                      //door opened when blackheart the inciter dies
#define SCREAMING_HALL_DOOR     183295                      //door opened when grandmaster vorpil dies

/* Shadow Labyrinth encounters:
1 - Ambassador Hellmaw event
2 - Blackheart the Inciter event
3 - Grandmaster Vorpil event
4 - Murmur event
*/

struct instance_shadow_labyrinth : public ScriptedInstance
{
    instance_shadow_labyrinth(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    GBK_handler m_gbk;

    uint64 RefectoryDoorGUID;
    uint64 ScreamingHallDoorGUID;
    std::list<uint64> RitualistGUIDList;

    uint32 RitualistCount;
    uint32 WaveOneDeadCounter;
    uint32 WaveTwoDeadCounter;
    uint32 WaveThreeDeadCounter;
    Timer Timer_SummonCreature;
    bool WaveOneDone;
    bool WaveTwoDone;
    bool WaveThreeDone;
    uint64 MurmurBossGUID;
    uint64 VorpilBossGUID;

    bool check;

    void Initialize()
    {
        RefectoryDoorGUID = 0;
        ScreamingHallDoorGUID = 0;

        RitualistGUIDList.clear();

        RitualistCount = 0;

        WaveOneDeadCounter = 0;
        WaveTwoDeadCounter = 0;
        WaveThreeDeadCounter = 0;
        WaveOneDone = false;
        WaveTwoDone = false;
        WaveThreeDone = false;
        Timer_SummonCreature.Reset(0);
        MurmurBossGUID = 0;
        VorpilBossGUID = 0;

        check = false;

        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            if(Encounter[i] == IN_PROGRESS) return true;

        return false;
    }

    void OnObjectCreate(GameObject *go)
    {
        switch(go->GetEntry())
        {
        case REFECTORY_DOOR:
            RefectoryDoorGUID = go->GetGUID();
            if (Encounter[2] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case SCREAMING_HALL_DOOR:
            ScreamingHallDoorGUID = go->GetGUID();
            if (Encounter[3] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch(creature_entry)
        {
            case 18708:
                MurmurBossGUID = creature->GetGUID();
                break;
            case 18732:
                VorpilBossGUID = creature->GetGUID();
                break;
            case 18794:
            //case 20645:
                if (creature->isAlive())
                {
                    RitualistGUIDList.push_back(creature->GetGUID());
                    ++RitualistCount;
                    SetData(TYPE_RITUALIST, NOT_STARTED);
                }
                break;
        }
    }

    void OnCreatureDeath(Creature* pCreature)
    {
        switch(pCreature->GetDBTableGUIDLow())
        {
            case 66883:
            case 66881:
            case 66886:
            case 66816:
            {
                WaveOneDeadCounter++;
                if(WaveOneDeadCounter >= 3)
                    WaveOneDone = true;
                break;
            }
            case 66817:
            case 66882:
            case 66848:
            case 66818:
            {
                WaveTwoDeadCounter++;
                if(WaveTwoDeadCounter >= 3)
                    WaveTwoDone = true;
                break;
            }
            case 66819:
            case 66885:
            case 66850:
            case 66820:
            {
                WaveThreeDeadCounter++;
                if(WaveThreeDeadCounter >= 3)
                    WaveThreeDone = true;
                break;
            }
            default: break;
        }
    }

    void HandleGameObject(uint64 guid, uint32 state)
    {
        if (!guid)
        {
            debug_log("TSCR: Shadow Labyrinth: HandleGameObject fail");
            return;
        }

        if (GameObject *go = instance->GetGameObject(guid))
            go->SetGoState(GOState(state));
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case TYPE_HELLMAW:                      return GBK_SL_HELLMAW;
            case DATA_BLACKHEARTTHEINCITEREVENT:    return GBK_SL_BLACKHEARTH;
            case DATA_GRANDMASTERVORPILEVENT:       return GBK_SL_VORPIL;
            case DATA_MURMUREVENT:                  return GBK_SL_MURMUR;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case TYPE_HELLMAW:
                Encounter[0] = data;
                break;

            case TYPE_RITUALIST:
                if (data == NOT_STARTED)
                    Encounter[1] = NOT_STARTED;
                if (data == DONE && RitualistCount)
                    --RitualistCount;
                if (RitualistCount == 0)
                    Encounter[1] = DONE;
                break;

            case DATA_BLACKHEARTTHEINCITEREVENT:
                if (data == DONE)
                    HandleGameObject(RefectoryDoorGUID,0);
                Encounter[2] = data;
                break;

            case DATA_GRANDMASTERVORPILEVENT:
                if (data == DONE)
                {
                    Timer_SummonCreature = 30000;
                    if(Creature* VorpilBoss = GetCreature(VorpilBossGUID))
                        VorpilBoss->SummonCreature(32001, -156.675, -284.365, 17.084, 4.71, TEMPSUMMON_DEAD_DESPAWN, 0);
                }
                Encounter[3] = data;
                break;

            case DATA_MURMUREVENT:
                Encounter[4] = data;
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
            if (type == TYPE_RITUALIST && RitualistCount != 0)
                return;

            SaveToDB();
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
        switch (type)
        {
            case TYPE_HELLMAW:                  return Encounter[0];
            case TYPE_RITUALIST:                return Encounter[1];
            case DATA_GRANDMASTERVORPILEVENT:   return Encounter[3];
            case DATA_MURMUREVENT:              return Encounter[4];
        }
        return false;
    }

    uint64 GetData64(uint32 identifier)
    {
        if (identifier == DATA_GRANDMASTERVORPIL)
            return VorpilBossGUID;
        else if(identifier == DATA_SCREAMING_HALL_DOOR)
            return ScreamingHallDoorGUID;

        return 0;
    }

    void Update(uint32 diff)
    {
        if (!check && !RitualistGUIDList.empty())
        {
            for (std::list<uint64>::iterator iter = RitualistGUIDList.begin(); iter != RitualistGUIDList.end(); ++iter)
            {
                Creature* ritualist = instance->GetCreature(*iter);
                if (ritualist && ritualist->isDead())
                    --RitualistCount;
            }

            if (!RitualistCount)
                Encounter[1] = DONE;

            check = true;
        }
        
        if(Timer_SummonCreature.Expired(diff))
        {
            Timer_SummonCreature = 30000;

            if(!MurmurBossGUID)
                return;

            Creature* MurmurBoss = GetCreature(MurmurBossGUID);
            if (!MurmurBoss)
                return;

            if(!WaveOneDone)
            {
                uint32 CreatureSummonID = urand(0,1) ? 18634 : 18639;
                if(Creature* SummonWaveOne = MurmurBoss->SummonCreature(CreatureSummonID, -136.037, -343.410, 17.164, 3.17, TEMPSUMMON_TIMED_DESPAWN, 25000))
                    SummonWaveOne->GetMotionMaster()->MovePoint(1, -156.511, -349.477, 17.083);
            }
            if(!WaveTwoDone)
            {
                uint32 CreatureSummonID = urand(0,1) ? 18634 : 18639;
                if(Creature* SummonWaveTwo = MurmurBoss->SummonCreature(CreatureSummonID, -134.274, -388.795, 17.165, 3.72, TEMPSUMMON_TIMED_DESPAWN, 25000))
                    SummonWaveTwo->GetMotionMaster()->MovePoint(1, -163.125, -394.712, 17.079);
            }
            if(!WaveOneDone)
            {
                if(Creature* SummonWaveThree1 = MurmurBoss->SummonCreature(18634, -176.776, -434.265, 17.162, 0.27, TEMPSUMMON_TIMED_DESPAWN, 25000))
                    SummonWaveThree1->GetMotionMaster()->MovePoint(1, -159.900, -428.419, 17.077);
                if(Creature* SummonWaveThree2 = MurmurBoss->SummonCreature(18639, -130.550, -443.696, 17.164, 3.14, TEMPSUMMON_TIMED_DESPAWN, 25000))
                    SummonWaveThree2->GetMotionMaster()->MovePoint(1, -152.535, -434.172, 17.077);
            }
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
        stream << Encounter[4];

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
        loadStream >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3] >> Encounter[4];

        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if (Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_shadow_labyrinth(Map* map)
{
    return new instance_shadow_labyrinth(map);
}

/*######
## npc_tortured_skeleton
######*/

struct npc_tortured_skeletonAI : public ScriptedAI
{
    npc_tortured_skeletonAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetStandState(UNIT_STAND_STATE_DEAD);
    }

    void EnterCombat(Unit* who)
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void EnterEvadeMode()
    {
        me->RemoveAllAuras();
        me->DeleteThreatList();
        me->CombatStop();
        
        if (!me->isAlive())
            return;    

        me->GetMotionMaster()->MoveTargetedHome();
    }

    void JustReachedHome()
    {
        Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_tortured_skeleton(Creature* creature)
{
    return new npc_tortured_skeletonAI (creature);
}

/*######
## npc_32001
######*/

struct npc_32001AI : public ScriptedAI
{
    npc_32001AI(Creature* creature) : ScriptedAI(creature) 
    {
        pInstance = (ScriptedInstance*)me->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    bool Intro;

    void Reset()
    {
        Intro = false;
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(!Intro && who && who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(who, 10.0f))
        {
            if(pInstance)
                pInstance->HandleGameObject(pInstance->GetData64(DATA_SCREAMING_HALL_DOOR), true);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, me, 2000, 200);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_B, me, 2000, 200);
            Intro = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_32001(Creature* creature)
{
    return new npc_32001AI (creature);
}

void AddSC_instance_shadow_labyrinth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_shadow_labyrinth";
    newscript->GetInstanceData = &GetInstanceData_instance_shadow_labyrinth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_tortured_skeleton";
    newscript->GetAI = &GetAI_npc_tortured_skeleton;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_32001";
    newscript->GetAI = &GetAI_npc_32001;
    newscript->RegisterSelf();
}

