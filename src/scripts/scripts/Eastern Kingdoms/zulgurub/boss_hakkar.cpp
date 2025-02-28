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
SDName: Boss_Hakkar
SD%Complete: 95
SDComment: Blood siphon spell buggy cause of Core Issue.
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SAY_AGGRO                   -1309020
#define SAY_FLEEING                 -1309021
#define SAY_MINION_DESTROY          -1309022                //where does it belong?
#define SAY_PROTECT_ALTAR           -1309023                //where does it belong?

#define SPELL_BLOODSIPHON            24322
#define SPELL_CORRUPTEDBLOOD         24328
#define SPELL_CAUSEINSANITY          24327                  //Not working disabled. // must be used on TANK. Should drop aggro from tank with this, but regain aggro later.
#define SPELL_WILLOFHAKKAR           24178
#define SPELL_ENRAGE                 24318

// The Aspects of all High Priests
#define SPELL_ASPECT_OF_JEKLIK       24687
#define SPELL_ASPECT_OF_VENOXIS      24688
#define SPELL_ASPECT_OF_MARLI        24686
#define SPELL_ASPECT_OF_THEKAL       24689
#define SPELL_ASPECT_OF_ARLOKK       24690

struct boss_hakkarAI : public ScriptedAI
{
    boss_hakkarAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer BloodSiphon_Timer;
    Timer CorruptedBlood_Timer;
    Timer CauseInsanity_Timer;
    Timer WillOfHakkar_Timer;
    Timer Enrage_Timer;

    Timer CheckJeklik_Timer;
    Timer CheckVenoxis_Timer;
    Timer CheckMarli_Timer;
    Timer CheckThekal_Timer;
    Timer CheckArlokk_Timer;

    Timer AspectOfJeklik_Timer;
    Timer AspectOfVenoxis_Timer;
    Timer AspectOfMarli_Timer;
    Timer AspectOfThekal_Timer;
    Timer AspectOfArlokk_Timer;

    bool Enraged;

    void Reset()
    {
        BloodSiphon_Timer.Reset(90000);
        CorruptedBlood_Timer.Reset(25000);
        CauseInsanity_Timer.Reset(17000);
        WillOfHakkar_Timer.Reset(17000);
        Enrage_Timer.Reset(600000);

        CheckJeklik_Timer.Reset(1000);
        CheckVenoxis_Timer.Reset(2000);
        CheckMarli_Timer.Reset(3000);
        CheckThekal_Timer.Reset(4000);
        CheckArlokk_Timer.Reset(5000);

        AspectOfJeklik_Timer.Reset(4000);
        AspectOfVenoxis_Timer.Reset(7000);
        AspectOfMarli_Timer.Reset(12000);
        AspectOfThekal_Timer.Reset(8000);
        AspectOfArlokk_Timer.Reset(18000);

        Enraged = false;

        if(pInstance)
            pInstance->SetData(DATA_HAKKAREVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        if(pInstance)
            pInstance->SetData(DATA_HAKKAREVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if(pInstance)
            pInstance->SetData(DATA_HAKKAREVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (BloodSiphon_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_BLOODSIPHON);
            BloodSiphon_Timer = 90000;
        }

        if (CorruptedBlood_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CORRUPTEDBLOOD);
            CorruptedBlood_Timer = 30000 + rand()%15000;
        }

        /*
        if (CauseInsanity_Timer.Expired(diff))
        {
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
        DoCast(target,SPELL_CAUSEINSANITY);

        CauseInsanity_Timer = 35000 + rand()%8000;
        }*/

        if (WillOfHakkar_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,GetSpellMaxRange(SPELL_WILLOFHAKKAR), true))
                DoCast(target,SPELL_WILLOFHAKKAR);

            WillOfHakkar_Timer = 25000 + rand()%10000;
        }

        if (!Enraged && Enrage_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_ENRAGE);
            Enraged = true;
        }

        if(CheckJeklik_Timer.Expired(diff))
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_JEKLIKEVENT) != DONE)
                {
                    if (AspectOfJeklik_Timer.Expired(diff))
                    {
                        DoCast(m_creature->GetVictim(),SPELL_ASPECT_OF_JEKLIK);
                        AspectOfJeklik_Timer = 10000 + rand()%4000;
                    }
                }
            }
            CheckJeklik_Timer = 1000;
        }
        
        //Checking if Venoxis is dead. If not we cast his Aspect
        if(CheckVenoxis_Timer.Expired(diff))
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_VENOXISEVENT) != DONE)
                {
                    if (AspectOfVenoxis_Timer.Expired(diff))
                    {
                        DoCast(m_creature->GetVictim(),SPELL_ASPECT_OF_VENOXIS);
                        AspectOfVenoxis_Timer = 8000;
                    }
                }
            }
            CheckVenoxis_Timer = 1000;
        }

        //Checking if Marli is dead. If not we cast her Aspect
        if(CheckMarli_Timer.Expired(diff))
        {
            if(pInstance)
            {
                if (pInstance->GetData(DATA_MARLIEVENT) != DONE)
                {
                    if (AspectOfMarli_Timer.Expired(diff))
                    {
                        DoCast(m_creature->GetVictim(),SPELL_ASPECT_OF_MARLI);
                        AspectOfMarli_Timer = 10000;
                    }

                }
            }
            CheckMarli_Timer = 1000;
        }

        //Checking if Thekal is dead. If not we cast his Aspect
        if(CheckThekal_Timer.Expired(diff))
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_THEKALEVENT) != DONE)
                {
                    if (AspectOfThekal_Timer.Expired(diff))
                    {
                        DoCast(m_creature,SPELL_ASPECT_OF_THEKAL);
                        AspectOfThekal_Timer = 15000;
                    }
                }
            }
            CheckThekal_Timer = 1000;
        }

        //Checking if Arlokk is dead. If yes we cast her Aspect
        if(CheckArlokk_Timer.Expired(diff))
        {
            if(pInstance)
            {
                if(pInstance->GetData(DATA_ARLOKKEVENT) != DONE)
                {
                    if (AspectOfArlokk_Timer.Expired(diff))
                    {
                        DoCast(m_creature,SPELL_ASPECT_OF_ARLOKK);
                        // DoResetThreat();

                        AspectOfArlokk_Timer = 10000 + rand()%5000;
                    }
                }
            }
            CheckArlokk_Timer = 1000;
        }
        

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_hakkar(Creature *_Creature)
{
    return new boss_hakkarAI (_Creature);
}

void AddSC_boss_hakkar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_hakkar";
    newscript->GetAI = &GetAI_boss_hakkar;
    newscript->RegisterSelf();
}


