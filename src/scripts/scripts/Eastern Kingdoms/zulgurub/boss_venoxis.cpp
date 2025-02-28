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
SDName: Boss_Venoxis
SD%Complete: 100
SDComment:
SDCategory: Zul'Gurub
EndScriptData */

#include "precompiled.h"
#include "def_zulgurub.h"

#define SAY_TRANSFORM       -1309000
#define SAY_DEATH           -1309001

#define SPELL_HOLY_FIRE     23860
#define SPELL_HOLY_WRATH    28883                           //Not sure if this or 23979
#define SPELL_VENOMSPIT     23862
#define SPELL_HOLY_NOVA     23858
#define SPELL_POISON_CLOUD  23861
#define SPELL_SNAKE_FORM    23849
#define SPELL_RENEW         23895
#define SPELL_BERSERK       23537
#define SPELL_DISPELL       23859

struct boss_venoxisAI : public ScriptedAI
{
    boss_venoxisAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    Timer _ChangedNameHolyFire_Timer;
    Timer _ChangedNameHolyWrath_Timer;
    Timer _ChangedNameVenomSpit_Timer;
    Timer _ChangedNameRenew_Timer;
    Timer _ChangedNamePoisonCloud_Timer;
    Timer _ChangedNameHolyNova_Timer;
    Timer _ChangedNameDispell_Timer;
    int32 TargetInRange;

    bool PhaseTwo;
    bool InBerserk;

    void Reset()
    {
        _ChangedNameHolyFire_Timer.Reset(10000);
        _ChangedNameHolyWrath_Timer.Reset(60500);
        _ChangedNameVenomSpit_Timer.Reset(5500);
        _ChangedNameRenew_Timer.Reset(30500);
        _ChangedNamePoisonCloud_Timer.Reset(2000);
        _ChangedNameHolyNova_Timer.Reset(5000);
        _ChangedNameDispell_Timer.Reset(35000);
        TargetInRange = 0;

        PhaseTwo = false;
        InBerserk= false;
        pInstance->SetData(DATA_VENOXISEVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        pInstance->SetData(DATA_VENOXISEVENT, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);
        if(pInstance)
            pInstance->SetData(DATA_VENOXISEVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
          if (!UpdateVictim())
            return;

            if ((me->GetHealthPercent() > 50))
            {
                if (_ChangedNameDispell_Timer.Expired(diff))
                {
                    DoCast(m_creature, SPELL_DISPELL);
                    _ChangedNameDispell_Timer = 15000 + rand()%15000;
                }
                

                if (_ChangedNameRenew_Timer.Expired(diff))
                {
                    DoCast(m_creature, SPELL_RENEW);
                    _ChangedNameRenew_Timer = 20000 + rand()%10000;
                }
                

                if(_ChangedNameHolyWrath_Timer.Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_HOLY_WRATH);
                    _ChangedNameHolyWrath_Timer = 15000 + rand()%10000;
                }
                

                if(_ChangedNameHolyNova_Timer.Expired(diff))
                {
                    if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0, 5, true))
                    {
                        DoCast(m_creature,SPELL_HOLY_NOVA);
                        _ChangedNameHolyNova_Timer = 1000;
                    }
                    else
                        _ChangedNameHolyNova_Timer = 2000;
                }
                

                if (_ChangedNameHolyFire_Timer.Expired(diff) && TargetInRange < 3)
                {
                    if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0, GetSpellMaxRange(SPELL_HOLY_FIRE), true))
                        DoCast(target, SPELL_HOLY_FIRE);

                    _ChangedNameHolyFire_Timer = 8000;
                }
                
            }
            else
            {
                if(!PhaseTwo)
                {
                    DoScriptText(SAY_TRANSFORM, m_creature);
                    m_creature->InterruptNonMeleeSpells(false);
                    DoCast(m_creature,SPELL_SNAKE_FORM);
                    m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, 2.00f);
                    const CreatureInfo *cinfo = m_creature->GetCreatureInfo();
                    m_creature->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg +((cinfo->mindmg/100) * 25)) * m_creature->GetCreatureDamageMod());
                    m_creature->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg +((cinfo->maxdmg/100) * 25)) * m_creature->GetCreatureDamageMod());
                    m_creature->UpdateDamagePhysical(BASE_ATTACK);
                    DoResetThreat();
                    PhaseTwo = true;
                }

                if(PhaseTwo && _ChangedNamePoisonCloud_Timer.Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_POISON_CLOUD);
                    _ChangedNamePoisonCloud_Timer = 15000;
                }
                

                if(PhaseTwo && _ChangedNameVenomSpit_Timer.Expired(diff))
                {
                    if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_VENOMSPIT), true))
                        DoCast(target, SPELL_VENOMSPIT);

                    _ChangedNameVenomSpit_Timer = 15000 + rand()%5000;
                }
                

                if(PhaseTwo && (me->GetHealthPercent() < 11))
                {
                    if(!InBerserk)
                    {
                        m_creature->InterruptNonMeleeSpells(false);
                        DoCast(m_creature, SPELL_BERSERK);
                        InBerserk = true;
                    }
                }
            }
            DoMeleeAttackIfReady();

    }
};
CreatureAI* GetAI_boss_venoxis(Creature *_Creature)
{
    return new boss_venoxisAI (_Creature);
}

void AddSC_boss_venoxis()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_venoxis";
    newscript->GetAI = &GetAI_boss_venoxis;
    newscript->RegisterSelf();
}


