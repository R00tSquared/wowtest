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
 SDName: Netherstorm
 SD%Complete: 99
 SDComment: Quest support: 10337,10438,10652 (special flight paths),10299,10321,10322,10323,10329,10330,10338,10365,10427,10345,10924,10191,10198,10309,10310,10425,10406,10439,10408,10273,10248

 SDCategory: Netherstorm
 EndScriptData */

 /* ContentData
 npc_manaforge_control_console
 go_manaforge_control_console
 npc_commander_dawnforge
 at_commander_dawnforge
 npc_protectorate_nether_drake
 npc_professor_dabiri
 npc_veronia
 mob_phase_hunter
 npc_bessy
 mob_talbuk
 npc_withered_corpse
 go_ethereum_prison
 npc_warp_chaser
 mob_epextraction
 mob_dr_boom
 mob_boom_bot
 npc_maxx_a_million
 npc_scrapped_reaver
 npc_drijya
 npc_captured_vanguard
 npc_controller
 npc_protectorate_demolitionist
 npc_saeed
 npc_dimensius
 npc_king_salhadaar
 npc_energy_ball
 npc_trader_marid
 npc_doctor_vomisa
 mob_warp_aberration
 mob_sunfury_astromancer
 EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

 /*######
 ## npc_manaforge_control_console
 ######*/

#define EMOTE_START             -1000296
#define EMOTE_60                -1000297
#define EMOTE_30                -1000298
#define EMOTE_10                -1000299
#define EMOTE_COMPLETE          -1000300
#define EMOTE_ABORT             -1000301

#define ENTRY_BNAAR_C_CONSOLE   20209
#define ENTRY_CORUU_C_CONSOLE   20417
#define ENTRY_DURO_C_CONSOLE    20418
#define ENTRY_ARA_C_CONSOLE     20440

#define ENTRY_SUNFURY_TECH      20218
#define ENTRY_SUNFURY_PROT      20436

#define ENTRY_ARA_TECH          20438
#define ENTRY_ARA_ENGI          20439
#define ENTRY_ARA_GORKLONN      20460

#define SPELL_DISABLE_VISUAL    35031
#define SPELL_INTERRUPT_1       35016                       //ACID mobs should cast this
#define SPELL_INTERRUPT_2       35176                       //ACID mobs should cast this (Manaforge Ara-version)
#define SPELL_SLEEP_VISUAL      34664                        // Used by in Creatures of Eco-Drome

struct npc_manaforge_control_consoleAI : public ScriptedAI
{
	npc_manaforge_control_consoleAI(Creature* creature) : ScriptedAI(creature) {}

	Timer_UnCheked Event_Timer;
	Timer_UnCheked Wave_Timer;
	uint32 Phase;
	bool Wave;
	uint64 someplayer;
	uint64 goConsole;

	void Reset()
	{
		Event_Timer.Reset(3000);
		Wave_Timer = 0;
		Phase = 1;
		Wave = false;
		someplayer = 0;
		goConsole = 0;
	}

	void EnterCombat(Unit* who) { return; }

	/*void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		//we have no way of telling the creature was hit by spell -> got aura applied after 10-12 seconds
		//then no way for the mobs to actually stop the shutdown as intended.
		if( spell->Id == SPELL_INTERRUPT_1 )
			DoSay("Silence! I kill you!",LANG_UNIVERSAL, NULL);
	}*/

	void JustDied(Unit* killer)
	{
		DoScriptText(EMOTE_ABORT, me);

		if (someplayer)
		{
			Player* p = Unit::GetPlayerInWorld(someplayer);
			if (p)
			{
				switch (me->GetEntry())
				{
				case ENTRY_BNAAR_C_CONSOLE:
					p->FailQuest(10299);
					p->FailQuest(10329);
					break;
				case ENTRY_CORUU_C_CONSOLE:
					p->FailQuest(10321);
					p->FailQuest(10330);
					break;
				case ENTRY_DURO_C_CONSOLE:
					p->FailQuest(10322);
					p->FailQuest(10338);
					break;
				case ENTRY_ARA_C_CONSOLE:
					p->FailQuest(10323);
					p->FailQuest(10365);
					break;
				}
			}
		}

		if (goConsole)
		{
			if (GameObject* go = GameObject::GetGameObject((*me), goConsole))
				go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
		}
	}

	void DoWaveSpawnForCreature(Creature* creature)
	{
		Creature* add = NULL;
		switch (creature->GetEntry())
		{
		case ENTRY_BNAAR_C_CONSOLE:
			if (rand() % 2)
			{
				add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2933.68, 4162.55, 164.00, 1.60, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 2927.36, 4212.97, 164.00);
			}
			else
			{
				add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2927.36, 4212.97, 164.00, 4.94, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 2933.68, 4162.55, 164.00);
			}
			Wave_Timer = 30000;
			break;
		case ENTRY_CORUU_C_CONSOLE:
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2445.21, 2765.26, 134.49, 3.93, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2424.21, 2740.15, 133.81);
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2429.86, 2731.85, 134.53, 1.31, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2435.37, 2766.04, 133.81);
			Wave_Timer = 20000;
			break;
		case ENTRY_DURO_C_CONSOLE:
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2986.80, 2205.36, 165.37, 3.74, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2985.15, 2197.32, 164.79);
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2952.91, 2191.20, 165.32, 0.22, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2060.01, 2185.27, 164.67);
			Wave_Timer = 15000;
			break;
		case ENTRY_ARA_C_CONSOLE:
			if (rand() % 2)
			{
				add = me->SummonCreature(ENTRY_ARA_TECH, 4035.11, 4038.97, 194.27, 2.57, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 4003.42, 4040.19, 193.49);
				add = me->SummonCreature(ENTRY_ARA_TECH, 4033.66, 4036.79, 194.28, 2.57, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 4003.42, 4040.19, 193.49);
				add = me->SummonCreature(ENTRY_ARA_TECH, 4037.13, 4037.30, 194.23, 2.57, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 4003.42, 4040.19, 193.49);
			}
			else
			{
				add = me->SummonCreature(ENTRY_ARA_TECH, 3099.59, 4049.30, 194.22, 0.05, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 4028.01, 4035.17, 193.59);
				add = me->SummonCreature(ENTRY_ARA_TECH, 3999.72, 4046.75, 194.22, 0.05, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 4028.01, 4035.17, 193.59);
				add = me->SummonCreature(ENTRY_ARA_TECH, 3996.81, 4048.26, 194.22, 0.05, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
				if (add) add->GetMotionMaster()->MovePoint(0, 4028.01, 4035.17, 193.59);
			}
			Wave_Timer = 15000;
			break;
		}
	}
	void DoFinalSpawnForCreature(Creature* creature)
	{
		Creature* add = NULL;
		switch (creature->GetEntry())
		{
		case ENTRY_BNAAR_C_CONSOLE:
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2946.52, 4201.42, 163.47, 3.54, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2927.49, 4192.81, 163.00);
			break;
		case ENTRY_CORUU_C_CONSOLE:
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2453.88, 2737.85, 133.27, 2.59, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2433.96, 2751.53, 133.85);
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2441.62, 2735.32, 134.49, 1.97, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2433.96, 2751.53, 133.85);
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2450.73, 2754.50, 134.49, 3.29, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2433.96, 2751.53, 133.85);
			break;
		case ENTRY_DURO_C_CONSOLE:
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2956.18, 2202.85, 165.32, 5.45, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2972.27, 2193.22, 164.48);
			add = me->SummonCreature(ENTRY_SUNFURY_TECH, 2975.30, 2211.50, 165.32, 4.55, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2972.27, 2193.22, 164.48);
			add = me->SummonCreature(ENTRY_SUNFURY_PROT, 2965.02, 2217.45, 164.16, 4.96, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 2972.27, 2193.22, 164.48);
			break;
		case ENTRY_ARA_C_CONSOLE:
			add = me->SummonCreature(ENTRY_ARA_ENGI, 3994.51, 4020.46, 192.18, 0.91, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 4008.35, 4035.04, 192.70);
			add = me->SummonCreature(ENTRY_ARA_GORKLONN, 4021.56, 4059.35, 193.59, 4.44, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			if (add) add->GetMotionMaster()->MovePoint(0, 4016.62, 4039.89, 193.46);
			break;
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (!someplayer)
			return;

		if (Event_Timer.Expired(diff))
		{
			switch (Phase)
			{
			case 1:
			{
				Unit* u = me->GetUnit(someplayer);
				if (u && u->GetTypeId() == TYPEID_PLAYER)
					DoScriptText(EMOTE_START, me, u);

				Event_Timer = 60000;
				Wave = true;
				++Phase;
				break;
			}
			case 2:
				DoScriptText(EMOTE_60, me);
				Event_Timer = 30000;
				++Phase;
				break;
			case 3:
				DoScriptText(EMOTE_30, me);
				Event_Timer = 20000;
				DoFinalSpawnForCreature(me);
				++Phase;
				break;
			case 4:
				DoScriptText(EMOTE_10, me);
				Event_Timer = 10000;
				Wave = false;
				++Phase;
				break;
			case 5:
				DoScriptText(EMOTE_COMPLETE, me);
				Player* player = Unit::GetPlayerInWorld(someplayer);
				if (player)
					player->KilledMonster(me->GetEntry(), me->GetGUID());
				DoCast(me, SPELL_DISABLE_VISUAL);

				if (goConsole)
				{
					if (GameObject* go = GameObject::GetGameObject((*me), goConsole))
						go->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
				}
				someplayer = 0;
				break;
			}
		}

		if (Wave)
		{
			if (Wave_Timer.Expired(diff))
				DoWaveSpawnForCreature(me);
		}
	}
};

CreatureAI* GetAI_npc_manaforge_control_console(Creature* creature)
{
	return new npc_manaforge_control_consoleAI(creature);
}

/*######
## go_manaforge_control_console
######*/

//TODO: clean up this workaround when Trinity adds support to do it properly (with gossip selections instead of instant summon)
bool GOUse_go_manaforge_control_console(Player* player, GameObject* go)
{
	if (go->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
	{
		player->PrepareQuestMenu(go->GetGUID());
		player->SendPreparedQuest(go->GetGUID());
	}

	Creature* manaforge = NULL;

	switch (go->GetAreaId())
	{
	case 3726:                                          //b'naar
		if ((player->GetQuestStatus(10299) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10329) == QUEST_STATUS_INCOMPLETE) &&
			player->HasItemCount(29366, 1))
			manaforge = go->GetMap()->GetCreatureById(ENTRY_BNAAR_C_CONSOLE);
		break;
	case 3730:                                          //coruu
		if ((player->GetQuestStatus(10321) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10330) == QUEST_STATUS_INCOMPLETE) &&
			player->HasItemCount(29396, 1))
			manaforge = go->GetMap()->GetCreatureById(ENTRY_CORUU_C_CONSOLE);
		break;
	case 3734:                                          //duro
		if ((player->GetQuestStatus(10322) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10338) == QUEST_STATUS_INCOMPLETE) &&
			player->HasItemCount(29397, 1))
			manaforge = go->GetMap()->GetCreatureById(ENTRY_DURO_C_CONSOLE);
		break;
	case 3722:                                          //ara
		if ((player->GetQuestStatus(10323) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10365) == QUEST_STATUS_INCOMPLETE) &&
			player->HasItemCount(29411, 1))
			manaforge = go->GetMap()->GetCreatureById(ENTRY_ARA_C_CONSOLE);
		break;
	}

	if (manaforge)
	{
		manaforge->AI()->Reset();
		((npc_manaforge_control_consoleAI*)manaforge->AI())->someplayer = player->GetGUID();
		((npc_manaforge_control_consoleAI*)manaforge->AI())->goConsole = go->GetGUID();
		go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
	}
	return true;
}

/*######
## npc_commander_dawnforge
######*/

// The Speech of Dawnforge, Ardonis & Pathaleon
#define SAY_COMMANDER_DAWNFORGE_1           -1000128
#define SAY_ARCANIST_ARDONIS_1              -1000129
#define SAY_COMMANDER_DAWNFORGE_2           -1000130
#define SAY_PATHALEON_CULATOR_IMAGE_1       -1000131
#define SAY_COMMANDER_DAWNFORGE_3           -1000132
#define SAY_PATHALEON_CULATOR_IMAGE_2       -1000133
#define SAY_PATHALEON_CULATOR_IMAGE_2_1     -1000134
#define SAY_PATHALEON_CULATOR_IMAGE_2_2     -1000135
#define SAY_COMMANDER_DAWNFORGE_4           -1000136
#define SAY_ARCANIST_ARDONIS_2              -1000136
#define SAY_COMMANDER_DAWNFORGE_5           -1000137

#define QUEST_INFO_GATHERING                10198
#define SPELL_SUNFURY_DISGUISE              34603
#define SPELL_FORCEFUL_CLEAVE               35473

// Entries of Arcanist Ardonis, Commander Dawnforge, Pathaleon the Curators Image
int CreatureEntry[3][1] =
{
	{19830},                                                // Ardonis
	{19831},                                                // Dawnforge
	{21504}                                                 // Pathaleon
};

struct npc_commander_dawnforgeAI : public ScriptedAI
{
	npc_commander_dawnforgeAI(Creature* creature) : ScriptedAI(creature) { Reset(); }


	uint64 playerGUID;
	uint64 ardonisGUID;
	uint64 pathaleonGUID;


	uint32 Phase;
	uint32 PhaseSubphase;
	Timer_UnCheked Phase_Timer;
	bool isEvent;

	float angle_dawnforge;
	float angle_ardonis;

	Timer forceful_cleave_Timer;

	void Reset()
	{
		playerGUID = 0;
		ardonisGUID = 0;
		pathaleonGUID = 0;

		Phase = 1;
		PhaseSubphase = 0;
		Phase_Timer.Reset(4000);
		isEvent = false;

		forceful_cleave_Timer.Reset(5000);
	}

	void EnterCombat(Unit* who) {}

	//Select any creature in a grid
	Creature* SelectCreatureInGrid(uint32 entry, float range)
	{
		Creature* creature = NULL;

		Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*me, entry, true, range, false);
		Hellground::ObjectLastSearcher<Creature, Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, creature_check);

		Cell::VisitGridObjects(me, searcher, range);

		return creature;
	}

	void JustSummoned(Creature* summoned)
	{
		pathaleonGUID = summoned->GetGUID();
	}

	// Emote Ardonis and Pathaleon
	void Turn_to_Pathaleons_Image()
	{
		Creature* ardonis = Unit::GetCreature(*me, ardonisGUID);
		Creature* pathaleon = Unit::GetCreature(*me, pathaleonGUID);
		Player* player = Unit::GetPlayerInWorld(playerGUID);

		if (!ardonis || !pathaleon || !player)
			return;

		//Calculate the angle to Pathaleon
		angle_dawnforge = me->GetOrientationTo(pathaleon->GetPositionX(), pathaleon->GetPositionY());
		angle_ardonis = ardonis->GetOrientationTo(pathaleon->GetPositionX(), pathaleon->GetPositionY());

		//Turn Dawnforge and update
		me->SetOrientation(angle_dawnforge);
		me->SendCreateUpdateToPlayer(player);
		//Turn Ardonis and update
		ardonis->SetOrientation(angle_ardonis);
		ardonis->SendCreateUpdateToPlayer(player);

		//Set them to kneel
		me->SetStandState(UNIT_STAND_STATE_KNEEL);
		ardonis->SetStandState(UNIT_STAND_STATE_KNEEL);
	}

	//Set them back to each other
	void Turn_to_eachother()
	{
		if (Unit* ardonis = Unit::GetUnit(*me, ardonisGUID))
		{
			Player* player = Unit::GetPlayerInWorld(playerGUID);

			if (!player)
				return;

			angle_dawnforge = me->GetOrientationTo(ardonis->GetPositionX(), ardonis->GetPositionY());
			angle_ardonis = ardonis->GetOrientationTo(me->GetPositionX(), me->GetPositionY());

			//Turn Dawnforge and update
			me->SetOrientation(angle_dawnforge);
			me->SendCreateUpdateToPlayer(player);
			//Turn Ardonis and update
			ardonis->SetOrientation(angle_ardonis);
			ardonis->SendCreateUpdateToPlayer(player);

			//Set state
			me->SetStandState(UNIT_STAND_STATE_STAND);
			ardonis->SetStandState(UNIT_STAND_STATE_STAND);
		}
	}

	bool CanStartEvent(Player* player)
	{
		if (!isEvent)
		{
			Creature* ardonis = SelectCreatureInGrid(CreatureEntry[0][0], 10.0f);
			if (!ardonis)
				return false;

			ardonisGUID = ardonis->GetGUID();
			playerGUID = player->GetGUID();

			isEvent = true;

			Turn_to_eachother();
			return true;
		}

		debug_log("TSCR: npc_commander_dawnforge event already in progress, need to wait.");
		return false;
	}

	void UpdateAI(const uint32 diff)
	{
		//Is event even running?
		if (!isEvent)
		{
			if (!UpdateVictim())
				return;

			if (forceful_cleave_Timer.Expired(diff))
			{
				DoCast(me->GetVictim(), SPELL_FORCEFUL_CLEAVE);
				forceful_cleave_Timer = urand(10000, 15000);
			}

			DoMeleeAttackIfReady();
			return;
		}

		//Phase timing
		if (!Phase_Timer.Expired(diff))
			return;

		Unit* ardonis = Unit::GetUnit(*me, ardonisGUID);
		Unit* pathaleon = Unit::GetUnit(*me, pathaleonGUID);
		Player* player = Unit::GetPlayerInWorld(playerGUID);

		if (!ardonis || !player)
		{
			Reset();
			return;
		}

		if (Phase > 4 && !pathaleon)
		{
			Reset();
			return;
		}

		//Phase 1 Dawnforge say
		switch (Phase)
		{
		case 1:
			DoScriptText(SAY_COMMANDER_DAWNFORGE_1, me);
			++Phase;
			Phase_Timer = 16000;
			break;
			//Phase 2 Ardonis say
		case 2:
			DoScriptText(SAY_ARCANIST_ARDONIS_1, ardonis);
			++Phase;
			Phase_Timer = 16000;
			break;
			//Phase 3 Dawnforge say
		case 3:
			DoScriptText(SAY_COMMANDER_DAWNFORGE_2, me);
			++Phase;
			Phase_Timer = 16000;
			break;
			//Phase 4 Pathaleon spawns up to phase 9
		case 4:
			//spawn pathaleon's image
			me->SummonCreature(CreatureEntry[2][0], 2325.851563, 2799.534668, 133.084229, 6.038996, TEMPSUMMON_TIMED_DESPAWN, 90000);
			++Phase;
			Phase_Timer = 500;
			break;
			//Phase 5 Pathaleon say
		case 5:
			DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_1, pathaleon);
			++Phase;
			Phase_Timer = 6000;
			break;
			//Phase 6
		case 6:
			switch (PhaseSubphase)
			{
				//Subphase 1: Turn Dawnforge and Ardonis
			case 0:
				Turn_to_Pathaleons_Image();
				++PhaseSubphase;
				Phase_Timer = 8000;
				break;
				//Subphase 2 Dawnforge say
			case 1:
				DoScriptText(SAY_COMMANDER_DAWNFORGE_3, me);
				PhaseSubphase = 0;
				++Phase;
				Phase_Timer = 8000;
				break;
			}
			break;
			//Phase 7 Pathaleons say 3 Sentence, every sentence need a subphase
		case 7:
			switch (PhaseSubphase)
			{
				//Subphase 1
			case 0:
				DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_2, pathaleon);
				++PhaseSubphase;
				Phase_Timer = 12000;
				break;
				//Subphase 2
			case 1:
				DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_2_1, pathaleon);
				++PhaseSubphase;
				Phase_Timer = 16000;
				break;
				//Subphase 3
			case 2:
				DoScriptText(SAY_PATHALEON_CULATOR_IMAGE_2_2, pathaleon);
				PhaseSubphase = 0;
				++Phase;
				Phase_Timer = 10000;
				break;
			}
			break;
			//Phase 8 Dawnforge & Ardonis say
		case 8:
			DoScriptText(SAY_COMMANDER_DAWNFORGE_4, me);
			DoScriptText(SAY_ARCANIST_ARDONIS_2, ardonis);
			++Phase;
			Phase_Timer = 4000;
			break;
			//Phase 9 Pathaleons Despawn, Reset Dawnforge & Ardonis angle
		case 9:
			Turn_to_eachother();
			//hide pathaleon, unit will despawn shortly
			pathaleon->SetVisibility(VISIBILITY_OFF);
			PhaseSubphase = 0;
			++Phase;
			Phase_Timer = 3000;
			break;
			//Phase 10 Dawnforge say
		case 10:
			DoScriptText(SAY_COMMANDER_DAWNFORGE_5, me);
			player->AreaExploredOrEventHappens(QUEST_INFO_GATHERING);
			Reset();
			break;
		}
	}
};

CreatureAI* GetAI_npc_commander_dawnforge(Creature* creature)
{
	return new npc_commander_dawnforgeAI(creature);
}

Creature* SearchDawnforge(Player* source, uint32 entry, float range)
{
	Creature* creature = NULL;

	Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*source, entry, true, range, false);
	Hellground::ObjectLastSearcher<Creature, Hellground::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, creature_check);

	Cell::VisitGridObjects(source, searcher, range);

	return creature;
}

bool AreaTrigger_at_commander_dawnforge(Player* player, AreaTriggerEntry const*at)
{
	//if player lost aura or not have at all, we should not try start event.
	if (!player->HasAura(SPELL_SUNFURY_DISGUISE, 0))
		return false;

	if (player->isAlive() && player->GetQuestStatus(QUEST_INFO_GATHERING) == QUEST_STATUS_INCOMPLETE)
	{
		Creature* Dawnforge = SearchDawnforge(player, CreatureEntry[1][0], 30.0f);

		if (!Dawnforge)
			return false;

		if (((npc_commander_dawnforgeAI*)Dawnforge->AI())->CanStartEvent(player))
			return true;
	}
	return false;
}

/*######
## npc_protectorate_nether_drake
######*/

#define GOSSIP_ITEM_PROTECTOORATE 16487

bool GossipHello_npc_protectorate_nether_drake(Player* player, Creature* creature)
{
	//On Nethery Wings
	if (player->GetQuestStatus(10438) == QUEST_STATUS_INCOMPLETE && player->HasItemCount(29778, 1))
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_PROTECTOORATE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

	player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

	return true;
}

bool GossipSelect_npc_protectorate_nether_drake(Player* player, Creature* creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_INFO_DEF + 1)
	{
		player->CLOSE_GOSSIP_MENU();

		std::vector<uint32> nodes;

		nodes.resize(2);
		nodes[0] = 152;                                     //from drake
		nodes[1] = 153;                                     //end at drake
		player->ActivateTaxiPathTo(nodes);                  //TaxiPath 627 (possibly 627+628(152->153->154->155) )
	}
	return true;
}

/*######
## npc_professor_dabiri
######*/

#define SPELL_PHASE_DISTRUPTOR  35780
#define GOSSIP_ITEM_DABIRI 16488
#define WHISPER_DABIRI -1000302

#define QUEST_DIMENSIUS 10439
#define QUEST_ON_NETHERY_WINGS 10438

bool GossipHello_npc_professor_dabiri(Player* player, Creature* creature)
{
	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());

	if (player->GetQuestStatus(QUEST_ON_NETHERY_WINGS) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(29778, 1))
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_DABIRI), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

	player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

	return true;
}

bool GossipSelect_npc_professor_dabiri(Player* player, Creature* creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_INFO_DEF + 1)
	{
		creature->CastSpell(player, SPELL_PHASE_DISTRUPTOR, false);
		player->CLOSE_GOSSIP_MENU();
	}

	return true;
}

bool QuestAccept_npc_professor_dabiri(Player* player, Creature* creature, Quest const*quest)
{
	if (quest->GetQuestId() == QUEST_DIMENSIUS)
		DoScriptText(WHISPER_DABIRI, creature, player);

	return true;
}

/*######
## npc_veronia
######*/

#define GOSSIP_HV 16489

bool GossipHello_npc_veronia(Player* player, Creature* creature)
{
	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());

	//Behind Enemy Lines
	if (player->GetQuestStatus(10652) && !player->GetQuestRewardStatus(10652))
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HV), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

	player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

	return true;
}

bool GossipSelect_npc_veronia(Player* player, Creature* creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_INFO_DEF)
	{
		player->CLOSE_GOSSIP_MENU();
		player->CastSpell(player, 34905, true);               //TaxiPath 606
	}
	return true;
}

/*######
## mob_phase_hunter
######*/

#define SUMMONED_MOB            19595
#define EMOTE_WEAK              -1000303

// Spells
#define SPELL_PHASE_SLIP        36574
#define SPELL_MANA_BURN         13321
#define SPELL_MATERIALIZE       34804
#define SPELL_DE_MATERIALIZE    34804

struct mob_phase_hunterAI : public ScriptedAI
{

	mob_phase_hunterAI(Creature* creature) : ScriptedAI(creature) {}

	bool Weak;
	bool Materialize;
	bool Drained;

	int WeakPercent;
	uint64 PlayerGUID;
	uint32 Health;
	uint32 Level;
	Timer_UnCheked PhaseSlipVulnerabilityTimer;
	Timer_UnCheked ManaBurnTimer;

	void Reset()
	{
		Weak = false;
		Materialize = false;
		Drained = false;

		WeakPercent = 25 + (rand() % 16); // 25-40
		PlayerGUID = 0;
		ManaBurnTimer.Reset(5000 + (rand() % 3 * 1000)); // 5-8 sec cd
	}

	void EnterCombat(Unit* who)
	{
		if (Player* player = who->GetCharmerOrOwnerPlayerOrPlayerItself())
			PlayerGUID = player->GetGUID();
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		DoCast(me, SPELL_DE_MATERIALIZE);
	}

	void UpdateAI(const uint32 diff)
	{
		if (!Materialize)
		{
			DoCast(me, SPELL_MATERIALIZE);
			Materialize = true;
		}

		if (me->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) || me->HasUnitState(UNIT_STAT_ROOT)) // if the mob is rooted/slowed by spells eg.: Entangling Roots, Frost Nova, Hamstring, Crippling Poison, etc. => remove it
			DoCast(me, SPELL_PHASE_SLIP);

		if (!UpdateVictim())
			return;

		if (ManaBurnTimer.Expired(diff)) // cast Mana Burn
		{
			if (me->GetVictim()->GetCreateMana() > 0)
			{
				DoCast(me->GetVictim(), SPELL_MANA_BURN);
				ManaBurnTimer = 8000 + (rand() % 10 * 1000); // 8-18 sec cd
			}
		}

		if (PlayerGUID) // start: support for quest 10190
		{
			Player* target = Unit::GetPlayerInWorld(PlayerGUID);

			if (target && !Weak && me->GetHealth() < (me->GetMaxHealth() / 100 * WeakPercent)
				&& target->GetQuestStatus(10190) == QUEST_STATUS_INCOMPLETE)
			{
				DoScriptText(EMOTE_WEAK, me);
				Weak = true;
			}

			if (Weak && !Drained && me->HasAura(34219, 0))
			{
				Drained = true;

				Health = me->GetHealth(); // get the normal mob's data
				Level = me->GetLevel();

				me->AttackStop(); // delete the normal mob
				me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
				me->RemoveCorpse();

				Creature* DrainedPhaseHunter = NULL;

				if (!DrainedPhaseHunter)
					DrainedPhaseHunter = me->SummonCreature(SUMMONED_MOB, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000); // summon the mob

				if (DrainedPhaseHunter)
				{
					DrainedPhaseHunter->SetLevel(Level); // set the summoned mob's data
					DrainedPhaseHunter->SetHealth(Health);
					DrainedPhaseHunter->LowerPlayerDamageReq(me->GetMaxHealth() - Health); // there is no credit for killing mob with such a little hp, so...
					DrainedPhaseHunter->AddThreat(target, 10000.0f);
					DrainedPhaseHunter->AI()->AttackStart(target);
				}
			}
		}// end: support for quest 10190

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_phase_hunter(Creature* creature)
{
	return new mob_phase_hunterAI(creature);
}

/*######
## npc_bessy
######*/

#define Q_ALMABTRIEB    10337
#define N_THADELL       20464
#define SPAWN_FIRST     20512
#define SPAWN_SECOND    19881
#define SAY_THADELL_1   -1000304
#define SAY_THADELL_2   -1000305

struct npc_bessyAI : public npc_escortAI
{

	npc_bessyAI(Creature* creature) : npc_escortAI(creature) {}

	void JustDied(Unit* killer)
	{
		if (Player* pPlayer = GetPlayerForEscort())
			pPlayer->FailQuest(Q_ALMABTRIEB);
	}

	void WaypointReached(uint32 i)
	{
		Player* player = GetPlayerForEscort();

		if (!player)
			return;

		switch (i)
		{
		case 3: //first spawn
			me->SummonCreature(SPAWN_FIRST, 2449.67, 2183.11, 96.85, 6.20, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
			me->SummonCreature(SPAWN_FIRST, 2449.53, 2184.43, 96.36, 6.27, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
			me->SummonCreature(SPAWN_FIRST, 2449.85, 2186.34, 97.57, 6.08, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
			break;

		case 7:
			me->SummonCreature(SPAWN_SECOND, 2309.64, 2186.24, 92.25, 6.06, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
			me->SummonCreature(SPAWN_SECOND, 2309.25, 2183.46, 91.75, 6.22, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
			break;

		case 12:
		{
			if (player)
				player->GroupEventHappens(Q_ALMABTRIEB, me);

			if (Unit* Thadell = FindCreature(N_THADELL, 30, me))
				DoScriptText(SAY_THADELL_1, me);
			break;
		}
		case 13:
		{
			Unit* Thadell = FindCreature(N_THADELL, 30, me);
			if (Thadell)
				DoScriptText(SAY_THADELL_2, me, player);
		}
		break;
		}
	}

	void JustSummoned(Creature* summoned)
	{
		summoned->AI()->AttackStart(me);
	}

	void EnterCombat(Unit* who) {}

	void Reset()
	{
		me->RestoreFaction();
	}

};

bool QuestAccept_npc_bessy(Player* player, Creature* creature, Quest const* quest)
{
	if (quest->GetQuestId() == Q_ALMABTRIEB)
	{
		creature->setFaction(113);
		creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
		if (npc_escortAI* pEscortAI = CAST_AI(npc_bessyAI, creature->AI()))
			pEscortAI->Start(true, true, player->GetGUID(), quest);
	}
	return true;
}

CreatureAI* GetAI_npc_bessy(Creature* creature)
{
	return new npc_bessyAI(creature);
}

/***
Script for Quest: Creatures of the Eco-Domes (10427)
***/
struct mob_talbukAI : public ScriptedAI
{
	mob_talbukAI(Creature* creature) : ScriptedAI(creature) {}

	Timer_UnCheked Tagged_Timer;

	void Reset()
	{
		me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
		me->ClearUnitState(UNIT_STAT_STUNNED);
		me->RemoveAurasDueToSpell(SPELL_SLEEP_VISUAL);
		Tagged_Timer.Reset(60000);
	}

	void EnterCombat(Unit* who) {}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim() && !me->HasAura(SPELL_SLEEP_VISUAL, 0))
			return;

		if (me->HasAura(SPELL_SLEEP_VISUAL, 0)) // Sleep Visual
		{
			if (Tagged_Timer.Expired(diff)) // Remove every effect caused by aura and reset creature.
			{
				me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
				me->ClearUnitState(UNIT_STAT_STUNNED);
				me->RemoveAurasDueToSpell(SPELL_SLEEP_VISUAL);
				EnterEvadeMode();
			}
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_talbuk(Creature* creature)
{
	return new mob_talbukAI(creature);
}

/***
Script for Quest: The Flesh Lies... (10345)
***/
struct npc_withered_corpseAI : public ScriptedAI
{
	npc_withered_corpseAI(Creature* creature) : ScriptedAI(creature) {}

	void Reset()
	{
		// makes creature appear dead
		me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);
	}

	void EnterCombat(Unit* who) {}

	void MoveInLineOfSight(Unit* who)
	{
		// summon Parasitic Fleshbeast(20335) when player gets very close, and then remove NPC
		if (who->GetTypeId() == TYPEID_PLAYER && me->IsWithinMeleeRange(who))
		{
			me->SummonCreature(20335, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
			me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
			me->RemoveCorpse();
		}
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == 35372)
		{
			me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
			me->RemoveCorpse();
		}
	}
};

CreatureAI* GetAI_npc_withered_corpse(Creature* creature)
{
	return new npc_withered_corpseAI(creature);
}

/*######
## go_ethereum_prison
######*/

float ethereum_NPC[2][7] =
{
	{20785,20790,20789,20784,20786,20783,20788}, // hostile npc
	{22810,22811,22812,22813,22814,22815,0}      // fiendly npc (need script in acid ? only to cast spell reputation reward)
};

bool GOUse_go_ethereum_prison(Player* player, GameObject* go)
{
	uint32 entry;
	switch (rand() % 2)
	{
	case 0:
		entry = ethereum_NPC[0][rand() % 7];
		go->SummonCreature(entry, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ() + 0.3, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);
		break;
	case 1:
		entry = ethereum_NPC[1][rand() % 6];
		if (Creature* prisoner = go->SummonCreature(entry, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ() + 0.3, 0, TEMPSUMMON_TIMED_DESPAWN, 15000))
		{
			int32 spellId = 0;
			switch (prisoner->GetEntry())
			{
			case 22810: spellId = 39460; break;  // Cenarion Expedition +500
			case 22811: spellId = 39456; break;  // Lower City +500
			case 22812: spellId = 39457; break;  // Sha'tar +500
			case 22813: spellId = 39474; break;  // Consortium +500
			case 22814: spellId = 39476; break;  // Sporegar +500
			case 22815: spellId = 39475; break;  // Keepers of Time +500
			}
			prisoner->CastSpell(player, spellId, false, NULL, NULL, 0);
		}
		break;
	}

	go->UseDoorOrButton(5 * MINUTE);

	return true;
}

/***
Script for Quest: Bloody Imp-ossible! (10924)
***/

struct npc_warp_chaserAI : public ScriptedAI
{
	npc_warp_chaserAI(Creature* creature) : ScriptedAI(creature) {}

	void JustDied(Unit* slayer)
	{
		Player* plr = slayer->GetCharmerOrOwnerPlayerOrPlayerItself();
		if (!plr)
			return;
		Unit* zeppit = plr->GetMiniPet();
		if (!zeppit || zeppit->GetEntry() != 22484)
			return;

		zeppit->CastSpell(zeppit, 39244, true);
	}

	void EnterCombat(Unit* who) {}

	void Reset()
	{

	}
};

CreatureAI* GetAI_npc_warp_chaser(Creature *creature)
{
	return new npc_warp_chaserAI(creature);
}

/*######
## Script for Quest: Elemental Power Extraction
######*/

// Spells
#define SPELL_EPEXTRACTOR       34520
#define SPELL_CREATE_EPOWER     34525
#define SPELL_SUMMON_SHARD      35310
#define ENTRY_RUMBLER           18881

struct mob_epextractionAI : public ScriptedAI
{

	mob_epextractionAI(Creature* creature) : ScriptedAI(creature) {}

	bool PowerExtracted;

	void Reset()
	{
		PowerExtracted = false;
	}

	void EnterCombat(Unit* who) {}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == SPELL_EPEXTRACTOR)
			PowerExtracted = true;
	}

	void JustDied(Unit* killer)
	{
		if (me->GetEntry() == ENTRY_RUMBLER)
			me->CastSpell(me, SPELL_SUMMON_SHARD, true);

		if (PowerExtracted)
			killer->CastSpell(me, SPELL_CREATE_EPOWER, true);
	}
};

CreatureAI* GetAI_mob_epextraction(Creature* creature)
{
	return new mob_epextractionAI(creature);
}

/*######
## npc_dr_boom
######*/

enum
{
	THROW_DYNAMITE = 35276,
	BOOM_BOT = 19692,
	BOOM_BOT_TARGET = 20392
};

struct mob_dr_boomAI : public ScriptedAI
{
	mob_dr_boomAI(Creature* creature) : ScriptedAI(creature) {}

	std::vector<uint64> targetGUID;

	Timer SummonTimer;

	void Reset()
	{
		SummonTimer.Reset(2000);

		std::list<Creature*> temp = FindAllCreaturesWithEntry(BOOM_BOT_TARGET, 30.0f);

		targetGUID.clear();

		for (std::list<Creature*>::iterator it = temp.begin(); it != temp.end(); it++)
			targetGUID.push_back((*it)->GetGUID());
	}

	void UpdateAI(const uint32 diff)
	{
		if (SummonTimer.Expired(diff))
		{
			if (targetGUID.size())
			{
				if (Unit* target = Unit::GetUnit(*me, targetGUID[rand() % targetGUID.size()]))
				{
					if (Unit* bot = DoSpawnCreature(BOOM_BOT, 0, 0, 0, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000))
						bot->GetMotionMaster()->MovePoint(0, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
				}
			}
			else
				Reset();

			SummonTimer = 2000;
		}

		if (!UpdateVictim())
			return;

		if (!me->IsWithinDistInMap(me->GetVictim(), 23.0f))
		{
			EnterEvadeMode();
			return;
		}

		if (me->isAttackReady() && me->IsWithinDistInMap(me->GetVictim(), 12.2f))
		{
			DoCast(me->GetVictim(), THROW_DYNAMITE, true);
			me->resetAttackTimer();
		}
	}
};

CreatureAI* GetAI_mob_dr_boom(Creature* creature)
{
	return new mob_dr_boomAI(creature);
}

/*######
## mob_boom_bot
######*/

#define    SPELL_BOOM    35132 

struct mob_boom_botAI : public ScriptedAI
{
	mob_boom_botAI(Creature* creature) : ScriptedAI(creature) {}

	void Reset()
	{
		me->SetWalk(true);
	}

	void EnterCombat(Unit* who) { return; }
	void AttackedBy(Unit* who) { return; }
	void AttackStart(Unit* who) { return; }

	void MovementInform(uint32 type, uint32 id)
	{
		if (type != POINT_MOTION_TYPE)
			return;

		DoCast(me, SPELL_BOOM, true);
		me->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN);
		me->ForcedDespawn(2000);
	}

	void MoveInLineOfSight(Unit* who)
	{
		if (!who->isCharmedOwnedByPlayerOrPlayer())
			return;

		if (me->IsWithinDistInMap(who, 1.0f, false))
		{
			DoCast(me, SPELL_BOOM, true);
			me->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN);
			me->ForcedDespawn(2000);
		}
	}
};

CreatureAI* GetAI_mob_boom_bot(Creature* creature)
{
	return new mob_boom_botAI(creature);
}

/*######
## npc_maxx_a_million
######*/

enum
{
	QUEST_MARK_V_IS_ALIVE = 10191,
	NPC_BOT_SPECIALIST_ALLEY = 19578,
	GO_DRAENEI_MACHINE = 183771,

	SAY_START = -1000575,
	SAY_ALLEY_FAREWELL = -1000576,
	SAY_CONTINUE = -1000577,
	SAY_ALLEY_FINISH = -1000578
};

struct npc_maxx_a_million_escortAI : public npc_escortAI
{
	npc_maxx_a_million_escortAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

	Timer m_uiSubEventTimer;
	uint8 m_uiSubEvent;

	void Reset()
	{
		if (!HasEscortState(STATE_ESCORT_ESCORTING))
		{
			m_uiSubEventTimer.Reset(0);
			m_uiSubEvent = 0;
			me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
		}
	}

	void WaypointReached(uint32 uiPoint)
	{
		switch (uiPoint)
		{
		case 1:
			me->SetOrientation(5.4f);
			DoScriptText(SAY_START, me);
			m_uiSubEventTimer = 3000;
			m_uiSubEvent = 1;
			me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
			me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
			break;
		case 7:
		case 17:
		case 29:
			if (GameObject* pGO = FindGameObject(GO_DRAENEI_MACHINE, INTERACTION_DISTANCE, me))
			{
				me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_ATTACKUNARMED);
				m_uiSubEvent = 2;
				m_uiSubEventTimer = 2500;
			}
			break;
		case 36:
			if (Player* pPlayer = GetPlayerForEscort())
				pPlayer->GroupEventHappens(QUEST_MARK_V_IS_ALIVE, me);
			if (Unit* pAlley = FindCreature(NPC_BOT_SPECIALIST_ALLEY, INTERACTION_DISTANCE * 2, me))
				DoScriptText(SAY_ALLEY_FINISH, pAlley);
			break;
		}
	}

	void WaypointStart(uint32 uiPoint)
	{
		switch (uiPoint)
		{
		case 8:
		case 18:
		case 30:
			DoScriptText(SAY_CONTINUE, me);
			break;
		}
	}

	void UpdateEscortAI(const uint32 diff)
	{
		if (m_uiSubEventTimer.Expired(diff))
		{
			switch (m_uiSubEvent)
			{
			case 1:
				if (Unit* pAlley = FindCreature(NPC_BOT_SPECIALIST_ALLEY, INTERACTION_DISTANCE * 2, me))
					DoScriptText(SAY_ALLEY_FAREWELL, pAlley);
				break;
			case 2:
				if (GameObject* pGO = FindGameObject(GO_DRAENEI_MACHINE, INTERACTION_DISTANCE, me))
				{
					if (Player* pPlayer = GetPlayerForEscort())
						pGO->DestroyForPlayer(pPlayer);
					me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_AT_EASE);
				}
				break;
			}
			m_uiSubEventTimer = 0;
			m_uiSubEvent = 0;
		}

		if (UpdateVictim())
			DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_maxx_a_million(Creature* creature)
{
	return new npc_maxx_a_million_escortAI(creature);
}

bool QuestAccept_npc_maxx_a_million(Player* pPlayer, Creature* creature, const Quest* quest)
{
	if (quest->GetQuestId() == QUEST_MARK_V_IS_ALIVE)
	{
		if (npc_maxx_a_million_escortAI* pEscortAI = dynamic_cast<npc_maxx_a_million_escortAI*>(creature->AI()))
		{
			if (pPlayer->GetTeam() == ALLIANCE)
				creature->setFaction(FACTION_ESCORT_A_NEUTRAL_ACTIVE);
			else
				creature->setFaction(FACTION_ESCORT_H_NEUTRAL_ACTIVE);

			pEscortAI->Start(true, false, pPlayer->GetGUID(), quest, true);
		}
	}
	return true;
}

/*######
## npc_scrapped_reaver
######*/

enum
{
	NPC_ZAXXIS = 20287,

	SPELL_ZAPPER = 35282
};

struct npc_scrapped_reaverAI : public ScriptedAI
{
	npc_scrapped_reaverAI(Creature* creature) : ScriptedAI(creature), zaxxis(me) {}

	bool Ambush;

	SummonList zaxxis;
	Timer ZaxxTimer;

	void Reset()
	{
		Ambush = false;
		ZaxxTimer.Reset(10000);
		me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == SPELL_ZAPPER && caster->GetTypeId() == TYPEID_PLAYER)
		{
			if (((Player*)caster)->GetQuestStatus(10309) == QUEST_STATUS_INCOMPLETE)
			{
				ScriptedAI::AttackStart(caster);
				Ambush = true;
			}
		}
	}

	void SpawnZaxx()
	{
		float angle = RAND(2.3f, 3.4f, 4.8f);
		float x, y, z;

		me->GetNearPoint(x, y, z, 0.0f, 35.0f, angle);
		me->SummonCreature(NPC_ZAXXIS, x, y, z + 2, me->GetOrientationTo(x, y), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 7000);
	}

	void JustSummoned(Creature* summoned)
	{
		zaxxis.Summon(summoned);

		if (summoned->GetEntry() == NPC_ZAXXIS)
		{
			if (me->GetVictim())
			{
				summoned->AI()->AttackStart(me->GetVictim());
			}
			else summoned->GetMotionMaster()->MoveChase(me);
		}
	}

	void JustDied(Unit* killer)
	{
		zaxxis.DespawnAll();
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (Ambush)
		{
			if (ZaxxTimer.Expired(diff))
			{
				SpawnZaxx();

				if (urand(0, 99) > 53)
					SpawnZaxx();

				ZaxxTimer = 24000;
			}
		}
	}
};

CreatureAI* GetAI_npc_scrapped_reaver(Creature* creature)
{
	return new npc_scrapped_reaverAI(creature);
}

/*######
## npc_drijya
######*/

enum
{
	SAY_DR_START = -1900156,
	SAY_DR_1 = -1900157,
	SAY_DR_2 = -1900158,
	SAY_DR_3 = -1900159,
	SAY_DR_4 = -1900160,
	SAY_DR_5 = -1900161,
	SAY_DR_6 = -1900162,
	SAY_DR_7 = -1900163,
	SAY_DR_COMPLETE = -1900164,

	QUEST_WARP_GATE = 10310,

	MAX_TROOPER = 9,
	MAX_IMP = 6,

	NPC_IMP = 20399,
	NPC_TROOPER = 20402,
	NPC_DESTROYER = 20403,

	GO_SMOKE = 185318,
	GO_FIRE = 185317,
	GO_BIG_FIRE = 185319
};

struct Pos
{
	float x, y, z;
};

static Pos S[] =
{
	{3025.752f, 2715.122f, 113.758f},
	{3019.741f, 2720.757f, 115.189f},
	{3049.354f, 2726.431f, 113.922f},
	{3020.842f, 2697.501f, 113.368f},
	{3008.503f, 2729.432f, 114.350f,},
	{3026.163f, 2723.538f, 113.681f},
	{3021.556f, 2718.887f, 115.055f}
};

struct npc_drijyaAI : public npc_escortAI
{
	npc_drijyaAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

	bool Destroy;
	bool SummonImp;
	bool SummonTrooper;
	bool SummonDestroyer;

	uint32 Count;
	Timer_UnCheked SpawnTimer;
	Timer_UnCheked StartSpawnTimer;
	Timer_UnCheked DestroyingTimer;

	void Reset()
	{
		Destroy = false;
		SummonImp = false;
		SummonTrooper = false;
		SummonDestroyer = false;
		Count = 0;
		SpawnTimer.Reset(3500);
		StartSpawnTimer.Reset(15000);
		DestroyingTimer.Reset(60000);
		me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
	}

	void AttackedBy(Unit* who) {}
	void AttackStart(Unit* who) {}

	void JustSummoned(Creature* summoned)
	{
		if (Player* player = GetPlayerForEscort())
			summoned->AddThreat(player, 0.0f);
		summoned->AI()->AttackStart(me);
	}

	void WaypointReached(uint32 i)
	{
		switch (i)
		{
		case 0:
			DoScriptText(SAY_DR_START, me);
			SetRun();
			break;
		case 1:
			DoScriptText(SAY_DR_1, me);
			break;
		case 5:
			DoScriptText(SAY_DR_2, me);
			break;
		case 7:
			SetEscortPaused(true);
			me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_NOSHEATHE);
			Destroy = true;
			SummonImp = true;
			break;
		case 8:
			me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
			me->SummonGameObject(GO_SMOKE, S[2].x, S[2].y, S[2].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 215);
			DoScriptText(SAY_DR_4, me);
			break;
		case 12:
			SetEscortPaused(true);
			me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_NOSHEATHE);
			Destroy = true;
			SummonTrooper = true;
			break;
		case 13:
			me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
			me->SummonGameObject(GO_SMOKE, S[3].x, S[3].y, S[3].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 140);
			DoScriptText(SAY_DR_5, me);
			break;
		case 17:
			SetEscortPaused(true);
			me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_NOSHEATHE);
			Destroy = true;
			SummonDestroyer = true;
			break;
		case 18:
			me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
			DoScriptText(SAY_DR_6, me);
			break;
		case 19:
			DoScriptText(SAY_DR_7, me);
			me->SummonGameObject(GO_SMOKE, S[4].x, S[4].y, S[4].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 60);
			me->SummonGameObject(GO_FIRE, S[5].x, S[5].y, S[5].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 60);
			me->SummonGameObject(GO_BIG_FIRE, S[6].x, S[6].y, S[6].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 60);
			break;
		case 22:
			SetRun(false);
			break;
		case 26:
			if (Player* player = GetPlayerForEscort())
			{
				DoScriptText(SAY_DR_COMPLETE, me, player);
				player->GroupEventHappens(QUEST_WARP_GATE, me);
			}
			break;
		}
	}

	void UpdateEscortAI(const uint32 diff)
	{
		if (SummonImp)
		{
			if (StartSpawnTimer.Expired(diff))
			{
				if (SpawnTimer.Expired(diff))
				{
					if (Count >= MAX_IMP)
					{
						DoScriptText(SAY_DR_3, me);
						SummonImp = false;
						StartSpawnTimer = 15000;
					}
					SpawnTimer = 3500;
					++Count;
					me->SummonCreature(NPC_IMP, S[0].x, S[0].y, S[0].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
				}
			}
		}

		if (SummonTrooper)
		{
			if (StartSpawnTimer.Expired(diff))
			{
				if (SpawnTimer.Expired(diff))
				{
					if (Count >= MAX_TROOPER)
					{
						SummonTrooper = false;
						StartSpawnTimer = 15000;
					}
					SpawnTimer = 3500;
					++Count;
					me->SummonCreature(NPC_TROOPER, S[0].x, S[0].y, S[0].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
				}
			}
		}

		if (SummonDestroyer)
		{
			if (StartSpawnTimer.Expired(diff))
			{
				me->SummonCreature(NPC_DESTROYER, S[1].x, S[1].y, S[1].z, 2.5f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
				SummonDestroyer = false;
				StartSpawnTimer = 15000;
			}
		}

		if (Destroy)
		{
			if (DestroyingTimer.Expired(diff))
			{
				SetEscortPaused(false);
				Destroy = false;
				DestroyingTimer = 60000;
			}
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_drijya(Creature* creature)
{
	return new npc_drijyaAI(creature);
}

bool QuestAccept_npc_drijya(Player* player, Creature* creature, const Quest* quest)
{
	if (quest->GetQuestId() == QUEST_WARP_GATE)
	{
		if (npc_drijyaAI* escortAI = dynamic_cast<npc_drijyaAI*>(creature->AI()))
		{
			creature->setFaction(FACTION_ESCORT_N_NEUTRAL_PASSIVE);
			escortAI->Start(false, false, player->GetGUID(), quest, true);
		}
	}
	return true;
}

/*######
## npc_captured_vanguard
######*/

enum
{
	SAY_VANGUARD_INTRO = -1900202,
	SAY_VANGUARD_START = -1900203,
	SAY_VANGUARD_FINISH = -1900204,
	EMOTE_VANGUARD_FINISH = -1900205,

	SPELL_ETHEREAL_TELEPORT = 34427,
	SPELL_GLAIVE = 36500,
	SPELL_HAMSTRING = 31553,

	NPC_GLADIATOR = 20854,
	NPC_VANGUARD = 20763,
	NPC_COMMANDER_AMEER = 20448,

	QUEST_ESCAPE_STAGING_GROUNDS = 10425,
};

struct npc_captured_vanguardAI : public npc_escortAI
{
	npc_captured_vanguardAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

	bool CantStart;

	Timer_UnCheked GlaiveTimer;
	Timer_UnCheked HamstringTimer;
	Timer_UnCheked EndTimer;

	void Reset()
	{
		CantStart = false;
		GlaiveTimer.Reset(urand(4000, 8000));
		HamstringTimer.Reset(urand(8000, 13000));
		EndTimer = 0;
	}

	void WaypointReached(uint32 i)
	{
		switch (i)
		{
		case 15:
			if (Player* pPlayer = GetPlayerForEscort())
				pPlayer->GroupEventHappens(QUEST_ESCAPE_STAGING_GROUNDS, me);
			break;
		case 16:
			DoScriptText(SAY_VANGUARD_FINISH, me);
			SetRun();
			break;
		case 17:
			if (Creature* Ameer = GetClosestCreatureWithEntry(me, NPC_COMMANDER_AMEER, 5.0f))
				DoScriptText(EMOTE_VANGUARD_FINISH, me, Ameer);
			break;
		case 18:
			DoCast(me, SPELL_ETHEREAL_TELEPORT);
			me->ForcedDespawn(1000);
			break;
		}
	}

	void JustReachedHome()
	{
		if (!HasEscortState(STATE_ESCORT_ESCORTING))
		{
			DoScriptText(SAY_VANGUARD_INTRO, me);
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
			EndTimer = 40000;
			CantStart = true;
		}
	}

	void UpdateAI(const uint32 diff)
	{
		npc_escortAI::UpdateAI(diff);

		if (CantStart)
		{
			if (EndTimer.Expired(diff))
			{
				if (HasEscortState(STATE_ESCORT_ESCORTING))
				{
					CantStart = false;
				}
				else
				{
					DoCast(me, SPELL_ETHEREAL_TELEPORT);
					me->ForcedDespawn(1500);
				}
			}
		}

		if (!me->GetVictim())
			return;

		if (GlaiveTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), SPELL_GLAIVE);
			GlaiveTimer = urand(5000, 9000);
		}

		if (HamstringTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), SPELL_HAMSTRING);
			HamstringTimer = urand(10000, 16000);
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_captured_vanguard(Creature* creature)
{
	return new npc_captured_vanguardAI(creature);
}

bool QuestAccept_npc_captured_vanguard(Player* player, Creature* creature, const Quest* quest)
{
	if (quest->GetQuestId() == QUEST_ESCAPE_STAGING_GROUNDS)
	{
		if (npc_captured_vanguardAI* pEscortAI = dynamic_cast<npc_captured_vanguardAI*>(creature->AI()))
			pEscortAI->Start(true, false, player->GetGUID(), quest);

		DoScriptText(SAY_VANGUARD_START, creature, player);
	}

	return true;
}

/*######
## npc_controller
######*/

struct npc_controllerAI : public ScriptedAI
{
	npc_controllerAI(Creature* creature) : ScriptedAI(creature) {}

	bool CanSpawn;

	void Reset()
	{
		CanSpawn = true;
		me->setFaction(35);
		me->SetVisibility(VISIBILITY_OFF);
	}

	void MoveInLineOfSight(Unit* who)
	{
		if (who->GetTypeId() == TYPEID_PLAYER && !((Player*)who)->GetQuestRewardStatus(QUEST_ESCAPE_STAGING_GROUNDS))
		{
			if (me->IsWithinDistInMap(((Player *)who), 8.0f))
			{
				DoSpawn();
				CanSpawn = false;
				me->ForcedDespawn(120000);
			}
		}
	}

	void DoSpawn()
	{
		if (CanSpawn)
		{
			me->SummonCreature(NPC_GLADIATOR, 4055.65f, 2322.45f, 112.39f, 3.1f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 7000);
			me->SummonCreature(NPC_VANGUARD, 4055.96f, 2296.44f, 113.29f, 0.8f, TEMPSUMMON_CORPSE_DESPAWN, 7000);
		}
	}

	void JustSummoned(Creature* summoned)
	{
		if (summoned->GetEntry() == NPC_VANGUARD)
		{
			if (Creature* Gladiator = GetClosestCreatureWithEntry(me, NPC_GLADIATOR, 20.0f))
				summoned->AI()->AttackStart(Gladiator);
		}

		summoned->CastSpell(summoned, SPELL_ETHEREAL_TELEPORT, true);
	}
};

CreatureAI* GetAI_npc_controller(Creature* creature)
{
	return new npc_controllerAI(creature);
}

/*######
## npc_protectorate_demolitionist
######*/

enum
{
	SAY_INTRO = -1900206,
	SAY_ATTACKED_1 = -1900207,
	SAY_ATTACKED_2 = -1900208,
	SAY_STAGING_GROUNDS = -1900209,
	SAY_TOXIC_HORROR = -1900210,
	SAY_SALHADAAR = -1900211,
	SAY_DISRUPTOR = -1900212,
	SAY_NEXUS_PROTECT = -1900213,
	SAY_FINISH_1 = -1900214,
	SAY_FINISH_2 = -1900215,

	SPELL_PROTECTORATE = 35679,

	NPC_NEXUS_STALKER = 20474,
	NPC_ARCHON = 20458,

	QUEST_DELIVERING_MESSAGE = 10406
};

struct npc_protectorate_demolitionistAI : public npc_escortAI
{
	npc_protectorate_demolitionistAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

	Timer_UnCheked EventTimer;
	uint8 EventStage;

	void Reset()
	{
		if (!HasEscortState(STATE_ESCORT_ESCORTING))
		{
			EventTimer = 0;
			EventStage = 0;
		}
	}

	void EnterCombat(Unit* who)
	{
		DoScriptText(urand(0, 1) ? SAY_ATTACKED_1 : SAY_ATTACKED_2, me);
	}

	void AttackStart(Unit* who) {}

	void MoveInLineOfSight(Unit* who)
	{
		if (HasEscortState(STATE_ESCORT_ESCORTING))
			return;

		if (who->GetTypeId() == TYPEID_PLAYER)
		{
			if (who->HasAura(SPELL_PROTECTORATE) && ((Player*)who)->GetQuestStatus(QUEST_DELIVERING_MESSAGE) == QUEST_STATUS_INCOMPLETE)
			{
				if (me->IsWithinDistInMap(who, 10.0f))
				{
					me->setFaction(FACTION_ESCORT_N_NEUTRAL_PASSIVE);
					Start(false, false, ((Player *)who)->GetGUID());
				}
			}
		}
	}

	void JustSummoned(Creature* summoned)
	{
		if (summoned->GetEntry() == NPC_NEXUS_STALKER)
			DoScriptText(SAY_NEXUS_PROTECT, summoned);
		else
		{
			if (summoned->GetEntry() == NPC_ARCHON)
				summoned->CastSpell(summoned, SPELL_ETHEREAL_TELEPORT, true);
		}

		summoned->AI()->AttackStart(me);
	}

	void WaypointReached(uint32 i)
	{
		switch (i)
		{
		case 0:
			DoScriptText(SAY_INTRO, me);
			break;
		case 3:
			DoScriptText(SAY_STAGING_GROUNDS, me);
			break;
		case 4:
			DoScriptText(SAY_TOXIC_HORROR, me);
			break;
		case 9:
			DoScriptText(SAY_SALHADAAR, me);
			break;
		case 12:
			DoScriptText(SAY_DISRUPTOR, me);
			me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_WORK_NOSHEATHE);
			SetEscortPaused(true);
			EventTimer = 5000;
			break;
		case 13:
			DoScriptText(SAY_FINISH_2, me);
			if (Player* player = GetPlayerForEscort())
			{
				me->SetFacingToObject(player);
				player->GroupEventHappens(QUEST_DELIVERING_MESSAGE, me);
			}
			SetEscortPaused(true);
			EventTimer = 6000;
			break;
		}
	}

	void UpdateAI(const uint32 diff)
	{
		npc_escortAI::UpdateAI(diff);

		if (EventTimer.Expired(diff))
		{
			switch (EventStage)
			{
			case 0:
				me->SummonCreature(NPC_ARCHON, 3875.69f, 2308.72f, 115.80f, 1.48f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
				EventTimer = 8000;
				break;
			case 1:
				me->SummonCreature(NPC_NEXUS_STALKER, 3884.06f, 2325.22f, 111.37f, 3.45f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
				me->SummonCreature(NPC_NEXUS_STALKER, 3861.54f, 2320.44f, 111.48f, 0.32f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
				EventTimer = 16000;
				break;
			case 2:
				me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
				DoScriptText(SAY_FINISH_1, me);
				SetRun();
				SetEscortPaused(false);
				EventTimer = 0;
				break;
			case 3:
				DoCast(me, SPELL_ETHEREAL_TELEPORT);
				me->ForcedDespawn(1000);
				EventTimer = 0;
				break;
			}
			++EventStage;
		}
	}
};

CreatureAI* GetAI_npc_protectorate_demolitionist(Creature* creature)
{
	return new npc_protectorate_demolitionistAI(creature);
}

/*######
## npc_saeed
######*/

#define GOSSIP_ITEM_START      16490
#define GOSSIP_ITEM_GO         16491

enum
{
	SAY_SAEED_1 = -1900216,
	SAY_SAEED_2 = -1900217,
	SAY_SAEED_3 = -1900218,
	SAY_SAEED_4 = -1900221,
	SAY_SAEED_5 = -1900222,
	SAY_DIMENSIUS_1 = -1900219,
	SAY_DIMENSIUS_2 = -1900220,

	MAX_DEFENDERS = 9,

	NPC_DIMENSIUS_ZERO = 21035,
	NPC_DIMENSIUS = 19554,
	NPC_DEFENDER = 20984,
	NPC_AVENGER = 21805,
	NPC_REGENERATOR = 21783,

	SPELL_CLEAVE = 15496
};

struct npc_saeedAI : public npc_escortAI
{
	npc_saeedAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

	bool Check;

	std::list<uint64> DifferentDefendersList;
	Timer CleaveTimer;
	Timer EventTimer;
	uint8 EventStage;

	void Reset()
	{
		if (!HasEscortState(STATE_ESCORT_ESCORTING))
		{
			EventTimer = 0;
			EventStage = 0;
		}

		Check = false;
		CleaveTimer.Reset(20000);
	}

	void DoSpawnDimensius()
	{
		if (Creature* Dim = GetClosestCreatureWithEntry(me, NPC_DIMENSIUS, 50.0f))
		{
			me->AI()->AttackStart(Dim);
		}
		else
		{
			if (Creature* Dimz = GetClosestCreatureWithEntry(me, NPC_DIMENSIUS_ZERO, 50.0f))
			{
				me->SummonCreature(NPC_DIMENSIUS, Dimz->GetPositionX(), Dimz->GetPositionY(), Dimz->GetPositionZ(), 1.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
				Dimz->SetVisibility(VISIBILITY_OFF);
			}
		}
	}

	void JustSummoned(Creature* summoned)
	{
		if (summoned->GetEntry() == NPC_DIMENSIUS)
		{
			summoned->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
			DoScriptText(SAY_DIMENSIUS_1, summoned);
			summoned->AI()->AttackStart(me);
		}
	}

	void SetFormation()
	{
		uint32 uiCount = 0;

		for (std::list<uint64>::iterator itr = DifferentDefendersList.begin(); itr != DifferentDefendersList.end(); ++itr)
		{
			if (Creature* it = me->GetCreature(*itr))
			{
				float fAngle = uiCount < MAX_DEFENDERS ? M_PI / MAX_DEFENDERS - (uiCount * 2 * M_PI / MAX_DEFENDERS) : 0.0f;
				if (it->isAlive())
					it->GetMotionMaster()->MoveFollow(me, 1.5f, fAngle);

				++uiCount;
			}
		}
	}

	void FindDefenders()
	{
		DifferentDefendersList.clear();

		std::list<Creature*> creList = FindAllCreaturesWithEntry(NPC_DEFENDER, 60.0f);
		for (std::list<Creature*>::iterator itr = creList.begin(); itr != creList.end(); ++itr)
			DifferentDefendersList.push_back((*itr)->GetGUID());

		creList = FindAllCreaturesWithEntry(NPC_AVENGER, 60.0f);
		for (std::list<Creature*>::iterator itr = creList.begin(); itr != creList.end(); ++itr)
			DifferentDefendersList.push_back((*itr)->GetGUID());

		creList = FindAllCreaturesWithEntry(NPC_REGENERATOR, 60.0f);
		for (std::list<Creature*>::iterator itr = creList.begin(); itr != creList.end(); ++itr)
			DifferentDefendersList.push_back((*itr)->GetGUID());
	}

	void JustStarted()
	{
		if (!DifferentDefendersList.empty())
			SetFormation();
	}

	void AttackStart(Unit* who)
	{
		npc_escortAI::AttackStart(who);

		for (std::list<uint64>::iterator itr = DifferentDefendersList.begin(); itr != DifferentDefendersList.end(); ++itr)
		{
			if (Creature* it = me->GetCreature(*itr))
			{
				float x, y, z;
				it->GetPosition(x, y, z);
				it->SetHomePosition(x, y, z, 0);
				it->AI()->AttackStart(who);
			}
		}
	}

	void DoEmote()
	{
		me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);

		for (std::list<uint64>::iterator itr = DifferentDefendersList.begin(); itr != DifferentDefendersList.end(); ++itr)
		{
			if (Creature* it = me->GetCreature(*itr))
				it->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
		}
	}

	void CleanEmote()
	{
		me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);

		for (std::list<uint64>::iterator itr = DifferentDefendersList.begin(); itr != DifferentDefendersList.end(); ++itr)
		{
			if (Creature* it = me->GetCreature(*itr))
				it->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_NONE);
		}
	}

	void PlayEmote()
	{
		me->HandleEmoteCommand(15);

		for (std::list<uint64>::iterator itr = DifferentDefendersList.begin(); itr != DifferentDefendersList.end(); ++itr)
		{
			if (Creature* it = me->GetCreature(*itr))
				it->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
		}
	}

	void DespawnDefenders()
	{
		for (std::list<uint64>::iterator itr = DifferentDefendersList.begin(); itr != DifferentDefendersList.end(); ++itr)
		{
			if (Creature* it = me->GetCreature(*itr))
				it->setDeathState(JUST_DIED);
		}
	}

	void WaypointReached(uint32 i)
	{
		switch (i)
		{
		case 0:
			DoScriptText(SAY_SAEED_1, me);
			FindDefenders();
			SetEscortPaused(true);
			Check = true;
			EventTimer = 2000;
			break;
		case 21:
			SetEscortPaused(true);
			DoEmote();
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
			if (Player* player = GetPlayerForEscort())
				DoScriptText(SAY_SAEED_2, me, player);
			break;
		case 22:
			me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
			break;
		case 24:
			EventTimer = 13000;
			break;
		}
	}

	void UpdateAI(const uint32 diff)
	{
		npc_escortAI::UpdateAI(diff);

		if (Check)
		{
			Player* player = GetPlayerForEscort();

			if (!me->IsWithinDistInMap(player, 105.0f))
			{
				DespawnDefenders();
				Check = false;
			}
		}

		if (EventTimer.Expired(diff))
		{
			switch (EventStage)
			{
			case 0:
				PlayEmote();
				EventTimer = 2000;
				break;
			case 1:
				JustStarted();
				SetEscortPaused(false);
				EventTimer = 0;
				break;
			case 2:
				DoScriptText(SAY_SAEED_4, me);
				EventTimer = 5000;
				break;
			case 3:
				CleanEmote();
				DoSpawnDimensius();
				EventTimer = 0;
				break;
			}

			++EventStage;
		}

		if (!UpdateVictim())
			return;

		if (CleaveTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), SPELL_CLEAVE);
			CleaveTimer = 20000;
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_saeed(Creature* creature)
{
	return new npc_saeedAI(creature);
}

bool GossipHello_npc_saeed(Player* player, Creature* creature)
{
	if (player->GetQuestStatus(10439) == QUEST_STATUS_INCOMPLETE && !((npc_saeedAI*)creature->AI())->HasEscortState(STATE_ESCORT_PAUSED))
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_START), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
	player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

	if (((npc_saeedAI*)creature->AI())->HasEscortState(STATE_ESCORT_PAUSED))
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_GO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
	player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

	return true;
}

bool GossipSelect_npc_saeed(Player* player, Creature* creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_INFO_DEF + 1)
	{
		creature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
		((npc_saeedAI*)creature->AI())->SetMaxPlayerDistance(105.0f);
		((npc_saeedAI*)creature->AI())->Start(true, true, player->GetGUID());
		player->RewardPlayerAndGroupAtEvent(20985, creature);
		player->CLOSE_GOSSIP_MENU();
	}

	if (action == GOSSIP_ACTION_INFO_DEF + 2)
	{
		DoScriptText(SAY_SAEED_3, creature);
		((npc_saeedAI*)creature->AI())->SetEscortPaused(false);
		player->CLOSE_GOSSIP_MENU();
	}

	return true;
}


/*######
## npc_dimensius
######*/

enum
{
	SPELL_PAIN5 = 37399,
	SPELL_PAIN10 = 37405,
	SPELL_PAIN15 = 37397,
	SPELL_PAIN25 = 37396,
	SPELL_PAIN35 = 37409,
	SPELL_SPIRAL = 37500,
	SPELL_VAULT = 37412,
	SPELL_FEEDING = 37450,

	NPC_SAEED = 20985,
	NPC_SPAWN = 21780
};
// this is fast one maybe need corrections.
struct npc_dimensiusAI : public ScriptedAI
{
	npc_dimensiusAI(Creature* creature) : ScriptedAI(creature), spawns(me) {}

	bool CanSpawn;
	bool DoSpawns;

	SummonList spawns;
	Timer VaultTimer;
	Timer SpiraltTimer;
	Timer StartTimer;
	Timer ShadowRainTimer;
	uint32 SpawnsCount;

	void Reset()
	{
		CanSpawn = true;
		DoSpawns = false;
		VaultTimer.Reset(15000);
		SpiraltTimer.Reset(2000);
		ShadowRainTimer.Reset(10000);
		SpawnsCount = 0;
	}

	void DoSpawn()
	{
		++SpawnsCount;

		float fangle = 0.0f;

		switch (SpawnsCount)
		{
		case 1: fangle = 0.0f; break;
		case 2: fangle = 4.6f; break;
		case 3: fangle = 1.5f; break;
		case 4: fangle = 3.1f; break;
		}

		float fx, fy, fz;
		me->GetNearPoint(fx, fy, fz, 0.0f, 20.0f, fangle);
		me->SummonCreature(NPC_SPAWN, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_DEAD_DESPAWN, 5000);
	}

	void JustSummoned(Creature* summoned)
	{
		spawns.Summon(summoned);

		summoned->CastSpell(me, SPELL_FEEDING, true);
	}

	void DamageTaken(Unit* doneby, uint32 & damage)
	{
		if (CanSpawn)
		{
			if ((me->GetHealth() * 100 - damage) / me->GetMaxHealth() < 80)
			{
				DoScriptText(SAY_DIMENSIUS_2, me);
				DoSpawns = true;
				CanSpawn = false;
			}
		}
	}

	void EnterEvadeMode()
	{
		spawns.DespawnAll();

		if (Creature* Dimz = GetClosestCreatureWithEntry(me, NPC_DIMENSIUS_ZERO, 25.0f))
			Dimz->SetVisibility(VISIBILITY_ON);

		me->ForcedDespawn();
	}

	void JustDied(Unit* killer)
	{
		std::list<Creature*> Defenders = FindAllCreaturesWithEntry(NPC_DEFENDER, 25.0f);
		std::list<Creature*> Avengers = FindAllCreaturesWithEntry(NPC_AVENGER, 25.0f);
		std::list<Creature*> Regenerators = FindAllCreaturesWithEntry(NPC_REGENERATOR, 25.0f);

		if (!Defenders.empty())
		{
			for (std::list<Creature*>::iterator it = Defenders.begin(); it != Defenders.end(); ++it)
			{
				DoCast((*it), SPELL_ETHEREAL_TELEPORT);
				(*it)->ForcedDespawn(1500);
			}
		}

		if (!Avengers.empty())
		{
			for (std::list<Creature*>::iterator it = Avengers.begin(); it != Avengers.end(); ++it)
			{
				DoCast((*it), SPELL_ETHEREAL_TELEPORT);
				(*it)->ForcedDespawn(1500);
			}
		}

		if (!Regenerators.empty())
		{
			for (std::list<Creature*>::iterator it = Regenerators.begin(); it != Regenerators.end(); ++it)
			{
				DoCast((*it), SPELL_ETHEREAL_TELEPORT);
				(*it)->ForcedDespawn(1500);
			}
		}

		if (Creature* Saeed = GetClosestCreatureWithEntry(me, NPC_SAEED, 25.0f))
		{
			DoScriptText(SAY_SAEED_5, Saeed);
			Saeed->ForcedDespawn(5000);
		}

		if (Creature* Dimz = GetClosestCreatureWithEntry(me, NPC_DIMENSIUS_ZERO, 25.0f))
			Dimz->SetVisibility(VISIBILITY_ON);

		spawns.DespawnAll();
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (DoSpawns)
		{
			if (SpawnsCount >= 4)
				DoSpawns = false;
			else DoSpawn();
		}

		if (SpiraltTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), SPELL_SPIRAL);

			SpiraltTimer = 13000;
		}

		if (VaultTimer.Expired(diff))
		{
			if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f))
				DoCast(target, SPELL_VAULT);

			VaultTimer = 20000;
		}

		if (ShadowRainTimer.Expired(diff))
		{
			if (me->HasAura(37450))
			{
				if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f))
					DoCast(target, RAND<uint32>(SPELL_PAIN35, SPELL_PAIN25, SPELL_PAIN5, SPELL_PAIN10, SPELL_PAIN15));
			}

			ShadowRainTimer = 10000;
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_dimensius(Creature* creature)
{
	return new npc_dimensiusAI(creature);
}

/*######
## npc_king_salhadaar
######*/

enum
{
	FACTION_HOSTILE = 90,
	FACTION_FRIENDLY = 35,

	SPELL_TESLA = 35515,
	SPELL_OVERSPARK = 35684,
	SPELL_FLUX = 36533,
	SPELL_STASIS = 36527,
	SPELL_IMAGE = 36848,
	SPELL_IMAGEI = 36847,
	SPELL_STATISI = 35514,

	NPC_BALL = 20769,
	NPC_IMAGE = 21425
};
//don't support kite. i didn't like what happened :P
struct npc_king_salhadaarAI : public ScriptedAI
{
	npc_king_salhadaarAI(Creature* creature) : ScriptedAI(creature), summons(me) {}

	bool Spawn;

	SummonList summons;
	std::list<uint64> Balls;
	uint32 Count;
	Timer_UnCheked FluxTimer;
	Timer_UnCheked StasisTimer;

	void Reset()
	{

		me->setFaction(FACTION_FRIENDLY);
		Count = 0;
		FluxTimer.Reset(6000);
		StasisTimer.Reset(22000);
		Spawn = true;

		Map* tmpMap = me->GetMap();

		if (!tmpMap)
			return;

		if (!Balls.empty())
		{
			Creature* ball = NULL;
			for (std::list<uint64>::iterator itr = Balls.begin(); itr != Balls.end(); ++itr)
			{
				if (ball = tmpMap->GetCreature((*itr)))
					me->CastSpell(ball, SPELL_STATISI, false);
			}
		}
	}

	void NoEnergy()
	{
		++Count;

		if (Count == 3)
			PartyTime();
	}

	void PartyTime()
	{
		//DoScriptText(YELL_INTRO, me);
		me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
		me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		me->setFaction(FACTION_HOSTILE);
		me->RemoveAllAuras();
		DoCast(me, SPELL_OVERSPARK);
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == SPELL_TESLA)
			Balls.push_back(caster->GetGUID());

		return;
	}

	void DamageTaken(Unit* doneby, uint32 & damage)
	{
		if (Spawn)
		{
			if ((me->GetHealth() * 100 - damage) / me->GetMaxHealth() < 60)
			{
				me->InterruptNonMeleeSpells(true);
				DoCast(me, SPELL_IMAGE);
				DoCast(me, SPELL_IMAGEI);
				Spawn = false;
			}
		}
	}

	void JustSummoned(Creature* summoned)
	{
		summons.Summon(summoned);

		summoned->CastSpell(summoned, SPELL_OVERSPARK, false);

		if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f))
			summoned->AI()->AttackStart(target);
	}

	void JustReachedHome()
	{
		summons.DespawnAll();
		Reset();
	}

	void JustDied(Unit* killer)
	{
		summons.DespawnAll();
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (FluxTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), SPELL_FLUX);

			FluxTimer = 12000;
		}

		if (StasisTimer.Expired(diff))
		{
			if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0, 40.0f))
				DoCast(target, SPELL_STASIS);

			StasisTimer = 22000;
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_king_salhadaar(Creature* creature)
{
	return new npc_king_salhadaarAI(creature);
}

/*######
## npc_energy_ball
######*/

enum
{
	SPELL_DISRUPTOR = 35683,

	NPC_KING = 20454,
};

struct npc_energy_ballAI : public ScriptedAI
{
	npc_energy_ballAI(Creature* creature) : ScriptedAI(creature) {}

	void Reset()
	{
		DoCast(me, SPELL_TESLA);
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == SPELL_DISRUPTOR)
		{
			me->InterruptNonMeleeSpells(true);

			if (Creature* king = GetClosestCreatureWithEntry(me, NPC_KING, 75.0f))
				CAST_AI(npc_king_salhadaarAI, king->AI())->NoEnergy();
		}

		if (spell->Id == SPELL_STATISI)
			DoCast(me, SPELL_TESLA);

		return;
	}
};

CreatureAI* GetAI_npc_energy_ball(Creature* creature)
{
	return new npc_energy_ballAI(creature);
}

/*######
# npc_trader_marid
######*/

#define GOSSIP_ITEM_GO1         16492

enum
{
	FACTION_HOSTILE_ = 90,
	MY_FACTION = 1731,

	SAY_START_1 = -1900223,
	SAY_GUARD = -1900224,

	QUEST_TROUBLE = 10273,
	NPC_BODYGUARD = 20101,
};

struct npc_trader_maridAI : public npc_escortAI
{
	npc_trader_maridAI(Creature* creature) : npc_escortAI(creature) {}

	void Reset()
	{
		me->setFaction(MY_FACTION);
	}

	void EnterCombat(Unit* who)
	{
		float fx, fy, fz;
		me->GetNearPoint(fx, fy, fz, 0.0f, 4.0f, 0.0f);
		me->SummonCreature(NPC_BODYGUARD, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
		me->GetNearPoint(fx, fy, fz, 0.0f, 4.0f, M_PI);
		me->SummonCreature(NPC_BODYGUARD, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
	}

	void JustSummoned(Creature* summoned)
	{
		summoned->setFaction(FACTION_HOSTILE_);
		summoned->AI()->AttackStart(me->GetVictim());

		if (Player* player = GetPlayerForEscort())
			DoScriptText(SAY_GUARD, summoned, player);
	}

	void WaypointReached(uint32 i)
	{
		switch (i)
		{
		case 6:
			me->setFaction(FACTION_HOSTILE_);
			if (Player* player = GetPlayerForEscort())
				me->AI()->AttackStart(player);
			break;
		}
	}

	void UpdateAI(const uint32 diff)
	{
		npc_escortAI::UpdateAI(diff);

		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_trader_marid(Creature* creature)
{
	return new npc_trader_maridAI(creature);
}

bool GossipHello_npc_trader_marid(Player* player, Creature* creature)
{
	if (player->GetQuestStatus(QUEST_TROUBLE) == QUEST_STATUS_INCOMPLETE)
	{
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_GO1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
	}
	else
	{
		if (creature->isQuestGiver())
			player->PrepareQuestMenu(creature->GetGUID());
		player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
	}
	return true;
}

bool GossipSelect_npc_trader_marid(Player* player, Creature* creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_INFO_DEF + 1)
	{
		DoScriptText(SAY_START_1, creature);
		((npc_trader_maridAI*)creature->AI())->Start(false, false, player->GetGUID(), 0, true);
		player->CLOSE_GOSSIP_MENU();
	}

	return true;
}

/*######
## npc_doctor_vomisa
######*/

enum
{
	QUEST_YOU_ROBOT = 10248,

	NPC_X6000 = 19849,
	NPC_NEGATRON = 19851
};

struct npc_doctor_vomisaAI : public ScriptedAI
{
	npc_doctor_vomisaAI(Creature* creature) : ScriptedAI(creature) {}

	ObjectGuid X6000GUID;
	ObjectGuid NegatronGUID;
	Timer_UnCheked CheckTimer;

	void Reset()
	{
		X6000GUID = 0;
		NegatronGUID = 0;
		CheckTimer = 0;
	}

	void RockAndRool()
	{
		if (NegatronGUID == 0)
		{
			CheckTimer = 5000;

			float fx, fy, fz;
			me->GetNearPoint(fx, fy, fz, 0.0f, 5.0f, 4.7f);
			me->SummonCreature(NPC_X6000, fx, fy, fz, me->GetOrientationTo(fx, fy), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 61000);
			me->GetNearPoint(fx, fy, fz, 0.0f, 56.0f, 4.7f);
			me->SummonCreature(NPC_NEGATRON, fx, fy, fz, 2.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 50000);
		}
	}

	void JustSummoned(Creature* summoned)
	{
		if (summoned->GetEntry() == NPC_X6000)
			X6000GUID = summoned->GetGUID();
		else
		{
			if (summoned->GetEntry() == NPC_NEGATRON)
			{
				NegatronGUID = summoned->GetGUID();
				//DoScriptText(YELL_INTRO, summoned);
			}
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (CheckTimer.Expired(diff))
		{
			if (Creature* negatron = GetClosestCreatureWithEntry(me, NPC_NEGATRON, 100.0f, true))
				return;
			else
			{
				Map* tmpMap = me->GetMap();

				if (!tmpMap)
					return;

				if (Creature* reaver = tmpMap->GetCreature(X6000GUID))
					reaver->ForcedDespawn(5000);

				Reset();
			}

			CheckTimer = 5000;
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_doctor_vomisa(Creature* creature)
{
	return new npc_doctor_vomisaAI(creature);
}

bool GossipHello_npc_doctor_vomisa(Player* player, Creature* creature)
{
	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());
	player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

	return true;
}

bool QuestAccept_npc_doctor_vomisa(Player* player, Creature* creature, const Quest* quest)
{
	if (quest->GetQuestId() == QUEST_YOU_ROBOT)
		CAST_AI(npc_doctor_vomisaAI, creature->AI())->RockAndRool();

	return true;
}

/*######
## ScriptName: mob_warp_aberration
## Name: Warp Aberration
## Entry: 18865
######*/
// SQL: DELETE FROM `creature_template_addon` WHERE `entry`='18865';
//      UPDATE `creature_template` SET `ScriptName`='mob_warp_aberration' WHERE `entry`='18865';

// Spells
#define SPELL_EPEXTRACTOR       34520
#define SPELL_CREATE_EPOWER     34525
#define SPELL_WARP_STORM        36577
#define SPELL_ARCANE_SHIELD     36640

struct mob_warp_aberrationAI : public ScriptedAI
{

	mob_warp_aberrationAI(Creature* creature) : ScriptedAI(creature) {}

	uint32 WarpStormTimer;
	bool PowerExtracted;

	void Reset()
	{
		PowerExtracted = false;
		WarpStormTimer = 5000; // Timer_UnCheked may be incorrect...
		me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
		me->ApplySpellImmune(1, IMMUNITY_STATE, SPELL_AURA_PERIODIC_MANA_LEECH, true);
		me->ApplySpellImmune(2, IMMUNITY_STATE, SPELL_AURA_POWER_BURN_MANA, true);
	}

	void EnterCombat(Unit* who)
	{
		me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, false);
		me->ApplySpellImmune(1, IMMUNITY_STATE, SPELL_AURA_PERIODIC_MANA_LEECH, false);
		me->ApplySpellImmune(2, IMMUNITY_STATE, SPELL_AURA_POWER_BURN_MANA, false);
		me->CastSpell(me, SPELL_ARCANE_SHIELD, true);
		me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_ARCANE, true);
		me->ApplySpellImmune(1, IMMUNITY_STATE, SPELL_AURA_PERIODIC_MANA_LEECH, true);
		me->ApplySpellImmune(2, IMMUNITY_STATE, SPELL_AURA_POWER_BURN_MANA, true);
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == SPELL_EPEXTRACTOR)
			PowerExtracted = true;
	}

	void JustDied(Unit* killer)
	{
		if (PowerExtracted)
			me->CastSpell(me, SPELL_CREATE_EPOWER, true);
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;
		if (WarpStormTimer <= diff)
		{
			me->CastSpell(me, SPELL_WARP_STORM, false);
			WarpStormTimer = 25000;
		}
		else
			WarpStormTimer -= diff;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_warp_aberration(Creature* creature)
{
	return new mob_warp_aberrationAI(creature);
}

/*######
## npc_ethereum_jailor_caller
######*/

struct npc_ethereum_jailor_callerAI : public ScriptedAI
{
	npc_ethereum_jailor_callerAI(Creature* creature) : ScriptedAI(creature) {}

	void Reset() { }

	void JustRespawned()
	{
		uint32 ChanceToSpawn = urand(0, 100);
		if (ChanceToSpawn <= 5)
			me->UpdateEntry(23008);
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_ethereum_jailor_caller(Creature* creature)
{
	return new npc_ethereum_jailor_callerAI(creature);
}

bool GossipHello_npc_19217(Player* player, Creature* creature)
{
	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());

	if (!player->HasItemCount(28455, 1) && (player->GetQuestStatus(10174) == QUEST_STATUS_AVAILABLE || player->GetQuestStatus(10188) == QUEST_STATUS_AVAILABLE ||
		player->GetQuestStatus(10192) == QUEST_STATUS_AVAILABLE || player->GetQuestStatus(10301) == QUEST_STATUS_AVAILABLE ||
		player->GetQuestStatus(10209) == QUEST_STATUS_AVAILABLE || player->GetQuestStatus(10176) == QUEST_STATUS_AVAILABLE))
		player->ADD_GOSSIP_ITEM(0, "Give me another Archmage Vargoth's Staff, please.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
	player->SEND_GOSSIP_MENU(9826, creature->GetGUID());

	return true;
}

bool GossipSelect_npc_19217(Player* player, Creature* creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_INFO_DEF + 1)
	{
		ItemPosCountVec dest;
		uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 28455, 1);
		if (msg == EQUIP_ERR_OK)
		{
			Item* item = player->StoreNewItem(dest, 28455, true);
			player->SendNewItem(item, 1, true, false);
		}
		player->CLOSE_GOSSIP_MENU();
	}

	return true;
}

struct npc_19723_19724AI : public ScriptedAI
{
	npc_19723_19724AI(Creature* creature) : ScriptedAI(creature) {}

	void Reset() { }

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (spell->Id == 34526)
		{
			me->SummonGameObject(181915, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, 0, 0, 0, 0, 60);
			me->DisappearAndDie();
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19723_19724(Creature* creature)
{
	return new npc_19723_19724AI(creature);
}

struct npc_19494AI : public ScriptedAI
{
	npc_19494AI(Creature* creature) : ScriptedAI(creature) {}

	Timer ArcaneExplosion;

	void Reset()
	{
		ArcaneExplosion.Reset(1000);
	}

	void DamageTaken(Unit* done_by, uint32 &damage)
	{
		damage = damage * 2;
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (ArcaneExplosion.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 11975, false);
			ArcaneExplosion = urand(5000, 9000);
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19494(Creature* creature)
{
	return new npc_19494AI(creature);
}

struct npc_19481AI : public ScriptedAI
{
	npc_19481AI(Creature* creature) : ScriptedAI(creature) {}

	Timer EventTimer;
	uint32 Phase;

	void Reset()
	{
		EventTimer.Reset(0);
		Phase = 0;
		me->SetFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
	}

	void StartEvent(Player* pPlayer)
	{
		me->RemoveFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
		me->Say(-1200578, LANG_UNIVERSAL, pPlayer->GetGUID());
		EventTimer = 7000;
		Phase = 0;
	}

	void JustSummoned(Creature* summoned)
	{
		summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
	}

	void UpdateAI(const uint32 diff)
	{
		if (EventTimer.Expired(diff))
		{
			switch (Phase)
			{
			case 0:
			{
				me->HandleEmote(22);
				me->SummonCreature(19916, 2236.6, 2224.6, 136.33, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
				me->SummonCreature(19916, 2240.3, 2234.3, 136.11, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
				me->SummonCreature(19916, 2248.5, 2240, 136.5, 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
				Phase = 1;
				EventTimer = 3000;
				break;
			}
			case 1:
			{
				me->CastSpell(me, 34679, false);
				EventTimer = 7000;
				Phase = 2;
				break;
			}
			case 2:
			{
				std::list<Creature*> crystalls = FindAllCreaturesWithEntry(19916, 20);
				for (std::list<Creature *>::iterator i = crystalls.begin(); i != crystalls.end(); i++)
					(*i)->CastSpell((*i), 17277, false);
				me->CastSpell(me, 22592, true);
				EventTimer = 3000;
				Phase = 3;
				break;
			}
			case 3:
			{
				std::list<Creature*> crys = FindAllCreaturesWithEntry(19916, 20);
				for (std::list<Creature *>::iterator i = crys.begin(); i != crys.end(); i++)
					(*i)->DisappearAndDie();
				me->HandleEmote(64);
				EventTimer = 4000;
				Phase = 4;
				break;
			}
			case 4:
			{
				me->Say(-1200579, LANG_UNIVERSAL, 0);
				EventTimer = 3000;
				Phase = 5;
				break;
			}
			case 5:
			{
				me->HandleEmote(30);
				me->Say(-1200580, LANG_UNIVERSAL, 0);
				EventTimer = 0;
				Phase = 0;
				Reset();
				break;
			}
			default: break;
			}
		}

		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19481(Creature* creature)
{
	return new npc_19481AI(creature);
}

bool QuestComplete_npc_19481(Player* player, Creature* creature, const Quest* quest)
{
	if (quest->GetQuestId() == 10176)
	{
		if (npc_19481AI* varg = dynamic_cast<npc_19481AI*>(creature->AI()))
			varg->StartEvent(player);
	}
	return true;
}


struct npc_19546AI : public ScriptedAI
{
	npc_19546AI(Creature* creature) : ScriptedAI(creature) {}

	Timer ManaShieldTimer;
	Timer ArcaneMissilesTimer;

	void Reset()
	{
		ManaShieldTimer.Reset(2000);
		ArcaneMissilesTimer.Reset(3000);
	}

	void EnterCombat(Unit* who)
	{
		me->CastSpell(me, 12544, false);
	}

	void MoveInLineOfSight(Unit* who)
	{
		if (who->GetTypeId() == TYPEID_PLAYER)
		{
			if (((Player*)who)->GetQuestStatus(10305) == QUEST_STATUS_COMPLETE && ((Player*)who)->GetQuestRewardStatus(10305))
			{
				((Player*)who)->GetReputationMgr().ApplyForceReaction(1007, ReputationRank(REP_NEUTRAL), true);
				((Player*)who)->GetReputationMgr().SendForceReactions();
			}
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (ManaShieldTimer.Expired(diff))
		{
			me->CastSpell(me, 17740, false);
			ManaShieldTimer = urand(12000, 25000);
		}

		if (ArcaneMissilesTimer.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 34447, false);
			ArcaneMissilesTimer = 8000;
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19546(Creature* creature)
{
	return new npc_19546AI(creature);
}

struct npc_19544AI : public ScriptedAI
{
	npc_19544AI(Creature* creature) : ScriptedAI(creature) {}

	Timer LightningBoltTimer;
	Timer DancingSwordsTimer;
	Timer BlinkTimer;

	void Reset()
	{
		LightningBoltTimer.Reset(2000);
		DancingSwordsTimer.Reset(urand(1000, 5000));
		BlinkTimer.Reset(urand(1000, 10000));
	}

	void MoveInLineOfSight(Unit* who)
	{
		if (who->GetTypeId() == TYPEID_PLAYER)
		{
			if (((Player*)who)->GetQuestStatus(10306) == QUEST_STATUS_COMPLETE && ((Player*)who)->GetQuestRewardStatus(10306))
			{
				((Player*)who)->GetReputationMgr().ApplyForceReaction(1008, ReputationRank(REP_NEUTRAL), true);
				((Player*)who)->GetReputationMgr().SendForceReactions();
			}
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (LightningBoltTimer.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 9532, false);
			LightningBoltTimer = urand(1000, 3500);
		}

		if (DancingSwordsTimer.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 36110, false);
			DancingSwordsTimer = 15000;
		}

		if (BlinkTimer.Expired(diff))
		{
			me->CastSpell(me, 36109, false);
			BlinkTimer = urand(5000, 15000);
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19544(Creature* creature)
{
	return new npc_19544AI(creature);
}


struct npc_19545AI : public ScriptedAI
{
	npc_19545AI(Creature* creature) : ScriptedAI(creature) {}

	Timer IceBarrierTimer;
	Timer FrostNovaTimer;
	Timer FrostBoltTimer;

	void Reset()
	{
		IceBarrierTimer.Reset(1000);
		FrostNovaTimer.Reset(urand(1000, 5000));
		FrostBoltTimer.Reset(2500);
	}

	void MoveInLineOfSight(Unit* who)
	{
		if (who->GetTypeId() == TYPEID_PLAYER)
		{
			if (((Player*)who)->GetQuestStatus(10307) == QUEST_STATUS_COMPLETE && ((Player*)who)->GetQuestRewardStatus(10307))
			{
				((Player*)who)->GetReputationMgr().ApplyForceReaction(1009, ReputationRank(REP_NEUTRAL), true);
				((Player*)who)->GetReputationMgr().SendForceReactions();
			}
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (IceBarrierTimer.Expired(diff))
		{
			me->CastSpell(me, 33245, false);
			IceBarrierTimer = urand(25000, 30000);
		}

		if (FrostNovaTimer.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 11831, false);
			FrostNovaTimer = 7500;
		}

		if (FrostBoltTimer.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 9672, false);
			FrostBoltTimer = urand(2500, 3000);
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19545(Creature* creature)
{
	return new npc_19545AI(creature);
}


struct npc_19543AI : public ScriptedAI
{
	npc_19543AI(Creature* creature) : ScriptedAI(creature) {}

	Timer TorrentOfFlames;
	Timer Pyroblast;

	void Reset()
	{
		TorrentOfFlames.Reset(1000);
		Pyroblast.Reset(3000);
	}

	void MoveInLineOfSight(Unit* who)
	{
		if (who->GetTypeId() == TYPEID_PLAYER)
		{
			if (((Player*)who)->GetQuestStatus(10182) == QUEST_STATUS_COMPLETE && ((Player*)who)->GetQuestRewardStatus(10182))
			{
				((Player*)who)->GetReputationMgr().ApplyForceReaction(1006, ReputationRank(REP_NEUTRAL), true);
				((Player*)who)->GetReputationMgr().SendForceReactions();
			}
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (TorrentOfFlames.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 36104, false);
			TorrentOfFlames = 8000;
		}

		if (Pyroblast.Expired(diff))
		{
			me->CastSpell(me->GetVictim(), 17273, false);
			Pyroblast = 12000;
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19543(Creature* creature)
{
	return new npc_19543AI(creature);
}

/*######
## mob_sunfury_astromancer
######*/

// Spells
#define SPELL_SCORCH            38391
#define SPELL_ASTRAL_FOCUS      35914
#define SPELL_FIERY_INTELLECT   35917

struct mob_sunfury_astromancerAI : public ScriptedAI
{
	mob_sunfury_astromancerAI(Creature* creature) : ScriptedAI(creature) {}

	bool BuffStatus;
	Timer ScorchTimer;
	Timer AstralFocusTimer;
	Timer FieryIntellectTimer;

	void Reset()
	{
		BuffStatus = false;
		ScorchTimer.Reset(4000);
		AstralFocusTimer.Reset(6000);
		FieryIntellectTimer.Reset(600000);

		//Adding mana from buff
		me->SetMaxPower(POWER_MANA, (me->GetCreatureInfo()->maxmana)*1.1);
		me->SetPower(POWER_MANA, (me->GetCreatureInfo()->maxmana)*1.1);
	}

	void OnAuraApply(Aura* aur, Unit* caster, bool stackApply) {
		if (aur->GetId() == SPELL_FIERY_INTELLECT)
		{
			//Adding mana
			me->SetMaxPower(POWER_MANA, (me->GetCreatureInfo()->maxmana)*1.1);
			me->SetPower(POWER_MANA, (me->GetCreatureInfo()->maxmana)*1.1);

			//For add damage to SPELL_SCORCH
			BuffStatus = true;
		}
	}

	void OnAuraRemove(Aura* aur, bool stackRemove) {
		if (aur->GetId() == SPELL_FIERY_INTELLECT) {
			me->SetMaxPower(POWER_MANA, me->GetCreatureInfo()->maxmana);
			me->SetPower(POWER_MANA, me->GetCreatureInfo()->maxmana);
			BuffStatus = false;
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (FieryIntellectTimer.Expired(diff) && !BuffStatus) {
			DoCast(me, SPELL_FIERY_INTELLECT);
			FieryIntellectTimer = 600000;
			BuffStatus = true;
		}

		if (!UpdateVictim())
			return;

		if (ScorchTimer.Expired(diff)) {
			if (BuffStatus) {
				int32 damage = (51 + rand() % 19) + 15;
				int32 manacost = 50;
				me->CastCustomSpell(me->GetVictim(), SPELL_SCORCH, &damage, &manacost, NULL, true);
			}
			else
				DoCast(me->GetVictim(), SPELL_SCORCH);

			ScorchTimer = 4000;
		}

		if (AstralFocusTimer.Expired(diff)) {
			DoCast(me->GetVictim(), SPELL_ASTRAL_FOCUS);
			AstralFocusTimer = 6000;
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_sunfury_astromancer(Creature* creature)
{
	return new mob_sunfury_astromancerAI(creature);
}


struct mob_19489AI : public ScriptedAI
{
	mob_19489AI(Creature* creature) : ScriptedAI(creature) {}

	void Reset()
	{
	}

	void SummonedMovementInform(Creature* pSummoned, uint32 MotionType, uint32 Point)
	{
		if (pSummoned->GetEntry() != 20618 || MotionType != POINT_MOTION_TYPE || !Point)
			return;

		if (Point == 1)
		{
			pSummoned->DisappearAndDie();
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_mob_19489(Creature* creature)
{
	return new mob_19489AI(creature);
}

Position InvadersPos[5] =
{
	{2321.13f, 2306.35f, 101.98f, 3.6f},
	{2340.72f, 2275.06f, 108.12f, 2.9f},
	{2284.57f, 2324.49f, 106.89f, 4.8f},
	{2302.53f, 2301.87f, 98.41f, 4.23f},
	{2335.56f, 2314.02f, 105.43f, 3.77f},
};

bool QuestComplete_mob_19489(Player* player, Creature* creature, const Quest* quest)
{
	if (quest->GetQuestId() == 10240)
	{
		creature->SummonGameObject(183955, 2259.09, 2264.64, 100.557, 0.89, 0, 0, 0, 0, 30);
		for (int i = 0; i < 5; i++)
		{
			if (Creature* invader = creature->SummonCreature(20618, InvadersPos[i].x, InvadersPos[i].y, InvadersPos[i].z, InvadersPos[i].o, TEMPSUMMON_MANUAL_DESPAWN, 0))
			{
				invader->SetSpeed(MOVE_WALK, 0.5);
				invader->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
				invader->GetMotionMaster()->MovePoint(1, 2290.45, 2283.704, 95.389);
			}
		}
	}
	return true;
}



struct npc_41000AI : public ScriptedAI
{
	npc_41000AI(Creature* creature) : ScriptedAI(creature), Summons(me) {}

	SummonList Summons;
	Timer SummonTimer;

	void Reset()
	{
		SummonTimer.Reset(1000);
	}

	void JustSummoned(Creature* pSummon)
	{
		Summons.Summon(pSummon);
	}

	void SummonedMovementInform(Creature* pSummoned, uint32 type, uint32 id)
	{
		if (type == POINT_MOTION_TYPE)
		{
			if (id == 100)
				pSummoned->Kill(pSummoned);
		}
	}

	void SummonedCreatureDies(Creature* unit, Unit* killer)
	{
		switch (me->GetDBTableGUIDLow())
		{
		case 2381566: // manaforge duro
		{
			me->SummonCreature(19731, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		case 2381560:
		{
			switch (urand(0, 6))
			{
			case 0:
				me->SummonCreature(19731, 3020.3, 2405, 133.9, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 1:
				me->SummonCreature(19731, 3006, 2414, 132.3, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 2:
				me->SummonCreature(19731, 3001, 2413, 132.3, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 3:
				me->SummonCreature(19731, 3010.75, 2410, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 4:
				me->SummonCreature(19731, 3007.75, 2408.3, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 5:
				me->SummonCreature(19731, 3015.4, 2409, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 6:
				me->SummonCreature(19731, 3012.6, 2406.76, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			}
			break;
		}
		case 2381561:
		{
			switch (urand(0, 3))
			{
			case 0:
				me->SummonCreature(19731, 2892.53, 1970.54, 122.29, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 1:
				me->SummonCreature(19731, 2900.27, 1963.42, 124.23, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 2:
				me->SummonCreature(19731, 2904, 1956.39, 122.24, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 3:
				me->SummonCreature(19731, 2905.57, 1949.67, 123.4, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			}
			break;
		}
		case 2381562:
		{
			switch (urand(0, 2))
			{
			case 0:
				me->SummonCreature(19731, 2509.43, 2283.52, 118.85, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 1:
				me->SummonCreature(19731, 2504.89, 2283.65, 118.05, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 2:
				me->SummonCreature(19731, 2491.07, 2289.03, 120.07, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			}
			break;
		}
		case 2381563:
		{
			switch (urand(0, 5))
			{
			case 0:
				me->SummonCreature(19731, 2569.4, 2260.17, 102.98, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 1:
				me->SummonCreature(19731, 2601.48, 2250.65, 96.07, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 2:
				me->SummonCreature(19731, 2605.45, 2263.48, 90.53, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 3:
				me->SummonCreature(19731, 2611.23, 2273.77, 93.84, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 4:
				me->SummonCreature(19731, 2619.24, 2282.95, 94.7, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 5:
				me->SummonCreature(19731, 2624.5, 2290.93, 95.97, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			}
			break;
		}
		case 2381564:
		{
			switch (urand(0, 5))
			{
			case 0:
				me->SummonCreature(19731, 2634.47, 2315.95, 98.16, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 1:
				me->SummonCreature(19731, 2638.9, 2323.5, 98.46, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 2:
				me->SummonCreature(19731, 2645.12, 2331.46, 96.1, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 3:
				me->SummonCreature(19731, 2646.49, 2349.24, 91.94, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 4:
				me->SummonCreature(19731, 2649.34, 2364.03, 90.71, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 5:
				me->SummonCreature(19731, 266.24, 2354.88, 79.29, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			}
			break;
		}
		case 2381565:
		{
			switch (urand(0, 7))
			{
			case 0:
				me->SummonCreature(19731, 2637.19, 2420.04, 96.28, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 1:
				me->SummonCreature(19731, 2627.27, 2441.33, 102.38, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 2:
				me->SummonCreature(19731, 2603.71, 2453.3, 105.92, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 3:
				me->SummonCreature(19731, 2591.6, 2469.49, 109.52, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 4:
				me->SummonCreature(19731, 2577.38, 2450.79, 103.13, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 5:
				me->SummonCreature(19731, 2558.8, 2473.53, 114.03, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 6:
				me->SummonCreature(19731, 2552.05, 2486.54, 118.58, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			case 7:
				me->SummonCreature(19731, 2530, 2472.66, 121.09, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
				break;
			}
		}
		break;
		}
	}

	void DoSummoning()
	{
		Summons.DespawnAll();
		switch (me->GetDBTableGUIDLow())
		{
		case 2381560: // manaforge duro
		{
			me->SummonCreature(19731, 3006, 2414, 132.3, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 3001, 2413, 132.3, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 3010.75, 2410, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 3007.75, 2408.3, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 3015.4, 2409, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 3012.6, 2406.76, 133.5, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 3020.3, 2405, 133.9, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		case 2381561: // manaforge duro
		{
			me->SummonCreature(19731, 2892.53, 1970.54, 122.29, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2900.27, 1963.42, 124.23, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2904, 1956.39, 122.24, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2905.57, 1949.67, 123.4, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		case 2381566: // manaforge duro
		{
			me->SummonCreature(19731, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		case 2381562: // sunfury hold
		{
			me->SummonCreature(19731, 2509.43, 2283.52, 118.85, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2504.89, 2283.65, 118.05, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2491.07, 2289.03, 120.07, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		case 2381563:
		{
			me->SummonCreature(19731, 2569.4, 2260.17, 102.98, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2601.48, 2250.65, 96.07, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2605.45, 2263.48, 90.53, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2611.23, 2273.77, 93.84, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2619.24, 2282.95, 94.7, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2624.5, 2290.93, 95.97, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		case 2381564:
		{
			me->SummonCreature(19731, 2634.47, 2315.95, 98.16, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2638.9, 2323.5, 98.46, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2645.12, 2331.46, 96.1, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2646.49, 2349.24, 91.94, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2649.34, 2364.03, 90.71, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 266.24, 2354.88, 79.29, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		case 2381565:
		{
			me->SummonCreature(19731, 2637.19, 2420.04, 96.28, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2627.27, 2441.33, 102.38, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2603.71, 2453.3, 105.92, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2591.6, 2469.49, 109.52, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2577.38, 2450.79, 103.13, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2558.8, 2473.53, 114.03, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2552.05, 2486.54, 118.58, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			me->SummonCreature(19731, 2530, 2472.66, 121.09, 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
			break;
		}
		default: break;
		}
	}
	void UpdateAI(const uint32 diff)
	{
		if (SummonTimer.Expired(diff))
		{
			DoSummoning();
			SummonTimer = 120000;
		}

		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_41000(Creature* creature)
{
	return new npc_41000AI(creature);
}


struct npc_19731AI : public ScriptedAI
{
	npc_19731AI(Creature* creature) : ScriptedAI(creature) {}

	Timer MoveTimer;

	void Reset()
	{
		MoveTimer.Reset(1000);
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if (!me->isDead() && spell->Id == 33796)
			me->Kill(me);
	}

	void UpdateAI(const uint32 diff)
	{
		if (MoveTimer.Expired(diff))
		{
			if (Creature* target = GetClosestCreatureWithEntry(me, 19939, 130, true))
			{
				me->GetMotionMaster()->MovePoint(100, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
				me->SetHomePosition(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0);
			}
			MoveTimer = 0;
		}

		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19731(Creature* creature)
{
	return new npc_19731AI(creature);
}


struct npc_19568AI : public ScriptedAI
{
	npc_19568AI(Creature* creature) : ScriptedAI(creature) {}

	Timer ResetTimer;

	void Reset()
	{
		ResetTimer.Reset(1000);
	}

	void MovementInform(uint32 type, uint32 id)
	{
		if (id == 1)
		{
			me->GetMotionMaster()->Clear();
			me->setDeathState(JUST_DIED);
			me->Respawn();
		}
	}

	void UpdateAI(const uint32 diff)
	{
		if (ResetTimer.Expired(diff))
		{
			me->setActive(true);
			me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
			me->GetMotionMaster()->Initialize();
			me->SetReactState(REACT_PASSIVE);
			ResetTimer = 0;
		}

		if (!UpdateVictim())
			return;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_19568(Creature* creature)
{
	return new npc_19568AI(creature);
}

/*######
## AddSC_netherstrom
######*/

void AddSC_netherstorm()
{
	Script *newscript;

	newscript = new Script;
	newscript->Name = "go_manaforge_control_console";
	newscript->pGOUse = &GOUse_go_manaforge_control_console;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19568";
	newscript->GetAI = &GetAI_npc_19568;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19731";
	newscript->GetAI = &GetAI_npc_19731;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_41000";
	newscript->GetAI = &GetAI_npc_41000;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_manaforge_control_console";
	newscript->GetAI = &GetAI_npc_manaforge_control_console;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_commander_dawnforge";
	newscript->GetAI = &GetAI_npc_commander_dawnforge;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "at_commander_dawnforge";
	newscript->pAreaTrigger = &AreaTrigger_at_commander_dawnforge;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_protectorate_nether_drake";
	newscript->pGossipHello = &GossipHello_npc_protectorate_nether_drake;
	newscript->pGossipSelect = &GossipSelect_npc_protectorate_nether_drake;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_professor_dabiri";
	newscript->pGossipHello = &GossipHello_npc_professor_dabiri;
	newscript->pGossipSelect = &GossipSelect_npc_professor_dabiri;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_professor_dabiri;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_veronia";
	newscript->pGossipHello = &GossipHello_npc_veronia;
	newscript->pGossipSelect = &GossipSelect_npc_veronia;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_phase_hunter";
	newscript->GetAI = &GetAI_mob_phase_hunter;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_bessy";
	newscript->GetAI = &GetAI_npc_bessy;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_bessy;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_talbuk";
	newscript->GetAI = &GetAI_mob_talbuk;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_withered_corpse";
	newscript->GetAI = &GetAI_npc_withered_corpse;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "go_ethereum_prison";
	newscript->pGOUse = &GOUse_go_ethereum_prison;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_warp_chaser";
	newscript->GetAI = &GetAI_npc_warp_chaser;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_epextraction";
	newscript->GetAI = &GetAI_mob_epextraction;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_dr_boom";
	newscript->GetAI = &GetAI_mob_dr_boom;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_boom_bot";
	newscript->GetAI = &GetAI_mob_boom_bot;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_maxx_a_million";
	newscript->GetAI = &GetAI_npc_maxx_a_million;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_maxx_a_million;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_scrapped_reaver";
	newscript->GetAI = &GetAI_npc_scrapped_reaver;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_drijya";
	newscript->GetAI = &GetAI_npc_drijya;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_drijya;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_captured_vanguard";
	newscript->GetAI = &GetAI_npc_captured_vanguard;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_captured_vanguard;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_controller";
	newscript->GetAI = &GetAI_npc_controller;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_protectorate_demolitionist";
	newscript->GetAI = &GetAI_npc_protectorate_demolitionist;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_saeed";
	newscript->pGossipHello = &GossipHello_npc_saeed;
	newscript->pGossipSelect = &GossipSelect_npc_saeed;
	newscript->GetAI = &GetAI_npc_saeed;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_dimensius";
	newscript->GetAI = &GetAI_npc_dimensius;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_king_salhadaar";
	newscript->GetAI = &GetAI_npc_king_salhadaar;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_energy_ball";
	newscript->GetAI = &GetAI_npc_energy_ball;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_trader_marid";
	newscript->GetAI = &GetAI_npc_trader_marid;
	newscript->pGossipHello = &GossipHello_npc_trader_marid;
	newscript->pGossipSelect = &GossipSelect_npc_trader_marid;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_doctor_vomisa";
	newscript->GetAI = &GetAI_npc_doctor_vomisa;
	newscript->pGossipHello = &GossipHello_npc_doctor_vomisa;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_doctor_vomisa;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_warp_aberration";
	newscript->GetAI = &GetAI_mob_warp_aberration;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_ethereum_jailor_caller";
	newscript->GetAI = &GetAI_npc_ethereum_jailor_caller;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19217";
	newscript->pGossipHello = &GossipHello_npc_19217;
	newscript->pGossipSelect = &GossipSelect_npc_19217;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19723_19724";
	newscript->GetAI = &GetAI_npc_19723_19724;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19494";
	newscript->GetAI = &GetAI_npc_19494;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19481";
	newscript->GetAI = &GetAI_npc_19481;
	newscript->pQuestRewardedNPC = &QuestComplete_npc_19481;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19546";
	newscript->GetAI = &GetAI_npc_19546;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19544";
	newscript->GetAI = &GetAI_npc_19544;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19545";
	newscript->GetAI = &GetAI_npc_19545;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_19543";
	newscript->GetAI = &GetAI_npc_19543;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_sunfury_astromancer";
	newscript->GetAI = &GetAI_mob_sunfury_astromancer;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_19489";
	newscript->GetAI = &GetAI_mob_19489;
	newscript->pQuestRewardedNPC = &QuestComplete_mob_19489;
	newscript->RegisterSelf();
}

