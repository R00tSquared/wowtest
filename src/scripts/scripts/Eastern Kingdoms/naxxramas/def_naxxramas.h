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

#ifndef DEF_NAXXRAMAS_H
#define DEF_NAXXRAMAS_H

#define DATA_ANUB_REKHAN                0
#define DATA_GRAND_WIDOW_FAERLINA       1
#define DATA_MAEXXNA                    2
#define DATA_NOTH_THE_PLAGUEBRINGER     3
#define DATA_HEIGAN_THE_UNCLEAN         4
#define DATA_LOATHEB                    5
#define DATA_INSTRUCTOR_RAZUVIOUS       6
#define DATA_GOTHIK_THE_HARVESTER       7
#define DATA_THE_FOUR_HORSEMEN          8
#define DATA_PATCHWERK                  9
#define DATA_GROBBULUS                  10
#define DATA_GLUTH                      11
#define DATA_THADDIUS                   12
#define DATA_SAPPHIRON                  13
#define DATA_KEL_THUZAD                 14
#define DATA_STALAGG                    15
#define DATA_FEUGEN                     16
#define DATA_SAPPHIRON_BIRTH            17
#define DATA_SAPPHIRON_DIALOG           18

enum GameObjectsIds
{
    // Frostwyrm Lair
    GO_BIRTH                    = 181356,
    GO_SAPPHIRON_DOOR           = 181225,
    // Construct Quarter
    GO_CONS_GLUT_EXIT_DOOR      = 181120,
    GO_CONS_THAD_DOOR           = 181121,
    GO_CONS_PATH_EXIT_DOOR      = 181123,
    GO_CONS_NOX_TESLA_FEUGEN    = 181477,
    GO_CONS_NOX_TESLA_STALAGG   = 181478,
    // Military Quarter
    GO_MILI_GOTH_ENTRY_GATE     = 181124,                   // used while encounter is in progress
    GO_MILI_GOTH_EXIT_GATE      = 181125,                   // exit, open at boss dead
    GO_MILI_GOTH_COMBAT_GATE    = 181170,                   // used while encounter is in progress
    GO_MILI_HORSEMEN_DOOR       = 181119,                   // encounter door
    // Plague Quarter
    GO_PLAG_SLIME01_DOOR        = 181198,                   // not used
    GO_PLAG_SLIME02_DOOR        = 181199,                   // not used
    GO_PLAG_NOTH_ENTRY_DOOR     = 181200,                   // encounter door
    GO_PLAG_NOTH_EXIT_DOOR      = 181201,                   // exit, open when boss dead
    GO_PLAG_HEIG_ENTRY_DOOR     = 181202,
    GO_PLAG_HEIG_EXIT_DOOR      = 181203,                   // exit door - not used here
    GO_PLAG_HEIG_EXIT_HALLWAY   = 181496,                   // exit door, at the end of the hallway
    GO_PLAG_LOAT_DOOR           = 181241,                   // encounter door
    // Teleports
    GO_TELEPORT_MAEXXNA         = 181575,
    GO_TELEPORT_THADDIUS        = 181576,
    GO_TELEPORT_LOATHEB         = 181577,
    GO_TELEPORT_HORSEMAN        = 181578,
    GO_TELEPORT_NAX_WORKING     = 181588
    
};

enum CreatureEntry
{
    NPC_KELTHUZAD                   = 15990,
    NPC_THE_LICH_KING               = 16980,
    NPC_MOUTH_OF_KELTHUZAD          = 16995,
    NPC_THADDIUS                    = 15928,
    NPC_STALAGG                     = 15929,
    NPC_FEUGEN                      = 15930,
    NPC_TESLA_COIL                  = 16218,
    // Gothik
    NPC_GOTHIK                      = 16060,
    NPC_SUB_BOSS_TRIGGER            = 16137,                    // summon locations
    NPC_UNREL_TRAINEE               = 16124,
    NPC_UNREL_DEATH_KNIGHT          = 16125,
    NPC_UNREL_RIDER                 = 16126,
    NPC_SPECT_TRAINEE               = 16127,
    NPC_SPECT_DEATH_KNIGHT          = 16148,
    NPC_SPECT_RIDER                 = 16150,
    NPC_SPECT_HORSE                 = 16149,
    // Four Horseman
    NPC_LADY_BLAUMEUX_N             = 16065,
    NPC_HIGHLORD_MOGRAINE_N         = 16062,
    NPC_SIR_ZELIEK_N                = 16063,
    NPC_THANE_KORTHAZZ_N            = 16064,
    NPC_SPIRIT_OF_MOGRAINE          = 16775,
    NPC_SPIRIT_OF_BLAUMEUX          = 16776,
    NPC_SPIRIT_OF_ZELIREK           = 16777,
    NPC_SPIRIT_OF_KORTHAZZ          = 16778
};

#define ENCOUNTERS  17
#define MAX_HEIGAN_TRAP_AREAS    4

struct GothTrigger
{
    bool bIsRightSide;
    bool bIsAnchorHigh;
};

struct instance_naxxramas : public ScriptedInstance
{
    instance_naxxramas(Map *map);

    uint32 Encounters[ENCOUNTERS];

    bool HorsemanChestSpawned;
    uint8 deadHorsemans;

    uint64 m_thaddiusGUID;
    uint64 m_stalaggGUID;
    uint64 m_feugenGUID;
    uint64 m_gothikGUID;
    uint64 m_lady_blaumeuxGUID;
    uint64 m_thane_korthazzGUID;
    uint64 m_sir_zeliekGUID;
    uint64 m_highlord_mograineGUID;
    uint64 SapphironGUID;
    uint64 KelThuzadGUID;
    uint64 SapphironGate;
    uint64 ThaddiusGate;
    uint64 GluthGate;
    uint64 PatchwerkGate;
    uint64 GothEntryGate;
    uint64 GothExitGate;
    uint64 GothCombatGate;
    uint64 NothEntryDoor;
    uint64 NothExitDoor;
    uint64 HeiganEntryDoor;
    //uint64 HeiganExitDoor;
    uint64 DeathknightDoor;

    uint32 custom_data; //current_boss_group
    GBK_handler m_gbk;

    std::list<uint64> m_lGothTriggerList;
    std::map<uint64, GothTrigger> m_mGothTriggerMap;
    std::list<uint64> m_alHeiganTrapGuids[MAX_HEIGAN_TRAP_AREAS];

    uint64 screemTimer;
    
    void Initialize();
    bool IsEncounterInProgress() const;
    uint32 GetEncounterForEntry(uint32 entry);
    void OnCreatureCreate(Creature *creature, uint32 creature_entry);
    void OnGameObjectCreate(GameObject* pGo, bool add);
    void OnCreatureDeath(Creature* creature);
    void SetData(uint32 type, uint32 data);
    uint32 GetData(uint32 type);
    void SetData64(uint32 type, uint64 data);
    uint64 GetData64(uint32 identifier);
    std::string GetSaveData();
    void OnPlayerDeath(Player * plr);
    void OnPlayerHealDamage(Player* plr, uint32 amount);
    void OnPlayerDealDamage(Player* plr, uint32 amount);
    void Load(const char* in);
    Player* GetPlayer();
    void SetGothTriggers();
    Creature* GetClosestAnchorForGoth(Creature* pSource, bool bRightSide);
    void GetGothSummonPointCreatures(std::list<Creature*>& lList, bool bRightSide);
    bool IsInRightSideGothArea(Unit* pUnit);
    void DoTriggerHeiganTraps(uint32 uiAreaIndex, uint64 pControllerGUID);
    void Update(uint32 diff);
    bool CheckEncounters();
    uint32 GetCustomData() { return custom_data; };

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
        case DATA_ANUB_REKHAN: return GBK_ANUB_REKHAN;
        case DATA_GRAND_WIDOW_FAERLINA: return GBK_GRAND_WIDOW_FAERLINA;
        case DATA_MAEXXNA: return GBK_MAEXXNA;
        case DATA_NOTH_THE_PLAGUEBRINGER: return GBK_NOTH_THE_PLAGUEBRINGER;
        case DATA_HEIGAN_THE_UNCLEAN: return GBK_HEIGAN_THE_UNCLEAN;
        case DATA_GLUTH: return GBK_GLUTH;
        case DATA_GROBBULUS: return GBK_GROBBULUS;
        case DATA_THADDIUS: return GBK_THADDIUS;
        case DATA_SAPPHIRON: return GBK_SAPPHIRON;
        case DATA_KEL_THUZAD: return GBK_KEL_THUZAD;
        case DATA_LOATHEB: return GBK_LOATHEB;
        case DATA_INSTRUCTOR_RAZUVIOUS: return GBK_INSTRUCTOR_RAZUVIOUS;
        case DATA_GOTHIK_THE_HARVESTER: return GBK_GOTHIK_THE_HARVESTER;
        case DATA_THE_FOUR_HORSEMEN: return GBK_FOUR_HORSEMEN;
        case DATA_PATCHWERK: return GBK_PATCHWERK;
        }
        return GBK_NONE;
    }
};

extern std::multimap<uint32, uint32> boss_groups;
extern uint32 boss_quarters[4];
#endif
