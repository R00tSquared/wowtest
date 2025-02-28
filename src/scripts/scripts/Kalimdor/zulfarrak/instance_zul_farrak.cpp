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
SDName: instance_zul_farrak
SD%Complete:
SDComment:
SDCategory: Zul'Farrak
EndScriptData
*/

#include "precompiled.h"
#include "def_zul_farrak.h"

#define ENCOUNTERS 2

float ZFWPs[11][3] =
{
    {1880.39, 1292.89, 47.27},
    {1887.35, 1263.67, 41.48},
    {1890.87, 1263.86, 41.41},
    {1883.12, 1263.76, 41.59},
    {1890.72, 1268.39, 41.47},
    {1882.84, 1267.99, 41.73},
    {1885.85, 1202.20, 8.88},
    {1889.46, 1204.23, 8.88},
    {1887.41, 1208.92, 8.88},
    {1895.49, 1204.23, 8.88},
    {1876.23, 1207.52, 8.88}
};

float spawns[26][4] ={
{1902.83,    1223.41,    8.96,    3.95},
{1894.64,    1206.29,    8.87,    2.22},
{1874.45,    1204.44,    8.87,    0.88},
{1874.18,    1221.24,    9.21,    0.84},
{1879.02,    1223.06,    9.12,    5.91},
{1882.07,    1225.70,    9.32,    5.69},
{1886.97,    1225.86,    9.85,    5.79},
{1892.28,    1225.49,    9.57,    5.63},
{1894.72,    1221.91,    8.87,    2.34},
{1890.08,    1218.68,    8.87,    1.59},
{1883.50,    1218.25,    8.90,    0.67},
{1886.93,    1221.40,    8.94,    1.60},
{1883.76,    1222.30,    9.11,    6.26},
{1889.82,    1222.51,    9.03,    0.97},
{1898.23,    1217.97,    8.87,    3.42},
{1877.40,    1216.41,    8.97,    0.37},
{1893.07,    1215.26,    8.87,    3.08},
{1878.57,    1214.16,    8.87,    3.12},
{1889.94,    1212.21,    8.87,    1.59},
{1883.74,    1212.35,    8.87,    1.59},
{1877.00,    1207.27,    8.87,    3.80},
{1873.63,    1204.65,    8.87,    0.61},
{1896.46,    1205.62,    8.87,    5.72},
{1899.63,    1202.52,    8.87,    2.46},
{1889.23,    1207.72,    8.87,    1.47},
{1879.77,    1207.96,    8.87,    1.55}
};

uint32 spawnentries[8] = {7789,7787,7787,8876,7788,8877,7275,7796};

struct instance_zul_farrak : public ScriptedInstance
{
    instance_zul_farrak(Map *map) : ScriptedInstance(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    uint8 waves;
    uint32 wavecounter;
    uint64 captives[5];
    uint64 doorsGUID;
    std::list<uint64> PyramidTrashGuidList;
    std::list<uint64> m_lShallowGravesGuidList;
    uint32 PyramidEventTimer;

    void Initialize()
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;
        waves = 0;
        wavecounter = 0;
        for(uint8 i = 0; i < 5; i++)
            captives[i] = 0;
        doorsGUID = 0;
        PyramidTrashGuidList.clear();
        PyramidEventTimer = 0;
    }

    void OnGameObjectCreate(GameObject *go, bool)
    {
        if(go->GetEntry() == GO_SHALLOW_GRAVE)
            m_lShallowGravesGuidList.push_back(go->GetGUID());
        if (go->GetEntry() >= CAGES_BEGIN && go->GetEntry() <= CAGES_END)
            if (Encounter[0] > IN_PROGRESS)
                go->SetGoState(GO_STATE_ACTIVE);
        if (go->GetEntry() == DOOR_ENTRY && Encounter[1] == DONE)
        {
            go->UseDoorOrButton();
            doorsGUID = go->GetGUID();
        }
    }

    void OnCreatureCreate(Creature *c, bool)
    {
        if (c->GetEntry() >= CAPTIVES_BEGIN && c->GetEntry() <= CAPTIVES_END)
        {
            if (Encounter[0] != NOT_STARTED)
                c->ForcedDespawn();
            captives[c->GetEntry() - CAPTIVES_BEGIN] = c->GetGUID();
        }
    }
    void OnCreatureDeath(Creature* pCreature)
    {
        switch (pCreature->GetEntry())
        {
        case 7275:
        case 7787:
        case 7788:
        case 7789:
        case 7796:
        case 8876:
        case 8877:
            if (wavecounter)
                --wavecounter;
            break;
        default: break;
        }
    }
    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " " << Encounter[1];

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
        loadStream >> Encounter[0] >> Encounter[1];

        if (Encounter[0] == IN_PROGRESS)
            Encounter[0] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }

    void SetData(uint32 Data, uint32 Value)
    {
        if (Data == DATA_PYRAMID_BATTLE)
        {
            if (Value == IN_PROGRESS && Encounter[0] == NOT_STARTED)
                Encounter[0] = Value;
            /*else if (Value == SPECIAL && wavecounter)
                --wavecounter;*/
        }
        else if (Data == DATA_DOOR_EVENT && Encounter[1] != DONE)
            Encounter[1] = Value;
        if (Data == DATA_DOOR_EVENT && Value == DONE)
        {
            if (GameObject * go = instance->GetGameObject(doorsGUID))
                go->UseDoorOrButton();
        }
        if (Value >= FAIL)
            SaveToDB();

    }

    uint32 GetData(uint32 Data)
    {
        if (Data == DATA_PYRAMID_BATTLE)
            return Encounter[0];
        if (Data == DATA_DOOR_EVENT)
            return Encounter[1];
        return 0;
    }

    void GetShallowGravesGuidList(std::list<uint64>& lList) const 
    { lList = m_lShallowGravesGuidList; }

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

        debug_log("TSCR: Instance Zul Farrak: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    void Update(uint32 diff)
    {
        if (Encounter[0] == IN_PROGRESS && wavecounter == 0)
        {

            if (waves == 0)
            {
                for (uint8 i = 0; i < 5; i++)
                {
                    if (Creature *c = GetCreature(captives[i]))
                    {
                        c->SetWalk(true);
                        c->GetMotionMaster()->MovePoint(0, ZFWPs[i + 1][0], ZFWPs[i + 1][1], ZFWPs[i + 1][2], false);
                        c->SetHomePosition(ZFWPs[i + 1][0], ZFWPs[i + 1][1], ZFWPs[i + 1][2], 0);
                    }
                }
                PyramidEventTimer = 5000;

                for (uint8 j = 0; j < 4; j++)
                {
                    for (uint8 i = 0; i < 6; i++)
                    {
                        if (Player * p = GetPlayerInMap())
                        {
                            if (Creature* c = p->SummonCreature(spawnentries[i], spawns[j*6 + i][0], spawns[j*6 + i][1], spawns[j*6 + i][2], spawns[j*6 + i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000))
                            {
                                PyramidTrashGuidList.push_back(c->GetGUID());
                                c->GetMotionMaster()->MoveRandomAroundPoint(c->GetPositionX(), c->GetPositionY(), c->GetPositionZ(), 3.0);
                                wavecounter++;
                            }
                        }
                    }
                }

                if (waves == 0)
                    waves = 1;
            }
            else if (waves == 1)
            {
                for (uint8 j = 0; j < 4; j++)
                {
                    for (uint8 i = 0; i < 6; i++)
                    {
                        if (Player * p = GetPlayerInMap())
                        {
                            if (Creature* c = p->SummonCreature(spawnentries[i], spawns[j * 6 + i][0], spawns[j * 6 + i][1], spawns[j * 6 + i][2], spawns[j * 6 + i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000))
                            {
                                PyramidTrashGuidList.push_back(c->GetGUID());
                                c->GetMotionMaster()->MoveRandomAroundPoint(c->GetPositionX(), c->GetPositionY(), c->GetPositionZ(), 3.0);
                                wavecounter++;
                            }
                        }
                    }
                }

                if (waves == 1)
                    waves = 2;
            }
            else if (waves == 2)
            {
                for (uint8 j = 0; j < 4; j++)
                {
                    for (uint8 i = 0; i < 6; i++)
                    {
                        if (Player * p = GetPlayerInMap())
                        {
                            if (Creature* c = p->SummonCreature(spawnentries[i], spawns[j * 6 + i][0], spawns[j * 6 + i][1], spawns[j * 6 + i][2], spawns[j * 6 + i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000))
                            {
                                PyramidTrashGuidList.push_back(c->GetGUID());
                                c->GetMotionMaster()->MoveRandomAroundPoint(c->GetPositionX(), c->GetPositionY(), c->GetPositionZ(), 3.0);
                                wavecounter++;
                            }
                        }
                    }
                }

                if (Player * p = GetPlayerInMap())
                {
                    if (Creature* c = p->SummonCreature(spawnentries[6],spawns[24][0],spawns[24][1],spawns[24][2],spawns[24][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000))
                    {
                        PyramidTrashGuidList.push_back(c->GetGUID());
                        wavecounter++;
                    }
                    if (Creature* c = p->SummonCreature(spawnentries[7],spawns[25][0],spawns[25][1],spawns[25][2],spawns[25][3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000))
                    {
                        PyramidTrashGuidList.push_back(c->GetGUID());
                        wavecounter++;
                    }
                }
                if (waves == 2)
                    waves = 3;
            }
            else if (waves == 3)
            {
                for (uint8 i = 0; i < 5; i++)
                {
                    if (Creature *c = GetCreature(captives[i]))
                    {
                        c->GetMotionMaster()->MovePoint(2, ZFWPs[i + 6][0], ZFWPs[i + 6][1], ZFWPs[i + 6][2]);
                        c->SetHomePosition(ZFWPs[i + 6][0], ZFWPs[i + 6][1], ZFWPs[i + 6][2], 0);
                        if (c->GetEntry() == 7604 || c->GetEntry() == 7607)
                            c->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    }
                }
                Encounter[0] = DONE;
                wavecounter = 0;
                waves++;
            }

        }


        if(PyramidEventTimer <= diff)
        {
            if (PyramidTrashGuidList.empty())
            {
                PyramidEventTimer = urand(5000, 15000);
                return;
            }

            std::list<uint64>::iterator itr = PyramidTrashGuidList.begin();
            advance(itr, urand(0, PyramidTrashGuidList.size() - 1));
            uint64 selectedGuid = *itr;
            PyramidTrashGuidList.erase(itr);
            if (Creature *pTroll = instance->GetCreature(selectedGuid))
            {
                // Pick another one if already in combat or already killed
                if (pTroll->GetVictim() || !pTroll->isAlive())
                {
                    PyramidEventTimer = urand(0, 2) ? urand(5000, 15000) : 3000;
                    return;
                }
                float fX, fY, fZ;
                if (Creature* pBly = instance->GetCreatureById(7604))
                {
                    if (!pBly->isAlive())
                    {
                        PyramidEventTimer = 0;
                        return;
                    }

                    pBly->GetRandomPoint(pBly->GetPositionX(), pBly->GetPositionY(), pBly->GetPositionZ(), 4.0f, fX, fY, fZ);
                    pTroll->SetWalk(false);
                    pTroll->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
                }
            }
            PyramidEventTimer = urand(0, 2) ? urand(5000, 15000) : 3000;
        }
        else PyramidEventTimer -= diff;
    }
};

InstanceData* GetInstanceData_instance_zul_farrak(Map* map)
{
    return new instance_zul_farrak(map);
}


enum
{
    /*SAY_INTRO                   = -1209000,
    SAY_AGGRO                   = -1209001,
    SAY_KILL                    = -1209002,
    SAY_SUMMON                  = -1209003,*/

    SPELL_SHADOW_BOLT           = 12739,
    SPELL_SHADOW_BOLT_VOLLEY    = 15245,
    SPELL_WARD_OF_ZUMRAH        = 11086,
    SPELL_HEALING_WAVE          = 12491,
    SPELL_SUMMON_ZOMBIES        = 10247,            // spell should be triggered by missing trap 128972

    // NPC_WARD_OF_ZUMRAH       = 7785,
    // NPC_SKELETON_OF_ZUMRAH   = 7786,
    NPC_ZULFARRAK_ZOMBIE        = 7286,             // spawned by the graves
    NPC_ZULFARRAK_DEAD_HERO     = 7276,             // spawned by the graves
};

struct ObjectDistanceOrderGO : public std::binary_function<const WorldObject, const WorldObject, bool>
{
    const Unit* m_pSource;
    ObjectDistanceOrderGO(const Unit* pSource) : m_pSource(pSource) {};

    bool operator()(const WorldObject* pLeft, const WorldObject* pRight) const
    {
        return m_pSource->GetDistanceOrder(pLeft, pRight, false);
    }
};

struct boss_zumrahAI : public ScriptedAI
{
    boss_zumrahAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_zul_farrak*)pCreature->GetInstanceData();
    }

    instance_zul_farrak *pInstance;

    Timer ShadowBoltTimer;
    Timer ShadowBoltVolleyTimer;
    Timer WardOfZumrahTimer;
    Timer HealingWaveTimer;
    Timer SpawnZombieTimer;

    bool HasTurnedHostile;

    void Reset()
    {
        ShadowBoltTimer.Reset(1000);
        ShadowBoltVolleyTimer.Reset(urand(6000, 30000));
        WardOfZumrahTimer.Reset(urand(7000, 20000));
        HealingWaveTimer.Reset(urand(10000, 15000));
        SpawnZombieTimer.Reset(1000);
        HasTurnedHostile = false;
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, true);
        me->setFaction(35);
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        me->Yell(-1200445, LANG_UNIVERSAL, 0);
    }

    void KilledUnit(Unit* /*pVictim*/)
    {
        me->Yell("Fall!", LANG_UNIVERSAL, 0);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!HasTurnedHostile && pWho->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(pWho, 9.0f) && me->IsWithinLOSInMap(pWho))
        {
            me->setFaction(14);
            me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
            me->Say(-1200446, LANG_UNIVERSAL, 0);
            HasTurnedHostile = true;
            AttackStart(pWho);
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_ZULFARRAK_ZOMBIE || pSummoned->GetEntry() == NPC_ZULFARRAK_DEAD_HERO)
            pSummoned->AI()->AttackStart(me->GetVictim());
    }

    GameObject* SelectNearbyShallowGrave()
    {
        if (!pInstance)
            return nullptr;

        // Get the list of usable graves (not used already by players)
        std::list<uint64> lTempList;
        std::list<GameObject*> lGravesInRange;

        pInstance->GetShallowGravesGuidList(lTempList);
        for (std::list<uint64>::const_iterator itr = lTempList.begin(); itr != lTempList.end(); ++itr)
        {
            GameObject* pGo = me->GetMap()->GetGameObject(*itr);
            // Go spawned and no looting in process
            if (pGo && pGo->isSpawned() && pGo->getLootState() == GO_READY)
                lGravesInRange.push_back(pGo);
        }

        if (lGravesInRange.empty())
            return nullptr;

        // Sort the graves
        lGravesInRange.sort(ObjectDistanceOrderGO(me));

        return *lGravesInRange.begin();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpawnZombieTimer.Expired(diff))
        {
            // Use a nearby grave to spawn zombies
            if (GameObject* pGrave = SelectNearbyShallowGrave())
            {
                pGrave->Use(me);
                pGrave->SetLootState(GO_JUST_DEACTIVATED);

                SpawnZombieTimer = 20000;
            }
            else // No Grave usable any more
                SpawnZombieTimer = 0;
        }

        if (ShadowBoltTimer.Expired(diff))
        {
            if (Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 5, false))
            {
                DoCast(random, SPELL_SHADOW_BOLT, false);
                ShadowBoltTimer = urand(3500, 5000);
            }
        }

        if (ShadowBoltVolleyTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SHADOW_BOLT_VOLLEY, false);
            ShadowBoltVolleyTimer = urand(10000, 18000);
        }

        if (WardOfZumrahTimer.Expired(diff))
        {
            DoCast(me, SPELL_WARD_OF_ZUMRAH, false);
            WardOfZumrahTimer = urand(15000, 32000);
        }

        if (HealingWaveTimer.Expired(diff))
        {
            if (Unit* pTarget = SelectLowestHpFriendly(40.0f))
            {
                DoCast(pTarget, SPELL_HEALING_WAVE, false);
                HealingWaveTimer = urand(15000, 23000);
            }
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_zumrah(Creature* pCreature)
{
    return new boss_zumrahAI(pCreature);
}

void AddSC_instance_zul_farrak()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_zul_farrak";
    newscript->GetInstanceData = &GetInstanceData_instance_zul_farrak;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_zumrah";
    newscript->GetAI = &GetAI_boss_zumrah;
    newscript->RegisterSelf();
}