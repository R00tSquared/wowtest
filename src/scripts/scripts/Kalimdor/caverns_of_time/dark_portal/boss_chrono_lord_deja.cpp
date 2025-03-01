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
SDName: Boss_Chrono_Lord_Deja
SD%Complete: 99
SDComment: Some timers may not be completely Blizzlike
SDCategory: Caverns of Time, The Dark Portal
EndScriptData */

#include "precompiled.h"
#include "def_dark_portal.h"

#define SAY_ENTER                   -1269006
#define SAY_AGGRO                   -1269007
#define SAY_BANISH                  -1269008
#define SAY_SLAY1                   -1269009
#define SAY_SLAY2                   -1269010
#define SAY_DEATH                   -1269011

#define SPELL_ARCANE_BLAST          31457
#define H_SPELL_ARCANE_BLAST        38538
#define SPELL_ARCANE_DISCHARGE      31472
#define H_SPELL_ARCANE_DISCHARGE    38539
#define SPELL_TIME_LAPSE            31467
#define SPELL_ATTRACTION            38540

struct boss_chrono_lord_dejaAI : public ScriptedAI
{
    boss_chrono_lord_dejaAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }

    ScriptedInstance *pInstance;
    bool HeroicMode;

    Timer ArcaneBlast_Timer;
    Timer ArcaneDischarge_Timer;
    Timer Attraction_Timer;
    Timer TimeLapse_Timer;

    bool arcane;

    void Reset()
    {
        if (HeroicMode)
        {
            GetSpellRangeStore();
            ArcaneBlast_Timer.Reset(2000);
            Attraction_Timer.Reset(18000);
        }
        else
            ArcaneBlast_Timer.Reset(20000);

        ArcaneDischarge_Timer.Reset(10000);
        TimeLapse_Timer.Reset(15000);
        arcane = false;
        m_creature->setActive(true);

        SayIntro();
        if(pInstance)
            pInstance->SetData(TYPE_DEJA_2, NOT_STARTED);
    }

    void SayIntro()
    {
        DoScriptText(SAY_ENTER, m_creature);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        if(pInstance)
            pInstance->SetData(TYPE_DEJA_2, IN_PROGRESS);
    }

    void MoveInLineOfSight(Unit *who)
    {
        //Despawn Time Keeper
        if (who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == C_TIME_KEEPER)
        {
            if (me->isAlive() && m_creature->IsWithinDistInMap(who,20.0f))
            {
                DoScriptText(SAY_BANISH, m_creature);
                who->ToCreature()->ForcedDespawn();
            }
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), m_creature);
    }

    void JustDied(Unit *victim)
    {
        if (pInstance->GetData(TYPE_MEDIVH) != FAIL)
            DoScriptText(SAY_DEATH, m_creature);

        if(pInstance)
        {
            pInstance->SetData(TYPE_RIFT,SPECIAL);
            pInstance->SetData(TYPE_C_DEJA,DONE);
            pInstance->SetData(TYPE_DEJA_2, DONE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        //Arcane Blast && Attraction on heroic mode
        if (!HeroicMode)
        {
            if (ArcaneBlast_Timer.Expired(diff))
            {
                AddSpellToCast(m_creature->GetVictim(), SPELL_ARCANE_BLAST, true);
                ArcaneBlast_Timer = urand(20000, 25000);
            }
        }
        else
        {
            if (Attraction_Timer.Expired(diff))
            {
                if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_ATTRACTION), true))
                    if (!arcane)
                    {
                        AddSpellToCast(target, SPELL_ATTRACTION, true);
                        arcane = true;
                    }


                if (ArcaneBlast_Timer.Expired(diff))
                {
                    AddSpellToCast(m_creature->GetVictim(), H_SPELL_ARCANE_BLAST, true);

                    arcane = false;
                    Attraction_Timer = urand(18000, 23000);
                    ArcaneBlast_Timer = 2000;
                }
            }
        }

        if (ArcaneDischarge_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, HeroicMode ? H_SPELL_ARCANE_DISCHARGE : SPELL_ARCANE_DISCHARGE);
            ArcaneDischarge_Timer = urand(15000, 25000);
        }

        if (TimeLapse_Timer.Expired(diff))
        {
            AddSpellToCastWithScriptText(m_creature, SPELL_TIME_LAPSE, SAY_BANISH);
            TimeLapse_Timer = urand(15000, 25000);
        }

        //if event failed, remove boss from instance
        if (pInstance->GetData(TYPE_MEDIVH) == FAIL)
        {
            m_creature->Kill(m_creature, false);
            m_creature->RemoveCorpse();
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_chrono_lord_deja(Creature *_Creature)
{
    return new boss_chrono_lord_dejaAI (_Creature);
}

void AddSC_boss_chrono_lord_deja()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_chrono_lord_deja";
    newscript->GetAI = &GetAI_boss_chrono_lord_deja;
    newscript->RegisterSelf();
}
