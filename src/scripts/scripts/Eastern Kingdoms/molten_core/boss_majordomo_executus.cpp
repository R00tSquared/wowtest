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
SDName: Boss_Majordomo_Executus
SD%Complete: 30
SDComment: Correct spawning and Event NYI
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "def_molten_core.h"

#define SAY_AGGRO           -1409003
#define SAY_SPAWN           -1409004
#define SAY_SLAY            -1409005
#define SAY_SPECIAL         -1409006
#define SAY_DEFEAT          -1409007

#define SAY_SUMMON_MAJ      -1409008
#define SAY_ARRIVAL1_RAG    -1409009
#define SAY_ARRIVAL2_MAJ    -1409010
#define SAY_ARRIVAL3_RAG    -1409011
#define SAY_ARRIVAL5_RAG    -1409012

#define SPAWN_RAG_X         838.51
#define SPAWN_RAG_Y         -829.84
#define SPAWN_RAG_Z         -232.00
#define SPAWN_RAG_O         1.70

#define SPELL_MAGIC_REFLECTION      20619
#define SPELL_DAMAGE_REFLECTION     21075

#define SPELL_BLASTWAVE             20229
#define SPELL_AEGIS                 20620                   //This is self cast whenever we are below 50%
#define SPELL_TELEPORT              20618
#define SPELL_SUMMON_RAGNAROS       19774
#define SPELL_SHADOW_BOLT           21077
#define SPELL_SHADOW_SHOCK          20603
#define SPELL_FIRE_BLAST            20623
#define SPELL_FIREBALL              20420

#define SPELL_TELEPORT_VISUAL       19484
#define ENTRY_FLAMEWALKER_HEALER    11663
#define ENTRY_FLAMEWALKER_ELITE     11664

#define CACHE_OF_THE_FIRELORD       179703
#define CACHE_OF_THE_FIRELORD_HEROIC       9479703

float AddLocations[8][4] =
{
    { 753.044, -1186.86, -118.333, 2.54516 },
    { 755.028, -1172.57, -118.636, 3.3227  },
    { 748.181, -1161.33, -118.807, 3.99422 },
    { 742.781, -1197.79, -118.008, 1.91291 },
    { 752.798, -1166.29, -118.766, 3.63844 },
    { 743.136, -1157.67, -119.021, 4.16779 },
    { 748.553, -1193.14, -118.099, 2.22158 },
    { 736.539, -1199.47, -118.334, 1.69143 }
};

struct boss_majordomoAI : public BossAI
{
    boss_majordomoAI(Creature *c) : BossAI(c, 1)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    Timer _ChangedNameMagicReflection_Timer;
    Timer _ChangedNameDamageReflection_Timer;
    Timer _ChangedNameBlastwave_Timer;
    Timer _ChangedNameTeleportVisual_Timer;
    Timer _ChangedNameTeleport_Timer;
    bool Teleport_Use;
    Timer _ChangedNameSummonRag_Timer;
    uint64 AddGUID[8];

    void Reset()
    {
        _ChangedNameMagicReflection_Timer.Reset(45000);                     //Damage reflection first so we alternate
        _ChangedNameDamageReflection_Timer.Reset(15000);
        _ChangedNameBlastwave_Timer.Reset(10000);
        _ChangedNameTeleportVisual_Timer.Reset(30000);
        _ChangedNameTeleport_Timer.Reset(20000);

        _ChangedNameSummonRag_Timer.Reset(20000);

        ClearCastQueue();

        if (pInstance && pInstance->GetData(DATA_MAJORDOMO_EXECUTUS_EVENT) < DONE)
        {
            pInstance->SetData(DATA_MAJORDOMO_EXECUTUS_EVENT, NOT_STARTED);
            me->SetVisibility(VISIBILITY_OFF);
            SpawnAdds();
        }

        me->SetReactState(REACT_PASSIVE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
    }

    void KilledUnit(Unit* victim)
    {
        if (rand() % 5)
            return;

        DoScriptText(SAY_SLAY, m_creature);
    }

    void EnterCombat(Unit *who)
    {
        if (me->GetVisibility() == VISIBILITY_OFF || me->GetReactState() == REACT_PASSIVE)
            return;

        DoZoneInCombat();
        for (uint8 i = 0; i < 8; ++i)
        {
            Creature* Temp = Unit::GetCreature((*m_creature), AddGUID[i]);
            if (Temp && Temp->isAlive())
            {
                Temp->SetReactState(REACT_AGGRESSIVE);
                Temp->AI()->AttackStart(m_creature->GetVictim());
            }
            else
            {
                EnterEvadeMode();
                break;
            }
        }
        DoAction(2);
    }

    void AttackStart(Unit *who)
    {
        if (me->GetVisibility() == VISIBILITY_ON || me->GetReactState() == REACT_AGGRESSIVE)
            BossAI::AttackStart(who);
    }

    void DoAction(const int32 action)
    {
        switch (action)
        {
            case 1:
            {
                DoScriptText(SAY_SPAWN, m_creature);
                me->SetVisibility(VISIBILITY_ON);
                me->SetReactState(REACT_AGGRESSIVE);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

                for (uint8 i = 0; i < 8; ++i)
                {
                    if (Creature* add = Unit::GetCreature((*m_creature), AddGUID[i]))
                    {
                        add->SetVisibility(VISIBILITY_ON);
                    }
                }
                break;
            }
            case 2:
            {
                pInstance->SetData(DATA_MAJORDOMO_EXECUTUS_EVENT, IN_PROGRESS);
                DoScriptText(SAY_AGGRO, m_creature);
                break;
            }
            case 3:
            {
                pInstance->SetData(DATA_MAJORDOMO_EXECUTUS_EVENT, DONE);
                EnterEvadeMode();
                DoScriptText(SAY_DEFEAT, m_creature);

                break;
            }
            case 4:
            {
                me->CastSpell(me, SPELL_TELEPORT_VISUAL, false);
                me->SetPosition(847.636, -814.667725, -229.79, 1.7, true);
                pInstance->SetData(DATA_SUMMON_RAGNAROS, IN_PROGRESS);
                break;
            }
            case 5:
            {
                me->CastSpell(me, SPELL_SUMMON_RAGNAROS, false);
                DoScriptText(SAY_SUMMON_MAJ, m_creature);
                pInstance->SetData(DATA_SUMMON_RAGNAROS, DONE);
                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (pInstance->GetData(DATA_RUNES) != RUNES_COMPLETE)
            return;

        if (pInstance->GetData(DATA_RUNES) == RUNES_COMPLETE && pInstance->GetData(DATA_MAJORDOMO_EXECUTUS_EVENT) == NOT_STARTED && me->GetVisibility() == VISIBILITY_OFF)
        {
            DoAction(1);
            return;
        }

        if (pInstance->GetData(DATA_MAJORDOMO_EXECUTUS_EVENT) == DONE && pInstance->GetData(DATA_SUMMON_RAGNAROS) == NOT_STARTED)
        {
            if (_ChangedNameTeleportVisual_Timer.Expired(diff))
            {
                DoAction(4);
                _ChangedNameTeleportVisual_Timer = 1000000;
            }
            return;
        }

        if (pInstance->GetData(DATA_MAJORDOMO_EXECUTUS_EVENT) == DONE && pInstance->GetData(DATA_SUMMON_RAGNAROS) == IN_PROGRESS)
        {
            if (_ChangedNameSummonRag_Timer.Expired(diff))
            {
                DoAction(5);
                _ChangedNameSummonRag_Timer = 1000000;
            }
            return;
        }

        if (!UpdateVictim())
            return;

        if (pInstance->GetData(DATA_MAJORDOMO_EXECUTUS_EVENT) == IN_PROGRESS)
        {
            if (AddsKilled())
            {
                if (HeroicMode)
                    me->GetVictim()->SummonGameObject(CACHE_OF_THE_FIRELORD_HEROIC, 752.492, -1188.51, -118.296, 2.4288, 0, 0, 0.93716, 0.348899, 0);
                else
                    me->GetVictim()->SummonGameObject(CACHE_OF_THE_FIRELORD, 752.492, -1188.51, -118.296, 2.4288, 0, 0, 0.93716, 0.348899, 0);
                DoAction(3);
                return;
            }
        }

        //Cast Ageis if less than 50% hp
        if (me->GetHealthPercent() < 50)
        {
            DoCast(m_creature, SPELL_AEGIS);
        }

        if (_ChangedNameTeleport_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_TELEPORT), true))
            {
                ForceSpellCast(target, SPELL_TELEPORT, DONT_INTERRUPT, false, true);
                target->SetPosition(736.767456, -1176.591797, -118.948753, 0, true);
                ((Player*)target)->TeleportTo(me->GetMapId(), 736.767456, -1176.591797, -118.948753, 0);
                _ChangedNameTeleport_Timer = 20000;
            }
        }

        if (_ChangedNameMagicReflection_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_MAGIC_REFLECTION, false);
            _ChangedNameMagicReflection_Timer = 30000;
        }

        if (_ChangedNameDamageReflection_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_DAMAGE_REFLECTION, false);
            _ChangedNameDamageReflection_Timer = 30000;
        }

        if (_ChangedNameBlastwave_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_BLASTWAVE, false);
            _ChangedNameBlastwave_Timer = 10000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    bool AddsKilled()
    {
        for (uint8 i = 0; i < 8; ++i)
        {
            Unit* add = Unit::GetUnit((*m_creature), AddGUID[i]);
            if (add && add->isAlive())
                return false;
        }
        return true;
    }

    void SpawnAdds()
    {
        for (uint8 i = 0; i < 8; ++i)
        {
            Creature *pCreature = (Unit::GetCreature((*m_creature), AddGUID[i]));
            if (!pCreature || !pCreature->isAlive())
            {
                if (pCreature)
                    pCreature->setDeathState(DEAD);
                pCreature = me->SummonCreature(i < 4 ? ENTRY_FLAMEWALKER_HEALER : ENTRY_FLAMEWALKER_ELITE, AddLocations[i][0], AddLocations[i][1], AddLocations[i][2], AddLocations[i][3], TEMPSUMMON_DEAD_DESPAWN, 0);
                if (pCreature)
                {
                    AddGUID[i] = pCreature->GetGUID();
                }
            }
            else
            {
                pCreature->AI()->Reset();
            }
        }
    }
};

CreatureAI* GetAI_boss_majordomo(Creature *_Creature)
{
    return new boss_majordomoAI(_Creature);
}

struct MCflamewakerAI : public ScriptedAI
{
    MCflamewakerAI(Creature *c) : ScriptedAI(c)
    {
        me->SetVisibility(VISIBILITY_OFF);
        me->SetReactState(REACT_PASSIVE);
    }

    uint64 ownerGUID;

    void Reset()
    {
        me->SetVisibility(VISIBILITY_OFF);
        me->SetReactState(REACT_PASSIVE);
        me->AI()->EnterEvadeMode();
        ScriptedAI::Reset();
        ownerGUID = 0;
    }

    void EnterCombat(Unit *who)
    {
        me->AI()->AttackStart(who);
    }

    void AttackStart(Unit *who)
    {
        Unit* owner = me->GetUnit(ownerGUID);

        if (me->GetReactState() == REACT_AGGRESSIVE)
            ScriptedAI::AttackStart(who);
        else if (me->GetReactState() == REACT_PASSIVE && owner && who)
            ((Creature*) owner)->AI()->AttackStart(who);

    }

    void IsSummonedBy(Unit *summoner)
    {
        ownerGUID = summoner->GetGUID();
    }
};

CreatureAI* GetAI_MCflamewaker(Creature *_Creature)
{
    return new MCflamewakerAI(_Creature);
}

struct flamewaker_healerAI: public MCflamewakerAI
{
    flamewaker_healerAI(Creature *c) : MCflamewakerAI(c)
    {
    }

    Timer _ChangedNameShadownBolt_Timer;
    Timer _ChangedNameShadownShock_Timer;

    void Reset()
    {
        _ChangedNameShadownBolt_Timer.Reset(1000);
        _ChangedNameShadownShock_Timer.Reset(8000);
        MCflamewakerAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->GetVictim())
            return;

        if (_ChangedNameShadownBolt_Timer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, GetSpellMaxRange(SPELL_SHADOW_BOLT), true))
            {
                AddSpellToCast(target, SPELL_SHADOW_BOLT, false);
                _ChangedNameShadownBolt_Timer = 2000;
            }
        }

        if (_ChangedNameShadownShock_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_SHADOW_SHOCK, false);
            _ChangedNameShadownShock_Timer = 9000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_flamewaker_healer(Creature *_Creature)
{
    return new flamewaker_healerAI(_Creature);
}

struct flamewaker_eliteAI: public MCflamewakerAI
{
    flamewaker_eliteAI(Creature *c) : MCflamewakerAI(c)
    {
    }

    Timer _ChangedNameBlastWave_Timer;
    Timer _ChangedNameFireBlast_Timer;
    Timer _ChangedNameFireball_Timer;

    void Reset()
    {
        _ChangedNameBlastWave_Timer.Reset(12000);
        _ChangedNameFireBlast_Timer.Reset(5000);
        _ChangedNameFireball_Timer.Reset(1000);
        MCflamewakerAI::Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->GetVictim())
            return;

        if (_ChangedNameBlastWave_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature, SPELL_BLASTWAVE, false);
            _ChangedNameBlastWave_Timer = 12000;
        }

        if (_ChangedNameFireBlast_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FIRE_BLAST, false);
            _ChangedNameFireBlast_Timer = 15000;
        }
        
        if (_ChangedNameFireball_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FIREBALL, false);
            _ChangedNameFireball_Timer = 8000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_flamewaker_elite(Creature *_Creature)
{
    return new flamewaker_eliteAI(_Creature);
}

void AddSC_boss_majordomo()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_majordomo";
    newscript->GetAI = &GetAI_boss_majordomo;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "flamewaker_healer";
    newscript->GetAI = &GetAI_flamewaker_healer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "flamewaker_elite";
    newscript->GetAI = &GetAI_flamewaker_elite;
    newscript->RegisterSelf();
}


