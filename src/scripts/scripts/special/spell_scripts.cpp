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

#include "precompiled.h"
#include "Spell.h"

bool Spell_intimidating_shout_5246_set_target(Unit* pCaster, std::list<Unit*> &unitList, SpellCastTargets const& targets, SpellEntry const *pSpell, uint32 effect_index)
{
    if (effect_index == 0)
        return true;

    if (unitList.empty())
        return true;

    // remove current target from AOE Fear, AOE Speed aura our target gets stun effect provided by 1st effect
    unitList.remove(targets.getUnitTarget());
    return true;
}

bool Spell_intimidating_shout_5246_effect(Unit *pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex)
{
    // handle only trigger effect -> the main target should not have diminishing from the spell
    if (effectIndex != 0)
        return false;

    // removing one of the diminishing levels, cause first spell - 5246 - always increases diminishing return level, doubling the effect!
    pUnit->DecrDiminishing(DIMINISHING_FEAR);

    // returning true will prevent from processing effect by core, but we need the effect -> so return false
    return false;
}

// Warrior: Deep Wounds dummyeffect implementation: 12162, 12850, 12868
bool Spell_deep_wounds(Unit *pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex)
{
    // handle only dummy efect
    if (pSpell->Effect[effectIndex] != SPELL_EFFECT_DUMMY)
        return false;

    if (!pUnit || pCaster->IsFriendlyTo(pUnit))
        return true;

    Unit* pTarget = pUnit;

    float damage;
    if (pCaster->haveOffhandWeapon() && pCaster->getAttackTimer(BASE_ATTACK) > pCaster->getAttackTimer(OFF_ATTACK))
        damage = (pCaster->GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE) + pCaster->GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE))/2;
    else
        damage = (pCaster->GetFloatValue(UNIT_FIELD_MINDAMAGE) + pCaster->GetFloatValue(UNIT_FIELD_MAXDAMAGE))/2;

    switch (pSpell->Id)
    {
        case 12162: damage *= 0.2f; break;
        case 12850: damage *= 0.4f; break;
        case 12868: damage *= 0.6f; break;
        default:
            // not handled spell assigned
            return false;
    };

    int32 deepWoundsDotBasePoints0 = int32(damage / 4);

    if (Aura *deepWounds = pUnit->GetAuraByCasterSpell(12721, pCaster->GetGUID()))
    {
        deepWounds->GetModifier()->m_amount = deepWoundsDotBasePoints0;
        deepWounds->SetAuraDuration(deepWounds->GetAuraMaxDuration());
        deepWounds->UpdateAuraDuration();

        Aura *bloodFrenzy = pUnit->GetAuraByCasterSpell(30070, pCaster->GetGUID());
        bloodFrenzy ? bloodFrenzy = bloodFrenzy/* do nothing */: bloodFrenzy = pUnit->GetAuraByCasterSpell(30069, pCaster->GetGUID());

        if (bloodFrenzy)
        {
            bloodFrenzy->SetAuraDuration(deepWounds->GetAuraMaxDuration());
            bloodFrenzy->UpdateAuraDuration();
        }
        return true;
    }

    pCaster->CastCustomSpell(pTarget, 12721, &deepWoundsDotBasePoints0, NULL, NULL, true, NULL);
    // we handled our effect, returning true will prevent from processing effect by core :]
    return true;
}

bool Spell_seed_of_corruption_proc(Unit* pCaster, std::list<Unit*> &unitList, SpellCastTargets const& targets, SpellEntry const *pSpell, uint32 effect_index)
{
    if (effect_index != 0)
        return true;

    if (unitList.empty())
        return true;

    unitList.remove(targets.getUnitTarget());
    return true;
}

bool Spell_arcane_torrent(Unit* caster, std::list<Unit*> &, SpellCastTargets const&, SpellEntry const *spellInfo, uint32 effectIndex)
{
    if (effectIndex != 0)
        return true;

    switch (spellInfo->Id)
    {
        case 28730:                                 // Arcane Torrent (Mana)
        case 33390:                                 // Arcane Torrent (mana Wretched Devourer)
        {
            Aura* dummy = caster->GetDummyAura(28734);
            if (dummy)
            {
                int32 bp = (2.17*caster->GetLevel() + 9.136) * dummy->GetStackAmount();
                caster->CastCustomSpell(caster, 28733, &bp, NULL, NULL, true);
                caster->RemoveAurasDueToSpell(28734);
            }
            break;
        }

        // Arcane Torrent (Energy)
        case 25046:
        {
            // Search Mana Tap auras on caster
            Aura* dummy = caster->GetDummyAura(28734);
            if (dummy)
            {
                int32 bp = dummy->GetStackAmount() * 10;
                caster->CastCustomSpell(caster, 25048, &bp, NULL, NULL, true);
                caster->RemoveAurasDueToSpell(28734);
            }
            break;
        }
    }
    return true;
}

bool Spell_strong_fetish(Unit *caster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex)
{
    if (caster->GetTypeId() != TYPEID_PLAYER)
        return true;

    if (Player* player = caster->ToPlayer())
    {
        if (player->GetQuestStatus(10544) == QUEST_STATUS_INCOMPLETE)
        {
            switch (player->GetAreaId())
            {
                case 3773:
                    if(Creature* sum = player->SummonCreature(21446, player->GetPositionX()+(rand()%4), player->GetPositionY()+(rand()%4), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
                        sum->AI()->AttackStart(player);
                    return true;
                    break;
                case 3776:
                    if(Creature* sum = player->SummonCreature(21452, player->GetPositionX()+(rand()%4), player->GetPositionY()+(rand()%4), player->GetPositionZ(), player->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
                        sum->AI()->AttackStart(player);
                    return true;
                    break;
            }
        }
        return false;
    }

    return true;
}

bool spell_34830_34857_effect(Unit *pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex)
{
    if (pCaster && !pCaster->HasUnitState(UNIT_STAT_CANNOT_TURN) && !pCaster->IsInCombat())
    {
        if(pSpell->Id == 34830)
            pCaster->NearTeleportTo(pCaster->GetPositionX(), pCaster->GetPositionY(), pCaster->GetPositionZ(), pCaster->GetOrientationTo(4199.7, 1766.39));
        else
            pCaster->NearTeleportTo(pCaster->GetPositionX(), pCaster->GetPositionY(), pCaster->GetPositionZ(), pCaster->GetOrientationTo(3923.06, 3873.36));
    }
    return true;
}

// Salvage Wreckage
bool spell_42287_effect(Unit *pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex)
{
    if (effectIndex != 1)
        return false;

    if (pCaster->GetTypeId() != TYPEID_PLAYER)
        return false;

    if (((Player*)pCaster)->GetQuestStatus(11140) == QUEST_STATUS_INCOMPLETE)
    {
        GameObject* ok = NULL;
        Hellground::GameObjectFocusCheck go_check(pCaster, pSpell->RequiresSpellFocus);
        Hellground::ObjectSearcher<GameObject, Hellground::GameObjectFocusCheck> checker(ok,go_check);
        Cell::VisitGridObjects(pCaster, checker, 10.0);

        if(ok)
            ok->SetLootState(GO_JUST_DEACTIVATED);

        pCaster->CastSpell(pCaster, urand(0, 1) ? 42289 : 42288, false);
    }
    return true;
}

// Expedition Flare
bool spell_32027_effect(Unit *pCaster, Unit* pUnit, Item* pItem, GameObject* pGameObject, SpellEntry const *pSpell, uint32 effectIndex)
{
    if (pCaster->GetTypeId() != TYPEID_PLAYER)
        return false;

    pCaster->CastSpell(pCaster, urand(0, 1) ? 32030 : 32029, false);
    return true;
}

void AddSC_spell_scripts()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "spell_intimidating_shout";
    newscript->pSpellTargetMap = &Spell_intimidating_shout_5246_set_target;
    newscript->pSpellHandleEffect = &Spell_intimidating_shout_5246_effect;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "spell_deep_wounds";
    newscript->pSpellHandleEffect = &Spell_deep_wounds;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "spell_seed_of_corruption_proc";
    newscript->pSpellTargetMap = &Spell_seed_of_corruption_proc;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "spell_arcane_torrent";
    newscript->pSpellTargetMap = &Spell_arcane_torrent;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "strong_fetish";
    newscript->pSpellHandleEffect = &Spell_strong_fetish;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "spell_34830_34857";
    newscript->pSpellHandleEffect = &spell_34830_34857_effect;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "spell_42287";
    newscript->pSpellHandleEffect = &spell_42287_effect;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "spell_32027";
    newscript->pSpellHandleEffect = &spell_32027_effect;
    newscript->RegisterSelf();
}
