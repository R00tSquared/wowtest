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
SDName: Boss_Gothik
SD%Complete: 90
SDComment: Need better support to summons in phase 1
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum
{
    SAY_SPEECH_1                = -1533040,
    SAY_SPEECH_2                = -1533140,
    SAY_SPEECH_3                = -1533141,
    SAY_SPEECH_4                = -1533142,

    SAY_KILL                    = -1533041,
    SAY_DEATH                   = -1533042,
    SAY_TELEPORT                = -1533043,

    EMOTE_TO_FRAY               = -1533138,
    EMOTE_GATE                  = -1533139,

    PHASE_SPEECH                = 0,
    PHASE_BALCONY               = 1,
    PHASE_STOP_SUMMONING        = 2,
    PHASE_TELEPORTING           = 3,
    PHASE_STOP_TELEPORTING      = 4,

    // Right is right side from gothik (eastern)
    SPELL_TELEPORT_RIGHT        = 28025,
    SPELL_TELEPORT_LEFT         = 28026,

    SPELL_HARVESTSOUL           = 28679,
    SPELL_SHADOWBOLT            = 29317,
};

enum eSpellDummy
{
    SPELL_A_TO_ANCHOR_1     = 27892,
    SPELL_B_TO_ANCHOR_1     = 27928,
    SPELL_C_TO_ANCHOR_1     = 27935,

    SPELL_A_TO_ANCHOR_2     = 27893,
    SPELL_B_TO_ANCHOR_2     = 27929,
    SPELL_C_TO_ANCHOR_2     = 27936,

    SPELL_A_TO_SKULL        = 27915,
    SPELL_B_TO_SKULL        = 27931,
    SPELL_C_TO_SKULL        = 27937
};

struct ObjectDistanceOrderReversed : public std::binary_function<const WorldObject, const WorldObject, bool>
{
    const Unit* m_pSource;
    ObjectDistanceOrderReversed(const Unit* pSource) : m_pSource(pSource) {};

    bool operator()(const WorldObject* pLeft, const WorldObject* pRight) const
    {
        return !m_pSource->GetDistanceOrder(pLeft, pRight, false);
    }
};

struct boss_gothikAI : public ScriptedAI
{
    boss_gothikAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (instance_naxxramas*)c->GetInstanceData();
    }

    instance_naxxramas* pInstance;

    std::list<uint64> SummonedAddGuids;
    std::list<uint64> TraineeSummonPosGuids;
    std::list<uint64> DeathKnightSummonPosGuids;
    std::list<uint64> RiderSummonPosGuids;

    uint8 Phase;
    uint8 Speech;

    uint32 TraineeTimer;
    uint32 DeathKnightTimer;
    uint32 RiderTimer;
    uint32 TeleportTimer;
    uint32 ShadowboltTimer;
    uint32 HarvestSoulTimer;
    uint32 PhaseTimer;
    uint32 ControlZoneTimer;
    uint32 SpeechTimer;

    void Reset()
    {
        // Remove immunity
        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);

        Phase = PHASE_SPEECH;
        Speech = 1;

        TraineeTimer = 24000;
        DeathKnightTimer = 74000;
        RiderTimer = 134000;
        TeleportTimer = 20000;
        ShadowboltTimer = 2000;
        HarvestSoulTimer = 2500;
        PhaseTimer = 247000; // last summon at 4:04, next would be 4:09
        ControlZoneTimer = urand(120000, 150000);
        SpeechTimer = 1000;

        // Despawn Adds
        for (std::list<uint64>::const_iterator itr = SummonedAddGuids.begin(); itr != SummonedAddGuids.end(); itr++)
        {
            if (Creature* pCreature = me->GetMap()->GetCreature(*itr))
                pCreature->ForcedDespawn();
        }

        SummonedAddGuids.clear();
        TraineeSummonPosGuids.clear();
        DeathKnightSummonPosGuids.clear();
        RiderSummonPosGuids.clear();

        if (pInstance && pInstance->GetData(DATA_GOTHIK_THE_HARVESTER) != DONE)
            pInstance->SetData(DATA_GOTHIK_THE_HARVESTER, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        if (!pInstance)
            return;

        pInstance->SetData(DATA_GOTHIK_THE_HARVESTER, IN_PROGRESS);
        me->SetRooted(true);

        // Make immune
        me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);

        pInstance->SetGothTriggers();
        PrepareSummonPlaces();
    }

    bool IsCentralDoorClosed()
    {
        return pInstance && pInstance->GetData(DATA_GOTHIK_THE_HARVESTER) != SPECIAL;
    }
    
    void ProcessCentralDoor()
    {
        if (IsCentralDoorClosed())
        {
            pInstance->SetData(DATA_GOTHIK_THE_HARVESTER, SPECIAL);
            DoScriptText(EMOTE_GATE, me);
        }
    }

    bool HasPlayersInLeftSide()
    {
        Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();

        if (PlayerList.isEmpty())
            return false;

        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            if (Player* pPlayer = itr->getSource())
            {
                if (!pInstance->IsInRightSideGothArea(pPlayer) && pPlayer->isAlive())
                    return true;
            }
        }

        return false;
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_KILL, me);
    }

    void JustDied(Unit * killer)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance)
            pInstance->SetData(DATA_GOTHIK_THE_HARVESTER, DONE);
    }

    void JustReachedHome() override
    {
        if (pInstance)
            pInstance->SetData(DATA_GOTHIK_THE_HARVESTER, FAIL);
    }

    void PrepareSummonPlaces()
    {
        std::list<Creature*> SummonList;
        pInstance->GetGothSummonPointCreatures(SummonList, true);
        if (SummonList.empty())
            return;

        // Trainees and Rider
        uint8 index = 0;
        uint8 TraineeCount = 3;
        SummonList.sort(Hellground::ObjectDistanceOrder(me));
        for (std::list<Creature*>::iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
        {
            if (*itr)
            {
                if (TraineeCount == 0)
                    break;

                if (index == 1)
                    RiderSummonPosGuids.push_back((*itr)->GetGUID());
                else
                {
                    TraineeSummonPosGuids.push_back((*itr)->GetGUID());
                    --TraineeCount;
                }
                index++;
            }
        }

        // DeathKnights
        uint8 DeathKnightCount = 2;
        SummonList.sort(ObjectDistanceOrderReversed(me));
        for (std::list<Creature*>::iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
        {
            if (*itr)
            {
                if (DeathKnightCount == 0)
                    break;

                DeathKnightSummonPosGuids.push_back((*itr)->GetGUID());
                --DeathKnightCount;
            }
        }
    }

    void SummonAdds(bool /*bRightSide*/, uint32 SummonEntry)
    {
        std::list<uint64>* plSummonPosGuids;
        switch (SummonEntry)
        {
            case NPC_UNREL_TRAINEE:
                plSummonPosGuids = &TraineeSummonPosGuids;
                break;
            case NPC_UNREL_DEATH_KNIGHT:
                plSummonPosGuids = &DeathKnightSummonPosGuids;
                break;
            case NPC_UNREL_RIDER:
                plSummonPosGuids = &RiderSummonPosGuids;
                break;
            default:
                return;
        }
        if (plSummonPosGuids->empty())
            return;

        for (std::list<uint64>::iterator itr = plSummonPosGuids->begin(); itr != plSummonPosGuids->end(); ++itr)
        {
            if (Creature* pPos = me->GetMap()->GetCreature(*itr))
                me->SummonCreature(SummonEntry, pPos->GetPositionX(), pPos->GetPositionY(), pPos->GetPositionZ(), pPos->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        SummonedAddGuids.push_back(pSummoned->GetGUID());
        if (!IsCentralDoorClosed())
        {
            pSummoned->AI()->DoZoneInCombat();
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                pSummoned->AI()->AttackStart(target);
        }
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* /*killer*/) override
    {
        SummonedAddGuids.remove(pSummoned->GetGUID());

        if (!pInstance)
            return;

        if (Creature* pAnchor = pInstance->GetClosestAnchorForGoth(pSummoned, true))
        {
            switch (pSummoned->GetEntry())
            {
                    // Wrong caster, it expected to be pSummoned.
                    // Mangos deletes the spell event at caster death, so for delayed spell like this
                    // it's just a workaround. Does not affect other than the visual though (+ spell takes longer to "travel")
                case NPC_UNREL_TRAINEE:         me->CastSpell(pAnchor, SPELL_A_TO_ANCHOR_1, true, NULL, NULL, pSummoned->GetGUID()); break;
                case NPC_UNREL_DEATH_KNIGHT:    me->CastSpell(pAnchor, SPELL_B_TO_ANCHOR_1, true, NULL, NULL, pSummoned->GetGUID()); break;
                case NPC_UNREL_RIDER:           me->CastSpell(pAnchor, SPELL_C_TO_ANCHOR_1, true, NULL, NULL, pSummoned->GetGUID()); break;
            }
        }
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        switch (Phase)
        {
            case PHASE_SPEECH:
                if (SpeechTimer < diff)
                {
                    switch (Speech)
                    {
                        case 1: DoScriptText(SAY_SPEECH_1, me); SpeechTimer = 41000; break;
                        case 2: DoScriptText(SAY_SPEECH_2, me); SpeechTimer = 61000; break;
                        case 3: DoScriptText(SAY_SPEECH_3, me); SpeechTimer = 51000; break;
                        case 4: DoScriptText(SAY_SPEECH_4, me); Phase = PHASE_BALCONY; break;
                    }
                    Speech++;
                }
                else
                    SpeechTimer -= diff;

                // No break here

            case PHASE_BALCONY:                            // Do summoning
                if (TraineeTimer < diff)
                {
                    SummonAdds(true, NPC_UNREL_TRAINEE);
                    TraineeTimer = 20000;
                }
                else
                    TraineeTimer -= diff;

                if (DeathKnightTimer < diff)
                {
                    SummonAdds(true, NPC_UNREL_DEATH_KNIGHT);
                    DeathKnightTimer = 25000;
                }
                else
                    DeathKnightTimer -= diff;

                if (RiderTimer < diff)
                {
                    SummonAdds(true, NPC_UNREL_RIDER);
                    RiderTimer = 30000;
                }
                else
                    RiderTimer -= diff;

                if (PhaseTimer < diff)
                {
                    Phase = PHASE_STOP_SUMMONING;
                    PhaseTimer = 27000;
                }
                else
                    PhaseTimer -= diff;

                break;

            case PHASE_STOP_SUMMONING:
                if (PhaseTimer < diff)
                {
                    DoCast(me, SPELL_TELEPORT_RIGHT, true);
                    Phase = pInstance ? PHASE_TELEPORTING : PHASE_STOP_TELEPORTING;

                    DoScriptText(SAY_TELEPORT, me);
                    DoScriptText(EMOTE_TO_FRAY, me);

                    me->SetRooted(false);
                    // Remove Immunity
                    me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);

                    DoResetThreat();
                    me->SetInCombatWithZone();
                }
                else
                    PhaseTimer -= diff;

                break;

            case PHASE_TELEPORTING:                         // Phase is only reached if pInstance is valid
                if (TeleportTimer < diff)
                {
                    uint32 TeleportSpell = pInstance->IsInRightSideGothArea(me) ? SPELL_TELEPORT_LEFT : SPELL_TELEPORT_RIGHT;
                    DoCast(me, TeleportSpell);
                    TeleportTimer = 20000;
                    ShadowboltTimer = 2000;
                }
                else
                    TeleportTimer -= diff;

                if (me->GetHealthPercent() <= 30)
                {
                    Phase = PHASE_STOP_TELEPORTING;
                    ProcessCentralDoor();
                    // as the doors now open, recheck whether mobs are standing around
                    ControlZoneTimer = 1;
                }
                // no break here

            case PHASE_STOP_TELEPORTING:
                if (HarvestSoulTimer < diff)
                {
                    DoCast(me, SPELL_HARVESTSOUL);
                    HarvestSoulTimer = 15000;
                }
                else
                    HarvestSoulTimer -= diff;

                if (ShadowboltTimer)
                {
                    if (ShadowboltTimer <= diff)
                    {
                        ShadowboltTimer = 0;
                    }
                    else
                        ShadowboltTimer -= diff;
                }
                // Shadowbold cooldown finished, cast when ready
                else if (!me->IsNonMeleeSpellCast(true))
                {
                    // Select valid target
                    if (Unit* pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 50, true))
                        DoCast(pTarget, SPELL_SHADOWBOLT);
                }

                break;
        }

        // Control Check, if Death zone empty
        if (ControlZoneTimer)
        {
            if (ControlZoneTimer <= diff)
            {
                ControlZoneTimer = 0;

                if (pInstance && !HasPlayersInLeftSide())
                {
                    ProcessCentralDoor();
                    for (std::list<uint64>::const_iterator itr = SummonedAddGuids.begin(); itr != SummonedAddGuids.end(); itr++)
                    {
                        if (Creature* pCreature = me->GetMap()->GetCreature(*itr))
                        {
                            if (!pCreature->IsInCombat())
                                pCreature->SetInCombatWithZone();
                        }
                    }
                }
            }
            else
                ControlZoneTimer -= diff;
        }
    }
};

CreatureAI* GetAI_boss_gothik(Creature *_Creature)
{
    return new boss_gothikAI (_Creature);
}

bool EffectDummyCreature_spell_anchor(Unit* pCaster, uint32 uiSpellId, uint32 uiEffIndex, Creature* pCreatureTarget)
{
    if (uiEffIndex != 0 || pCreatureTarget->GetEntry() != NPC_SUB_BOSS_TRIGGER)
    {
        return true;
    }

    instance_naxxramas* pInstance = (instance_naxxramas*)pCreatureTarget->GetInstanceData();

    if (!pInstance)
        return true;

    switch (uiSpellId)
    {
        case SPELL_A_TO_ANCHOR_1:                           // trigger mobs at high right side
        case SPELL_B_TO_ANCHOR_1:
        case SPELL_C_TO_ANCHOR_1:
        {
            if (Creature* pAnchor2 = pInstance->GetClosestAnchorForGoth(pCreatureTarget, false))
            {
                uint32 uiTriggered = SPELL_A_TO_ANCHOR_2;
                if (uiSpellId == SPELL_B_TO_ANCHOR_1)
                    uiTriggered = SPELL_B_TO_ANCHOR_2;
                else if (uiSpellId == SPELL_C_TO_ANCHOR_1)
                    uiTriggered = SPELL_C_TO_ANCHOR_2;

                pCreatureTarget->CastSpell(pAnchor2, uiTriggered, true);
            }

            return true;
        }
        case SPELL_A_TO_ANCHOR_2:                           // trigger mobs at high left side
        case SPELL_B_TO_ANCHOR_2:
        case SPELL_C_TO_ANCHOR_2:
        {
            std::list<Creature*> lTargets;
            pInstance->GetGothSummonPointCreatures(lTargets, false);
            if (!lTargets.empty())
            {
                std::list<Creature*>::iterator itr = lTargets.begin();
                uint32 uiPosition = urand(0, lTargets.size() - 1);
                advance(itr, uiPosition);

                if (Creature* pTarget = (*itr))
                {
                    uint32 uiTriggered = SPELL_A_TO_SKULL;

                    if (uiSpellId == SPELL_B_TO_ANCHOR_2)
                        uiTriggered = SPELL_B_TO_SKULL;
                    else if (uiSpellId == SPELL_C_TO_ANCHOR_2)
                        uiTriggered = SPELL_C_TO_SKULL;
                    pCreatureTarget->CastSpell(pTarget, uiTriggered, true);
                }
            }
            return true;
        }
        case SPELL_A_TO_SKULL:                              // final destination trigger mob
        case SPELL_B_TO_SKULL:
        case SPELL_C_TO_SKULL:
        {
            if (Creature* pGoth = pCaster->GetCreature(pInstance->GetData64(DATA_GOTHIK_THE_HARVESTER)))
            {
                uint32 uiNpcEntry = NPC_SPECT_TRAINEE;

                if (uiSpellId == SPELL_B_TO_SKULL)
                    uiNpcEntry = NPC_SPECT_DEATH_KNIGHT;
                else if (uiSpellId == SPELL_C_TO_SKULL)
                    uiNpcEntry = NPC_SPECT_RIDER;
                pGoth->SummonCreature(uiNpcEntry, pCreatureTarget->GetPositionX(), pCreatureTarget->GetPositionY(), pCreatureTarget->GetPositionZ(), pCreatureTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);

                if (uiNpcEntry == NPC_SPECT_RIDER)
                {
                    pGoth->SummonCreature(NPC_SPECT_HORSE, pCreatureTarget->GetPositionX(), pCreatureTarget->GetPositionY(), pCreatureTarget->GetPositionZ(), pCreatureTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
                }
            }
            return true;
        }
    }

    return true;
};

void AddSC_boss_gothik()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_gothik";
    newscript->GetAI = &GetAI_boss_gothik;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "spell_anchor";
    newscript->pEffectDummyNPC = &EffectDummyCreature_spell_anchor;
    newscript->RegisterSelf();
}
