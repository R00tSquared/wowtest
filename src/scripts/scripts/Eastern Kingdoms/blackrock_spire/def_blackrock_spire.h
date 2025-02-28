/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#ifndef SC_DEF_BLACKROCK_SPIRE_H
#define SC_DEF_BLACKROCK_SPIRE_H

enum
{
    MAX_ENCOUNTERS              = 6,

    AREATRIGGER_ENTER_UBRS      = 2046,
    AREATRIGGER_STADIUM         = 2026,

    TYPE_ROOM_EVENT             = 0,
    DATA_EMBERSEER              = 1,
    DATA_THE_BEAST              = 2,
    TYPE_FLAMEWREATH            = 3,
    TYPE_GYTH                   = 4,
    TYPE_STADIUM                = 5,

    MAX_ROOMS                   = 7,

    GO_ROOM_7_RUNE              = 175194,
    GO_ROOM_3_RUNE              = 175195,
    GO_ROOM_6_RUNE              = 175196,
    GO_ROOM_1_RUNE              = 175197,
    GO_ROOM_5_RUNE              = 175198,
    GO_ROOM_2_RUNE              = 175199,
    GO_ROOM_4_RUNE              = 175200,

    GO_EMBERSEER_IN_DOOR        = 175244,
    // 3
    GO_ROOKERY_EGG              = 175124,
    GO_FATHER_FLAME             = 175245,
    // 4
    GO_GYTH_ENTRY_DOOR          = 164726,
    GO_GYTH_COMBAT_DOOR         = 175185,
    GO_GYTH_EXIT_DOOR           = 175186,

    // 1
    NPC_BLACKHAND_SUMMONER      = 9818,
    NPC_BLACKHAND_VETERAN       = 9819,
    // 3
    NPC_SOLAKAR_FLAMEWREATH     = 10264,
    NPC_ROOKERY_GUARDIAN        = 10258,
    NPC_ROOKERY_HATCHER         = 10683,
    SAY_ROOKERY_EVENT_START     = -1229020,
    // 4
    NPC_LORD_VICTOR_NEFARIUS    = 10162,
    NPC_REND_BLACKHAND          = 10429,
    NPC_GYTH                    = 10339,
    NPC_CHROMATIC_WHELP         = 10442,
    NPC_CHROMATIC_DRAGON        = 10447,
    NPC_BLACKHAND_HANDLER       = 10742,

    MAX_STADIUM_WAVES           = 7,
    MAX_STADIUM_MOBS_PER_WAVE   = 5,

    FACTION_BLACK_DRAGON        = 103,
    // Arena event dialogue - handled by instance
    SAY_NEFARIUS_INTRO_1        = -1229004,
    SAY_NEFARIUS_INTRO_2        = -1229005,
    SAY_NEFARIUS_ATTACK_1       = -1229006,
    SAY_REND_JOIN               = -1229007,
    SAY_NEFARIUS_ATTACK_2       = -1229008,
    SAY_NEFARIUS_ATTACK_3       = -1229009,
    SAY_NEFARIUS_ATTACK_4       = -1229010,
    SAY_REND_LOSE_1             = -1229011,
    SAY_REND_LOSE_2             = -1229012,
    SAY_NEFARIUS_LOSE_3         = -1229013,
    SAY_NEFARIUS_LOSE_4         = -1229014,
    SAY_REND_ATTACK             = -1229015,
    SAY_NEFARIUS_WARCHIEF       = -1229016,
    SAY_NEFARIUS_VICTORY        = -1229018,

};

static const float rookeryEventSpawnPos[3] = {43.7685f, -259.82f, 91.6483f};

struct SpawnLocation
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static const SpawnLocation aStadiumLocs[7] =
{
    {210.00f, -420.30f, 110.94f, 3.14f},                    // dragons summon location
    {210.14f, -397.54f, 111.1f},                            // Gyth summon location
    {163.62f, -420.33f, 110.47f},                           // center of the stadium location (for movement)
    {164.63f, -444.04f, 121.97f, 3.22f},                    // Lord Nefarius summon position
    {161.01f, -443.73f, 121.97f, 6.26f},                    // Rend summon position
    {164.64f, -443.30f, 121.97f, 1.61f},                    // Nefarius move position
    {165.74f, -466.46f, 116.80f},                           // Rend move position
};

// Stadium event description
static const uint32 aStadiumEventNpcs[MAX_STADIUM_WAVES][5] =
{
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER},
};

struct instance_blackrock_spire : public ScriptedInstance, private DialogueHelper
{
    public:
        instance_blackrock_spire(Map *map);
    
        uint32 Encounters[MAX_ENCOUNTERS];
    
        // UBRS
        // 1. Event with runes. After defating creatures near runes it'll be deactivated automatically
        std::set<uint64> EmberseerInDoorGUID; // Door after rooms and before Emberseer
        uint64 RoomRuneGUID[MAX_ROOMS];
        std::list<uint64> RoomEventMobGUIDList;
        std::list<uint64> RoomEventMobGUIDSorted[MAX_ROOMS];
        // 2. Pyroguard emberseer
        uint64 emberseerOut;
        uint64 pyroguard_emberseerGUID;
        std::set<uint64> runesEmberGUID;
        std::set<uint64> channelersGUID;
        // 3. Solakar Flamewreath
        uint32 FlamewreathEventTimer;
        uint32 FlamewreathWaveCount;
        uint64 GoFatherFlameGUID;
        // 4. Gyth
        uint64 NefariusGUID;
        uint64 GythGUID;
        uint64 GythEntryDoorGUID;
        uint64 GythCombatDoorGUID;
        uint64 GythExitDoorGUID;
        uint32 StadiumEventTimer;
        uint8 StadiumWaves;
        uint32 StadiumMobsAlive;
    
    
        void Initialize();
        void OnObjectCreate(GameObject* go);
        void OnCreatureCreate(Creature *creature, uint32 entry);
        uint32 GetData(uint32 type);
        void DoSortRoomEventMobs();
        void OnCreatureDeath(Creature* creature);
        void OnCreatureEvade(Creature* pCreature);
        void SetData(uint32 type, uint32 data);
        std::string GetSaveData();
        void Load(const char* in);
        Creature* GetSpeakerByEntry(uint32 uiEntry);
        void Update(uint32 diff);
    
        void StartflamewreathEventIfCan();
        void DoSendNextFlamewreathWave();
        void DoSendNextStadiumWave(); 

    private:
        void JustDidDialogueStep(int32 iEntry);
};
#endif
