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
SDName: Boss_Jeklik
SD%Complete: 85
SDComment: Problem in finding the right flying batriders for spawning and making them fly.
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SAY_AGGRO                   -1309002
#define SAY_RAIN_FIRE               -1309003
#define SAY_DEATH                   -1309004

#define SPELL_CHARGE              22911
#define SPELL_SONICBURST          23918
#define SPELL_SCREECH             6605
#define SPELL_SHADOW_WORD_PAIN    23952
#define SPELL_MIND_FLAY           23953
#define SPELL_CHAIN_MIND_FLAY     26044                     //Right ID unknown. So disabled
#define SPELL_GREATERHEAL         23954
#define SPELL_BAT_FORM            23966

// Batriders Spell

#define SPELL_BOMB                40332                     //Wrong ID but Magmadars bomb is not working...

#define BAT_ID      11368
#define BAT_COUNT   6

float batLocations[BAT_COUNT][3] = {
        {-12291.6220, -1380.2640, 144.8304},
        {-12289.6220, -1380.2640, 144.8304},
        {-12293.6220, -1380.2640, 144.8304},
        {-12291.6220, -1380.2640, 144.8304},
        {-12289.6220, -1380.2640, 144.8304},
        {-12293.6220, -1380.2640, 144.8304}};

struct boss_jeklikAI : public ScriptedAI
{
    boss_jeklikAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer _ChangedNameCharge_Timer;
    Timer _ChangedNameSonicBurst_Timer;
    Timer _ChangedNameScreech_Timer;
    Timer _ChangedNameSpawnBats_Timer;
    Timer _ChangedNameShadowWordPain_Timer;
    Timer _ChangedNameMindFlay_Timer;
    Timer _ChangedNameChainMindFlay_Timer;
    Timer _ChangedNameGreaterHeal_Timer;
    Timer _ChangedNameSpawnFlyingBats_Timer;

    bool PhaseTwo;

    void Reset()
    {
        _ChangedNameCharge_Timer.Reset(20000);
        _ChangedNameSonicBurst_Timer.Reset(8000);
        _ChangedNameScreech_Timer.Reset(13000);
        _ChangedNameSpawnBats_Timer.Reset(60000);
        _ChangedNameShadowWordPain_Timer.Reset(6000);
        _ChangedNameMindFlay_Timer.Reset(11000);
        _ChangedNameChainMindFlay_Timer.Reset(26000);
        _ChangedNameGreaterHeal_Timer.Reset(50000);
        _ChangedNameSpawnFlyingBats_Timer.Reset(10000);

        PhaseTwo = false;

        pInstance->SetData(DATA_JEKLIKEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        DoCast(m_creature,SPELL_BAT_FORM);
        pInstance->SetData(DATA_JEKLIKEVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        pInstance->SetData(DATA_JEKLIKEVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_creature->GetVictim() && m_creature->isAlive())
        {
            if (me->GetHealthPercent() > 50)
            {
                if (_ChangedNameCharge_Timer.Expired(diff))
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_CHARGE), true))
                    {
                        DoCast(target,SPELL_CHARGE);
                        AttackStart(target);
                    }

                    _ChangedNameCharge_Timer = 15000 + rand()%15000;
                }
                

                if (_ChangedNameSonicBurst_Timer.Expired(diff))
                {
                    DoCast(m_creature->GetVictim(),SPELL_SONICBURST);
                    _ChangedNameSonicBurst_Timer = 8000 + rand()%5000;
                }
                

                if (_ChangedNameScreech_Timer.Expired(diff))
                {
                    DoCast(m_creature->GetVictim(),SPELL_SCREECH);
                    _ChangedNameScreech_Timer = 18000 + rand()%8000;
                }
                

                if (_ChangedNameSpawnBats_Timer.Expired(diff))
                {
                    Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0);

                    if (target)
                    {
                        Creature* Bat = NULL;
                        for (uint8 i = 0 ; i < BAT_COUNT; ++i)
                        {
                            Bat = m_creature->SummonCreature(BAT_ID, batLocations[i][0], batLocations[i][1], batLocations[i][2], 5.483, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                            if (Bat)
                                Bat->AI()->AttackStart(target);
                        }
                    }

                    _ChangedNameSpawnBats_Timer = 60000;
                }
            }
            else
            {
                if (PhaseTwo)
                {
                    if (PhaseTwo && _ChangedNameShadowWordPain_Timer.Expired(diff))
                    {
                        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0,GetSpellMaxRange(SPELL_SHADOW_WORD_PAIN), true))
                        {
                            DoCast(target, SPELL_SHADOW_WORD_PAIN);
                            _ChangedNameShadowWordPain_Timer = 12000 + rand()%6000;
                        }
                    }
                    

                    if (_ChangedNameMindFlay_Timer.Expired(diff))
                    {
                        DoCast(m_creature->GetVictim(), SPELL_MIND_FLAY);
                        _ChangedNameMindFlay_Timer = 16000;
                    }
                    

                    if (_ChangedNameChainMindFlay_Timer.Expired(diff))
                    {
                        m_creature->InterruptNonMeleeSpells(false);
                        DoCast(m_creature->GetVictim(), SPELL_CHAIN_MIND_FLAY);
                        _ChangedNameChainMindFlay_Timer = 15000 + rand()%15000;
                    }
                    

                    if (_ChangedNameGreaterHeal_Timer.Expired(diff))
                    {
                        m_creature->InterruptNonMeleeSpells(false);
                        DoCast(m_creature,SPELL_GREATERHEAL);
                        _ChangedNameGreaterHeal_Timer = 25000 + rand()%10000;
                    }
                    
                    if (_ChangedNameSpawnFlyingBats_Timer.Expired(diff))
                    {
                        Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                        if (!target)
                            return;
                        // Does 4k damage weird. They despawn almost immediately
                        Creature* FlyingBat = m_creature->SummonCreature(14965, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ()+15, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                        if (FlyingBat)
                            FlyingBat->AI()->AttackStart(target);

                        _ChangedNameSpawnFlyingBats_Timer = 10000 + rand()%5000;
                    }
                }
                else    
                {
                    m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, 15219);
                    DoResetThreat();
                    PhaseTwo = true;
                }
            }

            DoMeleeAttackIfReady();
        }
    }
};

//Flying Bat
struct mob_batriderAI : public ScriptedAI
{
    mob_batriderAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer _ChangedNameBomb_Timer;
    Timer _ChangedNameCheck_Timer;

    void Reset()
    {
        _ChangedNameBomb_Timer.Reset(2000);
        _ChangedNameCheck_Timer.Reset(1000);

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI (const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameBomb_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_BOMB), true))
            {
                DoCast(target, SPELL_BOMB);
                _ChangedNameBomb_Timer = 5000;
            }
        }
        

        if (_ChangedNameCheck_Timer.Expired(diff))
        {
            if (pInstance)
            {
                if (pInstance->GetData(DATA_JEKLIKEVENT) == DONE)
                {
                    m_creature->setDeathState(JUST_DIED);
                    m_creature->RemoveCorpse();
                }
            }

            _ChangedNameCheck_Timer = 1000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_jeklik(Creature *_Creature)
{
    return new boss_jeklikAI (_Creature);
}

CreatureAI* GetAI_mob_batrider(Creature *_Creature)
{
    return new mob_batriderAI (_Creature);
}

void AddSC_boss_jeklik()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_jeklik";
    newscript->GetAI = &GetAI_boss_jeklik;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_batrider";
    newscript->GetAI = &GetAI_mob_batrider;
    newscript->RegisterSelf();
}


