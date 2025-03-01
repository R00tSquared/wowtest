// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Grandmaster_Vorpil
SD%Complete: 100
SDComment:
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "precompiled.h"
#include "def_shadow_labyrinth.h"

#define SAY_INTRO                       -1555028
#define SAY_AGGRO1                      -1555029
#define SAY_AGGRO2                      -1555030
#define SAY_AGGRO3                      -1555031
#define SAY_HELP                        -1555032
#define SAY_SLAY1                       -1555033
#define SAY_SLAY2                       -1555034
#define SAY_DEATH                       -1555035

#define SPELL_RAIN_OF_FIRE          33617
#define H_SPELL_RAIN_OF_FIRE        39363

#define SPELL_DRAW_SHADOWS          33563
#define SPELL_SHADOWBOLT_VOLLEY     33841
#define SPELL_BANISH                38791

#define MOB_VOID_TRAVELER           19226
#define SPELL_SACRIFICE             33587
#define SPELL_SHADOW_NOVA           33846
#define SPELL_EMPOWERING_SHADOWS    33783
#define H_SPELL_EMPOWERING_SHADOWS  39364

#define MOB_VOID_PORTAL             19224
#define SPELL_VOID_PORTAL_VISUAL    33569

float VorpilPosition[3] = {-252.8820,-264.3030,17.1};

float VoidPortalCoords[5][3] =
{
    {-283.5894, -239.5718, 12.7},
    {-306.5853, -258.4539, 12.7},
    {-295.8789, -269.0899, 12.7},
    {-209.3401, -262.7564, 17.1},
    {-261.4533, -297.3298, 17.1}
};

class EmpoweringShadowsAura: public Aura
{
    public:
        EmpoweringShadowsAura(SpellEntry *spell, uint32 eff, int32 *bp, Unit *target, Unit *caster) : Aura(spell, eff, bp, target, caster, NULL) {}
};

struct mob_voidtravelerAI : public ScriptedAI
{
    mob_voidtravelerAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = me->GetMap()->IsHeroic();
    }

    bool HeroicMode;
    uint64 VorpilGUID;
    Timer MoveTimer;
    bool sacrificed;

    void Reset()
    {
        VorpilGUID = 0;
        MoveTimer.Reset(1);
        sacrificed = false;
        me->setActive(true);
    }

    void EnterCombat(Unit *who){}

    void UpdateAI(const uint32 diff)
    {
        /*
        if(Unit *Vorpil = Unit::GetUnit(*me, VorpilGUID))
        {
            me->DealDamage(me, me->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            return;
        }
        */

        if (MoveTimer.Expired(diff))
        {
            Unit *Vorpil = Unit::GetUnit(*me, VorpilGUID);
            if(!Vorpil)
                return;

            if(sacrificed)
            {
                Vorpil->CastSpell(Vorpil, HeroicMode?H_SPELL_EMPOWERING_SHADOWS:SPELL_EMPOWERING_SHADOWS, true);
                DoCast(me, SPELL_SHADOW_NOVA, true);
                //me->DealDamage(me, me->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                me->setDeathState(JUST_DIED);
                me->RemoveCorpse();
                return;
            }

            me->GetMotionMaster()->MoveFollow(Vorpil,0,0);
            me->SetSpeed(MOVE_RUN, 0.3f, true);

            if(me->IsWithinDistInMap(Vorpil, 3))
            {
                DoCast(me, SPELL_SACRIFICE, false);
                sacrificed = true;
                MoveTimer = 500;
                return;
            }

            if(!Vorpil->IsInCombat() || Vorpil->isDead())
            {
                me->DealDamage(me, me->GetMaxHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                return;
            }
            MoveTimer = 1000;
        }
    }
};
CreatureAI* GetAI_mob_voidtraveler(Creature *_Creature)
{
    return new mob_voidtravelerAI (_Creature);
}

struct boss_grandmaster_vorpilAI : public ScriptedAI
{
    boss_grandmaster_vorpilAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
        Intro = false;
    }

    ScriptedInstance *pInstance;
    bool Intro, HelpYell;
    bool sumportals;
    bool HeroicMode;

    Timer ShadowBoltVolley_Timer;
    Timer DrawShadows_Timer;
    Timer summonTraveler_Timer;
    Timer banish_Timer;
    Timer Emotion_Timer;
    uint64 PortalsGuid[5];

    void Reset()
    {
        ShadowBoltVolley_Timer.Reset(15000);
        DrawShadows_Timer.Reset(45000);
        summonTraveler_Timer.Reset(90000);
        banish_Timer.Reset(17000);
        Emotion_Timer.Reset(1000);
        HelpYell = false;
        destroyPortals();

        if(pInstance)
            pInstance->SetData(DATA_GRANDMASTERVORPILEVENT, NOT_STARTED);
    }

    void summonPortals()
    {
        if(!sumportals)
        {
            for (int i = 0;i<5;i++)
            {
                Creature *Portal = NULL;
                Portal = me->SummonCreature(MOB_VOID_PORTAL,VoidPortalCoords[i][0],VoidPortalCoords[i][1],VoidPortalCoords[i][2],0,TEMPSUMMON_CORPSE_DESPAWN,3000000);
                if(Portal)
                {
                    PortalsGuid[i] = Portal->GetGUID();
                    Portal->CastSpell(Portal,SPELL_VOID_PORTAL_VISUAL,false);
                }
            }
            sumportals = true;
            summonTraveler_Timer = 5000;
        }
    }

    void destroyPortals()
    {
        if(sumportals)
        {
            for (int i = 0;i < 5; i ++)
            {
                Unit *Portal = Unit::GetUnit((*me), PortalsGuid[i]);
                if (Portal && Portal->isAlive())
                    Portal->DealDamage(Portal, Portal->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                PortalsGuid[i] = 0;
            }
            sumportals = false;
        }
    }

    void spawnVoidTraveler()
    {
        int pos = rand()%5;
        me->SummonCreature(MOB_VOID_TRAVELER,VoidPortalCoords[pos][0],VoidPortalCoords[pos][1],VoidPortalCoords[pos][2],0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,5000);
        if(!HelpYell)
        {
            DoScriptText(SAY_HELP, me);
            HelpYell = true;
        }
    }

    void JustSummoned(Creature *summoned)
    {
        if (summoned && summoned->GetEntry() == MOB_VOID_TRAVELER)
            ((mob_voidtravelerAI*)summoned->AI())->VorpilGUID = me->GetGUID();
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);
        destroyPortals();

        if(pInstance)
            pInstance->SetData(DATA_GRANDMASTERVORPILEVENT, DONE);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);

        summonPortals();

        if(pInstance)
            pInstance->SetData(DATA_GRANDMASTERVORPILEVENT, IN_PROGRESS);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(who && !me->GetVictim() && me->canStartAttack(who))
            AttackStart(who);
        if (!Intro && who && me->IsWithinDistInMap(who, 100) && me->IsHostileTo(who) && me->IsWithinLOSInMap(who))
        {
            DoScriptText(SAY_INTRO, me);
            Intro = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if(Emotion_Timer.Expired(diff))
            {
                me->HandleEmoteCommand(1);
                Emotion_Timer = urand(3000, 5000);
            }
            return;
        }

        if (ShadowBoltVolley_Timer.Expired(diff))
        {
            DoCast(me,SPELL_SHADOWBOLT_VOLLEY);
            ShadowBoltVolley_Timer = 15000;
        }
        

        if (HeroicMode && banish_Timer.Expired(diff))
        {
            Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0,30,false);
            if (target)
            {
                DoCast(target,SPELL_BANISH);
                banish_Timer = 16000;
            }
        }

        if (DrawShadows_Timer.Expired(diff))
        {
            Map *map = me->GetMap();
            Map::PlayerList const &PlayerList = map->GetPlayers();
            for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                if (Player* i_pl = i->getSource())
                    if (i_pl->isAlive() && !i_pl->HasAura(SPELL_BANISH,0) && !i_pl->isGameMaster())
                        i_pl->TeleportTo(me->GetMapId(), VorpilPosition[0],VorpilPosition[1],VorpilPosition[2], 0, TELE_TO_NOT_LEAVE_COMBAT);

            me->Relocate(VorpilPosition[0],VorpilPosition[1],VorpilPosition[2]);
            DoCast(me,SPELL_DRAW_SHADOWS,true);

            DoCast(me,HeroicMode?H_SPELL_RAIN_OF_FIRE:SPELL_RAIN_OF_FIRE);

            ShadowBoltVolley_Timer = 6000;
            DrawShadows_Timer = 30000;
        }
        

        if (summonTraveler_Timer.Expired(diff))
        {
            spawnVoidTraveler();
            summonTraveler_Timer = 10000;
            //enrage at 20%
            if((me->GetHealth()*5) < me->GetMaxHealth())
                summonTraveler_Timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_grandmaster_vorpil(Creature *_Creature)
{
    return new boss_grandmaster_vorpilAI (_Creature);
}

void AddSC_boss_grandmaster_vorpil()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_grandmaster_vorpil";
    newscript->GetAI = &GetAI_boss_grandmaster_vorpil;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_voidtraveler";
    newscript->GetAI = &GetAI_mob_voidtraveler;
    newscript->RegisterSelf();
}

