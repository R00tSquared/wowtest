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
SDName: Boss_Darkweaver_Syth
SD%Complete: 85
SDComment: Shock spells/times need more work. Heroic partly implemented.
SDCategory: Auchindoun, Sethekk Halls
EndScriptData */

#include "precompiled.h"
#include "def_sethekk_halls.h"

#define SAY_SUMMON                  -1556000

#define SAY_AGGRO_1                 -1556001
#define SAY_AGGRO_2                 -1556002
#define SAY_AGGRO_3                 -1556003

#define SAY_SLAY_1                  -1556004
#define SAY_SLAY_2                  -1556005

#define SAY_DEATH                   -1556006

#define SAY_LAKKA                  -1900253
#define NPC_LAKKA                   18956

#define SPELL_FROST_SHOCK           21401 //37865
#define SPELL_FLAME_SHOCK           34354
#define SPELL_SHADOW_SHOCK          30138
#define SPELL_ARCANE_SHOCK          37132

#define SPELL_CHAIN_LIGHTNING       15659 //15305

#define NPC_ELEMENTAL               19203
#define NUM_ELEMENTALS              4

#define SPELL_FLAME_BUFFET          (HeroicMode?38141:33526)
#define SPELL_ARCANE_BUFFET         (HeroicMode?38138:33527)
#define SPELL_FROST_BUFFET          (HeroicMode?38142:33528)
#define SPELL_SHADOW_BUFFET         (HeroicMode?38143:33529)
#define SPELL_TELEPORT_VISUAL       41232

struct boss_darkweaver_sythAI : public ScriptedAI
{
    boss_darkweaver_sythAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        HeroicMode = me->GetMap()->IsHeroic();
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    SummonList summons;

    Timer FlameShock_Timer;
    Timer ArcaneShock_Timer;
    Timer FrostShock_Timer;
    Timer ShadowShock_Timer;
    Timer ChainLightning_Timer;

    bool summon90;
    bool summon50;
    bool summon10;
    bool HeroicMode;

    void Reset()
    {
        summons.DespawnAll();

        FlameShock_Timer.Reset(2000);
        ArcaneShock_Timer.Reset(4000);
        FrostShock_Timer.Reset(6000);
        ShadowShock_Timer.Reset(8000);
        ChainLightning_Timer.Reset(15000);

        summon90 = false;
        summon50 = false;
        summon10 = false;

        if(pInstance)
            pInstance->SetData(DATA_DARKWEAVEREVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), me);
        if(pInstance)
            pInstance->SetData(DATA_DARKWEAVEREVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        summons.DespawnAll();

        DoScriptText(SAY_DEATH, me);

        if (Creature* lakka = GetClosestCreatureWithEntry(me, NPC_LAKKA, 25.0f))
            DoScriptText(SAY_LAKKA, lakka);

        if(pInstance)
            pInstance->SetData(DATA_DARKWEAVEREVENT, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        if (rand()%2)
            return;

        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
    }

    void JustSummoned(Creature *summoned)
    {
        summoned->CastSpell(summoned, SPELL_TELEPORT_VISUAL, false);
        if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
            summoned->AI()->AttackStart(target);

        summons.Summon(summoned);
    }

    void SythSummoning()
    {
        DoScriptText(SAY_SUMMON, me);

        if (me->IsNonMeleeSpellCast(false))
            me->InterruptNonMeleeSpells(false);

        float px, py, pz;

        for (int id = NUM_ELEMENTALS; id--; )
        {
            me->GetNearPoint(px, py, pz, 0.0f, 8.0f, 0.5f * id * M_PI);

            if (Creature *elemental = me->SummonCreature(NPC_ELEMENTAL + id, px, py, pz, me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 0))
                summons.Summon(elemental);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if ((me->GetHealthPercent() < 90) && !summon90)
        {
            SythSummoning();
            summon90 = true;
        }

        if ((me->GetHealthPercent() < 50) && !summon50)
        {
            SythSummoning();
            summon50 = true;
        }

        if ((me->GetHealthPercent() < 10) && !summon10)
        {
            SythSummoning();
            summon10 = true;
        }

        if (FlameShock_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FLAME_SHOCK);

            FlameShock_Timer = 10000 + rand() % 5000;
        }

        if (ArcaneShock_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target,SPELL_ARCANE_SHOCK);

            ArcaneShock_Timer = 10000 + rand()%5000;
        } 

        if (FrostShock_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FROST_SHOCK);

            FrostShock_Timer = 10000 + rand() % 5000;
        }

        if (ShadowShock_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true))
                DoCast(target,SPELL_SHADOW_SHOCK);

            ShadowShock_Timer = 10000 + rand()%5000;
        } 

        if (ChainLightning_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true))
                DoCast(target,SPELL_CHAIN_LIGHTNING);

            ChainLightning_Timer = 25000;
        } 

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_darkweaver_syth(Creature *_Creature)
{
    return new boss_darkweaver_sythAI (_Creature);
}

/* ELEMENTALS */

struct mob_syth_fireAI : public ScriptedAI
{
    mob_syth_fireAI(Creature *c) : ScriptedAI(c)

    {
        HeroicMode = me->GetMap()->IsHeroic();
    }

    Timer FlameShock_Timer;
    Timer FlameBuffet_Timer;
    Timer SpawnMovement_Timer;
    bool HeroicMode;

    void Reset()
    {
        me->SetRooted(true);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        FlameShock_Timer.Reset(2500);
        FlameBuffet_Timer.Reset(5000);
        SpawnMovement_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpawnMovement_Timer.Expired(diff))
        {
            me->SetRooted(false);
            SpawnMovement_Timer = 0;
        }

        if (FlameShock_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FLAME_SHOCK);

            FlameShock_Timer = 5000;
        }

        if (FlameBuffet_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FLAME_BUFFET);

            FlameBuffet_Timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_fire(Creature *_Creature)
{
    return new mob_syth_fireAI (_Creature);
}

struct mob_syth_arcaneAI : public ScriptedAI
{
    mob_syth_arcaneAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = me->GetMap()->IsHeroic();
    }

    Timer ArcaneShock_Timer;
    Timer ArcaneBuffet_Timer;
    Timer SpawnMovement_Timer;
    bool HeroicMode;

    void Reset()
    {
        me->SetRooted(true);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
        ArcaneShock_Timer.Reset(2500);
        ArcaneBuffet_Timer.Reset(5000);
        SpawnMovement_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpawnMovement_Timer.Expired(diff))
        {
            me->SetRooted(false);
            SpawnMovement_Timer = 0;
        }

        if (ArcaneShock_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_ARCANE_SHOCK);

            ArcaneShock_Timer = 5000;
        }

        if (ArcaneBuffet_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_ARCANE_BUFFET);

            ArcaneBuffet_Timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_arcane(Creature *_Creature)
{
    return new mob_syth_arcaneAI (_Creature);
}

struct mob_syth_frostAI : public ScriptedAI
{
    mob_syth_frostAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = me->GetMap()->IsHeroic();
    }

    Timer FrostShock_Timer;
    Timer FrostBuffet_Timer;
    Timer SpawnMovement_Timer;
    bool HeroicMode;

    void Reset()
    {
        me->SetRooted(true);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);
        FrostShock_Timer.Reset(2500);
        FrostBuffet_Timer.Reset(5000);
        SpawnMovement_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpawnMovement_Timer.Expired(diff))
        {
            me->SetRooted(false);
            SpawnMovement_Timer = 0;
        }

        if (FrostShock_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FROST_SHOCK);

            FrostShock_Timer = 5000;
        }

        if (FrostBuffet_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_FROST_BUFFET);

            FrostBuffet_Timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_frost(Creature *_Creature)
{
    return new mob_syth_frostAI (_Creature);
}

struct mob_syth_shadowAI : public ScriptedAI
{
    mob_syth_shadowAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = me->GetMap()->IsHeroic();
    }

    Timer ShadowShock_Timer;
    Timer ShadowBuffet_Timer;
    Timer SpawnMovement_Timer;
    bool HeroicMode;

    void Reset()
    {
        me->SetRooted(true);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_SHADOW, true);
        ShadowShock_Timer.Reset(2500);
        ShadowBuffet_Timer.Reset(5000);
        SpawnMovement_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (SpawnMovement_Timer.Expired(diff))
        {
            me->SetRooted(false);
            SpawnMovement_Timer = 0;
        }

        if (ShadowShock_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                DoCast(target, SPELL_SHADOW_SHOCK);

            ShadowShock_Timer = 5000;
        }
        
        if (ShadowBuffet_Timer.Expired(diff))
        {
            if( Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true) )
                DoCast(target, SPELL_SHADOW_BUFFET);

            ShadowBuffet_Timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_syth_shadow(Creature *_Creature)
{
    return new mob_syth_shadowAI (_Creature);
}

void AddSC_boss_darkweaver_syth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_darkweaver_syth";
    newscript->GetAI = &GetAI_boss_darkweaver_syth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_fire";
    newscript->GetAI = &GetAI_mob_syth_arcane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_arcane";
    newscript->GetAI = &GetAI_mob_syth_arcane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_frost";
    newscript->GetAI = &GetAI_mob_syth_frost;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_syth_shadow";
    newscript->GetAI = &GetAI_mob_syth_shadow;
    newscript->RegisterSelf();
}

