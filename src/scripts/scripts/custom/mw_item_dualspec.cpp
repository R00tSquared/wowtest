#include "precompiled.h"
#include "Language.h"
#include "DBCStores.h"

bool can_use_dualspec(Player *player)
{
	time_t base = time(NULL);
	bool timer_pass = false;

    if (player->isGameMaster())
        return true;

    if (player->IsInCombat() || player->InBattleGroundOrArena() || player->IsBeingTeleported() || player->IsTaxiFlying() || player->GetLevel() < 10 || uint32(base - player->m_logintime) < 3)
		return false;

	return true;
}

bool GossipHello_custom_dualspec(Player *player, Item* item, SpellCastTargets const& targets)
{
	if (!can_use_dualspec(player))
	{
		player->CLOSE_GOSSIP_MENU();
		player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_CANT_USE_NOW));
		return true;
	}
    
    player->PlayerTalkClass->ClearMenus();
    for (uint8 i = 0; i < MAX_TALENT_SPECS; ++i)
    {
        std::stringstream specNameString;
        specNameString << player->GetSession()->GetHellgroundString(LANG_SPEC) << " " << "#" << i + 1 << " [" << player->PrintTalentCount(i) << "]";

        if (i == player->GetActiveSpec())
            specNameString << " " << player->GetSession()->GetHellgroundString(LANG_SPEC_ACTIVE);

        player->ADD_GOSSIP_ITEM(5, specNameString.str(), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + (2 + i));
    }

    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SPEC_ABOUT), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(990105, item->GetGUID());
    return true;
}

bool GossipSelect_custom_dualspec(Player* player, Item* item, uint32 uiSender, uint32 action, SpellCastTargets const& targets)
{
	if (!can_use_dualspec(player))
	{
		player->CLOSE_GOSSIP_MENU();
		player->GetSession()->SendNotification(player->GetSession()->GetHellgroundString(LANG_CANT_USE_NOW));
		return true;
	}
    
    player->PlayerTalkClass->ClearMenus();
    int32 spec;

    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(990104, item->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 1:
        GossipHello_custom_dualspec(player, item, targets);
        break;
    // only 6 specs allowed, so it's safe
    case GOSSIP_ACTION_INFO_DEF + 2:
    case GOSSIP_ACTION_INFO_DEF + 3:
    case GOSSIP_ACTION_INFO_DEF + 4:
    case GOSSIP_ACTION_INFO_DEF + 5:
    case GOSSIP_ACTION_INFO_DEF + 6:
    case GOSSIP_ACTION_INFO_DEF + 7:
        //if (player->GetFreeTalentPoints() > 0)
        //{
        //    ChatHandler(player->GetSession()).SendSysMessage(LANG_SPEND_ALL_TAL);
        //    GossipHello_custom_dualspec(player, item, targets);
        //    break;
        //}
        
        spec = action - GOSSIP_ACTION_INFO_DEF - 2;
        ASSERT(!(spec < 0 || spec > MAX_TALENT_SPECS));

        if (player->GetActiveSpec() == spec)
        {
            ChatHandler(player).SendSysMessage(LANG_SAME_SPEC);
            GossipHello_custom_dualspec(player, item, targets);
            break;
        }

        //player->CastCustomSpell(player, 55404, &spec, NULL, NULL, true);
        //player->CastCustomSpell(player, 55404, &spec, NULL, NULL, false);

		// destroying session...
        player->ActivateSpec(spec);
        break;
    default:
        break;
    }

    return true;
}

void AddSC_Item_Dualspec()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "item_dualspec";
    newscript->pItemUse = &GossipHello_custom_dualspec;
    newscript->pGossipSelectItem = &GossipSelect_custom_dualspec;
    newscript->RegisterSelf();
}