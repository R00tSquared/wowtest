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
SDName: Boss_Sulfuron_Harbringer
SD%Complete: 80
SDComment: Adds NYI
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define SPELL_DARKSTRIKE            19777
#define SPELL_DEMORALIZINGSHOUT     19778
#define SPELL_INSPIRE               19779
#define SPELL_KNOCKDOWN             19780
#define SPELL_FLAMESPEAR            19781

//Adds Spells
#define SPELL_HEAL                  19775
#define SPELL_SHADOWWORDPAIN        19776
#define SPELL_IMMOLATE              20294

struct boss_sulfuronAI : public ScriptedAI
{
    boss_sulfuronAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    Timer _ChangedNameDarkstrike_Timer;
    Timer _ChangedNameDemoralizingShout_Timer;
    Timer _ChangedNameInspire_Timer;
    Timer _ChangedNameKnockdown_Timer;
    Timer _ChangedNameFlamespear_Timer;
    ScriptedInstance *pInstance;

    void Reset()
    {
        _ChangedNameDarkstrike_Timer.Reset(10000);                             //These times are probably wrong
        _ChangedNameDemoralizingShout_Timer.Reset(15000);
        _ChangedNameInspire_Timer.Reset(13000);
        _ChangedNameKnockdown_Timer.Reset(6000);
        _ChangedNameFlamespear_Timer.Reset(2000);

        if (pInstance)
            pInstance->SetData(DATA_SULFURON_HARBRINGER_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (pInstance)
            pInstance->SetData(DATA_SULFURON_HARBRINGER_EVENT, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_SULFURON_HARBRINGER_EVENT, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (_ChangedNameDemoralizingShout_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_DEMORALIZINGSHOUT);
            _ChangedNameDemoralizingShout_Timer = 15000 + rand()%5000;
        }

        if (_ChangedNameInspire_Timer.Expired(diff))
        {
            Creature* target = NULL;
            std::list<Creature*> pList = FindFriendlyMissingBuff(45.0f,SPELL_INSPIRE);
            if (!pList.empty())
            {
                std::list<Creature*>::iterator i = pList.begin();
                advance(i, (rand()%pList.size()));
                target = (*i);
            }

            if (target)
                DoCast(target,SPELL_INSPIRE);

            DoCast(m_creature,SPELL_INSPIRE);

            _ChangedNameInspire_Timer = 20000 + rand()%6000;
        }

        if (_ChangedNameKnockdown_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_KNOCKDOWN);
            _ChangedNameKnockdown_Timer = 12000 + rand()%3000;
        }

        if (_ChangedNameFlamespear_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_FLAMESPEAR);

            _ChangedNameFlamespear_Timer = 12000 + rand()%4000;
        }

        if (_ChangedNameDarkstrike_Timer.Expired(diff))
        {
            DoCast(m_creature, SPELL_DARKSTRIKE);
            _ChangedNameDarkstrike_Timer = 15000 + rand()%3000;
        }

        DoMeleeAttackIfReady();
    }
};

struct mob_flamewaker_priestAI : public ScriptedAI
{
    mob_flamewaker_priestAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    Timer _ChangedNameHeal_Timer;
    Timer _ChangedNameShadowWordPain_Timer;
    Timer _ChangedNameImmolate_Timer;

    ScriptedInstance *pInstance;

    void Reset()
    {
        _ChangedNameHeal_Timer.Reset(15000+rand()%15000);
        _ChangedNameShadowWordPain_Timer.Reset(2000);
        _ChangedNameImmolate_Timer.Reset(8000);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        //Casting Heal to Sulfuron or other Guards.
        if(_ChangedNameHeal_Timer.Expired(diff))
        {
            Unit* pUnit = SelectLowestHpFriendly(60.0f, 1);
            if (!pUnit)
                return;

            DoCast(pUnit, SPELL_HEAL);

            _ChangedNameHeal_Timer = 15000+rand()%5000;
        }

        if (_ChangedNameShadowWordPain_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_SHADOWWORDPAIN);

            _ChangedNameShadowWordPain_Timer = 18000+rand()%8000;
        }

        
        if (_ChangedNameImmolate_Timer.Expired(diff))
        {
            Unit* target = NULL;
            target = SelectUnit(SELECT_TARGET_RANDOM,0);
            if (target) DoCast(target,SPELL_IMMOLATE);

            _ChangedNameImmolate_Timer = 15000+rand()%10000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_sulfuron(Creature *_Creature)
{
    return new boss_sulfuronAI (_Creature);
}

CreatureAI* GetAI_mob_flamewaker_priest(Creature *_Creature)
{
    return new mob_flamewaker_priestAI (_Creature);
}

void AddSC_boss_sulfuron()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_sulfuron";
    newscript->GetAI = &GetAI_boss_sulfuron;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_flamewaker_priest";
    newscript->GetAI = &GetAI_mob_flamewaker_priest;
    newscript->RegisterSelf();
}


