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
SDName: Boss_Void_Reaver
SD%Complete: 99
SDComment:
SDCategory: Tempest Keep, The Eye
EndScriptData */

#include "precompiled.h"
#include "def_the_eye.h"

#define SAY_AGGRO                   -1550000
#define SAY_SLAY1                   -1550001
#define SAY_SLAY2                   -1550002
#define SAY_SLAY3                   -1550003
#define SAY_DEATH                   -1550004
#define SAY_POUNDING1               -1550005
#define SAY_POUNDING2               -1550006

#define SPELL_POUNDING              34162
#define SPELL_ARCANE_ORB            34172
#define SPELL_KNOCK_AWAY            25778
#define SPELL_BERSERK               27680

#define TRIGGER                     29530

struct boss_void_reaverAI : public ScriptedAI
{
    boss_void_reaverAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        m_creature->GetPosition(wLoc);
    }

    ScriptedInstance* pInstance;

    Timer_UnCheked Pounding_Timer;
    Timer_UnCheked ArcaneOrb_Timer;
    Timer_UnCheked KnockAway_Timer;
    Timer_UnCheked Berserk_Timer;
    Timer_UnCheked Check_Timer;

    WorldLocation wLoc;

    bool Enraged;

    void Reset()
    {
        Pounding_Timer.Reset(15000);
        ArcaneOrb_Timer.Reset(3000);
        KnockAway_Timer.Reset(30000);
        Berserk_Timer.Reset(600000);
        Check_Timer.Reset(3000);
        m_creature->ApplySpellImmune(2, IMMUNITY_EFFECT, SPELL_EFFECT_HEALTH_LEECH, true);
        m_creature->ApplySpellImmune(3, IMMUNITY_STATE, SPELL_AURA_PERIODIC_LEECH, true);
        m_creature->ApplySpellImmune(4, IMMUNITY_STATE, SPELL_AURA_PERIODIC_MANA_LEECH, true);

        pInstance->SetData(DATA_VOIDREAVEREVENT, NOT_STARTED);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2, SAY_SLAY3), m_creature);
    }

    void JustDied(Unit *Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        pInstance->SetData(DATA_VOIDREAVEREVENT, DONE);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        DoZoneInCombat();

        pInstance->SetData(DATA_VOIDREAVEREVENT, IN_PROGRESS);
    }

    void SpellHitTarget(Unit *target, SpellEntry *spell)
    {
        if (spell->Id == SPELL_KNOCK_AWAY)
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_TOPAGGRO, 0))
               AttackStart(target, true);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() )
            return;

        //Check_Timer
        if (Check_Timer.Expired(diff))
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 135.0f))
                EnterEvadeMode();
            else
                DoZoneInCombat();

            Check_Timer = 3000;
        }

        // Pounding
        if (Pounding_Timer.Expired(diff))
        {
            AddSpellToCastWithScriptText(m_creature, SPELL_POUNDING, RAND(SAY_POUNDING1, SAY_POUNDING2));

            if (KnockAway_Timer.GetTimeLeft() < 3100)
                KnockAway_Timer.Reset(3100);

            Pounding_Timer = 12000;
        }

        // Arcane Orb
        if (ArcaneOrb_Timer.Expired(diff))
        {
            Unit * target = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true, 0, 18.0f);

            if (!target)
                target = m_creature->GetVictim();

            if (target)
              if (Creature* t = DoSpawnCreature(TRIGGER, 0.0f, 0.0f, 10.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 40000))
                 t->CastSpell(target, SPELL_ARCANE_ORB, false, 0, 0, m_creature->GetGUID());

            ArcaneOrb_Timer = urand(3000, 4000);
        }

        // Single Target knock back, reduces aggro
        if (KnockAway_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_KNOCK_AWAY);
            KnockAway_Timer = 30000;
        }

        //Berserk
        if (Berserk_Timer.Expired(diff))
        {
            ForceSpellCast(m_creature, SPELL_BERSERK);
            Berserk_Timer = 600000;
        }

        m_creature->RemoveAurasWithDispelType(DISPEL_POISON);
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_void_reaver(Creature *_Creature)
{
    return new boss_void_reaverAI (_Creature);
}

void AddSC_boss_void_reaver()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_void_reaver";
    newscript->GetAI = &GetAI_boss_void_reaver;
    newscript->RegisterSelf();
}
