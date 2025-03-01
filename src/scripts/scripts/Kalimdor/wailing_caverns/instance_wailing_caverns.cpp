// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

/* ScriptData
SDName: Instance_Wailing_Caverns
SD%Complete: 99
SDComment: Everything seems to work, still need some checking
SDCategory: Wailing Caverns
EndScriptData */

#include "precompiled.h"
#include "def_wailing_caverns.h"

#define MAX_ENCOUNTER   9

struct instance_wailing_caverns : public ScriptedInstance
{
    instance_wailing_caverns(Map* pMap) : ScriptedInstance(pMap) {Initialize();};

    uint32 m_auiEncounter[MAX_ENCOUNTER];

    bool yelled;
    uint64 NaralexGUID;

    void Initialize()
    {
        memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

        yelled = false;
        NaralexGUID = 0;
    }

    void OnCreatureCreate(Creature* pCreature, bool /*add*/)
    {
        if (pCreature->GetEntry() == DATA_NARALEX)
            NaralexGUID = pCreature->GetGUID();
    }

    void SetData(uint32 type, uint32 data)
    {
        switch (type)
        {
            case TYPE_LORD_COBRAHN:         m_auiEncounter[0] = data;break;
            case TYPE_LORD_PYTHAS:          m_auiEncounter[1] = data;break;
            case TYPE_LADY_ANACONDRA:       m_auiEncounter[2] = data;break;
            case TYPE_LORD_SERPENTIS:       m_auiEncounter[3] = data;break;
            case TYPE_NARALEX_EVENT:        m_auiEncounter[4] = data;break;
            case TYPE_NARALEX_PART1:        m_auiEncounter[5] = data;break;
            case TYPE_NARALEX_PART2:        m_auiEncounter[6] = data;break;
            case TYPE_NARALEX_PART3:        m_auiEncounter[7] = data;break;
            case TYPE_MUTANUS_THE_DEVOURER: m_auiEncounter[8] = data;break;
            case TYPE_NARALEX_YELLED:       yelled = true;      break;
        }
        if (data == DONE)SaveToDB();
    }

    uint32 GetData(uint32 type)
    {
        switch (type)
        {
            case TYPE_LORD_COBRAHN:         return m_auiEncounter[0];
            case TYPE_LORD_PYTHAS:          return m_auiEncounter[1];
            case TYPE_LADY_ANACONDRA:       return m_auiEncounter[2];
            case TYPE_LORD_SERPENTIS:       return m_auiEncounter[3];
            case TYPE_NARALEX_EVENT:        return m_auiEncounter[4];
            case TYPE_NARALEX_PART1:        return m_auiEncounter[5];
            case TYPE_NARALEX_PART2:        return m_auiEncounter[6];
            case TYPE_NARALEX_PART3:        return m_auiEncounter[7];
            case TYPE_MUTANUS_THE_DEVOURER: return m_auiEncounter[8];
            case TYPE_NARALEX_YELLED:       return yelled;
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        if (data == DATA_NARALEX)return NaralexGUID;
        return 0;
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << m_auiEncounter[0] << " ";
        stream << m_auiEncounter[1] << " ";
        stream << m_auiEncounter[2] << " ";
        stream << m_auiEncounter[3] << " ";
        stream << m_auiEncounter[4] << " ";
        stream << m_auiEncounter[5] << " ";
        stream << m_auiEncounter[6] << " ";
        stream << m_auiEncounter[7] << " ";
        stream << m_auiEncounter[8];

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
        loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
        >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7] >> m_auiEncounter[8];

        for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
            if (m_auiEncounter[i] != DONE)
                m_auiEncounter[i] = NOT_STARTED;

        OUT_LOAD_INST_DATA_COMPLETE;
    }

};

InstanceData* GetInstanceData_instance_wailing_caverns(Map* pMap)
{
    return new instance_wailing_caverns(pMap);
}

void AddSC_instance_wailing_caverns()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_wailing_caverns";
    newscript->GetInstanceData = &GetInstanceData_instance_wailing_caverns;
    newscript->RegisterSelf();
}
