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
SDName: Boss_Kormok
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

#include "precompiled.h"

#define SPELL_SHADOWBOLTVOLLEY      20741
#define SPELL_BONESHIELD            27688

struct boss_kormokAI : public ScriptedAI
{
    boss_kormokAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameShadowVolley_Timer;
    Timer _ChangedNameBoneShield_Timer;
    Timer _ChangedNameMinion_Timer;
    bool Mages;
    int Rand1;
    int Rand1X;
    int Rand1Y;
    int Rand2;
    int Rand2X;
    int Rand2Y;

    void Reset()
    {
        _ChangedNameShadowVolley_Timer.Reset(10000);
        _ChangedNameBoneShield_Timer.Reset(2000);
        _ChangedNameMinion_Timer.Reset(15000);
        Mages = false;
    }

    void EnterCombat(Unit *who)
    {
    }

    void SummonMinion(Unit* victim)
    {
        Rand1 = rand()%8;
        switch (rand()%2)
        {
            case 0: Rand1X = 0 - Rand1; break;
            case 1: Rand1X = 0 + Rand1; break;
        }
        Rand1 = 0;
        Rand1 = rand()%8;
        switch (rand()%2)
        {
            case 0: Rand1Y = 0 - Rand1; break;
            case 1: Rand1Y = 0 + Rand1; break;
        }
        Rand1 = 0;

        if(Creature* SummonedMinions  = DoSpawnCreature(16119, Rand1X, Rand1Y, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
            ((CreatureAI*)SummonedMinions->AI())->AttackStart(victim);
    }

    void SummonMages(Unit* victim)
    {
        Rand2 = rand()%10;
        switch (rand()%2)
        {
            case 0: Rand2X = 0 - Rand2; break;
            case 1: Rand2X = 0 + Rand2; break;
        }
        Rand2 = 0;
        Rand2 = rand()%10;
        switch (rand()%2)
        {
            case 0: Rand2Y = 0 - Rand2; break;
            case 1: Rand2Y = 0 + Rand2; break;
        }
        Rand2 = 0;

        if(Creature* SummonedMages = DoSpawnCreature(16120, Rand2X, Rand2Y, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 120000))
            ((CreatureAI*)SummonedMages->AI())->AttackStart(victim);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameShadowVolley_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWBOLTVOLLEY);
            _ChangedNameShadowVolley_Timer = 15000;
        }

        if (_ChangedNameBoneShield_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_BONESHIELD);
            _ChangedNameBoneShield_Timer = 45000;
        }

        if (_ChangedNameMinion_Timer.Expired(diff))
        {
            //Cast
            SummonMinion(m_creature->GetVictim());
            SummonMinion(m_creature->GetVictim());
            SummonMinion(m_creature->GetVictim());
            SummonMinion(m_creature->GetVictim());

            _ChangedNameMinion_Timer = 12000;
        }

        //Summon 2 Bone Mages
        if ( !Mages && me->GetHealthPercent() < 26 )
        {
            //Cast
            SummonMages(m_creature->GetVictim());
            SummonMages(m_creature->GetVictim());
            Mages = true;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_kormok(Creature *_Creature)
{
    return new boss_kormokAI (_Creature);
}

void AddSC_boss_kormok()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_kormok";
    newscript->GetAI = &GetAI_boss_kormok;
    newscript->RegisterSelf();
}


