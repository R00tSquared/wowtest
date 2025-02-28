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
SDName: Deadmines
SD%Complete: 0
SDComment: Placeholder
SDCategory: Deadmines
EndScriptData */

#include "precompiled.h"
#include "def_deadmines.h"
#include "Spell.h"

#define SOUND_CANNONFIRE        1400
#define SOUND_DESTROYDOOR       3079
#define SAY_MR_SMITE_ALARM1     -1036000
#define SOUND_MR_SMITE_ALARM1   5775
#define SAY_MR_SMITE_ALARM2     -1036001
#define SOUND_MR_SMITE_ALARM2   5777

#define NPC_RHAHK_ZOR           644
#define GO_FACTORY_DOOR         13965

#define NPC_SNEED               643
#define GO_HEAVY_DOOR_1         17153
#define GO_MAST_ROOM_DOOR       16400

#define NPC_GILNID              1763
#define GO_HEAVY_DOOR_2         17154
#define GO_FOUNDRY_DOOR         16399

#define GO_IRONCLAD_DOOR        16397
#define GO_DEFIAS_CANNON        16398
#define GO_DOOR_LEVER           101833
#define QUEST_FORTUNE           7938
#define GO_MYSTERIOUS_CHEST     180024

#define CANNON_BLAST_TIMER      3000
#define PIRATES_DELAY_TIMER     1000

struct instance_deadmines : public ScriptedInstance
{
    instance_deadmines(Map *Map) : ScriptedInstance(Map) {Initialize();};

    GameObject* IronCladDoor;
    GameObject* DefiasCannon;
    GameObject* DoorLever;
    uint64 FactoryDoorGUID;
    uint64 MastRoomDoorGUID;
    uint64 FoundryDoorGUID;
    uint64 summGUIDS[3];
    uint32 State;
    uint32 CannonBlast_Timer;
    uint32 PiratesDelay_Timer;
    bool MysteriousChestSpawned;

    void Initialize()
    {
        IronCladDoor            = NULL;
        DefiasCannon            = NULL;
        DoorLever               = NULL;
        FactoryDoorGUID         = 0;
        MastRoomDoorGUID        = 0;
        FoundryDoorGUID         = 0;
        State                   = CANNON_NOT_USED;
        MysteriousChestSpawned  = false;
        for (uint32 i = 0; i < 3; ++i)
            summGUIDS[i] = 0;
    }

    virtual void Update(uint32 diff)
    {
        if(!IronCladDoor || !DefiasCannon || !DoorLever)
            return;

        switch(State)
        {
            case CANNON_GUNPOWDER_USED:
                CannonBlast_Timer = CANNON_BLAST_TIMER;
                // it's a hack - Mr. Smite should do that but his too far away
                IronCladDoor->SetName("Mr. Smite");
                IronCladDoor->Yell(SAY_MR_SMITE_ALARM1, LANG_UNIVERSAL, 0);
                DoPlaySound(IronCladDoor, SOUND_MR_SMITE_ALARM1);
                State=CANNON_BLAST_INITIATED;
                break;
            case CANNON_BLAST_INITIATED:
                PiratesDelay_Timer = PIRATES_DELAY_TIMER;
                if(CannonBlast_Timer<diff)
                {
                    SummonCreatures();
                    ShootCannon();
                    BlastOutDoor();
                    LeverStucked();
                    IronCladDoor->Yell(SAY_MR_SMITE_ALARM2, LANG_UNIVERSAL, 0);
                    DoPlaySound(IronCladDoor, SOUND_MR_SMITE_ALARM2);
                    State = PIRATES_ATTACK;
                }else
                    CannonBlast_Timer-=diff;
                break;
            case PIRATES_ATTACK:
                if(PiratesDelay_Timer<diff)
                {
                    MoveCreaturesInside();
                    State = EVENT_DONE;
                }else
                    PiratesDelay_Timer-=diff;
                break;
        }
    }

    void OnCreatureDeath(Creature* pCreature)
    {
        switch(pCreature->GetEntry())
        {
            case NPC_RHAHK_ZOR:
            {
                if (GameObject* pFactoryDoor = instance->GetGameObject(FactoryDoorGUID))
                    pFactoryDoor->UseDoorOrButton();
                break;
            }
            case NPC_SNEED:
            {
                if (GameObject* pMastRoomDoor = instance->GetGameObject(MastRoomDoorGUID))
                    pMastRoomDoor->UseDoorOrButton();
                break;
            }
            case NPC_GILNID:
            {
                if (GameObject* pFoundryDoor = instance->GetGameObject(FoundryDoorGUID))
                    pFoundryDoor->UseDoorOrButton();
                break;
            }
            default: break;
        }
    }

    void SummonCreatures()
    {
        if (Creature* cre = IronCladDoor->SummonCreature(657, IronCladDoor->GetPositionX() - 2, IronCladDoor->GetPositionY() - 7, IronCladDoor->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
            summGUIDS[0] = cre->GetGUID();

        if (Creature* cre = IronCladDoor->SummonCreature(657,IronCladDoor->GetPositionX() + 3,IronCladDoor->GetPositionY()-6,IronCladDoor->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
            summGUIDS[1] = cre->GetGUID();

        if (Creature* cre = IronCladDoor->SummonCreature(3450,IronCladDoor->GetPositionX() + 2,IronCladDoor->GetPositionY()-6,IronCladDoor->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000))
            summGUIDS[2] = cre->GetGUID();
    }

    void MoveCreaturesInside()
    {
        Creature* cre[3] = { NULL, NULL, NULL };
        for (uint32 i = 0; i < 3; ++i)
        {
            if (!summGUIDS[i])
                return;

            cre[i] = GetCreature(summGUIDS[i]);
            if (!cre[i])
                return;
        }

        for (uint32 i = 0; i < 3; ++i)
            MoveCreatureInside(cre[i]);
    }

    void MoveCreatureInside(Creature *creature)
    {
        creature->SetWalk(false);
        creature->GetMotionMaster()->MovePoint(0, -102.7,-655.9, creature->GetPositionZ());
    }

    void ShootCannon()
    {
        DefiasCannon->SetUInt32Value(GAMEOBJECT_STATE, 0);
        DoPlaySound(DefiasCannon, SOUND_CANNONFIRE);
    }

    void BlastOutDoor()
    {
        IronCladDoor->SetUInt32Value(GAMEOBJECT_STATE, 2);
        DoPlaySound(IronCladDoor, SOUND_DESTROYDOOR);
    }

    void LeverStucked()
    {
        DoorLever->SetUInt32Value(GAMEOBJECT_FLAGS, 4);
    }

    void OnObjectCreate(GameObject *go)
    {
        switch(go->GetEntry())
        {
            case GO_IRONCLAD_DOOR:
                IronCladDoor = go;
                break;
            case GO_DEFIAS_CANNON:
                DefiasCannon = go;
                break;
            case GO_DOOR_LEVER:
                DoorLever = go;
                break;
            case GO_FACTORY_DOOR:
            {
                FactoryDoorGUID = go->GetGUID();
                break;
            }
            case GO_MAST_ROOM_DOOR:
            {
                MastRoomDoorGUID = go->GetGUID();
                break;
            }
            case GO_FOUNDRY_DOOR:
            {
                FoundryDoorGUID = go->GetGUID();
                break;
            }
        }
    }

    void SetData(uint32 type, uint32 data)
    {
        if (type == EVENT_STATE)
        {
            if (DefiasCannon && IronCladDoor)
                State=data;
        }
    }

    uint32 GetData(uint32 type)
    {
        if (type == EVENT_STATE)
            return State;
        return 0;
    }

    void DoPlaySound(GameObject* unit, uint32 sound)
    {
        WorldPacket data(Opcodes(4));
        data.SetOpcode(SMSG_PLAY_SOUND);
        data << uint32(sound);
        unit->BroadcastPacket(&data,false);
    }

    void DoPlaySoundCreature(Unit* unit, uint32 sound)
    {
        WorldPacket data(Opcodes(4));
        data.SetOpcode(SMSG_PLAY_SOUND);
        data << uint32(sound);
        unit->BroadcastPacket(&data,false);
    }

    void OnPlayerEnter(Player* plr)
    {
        if (!MysteriousChestSpawned && plr->GetQuestStatus(QUEST_FORTUNE) == QUEST_STATUS_INCOMPLETE)
        {
            plr->SummonGameObject(GO_MYSTERIOUS_CHEST,-30.4f,-374.8f,59.3f,6.1f,0.0f,0.0f,0.0f,0.0f,0);
            MysteriousChestSpawned = true;
        }
    }
};

/*#####
# item_Defias_Gunpowder
#####*/

bool ItemUse_item_defias_gunpowder(Player *player, Item* _Item, SpellCastTargets const& targets)
{
    ScriptedInstance *pInstance = (player->GetInstanceData()) ? (player->GetInstanceData()) : NULL;

    if(!pInstance)
    {
        player->GetSession()->SendNotification(-1200202);
        return true;
    }
    if (pInstance->GetData(EVENT_STATE)!= CANNON_NOT_USED)
        return false;
    if(targets.getGOTarget() && targets.getGOTarget()->GetTypeId() == TYPEID_GAMEOBJECT &&
       targets.getGOTarget()->GetEntry() == GO_DEFIAS_CANNON)
    {
        pInstance->SetData(EVENT_STATE, CANNON_GUNPOWDER_USED);
    }

    player->DestroyItemCount(_Item->GetEntry(), 1, true, false);
    return true;
}

InstanceData* GetInstanceData_instance_deadmines(Map* map)
{
    return new instance_deadmines(map);
}

enum MrSmite
{
    SAY_PHASE_2                     = -1036002,
    SAY_PHASE_3                     = -1036003,

    EQUIP_AXE                       = 13913,
    EQUIP_HAMMER                    = 19766,

    SPELL_NIBLE_REFLEXES            = 6433,                 // removed after phase 1
    SPELL_SMITE_SLAM                = 6435,
    SPELL_SMITE_STOMP               = 6432,
    SPELL_SMITE_HAMMER              = 6436,
    SPELL_THRASH                    = 12787,                // unclear, possible 3417 (only 10% proc chance)
    
    GO_SMITE_CHEST                  = 144111,

    PHASE_1                         = 1,
    PHASE_2                         = 2,
    PHASE_3                         = 3,
    PHASE_EQUIP_NULL                = 4,
    PHASE_EQUIP_START               = 5,
    PHASE_EQUIP_PROCESS             = 6,
    PHASE_EQUIP_END                 = 7,
};

struct boss_mr_smiteAI : public ScriptedAI
{
    boss_mr_smiteAI(Creature* pCreature) : ScriptedAI(pCreature)
    {}

    uint32 Phase;
    uint32 EquipTimer;
    uint32 SlamTimer;

    void Reset()
    {
        Phase = PHASE_1;
        EquipTimer = 0;
        SlamTimer = 9000;

        DoCast(me, SPELL_NIBLE_REFLEXES, true);

        SetEquipmentSlots(true);
    }

    void AttackedBy(Unit* pAttacker)
    {
        if (me->GetVictim())
            return;

        if (Phase > PHASE_3)
            return;

        AttackStart(pAttacker);
    }

    void AttackStart(Unit* pWho)
    {
        if (Phase > PHASE_3)
            return;

        if (me->Attack(pWho, true))
        {
            me->AddThreat(pWho, 0.0f);
            me->SetInCombatWith(pWho);
            pWho->SetInCombatWith(me);

            me->GetMotionMaster()->MoveChase(pWho);
        }
    }

    void MovementInform(uint32 MotionType, uint32 PointId)
    {
        if (MotionType != POINT_MOTION_TYPE)
            return;

        if(PointId == 1)
        {
            me->SetFacingTo(5.43);
            me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_UNARMED);
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            me->SetRooted(true);
            EquipTimer = 3000;
            Phase = PHASE_EQUIP_PROCESS;
        }
    }

    void PhaseEquipStart()
    {
        ScriptedInstance* pInstance = (ScriptedInstance*)me->GetInstanceData();

        if (!pInstance)
            return;

        Phase = PHASE_EQUIP_NULL;
        me->GetMotionMaster()->MovePoint(1, 1.521, -780.35, 9.816);
    }

    void PhaseEquipProcess()
    {
        if (me->GetHealthPercent() < 33.0f)
        {
            // It's Hammer, go Hammer!
            SetEquipmentSlots(false, EQUIP_HAMMER, EQUIP_UNEQUIP);
            DoCast(me, SPELL_SMITE_HAMMER);
        }
        else
            SetEquipmentSlots(false, EQUIP_AXE, EQUIP_AXE);

        me->SetRooted(false);
        me->SetStandState(UNIT_STAND_STATE_STAND);
        Phase = PHASE_EQUIP_END;
        EquipTimer = 5000;

        Unit* pVictim = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);

        if (!pVictim)
        {
            EnterEvadeMode();
            return;
        }

        // Takes longer to run further distance. Not accurate, but will probably be sufficient for most cases
        if (me->IsWithinDistInMap(pVictim, MELEE_RANGE))
            EquipTimer -= 1000;
        else if (me->IsWithinDistInMap(pVictim, 2*MELEE_RANGE))
            EquipTimer -= 2000;
        else
            EquipTimer -= 3000;
    }

    void PhaseEquipEnd()
    {
        // We don't have GetVictim, so select from threat list
        Unit* pVictim = SelectUnit(SELECT_TARGET_TOPAGGRO, 0);

        if (!pVictim)
        {
            EnterEvadeMode();
            return;
        }

        me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);

        Phase = me->GetHealthPercent() < 33.0f ? PHASE_3 : PHASE_2;

        if (Phase == PHASE_2)
            DoCast(me, SPELL_THRASH, true);

        AttackStart(pVictim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EquipTimer)
            {
                // decrease the cooldown in between equipment change phases
                if (EquipTimer >= diff)
                {
                    EquipTimer -= diff;
                    return;
                }
                else
                    EquipTimer = 0;
            }

            switch(Phase)
            {
                case PHASE_EQUIP_START:
                    PhaseEquipStart();
                    break;
                case PHASE_EQUIP_PROCESS:
                    PhaseEquipProcess();
                    break;
                case PHASE_EQUIP_END:
                    PhaseEquipEnd();
                    break;
            }

            return;
        }

        // the normal combat phases
        switch(Phase)
        {
            case PHASE_1:
            {
                if (me->GetHealthPercent() < 66.0f)
                {
                    DoCast(me, SPELL_SMITE_STOMP);
                    DoScriptText(SAY_PHASE_2, me);
                    Phase = PHASE_EQUIP_START;
                    EquipTimer = 2500;

                    // will clear GetVictim (m_attacking)
                    me->AttackStop();
                    me->RemoveAurasDueToSpell(SPELL_NIBLE_REFLEXES);
                }
                break;
            }
            case PHASE_2:
            {
                if (me->GetHealthPercent() < 33.0f)
                {
                    DoCast(me, SPELL_SMITE_STOMP);
                    DoScriptText(SAY_PHASE_3, me);
                    Phase = PHASE_EQUIP_START;
                    EquipTimer = 2500;

                    me->AttackStop();
                    me->RemoveAurasDueToSpell(SPELL_THRASH);
                }
                break;
            }
            case PHASE_3:
            {
                if (SlamTimer <= diff)
                {
                    DoCast(me->GetVictim(), SPELL_SMITE_SLAM);
                    SlamTimer = 11000;
                }
                else
                    SlamTimer -= diff;
                break;
            }
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_mr_smite(Creature* pCreature)
{
    return new boss_mr_smiteAI(pCreature);
}

void AddSC_instance_deadmines()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "instance_deadmines";
    newscript->GetInstanceData = &GetInstanceData_instance_deadmines;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "item_defias_gunpowder";
    newscript->pItemUse = &ItemUse_item_defias_gunpowder;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_mr_smite";
    newscript->GetAI = &GetAI_boss_mr_smite;
    newscript->RegisterSelf();
}
