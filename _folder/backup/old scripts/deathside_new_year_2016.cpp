// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#define SPELL_SNOWBALL 55185
#define SPELL_ICEBOLT 55186

#define SPELL_ARCANE_BLAST 55187
#define SPELL_HEAL 55188
#define SPELL_DISENGAGE 55189

#include "Language.h"

struct DeathSide_New_Year_2016_snowman_AI : public ScriptedAI
{
    DeathSide_New_Year_2016_snowman_AI(Creature *c) : ScriptedAI(c) {}
    
    Timer snowballTimer;
    void Reset()
    {
        snowballTimer.Reset(100);
        me->CastSpell(me, 39169, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (snowballTimer.Expired(diff))
        {
            me->SetFacingTo(me->GetOrientationTo(me->GetVictim()));
            DoCast(me->GetVictim(), SPELL_SNOWBALL);
            snowballTimer = 2000;
        }
    }
};

struct DeathSide_New_Year_2016_snowman_big_AI : public ScriptedAI
{
    DeathSide_New_Year_2016_snowman_big_AI(Creature *c) : ScriptedAI(c) {}
    
    Timer snowballTimer;
    Timer iceboltTimer;

    void Reset()
    {
        snowballTimer.Reset(4000);
        iceboltTimer.Reset(15000);
        me->CastSpell(me, 38695, true);
    }

    void DamageTaken(Unit* /*done_by*/, uint32 &damage)
    {
        if (damage >= me->GetHealth())
        {
            uint8 toGive = 25;
            uint8 count = 0;
            std::list<Player*> units;
            for (std::list<HostileReference*>::iterator i = me->getThreatManager().getThreatList().begin();
                    i != me->getThreatManager().getThreatList().end(); ++i)
                {
                    Unit* pUnit = Unit::GetUnit((*me), (*i)->getUnitGuid());
                    if (pUnit && pUnit->GetTypeId() == TYPEID_PLAYER
                        && me->GetDistance(pUnit) < 100 && pUnit->isInCombat())
                    {
                        units.push_back((Player*)pUnit);
                        ++count;
                    }
                }
            if (!count)
                return;

            while (toGive)
            {
                if (count > toGive)
                {
                    Hellground::RandomResizeList(units, toGive); // max 10 players who get 1 each.
                }
            
                for (std::list<Player*>::iterator itr = units.begin(); toGive && itr != units.end(); ++itr)
                {
                    uint32 noSpaceForCount = 0;
                    uint32 giving = 1;
                    ItemPosCountVec dest;
                    uint32 toyId = 693155;
                    uint8 msg = (*itr)->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, toyId, giving, &noSpaceForCount);
                    if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
                        giving -= noSpaceForCount;
                    if (giving == 0 || dest.empty())                         // can't add any
                    {
                        me->Whisper((*itr)->GetSession()->GetHellgroundString(LANG_SCRIPT_INVENTORY_NO_SPACE), (*itr)->GetGUID());
                        return;
                    }
                    Item* item = (*itr)->StoreNewItem(dest, toyId, true, Item::GenerateItemRandomPropertyId(toyId));
                    (*itr)->SendNewItem(item,1,true,false);
                    --toGive;
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (snowballTimer.Expired(diff))
        {
            if (me->GetVictim()->GetTypeId() != TYPEID_PLAYER)
            {
                me->SetFacingTo(me->GetOrientationTo(me->GetVictim()));
                DoCast(me->GetVictim(), SPELL_SNOWBALL);
            }
            std::list<Unit*> units;
            uint8 snowballs = 5;
            uint8 count = 0;
            for (std::list<HostileReference*>::iterator i = me->getThreatManager().getThreatList().begin();
                    i != me->getThreatManager().getThreatList().end(); ++i)
            {
                Unit* pUnit = Unit::GetUnit((*me), (*i)->getUnitGuid());
                if (pUnit && pUnit->GetTypeId() == TYPEID_PLAYER)
                {
                    units.push_back(pUnit);
                    ++count;
                }
            }

            if (count)
            while (snowballs)
            {
                if (count > snowballs)
                {
                    Hellground::RandomResizeList(units, snowballs); // max 10 players who get 1 each.
                }

                for (std::list<Unit*>::iterator itr = units.begin(); snowballs && itr != units.end(); ++itr)
                {
                    me->SetFacingTo(me->GetOrientationTo(*itr));
                    DoCast(*itr, SPELL_SNOWBALL);
                    --snowballs;
                }
            }
            snowballTimer = 2000;
        }

        if (iceboltTimer.Expired(diff))
        {
            for (std::list<HostileReference*>::iterator i = me->getThreatManager().getThreatList().begin();
                i != me->getThreatManager().getThreatList().end(); ++i)
            {
                Unit* pUnit = Unit::GetUnit((*me), (*i)->getUnitGuid());
                if (pUnit && pUnit->GetTypeId() == TYPEID_PLAYER)
                {
                    me->SetFacingTo(me->GetOrientationTo((*i)->getTarget()));
                    DoCast((*i)->getTarget(), SPELL_ICEBOLT);
                }
            }
            iceboltTimer = 30000;
        }        
    }
};

struct DeathSide_New_Year_2016_elf_AI : public ScriptedAI
{
    DeathSide_New_Year_2016_elf_AI(Creature* c) : ScriptedAI(c){}
    uint32 justSummonnedtime;
    Timer delay;
    void Reset()
    {
        me->SetReactState(REACT_DEFENSIVE);
        me->SetAggroRange(0);
        me->CombatStopWithPets();
        me->ClearInCombat();
        me->AttackStop();
        justSummonnedtime = 10000;
        delay.Reset(500);
    }

    void UpdateAI(const uint32 diff)
    {
       Unit *pOwner = me->GetOwner();
       Unit *victim = me->GetVictim();
       Unit *attacker = pOwner->getAttackerForHelper();

       if (pOwner)
       {
            if (!pOwner->isAlive())
            {
                me->ForcedDespawn();
                return;
            }

            if (justSummonnedtime > diff)
                justSummonnedtime -= diff;
            else
                justSummonnedtime = 0;

            if (!me->isInCombat() && !justSummonnedtime)
            {
                me->DestroyForNearbyPlayers();
                me->RemoveFromWorld();
                return;
            }

            if (!me->IsWithinDistInMap(pOwner, 50.0f) || (!victim || !attacker))
            {
                if (!me->GetVictim()|| !me->IsWithinDistInMap(pOwner, 50.0f))
                    if (!me->HasUnitState(UNIT_STAT_FOLLOW))
                    {
                    victim = NULL;
                    attacker = NULL;
                    me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, urand(M_PI, M_PI/2));
                    Reset();
                    return;
                    }
            }
            if (me->GetVictim() && me->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself() &&
                (pOwner->isInSanctuary() || me->isInSanctuary() || me->GetVictim()->isInSanctuary()))
            {
                victim = NULL;
                attacker = NULL;
                me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, M_PI);
                Reset();
                return;
            }

            if (victim || attacker)
            {
                if (attacker)
                {
                    if (attacker->GetTypeId() != TYPEID_UNIT || 
                    (((Creature*)attacker)->GetEntry() != 693104 && ((Creature*)attacker)->GetEntry() != 693105))
                    {
                        me->DestroyForNearbyPlayers();
                        me->RemoveFromWorld();
                        return;
                    }
                    me->SetInCombatWith(attacker);
                    ScriptedAI::AttackStartNoMove(attacker, CHECK_TYPE_CASTER);
                }
                else
                {
                    if (victim->GetTypeId() != TYPEID_UNIT || 
                    (((Creature*)victim)->GetEntry() != 693104 && ((Creature*)victim)->GetEntry() != 693105))
                    {
                        me->DestroyForNearbyPlayers();
                        me->RemoveFromWorld();
                        return;
                    }
                    me->SetInCombatWith(victim);
                    ScriptedAI::AttackStartNoMove(victim, CHECK_TYPE_CASTER);
                }
                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                if (delay.Expired(diff))
                {
                    me->CastSpell(me->GetVictim(), SPELL_DISENGAGE, true); 
                    if (me->GetOwner() && me->GetOwner()->GetHealthPercent() < 80)
                        DoCast(me->GetOwner(), SPELL_HEAL);
                    else
                        DoCast(me->GetVictim(), SPELL_ARCANE_BLAST);
                    delay = 500;
                }
            }
       }
    }
};

CreatureAI* GetAI_DeathSide_New_Year_2016_snowman_AI(Creature* pCreature)
{
return new DeathSide_New_Year_2016_snowman_AI (pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2016_snowman_big_AI(Creature* pCreature)
{
return new DeathSide_New_Year_2016_snowman_big_AI (pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2016_elf_AI(Creature* pCreature)
{
return new DeathSide_New_Year_2016_elf_AI (pCreature);
}

 void AddSC_DeathSide_New_Year_2016()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2016_snowman";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2016_snowman_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2016_snowman_big";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2016_snowman_big_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2016_elf";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2016_elf_AI;
     newscript->RegisterSelf();
 }
