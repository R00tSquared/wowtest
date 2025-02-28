// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
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

#include "precompiled.h"
#include "def_blackrock_depths.h"


#define BOSS_COREN_DIREBREW             23872
#define NPC_ILSA_DIREBREW               26764
#define NPC_URSULA_DIREBREW             26822
#define NPC_DARK_IRON_ANTAGONIST        23795
#define COREN_TEXT                      -1230037 // "You'll pay for this insult!"

#define SPELL_SUMMON_MINION_KNOCKBACK   50313 //wrong?
#define SPELL_SUMMON_MINION             47375
#define SPELL_DIREBREW_CHARGE           47718
#define SPELL_DIREBREWS_DISARM          47310
#define SPELL_DISARM_GROW               47409
#define SPELL_SUMMON_MOLE_MACHINE       43563


//////////////////////
//Coren Direbrew
//////////////////////
struct boss_coren_direbrewAI : public ScriptedAI
{
    boss_coren_direbrewAI(Creature *c) : ScriptedAI(c) { }

    Timer Disarm_Timer;
    Timer Summon_Timer;
    Timer Drink_Timer;
    Timer Ilsa_Timer;
    Timer Ursula_Timer;

    void Reset()
    {
        Disarm_Timer.Reset(20000);
        Summon_Timer.Reset(15000);
        Ilsa_Timer.Reset(1);
        Ursula_Timer.Reset(1);
        
        //me->setFaction(35);

        std::list<Creature*> antagonistList;
        Hellground::AllCreaturesOfEntryInRange check(me, NPC_DARK_IRON_ANTAGONIST, 100);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(antagonistList, check);

        Cell::VisitGridObjects(me, searcher, 100);

        if(antagonistList.size())
        {
            for(std::list<Creature*>::iterator i = antagonistList.begin(); i != antagonistList.end(); ++i)
            {
                //(*i)->setFaction(35);
                (*i)->Respawn();
            }
        }

    }

    void SummonedCreatureDespawn(Creature * creature)
    {
        switch(creature->GetEntry())
        {
            case NPC_ILSA_DIREBREW:
                Ilsa_Timer.Reset(5000);
                break;
            case NPC_URSULA_DIREBREW:
                Ursula_Timer.Reset(10000);
                break;
            default:
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(Disarm_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_DISARM_GROW, CAST_SELF, true);
            AddSpellToCast(SPELL_DISARM_GROW, CAST_SELF, true);
            AddSpellToCast(SPELL_DISARM_GROW, CAST_SELF, true);
            AddSpellToCast(SPELL_DIREBREWS_DISARM, CAST_NULL);
            Disarm_Timer = 30000;
        }
      
        if(Summon_Timer.Expired(diff))
        {
            if(Unit * target = SelectUnit(SELECT_TARGET_RANDOM, 0, 45, true))
            {
                AddSpellToCast(target, SPELL_SUMMON_MOLE_MACHINE);
                //me->SummonGameObject(188478, me->GetVictim()->GetPositionX(), me->GetVictim()->GetPositionY(), me->GetVictim()->GetPositionZ(), 0,0 ,0 ,0 ,0, 4);
                //me->GetVictim()->KnockBackFrom(me, 4, 7);
                //AddSpellToCast(me, SPELL_SUMMON_MINION_KNOCKBACK);
                AddSpellToCast(target, SPELL_SUMMON_MINION, true);
                Summon_Timer = 15000;
            }
        }


        if(float(me->GetHealth())/float(me->GetMaxHealth()) < 0.66f)
        {
            if (Ilsa_Timer.Expired(diff))
            {
                Creature * Ilsa = GetClosestCreatureWithEntry(me, NPC_ILSA_DIREBREW, 100);
                if (!Ilsa)
                {
                    me->SummonCreature(NPC_ILSA_DIREBREW, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                }
                Ilsa_Timer = 300000;
            }
        }

        if(float(me->GetHealth())/float(me->GetMaxHealth()) < 0.33f)
        {
            if(Ursula_Timer.Expired(diff))
            {
                Creature * Ursula = GetClosestCreatureWithEntry(me, NPC_URSULA_DIREBREW, 100);
                if (!Ursula)
                {
                    me->SummonCreature(NPC_URSULA_DIREBREW, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                }
                Ursula_Timer = 300000;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_coren_direbrew(Creature *creature)
{
    return new boss_coren_direbrewAI (creature);
}



//////////////////////
//Trigger
//////////////////////
struct direbrew_starter_triggerAI : public ScriptedAI
{
    direbrew_starter_triggerAI(Creature *c) : ScriptedAI(c) { }

    Timer Start_Timer;

    void Reset()
    {
        Start_Timer.Reset(1000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(Start_Timer.Expired(diff))
        {
            Creature * Coren = GetClosestCreatureWithEntry(me, BOSS_COREN_DIREBREW, 20);
            if (Coren && Coren->isAlive())
            {
                DoScriptText(COREN_TEXT, Coren, 0);
                Coren->setFaction(54);

                std::list<Creature*> antagonistList;
                Hellground::AllCreaturesOfEntryInRange check(me, NPC_DARK_IRON_ANTAGONIST, 100);
                Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(antagonistList, check);

                Cell::VisitGridObjects(me, searcher, 100);

                if(antagonistList.size())
                {
                    for(std::list<Creature*>::iterator i = antagonistList.begin(); i != antagonistList.end(); ++i)
                    {
                        (*i)->setFaction(54);
                    }
                }

                me->ForcedDespawn(0);
            }
        }
    }
};

CreatureAI* GetAI_direbrew_starter_trigger(Creature *creature)
{
    return new direbrew_starter_triggerAI (creature);
}

void AddSC_boss_coren_direbrew()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="direbrew_starter_trigger";
    newscript->GetAI = &GetAI_direbrew_starter_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_coren_direbrew";
    newscript->GetAI = &GetAI_boss_coren_direbrew;
    newscript->RegisterSelf();
}
