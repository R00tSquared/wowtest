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
SDName: Boss_The_Maker
SD%Complete: 85
SDComment:
SDCategory: Hellfire Citadel, Blood Furnace
EndScriptData */

#include "precompiled.h"
#include "def_blood_furnace.h"

#define SAY_AGGRO_1                 -1542009
#define SAY_AGGRO_2                 -1542010
#define SAY_AGGRO_3                 -1542011
#define SAY_KILL_1                  -1542012
#define SAY_KILL_2                  -1542013
#define SAY_DIE                     -1542014

#define SPELL_ACID_SPRAY            38153
#define SPELL_EXPLODING_BREAKER     (HeroicMode ? 40059 : 30925)
#define SPELL_KNOCKDOWN             20276
#define SPELL_DOMINATION            30923

struct boss_the_makerAI : public ScriptedAI
{
    boss_the_makerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = c->GetInstanceData();
    }

    Timer_UnCheked AcidSpray_Timer;
    Timer_UnCheked ExplodingBreaker_Timer;
    Timer_UnCheked Domination_Timer;
    Timer_UnCheked Knockdown_Timer;

    ScriptedInstance *pInstance;

    void Reset()
    {
        AcidSpray_Timer.Reset(15000);
        ExplodingBreaker_Timer.Reset(6000);
        Domination_Timer.Reset(20000);
        Knockdown_Timer.Reset(10000);

        if (pInstance)
            pInstance->SetData(DATA_MAKEREVENT, NOT_STARTED);
    }

    void RemoveEncounterDebuffs()
    {
        Map *map = me->GetMap();
        if (!map->IsDungeon())
            return;

        Map::PlayerList const &PlayerList = map->GetPlayers();
        Map::PlayerList::const_iterator i;
        for(i = PlayerList.begin(); i != PlayerList.end(); ++i)
            if (Player* i_pl = i->getSource())
                i_pl->RemoveAurasDueToSpell(SPELL_DOMINATION);
    }

    void JustReachedHome()
    {
        RemoveEncounterDebuffs();
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), m_creature);

        if (pInstance)
            pInstance->SetData(DATA_MAKEREVENT, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DIE, m_creature);
        RemoveEncounterDebuffs();
        if (pInstance)
            pInstance->SetData(DATA_MAKEREVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (AcidSpray_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_ACID_SPRAY);
            AcidSpray_Timer = 35000+rand()%8000; // not the correct spell. why spam ?
        }

        if (ExplodingBreaker_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                AddSpellToCast(target,SPELL_EXPLODING_BREAKER);

            ExplodingBreaker_Timer = urand(4000, 12000);
        }

        if (Domination_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                AddSpellToCast(target, SPELL_DOMINATION);

            Domination_Timer = 120000;
        }

        if (Knockdown_Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(),SPELL_KNOCKDOWN);
            Knockdown_Timer = urand(4000, 12000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_the_makerAI(Creature *_Creature)
{
    return new boss_the_makerAI (_Creature);
}

void AddSC_boss_the_maker()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_the_maker";
    newscript->GetAI = &GetAI_boss_the_makerAI;
    newscript->RegisterSelf();
}

