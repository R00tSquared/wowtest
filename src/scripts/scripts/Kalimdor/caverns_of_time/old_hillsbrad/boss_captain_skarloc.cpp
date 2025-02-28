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
SDName: Boss_Captain_Skarloc
SD%Complete: 99
SDComment:
SDCategory: Caverns of Time, Old Hillsbrad Foothills
EndScriptData */

#include "precompiled.h"
#include "def_old_hillsbrad.h"

enum CaptainSkarloc
{
    SAY_ENTER                   = -1560000,
    SAY_TAUNT1                  = -1560001,
    SAY_TAUNT2                  = -1560002,
    SAY_SLAY1                   = -1560003,
    SAY_SLAY2                   = -1560004,
    SAY_DEATH                   = -1560005,
    
    SPELL_HOLY_LIGHT            = 29427,
    SPELL_CLEANSE               = 29380,
    SPELL_HAMMER_OF_JUSTICE     = 13005,
    SPELL_HOLY_SHIELD           = 31904,
    SPELL_DEVOTION_AURA         = 8258,
    SPELL_CONSECRATION          = 38385,
    
    NPC_THRALL                  = 17876,
    C_WARDEN                    = 17833,
    C_VETERAN                   = 17860,
    SKARLOC_MOUNT               = 18798,
    SKARLOC_MOUNT_MODEL         = 18223
};

struct boss_captain_skarlocAI : public ScriptedAI
{
    boss_captain_skarlocAI(Creature *creature) : ScriptedAI(creature)
    {
        pInstance = (creature->GetInstanceData());
        HeroicMode = me->GetMap()->IsHeroic();
    }

    ScriptedInstance *pInstance;

    bool HeroicMode;
    bool Intro;

    Timer IntroTimer;
    Timer HolyLight_Timer;
    Timer Cleanse_Timer;
    Timer HammerOfJustice_Timer;
    Timer HolyShield_Timer;
    Timer DevotionAura_Timer;
    Timer Consecration_Timer;

    uint8 Next;
    uint64 Add1GUID;
    uint64 Add2GUID;
    uint64 ThrallinGUID;

    void Reset()
    {
        Intro = true;

        Next = 0;
        Add1GUID = 0;
        Add2GUID = 0;

        IntroTimer.Reset(20000);
        HolyLight_Timer.Reset(30000);
        Cleanse_Timer.Reset(10000);
        HammerOfJustice_Timer.Reset(urand(20000, 35000));
        HolyShield_Timer.Reset(240000);
        DevotionAura_Timer.Reset(3000);
        Consecration_Timer.Reset(8000);

        me->SetReactState(REACT_PASSIVE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->Mount(SKARLOC_MOUNT_MODEL);

        SummonGuards();

        me->GetMotionMaster()->MovePoint(0, 2047.90f, 254.85f, 62.822f);

        Map* tmpMap = me->GetMap();

        if (!tmpMap)
            return;

        if (Creature* Thrall = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_THRALL)))
            ThrallinGUID = Thrall->GetGUID();

        if(pInstance)
            pInstance->SetData(DATA_SKARLOC, NOT_STARTED);
    }

    void SummonGuards()
    {
        if (Creature *tAdd1 = DoSpawnCreature(C_WARDEN, -2, -2, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
        {
            Add1GUID = tAdd1->GetGUID();
            tAdd1->SetReactState(REACT_PASSIVE);
            tAdd1->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            tAdd1->GetMotionMaster()->MovePoint(0, 2044.12f, 253.47f, 62.748f);
        }

        if (Creature *tAdd2 = DoSpawnCreature(C_VETERAN, 2, 2, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
        {
            Add2GUID = tAdd2->GetGUID();
            tAdd2->SetReactState(REACT_PASSIVE);
            tAdd2->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            tAdd2->GetMotionMaster()->MovePoint(0, 2049.22f, 258.16f, 62.754f);
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            switch (Next)
            {
                case 0:
                    me->Unmount();

                    if (Creature* Thrall = me->GetMap()->GetCreature(ThrallinGUID))
                        Thrall->SummonCreature(SKARLOC_MOUNT,2047.90f, 254.85f, 62.822f, me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 5000);

                    me->SetWalk(true);
                    me->GetMotionMaster()->MovePoint(0, 2056.80f, 240.81f, 63.538f);

                    if (Creature* tAdd1 = me->GetCreature(Add1GUID))
                    {
                        tAdd1->SetWalk(true);
                        tAdd1->GetMotionMaster()->MovePoint(0, 2052.94f, 239.37f, 63.219f);
                    }

                    if (Creature* tAdd2 = me->GetCreature(Add2GUID))
                    {
                        tAdd2->SetWalk(true);
                        tAdd2->GetMotionMaster()->MovePoint(0, 2058.66f, 243.99f, 63.59f);
                    }

                    ++Next;
                    break;
                case 1:
                    DoScriptText(SAY_ENTER, me);
                    ++Next;
                    break;
            }
        }
    }

    void IntroEnd()
    {
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        me->SetWalk(false);

        if (Creature* Thrall = me->GetMap()->GetCreature(ThrallinGUID))
        {
            me->AI()->AttackStart(Thrall);

            if (Creature* tAdd1 = me->GetCreature(Add1GUID))
            {
                tAdd1->SetReactState(REACT_AGGRESSIVE);
                tAdd1->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                tAdd1->SetWalk(false);
                tAdd1->AI()->AttackStart(Thrall);
            }

            if (Creature* tAdd2 = me->GetCreature(Add2GUID))
            {
                tAdd2->SetReactState(REACT_AGGRESSIVE);
                tAdd2->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                tAdd2->SetWalk(false);
                tAdd2->AI()->AttackStart(Thrall);
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        me->SetPlayerDamageReqInPct(0.05); // should be lootable for players even if thrall damage was higher
        DoScriptText(SAY_TAUNT1, me);
        DoScriptText(SAY_TAUNT2, me);
        if(pInstance)
            pInstance->SetData(DATA_SKARLOC, IN_PROGRESS);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(RAND(SAY_SLAY1, SAY_SLAY2), me);
    }

    void EnterEvadeMode()
    {
        me->InterruptNonMeleeSpells(true);
        me->RemoveAllAuras();
        me->DeleteThreatList();
        me->CombatStop(true);

        Map* tmpMap = me->GetMap();

        if (!tmpMap)
            return;

        if (Creature* mount = tmpMap->GetCreature(tmpMap->GetCreatureGUID(SKARLOC_MOUNT)))
            mount->ForcedDespawn();
    }

    void JustDied(Unit *victim)
    {
        DoScriptText(SAY_DEATH, me);

        if (pInstance->GetData(TYPE_THRALL_EVENT) == IN_PROGRESS)
            pInstance->SetData(TYPE_THRALL_PART1, DONE);

        if (pInstance->GetData(DATA_SKARLOC_DEATH) == DONE)
            me->SetLootRecipient(NULL);
        else
            pInstance->SetData(DATA_SKARLOC_DEATH, DONE);
        if(pInstance)
            pInstance->SetData(DATA_SKARLOC, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Intro)
        {
            if (IntroTimer.Expired(diff))
            {
                IntroEnd();
                Intro = false;
            }
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (HolyLight_Timer.Expired(diff))
        {
            uint8 random = urand(0, 2);
            switch(random)
            {
                case 0:
                    DoCast(me, SPELL_HOLY_LIGHT);
                    break;
                case 1:
                    if(Add1GUID && me->GetCreature(Add1GUID) && me->GetCreature(Add1GUID)->isAlive() && me->GetCreature(Add1GUID)->GetHealthPercent() < 75)
                        DoCast(me->GetCreature(Add1GUID), SPELL_HOLY_LIGHT);
                    else
                        DoCast(me, SPELL_HOLY_LIGHT);
                    break;
                case 2:
                    if(Add2GUID && me->GetCreature(Add2GUID) && me->GetCreature(Add2GUID)->isAlive() && me->GetCreature(Add2GUID)->GetHealthPercent() < 75)
                        DoCast(me->GetCreature(Add2GUID), SPELL_HOLY_LIGHT);
                    else
                        DoCast(me, SPELL_HOLY_LIGHT);
                    break;
                default: break;
            }
            HolyLight_Timer = 30000;
        }

        if (Cleanse_Timer.Expired(diff))
        {
            DoCast(me, SPELL_CLEANSE);

            if(Add1GUID && me->GetCreature(Add1GUID) && me->GetCreature(Add1GUID)->isAlive())
                DoCast(me->GetCreature(Add1GUID), SPELL_CLEANSE);

            if(Add2GUID && me->GetCreature(Add2GUID) && me->GetCreature(Add2GUID)->isAlive())
                DoCast(me->GetCreature(Add2GUID), SPELL_CLEANSE);

            Cleanse_Timer = 10000;
        }

        if (HammerOfJustice_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HAMMER_OF_JUSTICE);
            HammerOfJustice_Timer = 60000;
        }

        if (HolyShield_Timer.Expired(diff))
        {
            DoCast(me, SPELL_HOLY_SHIELD);
            HolyShield_Timer = 240000;
        }

        if (DevotionAura_Timer.Expired(diff))
        {
            DoCast(me, SPELL_DEVOTION_AURA);
            DevotionAura_Timer = 60000;
        }

        if (HeroicMode)
        {
            if (Consecration_Timer.Expired(diff))
            {
                DoCast(me, SPELL_CONSECRATION);
                Consecration_Timer = 8000;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_captain_skarloc(Creature *creature)
{
    return new boss_captain_skarlocAI (creature);
}

void AddSC_boss_captain_skarloc()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_captain_skarloc";
    newscript->GetAI = &GetAI_boss_captain_skarloc;
    newscript->RegisterSelf();
}

