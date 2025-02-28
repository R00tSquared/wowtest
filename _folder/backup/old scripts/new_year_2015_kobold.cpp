// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "precompiled.h"
#include "Language.h"

struct npc_new_year_2015_AI : public ScriptedAI
{
    npc_new_year_2015_AI(Creature *c) : ScriptedAI(c) {}

    uint32 stealthRestoreCheck;

    void Reset()
    {
        stealthRestoreCheck = me->GetEntry() == 690002 ? 5000 : 0;
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == 42805 && !me->HasAura(55159) && !me->HasAura(42805) && me->CanFreeMove()) // Dirty Trick new year spell
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

 CreatureAI* GetAI_npc_new_year_2015_AI(Creature *_Creature)
{
    return new npc_new_year_2015_AI (_Creature);
}

 void AddSC_DeathSide_new_year_2015_kobold()
 {
     Script *newscript;

     newscript = new Script;
     newscript->Name = "new_year_2015_kobold";
     newscript->pGossipHello           = &DeathSide_new_year_2015_kobold;
     newscript->GetAI = &GetAI_npc_new_year_2015_AI;
     newscript->RegisterSelf();
 }