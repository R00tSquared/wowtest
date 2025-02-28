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
SDName: Bosses_Opera
SD%Complete: 90
SDComment: Oz, Hood, and RAJ event implemented. RAJ event requires more testing.
SDCategory: Karazhan
EndScriptData */

#include "precompiled.h"
#include "def_karazhan.h"

struct boss_operaAI : public ScriptedAI
{
    boss_operaAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        evade = false;
        AggroTimer = 0;
        YellTimer = 0;
    }

    ScriptedInstance* pInstance;

    Timer AggroTimer;
    Timer YellTimer;

    bool evade;
    bool eventStarted;

    void Reset()
    {
        ClearCastQueue();
        eventStarted = false;
        if (!pInstance)
            pInstance = me->GetInstanceData();
    }

    void AttackStart(Unit* who)
    {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            return;

        ScriptedAI::MoveInLineOfSight(who);
    }

    void DoAction(const int32 action)
    {
        switch(action)
        {
            case 0:
                eventStarted = true;
                //DoZoneInCombat();
                break;
            default:
                break;
        }
    }

    void EnterEvadeMode()
    {
        if (!eventStarted || AggroTimer.GetInterval())
            return;

        evade = true;

        ScriptedAI::EnterEvadeMode();

        me->Kill(me, false);
        me->RemoveCorpse();
    }

    void JustRespawned()
    {
        DoZoneInCombat();
    }

    void UpdateAI(const uint32 diff)
    {
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

/***********************************/
/*** OPERA WIZARD OF OZ EVENT *****/
/*********************************/

#define SAY_DOROTHEE_DEATH          -1532025
#define SAY_DOROTHEE_SUMMON         -1532026
#define SAY_DOROTHEE_TITO_DEATH     -1532027
#define SAY_DOROTHEE_AGGRO          -1532028

#define SAY_ROAR_AGGRO              -1532029
#define SAY_ROAR_DEATH              -1532030
#define SAY_ROAR_SLAY               -1532031

#define SAY_STRAWMAN_AGGRO          -1532032
#define SAY_STRAWMAN_DEATH          -1532033
#define SAY_STRAWMAN_SLAY           -1532034

#define SAY_TINHEAD_AGGRO           -1532035
#define SAY_TINHEAD_DEATH           -1532036
#define SAY_TINHEAD_SLAY            -1532037
#define EMOTE_RUST                  -1532038

#define SAY_CRONE_AGGRO             -1532039
#define SAY_CRONE_AGGRO2            -1532040
#define SAY_CRONE_DEATH             -1532041
#define SAY_CRONE_SLAY              -1532042

/**** Spells ****/
// Dorothee
#define SPELL_WATERBOLT         31012
#define SPELL_SCREAM            31013
#define SPELL_SUMMONTITO        31014

// Tito
#define SPELL_YIPPING           31015

// Strawman
#define SPELL_BRAIN_BASH        31046
#define SPELL_BRAIN_WIPE        31069
#define SPELL_BURNING_STRAW     31075

// Tinhead
#define SPELL_CLEAVE            31043
#define SPELL_RUST              31086

// Roar
#define SPELL_MANGLE            31041
#define SPELL_SHRED             31042
#define SPELL_FRIGHTENED_SCREAM 31013

// Crone
#define SPELL_CHAIN_LIGHTNING   32337

// Cyclone
#define SPELL_KNOCKBACK         32334
#define SPELL_CYCLONE_VISUAL    32332

/** Creature Entries **/
#define CREATURE_TITO           17548
#define CREATURE_CYCLONE        18412
#define CREATURE_CRONE          18168

void SummonCroneIfReady(ScriptedInstance* pInstance, Creature *_Creature)
{
    if(!pInstance)
        return;

    pInstance->SetData(DATA_OPERA_OZ_DEATHCOUNT, 0);        // Increment DeathCount
    if(pInstance->GetData(DATA_OPERA_OZ_DEATHCOUNT) == 4)
    {
        if (Creature * barnes = _Creature->GetCreature(pInstance->GetData64(DATA_BARNES)))
            barnes->SummonCreature(CREATURE_CRONE,  -10893.8, -1758.7, 90.5, 4.64, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
    }
};

struct boss_dorotheeAI : public boss_operaAI
{
    boss_dorotheeAI(Creature* c) : boss_operaAI(c) {}

    Timer WaterBoltTimer;
    Timer FearTimer;
    Timer SummonTitoTimer;

    bool SummonedTito;
    bool TitoDied;

    void Reset()
    {
        AggroTimer.Reset(10000);
        YellTimer.Reset(1000);

        WaterBoltTimer.Reset(5000);
        FearTimer.Reset(15000);
        SummonTitoTimer.Reset(47500);

        SummonedTito = false;
        TitoDied = false;

        boss_operaAI::Reset();
    }

    void EnterCombat(Unit* who)
    {
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CASTING_SPEED, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
    }

    void SummonTito();                                      // See below

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_DOROTHEE_DEATH, me);

        if(evade)
            pInstance->SetData(DATA_OPERA_EVENT, NOT_STARTED);
        else
            SummonCroneIfReady(pInstance, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!eventStarted)
            return;

        if(YellTimer.Expired(diff))
        {
            DoScriptText(SAY_DOROTHEE_AGGRO, me);
            YellTimer = 0;
        }

        if (AggroTimer.Expired(diff))
        {
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            DoZoneInCombat();
            AggroTimer = 0;
        }

        if (!UpdateVictim())
            return;

        if (WaterBoltTimer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_WATERBOLT)))
            {
                me->SetOrientation(me->GetOrientationTo(target));
                AddSpellToCast(target, SPELL_WATERBOLT, false, true);
            }
            WaterBoltTimer = TitoDied ? 1500 : 5000;
        }

        if (FearTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SCREAM);
            FearTimer = 30000;
        }

        if (!SummonedTito)
        {
            if (SummonTitoTimer.Expired(diff))
                SummonTito();
        }

        boss_operaAI::UpdateAI(diff);
    }
};

struct mob_titoAI : public ScriptedAI
{
    mob_titoAI(Creature* c) : ScriptedAI(c) {}

    uint64 DorotheeGUID;

    Timer YipTimer;

    void Reset()
    {
        ClearCastQueue();

        DorotheeGUID = 0;

        YipTimer.Reset(10000);
    }

    void JustDied(Unit* killer)
    {
        if (DorotheeGUID)
        {
            Creature* Dorothee = (Unit::GetCreature((*me), DorotheeGUID));
            if (Dorothee && Dorothee->isAlive())
            {
                ((boss_dorotheeAI*)Dorothee->AI())->TitoDied = true;
                DoScriptText(SAY_DOROTHEE_TITO_DEATH, Dorothee);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        
        if (YipTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_YIPPING);
            YipTimer = 10000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

void boss_dorotheeAI::SummonTito()
{
    Creature* Tito = DoSpawnCreature(CREATURE_TITO, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 45000);
    if (Tito)
    {
        DoScriptText(SAY_DOROTHEE_SUMMON, me);
        ((mob_titoAI*)Tito->AI())->DorotheeGUID = me->GetGUID();
        Tito->AI()->AttackStart(me->GetVictim());
        SummonedTito = true;
        TitoDied = false;
    }
}

struct boss_strawmanAI : public boss_operaAI
{
    boss_strawmanAI(Creature* c) : boss_operaAI(c){}

    Timer BrainBashTimer;
    Timer BrainWipeTimer;

    void Reset()
    {
        AggroTimer.Reset(24000);
        YellTimer.Reset(23500);
        BrainBashTimer.Reset(5000);
        BrainWipeTimer.Reset(7000);

        boss_operaAI::Reset();
    }

    void SpellHit(Unit* caster, const SpellEntry *Spell)
    {
        if ((Spell->SchoolMask == SPELL_SCHOOL_MASK_FIRE) && (!(rand()%2)))
            DoCast(me, SPELL_BURNING_STRAW, true);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_STRAWMAN_DEATH, me);

        if (evade)
            pInstance->SetData(DATA_OPERA_EVENT, NOT_STARTED);
        else
            SummonCroneIfReady(pInstance, me);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(SAY_STRAWMAN_SLAY, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!eventStarted)
            return;

        if (AggroTimer.Expired(diff))
        {
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            DoZoneInCombat();
            AggroTimer = 0;
        }

        if (YellTimer.Expired(diff))
        {
            DoScriptText(SAY_STRAWMAN_AGGRO, me);
            YellTimer = 0;
        }

        if (!UpdateVictim())
            return;

        if (BrainBashTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_BRAIN_BASH);
            BrainBashTimer = 15000;
        }

        if (BrainWipeTimer.Expired(diff))
        {
            AddSpellToCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_BRAIN_WIPE, false, true);
            BrainWipeTimer = 20000;
        }

        boss_operaAI::UpdateAI(diff);
    }
};

struct boss_tinheadAI : public boss_operaAI
{
    boss_tinheadAI(Creature* c) : boss_operaAI(c){}

    Timer CleaveTimer;
    Timer RustTimer;

    uint8 RustCount;

    void Reset()
    {
        AggroTimer.Reset(33000);
        YellTimer.Reset(32500);
        CleaveTimer.Reset(5000);
        RustTimer.Reset(30000);

        RustCount = 0;

        me->SetSpeed(MOVE_WALK, 1);
        me->SetSpeed(MOVE_RUN, 1);

        boss_operaAI::Reset();
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_TINHEAD_DEATH, me);

        if (evade)
            pInstance->SetData(DATA_OPERA_EVENT, NOT_STARTED);
        else
            SummonCroneIfReady(pInstance, me);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(SAY_TINHEAD_SLAY, me);
    }

    void OnAuraRemove(Aura* aur, bool stackRemove)
    {
        if (aur->GetId() == SPELL_RUST)
        {
            me->SetSpeed(MOVE_WALK, 0.2);
            me->SetSpeed(MOVE_RUN, 0.2);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!eventStarted)
            return;

        if (AggroTimer.Expired(diff))
        {
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            DoZoneInCombat();
            AggroTimer = 0;
        }

        if(YellTimer.Expired(diff))
        {
            DoScriptText(SAY_TINHEAD_AGGRO, me);
            YellTimer = 0;
        }

        if (!UpdateVictim())
            return;

        if (CleaveTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_CLEAVE);
            CleaveTimer = 5000;
        }

        if (RustCount < 8)
        {
            if (RustTimer.Expired(diff))
            {
                RustCount++;
                AddSpellToCastWithScriptText(me, SPELL_RUST, EMOTE_RUST);
                RustTimer = 6000;
            }
        }

        boss_operaAI::UpdateAI(diff);
    }
};

struct boss_roarAI : public boss_operaAI
{
    boss_roarAI(Creature* c) : boss_operaAI(c){}

    Timer MangleTimer;
    Timer ShredTimer;
    Timer ScreamTimer;

    void Reset()
    {
        AggroTimer.Reset(14500);
        YellTimer.Reset(14000);
        MangleTimer.Reset(5000);
        ShredTimer.Reset(10000);
        ScreamTimer.Reset(15000);

        boss_operaAI::Reset();
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_ROAR_DEATH, me);

        if (evade)
            pInstance->SetData(DATA_OPERA_EVENT, NOT_STARTED);
        else
            SummonCroneIfReady(pInstance, me);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(SAY_ROAR_SLAY, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!eventStarted)
            return;

        if (AggroTimer.Expired(diff))
        {
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            DoZoneInCombat();
            AggroTimer = 0;
        }

        if (YellTimer.Expired(diff))
        {
            DoScriptText(SAY_ROAR_AGGRO, me);
            YellTimer = 0;
        }

        if (!UpdateVictim())
            return;

        if (MangleTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_MANGLE);
            MangleTimer = 5000 + rand()%3000;
        }

        if (ShredTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_SHRED);
            ShredTimer = 10000 + rand()%5000;
        }

        if (ScreamTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_FRIGHTENED_SCREAM);
            ScreamTimer = 20000 + rand()%10000;
        }

        boss_operaAI::UpdateAI(diff);
    }
};

struct boss_croneAI : public boss_operaAI
{
    boss_croneAI(Creature* c) : boss_operaAI(c){}

    Timer ChainLightningTimer;
    Timer checkTimer;

    void Reset()
    {
        ChainLightningTimer.Reset(10000);
        checkTimer.Reset(3000);
        AggroTimer.Reset(7000);

        boss_operaAI::Reset();
        eventStarted = true;

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, true);
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(RAND(SAY_CRONE_AGGRO, SAY_CRONE_AGGRO2), me);

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);

		me->SummonCreature(CREATURE_CYCLONE, -10907.68, -1778.651, 90.56018, 0.61, TEMPSUMMON_MANUAL_DESPAWN, 0);
		me->SummonCreature(CREATURE_CYCLONE, -10907.68, -1778.651, 90.56018, 0.61, TEMPSUMMON_MANUAL_DESPAWN, 0);
    }

    void KilledUnit(Unit* pWho)
    {
        DoScriptText(SAY_CRONE_SLAY, me);
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_CRONE_DEATH, me);

        if (pInstance)
            pInstance->SetData(DATA_OPERA_EVENT, evade ? NOT_STARTED : DONE);
    }

	void JustSummoned(Creature* pSummoned) override
	{
		pSummoned->CastSpell(pSummoned, SPELL_CYCLONE_VISUAL, true);
		pSummoned->GetMotionMaster()->MoveRandomAroundPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 15.0f);
	}

    void UpdateAI(const uint32 diff)
    {
        if(AggroTimer.Expired(diff))
        {
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
            DoZoneInCombat();
            AggroTimer = 0;
        }

        if (!UpdateVictim())
            return;

        if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

        if (ChainLightningTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_CHAIN_LIGHTNING);
            ChainLightningTimer = 15000;
        }
        boss_operaAI::UpdateAI(diff);
    }
};

float movePoints[9][3] = {
	{-10876.415039, -1749.697632, 90.476601},
	{-10890.368164, -1750.050903, 90.476601},
	{-10904.783203, -1746.908325, 90.477386},
	{-10905.068359, -1760.567261, 90.477386},
	{-10904.399414, -1772.713135, 90.477386},
	{-10891.674805, -1779.214355, 90.477386},
	{-10879.286133, -1772.299316, 90.477386},
	{-10886.995117, -1764.901123, 90.477386},
	{-10899.231445, -1765.158691, 90.477386}
};

uint32 movePoints_size = 9;

struct mob_cycloneAI : public ScriptedAI
{
    mob_cycloneAI(Creature* c) : ScriptedAI(c) 
    {
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance *pInstance;

	Timer MoveTimer;

	void Reset()
	{
		MoveTimer.Reset(1000);
	}

    void MoveInLineOfSight(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if (!me->HasAura(SPELL_KNOCKBACK, 0))
            DoCast(me, SPELL_KNOCKBACK, true);

        if (pInstance->GetData(DATA_OPERA_EVENT) == DONE || pInstance->GetData(DATA_OPERA_EVENT) == NOT_STARTED)
            me->ForcedDespawn(1000);

		if (MoveTimer.Expired(diff))
		{
			int random = urand(0, movePoints_size - 1);
			me->GetMotionMaster()->MovePoint(0, movePoints[random][0], movePoints[random][1], movePoints[random][2]);
			MoveTimer = 5000 + rand() % 15000;
		}
    }
};

CreatureAI* GetAI_boss_dorothee(Creature* _Creature)
{
    return new boss_dorotheeAI(_Creature);
}

CreatureAI* GetAI_boss_strawman(Creature* _Creature)
{
    return new boss_strawmanAI(_Creature);
}

CreatureAI* GetAI_boss_tinhead(Creature* _Creature)
{
    return new boss_tinheadAI(_Creature);
}

CreatureAI* GetAI_boss_roar(Creature* _Creature)
{
    return new boss_roarAI(_Creature);
}

CreatureAI* GetAI_boss_crone(Creature* _Creature)
{
    return new boss_croneAI(_Creature);
}

CreatureAI* GetAI_mob_tito(Creature* _Creature)
{
    return new mob_titoAI(_Creature);
}

CreatureAI* GetAI_mob_cyclone(Creature* _Creature)
{
    return new mob_cycloneAI(_Creature);
}

/**************************************/
/**** Opera Red Riding Hood Event ****/
/************************************/

/**** Yells for the Wolf ****/
#define SAY_WOLF_AGGRO                  -1532043
#define SAY_WOLF_SLAY                   -1532044
#define SAY_WOLF_HOOD                   -1532045
#define SOUND_WOLF_DEATH                9275                //Only sound on death, no text.

/**** Spells For The Wolf ****/
#define SPELL_LITTLE_RED_RIDING_HOOD    30768
#define SPELL_TERRIFYING_HOWL           30752
#define SPELL_WIDE_SWIPE                30761

#define GOSSIP_GRANDMA_1 16149
#define GOSSIP_GRANDMA_2 16150
#define GOSSIP_GRANDMA_3 16151

/**** The Wolf's Entry ****/
#define CREATURE_BIG_BAD_WOLF           17521

bool GossipHello_npc_grandmother(Player* player, Creature* _Creature)
{
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_GRANDMA_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(15001, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_grandmother(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_GRANDMA_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(15002, _Creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+2:
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_GRANDMA_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(15003, _Creature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+3:
        {
            ScriptedInstance * pInstance = _Creature->GetInstanceData();
            if (pInstance)
            {
                _Creature->SetVisibility(VISIBILITY_OFF);
                float x,y,z;
                _Creature->GetPosition(x,y,z);
                Creature * barnes = _Creature->GetCreature(pInstance->GetData64(DATA_BARNES));
                if (barnes)
                {
                    Creature* BigBadWolf = barnes->SummonCreature(CREATURE_BIG_BAD_WOLF, x, y, z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
                    if (BigBadWolf)
                    {
                        BigBadWolf->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                        BigBadWolf->AI()->AttackStart(player);
                    }
                    _Creature->setDeathState(JUST_DIED);
                }
            }
            break;
        }
    }

    return true;
}

struct boss_bigbadwolfAI : public boss_operaAI
{
    boss_bigbadwolfAI(Creature* c) : boss_operaAI(c) { eventStarted = true; }

    Timer ChaseTimer;
    Timer FearTimer;
    Timer SwipeTimer;
    Timer checkTimer;

    uint64 HoodGUID;
    float TempThreat;

    bool IsChasing;

    void Reset()
    {
        ChaseTimer.Reset(urand(6000, 8000));
        FearTimer.Reset(urand(5000, 25000));
        SwipeTimer.Reset(urand(5000, 25000));
        checkTimer.Reset(1000);

        HoodGUID = 0;
        TempThreat = 0;

        IsChasing = false;

        boss_operaAI::Reset();
        eventStarted = true;
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_WOLF_AGGRO, me);
    }

    void JustDied(Unit* killer)
    {
        DoPlaySoundToSet(me, SOUND_WOLF_DEATH);

        pInstance->SetData(DATA_OPERA_EVENT, evade ? NOT_STARTED : DONE);
    }

    void KilledUnit(Unit* pUnit)
    {
        DoScriptText(SAY_WOLF_SLAY, me, pUnit);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (checkTimer.Expired(diff))
        {
            DoZoneInCombat();
            checkTimer = 1000;
        }

        DoMeleeAttackIfReady();

        if (ChaseTimer.Expired(diff))
        {
            if (!IsChasing)
            {
				// select main tank
				//uint64 tank_guid = 0;
				
				//if (!me->getThreatManager().getThreatList().size() > 1)
				//{
				//	if (HostileReference* tank = me->getThreatManager().getThreatList().front())
				//		tank_guid = tank->getUnitGuid();
				//	else if (uint64 victim = me->getVictimGUID())
				//		tank_guid = victim;
				//}
				
				// select target except main tank
				if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_LITTLE_RED_RIDING_HOOD), true, me->getVictimGUID()))
                {
                    ForceSpellCastWithScriptText(target, SPELL_LITTLE_RED_RIDING_HOOD,SAY_WOLF_HOOD, INTERRUPT_AND_CAST_INSTANTLY, true, true);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);

                    TempThreat = DoGetThreat(target);
                    if (TempThreat)
                        DoModifyThreatPercent(target, -100);
                    HoodGUID = target->GetGUID();
					me->AI()->AttackStart(target);
                    me->AddThreat(target, 1000000.0f);
                    ChaseTimer = 20000;
                    IsChasing = true;
                }
            }
            else
            {
                IsChasing = false;
                Unit* target = Unit::GetUnit((*me), HoodGUID);
                if (target)
                {
                    HoodGUID = 0;
                    if (DoGetThreat(target))
                        DoModifyThreatPercent(target, -100);
                    me->AddThreat(target, TempThreat);
                    TempThreat = 0;
                }

                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT,SPELL_EFFECT_ATTACK_ME, false);

                ChaseTimer = 10000;
            }
        }

        if (IsChasing)
            return;

        if (FearTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_TERRIFYING_HOWL);
            FearTimer = urand(25000, 35000);
        }

        if (SwipeTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_WIDE_SWIPE);
            SwipeTimer = urand(25000, 30000);
        }

        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_boss_bigbadwolf(Creature* _Creature)
{
    return new boss_bigbadwolfAI(_Creature);
}

/**********************************************/
/******** Opera Romeo and Juliet Event *******/
/********************************************/

/**** Speech *****/
#define SAY_JULIANNE_AGGRO              -1532046
#define SAY_JULIANNE_ENTER              -1532047
#define SAY_JULIANNE_DEATH01            -1532048
#define SAY_JULIANNE_DEATH02            -1532049
#define SAY_JULIANNE_RESURRECT          -1532050
#define SAY_JULIANNE_SLAY               -1532051

#define SAY_ROMULO_AGGRO                -1532052
#define SAY_ROMULO_DEATH                -1532053
#define SAY_ROMULO_ENTER                -1532054
#define SAY_ROMULO_RESURRECT            -1532055
#define SAY_ROMULO_SLAY                 -1532056

/***** Spells For Julianne *****/
#define SPELL_BLINDING_PASSION          30890
#define SPELL_DEVOTION                  30887
#define SPELL_ETERNAL_AFFECTION         30878
#define SPELL_POWERFUL_ATTRACTION       30889
#define SPELL_DRINK_POISON              30907

/***** Spells For Romulo ****/
#define SPELL_BACKWARD_LUNGE            30815
#define SPELL_DARING                    30841
#define SPELL_DEADLY_SWATHE             30817
#define SPELL_POISON_THRUST             30822

/**** Other Misc. Spells ****/
#define SPELL_UNDYING_LOVE              30951
#define SPELL_RES_VISUAL                24171

/*** Misc. Information ****/
#define CREATURE_ROMULO             17533
#define ROMULO_X                    -10900
#define ROMULO_Y                    -1758

enum RAJPhase
{
    PHASE_JULIANNE      = 0,
    PHASE_ROMULO        = 1,
    PHASE_BOTH          = 2,
};

void PretendToDie(Creature* _Creature)
{
    _Creature->InterruptNonMeleeSpells(true);
    _Creature->RemoveAllAuras();
    _Creature->SetHealth(0);
    _Creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    _Creature->GetMotionMaster()->MovementExpired();
    _Creature->GetMotionMaster()->MoveIdle();
    _Creature->SetUInt32Value(UNIT_FIELD_BYTES_1,UNIT_STAND_STATE_DEAD);
};

void Resurrect(Creature* target)
{
    target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    target->SetHealth(target->GetMaxHealth(), true);
    target->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
    target->CastSpell(target, SPELL_RES_VISUAL, true);

    if (target->GetVictim())
    {
        target->GetMotionMaster()->MoveChase(target->GetVictim());
        target->AI()->AttackStart(target->GetVictim());
    }
    else
        target->GetMotionMaster()->Initialize();
};

struct boss_julianneAI : public boss_operaAI
{
    boss_julianneAI(Creature* c) : boss_operaAI(c)
    {
        EntryYellTimer = 5000;
        AggroTimer = 15000;
    }

    Timer EntryYellTimer;

    uint32 Phase;
    uint64 RomuloGUID;
    Timer BlindingPassionTimer;
    Timer DevotionTimer;
    Timer EternalAffectionTimer;
    Timer PowerfulAttractionTimer;
    Timer SummonRomuloTimer;
    Timer ResurrectTimer;
    Timer DrinkPoisonTimer;
    Timer ResurrectSelfTimer;

    bool IsFakingDeath;
    bool SummonedRomulo;
    bool RomuloDead;

    void Reset()
    {
//        if (RomuloGUID)
//        {
//            if (Creature* Romulo = me->GetCreature(RomuloGUID))
//            {
//                Romulo->SetVisibility(VISIBILITY_OFF);
//                Romulo->AI()->EnterEvadeMode();
//            }
//        }

        RomuloGUID = 0;
        Phase = PHASE_JULIANNE;

        BlindingPassionTimer.Reset(30000);
        DevotionTimer.Reset(15000);
        EternalAffectionTimer.Reset(25000);
        PowerfulAttractionTimer.Reset(5000);
        SummonRomuloTimer.Reset(10000);
        ResurrectTimer.Reset(10000);
        DrinkPoisonTimer = 0;
        ResurrectSelfTimer = 0;

        if (IsFakingDeath)
            Resurrect(me);

        IsFakingDeath = false;
        SummonedRomulo = false;
        RomuloDead = false;
        boss_operaAI::Reset();
    }

    void EnterCombat(Unit* who)
    {
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CASTING_SPEED, true);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
    }

    void SpellHit(Unit* caster, const SpellEntry *Spell)
    {
        if (Spell->Id == SPELL_DRINK_POISON)
        {
            DoScriptText(SAY_JULIANNE_DEATH01, me);
            DrinkPoisonTimer = 2500;
        }
    }

    void DamageTaken(Unit* done_by, uint32 &damage);

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_JULIANNE_DEATH02, me);
        pInstance->SetData(DATA_OPERA_EVENT, evade ? NOT_STARTED : DONE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void KilledUnit(Unit* victim)
    {
       DoScriptText(SAY_JULIANNE_SLAY, me);
    }

    void UpdateAI(const uint32 diff);
};

struct boss_romuloAI : public boss_operaAI
{
    boss_romuloAI(Creature* c) : boss_operaAI(c)
    {
//        EntryYellTimer = 8000;
//        AggroTimer = 15000;
    }

    uint64 JulianneGUID;

    uint32 Phase;

    Timer EntryYellTimer;
    Timer BackwardLungeTimer;
    Timer DaringTimer;
    Timer DeadlySwatheTimer;
    Timer PoisonThrustTimer;
    Timer ResurrectTimer;
    uint64 MainTargetGUID;

    bool JulianneDead;
    bool IsFakingDeath;

    void Reset()
    {
        JulianneGUID = 0;

        Phase = PHASE_ROMULO;

        BackwardLungeTimer.Reset(15000);
        DaringTimer.Reset(20000);
        DeadlySwatheTimer.Reset(25000);
        PoisonThrustTimer.Reset(10000);
        ResurrectTimer.Reset(10000);

        MainTargetGUID = 0;

        IsFakingDeath = false;
        JulianneDead = false;
        boss_operaAI::Reset();
        eventStarted = true;
    }

    void DamageTaken(Unit* done_by, uint32 &damage);

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_ROMULO_AGGRO, me);
        if (JulianneGUID)
        {
            Creature* Julianne = (Unit::GetCreature((*me), JulianneGUID));
            if (Julianne && Julianne->GetVictim())
            {
                me->AddThreat(Julianne->GetVictim(), 1.0f);
            }
        }
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(SAY_ROMULO_DEATH, me);

        if (pInstance)
            pInstance->SetData(DATA_OPERA_EVENT, evade ? NOT_STARTED : DONE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void KilledUnit(Unit* victim)
    {
        DoScriptText(SAY_ROMULO_SLAY, me);
    }

    void UpdateAI(const uint32 diff);
};

void boss_julianneAI::DamageTaken(Unit* done_by, uint32 &damage)
{
    if (damage < me->GetHealth())
        return;

    if (Phase == PHASE_JULIANNE)
    {
        damage = 0;

        if (IsFakingDeath)
            return;

        me->InterruptNonMeleeSpells(true);
        DoCast(me, SPELL_DRINK_POISON);

        IsFakingDeath = true;
        return;
    }

    if (Phase == PHASE_ROMULO)
    {
        error_log("SD2: boss_julianneAI: cannot take damage in PHASE_ROMULO, why was i here?");
        damage = 0;
        return;
    }

    if (Phase == PHASE_BOTH)
    {
        //if this is true then we have to kill romulo too
        if (RomuloDead)
        {
            if (Creature* Romulo = ((Creature*)Unit::GetUnit((*me), RomuloGUID)))
            {
                Romulo->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                Romulo->SetHealth(1);
                done_by->Kill(Romulo);
            }

            return;
        }

        //if not already returned, then romulo is alive and we can pretend die
        if (Creature* Romulo = ((Creature*)Unit::GetUnit((*me), RomuloGUID)))
        {
            PretendToDie(me);
            IsFakingDeath = true;
            ((boss_romuloAI*)Romulo->AI())->ResurrectTimer = 10000;
            ((boss_romuloAI*)Romulo->AI())->JulianneDead = true;
            damage = 0;
            return;
        }
    }
    error_log("SD2: boss_julianneAI: DamageTaken reach end of code, that should not happen.");
}

void boss_romuloAI::DamageTaken(Unit* done_by, uint32 &damage)
{
    if (damage < me->GetHealth())
        return;

    if (Phase == PHASE_ROMULO)
    {
        DoScriptText(SAY_ROMULO_DEATH, me);
        PretendToDie(me);
        IsFakingDeath = true;
        Phase = PHASE_BOTH;

        if (Creature* Julianne = ((Creature*)Unit::GetUnit((*me), JulianneGUID)))
        {
            ((boss_julianneAI*)Julianne->AI())->RomuloDead = true;
            ((boss_julianneAI*)Julianne->AI())->ResurrectSelfTimer = 10000;
        }

        damage = 0;
        return;
    }

    if (Phase == PHASE_BOTH)
    {
        if (JulianneDead)
        {
            if (Creature* Julianne = ((Creature*)Unit::GetUnit((*me), JulianneGUID)))
            {
                Julianne->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                Julianne->SetHealth(1);
                done_by->Kill(Julianne);
            }
            return;
        }

        if (Creature* Julianne = ((Creature*)Unit::GetUnit((*me), JulianneGUID)))
        {
            PretendToDie(me);
            IsFakingDeath = true;
            ((boss_julianneAI*)Julianne->AI())->ResurrectTimer = 10000;
            ((boss_julianneAI*)Julianne->AI())->RomuloDead = true;
            damage = 0;
            return;
        }
    }

    error_log("SD2: boss_romuloAI: DamageTaken reach end of code, that should not happen.");
}

void boss_julianneAI::UpdateAI(const uint32 diff)
{
    if (!eventStarted)
        return;

    if (EntryYellTimer.Expired(diff))
    {
        DoScriptText(SAY_JULIANNE_ENTER, me);
        EntryYellTimer = 0;
    }

    if (AggroTimer.Expired(diff))
    {
        DoScriptText(SAY_JULIANNE_AGGRO, me);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        me->setFaction(16);
        AggroTimer = 0;
        DoZoneInCombat();
        return;
    }

    //will do this 2secs after spell hit. this is time to display visual as expected
    if (DrinkPoisonTimer.Expired(diff))
    {
        PretendToDie(me);
        Phase = PHASE_ROMULO;
        SummonRomuloTimer = 10000;
        DrinkPoisonTimer = 0;
    }

    if (Phase == PHASE_ROMULO && !SummonedRomulo)
    {
        if (SummonRomuloTimer.Expired(diff))
        {
            Creature* Romulo = me->SummonCreature(CREATURE_ROMULO, ROMULO_X, ROMULO_Y, me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000);
            if (Romulo)
            {
                RomuloGUID = Romulo->GetGUID();
                ((boss_romuloAI*)Romulo->AI())->JulianneGUID = me->GetGUID();
                ((boss_romuloAI*)Romulo->AI())->Phase = PHASE_ROMULO;
                Romulo->setFaction(16);

                if (me->GetVictim())
                {
                    Romulo->AddThreat(me->GetVictim(), 0.0f);
                }

                Romulo->AI()->DoZoneInCombat();
            }
            SummonedRomulo = true;
        }
    }

    if (ResurrectSelfTimer.Expired(diff))
    {
        Resurrect(me);
        Phase = PHASE_BOTH;
        IsFakingDeath = false;

        if (me->GetVictim())
            AttackStart(me->GetVictim());

        ResurrectSelfTimer = 0;
        ResurrectTimer = 1000;
    }

    if (!UpdateVictim() || IsFakingDeath)
        return;

    if (RomuloDead)
    {   
        if (ResurrectTimer.Expired(diff))
        {
            Creature* Romulo = (Unit::GetCreature((*me), RomuloGUID));
            if (Romulo && ((boss_romuloAI*)Romulo->AI())->IsFakingDeath)
            {
                DoScriptText(SAY_JULIANNE_RESURRECT, me);
                Resurrect(Romulo);
                ((boss_romuloAI*)Romulo->AI())->IsFakingDeath = false;
                RomuloDead = false;
                ResurrectTimer = 10000;
            }
        }
        
    }

    if (BlindingPassionTimer.Expired(diff))
    {
        AddSpellToCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_BLINDING_PASSION, false, true);
        BlindingPassionTimer = 30000 + rand()%15000;
    }

    if (DevotionTimer.Expired(diff))
    {
        AddSpellToCast(me, SPELL_DEVOTION);
        DevotionTimer = 15000 + rand()%30000;
    }

    if (PowerfulAttractionTimer.Expired(diff))
    {
        AddSpellToCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_POWERFUL_ATTRACTION, false, true);
        PowerfulAttractionTimer = 5000 + rand()%25000;
    }

    if (EternalAffectionTimer.Expired(diff))
    {
        if (rand()%2 == 1 && SummonedRomulo)
        {
            Creature* Romulo = (Unit::GetCreature((*me), RomuloGUID));
            if (Romulo && Romulo->isAlive() && !RomuloDead)
                AddSpellToCast(Romulo, SPELL_ETERNAL_AFFECTION);
        }
        else
            AddSpellToCast(me, SPELL_ETERNAL_AFFECTION);

        EternalAffectionTimer = 45000 + rand()%15000;
    }

    boss_operaAI::UpdateAI(diff);
}

void boss_romuloAI::UpdateAI(const uint32 diff)
{
    if (!UpdateVictim() || IsFakingDeath)
        return;

    if (JulianneDead)
    {
        if (ResurrectTimer.Expired(diff))
        {
            Creature* Julianne = (Unit::GetCreature((*me), JulianneGUID));
            if (Julianne && ((boss_julianneAI*)Julianne->AI())->IsFakingDeath)
            {
                DoScriptText(SAY_ROMULO_RESURRECT, me);
                Resurrect(Julianne);
                ((boss_julianneAI*)Julianne->AI())->IsFakingDeath = false;
                JulianneDead = false;
                ResurrectTimer = 10000;
            }
        }
    }

    if (BackwardLungeTimer.Expired(diff))
    {
        Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 1, 200, true, me->getVictimGUID());
        if (target && !me->HasInArc(M_PI, target))
        {
            me->SetOrientation(me->GetOrientationTo(target));
            AddSpellToCast(target, SPELL_BACKWARD_LUNGE, false, true);
            BackwardLungeTimer = 15000 + rand()%15000;
        }
    }

    if (DaringTimer.Expired(diff))
    {
        AddSpellToCast(me, SPELL_DARING);
        DaringTimer = 20000 + rand()%20000;
    }

    if (DeadlySwatheTimer.Expired(diff))
    {
        AddSpellToCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_DEADLY_SWATHE, false, true);
        DeadlySwatheTimer = 15000 + rand()%10000;
    }

    if (PoisonThrustTimer.Expired(diff))
    {
        AddSpellToCast(me->GetVictim(), SPELL_POISON_THRUST);
        PoisonThrustTimer = 10000 + rand()%10000;
    }
    boss_operaAI::UpdateAI(diff);
}

CreatureAI* GetAI_boss_julianne(Creature* _Creature)
{
    return new boss_julianneAI(_Creature);
}

CreatureAI* GetAI_boss_romulo(Creature* _Creature)
{
    return new boss_romuloAI(_Creature);
}

/*######
# npc_barnesAI
######*/

//#define GOSSIP_READY        "I'm not an actor."

#define SAY_READY           -1200236
//#define SAY_OZ_INTRO1       "Finally, everything is in place. Are you ready for your big stage debut?"
#define OZ_GOSSIP1          16152
//#define SAY_OZ_INTRO2       "Don't worry, you'll be fine. You look like a natural!"
#define OZ_GOSSIP2          16153

//#define SAY_RAJ_INTRO1      "The romantic plays are really tough, but you'll do better this time. You have TALENT. Ready?"
//#define RAJ_GOSSIP1         "I've never been more ready."

struct Dialogue
{
    int32 textid;
    Timer timer;
};

static Dialogue OzDialogue[]=
{
    {-1532103, 6000},
    {-1532104, 18000},
    {-1532105, 9000},
    {-1532106, 15000}
};

static Dialogue HoodDialogue[]=
{
    {-1532107, 6000},
    {-1532108, 10000},
    {-1532109, 14000},
    {-1532110, 15000}
};

static Dialogue RAJDialogue[]=
{
    {-1532111, 5000},
    {-1532112, 7000},
    {-1532113, 14000},
    {-1532114, 14000}
};

// Entries and spawn locations for creatures in Oz event
float Spawns[6][2]=
{
    {17535, -10896},                                        // Dorothee
    {17546, -10891},                                        // Roar
    {17547, -10884},                                        // Tinhead
    {17543, -10902},                                        // Strawman
    {17603, -10892},                                        // Grandmother
    {17534, -10900},                                        // Julianne
};

float StageLocations[7][2]=
{
    {-10866.711, -1779.816},                                // Open door, begin walking (0)
    {-10876.882, -1779.115},                                // (1)
    {-10895.084, -1782.776},                                // Begin Speech after this (2)
    {-10876.882, -1779.115},                                // Resume walking (back to spawn point now) after speech (3)
    {-10866.711, -1779.816},                                // (4)
    {-10866.700, -1781.030}                                 // Summon mobs, open curtains, close door (5)
};

#define CREATURE_SPOTLIGHT  19525

#define SPELL_SPOTLIGHT     25824
#define SPELL_TUXEDO        32616

#define SPAWN_Z             90.5
#define SPAWN_Y             -1758
#define SPAWN_O             4.738

#define GO_OZ_BACKDROP      183442
#define GO_OZ_HAY           183496
#define GO_HOOD_BACKDROP    183491
#define GO_HOOD_TREE        183492
#define GO_HOOD_HOUSE       183493
#define GO_RAJ_BACKDROP     183443
#define GO_RAJ_MOON         183494
#define GO_RAJ_BALCONY      183495

#define BARNES_SOUND_APPLAUSE   9332
#define OPERA_WATCHER_ENTRY     16468

struct npc_barnesAI : public ScriptedAI
{
    npc_barnesAI(Creature* c) : ScriptedAI(c), operaAdds(me)
    {
        RaidWiped = false;
        pInstance = c->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint64 SpotlightGUID;

    int32 TalkCount;
    Timer TalkTimer;
    Timer WipeTimer;
    Timer WatchersEmotionTimer;
    uint32 Event;
    std::list<uint64> DecorationGUIDList;

    bool PerformanceReady;
    bool RaidWiped;
    bool IsTalking;

    SummonList operaAdds;

    void EnterEvadeMode()
    {
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        RaidWiped = true;

        ScriptedAI::EnterEvadeMode();
    }

    void Reset()
    {
        TalkCount = 0;
        TalkTimer.Reset(2000);
        WipeTimer.Reset(5000);
        WatchersEmotionTimer.Reset(0);

        PerformanceReady = false;
        IsTalking = false;

        operaAdds.DespawnAll();
        DecorationGUIDList.clear();

        if (pInstance && pInstance->GetData(DATA_OPERA_EVENT) != DONE)
        {
            Event = pInstance->GetData(DATA_OPERA_PERFORMANCE);

            pInstance->HandleGameObject(pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT), true);

            if (GameObject* Curtain = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_CURTAINS)))
                Curtain->SetGoState(pInstance->GetData(DATA_OPERA_EVENT) == DONE ? GO_STATE_ACTIVE : GO_STATE_READY);
        }

        me->RemoveAurasDueToSpell(SPELL_TUXEDO);
    }

    void JustSummoned(Creature * summon)
    {
        operaAdds.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature * summon)
    {
        operaAdds.Despawn(summon);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch(id)
            {
                case 1:
                {
                    DoPlaySoundToSet(me, BARNES_SOUND_APPLAUSE);

                    std::list<Creature*> WatcherEntryList;
                    std::list<Creature*> WatchersList;

                    WatchersList.clear();

                    WatcherEntryList.clear();
                    WatcherEntryList = FindAllCreaturesWithEntry(OPERA_WATCHER_ENTRY, 100.0f);

                    for(std::list<Creature*>::iterator iter = WatcherEntryList.begin(); iter != WatcherEntryList.end(); ++iter)
                        WatchersList.push_back(*iter);

                    if(!WatchersList.empty())
                    {
                        for(std::list<Creature*>::iterator i = WatchersList.begin(); i != WatchersList.end(); ++i)
                            (*i)->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
                    }
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MovePoint(id+1, StageLocations[id+1][0], StageLocations[id+1][1], SPAWN_Z);
                    break;
                }
                case 4:
                    PrepareEncounter();
                case 0:
                case 3:
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MovePoint(id+1, StageLocations[id+1][0], StageLocations[id+1][1], SPAWN_Z);
                    break;
                case 2:
                    IsTalking = true;
                    TalkCount = 0;

                    float x,y,z;
                    me->GetPosition(x, y, z);
                    if (Creature* Spotlight = me->SummonCreature(CREATURE_SPOTLIGHT, x, y, z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 50000))
                    {
                        if(Unit* OperaTrigger = FindCreature(18654, 100, me))
                        {
                            me->SetOrientation(me->GetOrientationTo(OperaTrigger)); // serverside
                            me->SetFacingTo(me->GetOrientationTo(OperaTrigger)); // clientside
                        }
                        Spotlight->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        Spotlight->CastSpell(Spotlight, SPELL_SPOTLIGHT, false);
                        SpotlightGUID = Spotlight->GetGUID();
                    }
                    break;
                case 5:
                    if(pInstance)
                    {
                        if (GameObject* Door = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
                            Door->SetGoState(GO_STATE_READY);

                        if (GameObject* Curtain = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_CURTAINS)))
                            Curtain->SetGoState(GO_STATE_ACTIVE);
                    }

                    operaAdds.DoAction(0, 0);

                    me->RemoveAurasDueToSpell(SPELL_TUXEDO);
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MoveTargetedHome();
                    break;
            }
        }
    }

    void Talk(uint32 count)
    {
        int32 text = 0;

        switch(Event)
        {
            case EVENT_OZ:
                if (OzDialogue[count].textid)
                     text = OzDialogue[count].textid;
                if(OzDialogue[count].timer.GetInterval())
                    TalkTimer = OzDialogue[count].timer;     // FIXME: this and 2 bellow; it will create a duplicate or just set pointers to the same memory area?
                if(count == 1)
                    WatchersEmotionTimer = 15000;
                else if(count == 2)
                    WatchersEmotionTimer = 5000;
                break;

            case EVENT_HOOD:
                if (HoodDialogue[count].textid)
                    text = HoodDialogue[count].textid;
                if(HoodDialogue[count].timer.GetInterval())
                    TalkTimer = HoodDialogue[count].timer;
                break;

            case EVENT_RAJ:
                 if (RAJDialogue[count].textid)
                     text = RAJDialogue[count].textid;
                if(RAJDialogue[count].timer.GetInterval())
                    TalkTimer = RAJDialogue[count].timer;
                if(count == 1)
                    WatchersEmotionTimer = 4000;
                else if(count == 2)
                    WatchersEmotionTimer = 11000;
                break;
        }

        if(text)
            DoScriptText(text, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if(IsTalking)
        {
            if (TalkTimer.Expired(diff))
            {
                if(TalkCount > 3)
                {
                    if (Creature* Spotlight = me->GetCreature(SpotlightGUID))
                    {
                        Spotlight->Kill(Spotlight, false);
                        Spotlight->RemoveCorpse();
                    }

                    DoPlaySoundToSet(me, BARNES_SOUND_APPLAUSE);

                    std::list<Creature*> WatcherEntryList;
                    std::list<Creature*> WatchersList;

                    WatchersList.clear();

                    WatcherEntryList.clear();
                    WatcherEntryList = FindAllCreaturesWithEntry(OPERA_WATCHER_ENTRY, 100.0f);

                    for(std::list<Creature*>::iterator iter = WatcherEntryList.begin(); iter != WatcherEntryList.end(); ++iter)
                        WatchersList.push_back(*iter);

                    if(!WatchersList.empty())
                    {
                        for(std::list<Creature*>::iterator i = WatchersList.begin(); i != WatchersList.end(); ++i)
                            (*i)->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
                    }

                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                    IsTalking = false;
                    me->GetMotionMaster()->MovePoint(3, StageLocations[3][0], StageLocations[3][1], SPAWN_Z);
                    return;
                }

                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                Talk(TalkCount++);
            }

            if(WatchersEmotionTimer.Expired(diff))
            {
                std::list<Creature*> WatcherEntryList;
                std::list<Creature*> WatchersList;

                WatchersList.clear();

                WatcherEntryList.clear();
                WatcherEntryList = FindAllCreaturesWithEntry(OPERA_WATCHER_ENTRY, 100.0f);
                for(std::list<Creature*>::iterator iter = WatcherEntryList.begin(); iter != WatcherEntryList.end(); ++iter)
                    WatchersList.push_back(*iter);

                if(!WatchersList.empty())
                {
                    for(std::list<Creature*>::iterator i = WatchersList.begin(); i != WatchersList.end(); ++i)
                        (*i)->HandleEmoteCommand(RAND(EMOTE_ONESHOT_EXCLAMATION, EMOTE_ONESHOT_COWER, EMOTE_ONESHOT_ROAR));
                }
                WatchersEmotionTimer = 0;
            }
        }

        if (PerformanceReady)
        {
            if (WipeTimer.Expired(diff))
            {
                if (operaAdds.empty())
                {
                    for (std::list<uint64>::iterator itr = DecorationGUIDList.begin(); !DecorationGUIDList.empty();)
                    {
                        if (GameObject* decorations = GameObject::GetGameObject(*me, *itr))
                            me->RemoveGameObject(decorations, true);
                        itr = DecorationGUIDList.erase(itr);
                    }
                    EnterEvadeMode();
                }

                WipeTimer = 2000;
            }
        }
    }

    void StartEvent()
    {
        if(!pInstance)
            return;

        pInstance->SetData(DATA_OPERA_EVENT, IN_PROGRESS);

        if (GameObject* Door = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
            Door->SetGoState(GO_STATE_ACTIVE);

        me->CastSpell(me, SPELL_TUXEDO, true);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        me->SetSpeed(MOVE_WALK, 1.0);
        me->SetSpeed(MOVE_RUN, 1.0);
        me->SetWalk(true);

        me->GetMotionMaster()->MovePoint(0, StageLocations[0][0], StageLocations[0][1], SPAWN_Z);

        DoSay(-1200236, LANG_UNIVERSAL, 0);

        switch (Event)
        {
            case EVENT_OZ:
                if(GameObject* OzBackdrop = me->SummonGameObject(GO_OZ_BACKDROP, -10890.9, -1744.06, 90.4765, -1.67552, 0, 0, 0.743145, -0.669131, 0))
                    DecorationGUIDList.push_back(OzBackdrop->GetGUID());
                if(GameObject* OperaHay1 = me->SummonGameObject(GO_OZ_HAY, -10909.5, -1764.15, 90.4768, -1.65806, 0, 0, 0.737277, -0.67559, 0))
                    DecorationGUIDList.push_back(OperaHay1->GetGUID());
                if(GameObject* OperaHay2 = me->SummonGameObject(GO_OZ_HAY, -10876,   -1761.53, 90.477,  -1.69297, 0, 0, 0.748956, -0.66262, 0))
                    DecorationGUIDList.push_back(OperaHay2->GetGUID());
                if(GameObject* OperaHay3 = me->SummonGameObject(GO_OZ_HAY, -10883,   -1754.79, 90.4778, -1.72788, 0, 0, 0.760406, -0.649448, 0))
                    DecorationGUIDList.push_back(OperaHay3->GetGUID());
                if(GameObject* OperaHay4 = me->SummonGameObject(GO_OZ_HAY, -10904,   -1754.05, 90.4777, -1.69297, 0, 0, 0.748956, -0.66262, 0))
                    DecorationGUIDList.push_back(OperaHay4->GetGUID());
                break;
            case EVENT_HOOD:
                if(GameObject* HoodBackdrop = me->SummonGameObject(GO_HOOD_BACKDROP, -10890.9, -1744.06, 90.4765, -1.67552, 0, 0, 0.743145, -0.669131, 0))
                    DecorationGUIDList.push_back(HoodBackdrop->GetGUID());
                if(GameObject* OperaTree1 = me->SummonGameObject(GO_HOOD_TREE, -10909.5, -1761.53, 90.4768, -1.65806, 0, 0, 0.737277, -0.67559, 0))
                    DecorationGUIDList.push_back(OperaTree1->GetGUID());
                if(GameObject* OperaTree2 = me->SummonGameObject(GO_HOOD_TREE, -10876,   -1754.79, 90.477, -1.69297, 0, 0, 0.748956, -0.66262, 0))
                    DecorationGUIDList.push_back(OperaTree2->GetGUID());
                if(GameObject* OperaTree3 = me->SummonGameObject(GO_HOOD_TREE, -10904,   -1754.05, 90.4777, -1.69297, 0, 0, 0.748956, -0.66262, 0))
                    DecorationGUIDList.push_back(OperaTree3->GetGUID());
                if(GameObject* OperaHouse = me->SummonGameObject(GO_HOOD_HOUSE, -10883,   -1754.79, 90.4778, -1.72788, 0, 0, 0.760406, -0.649448, 0))
                    DecorationGUIDList.push_back(OperaHouse->GetGUID());
                break;
            case EVENT_RAJ:
                if(GameObject* RaJBackdrop = me->SummonGameObject(GO_RAJ_BACKDROP, -10890.9, -1744.06, 90.4765, -1.67552, 0, 0, 0.743145, -0.669131, 0))
                    DecorationGUIDList.push_back(RaJBackdrop->GetGUID());
                if(GameObject* RaJMoon = me->SummonGameObject(GO_RAJ_MOON, -10904.7, -1745.92, 109.642, 4.71554, 0, 0, 0.705992, -0.70822, 0))
                    DecorationGUIDList.push_back(RaJMoon->GetGUID());
                if(GameObject* RaJBalcony = me->SummonGameObject(GO_RAJ_BALCONY, -10890.7, -1749.17, 90.4762, -1.67552, 0, 0, 0.743145, -0.669131, 0))
                    DecorationGUIDList.push_back(RaJBalcony->GetGUID());
                break;
            default:
                break;
        }
        
    }

    void PrepareEncounter()
    {
        debug_log("TSCR: Barnes Opera Event - Introduction complete - preparing encounter %d", Event);
        uint8 index = 0;
        uint8 count = 0;
        switch(Event)
        {
            case EVENT_OZ:
                index = 0;
                count = 4;
                break;

            case EVENT_HOOD:
                index = 4;
                count = index+1;
                break;

            case EVENT_RAJ:
                index = 5;
                count = index+1;
                break;
        }

        for( ; index < count; ++index)
        {
            uint32 entry = ((uint32)Spawns[index][0]);
            float PosX = Spawns[index][1];
            if (Creature* pCreature = me->SummonCreature(entry, PosX, SPAWN_Y, SPAWN_Z, SPAWN_O, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                pCreature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        }

        PerformanceReady = true;
        RaidWiped = false;
    }
};

CreatureAI* GetAI_npc_barnesAI(Creature* _Creature)
{
    npc_barnesAI* Barnes_AI = new npc_barnesAI(_Creature);

    return ((CreatureAI*)Barnes_AI);
}

bool GossipHello_npc_barnes(Player* player, Creature* _Creature)
{
    if(player->isGameMaster())
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16154), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16155), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16156), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        player->SEND_GOSSIP_MENU(8978, _Creature->GetGUID());
    }
    else
    {
        // Check for death of Moroes.
        ScriptedInstance* pInstance = (_Creature->GetInstanceData());
        if(pInstance && (pInstance->GetData(DATA_MOROES_EVENT) >= DONE && pInstance->GetData(DATA_OPERA_EVENT) != DONE))
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(OZ_GOSSIP1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if(!((npc_barnesAI*)_Creature->AI())->RaidWiped)
                player->SEND_GOSSIP_MENU(8970, _Creature->GetGUID());
            else
                player->SEND_GOSSIP_MENU(8975, _Creature->GetGUID());
        }
        else
            player->SEND_GOSSIP_MENU(30000, _Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_barnes(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(OZ_GOSSIP2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(8971, _Creature->GetGUID());
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            ((npc_barnesAI*)_Creature->AI())->StartEvent();
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->CLOSE_GOSSIP_MENU();
            ((npc_barnesAI*)_Creature->AI())->Event = 1;
            ((npc_barnesAI*)_Creature->AI())->StartEvent();
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->CLOSE_GOSSIP_MENU();
            ((npc_barnesAI*)_Creature->AI())->Event = 2;
            ((npc_barnesAI*)_Creature->AI())->StartEvent();
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->CLOSE_GOSSIP_MENU();
            ((npc_barnesAI*)_Creature->AI())->Event = 3;
            ((npc_barnesAI*)_Creature->AI())->StartEvent();
            break;
    }

    return true;
}

void AddSC_opera_event()
{
    Script* newscript;

    // Barnes
    newscript = new Script;
    newscript->GetAI = &GetAI_npc_barnesAI;
    newscript->Name = "npc_barnes";
    newscript->pGossipHello = &GossipHello_npc_barnes;
    newscript->pGossipSelect = &GossipSelect_npc_barnes;
    newscript->RegisterSelf();

    // Oz
    newscript = new Script;
    newscript->GetAI = &GetAI_boss_dorothee;
    newscript->Name = "boss_dorothee";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_strawman;
    newscript->Name = "boss_strawman";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_tinhead;
    newscript->Name = "boss_tinhead";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_roar;
    newscript->Name = "boss_roar";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_crone;
    newscript->Name = "boss_crone";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_tito;
    newscript->Name = "mob_tito";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_mob_cyclone;
    newscript->Name = "mob_cyclone";
    newscript->RegisterSelf();

    // Hood
    newscript = new Script;
    newscript->pGossipHello = &GossipHello_npc_grandmother;
    newscript->pGossipSelect = &GossipSelect_npc_grandmother;
    newscript->Name = "npc_grandmother";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_bigbadwolf;
    newscript->Name = "boss_bigbadwolf";
    newscript->RegisterSelf();

    // Romeo And Juliet
    newscript = new Script;
    newscript->GetAI = &GetAI_boss_julianne;
    newscript->Name = "boss_julianne";
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_romulo;
    newscript->Name = "boss_romulo";
    newscript->RegisterSelf();
}
