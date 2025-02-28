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
SDName: Boss_Faerlina
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum FaerlinaTexts
{
    SAY_GREET                   = -1533009,
    SAY_AGGRO1                  = -1533010,
    SAY_AGGRO2                  = -1533011,
    SAY_AGGRO3                  = -1533012,
    SAY_AGGRO4                  = -1533013,
    SAY_SLAY1                   = -1533014,
    SAY_SLAY2                   = -1533015,
    SAY_DEATH                   = -1533016,
    EMOTE_BOSS_GENERIC_FRENZY   = -1000384,
};

enum FaerlinaSpells
{
    SPELL_POISONBOLT_VOLLEY     = 28796,
    SPELL_RAIN_OF_FIRE          = 28794,
    SPELL_WIDOWS_EMBRACE        = 28732,
    SPELL_ENRAGE                = 28798
};

enum FaerlinaEvents
{
    EVENT_POISONBOLT_VOLLEY     = 1,
    EVENT_RAIN_OF_FIRE          = 2,
    EVENT_ENRAGE                = 3
};

struct boss_faerlinaAI : public BossAI
{
    boss_faerlinaAI(Creature *c) : BossAI(c, DATA_GRAND_WIDOW_FAERLINA) { }

    uint32 PoisonBoltVolley_Timer;
    uint32 RainOfFire_Timer;
    uint32 Enrage_Timer;
    bool greet;

    void Reset()
    {
        events.Reset();
        events.ScheduleEvent(EVENT_POISONBOLT_VOLLEY, 8000);
        events.ScheduleEvent(EVENT_RAIN_OF_FIRE, 16000);
        events.ScheduleEvent(EVENT_ENRAGE, 60000);
        greet = false;

        if(instance)
            instance->SetData(DATA_GRAND_WIDOW_FAERLINA, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3, SAY_AGGRO4), me);
        if(instance)
            instance->SetData(DATA_GRAND_WIDOW_FAERLINA, IN_PROGRESS);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!greet && who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(who, 80.0f))
        {
            DoScriptText(SAY_GREET, me);
            greet = true;
        }
        ScriptedAI::MoveInLineOfSight(who);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
        if(instance)
            instance->SetData(DATA_GRAND_WIDOW_FAERLINA, DONE);
    }

    void SpellHit(Unit* caster, const SpellEntry* Spell) override
    {
        // Check if we hit with Widow's Embrave
        if (Spell->Id == SPELL_WIDOWS_EMBRACE)
        {
            bool bIsFrenzyRemove = false;

            // If we remove the Frenzy, the Enrage Timer_UnCheked is reseted to 60s
            if (me->HasAura(SPELL_ENRAGE, 0))
            {
                events.ScheduleEvent(EVENT_ENRAGE, 60000);
                me->RemoveAurasDueToSpell(SPELL_ENRAGE);
                bIsFrenzyRemove = true;
            }
            else events.ScheduleEvent(EVENT_ENRAGE, 30000);
            events.ScheduleEvent(EVENT_POISONBOLT_VOLLEY, urand(32000, 38000));
            me->Kill(caster);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 120.0f);

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_POISONBOLT_VOLLEY:
                {
                    AddSpellToCast(SPELL_POISONBOLT_VOLLEY, CAST_NULL);
                    events.ScheduleEvent(EVENT_POISONBOLT_VOLLEY, 11000);
                    break;
                }
                case EVENT_RAIN_OF_FIRE:
                {
                    AddSpellToCast(SPELL_RAIN_OF_FIRE, CAST_RANDOM);
                    events.ScheduleEvent(EVENT_RAIN_OF_FIRE, 16000);
                    break;
                }
                case EVENT_ENRAGE:
                {
                    AddSpellToCast(SPELL_ENRAGE, CAST_SELF);
                    events.ScheduleEvent(EVENT_ENRAGE, 61000);
                    break;
                }
                default:
                    break;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_faerlina(Creature *_Creature)
{
    return new boss_faerlinaAI (_Creature);
}

void AddSC_boss_faerlina()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_faerlina";
    newscript->GetAI = &GetAI_boss_faerlina;
    newscript->RegisterSelf();
}
