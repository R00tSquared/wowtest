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
SDName: Boss_Razorgore
SD%Complete: 90
SDComment: Grethok need AI
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

enum
{
    SAY_EGGS_BROKEN_1           = -1469022,
    SAY_EGGS_BROKEN_2           = -1469023,
    SAY_EGGS_BROKEN_3           = -1469024,
    SAY_DEATH                   = -1469025,

    // phase I event spells 
    SPELL_POSSESS               = 23014,                    // visual effect and increase the damage taken
    SPELL_DESTROY_EGG           = 19873,
    SPELL_EXPLODE_ORB           = 20037,                    // used if attacked without destroying the eggs - related to 20038

    SPELL_CLEAVE                = 19632,
    SPELL_WARSTOMP              = 24375,
    SPELL_FIREBALL_VOLLEY       = 22425,
    SPELL_CONFLAGRATION         = 23023,
};

struct boss_razorgoreAI : public ScriptedAI
{
    boss_razorgoreAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    uint32 Cleave_Timer;
    uint32 WarStomp_Timer;
    uint32 FireballVolley_Timer;
    uint32 Conflagration_Timer;
    uint32 IntroVisualTimer;
    uint32 AttackResume;
    uint32 CheckPossesCasterTimer;
    uint32 RemoveAuraTimer;
    uint64 PossessCasterGUID;

    bool EggsExploded;

    void Reset()
    {
        Cleave_Timer            = urand(4000, 8000);
        FireballVolley_Timer    = urand(15000, 20000);
        Conflagration_Timer     = urand(10000, 15000);
        WarStomp_Timer          = 35000;
        IntroVisualTimer        = 1000;
        AttackResume            = 0;
        EggsExploded            = false;
        PossessCasterGUID       = 0;
        CheckPossesCasterTimer  = 0;
        RemoveAuraTimer         = 0;
    }

    void SpellHit(Unit* caster, const SpellEntry* info) 
    {
        if (info && info->Id == 19832) // possess
        {
            PossessCasterGUID = caster->GetGUID();
            CheckPossesCasterTimer = 1500;
        }
    }

    void EnterCombat(Unit*)
    {
        AttackResume = 1; // start chase movement
    }

    void OnCharmed(bool apply)
    {
        if(apply)
        {
            AttackResume = 1000;
            me->GetMotionMaster()->Clear();
            if(pInstance)
            {
                if(pInstance->GetData(DATA_RAZORGORE) == NOT_STARTED)
                    pInstance->SetData(DATA_RAZORGORE, IN_PROGRESS);
                me->SetInCombatWithZone();
                DoZoneInCombat();
            }
        }
    }

    void JustReachedHome()
    {
        if (pInstance)
            pInstance->SetData(DATA_RAZORGORE, FAIL);
        Reset();
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
        if (pInstance)
        {
            // Don't set instance data unless all eggs are destroyed
            if (pInstance->GetData(DATA_RAZORGORE) != SPECIAL)
                return;

            pInstance->SetData(DATA_RAZORGORE, DONE);
        }
    }

    void DamageTaken(Unit* /*pDoneBy*/, uint32& damage)
    {
        if (damage < me->GetHealth())
            return;

        if (!pInstance)
            return;

        // Don't allow any accident
        if (EggsExploded)
        {
            damage = 0;
            return;
        }

        // Boss explodes everything and resets - this happens if not all eggs are destroyed
        if (pInstance->GetData(DATA_RAZORGORE) != SPECIAL)
        {
            damage = 0;
            EggsExploded = true;
            pInstance->SetData(DATA_RAZORGORE, FAIL);
            DoCast(me, SPELL_EXPLODE_ORB, true);
            me->ForcedDespawn();
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        // Defenders should attack the players and the boss
        pSummoned->SetInCombatWithZone();
        if(PossessCasterGUID)
        {
            if (Unit* target = Unit::GetUnit((*me), PossessCasterGUID))
                pSummoned->AI()->AttackStart(target);
        }
        else if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(target);

    }

    void KilledUnit(Unit* victim)
    {
        if (me->isPossessed())
        {
            // We killed a creature, disable victim's loot
            if (victim->GetTypeId() == TYPEID_UNIT)
                victim->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }
    }

    void AttackStart(Unit *target)
    {
        if (!target)
            return;
        
        me->Attack(target, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (me->isPossessed())
        {
            if (me->GetVictim())
            {
                if (!me->canAttack(me->GetVictim()))
                    me->AttackStop();
                else
                    DoMeleeAttackIfReady();
            }

            if(CheckPossesCasterTimer)
            {
                if(CheckPossesCasterTimer <= diff)
                {
                    if(PossessCasterGUID)
                    {
                        if (Unit* target = Unit::GetUnit((*me), PossessCasterGUID))
                        {
                            if(!target->isAlive())
                            {
                                RemoveAuraTimer = 500;
                                PossessCasterGUID = 0;
                                CheckPossesCasterTimer = 0;
                                if (GameObject *pOrb = FindGameObject(GO_ORB_OF_DOMINATION, 200, me))
                                    pOrb->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                            }
                        }
                    }
                    CheckPossesCasterTimer = 1000;
                } else CheckPossesCasterTimer -= diff;
            }
            if(RemoveAuraTimer)
            {
                if(RemoveAuraTimer <= diff)
                {
                    me->RemoveAurasDueToSpell(19832);
                    AttackResume = 500;
                    RemoveAuraTimer = 0;
                } else RemoveAuraTimer -= diff;
            }
        }
        else
        {
            if(AttackResume)
            {
                if(AttackResume <= diff)
                {
                    if(PossessCasterGUID)
                    {
                        if (Unit* target = Unit::GetUnit((*me), PossessCasterGUID))
                        {
                            if(target->isAlive())
                            {
                                AttackStart(target);
                                me->GetMotionMaster()->MoveChase(target);
                                AttackResume = 0;
                            } else AttackResume = 500;
                        }
                    }
                    else if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    {
                        AttackStart(target);
                        me->GetMotionMaster()->MoveChase(target);
                        AttackResume = 0;
                    }
                } else AttackResume -= diff;
            }

            if (!UpdateVictim())
            {
                if (IntroVisualTimer)
                {
                    if (IntroVisualTimer <= diff)
                    {
                        if (Creature* pOrbTrigger = me->GetMap()->GetCreature(me->GetMap()->GetCreatureGUID(NPC_BLACKWING_ORB_TRIGGER)))
                            pOrbTrigger->CastSpell(me, SPELL_POSSESS, false);
                        IntroVisualTimer = 0;
                    }
                    else
                        IntroVisualTimer -= diff;
                }        
                return;
            }

            if (Cleave_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_CLEAVE);
                Cleave_Timer = urand(4000, 8000);
            } else Cleave_Timer -= diff;

            if (WarStomp_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_WARSTOMP);
                WarStomp_Timer = 30000;
            } else WarStomp_Timer -= diff;

            if (FireballVolley_Timer <= diff)
            {
                DoCast(me->GetVictim(), SPELL_FIREBALL_VOLLEY);
                FireballVolley_Timer = urand(15000, 20000);
            } else FireballVolley_Timer -= diff;

            if (Conflagration_Timer <= diff)
            {
                DoCast(me->GetVictim(),SPELL_CONFLAGRATION);
                Conflagration_Timer = urand(15000, 25000);
            } else Conflagration_Timer -= diff;

            /* This is obsolete code, not working anymore, keep as reference, should be handled in core though
            // Aura Check. If the player is affected by conflagration we attack a random player.
            if (me->GetVictim()->HasAura(SPELL_CONFLAGRATION, 0))
            {
               if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1))
                   me->TauntApply(pTarget);
            } */

            DoMeleeAttackIfReady();
            CastNextSpellIfAnyAndReady();
        }
    }
};

CreatureAI* GetAI_boss_razorgore(Creature *_Creature)
{
    return new boss_razorgoreAI (_Creature);
}

bool GOUse_go_black_dragon_egg(Player *player, GameObject* go)
{
    if (ScriptedInstance* pInstance = (ScriptedInstance*)go->GetInstanceData())
    {
        Creature* pRazorgore = pInstance->GetCreatureById(NPC_RAZORGORE);
        if(pRazorgore)
        {
            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_EGGS_BROKEN_1, pRazorgore); break;
                case 1: DoScriptText(SAY_EGGS_BROKEN_2, pRazorgore); break;
                case 2: DoScriptText(SAY_EGGS_BROKEN_3, pRazorgore); break;
            }
        }

        // Store the eggs which are destroyed, in order to count them for the second phase
        pInstance->SetData64(DATA_DRAGON_EGG, go->GetGUID());
        go->SetGoState(GO_STATE_ACTIVE);
    }

    return true;
}

void AddSC_boss_razorgore()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_razorgore";
    newscript->GetAI = &GetAI_boss_razorgore;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_black_dragon_egg";
    newscript->pGOUse = &GOUse_go_black_dragon_egg;
    newscript->RegisterSelf();
}


