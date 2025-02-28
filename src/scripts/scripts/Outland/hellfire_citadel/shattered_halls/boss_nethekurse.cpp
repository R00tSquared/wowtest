// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_Grand_Warlock_Nethekurse
SD%Complete: 95
SDComment:
SDCategory: Hellfire Citadel, Shattered Halls
EndScriptData */

/* ContentData
boss_grand_warlock_nethekurse
mob_fel_orc_convert
EndContentData */

#include "precompiled.h"
#include "def_shattered_halls.h"

struct Say
{
    int32 id;
};

static Say PeonAttacked[]=
{
    {-1540001},
    {-1540002},
    {-1540003},
    {-1540004}
};
static Say PeonDies[]=
{
    {-1540005},
    {-1540006},
    {-1540007}
};

#define SAY_AGGRO_NO_PEONS          -1540008
#define SAY_AGGRO_ALL_PEONS         -1540000
#define SAY_AGGRO_PARTIAL_PEONS     -1540013
#define SAY_SHADOW_SEER             -1540009
#define SAY_DEATH_COIL              -1540010
#define SAY_SHADOW_FISSURE          -1540011
#define SAY_KILL_1                  -1540012
#define SAY_KILL_2                  -1540014
#define SAY_KILL_3                  -1540015
#define SAY_KILL_4                  -1540016
#define SAY_DIE                     -1540017

#define SPELL_AOE_DEATH_COIL        30741 // fix targeting
#define SPELL_DEATH_COIL            30500
#define SPELL_DARK_SPIN             30502
#define SPELL_SHADOW_BOLT           30505
#define SPELL_DARK_CLEAVE           30508
#define SPELL_SHADOW_FISSURE        30496
#define SPELL_SHADOW_CLEAVE         30495
#define H_SPELL_SHADOW_SLAM         35953
#define SPELL_SHADOW_SEAR           30735
#define SPELL_HEMORRHAGE            30478
#define SPELL_CONSUMPTION_N         32250
#define SPELL_CONSUMPTION_H         35952

#define NPC_PEON                    17083

struct boss_grand_warlock_nethekurseAI : public ScriptedAI
{
    boss_grand_warlock_nethekurseAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
        HeroicMode = me->GetMap()->IsHeroic();
        me->SetAggroRange(120.0f); // in correspondance to used SelectNearestTarget
    }

    ScriptedInstance* pInstance;
    bool HeroicMode;

    bool IntroOnce;
    bool IsIntroEvent;
    bool IsMainEvent;
    bool SpinOnce;
    bool Phase;
    bool Torture;

    uint32 PeonEngagedCount;
    uint32 PeonKilledCount;

    Timer IntroEvent_Timer;
    Timer DeathCoil_Timer;
    Timer ShadowFissure_Timer;
    Timer Cleave_Timer;
    Timer ShadowBolt_Timer;
    Timer DarkCleave_Timer;

    std::list<uint64> alivePeons;

    void Reset()
    {
        Torture = false;
        IsIntroEvent = false;
        IntroOnce = false;
        IsMainEvent = false;
        SpinOnce = false;
        Phase = false;

        PeonEngagedCount = 0;
        PeonKilledCount = 0;

        IntroEvent_Timer.Reset(0);
        DeathCoil_Timer.Reset(urand(10000, 18000));
        ShadowFissure_Timer.Reset(urand(14000, 18000));
        Cleave_Timer.Reset(HeroicMode ? 8000 : 16000);
        ShadowBolt_Timer.Reset(1000);
        DarkCleave_Timer.Reset(1000);

        std::list<Creature*> alivePeonsPtr = FindAllCreaturesWithEntry(NPC_PEON, 40.0f);
        for (std::list<Creature*>::iterator itr = alivePeonsPtr.begin(); itr != alivePeonsPtr.end(); ++itr)
            alivePeons.push_back((*itr)->GetGUID());

        if (pInstance)
        {
            pInstance->SetData(TYPE_NETHEKURSE, FAIL);
            pInstance->SetData(TYPE_NETHEKURSE, NOT_STARTED);
        }   
    }

    void AttackStart(Unit* who)
    {
        if(who->GetTypeId() == TYPEID_UNIT && !((Creature*)who)->isPet())
            return;

        ScriptedAI::AttackStart(who);
    }

    void EnterCombat(Unit* who)
    {
        if(alivePeons.size() == 3)
            DoScriptText(SAY_AGGRO_ALL_PEONS, me);
        else if(!alivePeons.empty())
            DoScriptText(SAY_AGGRO_PARTIAL_PEONS, me);

        for (std::list<uint64>::iterator itr = alivePeons.begin(); itr != alivePeons.end(); ++itr)
        {
            if (Creature* it = me->GetCreature(*itr))
                it->AI()->AttackStart(who);
        }

        IsIntroEvent = false;
        IsMainEvent = true;
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2, SAY_KILL_3, SAY_KILL_4), me);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_DIE, me);

        if(pInstance)
            pInstance->SetData(TYPE_NETHEKURSE,DONE);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!IntroOnce && me->IsWithinDistInMap(who, 40.0f))
        {
            if (who->GetTypeId() != TYPEID_PLAYER || ((Player*)who)->isGameMaster())
                return;

            IntroOnce = true;
            IsIntroEvent = true;
            IntroEvent_Timer = 1000;

            if (pInstance)
                pInstance->SetData(TYPE_NETHEKURSE, IN_PROGRESS);
        }

        if (IsIntroEvent || !IsMainEvent) // with this he shouldn't be able to attack players untill they will attack him
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustSummoned(Creature *summoned)
    {
        summoned->setFaction(14);
        summoned->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        summoned->CastSpell(summoned, HeroicMode ? SPELL_CONSUMPTION_H : SPELL_CONSUMPTION_N, false, 0, 0, me->GetGUID());
    }

    void DoYellForPeonEnterCombat()
    {
        if (PeonEngagedCount >= 3)
            return;

        if (Unit* pl = me->SelectNearestTarget(120))
        {
            me->SetFacingTo(me->GetAngleTo(pl));
            DoScriptText(PeonAttacked[PeonEngagedCount].id, me);
            ++PeonEngagedCount;
        }
    }

    void DoYellForPeonDeath()
    {
        if (PeonKilledCount > 3)
            return;

        if (PeonKilledCount == 3)
        {
            IsIntroEvent = false;
            IsMainEvent = true;
            DoScriptText(SAY_AGGRO_NO_PEONS, me);
            if (Unit* pl = me->SelectNearestTarget(120))
                me->AI()->AttackStart(pl);
        }
        else if (Unit* pl = me->SelectNearestTarget(120))
        {
            me->SetFacingTo(me->GetAngleTo(pl));
            me->HandleEmoteCommand(21); // Applaud
            DoScriptText(PeonDies[PeonKilledCount].id, me);
        }

        ++PeonKilledCount;
    }

    Creature* GetRandomPeon()
    {
        std::vector<Creature*> alivePeonsVector;
        for (std::list<uint64>::iterator i = alivePeons.begin(); i != alivePeons.end(); ++i)
        {
            if (Creature* it = me->GetCreature(*i))
            {
                if(it->IsInCombat()) // if there are any peon in combat - ignore everything and wait next check
                    return NULL;
                else if (!it->IsInCombat() || !it->isDead())
                    alivePeonsVector.push_back(it); 
            }
        }

        if (alivePeonsVector.empty())
            return NULL;

        uint32 randTarget = urand(0,alivePeonsVector.size()-1);
        return alivePeonsVector[randTarget];
    }

    void DoTortureOne()
    {
        if(Creature* RandomPeon = GetRandomPeon())
        {
            me->SetFacingTo(me->GetAngleTo(RandomPeon));
            DoCast(RandomPeon, SPELL_SHADOW_SEAR);
            DoScriptText(SAY_SHADOW_SEER, me);
            switch (urand(0, 3))
            {
                case 0:
                    RandomPeon->MonsterYell(-1200489, 0, me->GetGUID());
                    break;
                case 1:
                    RandomPeon->MonsterYell(-1200490, 0, me->GetGUID());
                    break;
                case 2:
                    RandomPeon->MonsterYell(-1200491, 0, me->GetGUID());
                    break;
                case 3:
                    RandomPeon->MonsterYell(-1200492, 0, me->GetGUID());
                    break;
            }
        }
    }

    void DoTortureTwo()
    {
        if (Creature* RandomPeon = GetRandomPeon())
        {
            me->SetFacingTo(me->GetAngleTo(RandomPeon));
            DoCast(RandomPeon, SPELL_AOE_DEATH_COIL, true);
            DoScriptText(SAY_DEATH_COIL, me);
        }
    }

    void DoTortureThree()
    {
        if (Creature* RandomPeon = GetRandomPeon())
        {
            me->SetFacingTo(me->GetAngleTo(RandomPeon));
            DoCast(RandomPeon, SPELL_SHADOW_FISSURE);
            DoScriptText(SAY_SHADOW_FISSURE, me);
            switch (urand(0, 3))
            {
                case 0:
                    RandomPeon->MonsterYell(-1200493, 0, me->GetGUID());
                    break;
                case 1:
                    RandomPeon->MonsterYell(-1200494, 0, me->GetGUID());
                    break;
                case 2:
                    RandomPeon->MonsterYell(-1200495, 0, me->GetGUID());
                    break;
                case 3:
                    RandomPeon->MonsterYell(-1200496, 0, me->GetGUID());
                    break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!Torture){
            DoTortureOne();
            IntroEvent_Timer = 22000;
            Torture = true;
        }

        if (IsIntroEvent)
        {
            if (IntroEvent_Timer.Expired(diff))
            {
                switch (urand(0, 2)) // select random torture
                {
                    case 0:
                        DoTortureOne();
                        IntroEvent_Timer = 22000;
                        break;
                    case 1:
                        DoTortureTwo();
                        IntroEvent_Timer = 10000;
                        break;
                    case 2:
                        DoTortureThree();
                        IntroEvent_Timer = 8000;
                        break;
                }
            }
        }

        if (!UpdateVictim())
            return;

        if (!IsMainEvent)
            return;

        if (Phase)
        {
            if (!SpinOnce)
            {
                //me->GetUnitStateMgr().PushAction(UNIT_ACTION_ROOT);
                DoCast(me, SPELL_DARK_SPIN);
                SpinOnce = true;
            }

            if (ShadowBolt_Timer.Expired(diff))
            {
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(pTarget, SPELL_SHADOW_BOLT);
                ShadowBolt_Timer = 1000;
            }

            if (DarkCleave_Timer.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_DARK_CLEAVE);
                DarkCleave_Timer = 1000;
            }
        }
        else
        {
            if (Cleave_Timer.Expired(diff))
            {
                DoCast(me->GetVictim(), (HeroicMode ? H_SPELL_SHADOW_SLAM : SPELL_SHADOW_CLEAVE));
                Cleave_Timer = HeroicMode ? urand(8000, 16000) : urand(16000, 32000);
            }

            if (ShadowFissure_Timer.Expired(diff))
            {
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(pTarget, SPELL_SHADOW_FISSURE);
                ShadowFissure_Timer = 8500;
            }

            if (DeathCoil_Timer.Expired(diff))
            {
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                    DoCast(pTarget, SPELL_DEATH_COIL);
                DeathCoil_Timer = HeroicMode? urand(6000, 12000) : urand(10000, 18000);
            }

            if (me->GetHealthPercent() <= 25)
                Phase = true;

            DoMeleeAttackIfReady();
        }
    }
};

struct mob_fel_orc_convertAI : public ScriptedAI
{
    mob_fel_orc_convertAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer Hemorrhage_Timer;
    Timer Kill_Timer;
    Timer EmoteReset_Timer;
    bool EmoteReset;

    bool NethercurseMob;

    void Reset()
    {
        me->SetNoCallAssistance(true);

        Hemorrhage_Timer.Reset(3000);
        Kill_Timer = 0;
        EmoteReset_Timer = 0;
        EmoteReset = false;

        switch(me->GetDBTableGUIDLow())
        {
            case 59481:
            case 59480:
            case 59478:
            case 59479:
                NethercurseMob = true;
                break;
            default:
                NethercurseMob = false;
                break;
        }

        /*if(NethercurseMob)
            me->SetReactState(REACT_PASSIVE);*/
    }

    void OnAuraRemove(Aura* aur, bool Stack)
    {
        if(aur->GetId() == SPELL_AOE_DEATH_COIL)
            me->GetMotionMaster()->MoveTargetedHome();
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (NethercurseMob)
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterCombat(Unit* who)
    {
        if(who->GetTypeId() == TYPEID_UNIT && !((Creature*)who)->isPet())
            return;

        if(NethercurseMob)
        {
            if (pInstance)
            {
                if (pInstance->GetData64(DATA_NETHEKURSE))
                {
                    Creature *pKurse = Unit::GetCreature(*me,pInstance->GetData64(DATA_NETHEKURSE));
                    if (pKurse)
                        ((boss_grand_warlock_nethekurseAI*)pKurse->AI())->DoYellForPeonEnterCombat();
                }

                if (pInstance->GetData(TYPE_NETHEKURSE) != IN_PROGRESS)
                    pInstance->SetData(TYPE_NETHEKURSE, IN_PROGRESS);
            }
        }
    }

    void OnAuraApply(Aura* aura, Unit* caster, bool addStack)
    {
        if(!EmoteReset && (aura->GetId() == 35951 || aura->GetId() == 32251))
        {
            me->HandleEmoteCommand(387);
            EmoteReset_Timer = 5000;
            EmoteReset = true;
        }
    }

    void JustDied(Unit* killer)
    {
        if(NethercurseMob)
        {
            if (pInstance)
            {
                if (pInstance->GetData64(DATA_NETHEKURSE))
                {
                    Creature *pKurse = Unit::GetCreature(*me,pInstance->GetData64(DATA_NETHEKURSE));
                    if (pKurse)
                        ((boss_grand_warlock_nethekurseAI*)pKurse->AI())->DoYellForPeonDeath();
                }
            }
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 10)
            me->HandleEmoteCommand(11);
        else if (eventType == 11)
            me->HandleEmoteCommand(22);
    }

    void UpdateAI(const uint32 diff)
    {
        if(EmoteReset_Timer.Expired(diff))
        {
            me->HandleEmoteCommand(0);
            EmoteReset_Timer = 0;
            EmoteReset = false;
        }

        if (Kill_Timer.Expired(diff))
        {
            me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            Kill_Timer = 0;
        }

        if (!UpdateVictim())
            return;

        if (Hemorrhage_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HEMORRHAGE);
            Hemorrhage_Timer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};

struct mob_lesser_shadow_fissureAI : public ScriptedAI
{
    mob_lesser_shadow_fissureAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer DespawnTimer;

    bool NethercurseMob;

    void Reset()
    {
        me->SetNoCallAssistance(true);

        if (pInstance)
        {
            if (pInstance->GetData64(DATA_NETHEKURSE))
            {
                Creature *pKurse = Unit::GetCreature(*me,pInstance->GetData64(DATA_NETHEKURSE));
                if (pKurse)
                {
                    if(pKurse->IsInCombat() && pKurse->GetVictim())
                        DespawnTimer.Reset(40000);
                    else
                        DespawnTimer.Reset(6000);
                }
            }
        }
        else
            DespawnTimer.Reset(26000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (DespawnTimer.Expired(diff))
        {
            me->DisappearAndDie();
            DespawnTimer = 0;
        }

        if(pInstance)
        {
            if(HeroicMode)
            {
                if (!me->HasAura(SPELL_CONSUMPTION_H))
                    DoCast(me, SPELL_CONSUMPTION_H, true);
            }
            else
            {
                if (!me->HasAura(SPELL_CONSUMPTION_N))
                    DoCast(me, SPELL_CONSUMPTION_N, true);
            }
        }
        else
        {
            if (!me->HasAura(SPELL_CONSUMPTION_N))
            {
                me->setFaction(14);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                DoCast(me, SPELL_CONSUMPTION_N, true);
            }
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_grand_warlock_nethekurse(Creature* creature)
{
    return new boss_grand_warlock_nethekurseAI (creature);
}

CreatureAI* GetAI_mob_fel_orc_convert(Creature* creature)
{
    return new mob_fel_orc_convertAI (creature);
}

CreatureAI* GetAI_mob_lesser_shadow_fissure(Creature* creature)
{
    return new mob_lesser_shadow_fissureAI (creature);
}

void AddSC_boss_grand_warlock_nethekurse()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_grand_warlock_nethekurse";
    newscript->GetAI = &GetAI_boss_grand_warlock_nethekurse;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_fel_orc_convert";
    newscript->GetAI = &GetAI_mob_fel_orc_convert;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_lesser_shadow_fissure";
    newscript->GetAI = &GetAI_mob_lesser_shadow_fissure;
    newscript->RegisterSelf();
}

