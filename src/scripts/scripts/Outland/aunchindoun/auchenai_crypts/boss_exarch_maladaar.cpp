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
SDName: Boss_Exarch_Maladaar
SD%Complete: 95
SDComment: Most of event implemented, some adjustments to timers remain and possibly make some better code for switching his dark side in to better "images" of player.
SDCategory: Auchindoun, Auchenai Crypts
EndScriptData */

/* ContentData
mob_stolen_soul
boss_exarch_maladaar
mob_avatar_of_martyred
EndContentData */

#include "precompiled.h"
#include "def_auchenai_crypts.h"

#define SPELL_MOONFIRE          37328
#define SPELL_FIREBALL          37329
#define SPELL_MIND_FLAY         37330
#define SPELL_HEMORRHAGE        37331
#define SPELL_FROSTSHOCK        37332
#define SPELL_CURSE_OF_AGONY    37334
#define SPELL_MORTAL_STRIKE     37335
#define SPELL_FREEZING_TRAP     37368
#define SPELL_HAMMER_OF_JUSTICE 37369

struct mob_stolen_soulAI : public ScriptedAI
{
    mob_stolen_soulAI(Creature *c) : ScriptedAI(c) {}

    uint8 myClass;
    Timer_UnCheked Class_Timer;

    void Reset()
    {
        Class_Timer.Reset(1000);
    }

    void EnterCombat(Unit *who)
    { }

    void SetMyClass(uint8 myclass)
    {
        myClass = myclass;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (Class_Timer.Expired(diff))
        {
            switch (myClass)
            {
                case CLASS_WARRIOR:
                    DoCast(m_creature->GetVictim(), SPELL_MORTAL_STRIKE);
                    Class_Timer = 6000;
                    break;
                case CLASS_PALADIN:
                    DoCast(m_creature->GetVictim(), SPELL_HAMMER_OF_JUSTICE);
                    Class_Timer = 6000;
                    break;
                case CLASS_HUNTER:
                    DoCast(m_creature->GetVictim(), SPELL_FREEZING_TRAP);
                    Class_Timer = 20000;
                    break;
                case CLASS_ROGUE:
                    DoCast(m_creature->GetVictim(), SPELL_HEMORRHAGE);
                    Class_Timer = 10000;
                    break;
                case CLASS_PRIEST:
                    DoCast(m_creature->GetVictim(), SPELL_MIND_FLAY);
                    Class_Timer = 5000;
                    break;
                case CLASS_SHAMAN:
                    DoCast(m_creature->GetVictim(), SPELL_FROSTSHOCK);
                    Class_Timer = 8000;
                    break;
                case CLASS_MAGE:
                    DoCast(m_creature->GetVictim(), SPELL_FIREBALL);
                    Class_Timer = 5000;
                    break;
                case CLASS_WARLOCK:
                    DoCast(m_creature->GetVictim(), SPELL_CURSE_OF_AGONY);
                    Class_Timer = 20000;
                    break;
                case CLASS_DRUID:
                    DoCast(m_creature->GetVictim(), SPELL_MOONFIRE);
                    Class_Timer = 10000;
                    break;
            }
        } 

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_stolen_soul(Creature *_Creature)
{
    return new mob_stolen_soulAI (_Creature);
}

#define SAY_INTRO                   -1558000
#define SAY_SUMMON                  -1558001

#define SAY_AGGRO_1                 -1558002
#define SAY_AGGRO_2                 -1558003
#define SAY_AGGRO_3                 -1558004

#define SAY_ROAR                    -1558005
#define SAY_SOUL_CLEAVE             -1558006

#define SAY_SLAY_1                  -1558007
#define SAY_SLAY_2                  -1558008

#define SAY_DEATH                   -1558009

#define SPELL_RIBBON_OF_SOULS       32422
#define SPELL_SOUL_SCREAM           32421

#define SPELL_STOLEN_SOUL           32346
#define SPELL_STOLEN_SOUL_VISUAL    32395

#define SPELL_SUMMON_AVATAR         32424

#define ENTRY_STOLEN_SOUL           18441

struct boss_exarch_maladaarAI : public ScriptedAI
{
    boss_exarch_maladaarAI(Creature *c) : ScriptedAI(c), summons(me)
    {
        HasTaunted = false;
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    SummonList summons;

    uint32 soulmodel;
    uint64 soulholder;
    uint8 soulclass;

    Timer_UnCheked Fear_timer;
    Timer_UnCheked Ribbon_of_Souls_timer;
    Timer_UnCheked StolenSoul_Timer;

    bool HasTaunted;
    bool Avatar_summoned;

    void Reset()
    {
        soulmodel = 0;
        soulholder = 0;
        soulclass = 0;

        Fear_timer.Reset(15000 + rand() % 5000);
        Ribbon_of_Souls_timer.Reset(5000);
        StolenSoul_Timer = 0;

        Avatar_summoned = false;
        summons.DespawnAll();
        if(pInstance)
            pInstance->SetData(TYPE_EXARACH_MALADAAR, NOT_STARTED);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!HasTaunted && m_creature->IsWithinDistInMap(who, 150.0))
        {
            DoScriptText(SAY_INTRO, m_creature);
            HasTaunted = true;
        }

        ScriptedAI::MoveInLineOfSight(who);
    }


    void EnterCombat(Unit *who)
    {
        DoScriptText(RAND(SAY_AGGRO_1, SAY_AGGRO_2, SAY_AGGRO_3), m_creature);
        if(pInstance)
            pInstance->SetData(TYPE_EXARACH_MALADAAR, IN_PROGRESS);
    }

    void JustSummoned(Creature *summoned)
    {
        summons.Summon(summoned);
        if (summoned->GetEntry() == ENTRY_STOLEN_SOUL)
        {
            //SPELL_STOLEN_SOUL_VISUAL has shapeshift effect, but not implemented feature in Trinity for this spell.
            summoned->CastSpell(summoned,SPELL_STOLEN_SOUL_VISUAL,false);
            summoned->SetDisplayId(soulmodel);
            summoned->setFaction(m_creature->getFaction());

            if (Unit *target = Unit::GetUnit(*m_creature,soulholder))
            {

            ((mob_stolen_soulAI*)summoned->AI())->SetMyClass(soulclass);
             summoned->AI()->AttackStart(target);
            }
        }
    }

    void KilledUnit(Unit* victim)
    {
        if (rand()%2)
            return;

        DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), m_creature);
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);
        //When Exarch Maladar is defeated D'ore appear.
        DoSpawnCreature(19412,0,0,0,0, TEMPSUMMON_TIMED_DESPAWN, 600000);
        if(pInstance)
            pInstance->SetData(TYPE_EXARACH_MALADAAR, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!Avatar_summoned && (me->GetHealthPercent() < 25))
        {
            if (m_creature->IsNonMeleeSpellCast(false))
                m_creature->InterruptNonMeleeSpells(true);

            DoScriptText(SAY_SUMMON, m_creature);

            DoCast(m_creature, SPELL_SUMMON_AVATAR);
            Avatar_summoned = true;
            StolenSoul_Timer = 15000 + rand() % 15000;
        }

        if (StolenSoul_Timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 100, true))
            {
                if (target->GetTypeId() == TYPEID_PLAYER)
                {
                    if (m_creature->IsNonMeleeSpellCast(false))
                        m_creature->InterruptNonMeleeSpells(true);

                    DoScriptText(rand() % 2 ? SAY_ROAR : SAY_SOUL_CLEAVE, m_creature);

                    soulmodel = target->GetDisplayId();
                    soulholder = target->GetGUID();
                    soulclass = target->GetClass();

                    DoCast(target,SPELL_STOLEN_SOUL);
                    DoSpawnCreature(ENTRY_STOLEN_SOUL,0,0,0,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,10000);

                    StolenSoul_Timer = 20000 + rand()% 10000;
                } 
            }
        }

        if (Ribbon_of_Souls_timer.Expired(diff))
        {
            if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0, 100, true))
                DoCast(target,SPELL_RIBBON_OF_SOULS);

            Ribbon_of_Souls_timer = 5000 + (rand()%20 * 1000);
        }

        if (Fear_timer.Expired(diff))
        {
            DoCast(m_creature,SPELL_SOUL_SCREAM);
            Fear_timer = 15000 + rand()% 15000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_exarch_maladaar(Creature *_Creature)
{
    return new boss_exarch_maladaarAI (_Creature);
}

enum AvatarOfMartyred
{
    SPELL_AV_MORTAL_STRIKE  = 16856, // before patch 2.1 it was 300% (spell), after 200%. - 300% spell is 15708
    SPELL_SUNDER_ARMOR      = 16145
};


struct mob_avatar_of_martyredAI : public ScriptedAI
{
    mob_avatar_of_martyredAI(Creature *c) : ScriptedAI(c) {}

    Timer MortalStrike_Timer;
    Timer SunderArmor_Timer;

    void Reset()
    {
        MortalStrike_Timer.Reset(5000);
        SunderArmor_Timer.Reset(urand(7000, 10000));
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (MortalStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_AV_MORTAL_STRIKE);
            MortalStrike_Timer = urand(10000, 20000);
        }

        if (SunderArmor_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SUNDER_ARMOR);
            SunderArmor_Timer = urand(7000, 10000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_avatar_of_martyred(Creature *_Creature)
{
    return new mob_avatar_of_martyredAI (_Creature);
}

void AddSC_boss_exarch_maladaar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_exarch_maladaar";
    newscript->GetAI = &GetAI_boss_exarch_maladaar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_avatar_of_martyred";
    newscript->GetAI = &GetAI_mob_avatar_of_martyred;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_stolen_soul";
    newscript->GetAI = &GetAI_mob_stolen_soul;
    newscript->RegisterSelf();
}

