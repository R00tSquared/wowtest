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
SDName: Boss_Buru
SD%Complete: 99
SDComment: Need to set instance data. But can't at the moment: while we set IN_PROGRESS eggs can't be respawned in combat
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_ruins_of_ahnqiraj.h"

enum
{
    EMOTE_TARGET            = -1509002,

    // boss spells
    SPELL_CREEPING_PLAGUE   = 20512,
    SPELL_DISMEMBER         = 96,
    SPELL_GATHERING_SPEED   = 1834,
    SPELL_FULL_SPEED        = 1557,
    SPELL_THORNS            = 25640,
    SPELL_BURU_TRANSFORM    = 24721,

    // egg spells
    SPELL_SUMMON_HATCHLING  = 1881,
    SPELL_EXPLODE           = 19593,
    SPELL_BURU_EGG_TRIGGER  = 26646,

    // npcs
    NPC_BURU_EGG            = 15514,
    NPC_HATCHLING           = 15521,
    NPC_BURU_EGG_TRIGGER    = 15964,

    PHASE_EGG               = 1,
    PHASE_TRANSFORM         = 2,
};

struct boss_buruAI : public ScriptedAI
{
    boss_buruAI(Creature* pCreature) : ScriptedAI(pCreature)
    { pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData(); }

    instance_ruins_of_ahnqiraj* pInstance;

    uint8 Phase;
    Timer_UnCheked DismemberTimer;
    Timer_UnCheked CreepingPlagueTimer;
    Timer_UnCheked GatheringSpeedTimer;
    Timer_UnCheked FullSpeedTimer;
    Timer_UnCheked EggRootTimer;

    void Reset()
    {
        DismemberTimer.Reset(5000);
        GatheringSpeedTimer.Reset(9000);
        CreepingPlagueTimer.Reset(0);
        FullSpeedTimer.Reset(60000);
        EggRootTimer.Reset(0);
        Phase = PHASE_EGG;
        
        std::list<Creature*> EggsList = FindAllCreaturesWithEntry(NPC_BURU_EGG, 150);
        for (std::list<Creature*>::iterator itr = EggsList.begin(); itr != EggsList.end(); ++itr)
        {
            if (!(*itr)->isAlive())
                (*itr)->Respawn();
            else if ((*itr)->isAlive())
            {
                me->Kill((*itr));
                (*itr)->Respawn();
            }
            (*itr)->setFaction(me->getFaction());
            if ((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
                (*itr)->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE, false);
            if((*itr)->GetVisibility() == VISIBILITY_OFF)
                (*itr)->SetVisibility(VISIBILITY_ON);
        }

        std::list<Creature*> EggsTriggerList = FindAllCreaturesWithEntry(NPC_BURU_EGG_TRIGGER, 150);
        for (std::list<Creature*>::iterator itr = EggsTriggerList.begin(); itr != EggsTriggerList.end(); ++itr)
            me->Kill((*itr));

        me->SetRooted(false);

        if(pInstance)
            pInstance->SetData(DATA_BURU_THE_GORGER, NOT_STARTED);
    }

    void EnterEvadeMode()
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        CreatureAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* pWho)
    {
        DoScriptText(EMOTE_TARGET, me, pWho);
        DoCast(me, SPELL_THORNS, true);
        me->AddThreat(pWho, 1000000.0f);
        me->SetPlayerDamageReqInPct(0.15);
        /*if(pInstance)
            pInstance->SetData(DATA_BURU_THE_GORGER, IN_PROGRESS);*/
    }
    
    void JustDied(Unit* who)
    {
        if(pInstance)
            pInstance->SetData(DATA_BURU_THE_GORGER, DONE);
    }

    void KilledUnit(Unit* pVictim)
    {
        // Attack a new random target when a player is killed
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            DoAttackNewTarget();
    }

    // Wrapper to attack a new target and remove the speed gathering buff
    void DoAttackNewTarget()
    {
        if (Phase == PHASE_TRANSFORM)
            return;

        me->RemoveAurasDueToSpell(SPELL_FULL_SPEED);
        me->RemoveAurasDueToSpell(SPELL_GATHERING_SPEED);

        if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM,0))
        {
            DoResetThreat();
            me->AddThreat(pTarget, 1000000.0f);
            DoScriptText(EMOTE_TARGET, me, pTarget);
        }

        FullSpeedTimer = 60000;
        GatheringSpeedTimer = 9000;
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_EXPLODE)
        {
            me->SetRooted(true);
            me->SetFacingToObject(caster);
            EggRootTimer = 2000;
            DoAttackNewTarget();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        switch (Phase)
        {
            case PHASE_EGG:
                if(EggRootTimer.GetInterval())
                {
                    if (EggRootTimer.Expired(diff))
                    {
                        me->SetRooted(false);
                        EggRootTimer = 0;
                    }
                }

                if (DismemberTimer.Expired(diff))
                {
                    AddSpellToCast(SPELL_DISMEMBER, CAST_TANK);
                    DismemberTimer = 5000;
                }

                if (FullSpeedTimer.GetInterval())
                {
                    if (GatheringSpeedTimer.Expired(diff))
                    {
                        AddSpellToCast(SPELL_GATHERING_SPEED, CAST_SELF);
                        GatheringSpeedTimer = 9000;
                    }

                    if (FullSpeedTimer.Expired(diff))
                    {
                        AddSpellToCast(SPELL_FULL_SPEED, CAST_SELF);
                        FullSpeedTimer = 0;
                    }
                }

                if (me->GetHealthPercent() < 20.0f)
                {
                    AddSpellToCast(SPELL_BURU_TRANSFORM, CAST_SELF);
                    AddSpellToCast(SPELL_FULL_SPEED, CAST_SELF);
                    me->RemoveAurasDueToSpell(SPELL_THORNS);
                    DoResetThreat();
                    Phase = PHASE_TRANSFORM;
                    std::list<Creature*> EggsList = FindAllCreaturesWithEntry(NPC_BURU_EGG, 150);
                    for (std::list<Creature*>::iterator itr = EggsList.begin(); itr != EggsList.end(); ++itr)
                    {
                        if (!(*itr)->isAlive())
                            (*itr)->Respawn();
                        (*itr)->setFaction(35);
                        (*itr)->SetVisibility(VISIBILITY_OFF);
                        (*itr)->SetNonAttackableFlag(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    }
                    std::list<Creature*> EggsTriggerList = FindAllCreaturesWithEntry(NPC_BURU_EGG_TRIGGER, 150);
                    for (std::list<Creature*>::iterator itr = EggsTriggerList.begin(); itr != EggsTriggerList.end(); ++itr)
                        me->Kill((*itr));
                }

                break;
            case PHASE_TRANSFORM:
                if(EggRootTimer.GetInterval())
                {
                    if (EggRootTimer.Expired(diff))
                    {
                        me->SetRooted(false);
                        EggRootTimer = 0;
                    }
                }

                if (CreepingPlagueTimer.Expired(diff))
                {
                    AddSpellToCast(SPELL_CREEPING_PLAGUE, CAST_SELF);
                    CreepingPlagueTimer = 6000;
                }

                break;
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_buru(Creature* pCreature)
{
    return new boss_buruAI(pCreature);
}

struct npc_buru_eggAI : public Scripted_NoMovementAI
{
    npc_buru_eggAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    void Reset()
    { }

    void EnterCombat(Unit* pAttacker)
    {
        if (Creature* pBuru = Unit::GetCreature(*me, pInstance->GetData64(DATA_BURU_THE_GORGER)))
            if (!pBuru->IsInCombat())
                pBuru->AI()->AttackStart(pAttacker);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_HATCHLING)
        {
            if (pInstance)
            {
                if (Creature* pBuru = Unit::GetCreature(*me, pInstance->GetData64(DATA_BURU_THE_GORGER)))
                {
                    if (Unit* pTarget = pBuru->AI()->SelectUnit(SELECT_TARGET_RANDOM, 0))
                        pSummoned->AI()->AttackStart(pTarget);
                }
            }
            
        }
        else if(pSummoned->GetEntry() == NPC_BURU_EGG_TRIGGER)
        {
            pSummoned->setFaction(35);
            pSummoned->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            pSummoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            pSummoned->CastSpell(me, SPELL_BURU_EGG_TRIGGER, false);
        }
    }

    void JustDied(Unit* pKiller)
    {
        if(pKiller->GetTypeId() != TYPEID_UNIT || (pKiller->GetTypeId() == TYPEID_UNIT && pKiller->GetEntry() != 15370))
        {
            // Explode and Summon hatchling
            DoCast(me, SPELL_EXPLODE, true);
            DoCast(me, SPELL_SUMMON_HATCHLING, true);
            me->SummonCreature(NPC_BURU_EGG_TRIGGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
        }
    }

    void UpdateAI(const uint32 ) { }
};

CreatureAI* GetAI_npc_buru_egg(Creature* pCreature)
{
    return new npc_buru_eggAI(pCreature);
}

void AddSC_boss_buru()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_buru";
    pNewScript->GetAI = &GetAI_boss_buru;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_buru_egg";
    pNewScript->GetAI = &GetAI_npc_buru_egg;
    pNewScript->RegisterSelf();
}
