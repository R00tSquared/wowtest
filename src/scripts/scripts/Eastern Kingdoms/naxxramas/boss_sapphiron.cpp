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
SDName: Boss_Sapphiron
SD%Complete: 95
SDComment: Debugging
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum SapphironTexts
{
    EMOTE_BREATH            = -1533082,
    EMOTE_ENRAGE            = -1533083,
};

enum SapphironSpells
{
    SPELL_FROST_AURA            = 28531,
    SPELL_CLEAVE                = 19983,
    SPELL_TAIL_SWEEP            = 15847,
    SPELL_BLIZZARD              = 28560,
    SPELL_LIFE_DRAIN            = 28542,
    SPELL_ICEBOLT               = 28522,
    SPELL_FROST_BREATH          = 29318,
    SPELL_FROST_EXPLOSION       = 28524,
    SPELL_FROST_MISSILE         = 30101,
    SPELL_BERSERK               = 26662,
    SPELL_CHILL                 = 28547,
    SPELL_DIES                  = 29357
};

enum Phases
{
    PHASE_NULL          = 0,
    PHASE_BIRTH,
    PHASE_GROUND,
    PHASE_FLIGHT
};

enum SapphironEvents
{
    EVENT_BERSERK       = 1,
    EVENT_CLEAVE,
    EVENT_TAIL,
    EVENT_DRAIN,
    EVENT_BLIZZARD,
    EVENT_FLIGHT,
    EVENT_LIFTOFF,
    EVENT_ICEBOLT,
    EVENT_BREATH,
    EVENT_EXPLOSION,
    EVENT_LAND,
    EVENT_GROUND,
    EVENT_BIRTH
};

enum Misc
{
    NPC_BLIZZARD            = 16474,
    GO_ICEBLOCK             = 181247
};

typedef std::map<uint64, uint64> IceBlockMap;

struct boss_sapphironAI : public BossAI
{
    boss_sapphironAI(Creature* c) : BossAI(c, DATA_SAPPHIRON), phase(PHASE_NULL)
    {
        Initialize();
        instance = (c->GetInstanceData());
    }

    ScriptedInstance* instance;
    Phases phase;
    uint32 _iceboltCount;
    IceBlockMap _iceblocks;

    void Initialize()
    {
        float x, y, z;
        me->GetPosition(x, y, z);
        me->SummonGameObject(GO_BIRTH, 3522.236572, -5243.176758, 137.625916, 4.617902, 0, 0, 0, 0, 0);
        me->SetVisibility(VISIBILITY_OFF);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FROST, true);
    }

    void Reset()
    {
        if (phase == PHASE_FLIGHT)
            ClearIceBlock();

        phase = PHASE_NULL;
        events.Reset();
        instance->SetData(DATA_SAPPHIRON, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        me->CastSpell(me, SPELL_FROST_AURA, true);
        events.ScheduleEvent(EVENT_BERSERK, 15 * MINUTE * 1000);
        EnterPhaseGround();
        events.ScheduleEvent(EVENT_FLIGHT, 45 * 1000); ///TODO
        instance->SetData(DATA_SAPPHIRON, IN_PROGRESS);
    }
    
    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_ICEBOLT)
        {
            IceBlockMap::iterator itr = _iceblocks.find(target->GetGUID());
            if (itr != _iceblocks.end() && !itr->second)
            {
                if (GameObject* iceblock = me->SummonGameObject(GO_ICEBLOCK, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, 0, 0, 0, 0, 25000))
                    itr->second = iceblock->GetGUID();
            }
        }
    }

    void JustDied(Unit * killer)
    {
        me->CastSpell(me, SPELL_DIES, true);
        instance->SetData(DATA_SAPPHIRON, DONE);
        instance->SetData(DATA_SAPPHIRON_DIALOG, IN_PROGRESS);
        me->SummonCreature(NPC_THE_LICH_KING, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 40 * 1000);
    }

    /*void MovementInform(uint32 type, uint32 id)
    {
        if (id == 1)
            events.ScheduleEvent(EVENT_LIFTOFF, 0);
    }*/

    void DoAction(const int32 param)
    {
        if (param == DATA_SAPPHIRON_BIRTH)
        {
            phase = PHASE_BIRTH;
            events.ScheduleEvent(EVENT_BIRTH, 23 * 1000);
        }
        BossAI::DoAction(param);
    }

    void EnterPhaseGround()
    {
        phase = PHASE_GROUND;
        me->SetReactState(REACT_AGGRESSIVE);
        events.SetPhase(PHASE_GROUND);
        events.ScheduleEvent(EVENT_CLEAVE, urand(5, 15) * 1000, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_TAIL, urand(5, 15) * 1000, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_DRAIN, 24 * 1000, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_BLIZZARD, urand(5, 10) * 1000, 0, PHASE_GROUND);
    }

    void ClearIceBlock()
    {
        for (IceBlockMap::const_iterator itr = _iceblocks.begin(); itr != _iceblocks.end(); ++itr)
        {
            if (Player* player = Unit::GetPlayerInWorld(itr->first))
                player->RemoveAurasDueToSpell(SPELL_ICEBOLT);

            if (GameObject* go = GameObject::GetGameObject(*me, itr->second))
                go->Delete();
        }
        _iceblocks.clear();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!phase)
            return;

        events.Update(diff);

        if ((phase != PHASE_BIRTH && !UpdateVictim()))
            return;

        if (phase == PHASE_GROUND)
        {
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BERSERK:
                        DoScriptText(EMOTE_ENRAGE, me);
                        DoCast(me, SPELL_BERSERK);
                        return;
                    case EVENT_CLEAVE:
                        DoCastVictim(SPELL_CLEAVE);
                        events.ScheduleEvent(EVENT_CLEAVE, urand(5, 15) * 1000, 0, PHASE_GROUND);
                        return;
                    case EVENT_TAIL:
                        DoCastAOE(SPELL_TAIL_SWEEP);
                        events.ScheduleEvent(EVENT_TAIL, urand(5, 15) * 1000, 0, PHASE_GROUND);
                        return;
                    case EVENT_DRAIN:
                        DoCastAOE(SPELL_LIFE_DRAIN);
                        events.ScheduleEvent(EVENT_DRAIN, 24 * 1000, 0, PHASE_GROUND);
                        return;
                    case EVENT_BLIZZARD:
                    {
                        Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                        if (Creature* summon = me->SummonCreature(NPC_BLIZZARD, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 25 * 1000))
                        {
                            summon->setFaction(me->getFaction());
                            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            summon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                            summon->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);
                        }
                        events.ScheduleEvent(EVENT_BLIZZARD, urand(15000, 20000), 0, PHASE_GROUND);
                        break;
                    }
                    case EVENT_FLIGHT:
                        if ((me->GetHealthPercent()) > 10)
                        {
                            phase = PHASE_FLIGHT;
                            events.SetPhase(PHASE_FLIGHT);
                            float x, y, z, o;
                            me->GetHomePosition(x, y, z, o);
                            me->GetMotionMaster()->MovePoint(1, x, y, z);
                            events.ScheduleEvent(EVENT_LIFTOFF, 1500);
                            return;
                        }
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
        else
        {
            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_LIFTOFF:
                    {
                        me->MonsterTextEmote(-1200264, me->GetGUID(), true);
                        me->setHover(true);
                        me->SetLevitate(true);
                        me->SetRooted(true);
                        events.ScheduleEvent(EVENT_ICEBOLT, 1500);
                        _iceboltCount = 5;
                        return;
                    }
                    case EVENT_ICEBOLT:
                    {
                        if (Creature* blizz = GetClosestCreatureWithEntry(me, NPC_BLIZZARD, 100))
                            blizz->ForcedDespawn(0);
                        
                        std::vector<Unit*> targets;
                        std::list<HostileReference*>::const_iterator i = me->getThreatManager().getThreatList().begin();
                        for (; i != me->getThreatManager().getThreatList().end(); ++i)
                            if ((*i)->getTarget()->GetTypeId() == TYPEID_PLAYER && !(*i)->getTarget()->HasAura(SPELL_ICEBOLT))
                                targets.push_back((*i)->getTarget());

                        if (targets.empty())
                            _iceboltCount = 0;
                        else
                        {
                            std::vector<Unit*>::const_iterator itr = targets.begin();
                            advance(itr, rand()%targets.size());
                            _iceblocks.insert(std::make_pair((*itr)->GetGUID(), 0));
                            DoCast(*itr, SPELL_ICEBOLT);
                            --_iceboltCount;
                        }

                        if (_iceboltCount)
                            events.ScheduleEvent(EVENT_ICEBOLT, 1 * 1000);
                        else
                            events.ScheduleEvent(EVENT_BREATH, 1 * 1000);
                        return;
                    }
                    case EVENT_BREATH:
                    {
                        DoScriptText(EMOTE_BREATH, me);
                        DoCastAOE(SPELL_FROST_MISSILE);
                        events.ScheduleEvent(EVENT_EXPLOSION, 8 * 1000);
                        return;
                    }
                    case EVENT_EXPLOSION:
                    {
                        CastExplosion();
                        ClearIceBlock();
                        events.ScheduleEvent(EVENT_LAND, 3 * 1000);
                        return;
                    }
                    case EVENT_LAND:
                    {
                        me->MonsterTextEmote(-1200265, me->GetGUID(), true);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAND);
                        me->setHover(false);
                        me->SetLevitate(false);
                        me->SetRooted(false);
                        if (Unit* target = me->SelectNearestTarget(30.0f))
                            me->GetMotionMaster()->MoveChase(target);
                        events.ScheduleEvent(EVENT_GROUND, 1500);
                        return;
                    }
                    case EVENT_GROUND:
                    {
                        EnterPhaseGround();
                        events.ScheduleEvent(EVENT_FLIGHT, 67 * 1000);
                        return;
                    }
                    case EVENT_BIRTH:
                    {
                        me->SetVisibility(VISIBILITY_ON);
                        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                        me->SetReactState(REACT_AGGRESSIVE);
                        return;
                    }
                }
            }
        }
        CastNextSpellIfAnyAndReady();
    }

    void CastExplosion()
    {
        DoZoneInCombat(); // make sure everyone is in threatlist
        std::vector<Unit*> targets;
        std::list<HostileReference*>::const_iterator i = me->getThreatManager().getThreatList().begin();
        for (; i != me->getThreatManager().getThreatList().end(); ++i)
        {
            Unit* target = (*i)->getTarget();
            if (target->GetTypeId() != TYPEID_PLAYER)
                continue;

            if (target->HasAura(SPELL_ICEBOLT))
            {
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_FROST_EXPLOSION, true);
                target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_FROST_BREATH, true);
                targets.push_back(target);
                continue;
            }

            for (IceBlockMap::const_iterator itr = _iceblocks.begin(); itr != _iceblocks.end(); ++itr)
            {
                if (GameObject* go = GameObject::GetGameObject(*me, itr->second))
                {
                    if (go->IsInBetween(me, target, 2.0f)
                        && me->GetExactDist2d(target->GetPositionX(), target->GetPositionY()) - me->GetExactDist2d(go->GetPositionX(), go->GetPositionY()) < 5.0f)
                    {
                        target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_FROST_EXPLOSION, true);
                        target->ApplySpellImmune(0, IMMUNITY_ID, SPELL_FROST_BREATH, true);
                        targets.push_back(target);
                        break;
                    }
                }
            }
        }

        me->CastSpell(me, SPELL_FROST_EXPLOSION, true);

        for (std::vector<Unit*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
        {
            (*itr)->ApplySpellImmune(0, IMMUNITY_ID, SPELL_FROST_EXPLOSION, false);
            (*itr)->ApplySpellImmune(0, IMMUNITY_ID, SPELL_FROST_BREATH, false);
        }
    }
};

CreatureAI* GetAI_boss_sapphiron(Creature *_Creature)
{
    return new boss_sapphironAI (_Creature);
}

struct npc_blizzardAI : public ScriptedAI
{
    npc_blizzardAI(Creature *c) : ScriptedAI(c) { }

    uint32 ChillTimer;
    void Reset()
    {
        ChillTimer = 100;
        me->SetWalk(true);
        me->SetSpeed(MOVE_RUN, 0.8);
    }
    void UpdateAI(const uint32 diff)
    {
        if(ChillTimer < diff)
        {
            DoCastAOE(SPELL_CHILL, true);
            ChillTimer = 2200;
        }
        else
            ChillTimer -= diff;

        if (!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_blizzard(Creature *_Creature)
{
    return new npc_blizzardAI(_Creature);
}

enum LichKingEvents
{
    EVENT_CHECK_DIALOGUE        = 1,
    EVENT_SAY_SAPP_DIALOG1,
    EVENT_SAY_SAPP_DIALOG2_LICH,
    EVENT_SAY_SAPP_DIALOG3,
    EVENT_SAY_SAPP_DIALOG4_LICH,
    EVENT_SAY_SAPP_DIALOG5
};
struct npc_the_lich_kingAI : public ScriptedAI
{
    npc_the_lich_kingAI(Creature *c) : ScriptedAI(c)
    {
        instance = (c->GetInstanceData());
    }
    ScriptedInstance* instance;
    EventMap events;
    void Reset()
    {
        events.Reset();
        events.ScheduleEvent(EVENT_CHECK_DIALOGUE, 1000);
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CHECK_DIALOGUE:
                {
                    if(instance->GetData(DATA_SAPPHIRON_DIALOG) != DONE)
                    {
                        events.ScheduleEvent(EVENT_SAY_SAPP_DIALOG1, 1000);
                        me->SummonCreature(NPC_MOUTH_OF_KELTHUZAD, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 40 * 1000);
                    }
                    break;
                }
                case EVENT_SAY_SAPP_DIALOG1:
                {
                    if (Unit* Kel = FindCreature(NPC_MOUTH_OF_KELTHUZAD, 100, me))
                        Kel->MonsterYell(-1200266, 0, Kel->GetGUID());
                    events.ScheduleEvent(EVENT_SAY_SAPP_DIALOG2_LICH, 5000);
                    break;
                }
                case EVENT_SAY_SAPP_DIALOG2_LICH:
                {
                    me->MonsterYell(-1200267, 0, me->GetGUID());
                    events.ScheduleEvent(EVENT_SAY_SAPP_DIALOG3, 17000);
                    break;
                }
                case EVENT_SAY_SAPP_DIALOG3:
                {
                    if (Unit* Kel = FindCreature(NPC_MOUTH_OF_KELTHUZAD, 100, me))
                        Kel->MonsterYell(-1200268, 0, Kel->GetGUID());
                    events.ScheduleEvent(EVENT_SAY_SAPP_DIALOG4_LICH, 6000);
                    break;
                }
                case EVENT_SAY_SAPP_DIALOG4_LICH:
                {
                    me->MonsterYell(-1200269, 0, me->GetGUID());
                    events.ScheduleEvent(EVENT_SAY_SAPP_DIALOG5, 8000);
                    break;
                }
                case EVENT_SAY_SAPP_DIALOG5:
                {
                    if (Unit* Kel = FindCreature(NPC_MOUTH_OF_KELTHUZAD, 100, me))
                        Kel->MonsterYell(-1200270, 0, Kel->GetGUID());
                    instance->SetData(DATA_SAPPHIRON_DIALOG, DONE);
                    break;
                }
            }
        }
        if (!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_the_lich_king(Creature *_Creature)
{
    return new npc_the_lich_kingAI(_Creature);
}

void AddSC_boss_sapphiron()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_sapphiron";
    newscript->GetAI = &GetAI_boss_sapphiron;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_blizzard";
    newscript->GetAI = &GetAI_npc_blizzard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_the_lich_king";
    newscript->GetAI = &GetAI_npc_the_lich_king;
    newscript->RegisterSelf();
}
