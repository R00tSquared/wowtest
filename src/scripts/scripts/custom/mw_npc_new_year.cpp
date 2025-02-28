// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "Language.h"

#define NPC_TRICKY_KOBOLD 693101
#define NPC_FEROCIOUS_KOBOLD 693103
#define NPC_WISE_KOBOLD 693104

const uint32 npc_adds[3] = { NPC_TRICKY_KOBOLD, NPC_FEROCIOUS_KOBOLD, NPC_WISE_KOBOLD };
const int32 princess_texts[5] = { -1000019, -1000015, -1000016, -1000017, -1000018 };
// NY Elf helper
#define SPELL_ARCANE_BLAST 55187 // ~1150 damage
#define SPELL_HEAL 55188 // ~1700 heal
#define SPELL_DISENGAGE 55189
#define SPELL_RESTORE_MANA 28722 // restores mana with castCustomSpell BP

// Warlock
#define SPELL_CURSE 29930
uint32 curses[6] = { 13338, 29930, 17105, 43439, 18223, 34942 };

// Tricky Kobold
#define SPELL_REND 43246 // 250 per 3 sec, stackable up to 99 charges. 15 sec duration

// Ferocious Kobold
#define SPELL_KNOCKDOWN 35783 // stuns for 2 sec one target + some damage
#define SPELL_CLEAVE 40504 // affects 3 targets for 110% weapon damage

// Wise Kobold
#define SPELL_FIREBALL 44237 // ~2k damage, 2 sec cast time
#define SPELL_FROST_ARMOR 18100 // just frost armor

// Kobold Prince
#define SPELL_HEALING_WAVE 38330 // Heal for ~100k, 0.9 sec cast
#define SPELL_HEALING_WAVE_SELF 25396 // heal for ~2300, 3 sec cast

// player snowball to stop summonning cast
#define SPELL_SNOWBALL_INTERRUPT 21343 //25677 // knockback effect switched to dummyeffect, cause it stops cast. Will be removed after event

// Kobold King
#define SPELL_SUMMONNING 55201 // calls 3 tricky, 3 ferocious and 3 wise kobolds
#define SPELL_CLEAVE_KING 40504 // 110% weapon damage on 3 targets
#define SPELL_FIRE_NOVA 55202 // triggers 26073 every 2 sec // ~5600 damage for 45 yd range. when modifier is 1

struct npc_new_year_2015_AI : public ScriptedAI
{
    npc_new_year_2015_AI(Creature* c) : ScriptedAI(c) {}

    uint32 stealthRestoreCheck;

    void Reset()
    {
        stealthRestoreCheck = me->GetEntry() == 690002 ? 5000 : 0;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == 42805 && !me->HasAura(55159) && !me->HasAura(42805) && me->CanFreeMove()) // Dirty Trick new year spell
        {
            if (me->GetEntry() == 690002)
            {
                me->RemoveAurasDueToSpell(42866);
                stealthRestoreCheck = me->GetEntry() == 690002 ? 5000 : 0;
            }
            else if (me->GetEntry() == 690003)
                me->CastSpell(caster, 25515, true);

            me->AddAura(55154, me);
            me->AddAura(6844, me);
            if (Aura* a = me->GetAura(6844, 0))
                a->SetAuraDuration(5000);
            if (caster->GetTypeId() == TYPEID_PLAYER)
                me->Say(caster->ToPlayer()->GetSession()->GetHellgroundString(LANG_SCRIPT_2015_EVENT_KOBOLD), LANG_UNIVERSAL, 0);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (stealthRestoreCheck)
        {
            if (stealthRestoreCheck <= diff)
            {
                me->CastSpell(me, 42866, true);
                stealthRestoreCheck = 0;
            }
            else
                stealthRestoreCheck -= diff;
        }
    }

};

bool DeathSide_new_year_2015_kobold(Player* Plr, Creature* pCreature)
{
    Plr->PlayerTalkClass->ClearMenus();
    if (!pCreature->HasAura(55154))
        return true; // should not talk

    switch (pCreature->GetEntry())
    {
    case 690000:
        if (Plr->GetQuestRewardStatus(690901))
            return true;
        break;
    case 690001:
        if (Plr->GetQuestRewardStatus(690902))
            return true;
        break;
    case 690002:
        if (Plr->GetQuestRewardStatus(690903))
            return true;
        break;
    default:
        break;
    }

    Plr->PrepareQuestMenu(pCreature->GetGUID());
    Plr->SendPreparedQuest(pCreature->GetGUID());
    return true;
}

CreatureAI* GetAI_npc_new_year_2015_AI(Creature* _Creature)
{
    return new npc_new_year_2015_AI(_Creature);
}

struct DeathSide_New_Year_2017_elf_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_elf_AI(Creature* c) : ScriptedAI(c){}
    bool checkAlone;

    Timer actionDelay;
    int32 manaBP0;
    void Reset()
    {
        checkAlone = false;
        me->SetReactState(REACT_DEFENSIVE);
        me->SetAggroRange(0);
        me->CombatStopWithPets();
        me->ClearInCombat();
        me->AttackStop();
        actionDelay.Reset(2000);

        me->CastSpell(me, 40733, true); // Divine Shield
        manaBP0 = 2000;
    }

	void UpdateAI(const uint32 diff)
	{
		Unit *owner = me->GetOwner();
		Unit *victim = me->GetVictim();
		Player* pOwner = NULL;

		if (owner && owner->GetTypeId() == TYPEID_PLAYER)
			pOwner = (Player*)owner;

		if (!pOwner || pOwner->isDead())
		{
			me->ForcedDespawn();
			return;
		}

		if (!checkAlone)
		{
			if (pOwner->CountGuardianWithEntry(me->GetEntry()) > 1) // we should despawn, has more than 1 guardian
			{
				me->ForcedDespawn();
				return;
			}
			checkAlone = true;
		}

		if (victim)
		{
			Unit* victimTarget = victim->GetVictim();
			if (victimTarget && victimTarget->GetTypeId() == TYPEID_UNIT)
			{
				if (((Creature*)victimTarget)->GetEntry() == me->GetEntry())
					return; // do nothing if the tank is another elf
			}
		}

		if (!me->IsInCombat())
		{
			if (actionDelay.Expired(diff))
			{
				if (pOwner->GetHealthPercent() < 100) // first priority - owner
					DoCast(pOwner, SPELL_HEAL, true);
				else if (Unit* healTarget = SelectLowestHpFriendly(50, 1)) // second priority - no full health player
					DoCast(healTarget, SPELL_HEAL, true);
				else if (me->GetHealthPercent() < 100) // at last, heal ourself if needed
					DoCast(me, SPELL_HEAL, true);

				if (pOwner->getPowerType() == POWER_MANA && pOwner->GetPower(POWER_MANA) < pOwner->GetMaxPower(POWER_MANA)*0.6f)
					me->CastCustomSpell(pOwner, SPELL_RESTORE_MANA, &manaBP0, NULL, NULL, true);

				actionDelay = 2000;
			}
		}

		Unit *attacker = pOwner->GetAttackerForHelper();

		if (!me->IsWithinDistInMap(pOwner, 50.0f) || (!victim || !attacker))
		{
			if (!me->GetVictim() || !me->IsWithinDistInMap(pOwner, 50.0f))
				if (!me->HasUnitState(UNIT_STAT_FOLLOW))
				{
					victim = NULL;
					attacker = NULL;
					me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, urand(M_PI, M_PI / 2));
					Reset();
					return;
				}
		}

		// TEST?
		if (victim)
		{
			Unit* victimVictim = victim->GetVictim();
			if (victimVictim && victimVictim->GetTypeId() == TYPEID_UNIT && victimVictim->GetEntry() == 693115)
			{
				((Creature*)victim)->AI()->EnterEvadeMode();
				me->AttackStop();
				return;
			}
		}

		if (victim || attacker)
		{
			if (attacker)
			{
				if (attacker->GetTypeId() != TYPEID_UNIT)
				{
					//me->DestroyForNearbyPlayers();
					//me->RemoveFromWorld();
					me->ForcedDespawn();
					return;
				}
				me->SetInCombatWith(attacker);
				ScriptedAI::AttackStartNoMove(attacker, CHECK_TYPE_CASTER);
			}
			else
			{
				if (victim->GetTypeId() != TYPEID_UNIT)
				{
					//me->DestroyForNearbyPlayers();
					//me->RemoveFromWorld();
					me->ForcedDespawn();
					return;
				}
				me->SetInCombatWith(victim);
				ScriptedAI::AttackStartNoMove(victim, CHECK_TYPE_CASTER);
			}

			if (me->HasUnitState(UNIT_STAT_CASTING))
				return;

			if (actionDelay.Expired(diff))
			{
				me->CastSpell(me->GetVictim(), SPELL_DISENGAGE, true); // we will not be the target for a unit
				if (pOwner->GetHealthPercent() < 90) // first priority - owner
					DoCast(pOwner, SPELL_HEAL);
				else if (Unit* healTarget = SelectLowestHpFriendly(50, 5000)) // second priority - player who have lost 3k health - needs healing!
					DoCast(healTarget, SPELL_HEAL);
				else if (me->GetHealthPercent() < 80) // at last, heal ourself if needed
					DoCast(me, SPELL_HEAL);
				else
					DoCast(me->GetVictim(), SPELL_ARCANE_BLAST);

				if (pOwner->getPowerType() == POWER_MANA && pOwner->GetPower(POWER_MANA) < pOwner->GetMaxPower(POWER_MANA)*0.6f)
					me->CastCustomSpell(pOwner, SPELL_RESTORE_MANA, &manaBP0, NULL, NULL, true);

				actionDelay = 500;
			}
		}
	}
};

struct DeathSide_New_Year_2017_tricky_kobold_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_tricky_kobold_AI(Creature* c) : ScriptedAI(c){}

    Timer rendTimer;
    bool enraged;

    void Reset()
    {
        rendTimer.Reset(urand(1000, 1500));
        enraged = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (rendTimer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100, true, me->getVictimGUID());
            if (target && target->GetTypeId() == TYPEID_PLAYER)
            {
                DoResetThreat();
                me->getThreatManager().setCurrentVictim((HostileReference*)target);
                me->AI()->AttackStart(target);
                me->AddThreat(target, 5000000.0f);
                
                DoCast(target, 41176);
                rendTimer = urand(10000, 12000);
            }
        }

        if (!enraged && HealthBelowPct(30))
        {
            DoCast(me, 30470);
            enraged = true;
        }   

        DoMeleeAttackIfReady();
    }
};

struct DeathSide_New_Year_2017_warlock_kobold_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_warlock_kobold_AI(Creature* c) : ScriptedAI(c){}

    Timer rendTimer;
    Timer fireballTimer;

    void Reset()
    {
        rendTimer.Reset(urand(2000, 3000));
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STAT_CASTING))
            return;

        if (rendTimer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 100.0f, true);
            if (target && target->GetTypeId() == TYPEID_PLAYER)
            {
                uint32 curse = curses[urand(0, 5)];
                if (curse == 43439 && target->HasAura(43439))
                    curse = 29930;

                DoCast(target, curse);
                rendTimer = urand(2000, 3000);
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct DeathSide_New_Year_2017_paladin_kobold_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_paladin_kobold_AI(Creature* c) : ScriptedAI(c){}

    Timer rendTimer;
    Timer consecration;
    bool enraged;

    void Reset()
    {
        rendTimer.Reset(urand(2000, 5000));
        consecration.Reset(urand(10000, 15000));
        enraged = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (rendTimer.Expired(diff))
        {
            Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if (target && target->GetTypeId() == TYPEID_PLAYER)
            {
                DoCast(target, 5588);
                rendTimer = urand(16000, 20000);
            }
        }

        if (consecration.Expired(diff))
        {
            DoCast(me->GetVictim(), 38385);
            consecration = urand(50000, 80000);
        }      

        if (!enraged && HealthBelowPct(50))
        {
            DoCast(me->GetVictim(), 31884); // wings
            enraged = true;
        }

        DoMeleeAttackIfReady();
    }
};

struct DeathSide_New_Year_2017_ferocious_kobold_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_ferocious_kobold_AI(Creature* c) : ScriptedAI(c){}

    Timer knockDownTimer;
    Timer shout;
    Timer cleaveTimer;
    void Reset()
    {
        knockDownTimer.Reset(urand(10000, 12000));
        cleaveTimer.Reset(urand(2000, 3500));
        shout.Reset(urand(12000, 18000));
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (knockDownTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_KNOCKDOWN);
            knockDownTimer = urand(15000, 16000);
        }

        if (cleaveTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE);
            cleaveTimer = urand(3000, 5000);
        }

        if (shout.Expired(diff))
        {
            DoCast(me->GetVictim(), 23511);
            shout = urand(12000, 18000);
        }

        DoMeleeAttackIfReady();
    }
};

struct DeathSide_New_Year_2017_wise_kobold_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_wise_kobold_AI(Creature* c) : ScriptedAI(c){}

    Timer fireballTimer;
    Timer blizzrad;
    bool frostArmor;
    bool enraged;

    void Reset()
    {
        fireballTimer.Reset(urand(500, 1500));
        blizzrad.Reset(urand(6000, 8000));
        frostArmor = false;
        enraged = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!enraged && HealthBelowPct(20))
        {
            DoCast(me->GetVictim(), 45438); // ice block
            enraged = true;
        }

        if (me->HasUnitState(UNIT_STAT_CASTING))
            return;

        if (!frostArmor)
        {
            AddSpellToCast(me, SPELL_FROST_ARMOR);
            frostArmor = true;
        }

        if (blizzrad.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 46195);
            blizzrad = urand(6000, 8000);
        }

        if (fireballTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_FIREBALL);
            fireballTimer = urand(500,1500);
        }

        CheckCasterNoMovementInRange(diff, 40.0f);
        CastNextSpellIfAnyAndReady();
    }
};

struct DeathSide_New_Year_2017_kobold_prince_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_kobold_prince_AI(Creature* c) : ScriptedAI(c){}

    Timer healTimer;
    void Reset()
    {
        healTimer.Reset(urand(500, 1500));
    }

    void SpellHit(Unit* caster, const SpellEntry* info)
    {
        if (info && info->Id == SPELL_SNOWBALL_INTERRUPT)
            DoCast(me, 5727);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (me->HasUnitState(UNIT_STAT_CASTING))
            return;

        if (healTimer.Expired(diff))
        {
            if (Unit* healTarget = SelectLowestHpFriendly(40, 2000))
            {
                if (healTarget != me)
                    AddSpellToCast(healTarget, SPELL_HEALING_WAVE);
                else
                    AddSpellToCast(healTarget, SPELL_HEALING_WAVE_SELF);

                healTimer = urand(2000, 5000);
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

struct DeathSide_New_Year_2017_kobold_princess_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_kobold_princess_AI(Creature* c) : ScriptedAI(c){}

    uint64 PlayerGUID;
    Timer_UnCheked enrageTimer;
    bool dmg_ignore;

    void Reset()
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE | UNIT_FLAG_PACIFIED);
        dmg_ignore = true;
    }

    void DamageTaken(Unit* done_by, uint32& damage)
    {
        if (dmg_ignore)
            damage = (done_by && done_by->GetEntry() == 693100 && damage == 1) ? 1 : 0;       
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (!caster)
            return;

        if (!me->IsInCombat())
            AttackStart(caster);

        if (spell && spell->Id == SPELL_SNOWBALL_INTERRUPT)
        {
            me->DealDamage(me, 1, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

            if (me->GetHealth() > 1 && !(rand() % 3))
                DoScriptText(princess_texts[urand(0, 4)], me);

            if (me->GetHealth() == 1)
            {
                dmg_ignore = false;
                me->SetPlayerDamageReqInPct(0);
                caster->ToPlayer()->DealDamage(me, 100, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            }
        }
    }

    void JustDied(Unit* killer)
    {
        DoScriptText(-1000020, me);
    }
};

struct DeathSide_New_Year_2017_kobold_king_AI : public ScriptedAI
{
    DeathSide_New_Year_2017_kobold_king_AI(Creature* c) : ScriptedAI(c){}

    Timer warnSpellTimer;
    bool nextIsFireNova;
    Timer cleaveTimer;
    Timer clapTimer;
    Timer windTimer;
    uint32 hit_count;
    bool enraged;

    void Reset()
    {
        warnSpellTimer.Reset(urand(20000, 40000));
        cleaveTimer.Reset(urand(2000, 3500));
        windTimer.Reset(urand(5000, 10000));
        clapTimer.Reset(10000);
        nextIsFireNova = true;
        hit_count = 0;
        enraged = false;
    }

    void SpellHit(Unit* caster, const SpellEntry* info)
    {
        if (info && info->Id == SPELL_SNOWBALL_INTERRUPT && nextIsFireNova)
        {
            if (hit_count == 20)
            {
                me->InterruptNonMeleeSpells(false);
                hit_count = 0;
            }
            else
            {
                ++hit_count;
            }            
        }      
    }

    void EventPulse(Unit* /*pSender*/, uint32 /*PulseEventNumber*/) // called to summon adds from SpellEffects on spellCast SPELL_SUMMONNING end
    {
        for (uint32 i = 0; i < 3; ++i)
        {
            for (uint32 j = 0; j < 3; ++j)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                {
                    float x, y, z;
                    target->GetNearPoint(x, y, z, 0.0f, 5.0f, frand(0, 2 * M_PI));
                    if (Creature* summ = me->SummonCreature(npc_adds[j], x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000))
                        summ->AI()->AttackStart(target);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (warnSpellTimer.Expired(diff))
        {            
            ForceSpellCast(me, nextIsFireNova ? SPELL_FIRE_NOVA : SPELL_SUMMONNING);
            warnSpellTimer = 30000;

            // tell everyone about cast start 
            Map *map = m_creature->GetMap();
            if (map && map->IsDungeon())
            {
                Map::PlayerList const &PlayerList = map->GetPlayers();
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    if (Player* i_pl = i->getSource())
                        me->Whisper(nextIsFireNova ? LANG_NY_2017_BOSS_SUMM_WARN_FIRE : LANG_NY_2017_BOSS_SUMM_WARN_SPAWN, i_pl->GetGUID(), true);
            }
            nextIsFireNova = !nextIsFireNova;
            return;
        }

        if (me->HasUnitState(UNIT_STAT_CASTING))
            return;

        if (cleaveTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_CLEAVE_KING);
            cleaveTimer = urand(3000, 5000);
        }

        if (windTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 38229);
            windTimer = urand(5000, 10000);
        }

        if (clapTimer.Expired(diff))
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 50.0f, true))
            {
                float x, y, z;
                target->GetNearPoint(x, y, z, 0.0f, 5.0f, frand(0, 2 * M_PI));
                if (Creature* summ = me->SummonCreature(npc_adds[urand(0,2)], x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
                    summ->AI()->AttackStart(target);
            }
            clapTimer = urand(30000, 50000);
        }

        if (!enraged && HealthBelowPct(10))
        {
            AddSpellToCast(me, 31901);
            enraged = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_DeathSide_New_Year_2017_elf_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_elf_AI (pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_warlock_kobold_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_warlock_kobold_AI(pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_paladin_kobold_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_paladin_kobold_AI(pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_tricky_kobold_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_tricky_kobold_AI(pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_ferocious_kobold_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_ferocious_kobold_AI(pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_wise_kobold_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_wise_kobold_AI(pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_kobold_prince_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_kobold_prince_AI(pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_kobold_king_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_kobold_king_AI(pCreature);
}

CreatureAI* GetAI_DeathSide_New_Year_2017_kobold_princess_AI(Creature* pCreature)
{
    return new DeathSide_New_Year_2017_kobold_princess_AI(pCreature);
}

bool Elf_Hello(Player* player, Creature* creature)
{
    if (!player->IsInCombat() && player->CountGuardianWithEntry(693115) == 0)
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15535), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_NO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    }    

    player->SEND_GOSSIP_MENU(1, creature->GetGUID());
    return true;
}

bool Elf_Gossip(Player *player, Creature *creature, uint32 uiSender, uint32 uiAction)
{
    if (player->IsInCombat() || player->CountGuardianWithEntry(693115) >= 1)
    {
        player->CLOSE_GOSSIP_MENU();
        return false;
    }
    
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CastSpell(player, 55199, false);
    }

    player->CLOSE_GOSSIP_MENU();
    return true;
}


 void AddSC_DeathSide_New_Year_2017()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "elf_gossip";
     newscript->pGossipHello = &Elf_Hello;
     newscript->pGossipSelect = &Elf_Gossip;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_elf";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_elf_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_tricky_kobold";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_tricky_kobold_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_warlock_kobold";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_warlock_kobold_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_paladin_kobold";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_paladin_kobold_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_ferocious_kobold";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_ferocious_kobold_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_wise_kobold";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_wise_kobold_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_kobold_prince";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_kobold_prince_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_kobold_king";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_kobold_king_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "DeathSide_New_Year_2017_kobold_princess";
     newscript->GetAI = &GetAI_DeathSide_New_Year_2017_kobold_princess_AI;
     newscript->RegisterSelf();

     newscript = new Script;
     newscript->Name = "new_year_2015_kobold";
     newscript->pGossipHello = &DeathSide_new_year_2015_kobold;
     newscript->GetAI = &GetAI_npc_new_year_2015_AI;
     newscript->RegisterSelf();
 }
