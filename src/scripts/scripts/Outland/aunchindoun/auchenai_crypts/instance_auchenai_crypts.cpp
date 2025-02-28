#include "precompiled.h"
#include "def_auchenai_crypts.h"

struct instance_auchenai_crypts : public ScriptedInstance
{
    instance_auchenai_crypts(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    GBK_handler m_gbk;

    uint64 ExarachMaladaarGUID;
    uint64 ShirakkGUID;

    void Initialize()
    {
        ExarachMaladaarGUID = 0;
        ShirakkGUID = 0;

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
            case 17976: ShirakkGUID = creature->GetGUID(); break;
            case 17975: ExarachMaladaarGUID = creature->GetGUID(); break;
        }
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case TYPE_SHIRAKK:          return GBK_AC_SHIRAKK;
            case TYPE_EXARACH_MALADAAR: return GBK_AC_EXARACH;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case TYPE_SHIRAKK:
                Encounter[0] = data;
                break;
            case TYPE_EXARACH_MALADAAR:
                Encounter[1] = data;
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

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
            case TYPE_SHIRAKK:
                return Encounter[0];
            case TYPE_EXARACH_MALADAAR:
                return Encounter[1];
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch(data)
        {
            case TYPE_SHIRAKK:
                return ShirakkGUID;
            case TYPE_EXARACH_MALADAAR:
                return ExarachMaladaarGUID;
        }
        return 0;
    }

   std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " ";
        stream << Encounter[1];

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
        stream >> Encounter[0] >> Encounter[1];
        for(uint8 i = 0; i < ENCOUNTERS-1; ++i)
            if(Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_auchenai_crypts(Map* map)
{
    return new instance_auchenai_crypts(map);
}

void AddSC_instance_auchenai_crypts()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_auchenai_crypts";
    newscript->GetInstanceData = &GetInstanceData_instance_auchenai_crypts;
    newscript->RegisterSelf();
}
