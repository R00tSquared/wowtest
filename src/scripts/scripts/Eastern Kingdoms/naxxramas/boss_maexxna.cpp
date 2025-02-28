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
SDName: Boss_Maexxna
SD%Complete: 90
SDComment: Maybe some improvements to Web Wrap, debugging
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum
{
    EMOTE_SPIN_WEB              = -1533146,
    EMOTE_SPIDERLING            = -1533147,
    EMOTE_SPRAY                 = -1533148,
    EMOTE_BOSS_GENERIC_FRENZY   = -1000384,

    SPELL_WEBWRAP               = 28622,
    SPELL_WEBWRAP_2             = 28673,                    // purpose unknown

    SPELL_WEBSPRAY              = 29484,
    SPELL_POISONSHOCK           = 28741,
    SPELL_NECROTICPOISON        = 28776,
    SPELL_FRENZY                = 28747,

    NPC_WEB_WRAP                = 16486,
    NPC_SPIDERLING              = 17055,

    MAX_SPIDERLINGS             = 8,
    MAX_WEB_WRAP_POSITIONS      = 3,
};

static const float aWebWrapLoc[MAX_WEB_WRAP_POSITIONS][3] =
{
    {3546.796f, -3869.082f, 296.450f},
    {3531.271f, -3847.424f, 299.450f},
    {3497.067f, -3843.384f, 302.384f}
};

struct npc_web_wrapAI : public ScriptedAI
{
    npc_web_wrapAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint64 victimGuid;
    uint32 WebWrapTimer;

    void Reset()
    {
        WebWrapTimer = 0;
    }

    void MoveInLineOfSight(Unit* /*pWho*/) {}
    void AttackStart(Unit* /*pWho*/) {}

    void SetVictim(Unit* pVictim)
    {
        if (pVictim && pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            float dist = m_creature->GetDistance2d(pVictim);
            // Switch the speed multiplier based on the distance from the web wrap
            uint32 EffectMiscValue = 500;
            if (dist < 25.0f)
                EffectMiscValue = 200;
            else if (dist < 50.0f)
                EffectMiscValue = 300;
            else if (dist < 75.0f)
                EffectMiscValue = 400;

            // This doesn't give the expected result in all cases
            ((Player*)pVictim)->KnockBackFrom(m_creature, -dist, 10.0);

            victimGuid = pVictim->GetGUID();
            WebWrapTimer = EffectMiscValue == 200 ? 1500 : 2500;
            
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,11686);  // Invisible Model
        }
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (victimGuid)
        {
            if (Player* pVictim = Unit::GetPlayerInWorld(victimGuid))
            {
                if (pVictim->isAlive())
                    pVictim->RemoveAurasDueToSpell(SPELL_WEBWRAP);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (WebWrapTimer)
        {
            // Finally the player gets web wrapped and he should change the display id until the creature is killed
            if (WebWrapTimer <= diff)
            {
                if (Player* pVictim = Unit::GetPlayerInWorld(victimGuid))
                {
                    if(m_creature->GetDistance2d(pVictim) > 2.0f)
                        pVictim->TeleportTo(m_creature->GetMapId(), m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), pVictim->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT);

                    pVictim->CastSpell(pVictim, SPELL_WEBWRAP, true, NULL, NULL, m_creature->GetGUID());
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID,16213); // Web Wrap Model
                }
                WebWrapTimer = 0;
            }
            else
                WebWrapTimer -= diff;
        }
    }
};

struct boss_maexxnaAI : public ScriptedAI
{
    boss_maexxnaAI(Creature* pCreature) : ScriptedAI(pCreature), summons(pCreature)
    {
        pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        m_creature->GetPosition(wLoc);
        Reset();
    }

    instance_naxxramas* pInstance;
    SummonList summons;
    WorldLocation wLoc;

    uint32 WebWrapTimer;
    uint32 CheckTimer;
    uint32 WebSprayTimer;
    uint32 PoisonShockTimer;
    uint32 NecroticPoisonTimer;
    uint32 SummonSpiderlingTimer;
    bool   Enraged;

    void Reset()
    {
        WebWrapTimer            = 15000;
        WebSprayTimer           = 40000;
        PoisonShockTimer        = urand(10000, 20000);
        NecroticPoisonTimer     = urand(20000, 30000);
        SummonSpiderlingTimer   = 30000;
        CheckTimer              = 3000;
        Enraged                 = false;
        summons.DespawnAll();
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        if (pInstance)
            pInstance->SetData(DATA_MAEXXNA, IN_PROGRESS);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (pInstance)
            pInstance->SetData(DATA_MAEXXNA, DONE);
        summons.DespawnAll();
        // Spawn Teleport to the Center of instance
        if (Player* objPlr = pInstance->GetPlayer())
        {
            if(Creature *TeleportTrigger = objPlr->SummonTrigger(3467.46, -3936.25, 304.41, 0.93, 0, nullptr, true))
                TeleportTrigger->SummonGameObject(GO_TELEPORT_NAX_WORKING, 3467.46, -3936.25, 304.41, 0.93, 0, 0, 0, 0, 0);
        }
    }

    void JustReachedHome()
    {
        if (pInstance)
            pInstance->SetData(DATA_MAEXXNA, FAIL);
    }

    void JustSummoned(Creature* pSummoned)
    {
        summons.Summon(pSummoned);
        if (pSummoned->GetEntry() == NPC_WEB_WRAP)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 1, SPELL_WEBWRAP, true))
            {
                if (npc_web_wrapAI* pWebAI = dynamic_cast<npc_web_wrapAI*>(pSummoned->AI()))
                    pWebAI->SetVictim(pTarget);
            }
            pSummoned->setFaction(me->getFaction());
        }
        else if (pSummoned->GetEntry() == NPC_SPIDERLING)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(pTarget);
        }
    }

    bool DoCastWebWrap()
    {
        // If we can't select a player for web wrap then skip the summoning
        Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1, uint32(0), true);
        if(!target)
            return false;

        uint8 uiPos1 = urand(0, MAX_WEB_WRAP_POSITIONS - 1);
        m_creature->SummonCreature(NPC_WEB_WRAP, aWebWrapLoc[uiPos1][0], aWebWrapLoc[uiPos1][1], aWebWrapLoc[uiPos1][2], 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
        
        uint8 uiPos2 = (uiPos1 + urand(1, MAX_WEB_WRAP_POSITIONS - 1)) % MAX_WEB_WRAP_POSITIONS;
        m_creature->SummonCreature(NPC_WEB_WRAP, aWebWrapLoc[uiPos2][0], aWebWrapLoc[uiPos2][1], aWebWrapLoc[uiPos2][2], 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);

        return true;
    }

    // Summons spiderlings around the boss
    void SummonSpiderlings()
    {
        for (uint8 i = 0; i < MAX_SPIDERLINGS; ++i)
            m_creature->SummonCreature(NPC_SPIDERLING, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CheckTimer < diff)
        {
            if (!m_creature->IsWithinDistInMap(&wLoc, 80.0f))
                EnterEvadeMode();
            else
                DoZoneInCombat();

            CheckTimer = 3000;
        }
        else
            CheckTimer -= diff;

        // Web Wrap
        if (WebWrapTimer < diff)
        {
            if (DoCastWebWrap())
                DoScriptText(EMOTE_SPIN_WEB, m_creature);
            WebWrapTimer = 40000;
        }
        else
            WebWrapTimer -= diff;

        // Web Spray
        if (WebSprayTimer < diff)
        {
            AddSpellToCast(SPELL_WEBSPRAY, CAST_NULL);
            DoScriptText(EMOTE_SPRAY, m_creature);
            WebSprayTimer = 40000;
        }
        else
            WebSprayTimer -= diff;

        // Poison Shock
        if (PoisonShockTimer < diff)
        {
            AddSpellToCast(SPELL_POISONSHOCK, CAST_NULL);
            PoisonShockTimer = urand(10000, 20000);
        }
        else
            PoisonShockTimer -= diff;

        // Necrotic Poison
        if (NecroticPoisonTimer < diff)
        {
            AddSpellToCast(SPELL_NECROTICPOISON, CAST_TANK);
            NecroticPoisonTimer = urand(20000, 30000);
        }
        else
            NecroticPoisonTimer -= diff;

        // Summon Spiderling
        if (SummonSpiderlingTimer < diff)
        {
            SummonSpiderlings();
            DoScriptText(EMOTE_SPIDERLING, m_creature);
            SummonSpiderlingTimer = 30000;
        }
        else
            SummonSpiderlingTimer -= diff;

        // Enrage if not already enraged and below 30%
        if (!Enraged && HealthBelowPct(30))
        {
            AddSpellToCast(SPELL_FRENZY, CAST_SELF);
            DoScriptText(EMOTE_BOSS_GENERIC_FRENZY, m_creature);
            Enraged = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_webwrap(Creature* _Creature)
{
    return new npc_web_wrapAI (_Creature);
}

CreatureAI* GetAI_boss_maexxna(Creature *_Creature)
{
    return new boss_maexxnaAI (_Creature);
}

void AddSC_boss_maexxna()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_maexxna";
    newscript->GetAI = &GetAI_boss_maexxna;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_webwrap";
    newscript->GetAI = &GetAI_mob_webwrap;
    newscript->RegisterSelf();
}
