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
 SDName: Boss_Shade_of_Aran
 SD%Complete:
 SDComment:
 SDCategory: Karazhan
 EndScriptData
 */

#include "precompiled.h"
#include "../../special/simple_ai.h"
#include "def_karazhan.h"
#include "GameObject.h"

#define SAY_AGGRO1                  -1532073
#define SAY_AGGRO2                  -1532074
#define SAY_AGGRO3                  -1532075
#define SAY_FLAMEWREATH1            -1532076
#define SAY_FLAMEWREATH2            -1532077
#define SAY_BLIZZARD1               -1532078
#define SAY_BLIZZARD2               -1532079
#define SAY_EXPLOSION1              -1532080
#define SAY_EXPLOSION2              -1532081
#define SAY_DRINK                   -1532082                //Low Mana / AoE Pyroblast
#define SAY_ELEMENTALS              -1532083
#define SAY_KILL1                   -1532084
#define SAY_KILL2                   -1532085
#define SAY_TIMEOVER                -1532086
#define SAY_DEATH                   -1532087
#define SAY_ATIESH                  -1532088                //Atiesh is equipped by a raid member

 // Spells
#define SPELL_FROSTBOLT         29954
#define SPELL_FIREBALL          29953
#define SPELL_ARCMISSLE         29955
#define SPELL_CHAINSOFICE       29991
//#define SPELL_DRAGONSBREATH     29964 disabled in 2.4.3
#define SPELL_MASSSLOW          30035
#define SPELL_FLAME_WREATH      30004
#define SPELL_AOE_CS            29961
#define SPELL_AEXPLOSION        29973
#define SPELL_MASS_POLY         29963
#define SPELL_BLINK_CENTER      29967
#define SPELL_ELEMENTAL1        29962
#define SPELL_ELEMENTAL2        37053
#define SPELL_ELEMENTAL3        37051
#define SPELL_ELEMENTAL4        37052
#define SPELL_CONJURE           29975
#define SPELL_DRINK             30024
#define SPELL_POTION            32453
#define SPELL_AOE_PYROBLAST     29978
#define SPELL_SUMMON_BLIZZARD   29969
#define SPELL_MAGNETIC_PULL     29979
#define SPELL_TELEPORT_MIDDLE   39567           // used also by npc_berthold not sure if valid here

// Creature Spells
#define SPELL_CIRCULAR_BLIZZARD         29952
#define SPELL_CIRCULAR_BLIZZARD_TICK    29951
#define SPELL_WATERBOLT                 37054
#define SPELL_SHADOW_PYRO               29978
#define SPELL_FROSTBOLT_VOLLEY          38837
#define SPELL_AMISSILE_VOLLEY           29960

// Creatures
#define CREATURE_WATER_ELEMENTAL    17167
#define CREATURE_SHADOW_OF_ARAN     18254
#define CREATURE_BLIZZARD_TRIGGER   17161

enum SuperSpell
{
	SUPER_FLAME = 0,
	SUPER_BLIZZARD,
	SUPER_AE,
};

enum DrinkingState
{
	DRINKING_NO_DRINKING,
	DRINKING_PREPARING,
	DRINKING_DONE_DRINKING,
	DRINKING_POTION
};

float ElementalSpawnPoints[2][4] = {
	{-11143.5, -11167.6, -11186.8, -11162.6},   // X coord
	{-1914.26, -1933.8,  -1909.7,  -1895.4}     // Y coord
};

float shadowOfAranSpawnPoints[2][8] = {
	{-11143.5, -11152.1, -11167.6, -11181.3, -11186.8, -11178,  -11162.6, -11148.6},// X coord
	{-1914.26, -1928.2,  -1933.8,  -1925.05, -1909.7,  -1895.7, -1895.4,  -1899}    // Y coord
};

struct SpawnPosition
{
	float X, Y, Z, R;
};

const SpawnPosition BlizzardPositions[] =
{
	{-11166.298, -1891.050, 232.008, 4.672},
	{-11146.043, -1904.276, 233.387, 3.532},
	{-11152.590, -1928.869, 232.008, 2.217},
	{-11177.150, -1929.750, 232.009, 0.894},
	{-11185.700, -1911.544, 232.008, 0.182},
	{-11172.933, -1892.884, 232.008, 5.163},
	{-11148.536, -1897.451, 232.008, 3.848},
	{-11144.821, -1917.088, 232.008, 2.851}
};
uint32 BlizzardPositions_Size = 8;

struct circular_blizzardAI : public Scripted_NoMovementAI
{
	circular_blizzardAI(Creature *c) : Scripted_NoMovementAI(c)
	{
		pInstance = c->GetInstanceData();
	}

	ScriptedInstance *pInstance;
	Timer castTimer;
	Timer despawnTimer;
	uint64 aranGuid;

	void Reset()
	{
		despawnTimer.Reset(41000);
	}

	void StartBlizzardIn(uint32 castTime, uint64 guid)
	{
		despawnTimer = castTime + 10000;
		castTimer.Reset(castTime);
		aranGuid = guid;
	}

	void JustDied(Unit* killer) {}

	void UpdateAI(const uint32 diff)
	{
		if (despawnTimer.Expired(diff))
			me->ForcedDespawn();

		if (castTimer.Expired(diff))
		{
			DoCast(me, SPELL_CIRCULAR_BLIZZARD_TICK, true);
			castTimer = 50000;
		}
	}
};

struct boss_aranAI : public ScriptedAI
{
	boss_aranAI(Creature *c) : ScriptedAI(c), Summons(me)
	{
		pInstance = (c->GetInstanceData());
		me->GetPosition(wLoc);
	}

	ScriptedInstance* pInstance;
	SummonList Summons;
	WorldLocation wLoc;

	Timer BasicSpellsCastTimer;
	Timer CounterSpellTimer;
	Timer SuperCastTimer;
	Timer BerserkTimer;
	Timer CheckTimer;
	Timer PyroblastTimer;
	Timer DrinkingDelay;
	Timer ConjureDelay;
	Timer DrinkDelay;

	uint8 LastSuperSpell;
	uint64 shadeOfAranTeleportCreatures[8];
	DrinkingState Drinking;
	uint64 TargetGUID;

	bool ElementalsSpawned;
	bool drinked;
	bool pyroDone;

	void Reset()
	{
		me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CASTING_SPEED, true);
		me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_HASTE_SPELLS, true);
		me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_SILENCE, true);

		ClearCastQueue();

		CounterSpellTimer.Reset(5000);
		BasicSpellsCastTimer.Reset(1000);
		SuperCastTimer.Reset(30000);
		BerserkTimer.Reset(720000);
		CheckTimer.Reset(3000);
		PyroblastTimer.Reset(0);
		DrinkingDelay.Reset(0);
		LastSuperSpell = rand() % 3;

		ElementalsSpawned = false;
		drinked = false;
		Drinking = DRINKING_NO_DRINKING;
		pyroDone = false;

		TargetGUID = 0;
		me->SetRooted(false);

		if (pInstance)
			pInstance->SetData(DATA_SHADEOFARAN_EVENT, NOT_STARTED);
	}

	void KilledUnit(Unit *victim)
	{
		DoScriptText(RAND(SAY_KILL1, SAY_KILL2), me);
	}

	void JustDied(Unit *victim)
	{
		DoScriptText(SAY_DEATH, me);

		if (pInstance)
			pInstance->SetData(DATA_SHADEOFARAN_EVENT, DONE);

		Summons.DespawnAll();
	}

	bool PlayerHaveAtiesh()
	{
		Map::PlayerList const &PlayerList = ((InstanceMap*)me->GetMap())->GetPlayers();
		for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
		{
			Player* i_pl = i->getSource();
			if (i_pl->HasEquiped(22632) || i_pl->HasEquiped(22631) || i_pl->HasEquiped(22630) || i_pl->HasEquiped(22589))
				return true;
		}
		return false;
	}

	void EnterCombat(Unit *who)
	{
		if (PlayerHaveAtiesh())
			DoScriptText(SAY_ATIESH, me);
		else
			DoScriptText(RAND(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3), me);

		if (pInstance)
			pInstance->SetData(DATA_SHADEOFARAN_EVENT, IN_PROGRESS);

		me->SetRooted(true);
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (CheckTimer.Expired(diff))
		{
			if (!me->IsWithinDistInMap(&wLoc, 35.0f))
				EnterEvadeMode();
			else
				DoZoneInCombat();

			CheckTimer = 3000;
		}

		if (ConjureDelay.Expired(diff))
		{
			me->CastSpell(me, SPELL_CONJURE, false);
			DoScriptText(SAY_DRINK, me);
			DrinkDelay = 2000;
			ConjureDelay = 0;
		}

		if (DrinkDelay.Expired(diff))
		{
			me->CastSpell(me, SPELL_DRINK, false);
			DrinkDelay = 0;
		}

		if (DrinkingDelay.GetInterval())
		{
			if (DrinkingDelay.Expired(diff))
				DrinkingDelay = 0;
		}

		if (!DrinkingDelay.GetInterval() && Drinking == DRINKING_NO_DRINKING && (me->GetPower(POWER_MANA) * 100 / me->GetMaxPower(POWER_MANA)) < 20)
		{
			if (!drinked)
			{
				DrinkingDelay = 5000;
				ClearCastQueue();
				me->SetRooted(true);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
				Drinking = DRINKING_PREPARING;
				me->CastSpell(me, SPELL_MASS_POLY, false);
				ConjureDelay = 2000;
				return;
			}
			else
			{
				DrinkingDelay = 15000;
				Drinking = DRINKING_POTION; // so every next drinking is potion
			}
		}

		if (Drinking == DRINKING_POTION)
		{
			DrinkingDelay = 15000;
			AddSpellToCast(SPELL_POTION, CAST_SELF);
			if (drinked)
				Drinking = DRINKING_NO_DRINKING;
		}

		if (!pyroDone)
		{
			if (PyroblastTimer.Expired(diff))
			{
				AddSpellToCast(SPELL_AOE_PYROBLAST, CAST_SELF);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
				PyroblastTimer = 0;
			}
		}

		if (Drinking == DRINKING_NO_DRINKING)
		{
			// Normal casts
			if (BasicSpellsCastTimer.Expired(diff))
			{
				if (!me->IsNonMeleeSpellCast(false))
				{
					uint32 Spells[3];
					uint8 AvailableSpells = 0;
					// Check for what spells are not on cooldown
					if (!me->isSchoolProhibited(SPELL_SCHOOL_MASK_ARCANE))
					{
						Spells[AvailableSpells] = SPELL_ARCMISSLE;
						AvailableSpells++;
					}

					if (!me->isSchoolProhibited(SPELL_SCHOOL_MASK_FIRE))
					{
						Spells[AvailableSpells] = SPELL_FIREBALL;
						AvailableSpells++;
					}

					if (!me->isSchoolProhibited(SPELL_SCHOOL_MASK_FROST))
					{
						Spells[AvailableSpells] = SPELL_FROSTBOLT;
						AvailableSpells++;
					}

					// If no available spells wait 1 second and try again
					if (AvailableSpells)
					{
						ForceSpellCast(Spells[rand() % AvailableSpells], CAST_RANDOM);
						me->SetRooted(true);
					}
					else
						me->SetRooted(false);
				}

				BasicSpellsCastTimer = 1000;
			}

			if (CounterSpellTimer.Expired(diff))
			{
				AddSpellToCast(SPELL_AOE_CS, CAST_NULL);
				CounterSpellTimer = urand(10000, 15000);
			}

			if (SuperCastTimer.Expired(diff))
			{
				uint8 Available[2];
				ClearCastQueue();

				switch (LastSuperSpell)
				{
				case SUPER_AE:
					Available[0] = SUPER_FLAME;
					Available[1] = SUPER_BLIZZARD;
					break;
				case SUPER_FLAME:
					Available[0] = SUPER_AE;
					Available[1] = SUPER_BLIZZARD;
					break;
				case SUPER_BLIZZARD:
					Available[0] = SUPER_FLAME;
					Available[1] = SUPER_AE;
					break;
				}

				LastSuperSpell = Available[rand() % 2];

				AddSpellToCast(SPELL_BLINK_CENTER, CAST_SELF);
				switch (LastSuperSpell)
				{
				case SUPER_AE:
					AddSpellToCast(SPELL_MAGNETIC_PULL, CAST_SELF);
					AddSpellToCast(SPELL_MASSSLOW, CAST_SELF);
					AddSpellToCast(SPELL_BLINK_CENTER, CAST_SELF);
					AddSpellToCastWithScriptText(SPELL_AEXPLOSION, CAST_SELF, RAND(SAY_EXPLOSION1, SAY_EXPLOSION2));
					AddSpellToCast(SPELL_BLINK_CENTER, CAST_SELF);
					DrinkingDelay = 15000;
					break;

				case SUPER_FLAME:
					AddSpellToCastWithScriptText(SPELL_FLAME_WREATH, CAST_SELF, RAND(SAY_FLAMEWREATH1, SAY_FLAMEWREATH2));
					DrinkingDelay = 25000;
					break;

				case SUPER_BLIZZARD:
					ForceSpellCastWithScriptText(SPELL_SUMMON_BLIZZARD, CAST_NULL, RAND(SAY_BLIZZARD1, SAY_BLIZZARD2));
					uint32 startBlizzardIn = 3700;
					// Insert all from start to end
					uint32 startPos = 0;
					std::vector<SpawnPosition> blizzardPos(BlizzardPositions + startPos, BlizzardPositions + BlizzardPositions_Size);
					// Insert all from start to end
					for (uint8 i = 0; i < startPos; ++i)
						blizzardPos.push_back(BlizzardPositions[i]);

					// Spawn all blizzard positions and offset their cast time
					int i = 0;
					for (auto pos : blizzardPos)
					{
						if (i == 0 || i == 5)
						{
							// additional blizzard near the door
							if (Creature* spawn = me->SummonCreature(CREATURE_BLIZZARD_TRIGGER, -11184.076, -1887.924, 231.974, 0.182, TEMPSUMMON_TIMED_DESPAWN, 50000))
							{
								if (circular_blizzardAI* blizzAI = dynamic_cast<circular_blizzardAI*>(spawn->AI()))
								{
									blizzAI->StartBlizzardIn(startBlizzardIn, me->GetGUID());
								}
							}
						}

						if (Creature* spawn = me->SummonCreature(CREATURE_BLIZZARD_TRIGGER, pos.X, pos.Y, pos.Z, pos.R, TEMPSUMMON_TIMED_DESPAWN, 50000))
						{
							if (circular_blizzardAI* blizzAI = dynamic_cast<circular_blizzardAI*>(spawn->AI()))
							{
								blizzAI->StartBlizzardIn(startBlizzardIn, me->GetGUID());
								startBlizzardIn += 3700;
							}
						}

						++i;
					}

					DrinkingDelay = 30000;
					break;
				}

				SuperCastTimer = urand(35000, 40000);
			}

			if (!ElementalsSpawned && HealthBelowPct(40))
			{
				ElementalsSpawned = true;
				AddSpellToCast(SPELL_TELEPORT_MIDDLE, CAST_SELF);
				AddSpellToCastWithScriptText(SPELL_ELEMENTAL1, CAST_SELF, SAY_ELEMENTALS);
				AddSpellToCast(SPELL_ELEMENTAL2, CAST_SELF);
				AddSpellToCast(SPELL_ELEMENTAL3, CAST_SELF);
				AddSpellToCast(SPELL_ELEMENTAL4, CAST_SELF);
			}
		}

		if (BerserkTimer.Expired(diff))
		{
			for (uint32 i = 0; i < 8; i++)
			{
				Creature* pUnit = me->SummonCreature(CREATURE_SHADOW_OF_ARAN, shadowOfAranSpawnPoints[0][i], shadowOfAranSpawnPoints[1][i], me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
				if (pUnit)
				{
					pUnit->Attack(me->GetVictim(), true);
					pUnit->setFaction(me->getFaction());
				}
			}
			DoScriptText(SAY_TIMEOVER, me);

			BerserkTimer = 60000;
		}

		CastNextSpellIfAnyAndReady();
		if (Drinking == DRINKING_NO_DRINKING)
			DoMeleeAttackIfReady();
	}


	void SpellHitTarget(Unit *target, const SpellEntry *spell)
	{
		if (spell->Id == SPELL_MAGNETIC_PULL)
		{
			target->RemoveAurasDueToSpell(29947);
			target->CastSpell(target, SPELL_BLINK_CENTER, true);
		}
		else if (spell->Id == SPELL_FROSTBOLT && roll_chance_i(30))
		{
			me->CastSpell(target, SPELL_CHAINSOFICE, true);
		}
		//else if (spell->Id == SPELL_FIREBALL && roll_chance_i(30) && SuperCastTimer.GetInterval() >= 7000)
		//{
		//	me->CastSpell(target, SPELL_DRAGONSBREATH, true);
		//}
		else if (spell->Id == SPELL_AOE_PYROBLAST && !pyroDone)
		{
			Drinking = DRINKING_NO_DRINKING;
			BasicSpellsCastTimer = 2000;
			pyroDone = true;
		}
	}

	void JustSummoned(Creature *c)
	{
		if (c->GetEntry() == 17167)
		{
			c->AI()->AttackStart(me->GetVictim());
			c->setFaction(me->getFaction());
		}
		Summons.Summon(c);
	}

	void OnAuraRemove(Aura *aura, bool)
	{
		if (aura->GetId() == SPELL_DRINK)
		{
			if (!drinked)
			{
				drinked = true;
				PyroblastTimer.Reset(1000);
			}
			me->SetStandState(UNIT_STAND_STATE_STAND);
			me->AI()->AttackStart(Unit::GetPlayerInWorld(TargetGUID));
			Drinking = DRINKING_DONE_DRINKING;
		}
	}

	void OnAuraApply(Aura *aura, Unit *caster, bool)
	{
		if (aura->GetId() == SPELL_DRINK)
		{
			me->SetSelection(0);
			TargetGUID = caster->GetGUID();
			me->AttackStop();
			me->SetStandState(UNIT_STAND_STATE_SIT);
		}
	}

	void SpellHit(Unit* pAttacker, const SpellEntry* spellEntry)
	{
		if (spellEntry->Id != SPELL_DRINK && me->HasAura(SPELL_DRINK))
		{
			Drinking = DRINKING_POTION;
			me->InterruptNonMeleeSpells(true);
			me->RemoveAurasDueToSpell(SPELL_DRINK);
		}
	}
};

struct water_elementalAI : public ScriptedAI
{
	water_elementalAI(Creature *c) : ScriptedAI(c) {}

	Timer CastTimer2;

	void Reset()
	{
		ClearCastQueue();

		CastTimer2.Reset(urand(2000, 5000));
	}

	void AttackStart(Unit *who)
	{
		ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (CastTimer2.Expired(diff))
		{
			//AddSpellToCast(me->GetVictim(), SPELL_WATERBOLT);
			DoCast(me->GetVictim(), SPELL_WATERBOLT);
			CastTimer2 = 2000 + (rand() % 3000);
		}

		CheckCasterNoMovementInRange(diff, 45.0);
		CastNextSpellIfAnyAndReady();
	}
};

struct shadow_of_aranAI : public ScriptedAI
{
	shadow_of_aranAI(Creature *c) : ScriptedAI(c) {}

	Timer CastTimer;

	void Reset()
	{
		CastTimer.Reset(2000);
	}

	void UpdateAI(const uint32 diff)
	{
		if (!UpdateVictim())
			return;

		if (CastTimer.Expired(diff))
		{
			if (rand() % 3)
			{
				me->CastSpell(me, SPELL_FROSTBOLT_VOLLEY, false);
				CastTimer = 5000;
			}
			else
			{
				me->CastSpell(me, SPELL_AMISSILE_VOLLEY, false);
				CastTimer = 20000;
			}
		}

	}
};

CreatureAI* GetAI_boss_aran(Creature *_Creature)
{
	return new boss_aranAI(_Creature);
}

CreatureAI* GetAI_water_elemental(Creature *_Creature)
{
	return new water_elementalAI(_Creature);
}

CreatureAI* GetAI_shadow_of_aran(Creature *_Creature)
{
	shadow_of_aranAI* shadowAI = new shadow_of_aranAI(_Creature);

	return (CreatureAI*)shadowAI;
}

CreatureAI* GetAI_circular_blizzard(Creature *_Creature)
{
	return new circular_blizzardAI(_Creature);
}

bool FlameWreathHandleEffect(Unit *pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex)
{
	if (!pCaster || !pUnit)
		return true;

	pCaster->CastSpell(pUnit, 29946, true);
	return true;
}

void AddSC_boss_shade_of_aran()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name = "boss_shade_of_aran";
	newscript->GetAI = &GetAI_boss_aran;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_shadow_of_aran";
	newscript->GetAI = &GetAI_shadow_of_aran;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_aran_elemental";
	newscript->GetAI = &GetAI_water_elemental;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_aran_blizzard";
	newscript->GetAI = &GetAI_circular_blizzard;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "spell_flame_wreath";
	newscript->pSpellHandleEffect = &FlameWreathHandleEffect;
	newscript->RegisterSelf();
}


