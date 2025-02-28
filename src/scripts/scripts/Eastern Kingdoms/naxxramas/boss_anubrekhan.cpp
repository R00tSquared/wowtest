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
SDName: Boss_Anubrekhan
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum AnubTexts
{
    SAY_GREET                   = -1533000,
    SAY_AGGRO1                  = -1533001,
    SAY_AGGRO2                  = -1533002,
    SAY_AGGRO3                  = -1533003,
    SAY_TAUNT1                  = -1533004,
    SAY_TAUNT2                  = -1533005,
    SAY_TAUNT3                  = -1533006,
    SAY_TAUNT4                  = -1533007,
    SAY_SLAY                    = -1533008
};

enum AnubEmotes
{
    EMOTE_CRYPT_GUARD           = -1533153,
    EMOTE_INSECT_SWARM          = -1533154,
    EMOTE_CORPSE_SCARABS        = -1533155
};

enum AnubSpells
{
    SPELL_IMPALE                = 28783,        // May be wrong spell id. Causes more dmg than I expect
    SPELL_LOCUSTSWARM           = 28785,        // This is a self buff that triggers the dmg debuff
    // invalid ?        
    SPELL_SUMMONGUARD           = 29508,        // Summons 1 crypt guard at targeted location
    SPELL_SELF_SPAWN_5          = 29105,        // This spawns 5 corpse scarabs ontop of us (most likely the pPlayer casts this on death)
    SPELL_SELF_SPAWN_10         = 28864         // This is used by the crypt guards when they die
};

enum AnubEvents
{
    EVENT_IMPALE    = 1,
    EVENT_SWARM     = 2,
    EVENT_SUMMON    = 3
};
#define NPC_CRYPT_GUARD         16573

static const float aCryptGuardLoc[4] = {3333.5f, -3475.9f, 287.1f, 3.17f};

struct boss_anubrekhanAI : public BossAI
{
    boss_anubrekhanAI(Creature *c) : BossAI(c, DATA_ANUB_REKHAN), Summons(me) { }

    bool greeted;
    SummonList Summons;

    void Reset()
    {
        greeted = false;

        events.Reset();
        events.ScheduleEvent(EVENT_IMPALE, 15000);
        events.ScheduleEvent(EVENT_SWARM, 90000);

        if(instance)
            instance->SetData(DATA_ANUB_REKHAN, NOT_STARTED);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!greeted && me->IsWithinDistInMap(who, 60.0f))
        {
            DoScriptText(RAND(SAY_GREET, SAY_TAUNT1, SAY_TAUNT2, SAY_TAUNT3, SAY_TAUNT4), me);
            greeted = true;
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);
        if(instance)
            instance->SetData(DATA_ANUB_REKHAN, IN_PROGRESS);
    }

    void KilledUnit(Unit* Victim)
    {
        if (!(rand()%5))
            if (Victim->GetTypeId() == TYPEID_PLAYER)
                Victim->CastSpell(Victim, SPELL_SELF_SPAWN_5, true);
        DoScriptText(SAY_SLAY, me);
    }

    void JustDied(Unit * killer)
    {
        Summons.DespawnAll();
        if(instance)
            instance->SetData(DATA_ANUB_REKHAN, DONE);
    }

    void JustSummoned(Creature* summon) override
    {
        Summons.Summon(summon);
        if (summon->GetEntry() == NPC_CRYPT_GUARD)
            DoScriptText(EMOTE_CRYPT_GUARD, summon);

        if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            summon->AI()->AttackStart(pTarget);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        // check if it is an actual killed guard
        if (!me->isAlive() || summon->isAlive() || summon->GetEntry() != NPC_CRYPT_GUARD)
            return;

        summon->CastSpell(summon, SPELL_SELF_SPAWN_10, true);
        DoScriptText(EMOTE_CORPSE_SCARABS, summon);
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
                case EVENT_IMPALE:
                {
                    //Cast Impale on a random target
                    //Do NOT cast it when we are afflicted by locust swarm
                    if (!me->HasAura(SPELL_LOCUSTSWARM,1))
                        AddSpellToCast(SPELL_IMPALE, CAST_RANDOM);

                    events.ScheduleEvent(EVENT_IMPALE, 15000);
                    break;
                }
                case EVENT_SWARM:
                {
                    AddSpellToCast(SPELL_LOCUSTSWARM, CAST_SELF);
                    DoScriptText(EMOTE_INSECT_SWARM, me);
                    events.ScheduleEvent(EVENT_SWARM, 90000);
                    events.ScheduleEvent(EVENT_SUMMON, 3000); 
                    break;
                }
                case EVENT_SUMMON:
                {
                    me->SummonCreature(NPC_CRYPT_GUARD, aCryptGuardLoc[0], aCryptGuardLoc[1], aCryptGuardLoc[2], aCryptGuardLoc[3], TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
                    events.ScheduleEvent(EVENT_SUMMON, 45000);
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

CreatureAI* GetAI_boss_anubrekhan(Creature *_Creature)
{
    return new boss_anubrekhanAI (_Creature);
}

void AddSC_boss_anubrekhan()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_anubrekhan";
    newscript->GetAI = &GetAI_boss_anubrekhan;
    newscript->RegisterSelf();
}
