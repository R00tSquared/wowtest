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
SDName: Boss_Skeram
SD%Complete: 90
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_temple_of_ahnqiraj.h"
#include "Group.h"

#define SAY_AGGRO1                  -1531000
#define SAY_AGGRO2                  -1531001
#define SAY_AGGRO3                  -1531002
#define SAY_SLAY1                   -1531003
#define SAY_SLAY2                   -1531004
#define SAY_SLAY3                   -1531005
#define SAY_SPLIT                   -1531006
#define SAY_DEATH                   -1531007

#define SPELL_ARCANE_EXPLOSION      26192
#define SPELL_EARTH_SHOCK           26194
#define SPELL_TRUE_FULFILLMENT      785
#define SPELL_BLINK                 28391
#define SPELL_TELEPORT_1            4801
#define SPELL_TELEPORT_2            8195
#define SPELL_TELEPORT_3            20449
#define SPELL_INITIALIZE_IMAGE      3730
#define SPELL_SUMMON_IMAGES         747

struct boss_skeramAI : public ScriptedAI
{
    boss_skeramAI(Creature *c) : ScriptedAI(c), summons(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance *pInstance;
    SummonList summons;

    uint32 ArcaneExplosion_Timer;
    uint32 FullFillment_Timer;
    uint32 EarthShock_Timer;
    uint32 Blink_Timer;

    float fHpCheck;

    bool IsImage;

    void Reset()
    {
        if (me->IsTemporarySummon())
            IsImage = true;
        else
            IsImage = false;

        ArcaneExplosion_Timer = urand(6000, 12000);
        FullFillment_Timer = 15000;
        if(IsImage)
            Blink_Timer = 1000;
        else
            Blink_Timer = urand(30000, 45000);
        EarthShock_Timer = 3000;

        fHpCheck = 75.0f;

        if (me->GetVisibility() != VISIBILITY_ON)
            me->SetVisibility(VISIBILITY_ON);

        if (pInstance)
            pInstance->SetData(DATA_THE_PROPHET_SKERAM, NOT_STARTED);

        summons.DespawnAll();
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2, SAY_SLAY3), me);
    }

    void JustDied(Unit* Killer)
    {
        if (!IsImage)
        {
            DoScriptText(SAY_DEATH, me);
            if (pInstance)
                pInstance->SetData(DATA_THE_PROPHET_SKERAM, DONE);
        }
        else
            m_creature->ForcedDespawn(1);
    }

    void EnterCombat(Unit *who)
    {
        if (IsImage)
            return;

        DoZoneInCombat();
        DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);
        if (pInstance)
            pInstance->SetData(DATA_THE_PROPHET_SKERAM, IN_PROGRESS);
    }

    void JustSummoned(Creature* pSummoned)
    {
        summons.Summon(pSummoned);
        if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true))
            pSummoned->AI()->AttackStart(pTarget);

        pSummoned->SetMaxHealth(me->GetMaxHealth() / 10);

        if(fHpCheck <= 75.0f && fHpCheck > 50.0f)
        {
            pSummoned->SetHealth(pSummoned->GetMaxHealth() * 0.75);
            me->SetHealth(me->GetMaxHealth() * 0.75);
        }
        else if(fHpCheck <= 50.0f && fHpCheck > 25.0f)
        {
            pSummoned->SetHealth(pSummoned->GetMaxHealth() * 0.50);
            me->SetHealth(me->GetMaxHealth() * 0.50);
        }
        else if(fHpCheck <= 25.0f)
        {
            pSummoned->SetHealth(pSummoned->GetMaxHealth() * 0.25);
            me->SetHealth(me->GetMaxHealth() * 0.25);
        }
        pSummoned->SetVisibility(VISIBILITY_OFF);
        AddSpellToCast(pSummoned, SPELL_INITIALIZE_IMAGE, true);
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim())
            return;

        //ArcaneExplosion_Timer
        if (ArcaneExplosion_Timer < diff)
        {
            uint32 PlayersInMeleeCounter = 0;
            std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
            std::list<HostileReference*>::iterator itr;

            for(itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
            {
                Unit* pUnit = NULL;
                pUnit = Unit::GetUnit((*m_creature), (*itr)->getUnitGuid());
                if(pUnit)
                    if(pUnit->GetTypeId() == TYPEID_PLAYER)
                        if(me->IsWithinMeleeRange(pUnit))
                            if(PlayersInMeleeCounter <= 4)
                                PlayersInMeleeCounter++;
            }

            if(PlayersInMeleeCounter >= 4)
                AddSpellToCast(me->GetVictim(), SPELL_ARCANE_EXPLOSION);
            ArcaneExplosion_Timer = urand(8000, 18000);
        }else ArcaneExplosion_Timer -= diff;

        // True Fullfilment
        if (FullFillment_Timer < diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_NEAREST, 0, GetSpellMaxRange(SPELL_TRUE_FULFILLMENT), true))
            {
                if(me->getThreatManager().getThreatList().size() >= 2)
                    AddSpellToCast(pTarget, SPELL_TRUE_FULFILLMENT);
                FullFillment_Timer = urand(20000, 30000);
            }
        }
        else
            FullFillment_Timer -= diff;

        //Blink_Timer
        if (Blink_Timer < diff)
        {
            switch(rand()%3)
            {
                case 0:
                    AddSpellToCast(me, SPELL_TELEPORT_1);
                    break;
                case 1:
                    AddSpellToCast(me, SPELL_TELEPORT_2);
                    break;
                case 2:
                    AddSpellToCast(me, SPELL_TELEPORT_3);
                    break;
            }
            DoResetThreat();

            if (me->GetVisibility() != VISIBILITY_ON)
                me->SetVisibility(VISIBILITY_ON);

            Blink_Timer = urand(10000, 30000);
        } else Blink_Timer -= diff;

        //If we are within range melee the target
        if(me->IsWithinMeleeRange(me->GetVictim()))
            DoMeleeAttackIfReady();
        else
        {
            if (!me->IsNonMeleeSpellCast(false))
            {
                //EarthShock_Timer
                if (EarthShock_Timer < diff)
                {
                    AddSpellToCast(m_creature->GetVictim(), SPELL_EARTH_SHOCK);
                    EarthShock_Timer = 1000;
                }else EarthShock_Timer -= diff;
            }
        }

        // Summon images at 75%, 50% and 25%
        if (!IsImage && (me->GetHealthPercent() < fHpCheck))
        {
            // AddSpellToCast(me, SPELL_SUMMON_IMAGES);
            me->SummonCreature(15263, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 1000);
            me->SummonCreature(15263, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 1000);
            DoScriptText(SAY_SPLIT, me);
            fHpCheck -= 25.0f;
            // Teleport shortly after the images are summoned and set invisible to clear the selection (Workaround alert!!!)
            me->SetVisibility(VISIBILITY_OFF);
            Blink_Timer = 1000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_boss_skeram(Creature *_Creature)
{
    return new boss_skeramAI (_Creature);
}

bool EffectDummyCreature_prophet_skeram(Unit* pCaster, uint32 spellId, uint32 effIndex, Creature* pCreatureTarget)
{
    // always check spellid and effectindex
    if (spellId == SPELL_INITIALIZE_IMAGE && effIndex == 0)
    {
        // check for target == caster first
        if (ScriptedInstance *pInstance = (pCaster->GetInstanceData()))
        {
            if (Creature* pProphet = pCaster->GetCreature(pInstance->GetData64(DATA_SKERAM)))
            {
                if (pProphet == pCreatureTarget)
                    return false;
            }
        }

        if (boss_skeramAI* pSkeramAI = dynamic_cast<boss_skeramAI*>(pCreatureTarget->AI()))
            pSkeramAI->Blink_Timer = 2000;
    }

    return false;
}

void AddSC_boss_skeram()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_skeram";
    newscript->GetAI = &GetAI_boss_skeram;
    newscript->pEffectDummyNPC = &EffectDummyCreature_prophet_skeram;
    newscript->RegisterSelf();
}

