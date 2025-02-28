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
SDName: Instance_Steam_Vault
SD%Complete: 80
SDComment:  Instance script and access panel GO
SDCategory: Coilfang Resevoir, The Steamvault
EndScriptData */

#include "precompiled.h"
#include "def_steam_vault.h"

#define ENCOUNTERS 3

#define MAIN_CHAMBERS_DOOR      183049
#define ACCESS_PANEL_HYDRO      184125
#define ACCESS_PANEL_MEK        184126

/* Steam Vaults encounters:
1 - Hydromancer Thespia Event
2 - Mekgineer Steamrigger Event
3 - Warlord Kalithresh Event
*/

bool GOUse_go_main_chambers_access_panel(Player *player, GameObject* _GO)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)_GO->GetInstanceData();

    if (!pInstance)
        return false;

    // Do the opening Emote
    if (_GO->GetEntry() == ACCESS_PANEL_HYDRO || _GO->GetEntry() == ACCESS_PANEL_MEK)
    {
        if (pInstance->GetData(TYPE_HYDROMANCER_THESPIA) != SPECIAL && pInstance->GetData(TYPE_MEKGINEER_STEAMRIGGER) != SPECIAL)
            _GO->TextEmote(-1200485, 0);
        else
            _GO->TextEmote(-1200486, 0);
    }

    if (_GO->GetEntry() == ACCESS_PANEL_HYDRO && (pInstance->GetData(TYPE_HYDROMANCER_THESPIA) == DONE || pInstance->GetData(TYPE_HYDROMANCER_THESPIA) == SPECIAL))
        pInstance->SetData(TYPE_HYDROMANCER_THESPIA, SPECIAL);

    if (_GO->GetEntry() == ACCESS_PANEL_MEK && (pInstance->GetData(TYPE_MEKGINEER_STEAMRIGGER) == DONE || pInstance->GetData(TYPE_MEKGINEER_STEAMRIGGER) == SPECIAL))
        pInstance->SetData(TYPE_MEKGINEER_STEAMRIGGER, SPECIAL);
    return true;
}

struct instance_steam_vault : public ScriptedInstance
{
    instance_steam_vault(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    GBK_handler m_gbk;

    uint64 ThespiaGUID;
    uint64 MekgineerGUID;
    uint64 KalithreshGUID;

    uint64 MainChambersDoor;
    uint64 AccessPanelHydro;
    uint64 AccessPanelMek;
    std::list<uint64> GuardsGUIDList;

    void Initialize()
    {
        ThespiaGUID = 0;
        MekgineerGUID = 0;
        KalithreshGUID = 0;
        MainChambersDoor = 0;
        AccessPanelHydro = 0;
        AccessPanelMek = 0;
        GuardsGUIDList.clear();

        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS-1; i++)
            if (Encounter[i] == IN_PROGRESS)
                return true;

        return false;
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch(creature->GetEntry())
        {
            case 17797: ThespiaGUID = creature->GetGUID(); break;
            case 17796: MekgineerGUID = creature->GetGUID(); break;
            case 17798: KalithreshGUID = creature->GetGUID(); break;
        }
        switch(creature->GetDBTableGUIDLow())
        {
            case 12637:
            case 12636:
            case 12615:
            case 12638:
            {
                GuardsGUIDList.push_back(creature->GetGUID());
                creature->SetVisibility(VISIBILITY_OFF);
                creature->SetReactState(REACT_PASSIVE);
                creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
                break;
            }
        }
    }

    void OnObjectCreate(GameObject *go)
    {
        switch(go->GetEntry())
        {
            case MAIN_CHAMBERS_DOOR:
                MainChambersDoor = go->GetGUID();
                if(GetData(TYPE_HYDROMANCER_THESPIA) == SPECIAL && GetData(TYPE_MEKGINEER_STEAMRIGGER) == SPECIAL)
                {
                    HandleGameObject(MainChambersDoor, true);
                    if(!GuardsGUIDList.empty())
                    {
                        for (std::list<uint64>::iterator iter = GuardsGUIDList.begin(); iter != GuardsGUIDList.end(); ++iter)
                        {
                            if(Creature* guard = instance->GetCreature(*iter))
                            {
                                guard->SetVisibility(VISIBILITY_ON);
                                guard->SetReactState(REACT_AGGRESSIVE);
                                guard->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                            }
                        }
                    }
                }
                break;
            case ACCESS_PANEL_HYDRO:
                AccessPanelHydro = go->GetGUID();
                break;
            case ACCESS_PANEL_MEK:
                AccessPanelMek = go->GetGUID();
                break;
        }
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case TYPE_HYDROMANCER_THESPIA:      return GBK_SV_HYDROMANCER_THEPSIA;
            case TYPE_MEKGINEER_STEAMRIGGER:    return GBK_SV_MEKGINEER_STEAM;
            case TYPE_WARLORD_KALITHRESH:       return GBK_SV_WARLORD_KALITHRESH;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case TYPE_HYDROMANCER_THESPIA:
                if (data == SPECIAL)
                {
                    HandleGameObject(AccessPanelHydro, true);
                    if (GetData(TYPE_MEKGINEER_STEAMRIGGER) == SPECIAL)
                    {
                        HandleGameObject(MainChambersDoor, true);
                        if(!GuardsGUIDList.empty())
                        {
                            for (std::list<uint64>::iterator iter = GuardsGUIDList.begin(); iter != GuardsGUIDList.end(); ++iter)
                            {
                                if(Creature* guard = instance->GetCreature(*iter))
                                {
                                    guard->SetVisibility(VISIBILITY_ON);
                                    guard->SetReactState(REACT_AGGRESSIVE);
                                    guard->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                                }
                            }
                        }
                    }
                    debug_log("TSCR: Instance Steamvault: Access panel used.");
                }
                Encounter[0] = data;
                break;
            case TYPE_MEKGINEER_STEAMRIGGER:
                if (data == SPECIAL)
                {
                    HandleGameObject(AccessPanelMek, true);
                    if (GetData(TYPE_HYDROMANCER_THESPIA) == SPECIAL)
                    {
                        HandleGameObject(MainChambersDoor, true);
                        if(!GuardsGUIDList.empty())
                        {
                            for (std::list<uint64>::iterator iter = GuardsGUIDList.begin(); iter != GuardsGUIDList.end(); ++iter)
                            {
                                if(Creature* guard = instance->GetCreature(*iter))
                                {
                                    guard->SetVisibility(VISIBILITY_ON);
                                    guard->SetReactState(REACT_AGGRESSIVE);
                                    guard->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                                }
                            }
                        }
                    }
                    debug_log("TSCR: Instance Steamvault: Access panel used.");
                }
                Encounter[1] = data;
                break;
            case TYPE_WARLORD_KALITHRESH:
                switch (data)
                {
                    case NOT_STARTED:
                    case DONE:
                        if (GetData(TYPE_MEKGINEER_STEAMRIGGER) == SPECIAL && GetData(TYPE_HYDROMANCER_THESPIA) == SPECIAL)
                        {
                            HandleGameObject(MainChambersDoor, true);
                            if(!GuardsGUIDList.empty())
                            {
                                for (std::list<uint64>::iterator iter = GuardsGUIDList.begin(); iter != GuardsGUIDList.end(); ++iter)
                                {
                                    if(Creature* guard = instance->GetCreature(*iter))
                                    {
                                        guard->SetVisibility(VISIBILITY_ON);
                                        guard->SetReactState(REACT_AGGRESSIVE);
                                        guard->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                                    }
                                }
                            }
                        }
                        break;
                    case IN_PROGRESS:
                        HandleGameObject(MainChambersDoor, false);
                        break;
                }

                Encounter[2] = data;
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

        if(data == DONE || data == SPECIAL)
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

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
            case TYPE_HYDROMANCER_THESPIA:
                return Encounter[0];
            case TYPE_MEKGINEER_STEAMRIGGER:
                return Encounter[1];
            case TYPE_WARLORD_KALITHRESH:
                return Encounter[2];
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch(data)
        {
            case DATA_THESPIA:
                return ThespiaGUID;
            case DATA_MEKGINEERSTEAMRIGGER:
                return MekgineerGUID;
            case DATA_KALITRESH:
                return KalithreshGUID;
            case DATA_MAIN_CHAMBERS_DOOR:
                return MainChambersDoor;
        }
        return 0;
    }

   std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " ";
        stream << Encounter[1] << " ";
        stream << Encounter[2];

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
        stream >> Encounter[0] >> Encounter[1] >> Encounter[2];
        for(uint8 i = 0; i < ENCOUNTERS-1; ++i)
            if(Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_steam_vault(Map* map)
{
    return new instance_steam_vault(map);
}

void AddSC_instance_steam_vault()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "go_main_chambers_access_panel";
    newscript->pGOUse = &GOUse_go_main_chambers_access_panel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "instance_steam_vault";
    newscript->GetInstanceData = &GetInstanceData_instance_steam_vault;
    newscript->RegisterSelf();
}
