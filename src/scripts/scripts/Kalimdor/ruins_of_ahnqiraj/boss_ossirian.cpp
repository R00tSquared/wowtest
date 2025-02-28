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
SDName: Boss_Ossirian
SD%Complete: 100%
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_ruins_of_ahnqiraj.h"

enum
{
    SAY_SUPREME_1           = -1509018,
    SAY_SUPREME_2           = -1509019,
    SAY_SUPREME_3           = -1509020,
    SAY_RAND_INTRO_1        = -1509021,
    SAY_RAND_INTRO_2        = -1509023,
    SAY_RAND_INTRO_3        = -1509024,
    SAY_AGGRO               = -1509025,
    SAY_SLAY                = -1509026,
    SAY_DEATH               = -1509027,

    SPELL_SILENCE           = 25195,
    SPELL_CYCLONE           = 25189,
    SPELL_STOMP             = 25188,
    SPELL_SUPREME           = 25176,
    SPELL_SAND_STORM        = 25160,                        // tornado spell
    SPELL_SUMMON            = 20477,                        // TODO NYI

    MAX_CRYSTAL_POSITIONS   = 1,                            // TODO

    SPELL_WEAKNESS_FIRE     = 25177,
    SPELL_WEAKNESS_FROST    = 25178,
    SPELL_WEAKNESS_NATURE   = 25180,
    SPELL_WEAKNESS_ARCANE   = 25181,
    SPELL_WEAKNESS_SHADOW   = 25183,

    NPC_SAND_VORTEX         = 15428,                        // tornado npc

    ZONE_ID_RUINS_AQ        = 3429,

    GO_OSSIRIAN_CRYSTAL     = 180619,
    NPC_OSSIRIAN_TRIGGER    = 15590,  // Triggers ossirian weakness
};

static const float aSandVortexSpawnPos[2][4] =
{
    { -9523.482f, 1880.435f, 85.645f, 5.08f},
    { -9321.39f,  1822.968f, 84.266f, 3.16f},
};

static const float aCrystalSpawnPos[3] = { -9355.75f, 1905.43f, 85.55f};
static const uint32 aWeaknessSpell[] = {SPELL_WEAKNESS_FIRE, SPELL_WEAKNESS_FROST, SPELL_WEAKNESS_NATURE, SPELL_WEAKNESS_ARCANE, SPELL_WEAKNESS_SHADOW};

struct boss_ossirianAI : public ScriptedAI
{

    boss_ossirianAI(Creature* pCreature) : ScriptedAI(pCreature), summons(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
        SaidIntro = false;
    }

    instance_ruins_of_ahnqiraj* pInstance;
    SummonList summons;

    uint32 SupremeTimer;
    uint32 CycloneTimer;
    uint32 StompTimer;
    uint32 SilenceTimer;
    uint8 CrystalPosition;

    bool SaidIntro;

    void Reset()
    {
        CrystalPosition = 0;
        CycloneTimer = 20000;
        StompTimer   = 30000;
        SilenceTimer = 30000;
        SupremeTimer = 45000;
    }

    void EnterEvadeMode()
    {
        summons.DespawnAll();
        std::list<GameObject*> OssirianCrystalList;
        Hellground::AllGameObjectsWithEntryInGrid go_check(GO_OSSIRIAN_CRYSTAL);
        Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher(OssirianCrystalList, go_check);
        Cell::VisitGridObjects(me, searcher, 300);
        
        if (!OssirianCrystalList.empty())
        {
            for (std::list<GameObject*>::const_iterator i = OssirianCrystalList.begin(); i != OssirianCrystalList.end(); ++i)
            {
                if (GameObject* go = (*i)->ToGameObject())
                    go->RemoveFromWorld();
            }
        }
        CreatureAI::EnterEvadeMode();
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        DoCast(me, SPELL_SUPREME, true);
        DoScriptText(SAY_AGGRO, me);
        DoSpawnNextCrystal();

        for (uint8 i = 0; i < 2; ++i)
            me->SummonCreature(NPC_SAND_VORTEX, aSandVortexSpawnPos[i][0], aSandVortexSpawnPos[i][1], aSandVortexSpawnPos[i][2], aSandVortexSpawnPos[i][3], TEMPSUMMON_CORPSE_DESPAWN, 0);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        DoScriptText(SAY_DEATH, me);
    }

    void KilledUnit(Unit* /*pVictim*/)
    {
        DoScriptText(SAY_SLAY, me);
    }

    void DoSpawnNextCrystal()
    {
        if (!pInstance)
            return;

        Creature* pOssirianTrigger = NULL;
        if (CrystalPosition == 0)
        {
            // Summon at static position 1st
            pOssirianTrigger = me->SummonCreature(NPC_OSSIRIAN_TRIGGER, -9427.219, 1969.089, 85.575302, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
        }
        else
        {
            // Summon a new crystal trigger at some position depending on CrystalPosition
            // Note: the summon points seem to be very random; requires additional research
            float fX, fY, fZ;
            me->GetRandomPoint(aCrystalSpawnPos[0], aCrystalSpawnPos[1], aCrystalSpawnPos[2], 50.0f, fX, fY, fZ);
            pOssirianTrigger = me->SummonCreature(NPC_OSSIRIAN_TRIGGER, fX, fY, fZ, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
        }
        if (!pOssirianTrigger)
            return;

        // Increase position
        ++CrystalPosition;
    }

    void JustSummoned(Creature* pSummoned)
    {
        summons.Summon(pSummoned);
        if (pSummoned->GetEntry() == NPC_OSSIRIAN_TRIGGER)
            pSummoned->SummonGameObject(GO_OSSIRIAN_CRYSTAL, pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 0, 0, 0, 0, 0, 0);
        else if (pSummoned->GetEntry() == NPC_SAND_VORTEX)
        {
            // The movement of this isn't very clear - may require additional research
            pSummoned->CastSpell(pSummoned, SPELL_SAND_STORM, true);
            pSummoned->GetMotionMaster()->MoveRandomAroundPoint(aCrystalSpawnPos[0], aCrystalSpawnPos[1], aCrystalSpawnPos[2], 100.0f);
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (pCaster->GetTypeId() == TYPEID_UNIT && pCaster->GetEntry() == NPC_OSSIRIAN_TRIGGER)
        {
            switch(pSpell->Id)
            {
                case SPELL_WEAKNESS_FIRE:
                {
                    me->AddAura(SPELL_WEAKNESS_FIRE, me);
                    me->SetResistance(SpellSchools(SPELL_SCHOOL_FIRE), 0);
                    break;
                }
                case SPELL_WEAKNESS_FROST:
                {
                    me->AddAura(SPELL_WEAKNESS_FROST, me);
                    me->SetResistance(SpellSchools(SPELL_SCHOOL_FROST), 0);
                    break;
                }
                case SPELL_WEAKNESS_NATURE:
                {
                    me->AddAura(SPELL_WEAKNESS_NATURE, me);
                    me->SetResistance(SpellSchools(SPELL_SCHOOL_NATURE), 0);
                    break;
                }
                case SPELL_WEAKNESS_ARCANE:
                {
                    me->AddAura(SPELL_WEAKNESS_ARCANE, me);
                    me->SetResistance(SpellSchools(SPELL_SCHOOL_ARCANE), 0);
                    break;
                }
                case SPELL_WEAKNESS_SHADOW:
                {
                    me->AddAura(SPELL_WEAKNESS_SHADOW, me);
                    me->SetResistance(SpellSchools(SPELL_SCHOOL_SHADOW), 0);
                    break;
                }
                default: break;
            }

            // Check for proper spell id
            bool bIsWeaknessSpell = false;
            for (uint8 i = 0; i < 5; ++i)
            {
                if (pSpell->Id == aWeaknessSpell[i])
                {
                    bIsWeaknessSpell = true;
                    break;
                }
            }

            if (!bIsWeaknessSpell)
                return;

            me->RemoveAurasDueToSpell(SPELL_SUPREME);
            SupremeTimer = 45000;

            ((Creature*)pCaster)->ForcedDespawn();
            DoSpawnNextCrystal();
        }
    }

    void OnAuraRemove(Aura* aur, bool stackRemove)
    {
        switch(aur->GetId())
        {
            case SPELL_WEAKNESS_FIRE:
            {
                me->SetResistance(SpellSchools(SPELL_SCHOOL_FIRE), 500);
                break;
            }
            case SPELL_WEAKNESS_FROST:
            {
                me->SetResistance(SpellSchools(SPELL_SCHOOL_FROST), 500);
                break;
            }
            case SPELL_WEAKNESS_NATURE:
            {
                me->SetResistance(SpellSchools(SPELL_SCHOOL_NATURE), 500);
                break;
            }
            case SPELL_WEAKNESS_ARCANE:
            {
                me->SetResistance(SpellSchools(SPELL_SCHOOL_ARCANE), 500);
                break;
            }
            case SPELL_WEAKNESS_SHADOW:
            {
                me->SetResistance(SpellSchools(SPELL_SCHOOL_SHADOW), 500);
                break;
            }
            default: break;
        }
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        // TODO: Range guesswork
        if (!SaidIntro && pWho->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(pWho, 75.0f, false))
        {
            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_RAND_INTRO_1, me); break;
                case 1: DoScriptText(SAY_RAND_INTRO_2, me); break;
                case 2: DoScriptText(SAY_RAND_INTRO_3, me); break;
            }
            SaidIntro = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        // Supreme
        if (SupremeTimer <= diff)
        {
            AddSpellToCast(SPELL_SUPREME, CAST_SELF);
            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_SUPREME_1, me); break;
                case 1: DoScriptText(SAY_SUPREME_2, me); break;
                case 2: DoScriptText(SAY_SUPREME_3, me); break;
            }
            SupremeTimer = 45000;
        }
        else
            SupremeTimer -= diff;

        // Stomp
        if (StompTimer < diff)
        {
            AddSpellToCast(SPELL_STOMP, CAST_NULL);
            StompTimer = 30000;
        }
        else
            StompTimer -= diff;

        // Cyclone
        if (CycloneTimer < diff)
        {
            AddSpellToCast(SPELL_CYCLONE, CAST_TANK);
            CycloneTimer = 20000;
        }
        else
            CycloneTimer -= diff;

        // Silence
        if (SilenceTimer < diff)
        {
            AddSpellToCast(SPELL_SILENCE, CAST_NULL);
            SilenceTimer = urand(20000, 30000);
        }
        else
            SilenceTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ossirian(Creature* pCreature)
{
    return new boss_ossirianAI(pCreature);
}

// This is actually a hack for a server-side spell
bool GOUse_go_ossirian_crystal(Player* /*pPlayer*/, GameObject* pGo)
{
    if (Creature* pOssirianTrigger = GetClosestCreatureWithEntry(pGo, NPC_OSSIRIAN_TRIGGER, 10.0f))
        pOssirianTrigger->CastSpell(pOssirianTrigger, aWeaknessSpell[urand(0, 4)], false);

    pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
    pGo->SetLootState(GO_READY);
    pGo->UseDoorOrButton(10000);

    return true;
}

struct sand_vortexAI : public ScriptedAI
{
    sand_vortexAI(Creature *c) : ScriptedAI(c) {}

    uint32 checkTimer;

    void Reset() 
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetSpeed(MOVE_RUN, 1.0f);
        checkTimer = 3000;
    }

    void EnterCombat(Unit* )
    {
        //SelectRandomTarget(false);
        if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0))
            AttackStart(target);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        damage = 0;
    }

    void UpdateAI(const uint32 diff)
    {
       if (checkTimer <= diff)
        {
            Creature* CrystallTrigger = GetClosestCreatureWithEntry(me, NPC_OSSIRIAN_TRIGGER, 7);
            if(CrystallTrigger)
            {
                if(Unit *target = SelectUnit(SELECT_TARGET_FARTHEST, 0))
                    AttackStart(target);
            }
            checkTimer = 2000;
        }
        else
            checkTimer -= diff;

        //if the vortex reach the target, it change his target to another player
        if(!me->GetVictim() || me->IsWithinMeleeRange(me->GetVictim()) || !me->GetVictim()->isAlive())
        {
            if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0))
                AttackStart(target);
        }
    }
};

CreatureAI* GetAI_sand_vortexAI(Creature *_Creature)
{
    return new sand_vortexAI (_Creature);
}

struct npc_ossirian_crystal_triggerAI : public ScriptedAI
{
    npc_ossirian_crystal_triggerAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    void Reset()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        damage = 0;
    }

    void AttackStart(Unit* /*pWho*/) { }
    void MoveInLineOfSight(Unit* /*pWho*/) { }
    void UpdateAI(const uint32 diff) {}
};

CreatureAI* GetAI_npc_ossirian_crystal_trigger(Creature* pCreature)
{
    return new npc_ossirian_crystal_triggerAI(pCreature);
}

void AddSC_boss_ossirian()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_ossirian";
    pNewScript->GetAI = &GetAI_boss_ossirian;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_ossirian_crystal";
    pNewScript->pGOUse = &GOUse_go_ossirian_crystal;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name="mob_sand_vortex";
    pNewScript->GetAI = &GetAI_sand_vortexAI;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name="npc_ossirian_crystal_trigger";
    pNewScript->GetAI = &GetAI_npc_ossirian_crystal_trigger;
    pNewScript->RegisterSelf();
}
