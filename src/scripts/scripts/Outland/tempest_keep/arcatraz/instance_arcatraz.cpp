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
SDName: Instance_Arcatraz
SD%Complete: 80
SDComment: Mainly Harbringer Skyriss event
SDCategory: Tempest Keep, The Arcatraz
EndScriptData */

#include "precompiled.h"
#include "def_arcatraz.h"

static const DialogueEntry aArcatrazDialogue[] =
{
    // Soccothares taunts
    {TYPE_DALLIAH,            0,             2000},
    {SAY_SOCCOTHRATES_AGGRO,  NPC_SOCCOTHRATES, 0},
    {TYPE_SOCCOTHRATES,       0,             1000},
    {SAY_SOCCOTHRATES_DEATH,  NPC_SOCCOTHRATES, 0},
    {0, 0, 0},
};

/* Arcatraz encounters:
1 - Zereketh the Unbound event
2 - Dalliah the Doomsayer event
3 - Wrath-Scryer Soccothrates event
4 - Harbinger Skyriss event, 5 sub-events
*/

struct instance_arcatraz : public ScriptedInstance, private DialogueHelper
{
    instance_arcatraz(Map *map) : ScriptedInstance(map), m_gbk(map), DialogueHelper(aArcatrazDialogue)
    {
        Initialize();
    };

    uint32 Encounter[ENCOUNTERS];
    GBK_handler m_gbk;

    uint64 Containment_Core_Security_Field_Alpha;
    uint64 Containment_Core_Security_Field_Beta;
    uint64 Pod_Alpha;
    uint64 Pod_Gamma;
    uint64 Pod_Beta;
    uint64 Pod_Delta;
    uint64 Pod_Omega;

    uint64 GoSphereGUID;
    uint64 MellicharGUID;

    Timer Timer_IntroEvent;
    uint64 KilledWardens;

    void Initialize()
    {
        InitializeDialogueHelper(this);
        Containment_Core_Security_Field_Alpha = 0;
        Containment_Core_Security_Field_Beta  = 0;
        Pod_Alpha = 0;
        Pod_Beta  = 0;
        Pod_Delta = 0;
        Pod_Gamma = 0;
        Pod_Omega = 0;

        GoSphereGUID = 0;
        MellicharGUID = 0;
        Timer_IntroEvent.Reset(0);
        KilledWardens = 0;

        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; i++)
            if(Encounter[i] == IN_PROGRESS)
                return true;

        return false;
    }

    void OnPlayerEnter(Player* plr)
    {
        if (GetData(TYPE_INTROEVENT) == DONE || GetData(TYPE_INTROEVENT) == SPECIAL)
        return;

        SetData(TYPE_INTROEVENT, SPECIAL);
        Timer_IntroEvent = 1000;
    }

    void OnObjectCreate(GameObject *go)
    {
        switch(go->GetEntry())
        {
            case CONTAINMENT_CORE_SECURITY_FIELD_ALPHA:
                Containment_Core_Security_Field_Alpha = go->GetGUID();
                if (GetData(TYPE_SOCCOTHRATES) == DONE)
                    HandleGameObject(0, true, go);
                break;
            case CONTAINMENT_CORE_SECURITY_FIELD_BETA:
                Containment_Core_Security_Field_Beta =  go->GetGUID();
                if (GetData(TYPE_DALLIAH) == DONE)
                    HandleGameObject(0, true, go);
                break;
            case SEAL_SPHERE: GoSphereGUID = go->GetGUID(); break;
            case POD_ALPHA: Pod_Alpha = go->GetGUID(); break;
            case POD_BETA:  Pod_Beta =  go->GetGUID(); break;
            case POD_DELTA: Pod_Delta = go->GetGUID(); break;
            case POD_GAMMA: Pod_Gamma = go->GetGUID(); break;
            case POD_OMEGA: Pod_Omega = go->GetGUID(); break;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        if (creature->GetEntry() == MELLICHAR)
            MellicharGUID = creature->GetGUID();
    }

    void OnCreatureDeath(Creature* pCreature)
    {
        if (pCreature->GetEntry() == NPC_ARCATRAZ_WARDEN || pCreature->GetEntry() == NPC_ARCATRAZ_DEFENDER)
        {
            ++KilledWardens;

            // Stop the intro spawns when the wardens are killed
            if (KilledWardens == MAX_WARDENS)
            {
                SetData(TYPE_INTROEVENT, DONE);
                Timer_IntroEvent = 0;
            }
        }
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case TYPE_ZEREKETH:         return GBK_AR_ZEREKETH;
            case TYPE_DALLIAH:          return GBK_AR_DALLIAH;
            case TYPE_SOCCOTHRATES:     return GBK_AR_SOCCOTHRATES;
            case TYPE_HARBINGERSKYRISS: return GBK_AR_SKYRISS;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch (type)
        {
            case TYPE_INTROEVENT:
                Encounter[9] = data;
                break;
            case TYPE_ZEREKETH:
                Encounter[0] = data;
                break;

            case TYPE_SOC_DAL_INTRO:
                Encounter[8] = data;
                break;

            case TYPE_DALLIAH:
                if (data == IN_PROGRESS)
                {
                    // Soccothares taunts after Dalliah gets aggro
                    StartNextDialogueText(TYPE_DALLIAH);
                }
                if (data == DONE)
                {
                   HandleGameObject(Containment_Core_Security_Field_Beta, true);
                   // Soccothares taunts after Dalliah dies
                    if (GetData(TYPE_SOCCOTHRATES) != DONE)
                        StartNextDialogueText(TYPE_SOCCOTHRATES);
                }
                Encounter[1] = data;
                break;

            case TYPE_SOCCOTHRATES:
                if (data == DONE)
                    HandleGameObject(Containment_Core_Security_Field_Alpha, true);
                Encounter[2] = data;
                break;

            case TYPE_HARBINGERSKYRISS:
                if (data == NOT_STARTED || data == FAIL)
                {
                    Encounter[4] = NOT_STARTED;
                    Encounter[5] = NOT_STARTED;
                    Encounter[6] = NOT_STARTED;
                    Encounter[7] = NOT_STARTED;
                    HandleGameObject(Pod_Alpha,false);
                    HandleGameObject(Pod_Beta,false);
                    HandleGameObject(Pod_Gamma,false);
                    HandleGameObject(Pod_Delta,false);
                    HandleGameObject(Pod_Omega,false);
                    HandleGameObject(GoSphereGUID,true);
                    if (GetCreature(MellicharGUID) && GetCreature(MellicharGUID)->isDead())
                        GetCreature(MellicharGUID)->Respawn();
                }
                Encounter[3] = data;
                break;

            case TYPE_WARDEN_1:;
                Encounter[4] = data;
                break;

            case TYPE_WARDEN_2:;
                Encounter[5] = data;
                break;

            case TYPE_WARDEN_3:
                Encounter[6] = data;
                break;

            case TYPE_WARDEN_4:
                Encounter[7] = data;
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

        if(data == DONE)
            SaveToDB();
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

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " ";
        stream << Encounter[1] << " ";
        stream << Encounter[2]  << " ";
        stream << Encounter[3]  << " ";
        stream << Encounter[4]  << " ";
        stream << Encounter[5]  << " ";
        stream << Encounter[6]  << " ";
        stream << Encounter[7]  << " ";
        stream << Encounter[8]  << " ";
        stream << Encounter[9];

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
        stream >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3] >> Encounter[4] >> Encounter[5] >> Encounter[6]
            >> Encounter[7] >> Encounter[8] >> Encounter[9];
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;

        if(GetData(TYPE_HARBINGERSKYRISS) == NOT_STARTED)
            SetData(TYPE_HARBINGERSKYRISS, NOT_STARTED);            // this will reset whole encounter
        OUT_LOAD_INST_DATA_COMPLETE;
    }

    uint32 GetData(uint32 type)
    {
         switch(type)
        {
            case TYPE_ZEREKETH:
                return Encounter[0];
            case TYPE_DALLIAH:
                return Encounter[1];
            case TYPE_SOCCOTHRATES:
                return Encounter[2];
            case TYPE_HARBINGERSKYRISS:
                return Encounter[3];
            case TYPE_WARDEN_1:
                return Encounter[4];
            case TYPE_WARDEN_2:
                return Encounter[5];
            case TYPE_WARDEN_3:
                return Encounter[6];
            case TYPE_WARDEN_4:
                return Encounter[7];
            case TYPE_SOC_DAL_INTRO:
                return Encounter[8];
            case TYPE_INTROEVENT:
                return Encounter[9];
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch(data)
        {
            case DATA_MELLICHAR:
                return MellicharGUID;
            case DATA_SPHERE_SHIELD:
                return GoSphereGUID;
            case DATA_POD_A:
                return Pod_Alpha;
            case DATA_POD_B:
                return Pod_Beta;
            case DATA_POD_D:
                return Pod_Delta;
            case DATA_POD_G:
                return Pod_Gamma;
            case DATA_POD_O:
                return Pod_Omega;
        }
        return 0;
    }
    
    Creature* GetSpeakerByEntry(uint32 uiEntry)
    {
        switch (uiEntry)
        {
            case NPC_SOCCOTHRATES:      return instance->GetCreatureById(NPC_SOCCOTHRATES);;
            default: return NULL;
        }
    
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
        return NULL;
    }

    void Update(uint32 diff)
    {
        DialogueUpdate(diff);

        if (Timer_IntroEvent.Expired(diff))
        {
            Player* pPlayer = GetPlayerInMap();
            if (!pPlayer)
                return;
    
            uint32 CreatureEntry = urand(0, 10) ? NPC_PROTEAN_HORROR : NPC_PROTEAN_NIGHTMARE;
    
            // Summon and move the intro creatures into combat positions
            if (Creature* pTemp = pPlayer->SummonCreature(CreatureEntry, EntranceSpawnLoc[0], EntranceSpawnLoc[1], EntranceSpawnLoc[2], EntranceSpawnLoc[3], TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000))
            {
                pTemp->SetWalk(false);
                pTemp->GetMotionMaster()->MovePoint(0, EntranceMoveLoc[0], EntranceMoveLoc[1], EntranceMoveLoc[2]);
            }
            Timer_IntroEvent = urand(5000, 12000);
        }
    }
};

InstanceData* GetInstanceData_instance_arcatraz(Map* map)
{
    return new instance_arcatraz(map);
}

void AddSC_instance_arcatraz()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_arcatraz";
    newscript->GetInstanceData = &GetInstanceData_instance_arcatraz;
    newscript->RegisterSelf();
}
