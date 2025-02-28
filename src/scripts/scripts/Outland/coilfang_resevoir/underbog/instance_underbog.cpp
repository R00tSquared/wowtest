#include "precompiled.h"
#include "def_underbog.h"

struct instance_underbog : public ScriptedInstance
{
    instance_underbog(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    GBK_handler m_gbk;

    uint64 HungarfenGUID;
    uint64 GhahzanGUID;
    uint64 SwamplordGUID;
    uint64 ClawGUID;
    uint64 BlackStalkerGUID;

    void Initialize()
    {
        HungarfenGUID = 0;
        GhahzanGUID = 0;
        SwamplordGUID = 0;
        ClawGUID = 0;
        BlackStalkerGUID = 0;

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
            case 17770: HungarfenGUID = creature->GetGUID(); break;
            case 18105: GhahzanGUID = creature->GetGUID(); break;
            case 17826: SwamplordGUID = creature->GetGUID(); break;
            case 17827: ClawGUID = creature->GetGUID(); break;
            case 17882: BlackStalkerGUID = creature->GetGUID(); break;
        }
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case TYPE_HUNGARFEN:    return GBK_UB_HUNGARFEN;
            case TYPE_GHAHZAN:      return GBK_UB_GHAHZAN;
            case TYPE_SWAMPLORD:    return GBK_UB_SWAMPLORD;
            case TYPE_BLACKSTALKER: return GBK_UB_BLACKSTALKER;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case TYPE_HUNGARFEN:
                Encounter[0] = data;
                break;
            case TYPE_GHAHZAN:
                Encounter[1] = data;
                break;
            case TYPE_SWAMPLORD:
                Encounter[2] = data;
                break;
            case TYPE_BLACKSTALKER:
                Encounter[3] = data;
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
            case TYPE_HUNGARFEN:
                return Encounter[0];
            case TYPE_GHAHZAN:
                return Encounter[1];
            case TYPE_SWAMPLORD:
                return Encounter[2];
            case TYPE_BLACKSTALKER:
                return Encounter[3];
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch(data)
        {
            case TYPE_HUNGARFEN:
                return HungarfenGUID;
            case TYPE_GHAHZAN:
                return GhahzanGUID;
            case TYPE_SWAMPLORD:
                return SwamplordGUID;
            case TYPE_BLACKSTALKER:
                return BlackStalkerGUID;
        }
        return 0;
    }

   std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " ";
        stream << Encounter[1] << " ";
        stream << Encounter[2] << " ";
        stream << Encounter[3];

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
        stream >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3];
        for(uint8 i = 0; i < ENCOUNTERS-1; ++i)
            if(Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_underbog(Map* map)
{
    return new instance_underbog(map);
}

void AddSC_instance_underbog()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_underbog";
    newscript->GetInstanceData = &GetInstanceData_instance_underbog;
    newscript->RegisterSelf();
}
