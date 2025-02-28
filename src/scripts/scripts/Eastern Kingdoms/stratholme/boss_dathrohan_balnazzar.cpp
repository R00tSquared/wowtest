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
SDName: boss_darhrohan_balnazzar
SD%Complete: 100
SDComment: CHECK SQL
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"

//Dathrohan spells
#define SPELL_CRUSADERSHAMMER    17286 //AOE stun
#define SPELL_CRUSADERSTRIKE    17281
#define SPELL_MINDBLAST    17287
#define SPELL_HOLYSTRIKE    17284 //weapon dmg +3
#define SPELL_DAZED    1604

//Transform
#define SPELL_BALNAZZARTRANSFORM    17288 //restore full HP/mana, trigger spell Balnazzar Transform Stun

//Balnazzar spells
#define SPELL_SHADOWSHOCK    20603 //AOE 740-860dmg
#define SPELL_PSYCHICSCREAM    15398 //One target, might want to make a code selecting random target
#define SPELL_DEEPSLEEP    24777 //AOE, ten sec
#define SPELL_SHADOWBOLTVOLLEY    20741 //AOE, 255-345dmg
//#define SPELL_MINDCONTROL    15690 //core support needed

//Summon
//G1 front, left
#define ADD_1X 3444.156250
#define ADD_1Y -3090.626709
#define ADD_1Z 135.002319
#define ADD_1O 2.240888
//G1 front, right
#define ADD_2X 3449.123535
#define ADD_2Y -3087.009766
#define ADD_2Z 135.002319
#define ADD_2O 2.240888
//G1 back left
#define ADD_3X 3446.246826
#define ADD_3Y -3093.466309
#define ADD_3Z 135.002319
#define ADD_3O 2.240888
//G1 back, right
#define ADD_4X 3451.160889
#define ADD_4Y -3089.904785
#define ADD_4Z 135.002136
#define ADD_4O 2.240888
//G2 front, left
#define ADD_5X 3457.995117
#define ADD_5Y -3080.916504
#define ADD_5Z 135.002319
#define ADD_5O 3.784981
//G2 front, right
#define ADD_6X 3454.302490
#define ADD_6Y -3076.330566
#define ADD_6Z 135.002319
#define ADD_6O 3.784981
//G2 back left
#define ADD_7X 3460.975098
#define ADD_7Y -3078.901367
#define ADD_7Z 135.002319
#define ADD_7O 3.784981
//G2 back, right
#define ADD_8X 3457.338867
#define ADD_8Y -3073.979004
#define ADD_8Z 135.002319
#define ADD_8O 3.784981

struct boss_dathrohan_balnazzarAI : public ScriptedAI
{
    boss_dathrohan_balnazzarAI(Creature *c) : ScriptedAI(c) {}

    Timer _ChangedNameCrusadersHammer_Timer;
    Timer _ChangedNameCrusaderStrike_Timer;
    Timer _ChangedNameMindBlast_Timer;
    Timer _ChangedNameHolyStrike_Timer;
    Timer _ChangedNameDazed_Timer;
    Timer _ChangedNameShadowShock_Timer;
    Timer _ChangedNamePsychicScream_Timer;
    Timer _ChangedNameDeepSleep_Timer;
    Timer _ChangedNameShadowBoltVolley_Timer;
    //    Timer _ChangedNameMindControl_Timer;
    bool Transformed;

    void Reset()
    {
        _ChangedNameCrusadersHammer_Timer.Reset(8000);
        _ChangedNameCrusaderStrike_Timer.Reset(14000);
        _ChangedNameMindBlast_Timer.Reset(17000);
        _ChangedNameHolyStrike_Timer.Reset(18000);
        _ChangedNameDazed_Timer.Reset(23000);
        _ChangedNameShadowShock_Timer.Reset(4000);
        _ChangedNamePsychicScream_Timer.Reset(16000);
        _ChangedNameDeepSleep_Timer.Reset(20000);
        _ChangedNameShadowBoltVolley_Timer.Reset(9000);
        //        _ChangedNameMindControl_Timer.Reset(10000);
        Transformed = false;

        m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,10545);
        m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.00f);

    }

    void JustDied(Unit* Victim)
    {
        m_creature->SummonCreature(10698,ADD_1X,ADD_1Y,ADD_1Z,ADD_1O,TEMPSUMMON_TIMED_DESPAWN,240000);
        m_creature->SummonCreature(10698,ADD_2X,ADD_2Y,ADD_2Z,ADD_2O,TEMPSUMMON_TIMED_DESPAWN,240000);
        m_creature->SummonCreature(10698,ADD_3X,ADD_3Y,ADD_3Z,ADD_3O,TEMPSUMMON_TIMED_DESPAWN,240000);
        m_creature->SummonCreature(10698,ADD_4X,ADD_4Y,ADD_4Z,ADD_4O,TEMPSUMMON_TIMED_DESPAWN,240000);
        m_creature->SummonCreature(10698,ADD_5X,ADD_5Y,ADD_5Z,ADD_5O,TEMPSUMMON_TIMED_DESPAWN,240000);
        m_creature->SummonCreature(10698,ADD_6X,ADD_6Y,ADD_6Z,ADD_6O,TEMPSUMMON_TIMED_DESPAWN,240000);
        m_creature->SummonCreature(10698,ADD_7X,ADD_7Y,ADD_7Z,ADD_7O,TEMPSUMMON_TIMED_DESPAWN,240000);
        m_creature->SummonCreature(10698,ADD_8X,ADD_8Y,ADD_8Z,ADD_8O,TEMPSUMMON_TIMED_DESPAWN,240000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //START NOT TRANSFORMED
        if (!Transformed)
        {
            if (_ChangedNameCrusadersHammer_Timer.Expired(diff) && !m_creature->IsNonMeleeSpellCast(false))
            {
                //Cast
                if (rand()%100 < 75) //50% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_CRUSADERSHAMMER);
                }
                //15 seconds until we should cast this again
                _ChangedNameCrusadersHammer_Timer = 12000;
            }

            if (_ChangedNameCrusaderStrike_Timer.Expired(diff) && !m_creature->IsNonMeleeSpellCast(false))
            {
                //Cast
                if (rand()%100 < 60) //50% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_CRUSADERSTRIKE);
                }
                //15 seconds until we should cast this again
                _ChangedNameCrusaderStrike_Timer = 15000;
            }

            if (_ChangedNameMindBlast_Timer.Expired(diff) && !m_creature->IsNonMeleeSpellCast(false))
            {
                //Cast
                if (rand()%100 < 70) //70% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_MINDBLAST);
                }
                //15 seconds until we should cast this again
                _ChangedNameMindBlast_Timer = 10000;
            }

            if (_ChangedNameHolyStrike_Timer.Expired(diff) && !m_creature->IsNonMeleeSpellCast(false))
            {
                //Cast
                if (rand()%100 < 50) //50% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_HOLYSTRIKE);
                }
                //15 seconds until we should cast this again
                _ChangedNameHolyStrike_Timer = 15000;
            }

            if (_ChangedNameDazed_Timer.Expired(diff) && !m_creature->IsNonMeleeSpellCast(false))
            {
                //Cast
                if (rand()%100 < 50) //50% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_DAZED);
                }
                //15 seconds until we should cast this again
                _ChangedNameDazed_Timer = 15000;
            }

            //BalnazzarTransform
            if (me->GetHealthPercent() < 40)
            {
                //Cast
                DoCast(m_creature,SPELL_BALNAZZARTRANSFORM); //restore hp, mana and stun
                m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,10691); //then change disaply id
                m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.00f); //then, change size
                Transformed = true;
            }

            //START ELSE TRANSFORMED
        } else {


            if (_ChangedNameMindBlast_Timer.Expired(diff) && !m_creature->IsNonMeleeSpellCast(false))
            {
                //Cast
                if (rand()%100 < 60) //70% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_MINDBLAST);
                }
                //15 seconds until we should cast this again
                _ChangedNameMindBlast_Timer = 10000;
            }

            if (_ChangedNameShadowShock_Timer.Expired(diff))
            {
                //Cast
                if (rand()%100 < 80) //80% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_SHADOWSHOCK);
                }
                //15 seconds until we should cast this again
                _ChangedNameShadowShock_Timer = 11000;
            }

            if (_ChangedNamePsychicScream_Timer.Expired(diff))
            {
                //Cast
                if (rand()%100 < 60) //60% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_PSYCHICSCREAM);
                    if(DoGetThreat(m_creature->GetVictim()))
                        DoModifyThreatPercent(m_creature->GetVictim(),-50);
                }
                //15 seconds until we should cast this again
                _ChangedNamePsychicScream_Timer = 20000;
            }

            if (_ChangedNameDeepSleep_Timer.Expired(diff))
            {
                //Cast
                if (rand()%100 < 55) //55% chance to cast
                {
                    //Cast
                    Unit* target = NULL;

                    target = SelectUnit(SELECT_TARGET_RANDOM,0);
                    if (target)
                    DoCast(target,SPELL_DEEPSLEEP);
                }
                //15 seconds until we should cast this again
                _ChangedNameDeepSleep_Timer = 15000;
            }

            if (_ChangedNameShadowBoltVolley_Timer.Expired(diff))
            {
                //Cast
                if (rand()%100 < 75) //75% chance to cast
                {
                    DoCast(m_creature->GetVictim(),SPELL_SHADOWBOLTVOLLEY);
                }
                //15 seconds until we should cast this again
                _ChangedNameShadowBoltVolley_Timer = 13000;
            }

            //            if (_ChangedNameMindControl_Timer.Expired(diff))
            //            {
            //Cast
            //                if (rand()%100 < 50) //50% chance to cast
            //                {
            //                DoCast(m_creature->GetVictim(),SPELL_MINDCONTROL);
            //                }
            //15 seconds until we should cast this again
            //                _ChangedNameMindControl_Timer = 15000;
            //            }

            //END ELSE TRANSFORMED
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_dathrohan_balnazzar(Creature *_Creature)
{
    return new boss_dathrohan_balnazzarAI (_Creature);
}

void AddSC_boss_dathrohan_balnazzar()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_dathrohan_balnazzar";
    newscript->GetAI = &GetAI_boss_dathrohan_balnazzar;
    newscript->RegisterSelf();
}


