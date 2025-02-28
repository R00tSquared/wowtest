// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
SDName: Instance_Shattered_Halls
SD%Complete: 99
SDComment:
SDCategory: Hellfire Citadel, Shattered Halls
EndScriptData */

#include "precompiled.h"
#include "def_shattered_halls.h"

#define ENCOUNTERS           6

#define DOOR_NETHEKURSE1     182539
#define DOOR_NETHEKURSE2     182540
#define NPC_FEL_ORC          17083
#define NPC_NETHEKURSE       16807
#define NPC_WARBRINGER       16809
#define NPC_KARGATH          16808
#define SPELL_SHADOW_SEAR    30735

enum
{
    NPC_EXECUTIONER                = 17301,
    NPC_SOLDIER_ALLIANCE_1         = 17288,
    NPC_SOLDIER_ALLIANCE_2         = 17289,
    NPC_SOLDIER_ALLIANCE_3         = 17292,
    NPC_OFFICER_ALLIANCE           = 17290,

    NPC_SOLDIER_HORDE_1            = 17294,
    NPC_SOLDIER_HORDE_2            = 17295,
    NPC_SOLDIER_HORDE_3            = 17297,
    NPC_OFFICER_HORDE              = 17296,

    SPELL_KARGATH_EXECUTIONER_1    = 39288,
    SPELL_KARGATH_EXECUTIONER_2    = 39289,
    SPELL_KARGATH_EXECUTIONER_3    = 39290,

    SAY_KARGATH_EXECUTE_ALLY       = -1540050,
    SAY_KARGATH_EXECUTE_HORDE      = -1540049,

    SAY_EXECUTE_SOLDIER_ALLIANCE_2 = -1540054,
    SAY_EXECUTE_SOLDIER_ALLIANCE_3 = -1540056,
    SAY_EXECUTE_OFFICER_ALLIANCE   = -1540055,

    SAY_EXECUTE_SOLDIER_HORDE_2    = -1540051,
    SAY_EXECUTE_SOLDIER_HORDE_3    = -1540053,
    SAY_EXECUTE_OFFICER_HORDE      = -1540052
};

struct SpawnLocation
{
    uint32 AllianceEntry, HordeEntry;
    float fX, fY, fZ, fO;
};

const float afExecutionerLoc[4] = {151.443f, -84.439f, 1.938f, 6.283f};

static SpawnLocation aSoldiersLocs[]=
{
    {0, NPC_SOLDIER_HORDE_1, 119.609f, 256.127f, -45.254f, 5.133f},
    {NPC_SOLDIER_ALLIANCE_1, 0, 131.106f, 254.520f, -45.236f, 3.951f},
    {NPC_SOLDIER_ALLIANCE_3, NPC_SOLDIER_HORDE_3, 151.040f, -91.558f, 1.936f, 1.559f},
    {NPC_SOLDIER_ALLIANCE_2, NPC_SOLDIER_HORDE_2, 150.669f, -77.015f, 1.933f, 4.705f},
    {NPC_OFFICER_ALLIANCE, NPC_OFFICER_HORDE, 138.241f, -84.198f, 1.907f, 0.055f}
};

enum Summon
{
    NOT_SUMMONED    = 0,
    WAIT_FOR_SUMMON = 1,
    SUMMONED        = 2
};

struct instance_shattered_halls : public ScriptedInstance
{
    instance_shattered_halls(Map *map) : ScriptedInstance(map), m_gbk(map) {Initialize();};

    GBK_handler m_gbk;
    uint32 Encounter[ENCOUNTERS];
    std::list<uint64> OrcGUID;
    uint64 nethekurseGUID;
    uint64 warbringerGUID;
    uint64 nethekurseDoor1GUID;
    uint64 nethekurseDoor2GUID;
    uint64 kargathGUID;
    uint64 executionerGUID;
    uint64 officeraGUID;
    uint64 officerhGUID;
    uint64 soldiera2GUID;
    uint64 soldiera3GUID;
    uint64 soldierh2GUID;
    uint64 soldierh3GUID;
    uint64 HallOfFathersWaveOne;
    uint64 HallOfFathersWaveTwo;
    uint64 Zealot1GUID;
    uint64 Zealot2GUID;
    uint64 Zealot3GUID;
    uint64 Hunter1GUID;
    uint64 Hunter2GUID;
    uint64 GladiatorGUID[16];

	Timer ExecutionTimer;
    uint32 Team;
    uint8 ExecutionStage;

    Summon summon;

    void Initialize()
    {
        nethekurseGUID = 0;
        warbringerGUID = 0;
        nethekurseDoor1GUID = 0;
        nethekurseDoor2GUID = 0;
        kargathGUID = 0;
        executionerGUID = 0;
        officeraGUID = 0;
        officerhGUID = 0;
        soldiera2GUID = 0;
        soldiera3GUID = 0;
        soldierh2GUID = 0;
        soldierh3GUID = 0;
        HallOfFathersWaveOne = 0;
        HallOfFathersWaveTwo = 0;
        Zealot1GUID = 0;
        Zealot2GUID = 0;
        Zealot3GUID = 0;
        Hunter1GUID = 0;
        Hunter2GUID = 0;

        Team = 0;
        ExecutionStage = 0;
        ExecutionTimer = 0;

        summon = NOT_SUMMONED;

        for(uint8 i = 0; i < ENCOUNTERS; i++)
            Encounter[i] = NOT_STARTED;

        for(uint8 i = 0; i < 16; i++)
            GladiatorGUID[i] = 0;
    }

    void OnPlayerEnter(Player* player)
    {
        if (!instance->IsHeroic())
            return;

        if (summon == NOT_SUMMONED)
            summon = WAIT_FOR_SUMMON;
    }

    bool IsEncounterInProgress() const
    {
        for (uint8 i = 0; i < ENCOUNTERS; ++i)
            if (Encounter[i] == IN_PROGRESS)
                return true;
        return false;
    }

    Player* GetPlayerInMap()
    {
        Map::PlayerList const& players = instance->GetPlayers();

        if (!players.isEmpty())
        {
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* plr = itr->getSource())
                    return plr;
            }
        }

        debug_log("TSCR: Instance Shattered Halls: GetPlayerInMap, but PlayerList is empty!");
        return NULL;
    }

    void OnObjectCreate(GameObject *go)
    {
        switch (go->GetEntry())
        {
            case DOOR_NETHEKURSE1:
                nethekurseDoor1GUID = go->GetGUID();
                if(GetData(TYPE_NETHEKURSE) == DONE)
                    HandleGameObject(nethekurseDoor1GUID, 0);
                break;
            case DOOR_NETHEKURSE2:
                nethekurseDoor2GUID = go->GetGUID();
                if(GetData(TYPE_NETHEKURSE) == DONE)
                    HandleGameObject(nethekurseDoor2GUID, 0);
                break;
        }
    }

    void OnCreatureCreate(Creature *creature, uint32 creature_entry)
    {
        switch (creature_entry)
        {
            case NPC_NETHEKURSE: nethekurseGUID = creature->GetGUID(); break;
            case NPC_WARBRINGER: warbringerGUID = creature->GetGUID(); break;
            case NPC_KARGATH: kargathGUID = creature->GetGUID(); break;
            case NPC_EXECUTIONER: executionerGUID = creature->GetGUID(); break;
            case NPC_SOLDIER_ALLIANCE_2: soldiera2GUID = creature->GetGUID(); break;
            case NPC_SOLDIER_ALLIANCE_3: soldiera3GUID = creature->GetGUID(); break;
            case NPC_OFFICER_ALLIANCE: officeraGUID = creature->GetGUID(); break;
            case NPC_SOLDIER_HORDE_2: soldierh2GUID = creature->GetGUID(); break;
            case NPC_SOLDIER_HORDE_3: soldierh3GUID = creature->GetGUID(); break;
            case NPC_OFFICER_HORDE: officerhGUID = creature->GetGUID(); break;
            case NPC_FEL_ORC: OrcGUID.push_back(creature->GetGUID()); break;
            case 17427:
            case 20923:
            {
                creature->SetReactState(REACT_PASSIVE);
                creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NOT_ATTACKABLE_2, true);
                break;
            }
        }
        switch(creature->GetDBTableGUIDLow())
        {
            case 62932: Zealot1GUID = creature->GetGUID(); break;
            case 62927: Zealot2GUID = creature->GetGUID(); break;
            case 62926: Zealot3GUID = creature->GetGUID(); break;
            case 62871: Hunter1GUID = creature->GetGUID(); break;
            case 62872: Hunter2GUID = creature->GetGUID(); break;

            case 62938: GladiatorGUID[0] = creature->GetGUID(); break;
            case 62939: GladiatorGUID[1] = creature->GetGUID(); break;
            case 62940: GladiatorGUID[2] = creature->GetGUID(); break;
            case 62941: GladiatorGUID[3] = creature->GetGUID(); break;

            case 62937: GladiatorGUID[4] = creature->GetGUID(); break;
            case 62934: GladiatorGUID[5] = creature->GetGUID(); break;
            case 62936: GladiatorGUID[6] = creature->GetGUID(); break;
            case 62935: GladiatorGUID[7] = creature->GetGUID(); break;

            case 62944: GladiatorGUID[8] = creature->GetGUID(); break;
            case 62943: GladiatorGUID[9] = creature->GetGUID(); break;
            case 62945: GladiatorGUID[10] = creature->GetGUID(); break;
            case 62942: GladiatorGUID[11] = creature->GetGUID(); break;

            case 62947: GladiatorGUID[12] = creature->GetGUID(); break;
            case 62948: GladiatorGUID[13] = creature->GetGUID(); break;
            case 62949: GladiatorGUID[14] = creature->GetGUID(); break;
            case 62946: GladiatorGUID[15] = creature->GetGUID(); break;
        }
    }

    GBK_Encounters EncounterForGBK(uint32 enc)
    {
        switch (enc)
        {
            case TYPE_NETHEKURSE:       return GBK_SHH_NETHERKURSE;
            case TYPE_WARBRINGER:       return GBK_SHH_WARBRINGER;
            case TYPE_PORUNG:           return GBK_SHH_PORUNG;
            case TYPE_KARGATH:          return GBK_SHH_KARGATH;
        }
        return GBK_NONE;
    }

    void SetData(uint32 type, uint32 data)
    {
        switch( type )
        {
            case TYPE_NETHEKURSE:
                if (data == FAIL)
                {
                    for (std::list<uint64>::iterator itr = OrcGUID.begin(); itr != OrcGUID.end(); ++itr)
                    {
                        if (Creature* Orc = instance->GetCreature(*itr))
                        {
                            if (!Orc->isAlive())
                            {
                                Orc->ForcedDespawn();
                                Orc->Respawn();
                            }
                        }
                    }
                }
                if (data == DONE)
                {
                    HandleGameObject(nethekurseDoor1GUID, 0);
                    HandleGameObject(nethekurseDoor2GUID, 0);
                }

                if (Encounter[0] != DONE)
                    Encounter[0] = data;
                break;
            case TYPE_WARBRINGER:
                if (Encounter[1] != DONE)
                    Encounter[1] = data;
                break;
            case TYPE_PORUNG:
                if (Encounter[2] != DONE)
                    Encounter[2] = data;
                break;
            case TYPE_KARGATH:
                if (data == DONE)
                {
                    if (Creature* Executioner = instance->GetCreature(executionerGUID))
                        Executioner->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, false);
                }
                else
                    Encounter[3] = data;
                break;
            case TYPE_EXECUTION:
                if (data == DONE && !instance->GetCreature(executionerGUID))
                {
                    if (Player* player = GetPlayerInMap())
                    {
                        for (uint8 i = 2; i < 5; ++i)
                            player->SummonCreature(Team == ALLIANCE ? aSoldiersLocs[i].AllianceEntry : aSoldiersLocs[i].HordeEntry, aSoldiersLocs[i].fX, aSoldiersLocs[i].fY, aSoldiersLocs[i].fZ, aSoldiersLocs[i].fO, TEMPSUMMON_DEAD_DESPAWN, 0);

						if (Creature* Executioner = player->SummonCreature(NPC_EXECUTIONER, afExecutionerLoc[0], afExecutionerLoc[1], afExecutionerLoc[2], afExecutionerLoc[3], TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90 * MINUTE*MILLISECONDS))
                            Executioner->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);

                        if (Player* playerTeam = instance->GetPlayers().begin()->getSource())
                            if (uint32 checkTeam = playerTeam->GetTeam())
                                DoScriptText(checkTeam == ALLIANCE ? SAY_KARGATH_EXECUTE_ALLY : SAY_KARGATH_EXECUTE_HORDE, instance->GetCreature(kargathGUID));

                        DoCastGroupDebuff(SPELL_KARGATH_EXECUTIONER_1);
                        ExecutionTimer = 55*MINUTE*MILLISECONDS;
                   }
               }
               else
                   Encounter[4] = data;
               break;
            case TYPE_EXECUTION_DONE:
               if (data == DONE)
               {
                   if (Creature* Officer = instance->GetCreature(Team == ALLIANCE ? officeraGUID : officerhGUID))
                       Officer->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
               }
               else
                   Encounter[5] = data;
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
        {
            SaveToDB();
            OUT_SAVE_INST_DATA_COMPLETE;
        }
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
        switch (type)
        {
            case TYPE_NETHEKURSE:
                return Encounter[0];
            case TYPE_WARBRINGER:
                return Encounter[1];
            case TYPE_PORUNG:
                return Encounter[2];
            case TYPE_KARGATH:
                return Encounter[3];
            case TYPE_EXECUTION:
                return Encounter[4];
            case TYPE_EXECUTION_DONE:
                return Encounter[5];
			case TYPE_EXECUTION_TIMER:
				return ExecutionTimer.GetTimeLeft();
        }
        return 0;
    }

    uint64 GetData64(uint32 data)
    {
        switch (data)
        {
            case DATA_NETHEKURSE:
                return nethekurseGUID;
            case DATA_WARBRINGER:
                return warbringerGUID;
            case DATA_ZEALOT_1:
                return Zealot1GUID;
            case DATA_ZEALOT_2:
                return Zealot2GUID;
            case DATA_ZEALOT_3:
                return Zealot3GUID;
            case DATA_HUNTER_1:
                return Hunter1GUID;
            case DATA_HUNTER_2:
                return Hunter2GUID;
            case DATA_GLAD_1_1:
                return GladiatorGUID[0];
            case DATA_GLAD_1_2:
                return GladiatorGUID[1];
            case DATA_GLAD_1_3:
                return GladiatorGUID[2];
            case DATA_GLAD_1_4:
                return GladiatorGUID[3];
            case DATA_GLAD_2_1:
                return GladiatorGUID[4];
            case DATA_GLAD_2_2:
                return GladiatorGUID[5];
            case DATA_GLAD_2_3:
                return GladiatorGUID[6];
            case DATA_GLAD_2_4:
                return GladiatorGUID[7];
            case DATA_GLAD_3_1:
                return GladiatorGUID[8];
            case DATA_GLAD_3_2:
                return GladiatorGUID[9];
            case DATA_GLAD_3_3:
                return GladiatorGUID[10];
            case DATA_GLAD_3_4:
                return GladiatorGUID[11];
            case DATA_GLAD_4_1:
                return GladiatorGUID[12];
            case DATA_GLAD_4_2:
                return GladiatorGUID[13];
            case DATA_GLAD_4_3:
                return GladiatorGUID[14];
            case DATA_GLAD_4_4:
                return GladiatorGUID[15];
        }
        return 0;
    }

    void OnCreatureDeath(Creature* pCreature)
    {
        if (pCreature->GetEntry() == NPC_EXECUTIONER)
            SetData(TYPE_EXECUTION_DONE, DONE);

        switch(pCreature->GetDBTableGUIDLow())
        {
            case 57222:
            case 57223:
            {
                if(HallOfFathersWaveOne >= 1)
                    pCreature->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, pCreature, 0, 150);
                else
                    HallOfFathersWaveOne++;
                break;
            }
            case 57694:
            case 15331:
            case 57688:
            case 57698:
            case 15610:
            {
                if(HallOfFathersWaveTwo >= 4)
                    pCreature->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_C, pCreature, 0, 150);
                else
                    HallOfFathersWaveTwo++;
                break;
            }
            case 187294: // Summon Fel Orc Convert on Death
            {
                if(Creature* FelOrc = pCreature->SummonCreature(17083, 70.291, 119.655, -13.221, 4.702, TEMPSUMMON_DEAD_DESPAWN, 0))
                    FelOrc->GetMotionMaster()->MovePoint(1, 69.913, 86.899, -13.221);
                break;
            }
            case 57695: // Summon Fel Orc Convert on Death
            {
                if(Creature* FelOrc1 = pCreature->SummonCreature(17083, 83.284, 223.792, -13.210, 3.29, TEMPSUMMON_DEAD_DESPAWN, 0))
                    FelOrc1->GetMotionMaster()->MovePoint(1, 69.99, 184.272, -13.237);
                break;
            }
            case 19314: // Summon Fel Orc Convert on Death
            {
                if(Creature* FelOrc2 = pCreature->SummonCreature(17083, 81.275, 251.521, -13.198, 3.31, TEMPSUMMON_DEAD_DESPAWN, 0))
                    FelOrc2->GetMotionMaster()->MovePoint(1, 70.007, 205.615, -13.192);
                break;
            }
            default: break;
        }
    }

    void DoCastGroupDebuff(uint32 SpellId)
    {
        Map::PlayerList const& lPlayers = instance->GetPlayers();

        if (lPlayers.isEmpty())
            return;

        for (Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
        {
            Player* player = itr->getSource();
            if (player && !player->HasAura(SpellId, 0))
                player->CastSpell(player, SpellId, true);
        }
    }

    void HandleGameObject(uint64 guid, uint32 state)
    {
        Player *player = GetPlayerInMap();

        if (!player || !guid)
            return;

        if (GameObject *go = GameObject::GetGameObject(*player,guid))
            go->SetGoState(GOState(state));
    }

	void Update(uint32 diff)
	{
		if (Encounter[5] == NOT_STARTED && ExecutionTimer.Expired(diff))
		{
			switch (ExecutionStage)
			{
			case 0:
				if (Creature* Soldier = instance->GetCreature(Team == ALLIANCE ? officeraGUID : officerhGUID))
					Soldier->DealDamage(Soldier, Soldier->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

				//DoScriptText(Team == ALLIANCE ? SAY_KARGATH_EXECUTE_ALLY : SAY_KARGATH_EXECUTE_HORDE, instance->GetCreature(kargathGUID));

				DoCastGroupDebuff(SPELL_KARGATH_EXECUTIONER_2);
				ExecutionTimer = 10 * MINUTE*MILLISECONDS;
				break;
			case 1:
				if (Creature* Soldier = instance->GetCreature(Team == ALLIANCE ? soldiera2GUID : soldierh2GUID))
					Soldier->DealDamage(Soldier, Soldier->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

				DoCastGroupDebuff(SPELL_KARGATH_EXECUTIONER_3);
				ExecutionTimer = 15 * MINUTE*MILLISECONDS;
				break;
			case 2:
				if (Creature* Soldier = instance->GetCreature(Team == ALLIANCE ? soldiera3GUID : soldierh3GUID))
					Soldier->DealDamage(Soldier, Soldier->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

				SetData(TYPE_EXECUTION_DONE, FAIL);
				ExecutionTimer = 0;
				break;
			}
			++ExecutionStage;
		}

		if (summon == WAIT_FOR_SUMMON)
		{
			if (instance->GetPlayers().isEmpty() || Team)
				return;

			Player* player = instance->GetPlayers().begin()->getSource();
			Team = player->GetTeam();

			if (Team == ALLIANCE)
				player->SummonCreature(aSoldiersLocs[1].AllianceEntry, aSoldiersLocs[1].fX, aSoldiersLocs[1].fY, aSoldiersLocs[1].fZ, aSoldiersLocs[1].fO, TEMPSUMMON_DEAD_DESPAWN, 0);
			else
				player->SummonCreature(aSoldiersLocs[0].HordeEntry, aSoldiersLocs[0].fX, aSoldiersLocs[0].fY, aSoldiersLocs[0].fZ, aSoldiersLocs[0].fO, TEMPSUMMON_DEAD_DESPAWN, 0);

			summon = SUMMONED;
		}
	}

    std::string GetSaveData()
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream stream;
        stream << Encounter[0] << " ";
        stream << Encounter[1] << " ";
        stream << Encounter[2] << " ";
        stream << Encounter[3] << " ";
        stream << Encounter[4] << " ";
        stream << Encounter[5] ;

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
        stream  >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3] >> Encounter[4] >> Encounter[5];

        for (uint8 i = 0; i < ENCOUNTERS; ++i)
            if (Encounter[i] == IN_PROGRESS)
                Encounter[i] = NOT_STARTED;

        if (Encounter[0] == DONE)
        {
            HandleGameObject(nethekurseDoor1GUID, 0);
            HandleGameObject(nethekurseDoor2GUID, 0);
        }

        OUT_LOAD_INST_DATA_COMPLETE;
    }
};

InstanceData* GetInstanceData_instance_shattered_halls(Map* map)
{
    return new instance_shattered_halls(map);
}

bool AreaTrigger_at_shattered_halls(Player* player, AreaTriggerEntry const* /*pAt*/)
{
    if (player->isGameMaster() || player->isDead())
        return false;

    instance_shattered_halls* pInstance = (instance_shattered_halls*)player->GetInstanceData();

    if (!pInstance)
        return false;

    if (!pInstance->instance->IsHeroic())
        return false;

    if (pInstance->GetData(TYPE_KARGATH) == DONE || pInstance->GetData(TYPE_WARBRINGER) == DONE)
        return false;

    if (pInstance->GetData(TYPE_EXECUTION) == NOT_STARTED)
        pInstance->SetData(TYPE_EXECUTION, DONE);

    return true;
}

struct npc_sh_gladiatorAI : public ScriptedAI
{
    npc_sh_gladiatorAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance* pInstance;

    Timer MortalStrikeTimer;

    void Reset() 
    {
        MortalStrikeTimer.Reset(3000);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == AI_EVENT_CUSTOM_EVENTAI_B)
        {
            switch(me->GetDBTableGUIDLow())
            {
                case 62938:
                    if (Creature* Glad2 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_1_2)))
                        me->AI()->AttackStart(Glad2);
                    break;
                case 62939:
                    if (Creature* Glad1 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_1_1)))
                        me->AI()->AttackStart(Glad1);
                    break;
                case 62940:
                    if (Creature* Glad4 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_1_4)))
                        me->AI()->AttackStart(Glad4);
                    break;
                case 62941:
                    if (Creature* Glad3 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_1_3)))
                        me->AI()->AttackStart(Glad3);
                    break;
                
                case 62937:
                    if (Creature* Glad6 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_2_2)))
                        me->AI()->AttackStart(Glad6);
                    break;
                case 62934:
                    if (Creature* Glad5 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_2_1)))
                        me->AI()->AttackStart(Glad5);
                    break;
                case 62936:
                    if (Creature* Glad8 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_2_4)))
                        me->AI()->AttackStart(Glad8);
                    break;
                case 62935:
                    if (Creature* Glad7 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_2_3)))
                        me->AI()->AttackStart(Glad7);
                    break;

                case 62944:
                    if (Creature* Glad10 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_3_2)))
                        me->AI()->AttackStart(Glad10);
                    break;
                case 62943:
                    if (Creature* Glad9 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_3_1)))
                        me->AI()->AttackStart(Glad9);
                    break;
                case 62945:
                    if (Creature* Glad12 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_3_4)))
                        me->AI()->AttackStart(Glad12);
                    break;
                case 62942:
                    if (Creature* Glad11 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_3_3)))
                        me->AI()->AttackStart(Glad11);
                    break;

                case 62947:
                    if (Creature* Glad14 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_4_2)))
                        me->AI()->AttackStart(Glad14);
                    break;
                case 62948:
                    if (Creature* Glad13 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_4_1)))
                        me->AI()->AttackStart(Glad13);
                    break;
                case 62949:
                    if (Creature* Glad16 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_4_4)))
                        me->AI()->AttackStart(Glad16);
                    break;
                case 62946:
                    if (Creature* Glad15 = pInstance->GetCreature(pInstance->GetData64(DATA_GLAD_4_3)))
                        me->AI()->AttackStart(Glad15);
                    break;
                default: break;
            }
        }
    }

    void DamageTaken(Unit* done_by, uint32& damage)
    {
        if (done_by->GetTypeId() == TYPEID_UNIT && !done_by->isCharmedOwnedByPlayerOrPlayer())
            damage = me->GetMaxHealth() / 20; // 5% per attack
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(MortalStrikeTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 16856, true);
            // GetVictim() can disappear after this -> it causes damage and may kill the target.
            MortalStrikeTimer = urand(10000, 15000);
        }

        if(me->GetHealthPercent() <= 35)
        {
            Unit* victim = me->GetVictim();
            if (victim && victim->GetTypeId() == TYPEID_UNIT && !victim->isCharmedOwnedByPlayerOrPlayer()) // GetVictim() exists, checked by UpdateVictim();
            {
                SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_C, me, 0, 10);

                if (victim->IsAIEnabled)
                    ((Creature*)victim)->AI()->EnterEvadeMode();
                victim->SetHealth(victim->GetMaxHealth());

                EnterEvadeMode();
                me->SetHealth(me->GetMaxHealth());
            }
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_sh_gladiator(Creature *_Creature)
{
    return new npc_sh_gladiatorAI (_Creature);
}

struct npc_sh_centurionAI : public ScriptedAI
{
    npc_sh_centurionAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance* pInstance;

    Timer BattleRoarTimer;
    Timer SunderArmorTimer;
    Timer YellTimer;
    bool Yelled;

    void Reset() 
    {
        Yelled = false;
        BattleRoarTimer.Reset(5000);
        SunderArmorTimer.Reset(1000);
        YellTimer.Reset(0);

        me->SetWalk(false);
        me->setActive(true);
        
        switch(me->GetDBTableGUIDLow())
        {
            case 62953:
                me->LoadPath(1418);
                break;
            case 62952:
                me->LoadPath(1417);
                break;
            case 62954:
                me->LoadPath(1419);
                break;
            case 62955:
                me->LoadPath(1420);
                break;
            default: break;
        }
        me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        me->GetMotionMaster()->Initialize();
    }

    void EnterCombat(Unit* who)
    {
        std::list<Creature*> gladiators = FindAllCreaturesWithEntry(17464, 12);
        if (gladiators.empty())
            return;
        for (std::list<Creature*>::iterator it = gladiators.begin(); it != gladiators.end(); it++)
            (*it)->AI()->AttackStart(who);
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type != WAYPOINT_MOTION_TYPE)
            return;

        bool doFight = false;
        switch(me->GetDBTableGUIDLow())
        {
            case 62953:
                if (Id == 7)
                    doFight = true;
                break;
            case 62952:
                if(Id == 6)
                    doFight = true;
                break;
            case 62954:
                if(Id == 9)
                    doFight = true;
                break;
            case 62955:
                if(Id == 8)
                    doFight = true;
                break;
            default: break;
        }

        if (doFight)
        {
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_B, me, 0, 15);
            me->Say(-1200497, LANG_UNIVERSAL, 0);
            me->StopMoving();
            me->GetMotionMaster()->Clear(true);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == AI_EVENT_CUSTOM_EVENTAI_C)
        {
            if(!Yelled)
            {
                me->Say(-1200498, LANG_UNIVERSAL, 0);
                Yelled = true;
                YellTimer = 5000;

                switch (me->GetDBTableGUIDLow())
                {
                    case 62953:
                        me->LoadPath(1418);
                        break;
                    case 62952:
                        me->LoadPath(1417);
                        break;
                    case 62954:
                        me->LoadPath(1419);
                        break;
                    case 62955:
                        me->LoadPath(1420);
                        break;
                    default: break;
                }
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(YellTimer.Expired(diff))
        {
            Yelled = false;
            YellTimer = 5000;
        }

        if(BattleRoarTimer.Expired(diff))
        {
            me->CastSpell(me, 30931, true);
            BattleRoarTimer = 110000;
        }

        if (!UpdateVictim())
            return;

        if(SunderArmorTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 15572, true);
            SunderArmorTimer = urand(10000, 15000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_sh_centurion(Creature *_Creature)
{
    return new npc_sh_centurionAI (_Creature);
}

void AddSC_instance_shattered_halls()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_shattered_halls";
    newscript->GetInstanceData = &GetInstanceData_instance_shattered_halls;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "at_shattered_halls";
    newscript->pAreaTrigger = &AreaTrigger_at_shattered_halls;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_sh_gladiator";
    newscript->GetAI = &GetAI_npc_sh_gladiator;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_sh_centurion";
    newscript->GetAI = &GetAI_npc_sh_centurion;
    newscript->RegisterSelf();
}

