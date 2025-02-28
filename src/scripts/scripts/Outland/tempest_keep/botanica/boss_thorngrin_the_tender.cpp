/* ScriptData
SDName: boss_thorngrin_the_tender
SD%Complete:
SDComment: Boss Thorngrin the Tender
SDCategory: The Botanica
EndScriptData */

#include "precompiled.h"
#include "def_botanica.h"

enum ThorngrinTheTender
{
    SPELL_SACRIFICE     = 34661,
    SPELL_HELLFIRE      = 34659,
    SPELL_HELLFIRE_HC   = 39131,
    SPELL_ENRAGE        = 34670,

    YELL_AGGRO          = -1901014,
    YELL_HELLFIRE_1     = -1901015,
    YELL_HELLFIRE_2     = -1901016,
    YELL_SACRIFICE      = -1901017,
    YELL_PLAYERKILL     = -1901018,
    YELL_DEATH          = -1901019,
    YELL_50HEALTH       = -1901020,
    YELL_20HEALTH       = -1901021,
    YELL_INTRO          = -1901022
};

struct boss_thorngrin_the_tenderAI : public ScriptedAI
{
    boss_thorngrin_the_tenderAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer SacrificeTimer;
    Timer HellfireTimer;
    Timer EnrageTimer;
    bool Yelled50;
    bool Yelled20;
    bool YelledIntro;

    void Reset()
    {
        ClearCastQueue();
        SacrificeTimer.Reset(12000);
        HellfireTimer.Reset(urand(4800, 12100));
        EnrageTimer.Reset(urand(12000, 17000));
        Yelled50 = false;
        Yelled20 = false;
        YelledIntro = false;
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STUN, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_KNOCKOUT, true);
        if(pInstance)
            pInstance->SetData(TYPE_THORNGRIN, NOT_STARTED);
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
        DoScriptText(YELL_AGGRO, me);
        if(pInstance)
            pInstance->SetData(TYPE_THORNGRIN, IN_PROGRESS);
    }

    void KilledUnit(Unit * victim)
    {
        if (victim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(YELL_PLAYERKILL, me);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(YELL_DEATH, me);
        if(pInstance)
            pInstance->SetData(TYPE_THORNGRIN, DONE);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(who->GetTypeId() != TYPEID_PLAYER)
            return;

        if(!YelledIntro)
        {
            if (who->IsWithinDistInMap(me, 45))
            {
                YelledIntro = true;
                DoScriptText(YELL_INTRO, me);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (SacrificeTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SACRIFICE, CAST_RANDOM_WITHOUT_TANK);
            DoScriptText(YELL_SACRIFICE, me);
            SacrificeTimer = 26000;
        }

        if (HellfireTimer.Expired(diff))
        {
            AddSpellToCast(HeroicMode ? SPELL_HELLFIRE_HC : SPELL_HELLFIRE, CAST_NULL);
            DoScriptText(RAND(YELL_HELLFIRE_1, YELL_HELLFIRE_2), me);
            HellfireTimer = HeroicMode ? urand(16900, 25300) : urand(16900, 22900);
        }
        
        if(EnrageTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_ENRAGE, CAST_SELF);
            EnrageTimer = urand(27000, 33000);
        }
        
        if(HealthBelowPct(50) && !Yelled50)
        {
            DoScriptText(YELL_50HEALTH, me);
            Yelled50 = true;
        }
        
        if(HealthBelowPct(20) && !Yelled20)
        {
            DoScriptText(YELL_20HEALTH, me);
            Yelled20 = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_thorngrin_the_tenderAI(Creature *_Creature)
{
    return new boss_thorngrin_the_tenderAI(_Creature);
}

void AddSC_boss_thorngrin_the_tender()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_thorngrin_the_tender";
    newscript->GetAI = &GetAI_boss_thorngrin_the_tenderAI;
    newscript->RegisterSelf();
}