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

#ifndef HELLGROUND_CHAT_H
#define HELLGROUND_CHAT_H

#include "SharedDefines.h"

class ChatHandler;
class WorldSession;
class Creature;
class Player;
class Unit;
struct GameTele;

static const std::string raidRuleStr[4][2] = 
{
    { "П:", "П+:" },
    { "ПРАВИЛА:", "ПРАВИЛА+:" },
    { "R:", "R+:" },
    { "RULES:", "RULES+:"}    
};

class ChatCommand
{
    public:
        const char *       Name;
        uint64             RequiredPermissions;                   // function pointer required correct align (use uint32)
        uint64             SelfPermissions;
        bool               AllowConsole;
        bool (ChatHandler::*Handler)(const char* args);
        std::string        Help;
        ChatCommand *      ChildCommands;
};

class HELLGROUND_IMPORT_EXPORT ChatHandler
{
    public:
        explicit ChatHandler(WorldSession* session) : m_session(session) {}
        explicit ChatHandler(Player* player) : m_session(player->GetSession()) {}
         ~ChatHandler() {}

        static void FillMessageData(WorldPacket *data, WorldSession* session, uint8 type, uint32 language, const char *channelName, uint64 target_guid, const char *message, Unit *speaker);

        void FillMessageData(WorldPacket *data, uint8 type, uint32 language, uint64 target_guid, const char* message)
        {
            FillMessageData(data, m_session, type, language, NULL, target_guid, message, NULL);
        }

        void FillSystemMessageData(WorldPacket *data, const char* message)
        {
            FillMessageData(data, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, message);
        }

        static char* LineFromMessage(char*& pos) { char* start = strtok(pos,"\n"); pos = NULL; return start; }
        static std::string GetNameLink(const std::string & name);

        virtual const char *GetHellgroundString(int32 entry) const;

        virtual void SendSysMessage( const char *str);
        void SendSysMessage(int32 entry);
        void SendNotification(int32 entry);
        void PSendSysMessage(const char *format, ...) ATTR_PRINTF(2,3);
        void PSendSysMessage(int32 entry, ... );
        const char* PGetSysMessage(int32 entry, ...);

        int ParseCommands(const char* text);

        static bool ContainsNotAllowedSigns(std::string text,bool strict = false);

        virtual char const* GetName() const;
        static ChatCommand* getCommandTable();
        bool isValidChatMessage(const char* msg);

        bool HandleServerInfoCommand(const char* args);

    protected:
        explicit ChatHandler() : m_session(NULL) {}      // for CLI subclass

        bool hasStringAbbr(const char* name, const char* part);

        virtual bool isAvailable(ChatCommand const& cmd, bool self) const;
        virtual bool needReportToTarget(Player* chr) const;

        void SendGlobalSysMessage(const char *str);
        void SendGlobalGMSysMessage(const char *str);
        void SendGlobalGMSysMessage(int32 entry, ...);

        bool SendGMMail(const char* pName, const char* msgSubject, const char* msgText);

        bool ExecuteCommandInTable(ChatCommand *table, const char* text, std::string& fullcommand);
        bool ShowHelpForCommand(ChatCommand *table, const char* cmd);
        bool ShowHelpForSubCommands(ChatCommand *table, char const* cmd, char const* subcmd);

        bool HandleAccountCommand(const char* args);
        bool HandleAccountCreateCommand(const char* args);
        bool HandleAccountDeleteCommand(const char* args);
        bool HandleAccountOnlineListCommand(const char* args);
        bool HandleAccountSetAddonCommand(const char* args);
        bool HandleAccountSetPermissionsCommand(const char* args);
        bool HandleAccountSetPasswordCommand(const char* args);
        bool HandleAccountWeatherCommand(const char*args);
        //bool HandleAccountSpecialLogCommand(const char* args);
        bool HandleAccountWhispLogCommand(const char* args);
        bool HandleAccountGuildAnnToggleCommand(const char* args);
        bool HandleAccountBonesHideCommand(const char* args);
        bool HandleAccountBattleGroundAnnCommand(const char* args);
        bool HandleLootAnnounces(const char* args);
        //bool HandleAccountBGMarksCommand(const char* args);
        //bool HandleAccountAnnounceBroadcastCommand(const char* args);
        bool HandleLanguageCommand(const char* args);
        //bool HandleAccountAltRealmCommand(const char* args);
        
        bool HandleBanAccountCommand(const char* args);
        bool HandleBanCharacterCommand(const char* args);
        bool HandleBanIPCommand(const char* args);
        bool HandleBanInfoAccountCommand(const char* args);
        bool HandleBanInfoCharacterCommand(const char* args);
        bool HandleBanInfoIPCommand(const char* args);
        bool HandleBanListAccountCommand(const char* args);
        bool HandleBanListCharacterCommand(const char* args);
        bool HandleBanListIPCommand(const char* args);

        bool HandleBanArena3v3Command(const char* args);
        bool HandleBanInfoArena3v3Command(const char* args);
        bool HandleBanListArena3v3Command(const char* args);
        bool HandleUnBanArena3v3Command(const char* args);

        bool HandleMarkBot(const char* args);

        bool HandleCastCommand(const char *args);
        bool HandleCastBackCommand(const char *args);
        bool HandleCastDistCommand(const char *args);
        bool HandleCastNullCommand(const char *args);
        bool HandleCastSelfCommand(const char *args);
        bool HandleCastTargetCommand(const char *args);

        bool HandleCrashMapCommand(const char *args);
        bool HandleCrashServerCommand(const char *args);
        bool HandleCrashFreezeCommand(const char *args);
        bool HandleCrashMemleakCommand(const char *args);

        bool HandleCacheItemCommand(const char *args);

        //bool HandleLootItemInfoCommand(const char *args);
        //bool HandleLootPlayerInfoCommand(const char *args);
        //bool HandleLootDeleteCommand(const char *args);
        //bool HandleLootMoveCommand(const char *args);

        bool HandleWardenForceCheckCommand(const char *args);
        bool HandleTestGridNeedsVMapWaterCommand(const char *args);
        bool HandleFakeBotAdd(const char *args);
        bool HandleCreatureCheckCommand(const char *args);
        bool HandleGameobjectCheckCommand(const char *args);
        bool HandleGameobjectDuplicateCommand(const char *args);
        bool HandleCreatureDuplicateCommand(const char *args);
        bool HandleGameobjectPoolCommand(const char *args);
        bool HandleDebugCheckNewPosCommand(const char *args);

        bool HandleDebugAddFormationToFileCommand(const char* args);
        bool HandleDebugFormationCommand(const char* args);
        bool HandleDebugAnimCommand(const char* args);
        bool HandleDebugArenaCommand(const char * args);
        bool HandleDebugBattleGroundCommand(const char * args);
        bool HandleBG(const char * args);
        bool HandleDebugBossEmoteCommand(const char* args);
        bool HandleDebugCooldownsCommand(const char* args);
        bool HandleDebugGetInstanceDataCommand(const char* args);
        bool HandleDebugGetInstanceData64Command(const char* args);
        bool HandleDebugGetItemState(const char * args);
        bool HandleDebugGetLootRecipient(const char * args);
        bool HandleDebugGetValue(const char* args);
        bool HandleDebugJoinBG(const char* args);
        bool HandleDebugMod32Value(const char* args);
        bool HandleDebugPlayCinematicCommand(const char* args);
        bool HandleDebugPlaySoundCommand(const char* args);
        bool HandleDebugPlayRangeSound(const char* args);
        bool HandleDebugSetInstanceDataCommand(const char* args);
        bool HandleDebugSetInstanceData64Command(const char* args);
        bool HandleDebugSetItemFlagCommand(const char * args);
        bool HandleDebugSetSpellSpeed(const char* args);
        bool HandleDebugSetValue(const char* args);
        bool HandleDebugShowCombatStats(const char* args);
        bool HandleDebugThreatList(const char * args);
        bool HandleDebugUnitState(const char * args);
        bool HandleDebugUpdate(const char* args);
        bool HandleDebugUpdateWorldStateCommand(const char* args);
        //bool HandleDebugCooldownCommand(const char* args);
        bool HandleDebugVmapsCommand(const char* args);
        //bool HandleDebugWPCommand(const char* args);

        bool HandleDebugSendBattlegroundOpcodes(const char* args);
        bool HandleDebugSendBuyErrorCommand(const char* args);
        bool HandleDebugSendChannelNotifyCommand(const char* args);
        bool HandleDebugSendChatMsgCommand(const char* args);
        bool HandleDebugSendEquipErrorCommand(const char* args);
        bool HandleDebugSendOpcodeCommand(const char* args);
        bool HandleDebugSendPetSpellInitCommand(const char* args);
        bool HandleDebugSendPoiCommand(const char* args);
        bool HandleDebugSendQuestPartyMsgCommand(const char* args);
        bool HandleDebugSendQuestInvalidMsgCommand(const char* args);
        bool HandleDebugSendSellErrorCommand(const char* args);
        bool HandleDebugSendSpellFailCommand(const char* args);

        bool HandleEventActiveListCommand(const char* args);
        bool HandleEventAwardCommand(const char* args);
        bool HandleEventStartCommand(const char* args);
        bool HandleEventStopCommand(const char* args);
        bool HandleEventInfoCommand(const char* args);

        bool HandleGameObjectActivateCommand(const char* args);
        bool HandleGameObjectAddCommand(const char* args);
        bool HandleGameObjectAddTempCommand(const char* args);
        bool HandleGameObjectDeleteCommand(const char* args);
        bool HandleGameObjectGridCommand(const char* args);
        bool HandleGameObjectMoveCommand(const char* args);
        bool HandleGameObjectNearCommand(const char* args);
        bool HandleGameObjectResetCommand(const char* args);
        bool HandleGameObjectStateCommand(const char* args);
        bool HandleGameObjectTargetCommand(const char* args);
        bool HandleGameObjectTurnCommand(const char* args);

        //bool HandleAccountFriendAddCommand(const char* args);
        //bool HandleAccountFriendDeleteCommand(const char* args);
        //bool HandleAccountFriendListCommand(const char* args);

        bool HandleGMAnnounceCommand(const char* args);
        bool HandleGMCommand(const char* args);
        bool HandleGMChatCommand(const char* args);
        bool HandleGMFlyCommand(const char* args);
        bool HandleGMListFullCommand(const char* args);
        bool HandleGMNameAnnounceCommand(const char* args);
        bool HandleGMNotifyCommand(const char* args);
        bool HandleGMVisibleCommand(const char* args);
        bool HandleGMIngameCommand(const char* args);
        bool HandleGMFreeCommand(const char* args);

        //bool HandleGuildAdvertCommand(const char *args);
        bool HandleGuildAnnounceCommand(const char *args);
        bool HandleGuildCreateCommand(const char* args);
        bool HandleGuildRenameCommand(const char* args);
        bool HandleGuildInviteCommand(const char* args);
        bool HandleGuildUninviteCommand(const char* args);
        bool HandleGuildRankCommand(const char* args);
        bool HandleGuildDeleteCommand(const char* args);

        bool HandleGuildDisableAnnounceCommand(const char *args);
        bool HandleGuildEnableAnnounceCommand(const char *args);

        bool HandleHonorAddCommand(const char* args);
        bool HandleHonorAddKillCommand(const char* args);
        bool HandleHonorUpdateCommand(const char* args);

        bool HandleInstanceListBindsCommand(const char* args);
        bool HandleInstanceUnbindCommand(const char* args);
        bool HandleInstanceSelfUnbindCommand(const char* args);
        bool HandleInstanceStatsCommand(const char* args);
        bool HandleInstanceSaveDataCommand(const char * args);
        bool HandleInstanceBindCommand(const char* args);
        bool HandleInstanceResetEncountersCommand(const char* args);
        bool HandleInstanceInsidePlayersCommand(const char* args);
        bool HandleInstanceGetData(const char* args);
        bool HandleInstanceSetData(const char* args);

        bool HandleMmapPathCommand(const char* args);
        bool HandleMmapLocCommand(const char* args);
        bool HandleMmapLoadedTilesCommand(const char* args);
        bool HandleMmapStatsCommand(const char* args);
        bool HandleMmapOffsetCreateCommand(const char* /*args*/);
        bool HandleMmap(const char* args);
        bool HandleMmapTestArea(const char* args);
        bool HandleMmapFollowArea(const char* args);

        bool HandleLearnCommand(const char* args);
        bool HandleLearnAllCommand(const char* args);
        bool HandleLearnAllGMCommand(const char* args);
        bool HandleLearnAllCraftsCommand(const char* args);
        bool HandleLearnAllRecipesCommand(const char* args);
        bool HandleLearnAllDefaultCommand(const char* args);
        bool HandleLearnAllLangCommand(const char* args);
        bool HandleLearnAllMyClassCommand(const char* args);
        bool HandleLearnAllMySpellsCommand(const char* args);
        bool HandleLearnAllMyTalentsCommand(const char* args);

        bool HandleListAurasCommand(const char * args);
        bool HandleListCreatureCommand(const char* args);
        bool HandleListItemCommand(const char* args);
        bool HandleListObjectCommand(const char* args);

        bool HandleLookupAreaCommand(const char* args);
        bool HandleLookupCreatureCommand(const char* args);
        bool HandleLookupEventCommand(const char* args);
        bool HandleLookupFactionCommand(const char * args);
        bool HandleLookupItemCommand(const char * args);
        bool HandleLookupItemSetCommand(const char * args);
        bool HandleLookupObjectCommand(const char* args);
        bool HandleLookupPlayerIpCommand(const char* args);
        bool HandleLookupPlayerAccountCommand(const char* args);
        bool HandleLookupPlayerEmailCommand(const char* args);
        bool HandleLookupQuestCommand(const char* args);
        bool HandleLookupSkillCommand(const char* args);
        bool HandleLookupSpellCommand(const char* args);
        bool HandleLookupTeleCommand(const char * args);

        //bool HandleModForceACCommand(const char* args);
        bool HandleModifyAddTitleCommand(const char* args);
        bool HandleModifyRemoveTitleCommand(const char* args);
        bool HandleModifyAwardTitleCommand(const char* args);
        bool HandleModifyKnownTitlesCommand(const char* args);
        bool HandleModifyHPCommand(const char* args);
        bool HandleModifyManaCommand(const char* args);
        bool HandleModifyRageCommand(const char* args);
        bool HandleModifyEnergyCommand(const char* args);
        bool HandleModifyMoneyCommand(const char* args);
        bool HandleModifyASpeedCommand(const char* args);
        bool HandleModifySpeedCommand(const char* args);
        bool HandleModifyBWalkCommand(const char* args);
        bool HandleModifyFlyCommand(const char* args);
        bool HandleModifySwimCommand(const char* args);
        bool HandleModifyScaleCommand(const char* args);
        bool HandleModifyMountCommand(const char* args);
        bool HandleModifyBitCommand(const char* args);
        bool HandleModifyFactionCommand(const char* args);
        bool HandleModifySpellCommand(const char* args);
        bool HandleModifyTalentCommand (const char* args);
        bool HandleModifyHonorCommand (const char* args);
        bool HandleModifyRepCommand(const char* args);
        bool HandleModifyArenaCommand(const char* args);
        bool HandleModifyGenderCommand(const char* args);

        bool HandleModifyStrengthCommand(const char* args);
        bool HandleModifyAgilityCommand(const char* args);
        bool HandleModifyStaminaCommand(const char* args);
        bool HandleModifyIntellectCommand(const char* args);
        bool HandleModifySpiritCommand(const char* args);
        bool HandleModifyArmorCommand(const char* args);
        bool HandleModifyHolyCommand(const char* args);
        bool HandleModifyFireCommand(const char* args);
        bool HandleModifyNatureCommand(const char* args);
        bool HandleModifyFrostCommand(const char* args);
        bool HandleModifyShadowCommand(const char* args);
        bool HandleModifyArcaneCommand(const char* args);
        bool HandleModifyMeleeApCommand(const char* args);
        bool HandleModifyRangedApCommand(const char* args);
        bool HandleModifySpellPowerCommand(const char* args);
        bool HandleModifyMeleeCritCommand(const char* args);
        bool HandleModifyRangedCritCommand(const char* args);
        bool HandleModifySpellCritCommand(const char* args);
        bool HandleModifyMainSpeedCommand(const char* args);
        bool HandleModifyOffSpeedCommand(const char* args);
        bool HandleModifyRangedSpeedCommand(const char* args);
        bool HandleModifyCastSpeedCommand(const char* args);
        bool HandleModifyBlockCommand(const char* args);
        bool HandleModifyDodgeCommand(const char* args);
        bool HandleModifyParryCommand(const char* args);
        bool HandleModifyCrCommand(const char* args);
        bool HandleModifyBrCommand(const char* args);

        bool HandleNpcAddCommand(const char* args);
        bool HandleNpcAddMoveCommand(const char* args);
        bool HandleNpcChangeEntryCommand(const char *args);
        bool HandleNpcDeleteCommand(const char* args);
        bool HandleNpcExtraFlagCommand(const char* args);
        bool HandleNpcFactionIdCommand(const char* args);
        bool HandleNpcFieldFlagCommand(const char* args);
        bool HandleNpcFlagCommand(const char* args);
        bool HandleNpcFollowCommand(const char* args);
        bool HandleNpcInfoCommand(const char* args);
        bool HandleUnitInfoCommand(const char* args);
        bool HandleNpcMoveCommand(const char* args);
        bool HandleNpcPlayEmoteCommand(const char* args);
        bool HandleNpcSayCommand(const char* args);
        bool HandleNpcSetModelCommand(const char* args);
        bool HandleNpcSetMoveTypeCommand(const char* args);
        bool HandleNpcSpawnDistCommand(const char* args);
        bool HandleNpcSpawnTimeCommand(const char* args);
        bool HandleNpcStandState(const char* args);
        bool HandleNpcTameCommand(const char* args);
        bool HandleNpcTextEmoteCommand(const char* args);
        bool HandleNpcUnFollowCommand(const char* args);
        bool HandleNpcWhisperCommand(const char* args);
        bool HandleNpcYellCommand(const char* args);
        bool HandleNpcSetLinkCommand(const char* args);
        bool HandleNpcResetAICommand(const char* args);
        bool HandleNpcDoActionCommand(const char* args);
        bool HandleNpcEnterEvadeModeCommand(const char* args);

        bool HandleQuestAdd(const char * args);
        bool HandleQuestRemove(const char * args);
        bool HandleQuestComplete(const char * args);

        bool HandleReloadCommand(const char* args);
        bool HandleReloadAllCommand(const char* args);
        bool HandleReloadAllAreaCommand(const char* args);
        bool HandleReloadAllItemCommand(const char* args);
        bool HandleReloadAllLootCommand(const char* args);
        bool HandleReloadAllNpcCommand(const char* args);
        bool HandleReloadAllQuestCommand(const char* args);
        bool HandleReloadAllScriptsCommand(const char* args);
        bool HandleReloadAllSpellCommand(const char* args);
        bool HandleReloadAllLocalesCommand(const char* args);

        bool HandleReloadConfigCommand(const char* args);

        bool HandleReloadWpScriptsCommand(const char* args);
        bool HandleReloadRevisionCommand(const char* args);
        bool HandleReloadAreaTriggerTavernCommand(const char* args);
        bool HandleReloadAreaTriggerTeleportCommand(const char* args);
        bool HandleReloadAccessRequirementCommand(const char* args);
        bool HandleReloadAutobroadcastCommand(const char* args);
        bool HandleReloadEventScriptsCommand(const char* args);
        bool HandleReloadCommandCommand(const char* args);
        bool HandleReloadCreatureQuestRelationsCommand(const char* args);
        bool HandleReloadCreatureQuestInvRelationsCommand(const char* args);
        bool HandleReloadCreatureLinkedRespawnCommand(const char* args);
        bool HandleReloadUnqueuedAccountListCommand(const char* args);
        bool HandleReloadDbScriptStringCommand(const char* args);
        bool HandleReloadGameGraveyardZoneCommand(const char* args);
        bool HandleReloadGameObjectScriptsCommand(const char* args);
        bool HandleReloadGameTeleCommand(const char* args);
        bool HandleReloadGMTicketCommand(const char*);
        bool HandleReloadFakeBots(const char*);
        bool HandleReloadGOQuestRelationsCommand(const char* args);
        bool HandleReloadGOQuestInvRelationsCommand(const char* args);
        bool HandleReloadLootTemplatesCreatureCommand(const char* args);
        bool HandleReloadLootTemplatesDisenchantCommand(const char* args);
        bool HandleReloadLootTemplatesFishingCommand(const char* args);
        bool HandleReloadLootTemplatesGameobjectCommand(const char* args);
        bool HandleReloadLootTemplatesItemCommand(const char* args);
        bool HandleReloadLootTemplatesPickpocketingCommand(const char* args);
        bool HandleReloadLootTemplatesProspectingCommand(const char* args);
        bool HandleReloadLootTemplatesReferenceCommand(const char* args);
        bool HandleReloadLootTemplatesQuestMailCommand(const char* args);
        bool HandleReloadLootTemplatesSkinningCommand(const char* args);
        bool HandleReloadHellgroundStringCommand(const char* args);
        bool HandleReloadNpcGossipCommand(const char* args);
        bool HandleReloadNpcOptionCommand(const char* args);
        bool HandleReloadNpcTrainerCommand(const char* args);
        bool HandleReloadNpcVendorCommand(const char* args);
        bool HandleReloadQuestAreaTriggersCommand(const char* args);
        bool HandleReloadQuestEndScriptsCommand(const char* args);
        bool HandleReloadQuestStartScriptsCommand(const char* args);
        bool HandleReloadQuestTemplateCommand(const char* args);
        bool HandleReloadReservedNameCommand(const char*);
        bool HandleReloadReputationRewardRateCommand(const char* args);
        bool HandleReloadReputationSpilloverTemplateCommand(const char* args);
        bool HandleReloadShopCommand(const char* args);
        bool HandleReloadArenaRestrictionsCommand(const char* args);
        bool HandleReloadWardenCommand(const char* args);
        bool HandleReloadMinorityPropagandaChar(const char* args);
        bool HandleReloadSkillDiscoveryTemplateCommand(const char* args);
        bool HandleReloadSkillExtraItemPrototypeCommand(const char* args);
        bool HandleReloadSkillFishingBaseLevelCommand(const char* args);
        bool HandleReloadSpellAffectCommand(const char* args);
        bool HandleReloadSpellRequiredCommand(const char* args);
        bool HandleReloadSpellElixirCommand(const char* args);
        bool HandleReloadSpellLearnSpellCommand(const char* args);
        bool HandleReloadSpellLinkedSpellCommand(const char* args);
        bool HandleReloadSpellProcEventCommand(const char* args);
        bool HandleReloadSpellEnchantDataCommand(const char*);
        bool HandleReloadSpellScriptTargetCommand(const char* args);
        bool HandleReloadEventAIScriptsCommand(const char* args);
        bool HandleReloadSpellScriptsCommand(const char* args);
        bool HandleReloadSpellTargetPositionCommand(const char* args);
        bool HandleReloadSpellThreatsCommand(const char* args);
        bool HandleReloadSpellPetAurasCommand(const char* args);
        bool HandleReloadSpellAnalogsCommand(const char*);
        bool HandleReloadSpellDisabledCommand(const char* args);
        bool HandleReloadPageTextsCommand(const char* args);
        bool HandleReloadItemEnchantementsCommand(const char* args);
        bool HandleReloadLocalesCreatureCommand(const char* args);
        bool HandleReloadLocalesGameobjectCommand(const char* args);
        bool HandleReloadLocalesItemCommand(const char* args);
        bool HandleReloadLocalesNpcTextCommand(const char* args);
        bool HandleReloadLocalesPageTextCommand(const char* args);
        bool HandleReloadLocalesQuestCommand(const char* args);
        bool HandleReloadAuctionsCommand(const char* args);
        bool HandleReloadEventAICommand(const char* args);

        bool HandleResetAllCommand(const char * args);
        bool HandleResetHonorCommand(const char * args);
        bool HandleResetLevelCommand(const char * args);
        bool HandleResetSpellsCommand(const char * args);
        bool HandleResetStatsCommand(const char * args);
        bool HandleResetTalentsCommand(const char * args);

        bool HandleSendItemsCommand(const char* args);
        bool HandleSendMailCommand(const char* args);
        bool HandleSendMessageCommand(const char * args);
        bool HandleSendMoneyCommand(const char* args);

        bool HandleServerCorpsesCommand(const char* args);
        //bool HandleServerEventsCommand(const char* args);
        bool HandleServerExitCommand(const char* args);
        bool HandleServerIdleRestartCommand(const char* args);
        bool HandleServerIdleShutDownCommand(const char* args);
        bool HandleServerKickallCommand(const char* args);
        //bool HandleServerMotdCommand(const char* args);
        //bool HandleCharInfoCommand(const char* args);
        //bool HandleGuildPointsInfoCommand(const char* args);
        bool HandleCoinsInfoCommand(const char* args);
        bool HandleLosTestCommand(const char* args);
        bool HandleServerMuteCommand(const char* args);
        bool HandleServerRestartCommand(const char* args);
        bool HandleServerSetMotdCommand(const char* args);
        bool HandleServerSetDiffTimeCommand(const char* args);
        bool HandleServerShutDownCommand(const char* args);
        bool HandleServerRollShutDownCommand(const char* args);
        bool HandleServerShutDownCancelCommand(const char* args);
        //bool HandleServerPVPCommand(const char* args);
        bool HandleLookGMItemCommand(const char* args);
        bool HandleDeleteGMItemCommand(const char* args);

        bool HandleTeleCommand(const char * args);
        bool HandleTeleAddCommand(const char * args);
        bool HandleTeleDelCommand(const char * args);
        bool HandleTeleGroupCommand(const char* args);
        bool HandleTeleNameCommand(const char* args);

        bool HandleRemoveBanHistoryCommand(const char* args);
        bool HandleUnBanAccountCommand(const char* args);
        bool HandleUnBanCharacterCommand(const char* args);
        bool HandleUnBanIPCommand(const char* args);

        bool HandleWpAddCommand(const char* args);
        bool HandleWpEventCommand(const char* args);
        bool HandleWpLoadPathCommand(const char* args);
        bool HandleWpModifyCommand(const char* args);
        bool HandleWpReloadPath(const char *args);
        bool HandleWpShowCommand(const char* args);
        bool HandleWPToFileCommand(const char* args);
        bool HandleWpUnLoadPathCommand(const char* args);

        bool HandleHelpCommand(const char* args);
        bool HandleCommandsCommand(const char* args);
        //bool HandleAHBotOptionsCommand(const char* args);
        bool HandleStartCommand(const char* args);
        bool HandleDismountCommand(const char* args);
        bool HandleSaveCommand(const char* args);

        bool HandleNamegoCommand(const char* args);
        bool HandleGonameCommand(const char* args);
        bool HandleGroupgoCommand(const char* args);
        bool HandleRecallCommand(const char* args);
        bool HandleAnnounceCommand(const char* args);
        bool HandleAnnIdCommand(const char* args);
        bool HandleNameAnnounceCommand(const char* args);
        bool HandleNotifyCommand(const char* args);
        bool HandleGPSCommand(const char* args);
        bool HandlePosInfoCommand(const char* args);
        bool HandleInfo(const char* args);
        bool HandleTaxiCheatCommand(const char* args);
        bool HandleWhispersCommand(const char* args);
        bool HandleModifyDrunkCommand(const char* args);
        bool HandleHDevAnnounceCommand(const char* args);

        bool HandleLoadScriptsCommand(const char* args);

        //bool HandleAddFormationCommand(const char* args);
        bool HandleRelocateCreatureCommand(const char *args);

        bool HandleGUIDCommand(const char* args);
        bool HandleNameCommand(const char* args);
        bool HandleSubNameCommand(const char* args);
        bool HandleItemMoveCommand(const char* args);
        bool HandleDeMorphCommand(const char* args);
        bool HandleTrueDeMorphCommand(const char* args);
        bool HandleNpcAddItemCommand(const char* args);
        bool HandleNpcDelItemCommand(const char* args);
        bool HandleNpcChangeLevelCommand(const char* args);
        bool HandleNpcDebugAICommand(const char* args);
        bool HandleGoCreatureCommand(const char* args);
        bool HandleGoCreatureDirectCommand(const char* args);
        bool HandleGoCreatureInMapCommand(const char* args);
        bool HandleGoObjectCommand(const char* args);
        bool HandleGoTicketCommand(const char* args);
        bool HandleGoTriggerCommand(const char* args);
        bool HandleGoGraveyardCommand(const char* args);
        bool HandlePInfoCommand(const char* args);
        bool HandlePLimitCommand(const char* args);
        bool HandleMuteCommand(const char* args);
        bool HandleTrollmuteCommand(const char* args);
        bool HandleTrollmuteInfoCommand(const char* args);
        bool HandleUnmuteCommand(const char* args);
        bool HandleMuteInfoCommand(const char* args);
        bool HandleMovegensCommand(const char* args);
        bool HandleFreezeCommand(const char *args);
        bool HandleUnFreezeCommand(const char *args);
        bool HandleListFreezeCommand(const char* args);

        bool HandleCharacterDeleteCommand(const char* args);
        bool HandleGoXYCommand(const char* args);
        bool HandleGoXYZCommand(const char* args);
        bool HandleGoZoneXYCommand(const char* args);
        bool HandleGoGridCommand(const char* args);
        bool HandleAddWeaponCommand(const char* args);
        bool HandleNpcSetModifierCommand(const char* args);
        bool HandleAllowMovementCommand(const char* args);
        bool HandleGoCommand(const char* args);

        bool HandleCooldownCommand(const char* args);
        bool HandleUnLearnCommand(const char* args);
        bool HandleGetDistanceCommand(const char* args);
        bool HandleModifyStandStateCommand(const char* args);
        bool HandleDieCommand(const char* args);
        bool HandleDamageCommand(const char *args);
        bool HandleReviveCommand(const char* args);
        bool HandleReviveGroupCommand(const char* args);
        bool HandleModifyMorphCommand(const char* args);
        bool HandleTrueMorphCommand(const char* args);
        bool HandleSpellAffectCheckCommand(const char* args);
        bool HandleSpellKnownCheckCommand(const char *args);
        bool HandleSpellInfoCheckCommand(const char *args);
        bool HandleItemLevelCheckCommand(const char *args);
        bool HandleAuraCommand(const char* args);
        bool HandleUnAuraCommand(const char* args);
        bool HandleLinkGraveCommand(const char* args);
        bool HandleNearGraveCommand(const char* args);
        //bool HandleSpawnTransportCommand(const char* args);
        bool HandleExploreCheatCommand(const char* args);
        bool HandleHoverCommand(const char* args);
        bool HandleWaterwalkCommand(const char* args);
        bool HandleLevelUpCommand(const char* args);
        bool HandleShowAreaCommand(const char* args);
        bool HandleHideAreaCommand(const char* args);
        bool HandleAddItemCommand(const char* args);
        bool HandleAddItemSetCommand(const char* args);
        bool HandlePetTpCommand(const char* args);
        bool HandlePetUnlearnCommand(const char* args);
        bool HandlePetLearnCommand(const char* args);
        bool HandleCreatePetCommand(const char* args);

        bool HandleGroupLeaderCommand(const char* args);
        bool HandleGroupDisbandCommand(const char* args);
        bool HandleGroupRemoveCommand(const char* args);

        bool HandleBankCommand(const char* args);
        bool HandleChangeWeather(const char* args);
        bool HandleKickPlayerCommand(const char * args);

        // GM ticket command handlers
        bool HandleGMTicketListCommand(const char* args);
        bool HandleGMTicketListOnlineCommand(const char* args);
        bool HandleGMTicketListClosedCommand(const char* args);
        //bool HandleGMTicketGetByIdCommand(const char* args);
        bool HandleGMTicketGetByNameCommand(const char* args);
        bool HandleGMTicketCloseByIdCommand(const char* args);
        bool HandleGMTicketResponseCommand(const char* args);
        bool HandleGMTicketAssignToCommand(const char* args);
        bool HandleGMTicketSelfAssignCommand(const char* args);
        bool HandleGMTicketDenyCommand(const char* args);
        bool HandleGMTicketUnAssignCommand(const char* args);
        bool HandleGMTicketCommentCommand(const char* args);
        bool HandleGMTicketDeleteByIdCommand(const char* args);
        bool HandleGMTicketHistoryCommand(const char* args);

        bool HandleMaxSkillCommand(const char* args);
        bool HandleSetSkillCommand(const char* args);
        //bool HandlePasswordCommand(const char* args);
        //bool HandleLockAccountCommand(const char* args);
        bool HandleRespawnCommand(const char* args);

        bool HandleRenameCommand(const char * args);
		bool HandleRenamePet(const char * args);
        bool HandleNameBanCommand(const char * args);
        bool HandleComeToMeCommand(const char *args);
        bool HandleCombatStopCommand(const char *args);
        bool HandleCharDeleteCommand(const char *args);
        bool HandleFlushArenaPointsCommand(const char *args);
        bool HandlePlayAllCommand(const char* args);
        bool HandlePlaySelfCommand(const char* args);
        bool HandleRepairitemsCommand(const char* args);

        bool HandleNpcAddTempCommand(const char* args);

        //! Development Commands
        bool HandleSet32Bit(const char* args);
        bool HandleDebugHostileRefList(const char * args);
        bool HandlePossessCommand(const char* args);
        bool HandleUnPossessCommand(const char* args);
        bool HandleBindSightCommand(const char* args);
        bool HandleUnbindSightCommand(const char* args);
        bool HandleGetPoolObjectStatsCommand(const char* args);

        bool HandleBindFollowCommand(const char* args);
        bool HandleUnbindFollowCommand(const char* args);

        bool HandleChannelListCommand(const char* args);
        bool HandleChannelPassCommand(const char* args);
        bool HandleChannelKickCommand(const char* args);
        bool HandleChannelMassKickCommand(const char* args);

        bool HandleDiffCommand(const char* args);

        bool HandleSendSpellVisualCommand(const char* args);
        bool HandleDeletedCharactersCommand(const char* args);
        bool HandleCharacterRestoreCommand(const char* args);
        bool HandleFakePacketCommand(const char* args);

        //bool HandleTeamInfoCommand(const char* args); // To get info about 1v1 team of selected player

        //bool HandleRaidRulesCommand(const char* args);

        bool HandleGiftCommand(const char* args);
        bool HandleBonusCommand(const char* args);
        bool HandleCaptcha(const char* args);
        //bool HandleBetaParticipationGiftCommand(const char* args);
        bool HandleBoxCommand(const char* args);
        //bool HandlePvpBonusCommand(const char* args);

        // CUSTOM

        bool HandleShopCommand(const char* args);
        bool HandlePremiumCommand(const char* args);
        bool HandleXpRatesCommand(const char* args);
        bool HandleBgRegSummonCommand(const char* args);
        //bool HandleSpecializationCommand(const char* args);
        bool HandleBgEventCommand(const char* args);
        //bool HandleBgWinsCommand(const char* args);
        bool HandleDeleteTransmog(const char* args);
        bool HandleNicknameReservation(const char* args);
        //bool HandleBetaFree(const char* args);
        bool HandleThrowCode(const char * args);
        bool HandleActivateCode(const char * args);
        bool HandleTest(const char * args);
		bool HandleMW(const char * args);
        bool HandleBerserk(const char * args);
        bool HandleRaidtest(const char* args);
        bool HandleGuildMarker(const char* args);
        //bool HandleRaidChest(const char* args);
        //bool HandleSetClass(const char* args);
        bool HandleHide(const char* args);
        //bool HandleBGKick(const char* args);
		bool HandleGolem(const char* args);
		bool HandleSetMapMod(const char* args);
		bool HandleGetMapMod(const char* args);
		bool HandleRaidStats(const char* args);
		bool HandleEnchantLegweapon(const char* args);

        // END CUSTOM

        Player*   getSelectedPlayer();
        Creature* getSelectedCreature();
        Unit*     getSelectedUnit();
        char*     extractKeyFromLink(char* text, char const* linkType, char** something1 = NULL);
        char*     extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1 = NULL);
        uint32    extractSpellIdFromLink(char* text);
        GameTele const* extractGameTeleFromLink(char* text);
        bool GetPlayerGroupAndGUIDByName(const char* cname, Player* &plr, Group* &group, uint64 &guid, bool offline = false);

        GameObject* GetObjectGlobalyWithGuidOrNearWithDbGuid(uint32 lowguid,uint32 entry);

        // Utility methods for commands
        bool LookupPlayerSearchCommand(QueryResultAutoPtr result, int32 limit);
        bool HandleBanListHelper(QueryResultAutoPtr result);
        bool HandleBanHelper(BanMode mode,char const* args);
        bool HandleBanInfoHelper(uint32 accountid, char const* accountname, uint32 ban_type);
        bool HandleUnBanHelper(BanMode mode,char const* args);
        
        void SetSentErrorMessage(bool val){ sentErrorMessage = val;};

        // must be in protected, not in private, cause otherwise CLI handler can't access it
        WorldSession * m_session;                           // != NULL for chat command call and NULL for CLI command
    private:

        // common global flag
        static bool load_command_table;
        bool sentErrorMessage;
};

class CliHandler : public ChatHandler
{
    public:
        typedef void Print(char const*);
        explicit CliHandler(Print* zprint) : m_print(zprint) {}

        // overwrite functions
        const char *GetHellgroundString(int32 entry) const;
        bool isAvailable(ChatCommand const& cmd, bool) const;
        void SendSysMessage(const char *str);
        char const* GetName() const;
        bool needReportToTarget(Player* chr) const;

    private:
        Print* m_print;
};

class ServerInfoEvent : public BasicEvent
{
public:
    ServerInfoEvent(Player& owner) : BasicEvent(), _owner(owner) {}

    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
    {
        if (WorldSession* s = _owner.GetSession())
        {
            ChatHandler(&_owner).HandleServerInfoCommand("");
        }
        return true;
    }

private:
    Player& _owner;
};

class SendSysMessageEvent : public BasicEvent
{
    public:
        SendSysMessageEvent(Player& owner, uint32 msgId) : BasicEvent(), _owner(owner), _msgId(msgId) {}

        bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
        {
            if (WorldSession* s = _owner.GetSession())
            {
                ChatHandler(&_owner).SendSysMessage(s->GetHellgroundString(_msgId));
            }
            return true;
        }

    private:
        Player& _owner;
        uint32 _msgId;
};

class PSendSysMessageEvent : public BasicEvent
{
public:
    PSendSysMessageEvent(Player& owner, uint32 msgId, ...) : BasicEvent(), _owner(owner), _msgId(msgId)
    {
        if (WorldSession* s = _owner.GetSession())
        {
            va_list args;
            va_start(args, msgId);
            const char* format = s->GetHellgroundString(msgId);
            vsnprintf(_str, sizeof(_str), format, args);
            va_end(args);
        }
    }

    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
    {
        if (WorldSession* s = _owner.GetSession())
        {
            ChatHandler(&_owner).SendSysMessage(_str);
        }
        return true;
    }

private:
    Player& _owner;
    uint32 _msgId;
    char _str[1024];
};

char const *fmtstring(char const *format, ...);

#endif

