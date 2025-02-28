/* ScriptData
SDName: boss_commander_sarannis
SD%Complete:
SDComment: Boss Commander Sarannis
SDCategory: The Botanica
EndScriptData */

#include "precompiled.h"
#include "def_botanica.h"

enum CommanderSarannis
{
    SPELL_ARCANE_RESONANCE      = 34974,
    SPELL_ARCANE_DEVASTATION    = 34799,
    NPC_BLOODWARDER_RESERVIST   = 20078,
    NPC_BLOODWARDER_MENDER      = 19633,
    YELL_INTRO                  = -1901030,
    YELL_AGGRO                  = -1901023,
    YELL_KILL1                  = -1901024,
    YELL_KILL2                  = -1901025,
    YELL_SUMMON                 = -1901026,
    YELL_ARCANE_RESONANCE       = -1901027,
    YELL_ARCANE_BLAST           = -1901028,
    YELL_DEATH                  = -1901029
};

struct boss_commander_sarannisAI : public ScriptedAI
{
    boss_commander_sarannisAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer ArcaneResonance_Timer;
    Timer ArcaneDevastation_Timer;
    Timer SummonTimer;
    bool Summoned55;
    bool YelledIntro;
    SummonList summons;

    void Reset()
    {
        ClearCastQueue();
        ArcaneResonance_Timer.Reset(HeroicMode ? urand(2800, 15300) : urand(3800, 12900));
        SummonTimer.Reset(HeroicMode ? 30000 : 0);
        Summoned55 = false;
        YelledIntro = false;
        summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(TYPE_SARANNIS, NOT_STARTED);
    }

    void JustSummoned(Creature* summon)
    {
        if (summon)
        {
            summons.Summon(summon);
            summon->AI()->AttackStart(me->GetVictim());
        }
    }

    void DamageMade(Unit* target, uint32 &damage, bool direct_damage)
    {
        if (damage)
        {
            if (target->HasAura(SPELL_ARCANE_RESONANCE, 0))
            {
                Aura* Aur = target->GetAura(SPELL_ARCANE_RESONANCE, 0);
                if (Aur && Aur->GetStackAmount() == 3)
                {
                    ForceSpellCastWithScriptText(target, SPELL_ARCANE_DEVASTATION, YELL_ARCANE_RESONANCE);
                    target->RemoveAurasDueToSpell(SPELL_ARCANE_RESONANCE);
                }
            }
            
        }
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
        DoScriptText(YELL_AGGRO, me);
        if(pInstance)
            pInstance->SetData(TYPE_SARANNIS, IN_PROGRESS);
    }

    void KilledUnit(Unit * victim)
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(RAND(YELL_KILL1, YELL_KILL2), me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(YELL_DEATH, me);
        if(pInstance)
            pInstance->SetData(TYPE_SARANNIS, DONE);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(who->GetTypeId() != TYPEID_PLAYER)
            return;

        if(!YelledIntro)
        {
            if (who->IsWithinDistInMap(me, 45))
            {
                DoScriptText(YELL_INTRO, me);
                YelledIntro = true;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ArcaneResonance_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_ARCANE_RESONANCE, CAST_TANK); // have to be casted only on units in 10 yd range
            DoScriptText(YELL_ARCANE_RESONANCE, me);
            ArcaneResonance_Timer = HeroicMode ? urand(2800, 15300) : urand(3800, 12900);
        }
        
        if(!HeroicMode && HealthBelowPct(55) && !Summoned55)
        {
            float x, y, z;
            for (uint8 i = 0; i < 3; ++i)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                {
                    target->GetNearPoint(x, y, z, 5.0f);
                    me->SummonCreature(NPC_BLOODWARDER_RESERVIST, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                }
            }
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
            {
                target->GetNearPoint(x, y, z, 5.0f);
                me->SummonCreature(NPC_BLOODWARDER_MENDER, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
            }
            DoScriptText(YELL_SUMMON, me);
            Summoned55 = true;
        }
        
        if(HeroicMode)
        {
            if(SummonTimer.Expired(diff))
            {
                DoScriptText(YELL_SUMMON, me);
                float x, y, z;
                for (uint8 i = 0; i < 3; ++i)
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                    {
                        target->GetNearPoint(x, y, z, 5.0f);
                        me->SummonCreature(NPC_BLOODWARDER_RESERVIST, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                    }
                }
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                {
                    target->GetNearPoint(x, y, z, 5.0f);
                    me->SummonCreature(NPC_BLOODWARDER_MENDER, x, y, z, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                }
                SummonTimer = 60000;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_commander_sarannisAI(Creature *_Creature)
{
    return new boss_commander_sarannisAI(_Creature);
}

void AddSC_boss_commander_sarannis()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_commander_sarannis";
    newscript->GetAI = &GetAI_boss_commander_sarannisAI;
    newscript->RegisterSelf();
}