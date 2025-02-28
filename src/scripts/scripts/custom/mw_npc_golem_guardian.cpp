#include "precompiled.h"
#include "PetAI.h"
#include "SpellMgr.h"
#include "Common.h"
#include "ObjectMgr.h"
#include "Player.h"

#define SPELL_PYROBLAST 55421 //33975 
#define SPELL_ARCANE_EXPLOSION 55422 //15253
#define SPELL_CHAIN_LIGHTNING 55423 //39945

#define SPELL_HEAL 55424 //29564
#define SPELL_MANA 55425 //17531
#define SPELL_ARCANE_BOLT 55426 // 31445 single target 2k

#define SPELL_DIVINE_SHIELD 40733

uint32 damage_spells[4][2] =
{
    {0, SPELL_ARCANE_BOLT},
    {1, SPELL_PYROBLAST},
    {2, SPELL_ARCANE_EXPLOSION},
    {3, SPELL_CHAIN_LIGHTNING},
};

uint32_t getNextSpell(uint32_t spell)
{
    switch (spell)
    {
    case SPELL_ARCANE_BOLT:
        return SPELL_PYROBLAST;
    case SPELL_PYROBLAST:
        return SPELL_ARCANE_EXPLOSION;
    case SPELL_ARCANE_EXPLOSION:
        return SPELL_CHAIN_LIGHTNING;
    case SPELL_CHAIN_LIGHTNING:
        return SPELL_ARCANE_BOLT;
    default:
        return spell;
    }
}

enum GolemActions
{
    GOLEM_SPELL_ARCANE_BOLT,
    GOLEM_SPELL_PYROBLAST,
    GOLEM_SPELL_FIRE_RAIN,
    GOLEM_SPELL_CHAIN_LIGHTNING,
    GOLEM_HEAL,
    GOLEM_ACTION_MAX,
};

struct npc_golem_guardian_AI : public PetAI
{
    npc_golem_guardian_AI(Creature* c) : PetAI(c) 
    {
        selected_damage_spell = SPELL_ARCANE_BOLT;
        me->CastSpell(me, 40733, true); // Divine Shield
        me->SetReactState(REACT_DEFENSIVE);
    }

    uint32 selected_damage_spell;

    void EnterCombat(Unit* who)
    {
        me->addUnitState(UNIT_STAT_IGNORE_ATTACKERS);
        me->SetUInt32Value(UNIT_NPC_FLAGS, 0);
    }

    void OnCombatStop()
    {
        me->SetUInt32Value(UNIT_NPC_FLAGS, 1);
    }

    void SetDamageSpell(uint32 spell)
    {
        // disable all except one 
        for (auto v : damage_spells)
        {
            if (v[1] == spell)
            {
                selected_damage_spell = spell;
                return;
            }
        }

        selected_damage_spell = SPELL_ARCANE_BOLT;
    }

    void Follow()
    {
        if (!me->HasUnitState(UNIT_STAT_FOLLOW | UNIT_STAT_CASTING_NOT_MOVE))
            me->GetMotionMaster()->MoveFollow(m_owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }

    bool DoHealing(Unit* target)
    {
        Pet* pet = target->GetPet();

        uint32 spell = 0;

        if (target->IsInCombat())
        {
            if (target->GetHealthPercent() < 50)
            {
                spell = SPELL_HEAL;
            }
            else if (pet && pet->isAlive() && pet->GetHealthPercent() < 50)
            {
                spell = SPELL_HEAL;
                target = pet;
            }
            else if (target->getPowerType() == POWER_MANA && target->GetPower(POWER_MANA) < float(target->GetMaxPower(POWER_MANA) * 0.2))
            {
                spell = SPELL_MANA;
            }
        }
        else
        {
            if (target->GetHealthPercent() < 100)
            {
                spell = SPELL_HEAL;
            }
            else if (pet && pet->isAlive() && pet->GetHealthPercent() < 100)
            {
                spell = SPELL_HEAL;
                target = pet;
            }
            else if (target->getPowerType() == POWER_MANA && target->GetPower(POWER_MANA) < target->GetMaxPower(POWER_MANA))
            {
                spell = SPELL_MANA;
            }

            // get closer to heal
            if (spell && !me->IsWithinDistInMap(target, 10.0f))
            {
                Follow();
                return true;
            }
        }

        if (!spell)
            return false;

        if (!me->HasUnitState(UNIT_STAT_CASTING))
        {
            AddSpellForAutocast(spell, target);
            
            if (!AutocastPreparedSpells())
            {
                Follow();
                return true;
            }      
        }

        return true;
    }

    void UpdateAI(const uint32 diff)
    {
        m_owner = me->GetCharmerOrOwner();
        
        if (!m_owner || !m_owner->isAlive() || !me->isAlive() || !me->GetCharmInfo())
        {
            me->ForcedDespawn();
            return;
        }

        if (me->GetReactState() == REACT_PASSIVE)
            return;

        if (DoHealing(m_owner))
            return;

        Unit* target = nullptr;
        if (target = me->GetVictim())
        {
            if (_needToStop())
            {
                _stopAttack();
                return;
            }
        }
        else
        {
            if (m_owner->IsInCombat())
            {
                if (m_owner->GetVictim() && !targetHasInterruptableAura(m_owner->GetVictim()))
                    target = m_owner->GetVictim();
                else if (!m_owner->GetAttackers().empty() && !targetHasInterruptableAura(m_owner->GetAttackerForHelper()))
                    target = m_owner->GetAttackerForHelper();
            }

            if (target)
                AttackStart(target);
            else 
                Follow();
        }

        if (!target)
            return;

        if (!target->GetVictim() && me->IsInCombat())
        {
            _stopAttack();
            return;
        }

        //bool can_find_next_target = false;

        //Unit::AttackerSet const& attackers = m_owner->GetAttackers();
        //if (!attackers.empty())
        //{
        //    for (Unit::AttackerSet::const_iterator itr = attackers.begin(); itr != attackers.end(); ++itr)
        //    {
        //        Unit* attacker = (*itr)->ToUnit();
        //        if (!attacker || attacker == target)
        //            continue;

        //        AttackStart(attacker);
        //        can_find_next_target = true;
        //    }
        //}

        //if (!can_find_next_target)
        //{
        //    if (me->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW) && !me->HasUnitState(UNIT_STAT_FOLLOW))
        //        me->GetMotionMaster()->MoveFollow(m_owner, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

        //    me->CombatStop();
        //}

        if (!me->HasUnitState(UNIT_STAT_CASTING))
        {
            AddSpellForAutocast(selected_damage_spell, target);
            AutocastPreparedSpells();
        }
    }
};

bool npc_golem_guardian_Hello(Player* player, Creature* creature)
{
    npc_golem_guardian_AI* golem = dynamic_cast<npc_golem_guardian_AI*>(creature->AI());
    if (!golem)
        return true;

    uint32 selected_spell = golem->selected_damage_spell;
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(selected_spell);
    if (!spellInfo)
        return true;

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, player->GetSession()->PGetHellgroundString(16681, spellInfo->SpellName[0]), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    const char* msg = creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE ? player->GetSession()->GetHellgroundString(16682) : player->GetSession()->GetHellgroundString(16677);

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, msg, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(16680), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
    player->SEND_GOSSIP_MENU(990135, creature->GetObjectGuid());
    return true;
}

bool npc_golem_guardian_Gossip(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
{
    npc_golem_guardian_AI* golem = dynamic_cast<npc_golem_guardian_AI*>(creature->AI());
    if (!golem)
        return true;

    switch (uiAction)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
    {
        uint32 next_spell = getNextSpell(golem->selected_damage_spell);
        golem->SetDamageSpell(next_spell);
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 2:
    {
        if (creature->GetReactState() == REACT_PASSIVE)
        {
            creature->MonsterSay(player->GetSession()->GetHellgroundString(16679), LANG_UNIVERSAL, player->GetGUID());
            creature->SetReactState(REACT_DEFENSIVE);
            creature->GetMotionMaster()->MoveFollow(creature->GetOwner(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE / 1.5f);
            creature->RemoveAurasDueToSpell(9454);
        }
        else
        {
            creature->GetMotionMaster()->MoveIdle();
            creature->SetReactState(REACT_PASSIVE);
            creature->MonsterSay(player->GetSession()->GetHellgroundString(16678), LANG_UNIVERSAL, player->GetGUID());
            creature->AddAura(9454);
        }
        break;
    }
    case GOSSIP_ACTION_INFO_DEF + 3:
    {
        creature->ForcedDespawn();
        break;
    }
    }

    player->PlayerTalkClass->ClearMenus();
    npc_golem_guardian_Hello(player, creature);
    return true;
}

CreatureAI* GetAI_npc_golem_guardian(Creature* _Creature)
{
    return new npc_golem_guardian_AI(_Creature);
}

void AddSC_npc_golem_guardian()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_golem_guardian";
    newscript->GetAI = &GetAI_npc_golem_guardian;
    newscript->pGossipHello = &npc_golem_guardian_Hello;
    newscript->pGossipSelect = &npc_golem_guardian_Gossip;
    newscript->RegisterSelf();
}
