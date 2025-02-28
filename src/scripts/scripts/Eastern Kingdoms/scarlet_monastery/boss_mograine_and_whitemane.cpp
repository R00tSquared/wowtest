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
SDName: Boss_Mograine_And_Whitemane
SD%Complete: 75
SDComment: Event not implemented
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"
#include "def_scarlet_monastery.h"

#define SAY_MO_AGGRO                -1189005
#define SAY_MO_KILL                 -1189006
#define SAY_MO_RESSURECTED          -1189007

#define SAY_WH_INTRO                -1189008
#define SAY_WH_KILL                 -1189009
#define SAY_WH_RESSURECT            -1189010

#define SPELL_DIVINESHIELD2         1020
#define SPELL_CRUSADERSTRIKE5       35395
#define SPELL_HAMMEROFJUSTICE3      5589
#define SPELL_HOLYLIGHT6            3472
#define SPELL_CONSECRATION3         20922
#define SPELL_BLESSINGOFWISDOM      1044
#define SPELL_RETRIBUTIONAURA3      10299
#define SPELL_BLESSINGOFPROTECTION3 10278
#define SPELL_FLASHHEAL6            10916
#define SOUND_MOGRAINE_FAKE_DEATH   1326
#define SPELL_SCARLETRESURRECTION   9232
#define SPELL_LAYONHANDS            9257

#define ENTRY_SCARLET_CHAPLAIN      4299
#define ENTRY_SCARLET_WIZARD        4300
#define ENTRY_SCARLET_CENTURION     4301
#define ENTRY_SCARLET_CHAMPION      4302
#define ENTRY_SCARLET_ABBOT         4303
#define ENTRY_SCARLET_MONK          4540

struct boss_scarlet_commander_mograineAI : public ScriptedAI
{
    boss_scarlet_commander_mograineAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)me->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer CrusaderStrike_Timer;
    Timer HammerOfJustice_Timer;
    bool HasDied;
    bool Heal;
    bool FakeDeath;

    void Reset()
    {
        CrusaderStrike_Timer.Reset(8400);
        HammerOfJustice_Timer.Reset(9600);
        HasDied = false;
        Heal = false;
        FakeDeath = false;

        // Incase wipe during phase that mograine fake death
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->SetStandState(UNIT_STAND_STATE_STAND);

        if(pInstance)
            pInstance->HandleGameObject(pInstance->GetData64(DATA_DOOR_WHITEMANE), false);

        if(pInstance)
        {
            if (Creature* pWhitemane = pInstance->GetCreature(pInstance->GetData64(DATA_WHITEMANE)))
            {
                if (me->isAlive() && !pWhitemane->isAlive())
                    pWhitemane->Respawn();
            }
        }
    }

    void DamageTaken(Unit* /*pDoneBy*/, uint32& damage)
    {
        if (damage < me->GetHealth() || HasDied)
            return;

        if (!pInstance)
            return;

        // On first death, fake death and open door, as well as initiate whitemane if exist
        if (Creature* pWhitemane = pInstance->GetCreature(pInstance->GetData64(DATA_WHITEMANE)))
        {
            pInstance->SetData(TYPE_MOGRAINE_AND_WHITE_EVENT, IN_PROGRESS);

            pWhitemane->GetMotionMaster()->MovePoint(100, 1163.113370f, 1398.856812f, 32.527786f);

            if(pInstance)
                pInstance->HandleGameObject(pInstance->GetData64(DATA_DOOR_WHITEMANE), true);

            me->GetMotionMaster()->MovementExpired();
            me->GetMotionMaster()->MoveIdle();

            if (me->IsNonMeleeSpellCast(false))
                me->InterruptNonMeleeSpells(false);

            me->ClearComboPointHolders();
            me->RemoveAllAurasOnDeath();
            me->ClearAllReactives();

            me->PlayDistanceSound(SOUND_MOGRAINE_FAKE_DEATH);

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetStandState(UNIT_STAND_STATE_DEAD);

            HasDied = true;
            FakeDeath = true;

            damage = std::min(damage, me->GetHealth() - 1);
        }
    }

    void SpellHit(Unit* /*pWho*/, const SpellEntry* pSpell)
    {
        // When hit with ressurection say text
        if (pSpell->Id == SPELL_SCARLETRESURRECTION)
        {
            DoScriptText(SAY_MO_RESSURECTED, me);
            FakeDeath = false;

            if (pInstance)
                pInstance->SetData(TYPE_MOGRAINE_AND_WHITE_EVENT, SPECIAL);
        }
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_MO_AGGRO, me);
        DoCast(me, SPELL_RETRIBUTIONAURA3);
		CallAllies(who);
    }

	void CallAllies(Unit* who) {
		uint32 creatures[6] = { ENTRY_SCARLET_CHAPLAIN,ENTRY_SCARLET_WIZARD,ENTRY_SCARLET_CENTURION,ENTRY_SCARLET_CHAMPION,ENTRY_SCARLET_ABBOT,ENTRY_SCARLET_MONK };

		for (auto cr : creatures)
		{
			std::list<Creature*> cr_list = FindAllCreaturesWithEntry(cr, 78);
			if (!cr_list.empty()) {
				for (std::list<Creature*>::iterator itr = cr_list.begin(); itr != cr_list.end(); ++itr)
				{
					if ((*itr)->isAlive() && (*itr)->AI())
						(*itr)->AI()->AttackStart(who);
				}
			}
		}
	}

    void KilledUnit(Unit *victim)
    {
        DoScriptText(SAY_MO_KILL, me);
    }

    void JustDied(Unit *who)
    {
        if(!pInstance)
            return;

        pInstance->HandleGameObject(pInstance->GetData64(DATA_DOOR_WHITEMANE), true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (HasDied && !Heal && pInstance && pInstance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) == SPECIAL)
        {
            // On ressurection, stop fake death and heal whitemane and resume fight
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetStandState(UNIT_STAND_STATE_STAND);
            // spell has script target on Whitemane
            DoCast(me, SPELL_LAYONHANDS);

            CrusaderStrike_Timer = 8400;
            HammerOfJustice_Timer = 9600;

            if (me->GetVictim())
                me->GetMotionMaster()->MoveChase(me->GetVictim());

            Heal = true;
        }

        // This if-check to make sure mograine does not attack while fake death
        if (FakeDeath)
            return;

        if (CrusaderStrike_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CRUSADERSTRIKE5);
            CrusaderStrike_Timer = 20000;
        }

        if (HammerOfJustice_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HAMMEROFJUSTICE3);
            HammerOfJustice_Timer = 30000;
        }
        DoMeleeAttackIfReady();
    }
};

#define SPELL_DEEPSLEEP                 9256

#define SPELL_CRUSADERSTRIKE            17281
#define SPELL_HAMMEROFJUSTICE           13005
#define SPELL_HOLYSMITE6                9481
#define SPELL_HOLYFIRE5                 15265
#define SPELL_MINDBLAST6                8106

#define SPELL_POWERWORDSHIELD           6065

#define SPELL_RENEW                     6078
#define SPELL_FLASHHEAL6                10916

struct boss_high_inquisitor_whitemaneAI : public ScriptedAI
{
    boss_high_inquisitor_whitemaneAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)me->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    Timer Healing_Timer;
    // Timer Renew_Timer;
    Timer PowerWordShield_Timer;
    // Timer CrusaderStrike_Timer;
    // Timer HammerOfJustice_Timer;
    Timer HolySmite_Timer;
    // Timer HolyFire5_Timer;
    // Timer MindBlast6_Timer;
    Timer Wait_Timer;

    bool CanRessurect;
    bool CanRessurectCheck;

    void Reset()
    {
        Healing_Timer.Reset(10000);
        // Renew_Timer.Reset(1);
        PowerWordShield_Timer.Reset(15000);
        // CrusaderStrike_Timer.Reset(12000);
        // HammerOfJustice_Timer.Reset(18000);
        HolySmite_Timer.Reset(4000);
        // HolyFire5_Timer.Reset(20000);
        // MindBlast6_Timer.Reset(6000);
        Wait_Timer.Reset(7000);
        CanRessurect = false;
        CanRessurectCheck = false;

        if(pInstance)
        {
            if (Creature* pMograine = pInstance->GetCreature(pInstance->GetData64(DATA_MOGRAINE)))
            {
                if (me->isAlive() && !pMograine->isAlive())
                    pMograine->Respawn();
            }
        }
    }

    void MoveInLineOfSight(Unit* /*pWho*/) override
    {
        // This needs to be empty because Whitemane should NOT aggro while fighting Mograine. Mograine will give us a target.
    }

    void DamageTaken(Unit* /*pDoneBy*/, uint32& damage)
    {
        if (damage < me->GetHealth())
            return;

        if (!CanRessurectCheck || CanRessurect)
        {
            // prevent killing blow before rezzing commander
            damage = std::min(damage, me->GetHealth() - 1);
        }
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type != POINT_MOTION_TYPE)
            return;

        if(Id != 100)
            return;

        me->SetRooted(true);
        if (Unit *attacker = me->SelectNearestTarget(15.0))
            me->AI()->AttackStart(attacker);
    }

    void EnterCombat(Unit *who)
    {
        DoScriptText(SAY_WH_INTRO, me);
        me->SetRooted(false);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(SAY_WH_KILL, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CanRessurect)
        {
            // When casting resuruction make sure to delay so on rez when reinstate battle deepsleep runs out
            if (Wait_Timer.Expired(diff))
            {
                // spell has script target on Mograine
                DoCast(me, SPELL_SCARLETRESURRECTION);
                DoScriptText(SAY_WH_RESSURECT, me);
                CanRessurect = false;
            }
        }

        // Cast Deep sleep when health is less than 50%
        if (!CanRessurectCheck && me->GetHealthPercent() <= 50.0f)
        {
            me->InterruptNonMeleeSpells(true);
            DoCast(me, SPELL_DEEPSLEEP);
            CanRessurectCheck = true;
            CanRessurect = true;
            return;
        }

        // while in "resurrect-mode", don't do anything
        if (CanRessurect)
            return;

        // If we are <75% hp cast healing spells at self or Mograine
        if (Healing_Timer.Expired(diff))
        {
            if (Unit* pTarget = SelectLowestHpFriendly(50.0f))
            {
                DoCast(pTarget,SPELL_FLASHHEAL6);
                Healing_Timer = 13000;
            }
        }

        if (PowerWordShield_Timer.Expired(diff))
        {
            DoCast(me, SPELL_POWERWORDSHIELD);
            PowerWordShield_Timer = urand(22000, 45000);
        }

        if (HolySmite_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HOLYSMITE6);
            HolySmite_Timer = urand(3500, 5000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_scarlet_commander_mograine(Creature *_Creature)
{
    return new boss_scarlet_commander_mograineAI (_Creature);
}

CreatureAI* GetAI_boss_high_inquisitor_whitemane(Creature *_Creature)
{
    return new boss_high_inquisitor_whitemaneAI (_Creature);
}

void AddSC_boss_mograine_and_whitemane()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_scarlet_commander_mograine";
    newscript->GetAI = &GetAI_boss_scarlet_commander_mograine;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "boss_high_inquisitor_whitemane";
    newscript->GetAI = &GetAI_boss_high_inquisitor_whitemane;
    newscript->RegisterSelf();
}


