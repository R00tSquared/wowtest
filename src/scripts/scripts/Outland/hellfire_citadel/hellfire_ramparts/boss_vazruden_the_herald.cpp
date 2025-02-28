// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
Name: Boss_Vazruden_the_Herald
%Complete: 95
Comment:
Category: Hellfire Citadel, Hellfire Ramparts
EndScriptData */

#include "precompiled.h"
#include "hellfire_ramparts.h"

enum VazrudenAndNazan
{
    SAY_INTRO                   = -1543017,
    SAY_AGGRO1                  = -1543018,
    SAY_AGGRO2                  = -1543019,
    SAY_AGGRO3                  = -1543020,
    SAY_TAUNT                   = -1543021,
    SAY_KILL1                   = -1543022,
    SAY_KILL2                   = -1543023,
    SAY_DEATH                   = -1543024,
    EMOTE_DESCEND               = -1543025,

    POINT_ID_CENTER             = 11,
    POINT_ID_FLYING             = 12,
    POINT_ID_COMBAT             = 13,

    SPELL_SUMMON_VAZRUDEN       = 30717,
    // Vazruden
    SPELL_REVENGE_N             = 19130,
    SPELL_REVENGE_H             = 40392,
    // Nazan
    SPELL_FIREBALL_N            = 34653,
    SPELL_FIREBALL_H            = 36920,
    SPELL_CONE_OF_FIRE_N        = 30926,
    SPELL_CONE_OF_FIRE_H        = 36921,
    SPELL_SUMMON_LIQUID_FIRE_N  = 23971,
    SPELL_SUMMON_LIQUID_FIRE_H  = 30928,
    SPELL_BELLOW_ROAR           = 39427,
    
    SPELL_BLAZE_N               = 30927,
    SPELL_BLAZE_H               = 32492,
    SPELL_KIDNEY_SHOT           = 30621,
    SPELL_FIRE_NOVA_VISUAL      = 19823,

    ENTRY_HELLFIRE_SENTRY       = 17517,
    ENTRY_VAZRUDEN_HERALD       = 17307,
    ENTRY_VAZRUDEN              = 17537,
    ENTRY_NAZAN                 = 17536,
    ENTRY_LIQUID_FIRE           = 22515,

    PATH_ENTRY                  = 2081,
    ENTRY_REINFORCED_FEL_IRON_CHEST_N   = 185168,
    ENTRY_REINFORCED_FEL_IRON_CHEST_H   = 185169
};

const float CenterPos[3] = { -1399.401f, 1736.365f, 87.008f}; // moves here to drop off nazan
const float CombatPos[3] = { -1413.848f, 1754.019f, 83.146f}; // moves here when decending

struct boss_vazruden_the_heraldAI : public ScriptedAI
{
    boss_vazruden_the_heraldAI(Creature* creature) : ScriptedAI(creature)
    {
        pInstance = (creature->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;

    bool HeroicMode;
    bool SentryDown;
    bool IsEventInProgress;
    bool IsDescending;
    uint8 Phase;
    Timer Timer_Movement;
    Timer Timer_Fireball;
    Timer ConeOfFireTimer;
    Timer BellowingRoarTimer;
    Timer Timer_Check;
    Timer Timer_Cast;
    Timer Timer_Blaze;
    uint64 PlayerGUID;
    uint64 VazrudenGUID;
    uint64 VictimGUID;
    uint64 SummonedGUID;

    void Reset()
    {
        if (me->GetEntry() != ENTRY_VAZRUDEN_HERALD)
            me->UpdateEntry(ENTRY_VAZRUDEN_HERALD);

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);

        Timer_Movement.Reset(0);
        Timer_Fireball.Reset(0);
        Timer_Check.Reset(0);
        ConeOfFireTimer.Reset(7000);
        BellowingRoarTimer.Reset(20000);
        Timer_Cast.Reset(0);
        Timer_Blaze.Reset(0);

        SentryDown = false;
        IsEventInProgress = false;
        IsDescending = false;

        Phase = 0;

        PlayerGUID = 0;
        VazrudenGUID = 0;
        VictimGUID = 0;
        SummonedGUID = 0;
        
        me->SetLevitate(true);
        me->SetSpeed(MOVE_FLIGHT, 1.0f);
        me->GetMotionMaster()->MovePath(PATH_ENTRY, true);

        if (Creature* Vazruden = GetClosestCreatureWithEntry(me, ENTRY_VAZRUDEN, 100.0f, false))
        {
            Vazruden->SetLootRecipient(NULL);
            Vazruden->RemoveCorpse();
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* entry)
     {
        if (target && ((entry->Id == SPELL_FIREBALL_N) || (entry->Id == SPELL_FIREBALL_H)))
            me->SummonCreature(ENTRY_LIQUID_FIRE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);
     }

    void AttackStart(Unit* who)
    {
        if (pInstance && pInstance->GetData(DATA_NAZAN) != IN_PROGRESS)
            return;

        ScriptedAI::AttackStart(who);
        DoZoneInCombat();
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (!pInstance)
            return;

        if (type == POINT_MOTION_TYPE)
        {
            switch (id)
            {
                case POINT_ID_CENTER:
                    if (!VazrudenGUID)
                        DoSplit();
                    break;
                case POINT_ID_COMBAT:
                    me->ClearUnitState(UNIT_STAT_IGNORE_PATHFINDING);
                    me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                    pInstance->SetData(DATA_NAZAN, IN_PROGRESS);
                    me->SetLevitate(false);
                    me->SetWalk(true);
                    me->GetMotionMaster()->Clear();

                    if (Unit *victim = SelectUnit(SELECT_TARGET_NEAREST,0))
                        me->AI()->AttackStart(victim);

                    Timer_Fireball = 8000;
                    Phase = 2;
                    break;
                case POINT_ID_FLYING:
                    if (IsEventInProgress) // Additional check for wipe case, while nazan is flying to this point
                    {
                        me->GetMotionMaster()->MovePath(PATH_ENTRY, true);
                        Timer_Fireball = 1;
                    }
                    break;
            }
        }
    }

    void DoStart()
    {
        if (Timer_Movement.GetInterval() || IsEventInProgress)
            return;

        if (pInstance)
        {
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            DoMoveToCenter();
            IsEventInProgress = true;
        }

        me->SetInCombatWithZone();
        Phase = 1;
    }

    void DoMoveToCenter()
    {
        DoScriptText(SAY_INTRO, me);
        me->GetMotionMaster()->MovePoint(POINT_ID_CENTER, CenterPos[0], CenterPos[1], CenterPos[2]);
    }

    void DoSplit()
    {
        me->UpdateEntry(ENTRY_NAZAN);
        DoCast(me, SPELL_SUMMON_VAZRUDEN);
        Timer_Movement = 2000;
        Timer_Check = 10000;
        me->SetSpeed(MOVE_FLIGHT, 2.0f);
        me->GetMotionMaster()->MoveIdle();
    }

    void DoMoveToAir()
    {
        float x, y, z, o;
        me->GetHomePosition(x, y, z, o);
        me->GetMotionMaster()->MovePoint(POINT_ID_FLYING, x, y, z);
    }

    void DoMoveToCombat()
    {
        if (IsDescending || !pInstance || pInstance->GetData(DATA_NAZAN) == IN_PROGRESS)
            return;

        IsDescending = true;
        me->GetMotionMaster()->MovePoint(POINT_ID_COMBAT, CombatPos[0], CombatPos[1], CombatPos[2]);
        DoScriptText(EMOTE_DESCEND, me);
    }

    void JustSummoned(Creature* summoned)
    {
        switch (summoned->GetEntry())
        {
            case ENTRY_VAZRUDEN:
                if (Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID))
                    summoned->AI()->AttackStart(pPlayer);

                VazrudenGUID = summoned->GetGUID();
                summoned->AI()->DoZoneInCombat();
                break;
            case ENTRY_LIQUID_FIRE:
                summoned->SetLevel(me->GetLevel());
                summoned->setFaction(me->getFaction());
                SummonedGUID = summoned->GetGUID();
                Timer_Cast = 1000;
                break;
        }
    }

    void DoDescend()
    {
        if (IsDescending)
            return;

        DoMoveToCombat();
    }

    void SentryDownBy(Unit* who)
    {
        if (SentryDown)
        { 
            PlayerGUID = who->GetGUID();
            DoStart();
            SentryDown = false;
        }
        else
            SentryDown = true;
    }

    void SelectVictim(Unit* victim)
    {
        VictimGUID = victim->GetGUID();
    }

    void JustDied(Unit* killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_NAZAN, DONE);
    }

    void JustReachedHome()
    {
        if (pInstance)
            pInstance->SetData(DATA_NAZAN, FAIL);
    }

    void UpdateAI(const uint32 diff)
    {
        if(Timer_Cast.Expired(diff))
        {
            if (Creature* trigger = me->GetCreature(SummonedGUID))
            {
                trigger->CastSpell(trigger, SPELL_FIRE_NOVA_VISUAL, true);
                trigger->CastSpell(trigger, HeroicMode ? SPELL_SUMMON_LIQUID_FIRE_H : SPELL_SUMMON_LIQUID_FIRE_N, true);
            }
            Timer_Cast = 0;
            Timer_Blaze = 1000;
        }

        if(Timer_Blaze.Expired(diff))
        {
            if (Creature* trigger = me->GetCreature(SummonedGUID))
            {
                trigger->CastSpell(trigger, HeroicMode ? SPELL_BLAZE_H : SPELL_BLAZE_N, true);
            }
            Timer_Blaze = 1000;
        }

        switch (Phase)
        {
            case 0:
                return;
                break;
            case 1: // flight
            {
                if (Timer_Movement.Expired(diff))
                {
                    DoMoveToAir();
                    Timer_Movement = 0;
                }

                if (VazrudenGUID && Timer_Fireball.GetInterval())
                {
                    if (Timer_Fireball.Expired(diff))
                    {
                        if (Creature* Vazruden = me->GetMap()->GetCreature(VazrudenGUID))
                        {
                            Vazruden->AI()->DoAction();

                            if (Unit* victim = Unit::GetUnit(*me, VictimGUID))
                            {
                                DoCast(victim, HeroicMode ? SPELL_FIREBALL_H : SPELL_FIREBALL_N);
                                Timer_Fireball = 8000;
                            }
                        }
                    }
                }

                if ((me->GetHealth())*100 / me->GetMaxHealth() < 20)
                    DoMoveToCombat();

                if (me->isAlive() && Timer_Check.Expired(diff))
                {
                    if (Creature* Vazruden = me->GetMap()->GetCreature(VazrudenGUID))
                    {
                        if (Vazruden && Vazruden->GetVictim() || me->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE)
                            return;
                        else
                            EnterEvadeMode();
                    }

                    Timer_Check = 2000;
                       
                }
                break;
            }
            case 2: // In Combat
            {
                if (Timer_Fireball.Expired(diff))
                {
                    if (Unit *victim = SelectUnit(SELECT_TARGET_RANDOM,0))
                    {
                        DoCast(victim, HeroicMode ? SPELL_FIREBALL_H : SPELL_FIREBALL_N);
                        Timer_Fireball = 7000+rand()%3000;
                    }
                }

                if (ConeOfFireTimer.Expired(diff))
                {
                    DoCast(me->GetVictim(), HeroicMode ? SPELL_CONE_OF_FIRE_H : SPELL_CONE_OF_FIRE_N);
                    ConeOfFireTimer = 8300+rand()%3000;
                }

                if (HeroicMode)
                {
                    if (BellowingRoarTimer.Expired(diff))
                    {
                        DoCast(me, SPELL_BELLOW_ROAR);
                        BellowingRoarTimer = 35000+rand()%4000;
                    }
                }

                DoMeleeAttackIfReady();

                if (Timer_Check.Expired(diff))
                {
                    if (!me->GetVictim())
                    {
                        if (Unit *victim = SelectUnit(SELECT_TARGET_NEAREST,0))
                            me->AI()->AttackStart(victim);
                        else
                            EnterEvadeMode();
                    }
                        
                    Timer_Check = 2000;
                        
                }
                break;
            }
        }
    }
};

struct boss_vazrudenAI : public ScriptedAI
{
    boss_vazrudenAI(Creature* creature) : ScriptedAI(creature)
    {
        pInstance = (creature->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;

    bool HeroicMode;
    bool HealthBelow;

    Timer_UnCheked RevengeTimer;
    uint64 VazHeraldGUID;
    
    void Reset()
    {
        HealthBelow = false;
        RevengeTimer.Reset(4000 + rand() % 3000);
        VazHeraldGUID = 0;
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
        {
            if (uint64 VazHeraldGUID = pInstance->GetData64(DATA_VAZHERALD))
            {
                Creature* Nazan = (Unit::GetCreature(*me, VazHeraldGUID));
                CAST_AI(boss_vazruden_the_heraldAI, Nazan->AI())->DoDescend();
            }
        }
    }

    void JustReachedHome()
    {
        me->SetVisibility(VISIBILITY_OFF);
        me->ForcedDespawn();
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(RAND(SAY_KILL1, SAY_KILL2), me);
    }

    void DamageTaken(Unit* dealer, uint32& damage)
    {
        if (!HealthBelow && pInstance && (me->GetHealth()*100 - damage) / me->GetMaxHealth() < 30)
        {
            if (uint64 VazHeraldGUID = pInstance->GetData64(DATA_VAZHERALD))
            {
                Creature* Nazan = (Unit::GetCreature(*me, VazHeraldGUID));
                CAST_AI(boss_vazruden_the_heraldAI, Nazan->AI())->DoDescend();
                Nazan->AI()->DoZoneInCombat();
            }

            HealthBelow = true;
        }
    }

    void DoAction(const int32 action)
    {
        if (Unit *victim = SelectUnit(SELECT_TARGET_RANDOM,0))
        {
            if (uint64 VazHeraldGUID = pInstance->GetData64(DATA_VAZHERALD))
            {
                Creature* Nazan = (Unit::GetCreature(*me, VazHeraldGUID));
                CAST_AI(boss_vazruden_the_heraldAI, Nazan->AI())->SelectVictim(victim);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        DoSpecialThings(diff, DO_PULSE_COMBAT);

        if (RevengeTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), HeroicMode ? SPELL_REVENGE_H : SPELL_REVENGE_N);
            RevengeTimer = 10000+rand()%3000;
        }

        DoMeleeAttackIfReady();
    }
};

struct mob_hellfire_sentryAI : public ScriptedAI
{
    mob_hellfire_sentryAI(Creature *creature) : ScriptedAI(creature)
    {
        pInstance = (creature->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer_UnCheked KidneyShot_Timer;
    uint64 VazHeraldGUID;

    void Reset()
    {
        KidneyShot_Timer.Reset(3000 + rand() % 4000);
        VazHeraldGUID = 0;
    }

    void JustDied(Unit* who)
    {
        if (uint64 VazHeraldGUID = pInstance->GetData64(DATA_VAZHERALD))
        {
            Creature* Nazan = (Unit::GetCreature(*me, VazHeraldGUID));
            CAST_AI(boss_vazruden_the_heraldAI, Nazan->AI())->SentryDownBy(who);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (KidneyShot_Timer.Expired(diff))
        {
            if (Unit *victim = me->GetVictim())
                DoCast(victim, SPELL_KIDNEY_SHOT);

            KidneyShot_Timer = 18000+rand()%3000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_vazruden_the_herald(Creature *creature)
{
    return new boss_vazruden_the_heraldAI (creature);
}

CreatureAI* GetAI_boss_vazruden(Creature *creature)
{
    return new boss_vazrudenAI (creature);
}

CreatureAI* GetAI_mob_hellfire_sentry(Creature *creature)
{
    return new mob_hellfire_sentryAI (creature);
}

void AddSC_boss_vazruden_the_herald()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_vazruden_the_herald";
    newscript->GetAI = &GetAI_boss_vazruden_the_herald;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_vazruden";
    newscript->GetAI = &GetAI_boss_vazruden;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_hellfire_sentry";
    newscript->GetAI = &GetAI_mob_hellfire_sentry;
    newscript->RegisterSelf();
}

