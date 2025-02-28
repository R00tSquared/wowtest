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
SDName: Boss_Nerubenkan
SD%Complete: 70
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "def_stratholme.h"

#define SPELL_ENCASINGWEBS          4962
#define SPELL_PIERCEARMOR           6016
#define SPELL_CRYPT_SCARABS         31602
#define SPELL_RAISEUNDEADSCARAB     17235

struct boss_nerubenkanAI : public ScriptedAI
{
    boss_nerubenkanAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)m_creature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameEncasingWebs_Timer;
    Timer _ChangedNamePierceArmor_Timer;
    Timer _ChangedNameCryptScarabs_Timer;
    Timer _ChangedNameRaiseUndeadScarab_Timer;
    int Rand;
    int RandX;
    int RandY;

    void Reset()
    {
       _ChangedNameCryptScarabs_Timer.Reset(3000);
        _ChangedNameEncasingWebs_Timer.Reset(7000);
        _ChangedNamePierceArmor_Timer.Reset(19000);
        _ChangedNameRaiseUndeadScarab_Timer.Reset(3000);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(TYPE_NERUB,IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance)
            pInstance->SetData(TYPE_NERUB,SPECIAL);
    }

    void RaiseUndeadScarab(Unit* victim)
    {
        Rand = rand()%10;
        switch (rand()%2)
        {
        case 0: RandX = 0 - Rand; break;
        case 1: RandX = 0 + Rand; break;
        }
        Rand = 0;
        Rand = rand()%10;
        switch (rand()%2)
        {
        case 0: RandY = 0 - Rand; break;
        case 1: RandY = 0 + Rand; break;
        }
        Rand = 0;

        if(Creature* Summoned = DoSpawnCreature(10876, RandX, RandY, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 180000))
            ((CreatureAI*)Summoned->AI())->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameEncasingWebs_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_ENCASINGWEBS);
            _ChangedNameEncasingWebs_Timer = 30000;
        }

        if (_ChangedNamePierceArmor_Timer.Expired(diff))
        {
            if (rand()%100 < 75)
                DoCast(m_creature->GetVictim(),SPELL_PIERCEARMOR);
            _ChangedNamePierceArmor_Timer = 35000;
        }

        if (_ChangedNameCryptScarabs_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_CRYPT_SCARABS);
            _ChangedNameCryptScarabs_Timer = 20000;
        }

        if (_ChangedNameRaiseUndeadScarab_Timer.Expired(diff))
        {
            RaiseUndeadScarab(m_creature->GetVictim());
            _ChangedNameRaiseUndeadScarab_Timer = 16000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_nerubenkan(Creature *_Creature)
{
    return new boss_nerubenkanAI (_Creature);
}

void AddSC_boss_nerubenkan()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_nerubenkan";
    newscript->GetAI = &GetAI_boss_nerubenkan;
    newscript->RegisterSelf();
}


