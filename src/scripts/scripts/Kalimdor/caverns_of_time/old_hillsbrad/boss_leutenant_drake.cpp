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
SDName: Boss_Luetenant_Drake
SD%Complete: 99
SDComment:
SDCategory: Caverns of Time, Old Hillsbrad Foothills
EndScriptData */

#include "precompiled.h"
#include "def_old_hillsbrad.h"
#include "escort_ai.h"

/*######
## boss_lieutenant_drake
######*/

enum LieutinantDrake
{
    SAY_AGGRO                   = -1560007,
    SAY_SLAY1                   = -1560008,
    SAY_SLAY2                   = -1560009,
    SAY_MORTAL                  = -1560010,
    SAY_SHOUT                   = -1560011,
    SAY_DEATH                   = -1560012,

    SPELL_WHIRLWIND             = 31909,
    SPELL_HAMSTRING             = 9080,
    SPELL_MORTAL_STRIKE         = 31911,
    SPELL_FRIGHTENING_SHOUT     = 33789,
    SPELL_EXPLODING_SHOT        = 33792
};

struct Location
{
    uint32 wpId;
    float x;
    float y;
    float z;
};

static Location DrakeWP[] =
{
    { 0, 2125.84, 88.2535, 54.8830 },
    { 1, 2111.01, 93.8022, 52.6356 },
    { 2, 2106.70, 114.753, 53.1965 },
    { 3, 2107.76, 138.746, 52.5109 },
    { 4, 2114.83, 160.142, 52.4738 },
    { 5, 2125.24, 178.909, 52.7283 },
    { 6, 2151.02, 208.901, 53.1551 },
    { 7, 2177.00, 233.069, 52.4409 },
    { 8, 2190.71, 227.831, 53.2742 },
    { 9, 2178.14, 214.219, 53.0779 },
    { 10, 2154.99, 202.795, 52.6446 },
    { 11, 2132.00, 191.834, 52.5709 },
    { 12, 2117.59, 166.708, 52.7686 },
    { 13, 2093.61, 139.441, 52.7616 },
    { 14, 2086.29, 104.950, 52.9246 },
    { 15, 2094.23, 81.2788, 52.6946 },
    { 16, 2108.70, 85.3075, 53.3294 }
};

struct boss_lieutenant_drakeAI : public ScriptedAI
{
    boss_lieutenant_drakeAI(Creature *creature) : ScriptedAI(creature)
    {
        pInstance = creature->GetInstanceData();
    }

    ScriptedInstance * pInstance;

    bool WaypointReached;
    uint32 wpId;

    Timer Whirlwind_Timer;
    Timer Fear_Timer;
    Timer MortalStrike_Timer;
    Timer ExplodingShot_Timer;
    Timer Hamstring_Timer;

    void Reset()
    {
        WaypointReached = false;
        wpId = 0;
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(DrakeWP[wpId].wpId, DrakeWP[wpId].x, DrakeWP[wpId].y, DrakeWP[wpId].z);
        Whirlwind_Timer.Reset(15000);
        Fear_Timer.Reset(30000);
        MortalStrike_Timer.Reset(10000);
        ExplodingShot_Timer.Reset(20000);
        Hamstring_Timer.Reset(8000);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STUN, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_KNOCKOUT, true);
        if(pInstance)
            pInstance->SetData(DATA_LEUTENANT_DRAKE, NOT_STARTED);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (!me->IsInCombat())
            {
                ++wpId;
                WaypointReached = true;

                if (wpId == 16)
                    wpId = 2;
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        me->SetPlayerDamageReqInPct(0.05); // should be lootable for players even if thrall damage was higher
        DoScriptText(SAY_AGGRO, me);
        me->StopMoving();
        me->SetWalk(false);
        if(pInstance)
            pInstance->SetData(DATA_LEUTENANT_DRAKE, IN_PROGRESS);
    }

    void EnterEvadeMode()
    {
        me->InterruptNonMeleeSpells(true);
        me->RemoveAllAuras();
        me->DeleteThreatList();
        me->CombatStop(true);
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(0, me->GetPositionX() - 1.0f, me->GetPositionY() + 1.0f, me->GetPositionZ());
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance->GetData(DATA_DRAKE_DEATH) == DONE)
            me->SetLootRecipient(NULL);
        else
            pInstance->SetData(DATA_DRAKE_DEATH, DONE);
        if(pInstance)
            pInstance->SetData(DATA_LEUTENANT_DRAKE, DONE);

    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
        {
            if (WaypointReached)
            {
                me->GetMotionMaster()->MovePoint(DrakeWP[wpId].wpId, DrakeWP[wpId].x, DrakeWP[wpId].y, DrakeWP[wpId].z);
                WaypointReached = false;
            }
            return;
        }

        if (Whirlwind_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_WHIRLWIND);
            Whirlwind_Timer = urand(15000, 25000);
        }

        if (Fear_Timer.Expired(diff))
        {
            DoScriptText(SAY_SHOUT, me);
            DoCast(me->GetVictim(), SPELL_FRIGHTENING_SHOUT);
            Fear_Timer = urand(15000, 30000);
        }

        if (MortalStrike_Timer.Expired(diff))
        {
            DoScriptText(SAY_MORTAL, me);
            DoCast(me->GetVictim(), SPELL_MORTAL_STRIKE);
            MortalStrike_Timer = urand(15000, 20000);
        }

        if (Hamstring_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HAMSTRING);
            Hamstring_Timer = urand(10000, 20000);
        }

        if (ExplodingShot_Timer.Expired(diff))
        {
            if(!me->IsWithinMeleeRange(me->GetVictim()))
            {
                DoCast(me->GetVictim(), SPELL_EXPLODING_SHOT);
                ExplodingShot_Timer = urand(15000, 20000);
            }
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_lieutenant_drake(Creature *creature)
{
    return new boss_lieutenant_drakeAI(creature);
}

void AddSC_boss_lieutenant_drake()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_lieutenant_drake";
    newscript->GetAI = &GetAI_boss_lieutenant_drake;
    newscript->RegisterSelf();
}

