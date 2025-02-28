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
SDName: Boss_Twinemperors
SD%Complete: 95
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_temple_of_ahnqiraj.h"
#include "WorldPacket.h"

#include "Item.h"
#include "Spell.h"

#define SPELL_HEAL_BROTHER          7393
#define SPELL_TWIN_TELEPORT_BLINK   799
#define SPELL_TWIN_TELEPORT         800                     // includes 2 sec stun
#define SPELL_TWIN_TELEPORT_VISUAL  26638                   // visual

#define SPELL_EXPLODEBUG            804
#define SPELL_MUTATE_BUG            802

#define SOUND_VN_DEATH              8660                    //8660 - Death - Feel
#define SOUND_VN_AGGRO              8661                    //8661 - Aggro - Let none
#define SOUND_VN_KILL               8662                    //8661 - Kill - your fate

#define SOUND_VL_AGGRO              8657                    //8657 - Aggro - To Late
#define SOUND_VL_KILL               8658                    //8658 - Kill - You will not
#define SOUND_VL_DEATH              8659                    //8659 - Death

#define PULL_RANGE                  50
#define ABUSE_BUG_RANGE             20
#define SPELL_BERSERK               26662
#define TELEPORTTIME                30000

#define SPELL_UPPERCUT              26007
#define SPELL_UNBALANCING_STRIKE    26613

#define VEKLOR_DIST                 20                      // VL will not come to melee when attacking

#define SPELL_SHADOWBOLT            26006
#define SPELL_BLIZZARD              26607
#define SPELL_ARCANEBURST           568

struct boss_twinemperorsAI : public ScriptedAI
{
    ScriptedInstance *pInstance;

    Timer Abuse_Bug_Timer;
    Timer EnrageTimer;
    bool AfterTeleport;
    bool DontYellWhenDead;

    virtual bool IAmVeklor() = 0;
    virtual void Reset() = 0;

    boss_twinemperorsAI(Creature *c): ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    void TwinReset()
    {
        Abuse_Bug_Timer.Reset(10000 + rand()%7000);
        EnrageTimer.Reset(15*60000);
        AfterTeleport       = false;
        DontYellWhenDead    = false;

        if (pInstance)
            pInstance->SetData(DATA_TWIN_EMPERORS, NOT_STARTED);
    }

    Creature *GetOtherBoss()
    {
        if(pInstance)
            return (Creature *)Unit::GetUnit((*me), pInstance->GetData64(IAmVeklor() ? DATA_VEKNILASH : DATA_VEKLOR));
        else
            return (Creature *)0;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        Creature* pOtherBoss = GetOtherBoss();
        if (pOtherBoss)
        {
            if (me->GetHealth() > damage) // otherwise we will just die
            {
                // we gotta set NEW health
                float myPct = (me->GetHealth() - damage) * 100.0f / me->GetMaxHealth();
                float broPct = pOtherBoss->GetHealthPercent();

                if (myPct < broPct) // we were damaged -> we should have less health than before. Percents should already be in sync
                    pOtherBoss->SetHealth(pOtherBoss->GetMaxHealth() / 100 * myPct);
            }
        }
    }

    void JustDied(Unit* Killer)
    {
        Creature *pOtherBoss = GetOtherBoss();
        if (pOtherBoss && !DontYellWhenDead)
        {
            ((boss_twinemperorsAI *)pOtherBoss->AI())->DontYellWhenDead = true;

            if (HeroicMode)
            {
                pOtherBoss->Kill(pOtherBoss);
                pOtherBoss->SetLootRecipient(NULL); // on heroic only one of them is lootable
            }
            else
            {
                pOtherBoss->SetLootRecipient(Killer); // both are lootable on normal after death
                Killer->Kill(pOtherBoss);
            }
        }

        if (!DontYellWhenDead)                              // I hope AI is not threaded
            DoPlaySoundToSet(me, IAmVeklor() ? SOUND_VL_DEATH : SOUND_VN_DEATH);

        if (pInstance)
            pInstance->SetData(DATA_TWIN_EMPERORS, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        DoPlaySoundToSet(me, IAmVeklor() ? SOUND_VL_KILL : SOUND_VN_KILL);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();
        Creature *pOtherBoss = GetOtherBoss();
        if (pOtherBoss)
        {
            // TODO: we should activate the other boss location so he can start attackning even if nobody
            // is near I dont know how to do that
            ScriptedAI *otherAI = (ScriptedAI*)pOtherBoss->AI();
            if (!pOtherBoss->IsInCombat())
            {
                DoPlaySoundToSet(me, IAmVeklor() ? SOUND_VL_AGGRO : SOUND_VN_AGGRO);
                otherAI->AttackStart(who);
                otherAI->DoZoneInCombat();
                me->SetPlayerDamageReqInPct(0); // Lootable by any means, cause damage is divided
            }
        }
        me->SetPlayerDamageReqInPct(0); // lootable by any means, cause damage is divided
        if (pInstance)
            pInstance->SetData(DATA_TWIN_EMPERORS, IN_PROGRESS);
    }

    void SetAfterTeleport()
    {
        me->InterruptNonMeleeSpells(false);
        DoStopAttack();
        DoCast(me, SPELL_TWIN_TELEPORT_VISUAL);
        AfterTeleport = true;
        DoCast(me, SPELL_TWIN_TELEPORT_BLINK, true);
        DoCast(me, SPELL_TWIN_TELEPORT, true);
    }

    void ActivateAfterTeleportIfNeeded()
    {
        if (AfterTeleport)
        {
            DoResetThreat();
            AfterTeleport = false;
            // searches through aggrolist, which was created by DoZoneInCombat()
            if (Unit *nearu = SelectUnit(SELECT_TARGET_NEAREST, 0, 100.0f, true))
            {
                AttackStart(nearu);
                me->getThreatManager().addThreat(nearu, HeroicMode ? 17000 : 10000);
            }
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || me->GetVictim())
            return;

        if (who->isTargetableForAttack() && who->isInAccessiblePlacefor(me) && me->IsHostileTo(who))
        {
            float attackRadius = me->GetAttackDistance(who);
            if (attackRadius < PULL_RANGE)
                attackRadius = PULL_RANGE;
            if (me->IsWithinDistInMap(who, attackRadius) && me->GetDistanceZ(who) <= /*CREATURE_Z_ATTACK_RANGE*/7 /*there are stairs*/)
            {
                //if(who->HasStealthAura())
                //    who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                AttackStart(who);
            }
        }
    }

    class AnyBugCheck
    {
        public:
            AnyBugCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
            bool operator()(Unit* u)
            {
                Creature *c = (Creature *)u;
                if (!i_obj->IsWithinDistInMap(c, i_range))
                    return false;
                return (c->GetEntry() == 15316 || c->GetEntry() == 15317);
            }
        private:
            WorldObject const* i_obj;
            float i_range;
    };

    Creature *RespawnNearbyBugsAndGetOne()
    {
        std::list<Creature*> unitList;

        AnyBugCheck u_check(me, 150);
        Hellground::ObjectListSearcher<Creature, AnyBugCheck> searcher(unitList, u_check);
        Cell::VisitGridObjects(me, searcher, 150);

        Creature *nearb = NULL;

        for(std::list<Creature*>::iterator iter = unitList.begin(); iter != unitList.end(); ++iter)
        {
            Creature *c = *iter;
            if (c->isDead())
            {
                c->Respawn();
                c->setFaction(7);
                c->RemoveAllAuras();
            }

            if (c->IsWithinDistInMap(me, ABUSE_BUG_RANGE))
            {
                if (!nearb || (rand()%4)==0)
                    nearb = c;
            }
        }
        return nearb;
    }

    void HandleBugs(uint32 diff)
    {
        if (Abuse_Bug_Timer.Expired(diff))
        {
            if (Creature *c = RespawnNearbyBugsAndGetOne())
            {
                c->setFaction(14);
                ((CreatureAI*)c->AI())->AttackStart(me->getThreatManager().getHostilTarget());
                c->AddAura(IAmVeklor() ? SPELL_EXPLODEBUG : SPELL_MUTATE_BUG, c);
                c->SetHealth(c->GetMaxHealth());
                Abuse_Bug_Timer = 10000 + rand()%7000;
            }
            else
                Abuse_Bug_Timer = 1000; // recheck in 1 sec
        }
    }

    void CheckEnrage(uint32 diff)
    {
        if (EnrageTimer.Expired(diff))
        {
            if (me->IsNonMeleeSpellCast(true))
                EnrageTimer = 1000; // recheck in 1 sec
            else
            {
                DoCast(me, SPELL_BERSERK);
                EnrageTimer = 5*60000; // 5 min is berserk duration, recast it on aura remove
            }
        } 
    }
};

struct boss_veklorAI : public boss_twinemperorsAI
{
    bool IAmVeklor() { return true; }
    boss_veklorAI(Creature *c) : boss_twinemperorsAI(c) {}

    Timer ShadowBolt_Timer;
    Timer Blizzard_Timer;
    Timer ArcaneBurst_Timer;

    void Reset()
    {
        TwinReset();
        ShadowBolt_Timer.Reset(2000);
        Blizzard_Timer.Reset(15000 + rand() % 5000);
        ArcaneBurst_Timer.Reset(1000);
        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, true);
        me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 0);
        me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 0);
        m_creature->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, 0.55f);   //custom, should be verified
        m_creature->SetFloatValue(UNIT_FIELD_COMBATREACH, 2);
    }

    void SpellHit(Unit *caster, const SpellEntry *entry)
    {
        Creature *pOtherBoss = GetOtherBoss();
        if (entry->Id != SPELL_HEAL_BROTHER || !pOtherBoss)
            return;

        float myPct = me->GetHealthPercent();
        float broPct = pOtherBoss->GetHealthPercent();

        if (myPct > broPct) // we were healed -> we should have more health than before. Percents should already be in sync
            pOtherBoss->SetHealth(pOtherBoss->GetMaxHealth()/100*myPct);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        ActivateAfterTeleportIfNeeded();

        //Blizzard_Timer
        if (Blizzard_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_BLIZZARD), true))
            {
                DoCast(target, SPELL_BLIZZARD);
                Blizzard_Timer = 15000 + rand() % 15000;
            }
            else
                Blizzard_Timer = 1000; // recheck in 1 sec
        }

        if (ArcaneBurst_Timer.Expired(diff))
        {
            if (Unit *mvic = SelectUnit(SELECT_TARGET_NEAREST, 0, NOMINAL_MELEE_RANGE, true))
            {
                DoCast(mvic, SPELL_ARCANEBURST);
                ArcaneBurst_Timer = 5000;
            }
            else
                ArcaneBurst_Timer = 1000; // recheck in 1 sec
        }

        HandleBugs(diff);

        CheckEnrage(diff);

        //ShadowBolt_Timer
        if (ShadowBolt_Timer.Expired(diff))
        {
            if (!me->IsWithinDistInMap(me->GetVictim(), 45))
                me->GetMotionMaster()->MoveChase(me->GetVictim(), VEKLOR_DIST, 0);
            else
                DoCast(me->GetVictim(), SPELL_SHADOWBOLT);
            ShadowBolt_Timer = 2000;
        }

        //VL doesn't melee
        //DoMeleeAttackIfReady();
    }

    void AttackStart(Unit* who)
    {
        if (!who)
            return;

        if (who->isTargetableForAttack())
        {
            // VL doesn't melee
            if (me->Attack(who, false))
            {
                me->GetMotionMaster()->MoveChase(who, VEKLOR_DIST, 0);
                me->AddThreat(who, 0.0f);
            }

            if (!me->IsInCombat())
                EnterCombat(who);
        }
    }
};

struct boss_veknilashAI : public boss_twinemperorsAI
{
    bool IAmVeklor() {return false;}
    boss_veknilashAI(Creature *c) : boss_twinemperorsAI(c) {}

    Timer Heal_Timer;
    Timer Teleport_Timer;

    Timer UpperCut_Timer;
    Timer UnbalancingStrike_Timer;

    void Reset()
    {
        TwinReset();
        Heal_Timer.Reset(1000);                                     // first heal immediately when they get close together
        Teleport_Timer.Reset(TELEPORTTIME);
        UpperCut_Timer.Reset(14000 + rand() % 15000);
        UnbalancingStrike_Timer.Reset(8000 + rand() % 10000);
        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        ActivateAfterTeleportIfNeeded();

        //UnbalancingStrike_Timer
        if (UnbalancingStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_UNBALANCING_STRIKE);
            UnbalancingStrike_Timer = 8000+rand()%12000;
        }

        if (UpperCut_Timer.Expired(diff))
        {
            Unit* randomMelee = SelectUnit(SELECT_TARGET_RANDOM, 0, NOMINAL_MELEE_RANGE, true);
            if (randomMelee)
                DoCast(randomMelee,SPELL_UPPERCUT);
            UpperCut_Timer = 15000+rand()%15000;
        }

        HandleBugs(diff);

        //Heal brother when 60yrds close
        if (Heal_Timer.Expired(diff))
        {
            Unit *pOtherBoss = GetOtherBoss();
            if (pOtherBoss && pOtherBoss->isAlive() && (pOtherBoss->IsWithinDistInMap(me, 60)))
                DoCast(pOtherBoss, SPELL_HEAL_BROTHER); // two effects, first heals Vek'Lor, second heals self. Vek'Lor has less max HP, so SpellHit is on him
            Heal_Timer = 1000;
        }

        //Teleporting to brother
        if (Teleport_Timer.Expired(diff))
        {
            if (Creature *pOtherBoss = GetOtherBoss())
            {
                float other_x = pOtherBoss->GetPositionX();
                float other_y = pOtherBoss->GetPositionY();
                float other_z = pOtherBoss->GetPositionZ();
                float other_o = pOtherBoss->GetOrientation();

                pOtherBoss->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                me->NearTeleportTo(other_x, other_y, other_z, other_o);

                SetAfterTeleport();
                ((boss_twinemperorsAI*)pOtherBoss->AI())->SetAfterTeleport();
                // reset arcane burst after teleport - we need to do this because
                // when VL jumps to VN's location there will be a warrior who will get only 2s to run away
                // which is almost impossible
                ((boss_veklorAI*)pOtherBoss->AI())->ArcaneBurst_Timer = 5000;
            }
            Teleport_Timer = TELEPORTTIME;
            return; // stunned right after teleport
        }

        CheckEnrage(diff);

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_veklor(Creature *_Creature)
{
    return new boss_veklorAI(_Creature);
}

CreatureAI* GetAI_boss_veknilash(Creature *_Creature)
{
    return new boss_veknilashAI (_Creature);
}

void AddSC_boss_twinemperors()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_veklor";
    newscript->GetAI = &GetAI_boss_veklor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_veknilash";
    newscript->GetAI = &GetAI_boss_veknilash;
    newscript->RegisterSelf();
}

