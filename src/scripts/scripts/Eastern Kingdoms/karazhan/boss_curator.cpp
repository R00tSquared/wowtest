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
SDName: Boss_Curator
SD%Complete: 100
SDComment: Evocation may cause client crash (Core related)
SDCategory: Karazhan
EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

#define SAY_AGGRO                       -1532057
#define SAY_SUMMON1                     -1532058
#define SAY_SUMMON2                     -1532059
#define SAY_EVOCATE                     -1532060
#define SAY_ENRAGE                      -1532061
#define SAY_KILL1                       -1532062
#define SAY_KILL2                       -1532063
#define SAY_DEATH                       -1532064

//Flare spell info
#define SPELL_ASTRAL_FLARE_PASSIVE      30234               // Visual effect + Flare damage
#define SPELL_ASTRAL_FLARE_NE           30236
#define SPELL_ASTRAL_FLARE_NW           30239
#define SPELL_ASTRAL_FLARE_SE           30240
#define SPELL_ASTRAL_FLARE_SW           30241

//Curator spell info
#define SPELL_HATEFUL_BOLT              30383
#define SPELL_EVOCATION                 30254
#define SPELL_ENRAGE                    30403               // Arcane Infusion: Transforms Curator and adds damage.
#define SPELL_BERSERK                   30407               // Arcane Deconstruction

struct boss_curatorAI : public ScriptedAI
{
    boss_curatorAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
        me->GetPosition(wLoc);
    }

    ScriptedInstance* pInstance;

    Timer addTimer;
    Timer hatefulBoltTimer;
    Timer berserkTimer;

    WorldLocation wLoc;

    bool enraged;
    bool evocating;

    void Reset()
    {
        addTimer.Reset(10000);
        hatefulBoltTimer.Reset(urand(10000, 12000));
        berserkTimer.Reset(720000);                     // 12 minutes
        enraged = false;
        evocating = false;
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_PERIODIC_LEECH, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_PERIODIC_MANA_LEECH, true);

        if(pInstance)
            pInstance->SetData(DATA_CURATOR_EVENT, NOT_STARTED);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_KILL1, SAY_KILL2), me);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);
        if(pInstance)
            pInstance->SetData(DATA_CURATOR_EVENT, DONE);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, me);
        if(pInstance)
            pInstance->SetData(DATA_CURATOR_EVENT, IN_PROGRESS);
    }

    void OnAuraRemove(Aura * aur, bool removeStack)
    {
        if (aur->GetId() == SPELL_EVOCATION)
        {
            evocating = false;
            me->SetRooted(false);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_COMBAT_N_EVADE, 135.0f);

        if (!evocating)
        {
            if(me->GetPower(POWER_MANA) <= me->GetMaxPower(POWER_MANA) * 0.15f)
            {
                evocating = true;
                ForceSpellCastWithScriptText(SPELL_EVOCATION, CAST_SELF, SAY_EVOCATE);
                me->SetRooted(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
            }
        }

        if (!enraged && !evocating)
        {
            if (addTimer.Expired(diff))
            {
                //Summon Astral Flare
                Creature* astralFlare = DoSpawnCreature(17096, rand()%37, rand()%37, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);

                if (astralFlare && target)
                {
                    astralFlare->CastSpell(astralFlare, SPELL_ASTRAL_FLARE_PASSIVE, false);
                    astralFlare->AI()->AttackStart(target);
                }

                // Reduce Mana by 10%
                int32 mana = (int32)(0.1f*(me->GetMaxPower(POWER_MANA)));
                me->ModifyPower(POWER_MANA, -mana);

                DoScriptText(RAND(SAY_SUMMON1, SAY_SUMMON2, 0, 0), me);

                addTimer = 10000;
            }
        }

        if (!evocating && hatefulBoltTimer.Expired(diff))
        {
            if (Unit* target1 = SelectUnit(SELECT_TARGET_TOPAGGRO, 1, GetSpellMaxRange(SPELL_HATEFUL_BOLT)))
            {
                Unit* target2 = SelectUnit(SELECT_TARGET_TOPAGGRO, 2, GetSpellMaxRange(SPELL_HATEFUL_BOLT));
                bool tar2MostHp = target2 ? target2->GetHealth() > target1->GetHealth() : false;
                AddSpellToCast(tar2MostHp ? target2 : target1, SPELL_HATEFUL_BOLT, false, true);
            }
            hatefulBoltTimer = enraged ? 7000 : 15000;
        }

        if (!enraged && HealthBelowPct(15))
        {
            enraged = true;
            me->RemoveAurasDueToSpell(SPELL_EVOCATION);
            ForceSpellCastWithScriptText(SPELL_ENRAGE, CAST_SELF, SAY_ENRAGE);
        }

        if (berserkTimer.Expired(diff))
        {
            me->RemoveAurasDueToSpell(SPELL_EVOCATION);
            ForceSpellCastWithScriptText(SPELL_BERSERK, CAST_SELF, SAY_ENRAGE);
            berserkTimer = 60000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_curator(Creature *_Creature)
{
    return new boss_curatorAI (_Creature);
}

void AddSC_boss_curator()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_curator";
    newscript->GetAI = &GetAI_boss_curator;
    newscript->RegisterSelf();
}
