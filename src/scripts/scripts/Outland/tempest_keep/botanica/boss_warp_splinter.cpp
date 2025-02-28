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
SDName: Boss_Warp_Splinter
SD%Complete: 80
SDComment: Includes Sapling (need some better control with these).
SDCategory: Tempest Keep, The Botanica
EndScriptData */

#include "precompiled.h"
#include "def_botanica.h"

/*#####
# mob_treant (Sapling)
#####*/

#define SPELL_HEAL_FATHER   6262

struct mob_treantAI  : public ScriptedAI
{
    mob_treantAI (Creature *c) : ScriptedAI(c), WarpGuid(0) {}

    uint64 WarpGuid;
    Timer AncestralLife_Timer;

    void Reset()
    {
        AncestralLife_Timer.Reset(25000);
        me->SetSpeed(MOVE_RUN, 0.3f, true);
    }

    void EnterCombat(Unit *who) {}

	void MoveInLineOfSight(Unit *who)
	{
		if (me->canStartAttack(who))
		{
			who->CombatStart(me);
		}
	}

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE && id == 1)
            me->GetMotionMaster()->MoveIdle();
    }

    void UpdateAI(const uint32 diff)
    {
		if (WarpGuid && AncestralLife_Timer.Expired(diff))
		{
			AncestralLife_Timer = 0;

			if (Unit *Warp = me->GetUnit(WarpGuid))
			{
				me->CastSpell(Warp, 31270, true);
				me->ForcedDespawn();
				return;
			}
		}

		if (!UpdateVictim())
			return;

        if (me->getVictimGUID() != WarpGuid)
            DoMeleeAttackIfReady();
    }
};

/*#####
# boss_warp_splinter
#####*/

enum WarpSplinter
{
    SAY_AGGRO               = -1553007,
    SAY_SLAY_1              = -1553008,
    SAY_SLAY_2              = -1553009,
    SAY_SUMMON_1            = -1553010,
    SAY_SUMMON_2            = -1553011,
    SAY_DEATH               = -1553012,
    
    WAR_STOMP               = 34716,
    ARCANE_VOLLEY           = 36705,
    ARCANE_VOLLEY_H         = 39133,
    SPELL_SUMMON            = 34741,
    SPELL_SAPLING_1         = 34727,
    SPELL_SAPLING_2         = 34730,
    SPELL_SAPLING_3         = 34731,
    SPELL_SAPLING_4         = 34733,
    SPELL_SAPLING_5         = 34732,
    SPELL_SAPLING_6         = 34734
};

struct boss_warp_splinterAI : public ScriptedAI
{
    boss_warp_splinterAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = c->GetMap()->IsHeroic();
        me->GetPosition(wLoc);
    }

    ScriptedInstance *pInstance;

    SummonList summons;
    Timer WarStomp_Timer;
    Timer ArcaneVolley_Timer;
    Timer SummonTreants_Timer;
    Timer Check_Timer;

    WorldLocation wLoc;
    bool HeroicMode;

    void Reset()
    {
        summons.DespawnAll();
        WarStomp_Timer.Reset(urand(5000, 10000));
        ArcaneVolley_Timer.Reset(17000);
        SummonTreants_Timer.Reset(25000);
        Check_Timer.Reset(3000);
        me->SetSpeed(MOVE_RUN, 0.7f, true);
        if(pInstance)
            pInstance->SetData(TYPE_WARP_SPLINTER, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, me);
        if(pInstance)
            pInstance->SetData(TYPE_WARP_SPLINTER, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
    }

    void JustSummoned(Creature* treant)
    {
        if(treant)
        {
            treant->GetMotionMaster()->MovePoint(1, me->GetPositionX()+rand()%3, me->GetPositionY()+rand()%3, me->GetPositionZ());
            treant->setFaction(me->getFaction());
            ((mob_treantAI*)treant->AI())->WarpGuid = me->GetGUID();
        }
        summons.Summon(treant);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, me);
        summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(TYPE_WARP_SPLINTER, IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(Check_Timer.Expired(diff))
        {
            if(!me->IsWithinDistInMap(&wLoc, 35.0f))
                EnterEvadeMode();

            Check_Timer = 1000;
        }

        if(WarStomp_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),WAR_STOMP);
            WarStomp_Timer = urand(20000, 40000);
        }

        if(ArcaneVolley_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),ARCANE_VOLLEY);
            ArcaneVolley_Timer = urand(20000, 25000);
        }

        if(SummonTreants_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SUMMON);
            AddSpellToCast(me, SPELL_SAPLING_1, true);
            AddSpellToCast(me, SPELL_SAPLING_2, true);
            AddSpellToCast(me, SPELL_SAPLING_3, true);
            AddSpellToCast(me, SPELL_SAPLING_4, true);
            AddSpellToCast(me, SPELL_SAPLING_5, true);
            AddSpellToCast(me, SPELL_SAPLING_6, true);
            SummonTreants_Timer = 45000;
            DoScriptText(RAND(SAY_SUMMON_1, SAY_SUMMON_2), me);
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_warp_splinter(Creature *_Creature)
{
    return new boss_warp_splinterAI (_Creature);
}

CreatureAI* GetAI_mob_treant(Creature *_Creature)
{
    return new mob_treantAI (_Creature);
}

void AddSC_boss_warp_splinter()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_warp_splinter";
    newscript->GetAI = &GetAI_boss_warp_splinter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_warp_splinter_treant";
    newscript->GetAI = &GetAI_mob_treant;
    newscript->RegisterSelf();
}

