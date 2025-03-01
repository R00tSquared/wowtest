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
SDName: Boss_Mekgineer_Steamrigger
SD%Complete: 90
SDComment:
SDCategory: Coilfang Resevoir, The Steamvault
EndScriptData */

/* ContentData
boss_mekgineer_steamrigger
mob_steamrigger_mechanic
EndContentData */

#include "precompiled.h"
#include "def_steam_vault.h"

#define SAY_MECHANICS               -1545007
#define SAY_AGGRO_1                 -1545008
#define SAY_AGGRO_2                 -1545009
#define SAY_AGGRO_3                 -1545010
#define SAY_AGGRO_4                 -1545011
#define SAY_SLAY_1                  -1545012
#define SAY_SLAY_2                  -1545013
#define SAY_SLAY_3                  -1545014
#define SAY_DEATH                   -1545015

#define SPELL_SUPER_SHRINK_RAY      31485
#define SPELL_SAW_BLADE             31486
#define SPELL_ELECTRIFIED_NET       35107
#define H_SPELL_ENRAGE              41924

#define ENTRY_STREAMRIGGER_MECHANIC 17951

struct SumonPos
{
    float x, y, z;
};

static SumonPos Pos[]=
{
    {-339.99f, -118.05f, -7.827f}
};

struct boss_mekgineer_steamriggerAI : public ScriptedAI
{
    boss_mekgineer_steamriggerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
		m_creature->GetPosition(wLoc);
    }

    ScriptedInstance *pInstance;
    bool HeroicMode;

    Timer_UnCheked Shrink_Timer;
    Timer_UnCheked Saw_Blade_Timer;
    Timer_UnCheked Electrified_Net_Timer;
    Timer_UnCheked Berserk_timer;
	Timer_UnCheked checkTimer;
	WorldLocation wLoc;
    bool Summon75;
    bool Summon50;
    bool Summon25;

    void Reset()
    {
        Shrink_Timer.Reset(20000);
        Saw_Blade_Timer.Reset(15000);
        Electrified_Net_Timer.Reset(10000);
        Berserk_timer.Reset(300000);
		checkTimer.Reset(3000);

        Summon75 = false;
        Summon50 = false;
        Summon25 = false;

        if (pInstance && me->isAlive())
            pInstance->SetData(TYPE_MEKGINEER_STEAMRIGGER, NOT_STARTED);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(TYPE_MEKGINEER_STEAMRIGGER, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2, SAY_SLAY_3), me);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), me);

        if (pInstance)
            pInstance->SetData(TYPE_MEKGINEER_STEAMRIGGER, IN_PROGRESS);
    }

    void SummonMechanichs()
    {
        DoScriptText(SAY_MECHANICS, me);

        me->SummonCreature(ENTRY_STREAMRIGGER_MECHANIC, Pos[0].x,Pos[0].y ,Pos[0].z , 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 240000);
        me->SummonCreature(ENTRY_STREAMRIGGER_MECHANIC, Pos[0].x,Pos[0].y ,Pos[0].z , 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 240000);
        me->SummonCreature(ENTRY_STREAMRIGGER_MECHANIC, Pos[0].x,Pos[0].y ,Pos[0].z , 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 240000);

        if (roll_chance_i(30))
            me->SummonCreature(ENTRY_STREAMRIGGER_MECHANIC, Pos[0].x,Pos[0].y ,Pos[0].z , 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 240000);

        if (roll_chance_i(10))
            me->SummonCreature(ENTRY_STREAMRIGGER_MECHANIC, Pos[0].x,Pos[0].y ,Pos[0].z , 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 240000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
				
		//Evade
        if (checkTimer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 80))
                EnterEvadeMode();
            else
                DoZoneInCombat();
            checkTimer= 3000;
		}

        if (Shrink_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SUPER_SHRINK_RAY);
            Shrink_Timer = 20000;
        }

        if (Saw_Blade_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true, me->getVictimGUID()))
                DoCast(target,SPELL_SAW_BLADE);
            else
                DoCast(me->GetVictim(),SPELL_SAW_BLADE);

            Saw_Blade_Timer = 15000;
        } 

        if (Electrified_Net_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_ELECTRIFIED_NET);
            Electrified_Net_Timer = 10000;
        }
        

        if (Berserk_timer.Expired(diff))
        {
            if (HeroicMode)
                DoCast(me, H_SPELL_ENRAGE);

            Berserk_timer = 300000+rand()%10000;
        }

        if (!Summon75)
        {
            if ((me->GetHealthPercent()) < 75)
            {
                SummonMechanichs();
                Summon75 = true;
            }
        }

        if (!Summon50)
        {
            if ((me->GetHealthPercent()) < 50)
            {
                SummonMechanichs();
                Summon50 = true;
            }
        }

        if (!Summon25)
        {
            if ((me->GetHealthPercent()) < 25)
            {
                SummonMechanichs();
                Summon25 = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_mekgineer_steamrigger(Creature *_Creature)
{
    return new boss_mekgineer_steamriggerAI (_Creature);
}

#define SPELL_DISPEL_MAGIC          17201
#define SPELL_REPAIR (HeroicMode ? 37936 : 31532)

#define MAX_REPAIR_RANGE            (13.0f)

struct mob_steamrigger_mechanicAI : public ScriptedAI
{
    mob_steamrigger_mechanicAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;
    bool HeroicMode;

    Timer_UnCheked Repair_Timer;

    void Reset()
    {
        Repair_Timer.Reset(irand(1500, 2500));
	
        if (Unit* Mekgineer = Unit::GetUnit((*me), pInstance->GetData64(DATA_MEKGINEERSTEAMRIGGER)))
        {
            float angle = RAND(0.0f, 1.5f, 3.1f, 4.6f);
            me->GetMotionMaster()->MoveFollow(Mekgineer, 7.0f, angle);
        }
    }

    void EnterCombat(Unit *who) {}

    void AttackStart(Unit* who)
    {
        if (pInstance && pInstance->GetData(TYPE_MEKGINEER_STEAMRIGGER) == IN_PROGRESS)
            return;

        ScriptedAI::AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Repair_Timer.Expired(diff))
        {
            if (pInstance && pInstance->GetData64(DATA_MEKGINEERSTEAMRIGGER) && pInstance->GetData(TYPE_MEKGINEER_STEAMRIGGER) == IN_PROGRESS)
            {
                if (Unit* Mekgineer = Unit::GetUnit((*me), pInstance->GetData64(DATA_MEKGINEERSTEAMRIGGER)))
                {
                    if (me->IsWithinDistInMap(Mekgineer, MAX_REPAIR_RANGE))
                    {
                        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                        DoCast(Mekgineer, SPELL_REPAIR);
                        Repair_Timer = irand(1500, 3000);
                    }
                }
            }else Repair_Timer = irand(1500, 3000);
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
		
    }

};

CreatureAI* GetAI_mob_steamrigger_mechanic(Creature *_Creature)
{
    return new mob_steamrigger_mechanicAI (_Creature);
}

void AddSC_boss_mekgineer_steamrigger()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_mekgineer_steamrigger";
    newscript->GetAI = &GetAI_boss_mekgineer_steamrigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_steamrigger_mechanic";
    newscript->GetAI = &GetAI_mob_steamrigger_mechanic;
    newscript->RegisterSelf();
}

