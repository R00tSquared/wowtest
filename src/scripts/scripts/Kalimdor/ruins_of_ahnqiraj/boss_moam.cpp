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
SDName: Boss_Moam
SD%Complete: 90
SDComment: Mana Fiends not 100% scripted, rest should be OK
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_ruins_of_ahnqiraj.h"

enum
{
    EMOTE_AGGRO                 = -1509000,
    EMOTE_MANA_FULL             = -1509001,
    EMOTE_ENERGIZING            = -1509028,

    SPELL_TRAMPLE               = 15550,
    SPELL_DRAINMANA             = 25671,
    SPELL_ARCANE_ERUPTION       = 25672,
    // SPELL_SUMMONMANA         = 25681,
    SPELL_SUMMON_MANAFIEND_1    = 25681,
    SPELL_SUMMON_MANAFIEND_2    = 25682,
    SPELL_SUMMON_MANAFIEND_3    = 25683,
    SPELL_ENERGIZE              = 25685,

    SPELL_ARCANE_EXPLOSION      = 25679,
    SPELL_COUNTERSPELL          = 15122,

    PHASE_ATTACKING             = 0,
    PHASE_ENERGAZING            = 1
};


struct boss_moamAI : public ScriptedAI
{
    boss_moamAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)c->GetInstanceData();
    }

    ScriptedInstance * pInstance;

    uint8 Phase;

    Timer TrampleTimer;
    Timer ManaDrainTimer;
    Timer CheckoutManaTimer;
    Timer SummonManaFiendsTimer;

    void Reset()
    {
        TrampleTimer.Reset(9000);
        ManaDrainTimer.Reset(3000);
        CheckoutManaTimer.Reset(1500);
        SummonManaFiendsTimer.Reset(90000);
        Phase = PHASE_ATTACKING;
        me->SetPower(POWER_MANA, 0);
        me->SetMaxPower(POWER_MANA, 0);
        me->RemoveAllAurasNotCreatureAddon();

        if (pInstance)
            pInstance->SetData(DATA_MOAM, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(EMOTE_AGGRO, me);
        me->SetMaxPower(POWER_MANA, me->GetCreatureInfo()->maxmana);

        if (pInstance)
        {
            DoZoneInCombat();
            pInstance->SetData(DATA_MOAM, IN_PROGRESS);
        }
    }

    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_MOAM, DONE);
        // summon obsidian shard. IDK what it is for
        me->SummonGameObject(181069, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, 0, 0, 0, 0, 0);
    }

    void JustSummoned(Creature *creature)
    {
        // Add threat to everyone from Moam' threatlist
        std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
        std::list<HostileReference*>::iterator itr;

        for(itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
        {
            Unit* target = NULL;
            target = Unit::GetUnit((*me), (*itr)->getUnitGuid());
            if(target)
            {
                creature->AddThreat(target, 1.0f);
                creature->SetInCombatWith(target);
                target->SetInCombatWith(creature);
            }
        }
        // Attack random target
        Unit* randomTarget = SelectUnit(SELECT_TARGET_RANDOM, 0);
        if(randomTarget)
        {
            creature->setActive(true);
            creature->AI()->AttackStart(randomTarget);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(pInstance && pInstance->GetData(DATA_MOAM) == IN_PROGRESS)
        {
            if(Phase == PHASE_ENERGAZING)
            {
                if(CheckoutManaTimer.Expired(diff))
                {
                    if (me->GetPower(POWER_MANA) == me->GetMaxPower(POWER_MANA))
                    {
                        me->RemoveAurasDueToSpell(SPELL_ENERGIZE);
                        DoCast(me, SPELL_ARCANE_ERUPTION);
                        DoScriptText(EMOTE_MANA_FULL, me);
                        Phase = PHASE_ATTACKING;
                        SummonManaFiendsTimer = 90000;
                        return;
                    }
                    CheckoutManaTimer = 1500;
                }
            }
        }

        if (!UpdateVictim())
            return;

        if(Phase == PHASE_ATTACKING)
        {
            if(SummonManaFiendsTimer.Expired(diff))
            {
                // Summon 3 Manafiends
                DoCast(me->GetVictim(), SPELL_SUMMON_MANAFIEND_1, true);
                DoCast(me->GetVictim(), SPELL_SUMMON_MANAFIEND_2, true);
                DoCast(me->GetVictim(), SPELL_SUMMON_MANAFIEND_3, true);
                // Cast Energize
                DoCast(me, SPELL_ENERGIZE);
                DoScriptText(EMOTE_ENERGIZING, me);
                Phase = PHASE_ENERGAZING;
                return;
            }

            if (ManaDrainTimer.Expired(diff))
            {
                std::list<Player*> manaList;
                Map::PlayerList const &PlayerList = ((InstanceMap*)me->GetMap())->GetPlayers();
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {
                    if (Player* manaTarget = i->getSource())
                        if (manaTarget->isAlive() && me->IsWithinDist(manaTarget, 100) && manaTarget->getPowerType() == POWER_MANA && manaTarget->GetPower(POWER_MANA) > 500)
                            manaList.push_back(manaTarget);
                }
                // if there's 3 targets - this will return same list and will not do anything
                Hellground::RandomResizeList(manaList, 6); // max 6 targets
        
                for (std::list<Player*>::const_iterator t = manaList.begin(); t != manaList.end(); t++)
                    DoCast(*t, SPELL_DRAINMANA, true);
        
                ManaDrainTimer = 5000;
            }

            if (TrampleTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_TRAMPLE);
                TrampleTimer = 15000;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_moam(Creature *_Creature)
{
    return new boss_moamAI (_Creature);
}

struct mana_fiendAI : public ScriptedAI
{
    mana_fiendAI(Creature *c) : ScriptedAI(c) {}

    Timer ArcaneExplosionTimer;
    Timer CounterspellTimer;

    void Reset()
    {
        ArcaneExplosionTimer = 3000;
        CounterspellTimer    = 2000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ArcaneExplosionTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_ARCANE_EXPLOSION);
            ArcaneExplosionTimer = 3000;
        }

        if(CounterspellTimer.Expired(diff))
        {
            if(Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, SPELL_COUNTERSPELL, true, POWER_MANA))
                DoCast(pTarget, SPELL_COUNTERSPELL);
            CounterspellTimer = urand(5000, 10000);
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mana_fiend(Creature *_Creature)
{
    return new mana_fiendAI (_Creature);
}

void AddSC_boss_moam()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_moam";
    newscript->GetAI = &GetAI_boss_moam;
    newscript->RegisterSelf();
}

void AddSC_mana_fiend()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="mana_fiend";
    newscript->GetAI = &GetAI_mana_fiend;
    newscript->RegisterSelf();
}

