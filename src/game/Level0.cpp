// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008 TrinityCore <http://www.trinitycore.org/>
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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "AccountMgr.h"
#include "SystemConfig.h"
#include "revision.h"
#include "Util.h"
#include "TicketMgr.h"
#include "GameEvent.h"
#include "BattleGroundMgr.h"
#include "ArenaTeam.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "Guild.h"
#include "Transmogrification.h"
#include "GossipDef.h"
#include "Util.h"
#include "InstanceSaveMgr.h"
#include "ScriptMgr.h"

bool ChatHandler::HandleAccountBonesHideCommand(const char* args)
{
    if (m_session->IsAccountFlagged(ACC_HIDE_BONES))
    {
        m_session->RemoveAccountFlag(ACC_HIDE_BONES);

        SendSysMessage("Client will show bones for this account now.");
    }
    else
    {
        m_session->AddAccountFlag(ACC_HIDE_BONES);

        SendSysMessage("Client won't show bones for this account now.");
    }

    return true;
}

bool ChatHandler::HandleAccountGuildAnnToggleCommand(const char* args)
{
    if (m_session->IsAccountFlagged(ACC_DISABLED_GANN))
    {
        m_session->RemoveAccountFlag(ACC_DISABLED_GANN);

        SendSysMessage(LANG_GUILD_ANN_ENABLED);
    }
    else
    {
        m_session->AddAccountFlag(ACC_DISABLED_GANN);

        SendSysMessage(LANG_GUILD_ANN_DISABLED);
    }

    return true;
}

bool ChatHandler::HandleAccountBattleGroundAnnCommand(const char* args)
{
    if (m_session->IsAccountFlagged(ACC_DISABLED_BGANN))
    {
        m_session->RemoveAccountFlag(ACC_DISABLED_BGANN);
        SendSysMessage(LANG_BGANN_ENABLED);
    }
    else
    {
        m_session->AddAccountFlag(ACC_DISABLED_BGANN);
        SendSysMessage(LANG_BGANN_DISABLED);
    }

    return true;
}

bool ChatHandler::HandleLootAnnounces(const char* args)
{
    if (m_session->IsAccountFlagged(ACC_DISABLE_LOOT_ANNOUNCES))
    {
        m_session->RemoveAccountFlag(ACC_DISABLE_LOOT_ANNOUNCES);
        SendSysMessage(16670);
    }
    else
    {
        m_session->AddAccountFlag(ACC_DISABLE_LOOT_ANNOUNCES);
        SendSysMessage(16671);
    }

    return true;
}

//bool ChatHandler::HandleAccountAnnounceBroadcastCommand(const char* args)
//{
//    if (m_session->IsAccountFlagged(ACC_DISABLED_BROADCAST))
//    {
//        m_session->RemoveAccountFlag(ACC_DISABLED_BROADCAST);
//        SendSysMessage("AutoBroadcast announces have been enabled for this account.");
//    }
//    else
//    {
//        m_session->AddAccountFlag(ACC_DISABLED_BROADCAST);
//        SendSysMessage("AutoBroadcast announces have been disabled for this account.");
//    }
//
//    return true;
//}

bool ChatHandler::HandleHelpCommand(const char* args)
{
    char* cmd = strtok((char*)args, " ");
    if (!cmd)
    {
        ShowHelpForCommand(getCommandTable(), "help");
        ShowHelpForCommand(getCommandTable(), "");
    }
    else
    {
        if (!ShowHelpForCommand(getCommandTable(), cmd))
            SendSysMessage(LANG_NO_HELP_CMD);
    }

    return true;
}

bool ChatHandler::HandleCommandsCommand(const char* args)
{
    ShowHelpForCommand(getCommandTable(), "");
    return true;
}

bool ChatHandler::HandleAccountCommand(const char* /*args*/)
{
    uint64 permissions = m_session->GetPermissions();
    PSendSysMessage(LANG_ACCOUNT_LEVEL, permissions);
    return true;
}

bool ChatHandler::HandleStartCommand(const char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    if (chr->IsTaxiFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    if (chr->IsInCombat())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    // cast spell Stuck
    chr->CastSpell(chr,7355,false);
    return true;
}

bool ChatHandler::HandleAccountWeatherCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_USE_BOL);
        return true;
    }

    std::string argstr = (char*)args;
    if (argstr == "off")
        m_session->AddOpcodeDisableFlag(OPC_DISABLE_WEATHER);
    else if (argstr == "on")
        m_session->RemoveOpcodeDisableFlag(OPC_DISABLE_WEATHER);
    else
    {
        SendSysMessage(LANG_USE_BOL);
        return true;
    }

    PSendSysMessage(LANG_SET_WEATHER, argstr.c_str());
    return true;
}

bool ChatHandler::HandleServerInfoCommand(const char* /*args*/)
{       
    Player* plr = m_session->GetPlayer();
    if (!plr)
        return true;

    if (plr->GetDummyAura(54830))
    {
        SendSysMessage(GetHellgroundString(LANG_NOT_MORE_OFTEN_THAN_10_SEC));
        return true;
    }
    else
        plr->CastSpell(plr, 54830, true);

	// realm
	PSendSysMessage(16574, sWorld.GetMotd());

    // online
    PSendSysMessage(16609, sWorld.GetActiveSessionCount(), sWorld.online.weekly_max);

    if (sWorld.isEasyRealm())
    {
        std::string guild_house_owner = "-";
        if (sWorld.m_guild_house_owner)
        {
            if (Guild* go = getGuildMgr()->GetGuildById(sWorld.m_guild_house_owner))
                guild_house_owner = go->GetName();
        }

        PSendSysMessage(15552, guild_house_owner.c_str());
    }     

    // version
    std::string rev_id = REVISION;
    rev_id = rev_id.substr(0, 7);
    PSendSysMessage(LANG_SERVER_REV, rev_id.c_str(), REVISION_DATE);

    // server time
    time_t t = time(NULL);
    tm* aTm = localtime(&t);

    char exp_chr[32];
    snprintf(exp_chr, 32, "%04d-%02d-%02d %02d:%02d:%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);

    PSendSysMessage(LANG_SERVER_TIME, exp_chr);

    // uptime
    std::string str = m_session->secondsToTimeString(sWorld.GetUptime());
    PSendSysMessage(LANG_SERVER_WORKS, str.c_str());

	// current phase
	if (!sWorld.isEasyRealm())
	{
		uint8 phase = 0;
		if (isGameEventActive(204))
			phase = 5;
		else if (isGameEventActive(203))
			phase = 4;
		else if (isGameEventActive(202))
			phase = 3;
		else if (isGameEventActive(201))
			phase = 2;
		else if (isGameEventActive(200))
			phase = 1;

		PSendSysMessage(15670, phase);
	}

    // website
    SendSysMessage(15237);

    // discord
    SendSysMessage(15239);

    if (sWorld.IsShutdowning())
    {
        SendSysMessage("");
        PSendSysMessage(LANG_SERVERINFO_0, (sWorld.GetShutdownMask() & SHUTDOWN_MASK_RESTART ? GetHellgroundString(LANG_RESTART) : GetHellgroundString(LANG_SHUTDOWN)), m_session->secondsToTimeString(sWorld.GetShutdownTimer()).c_str());
        PSendSysMessage("%s: %s.", GetHellgroundString(LANG_REASON), sWorld.GetShutdownReason());
    }

    if (BattleGround* bg = plr->GetBattleGround())
    {
        if (bg->isBattleGround())
        {
            PSendSysMessage("Battleground ID %u, current class count A/H %u/%u, %s",
				bg->GetBgInstanceId(), bg->GetTeamClassMaskCount(TEAM_ALLIANCE, plr->GetBGClassMask()), 
                bg->GetTeamClassMaskCount(TEAM_HORDE, plr->GetBGClassMask()), bg->GetPlayerCountInfo().c_str());
        }
    }

    return true;
}

//bool ChatHandler::HandleServerEventsCommand(const char*)
//{
//    std::string active_events = sGameEventMgr.getActiveEventsString();
//    PSendSysMessage("%s", active_events.c_str());//ChatHandler::FillMessageData(&data, this, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, GetPlayer()->GetGUID(), active_events, NULL);
//    if(sWorld.getConfig(CONFIG_ARENA_DAILY_REQUIREMENT))
//    {
//        PSendSysMessage("Daily Arenas! Get %u AP for winning %u rated arenas",
//            sWorld.getConfig(CONFIG_ARENA_DAILY_AP_REWARD),sWorld.getConfig(CONFIG_ARENA_DAILY_REQUIREMENT));
//    }
//    return true;
//}

//bool ChatHandler::HandleCharInfoCommand(const char* /*args*/)
//{
//    Player* player = m_session->GetPlayer();
//
//    if (!player)
//        return true;
//
//    if (sWorld.getConfig(CONFIG_REALM_TYPE) != 3/*REALM_FUN*/)
//        return true;
//
//    if (player->GetDummyAura(54830))
//    {
//        SendSysMessage(GetHellgroundString(LANG_NOT_MORE_OFTEN_THAN_10_SEC));
//        return true;
//    }
//    else
//        player->CastSpell(player, 54830, true);
//
//    if (getSelectedPlayer() && getSelectedPlayer() != player)
//    {
//        Player* TargetPlayer = getSelectedPlayer();
//        uint32 LegPercent = TargetPlayer->GetLegendaryItemsNumber();
//        uint32 LegTabard = 0;
//        if (Item* pItem = TargetPlayer->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TABARD))
//            if (pItem->GetProto()->Quality == 5 || pItem->GetProto()->Quality == 6)
//                LegTabard += pItem->GetProto()->ItemLevel;
//
//        uint8 ReinforceLevel = 0;
//        bool added;
//        for (uint8 slot = 0; slot < 10; slot++)
//        {
//            if (slot == 1 || slot == 3)
//                continue;
//            added = false;
//            Item *pItem = TargetPlayer->GetItemByPos(255, slot);
//            if (pItem && pItem->GetProto()->Quality == 5 && (pItem->GetProto()->ItemId >= 903180 || pItem->GetProto()->ItemId == 339111))
//            {
//                if (uint16 EnchantId = pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
//                {
//                    for (uint8 i = 0; i < 10 && !added; i++)
//                        for (uint8 j = 0; j < 5 && !added; j++)
//                        {
//                            if (ReinforcementIDs[i][j] == EnchantId)
//                            {
//                                added = true;
//                                ReinforceLevel += j+1;
//                            }
//                        }
//                }
//            }
//        }
//        PSendSysMessage("Уровень легендарной экипировки = %u/100. \nУровень легендарной накидки = %u/150. \nУровень рунных улучшений = %u/40.", LegPercent, LegTabard, ReinforceLevel);
//    }
//    else
//    {
//    // Defense
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_DEFENSE_SKILL) > 0)
//    {
//        uint32 ActualDefense = 350 + uint32(player->GetRatingBonusValue(CR_DEFENSE_SKILL)/(1.0f + player->GetLegendaryItemsNumber(true) * 0.006f));
//        PSendSysMessage("Настоящий уровень защиты = %u", ActualDefense);
//    }
//    {
//        uint32 Socket_Rating_Ratio = 100.0f / (1.0f + 0.006f * player->GetLegendaryItemsNumber(true));
//        // Rage Gain
//        if (player->getPowerType() == POWER_RAGE)
//            PSendSysMessage("Эффективность получения ярости снижена до %u%%", Socket_Rating_Ratio);
//        // Mana Regeneration modifier from Intellect
//        else if (player->getPowerType() == POWER_MANA)
//            PSendSysMessage("Множитель регенерации маны снижен до %u%%", Socket_Rating_Ratio);
//    }
//    // Agility - Dodge
//    {
//    float dodge_ratio;
//    float crit_to_dodge[MAX_CLASSES] = {
//         1.1f,      // Warrior
//         1.0f,      // Paladin
//         1.6f,      // Hunter
//         2.0f,      // Rogue
//         1.0f,      // Priest
//         1.0f,      // DK?
//         1.0f,      // Shaman
//         1.0f,      // Mage
//         1.0f,      // Warlock
//         0.0f,      // ??
//         1.7f       // Druid
//    };
//    GtChanceToMeleeCritEntry  const *dodgeRatio = sGtChanceToMeleeCritStore.LookupEntry((player->GetClass()-1)*GT_MAX_LEVEL + 69);
//    if (dodgeRatio == NULL || player->GetClass() > MAX_CLASSES)
//        dodge_ratio =  0.0f;
//    else
//        dodge_ratio = (1.0f + player->GetLegendaryItemsNumber(true) * 0.006f) / crit_to_dodge[player->GetClass()-1] / dodgeRatio->ratio / 100.0f;
//    PSendSysMessage("%f ловкости = 1%% уклона", dodge_ratio);
//    }
//    // Agility - Crit
//    {
//    float crit_ratio_agility;
//    GtChanceToMeleeCritEntry     const *critRatio = sGtChanceToMeleeCritStore.LookupEntry((player->GetClass()-1)*GT_MAX_LEVEL + 69);
//    if (critRatio == NULL)
//        crit_ratio_agility = 0.0f;
//    else
//        crit_ratio_agility = (1.0f + player->GetLegendaryItemsNumber(true) * 0.006f) / critRatio->ratio / 100.0f ;
//    PSendSysMessage("%f ловкости = 1%% крит. удара", crit_ratio_agility);
//    }
//    // Intellect spell crit
//    if (player->getPowerType() == POWER_MANA)
//    {
//    float spell_crit_intellect_ratio;
//    GtChanceToSpellCritEntry     const *spell_intellect_critRatio = sGtChanceToSpellCritStore.LookupEntry((player->GetClass()-1)*GT_MAX_LEVEL + 69);
//    if (spell_intellect_critRatio == NULL)
//        spell_crit_intellect_ratio = 0.0f;
//    else
//        spell_crit_intellect_ratio = (1.0f + player->GetLegendaryItemsNumber(true) * 0.006f) / spell_intellect_critRatio->ratio / 100.0f ;
//    PSendSysMessage("%f интеллекта = 1%% крит. удара заклинаниями", spell_crit_intellect_ratio);
//    }
//    // Crit rating
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_MELEE) > 1)
//    {
//    float phys_crit_Ratio;
//    GtCombatRatingsEntry const *phys_crit_Rating = sGtCombatRatingsStore.LookupEntry(CR_CRIT_MELEE*GT_MAX_LEVEL+69);
//    if (phys_crit_Rating == NULL)
//        phys_crit_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//    else
//        phys_crit_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * phys_crit_Rating->ratio;
//    PSendSysMessage("%f рейтинга крит. удара = 1%% крит. удара", phys_crit_Ratio);
//    }
//    // Spell crit rating
//    if (player->getPowerType() == POWER_MANA && player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_SPELL) > 1)
//    {
//    float Spell_Crit_Ratio;
//    GtCombatRatingsEntry const *Spell_Crit_Rating = sGtCombatRatingsStore.LookupEntry(CR_CRIT_SPELL*GT_MAX_LEVEL+69);
//    if (Spell_Crit_Rating == NULL)
//        Spell_Crit_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//    else
//        Spell_Crit_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Spell_Crit_Rating->ratio;
//    PSendSysMessage("%f рейтинга крит. удара заклинаниями = 1%% крит. удара заклинаниями", Spell_Crit_Ratio);
//    }
//
//    // Haste rating
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HASTE_MELEE) > 1)
//    {
//        float Haste_Ratio;
//        GtCombatRatingsEntry const *Haste_Rating = sGtCombatRatingsStore.LookupEntry(CR_HASTE_MELEE*GT_MAX_LEVEL+69);
//        if (Haste_Rating == NULL)
//            Haste_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//        else
//            Haste_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Haste_Rating->ratio;
//        PSendSysMessage("%f рейтинга скорости = 1%% прибавки к скорости", Haste_Ratio);
//    }
//
//    // Spell Haste rating
//    if (player->getPowerType() == POWER_MANA && player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HASTE_SPELL) > 1)
//    {
//        float Spell_Haste_Ratio;
//        GtCombatRatingsEntry const *Spell_Haste_Rating = sGtCombatRatingsStore.LookupEntry(CR_HASTE_SPELL*GT_MAX_LEVEL+69);
//        if (Spell_Haste_Rating == NULL)
//            Spell_Haste_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//        else
//            Spell_Haste_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Spell_Haste_Rating->ratio;
//        PSendSysMessage("%f рейтинга магической скорости = 1%% прибавки к магической скорости", Spell_Haste_Ratio);
//    }
//
//    // Hit rating
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HIT_MELEE) > 0 || player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HIT_RANGED) > 0)
//    {
//        float Hit_Ratio;
//        GtCombatRatingsEntry const *Hit_Rating = sGtCombatRatingsStore.LookupEntry(CR_HIT_MELEE*GT_MAX_LEVEL+69);
//        if (Hit_Rating == NULL)
//            Hit_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//        else
//            Hit_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Hit_Rating->ratio;
//        PSendSysMessage("%f рейтинга меткости = 1%% прибавки к шансу попадания", Hit_Ratio);
//    }
//
//    // Spell Hit rating
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_HIT_SPELL) > 0)
//    {
//        float Spell_Hit_Ratio;
//        GtCombatRatingsEntry const *Spell_Hit_Rating = sGtCombatRatingsStore.LookupEntry(CR_HIT_SPELL*GT_MAX_LEVEL+69);
//        if (Spell_Hit_Rating == NULL)
//            Spell_Hit_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//        else
//            Spell_Hit_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Spell_Hit_Rating->ratio;
//        PSendSysMessage("%f рейтинга меткости магиями = 1%% прибавки к шансу попадания магями", Spell_Hit_Ratio);
//    }
//
//    // Parry Rating
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_PARRY))
//    {
//    float Parry_Ratio;
//    GtCombatRatingsEntry const *Parry_Rating = sGtCombatRatingsStore.LookupEntry(CR_PARRY*GT_MAX_LEVEL+69);
//    if (Parry_Rating == NULL)
//        Parry_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//    else
//        Parry_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Parry_Rating->ratio;
//    PSendSysMessage("%f рейтинга парирования = 1%% парирования", Parry_Ratio);
//    }
//    // Dodge Rating
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_DODGE))
//    {
//    float Dodge_Ratio;
//    GtCombatRatingsEntry const *Dodge_Rating = sGtCombatRatingsStore.LookupEntry(CR_DODGE*GT_MAX_LEVEL+69);
//    if (Dodge_Rating == NULL)
//        Dodge_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//    else
//        Dodge_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Dodge_Rating->ratio;
//    PSendSysMessage("%f рейтинга уклонения = 1%% уклонения", Dodge_Ratio);
//    }
//    // Resilience
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_TAKEN_SPELL))
//    {
//    float Resilience_Ratio;
//    GtCombatRatingsEntry const *Resilience_Rating = sGtCombatRatingsStore.LookupEntry(CR_CRIT_TAKEN_SPELL*GT_MAX_LEVEL+69);
//    if (Resilience_Rating == NULL)
//        Resilience_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//    else
//        Resilience_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Resilience_Rating->ratio; // *2 for crits
//    PSendSysMessage("%f рейтинга устойчивости = 1%% устойчивости к DoT заклинаниям, 2%% уменьшения урона крит. ударов", Resilience_Ratio);
//    }
//    // Block Rating
//    if (player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_BLOCK))
//    {
//    float Block_Ratio;
//    GtCombatRatingsEntry const *Block_Rating = sGtCombatRatingsStore.LookupEntry(CR_BLOCK*GT_MAX_LEVEL+69);
//    if (Block_Rating == NULL)
//        Block_Ratio = 0.0f;                                        // By default use minimum coefficient (not must be called)
//    else
//        Block_Ratio = (1.0f + 0.006f * player->GetLegendaryItemsNumber(true)) * Block_Rating->ratio;
//    PSendSysMessage("%f рейтинга блокировки = 1%% блока", Block_Ratio);
//    }
//    // ITEMLEVELS
//    {
//    uint8 ReinforceLevel = 0;
//    bool added;
//    for (uint8 slot = 0; slot < 10; slot++)
//    {
//        if (slot == 1 || slot == 3)
//            continue;
//        added = false;
//        Item *pItem = player->GetItemByPos(255, slot);
//        if (pItem && pItem->GetProto()->Quality == 5 && (pItem->GetProto()->ItemId >= 903180 || pItem->GetProto()->ItemId == 339111))
//        {
//            if (uint16 EnchantId = pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
//            {
//                for (uint8 i = 0; i < 10 && !added; i++)
//                    for (uint8 j = 0; j < 5 && !added; j++)
//                    {
//                        if (ReinforcementIDs[i][j] == EnchantId)
//                        {
//                            added = true;
//                            ReinforceLevel += j+1;
//                        }
//                    }
//            }
//        }
//    }
//    uint32 LegPercent = player->GetLegendaryItemsNumber();
//    uint32 LegTabard = 0;
//    if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TABARD))
//        if (pItem->GetProto()->Quality == 5 || pItem->GetProto()->Quality == 6)
//            LegTabard += pItem->GetProto()->ItemLevel;
//        PSendSysMessage("Уровень легендарной экипировки = %u/100. \nУровень легендарной накидки = %u/150. \nУровень рунных улучшений = %u/40.", LegPercent, LegTabard, ReinforceLevel);
//    }
//    }
//    return true;
//}

//bool ChatHandler::HandleGuildPointsInfoCommand(const char* /*args*/)
//{
//    return false; // not needed at all
//    
//    Player* player = m_session->GetPlayer();
//
//    if (!player || !player->GetGuildId())
//        return true;
//
//    if (player->GetDummyAura(54830))
//    {
//        SendSysMessage(GetHellgroundString(LANG_NOT_MORE_OFTEN_THAN_10_SEC));
//        return true;
//    }
//    else
//        player->CastSpell(player, 54830, true);
//    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT name, GuildPoints, totalguildpoints FROM guild WHERE guildid = '%u' LIMIT 1", player->GetGuildId());
//    if (result)
//        PSendSysMessage("Очки гильдии %s: %u (%u).", (*result)[0].GetCppString().c_str(), (*result)[1].GetUInt32(), (*result)[2].GetUInt32());
//    return true;
//}

bool ChatHandler::HandleCoinsInfoCommand(const char* /*args*/)
{
    Player* player = m_session->GetPlayer();

    if (!player)
        return true;

    uint32 accId = m_session->GetAccountId();

    if (!accId)
        return true;

    if (player->GetDummyAura(54830))
    {
        SendSysMessage(GetHellgroundString(LANG_NOT_MORE_OFTEN_THAN_10_SEC));
        return true;
    }
    else
        player->CastSpell(player, 54830, true);

    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT `coins` FROM `account` WHERE `account_id`='%u'", accId);
    if (result)
        PSendSysMessage(LANG_COMMAND_COINS_INFO, (int32)((*result)[0].GetFloat()));
    return true;
}

bool ChatHandler::HandleDismountCommand(const char* /*args*/)
{
    //If player is not mounted, so go out :)
    if (!m_session->GetPlayer()->IsMounted())
    {
        SendSysMessage(LANG_CHAR_NON_MOUNTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (m_session->GetPlayer()->IsTaxiFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->Unmount();
    m_session->GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
    return true;
}

bool ChatHandler::HandleSaveCommand(const char* /*args*/)
{
    Player *player=m_session->GetPlayer();

    // save GM account without delay and output message (testing, etc)
    if (m_session->HasPermissions(PERM_GMT))
    {
        player->SaveToDB();
        SendSysMessage(LANG_PLAYER_SAVED);
        return true;
    }

    // save or plan save after 20 sec (logout delay) if current next save time more this value and _not_ output any messages to prevent cheat planning
    uint32 save_interval = sWorld.getConfig(CONFIG_INTERVAL_SAVE);
    if (save_interval==0 || save_interval > 20*1000 && player->GetSaveTimer() <= save_interval - 20*1000)
        player->SaveToDB();

    return true;
}

//bool ChatHandler::HandlePasswordCommand(const char* args)
//{
//    return false; // not needed at all
//    
//    if (!*args)
//        return false;
//
//    char *old_pass = strtok ((char*)args, " ");
//    char *new_pass = strtok (NULL, " ");
//    char *new_pass_c  = strtok (NULL, " ");
//
//    if (!old_pass || !new_pass || !new_pass_c)
//        return false;
//
//    std::string password_old = old_pass;
//    std::string password_new = new_pass;
//    std::string password_new_c = new_pass_c;
//
//    if (strcmp(new_pass, new_pass_c) != 0)
//    {
//        SendSysMessage (LANG_NEW_PASSWORDS_NOT_MATCH);
//        SetSentErrorMessage (true);
//        return false;
//    }
//
//    if (!AccountMgr::CheckPassword (m_session->GetAccountId(), password_old))
//    {
//        SendSysMessage (LANG_COMMAND_WRONGOLDPASSWORD);
//        SetSentErrorMessage (true);
//        return false;
//    }
//
//    AccountOpResult result = AccountMgr::ChangePassword(m_session->GetAccountId(), password_new);
//
//    switch (result)
//    {
//        case AOR_OK:
//            SendSysMessage(LANG_COMMAND_PASSWORD);
//            break;
//        case AOR_PASS_TOO_LONG:
//            SendSysMessage(LANG_PASSWORD_TOO_LONG);
//            SetSentErrorMessage(true);
//            return false;
//        case AOR_NAME_NOT_EXIST:                            // not possible case, don't want get account name for output
//        default:
//            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
//            SetSentErrorMessage(true);
//            return false;
//    }
//
//    return true;
//}

//bool ChatHandler::HandleLockAccountCommand(const char* args)
//{
//    return false; // not needed at all
//    
//    if (!*args)
//    {
//        SendSysMessage(LANG_USE_BOL);
//        return true;
//    }
//
//    std::string argstr = (char*)args;
//    if (argstr == "on")
//    {
//        AccountsDatabase.PExecute("UPDATE account SET account_state_id = '2' WHERE account_id = '%u'",m_session->GetAccountId());
//        SendSysMessage(LANG_COMMAND_ACCLOCKLOCKED);
//        return true;
//    }
//
//    if (argstr == "off")
//    {
//        AccountsDatabase.PExecute("UPDATE account SET account_state_id = '1' WHERE account_id = '%u'",m_session->GetAccountId());
//        SendSysMessage(LANG_COMMAND_ACCLOCKUNLOCKED);
//        return true;
//    }
//
//    SendSysMessage(LANG_USE_BOL);
//    return true;
//}

/// Display the 'Message of the day' for the realm
//bool ChatHandler::HandleServerMotdCommand(const char* /*args*/)
//{
//    return false; // not needed at all
//    
//    PSendSysMessage(LANG_MOTD_CURRENT, sWorld.GetMotd());
//    return true;
//}

//bool ChatHandler::HandleServerPVPCommand(const char* /*args*/)
//{
//    Player *player = m_session->GetPlayer();
//
//    if (!sWorld.getConfig(CONFIG_BATTLEGROUND_QUEUE_INFO))
//        SendSysMessage("Battleground queue info is disabled");
//    else
//    {
//        if (!(player->InBattleGroundOrArenaQueue()))
//            SendSysMessage("You aren't in any battleground queue");
//        else
//        {
//            BattleGroundQueueTypeId qtype;
//            BattleGroundTypeId bgtype;
//            bool isbg;
//            for (int i=0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
//            {
//                qtype = player->GetBattleGroundQueueTypeId(i);
//                isbg = false;
//                switch (qtype)
//                {
//                case BATTLEGROUND_QUEUE_AB:
//                    {
//                        SendSysMessage("You are queued for Arathi Basin");
//                        isbg = true;
//                        bgtype = BATTLEGROUND_AB;
//                        break;
//                    }
//                case BATTLEGROUND_QUEUE_AV:
//                    {
//                        SendSysMessage("You are queued for Alterac Valley");
//                        isbg = true;
//                        bgtype = BATTLEGROUND_AV;
//                        break;
//                    }
//                case BATTLEGROUND_QUEUE_WS:
//                    {
//                        SendSysMessage("You are queued for Warsong Gulch");
//                        isbg = true;
//                        bgtype = BATTLEGROUND_WS;
//                        break;
//                    }
//                case BATTLEGROUND_QUEUE_EY:
//                    {
//                        SendSysMessage("You are queued for Eye of the storm");
//                        isbg = true;
//                        bgtype = BATTLEGROUND_EY;
//                        break;
//                    }
//                default:
//                    break;
//                }
//
//                if (isbg)
//                {
//                    uint32 minPlayers = sBattleGroundMgr.GetBattleGroundTemplate(bgtype)->GetMinPlayersPerTeam();
//                    uint32 queuedHorde = sBattleGroundMgr.m_BattleGroundQueues[qtype].GetQueuedPlayersCount(TEAM_HORDE, player->GetBattleGroundBracketIdFromLevel(bgtype));
//                    uint32 queuedAlliance = sBattleGroundMgr.m_BattleGroundQueues[qtype].GetQueuedPlayersCount(TEAM_ALLIANCE, player->GetBattleGroundBracketIdFromLevel(bgtype));
//                    PSendSysMessage("Horde queued: %u, Alliance queued: %u. Minimum per team: %u", queuedHorde, queuedAlliance, minPlayers);
//                }
//            }
//        }
//    }
//
//    if (sWorld.getConfig(CONFIG_ARENA_STATUS_INFO))
//        PSendSysMessage("Arena status: %u players in 2v2, %u in 3v3, %u in 5v5",
//            sBattleGroundMgr.inArenasCount[0],sBattleGroundMgr.inArenasCount[1],sBattleGroundMgr.inArenasCount[2]);
//    else
//        SendSysMessage("Arena status is disabled");
//
//    if(sWorld.getConfig(CONFIG_ARENA_DAILY_REQUIREMENT))
//        PSendSysMessage("Today you won %u rated arenas (%u required for reward)",
//            player->m_DailyArenasWon,sWorld.getConfig(CONFIG_ARENA_DAILY_REQUIREMENT));
//
//    return true;
//}

//bool ChatHandler::HandleTeamInfoCommand(const char* args)
//{
//    return false; // not needed at all
//    
//    Player* plr = m_session->GetPlayer();
//
//    if (getSelectedPlayer() && getSelectedPlayer() != plr)
//    {
//        Player* chr = getSelectedPlayer();
//        ArenaTeam* at = sObjectMgr.GetArenaTeamById(chr->GetArenaTeamId(3));
//        if(at)
//        {
//            std::stringstream s;
//            s << "Статистика команды 1х1 игрока " << chr->GetName();
//            s << "\nРейтинг: " << at->GetStats().rating;
//            s << "\nРанг: " << at->GetStats().rank;
//            s << "\nВсего игр за сезон: " << at->GetStats().games_season;
//            s << "\nПобед за сезон: " << at->GetStats().wins_season;
//            s << "\nПроигрышей за сезон: " << (at->GetStats().games_season - at->GetStats().wins_season);
//            SendSysMessage(s.str().c_str());
//            return true;
//        }
//    }
//    else
//    {
//        ArenaTeam* at = sObjectMgr.GetArenaTeamById(plr->GetArenaTeamId(3));
//        if(at)
//        {
//            std::stringstream s;
//            s << "Статистика вашей команды 1х1:";
//            s << "\nРейтинг: " << at->GetStats().rating;
//            s << "\nРанг: " << at->GetStats().rank;
//            s << "\nВсего игр за сезон: " << at->GetStats().games_season;
//            s << "\nПобед за сезон: " << at->GetStats().wins_season;
//            s << "\nПроигрышей за сезон: " << (at->GetStats().games_season - at->GetStats().wins_season);
//            s << "\nВсего игр за неделю: " << at->GetStats().games_week;
//            s << "\nПобед за неделю: " << at->GetStats().wins_week;
//            s << "\nПроигрышей за неделю: " << (at->GetStats().games_week - at->GetStats().wins_week);
//            SendSysMessage(s.str().c_str());
//            return true;
//        }
//    }
//
//    return true;
//}

bool ChatHandler::HandleGMIngameCommand(const char* /*args*/)
{
    bool first = true;

    GmActivityMap const & GMs = sTicketMgr.GetGms();
    for (GmActivityMap::const_iterator itr = GMs.begin(); itr != GMs.end(); ++itr)
    {
        Player* gm = (*itr).first->GetPlayer();
        if (!gm || !gm->IsInWorld())
            continue;

        if (first)
        {
            PSendSysMessage(LANG_GMS_ON_SRV);
            first = false;
        }
        // add additional rules here. If busy - say GM is busy. IF GM is AFK - say he's AFK
        if ((*itr).second.busyCount || (*itr).second.responseDelay >= MAX_RESPONSE_DELAY)
            PSendSysMessage("%s%s", gm->GetName(), GetHellgroundString(LANG_GM_INGAME_BUSY));
        else
            SendSysMessage(gm->GetName());
    }

    if (first)
        SendSysMessage(LANG_GMS_NOT_LOGGED);

    return true;
}

//bool ChatHandler::HandleAccountBGMarksCommand(const char* /*args*/)
//{
//    if (m_session->IsAccountFlagged(ACC_RESTRICT_BG_MARKS))
//    {
//        m_session->RemoveAccountFlag(ACC_RESTRICT_BG_MARKS);
//        SendSysMessage("Battleground Marks of Honor restriction has been disabled for this account.");
//    }
//    else
//    {
//        m_session->AddAccountFlag(ACC_RESTRICT_BG_MARKS);
//        SendSysMessage("Battleground Marks of Honor restriction has been enabled for this account.");
//    }
//
//    return true;
//}

bool ChatHandler::HandleLanguageCommand(const char* args)
{
    if (m_session->IsAccountFlagged(ACC_INFO_LANG_RU))
    {
        m_session->RemoveAccountFlag(ACC_INFO_LANG_RU);
        SendSysMessage(LANG_INFO_LANGUAGE_BLIZZ);
    }
    else
    {
        m_session->AddAccountFlag(ACC_INFO_LANG_RU);
        SendSysMessage(LANG_INFO_LANGUAGE_RUSSIAN);
    }

    return true;
}

//bool ChatHandler::HandleAccountAltRealmCommand(const char* args)
//{
//    return false; // not needed at all
//    
//    if (m_session->IsAccountFlagged(ACC_REALM_ALT_ADDRESS))
//    {
//        m_session->RemoveAccountFlag(ACC_REALM_ALT_ADDRESS);
//        SendSysMessage(LANG_IP_ADDR_ALT_SWITCH_OFF);
//    }
//    else
//    {
//        m_session->AddAccountFlag(ACC_REALM_ALT_ADDRESS);
//        SendSysMessage(LANG_IP_ADDR_ALT_SWITCH_ON);
//    }
//
//    return true;
//}

bool ChatHandler::HandleGuildAnnounceCommand(const char *args)
{
    if (!*args)
        return false;

    std::string msg = args;
    std::wstring wmsg;
    
    if (!Utf8toWStr(msg, wmsg))
        return false;

    SetSentErrorMessage(true);
    
    uint32 gId = m_session->GetPlayer()->GetGuildId();
    if (!gId)
    {
        PSendSysMessage(LANG_GUILD_ANN_1);
        return false;
    }
    
    if (sGuildMgr.GetGuildAnnCooldown(gId) > time(NULL))
    {
        PSendSysMessage(LANG_GUILD_ANN_2, m_session->secondsToTimeString(uint32(sGuildMgr.GetGuildAnnCooldown(gId) - time(NULL))).c_str());
        return false;
    }
    
    if (wmsg.size() > sWorld.getConfig(CONFIG_GUILD_ANN_LENGTH))
    {
        PSendSysMessage(LANG_GUILD_ANN_3, sWorld.getConfig(CONFIG_GUILD_ANN_LENGTH));
        return false;
    }
    
    Guild * pGuild = sGuildMgr.GetGuildById(gId);
    if (!pGuild)
    {
        SendSysMessage("Error");
        return false;
    }

    if (pGuild->IsFlagged(GUILD_FLAG_DISABLE_ANN))
    {
        PSendSysMessage(LANG_GUILD_ANN_8);
        return false;
    }

    if (!pGuild->HasRankRight(m_session->GetPlayer()->GetRank(), GR_RIGHT_OFFCHATLISTEN))
    {
        PSendSysMessage(LANG_GUILD_ANN_4);
        return false;
    }

    if (pGuild->GetMemberSize() < 10)
    {
        PSendSysMessage(LANG_GUILD_ANN_5);
        return false;
    }

    if (ContainsNotAllowedSigns(msg, true))
    {
        PSendSysMessage(LANG_GUILD_ANN_6);
        return false;
    }

    PSendSysMessage(LANG_GUILD_ANN_7, m_session->secondsToTimeString(sWorld.getConfig(CONFIG_GUILD_ANN_COOLDOWN)).c_str());

    sGuildMgr.SaveGuildAnnCooldown(gId);
    sLog.outLog(LOG_GUILD_ANN, "Player %s (%llu) - guild: %s (%u) append guild announce: %s", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetGUID(), pGuild->GetName().c_str(), gId, msg.c_str());
    sWorld.QueueGuildAnnounce(gId, m_session->GetPlayer()->GetTeam(), msg);

    if (!pGuild->IsFlagged(GUILD_FLAG_ADVERT_SET))
    {
        RealmDataDatabase.escape_string(msg);
        RealmDataDatabase.PExecute("UPDATE guild SET ShortAdvert='%s' WHERE guildid='%u'", msg.c_str(), gId);
    }
    return true;
}

//bool ChatHandler::HandleGuildAdvertCommand(const char *args)
//{
//    if (!*args)
//        return false;
//
//    std::string msg = args;
//
//    SetSentErrorMessage(true);
//    
//    uint32 gId = m_session->GetPlayer()->GetGuildId();
//    if (!gId)
//    {
//        SendSysMessage("You are not in a guild.");
//        return false;
//    }
//    
//    if (msg.size() > 100)
//    {
//        SendSysMessage("Your message is to long, limit: 100 chars.");
//        return false;
//    }
//    
//    Guild * pGuild = sGuildMgr.GetGuildById(gId);
//    if (!pGuild)
//    {
//        SendSysMessage("You are not in a guild.");
//        return false;
//    }
//    
//    if (pGuild->GetLeader() != m_session->GetPlayer()->GetGUID())
//    {
//        SendSysMessage("You need to be guild master to use this command.");
//        return false;
//    }
//    
//    if (pGuild->GetMemberSize() < 25)
//    {
//        SendSysMessage("Your guild is to small to set up an advert, minimum 25 players.");
//        return false;
//    }
//
//    RealmDataDatabase.escape_string(msg);
//    RealmDataDatabase.PExecute("UPDATE guild SET ShortAdvert='%s' WHERE guildid='%u'", msg.c_str(), gId);
//    pGuild->AddFlag(GUILD_FLAG_ADVERT_SET);
//    return true;
//}

//bool ChatHandler::HandleRaidRulesCommand(const char* /*args*/)
//{
//    // disabled
//    return false;
//    
//    Player* player = m_session->GetPlayer();
//
//    if (!player)
//        return true;
//
//    Group* group = player->GetGroup();
//    if (!group)
//    {
//        SendSysMessage("You are not in group.");
//        return false;
//    }
//
//    if (!group->isRaidGroup() || group->isBGGroup())
//    {
//        SendSysMessage("You are not in raid group.");
//        return false;
//    }
//
//    //group->SendRulesTo(player);
//    return true;
//}


bool ChatHandler::HandleCaptcha(const char* args)
{
	if (!sWorld.getConfig(CONFIG_CAPTCHA_ENABLED))
		return true;
	
	Player* plr = m_session->GetPlayer();
	if (!plr)
	{
		SendSysMessage("Specified player not found.");
		SetSentErrorMessage(true);
		return true;
	}

	if (m_session->GetPlayer()->GetDummyAura(54830))
	{
		SendSysMessage(GetHellgroundString(LANG_NOT_MORE_OFTEN_THAN_10_SEC));
		return true;
	}
	else
		m_session->GetPlayer()->CastSpell(m_session->GetPlayer(), 54830, true);

	uint32 answer = atoi(args);
	//std::string str(args);

	if (!answer)
	{
		SendSysMessage(16597); //You need to solve an example by specifying...
		return true;
	}

	if (plr->captcha_current.empty())
		return true;

	if (plr->DoCaptcha(answer))
		SendSysMessage(16581); //Challenge passed.
	else	
		SendSysMessage(16582); //You entered the wrong code in the message. Try again.

	return true;
}

// promo bonus for registration with code
bool ChatHandler::HandleBonusCommand(const char* /*args*/)
{
	//if (!sWorld.isEasyRealm())
	//{
	//	// You can't do that at the moment.
	//	SendSysMessage(16553);
	//	return true;
	//}

	if (!m_session->IsAccountFlagged(ACC_PROMO_BONUS))
	{
		// Already received
		SendSysMessage(LANG_VK_GIFT_ALREADY_RECEIVED);
		return true;
	}

	Player* plr = m_session->GetPlayer();
	if (!plr)
	{
		SendSysMessage("Specified player not found.");
		SetSentErrorMessage(true);
		return true;
	}

	if (sWorld.isEasyRealm())
	{
		if (plr->GiveItem(10594, 1))
		{
			m_session->RemoveAccountFlag(ACC_PROMO_BONUS);
			return true;
		}
	}
	else
	{
		// 8 slot bag
		if (plr->GiveItem(5603, 1))
		{
			m_session->RemoveAccountFlag(ACC_PROMO_BONUS);
			return true;
		}
	}

	// error if got here
	SendSysMessage("ERROR");
	SetSentErrorMessage(true);
	return true;
}

// VK gift
bool ChatHandler::HandleGiftCommand(const char* /*args*/)
{
	//if (!sWorld.isEasyRealm())
	//{
	//	// You can't do that at the moment.
	//	SendSysMessage(16553);
	//	return true;		
	//}

    if (m_session->IsAccountFlagged(ACC_VK_GIFT_RECEIVED))
    {
        // Already received
        SendSysMessage(LANG_VK_GIFT_ALREADY_RECEIVED);
        return true;
    }

    Player* plr = m_session->GetPlayer();
    if (!plr)
    {
        SendSysMessage("Specified player not found.");
        SetSentErrorMessage(true);
        return true;
    }

    if (m_session->IsAccountFlagged(ACC_ALLOW_VK_GIFT_RECEIVE))
    {
		if (sWorld.isEasyRealm())
		{
			if (plr->GiveItem(SUBSCRIBER_CHEST, 1))
			{
				m_session->AddAccountFlag(ACC_VK_GIFT_RECEIVED);
				SendSysMessage(LANG_VK_GIFT_SENT);
				return true;
			}
		}
		else
		{
			// 8 slot bag
			if (plr->GiveItem(5603, 1))
			{
				m_session->AddAccountFlag(ACC_VK_GIFT_RECEIVED);
				SendSysMessage(LANG_VK_GIFT_SENT);
				return true;
			}
		}
    }
    else
    {
        SendSysMessage(LANG_VK_GIFT_NEED_SUBSCRIPTION);
        return true;
    }

    // error if got here
    SendSysMessage("Could not create item.");
    SetSentErrorMessage(true);
    return true;
}

// given by special events (like "Last login over 3 months...")
// view BOX_BONUS.sql
bool ChatHandler::HandleBoxCommand(const char* /*args*/)
{
	//if (!sWorld.isEasyRealm())
	//{
	//	// You can't do that at the moment.
	//	SendSysMessage(16553);
	//	return true;
	//}

	if (m_session->IsAccountFlagged(ACC_BOX_BONUS_RECEIVED))
	{
		// Already received
		SendSysMessage(LANG_BOX_BONUS_ALREADY_RECEIVED);
		return true;
	}

	Player* plr = m_session->GetPlayer();
	if (!plr)
	{
		SendSysMessage("Specified player not found.");
		SetSentErrorMessage(true);
		return false;
	}

	if (m_session->IsAccountFlagged(ACC_ALLOW_BOX_BONUS_RECEIVE))
	{
		if (plr->GiveItem(OLD_MAN_CHEST, 1))
		{
			m_session->AddAccountFlag(ACC_BOX_BONUS_RECEIVED);
			return true;
		}	
	}
	else
	{
		SendSysMessage(LANG_BOX_BONUS_NEED_PARTICIPATION);
	}

	// error if got here
	SendSysMessage("Could not create item.");
	SetSentErrorMessage(true);
	return true;
}

//bool ChatHandler::HandleBetaParticipationGiftCommand(const char* /*args*/)
//{
//    return false; // not needed at all
//
//    if (m_session->IsAccountFlagged(ACC_BETA_PART_GIFT_RECEIVED))
//    {
//        // Already received
//        SendSysMessage(LANG_BETA_PART_GIFT_ALREADY_RECEIVED);
//        return true;
//    }
//
//    Player* plr = m_session->GetPlayer();
//    if (!plr)
//    {
//        SendSysMessage("Specified player not found.");
//        SetSentErrorMessage(true);
//        return false;
//    }
//
//    if (m_session->IsAccountFlagged(ACC_ALLOW_BETA_PART_GIFT_RECEIVE))
//    {
//        if (ObjectMgr::GetItemPrototype(21930))
//        {
//            ItemPosCountVec dest;
//            uint32 no_space_count = 0;
//            uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 21930, 1, &no_space_count);
//            if (msg != EQUIP_ERR_OK)                       // convert to possible store amount
//            {
//                plr->SendEquipError(msg, NULL, NULL);
//                return true;
//            }
//
//            if (!dest.empty())                // can add some
//            {
//                if (Item* item = plr->StoreNewItem(dest, 21930, true, 0, "PLAYER_COMMAND"))
//                {
//                    plr->SendNewItem(item, 1, true, false);
//                    m_session->AddAccountFlag(ACC_BETA_PART_GIFT_RECEIVED);
//                    SendSysMessage(LANG_BETA_PART_GIFT_SENT);
//                    return true;
//                }
//            }
//        }
//    }
//    else
//    {
//        SendSysMessage(LANG_BETA_PART_GIFT_NEED_PARTICIPATION);
//        return true;
//    }
//
//    // error if got here
//    SendSysMessage("Could not create item.");
//    SetSentErrorMessage(true);
//    return true;
//}

bool ChatHandler::HandleShopCommand(const char* /*args*/)
{
    Player* plr = m_session->GetPlayer();
    if (!plr)
    {
        SendSysMessage("Specified player not found.");
        SetSentErrorMessage(true);
        return false;
    }

    // cast "Summon Shop NPC"
    plr->CastSpell(plr, 55313, false);
    return true;
}

bool ChatHandler::HandlePremiumCommand(const char* /*args*/)
{
    Player* plr = m_session->GetPlayer();
    if (!plr)
    {
        SendSysMessage("Specified player not found.");
        SetSentErrorMessage(true);
        return false;
    }

    // cast "Summon Premium NPC"
    if (!plr->GetSession()->isPremium())
        ChatHandler(plr).SendSysMessage(LANG_PREMIUM_NOT_ACTIVE);
    else
    {
        plr->CastSpell(plr, 55314, false);
        plr->SendPlaySound(12182, true);
    }
    return true;
}

bool ChatHandler::HandleXpRatesCommand(const char* args)
{
	if (sWorld.isEasyRealm())
	{
		// You can't do that at the moment.
		SendSysMessage(16553);
		return true;
	}
	
	if (!*args)
        return false;

    Player* plr = m_session->GetPlayer();
    if (!plr)
    {
        SendSysMessage("Specified player not found.");
        SetSentErrorMessage(true);
        return false;
    }

    std::string msg = args;
    if (msg == "freeze")
    {
        // x1 rates flag is not touched

		if (!plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_FROZEN))
		{
			plr->AddPlayerCustomFlag(PL_CUSTOM_XP_RATE_FROZEN);
			sLog.outLog(LOG_EXP, "Player %s (guid: %u) flag PL_CUSTOM_XP_RATE_FROZEN added", plr->GetName(), plr->GetGUIDLow());
		}

        // Hardcore flag is not touched

        SendSysMessage(m_session->GetHellgroundString(LANG_XP_X0_SYSMSG));
    }
    else if (msg == "hardcore")
    {
        // unfreeze xp gain
        if (plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_FROZEN))
            plr->RemovePlayerCustomFlag(PL_CUSTOM_XP_RATE_FROZEN);

        // add x1 flag if not present
		if (!plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1))
		{
			plr->AddPlayerCustomFlag(PL_CUSTOM_XP_RATE_X1);
			sLog.outLog(LOG_EXP, "Player %s (guid: %u) flag PL_CUSTOM_XP_RATE_X1 added", plr->GetName(), plr->GetGUIDLow());
		}

        SendSysMessage(m_session->GetHellgroundString(LANG_XP_X1_SYSMSG));

        // If level < level 10 and doesn't have hardcore flag -> then add it.
        // A player can switch between x0 and x1 without losing hardcore flag
		if (plr->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1))
			// For reaching level 70 with x1 experience rates you will receive a special reward! Read more: https://moonwell.su/bonus   
			SendSysMessage(m_session->GetHellgroundString(LANG_XP_HARDCORE_INFO));

        if (plr->GetLevel() < 10 && !plr->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1))
        {
            plr->AddPlayerCustomFlag(PL_CUSTOM_HARDCORE_X1);
			sLog.outLog(LOG_EXP, "Player %s (guid: %u) flag PL_CUSTOM_HARDCORE_X1 added", plr->GetName(), plr->GetGUIDLow());			     
        }
    }
    // allow usage of "default" when we can easily reapply hardcore flag. Allow only "confirm_default" when we cannot reapply hardcore flag and we have it at the moment
    else if ((msg == "default" && (!plr->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1) || plr->GetLevel() < 10)) ||
        (msg == "confirm_default" && plr->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1) && plr->GetLevel() >= 10))
    {
        // unfreeze xp gain
		if (plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_FROZEN))
		{
			plr->RemovePlayerCustomFlag(PL_CUSTOM_XP_RATE_FROZEN);
			sLog.outLog(LOG_EXP, "Player %s (guid: %u) flag PL_CUSTOM_XP_RATE_FROZEN removed", plr->GetName(), plr->GetGUIDLow());
		}

        // remove x1 xp flag is present
		if (plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1))
		{
			plr->RemovePlayerCustomFlag(PL_CUSTOM_XP_RATE_X1);
			sLog.outLog(LOG_EXP, "Player %s (guid: %u) flag PL_CUSTOM_XP_RATE_X1 removed", plr->GetName(), plr->GetGUIDLow());
		}

        // remove hardcore flag
		if (plr->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1))
		{
			plr->RemovePlayerCustomFlag(PL_CUSTOM_HARDCORE_X1);
			sLog.outLog(LOG_EXP, "Player %s (guid: %u) flag PL_CUSTOM_HARDCORE_X1 removed", plr->GetName(), plr->GetGUIDLow());

			// You will not receive special rewards for reaching level 70.
			SendSysMessage(m_session->GetHellgroundString(16550));
		}   

        SendSysMessage(m_session->GetHellgroundString(LANG_XP_DEFAULT_SYSMSG));
    }
    else if (msg == "default" && plr->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1))
    {
        // only allow "confirm_default" for hardcore
        SendSysMessage(LANG_XP_RATE_NEED_CONFIRM);
    }
    else // wrong args
        return false;

    return true;
}

bool ChatHandler::HandleBgRegSummonCommand(const char* /*args*/)
{
	if (!sWorld.getConfig(CONFIG_BG_QUEUE_FROM_ANYWHERE))
    {
        SendSysMessage("This command is disabled.");
        return true;
    }

    Player* plr = m_session->GetPlayer();
    if (!plr)
		return true;

	if (plr->InBattleGroundOrArena())
	{
		SendSysMessage(16558);
		return true;
	}

    // cast "Summon bg registrator"
    plr->CastSpell(plr, 55325, false);
    return true;
}

//bool ChatHandler::HandleSpecializationCommand(const char* /*args*/)
//{
    //// only x3
    //if (sWorld.getConfig(CONFIG_REALM_TYPE) != REALM_X3)
    //    return false;
    //
    //Player* plr = m_session->GetPlayer();
    //if (!plr)
    //{
    //    SendSysMessage("Specified player not found.");
    //    SetSentErrorMessage(true);
    //    return false;

    //}

    //uint32 availableCharges = plr->GetSession()->isPremium() ? 10 : 2;

    //// 55325 is NOT the first spec reset spell. First spell is 55326.
    //uint32 useSpellId = 55325;
    //for (uint32 i = 1; i < (availableCharges + 1); ++i)
    //{
    //    if (!plr->HasSpellCooldown(useSpellId + i))
    //    {
    //        useSpellId += i;
    //        break;
    //    }
    //}

    //if (useSpellId == 55325)
    //{
    //    uint32 lowestCD = plr->GetSpellCooldownDelay(55326);
    //    for (uint32 i = 0; i < (availableCharges - 1); ++i)
    //    {
    //        uint32 curCD = plr->GetSpellCooldownDelay(55327 + i);
    //        if (curCD < lowestCD)
    //            lowestCD = curCD;
    //    }

    //    PSendSysMessage(LANG_AMNESIA_SPEC_CHANGE, m_session->secondsToTimeString(lowestCD).c_str());
    //    return true;
    //}

    //// cast "Amnesia"
    //plr->CastSpell(plr, useSpellId, false);
    //return true;
//}

//bool ChatHandler::HandleBgWinsCommand(const char* /*args*/)
//{
//	return;
//	
//	Player* plr = m_session->GetPlayer();
//    if (!plr)
//    {
//        SendSysMessage("Specified player not found.");
//        SetSentErrorMessage(true);
//        return false;
//
//    }
//
//    uint32 games = 0;
//    uint32 wins = 0;
//    //uint32 rwins = 0;
//    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT bg_games,bg_wins FROM characters_bg WHERE guid = %u", plr->GetGUIDLow());
//    if (result)
//    {
//        games = (*result)[0].GetUInt32();
//        wins = (*result)[1].GetUInt32();
//        //rwins = (*result)[2].GetUInt32();
//    }
//
//    PSendSysMessage(LANG_BG_WINS_CURR, games, wins);
//    return true;
//}

bool ChatHandler::HandleDeleteTransmog(const char* args)
{
    if (!*args)
        return false;

    Player* plr = m_session->GetPlayer();
    if (!plr)
    {
        SendSysMessage("Specified player not found.");
        SetSentErrorMessage(true);
        return false;
    }
    
    uint32 itemId = 0;

    if (args[0] == '[')                                        // [name] manual form
    {
        char* citemName = citemName = strtok((char*)args, "]");

        if (citemName && citemName[0])
        {
            std::string itemName = citemName + 1;
            GameDataDatabase.escape_string(itemName);
            QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
            if (!result)
                return false;

            itemId = result->Fetch()->GetUInt16();
        }
        else
            return false;
    }
    else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
    {
        char* cId = extractKeyFromLink((char*)args, "Hitem");
        if (!cId)
            return false;
        itemId = atol(cId);
    }

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemId);
    if (!pProto)
        return false;

    uint32 active;
    // GENSENTODO need to remove select and get from memory
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT active FROM character_transmogrification WHERE guid = %u AND itemId = %u", plr->GetGUIDLow(), itemId);
    if (result)
    { 
        active = (*result)[0].GetUInt32();
        if (active)
        {
            PSendSysMessage(LANG_TRANSMOG_MUST_BE_DEACTIVATED, itemId);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        PSendSysMessage(LANG_TRANSMOG_ITEM_NOT_FOUND, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    plr->GetTransmogManager()->RemoveTransmogItemid(itemId);
    SendSysMessage(LANG_TRANSMOG_REMOVED);

    //plr->PlayerTalkClass->CloseGossip();
    return true;
}

//bool ChatHandler::HandleNicknameReservation(const char* args)
//{
//    if (sWorld.getConfig(CONFIG_REALM_TYPE) != REALM_X100)
//        return false;
//    
//    Player* plr = m_session->GetPlayer();
//    if (!plr)
//    {
//        SendSysMessage("Specified player not found.");
//        SetSentErrorMessage(true);
//        return false;
//    }
//    
//    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT 1 FROM reserved_name WHERE name = '%s'", plr->GetName());
//    if (result)
//    {
//        PSendSysMessage(LANG_ALREADY_RESERVED);
//        return true;
//    }
//    
//    static SqlStatementID insert;
//    SqlStatement stmt = RealmDataDatabase.CreateStatement(insert, "INSERT INTO reserved_name (`guid`, `name`, `accid`) VALUES (?, ?, ?)");
//    stmt.addUInt32(uint32(plr->GetGUID()));
//    stmt.addString(plr->GetName());
//    stmt.addUInt32(plr->GetSession()->GetAccountId());
//    stmt.Execute();
//    HandleReloadReservedNameCommand("");
//
//    PSendSysMessage(LANG_RESERVED);
//
//    return true;
//}

//bool ChatHandler::HandleBetaFree(const char* args)
//{
//    if (sWorld.getConfig(CONFIG_REALM_TYPE) != REALM_X100)
//        return false;
//    
//    Player* plr = m_session->GetPlayer();
//    if (!plr)
//    {
//        SendSysMessage("Specified player not found.");
//        SetSentErrorMessage(true);
//        return false;
//    }
//
//    plr->GiveItem(29434, 1000);
//    plr->GiveItem(30796, 200); // gold
//    plr->GiveItem(20558, 20); // wsg
//    plr->GiveItem(35521, 1); // fly mount
//    plr->GiveItem(1136, 1); // teleport
//
//    return true;
//}

// Discord code bonus
bool ChatHandler::HandleActivateCode(const char* args)
{
	if (!sWorld.isEasyRealm())
	{
		SendSysMessage("Command disabled");
		SetSentErrorMessage(true);
		return false;
	}

	if (!*args)
        return false;

    Player* plr = m_session->GetPlayer();
    if (!plr)
    {
        SendSysMessage("Specified player not found.");
        SetSentErrorMessage(true);
        return false;
    }

    std::string code = strtok((char*)args, " ");
    std::string needed_code = sWorld.m_bonusCode;

    uint32 max_winners = 30;

    if (needed_code.empty() || code != needed_code)
    {
        SendSysMessage(LANG_BONUS_CODE_ERROR);
        return true;
    }

    for (auto& val : sWorld.m_bonusCodeUsers)
    {
        if (val.first == m_session->GetAccountId() || val.second == m_session->GetRemoteAddress())
        {
            SendSysMessage(15554);
            return true;
        }
    }

    sWorld.m_bonusCodeUsers[m_session->GetAccountId()] = m_session->GetRemoteAddress();

    plr->GiveItem(BONUS_CHEST, 1);

    uint32 size = sWorld.m_bonusCodeUsers.size();

    sWorld.SendWorldText(LANG_BONUS_CODE_ACTIVATED, 0, plr->GetName(), max_winners - size);
    sLog.outLog(LOG_SPECIAL, "Bonus code activated by %s", plr->GetName());

    if (size >= max_winners)
    {
        sWorld.m_bonusCode.clear();
        sWorld.m_bonusCodeUsers.clear();
    }

    return true;
}

bool ChatHandler::HandleRaidtest(const char* args)
{
    if (!*args)
        return false;

    if (!sWorld.getConfig(CONFIG_SPECIAL_COMMAND_USERS_ENABLED))
    { 
        SendSysMessage("Command disabled. Please, contact admin.");
        SetSentErrorMessage(true);
        return false;
    }

    // add command_users
    uint32 command_users[] = {
        // exodus
        138664, 139916, 139826, 143241, 142823, 142126,
        // signa
        149204
    };
    
    // can be used for
    // select a.guid from characters a, realmd.exodus b where a.account=b.account_id;
    uint32 allowed_players[] = {
		1,
        // exodus
        //139002, 140079, 139916, 140153, 140123, 139062, 140112, 139826, 140096, 140832, 140834, 138664, 140080, 151132, 139833, 139055, 139933, 140174, 139001, 140107, 138976, 140826, 140105, 139801, 140094, 140076, 139094, 139751, 140083, 140138, 139020, 140086, 140124, 139821, 140162, 138974, 140113, 138633, 139154, 140072, 139088, 140103, 139790, 139874, 138985, 140104, 139895, 140209, 139059, 140093, 139071, 139057, 142126, 23722, 142670, 142297, 142823, 140506, 143241,
        // signa
        //149225, 149236, 149237, 149238, 149240, 149241, 149242, 149243, 149245, 149246, 149408, 149226, 149248, 149250, 149251, 149227, 149228, 149229, 149230, 149232, 149234, 149235, 149204, 149258, 149253
    };

    Player* plr = m_session->GetPlayer();

    //uint32 command_user = sWorld.getConfig(CONFIG_SPECIAL_COMMAND_USER);
    if (!plr)
        return false;

    bool can = std::find(std::begin(command_users), std::end(command_users), plr->GetGUID()) != std::end(command_users);
    if (!can)
    {
        return false;
    }

    Unit* target = getSelectedUnit();

    if (!target || !plr->GetSelection())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // check allowed players
    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        bool exist = std::find(std::begin(allowed_players), std::end(allowed_players), target->GetGUID()) != std::end(allowed_players);
        if (!exist)
        {
            PSendSysMessage("Can not be used for this player.");
            SetSentErrorMessage(true);
            return false;
        }
    }

    std::string msg = args;

    if (msg == "revive")
    {
        if (!plr->GetMap()->IsDungeon())
        {
            PSendSysMessage("Can be only used in instances.");
            SetSentErrorMessage(true);
            return false;
        }

        if (!plr->GetMap()->IsHeroicRaid())
        {
            PSendSysMessage("Can not be used in normal raids.");
            SetSentErrorMessage(true);
            return false;
        }
        
        if (target->GetTypeId() == TYPEID_PLAYER && ((Player*)target)->isDead())
        {
            ((Player*)target)->ResurrectPlayer(0.5f);
            ((Player*)target)->SpawnCorpseBones();
            ((Player*)target)->SaveToDB();
        }
    }
    else if (msg == "respawn")
    {
        if (!plr->GetMap()->IsDungeon())
        {
            PSendSysMessage("Can be only used in instances.");
            SetSentErrorMessage(true);
            return false;
        }

        if (!plr->GetMap()->IsHeroicRaid())
        {
            PSendSysMessage("Can not be used in normal raids.");
            SetSentErrorMessage(true);
            return false;
        }
        
        if (target->GetTypeId() != TYPEID_UNIT)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }

        if (target->isDead())
        {
            ((Creature*)target)->Respawn();
        }
    }
    else if (msg == "unbind")
    {
        Player* player = getSelectedPlayer();
        if (!player) player = m_session->GetPlayer();
        uint32 counter = 0;
        for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        {
            Player::BoundInstancesMap &binds = player->GetBoundInstances(i);
            for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
            {
                if (itr->first != player->GetMapId())
                {
                    InstanceSave *save = itr->second.save;
                    std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
                    PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetSaveInstanceId(), itr->second.perm ? "yes" : "no", save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                    player->UnbindInstance(itr, i);
                    counter++;
                }
                else
                    ++itr;
            }
        }
        PSendSysMessage("instances unbound: %d", counter);
    }
    else if (msg == "die")
    {
        if (!plr->GetMap()->IsDungeon())
        {
            PSendSysMessage("Can be only used in instances.");
            SetSentErrorMessage(true);
            return false;
        }

        if (!plr->GetMap()->IsHeroicRaid())
        {
            PSendSysMessage("Can not be used in normal raids.");
            SetSentErrorMessage(true);
            return false;
        }
        
        if (target->isAlive() && target->GetTypeId() == TYPEID_UNIT)
        {
            plr->Kill(target);
        }
    }

    return true;
}

//bool ChatHandler::HandleBGKick(const char* args)
//{
//    Player* target = NULL;
//
//    char* px = strtok((char*)args, " ");
//    char* py = NULL;
//
//    std::string name;
//
//    if (px)
//    {
//        name = px;
//
//        if (name.empty())
//            return false;
//
//        if (!normalizePlayerName(name))
//        {
//            SendSysMessage(LANG_PLAYER_NOT_FOUND);
//            SetSentErrorMessage(true);
//            return false;
//        }
//
//        target = sObjectMgr.GetPlayerInWorld(name.c_str());
//    }
//
//    if (!target)
//    {
//        target = getSelectedPlayer();
//    }
//
//    if (!target)
//    {
//        SendSysMessage(LANG_PLAYER_NOT_FOUND);
//        SetSentErrorMessage(true);
//        return false;
//    }
//
//    if (!target->InBattleGroundOrArena())
//    {
//        SendSysMessage("Player is not in Battleground");
//        SetSentErrorMessage(true);
//        return false;
//    }
//
//    target->LeaveBattleground();
//    ChatHandler(target).PSendSysMessage(15490, m_session->GetPlayer()->GetName());
//
//    return true;
//}

bool ChatHandler::HandleGuildMarker(const char* /*args*/)
{
	if (m_session->GetPlayer()->IsPlayerCustomFlagged(PL_CUSTOM_DISABLE_GH_DOTS))
	{
		m_session->GetPlayer()->RemovePlayerCustomFlag(PL_CUSTOM_DISABLE_GH_DOTS);
		SendSysMessage(15645);
	}
	else
	{
		m_session->GetPlayer()->AddPlayerCustomFlag(PL_CUSTOM_DISABLE_GH_DOTS);
		SendSysMessage(15644);
	}

    return true;
}

bool ChatHandler::HandleGolem(const char* args)
{
	if (!sWorld.isEasyRealm())
		return true;
	
	if (!*args)
	{
		SendSysMessage(15628);
		return true;
	}

	Player* p = m_session->GetPlayer();
	Creature* golem = nullptr;

	GuardianPetList const& plguardians = p->GetGuardians();
	for (GuardianPetList::const_iterator itr = plguardians.begin(); itr != plguardians.end(); ++itr)
		if (Creature* guardian = p->GetMap()->GetCreatureOrPet(*itr))
			if (guardian->GetEntry() == NPC_GOLEM_GUARDIAN)
			{
				golem = guardian;
			}		

	if (!golem)
	{
		SendSysMessage(15646);
		return true;
	}

	std::string argstr = (char*)args;
	if (argstr == "kill")
	{
		if(golem->isAlive())
			p->Kill(golem);
	}
	//else if (argstr == "testcrash2")
	//{
	//	if (golem->isAlive() && !golem->HasAura(9454))
	//	{
	//		golem->setDeathState(JUST_DIED);
	//		golem->Respawn();
	//	}
	//}
	else
	{
		SendSysMessage(15647);
		return true;
	}

	return true;
}

//bool ChatHandler::HandleRaidChest(const char* args)
//{
//	if (!sWorld.isEasyRealm())
//		return true;
//
//    return true;
//}

bool ChatHandler::HandleHide(const char* /*args*/)
{
    Player* plr = m_session->GetPlayer();
    if (!plr)
        return false;
    
    if (!plr->IsHidden())
    {
        if (plr->GetArenaPersonalRating(0) < 1800)
        {
            SendSysMessage(15555);
            return true;
        }
        
        plr->AddPlayerCustomFlag(PL_CUSTOM_HIDDEN);
        SendSysMessage(15556);
    }
    else
    {
        plr->RemovePlayerCustomFlag(PL_CUSTOM_HIDDEN);
        SendSysMessage(15557);
    }
    return true;
}

bool ChatHandler::HandleInfo(const char* /*args*/)
{
	Player* plr = m_session->GetPlayer();
	if (!plr)
		return false;

	if (!sWorld.getConfig(CONFIG_IS_LOCAL))
	{
		if (m_session->GetPlayer()->GetDummyAura(54830))
		{
			SendSysMessage(GetHellgroundString(LANG_NOT_MORE_OFTEN_THAN_10_SEC));
			return true;
		}
		else
			m_session->GetPlayer()->CastSpell(m_session->GetPlayer(), 54830, true);
	}

	uint32 RAF_exp_bonus = (plr->GetSession()->IsRaf() && plr->CheckRAFConditions_XP() && !plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1)) ? 16559 : 16560;

	uint32 RAF_flags_good = 3;
	uint32 RAF_flags = 0;
	if (plr->GetSession()->IsRaf())
		RAF_flags |= 1;
	if (plr->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_REFER_A_FRIEND))
		RAF_flags |= 2;

	uint32 RAF_status = RAF_flags == RAF_flags_good ? 16559 : 16560;

	uint8 rate = 0;

	if (plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_FROZEN))
		rate = 0;
	else if (plr->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1))
		rate = 1;
	else
	{
		rate = plr->GetXPRate(RATE_XP_KILL);
	}

	std::string RAF_status_str = std::string(GetHellgroundString(RAF_status)) + (RAF_status ? "" : " (" + std::to_string(RAF_flags) + ")");

	// Experience rates: %u
	PSendSysMessage(16562, rate);

	// Referral status: %s
	PSendSysMessage(16566, RAF_status_str.c_str());

	// Referral experience bonus: %s
	if (RAF_status)
	PSendSysMessage(16563, GetHellgroundString(RAF_exp_bonus));

	// Bonus levels: %u
	if (RAF_status)
	{	
		PSendSysMessage(16565, plr->GetGrantableLevels());
	}

	// Hardcore mode: %s
	if (!sWorld.isEasyRealm())
	{
		uint32 HC_mode = plr->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1) ? 16559 : 16560;
		PSendSysMessage(16564, GetHellgroundString(HC_mode));
	}

	return true;
}

bool ChatHandler::HandleRaidStats(const char* args)
{
    if (m_session->IsAccountFlagged(ACC_DISABLED_RAIDSTATS))
    {
        m_session->RemoveAccountFlag(ACC_DISABLED_RAIDSTATS);
        SendSysMessage(16646);
    }
    else
    {
        m_session->AddAccountFlag(ACC_DISABLED_RAIDSTATS);
        SendSysMessage(16647);
    }

    return true;
}

bool ChatHandler::HandleEnchantLegweapon(const char* args)
{
    if (!sWorld.isEasyRealm())
    {
    	// You can't do that at the moment.
    	SendSysMessage(16553);
    	return true;		
    }
    
    Player* plr = m_session->GetPlayer();
    if (!plr)
        return false;
    
    if (uint32 id = sScriptMgr.GetScriptId("mw_player_enchant_legweapon"))
        sScriptMgr.OnGossipHello(plr, id);

    return true;
}
