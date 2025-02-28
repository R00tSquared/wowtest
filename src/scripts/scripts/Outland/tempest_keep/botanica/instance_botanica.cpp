#include "precompiled.h"
#include "def_botanica.h"

struct instance_botanica : public ScriptedInstance
{
    instance_botanica(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    uint32 Encounter[ENCOUNTERS];
    GBK_handler m_gbk;

    uint64 SarannisGUID;
    uint64 FreyGUID;
    uint64 LajGUID;
    uint64 ThorngrinGUID;
    uint64 WarpSplinterGUID;

    void Initialize()
    {
        SarannisGUID = 0;
        FreyGUID = 0;
        LajGUID = 0;
        ThorngrinGUID = 0;
        WarpSplinterGUID = 0;

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
            case 17976: SarannisGUID = creature->GetGUID(); break;
            case 17975: FreyGUID = creature->GetGUID(); break;
            case 17978: ThorngrinGUID = creature->GetGUID(); break;
            case 17980: LajGUID = creature->GetGUID(); break;
            case 17977: WarpSplinterGUID = creature->GetGUID(); break;
        }
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case TYPE_SARANNIS:         return GBK_BOT_SARANNIS;
            case TYPE_FREY:             return GBK_BOT_FREY;
            case TYPE_THORNGRIN:        return GBK_BOT_THORN;
            case TYPE_LAJ:              return GBK_BOT_LAJ;
            case TYPE_WARP_SPLINTER:    return GBK_BOT_WARP;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
            case TYPE_SARANNIS:
                Encounter[0] = data;
                break;
            case TYPE_FREY:
                Encounter[1] = data;
                break;
            case TYPE_THORNGRIN:
                Encounter[2] = data;
                break;
            case TYPE_LAJ:
                Encounter[3] = data;
                break;
            case TYPE_WARP_SPLINTER:
                Encounter[4] = data;
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
            case TYPE_SARANNIS:
                return Encounter[0];
            case TYPE_FREY:
                return Encounter[1];
            case TYPE_THORNGRIN:
                return Encounter[2];
            case TYPE_LAJ:
                return Encounter[3];
            case TYPE_WARP_SPLINTER:
                return Encounter[4];
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch(data)
        {
            case TYPE_SARANNIS:
                return SarannisGUID;
            case TYPE_FREY:
                return FreyGUID;
            case TYPE_THORNGRIN:
                return ThorngrinGUID;
            case TYPE_LAJ:
                return LajGUID;
            case TYPE_WARP_SPLINTER:
                return WarpSplinterGUID;
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
        stream << Encounter[3] << " ";
        stream << Encounter[4];

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
        stream >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3] >> Encounter[4];
        for(uint8 i = 0; i < ENCOUNTERS-1; ++i)
            if(Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_botanica(Map* map)
{
    return new instance_botanica(map);
}

void AddSC_instance_botanica()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_botanica";
    newscript->GetInstanceData = &GetInstanceData_instance_botanica;
    newscript->RegisterSelf();
}
