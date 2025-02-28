// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2014 Hellground <http://hellground.net/>
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
SDName: Boss_Onyxia
SD%Complete: 90
SDComment: Spell Heated Ground is wrong, flying animation, visual for area effect
SDCategory: Onyxia's Lair
EndScriptData */

#include "precompiled.h"
#include "def_onyxia_lair.h"

#define SAY_AGGRO                   -1249000
#define SAY_KILL                    -1249001
#define SAY_PHASE_2_TRANS           -1249002
#define SAY_PHASE_3_TRANS           -1249003
#define EMOTE_BREATH                -1249004

enum OnyxiaSpells
{
    // Phase 1 & 3
    SPELL_FLAMEBREATH           = 18435,
    SPELL_CLEAVE                = 19983, // find correct id
    SPELL_WINGBUFFET            = 18500,
    SPELL_TAILSWEEP             = 15847,
    SPELL_KNOCK_AWAY            = 19633, // this prob too

    // Phase 2
    SPELL_SUMMONWHELP           = 17646,
    SPELL_FIREBALL              = 18392,
    SPELL_DEEPBREATH            = 23461,

    // Phase 3
    SPELL_BELLOWINGROAR         = 18431,

    // Whelp Spell
    SPELL_PYROBLAST             = 20228
};


#define SPELL_ENGULFINGFLAMES   20019
#define CREATURE_WHELP          11262
#define NPC_ONYXIAN_WARDER      12129

struct Position;
enum SpawnDefinitions;
extern cPosition spawnEntrancePoints[MAX];

static cPosition center = {-24.8694, -214.071, -89.246};

static cPosition flyLocations[] =
{
    {-65.5955, -222.839, -84.3624},
    {-48.26, -196.624, -86.1145},
    {-12.4892, -213.19, -88.0036},
    {20.0359, -216.578, -85.3187},
    {-66.6432, -214.359, -84.2238}
};

enum PhaseMask
{
    PHASE_1 = 0x01,
    PHASE_2 = 0x02,
    PHASE_3 = 0x04
};

struct boss_onyxiaAI : public ScriptedAI
{
    boss_onyxiaAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance *)c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

    uint32 phaseMask;

    uint32 rangeCheckTimer;
    uint32 flameBreathTimer;
    uint32 cleaveTimer;
    uint32 tailSweepTimer;
    uint32 knockBackTimer;
    uint32 wingBuffetTimer;

    uint32 summonWhelpsTimer;

    uint32 eruptionTimer;
    uint32 eruption;
    uint32 fearTimer;

    uint32 nextWay;
    uint32 nextMoveTimer;

    void Fly()
    {
        if (me->IsLevitating())
        {
            me->SendMeleeAttackStart(me->getVictimGUID());
            me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
            me->SetLevitate(false);
            DoStartMovement(me->GetVictim());
        }
        else
        {
            me->SendMeleeAttackStop(me->GetVictim());
            me->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
            me->SetLevitate(true);
        }
    }

    void DoMeleeAttackIfReady()
    {
        if(me->HasUnitState(UNIT_STAT_CASTING))
            return;

        if (phaseMask & PHASE_2)
        {
            if (!nextWay || nextWay == 6)
                return;

            me->SendMeleeAttackStop(me->GetVictim());
            DoCast(m_creature->GetVictim(), SPELL_FIREBALL);
            me->getThreatManager().modifyThreatPercent(me->GetVictim(), 100);
        }
        else
        {
            //Make sure our attack is ready and we aren't currently casting before checking distance
            if (me->isAttackReady())
            {
                //If we are within range melee the target
                if (me->IsWithinMeleeRange(me->GetVictim()))
                {
                    me->AttackerStateUpdate(me->GetVictim());
                    me->resetAttackTimer();
                }
            }
            if (me->haveOffhandWeapon() && me->isAttackReady(OFF_ATTACK))
            {
                //If we are within range melee the target
                if (me->IsWithinMeleeRange(me->GetVictim()))
                {
                    me->AttackerStateUpdate(me->GetVictim(), OFF_ATTACK);
                    me->resetAttackTimer(OFF_ATTACK);
                }
            }
        }
    }

    void Reset()
    {
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, 3); // Lie animation
        if (pInstance)
            pInstance->SetData(DATA_ONYXIA, NOT_STARTED);

        me->SetLevitate(false);
        phaseMask = PHASE_1;

        nextWay = 0;
        nextMoveTimer = 0;

        rangeCheckTimer = 3000;
        flameBreathTimer = 20000;
        cleaveTimer = 8000;
        tailSweepTimer = 5000;
        knockBackTimer = 15000;
        wingBuffetTimer = 12000;

        summonWhelpsTimer = 10000;

        eruption = 20;
        eruptionTimer = 12000;
        fearTimer = 10000;
    }

    void EnterCombat(Unit* who)
    {
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
        DoScriptText(SAY_AGGRO, me);
        DoZoneInCombat();

        std::list<Creature*> warders = FindAllCreaturesWithEntry(NPC_ONYXIAN_WARDER, 200.0f);

        for (std::list<Creature*>::iterator i = warders.begin(); i != warders.end(); ++i)
            if (!(*i)->isAlive())
            {
                (*i)->setDeathState(DEAD);
                (*i)->Respawn();
            }

        if (pInstance)
            pInstance->SetData(DATA_ONYXIA, IN_PROGRESS);
    }

    void JustDied(Unit* Killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_ONYXIA, DONE);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(SAY_KILL, me);
    }

    void UpdatePhase()
    {
        switch (phaseMask)
        {
            case PHASE_1:
                if (me->GetHealthPercent() < 65.0f)
                {
                    phaseMask = PHASE_2;
                    DoScriptText(SAY_PHASE_2_TRANS, me);
                    me->GetMotionMaster()->MovePoint(0, center.x, center.y, center.z);
                }
                break;
            case PHASE_2:
                if (me->GetHealthPercent() < 30.0f)
                {
                    phaseMask = PHASE_1 | PHASE_3;
                    DoScriptText(SAY_PHASE_3_TRANS, me);
                    me->GetMotionMaster()->MovePoint(1, center.x, center.y, center.z);
                }
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 i)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (i)
        {
            // Phase Change
            case 0:
                nextWay = 2;
                nextMoveTimer = 3000;
                if (pInstance)
                    pInstance->SetData(DATA_HATCH_EGGS, 7);
            case 1:
                Fly();
                break;
            // Random Movements in phase 2
            case 2:
            case 3:
            case 4:
                nextWay = i + 1;
                nextMoveTimer = irand(12, 24)*1000;
                break;
            // Deep Breath
            case 5:
                nextWay = i + 1;
                nextMoveTimer = 2500;
                //DoTextEmote("Onyxia takes in a deep breath...", NULL, true);//DoScriptText(EMOTE_BREATH, me);
                me->SendMeleeAttackStop(me->GetVictim());
                DoCast(me, SPELL_DEEPBREATH); // here should be instant
                me->SetSpeed(MOVE_RUN, 2.5f);
                break;
            case 6:
                nextWay = 2;
                nextMoveTimer = 2000;
                me->SetSpeed(MOVE_RUN, 1.0f);
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_EVADE_CHECK, 75.0f);

        UpdatePhase();

        if (rangeCheckTimer < diff)
        {
            Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();

            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                if (Player* plr = i->getSource())
                    if (plr->isAlive() && !plr->isGameMaster() && !plr->IsWithinDistInMap(me, 100.0f))
                        plr->TeleportTo(me->GetMapId(), me->GetPositionX(), me->GetPositionY(),
                            me->GetPositionZ(), plr->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT);

            rangeCheckTimer = 3000;
        }
        else
            rangeCheckTimer -= diff;

        if (phaseMask & PHASE_1)
        {
            if (flameBreathTimer < diff)
            {
                AddSpellToCast(SPELL_FLAMEBREATH, CAST_TANK);
                flameBreathTimer = irand(16, 28) * 1000;
                std::list<GameObject*> BladesInRange;
                Hellground::AllGameObjectsWithEntryInGrid go_check(179561);
                Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> go_search(BladesInRange, go_check);
                Cell::VisitGridObjects(me, go_search, 150.0f);

                for (std::list<GameObject*>::const_iterator itr = BladesInRange.begin(); itr != BladesInRange.end(); ++itr)
                {
                    if (GameObject* go = (*itr)->ToGameObject())
                    {
						if (Unit *bladeOwner = go->GetOwner()) 
						{
							bladeOwner->SummonGameObject(179562, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), 0, 0, 0, 0, 0, 300000);
							go->RemoveFromWorld();
						}
                    }
                }
            }
            else
                flameBreathTimer -= diff;

            if (cleaveTimer < diff)
            {
                AddSpellToCast(SPELL_CLEAVE, CAST_TANK);
                cleaveTimer = irand(4, 10) * 1000;
            }
            else
                cleaveTimer -= diff;

            if (tailSweepTimer < diff)
            {
                AddSpellToCast(SPELL_TAILSWEEP, CAST_TANK);
                tailSweepTimer = irand(6, 14) * 1000;
            }
            else
                tailSweepTimer -= diff;

            if (knockBackTimer < diff)
            {
                AddSpellToCast(SPELL_KNOCK_AWAY, CAST_TANK);
                knockBackTimer = irand(22, 32) * 1000;
            }
            else
                knockBackTimer -= diff;

            if (wingBuffetTimer < diff)
            {
                AddSpellToCast(SPELL_WINGBUFFET, CAST_TANK);
                wingBuffetTimer = irand(24, 36) * 1000;
            }
            else
                wingBuffetTimer -= diff;
        }

        if (phaseMask & PHASE_3)
        {
            if (eruptionTimer < diff)
            {
                if (pInstance)
                    pInstance->SetData(DATA_ERUPT, 0);
                if ((eruption -= 3) == 2)
                    eruption = 20;
                eruptionTimer = eruption * 500;
            }
            else
                eruptionTimer -= diff;

            if (fearTimer < diff)
            {
                fearTimer = irand(10, 30) * 1000;
                AddSpellToCast(SPELL_BELLOWINGROAR, CAST_TANK);
            }
            else
                fearTimer -= diff;
        }

        if (phaseMask & PHASE_2)
        {
            if (nextWay)
            {
                if (nextMoveTimer < diff)
                {
                    if (!me->IsNonMeleeSpellCast(false))
                    {
                        me->InterruptNonMeleeSpells(false);
                        me->GetMotionMaster()->MovePoint(nextWay, flyLocations[nextWay-2].x, flyLocations[nextWay-2].y, flyLocations[nextWay-2].z);
                        nextWay = 0;
                    }
                }
                else
                    nextMoveTimer -= diff;
            }
        }

        if (phaseMask & (PHASE_3 | PHASE_2))
        {
            if (summonWhelpsTimer < diff)
            {
                if (pInstance)
                    pInstance->SetData(DATA_HATCH_EGGS, 2);
                summonWhelpsTimer = irand(18, 24) * 1000;
                if (phaseMask & PHASE_3)
                    summonWhelpsTimer *= 2.0;
            }
            else
                summonWhelpsTimer -= diff;
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_onyxiaAI(Creature *_Creature)
{
    return new boss_onyxiaAI (_Creature);
}

struct mob_onyxiawhelpAI : public ScriptedAI
{
    mob_onyxiawhelpAI(Creature* c) : ScriptedAI(c) {}

    uint32 pyroblastTimer;

    void Reset()
    {
        pyroblastTimer = irand(2, 8)*1000;
        if (!UpdateVictim())
        {
            uint32 moveTo = RIGHT;
            if (me->GetDistance2d(spawnEntrancePoints[LEFT].x, spawnEntrancePoints[LEFT].y) < me->GetDistance2d(spawnEntrancePoints[RIGHT].x, spawnEntrancePoints[RIGHT].y))
                moveTo = LEFT;

            me->GetMotionMaster()->MovePoint(0, spawnEntrancePoints[moveTo].x, spawnEntrancePoints[moveTo].y, spawnEntrancePoints[moveTo].z);
        }
    }

    void MovementInform(uint32 type, uint32 i)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (i)
        {
            case 0:
            DoZoneInCombat();
            break;
        }
    }

    void EnterCombat(Unit* who)
    {
        DoZoneInCombat();
    }

    void JustDied(Unit* Killer) {}

    void KilledUnit(Unit *victim) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (pyroblastTimer < diff)
        {
            AddSpellToCast(SPELL_PYROBLAST, CAST_TANK);
            pyroblastTimer = 6000 + irand(0, 6)*1000;
        }
        else
            pyroblastTimer -= diff;
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_onyxiawhelpAI(Creature *_Creature)
{
    return new mob_onyxiawhelpAI (_Creature);
}

void AddSC_boss_onyxia()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_onyxia";
    newscript->GetAI = &GetAI_boss_onyxiaAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_onyxia_whelp";
    newscript->GetAI = &GetAI_mob_onyxiawhelpAI;
    newscript->RegisterSelf();
}

