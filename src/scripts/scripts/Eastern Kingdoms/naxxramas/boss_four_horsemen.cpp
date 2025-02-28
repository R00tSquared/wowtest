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
SDName: Boss_Four_Horsemen
SD%Complete: ??
SDComment: Lady Blaumeux, Thane Korthazz, Sir Zeliek, Highlord Mograine
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

//all horsemen
enum HorsemenEnum
{
    SPELL_SHIELDWALL            = 29061,
    SPELL_BESERK                = 26662,

    // BlaumeuxSpells
    SPELL_MARK_OF_BLAUMEUX      = 28833,
    SPELL_VOID_ZONE             = 28863,
    SPELL_SPIRIT_BLAUMEUX       = 28931, // 16776 npc

    // MograineSpells
    SPELL_MARK_OF_MOGRAINE      = 28834,
    SPELL_RIGHTEOUS_FIRE        = 28882,
    SPELL_SPIRIT_MOGRAINE       = 28928,

    // KorthazzSpells
    SPELL_MARK_OF_KORTHAZZ      = 28832,
    SPELL_METEOR                = 26558,
    SPELL_SPIRIT_KORTHAZZ       = 28932,
    
    // ZeliekSpells
    SPELL_MARK_OF_ZELIEK        = 28835,
    SPELL_HOLY_WRATH            = 28883,
    SPELL_SPIRIT_ZELIEK         = 28934,

    // BlaumeuxTexts
    SAY_BLAU_AGGRO              = -1533044,
    SAY_BLAU_TAUNT1             = -1533045,
    SAY_BLAU_TAUNT2             = -1533046,
    SAY_BLAU_TAUNT3             = -1533047,
    SAY_BLAU_SPECIAL            = -1533048,
    SAY_BLAU_SLAY               = -1533049,
    SAY_BLAU_DEATH              = -1533050,

    // MograineTexts
    SAY_MOGR_TAUNT1             = -1533210,
    SAY_MOGR_TAUNT2             = -1533211,
    SAY_MOGR_TAUNT3             = -1533212,
    SAY_MOGR_AGGRO1             = -1533213,
    SAY_MOGR_AGGRO2             = -1533214,
    SAY_MOGR_AGGRO3             = -1533215,
    SAY_MOGR_SLAY1              = -1533216,
    SAY_MOGR_SLAY2              = -1533217,
    SAY_MOGR_SPECIAL            = -1533218,
    SAY_MOGR_DEATH              = -1533219,

    // KorthazzTexts
    SAY_KORT_AGGRO              = -1533051,
    SAY_KORT_TAUNT1             = -1533052,
    SAY_KORT_TAUNT2             = -1533053,
    SAY_KORT_TAUNT3             = -1533054,
    SAY_KORT_SPECIAL            = -1533055,
    SAY_KORT_SLAY               = -1533056,
    SAY_KORT_DEATH              = -1533057,
    
    // ZeliekTexts
    SAY_ZELI_AGGRO              = -1533058,
    SAY_ZELI_TAUNT1             = -1533059,
    SAY_ZELI_TAUNT2             = -1533060,
    SAY_ZELI_TAUNT3             = -1533061,
    SAY_ZELI_SPECIAL            = -1533062,
    SAY_ZELI_SLAY               = -1533063,
    SAY_ZELI_DEATH              = -1533064
};

enum BlaumeuxEvents
{
    EVENT_BLAU_MARK         = 1,
    EVENT_BLAU_VOID_ZONE    = 2
};

enum MograineEvents
{
    EVENT_MOGRAINE_MARK     = 1
};

enum KorthazzEvents
{
    EVENT_KORTHAZZ_MARK     = 1,
    EVENT_KORTHAZZ_METEOR   = 2
};

enum ZeliekEvents
{
    EVENT_ZELIEK_MARK   = 1,
    EVENT_ZELIEK_WRATH  = 2
};

struct boss_lady_blaumeuxAI : public BossAI
{
    boss_lady_blaumeuxAI(Creature *c) : BossAI(c, DATA_THE_FOUR_HORSEMEN) { }

    bool ShieldWall1;
    bool ShieldWall2;
    uint32 MarkCounter;

    void Reset()
    {
        ShieldWall1 = false;
        ShieldWall2 = false;
        MarkCounter = 0;

        events.Reset();
        events.ScheduleEvent(EVENT_BLAU_MARK, 20000);
        events.ScheduleEvent(EVENT_BLAU_VOID_ZONE, 12000);

        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, NOT_STARTED);
        
        if(instance)
        {
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_HIGHLORD_MOGRAINE_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_SIR_ZELIEK_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_THANE_KORTHAZZ_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_BLAU_AGGRO, me);

        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* Victim)
    {
        DoScriptText(SAY_BLAU_SLAY, me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_BLAU_DEATH, me);
        DoCast(me, SPELL_SPIRIT_BLAUMEUX, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 180.0f);

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BLAU_MARK:
                {
                    AddSpellToCast(SPELL_MARK_OF_BLAUMEUX, CAST_NULL);
                    events.ScheduleEvent(EVENT_BLAU_MARK, 12000);
                    MarkCounter++;
                    break;
                }
                case EVENT_BLAU_VOID_ZONE:
                {
                    if (Unit * tmpUnit = SelectUnit(SELECT_TARGET_RANDOM, 0, 45.0f))
                        AddSpellToCast(tmpUnit, SPELL_VOID_ZONE);
                    events.ScheduleEvent(EVENT_BLAU_VOID_ZONE, 12000);
                    break;
                }
                default:
                    break;
            }
        }

        // Shield Wall - All 4 horsemen will shield wall at 50% hp and 20% hp for 20 seconds
        if (!ShieldWall1)
        {
            if (HealthBelowPct(50))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall1 = true;
            }
        }
        else
        {
            if (!ShieldWall2 && HealthBelowPct(20))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall2 = true;
            }
        }

        if(MarkCounter >= 100)
        {
            AddSpellToCast(SPELL_BESERK, CAST_SELF);
            MarkCounter = 0;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lady_blaumeux(Creature *_Creature)
{
    return new boss_lady_blaumeuxAI (_Creature);
}

struct boss_highlord_mograineAI : public BossAI
{
    boss_highlord_mograineAI(Creature *c) : BossAI(c, DATA_THE_FOUR_HORSEMEN)
    {
        pInstance = (instance_naxxramas*)c->GetInstanceData();
    }

    bool ShieldWall1;
    bool ShieldWall2;
    uint32 MarkCounter;
    instance_naxxramas* pInstance;

    void Reset()
    {
        ShieldWall1 = false;
        ShieldWall2 = false;
        MarkCounter = 0;

        events.Reset();
        events.ScheduleEvent(EVENT_MOGRAINE_MARK, 20000);

        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, NOT_STARTED);

        if(instance)
        {
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_LADY_BLAUMEUX_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_SIR_ZELIEK_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_THANE_KORTHAZZ_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
        }
    }

    void KilledUnit()
    {
        DoScriptText(RAND(SAY_MOGR_SLAY1, SAY_MOGR_SLAY2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_MOGR_DEATH, me);
        DoCast(me, SPELL_SPIRIT_MOGRAINE, true);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_MOGR_AGGRO1, SAY_MOGR_AGGRO2, SAY_MOGR_AGGRO3), me);
        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void DamageMade(Unit* target, uint32 & damage, bool direct_damage)
    {
        if (direct_damage && roll_chance_f(25.0f))
            me->CastSpell(target, SPELL_RIGHTEOUS_FIRE, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 180.0f);

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MOGRAINE_MARK:
                {
                    AddSpellToCast(SPELL_MARK_OF_MOGRAINE, CAST_NULL);
                    events.ScheduleEvent(EVENT_MOGRAINE_MARK, 12000);
                    MarkCounter++;
                    break;
                }
                default:
                    break;
            }
        }

        // Shield Wall - All 4 horsemen will shield wall at 50% hp and 20% hp for 20 seconds
        if (!ShieldWall1)
        {
            if (HealthBelowPct(50))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall1 = true;
            }
        }
        else
        {
            if (!ShieldWall2 && HealthBelowPct(20))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall2 = true;
            }
        }

        if(MarkCounter >= 100)
        {
            AddSpellToCast(SPELL_BESERK, CAST_SELF);
            MarkCounter = 0;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_highlord_mograine(Creature *_Creature)
{
    return new boss_highlord_mograineAI (_Creature);
}

struct boss_thane_korthazzAI : public BossAI
{
    boss_thane_korthazzAI(Creature *c) : BossAI(c, DATA_THE_FOUR_HORSEMEN) { }

    bool ShieldWall1;
    bool ShieldWall2;
    uint32 MarkCounter;

    void Reset()
    {
        ShieldWall1 = false;
        ShieldWall2 = false;
        MarkCounter = 0;

        events.Reset();
        events.ScheduleEvent(EVENT_KORTHAZZ_MARK, 20000);
        events.ScheduleEvent(EVENT_KORTHAZZ_METEOR, 12000); // needs proper timer

        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, NOT_STARTED);

        if(instance)
        {
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_LADY_BLAUMEUX_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_SIR_ZELIEK_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_HIGHLORD_MOGRAINE_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_KORT_AGGRO, me);
        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* Victim)
    {
        DoScriptText(SAY_KORT_SLAY, me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_KORT_DEATH, me);
        DoCast(me, SPELL_SPIRIT_KORTHAZZ, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 180.0f);

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_KORTHAZZ_MARK:
                {
                    AddSpellToCast(SPELL_MARK_OF_KORTHAZZ, CAST_NULL);
                    events.ScheduleEvent(EVENT_KORTHAZZ_MARK, 12000);
                    MarkCounter++;
                    break;
                }
                case EVENT_KORTHAZZ_METEOR:
                {
                    if (Unit * tmpUnit = SelectUnit(SELECT_TARGET_RANDOM, 0, 20.0f, true))
                        AddSpellToCast(tmpUnit, SPELL_METEOR);
                    events.ScheduleEvent(EVENT_KORTHAZZ_METEOR, 12000); // needs proper timer
                    break;
                }
                default:
                    break;
            }
        }

        // Shield Wall - All 4 horsemen will shield wall at 50% hp and 20% hp for 20 seconds
        if (!ShieldWall1)
        {
            if (HealthBelowPct(50))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall1 = true;
            }
        }
        else
        {
            if (!ShieldWall2 && HealthBelowPct(20))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall2 = true;
            }
        }

        if(MarkCounter >= 100)
        {
            AddSpellToCast(SPELL_BESERK, CAST_SELF);
            MarkCounter = 0;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_thane_korthazz(Creature *_Creature)
{
    return new boss_thane_korthazzAI (_Creature);
}

struct boss_sir_zeliekAI : public BossAI
{
    boss_sir_zeliekAI(Creature *c) : BossAI(c, DATA_THE_FOUR_HORSEMEN) {}

    bool ShieldWall1;
    bool ShieldWall2;
    uint32 MarkCounter;

    void Reset()
    {
        ShieldWall1 = false;
        ShieldWall2 = false;
        MarkCounter = 0;

        events.Reset();
        events.ScheduleEvent(EVENT_ZELIEK_MARK, 20000);
        events.ScheduleEvent(EVENT_ZELIEK_WRATH, 12000);

        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, NOT_STARTED);

        if(instance)
        {
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_LADY_BLAUMEUX_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_THANE_KORTHAZZ_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
            if (Creature* pOtherHorseman = instance->GetCreatureById(NPC_HIGHLORD_MOGRAINE_N))
                if(!pOtherHorseman->isAlive())
                    pOtherHorseman->Respawn();
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_ZELI_AGGRO, me);
        if(instance)
            instance->SetData(DATA_THE_FOUR_HORSEMEN, IN_PROGRESS);
    }

    void KilledUnit(Unit* Victim)
    {
        DoScriptText(SAY_ZELI_SLAY, me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_ZELI_DEATH, me);
        DoCast(me, SPELL_SPIRIT_ZELIEK, true);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 180.0f);

        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ZELIEK_MARK:
                {
                    AddSpellToCast(SPELL_MARK_OF_ZELIEK, CAST_NULL);
                    events.ScheduleEvent(EVENT_ZELIEK_MARK, 12000);
                    MarkCounter++;
                    break;
                }
                case EVENT_ZELIEK_WRATH:
                {
                    AddSpellToCast(SPELL_HOLY_WRATH, CAST_TANK);
                    events.ScheduleEvent(EVENT_ZELIEK_WRATH, 12000);
                    break;
                }
                default:
                    break;
            }
        }

        // Shield Wall - All 4 horsemen will shield wall at 50% hp and 20% hp for 20 seconds
        if (!ShieldWall1)
        {
            if (HealthBelowPct(50))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall1 = true;
            }
        }
        else
        {
            if (!ShieldWall2 && HealthBelowPct(20))
            {
                AddSpellToCast(SPELL_SHIELDWALL, CAST_SELF);
                ShieldWall2 = true;
            }
        }

        if(MarkCounter >= 100)
        {
            AddSpellToCast(SPELL_BESERK, CAST_SELF);
            MarkCounter = 0;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_sir_zeliek(Creature *_Creature)
{
    return new boss_sir_zeliekAI (_Creature);
}

void AddSC_boss_four_horsemen()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_lady_blaumeux";
    newscript->GetAI = &GetAI_boss_lady_blaumeux;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_highlord_mograine";
    newscript->GetAI = &GetAI_boss_highlord_mograine;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_thane_korthazz";
    newscript->GetAI = &GetAI_boss_thane_korthazz;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_sir_zeliek";
    newscript->GetAI = &GetAI_boss_sir_zeliek;
    newscript->RegisterSelf();
}
