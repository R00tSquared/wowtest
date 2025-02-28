// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2006-2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: boss_archaedas
SD%Complete: 100
SDComment: Archaedas is activated when 3 prople click on his altar.
Every 10 seconds he will awaken one of his minions along the wall.
At 66%, he will awaken the 6 Guardians.
At 33%, he will awaken the Vault Walkers
On his death the vault door opens.
EndScriptData */

#include "precompiled.h"
#include "def_uldaman.h"

#define SAY_AGGRO           -1200299
#define SOUND_AGGRO         5855

#define SAY_SUMMON          -1200300
#define SOUND_SUMMON        5856

#define SAY_SUMMON2         -1200301
#define SOUND_SUMMON2       5857

#define SAY_KILL            -1200302
#define SOUND_KILL          5858

#define SPELL_GROUND_TREMOR           6524
#define SPELL_ARCHAEDAS_AWAKEN        10347
#define SPELL_BOSS_OBJECT_VISUAL      11206
#define SPELL_BOSS_AGGRO              10340
#define SPELL_SUB_BOSS_AGGRO          11568
#define SPELL_AWAKEN_VAULT_WALKER     10258
#define SPELL_AWAKEN_EARTHEN_GUARDIAN 10252

struct boss_archaedasAI : public ScriptedAI
{
    boss_archaedasAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (m_creature->GetInstanceData());
    }

    Timer _ChangedNameTremor_Timer;
    Timer _ChangedNameAwaken_Timer;
    Timer _ChangedNameWallMinionTimer;
    bool wakingUp;

    bool InCombat;
    bool guardiansAwake;
    bool vaultWalkersAwake;
    ScriptedInstance* pInstance;

    void Reset()
    {
        _ChangedNameTremor_Timer.Reset(60000);
        _ChangedNameAwaken_Timer.Reset(1);
        _ChangedNameWallMinionTimer.Reset(10000);

        wakingUp = false;
        guardiansAwake = false;
        vaultWalkersAwake = false;

        if (pInstance) pInstance->SetData(DATA_ANCIENT_DOOR, NOT_STARTED);
        if (pInstance) pInstance->SetData(DATA_MINIONS, NOT_STARTED);    // respawn any dead minions
        m_creature->setFaction(35);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

    }

    void ActivateMinion(uint64 guid, bool flag)
    {
        Unit *minion = Unit::GetUnit(*m_creature, guid);

        if (minion && minion->isAlive())
        {
            DoCast(minion, SPELL_AWAKEN_VAULT_WALKER, flag);
            minion->CastSpell(minion, SPELL_ARCHAEDAS_AWAKEN, true);
        }
    }

    void SpellHit(Unit* caster, const SpellEntry *spell)
    {
        // Being woken up from the altar, start the awaken sequence
        if (spell == GetSpellStore()->LookupEntry<SpellEntry>(SPELL_ARCHAEDAS_AWAKEN))
        {
            DoYell(-1200299, LANG_UNIVERSAL, NULL);
            DoPlaySoundToSet(m_creature, SOUND_AGGRO);
            _ChangedNameAwaken_Timer.Reset(4000);
            wakingUp = true;
        }
    }

    void KilledUnit(Unit *victim)
    {
        DoYell(-1200302, LANG_UNIVERSAL, NULL);
        DoPlaySoundToSet(m_creature, SOUND_KILL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!pInstance)
            return;
        // we're still doing awaken animation
        if (wakingUp)
        {
            // dont do anything until we are done
            if (_ChangedNameAwaken_Timer.Expired(diff))
            {
                wakingUp = false;
                m_creature->setFaction(14);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                m_creature->RemoveAllAuras();
            }

            return;
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;


        // wake a wall minion
        if (_ChangedNameWallMinionTimer.Expired(diff))
        {
            pInstance->SetData(DATA_MINIONS, IN_PROGRESS);

            _ChangedNameWallMinionTimer = 10000;
        }
        

        //If we are <66 summon the guardians
        if (!guardiansAwake && me->GetHealthPercent() <= 66)
        {
            ActivateMinion(pInstance->GetData64(5), true);   // EarthenGuardian1
            ActivateMinion(pInstance->GetData64(6), true);   // EarthenGuardian2
            ActivateMinion(pInstance->GetData64(7), true);   // EarthenGuardian3
            ActivateMinion(pInstance->GetData64(8), true);   // EarthenGuardian4
            ActivateMinion(pInstance->GetData64(9), true);   // EarthenGuardian5
            ActivateMinion(pInstance->GetData64(10), false); // EarthenGuardian6
            DoYell(-1200300, LANG_UNIVERSAL, NULL);
            DoPlaySoundToSet(m_creature, SOUND_SUMMON);
            guardiansAwake = true;
        }

        //If we are <33 summon the vault walkers
        if (!vaultWalkersAwake && me->GetHealthPercent() <= 33)
        {
            ActivateMinion(pInstance->GetData64(1), true);    // VaultWalker1
            ActivateMinion(pInstance->GetData64(2), true);    // VaultWalker2
            ActivateMinion(pInstance->GetData64(3), true);    // VaultWalker3
            ActivateMinion(pInstance->GetData64(4), false);    // VaultWalker4
            DoYell(-1200301, LANG_UNIVERSAL, NULL);
            DoPlaySoundToSet(m_creature, SOUND_SUMMON2);
            vaultWalkersAwake = true;
        }


        if (_ChangedNameTremor_Timer.Expired(diff))
        {
            //Cast
            DoCast(m_creature->GetVictim(), SPELL_GROUND_TREMOR);

            //45 seconds until we should cast this agian
            _ChangedNameTremor_Timer = 45000;
        }

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit *killer)
    {
        if (pInstance)
        {
            pInstance->SetData(DATA_ANCIENT_DOOR, DONE);        // open the vault door
            pInstance->SetData(DATA_MINIONS, SPECIAL);        // deactivate his minions
        }
    }

};

CreatureAI* GetAI_boss_archaedas(Creature *_Creature)
{
    return new boss_archaedasAI(_Creature);
}

/* ScriptData
SDName: mob_archaedas_minions
SD%Complete: 100
SDComment: These mobs are initially frozen until Archaedas awakens them
one at a time.
EndScriptData */


#define SPELL_ARCHAEDAS_AWAKEN        10347

struct mob_archaedas_minionsAI : public ScriptedAI
{
    mob_archaedas_minionsAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (m_creature->GetInstanceData());
    }

    Timer _ChangedNameAwaken_Timer;
    bool wakingUp;

    bool InCombat;
    bool amIAwake;
    ScriptedInstance* pInstance;

    void Reset()
    {
        _ChangedNameAwaken_Timer.Reset(1);

        wakingUp = false;
        amIAwake = false;

        m_creature->setFaction(35);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
    }


    void SpellHit(Unit* caster, const SpellEntry *spell)
    {
        // time to wake up, start animation
        if (spell == GetSpellStore()->LookupEntry<SpellEntry>(SPELL_ARCHAEDAS_AWAKEN))
        {
            _ChangedNameAwaken_Timer = 5000;
            wakingUp = true;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (amIAwake)
            ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    {
        // we're still in the awaken animation
        if (wakingUp)
        {
            // dont do anything until we are done
            if (_ChangedNameAwaken_Timer.Expired(diff))
            {
                wakingUp = false;
                amIAwake = true;

                m_creature->setFaction(14);
                m_creature->RemoveAllAuras();
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                AttackStart(me->GetCreature(pInstance->GetData64(0))->GetVictim());
            }
            return;
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;


        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_archaedas_minions(Creature *_Creature)
{
    return new mob_archaedas_minionsAI(_Creature);
}

/* ScriptData
SDName: mob_stonekeepers
SD%Complete: 100
SDComment: After activating the altar of the keepers, the stone keepers will
wake up one by one.
EndScriptData */

#include "precompiled.h"

#define SPELL_SELF_DESTRUCT           9874

struct mob_stonekeepersAI : public ScriptedAI
{
    mob_stonekeepersAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (m_creature->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    void Reset()
    {
        m_creature->setFaction(35);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
    }

    void EnterCombat(Unit *who)
    {
        m_creature->setFaction(14);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        m_creature->RemoveAllAuras();
    }

    void UpdateAI(const uint32 diff)
    {

        //Return since we have no target
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit *killer)
    {
        DoCast(m_creature, SPELL_SELF_DESTRUCT, true);
        if (pInstance)
            pInstance->SetData(DATA_STONE_KEEPERS, IN_PROGRESS);    // activate next stonekeeper
    }

};

CreatureAI* GetAI_mob_stonekeepers(Creature *_Creature)
{
    return new mob_stonekeepersAI(_Creature);
}

void AddSC_boss_archaedas()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_archaedas";
    newscript->GetAI = &GetAI_boss_archaedas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_archaedas_minions";
    newscript->GetAI = &GetAI_mob_archaedas_minions;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_stonekeepers";
    newscript->GetAI = &GetAI_mob_stonekeepers;
    newscript->RegisterSelf();
}


