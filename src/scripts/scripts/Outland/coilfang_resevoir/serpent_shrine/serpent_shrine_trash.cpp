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
#include "def_serpent_shrine.h"


#define SPELL_FRIGHTENING_SHOUT     38945
#define SPELL_FRENZY                38947
#define SPELL_KNOCKBACK             38576
#define SPELL_EXECUTE               38959
#define SPELL_MORTAL_CLEAVE         38572

struct mob_vashjir_honor_guardAI : public ScriptedAI
{
    mob_vashjir_honor_guardAI(Creature *c) : ScriptedAI(c) {}

    Timer Shout_Timer;
    Timer Knockback_Timer;
    Timer Execute_Timer;
    Timer Cleave_Timer;
    bool Frenzy_Cast;

    Timer Talk_Timer;
    Timer Check_Timer;
    bool Talking;
    uint64 Talk_Creature;

    void Reset()
    {
        Shout_Timer.Reset(urand(25000, 40000));
        Knockback_Timer.Reset(urand(10000, 30000));
        Execute_Timer.Reset(0);
        Cleave_Timer.Reset(urand(3000, 6000));
        Frenzy_Cast = false;
        Talk_Timer.Reset(10000);
        Check_Timer.Reset(1000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (Shout_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_FRIGHTENING_SHOUT, CAST_TANK);
            Shout_Timer = urand(30000, 40000);
        }
        

        if (Knockback_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_KNOCKBACK, CAST_TANK);
            Knockback_Timer = urand(10000, 15000);
        }


        if (Execute_Timer.Expired(diff))
        {
            if(me->GetVictim()->GetHealth() * 5 <= me->GetVictim()->GetMaxHealth()) // below 20%
            {
                AddSpellToCast(SPELL_EXECUTE, CAST_TANK);
                Execute_Timer = 30000;
            }
        }


        if (Cleave_Timer.Expired(diff))
        {
            AddSpellToCast(SPELL_MORTAL_CLEAVE, CAST_TANK);
            Cleave_Timer = urand(3000, 6000);
        }
        

        if(!Frenzy_Cast && HealthBelowPct(50))
        {
            AddSpellToCast(SPELL_FRENZY, CAST_SELF);
            me->Yell(-1200482, LANG_UNIVERSAL, 0);
            Frenzy_Cast = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    void UpdateAIOOC(const uint32 diff)
    {
        // need some tweaking
        if(!Talking)
        {
            if (Talk_Timer.Expired(diff))
            {
                if (Check_Timer.Expired(diff))
                {
                    std::list<Creature*> friends = FindAllFriendlyInGrid(3);
                    if(!friends.empty())
                    {

                        Creature *c = friends.front();
                        c->GetMotionMaster()->MoveIdle();
                        c->SetInFront(me);
                        me->GetMotionMaster()->MoveIdle();
                        me->SetInFront(c);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_ROAR);

                        Check_Timer = 1500;
                        Talk_Timer = urand(4000, 6000);
                        Talking = true;
                        Talk_Creature = c->GetGUID();

                        //c->Say("Check1", 0, 0);
                        //me->Say("Check0", 0, 0);
                    }
                }
            }
        }
        else // Talking
        {
            if (Check_Timer.Expired(diff))
                {
                    //me->Say("Check2", 0, 0);
                    if(Creature *c = me->GetCreature(Talk_Creature))
                    {
                        c->SetInFront(me);
                        me->SetInFront(c);
                        c->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_TALK);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_TALK);
                    }
                    Check_Timer = 0;
                
            }

            if (Talk_Timer.Expired(diff))
            {
                //me->Say("Check3", 0, 0);
                Talking = false;
                Talk_Timer = urand(4000, 8000);
                me->GetMotionMaster()->MovementExpired();
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);

                if (Creature *c = me->GetCreature(Talk_Creature))
                {
                    c->GetMotionMaster()->MovementExpired();
                    c->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                }
            }
        }
    }
};

CreatureAI* GetAI_mob_vashjir_honor_guard(Creature *_Creature)
{
    return new mob_vashjir_honor_guardAI(_Creature);
}

#define SPELL_SUMMON_SERPENTSHRINE_PARASITE         39053
#define SPELL_INITIAL_INFECTION                     39032
#define SPELL_SPORE_QUAKE                           38976
#define SPELL_ACID_GEYSER                           38971
#define SPELL_ATHROPIC_BLOW                         39015
#define SPELL_ENRAGE                                39031
#define SPELL_FIRE_VULNERABILITY                    38715
#define SPELL_FROST_VULNERABILITY                   38714
#define SPELL_NATURE_VULNERABILITY                  38717
#define SPELL_TOXIC_POOL                            38718

#define NPC_MUSHROM                                 22335
#define NPC_COLOSSUS_LURKER                         22347
#define NPC_COLOSSUS_RAGER                          22352


struct mob_underbog_colossusAI : public ScriptedAI
{
    mob_underbog_colossusAI(Creature *c) : ScriptedAI(c)
    {
        type = urand(0, 2);
        Vulnerability = RAND(SPELL_FIRE_VULNERABILITY, SPELL_FROST_VULNERABILITY, SPELL_NATURE_VULNERABILITY);
    }

    uint8 type;
    uint32 Vulnerability;

    Timer Infection_Timer;
    Timer Parasite_Timer;
    Timer Quake_Timer;
    Timer Geyser_Timer;
    Timer Blow_Timer;
    Timer Enrage_Timer;

    void Reset()
    {
        Infection_Timer.Reset(urand(1000, 10000));
        Parasite_Timer.Reset(10000);
        Quake_Timer.Reset(urand(10000, 40000));
        Geyser_Timer.Reset(15000);
        Blow_Timer.Reset(1000);
        Enrage_Timer.Reset(15000);
        me->CastSpell(me, Vulnerability, false);
    }

    void JustDied(Unit *)
    {
        uint32 count = 0;
        uint32 entry = 0;

        switch(urand(0, 4))
        {
        case 0:
            // nothing
            break;
        case 1:
            count = 4;
            entry = NPC_MUSHROM;
            break;
        case 2:
            count = urand(2, 3);
            entry = NPC_COLOSSUS_LURKER;
            break;
        case 3:
            count = urand(8, 10);
            entry = NPC_COLOSSUS_RAGER;
            break;
        case 4:
            me->CastSpell(me, SPELL_TOXIC_POOL, true);
            break;
        }

        for(uint8 i = 0; i < count; i++)
        {
            // float x, y, z;
            // me->GetPosition(x, y, z);
            
            Position pos;
            me->GetRandomPoint(pos.x, pos.y, pos.z, frand(1, 7), pos.x, pos.y, pos.z);
            me->GetValidPointInAngle(pos, frand(1, 7), frand(0, 2*M_PI), true);
            me->SummonCreature(entry, pos.x, pos.y, pos.z, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
            if (entry == NPC_MUSHROM)
                me->SummonGameObject(185199, pos.x, pos.y, pos.z, 0, 0, 0, 0, 0, 20);
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {
        // tried to fix geyser facing, but failed
        //if (spell->Id == SPELL_ACID_GEYSER)
        //{
        //    me->SetFacingTo(me->GetAngleTo(target));
        //}
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        switch(type)
        {
        case 0:
            if (Infection_Timer.Expired(diff))
            {
                AddSpellToCast(SPELL_INITIAL_INFECTION, CAST_RANDOM);
                Infection_Timer = urand(25000, 35000);
            }
            

            if (Quake_Timer.Expired(diff))
            {
                AddSpellToCast(SPELL_SPORE_QUAKE, CAST_SELF);
                Quake_Timer = urand(30000, 60000);
            }
            break;
        case 1:
            if (Geyser_Timer.Expired(diff))
            {
                AddSpellToCast(SPELL_ACID_GEYSER, CAST_RANDOM_WITHOUT_TANK);
                Geyser_Timer = urand(30000, 60000);
            }
            

            if (Parasite_Timer.Expired(diff))
            {
                AddSpellToCast(SPELL_SUMMON_SERPENTSHRINE_PARASITE, CAST_RANDOM);
                Parasite_Timer = urand(10000, 20000);
            }
            
            break;
        case 2:
            if (Enrage_Timer.Expired(diff))
            {
                AddSpellToCast(SPELL_ENRAGE, CAST_SELF);
                Enrage_Timer = urand(30000, 50000);
            }
            

            if (me->HasAura(SPELL_FRENZY, 0))
            {
                if (Blow_Timer.Expired(diff))
                {
                    AddSpellToCast(SPELL_ATHROPIC_BLOW, CAST_TANK);
                    Blow_Timer = 1500;
                }
            }
            break;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_mob_underbog_colossus(Creature *_Creature)
{
    return new mob_underbog_colossusAI(_Creature);
}

struct mob_serpentshrine_parasiteAI : public ScriptedAI
{
    mob_serpentshrine_parasiteAI(Creature *c) : ScriptedAI(c) {}

    uint64 TargetGUID;
    Timer_UnCheked Check_Timer;

    void Reset()
    {
        TargetGUID = 0;
        Check_Timer.Reset(1000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!TargetGUID)
        {
            std::list<Player*> players = FindAllPlayersInRange(50);
            if(players.empty())
            {
                me->Kill(me);
                std::list<Player*>::iterator i = players.begin();
                advance(i, urand(0, players.size()-1));
                Player *target = *i;
                if(target->isAlive())
                {
                    TargetGUID = target->GetGUID();
                    me->GetMotionMaster()->MoveChase(target);
                }
            }
        }
    }
};

CreatureAI* GetAI_mob_serpentshrine_parasite(Creature *_Creature)
{
    return new mob_serpentshrine_parasiteAI(_Creature);
}

#define MOB_SS_TIDECALLER   22238

struct mob_ssc_elementalAI : public ScriptedAI
{
    mob_ssc_elementalAI(Creature *c) : ScriptedAI(c) 
    {
        CreatureEntry = c->GetEntry();
    }

    uint64 OwnerGUID;
    uint32 CreatureEntry;
    Timer VolleyTimer;
    Timer NovaTimer;

    void Reset()
    {
        OwnerGUID = 0;
        VolleyTimer.Reset(2400);
        NovaTimer.Reset(7000);
    }

    void IsSummonedBy(Unit* summoner)
    {
        OwnerGUID = summoner->GetGUID();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CreatureEntry == MOB_SS_TIDECALLER)
        {
            if (VolleyTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), 38623);
                VolleyTimer = urand(17000, 24000);
            }

            if (NovaTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), 39035);
                NovaTimer = urand(20000, 30000);
            }
        }

        if (OwnerGUID)
        {
            Unit* owner = me->GetMap()->GetCreature(OwnerGUID);
            if (!owner || owner->isDead())
            {
                me->ForcedDespawn();
                return;
            }
        }
        else
        {
            if (CreatureEntry == MOB_SS_TIDECALLER)
            {
                me->ForcedDespawn();
                return;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_ssc_elemental(Creature *_Creature)
{
    return new mob_ssc_elementalAI(_Creature);
}

enum
{
    // Mushrooms
    SPELL_SPORE_CLOUD = 38652,
    SPELL_PUTRID_MUSHROOM = 31690,
    SPELL_GROW = 31698
};

struct mob_ssc_mushroomAI : public ScriptedAI
{
    mob_ssc_mushroomAI(Creature *c) : ScriptedAI(c) {}

    Timer Grow_Timer;
    Timer Spore_Timer;
    Timer Shrink_Timer;
    Timer Despawn_Timer;

    void Reset()
    {
        Grow_Timer.Reset(1000);
        Spore_Timer.Reset(20000);
        Shrink_Timer.Reset(24000);
        Despawn_Timer.Reset(0);

        DoCast(me, SPELL_PUTRID_MUSHROOM, true);
    }

    void MoveInLineOfSight(Unit *who)
    {
        return;
    }

    void AttackStart(Unit* who)
    {
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (Spore_Timer.Expired(diff))
        {
            DoCast(me, SPELL_SPORE_CLOUD);
            Grow_Timer = 0;
            Spore_Timer = 0;
        }

        if (Grow_Timer.Expired(diff))
        {
            DoCast(me, SPELL_GROW);
            Grow_Timer = 2000;
        }

        if (Shrink_Timer.Expired(diff))
        {
            me->RemoveAurasDueToSpell(SPELL_GROW);
            Despawn_Timer = 1650;
        }

        if (Despawn_Timer.Expired(diff))
        {
            me->SetUInt32Value(UNIT_FIELD_DISPLAYID, 11686);
            // me->RemoveAurasDueToSpell(SPELL_PUTRID_MUSHROOM);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->ForcedDespawn(20000);
            Despawn_Timer = 0;
        }
    }
};
CreatureAI* GetAI_mob_ssc_mushroom(Creature *_Creature)
{
    return new mob_ssc_mushroomAI(_Creature);
}

void AddSC_serpent_shrine_trash()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "mob_vashjir_honor_guard";
    newscript->GetAI = &GetAI_mob_vashjir_honor_guard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_underbog_colossus";
    newscript->GetAI = &GetAI_mob_underbog_colossus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_serpentshrine_parasite";
    newscript->GetAI = &GetAI_mob_serpentshrine_parasite;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ssc_elemental";
    newscript->GetAI = &GetAI_mob_ssc_elemental;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_ssc_mushroom";
    newscript->GetAI = &GetAI_mob_ssc_mushroom;
    newscript->RegisterSelf();
}
