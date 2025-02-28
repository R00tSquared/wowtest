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
SDName: Boss_Hungarfen
SD%Complete: 95
SDComment: Need confirmation if spell data are same in both modes. Summons should have faster rate in heroic
SDCategory: Coilfang Resevoir, Underbog
EndScriptData */

#include "precompiled.h"
#include "def_underbog.h"

enum
{
    // Hungarfen
    SPELL_FOUL_SPORES       = 31673,
    SPELL_ACID_GEYSER       = 38739,
    // Mushrooms
    SPELL_SPORE_CLOUD       = 34168,
    SPELL_PUTRID_MUSHROOM   = 31690,
    SPELL_GROW              = 31698,

    NPC_UNDERBOG_MUSHROOM   = 17990
};

struct boss_hungarfenAI : public ScriptedAI
{
    boss_hungarfenAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }

    ScriptedInstance* pInstance;

    bool HeroicMode;
    bool Root;
    Timer Mushroom_Timer;
    Timer AcidGeyser_Timer;
    SummonList summons;

    void Reset()
    {
        Root = false;
        Mushroom_Timer.Reset(5000);     // 1 mushroom after 5s, then one per 10s. This should be different in heroic mode
        AcidGeyser_Timer.Reset(10000);
        summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(TYPE_HUNGARFEN, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoZoneInCombat(80.0f);
        if(pInstance)
            pInstance->SetData(TYPE_HUNGARFEN, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(TYPE_HUNGARFEN, DONE);
    }

    void JustSummoned(Creature* summ)
    {
        if (summ)
        {
            summ->SetOwnerGUID(me->GetGUID());
            summons.Summon(summ);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(me->GetHealthPercent() <= 20)
        {
            if(!Root)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);
                DoCast(me, SPELL_FOUL_SPORES);
                Root = true;
            }
        }

        if (Mushroom_Timer.Expired(diff))
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0))
                me->SummonCreature(NPC_UNDERBOG_MUSHROOM, target->GetPositionX()+(rand()%8), target->GetPositionY()+(rand()%8), target->GetPositionZ(), (rand()%5), TEMPSUMMON_CORPSE_DESPAWN, 0);
            else
                me->SummonCreature(NPC_UNDERBOG_MUSHROOM, me->GetPositionX()+(rand()%8), me->GetPositionY()+(rand()%8), me->GetPositionZ(), (rand()%5), TEMPSUMMON_CORPSE_DESPAWN, 0);

            Mushroom_Timer = 10000;
        }

        if(HeroicMode)
        {
            if (AcidGeyser_Timer.Expired(diff))
            {
                if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0))
                    DoCast(target,SPELL_ACID_GEYSER);
                AcidGeyser_Timer = 10000+rand()%7500;
            }
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_hungarfen(Creature *_Creature)
{
    return new boss_hungarfenAI (_Creature);
}

struct mob_underbog_mushroomAI : public ScriptedAI
{
    mob_underbog_mushroomAI(Creature *c) : ScriptedAI(c) {}

    Timer Grow_Timer;
    Timer Spore_Timer;
    Timer Shrink_Timer;
    Timer Despawn_Timer;

    void Reset()
    {
        Grow_Timer.Reset(1000);
        Spore_Timer.Reset(15000);
        Shrink_Timer.Reset(20000);
        Despawn_Timer.Reset(0);

        DoCast(me, SPELL_PUTRID_MUSHROOM, true);
    }

    void MoveInLineOfSight(Unit *who)
    {
        return;
    }

    void AttackStart(Unit* who)
    {
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (Spore_Timer.Expired(diff))
        {
            DoCast(me, SPELL_SPORE_CLOUD);
            Grow_Timer = 0;
            Spore_Timer = 0;
        }

        if (Grow_Timer.Expired(diff))
        {
            DoCast(me, SPELL_GROW);
            Grow_Timer = 3000;
        }

        if (Shrink_Timer.Expired(diff))
        {
            me->RemoveAurasDueToSpell(SPELL_GROW);
            Despawn_Timer = 1650;
        }
        
        if (Despawn_Timer.Expired(diff))
        {
            me->SetUInt32Value(UNIT_FIELD_DISPLAYID, 11686);
            Despawn_Timer = 0;
        }
    }
};
CreatureAI* GetAI_mob_underbog_mushroom(Creature *_Creature)
{
    return new mob_underbog_mushroomAI (_Creature);
}

void AddSC_boss_hungarfen()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_hungarfen";
    newscript->GetAI = &GetAI_boss_hungarfen;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_underbog_mushroom";
    newscript->GetAI = &GetAI_mob_underbog_mushroom;
    newscript->RegisterSelf();
}

