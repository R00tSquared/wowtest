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
SDName: Instance_Slave_Pens
SD%Complete: 100
SDComment: Instance Data with save to database for Ahune, Slave Pens
SDCategory: Slave Pens
EndScriptData */

#include "precompiled.h"
#include "def_slave_pens.h"

struct instance_slave_pens : public ScriptedInstance
{
    instance_slave_pens(Map* map) : ScriptedInstance(map), m_gbk(map)
    {
        Heroic = map->IsHeroic();
        Initialize();
    };

    uint64 AhuneGUID;
    uint64 MennuGUID;
    uint64 RokmarGUID;
    uint64 QuagmirranGUID;
    uint64 IceStoneGUID;
    uint32 IceStoneUsableTimer;
    bool Heroic;

    uint32 Encounters[ENCOUNTERS];
    GBK_handler m_gbk;

    void Initialize()
    {
        AhuneGUID = 0;
        MennuGUID = 0;
        RokmarGUID = 0;
        QuagmirranGUID = 0;
        IceStoneGUID = 0;
        IceStoneUsableTimer = 0;

        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            Encounters[i] = NOT_STARTED;
    }

    bool IsEncounterInProgress() const
    {
        for(uint8 i = 0; i < ENCOUNTERS; ++i)
            if(Encounters[i] == IN_PROGRESS)
                return true;

        return false;
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
    }

    void OnObjectCreate(GameObject* go) //placeholder for GO
    {
        switch(go->GetEntry())
        {
        case 187882: // ice stone ahune
            IceStoneGUID = go->GetGUID();
            break;
        default:
            break;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        if(Heroic)
        {
            switch(creature_entry)
            {
                case 25740:
                    AhuneGUID = creature->GetGUID();
                    if(instance && instance->GetGameObject(IceStoneGUID))
                    {
                    instance->GetGameObject(IceStoneGUID)->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                    instance->GetGameObject(IceStoneGUID)->SetLootState(GO_JUST_DEACTIVATED);
                    }
                    break;
            }
        }
        switch(creature_entry)
        {
            case 17941: MennuGUID = creature->GetGUID(); break;
            case 17991: RokmarGUID = creature->GetGUID(); break;
            case 17942: QuagmirranGUID = creature->GetGUID(); break;
        }
    }

    uint64 GetData64(uint32 identifier)
    {
        if(Heroic)
        {
            switch(identifier)
            {
                case DATA_AHUNEEVENT:   return AhuneGUID;
            }
        }
        switch(identifier)
        {
            case DATA_MENNU:        return MennuGUID;
            case DATA_ROKMAR:       return RokmarGUID;
            case DATA_QUAGMIRRAN:   return QuagmirranGUID;
        }
        return 0;
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case DATA_AHUNEEVENT:   return GBK_SP_AHUNE;
            case DATA_MENNU:        return GBK_SP_MENNU;
            case DATA_ROKMAR:       return GBK_SP_ROKMAR;
            case DATA_QUAGMIRRAN:   return GBK_SP_QUAGMIRRAN;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch(type)
        {
        case DATA_AHUNEEVENT:
            if(Encounters[0] != DONE)
                Encounters[0] = data;
            else if (Encounters[0] == DONE)
            {
                IceStoneUsableTimer = 20000;
            }
            break;
        case DATA_MENNU:
            if(Encounters[1] != DONE)
                Encounters[1] = data;
            break;
        case DATA_ROKMAR:
            if(Encounters[2] != DONE)
                Encounters[2] = data;
            break;
        case DATA_QUAGMIRRAN:
            if(Encounters[3] != DONE)
                Encounters[3] = data;
            break;
        default:
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

        if (data == DONE)
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

	void OnCreatureDeath(Creature* creature)
	{
		if (creature->GetEntry() == 17942 || creature->GetEntry() == 19894)
			SetData(DATA_QUAGMIRRAN, DONE);
		else if (creature->GetEntry() == 17991 || creature->GetEntry() == 19895)
			SetData(DATA_ROKMAR, DONE);
	}

    uint32 GetData(uint32 type)
    {
        switch(type)
        {
            case DATA_AHUNEEVENT:       return Encounters[0];
            case DATA_MENNU:            return Encounters[1];
            case DATA_ROKMAR:           return Encounters[2];
            case DATA_QUAGMIRRAN:       return Encounters[3];
        }

        return 0;
    }

    void Update(uint32 diff)
    {
        if (instance && instance->GetGameObject(IceStoneGUID) && IceStoneUsableTimer)
        {
            if (IceStoneUsableTimer <= diff)
            {
                instance->GetGameObject(IceStoneGUID)->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                instance->GetGameObject(IceStoneGUID)->SetLootState(GO_READY);
                IceStoneUsableTimer = 0;
            }
            else IceStoneUsableTimer -= diff;
        }
    }

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounters[0] << " ";
        stream << Encounters[1] << " ";
        stream << Encounters[2] << " ";
        stream << Encounters[3];

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
        stream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3];
        for(uint8 i = 0; i < ENCOUNTERS-1; ++i)
            if(Encounters[i] == IN_PROGRESS)
                Encounters[i] = NOT_STARTED;
        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_slave_pens(Map* map)
{
    return new instance_slave_pens(map);
}

struct npc_17893AI : public ScriptedAI
{
    npc_17893AI(Creature* c) : ScriptedAI(c), summons(c)
    {
        SummonCount = 0;
        Yelled = false;
        EventDone = false;
    }

    SummonList summons;
    bool Yelled;
    uint32 SummonCount;
    bool EventDone;

    void Reset()
    {
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        if(EventDone)
            me->CastSpell(me, 34906, true);
    }

    void JustDied(Unit* victim)
    {
        SummonCount = 0;
        Yelled = false;
        EventDone = false;
        if(GameObject* cage  = FindGameObject(182094, 10, me))
                cage->SetGoState(GO_STATE_READY);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (!Yelled && pWho->GetTypeId() == TYPEID_PLAYER && !((Player*)pWho)->isGameMaster() && me->IsWithinDistInMap(pWho, 50.0f))
        {
            Yelled = true;
            me->Yell(-1200483, LANG_UNIVERSAL, 0);
        }
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        damage = damage/4;
    }

    void JustSummoned(Creature *summoned)
    {
        if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
            summoned->AI()->AttackStart(me);

        summons.Summon(summoned);
        SummonCount++;
        if(SummonCount >= 2)
            me->CastSpell(me, 34906, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_17893(Creature* _Creature)
{
    return new npc_17893AI(_Creature);
}

bool GossipHello_npc_17893(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    if (player->GetQuestStatus(9738) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(9738, 17893) < 1)
        player->KilledMonster(17893, _Creature->GetGUID());

    if(!_Creature->HasAura(34906, 0))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16434), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(9119, _Creature->GetGUID());
    }
    else
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16435), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->SEND_GOSSIP_MENU(9144, _Creature->GetGUID());
    }

    return true;
}

bool GossipSelect_npc_17893(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            CAST_AI(npc_17893AI, _Creature->AI())->EventDone = true;
            player->CLOSE_GOSSIP_MENU();
            _Creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            if(GameObject* cage  = FindGameObject(182094, 10, _Creature))
                cage->SetGoState(GO_STATE_ACTIVE);
            _Creature->GetMotionMaster()->MovePoint(2, -195.438,  -795.951,  43.799);
            _Creature->SetHomePosition(-195.438,  -795.951,  43.799, 0.94);
            _Creature->SummonCreature(17958, -179.546, -784.513, 43.79, 3.81, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000, false);
            _Creature->SummonCreature(17938, -184.940, -780.711, 43.79, 4.08, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000, false);
            if(Creature* yeller = _Creature->SummonCreature(17957, -190.757, -776.950, 43.79, 4.29, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000, false))
                yeller->Yell(-1200484, LANG_UNIVERSAL, 0);
            
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            _Creature->CastSpell(player, 34906, true);
            player->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}

void AddSC_instance_slave_pens()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_slave_pens";
    newscript->GetInstanceData = &GetInstanceData_instance_slave_pens;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_17893";
    newscript->pGossipHello = &GossipHello_npc_17893;
    newscript->pGossipSelect = &GossipSelect_npc_17893;
    newscript->GetAI = &GetAI_npc_17893;
    newscript->RegisterSelf();
}
