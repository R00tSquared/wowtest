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
SDName: Boss_Bloodmage_Thalnos
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"

#define SPELL_FROSTNOVA2                865
#define SPELL_FLAMESHOCK3               8053
#define SPELL_SHADOWBOLT5               1106
#define SPELL_FLAMESPIKE                8814
#define SPELL_FIRENOVA                  16079

#define SAY_AGGRO                       -1200273
#define SAY_HEALTH                      -1200274
//#define SAY_DEATH                       "More... More souls!"

#define SOUND_AGGRO                     5844
#define SOUND_HEALTH                    5846
#define SOUND_DEATH                     5845

struct boss_bloodmage_thalnosAI : public ScriptedAI
{
    boss_bloodmage_thalnosAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameFrostNova2_Timer;
    Timer _ChangedNameFlameShock3_Timer;
    Timer _ChangedNameShadowBolt5_Timer;
    Timer _ChangedNameFlameSpike_Timer;
    Timer _ChangedNameFireNova_Timer;
    Timer _ChangedNameYell_Timer;

    void Reset()
    {
        _ChangedNameYell_Timer.Reset(1);
        _ChangedNameFrostNova2_Timer.Reset(10000);
        _ChangedNameFlameShock3_Timer.Reset(15000);
        _ChangedNameShadowBolt5_Timer.Reset(20000);
        _ChangedNameFlameSpike_Timer.Reset(20000);
        _ChangedNameFireNova_Timer.Reset(10000);
    }

    void EnterCombat(Unit *who)
    {
        DoYell(-1200273,LANG_UNIVERSAL,NULL);
        DoPlaySoundToSet(m_creature,SOUND_AGGRO);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        //If we are <35% hp
        if ( me->GetHealthPercent() <= 35)
        {

            if (_ChangedNameYell_Timer.Expired(diff))
            {
                DoYell(-1200274,LANG_UNIVERSAL,NULL);
                DoPlaySoundToSet(m_creature,SOUND_HEALTH);
                _ChangedNameYell_Timer = 900000;
            }
        }

        if (_ChangedNameFrostNova2_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTNOVA2);
            _ChangedNameFrostNova2_Timer = 10000;
        }

        if (_ChangedNameFlameShock3_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FLAMESHOCK3);
            _ChangedNameFlameShock3_Timer = 15000;
        }

        if (_ChangedNameShadowBolt5_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_SHADOWBOLT5);
            _ChangedNameShadowBolt5_Timer = 20000;
        }

        if (_ChangedNameFlameSpike_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FLAMESPIKE);
            _ChangedNameFlameSpike_Timer = 30000;
        }

        if (_ChangedNameFireNova_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FIRENOVA);
            _ChangedNameFireNova_Timer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_bloodmage_thalnos(Creature *_Creature)
{
    return new boss_bloodmage_thalnosAI (_Creature);
}

void AddSC_boss_bloodmage_thalnos()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_bloodmage_thalnos";
    newscript->GetAI = &GetAI_boss_bloodmage_thalnos;
    newscript->RegisterSelf();
}


