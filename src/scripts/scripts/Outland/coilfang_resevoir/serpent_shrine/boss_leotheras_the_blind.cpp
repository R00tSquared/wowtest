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
SDName: Boss_Leotheras_The_Blind
SD%Complete: 80
SDComment: Possesion Support
SDCategory: Coilfang Resevoir, Serpent Shrine Cavern
EndScriptData */

#include "precompiled.h"
#include "def_serpent_shrine.h"

// --- Spells used by Leotheras The Blind
#define SPELL_WHIRLWIND         37640
#define SPELL_CHAOS_BLAST       37674
#define SPELL_BERSERK           26662
#define SPELL_INSIDIOUS_WHISPER 37676
#define SPELL_DUAL_WIELD        42459

// --- Spells used in banish phase ---
#define BANISH_BEAM             37626
#define AURA_BANISH             37833

// --- Spells used by Greyheart Spellbinders
#define SPELL_EARTHSHOCK        39076
#define SPELL_MINDBLAST         37531

// --- Spells used by Inner Demons and creature ID
#define INNER_DEMON_ID          21857
#define AURA_DEMONIC_ALIGNMENT  37713
#define SPELL_SHADOWBOLT        39309
#define SPELL_SOUL_LINK         38007
#define SPELL_CONSUMING_MADNESS 37749 //not supported by core yet

//Misc.
#define MODEL_DEMON             20125
#define MODEL_NIGHTELF          20514
#define DEMON_FORM              21875
#define MOB_SPELLBINDER         21806
#define MOB_PHANTOM_LEO         21812

#define SAY_AGGRO               -1548009
#define SAY_SWITCH_TO_DEMON     -1548010
#define SAY_INNER_DEMONS        -1548011
#define SAY_DEMON_SLAY1         -1548012
#define SAY_DEMON_SLAY2         -1548013
#define SAY_DEMON_SLAY3         -1548014
#define SAY_NIGHTELF_SLAY1      -1548015
#define SAY_NIGHTELF_SLAY2      -1548016
#define SAY_NIGHTELF_SLAY3      -1548017
#define SAY_FINAL_FORM          -1548018
#define SAY_FREE                -1548019
#define SAY_DEATH               -1548020

class InsidiousAura : public Aura {
public:
    InsidiousAura(SpellEntry *spell, uint32 eff, int32 *bp, Unit *target, Unit *caster) : Aura(spell, eff, bp, target, caster, NULL)
    {}
};

struct mob_inner_demonAI : public ScriptedAI
{
    mob_inner_demonAI(Creature *c) : ScriptedAI(c)
    {
        victimGUID = 0;
    }

    Timer_UnCheked ShadowBolt_Timer;

    Timer_UnCheked Link_Timer;
    uint64 victimGUID;

    void Reset()
    {
        ShadowBolt_Timer.Reset(10000);
        Link_Timer.Reset(1000);
    }
    void JustDied(Unit *victim)
    {
        Unit* pUnit = Unit::GetUnit((*m_creature),victimGUID);
        if (pUnit && pUnit->HasAura(SPELL_INSIDIOUS_WHISPER,0))
            pUnit->RemoveAurasDueToSpell(SPELL_INSIDIOUS_WHISPER);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if(done_by->GetCharmerOrOwnerOrOwnGUID() != victimGUID && done_by->GetCharmerOrOwnerOrOwnGUID() != m_creature->GetGUID())
        {
            damage = 0;
            DoModifyThreatPercent(done_by, -100);
        }
    }

    void OnAuraApply(Aura* aura, Unit* caster, bool addStack)
    {
        if (caster->GetCharmerOrOwnerOrOwnGUID() != victimGUID && caster->GetCharmerOrOwnerOrOwnGUID() != m_creature->GetGUID())
            me->RemoveAurasDueToSpell(aura->GetId());
    }

    void EnterCombat(Unit *who)
    {
        if (!victimGUID) return;
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if(Unit* owner = Unit::GetUnit((*m_creature),victimGUID))
        {
            if(m_creature->getVictimGUID() != victimGUID)
            {
                if(owner->isAlive())
                    AttackStart(owner);
                else
                    m_creature->Kill(m_creature,false);
            }
        }

        if(Link_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SOUL_LINK, true);
            Link_Timer = 1000;
			m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());
        }
        


        if(!m_creature->HasAura(AURA_DEMONIC_ALIGNMENT, 0))
            DoCast(m_creature, AURA_DEMONIC_ALIGNMENT,true);

        if (ShadowBolt_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SHADOWBOLT, false);
            ShadowBolt_Timer = 10000;
        }
        

       DoMeleeAttackIfReady();
    }
};
//Original Leotheras the Blind AI
struct boss_leotheras_the_blindAI : public ScriptedAI
{
    boss_leotheras_the_blindAI(Creature *c) : ScriptedAI(c)
    {
        m_creature->GetPosition(x,y,z);
        m_creature->GetPosition(wLoc);
        pInstance = (c->GetInstanceData());
        minstance = m_creature->GetMap();
        Demon = 0;
        Berserk_Timer = 600000;
    }

    ScriptedInstance *pInstance;

    Map *minstance;

    Timer Whirlwind_Timer;
    Timer ChaosBlast_Timer;
    Timer PulseCombat_Timer;
    Timer SwitchToDemon_Timer;
    Timer SwitchToHuman_Timer;
    Timer Berserk_Timer;
    Timer InnerDemons_Timer;
    Timer BanishTimer;
    Timer SummonPhantom_Timer;
    Timer RPEndTimer;

    WorldLocation wLoc;

    bool DealDamage;
    bool NeedThreatReset;
    bool DemonForm;
    bool IsFinalForm;
    bool RPPhase;
    float x,y,z;

    uint64 InnderDemon[5];
    uint32 InnderDemon_Count;
    uint64 Demon;
    uint64 SpellBinderGUID[3];
    uint64 actualtarget;

    void Reset()
    {
        EnterEvadeMode();
        RPEndTimer.Reset(0);
        SummonPhantom_Timer.Reset(10000);
        BanishTimer.Reset(1000);
        PulseCombat_Timer.Reset(5000);
        Whirlwind_Timer.Reset(15000);
        ChaosBlast_Timer.Reset(1000);
        SwitchToDemon_Timer.Reset(45000);
        SwitchToHuman_Timer.Reset(60000);
        Berserk_Timer.Reset(600000);
        InnerDemons_Timer.Reset(30000);
        m_creature->SetCanDualWield(true);
        DealDamage = true;
        DemonForm = false;
        IsFinalForm = false;
        NeedThreatReset = false;
        RPPhase = false;
        InnderDemon_Count = 0;
        m_creature->SetSpeed( MOVE_RUN, 2.0f, true);
        m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_NIGHTELF);
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY  , 0);
        m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 0);
        m_creature->CastSpell(m_creature, SPELL_DUAL_WIELD, true);
        m_creature->SetCorpseDelay(1000*60*60);

        if(pInstance && pInstance->GetData(DATA_LEOTHERASTHEBLINDEVENT) != DONE)
        {
            pInstance->SetData(DATA_LEOTHERASTHEBLINDEVENT, NOT_STARTED);
            pInstance->SetData64(DATA_LEOTHERAS_EVENT_STARTER, 0);
        }

        m_creature->SetReactState(REACT_AGGRESSIVE);
        m_creature->SetMeleeDamageSchool(SPELL_SCHOOL_NORMAL);

        std::list<uint64> banishers = minstance->GetCreaturesGUIDList(MOB_SPELLBINDER, GET_FIRST_CREATURE_GUID, 3);
        for (std::list<uint64>::iterator itr = banishers.begin(); itr != banishers.end(); itr++)
        {
            if (Creature* banisher = m_creature->GetMap()->GetCreature(*itr))
                if (!banisher->isAlive())
                {
                    banisher->Respawn();
                    banisher->AI()->EnterEvadeMode();
                }
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(m_creature->HasAura(AURA_BANISH, 0))
            return;

        if( !m_creature->GetVictim() && who->isTargetableForAttack() && ( m_creature->IsHostileTo( who )) && who->isInAccessiblePlacefor(m_creature) )
        {
            if (m_creature->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                return;

            float attackRadius = m_creature->GetAttackDistance(who);
            if(m_creature->IsWithinDistInMap(who, attackRadius))
            {
                // Check first that object is in an angle in front of this one before LoS check
                if( m_creature->HasInArc(M_PI/2.0f, who) && m_creature->IsWithinLOSInMap(who) )
                {
                    AttackStart(who);
                }
            }
        }
    }

    void StartEvent()
    {
        DoZoneInCombat();
        DoScriptText(SAY_AGGRO, m_creature);
        if(pInstance)
            pInstance->SetData(DATA_LEOTHERASTHEBLINDEVENT, IN_PROGRESS);
    }

    void OnAuraApply(Aura* aura, Unit* who, bool stacks)
    {
        if (aura->GetId() == AURA_BANISH)
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
    }

    void CheckBanish()
    {
        if (me->IsInEvadeMode())
            return;

        bool channelersAlive;
        if (minstance->GetCreatureById(MOB_SPELLBINDER, GET_ALIVE_CREATURE_GUID))
            channelersAlive = true;
        else
            channelersAlive = false;

        if (!channelersAlive && m_creature->HasAura(AURA_BANISH, 0))
        {
            // removing banish aura
            m_creature->RemoveAurasDueToSpell(AURA_BANISH);

            // Leotheras is getting immune again
            m_creature->ApplySpellImmune(AURA_BANISH, IMMUNITY_MECHANIC, MECHANIC_BANISH, true);

            // changing model to bloodelf
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_NIGHTELF);

            // and reseting equipment
            m_creature->LoadEquipment(m_creature->GetEquipmentId());

            // set stand
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);

            if(pInstance && pInstance->GetData64(DATA_LEOTHERAS_EVENT_STARTER))
            {
                Unit *victim = Unit::GetUnit(*m_creature, pInstance->GetData64(DATA_LEOTHERAS_EVENT_STARTER));
                if (victim)
                    m_creature->getThreatManager().addThreat(victim, 1);
                StartEvent();
            }
        }
        else if (channelersAlive && !m_creature->HasAura(AURA_BANISH, 0))
        {
            // Channelers are alive and banish is not present, reapply
            // removing Leotheras banish immune to apply AURA_BANISH
            m_creature->ApplySpellImmune(AURA_BANISH, IMMUNITY_MECHANIC, MECHANIC_BANISH, false);
            DoCast(m_creature, AURA_BANISH);

            // changing model
            // m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_DEMON);

            // and removing weapons
            // m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY  , 0);
            // m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 0);

            if (m_creature->IsInCombat())
                EnterEvadeMode();
        }
    }

    //Despawn all Inner Demon summoned
    void DespawnDemon()
    {
        for(int i=0; i<5; i++)
        {
            if(InnderDemon[i])
            {
                //delete creature
                Unit* pUnit = Unit::GetUnit((*m_creature), InnderDemon[i]);
                if(pUnit && pUnit->isAlive())
                    pUnit->Kill(pUnit, false);

                InnderDemon[i] = 0;
            }
        }

        InnderDemon_Count = 0;
    }

    void CastConsumingMadness() //remove this once SPELL_INSIDIOUS_WHISPER is supported by core
    {
        for(int i=0; i<5; i++)
        {
            if(InnderDemon[i] > 0 )
            {
                Unit* pUnit = Unit::GetUnit((*m_creature), InnderDemon[i]);
                if (pUnit && pUnit->isAlive())
                {
                    Unit* pUnit_target = Unit::GetUnit((*pUnit), ((mob_inner_demonAI *)((Creature *)pUnit)->AI())->victimGUID);
                    if( pUnit_target && pUnit_target->isAlive())
                        m_creature->CastSpell(pUnit_target, SPELL_CONSUMING_MADNESS, true);
                }
            }
        }
    }

    void KilledUnit(Unit *victim)
    {
        if (victim->GetTypeId() != TYPEID_PLAYER)
            return;

        if (DemonForm)
            DoScriptText(RAND(SAY_DEMON_SLAY1, SAY_DEMON_SLAY2, SAY_DEMON_SLAY3), m_creature);
        else
            DoScriptText(RAND(SAY_NIGHTELF_SLAY1, SAY_NIGHTELF_SLAY2, SAY_NIGHTELF_SLAY3), m_creature);
    }

    void JustDied(Unit *Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        //despawn copy
        if (Demon)
        {
            Unit *pUnit = Unit::GetUnit((*m_creature), Demon);
            if (pUnit)
                pUnit->DealDamage(pUnit, pUnit->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }
        if (pInstance)
            pInstance->SetData(DATA_LEOTHERASTHEBLINDEVENT, DONE);
    }

    void EnterCombat(Unit *who)
    {
        if(m_creature->HasAura(AURA_BANISH, 0))
            return;
        m_creature->LoadEquipment(m_creature->GetEquipmentId());
    }

    void UpdateAI(const uint32 diff)
    {
        if (RPPhase)
        {
            if (RPEndTimer.Expired(diff))
            {
                me->SetRooted(false);
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE, false);
                me->ClearUnitState(UNIT_STAT_CANNOT_TURN);
                Creature *Copy = NULL;
                Copy = DoSpawnCreature(DEMON_FORM, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 6000);
                if (Copy)
                {
                    Demon = Copy->GetGUID();
                    Copy->SetMeleeDamageSchool(SPELL_SCHOOL_FIRE);
                    if (m_creature->GetVictim())
                        Copy->AI()->AttackStart(m_creature->GetVictim());
                }
                RPEndTimer = 0;
                RPPhase = false;
            }
            return;
        }

        //Return since we have no target
        if (!UpdateVictim() || m_creature->HasAura(AURA_BANISH, 0))
        {
            if (SummonPhantom_Timer.Expired(diff))
            {
                me->SummonCreature(MOB_PHANTOM_LEO, 380.814, -441.007, 29.527, 2.703, TEMPSUMMON_TIMED_DESPAWN, 4000);
                SummonPhantom_Timer = 15000;
            }

            if (BanishTimer.Expired(diff))
            {
                CheckBanish();         //no need to check every update tick
                BanishTimer = 1000;
            }
            return;
        }

        if (PulseCombat_Timer.Expired(diff))
        {
            DoZoneInCombat();
            PulseCombat_Timer = 2000;
            me->SetSpeed(MOVE_RUN, DemonForm ? 3.0 : 2.0);
        }
        

        if(m_creature->HasAura(SPELL_WHIRLWIND, 0))
        {
            if (Whirlwind_Timer.Expired(diff))
            {
                
                DoResetThreat();
                
                Whirlwind_Timer = 2000;
            }
        }

        // reseting after changing forms and after ending whirlwind
        if(NeedThreatReset && !m_creature->HasAura(SPELL_WHIRLWIND, 0))
        {
            // when changing forms seting timers (or when ending whirlwind - to avoid adding new variable i use Whirlwind_Timer to countdown 2s while whirlwinding)
            if(DemonForm)
                InnerDemons_Timer = 30000;
            else
                Whirlwind_Timer =  15000;

            actualtarget = m_creature->getVictimGUID();
            if (actualtarget)
            {
                NeedThreatReset = false;
                DoResetThreat();
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MoveChase(m_creature->GetUnit(actualtarget));
                DoZoneInCombat();
                m_creature->SetReactState(REACT_AGGRESSIVE);
            }
        }

        //Enrage_Timer ( 10 min )
        if (Berserk_Timer.Expired(diff))
        {
            if(!m_creature->HasAura(SPELL_BERSERK, 0))
                DoCast(m_creature, SPELL_BERSERK);
            m_creature->SetSpeed(MOVE_RUN, 3.0);
            Berserk_Timer = 5000;
        }
        

        if(!DemonForm)
        {
            //Whirldind Timer
            if(!m_creature->HasAura(SPELL_WHIRLWIND, 0))
            {
                if (Whirlwind_Timer.Expired(diff))
                {
                    DoCast(m_creature, SPELL_WHIRLWIND);
                    // while whirlwinding this variable is used to countdown target's change
                    Whirlwind_Timer = 2000;
                    NeedThreatReset = true;
                    //m_creature->SetReactState(REACT_PASSIVE);
                }
            }

            //Switch_Timer
            if(!IsFinalForm)
            {
                if (SwitchToDemon_Timer.Expired(diff))
                {
                    //switch to demon form
                    m_creature->RemoveAurasDueToSpell(SPELL_WHIRLWIND,0);
                    m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_DEMON);
                    DoScriptText(SAY_SWITCH_TO_DEMON, m_creature);
                    m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY  , 0);
                    m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 0);
                    DemonForm = true;
                    NeedThreatReset = true;
                    SwitchToDemon_Timer = 45000;
                    m_creature->SetReactState(REACT_AGGRESSIVE);
                    m_creature->SetMeleeDamageSchool(SPELL_SCHOOL_FIRE);
                }
            }
            DoMeleeAttackIfReady();
        }
        else
        {
            //ChaosBlast_Timer
            if (!m_creature->GetVictim())
                return;

            if(m_creature->IsWithinDistInMap(m_creature->GetVictim(), 30))
                m_creature->StopMoving();

            if (ChaosBlast_Timer.Expired(diff))
            {
                // will cast only when in range of spell
                if(m_creature->IsWithinDistInMap(m_creature->GetVictim(), 30))
                {
                    //m_creature->CastSpell(m_creature->GetVictim(), SPELL_CHAOS_BLAST, true);
                    int damage = 100;
                    m_creature->CastCustomSpell(m_creature->GetVictim(), SPELL_CHAOS_BLAST, &damage, NULL, NULL, false, NULL, NULL, m_creature->GetGUID());
                }
                ChaosBlast_Timer = 3000;
            }
           

            //Summon Inner Demon
            if (InnerDemons_Timer.Expired(diff))
            {
                std::list<HostileReference *>& ThreatList = m_creature->getThreatManager().getThreatList();
                std::vector<Unit *> TargetList;

                for(std::list<HostileReference *>::iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
                {
                    Unit *tempTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true);
                    if(tempTarget && !tempTarget->HasAura(SPELL_CONSUMING_MADNESS,0) && tempTarget->GetGUID() != m_creature->getVictimGUID() && std::find(TargetList.begin(), TargetList.end(), tempTarget) == TargetList.end() && TargetList.size() < 5)
                        TargetList.push_back(tempTarget);
                }

                SpellEntry *spell = (SpellEntry *)GetSpellStore()->LookupEntry<SpellEntry>(SPELL_INSIDIOUS_WHISPER);
                for(std::vector<Unit *>::iterator itr = TargetList.begin(); itr != TargetList.end(); ++itr)
                {
                    if( (*itr) && (*itr)->isAlive() )
                    {
                        float x, y, z;
                        (*itr)->GetNearPoint(x, y, z, 0.0f, 15.0f, frand(0, 2 * M_PI));
                        Creature * demon = (Creature *)m_creature->SummonCreature(INNER_DEMON_ID, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                        if(demon)
                        {
                            ((ScriptedAI *)demon->AI())->AttackStart( (*itr) );
                            ((mob_inner_demonAI *)demon->AI())->victimGUID = (*itr)->GetGUID();

                            for (int i=0; i<3; i++)
                            {
                                if (!spell->Effect[i])
                                    continue;
                                (*itr)->AddAura(new InsidiousAura(spell, i, NULL, (*itr), (*itr)));
                            }
                            if( InnderDemon_Count > 4 ) InnderDemon_Count = 0;

                            //Safe storing of creatures
                            InnderDemon[InnderDemon_Count] = demon->GetGUID();

                            //Update demon count
                            InnderDemon_Count++;
                        }
                    }
                }
                DoScriptText(SAY_INNER_DEMONS, m_creature);

                InnerDemons_Timer = 0;
            }


            //Switch_Timer
            if (SwitchToHuman_Timer.Expired(diff))
            {
                //switch to nightelf form
                m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_NIGHTELF);
                m_creature->LoadEquipment(m_creature->GetEquipmentId());

                CastConsumingMadness();
                DespawnDemon();

                DemonForm = false;
                NeedThreatReset = true;

                SwitchToHuman_Timer = 60000;
                m_creature->SetMeleeDamageSchool(SPELL_SCHOOL_NORMAL);
            }
        }

        if (!IsFinalForm && (me->GetHealthPercent()) < 15)
        {
            RPPhase = true;
            IsFinalForm = true;
            DemonForm = false;
            me->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
            me->SetRooted(true);
            me->AttackStop();
            me->SetReactState(REACT_PASSIVE);
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_NIGHTELF);
            m_creature->LoadEquipment(m_creature->GetEquipmentId());
            m_creature->SetMeleeDamageSchool(SPELL_SCHOOL_NORMAL);
            DoScriptText(SAY_FINAL_FORM, m_creature);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE, true);
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            me->addUnitState(UNIT_STAT_CANNOT_TURN);
            RPEndTimer = 12000;
        }
    }
};

//Leotheras the Blind Demon Form AI
struct boss_leotheras_the_blind_demonformAI : public ScriptedAI
{
    boss_leotheras_the_blind_demonformAI(Creature *c) : ScriptedAI(c) {}

    Timer_UnCheked ChaosBlast_Timer;
    bool DealDamage;
    Timer_UnCheked checkTimer;

    void Reset()
    {
        ChaosBlast_Timer.Reset(1000);
        checkTimer.Reset(2000);
        DealDamage = true;
        m_creature->SetMeleeDamageSchool(SPELL_SCHOOL_FIRE);
    }

    void StartEvent()
    {
        DoScriptText(SAY_FREE, m_creature);
    }

    void KilledUnit(Unit *victim)
    {
        if (victim->GetTypeId() != TYPEID_PLAYER)
            return;

        DoScriptText(RAND(SAY_DEMON_SLAY1, SAY_DEMON_SLAY2, SAY_DEMON_SLAY3), m_creature);
    }

    void JustDied(Unit *victim)
    {
        //invisibility (blizzlike, at the end of the fight he doesn't die, he disappears)
        m_creature->CastSpell(m_creature, 8149, true);
    }

    void EnterCombat(Unit *who)
    {
        StartEvent();
    }

    void UpdateAI(const uint32 diff)
    {
       
        //Return since we have no target
        if (!UpdateVictim() )
            return;
        if (checkTimer.Expired(diff))
        {
            checkTimer = 2000;
            me->SetSpeed(MOVE_RUN, 3.0);
        }

        //ChaosBlast_Timer
        if(m_creature->IsWithinDistInMap(m_creature->GetVictim(), 25))
            m_creature->StopMoving();

        if (ChaosBlast_Timer.Expired(diff))
         {
            // will cast only when in range od spell
            if(m_creature->IsWithinDistInMap(m_creature->GetVictim(), 25))
            {
                //m_creature->CastSpell(m_creature->GetVictim(),SPELL_CHAOS_BLAST,true);
                int damage = 100;
                m_creature->CastCustomSpell(m_creature->GetVictim(), SPELL_CHAOS_BLAST, &damage, NULL, NULL, false, NULL, NULL, m_creature->GetGUID());
                ChaosBlast_Timer = 3000;
            }
         }

        //Do NOT deal any melee damage to the target.
    }
};
struct mob_greyheart_spellbinderAI : public ScriptedAI
{
    mob_greyheart_spellbinderAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance *)c->GetInstanceData());
        leotherasGUID = 0;
    }

    ScriptedInstance *pInstance;

    uint64 leotherasGUID;

    Timer_UnCheked Mindblast_Timer;
    Timer_UnCheked Earthshock_Timer;
    Timer ChannelingTimer;
    Timer UnattackableFlagTimer;

    void Reset()
    {
        Mindblast_Timer.Reset(3000 + rand() % 5000);
        Earthshock_Timer.Reset(5000 + rand() % 5000);
        ChannelingTimer.Reset(1000);
        UnattackableFlagTimer.Reset(1000); // first comes faster, then check every 10 sec
        if(pInstance)
            pInstance->SetData64(DATA_LEOTHERAS_EVENT_STARTER, 0);

        std::list<uint64> banishers = me->GetMap()->GetCreaturesGUIDList(MOB_SPELLBINDER, GET_FIRST_CREATURE_GUID, 3);
        for (std::list<uint64>::iterator itr = banishers.begin(); itr != banishers.end(); itr++)
        {
            if (Creature* banisher = m_creature->GetMap()->GetCreature(*itr))
                if (!banisher->isAlive())
                {
                    banisher->Respawn();
                    banisher->AI()->EnterEvadeMode();
                }
        }
    }

    void EnterCombat(Unit *who)
    {
        m_creature->InterruptNonMeleeSpells(false);
        if(pInstance)
            pInstance->SetData64(DATA_LEOTHERAS_EVENT_STARTER, who->GetGUID());
    }

    void JustRespawned()
    {
        EnterEvadeMode();
    }

    void CastChannelingIfNeeded()
    {
        if (!m_creature->IsInEvadeMode() && !m_creature->IsInCombat() && !m_creature->m_currentSpells[CURRENT_CHANNELED_SPELL])
        {
            if(leotherasGUID)
            {
                Creature *leotheras = (Creature *)Unit::GetUnit(*m_creature, leotherasGUID);
                if (leotheras && leotheras->isAlive() && leotheras->HasAura(AURA_BANISH, 0))
                {
                    DoCast(leotheras, BANISH_BEAM);
                }
            }
        }
    }

    void CheckUnattackableFlag()
    {
        if (!m_creature->IsInEvadeMode() && !m_creature->IsInCombat())
        {
            if (leotherasGUID)
            {
                Creature *leotheras = (Creature *)Unit::GetUnit(*m_creature, leotherasGUID);
                if (leotheras && leotheras->isAlive())
                {
                    bool bossUnattackable = leotheras->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    bool meUnattackable = me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    if (bossUnattackable != meUnattackable)
                        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, bossUnattackable);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(pInstance)
        {
            if(!leotherasGUID)
                leotherasGUID = pInstance->GetData64(DATA_LEOTHERAS);

            if(!m_creature->IsInCombat() && pInstance->GetData64(DATA_LEOTHERAS_EVENT_STARTER))
            {
                Unit *victim = Unit::GetUnit(*m_creature, pInstance->GetData64(DATA_LEOTHERAS_EVENT_STARTER));
                if(victim)
                    AttackStart(victim);
            }
        }

        if(!UpdateVictim())
        {
            if (ChannelingTimer.Expired(diff))
            {
                CastChannelingIfNeeded();         //no need to check every update tick
                ChannelingTimer = 1000;
            }
            if (UnattackableFlagTimer.Expired(diff))
            {
                CheckUnattackableFlag();         //no need to check every update tick
                UnattackableFlagTimer = 10000;
            }
            return;
        }

        if(pInstance && pInstance->GetData64(DATA_LEOTHERAS_EVENT_STARTER) == 0)
        {
            EnterEvadeMode(); // if one of them is evaded - all evade
            return;
        }

        if (Mindblast_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0, GetSpellMaxRange(SPELL_MINDBLAST), true))
                DoCast(target, SPELL_MINDBLAST);

            Mindblast_Timer = 10000 + rand()%5000;
        }

        if (Earthshock_Timer.Expired(diff))
        {
            Map *map = m_creature->GetMap();
            Map::PlayerList const &PlayerList = map->GetPlayers();
            for(Map::PlayerList::const_iterator itr = PlayerList.begin();itr != PlayerList.end(); ++itr)
            {
                if (Player* i_pl = itr->getSource())
                {
                    bool isCasting = false;
                    for(uint8 i = 0; i < CURRENT_MAX_SPELL; ++i)
                        if(i_pl->m_currentSpells[i])
                            isCasting = true;

                    if(isCasting)
                    {
                        DoCast(i_pl, SPELL_EARTHSHOCK);
                        break;
                    }
                }
            }
            Earthshock_Timer = 8000 + rand()%7000;
        }
        DoMeleeAttackIfReady();
    }

    void JustDied(Unit *killer) {}
};
CreatureAI* GetAI_boss_leotheras_the_blind(Creature *_Creature)
{
    return new boss_leotheras_the_blindAI (_Creature);
}

CreatureAI* GetAI_boss_leotheras_the_blind_demonform(Creature *_Creature)
{
    return new boss_leotheras_the_blind_demonformAI (_Creature);
}

CreatureAI* GetAI_mob_greyheart_spellbinder(Creature *_Creature)
{
    return new mob_greyheart_spellbinderAI (_Creature);
}

CreatureAI* GetAI_mob_inner_demon(Creature *_Creature)
{
    return new mob_inner_demonAI (_Creature);
}
void AddSC_boss_leotheras_the_blind()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_leotheras_the_blind";
    newscript->GetAI = &GetAI_boss_leotheras_the_blind;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_leotheras_the_blind_demonform";
    newscript->GetAI = &GetAI_boss_leotheras_the_blind_demonform;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_greyheart_spellbinder";
    newscript->GetAI = &GetAI_mob_greyheart_spellbinder;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_inner_demon";
    newscript->GetAI = &GetAI_mob_inner_demon;
    newscript->RegisterSelf();
}

