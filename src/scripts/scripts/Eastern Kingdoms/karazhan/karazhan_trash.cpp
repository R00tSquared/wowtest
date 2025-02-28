// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
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

#include "precompiled.h"
#include "def_karazhan.h"

/*###
# npc_spectral_stable_hand
####*/

#define SPECTRAL_SAY_1 -1200000
#define SPECTRAL_SAY_2 -1200001
#define SPECTRAL_SAY_3 -1200002
#define SPECTRAL_SAY_4 -1200003
#define SPECTRAL_SAY_5 -1200004
#define SPECTRAL_SAY_6 -1200005
#define SPECTRAL_SAY_7 -1200006

enum SpectralStableHand
{
    SPELL_WHIP_RAGE = 29340,
    SPELL_KNOCKDOWN = 18812,
    SPELL_PIERCE_ARMOR = 6016,
    SPELL_HEALING_TOUCH = 29339,

    NPC_SPECTRAL_STALLION = 15548,
    NPC_SPECTRAL_CHARGER = 15547,

    ITEM_SHOVEL = 7495,
};

struct npc_spectral_stable_handAI : public ScriptedAI
{
    npc_spectral_stable_handAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();

        if (urand(0, 1))
        {
            ForkMob = false;
            // Change weapon to shovel:
            SetEquipmentSlots(false, ITEM_SHOVEL);
        }
        else
        {
            ForkMob = true;
            // Weapon is fork by default, no need to change
        }
    }

    // Can either be a shovel mob or a fork mob
    bool ForkMob;

    Timer SayTimer;
    Timer whipTimer;
    // Shovel:
    Timer knockTimer;
    // Fork:
    Timer healTimer;
    Timer pierceTimer;

    void Reset()
    {
        SayTimer.Reset(urand(10000, 20000));
        whipTimer.Reset(15000);
        healTimer.Reset(35000);
        pierceTimer.Reset(urand(20000, 25000));
        knockTimer.Reset(urand(5000, 10000));
    }

    void EnterCombat(Unit* who)
    {
        me->Say(RAND(-1200003, -1200006), 0, 0);
    }

    void JustDied(Unit* who)
    {
        me->Say(RAND(-1200004, -1200005), 0, 0);
    }

    void GetNearbyHorses(std::vector<Creature*>& out, float range)
    {
        std::list<Creature*> horses1list = FindAllCreaturesWithEntry(NPC_SPECTRAL_STALLION, range);
        std::list<Creature*> horses2list = FindAllCreaturesWithEntry(NPC_SPECTRAL_CHARGER, range);
        std::vector<Creature*> horse1;
        std::vector<Creature*> horse2;
        for (std::list<Creature*>::iterator i = horses1list.begin(); i != horses1list.end(); ++i)
            horse1.push_back(*i);

        for (std::list<Creature*>::iterator i = horses2list.begin(); i != horses2list.end(); ++i)
            horse2.push_back(*i);

        out.reserve(horse1.size() + horse2.size());
        out.insert(out.end(), horse1.begin(), horse1.end());
        out.insert(out.end(), horse2.begin(), horse2.end());
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (SayTimer.Expired(diff))
            {
                // 20% chance to do a random say
                if (!urand(0, 5))
                {
                    switch (urand(1, 3))
                    {
                    case 1:
                        me->MonsterSay(-1200000, LANG_UNIVERSAL);
                        break;
                    case 2:
                        me->MonsterSay(-1200001, LANG_UNIVERSAL);
                        break;
                    case 3:
                        me->MonsterSay(-1200002, LANG_UNIVERSAL);
                        break;
                    }
                }
                SayTimer = urand(10000, 20000);
            }
            return;
        }

        // Both
        if (whipTimer.Expired(diff))
        {
            std::vector<Creature*> horses;
            GetNearbyHorses(horses, 30.0f);
            Creature* tar = NULL;
            for (auto& horse : horses)
            {
                if (!(horse)->HasAura(SPELL_WHIP_RAGE) &&
                    (horse)->IsWithinLOSInMap(me) &&
                    (horse)->isAlive())
                {
                    tar = horse;
                    break;
                }
            }
            if (tar)
                AddSpellToCast(tar, SPELL_WHIP_RAGE);
            whipTimer = 30000;
        }

        if (ForkMob)
        {
            // Fork Mob
            if (healTimer.Expired(diff))
            {
                std::vector<Creature*> horses;
                GetNearbyHorses(horses, 40.0f);
                Creature* tar = NULL;
                float lastHp = 100.0f;
                for (auto& horse : horses)
                {
                    float hp = (horse)->GetHealthPercent();
                    if (hp < lastHp &&
                        (horse)->IsWithinLOSInMap(me) &&
                        (horse)->isAlive())
                    {
                        tar = horse;
                        lastHp = hp;
                    }
                }
                if (tar)
                    AddSpellToCast(tar, SPELL_HEALING_TOUCH);
                healTimer = 70000;
            }

            if (pierceTimer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), SPELL_PIERCE_ARMOR);
                pierceTimer = urand(40000, 50000);
            }
        }
        else
        {
            // Shovel Mob
            if (knockTimer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), SPELL_KNOCKDOWN);
                knockTimer = urand(10000, 20000);
            }
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_spectral_stable_hand(Creature* pCreature)
{
    return new npc_spectral_stable_handAI(pCreature);
}

#define SPELL_SP_CHARGE    29320

struct mob_spectral_chargerAI : public ScriptedAI
{
    mob_spectral_chargerAI(Creature* c) : ScriptedAI(c) 
    {}

    Timer ChargeTimer;

    void Reset()
    {
        ChargeTimer.Reset(urand(5000, 15000));
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ChargeTimer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_FARTHEST, 0))
                AddSpellToCast(target, SPELL_SP_CHARGE, false);
            ChargeTimer = urand(5000,15000);
        }

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_spectral_charger(Creature *_Creature)
{
    return new mob_spectral_chargerAI(_Creature);
}

/*bool Spell_charge(const Aura* aura, bool apply)
{
    if(!apply)
    {
        if(Unit* caster = aura->GetCaster())
            caster->CastSpell(aura->GetTarget(), 29321, true);      // trigger fear after charge
    }
    return true;
}*/

#define SPELL_DANCE_VIBE            29521
#define SPELL_SEARING_PAIN          29492
#define SPELL_IMMOLATE              29928
#define SPELL_THROW                 29582
#define SPELL_IMPALE                29583
#define SPELL_GOBLIN_DRAGON_GUN     29513
#define SPELL_THROW_DYNAMITE        29579
#define SPELL_PUNCH                 29581
#define SPELL_CURSE_OF_AGONY        29930
#define SPELL_HEAL                  29580
#define SPELL_HOLY_NOVA             29514

/*
Chairs:
Entry  guid  npcguid
183522 24402 496012
183535 24415 496236
183533 24413 496143
183612 24492 85145
183611 24491 85144
183776 24641 85148
183605 24485 85162
183588 24468 85170
*/


struct mob_phantom_guestAI : public ScriptedAI
{
    mob_phantom_guestAI(Creature* c) : ScriptedAI(c) 
    {
        Type = urand(0, 4);
    }

    uint32 Type;
    uint64 ChairGUID;
    Timer MainTimer;
    Timer SecondaryTimer;
    Timer SitTimer;

    GameObject* SelectChairInRange(uint64 guid, float radius)
    {
        GameObject *go = NULL;

        Hellground::GameObjectWithDbGUIDCheck go_check(*me, guid);
        Hellground::ObjectSearcher<GameObject, Hellground::GameObjectWithDbGUIDCheck> checker(go, go_check);

        Cell::VisitGridObjects(me, checker, radius);
        return go;
    }

    void Reset()
    {
        me->CastSpell(me, SPELL_DANCE_VIBE, true);

        MainTimer.Reset(1000);
        SecondaryTimer.Reset(urand(5000, 20000));
        switch(me->GetDBTableGUIDLow())
        {
            case 496012:
            {
                ChairGUID = 24402;
                SitTimer.Reset(1000);
                break;
            }
            case 496236:
            {
                ChairGUID = 24415;
                SitTimer.Reset(1000);
                break;
            }
            case 496143:
            {
                ChairGUID = 24413;
                SitTimer.Reset(1000);
                break;
            }
            case 85145:
            {
                ChairGUID = 24492;
                SitTimer.Reset(1000);
                break;
            }
            case 85144:
            {
                ChairGUID = 24491;
                SitTimer.Reset(1000);
                break;
            }
            case 85148:
            {
                ChairGUID = 24641;
                SitTimer.Reset(1000);
                break;
            }
            case 85162:
            {
                ChairGUID = 24485;
                SitTimer.Reset(1000);
                break;
            }
            case 85170:
            {
                ChairGUID = 24468;
                SitTimer.Reset(1000);
                break;
            }
            default:
            {
                ChairGUID = 0;
                SitTimer.Reset(0);
                break;
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        me->SetRooted(false);
        me->SetStandState(UNIT_STAND_STATE_STAND);
    }

    void AttackStart(Unit *who)
    {
        if(Type == 0 || Type == 1)
            ScriptedAI::AttackStartNoMove(who, Type == 0 ? CHECK_TYPE_CASTER : CHECK_TYPE_SHOOTER);
        else
            ScriptedAI::AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(SitTimer.Expired(diff))
        {
            if (GameObject *chair = SelectChairInRange(ChairGUID, 3))
            {
                me->NearTeleportTo(chair->GetPositionX(), chair->GetPositionY(), chair->GetPositionZ(), chair->GetOrientation());
                me->SetRooted(true);
                me->SetStandState(UNIT_STAND_STATE_SIT_LOW_CHAIR);
            }
            SitTimer = 0;
        }

        if(!UpdateVictim())
            return;

        if (MainTimer.Expired(diff))
        {
            switch(Type)
            {
            case 0:
                AddSpellToCast(SPELL_SEARING_PAIN, CAST_TANK);
                MainTimer = 3500;
                break;
            case 1:
                AddSpellToCast(SPELL_THROW, CAST_TANK);
                MainTimer = 2000;
                break;
            case 2:
                AddSpellToCast(SPELL_GOBLIN_DRAGON_GUN, CAST_SELF);
                MainTimer = 20000;
                break;
            case 3:
                AddSpellToCast(SPELL_PUNCH, CAST_TANK);
                MainTimer = urand(10000, 30000);
                break;
            case 4:
                AddSpellToCast(SPELL_HEAL, CAST_LOWEST_HP_FRIENDLY);
                MainTimer = urand(5000, 20000);
                break;
            }
        } 

        if (SecondaryTimer.Expired(diff))
        {
            switch(Type)
            {
            case 0:
                AddSpellToCast(SPELL_IMMOLATE, CAST_RANDOM);
                SecondaryTimer = urand(7000, 30000);
                break;
            case 1:
                AddSpellToCast(SPELL_IMPALE, CAST_RANDOM);
                SecondaryTimer = urand(5000, 30000);
                break;
            case 2:
                AddSpellToCast(SPELL_THROW_DYNAMITE, CAST_RANDOM);
                SecondaryTimer = urand(15000, 40000);
                break;
            case 3:
                AddSpellToCast(SPELL_CURSE_OF_AGONY, CAST_RANDOM);
                SecondaryTimer = urand(10000, 30000);
                break;
            case 4:
                AddSpellToCast(SPELL_HOLY_NOVA, CAST_SELF);
                SecondaryTimer = urand(10000, 30000);
                break;
            }
        }

        if(Type == 0)
            CheckCasterNoMovementInRange(diff, 30.0);
        else if(Type == 1)
            CheckShooterNoMovementInRange(diff, 30.0);

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_phantom_guest(Creature *_Creature)
{
    return new mob_phantom_guestAI(_Creature);
}

#define SPELL_DUAL_WIELD    674
#define SPELL_SHOT          29575
#define SPELL_MULTI_SHOT    29576

#define SENTRY_SAY_DEATH_1  -1200040
#define SENTRY_SAY_DEATH_2  -1200041

struct mob_spectral_sentryAI : public ScriptedAI
{
    mob_spectral_sentryAI(Creature* c) : ScriptedAI(c) {}

    Timer ShotTimer;
    Timer MultiShotTimer;
    Timer RandomSayTimer;
    Timer TalkEmoteTimer;

    void Reset()
    {
        me->CastSpell(me, SPELL_DUAL_WIELD, true);

        ShotTimer = 0;
        MultiShotTimer.Reset(8000);
        RandomSayTimer.Reset(urand(40000, 80000));
        switch(me->GetDBTableGUIDLow())
        {
            case 384589:
            case 384686:
            case 380123:
            case 380082:
            case 379950:
            case 379841:
                TalkEmoteTimer.Reset(urand(3000, 30000));
                break;
            default:
                TalkEmoteTimer.Reset(0);
                break;
        }
    }

    void AttackStart(Unit *who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
    }

    void JustDied(Unit *)
    {
        if(roll_chance_i(30))
            me->Say(RAND(-1200040, -1200041), 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(TalkEmoteTimer.Expired(diff))
        {
            me->HandleEmoteCommand(1);
            TalkEmoteTimer = urand(3000, 30000);
        }

        if(!UpdateVictim())
            return;

        if (ShotTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_SHOT, CAST_TANK);
            ShotTimer = 2000;
        } 

        if (MultiShotTimer.Expired(diff))
        {
            AddSpellToCast(SPELL_MULTI_SHOT, CAST_RANDOM);
            MultiShotTimer = 8000;
        }

        CheckShooterNoMovementInRange(diff, 20.0);
        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_spectral_sentry(Creature *_Creature)
{
    return new mob_spectral_sentryAI(_Creature);
}

#define SPELL_RETURN_FIRE1  29793
#define SPELL_RETURN_FIRE2  29794
#define SPELL_RETURN_FIRE3  29788
#define SPELL_FIST_OF_STONE 29840
#define SPELL_DETONATE      29876
#define SPELL_SEAR          29864
#define NPC_ASTRAL_SPARK    17283
#define ARC_PROTECTOR_AGGR_1    -1200019
#define ARC_PROTECTOR_AGGR_2    -1200020
#define ARC_PROTECTOR_AGGR_3    -1200021

struct mob_arcane_protectorAI : public ScriptedAI
{
    mob_arcane_protectorAI(Creature* c) : ScriptedAI(c) {}

    Timer SkillTimer;

    void Reset()
    {
        SkillTimer.Reset(urand(10000, 20000));
    }
    
    void EnterCombat(Unit *who)
    {
        me->CastSpell(me, RAND(SPELL_RETURN_FIRE1, SPELL_RETURN_FIRE2, SPELL_RETURN_FIRE3), false); 
    }

    void JustSummoned(Creature *c)
    {
        if (c->GetEntry() == NPC_ASTRAL_SPARK)
        {
            c->CastSpell(me, SPELL_DETONATE, true);
            c->CastSpell(me, SPELL_SEAR, true);
        }
    }

    void OnAuraApply(Aura *aur, Unit*, bool stack)
    {
        if(aur->GetEffIndex() == 0)
        {
            switch(aur->GetId())
            {
            case SPELL_RETURN_FIRE1:
                me->Say(-1200026, 0, 0);
                me->Say(-1200019, LANG_UNIVERSAL, 0);
                break;
            case SPELL_RETURN_FIRE2:
                me->Say(-1200027, 0, 0);
                me->Say(-1200020, LANG_UNIVERSAL, 0);
                break;
            case SPELL_RETURN_FIRE3:
                me->Say(-1200028, 0, 0);
                me->Say(-1200021, LANG_UNIVERSAL, 0);
                break;
            }
        }
    }

    void JustDied(Unit *)
    {
        if(roll_chance_i(75))
            me->Say(RAND(-1200037,
                                      -1200038,
                                      -1200039), 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (SkillTimer.Expired(diff))
        {
            if(roll_chance_i(50))
                me->SummonCreature(NPC_ASTRAL_SPARK, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(),
                        TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
            else
                me->CastSpell(me, SPELL_FIST_OF_STONE, false);
            SkillTimer = urand(15000, 30000);
        }
        

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_arcane_protector(Creature *_Creature)
{
    return new mob_arcane_protectorAI(_Creature);
}

#define SPELL_WARP_BREACH_AOE       29919
#define SPELL_WARP_BREACH_VISUAL    37079

struct mob_mana_warpAI : public ScriptedAI
{
    mob_mana_warpAI(Creature* c) : ScriptedAI(c) {}

	Timer Exploded;

    void Reset()
    {
        Exploded = 0;
    }
    
    void DamageTaken(Unit* pDone_by, uint32& uiDamage)
    {
        if(me->IsNonMeleeSpellCast(true) && uiDamage > me->GetHealth())
            uiDamage = me->GetHealth() - 1;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

		if (!Exploded.GetInterval() && HealthBelowPct(10))
        {
            me->CastSpell(me, SPELL_WARP_BREACH_AOE, false);
            me->CastSpell(me, SPELL_WARP_BREACH_VISUAL, true);
			Exploded.Reset(1000);
        }

		if (Exploded.Expired(diff))
			me->Kill(me);

        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_mob_mana_warp(Creature *_Creature)
{
    return new mob_mana_warpAI(_Creature);
}

#define SPELL_SEARING_PAIN      29492
#define SPELL_IMMOLATE          29928
#define SPELL_CURSE_OF_AGONY    29930
#define SHADOW_PILLAGER_AGGRO_1 -1200022
#define SHADOW_PILLAGER_AGGRO_2 -1200023
#define SHADOW_PILLAGER_DEATH_1 -1200024
#define SHADOW_PILLAGER_DEATH_2 -1200025

struct mob_shadow_pillagerAI : public ScriptedAI
{
    mob_shadow_pillagerAI(Creature* c) : ScriptedAI(c) {}

    Timer_UnCheked DotTimer;

    void Reset()
    {
        SetAutocast(SPELL_SEARING_PAIN, 2500, true);
        DotTimer.Reset(urand(2000, 6000));
    }

    void EnterCombat(Unit* pWho)
    {
        if(roll_chance_i(75))
            me->Say(RAND(-1200022, -1200023), 0, 0);
    }

    void JustDied(Unit* pKiller)
    {
        if(roll_chance_i(75))
            me->Say(RAND(-1200024, -1200025), 0, 0);
    }

    void AttackStart(Unit *who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
        
        
        if (DotTimer.Expired(diff))
        {
            AddSpellToCast(roll_chance_i(50) ? SPELL_IMMOLATE : SPELL_CURSE_OF_AGONY, CAST_RANDOM);
            DotTimer = urand(2000, 8000);
        } 
        

        CheckCasterNoMovementInRange(diff, 40.0);
        CastNextSpellIfAnyAndReady(diff);
    }    
};

CreatureAI* GetAI_mob_shadow_pillager(Creature *_Creature)
{
    return new mob_shadow_pillagerAI(_Creature);
}


#define SPELL_FIREBOLT    30180

struct mob_homunculusAI : public ScriptedAI
{
    mob_homunculusAI(Creature* c) : ScriptedAI(c) {}

    void Reset()
    {
        SetAutocast(SPELL_FIREBOLT, 2000, true);
    }
    
    void AttackStart(Unit *who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        CheckCasterNoMovementInRange(diff, 40.0);
        CastNextSpellIfAnyAndReady(diff);
    }    
};

CreatureAI* GetAI_mob_homunculus(Creature *_Creature)
{
    return new mob_homunculusAI(_Creature);
}

#define GOSSIP_MENU_KOREN_NEUTRAL	9002
#define GOSSIP_MENU_KOREN_FRIENDLY	9004
#define GOSSIP_MENU_KOREN_EXALTED	9003

#define GOSSIP_TEXT_KOREN_EXALTED	16000

bool GossipHello_npc_koren_vendor(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

	if (player->GetReputationMgr().GetRank(967) >= REP_FRIENDLY && player->GetReputationMgr().GetRank(967) != REP_EXALTED)
	{
		if (player->GetReputationMgr().GetRank(967) >= REP_HONORED)
			player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16000), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

		player->SEND_GOSSIP_MENU(GOSSIP_MENU_KOREN_FRIENDLY, _Creature->GetGUID());
	}	
	else if (player->GetReputationMgr().GetRank(967) == REP_EXALTED)
	{
		if (_Creature->isVendor())
			player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16000), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
		player->SEND_GOSSIP_MENU(GOSSIP_MENU_KOREN_EXALTED, _Creature->GetGUID());
	}
	else
		player->SEND_GOSSIP_MENU(GOSSIP_MENU_KOREN_NEUTRAL, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_koren_vendor(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_TRADE)
		player->SEND_VENDORLIST(_Creature->GetGUID());

    return true;
}

#define SPELL_DEM_SHOUT 29584
#define SPELL_STEALTH_DETECTION 18950

#define PHANTOM_AGGRO_1 -1200029
#define PHANTOM_AGGRO_2 -1200030
#define PHANTOM_AGGRO_3 -1200031

#define PHANTOM_STEALTH_1 -1200032
#define PHANTOM_STEALTH_2 -1200033

#define PHANTOM_DEATH_1 -1200034
#define PHANTOM_DEATH_2 -1200035
#define PHANTOM_DEATH_3 -1200036

struct npc_phantom_valetAI : public ScriptedAI
{
    npc_phantom_valetAI(Creature* c) : ScriptedAI(c) {}

    Timer DemoralizingShoutTimer;

    void Reset()
    {
        DemoralizingShoutTimer.Reset(urand(5000, 8000));
        me->CastSpell(me, SPELL_STEALTH_DETECTION, true);
    }
    
    void EnterCombat(Unit *who)
    {
        if(who->HasStealthAura() || who->HasInvisibilityAura())
            me->Say(RAND(-1200032, -1200033), 0, 0);
        else
            me->Say(RAND(-1200029, -1200030, -1200031), 0, 0);
    }

    void JustDied(Unit *who)
    {
        if(roll_chance_i(75))
            me->Say(RAND(-1200034, -1200035, -1200036), 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(DemoralizingShoutTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_DEM_SHOUT, false);
            DemoralizingShoutTimer = urand(14000, 17000);
        }
        // CheckCasterNoMovementInRange(diff, 40.0);
        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }    
};

CreatureAI* GetAI_npc_phantom_valet(Creature *_Creature)
{
    return new npc_phantom_valetAI(_Creature);
}

void AddSC_karazhan_trash()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_koren_vendor";
    newscript->pGossipHello =  &GossipHello_npc_koren_vendor;
    newscript->pGossipSelect = &GossipSelect_npc_koren_vendor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_phantom_guest";
    newscript->GetAI = &GetAI_mob_phantom_guest;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_spectral_sentry";
    newscript->GetAI = &GetAI_mob_spectral_sentry;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_arcane_protector";
    newscript->GetAI = &GetAI_mob_arcane_protector;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_mana_warp";
    newscript->GetAI = &GetAI_mob_mana_warp;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadow_pillager";
    newscript->GetAI = &GetAI_mob_shadow_pillager;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_homunculus";
    newscript->GetAI = &GetAI_mob_homunculus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_15547";
    newscript->GetAI = &GetAI_mob_spectral_charger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_spectral_stable_hand";
    newscript->GetAI = &GetAI_npc_spectral_stable_hand;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_phantom_valet";
    newscript->GetAI = &GetAI_npc_phantom_valet;
    newscript->RegisterSelf();
}
