// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* ScriptData
SDName: trash_instance_blood_furnace
SD%Complete: 0%(0/Unk)
SDComment: Trash NPC
SDCategory: Blood Furnance
EndScriptData */

#include "precompiled.h"
#include "def_blood_furnace.h"

#define AGGRO_RANGE     25.0

/****************
* Shadowmoon Summoner - N: 17395 H: 18617
*****************/

enum ShadowmoonSummoner
{
    SPELL_SS_H_FLAMESTRIKE          = 16102,
    SPELL_SS_N_FLAMESTRIKE          = 18399,
    SPELL_SS_H_FIREBALL             = 17290,
    SPELL_SS_N_FIREBALL             = 15242,
    SPELL_SS_FELHOUND_MANASTALKER   = 30851,
    SPELL_SS_SEDUCTRESS             = 30853,
    SPELL_SS_VISUAL                 = 31059,
    MOB_SS_NASCENT_ORC              = 17398,
    MOB_SS_ORC_NEOPHYTE             = 17429,
    MOB_SS_SHADOWMOON_ADEPT         = 17397
};

struct trash_17395AI : public ScriptedAI
{
    trash_17395AI(Creature *c) : ScriptedAI(c), summons(c)
    { }

    SummonList summons;

    Timer SummonFelgoundTimer;
    Timer SummonSeductressTimer;
    Timer FlamestrikeTimer;
    Timer FireballTimer;
    Timer VisualAuraTimer;
    Timer EmoteSpeechTimer;
    bool VisualCasted;

    void Reset()
    {
        ClearCastQueue();
        SummonFelgoundTimer.Reset(urand(1000, 3000));
        SummonSeductressTimer.Reset(urand(5000, 9000));
        FlamestrikeTimer.Reset(urand(12000, 14000));
        FireballTimer.Reset(urand(2000, 3000));
        VisualAuraTimer.Reset(1000);
        EmoteSpeechTimer.Reset(urand(2000, 5000));
        summons.DespawnAll();
        VisualCasted = false;
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void JustReachedHome()
    {
        ClearCastQueue();
        if((FindCreature(MOB_SS_NASCENT_ORC, 10, me)) || (FindCreature(MOB_SS_ORC_NEOPHYTE, 10, me)))
            DoCast(me, SPELL_SS_VISUAL, false);
    }

    void JustDied(Unit* killer)
    {
        summons.DespawnAll();
    }

    void JustSummoned(Creature *summoned)
    {
        if(summoned)
        {
            summons.Summon(summoned);
            if(me->GetVictim())
                summoned->AI()->AttackStart(me->GetVictim());
        }
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!VisualCasted)
        {
            if(VisualAuraTimer.Expired(diff))
            {
                if((FindCreature(MOB_SS_NASCENT_ORC, 10, me)) || (FindCreature(MOB_SS_ORC_NEOPHYTE, 10, me)))
                {
                    ClearCastQueue();
                    DoCast(me, SPELL_SS_VISUAL, false);
                    VisualCasted = true;
                }
                VisualAuraTimer = 0;
            }
        }

        if(!UpdateVictim())
        {
            if(EmoteSpeechTimer.Expired(diff))
            {
                if(FindCreature(MOB_SS_SHADOWMOON_ADEPT, 7, me))
                    me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                EmoteSpeechTimer = urand(2000, 5000);
            }
            return;
        }

        if(SummonFelgoundTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SS_FELHOUND_MANASTALKER, CAST_NULL);
            SummonFelgoundTimer = 0;
        }

        if(SummonSeductressTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SS_SEDUCTRESS, CAST_NULL);
            SummonSeductressTimer = 0;
        }

        if(FlamestrikeTimer.Expired(diff))
        {
            AddSpellToCast(HeroicMode ? SPELL_SS_H_FLAMESTRIKE : SPELL_SS_N_FLAMESTRIKE, CAST_RANDOM);
            FlamestrikeTimer = urand(12000, 14000);
        }

        if(FireballTimer.Expired(diff))
        {
            AddSpellToCast(HeroicMode ? SPELL_SS_H_FIREBALL : SPELL_SS_N_FIREBALL, CAST_TANK);
            FireballTimer = urand(2000, 3000);
        }

        CheckCasterNoMovementInRange(diff, 25.0);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_trash_17395(Creature *_Creature)
{
    return new trash_17395AI(_Creature);
}

/****************
* Shadowmoon Adept - N: 17397 H: 18615
*****************/

enum ShadowmoonAdept
{
    SPELL_SA_THRASH             = 3391,
    SPELL_SA_KICK               = 11978,
    SPELL_SA_VISUAL             = 31059,
    SAY_SA_1                    = -1901002,
    SAY_SA_2                    = -1901003,
    SAY_SA_3                    = -1901006,
    EMOTE_SA_FLEE               = -1901007,
    MOB_HF_IMP                  = 17477,
    MOB_SA_SHADOWMOON_SUMMONER  = 17395
};

struct trash_17397_18615AI : public ScriptedAI
{
    trash_17397_18615AI(Creature *c) : ScriptedAI(c) 
    {
    }

    Timer ThrashTimer;
    Timer KickTimer;
    Timer VisualAuraTimer;
    Timer EmoteSpeechTimer;
    bool Fleeing;
    bool VisualCasted;

    void Reset()
    {
        ThrashTimer.Reset(2000);
        KickTimer.Reset(5000);
        VisualAuraTimer.Reset(1000);
        EmoteSpeechTimer.Reset(urand(2000, 5000));
        Fleeing = false;
        VisualCasted = false;
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void JustReachedHome()
    {
        ClearCastQueue();
        if(FindCreature(MOB_HF_IMP, 10, me))
            DoCast(me, SPELL_SA_VISUAL, false);
    }

    void EnterCombat(Unit*)
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(RAND(SAY_SA_1, SAY_SA_2, SAY_SA_3), me); break;
            default: break;
        }
        
        me->InterruptNonMeleeSpells(true);
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!VisualCasted)
        {
            if(VisualAuraTimer.Expired(diff))
            {
                if(FindCreature(MOB_HF_IMP, 10, me))
                {
                    ClearCastQueue();
                    DoCast(me, SPELL_SA_VISUAL, false);
                    VisualCasted = true;
                }
                VisualAuraTimer = 0;
            }
        }

        if(!UpdateVictim())
        {
            if(EmoteSpeechTimer.Expired(diff))
            {
                if(FindCreature(MOB_SA_SHADOWMOON_SUMMONER, 7, me))
                    me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                EmoteSpeechTimer = urand(2000, 5000);
            }
            return;
        }

        if (ThrashTimer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SA_THRASH);
            ThrashTimer = urand(3000, 7000);
        }

        if (KickTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SA_KICK);
            KickTimer = urand(9000, 12000);
        }

        if (!Fleeing && HealthBelowPct(15))
        {
            Fleeing = true;
            me->DoFleeToGetAssistance();
            DoScriptText(EMOTE_SA_FLEE, me);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_trash_17397_18615AI(Creature *_Creature)
{
    return new trash_17397_18615AI(_Creature);
}

/****************
* Shadowmoon Warlock - N: 17371 H: 18619
*****************/

enum ShadowmoonWarlock
{
    SPELL_SW_SHADOWBOLT     = 12739,
    SPELL_SW_SHADOWBOLT_H   = 15472,
    SPELL_SW_CORRUPTION     = 32197,
    SPELL_SW_CORRUPTION_H   = 37113,
    SPELL_SW_CURSE_OF_TON   = 13338,
    SPELL_SW_VISUAL         = 31059,
    SPELL_SW_FEL_POWER      = 33111,
    SAY_SW_1                = -1901002,
    SAY_SW_2                = -1901003,
    SAY_SW_3                = -1901006,
    MOB_NASCENT_ORC         = 17398,
    MOB_ORC_NEOPHYTE        = 17429
};

struct trash_17371_18619AI : public ScriptedAI
{
    trash_17371_18619AI(Creature *c) : ScriptedAI(c) 
    {
    }

    Timer ShadowboltTimer;
    Timer CorruptionTimer;
    Timer CurseofTonguesTimer;
    Timer VisualAuraTimer;
    Timer FelPowerTimer;
    bool VisualCasted;

    void Reset()
    {
        ShadowboltTimer.Reset(1000);
        CorruptionTimer.Reset(5000);
        CurseofTonguesTimer.Reset(7000);
        VisualAuraTimer.Reset(1000);
        FelPowerTimer.Reset(urand(3000, 10000));
        VisualCasted = false;
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void JustReachedHome()
    {
        ClearCastQueue();
        if((FindCreature(MOB_NASCENT_ORC, 10, me)) || (FindCreature(MOB_ORC_NEOPHYTE, 10, me)))
            DoCast(me, SPELL_SW_VISUAL, false);
    }

    void EnterCombat(Unit*)
    {
        switch (urand(0, 3))
        {
            case 0: DoScriptText(RAND(SAY_SW_1, SAY_SW_2, SAY_SW_3), me); break;
            default: break;
        }
        
        me->InterruptNonMeleeSpells(true);
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!VisualCasted)
        {
            if(VisualAuraTimer.Expired(diff))
            {
                if((FindCreature(MOB_NASCENT_ORC, 10, me)) || (FindCreature(MOB_ORC_NEOPHYTE, 10, me)))
                {
                    ClearCastQueue();
                    DoCast(me, SPELL_SW_VISUAL, false);
                    VisualCasted = true;
                }
                VisualAuraTimer = 0;
            }
        }

        if(me->IsInCombat())
        {
            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
            {
                if(FelPowerTimer.Expired(diff))
                {
                    DoCast(me, SPELL_SW_FEL_POWER, false);
                    FelPowerTimer = urand(15000, 30000);
                }
            }
        }

        if(!UpdateVictim())
            return;

        if (ShadowboltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), HeroicMode ? SPELL_SW_SHADOWBOLT_H : SPELL_SW_SHADOWBOLT);
            ShadowboltTimer = urand(3500, 4500);
        }

        if (CorruptionTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), HeroicMode ? SPELL_SW_CORRUPTION_H : SPELL_SW_CORRUPTION);
            CorruptionTimer = 7000;
        }
        
        if(CurseofTonguesTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SW_CURSE_OF_TON);
            CurseofTonguesTimer = 12000;
        }

        CheckCasterNoMovementInRange(diff, 25.0);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_trash_17371_18619AI(Creature *_Creature)
{
    return new trash_17371_18619AI(_Creature);
}

/****************
* Shadowmoon Technician - N: 17414 H: 18618
*****************/

enum ShadowmoonTechnician
{
    SPELL_ST_DYNAMITE       = 40062,
    SPELL_ST_DYNAMITE_H     = 40064,
    SPELL_ST_SILENCE        = 6726,
    SPELL_ST_VISUAL         = 31059,
    SPELL_ST_PROX_BOMB      = 30846,
    SPELL_ST_PROX_BOMB_H    = 32784,
    SAY_ST_1                = -1901002,
    SAY_ST_2                = -1901003,
    SAY_ST_3                = -1901006,
    MOB_ST_FEL_ORC_NEO      = 17429
};

struct trash_17414_18618AI : public ScriptedAI
{
    trash_17414_18618AI(Creature *c) : ScriptedAI(c) 
    {
    }

    Timer ProximityBombTimer;
    Timer DynamiteTimer;
    Timer SilenceTimer;
    Timer VisualAuraTimer;
    bool VisualCasted;

    void Reset()
    {
        ProximityBombTimer.Reset(2800);
        DynamiteTimer.Reset(4400);
        SilenceTimer.Reset(5400);
        VisualAuraTimer.Reset(1000);
        VisualCasted = false;
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void JustReachedHome()
    {
        ClearCastQueue();
        if(FindCreature(MOB_ST_FEL_ORC_NEO, 10, me))
            DoCast(me, SPELL_ST_VISUAL, false);
    }

    void EnterCombat(Unit*)
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(RAND(SAY_ST_1, SAY_ST_2, SAY_ST_3), me); break;
            default: break;
        }
        
        me->InterruptNonMeleeSpells(true);
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!VisualCasted)
        {
            if(VisualAuraTimer.Expired(diff))
            {
                if(FindCreature(MOB_ST_FEL_ORC_NEO, 10, me))
                {
                    ClearCastQueue();
                    DoCast(me, SPELL_ST_VISUAL, false);
                    VisualCasted = true;
                }
                VisualAuraTimer = 0;
            }
        }

        if(!UpdateVictim())
            return;

        if (ProximityBombTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), HeroicMode ? SPELL_ST_PROX_BOMB_H : SPELL_ST_PROX_BOMB);
            ProximityBombTimer = urand(12000, 20000);
        }

        if (DynamiteTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), HeroicMode ? SPELL_ST_DYNAMITE_H : SPELL_ST_DYNAMITE);
            DynamiteTimer = urand(4000, 10000);
        }

        if (SilenceTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_ST_SILENCE, CAST_RANDOM_WITHOUT_TANK);
            SilenceTimer = 20000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_trash_17414_18618AI(Creature *_Creature)
{
    return new trash_17414_18618AI(_Creature);
}

/****************
* Magtheridon - 21174
*****************/

enum Magtheridon
{
    YELL_MAG_1  = -1901008,
    YELL_MAG_2  = -1901009,
    YELL_MAG_3  = -1901010,
    YELL_MAG_4  = -1901011,
    YELL_MAG_5  = -1901012,
    YELL_MAG_6  = -1901013
};

struct trash_21174AI : public ScriptedAI
{
    trash_21174AI(Creature *c) : ScriptedAI(c) 
    {
    }

    Timer YellTimer;

    void Reset()
    {
        YellTimer.Reset(15000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(YellTimer.Expired(diff))
        {
            DoScriptText(RAND(YELL_MAG_1, YELL_MAG_2, YELL_MAG_3, YELL_MAG_4, YELL_MAG_5, YELL_MAG_6), me);
            YellTimer = urand(100000, 150000);
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_trash_21174AI(Creature *_Creature)
{
    return new trash_21174AI(_Creature);
}

void AddSC_trash_instance_blood_furnace()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "trash_17395";
    newscript->GetAI = &GetAI_trash_17395;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trash_17397_18615";
    newscript->GetAI = &GetAI_trash_17397_18615AI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trash_17371_18619";
    newscript->GetAI = &GetAI_trash_17371_18619AI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trash_17414_18618";
    newscript->GetAI = &GetAI_trash_17414_18618AI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trash_21174";
    newscript->GetAI = &GetAI_trash_21174AI;
    newscript->RegisterSelf();
}