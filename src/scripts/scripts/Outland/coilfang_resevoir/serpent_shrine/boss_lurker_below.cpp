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
SDName: Boss_Lurker_Below
SD%Complete: 90
SDComment: Submerged Spell Swimming in Circles, Check Timers
SDCategory: Coilfang Resevoir, Serpent Shrine Cavern
EndScriptData */

#include "precompiled.h"
#include "def_serpent_shrine.h"
#include "../../../special/simple_ai.h"
#include "Spell.h"

#define EMOTE_SPOUT "takes a deep breath."

#define SPELL_SPOUT_ANIM    42835
#define SPELL_SPOUT         37433
#define SPOUT_DIST  100

enum RotationType
{
    NOROTATE = 0,
    CLOCKWISE = 1,
    COUNTERCLOCKWISE = 2,
};

enum lurkerAdds
{
    MOB_COILFANG_GUARDIAN = 21873,
    MOB_COILFANG_AMBUSHER = 21865
};

float addPos[9][4] =
{
    { MOB_COILFANG_AMBUSHER, 61.4f,  -484.9f, -22.3f },   //MOVE_AMBUSHER_1 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 69.8f,  -480.9f, -21.5f },   //MOVE_AMBUSHER_2 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 7.2f,   -487.3f, -21.3f },   //MOVE_AMBUSHER_3 X, Y, Z
    { MOB_COILFANG_AMBUSHER, -5.9f,  -482.3f, -21.3f },   //MOVE_AMBUSHER_4 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 59.9f,  -360.9f, -21.3f },   //MOVE_AMBUSHER_5 X, Y, Z
    { MOB_COILFANG_AMBUSHER, 93.3f,  -387.5f, -21.5f },   //MOVE_AMBUSHER_6 X, Y, Z
    { MOB_COILFANG_GUARDIAN, -13.5f, -420.8f, -21.3f },   //MOVE_GUARDIAN_1 X, Y, Z
    { MOB_COILFANG_GUARDIAN, 49.7f,  -369.9f, -21.4f },   //MOVE_GUARDIAN_2 X, Y, Z
    { MOB_COILFANG_GUARDIAN, 92.1f,  -440.7f, -21.3f }    //MOVE_GUARDIAN_3 X, Y, Z
};

float addMovePos[9][3] =
{
    { 50.0f, -458.9f, -19.8f },   //MOVE_AMBUSHER_1 X, Y, Z
    { 62.7f, -456.1f, -19.8f },   //MOVE_AMBUSHER_2 X, Y, Z
    { 16.9f, -457.8f, -19.8f },   //MOVE_AMBUSHER_3 X, Y, Z
    { 6.1f,  -457.2f, -19.8f },   //MOVE_AMBUSHER_4 X, Y, Z
    { 64.1f, -379.4f, -19.7f },   //MOVE_AMBUSHER_5 X, Y, Z
    { 76.6f, -384.4f, -19.7f },   //MOVE_AMBUSHER_6 X, Y, Z
    { 11.2f, -427.5f, -19.7f },   //MOVE_GUARDIAN_1 X, Y, Z
    { 45.0f, -389.6f, -19.4f },   //MOVE_GUARDIAN_2 X, Y, Z
    { 63.9f, -432.1f, -19.5f }    //MOVE_GUARDIAN_3 X, Y, Z
};



enum lurkerSpells
{
    SPELL_SPOUT_VISUAL = 37429,
    SPELL_SPOUT_BREATH = 37431,
    SPELL_SPOUT_EFFECT = 37433,
    SPELL_GEYSER       = 37478,
    SPELL_WHIRL        = 37660,
    SPELL_WATERBOLT    = 37138,
    SPELL_SUBMERGE     = 37550,
    SPELL_SUBMERGED    = 37751,
    SPELL_EMERGE       = 20568
};

#define SPOUT_WIDTH 1.2f

enum LurkerEvents
{
    LURKER_EVENT_SPOUT_EMOTE    = 1,
    LURKER_EVENT_SPOUT          = 2,
    LURKER_EVENT_WHIRL          = 3,
    LURKER_EVENT_GEYSER         = 4,
    LURKER_EVENT_SUBMERGE       = 5,
    LURKER_EVENT_REEMERGE       = 6
};

struct boss_the_lurker_belowAI : public BossAI
{
    boss_the_lurker_belowAI(Creature *c) : BossAI(c, DATA_THELURKERBELOW) { 
        SpellEntry *TempSpell = (SpellEntry*)GetSpellStore()->LookupEntry<SpellEntry>(36151);
        if(TempSpell)
        {
            TempSpell->SpellVisual = 0;
            TempSpell->DurationIndex = 564;
        }
    
    }

    double SpoutAngle;
    uint8 RotType;
    uint32 RotTimer;
    uint32 SpoutAnimTimer;
    uint32 SpoutTimer;

    bool m_rotating;
    bool m_submerged;
    bool m_emoteDone;
    bool CanStartEvent;
    bool ConsecutiveSubmerge;

    uint32 m_checkTimer;
    uint32 WaitTimer;
    uint32 WaitTimer2;

    void Reset()
    {
        ClearCastQueue();
        events.Reset();

        instance->SetData(DATA_THELURKERBELOWEVENT, NOT_STARTED);
        instance->SetData(DATA_STRANGE_POOL, NOT_STARTED);

        // Do not fall to the ground ;]
        me->AddUnitMovementFlag(MOVEFLAG_SWIMMING);
        me->SetLevitate(true);

        // Set reactstate to: Defensive
        me->SetReactState(REACT_DEFENSIVE);
        me->SetVisibility(VISIBILITY_OFF);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);

        events.ScheduleEvent(LURKER_EVENT_SPOUT_EMOTE, 42000);
        events.ScheduleEvent(LURKER_EVENT_WHIRL, 18000);
        events.ScheduleEvent(LURKER_EVENT_GEYSER, rand()%5000 + 15000);
        events.ScheduleEvent(LURKER_EVENT_SUBMERGE, 90000);

        RotType = NOROTATE;
        SpoutAngle = 0;
        SpoutAnimTimer = 1000;
        RotTimer = 20000;
        SpoutTimer = 15000;
        m_checkTimer = 3000;
       
        m_rotating = false;
        m_submerged = true;
        ConsecutiveSubmerge = false;

        //Time values here is irrelevant, they just need to be set
        WaitTimer = 60000;
        WaitTimer2 = 60000;
        CanStartEvent = false;

        summons.DespawnAll();
        me->CastSpell(me, SPELL_SUBMERGE, false);
    }

    void Rotate(const uint32 diff)
    {
        bool Spout = false;
        switch (RotType)
        {
        case NOROTATE:
            return;
        case CLOCKWISE://20secs for 360turn
            //no target if rotating!
            m_rotating = true;
            me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
            SpoutAngle += (double)diff/20000*(double)M_PI*2;
            if (SpoutAngle >= M_PI*2)SpoutAngle = 0;
            me->SetOrientation(SpoutAngle);
            me->SendHeartBeat();
            me->StopMoving();
            Spout = true;
            break;
        case COUNTERCLOCKWISE://20secs for 360turn
            //no target if rotating!
            m_rotating = true;
            me->SetUInt64Value(UNIT_FIELD_TARGET, 0);
            SpoutAngle -= (double)diff/20000*(double)M_PI*2;
            if (SpoutAngle <= 0)SpoutAngle = M_PI*2;
            me->SetOrientation(SpoutAngle);
            me->SendHeartBeat();
            me->StopMoving();
            Spout = true;
            break;
        }

        if(!Spout)
            return;

        if(RotTimer<diff)//end rotate
        {
            RotType = NOROTATE;//set norotate state
            me->ClearUnitState(UNIT_STAT_ROTATING);
            RotTimer=20000;
            me->SetReactState(REACT_AGGRESSIVE);
            m_rotating = false;
            me->InterruptNonMeleeSpells(false);
            events.RescheduleEvent(LURKER_EVENT_WHIRL, 4000); //whirl directly after spout ends
            return;
        }else RotTimer-=diff;

        if(SpoutAnimTimer<diff)
        {
            DoCast(me,SPELL_SPOUT_ANIM,true);
            SpoutAnimTimer = 1000;
        }else SpoutAnimTimer-=diff;

        Map *map = me->GetMap();
        Map::PlayerList const &PlayerList = map->GetPlayers();
        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            //Player *target = i->getSource();            
            if (i->getSource() && !i->getSource()->IsSwimming() && (i->getSource()->GetPositionZ() >= -21.0f)) {
                if (i->getSource()->isAlive() && !i->getSource()->HasAura(36151) && me->HasInArc((double)diff / 20000 * (double)M_PI * 2, i->getSource()) && (me->GetDistance(i->getSource()) <= SPOUT_DIST)) {
                    //Remove Immunity to Spout spell, if set
                    i->getSource()->ApplySpellImmune(0, IMMUNITY_ID, 37433, false);

                    DoCast(i->getSource(), SPELL_SPOUT, true);//only knock back palyers in arc, in 100yards, not in water
                    DoCast(i->getSource(), 36151, true);
                }
            }
            else {
                //Since target is invalid, apply Immunity to Spout spell, to avoid being caught by it because a target nearby gets hit by spout
                i->getSource()->ApplySpellImmune(0, IMMUNITY_ID, 37433, true);
            }
                
        }

    }

    void StartRotate(Unit* victim)
    {
        switch (rand()%2)
        {
        case 0: RotType = CLOCKWISE; break;
        case 1: RotType = COUNTERCLOCKWISE; break;
        }
        RotTimer=20000;

        if(victim)
            SpoutAngle = me->GetOrientationTo(victim);

        me->StopMoving();
        me->addUnitState(UNIT_STAT_ROTATING);
        me->SetReactState(REACT_PASSIVE);
        m_rotating = true;

        events.RescheduleEvent(LURKER_EVENT_WHIRL, 20000);
        events.RescheduleEvent(LURKER_EVENT_GEYSER, rand()%25000 + 30000); // Geysir can´t come while spout
        events.ScheduleEvent(LURKER_EVENT_SPOUT_EMOTE, 45000);

        me->MonsterTextEmote(EMOTE_SPOUT,0,true);
        //DoCast(me,SPELL_SPOUT_BREATH);//take breath anim
    }

    void EnterCombat(Unit *who)
    {        
        instance->SetData(DATA_THELURKERBELOWEVENT, IN_PROGRESS);
        me->SetReactState(REACT_AGGRESSIVE);        
        AttackStart(who);
    }

    void AttackStart(Unit *pWho)
    {
        if (!pWho || !me->HasReactState(REACT_AGGRESSIVE))
            return;

        if (m_creature->Attack(pWho, true))
            DoStartNoMovement(pWho);
    }

    void MoveInLineOfSight(Unit *pWho)
    {
        if (me->GetVisibility() == VISIBILITY_OFF || me->IsInCombat())
            return;

        AttackStart(pWho);
    }

    void JustDied(Unit* killer)
    {
        instance->SetData(DATA_THELURKERBELOWEVENT, DONE);
        me->RemoveAurasDueToSpell(SPELL_SUBMERGE);
        me->SetVisibility(VISIBILITY_ON);
        me->NearTeleportTo(36.48, -416.07, -21.83, me->GetOrientation());
    }

    void SummonAdds()
    {
        for (uint8 i = 0; i < 9; i++)
        {
            Creature *pSummon = me->SummonCreature(addPos[i][0], addPos[i][1], addPos[i][2], addPos[i][3], 0, TEMPSUMMON_DEAD_DESPAWN, 2000);
            if (pSummon)
                pSummon->GetMotionMaster()->MovePoint(1, addMovePos[i][0], addMovePos[i][1], addMovePos[i][2]);
        }
    }


    void DoMeleeAttackIfReady()
    {
        if (me->HasUnitState(UNIT_STAT_CASTING) || m_submerged || m_rotating  || RotType)
            return;

        //Make sure our attack is ready and we aren't currently casting before checking distance
        if (me->isAttackReady())
        {
            //If we are within range melee the target
            if (me->IsWithinMeleeRange(me->GetVictim()))
            {
                me->AttackerStateUpdate(me->GetVictim());
                me->resetAttackTimer();
            }
            else
            {
                if (Unit *pTarget = SelectUnit(SELECT_TARGET_TOPAGGRO, 0, 5.0f, true))
                {
                    me->AttackerStateUpdate(pTarget);
                    me->resetAttackTimer();
                }
                else
                {
                    if (Unit *pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true, 0, 5.0f))
                        AddSpellToCast(pTarget, SPELL_WATERBOLT);
                    else
                        EnterEvadeMode();
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (instance->GetData(DATA_STRANGE_POOL) != IN_PROGRESS)
            return;

        //boss is invisible, don't attack
        if (!CanStartEvent)
        {            
                if (m_submerged)
                {
                    m_submerged = false;
                    WaitTimer2 = 500;
                }
                
                //wait 500ms before emerge anim
                if (!m_submerged && WaitTimer2 <= diff)
                {
                    me->SetVisibility(VISIBILITY_ON);
                    me->RemoveAllAuras();
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, 0);                    
                    DoCast(me, SPELL_EMERGE, false);
                    WaitTimer2 = 60000;//never reached
                    WaitTimer = 3000;
                }
                else
                    WaitTimer2 -= diff;

                //wait 3secs for emerge anim, then attack
                if (WaitTimer <= diff)
                {
                    //fresh fished from pool
                    WaitTimer = 3000;                    
                    CanStartEvent = true;
                    me->RemoveAurasDueToSpell(SPELL_SUBMERGE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    DoZoneInCombat();

                    if (ConsecutiveSubmerge)
                    {
                        events.RescheduleEvent(LURKER_EVENT_WHIRL, 2000);
                        events.RescheduleEvent(LURKER_EVENT_GEYSER, urand(5000, 15000));
                        events.RescheduleEvent(LURKER_EVENT_SUBMERGE, 90000);
                    }
                }
                else
                    WaitTimer -= diff;
            return;
        }

        if (!UpdateVictim())
            return;

        DoSpecialThings(diff, DO_PULSE_COMBAT);

        Rotate(diff);//always check rotate things

        events.Update(diff);

        if(!m_submerged && RotType == NOROTATE)//is not spouting and not submerged
        {
            if(SpoutTimer < diff)
            {
                if(me->GetVictim() && RotType == NOROTATE)
                    StartRotate(me->GetVictim());//start spout and random rotate

                SpoutTimer= 35000;
                return;
            } else SpoutTimer -= diff;
        }

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {    
            case LURKER_EVENT_WHIRL:
                {
                    if (m_submerged == false) {
                        AddSpellToCast(me, SPELL_WHIRL);
                    }
                    events.RescheduleEvent(LURKER_EVENT_WHIRL, 18000);                    
                    break;
                }
            case LURKER_EVENT_GEYSER:
                {
                    AddSpellToCast(SPELL_GEYSER, CAST_RANDOM);
                    events.ScheduleEvent(LURKER_EVENT_GEYSER, urand(15000, 20000));
                    break;
                }
            case LURKER_EVENT_SUBMERGE:
                {
                    ForceSpellCast(me, SPELL_SUBMERGE, INTERRUPT_AND_CAST_INSTANTLY);

                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                    me->SetVisibility(VISIBILITY_OFF);

                    SummonAdds();
                    m_submerged = true;
                    
                    // directly cast Spout after emerging!                    
                    SpoutTimer = 4000; 
                    events.CancelEvent(LURKER_EVENT_WHIRL);
                    events.CancelEvent(LURKER_EVENT_GEYSER);
                    events.ScheduleEvent(LURKER_EVENT_REEMERGE, 60000);
                    break;
                }
            case LURKER_EVENT_REEMERGE:
                {
                    me->SetVisibility(VISIBILITY_OFF);
                    DoStopAttack();

                    //Time values here is irrelevant, they just need to be set
                    WaitTimer = 60000;
                    WaitTimer2 = 60000;
                    CanStartEvent = false;
                    m_submerged = true;
                    ConsecutiveSubmerge = true;                    
                    break;
                }
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

enum guardianSpells
{
    SPELL_HARMSTRING   = 9080,
    SPELL_ARCING_SMASH = 28168
};

struct mob_coilfang_guardianAI : public ScriptedAI
{
    mob_coilfang_guardianAI(Creature *c) : ScriptedAI(c) { }

    uint32 m_harmstringTimer;
    uint32 m_arcingTimer;

    void Reset()
    {
        ClearCastQueue();

        m_harmstringTimer = urand(5000, 15000);
        m_arcingTimer = urand(15000, 20000);
    }

    void JustRespawned()
    {
        DoZoneInCombat(200.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_harmstringTimer < diff)
        {
            AddSpellToCast(SPELL_HARMSTRING);
            m_harmstringTimer = 10500;
        }
        else
            m_harmstringTimer -= diff;

        if (m_arcingTimer < diff)
        {
            AddSpellToCast(SPELL_ARCING_SMASH);
            m_arcingTimer = urand(10000, 20000);
        }
        else
            m_arcingTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

enum ambusherSpells
{
    SPELL_SPREAD_SHOT = 37790,
    SPELL_NORMAL_SHOT = 37770
};

struct mob_coilfang_ambusherAI : public Scripted_NoMovementAI
{
    mob_coilfang_ambusherAI(Creature *c) : Scripted_NoMovementAI(c) { }

    uint32 m_spreadTimer;
    uint32 m_shootTimer;

    void Reset()
    {
        ClearCastQueue();

        m_spreadTimer = urand(10000, 20000);
        m_shootTimer = 2000;
    }

    void JustRespawned()
    {
        DoZoneInCombat(200.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (m_spreadTimer < diff)
        {
            AddSpellToCast(SPELL_SPREAD_SHOT, CAST_RANDOM);
            m_spreadTimer = urand(10000, 20000);
        }
        else
            m_spreadTimer -= diff;

        if (m_shootTimer < diff)
        {
            AddSpellToCast(SPELL_NORMAL_SHOT, CAST_RANDOM);
            m_shootTimer = 2000;
        }
        else
            m_shootTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_coilfang_guardian(Creature* pCreature)
{
    return new mob_coilfang_guardianAI (pCreature);
}

CreatureAI* GetAI_mob_coilfang_ambusher(Creature* pCreature)
{
    return new mob_coilfang_ambusherAI (pCreature);
}

CreatureAI* GetAI_boss_the_lurker_below(Creature* pCreature)
{
    return new boss_the_lurker_belowAI (pCreature);
}

void AddSC_boss_the_lurker_below()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_the_lurker_below";
    newscript->GetAI = &GetAI_boss_the_lurker_below;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilfang_guardian";
    newscript->GetAI = &GetAI_mob_coilfang_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_coilfang_ambusher";
    newscript->GetAI = &GetAI_mob_coilfang_ambusher;
    newscript->RegisterSelf();
}