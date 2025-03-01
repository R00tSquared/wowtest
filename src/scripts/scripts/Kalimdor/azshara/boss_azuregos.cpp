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
SDName: Boss_Azuregos
SD%Complete: 90
SDComment: Teleport not included, spell reflect not effecting dots (Core problem)
SDCategory: Azshara
EndScriptData */

#include "precompiled.h"

#define SAY_TELEPORT            -1000100

#define SPELL_MARKOFFROST        23182
#define SPELL_MANASTORM          21097
#define SPELL_CHILL              21098
#define SPELL_FROSTBREATH        21099
#define SPELL_REFLECT            22067
#define SPELL_CLEAVE              8255                      //Perhaps not right ID
#define SPELL_ENRAGE             23537

struct boss_azuregosAI : public ScriptedAI
{
    boss_azuregosAI(Creature *c) : ScriptedAI(c) {}

    //Timer MarkOfFrost_Timer;
    Timer ManaStorm_Timer;
    Timer Chill_Timer;
    Timer Breath_Timer;
    Timer Teleport_Timer;
    Timer Reflect_Timer;
    Timer Cleave_Timer;
    bool Enraged;

    void Reset()
    {
        //MarkOfFrost_Timer.Reset(35000);
        ManaStorm_Timer.Reset(5000 + rand()%12000);
        Chill_Timer.Reset(10000 + rand()%20000);
        Breath_Timer.Reset(2000 + rand()%6000);
        Teleport_Timer.Reset(30000);
        Reflect_Timer.Reset(15000 + rand()%15000);
        Cleave_Timer.Reset(7000);
        Enraged = false;
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if(Teleport_Timer.Expired(diff))
        {
            DoScriptText(SAY_TELEPORT, m_creature);
            std::list<HostileReference*>& m_threatlist = m_creature->getThreatManager().getThreatList();
            std::list<HostileReference*>::iterator i = m_threatlist.begin();
            for (i = m_threatlist.begin(); i!= m_threatlist.end();++i)
            {
                Unit* pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid());
                if(pUnit && (pUnit->GetTypeId() == TYPEID_PLAYER))
                {
                    DoTeleportPlayer(pUnit, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ()+3, pUnit->GetOrientation());
                }
            }

            DoResetThreat();
            Teleport_Timer = 30000;
        }

        //        if (MarkOfFrost_Timer.Expired(diff))
        //        {
        //            DoCast(m_creature->GetVictim(),SPELL_MARKOFFROST);
        //            MarkOfFrost_Timer.Delay(25000);
        //        }

        if (Chill_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CHILL);
            Chill_Timer = 13000 + rand()%12000;
        }

        if (Breath_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTBREATH);
            Breath_Timer = 10000 + rand()%5000;
        }

        if (ManaStorm_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0, 60, true))
                DoCast(target,SPELL_MANASTORM);
            ManaStorm_Timer = 7500 + rand()%5000;
        }

        if (Reflect_Timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_REFLECT);
            Reflect_Timer = 20000 + rand()%15000;
        }

        if (Cleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            Cleave_Timer = 7000;
        }

        if (me->GetHealthPercent() < 26 && !Enraged)
        {
            DoCast(m_creature, SPELL_ENRAGE);
            Enraged = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_azuregos(Creature *_Creature)
{
    return new boss_azuregosAI (_Creature);
}

void AddSC_boss_azuregos()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_azuregos";
    newscript->GetAI = &GetAI_boss_azuregos;
    newscript->RegisterSelf();
}

