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
SDName: Boss_Nightbane
SD%Complete: 80
SDComment: SDComment: Timers may incorrect
SDCategory: Karazhan
EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

#define EMOTE_SUMMON                -1901037
#define YELL_AGGRO                  -1200231
#define YELL_FLY_PHASE              -1200232
#define YELL_LAND_PHASE_1           -1200233
#define YELL_LAND_PHASE_2           -1200234
#define EMOTE_BREATH                -1200235

float FlightPhase[6][4]=
{
    { -11165.3, -1870.9,  117.1,   5.4 },
    { -11130,   -1905.58, 117.1,   0   },
    { -11121.4, -1945.46, 117.1,   0   },
    { -11156.3, -1957.35, 117.1,   0   },
    { -11169.9, -1933.05, 117.1,   0   },
    { -11163,   -1907.3,  91.4738, 0   }
};

float IntroNew[9][3]=
{
    { -11023.3, -1777.1,  167.7   },
    { -11115.8, -1879,    130.7   },
    { -11198.7, -1848,    98.8    },
    { -11247.5, -1809.3,  95.6    },
    { -11241.9, -1762,    86      },
    { -11180.9, -1812.2,  100.2   },
    { -11173.3, -1873.72, 100.416 },
    { -11115.2, -1869.09, 100.017 },
    { -11143.5, -1894.3,  91.4738 }
};

struct boss_nightbaneAI : public ScriptedAI
{
    boss_nightbaneAI(Creature* c) : ScriptedAI(c), summons(me)
    {
        pInstance = (c->GetInstanceData());
        c->setActive(true);
        Intro = true;
        Summoned = true;
    }

    enum Spells
    {
        //phase 1
        SPELL_BELLOWING_ROAR        = 39427,
        SPELL_CHARRED_EARTH         = 30129,
        SPELL_DISTRACTING_ASH       = 30130,
        SPELL_SMOLDERING_BREATH     = 30210,
        SPELL_TAIL_SWEEP            = 25653,
        SPELL_CLEAVE                = 30131,

        //phase 2
        SPELL_RAIN_OF_BONES         = 37098,
        SPELL_RAIN_OF_BONES_EFFECT  = 37091,
        SPELL_SMOKING_BLAST         = 37057,
        SPELL_FIREBALL_BARRAGE      = 30282,
        SPELL_SEARING_CINDERS       = 30127,
        SPELL_SUMMON_SKELETON       = 30170
    };

    ScriptedInstance* pInstance;
    SummonList summons;

    enum
    {
        PHASE_GROUND,
        PHASE_FLIGHT
    } Phase;

    Timer BellowingRoarTimer;
    Timer CharredEarthTimer;
    Timer DistractingAshTimer;
    Timer SmolderingBreathTimer;
    Timer TailSweepTimer;
    Timer RainofBonesTimer;
    Timer SmokingBlastTimer;
    Timer FireballBarrageTimer;
    Timer SearingCindersTimer;
    Timer Cleave_Timer;
    Timer SkeletonSummonTimer;

    uint64 RoBTarget;
    uint32 SkeletonSummoned;
    uint32 FlyCount;
    Timer FlyTimer;

    int32 MovePhase;
    Timer WaitTimer;

    bool Summoned;
    bool Intro;
    bool Flying;
    bool Movement;
    bool FlightMovementAllowed;

    void Reset()
    {
        ClearCastQueue();
        if (Summoned)
        {
            if(pInstance)
            {
                if (pInstance->GetData64(DATA_NIGHTBANE) || pInstance->GetData(DATA_NIGHTBANE_EVENT) == DONE)
                {
                    me->setDeathState(JUST_DIED);
                    me->RemoveCorpse();
                }
                else
                    pInstance->SetData64(DATA_NIGHTBANE, me->GetGUID());
            }
        }

        // ground phase timers
        BellowingRoarTimer.Reset(urand(20000, 30000));
        CharredEarthTimer.Reset(urand(10000, 15000));
        SmolderingBreathTimer.Reset(urand(9000, 13000));
        TailSweepTimer.Reset(urand(12000, 15000));
        Cleave_Timer.Reset(urand(4000, 8000));

        // flight phase timers
        DistractingAshTimer.Reset(0);
        RainofBonesTimer = 0;
        SmokingBlastTimer = 0;
        FireballBarrageTimer.Reset(10000);
        SearingCindersTimer.Reset(14000);
        WaitTimer.Reset(1000);
        SkeletonSummonTimer.Reset(0);

        RoBTarget = 0;
        SkeletonSummoned = 0;

        Phase = PHASE_GROUND;
        FlyCount = 0;
        MovePhase = 1;

        me->SetReactState(REACT_AGGRESSIVE);

        if (pInstance && pInstance->GetData(DATA_NIGHTBANE_EVENT) != DONE)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, NOT_STARTED);

        HandleTerraceDoors(true);

        Flying = false;
        Movement = false;
        FlightMovementAllowed = true;

        me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);

        if (!Intro)
        {
            me->SetHomePosition(IntroNew[8][0], IntroNew[8][1], IntroNew[8][2], 0);
            me->GetMotionMaster()->MoveTargetedHome();
        }

        Summoned = false;
        summons.DespawnAll();
    }

    void HandleTerraceDoors(bool open)
    {
        if(pInstance)
        {
            if (GameObject *Door = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_MASTERS_TERRACE_DOOR_1)))
            {
                Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
                if(open)
                    Door->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                else
                    Door->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
            }
            if (GameObject *Door = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_MASTERS_TERRACE_DOOR_2)))
            {
                Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
                if(open)
                    Door->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                else
                    Door->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, IN_PROGRESS);

        HandleTerraceDoors(false);
        DoYell(-1200231, LANG_UNIVERSAL, NULL);
    }

    void DamageTaken(Unit*, uint32 &damage)
    {
        if (Phase == PHASE_FLIGHT && (damage >= me->GetHealth() || me->GetHealth() <= 1))
            damage = 0;
    }

    void AttackStart(Unit* who)
    {
        if (!Intro && !Flying)
            ScriptedAI::AttackStart(who);
    }

    void JustDied(Unit* killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_NIGHTBANE_EVENT, DONE);

        HandleTerraceDoors(true);
        summons.DespawnAll();

        // in fact this check needless for low-rate - impossible to kill boss in fly phase, it's needed for x100 realm
        if((killer->GetPositionZ() - me->GetPositionZ() >= 10.0) || (me->GetPositionZ() - killer->GetPositionZ() >= 10.0))
            me->Relocate(killer->GetPositionX(), killer->GetPositionY(), killer->GetPositionZ());
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!Intro && !Flying)
            if (!me->GetVictim() && me->canStartAttack(who))
                ScriptedAI::AttackStart(who);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
        {
            if (Flying)
                DoStartNoMovement(me->GetVictim());
            return;
        }

        if (Intro)
        {
            if (id >= 8)
            {
                Intro = false;
                me->SetHomePosition(IntroNew[8][0], IntroNew[8][1], IntroNew[8][2], 0);
                me->SetSpeed(MOVE_FLIGHT, 1, true);
                me->SetSpeed(MOVE_RUN, 1.5, true);
                me->SetLevitate(false);
                me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                DoZoneInCombat(200);
                return;
            }

            WaitTimer = 100;
        }

        if (Flying)
        {
            switch (id)
            {
                case 0:
                {
                    DoTextEmote(-1200235, NULL, true);
                    RainofBonesTimer = 1000;
                    me->SetRooted(true);
                    Flying = true;
                    break;
                }
                case 5:
                {
                    ClearCastQueue();
                    DoResetThreat();
                    me->SetLevitate(false);
                    me->setHover(false);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                    me->SetWalk(true);
                    DoZoneInCombat(200.0f);
                    UpdateVictim();
                    Flying = false;
                    Phase = PHASE_GROUND;
                    Movement = true;
                    FlightMovementAllowed = true;
                    return;
                }
            }

            if(FlightMovementAllowed)
                WaitTimer = 100;
        }
    }

    void JustSummoned(Creature *summoned)
    {
        summons.Summon(summoned);
        if (summoned->GetPositionZ() < 85.0f)
        {
            if (Unit* victim = me->GetVictim())
                summoned->Relocate(victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ());
            else
            {
                summoned->DisappearAndDie();
                return;
            }
        }

        if(me->GetVictim())
            summoned->AI()->AttackStart(me->GetVictim());
        else
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
                summoned->AI()->AttackStart(target);
        }
    }
    
    void JustRespawned()
    {
        me->MonsterTextEmoteToZone(EMOTE_SUMMON, 0, false, true);
        me->SetSpeed(MOVE_RUN, 1.5f);
        me->SetLevitate(true);
        me->SetWalk(false);
        me->setHover(true);
    }

    void TakeOff()
    {
        ClearCastQueue();

        DoYell(-1200232, LANG_UNIVERSAL, NULL);

        DoResetThreat();

        me->InterruptSpell(CURRENT_GENERIC_SPELL);
        me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);

        me->SetLevitate(true);
        me->SetWalk(false);
        me->setHover(true);

        me->GetMotionMaster()->Clear(false);
        me->GetMotionMaster()->MovePoint(0, FlightPhase[0][0], FlightPhase[0][1], FlightPhase[0][2]);

        
        Phase = PHASE_FLIGHT;
        MovePhase = 1;

        FlightMovementAllowed = false;
        Flying = true;

        ++FlyCount;

        FlyTimer.Reset(48000); // seems like good value
        DistractingAshTimer = 0;
        SmokingBlastTimer = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (WaitTimer.Expired(diff))
        {
            if (Intro)
            {
                if (MovePhase >= 7)
                {
                    me->GetMotionMaster()->MovePoint(8, IntroNew[8][0], IntroNew[8][1], IntroNew[8][2]);
                }
                else
                {
                    me->GetMotionMaster()->MovePoint(MovePhase, IntroNew[MovePhase][0], IntroNew[MovePhase][1], IntroNew[MovePhase][2]);
                    me->SetSpeed(MOVE_FLIGHT, 2, true);
                    me->SetSpeed(MOVE_RUN, 2, true);
                    me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                    ++MovePhase;
                }
            }

            if (Flying)
            {
                if (MovePhase == 5)
                {
                    me->GetMotionMaster()->MovePoint(1, FlightPhase[1][0], FlightPhase[1][1], FlightPhase[1][2]);
                    MovePhase = 2;
                }
                else
                {
                    me->GetMotionMaster()->MovePoint(MovePhase, FlightPhase[MovePhase][0], FlightPhase[MovePhase][1], FlightPhase[MovePhase][2]);
                    ++MovePhase;
                    me->SetRooted(false);
                    me->setHover(false);
                }
            }

            WaitTimer = 0;
        }

        if (!Flying || !me->IsInCombat())
            if (!UpdateVictim())
                return;

        DoSpecialThings(diff, DO_PULSE_COMBAT);

        if (Phase == PHASE_GROUND)
        {
            if (Movement)
            {
                DoStartMovement(me->GetVictim());
                Movement = false;
            }

            if (BellowingRoarTimer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), SPELL_BELLOWING_ROAR);
                BellowingRoarTimer = urand(20000, 30000);
            }

            if (SmolderingBreathTimer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), SPELL_SMOLDERING_BREATH);
                SmolderingBreathTimer = urand(14000, 20000);
            }

            if (Cleave_Timer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), SPELL_CLEAVE);
                Cleave_Timer = urand(6000, 12000);
            }

            if (CharredEarthTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_CHARRED_EARTH), true))
                    AddSpellToCast(target, SPELL_CHARRED_EARTH);
                CharredEarthTimer = urand(25000, 35000);
            }

            if (TailSweepTimer.Expired(diff))
            {
                std::vector<Unit*> playersBehind;
                std::list<HostileReference*>& threatList = me->getThreatManager().getThreatList();
                for(std::list<HostileReference*>::iterator i = threatList.begin(); i != threatList.end(); ++i)
                {
                    if (Unit* target = Unit::GetUnit((*me), (*i)->getUnitGuid()))
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER && !target->isInBack(me, 30, M_PI/3)) // 60 degrees
                            playersBehind.push_back(target);
                    }
                }

                Unit* target = !playersBehind.empty() ? playersBehind[urand(0, playersBehind.size()-1)] : 0;
                if(target)
                    AddSpellToCast(target, SPELL_TAIL_SWEEP, false, true);
                TailSweepTimer = urand(14000, 20000);
            }

            if (HealthBelowPct((3 - FlyCount) * 25))
                TakeOff();

            DoMeleeAttackIfReady();
        }
        else if (Phase == PHASE_FLIGHT)
        {
            if (RainofBonesTimer.Expired(diff)) // only once at the beginning of phase 2
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_RAIN_OF_BONES), true, 0))
                {
                    AddSpellToCast(target, SPELL_RAIN_OF_BONES, false, true);
                    SkeletonSummonTimer = 2000;
                    RoBTarget = target->GetGUID();
                }

                RainofBonesTimer = 0;
                WaitTimer = 11000;
                SmokingBlastTimer = 12000; // add to cast queue
                DistractingAshTimer = 12000; // add to cast queue
                FlightMovementAllowed = true;
            }

            if(SkeletonSummonTimer.Expired(diff))
            {
                if (Unit* target = Unit::GetUnit((*me), RoBTarget))
                {
                    DoCast(target, SPELL_SUMMON_SKELETON, true);
                    SkeletonSummoned++;
                }

                if(SkeletonSummoned >= 5)
                {
                    SkeletonSummonTimer = 0;
                    SkeletonSummoned = 0;
                }
                else
                    SkeletonSummonTimer = 2000;
            }

            if (DistractingAshTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_DISTRACTING_ASH), true))
                    me->AddAura(SPELL_DISTRACTING_ASH, target);

                DistractingAshTimer = urand(7000, 13000);
            }

            if (SmokingBlastTimer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), SPELL_SMOKING_BLAST);
                SmokingBlastTimer = 1000;
            }

            if (SearingCindersTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_SEARING_CINDERS), true))
                    AddSpellToCast(target, SPELL_SEARING_CINDERS);
                SearingCindersTimer = urand(10000, 13000);
            }

            if (FireballBarrageTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_FARTHEST, 0, 200.0f, true))
                    if (target->GetDistance(me) >= 80.0)
                        AddSpellToCast(me->GetVictim(), SPELL_FIREBALL_BARRAGE);
                FireballBarrageTimer = 1000;
            }

            if (FlyTimer.Expired(diff)) // landing
            {
                if (rand() % 2)
                    DoYell(-1200233, LANG_UNIVERSAL, NULL);
                else
                    DoYell(-1200234, LANG_UNIVERSAL, NULL);

                me->GetMotionMaster()->Clear(false);
                FlightMovementAllowed = false;
                me->GetMotionMaster()->MovePoint(5, FlightPhase[5][0], FlightPhase[5][1], FlightPhase[5][2]);

                FlyTimer = 0;
            }
        }
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_boss_nightbane(Creature *_Creature)
{
    return new boss_nightbaneAI(_Creature);
}

void AddSC_boss_nightbane()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_nightbane";
    newscript->GetAI = &GetAI_boss_nightbane;
    newscript->RegisterSelf();
}

