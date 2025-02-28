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
SDName: Boss_Ambassador_Hellmaw
SD%Complete: 99
SDComment: It appears that it's done now.
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "precompiled.h"
#include "def_shadow_labyrinth.h"

#define AURA_BANISH             37833

enum AmbassadorHellmaw
{
    SAY_INTRO               = -1555000,
    SAY_AGGRO1              = -1555001,
    SAY_AGGRO2              = -1555002,
    SAY_AGGRO3              = -1555003,
    SAY_HELP                = -1555004,
    SAY_SLAY1               = -1555005,
    SAY_SLAY2               = -1555006,
    SAY_DEATH               = -1555007,
    
    SPELL_CORROSIVE_ACID    = 33551,
    SPELL_FEAR              = 33547,
    SPELL_ENRAGE            = 26662,
    
    PATH_PATROL             = 2100,
    PATH_FINAL              = 2101,
    
    SPELL_CONTAINMENT_BEAM  = 32958,
    SPELL_RED_BEAM          = 30944,
    NPC_TRIGGER             = 21159
};

struct boss_ambassador_hellmawAI : public ScriptedAI
{
    boss_ambassador_hellmawAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;
    bool HeroicMode;

    Timer EventCheck_Timer;
    Timer CorrosiveAcid_Timer;
    Timer Fear_Timer;
    Timer Enrage_Timer;
    Timer OnPath_Delay;
    bool IntroDone;
    bool patrol;
    bool CalledForHelp;
    bool KillScript;

    void Reset()
    {
        EventCheck_Timer.Reset(2000);
        CorrosiveAcid_Timer.Reset(25000);
        Fear_Timer.Reset(18000);
        Enrage_Timer.Reset(180000);
        OnPath_Delay = 0;
        CalledForHelp = false;

        if (pInstance->GetData(TYPE_HELLMAW) == NOT_STARTED)
        {
            IntroDone = false;
            //IsBanished = false;
            KillScript = false;
        }

        if (pInstance)
        {
            if (pInstance->GetData(TYPE_HELLMAW) == NOT_STARTED)
            {
                me->ApplySpellImmune(AURA_BANISH, IMMUNITY_MECHANIC, MECHANIC_BANISH, true);
                DoCast(m_creature, AURA_BANISH);
            }
        }
    }

    void DoIntro()
    {
        if (pInstance)
            pInstance->SetData(TYPE_HELLMAW, SPECIAL);

        DoScriptText(SAY_INTRO, me);

        me->RemoveAurasDueToSpell(AURA_BANISH);
        me->ApplySpellImmune(AURA_BANISH, IMMUNITY_MECHANIC, MECHANIC_BANISH, false);
        
        if (me->HasAura(SPELL_RED_BEAM, 0))
            me->RemoveAurasDueToSpell(SPELL_RED_BEAM);

        std::list<Creature*> TriggerList;
        Hellground::AllCreaturesOfEntryInRange check(me, NPC_TRIGGER, 20);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(TriggerList, check);
        Cell::VisitGridObjects(me, searcher, 20);
        for(std::list<Creature*>::iterator i = TriggerList.begin(); i != TriggerList.end(); ++i)
            me->Kill((*i));

        KillScript = true;
        //IsBanished = false;
        IntroDone = true;
    }

    void EnterCombat(Unit *who)
    {
        if (me->HasAura(AURA_BANISH, 0))
        {
            return;
        }

        me->GetMotionMaster()->Clear();
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);
        
        if (pInstance)
            pInstance->SetData(TYPE_HELLMAW, IN_PROGRESS);
    }

    void EnterEvadeMode()
    {
        pInstance->SetData(TYPE_HELLMAW, SPECIAL);
        ScriptedAI::EnterEvadeMode();
    }

    void KilledUnit(Unit *victim)
    {
        if (KillScript)
            DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(TYPE_HELLMAW, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!pInstance)
            return;

        if (!IntroDone)
        {
            if (EventCheck_Timer.Expired(diff))
            {
                if(pInstance->GetData(TYPE_RITUALIST) == DONE)
                {
                    OnPath_Delay = 0;
                    DoIntro();
                }
                EventCheck_Timer = 5000;
            }
        }

        if (!me->IsInCombat() && !me->HasAura(AURA_BANISH, 0) && !OnPath_Delay.GetInterval())
        {
            me->GetMotionMaster()->MovePath(PATH_PATROL, false);
            OnPath_Delay = 55000;
            patrol = false;
        }

        if (!me->IsInCombat() && !patrol && OnPath_Delay.Expired(diff))
        {
            me->GetMotionMaster()->MovePath(PATH_FINAL, true);
            patrol = true;
        }

        if (!UpdateVictim())
            return;

        if (CorrosiveAcid_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CORROSIVE_ACID);
            CorrosiveAcid_Timer = 25000;
        }

        if (Fear_Timer.Expired(diff))
        {
            DoCast(me, SPELL_FEAR);
            Fear_Timer = 25000;
        }

        if (HeroicMode)
        {
            if (Enrage_Timer.Expired(diff))
            {
                DoCast(me,SPELL_ENRAGE);
                Enrage_Timer = 5*MINUTE*1000;
            }
        }
        if (me->GetHealthPercent() <= 20 && !CalledForHelp)
        {
            DoScriptText(SAY_HELP, me);
            CalledForHelp = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_ambassador_hellmaw(Creature *_Creature)
{
    return new boss_ambassador_hellmawAI (_Creature);
}

/****************
* Containment Beam - 21159
*****************/

enum SunseekerChanneler
{
    NPC_AMBASSADOR_HELLMAW  = 18731,
};

struct mob_21159AI : public ScriptedAI
{
    mob_21159AI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer Timer_Beam;

    void Reset()
    {
        ClearCastQueue();
        Timer_Beam.Reset(1000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if (Timer_Beam.Expired(diff) && pInstance->GetData(TYPE_HELLMAW) != SPECIAL)
            {
                if (me->HasAura(SPELL_CONTAINMENT_BEAM))
                {
                    switch(me->GetDBTableGUIDLow())
                    {
                        case 74234:
                            if(Unit* Ambassador = FindCreature(NPC_AMBASSADOR_HELLMAW, 20.0f, me))
                                me->CastSpell(Ambassador, SPELL_RED_BEAM, true);
                            break;
                        case 74233:
                            if(Unit* Ambassador = FindCreature(NPC_AMBASSADOR_HELLMAW, 20.0f, me))
                                me->CastSpell(Ambassador, SPELL_RED_BEAM, true);
                            break;
                        default:
                            break;
                    }
                    Timer_Beam = urand(30000, 60000);
                }
            }
            return;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_21159AI(Creature *_Creature)
{
    return new mob_21159AI(_Creature);
}


/****************
* Cabal Ritualist - N: 18794 H: 20645
*****************/

enum CabalRitualist
{
    SPELL_GOUGE                             = 12540,
    SPELL_FLAME_BUFFET                      = 9574,
    SPELL_FIRE_BLAST_N                      = 14145,
    SPELL_FIRE_BLAST_H                      = 20795,
    SPELL_ADDLE_HUMANOID                    = 33487,
    SPELL_DISPEL_MAGIC                      = 17201,
    SPELL_ARCANE_MISSILES_N                 = 33833,
    SPELL_ARCANE_MISSILES_H                 = 38264,
    SPELL_FROSTNOVA_N                       = 15063,
    SPELL_FROSTNOVA_H                       = 15532,
    SPELL_FROSTBOLT_N                       = 15497,
    SPELL_FROSTBOLT_H                       = 12675
};

struct mob_cabal_ritualistAI : public ScriptedAI
{
    mob_cabal_ritualistAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;
    bool HeroicMode;

    Timer Timer_Gouge;
    Timer Timer_FlameBuffet;
    Timer Timer_FireBlast;
    Timer Timer_AddleHumanoid;
    Timer Timer_DispelMagic;
    Timer Timer_ArcaneMissiles;
    Timer Timer_FrostNova;
    Timer Timer_FrostBolt;
    Timer Timer_Beam;
    uint8 Phase;

    void Reset()
    {
        ClearCastQueue();
        Timer_Gouge.Reset(urand(4000, 8000));
        Timer_FlameBuffet.Reset(2000);
        Timer_FireBlast.Reset(urand(6000, 10000));
        Timer_AddleHumanoid.Reset(urand(9000, 16000));
        Timer_DispelMagic.Reset(urand(7000, 9000));
        Timer_ArcaneMissiles.Reset(urand(1000, 2000));
        Timer_FrostNova.Reset(urand(2000, 3000));
        Timer_FrostBolt.Reset(urand(1000, 2000));
        Timer_Beam.Reset(1000);
        Phase = 0;
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat(80.0f);
        me->RemoveAurasDueToSpell(SPELL_CONTAINMENT_BEAM);
        me->InterruptNonMeleeSpells(true);
        Phase = urand(1, 3);
    }

    void JustDied(Unit *victim)
    {
        if (pInstance)
            pInstance->SetData(TYPE_RITUALIST, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if (Timer_Beam.Expired(diff))
            {
                if(Unit* Trigger = FindCreature(NPC_TRIGGER, 10.0f, me))
                    me->CastSpell(Trigger, SPELL_CONTAINMENT_BEAM, false);
                Timer_Beam = urand(30000, 60000);
            }
            return;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();

        switch(Phase)
        {
            case 0:
                Phase = urand(1, 3);
                break;
            case 1: // frost
            {
                if (Timer_FrostBolt.Expired(diff))
                {
                    AddSpellToCast(HeroicMode ? SPELL_FROSTBOLT_H : SPELL_FROSTBOLT_N, CAST_TANK);
                    Timer_FrostBolt = 3500;
                }

                if (Timer_FrostNova.Expired(diff))
                {
                    AddSpellToCast(HeroicMode ? SPELL_FROSTNOVA_H : SPELL_FROSTNOVA_N, CAST_NULL);
                    Timer_FrostNova = urand(7000, 12000);
                }

                break;
            }
            case 2: // arcane
            {
                if (Timer_ArcaneMissiles.Expired(diff))
                {
                    AddSpellToCast(HeroicMode ? SPELL_ARCANE_MISSILES_H : SPELL_ARCANE_MISSILES_N, CAST_TANK);
                    Timer_ArcaneMissiles = urand(1000, 1500);
                }

                if (Timer_AddleHumanoid.Expired(diff))
                {
                    AddSpellToCast(SPELL_ADDLE_HUMANOID, CAST_RANDOM);
                    Timer_AddleHumanoid = urand(12000, 16000);
                }
                break;
            }
            case 3: // fire
            {
                if (Timer_FireBlast.Expired(diff))
                {
                    AddSpellToCast(HeroicMode ? SPELL_FIRE_BLAST_H : SPELL_FIRE_BLAST_N, CAST_TANK);
                    Timer_FireBlast = urand(4000, 6000);
                }

                if (Timer_FlameBuffet.Expired(diff))
                {
                    AddSpellToCast(SPELL_FLAME_BUFFET, CAST_TANK);
                    Timer_FlameBuffet = urand(10000, 12000);
                }
                break;
            }
            default: break;
        }

        if(me->GetPower(POWER_MANA)*100/me->GetMaxPower(POWER_MANA) <= 25)
        {
            if (Timer_Gouge.Expired(diff))
            {
                AddSpellToCast(SPELL_GOUGE, CAST_TANK);
                Timer_Gouge = urand(4000, 6000);
            }
        }

        if(HeroicMode)
        {
            if (Timer_DispelMagic.Expired(diff))
            {
                AddSpellToCast(SPELL_DISPEL_MAGIC, CAST_SELF);
                Timer_DispelMagic = urand(9000, 15000);
            }
        }
    }
};

CreatureAI* GetAI_mob_cabal_ritualistAI(Creature *_Creature)
{
    return new mob_cabal_ritualistAI(_Creature);
}

void AddSC_boss_ambassador_hellmaw()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_ambassador_hellmaw";
    newscript->GetAI = &GetAI_boss_ambassador_hellmaw;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_21159";
    newscript->GetAI = &GetAI_mob_21159AI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_cabal_ritualist";
    newscript->GetAI = &GetAI_mob_cabal_ritualistAI;
    newscript->RegisterSelf();
}

