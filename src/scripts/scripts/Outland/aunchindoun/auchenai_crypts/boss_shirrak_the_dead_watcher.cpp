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
Name: Boss_Shirrak_the_dead_watcher
%Complete: 80
Comment: InhibitMagic should stack slower far from the boss, proper Visual for Focus Fire, heroic implemented
Category: Auchindoun, Auchenai Crypts
EndScriptData */

#include "precompiled.h"
#include "def_auchenai_crypts.h"

#define SPELL_INHIBITMAGIC          32264
#define SPELL_ATTRACTMAGIC          32265
#define N_SPELL_CARNIVOROUSBITE     36383
#define H_SPELL_CARNIVOROUSBITE     39382
#define SPELL_CARNIVOROUSBITE       (HeroicMode?H_SPELL_CARNIVOROUSBITE:N_SPELL_CARNIVOROUSBITE)

#define ENTRY_FOCUS_FIRE            18374

#define N_SPELL_FIERY_BLAST         32302
#define H_SPELL_FIERY_BLAST         38382
#define SPELL_FIERY_BLAST           (HeroicMode?H_SPELL_FIERY_BLAST:N_SPELL_FIERY_BLAST)
#define SPELL_FOCUS_FIRE_VISUAL     42075 //need to find better visual

struct boss_shirrak_the_dead_watcherAI : public ScriptedAI
{
    boss_shirrak_the_dead_watcherAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;
    Timer Inhibitmagic_Timer;
    Timer Attractmagic_Timer;
    Timer Carnivorousbite_Timer;
    Timer FocusFire_Timer;
    bool HeroicMode;
    uint64 focusedTargetGUID;

    void Reset()
    {
        Inhibitmagic_Timer.Reset(1); //immediate
        Attractmagic_Timer.Reset(28000);
        Carnivorousbite_Timer.Reset(10000);
        FocusFire_Timer.Reset(17000);
        focusedTargetGUID = 0;
        if(pInstance)
            pInstance->SetData(TYPE_SHIRAKK, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if(pInstance)
            pInstance->SetData(TYPE_SHIRAKK, IN_PROGRESS);
    }

    void JustDied(Unit *killer)
    {
        Map *map = m_creature->GetMap();
        Map::PlayerList const &PlayerList = map->GetPlayers();
        for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            if(Player* i_pl = i->getSource())
                i_pl->RemoveAurasDueToSpell(SPELL_INHIBITMAGIC,0);
        }
        if(pInstance)
            pInstance->SetData(TYPE_SHIRAKK, DONE);
    }

    void JustSummoned(Creature *summoned)
    {
        if (summoned && summoned->GetEntry() == ENTRY_FOCUS_FIRE)
        {
            summoned->CastSpell(summoned, SPELL_FOCUS_FIRE_VISUAL, false);
            summoned->setFaction(m_creature->getFaction());
            summoned->SetLevel(m_creature->GetLevel());
            summoned->addUnitState(UNIT_STAT_ROOT);

            if(focusedTargetGUID)
            {
                if (Unit* focusedTarget = me->GetUnit(focusedTargetGUID))
                    summoned->AI()->AttackStart(focusedTarget);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Inhibitmagic_Timer
        if (Inhibitmagic_Timer.Expired(diff))
        {
            float dist;
            Map *map = m_creature->GetMap();
            Map::PlayerList const &PlayerList = map->GetPlayers();
            for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* i_pl = i->getSource())
                {
                    if(i_pl->isAlive() && (dist = i_pl->GetDistance(m_creature)) <= 45)
                    {
                        i_pl->RemoveAurasDueToSpell(SPELL_INHIBITMAGIC,0);
                        m_creature->AddAura(SPELL_INHIBITMAGIC, i_pl);
                        if(Aura *inh_magic = i_pl->GetAura(SPELL_INHIBITMAGIC,0))
                        {
                            if(dist < 35)
                                m_creature->AddAura(SPELL_INHIBITMAGIC, i_pl);
                            if(dist < 25)
                                m_creature->AddAura(SPELL_INHIBITMAGIC, i_pl);
                            if(dist < 15)
                               m_creature->AddAura(SPELL_INHIBITMAGIC, i_pl);
                        }
                    }
                }
            }
            Inhibitmagic_Timer = 3500;
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Attractmagic_Timer
        if (Attractmagic_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_ATTRACTMAGIC);
            Attractmagic_Timer = 30000;
            Carnivorousbite_Timer = 1500;
        }

        //Carnivorousbite_Timer
        if (Carnivorousbite_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_CARNIVOROUSBITE);
            Carnivorousbite_Timer = 10000;
        }

        //FocusFire_Timer
        if (FocusFire_Timer.Expired(diff))
        {
            // Summon Focus Fire & Emote
            Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0,60, true, m_creature->getVictimGUID());
            if (target && target->GetTypeId() == TYPEID_PLAYER && target->isAlive())
            {
                focusedTargetGUID = target->GetGUID();
                m_creature->SummonCreature(ENTRY_FOCUS_FIRE,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,5500);

                // Emote on focus fire target
				DoTextEmote(-1200453, target, true);
            }
            FocusFire_Timer = 15000+(rand()%5000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_shirrak_the_dead_watcher(Creature *_Creature)
{
    return new boss_shirrak_the_dead_watcherAI (_Creature);
}

struct mob_focus_fireAI : public Scripted_NoMovementAI
{
    mob_focus_fireAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }

    bool HeroicMode;
    Timer_UnCheked FieryBlast_Timer;
    bool fiery1, fiery2;

    void Reset()
    {
        FieryBlast_Timer.Reset(3000 + (rand() % 1000));
        fiery1 = fiery2 = true;
    }

    void EnterCombat(Unit *who)
    { }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        //FieryBlast_Timer
        if (fiery2 && FieryBlast_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_FIERY_BLAST);

            if(fiery1)
                fiery1 = false;
            else
                if(fiery2)
                    fiery2 = false;

            FieryBlast_Timer = 1000;
        }

        //DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_focus_fire(Creature *_Creature)
{
    return new mob_focus_fireAI (_Creature);
}

struct npc_auchenai_triggerAI : public ScriptedAI
{
    npc_auchenai_triggerAI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer Beam_Timer;

    void Reset()
    {
        Beam_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who)
    { }

    void UpdateAI(const uint32 diff)
    {
        if (Beam_Timer.Expired(diff))
        {
            std::list<Creature*> Vindicators;
            Hellground::AllCreaturesOfEntryInRange check(me, 18495, 10);
            Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(Vindicators, check);
            Cell::VisitGridObjects(me, searcher, 10);
    
            for(std::list<Creature*>::iterator i = Vindicators.begin(); i != Vindicators.end(); ++i)
            {
                if(!(*i)->IsInCombat() && !(*i)->isDead())
                {
                    if(!(*i)->IsNonMeleeSpellCast(false))
                    {
                        (*i)->CastSpell(me, 38034, false);
                        me->CastSpell(me, 34431, true);
                    }
                }
                else
                    me->RemoveAurasDueToSpell(34431);
            }
            
            if(Vindicators.empty())
                me->RemoveAurasDueToSpell(34431);

            Beam_Timer = 3000;
        }

        if (!UpdateVictim())
            return;

        //DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_auchenai_trigger(Creature *_Creature)
{
    return new npc_auchenai_triggerAI (_Creature);
}

void AddSC_boss_shirrak_the_dead_watcher()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_shirrak_the_dead_watcher";
    newscript->GetAI = &GetAI_boss_shirrak_the_dead_watcher;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_focus_fire";
    newscript->GetAI = &GetAI_mob_focus_fire;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_auchenai_trigger";
    newscript->GetAI = &GetAI_npc_auchenai_trigger;
    newscript->RegisterSelf();
}

