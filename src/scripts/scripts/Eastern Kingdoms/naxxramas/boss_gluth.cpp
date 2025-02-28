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
SDName: Boss_Gluth
SD%Complete: 99
SDComment: Debug, Probably fix spells
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "def_naxxramas.h"

enum
{
    EMOTE_ZOMBIE                    = -1533119,
    EMOTE_BOSS_GENERIC_ENRAGED      = -1000006,
    EMOTE_DECIMATE                  = -1533152,

    SPELL_MORTALWOUND               = 25646,
    SPELL_DECIMATE                  = 28374,
    SPELL_ENRAGE                    = 28371,
    SPELL_BERSERK                   = 26662,
    SPELL_TERRIFYING_ROAR           = 29685,
    // SPELL_SUMMON_ZOMBIE_CHOW      = 28216,               // removed from dbc: triggers 28217 every 6 secs
    // SPELL_CALL_ALL_ZOMBIE_CHOW    = 29681,               // removed from dbc: triggers 29682
    // SPELL_ZOMBIE_CHOW_SEARCH      = 28235,               // removed from dbc: triggers 28236 every 3 secs

    NPC_ZOMBIE_CHOW                 = 16360,                // old vanilla summoning spell 28217

    MAX_ZOMBIE_LOCATIONS            = 3,
};

static const float aZombieSummonLoc[MAX_ZOMBIE_LOCATIONS][3] =
{
    {3267.9f, -3172.1f, 297.42f},
    {3253.2f, -3132.3f, 297.42f},
    {3308.3f, -3185.8f, 297.42f},
};

struct boss_gluthAI : public BossAI
{
    boss_gluthAI(Creature *c) : BossAI(c, DATA_GLUTH), Summons(me) {}

    uint32 MortalWoundTimer;
    uint32 DecimateTimer;
    uint32 EnrageTimer;
    uint32 RoarTimer;
    uint32 SummonTimer;
    uint32 ZombieSearchTimer;
    uint32 BerserkTimer;
    uint32 CheckTimer;
    std::list<uint64> ZombieChowGuidList;
    SummonList Summons;

    void Reset()
    {
        MortalWoundTimer  = 10000;
        DecimateTimer     = 110000;
        EnrageTimer       = 25000;
        SummonTimer       = 6000;
        RoarTimer         = 15000;
        ZombieSearchTimer = 3000;
        CheckTimer        = 5000;

        BerserkTimer      = MINUTE * 8 * 1000;

        instance->SetData(DATA_GLUTH, NOT_STARTED);
    }

    void EnterCombat(Unit *who)
    {
        instance->SetData(DATA_GLUTH, IN_PROGRESS);
    }

    void JustDied(Unit * killer)
    {
        instance->SetData(DATA_GLUTH, DONE);
        Summons.DespawnAll();
    }
    
    void EnterEvadeMode()
    {
        Summons.DespawnAll();
        ScriptedAI::EnterEvadeMode();
    }

    void KilledUnit(Unit * unit)
    {
        if (me->isAlive() && unit->GetTypeId() == TYPEID_UNIT && unit->GetEntry() == NPC_ZOMBIE_CHOW)
        {
            DoScriptText(EMOTE_ZOMBIE, me);
            me->ModifyHealth(me->GetMaxHealth()*0.05);  // if zombie was eaten grow hp by 5%
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->GetMotionMaster()->MoveFollow(me, 1.0f, 0);
        ZombieChowGuidList.push_back(pSummoned->GetObjectGuid());
        Summons.Summon(pSummoned);
    }

    void SummonedCreatureDespawn(Creature* pSummoned) override
    {
        Summons.DespawnAll();
        ZombieChowGuidList.remove(pSummoned->GetObjectGuid());
    }

    bool FindPlayersOnTop()
    {
        std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
        if(m_threatlist.empty())
            return false;

        for(std::list<HostileReference*>::iterator itr = m_threatlist.begin(); itr != m_threatlist.end(); ++itr)
        {
            Unit* pUnit = Unit::GetUnit((*me), (*itr)->getUnitGuid());
            if(pUnit && pUnit->IsInCombat() && me->canAttack(pUnit) && !pUnit->HasUnitMovementFlag(MOVEFLAG_FALLING) && !pUnit->HasUnitMovementFlag(MOVEFLAG_FALLINGFAR)
                && !pUnit->HasUnitMovementFlag(MOVEFLAG_SAFE_FALL) && !pUnit->HasUnitMovementFlag(MOVEFLAG_FLYING) && !pUnit->HasUnitMovementFlag(MOVEFLAG_PITCH_DOWN) && pUnit->GetPositionZ() > 315)
                return true;
        }
        return false;
    }

    // Replaces missing spell 29682
    void DoCallAllZombieChow()
    {
        for (std::list<uint64>::const_iterator itr = ZombieChowGuidList.begin(); itr != ZombieChowGuidList.end(); ++itr)
        {
            if (Creature* pZombie = me->GetMap()->GetCreature(*itr))
            {
                pZombie->GetMotionMaster()->MoveFollow(me, 1.0f, 0);
            }
        }
    }

    // Replaces missing spell 28236
    void DoSearchZombieChow()
    {
        for (std::list<uint64>::const_iterator itr = ZombieChowGuidList.begin(); itr != ZombieChowGuidList.end(); ++itr)
        {
            if (Creature* pZombie = me->GetMap()->GetCreature(*itr))
            {
                if (!pZombie->isAlive())
                    continue;

                // Devour a Zombie
                if (pZombie->IsWithinDistInMap(me, 15.0f))
                    me->DealDamage(pZombie, pZombie->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CheckTimer < diff)
        {
            if(FindPlayersOnTop())
            {
                EnterEvadeMode();
                return;
            }
            CheckTimer = 10000;
        }
        else
            CheckTimer -= diff;

        if (ZombieSearchTimer < diff)
        {
            DoSearchZombieChow();
            ZombieSearchTimer = 3000;
        }
        else
            ZombieSearchTimer -= diff;

        // Mortal Wound
        if (MortalWoundTimer < diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_MORTALWOUND);
            MortalWoundTimer = 10000;
        }
        else
            MortalWoundTimer -= diff;

        // Decimate
        if (DecimateTimer < diff)
        {
            AddSpellToCast(me, SPELL_DECIMATE);
            DoScriptText(EMOTE_DECIMATE, me);
            DoCallAllZombieChow();
            DecimateTimer = 100000;
        }
        else
            DecimateTimer -= diff;

        // Enrage
        if (EnrageTimer < diff)
        {
            AddSpellToCast(me, SPELL_ENRAGE);
            DoScriptText(EMOTE_BOSS_GENERIC_ENRAGED, me);
            EnrageTimer = urand(20000, 30000);
        }
        else
            EnrageTimer -= diff;

        // Terrifying Roar
        if (RoarTimer < diff)
        {
            AddSpellToCast(me, SPELL_TERRIFYING_ROAR);
            RoarTimer = 20000;
        }
        else
            RoarTimer -= diff;

        // Summon
        if (SummonTimer < diff)
        {
            uint8 uiPos1 = urand(0, MAX_ZOMBIE_LOCATIONS - 1);
            me->SummonCreature(NPC_ZOMBIE_CHOW, aZombieSummonLoc[uiPos1][0], aZombieSummonLoc[uiPos1][1], aZombieSummonLoc[uiPos1][2], 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);

            uint8 uiPos2 = (uiPos1 + urand(1, MAX_ZOMBIE_LOCATIONS - 1)) % MAX_ZOMBIE_LOCATIONS;
            me->SummonCreature(NPC_ZOMBIE_CHOW, aZombieSummonLoc[uiPos2][0], aZombieSummonLoc[uiPos2][1], aZombieSummonLoc[uiPos2][2], 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);

            SummonTimer = 6000;
        }
        else
            SummonTimer -= diff;

        // Berserk
        if (BerserkTimer < diff)
        {
            AddSpellToCast(me, SPELL_BERSERK);
            BerserkTimer = MINUTE * 5 * 1000;
        }
        else
            BerserkTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_gluth(Creature *_Creature)
{
    return new boss_gluthAI (_Creature);
}

void AddSC_boss_gluth()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_gluth";
    newscript->GetAI = &GetAI_boss_gluth;
    newscript->RegisterSelf();
}
