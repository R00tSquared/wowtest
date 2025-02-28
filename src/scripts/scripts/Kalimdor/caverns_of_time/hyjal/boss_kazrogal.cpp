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

#define SPELL_CLEAVE   31436
#define SPELL_WARSTOMP 38911
#define SPELL_MARK     31447
#define SPELL_CRIPPLE  31477

#define SOUND_ONDEATH 11018

#define SAY_ONSLAY1 -1200369
#define SAY_ONSLAY2 -1200370
#define SAY_ONSLAY3 -1200371
#define SOUND_ONSLAY1 11017
#define SOUND_ONSLAY2 11053
#define SOUND_ONSLAY3 11054

#define SAY_MARK1   -1200372
#define SAY_MARK2   -1200373
#define SOUND_MARK1 11016
#define SOUND_MARK2 11052

#define SAY_ONAGGRO -1200374
#define SOUND_ONAGGRO 11015

struct boss_kazrogalAI : public hyjal_trashAI
{
    boss_kazrogalAI(Creature *c) : hyjal_trashAI(c)
    {
        pInstance = (c->GetInstanceData());
        go = false;
        pos = 0;
    }

    Timer_UnCheked CleaveTimer;
    Timer_UnCheked WarStompTimer;
    Timer_UnCheked MarkTimer;
    uint32 MarkTimerBase;
    Timer_UnCheked CheckTimer;
    Timer_UnCheked CrippleTimer;

    bool go;
    uint32 pos;

    void Reset()
    {
        damageTaken = 0;
        CleaveTimer.Reset(5000);
        WarStompTimer.Reset(15000);
        MarkTimer.Reset(45000);
        MarkTimerBase = 45000;
        CheckTimer.Reset(3000);
        CrippleTimer.Reset(15000 + rand() % 10000);

        if(pInstance && IsEvent)
            pInstance->SetData(DATA_KAZROGALEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if(pInstance && IsEvent)
            pInstance->SetData(DATA_KAZROGALEVENT, IN_PROGRESS);

        DoPlaySoundToSet(m_creature, SOUND_ONAGGRO);
        DoYell(-1200374, LANG_UNIVERSAL, NULL);
    }

    void KilledUnit(Unit *victim)
    {
        switch(rand()%3)
        {
            case 0:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY1);
                DoYell(-1200369, LANG_UNIVERSAL, NULL);
                break;
            case 1:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY2);
                DoYell(-1200370, LANG_UNIVERSAL, NULL);
                break;
            case 2:
                DoPlaySoundToSet(m_creature, SOUND_ONSLAY3);
                DoYell(-1200371, LANG_UNIVERSAL, NULL);
                break;
        }
    }

    void WaypointReached(uint32 i)
    {
        pos = i;
        if (i == 7 && pInstance)
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_THRALL));
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
            pInstance->SetData(DATA_KAZROGALEVENT, DONE);
        DoPlaySoundToSet(m_creature, SOUND_ONDEATH);
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
                    AddWaypoint(0, 5492.91,    -2404.61,    1462.63);
                    AddWaypoint(1, 5531.76,    -2460.87,    1469.55);
                    AddWaypoint(2, 5554.58,    -2514.66,    1476.12);
                    AddWaypoint(3, 5554.16,    -2567.23,    1479.90);
                    AddWaypoint(4, 5540.67,    -2625.99,    1480.89);
                    AddWaypoint(5, 5508.16,    -2659.2,    1480.15);
                    AddWaypoint(6, 5489.62,    -2704.05,    1482.18);
                    AddWaypoint(7, 5457.04,    -2726.26,    1485.10);
                    Start(false, true);
                    SetDespawnAtEnd(false);
                }
            }
        }

        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (CheckTimer.Expired(diff))
        {
            DoZoneInCombat();
            m_creature->SetSpeed(MOVE_RUN, 3.0);
            CheckTimer = 3000;
        }

        if (CleaveTimer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_CLEAVE);
            CleaveTimer = 6000+rand()%15000;
        }

        if (WarStompTimer.Expired(diff))
        {
            DoCast(m_creature, SPELL_WARSTOMP);
            WarStompTimer = 60000;
        }

        if (CrippleTimer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 1, 20, true))
                DoCast(target, SPELL_CRIPPLE);

            CrippleTimer = 20000+rand()%10000;
        }

        if (MarkTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature, SPELL_MARK, false);

            MarkTimerBase -= 5000;

            if(MarkTimerBase <= 5500)
                MarkTimerBase = 10500;

            MarkTimer = MarkTimerBase;
            switch(rand()%3)
            {
                case 0:
                    DoPlaySoundToSet(m_creature, SOUND_MARK1);
                    DoYell(-1200372, LANG_UNIVERSAL, NULL);
                    break;
                case 1:
                    DoPlaySoundToSet(m_creature, SOUND_MARK2);
                    DoYell(-1200373, LANG_UNIVERSAL, NULL);
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_kazrogal(Creature *_Creature)
{
    return new boss_kazrogalAI (_Creature);
}

void AddSC_boss_kazrogal()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_kazrogal";
    newscript->GetAI = &GetAI_boss_kazrogal;
    newscript->RegisterSelf();
}
