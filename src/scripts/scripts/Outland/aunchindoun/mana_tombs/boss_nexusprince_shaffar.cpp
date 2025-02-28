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
SDName: Boss_NexusPrince_Shaffar
SD%Complete: 80
SDComment: Need more tuning of spell timers, it should not be as linear fight as current. Also should possibly find a better way to deal with his three initial beacons to make sure all aggro.
SDCategory: Auchindoun, Mana Tombs
EndScriptData */

/* ContentData
boss_nexusprince_shaffar
mob_ethereal_beacon
EndContentData */

#include "precompiled.h"
#include "def_mana_tombs.h"

#define SAY_INTRO                       -1557000

#define SAY_AGGRO_1                     -1557001
#define SAY_AGGRO_2                     -1557002
#define SAY_AGGRO_3                     -1557003

#define SAY_SLAY_1                      -1557004
#define SAY_SLAY_2                      -1557005

#define SAY_SUMMON                      -1557006

#define SAY_DEAD                        -1557007

#define SPELL_BLINK                     34605
#define SPELL_FROSTBOLT                 32370
#define SPELL_FIREBALL                  20420
#define SPELL_FROSTNOVA                 32365

#define SPELL_ETHEREAL_BEACON           32371               // Summon 18431
#define SPELL_ETHEREAL_BEACON_VISUAL    32368

#define ENTRY_BEACON                    18431
#define ENTRY_SHAFFAR                   18344

struct boss_nexusprince_shaffarAI : public ScriptedAI
{
    boss_nexusprince_shaffarAI(Creature *c) : ScriptedAI(c), Summons(c)
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    Timer Blink_Timer;
    Timer Beacon_Timer;
    Timer FireBall_Timer;
    Timer Frostbolt_Timer;
    Timer FrostNova_Timer;

    bool HasTaunted;
    bool CanBlink;

    SummonList Summons;

    void Reset() override
    {
        Blink_Timer.Reset(1500);
        Beacon_Timer.Reset(10000);
        FireBall_Timer.Reset(8000);
        Frostbolt_Timer.Reset(4000);
        FrostNova_Timer.Reset(15000);

        HasTaunted = false;
        CanBlink = false;

        if (Summons.isEmpty())
            SummonInitialBeacons();

        if(pInstance)
            pInstance->SetData(DATA_NEXUSPRINCEEVENT, NOT_STARTED);
    }

    void MoveInLineOfSight(Unit *who) override
    {
        if( !m_creature->GetVictim() && who->isTargetableForAttack() && ( m_creature->IsHostileTo( who )) && who->isInAccessiblePlacefor(m_creature) )
        {
            if( !HasTaunted && m_creature->IsWithinDistInMap(who, 100.0) )
            {
                DoScriptText(SAY_INTRO, m_creature);
                HasTaunted = true;
            }

            if (!m_creature->CanFly() && m_creature->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                return;

            float attackRadius = m_creature->GetAttackDistance(who);
            if( m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->IsWithinLOSInMap(who) )
            {
                //who->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
                AttackStart(who);
            }
        }
    }

    void EnterCombat(Unit *who) override
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), m_creature);

        for (SummonList::const_iterator i = Summons.begin(); i != Summons.end(); ++i)
            if (Creature *beacon = m_creature->GetCreature(*i))
                beacon->AI()->AttackStart(who);

        if(pInstance)
            pInstance->SetData(DATA_NEXUSPRINCEEVENT, IN_PROGRESS);
    }

    void JustSummoned(Creature *summoned) override
    {
        Summons.Summon(summoned);
        if (m_creature->IsInCombat() && summoned->GetEntry() == ENTRY_BEACON)
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 60, true))
                summoned->AI()->AttackStart(target);
    }

    void KilledUnit(Unit* victim) override
    {
        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), m_creature);
    }

    void JustDied(Unit* Killer) override
    {
        DoScriptText(SAY_DEAD, m_creature);
        Summons.DespawnAll();

        if(pInstance)
            pInstance->SetData(DATA_NEXUSPRINCEEVENT, DONE);
    }

    void JustReachedHome() override {
        Summons.DespawnAll();
        SummonInitialBeacons();
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (FrostNova_Timer.Expired(diff))
        {
            if( m_creature->IsNonMeleeSpellCast(false) )
                m_creature->InterruptNonMeleeSpells(true);

            DoCast(m_creature,SPELL_FROSTNOVA);
            FrostNova_Timer  = 17500 + rand()%7500;
            CanBlink = true;
        }
        

        if (Frostbolt_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FROSTBOLT);
            Frostbolt_Timer = 4500 + rand()%1500;
        }
        

        if (FireBall_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FIREBALL);
            FireBall_Timer = 4500 + rand()%1500;
        }
        

        if( CanBlink )
        {
            if (Blink_Timer.Expired(diff))
            {
                if( m_creature->IsNonMeleeSpellCast(false) )
                    m_creature->InterruptNonMeleeSpells(true);

                DoCast(m_creature,SPELL_BLINK);
                Blink_Timer = 1000 + rand()%1500;
                CanBlink = false;
            }
            
        }

        if (Beacon_Timer.Expired(diff))
        {
            if( m_creature->IsNonMeleeSpellCast(false) )
                m_creature->InterruptNonMeleeSpells(true);

            if( !urand(0,3) )
                DoScriptText(SAY_SUMMON, m_creature);

            DoCast(m_creature, SPELL_ETHEREAL_BEACON);

            Beacon_Timer = 10000;
        }
        

        DoMeleeAttackIfReady();
    }

    void SummonInitialBeacons()
    {
        float dist = 8.0f;
        float posX, posY, posZ, angle;
        m_creature->GetHomePosition(posX, posY, posZ, angle);
        m_creature->SummonCreature(ENTRY_BEACON, posX - dist, posY - dist, posZ, angle, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 7200000);
        m_creature->SummonCreature(ENTRY_BEACON, posX - dist, posY + dist, posZ, angle, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 7200000);
        m_creature->SummonCreature(ENTRY_BEACON, posX + dist, posY, posZ, angle, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 7200000);
    }
};

CreatureAI* GetAI_boss_nexusprince_shaffar(Creature *_Creature)
{
    return new boss_nexusprince_shaffarAI (_Creature);
}

#define SPELL_ARCANE_BOLT               15254
#define SPELL_ETHEREAL_APPRENTICE       32372               // Summon 18430

struct mob_ethereal_beaconAI : public ScriptedAI
{
    mob_ethereal_beaconAI(Creature *c) : ScriptedAI(c)
    {
        HeroicMode = m_creature->GetMap()->IsHeroic();
    }

    bool HeroicMode;
    Timer_UnCheked Apprentice_Timer;
    Timer_UnCheked ArcaneBolt_Timer;

    void Reset() override
    {
        m_creature->CastSpell(m_creature, SPELL_ETHEREAL_BEACON_VISUAL, false);
        Apprentice_Timer.Reset((HeroicMode ? 10000 : 20000));
        ArcaneBolt_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who) override
    {
        Unit *shaffar = FindCreature(ENTRY_SHAFFAR, 100, me);
        if (shaffar && !shaffar->IsInCombat())
            shaffar->ToCreature()->AI()->AttackStart(who);
    }

    void JustSummoned(Creature *summoned) override
    {
        if (m_creature->IsInCombat())
            summoned->AI()->AttackStart(m_creature->GetVictim());
    }

    void UpdateAI(const uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (ArcaneBolt_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_ARCANE_BOLT);
            ArcaneBolt_Timer = 2000 + rand() % 2500;
        }

        if (Apprentice_Timer.Expired(diff))
        {
            if (m_creature->IsNonMeleeSpellCast(false))
                m_creature->InterruptNonMeleeSpells(true);

            DoCast(m_creature, SPELL_ETHEREAL_APPRENTICE, true);
            // Wait SPELL_ETHEREAL_APPRENTICE visual (credits CoreCraft)
            m_creature->ForcedDespawn(2000);
            Apprentice_Timer.Reset(0);
        }
    }
};

CreatureAI* GetAI_mob_ethereal_beacon(Creature *_Creature)
{
    return new mob_ethereal_beaconAI (_Creature);
}

#define SPELL_ETHEREAL_APPRENTICE_FIREBOLT          32369
#define SPELL_ETHEREAL_APPRENTICE_FROSTBOLT         32370

struct mob_ethereal_apprenticeAI : public ScriptedAI
{
    mob_ethereal_apprenticeAI(Creature *c) : ScriptedAI(c)
    {
        isFireboltTurn = true;
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    Timer_UnCheked Cast_Timer;

    bool isFireboltTurn;

    void Reset() override
    {
        Cast_Timer.Reset(3000);
        isFireboltTurn = true;
    }

    void JustReachedHome() override
    {
        m_creature->ForcedDespawn(1000);
    }

    void UpdateAI(const uint32 diff) override
    {
        if(!UpdateVictim())
            return;

        if (Cast_Timer.Expired(diff))
        {
            if(isFireboltTurn)
            {
                m_creature->CastSpell(m_creature->GetVictim(), SPELL_ETHEREAL_APPRENTICE_FIREBOLT, true);
                isFireboltTurn = false;
            }
            else
            {
                m_creature->CastSpell(m_creature->GetVictim(), SPELL_ETHEREAL_APPRENTICE_FROSTBOLT, true);
                isFireboltTurn = true;
            }
            Cast_Timer = 3000;
        }

        if (pInstance->GetData(DATA_NEXUSPRINCEEVENT) == DONE)
            m_creature->ForcedDespawn(1000);

    }
};

CreatureAI* GetAI_mob_ethereal_apprentice(Creature *_Creature)
{
    return new mob_ethereal_apprenticeAI (_Creature);
}

void AddSC_boss_nexusprince_shaffar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_nexusprince_shaffar";
    newscript->GetAI = &GetAI_boss_nexusprince_shaffar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_ethereal_beacon";
    newscript->GetAI = &GetAI_mob_ethereal_beacon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_ethereal_apprentice";
    newscript->GetAI = &GetAI_mob_ethereal_apprentice;
    newscript->RegisterSelf();
}

