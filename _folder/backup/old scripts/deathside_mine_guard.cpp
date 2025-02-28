// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#define SPELL_CHARGE 43519
#define SPELL_CLEAVE 46468

struct DeathSide_Mine_GuardAI : public ScriptedAI
{
    DeathSide_Mine_GuardAI(Creature *c) : ScriptedAI(c) {}

    uint32 CleaveTimer;
    uint32 ChargeChecker;

    void Reset()
    {
        int32 Chance = 80;
        me->RemoveAurasDueToSpell(10021);
        me->CastCustomSpell(me, 10021, &Chance, NULL, NULL, true); // block chance
        Chance = 40;
        me->RemoveAurasDueToSpell(9941);
        me->CastCustomSpell(me, 9941, &Chance, NULL, NULL, true); // reflect chance
        me->SetStat(STAT_STRENGTH, 40000);
        CleaveTimer = 3000;
        ChargeChecker = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ChargeChecker <= diff)
        {
            if (!me->IsWithinMeleeRange(me->GetVictim()))
                DoCast(me->GetVictim(), SPELL_CHARGE, false);
            ChargeChecker = 2000;
        }
        else
            ChargeChecker -= diff;

        if (CleaveTimer <= diff)
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE, false);
            CleaveTimer = urand(2000, 4000);
        }
        else
            CleaveTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_DeathSide_Mine_GuardAI(Creature* pCreature)
{
return new DeathSide_Mine_GuardAI (pCreature);
}

 void AddSC_DeathSide_Mine_Guard()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Mine_Guard";
     newscript->GetAI = &GetAI_DeathSide_Mine_GuardAI;
     newscript->RegisterSelf();
 }
