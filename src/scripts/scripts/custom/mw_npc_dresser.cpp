#include "ObjectMgr.h"
#include "precompiled.h"

#define PAGE_LIMIT 13

bool Hello_mw_npc_dresser(Player* player, Creature* pCreature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, player->GetSession()->GetHellgroundString(16727), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 100);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, player->GetSession()->GetHellgroundString(16728), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 200);
    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
    return true;
}

bool Select_mw_npc_dresser(Player* player, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    uint32 action = uiAction - GOSSIP_ACTION_INFO_DEF;

    // Try on morphing shirt 
    if (action >= 100 && action < 200)
    {
        auto& shirts = sWorld.entryGroups[ENTRY_GROUP_MORPH_SHIRT];

        uint32 page = action % 100;

        auto it = shirts.begin();
        if (page > 0)
            std::advance(it, page * PAGE_LIMIT);

        uint32 i = 1;
        for (; it != shirts.end(); ++it)
        {
            player->ADD_GOSSIP_ITEM(0, player->GetItemLink(*it, false, ITEM_LINK_SPECIAL_COLORS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + (*it * 100 + page) );

            if (i >= PAGE_LIMIT)
                break;

            ++i;
        }     

        if (it != shirts.end())
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(12001), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + action + 1);

        if (page > 0)
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(12002), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + action - 1);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        return true;
    }
    else if (action >= 200 && action < 300)
    {
        auto& tabards = sWorld.entryGroups[ENTRY_GROUP_AURA_TABARD];

        uint32 page = action % 100;

        auto it = tabards.begin();
        if (page > 0)
            std::advance(it, page * PAGE_LIMIT);

        uint32 i = 1;
        for (; it != tabards.end(); ++it)
        {
            player->ADD_GOSSIP_ITEM(0, player->GetItemLink(*it, false, ITEM_LINK_SPECIAL_COLORS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + (*it * 100 + page));

            if (i >= PAGE_LIMIT)
                break;

            ++i;
        }

        if (it != tabards.end())
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(12001), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + action + 1);

        if (page > 0)
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(12002), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + action - 1);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
        return true;
    }
    else
    {
        int8 type = -1;
        
        uint32 page = action % 100;
        action = (action - page) / 100;

        for (const auto& grp : { ENTRY_GROUP_MORPH_SHIRT, ENTRY_GROUP_AURA_TABARD })
        {
            if (type != -1)
                break;

            for (auto& item : sWorld.entryGroups[grp])
            {
                if (item == action)
                {
                    type = grp;
                    break;
                }
            }
        }

        if (type == -1)
            return true;

        ItemPrototype const* proto = ObjectMgr::GetItemPrototype(action);
        if (proto->Spells[0].SpellId == 0)
            return true;

        // need to optimize...
        auto& spells = sWorld.entryGroups[type == ENTRY_GROUP_MORPH_SHIRT ? ENTRY_GROUP_MORPH_SHIRT_AURAS : ENTRY_GROUP_AURA_TABARD_AURAS];
        for (auto& spell : spells)
        {
            player->RemoveAurasDueToSpell(spell);
        }

        player->AddAura(proto->Spells[0].SpellId, player);
        if (Aura* a = player->GetAura(proto->Spells[0].SpellId, 0))
        {
            a->SetAuraDuration(10000);
            a->UpdateAuraDuration();
            a->SetPassive(false);
        }

        Select_mw_npc_dresser(player, pCreature, 0, GOSSIP_ACTION_INFO_DEF + (type == ENTRY_GROUP_MORPH_SHIRT ? 100 : 200) + page);
        return true;
    }

    Hello_mw_npc_dresser(player, pCreature);
    return true;
}

void AddSC_mw_npc_dresser()
{
    Script* newscript;
    newscript = new Script;
    newscript->Name = "mw_npc_dresser";
    newscript->pGossipHello = &Hello_mw_npc_dresser;
    newscript->pGossipSelect = &Select_mw_npc_dresser;
    newscript->RegisterSelf();
}