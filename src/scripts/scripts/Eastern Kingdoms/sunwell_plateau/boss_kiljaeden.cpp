// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* Copyright (C) 2009 Trinity <http://www.trinitycore.org/>
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
SDName: Boss_Kiljaeden
SD%Complete: 90
SDComment: BossAI can't be used here due to specific cast.
SDCategory: Sunwell_Plateau
EndScriptData */

#include "precompiled.h"
#include "def_sunwell_plateau.h"
#include "boss_kiljaeden.h"
#include <math.h>

/*### ToDo List ###
- Check Armageddon(Need to look how it works in 10-25ppl raid)
- Correct Kil'Jaeden spawn position
*/

class AllOrbsInGrid
{
public:
    AllOrbsInGrid() {}
    bool operator() (GameObject* go)
    {
        if(go->GetEntry() == GAMEOBJECT_ORB_OF_THE_BLUE_DRAGONFLIGHT)
            return true;
        return false;
    }
};

bool GOUse_go_orb_of_the_blue_flight(Player *plr, GameObject* go)
{
    if (go->GetUInt32Value(GAMEOBJECT_FACTION) == 35)
    {
        ScriptedInstance* pInstance = (go->GetInstanceData());
        float x, y, z, dx, dy, dz;
        Unit* dragon = go->SummonCreature(CREATURE_POWER_OF_THE_BLUE_DRAGONFLIGHT, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 121000);
        if (dragon)
        {
            // Save current pet
            if (Pet* pet = plr->GetPet())
            {
                if (pet->isControlled())
                {
                    plr->SetTemporaryUnsummonedPetNumber(pet->GetCharmInfo()->GetPetNumber());
                    plr->SetOldPetSpell(pet->GetUInt32Value(UNIT_CREATED_BY_SPELL));
                }
                plr->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT);
            }
            plr->CastSpell(dragon, SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT, true);
            plr->CastSpell(plr, SPELL_POSSESS_DRAKE_IMMUNE, true);
        }
        go->SetUInt32Value(GAMEOBJECT_FACTION, 0);
        Unit* Kalec = pInstance->GetCreatureById(CREATURE_KALECGOS);
        if (!Kalec)
            return true;

        go->GetPosition(x, y, z);
        for (uint8 i = 0; i < 4; ++i)
        {
            DynamicObject* Dyn = Kalec->GetDynObject(SPELL_RING_OF_BLUE_FLAMES);
            if (Dyn)
            {
                Dyn->GetPosition(dx, dy, dz);
                if (x == dx && dy == y && dz == z)
                {
                    Dyn->RemoveFromWorld();
                    break;
                }
            }
        }
        go->Refresh();
    }
    return true;
}

//AI for Kalecgos
struct boss_kalecgos_kjAI : public ScriptedAI
{
    boss_kalecgos_kjAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance* pInstance;
    Timer FelmystOutroTimer;
    
    GameObject* Orb[4];
    uint8 OrbsEmpowered;
    uint8 EmpowerCount; // used in HG for different speech when activating orbs
    bool Searched;
    
    Timer MoveTimer;
    uint32 WayPointCounter;
    bool EnableRandomMovement;
    
    void Reset()
    {
        for (uint8 i = 0; i < 4; ++i)
            Orb[i] = NULL;
        FindOrbs();

        FelmystOutroTimer = 0;
        OrbsEmpowered = 0;
        EmpowerCount = 0;
        m_creature->SetLevitate(true);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->setActive(true);
        Searched = false;
        EnableRandomMovement = false;
        MoveTimer = 0;
        WayPointCounter = 0;
    }
    
    void MovementInform(uint32 Type, uint32 Id)
    {
        //if(Type == POINT_MOTION_TYPE)
        //{
        //    switch(Id)
        //    {
        //        if(!EnableRandomMovement)
        //        {
        //            case 1:
        //            case 2:
        //            case 3:
        //            case 4:
        //            {
        //                if (Creature* KJ = pInstance->GetCreatureById(CREATURE_KILJAEDEN))
        //                me->SetFacingToObject(KJ);
        //                break;
        //            }
        //            case 50: // Felmyst outro speach
        //                DoScriptText(YELL_KALECGOS, me);
        //                FelmystOutroTimer.Reset(10000);
        //                break;
        //            case 60: // On starting phase 2
        //                me->DisappearAndDie();
        //                break;
        //        }
        //        default:
        //            break;
        //    }
        //}
    }

    void FindOrbs()
    {
        std::list<GameObject*> orbList;
        AllOrbsInGrid check;
        Hellground::ObjectListSearcher<GameObject, AllOrbsInGrid> searcher(orbList, check);
        Cell::VisitGridObjects(me, searcher, me->GetMap()->GetVisibilityDistance());
        if(orbList.empty())
            return;
        uint8 i = 0;
        for(std::list<GameObject*>::iterator itr = orbList.begin(); itr != orbList.end(); ++itr, ++i)
        {
            Orb[i] = GameObject::GetGameObject(*me, (*itr)->GetGUID());
        }
    }

    void ResetOrbs()
    {
        me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
        for(uint8 i = 0; i < 4; ++i)
            if(Orb[i]) 
                Orb[i]->SetUInt32Value(GAMEOBJECT_FACTION, 0);
    }
    
    // There are two orbs in phase 3, one in phase 4, and all of them in phase 5 at once
    void EmpowerOrb(bool all)
    {
        if (!Orb[OrbsEmpowered])
            return;

        if (all)
        {
            me->RemoveDynObject(SPELL_RING_OF_BLUE_FLAMES);
            for (uint8 i = 0; i < 4; ++i)
            {
                if (!Orb[i]) return;
                Orb[i]->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
                Orb[i]->SetUInt32Value(GAMEOBJECT_FACTION, 35);
                Orb[i]->setActive(true);
                Orb[i]->Refresh();
            }
        }
        else
        {
            uint8 random = urand(0, 3);
            float x, y, z, dx, dy, dz;
            Orb[random]->GetPosition(x, y, z);
            for (uint8 i = 0; i < 4; ++i)
            {
                DynamicObject* Dyn = me->GetDynObject(SPELL_RING_OF_BLUE_FLAMES);
                if (Dyn)
                {
                    Dyn->GetPosition(dx, dy, dz);
                    if (x == dx && dy == y && dz == z)
                    {
                        Dyn->RemoveFromWorld();
                        break;
                    }
                }
            }
            Orb[random]->CastSpell(me, SPELL_RING_OF_BLUE_FLAMES);
            Orb[random]->SetUInt32Value(GAMEOBJECT_FACTION, 35);
            Orb[random]->setActive(true);
            Orb[random]->Refresh();
            ++OrbsEmpowered;
        }
        ++EmpowerCount;

        switch (EmpowerCount)
        {
            case 1: DoScriptText(SAY_KALEC_ORB_READY1, m_creature); break;
            case 2: DoScriptText(SAY_KALEC_ORB_READY2, m_creature); break;
            case 3: DoScriptText(SAY_KALEC_ORB_READY3, m_creature); break;
            case 4: DoScriptText(SAY_KALEC_ORB_READY4, m_creature); break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EnableRandomMovement)
        {
            if(MoveTimer.Expired(diff))
            {
                WayPointCounter++;
                me->SetSpeed(MOVE_FLIGHT, 2.5);
                if(WayPointCounter == 1)
                    me->GetMotionMaster()->MovePoint(1, 1727.970, 654.295, 68.1);
                if(WayPointCounter == 2)
                    me->GetMotionMaster()->MovePoint(2, 1669.036, 660.382, 68.1);
                if(WayPointCounter == 3)
                    me->GetMotionMaster()->MovePoint(3, 1668.920, 599.104, 68.1);
                if(WayPointCounter == 4)
                {
                    me->GetMotionMaster()->MovePoint(4, 1734.431, 593.197, 68.1);
                    WayPointCounter = 0;
                }
                MoveTimer = 9000;
            }
        }

        if(!Searched)
        {
            FindOrbs();
            Searched = true;
        }

        if (FelmystOutroTimer.Expired(diff))
        {
            me->SetSpeed(MOVE_FLIGHT, 2.5);
            me->GetMotionMaster()->MovePoint(60, 1547, 531, 161);
            FelmystOutroTimer = 0;
        }

        if(OrbsEmpowered == 4) OrbsEmpowered = 0;
    }
};

CreatureAI* GetAI_boss_kalecgos_kj(Creature *_Creature)
{
    return new boss_kalecgos_kjAI (_Creature);
}

//AI for Kil'jaeden
struct boss_kiljaedenAI : public Scripted_NoMovementAI
{
    boss_kiljaedenAI(Creature* c) : Scripted_NoMovementAI(c), Summons(me)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    SummonList Summons;

    /* Pre-event start timers - Emerging*/
    uint32 ComeFromFireTimer;
    /* Pre-event start timers - Emerging*/
    uint32 AggroTimer;
    /* Pre-event start timers - Emerging*/
    uint32 KnockBackTimer;
    


    /* 1, 2, 3, 4, 5*/
    uint8 Phase;

    /* all abilities timers*/
    Timer _Timer[TIMER_KJ_MAX];
    Timer SoulFlayCheck;

    /* Kalec is joined on 95% of health if could not make it by timer*/
    bool IsKalecJoined;

    /* Are we casting Darkness at the moment?*/
    bool IsInDarkness;

    /* Orb was already activated in this phase, stops only timer, orb can still be added in other place*/
    bool OrbActivated;

    uint32 waitForTime;
    uint32 activeTimers;

    void Reset()
    {
        // summons are despawned in enterevademode()
        SoulFlayCheck.Reset(750);
        //Phase 2 Timer
        _Timer[P_2_TIMER_KALEC_JOIN].Reset(26000);
        _Timer[P_2_TIMER_LEGION_LIGHTNING].Reset(10000);
        _Timer[P_2_TIMER_FIRE_BLOOM].Reset(13000);
        _Timer[P_2_TIMER_SUMMON_SHILEDORB].Reset(urand(10000, 16000));
        //Phase 3 inactive
        _Timer[P_3_TIMER_SHADOW_SPIKE].Reset(0);
        _Timer[P_3_TIMER_FLAME_DART].Reset(0);
        _Timer[P_3_TIMER_DARKNESS].Reset(0);
        _Timer[P_3_TIMER_ORBS_EMPOWER].Reset(0);
        //Phase 4 inactive
        _Timer[P_4_TIMER_ARMAGEDDON].Reset(0);

        // Other Timers 
        ComeFromFireTimer               = 9000;
        AggroTimer                      = 0;
        KnockBackTimer                  = 2000;

        Phase                           = PHASE_DECEIVERS;
        IsKalecJoined                   = false;
        IsInDarkness                    = false;
        OrbActivated                    = false;

        me->SetFloatValue(UNIT_FIELD_COMBATREACH, 18);
        me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_SINISTER_REFLECTION, true);
        me->SetReactState(REACT_PASSIVE);

        waitForTime = 0;
        activeTimers = TIMER_KJ_MAX;
    }
    
    void DoEpilogue()
    {
        Creature* Kalec = pInstance ? pInstance->GetCreatureById(CREATURE_KALECGOS) : NULL;

        // Kalec could disappear???
        if(Kalec)
        {
            Kalec->setFaction(35);
            Kalec->NearTeleportTo(1667.391968, 634.631409, 28.050337, 0.120061);
            Kalec->SetFacingToObject(me);
            Kalec->CastSpell(Kalec, SPELL_KALEC_TELEPORT, true);
            Kalec->SetLevitate(false);
            Kalec->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            Kalec->CastSpell(Kalec, SPELL_TELEPORT_VISUAL, true);
            // DoScriptText(SAY_KALECGOS_GOODBYE, Kalec);
            ((boss_kalecgos_kjAI*)Kalec->AI())->EnableRandomMovement = false;
            ((boss_kalecgos_kjAI*)Kalec->AI())->MoveTimer = 0;
        }
        Creature* Control = pInstance ? pInstance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER) : 0;
        if(Control)
        {
            if(Creature* CoreEnt = Control->SummonCreature(CREATURE_CORE_ENTROPIUS, me->GetPositionX(), me->GetPositionY(), 75, 0, TEMPSUMMON_CORPSE_DESPAWN, 0))
            {
                CoreEnt->CastSpell(CoreEnt, SPELL_ENTROPIUS_BODY, true);
                CoreEnt->SetLevitate(true);
            }
            Control->SummonCreature(CREATURE_BOSS_PORTAL, aOutroLocations[0].x, aOutroLocations[0].y, aOutroLocations[0].z, aOutroLocations[0].o, TEMPSUMMON_CORPSE_DESPAWN, 0);
        }
    }

    void JustSummoned(Creature* summoned)
    {
        if(summoned->GetEntry() == CREATURE_KALECGOS)
        {
            DoScriptText(SAY_KALECGOS_JOIN, summoned);
            summoned->CastSpell(summoned, SPELL_ARCANE_BOLT, true);
            ((boss_kalecgos_kjAI*)summoned->AI())->EnableRandomMovement = true;
            ((boss_kalecgos_kjAI*)summoned->AI())->MoveTimer = 9000;
        }
        else if(summoned->GetEntry() == CREATURE_ARMAGEDDON_TARGET)
        {
            summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            summoned->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        }      
        else
        {
            summoned->SetLevel(me->GetLevel());
        }
        summoned->setFaction(me->getFaction());
        Summons.Summon(summoned);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_KJ_DEATH, me);

        if(pInstance)
        {
            pInstance->SetData(DATA_KILJAEDEN_EVENT, DONE);
            Creature* Control = pInstance->instance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER);
            if (Control)
                Control->Kill(Control);
        }
        Summons.DespawnAllExcept(CREATURE_KALECGOS);

        // epilogue part
        DoEpilogue();
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KJ_SLAY1, SAY_KJ_SLAY2), me);
    }

    void EnterEvadeMode()
    {
        Scripted_NoMovementAI::EnterEvadeMode();
        Summons.DespawnAll();
       
        if(pInstance)
        {
            // Reset the Orbs of Blue Flight
            Creature* Kalec = pInstance ? pInstance->GetCreatureById(CREATURE_KALECGOS) : NULL;

            if(Kalec)
                CAST_AI(boss_kalecgos_kjAI, Kalec->AI())->ResetOrbs();
            else
                sLog.outLog(LOG_DEFAULT, "Kil'Jaeden/line 757: Kalecgos not found.", Phase);

            // Reset the controller - it will despawn KJ
            Creature* Control = pInstance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER);
            if (Control)
                ((Scripted_NoMovementAI*)Control->AI())->EnterEvadeMode();
        }
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();
    }

    Unit* SelectNonShieldedRandom()
    {
        return SelectUnit(SELECT_TARGET_RANDOM, 0,
            [this](Unit* u) {return u->GetTypeId() == TYPEID_PLAYER && !u->HasAura(SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT, 0) &&
            me->IsWithinDist(u, 100, false); });
    }

    /* selects one random target, not affected by shield of the dragonflight,
    and summons 4 reflections from different sides*/
    void CastSinisterReflection()
    {
        DoScriptText(RAND(SAY_KJ_REFLECTION1, SAY_KJ_REFLECTION2), me);

        Unit* target = SelectNonShieldedRandom();
        if (!target)
            return;

        float x, y, z;
        target->GetPosition(x, y, z);

        for (uint8 i = 0; i < 4; ++i)
        {
            float tempx = x - sinf(M_PI / 2 * i);
            float tempy = y - cosf(M_PI / 2 * i);
            if (!target->IsWithinLOS(tempx, tempy, z))
            {
                tempx = x;
                tempy = y;
            }

            Creature* SinisterReflection = me->SummonCreature(CREATURE_SINISTER_REFLECTION, tempx, tempy, z, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
            if (SinisterReflection)
                SinisterReflection->AI()->AttackStart(target);
        }
    }

    bool DoPreEvent(const uint32 diff)
    {
        if (KnockBackTimer)
        {
            if (KnockBackTimer <= diff)
            {
                std::list<Player*> playerList = FindAllPlayersInRange(15);
                if (!playerList.empty())
                {
                    for (std::list<Player*>::iterator i = playerList.begin(); i != playerList.end(); i++)
                    {
                        (*i)->KnockBackFrom(me, 15, 10);
                        me->DealDamage((*i), 475, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE);
                        me->SendSpellNonMeleeDamageLog((*i), SPELL_SUNWELL_KNOCKBACK, 475, SPELL_SCHOOL_MASK_FIRE, 0, 0, false, 0);
                    }
                }
                KnockBackTimer = 0;
            }
            else KnockBackTimer -= diff;
        }

        if (ComeFromFireTimer)
        {
            if (ComeFromFireTimer <= diff)
            {
                DoScriptText(SAY_KJ_EMERGE, me);
                AggroTimer = 23000;
                ComeFromFireTimer = 0;
            }
            else
                ComeFromFireTimer -= diff;
            return true;
        }

        if (AggroTimer)
        {
            if (AggroTimer <= diff)
            {
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat();
                AggroTimer = 0;
            }
            else
                AggroTimer -= diff;
            return true;
        }
        return false;
    }

    /* stops timers with timer_num more than 'timer_num' from updating for 'duration'*/
    void DoWaitFor(KilJaedenTimers timer_num, uint32 duration)
    {
        waitForTime = duration;
        activeTimers = timer_num+1; // the given timer still gets updated
    }

    void UpdateWaitTimer(const uint32 diff)
    {
        if (waitForTime)
        {
            if (waitForTime <= diff)
            {
                waitForTime = 0;
                activeTimers = TIMER_KJ_MAX;
            }
            else
                waitForTime -= diff;
        }
    }

    void DoSummonKalecIfNeeded()
    {
        if (!IsKalecJoined)
        {
            me->SummonCreature(CREATURE_KALECGOS, 1734.431, 593.1974, 68.1, 4.55, TEMPSUMMON_MANUAL_DESPAWN, 0);
            _Timer[P_2_TIMER_KALEC_JOIN] = 0;
            IsKalecJoined = true;
        }
    }

    // Needs to be called before phase change.
    void DoEmpowerOrbsIfNeeded()
    {
        if (OrbActivated) // was already activated
            return;

        if (pInstance)
        {
            if (Creature* pKalec = pInstance->GetCreatureById(CREATURE_KALECGOS))
                CAST_AI(boss_kalecgos_kjAI, pKalec->AI())->EmpowerOrb(Phase == PHASE_SACRIFICE);
            else
                sLog.outLog(LOG_DEFAULT, "Kil'Jaeden/line 922: Kalecgos not found. Phase: %u", Phase);
            OrbActivated = true;
            _Timer[P_3_TIMER_ORBS_EMPOWER] = 0;
        }
        else
            sLog.outLog(LOG_DEFAULT, "Kil'Jaeden/line 926: Instance not found. Phase: %u", Phase);
    }

    // deactivate all timers for 6 sec (wiki says ~5 sec)
    void DoStun()
    {
        ClearCastQueue();
        me->InterruptNonMeleeSpells(true);
        if (IsInDarkness) // need to reset to pre-cast-start state
            IsInDarkness = false;
        DoWaitFor(P_3_TIMER_ORBS_EMPOWER, 6000); // deactivating only negative timers
    }

    void DoAction(const int32 action)
    {
        if (action == 1) // do text for Darkness of a Thousand Souls trigger
        {
            IsInDarkness = false;
            DoScriptText(RAND(SAY_KJ_DARKNESS1, SAY_KJ_DARKNESS2, SAY_KJ_DARKNESS3), m_creature, m_creature->GetVictim());
            if (Phase != PHASE_SACRIFICE)
                _Timer[P_2_TIMER_SUMMON_SHILEDORB].Reset(5000); // don't know why it's here, but i'll leave it :)
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Phase < PHASE_NORMAL)
            return;

        if (DoPreEvent(diff))
            return;

        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_PULSE_COMBAT);

        UpdateWaitTimer(diff); // sets activeTimers back to normal when cast has end

        for (uint8 t = 0; t < activeTimers; ++t)
        {
            if (_Timer[t].Expired(diff))
            {
                switch(t)
                {   
                    case P_2_TIMER_KALEC_JOIN:
                    {
                        DoSummonKalecIfNeeded();
                        break;
                    }
                    case P_3_TIMER_ORBS_EMPOWER:
                    {
                        DoEmpowerOrbsIfNeeded();
                        break;
                    }
                    case P_3_TIMER_DARKNESS:
                    {
                        ClearCastQueue();
                        // Begins to channel for 8 seconds, then deals 50'000 damage to all raid members.
                        if (!IsInDarkness)
                        {
                            me->InterruptNonMeleeSpells(true);
                            ForceSpellCastWithScriptText(SPELL_DARKNESS_OF_A_THOUSAND_SOULS, CAST_NULL, EMOTE_KJ_DARKNESS);
                            DoWaitFor(P_3_TIMER_DARKNESS, 9500); // deactivate all timers for 9.5 sec. 8 sec is duration and 0.75 is cast time + 0.75 waiting
                            _Timer[P_3_TIMER_DARKNESS] = ((Phase == PHASE_SACRIFICE) ? urand(20000, 30000) : 45000) + 8750/*cast_time + duration*/;
                            IsInDarkness = true;
                        }
                        break;
                    }
                    case P_2_TIMER_SUMMON_SHILEDORB:
                    {
                        for (uint8 i = 1; i < Phase; ++i)
                        {
                            float sx, sy;
                            sx = ShieldOrbLocations[0][0] + ShieldOrbLocations[i][1] * sin(ShieldOrbLocations[i][0]);
                            sy = ShieldOrbLocations[0][1] + ShieldOrbLocations[i][1] * cos(ShieldOrbLocations[i][0]);
                            Creature* shieldorb = m_creature->SummonCreature(CREATURE_SHIELD_ORB, sx, sy, SHIELD_ORB_Z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 45000);
                            if (shieldorb)
                                shieldorb->AI()->DoAction(i);
                        }
                        _Timer[P_2_TIMER_SUMMON_SHILEDORB] = urand(30000, 60000); // 30-60seconds cooldown, or 5 sec after darkness is done
                        break;
                    }
                    case P_3_TIMER_SHADOW_SPIKE:
                    {
                        ClearCastQueue();

                        // will break anything but Darkness, cause when darkness is being cast this timer is not updated
                        me->InterruptNonMeleeSpells(true);

                        ForceSpellCast(SPELL_SHADOW_SPIKE, CAST_NULL);
                        DoWaitFor(P_3_TIMER_SHADOW_SPIKE, 29500); // 28 sec duration, 0.75 cast time + 0.75 wait
                        _Timer[P_3_TIMER_SHADOW_SPIKE] = 0;
                        break;
                    }
                    case P_4_TIMER_ARMAGEDDON:
                    {
                        Unit* target = SelectNonShieldedRandom();
                        if (target)
                        {
                            float x, y, z;
                            target->GetPosition(x, y, z);
                            me->SummonCreature(CREATURE_ARMAGEDDON_TARGET, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
                        }
                        _Timer[P_4_TIMER_ARMAGEDDON] = 2500;
                        break;
                    }
                    case P_2_TIMER_LEGION_LIGHTNING:
                    {
                        Unit* randomPlayer = SelectNonShieldedRandom();
                        if (randomPlayer)
                            AddSpellToCast(randomPlayer, SPELL_LEGION_LIGHTNING);
                        else
                            error_log("try to cast SPELL_LEGION_LIGHTNING on invalid target");
                        _Timer[P_2_TIMER_LEGION_LIGHTNING] = (Phase == PHASE_SACRIFICE) ? 18000 : 30000; // 18 seconds in PHASE_SACRIFICE
                        break;
                    }
                    case P_2_TIMER_FIRE_BLOOM:
                    {
                        AddSpellToCast(SPELL_FIRE_BLOOM, CAST_NULL);
                        _Timer[P_2_TIMER_FIRE_BLOOM] = (Phase == PHASE_SACRIFICE) ? 25000 : 40000; // 25 seconds in PHASE_SACRIFICE
                        break;
                    }
                    case P_3_TIMER_FLAME_DART:
                    {
                        AddSpellToCast(SPELL_FLAME_DART, CAST_NULL);
                        _Timer[P_3_TIMER_FLAME_DART] = urand(18000, 22000);
                        break;
                    }                    
                }
            }
        }

        if (me->GetHealthPercent() < 95) // Spawn Kalecgos at 94% if players are faster than timer
            DoSummonKalecIfNeeded();

        CastNextSpellIfAnyAndReady();

        //Phase 3
        if (Phase == PHASE_NORMAL && !IsInDarkness && (me->GetHealthPercent() < 85))
        {
            DoEmpowerOrbsIfNeeded(); // The first orb.
            // nothing should be cast before shadow spike ends
            ClearCastQueue();

            // First priority
            _Timer[P_3_TIMER_ORBS_EMPOWER].Reset(35000); // activating timer. The second orb timer
            _Timer[P_3_TIMER_DARKNESS].Reset(45000); // activating this timer

            // Second priority - does not get updated when Darkness is being casted
            //_Timer[P_2_TIMER_SUMMON_SHILEDORB].Reset(XXX); // No need to reset
            _Timer[P_3_TIMER_SHADOW_SPIKE].Reset(4000); // activating this timer
            DoWaitFor(P_3_TIMER_SHADOW_SPIKE, 5000); // not to cast any other spells during that time

            // Third priority - Does not get updated when Darkness or Shadow Spike is being casted
            //_Timer[P_2_TIMER_LEGION_LIGHTNING].Reset(XXX); // no need to reset
            //_Timer[P_2_TIMER_FIRE_BLOOM].Reset(XXX); // no need to reset
            _Timer[P_3_TIMER_FLAME_DART].Reset(urand(18000, 22000)); // activating timer
            
            CastSinisterReflection();

            DoScriptText(SAY_KJ_PHASE3, me);
            Phase = PHASE_DARKNESS;
            OrbActivated = false;
        }

        //Phase 4
        if (Phase == PHASE_DARKNESS && !IsInDarkness && (me->GetHealthPercent() < 55))
        {
            // second orb if too fast
            DoEmpowerOrbsIfNeeded(); // if players are faster than timers and we didn't activate orb in previous phase

            ClearCastQueue();

            // First priority
            _Timer[P_3_TIMER_ORBS_EMPOWER].Reset(35000); // activating timer
            _Timer[P_3_TIMER_DARKNESS].Reset(45000); // activating this timer

            // Second priority - does not get updated when Darkness is being casted
            //_Timer[P_2_TIMER_SUMMON_SHILEDORB].Reset(XXX); // No need to reset
            _Timer[P_3_TIMER_SHADOW_SPIKE].Reset(4000); // activating this timer
            DoWaitFor(P_3_TIMER_SHADOW_SPIKE, 5000); // not to cast any other spells during that time

            // Third priority - Does not get updated when Darkness or Shadow Spike is being casted
            _Timer[P_4_TIMER_ARMAGEDDON].Reset(4000); // activate timer, only 4 sec after shadow spikes
            //_Timer[P_2_TIMER_LEGION_LIGHTNING].Reset(XXX); // no need to reset
            //_Timer[P_2_TIMER_FIRE_BLOOM].Reset(XXX); // no need to reset
            // _Timer[P_3_TIMER_FLAME_DART].Reset(XXX); // no need to reset

            CastSinisterReflection();

            DoScriptText(SAY_KJ_PHASE4, me);
            Phase = PHASE_ARMAGEDDON;
            OrbActivated = false;
        }

        //Phase 5 specific spells all we can
        if (Phase == PHASE_ARMAGEDDON && !IsInDarkness && (me->GetHealthPercent() < 25))
        {
            // third orb if too fast
            DoEmpowerOrbsIfNeeded(); // if players are faster than timers and we didn't activate orb in previous phase

            ClearCastQueue();

            // First priority
            _Timer[P_3_TIMER_ORBS_EMPOWER].Reset(35000); // activating timer
            _Timer[P_3_TIMER_DARKNESS].Reset(45000); // activating this timer

            // Second priority - does not get updated when Darkness is being casted
            _Timer[P_2_TIMER_SUMMON_SHILEDORB].Reset(0); // deactivating timer
            _Timer[P_3_TIMER_SHADOW_SPIKE].Reset(4000); // activating this timer
            DoWaitFor(P_3_TIMER_SHADOW_SPIKE, 5000); // not to cast any other spells during that time

            // Third priority - Does not get updated when Darkness or Shadow Spike is being casted
            _Timer[P_4_TIMER_ARMAGEDDON].Reset(0); // deactivate timer
            //_Timer[P_2_TIMER_LEGION_LIGHTNING].Reset(XXX); // no need to reset
            //_Timer[P_2_TIMER_FIRE_BLOOM].Reset(XXX); // no need to reset
            // _Timer[P_3_TIMER_FLAME_DART].Reset(XXX); // no need to reset

            CastSinisterReflection();

            Phase = PHASE_SACRIFICE;
            if (Creature* pAnveena = pInstance->GetCreatureById(CREATURE_ANVEENA))
            {
                pAnveena->CastSpell(me, SPELL_SACRIFICE_OF_ANVEENA, false);
                //pAnveena->SetVisibility(VISIBILITY_OFF);
            }
            OrbActivated = false;
        }

        if (!waitForTime && SoulFlayCheck.Expired(diff))
        {
            // if no spells are being casted at the moment
            if (!me->IsNonMeleeSpellCast(false))
            {
                Unit* target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0,
                    [this](Unit* u) {return u->GetTypeId() == TYPEID_PLAYER && !u->IsImmunedToDamage(SPELL_SCHOOL_MASK_SHADOW); });
                if (target)
                    DoCast(target, SPELL_SOUL_FLAY);
                // AddSpellToCast(SPELL_SOUL_FLAY_SLOW, CAST_TANK); // Already casted by the spell above.
            }
            SoulFlayCheck = 750;
        }
    }
};

CreatureAI* GetAI_boss_kiljaeden(Creature *_Creature)
{
    return new boss_kiljaedenAI (_Creature);
}

//AI for Kil'jaeden Event Controller
struct mob_kiljaeden_controllerAI : public Scripted_NoMovementAI
{
    mob_kiljaeden_controllerAI(Creature* c) : Scripted_NoMovementAI(c), Summons(me)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    SummonList Summons;

    Timer RandomSayTimer;
    uint8 DeceiverDeathCount; // when reaches 3 - we summon KJ and set it to 0

    void Reset()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->addUnitState(UNIT_STAT_STUNNED);

        pInstance->SetData(DATA_KILJAEDEN_EVENT, NOT_STARTED);
        Summons.DespawnAll();
        DeceiverDeathCount = 0;

        // spawn new deceivers
        for (uint8 i = 0; i < 3; ++i)
        {
            me->SummonCreature(CREATURE_HAND_OF_THE_DECEIVER, DeceiverLocations[i][0], DeceiverLocations[i][1], FLOOR_Z, DeceiverLocations[i][2], TEMPSUMMON_DEAD_DESPAWN, 0);
        }

        if (Creature* KalecKJ = pInstance->GetCreatureById(CREATURE_KALECGOS))
            ((boss_kalecgos_kjAI*)KalecKJ->AI())->ResetOrbs();

        // spawn new Anveena
        //DoSpawnCreature(CREATURE_ANVEENA, 0, 0, 40, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
        me->SummonCreature(CREATURE_ANVEENA, 1699.029, 628.079, 67, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
        DoCast(me, SPELL_ANVEENA_ENERGY_DRAIN);

        RandomSayTimer = 30000;

        me->SetReactState(REACT_AGGRESSIVE);
        DoCastAOE(SPELL_KILL_DRAKES);
    }

    void EnterCombat(Unit* who)
    {
        pInstance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);
        DoZoneInCombat(1000.0f);
    }

    void JustSummoned(Creature* summoned)
    {
        switch(summoned->GetEntry())
        {
            case CREATURE_ANVEENA:
            {
                summoned->SetLevitate(true);
                summoned->CastSpell(summoned, SPELL_ANVEENA_PRISON, true);
                summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                break;
            }
            case CREATURE_KILJAEDEN:
                summoned->CastSpell(summoned, SPELL_REBIRTH, false);
                ((boss_kiljaedenAI*)summoned->AI())->Phase = PHASE_NORMAL;
                summoned->AddThreat(me->GetVictim(), 1.0f);
                summoned->AI()->DoZoneInCombat();
                break;
        }
        Summons.Summon(summoned);
    }

    void DoRandomSay(uint32 diff)
    {    // do not yell when any encounter in progress
        if (pInstance->IsEncounterInProgress() || pInstance->GetData(DATA_MURU_EVENT) == DONE)
            return;

        if (RandomSayTimer.Expired(diff))
        {
            DoScriptText(RAND(SAY_KJ_OFFCOMBAT1, SAY_KJ_OFFCOMBAT2, SAY_KJ_OFFCOMBAT3, SAY_KJ_OFFCOMBAT4, SAY_KJ_OFFCOMBAT5), me);
            RandomSayTimer = 60000;
        }
    }

    void DoAction(const int32 action)
    {
        if (action == 1)
        {
            ++DeceiverDeathCount;
            if (DeceiverDeathCount > 2)
            {
                if (pInstance->GetData(DATA_MURU_EVENT) == DONE)
                {
                    me->RemoveAurasDueToSpell(SPELL_ANVEENA_ENERGY_DRAIN);
                    DeceiverDeathCount = 0;
                    Creature* KjImmunity = DoSpawnCreature(CREATURE_KILJAEDEN, 0, 0, 0, 3.851238, TEMPSUMMON_MANUAL_DESPAWN, 0);
                    if (KjImmunity)
                        KjImmunity->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        DoRandomSay(diff);

        if (me->IsInCombat())
        {
            DoSpecialThings(diff, DO_PULSE_COMBAT);
            if (me->getThreatManager().isThreatListEmpty())
            {
                EnterEvadeMode(); // we use this instead of UpdateVictim()
            }
        }
    }
};

CreatureAI* GetAI_mob_kiljaeden_controller(Creature *_Creature)
{
    return new mob_kiljaeden_controllerAI (_Creature);
}

//AI for Hand of the Deceiver
struct mob_hand_of_the_deceiverAI : public ScriptedAI
{
    mob_hand_of_the_deceiverAI(Creature* c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer ShadowBoltVolleyTimer;
    Timer FelfirePortalTimer;
    SummonList Summons;

    void Reset()
    {
        if (!me->IsInEvadeMode())
            DoCast(me, SPELL_SHADOW_CHANNELING);

        ShadowBoltVolleyTimer.Reset(1000 + urand(0, 3000)); // So they don't all cast it in the same moment.
        FelfirePortalTimer.Reset(20000);
        Summons.DespawnAll();
    }

    void JustReachedHome()
    {
        DoCast(me, SPELL_SHADOW_CHANNELING);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->setFaction(me->getFaction());
        summoned->SetLevel(me->GetLevel());
        summoned->AI()->DoZoneInCombat();
        Summons.Summon(summoned);
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
        {
            Creature* Control = pInstance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER);
            if (Control)
                Control->AI()->EnterCombat(who);
        }
        me->InterruptNonMeleeSpells(true);
        DoZoneInCombat();
    }

    void JustDied(Unit* killer)
    {
        if(!pInstance)
            return;

        Creature* Control = pInstance->GetCreatureById(CREATURE_KILJAEDEN_CONTROLLER);
        if(Control)
            ((mob_kiljaeden_controllerAI*)Control->AI())->DoAction(1);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        // Gain Shadow Infusion at 20% health
        if(((me->GetHealthPercent()) < 20) && !me->HasAura(SPELL_SHADOW_INFUSION, 0))
            DoCast(me, SPELL_SHADOW_INFUSION, true);

        // Shadow Bolt Volley - Shoots Shadow Bolts at all enemies within 30 yards, for ~2k Shadow damage.
        if (ShadowBoltVolleyTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SHADOW_BOLT_VOLLEY);
            ShadowBoltVolleyTimer = urand(2500, 4500);
        }

        // Felfire Portal - Creatres a portal, that spawns Volatile Felfire Fiends, which do suicide bombing.
        if (FelfirePortalTimer.Expired(diff))
        {
            DoSpawnCreature(CREATURE_FELFIRE_PORTAL, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 20000);
            FelfirePortalTimer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hand_of_the_deceiver(Creature *_Creature)
{
    return new mob_hand_of_the_deceiverAI (_Creature);
}

//AI for Felfire Portal
struct mob_felfire_portalAI : public Scripted_NoMovementAI
{
    mob_felfire_portalAI(Creature* c) : Scripted_NoMovementAI(c) {}

    Timer SpawnFiendTimer;

    void InitializeAI()
    {
        SpawnFiendTimer = urand(3000, 6000);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
    }


    void JustSummoned(Creature* summoned)
    {
        summoned->setFaction(me->getFaction());
        summoned->SetLevel(me->GetLevel());
    }

    void UpdateAI(const uint32 diff)
    {
        if (SpawnFiendTimer.Expired(diff))
        {
            // fiends select target themself
            DoSpawnCreature(CREATURE_VOLATILE_FELFIRE_FIEND, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 20000);
            SpawnFiendTimer = urand(4000, 8000);
        }
    }
};

CreatureAI* GetAI_mob_felfire_portal(Creature *_Creature)
{
    return new mob_felfire_portalAI (_Creature);
}

//AI for Felfire Fiend
struct mob_volatile_felfire_fiendAI : public ScriptedAI
{
    mob_volatile_felfire_fiendAI(Creature* c) : ScriptedAI(c) {}

    Timer ExplodeTimer;
    Timer WaitTimer;

    void Reset()
    {
        WaitTimer.Reset(1500);
        ExplodeTimer.Reset(5000);
        me->SetRooted(true);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if(damage > me->GetHealth())
            DoCast(me, SPELL_FELFIRE_FISSION, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (WaitTimer.Expired(diff))
        {
            WaitTimer = 0;
            me->SetRooted(false);
            DoZoneInCombat(100.0f);
            Unit* random = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
            if (random)
            {
                AttackStart(random);
                me->AddThreat(me->GetVictim(), 10000000.0f);
            }
        }

        if (WaitTimer.GetTimeLeft())
            return;

        if(!UpdateVictim())
            return;

        if (ExplodeTimer.Expired(diff) || me->IsWithinDistInMap(me->GetVictim(), 3)) // Explode if it's close enough to it's target
        {
            DoCast(me->GetVictim(), SPELL_FELFIRE_FISSION);
            me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }
    }
};

CreatureAI* GetAI_mob_volatile_felfire_fiend(Creature *_Creature)
{
    return new mob_volatile_felfire_fiendAI (_Creature);
}

//AI for Armageddon target
struct mob_armageddonAI : public Scripted_NoMovementAI
{
    mob_armageddonAI(Creature* c) : Scripted_NoMovementAI(c) {}

    uint8 Spell;
    uint32 Timer;

    void InitializeAI(){
        Spell = 0;
        Timer = 0;
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        me->SetFloatValue(OBJECT_FIELD_SCALE_X, 4.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Timer <= diff)
        {
            switch(Spell)
            {
                case 0:
                    DoCast(me, SPELL_ARMAGEDDON_VISUAL, true);
                    ++Spell;
                    break;
                case 1:
                    DoCast(me, SPELL_ARMAGEDDON_VISUAL2, true);
                    Timer = 9000;
                    ++Spell;
                    break;
                case 2:
                    DoCast(me, SPELL_ARMAGEDDON_TRIGGER, true);
                    ++Spell;
                    Timer = 5000;
                    break;
                case 3:
                    me->DealDamage(me, me->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                    me->RemoveCorpse();
                    break;
            }
        }else Timer -=diff;
    }
};

CreatureAI* GetAI_mob_armageddon(Creature *_Creature)
{
    return new mob_armageddonAI (_Creature);
}

//AI for Shield Orbs
struct mob_shield_orbAI : public ScriptedAI
{
    mob_shield_orbAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    bool PointReached;
    bool Clockwise;
    Timer _Timer;
    ScriptedInstance* pInstance;
    float x, y, r, c, mx, my;

    void InitializeAI()
    {
        me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_SINISTER_REFLECTION, true);
        me->SetIgnoreVictimSelection(true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        me->SetSelection(0);
        me->SetLevitate(true);
        PointReached = true;
        _Timer.Reset(urand(500, 1000));
        mx = ShieldOrbLocations[0][0];
        my = ShieldOrbLocations[0][1];
        Clockwise = true;
    }

    void DoAction(const int32 act)
    {
        c = ShieldOrbLocations[act][0];
        r = ShieldOrbLocations[act][1];
        if (act == 1 || act == 3)
            Clockwise = false; // Wowwiki says that first orb goes counterclockwise, second clockwise, third - counterclock again.
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->GetSelection())
            me->SetSelection(0);
        if (PointReached)
        {
            y = my + r * cos(c);
            x = mx + r * sin(c);
            PointReached = false;
            m_creature->GetMotionMaster()->MovePoint(1, x, y, SHIELD_ORB_Z);
            if (Clockwise)
                c += M_PI / 50;
            else
                c -= M_PI / 50;

            if (c > 2 * M_PI)
                c -= 2 * M_PI;
            if (c < 0)
                c += 2 * M_PI;
        }



        if (_Timer.Expired(diff))
        {
            ForceSpellCast(SPELL_SHADOW_BOLT, CAST_RANDOM, INTERRUPT_AND_CAST_INSTANTLY, true);
            _Timer = urand(400,700); // Wowwiki says up to 3 per sec. We set it to 2
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        PointReached = true;
    }

};

CreatureAI* GetAI_mob_shield_orb(Creature *_Creature)
{
    return new mob_shield_orbAI (_Creature);
}

//AI for Sinister Reflection
struct mob_sinster_reflectionAI : public ScriptedAI
{
    mob_sinster_reflectionAI(Creature* c) : ScriptedAI(c) {}

    uint8 Class;
    Timer _Timer[3];
    Timer Wait;

    void Reset()
    {
        _Timer[0].Reset(1);
        _Timer[1].Reset(1);
        _Timer[2].Reset(1);
        Wait.Reset(5000);
        Class = 0;
        me->SetRooted(true);
        me->CastSpell(me, SPELL_SINISTER_REFLECTION_ENLARGE, true);
    }
    
    void EnterCombat(Unit* who)
    {
        if(who)
            who->CastSpell(me, SPELL_SINISTER_REFLECTION, true);
        DoZoneInCombat();
    }
    
    void SpellHit(Unit* pCaster, const SpellEntry *Spell)
    {
        if(Spell->Id == SPELL_SINISTER_REFLECTION)
            me->ApplySpellImmune(0, IMMUNITY_ID, SPELL_SINISTER_REFLECTION, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Wait.Expired(diff))
        {
            me->SetRooted(false);
            Wait = 0;
        }

        if (Wait.GetTimeLeft())
            return;

        if (!UpdateVictim())
            return;

        if (Class == 0)
        {
            Class = me->GetVictim()->GetClass();
            switch (Class)
            {
                case CLASS_DRUID:
                    break;
                case CLASS_HUNTER:
                    break;
                case CLASS_MAGE:
                    break;
                case CLASS_WARLOCK:
                    break;
                case CLASS_WARRIOR:
                    me->SetCanDualWield(true);
                    break;
                case CLASS_PALADIN:
                    break;
                case CLASS_PRIEST:
                    break;
                case CLASS_SHAMAN:
                    me->SetCanDualWield(true);
                    break;
                case CLASS_ROGUE:
                    me->SetCanDualWield(true);
                    break;
            }
        }

        switch (Class)
        {
            case CLASS_DRUID:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_MOONFIRE, false);
                    _Timer[1] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_HUNTER:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_MULTI_SHOT, false);
                    _Timer[1] = 9000;
                }
                if (_Timer[2].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_SHOOT, false);
                    _Timer[2] = 5000;
                }
                if (m_creature->IsWithinMeleeRange(m_creature->GetVictim(), 6))
                {
                    if (_Timer[0].Expired(diff))
                    {
                        DoCast(m_creature->GetVictim(), SPELL_SR_WING_CLIP, false);
                        _Timer[0] = 7000;
                    }
                    DoMeleeAttackIfReady();
                }
                break;
            case CLASS_MAGE:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_FIREBALL, false);
                    _Timer[1] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_WARLOCK:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_SHADOW_BOLT, false);
                    _Timer[1] = 4000;
                }
                if (_Timer[2].Expired(diff))
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                        DoCast(target, SPELL_SR_CURSE_OF_AGONY, true);
                    _Timer[2] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_WARRIOR:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_WHIRLWIND, false);
                    _Timer[1] = 10000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_PALADIN:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HAMMER_OF_JUSTICE, false);
                    _Timer[1] = 7000;
                }
                if (_Timer[2].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HOLY_SHOCK, false);
                    _Timer[2] = 3000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_PRIEST:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HOLY_SMITE, false);
                    _Timer[1] = 5000;
                }
                if (_Timer[2].Expired(diff) && (me->GetHealth() * 2 < me->GetMaxHealth()))
                {
                    DoCast(m_creature, SPELL_SR_RENEW, false);
                    _Timer[2] = 7000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_SHAMAN:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_EARTH_SHOCK, false);
                    _Timer[1] = 5000;
                }
                DoMeleeAttackIfReady();
                break;
            case CLASS_ROGUE:
                if (_Timer[1].Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_SR_HEMORRHAGE, true);
                    _Timer[1] = 5000;
                }
                DoMeleeAttackIfReady();
                break;
        }
    }
};

CreatureAI* GetAI_mob_sinster_reflection(Creature *_Creature)
{
    return new mob_sinster_reflectionAI (_Creature);
}

struct mob_26259AI : public ScriptedAI
{
    mob_26259AI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance* pInstance;

    void Reset()
    {
    }
    
    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        if (uiMotionType != POINT_MOTION_TYPE)
            return;
            
        if (uiPointId == 1)
        {
            if (Creature* SoldTarget = (Creature*)FindCreature(CREATURE_SOLDIER_TARGET, 15, me))
                me->SetFacingToObject(SoldTarget);
        }

        if (uiPointId == 2)
        {
            me->CastSpell(me, SPELL_TELEPORT_VISUAL, true);
            me->ForcedDespawn(9000);
            std::list<Creature*> rifters = FindAllCreaturesWithEntry(CREATURE_RIFTWALKER, 100);
            if (!rifters.empty())
            {
                for(std::list<Creature *>::iterator i = rifters.begin(); i != rifters.end(); i++)
                {
                    (*i)->CastSpell((*i), SPELL_TELEPORT_VISUAL, true);
                    (*i)->ForcedDespawn(11000);
                }
            }
            if(Creature* BossTp = (Creature*)FindCreature(CREATURE_BOSS_PORTAL, 100, me))
                BossTp->ForcedDespawn(11000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
    }
};

enum mob_24925_events
{
    EVENT_24925_1     = 1,
    EVENT_24925_2     = 2,
    EVENT_24925_3     = 3,
    EVENT_24925_4     = 4,
    EVENT_24925_5     = 5
};

//Boss Portal: Summon soldiers, Velen and Liadrin
struct mob_24925AI : public ScriptedAI
{
    mob_24925AI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    
    ScriptedInstance* pInstance;
    EventMap events;

    void Reset()
    {
        events.ScheduleEvent(EVENT_24925_1, 30000);
    }
    
    void HandleSoldiers()
    {
        std::list<Creature*> SoldierEntryList;
        std::list<Creature*> SoldierList;
        SoldierList.clear();
        for(uint8 i = 0; i < 20; ++i)
        {
            SoldierEntryList.clear();
            SoldierEntryList = FindAllCreaturesWithEntry(CREATURE_SOLDIER, 100.0f);
            for(std::list<Creature*>::iterator iter = SoldierEntryList.begin(); iter != SoldierEntryList.end(); ++iter)
                SoldierList.push_back(*iter);
        }
        if(!SoldierList.empty())
        {
            for(std::list<Creature*>::iterator i = SoldierList.begin(); i != SoldierList.end(); ++i)
            {
                (*i)->GetMotionMaster()->MovePoint(2, 1723.380249, 651.60083, 28.050344);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_24925_1:
                {
                    if(Creature* pKalecgos = (Creature*)FindCreature(CREATURE_KALECGOS, 100, me))
                       DoScriptText(SAY_KALECGOS_GOODBYE, pKalecgos);
                    events.ScheduleEvent(EVENT_24925_2, 15000);
                    break;
                }
                case EVENT_24925_2:
                {
                    if(Creature* SSR1 = me->SummonCreature(CREATURE_RIFTWALKER,  1697.21569, 634.927612, 27.732185, 1.093424, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    {
                        SSR1->CastSpell(SSR1, SPELL_TELEPORT_VISUAL, true);
                        SSR1->GetMotionMaster()->MovePoint(0, 1710.488403, 664.128174, 28.050381, 6.18847);
                    }
                    if(Creature* SSR2 = me->SummonCreature(CREATURE_RIFTWALKER, 1705.753662, 626.737549, 27.734455, 0.221632, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    {
                        SSR2->CastSpell(SSR2, SPELL_TELEPORT_VISUAL, true);
                        SSR2->GetMotionMaster()->MovePoint(0, 1736.46106, 633.28302, 28.05038, 1.38969);
                    }
                    me->SummonCreature(CREATURE_SOLDIER_TARGET, 1721.23, 604.741, 28.0502, 0.000822067, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    me->SummonCreature(CREATURE_SOLDIER_TARGET, 1677.26, 650.797, 28.0504, 2.27298, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    events.ScheduleEvent(EVENT_24925_3, 6000);
                    break;
                }
                case EVENT_24925_3:
                {
                    Creature* Summoned = NULL;
                    for(int i = 0; i<10; i++)
                    {
                        if(Summoned = me->SummonCreature(CREATURE_SOLDIER, SpawnPosition[i].x, SpawnPosition[i].y, SpawnPosition[i].z, SpawnPosition[i].o, TEMPSUMMON_CORPSE_DESPAWN, 0))
                        {
                            Summoned->CastSpell(Summoned, SPELL_TELEPORT_VISUAL, true);
                            Summoned->GetMotionMaster()->MovePoint(1, MovePositionRight[i].x, MovePositionRight[i].y, MovePositionRight[i].z, MovePositionRight[i].o);
                        }
                    }
                    events.ScheduleEvent(EVENT_24925_4, 5500);
                    break;
                }
                case EVENT_24925_4:
                {
                    Creature* Summoned = NULL;
                    for(int i = 0; i<10; i++)
                    {
                        if(Summoned = me->SummonCreature(CREATURE_SOLDIER, SpawnPosition[i].x, SpawnPosition[i].y, SpawnPosition[i].z, SpawnPosition[i].o, TEMPSUMMON_CORPSE_DESPAWN, 0))
                        {
                            Summoned->CastSpell(Summoned, SPELL_TELEPORT_VISUAL, true);
                            Summoned->GetMotionMaster()->MovePoint(1, MovePositionLeft[i].x, MovePositionLeft[i].y, MovePositionLeft[i].z, MovePositionLeft[i].o);
                        }
                    }
                    events.ScheduleEvent(EVENT_24925_5, 4000);
                    break;
                }
                case EVENT_24925_5:
                {
                    if(Creature* Velen = me->SummonCreature(CREATURE_VELEN, aOutroLocations[1].x, aOutroLocations[1].y, aOutroLocations[1].z, aOutroLocations[1].o, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    {
                        Velen->SetSpeed(MOVE_RUN, 0.3, true);
                        Velen->SetSpeed(MOVE_WALK, 0.3, true);
                        Velen->SetWalk(true);
                        Velen->GetMotionMaster()->MovePoint(0, 1710.798706, 639.678223, 27.45088, 3.898754);
                    }
                    me->SummonCreature(CREATURE_LIADRIN, aOutroLocations[2].x, aOutroLocations[2].y, aOutroLocations[2].z, aOutroLocations[2].o, TEMPSUMMON_TIMED_DESPAWN, 4 * MINUTE * 1000);
                    break;
                }
            }
        }
    }
};

CreatureAI* GetAI_mob_24925(Creature *_Creature)
{
    return new mob_24925AI (_Creature);
}

struct mob_26289AI : public ScriptedAI
{
    mob_26289AI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance* pInstance;

    bool Check;
    uint32 CheckTimer;

    void Reset()
    {
        me->CastSpell(me, SPELL_TELEPORT_VISUAL, true);
        CheckTimer = 5000;
        Check = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Check)
        {
            if (CheckTimer < diff)
            {
                if(Creature* BossPortal = (Creature*)FindCreature(CREATURE_BOSS_PORTAL, 20.0, me))
                {
                    me->SetFacingToObject(BossPortal);
                    BossPortal->CastSpell(BossPortal, SPELL_SHADOW_PORTAL_STATE, true);
                    me->CastSpell(BossPortal, SPELL_PORTAL_OPENING, true);
                }
                Check = true;
            }
            else
                CheckTimer -= diff;
        }
    }
};

CreatureAI* GetAI_mob_26289(Creature *_Creature)
{
    return new mob_26289AI (_Creature);
}

CreatureAI* GetAI_mob_26259(Creature *_Creature)
{
    return new mob_26259AI (_Creature);
}

enum mob_26246_events
{
    EVENT_26246_1     = 1,
    EVENT_26246_2     = 2,
    EVENT_26246_3     = 3,
    EVENT_26246_4     = 4,
    EVENT_26246_5     = 5,
    EVENT_26246_6     = 6,
    EVENT_26246_7     = 7,
    EVENT_26246_8     = 8,
    EVENT_26246_9     = 9,
    EVENT_26246_10    = 10,
    EVENT_26246_11    = 11,
    EVENT_26246_12    = 12,
    EVENT_26246_13    = 13,
    EVENT_26246_14    = 14,
    EVENT_26246_15    = 15
};

struct mob_26246AI : public ScriptedAI
{
    mob_26246AI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance* pInstance;

    EventMap events;

    void Reset()
    {
        events.ScheduleEvent(EVENT_26246_1, 10500);
        me->SetSpeed(MOVE_RUN, 1.3);
        me->SetSpeed(MOVE_WALK, 1.3);
    }
    
    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        if (uiMotionType != POINT_MOTION_TYPE)
            return;

        if (uiPointId == 1)
        {
            me->CastSpell(me, SPELL_TELEPORT_VISUAL, true);
            me->ForcedDespawn(1000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_26246_1:
                {
                    DoScriptText(SAY_OUTRO_1, me);
                    events.ScheduleEvent(EVENT_26246_2, 24500);
                    break;
                }
                case EVENT_26246_2:
                {
                    DoScriptText(SAY_OUTRO_2, me);
                    events.ScheduleEvent(EVENT_26246_3, 13500);
                    break;
                }
                case EVENT_26246_3:
                {
                    DoScriptText(SAY_OUTRO_3, me);
                    events.ScheduleEvent(EVENT_26246_4, 10000);
                    break;
                }
                case EVENT_26246_4:
                {
                    if (Creature* pEntropius = (Creature*)FindCreature(CREATURE_CORE_ENTROPIUS, 100, me))
                    {
                        pEntropius->SetWalk(false);
                        pEntropius->GetMotionMaster()->MovePoint(1, 1698.691162, 628.793762, 30.0f);
                        DoCast(pEntropius, SPELL_CALL_ENTROPIUS);
                    }
                    events.ScheduleEvent(EVENT_26246_5, 7500);
                    break;
                }
                case EVENT_26246_5:
                {
                    me->InterruptNonMeleeSpells(false);
                    DoScriptText(SAY_OUTRO_4, me);
                    events.ScheduleEvent(EVENT_26246_6, 22000);
                    break;
                }
                case EVENT_26246_6:
                {
                    if (Creature* pLiadrin = (Creature*)FindCreature(CREATURE_LIADRIN, 30, me))
                    {
                        pLiadrin->GetMotionMaster()->MovePoint(0, 1712.408813, 634.790039, 28, 3.5);
                        DoScriptText(SAY_OUTRO_5, pLiadrin);
                    }
                    events.ScheduleEvent(EVENT_26246_7, 9500);
                    break;
                }
                case EVENT_26246_7:
                {
                    DoScriptText(SAY_OUTRO_6, me);
                    events.ScheduleEvent(EVENT_26246_8, 14500);
                    break;
                }
                case EVENT_26246_8:
                {
                    if (Creature* pLiadrin = (Creature*)FindCreature(CREATURE_LIADRIN, 30, me))
                        DoScriptText(SAY_OUTRO_7, pLiadrin);
                    events.ScheduleEvent(EVENT_26246_9, 2000);
                    break;
                }
                case EVENT_26246_9:
                {
                    DoScriptText(SAY_OUTRO_8, me);
                    events.ScheduleEvent(EVENT_26246_10, 16000);
                    break;
                }
                case EVENT_26246_10:
                {
                    if (Creature* pEntropius = (Creature*)FindCreature(CREATURE_CORE_ENTROPIUS, 50, me))
                    {
                        pEntropius->CastSpell(pEntropius, SPELL_BLAZE_TO_LIGHT, true);
                        pEntropius->RemoveAurasDueToSpell(SPELL_ENTROPIUS_BODY);
                        pEntropius->SetWalk(true);
                        pEntropius->GetMotionMaster()->MovePoint(2, 1698.691162, 628.793762, 28);
                    }
                    events.ScheduleEvent(EVENT_26246_11, 3000);
                    break;
                }
                case EVENT_26246_11:
                {
                    DoScriptText(SAY_OUTRO_9, me);
                    if (Creature* pEntropius = (Creature*)FindCreature(CREATURE_CORE_ENTROPIUS, 50, me))
                        pEntropius->CastSpell(pEntropius, SPELL_SUNWELL_IGNITION, true);
                    events.ScheduleEvent(EVENT_26246_12, 16000);
                    break;
                }
                case EVENT_26246_12:
                {
                    if (Creature* pLiadrin = (Creature*)FindCreature(CREATURE_LIADRIN, 30, me))
                    {
                        DoScriptText(SAY_OUTRO_10, pLiadrin);
                        pLiadrin->SetStandState(UNIT_STAND_STATE_KNEEL);
                    }
                    events.ScheduleEvent(EVENT_26246_13, 15000);
                    break;
                }
                case EVENT_26246_13:
                {
                    DoScriptText(SAY_OUTRO_11, me);
                    events.ScheduleEvent(EVENT_26246_14, 6000);
                    break;
                }
                case EVENT_26246_14:
                {
                    DoScriptText(SAY_OUTRO_12, me);
                    me->SetSpeed(MOVE_RUN, 1, true);
                    me->SetSpeed(MOVE_WALK, 1, true);
                    me->GetMotionMaster()->MovePoint(1, aOutroLocations[1].x, aOutroLocations[1].y, aOutroLocations[1].z);
                    if (Creature* pPortal = (Creature*)FindCreature(CREATURE_BOSS_PORTAL, 100, me))
                    CAST_AI(mob_24925AI, pPortal->AI())->HandleSoldiers();
                    break;
                }
            }
        }
    }
};

CreatureAI* GetAI_mob_26246(Creature *_Creature)
{
    return new mob_26246AI (_Creature);
}

struct npc_anveenaAI : public ScriptedAI
{
    npc_anveenaAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }
    ScriptedInstance* pInstance;

    bool Check[10];
    uint32 CheckTimer[9];
    Timer CrashPreventionTimer;

    void Reset()
    {
        CrashPreventionTimer.Reset(0);
        Check[0] = true;
        Check[1] = true;
        Check[2] = true;
        Check[3] = true;
        Check[4] = true;
        Check[5] = true;
        Check[6] = true;
        Check[7] = true;
        Check[8] = true;
        Check[9] = true;
        Check[10] = true;
        CheckTimer[0] = 0;
        CheckTimer[1] = 0;
        CheckTimer[2] = 0;
        CheckTimer[3] = 0;
        CheckTimer[4] = 0;
        CheckTimer[5] = 0;
        CheckTimer[6] = 0;
        CheckTimer[7] = 0;
        CheckTimer[8] = 0;
        CheckTimer[9] = 0;
        
    }

    void SpellHitTarget(Unit *target, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_SACRIFICE_OF_ANVEENA)
        {
            if(Check[10])
            {
                target->RemoveAurasDueToSpell(SPELL_SACRIFICE_OF_ANVEENA);
                CheckTimer[0] = 1000;
                Check[0] = false;
                Check[10] = false;
                CrashPreventionTimer = 1000;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(CrashPreventionTimer.Expired(diff))
        {
            if(!Check[0])
            {
                if (CheckTimer[0] < diff)
                {
                    if(Creature* pKalecgos = (Creature*)FindCreature(CREATURE_KALECGOS, 100, me))
                        DoScriptText(SAY_KALECGOS_AWAKEN, pKalecgos);
                    CheckTimer[1] = 4000;
                    Check[0] = true;
                    Check[1] = false;
                }
                else
                    CheckTimer[0] -= diff;
            }
            if(!Check[1])
            {
                if(CheckTimer[1] < diff)
                {
                    DoScriptText(SAY_ANVEENA_IMPRISONED, me);
                    CheckTimer[2] = 2000;
                    Check[1] = true;
                    Check[2] = false;
                }
                else
                    CheckTimer[1] -= diff;
            }
            if(!Check[2])
            {
                if (CheckTimer[2] < diff)
                {
                    if(Creature* pKalecgos = (Creature*)FindCreature(CREATURE_KALECGOS, 100, me))
                        DoScriptText(SAY_KALECGOS_LETGO, pKalecgos);
                    CheckTimer[3] = 6000;
                    Check[2] = true;
                    Check[3] = false;
                }
                else
                    CheckTimer[2] -= diff;
            }
            if(!Check[3])
            {
                if (CheckTimer[3] < diff)
                {
                    DoScriptText(SAY_ANVEENA_LOST, me);
                    CheckTimer[4] = 3000;
                    Check[3] = true;
                    Check[4] = false;
                }
                else
                    CheckTimer[3] -= diff;
            }
            if(!Check[4])
            {
                if (CheckTimer[4] < diff)
                {
                    if(Creature* pKalecgos = (Creature*)FindCreature(CREATURE_KALECGOS, 100, me))
                        DoScriptText(SAY_KALECGOS_FOCUS, pKalecgos);
                    CheckTimer[5] = 6000;
                    Check[4] = true;
                    Check[5] = false;
                }
                else
                    CheckTimer[4] -= diff;
            }
            if(!Check[5])
            {
                if (CheckTimer[5] < diff)
                {
                    DoScriptText(SAY_ANVEENA_KALEC, me);
                    CheckTimer[6] = 2000;
                    Check[5] = true;
                    Check[6] = false;
                }
                else
                    CheckTimer[5] -= diff;
            }
            if(!Check[6])
            {
                if (CheckTimer[6] < diff)
                {
                    if(Creature* pKalecgos = (Creature*)FindCreature(CREATURE_KALECGOS, 100, me))
                        DoScriptText(SAY_KALECGOS_FATE, pKalecgos);
                    CheckTimer[7] = 4000;
                    Check[6] = true;
                    Check[7] = false;
                }
                else
                    CheckTimer[6] -= diff;
            }
            if(!Check[7])
            {
                if (CheckTimer[7] < diff)
                {
                    DoScriptText(SAY_ANVEENA_GOODBYE, me);
                    CheckTimer[8] = 6000;
                    Check[7] = true;
                    Check[8] = false;
                }
                else
                    CheckTimer[7] -= diff;
            }
            if(!Check[8])
            {
                if (CheckTimer[8] < diff)
                {
                    if(Creature* pKalecgos = (Creature*)FindCreature(CREATURE_KALECGOS, 100, me))
                        DoScriptText(SAY_KALECGOS_GOODBYE, pKalecgos);
                    CheckTimer[9] = 8000;
                    Check[8] = true;
                    Check[9] = false;
                }
                else
                    CheckTimer[8] -= diff;
            }
            if(!Check[9])
            {
                if (CheckTimer[9] < diff)
                {
                    if(Creature* pKalecgos = (Creature*)FindCreature(CREATURE_KALECGOS, 100, me))
                        DoScriptText(SAY_KALECGOS_ENCOURAGE, pKalecgos);
                    if(Creature* pKilJaeden = (Creature*)FindCreature(CREATURE_KILJAEDEN, 100, me))
                    {
                        me->CastSpell(pKilJaeden, SPELL_SACRIFICE_OF_ANVEENA, true);
                        CAST_AI(boss_kiljaedenAI, pKilJaeden->AI())->DoStun();
                    }
                    me->SetVisibility(VISIBILITY_OFF);
                    Check[9] = true;
                }
                else
                    CheckTimer[9] -= diff;
            }
            CrashPreventionTimer = 1000;
        }
        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_anveena(Creature *_Creature)
{
    return new npc_anveenaAI (_Creature);
}

void AddSC_boss_kiljaeden()
{
    Script* newscript;

    newscript = new Script;
    newscript->pGOUse = &GOUse_go_orb_of_the_blue_flight;
    newscript->Name = "go_orb_of_the_blue_flight";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_kalecgos_kj;
    newscript->Name = "boss_kalecgos_kj";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_kiljaeden;
    newscript->Name = "boss_kiljaeden";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_kiljaeden_controller;
    newscript->Name = "mob_kiljaeden_controller";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_hand_of_the_deceiver;
    newscript->Name = "mob_hand_of_the_deceiver";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_felfire_portal;
    newscript->Name = "mob_felfire_portal";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_volatile_felfire_fiend;
    newscript->Name = "mob_volatile_felfire_fiend";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_armageddon;
    newscript->Name = "mob_armageddon";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_shield_orb;
    newscript->Name = "mob_shield_orb";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_sinster_reflection;
    newscript->Name = "mob_sinster_reflection";
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->GetAI = &GetAI_mob_24925;
    newscript->Name = "mob_24925";
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->GetAI = &GetAI_mob_26289;
    newscript->Name = "mob_26289";
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->GetAI = &GetAI_mob_26246;
    newscript->Name = "mob_26246";
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->GetAI = &GetAI_mob_26259;
    newscript->Name = "mob_26259";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_npc_anveena;
    newscript->Name = "npc_anveena";
    newscript->RegisterSelf();
};