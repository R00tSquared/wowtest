// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
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

#include "precompiled.h"
#include "def_slave_pens.h"

enum MennuTheBetrayer
{
    SAY_AGGRO_1                 = -1611000,
    SAY_AGGRO_2                 = -1611001,
    SAY_AGGRO_3                 = -1611002,
    SAY_KILL_1                  = -1611003,
    SAY_KILL_2                  = -1611004,
    SAY_DEATH_1                 = -1611005,

    SPELL_LIGHTNING_BOLT        = 35010,
    SPELL_CORRUPTED_NOVA_TOTEM  = 31991,
    SPELL_EARTHGRAB_TOTEM       = 31981,
    SPELL_STONESKIN_TOTEM       = 31985,
    SPELL_HEALING_TOTEM         = 34980,

    SPELL_FIRE_NOVA             = 33132,
    SPELL_FIRE_NOVA_H           = 43464,
    SPELL_ENTANGLING_ROOTS      = 20654,
    SPELL_STONESKIN             = 31986,
    SPELL_HEAL_N                = 34978,
    SPELL_HEAL_H                = 38799,

    NPC_STONESKIN_TOTEM         = 18177,
    NPC_H_STONESKIN_TOTEM       = 19900,
    NPC_HEALING_TOTEM           = 20208
};


struct boss_mennu_the_betrayerAI : public ScriptedAI
{
    boss_mennu_the_betrayerAI(Creature *c) : ScriptedAI(c), Summons(m_creature)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }


    ScriptedInstance *pInstance;
    bool HeroicMode;

    SummonList Summons;

    std::vector<uint32> remainingTotems;
    Timer Totem_Timer;
    Timer LightningBolt_Timer;

    void Reset()
    {
        ClearCastQueue();
        LightningBolt_Timer.Reset(10000);
        Totem_Timer.Reset(2000);
        GenerateRandomTotemOrder();
        Summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(DATA_MENNU, NOT_STARTED);
    }

    void GenerateRandomTotemOrder()
    {
        remainingTotems.clear();
        std::vector<uint32> temp;
        temp.push_back(SPELL_CORRUPTED_NOVA_TOTEM);
        temp.push_back(SPELL_STONESKIN_TOTEM);
        temp.push_back(SPELL_EARTHGRAB_TOTEM);
        temp.push_back(SPELL_HEALING_TOTEM);
        for (uint32 i = 0; i < 4; ++i)
        {
            uint32 index = urand(0, temp.size() - 1);
            remainingTotems.push_back(temp[index]);
            temp.erase(temp.begin() + index);
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), me);
        if(pInstance)
            pInstance->SetData(DATA_MENNU, IN_PROGRESS);
    }

    void JustSummoned(Creature* summoned)
    {
        switch(summoned->GetEntry())
        {
            case NPC_STONESKIN_TOTEM:
            case NPC_H_STONESKIN_TOTEM:
                summoned->CastSpell(summoned, SPELL_STONESKIN, true);
                summoned->SetDefaultMovementType(IDLE_MOTION_TYPE);
                break;
            case NPC_HEALING_TOTEM:
                summoned->setFaction(me->getFaction());
                summoned->AddAura(HeroicMode ? SPELL_HEAL_H : SPELL_HEAL_N, summoned);
                summoned->SetDefaultMovementType(IDLE_MOTION_TYPE);
                break;
        }
        Summons.Summon(summoned);
    }

    void SummonedCreatureDespawn(Creature *summon)
    {
        Summons.Despawn(summon);
    }

    void KilledUnit(Unit * victim)
    {
        DoScriptText(RAND(SAY_KILL_1, SAY_KILL_2), me, victim);
    }

    void JustDied(Unit *u)
    {
        DoScriptText(SAY_DEATH_1, me);
        Summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(DATA_MENNU, DONE);
    }


    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (LightningBolt_Timer.Expired(diff))
        {
            if (HeroicMode)
                AddCustomSpellToCast(m_creature->GetVictim(), SPELL_LIGHTNING_BOLT,142,0,0);
            else
            AddCustomSpellToCast(m_creature->GetVictim(), SPELL_LIGHTNING_BOLT,175,0,0);
            LightningBolt_Timer = urand(10000, 15000);
        }

        if (Totem_Timer.Expired(diff))
        {
            if(remainingTotems.empty())
            {
                GenerateRandomTotemOrder();
                Totem_Timer = 20000;
            }
            else
            {
                AddSpellToCast(me, remainingTotems[0]);
                Totem_Timer = 2000;
                remainingTotems.erase(remainingTotems.begin());
            }
        }

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_mennu_the_betrayer(Creature *_Creature)
{
    return new boss_mennu_the_betrayerAI (_Creature);
}

struct npc_corrupted_nova_totemAI : public Scripted_NoMovementAI
{
    npc_corrupted_nova_totemAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = (c->GetInstanceData());
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }

    ScriptedInstance *pInstance;
    bool HeroicMode;
    Timer Life_Timer;
    Timer Destroy_Timer;
    bool TimerLifeDone;

    void Reset()
    {
        Life_Timer.Reset(HeroicMode ? 15000 : 5000);
        Destroy_Timer.Reset(0);
        TimerLifeDone = false;
    }

    void DamageTaken(Unit*, uint32& damage)
    {
        if(!HeroicMode)
            damage = 0;
        else if(!TimerLifeDone && damage >= me->GetHealth() && HeroicMode) // Can be destroyed with next 6k dmg only in HC
        {
            damage = me->GetHealth() - 1;
            DoCast(me, SPELL_FIRE_NOVA_H);
            TimerLifeDone = true;
            Life_Timer = 0;
            Destroy_Timer = 300;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Life_Timer.Expired(diff))
        {
            if(!TimerLifeDone)
            {
                if(!HeroicMode)
                    DoCast(me, SPELL_FIRE_NOVA);
                else
                    DoCast(me, SPELL_FIRE_NOVA_H);
    
                TimerLifeDone = true;
                Life_Timer = 0;
                Destroy_Timer = 300;
            }
        }

        if (Destroy_Timer.Expired(diff))
        {
            me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE);
            Destroy_Timer = 0;
        }
    }

};

CreatureAI* GetAI_npc_corrupted_nova_totem(Creature *_Creature)
{
    return new npc_corrupted_nova_totemAI (_Creature);
}

struct npc_earthgrab_totemAI : public Scripted_NoMovementAI
{
    npc_earthgrab_totemAI(Creature *c) : Scripted_NoMovementAI(c)
    {
    }

    Timer Earthgrab_Timer;

    void Reset()
    {
        Earthgrab_Timer.Reset(4000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Earthgrab_Timer.Expired(diff))
        {
            DoCast(me, SPELL_ENTANGLING_ROOTS);
            Earthgrab_Timer = urand(18000, 22000);
        }
    }
};

CreatureAI* GetAI_npc_earthgrab_totem(Creature *_Creature)
{
    return new npc_earthgrab_totemAI (_Creature);
}

void AddSC_boss_mennu_the_betrayer()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_mennu_the_betrayer";
    newscript->GetAI = &GetAI_boss_mennu_the_betrayer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_corrupted_nova_totem";
    newscript->GetAI = &GetAI_npc_corrupted_nova_totem;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_earthgrab_totem";
    newscript->GetAI = &GetAI_npc_earthgrab_totem;
    newscript->RegisterSelf();
}
