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
SDName: boss_kelthuzad
SD%Complete: 90
SDComment: Debugging
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum KelThuzad
{
    // creatures
    NPC_THE_LICHKING            = 16980,
    NPC_SOLDIER_FROZEN          = 16427,
    NPC_UNSTOPPABLE_ABOM        = 16428,
    NPC_SOUL_WEAVER             = 16429,
    NPC_GUARDIAN                = 16441,

    // GO
    GO_KELTHUZAD_WINDOW_1       = 181402,
    GO_KELTHUZAD_WINDOW_2       = 181403,
    GO_KELTHUZAD_WINDOW_3       = 181404,
    GO_KELTHUZAD_WINDOW_4       = 181405,
    GO_KELTHUZAD_DOOR           = 181228,

    //when shappiron dies. dialog between kel and lich king (in this order)
    /*SAY_SAPP_DIALOG1            = -1533084,
    SAY_SAPP_DIALOG2_LICH       = -1533085,
    SAY_SAPP_DIALOG3            = -1533086,
    SAY_SAPP_DIALOG4_LICH       = -1533087,
    SAY_SAPP_DIALOG5            = -1533088,
    SAY_CAT_DIED                = -1533089, //when cat dies*/

    SAY_SUMMON_MINIONS          = -1533105,                // start of phase 1
    EMOTE_PHASE2                = -1533135,                // start of phase 2
    SAY_AGGRO1                  = -1533094,                // start of phase 2
    SAY_AGGRO2                  = -1533095,
    SAY_AGGRO3                  = -1533096,
    SAY_SLAY1                   = -1533097,
    SAY_SLAY2                   = -1533098,
    SAY_DEATH                   = -1533099,
    SAY_CHAIN1                  = -1533100,
    SAY_CHAIN2                  = -1533101,
    SAY_FROST_BLAST             = -1533102,
    SAY_REQUEST_AID             = -1533103,                //start of phase 3
    SAY_ANSWER_REQUEST          = -1533104,                //lich king answer
    SAY_SPECIAL1_MANA_DET       = -1533106,
    SAY_SPECIAL3_MANA_DET       = -1533107,
    SAY_SPECIAL2_DISPELL        = -1533108,

    //common needed defines
    NAXXRAMAS_MAP               = 533,

    //spells to be casted
    SPELL_KELTHUZAD_CHANNEL     = 29423,
    SPELL_FROST_BOLT            = 28478,
    H_SPELL_FROST_BOLT          = 55802,
    SPELL_FROST_BOLT_NOVA       = 28479,
    H_SPELL_FROST_BOLT_NOVA     = 55807,
    SPELL_CHAINS_OF_KELTHUZAD   = 28410,
    SPELL_MANA_DETONATION       = 27819,
    SPELL_SHADOW_FISURE         = 27810,
    SPELL_FROST_BLAST           = 27808,
};

struct Locations
{
    float x, y, z, o;
};

static Locations SpawnPosition_Room1_UA[]=
{
    {3759.45, -5150, 143.7, 4},
    {3755.92, -5168.88, 143.57, 2},
    {3739.54, -5160.93, 143.4, 0.29}
};

static Locations SpawnPosition_Room1_SF[]=
{
    {3751.69, -5150.73, 143.17, 4.72},
    {3745.92, -5156.49, 143.17, 0.54},
    {3750.31, -5165.72, 143.47, 1.86},
    {3761.7, -5162.139, 143.68, 2.78},
    {3757.19, -5167.12, 143.54, 2.22}
};

static Locations SpawnPosition_Room2_SF[]=
{
    {3713.43, -5178.08, 143.87, 2.89},
    {3693.53, -5177.54, 143.87, 1.02},
    {3691.47, -5170.70, 143.83, 0.01},
    {3692.10, -5164.16, 143.83, 5.90},
    {3717.16, -5171.30, 143.90, 3.01},
    {3717.41, -5165.94, 143.77, 3.17},
    {3700.74, -5170.02, 143.45, 0.93},
    {3709.82, -5163.81, 143.46, 1.80}
};

static Locations SpawnPosition_Room3_UA[]=
{
    {3673.02, -5150.77, 143.77, 2.07},
    {3656.376465, -5147.067871, 143.512665, 0.6},
    {3657.510254, -5131.799805, 143.753433, 5.3}
};

static Locations SpawnPosition_Room3_SF[]=
{
    {3657.799561, -5139.067383 , 143.479279 , 6.158751},
    {3661.150146, -5146.908203 , 143.442444 , 1.179326},
    {3675.916992, -5145.345215 , 143.239166 , 2.742269},
    {3665.958740, -5132.059570 , 143.170654 , 6.025228},
    {3661.792236, -5139.501465 , 143.325195 , 0.415125}
};

static Locations SpawnPosition_Room4_UA[]=
{
    {3646.409424, -5100.722656 , 143.655716 , 0.576913},
    {3647.306885, -5084.346680 , 143.629501 , 5.312863},
    {3659.596436, -5089.905762 , 143.496811 , 4.47327}
};

static Locations SpawnPosition_Room5_UA[]=
{
    {3673.525635, -5065.437012 , 143.625046 , 0.949983},
    {3682.042969, -5043.799805 , 143.648956 , 4.725804},
    {3693.411133, -5050.823730 , 143.645340 , 4.496471}
};

static Locations SpawnPosition_Room6_UA[]=
{
    {3730.479736, -5031.209473 , 143.818604 , 4.857750},
    {3718.561035, -5035.487305 , 143.882599 , 5.316417},
    {3737.832764, -5054.908203 , 143.545197 , 2.325618}
};

static Locations SpawnPosition_Room6_SF[]=
{
    {3727.100830, -5040.920410 , 143.499176 , 4.9944},
    {3726.416504, -5037.010742 , 143.648956 , 5.491555},
    {3727.749268, -5048.810547 , 143.284698 , 4.607988}
};

static Locations SpawnPosition_Room7_UA[]=
{
    {3775.286621, -5084.580078 , 143.869400 , 2.114017},
    {3767.375244, -5088.425781 , 143.272125 , 1.808498},
    {3763.045654, -5060.428223 , 143.824844 , 5.102453}
};

static Locations SpawnPosition_Room7_SF[]=
{
    {3773.645264, -5065.687500 , 143.494675 , 4.375179},
    {3774.354736, -5075.035156 , 143.475952 , 3.149173},
    {3760.157959, -5072.154297 , 143.172699 , 5.005066}
};

const Locations Pos[] =
{
    {3783.272705f, -5062.697266f, 143.711203f, 3.617599f},     //LEFT_FAR
    {3730.291260f, -5027.239258f, 143.956909f, 4.461900f},     //LEFT_MIDDLE
    {3757.6f, -5172.0f, 143.7f, 1.97f},                        //WINDOW_PORTAL05
    {3759.355225f, -5174.128418f, 143.802383f, 2.170104f},     //RIGHT_FAR
    {3700.724365f, -5185.123047f, 143.928024f, 1.309310f},     //RIGHT_MIDDLE
    {3700.86f, -5181.29f, 143.928024f, 1.42f},                 //WINDOW_PORTAL04
    {3754.431396f, -5080.727734f, 142.036316f, 3.736189f},     //LEFT_FAR
    {3724.396484f, -5061.330566f, 142.032700f, 4.564785f},     //LEFT_MIDDLE
    {3732.02f, -5028.53f, 143.92f, 4.49f},                     //WINDOW_PORTAL02
    {3687.571777f, -5126.831055f, 142.017807f, 0.604023f},     //RIGHT_FAR
    {3707.990733f, -5151.450195f, 142.032562f, 1.376855f},     //RIGHT_MIDDLE
    {3782.76f, -5062.97f, 143.79f, 3.82f},                     //WINDOW_PORTAL03
};

struct boss_kelthuzadAI : public ScriptedAI
{
    boss_kelthuzadAI(Creature* c) : ScriptedAI(c), summons(c)
    {
        GuardiansOfIcecrown[0] = 0;
        GuardiansOfIcecrown[1] = 0;
        GuardiansOfIcecrown[2] = 0;
        GuardiansOfIcecrown[3] = 0;
        GuardiansOfIcecrown[4] = 0;
        GuardiansOfIcecrown_Count = 0;
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    SummonList summons;
    uint64 GuardiansOfIcecrown[5];
    uint32 GuardiansOfIcecrown_Count;
    uint32 GuardiansOfIcecrown_Timer;
    uint32 LichKingResponseTimer;
    uint32 CheckShackle;
    uint32 FrostBolt_Timer;
    uint32 FrostBoltNova_Timer;
    uint32 ChainsOfKelthuzad_Timer;
    uint32 ManaDetonation_Timer;
    uint32 ShadowFisure_Timer;
    uint32 FrostBlast_Timer;
    uint32 ChainsOfKelthuzad_Targets;
    uint32 Phase1_Timer;
    uint32 Phase1SummonTimer;
    uint32 SoldierFrozenTimer;
    uint32 UnstoppableAbdomenTimer;
    uint32 SoulWeawerTimer;
    uint8 nSoldierFrozen;
    uint8 nAbomination;
    uint8 nWeaver;
    uint8 nShackeled;

    uint32 Phase1PackCount;

    bool Phase1; // Intro.
    bool Phase2;
    bool Phase3;
    bool LKShouldSay;

    void Reset()
    {
        Phase1_Timer = 310000;                              //Phase 1 lasts 5 minutes and 10 seconds
        FrostBolt_Timer = (rand()%60)*1000;                 //It won't be more than a minute without cast it
        FrostBoltNova_Timer = 15000;                        //Cast every 15 seconds
        ChainsOfKelthuzad_Timer = (rand()%30+30)*1000;      //Cast no sooner than once every 30 seconds
        ManaDetonation_Timer = 20000;                       //Seems to cast about every 20 seconds
        ShadowFisure_Timer = 25000;                         //25 seconds
        FrostBlast_Timer = (rand()%30+30)*1000;             //Random time between 30-60 seconds
        GuardiansOfIcecrown_Timer = 5000;                   //5 seconds for summoning each Guardian of Icecrown in phase 3
        LichKingResponseTimer = 4000;
        CheckShackle = 5000;
        Phase1PackCount = 0;
        Phase1SummonTimer = 0;
        SoldierFrozenTimer = 0;
        UnstoppableAbdomenTimer = 0;
        SoulWeawerTimer = 0;
        nSoldierFrozen = 0;
        nAbomination = 0;
        nWeaver = 0;
        nShackeled = 0;

        Phase1 = false;
        Phase2 = false;
        Phase3 = false;
        LKShouldSay = true;

        DespawnEverything();

        if (pInstance)
            pInstance->SetData(DATA_KEL_THUZAD, NOT_STARTED);
    }

    void JustSummoned(Creature* summoned)
    {
        summons.Summon(summoned);
    }

    void KilledUnit()
    {
        if (rand()%2)
            DoScriptText(SAY_SLAY1, me);
        else
            DoScriptText(SAY_SLAY2, me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
        for(int i=0; i<5; i++)
        {
            if(GuardiansOfIcecrown[i])
            {
                Unit* pUnit = Unit::GetUnit((*me), GuardiansOfIcecrown[i]);
                if (!pUnit || !pUnit->isAlive())
                    continue;

                pUnit->CombatStop();
                uint32 ran = RAND(2, 5, 8, 11);
                pUnit->GetMotionMaster()->MovePoint(1, Pos[ran].x, Pos[ran].y, Pos[ran].z, Pos[ran].o);
            }
        }

        if (pInstance)
            pInstance->SetData(DATA_KEL_THUZAD, DONE);
        DespawnEverything();
    }

    void EnterEvadeMode()
    {
        if (!me->isAlive())
            return;

        bool alive = false;
        Map::PlayerList const &plList = me->GetMap()->GetPlayers();
        if (plList.isEmpty())
            return;
        for (Map::PlayerList::const_iterator i = plList.begin(); i != plList.end(); ++i)
        {
            if (Player* plr = i->getSource())
            {
                if (plr->isAlive() && plr->isCharmed())
                {
                    alive = true;
                    break;
                }
            }
        }
        if (!alive)
        {
            pInstance->SetData(DATA_KEL_THUZAD, FAIL);
            ScriptedAI::EnterEvadeMode();
            return;
        }
    }

    void EnterCombat(Unit* who)
    {
        if (pInstance->GetData(DATA_SAPPHIRON) != DONE)
        {
            EnterEvadeMode();
            return;
        }

        for (const auto& boss : boss_groups) 
        {
            if ((me->GetMap()->IsHeroicRaid() || pInstance->GetCustomData() == boss.first) && pInstance->GetData(boss.second) != DONE) 
            {
                EnterEvadeMode();
                return;
            }
        }
        
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetRooted(true);
        DoCast(me, SPELL_KELTHUZAD_CHANNEL);
        Phase1 = true;
        pInstance->SetData(DATA_KEL_THUZAD, IN_PROGRESS);
    }
    
    void DespawnEverything()
    {
        // Despawn everything
        for(int i=0; i<5; i++)
        {
            if(GuardiansOfIcecrown[i])
            {
                //delete creature
                Unit* pUnit = Unit::GetUnit((*me), GuardiansOfIcecrown[i]);
                if (pUnit && pUnit->isAlive())
                    pUnit->DealDamage(pUnit, pUnit->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                GuardiansOfIcecrown[i] = 0;
            }
        }
        summons.DespawnAll();
        if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_1, 100, me))
            door->RemoveFromWorld();
        if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_2, 100, me))
            door->RemoveFromWorld();
        if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_3, 100, me))
            door->RemoveFromWorld();
        if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_4, 100, me))
            door->RemoveFromWorld();
        if (GameObject* door = FindGameObject(GO_KELTHUZAD_DOOR, 100, me))
            door->RemoveFromWorld();
    }

    void Phase1SummonCreatures(uint32 Phase1SummonCreatures)
    {
        if (Phase1SummonCreatures == 1)
        {
            me->SummonGameObject(GO_KELTHUZAD_WINDOW_1, 3760.408691, -5173.944824 , 143.818130 , 2.329471, 0, 0, 0, 0, 0);
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_UNSTOPPABLE_ABOM, SpawnPosition_Room1_UA[i].x, SpawnPosition_Room1_UA[i].y, SpawnPosition_Room1_UA[i].z, SpawnPosition_Room1_UA[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
            for(int j = 0; j<5; j++)
            {
                Creature* summon = me->SummonCreature(NPC_SOLDIER_FROZEN, SpawnPosition_Room1_SF[j].x, SpawnPosition_Room1_SF[j].y, SpawnPosition_Room1_SF[j].z, SpawnPosition_Room1_SF[j].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }
        if (Phase1SummonCreatures == 2)
        {
            me->SummonGameObject(GO_KELTHUZAD_WINDOW_2, 3699.928467, -5185.792480 , 143.956100 , 1.278604, 0, 0, 0, 0, 0);
            Creature* summon = me->SummonCreature(NPC_SOUL_WEAVER, 3701.44, -5179.46, 143.72, 1.3, TEMPSUMMON_DEAD_DESPAWN, 0);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            summon->SetReactState(REACT_PASSIVE);
            for(int j = 0; j<8; j++)
            {
                Creature* summon = me->SummonCreature(NPC_SOLDIER_FROZEN, SpawnPosition_Room2_SF[j].x, SpawnPosition_Room2_SF[j].y, SpawnPosition_Room2_SF[j].z, SpawnPosition_Room2_SF[j].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }
        if (Phase1SummonCreatures == 3)
        {
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_UNSTOPPABLE_ABOM, SpawnPosition_Room3_UA[i].x, SpawnPosition_Room3_UA[i].y, SpawnPosition_Room3_UA[i].z, SpawnPosition_Room3_UA[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
            for(int j = 0; j<5; j++)
            {
                Creature* summon = me->SummonCreature(NPC_SOLDIER_FROZEN, SpawnPosition_Room3_SF[j].x, SpawnPosition_Room3_SF[j].y, SpawnPosition_Room3_SF[j].z, SpawnPosition_Room3_SF[j].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }
        if (Phase1SummonCreatures == 4)
        {
            me->SummonGameObject(GO_KELTHUZAD_DOOR, 3636.790039, -5090.569824, 143.205994, 4.46595, 0, 0, 0, 0, 0);
            Creature* summon = me->SummonCreature(NPC_SOUL_WEAVER, 3645.214844, -5092.449707 , 143.277756 , 6.055067, TEMPSUMMON_DEAD_DESPAWN, 0);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            summon->SetReactState(REACT_PASSIVE);
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_UNSTOPPABLE_ABOM, SpawnPosition_Room4_UA[i].x, SpawnPosition_Room4_UA[i].y, SpawnPosition_Room4_UA[i].z, SpawnPosition_Room4_UA[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }
        if (Phase1SummonCreatures == 5)
        {
            Creature* summon = me->SummonCreature(NPC_SOUL_WEAVER, 3670.980469, -5050.056641, 143.683578, 5.797875, TEMPSUMMON_DEAD_DESPAWN, 0);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            summon->SetReactState(REACT_PASSIVE);
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_UNSTOPPABLE_ABOM, SpawnPosition_Room5_UA[i].x, SpawnPosition_Room5_UA[i].y, SpawnPosition_Room5_UA[i].z, SpawnPosition_Room5_UA[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }
        if (Phase1SummonCreatures == 6)
        {
            me->SummonGameObject(GO_KELTHUZAD_WINDOW_3, 3732.718506, -5027.854004 , 143.952759 , 4.523870, 0, 0, 0, 0, 0);
            Creature* summon = me->SummonCreature(NPC_SOUL_WEAVER, 3740.917725, -5039.327148 , 143.865143 , 3.815528, TEMPSUMMON_DEAD_DESPAWN, 0);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            summon->SetReactState(REACT_PASSIVE);
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_UNSTOPPABLE_ABOM, SpawnPosition_Room6_UA[i].x, SpawnPosition_Room6_UA[i].y, SpawnPosition_Room6_UA[i].z, SpawnPosition_Room6_UA[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_SOLDIER_FROZEN, SpawnPosition_Room6_SF[i].x, SpawnPosition_Room6_SF[i].y, SpawnPosition_Room6_SF[i].z, SpawnPosition_Room6_SF[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
        }
        if (Phase1SummonCreatures == 7)
        {
            me->SummonGameObject(GO_KELTHUZAD_WINDOW_4, 3783.370117, -5062.418945 , 143.820145 , 3.864922, 0, 0, 0, 0, 0);
            Creature* summon = me->SummonCreature(NPC_SOUL_WEAVER, 3781.190674, -5074.685547 , 143.763641 , 2.683765, TEMPSUMMON_DEAD_DESPAWN, 0);
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            summon->SetReactState(REACT_PASSIVE);
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_UNSTOPPABLE_ABOM, SpawnPosition_Room7_UA[i].x, SpawnPosition_Room7_UA[i].y, SpawnPosition_Room7_UA[i].z, SpawnPosition_Room7_UA[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
            for(int i = 0; i<3; i++)
            {
                Creature* summon = me->SummonCreature(NPC_SOLDIER_FROZEN, SpawnPosition_Room7_SF[i].x, SpawnPosition_Room7_SF[i].y, SpawnPosition_Room7_SF[i].z, SpawnPosition_Room7_SF[i].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                summon->SetReactState(REACT_PASSIVE);
            }
            SoldierFrozenTimer = urand(2000, 5000);
            UnstoppableAbdomenTimer = 25000;
            SoulWeawerTimer = 30000;
        }
    }

    void UpdateAI(const uint32 diff)
    {        
        if (!UpdateVictim())
            return;

        if(Phase1)
        {
            if(Phase1PackCount <= 7)
            {
                if(Phase1SummonTimer < diff)
                {
                    if(Phase1PackCount == 0)
                        DoScriptText(SAY_SUMMON_MINIONS, me);
                    Phase1SummonCreatures(Phase1PackCount);
                    ++Phase1PackCount;
                    Phase1SummonTimer = 2000;
                }
                else Phase1SummonTimer -= diff;
            }
            else
            {
                if(SoldierFrozenTimer < diff)
                {
                    if(nSoldierFrozen < 20)
                    {
                        uint32 ran = RAND(0, 3, 6, 9);
                        Creature* summon = me->SummonCreature(NPC_SOLDIER_FROZEN, Pos[ran].x, Pos[ran].y, Pos[ran].z, Pos[ran].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                        summon->GetMotionMaster()->MovePoint(0, 3716.023926, -5106.272949 , 141.28894);
                        nSoldierFrozen++;
                        SoldierFrozenTimer = 5000;
                    }
                    else SoldierFrozenTimer = 10000;
                }
                else
                    SoldierFrozenTimer -= diff;

                if(UnstoppableAbdomenTimer < diff)
                {
                    if (nAbomination < 8)
                    {
                        uint32 ran = RAND(1, 4, 7, 10);
                        Creature* summon = me->SummonCreature(NPC_UNSTOPPABLE_ABOM, Pos[ran].x, Pos[ran].y, Pos[ran].z, Pos[ran].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                        summon->GetMotionMaster()->MovePoint(0, 3716.023926, -5106.272949 , 141.28894);
                        nAbomination++;
                        UnstoppableAbdomenTimer = 25000;
                    }
                    else UnstoppableAbdomenTimer = 35000;
                }
                else UnstoppableAbdomenTimer -= diff;
                if(SoulWeawerTimer < diff)
                {
                    if (nWeaver < 8)
                    {
                        uint32 ran = RAND(0, 3, 6, 9);
                        Creature* summon =  me->SummonCreature(NPC_SOUL_WEAVER, Pos[ran].x, Pos[ran].y, Pos[ran].z, Pos[ran].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                        summon->GetMotionMaster()->MovePoint(0, 3716.023926, -5106.272949 , 141.28894);
                        nWeaver++;
                        SoulWeawerTimer = 30000;
                    }
                    else SoulWeawerTimer = 40000;
                }
                else SoulWeawerTimer -= diff;
            }
        }

        if((Phase1_Timer < diff) || Phase2)
        {
            if(Phase1)
            {
                Phase1 = false;
                Phase2 = true;
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                me->SetRooted(false);
                me->GetMotionMaster()->MoveChase(me->GetVictim());
                DoScriptText(EMOTE_PHASE2, me);
                DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);
                me->InterruptNonMeleeSpells(true);
            }
            if(Phase2)
            {
                if(me->GetVictim() && me->isAlive())
                {
                    if(FrostBolt_Timer < diff) // Check for Frost Bolt
                    {
                        DoCast(me->GetVictim(), SPELL_FROST_BOLT);
                        FrostBolt_Timer = urand(1000, 60000);
                    }
                    else
                        FrostBolt_Timer -= diff;
        
                    if(FrostBoltNova_Timer < diff) // Check for Frost Bolt Nova
                    {
                        DoCast(me->GetVictim(), SPELL_FROST_BOLT_NOVA);
                        FrostBoltNova_Timer = 15000;
                    }
                    else
                        FrostBoltNova_Timer -= diff;
        
                    if(ManaDetonation_Timer < diff) // Check for Mana Detonation
                    {
                        if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, SPELL_MANA_DETONATION, true, POWER_MANA))
                            DoCast(pTarget, SPELL_MANA_DETONATION);
                        if (urand(0, 1))
                            DoScriptText(SAY_SPECIAL1_MANA_DET, me);
                        ManaDetonation_Timer = 20000;
                    }
                    else
                        ManaDetonation_Timer -= diff;
        
                    if(ShadowFisure_Timer < diff) // Check for Shadow Fissure
                    {
                        if(Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, SPELL_SHADOW_FISURE, true))
                            DoCast(pTarget, SPELL_SHADOW_FISURE);
                        if (urand(0, 1))
                            DoScriptText(SAY_SPECIAL3_MANA_DET, me);
                        ShadowFisure_Timer = 25000;
                    }
                    else
                        ShadowFisure_Timer -= diff;
        
                    if(FrostBlast_Timer < diff) // Check for Frost Blast
                    {
                        //time to cast
                        DoCast(me->GetVictim(), SPELL_FROST_BLAST);
                        if (urand(0, 1))
                            DoScriptText(SAY_FROST_BLAST, me);
                        FrostBlast_Timer = urand(30000, 60000);
                    }
                    else
                        FrostBlast_Timer -= diff;
        
                    if(ChainsOfKelthuzad_Timer < diff) // Check for Chains Of Kelthuzad
                    {
                        Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, SPELL_CHAINS_OF_KELTHUZAD, true);
                        DoCast(target,SPELL_CHAINS_OF_KELTHUZAD);
                        DoScriptText(urand(0, 1) ? SAY_CHAIN1 : SAY_CHAIN2, me);
                        ChainsOfKelthuzad_Timer = urand(30000, 60000);
                    }
                    else
                        ChainsOfKelthuzad_Timer -= diff;
        
                    //start phase 3 when we are 40% health
                    if(!Phase3 && (me->GetHealthPercent()) < 40)
                    {
                        Phase3 = true;
                        DoScriptText(SAY_REQUEST_AID, me);
                        me->SummonCreature(NPC_THE_LICHKING, 3716.023926, -5106.272949 , 141.28894, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                    }
        
                    if(Phase3)
                    {
                        if(GuardiansOfIcecrown_Count < 5)
                        {
                            if(LKShouldSay)
                            {
                                if(LichKingResponseTimer < diff)
                                {
                                    if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_1, 100, me))
                                        door->UseDoorOrButton();
                                    if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_2, 100, me))
                                        door->UseDoorOrButton();
                                    if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_3, 100, me))
                                        door->UseDoorOrButton();
                                    if (GameObject* door = FindGameObject(GO_KELTHUZAD_WINDOW_4, 100, me))
                                        door->UseDoorOrButton();
                                    if (Unit* LichKing = FindCreature(NPC_THE_LICHKING, 100, me))
                                        DoScriptText(SAY_ANSWER_REQUEST, LichKing, me);
                                    LichKingResponseTimer = 0;
                                    LKShouldSay = false;
                                } else LichKingResponseTimer -= diff;
                            }
                            if(GuardiansOfIcecrown_Timer < diff)
                            {
                                //Summon a Guardian of Icecrown in a random alcove (Creature # 16441)
                                uint32 ran = RAND(2, 5, 8, 11);
                                Creature* summon = me->SummonCreature(NPC_GUARDIAN, Pos[ran].x, Pos[ran].y, Pos[ran].z, Pos[ran].o, TEMPSUMMON_DEAD_DESPAWN, 0);
                
                                if (summon)
                                {
                                    //if we find no one to fight - walk to the center
                                    if(!summon->IsInCombat())
                                        summon->GetMotionMaster()->MovePoint(0, 3716.023926, -5106.272949 , 141.28894);
    
                                    //Safe storing of creatures
                                    GuardiansOfIcecrown[GuardiansOfIcecrown_Count] = summon->GetGUID();
                                    //Update guardian count
                                    GuardiansOfIcecrown_Count++;
                
                                }
                                //5 seconds until summoning next guardian
                                GuardiansOfIcecrown_Timer = 5000;
                            } else GuardiansOfIcecrown_Timer -= diff;
                        }
                        else
                        {
                            if(CheckShackle < diff)
                            {
                                for(int i=0; i<5; i++)
                                {
                                    if(GuardiansOfIcecrown[i])
                                    {
                                        Unit* pUnit = Unit::GetUnit((*me), GuardiansOfIcecrown[i]);
                                        if(pUnit->HasAura(10955))
                                            nShackeled++;
                                    }
                                }
                                if (nShackeled > 3)
                                {
                                    for(int i=0; i<5; i++)
                                    {
                                        if(GuardiansOfIcecrown[i])
                                        {
                                            Unit* pUnit = Unit::GetUnit((*me), GuardiansOfIcecrown[i]);
                                            pUnit->RemoveAurasDueToSpell(10955);
                                            nShackeled--;
                                        }
                                    }
                                }
                                CheckShackle = 5000;
                            } else CheckShackle -= diff;
                        }
                    }
        
                    DoMeleeAttackIfReady();
                }
            }
            Phase1_Timer = 0;
        } else Phase1_Timer -= diff;
    }
};

CreatureAI* GetAI_boss_kelthuzadAI(Creature *_Creature)
{
    return new boss_kelthuzadAI (_Creature);
}

void AddSC_boss_kelthuzad()
{

    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_kelthuzad";
    newscript->GetAI = &GetAI_boss_kelthuzadAI;
    newscript->RegisterSelf();
}

