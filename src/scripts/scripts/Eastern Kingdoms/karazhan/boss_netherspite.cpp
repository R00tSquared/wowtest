// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
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
 SDName: Boss_Netherspite
 SD%Complete: 90
 SDComment: Not sure about timing and portals placing
 SDCategory: Karazhan
 EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

enum Netherspite
{
    BANISH_EMOTE            = -1532090,
    PORTAL_EMOTE            = -1532089,

    // Perseverence: Red, Serenity: Green, Domination: Blue

    // Player beams:
    SPELL_BEAM_RED          = 30400,
    SPELL_BEAM_GREEN        = 30401,
    SPELL_BEAM_BLUE         = 30402,

    // Netherspite beams:
    SPELL_NETHERBEAM_RED    = 30465,
    SPELL_NETHERBEAM_GREEN  = 30464,
    SPELL_NETHERBEAM_BLUE   = 30463,

    SPELL_PORTAL_RED        = 30487,
    SPELL_PORTAL_GREEN      = 30490,
    SPELL_PORTAL_BLUE       = 30491,

    SPELL_PBUFF_RED         = 30421,
    SPELL_PBUFF_GREEN       = 30422,
    SPELL_PBUFF_BLUE        = 30423,

    SPELL_NBUFF_RED         = 30466,
    SPELL_NBUFF_GREEN       = 30467,
    SPELL_NBUFF_BLUE        = 30468,

    SPELL_EXHAUST_RED       = 38637,
    SPELL_EXHAUST_GREEN     = 38638,
    SPELL_EXHAUST_BLUE      = 38639,

    NPC_PORTAL_RED          = 17369,
    NPC_PORTAL_GREEN        = 17367,
    NPC_PORTAL_BLUE         = 17368,

    SPELL_VOID_ZONE         = 37063,
    SPELL_CONSUMPTION_TICK  = 28865,
    SPELL_NETHERBREATH      = 38523,
    SPELL_NETHER_BURN       = 30522,
    SPELL_EMPOWERMENT       = 38549,
    SPELL_VIS_NETHER_RAGE   = 39833,
    SPELL_NETHER_INFUSION   = 38688,

    SPELL_NETHERSPITE_ROAR  = 38684,

    SPELL_INVISIBLE_ROOT    = 42716,
    SPELL_VOID_ZONE_EFFECT  = 46264,
    NETHER_PATROL_PATH      = 15689,
};

enum NetherspitePhase
{
    PORTAL_PHASE = 1,
    BANISH_PHASE = 2,
};

enum PortalColour
{
    PORTAL_RED,
    PORTAL_GREEN,
    PORTAL_BLUE,
};

#define IN_FRONT_10_F M_PI_F / 18

struct PortalPos
{
    float X, Y, Z;
};

const PortalPos PortalPositions[] = 
{
    {-11140.0f, -1680.5f, 278.2f},
    {-11196.0f, -1613.5f, 278.2f},
    {-11107.7f, -1602.6f, 279.9f}
};

ptrdiff_t netherspite_shufflerng(ptrdiff_t i)
{
    return urand(0, i - 1);
}

struct PortalDebuff
{
    PortalDebuff(ObjectGuid o, uint32 t) : target_guid(std::move(o)), time_remaining(t)
    {
    }
    ObjectGuid target_guid;
    uint32 time_remaining;
};

/**************
* Void Zone - id 16697
***************/

struct mob_void_zoneAI : public Scripted_NoMovementAI
{
    mob_void_zoneAI(Creature* c) : Scripted_NoMovementAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    Timer checkTimer;
    Timer dieTimer;

    void Reset()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        checkTimer.Reset(500);
        dieTimer.Reset(25000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (checkTimer.Expired(diff))
        {
            if (pInstance && pInstance->GetData(DATA_NETHERSPITE_EVENT) == DONE)
            {
                me->Kill(me, false);
                me->RemoveCorpse();
            }

            const int32 dmg = frand(1000, 1500);    // workaround here, no proper spell known
            me->CastCustomSpell(NULL, SPELL_VOID_ZONE_EFFECT, &dmg, NULL, NULL, false);
            checkTimer = 2000;
        }

        if (dieTimer.Expired(diff))
        {
            me->Kill(me, false);
            me->RemoveCorpse();
            dieTimer = 25000;
        }
    }
};

CreatureAI* GetAI_mob_void_zone(Creature *_Creature)
{
    return new mob_void_zoneAI(_Creature);
}

struct nether_portalAI : public ScriptedAI
{
    nether_portalAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    // Disable combat:
    void MoveInLineOfSight(Unit* /*pWho*/) {}
    void AttackStart(Unit* /*pWho*/) {}
    void AttackedBy(Unit* /*pAttacker*/) {}

    PortalColour colour;
    ObjectGuid netherspiteGuid;
    ObjectGuid currentTarget;
    Timer checkTimer;
    uint32 activationTimer;
    bool hasActivatedPortalSpell;
    std::vector<PortalDebuff> debuffedTargets;

    void Reset()
    {
        hasActivatedPortalSpell = false;
        checkTimer.Reset(1000);
        activationTimer = 10000;
        currentTarget = ObjectGuid();
    }

    void StopPortal()
    {
        me->InterruptNonMeleeSpells(false);
        for (auto& elem : debuffedTargets)
        {
            if (Player* pPlayer = Unit::GetPlayerInWorld((elem).target_guid))
            {
                switch (colour)
                {
                    case PORTAL_RED:
                        pPlayer->AddAura(SPELL_EXHAUST_RED, pPlayer);
                        pPlayer->RemoveAurasDueToSpell(SPELL_PBUFF_RED); // Just in case it lingers...
                        me->RemoveAurasDueToSpell(SPELL_PORTAL_RED);
                        break;
                    case PORTAL_GREEN:
                        pPlayer->AddAura(SPELL_EXHAUST_GREEN, pPlayer);
                        pPlayer->RemoveAurasDueToSpell(SPELL_PBUFF_GREEN);
                        me->RemoveAurasDueToSpell(SPELL_PORTAL_GREEN);
                        break;
                    case PORTAL_BLUE:
                        pPlayer->AddAura(SPELL_EXHAUST_BLUE, pPlayer);
                        pPlayer->RemoveAurasDueToSpell(SPELL_PBUFF_BLUE);
                        me->RemoveAurasDueToSpell(SPELL_PORTAL_BLUE);
                        break;
                }
            }
        }
        me->ForcedDespawn();
        netherspiteGuid.Set(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!netherspiteGuid)
            return;

        if (!hasActivatedPortalSpell)
        {
            switch (colour)
            {
                case PORTAL_RED:
                    DoCast(me, SPELL_PORTAL_RED);
                    hasActivatedPortalSpell = true;
                    break;
                case PORTAL_GREEN:
                    DoCast(me, SPELL_PORTAL_GREEN);
                    hasActivatedPortalSpell = true;
                    break;
                case PORTAL_BLUE:
                    DoCast(me, SPELL_PORTAL_BLUE);
                    hasActivatedPortalSpell = true;
                    break;
            }
            if (!hasActivatedPortalSpell)
                return;
        }

        if (activationTimer)
        {
            if (activationTimer <= diff)
                activationTimer = 0;
            else
            {
                activationTimer -= diff;
                return;
            }
        }

        Creature* netherspite = me->GetMap()->GetCreature(netherspiteGuid);

        if (!netherspite)
            return;

        me->SetOrientation(me->GetAngleTo(netherspite));
        me->SetFacingTo(me->GetAngleTo(netherspite));

        for (std::vector<PortalDebuff>::iterator itr = debuffedTargets.begin(); itr != debuffedTargets.end();)
        {
            if (itr->time_remaining <= diff)
            {
                Player* plr = Unit::GetPlayerInWorld(itr->target_guid);
                if (plr && plr->isAlive())
                {
                    switch (colour)
                    {
                        case PORTAL_RED:
                            plr->AddAura(SPELL_EXHAUST_RED, plr); // You cannot avoid the exhaustion
                            plr->RemoveAurasDueToSpell(SPELL_PBUFF_RED); // Just in case it lingers...
                            plr->UpdateMaxHealth();
                            break;
                        case PORTAL_GREEN:
                            plr->AddAura(SPELL_EXHAUST_GREEN, plr);
                            plr->RemoveAurasDueToSpell(SPELL_PBUFF_GREEN);
                            break;
                        case PORTAL_BLUE:
                            plr->AddAura(SPELL_EXHAUST_BLUE, plr);
                            plr->RemoveAurasDueToSpell(SPELL_PBUFF_BLUE);
                            break;
                    }
                }
                itr = debuffedTargets.erase(itr);
            }
            else
            {
                itr->time_remaining -= diff;
                ++itr;
            }
        }

        if (checkTimer.Expired(diff))
        {
            float distNether = me->GetDistance(netherspite->GetPositionX(), netherspite->GetPositionY(), netherspite->GetPositionZ());

            std::list<Player*> unitList;
            Hellground::AnyPlayerInObjectRangeCheck check(me, distNether, true);
            Hellground::ObjectListSearcher<Player, Hellground::AnyPlayerInObjectRangeCheck> searcher(unitList, check);

            Cell::VisitAllObjects(me, searcher, distNether);

            std::vector<Player*> possibleTargets;
            for (auto temp : unitList)
            {
                if (temp->isGameMaster())
                    continue;

                if (temp->isAlive())
                {
                    if ((colour == PORTAL_RED && temp->HasAura(SPELL_EXHAUST_RED)) ||
                        (colour == PORTAL_GREEN && temp->HasAura(SPELL_EXHAUST_GREEN)) ||
                        (colour == PORTAL_BLUE && temp->HasAura(SPELL_EXHAUST_BLUE)) ||
                        temp->IsImmunedToDamage(SPELL_SCHOOL_MASK_ALL))
                        continue;

                    if (me->isInFront(temp, distNether, IN_FRONT_10_F) &&
                        me->GetDistance(temp) < distNether)
                        possibleTargets.push_back(temp);
                }
            }

            uint32 beamId = colour == PORTAL_RED ? SPELL_NETHERBEAM_RED :
                                                     colour == PORTAL_GREEN ?
                                                     SPELL_NETHERBEAM_GREEN :
                                                     SPELL_NETHERBEAM_BLUE;
            if (possibleTargets.empty())
            {
                // No players to cast on, cast on netherspite
                if (currentTarget != netherspite->GetObjectGuid())
                    me->InterruptNonMeleeSpells(true);

                DoCast(netherspite, beamId);
                //if (currentTarget == netherspite->GetObjectGuid())
                //{
                    uint32 buffId = colour == PORTAL_RED ?
                                        SPELL_NBUFF_RED :
                                        colour == PORTAL_GREEN ?
                                        SPELL_NBUFF_GREEN :
                                        SPELL_NBUFF_BLUE;
                    netherspite->CastSpell(netherspite, buffId, true);
                    currentTarget = netherspite->GetObjectGuid();
                //}
            }
            else
            {
                // There exists possible players to cast on
                Player* target = NULL;
                float lastDist = 1000.0f;
                for (auto& possibleTarget : possibleTargets)
                {
                    float dist = possibleTarget->GetDistance(me);
                    if (dist < lastDist)
                    {
                        target = possibleTarget;
                        lastDist = dist;
                    }
                }

                uint32 buffId = colour == PORTAL_RED ?
                                    SPELL_PBUFF_RED :
                                    colour == PORTAL_GREEN ?
                                    SPELL_PBUFF_GREEN :
                                    SPELL_PBUFF_BLUE;
                uint32 duration = colour == PORTAL_RED ?
                                      20000 :
                                      colour == PORTAL_GREEN ? 10000 : 8000;
                if (currentTarget != target->GetObjectGuid())
                    me->InterruptNonMeleeSpells(true);

                DoCast(target, beamId);
                //if (currentTarget == target->GetObjectGuid())
                //{
                    target->CastSpell(target, buffId, true);

                    if (colour == PORTAL_RED)
                    {
                        netherspite->getThreatManager().modifyThreatPercent(target, -100); // reset threat to 0
                        netherspite->getThreatManager().addThreat(target, 200000.0f + DoGetThreat(target)); // set from second target PLUS 200k
                    }
                    bool addNew = true;
                    std::vector<PortalDebuff>::iterator itr;
                    for (itr = debuffedTargets.begin();itr != debuffedTargets.end(); ++itr)
                    {
                        if (itr->target_guid == target->GetObjectGuid())
                        {
                            addNew = false;
                            break;
                        }
                    }
                    if (addNew)
                        debuffedTargets.push_back(
                            PortalDebuff(target->GetObjectGuid(), duration));
                    else
                        itr->time_remaining = duration;
                    currentTarget = target->GetObjectGuid();
                //}
            }
            checkTimer = 1000;
        }
    }
};

struct nether_portal_redAI : public nether_portalAI
{
    nether_portal_redAI(Creature* pCreature) : nether_portalAI(pCreature)
    {
        Reset();
    }

    void Reset() { colour = PORTAL_RED; }
};
CreatureAI* GetAI_nether_portal_red(Creature* pCreature)
{
    return new nether_portal_redAI(pCreature);
}

struct nether_portal_greenAI : public nether_portalAI
{
    nether_portal_greenAI(Creature* pCreature) : nether_portalAI(pCreature)
    {
        Reset();
    }

    void Reset() { colour = PORTAL_GREEN; }
};
CreatureAI* GetAI_nether_portal_green(Creature* pCreature)
{
    return new nether_portal_greenAI(pCreature);
}

struct nether_portal_blueAI : public nether_portalAI
{
    nether_portal_blueAI(Creature* pCreature) : nether_portalAI(pCreature)
    {
        Reset();
    }

    void Reset() { colour = PORTAL_BLUE; }
};
CreatureAI* GetAI_nether_portal_blue(Creature* pCreature)
{
    return new nether_portal_blueAI(pCreature);
}

struct boss_netherspiteAI : public ScriptedAI
{
    boss_netherspiteAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (pCreature->GetInstanceData());
        Reset();
    }

    ScriptedInstance* pInstance;

    NetherspitePhase phase;
    Timer enrageTimer;
    Timer phaseTimer;
    // Phase One
    std::vector<ObjectGuid> portalGuids;
    Timer portalsTimer;
    Timer voidZoneTimer;
    Timer empowermentTimer;
    bool firstSpawn;
    bool Berserk;
    bool Empowerment;
    bool PortalsPhase;
    // Phase Two
    Timer netherbreathTimer;

    void Reset()
    {
        ClearCastQueue();
        phase = PORTAL_PHASE;
        phaseTimer.Reset(60000);
        portalsTimer.Reset(5000);
        PortalsPhase = true;
        voidZoneTimer.Reset(urand(5000, 15000));
        netherbreathTimer.Reset(0);
        empowermentTimer.Reset(15000);
        enrageTimer.Reset(9 * 60 * 1000);
        firstSpawn = true;
        Berserk = false;
        Empowerment = false;

        SetCombatMovement(true);
        me->SetSelection(0);

        me->GetMotionMaster()->MovePath(NETHER_PATROL_PATH, true);

        DespawnPortals();
        HandleDoors(true);

        if(pInstance && pInstance->GetData(DATA_NETHERSPITE_EVENT) != DONE)
            pInstance->SetData(DATA_NETHERSPITE_EVENT, NOT_STARTED);
    }

    void EnterCombat(Unit* who)
    {
        me->GetMotionMaster()->Clear();
        DoStartMovement(who);
        DoCast(me, SPELL_NETHER_BURN, true);
        if (pInstance)
        {
            pInstance->SetData(DATA_NETHERSPITE_EVENT, IN_PROGRESS);
            HandleDoors(false);
        }
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (pInstance)
        {
            pInstance->SetData(DATA_NETHERSPITE_EVENT, DONE);
            HandleDoors(true);
        }
        DespawnPortals();
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!me->IsInCombat() && me->IsWithinDistInMap(who, 25.0) && me->IsHostileTo(who))
            AttackStart(who);
    }

    void DespawnPortals()
    {
        for (auto& elem : portalGuids)
        {
            if (Creature* portal = me->GetMap()->GetCreature(elem))
            {
                if (nether_portalAI* portalAI = dynamic_cast<nether_portalAI*>(portal->AI()))
                    portalAI->StopPortal();
                portal->ForcedDespawn();
            }
        }
        portalGuids.clear();
    }

    void DoPhaseSwitchIfNeeded(const uint32 diff)
    {
        if (phaseTimer.Expired(diff))
        {
            if (phase == PORTAL_PHASE)
            {
                DoCast(me, SPELL_NETHERSPITE_ROAR);
                DoCast(me, SPELL_INVISIBLE_ROOT, true);
                SetCombatMovement(false);
                DespawnPortals();
                me->RemoveAurasDueToSpell(SPELL_EMPOWERMENT);
                me->RemoveAurasDueToSpell(SPELL_NETHER_BURN);
                Empowerment = true;
                DoScriptText(BANISH_EMOTE, me);
                DoCast(me, SPELL_VIS_NETHER_RAGE, true);
                DoResetThreat();
                me->SetSelection(0);
                phase = BANISH_PHASE;
                netherbreathTimer = urand(10000, 15000);
                phaseTimer = 30000;
            }
            else if (phase == BANISH_PHASE)
            {
                DoCast(me, SPELL_NETHERSPITE_ROAR);
                me->RemoveAurasDueToSpell(SPELL_INVISIBLE_ROOT);
                SetCombatMovement(true);
                me->RemoveAurasDueToSpell(SPELL_VIS_NETHER_RAGE);
                DoScriptText(PORTAL_EMOTE, me);
                DoResetThreat();
                phase = PORTAL_PHASE;
                portalsTimer = 5000;
                PortalsPhase = true;
                empowermentTimer = 10000;
                Empowerment = false;
                phaseTimer = 60000;
                voidZoneTimer = urand(5000, 15000);
            }
        }
    }

    void HandleDoors(bool open) // Massive Door switcher
    {
        if(GameObject *Door = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GAMEOBJECT_MASSIVE_DOOR)))
            Door->SetUInt32Value(GAMEOBJECT_STATE, open ? 0 : 1);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVERYTHING, 125.0f, 1.5f);

        if (!Berserk && enrageTimer.Expired(diff))
        {
            DoCast(me, SPELL_NETHER_INFUSION);
            ForceSpellCast(me, SPELL_NETHERSPITE_ROAR, INTERRUPT_AND_CAST_INSTANTLY);
            enrageTimer = 0;
            Berserk = true;
        }

        DoPhaseSwitchIfNeeded(diff);

        if (phase == PORTAL_PHASE)
        {
            if (!Empowerment && !Berserk) // Stop casting Empowerement if enrage is active
            {
                if (empowermentTimer.Expired(diff))
                {
                    ForceSpellCast(me, SPELL_EMPOWERMENT);
                    DoCast(me, SPELL_NETHER_BURN);
                    empowermentTimer = 0;
                    Empowerment = true;
                }
            }

            if (voidZoneTimer.Expired(diff))
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,1,GetSpellMaxRange(SPELL_VOID_ZONE),true, me->getVictimGUID()))
                    AddSpellToCast(target, SPELL_VOID_ZONE, true);
                voidZoneTimer = 15000;
            }

            if (PortalsPhase)
            {
                if (portalsTimer.Expired(diff))
                {
                    std::vector<PortalColour> colours;
                    colours.push_back(PORTAL_RED);
                    colours.push_back(PORTAL_GREEN);
                    colours.push_back(PORTAL_BLUE);
                    // Keep RGB order first time, random otherwise
                    if (!firstSpawn)
                        std::random_shuffle(colours.begin(), colours.end(), netherspite_shufflerng);
                    else
                        firstSpawn = false;

                    int i = 0;
                    for (std::vector<PortalColour>::iterator itr = colours.begin(); itr != colours.end() && i < 3; ++itr)
                    {
                        PortalPos p = PortalPositions[i++];
                        switch (*itr)
                        {
                            case PORTAL_RED:
                                if (Creature* portal = me->SummonCreature(NPC_PORTAL_RED, p.X, p.Y, p.Z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 100 * 1000))
                                {
                                    if (nether_portalAI* pAI = dynamic_cast<nether_portalAI*>(portal->AI()))
                                        pAI->netherspiteGuid = me->GetObjectGuid();
                                    portalGuids.push_back(portal->GetObjectGuid());
                                }
                                break;
                            case PORTAL_BLUE:
                                if (Creature* portal = me->SummonCreature(NPC_PORTAL_BLUE, p.X, p.Y, p.Z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 100 * 1000))
                                {
                                    if (nether_portalAI* pAI = dynamic_cast<nether_portalAI*>(portal->AI()))
                                        pAI->netherspiteGuid = me->GetObjectGuid();
                                    portalGuids.push_back(portal->GetObjectGuid());
                                }
                                break;
                            case PORTAL_GREEN:
                                if (Creature* portal = me->SummonCreature(NPC_PORTAL_GREEN, p.X, p.Y, p.Z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 100 * 1000))
                                {
                                    if (nether_portalAI* pAI = dynamic_cast<nether_portalAI*>(portal->AI()))
                                        pAI->netherspiteGuid = me->GetObjectGuid();
                                    portalGuids.push_back(portal->GetObjectGuid());
                                }
                                break;
                        }
                    }
                    portalsTimer = 0;
                    PortalsPhase = false;
                }
            }
        }
        else if (phase == BANISH_PHASE)
        {
            if (netherbreathTimer.Expired(diff))
            {
                if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_NETHERBREATH), true))
                    AddSpellToCast(target, SPELL_NETHERBREATH, false, true);
                netherbreathTimer = 5000;
            }
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_boss_netherspite(Creature* pCreature)
{
    return new boss_netherspiteAI(pCreature);
}

void AddSC_boss_netherspite()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_netherspite";
    newscript->GetAI = GetAI_boss_netherspite;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_void_zone";
    newscript->GetAI = GetAI_mob_void_zone;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "nether_portal_red";
    newscript->GetAI = GetAI_nether_portal_red;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "nether_portal_green";
    newscript->GetAI = GetAI_nether_portal_green;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "nether_portal_blue";
    newscript->GetAI = GetAI_nether_portal_blue;
    newscript->RegisterSelf();
}
