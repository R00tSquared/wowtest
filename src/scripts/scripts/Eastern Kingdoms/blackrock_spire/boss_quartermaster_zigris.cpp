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
SDName: Boss_Quartmaster_Zigris
SD%Complete: 100
SDComment: Needs revision
SDCategory: Blackrock Spire
EndScriptData */

#include "precompiled.h"

#define SPELL_SHOOT             16496
#define SPELL_STUNBOMB          16497
#define SPELL_HEALING_POTION    15504
#define SPELL_HOOKEDNET         15609

struct boss_quatermasterzigrisAI : public ScriptedAI
{
    boss_quatermasterzigrisAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameShoot_Timer;
    Timer _ChangedNameStunBomb_Timer;
    //uint32 HelingPotion_Timer;

    void Reset()
    {
        _ChangedNameShoot_Timer.Reset(1000);
        _ChangedNameStunBomb_Timer.Reset(16000);
        //HelingPotion_Timer = 25000;
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        if (_ChangedNameShoot_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHOOT);
            _ChangedNameShoot_Timer = 500;
        }

        if (_ChangedNameStunBomb_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_STUNBOMB);
            _ChangedNameStunBomb_Timer = 14000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_quatermasterzigris(Creature *_Creature)
{
    return new boss_quatermasterzigrisAI (_Creature);
}

void AddSC_boss_quatermasterzigris()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="quartermaster_zigris";
    newscript->GetAI = &GetAI_boss_quatermasterzigris;
    newscript->RegisterSelf();
}


