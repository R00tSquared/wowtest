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
SDName: Boss_Grobbulus
SD%Complete: 90
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum
{
    EMOTE_SPRAY_SLIME               = -1533220,
    EMOTE_INJECTION                 = -1533221,

    SPELL_SLIME_STREAM              = 28137,
    SPELL_MUTATING_INJECTION        = 28169,
    SPELL_POISON_CLOUD              = 28240,
    SPELL_SLIME_SPRAY               = 28157,
    SPELL_BERSERK                   = 26662,
    SPELL_POISON                    = 28158,

    NPC_FALLOUT_SLIME               = 16290
};

struct boss_grobbulusAI : public ScriptedAI
{
    boss_grobbulusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance * pInstance;

    uint32 InjectionTimer;
    uint32 PoisonCloudTimer;
    uint32 SlimeSprayTimer;
    uint32 BerserkTimeSecs;
    uint32 BerserkTimer;
    uint32 SlimeStreamTimer;

    void Reset()
    {
        ClearCastQueue();
        InjectionTimer = 12000;
        PoisonCloudTimer = urand(20000, 25000);
        SlimeSprayTimer = urand(20000, 30000);
        BerserkTimeSecs = 12 * MINUTE;
        BerserkTimer = BerserkTimeSecs * MILLISECONDS;
        SlimeStreamTimer = 5000;         // The first few secs it is ok to be out of range
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        if (pInstance)
            pInstance->SetData(DATA_GROBBULUS, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (pInstance)
            pInstance->SetData(DATA_GROBBULUS, DONE);
    }

    void JustReachedHome()
    {
        if (pInstance)
            pInstance->SetData(DATA_GROBBULUS, FAIL);
    }

    // This custom selecting function, because we only want to select players without mutagen aura
    bool DoCastMutagenInjection()
    {
        if (me->IsNonMeleeSpellCast(true))
            return false;

        std::vector<Unit*> suitableTargets;
        std::list<HostileReference*>& threatList = me->getThreatManager().getThreatList();

        for(std::list<HostileReference*>::iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
        {
            if (Unit* pTarget = Unit::GetUnit((*me), (*itr)->getUnitGuid()))
            {
                if (pTarget->GetTypeId() == TYPEID_PLAYER && !pTarget->HasAura(SPELL_MUTATING_INJECTION))
                    suitableTargets.push_back(pTarget);
            }
        }

        if (suitableTargets.empty())
            return false;

        Unit* pTarget = suitableTargets[urand(0, suitableTargets.size() - 1)];
        if(pTarget)
        {
            AddSpellToCast(pTarget, SPELL_MUTATING_INJECTION, true, true);
            DoScriptText(EMOTE_INJECTION, me, pTarget);
            return true;
        }
        else
            return false;
        
    }

    void SpellHitTarget(Unit* pTarget, const SpellEntry* pSpell)
    {
        if ((pSpell->Id == SPELL_SLIME_SPRAY) && pTarget->GetTypeId() == TYPEID_PLAYER)
            me->SummonCreature(NPC_FALLOUT_SLIME, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 10000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // Slime Stream
        if (!SlimeStreamTimer)
        {
            if (!me->IsWithinMeleeRange(me->GetVictim()))
                SlimeStreamTimer = 3000;
        }
        else
        {
            if (SlimeStreamTimer < diff)
            {
                if (!me->IsWithinMeleeRange(me->GetVictim()))
                {
                    AddSpellToCast(SPELL_SLIME_STREAM, CAST_SELF);
                    // Give some time, to re-reach grobbulus
                    SlimeStreamTimer = 3000;
                } else SlimeStreamTimer = 0;
            }
            else
                SlimeStreamTimer -= diff;
        }

        // Berserk
        if (BerserkTimer)
        {
            if (BerserkTimer <= diff)
            {
                AddSpellToCast(SPELL_BERSERK, CAST_SELF);
                BerserkTimer = 0;
            }
            else
                BerserkTimer -= diff;
        }

        // SlimeSpray
        if (SlimeSprayTimer < diff)
        {
            AddSpellToCast(SPELL_SLIME_SPRAY, CAST_TANK);
            SlimeSprayTimer = urand(30000, 60000);
            DoScriptText(EMOTE_SPRAY_SLIME, me);
        }
        else
            SlimeSprayTimer -= diff;

        // Mutagen Injection
        if (InjectionTimer < diff)
        {
            if (DoCastMutagenInjection())
                InjectionTimer = urand(10000, 13000) -  5 * (BerserkTimeSecs * 1000 - BerserkTimer) / BerserkTimeSecs;
        }
        else
            InjectionTimer -= diff;

        // Poison Cloud
        if (PoisonCloudTimer < diff)
        {
            AddSpellToCast(SPELL_POISON_CLOUD, CAST_NULL);
            PoisonCloudTimer = 15000;
        }
        else
            PoisonCloudTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_grobbulus(Creature *_Creature)
{
    return new boss_grobbulusAI (_Creature);
}

struct mob_grobbulus_cloudAI : public Scripted_NoMovementAI
{
    mob_grobbulus_cloudAI(Creature *c) : Scripted_NoMovementAI(c) {}

    uint32 PoisonTimer;

    void Reset()
    {
        PoisonTimer = 1000;
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
        SetCombatMovement(false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(PoisonTimer)
        {
            if(PoisonTimer < diff)
            {
                DoCast(me, SPELL_POISON, true);
                PoisonTimer = 0;
            }
            else PoisonTimer -= diff;
        }
    }
};

CreatureAI* GetAI_mob_grobbulus_cloud(Creature *_Creature)
{
    return new mob_grobbulus_cloudAI(_Creature);
}

void AddSC_boss_grobbulus()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_grobbulus";
    newscript->GetAI = &GetAI_boss_grobbulus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_16363";
    newscript->GetAI = &GetAI_mob_grobbulus_cloud;
    newscript->RegisterSelf();
}
