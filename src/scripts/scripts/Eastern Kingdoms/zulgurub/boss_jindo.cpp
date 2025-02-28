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
SDName: Boss_Jin'do the Hexxer
SD%Complete: 85
SDComment: Mind Control not working because of core bug. Shades visible for all.
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SAY_AGGRO                       -1309014

#define SPELL_BRAINWASHTOTEM            24262
#define SPELL_POWERFULLHEALINGWARD      24309
#define SPELL_HEX                       24053 // should drop aggro from target if spell succeded. - but regain after it ends
#define SPELL_DELUSIONSOFJINDO          24306

//Shade of Jindo Spell
#define SPELL_SHADOWSHOCK               19460
#define SPELL_INVISIBLE                 24307

struct boss_jindoAI : public ScriptedAI
{
    boss_jindoAI(Creature *c) : ScriptedAI(c), Summons(me)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    SummonList Summons;

    Timer _ChangedNameBrainWashTotem_Timer;
    Timer _ChangedNameHealingWard_Timer;
    Timer _ChangedNameHex_Timer;
    Timer _ChangedNameDelusions_Timer;
    Timer _ChangedNameTeleport_Timer;

    void Reset()
    {
        _ChangedNameBrainWashTotem_Timer.Reset(20000);
        _ChangedNameHealingWard_Timer.Reset(16000);
        _ChangedNameHex_Timer.Reset(8000);
        _ChangedNameDelusions_Timer.Reset(10000);
        _ChangedNameTeleport_Timer.Reset(5000);
        pInstance->SetData(DATA_JINDOEVENT, NOT_STARTED);
        Summons.DespawnAll();
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        pInstance->SetData(DATA_JINDOEVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        pInstance->SetData(DATA_JINDOEVENT, DONE);
    }

    void JustSummoned(Creature* summoned)
    {
        Summons.Summon(summoned);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameBrainWashTotem_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_BRAINWASHTOTEM);
            _ChangedNameBrainWashTotem_Timer = 18000 + rand()%8000;
        }


        if (_ChangedNameHealingWard_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_POWERFULLHEALINGWARD);
            _ChangedNameHealingWard_Timer = 14000 + rand()%6000;
        }
        

        if (_ChangedNameHex_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_HEX);

            if(DoGetThreat(m_creature->GetVictim()))
                DoModifyThreatPercent(m_creature->GetVictim(),-80);

            _ChangedNameHex_Timer = 12000 + rand()%8000;
        }
        

        //Casting the delusion curse with a shade. So shade will attack the same target with the curse.
        if(_ChangedNameDelusions_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,1, GetSpellMaxRange(SPELL_DELUSIONSOFJINDO), true))
            {
                DoCast(target, SPELL_DELUSIONSOFJINDO);

                if(Creature* Shade = m_creature->SummonCreature(14986, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                {
                    Shade->AI()->AttackStart(target);
                    Shade->AddThreat(target, 1000000); // i think they should not change targets - because they are invisible for others
                }
            }
            _ChangedNameDelusions_Timer = 4000 + rand()%8000;
        }
        
        //Teleporting a random gamer and spawning 9 skeletons that will attack this gamer
        if(_ChangedNameTeleport_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,1, 200, true))
            {
                DoTeleportPlayer(target, -11583.7783,-1249.4278,77.5471,4.745);

                if(DoGetThreat(m_creature->GetVictim()))
                    DoModifyThreatPercent(target,-100);

                Creature* Skeletons = NULL;

                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX() + 2, target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX()-2, target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX()+4, target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX()-4, target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX(), target->GetPositionY()+2, target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX(), target->GetPositionY()-2, target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX(), target->GetPositionY()+4, target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX(), target->GetPositionY()-4, target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
                Skeletons = m_creature->SummonCreature(14826, target->GetPositionX()+3, target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(Skeletons)
                    Skeletons->AI()->AttackStart(target);
            }

            _ChangedNameTeleport_Timer = 15000 + rand()%8000;
        }
        

        DoMeleeAttackIfReady();
    }
};


//Shade of Jindo
struct mob_shade_of_jindoAI : public ScriptedAI
{
    mob_shade_of_jindoAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    Timer _ChangedNameShadowShock_Timer;

    ScriptedInstance *pInstance;

    void Reset()
    {
        _ChangedNameShadowShock_Timer.Reset(1000);
        m_creature->CastSpell(m_creature, SPELL_INVISIBLE,true);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI (const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(_ChangedNameShadowShock_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_SHADOWSHOCK);
            _ChangedNameShadowShock_Timer = 2000;
        }
        

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_jindo(Creature *_Creature)
{
    return new boss_jindoAI (_Creature);
}

CreatureAI* GetAI_mob_shade_of_jindo(Creature *_Creature)
{
    return new mob_shade_of_jindoAI (_Creature);
}

void AddSC_boss_jindo()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_jindo";
    newscript->GetAI = &GetAI_boss_jindo;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shade_of_jindo";
    newscript->GetAI = &GetAI_mob_shade_of_jindo;
    newscript->RegisterSelf();
}


