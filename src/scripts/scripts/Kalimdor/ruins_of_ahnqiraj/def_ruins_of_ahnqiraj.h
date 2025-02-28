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

#ifndef DEF_RUINS_OF_AHNQIRAJ_H
#define DEF_RUINS_OF_AHNQIRAJ_H

enum InstanceRoAQ
{
    MAX_ENCOUNTERS                  = 6,
    MAX_HELPERS                     = 4,

    DATA_KURINNAXX                  = 0,
    DATA_GENERAL_RAJAXX             = 1,
    DATA_MOAM                       = 2,
    DATA_BURU_THE_GORGER            = 3,
    DATA_AYAMISS_THE_HUNTER         = 4,
    DATA_OSSIRIAN_THE_UNSCARRED     = 5,

    // Rajax encounter
    NPC_RAJAXX                      = 15341,
    NPC_SWARMGUARD_NEEDLER          = 15344,
    NPC_COLONEL_ZERRAN              = 15385,
    NPC_MAJOR_YEGGETH               = 15386,
    NPC_QIRAJI_WARRIOR              = 15387,
    NPC_MAJOR_PAKKON                = 15388,
    NPC_CAPTAIN_DRENN               = 15389,
    NPC_CAPTAIN_XURREM              = 15390,
    NPC_CAPTAIN_QEEZ                = 15391,
    NPC_CAPTAIN_TUUBID              = 15392,
    NPC_GENERAL_ANDOROV             = 15471,
    NPC_KALDOREI_ELITE              = 15473,
    MAX_ARMY_WAVES                  = 7,
    SAY_WAVE3                       = -1509005,
    SAY_WAVE4                       = -1509006,
    SAY_WAVE5                       = -1509007,
    SAY_WAVE6                       = -1509008,
    SAY_WAVE7                       = -1509009,
    SAY_INTRO                       = -1509010,
    SAY_DEAGGRO                     = -1509015,                 // on Rajaxx evade
};

struct SpawnLocation
{
    uint32 Entry;
    float m_fX, m_fY, m_fZ, m_fO;
    uint32 SpecialFlags;
};

// Spawn coords for Andorov and his team
static const SpawnLocation aAndorovSpawnLocs[MAX_HELPERS] =
{
    {NPC_KALDOREI_ELITE,  -8876.187, 1654.478, 21.38, 5.51},
    {NPC_KALDOREI_ELITE,  -8879.444, 1650.783, 21.38, 5.51},
    {NPC_KALDOREI_ELITE,  -8881.749, 1650.180, 21.38, 5.51},
    {NPC_KALDOREI_ELITE,  -8875.478, 1656.449, 21.38, 5.51}
};

// Movement locations for Andorov
static const SpawnLocation aAndorovMoveLocs[] =
{
    {0, -8882.15f, 1602.77f, 21.386f},
    {0, -8940.45f, 1550.69f, 21.616f}, // Before the arc
};

static const SpawnLocation AllRajaxWaves[] =
{
    {15344, -9022.04, 1612.05, 22.8073, 3.15905, 1},
    {15344, -8998.96, 1560.49, 22.0967, 3.21628, 0}, // 1st wave
    {15344, -8998.41, 1543.04, 21.8216, 3.06862, 0}, // 1st wave
    {15344, -9089.96, 1520,    21.3862, 2.21447, 1},
    {15344, -9083.14, 1524.76, 21.4697, 2.14675, 1},
    {15344, -9076.42, 1530.23, 21.4697, 2.23402, 1},
    {15344, -9066.22, 1621.18, 21.3874, 3.69114, 1},
    {15344, -9070.33, 1627.91, 21.3874, 3.85797, 1},
    {15344, -9075.64, 1634.99, 21.3871, 3.99251, 1},
    {15344, -9038.44, 1491.04, 23.2296, 2.26893, 1},
    {15344, -9031.99, 1592.26, 21.4697, 2.98451, 1},
    {15344, -9029.94, 1498.08, 22.1401, 2.33874, 1},
    {15344, -9022.71, 1505.88, 21.5596, 2.40855, 1},
    {15344, -9021.05, 1598.01, 21.3861, 2.56757, 1},
    {15344, -9037.07, 1599.34, 21.387,  2.68696, 1},
    {15344, -9030.72, 1610.7,  21.4034, 2.61784, 1},
    {15344, -9071.02, 1535.34, 21.679,  2.2938 , 1},
    {15344, -9063.18, 1613.94, 21.6647, 3.54977, 1},
    {15344, -9115.02, 1543.44, 21.3875, 1.52737, 1},
    {15344, -9129.72, 1545,    21.386,  1.43312, 1},
    {15344, -9103.11, 1569.96, 21.3958, 2.49939, 1},
    {15344, -9097.46, 1577.83, 21.3876, 2.47191, 1},
    {15344, -9092.34, 1586.39, 21.3861, 2.56615, 1},
    {15344, -9089.66, 1525.94, 21.3862, 2.18688, 1},
    {15387, -8998.39, 1551.71, 22.1049, 3.17308, 0}, // 1st wave
    {15387, -9007.2,  1560.21, 21.6472, 3.14638, 0}, // 1st wave
    {15387, -9007.71, 1542.14, 21.3866, 3.2257 , 0}, // 1st wave
    {15387, -9007.32, 1552.11, 21.4679, 3.1974 , 0}, // 1st wave
    {15387, -9068.87, 1615.43, 21.3998, 3.57333, 1},
    {15387, -9035.73, 1505.45, 21.4963, 2.26123, 1},
    {15387, -9028.79, 1511.05, 21.3976, 2.31228, 1},
    {15387, -9043.47, 1498.77, 21.8811, 2.20232, 1},
    {15387, -9027.1,  1601.97, 21.387,  2.69089, 1},
    {15387, -9077.42, 1536.52, 21.3862, 2.34396, 1},
    {15387, -9076.88, 1629.39, 21.3867, 3.64009, 1},
    {15387, -9112.04, 1538.12, 21.6402, 1.4841 , 1},
    {15387, -9118.84, 1538.33, 21.3945, 1.4881 , 1},
    {15387, -9127.07, 1539.02, 21.3873, 1.4881 , 1},
    {15387, -9134.44, 1539.88, 21.3873, 1.4881 , 1},
    {15387, -9103.23, 1581.89, 21.388,  2.70858, 1},
    {15387, -9098.11, 1590.06, 21.3865, 2.5728 , 1},
    {15387, -9108.75, 1574.07, 21.4048, 2.4993 , 1},
    {15385, -9109.44, 1585.79, 21.388,  2.56406, 1},
    {15392, -9039.74, 1510.3,  21.406,  2.26123, 1},
    {15391, -9014.5,  1552.65, 21.3866, 3.04035, 0}, // 1st wave
    {15389, -9037.66, 1606.4,  21.3865, 2.76078, 1},
    {15390, -9086.33, 1534.53, 21.3862, 2.30076, 1},
    {15386, -9076.47, 1620.4,  21.3867, 3.75397, 1},
    {15388, -9121.77, 1547.73, 21.386,  1.46453, 1},
};
                                        
static const int32 aArmySortingParameters[MAX_ARMY_WAVES] = {0, 0,  SAY_WAVE3, SAY_WAVE4,  SAY_WAVE5, SAY_WAVE6, SAY_WAVE7};

class instance_ruins_of_ahnqiraj : public ScriptedInstance
{
    public:
        instance_ruins_of_ahnqiraj(Map* pMap);
        ~instance_ruins_of_ahnqiraj() {}

        void Initialize();
        bool IsEncounterInProgress() const;
        uint32 GetEncounterForEntry(uint32 entry);
        void OnCreatureCreate(Creature *creature, uint32 creature_entry);
        void OnCreatureDeath(Creature* creature);
        uint64 GetData64(uint32 identifier);
        void SetData(uint32 type, uint32 data);
        uint32 GetData(uint32 type);
        std::string GetSaveData();

        void GetKaldoreiGuidList(std::list<uint64>& lList) { lList = KaldoreiGuidList; }
        void InsertGuidInArmyList(uint64 CreatureGuid);
        void Update(uint32 diff);
        Player* GetPlayerInMap();
        void DoSpawnAndorovIfCan();
        void Load(const char* in);

    private:
        void DoSendNextArmyWave();

        uint32 Encounters[MAX_ENCOUNTERS];

        uint64 KurinaxxGUID;
        uint64 BuruGUID;
        uint64 GeneralRajaxGUID;

        std::list<uint64> KaldoreiGuidList;
        std::set<uint64> ArmyWavesGuids;

        uint32 ArmyDelayTimer;
        uint8 CurrentArmyWave;
};

#endif
