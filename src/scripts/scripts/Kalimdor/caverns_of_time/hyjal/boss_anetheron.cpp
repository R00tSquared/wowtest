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
#include "def_hyjal.h"
#include "hyjal_trash.h"

#define SPELL_CARRION_SWARM 31306
#define SPELL_SLEEP         31298
#define SPELL_VAMPIRIC_AURA 31317
#define SPELL_INFERNO       31299

#define SAY_ONDEATH  -1200354
#define SOUND_ONDEATH   10982

#define SAY_ONSLAY1  -1200355
#define SAY_ONSLAY2  -1200356
#define SAY_ONSLAY3  -1200357
#define SOUND_ONSLAY1   10981
#define SOUND_ONSLAY2   11038
#define SOUND_ONSLAY3   11039

#define SAY_SWARM1   -1200358
#define SAY_SWARM2   -1200359
#define SOUND_SWARM1    10979
#define SOUND_SWARM2    11037

#define SAY_SLEEP1   -1200360
#define SAY_SLEEP2   -1200361
#define SOUND_SLEEP1    10978
#define SOUND_SLEEP2    11545

#define SAY_INFERNO1 -1200362
#define SAY_INFERNO2 -1200363
#define SOUND_INFERNO1  10980
#define SOUND_INFERNO2  11036

#define SAY_ONAGGRO  -1200364
#define SOUND_ONAGGRO   10977

struct boss_anetheronAI : public hyjal_trashAI
{
    boss_anetheronAI(Creature *c) : hyjal_trashAI(c)
    {
        pInstance = (c->GetInstanceData());
        go = false;
        pos = 0;
    }

    Timer_UnCheked SwarmTimer;
    Timer_UnCheked SleepTimer;
    Timer_UnCheked CheckTimer;
    Timer_UnCheked InfernoTimer;
    bool go;
    uint32 pos;

    void Reset()
    {
        ClearCastQueue();

        damageTaken = 0;
        SwarmTimer.Reset(10000);
        SleepTimer.Reset(60000);
        InfernoTimer.Reset(60000);
        CheckTimer.Reset(3000);

        if(pInstance && IsEvent)
            pInstance->SetData(DATA_ANETHERONEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if(pInstance && IsEvent)
            pInstance->SetData(DATA_ANETHERONEVENT, IN_PROGRESS);

        DoPlaySoundToSet(m_creature, SOUND_ONAGGRO);
        DoYell(SAY_ONAGGRO, LANG_UNIVERSAL, NULL);
        DoCast(m_creature, SPELL_VAMPIRIC_AURA, true);
    }

    void KilledUnit(Unit *victim)
    {
        switch(rand()%3)
        {
            case 0:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY1);
                DoYell(-1200355, LANG_UNIVERSAL, NULL);
                break;
            case 1:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY2);
                DoYell(-1200356, LANG_UNIVERSAL, NULL);
                break;
            case 2:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY3);
                DoYell(-1200357, LANG_UNIVERSAL, NULL);
                break;
        }
    }

    void WaypointReached(uint32 i)
    {
        pos = i;
        if (i == 7 && pInstance)
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_JAINAPROUDMOORE));
            if (target && target->isAlive())
            {
                m_creature->AddThreat(target,0.0);
                AttackStart(target);
            }
            else
            {
                if(target = m_creature->SelectNearbyTarget(200.0))
                    AttackStart(target);
            }
        }
    }

    void JustDied(Unit *Killer)
    {
        hyjal_trashAI::JustDied(Killer);
        if(pInstance && IsEvent)
            pInstance->SetData(DATA_ANETHERONEVENT, DONE);

        DoPlaySoundToSet(m_creature, SOUND_ONDEATH);
        DoYell(-1200354, LANG_UNIVERSAL, NULL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (IsEvent)
        {
            //Must update npc_escortAI
            npc_escortAI::UpdateAI(diff);
            if(!go)
            {
                go = true;
                if(pInstance)
                {
                    AddWaypoint(0, 4896.08,    -1576.35,    1333.65);
                    AddWaypoint(1, 4898.68,    -1615.02,    1329.48);
                    AddWaypoint(2, 4907.12,    -1667.08,    1321.00);
                    AddWaypoint(3, 4963.18,    -1699.35,    1340.51);
                    AddWaypoint(4, 4989.16,    -1716.67,    1335.74);
                    AddWaypoint(5, 5026.27,    -1736.89,    1323.02);
                    AddWaypoint(6, 5037.77,    -1770.56,    1324.36);
                    AddWaypoint(7, 5067.23,    -1789.95,    1321.17);
                    Start(false, true);
                    SetDespawnAtEnd(false);
                }
            }
        }

        if (!m_creature->IsInCombat())
            return;

        if (CheckTimer.Expired(diff))
        {
            DoZoneInCombat();
            m_creature->SetSpeed(MOVE_RUN, 3.0);
            CheckTimer = 2000;
        }

        SwarmTimer.Update(diff);
        SleepTimer.Update(diff);
        InfernoTimer.Update(diff);

        // do not update victim when casting
        if (m_creature->IsNonMeleeSpellCast(true) || !UpdateVictim())
            return;

        if (SwarmTimer.Passed())
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,65,true))
            {
                AddSpellToCast(target, SPELL_CARRION_SWARM, false, true);
                SwarmTimer = 12000+rand()%6000;

                switch(rand()%2)
                {
                    case 0:
                        DoPlaySoundToSet(m_creature, SOUND_SWARM1);
                        DoYell(-1200358, LANG_UNIVERSAL, NULL);
                    break;
                    case 1:
                        DoPlaySoundToSet(m_creature, SOUND_SWARM2);
                        DoYell(-1200359, LANG_UNIVERSAL, NULL);
                    break;
                }
            }
        }

        if (SleepTimer.Passed())
        {
            DoCast(m_creature, SPELL_SLEEP, true);

            SleepTimer = 60000;

            switch(rand()%2)
            {
                case 0:
                    DoPlaySoundToSet(m_creature, SOUND_SLEEP1);
                    DoYell(-1200360, LANG_UNIVERSAL, NULL);
                    break;
                case 1:
                    DoPlaySoundToSet(m_creature, SOUND_SLEEP2);
                    DoYell(-1200361, LANG_UNIVERSAL, NULL);
                    break;
            }
        }
        

        if (InfernoTimer.Passed())
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0,200,true))
            {
                m_creature->CastStop();
                AddSpellToCast(target, SPELL_INFERNO, false, true);

                switch(rand()%2)
                {
                    case 0:
                        DoPlaySoundToSet(m_creature, SOUND_INFERNO1);
                        DoYell(-1200362, LANG_UNIVERSAL, NULL);
                    break;
                    case 1:
                        DoPlaySoundToSet(m_creature, SOUND_INFERNO2);
                        DoYell(-1200363, LANG_UNIVERSAL, NULL);
                    break;
                }
                InfernoTimer = 60000;
            }
        }
        

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_anetheron(Creature *_Creature)
{
    return new boss_anetheronAI (_Creature);
}

#define SPELL_IMMOLATION 31304
#define SPELL_INFERNO_EFFECT 31302

struct mob_towering_infernalAI : public ScriptedAI
{
    mob_towering_infernalAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    Timer_UnCheked CheckTimer;
    Timer_UnCheked WaitTimer;
    ScriptedInstance* pInstance;

    void Reset()
    {
        m_creature->setFaction(1720);
        m_creature->ApplySpellImmune(2, IMMUNITY_MECHANIC, MECHANIC_BANISH, true);
        CheckTimer.Reset(5000);
        WaitTimer.Reset(3500);
    }

    void JustRespawned()
    {
        DoCast(m_creature, SPELL_INFERNO_EFFECT);
        DoCast(m_creature, SPELL_IMMOLATION);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!m_creature->IsInCombat() && m_creature->IsWithinDistInMap(who, 50) && m_creature->IsHostileTo(who))
        {
            m_creature->AddThreat(who,0.0);
            m_creature->Attack(who,false);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        
        if (CheckTimer.Expired(diff))
        {
            if(pInstance)
            {
                Creature *pAnetheron = pInstance->GetCreature(pInstance->GetData64(DATA_ANETHERON));
                if(!pAnetheron || !pAnetheron->isAlive())
                {
                    m_creature->setDeathState(JUST_DIED);
                    m_creature->RemoveCorpse();
                    return;
                }
            }
            CheckTimer = 2000;
        }
        

        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (!WaitTimer.Expired(diff)) //FIXME: not sure if that will work :p
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_towering_infernal(Creature *_Creature)
{
    return new mob_towering_infernalAI (_Creature);
}

void AddSC_boss_anetheron()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_anetheron";
    newscript->GetAI = &GetAI_boss_anetheron;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_towering_infernal";
    newscript->GetAI = &GetAI_mob_towering_infernal;
    newscript->RegisterSelf();
}
