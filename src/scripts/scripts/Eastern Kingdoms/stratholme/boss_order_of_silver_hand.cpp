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
SDName: Boss_Silver_Hand_Bosses
SD%Complete: 40
SDComment: Basic script to have support for Horde paladin epic mount (quest 9737). All 5 members of Order of the Silver Hand running this script (least for now)
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "def_stratholme.h"

/*#####
# Additional:
# Although this is a working solution, the correct would be in addition to check if Aurius is dead.
# Once player extinguish the eternal flame (cast spell 31497->start event 11206) Aurius should become hostile.
# Once Aurius is defeated, he should be the one summoning the ghosts.
#####*/

#define SH_GREGOR           17910
#define SH_CATHELA          17911
#define SH_NEMAS            17912
#define SH_AELMAR           17913
#define SH_VICAR            17914
#define SH_QUEST_CREDIT     17915

#define SPELL_HOLY_LIGHT    25263
#define SPELL_DIVINE_SHIELD 13874

struct boss_silver_hand_bossesAI : public ScriptedAI
{
    boss_silver_hand_bossesAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer _ChangedNameHolyLight_Timer;
    Timer _ChangedNameDivineShield_Timer;

    void Reset()
    {
        _ChangedNameHolyLight_Timer.Reset(20000);
        _ChangedNameDivineShield_Timer.Reset(20000);

        if(pInstance)
        {
            switch(m_creature->GetEntry())
            {
                case SH_AELMAR:
                    pInstance->SetData(TYPE_SH_AELMAR, 0);
                    break;
                case SH_CATHELA:
                    pInstance->SetData(TYPE_SH_CATHELA, 0);
                    break;
                case SH_GREGOR:
                    pInstance->SetData(TYPE_SH_GREGOR, 0);
                    break;
                case SH_NEMAS:
                    pInstance->SetData(TYPE_SH_NEMAS, 0);
                    break;
                case SH_VICAR:
                    pInstance->SetData(TYPE_SH_VICAR, 0);
                    break;
            }
        }
    }

    void JustDied(Unit* Killer)
    {
        if(pInstance)
        {
            switch(m_creature->GetEntry())
            {
                case SH_AELMAR:
                    pInstance->SetData(TYPE_SH_AELMAR, 2);
                    break;
                case SH_CATHELA:
                    pInstance->SetData(TYPE_SH_CATHELA, 2);
                    break;
                case SH_GREGOR:
                    pInstance->SetData(TYPE_SH_GREGOR, 2);
                    break;
                case SH_NEMAS:
                    pInstance->SetData(TYPE_SH_NEMAS, 2);
                    break;
                case SH_VICAR:
                    pInstance->SetData(TYPE_SH_VICAR, 2);
                    break;
            }
            if(pInstance->GetData(TYPE_SH_QUEST) && Killer->GetTypeId() == TYPEID_PLAYER)
                ((Player*)Killer)->KilledMonster(SH_QUEST_CREDIT,m_creature->GetGUID());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameHolyLight_Timer.Expired(diff))
        {
            if (m_creature->GetHealth()*5 < m_creature->GetMaxHealth())
            {
                DoCast(m_creature, SPELL_HOLY_LIGHT);
                _ChangedNameHolyLight_Timer = 20000;
            }
        }

        if (_ChangedNameDivineShield_Timer.Expired(diff))
        {
            if (m_creature->GetHealth()*20 < m_creature->GetMaxHealth())
            {
                DoCast(m_creature, SPELL_DIVINE_SHIELD);
                _ChangedNameDivineShield_Timer = 40000;
            }
        }

        DoMeleeAttackIfReady();
    }

};
CreatureAI* GetAI_boss_silver_hand_bossesAI(Creature *_Creature)
{
    return new boss_silver_hand_bossesAI (_Creature);
}

void AddSC_boss_order_of_silver_hand()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_silver_hand_bosses";
    newscript->GetAI = &GetAI_boss_silver_hand_bossesAI;
    newscript->RegisterSelf();
}


