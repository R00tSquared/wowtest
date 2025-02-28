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
SDName: Instance_Naxxramas
SD%Complete: 0
SDComment:
SDCategory: Naxxramas
EndScriptData */

/*#############
Release TODO LIST in Naxxramas:
- Fix doors states in Plague Wing
- Add waypoints for NPC GUID 88100, 88094, 88095
- Fix Cat and Kel'Thuzad event(just for fun)
- Add gossip dialogue to NPC ID 16381(Archmage Tarsis Kir-Moldir)
- Fix teleports: they should work like traps, correct positions
- Fix gates for Maexxna
- Fix Four Horseman's chest(loot)
- Add gates for Heigan
##############*/

#include "precompiled.h"
#include "def_naxxramas.h"
#include "InstanceSaveMgr.h"

std::multimap<uint32, uint32> boss_groups = {
    {0,DATA_ANUB_REKHAN,             },
    {0,DATA_GRAND_WIDOW_FAERLINA,    },
    {0,DATA_MAEXXNA,                 },
    {1,DATA_NOTH_THE_PLAGUEBRINGER,  },
    {1,DATA_HEIGAN_THE_UNCLEAN,      },
    {1,DATA_LOATHEB,                 },
    {2,DATA_INSTRUCTOR_RAZUVIOUS,    },
    {2,DATA_GOTHIK_THE_HARVESTER,    },
    {2,DATA_THE_FOUR_HORSEMEN,       },
    {3,DATA_PATCHWERK,               },
    {3,DATA_GROBBULUS,               },
    {3,DATA_GLUTH,                   },
    {3,DATA_THADDIUS,                },
};

uint32 boss_quarters[4] = 
{
    16657, // 0 Spider Wing Bosses
    16658, // 1 Plague Wing Bosses
    16659, // 2 Deathknight Wing
    16660, // 3 Abomination Wing
};

// This spawns 5 corpse scarabs ontop of us (most likely the player casts this on death)
#define SPELL_SELF_SPAWN_5  29105

#define GO_FOUR_HORSEMEN_CHEST 181366
#define GO_FOUR_HORSEMEN_CHEST_HEROIC 9481366

#define THADDIUS_LAMENT_1   8873
#define THADDIUS_LAMENT_2   8874
#define THADDIUS_LAMENT_3   8875
#define THADDIUS_LAMENT_4   8876

instance_naxxramas::instance_naxxramas(Map* map) : ScriptedInstance(map), m_gbk(map) {Initialize();}

void instance_naxxramas::Initialize()
{
    for (uint8 i = 0; i < ENCOUNTERS; ++i)
        Encounters[i] = NOT_STARTED;

    HorsemanChestSpawned = false;
    m_thaddiusGUID  = 0;
    m_stalaggGUID   = 0;
    m_feugenGUID    = 0;
    m_gothikGUID    = 0;
    deadHorsemans   = 0;
    SapphironGUID   = 0;
    KelThuzadGUID   = 0;
    SapphironGate   = 0;
    ThaddiusGate    = 0;
    GluthGate       = 0;
    PatchwerkGate   = 0;
    GothEntryGate   = 0;
    GothExitGate    = 0;
    GothCombatGate  = 0;
    NothEntryDoor   = 0;
    NothExitDoor    = 0;
    DeathknightDoor = 0;

    time_t reset_time = sInstanceSaveManager.GetResetTimefor(MAP_NAXX, false);
    std::tm* tm = std::localtime(&reset_time);

    custom_data = (tm->tm_yday + 1) % 4;
    sLog.outLog(LOG_SPECIAL, "Naxxramas set quater %u", custom_data);

    screemTimer = urand(3*MINUTE*MILLISECONDS, 5*MINUTE*MILLISECONDS);
}

bool instance_naxxramas::IsEncounterInProgress() const
{
    for (uint8 i = 0; i < ENCOUNTERS; ++i)
        if (Encounters[i] == IN_PROGRESS)
            return true;

    if(Encounters[DATA_GOTHIK_THE_HARVESTER] == SPECIAL)
        return true;

    return false;
}

uint32 instance_naxxramas::GetEncounterForEntry(uint32 entry)
{
    switch (entry)
    {
        case 15956:
            return DATA_ANUB_REKHAN;
        case 15953:
            return DATA_GRAND_WIDOW_FAERLINA;
        case 15952:
            return DATA_MAEXXNA;
        case 15954:
            return DATA_NOTH_THE_PLAGUEBRINGER;
        case 15936:
            return DATA_HEIGAN_THE_UNCLEAN;
        case 16011:
            return DATA_LOATHEB;
        case 16061:
            return DATA_INSTRUCTOR_RAZUVIOUS;
        case 16060:
            return DATA_GOTHIK_THE_HARVESTER;
        case 16064:
        case 16065:
        case 16063:
        case 16062:
            return DATA_THE_FOUR_HORSEMEN;
        case 16028:
            return DATA_PATCHWERK;
        case 15931:
            return DATA_GROBBULUS;
        case 15932:
            return DATA_GLUTH;
        case 15928:
        case 15929:
        case 15930:
            return DATA_THADDIUS;
        case 15989:
            return DATA_SAPPHIRON;
        case 15990:
            return DATA_KEL_THUZAD;
        default:
            return 0;
    }
}

void instance_naxxramas::OnCreatureCreate(Creature *creature, uint32 creature_entry)
{
    switch (creature_entry)
    {
        case 15928:
            m_thaddiusGUID = creature->GetGUID();
            break;
        case 15929:
            m_stalaggGUID = creature->GetGUID();
            break;
        case 15930:
            m_feugenGUID = creature->GetGUID();
            break;
        case 15989:
            SapphironGUID = creature->GetGUID();
            break;
        case 16060:
            m_gothikGUID = creature->GetGUID();
        case 16137:
            m_lGothTriggerList.push_back(creature->GetGUID());
            break;
        case 16028: // Patchwerk
            creature->setActive(true);
            creature->LoadPath(882980);
            creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
            creature->GetMotionMaster()->Initialize();
            break;
        case 15990:
            KelThuzadGUID = creature->GetGUID();
            for (const auto& boss : boss_groups)
            {
                if ((creature->GetMap()->IsHeroicRaid() || GetCustomData() == boss.first) && GetData(boss.second) != DONE) 
                {
                    creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                    break;
                }
            }

            break;
        default:
            break;
    }

    HandleInitCreatureState(creature);
}

void instance_naxxramas::OnGameObjectCreate(GameObject* pGo, bool add)
{
    switch(pGo->GetEntry())
    {
        case GO_BIRTH:
            if (!add && SapphironGUID)
            {
                if (Creature *pSapphiron = instance->GetCreature(SapphironGUID))
                    pSapphiron->AI()->DoAction(DATA_SAPPHIRON_BIRTH);
                    return;
            }
        case GO_SAPPHIRON_DOOR:
            SapphironGate = pGo->GetGUID();
            break;
        case GO_CONS_PATH_EXIT_DOOR:
            PatchwerkGate = pGo->GetGUID();
            if(GetData(DATA_PATCHWERK) == DONE)
                HandleGameObject(PatchwerkGate, true);
            break;
        case GO_CONS_GLUT_EXIT_DOOR:
            GluthGate = pGo->GetGUID();
            if(GetData(DATA_GLUTH) == DONE)
                HandleGameObject(GluthGate, true);
            break;
        case GO_CONS_THAD_DOOR:
            ThaddiusGate = pGo->GetGUID();
            if(GetData(DATA_THADDIUS) == DONE)
                HandleGameObject(ThaddiusGate, true);
            break;
        // Military Quarter
        case GO_MILI_GOTH_ENTRY_GATE:
            GothEntryGate = pGo->GetGUID();
            break;
        case GO_MILI_GOTH_EXIT_GATE:
            GothExitGate = pGo->GetGUID();
            break;
        case GO_MILI_GOTH_COMBAT_GATE:
            GothCombatGate = pGo->GetGUID();
            break;
        case GO_MILI_HORSEMEN_DOOR:
            DeathknightDoor = pGo->GetGUID();
        // Plague Quarter
        case GO_PLAG_NOTH_ENTRY_DOOR:
            NothEntryDoor = pGo->GetGUID();
            break;
        case GO_PLAG_NOTH_EXIT_DOOR:
            NothExitDoor = pGo->GetGUID();
            if (GetData(DATA_NOTH_THE_PLAGUEBRINGER) == DONE)
                HandleGameObject(NothExitDoor, true);
            break;
        case GO_PLAG_HEIG_ENTRY_DOOR:
            HeiganEntryDoor = pGo->GetGUID();
            break;
        //case GO_PLAG_HEIG_EXIT_DOOR:
        //    HeiganExitDoor = pGo->GetGUID();
        //    if (GetData(DATA_HEIGAN_THE_UNCLEAN) == DONE)
        //        HandleGameObject(HeiganExitDoor, true);
        //    break;
        default:
            // Heigan Traps - many different entries which are only required for sorting
            if (pGo->GetGoType() == GAMEOBJECT_TYPE_TRAP)
            {
                uint32 GoEntry = pGo->GetEntry();

                switch(GoEntry)
                {
                    case 181517:
                        m_alHeiganTrapGuids[0].push_back(pGo->GetGUID());
                        break;
                    case 181518:
                        m_alHeiganTrapGuids[1].push_back(pGo->GetGUID());
                        break;
                    case 181519:
                        m_alHeiganTrapGuids[2].push_back(pGo->GetGUID());
                        break;
                    case 181520:
                        m_alHeiganTrapGuids[3].push_back(pGo->GetGUID());
                        break;
                }
            }
            break;
    }
}

void instance_naxxramas::OnCreatureDeath(Creature* creature)
{
    switch(creature->GetEntry())
    {
        case NPC_LADY_BLAUMEUX_N:
        case NPC_HIGHLORD_MOGRAINE_N:
        case NPC_SIR_ZELIEK_N:
        case NPC_THANE_KORTHAZZ_N:
        {
            if (deadHorsemans == 3)
            {
                SetData(DATA_THE_FOUR_HORSEMEN, DONE);

                while (Creature* voidzone = GetClosestCreatureWithEntry(creature, 16697, 100))
                    voidzone->ForcedDespawn(0);
            }
            else
                deadHorsemans++;
            break;
        }
    }
}

void instance_naxxramas::SetData(uint32 type, uint32 data)
{
    switch (type)
    {
        case DATA_ANUB_REKHAN:
            if (Encounters[0] != DONE)
                Encounters[0] = data;
            break;
        case DATA_GRAND_WIDOW_FAERLINA:
            if (Encounters[1] != DONE)
                Encounters[1] = data;
            break;
        case DATA_MAEXXNA:
            if (Encounters[2] != DONE)
                Encounters[2] = data;
            break;
        case DATA_NOTH_THE_PLAGUEBRINGER:
            switch (data)
            {
                case DONE:
                    HandleGameObject(NothEntryDoor, true);
                    HandleGameObject(NothExitDoor, true);
                    Encounters[3] = DONE;
                    break;
                case IN_PROGRESS:
                    HandleGameObject(NothEntryDoor, false);
                    HandleGameObject(NothExitDoor, false);
                    break;
                default:
                    HandleGameObject(NothEntryDoor, true);
                    if (Encounters[3] != DONE)
                        Encounters[3] = data;
                    break;
            }
            break;
        case DATA_HEIGAN_THE_UNCLEAN:
            switch (data)
            {
            case DONE:
                HandleGameObject(HeiganEntryDoor, true);
                //HandleGameObject(HeiganExitDoor, true);
                Encounters[4] = DONE;
                break;
            case IN_PROGRESS:
                HandleGameObject(HeiganEntryDoor, false);
                //HandleGameObject(HeiganExitDoor, false);
                break;
            default:
                HandleGameObject(HeiganEntryDoor, true);
                if (Encounters[4] != DONE)
                    Encounters[4] = data;
                break;
            }
        case DATA_LOATHEB:
            if (Encounters[5] != DONE)
                Encounters[5] = data;
            break;
        case DATA_INSTRUCTOR_RAZUVIOUS:
            if(data == DONE)
                HandleGameObject(GothEntryGate, true);

            if (Encounters[6] != DONE)
                Encounters[6] = data;
            break;
        case DATA_GOTHIK_THE_HARVESTER:
            switch (data)
            {
                case IN_PROGRESS:
                    HandleGameObject(GothEntryGate, false);
                    HandleGameObject(GothCombatGate, false);
                    break;
                case SPECIAL:
                    HandleGameObject(GothCombatGate, true);
                    break;
                case FAIL:
                    HandleGameObject(GothCombatGate, true);
                    HandleGameObject(GothEntryGate, true);
                    break;
                case DONE:
                    HandleGameObject(GothEntryGate, true);
                    HandleGameObject(GothExitGate, true);
                    HandleGameObject(DeathknightDoor, true);
                    Encounters[7] = DONE;
                    break;
                default:
                if (Encounters[7] != DONE)
                    Encounters[7] = data;
                break;
            }
            break;
        case DATA_THE_FOUR_HORSEMEN:
            switch (data)
            {
                case DONE:
                    if (deadHorsemans == 3)
                    {
                        Encounters[8] = data;
                        if (Player *player = GetPlayer())
                        {
                            if(!HorsemanChestSpawned)
                            {
                                player->SummonGameObject(GO_FOUR_HORSEMEN_CHEST, 2514, -2945, 245.5, 5.48, 0, 0, 0, 0, 0);
                                HorsemanChestSpawned = true;
                            }

                            if(Creature *TeleportTrigger = player->SummonTrigger(2493.02, -2921.78, 241.19, 3.14, 0, nullptr, true))
                                TeleportTrigger->SummonGameObject(GO_TELEPORT_NAX_WORKING, 2493.02, -2921.78, 241.19, 3.14, 0, 0, 0, 0, 0);
                        }
                        // Despawn spirits
                        if (Creature* pSpirit = instance->GetCreatureById(NPC_SPIRIT_OF_BLAUMEUX))
                            pSpirit->ForcedDespawn();
                        if (Creature* pSpirit = instance->GetCreatureById(NPC_SPIRIT_OF_MOGRAINE))
                            pSpirit->ForcedDespawn();
                        if (Creature* pSpirit = instance->GetCreatureById(NPC_SPIRIT_OF_KORTHAZZ))
                            pSpirit->ForcedDespawn();
                        if (Creature* pSpirit = instance->GetCreatureById(NPC_SPIRIT_OF_ZELIREK))
                            pSpirit->ForcedDespawn();
                    }
                    break;
                case NOT_STARTED:
                    if (Encounters[8] != DONE)
                    {
                        Encounters[8] = data;
                        deadHorsemans = 0;
                    }
                    break;
                default:
                    if (Encounters[8] != DONE)
                        Encounters[8] = data;
                    break;
            }
            break;
        case DATA_PATCHWERK:
            if (data == DONE)
                HandleGameObject(PatchwerkGate, true);
            if (Encounters[9] != DONE)
                Encounters[9] = data;
            break;
        case DATA_GROBBULUS:
            if (Encounters[10] != DONE)
                Encounters[10] = data;
            break;
        case DATA_GLUTH:
            if (data == DONE)
            {
                HandleGameObject(GluthGate, true);
                HandleGameObject(ThaddiusGate,true);
            }
            if (Encounters[11] != DONE)
                Encounters[11] = data;
            break;
        case DATA_THADDIUS:
            if (data == IN_PROGRESS)
                HandleGameObject(ThaddiusGate,false);
            if (data == FAIL || data == DONE)
                HandleGameObject(ThaddiusGate,true);
            if (Encounters[12] != DONE)
                Encounters[12] = data;
            break;
        case DATA_SAPPHIRON:
            if (data == DONE)
            {
                HandleGameObject(SapphironGate, true);

                if (Creature *ktz = GetCreature(GetData64(DATA_KEL_THUZAD)))
                    ktz->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            }               
            if (Encounters[13] != DONE)
                Encounters[13] = data;
            break;
        case DATA_KEL_THUZAD:
            if (Encounters[14] != DONE)
                Encounters[14] = data;
            break;
        case DATA_SAPPHIRON_DIALOG:
            Encounters[15] = data;
            break;
    }

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

    if (data == DONE)
        SaveToDB();
}

void instance_naxxramas::OnPlayerDealDamage(Player* plr, uint32 amount)
{
    m_gbk.DamageDone(plr->GetGUIDLow(), amount);
}

void instance_naxxramas::OnPlayerHealDamage(Player* plr, uint32 amount)
{
    m_gbk.HealingDone(plr->GetGUIDLow(), amount);
}

uint32 instance_naxxramas::GetData(uint32 type)
{
    switch (type)
    {
        case DATA_ANUB_REKHAN:
            return Encounters[0];
        case DATA_GRAND_WIDOW_FAERLINA:
            return Encounters[1];
        case DATA_MAEXXNA:
            return Encounters[2];
        case DATA_NOTH_THE_PLAGUEBRINGER:
            return Encounters[3];
        case DATA_HEIGAN_THE_UNCLEAN:
            return Encounters[4];
        case DATA_LOATHEB:
            return Encounters[5];
        case DATA_INSTRUCTOR_RAZUVIOUS:
            return Encounters[6];
        case DATA_GOTHIK_THE_HARVESTER:
            return Encounters[7];
        case DATA_THE_FOUR_HORSEMEN:
            return Encounters[8];
        case DATA_PATCHWERK:
            return Encounters[9];
        case DATA_GROBBULUS:
            return Encounters[10];
        case DATA_GLUTH:
            return Encounters[11];
        case DATA_THADDIUS:
            return Encounters[12];
        case DATA_SAPPHIRON:
            return Encounters[13];
        case DATA_KEL_THUZAD:
            return Encounters[14];
        case DATA_SAPPHIRON_DIALOG:
            return Encounters[15];
        default:
            return 0;
    }
}

void instance_naxxramas::SetData64(uint32 type, uint64 data){}

uint64 instance_naxxramas::GetData64(uint32 identifier)
{
    switch (identifier)
    {
        case DATA_THADDIUS:
            return m_thaddiusGUID;
        case DATA_STALAGG:
            return m_stalaggGUID;
        case DATA_FEUGEN:
            return m_feugenGUID;
        case DATA_GOTHIK_THE_HARVESTER:
            return m_gothikGUID;
        case DATA_KEL_THUZAD:
            return KelThuzadGUID;
        default:
            return 0;
    }
}

std::string instance_naxxramas::GetSaveData()
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
    stream << Encounters[7] << " ";
    stream << Encounters[8] << " ";
    stream << Encounters[9] << " ";
    stream << Encounters[10] << " ";
    stream << Encounters[11] << " ";
    stream << Encounters[12] << " ";
    stream << Encounters[13] << " ";
    stream << Encounters[14];

    OUT_SAVE_INST_DATA_COMPLETE;

    return stream.str();
}

void instance_naxxramas::OnPlayerDeath(Player * plr)
{
    if (GetData(DATA_ANUB_REKHAN) == IN_PROGRESS)
        plr->CastSpell(plr, SPELL_SELF_SPAWN_5, true);

    m_gbk.PlayerDied(plr->GetGUIDLow());
}

void instance_naxxramas::Load(const char* in)
{
    if (!in)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(in);
    std::istringstream stream(in);
    stream  >> Encounters[0]
            >> Encounters[1]
            >> Encounters[2]
            >> Encounters[3]
            >> Encounters[4]
            >> Encounters[5]
            >> Encounters[6]
            >> Encounters[7]
            >> Encounters[8]
            >> Encounters[9]
            >> Encounters[10]
            >> Encounters[11]
            >> Encounters[12]
            >> Encounters[13]
            >> Encounters[14];

    for (uint8 i = 0; i < ENCOUNTERS; ++i)
        if (Encounters[i] == IN_PROGRESS) // Do not load an encounter as "In Progress" - reset it instead.
            Encounters[i] = NOT_STARTED;

    OUT_LOAD_INST_DATA_COMPLETE;
}

Player* instance_naxxramas::GetPlayer()
{
    Map::PlayerList const &players = instance->GetPlayers();
    for(Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
    {
        if (i->getSource())
            return i->getSource();
    }
    return NULL;
}

void instance_naxxramas::SetGothTriggers()
{
    Creature* pGoth = GetCreature(GetData64(DATA_GOTHIK_THE_HARVESTER));

    if (!pGoth)
        return;

    if (m_lGothTriggerList.empty())
        return;

    for (std::list<uint64>::const_iterator itr = m_lGothTriggerList.begin(); itr != m_lGothTriggerList.end(); ++itr)
    {
        if (Creature* pTrigger = instance->GetCreature(*itr))
        {
            GothTrigger pGt;
            pGt.bIsAnchorHigh = (pTrigger->GetPositionZ() >= (pGoth->GetPositionZ() - 5.0f));
            pGt.bIsRightSide = IsInRightSideGothArea(pTrigger);

            m_mGothTriggerMap[pTrigger->GetGUID()] = pGt;
        }
    }
}

Creature* instance_naxxramas::GetClosestAnchorForGoth(Creature* pSource, bool bRightSide)
{
    std::list<Creature*> lList;

    for (std::map<uint64, GothTrigger>::iterator itr = m_mGothTriggerMap.begin(); itr != m_mGothTriggerMap.end(); ++itr)
    {
        if (!itr->second.bIsAnchorHigh)
            continue;

        if (itr->second.bIsRightSide != bRightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr->first))
            lList.push_back(pCreature);
    }

    if (!lList.empty())
    {
        lList.sort(Hellground::ObjectDistanceOrder(pSource));
        return lList.front();
    }

    return NULL;
}

void instance_naxxramas::GetGothSummonPointCreatures(std::list<Creature*>& lList, bool bRightSide)
{
    if(m_mGothTriggerMap.empty())
        return;

    for (std::map<uint64, GothTrigger>::iterator itr = m_mGothTriggerMap.begin(); itr != m_mGothTriggerMap.end(); ++itr)
    {
        if (itr->second.bIsAnchorHigh)
            continue;

        if (itr->second.bIsRightSide != bRightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr->first))
            lList.push_back(pCreature);
    }
}

bool instance_naxxramas::IsInRightSideGothArea(Unit* pUnit)
{
    if (GameObject* pCombatGate = GameObject::GetGameObject(*pUnit, GetData64(GO_MILI_GOTH_COMBAT_GATE)))
        return (pCombatGate->GetPositionY() >= pUnit->GetPositionY());

    return true;
}

void instance_naxxramas::DoTriggerHeiganTraps(uint32 uiAreaIndex, uint64 pControllerGUID)
{
    if (Unit *target = (instance->GetPlayers().begin() != instance->GetPlayers().end()) ? instance->GetPlayers().begin()->getSource() : NULL)
    {
        for (std::list<uint64>::const_iterator itr = m_alHeiganTrapGuids[uiAreaIndex].begin(); itr != m_alHeiganTrapGuids[uiAreaIndex].end(); ++itr)
        {
            if (GameObject* pTrap = instance->GetGameObject(*itr))
            {
                pTrap->CastSpell(target, 29371, pControllerGUID);
                pTrap->SendCustomAnimation();
            }
        }
    }
}

void instance_naxxramas::Update(uint32 diff)
{
    if (screemTimer <= diff)
    {
        if (GetData(DATA_THADDIUS) != DONE)
        {
            uint32 sound = RAND(THADDIUS_LAMENT_1, THADDIUS_LAMENT_2, THADDIUS_LAMENT_3, THADDIUS_LAMENT_4);

            Map::PlayerList const &players = instance->GetPlayers();
            for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                if (Player *player = i->getSource())
                    player->SendPlaySound(sound, true);
        }

        screemTimer = urand(3*MINUTE*MILLISECONDS, 5*MINUTE*MILLISECONDS);
    }
    else
        screemTimer -= diff;
}

InstanceData* GetInstanceData_instance_naxxramas(Map* map)
{
    return new instance_naxxramas(map);
}

/*####
## at_naxxramas_stage
####*/

bool AreaTrigger_at_naxxramas_stage(Player* player, AreaTriggerEntry const* trigger)
{
    ScriptedInstance* pInstance = (player->GetInstanceData());
    
    if (pInstance)
    {
        bool heroic_mode = player->GetMap()->IsHeroicRaid();

        uint32 bosses_left = 0;
        for (auto& boss : boss_groups)
        {
            if (!heroic_mode && pInstance->GetCustomData() != boss.first)
                continue;

            if (pInstance->GetData(boss.second) != DONE)
                ++bosses_left;
        }

        if (bosses_left == 0)
        {
            player->TeleportTo(533, 3498.28, -5349.9, 144.968, 1.31324);
            return false;
        }

        if (heroic_mode)
			ChatHandler(player).PSendSysMessage(16744, bosses_left);
        else
        {
            const char* msg = ChatHandler(player).PGetSysMessage(boss_quarters[pInstance->GetCustomData()]);
            ChatHandler(player).PSendSysMessage(15524, msg, bosses_left);
        }

        return false;
    }

    return false;
}

bool GOUse_go_naxxramas_teleport(Player *player, GameObject* go)
{
    player->TeleportTo(533, 3005.87, -3435.01, 294.882, 0);
    return true;
}

void AddSC_instance_naxxramas()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_naxxramas";
    newscript->GetInstanceData = &GetInstanceData_instance_naxxramas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_naxxramas_stage";
    newscript->pAreaTrigger = &AreaTrigger_at_naxxramas_stage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_naxxramas_teleport";
    newscript->pGOUse = &GOUse_go_naxxramas_teleport;
    newscript->RegisterSelf();
}
