// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2014 Hellground <http://hellground.net/>
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
SDName: Boss_Ayamiss
SD%Complete: 50
SDComment: VERIFY SCRIPT, Wasp include missing, larvas and sacrifice too, not flying in phase one
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_ruins_of_ahnqiraj.h"

enum
{
    EMOTE_GENERIC_FRENZY    = -1000002,

    SPELL_STINGER_SPRAY     = 25749,
    SPELL_POISON_STINGER    = 25748,                // only used in phase1
    // SPELL_SUMMON_SWARMER  = 25844,                // might be 25708    - spells were removed since 2.0.1
    SPELL_PARALYZE          = 25725,
    SPELL_LASH              = 25852,
    SPELL_FRENZY            = 8269,
    SPELL_TRASH             = 3391,

    SPELL_FEED              = 25721,                // cast by the Larva when reaches the player on the altar

    NPC_LARVA               = 15555,
    NPC_SWARMER             = 15546,
    NPC_HORNET              = 15934,

    PHASE_AIR               = 0,
    PHASE_GROUND            = 1
};

struct SummonLocation
{
    float m_fX, m_fY, m_fZ;
};

// Spawn locations
static const SummonLocation aAyamissSpawnLocs[] =
{
    { -9674.4707f, 1528.4133f, 22.457f},        // larva
    { -9701.6005f, 1566.9993f, 24.118f},        // larva
    { -9647.352f, 1578.062f, 55.32f},           // anchor point for swarmers
    { -9703.216f, 1531.17f, 21.444132f},        // 3
    { -9704.497f, 1529.96f, 21.856441f},
    { -9705.691f, 1528.82f, 22.861944f},
    { -9707.026f, 1527.63f, 23.959238f},
    { -9708.251f, 1526.52f, 24.966782f},
    { -9709.476f, 1525.43f, 25.976612f},
    { -9711.222f, 1523.87f, 27.416300f},
    { -9711.932f, 1523.25f, 27.468222f},
    { -9712.891f, 1522.31f, 27.468222f},
    { -9713.561f, 1521.66f, 27.468222f},
    { -9714.402f, 1520.84f, 27.468222f},
    { -9715.152f, 1520.10f, 27.468222f},
    { -9716.243f, 1519.03f, 27.468222f},
    { -9717.179f, 1517.72f, 27.467699f},           // teleport location - need to be hardcoded because the player is teleported after the larva is summoned
};

struct boss_ayamissAI : public ScriptedAI
{
    boss_ayamissAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;

    uint32 StingerSprayTimer;
    uint32 PoisonStingerTimer;
    uint32 SummonSwarmerTimer;
    uint32 SwarmerAttackTimer;
    uint32 ParalyzeTimer;
    uint32 LashTimer;
    uint32 TrashTimer;
    uint8 Phase;

    bool HasFrenzy;

    uint64 m_paralyzeTarget;
    std::list<uint64> SwarmersGuidList;

    void Reset()
    {
        StingerSprayTimer = urand(20000, 30000);
        PoisonStingerTimer = 5000;
        SummonSwarmerTimer = 5000;
        SwarmerAttackTimer = 60000;
        ParalyzeTimer = 15000;
        LashTimer = urand(5000, 8000);
        TrashTimer = urand(3000, 6000);

        HasFrenzy = false;
        // me->SetDefaultMovementType(POINT_MOTION_TYPE);
        Phase = PHASE_AIR;
        // SetCombatMovement(false);
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        me->SetLevitate(true);
        me->GetMap()->CreatureRelocation(me, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 15.0f, me->GetOrientation());
        me->GetMotionMaster()->MoveIdle();
        me->SendHeartBeat();
        me->SetRooted(true);
        // me->GetMotionMaster()->MovePoint(1, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 15.0f);
    }

    void JustSummoned(Creature* pSummoned)
    {
        // store the swarmers for a future attack
        if (pSummoned->GetEntry() == NPC_SWARMER)
            SwarmersGuidList.push_back(pSummoned->GetGUID());
        // move the larva to paralyze target position
        else if (pSummoned->GetEntry() == NPC_LARVA)
        {
            pSummoned->SetWalk(false);
            pSummoned->GetMotionMaster()->MovePoint(1, aAyamissSpawnLocs[3].m_fX, aAyamissSpawnLocs[3].m_fY, aAyamissSpawnLocs[3].m_fZ);
        }
        else if (pSummoned->GetEntry() == NPC_HORNET)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    void MovementInform(uint32 type, uint32 uiPointId)
    {
        /*if (uiPointId == 1)
            me->SetRooted(true);
        else if(uiPointId == 2)
        {
            me->SetDefaultMovementType(IDLE_MOTION_TYPE);
            if (me->GetVictim())
                me->AI()->AttackStart(me->GetVictim());
        }*/
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId)
    {
        if(pSummoned->GetEntry() != NPC_LARVA || uiMotionType != POINT_MOTION_TYPE)
            return;

        switch(uiPointId)
        {
            case 1:
                pSummoned->GetMotionMaster()->MovePoint(2, aAyamissSpawnLocs[4].m_fX, aAyamissSpawnLocs[4].m_fY, aAyamissSpawnLocs[4].m_fZ);
                break;
            case 2:
                pSummoned->GetMotionMaster()->MovePoint(3, aAyamissSpawnLocs[5].m_fX, aAyamissSpawnLocs[5].m_fY, aAyamissSpawnLocs[5].m_fZ);
                break;
            case 3:
                pSummoned->GetMotionMaster()->MovePoint(4, aAyamissSpawnLocs[6].m_fX, aAyamissSpawnLocs[6].m_fY, aAyamissSpawnLocs[6].m_fZ);
                break;
            case 4:
                pSummoned->GetMotionMaster()->MovePoint(5, aAyamissSpawnLocs[7].m_fX, aAyamissSpawnLocs[7].m_fY, aAyamissSpawnLocs[7].m_fZ);
                break;
            case 5:
                pSummoned->GetMotionMaster()->MovePoint(6, aAyamissSpawnLocs[8].m_fX, aAyamissSpawnLocs[8].m_fY, aAyamissSpawnLocs[8].m_fZ);
                break;
            case 6:
                pSummoned->GetMotionMaster()->MovePoint(7, aAyamissSpawnLocs[9].m_fX, aAyamissSpawnLocs[9].m_fY, aAyamissSpawnLocs[9].m_fZ);
                break;
            case 7:
                pSummoned->GetMotionMaster()->MovePoint(8, aAyamissSpawnLocs[10].m_fX, aAyamissSpawnLocs[10].m_fY, aAyamissSpawnLocs[10].m_fZ);
                break;
            case 8:
                pSummoned->GetMotionMaster()->MovePoint(9, aAyamissSpawnLocs[11].m_fX, aAyamissSpawnLocs[11].m_fY, aAyamissSpawnLocs[1].m_fZ);
                break;
            case 9:
                pSummoned->GetMotionMaster()->MovePoint(10, aAyamissSpawnLocs[12].m_fX, aAyamissSpawnLocs[12].m_fY, aAyamissSpawnLocs[12].m_fZ);
                break;
            case 10:
                pSummoned->GetMotionMaster()->MovePoint(11, aAyamissSpawnLocs[13].m_fX, aAyamissSpawnLocs[13].m_fY, aAyamissSpawnLocs[13].m_fZ);
                break;
            case 11:
                pSummoned->GetMotionMaster()->MovePoint(12, aAyamissSpawnLocs[14].m_fX, aAyamissSpawnLocs[14].m_fY, aAyamissSpawnLocs[15].m_fZ);
                break;
            case 12:
                pSummoned->GetMotionMaster()->MovePoint(13, aAyamissSpawnLocs[15].m_fX, aAyamissSpawnLocs[15].m_fY, aAyamissSpawnLocs[15].m_fZ);
                break;
            case 13:
                pSummoned->GetMotionMaster()->MovePoint(14, aAyamissSpawnLocs[16].m_fX, aAyamissSpawnLocs[16].m_fY, aAyamissSpawnLocs[16].m_fZ);
                break;
            case 14: // TP Point
            {
                // Cast feed on target
                if (Unit* pTarget = Unit::GetUnit((*me), m_paralyzeTarget))
                {
                    if (pSummoned->IsWithinMeleeRange(pTarget))
                        pSummoned->CastSpell(pTarget, SPELL_FEED, true, NULL, NULL, me->GetGUID());
                }
                break;
            }
            default: break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!HasFrenzy && me->GetHealthPercent() < 20.0f)
        {
            AddSpellToCast(SPELL_FRENZY, CAST_SELF);
            DoScriptText(EMOTE_GENERIC_FRENZY, me);
            HasFrenzy = true;
        }

        // Stinger Spray
        if (StingerSprayTimer < diff)
        {
            AddSpellToCast(SPELL_STINGER_SPRAY, CAST_NULL);
            StingerSprayTimer = urand(15000, 20000);
        }
        else
            StingerSprayTimer -= diff;

        // Paralyze
        if (ParalyzeTimer < diff)
        {
            Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1, GetSpellMaxRange(SPELL_PARALYZE), true);
            if (!pTarget)
                pTarget = me->GetVictim();

            DoCast(pTarget, SPELL_PARALYZE, true);
            m_paralyzeTarget = pTarget->GetGUID();
            ParalyzeTimer = 15000;

            // Summon a larva
            uint8 uiLoc = urand(0, 1);
            me->SummonCreature(NPC_LARVA, aAyamissSpawnLocs[uiLoc].m_fX, aAyamissSpawnLocs[uiLoc].m_fY, aAyamissSpawnLocs[uiLoc].m_fZ, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 30000);
        }
        else
            ParalyzeTimer -= diff;

        // Summon Swarmer
        if (SummonSwarmerTimer < diff)
        {
            // The spell which summons these guys was removed in 2.0.1 -> therefore we need to summon them manually at a random location around the area
            // The summon locations is guesswork - the real location is supposed to be handled by world triggers
            // There should be about 24 swarmers per min
            float fX, fY, fZ;
            for (uint8 i = 0; i < 2; ++i)
            {
                me->GetRandomPoint(aAyamissSpawnLocs[2].m_fX, aAyamissSpawnLocs[2].m_fY, aAyamissSpawnLocs[2].m_fZ, 80.0f, fX, fY, fZ);
                me->SummonCreature(NPC_SWARMER, fX, fY, aAyamissSpawnLocs[2].m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 0);
            }
            SummonSwarmerTimer = 5000;
        }
        else
            SummonSwarmerTimer -= diff;

        // All the swarmers attack at a certain period of time
        if (SwarmerAttackTimer < diff)
        {
            for (std::list<uint64>::const_iterator itr = SwarmersGuidList.begin(); itr != SwarmersGuidList.end(); ++itr)
            {
                if (Creature* pTemp = me->GetMap()->GetCreature(*itr))
                {
                    if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                        pTemp->AI()->AttackStart(pTarget);
                }
            }
            SwarmersGuidList.clear();
            SwarmerAttackTimer = 60000;
        }
        else
            SwarmerAttackTimer -= diff;

        if (Phase == PHASE_AIR)
        {
            // Start ground phase at 70% of HP
            if (me->GetHealthPercent() <= 70.0f)
            {
                Phase = PHASE_GROUND;
                // SetCombatMovement(true);
                me->SetRooted(false);
                me->SetLevitate(false);
                DoResetThreat();

                me->GetMotionMaster()->MoveIdle();
                me->GetMotionMaster()->MoveFall();
                me->SendHeartBeat();
            }

            // Poison Stinger
            if (PoisonStingerTimer < diff)
            {
                AddSpellToCast(SPELL_POISON_STINGER, CAST_TANK);
                PoisonStingerTimer = urand(2000, 3000);
            }
            else
                PoisonStingerTimer -= diff;
        }
        else
        {
            if (LashTimer < diff)
            {
                AddSpellToCast(SPELL_LASH, CAST_TANK);
                LashTimer = urand(8000, 15000);
            }
            else
                LashTimer -= diff;

            if (TrashTimer < diff)
            {
                AddSpellToCast(SPELL_TRASH, CAST_TANK);
                TrashTimer = urand(5000, 7000);
            }
            else
                TrashTimer -= diff;

            CastNextSpellIfAnyAndReady();
            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_boss_ayamiss(Creature *_Creature)
{
    return new boss_ayamissAI (_Creature);
}

struct larvaAI : public ScriptedAI
{
    larvaAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)c->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    void Reset()
    {
    }

    void AttackStart(Unit* pWho)
    {
        // don't attack anything during the Ayamiss encounter
        if (pInstance)
        {
            if (pInstance->GetData(DATA_AYAMISS_THE_HUNTER) == IN_PROGRESS)
                return;
        }

        ScriptedAI::AttackStart(pWho);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        // don't attack anything during the Ayamiss encounter
        if (pInstance)
        {
            if (pInstance->GetData(DATA_AYAMISS_THE_HUNTER) == IN_PROGRESS)
                return;
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void UpdateAI(const uint32 diff)
    {
        if (pInstance)
        {
            if (pInstance->GetData(DATA_AYAMISS_THE_HUNTER) == IN_PROGRESS)
                return;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_larva(Creature *_Creature)
{
    return new larvaAI (_Creature);
}

void AddSC_boss_ayamiss()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_ayamiss";
    newscript->GetAI = &GetAI_boss_ayamiss;
    newscript->RegisterSelf();
}

void AddSC_larva()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="larva";
    newscript->GetAI = &GetAI_larva;
    newscript->RegisterSelf();
}
