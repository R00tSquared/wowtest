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
SDName: Boss_Broodlord_Lashlayer
SD%Complete: 100
SDComment:
SDCategory: Blackwing Lair
EndScriptData */

#include "precompiled.h"
#include "def_blackwing_lair.h"

#define SAY_AGGRO               -1469000
#define SAY_LEASH               -1469001

#define SPELL_CLEAVE            26350
#define SPELL_BLASTWAVE         23331
#define SPELL_MORTALSTRIKE      24573
#define SPELL_KNOCKBACK         25778

struct boss_broodlordAI : public ScriptedAI
{
    boss_broodlordAI(Creature *c) : ScriptedAI(c)
    {
        c->GetPosition(wLoc);
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;
    Timer _ChangedNameCleave_Timer;
    Timer _ChangedNameBlastWave_Timer;
    Timer _ChangedNameMortalStrike_Timer;
    Timer _ChangedNameKnockBack_Timer;
    Timer _ChangedNameLeashCheck_Timer;
    WorldLocation wLoc;

    void Reset()
    {
        _ChangedNameCleave_Timer.Reset(8000);                                //These times are probably wrong
        _ChangedNameBlastWave_Timer.Reset(12000);
        _ChangedNameMortalStrike_Timer.Reset(20000);
        _ChangedNameKnockBack_Timer.Reset(30000);
        _ChangedNameLeashCheck_Timer.Reset(2000);

        if (pInstance && pInstance->GetData(DATA_BROODLORD_LASHLAYER_EVENT) != DONE)
            pInstance->SetData(DATA_BROODLORD_LASHLAYER_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        DoZoneInCombat();

        if (pInstance)
            pInstance->SetData(DATA_BROODLORD_LASHLAYER_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_BROODLORD_LASHLAYER_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameLeashCheck_Timer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 250))
            {
                DoScriptText(SAY_LEASH, m_creature);
                EnterEvadeMode();
                return;
            }
            _ChangedNameLeashCheck_Timer = 2000;
        }

        if (_ChangedNameCleave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CLEAVE);
            _ChangedNameCleave_Timer = 7000;
        }

        if (_ChangedNameBlastWave_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_BLASTWAVE);
            _ChangedNameBlastWave_Timer = 8000 + rand()%8000;
        }

        if (_ChangedNameMortalStrike_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_MORTALSTRIKE);
            _ChangedNameMortalStrike_Timer = 25000 + rand()%10000;
        }

        if (_ChangedNameKnockBack_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_KNOCKBACK);
            //Drop 50% aggro
            if (DoGetThreat(m_creature->GetVictim()))
                DoModifyThreatPercent(m_creature->GetVictim(),-50);

            _ChangedNameKnockBack_Timer = 15000 + rand()%15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_broodlord(Creature *_Creature)
{
    return new boss_broodlordAI (_Creature);
}

void AddSC_boss_broodlord()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_broodlord";
    newscript->GetAI = &GetAI_boss_broodlord;
    newscript->RegisterSelf();
}


