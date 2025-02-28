// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"

struct DeathSide_Mine_WorkerAI : public ScriptedAI
{
    DeathSide_Mine_WorkerAI(Creature *c) : ScriptedAI(c) {}

    uint32 RunTimeCheker;
    uint32 YellTimer;

    void Reset()
    {
        if (me->HasUnitState(UNIT_STAT_FLEEING))
            me->SetFeared(false, NULL);
        RunTimeCheker = 0;
        YellTimer = 0;
    }

    void EnterCombat(Unit* /*who*/)
    {
        me->CastSpell(me, 54863, true); // Dummy aura that at apply commands guard to attack target. In spellauras.cpp
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (RunTimeCheker <= diff)
        {
            if (!me->HasUnitState(UNIT_STAT_FLEEING))
                me->SetFeared(true, me->GetVictim(), 10000);
            RunTimeCheker = 1000;
        }
        else
            RunTimeCheker -= diff;

        if (YellTimer <= diff)
        {
            switch (urand(0, 2))
            {
                case 0:
                    DoYell("На помощь!", LANG_UNIVERSAL, NULL);
                    break;
                case 1:
                    DoYell("Спасите!", LANG_UNIVERSAL, NULL);
                    break;
                case 2:
                    DoYell("Помогите!", LANG_UNIVERSAL, NULL);
                    break;              
            }
            YellTimer = urand(6000, 10000);
        }
        else
            YellTimer -= diff;
    }
};

CreatureAI* GetAI_DeathSide_Mine_WorkerAI(Creature* pCreature)
{
return new DeathSide_Mine_WorkerAI (pCreature);
}

 void AddSC_DeathSide_Mine_Worker()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_Mine_Worker";
     newscript->GetAI = &GetAI_DeathSide_Mine_WorkerAI;
     newscript->RegisterSelf();
 }
