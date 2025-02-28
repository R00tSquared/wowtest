// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_Murmur
SD%Complete: 90
SDComment: TODO: add fight event when player enter in the room
SDCategory: Auchindoun, Shadow Labyrinth
EndScriptData */

#include "precompiled.h"

#define EMOTE_SONIC_BOOM            -1555036

// Intro spells
#define SPELL_SUPPRESSION_BLAST     33332
#define SPELL_MURMURS_WRATH         33331

// Battle spells
#define SPELL_SONIC_BOOM_CAST       (HeroicMode?38796:33923)
// triggered by previous in dummy effect
#define SPELL_SONIC_BOOM            (HeroicMode?38795:33666)
#define SPELL_RESONANCE             33657
#define SPELL_MURMURS_TOUCH         (HeroicMode?38794:33711)
#define SPELL_MAGNETIC_PULL         33689
#define SPELL_SONIC_SHOCK           38797
#define SPELL_THUNDERING_STORM      39365

struct boss_murmurAI : public Scripted_NoMovementAI
{
    boss_murmurAI(Creature *c) : Scripted_NoMovementAI(c), summons(c)
    {
        HeroicMode = me->GetMap()->IsHeroic();
    }

    SummonList summons;
    Timer SonicBoom_Timer;
    Timer MurmursTouch_Timer;
    Timer Resonance_Timer;
    Timer MagneticPull_Timer;
    Timer SonicShock_Timer;
    Timer ThunderingStorm_Timer;
    Timer Timer_SuppressionBlast;

    bool HeroicMode;

    void Reset()
    {
        summons.DespawnAll();
        Timer_SuppressionBlast.Reset(5000);
        SonicBoom_Timer.Reset((HeroicMode ? 40000 : 50000));
        MurmursTouch_Timer.Reset((HeroicMode ? 28000 : 31000));
        Resonance_Timer.Reset(5000);
        MagneticPull_Timer.Reset(45000);
        ThunderingStorm_Timer.Reset(5000);
        SonicShock_Timer.Reset(5000);

        //database should have `RegenHealth`=0 to prevent regen
        uint32 hp = (me->GetMaxHealth()*40)/100;
        if (hp)
            me->SetHealth(hp);

        me->ResetPlayerDamageReq();
    }

    void JustSummoned(Creature* summ)
    {
        if (summ->GetEntry() == 18634 || summ->GetEntry() == 18639)
        {
            summ->AI()->ReceiveAIEvent(AI_EVENT_CUSTOM_EVENTAI_B, me, me, 0);
            summ->SetOwnerGUID(me->GetGUID());
        }
        summons.Summon(summ);
    }

    void EnterCombat(Unit *who) { }

    void SpellHit(Unit* caster, const SpellEntry* spell) 
    {
        if (spell && spell->Id == 33335 && caster && caster->GetOwnerGUID() == me->GetGUID())
            me->CastSpell(caster, SPELL_MURMURS_WRATH, false);
    }

    // Murmurs Touch heroic, maybe spell exist ?
    void SpellHitTarget(Unit *target, const SpellEntry *spell)
    {
        if (target && target->isAlive() && spell && spell->Id == 38794)
        {
            std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
            for(std::list<HostileReference*>::iterator i = m_threatlist.begin(); i != m_threatlist.end(); ++i)
            {
                if (Unit* targets = Unit::GetUnit((*me),(*i)->getUnitGuid()))
                {
                    if (targets->isAlive() && target->GetGUID() != (*i)->getUnitGuid())
                        target->CastSpell(targets, SPELL_MAGNETIC_PULL, true);
                }
            }
        }

        return;
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target or casting
        if (!UpdateVictim() || me->IsNonMeleeSpellCast(false))
        {
            if(Timer_SuppressionBlast.Expired(diff))
            {
                DoCast(me, SPELL_SUPPRESSION_BLAST, false);
                Timer_SuppressionBlast = 5000;
            }
            return;
        }

        // Murmur's Touch
        if (MurmursTouch_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                AddSpellToCast(target, SPELL_MURMURS_TOUCH);

            MurmursTouch_Timer = (HeroicMode ? 30000 : 20000);
        }
        

        // Resonance
        if (Resonance_Timer.Expired(diff))
        {
            Unit *target = SelectUnit(SELECT_TARGET_NEAREST, 0, 100, true);

            if (target && !me->IsWithinMeleeRange(target))
                AddSpellToCast(me, SPELL_RESONANCE);

            Resonance_Timer = 5000;
        }
        

        if (HeroicMode)
        {
            // Thundering Storm cast to all which are too far away
            if (ThunderingStorm_Timer.Expired(diff))
            {
                std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
                for(std::list<HostileReference*>::iterator i = m_threatlist.begin(); i != m_threatlist.end(); ++i)
                {
                    if (Unit* target = Unit::GetUnit((*me),(*i)->getUnitGuid()))
                    {
                        if (target->isAlive() && !target->IsWithinDistInMap(me, 12.0f))
                            ForceSpellCast(target, SPELL_THUNDERING_STORM, DONT_INTERRUPT);
                    }
                }

                ThunderingStorm_Timer = 7500;
            }


            // Sonic Shock cast to tank if someone is too far away
            if (SonicShock_Timer.Expired(diff))
            {
                std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
                for(std::list<HostileReference*>::iterator i = m_threatlist.begin(); i != m_threatlist.end(); ++i)
                {
                    if (Unit* target = Unit::GetUnit((*me),(*i)->getUnitGuid()))
                    {
                        if (target->isAlive() && !target->IsWithinDistInMap(me, 12.0f))
                        {
                            AddSpellToCast(SPELL_SONIC_SHOCK, CAST_TANK);
                            break;
                        }
                    }
                }

                SonicShock_Timer = 2000;
            }
        }

        if (!HeroicMode)
        {
            // Magnetic Pull normal only
            if (MagneticPull_Timer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                    ForceSpellCast(target, SPELL_MAGNETIC_PULL);

                 MagneticPull_Timer = 40000;
            }
        }

        // Sonic Boom
        if (SonicBoom_Timer.Expired(diff))
        {
            ForceSpellCastWithScriptText(me, SPELL_SONIC_BOOM_CAST, EMOTE_SONIC_BOOM);
            SonicBoom_Timer = (HeroicMode ? 30000 : 40000);
            Resonance_Timer = 5000;
            ThunderingStorm_Timer = 5000;
            SonicShock_Timer = 5000;
        }
        

        CastNextSpellIfAnyAndReady();

        // Select nearest most aggro target if top aggro too far
        if (!me->isAttackReady())
            return;

        if (!me->IsWithinMeleeRange(me->GetVictim()))
        {
            std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
            for(std::list<HostileReference*>::iterator i = m_threatlist.begin(); i != m_threatlist.end(); ++i)
            {
                if (Unit* target = Unit::GetUnit((*me),(*i)->getUnitGuid()))
                {
                    if (target->isAlive() && me->IsWithinMeleeRange(target))
                    {
                        me->TauntApply(target);
                        break;
                    }
                }
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_murmur(Creature *_Creature)
{
    return new boss_murmurAI (_Creature);
}

void AddSC_boss_murmur()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_murmur";
    newscript->GetAI = &GetAI_boss_murmur;
    newscript->RegisterSelf();
}

