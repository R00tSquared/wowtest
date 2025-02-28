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
SDName: Thaddius encounter
SD%Complete: 99
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

//Stalagg
#define SAY_STAL_AGGRO          -1533023
#define SAY_STAL_SLAY           -1533024
#define SAY_STAL_DEATH          -1533025

//Feugen
#define SAY_FEUG_AGGRO          -1533026
#define SAY_FEUG_SLAY           -1533027
#define SAY_FEUG_DEATH          -1533028

//Thaddus
#define SAY_GREET               -1533029
#define SAY_AGGRO1              -1533030
#define SAY_AGGRO2              -1533031
#define SAY_AGGRO3              -1533032
#define SAY_SLAY                -1533033
#define SAY_ELECT               -1533034
#define SAY_DEATH               -1533035
#define SAY_SCREAM1             -1533036
#define SAY_SCREAM2             -1533037
#define SAY_SCREAM3             -1533038
#define SAY_SCREAM4             -1533039

enum eSpells
{
    // Thaddius Spells
    SPELL_THADIUS_SPAWN             = 28160,
    SPELL_THADIUS_LIGHTNING_VISUAL  = 28136,
    SPELL_BALL_LIGHTNING            = 28299,
    SPELL_CHAIN_LIGHTNING           = 28167,
    SPELL_POLARITY_SHIFT            = 28089,
    SPELL_BESERK                    = 27680,

    // Stalagg & Feugen Spells
    SPELL_WARSTOMP                  = 28125,
    SPELL_MAGNETIC_PULL             = 28338, // Just visual effect
    SPELL_STATIC_FIELD              = 28135, // Mana Burn
    SPELL_POWERSURGE                = 28134,

    // Tesla Spells
    SPELL_FEUGEN_CHAIN              = 28111,
    SPELL_STALAGG_CHAIN             = 28096,
    SPELL_FEUGEN_TESLA_PASSIVE      = 28109,
    SPELL_STALAGG_TESLA_PASSIVE     = 28097,
    SPELL_SHOCK_OVERLOAD            = 28159,
    SPELL_SHOCK                     = 28099,
};

struct boss_thaddiusAI : public Scripted_NoMovementAI
{
    boss_thaddiusAI(Creature *c) : Scripted_NoMovementAI(c)
    { pInstance = (instance_naxxramas*)c->GetInstanceData(); }

    instance_naxxramas* pInstance;
    uint32 PolarityShiftTimer;
    uint32 ChainLightningTimer;
    uint32 BallLightningTimer;
    uint32 BerserkTimer;
    bool IntroSay;

    void Reset()
    {
        PolarityShiftTimer = 30000;
        ChainLightningTimer = 8000;
        BallLightningTimer = 1000;
        BerserkTimer = 300000; // 5 minutes
        IntroSay = true;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        DoCast(me, SPELL_THADIUS_SPAWN);
        if (pInstance)
            pInstance->SetData(DATA_THADDIUS, NOT_STARTED);
    }

    void EnterCombat(Unit*)
    {
        if (pInstance)
            pInstance->SetData(DATA_THADDIUS, IN_PROGRESS);
    }
    
    void MoveInLineOfSight(Unit* who)
    {
        if(IntroSay)
        {
            DoScriptText(SAY_GREET, me);
            IntroSay = false;
        }
        if(who->GetTypeId() == TYPEID_PLAYER && me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
        {
            if(me->IsWithinDistInMap(who, 50))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DoScriptText(SAY_AGGRO1, me);
            }
        }
    }
    
    void EnterEvadeMode()
    {
        if (pInstance)
            pInstance->SetData(DATA_THADDIUS, FAIL);
        if(Creature* Feugen = (Creature*)FindCreature(NPC_FEUGEN, 140, me))
            Feugen->AI()->Reset();
        if(Creature* Stalagg = (Creature*)FindCreature(NPC_STALAGG, 140, me))
            Stalagg->AI()->Reset();
        Scripted_NoMovementAI::EnterEvadeMode();
    }

    void KilledUnit(Unit* who)
    {
        DoScriptText(RAND(SAY_AGGRO2, SAY_AGGRO3, SAY_SLAY), me);
    }

    void JustDied(Unit* who)
    {
        DoScriptText(SAY_DEATH, me);
        if (pInstance)
            pInstance->SetData(DATA_THADDIUS, DONE);
        if(Creature* Feugen = (Creature*)FindCreature(NPC_FEUGEN, 200, me))
            Feugen->DisappearAndDie();
        if(Creature* Stalagg = (Creature*)FindCreature(NPC_STALAGG, 200, me))
            Stalagg->DisappearAndDie();

        if (Player* objPlr = pInstance->GetPlayer())
        {
            if(Creature *TeleportTrigger = objPlr->SummonTrigger(3539.02, -2936.82, 302.48, 3.14, 0, nullptr, true))
                TeleportTrigger->SummonGameObject(GO_TELEPORT_NAX_WORKING, 3539.02, -2936.82, 302.48, 3.14, 0, 0, 0, 0, 0);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // Berserk
        if (BerserkTimer < diff)
        {
            DoCast(me, SPELL_BESERK); // allow combat movement?
            BerserkTimer = 10 * MINUTE * 1000;
        }
        else
            BerserkTimer -= diff;

        // Polarity Shift
        if (PolarityShiftTimer < diff)
        {
            DoCast(me, SPELL_POLARITY_SHIFT);
            DoScriptText(SAY_ELECT, me);
            PolarityShiftTimer = 30000;
        }
        else
            PolarityShiftTimer -= diff;

        // Chain Lightning
        if (ChainLightningTimer < diff)
        {
            Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM);
            if (pTarget)
                DoCast(pTarget, SPELL_CHAIN_LIGHTNING);
            ChainLightningTimer = 15000;
        }
        else
            ChainLightningTimer -= diff;

        if (!me->IsWithinMeleeRange(me->GetVictim()))
        {
            if (BallLightningTimer < diff)
            {
                DoCast(me->GetVictim(), SPELL_BALL_LIGHTNING);
                BallLightningTimer = 1000;
            }
            else
                BallLightningTimer -= diff;
        }
        else
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_thaddius(Creature *_Creature)
{
    return new boss_thaddiusAI(_Creature);
}
    
bool EffectDummyNPC_spell_thaddius_encounter(Unit* pCaster, uint32 uiSpellId, uint32 uiEffIndex, Creature* pCreatureTarget)
{
    switch (uiSpellId)
    {
        case SPELL_SHOCK_OVERLOAD:
            if (uiEffIndex == 0)
            {
                if (pCreatureTarget->GetEntry() != NPC_THADDIUS || !pCreatureTarget->HasAura(SPELL_THADIUS_SPAWN))
                    return true;
                pCreatureTarget->RemoveAurasDueToSpell(SPELL_THADIUS_SPAWN);
                pCreatureTarget->CastSpell(pCreatureTarget, SPELL_THADIUS_LIGHTNING_VISUAL, false);
            }
            return true;
        case SPELL_THADIUS_LIGHTNING_VISUAL:
            if (uiEffIndex == 0 && pCreatureTarget->GetEntry() == NPC_THADDIUS)
            {
                if (ScriptedInstance* pInstance = (ScriptedInstance*)pCreatureTarget->GetInstanceData())
                {
                        pCreatureTarget->AI()->DoZoneInCombat();
                        pCreatureTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                }
            }
            return true;
    }
    return false;
}

/************
** npc_tesla_coil
************/

struct npc_tesla_coilAI : public Scripted_NoMovementAI
{
    npc_tesla_coilAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        SetupTimer = 1000;
        OverloadTimer = 0;
        bReapply = false;
        Reset();
    }

    ScriptedInstance* pInstance;
    bool bToFeugen;
    bool bReapply;

    uint32 SetupTimer;
    uint32 OverloadTimer;

    void Reset() {}

    void EnterCombat(Unit* who)
    {
        DoScriptText(-1533149, me);
    }

    void EnterEvadeMode()
    {
        me->DeleteThreatList();
        me->CombatStop();
    }

    bool SetupChain()
    {
        if (!pInstance || pInstance->GetData(DATA_THADDIUS) == DONE)
            return true;

        GameObject* pNoxTeslaFeugen  = FindGameObject(GO_CONS_NOX_TESLA_FEUGEN, 60, me);
        GameObject* pNoxTeslaStalagg = FindGameObject(GO_CONS_NOX_TESLA_STALAGG, 60, me);

        // Try again, till Tesla GOs are spawned
        if (!pNoxTeslaFeugen || !pNoxTeslaStalagg)
            return false;

        bToFeugen = me->GetDistanceOrder(pNoxTeslaFeugen, pNoxTeslaStalagg);
        DoCast(me, bToFeugen ? SPELL_FEUGEN_CHAIN : SPELL_STALAGG_CHAIN);
        return true;
    }

    void ReApplyChain(uint32 uiEntry)
    {
        if (uiEntry)
        {
            if ((uiEntry == NPC_FEUGEN && !bToFeugen) || (uiEntry == NPC_STALAGG && bToFeugen))
                return;

            bReapply = true;
        }
        else
        {
            bReapply = false;
            me->InterruptNonMeleeSpells(true);
            GameObject* pGo = FindGameObject(bToFeugen ? GO_CONS_NOX_TESLA_FEUGEN : GO_CONS_NOX_TESLA_STALAGG, 60, me);

            if (pGo && pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON && pGo->getLootState() == GO_ACTIVATED)
                pGo->ResetDoorOrButton();

            DoCast(me, bToFeugen ? SPELL_FEUGEN_CHAIN : SPELL_STALAGG_CHAIN);
        }
    }

    void SetOverloading()
    {
        OverloadTimer = 14000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (OverloadTimer)
        {
            if (OverloadTimer <=  diff)
            {
                OverloadTimer = 0;
                if (me->HasAura(SPELL_FEUGEN_CHAIN))
                {
                    me->RemoveAurasDueToSpell(SPELL_FEUGEN_TESLA_PASSIVE);
                    me->RemoveAurasDueToSpell(SPELL_FEUGEN_CHAIN);
                }
                if (me->HasAura(SPELL_STALAGG_CHAIN))
                {
                    me->RemoveAurasDueToSpell(SPELL_STALAGG_TESLA_PASSIVE);
                    me->RemoveAurasDueToSpell(SPELL_STALAGG_CHAIN);
                }
                if(Creature* Thaddius = (Creature*)FindCreature(NPC_THADDIUS, 150, me))
                {
                    me->InterruptNonMeleeSpells(false);
                    DoCast(Thaddius,  SPELL_SHOCK_OVERLOAD);
                }
                if (GameObject* pGo = FindGameObject(bToFeugen ? GO_CONS_NOX_TESLA_FEUGEN : GO_CONS_NOX_TESLA_STALAGG, 120, me))
                    pGo->UseDoorOrButton(bToFeugen ? GO_CONS_NOX_TESLA_FEUGEN : GO_CONS_NOX_TESLA_STALAGG);
            }
            else
                OverloadTimer -= diff;
        }
        me->getThreatManager().getHostilTarget();

        if (!OverloadTimer && !SetupTimer && !bReapply)
            return;

        if (SetupTimer)
        {
            if (SetupTimer <= diff)
            {
                if (SetupChain())
                    SetupTimer = 0;
                else
                    SetupTimer = 5000;
            }
            else
                SetupTimer -= diff;
        }

        if (bReapply)
            ReApplyChain(0);
    }
};

CreatureAI* GetAI_npc_tesla_coil(Creature* pCreature)
{
    return new npc_tesla_coilAI(pCreature);
}

struct boss_thaddiusAddsAI : public ScriptedAI
{
    boss_thaddiusAddsAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    bool bFakeDeath;
    bool bBothDead;

    uint32 HoldTimer;
    uint32 WarStompTimer;
    uint32 ReviveTimer;
    uint32 CheckTimer;

    void Reset()
    {
        bFakeDeath = false;
        bBothDead = false;

        ReviveTimer = 5 * 1000;
        HoldTimer = 2 * 1000;
        WarStompTimer = urand(8 * 1000, 10 * 1000);
        CheckTimer = 7000;

        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetHealth(me->GetMaxHealth());
        me->SetStandState(UNIT_STAND_STATE_STAND);
        if (!me->HasAura(SPELL_FEUGEN_CHAIN) || !me->HasAura(SPELL_STALAGG_CHAIN))
            ApplyChain();
    }

    bool FindPlayers()
    {
        std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
        if(m_threatlist.empty())
            return false;

        for(std::list<HostileReference*>::iterator itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
        {
            Unit* pUnit = Unit::GetUnit((*me), (*itr)->getUnitGuid());
            if(pUnit && pUnit->isAlive() && pUnit->IsInCombat() && me->canAttack(pUnit) && pUnit->IsWithinDistInMap(me, 150.0f))
                return true;
        }
        return false;
    }

    Creature* GetOtherAdd() // For Stalagg returns Feugen, for Feugen returns Stalagg
    {
        switch (me->GetEntry())
        {
            case NPC_FEUGEN:  return (Creature*)FindCreature(NPC_STALAGG, 150, me);
            case NPC_STALAGG: return (Creature*)FindCreature(NPC_FEUGEN, 150, me);
            default:
                return NULL;
        }
    }

    void EnterCombat(Unit* pWho)
    {
        if (!pInstance)
            return;
        pInstance->SetData(DATA_THADDIUS, IN_PROGRESS);
        DoZoneInCombat();
        if (Creature* pOtherAdd = GetOtherAdd())
        {
            if (!pOtherAdd->IsInCombat())
                pOtherAdd->AI()->AttackStart(pWho);
        }
        if(Creature* Thaddius = (Creature*)FindCreature(NPC_THADDIUS, 250, me))
        {
            if (!Thaddius->IsInCombat())
                Thaddius->SetInCombatWithZone();
        }
    }
    
    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void ApplyChain()
    {
        std::list<Creature*> lTeslaGdList = FindAllCreaturesWithEntry(NPC_TESLA_COIL,150);
        if (!pInstance)
            return;

        if (lTeslaGdList.empty())
            return;

        for (std::list<Creature*>::const_iterator itr = lTeslaGdList.begin(); itr != lTeslaGdList.end(); ++itr)
        {
            if (Creature* pTesla = Unit::GetCreature(*me, (*itr)->GetGUID()))
            {
                if (npc_tesla_coilAI* pTeslaAI = dynamic_cast<npc_tesla_coilAI*>(pTesla->AI()))
                    pTeslaAI->ReApplyChain(me->GetEntry());
            }
        }
    }

    void JustRespawned()
    {
        Reset();
    }

    void JustReachedHome()
    {
        if (!pInstance)
            return;

        if (Creature* pOther = GetOtherAdd())
        {
            if (boss_thaddiusAddsAI* pOtherAI = dynamic_cast<boss_thaddiusAddsAI*>(pOther->AI()))
            {
                if (pOtherAI->IsCountingDead())
                {
                    pOther->ForcedDespawn();
                    pOther->Respawn();
                }
            }
        }

        // Reapply Chains if needed
        if (!me->HasAura(SPELL_FEUGEN_CHAIN) && !me->HasAura(SPELL_STALAGG_CHAIN))
            JustRespawned();

        pInstance->SetData(DATA_THADDIUS, FAIL);
    }

    void Revive()
    {
        DoResetThreat();
        PauseCombatMovement();
        Reset();
    }

    bool IsCountingDead()
    {
        return bFakeDeath || me->isDead();
    }

    void PauseCombatMovement()
    {
        SetCombatMovement(false);
        HoldTimer = 1500;
    }

    virtual void UpdateAddAI(const uint32 /*diff*/) {}

    void UpdateAI(const uint32 diff)
    {
        if(CheckTimer < diff)
        {
            if(!FindPlayers())
            {
                EnterEvadeMode();
                return;
            }
            CheckTimer = 7000;
        }
        else
            CheckTimer -= diff;

        if (bBothDead)
            return;

        if (bFakeDeath)
        {
            if (ReviveTimer < diff)
            {
                if (Creature* pOther = GetOtherAdd())
                {
                    if (boss_thaddiusAddsAI* pOtherAI = dynamic_cast<boss_thaddiusAddsAI*>(pOther->AI()))
                    {
                        if (!pOtherAI->IsCountingDead()) // Raid was to slow to kill the second add
                            Revive();
                        else
                        {
                            bBothDead = true; // Now both adds are counting dead
                            pOtherAI->bBothDead = true;
                            // Set both Teslas to overload
                            std::list<Creature*> lTeslaGdList = FindAllCreaturesWithEntry(NPC_TESLA_COIL,150);
                            if (lTeslaGdList.empty())
                                return;
                            for (std::list<Creature*>::const_iterator itr = lTeslaGdList.begin(); itr != lTeslaGdList.end(); ++itr)
                            {
                                if (Creature* pTesla = Unit::GetCreature(*me, (*itr)->GetGUID()))
                                {
                                    if (npc_tesla_coilAI* pTeslaAI = dynamic_cast<npc_tesla_coilAI*>(pTesla->AI()))
                                        pTeslaAI->SetOverloading();
                                }
                            }
                            me->Kill(me);
                        }
                    }
                }
            }
            else
                ReviveTimer -= diff;
            return;
        }

        if (!me->getThreatManager().getHostilTarget() || !me->GetVictim())
            return;

        if (HoldTimer) // A short timer preventing combat movement after revive
        {
            if (HoldTimer <= diff)
            {
                SetCombatMovement(true);
                me->GetMotionMaster()->MoveChase(me->GetVictim());
                HoldTimer = 0;
            }
            else
                HoldTimer -= diff;
        }

        if (WarStompTimer < diff)
        {
            DoCast(me, SPELL_WARSTOMP);
            WarStompTimer = urand(8000, 10000);
        }
        else
            WarStompTimer -= diff;

        UpdateAddAI(diff); // For Add Specific Abilities

        DoMeleeAttackIfReady();
    }

    void DamageTaken(Unit* pKiller, uint32& damage)
    {
        if (damage < me->GetHealth())
            return;

        // Prevent glitch if in fake death
        if (bFakeDeath)
        {
            damage = 0;
            return;
        }

        // prevent death
        damage = 0;
        bFakeDeath = true;

        me->InterruptNonMeleeSpells(false);
        me->SetHealth(0);
        me->StopMoving();
        me->ClearComboPointHolders();
        me->RemoveAllAurasOnDeath();
        me->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        me->ModifyAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, false);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->ClearAllReactives();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        me->SetStandState(UNIT_STAND_STATE_DEAD);

        JustDied(pKiller);                                  // Texts
    }
};

struct boss_stalaggAI : public boss_thaddiusAddsAI
{
    boss_stalaggAI(Creature *c) : boss_thaddiusAddsAI(c) {}
    uint32 m_uiPowerSurgeTimer;
    uint32 m_uiMagneticPullTimer;

    void Reset()
    {
        boss_thaddiusAddsAI::Reset();
        m_uiPowerSurgeTimer = urand(10000, 15000);
        m_uiMagneticPullTimer = 20600;
    }

    void EnterCombat(Unit* pWho)
    {
        DoScriptText(SAY_STAL_AGGRO, me);
        boss_thaddiusAddsAI::EnterCombat(pWho);
    }

    void KilledUnit(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_STAL_SLAY, me);
    }

    void JustDied(Unit *pKiller)
    {
        DoScriptText(SAY_STAL_DEATH, me);
    }

    void UpdateAddAI(const uint32 diff)
    {
        if (m_uiPowerSurgeTimer < diff)
        {
            DoCast(m_creature, SPELL_POWERSURGE);
            m_uiPowerSurgeTimer = urand(10000, 15000);
        }
        else
            m_uiPowerSurgeTimer -= diff;

        if(m_uiMagneticPullTimer < diff)
        {
            if(Creature* Feugen = (Creature*)FindCreature(NPC_FEUGEN, 150, me))
                if(Unit* FeugenTarget = Feugen->GetVictim())
                {
                    FeugenTarget->KnockBackFrom(me, -FeugenTarget->GetDistance2d(me), 10);
                    Feugen->getThreatManager().modifyThreatPercent(FeugenTarget, -100);
                    DoCast(FeugenTarget, SPELL_MAGNETIC_PULL);
                }
            m_uiMagneticPullTimer = 20600;
        } else m_uiMagneticPullTimer -= diff;
    }
};

CreatureAI* GetAI_mob_stalagg(Creature *_Creature)
{
    return new boss_stalaggAI(_Creature);
}

struct boss_feugenAI : public boss_thaddiusAddsAI
{
    boss_feugenAI(Creature *c): boss_thaddiusAddsAI(c) {}

    uint32 m_uiStaticFieldTimer;
    uint32 m_uiMagneticPullTimer;                           // TODO, missing
    void Reset()
    {
        boss_thaddiusAddsAI::Reset();
        m_uiStaticFieldTimer = urand(10000, 15000);
        m_uiMagneticPullTimer = 20600;
    }

    void EnterCombat(Unit* pWho)
    {
        DoScriptText(SAY_FEUG_AGGRO, me);
        boss_thaddiusAddsAI::EnterCombat(pWho);
    }

    void KilledUnit(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_FEUG_SLAY, me);
    }

    void JustDied(Unit *pKiller)
    {
        DoScriptText(SAY_FEUG_DEATH, me);
    }

    void UpdateAddAI(const uint32 diff)
    {
        if (m_uiStaticFieldTimer < diff)
        {
            DoCast(m_creature, SPELL_STATIC_FIELD);
            m_uiStaticFieldTimer = urand(10000, 15000);
        }
        else
            m_uiStaticFieldTimer -= diff;

        if(m_uiMagneticPullTimer < diff)
        {
            if(Creature* Stalagg = (Creature*)FindCreature(NPC_STALAGG, 150, me))
                if(Unit* StaloggTarget = Stalagg->GetVictim())
                {
                    StaloggTarget->KnockBackFrom(me, -StaloggTarget->GetDistance2d(me), 10);
                    Stalagg->getThreatManager().modifyThreatPercent(StaloggTarget, -100);
                    DoCast(StaloggTarget, SPELL_MAGNETIC_PULL);
                }
            m_uiMagneticPullTimer = 20600;
        } else m_uiMagneticPullTimer -= diff;
    }
};

CreatureAI* GetAI_mob_feugen(Creature *_Creature)
{
    return new boss_feugenAI(_Creature);
}

void AddSC_boss_thaddius()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_thaddius";
    newscript->GetAI = &GetAI_boss_thaddius;
    newscript->pEffectDummyNPC = &EffectDummyNPC_spell_thaddius_encounter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_feugen";
    newscript->GetAI = &GetAI_mob_feugen;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_stalagg";
    newscript->GetAI = &GetAI_mob_stalagg;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tesla_coil";
    newscript->GetAI = &GetAI_npc_tesla_coil;
    newscript->pEffectDummyNPC = &EffectDummyNPC_spell_thaddius_encounter;
    newscript->RegisterSelf();
}
