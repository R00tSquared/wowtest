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
SDName: Boss_Ouro
SD%Complete: 65%, no scarab summons, no enrage, no dirt mounds when submerged, no boulders when enraged
SDComment: No model for submerging. Currently just invisible.
SDCategory: Temple of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_temple_of_ahnqiraj.h"

enum
{
    // ground spells
    SPELL_SWEEP             = 26103,
    SPELL_SANDBLAST         = 26102,
    SPELL_BOULDER           = 26616,
    SPELL_BERSERK           = 26615,

    // emerge spells
    SPELL_BIRTH             = 26262,                        // The Birth Animation
    SPELL_GROUND_RUPTURE    = 26100,                        // spell not confirmed
    // SPELL_SUMMON_BASE       = 26133,                        // summons gameobject 180795

    // submerge spells
    SPELL_SUBMERGE_VISUAL   = 26063,
    SPELL_SUMMON_OURO_MOUND = 26058,                        // summons 5 dirt mounds
    SPELL_SUMMON_TRIGGER    = 26284,

    SPELL_SUMMON_OURO       = 26642,
    SPELL_QUAKE             = 26093,

    // other spells - not used
    //SPELL_SUMMON_SCARABS    = 26060,                        // triggered after 30 secs - cast by the Dirt Mounds - non-casted, those are summoned by summoncreature
    // SPELL_DIRTMOUND_PASSIVE = 26092,                     // casts 26093 every 1 sec - removed from DBC
    // SPELL_SET_OURO_HEALTH   = 26075,                     // removed from DBC
    // SPELL_SAVE_OURO_HEALTH  = 26076,                     // removed from DBC
    // SPELL_TELEPORT_TRIGGER  = 26285,                     // removed from DBC
    // SPELL_SUBMERGE_TRIGGER  = 26104,                     // removed from DBC
    // SPELL_SUMMON_OURO_MOUND = 26617,                     // removed from DBC
    // SPELL_SCARABS_PERIODIC  = 26619,                     // cast by the Dirt Mounds in order to spawn the scarabs - removed from DBC

    // summoned npcs
    NPC_OURO                = 15517,
    // NPC_OURO_SCARAB       = 15718,                       // summoned by Dirt Mounds
    NPC_OURO_TRIGGER        = 15717,
    NPC_DIRT_MOUND          = 15712,                        // summoned also by missing spell 26617
    NPC_SCARAB              = 15718
};

struct boss_ouroAI : public Scripted_NoMovementAI
{
    boss_ouroAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature), Summons(me)
    {
        pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
    }

    ScriptedInstance* pInstance;
    SummonList Summons;

    uint32 SweepTimer;
    uint32 SandBlastTimer;
    uint32 SubmergeTimer;
    uint32 SummonBaseTimer;
    uint32 SummonMoundTimer;
    uint32 MergedOutTimer;
    uint64 m_ouroTriggerGuid;

    bool Enraged;
    bool Submerged;

    void Reset()
    {
        SweepTimer        = urand(35000, 40000);
        SandBlastTimer    = urand(30000, 45000);
        SubmergeTimer     = 90000;
        SummonBaseTimer   = 1000;
        SummonMoundTimer  = 10000;
        MergedOutTimer    = 10000;

        Enraged            = false;
        Submerged          = false;
        Summons.DespawnAll();
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        DoZoneInCombat();
        if (pInstance)
            pInstance->SetData(DATA_OURO, IN_PROGRESS);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (Submerged)
            damage = 0; // no damage when submerged, should not happen at all
    }

    void JustReachedHome()
    {
        if (pInstance)
            pInstance->SetData(DATA_OURO, FAIL);

        me->ForcedDespawn();
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (pInstance)
            pInstance->SetData(DATA_OURO, DONE);
        Summons.DespawnAll();
    }

    void JustSummoned(Creature* pSummoned)
    {
        Summons.Summon(pSummoned);
        switch(pSummoned->GetEntry())
        {
            case NPC_OURO_TRIGGER:
                m_ouroTriggerGuid = pSummoned->GetGUID();
                pSummoned->SetSpeed(MOVE_WALK, 3, true);
                pSummoned->SetSpeed(MOVE_RUN, 3, true);
                pSummoned->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                pSummoned->GetMotionMaster()->MoveRandomAroundPoint(pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 40.0f);
                break;
        }
    }

    bool GetPlayer()
    {
        Player* pPlayer = NULL;
        Hellground::AnyPlayerInObjectRangeCheck p_check(me, 10.0f);
        Hellground::ObjectSearcher<Player, Hellground::AnyPlayerInObjectRangeCheck> searcher(pPlayer, p_check);

        Cell::VisitAllObjects(me, searcher, 10.0f);
        return pPlayer;
    }

    void UpdateAI(const uint32 diff)
    {
        // Summon sandworm base
        if (SummonBaseTimer)
        {
            if (SummonBaseTimer <= diff)
            {
                me->SummonGameObject(180795, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, 0, 0, 0, 0, 0);
                std::list<Player*> playerList = FindAllPlayersInRange(2);
                if (!playerList.empty())
                {
                    for(std::list<Player*>::iterator i = playerList.begin(); i != playerList.end(); i++)
                        DoCast((*i), SPELL_GROUND_RUPTURE, true);
                }
                SummonBaseTimer = 0;
            }
            else
                SummonBaseTimer -= diff;
        }

        // Return since we have no pTarget
        if (!UpdateVictim())
            return;

        if (!Submerged)
        {
            if (MergedOutTimer < diff)
            {
                if (!me->IsWithinMeleeRange(me->GetVictim()))
                {
                    if (!GetPlayer()) // no players near at all - go submerge
                        SubmergeTimer = 1000; // go submerge
                    else
                        DoModifyThreatPercent(me->GetVictim(), -100); // drop aggro from this target - wont affect tanks due to MergedOutTimer
                }
                MergedOutTimer = 2000; // Wowwiki says: "It will also submerge if no player is in melee range of it.", so I hope 2sec enough.
            }
            else
                MergedOutTimer -= diff;

            // Sweep
            if (SweepTimer < diff)
            {
                AddSpellToCast(SPELL_SWEEP, CAST_SELF);
                SweepTimer = 20000;
                MergedOutTimer = 10000;
            }
            else
                SweepTimer -= diff;

            // Sand Blast
            if (SandBlastTimer < diff)
            {
                AddSpellToCast(SPELL_SANDBLAST, CAST_SELF);
                SandBlastTimer = 22000;
                if (SubmergeTimer < 5000) // don't submerge for the next 5 sec (stun duration) to allow tank to run away when submerging
                    SubmergeTimer = 5000;
            }
            else
                SandBlastTimer -= diff;

            if (!Enraged)
            {
                // Enrage at 20% HP
                if (me->GetHealthPercent() < 20.0f)
                {
                    AddSpellToCast(SPELL_BERSERK, CAST_SELF);
                    Enraged = true;
                    return;
                }

                // Submerge
                if (SubmergeTimer < diff)
                {
                    AddSpellToCast(SPELL_SUBMERGE_VISUAL, CAST_SELF);
                    AddSpellToCast(SPELL_SUMMON_OURO_MOUND, CAST_SELF, true);
                    AddSpellToCast(SPELL_SUMMON_TRIGGER, CAST_SELF, true);

                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);

                    if (GameObject* SandwormBase = FindGameObject(180795, 50.0f, me))
                        SandwormBase->RemoveFromWorld();
                    
                    Submerged = true;
                    SubmergeTimer = 30000;
                }
                else
                    SubmergeTimer -= diff;
            }
            else
            {
                // Summon 1 mound every 10 secs when enraged
                if (SummonMoundTimer < diff)
                {
                    DoSpawnCreature(NPC_DIRT_MOUND, 0, 0, 0, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                    SummonMoundTimer = 10000;
                }
                else
                    SummonMoundTimer -= diff;
            }

            // If we are within range melee the target
            if (me->IsWithinMeleeRange(me->GetVictim()))
                DoMeleeAttackIfReady();
            // Spam Boulder spell when enraged and not tanked
            else if (Enraged)
            {
                if (!GetPlayer())
                {
                    if (!me->IsNonMeleeSpellCast(false))
                        DoCast(me->GetVictim(), SPELL_BOULDER); // it's said in tactics, that this is like in ragnaros encounter - thus boulder on victim, not random targets
                }
                else
                    DoModifyThreatPercent(me->GetVictim(), -100); // not in range with target, but there are other targets in range - try to target them
            }
        }
        else
        {
            // Resume combat
            if (SubmergeTimer < diff)
            {
                // Teleport to the trigger in order to get a new location
                if (Creature* pTrigger = me->GetMap()->GetCreature(m_ouroTriggerGuid))
                    me->NearTeleportTo(pTrigger->GetPositionX(), pTrigger->GetPositionY(), pTrigger->GetPositionZ(), 0);
                std::list<Creature*> DirtMounds = FindAllCreaturesWithEntry(NPC_DIRT_MOUND, 150);
                if (!DirtMounds.empty())
                {
                    for(std::list<Creature *>::iterator i = DirtMounds.begin(); i != DirtMounds.end(); i++)
                    {
                        uint8 cnt = HeroicMode ? urand(1, 2) : 3;
                        for(int j = 0; j < cnt; j++)
                        {
                            if(Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true))
                            {
                                Creature* Scarab = me->SummonCreature(NPC_SCARAB, (*i)->GetPositionX(), (*i)->GetPositionY(), (*i)->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 45000);
                                if(Scarab)
                                {
                                    Scarab->AI()->AttackStart(pTarget);
                                    Scarab->AddThreat(pTarget, 10000000.0f); // Bind to the target
                                    Scarab->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                                    Scarab->ApplySpellImmune(0, IMMUNITY_EFFECT,SPELL_EFFECT_ATTACK_ME, true);
                                }
                            }
                        }
                        (*i)->DisappearAndDie();
                    }
                }

                AddSpellToCast(SPELL_BIRTH, CAST_SELF);
                me->RemoveAurasDueToSpell(SPELL_SUBMERGE_VISUAL);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

                Submerged = false;
                SummonBaseTimer = 1000;
                SubmergeTimer = 90000;
                MergedOutTimer = 10000;
            }
            else
                SubmergeTimer -= diff;
        }
        CastNextSpellIfAnyAndReady();
        //DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ouro(Creature* pCreature)
{
    return new boss_ouroAI(pCreature);
}

struct npc_ouro_spawnerAI : public Scripted_NoMovementAI
{
    npc_ouro_spawnerAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) {Reset();}

    uint32 QuakeTimer;
    bool HasSummoned;

    void Reset()
    {
        QuakeTimer = 1000;
        HasSummoned = false;
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        // Spawn Ouro on LoS check
        if (!HasSummoned && pWho->GetTypeId() == TYPEID_PLAYER && !((Player*)pWho)->isGameMaster() && me->IsWithinDistInMap(pWho, 50.0f))
        {
            DoCast(me, SPELL_SUMMON_OURO, true);
            HasSummoned = true;
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void JustSummoned(Creature* pSummoned)
    {
        // Despanw when Ouro is spawned
        if (pSummoned->GetEntry() == NPC_OURO)
        {
            pSummoned->CastSpell(pSummoned, SPELL_BIRTH, false);
            pSummoned->SetInCombatWithZone();
            me->ForcedDespawn();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (HasSummoned)
        {
            if (QuakeTimer < diff)
            {
                DoCast(me, SPELL_QUAKE, true);
                QuakeTimer = 1000;
            }
            else
                QuakeTimer -= diff;
        }
    }
};

CreatureAI* GetAI_npc_ouro_spawner(Creature* pCreature)
{
    return new npc_ouro_spawnerAI(pCreature);
}

struct npc_dirt_moundAI : public ScriptedAI
{
    npc_dirt_moundAI(Creature* pCreature) : ScriptedAI(pCreature)
    { }

    uint32 QuakeTimer;
    uint32 targetChangeTimer;
    bool isRooted;

    void Reset()
    {
        QuakeTimer = 2000;
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_NOT_SELECTABLE);
        me->setFaction(14);
        targetChangeTimer = 2000;
        isRooted = true;
        me->SetRooted(true);
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        DoZoneInCombat(); // to fill list of targets to use SelectUnit later
    }

    void UpdateAI(const uint32 diff)
    {
        if (QuakeTimer < diff)
        {
            DoCast(me, SPELL_QUAKE, true);
            QuakeTimer = 1000;
        }
        else
            QuakeTimer -= diff;

        if (targetChangeTimer < diff)
        {
            if (Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200, true))
            {
                DoResetThreat();
                me->AI()->AttackStart(pTarget);
                me->AddThreat(pTarget, 100000);
            }
            targetChangeTimer = 5000;
            if (isRooted)
                me->SetRooted(false);
        }
        else
            targetChangeTimer -= diff;
    }
};

CreatureAI* GetAI_npc_dirt_mound(Creature* pCreature)
{
    return new npc_dirt_moundAI(pCreature);
}

void AddSC_boss_ouro()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name= "boss_ouro";
    newscript->GetAI = &GetAI_boss_ouro;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ouro_spawner";
    newscript->GetAI = &GetAI_npc_ouro_spawner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ouro_dirt_mound";
    newscript->GetAI = &GetAI_npc_dirt_mound;
    newscript->RegisterSelf();
}

