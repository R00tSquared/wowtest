// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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
SDName: Boss_Omar_The_Unscarred
SD%Complete: 98
SDComment: Timers may be incorrect
SDCategory: Hellfire Citadel, Hellfire Ramparts
EndScriptData */

#include "precompiled.h"
#include "hellfire_ramparts.h"

enum OmorTheUnscarred
{
    SAY_AGGRO_1                     = -1543009,
    SAY_AGGRO_2                     = -1543010,
    SAY_AGGRO_3                     = -1543011,
    SAY_SUMMON                      = -1543012,
    SAY_CURSE                       = -1543013,
    SAY_KILL_1                      = -1543014,
    SAY_DIE                         = -1543015,
    SAY_WIPE                        = -1543016,

    SPELL_ORBITAL_STRIKE            = 30637,
    SPELL_SHADOW_WHIP               = 30638,
    SPELL_BANE_OF_AURA_TREACHERY    = 30695,
    SPELL_BANE_OF_AURA_TREACHERY_H  = 37566,
    SPELL_DEMONIC_SHIELD            = 31901,
    SPELL_SHADOW_BOLT               = 30686,
    SPELL_SHADOW_BOLT_H             = 39297,
    SPELL_SUMMON_FIENDISH_HOUND     = 30707
};


struct boss_omor_the_unscarredAI : public ScriptedAI
{
    boss_omor_the_unscarredAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer OrbitalStrike_Timer;
    Timer ShadowWhip_Timer;
    Timer Aura_Timer;
    Timer DemonicShield_Timer;
    Timer Shadowbolt_Timer;
    Timer Summon_Timer;
    Timer Check_Timer;
    uint64 playerGUID;
    bool CanPullBack;

    void Reset()
    {
        OrbitalStrike_Timer.Reset(22000);
        ShadowWhip_Timer.Reset(2000);
        Aura_Timer.Reset(urand(10000, 15000));
        DemonicShield_Timer.Reset(1000);
        Shadowbolt_Timer.Reset(1000);
        Summon_Timer.Reset(20000);
        Check_Timer.Reset(3000);
        playerGUID = 0;
        CanPullBack = false;

        if (pInstance)
            pInstance->SetData(DATA_OMOR, NOT_STARTED);
    }

    void EnterEvadeMode()
    {
        DoScriptText(SAY_WIPE, me);
        CreatureAI::EnterEvadeMode();
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), me);

        if (pInstance)
            pInstance->SetData(DATA_OMOR, IN_PROGRESS);
    }

    void KilledUnit(Unit* victim)
    {
        if (rand()%2)
            return;

        DoScriptText(SAY_KILL_1, me);
    }

    void JustSummoned(Creature* summoned)
    {
        DoScriptText(SAY_SUMMON, me);

        if (Unit* random = SelectUnit(SELECT_TARGET_RANDOM,0))
            summoned->AI()->AttackStart(random);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DIE, me);

        if (pInstance)
        {
            pInstance->SetData(DATA_OMOR, DONE);
            Map::PlayerList const &plList = me->GetMap()->GetPlayers();
            if (plList.isEmpty())
                return;
            for (Map::PlayerList::const_iterator i = plList.begin(); i != plList.end(); ++i)
            {
                if (Player* plr = i->getSource())
                {
                    if (plr->isAlive() && plr->HasAura(HeroicMode ? SPELL_BANE_OF_AURA_TREACHERY_H : SPELL_BANE_OF_AURA_TREACHERY))
                        plr->RemoveAurasDueToSpell(HeroicMode ? SPELL_BANE_OF_AURA_TREACHERY_H : SPELL_BANE_OF_AURA_TREACHERY);
                }
            }
        }     
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Check_Timer.Expired(diff))
        {
            if(me->GetDistance(-1125.04, 1708.81, 89.692) > 75.0f)
                EnterEvadeMode();
            Check_Timer = 3000;
        }

        if (Summon_Timer.Expired(diff))
        {
            AddSpellToCast(me, SPELL_SUMMON_FIENDISH_HOUND);
            Summon_Timer = 18000;
        }

        if (CanPullBack)
        {
            if (ShadowWhip_Timer.Expired(diff))
            {
                if (Unit* temp = Unit::GetUnit(*me,playerGUID))
                {
                    if (temp->HasUnitMovementFlag(MOVEFLAG_FALLINGFAR))
                    {
                        me->InterruptNonMeleeSpells(false);
                        DoCast(temp, SPELL_SHADOW_WHIP);
                    }
                    else 
                    {
                        if (!temp->HasUnitMovementFlag(MOVEFLAG_FALLINGFAR))
                        {
                            playerGUID = 0;
                            CanPullBack = false;
                        }
                    }
                }
                ShadowWhip_Timer = 2200;
            }
        }
        else if (OrbitalStrike_Timer.Expired(diff))
        {
            Unit *temp = SelectUnit(SELECT_TARGET_NEAREST, 0, 100, true);

            if (temp && temp->GetTypeId() == TYPEID_PLAYER && me->IsWithinMeleeRange(temp))
            {
                me->InterruptNonMeleeSpells(false);
                me->SetSelection(temp->GetGUID());
                DoCast(temp, SPELL_ORBITAL_STRIKE);
                OrbitalStrike_Timer = urand(14000, 16000);
                playerGUID = temp->GetGUID();

                if (playerGUID)
                {
                    CanPullBack = true;
                    ShadowWhip_Timer = 3000;
                }
            }
        }

        if (me->GetHealthPercent() < 20)
        {
            if (DemonicShield_Timer.Expired(diff))
            {
                AddSpellToCast(me, SPELL_DEMONIC_SHIELD);
                DemonicShield_Timer = 15000;
            }
        }

        if (Aura_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_BANE_OF_AURA_TREACHERY), true))
            {
                DoScriptText(SAY_CURSE, me);
                AddSpellToCast(target, HeroicMode ? SPELL_BANE_OF_AURA_TREACHERY_H : SPELL_BANE_OF_AURA_TREACHERY);
                Aura_Timer = urand(8000, 16000);
            }
        }

        if (Shadowbolt_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
            {
                if(!me->IsWithinMeleeRange(target))
                {
                    AddSpellToCast(target, HeroicMode ? SPELL_SHADOW_BOLT_H : SPELL_SHADOW_BOLT);
                    Shadowbolt_Timer = 3000;
                }
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_omor_the_unscarredAI(Creature *_Creature)
{
    return new boss_omor_the_unscarredAI (_Creature);
}

void AddSC_boss_omor_the_unscarred()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_omor_the_unscarred";
    newscript->GetAI = &GetAI_boss_omor_the_unscarredAI;
    newscript->RegisterSelf();
}

