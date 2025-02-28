// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2008-2009 TrinityCore <http://www.trinitycore.org/>
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
#include <regex>

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "AuctionHouseMgr.h"
#include "AccountMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "GameObject.h"
#include "Chat.h"
#include "Log.h"
#include "Guild.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "SpellAuras.h"
#include "ScriptMgr.h"
#include "Language.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "Weather.h"
#include "PointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "PathFinder.h"
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "SystemConfig.h"
#include "Util.h"
#include "ItemEnchantmentMgr.h"
#include "BattleGroundMgr.h"
#include "InstanceSaveMgr.h"
#include "InstanceData.h"
#include "CreatureEventAIMgr.h"
#include "ChannelMgr.h"
#include "GuildMgr.h"
#include "Shop.h"
#include "WardenDataStorage.h"
#include "World.h"
#include "GameEvent.h"
#include "Spell.h"

bool ChatHandler::HandleReloadAutobroadcastCommand(const char*)
{
    sWorld.LoadAutobroadcasts();
    SendGlobalGMSysMessage("DB table `autobroadcast` reloaded.");
    return true;
}

//reload commands
bool ChatHandler::HandleReloadCommand(const char* arg)
{
    // this is error catcher for wrong table name in .reload commands
    PSendSysMessage("Db table with name starting from '%s' not found and can't be reloaded.",arg);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleReloadAllCommand(const char*)
{
    HandleReloadAreaTriggerTeleportCommand("");
    HandleReloadSkillFishingBaseLevelCommand("");

    HandleReloadAllAreaCommand("");
    HandleReloadAllLootCommand("");
    HandleReloadAllNpcCommand("");
    HandleReloadAllQuestCommand("");
    HandleReloadAllSpellCommand("");
    HandleReloadAllItemCommand("");
    HandleReloadAllLocalesCommand("");

    HandleReloadCommandCommand("");
    HandleReloadReservedNameCommand("");
    HandleReloadHellgroundStringCommand("");
    HandleReloadGameTeleCommand("");
    HandleReloadUnqueuedAccountListCommand("");
    HandleReloadAutobroadcastCommand("");
    HandleReloadRevisionCommand("");

    return true;
}

bool ChatHandler::HandleReloadAllAreaCommand(const char*)
{
    //HandleReloadQuestAreaTriggersCommand(""); -- reloaded in HandleReloadAllQuestCommand
    HandleReloadAreaTriggerTeleportCommand("");
    HandleReloadAreaTriggerTavernCommand("");
    HandleReloadGameGraveyardZoneCommand("");
    return true;
}

bool ChatHandler::HandleReloadAllLootCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables...");
    LoadLootTables();
    SendGlobalGMSysMessage("DB tables `*_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllNpcCommand(const char* /*args*/)
{
    HandleReloadNpcGossipCommand("a");
    HandleReloadNpcOptionCommand("a");
    HandleReloadNpcTrainerCommand("a");
    HandleReloadNpcVendorCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllQuestCommand(const char* /*args*/)
{
    HandleReloadQuestAreaTriggersCommand("a");
    HandleReloadQuestTemplateCommand("a");

    sLog.outString("Re-Loading Quests Relations...");
    sObjectMgr.LoadQuestRelations();
    SendGlobalGMSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllScriptsCommand(const char*)
{
    if (sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    sLog.outString("Re-Loading Scripts...");
    HandleReloadGameObjectScriptsCommand("a");
    HandleReloadEventScriptsCommand("a");
    HandleReloadQuestEndScriptsCommand("a");
    HandleReloadQuestStartScriptsCommand("a");
    HandleReloadSpellScriptsCommand("a");
    SendGlobalGMSysMessage("DB tables `*_scripts` reloaded.");
    HandleReloadDbScriptStringCommand("a");
    HandleReloadWpScriptsCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllSpellCommand(const char*)
{
    HandleReloadSkillDiscoveryTemplateCommand("a");
    HandleReloadSkillExtraItemPrototypeCommand("a");
    HandleReloadSpellAffectCommand("a");
    HandleReloadSpellRequiredCommand("a");
    HandleReloadSpellElixirCommand("a");
    HandleReloadSpellLearnSpellCommand("a");
    HandleReloadSpellLinkedSpellCommand("a");
    HandleReloadSpellProcEventCommand("a");
    HandleReloadSpellScriptTargetCommand("a");
    HandleReloadSpellTargetPositionCommand("a");
    HandleReloadSpellThreatsCommand("a");
    HandleReloadSpellPetAurasCommand("a");
    HandleReloadSpellDisabledCommand("a");
    HandleReloadSpellAnalogsCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllItemCommand(const char*)
{
    HandleReloadPageTextsCommand("a");
    HandleReloadItemEnchantementsCommand("a");
    return true;
}

bool ChatHandler::HandleReloadAllLocalesCommand(const char* /*args*/)
{
    HandleReloadLocalesCreatureCommand("a");
    HandleReloadLocalesGameobjectCommand("a");
    HandleReloadLocalesItemCommand("a");
    HandleReloadLocalesNpcTextCommand("a");
    HandleReloadLocalesPageTextCommand("a");
    HandleReloadLocalesQuestCommand("a");
    return true;
}

bool ChatHandler::HandleReloadConfigCommand(const char* /*args*/)
{
    sLog.outString("Re-Loading config settings...");
    sWorld.LoadConfigSettings(true);
    sMapMgr.InitializeVisibilityDistanceInfo();
    SendGlobalGMSysMessage("World config settings reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTavernCommand(const char*)
{
    sLog.outString("Re-Loading Tavern Area Triggers...");
    sObjectMgr.LoadTavernAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_tavern` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTeleportCommand(const char*)
{
    sLog.outString("Re-Loading AreaTrigger teleport definitions...");
    sObjectMgr.LoadAreaTriggerTeleports();
    SendGlobalGMSysMessage("DB table `areatrigger_teleport` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAccessRequirementCommand(const char*)
{
    sLog.outString("Re-Loading Access Requirement definitions...");
    sObjectMgr.LoadAccessRequirements();
    SendGlobalGMSysMessage("DB table `access_requirement` reloaded.");
     return true;
 }

bool ChatHandler::HandleReloadCommandCommand(const char*)
{
    load_command_table = true;
    SendGlobalGMSysMessage("DB table `command` will be reloaded at next chat command use.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestRelationsCommand(const char*)
{
    sLog.outString("Loading Quests Relations... (`creature_questrelation`)");
    sObjectMgr.LoadCreatureQuestRelations();
    SendGlobalGMSysMessage("DB table `creature_questrelation` (creature quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureLinkedRespawnCommand(const char* args)
{
    sLog.outString("Loading Linked Respawns... (`creature_linked_respawn`)");
    sObjectMgr.LoadCreatureLinkedRespawn();
    SendGlobalGMSysMessage("DB table `creature_linked_respawn` (creature linked respawns) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadUnqueuedAccountListCommand(const char* args)
{
    sObjectMgr.LoadUnqueuedAccountList();
    SendGlobalGMSysMessage("DB table `unqueue_account` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestInvRelationsCommand(const char*)
{
    sLog.outString("Loading Quests Relations... (`creature_involvedrelation`)");
    sObjectMgr.LoadCreatureInvolvedRelations();
    SendGlobalGMSysMessage("DB table `creature_involvedrelation` (creature quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGOQuestRelationsCommand(const char*)
{
    sLog.outString("Loading Quests Relations... (`gameobject_questrelation`)");
    sObjectMgr.LoadGameobjectQuestRelations();
    SendGlobalGMSysMessage("DB table `gameobject_questrelation` (gameobject quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGOQuestInvRelationsCommand(const char*)
{
    sLog.outString("Loading Quests Relations... (`gameobject_involvedrelation`)");
    sObjectMgr.LoadGameobjectInvolvedRelations();
    SendGlobalGMSysMessage("DB table `gameobject_involvedrelation` (gameobject quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestAreaTriggersCommand(const char*)
{
    sLog.outString("Re-Loading Quest Area Triggers...");
    sObjectMgr.LoadQuestAreaTriggers();
    SendGlobalGMSysMessage("DB table `areatrigger_involvedrelation` (quest area triggers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestTemplateCommand(const char*)
{
    sLog.outString("Re-Loading Quest Templates...");
    sObjectMgr.LoadQuests();
    SendGlobalGMSysMessage("DB table `quest_template` (quest definitions) reloaded.");

    return true;
}

bool ChatHandler::HandleReloadLootTemplatesCreatureCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`creature_loot_template`)");
    LoadLootTemplates_Creature();
    LootTemplates_Creature.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `creature_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesDisenchantCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`disenchant_loot_template`)");
    LoadLootTemplates_Disenchant();
    LootTemplates_Disenchant.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `disenchant_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesFishingCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`fishing_loot_template`)");
    LoadLootTemplates_Fishing();
    LootTemplates_Fishing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `fishing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesGameobjectCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`gameobject_loot_template`)");
    LoadLootTemplates_Gameobject();
    LootTemplates_Gameobject.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `gameobject_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesItemCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`item_loot_template`)");
    LoadLootTemplates_Item();
    LootTemplates_Item.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `item_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesPickpocketingCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`pickpocketing_loot_template`)");
    LoadLootTemplates_Pickpocketing();
    LootTemplates_Pickpocketing.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `pickpocketing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesProspectingCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`prospecting_loot_template`)");
    LoadLootTemplates_Prospecting();
    LootTemplates_Prospecting.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `prospecting_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesQuestMailCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`quest_mail_loot_template`)");
    LoadLootTemplates_QuestMail();
    LootTemplates_QuestMail.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `quest_mail_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesReferenceCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`reference_loot_template`)");
    LoadLootTemplates_Reference();
    SendGlobalGMSysMessage("DB table `reference_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesSkinningCommand(const char*)
{
    sLog.outString("Re-Loading Loot Tables... (`skinning_loot_template`)");
    LoadLootTemplates_Skinning();
    LootTemplates_Skinning.CheckLootRefs();
    SendGlobalGMSysMessage("DB table `skinning_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadHellgroundStringCommand(const char*)
{
    sLog.outString("Re-Loading hellground_string Table!");
    sObjectMgr.LoadHellgroundStrings();
    SendGlobalGMSysMessage("DB table `hellground_string` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcOptionCommand(const char*)
{
    sLog.outString("Re-Loading `npc_option` Table!");
    sObjectMgr.LoadNpcOptions();
    SendGlobalGMSysMessage("DB table `npc_option` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcGossipCommand(const char*)
{
    sLog.outString("Re-Loading `npc_gossip` Table!");
    sObjectMgr.LoadNpcTextId();
    SendGlobalGMSysMessage("DB table `npc_gossip` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcTrainerCommand(const char*)
{
    sLog.outString("Re-Loading `npc_trainer` Table!");
    sObjectMgr.LoadTrainerSpell();
    SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcVendorCommand(const char*)
{
    sLog.outString("Re-Loading `npc_vendor` Table!");
    sObjectMgr.LoadVendors();
    sWorld.LoadCustomVendor();
    SendGlobalGMSysMessage("DB table `npc_vendor` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReservedNameCommand(const char*)
{
    sLog.outString("Loading ReservedNames... (`reserved_name`)");
    sObjectMgr.LoadReservedPlayersNames();
    SendGlobalGMSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReputationRewardRateCommand(const char*)
{
    sLog.outString("Re-Loading `reputation_reward_rate` Table!");
    sObjectMgr.LoadReputationRewardRate();
    SendGlobalGMSysMessage("DB table `reputation_reward_rate` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReputationSpilloverTemplateCommand(const char*)
{
    sLog.outString("Re-Loading `reputation_spillover_template` Table!");
    sObjectMgr.LoadReputationSpilloverTemplate();
    SendGlobalGMSysMessage("DB table `reputation_spillover_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillDiscoveryTemplateCommand(const char* /*args*/)
{
    sLog.outString("Re-Loading Skill Discovery Table...");
    LoadSkillDiscoveryTable();
    SendGlobalGMSysMessage("DB table `skill_discovery_template` (recipes discovered at crafting) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillExtraItemPrototypeCommand(const char* /*args*/)
{
    sLog.outString("Re-Loading Skill Extra Item Table...");
    LoadSkillExtraItemTable();
    SendGlobalGMSysMessage("DB table `skill_extra_item_template` (extra item creation when crafting) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillFishingBaseLevelCommand(const char* /*args*/)
{
    sLog.outString("Re-Loading Skill Fishing base level requirements...");
    sObjectMgr.LoadFishingBaseSkillLevel();
    SendGlobalGMSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellAffectCommand(const char*)
{
    sLog.outString("Re-Loading SpellAffect definitions...");
    sSpellMgr.LoadSpellAffects();
    SendGlobalGMSysMessage("DB table `spell_affect` (spell mods apply requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellRequiredCommand(const char*)
{
    sLog.outString("Re-Loading Spell Required Data... ");
    sSpellMgr.LoadSpellRequired();
    SendGlobalGMSysMessage("DB table `spell_required` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellElixirCommand(const char*)
{
    sLog.outString("Re-Loading Spell Elixir types...");
    sSpellMgr.LoadSpellElixirs();
    SendGlobalGMSysMessage("DB table `spell_elixir` (spell elixir types) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellLearnSpellCommand(const char*)
{
    sLog.outString("Re-Loading Spell Learn Spells...");
    sSpellMgr.LoadSpellLearnSpells();
    SendGlobalGMSysMessage("DB table `spell_learn_spell` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellLinkedSpellCommand(const char*)
{
    sLog.outString("Re-Loading Spell Linked Spells...");
    sSpellMgr.LoadSpellLinked();
    SendGlobalGMSysMessage("DB table `spell_linked_spell` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellProcEventCommand(const char*)
{
    sLog.outString("Re-Loading Spell Proc Event conditions...");
    sSpellMgr.LoadSpellProcEvents();
    SendGlobalGMSysMessage("DB table `spell_proc_event` (spell proc trigger requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellEnchantDataCommand(const char*)
{
    sLog.outString("Re-Loading Spell Enchant Proc Data conditions...");
    sSpellMgr.LoadSpellEnchantProcData();
    SendGlobalGMSysMessage("DB table `spell_enchant_proc_data` (spell enchant proc trigger requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellScriptTargetCommand(const char*)
{
    sLog.outString("Re-Loading SpellsScriptTarget...");
    sSpellMgr.LoadSpellScriptTarget();
    SendGlobalGMSysMessage("DB table `spell_script_target` (spell targets selection in case specific creature/GO requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadEventAIScriptsCommand(const char*)
{
    sLog.outString("Re-Loading creature_ai_scripts...");
    sCreatureEAIMgr.LoadCreatureEventAI_Scripts();
    SendGlobalGMSysMessage("DB table `creature_ai_scripts` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellTargetPositionCommand(const char*)
{
    sLog.outString("Re-Loading Spell target coordinates...");
    sSpellMgr.LoadSpellTargetPositions();
    SendGlobalGMSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellThreatsCommand(const char*)
{
    sLog.outString("Re-Loading Aggro Spells Definitions...");
    sSpellMgr.LoadSpellThreats();
    SendGlobalGMSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellPetAurasCommand(const char*)
{
    sLog.outString("Re-Loading Spell pet auras...");
    sSpellMgr.LoadSpellPetAuras();
    SendGlobalGMSysMessage("DB table `spell_pet_auras` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellAnalogsCommand(const char*)
{
    sLog.outString("Re-Loading Spell Analog Definitions...");
    sSpellMgr.LoadSpellAnalogs();
    SendGlobalGMSysMessage("DB table `spell_analog` (spell analog definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadPageTextsCommand(const char*)
{
    sLog.outString("Re-Loading Page Texts...");
    sObjectMgr.LoadPageTexts();
    SendGlobalGMSysMessage("DB table `page_texts` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadItemEnchantementsCommand(const char*)
{
    sLog.outString("Re-Loading Item Random Enchantments Table...");
    LoadRandomEnchantmentsTable();
    SendGlobalGMSysMessage("DB table `item_enchantment_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameObjectScriptsCommand(const char* arg)
{
    if (sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if (*arg!='a')
        sLog.outString("Re-Loading Scripts from `gameobject_scripts`...");

    sScriptMgr.LoadGameObjectScripts();

    if (*arg!='a')
        SendGlobalGMSysMessage("DB table `gameobject_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadEventScriptsCommand(const char* arg)
{
    if (sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if (*arg!='a')
        sLog.outString("Re-Loading Scripts from `event_scripts`...");

    sScriptMgr.LoadEventScripts();

    if (*arg!='a')
        SendGlobalGMSysMessage("DB table `event_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadWpScriptsCommand(const char* arg)
{
    if (sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if (*arg!='a')
        sLog.outString("Re-Loading Scripts from `waypoint_scripts`...");

    sScriptMgr.LoadWaypointScripts();

    if (*arg!='a')
        SendGlobalGMSysMessage("DB table `waypoint_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadRevisionCommand(const char*)
{
    sLog.outString("Re-Loading rev Table!");
    sObjectMgr.LoadRevision();
    SendGlobalGMSysMessage("DB table `rev` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestEndScriptsCommand(const char* arg)
{
    if (sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if (*arg!='a')
        sLog.outString("Re-Loading Scripts from `quest_end_scripts`...");

    sScriptMgr.LoadQuestEndScripts();

    if (*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_end_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadQuestStartScriptsCommand(const char* arg)
{
    if (sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if (*arg!='a')
        sLog.outString("Re-Loading Scripts from `quest_start_scripts`...");

    sScriptMgr.LoadQuestStartScripts();

    if (*arg!='a')
        SendGlobalGMSysMessage("DB table `quest_start_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadSpellScriptsCommand(const char* arg)
{
    if (sWorld.IsScriptScheduled())
    {
        SendSysMessage("DB scripts used currently, please attempt reload later.");
        SetSentErrorMessage(true);
        return false;
    }

    if (*arg!='a')
        sLog.outString("Re-Loading Scripts from `spell_scripts`...");

    sScriptMgr.LoadSpellScripts();

    if (*arg!='a')
        SendGlobalGMSysMessage("DB table `spell_scripts` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDbScriptStringCommand(const char* arg)
{
    sLog.outString("Re-Loading Script strings from `db_script_string`...");
    sScriptMgr.LoadDbScriptStrings();
    SendGlobalGMSysMessage("DB table `db_script_string` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGameGraveyardZoneCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Graveyard-zone links...");

    sObjectMgr.LoadGraveyardZones();

    SendGlobalGMSysMessage("DB table `game_graveyard_zone` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadGameTeleCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Game Tele coordinates...");

    sObjectMgr.LoadGameTele();

    SendGlobalGMSysMessage("DB table `game_tele` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadSpellDisabledCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading spell disabled table...");

    sObjectMgr.LoadSpellDisabledEntrys();

    SendGlobalGMSysMessage("DB table `spell_disabled` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadLocalesCreatureCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Locales Creature ...");
    sObjectMgr.LoadCreatureLocales();
    SendGlobalGMSysMessage("DB table `locales_creature` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesGameobjectCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Locales Gameobject ... ");
    sObjectMgr.LoadGameObjectLocales();
    SendGlobalGMSysMessage("DB table `locales_gameobject` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesItemCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Locales Item ... ");
    sObjectMgr.LoadItemLocales();
    SendGlobalGMSysMessage("DB table `locales_item` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesNpcTextCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Locales NPC Text ... ");
    sObjectMgr.LoadNpcTextLocales();
    SendGlobalGMSysMessage("DB table `locales_npc_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesPageTextCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Locales Page Text ... ");
    sObjectMgr.LoadPageTextLocales();
    SendGlobalGMSysMessage("DB table `locales_page_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesQuestCommand(const char* /*arg*/)
{
    sLog.outString("Re-Loading Locales Quest ... ");
    sObjectMgr.LoadQuestLocales();
    SendGlobalGMSysMessage("DB table `locales_quest` reloaded.");
    return true;
}

bool ChatHandler::HandleLoadScriptsCommand(const char* args)
{
     if (!sScriptMgr.LoadScriptLibrary())
         return true;

    sWorld.SendGMText(LANG_SCRIPTS_RELOADED);
    return true;
}

bool ChatHandler::HandleReloadAuctionsCommand(const char* args)
{
    ///- Reload dynamic data tables from the database
    sLog.outString("Re-Loading Auctions...");
    sAuctionMgr.LoadAuctionItems();
    sAuctionMgr.LoadAuctions();
    SendGlobalGMSysMessage("Auctions reloaded.");
    return true;
}

bool ChatHandler::HandleReloadShopCommand(const char*)
{
    sWorld.GetShop()->LoadShop();
    SendGlobalGMSysMessage("DB table `shop` and `shop_items` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadArenaRestrictionsCommand(const char*)
{
    sBattleGroundMgr.LoadArenaRestrictions();
    SendGlobalGMSysMessage("DB table `arena_restrictions_specs` and `arena_restrictions_items` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadWardenCommand(const char*)
{
    if (sWorld.GetActiveSessionCount() > 5) // not game realm. ok to use
    {
        SendSysMessage("You cant use this command on live server (more than 5 active players restriction)");
        SetSentErrorMessage(true);
        return false;
    }

    sWardenDataStorage.LoadWardenDataResult(true);
    SendGlobalGMSysMessage("DB table `warden_data_result` reloaded. Do not use this to DELETE checks or MODIFY them, only to ADD! Real people could get banned cause of that!");
    return true;
}

bool ChatHandler::HandleReloadMinorityPropagandaChar(const char*)
{
    sWorld.LoadMinorityPropagandaChar();
    SendGlobalGMSysMessage("MinorityPropagandaChar reloaded.");
    return true;
}

bool ChatHandler::HandleAccountSetPermissionsCommand(const char* args)
{
    return false;
    //
    //if (!*args)
    //    return false;

    //std::string targetAccountName;
    //uint32 targetAccountId = 0;
    //uint32 targetPermissions = 0;
    //uint32 gm = 0;
    //char* arg1 = strtok((char*)args, " ");
    //char* arg2 = strtok(NULL, " ");

    //if (getSelectedPlayer() && arg1 && !arg2)
    //{
    //    targetAccountId = getSelectedPlayer()->GetSession()->GetAccountId();
    //    AccountMgr::GetName(targetAccountId, targetAccountName);
    //    Player* targetPlayer = getSelectedPlayer();
    //    gm = atoi(arg1);

    //    // Check for invalid specified GM level.
    //    if (!(gm & PERM_ALL))
    //    {
    //        SendSysMessage(LANG_BAD_VALUE);
    //        SetSentErrorMessage(true);
    //        return false;
    //    }

    //    // Check if targets GM level and specified GM level is not higher than current gm level
    //    targetPermissions = targetPlayer->GetSession()->GetPermissions();
    //    if (targetPermissions >= m_session->GetPermissions() || gm >= m_session->GetPermissions())
    //    {
    //        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
    //        SetSentErrorMessage(true);
    //        return false;
    //    }

    //    // Decide which string to show
    //    if (m_session->GetPlayer() != targetPlayer)
    //        PSendSysMessage(LANG_YOU_CHANGE_SECURITY, targetAccountName.c_str(), gm);
    //    else
    //        PSendSysMessage(LANG_YOURS_SECURITY_CHANGED, m_session->GetPlayer()->GetName(), gm);

    //    AccountsDatabase.PExecute("UPDATE account_permissions SET permission_mask = '%u' WHERE account_id = '%u' AND realm_id = '%u'", gm, targetAccountId, realmID);
    //    return true;
    //}
    //else
    //{
    //    // Check for second parameter
    //    if (!arg2)
    //        return false;

    //    // Check for account
    //    targetAccountName = arg1;
    //    if (!AccountMgr::normalizeString(targetAccountName))
    //    {
    //        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, targetAccountName.c_str());
    //        SetSentErrorMessage(true);
    //        return false;
    //    }

    //    // Check for invalid specified GM level.
    //    gm = atoi(arg2);
    //    if (!(gm & PERM_ALL))
    //    {
    //        SendSysMessage(LANG_BAD_VALUE);
    //        SetSentErrorMessage(true);
    //        return false;
    //    }

    //    targetAccountId = AccountMgr::GetId(arg1);
    //    /// m_session==NULL only for console
    //    uint32 plSecurity = m_session ? m_session->GetPermissions() : PERM_CONSOLE;

    //    /// can set security level only for target with less security and to less security that we have
    //    /// This is also reject self apply in fact
    //    targetPermissions = AccountMgr::GetPermissions(targetAccountId);
    //    if (targetPermissions >= plSecurity || gm >= plSecurity)
    //    {
    //        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
    //        SetSentErrorMessage(true);
    //        return false;
    //    }

    //    PSendSysMessage(LANG_YOU_CHANGE_SECURITY, targetAccountName.c_str(), gm);
    //    AccountsDatabase.PExecute("UPDATE account_permissions SET permission_mask = '%u' WHERE account_id = '%u' AND realm_id = '%u'", gm, targetAccountId, realmID);
    //    return true;
    //}
}

// Set password for account
bool ChatHandler::HandleAccountSetPasswordCommand(const char* args)
{
    if (!*args)
        return false;

    ///- Get the command line arguments
    char *szAccount = strtok ((char*)args," ");
    char *szPassword1 =  strtok (NULL," ");
    char *szPassword2 =  strtok (NULL," ");

    if (!szAccount||!szPassword1 || !szPassword2)
        return false;

    std::string account_name = szAccount;
    if (!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetAccountId = AccountMgr::GetId(account_name);
    if (!targetAccountId)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 targetPermissions = AccountMgr::GetPermissions(targetAccountId);

    /// m_session==NULL only for console
    uint32 plPermiossions = m_session ? m_session->GetPermissions() : PERM_CONSOLE;

    /// can set password only for target with less security
    /// This is also reject self apply in fact
    if (targetPermissions >= plPermiossions)
    {
        SendSysMessage (LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage (true);
        return false;
    }

    if (strcmp(szPassword1,szPassword2))
    {
        SendSysMessage (LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage (true);
        return false;
    }

    AccountOpResult result = AccountMgr::ChangePassword(targetAccountId, szPassword1);

    switch (result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_NAME_NOT_EXIST:
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleAllowMovementCommand(const char* /*args*/)
{
    if (sWorld.getAllowMovement())
    {
        sWorld.SetAllowMovement(false);
        SendSysMessage(LANG_CREATURE_MOVE_DISABLED);
    }
    else
    {
        sWorld.SetAllowMovement(true);
        SendSysMessage(LANG_CREATURE_MOVE_ENABLED);
    }
    return true;
}

bool ChatHandler::HandleMaxSkillCommand(const char* /*args*/)
{
    Player* SelectedPlayer = getSelectedPlayer();
    if (!SelectedPlayer)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // each skills that have max skill value dependent from level seted to current level max skill value
    SelectedPlayer->SetWeaponSkillsToMax();
    return true;
}

bool ChatHandler::HandleSetSkillCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hskill:skill_id|h[name]|h|r
    char* skill_p = extractKeyFromLink((char*)args,"Hskill");
    if (!skill_p)
        return false;

    char *level_p = strtok (NULL, " ");

    if (!level_p)
        return false;

    char *max_p   = strtok (NULL, " ");

    int32 skill = atoi(skill_p);
    if (skill <= 0)
    {
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
        SetSentErrorMessage(true);
        return false;
    }

    int32 level = atol (level_p);

    Player * target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
    if (!sl)
    {
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
        SetSentErrorMessage(true);
        return false;
    }

    if (!target->GetSkillValue(skill))
    {
        PSendSysMessage(LANG_SET_SKILL_ERROR, target->GetName(), skill, sl->name[0]);
        SetSentErrorMessage(true);
        return false;
    }

    int32 max   = max_p ? atol (max_p) : target->GetPureMaxSkillValue(skill);

    if (level <= 0 || level > max || max <= 0)
        return false;

    target->SetSkill(skill, level, max);
    PSendSysMessage(LANG_SET_SKILL, skill, sl->name[0], target->GetName(), level, max);

    return true;
}

bool ChatHandler::HandleUnLearnCommand(const char* args)
{
    if (!*args)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    uint32 min_id = extractSpellIdFromLink((char*)args);
    if (!min_id)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    char* tail = strtok(NULL,"");

    uint32 max_id = extractSpellIdFromLink(tail);

    if (!max_id)
    {
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
        max_id =  min_id+1;
    }
    else
    {
        if (max_id < min_id)
            std::swap(min_id,max_id);

        max_id=max_id+1;
    }

    Player* target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    for (uint32 spell=min_id;spell<max_id;spell++)
    {
        if (target->HasSpell(spell))
            target->removeSpell(spell);
        else
            SendSysMessage(LANG_FORGET_SPELL);
    }

    return true;
}

bool ChatHandler::HandleCooldownCommand(const char* args)
{
    Player* target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (!*args)
    {
        target->RemoveAllSpellCooldown();
        PSendSysMessage(LANG_REMOVEALL_COOLDOWN, target->GetName());
    }
    else
    {
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spell_id = extractSpellIdFromLink((char*)args);
        if (!spell_id)
            return false;

        if (!sSpellTemplate.LookupEntry<SpellEntry>(spell_id))
        {
            PSendSysMessage(LANG_UNKNOWN_SPELL, target==m_session->GetPlayer() ? GetHellgroundString(LANG_YOU) : target->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        target->RemoveSpellCooldown(spell_id,true);
        PSendSysMessage(LANG_REMOVE_COOLDOWN, spell_id, target==m_session->GetPlayer() ? GetHellgroundString(LANG_YOU) : target->GetName());
    }
    return true;
}

bool ChatHandler::HandleLearnAllCommand(const char* /*args*/)
{
    static const char *allSpellList[] =
    {
        "3365",
        "6233",
        "6247",
        "6246",
        "6477",
        "6478",
        "22810",
        "8386",
        "21651",
        "21652",
        "522",
        "7266",
        "8597",
        "2479",
        "22027",
        "6603",
        "5019",
        "133",
        "168",
        "227",
        "5009",
        "9078",
        "668",
        "203",
        "20599",
        "20600",
        "81",
        "20597",
        "20598",
        "20864",
        "1459",
        "5504",
        "587",
        "5143",
        "118",
        "5505",
        "597",
        "604",
        "1449",
        "1460",
        "2855",
        "1008",
        "475",
        "5506",
        "1463",
        "12824",
        "8437",
        "990",
        "5145",
        "8450",
        "1461",
        "759",
        "8494",
        "8455",
        "8438",
        "6127",
        "8416",
        "6129",
        "8451",
        "8495",
        "8439",
        "3552",
        "8417",
        "10138",
        "12825",
        "10169",
        "10156",
        "10144",
        "10191",
        "10201",
        "10211",
        "10053",
        "10173",
        "10139",
        "10145",
        "10192",
        "10170",
        "10202",
        "10054",
        "10174",
        "10193",
        "12826",
        "2136",
        "143",
        "145",
        "2137",
        "2120",
        "3140",
        "543",
        "2138",
        "2948",
        "8400",
        "2121",
        "8444",
        "8412",
        "8457",
        "8401",
        "8422",
        "8445",
        "8402",
        "8413",
        "8458",
        "8423",
        "8446",
        "10148",
        "10197",
        "10205",
        "10149",
        "10215",
        "10223",
        "10206",
        "10199",
        "10150",
        "10216",
        "10207",
        "10225",
        "10151",
        "116",
        "205",
        "7300",
        "122",
        "837",
        "10",
        "7301",
        "7322",
        "6143",
        "120",
        "865",
        "8406",
        "6141",
        "7302",
        "8461",
        "8407",
        "8492",
        "8427",
        "8408",
        "6131",
        "7320",
        "10159",
        "8462",
        "10185",
        "10179",
        "10160",
        "10180",
        "10219",
        "10186",
        "10177",
        "10230",
        "10181",
        "10161",
        "10187",
        "10220",
        "2018",
        "2663",
        "12260",
        "2660",
        "3115",
        "3326",
        "2665",
        "3116",
        "2738",
        "3293",
        "2661",
        "3319",
        "2662",
        "9983",
        "8880",
        "2737",
        "2739",
        "7408",
        "3320",
        "2666",
        "3323",
        "3324",
        "3294",
        "22723",
        "23219",
        "23220",
        "23221",
        "23228",
        "23338",
        "10788",
        "10790",
        "5611",
        "5016",
        "5609",
        "2060",
        "10963",
        "10964",
        "10965",
        "22593",
        "22594",
        "596",
        "996",
        "499",
        "768",
        "17002",
        "1448",
        "1082",
        "16979",
        "1079",
        "5215",
        "20484",
        "5221",
        "15590",
        "17007",
        "6795",
        "6807",
        "5487",
        "1446",
        "1066",
        "5421",
        "3139",
        "779",
        "6811",
        "6808",
        "1445",
        "5216",
        "1737",
        "5222",
        "5217",
        "1432",
        "6812",
        "9492",
        "5210",
        "3030",
        "1441",
        "783",
        "6801",
        "20739",
        "8944",
        "9491",
        "22569",
        "5226",
        "6786",
        "1433",
        "8973",
        "1828",
        "9495",
        "9006",
        "6794",
        "8993",
        "5203",
        "16914",
        "6784",
        "9635",
        "22830",
        "20722",
        "9748",
        "6790",
        "9753",
        "9493",
        "9752",
        "9831",
        "9825",
        "9822",
        "5204",
        "5401",
        "22831",
        "6793",
        "9845",
        "17401",
        "9882",
        "9868",
        "20749",
        "9893",
        "9899",
        "9895",
        "9832",
        "9902",
        "9909",
        "22832",
        "9828",
        "9851",
        "9883",
        "9869",
        "17406",
        "17402",
        "9914",
        "20750",
        "9897",
        "9848",
        "3127",
        "107",
        "204",
        "9116",
        "2457",
        "78",
        "18848",
        "331",
        "403",
        "2098",
        "1752",
        "11278",
        "11288",
        "11284",
        "6461",
        "2344",
        "2345",
        "6463",
        "2346",
        "2352",
        "775",
        "1434",
        "1612",
        "71",
        "2468",
        "2458",
        "2467",
        "7164",
        "7178",
        "7367",
        "7376",
        "7381",
        "21156",
        "5209",
        "3029",
        "5201",
        "9849",
        "9850",
        "20719",
        "22568",
        "22827",
        "22828",
        "22829",
        "6809",
        "8972",
        "9005",
        "9823",
        "9827",
        "6783",
        "9913",
        "6785",
        "6787",
        "9866",
        "9867",
        "9894",
        "9896",
        "6800",
        "8992",
        "9829",
        "9830",
        "780",
        "769",
        "6749",
        "6750",
        "9755",
        "9754",
        "9908",
        "20745",
        "20742",
        "20747",
        "20748",
        "9746",
        "9745",
        "9880",
        "9881",
        "5391",
        "842",
        "3025",
        "3031",
        "3287",
        "3329",
        "1945",
        "3559",
        "4933",
        "4934",
        "4935",
        "4936",
        "5142",
        "5390",
        "5392",
        "5404",
        "5420",
        "6405",
        "7293",
        "7965",
        "8041",
        "8153",
        "9033",
        "9034",
        //"9036", problems with ghost state
        "16421",
        "21653",
        "22660",
        "5225",
        "9846",
        "2426",
        "5916",
        "6634",
        //"6718", phasing stealth, annoying for learn all case.
        "6719",
        "8822",
        "9591",
        "9590",
        "10032",
        "17746",
        "17747",
        "8203",
        "11392",
        "12495",
        "16380",
        "23452",
        "4079",
        "4996",
        "4997",
        "4998",
        "4999",
        "5000",
        "6348",
        "6349",
        "6481",
        "6482",
        "6483",
        "6484",
        "11362",
        "11410",
        "11409",
        "12510",
        "12509",
        "12885",
        "13142",
        "21463",
        "23460",
        "11421",
        "11416",
        "11418",
        "1851",
        "10059",
        "11423",
        "11417",
        "11422",
        "11419",
        "11424",
        "11420",
        "27",
        "31",
        "33",
        "34",
        "35",
        "15125",
        "21127",
        "22950",
        "1180",
        "201",
        "12593",
        "12842",
        "16770",
        "6057",
        "12051",
        "18468",
        "12606",
        "12605",
        "18466",
        "12502",
        "12043",
        "15060",
        "12042",
        "12341",
        "12848",
        "12344",
        "12353",
        "18460",
        "11366",
        "12350",
        "12352",
        "13043",
        "11368",
        "11113",
        "12400",
        "11129",
        "16766",
        "12573",
        "15053",
        "12580",
        "12475",
        "12472",
        "12953",
        "12488",
        "11189",
        "12985",
        "12519",
        "16758",
        "11958",
        "12490",
        "11426",
        "3565",
        "3562",
        "18960",
        "3567",
        "3561",
        "3566",
        "3563",
        "1953",
        "2139",
        "12505",
        "13018",
        "12522",
        "12523",
        "5146",
        "5144",
        "5148",
        "8419",
        "8418",
        "10213",
        "10212",
        "10157",
        "12524",
        "13019",
        "12525",
        "13020",
        "12526",
        "13021",
        "18809",
        "13031",
        "13032",
        "13033",
        "4036",
        "3920",
        "3919",
        "3918",
        "7430",
        "3922",
        "3923",
        "7411",
        "7418",
        "7421",
        "13262",
        "7412",
        "7415",
        "7413",
        "7416",
        "13920",
        "13921",
        "7745",
        "7779",
        "7428",
        "7457",
        "7857",
        "7748",
        "7426",
        "13421",
        "7454",
        "13378",
        "7788",
        "14807",
        "14293",
        "7795",
        "6296",
        "20608",
        "755",
        "444",
        "427",
        "428",
        "442",
        "447",
        "3578",
        "3581",
        "19027",
        "3580",
        "665",
        "3579",
        "3577",
        "6755",
        "3576",
        "2575",
        "2577",
        "2578",
        "2579",
        "2580",
        "2656",
        "2657",
        "2576",
        "3564",
        "10248",
        "8388",
        "2659",
        "14891",
        "3308",
        "3307",
        "10097",
        "2658",
        "3569",
        "16153",
        "3304",
        "10098",
        "4037",
        "3929",
        "3931",
        "3926",
        "3924",
        "3930",
        "3977",
        "3925",
        "136",
        "228",
        "5487",
        "43",
        "202",
        "0"
    };

    int loop = 0;
    while (strcmp(allSpellList[loop], "0"))
    {
        uint32 spell = atol((char*)allSpellList[loop++]);

        if (m_session->GetPlayer()->HasSpell(spell))
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
        {
            PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
            continue;
        }

        m_session->GetPlayer()->learnSpell(spell);
    }

    SendSysMessage(LANG_COMMAND_LEARN_MANY_SPELLS);

    return true;
}

bool ChatHandler::HandleLearnAllGMCommand(const char* /*args*/)
{
    static const char *gmSpellList[] =
    {
        "24347",                                            // Become A Fish, No Breath Bar
        "35132",                                            // Visual Boom
        "38488",                                            // Attack 4000-8000 AOE
        "38795",                                            // Attack 2000 AOE + Slow Down 90%
        "15712",                                            // Attack 200
        "1852",                                             // GM Spell Silence
        "31899",                                            // Kill
        "31924",                                            // Kill
        "29878",                                            // Kill My Self
        "26644",                                            // More Kill

        "28550",                                            //Invisible 24
        "23452",                                            //Invisible + Target
        "0"
    };

    uint16 gmSpellIter = 0;
    while (strcmp(gmSpellList[gmSpellIter], "0"))
    {
        uint32 spell = atol((char*)gmSpellList[gmSpellIter++]);

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
        {
            PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
            continue;
        }

        m_session->GetPlayer()->learnSpell(spell);
    }

    SendSysMessage(LANG_LEARNING_GM_SKILLS);
    return true;
}

bool ChatHandler::HandleLearnAllMyClassCommand(const char* /*args*/)
{
    HandleLearnAllMySpellsCommand("");
    HandleLearnAllMyTalentsCommand("");
    return true;
}

bool ChatHandler::HandleLearnAllMySpellsCommand(const char* /*args*/)
{
    ChrClassesEntry const* clsEntry = sChrClassesStore.LookupEntry(m_session->GetPlayer()->GetClass());
    if (!clsEntry)
        return true;
    uint32 family = clsEntry->spellfamily;

    for (uint32 i = 0; i < sSpellTemplate.GetMaxEntry(); ++i)
    {
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(i);
        if (!spellInfo)
            continue;

        // skip server-side/triggered spells
        if(spellInfo->spellLevel==0)
            continue;

        // skip wrong class/race skills
        if (!m_session->GetPlayer()->IsSpellFitByClassAndRace(spellInfo->Id))
            continue;

        // skip other spell families
        if (spellInfo->SpellFamilyName != family)
            continue;

        // skip spells with first rank learned as talent (and all talents then also)
        uint32 first_rank = sSpellMgr.GetFirstSpellInChain(spellInfo->Id);
        if (GetTalentSpellCost(first_rank) > 0)
            continue;

        // skip broken spells
        if (!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
            continue;

        m_session->GetPlayer()->learnSpell(i);
    }

    SendSysMessage(LANG_COMMAND_LEARN_CLASS_SPELLS);
    return true;
}

static void learnAllHighRanks(Player* player, uint32 spellid)
{
    SpellChainNode const* node = nullptr;
    do
    {
        node = sSpellMgr.GetSpellChainNode(spellid);
        player->learnSpell(spellid);
        if (!node)
            break;
        spellid=node->next;
    }
    while (node->next);
}

bool ChatHandler::HandleLearnAllMyTalentsCommand(const char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    uint32 classMask = player->GetClassMask();

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); i++)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        // search highest talent rank
        uint32 spellid = 0;
        int rank = 4;
        for (; rank >= 0; --rank)
        {
            if (talentInfo->RankID[rank]!=0)
            {
                spellid = talentInfo->RankID[rank];
                break;
            }
        }

        if (!spellid)                                        // ??? none spells in talent
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer(),false))
            continue;

        // learn highest rank of talent
        player->learnSpell(spellid);

        // and learn all non-talent spell ranks (recursive by tree)
        learnAllHighRanks(player,spellid);
    }

    SendSysMessage(LANG_COMMAND_LEARN_CLASS_TALENTS);
    return true;
}

bool ChatHandler::HandleLearnAllLangCommand(const char* /*args*/)
{
    // skipping UNIVERSAL language (0)
    for (int i = 1; i < LANGUAGES_COUNT; ++i)
        m_session->GetPlayer()->learnSpell(lang_description[i].spell_id);

    SendSysMessage(LANG_COMMAND_LEARN_ALL_LANG);
    return true;
}

bool ChatHandler::HandleLearnAllDefaultCommand(const char* args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;

        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectMgr.GetPlayerInWorld(name.c_str());
    }
    else
        player = getSelectedPlayer();

    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    player->learnDefaultSpells();
    player->learnQuestRewardedSpells();

    PSendSysMessage(LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST,player->GetName());
    return true;
}

bool ChatHandler::HandleLearnCommand(const char* args)
{
    Player* targetPlayer = getSelectedPlayer();

    if (!targetPlayer)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if (!spell || !sSpellTemplate.LookupEntry<SpellEntry>(spell))
        return false;

    if (targetPlayer->HasSpell(spell))
    {
        if (targetPlayer == m_session->GetPlayer())
            SendSysMessage(LANG_YOU_KNOWN_SPELL);
        else
            PSendSysMessage(LANG_TARGET_KNOWN_SPELL,targetPlayer->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    targetPlayer->learnSpell(spell);

    return true;
}

bool ChatHandler::HandleAddItemCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 itemId = 0;

    if (args[0]=='[')                                        // [name] manual form
    {
        char* citemName = citemName = strtok((char*)args, "]");

        if (citemName && citemName[0])
        {
            std::string itemName = citemName+1;
            GameDataDatabase.escape_string(itemName);
            QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
            if (!result)
            {
                PSendSysMessage(LANG_COMMAND_COULDNOTFIND, citemName+1);
                SetSentErrorMessage(true);
                return false;
            }
            itemId = result->Fetch()->GetUInt16();
        }
        else
            return false;
    }
    else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
    {
        char* cId = extractKeyFromLink((char*)args,"Hitem");
        if (!cId)
            return false;
        itemId = atol(cId);
    }

    char* ccount = strtok(NULL, " ");

    int32 count = 1;

    if (ccount)
        count = strtol(ccount, NULL, 10);

    if (count == 0)
        count = 1;

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if (!plTarget)
        plTarget = pl;

    sLog.outDetail(GetHellgroundString(LANG_ADDITEM), itemId, count);

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemId);
    if (!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    //Subtract
    if (count < 0)
    {
        plTarget->DestroyItemCount(itemId, -count, true, false, "GM_COMMAND");
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, plTarget->GetName());
        return true;
    }

    //Adding items
    uint32 noSpaceForCount = 0;

    // check space and find places
    ItemPosCountVec dest;
    uint8 msg = plTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK)                               // convert to possible store amount
        count -= noSpaceForCount;

    if (count == 0 || dest.empty())                         // can't add any
    {
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
        SetSentErrorMessage(true);
        return false;
    }

    Item* item = plTarget->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId), "GM_COMMAND");

    if (count > 0 && item)
    {
        pl->SendNewItem(item,count,false,true);
        if (pl!=plTarget)
            plTarget->SendNewItem(item,count,true,false);
    }

    if (noSpaceForCount > 0)
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

    return true;
}

bool ChatHandler::HandleAddItemSetCommand(const char* args)
{
    if (!*args)
        return false;

    char* cId = extractKeyFromLink((char*)args,"Hitemset"); // number or [name] Shift-click form |color|Hitemset:itemset_id|h[name]|h|r
    if (!cId)
        return false;

    uint32 itemsetId = atol(cId);

    // prevent generation all items with itemset field value '0'
    if (itemsetId == 0)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);
        SetSentErrorMessage(true);
        return false;
    }

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if (!plTarget)
        plTarget = pl;

    sLog.outDetail(GetHellgroundString(LANG_ADDITEMSET), itemsetId);

    bool found = false;
    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); id++)
    {
        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype>(id);
        if (!pProto)
            continue;

        if (pProto->ItemSet == itemsetId)
        {
            found = true;
            ItemPosCountVec dest;
            uint8 msg = plTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pProto->ItemId, 1);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = plTarget->StoreNewItem(dest, pProto->ItemId, true, 0, "GM_COMMAND");

                // remove binding (let GM give it to another player later)
                if (pl==plTarget)
                    item->SetBinding(false);

                pl->SendNewItem(item,1,false,true);
                if (pl!=plTarget)
                    plTarget->SendNewItem(item,1,true,false);
            }
            else
            {
                pl->SendEquipError(msg, NULL, NULL);
                PSendSysMessage(LANG_ITEM_CANNOT_CREATE, pProto->ItemId, 1);
            }
        }
    }

    if (!found)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND,itemsetId);

        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleListItemCommand(const char* args)
{
    if (!*args)
        return false;

    char* cId = extractKeyFromLink((char*)args,"Hitem");
    if (!cId)
        return false;

    uint32 item_id = atol(cId);
    if (!item_id)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype(item_id);
    if (!itemProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(NULL, " ");
    int count = c_count ? atol(c_count) : 10;

    if (count < 0)
        return false;

    QueryResultAutoPtr result;

    // inventory case
    uint32 inv_count = 0;
    result=RealmDataDatabase.PQuery("SELECT COUNT(item_template) FROM character_inventory WHERE item_template='%u'",item_id);
    if (result)
        inv_count = (*result)[0].GetUInt32();

    result=RealmDataDatabase.PQuery(
    //          0        1             2             3        4                  5
        "SELECT ci.item, cibag.slot AS bag, ci.slot, ci.guid, characters.account,characters.name "
        "FROM character_inventory AS ci LEFT JOIN character_inventory AS cibag ON (cibag.item=ci.bag),characters "
        "WHERE ci.item_template='%u' AND ci.guid = characters.guid LIMIT %u ",
        item_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid = fields[0].GetUInt32();
            uint32 item_bag = fields[1].GetUInt32();
            uint32 item_slot = fields[2].GetUInt32();
            uint32 owner_guid = fields[3].GetUInt32();
            uint32 owner_acc = fields[4].GetUInt32();
            std::string owner_name = fields[5].GetCppString();

            char const* item_pos = 0;
            if (Player::IsEquipmentPos(item_bag,item_slot))
                item_pos = "[equipped]";
            else if (Player::IsInventoryPos(item_bag,item_slot))
                item_pos = "[in inventory]";
            else if (Player::IsBankPos(item_bag,item_slot))
                item_pos = "[in bank]";
            else
                item_pos = "";

            PSendSysMessage(LANG_ITEMLIST_SLOT,
                item_guid,owner_name.c_str(),owner_guid,owner_acc,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if (count > res_count)
            count-=res_count;
        else if (count)
            count = 0;
    }

    // mail case
    uint32 mail_count = 0;
    result=RealmDataDatabase.PQuery("SELECT COUNT(item_template) FROM mail_items WHERE item_template='%u'", item_id);
    if (result)
        mail_count = (*result)[0].GetUInt32();

    if (count > 0)
    {
        result=RealmDataDatabase.PQuery(
        //          0                     1            2              3               4            5               6
            "SELECT mail_items.item_guid, mail.sender, mail.receiver, char_s.account, char_s.name, char_r.account, char_r.name "
            "FROM mail,mail_items,characters as char_s,characters as char_r "
            "WHERE mail_items.item_template='%u' AND char_s.guid = mail.sender AND char_r.guid = mail.receiver AND mail.id=mail_items.mail_id LIMIT %u",
            item_id,uint32(count));
    }
    else
        result = QueryResultAutoPtr(NULL);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid        = fields[0].GetUInt32();
            uint32 item_s           = fields[1].GetUInt32();
            uint32 item_r           = fields[2].GetUInt32();
            uint32 item_s_acc       = fields[3].GetUInt32();
            std::string item_s_name = fields[4].GetCppString();
            uint32 item_r_acc       = fields[5].GetUInt32();
            std::string item_r_name = fields[6].GetCppString();

            char const* item_pos = "[in mail]";

            PSendSysMessage(LANG_ITEMLIST_MAIL,
                item_guid,item_s_name.c_str(),item_s,item_s_acc,item_r_name.c_str(),item_r,item_r_acc,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if (count > res_count)
            count-=res_count;
        else if (count)
            count = 0;
    }

    // auction case
    uint32 auc_count = 0;
    result = RealmDataDatabase.PQuery("SELECT COUNT(item_template) FROM auctionhouse WHERE item_template='%u'",item_id);
    if (result)
        auc_count = (*result)[0].GetUInt32();

    if (count > 0)
    {
        result=RealmDataDatabase.PQuery(
        //           0                      1                       2                   3
            "SELECT  auctionhouse.itemguid, auctionhouse.itemowner, characters.account, characters.name "
            "FROM auctionhouse,characters WHERE auctionhouse.item_template='%u' AND characters.guid = auctionhouse.itemowner LIMIT %u",
            item_id,uint32(count));
    }
    else
        result = QueryResultAutoPtr(NULL);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid       = fields[0].GetUInt32();
            uint32 owner           = fields[1].GetUInt32();
            uint32 owner_acc       = fields[2].GetUInt32();
            std::string owner_name = fields[3].GetCppString();

            char const* item_pos = "[in auction]";

            PSendSysMessage(LANG_ITEMLIST_AUCTION, item_guid, owner_name.c_str(), owner, owner_acc,item_pos);
        } while (result->NextRow());
    }

    // guild bank case
    uint32 guild_count = 0;
    result=RealmDataDatabase.PQuery("SELECT COUNT(item_entry) FROM guild_bank_item WHERE item_entry='%u'",item_id);
    if (result)
        guild_count = (*result)[0].GetUInt32();

    result=RealmDataDatabase.PQuery(
        //      0             1           2
        "SELECT gi.item_guid, gi.guildid, guild.name "
        "FROM guild_bank_item AS gi, guild WHERE gi.item_entry='%u' AND gi.guildid = guild.guildid LIMIT %u ",
        item_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 item_guid = fields[0].GetUInt32();
            uint32 guild_guid = fields[1].GetUInt32();
            std::string guild_name = fields[2].GetCppString();

            char const* item_pos = "[in guild bank]";

            PSendSysMessage(LANG_ITEMLIST_GUILD,item_guid,guild_name.c_str(),guild_guid,item_pos);
        } while (result->NextRow());

        int64 res_count = result->GetRowCount();

        if (count > res_count)
            count-=res_count;
        else if (count)
            count = 0;
    }

    if (inv_count+mail_count+auc_count+guild_count == 0)
    {
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_COMMAND_LISTITEMMESSAGE,item_id,inv_count+mail_count+auc_count+guild_count,inv_count,mail_count,auc_count,guild_count);

    return true;
}

bool ChatHandler::HandleListObjectCommand(const char* args)
{
    if (!*args)
        return false;

    // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hgameobject_entry");
    if (!cId)
        return false;

    uint32 go_id = atol(cId);
    if (!go_id)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    GameObjectInfo const * gInfo = ObjectMgr::GetGameObjectInfo(go_id);
    if (!gInfo)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(NULL, " ");
    int count = c_count ? atol(c_count) : 10;

    if (count < 0)
        return false;

    QueryResultAutoPtr result;

    uint32 obj_count = 0;
    result=GameDataDatabase.PQuery("SELECT COUNT(guid) FROM gameobject WHERE id='%u'",go_id);
    if (result)
        obj_count = (*result)[0].GetUInt32();

    if (m_session)
    {
        Player* pl = m_session->GetPlayer();
        result = GameDataDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE id = '%u' ORDER BY order_ ASC LIMIT %u",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),go_id,uint32(count));
    }
    else
        result = GameDataDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map FROM gameobject WHERE id = '%u' LIMIT %u",
            go_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            int mapid = fields[4].GetUInt16();

            if (m_session)
                PSendSysMessage(LANG_GO_LIST_CHAT, guid, guid, gInfo->name, x, y, z, mapid);
            else
                PSendSysMessage(LANG_GO_LIST_CONSOLE, guid, gInfo->name, x, y, z, mapid);
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_LISTOBJMESSAGE,go_id,obj_count);
    return true;
}

bool ChatHandler::HandleGameObjectNearCommand(const char* args)
{
    float distance = (!*args) ? 10 : atol(args);
    uint32 count = 0;

    Player* pl = m_session->GetPlayer();
    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, map, "
        "(POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ "
        "FROM gameobject WHERE map='%u' AND (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) <= '%f' ORDER BY order_",
        pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),
        pl->GetMapId(),pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(),distance*distance);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            float x = fields[2].GetFloat();
            float y = fields[3].GetFloat();
            float z = fields[4].GetFloat();
            int mapid = fields[5].GetUInt16();

            GameObjectInfo const * gInfo = ObjectMgr::GetGameObjectInfo(entry);

            if (!gInfo)
                continue;

            PSendSysMessage(LANG_GO_LIST_CHAT, guid, guid, gInfo->name, x, y, z, mapid);
            GameObject * tmp = pl->GetMap()->GetGameObject(MAKE_NEW_GUID(guid, entry, HIGHGUID_GAMEOBJECT));
            PSendSysMessage("Is in Map Guid Store: %s, Is in world: %s", tmp ? "Yes" : "No", tmp ? (tmp->IsInWorld() ? "Yes" : "No") : "No!");

            ++count;
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE,distance,count);
    return true;
}

bool ChatHandler::HandleGameObjectStateCommand(const char* args)
{
    // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args, "Hgameobject");
    if (!cId)
        return false;

    uint32 lowguid = atoi(cId);
    if (!lowguid)
        return false;

    GameObject* gobj = NULL;

    if (GameObjectData const* goData = sObjectMgr.GetGOData(lowguid))
        gobj = GetObjectGlobalyWithGuidOrNearWithDbGuid(lowguid, goData->id);

    if (!gobj)
    {
        PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, lowguid);
        SetSentErrorMessage(true);
        return false;
    }

    char* cstate = strtok(NULL, " ");
    if (!cstate)
        return false;

    int32 state = atoi(cstate);
    if (state < 0)
        gobj->SendObjectDeSpawnAnim(gobj->GetGUID());
    else
        gobj->SetGoState((GOState)state);

    return true;
}

bool ChatHandler::HandleListCreatureCommand(const char* args)
{
    if (!*args)
        return false;

    // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hcreature_entry");
    if (!cId)
        return false;

    uint32 cr_id = atol(cId);
    if (!cr_id)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cr_id);
    if (!cInfo)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    char* c_count = strtok(NULL, " ");
    int count = c_count ? atol(c_count) : 10;

    if (count < 0)
        return false;

    QueryResultAutoPtr result;

    uint32 cr_count = 0;
    result=GameDataDatabase.PQuery("SELECT COUNT(guid) FROM creature WHERE id='%u'",cr_id);
    if (result)
        cr_count = (*result)[0].GetUInt32();

    if (m_session)
    {
        Player* pl = m_session->GetPlayer();
        result = GameDataDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM creature WHERE id = '%u' ORDER BY order_ ASC LIMIT %u",
            pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), cr_id,uint32(count));
    }
    else
        result = GameDataDatabase.PQuery("SELECT guid, position_x, position_y, position_z, map FROM creature WHERE id = '%u' LIMIT %u",
            cr_id,uint32(count));

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            int mapid = fields[4].GetUInt16();

            if  (m_session)
                PSendSysMessage(LANG_CREATURE_LIST_CHAT, guid, guid, cInfo->Name, x, y, z, mapid);
            else
                PSendSysMessage(LANG_CREATURE_LIST_CONSOLE, guid, cInfo->Name, x, y, z, mapid);
        } while (result->NextRow());
    }

    PSendSysMessage(LANG_COMMAND_LISTCREATUREMESSAGE,cr_id,cr_count);
    return true;
}

bool ChatHandler::HandleLookupItemCommand(const char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    Player* target = getSelectedPlayer();

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    // Search in `item_template`
    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); ++id)
    {
        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype >(id);
        if (!pProto)
            continue;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : sObjectMgr.GetDBCLocaleIndex();

        std::string name;                                   // "" for let later only single time check default locale name directly
        sObjectMgr.GetItemLocaleStrings(id, loc_idx, &name);
        if ((name.empty() || !Utf8FitTo(name, wnamepart)) && !Utf8FitTo(pProto->Name1, wnamepart))
            continue;

        ItemPrototype const *itemProto = sItemStorage.LookupEntry<ItemPrototype >(id);
        if(!itemProto)
            continue;
        
        std::string nameItem = itemProto->Name1;
        sObjectMgr.GetItemLocaleStrings(itemProto->ItemId, loc_idx, &nameItem);

        if (m_session)
        {
            char chr[256] = "";
            char chrcount[32] = "";
            if (target)
            {
                uint32 overall = target->GetItemCount(id, true);
                uint32 withself = target->GetItemCount(id);
                if (target->HasItemCount(id, 1, true))
                    sprintf(chrcount, "[has %u] [has %u in bank]", overall, overall - withself);//, usableStr);
            }
            sprintf(chr, GetHellgroundString(LANG_ITEM_LIST_CHAT), id, id, nameItem.c_str());//, usableStr);

            char chrSay[256] = "";
            sprintf(chrSay, "%s%s", chr, chrcount);
            SendSysMessage(chrSay);
        }
        else
            PSendSysMessage(LANG_ITEM_LIST_CONSOLE, id, nameItem.c_str());//, usableStr);

        ++counter;
    }

    if (!counter)
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);

    return true;
}

bool ChatHandler::HandleLookupItemSetCommand(const char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in ItemSet.dbc
    for (uint32 id = 0; id < sItemSetStore.GetNumRows(); id++)
    {
        ItemSetEntry const *set = sItemSetStore.LookupEntry(id);
        if (set)
        {
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = set->name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (m_session && loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = set->name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                // send item set in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_ITEMSET_LIST_CHAT,id,id,name.c_str(),localeNames[loc]);
                else
                    PSendSysMessage(LANG_ITEMSET_LIST_CONSOLE,id,name.c_str(),localeNames[loc]);
                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOITEMSETFOUND);
    return true;
}

bool ChatHandler::HandleLookupSkillCommand(const char* args)
{
    if (!*args)
        return false;

    // can be NULL in console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in SkillLine.dbc
    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); id++)
    {
        SkillLineEntry const *skillInfo = sSkillLineStore.LookupEntry(id);
        if (skillInfo)
        {
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = skillInfo->name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (m_session && loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = skillInfo->name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                char const* knownStr = "";
                if (target && target->HasSkill(id))
                    knownStr = GetHellgroundString(LANG_KNOWN);

                // send skill in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_SKILL_LIST_CHAT,id,id,name.c_str(),localeNames[loc],knownStr);
                else
                    PSendSysMessage(LANG_SKILL_LIST_CONSOLE,id,name.c_str(),localeNames[loc],knownStr);

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSKILLFOUND);
    return true;
}

bool ChatHandler::HandleLookupSpellCommand(const char* args)
{
    if (!*args)
        return false;

    // can be NULL at console call
    Unit* target = getSelectedUnit();

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in Spell.dbc
    for (uint32 id = 0; id < sSpellTemplate.GetMaxEntry(); ++id)
    {
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(id);
        if (spellInfo)
        {
            int loc = m_session ? m_session->GetSessionDbcLocale() : sWorld.GetDefaultDbcLocale();
            std::string name = spellInfo->SpellName[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (m_session && loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = spellInfo->SpellName[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                bool known = target && target->HasSpell(id);
                bool learn = (spellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL);

                uint32 talentCost = GetTalentSpellCost(id);

                bool talent = (talentCost > 0);
                bool passive = SpellMgr::IsPassiveSpell(id);
                bool active = target && target->HasAura(id);

                // unit32 used to prevent interpreting uint8 as char at output
                // find rank of learned spell for learning spell, or talent rank
                uint32 rank = talentCost ? talentCost : sSpellMgr.GetSpellRank(learn ? spellInfo->EffectTriggerSpell[0] : id);

                // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
                std::ostringstream ss;
                if (m_session)
                    ss << id << " - |cffffffff|Hspell:" << id << "|h[" << name;
                else
                    ss << id << " - " << name;

                // include rank in link name
                if (rank)
                    ss << GetHellgroundString(LANG_SPELL_RANK) << rank;

                if (m_session)
                    ss << " " << localeNames[loc] << "]|h|r";
                else
                    ss << " " << localeNames[loc];

                if (talent)
                    ss << GetHellgroundString(LANG_TALENT);
                if (passive)
                    ss << GetHellgroundString(LANG_PASSIVE);
                if (learn)
                    ss << GetHellgroundString(LANG_LEARN);
                if (known)
                    ss << GetHellgroundString(LANG_KNOWN);
                if (active)
                    ss << GetHellgroundString(LANG_ACTIVE);

                SendSysMessage(ss.str().c_str());

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSPELLFOUND);
    return true;
}

bool ChatHandler::HandleLookupQuestCommand(const char* args)
{
    if (!*args)
        return false;

    // can be NULL at console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0 ;

    ObjectMgr::QuestMap const& qTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator iter = qTemplates.begin(); iter != qTemplates.end(); ++iter)
    {
        Quest * qinfo = iter->second;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : sObjectMgr.GetDBCLocaleIndex();
        if (loc_idx >= 0)
        {
            QuestLocale const *il = sObjectMgr.GetQuestLocale(qinfo->GetQuestId());
            if (il)
            {
                if (il->Title.size() > loc_idx && !il->Title[loc_idx].empty())
                {
                    std::string name = il->Title[loc_idx];

                    if (Utf8FitTo(name, wnamepart))
                    {
                        char const* statusStr = "";

                        if (target)
                        {
                            QuestStatus status = target->GetQuestStatus(qinfo->GetQuestId());

                            if (status == QUEST_STATUS_COMPLETE)
                            {
                                if (target->GetQuestRewardStatus(qinfo->GetQuestId()))
                                    statusStr = GetHellgroundString(LANG_COMMAND_QUEST_REWARDED);
                                else
                                    statusStr = GetHellgroundString(LANG_COMMAND_QUEST_COMPLETE);
                            }
                            else if (status == QUEST_STATUS_INCOMPLETE)
                                statusStr = GetHellgroundString(LANG_COMMAND_QUEST_ACTIVE);
                        }

                        if (m_session)
                            PSendSysMessage(LANG_QUEST_LIST_CHAT,qinfo->GetQuestId(),qinfo->GetQuestId(),name.c_str(),statusStr);
                        else
                            PSendSysMessage(LANG_QUEST_LIST_CONSOLE,qinfo->GetQuestId(),name.c_str(),statusStr);
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = qinfo->GetName();
        if (name.empty())
            continue;

        if (Utf8FitTo(name, wnamepart))
        {
            char const* statusStr = "";

            if (target)
            {
                QuestStatus status = target->GetQuestStatus(qinfo->GetQuestId());

                if (status == QUEST_STATUS_COMPLETE)
                {
                    if (target->GetQuestRewardStatus(qinfo->GetQuestId()))
                        statusStr = GetHellgroundString(LANG_COMMAND_QUEST_REWARDED);
                    else
                        statusStr = GetHellgroundString(LANG_COMMAND_QUEST_COMPLETE);
                }
                else if (status == QUEST_STATUS_INCOMPLETE)
                    statusStr = GetHellgroundString(LANG_COMMAND_QUEST_ACTIVE);
            }

            if (m_session)
                PSendSysMessage(LANG_QUEST_LIST_CHAT,qinfo->GetQuestId(),qinfo->GetQuestId(),name.c_str(),statusStr);
            else
                PSendSysMessage(LANG_QUEST_LIST_CONSOLE,qinfo->GetQuestId(),name.c_str(),statusStr);

            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_COMMAND_NOQUESTFOUND);

    return true;
}

bool ChatHandler::HandleLookupCreatureCommand(const char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr (namepart,wnamepart))
        return false;

    wstrToLower (wnamepart);

    uint32 counter = 0;

    for (uint32 id = 0; id< sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo> (id);
        if (!cInfo)
            continue;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : sObjectMgr.GetDBCLocaleIndex();

        char const* name = "";                              // "" for avoid repeating check for default locale
        sObjectMgr.GetCreatureLocaleStrings(id, loc_idx, &name);
        if (!*name || !Utf8FitTo(name, wnamepart))
        {
            name = cInfo->Name;
            if (!Utf8FitTo(name, wnamepart))
                continue;
        }

        if (m_session)
            PSendSysMessage ("%d - |cffffffff|Hcreature_entry:%d|h[%s]|h|r - level %u, hp %u", id, id, name, cInfo->maxlevel, cInfo->maxhealth);
        else
            PSendSysMessage (LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name);
        ++counter;
    }

    if (!counter)
        SendSysMessage (LANG_COMMAND_NOCREATUREFOUND);

    return true;
}

bool ChatHandler::HandleLookupObjectCommand(const char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart,wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    for (uint32 id = 0; id< sGOStorage.GetMaxEntry(); id++)
    {
        GameObjectInfo const* gInfo = sGOStorage.LookupEntry<GameObjectInfo>(id);
        if (!gInfo)
            continue;

        int loc_idx = m_session ? m_session->GetSessionDbLocaleIndex() : sObjectMgr.GetDBCLocaleIndex();
        if (loc_idx >= 0)
        {
            GameObjectLocale const *gl = sObjectMgr.GetGameObjectLocale(id);
            if (gl)
            {
                if (gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                {
                    std::string name = gl->Name[loc_idx];

                    if (Utf8FitTo(name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, id, id, name.c_str());
                        else
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, id, name.c_str());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = gInfo->name;
        if (name.empty())
            continue;

        if (Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, id, id, name.c_str());
            else
                PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, id, name.c_str());
            ++counter;
        }
    }

    if (counter==0)
        SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);

    return true;
}

/** \brief GM command level 3 - Create a guild.
 *
 * This command allows a GM (level 3) to create a guild.
 *
 * The "args" parameter contains the name of the guild leader
 * and then the name of the guild.
 *
 */
bool ChatHandler::HandleGuildRenameCommand(const char* args)
{
	if (!*args)
		return false;

    std::string arg1, arg2;
    std::istringstream iss(args);

    if (iss.ignore(1) && std::getline(iss, arg1, '"') && iss.ignore(1)) {
        iss.ignore(1) && std::getline(iss, arg2, '"');
    }

	std::string glName = arg1;
	Guild* targetGuild = sGuildMgr.GetGuildByName(glName);
	if (!targetGuild)
	{
		SendSysMessage("No guild found");
		SetSentErrorMessage(true);
		return false;
	}

	std::string glNewName = arg2;
	if (sGuildMgr.GetGuildByName(glNewName))
	{
		SendSysMessage("Guild with new name is already exists");
		SetSentErrorMessage(true);
		return false;
	}

	GameDataDatabase.escape_string(glNewName);
	RealmDataDatabase.PExecute("UPDATE guild SET name='%s' WHERE guildid='%u'", glNewName.c_str(), targetGuild->GetId());
	SendSysMessage("Success! New guild name will be set after server restart.");
	return true;
}

bool ChatHandler::HandleGuildCreateCommand(const char* args)
{
    if (!*args)
        return false;

    char *lname = strtok ((char*)args, " ");
    char *gname = strtok (NULL, "");

    if (!lname)
        return false;

    if (!gname)
    {
        SendSysMessage (LANG_INSERT_GUILD_NAME);
        SetSentErrorMessage (true);
        return false;
    }

    std::string guildname = gname;

    Player* player = sObjectAccessor.GetPlayerByNameInWorld(lname);
    if (!player)
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    if (player->GetGuildId())
    {
        SendSysMessage (LANG_PLAYER_IN_GUILD);
        return true;
    }

    Guild *guild = new Guild;
    if (!guild->create (player->GetGUID (),guildname))
    {
        delete guild;
        SendSysMessage (LANG_GUILD_NOT_CREATED);
        SetSentErrorMessage (true);
        return false;
    }

    sGuildMgr.AddGuild (guild);
    return true;
}

bool ChatHandler::HandleGuildInviteCommand(const char *args)
{
    if (!*args)
        return false;

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok (NULL, "");
    if (!par1 || !par2)
        return false;

    std::string glName = par2;
    Guild* targetGuild = sGuildMgr.GetGuildByName (glName);
    if (!targetGuild)
        return false;

    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    uint64 plGuid = 0;
    if (Player* targetPlayer = sObjectAccessor.GetPlayerByNameInWorld(plName))
        plGuid = targetPlayer->GetGUID ();
    else
        plGuid = sObjectMgr.GetPlayerGUIDByName (plName.c_str ());

    if (!plGuid)
        return false;

    // player's guild membership checked in AddMember before add
    if (!targetGuild->AddMember (plGuid,targetGuild->GetLowestRank ()))
        return false;

    return true;
}

bool ChatHandler::HandleGuildUninviteCommand(const char *args)
{
    if (!*args)
        return false;

    char* par1 = strtok ((char*)args, " ");
    if (!par1)
        return false;

    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    uint64 plGuid = 0;
    uint32 glId   = 0;
    if (Player* targetPlayer = sObjectAccessor.GetPlayerByNameInWorld(plName))
    {
        plGuid = targetPlayer->GetGUID ();
        glId   = targetPlayer->GetGuildId ();
    }
    else
    {
        plGuid = sObjectMgr.GetPlayerGUIDByName (plName.c_str ());
        glId = Player::GetGuildIdFromDB (plGuid);
    }

    if (!plGuid || !glId)
        return false;

    Guild* targetGuild = sGuildMgr.GetGuildById (glId);
    if (!targetGuild)
        return false;

    targetGuild->DelMember (plGuid);

    return true;
}

bool ChatHandler::HandleGuildRankCommand(const char *args)
{
    if (!*args)
        return false;

    char* par1 = strtok ((char*)args, " ");
    char* par2 = strtok (NULL, " ");
    if (!par1 || !par2)
        return false;
    std::string plName = par1;
    if (!normalizePlayerName (plName))
    {
        SendSysMessage (LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage (true);
        return false;
    }

    uint64 plGuid = 0;
    uint32 glId   = 0;
    if (Player* targetPlayer = sObjectAccessor.GetPlayerByNameInWorld(plName))
    {
        plGuid = targetPlayer->GetGUID ();
        glId   = targetPlayer->GetGuildId ();
    }
    else
    {
        plGuid = sObjectMgr.GetPlayerGUIDByName (plName.c_str ());
        glId = Player::GetGuildIdFromDB (plGuid);
    }

    if (!plGuid || !glId)
        return false;

    Guild* targetGuild = sGuildMgr.GetGuildById (glId);
    if (!targetGuild)
        return false;

    uint32 newrank = uint32 (atoi (par2));
    if (newrank > targetGuild->GetLowestRank ())
        return false;

    targetGuild->ChangeRank (plGuid,newrank);

    return true;
}

bool ChatHandler::HandleGuildDeleteCommand(const char* args)
{
    if (!*args)
        return false;

    std::string gld = args;

    Guild* targetGuild = sGuildMgr.GetGuildByName(gld);
    if (!targetGuild)
        return false;

    targetGuild->Disband ();

    return true;
}

bool ChatHandler::HandleGetDistanceCommand(const char* /*args*/)
{
    Unit* pUnit = getSelectedUnit();

    if (!pUnit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

	PSendSysMessage("The distance is: (3D) %f (2D) %f yards", m_session->GetPlayer()->GetDistance(pUnit), m_session->GetPlayer()->GetDistance2d(pUnit));
    PSendSysMessage("Exact distance: %f", m_session->GetPlayer()->GetExactDist2d(pUnit->GetPositionX(), pUnit->GetPositionY()));
    PSendSysMessage("My object size %f, target %f", m_session->GetPlayer()->GetObjectSize(), pUnit->GetObjectSize());
    PSendSysMessage("Your orientation to unit: %f", m_session->GetPlayer()->GetOrientationTo(pUnit));
    PSendSysMessage("Unit orientation to you: %f", pUnit->GetOrientationTo(m_session->GetPlayer()));
    PSendSysMessage("Your angle to target: %f", MapManager::NormalizeOrientation(m_session->GetPlayer()->GetAngleTo(pUnit)));
    PSendSysMessage("Target angle to you: %f", MapManager::NormalizeOrientation(pUnit->GetAngleTo(m_session->GetPlayer())));
    return true;
}

// FIX-ME!!!

bool ChatHandler::HandleAddWeaponCommand(const char* /*args*/)
{
    /*if (!*args)
        return false;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SendSysMessage(LANG_NO_SELECTION);
        return true;
    }

    Creature *pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(), guid);

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    char* pSlotID = strtok((char*)args, " ");
    if (!pSlotID)
        return false;

    char* pItemID = strtok(NULL, " ");
    if (!pItemID)
        return false;

    uint32 ItemID = atoi(pItemID);
    uint32 SlotID = atoi(pSlotID);

    ItemPrototype* tmpItem = ObjectMgr::GetItemPrototype(ItemID);

    bool added = false;
    if (tmpItem)
    {
        switch (SlotID)
        {
            case 1:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, ItemID);
                added = true;
                break;
            case 2:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, ItemID);
                added = true;
                break;
            case 3:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, ItemID);
                added = true;
                break;
            default:
                PSendSysMessage(LANG_ITEM_SLOT_NOT_EXIST,SlotID);
                added = false;
                break;
        }
        if (added)
        {
            PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT,ItemID,tmpItem->Name1,SlotID);
        }
    }
    else
    {
        PSendSysMessage(LANG_ITEM_NOT_FOUND,ItemID);
        return true;
    }
    */
    return true;
}

bool ChatHandler::HandleDieCommand(const char* /*args*/)
{
    Unit* target = getSelectedUnit();

    if (!target || !m_session->GetPlayer()->GetSelection())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (target->isAlive())
    {
        //m_session->GetPlayer()->DealDamage(target, target->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        m_session->GetPlayer()->Kill(target);
    }

    return true;
}

bool ChatHandler::HandleDamageCommand(const char * args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();

    if (!target || !m_session->GetPlayer()->GetSelection())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!target->isAlive())
        return true;

    char* damageStr = strtok((char*)args, " ");
    if (!damageStr)
        return false;

    int32 damage = atoi((char*)damageStr);
    if (damage <=0)
        return true;

    char* schoolStr = strtok((char*)NULL, " ");

    // flat melee damage without resistence/etc reduction
    if (!schoolStr)
    {
        m_session->GetPlayer()->DealDamage(target, damage, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        m_session->GetPlayer()->SendAttackStateUpdate (HITINFO_NORMALSWING2, target, 1, SPELL_SCHOOL_MASK_NORMAL, damage, 0, 0, VICTIMSTATE_NORMAL, 0);
        return true;
    }

    uint32 school = schoolStr ? atoi((char*)schoolStr) : SPELL_SCHOOL_NORMAL;
    if (school >= MAX_SPELL_SCHOOL)
        return false;

    SpellSchoolMask schoolmask = SpellSchoolMask(1 << school);

    if (schoolmask & SPELL_SCHOOL_MASK_NORMAL)
        damage = m_session->GetPlayer()->CalcArmorReducedDamage(target, damage);

    char* spellStr = strtok((char*)NULL, " ");

    // melee damage by specific school
    if (!spellStr)
    {
        uint32 absorb = 0;
        uint32 resist = 0;

        m_session->GetPlayer()->CalcAbsorbResist(target,schoolmask, SPELL_DIRECT_DAMAGE, damage, &absorb, &resist);

        if (damage <= absorb + resist)
            return true;

        damage -= absorb + resist;

        m_session->GetPlayer()->DealDamage(target, damage, DIRECT_DAMAGE, schoolmask, NULL, false);
        m_session->GetPlayer()->SendAttackStateUpdate (HITINFO_NORMALSWING2, target, 1, schoolmask, damage, absorb, resist, VICTIMSTATE_NORMAL, 0);
        return true;
    }

    // non-melee damage

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spellid = extractSpellIdFromLink((char*)args);
    if (!spellid || !sSpellTemplate.LookupEntry<SpellEntry>(spellid))
        return false;

    m_session->GetPlayer()->SpellNonMeleeDamageLog(target, spellid, damage, false);
    return true;
}

bool ChatHandler::HandleModifyArenaCommand(const char * args)
{
    if (!*args)
        return false;

    Player *target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = (uint32)atoi(args);

    target->ModifyArenaPoints(amount);

    PSendSysMessage(LANG_COMMAND_MODIFY_ARENA, target->GetName(), target->GetArenaPoints());

    return true;
}

bool ChatHandler::HandleReviveCommand(const char* args)
{
    Player* SelectedPlayer = NULL;

    //if (*args)
    //{
    //    std::string name = args;
    //    if (!normalizePlayerName(name))
    //    {
    //        SendSysMessage(LANG_PLAYER_NOT_FOUND);
    //        SetSentErrorMessage(true);
    //        return false;
    //    }

    //    SelectedPlayer = sObjectMgr.GetPlayerInWorld(name.c_str());
    //}
    //else
    SelectedPlayer = getSelectedPlayer();

    if (!SelectedPlayer)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    SelectedPlayer->ResurrectPlayer(0.5f);
    SelectedPlayer->SpawnCorpseBones();
    SelectedPlayer->SaveToDB();
    return true;
}

bool ChatHandler::HandleReviveGroupCommand(const char* args)
{
    Player* gm = m_session->GetPlayer();

    if (!gm)
        return false;

    Group * tmpG = gm->GetGroup();

    if (!tmpG)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP, gm->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    for (GroupReference *itr = tmpG->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();

        if (!pl || pl->isAlive() || pl->GetMapId() != gm->GetMapId())
            continue;

        // before GM
        float x,y,z;
        gm->GetNearPoint(x, y, z, pl->GetObjectSize());
        pl->TeleportTo(gm->GetMapId(), x, y, z, pl->GetOrientation());

        pl->ResurrectPlayer(0.5f);
        pl->SpawnCorpseBones();
        pl->SaveToDB();
    }

    return true;
}

bool ChatHandler::HandleAuraCommand(const char* args)
{
    Unit *target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }
    char* spellname = strtok((char*)args, " ");
    char* duration = strtok(NULL, " ");
    uint32 spellID = extractSpellIdFromLink((char*)spellname);
    
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellID);
    if (spellInfo)
    {
        for (uint32 i = 0;i<3;i++)
        {
            uint8 eff = spellInfo->Effect[i];
            if (eff>=TOTAL_SPELL_EFFECTS)
                continue;
            if (SpellMgr::IsAreaAuraEffect(eff)           ||
                eff == SPELL_EFFECT_APPLY_AURA  ||
                eff == SPELL_EFFECT_PERSISTENT_AREA_AURA || target->GetDummyAura(54839))
            {
                Aura *Aur = CreateAura(spellInfo, i, NULL, target);
                target->AddAura(Aur);
                if (duration != NULL)
                {
                    uint32 num_duration = atoi(duration);
                    Aur->SetAuraDuration(num_duration);
                    Aur->UpdateAuraDuration();
                }
            }
        }
    }

    return true;
}

bool ChatHandler::HandleUnAuraCommand(const char* args)
{
    Unit *target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    std::string argstr = args;
    if (argstr == "all")
    {
        target->RemoveAllAuras();
        return true;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spellID = extractSpellIdFromLink((char*)args);
    if(!spellID)
        return false;

    target->RemoveAurasDueToSpell(spellID);

    return true;
}

bool ChatHandler::HandleLinkGraveCommand(const char* args)
{
    if (!*args)
        return false;

    char* px = strtok((char*)args, " ");
    if (!px)
        return false;

    uint32 g_id = (uint32)atoi(px);

    PlayerTeam g_team;

    char* px2 = strtok(NULL, " ");

    if (!px2)
        g_team = TEAM_NONE;
    else if (strncmp(px2,"horde",6)==0)
        g_team = HORDE;
    else if (strncmp(px2,"alliance",9)==0)
        g_team = ALLIANCE;
    else
        return false;

    WorldSafeLocsEntry const* graveyard =  sWorldSafeLocsStore.LookupEntry(g_id);

    if (!graveyard)
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, g_id);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();

    uint32 zoneId = player->GetCachedZone();

    AreaTableEntry const *areaEntry = GetAreaEntryByAreaID(zoneId);
    if (!areaEntry || areaEntry->zone !=0)
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDWRONGZONE, g_id,zoneId);
        SetSentErrorMessage(true);
        return false;
    }

    if (sObjectMgr.AddGraveYardLink(g_id,zoneId,g_team))
        PSendSysMessage(LANG_COMMAND_GRAVEYARDLINKED, g_id,zoneId);
    else
        PSendSysMessage(LANG_COMMAND_GRAVEYARDALRLINKED, g_id,zoneId);

    return true;
}

bool ChatHandler::HandleNearGraveCommand(const char* args)
{
    PlayerTeam g_team;

    size_t argslen = strlen(args);

    if (!*args)
        g_team = TEAM_NONE;
    else if (strncmp((char*)args,"horde",argslen)==0)
        g_team = HORDE;
    else if (strncmp((char*)args,"alliance",argslen)==0)
        g_team = ALLIANCE;
    else
        return false;

    Player* player = m_session->GetPlayer();

    WorldSafeLocsEntry const* graveyard = sObjectMgr.GetClosestGraveYard(
        player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(),player->GetMapId(),g_team);

    if (graveyard)
    {
        uint32 g_id = graveyard->ID;

        GraveYardData const* data = sObjectMgr.FindGraveYardData(g_id,player->GetCachedZone());
        if (!data)
        {
            PSendSysMessage(LANG_COMMAND_GRAVEYARDERROR,g_id);
            SetSentErrorMessage(true);
            return false;
        }

        g_team = data->team;

        std::string team_name = GetHellgroundString(LANG_COMMAND_GRAVEYARD_NOTEAM);

        if (g_team == 0)
            team_name = GetHellgroundString(LANG_COMMAND_GRAVEYARD_ANY);
        else if (g_team == HORDE)
            team_name = GetHellgroundString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if (g_team == ALLIANCE)
            team_name = GetHellgroundString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

        PSendSysMessage(LANG_COMMAND_GRAVEYARDNEAREST, g_id,team_name.c_str(),player->GetCachedZone());
    }
    else
    {
        std::string team_name;

        if (g_team == 0)
            team_name = GetHellgroundString(LANG_COMMAND_GRAVEYARD_ANY);
        else if (g_team == HORDE)
            team_name = GetHellgroundString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if (g_team == ALLIANCE)
            team_name = GetHellgroundString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

        if (g_team == ~uint32(0))
            PSendSysMessage(LANG_COMMAND_ZONENOGRAVEYARDS, player->GetCachedZone());
        else
            PSendSysMessage(LANG_COMMAND_ZONENOGRAFACTION, player->GetCachedZone(),team_name.c_str());
    }

    return true;
}

//play npc emote
bool ChatHandler::HandleNpcPlayEmoteCommand(const char* args)
{
    uint32 emote = atoi((char*)args);

    Creature* target = getSelectedCreature();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->SetUInt32Value(UNIT_NPC_EMOTESTATE,emote);

    return true;
}

bool ChatHandler::HandleUnitInfoCommand(const char* args)
{
    Unit* target = getSelectedUnit();

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage("Run/Walk speed: %.2f %.2f", target->GetSpeed(MOVE_RUN), target->GetSpeed(MOVE_WALK));
    return true;
}

bool ChatHandler::HandleNpcInfoCommand(const char* args)
{
    Creature* target = getSelectedCreature();

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 faction = target->getFaction();
    uint32 npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    uint32 displayid = target->GetDisplayId();
    uint32 nativeid = target->GetNativeDisplayId();
    uint32 Entry = target->GetEntry();
    uint32 Gender = target->GetGender();
    CreatureInfo const* cInfo = target->GetCreatureInfo();

    int32 curRespawnDelay = target->GetRespawnTimeEx()-time(NULL);
    if (curRespawnDelay < 0)
        curRespawnDelay = 0;
    std::string curRespawnDelayStr = m_session->secondsToTimeString(curRespawnDelay,true);
    std::string defRespawnDelayStr = m_session->secondsToTimeString(target->GetRespawnDelay(),true);

    PSendSysMessage("GUID: %u, Entry: %u, DisplayID: %u (Native :%u)\nScriptname :%s", target->GetDBTableGUIDLow(), Entry, displayid, nativeid, target->GetScriptName().c_str());
    
    if (char* trig_str = strtok((char*)args, " "))
    {
        PSendSysMessage(LANG_NPCINFO_LEVEL, target->GetLevel());
        PSendSysMessage("Gender: %u", Gender);
        PSendSysMessage(LANG_NPCINFO_HEALTH, target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
        PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->getFaction());
        PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
        PSendSysMessage(LANG_NPCINFO_LOOT, cInfo->lootid, cInfo->pickpocketLootId, cInfo->SkinLootId);
        PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetAnyInstanceId());
        PSendSysMessage(LANG_NPCINFO_POSITION, float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()));
        if (const CreatureData* const linked = target->GetLinkedRespawnCreatureData())
            if (CreatureInfo const* master = GetCreatureInfo(linked->id))
                PSendSysMessage(LANG_NPCINFO_LINKGUID, sObjectMgr.GetLinkedRespawnGuid(target->GetDBTableGUIDLow()), linked->id, master->Name);

        if (npcflags & UNIT_NPC_FLAG_VENDOR)

            SendSysMessage(LANG_NPCINFO_VENDOR);

        if (npcflags & UNIT_NPC_FLAG_TRAINER)
            SendSysMessage(LANG_NPCINFO_TRAINER);

        PSendSysMessage("AVG damage: %f, max: %f", round(float(cInfo->mindmg + cInfo->maxdmg) * target->GetCreatureDamageMod() / 2.0f), cInfo->maxdmg * target->GetCreatureDamageMod());
        PSendSysMessage("creature: XpMOD %f, HPmod: %f, DMGmod: %f,", target->GetXPMod(), target->GetCreatureHealthMod(), target->GetCreatureDamageMod());
        PSendSysMessage("AIName: %s/%s, ScriptName: %s", target->GetAIName().c_str(), target->AI()->m_AIName.c_str(), target->GetScriptName().c_str());
        PSendSysMessage("DeadByDefault: %i", (int)target->GetIsDeadByDefault());

        CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.LookupEntry(displayid);
        if (!displayInfo)
            return true;

        CreatureModelDataEntry const* modelInfo = sCreatureModelDataStore.LookupEntry(displayInfo->ModelId);
        if (!modelInfo)
            return true;

        PSendSysMessage("Combat reach: %f, BoundingRadius: %f", target->GetFloatValue(UNIT_FIELD_COMBATREACH), target->GetFloatValue(UNIT_FIELD_BOUNDINGRADIUS));
        PSendSysMessage("Determinative Size: %f, CollisionWidth: %f, Scale DB: %f, Scale DBC: %f", target->GetDeterminativeSize(), modelInfo->CollisionWidth, cInfo->scale, displayInfo->scale);
        PSendSysMessage("Active flags: %x; Extra flags: 0x%08X", target->isActiveObject(), cInfo->flags_extra);
        PSendSysMessage("Loot recipient: %s; loot dmg requirement: %s", target->GetLootRecipient() ? target->GetLootRecipient()->GetName() : "<NULL>", target->IsDamageEnoughForLootingAndReward() ? "enough" : "not enough");
        SendSysMessage("Loot players to loot:");
        Loot* loot = &target->loot;
        std::string playername;
        for (std::set<uint64>::const_iterator itr = loot->players_allowed_to_loot.begin(); itr != loot->players_allowed_to_loot.end(); itr++)
        {
            if (sObjectMgr.GetPlayerNameByGUID(*itr, playername))
                PSendSysMessage("%s", playername.c_str());
        }
        SendSysMessage("Creature players to loot:");
        for (std::set<uint64>::const_iterator itr = target->m_PlayersAllowedToLoot.begin(); itr != target->m_PlayersAllowedToLoot.end(); itr++)
        {
            if (sObjectMgr.GetPlayerNameByGUID(*itr, playername))
                PSendSysMessage("%s", playername.c_str());
        }

        SendSysMessage("Loot accounts to loot:");
        //for (std::set<uint32>::const_iterator itr = loot->mother_accs_allowed_to_loot.begin(); itr != loot->mother_accs_allowed_to_loot.end(); itr++)
        //    PSendSysMessage("%u", *itr);
        //SendSysMessage("Creature accounts to loot:");
        //for (std::set<uint32>::const_iterator itr = target->m_MotherAccsAllowedToLoot.begin(); itr != target->m_MotherAccsAllowedToLoot.end(); itr++)
        //    PSendSysMessage("%u", *itr);
    }

    return true;
}

bool ChatHandler::HandleExploreCheatCommand(const char* args)
{
    if (!*args)
        return false;

    int flag = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (flag != 0)
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_ALL, chr->GetName());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_ALL,GetName());
    }
    else
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_NOTHING, chr->GetName());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_NOTHING,GetName());
    }

    for (uint8 i=0; i<128; i++)
    {
        if (flag != 0)
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0xFFFFFFFF);
        }
        else
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1+i,0);
        }
    }

    return true;
}

bool ChatHandler::HandleHoverCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    uint32 flag;
    if (!px)
        flag = 1;
    else
        flag = atoi(px);

    m_session->GetPlayer()->setHover(flag);

    if (flag)
        SendSysMessage(LANG_HOVER_ENABLED);
    else
        SendSysMessage(LANG_HOVER_DISABLED);

    return true;
}

bool ChatHandler::HandleWaterwalkCommand(const char* args)
{
    if (!args)
        return false;

    Player *player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (strncmp(args, "on", 3) == 0)
        player->SetMovement(MOVE_WATER_WALK);               // ON
    else if (strncmp(args, "off", 4) == 0)
        player->SetMovement(MOVE_LAND_WALK);                // OFF
    else
    {
        SendSysMessage(LANG_USE_BOL);
        return false;
    }

    PSendSysMessage(LANG_YOU_SET_WATERWALK, args, player->GetName());
    if (needReportToTarget(player))
        ChatHandler(player).PSendSysMessage(LANG_YOUR_WATERWALK_SET, args, GetName());
    return true;

}

bool ChatHandler::HandleLevelUpCommand(const char* args)
{
    char* px = strtok((char*)args, " ");
    char* py = strtok((char*)NULL, " ");

    // command format parsing
    char* pname = (char*)NULL;
    int addlevel = 1;

    if (px && py)                                            // .levelup name level
    {
        addlevel = atoi(py);
        pname = px;
    }
    else if (px && !py)                                      // .levelup name OR .levelup level
    {
        if (isalpha(px[0]))                                  // .levelup name
            pname = px;
        else                                                // .levelup level
            addlevel = atoi(px);
    }
    // else .levelup - nothing do for preparing

    // player
    Player *chr = NULL;
    uint64 chr_guid = 0;

    std::string name;

    if (pname)                                               // player by name
    {
        name = pname;
        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        chr = sObjectMgr.GetPlayerInWorld(name.c_str());
        if (!chr)                                            // not in game
        {
            chr_guid = sObjectMgr.GetPlayerGUIDByName(name);
            if (chr_guid == 0)
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
        }
    }
    else                                                    // player by selection
    {
        chr = getSelectedPlayer();

        if (chr == NULL)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            SetSentErrorMessage(true);
            return false;
        }

        name = chr->GetName();
    }

    ASSERT(chr || chr_guid);

    int32 oldlevel = chr ? chr->GetLevel() : Player::GetUInt32ValueFromDB(UNIT_FIELD_LEVEL,chr_guid);
    int32 newlevel = oldlevel + addlevel;
    if (newlevel < 1)
        newlevel = 1;
    if (newlevel > STRONG_MAX_LEVEL)                         // hardcoded maximum level
        newlevel = STRONG_MAX_LEVEL;

    if (chr)
    {
        chr->SetDifficulty(DIFFICULTY_NORMAL);
        chr->GiveLevel(newlevel);
        chr->InitTalentForLevel();
        chr->SetUInt32Value(PLAYER_XP,0);
        chr->LevelReached(newlevel);

        if (oldlevel == newlevel)
            ChatHandler(chr).SendSysMessage(LANG_YOURS_LEVEL_PROGRESS_RESET);
        else
        if (oldlevel < newlevel)
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_LEVEL_UP,newlevel-oldlevel);
        else
        if (oldlevel > newlevel)
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_LEVEL_DOWN,newlevel-oldlevel);
    }
    else
    {
        // update level and XP at level, all other will be updated at loading
        RealmDataDatabase.PExecute("UPDATE characters SET level = '%u', xp = 0 WHERE guid = '%llu'", newlevel, chr_guid);
    }

    if (m_session->GetPlayer() != chr)                       // including chr==NULL
        PSendSysMessage(LANG_YOU_CHANGE_LVL,name.c_str(),newlevel);
    return true;
}

bool ChatHandler::HandleShowAreaCommand(const char* args)
{
    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    if (offset >= 128)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

    SendSysMessage(LANG_EXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleHideAreaCommand(const char* args)
{
    if (!*args)
        return false;

    int area = atoi((char*)args);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    if (offset >= 128)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

    SendSysMessage(LANG_UNEXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleDebugUpdate(const char* args)
{
    if (!*args)
        return false;

    uint32 updateIndex;
    uint32 value;

    char* pUpdateIndex = strtok((char*)args, " ");

    Unit* chr = getSelectedUnit();
    if (chr == NULL)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!pUpdateIndex)
    {
        return true;
    }
    updateIndex = atoi(pUpdateIndex);
    //check updateIndex
    if (chr->GetTypeId() == TYPEID_PLAYER)
    {
        if (updateIndex>=PLAYER_END) return true;
    }
    else
    {
        if (updateIndex>=UNIT_END) return true;
    }

    char*  pvalue = strtok(NULL, " ");
    if (!pvalue)
    {
        value=chr->GetUInt32Value(updateIndex);

        PSendSysMessage(LANG_UPDATE, chr->GetGUIDLow(),updateIndex,value);
        return true;
    }

    value=atoi(pvalue);

    PSendSysMessage(LANG_UPDATE_CHANGE, chr->GetGUIDLow(),updateIndex,value);

    chr->SetUInt32Value(updateIndex,value);

    return true;
}

bool ChatHandler::HandleBankCommand(const char* /*args*/)
{
    m_session->SendShowBank(m_session->GetPlayer()->GetGUID());

    return true;
}

bool ChatHandler::HandleChangeWeather(const char* args)
{
    if (!*args)
        return false;

    //Weather is OFF
    if (!sWorld.getConfig(CONFIG_WEATHER))
    {
        SendSysMessage(LANG_WEATHER_DISABLED);
        SetSentErrorMessage(true);
        return false;
    }

    //*Change the weather of a cell
    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 type = (uint32)atoi(px);                         //0 to 3, 0: fine, 1: rain, 2: snow, 3: sand
    float grade = (float)atof(py);                          //0 to 1, sending -1 is instand good weather

    Player *player = m_session->GetPlayer();
    uint32 zoneid = player->GetCachedZone();

    Weather* wth = sWorld.FindWeather(zoneid);

    if (!wth)
        wth = sWorld.AddWeather(zoneid);
    if (!wth)
    {
        SendSysMessage(LANG_NO_WEATHER);
        SetSentErrorMessage(true);
        return false;
    }

    wth->SetWeather(WeatherType(type), grade);

    return true;
}

bool ChatHandler::HandleDebugSetValue(const char* args)
{
    if (!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");

    if (!px || !py)
        return false;

    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = target->GetGUID();

    uint32 Opcode = (uint32)atoi(px);
    if (Opcode >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint32 iValue;
    float fValue;
    bool isint32 = true;
    if (pz)
        isint32 = (bool)atoi(pz);
    if (isint32)
    {
        iValue = (uint32)atoi(py);
        sLog.outDebug(GetHellgroundString(LANG_SET_UINT), GUID_LOPART(guid), Opcode, iValue);
        target->SetUInt32Value(Opcode , iValue);
        PSendSysMessage(LANG_SET_UINT_FIELD, GUID_LOPART(guid), Opcode,iValue);
    }
    else
    {
        fValue = (float)atof(py);
        sLog.outDebug(GetHellgroundString(LANG_SET_FLOAT), GUID_LOPART(guid), Opcode, fValue);
        target->SetFloatValue(Opcode , fValue);
        PSendSysMessage(LANG_SET_FLOAT_FIELD, GUID_LOPART(guid), Opcode,fValue);
    }

    return true;
}

bool ChatHandler::HandleDebugGetValue(const char* args)
{
    if (!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* pz = strtok(NULL, " ");

    if (!px)
        return false;

    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 guid = target->GetGUID();
    if (strcmp(px, "movement") == 0)
    {
        PSendSysMessage("Target's movement flags: 0x%08X",target->GetUnitMovementFlags());
        return true;
    }

    uint32 Opcode = (uint32)atoi(px);
    if (Opcode >= target->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, GUID_LOPART(guid), target->GetValuesCount());
        return false;
    }
    uint32 iValue;
    float fValue;
    bool isint32 = true;
    if (pz)
        isint32 = (bool)atoi(pz);

    if (isint32)
    {
        iValue = target->GetUInt32Value(Opcode);
        sLog.outDebug(GetHellgroundString(LANG_GET_UINT), GUID_LOPART(guid), Opcode, iValue);
        PSendSysMessage(LANG_GET_UINT_FIELD, GUID_LOPART(guid), Opcode,    iValue);
    }
    else
    {
        fValue = target->GetFloatValue(Opcode);
        sLog.outDebug(GetHellgroundString(LANG_GET_FLOAT), GUID_LOPART(guid), Opcode, fValue);
        PSendSysMessage(LANG_GET_FLOAT_FIELD, GUID_LOPART(guid), Opcode, fValue);
    }

    return true;
}

bool ChatHandler::HandleSet32Bit(const char* args)
{
    if (!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 Opcode = (uint32)atoi(px);
    uint32 Value = (uint32)atoi(py);
    if (Value > 32)                                         //uint32 = 32 bits
        return false;

    sLog.outDebug(GetHellgroundString(LANG_SET_32BIT), Opcode, Value);

    m_session->GetPlayer()->SetUInt32Value(Opcode , 2^Value);

    PSendSysMessage(LANG_SET_32BIT_FIELD, Opcode,1);
    return true;
}

bool ChatHandler::HandleDebugMod32Value(const char* args)
{
    if (!*args)
        return false;

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");

    if (!px || !py)
        return false;

    uint32 Opcode = (uint32)atoi(px);
    int Value = atoi(py);

    if (Opcode >= m_session->GetPlayer()->GetValuesCount())
    {
        PSendSysMessage(LANG_TOO_BIG_INDEX, Opcode, m_session->GetPlayer()->GetGUIDLow(), m_session->GetPlayer()->GetValuesCount());
        return false;
    }

    sLog.outDebug(GetHellgroundString(LANG_CHANGE_32BIT), Opcode, Value);

    int CurrentValue = (int)m_session->GetPlayer()->GetUInt32Value(Opcode);

    CurrentValue += Value;
    m_session->GetPlayer()->SetUInt32Value(Opcode , (uint32)CurrentValue);

    PSendSysMessage(LANG_CHANGE_32BIT_FIELD, Opcode,CurrentValue);

    return true;
}

bool ChatHandler::HandleTeleAddCommand(const char * args)
{
    if (!*args)
        return false;

    Player *player=m_session->GetPlayer();
    if (!player)
        return false;

    std::string name = args;

    if (sObjectMgr.GetGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TP_ALREADYEXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameTele tele;
    tele.position_x  = player->GetPositionX();
    tele.position_y  = player->GetPositionY();
    tele.position_z  = player->GetPositionZ();
    tele.orientation = player->GetOrientation();
    tele.mapId       = player->GetMapId();
    tele.name        = name;

    if (sObjectMgr.AddGameTele(tele))
    {
        SendSysMessage(LANG_COMMAND_TP_ADDED);
    }
    else
    {
        SendSysMessage(LANG_COMMAND_TP_ADDEDERR);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleTeleDelCommand(const char * args)
{
    if (!*args)
        return false;

    std::string name = args;

    if (!sObjectMgr.DeleteGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    SendSysMessage(LANG_COMMAND_TP_DELETED);
    return true;
}

bool ChatHandler::HandleListAurasCommand (const char *args)
{
    Unit *unit = getSelectedUnit();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    char const* talentStr = GetHellgroundString(LANG_TALENT);
    char const* passiveStr = GetHellgroundString(LANG_PASSIVE);

    Unit::AuraMap const& uAuras = unit->GetAuras();
    PSendSysMessage(LANG_COMMAND_TARGET_LISTAURAS, uAuras.size());
    for (Unit::AuraMap::const_iterator itr = uAuras.begin(); itr != uAuras.end(); ++itr)
    {
        bool talent = GetTalentSpellCost(itr->second->GetId()) > 0;

        char const* name = itr->second->GetSpellProto()->SpellName[0];
 
        if (args != "" && strstr(name, args) == nullptr)
            continue;

        if (m_session)
        {
            std::ostringstream ss_name;
            ss_name << "|cffffffff|Hspell:" << itr->second->GetId() << "|h[" << name << "]|h|r";

            PSendSysMessage(LANG_COMMAND_TARGET_AURADETAIL, itr->second->GetId(), itr->second->GetEffIndex(),
                itr->second->GetModifier()->m_auraname, itr->second->GetAuraDuration(), itr->second->GetAuraMaxDuration(),
                ss_name.str().c_str(),
                (itr->second->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
                IS_PLAYER_GUID(itr->second->GetCasterGUID()) ? "player" : "creature",GUID_LOPART(itr->second->GetCasterGUID()));
        }
        else
        {
            PSendSysMessage(LANG_COMMAND_TARGET_AURADETAIL, itr->second->GetId(), itr->second->GetEffIndex(),
                itr->second->GetModifier()->m_auraname, itr->second->GetAuraDuration(), itr->second->GetAuraMaxDuration(),
                name,
                (itr->second->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
                IS_PLAYER_GUID(itr->second->GetCasterGUID()) ? "player" : "creature",GUID_LOPART(itr->second->GetCasterGUID()));
        }
    }

    for (int i = 0; i < TOTAL_AURAS; i++)
    {
        Unit::AuraList const& uAuraList = unit->GetAurasByType(AuraType(i));
        if (uAuraList.empty()) continue;
        
        bool type_print = true;
        for (Unit::AuraList::const_iterator itr = uAuraList.begin(); itr != uAuraList.end(); ++itr)
        {
            bool talent = GetTalentSpellCost((*itr)->GetId()) > 0;

            char const* name = (*itr)->GetSpellProto()->SpellName[m_session->GetSessionDbcLocale()];

            if (args != "" && strstr(name, args) == nullptr)
                continue;

            if (type_print)
            {
                PSendSysMessage(LANG_COMMAND_TARGET_LISTAURATYPE, uAuraList.size(), i);
                type_print = false;
            }

            if (m_session)
            {
                std::ostringstream ss_name;
                ss_name << "|cffffffff|Hspell:" << (*itr)->GetId() << "|h[" << name << "]|h|r";

                PSendSysMessage(LANG_COMMAND_TARGET_AURASIMPLE, (*itr)->GetId(), (*itr)->GetEffIndex(),
                    ss_name.str().c_str(),((*itr)->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
                    IS_PLAYER_GUID((*itr)->GetCasterGUID()) ? "player" : "creature",GUID_LOPART((*itr)->GetCasterGUID()));
            }
            else
            {
                PSendSysMessage(LANG_COMMAND_TARGET_AURASIMPLE, (*itr)->GetId(), (*itr)->GetEffIndex(),
                    name,((*itr)->IsPassive() ? passiveStr : ""),(talent ? talentStr : ""),
                    IS_PLAYER_GUID((*itr)->GetCasterGUID()) ? "player" : "creature",GUID_LOPART((*itr)->GetCasterGUID()));
            }
        }
    }
    return true;
}

bool ChatHandler::HandleResetHonorCommand (const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;
        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sObjectMgr.GetPlayerGUIDByName(name.c_str());
        player = sObjectMgr.GetPlayerInWorld(guid);
    }
    else
        player = getSelectedPlayer();

    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        return true;
    }

    player->SetUInt32Value(PLAYER_FIELD_KILLS, 0);
    player->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);
    player->SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, 0);
    player->SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
    player->SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);

    return true;
}

static bool HandleResetStatsOrLevelHelper(Player* player)
{
    PlayerInfo const *info = sObjectMgr.GetPlayerInfo(player->GetRace(), player->GetClass());
    if (!info) return false;

    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(player->GetClass());
    if (!cEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Class %u not found in DBC (Wrong DBC files?)",player->GetClass());
        return false;
    }

    uint8 powertype = cEntry->powerType;

    uint32 unitfield;
    if (powertype == POWER_RAGE)
        unitfield = 0x1100EE00;
    else if (powertype == POWER_ENERGY)
        unitfield = 0x00000000;
    else if (powertype == POWER_MANA)
        unitfield = 0x0000EE00;
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Invalid default powertype %u for player (class %u)",powertype,player->GetClass());
        return false;
    }

    // reset m_form if no aura
    if (!player->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        player->m_form = FORM_NONE;

    player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_WORLD_OBJECT_SIZE);
    player->SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_COMBAT_REACH);

    player->SetUInt32Value(UNIT_FIELD_BYTES_0, ((player->GetRace()) | (player->GetClass() << 8) | (player->GetGender() << 16) | (powertype << 24)));

    player->RestoreFaction();

    // reset only if player not in some form;
    if (player->m_form==FORM_NONE)
    {
        switch (player->GetGender())
        {
            case GENDER_FEMALE:
                player->SetDisplayId(info->displayId_f);
                player->SetNativeDisplayId(info->displayId_f);
                break;
            case GENDER_MALE:
                player->SetDisplayId(info->displayId_m);
                player->SetNativeDisplayId(info->displayId_m);
                break;
            default:
                break;
        }
    }

    // set UNIT_FIELD_BYTES_1 to init state but preserve m_form value
    player->SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield);
    player->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY | UNIT_BYTE2_FLAG_UNK5);
    player->SetByteValue(UNIT_FIELD_BYTES_2, 3, player->m_form);

    player->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

    //-1 is default value
    player->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    //player->SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000);
    return true;
}

bool ChatHandler::HandleResetLevelCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;
        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sObjectMgr.GetPlayerGUIDByName(name.c_str());
        player = sObjectMgr.GetPlayerInWorld(guid);
    }
    else
        player = getSelectedPlayer();

    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (!HandleResetStatsOrLevelHelper(player))
        return false;

    player->SetLevel(1);
    player->InitStatsForLevel(true);
    player->InitTaxiNodesForLevel();
    player->InitTalentForLevel();
    player->SetUInt32Value(PLAYER_XP,0);

    // reset level to summoned pet
    Pet* pet = player->GetPet();
    if (pet && pet->getPetType()==SUMMON_PET)
        pet->InitStatsForLevel(1);

    return true;
}

bool ChatHandler::HandleResetStatsCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    if (pName)
    {
        std::string name = pName;
        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        uint64 guid = sObjectMgr.GetPlayerGUIDByName(name.c_str());
        player = sObjectMgr.GetPlayerInWorld(guid);
    }
    else
        player = getSelectedPlayer();

    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (!HandleResetStatsOrLevelHelper(player))
        return false;

    player->InitStatsForLevel(true);
    player->InitTaxiNodesForLevel();
    player->InitTalentForLevel();

    return true;
}

bool ChatHandler::HandleResetSpellsCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    uint64 playerGUID = 0;
    if (pName)
    {
        std::string name = pName;

        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectMgr.GetPlayerInWorld(name.c_str());
        if (!player)
            playerGUID = sObjectMgr.GetPlayerGUIDByName(name.c_str());
    }
    else
        player = getSelectedPlayer();

    if (!player && !playerGUID)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (player)
    {
        player->resetSpells();

        ChatHandler(player).SendSysMessage(LANG_RESET_SPELLS);

        if (m_session->GetPlayer()!=player)
            PSendSysMessage(LANG_RESET_SPELLS_ONLINE,player->GetName());
    }
    else
    {
        RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'",uint32(AT_LOGIN_RESET_SPELLS), GUID_LOPART(playerGUID));
        PSendSysMessage(LANG_RESET_SPELLS_OFFLINE,pName);
    }

    return true;
}

bool ChatHandler::HandleResetTalentsCommand(const char * args)
{
    char* pName = strtok((char*)args, "");
    Player *player = NULL;
    uint64 playerGUID = 0;
    if (pName)
    {
        std::string name = pName;
        if (!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        player = sObjectMgr.GetPlayerInWorld(name.c_str());
        if (!player)
            playerGUID = sObjectMgr.GetPlayerGUIDByName(name.c_str());
    }
    else
        player = getSelectedPlayer();

    if (!player && !playerGUID)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (player)
    {
        player->resetTalents(true);

        ChatHandler(player).SendSysMessage(LANG_RESET_TALENTS);

        if (m_session->GetPlayer()!=player)
            PSendSysMessage(LANG_RESET_TALENTS_ONLINE,player->GetName());
    }
    else
    {
        RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'",uint32(AT_LOGIN_RESET_TALENTS), GUID_LOPART(playerGUID));
        PSendSysMessage(LANG_RESET_TALENTS_OFFLINE,pName);
    }

    return true;
}

bool ChatHandler::HandleResetAllCommand(const char * args)
{
    if (!*args)
        return false;

    std::string casename = args;

    AtLoginFlags atLogin;

    // Command specially created as single command to prevent using short case names
    if (casename=="spells")
    {
        atLogin = AT_LOGIN_RESET_SPELLS;
        sWorld.SendWorldText(LANG_RESETALL_SPELLS, 0);
    }
    else if (casename=="talents")
    {
        atLogin = AT_LOGIN_RESET_TALENTS;
        sWorld.SendWorldText(LANG_RESETALL_TALENTS, 0);
    }
    else
    {
        PSendSysMessage(LANG_RESETALL_UNKNOWN_CASE, 0, args);
        SetSentErrorMessage(true);
        return false;
    }

    RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE (at_login & '%u') = '0'",atLogin,atLogin);

    ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, *HashMapHolder<Player>::GetLock(), true);
    HashMapHolder<Player>::MapType const& plist = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = plist.begin(); itr != plist.end(); ++itr)
        itr->second->SetAtLoginFlag(atLogin);

    return true;
}

bool ChatHandler::HandleServerMuteCommand(const char* args)
{
    if (!*args)
        return false;

    char* time_str = strtok((char*) args, " ");
    char* reason = strtok (NULL, "");

    if (!time_str)
        return false;

    int32 time2 = atoi(time_str);

    if (time2 <= 0)
        return false;

    sWorld.SetMassMute(time(NULL) + time2, reason);

    if (reason)
        PSendSysMessage("Server was muted for %i seconds with reason: %s.", time2, reason);
    else
        PSendSysMessage("Server was muted for %i seconds.", time2);

    return true;
}

bool ChatHandler::HandleServerShutDownCancelCommand(const char* /*args*/)
{
    sWorld.ShutdownCancel();
    return true;
}

bool ChatHandler::HandleServerShutDownCommand(const char* args)
{
    if (!*args)
        return false;

    const char* time_str = strtok ((char*) args, " ");
    const char* exitmsg = strtok(NULL, "");

    int32 time = atoi (time_str);

    if (time <= 0)
        return false;

    if (exitmsg)
        sWorld.ShutdownServ(time,0,SHUTDOWN_EXIT_CODE, exitmsg);
    else
        sWorld.ShutdownServ(time,0,SHUTDOWN_EXIT_CODE);

    return true;
}

bool ChatHandler::HandleServerRollShutDownCommand(const char* args)
{
    if (!*args)
        return false;

    const char* time_str = strtok((char*) args, " ");
    const char* exitmsg = strtok(NULL, "");

    int time;
    int roll = atoi(time_str);

    if (roll <= 0)
        return false;

    if (!exitmsg)
        exitmsg = "";

    time = urand(1, roll);

    sWorld.SendWorldText(LANG_ROLLSHUTDOWN, 0, roll, time, exitmsg);

    sWorld.ShutdownServ(time, 0, SHUTDOWN_EXIT_CODE, exitmsg);
    return true;
}

bool ChatHandler::HandleServerRestartCommand(const char* args)
{
    if (!*args)
        return false;

    char const* time_str = strtok ((char*) args, " ");
    char const* exitcode_str = strtok (NULL, "");

    int32 time = atoi (time_str);

    if (time <= 0)
        return false;

    if (exitcode_str)
        sWorld.ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE, exitcode_str);
    else
        sWorld.ShutdownServ(time, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE);
    return true;
}

bool ChatHandler::HandleServerIdleRestartCommand(const char* args)
{
    if (!*args)
        return false;

    char const* time_str = strtok ((char*) args, " ");
    char const* exitcode_str = strtok (NULL, "");

    int32 time = atoi (time_str);

    if (time <= 0)
        return false;

    if (exitcode_str)
        sWorld.ShutdownServ(time,SHUTDOWN_MASK_RESTART|SHUTDOWN_MASK_IDLE,RESTART_EXIT_CODE, exitcode_str);
    else
        sWorld.ShutdownServ(time,SHUTDOWN_MASK_RESTART|SHUTDOWN_MASK_IDLE,RESTART_EXIT_CODE);
    return true;
}

bool ChatHandler::HandleServerIdleShutDownCommand(const char* args)
{
    if (!*args)
        return false;

    char const* time_str = strtok ((char*) args, " ");
    char const* exitcode_str = strtok (NULL, "");

    int32 time = atoi (time_str);

    if (time <= 0)
        return false;

    if (exitcode_str)
        sWorld.ShutdownServ(time,SHUTDOWN_MASK_IDLE,SHUTDOWN_EXIT_CODE, exitcode_str);
    else
        sWorld.ShutdownServ(time,SHUTDOWN_MASK_IDLE,SHUTDOWN_EXIT_CODE);
    return true;
}

bool ChatHandler::HandleQuestAdd(const char* args)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .addquest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if (!cId)
        return false;

    uint32 entry = atol(cId);

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    if (!pQuest)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND,entry);
        SetSentErrorMessage(true);
        return false;
    }

    // check item starting quest (it can work incorrectly if added without item in inventory)
    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); id++)
    {
        ItemPrototype const *pProto = sItemStorage.LookupEntry<ItemPrototype>(id);
        if (!pProto)
            continue;

        if (pProto->StartQuest == entry)
        {
            PSendSysMessage(LANG_COMMAND_QUEST_STARTFROMITEM, entry, pProto->ItemId);
            SetSentErrorMessage(true);
            return false;
        }
    }

    // ok, normal (creature/GO starting) quest
    if (player->CanAddQuest(pQuest, true))
    {
        player->AddQuest(pQuest, NULL);

        if (player->CanCompleteQuest(entry))
            player->CompleteQuest(entry);
    }

    return true;
}

bool ChatHandler::HandleQuestRemove(const char* args)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .removequest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if (!cId)
        return false;

    uint32 entry = atol(cId);

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    if (!pQuest)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }

    // remove all quest entries for 'entry' from quest log
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 quest = player->GetQuestSlotQuestId(slot);
        if (quest==entry)
        {
            player->SetQuestSlot(slot,0);

            // we ignore unequippable quest items in this case, its' still be equipped
            player->TakeQuestSourceItem(quest, false);
        }
    }

    // set quest status to not started (will updated in DB at next save)
    player->SetQuestStatus(entry, QUEST_STATUS_NONE);

    // reset rewarded for restart repeatable quest
    player->getQuestStatusMap()[entry].m_rewarded = false;

    SendSysMessage(LANG_COMMAND_QUEST_REMOVED);
    return true;
}

bool ChatHandler::HandleQuestComplete(const char* args)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .quest complete #entry
    // number or [name] Shift-click form |color|Hquest:quest_id|h[name]|h|r
    char* cId = extractKeyFromLink((char*)args,"Hquest");
    if (!cId)
        return false;

    uint32 entry = atol(cId);

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    // If player doesn't have the quest
    if (!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }

    // Add quest items for quests that require items
    for (uint8 x = 0; x < QUEST_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->ReqItemId[x];
        uint32 count = pQuest->ReqItemCount[x];
        if (!id || !count)
            continue;

        uint32 curItemCount = player->GetItemCount(id,true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count-curItemCount);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, id, true, 0, "GM_COMMAND");
            player->SendNewItem(item,count-curItemCount,true,false);
        }
    }

    // All creature/GO slain/cast (not required, but otherwise it will display "Creature slain 0/10")
    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        uint32 creature = pQuest->ReqCreatureOrGOId[i];
        uint32 creaturecount = pQuest->ReqCreatureOrGOCount[i];

        if (uint32 spell_id = pQuest->ReqSpell[i])
        {
            for (uint16 z = 0; z < creaturecount; ++z)
                player->CastCreatureOrGO(creature,0,spell_id);
        }
        else if (creature > 0)
        {
            for (uint16 z = 0; z < creaturecount; ++z)
                player->KilledMonster(creature,0);
        }
        else if (creature < 0)
        {
            for (uint16 z = 0; z < creaturecount; ++z)
                player->CastCreatureOrGO(creature,0,0);
        }
    }

    // If the quest requires reputation to complete
    if (uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
        if (curRep < repValue)
            if (FactionEntry const *factionEntry = sFactionStore.LookupEntry(repFaction))
                player->GetReputationMgr().SetReputation(factionEntry,repValue);
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if (ReqOrRewMoney < 0) // CanComplete quest has check for is player has more money than can have or not
        player->ModifyMoney(-ReqOrRewMoney);

    player->CompleteQuest(entry);
    return true;
}

bool ChatHandler::HandleMarkBot(const char* args)
{
	char* strname = strtok((char*)args, " ");
	if (!strname)
		return false;

	char* strconf = strtok((char*)NULL, " ");
	
	std::string name = strname;
	std::string conf = strconf ? strconf : "";

	if (!normalizePlayerName(name))
	{
		SendSysMessage(LANG_PLAYER_NOT_FOUND);
		SetSentErrorMessage(true);
		return false;
	}
	
	bool remove = false;

	if (Player *p = sObjectAccessor.GetPlayerByNameInWorld(name))
	{
		if (p->IsPlayerCustomFlagged(PL_CUSTOM_MARKED_BOT))
		{
			if (conf != "ok")
			{
				PSendSysMessage("%s is already marked, need confirmation (ok at the end)", name.c_str());
				return true;
			}
			
			p->RemovePlayerCustomFlag(PL_CUSTOM_MARKED_BOT);
			remove = true;
		}
		else
			p->AddPlayerCustomFlag(PL_CUSTOM_MARKED_BOT);
	}
	else
	{
		QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT char_custom_flags FROM characters WHERE name = '%s'", name.c_str());
		if (!result)
		{
			SendSysMessage(LANG_PLAYER_NOT_FOUND);
			SetSentErrorMessage(true);
			return false;
		}

		if (result->Fetch()->GetUInt32() & 256)
		{
			if (conf != "ok")
			{
				PSendSysMessage("%s is already marked, need confirmation (ok at the end)", name.c_str());
				return true;
			}

			RealmDataDatabase.PExecute("UPDATE characters SET char_custom_flags = char_custom_flags &~ 256 where name = '%s'", name.c_str());
			remove = true;
		}
		else
			RealmDataDatabase.PExecute("UPDATE characters SET char_custom_flags = char_custom_flags | 256 where name = '%s'", name.c_str());
	}

	PSendSysMessage(remove ? "%s was UNMARKED!" : "%s was marked as bot", name.c_str());

	return true;
}

bool ChatHandler::HandleBanAccountCommand(const char* args)
{
    return HandleBanHelper(BAN_ACCOUNT,args);
}

bool ChatHandler::HandleBanCharacterCommand(const char* args)
{
    return HandleBanHelper(BAN_CHARACTER,args);
}

bool ChatHandler::HandleBanIPCommand(const char* args)
{
    return HandleBanHelper(BAN_IP,args);
}

bool ChatHandler::HandleBanHelper(BanMode mode, const char* args)
{
    if (!args)
        return false;

    char* cnameIPOrMail = strtok ((char*)args, " ");
    if (!cnameIPOrMail)
        return false;

    std::string nameIPOrMail = cnameIPOrMail;

    char* duration;
    duration = strtok(NULL, " ");
    if (!duration || !atoi(duration) || (atoi(duration) > 0 && !TimeStringToSecs(duration)))
        return false;

    char* reason = strtok (NULL,"");
    if (!reason)
    {
        SendSysMessage("Reason must be set");
        SetSentErrorMessage(true);
        return false;
    }

    std::string mutereasonstr = reason;

    if (m_session && !std::regex_search(mutereasonstr, std::regex("^\\d+\\.\\d+ \\(\\d+\\)$")))
    {
        SendSysMessage("Specify violated rule and the number of repetitions. Example: 1.4 (1), 2.4 (6), etc...");
        SetSentErrorMessage(true);
        return false;
    }

    switch (mode)
    {
        case BAN_ACCOUNT:
            if (!AccountMgr::normalizeString(nameIPOrMail))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,nameIPOrMail.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_CHARACTER:
        case BAN_ARENA3V3:
            if (!normalizePlayerName(nameIPOrMail))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            else if (Player *bannedPlayer = sObjectAccessor.GetPlayerByNameInWorld(nameIPOrMail))
                if (bannedPlayer->HasAura(9454, 0))
                    bannedPlayer->RemoveAura(9454, 0);
            break;
        case BAN_IP:
            if (!IsIPAddress(nameIPOrMail.c_str()))
                return false;
            break;
    }

    switch (sWorld.BanAccount(mode, nameIPOrMail, duration, reason,m_session ? m_session->GetPlayerName() : ""))
    {
        case BAN_SUCCESS:
            if (atoi(duration)>0)
                PSendSysMessage(LANG_BAN_YOUBANNED,nameIPOrMail.c_str(), WorldSession::secondsToTimeString(m_session, TimeStringToSecs(duration),true).c_str(),reason);
            else
                PSendSysMessage(LANG_BAN_YOUPERMBANNED,nameIPOrMail.c_str(),reason);
            break;
        case BAN_SYNTAX_ERROR:
            return false;
        case BAN_NOTFOUND:
            switch (mode)
            {
                default:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"account",nameIPOrMail.c_str());
                    break;
                case BAN_CHARACTER:
                case BAN_ARENA3V3:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"character",nameIPOrMail.c_str());
                    break;
                case BAN_IP:
                    PSendSysMessage(LANG_BAN_NOTFOUND,"ip",nameIPOrMail.c_str());
                    break;
            }
            SetSentErrorMessage(true);
            return false;
    }

    if ((mode == BAN_CHARACTER || mode == BAN_ARENA3V3) && m_session && !(m_session->GetPermissions() & 0x2000/*admin*/ || m_session->GetPermissions() == 0x9/*moderator*/) && sWorld.getConfig(CONFIG_SHOW_KICK_IN_WORLD) == 1)
    {
        sWorld.SendWorldText((m_session->GetPermissions() & 0x400/*GM*/) ? LANG_ANNOUNCE_BAN_GM : LANG_ANNOUNCE_BAN_EM,
            0, m_session->GetPlayerName(), nameIPOrMail.c_str(), duration, reason);
    }
    
    SendGlobalGMSysMessage(LANG_GM_BANNED_PLAYER, m_session ? m_session->GetPlayerName() : "", nameIPOrMail.c_str(), duration, reason);

    return true;
}

bool ChatHandler::HandleRemoveBanHistoryCommand(const char* args)
{
    if (!args)
        return false;

    char *u = strtok((char*)args, " ");
    char *d = strtok(NULL, " ");

    if (!u || !d)
    {
        PSendSysMessage("ERROR! USE: .removebanhistory AccountUsername BanID");
        SetSentErrorMessage(true);
        return false;
    }

    std::string username = u;
    uint64 punishment_date = atol(d);
    
    if (!AccountMgr::normalizeString(username))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, username.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 account = 0;
    account = AccountMgr::GetId(username);

    if (!account)
    {
        PSendSysMessage("Check account username");
        return false;
    }

    if (!AccountsDatabase.PQuery("SELECT 1 FROM account_punishment WHERE `punishment_date` = '%llu' AND `account_id` = '%u' LIMIT 1", punishment_date, account))
    {
        PSendSysMessage("Wrong BanID");
        return AOR_DB_INTERNAL_ERROR;
    }

    if (!AccountsDatabase.PExecute("DELETE FROM `account_punishment` WHERE `punishment_date` = '%llu' AND `account_id` = '%u' LIMIT 1", punishment_date, account))
    {
        PSendSysMessage("Error!");
        return AOR_DB_INTERNAL_ERROR;
    }

    PSendSysMessage("History removed");
    return true;
}

bool ChatHandler::HandleUnBanAccountCommand(const char* args)
{
    return HandleUnBanHelper(BAN_ACCOUNT,args);
}

bool ChatHandler::HandleUnBanCharacterCommand(const char* args)
{
    return HandleUnBanHelper(BAN_CHARACTER,args);
}

bool ChatHandler::HandleUnBanIPCommand(const char* args)
{
    return HandleUnBanHelper(BAN_IP,args);
}

bool ChatHandler::HandleUnBanHelper(BanMode mode, const char* args)
{
    if (!args)
        return false;

    char* cnameIPOrMail = strtok ((char*)args, " ");


    if (!cnameIPOrMail)
        return false;

    std::string nameIPOrMail = cnameIPOrMail;

    switch (mode)
    {
        case BAN_ACCOUNT:
            if (!AccountMgr::normalizeString(nameIPOrMail))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,nameIPOrMail.c_str());
                SetSentErrorMessage(true);
                return false;
            }

            break;
        case BAN_CHARACTER:
        case BAN_ARENA3V3:
            if (!normalizePlayerName(nameIPOrMail))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_IP:
            if (!IsIPAddress(nameIPOrMail.c_str()))
                return false;
            break;
    }

    if (m_session && (mode == BAN_ACCOUNT || mode == BAN_CHARACTER || mode == BAN_ARENA3V3))
    {
        uint32 account = 0;
        if (mode == BAN_ACCOUNT)
            account = AccountMgr::GetId(nameIPOrMail);
        else if (mode == BAN_CHARACTER || mode == BAN_ARENA3V3)
            account = sObjectMgr.GetPlayerAccountIdByPlayerName(nameIPOrMail);

        if (!account)
        {
            PSendSysMessage(LANG_UNBAN_ERROR, nameIPOrMail.c_str());
            return false;
        }

        uint32 ban_type = mode == BAN_ARENA3V3 ? PUNISHMENT_ARENA3V3 : PUNISHMENT_BAN;

        QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT punished_by FROM account_punishment WHERE account_id = '%u' AND punishment_type_id = '%u' AND active = 1 LIMIT 1", account, ban_type);
        if (result)
        {
            Field *fields = result->Fetch();
            std::string punished_by = fields[0].GetString();
            if (punished_by == "[CONSOLE]")
            {
                m_session->SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
                return false;
            }

            if (uint32 punished_by_acc = sObjectMgr.GetPlayerAccountIdByPlayerName(punished_by))
                if (AccountMgr::GetPermissions(punished_by_acc) > m_session->GetPermissions())
                {
                    m_session->SendNotification(LANG_YOU_NOT_HAVE_PERMISSION);
                    return false;
                }

        }
    }

    if (sWorld.RemoveBanAccount(mode,nameIPOrMail))
        PSendSysMessage(LANG_UNBAN_UNBANNED,nameIPOrMail.c_str());
    else
        PSendSysMessage(LANG_UNBAN_ERROR,nameIPOrMail.c_str());

    return true;
}

bool ChatHandler::HandleBanInfoAccountCommand(const char* args)
{
    if (!args)
        return false;

    char* cname = strtok((char*)args, "");
    if (!cname)
        return false;

    std::string account_name = cname;
    if (!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = AccountMgr::GetId(account_name);
    if (!accountid)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        return true;
    }

    return HandleBanInfoHelper(accountid,account_name.c_str(), PUNISHMENT_BAN);
}

bool ChatHandler::HandleBanInfoCharacterCommand(const char* args)
{
    if (!args)
        return false;

    char* cname = strtok ((char*)args, "");
    if (!cname)
        return false;

    std::string name = cname;
    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sObjectMgr.GetPlayerAccountIdByPlayerName(name);
    if (!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if (!AccountMgr::GetName(accountid,accountname))
    {
        SendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }

    return HandleBanInfoHelper(accountid,accountname.c_str(), PUNISHMENT_BAN);
}

bool ChatHandler::HandleBanInfoHelper(uint32 accountid, char const* accountname, uint32 ban_type)
{
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT FROM_UNIXTIME(punishment_date), expiration_date-punishment_date, expiration_date, reason, punished_by, active, punishment_date "
                                                        "FROM account_punishment "
                                                        "WHERE punishment_type_id = '%u' AND account_id = '%u' "
                                                        "ORDER BY punishment_date ASC", ban_type, accountid);

    if (!result)
    {
        PSendSysMessage(LANG_BANINFO_NOACCOUNTBAN, accountname);
        return true;
    }

    PSendSysMessage(LANG_BANINFO_BANHISTORY,accountname);
    do
    {
        Field* fields = result->Fetch();

        time_t unbandate = time_t(fields[2].GetUInt64());
        uint64 banLength = fields[1].GetUInt64();
        bool active = false;
        bool permanent = (banLength == 0);

        uint32 date_id = fields[6].GetUInt32();

        if ((permanent || unbandate >= time(NULL)) && fields[5].GetBool())
            active = true;

        std::string bantime = permanent ? GetHellgroundString(LANG_BANINFO_INFINITE) : WorldSession::secondsToTimeString(m_session, banLength, true);

        PSendSysMessage(LANG_BANINFO_HISTORYENTRY,
            date_id, fields[0].GetString(), bantime.c_str(), active ? GetHellgroundString(LANG_BANINFO_YES) : GetHellgroundString(LANG_BANINFO_NO), fields[3].GetString(), fields[4].GetString());
    }
    while (result->NextRow());

    return true;
}

bool ChatHandler::HandleBanInfoIPCommand(const char* args)
{
    if (!args)
        return false;

    char* cIP = strtok ((char*)args, "");
    if (!cIP)
        return false;

    if (!IsIPAddress(cIP))
        return false;

    std::string IP = cIP;

    AccountsDatabase.escape_string(IP);
    QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT ip, FROM_UNIXTIME(punishment_date), FROM_UNIXTIME(expiration_date), expiration_date-UNIX_TIMESTAMP(), ban_reason, punished_by, expiration_date-punishment_date FROM ip_banned WHERE ip = '%s'", IP.c_str());
    if (!result)
    {
        SendSysMessage(LANG_BANINFO_NOIP);
        return true;
    }

    Field *fields = result->Fetch();
    bool permanent = !fields[6].GetUInt64();
    PSendSysMessage(LANG_BANINFO_IPENTRY,
        fields[0].GetString(), fields[1].GetString(), permanent ? GetHellgroundString(LANG_BANINFO_NEVER):fields[2].GetString(),
        permanent ? GetHellgroundString(LANG_BANINFO_INFINITE): WorldSession::secondsToTimeString(m_session, fields[3].GetUInt64(), true).c_str(), fields[4].GetString(), fields[5].GetString());
    return true;
}

bool ChatHandler::HandleBanListCharacterCommand(const char* args)
{
    // unsafe (sql injection)?
    return 0;
    
    char* cFilter = strtok ((char*)args, " ");
    if (!cFilter)
        return false;

    std::string filter = cFilter;
    AccountsDatabase.escape_string(filter);
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT account FROM characters WHERE name LIKE '%%%s%%'", filter.c_str());
    if (!result)
    {
        SendSysMessage(LANG_BANLIST_NOCHARACTER);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleBanListAccountCommand(const char* args)
{
    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    AccountsDatabase.escape_string(filter);

    QueryResultAutoPtr result;

    if (filter.empty())
        result = AccountsDatabase.PQuery("SELECT account.account_id, username "
                                        "FROM account JOIN account_punishment ON account.account_id = account_punishment.account_id "
                                        "WHERE punishment_type_id = '%u' AND active = 1 GROUP BY account.account_id", PUNISHMENT_BAN);
    else
        result = AccountsDatabase.PQuery("SELECT account.account_id, username "
                                        "FROM account JOIN account_punishment ON account.account_id = account_punishment.account_id "
                                        "WHERE punishment_type_id = '%u' AND active = 1 AND "
                                        "username LIKE '%%%s%%' GROUP BY account.account_id", PUNISHMENT_BAN, filter.c_str());


    if (!result)
    {
        SendSysMessage(LANG_BANLIST_NOACCOUNT);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleBanListHelper(QueryResultAutoPtr result)
{
    SendSysMessage(LANG_BANLIST_MATCHINGACCOUNT);

    // Chat short output
    if (m_session)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 accountid = fields[0].GetUInt32();

            QueryResultAutoPtr banresult = AccountsDatabase.PQuery("SELECT account.username "
                                                                    "FROM account JOIN account_punishment ON account.account_id = account_punishment.account_id "
                                                                    "WHERE account.account_id = '%u' AND punishment_type_id = '%u'", accountid, PUNISHMENT_BAN);
            if (banresult)
            {
                Field* fields2 = banresult->Fetch();
                PSendSysMessage("%s", fields2[0].GetString());
            }
        }
        while (result->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_ACCOUNTS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_ACCOUNTS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field *fields = result->Fetch();
            uint32 account_id = fields[0].GetUInt32 ();

            std::string account_name;

            // "account" case, name can be get in same query
            if (result->GetFieldCount() > 1)
                account_name = fields[1].GetCppString();
            // "character" case, name need extract from another DB
            else
                AccountMgr::GetName (account_id,account_name);

            // No SQL injection. id is uint32.
            QueryResultAutoPtr banInfo = AccountsDatabase.PQuery("SELECT punishment_date, expiration_date, punished_by, reason "
                                                                "FROM account_punishment "
                                                                "WHERE account_id = '%u' AND punishment_type_id = '%u' "
                                                                "ORDER BY expiration_date", account_id, PUNISHMENT_BAN);

            if (banInfo)
            {
                Field *fields2 = banInfo->Fetch();
                do
                {
                    time_t t_ban = fields2[0].GetUInt64();
                    tm* aTm_ban = localtime(&t_ban);

                    if (fields2[0].GetUInt64() == fields2[1].GetUInt64())
                    {
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                            account_name.c_str(),aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                            fields2[2].GetString(),fields2[3].GetString());
                    }
                    else
                    {
                        time_t t_unban = fields2[1].GetUInt64();
                        tm* aTm_unban = localtime(&t_unban);
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                            account_name.c_str(),aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                            aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                            fields2[2].GetString(),fields2[3].GetString());
                    }
                }
                while (banInfo->NextRow());
            }
        }
        while (result->NextRow());
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleBanListIPCommand(const char* args)
{
    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    AccountsDatabase.escape_string(filter);

    QueryResultAutoPtr result;

    if (filter.empty())
        result = AccountsDatabase.Query("SELECT ip, punishment_date, expiration_date, punished_by, ban_reason FROM ip_banned"
            " WHERE (punishment_date = expiration_date OR expiration_date > UNIX_TIMESTAMP())"
            " ORDER BY expiration_date");
    else
        result = AccountsDatabase.PQuery("SELECT ip, punishment_date, expiration_date, punished_by, ban_reason FROM ip_banned"
            " WHERE (punishment_date = expiration_date OR expiration_date > UNIX_TIMESTAMP()) AND ip LIKE '%%%s%%'"
            " ORDER BY expiration_date", filter.c_str());

    if (!result)
    {
        SendSysMessage(LANG_BANLIST_NOIP);
        return true;
    }

    SendSysMessage(LANG_BANLIST_MATCHINGIP);
    // Chat short output
    if (m_session)
    {
        do
        {
            Field* fields = result->Fetch();
            PSendSysMessage("%s", fields[0].GetString());
        }
        while (result->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_IPS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_IPS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field *fields = result->Fetch();
            time_t t_ban = fields[1].GetUInt64();
            tm* aTm_ban = localtime(&t_ban);
            if (fields[1].GetUInt64() == fields[2].GetUInt64())
            {
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                    fields[0].GetString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    fields[3].GetString(), fields[4].GetString());
            }
            else
            {
                time_t t_unban = fields[2].GetUInt64();
                tm* aTm_unban = localtime(&t_unban);
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                    fields[0].GetString(), aTm_ban->tm_year%100, aTm_ban->tm_mon+1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                    aTm_unban->tm_year%100, aTm_unban->tm_mon+1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                    fields[3].GetString(), fields[4].GetString());
            }
        }
        while (result->NextRow());
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleRespawnCommand(const char* /*args*/)
{
    Player* pl = m_session->GetPlayer();

    // accept only explicitly selected target (not implicitly self targeting case)
    Unit* target = getSelectedUnit();
    if (pl->GetSelection() && target)
    {
        if (target->GetTypeId()!=TYPEID_UNIT)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }

        if (target->isDead())
            ((Creature*)target)->Respawn();
        return true;
    }

    Hellground::RespawnDo u_do;
    Hellground::ObjectWorker<Creature, Hellground::RespawnDo> worker(u_do);

    Cell::VisitGridObjects(pl, worker, pl->GetMap()->GetVisibilityDistance());
    return true;
}

bool ChatHandler::HandleGMFlyCommand(const char* args)
{
    if (!args)
        return false;

    Unit *unit = getSelectedUnit();
    if (!unit || (unit->GetTypeId() != TYPEID_PLAYER) || m_session->GetPermissions() < PERM_ADM)
        unit = m_session->GetPlayer();

    WorldPacket data(Opcodes(12));
    if (strncmp(args, "on", 3) == 0)
        unit->AddAura(55195, unit);
        //data.SetOpcode(SMSG_MOVE_SET_CAN_FLY);
    else if (strncmp(args, "off", 4) == 0)
        unit->RemoveAurasDueToSpell(55195);
        //data.SetOpcode(SMSG_MOVE_UNSET_CAN_FLY);
    else
    {
        SendSysMessage(LANG_USE_BOL);
        return false;
    }
    /*data << unit->GetPackGUID();
    data << uint32(0);                                      // unknown
    unit->BroadcastPacket(&data, true);*/
    PSendSysMessage(LANG_COMMAND_FLYMODE_STATUS, unit->GetName(), args);
    return true;
}

bool ChatHandler::HandleNpcChangeEntryCommand(const char *args)
{
    if (!args)
        return false;

    uint32 newEntryNum = atoi(args);
    if (!newEntryNum)
        return false;

    Unit* unit = getSelectedUnit();
    if (!unit || unit->GetTypeId() != TYPEID_UNIT)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }
    Creature* creature = (Creature*)unit;
    if (creature->UpdateEntry(newEntryNum))
        SendSysMessage(LANG_DONE);
    else
        SendSysMessage(LANG_ERROR);
    return true;
}

bool ChatHandler::HandleMovegensCommand(const char* /*args*/)
{
    Unit* unit = getSelectedUnit();
    if(!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_MOVEGENS_LIST,unit->GetObjectGuid().GetString().c_str(), unit->GetGUIDLow());

    UnitStateMgr& statemgr = unit->GetUnitStateMgr();

    float x,y,z;
    for (int32 i = UNIT_ACTION_PRIORITY_IDLE; i != UNIT_ACTION_PRIORITY_END; ++i)
    {
        ActionInfo* actionInfo = statemgr.GetAction(UnitActionPriority(i));
        if (!actionInfo)
            continue;

        UnitActionPtr action = actionInfo->Action();

        PSendSysMessage("[%d] %s", i, action->Name());

        switch(action->GetMovementGeneratorType())
        {
        case IDLE_MOTION_TYPE:
        case RANDOM_MOTION_TYPE:
        case WAYPOINT_MOTION_TYPE:
        case CONFUSED_MOTION_TYPE:
        case FLIGHT_MOTION_TYPE:
        case FLEEING_MOTION_TYPE:
        case DISTRACT_MOTION_TYPE:
        case EFFECT_MOTION_TYPE:
            break;

        case CHASE_MOTION_TYPE:
            {
                Unit* target = NULL;
                if(unit->GetTypeId()==TYPEID_PLAYER)
                {
                    target = static_cast<ChaseMovementGenerator<Player> const*>(&*action)->GetTarget();
                    PSendSysMessage(LANG_MOVEGENS_CHASE_PLAYER, target ? target->GetObjectGuid().GetString().c_str() : "<NULL>", target->GetGUIDLow());
                }
                else
                {
                    target = static_cast<ChaseMovementGenerator<Creature> const*>(&*action)->GetTarget();
                    PSendSysMessage(LANG_MOVEGENS_CHASE_CREATURE, target ? target->GetObjectGuid().GetString().c_str() : "<NULL>", target->GetGUIDLow());
                }

                break;
            }
        case FOLLOW_MOTION_TYPE:
            {
                Unit* target = NULL;
                if(unit->GetTypeId()==TYPEID_PLAYER)
                {
                    target = static_cast<FollowMovementGenerator<Player> const*>(&*action)->GetTarget();
                    PSendSysMessage(LANG_MOVEGENS_FOLLOW_PLAYER, target ? target->GetObjectGuid().GetString().c_str() : "<NULL>", target->GetGUIDLow());
                }
                else
                {
                    target = static_cast<FollowMovementGenerator<Creature> const*>(&*action)->GetTarget();
                    PSendSysMessage(LANG_MOVEGENS_FOLLOW_CREATURE, target ? target->GetObjectGuid().GetString().c_str() : "<NULL>", target->GetGUIDLow());
                }

                break;
            }
        case HOME_MOTION_TYPE:
            if(unit->GetTypeId()==TYPEID_UNIT)
            {
                ((Creature*)unit)->GetRespawnCoord(x,y,z);
                PSendSysMessage(LANG_MOVEGENS_HOME_CREATURE,x,y,z);
            }
            else
                SendSysMessage(LANG_MOVEGENS_HOME_PLAYER);
            break;
        case POINT_MOTION_TYPE:
            {
                //static_cast<PointMovementGenerator<Creature> const*>(&*action)->GetDestination(x,y,z);
                //PSendSysMessage(LANG_MOVEGENS_POINT,x,y,z);
                break;
            }
        default:
            PSendSysMessage(LANG_MOVEGENS_UNKNOWN,action->Name());
            break;
        }
    }

    return true;
}

bool ChatHandler::HandlePLimitCommand(const char *args)
{
    if (*args)
    {
        char* param = strtok((char*)args, " ");
        if (!param)
            return false;

        int val = atoi(param);
        sWorld.SetPlayerLimit(val);

        // kick all low security level players
        if (!(sWorld.GetMinimumPermissionMask() & PERM_PLAYER))
            sWorld.KickAllWithoutPermissions(sWorld.GetMinimumPermissionMask());
    }

    uint32 pLimit = sWorld.GetPlayerAmountLimit();
    uint64 requiredPermissions = sWorld.GetMinimumPermissionMask();

    PSendSysMessage("Player limits: amount %u, required permissions %llu.", pLimit, requiredPermissions);

    return true;
}

bool ChatHandler::HandleCastCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if (!spell)
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo)
        return false;

    if (!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    char* trig_str = strtok(NULL, " ");
    if (trig_str)
    {
        int l = strlen(trig_str);
        if (strncmp(trig_str,"triggered",l) != 0)
            return false;
    }

    bool triggered = (trig_str != NULL);
    int32 bp0 = 0;
    int32 bp1 = 0;
    int32 bp2 = 0;
    if (char* Charbp0 = strtok (NULL, " "))
        bp0 = atoi(Charbp0);
    if (char* Charbp1 = strtok (NULL, " "))
        bp1 = atoi(Charbp1);
    if (char* Charbp2 = strtok (NULL, " "))
        bp2 = atoi(Charbp2);

    if (bp0 !=0 || bp1 != 0 || bp2 != 0)
        m_session->GetPlayer()->CastCustomSpell(target,spell,&bp0,&bp1,&bp2,triggered);
    else
        m_session->GetPlayer()->CastSpell(target,spell,triggered);

    return true;
}

bool ChatHandler::HandleCastBackCommand(const char* args)
{
    Unit* caster = getSelectedUnit();

    if (!caster)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if (!spell || !sSpellTemplate.LookupEntry<SpellEntry>(spell))
        return false;

    char* trig_str = strtok(NULL, " ");
    if (trig_str)
    {
        int l = strlen(trig_str);
        if (strncmp(trig_str,"triggered",l) != 0)
            return false;
    }

    bool triggered = (trig_str != NULL);

    // update orientation at server
    if (!caster->HasUnitState(UNIT_STAT_CANNOT_TURN))
        caster->SetFacingTo(caster->GetOrientationTo(m_session->GetPlayer()));

    caster->CastSpell(m_session->GetPlayer(),spell,triggered);
    return true;
}

bool ChatHandler::HandleCastDistCommand(const char* args)
{
    if (!*args)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if (!spell)
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo)
        return false;

    if (!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    char *distStr = strtok(NULL, " ");

    float dist = 0;

    if (distStr)
        sscanf(distStr, "%f", &dist);

    char* trig_str = strtok(NULL, " ");
    if (trig_str)
    {
        int l = strlen(trig_str);
        if (strncmp(trig_str,"triggered",l) != 0)
            return false;
    }

    bool triggered = (trig_str != NULL);

    float x,y,z;
    m_session->GetPlayer()->GetNearPoint(x,y,z,dist);

    m_session->GetPlayer()->CastSpell(x,y,z,spell,triggered);
    return true;
}

bool ChatHandler::HandleCastTargetCommand(const char* args)
{
    Unit* caster = getSelectedUnit();

    if (!caster)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!caster->GetVictim())
    {
        SendSysMessage(LANG_SELECTED_TARGET_NOT_HAVE_VICTIM);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if (!spell || !sSpellTemplate.LookupEntry<SpellEntry>(spell))
        return false;

    char* trig_str = strtok(NULL, " ");
    if (trig_str)
    {
        int l = strlen(trig_str);
        if (strncmp(trig_str,"triggered",l) != 0)
            return false;
    }

    bool triggered = (trig_str != NULL);

    // update orientation at server
    if (!caster->HasUnitState(UNIT_STAT_CANNOT_TURN))
        caster->SetFacingTo(caster->GetOrientationTo(m_session->GetPlayer()));

    caster->CastSpell(caster->GetVictim(),spell,triggered);

    return true;
}

/*
ComeToMe command REQUIRED for 3rd party scripting library to have access to PointMovementGenerator
Without this function 3rd party scripting library will get linking errors (unresolved external)
when attempting to use the PointMovementGenerator
*/
bool ChatHandler::HandleComeToMeCommand(const char *args)
{
    Creature* caster = getSelectedCreature();
    if (!caster)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* pl = m_session->GetPlayer();

    caster->GetMotionMaster()->MovePoint(0, pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ());
    return true;
}

bool ChatHandler::HandleCastSelfCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if (!spell)
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo)
        return false;

    if (!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    target->CastSpell(target,spell,false);

    return true;
}
bool ChatHandler::HandleCastNullCommand(const char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();

    if (!target)
        target = m_session->GetPlayer();

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = extractSpellIdFromLink((char*)args);
    if (!spell)
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo)
        return false;

    if (!SpellMgr::IsSpellValid(spellInfo,m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spell);
        SetSentErrorMessage(true);
        return false;
    }

    target->CastSpell((Unit*)NULL,spell,false);

    return true;
}

bool ChatHandler::HandleInstanceListBindsCommand(const char* /*args*/)
{
    Player* player = getSelectedPlayer();
    if (!player) player = m_session->GetPlayer();
    uint32 counter = 0;
    for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
    {
        Player::BoundInstancesMap &binds = player->GetBoundInstances(i);
        for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end(); ++itr)
        {
            InstanceSave *save = itr->second.save;
            std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
            PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetSaveInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
            counter++;
        }
    }
    PSendSysMessage("player binds: %d", counter);
    counter = 0;
    Group *group = player->GetGroup();
    if (group)
    {
        for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        {
            Group::BoundInstancesMap &binds = group->GetBoundInstances(i);
            for (Group::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end(); ++itr)
            {
                InstanceSave *save = itr->second.save;
                std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
                PSendSysMessage("map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetSaveInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                counter++;
            }
        }
    }
    PSendSysMessage("group binds: %d", counter);

    return true;
}

bool ChatHandler::HandleInstanceUnbindCommand(const char* args)
{
    if (!*args)
        return false;

    std::string cmd = args;
    if (cmd == "all")
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
                    PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetSaveInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                    player->UnbindInstance(itr, i);
                    counter++;
                }
                else
                    ++itr;
            }
        }
        PSendSysMessage("instances unbound: %d", counter);
    }
    else
    {
        Player* player = getSelectedPlayer();
        if (!player) player = m_session->GetPlayer();
        uint32 unbInst = 0;
        for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        {
            Player::BoundInstancesMap &binds = player->GetBoundInstances(i);
            for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end(); ++itr)
            {
                if (itr->first != player->GetMapId())
                {
                    InstanceSave *save = itr->second.save;
                    char * tmp = new char[20];
                    sprintf(tmp, "%u", save->GetSaveInstanceId());
                    std::string instId = tmp;

                    if (cmd == instId)
                    {
                        unbInst = save->GetSaveInstanceId();
                        std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
                        PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetSaveInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                        player->UnbindInstance(itr, i);
                        break;
                    }
                }
            }
        }

        if (unbInst)
            PSendSysMessage("instance unbound: %d", unbInst);
        else
            PSendSysMessage("instance id %s not found or player in instance", args);
    }

    return true;
}

bool ChatHandler::HandleInstanceSelfUnbindCommand(const char* args)
{
    if (!*args)
        return false;

    std::string cmd = args;
    if (cmd == "all")
    {
        Player* player = m_session->GetPlayer();
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
                    PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetSaveInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                    player->UnbindInstance(itr, i);
                    counter++;
                }
                else
                    ++itr;
            }
        }
        PSendSysMessage("instances unbound: %d", counter);
    }
    else
    {
        Player* player = m_session->GetPlayer();
        uint32 unbInst = 0;
        for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        {
            Player::BoundInstancesMap &binds = player->GetBoundInstances(i);
            for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end(); ++itr)
            {
                if (itr->first != player->GetMapId())
                {
                    InstanceSave *save = itr->second.save;
                    char * tmp = new char[20];
                    sprintf(tmp, "%u", save->GetSaveInstanceId());
                    std::string instId = tmp;

                    if (cmd == instId)
                    {
                        unbInst = save->GetSaveInstanceId();
                        std::string timeleft = GetTimeString(save->GetResetTime() - time(NULL));
                        PSendSysMessage("unbinding map: %d inst: %d perm: %s diff: %s canReset: %s TTR: %s", itr->first, save->GetSaveInstanceId(), itr->second.perm ? "yes" : "no",  save->GetDifficulty() == DIFFICULTY_NORMAL ? "normal" : "heroic", save->CanReset() ? "yes" : "no", timeleft.c_str());
                        player->UnbindInstance(itr, i);
                        break;
                    }
                }
            }
        }

        if (unbInst)
            PSendSysMessage("instance unbound: %d", unbInst);
        else
            PSendSysMessage("instance id %s not found or player in instance", args);
    }

    return true;
}

bool ChatHandler::HandleInstanceStatsCommand(const char* /*args*/)
{
    PSendSysMessage("instances loaded: %d", sMapMgr.GetNumInstances());
    PSendSysMessage("players in instances: %d", sMapMgr.GetNumPlayersInInstances());
    PSendSysMessage("instance saves: %d", sInstanceSaveManager.GetNumInstanceSaves());
    PSendSysMessage("players bound: %d", sInstanceSaveManager.GetNumBoundPlayersTotal());
    PSendSysMessage("groups bound: %d", sInstanceSaveManager.GetNumBoundGroupsTotal());
    return true;
}

bool ChatHandler::HandleInstanceSaveDataCommand(const char * /*args*/)
{
    Player* pl = m_session->GetPlayer();

    Map* map = pl->GetMap();
    if (!map->IsDungeon())
    {
        SendSysMessage("Map is not a dungeon.");
        SetSentErrorMessage(true);
        return false;
    }

    if (!((InstanceMap*)map)->GetInstanceData())
    {
        SendSysMessage("Map has no instance data.");
        SetSentErrorMessage(true);
        return false;
    }

    ((InstanceMap*)map)->GetInstanceData()->SaveToDB();
    return true;
}

bool ChatHandler::HandleInstanceBindCommand(const char* args)
{
    if (!*args)
        return false;

    int32 InstanceId = atoi((char*)args);

    Player* player = m_session->GetPlayer();
    if (InstanceSave *save = sInstanceSaveManager.GetInstanceSave(InstanceId))
        player->BindToInstance(save, true, false);
    else
    {
        SendSysMessage("There is no such instance save");
        return false;
    }

    SendSysMessage("You have been binded successfully");
    return true;
}

bool ChatHandler::HandleInstanceResetEncountersCommand(const char* args)
{
    if (!*args)
        return false;

    int32 InstanceId = atoi((char*)args);

    Player* player = m_session->GetPlayer();
    if (InstanceSave *save = sInstanceSaveManager.GetInstanceSave(InstanceId))
    {
        if (Map* map = sMapMgr.FindMap(save->GetMapId(), save->GetSaveInstanceId()))
        {
            if (InstanceData* data = reinterpret_cast<InstanceMap*>(map)->GetInstanceData())
            {
                data->ResetEncounterInProgress();
                SendSysMessage("You have called ResetEncounters successfully.");
                return true;
            }
        }
    }
    else
    {
        SendSysMessage("There is no instance save for that instance id!");
        return false;
    }

    SendSysMessage("Something went wrong, there were no such map or map didn't have instance data.");
    return false;
}

bool ChatHandler::HandleInstanceInsidePlayersCommand(const char* args)
{
    if (!*args)
        return false;

    int32 InstanceId = atoi((char*)args);

    Player* player = m_session->GetPlayer();
    if (InstanceSave *save = sInstanceSaveManager.GetInstanceSave(InstanceId))
    {
        if (Map* map = sMapMgr.FindMap(save->GetMapId(), save->GetSaveInstanceId()))
        {
            PSendSysMessage("Players count inside into instance: %d (MapID: %d, InstanceID: %d)", map->GetPlayersCountExceptGMs(), save->GetMapId(), save->GetSaveInstanceId());
            return true;
        }
    }
    else
    {
        SendSysMessage("There is no instance save for that instance id!");
        return false;
    }

    SendSysMessage("Something went wrong, there were no such map or map didn't have instance data.");
    return false;
}

bool ChatHandler::HandleInstanceGetData(const char* args)
{
    if (!*args)
        return false;

    int32 InstanceId = atoi((char*)args);

    if (InstanceSave *save = sInstanceSaveManager.GetInstanceSave(InstanceId))
    {
        if (Map* map = sMapMgr.FindMap(save->GetMapId(), save->GetSaveInstanceId()))
        {
            if (InstanceData* data = reinterpret_cast<InstanceMap*>(map)->GetInstanceData())
            {
                PSendSysMessage("Encounters in started state for MapId: %d, InstanceID: %d:", save->GetMapId(), save->GetSaveInstanceId());
                for (uint32 i = 0; i < 100;/*there are never so many encounters, just to be sure we've got all*/ ++i)
                    if (uint32 enc = data->GetData(i)) // 0 won't be sent
                        PSendSysMessage("Encounter number: %u, encounter state %u", i, enc);
                return true;
            }
        }
    }
    else
    {
        SendSysMessage("There is no instance save for that instance id!");
        return false;
    }

    SendSysMessage("Something went wrong, there were no such map or map didn't have instance data.");
    return false;
}

bool ChatHandler::HandleInstanceSetData(const char* args)
{
	if (!*args)
		return false;

	char* a = strtok((char*)args, " ");
	char* b = strtok(NULL, " ");
	char* c = strtok(NULL, " ");

	if (!a || !b || !c)
		return false;

	uint32 InstanceId = atoi((char*)a);
	uint32 encounter = atoi((char*)b);
	uint32 setdata = atoi((char*)c);

	if (InstanceSave *save = sInstanceSaveManager.GetInstanceSave(InstanceId))
	{
		if (Map* map = sMapMgr.FindMap(save->GetMapId(), save->GetSaveInstanceId()))
		{
			if (InstanceData* data = reinterpret_cast<InstanceMap*>(map)->GetInstanceData())
			{
				PSendSysMessage("Trying to set encounter %u to %u...", encounter, setdata);
				data->SetData(encounter, setdata);
				PSendSysMessage("Encounter %u is %u", encounter, data->GetData(encounter));
				PSendSysMessage(data->GetData(encounter) == setdata ? "SUCCESS" : "ERROR");
			}
		}

		return true;
	}
	else
	{
		SendSysMessage("There is no instance save for that instance id!");
		return false;
	}

	SendSysMessage("Something went wrong, there were no such map or map didn't have instance data.");
	return false;
}

/// Display the list of GMs
/// Display the list of GMs
bool ChatHandler::HandleGMListFullCommand(const char* /*args*/)
{
    return false;
    // disabled
    
    ///- Get the accounts with GM Level >0
    //QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT username, permission_mask FROM account JOIN account_permission WHERE permission_mask & '%u' AND realm_id = '%u'", PERM_GMT_DEV, realmID);
    //if (result)
    //{
    //    SendSysMessage(LANG_GMLIST);
    //    SendSysMessage("========================");
    //    SendSysMessage(LANG_GMLIST_HEADER);
    //    SendSysMessage("========================");

    //    ///- Circle through them. Display username and GM level
    //    do
    //    {
    //        Field *fields = result->Fetch();
    //        PSendSysMessage("|%15s|%6s|", fields[0].GetString(),fields[1].GetString());
    //    }while (result->NextRow());

    //    SendSysMessage("========================");
    //}
    //else
    //    SendSysMessage(LANG_GMLIST_EMPTY);
    //return true;
}

/// Define the 'Message of the day' for the realm
bool ChatHandler::HandleServerSetMotdCommand(const char* args)
{
    sWorld.SetMotd(args);
    PSendSysMessage(LANG_MOTD_NEW, args);
    return true;
}

/// Set/Unset the expansion level for an account
bool ChatHandler::HandleAccountSetAddonCommand(const char* args)
{
    ///- Get the command line arguments
    char *szAcc = strtok((char*)args," ");
    char *szExp = strtok(NULL," ");

    if (!szAcc)
        return false;

    std::string account_name;
    uint32 account_id;

    if (!szExp)
    {
        Player* player = getSelectedPlayer();
        if (!player)
            return false;

        account_id = player->GetSession()->GetAccountId();
        AccountMgr::GetName(account_id,account_name);
        szExp = szAcc;
    }
    else
    {
        ///- Convert Account name to Upper Format
        account_name = szAcc;
        if (!AccountMgr::normalizeString(account_name))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        account_id = AccountMgr::GetId(account_name);
        if (!account_id)
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        }
    }

    int lev=atoi(szExp);                                    //get int anyway (0 if error)
    if (lev < 0)
        return false;

    // No SQL injection
    AccountsDatabase.PExecute("UPDATE account SET expansion_id = '%u' WHERE account_id = '%u'", lev, account_id);
    PSendSysMessage(LANG_ACCOUNT_SETADDON, account_name.c_str(), account_id, lev);
    return true;
}

//Send items by mail
bool ChatHandler::HandleSendItemsCommand(const char* args)
{
    if (!*args)
        return false;

    // format: name "subject text" "mail text" item1[:count1] item2[:count2] ... item12[:count12]

    char* pName = strtok((char*)args, " ");
    if (!pName)
        return false;

    char* tail1 = strtok(NULL, "");
    if (!tail1)
        return false;

    char* msgSubject;
    if (*tail1=='"')
        msgSubject = strtok(tail1+1, "\"");
    else
    {
        char* space = strtok(tail1, "\"");
        if (!space)
            return false;
        msgSubject = strtok(NULL, "\"");
    }

    if (!msgSubject)
        return false;

    char* tail2 = strtok(NULL, "");
    if (!tail2)
        return false;

    char* msgText;
    if (*tail2=='"')
        msgText = strtok(tail2+1, "\"");
    else
    {
        char* space = strtok(tail2, "\"");
        if (!space)
            return false;
        msgText = strtok(NULL, "\"");
    }

    if (!msgText)
        return false;

    // pName, msgSubject, msgText isn't NUL after prev. check
    std::string name    = pName;
    std::string subject = msgSubject;
    std::string text    = msgText;

    // extract items
    typedef std::pair<uint32,uint32> ItemPair;
    typedef std::list< ItemPair > ItemPairs;
    ItemPairs items;

    // get all tail string
    char* tail = strtok(NULL, "");

    // get from tail next item str
    while (char* itemStr = strtok(tail, " "))
    {
        // and get new tail
        tail = strtok(NULL, "");

        // parse item str
        char* itemIdStr = strtok(itemStr, ":");
        char* itemCountStr = strtok(NULL, " ");

        uint32 item_id = atoi(itemIdStr);
        if (!item_id)
            return false;

        ItemPrototype const* item_proto = ObjectMgr::GetItemPrototype(item_id);
        if (!item_proto)
        {
            PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
            SetSentErrorMessage(true);
            return false;
        }

        uint32 item_count = itemCountStr ? atoi(itemCountStr) : 1;
        if (item_count < 1 || item_proto->MaxCount && item_count > item_proto->MaxCount)
        {
            PSendSysMessage(LANG_COMMAND_INVALID_ITEM_COUNT, item_count,item_id);
            SetSentErrorMessage(true);
            return false;
        }

        while (item_count > item_proto->GetMaxStackSize())
        {
            items.push_back(ItemPair(item_id,item_proto->GetMaxStackSize()));
            item_count -= item_proto->GetMaxStackSize();
        }

        items.push_back(ItemPair(item_id,item_count));

        if (items.size() > MAX_MAIL_ITEMS)
        {
            PSendSysMessage(LANG_COMMAND_MAIL_ITEMS_LIMIT, MAX_MAIL_ITEMS);
            SetSentErrorMessage(true);
            return false;
        }
    }

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 receiver_guid = sObjectMgr.GetPlayerGUIDByName(name);
    if (!receiver_guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // from console show not existed sender
    MailSender sender(MAIL_NORMAL,m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM);

    uint32 itemTextId = !text.empty() ? sObjectMgr.CreateItemText(text) : 0;

    Player *receiver = sObjectMgr.GetPlayerInWorld(receiver_guid);

    // fill mail
    MailDraft draft(subject, itemTextId);

    for (ItemPairs::const_iterator itr = items.begin(); itr != items.end(); ++itr)
    {
        if (Item* item = Item::CreateItem(itr->first,itr->second,m_session ? m_session->GetPlayer() : 0))
        {
            item->SetOwnerGUID(receiver_guid);
            item->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted
            draft.AddItem(item);
        }
    }

    draft.SendMailTo(MailReceiver(receiver, ObjectGuid(receiver_guid)), sender);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());
    return true;
}

///Send money by mail
bool ChatHandler::HandleSendMoneyCommand(const char* args)
{
    if (!*args)
        return false;

    /// format: name "subject text" "mail text" money

    char* pName = strtok((char*)args, " ");
    if (!pName)
        return false;

    char* tail1 = strtok(NULL, "");
    if (!tail1)
        return false;

    char* msgSubject;
    if (*tail1=='"')
        msgSubject = strtok(tail1+1, "\"");
    else
    {
        char* space = strtok(tail1, "\"");
        if (!space)
            return false;
        msgSubject = strtok(NULL, "\"");
    }

    if (!msgSubject)
        return false;

    char* tail2 = strtok(NULL, "");
    if (!tail2)
        return false;

    char* msgText;
    if (*tail2=='"')
        msgText = strtok(tail2+1, "\"");
    else
    {
        char* space = strtok(tail2, "\"");
        if (!space)
            return false;
        msgText = strtok(NULL, "\"");
    }

    if (!msgText)
        return false;

    char* money_str = strtok(NULL, "");
    int32 money = money_str ? atoi(money_str) : 0;
    if (money <= 0)
        return false;

    // pName, msgSubject, msgText isn't NUL after prev. check
    std::string name    = pName;
    std::string subject = msgSubject;
    std::string text    = msgText;

    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 receiver_guid = sObjectMgr.GetPlayerGUIDByName(name);
    if (!receiver_guid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // from console show not existed sender
    MailSender sender(MAIL_NORMAL,m_session ? m_session->GetPlayer()->GetGUIDLow() : 0, MAIL_STATIONERY_GM);

    uint32 itemTextId = !text.empty() ? sObjectMgr.CreateItemText(text) : 0;

    Player *receiver = sObjectMgr.GetPlayerInWorld(receiver_guid);

    MailDraft(subject, itemTextId)
        .SetMoney(money)
        .SendMailTo(MailReceiver(receiver, ObjectGuid(receiver_guid)), sender);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());
    return true;
}

/// Send a message to a player in game
bool ChatHandler::HandleSendMessageCommand(const char* args)
{
    ///- Get the command line arguments
    char* name_str = strtok((char*)args, " ");
    char* msg_str = strtok(NULL, "");

    if (!name_str || !msg_str)
        return false;

    std::string name = name_str;

    if (!normalizePlayerName(name))
        return false;

    ///- Find the player and check that he is not logging out.
    Player *rPlayer = sObjectMgr.GetPlayerInWorld(name.c_str());
    if (!rPlayer)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (rPlayer->GetSession()->isLogingOut())
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    ///- Send the message
    //Use SendAreaTriggerMessage for fastest delivery.
    rPlayer->GetSession()->SendAreaTriggerMessage("%s", msg_str);
    rPlayer->GetSession()->SendAreaTriggerMessage("|cffff0000[Message from administrator]:|r");

    //Confirmation message
    PSendSysMessage(LANG_SENDMESSAGE,name.c_str(),msg_str);
    return true;
}

bool ChatHandler::HandleFlushArenaPointsCommand(const char* args)
{
    // who did this and for what?
    //if (strcmp(args, "i know what i am doing") != 0)
    //{
    //    SendSysMessage("You know what are you doing?");
    //    SetSentErrorMessage(true);
    //    return false;
    //}
    sBattleGroundMgr.DistributeArenaPoints();
    return true;
}

bool ChatHandler::HandleModifyGenderCommand(const char *args)
{
    if (!*args)
        return false;

    Player *player = getSelectedPlayer();

    if (!player)
    {
        SendSysMessage(LANG_NO_PLAYER);
        SetSentErrorMessage(true);
        return false;
    }

    char const* gender_str = (char*)args;
    int gender_len = strlen(gender_str);

    uint32 displayId = player->GetNativeDisplayId();
    char const* gender_full = NULL;
    uint32 new_displayId = displayId;
    Gender gender;

    if (!strncmp(gender_str,"male",gender_len))              // MALE
    {
        if (player->GetGender() == GENDER_MALE)
            return true;

        gender_full = "male";
        new_displayId = player->GetRace() == RACE_BLOODELF ? displayId+1 : displayId-1;
        gender = GENDER_MALE;
    }
    else if (!strncmp(gender_str,"female",gender_len))      // FEMALE
    {
        if (player->GetGender() == GENDER_FEMALE)
            return true;

        gender_full = "female";
        new_displayId = player->GetRace() == RACE_BLOODELF ? displayId-1 : displayId+1;
        gender = GENDER_FEMALE;
    }
    else
    {
        SendSysMessage(LANG_MUST_MALE_OR_FEMALE);
        SetSentErrorMessage(true);
        return false;
    }

    // Set gender
    player->SetByteValue(UNIT_FIELD_BYTES_0, 2, gender);
    player->SetByteValue(PLAYER_BYTES_3, 0, gender);

    // Change display ID
    player->SetDisplayId(new_displayId);
    player->SetNativeDisplayId(new_displayId);

    PSendSysMessage(LANG_YOU_CHANGE_GENDER, player->GetName(),gender_full);
    if (needReportToTarget(player))
        ChatHandler(player).PSendSysMessage(LANG_YOUR_GENDER_CHANGED, gender_full,GetName());
    return true;
}

/*------------------------------------------
 *-------------TRINITY----------------------
 *-------------------------------------*/

bool ChatHandler::HandlePlayAllCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 soundId = atoi((char*)args);

    if (!sSoundEntriesStore.LookupEntry(soundId))
    {
        PSendSysMessage(LANG_SOUND_NOT_EXIST, soundId);
        SetSentErrorMessage(true);
        return false;
    }

    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(soundId) << m_session->GetPlayer()->GetGUID();
    sWorld.SendGlobalMessage(&data);

    PSendSysMessage(LANG_COMMAND_PLAYED_TO_ALL, soundId);
    return true;
}

bool ChatHandler::HandlePlaySelfCommand(const char* args)
{
    if (!*args || !m_session->GetPlayer())
        return false;

    uint32 soundId = atoi((char*)args);

    if (!sSoundEntriesStore.LookupEntry(soundId))
    {
        PSendSysMessage(LANG_SOUND_NOT_EXIST, soundId);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->GetPlayer()->SendPlaySound(soundId, true);
    return true;
}

bool ChatHandler::HandleFreezeCommand(const char *args)
{
    std::string name;
    Player* player;
    char* TargetName = strtok((char*)args, " "); //get entered name
    if (!TargetName) //if no name entered use target
    {
        player = getSelectedPlayer();
        if (player) //prevent crash with creature as target
        {
            name = player->GetName();
            normalizePlayerName(name);
        }
    }
    else // if name entered
    {
        name = TargetName;
        normalizePlayerName(name);
        player = sObjectMgr.GetPlayerInWorld(name.c_str()); //get player by name
    }

    if (!player)
    {
        SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
        return true;
    }

    if (player == m_session->GetPlayer() || player->GetSession()->GetPermissions() > m_session->GetPermissions())
    {
        SendSysMessage(LANG_COMMAND_FREEZE_ERROR);
        return true;
    }

    //effect
    if ((player) && (!(player==m_session->GetPlayer())))
    {
        PSendSysMessage(LANG_COMMAND_FREEZE,name.c_str());

        //stop combat + make player unattackable + duel stop + stop some spells
        player->setFaction(35);
        player->CombatStop();
        if (player->IsNonMeleeSpellCast(true))
            player->InterruptNonMeleeSpells(true);
        player->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        player->SetUInt32Value(PLAYER_DUEL_TEAM, 1);

        //if player class = hunter || warlock remove pet if alive
        if ((player->GetClass() == CLASS_HUNTER) || (player->GetClass() == CLASS_WARLOCK))
        {
            if (Pet* pet = player->GetPet())
            {
                pet->SavePetToDB(PET_SAVE_AS_CURRENT);
                // not let dismiss dead pet
                if (pet && pet->isAlive())
                    player->RemovePet(pet,PET_SAVE_NOT_IN_SLOT);
            }
        }

        //stop movement and disable spells
        uint32 spellID = 9454;
        //m_session->GetPlayer()->CastSpell(player,spellID,false);
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellID);
        if (spellInfo) //TODO: Change the duration of the aura to -1 instead of 5000000
        {
            for (uint32 i = 0;i<3;i++)
            {
                uint8 eff = spellInfo->Effect[i];
                if (eff>=TOTAL_SPELL_EFFECTS)
                    continue;
                if (eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA ||
                    eff == SPELL_EFFECT_PERSISTENT_AREA_AURA || eff == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND ||
                    eff == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY)
                {
                    Aura *Aur = CreateAura(spellInfo, i, NULL, player);
                    player->AddAura(Aur);
                }
            }
        }

        //save player
        player->SaveToDB();
    }
    return true;
}

bool ChatHandler::HandleUnFreezeCommand(const char *args)
{
    std::string name;
    Player* player;
    char* TargetName = strtok((char*)args, " "); //get entered name
    if (!TargetName) //if no name entered use target
    {
        player = getSelectedPlayer();
        if (player) //prevent crash with creature as target
        {
            name = player->GetName();
        }
    }

    else // if name entered
    {
        name = TargetName;
        normalizePlayerName(name);
        player = sObjectMgr.GetPlayerInWorld(name.c_str()); //get player by name
    }

    //effect
    if (player)
    {
        PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());

        //Reset player faction + allow combat + allow duels
        player->RestoreFaction();
        player->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

        //allow movement and spells
        uint32 spellID = 9454;
        player->RemoveAurasDueToSpell(spellID);

        //save player
        player->SaveToDB();
    }

    if (!player)
    {
        if (TargetName)
        {
            //check for offline players
            QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT characters.guid FROM `characters` WHERE characters.name = '%s'",name.c_str());
            if (!result)
            {
                SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                return true;
            }
            //if player found: delete his freeze aura
            Field *fields=result->Fetch();
            uint64 pguid = fields[0].GetUInt64();
            RealmDataDatabase.PQuery("DELETE FROM `character_aura` WHERE character_aura.spell = 9454 AND character_aura.guid = '%llu'",pguid);
            PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());
            return true;
        }
        else
        {
            SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
            return true;
        }
    }

    return true;
}

bool ChatHandler::HandleListFreezeCommand(const char* /*args*/)
{
    //Get names from DB
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT characters.name FROM `characters` LEFT JOIN `character_aura` ON (characters.guid = character_aura.guid) WHERE character_aura.spell = 9454");
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_NO_FROZEN_PLAYERS);
        return true;
    }
    //Header of the names
    SendSysMessage(LANG_COMMAND_LIST_FREEZE);

    //Output of the results
    do
    {
        Field *fields = result->Fetch();
        std::string fplayers = fields[0].GetCppString();
        PSendSysMessage(LANG_COMMAND_FROZEN_PLAYERS,fplayers.c_str());
    } while (result->NextRow());

    return true;
}

bool ChatHandler::HandleGroupLeaderCommand(const char* args)
{
    Player* plr  = NULL;
    Group* group = NULL;
    uint64 guid  = 0;
    char* cname  = strtok((char*)args, " ");

    if (GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
        if (group && group->GetLeaderGUID() != guid)
            group->ChangeLeader(guid);

    return true;
}

bool ChatHandler::HandleGroupDisbandCommand(const char* args)
{
    Player* plr  = NULL;
    Group* group = NULL;
    uint64 guid  = 0;
    char* cname  = strtok((char*)args, " ");

    if (GetPlayerGroupAndGUIDByName(cname, plr, group, guid))
        if (group)
            group->Disband();

    return true;
}

bool ChatHandler::HandleGroupRemoveCommand(const char* args)
{
    Player* plr  = NULL;
    Group* group = NULL;
    uint64 guid  = 0;
    char* cname  = strtok((char*)args, " ");

    if (GetPlayerGroupAndGUIDByName(cname, plr, group, guid, true))
        if (group)
            group->RemoveMember(guid, 0);

    return true;
}

bool ChatHandler::HandlePossessCommand(const char* /*args*/)
{
    Unit* pUnit = getSelectedUnit();
    if (!pUnit)
        return false;

    m_session->GetPlayer()->CastSpell(pUnit, 530, true);
    return true;
}

bool ChatHandler::HandleUnPossessCommand(const char* /*args*/)
{
    Unit* pUnit = getSelectedUnit();
    if (!pUnit) pUnit = m_session->GetPlayer();

    pUnit->RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM);
    pUnit->RemoveSpellsCausingAura(SPELL_AURA_AOE_CHARM);
    pUnit->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS_PET);
    pUnit->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS);

    return true;
}

bool ChatHandler::HandleBindFollowCommand(const char* /*args*/)
{
    Unit* pUnit = getSelectedUnit();
    if (!pUnit)
        return false;

    if (pUnit->GetTypeId() != TYPEID_PLAYER)
        return false;

    Player *gamemaster = m_session->GetPlayer();

    gamemaster->setFollowTarget(pUnit->GetGUID());
    ((Player *)pUnit)->setGMFollow(gamemaster->GetGUID());
    gamemaster->GetMotionMaster()->MoveFollow(pUnit, 1.0f, M_PI);
    return true;
}

bool ChatHandler::HandleUnbindFollowCommand(const char* /*args*/)
{
    Player *gamemaster = m_session->GetPlayer();
    if (gamemaster->getFollowTarget() == 0)
        return false;

    Player *pTarget = Unit::GetPlayerInWorld(gamemaster->getFollowTarget());
    if (pTarget)
        pTarget->setGMFollow(0);
    gamemaster->setFollowTarget(0);
    gamemaster->GetUnitStateMgr().InitDefaults(true);
    return true;
}

bool ChatHandler::HandleBindSightCommand(const char* /*args*/)
{
    Unit* pUnit = getSelectedUnit();
    if (!pUnit)
        return false;

    m_session->GetPlayer()->SetFarsightTarget(pUnit);
    return true;
}

bool ChatHandler::HandleUnbindSightCommand(const char* /*args*/)
{
    m_session->GetPlayer()->ClearFarsight();
    //pUnit->RemovePlayerFromVision(m_session->GetPlayer());
    return true;
}

bool ChatHandler::HandleGameObjectGridCommand(const char* /*args*/)
{
    std::list<GameObject*> tmpL;
    Hellground::AllGameObjectsInRange go_check(m_session->GetPlayer(), 20.0f);
    Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsInRange> searcher(tmpL, go_check);

    Cell::VisitGridObjects(m_session->GetPlayer(), searcher, 20.0f);

    SendSysMessage("All Game Objects in 20 yd range:");
    SendSysMessage("--------------------------------");
    for (std::list<GameObject*>::const_iterator itr = tmpL.begin(); itr != tmpL.end(); ++itr)
    {
        GameObject * tmp = *itr;
        if (tmp)
        {
            GameObject * tmp2 = tmp->GetMap()->GetGameObject(tmp->GetGUID());
            GameObjectInfo const * gInfo = ObjectMgr::GetGameObjectInfo(tmp->GetEntry());

            if (!gInfo)
                continue;

            PSendSysMessage(LANG_GO_LIST_CHAT, tmp->GetGUID(), tmp->GetGUIDLow(), gInfo->name, tmp->GetPositionX(), tmp->GetPositionY(), tmp->GetPositionZ(), tmp->GetMapId());
            PSendSysMessage("is in world: %s | is in map guid store: %s", tmp->IsInWorld() ? "Yes" : "No", tmp2 ? "Yes" : "No");
            SendSysMessage("--------------------------------");
        }
        else
            SendSysMessage("Broken Pointer");
    }

    return true;
}

bool ChatHandler::HandleReloadEventAICommand(const char* args)
{
    char * cId = strtok((char*)args, " ");

    uint32 creatureId = 0;

    if (cId)
        creatureId = atoi(cId);
    else if (getSelectedCreature())
        creatureId = getSelectedCreature()->GetEntry();

    if (!creatureId)
    {
        SendSysMessage("You should select creature or enter id to reload");
        return false;
    }

    sCreatureEAIMgr.LoadCreatureEventAI_Scripts(creatureId);

    CreatureAIReInitialize[creatureId] = WorldTimer::getMSTime();

    PSendSysMessage("EventAI for creature %u prepared to replace", creatureId);

    return true;
}

bool ChatHandler::HandleChannelPassCommand(const char* args)
{
    if (!args)
        return false;

    char * chName = strtok((char*)args, " ");
    char * chPass = strtok(NULL, "");

    if (!chName || !chPass)
        return false;

    ChannelMgr* cMgr = channelMgr(m_session->GetPlayer()->GetTeam());

    if (!cMgr)
        return false;

    Channel *chn = cMgr->GetChannel(chName, m_session->GetPlayer());

    if (!chn)
    {
        PSendSysMessage("Channel %s not found.", chName);
        return false;
    }

    chn->SetPassword(chPass);

    PSendSysMessage("Password for channel %s was succesfully set to %s", chName, chPass);

    return true;
}

bool ChatHandler::HandleChannelKickCommand(const char* args)
{
    if (!args)
        return false;

    char * chName = strtok((char*)args, " ");
    char * player = strtok(NULL, "");

    if (!chName || !player)
        return false;

    std::string playerName = player;

    normalizePlayerName(playerName);

    ChannelMgr* cMgr = channelMgr(m_session->GetPlayer()->GetTeam());

    if (!cMgr)
        return false;

    Channel *chn = cMgr->GetChannel(chName, m_session->GetPlayer());

    if (!chn)
    {
        PSendSysMessage("Channel %s not found.", chName);
        return false;
    }

    chn->Kick(m_session->GetPlayer()->GetGUID(), playerName.c_str());

    PSendSysMessage("Player %s was succesfully kicked from channel %s", playerName.c_str(), chName);

    return true;
}

bool ChatHandler::HandleChannelMassKickCommand(const char* args)
{
    if (!args)
        return false;

    if (!m_session->GetPlayer()->isGameMaster())
    {
        SendSysMessage("You must have GM mode ON to use this command");
        return false;
    }

    char * chName = strtok((char*)args, " ");

    if (!chName)
        return false;

    ChannelMgr* cMgr = channelMgr(m_session->GetPlayer()->GetTeam());

    if (!cMgr)
        return false;

    Channel *chn = cMgr->GetChannel(chName, m_session->GetPlayer());

    if (!chn)
    {
        PSendSysMessage("Channel %s not found.", chName);
        return false;
    }

    std::list<uint64> tmpPlList = chn->GetPlayers();

    for (std::list<uint64>::const_iterator itr = tmpPlList.begin(); itr != tmpPlList.end(); ++itr)
    {
        Player * tmpPl = ObjectAccessor::GetPlayerInWorldOrNot(*itr);
        if (tmpPl && !tmpPl->isGameMaster())
            chn->Kick(m_session->GetPlayer()->GetGUID(), tmpPl->GetName());
    }

    PSendSysMessage("Players was succesfully kicked from channel %s", chName);

    return true;
}

bool ChatHandler::HandleCrashMapCommand(const char * /*args*/)
{
    if (!m_session)
        return false;

    Player * tmpPlayer = m_session->GetPlayer();
    if (!tmpPlayer)
        return false;

    tmpPlayer->GetMap()->SetBroken(true);
    return false;
}

bool ChatHandler::HandleCrashServerCommand(const char * /*args*/)
{   
    *((uint32 volatile*)NULL) = 0;                       // bang crash
    return false;
}

bool ChatHandler::HandleCrashFreezeCommand(const char * /*args*/)
{
    for (uint32 i = 0; true; ++i)
        ++i; // freeze
    return false;
}

void memleak() {
    double *x = new double(999.99); //memleak! ~10KB/sec
}

bool ChatHandler::HandleCrashMemleakCommand(const char * /*args*/)
{
    for (uint32 i = 0; true; ++i) {
        memleak();
        if ((i % 1000) == 0) {
            sLog.outString("%u done",i);
        }
       //  Sleep(1);
    }
        
    return false;
}

bool ChatHandler::HandleMmap(const char* /*args*/)
{
    sWorld.setConfig(CONFIG_MMAP_ENABLED, sWorld.getConfig(CONFIG_MMAP_ENABLED) ^ 1);

    PSendSysMessage("mmaps are %sabled", sWorld.getConfig(CONFIG_MMAP_ENABLED) ? "en" : "dis");

    return true;
}

bool ChatHandler::HandleMmapTestArea(const char* args)
{
    float radius = 40.0f;
    //ExtractFloat(&args, radius);

    std::list<Creature*> creatureList;

    Hellground::AnyUnitInObjectRangeCheck go_check(m_session->GetPlayer(), radius);
    Hellground::ObjectListSearcher<Creature, Hellground::AnyUnitInObjectRangeCheck> go_search(creatureList, go_check);
    // Get Creatures
    Cell::VisitGridObjects(m_session->GetPlayer(), go_search, radius);

    if (!creatureList.empty())
    {
        PSendSysMessage("Found %llu creatures.", creatureList.size());

        uint32 paths = 0;
        uint32 uStartTime = WorldTimer::getMSTime();

        float gx,gy,gz;
        m_session->GetPlayer()->GetPosition(gx,gy,gz);
        for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
        {
            PathFinder path(*itr);
            path.calculate(gx, gy, gz);
            ++paths;
        }

        uint32 uPathLoadTime = WorldTimer::getMSTimeDiff(uStartTime, WorldTimer::getMSTime());
        PSendSysMessage("Generated %i paths in %i ms", paths, uPathLoadTime);
    }
    else
    {
        PSendSysMessage("No creatures in %f yard range.", radius);
    }

    return true;
}

bool ChatHandler::HandleMmapFollowArea(const char* /*args*/)
{
    float radius = 40.0f;
    //ExtractFloat(&args, radius);

    std::list<Creature*> creatureList;

    Hellground::AnyUnitInObjectRangeCheck go_check(m_session->GetPlayer(), radius);
    Hellground::ObjectListSearcher<Creature, Hellground::AnyUnitInObjectRangeCheck> go_search(creatureList, go_check);
    // Get Creatures
    Cell::VisitGridObjects(m_session->GetPlayer(), go_search, radius);

    if (!creatureList.empty())
    {
        PSendSysMessage("Found %llu creatures.", creatureList.size());

        uint32 paths = 0;
        uint32 uStartTime = WorldTimer::getMSTime();
        Player *gamemaster = m_session->GetPlayer();
        float gx,gy,gz;
        m_session->GetPlayer()->GetPosition(gx,gy,gz);
        for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
        {
            gamemaster->setFollowTarget((*itr)->GetGUID());
            ((Player *)(*itr))->setGMFollow(gamemaster->GetGUID());
            gamemaster->GetMotionMaster()->MoveFollow((*itr), 1.0f, M_PI);
        }

        uint32 uPathLoadTime = WorldTimer::getMSTimeDiff(uStartTime, WorldTimer::getMSTime());
        PSendSysMessage("Generated %i paths in %i ms", paths, uPathLoadTime);
    }
    else
    {
        PSendSysMessage("No creatures in %f yard range.", radius);
    }

    return true;
}

// Set friends for account
/*bool ChatHandler::HandleAccountFriendAddCommand(const char* args)
{
    ///- Get the command line arguments
    char* arg1 = strtok((char*) args, " ");
    if (arg1 == nullptr || arg1 == "")
        return false;
        
    uint32 targetAccountId = atoi(arg1);
    if (!targetAccountId)
        return false;

    uint32 friendAccountId = atoi(strtok(NULL, " "));

    if (!friendAccountId)
        return false;

    AccountOpResult result = AccountMgr::AddRAFLink(targetAccountId, friendAccountId);

    switch (result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_FRIEND);
            break;
        default:
            SendSysMessage(LANG_COMMAND_FRIEND_ERROR);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}*/

// Delete friends for account
/*bool ChatHandler::HandleAccountFriendDeleteCommand(const char* args)
{
    ///- Get the command line arguments
    char* arg1 = strtok((char*) args, " ");
    if (arg1 == nullptr || arg1 == "")
        return false;
        
    uint32 targetAccountId = atoi(arg1);
    if (!targetAccountId)
        return false;

    char* arg2 = strtok(NULL, " ");
    if (arg2 == nullptr || arg2 == "")
        return false;
        
    uint32 friendAccountId = atoi(arg2);
    if (!friendAccountId)
        return false;

    AccountOpResult result = AccountMgr::DeleteRAFLink(targetAccountId, friendAccountId);

    switch (result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_FRIEND);
            break;
        default:
            SendSysMessage(LANG_COMMAND_FRIEND_ERROR);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}*/

// List friends for account
/*bool ChatHandler::HandleAccountFriendListCommand(const char* args)
{
    uint32 targetAccountId = atoi(args);
    if (!targetAccountId)
        return false;

    std::vector<uint32> listed = AccountMgr::GetRAFAccounts(targetAccountId);
    for (uint32 i = 0; i < listed.size(); i++)
        PSendSysMessage("%u was reffered to %u", targetAccountId, listed[i]);
    
    listed = AccountMgr::GetRAFAccounts(targetAccountId, false);
    for (uint32 i = 0; i < listed.size(); i++)
        PSendSysMessage("%u was reffered to %u", listed[i], targetAccountId);

    return true;
}*/

bool ChatHandler::HandleDiffCommand(const char* /*args*/)
{
    if (!m_session->GetPlayer())
        return true;
    
    PSendSysMessage("Server Diff: %u", sWorld.GetUpdateTime());

    uint32 check_maps[6] = {0,1,530,MAP_SWP,MAP_ZA,MAP_BT};

    for (uint32 check_map : check_maps)
    {
        Map* map = sMapMgr.FindMap(check_map, 0);
        if (map)
            PSendSysMessage("Map Diff for map %u: %u", check_map, map->m_updateTracker.timeElapsed());
    }

    return true;
}

bool ChatHandler::HandleSendSpellVisualCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 spell = (uint32)atoi((char*)args);
    if (!spell)
        return false;

    m_session->GetPlayer()->SendSpellVisual(spell);
    return true;
}

bool ChatHandler::HandleDeletedCharactersCommand(const char* args)
{
    if (!*args)
        return false;

    char* cname = strtok((char*)args, "");
    if (!cname)
        return false;

    std::string account_name = cname;
    if (!AccountMgr::normalizeString(account_name))
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountId = AccountMgr::GetId(account_name);
    if (!accountId)
    {
        PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
        return true;
    }

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT d.id, d.oldname, c.race, c.class, c.totaltime from deleted_chars d JOIN characters c on char_guid=guid WHERE d.acc=%u LIMIT 30", accountId);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint32 DeletedId = fields[0].GetUInt32();
            std::string OldName = fields[1].GetCppString();
            uint8 race = fields[2].GetUInt8();
            uint8 _class = fields[3].GetUInt8();
            std::string race_s, Class_s;
            switch(race)
            {
                case RACE_HUMAN:            race_s = "Human";       break;
                case RACE_ORC:              race_s = "Orc";         break;
                case RACE_DWARF:            race_s = "Dwarf";       break;
                case RACE_NIGHTELF:         race_s = "Night Elf";   break;
                case RACE_UNDEAD_PLAYER:    race_s = "Undead";      break;
                case RACE_TAUREN:           race_s = "Tauren";      break;
                case RACE_GNOME:            race_s = "Gnome";       break;
                case RACE_TROLL:            race_s = "Troll";       break;
                case RACE_BLOODELF:         race_s = "Blood Elf";   break;
                case RACE_DRAENEI:          race_s = "Draenei";     break;
            }

            switch(_class)
            {
                case CLASS_WARRIOR:         Class_s = "Warrior";        break;
                case CLASS_PALADIN:         Class_s = "Paladin";        break;
                case CLASS_HUNTER:          Class_s = "Hunter";         break;
                case CLASS_ROGUE:           Class_s = "Rogue";          break;
                case CLASS_PRIEST:          Class_s = "Priest";         break;
                case CLASS_SHAMAN:          Class_s = "Shaman";         break;
                case CLASS_MAGE:            Class_s = "Mage";           break;
                case CLASS_WARLOCK:         Class_s = "Warlock";        break;
                case CLASS_DRUID:           Class_s = "Druid";          break;
            }
            uint32 totaltime = fields[4].GetUInt32();

            PSendSysMessage("Deleted char id: %u, oldname: %s, race: %s, class: %s, totaltime %u", DeletedId, OldName.c_str(), race_s.c_str(), Class_s.c_str(), totaltime);
        } while (result->NextRow());
    }
    else
        SendSysMessage("No deleted characters found");
    return true;
}

bool ChatHandler::HandleCharacterRestoreCommand(const char* args)
{
    if (!*args)
        return false;

    uint32 DeletedId = (uint32)atoi((char*)args);
    if (!DeletedId)
    {
        SendSysMessage("No ID provided");
        return false;
    }

    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT char_guid, oldname, acc from deleted_chars WHERE id=%u", DeletedId);
    if (!result)
    {
        SendSysMessage("Nothing found by provided ID");
        return false;
    }

    Field *fields = result->Fetch();
    uint32 DeletedGuid = fields[0].GetUInt32();
    std::string OldName = fields[1].GetCppString();
    uint32 AccountId = fields[2].GetUInt32();

    QueryResultAutoPtr resultacct = AccountsDatabase.PQuery("SELECT SUM(characters_count) FROM realm_characters WHERE account_id = '%d'", AccountId);
    if (!resultacct)
    {
        SendSysMessage("Account not found");
        return false;
    }

    Field *fields2=resultacct->Fetch();
    uint32 acctcharcount = fields2[0].GetUInt32();

    if (acctcharcount >= sWorld.getConfig(CONFIG_CHARACTERS_PER_ACCOUNT))
    {
        SendSysMessage("Already has max characters on account");
        return false;
    }

    QueryResultAutoPtr hasRenameFlag = RealmDataDatabase.PQuery("SELECT 1 FROM characters WHERE guid=%u AND at_login & 1", DeletedGuid);
    
    if (sObjectMgr.GetPlayerGUIDByName(OldName) && !hasRenameFlag)
    {
        SendSysMessage("There is a player with the same name and the deleted character has no Rename flag.");
        return false;
    }

    RealmDataDatabase.PExecute("UPDATE characters SET account = '%u' WHERE guid = '%u'", AccountId, DeletedGuid);
    /*if (sObjectMgr.GetPlayerGUIDByName(OldName))
        RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login | '1' WHERE guid = '%u'", DeletedGuid);*/
    RealmDataDatabase.PExecute("UPDATE characters SET name = '%s' WHERE guid = '%u'", OldName.c_str(), DeletedGuid);

    PSendSysMessage("Character guid %u with oldname %s has been restored to an account of %u", DeletedGuid, OldName.c_str(), AccountId);
    RealmDataDatabase.PExecute("DELETE FROM deleted_chars WHERE id=%u", DeletedId);
    sWorld.UpdateRealmCharCount(AccountId);
    return true;
}

// example of syntax(not working example): .fakepacket opcode length 167 244 0 164 12 19
// Bad implementation for now... Not even working?). Freezes the server!
bool ChatHandler::HandleFakePacketCommand(const char* args)
{

    Player* plra = getSelectedPlayer();
    if (plra)
    {
        char* chhh = strtok((char*)args, " ");
        char* chhh2 = strtok(NULL, " ");
        std::string name = chhh;
        name += " ";
        name += chhh2;
        plra->SetName(name);
    m_session->SendNameQueryOpcode(plra);
    }
    return false; // i dont want anyone to use it cause it can freeze hard

    Player* plr = getSelectedPlayer();
    if (!plr)
        plr = m_session->GetPlayer();

    char* packet_opcode = strtok(NULL, " ");
    if (!packet_opcode)
        return false;
    
    Opcodes opcode = Opcodes(atoi(packet_opcode));
    uint8 info[256];
    bool used[256];
    uint8 counter;
    for (counter = 0; counter < 256; counter++)
    {
        info[counter] = 0;
        used[counter] = false;
    }
    counter = 0;
    while (char* packet_info = strtok(NULL, " "))
    {
        info[counter] = atoi(packet_info);
        used[counter] = true;
        counter++;
    }
    counter = 0;
    WorldPacket data(opcode, counter);
    while (used[counter])
    {
        data << info[counter];
        counter++;

    plr->SendPacketToSelf(&data);
    }
}

bool ChatHandler::HandleNpcSetModifierCommand(const char* args)
{
    Creature* target = getSelectedCreature();

    if (!target)
    {
        /*for (std::map<uint32,char*>::iterator itr = sWorld.noModCreatures.begin(); itr != sWorld.noModCreatures.end(); itr++)
        {
            char chr[256];
            sprintf(chr, "Creature entry %u, name %s still doesn't have mod", itr->first, itr->second);
            SendSysMessage(chr);
        }*/
        return false;
    }

    char* damageC = strtok((char*)args, " ");
    char* healthC = strtok(NULL, " ");

    if (!healthC || !damageC)
        return false;
  
    float damage = atof(damageC);
    float health = atof(healthC);

	CreatureInfo const* info = target->GetCreatureInfo();

	if (!info)
		return false;

	//if (sWorld.RaidForceDefaultModifiers(target->GetMap()->GetId()))
	//{
	//	PSendSysMessage("This NPC is forced to use default modifiers! Cannot change!");
	//	return true;
	//}

	//if (target->GetMap()->IsHeroic() && info->Entry < 9300000)
	//{
	//	PSendSysMessage("This creature don't have a heroic copy! Can't update! entry %u heroic %u", target->GetEntry(), target->GetCreatureInfo()->HeroicEntry);
	//	return true;
	//}

    const_cast<CreatureInfo*>(info)->hpMod = health;
    const_cast<CreatureInfo*>(info)->dmgMod = damage;

    GameDataDatabase.PExecuteLog("UPDATE creature_template SET hp_modifier=%f, dmg_modifier=%f WHERE entry=%u", health, damage, info->Entry);
    
    std::list<Creature*> SpawnList;
    Hellground::AllCreaturesOfEntryInRange u_check(m_session->GetPlayer(), target->GetEntry(), 1000); // all we can find
    Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(SpawnList, u_check);
    Cell::VisitAllObjects(m_session->GetPlayer(), searcher, 1000);

    if(!SpawnList.empty())
    {
        for(std::list<Creature*>::iterator i = SpawnList.begin(); i != SpawnList.end(); i++)
        {
            if((*i)->GetHPMod() != health || (*i)->GetDMGMod() != damage)
                (*i)->UpdateEntry((*i)->GetEntry());
        }
    }
    //sWorld.noModCreatures.erase(target->GetEntry());
    PSendSysMessage("Creature %s (entry %u) now has: health %f, damage %f", target->GetName(), info->Entry, health, damage);
    return true;
}

bool ChatHandler::HandleServerKickallCommand(const char* args)
{
    sWorld.KickAllWithoutPermissions(m_session ? PERM_ADM : 0);
    return true;
}

bool ChatHandler::HandleModifyAddTitleCommand(const char* args)
{
    if (!*args)
        return false;

    uint64 titles = 0;

    sscanf((char*)args, "%llu", &titles);

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    titles |= chr->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES);
    uint64 existingTitles;

    for (int i = 1; i < sCharTitlesStore.GetNumRows(); ++i)
        if (CharTitlesEntry const* tEntry = sCharTitlesStore.LookupEntry(i))
            existingTitles |= (uint64(1) << tEntry->bit_index);

    titles &= existingTitles;

    chr->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles);
    SendSysMessage(LANG_DONE);

    return true;
}

bool ChatHandler::HandleModifyRemoveTitleCommand(const char* args)
{
    if (!*args)
        return false;

    uint64 titles = 0;

    sscanf((char*)args, "%llu", &titles);

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if ((chr->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES) & titles) == 0)
    {
		SendSysMessage("Player doesn't have this title");
		return false;
	}

    titles = chr->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES) & ~(titles);

    chr->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles);
    SendSysMessage(LANG_DONE);

    return true;
}

bool ChatHandler::HandleWardenForceCheckCommand(const char* /*args*/)
{
    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    WorldSession* sess = chr->GetSession();
    if (!sess)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    sess->WardenForceRequest();
    return true;
}

void TestGridNeedsVMapWater(float px, float py, Map* m, uint64 plrGuid, bool doLog)
{
    const TerrainInfo* t = m->GetTerrain();

    float sx = px - SIZE_OF_GRIDS;
    float sy = py - SIZE_OF_GRIDS;
    float ex = px + SIZE_OF_GRIDS;
    float ey = py + SIZE_OF_GRIDS;

    uint32 thisGridX = uint32(32 - px / SIZE_OF_GRIDS);                       //grid x
    uint32 thisGridY = uint32(32 - py / SIZE_OF_GRIDS);                       //grid y

    bool found = false;
    bool foundSpecial = false;

    for (float x = sx; x < ex; x += 0.2f)
    {
        for (float y = sy; y < ey; y += 0.2f)
        {
            uint32 tempGridX = uint32(32 - x / SIZE_OF_GRIDS);                       //grid x
            uint32 tempGridY = uint32(32 - y / SIZE_OF_GRIDS);                       //grid y

            if (!(tempGridX == thisGridX && tempGridY == thisGridY)) // only checking current grid
                continue;

            float z = MAX_HEIGHT;
            while (true)
            {
                z = t->GetHeight(x, y, z - 2.1f, true, MAX_HEIGHT * 2);
                if (!(z > FALL_UNDER_MAP_HEIGHT)/* || (z == 0)*/)
                    break; // no ground/floors anymore - go to next "y" coordinate

                GridMapLiquidData data;
                // Scenario that we need to track: VMap finds something (ANYTHING!)
                if (t->getLiquidStatusVMapOnlyForTest(x, y, z, &data))
                {
                    if (doLog)
                    {
                        sLog.outLog(LOG_DIFF, "VMap: XYZ %f %f %f: t %u, l %f, g %f", x, y, z, data.type, data.level, data.depth_level);
                        if (data.level - data.depth_level > 1.0f)
                            sLog.outLog(LOG_DIFF, "Depth %f", data.level - data.depth_level);
                    }

                    found = true;
                    if (data.type != MAP_LIQUID_TYPE_WATER)
                    {
                        if (doLog)
                            sLog.outLog(LOG_DIFF, "Special liquid %u", data.type);

                        foundSpecial = true;
                    }
                }
                else if (t->getLiquidStatusIfSwimmingOrSpecial(x, y, z, &data, true)) // map found something?
                {
                    if (data.type != MAP_LIQUID_TYPE_WATER)
                    {
                        if (doLog)
                        {
                            sLog.outLog(LOG_DIFF, "N-Map: XYZ %f %f %f: t %u, l %f, g %f", x, y, z, data.type, data.level, data.depth_level);
                            sLog.outLog(LOG_DIFF, "Special liquid %u", data.type);
                        }
                        foundSpecial = true;
                    }
                }
            }
        }
    }
    Player* plr = sObjectMgr.GetPlayerInWorld(plrGuid);
    if (plr)
        ChatHandler(plr).PSendSysMessage("Map %u, Grid for map_water X %u Y %u needs VMap water: %u. Using special liquid: %u", m->GetId(), thisGridX, thisGridY, found, foundSpecial);
}

class TestGridNeedsVMapWaterThread : public ACE_Based::Runnable
{
public:
    TestGridNeedsVMapWaterThread(float px, float py, Map* m, uint64 plrGuid, bool doLog) : _px(px), _py(py), map(m), _plrGuid(plrGuid), _doLog(doLog) {}
    virtual void run()
    {
        TestGridNeedsVMapWater(_px, _py, map, _plrGuid, _doLog);
    }
    bool _doLog;
    uint64 _plrGuid;
    float _px, _py;
    Map* map;
};

bool ChatHandler::HandleTestGridNeedsVMapWaterCommand(const char* args)
{
    Player* p = m_session->GetPlayer();
    Map* m = p->GetMap();
    float px = p->GetPositionX();
    float py = p->GetPositionY();
    std::string argstr = (char*)args;

    if (argstr == "show")
    {
        uint32 thisGridX = uint32(32 - px / SIZE_OF_GRIDS);                       //grid x
        uint32 thisGridY = uint32(32 - py / SIZE_OF_GRIDS);                       //grid y
        ChatHandler(p).PSendSysMessage("Grid X %u, Y %u uses water: %u, specialLiquid: %u", thisGridX, thisGridY, m->GetTerrain()->waterNeedsVMap(px, py), m->GetTerrain()->hasSpecialLiquid(px, py));
        return true;
    }

    new ACE_Based::Thread(new TestGridNeedsVMapWaterThread(px, py, m, p->GetGUID(), argstr == "log"));
    // Not waiting for thread end and NOT DELETING IT. Mem leak, this is dev-only command!
    return true;
}

bool ChatHandler::HandleFakeBotAdd(const char* args)
{
    char* cntStr = strtok((char*)args, "");
    if (!cntStr)
        return false;

    uint32 count = atoi(cntStr);
    sWorld.AddFakeBots(count);

    return true;
}

bool ChatHandler::HandleGameobjectDuplicateCommand(const char* args)
{
    // Select ALL objects
    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT gameobject.guid, game_event_gameobject.event, pool_gameobject.pool_entry FROM gameobject "
        "LEFT OUTER JOIN game_event_gameobject ON gameobject.guid=game_event_gameobject.guid "
        "LEFT OUTER JOIN pool_gameobject ON gameobject.guid=pool_gameobject.guid WHERE gameobject.id != 183407"); // 183407 SSC elevator, should be duplicated

    if (!result) // error.
        return true;

    std::string argstr = args ? (char*)args : "";

    bool partCompare = argstr == "part";
    bool ignoreDiffPools = argstr == "partPools";
    if (ignoreDiffPools)
        partCompare = true;

    struct listObj
    {
        const GameObjectData* data;
        int16 eventId;
        int16 poolId;
        std::set<uint32> guids;
    };

    std::list<listObj> sameObjects;
    do
    {
        Field *fields = result->Fetch();

        uint32 lowguid = fields[0].GetUInt32();
        int16 eventId = fields[1].GetInt16();
        int16 poolId = fields[2].GetInt16();

        if (GameObjectData const* goData = sObjectMgr.GetGOData(lowguid))
        {
            bool added = false;
            for (auto itr = sameObjects.begin(); itr != sameObjects.end(); ++itr)
            {
                // event id and pool id are the same (can be both 0, still the same)
                if (itr->eventId == eventId && (itr->poolId == poolId || ignoreDiffPools))
                {
                    GameObjectData const* goData2 = itr->data;
                    // partly comparing --- ID, X,Y,Z, map, spawnmask bit, event and pool
                    if (partCompare)
                    {
                        if (goData2->id == goData->id &&
                            goData2->mapid == goData->mapid &&
                            (goData2->spawnMask & goData->spawnMask) &&
                            (fabs(goData2->posX - goData->posX) < 0.1f) &&
                            (fabs(goData2->posY - goData->posY) < 0.1f) &&
                            (fabs(goData2->posZ - goData->posZ) < 0.1f))
                        {
                            if (goData2->animprogress != goData->animprogress)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object animprogress %u, orig %u. Guids dupe %u, orig %u", goData->animprogress, goData2->animprogress, lowguid, *(itr->guids.begin()));

                            if (goData2->ArtKit != goData->ArtKit)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object ArtKit %u, orig %u. Guids dupe %u, orig %u", goData->ArtKit, goData2->ArtKit, lowguid, *(itr->guids.begin()));

                            if (goData2->dbData != goData->dbData)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object dbData %u, orig %u. Guids dupe %u, orig %u", goData->dbData, goData2->dbData, lowguid, *(itr->guids.begin()));

                            if (goData2->go_state != goData->go_state)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object go_state %u, orig %u. Guids dupe %u, orig %u", goData->go_state, goData2->go_state, lowguid, *(itr->guids.begin()));

                            if (goData2->orientation != goData->orientation)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object orientation %f, orig %f. Guids dupe %u, orig %u", goData->orientation, goData2->orientation, lowguid, *(itr->guids.begin()));

                            if (goData2->rotation0 != goData->rotation0)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object rotation0 %f, orig %f. Guids dupe %u, orig %u", goData->rotation0, goData2->rotation0, lowguid, *(itr->guids.begin()));

                            if (goData2->rotation1 != goData->rotation1)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object rotation1 %f, orig %f. Guids dupe %u, orig %u", goData->rotation1, goData2->rotation1, lowguid, *(itr->guids.begin()));

                            if (goData2->rotation2 != goData->rotation2)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object rotation2 %f, orig %f. Guids dupe %u, orig %u", goData->rotation2, goData2->rotation2, lowguid, *(itr->guids.begin()));

                            if (goData2->rotation3 != goData->rotation3)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object rotation3 %f, orig %f. Guids dupe %u, orig %u", goData->rotation3, goData2->rotation3, lowguid, *(itr->guids.begin()));

                            if (goData2->spawnMask != goData->spawnMask)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object spawnMask %u, orig %u. Guids dupe %u, orig %u", goData->spawnMask, goData2->spawnMask, lowguid, *(itr->guids.begin()));

                            if (goData2->spawntimesecs != goData->spawntimesecs)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object spawntimesecs %i, orig %i. Guids dupe %u, orig %u", goData->spawntimesecs, goData2->spawntimesecs, lowguid, *(itr->guids.begin()));


                            itr->guids.insert(lowguid);
                            added = true;
                            break;
                        }
                    }
                    else if (memcmp(goData2, goData, sizeof(GameObjectData)) == 0)// found -> add guid to this list objects set of guids
                    {
                        itr->guids.insert(lowguid);
                        added = true;
                        break;
                    }
                }
            }

            if (!added) // create new object
            {
                listObj obj;
                obj.data = goData;
                obj.eventId = eventId;
                obj.poolId = poolId;
                obj.guids.insert(lowguid);
                sameObjects.push_back(obj);
            }
        }
    } while (result->NextRow());


    bool duplicate = false;
    for (auto obj = sameObjects.begin(); obj != sameObjects.end(); ++obj)
    {
        auto &set = obj->guids;
        if (set.size() > 1) // more than one of such identifier
        {
            const GameObjectData* data = obj->data;

            for (auto j = set.begin(); j != set.end(); ++j)
            {
                if (j == set.begin()) // leave 1 in there as original
                    continue;

                if (partCompare)
                    sLog.outLog(LOG_DEFAULT, "-- gameobject guid %u is PART duplicate (gameobject(id,x,y,z,map, bit spawnmask), game_event_gameobject, pool_gameobject) with guid %u", *j, *set.begin());
                else
                    sLog.outLog(LOG_DEFAULT, "-- gameobject guid %u is FULL duplicate (gameobject, game_event_gameobject, pool_gameobject) with guid %u", *j, *set.begin());

                sLog.outLog(LOG_DEFAULT, "DELETE FROM gameobject WHERE guid=%u AND id=%u;", *j, data->id);
                duplicate = true;
            }
        }
    }

    if (duplicate) // dont continue
    {
        SendSysMessage("Duplicates found");
        return true;
    }

    SendSysMessage("Duplicates not found");
    return true;
}

bool ChatHandler::HandleCreatureDuplicateCommand(const char* args)
{
    // Select ALL objects
    QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT creature.guid, game_event_creature.event, pool_creature.pool_entry FROM creature "
        "LEFT OUTER JOIN game_event_creature ON creature.guid=game_event_creature.guid "
        "LEFT OUTER JOIN pool_creature ON creature.guid=pool_creature.guid");

    if (!result) // error.
        return true;

    std::string argstr = args ? (char*)args : "";

    bool partCompare = argstr == "part";
    bool ignoreDiffPools = argstr == "partPools";
    if (ignoreDiffPools)
        partCompare = true;

    struct listObj
    {
        const CreatureData* data;
        int16 eventId;
        int16 poolId;
        std::set<uint32> guids;
    };

    std::list<listObj> sameObjects;
    do
    {
        Field *fields = result->Fetch();

        uint32 lowguid = fields[0].GetUInt32();
        int16 eventId = fields[1].GetInt16();
        int16 poolId = fields[2].GetInt16();

        if (CreatureData const* creData = sObjectMgr.GetCreatureData(lowguid))
        {
            bool added = false;
            for (auto itr = sameObjects.begin(); itr != sameObjects.end(); ++itr)
            {
                // event id and pool id are the same (can be both 0, still the same)
                if (itr->eventId == eventId && (itr->poolId == poolId || ignoreDiffPools))
                {
                    CreatureData const* creData2 = itr->data;
                    // partly comparing --- ID, X,Y,Z, map, spawnmask bit, event and pool
                    if (partCompare)
                    {
                        if (creData2->id == creData->id &&
                            creData2->mapid == creData->mapid &&
                            (creData2->spawnMask & creData->spawnMask) &&
                            (fabs(creData2->posX - creData->posX) < 0.1f) &&
                            (fabs(creData2->posY - creData->posY) < 0.1f) &&
                            (fabs(creData2->posZ - creData->posZ) < 0.1f))
                        {
                            if (creData2->currentwaypoint != creData->currentwaypoint)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object currentwaypoint %u, orig %u. Guids dupe %u, orig %u", creData->currentwaypoint, creData2->currentwaypoint, lowguid, *(itr->guids.begin()));

                            if (creData2->displayid != creData->displayid)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object displayid %u, orig %u. Guids dupe %u, orig %u", creData->displayid, creData2->displayid, lowguid, *(itr->guids.begin()));

                            if (creData2->curhealth != creData->curhealth)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object curhealth %u, orig %u. Guids dupe %u, orig %u", creData->curhealth, creData2->curhealth, lowguid, *(itr->guids.begin()));

                            if (creData2->curmana != creData->curmana)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object curmana %u, orig %u. Guids dupe %u, orig %u", creData->curmana, creData2->curmana, lowguid, *(itr->guids.begin()));

                            if (creData2->equipmentId != creData->equipmentId)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object equipmentId %u, orig %u. Guids dupe %u, orig %u", creData->equipmentId, creData2->equipmentId, lowguid, *(itr->guids.begin()));

                            if (creData2->is_dead != creData->is_dead)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object is_dead %u, orig %u. Guids dupe %u, orig %u", creData->is_dead, creData2->is_dead, lowguid, *(itr->guids.begin()));

                            if (creData2->movementType != creData->movementType)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object movementType %u, orig %u. Guids dupe %u, orig %u", creData->movementType, creData2->movementType, lowguid, *(itr->guids.begin()));

                            if (creData2->orientation != creData->orientation)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object orientation %f, orig %f. Guids dupe %u, orig %u", creData->orientation, creData2->orientation, lowguid, *(itr->guids.begin()));

                            if (creData2->spawndist != creData->spawndist)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object spawndist %f, orig %f. Guids dupe %u, orig %u", creData->spawndist, creData2->spawndist, lowguid, *(itr->guids.begin()));

                            if (creData2->spawnMask != creData->spawnMask)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object spawnMask %u, orig %u. Guids dupe %u, orig %u", creData->spawnMask, creData2->spawnMask, lowguid, *(itr->guids.begin()));

                            if (creData2->spawntimesecs != creData->spawntimesecs)
                                sLog.outLog(LOG_DEFAULT, "-- dupe object spawntimesecs %u, orig %u. Guids dupe %u, orig %u", creData->spawntimesecs, creData2->spawntimesecs, lowguid, *(itr->guids.begin()));


                            itr->guids.insert(lowguid);
                            added = true;
                            break;
                        }
                    }
                    else if (memcmp(creData2, creData, sizeof(CreatureData)) == 0)// found -> add guid to this list objects set of guids
                    {
                        itr->guids.insert(lowguid);
                        added = true;
                        break;
                    }
                }
            }

            if (!added) // create new object
            {
                listObj obj;
                obj.data = creData;
                obj.eventId = eventId;
                obj.poolId = poolId;
                obj.guids.insert(lowguid);
                sameObjects.push_back(obj);
            }
        }
    } while (result->NextRow());


    bool duplicate = false;
    for (auto obj = sameObjects.begin(); obj != sameObjects.end(); ++obj)
    {
        auto &set = obj->guids;
        if (set.size() > 1) // more than one of such identifier
        {
            const CreatureData* data = obj->data;

            for (auto j = set.begin(); j != set.end(); ++j)
            {
                if (j == set.begin()) // leave 1 in there as original
                    continue;

                CreatureDataAddon const* addon = ObjectMgr::GetCreatureAddon(*j);
                CreatureDataAddon const* addonOrig = ObjectMgr::GetCreatureAddon(*set.begin());
                bool sameAddon = (!addon && !addonOrig) || (addon && addonOrig && 
                    addon->auras == addonOrig->auras && addon->bytes0 == addonOrig->bytes0 && addon->bytes1 == addonOrig->bytes1 && addon->bytes2 == addonOrig->bytes2 && addon->emote == addonOrig->emote &&
                    addon->mount == addonOrig->mount && addon->move_flags == addonOrig->move_flags && addon->path_id == addonOrig->path_id);

                if (partCompare)
                    sLog.outLog(LOG_DEFAULT, "-- creature guid %u is PART duplicate (creature(id,x,y,z,map, bit spawnmask), game_event_creature, pool_creature) with guid %u. Same creature addon %u", *j, *set.begin(), sameAddon);
                else
                    sLog.outLog(LOG_DEFAULT, "-- creature guid %u is FULL duplicate (creature, game_event_creature, pool_creature) with guid %u. Same creature addon %u", *j, *set.begin(), sameAddon);

                sLog.outLog(LOG_DEFAULT, "DELETE FROM creature WHERE guid=%u AND id=%u;", *j, data->id);
                duplicate = true;
            }
        }
    }

    if (duplicate) // dont continue
    {
        SendSysMessage("Duplicates found");
        return true;
    }

    SendSysMessage("Duplicates not found");
    return true;
}

bool ChatHandler::HandleGameobjectCheckCommand(const char* args)
{
    QueryResultAutoPtr resCheckedGOs = GameDataDatabase.Query("SELECT guid FROM checked_objects WHERE `type`=1");

    std::set<uint32> checkedGOs;
    if (resCheckedGOs)
    {
        do
        {
            Field *fields = resCheckedGOs->Fetch();

            uint32 id = fields[0].GetUInt32();
            if (id)
                checkedGOs.insert(id);

        } while (resCheckedGOs->NextRow());
    }

    // Select ids of objects that we need to create pool for
    QueryResultAutoPtr resultObjectIDs = GameDataDatabase.Query("SELECT entry, type='herb' FROM pool_gameobject_ids");

    if (!resultObjectIDs) // error.
        return true;

    // uint32 object id, bool true if herb
    std::map<uint32, bool> go_to_pool_ids;
    do
    {
        Field *fields = resultObjectIDs->Fetch();

        uint32 id = fields[0].GetUInt32();
        if (id)
            go_to_pool_ids[id] = fields[1].GetBool();

    } while (resultObjectIDs->NextRow());

    char* minStr = strtok((char*)args, " ");
    char* maxStr = strtok(NULL, "");
    if (!minStr || !maxStr)
        return false;

    uint32 min = atoi(minStr);
    uint32 max = atoi(maxStr);

    GameObjectDataMap const& dataMap = sObjectMgr.GetGODataMap();
    std::map<uint32, uint32> mapInstIds;
    for (GameObjectDataMap::const_iterator itr = dataMap.begin(); itr != dataMap.end(); ++itr)
    {
        GameObjectData const& d = itr->second;
        uint32 guid = itr->first;

        auto poolObjItr = go_to_pool_ids.find(d.id);
        if (poolObjItr == go_to_pool_ids.end())
            continue; // not pooled object

        if (guid < min || guid > max || checkedGOs.find(guid) != checkedGOs.end())
            continue;

        float x = d.posX;
        float y = d.posY;
        float z = d.posZ;
        int mapid = d.mapid;

        Map* map = sMapMgr.FindMap(mapid, mapInstIds.find(mapid) != mapInstIds.end() ? mapInstIds.find(mapid)->second : 0);
        if (!map)
        {
            const MapEntry* entry = sMapStore.LookupEntry(mapid);
            if (entry->IsBattleGroundOrArena())
            {
                for (uint32 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
                {
                    BattleGround* bg = sBattleGroundMgr.CreateNewBattleGround((BattleGroundTypeId)i, BG_BRACKET_ID_LAST, 0, false);
                    if (bg && bg->GetMapId() == mapid)
                    {
                        map = bg->GetMap();
                        mapInstIds[mapid] = map->GetAnyInstanceId();
                        break;
                    }
                }
            }
            else
            {
                map = sMapMgr.CreateMap(mapid, m_session->GetPlayer()); // not caring about sInstance here, so no relocate is called before CreateMap
                mapInstIds[mapid] = map->GetAnyInstanceId();
            }
        }

        if (map && map->GetTerrain())
        {
            if (!map->DebugEnsureGridLoadedAtEnter(d.posX, d.posY))
            {
                sLog.outLog(LOG_DEFAULT, "Gameobject guid %u over the map.", guid);
                continue; // main loop
            }

            GameObject* go = map->GetGameObject(MAKE_NEW_GUID(guid, d.id, HIGHGUID_GAMEOBJECT));

            if (!go)
            {
                GameObject *newgo = new GameObject();
                if (!newgo->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT), d.id, map, x, y, z, 0, 0, 0, 0, 0, 100, GO_STATE_READY))
                {
                    delete newgo;
                    sLog.outLog(LOG_DEFAULT, "Could not create GO for entry %u, guid %u.", d.id, guid);
                    continue;
                }
                newgo->SetSpawnedByDefault(false);
                map->Add(newgo);

                go = newgo;
            }

            if (GameObject* g = go)
            {
                Position bestCandidate;
                Position bestCandidateStartPos;
                bool requirePath = GetVirtualMapForMapAndZone(d.mapid, map->GetTerrain()->GetZoneId(d.posX, d.posY, d.posZ)) != 530;

                bool finePath = map->GetTerrain()->IsInWater(x,y,z) || map->GetTerrain()->IsInWater(x, y, z-2.0f);
                if (!finePath)
                {
                    std::list<Creature*> creatureList;
                    float radius = 500.0f;

                    Hellground::AnyUnitInObjectRangeCheck go_check(g, radius);
                    Hellground::ObjectListSearcher<Creature, Hellground::AnyUnitInObjectRangeCheck> go_search(creatureList, go_check);
                    // Get Creatures
                    Cell::VisitGridObjects(g, go_search, radius);

                    if (!creatureList.empty())
                    {
                        creatureList.sort([=](Creature* cre, Creature* cre2)->bool
                        {
                            return g->GetDistance(cre->GetPositionX(), cre->GetPositionY(), cre->GetPositionZ()) < g->GetDistance(cre2->GetPositionX(), cre2->GetPositionY(), cre2->GetPositionZ());
                        });

                        for (std::list<Creature*>::const_iterator i = creatureList.begin(); i != creatureList.end(); ++i)
                        {
                            Creature* target = *i;

                            if (target->CanFly())
                                continue; // skip flying -> they can path to anywhere

                            PathFinder path(target);
                            path.setPathLengthLimit(1000);
                            bool result = path.calculate(x, y, z);
                            if (!result || !(path.getPathType() & PATHFIND_NORMAL))
                            {
                                // 3D distance for gathering objects is 5 yds, so this distance is allowed.
                                if ((path.getPathType() & PATHFIND_INCOMPLETE))
                                {
                                    Vector3 end = path.getActualEndPosition();
                                    if (g->GetDistance(end.x, end.y, end.z) <= g->GetDistance(bestCandidate.x, bestCandidate.y, bestCandidate.z))
                                    {
                                        bestCandidate.x = end.x;
                                        bestCandidate.y = end.y;
                                        bestCandidate.z = end.z;

                                        target->GetPosition(bestCandidateStartPos);
                                        // UpdateAllowedPositionZ_WithUndermap will update z too much in places where VMap is bugged, such as caverns. Just use +5 to 'fix' the creatures that are underground by an error.
                                        // target->UpdateAllowedPositionZ_WithUndermap(bestCandidateStartPos.x, bestCandidateStartPos.y, bestCandidateStartPos.z);
                                        bestCandidateStartPos.z += 5.0f;
                                    }

                                    if (g->GetDistance(end.x, end.y, end.z) < 5.0f && g->IsWithinLOS(end.x, end.y, end.z)
                                        || g->GetDistance(end.x, end.y, end.z) < 1.0f)
                                    {
                                        finePath = true;
                                        //break; // not breaking to possibly find a better candidate pos
                                    }
                                }
                                continue; // continue this loop to try to find a path to another creature.
                            }

                            Vector3 end = path.getActualEndPosition();
                            if (g->GetDistance(end.x, end.y, end.z) <= g->GetDistance(bestCandidate.x, bestCandidate.y, bestCandidate.z))
                            {
                                bestCandidate.x = end.x;
                                bestCandidate.y = end.y;
                                bestCandidate.z = end.z;

                                target->GetPosition(bestCandidateStartPos);
                                // UpdateAllowedPositionZ_WithUndermap will update z too much in places where VMap is bugged, such as caverns. Just use +5 to 'fix' the creatures that are underground by an error.
                                // target->UpdateAllowedPositionZ_WithUndermap(bestCandidateStartPos.x, bestCandidateStartPos.y, bestCandidateStartPos.z);
                                bestCandidateStartPos.z += 5.0f;
                            }

                            finePath = true;
                            break; // path found - AND it is the best startPos candidate, cause creatures were sorted by distance to them
                        }
                    }
                }

                if (!finePath && requirePath)
                {
                    sLog.outLog(LOG_DEFAULT, "Gameobject guid %u, cannot be reached by path.", guid);
                    sWorld.debugGOeModPos[guid] = bestCandidate;
                }

                float destZ = z;
                g->UpdateAllowedPositionZ_WithUndermap(x, y, destZ);
                bool added = false;
                if (destZ > z)
                {
                    // GOs are allowed to be right under other height, so we need to check if there's other, right height for this object.
                    float destZ2 = z - 1.99f;
                    g->UpdateAllowedPositionZ_WithUndermap(x, y, destZ2);

                    float checkByZ = fabs(z - destZ) < fabs(z - destZ2) ? destZ : destZ2;

                    if (!(finePath && (checkByZ - z > 5.0f))) // holes in VMap. But MMAP has got this map covered and it's reachable. So don't report about the "underground"
                    {
                        if (checkByZ - z > 2.0f)
                        {
                            sLog.outLog(LOG_DEFAULT, "Gameobject guid %u is FAR under map. Current z %f, closest height above %f. Diff %f. UPDATE `gameobject` SET position_z='%f' WHERE `guid`='%u';", guid, z, destZ, fabs(z - destZ), destZ, guid);
                            sWorld.debugGOeModPos[guid] = bestCandidate;
                            added = true;
                        }
                        // Check 0.25 for herbs, 0.7 for minerals
                        // (copper vein is 1.35 height, and it is the smallest of veins). Terocone is about 0.5 in height, Mageroyal - about 0.45
                        else if ((checkByZ - z) > (poolObjItr->second ? 0.25f : 0.7f))
                        {
                            sLog.outLog(LOG_DEFAULT, "Gameobject guid %u is a little under map. Current z %f, closest height above %f. Diff %f. UPDATE `gameobject` SET position_z='%f' WHERE `guid`='%u';", guid, z, destZ, fabs(z - destZ), destZ, guid);
                            sWorld.debugGOeModPos[guid] = bestCandidate;
                            added = true;
                        }
                    }
                }

                if (!added)
                {
                    float gridHeightObj = map->GetTerrain()->GetHeight(x, y, MAX_HEIGHT, false);
                    float gridHeightCandidateStart = map->GetTerrain()->GetHeight(bestCandidateStartPos.x, bestCandidateStartPos.y, MAX_HEIGHT, false);
                    bool candidateOverGridHeight = bestCandidateStartPos.z >= gridHeightCandidateStart;
                    bool objectOverGridHeight = z >= gridHeightObj;
                    bool groundLosErr = candidateOverGridHeight != objectOverGridHeight;

                    // Check 0.25 for herbs, 0.7 for minerals
                    // (copper vein is 1.35 height, and it is the smallest of veins). Terocone is about 0.5 in height, Mageroyal - about 0.45
                    if (groundLosErr && ((gridHeightObj - z) > (poolObjItr->second ? 0.25f : 0.7f)))
                    {
                        bestCandidate.x = x;
                        bestCandidate.y = y;
                        bestCandidate.z = gridHeightObj;

                        sLog.outLog(LOG_DEFAULT, "Gameobject guid %u is a little under map (under grid height). Current z %f, closest height above %f. Diff %f. UPDATE `gameobject` SET position_z='%f' WHERE `guid`='%u';", guid, z, gridHeightObj, fabs(z - gridHeightObj), gridHeightObj, guid);
                        sWorld.debugGOeModPos[guid] = bestCandidate;
                    }
                }

                if (!requirePath && !finePath) // objects that should be reachable by flying. Need to still find those out of LoS
                {
                    bool finePoint = false;
                    for (uint32 i = 0; i < 16; ++i)
                    {
                        Position pos;
                        g->GetValidPointInAngle(pos, 30, M_PI / 8.0f*i, true, 30);
                        if (g->GetDistance(pos.x, pos.y, pos.z) > 15.0f)
                        {
                            finePoint = true;
                            break;
                        }
                    }

                    if (!finePoint)
                    {
                        sLog.outLog(LOG_DEFAULT, "Gameobject guid %u, cannot get out by validPoint.", guid);
                        sWorld.debugGOeModPos[guid] = bestCandidate;
                    }
                }
            }
        }
    }
    SendSysMessage("Done");
    return true;
}

bool ChatHandler::HandleCreatureCheckCommand(const char* args)
{
    QueryResultAutoPtr resCheckedCreatures = GameDataDatabase.Query("SELECT guid FROM checked_objects WHERE `type`=0");

    std::set<uint32> checkedCreatures;
    if (resCheckedCreatures)
    { 
        do
        {
            Field *fields = resCheckedCreatures->Fetch();

            uint32 id = fields[0].GetUInt32();
            if (id)
                checkedCreatures.insert(id);

        } while (resCheckedCreatures->NextRow());
    }

    char* minStr = strtok((char*)args, " ");
    char* maxStr = strtok(NULL, "");
    if (!minStr || !maxStr)
        return false;

    uint32 min = atoi(minStr);
    uint32 max = atoi(maxStr);

    CreatureDataMap const& dataMap = sObjectMgr.GetCreatureDataMap();
    std::map<uint32, uint32> mapInstIds;
    for (CreatureDataMap::const_iterator itr = dataMap.begin(); itr != dataMap.end(); ++itr)
    {
        CreatureData const& d = itr->second;
        uint32 guid = itr->first;

        if (guid < min || guid > max || checkedCreatures.find(guid) != checkedCreatures.end())
            continue;

        float x = d.posX;
        float y = d.posY;
        float z = d.posZ;
        int mapid = d.mapid;

        Map* map = sMapMgr.FindMap(mapid, mapInstIds.find(mapid) != mapInstIds.end() ? mapInstIds.find(mapid)->second : 0);
        if (!map)
        {
            const MapEntry* entry = sMapStore.LookupEntry(mapid);
            if (entry->IsBattleGroundOrArena())
            {
                for (uint32 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
                {
                    BattleGround* bg = sBattleGroundMgr.CreateNewBattleGround((BattleGroundTypeId)i, BG_BRACKET_ID_LAST, 0, false);
                    if (bg && bg->GetMapId() == mapid)
                    {
                        map = bg->GetMap();
                        mapInstIds[mapid] = map->GetAnyInstanceId();
                        break;
                    }
                }
            }
            else
            {
                map = sMapMgr.CreateMap(mapid, m_session->GetPlayer()); // not caring about sInstance here, so no relocate is called before CreateMap
                mapInstIds[mapid] = map->GetAnyInstanceId();
            }
        }

        if (map && map->GetTerrain())
        {
            if (!map->DebugEnsureGridLoadedAtEnter(d.posX, d.posY))
            {
                sLog.outLog(LOG_DEFAULT, "Creature guid %u over the map.", guid);
                continue; // main loop
            }

            Creature* cre = map->GetCreature(MAKE_NEW_GUID(guid, d.id, HIGHGUID_UNIT));

            if (!cre)
            {
                Creature* pCreature = new Creature();

                if (!pCreature->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_UNIT), map, d.id, TEAM_NONE, x, y, z, 0))
                {
                    delete pCreature;
                    sLog.outLog(LOG_DEFAULT, "Could not create creature for entry %u, guid %u.", d.id, guid);
                    continue;
                }

                pCreature->SetHomePosition(x, y, z, 0);
                map->Add(pCreature);

                cre = pCreature;
            }

            if (Creature* c = cre)
            {
                if (c->isTrigger() || (c->isCivilian() && c->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2)))
                    continue; // main loop

                Position bestCandidate;
                Position bestCandidateStartPos;

                bool finePath = c->IsSwimming() || c->CanFly();

                if (!finePath)
                {
                    std::list<Creature*> creatureList;
                    float radius = 500.0f;

                    Hellground::AnyUnitInObjectRangeCheck go_check(c, radius);
                    Hellground::ObjectListSearcher<Creature, Hellground::AnyUnitInObjectRangeCheck> go_search(creatureList, go_check);
                    // Get Creatures
                    Cell::VisitGridObjects(c, go_search, radius);

                    if (!creatureList.empty())
                    {
                        creatureList.sort([=](Creature* cre, Creature* cre2)->bool
                        {
                            return c->GetDistance(cre->GetPositionX(), cre->GetPositionY(), cre->GetPositionZ()) < c->GetDistance(cre2->GetPositionX(), cre2->GetPositionY(), cre2->GetPositionZ());
                        });

                        for (std::list<Creature*>::const_iterator i = creatureList.begin(); i != creatureList.end(); ++i)
                        {
                            Creature* target = *i;
                            if (target == c) // skip self
                                continue;

                            if (target->CanFly())
                                continue; // skip flying -> they can path to anywhere

                            PathFinder path(target);
                            path.setPathLengthLimit(1000);
                            bool result = path.calculate(x, y, z);
                            if (!result || !(path.getPathType() & PATHFIND_NORMAL))
                            {
                                if ((path.getPathType() & PATHFIND_INCOMPLETE))
                                {
                                    Vector3 end = path.getActualEndPosition();
                                    if (c->GetDistance(end.x, end.y, end.z) <= c->GetDistance(bestCandidate.x, bestCandidate.y, bestCandidate.z))
                                    {
                                        bestCandidate.x = end.x;
                                        bestCandidate.y = end.y;
                                        bestCandidate.z = end.z;

                                        target->GetPosition(bestCandidateStartPos);
                                        // UpdateAllowedPositionZ_WithUndermap will update z too much in places where VMap is bugged, such as caverns. Just use +5 to 'fix' the creatures that are underground by an error.
                                        // target->UpdateAllowedPositionZ_WithUndermap(bestCandidateStartPos.x, bestCandidateStartPos.y, bestCandidateStartPos.z);
                                        bestCandidateStartPos.z += 5.0f;
                                    }
                                }

                                continue; // continue this loop to try to find a path to another creature.
                            }

                            Vector3 end = path.getActualEndPosition();
                            if (c->GetDistance(end.x, end.y, end.z) <= c->GetDistance(bestCandidate.x, bestCandidate.y, bestCandidate.z))
                            {
                                bestCandidate.x = end.x;
                                bestCandidate.y = end.y;
                                bestCandidate.z = end.z;

                                target->GetPosition(bestCandidateStartPos);
                                // UpdateAllowedPositionZ_WithUndermap will update z too much in places where VMap is bugged, such as caverns. Just use +5 to 'fix' the creatures that are underground by an error.
                                // target->UpdateAllowedPositionZ_WithUndermap(bestCandidateStartPos.x, bestCandidateStartPos.y, bestCandidateStartPos.z);
                                bestCandidateStartPos.z += 5.0f;
                            }
                            
                            finePath = true;
                            break; // path found - AND it is the best startPos candidate, cause creatures were sorted by distance to them
                        }
                    }
                }

                float destZ = z;
                c->UpdateAllowedPositionZ_WithUndermap(x, y, destZ);
                bool added = false;
                if (destZ > z)
                {
                    if (!(finePath && (destZ - z > 5.0f))) // holes in VMap. But MMAP has got this map covered and it's reachable. So don't report about the "underground"
                    {
                        if (destZ - z > 2.0f)
                        {
                            sLog.outLog(LOG_DEFAULT, "Creature guid %u is FAR under map. Current z %f, closest height above %f. Diff %f. UPDATE `creature` SET position_z='%f' WHERE `guid`='%u';", guid, z, destZ, fabs(z - destZ), destZ, guid);
                            sWorld.debugCreatureModPos[guid] = bestCandidate;
                            added = true;
                        }
                        else if (destZ - z > 0.3f)
                        {
                            sLog.outLog(LOG_DEFAULT, "Creature guid %u is a little under map. Current z %f, closest height above %f. Diff %f. UPDATE `creature` SET position_z='%f' WHERE `guid`='%u';", guid, z, destZ, fabs(z - destZ), destZ, guid);
                            sWorld.debugCreatureModPos[guid] = bestCandidate;
                            added = true;
                        }
                    }
                }
                
                if (!added)
                {
                    float gridHeightObj = map->GetTerrain()->GetHeight(x, y, MAX_HEIGHT, false);
                    float gridHeightCandidateStart = map->GetTerrain()->GetHeight(bestCandidateStartPos.x, bestCandidateStartPos.y, MAX_HEIGHT, false);
                    bool candidateOverGridHeight = bestCandidateStartPos.z >= gridHeightCandidateStart;
                    bool objectOverGridHeight = z >= gridHeightObj;
                    bool groundLosErr = candidateOverGridHeight != objectOverGridHeight;

                    if (groundLosErr && ((gridHeightObj - z) > 0.3f))
                    {
                        bestCandidate.x = x;
                        bestCandidate.y = y;
                        bestCandidate.z = gridHeightObj;

                        sLog.outLog(LOG_DEFAULT, "Creature guid %u is a little under map (under grid height). Current z %f, closest height above %f. Diff %f. UPDATE `gameobject` SET position_z='%f' WHERE `guid`='%u';", guid, z, gridHeightObj, fabs(z - gridHeightObj), gridHeightObj, guid);
                        sWorld.debugGOeModPos[guid] = bestCandidate;
                    }
                }

                //if (!finePath)
                //    sLog.outLog(LOG_DEFAULT, "Creature guid %u, cannot get out by path.", guid);

                bool finePoint = false;
                for (uint32 i = 0; i < 16; ++i)
                {
                    Position pos;
                    c->GetValidPointInAngle(pos, 30, M_PI / 8.0f*i, true, 30);
                    if (c->GetDistance(pos.x, pos.y, pos.z) > 15.0f)
                    {
                        finePoint = true;
                        break;
                    }
                }

                //if (!finePoint)
                //    sLog.outLog(LOG_DEFAULT, "Creature guid %u, cannot get out by validPoint.", guid);

                if (!finePath && !finePoint)
                {
                    sLog.outLog(LOG_DEFAULT, "Creature guid %u, cannot get out by path AND validPoint.", guid);
                    sWorld.debugCreatureModPos[guid] = bestCandidate;
                }
            }

            if (CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(d.id))
            {
                bool canWater = cInfo->InhabitType & INHABIT_WATER;
                bool isWater = map->GetTerrain()->IsInWater(x, y, z);
                bool onlyWater = cInfo->InhabitType == INHABIT_WATER;
                // should not be in water
                if (onlyWater && !isWater)
                    sLog.outLog(LOG_DEFAULT, "Creature guid %u, entry %u is not in water, but can only swim.", guid, d.id);
                else if (!canWater && isWater)
                    sLog.outLog(LOG_DEFAULT, "Creature guid %u, entry %u is in water, but cannot swim.", guid, d.id);
            }
        }
    }
    SendSysMessage("Done");
	return true;
}

bool ChatHandler::HandleDebugCheckNewPosCommand(const char* args)
{
    char* typeStr = strtok((char*)args, " ");
    char* cmdStr = strtok(NULL, "");
    Player* pl = m_session->GetPlayer();
    if (!typeStr || !cmdStr || !pl)
        return false;

    uint32 type = atoi(typeStr);
    if (type == 0)
    {
        if (sWorld.debugCreatureModPos.empty())
            return true;
    }
    else if (type == 1)
    {
        if (sWorld.debugGOeModPos.empty())
            return true;
    }
    else
        return true;

    if (strcmp(cmdStr, "cur") == 0)
    {
        switch (type)
        {
            case 0: // creature
            {
                char guidStr[20];
                sprintf(guidStr, "%u", (*(sWorld.debugCreatureModPos.begin())).first);
                PSendSysMessage("Porting to cre %u", (*(sWorld.debugCreatureModPos.begin())).first);
                HandleGoCreatureCommand(guidStr);
                break;
            }
            case 1: // GO
            {
                char guidStr[20];
                sprintf(guidStr, "%u", (*(sWorld.debugGOeModPos.begin())).first);
                PSendSysMessage("Porting to obj %u", (*(sWorld.debugGOeModPos.begin())).first);
                HandleGoObjectCommand(guidStr);
                break;
            }
            default:
                return true;
        }

    }
    else if (strcmp(cmdStr, "mod") == 0)
    {
        switch (type)
        {
            case 0: // creature
            {
                Position pos = (*(sWorld.debugCreatureModPos.begin())).second;
                PSendSysMessage("Porting to candidate for cre %u", (*(sWorld.debugCreatureModPos.begin())).first);
                if (pos.x != 0)
                    pl->TeleportTo(pl->GetMapId(), pos.x, pos.y, pos.z, 0);
                else
                    PSendSysMessage("No candidate for cre guid %u", (*(sWorld.debugCreatureModPos.begin())).first);
                break;
            }
            case 1: // GO
            {
                Position pos = (*(sWorld.debugGOeModPos.begin())).second;
                PSendSysMessage("Porting to candidate for obj %u", (*(sWorld.debugGOeModPos.begin())).first);
                if (pos.x != 0)
                    pl->TeleportTo(pl->GetMapId(), pos.x, pos.y, pos.z, 0);
                else
                    PSendSysMessage("No candidate for GO guid %u", (*(sWorld.debugGOeModPos.begin())).first);
                break;
            }
            default:
                return true;
        }
    }
    else if (strcmp(cmdStr, "app") == 0)
    {
        uint32 guid = 0;
        switch (type)
        {
            case 0: // creature
            {
                guid = (*(sWorld.debugCreatureModPos.begin())).first;
                Position p = (*(sWorld.debugCreatureModPos.begin())).second;

                //sLog.outLog(LOG_SPECIAL, "UPDATE `creature` SET position_x='%f', position_y='%f', position_z='%f' WHERE guid='%u'", p.x, p.y, p.z, guid);
                sWorld.debugCreatureModPos.erase(guid);
                PSendSysMessage("Auto point approved for cre %u", guid);
                break;
            }
            case 1: // GO
            {
                guid = (*(sWorld.debugGOeModPos.begin())).first;
                Position p = (*(sWorld.debugGOeModPos.begin())).second;

                //sLog.outLog(LOG_SPECIAL, "UPDATE `gameobject` SET position_x='%f', position_y='%f', position_z='%f' WHERE guid='%u'", p.x, p.y, p.z, guid);
                sWorld.debugGOeModPos.erase(guid);
                PSendSysMessage("Auto point approved for obj %u", guid);
                break;
            }
            default:
                return true;
        }
        //sLog.outLog(LOG_SPECIAL, "INSERT INTO checked_objects(guid, type) VALUES('%u', '%u')", guid, type);
    }
    else if (strcmp(cmdStr, "spec") == 0)
    {
        uint32 guid = 0;
        switch (type)
        {
            case 0: // creature
            {
                guid = (*(sWorld.debugCreatureModPos.begin())).first;

                //sLog.outLog(LOG_SPECIAL, "-- SPECIAL UPDATE `creature` SET position_x='X', position_y='X', position_z='X' WHERE guid='%u'", guid);
                sWorld.debugCreatureModPos.erase(guid);
                PSendSysMessage("Special point needed for cre %u", guid);
                break;
            }
            case 1: // GO
            {
                guid = (*(sWorld.debugGOeModPos.begin())).first;

                //sLog.outLog(LOG_SPECIAL, "-- SPECIAL UPDATE `gameobject` SET position_x='X', position_y='X', position_z='X' WHERE guid='%u'", guid);
                sWorld.debugGOeModPos.erase(guid);
                PSendSysMessage("Special point needed for obj %u", guid);
                break;
            }
            default:
                return true;
        }
        //sLog.outLog(LOG_SPECIAL, "INSERT INTO checked_objects(guid, type) VALUES('%u', '%u')", guid, type);
    }
    else if (strcmp(cmdStr, "dis") == 0)
    {
        uint32 guid = 0;
        switch (type)
        {
            case 0: // creature
            {
                guid = (*(sWorld.debugCreatureModPos.begin())).first;
                sWorld.debugCreatureModPos.erase(guid);
                PSendSysMessage("Creature is already fine cre %u", guid);
                break;
            }
            case 1: // GO
            {
                guid = (*(sWorld.debugGOeModPos.begin())).first;
                sWorld.debugGOeModPos.erase(guid);
                PSendSysMessage("Object is already fine obj %u", guid);
                break;
            }
            default:
                return true;
        }

        //sLog.outLog(LOG_SPECIAL, "INSERT INTO checked_objects(guid, type) VALUES('%u', '%u')", guid, type);
    }
    
    return true;
}

bool ChatHandler::HandleGameobjectPoolCommand(const char* args)
{
    Player* p = m_session->GetPlayer();
    if (!p)
        return false;

    // Works by zones
    uint32 curZone = p->GetCachedZone();
    uint32 curMap = p->GetMapId();

    std::string argstr = args ? (char*)args : "";

    bool full = argstr == "full";

    // Select ALL objects that have no game_event and no pool_entry
    // type 6 are objects that activate on object open, like id 181272, they should not be pooled
    QueryResultAutoPtr result;
    if (full) 
        result = GameDataDatabase.Query("SELECT g.guid FROM gameobject g "
            "LEFT OUTER JOIN game_event_gameobject geg ON g.guid=geg.guid "
            "LEFT OUTER JOIN pool_gameobject pg ON g.guid=pg.guid "
            "INNER JOIN gameobject_template gt ON g.id=gt.entry WHERE "
            "geg.event IS NULL AND pg.pool_entry IS NULL AND gt.type!=6");
    else // by map
        result = GameDataDatabase.PQuery("SELECT g.guid FROM gameobject g "
            "LEFT OUTER JOIN game_event_gameobject geg ON g.guid=geg.guid "
            "LEFT OUTER JOIN pool_gameobject pg ON g.guid=pg.guid "
            "INNER JOIN gameobject_template gt ON g.id=gt.entry WHERE "
            "geg.event IS NULL AND pg.pool_entry IS NULL AND gt.type!=6 AND g.map = %u", curMap);
        
    // Select ids of objects that we need to create pool for
    QueryResultAutoPtr resultObjectIDs = GameDataDatabase.Query("SELECT entry FROM pool_gameobject_ids");

    if (!result || !resultObjectIDs) // error.
        return true;   

    std::set<uint32> go_to_pool_ids;
    do
    {
        Field *fields = resultObjectIDs->Fetch();

        uint32 id = fields[0].GetUInt32();
        if (id)
            go_to_pool_ids.insert(id);

    } while (resultObjectIDs->NextRow());

    // pool and game_event are NULL here. No id, cause this is only a place
    struct spot
    {
        float x;
        float y;
        float z;
        std::list<uint32> guids;
        std::list<uint32> ids;
    };

    struct zoneInfo
    {
        std::list<spot> spotsList;
        /*object id, guids of those objecs*/
        //std::map<uint32, std::set<uint32>> objectsByIdSet;
        std::string zoneName;
    };

    std::map<uint32, uint32> mapInstIds;
    // zoneId --- zoneInfo
    std::map<uint32, zoneInfo> zonesMap;
    do
    {
        Field *fields = result->Fetch();

        uint32 lowguid = fields[0].GetUInt32();

        if (GameObjectData const* goData = sObjectMgr.GetGOData(lowguid))
        {
            uint32 zoneId = 0;
            Map* map = sMapMgr.FindMap(goData->mapid, mapInstIds.find(goData->mapid) != mapInstIds.end() ? mapInstIds.find(goData->mapid)->second : 0);
            if (!map)
            {
                const MapEntry* entry = sMapStore.LookupEntry(goData->mapid);
                if (entry->IsBattleGroundOrArena())
                {
                    for (uint32 i = BATTLEGROUND_AV; i < MAX_BATTLEGROUND_TYPE_ID; ++i)
                    {
                        BattleGround* bg = sBattleGroundMgr.CreateNewBattleGround((BattleGroundTypeId)i, BG_BRACKET_ID_LAST, 0, false);
                        if (bg && bg->GetMapId() == goData->mapid)
                        {
                            map = bg->GetMap();
                            mapInstIds[goData->mapid] = map->GetAnyInstanceId();
                            break;
                        }
                    }
                }
                else
                {
                    map = sMapMgr.CreateMap(goData->mapid, m_session->GetPlayer()); // not caring about sInstance here, so no relocate is called before CreateMap
                    mapInstIds[goData->mapid] = map->GetAnyInstanceId();
                }
            }

            if (map)
            {
                if (map->GetTerrain())
                {
                    zoneId = map->GetTerrain()->GetZoneId(goData->posX, goData->posY, goData->posZ);
                    if (!zoneId)
                        zoneId = map->GetTerrain()->GetZoneId(goData->posX, goData->posY, goData->posZ); // retry

                    if (!zoneId)
                        continue; // always skip for instances/not_found

                    if (!full && zoneId != curZone)
                        continue;

                    AreaTableEntry const* zoneEntry = GetAreaEntryByAreaID(zoneId);
                    if (zoneEntry)
                        zonesMap[zoneId].zoneName = zoneEntry->area_name[sWorld.GetDefaultDbcLocale()];
                    else
                    {
                        sLog.outLog(LOG_DEFAULT, "-- Could not determine zoneName for zoneId %u", zoneId);
                        continue;
                    }
                }
                else
                    continue;
            }
            else
                continue;

            //zonesMap[zoneId].objectsByIdSet[goData->id].insert(lowguid);

            // Now to spots
            bool added = false;
            for (auto itr = zonesMap[zoneId].spotsList.begin(); itr != zonesMap[zoneId].spotsList.end(); ++itr)
            {
                if (itr->x == goData->posX &&
                    itr->y == goData->posY &&
                    itr->z == goData->posZ)
                {
                    itr->guids.push_back(lowguid);
                    itr->ids.push_back(goData->id);
                    added = true;
                    break;
                }
            }

            if (!added) // create new object
            {
                spot s;
                s.x = goData->posX;
                s.y = goData->posY;
                s.z = goData->posZ;
                s.guids.push_back(lowguid);
                s.ids.push_back(goData->id);
                
                zonesMap[zoneId].spotsList.push_back(s);
            }
        }
    } while (result->NextRow());


    bool unexpected = false;
    // determine unexpected behaviour / remove unneeded spots

    for (auto zItr = zonesMap.begin(); zItr != zonesMap.end(); ++zItr)
    {
        zoneInfo &info = zItr->second;
        auto &spots = info.spotsList;
        auto next = spots.begin();
        for (auto itr = spots.begin(); itr != spots.end(); itr = next)
        {
            next = itr;
            ++next;

            auto &set = itr->guids;

            bool isPooled = false;
            for (auto guidItr = set.begin(); guidItr != set.end(); ++guidItr)
            {
                if (GameObjectData const* goData = sObjectMgr.GetGOData(*guidItr))
                {
                    // at least one object in this spot belongs to our pool ids
                    if (go_to_pool_ids.find(goData->id) != go_to_pool_ids.end())
                        isPooled = true;
                }
            }

            if (!isPooled)
            {
                spots.erase(itr); // Remove this spot from data at all. Might be some weird objects spot, not what we need.
                continue; // go check next spot
            }

            // If set size IS 1 and we got here, then this one object IS one of the go_to_pool_ids,
            // otherwise we need to make sure that all objects in this spot are right
            if (set.size() > 1)
            {
                // check that all IDs in this spot is in our range of pool_ids, otherwise it's something unexpected
                for (auto guidItr = set.begin(); guidItr != set.end(); ++guidItr)
                {
                    if (GameObjectData const* goData = sObjectMgr.GetGOData(*guidItr))
                    {
                        if (go_to_pool_ids.find(goData->id) == go_to_pool_ids.end())
                        {
                            sLog.outLog(LOG_DEFAULT, "-- gameobject id %u guid %u is unexpected in place with objects that should be in pool. Map %u xy %f %f", goData->id, *guidItr, goData->mapid, goData->posX, goData->posY);
                            unexpected = true;
                        }
                    }
                }
            }
        }
    }

    // Leave only object IDs that we gotta get in pool now. Same as for spots.
    /*for (auto zItr = zonesMap.begin(); zItr != zonesMap.end(); ++zItr)
    {
        zoneInfo &info = zItr->second;
        auto &objByIds = info.objectsByIdSet;
        auto next2 = objByIds.begin();
        for (auto itr = objByIds.begin(); itr != objByIds.end(); itr = next2)
        {
            next2 = itr;
            ++next2;

            uint32 id = itr->first;

            if (go_to_pool_ids.find(id) == go_to_pool_ids.end())
                objByIds.erase(itr); // Remove this id from data at all.
        }
    }*/

    if (unexpected)
    {
        sLog.outLog(LOG_DEFAULT, "Unexpected non-pool object found in spot with pool objects");
        SendSysMessage("Unexpected non-pool object found in spot with pool objects");
        sLog.outLog(LOG_DEFAULT, "-- Unexpected non-pool object found in spot with pool objects");
        return true;
    }


    uint32 nextPoolEntry = 0;
    QueryResultAutoPtr resultMax = GameDataDatabase.Query("SELECT MAX(entry) FROM pool_template");
    if (!resultMax)
    {
        sLog.outLog(LOG_DEFAULT, "Could not get max entry from pool_template");
        SendSysMessage("Could not get max entry from pool_template");
        sLog.outLog(LOG_DEFAULT, "-- Could not get max entry from pool_template, if it is empty then OK");
        nextPoolEntry = 1;
    }
    else
    {
        Field *fields = resultMax->Fetch();
        nextPoolEntry = fields[0].GetUInt16()+1;
    }


    // Now for each zone we gotta create lists of "same-spot-objects" sets, which will later be added into pools
    for (auto zItr = zonesMap.begin(); zItr != zonesMap.end(); ++zItr)
    {
        zoneInfo &info = zItr->second;
        auto &spots = info.spotsList;

        // each spot that has 1 object goes to map of sets of guids
        // id ----- guids
        std::map<uint32, std::set<uint32>> loneSpots;

        // sort ids (need for later comparison of lists) and determine loneSpots (for pools)
        auto next = spots.begin();
        for (auto itr = spots.begin(); itr != spots.end(); itr = next)
        {
            next = itr;
            ++next;
            if (itr->ids.size() > 1) // if more than 1 then sort for later
                itr->ids.sort();
            else
            {
                loneSpots[itr->ids.front()].insert(itr->guids.front());
                // remove from spots now
                spots.erase(itr);
            }
        }

        // loneSpots is now ready to go in DB. Need to decide pool_entry for it
        sLog.outLog(LOG_DEFAULT, "-- Lone spots for zone %u '%s'", zItr->first, info.zoneName.c_str());
        for (auto sItr = loneSpots.begin(); sItr != loneSpots.end(); ++sItr)
        {
            GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(sItr->first);

            sLog.outLog(LOG_DEFAULT, "-- GO id %u %s guids:", sItr->first, goInfo ? goInfo->name : "");
            for (auto setItr = sItr->second.begin(); setItr != sItr->second.end(); ++setItr)
            {
                sLog.outLog(LOG_DEFAULT, "INSERT INTO pool_gameobject VALUES (%u, %u, 0, 'SimplePool zone %u %s object %u %s');", *setItr, nextPoolEntry, zItr->first, info.zoneName.c_str(), sItr->first, goInfo ? goInfo->name : "");
            }
            sLog.outLog(LOG_DEFAULT, "INSERT INTO pool_template VALUES (%u, '1', 'SimplePool zone %u %s object %u %s, 1 out of %u');", nextPoolEntry, zItr->first, info.zoneName.c_str(), sItr->first, goInfo ? goInfo->name : "", sItr->second.size());
            ++nextPoolEntry;
        }

        // now we have spots with > 1 objects in them. Ids are all sorted
        // list of object ids as identifier ------- list of pool entries
        std::map<std::list<uint32>, std::list<uint32>> multipleSpots;

        // each spot that has > 1 object gets own internal pool
        // pool entry ----- list of guids
        std::map<uint32, std::list<uint32>> internalPools;

        for (auto itr = spots.begin(); itr != spots.end(); ++itr)
        {
            internalPools[nextPoolEntry] = itr->guids;
            multipleSpots[itr->ids].push_back(nextPoolEntry);

            ++nextPoolEntry;
        }

        // internalPools is now ready to go into DB, already have pool_entry decided for it
        sLog.outLog(LOG_DEFAULT, "-- Internal pools for zone %u '%s'", zItr->first, info.zoneName.c_str());
        for (auto inItr = internalPools.begin(); inItr != internalPools.end(); ++inItr)
        {
            for (auto listItr = inItr->second.begin(); listItr != inItr->second.end(); ++listItr)
            {
                sLog.outLog(LOG_DEFAULT, "INSERT INTO pool_gameobject VALUES (%u, %u, 0, 'InternalPool zone %u %s');", *listItr, inItr->first, zItr->first, info.zoneName.c_str());
            }
            sLog.outLog(LOG_DEFAULT, "INSERT INTO pool_template VALUES (%u, '1', 'InternalPool zone %u %s');", inItr->first, zItr->first, info.zoneName.c_str());
        }

        // multipleSpots is now ready to go in DB. Need to decide pool_entry for it
        sLog.outLog(LOG_DEFAULT, "-- MultiPools for zone %u '%s'", zItr->first, info.zoneName.c_str());
        for (auto msItr = multipleSpots.begin(); msItr != multipleSpots.end(); ++msItr)
        {
            // make a list of ids to print
            std::string toPrintIds = "";
            for (auto sId = msItr->first.begin(); sId != msItr->first.end(); ++sId)
            {
                GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(*sId);

                char chr[512];
                sprintf(chr, " %u %s", *sId, goInfo ? goInfo->name : "");
                toPrintIds += chr;
            }

            sLog.outLog(LOG_DEFAULT, "-- GO ids %s Internal pool entries:", toPrintIds.c_str());
            for (auto listItr = msItr->second.begin(); listItr != msItr->second.end(); ++listItr)
            {
                sLog.outLog(LOG_DEFAULT, "INSERT INTO pool_pool VALUES (%u, %u, 0, 'MultiPool zone %u %s objects %s');", *listItr, nextPoolEntry, zItr->first, info.zoneName.c_str(), toPrintIds.c_str());
            }

            sLog.outLog(LOG_DEFAULT, "INSERT INTO pool_template VALUES (%u, '1', 'MultiPool zone %u %s objects %s, 1 out of %u');", nextPoolEntry, zItr->first, info.zoneName.c_str(), toPrintIds.c_str(), msItr->second.size());
            ++nextPoolEntry;
        }
    }

    SendSysMessage("Done");
    return true;
}

bool ChatHandler::HandleBG(const char* args)
{
    Player *plr = m_session->GetPlayer();
    BattleGround* bg = plr->GetBattleGround();

    if (!bg)
        return false;

    std::string argstr = (char*)args;

    if (argstr == "start")
        bg->SetStartDelayTime(1);
	else if (argstr == "horde")
		bg->EndBattleGround(HORDE);
	else if (argstr == "alliance")
		bg->EndBattleGround(ALLIANCE);
    else if (argstr == "end")
        bg->EndBattleGround(TEAM_NONE);
        
    return true;
}

bool ChatHandler::HandleThrowCode(const char* args)
{
    if (!*args)
        return false;

    std::string code = strtok((char*)args, " ");
    sWorld.m_bonusCode = code;

    sWorld.SendWorldText(LANG_BONUS_CODE_THROWN, 0);
    return true;
}

// mw command mwcommand
bool ChatHandler::HandleMW(const char * args)
{
	if (!*args)
		return false;

    Player* player = m_session->GetPlayer();
    if (!player)
        return false;

	char* par1 = strtok((char*)args, " ");
	char* par2 = strtok(NULL, "");
	if (!par1)
		return false;

	std::string arg = par1;
	std::string arg2 = "";
	if (par2)
		arg2 = par2;

	if (arg == "iphash_clear")
	{
		auto a = sWorld.haship_arena_queue;	
		sWorld.haship_arena_queue.clear();
	}
    else if (arg == "iphash_print")
    {
        for (auto& val : sWorld.haship_arena_queue)
        {
            ChatHandler(player).PSendSysMessage("Player guid: %u, hash: %u", val.first, val.second);
        }
    }
    else if (arg == "gs")
    {
        if (Player* target = getSelectedPlayer())
        {
            ChatHandler(player).PSendSysMessage("Player gearscore: %u", target->GetGearScore());
        }
    }
    else if (arg == "talents")
    {
        if (Player* target = getSelectedPlayer())
        {
            ChatHandler(player).PSendSysMessage("Player talents: %s, healer %u (semi %u)", target->PrintTalentCount(target->GetActiveSpec()).c_str(), target->IsHealer(), target->IsSemiHealer());
        }
    }
    else if (arg == "fix_creature")
    {
        if (Creature* target = getSelectedCreature())
        {
            target->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    } 

	return true;
}

bool ChatHandler::HandleBanArena3v3Command(const char* args)
{
    return HandleBanHelper(BAN_ARENA3V3, args);
}

bool ChatHandler::HandleUnBanArena3v3Command(const char* args)
{
    return HandleUnBanHelper(BAN_ARENA3V3, args);
}

bool ChatHandler::HandleBanInfoArena3v3Command(const char* args)
{
    if (!args)
        return false;

    char* cname = strtok((char*)args, "");
    if (!cname)
        return false;

    std::string name = cname;
    if (!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 accountid = sObjectMgr.GetPlayerAccountIdByPlayerName(name);
    if (!accountid)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string accountname;
    if (!AccountMgr::GetName(accountid, accountname))
    {
        SendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }

    return HandleBanInfoHelper(accountid, accountname.c_str(), PUNISHMENT_ARENA3V3);
}

bool ChatHandler::HandleBanListArena3v3Command(const char* args)
{
    char* cFilter = strtok((char*)args, " ");
    std::string filter = cFilter ? cFilter : "";
    AccountsDatabase.escape_string(filter);

    QueryResultAutoPtr result;

    if (filter.empty())
        result = AccountsDatabase.PQuery("SELECT account.account_id, username "
            "FROM account JOIN account_punishment ON account.account_id = account_punishment.account_id "
            "WHERE punishment_type_id = '%u' AND active = 1 GROUP BY account.account_id", PUNISHMENT_BAN);
    else
        result = AccountsDatabase.PQuery("SELECT account.account_id, username "
            "FROM account JOIN account_punishment ON account.account_id = account_punishment.account_id "
            "WHERE punishment_type_id = '%u' AND active = 1 AND "
            "username LIKE '%%%s%%' GROUP BY account.account_id", PUNISHMENT_BAN, filter.c_str());


    if (!result)
    {
        SendSysMessage(LANG_BANLIST_NOACCOUNT);
        return true;
    }

    return HandleBanListHelper(result);
}

bool ChatHandler::HandleSetMapMod(const char* args)
{
    if (!*args)
        return false;

    //Player* player = m_session->GetPlayer();
    //if (!player)
    //    return false;

    //char* c_damage = strtok((char*)args, " ");
    //if (!c_damage)
    //    return false;

    //char* c_hp = strtok(NULL, "");
    //if (!c_hp)
    //    return false;

    //float damage = (float)atof(c_damage);
    //float hp = (float)atof(c_hp);

    //if (damage <= 0 || hp <= 0)
    //    return false;

    //Map* map = player->GetMap();
    //if (!map)
    //    return false;

    //uint32 pMap = map->GetId();
    //uint32 heroic = map->IsHeroic();
        
    // todo
    //GameDataDatabase.PExecute("REPLACE INTO creature_map_mod_normal (map,damage,hp,comment) values ('%u','%f','%f','%s')", pMap, damage, hp, map->GetMapName());

    //SendSysMessage("Modifications have been uploaded. A server restart is required.");
    
    return true;
}

bool ChatHandler::HandleGetMapMod(const char* args)
{
    Player* player = m_session->GetPlayer();
    if (!player)
        return false;
    
    Map* map = player->GetMap();
    if (!map)
        return false;

    uint32 pMap = map->GetId();
    bool heroic = map->IsHeroic();
    
    ModDamageHP mod = Map::GetMapCreatureMod(pMap, heroic);

    PSendSysMessage("MapID: %u, Name: %s, Heroic: %s", pMap, map->GetMapName(), heroic ? "Yes" : "No");
    PSendSysMessage("Damage modifiers: %.2f", mod.damage);
    PSendSysMessage("HP modifiers: %.2f", mod.hp);

    if (map->IsDungeon() && !heroic)
        SendSysMessage("+ Trash HP reduced by 50%");

    return true;
}

//handletest
//testcommand
//mwtest
// testcommand test command
bool ChatHandler::HandleTest(const char* args)
{
    //PSendSysMessage("Syntax is: ahbotoptions bidsperinterval $ahMapID (2, 6 or 7) $bids");
    Player* p = nullptr;

    if (m_session)
        p = m_session->GetPlayer();

    std::string command;
    char* carg1;
    char* carg2;
    char* carg3;
    std::string sarg1;
    std::string sarg2;
    std::string sarg3;
    uint32 arg1;
    uint32 arg2;
    uint32 arg3;

    if (*args)
    {
        command = strtok((char*)args, " ");
        carg1 = strtok((char*)NULL, " ");
        carg2 = strtok((char*)NULL, " ");
        carg3 = strtok((char*)NULL, " ");
    }

    if (carg1)
    {
        arg1 = atoi(carg1);
        sarg1 = carg1;
        if (carg2)
        {
            arg2 = atoi(carg2);
            sarg2 = carg2;
        }
        if (carg3)
        {
            arg3 = atoi(carg3);
            sarg3 = carg3;
        }
    }

    //if (!*args)
    //{
    //    p->CastSpell(p, 20375, false);
    //    Aura* aura = p->GetAura(20375, 0);
    //    aura->SetAuraDuration(500);
    //}
    
    Unit* target = getSelectedUnit();

    if (command == "a")
    {
        std::string msg = ChatHandler(p).PGetSysMessage(16732, 60000 / 1000);

        ChatHandler(p).SendSysMessage(msg.c_str());
        p->SendCenterMessage(msg.c_str());

        //WorldPacket data(MSG_RAID_READY_CHECK, 8);
        //data << p->GetGUID();
        //p->SendPacketToSelf(&data);

        //Player* pUser = _player;
        //uint8 bagIndex, slot;
        //uint8 spell_count;      
        //uint8 cast_count;       
        //uint64 item_guid;


        // remove old enchanting before applying new if equipped
        //p->ApplyEnchantment(Item, TEMP_ENCHANTMENT_SLOT, false);
        //
        //Item->SetEnchantment(TEMP_ENCHANTMENT_SLOT, 2506, 1000 * 1000, 0);
        //
        //// add new enchanting if equipped
        //p->ApplyEnchantment(Item, TEMP_ENCHANTMENT_SLOT, true);

        /*sLog.outString("Walking? %u, speed %f %f, moveflags %u", target->IsWalking(), target->GetSpeed(MOVE_WALK), target->GetSpeed(MOVE_RUN), target->GetUnitMovementFlags());*/
    }
    else if (command == "b")
    {
        sWorld.SelectNextBGEvent(21, EVENT_BG_ALTERAC);
    }
    else if (command == "c")
    {
        Pet* pet = p->GetPet();
        if (pet)
            pet->SetFacingTo(p->GetOrientation());
    }
    else if (command == "swpreset")
    {
        sGameEventMgr.SWPResetHappens();
    }
    else if (command == "color")
    {
        ChatHandler(p).PSendSysMessage("|cff%sLorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.|r", carg1);
    }
    else if (command == "test")
    {
        p->KilledMonster(200900, 0);
        p->KilledMonster(200901, 0);
    }
    else if (command == "test1")
    {
        float x, y, z;
        p->GetNearPoint(x, y, z, 0.0f, 0.1f, frand(0, 2 * M_PI));
        p->SummonCreature(21857, x, y, p->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
    }
    else if (command == "test2")
    {
        float x, y, z;
        p->GetNearPoint(x, y, z, 0.0f, 30.0f, frand(0, 2 * M_PI));
        p->SummonCreature(693131, x, y, z, 0, TEMPSUMMON_TIMED_DESPAWN, 5000);
    }
    else if (command == "test3")
    {
        Position dest;
        p->GetValidPointInAngle(dest, 30.0f, frand(0.0f, 2 * M_PI), true);
        p->SummonCreature(693131, dest.x, dest.y, dest.z, 0.f, TEMPSUMMON_TIMED_DESPAWN, 900);
    }
    else if (command == "speed")
    {
        bool force = carg3 == "f" ? true : false;

        if (sarg1 == "run")
        {
            target->SetSpeed(MOVE_RUN, arg2, force);
            SendSysMessage("Speed updated");
            return false;
        }
        else if (sarg1 == "walk")
        {
            target->SetSpeed(MOVE_WALK, arg2, force);
            SendSysMessage("Speed updated");
            return false;
        }
    }
    else
        if (command == "string")
        {
            sObjectMgr.LoadHellgroundStrings();
            SendSysMessage(arg1);
        }

        else if (command == "regarena")
        {
            Unit* bg_master = getSelectedUnit();
            if (!bg_master || bg_master->GetTypeId() != TYPEID_UNIT)
            {
                SendSysMessage("Select BG master");
                SetSentErrorMessage(true);
                return false;
            }

            SessionMap sessionmap = sWorld.GetSessions();
            for (SessionMap::iterator itr = sessionmap.begin(); itr != sessionmap.end(); ++itr)
            {
                if (!itr->second)
                    continue;

                Player* pl = itr->second->GetPlayer();

                if (!pl || !pl->IsInWorld())
                    continue;

                //float x, y, z;
                //p->GetNearPoint(x, y, z, pl->GetObjectSize());
                //pl->TeleportTo(p->GetMapId(), x, y, z, pl->GetOrientation());

                WorldPacket Data;
                Data << (uint64)bg_master->GetGUID() << (uint8)ArenaTeam::GetSlotByType(ARENA_TEAM_3v3)/*3v3 slot*/ << (uint8)0/*asGroup*/ << (uint8)1/*isRated*/;
                pl->GetSession()->HandleArenaJoinOpcode(Data);
            }
        }

        //find item model
        else if (command == "item_set" || command == "item_forward" || command == "item_back")
        {
            uint32 itemid;
            int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (15 * MAX_VISIBLE_ITEM_OFFSET);

            if (command == "item_set")
            {
                itemid = arg1;
            }
            else if (command == "item_forward")
            {
                itemid = p->GetMaxHealth() + 1;

                //QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT entry FROM items_unused WHERE entry > %u", p->GetMaxHealth());
                //if (result)
                //    itemid = (*result)[0].GetUInt32();
            }
            else if (command == "item_back")
            {
                itemid = p->GetMaxHealth() - 1;

                //QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT entry FROM items_unused WHERE entry < %u", p->GetMaxHealth());
                //if (result)
                //    itemid = (*result)[0].GetUInt32();
            }
            else
            {
                return false;
            }

            while (true)
            {
                if (command != "item_set")
                {
                    if (itemid == 0 || itemid > 699999)
                        break;

                    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(itemid);
                    if (!proto)
                    {
                        itemid = command == "item_back" ? --itemid : ++itemid;
                        continue;
                    }

                    //if (!(proto->Class == 2 && proto->SubClass == 13))
                    if (!(proto->Class == 2 && (proto->SubClass == 10)))
                    {
                        itemid = command == "item_back" ? --itemid : ++itemid;
                        continue;
                    }
                }

                for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)         // string: item id, ench (perm/temp)
                {
                    if (i == 15 || i == 16 || i == 17)
                        p->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + (i * MAX_VISIBLE_ITEM_OFFSET), itemid);
                }

                p->SetMaxHealth(itemid);
                //player->SetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + (16 * MAX_VISIBLE_ITEM_OFFSET), itemid);
                PSendSysMessage("Item entry: %u", itemid);
                break;
            }
        }

        //find creature model
        else if (command == "npc_set" || command == "npc_forward" || command == "npc_back")
        {
            uint32 model = 0;
            uint32 npc_entry = 0;

            if (command == "npc_set")
            {
                model = arg1;
            }
            else if (command == "npc_forward")
            {
                //entry = p->GetMaxHealth() + 1;

                QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT min(displayid),entry FROM commnad_creature_models WHERE displayid > %u", p->GetMaxHealth());
                if (result)
                {
                    model = (*result)[0].GetUInt32();
                    npc_entry = (*result)[1].GetUInt32();
                }
            }
            else if (command == "npc_back")
            {
                //entry = p->GetMaxHealth() - 1;

                QueryResultAutoPtr result = GameDataDatabase.PQuery("SELECT max(displayid),entry FROM commnad_creature_models WHERE displayid < %u", p->GetMaxHealth());
                if (result)
                {
                    model = (*result)[0].GetUInt32();
                    npc_entry = (*result)[1].GetUInt32();
                }
            }
            else
            {
                return false;
            }

            p->SetDisplayId(model);

            p->SetMaxHealth(model);
            PSendSysMessage("Display: %u, NPC entry %u", model, npc_entry);

            //while (true)
            //{
            //    //CreatureInfo const* proto;
            //    
            //    //if (command != "npc_set")
            //    //{
            //    //    if (model == 0 || model > 699999)
            //    //        break;

            //    //    proto = GetCreatureTemplateStore(model);
            //    //    if (!proto)
            //    //    {
            //    //        model = command == "npc_back" ? --model : ++model;
            //    //        continue;
            //    //    }
            //    //}

            //    //uint32 model_id = proto ? proto->Modelid_H1 : 0;


            //}
        }

        //find mount model
        else if (command == "mount_set")
        {
            p->AddAura(54839, p);
            uint32 model = arg1;
            p->SetMaxHealth(model);
            p->SetHealth(model);
            p->AddAura(42777, p);
            PSendSysMessage("Model: %u", p->GetMaxHealth());
        }
        else if (command == "mount_forward")
        {
            p->AddAura(54839, p);
            uint32 model = p->GetMaxHealth() + 1;
            p->SetMaxHealth(model);
            p->SetHealth(model);
            p->AddAura(42777, p);
            PSendSysMessage("Model: %u", p->GetMaxHealth());
        }
        else if (command == "mount_back")
        {
            p->AddAura(54839, p);
            uint32 model = p->GetMaxHealth() - 1;
            p->SetMaxHealth(model);
            p->SetHealth(model);
            p->AddAura(42777, p);
            PSendSysMessage("Model: %u", p->GetMaxHealth());
        }

        //find morph model
        else if (command == "model_set")
        {
            uint32 model = arg1;
            p->SetMaxHealth(model);
            p->SetDisplayId(model);
            PSendSysMessage("Model: %u", p->GetMaxHealth());
        }
        else if (command == "model_forward")
        {
            uint32 model = p->GetMaxHealth() + 1;
            p->SetMaxHealth(model);
            p->SetDisplayId(model);
            PSendSysMessage("Model: %u", p->GetMaxHealth());
        }
        else if (command == "model_back")
        {
            uint32 model = p->GetMaxHealth() - 1;
            p->SetMaxHealth(model);
            p->SetDisplayId(model);
            PSendSysMessage("Model: %u", p->GetMaxHealth());
        }
    // ---------------------- END TEST

    return true;
}

// don't paste anything below