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
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateMask.h"
#include "Player.h"
#include "SkillDiscovery.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "UpdateData.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"
#include "Group.h"
#include "Guild.h"
#include "Pet.h"
#include "SpellAuras.h"
#include "Util.h"
#include "Transports.h"
#include "Weather.h"
#include "BattleGround.h"
#include "BattleGroundAV.h"
#include "BattleGroundMgr.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "ArenaTeam.h"
#include "Chat.h"
#include "Database/DatabaseImpl.h"
#include "Spell.h"
#include "ScriptMgr.h"
#include "SocialMgr.h"
#include "GameEvent.h"
#include "GridMap.h"
#include "WorldEventProcessor.h"
#include "AccountMgr.h"
#include "PlayerAI.h"
#include "GuildMgr.h"
#include "Shop.h"
#include "TicketMgr.h"
#include "PetitionsHandler.h"

#include <cmath>
#include <cctype>
#include <iomanip> // std::setfill, std::setw for Gladdy updates
#include <random>

#define PLAYER_SKILL_INDEX(x)       (PLAYER_SKILL_INFO_1_1 + ((x)*3))
#define PLAYER_SKILL_VALUE_INDEX(x) (PLAYER_SKILL_INDEX(x)+1)
#define PLAYER_SKILL_BONUS_INDEX(x) (PLAYER_SKILL_INDEX(x)+2)

#define SKILL_VALUE(x)         PAIR32_LOPART(x)
#define SKILL_MAX(x)           PAIR32_HIPART(x)
#define MAKE_SKILL_VALUE(v, m) MAKE_PAIR32(v,m)

#define SKILL_TEMP_BONUS(x)    int16(PAIR32_LOPART(x))
#define SKILL_PERM_BONUS(x)    int16(PAIR32_HIPART(x))
#define MAKE_SKILL_BONUS(t, p) MAKE_PAIR32(t,p)

enum CharacterFlags
{
    CHARACTER_FLAG_NONE                 = 0x00000000,
    CHARACTER_FLAG_UNK1                 = 0x00000001,
    CHARACTER_FLAG_UNK2                 = 0x00000002,
    CHARACTER_LOCKED_FOR_TRANSFER       = 0x00000004,
    CHARACTER_FLAG_UNK4                 = 0x00000008,
    CHARACTER_FLAG_UNK5                 = 0x00000010,
    CHARACTER_FLAG_UNK6                 = 0x00000020,
    CHARACTER_FLAG_UNK7                 = 0x00000040,
    CHARACTER_FLAG_UNK8                 = 0x00000080,
    CHARACTER_FLAG_UNK9                 = 0x00000100,
    CHARACTER_FLAG_UNK10                = 0x00000200,
    CHARACTER_FLAG_HIDE_HELM            = 0x00000400,
    CHARACTER_FLAG_HIDE_CLOAK           = 0x00000800,
    CHARACTER_FLAG_UNK13                = 0x00001000,
    CHARACTER_FLAG_GHOST                = 0x00002000,
    CHARACTER_FLAG_RENAME               = 0x00004000,
    CHARACTER_FLAG_UNK16                = 0x00008000,
    CHARACTER_FLAG_UNK17                = 0x00010000,
    CHARACTER_FLAG_UNK18                = 0x00020000,
    CHARACTER_FLAG_UNK19                = 0x00040000,
    CHARACTER_FLAG_UNK20                = 0x00080000,
    CHARACTER_FLAG_UNK21                = 0x00100000,
    CHARACTER_FLAG_UNK22                = 0x00200000,
    CHARACTER_FLAG_UNK23                = 0x00400000,
    CHARACTER_FLAG_UNK24                = 0x00800000,
    CHARACTER_FLAG_LOCKED_BY_BILLING    = 0x01000000,
    CHARACTER_FLAG_DECLINED             = 0x02000000,
    CHARACTER_FLAG_UNK27                = 0x04000000,
    CHARACTER_FLAG_UNK28                = 0x08000000,
    CHARACTER_FLAG_UNK29                = 0x10000000,
    CHARACTER_FLAG_UNK30                = 0x20000000,
    CHARACTER_FLAG_UNK31                = 0x40000000,
    CHARACTER_FLAG_UNK32                = 0x80000000
};

// corpse reclaim times
#define DEATH_EXPIRE_STEP (5*MINUTE)
#define MAX_DEATH_COUNT 3

static uint32 copseReclaimDelay[MAX_DEATH_COUNT] = { 30, 60, 120 };


//== PlayerTaxi ================================================

PlayerTaxi::PlayerTaxi()
{
    // Taxi nodes
    memset(m_taximask, 0, sizeof(m_taximask));
}

void PlayerTaxi::InitTaxiNodesForLevel(uint32 race, uint32 level)
{
    // capital and taxi hub masks
    switch (race)
    {
        case RACE_HUMAN:    SetTaximaskNode(2);  break;     // Human
        case RACE_ORC:      SetTaximaskNode(23); break;     // Orc
        case RACE_DWARF:    SetTaximaskNode(6);  break;     // Dwarf
        case RACE_NIGHTELF: SetTaximaskNode(26);
                            SetTaximaskNode(27); break;     // Night Elf
        case RACE_UNDEAD_PLAYER: SetTaximaskNode(11); break;// Undead
        case RACE_TAUREN:   SetTaximaskNode(22); break;     // Tauren
        case RACE_GNOME:    SetTaximaskNode(6);  break;     // Gnome
        case RACE_TROLL:    SetTaximaskNode(23); break;     // Troll
        case RACE_BLOODELF: SetTaximaskNode(82); break;     // Blood Elf
        case RACE_DRAENEI:  SetTaximaskNode(94); break;     // Draenei
    }
    // new continent starting masks (It will be accessible only at new map)
    switch (Player::TeamForRace(race))
    {
        case ALLIANCE: SetTaximaskNode(100); break;
        case HORDE:    SetTaximaskNode(99);  break;
    }
    // level dependent taxi hubs
    if (level>=68)
        SetTaximaskNode(213);                               //Shattered Sun Staging Area
}

void PlayerTaxi::LoadTaxiMask(const char* data)
{
    Tokens tokens = StrSplit(data, " ");

    int index;
    Tokens::iterator iter;
    for (iter = tokens.begin(), index = 0;
        (index < TaxiMaskSize) && (iter != tokens.end()); ++iter, ++index)
    {
        // load and set bits only for existed taxi nodes
        m_taximask[index] = sTaxiNodesMask[index] & uint32(atol((*iter).c_str()));
    }
}

void PlayerTaxi::AppendTaximaskTo(ByteBuffer& data, bool all)
{
    if (all)
    {
        for (uint8 i=0; i<TaxiMaskSize; i++)
            data << uint32(sTaxiNodesMask[i]);              // all existed nodes
    }
    else
    {
        for (uint8 i=0; i<TaxiMaskSize; i++)
            data << uint32(m_taximask[i]);                  // known nodes
    }
}

bool PlayerTaxi::LoadTaxiDestinationsFromString(const std::string& values, uint32 plrMapId)
{
    ClearTaxiDestinations();

    Tokens tokens = StrSplit(values," ");

    for (Tokens::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
    {
        uint32 node = uint32(atol(iter->c_str()));
        AddTaxiDestination(node);
    }

    if (m_TaxiDestinations.empty())
        return true;

    // Check integrity
    if (m_TaxiDestinations.size() < 2)
        return false;

    // on player save if you have already reached one point it gets removed,
    // so every time you log in it checks current node and next node for map adequacy
    // (there are some nodes that change map, like from vanilla to BC)
    TaxiNodesEntry const* nodeCurr = sTaxiNodesStore.LookupEntry(m_TaxiDestinations.front());
    TaxiNodesEntry const* nodeNext = sTaxiNodesStore.LookupEntry(*(++m_TaxiDestinations.begin()));
    if (!nodeCurr || !nodeNext || (nodeCurr->map_id != plrMapId && nodeNext->map_id != plrMapId))
        return false;

    for (size_t i = 1; i < m_TaxiDestinations.size(); ++i)
    {
        uint32 cost;
        uint32 path;
        sObjectMgr.GetTaxiPath(m_TaxiDestinations[i-1],m_TaxiDestinations[i],path,cost);
        if (!path)
            return false;
    }

    return true;
}

std::string PlayerTaxi::SaveTaxiDestinationsToString()
{
    if (m_TaxiDestinations.empty())
        return "";

    std::ostringstream ss;

    for (size_t i=0; i < m_TaxiDestinations.size(); ++i)
        ss << m_TaxiDestinations[i] << " ";

    return ss.str();
}

uint32 PlayerTaxi::GetCurrentTaxiPath() const
{
    if (m_TaxiDestinations.size() < 2)
        return 0;

    uint32 path;
    uint32 cost;

    sObjectMgr.GetTaxiPath(m_TaxiDestinations[0],m_TaxiDestinations[1],path,cost);

    return path;
}

std::ostringstream& operator<< (std::ostringstream& ss, PlayerTaxi const& taxi)
{
    ss << "'";
    for(int i = 0; i < TaxiMaskSize; ++i)
        ss << taxi.m_taximask[i] << " ";
    ss << "'";
    return ss;
}

//== Player ====================================================

UpdateMask Player::updateVisualBits;

Player::Player (WorldSession *session): Unit(), m_reputationMgr(this), m_camera(this)
{
    m_transport = 0;

    m_mover = this;

    m_speakTime = 0;
    m_speakCount = 0;

    m_GMfollowtarget_GUID = 0;
    m_GMfollow_GUID = 0;

    m_objectType |= TYPEMASK_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;

    m_valuesCount = PLAYER_END;

    m_session = session;

    m_divider = 0;

    m_ExtraFlags = 0;

    // players always accept
    if (!GetSession()->HasPermissions(PERM_GMT_HDEV))
        SetAcceptWhispers(true, true);

    m_curSelection = 0;
    m_lootGuid = 0;

    m_comboTarget = 0;
    m_comboPoints = 0;

    m_usedTalentCount = 0;

    m_zoneUpdateId = 0;
    m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL; // will be overwritten in UpdateZone() on load

    m_areaUpdateId = 0;

    m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

    // randomize first save time in range [CONFIG_INTERVAL_SAVE] around [CONFIG_INTERVAL_SAVE]
    // this must help in case next save after mass player load after server startup
    m_nextSave = urand(m_nextSave/2,m_nextSave*3/2);

    clearResurrectRequestData();

    m_SpellModRemoveCount = 0;

    memset(m_items, 0, sizeof(Item*)*PLAYER_SLOTS_COUNT);

    m_social = NULL;

    // group is initialized in the reference constructor
    SetGroupInvite(NULL);
    m_groupUpdateMask = 0;
    m_auraUpdateMask = 0;
    m_bPassOnGroupLoot = false;

    duel = NULL;

    m_GuildIdInvited = 0;
    m_ArenaTeamIdInvited = 0;

    m_atLoginFlags = AT_LOGIN_NONE;

    m_teleport_options = 0;

    pTrader = 0;

    ClearTrade();

    m_cinematic = false;
    m_watchingCinematicId = 0;

    PlayerTalkClass = new PlayerMenu(GetSession());
    m_currentBuybackSlot = BUYBACK_SLOT_START;

    for (int aX = 0 ; aX < 8 ; aX++)
        m_Tutorials[ aX ] = 0x00;

    m_TutorialsChanged = false;

    m_DailyQuestChanged = false;
    m_lastDailyQuestTime = 0;
    m_DailyArenasWon = 0;

    for (uint8 i=0; i< MAX_TIMERS; i++)
        m_MirrorTimer[i] = DISABLED_MIRROR_TIMER;

    m_MirrorTimerFlags = UNDERWATER_NONE;
    m_MirrorTimerFlagsLast = UNDERWATER_NONE;

    m_regenTimer = 0;
    m_isSwimming = false;
    m_drunkTimer = 0;
    m_drunk = 0;
    m_restTime = 0;
    m_deathTimer = 0;
    m_deathExpireTime = 0;

    m_swingErrorMsg = 0;

    // INVISIBILITY_DETECT_TIMER STEALTH_DETECT_TIMER
    m_DetectInvTimer = 1000;

    m_bgBattleGroundID = 0;
    m_bgTypeID = BATTLEGROUND_TYPE_NONE;
    for (int j=0; j < PLAYER_MAX_BATTLEGROUND_QUEUES; j++)
    {
        m_bgBattleGroundQueueID[j].bgQueueTypeId  = BATTLEGROUND_QUEUE_NONE;
        m_bgBattleGroundQueueID[j].invitedToInstance = 0;
    }
    m_bgTeam = TEAM_NONE;

    m_logintime = time(NULL);
    m_Last_tick = m_logintime;
    m_WeaponProficiency = 0;
    m_ArmorProficiency = 0;
    m_canParry = false;
    m_canBlock = false;
    m_canDualWield = false;
    m_ammoDPS = 0.0f;

    m_temporaryUnsummonedPetNumber = 0;
    //cache for UNIT_CREATED_BY_SPELL to allow
    //returning reagents for temporarily removed pets
    //when dying/logging out
    m_oldpetspell = 0;
    m_lastpetnumber = 0;
    m_refreshPetSpells = false;

    ////////////////////Rest System/////////////////////
    time_inn_enter=0;
    inn_pos_mapid=0;
    inn_pos_x=0;
    inn_pos_y=0;
    inn_pos_z=0;
    m_rest_bonus=0;
    rest_type=REST_TYPE_NO;
    ////////////////////Rest System/////////////////////

    // movement anticheat
    m_anti_lastmovetime = 0;          //last movement time
    m_anti_transportGUID = 0;         //current transport GUID
    m_anti_last_hspeed = 7.0f;        //horizontal speed, default RUN speed
    m_anti_lastspeed_changetime = 0;  //last speed change time
    m_anti_justteleported_timer = 0;  //seted when player was teleported
    m_anti_justteleported_distance = 0.0f;  //seted when player was teleported
    m_anti_ontaxipath = false;        // seted when player is on a taxi fight
    m_anti_isknockedback = false;     // seted when player is knocked back
    m_anti_ping_time = 0;               // needed to detect cheat engine
    
    m_anti_alarmcount = 0;            // alarm counter
    m_anti_airbrakes_alarmcount = 0;  // air-ctrl brakes alarmcount
    m_anti_alarmcount_skip_timer = 3000;
    m_anti_jump_start_z = 0;

    /////////////////////////////////

    m_mailsUpdated = false;
    unReadMails = 0;
    m_nextMailDelivereTime = 0;

    m_resetTalentsCost = 0;
    m_resetTalentsTime = 0;
    m_itemUpdateQueueBlocked = false;

    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_forced_speed_changes[i] = 0;

    m_stableSlots = 0;

    /////////////////// Instance System /////////////////////

    m_HomebindTimer = 0;
    m_InstanceValid = true;
    m_dungeonDifficulty = DIFFICULTY_NORMAL;

    for (int i = 0; i < BASEMOD_END; i++)
    {
        m_auraBaseMod[i][FLAT_MOD] = 0.0f;
        m_auraBaseMod[i][PCT_MOD] = 1.0f;
    }

    m_spellPenetrationItemMod = 0;

    // Honor System
    m_lastHonorUpdateTime = time(NULL);

    // Player summoning
    m_summon_expire = 0;
    m_summon_mapid = 0;
    m_summon_x = 0.0f;
    m_summon_y = 0.0f;
    m_summon_z = 0.0f;

    m_miniPet = 0;

    m_bgAfkReportedCount = 0;
    m_bgAfkReportedTimer = 0;
    m_contestedPvPTimer = 0;

    m_declinedname = NULL;

    m_farsightVision = false;

    m_activeBy = 0;

    _preventSave = false;
    _preventUpdate = false;

    positionStatus.Reset(0);

    m_GrantableLevelsCount = 0;
    m_ChCustomFlags = 0;
    m_outdoors = GetTerrain()->IsOutdoors(GetPositionX(), GetPositionY(), GetPositionZ());

    StartTeleportTimer();

    m_changeRaceTo = 0;

    m_transmogManager = new Transmogrification(this);

    m_arena_restricted_swaps = NULL;
    m_arena_restricted_swaps_size = 0;

    m_lastInGameSaveTime = time(NULL);
    m_restStateTimer = 10000;

    m_activeSpec = 0;

    personalCraftWarningLastEntry = 0;

    raid_chest_info.Class = CLASS_NONE;
    raid_chest_info.specMask = SPEC_MASK_NONE;
    raid_chest_info.leg_weapon = 0;
    raid_chest_info.only_pve = true;

    selectedCharacterGuid = 0; // my another character who to send items
	captcha_lastused = 0;
	creature_last_killed = 0;

    ingame_time = 0;

    bg_premade_leader_guid = 0;

    CharacterCustomData ccd;
    ccd.souls_quests_done = 0;
    custom_data = ccd;
}

Player::~Player()
{
    sLog.outLog(LOG_TMP, "~Player() called for %s (guid %u)", GetName(), GetGUIDLow());
    
    CleanupsBeforeDelete();

    // it must be unloaded already in PlayerLogout and accessed only for loggined player
    //m_social = NULL;

    // Note: buy back item already deleted from DB when player was saved
    for (int i = 0; i < PLAYER_SLOTS_COUNT; ++i)
    {
        if (m_items[i])
            delete m_items[i];
    }
    CleanupChannels();

    for (uint8 i = 0; i < MAX_TALENT_SPECS; ++i)
    {
        for (PlayerTalentMap::const_iterator itr = m_talents[i].begin(); itr != m_talents[i].end(); ++itr)
            delete itr->second;
    }

    //all mailed items should be deleted, also all mail should be deallocated
    for (PlayerMails::iterator itr =  m_mail.begin(); itr != m_mail.end();++itr)
        delete *itr;

    for (ItemMap::iterator iter = mMitems.begin(); iter != mMitems.end(); ++iter)
        delete iter->second;                                //if item is duplicated... then server may crash ... but that item should be deallocated

    delete PlayerTalkClass;

    if (m_transport)
    {
        m_transport->RemovePassenger(this);
    }

    for (size_t x = 0; x < ItemSetEff.size(); x++)
        if (ItemSetEff[x])
            delete ItemSetEff[x];

    // clean up player-instance binds, may unload some instance saves
    for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
    {
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
        {
            InstanceSave* save = itr->second.save;
            if (save != nullptr && save->HasPlayer(GetGUID()))
                save->RemovePlayer(GetGUID());
        }
    }

    delete m_declinedname;

    DeleteCharmAI();

    delete m_transmogManager;

    if (m_arena_restricted_swaps)
        delete[] m_arena_restricted_swaps;
}

void Player::CleanupsBeforeDelete()
{
    if (m_uint32Values)                                      // only for fully created Object
    {
        TradeCancel(false);
        DuelComplete(DUEL_INTERUPTED);

        if (getFollowingGM())
        {
            Player *gamemaster = Unit::GetPlayerInWorld(getFollowingGM());
            if (gamemaster)
            {
                gamemaster->setFollowTarget(0);
                gamemaster->GetUnitStateMgr().InitDefaults(true);
            }
            setGMFollow(0);
        }

        // just to be sure that we are removed from all outdoorpvp before we are deleted
        sOutdoorPvPMgr.HandlePlayerLeave(this);
    }

    ClearLFG();
    ClearLFM();

    Unit::CleanupsBeforeDelete();
}

bool Player::Create(uint32 guidlow, const std::string& name, uint8 race, uint8 class_, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair, uint8 outfitId)
{
    //FIXME: outfitId not used in player creating

    Object::_Create(guidlow, 0, HIGHGUID_PLAYER);

    m_name = name;
	m_guid = guidlow;

    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(race, class_);
    if (!info)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player have incorrect race/class pair. Can't be loaded.");
        return false;
    }

    for (int i = 0; i < PLAYER_SLOTS_COUNT; i++)
        m_items[i] = NULL;

    SetMapId(info->mapId);
    Relocate(info->positionX,info->positionY,info->positionZ);

    SetMap(sMapMgr.CreateMap(info->mapId, this));

    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(class_);
    if (!cEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Class %u not found in DBC (Wrong DBC files?)",class_);
        return false;
    }

    uint8 powertype = cEntry->powerType;

    uint32 unitfield;

    switch (powertype)
    {
        case POWER_ENERGY:
        case POWER_MANA:
            unitfield = 0x00000000;
            break;
        case POWER_RAGE:
            unitfield = 0x00110000;
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: Invalid default powertype %u for player (class %u)",powertype,class_);
            return false;
    }

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_WORLD_OBJECT_SIZE);
    SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_COMBAT_REACH);

    switch (gender)
    {
        case GENDER_FEMALE:
            SetDisplayId(info->displayId_f);
            SetNativeDisplayId(info->displayId_f);
            break;
        case GENDER_MALE:
            SetDisplayId(info->displayId_m);
            SetNativeDisplayId(info->displayId_m);
            break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: Invalid gender %u for player",gender);
            return false;
            break;
    }

    m_team = TeamForRace(race);
    setFactionForRace(race);

    uint32 RaceClassGender = (race) | (class_ << 8) | (gender << 16);

    SetUInt32Value(UNIT_FIELD_BYTES_0, (RaceClassGender | (powertype << 24)));
    SetUInt32Value(UNIT_FIELD_BYTES_1, unitfield);
    SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SANCTUARY | UNIT_BYTE2_FLAG_UNK5);
    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);               // fix cast time showed in spell tooltip on client

                                                            //-1 is default value
    SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));

    //LoadAccountLinkedState();

    uint32 pBytes = (skin | (face << 8) | (hairStyle << 16) | (hairColor << 24));
    uint32 pBytes2 = (facialHair | (0x00 << 8) | (0x00 << 16) | (REST_STATE_NORMAL << 24));

    AppearanceCheckAndFixIfNeeded(pBytes, pBytes2);

    SetUInt32Value(PLAYER_BYTES, pBytes);
    SetUInt32Value(PLAYER_BYTES_2, pBytes2);

    SetByteValue(PLAYER_BYTES_3, 0, gender);

    SetUInt32Value(PLAYER_GUILDID, 0);
    SetUInt32Value(PLAYER_GUILDRANK, 0);
    SetUInt32Value(PLAYER_GUILD_TIMESTAMP, 0);

    SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, 0);        // 0=disabled
    SetUInt32Value(PLAYER_CHOSEN_TITLE, 0);
    SetUInt32Value(PLAYER_FIELD_KILLS, 0);
    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);
    SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
    SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);

    // set starting level
    if (GetSession()->HasPermissions(PERM_GMT_HDEV))
        SetUInt32Value (UNIT_FIELD_LEVEL, sWorld.getConfig(CONFIG_START_GM_LEVEL));
    else
        SetUInt32Value (UNIT_FIELD_LEVEL, sWorld.getConfig(CONFIG_START_PLAYER_LEVEL));

    if (sWorld.getConfig(CONFIG_ALWAYS_MAX_WEAPON_SKILL))
        SetWeaponSkillsToMax();

    SetUInt32Value (PLAYER_FIELD_COINAGE, sWorld.getConfig(CONFIG_START_PLAYER_MONEY));
    SetUInt32Value (PLAYER_FIELD_HONOR_CURRENCY, sWorld.getConfig(CONFIG_START_HONOR_POINTS));
    SetUInt32Value (PLAYER_FIELD_ARENA_CURRENCY, sWorld.getConfig(CONFIG_START_ARENA_POINTS));

    // start with every map explored
    if (sWorld.getConfig(CONFIG_START_ALL_EXPLORED))
    {
        for (uint8 i=0; i<64; i++)
            SetFlag(PLAYER_EXPLORED_ZONES_1+i,0xFFFFFFFF);
    }

    //Reputations if "StartAllReputation" is enabled, -- TODO: Fix this in a better way
    if (sWorld.getConfig(CONFIG_START_ALL_REP))
    {
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(942), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(935), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(936), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(1011), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(970), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(967), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(989), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(932), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(934), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(1038), 42999);
        m_reputationMgr.SetReputation(sFactionStore.LookupEntry(1077), 42999);

        // Factions depending on team, like cities and some more stuff
        switch (GetTeam())
        {
            case ALLIANCE:
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(72), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(47), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(69), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(930), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(730), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(978), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(54), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(946), 42999);
                break;
            case HORDE:
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(76), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(68), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(81), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(911), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(729), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(941), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(530), 42999);
                m_reputationMgr.SetReputation(sFactionStore.LookupEntry(947), 42999);
                break;
            default:
                break;
        }
    }

    // Played time
    m_Last_tick = time(NULL);
    m_Played_time[0] = 0;
    m_Played_time[1] = 0;

    // base stats and related field values
    InitStatsForLevel();
    InitTaxiNodesForLevel();
    InitTalentForLevel();
    InitPrimaryProffesions();                               // to max set before any spell added

    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()
    UpdateMaxHealth();                                      // Update max Health (for add bonus from stamina)
    SetHealth(GetMaxHealth());
    if (getPowerType()==POWER_MANA)
    {
        UpdateMaxPower(POWER_MANA);                         // Update max Mana (for add bonus from intellect)
        SetPower(POWER_MANA,GetMaxPower(POWER_MANA));
    }

    // original spells
    learnDefaultSpells(true);

    // original action bar
    std::list<uint16>::const_iterator action_itr[4];
    for (int i=0; i<4; i++)
        action_itr[i] = info->action[i].begin();

    for (; action_itr[0]!=info->action[0].end() && action_itr[1]!=info->action[1].end();)
    {
        uint16 taction[4];
        for (int i=0; i<4 ;i++)
            taction[i] = (*action_itr[i]);

        addActionButton((uint8)taction[0], taction[1], (uint8)taction[2], (uint8)taction[3]);

        for (int i=0; i<4 ;i++)
            ++action_itr[i];
    }

    // original items
    CharStartOutfitEntry const* oEntry = NULL;
    for (uint32 i = 1; i < sCharStartOutfitStore.GetNumRows(); ++i)
    {
        if (CharStartOutfitEntry const* entry = sCharStartOutfitStore.LookupEntry(i))
        {
            if (entry->RaceClassGender == RaceClassGender)
            {
                oEntry = entry;
                break;
            }
        }
    }

    if (oEntry)
    {
        for (int j = 0; j < MAX_OUTFIT_ITEMS; ++j)
        {
            if (oEntry->ItemId[j] <= 0)
                continue;

            uint32 item_id = oEntry->ItemId[j];

            ItemPrototype const* iProto = ObjectMgr::GetItemPrototype(item_id);
            if (!iProto)
            {
                sLog.outLog(LOG_DB_ERR, "Initial item id %u (race %u class %u) from CharStartOutfit.dbc not listed in `item_template`, ignoring.",item_id,GetRace(),GetClass());
                continue;
            }

            uint32 count = iProto->Stackable;               // max stack by default (mostly 1)
            if (iProto->Class==ITEM_CLASS_CONSUMABLE && iProto->SubClass==ITEM_SUBCLASS_FOOD)
            {
                switch (iProto->Spells[0].SpellCategory)
                {
                    case SPELL_CATEGORY_FOOD:                                // food
                        if (iProto->Stackable > 4)
                            count = 4;
                        break;
                    case SPELL_CATEGORY_DRINK:                                // drink
                        if (iProto->Stackable > 2)
                            count = 2;
                        break;
                }
            }

            StoreNewItemInBestSlots(item_id, count);
        }
    }

    for (PlayerCreateInfoItems::const_iterator item_id_itr = info->item.begin(); item_id_itr!=info->item.end(); ++item_id_itr++) // DAFUQ IS THAT???
        StoreNewItemInBestSlots(item_id_itr->item_id, item_id_itr->item_amount);

    // bags and main-hand weapon must equipped at this moment
    // now second pass for not equipped (offhand weapon/shield if it attempt equipped before main-hand weapon)
    // or ammo not equipped in special bag
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            uint16 eDest;
            // equip offhand weapon/shield if it attempt equipped before main-hand weapon
            uint8 msg = CanEquipItem(NULL_SLOT, eDest, pItem, false);
            if (msg == EQUIP_ERR_OK)
            {
                RemoveItem(INVENTORY_SLOT_BAG_0, i,true);
                EquipItem(eDest, pItem, true);
            }
            // move other items to more appropriate slots (ammo not equipped in special bag)
            else
            {
                ItemPosCountVec sDest;
                msg = CanStoreItem(NULL_BAG, NULL_SLOT, sDest, pItem, false);
                if (msg == EQUIP_ERR_OK)
                {
                    RemoveItem(INVENTORY_SLOT_BAG_0, i,true);
                    pItem = StoreItem(sDest, pItem, true);
                }

                // if this is ammo then use it
                msg = CanUseAmmo(pItem->GetEntry());
                if (msg == EQUIP_ERR_OK)
                    SetAmmo(pItem->GetEntry());
            }
        }
    }
    // all item positions resolved

    return true;
}

uint8 Player::GetAppearanceMaxIdx(uint8 byteIdx)
{
    if (byteIdx > 3)
        return 0;

    uint8 race = GetRace();
    uint8 gender = GetGender();
    
    if (HasAura(55401))
    {
        std::pair<uint8, uint8> morphval = GetMorphShirtRaceGender(this);
        race = morphval.first;
        gender = morphval.second;
    }

    if (gender > GENDER_FEMALE)
        return 0;

    return PlayerAppearanceMax_Bytes[race][byteIdx + 4 * gender];
}

uint8 Player::GetAppearanceMaxIdx_2()
{
    uint8 race = GetRace();
    uint8 gender = GetGender();

    if (HasAura(55401))
    {
        std::pair<uint8, uint8> morphval = GetMorphShirtRaceGender(this);
        race = morphval.first;
        gender = morphval.second;
    }
    
    if (gender > GENDER_FEMALE)
        return 0;

    return PlayerAppearanceMax_Bytes_2[race][gender];
}

void Player::AppearanceCheckAndFixIfNeeded(uint32 &player_bytes, uint32 &player_bytes_2)
{
    for (uint8 i = 0; i < 4; ++i)
    {
        uint8 allowedMax = GetAppearanceMaxIdx(i);
        uint8 has = uint8(player_bytes >> (i * 8));
        if (has > allowedMax)
            player_bytes &= 0xFFFFFFFF - (0xFF << (i * 8)); // reset byte value to 0
    }

    uint8 allowedMaxFacial = GetAppearanceMaxIdx_2();
    if (uint8(player_bytes_2) > allowedMaxFacial)
        player_bytes_2 &= 0xFFFFFF00; // reset byte value to 0
}

void Player::ForceDisplayUpdate()
{
    // First Send update to player, so most recent datas are up
    SendCreateUpdateToPlayer(this);

    // Force client to reload this player, so changes are visible
    WorldPacket data(SMSG_FORCE_DISPLAY_UPDATE, 8);
    data << GetGUID();
    BroadcastPacket(&data, true);
}

bool Player::StoreNewItemInBestSlots(uint32 titem_id, uint32 titem_amount)
{
    sLog.outDebug("STORAGE: Creating initial item, itemId = %u, count = %u",titem_id, titem_amount);

    // attempt equip by one
    while (titem_amount > 0)
    {
        uint16 eDest;
        uint8 msg = CanEquipNewItem(NULL_SLOT, eDest, titem_id, false);
        if (msg != EQUIP_ERR_OK)
            break;

        EquipNewItem(eDest, titem_id, true);
        AutoUnequipOffhandIfNeed();
        --titem_amount;
    }

    if (titem_amount == 0)
        return true;                                        // equipped

    // attempt store
    ItemPosCountVec sDest;
    // store in main bag to simplify second pass (special bags can be not equipped yet at this moment)
    uint8 msg = CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, sDest, titem_id, titem_amount);
    if (msg == EQUIP_ERR_OK)
    {
        StoreNewItem(sDest, titem_id, true, Item::GenerateItemRandomPropertyId(titem_id), "CHARACTER_CREATE");
        return true;                                        // stored
    }

    // item can't be added
    sLog.outLog(LOG_DEFAULT, "ERROR: STORAGE: Can't equip or store initial item %u for race %u class %u , error msg = %u",titem_id,GetRace(),GetClass(),msg);
    return false;
}

void Player::SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen)
{
    if (int(MaxValue) == DISABLED_MIRROR_TIMER)
    {
        if (int(CurrentValue) != DISABLED_MIRROR_TIMER)
            StopMirrorTimer(Type);
        return;
    }
    WorldPacket data(SMSG_START_MIRROR_TIMER, (21));
    data << (uint32)Type;
    data << CurrentValue;
    data << MaxValue;
    data << Regen;
    data << (uint8)0;
    data << (uint32)0;                                      // spell id
    SendPacketToSelf(&data);
}

void Player::StopMirrorTimer(MirrorTimerType Type)
{
    m_MirrorTimer[Type] = DISABLED_MIRROR_TIMER;
    WorldPacket data(SMSG_STOP_MIRROR_TIMER, 4);
    data << (uint32)Type;
    SendPacketToSelf(&data);
}

void Player::UpdateFallInformationIfNeed(MovementInfo const& minfo,uint16 opcode)
{
    if (m_lastFallTime >= minfo.GetFallTime() || m_lastFallZ <= minfo.GetPos()->z || opcode == MSG_MOVE_FALL_LAND)
        SetFallInformation(minfo.GetFallTime(), minfo.GetPos()->z);
}

void Player::EnvironmentalDamage(EnviromentalDamage type, uint32 damage)
{
    if (!isAlive() || isGameMaster())
        return;

    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, (21));
    data << uint64(GetGUID());
    data << uint8(type != DAMAGE_FALL_TO_VOID ? type : DAMAGE_FALL);
    data << uint32(damage);
    data << uint32(0);
    data << uint32(0);
    BroadcastPacket(&data, true);

    DealDamage(this, damage, SELF_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

    if (sWorld.getConfig(CONFIG_DURABILITY_LOSS_ON_DEATH))
    {
        if (type == DAMAGE_FALL && !isAlive() && !InBattleGroundOrArena())                     // DealDamage not apply item durability loss at self damage
        {
            debug_log("We are fall to death, loosing 10 percents durability");
            DurabilityLossAll(0.10f, false);
            // durability lost message
            WorldPacket data2(SMSG_DURABILITY_DAMAGE_DEATH, 0);
            SendPacketToSelf(&data2);
        }
    }
}

int32 Player::getMaxTimer(MirrorTimerType timer)
{
    switch (timer)
    {
        case FATIGUE_TIMER:
            return MINUTE*MILLISECONDS;
        case BREATH_TIMER:
        {
            if (!isAlive() || HasAuraType(SPELL_AURA_WATER_BREATHING) || GetSession()->HasPermissions(sWorld.getConfig(CONFIG_DISABLE_BREATHING)))
                return DISABLED_MIRROR_TIMER;

            int32 UnderWaterTime = MINUTE*MILLISECONDS;
            AuraList const& mModWaterBreathing = GetAurasByType(SPELL_AURA_MOD_WATER_BREATHING);
            for (AuraList::const_iterator i = mModWaterBreathing.begin(); i != mModWaterBreathing.end(); ++i)
                UnderWaterTime = uint32(UnderWaterTime * (100.0f + (*i)->GetModifierValue()) / 100.0f);

            return UnderWaterTime;
        }
        case FIRE_TIMER:
        {
            if (!isAlive())
                return DISABLED_MIRROR_TIMER;

            return MILLISECONDS;
        }
        default:
            return 0;
    }
}

void Player::UpdateMirrorTimers()
{
    // Desync flags for update on next HandleDrowning
    if (m_MirrorTimerFlags)
        m_MirrorTimerFlagsLast = ~m_MirrorTimerFlags;
}

void Player::HandleDrowning(uint32 time_diff)
{
    if (!m_MirrorTimerFlags)
        return;

    // In water
    if (m_MirrorTimerFlags & UNDERWATER_INWATER)
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[BREATH_TIMER] == DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimer[BREATH_TIMER] = getMaxTimer(BREATH_TIMER);
            SendMirrorTimer(BREATH_TIMER, m_MirrorTimer[BREATH_TIMER], m_MirrorTimer[BREATH_TIMER], -1);
        }
        else                                                              // If activated - do tick
        {
            m_MirrorTimer[BREATH_TIMER]-=time_diff;
            // Timer limit - need deal damage
            if (m_MirrorTimer[BREATH_TIMER] < 0)
            {
                m_MirrorTimer[BREATH_TIMER]+= 1*MILLISECONDS;
                // Calculate and deal damage
                // TODO: Check this formula
                uint32 damage = GetMaxHealth() / 5 + urand(0, GetLevel()-1);
                EnvironmentalDamage(DAMAGE_DROWNING, damage);
            }
            else if (!(m_MirrorTimerFlagsLast & UNDERWATER_INWATER))      // Update time in client if need
                SendMirrorTimer(BREATH_TIMER, getMaxTimer(BREATH_TIMER), m_MirrorTimer[BREATH_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[BREATH_TIMER] != DISABLED_MIRROR_TIMER)        // Regen timer
    {
        int32 UnderWaterTime = getMaxTimer(BREATH_TIMER);
        // Need breath regen
        m_MirrorTimer[BREATH_TIMER]+=10*time_diff;
        if (m_MirrorTimer[BREATH_TIMER] >= UnderWaterTime || !isAlive())
            StopMirrorTimer(BREATH_TIMER);
        else if (m_MirrorTimerFlagsLast & UNDERWATER_INWATER)
            SendMirrorTimer(BREATH_TIMER, UnderWaterTime, m_MirrorTimer[BREATH_TIMER], 10);
    }

    // In dark water
    if (m_MirrorTimerFlags & UNDERWATER_INDARKWATER)
    {
        // Fatigue timer not activated - activate it
        if (m_MirrorTimer[FATIGUE_TIMER] == DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimer[FATIGUE_TIMER] = getMaxTimer(FATIGUE_TIMER);
            SendMirrorTimer(FATIGUE_TIMER, m_MirrorTimer[FATIGUE_TIMER], m_MirrorTimer[FATIGUE_TIMER], -1);
        }
        else
        {
            m_MirrorTimer[FATIGUE_TIMER]-=time_diff;
            // Timer limit - need deal damage or teleport ghost to graveyard
            if (m_MirrorTimer[FATIGUE_TIMER] < 0)
            {
                m_MirrorTimer[FATIGUE_TIMER]+= 1*MILLISECONDS;
                if (isAlive())                                            // Calculate and deal damage
                {
                    uint32 damage = GetMaxHealth() / 5 + urand(0, GetLevel()-1);
                    EnvironmentalDamage(DAMAGE_EXHAUSTED, damage);
                }
                else if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))       // Teleport ghost to graveyard
                    RepopAtGraveyard();
            }
            else if (!(m_MirrorTimerFlagsLast & UNDERWATER_INDARKWATER))
                SendMirrorTimer(FATIGUE_TIMER, getMaxTimer(FATIGUE_TIMER), m_MirrorTimer[FATIGUE_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[FATIGUE_TIMER] != DISABLED_MIRROR_TIMER)       // Regen timer
    {
        int32 DarkWaterTime = getMaxTimer(FATIGUE_TIMER);
        m_MirrorTimer[FATIGUE_TIMER]+=10*time_diff;
        if (m_MirrorTimer[FATIGUE_TIMER] >= DarkWaterTime || !isAlive())
            StopMirrorTimer(FATIGUE_TIMER);
        else if (m_MirrorTimerFlagsLast & UNDERWATER_INDARKWATER)
            SendMirrorTimer(FATIGUE_TIMER, DarkWaterTime, m_MirrorTimer[FATIGUE_TIMER], 10);
    }

    if (m_MirrorTimerFlags & (UNDERWATER_INLAVA|UNDERWATER_INSLIME))
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[FIRE_TIMER] == DISABLED_MIRROR_TIMER)
            m_MirrorTimer[FIRE_TIMER] = getMaxTimer(FIRE_TIMER);
        else
        {
            m_MirrorTimer[FIRE_TIMER]-=time_diff;
            if (m_MirrorTimer[FIRE_TIMER] < 0)
            {
                m_MirrorTimer[FIRE_TIMER]+= 1*MILLISECONDS;
                // Calculate and deal damage
                // TODO: Check this formula
                uint32 damage = urand(600, 700);
                if (m_MirrorTimerFlags&UNDERWATER_INLAVA)
                    EnvironmentalDamage(DAMAGE_LAVA, damage);
                else if (m_zoneUpdateId != 1497)
                    EnvironmentalDamage(DAMAGE_SLIME, damage);
            }
        }
    }
    else
        m_MirrorTimer[FIRE_TIMER] = DISABLED_MIRROR_TIMER;

    // Recheck timers flag
    m_MirrorTimerFlags&=~UNDERWATER_EXIST_TIMERS;
    for (uint8 i = 0; i< MAX_TIMERS; ++i)
        if (m_MirrorTimer[i] != DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimerFlags|=UNDERWATER_EXIST_TIMERS;
            break;
        }
    m_MirrorTimerFlagsLast = m_MirrorTimerFlags;
}

///The player sobers by 256 every 10 seconds
void Player::HandleSobering()
{
    m_drunkTimer = 0;

    uint32 drunk = (m_drunk <= 256) ? 0 : (m_drunk - 256);
    SetDrunkValue(drunk);
}

DrunkenState Player::GetDrunkenstateByValue(uint16 value)
{
    if (value >= 23000)
        return DRUNKEN_SMASHED;
    if (value >= 12800)
        return DRUNKEN_DRUNK;
    if (value & 0xFFFE)
        return DRUNKEN_TIPSY;
    return DRUNKEN_SOBER;
}

void Player::SetDrunkValue(uint16 newDrunkenValue, uint32 itemId)
{
    uint32 oldDrunkenState = Player::GetDrunkenstateByValue(m_drunk);

    m_drunk = newDrunkenValue;
    SetUInt32Value(PLAYER_BYTES_3,(GetUInt32Value(PLAYER_BYTES_3) & 0xFFFF0001) | (m_drunk & 0xFFFE));

    uint32 newDrunkenState = Player::GetDrunkenstateByValue(m_drunk);

    // special drunk invisibility detection
    if (newDrunkenState >= DRUNKEN_DRUNK)
        m_detectInvisibilityMask |= (1<<6);
    else
        m_detectInvisibilityMask &= ~(1<<6);

    if (newDrunkenState == oldDrunkenState)
        return;

    WorldPacket data(SMSG_CROSSED_INEBRIATION_THRESHOLD, (8+4+4));
    data << uint64(GetGUID());
    data << uint32(newDrunkenState);
    data << uint32(itemId);

    BroadcastPacket(&data, true);
}

void Player::CreateCharmAI()
{
    if (GetSession()->isFakeBot())
    {
        i_AI = new FakeBotAI(this);
        CharmAI(true);
    }
    else
    {
        switch (GetClass())
        {
            case CLASS_WARRIOR:
                i_AI = new WarriorAI(this);
                break;
            case CLASS_PALADIN:
                i_AI = new PaladinAI(this);
                break;
            case CLASS_HUNTER:
                i_AI = new HunterAI(this);
                break;
            case CLASS_ROGUE:
                i_AI = new RogueAI(this);
                break;
            case CLASS_PRIEST:
                i_AI = new PriestAI(this);
                break;
            case CLASS_SHAMAN:
                i_AI = new ShamanAI(this);
                break;
            case CLASS_MAGE:
                i_AI = new MageAI(this);
                break;
            case CLASS_WARLOCK:
                i_AI = new WarlockAI(this);
                break;
            case CLASS_DRUID:
                i_AI = new DruidAI(this);
                break;
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Unhandled class type, while creating charmAI");
                break;
        }
    }
}

void Player::DeleteCharmAI()
{
    if (i_AI)
        delete i_AI;
}

void Player::CharmAI(bool apply)
{
    if (IsAIEnabled = apply)
        AI()->Reset();

    GetMotionMaster()->StopControlledMovement();
}

void Player::Update(uint32 update_diff, uint32 p_time)
{
    if (!IsInWorld() || _preventUpdate)
        return;

    updateMutex.acquire();

    _preventUpdate = true;

	// do not allow the AI to be changed during update
	if (IsAIEnabled)
	{
		m_AI_locked = true;
		i_AI->UpdateAI(p_time);
		m_AI_locked = false;

		if (GetSession()->isFakeBot())
		{
			_preventUpdate = false;
			updateMutex.release();
			return;
		}
	}

    if (m_refreshPetSpells) // must be before spells update
    {
        PetSpellInitialize();
        m_refreshPetSpells = false;
    }

    positionStatus.Update(update_diff);

    // undelivered mail
    if (m_nextMailDelivereTime && m_nextMailDelivereTime <= time(NULL))
    {
        SendNewMail();
        ++unReadMails;

        // It will be recalculate at mailbox open (for unReadMails important non-0 until mailbox open, it also will be recalculated)
        m_nextMailDelivereTime = 0;
    }

    if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_MOVEMENT_FLAGS)
    {
        ChatHandler(this).PSendSysMessage("MovementFlags: %u", m_movementInfo.GetMovementFlags());
    }

    Unit::Update(update_diff, p_time);

    time_t now = time(NULL);

    UpdatePvPFlag(now);

    UpdateContestedPvP(update_diff);

    UpdateDuelFlag(now);

    CheckDuelDistance(now);

    UpdateAfkReport(now);

    // Update items that have just a limited lifetime
    if (now>m_Last_tick)
        UpdateItemDuration(uint32(now- m_Last_tick));

    if (!m_timedquests.empty())
    {
        std::set<uint32>::iterator iter = m_timedquests.begin();
        while (iter != m_timedquests.end())
        {
            QuestStatusData& q_status = mQuestStatus[*iter];
            if (q_status.m_timer <= update_diff)
            {
                uint32 quest_id  = *iter;
                ++iter;                                     // current iter will be removed in FailTimedQuest
                FailTimedQuest(quest_id);
            }
            else
            {
                q_status.m_timer -= update_diff;
                if (q_status.uState != QUEST_NEW)
                    q_status.uState = QUEST_CHANGED;

                ++iter;
            }
        }
    }

    if (HasUnitState(UNIT_STAT_MELEE_ATTACKING) && !HasUnitState(UNIT_STAT_CANNOT_AUTOATTACK))
    {
        if (Unit *pVictim = GetVictim())
        {
            if (isAttackReady(BASE_ATTACK))
            {
                if (!IsWithinMeleeRange(pVictim))
                {
                    setAttackTimer(BASE_ATTACK,100);
                    if (m_swingErrorMsg != 1)                // send single time (client auto repeat)
                    {
                        SendAttackSwingNotInRange();
                        m_swingErrorMsg = 1;
                    }
                }
                // 120 degrees of radiant range
				// DANGEROUS No orientation check for auto attacks or spells below this distance > 1.4f
                else if (!HasInArc(2*M_PI/3, pVictim) && 
					(pVictim->GetTypeId() != TYPEID_PLAYER || (pVictim->GetTypeId() == TYPEID_PLAYER && GetExactDistance2d(pVictim->GetPositionX(), pVictim->GetPositionY()) > 1.4f)))
                {
                    setAttackTimer(BASE_ATTACK,100);
                    if (m_swingErrorMsg != 2)                // send single time (client auto repeat)
                    {
                        SendAttackSwingBadFacingAttack();
                        m_swingErrorMsg = 2;
                    }
                }
                else
                {
                    m_swingErrorMsg = 0;                    // reset swing error state

                    // prevent base and off attack in same time, delay attack at 0.2 sec
                    if (haveOffhandWeapon())
                    {
                        uint32 off_att = getAttackTimer(OFF_ATTACK);
                        if (off_att < ATTACK_DISPLAY_DELAY)
                            setAttackTimer(OFF_ATTACK,ATTACK_DISPLAY_DELAY);
                    }
                    AttackerStateUpdate(pVictim, BASE_ATTACK);
                    resetAttackTimer(BASE_ATTACK);
                }
            }

            if (haveOffhandWeapon() && isAttackReady(OFF_ATTACK))
            {
                if (!IsWithinMeleeRange(pVictim))
                {
                    setAttackTimer(OFF_ATTACK,100);
                }
				// DANGEROUS No orientation check for auto attacks or spells below this distance > 1.4f
				else if (!HasInArc(2 * M_PI / 3, pVictim) &&
					(pVictim->GetTypeId() != TYPEID_PLAYER || (pVictim->GetTypeId() == TYPEID_PLAYER && GetExactDistance2d(pVictim->GetPositionX(), pVictim->GetPositionY()) > 1.4f)))
                {
                    setAttackTimer(OFF_ATTACK,100);
                }
                else
                {
                    // prevent base and off attack in same time, delay attack at 0.2 sec
                    uint32 base_att = getAttackTimer(BASE_ATTACK);
                    if (base_att < ATTACK_DISPLAY_DELAY)
                        setAttackTimer(BASE_ATTACK,ATTACK_DISPLAY_DELAY);
                    // do attack
                    AttackerStateUpdate(pVictim, OFF_ATTACK);
                    resetAttackTimer(OFF_ATTACK);
                }
            }
        }
    }

    if (m_restStateTimer <= update_diff)
    {
        if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
        {
            if (uint32 timeInnEnter = GetTimeInnEnter())
            {
                uint32 time_at_inn = time(NULL) - timeInnEnter;
                if (time_at_inn >= 10)                             //freeze update
                {
                    float bubble = 0.125 * (!IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1) ? sWorld.getConfig(RATE_REST_INGAME) : 1.0f);
                    //speed collect rest bonus (section/in hour)
                    SetRestBonus(GetRestBonus() + time_at_inn*((float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP) / 72000)*bubble);
                    AddInnEnterTime(time_at_inn);
                }
            }
        }

        if (GetByteValue(PLAYER_BYTES_2, 3) == REST_STATE_RAF)
        {
            if (!CheckRAFConditions_XP())
            {
                if (m_rest_bonus > 10)
                    SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_RESTED);             // Set Reststate = Rested
                else if (m_rest_bonus <= 1)
                    SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_NORMAL);             // Set Reststate = Normal
            }
        }
        // On check update rest state and RAF state if RAF conditions are no longer met. Trigger-gained on GiveXP
        m_restStateTimer = 10000;
    }
    else
        m_restStateTimer -= update_diff;

    if (update_diff >= m_zoneUpdateTimer)
    {
        uint32 newzone = GetZoneId();
        if (m_zoneUpdateId != newzone)
            UpdateZone(newzone);                        // also update area
        else
        {
            // use area updates as well
            // needed for free far all arenas for example
            uint32 newarea = GetAreaId();
            if (m_areaUpdateId != newarea)
                UpdateArea(newarea);

            if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
            {
                if (GetRestType() == REST_TYPE_IN_TAVERN)          // has been in tavern. Is still in?
                {
                    if (GetMapId() != GetInnPosMapId() || sqrt((GetPositionX() - GetInnPosX())*(GetPositionX() - GetInnPosX()) + (GetPositionY() - GetInnPosY())*(GetPositionY() - GetInnPosY()) + (GetPositionZ() - GetInnPosZ())*(GetPositionZ() - GetInnPosZ())) > 40)
                    {
                        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
                        SetRestType(REST_TYPE_NO);
                    }
                }
            }

            m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL;
        }
    }
    else
        m_zoneUpdateTimer -= update_diff;

    if (m_timeSyncTimer > 0)
    {
        if (update_diff >= m_timeSyncTimer)
            SendTimeSync();
        else
            m_timeSyncTimer -= update_diff;
    }

    m_regenTimer += update_diff;
    if (m_regenTimer >= 2000)
    {
        CombatTimerTick(update_diff);

        if (isAlive())
            RegenerateAll();
        
        m_regenTimer -= 2000;
    }

    if (m_deathState == JUST_DIED)
        KillPlayer();

    if (m_nextSave > 0)
    {
        if (update_diff >= m_nextSave)
        {
            // m_nextSave reseted in SaveToDB call
            SaveToDB();
            sLog.outDetail("Player '%s' (GUID: %u) saved", GetName(), GetGUIDLow());
        }
        else
            m_nextSave -= update_diff;
    }

    //Handle drowning
    HandleDrowning(update_diff);

    //Handle detect stealth players
    if (m_DetectInvTimer > 0)
    {
        if (update_diff >= m_DetectInvTimer)
        {
            HandleStealthedUnitsDetection();
            m_DetectInvTimer = (InBattleGroundOrArena() || duel) ? 500 : 1000;
        }
        else
            m_DetectInvTimer -= update_diff;
    }

    // Played time
    if (now > m_Last_tick)
    {
        uint32 elapsed = uint32(now - m_Last_tick);
        m_Played_time[0] += elapsed;                        // Total played time
        m_Played_time[1] += elapsed;                        // Level played time
        m_Last_tick = now;

        // GENSENTODO check if it works correctly after list -> vector
        if (!sWorld.isEasyRealm())
        {
            ingame_time += elapsed;
            if (ingame_time > HOUR)
            {
                ingame_time = 0;

                TwinkGuids same_chars = GetSamePlayers();
                if (!same_chars.empty() && GetGUIDLow() == *same_chars.begin())
                {
                    bool exists = false;
                    for (PlayerMails::iterator itr = GetmailBegin(); itr != GetmailEnd(); ++itr)
                    {
                        Mail* m = (*itr);
                        if (m->subject == "Supplies" && m->has_items)
                        {
                            exists = true;
                            break;
                        }
                    }

                    if (!exists)
                    {
                        // send mail
                        if (Item* item = Item::CreateItem(5049, 1, this))
                        {
                            item->SaveToDB();

                            MailDraft("Supplies", 0)
                                .AddItem(item)
                                .SendMailTo(MailReceiver(this), MailSender(MAIL_NORMAL, uint32(0), MAIL_STATIONERY_GM), MAIL_CHECK_MASK_RETURNED);

                            sLog.outLog(LOG_SPECIAL, "Supply Package sent to player %s (guid %u)", GetName(), GetGUIDLow());
                        }
                    }

                    // set other same_chars ingame_time to 0 to prevent abuses
                    for (auto sc : same_chars)
                    {
                        if (GetGUIDLow() == sc)
                            continue;

                        Player* sp = sObjectMgr.GetPlayerInWorld(sc);
                        if (sp)
                            sp->ingame_time = 0;
                    }
                }
            }
        }
    }

    if (m_drunk)
    {
        m_drunkTimer += update_diff;

        if (m_drunkTimer > 10000)
            HandleSobering();
    }

    // not auto-free ghost from body in instances
    if (m_deathTimer > 0  && !GetMap()->Instanceable())
    {
        if (update_diff >= m_deathTimer)
        {
            m_deathTimer = 0;
            BuildPlayerRepop();
            RepopAtGraveyard();
        }
        else
            m_deathTimer -= update_diff;
    }

    UpdateEnchantTime(update_diff);
    UpdateHomebindTime(update_diff);

    // group update
    SendUpdateToOutOfRangeGroupMembers();

    UpdateConsecutiveKills();

    // update_pets
    for (auto pet : GetAllPets())
    {
        if (pet)
            pet->Update(update_diff, p_time);
    }

    _preventUpdate = false;
    updateMutex.release();
}

void Player::setDeathState(DeathState s)
{
    uint32 ressSpellId = 0;

    bool cur = isAlive();

    if (s == JUST_DIED)
    {
        if (!cur)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: setDeathState: attempt to kill a dead player %s(%d)", GetName(), GetGUIDLow());
            return;
        }

        // drunken state is cleared on death
        SetDrunkValue(0);
        // lost combo points at any target (targeted combo points clear in Unit::setDeathState)
        ClearComboPoints();

        clearResurrectRequestData();

        // remove form before other mods to prevent incorrect stats calculation
		if (m_ShapeShiftFormSpellId && m_form != FORM_BATTLESTANCE && m_form != FORM_BERSERKERSTANCE && m_form != FORM_DEFENSIVESTANCE)
			RemoveAurasDueToSpell(m_ShapeShiftFormSpellId);

        //FIXME: is pet dismissed at dying or releasing spirit? if second, add setDeathState(DEAD) to HandleRepopRequestOpcode and define pet unsummon here with (s == DEAD)
        RemovePet(NULL, PET_SAVE_NOT_IN_SLOT, true, true);

        // remove uncontrolled pets
        RemoveMiniPet();
        RemoveGuardians();

		if (Unit* charmed = GetCharm())
			charmed->RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM);

        // save value before aura remove in Unit::setDeathState
        ressSpellId = GetUInt32Value(PLAYER_SELF_RES_SPELL);

        // passive spell
        if (!ressSpellId)
            ressSpellId = GetResurrectionSpellId();
    }
    Unit::setDeathState(s);

    // restore resurrection spell id for player after aura remove
    if (s == JUST_DIED && cur && ressSpellId)
        SetUInt32Value(PLAYER_SELF_RES_SPELL, ressSpellId);

    if (isAlive() && !cur)
    {
        // restore custom aura tabard aura after ressurect
        if (sWorld.isEasyRealm())
            ApplyTabardAura();
        
        //clear aura case after resurrection by another way (spells will be applied before next death)
        SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);

        // restore default warrior stance
        //if (GetClass() == CLASS_WARRIOR)
        //    CastSpell(this,SPELL_ID_PASSIVE_BATTLE_STANCE,true);
    }
}

bool Player::BuildEnumData(QueryResultAutoPtr result, WorldPacket * p_data)
{
    Field *fields = result->Fetch();
    uint32 guid = fields[0].GetUInt32();
    uint8 pRace = fields[2].GetUInt8();
    uint8 pClass = fields[3].GetUInt8();
    PlayerInfo const *info = sObjectMgr.GetPlayerInfo(pRace, pClass);
    if (!info)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player %u has incorrect race/class pair. Don't build enum.", guid);
        return false;
    }

    *p_data << uint64(MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER));
    *p_data << fields[1].GetString();                       // name
    *p_data << uint8(pRace);                                // race
    *p_data << uint8(pClass);                               // class
    *p_data << uint8(fields[4].GetUInt8());                 // gender
    uint32 playerBytes = fields[5].GetUInt32();
    *p_data << uint8(playerBytes);                          // skin
    *p_data << uint8(playerBytes >> 8);                     // face
    *p_data << uint8(playerBytes >> 16);                    // hair style
    *p_data << uint8(playerBytes >> 24);                    // hair color

    uint32 playerBytes2 = fields[6].GetUInt32();
    *p_data << uint8(playerBytes2 & 0xFF);                  // facial hair
    *p_data << uint8(fields[7].GetUInt8());                 // level
    *p_data << uint32(fields[8].GetUInt32());               // zone
    *p_data << uint32(fields[9].GetUInt32());               // map

    *p_data << fields[10].GetFloat();                       // x
    *p_data << fields[11].GetFloat();                       // y
    *p_data << fields[12].GetFloat();                       // z

    *p_data << (result ? result->Fetch()[13].GetUInt32() : 0);

    uint32 char_flags = 0;
    uint32 playerFlags = fields[14].GetUInt32();
    uint32 atLoginFlags = fields[15].GetUInt32();
    if (playerFlags & PLAYER_FLAGS_HIDE_HELM)
        char_flags |= CHARACTER_FLAG_HIDE_HELM;
    if (playerFlags & PLAYER_FLAGS_HIDE_CLOAK)
        char_flags |= CHARACTER_FLAG_HIDE_CLOAK;
    if (playerFlags & PLAYER_FLAGS_GHOST)
        char_flags |= CHARACTER_FLAG_GHOST;
    if (atLoginFlags & AT_LOGIN_RENAME)
        char_flags |= CHARACTER_FLAG_RENAME;
    if (sWorld.getConfig(CONFIG_DECLINED_NAMES_USED))
    {
        if (!fields[20].GetCppString().empty())
            char_flags |= CHARACTER_FLAG_DECLINED;
    }
    else
        char_flags |= CHARACTER_FLAG_DECLINED;

    // used by propaganda char and might be used by us. Maybe
    if (playerFlags & PLAYER_FLAGS_LOCKED_BY_BILLING)
        char_flags |= CHARACTER_FLAG_LOCKED_BY_BILLING;

    *p_data << uint32(char_flags);                          // character flags
    *p_data << uint8(1);                                    // unknown

    // Pets info
    {
        uint32 petDisplayId = 0;
        uint32 petLevel   = 0;
        uint32 petFamily  = 0;

        // show pet at selection character in character list only for non-ghost character
        if (result && !(playerFlags & PLAYER_FLAGS_GHOST) && (pClass == CLASS_WARLOCK || pClass == CLASS_HUNTER))
        {
            uint32 entry = fields[16].GetUInt32();
            CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(entry);
            if (cInfo)
            {
                petDisplayId = fields[17].GetUInt32();
                petLevel     = fields[18].GetUInt32();
                petFamily    = cInfo->family;
            }
        }

        *p_data << uint32(petDisplayId);
        *p_data << uint32(petLevel);
        *p_data << uint32(petFamily);
    }
    Tokens data = StrSplit(fields[19].GetCppString(), " ");
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        uint32 visualbase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
        uint32 item_id = GetUInt32ValueFromArray(data, visualbase);

        const ItemPrototype * proto = ObjectMgr::GetItemPrototype(item_id);
        if (!proto)
        {
            *p_data << uint32(0);
            *p_data << uint8(0);
            *p_data << uint32(0);
            continue;
        }

        SpellItemEnchantmentEntry const *enchant = NULL;

        for (uint8 enchantSlot = PERM_ENCHANTMENT_SLOT; enchantSlot <= TEMP_ENCHANTMENT_SLOT; ++enchantSlot)
        {
            uint32 enchantId = GetUInt32ValueFromArray(data, visualbase+1+enchantSlot);
            if (!enchantId)
                continue;

            if (enchant = sSpellItemEnchantmentStore.LookupEntry(enchantId))
                break;
        }

        *p_data << uint32(proto->DisplayInfoID);
        *p_data << uint8(proto->InventoryType);
        *p_data << uint32(enchant ? enchant->aura_id : 0);
    }
    *p_data << uint32(0);                                   // first bag display id
    *p_data << uint8(0);                                    // first bag inventory type
    *p_data << uint32(0);                                   // enchant?

    return true;
}

bool Player::ToggleAFK()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK);

    // afk player not allowed in battleground
    if (isAFK() && InBattleGroundOrArena() && !InArena() && !isGameMaster())
    {
        if (IsSpectator())
            SetSpectator(false);
        LeaveBattleground();
    }    

    return isAFK();
}

bool Player::ToggleDND()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_DND);

    return HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_DND);
}

uint8 Player::chatTag() const
{
    // it's bitmask
    // 0x8 - ??
    // 0x4 - gm
    // 0x2 - dnd
    // 0x1 - afk
    if (isGMChat())
        return 4;
    else if (isDND())
        return 3;
    if (isAFK())
        return 1;
    else
        return 0;
}

bool Player::TeleportTo(uint32 mapid, float x, float y, float z, float orientation, uint32 options)
{
    if (!MapManager::IsValidMapCoord(mapid, x, y, z, orientation))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: TeleportTo: invalid map %d or absent instance template.", mapid);
        return false;
    }

    if (!GetSession()->HasPermissions(PERM_ADM) && !sWorld.IsAllowedMap(mapid))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player %s tried to enter a forbidden map", GetName());
        return false;
    }

    // preparing unsummon pet if lost (we must get pet before teleportation or will not find it later)
    Pet* pet = GetPet();

    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);

    // don't let enter battlegrounds without assigned battleground id (for example through areatrigger)...
    // don't let gm level > 1 either
    if (!InBattleGroundOrArena() && mEntry->IsBattleGroundOrArena())
        return false;

    // 449 - Champions' Hall (Alliance) // 450 - Hall of Legends (Horde)
    if (!GetDummyAura(54844)) // Battleground switcher DUMMY
    {
        if (mapid == 449 && GetTeam() == HORDE && !isGameMaster())
        {
            GetSession()->SendNotification(LANG_NO_ENTER_CHAMPIONS_HALL);
            return false;
        }

        if (mapid == 450 && GetTeam() == ALLIANCE && !isGameMaster())
        {
            GetSession()->SendNotification(LANG_NO_ENTER_HALL_OF_LEGENDS);
            return false;
        }
    }

    // client without expansion support
    if (GetSession()->Expansion() < mEntry->Expansion())
    {
        sLog.outDebug("Player %s using client without required expansion tried teleport to non accessible map %u", GetName(), mapid);

        if (GetTransport())
            RepopAtGraveyard();                             // teleport to near graveyard if on transport, looks blizz like :)

        SendTransferAborted(mapid, TRANSFER_ABORT_INSUF_EXPAN_LVL1);
        return false;                                       // normal client can't teleport to this map...
    }
    else
    {
        sLog.outDebug("Player %s will teleported to map %u", GetName(), mapid);
    }

    // if we were on a transport, leave
    if (!(options & TELE_TO_NOT_LEAVE_TRANSPORT) && m_transport)
    {
        m_transport->RemovePassenger(this);
        m_transport = NULL;
        m_movementInfo.ClearTransportData();
    }
    
    SetSemaphoreTeleport(true);

    // The player was ported to another map and looses the duel immediatly.
    // We have to perform this check before the teleport, otherwise the
    // ObjectAccessor won't find the flag.
    if (duel && GetMapId() != mapid)
    {
        GameObject* obj = GetMap()->GetGameObject(GetUInt64Value(PLAYER_DUEL_ARBITER));
        if (obj)
            DuelComplete(DUEL_FLED);
    }

    // reset movement flags at teleport, because player will continue move with these flags after teleport
    m_movementInfo.SetMovementFlags(MOVEFLAG_NONE);
    DisableSpline();

	// remove spectator if active
	if ((mapid == 0 || mapid == 1 || mapid == 530) && IsSpectator())
		SetSpectator(false);

    //remove GM berserk aura
    if (HasAura(41924))
        RemoveAurasDueToSpell(41924);

    if ((GetMapId() == mapid) && (!m_transport))
    {
        float DistanceToTeleport = GetDistance(x, y, z);
        SetAntiCheatJustTeleported(DistanceToTeleport);

        // prepare zone change detect
        uint32 old_zone = GetZoneId();

        WorldLocation tmpWLoc(mapid, x, y, z, orientation);

        if (!IsWithinDistInMap(&tmpWLoc, GetMap()->GetVisibilityDistance()))
            DestroyForNearbyPlayers();

        // near teleport
        if (!GetSession()->PlayerLogout())
        {
            // send transfer packet to display load screen
            WorldPacket data;
            BuildTeleportAckMsg(data, x, y, z, orientation);
            SendPacketToSelf(&data);
            SetPosition(x, y, z, orientation, true);
        }
        else
            // this will be used instead of the current location in SaveToDB
            m_teleport_dest = tmpWLoc;

        SetFallInformation(0, z);

        //if (!(options & TELE_TO_NOT_UNSUMMON_PET))
        //{
            //same map, only remove pet if out of range
            if (pet && !IsWithinDistInMap(pet, OWNER_MAX_DISTANCE))
            {
                if (pet->isControlled() && !pet->isTemporarySummoned())
                    m_temporaryUnsummonedPetNumber = pet->GetCharmInfo()->GetPetNumber();
                else
                    m_temporaryUnsummonedPetNumber = 0;

                RemovePet(pet, PET_SAVE_NOT_IN_SLOT);
            }
        //}

        if (!(options & TELE_TO_NOT_LEAVE_COMBAT))
            CombatStop();

        if (!(options & TELE_TO_NOT_UNSUMMON_PET))
        {
            // resummon pet
            if (pet && m_temporaryUnsummonedPetNumber)
            {
                Pet* NewPet = new Pet;
                if (!NewPet->LoadPetFromDB(this, 0, m_temporaryUnsummonedPetNumber, true))
                    delete NewPet;

                m_temporaryUnsummonedPetNumber = 0;
            }
        }

        // near teleport, triggering send MSG_MOVE_TELEPORT_ACK from client at landing
        if (!GetSession()->PlayerLogout())
        {
            // don't reset teleport semaphore while logging out, otherwise m_teleport_dest won't be used in Player::SaveToDB
            SetSemaphoreTeleport(false);
            UpdateZone(GetZoneId());
        }

        // new zone
        if (old_zone != GetZoneId())
        {
            // honorless target
            if (pvpInfo.inHostileArea)
                CastSpell(this, 2479, true);
            else if (IsPvP() && !HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_IN_PVP) && DistanceToTeleport > 50)
                UpdatePvP(false);
        }

        if (getFollowingGM())
        {
            setGMFollow(0);
        }
        else if (getFollowTarget())
        {
            setFollowTarget(0);
            GetMotionMaster()->Clear(true);
        }
    }
    else
    {
        m_teleport_options = options;
        // far teleport to another map
        Map* oldmap = IsInWorld() ? GetMap() : NULL;
        // check if we can enter before stopping combat / removing pet / totems / interrupting spells

        // Check enter rights before map getting to avoid creating instance copy for player
        // this check not dependent from map instance copy and same for all instance copies of selected map
        if (!sMapMgr.CanPlayerEnter(mapid, this))
        {
            SetSemaphoreTeleport(false);
            return false;
        }

        if (InstanceGroupBind *igb = GetGroup() ? GetGroup()->GetBoundInstance(mapid, GetDifficulty()) : NULL)
        {
            if (Map *iMap = sMapMgr.FindMap(mapid,igb->save->GetSaveInstanceId()))
            {
                if (iMap->EncounterInProgress(this) && !(options & TELE_TO_RESURRECT))
                {
                    SetSemaphoreTeleport(false);
                    return false;
                }
            }
        }

        // If the map is not created, assume it is possible to enter it.
        // It will be created in the WorldPortAck.
        Map *map = sMapMgr.FindMap(mapid, sObjectMgr.GetSingleInstance(mapid, x, y));
        if (!map || map->CanEnter(this))
        {
            SaveSelection(0);
            CombatStop();
            ResetContestedPvP();

            StartTeleportTimer();

            // remove player from battleground on far teleport (when changing maps)
            if (BattleGround const* bg = GetBattleGround())
            {
                // Note: at battleground join battleground id set before teleport
                // and we already will found "current" battleground
                // just need check that this is targeted map or leave
                if (bg->GetMapId() != mapid)
                    LeaveBattleground(false);                   // don't teleport to entry point
            }

            // remove pet on map change
            if (pet)
            {
                //leaving map -> delete pet right away (doing this later will cause problems)
                if (pet->isControlled() && !pet->isTemporarySummoned())
                    m_temporaryUnsummonedPetNumber = pet->GetCharmInfo()->GetPetNumber();
                else
                    m_temporaryUnsummonedPetNumber = 0;

                RemovePet(pet, PET_SAVE_NOT_IN_SLOT);
            }

            // remove all dyn objects
            RemoveAllDynObjects();

            // stop spellcasting
            // not attempt interrupt teleportation spell at caster teleport
            if (!(options & TELE_TO_SPELL))
                if (IsNonMeleeSpellCast(true))
                    InterruptNonMeleeSpells(true);

            if(oldmap && oldmap->GetId() == 560) // Old Hillsbrad Foothills taxi flight removing
                InterruptTaxiFlying();

            //remove auras before removing from map...
            RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING);
            
            if (mEntry->IsRaid())
                RemoveAurasWithInterruptFlags(AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_IN_RAID_INSTANCE);

            if (GetClass() == CLASS_ROGUE && mEntry->IsDungeon())
            {
                if (!HasAura(SPELL_ROGUE_RAID_BUFF))
                    AddAuraCreate(SPELL_ROGUE_RAID_BUFF, this);
            }
            else
                RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ONLY_IN_RAID);

            if (mEntry->IsBattleGroundOrArena())
                RemoveAurasWithInterruptFlags(GetClass() == CLASS_HUNTER ? (AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP | AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_HUNTER_PVP) : AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_PVP);

            if (!GetSession()->PlayerLogout())
            {
                // send transfer packets
                WorldPacket data(SMSG_TRANSFER_PENDING, (4+4+4));
                data << uint32(mapid);
                if (m_transport)
                {
                    data << uint32(m_transport->GetEntry());
                    data << uint32(GetMapId());
                }
                SendPacketToSelf(&data);

                data.Initialize(SMSG_NEW_WORLD, (20));
                if (m_transport)
                {
                    data << uint32(mapid);
                    data << float(m_movementInfo.GetTransportPos()->x);
                    data << float(m_movementInfo.GetTransportPos()->y);
                    data << float(m_movementInfo.GetTransportPos()->z);
                    data << float(m_movementInfo.GetTransportPos()->o);
                }
                else
                {
                    data << uint32(mapid);
                    data << float(x);
                    data << float(y);
                    data << float(z);
                    data << float(orientation);
                }

                SendPacketToSelf(&data);
                SendSavedInstances();

                if (getFollowingGM())
                {
                    setGMFollow(0);
                }
                else if (getFollowTarget())
                {
                    setFollowTarget(0);
                    GetMotionMaster()->Clear(true);
                }

                volatile uint32 debug_map_id = GetMapId();
                volatile uint32 debug_oldmap_id = mapid;
                volatile uint32 debug_target = GetGUIDLow();

                // remove from old map now
                if(oldmap)
                    oldmap->Remove(this, false);
            }

            // new final coordinates
            float final_x = x;
            float final_y = y;
            float final_z = z;
            float final_o = orientation;

            if (m_transport)
            {
                final_x += m_movementInfo.GetTransportPos()->x;
                final_y += m_movementInfo.GetTransportPos()->y;
                final_z += m_movementInfo.GetTransportPos()->z;
                final_o += m_movementInfo.GetTransportPos()->o;
            }

            m_teleport_dest = WorldLocation(mapid, final_x, final_y, final_z, final_o);

            SetFallInformation(0, final_z);
            // if the player is saved before worldportack (at logout for example)
            // this will be used instead of the current location in SaveToDB

            if (isGameMaster() && oldmap && oldmap->IsBattleGround()) // remove pvp minibutton
            {
                if (BattleGround* bg = sBattleGroundMgr.GetBattleGround(oldmap->GetInstanciableInstanceId(), BATTLEGROUND_TYPE_NONE))
                {
                    WorldPacket data;
                    sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, bg, GetTeam(), 0, STATUS_NONE, 0, bg->GetStartTime());
                    m_session->SendPacket(&data);
                }
            }
            SetAntiCheatJustTeleported();

        }
        else
            return false;
    }

    return true;
}

void Player::AddToWorld()
{
    ///- Do not add/remove the player from the object storage
    ///- It will crash when updating the ObjectAccessor
    ///- The player should only be added when logging in
    Unit::AddToWorld();

    AddPlayerIPHash();

    for (int i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
    {
        if (m_items[i])
            m_items[i]->AddToWorld();
    }
}

void Player::RemoveFromWorld()
{
    // cleanup
    if (IsInWorld())
    {
        ///- Release charmed creatures, unsummon totems and remove pets/guardians
        StopCastingCharm();
        StopCastingBindSight();
        UnsummonAllTotems();
        RemoveMiniPet();
        RemoveGuardians();
    }

    RemovePlayerIPHash();

    sOutdoorPvPMgr.HandlePlayerLeave(this);

    for (int i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; i++)
    {
        if (m_items[i])
            m_items[i]->RemoveFromWorld();
    }

    ///- Do not add/remove the player from the object storage
    ///- It will crash when updating the ObjectAccessor
    ///- The player should only be removed when logging out
    Unit::RemoveFromWorld();
}

void Player::RewardRage(uint32 damage, uint32 weaponSpeedHitFactor, bool attacker, Creature* attackerCr)
{
    float addRage;

    float rageconversion = ((0.0091107836 * GetLevel()*GetLevel())+3.225598133*GetLevel())+4.2652911;

    if (attacker)
    {
		addRage = ((float(damage) / rageconversion * 7.5 + weaponSpeedHitFactor) / 2);

        if (addRage > 15*damage/rageconversion)
            addRage = 15*damage/rageconversion;

        // talent who gave more rage on attack
        addRage *= 1.0f + GetTotalAuraModifier(SPELL_AURA_MOD_RAGE_FROM_DAMAGE_DEALT) / 100.0f;
    }
    else
    {
		addRage = (float(damage) / rageconversion)*2.5;
        
        // Berserker Rage effect
        if (HasAura(18499,0))
            addRage *= 2.0f;

        if (attackerCr)
        {
            float dmgMod = attackerCr->GetCreatureDamageMod();
            if (dmgMod < 1.0f) // only for config-lowered damage
                addRage /= dmgMod;
        }

		// @!tanks_boost warriors gain 50% bonus rage in instances
		if (sWorld.isEasyRealm() && GetClass() == CLASS_WARRIOR)
		{
			Map* map = GetMap();
			if (map && map->IsDungeon())
				addRage *= 1.5f;
		}
    }

    addRage *= sWorld.getConfig(RATE_POWER_RAGE_INCOME);

    ModifyPower(POWER_RAGE, uint32(addRage*10));
}

void Player::RegenerateAll()
{
    // Not in combat or they have regeneration
    if (!IsInCombat() || HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT) ||
        HasAuraType(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT) || IsPolymorphed())
    {
        RegenerateHealth();
        if (!IsInCombat() && !HasAuraType(SPELL_AURA_INTERRUPT_REGEN))
            Regenerate(POWER_RAGE);
    }

    Regenerate(POWER_ENERGY);

    Regenerate(POWER_MANA);
}

void Player::Regenerate(Powers power)
{
    uint32 curValue = GetPower(power);
    uint32 maxValue = GetMaxPower(power);

    float addvalue = 0.0f;

    switch (power)
    {
        case POWER_MANA:
        {
            bool recentCast = IsUnderLastManaUseEffect();
            float ManaIncreaseRate = sWorld.getConfig(RATE_POWER_MANA);
            if (recentCast)
            {
                // Trinity Updates Mana in intervals of 2s, which is correct
                addvalue = GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT) *  ManaIncreaseRate * 2.00f;
            }
            else
            {
                addvalue = GetFloatValue(PLAYER_FIELD_MOD_MANA_REGEN) * ManaIncreaseRate * 2.00f;
            }
        }   break;
        case POWER_RAGE:                                    // Regenerate rage
        {
            float RageDecreaseRate = sWorld.getConfig(RATE_POWER_RAGE_LOSS);

            int mult = 30;
            const AuraList &auras = GetAurasByType(SPELL_AURA_MOD_POWER_REGEN);
            for (AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            {
                if ((*itr)->GetSpellProto()->Id == 12296)
                {
                    mult = 20;
                    break;
                }
            }
            addvalue = mult * RageDecreaseRate;               // 3 rage by tick
        }   break;
        case POWER_ENERGY:                                  // Regenerate energy (rogue)
            addvalue = 20;
            break;
        case POWER_FOCUS:
        case POWER_HAPPINESS:
            break;
    }

    // Mana regen calculated in Player::UpdateManaRegen()
    // Exist only for POWER_MANA, POWER_ENERGY, POWER_FOCUS auras
    if (power != POWER_MANA)
    {
        AuraList const& ModPowerRegenPCTAuras = GetAurasByType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
        for (AuraList::const_iterator i = ModPowerRegenPCTAuras.begin(); i != ModPowerRegenPCTAuras.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue == power)
                addvalue *= ((*i)->GetModifierValue() + 100) / 100.0f;
    }

    if (power != POWER_RAGE)
    {
        curValue += uint32(addvalue);
        if (curValue > maxValue)
            curValue = maxValue;
    }
    else
    {
        if (curValue <= uint32(addvalue))
            curValue = 0;
        else
            curValue -= uint32(addvalue);
    }
    SetPower(power, curValue);
}

void Player::RegenerateHealth()
{
    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue) return;

    float HealthIncreaseRate = sWorld.getConfig(RATE_HEALTH);

    float addvalue = 0.0f;

    // polymorphed case
    if (IsPolymorphed())
        addvalue = GetMaxHealth()/5;
    // normal regen case (maybe partly in combat case)
    else if (!IsInCombat() || HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
    {
        addvalue = OCTRegenHPPerSpirit()* HealthIncreaseRate;
        if (!IsInCombat())
        {
            AuraList const& mModHealthRegenPct = GetAurasByType(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
            for (AuraList::const_iterator i = mModHealthRegenPct.begin(); i != mModHealthRegenPct.end(); ++i)
                addvalue *= (100.0f + (*i)->GetModifierValue()) / 100.0f;
        }
        else if (HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
            addvalue *= GetTotalAuraModifier(SPELL_AURA_MOD_REGEN_DURING_COMBAT) / 100.0f;

        if (!IsStandState())
            addvalue *= 1.5;
    }

    // always regeneration bonus (including combat)
    addvalue += GetTotalAuraModifier(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT);

    if (addvalue < 0)
        addvalue = 0;

    ModifyHealth(int32(addvalue));
}

bool Player::CanInteractWithNPCs(bool alive) const
{
    if (alive && !isAlive())
        return false;
    if (IsTaxiFlying())
        return false;

    return true;
}

Creature* Player::GetNPCIfCanInteractWith(uint64 guid, uint32 npcflagmask)
{
    // unit checks
    if (!guid)
        return NULL;

    // exist
    Creature *unit = GetMap()->GetCreatureOrPet(guid);
    if (!unit)
        return NULL;

    // player check
    if (!CanInteractWithNPCs(!unit->isSpiritService() && !(unit->GetCreatureInfo()->type_flags & CREATURE_TYPEFLAGS_GHOST)))
        return NULL;

    if (IsHostileTo(unit))
        return NULL;

    // appropriate npc type
    if (npcflagmask && !unit->HasFlag(UNIT_NPC_FLAGS, npcflagmask))
        return NULL;

    // alive or spirit healer
    if (!unit->isAlive() && (!unit->isSpiritService() || isAlive()))
        return NULL;

    // not allow interaction under control
    if (unit->GetCharmerGUID())
        return NULL;

    // not enemy
    if (unit->IsHostileTo(this))
        return NULL;

    // not unfriendly
    FactionTemplateEntry const* factionTemplate = sFactionTemplateStore.LookupEntry(unit->getFaction());
    if (factionTemplate)
    {
        // check forced reactions if exist
        if (ReputationRank const * rank = m_reputationMgr.GetForcedRankIfAny(factionTemplate))
        {
            if ((*rank) <= REP_UNFRIENDLY)
                return NULL;
        }
        else    // normal case
        {
            FactionEntry const* faction = sFactionStore.LookupEntry(factionTemplate->faction);
            if (faction->reputationListID >= 0 && m_reputationMgr.GetRank(faction) <= REP_UNFRIENDLY)
                return NULL;
        }
    }

    // not too far
    if (!unit->IsWithinDistInMap(this, INTERACTION_DISTANCE))
        return NULL;

    return unit;
}

GameObject* Player::GetGameObjectIfCanInteractWith(uint64 guid, GameobjectTypes type) const
{
    if (GameObject *gObject = GetMap()->GetGameObject(guid))
    {
        if (gObject->GetGoType() == type)
        {
            float maxdist;
            switch (type)
            {
                case GAMEOBJECT_TYPE_GUILD_BANK:
                case GAMEOBJECT_TYPE_MAILBOX:
                    maxdist = 10.0f;
                    break;
                case GAMEOBJECT_TYPE_FISHINGHOLE:
                    maxdist = 20.0f + CONTACT_DISTANCE;       // max spell range
                    break;
                default:
                    maxdist = INTERACTION_DISTANCE;
                    break;
            }

            if (gObject->IsWithinDistInMap(this, maxdist))
                return gObject;
        }
    }
    return NULL;
}

void Player::SetSwimming(bool apply)
{
    if (m_isSwimming == apply)
        return;

    //define player in water by opcodes
    //move player's guid into HateOfflineList of those mobs
    //which can't swim and move guid back into ThreatList when
    //on surface.
    //TODO: exist also swimming mobs, and function must be symmetric to enter/leave water
    m_isSwimming = apply;

    // remove auras that need water/land
    RemoveAurasWithInterruptFlags(apply ? AURA_INTERRUPT_FLAG_NOT_ABOVEWATER : AURA_INTERRUPT_FLAG_NOT_UNDERWATER);

    getHostileRefManager().updateThreatTables();
}

void Player::SetGameMaster(bool on)
{
    if (on)
    {
        m_ExtraFlags |= PLAYER_EXTRA_GM_ON;
        setFaction(35);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);

        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
        ResetContestedPvP();

        CombatStop();
    }
    else
    {
        m_ExtraFlags &= ~ PLAYER_EXTRA_GM_ON;
        RestoreFaction();
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);

        // restore FFA PvP Server state
        if (pvpInfo.inHostileArea)
            UpdatePvP(true,true);
        else
            UpdatePvP(false);

        // restore FFA PvP area state, remove not allowed for GM mounts
        UpdateArea(m_areaUpdateId);
    }

    getHostileRefManager().setOnlineOfflineState(!on);
    UpdateVisibilityAndView();
}

void Player::SetGMVisible(bool on)
{
    if (on)
    {
        m_ExtraFlags &= ~PLAYER_EXTRA_GM_INVISIBLE;         //remove flag

        // Reapply stealth/invisibility if active or show if not any
        if (HasAuraType(SPELL_AURA_MOD_STEALTH))
            SetVisibility(VISIBILITY_GROUP_STEALTH);
        //else if (HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
        //    SetVisibility(VISIBILITY_GROUP_INVISIBILITY);
        else
            SetVisibility(VISIBILITY_ON);
    }
    else
    {
        m_ExtraFlags |= PLAYER_EXTRA_GM_INVISIBLE;          //add flag
        SetGameMaster(true);

        SetVisibility(VISIBILITY_OFF);
    }
}

bool Player::IsGroupVisiblefor (Player* p) const
{
    /*switch (sWorld.getConfig(CONFIG_GROUP_VISIBILITY))
    {
        default: return IsInSameGroupWith(p);
        case 1:  */return IsInSameRaidWith(p);/*
        case 2:  return GetTeam()==p->GetTeam();
    }*/
}

bool Player::IsInSameGroupWith(Player const* p) const
{
    return  p==this || GetGroup() != NULL &&
        GetGroup() == p->GetGroup() &&
        GetGroup()->SameSubGroup((Player*)this, (Player*)p);
}

///- If the player is invited, remove him. If the group if then only 1 person, disband the group.
/// \todo Shouldn't we also check if there is no other invitees before disbanding the group?
void Player::UninviteFromGroup()
{
    Group* group = GetGroupInvite();
    if (!group)
        return;

    group->RemoveInvite(this);

    if (group->GetMembersCount() <= 1)                   // group has just 1 member => disband
    {
        if (group->IsCreated())
        {
            group->Disband(true);
            sObjectMgr.RemoveGroup(group);
        }
        else
            group->RemoveAllInvites();

        delete group;
    }
}

void Player::RemoveFromGroup(Group* group, uint64 guid)
{
    if (group)
    {
        if (group->RemoveMember(guid, 0) <= 1)
        {
            // group->Disband(); already disbanded in RemoveMember
            sObjectMgr.RemoveGroup(group);
            delete group;
            // removemember sets the player's group pointer to NULL
        }
    }
}

void Player::SendLogXPGain(uint32 GivenXP, Unit* victim, uint32 BonusXP, bool ReferAFriend)
{
    WorldPacket data(SMSG_LOG_XPGAIN, 21);
    data << uint64(victim ? victim->GetGUID() : 0);         // guid
    data << uint32(GivenXP + BonusXP);                         // given experience
    data << uint8(victim ? 0 : 1);                          // 00-kill_xp type, 01-non_kill_xp type
    if (victim)
    {
        data << uint32(GivenXP);                            // experience without rested bonus
        data << float(1);                                   // 1 - none 0 - 100% group bonus output
    }
    data << (ReferAFriend ? 1 : 0);                         // Refer-A-Friend State
    SendPacketToSelf(&data);
}

void Player::GiveXP(uint32 xp, Unit* victim)
{
    if (xp < 1)
        return;

    if (!isAlive())
        return;

    if (IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_FROZEN))
        return;

    uint32 level = GetLevel();

    // XP to money conversion processed in Player::RewardQuest
    if (level >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        return;

    // handle SPELL_AURA_MOD_XP_PCT auras
    Unit::AuraList const& ModXPPctAuras = GetAurasByType(SPELL_AURA_MOD_XP_PCT);
    for (Unit::AuraList::const_iterator i = ModXPPctAuras.begin();i != ModXPPctAuras.end(); ++i)
        xp = uint32(xp*(1.0f + (*i)->GetModifierValue() / 100.0f));

    uint32 bonus_xp = 0;
    //if (uint32 goodEntry = sWorld.getConfig(CONFIG_XP_RATE_MODIFY_ITEM_ENTRY))
    //{
    //    if (!IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1))
    //    {
    //        for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    //        {
    //            Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
    //            if (pItem && pItem->GetEntry() == goodEntry)
    //            {
    //                bonus_xp = uint32(xp*sWorld.getConfig(CONFIG_XP_RATE_MODIFY_ITEM_PCT) / 100.0f);
    //                break;
    //            }
    //        }
    //    }
    //}

    // RAF can already be assigned or not yet assigned
	bool RAF_active = GetByteValue(PLAYER_BYTES_2, 3) == REST_STATE_RAF;
    if (GetSession()->IsRaf() && CheckRAFConditions_XP() && !IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1))
    {
        bonus_xp += xp * sWorld.getConfig(CONFIG_FLOAT_RATE_RAF_XP);
        
		if (!RAF_active)
        {
            SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_RAF);
            RAF_active = true;
        }
    }
    else
    {
        // XP resting bonus for kill, doesn't get used when RAF is active
        bonus_xp += victim ? GetXPRestBonus(xp) : 0;

		if (RAF_active)
		{
			SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_NORMAL);
			RAF_active = false;
		}
    }

    SendLogXPGain(xp, victim, bonus_xp, RAF_active);

    uint32 curXP = GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 newXP = curXP + xp + bonus_xp;

    while (newXP >= nextLvlXP && level < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        newXP -= nextLvlXP;

        GiveLevel(level + 1);
        level = GetLevel();
        // Refer-A-Friend. It is here and not in ::GiveLevel because GiveLevel is used in debug commands
        if (GetSession()->IsRaf())
        {
            if (level <= sWorld.getConfig(CONFIG_UINT32_RAF_MAXGRANTLEVEL))
            {
                if (sWorld.getConfig(CONFIG_FLOAT_RATE_RAF_LEVELPERLEVEL) < 1.0f)
                {
                    if (level%uint8(1.0f / sWorld.getConfig(CONFIG_FLOAT_RATE_RAF_LEVELPERLEVEL)) == 0)
                        ChangeGrantableLevels(1);
                }
                else
                    ChangeGrantableLevels(uint8(sWorld.getConfig(CONFIG_FLOAT_RATE_RAF_LEVELPERLEVEL)));
            }
        }

        nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);

        LevelReached(level);
    }

    SetUInt32Value(PLAYER_XP, newXP);
}

// Update player to next level
// Current player experience not update (must be update by caller)
void Player::GiveLevel(uint32 level)
{
    if (level == GetLevel())
        return;

    PlayerLevelInfo info;
    sObjectMgr.GetPlayerLevelInfo(GetRace(),GetClass(),level,&info);

    PlayerClassLevelInfo classInfo;
    sObjectMgr.GetPlayerClassLevelInfo(GetClass(),level,&classInfo);

    // send levelup info to client
    WorldPacket data(SMSG_LEVELUP_INFO, (4+4+MAX_POWERS*4+MAX_STATS*4));
    data << uint32(level);
    data << uint32(int32(classInfo.basehealth) - int32(GetCreateHealth()));
    // for (int i = 0; i < MAX_POWERS; ++i)                  // Powers loop (0-6)
    data << uint32(int32(classInfo.basemana)   - int32(GetCreateMana()));
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    // end for
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)          // Stats loop (0-4)
        data << uint32(int32(info.stats[i]) - GetCreateStat(Stats(i)));

    SendPacketToSelf(&data);

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, Hellground::XP::xp_to_level(level));

    //update level, max level of skills
    if (GetLevel()!= level)
        m_Played_time[1] = 0;                               // Level Played Time reset
    SetLevel(level);
    UpdateSkillsForLevel ();

    // save base values (bonuses already included in stored stats
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetCreateStat(Stats(i), info.stats[i]);

    SetCreateHealth(classInfo.basehealth);
    SetCreateMana(classInfo.basemana);

    InitTalentForLevel();
    InitTaxiNodesForLevel();

    UpdateAllStats();

    if (sWorld.getConfig(CONFIG_ALWAYS_MAX_WEAPON_SKILL)) // Max weapon skill when leveling up
        SetWeaponSkillsToMax();

    // set current level health and mana/energy to maximum after applying all mods.
    SetHealth(GetMaxHealth());
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
    if (GetPower(POWER_RAGE) > GetMaxPower(POWER_RAGE))
        SetPower(POWER_RAGE, GetMaxPower(POWER_RAGE));
    SetPower(POWER_FOCUS, 0);
    SetPower(POWER_HAPPINESS, 0);

    // give level to summoned pet
    Pet* pet = GetPet();
    if (pet && pet->getPetType()==SUMMON_PET)
        pet->GivePetLevel(level);

    if (level == 60 && sWorld.isEasyRealm())
    {
        AreaTableEntry const* zone = GetAreaEntryByAreaID(GetZoneId());
        if (!zone)
        {
            sLog.outLog(LOG_DEFAULT, "No zone in givelevel on getting level 60 realm x100");
            return;
        }
        pvpInfo.inHostileArea =
        (GetMap()->IsDungeon() || GetTeam() == ALLIANCE && zone->team == AREATEAM_HORDE ||
        GetTeam() == HORDE    && zone->team == AREATEAM_ALLY  ||
        sWorld.IsPvPRealm()   && zone->team == AREATEAM_NONE && 
        (!sWorld.IsFFAPvPRealm() || !(zone->flags & AREA_FLAG_SANCTUARY)) ||
        InBattleGroundOrArena()) && !sWorld.isPvPArea(GetCachedArea());         // overwrite for battlegrounds, maybe batter some zone flags but current known not 100% fit to this
        if (pvpInfo.inHostileArea)                               // in hostile area
        {
            if (!IsPvP() || pvpInfo.endTimer != 0)
                UpdatePvP(true, true);
        }
        else if (!(zone->flags & AREA_FLAG_SANCTUARY))             // in friendly area
        {
            if (IsPvP() && !HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_IN_PVP) && pvpInfo.endTimer == 0)
                pvpInfo.endTimer = time(0);                     // start toggle-off
        }
    }

    if (sWorld.isEasyRealm())
        LearnAllSpells();

    // recalculate mods that are not recalculated due to stat gain
    ApplyRatingMod(CR_HIT_MELEE, 0, true);
    ApplyRatingMod(CR_HIT_RANGED, 0, true);
    ApplyRatingMod(CR_HIT_SPELL, 0, true);
    ApplyRatingMod(CR_HASTE_MELEE, 0, true);
    ApplyRatingMod(CR_HASTE_RANGED, 0, true);
    ApplyRatingMod(CR_HASTE_SPELL, 0, true);
}

void Player::UpdateFreeTalentPoints(bool resetIfNeed)
{
    uint32 level = GetLevel();
    // talents base at level diff (talents = level - 9 but some can be used already)
    if (level < 10)
    {
        // Remove all talent points
        if (m_usedTalentCount > 0)                           // Free any used talents
        {
            if (resetIfNeed)
                resetTalents(true);
            SetFreeTalentPoints(0);
        }
    }
    else
    {
        uint32 talentPointsForLevel = CalculateTalentsPoints();
        // if used more that have then reset
        if (m_usedTalentCount > talentPointsForLevel)
        {
            if (resetIfNeed && !GetSession()->HasPermissions(PERM_ADM))
                resetTalents(true);
            else
                SetFreeTalentPoints(0);
        }
        // else update amount of free points
        else
            SetFreeTalentPoints(talentPointsForLevel-m_usedTalentCount);
    }
}

void Player::InitTalentForLevel()
{
    UpdateFreeTalentPoints();
}

void Player::InitStatsForLevel(bool reapplyMods)
{
    if (reapplyMods)                                         //reapply stats values only on .reset stats (level) command
        _RemoveAllStatBonuses();

    PlayerClassLevelInfo classInfo;
    sObjectMgr.GetPlayerClassLevelInfo(GetClass(),GetLevel(),&classInfo);

    PlayerLevelInfo info;
    sObjectMgr.GetPlayerLevelInfo(GetRace(),GetClass(),GetLevel(),&info);

    SetUInt32Value(PLAYER_FIELD_MAX_LEVEL, sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL));
    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, Hellground::XP::xp_to_level(GetLevel()));

    UpdateSkillsForLevel ();

    // set default cast time multiplier
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    // reset size before reapply auras
    SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);

    // save base values (bonuses already included in stored stats
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetCreateStat(Stats(i), info.stats[i]);

    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetStat(Stats(i), info.stats[i]);

    SetCreateHealth(classInfo.basehealth);

    //set create powers
    SetCreateMana(classInfo.basemana);

    SetArmor(int32(m_createStats[STAT_AGILITY]*2));

    InitStatBuffMods();

    //reset rating fields values
    for (uint16 index = PLAYER_FIELD_COMBAT_RATING_1; index < PLAYER_FIELD_COMBAT_RATING_1 + MAX_COMBAT_RATING; ++index)
        SetUInt32Value(index, 0);

    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
    {
        SetInt32Value(UNIT_FIELD_POWER_COST_MODIFIER+i, 0);
        SetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+i, 0.0f);
    }

    SetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS,0);
    for (int i = 0; i < 7; i++)
    {
        SetInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG+i, 0);
        SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS+i, 0);
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT+i, 1.00f);
    }

    //reset attack power, damage and attack speed fields
    SetFloatValue(UNIT_FIELD_BASEATTACKTIME, 2000.0f);
    SetFloatValue(UNIT_FIELD_BASEATTACKTIME + 1, 2000.0f); // offhand attack time
    SetFloatValue(UNIT_FIELD_RANGEDATTACKTIME, 2000.0f);

    SetFloatValue(UNIT_FIELD_MINDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, 0.0f);

    SetInt32Value(UNIT_FIELD_ATTACK_POWER,            0);
    SetInt32Value(UNIT_FIELD_ATTACK_POWER_MODS,       0);
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER,0.0f);
    SetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER,     0);
    SetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS,0);
    SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER,0.0f);

    // Base crit values (will be recalculated in UpdateAllStats() at loading and in _ApplyAllStatBonuses() at reset
    SetFloatValue(PLAYER_CRIT_PERCENTAGE,0.0f);
    SetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE,0.0f);
    SetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE,0.0f);

    // Init spell schools (will be recalculated in UpdateAllStats() at loading and in _ApplyAllStatBonuses() at reset
    for (uint8 i = 0; i < 7; ++i)
        SetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1+i, 0.0f);

    SetFloatValue(PLAYER_PARRY_PERCENTAGE, 0.0f);
    SetFloatValue(PLAYER_BLOCK_PERCENTAGE, 0.0f);
    SetUInt32Value(PLAYER_SHIELD_BLOCK, 0);

    // Dodge percentage
    SetFloatValue(PLAYER_DODGE_PERCENTAGE, 0.0f);

    // set armor (resistance 0) to original value (create_agility*2)
    SetArmor(int32(m_createStats[STAT_AGILITY]*2));
    SetResistanceBuffMods(SpellSchools(0), true, 0.0f);
    SetResistanceBuffMods(SpellSchools(0), false, 0.0f);
    // set other resistance to original value (0)
    for (int i = 1; i < MAX_SPELL_SCHOOL; i++)
    {
        SetResistance(SpellSchools(i), 0);
        SetResistanceBuffMods(SpellSchools(i), true, 0.0f);
        SetResistanceBuffMods(SpellSchools(i), false, 0.0f);
    }

    SetUInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE,0);
    SetUInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE,0);

    // Init data for form but skip reapply item mods for form
    InitDataForForm(reapplyMods);

    // save new stats
    for (int i = POWER_MANA; i < MAX_POWERS; i++)
        SetMaxPower(Powers(i),  uint32(GetCreatePowers(Powers(i))));

    SetMaxHealth(classInfo.basehealth);                     // stamina bonus will applied later

    // cleanup mounted state (it will set correctly at aura loading if player saved at mount.
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);

    // cleanup unit flags (will be re-applied if need at aura load).
    SetNonAttackableFlag(
        UNIT_FLAG_UNK_0 | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_ATTACKABLE_1 |
        UNIT_FLAG_NOT_ATTACKABLE_2 | UNIT_FLAG_PASSIVE | UNIT_FLAG_LOOTING |
        UNIT_FLAG_PET_IN_COMBAT | UNIT_FLAG_SILENCED | UNIT_FLAG_PACIFIED |
        UNIT_FLAG_DISABLE_ROTATE | UNIT_FLAG_IN_COMBAT | UNIT_FLAG_DISARMED |
        UNIT_FLAG_CONFUSED | UNIT_FLAG_FLEEING | UNIT_FLAG_PLAYER_CONTROLLED |
        UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_SKINNABLE | UNIT_FLAG_MOUNT |
        UNIT_FLAG_TAXI_FLIGHT | UNIT_FLAG_UNKNOWN6
        , false);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);   // must be set

    // cleanup player flags (will be re-applied if need at aura load), to avoid have ghost flag without ghost aura, for example.
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK | PLAYER_FLAGS_DND | PLAYER_FLAGS_GM | PLAYER_FLAGS_GHOST | PLAYER_FLAGS_FFA_PVP);

    SetByteValue(UNIT_FIELD_BYTES_1, 2, 0x00);              // one form stealth modified bytes

    // restore if need some important flags
    SetUInt32Value(PLAYER_FIELD_BYTES2, 0);                // flags empty by default

    if (reapplyMods)                                         //reapply stats values only on .reset stats (level) command
        _ApplyAllStatBonuses();

    // set current level health and mana/energy to maximum after applying all mods.
    SetHealth(GetMaxHealth());
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
    if (GetPower(POWER_RAGE) > GetMaxPower(POWER_RAGE))
        SetPower(POWER_RAGE, GetMaxPower(POWER_RAGE));
    SetPower(POWER_FOCUS, 0);
    SetPower(POWER_HAPPINESS, 0);
}

void Player::SendInitialSpells()
{
    uint16 spellCount = 0;

    WorldPacket data(SMSG_INITIAL_SPELLS, (1+2+4*m_spells.size()+2+m_spellCooldowns.size()*(2+2+2+4+4)+m_spellItemCooldowns.size()*(2+2+2+4+4)));
    data << uint8(0);

    size_t countPos = data.wpos();
    data << uint16(spellCount);                             // spell count placeholder

    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second.state == PLAYERSPELL_REMOVED)
            continue;

        if (!itr->second.active || itr->second.disabled)
            continue;

        data << uint16(itr->first);
        data << uint16(0);                                  // it's not slot id

        spellCount +=1;
    }

    data.put<uint16>(countPos,spellCount);                  // write real count value

    uint16 spellCooldowns = m_spellCooldowns.size() + m_spellItemCooldowns.size();
    data << uint16(spellCooldowns);
    for (SpellCooldowns::const_iterator itr=m_spellCooldowns.begin(); itr!=m_spellCooldowns.end(); ++itr)
    {
        SpellEntry const *sEntry = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
        if (!sEntry)
            continue;

        data << uint16(itr->first);

        time_t cooldown = 0;
        time_t curTime = time(NULL);
        if (itr->second > curTime)
            cooldown = (itr->second-curTime)*1000;

        data << uint16(0);                 // cast item id
        data << uint16(sEntry->Category);                   // spell category
        if (sEntry->Category)                                // may be wrong, but anyway better than nothing...
        {
            data << uint32(0);                              // cooldown
            data << uint32(cooldown);                       // category cooldown
        }
        else
        {
            data << uint32(cooldown);                       // cooldown
            data << uint32(0);                              // category cooldown
        }
    }

    for (SpellItemCooldowns::const_iterator itr=m_spellItemCooldowns.begin(); itr!=m_spellItemCooldowns.end(); ++itr)
    {
        SpellEntry const *sEntry = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
        if (!sEntry)
            continue;

        data << uint16(itr->first);

        time_t cooldown = 0;
        time_t curTime = time(NULL);
        if (itr->second.end > curTime)
            cooldown = (itr->second.end-curTime)*1000;

        data << uint16(itr->second.itemid);                 // cast item id
        
        data << uint16(itr->second.category);                   // spell category
        if (itr->second.category)                                // may be wrong, but anyway better than nothing...
        {
            data << uint32(0);                              // cooldown
            data << uint32(cooldown);                       // category cooldown
        }
        else
        {
            data << uint32(cooldown);                       // cooldown
            data << uint32(0);                              // category cooldown
        }
    }

    SendPacketToSelf(&data);

    sLog.outDetail("CHARACTER: Sent Initial Spells");
}

void Player::RemoveMail(uint32 id)
{
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end();++itr)
    {
        if ((*itr)->messageID == id)
        {
            //do not delete item, because Player::removeMail() is called when returning mail to sender.
            m_mail.erase(itr);
            return;
        }
    }
}

void Player::SendMailResult(uint32 mailId, uint32 mailAction, uint32 mailError, uint32 equipError, uint32 item_guid, uint32 item_count)
{
    WorldPacket data(SMSG_SEND_MAIL_RESULT, (4+4+4+(mailError == MAIL_ERR_EQUIP_ERROR?4:(mailAction == MAIL_ITEM_TAKEN?4+4:0))));
    data << (uint32) mailId;
    data << (uint32) mailAction;
    data << (uint32) mailError;
    if (mailError == MAIL_ERR_EQUIP_ERROR)
        data << (uint32) equipError;
    else if (mailAction == MAIL_ITEM_TAKEN)
    {
        data << (uint32) item_guid;                         // item guid low?
        data << (uint32) item_count;                        // item count?
    }
    SendPacketToSelf(&data);
}

void Player::SendNewMail()
{
    // deliver undelivered mail
    WorldPacket data(SMSG_RECEIVED_MAIL, 4);
    data << (uint32) 0;
    SendPacketToSelf(&data);
}

void Player::UpdateNextMailTimeAndUnreads()
{
    // calculate next delivery time (min. from non-delivered mails
    // and recalculate unReadMail
    time_t cTime = time(NULL);
    m_nextMailDelivereTime = 0;
    unReadMails = 0;
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        if ((*itr)->deliver_time > cTime)
        {
            if (!m_nextMailDelivereTime || m_nextMailDelivereTime > (*itr)->deliver_time)
                m_nextMailDelivereTime = (*itr)->deliver_time;
        }
        else if (((*itr)->checked & MAIL_CHECK_MASK_READ) == 0)
            ++unReadMails;
    }
}

void Player::AddNewMailDeliverTime(time_t deliver_time)
{
    if (deliver_time <= time(NULL))                          // ready now
    {
        ++unReadMails;
        SendNewMail();
    }
    else                                                    // not ready and no have ready mails
    {
        if (!m_nextMailDelivereTime || m_nextMailDelivereTime > deliver_time)
            m_nextMailDelivereTime =  deliver_time;
    }
}

bool Player::addSpell(uint32 spell_id, bool active, bool learning, bool loading, uint16 slot_id, bool disabled)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
    if (!spellInfo)
    {
        // do character spell book cleanup (all characters)
        if (loading && !learning)                            // spell load case
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Player::addSpell: Non-existed in SpellStore spell #%u request, deleting for all characters in `character_spell`.",spell_id);
            RealmDataDatabase.PExecute("DELETE FROM character_spell WHERE spell = '%u'",spell_id);
        }
        else
            sLog.outLog(LOG_DEFAULT, "ERROR: Player::addSpell: Non-existed in SpellStore spell #%u request.",spell_id);

        return false;
    }

    if (!SpellMgr::IsSpellValid(spellInfo,this,false))
    {
        // do character spell book cleanup (all characters)
        if (loading && !learning)                            // spell load case
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Player::addSpell: Broken spell #%u learning not allowed, deleting for all characters in `character_spell`.",spell_id);
            RealmDataDatabase.PExecute("DELETE FROM character_spell WHERE spell = '%u'",spell_id);
        }
        else
            sLog.outLog(LOG_DEFAULT, "ERROR: Player::addSpell: Broken spell #%u learning not allowed.",spell_id);

        return false;
    }

    PlayerSpellState state = learning ? PLAYERSPELL_NEW : PLAYERSPELL_UNCHANGED;

    bool disabled_case = false;
    bool superceded_old = false;

    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        // update active state for known spell
        if (itr->second.active != active && itr->second.state != PLAYERSPELL_REMOVED && !itr->second.disabled)
        {
            itr->second.active = active;

            // loading && !learning == explicitly load from DB and then exist in it already and set correctly
            if (loading && !learning)
                itr->second.state = PLAYERSPELL_UNCHANGED;
            else if (itr->second.state != PLAYERSPELL_NEW)
                itr->second.state = PLAYERSPELL_CHANGED;

            if (!active)
            {
                WorldPacket data(SMSG_REMOVED_SPELL, 4);
                data << uint16(spell_id);
                SendPacketToSelf(&data);
            }
            return active;                                  // learn (show in spell book if active now)
        }

        if (itr->second.disabled != disabled && itr->second.state != PLAYERSPELL_REMOVED)
        {
            if (itr->second.state != PLAYERSPELL_NEW)
                itr->second.state = PLAYERSPELL_CHANGED;

            itr->second.disabled = disabled;

            if (disabled)
                return false;

            disabled_case = true;
        }
        else switch (itr->second.state)
        {
            case PLAYERSPELL_UNCHANGED:                     // known saved spell
                return false;
            case PLAYERSPELL_REMOVED:                       // re-learning removed not saved spell
            {
                m_spells.erase(itr);
                state = PLAYERSPELL_CHANGED;
                break;                                      // need re-add
            }
            default:                                        // known not saved yet spell (new or modified)
            {
                // can be in case spell loading but learned at some previous spell loading
                if (loading && !learning)
                    itr->second.state = PLAYERSPELL_UNCHANGED;

                return false;
            }
        }
    }

    TalentSpellPos const* talentPos = GetTalentSpellPos(spell_id);

    if (!disabled_case) // skip new spell adding if spell already known (disabled spells case)
    {
        // talent: unlearn all other talent ranks (high and low)
        if (talentPos)
        {
            if (TalentEntry const *talentInfo = sTalentStore.LookupEntry(talentPos->talent_id))
            {
                for (int i=0; i <5; ++i)
                {
                    // skip learning spell and no rank spell case
                    uint32 rankSpellId = talentInfo->RankID[i];
                    if (!rankSpellId || rankSpellId==spell_id)
                        continue;

                    // skip unknown ranks
                    if (!HasSpell(rankSpellId))
                        continue;

                    removeSpell(rankSpellId);
                }
            }
        }
        // non talent spell: learn low ranks (recursive call)
        else if (uint32 prev_spell = sSpellMgr.GetPrevSpellInChain(spell_id))
        {
            if (loading)                                     // at spells loading, no output, but allow save
                addSpell(prev_spell,active,true,loading,SPELL_WITHOUT_SLOT_ID,disabled);
            else                                            // at normal learning
                learnSpell(prev_spell);
        }

        PlayerSpell newspell;
        newspell.active = active;
        newspell.state = state;
        newspell.disabled = disabled;

        // replace spells in action bars and spell book to bigger rank if only one spell rank must be accessible
        if (newspell.active && !newspell.disabled && !SpellMgr::canStackSpellRanks(spellInfo) && sSpellMgr.GetSpellRank(spellInfo->Id) != 0)
        {
            for (PlayerSpellMap::iterator itr2 = m_spells.begin(); itr2 != m_spells.end(); ++itr2)
            {
                if (itr2->second.state == PLAYERSPELL_REMOVED) continue;
                SpellEntry const *i_spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr2->first);
                if (!i_spellInfo)
                    continue;

                if (sSpellMgr.IsRankSpellDueToSpell(spellInfo, itr2->first))
                {
                    if (itr2->second.active)
                    {
                        if (sSpellMgr.IsHighRankOfSpell(spell_id, itr2->first))
                        {
                            if (!loading)                    // not send spell (re-/over-)learn packets at loading
                            {
                                WorldPacket data(SMSG_SUPERCEDED_SPELL, (4));
                                data << uint16(itr2->first);
                                data << uint16(spell_id);
                                SendPacketToSelf(&data);
                            }

                            // mark old spell as disable (SMSG_SUPERCEDED_SPELL replace it in client by new)
                            itr2->second.active = false;
                            itr2->second.state = PLAYERSPELL_CHANGED;
                            superceded_old = true;          // new spell replace old in action bars and spell book.
                        }
                        else if (sSpellMgr.IsHighRankOfSpell(itr2->first, spell_id))
                        {
                            if (!loading)                    // not send spell (re-/over-)learn packets at loading
                            {
                                WorldPacket data(SMSG_SUPERCEDED_SPELL, (4));
                                data << uint16(spell_id);
                                data << uint16(itr2->first);
                                SendPacketToSelf(&data);
                            }

                            // mark new spell as disable (not learned yet for client and will not learned)
                            newspell.active = false;
                            if (newspell.state != PLAYERSPELL_NEW)
                                newspell.state = PLAYERSPELL_CHANGED;
                        }
                    }
                }
            }
        }

        uint16 tmpslot=slot_id;

        if (tmpslot == SPELL_WITHOUT_SLOT_ID)
        {
            uint16 maxid = 0;
            for (PlayerSpellMap::iterator itr2 = m_spells.begin(); itr2 != m_spells.end(); ++itr2)
            {
                if (itr2->second.state == PLAYERSPELL_REMOVED)
                    continue;
                if (itr2->second.slotId > maxid)
                    maxid = itr2->second.slotId;
            }
            tmpslot = maxid + 1;
        }

        newspell.slotId = tmpslot;
        m_spells[spell_id] = newspell;

        // return false if spell disabled
        if (newspell.disabled)
            return false;
    }

    if (talentPos)
    {
        // update used talent points count
        m_usedTalentCount += GetTalentSpellCost(talentPos);
        UpdateFreeTalentPoints(false);
    }

    // update free primary prof.points (if any, can be none in case GM .learn prof. learning)
    if (uint32 freeProfs = GetFreePrimaryProfessionPoints())
    {
        if (sSpellMgr.IsPrimaryProfessionFirstRankSpell(spell_id))
            SetFreePrimaryProfessions(freeProfs-1);
    }

    // cast talents with SPELL_EFFECT_LEARN_SPELL (other dependent spells will learned later as not auto-learned)
    // note: all spells with SPELL_EFFECT_LEARN_SPELL isn't passive
    if (talentPos && spellInfo->HasEffect(SPELL_EFFECT_LEARN_SPELL))
    {
        // ignore stance requirement for talent learn spell (stance set for spell only for client spell description show)
        CastSpell(this, spell_id, true);
    }
    // also cast passive spells (including all talents without SPELL_EFFECT_LEARN_SPELL) with additional checks
    else if (SpellMgr::IsPassiveSpell(spell_id))
    {
        // if spell doesn't require a stance or the player is in the required stance
        if ((!spellInfo->Stances &&
            spell_id != 5420 && spell_id != 5419 && spell_id != 7376 &&
            spell_id != 7381 && spell_id != 21156 && spell_id != 21009 &&
            spell_id != 21178 && spell_id != 33948 && spell_id != 40121) ||
            m_form != 0 && (spellInfo->Stances & (1<<(m_form-1))) ||
            (spell_id ==  5420 && m_form == FORM_TREE) ||
            (spell_id ==  5419 && m_form == FORM_TRAVEL) ||
            (spell_id ==  7376 && m_form == FORM_DEFENSIVESTANCE) ||
            (spell_id ==  7381 && m_form == FORM_BERSERKERSTANCE) ||
            (spell_id == 21156 && m_form == FORM_BATTLESTANCE)||
            (spell_id == 21178 && (m_form == FORM_BEAR || m_form == FORM_DIREBEAR)) ||
            (spell_id == 33948 && m_form == FORM_FLIGHT) ||
            (spell_id == 40121 && m_form == FORM_FLIGHT_EPIC))
                                                            //Check CasterAuraStates
            if (!spellInfo->CasterAuraState || HasAuraState(AuraState(spellInfo->CasterAuraState)))
                CastSpell(this, spell_id, true);
    }
    else if (spellInfo->HasEffect(SPELL_EFFECT_SKILL_STEP))
    {
        CastSpell(this, spell_id, true);
        return false;
    }

    // add dependent skills
    uint16 maxskill     = GetMaxSkillValueForLevel();

    SpellLearnSkillNode const* spellLearnSkill = sSpellMgr.GetSpellLearnSkill(spell_id);

    if (spellLearnSkill)
    {
        uint32 skill_value = GetPureSkillValue(spellLearnSkill->skill);
        uint32 skill_max_value = GetPureMaxSkillValue(spellLearnSkill->skill);

        if (skill_value < spellLearnSkill->value)
            skill_value = spellLearnSkill->value;

        uint32 new_skill_max_value = spellLearnSkill->maxvalue == 0 ? maxskill : spellLearnSkill->maxvalue;

        if (skill_max_value < new_skill_max_value)
            skill_max_value =  new_skill_max_value;

        uint32 skill_id = spellLearnSkill->skill;

        SetSkill(spellLearnSkill->skill, skill_value, skill_max_value);
    }
    else
    {
        // not ranked skills
        SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(spell_id);
        SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(spell_id);

        for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(_spell_idx->second->skillId);
            if (!pSkill)
                continue;

            if (HasSkill(pSkill->id))
                continue;

            if (_spell_idx->second->learnOnGetSkill == ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL ||
                // poison special case, not have ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL
                pSkill->id==SKILL_POISONS && _spell_idx->second->max_value==0 ||
                // lockpicking special case, not have ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL
                pSkill->id==SKILL_LOCKPICKING && _spell_idx->second->max_value==0)
            {
                switch (GetSkillRangeType(pSkill,_spell_idx->second->racemask!=0))
                {
                    case SKILL_RANGE_LANGUAGE:
                        SetSkill(pSkill->id, 300, 300);
                        break;
                    case SKILL_RANGE_LEVEL:
                        if (sWorld.getConfig(CONFIG_ALWAYS_MAX_WEAPON_SKILL))
                            SetSkill(pSkill->id, GetMaxSkillValueForLevel(), GetMaxSkillValueForLevel());
                        else
                            SetSkill(pSkill->id, 1, GetMaxSkillValueForLevel());
                        break;
                    case SKILL_RANGE_MONO:
                        SetSkill(pSkill->id, 1, 1);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // learn dependent spells
    SpellLearnSpellMap::const_iterator spell_begin = sSpellMgr.GetBeginSpellLearnSpell(spell_id);
    SpellLearnSpellMap::const_iterator spell_end   = sSpellMgr.GetEndSpellLearnSpell(spell_id);

    for (SpellLearnSpellMap::const_iterator itr2 = spell_begin; itr2 != spell_end; ++itr2)
    {
        if (!itr2->second.autoLearned)
        {
            if (loading)                                     // at spells loading, no output, but allow save
                addSpell(itr2->second.spell,true,true,loading);
            else                                            // at normal learning
                learnSpell(itr2->second.spell);
        }
    }

    // return true (for send learn packet) only if spell active (in case ranked spells) and not replace old spell
    return active && !disabled && !superceded_old;
}

void Player::learnSpell(uint32 spell_id)
{
    PlayerSpellMap::iterator itr = m_spells.find(spell_id);

    bool disabled = (itr != m_spells.end()) ? itr->second.disabled : false;
    bool active = disabled ? itr->second.active : true;

    bool learning = addSpell(spell_id,active);

    // max profession skills to 300 on x100
    if (sWorld.isEasyRealm())
        ProfessionSkillUpgradeOnLearn(spell_id, 300);

    // learn all disabled higher ranks (recursive)
    SpellChainNode const* node = sSpellMgr.GetSpellChainNode(spell_id);
    if (node)
    {
        PlayerSpellMap::iterator iter = m_spells.find(node->next);
        if (disabled && iter != m_spells.end() && iter->second.disabled)
            learnSpell(node->next);
    }

    // prevent duplicated entires in spell book
    if (!learning)
        return;

	// hack for paladin seals
	if (spell_id == 31801 && !HasSpell(31892))
		learnSpell(31892);
	else if (spell_id == 31892 && !HasSpell(31801))
		learnSpell(31801);

    WorldPacket data(SMSG_LEARNED_SPELL, 4);
    data << uint32(spell_id);
    SendPacketToSelf(&data);
}

void Player::removeSpell(uint32 spell_id, bool disabled)
{    
    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return;

    if (itr->second.state == PLAYERSPELL_REMOVED || disabled && itr->second.disabled)
        return;

    // unlearn non talent higher ranks (recursive)
    SpellChainNode const* node = sSpellMgr.GetSpellChainNode(spell_id);
    if (node)
    {
        if (HasSpell(node->next) && !GetTalentSpellPos(node->next))
            removeSpell(node->next,disabled);
    }
    //unlearn spells dependent from recently removed spells
    SpellsRequiringSpellMap const& reqMap = sSpellMgr.GetSpellsRequiringSpell();
    SpellsRequiringSpellMap::const_iterator itr2 = reqMap.find(spell_id);
    for (uint32 i=reqMap.count(spell_id);i>0;i--,itr2++)
        removeSpell(itr2->second,disabled);

    if (CanDualWield())
    {
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);

        if (spellInfo && spellInfo->HasEffect(SPELL_EFFECT_DUAL_WIELD))
        {
            SetCanDualWield(false);

            if (Item* offItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND)) // off hand here needed
            {
                uint16 dest;
                if (CanEquipItem(EQUIPMENT_SLOT_OFFHAND, dest, offItem, false, false) != EQUIP_ERR_OK)
                {
                    ItemPosCountVec off_dest;
                    uint8 off_msg = CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false);
                    if (off_msg == EQUIP_ERR_OK) 
                    {
                        RemoveItem(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND, true);
                        StoreItem(off_dest, offItem, true);
                    } // TRENTONE FIXME CHEATCON01
                    else
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Player::removeSpell: Can's store offhand item at spell remove player (GUID: %u).",GetGUIDLow());
                    }
                }
            }
        }
    }

    // removing
    WorldPacket data(SMSG_REMOVED_SPELL, 4);
    data << uint16(spell_id);
    SendPacketToSelf(&data);

    if (disabled)
    {
        itr->second.disabled = disabled;
        if (itr->second.state != PLAYERSPELL_NEW)
            itr->second.state = PLAYERSPELL_CHANGED;
    }
    else
    {
        if (itr->second.state == PLAYERSPELL_NEW)
            m_spells.erase(itr);
        else
            itr->second.state = PLAYERSPELL_REMOVED;
    }

    // buff scroll don't remove auras when switching specs (unlearning talent spelld like BoK)
    bool remove = true;
    switch (spell_id)
    {
        case 25389:
        case 25312:
        case 25433:
        case 27126:
        case 26990:
        case 26992:
        case 20217:
        case 27144:
        case 27140:
        case 1038:
        case 27168:
        case 27142:
            for (int i = 0; i < 3; ++i)
            {
                if (Aura* Aur = GetAura(spell_id, i))
                {
                    if (IS_CREATURE_GUID(Aur->GetCasterGUID()))
                    {
                        remove = false;
                        break;
                    }
                }
            }
    }

    if (remove)
        RemoveAurasDueToSpell(spell_id);

    if (spell_id == 18788) // Demonic Sacrifice
    {
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (Unit::AuraList::const_iterator itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
        {
            if ((*itr)->GetModifier()->m_miscvalue==2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }
    }

    // remove pet auras
    if (PetAura const* petSpell = sSpellMgr.GetPetAura(spell_id))
        RemovePetAura(petSpell);

    TalentSpellPos const* talentPos = GetTalentSpellPos(spell_id);
    if (talentPos)
    {
        // free talent points
        uint32 talentCosts = GetTalentSpellCost(talentPos);

        if (talentCosts < m_usedTalentCount)
            m_usedTalentCount -= talentCosts;
        else
            m_usedTalentCount = 0;

        UpdateFreeTalentPoints(false);
    }

    // update free primary prof.points (if not overflow setting, can be in case GM use before .learn prof. learning)
    if (sSpellMgr.IsPrimaryProfessionFirstRankSpell(spell_id))
    {
        uint32 freeProfs = GetFreePrimaryProfessionPoints()+1;
        if (freeProfs <= sWorld.getConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL))
            SetFreePrimaryProfessions(freeProfs);
    }

    // remove dependent skill
    SpellLearnSkillNode const* spellLearnSkill = sSpellMgr.GetSpellLearnSkill(spell_id);
    if (spellLearnSkill)
    {
        uint32 prev_spell = sSpellMgr.GetPrevSpellInChain(spell_id);
        if (!prev_spell)                                     // first rank, remove skill
            SetSkill(spellLearnSkill->skill,0,0);
        else
        {
            // search prev. skill setting by spell ranks chain
            SpellLearnSkillNode const* prevSkill = sSpellMgr.GetSpellLearnSkill(prev_spell);
            while (!prevSkill && prev_spell)
            {
                prev_spell = sSpellMgr.GetPrevSpellInChain(prev_spell);
                prevSkill = sSpellMgr.GetSpellLearnSkill(sSpellMgr.GetFirstSpellInChain(prev_spell));
            }

            if (!prevSkill)                                  // not found prev skill setting, remove skill
                SetSkill(spellLearnSkill->skill,0,0);
            else                                            // set to prev. skill setting values
            {
                uint32 skill_value = GetPureSkillValue(prevSkill->skill);
                uint32 skill_max_value = GetPureMaxSkillValue(prevSkill->skill);

                if (skill_value >  prevSkill->value)
                    skill_value = prevSkill->value;

                uint32 new_skill_max_value = prevSkill->maxvalue == 0 ? GetMaxSkillValueForLevel() : prevSkill->maxvalue;

                if (skill_max_value > new_skill_max_value)
                    skill_max_value =  new_skill_max_value;

                SetSkill(prevSkill->skill,skill_value,skill_max_value);
            }
        }

    }
    else
    {
        // not ranked skills
        SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(spell_id);
        SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(spell_id);

        for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(_spell_idx->second->skillId);
            if (!pSkill)
                continue;

            if (_spell_idx->second->learnOnGetSkill == ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL ||
                // poison special case, not have ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL
                pSkill->id==SKILL_POISONS && _spell_idx->second->max_value==0 ||
                // lockpicking special case, not have ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL
                pSkill->id==SKILL_LOCKPICKING && _spell_idx->second->max_value==0)
            {
                // not reset skills for professions and racial abilities
                if ((pSkill->categoryId==SKILL_CATEGORY_SECONDARY || pSkill->categoryId==SKILL_CATEGORY_PROFESSION) &&
                    (SpellMgr::IsProfessionSkill(pSkill->id) || _spell_idx->second->racemask!=0))
                    continue;

                SetSkill(pSkill->id, 0, 0);
            }
        }
    }

    // remove dependent spells
    SpellLearnSpellMap::const_iterator spell_begin = sSpellMgr.GetBeginSpellLearnSpell(spell_id);
    SpellLearnSpellMap::const_iterator spell_end   = sSpellMgr.GetEndSpellLearnSpell(spell_id);

    for (SpellLearnSpellMap::const_iterator itr3 = spell_begin; itr3 != spell_end; ++itr3)
        removeSpell(itr3->second.spell, disabled);
}

void Player::RemoveSpellCooldown(uint32 spell_id, bool update /* = false */)
{
    SpellEntry const * entry = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
    // check if spellentry is present and if the cooldown is less than 15 mins

    //normal spell
    if (entry && entry->Category)
        RemoveSpellCategoryCooldown(entry->Category); // only needed here, cause in DIRECT spell cooldown removal it is needed to ensure that cooldown won't be SERVERSIDE
    m_spellCooldowns.erase(spell_id);
    // as an example: i have cooldown on Sprint 1,2,3 by using Sprint3, but server removes cooldown from Sprint2, if not remove category cooldown - Sprint2 won't be available for us
    
    // item spell
    SpellItemCooldowns::const_iterator itr = m_spellItemCooldowns.find(spell_id);
    if (itr != m_spellItemCooldowns.end() && itr->second.category)
    {
        RemoveSpellCategoryCooldown(itr->second.category);
        m_spellItemCooldowns.erase(itr);
    }

    // send packet if needed
    if (update)
        SendRemoveCooldownPacket(spell_id);
}

void Player::RemoveSpellCategoryCooldown(uint32 category)
{
    m_spellCategoryCooldowns.erase(category);
}

void Player::RemoveArenaSpellCooldowns()
{
    // remove cooldowns on spells that has < 15 min CD
    SpellCooldowns::iterator itr, next;
    // iterate spell cooldowns
    time_t thetime = time(NULL);
    for (itr = m_spellCooldowns.begin();itr != m_spellCooldowns.end(); itr = next)
    {
        next = itr;
        ++next;
        SpellEntry const * entry = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
        // check if spellentry is present and if the cooldown is less than 15 mins
        if (entry && !(entry->AttributesEx4 & SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA) && entry->RecoveryTime <= 15 * MINUTE * 1000 && entry->CategoryRecoveryTime <= 15 * MINUTE * 1000)
        {
            if (entry->Category)
                RemoveSpellCategoryCooldown(entry->Category);

            // notify player
            if (itr->second >= thetime) // only send packet if there is such cooldown
                SendRemoveCooldownPacket(itr->first);

            // remove cooldown
            m_spellCooldowns.erase(itr);
        }
    }

    SpellItemCooldowns::iterator itr2, next2;
    for (itr2 = m_spellItemCooldowns.begin();itr2 != m_spellItemCooldowns.end(); itr2 = next2)
    {
        next2 = itr2;
        ++next2;
        SpellEntry const * entry = sSpellTemplate.LookupEntry<SpellEntry>(itr2->first);
        uint32 timeLeft = itr2->second.end > thetime ? itr2->second.end - thetime : 0;
        // check if spellentry is present and if the cooldown is less than 15 mins
        if (entry && !(entry->AttributesEx4 & SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA) && entry->RecoveryTime <= 15 * MINUTE * 1000 && entry->CategoryRecoveryTime <= 15 * MINUTE * 1000 &&
            timeLeft <= 15 * MINUTE)
        {
            if (itr2->second.category)
                RemoveSpellCategoryCooldown(itr2->second.category);

            // notify player
            if (timeLeft) // only send packet if there is such cooldown
                SendRemoveCooldownPacket(itr2->first);

            // remove cooldown
            m_spellItemCooldowns.erase(itr2);
        }
    }

    m_spellCategoryCooldowns.clear();
}

void Player::RemoveAllSpellCooldown()
{
    time_t thetime = time(NULL);
    for (SpellCooldowns::const_iterator itr = m_spellCooldowns.begin();itr != m_spellCooldowns.end(); ++itr)
    {
        if (itr->second < thetime)
            continue;
        SendRemoveCooldownPacket(itr->first);
    }
    for (SpellItemCooldowns::const_iterator itr = m_spellItemCooldowns.begin();itr != m_spellItemCooldowns.end(); ++itr)
    {
        if (itr->second.end < thetime)
            continue;
        SendRemoveCooldownPacket(itr->first);
    }

    m_spellCooldowns.clear();
    m_spellCategoryCooldowns.clear();
    m_spellItemCooldowns.clear();
}

void Player::SendRemoveCooldownPacket(uint32 spellId)
{
    WorldPacket data(SMSG_CLEAR_COOLDOWN, (4+8));
    data << uint32(spellId);
    data << uint64(GetGUID());
    SendPacketToSelf(&data);
}

void Player::_LoadSpellCooldowns(QueryResultAutoPtr result)
{
    m_spellCooldowns.clear();
    m_spellItemCooldowns.clear();

    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT spell,item,time FROM character_spell_cooldown WHERE guid = '%u'",GetGUIDLow());

    if (result)
    {
        time_t curTime = time(NULL);

        uint32 cat   = 0;
        int32 rec    = -1;
        int32 catrec = -1;

        do
        {
            Field *fields = result->Fetch();

            uint32 spell_id = fields[0].GetUInt32();
            uint32 item_id  = fields[1].GetUInt32();
            time_t db_time  = (time_t)fields[2].GetUInt64();

            if (!sSpellTemplate.LookupEntry<SpellEntry>(spell_id) &&
                spell_id != COMMAND_COOLDOWN)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Player %u have unknown spell %u in `character_spell_cooldown`, skipping.",GetGUIDLow(),spell_id);
                continue;
            }

            // skip outdated cooldown
            if (db_time <= curTime)
                continue;

            // preparations
            cat   = 0;
            rec    = -1;
            catrec = -1;

            if (ItemPrototype const* proto = ObjectMgr::GetItemPrototype(item_id))
            {
                if (proto)
                {
                    for (int idx = 0; idx < MAX_ITEM_PROTO_SPELLS; ++idx)
                    {
                        if (proto->Spells[idx].SpellId == spell_id)
                        {
                            cat    = proto->Spells[idx].SpellCategory;
                            rec    = proto->Spells[idx].SpellCooldown;
                            catrec = proto->Spells[idx].SpellCategoryCooldown;
                            break;
                        }
                    }
                }
            }

            if (SpellEntry const* spellInfoC = sSpellTemplate.LookupEntry<SpellEntry>(spell_id))
                // if no cooldown found above then base at DBC data
                if (rec < 0 && catrec < 0)
                {
                    cat = spellInfoC->Category;
                    rec = spellInfoC->RecoveryTime;
                    catrec = spellInfoC->CategoryRecoveryTime;
                }

            if (item_id)
                AddSpellItemCooldown(spell_id, item_id, cat, db_time);
            else
                AddSpellCooldown(spell_id, db_time);

            // shoot spells used equipped item cooldown values already assigned in GetAttackTime(RANGED_ATTACK)
            // prevent 0 cooldowns set by another way
            if (rec <= 0 && catrec <= 0 && (cat == 76 || cat == 351))
                rec = GetAttackTime(RANGED_ATTACK);

            // Now we have cooldown data (if found any), time to apply mods
            if (rec > 0)
                ApplySpellMod(spell_id, SPELLMOD_COOLDOWN, rec);

            if (catrec > 0)
                ApplySpellMod(spell_id, SPELLMOD_COOLDOWN, catrec);

            // replace negative cooldowns by 0
            if (rec < 0) rec = 0;
            if (catrec < 0) catrec = 0;

            // no cooldown after applying spell mods
            if (!(rec == 0 && catrec == 0))
            {
                time_t catrecTime;
                if (rec && rec > catrec)
                {
                    catrecTime = db_time - ((rec - catrec)/1000);   // in secs
                }
                else
                    catrecTime = db_time;

                if (catrecTime <= curTime)
                    continue;
                
                // category spells
                if (cat && catrec > 0)
                    AddSpellCategoryCooldown(cat, catrecTime);
            }

            sLog.outDebug("Player (GUID: %u) spell %u, item %u cooldown loaded (%u secs).", GetGUIDLow(), spell_id, item_id, uint32(db_time-curTime));
        }
        while (result->NextRow());
    }
}

void Player::_SaveSpellCooldowns()
{
    RealmDataDatabase.PExecute("DELETE FROM character_spell_cooldown WHERE guid = '%u'", GetGUIDLow());

    time_t curTime = time(NULL);

    // remove outdated and save active
    for (SpellCooldowns::iterator itr = m_spellCooldowns.begin();itr != m_spellCooldowns.end();)
    {
        if (itr->second <= curTime)
            m_spellCooldowns.erase(itr++);
        else
        {
            RealmDataDatabase.PExecute("INSERT INTO character_spell_cooldown (guid,spell,item,time) VALUES ('%u', '%u', '0', '%llu')", 
                GetGUIDLow(), itr->first, uint64(itr->second));
            ++itr;
        }
    }
    for (SpellItemCooldowns::iterator itr = m_spellItemCooldowns.begin();itr != m_spellItemCooldowns.end();)
    {
        if (itr->second.end <= curTime)
            m_spellItemCooldowns.erase(itr++);
        else
        {
            RealmDataDatabase.PExecute("INSERT INTO character_spell_cooldown (guid,spell,item,time) VALUES ('%u', '%u', '%u', '%llu')", 
                GetGUIDLow(), itr->first, itr->second.itemid, uint64(itr->second.end));
            ++itr;
        }
    }
}

uint32 Player::resetTalentsCost() const
{
    // The first time reset costs 1 gold
    if (m_resetTalentsCost < 1 * GOLD || sWorld.getConfig(CONFIG_FACTION_MINORITY) == GetTeam())
        return 1*GOLD;
    // then 5 gold
    else if (m_resetTalentsCost < 5*GOLD)
        return 5*GOLD;
    // After that it increases in increments of 5 gold
    else if (m_resetTalentsCost < 10*GOLD)
        return 10*GOLD;
    else
    {
        uint32 months = (sWorld.GetGameTime() - m_resetTalentsTime)/MONTH;
        if (months > 0)
        {
            // This cost will be reduced by a rate of 5 gold per month
            int32 new_cost = int32(m_resetTalentsCost) - 5*GOLD*months;
            // to a minimum of 10 gold.
            return (new_cost < 10*GOLD ? 10*GOLD : new_cost);
        }
        else
        {
            // After that it increases in increments of 5 gold
            int32 new_cost = m_resetTalentsCost + 5*GOLD;
            // until it hits a cap of 50 gold.
            if (new_cost > 50*GOLD)
                new_cost = 50*GOLD;
            return new_cost;
        }
    }
}

bool Player::resetTalents(bool no_cost)
{
    // not need after this call
    if (HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
    {
        m_atLoginFlags = m_atLoginFlags & ~AT_LOGIN_RESET_TALENTS;
        RealmDataDatabase.PExecute("UPDATE characters set at_login = at_login & ~ %u WHERE guid ='%u'", uint32(AT_LOGIN_RESET_TALENTS), GetGUIDLow());
    }

    uint32 talentPointsForLevel = CalculateTalentsPoints();

    if (m_usedTalentCount == 0)
    {
        SetFreeTalentPoints(talentPointsForLevel);
        return false;
    }

    uint32 cost = 0;

    if (!no_cost && !sWorld.getConfig(CONFIG_NO_RESET_TALENT_COST))
    {
        cost = resetTalentsCost();

        if (GetMoney() < cost)
        {
            SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
            return false;
        }
    }

    for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(talentId);

        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

        if (!talentTabInfo)
            continue;

        // unlearn only talents for character class
        // some spell learned by one class as normal spells or know at creation but another class learn it as talent,
        // to prevent unexpected lost normal learned spell skip another class talents
        if ((GetClassMask() & talentTabInfo->ClassMask) == 0)
            continue;

        //for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
        //{
        //    // skip non-existant talent ranks
        //    if (talentInfo->RankID[rank] == 0)
        //        continue;

        //    const SpellEntry* _spellEntry = GetSpellStore()->LookupEntry<SpellEntry>(talentInfo->RankID[rank]);
        //    if (!_spellEntry)
        //        continue;
        //    removeSpell(talentInfo->RankID[rank], true);
        //    // skip non-existant talent ranks
        //    if (talentInfo->RankID[rank] == 0)
        //        continue;

        //    const SpellEntry* spellEntry = GetSpellStore()->LookupEntry<SpellEntry>(talentInfo->RankID[rank]);
        //    if (!spellEntry)
        //        continue;
        //    removeSpell(talentInfo->RankID[rank], true);
        //    // search for spells that the talent teaches and unlearn them
        //    for (uint8 i = 0; i < 3; ++i)
        //        if (_spellEntry->EffectTriggerSpell[i] > 0 && _spellEntry->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
        //            removeSpell(_spellEntry->EffectTriggerSpell[i], true);
        //    // if this talent rank can be found in the PlayerTalentMap, mark the talent as removed so it gets deleted
        //    PlayerTalentMap::iterator plrTalent = m_talents[m_activeSpec].find(talentInfo->RankID[rank]);
        //    if (plrTalent != m_talents[m_activeSpec].end())
        //        plrTalent->second->state = PLAYERSPELL_REMOVED;
        //}

        for (int j = 0; j < 5; j++)
        {
            for (PlayerSpellMap::iterator itr = GetSpellMap().begin(); itr != GetSpellMap().end();)
            {
                if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled)
                {
                    ++itr;
                    continue;
                }

                // remove learned spells (all ranks)
                uint32 itrFirstId = sSpellMgr.GetFirstSpellInChain(itr->first);

                // unlearn if first rank is talent or learned by talent or spell itself is learned by talent
                if (itrFirstId == talentInfo->RankID[j] || sSpellMgr.IsSpellLearnToSpell(talentInfo->RankID[j], itrFirstId) || sSpellMgr.IsSpellLearnToSpell(talentInfo->RankID[j], itr->first))
                {
                    SpellEntry const * spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itrFirstId);
                    if (spellInfo)
                        for (uint8 i = 0; i < 3; ++i)
                        {
                            if (spellInfo->Effect[i] == SPELL_EFFECT_TRIGGER_SPELL && HasAura(spellInfo->EffectTriggerSpell[i]))
                                RemoveAurasDueToSpell(spellInfo->EffectTriggerSpell[i]);
                            // special case for warrior Defiance
                            if ((spellInfo->Id == 12303 || spellInfo->Id == 12788 || spellInfo->Id == 12789) && HasAura(45471))
                                RemoveAurasDueToSpell(45471);
                        }
                    // for dual wield                        
                    removeSpell(itr->first, !SpellMgr::IsPassiveSpell(itr->first));

                    // if this talent rank can be found in the PlayerTalentMap, mark the talent as removed so it gets deleted
                    PlayerTalentMap::iterator plrTalent = m_talents[m_activeSpec].find(talentInfo->RankID[j]);
                    if (plrTalent != m_talents[m_activeSpec].end())
                        plrTalent->second->state = PLAYERSPELL_REMOVED;

                    itr = GetSpellMap().begin();
                    continue;
                }
                else
                    ++itr;
            }
        }
    }

    //SetFreeTalentPoints(talentPointsForLevel);
    _ResetTalentMap(GetActiveSpec());
    _SaveTalents();
    _SaveSpells();

    SetFreeTalentPoints(talentPointsForLevel);

    if (!no_cost)
    {
        ModifyMoney(-(int32)cost);

        m_resetTalentsCost = cost;
        m_resetTalentsTime = time(NULL);
    }

    //FIXME: remove pet before or after unlearn spells? for now after unlearn to allow removing of talent related, pet affecting auras
    RemovePet(NULL,PET_SAVE_NOT_IN_SLOT, true);

    return true;
}

bool Player::_removeSpell(uint16 spell_id)
{
    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        m_spells.erase(itr);
        return true;
    }
    return false;
}

Mail* Player::GetMail(uint32 id)
{
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        if ((*itr)->messageID == id)
        {
            return (*itr);
        }
    }
    return NULL;
}

void Player::_SetCreateBits(UpdateMask *updateMask, Player *target) const
{
    if (target == this)
    {
        Object::_SetCreateBits(updateMask, target);
    }
    else
    {
        for (uint16 index = 0; index < m_valuesCount; index++)
        {
            if (GetUInt32Value(index) != 0 && updateVisualBits.GetBit(index))
                updateMask->SetBit(index);
        }
    }
}

void Player::_SetUpdateBits(UpdateMask *updateMask, Player *target) const
{
    if (target == this)
    {
        Object::_SetUpdateBits(updateMask, target);
    }
    else
    {
        Object::_SetUpdateBits(updateMask, target);
        *updateMask &= updateVisualBits;
    }
}

void Player::InitVisibleBits()
{
    updateVisualBits.SetCount(PLAYER_END);

    // TODO: really implement OWNER_ONLY and GROUP_ONLY. Flags can be found in UpdateFields.h

    updateVisualBits.SetBit(OBJECT_FIELD_GUID);
    updateVisualBits.SetBit(OBJECT_FIELD_TYPE);
    updateVisualBits.SetBit(OBJECT_FIELD_SCALE_X);

    updateVisualBits.SetBit(UNIT_FIELD_CHARM);
    updateVisualBits.SetBit(UNIT_FIELD_CHARM+1);

    updateVisualBits.SetBit(UNIT_FIELD_SUMMON);
    updateVisualBits.SetBit(UNIT_FIELD_SUMMON+1);

    updateVisualBits.SetBit(UNIT_FIELD_CHARMEDBY);
    updateVisualBits.SetBit(UNIT_FIELD_CHARMEDBY+1);

    updateVisualBits.SetBit(UNIT_FIELD_TARGET);
    updateVisualBits.SetBit(UNIT_FIELD_TARGET+1);

    updateVisualBits.SetBit(UNIT_FIELD_CHANNEL_OBJECT);
    updateVisualBits.SetBit(UNIT_FIELD_CHANNEL_OBJECT+1);

    updateVisualBits.SetBit(UNIT_FIELD_HEALTH);
    updateVisualBits.SetBit(UNIT_FIELD_POWER1);
    updateVisualBits.SetBit(UNIT_FIELD_POWER2);
    updateVisualBits.SetBit(UNIT_FIELD_POWER3);
    updateVisualBits.SetBit(UNIT_FIELD_POWER4);
    updateVisualBits.SetBit(UNIT_FIELD_POWER5);

    updateVisualBits.SetBit(UNIT_FIELD_MAXHEALTH);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER1);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER2);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER3);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER4);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER5);

    updateVisualBits.SetBit(UNIT_FIELD_LEVEL);
    updateVisualBits.SetBit(UNIT_FIELD_FACTIONTEMPLATE);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_0);
    updateVisualBits.SetBit(UNIT_FIELD_FLAGS);
    updateVisualBits.SetBit(UNIT_FIELD_FLAGS_2);
    for (uint16 i = UNIT_FIELD_AURA; i < UNIT_FIELD_AURASTATE; ++i)
        updateVisualBits.SetBit(i);
    updateVisualBits.SetBit(UNIT_FIELD_AURASTATE);
    updateVisualBits.SetBit(UNIT_FIELD_BASEATTACKTIME);
    updateVisualBits.SetBit(UNIT_FIELD_BASEATTACKTIME + 1);
    updateVisualBits.SetBit(UNIT_FIELD_BOUNDINGRADIUS);
    updateVisualBits.SetBit(UNIT_FIELD_COMBATREACH);
    updateVisualBits.SetBit(UNIT_FIELD_DISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_NATIVEDISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_1);
    updateVisualBits.SetBit(UNIT_FIELD_PETNUMBER);
    updateVisualBits.SetBit(UNIT_FIELD_PET_NAME_TIMESTAMP);
    updateVisualBits.SetBit(UNIT_DYNAMIC_FLAGS);
    updateVisualBits.SetBit(UNIT_CHANNEL_SPELL);
    updateVisualBits.SetBit(UNIT_MOD_CAST_SPEED);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_2);

    updateVisualBits.SetBit(PLAYER_DUEL_ARBITER);
    updateVisualBits.SetBit(PLAYER_DUEL_ARBITER+1);
    updateVisualBits.SetBit(PLAYER_FLAGS);
    updateVisualBits.SetBit(PLAYER_GUILDID);
    updateVisualBits.SetBit(PLAYER_GUILDRANK);
    updateVisualBits.SetBit(PLAYER_BYTES);
    updateVisualBits.SetBit(PLAYER_BYTES_2);
    updateVisualBits.SetBit(PLAYER_BYTES_3);
    updateVisualBits.SetBit(PLAYER_DUEL_TEAM);
    updateVisualBits.SetBit(PLAYER_GUILD_TIMESTAMP);

    // PLAYER_QUEST_LOG_x also visible bit on official (but only on party/raid)...
    for (uint16 i = PLAYER_QUEST_LOG_1_1; i < PLAYER_QUEST_LOG_25_2; i+=4)
        updateVisualBits.SetBit(i);

    //Players visible items are not inventory stuff
    //431) = 884 (0x374) = main weapon
    for (uint16 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        // item creator
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_CREATOR + (i*MAX_VISIBLE_ITEM_OFFSET) + 0);
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_CREATOR + (i*MAX_VISIBLE_ITEM_OFFSET) + 1);

        uint16 visual_base = PLAYER_VISIBLE_ITEM_1_0 + (i*MAX_VISIBLE_ITEM_OFFSET);

        // item entry
        updateVisualBits.SetBit(visual_base + 0);

        // item enchantment IDs
        for (uint8 j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
            updateVisualBits.SetBit(visual_base + 1 + j);

        // random properties
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 0 + (i*MAX_VISIBLE_ITEM_OFFSET));
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (i*MAX_VISIBLE_ITEM_OFFSET));
    }

    updateVisualBits.SetBit(PLAYER_CHOSEN_TITLE);
}

void Player::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if (target == this)
    {
        for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer(data, target);
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer(data, target);
        }
        for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer(data, target);
        }
    }

    Unit::BuildCreateUpdateBlockForPlayer(data, target);
}

void Player::DestroyForPlayer(Player *target) const
{
    Unit::DestroyForPlayer(target);

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (m_items[i] == NULL)
            continue;

        m_items[i]->DestroyForPlayer(target);
    }

    if (target == this)
    {
        for (int i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->DestroyForPlayer(target);
        }
        for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->DestroyForPlayer(target);
        }
    }
}

bool Player::HasSpell(uint32 spell) const
{
    PlayerSpellMap::const_iterator itr = m_spells.find((uint16)spell);
    return (itr != m_spells.end() && itr->second.state != PLAYERSPELL_REMOVED && !itr->second.disabled);
}

TrainerSpellState Player::GetTrainerSpellState(TrainerSpell const* trainer_spell) const
{
    if (!trainer_spell)
        return TRAINER_SPELL_RED;

    if (!trainer_spell->spell)
        return TRAINER_SPELL_RED;

    // known spell
    if (HasSpell(trainer_spell->spell))
        return TRAINER_SPELL_GRAY;

    // check race/class requirement
    if (!IsSpellFitByClassAndRace(trainer_spell->spell))
        return TRAINER_SPELL_RED;

    // check level requirement
    if (GetLevel() < trainer_spell->reqLevel)
        return TRAINER_SPELL_RED;

    if (SpellChainNode const* spell_chain = sSpellMgr.GetSpellChainNode(trainer_spell->spell))
    {
        // check prev.rank requirement
        if (spell_chain->prev && !HasSpell(spell_chain->prev))
            return TRAINER_SPELL_RED;
    }

    if (uint32 spell_req = sSpellMgr.GetSpellRequired(trainer_spell->spell))
    {
        // check additional spell requirement
        if (!HasSpell(spell_req))
            return TRAINER_SPELL_RED;
    }

    // check skill requirement
    if (trainer_spell->reqSkill && GetBaseSkillValue(trainer_spell->reqSkill) < trainer_spell->reqSkillValue)
        return TRAINER_SPELL_RED;

    // exist, already checked at loading
    SpellEntry const* spell = sSpellTemplate.LookupEntry<SpellEntry>(trainer_spell->spell);

    // secondary prof. or not prof. spell
    uint32 skill = spell->EffectMiscValue[1];

    if (spell->Effect[1] != SPELL_EFFECT_SKILL || !SpellMgr::IsPrimaryProfessionSkill(skill))
        return TRAINER_SPELL_GREEN;

    // check primary prof. limit
    if (sSpellMgr.IsPrimaryProfessionFirstRankSpell(spell->Id) && GetFreePrimaryProfessionPoints() == 0)
        return TRAINER_SPELL_RED;

    return TRAINER_SPELL_GREEN;
}

void Player::DeleteCharacterInfoFromDB(uint32 playerGUIDLow)
{
    // completely removes the character, calls after KeepDeletedCharsTime
    
    // unsummon and delete for pets in world is not required: player deleted from CLI or character list with not loaded pet.
    // Get guids of character's pets, will deleted in transaction
    QueryResultAutoPtr resultPets = RealmDataDatabase.PQuery("SELECT id FROM character_pet WHERE owner = '%u'", playerGUIDLow);

    // NOW we can finally clear other DB data related to character
    RealmDataDatabase.BeginTransaction();
    if (resultPets)
    {
        do
        {
            Field *fields3 = resultPets->Fetch();
            uint32 petguidlow = fields3[0].GetUInt32();
            //do not create separate transaction for pet delete otherwise we will get fatal error!
            Pet::DeleteFromDB(petguidlow, false);
        }
        while (resultPets->NextRow());
    }

    RealmDataDatabase.PExecute("DELETE FROM characters WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_declinedname WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_action WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_aura WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_gifts WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_homebind WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM group_instance WHERE leaderGuid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_queststatus WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_reputation WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_spell WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_spell_cooldown WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_talent WHERE guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM gm_tickets WHERE playerGuid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE owner_guid = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_social WHERE guid = '%u' OR friend='%u'", playerGUIDLow, playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM mail WHERE receiver = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM mail_items WHERE receiver = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_pet WHERE owner = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM character_pet_declinedname WHERE owner = '%u'", playerGUIDLow);
    RealmDataDatabase.PExecute("DELETE FROM deleted_chars WHERE char_guid = '%u'", playerGUIDLow);

    // fix guild logs
    // must be removed from DB, because 0 cannot be used (breaks view)
    RealmDataDatabase.PExecute("UPDATE guild_eventlog SET PlayerGuid2 = 0 WHERE PlayerGuid2 = '%u'", playerGUIDLow);

    // set 0 to display UNDEFINED as name
    RealmDataDatabase.PExecute("DELETE FROM guild_eventlog WHERE PlayerGuid1 ='%u'", playerGUIDLow);

    // for solo 3v3
    uint32 at_id = GetArenaTeamIdFromDB(playerGUIDLow, ARENA_TEAM_3v3);
    if (at_id != 0)
    {
        ArenaTeam * at = sObjectMgr.GetArenaTeamById(at_id);
        if (at)
        {
            at->DelMember(playerGUIDLow);
            RealmDataDatabase.PExecute("DELETE FROM arena_team WHERE arenateamid = '%u'", at_id);
            RealmDataDatabase.PExecute("DELETE FROM arena_team_member WHERE arenateamid = '%u'", at_id); //< this should be alredy done by calling DelMember(memberGuids[j]); for each member
            RealmDataDatabase.PExecute("DELETE FROM arena_team_stats WHERE arenateamid = '%u'", at_id);
        }            
    }

    RealmDataDatabase.CommitTransaction();
}

void Player::DeleteFromDB(uint64 playerguid, uint32 accountId, bool updateRealmChars)
{
    // character is saved by CONFIG_DONT_DELETE_CHARS
    
    uint32 guid = GUID_LOPART(playerguid);

    // convert corpse to bones if exist (to prevent exiting Corpse in World without DB entry)
    // bones will be deleted by corpse/bones deleting thread shortly
    sObjectAccessor.ConvertCorpseForPlayer(playerguid);

    // remove from guild
    uint32 guildId = GetGuildIdFromDB(playerguid);
    if (guildId != 0)
    {
        Guild* guild = sGuildMgr.GetGuildById(guildId);
        if (guild)
            guild->DelMember(guid);
    }

    // remove from arena teams
    uint32 at_id = GetArenaTeamIdFromDB(playerguid,ARENA_TEAM_2v2);
    if (at_id != 0)
    {
        ArenaTeam * at = sObjectMgr.GetArenaTeamById(at_id);
        if (at)
            at->DelMember(playerguid);
    }

    // we dont need 3v3 at this step

    at_id = GetArenaTeamIdFromDB(playerguid,ARENA_TEAM_5v5);
    if (at_id != 0)
    {
        ArenaTeam * at = sObjectMgr.GetArenaTeamById(at_id);
        if (at)
            at->DelMember(playerguid);
    }

    // the player was uninvited already on logout so just remove from group
    QueryResultAutoPtr resultGroup = RealmDataDatabase.PQuery("SELECT leaderGuid FROM group_member WHERE memberGuid='%u'", guid);
    if (resultGroup)
    {
        uint64 leaderGuid = MAKE_NEW_GUID((*resultGroup)[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        Group* group = sObjectMgr.GetGroupByLeader(leaderGuid);
        if (group)
        {
            RemoveFromGroup(group, playerguid);
        }
    }

    // remove signs from petitions (also remove petitions if owner);
    RemovePetitionsAndSigns(playerguid, PETITION_TYPE_REMOVE_ALL);

    // return back all mails with COD and Item                        0     1              2         3       4          5      6       7
    QueryResultAutoPtr resultMail = RealmDataDatabase.PQuery("SELECT id,messageType,mailTemplateId,sender,subject,itemTextId,money,has_items FROM mail WHERE receiver='%u' AND has_items<>0 AND cod<>0", guid);
    if (resultMail)
    {
        do
        {
            Field *fields = resultMail->Fetch();

            uint32 mail_id          = fields[0].GetUInt32();
            uint16 mailType         = fields[1].GetUInt16();
            uint16 mailTemplateId   = fields[2].GetUInt16();
            uint32 sender           = fields[3].GetUInt32();
            std::string subject     = fields[4].GetCppString();
            uint32 itemTextId       = fields[5].GetUInt32();
            uint32 money            = fields[6].GetUInt32();
            bool has_items          = fields[7].GetBool();

            //we can return mail now
            //so firstly delete the old one
            RealmDataDatabase.PExecute("DELETE FROM mail WHERE id = '%u'", mail_id);

            // mail not from player
            if (mailType != MAIL_NORMAL)
            {
                if(has_items)
                    RealmDataDatabase.PExecute("DELETE FROM mail_items WHERE mail_id = '%u'", mail_id);
                continue;
            }

            MailDraft draft;
            if (mailTemplateId)
                draft.SetMailTemplate(mailTemplateId, false);// items already included
            else
                draft.SetSubjectAndBodyId(subject, itemTextId);

            if (has_items)
            {
                QueryResultAutoPtr resultItems = RealmDataDatabase.PQuery("SELECT item_guid,item_template FROM mail_items WHERE mail_id='%u'", mail_id);
                if (resultItems)
                {
                    do
                    {
                        Field *fields2 = resultItems->Fetch();

                        uint32 item_guidlow = fields2[0].GetUInt32();
                        uint32 item_template = fields2[1].GetUInt32();

                        ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype(item_template);
                        if (!itemProto)
                        {
                            RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", item_guidlow);
                            continue;
                        }

                        Item *pItem = NewItemOrBag(itemProto);
                        if (!pItem->LoadFromDB(item_guidlow, MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER)))
                        {
                            pItem->FSetState(ITEM_REMOVED);
                            pItem->SaveToDB();              // it also deletes item object !
                            continue;
                        }

                        draft.AddItem(pItem);
                    }
                    while (resultItems->NextRow());
                }
            }

            RealmDataDatabase.PExecute("DELETE FROM mail_items WHERE mail_id = '%u'", mail_id);

            uint32 pl_account = sObjectMgr.GetPlayerAccountIdByGUID(MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER));

            draft.SetMoney(money).SendReturnToSender(pl_account, ObjectGuid(playerguid), ObjectGuid(HIGHGUID_PLAYER, sender));
        }
        while (resultMail->NextRow());
    }

    if (sWorld.getConfig(CONFIG_DONT_DELETE_CHARS))
    {
        if (QueryResultAutoPtr result= RealmDataDatabase.PQuery("SELECT data FROM characters WHERE guid='%u'",guid))
        {
            Field *fields = result->Fetch();

            Tokens data = StrSplit(fields[0].GetCppString(), " ");
            uint32 plLevel = Player::GetUInt32ValueFromArray(data,UNIT_FIELD_LEVEL);

            if (plLevel >= sWorld.getConfig(CONFIG_DONT_DELETE_CHARS_LVL))
            {
                RealmDataDatabase.PExecute("INSERT INTO deleted_chars SELECT NULL, %u, name, account, NOW() FROM characters WHERE guid = %u;", guid, guid);
                RealmDataDatabase.PExecute("UPDATE characters SET account = 1 WHERE guid = %u", guid);
                RealmDataDatabase.PExecute("UPDATE characters SET name = '_deleted' WHERE guid = %u", guid);
                RealmDataDatabase.PExecute("DELETE FROM character_social WHERE guid = %u OR friend = %u", guid, guid);
                RealmDataDatabase.PExecute("DELETE FROM mail WHERE receiver = %u", guid);
                RealmDataDatabase.PExecute("DELETE FROM mail_items WHERE receiver = %u", guid);

                if (updateRealmChars)
                    sWorld.UpdateRealmCharCount(accountId);
                return;
            }
        }
    }

    // finally delete rest character informations (pets, spells, auras, items etc)
    DeleteCharacterInfoFromDB(guid);

    //LoginDatabase.PExecute("UPDATE realmcharacters SET numchars = numchars - 1 WHERE acctid = %d AND realmid = %d", accountId, realmID);
    if (updateRealmChars)
        sWorld.UpdateRealmCharCount(accountId);
}

void Player::SetMovement(PlayerMovementType pType)
{
    WorldPacket data;
    switch (pType)
    {
        case MOVE_ROOT:       data.Initialize(SMSG_FORCE_MOVE_ROOT,   GetPackGUID().size()+4); break;
        case MOVE_UNROOT:     data.Initialize(SMSG_FORCE_MOVE_UNROOT, GetPackGUID().size()+4); break;
        case MOVE_WATER_WALK: data.Initialize(SMSG_MOVE_WATER_WALK,   GetPackGUID().size()+4); break;
        case MOVE_LAND_WALK:  data.Initialize(SMSG_MOVE_LAND_WALK,    GetPackGUID().size()+4); break;
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: Player::SetMovement: Unsupported move type (%d), data not sent to client.",pType);
            return;
    }
    data << GetPackGUID();
    data << uint32(0);
    SendPacketToSelf(&data);
}

/* Preconditions:
  - a resurrectable corpse must not be loaded for the player (only bones)
  - the player must be in world
*/
void Player::BuildPlayerRepop()
{
    if (GetRace() == RACE_NIGHTELF)
        CastSpell(this, 20584, true);                       // auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
    CastSpell(this, 8326, true);                            // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

    // there must be SMSG.FORCE_RUN_SPEED_CHANGE, SMSG.FORCE_SWIM_SPEED_CHANGE, SMSG.MOVE_WATER_WALK
    // there must be SMSG.STOP_MIRROR_TIMER
    // there we must send 888 opcode

    // the player cannot have a corpse already, only bones which are not returned by GetCorpse
    if (GetCorpse())
    {
        sLog.outLog(LOG_CRASH, "ERROR: BuildPlayerRepop: player %s(%d) already has a corpse", GetName(), GetGUIDLow());
        ASSERT(false);
    }

    // create a corpse and place it at the player's location
    // don't create corpses in Shattrath
    Corpse* corpse = nullptr;
    if (GetZoneId() != 3703)
    {
        CreateCorpse();
        corpse = GetCorpse();
        if (!corpse)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Error creating corpse for Player %s [%u]", GetName(), GetGUIDLow());
            return;
        }
        GetMap()->Add(corpse);
    }

    SetHealth(1, true);

    SetMovement(MOVE_WATER_WALK);
    if (!GetSession()->isLogingOut())
        SetMovement(MOVE_UNROOT);

    // BG - remove insignia related
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

//    SendCorpseReclaimDelay();

    // to prevent cheating
    if (corpse)
    corpse->ResetGhostTime();

    StopMirrorTimers();                                     //disable timers(bars)

    SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, (float)1.0);   //see radius of death player?

    SetByteValue(UNIT_FIELD_BYTES_1, 3, PLAYER_STATE_FLAG_ALWAYS_STAND);
}

void Player::SendDelayResponse(const uint32 ml_seconds)
{
    //FIXME: is this delay time arg really need? 50msec by default in code
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4+4);
    data << (uint32)time(NULL);
    data << (uint32)0;
    SendPacketToSelf(&data);
}

void Player::ResurrectPlayer(float restore_percent, bool applySickness)
{
    WorldPacket data(SMSG_DEATH_RELEASE_LOC, 4*4);          // remove spirit healer position
    data << uint32(-1);
    data << float(0);
    data << float(0);
    data << float(0);
    SendPacketToSelf(&data);

    // speed change, land walk

    // remove death flag + set aura
    SetByteValue(UNIT_FIELD_BYTES_1, 3, 0x00);
    if (GetRace() == RACE_NIGHTELF)
        RemoveAurasDueToSpell(20584);                       // speed bonuses
    RemoveAurasDueToSpell(8326);                            // SPELL_AURA_GHOST

    setDeathState(ALIVE);

    SetMovement(MOVE_LAND_WALK);
    SetMovement(MOVE_UNROOT);

    m_deathTimer = 0;

    // refer-a-friend flag - maybe wrong and hacky
    if (GetSession()->IsRaf())
        SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_REFER_A_FRIEND);

    // set health/powers (0- will be set in caller)
    if (restore_percent>0.0f)
    {
		// 1 * 0.5 = 0, ceil is not a solution
		uint32 sethealth = uint32(GetMaxHealth()*restore_percent);
		
		SetHealth(sethealth > 0 ? sethealth : 1);
        SetPower(POWER_MANA, uint32(GetMaxPower(POWER_MANA)*restore_percent));
        SetPower(POWER_RAGE, 0);
        SetPower(POWER_ENERGY, uint32(GetMaxPower(POWER_ENERGY)*restore_percent));
    }

    //HACK restore Alchemist stone aura
    Item * item = NULL;
    if ((item = HasEquiped(13503)) ||
        (item = HasEquiped(35751)) ||
        (item = HasEquiped(35748)) ||
        (item = HasEquiped(35750)) ||
        (item = HasEquiped(35749)))
        CastSpell(this, 17619, true, item);

    UpdateZone(GetZoneId());

    // update visibility
    UpdateVisibilityAndView();

    // summon golem guardian at relog, revive, entering instance
    if (sWorld.isEasyRealm() && IsLowHeroicDungeonOrNonactualRaid(GetMapId(), false))
        CastSpell(this, 55200, true);

    if (!applySickness)
        return;

    //Characters from level 1-10 are not affected by resurrection sickness.
    //Characters from level 11-19 will suffer from one minute of sickness
    //for each level they are above 10.
    //Characters level 20 and up suffer from ten minutes of sickness.
    int32 startLevel = sWorld.getConfig(CONFIG_DEATH_SICKNESS_LEVEL);

    if (int32(GetLevel()) >= startLevel)
    {
        // set resurrection sickness
        CastSpell(this,SPELL_ID_PASSIVE_RESURRECTION_SICKNESS,true);

        // not full duration
        if (int32(GetLevel()) < startLevel+9)
        {
            int32 delta = (int32(GetLevel()) - startLevel + 1)*MINUTE;

            for (int i =0; i < 3; ++i)
            {
                if (Aura* Aur = GetAura(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS,i))
                {
                    Aur->SetAuraDuration(delta*1000);
                    Aur->UpdateAuraDuration();
                }
            }
        }
    }
}

void Player::KillPlayer()
{
    SetMovement(MOVE_ROOT);

    StopMirrorTimers();                                     //disable timers(bars)

    setDeathState(CORPSE);
    //SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_IN_PVP);

    SetFlag(UNIT_DYNAMIC_FLAGS, 0x00);
    ApplyModFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_RELEASE_TIMER, !sMapStore.LookupEntry(GetMapId())->Instanceable());

    // 6 minutes until repop at graveyard
    m_deathTimer = 6*MINUTE*1000;

    UpdateCorpseReclaimDelay();                             // dependent at use SetDeathPvP() call before kill
    SendCorpseReclaimDelay();

    // don't create corpse at this moment, player might be falling

    // update visibility
    UpdateObjectVisibility();
}

void Player::CreateCorpse()
{
    // prevent existence 2 corpse for player
    SpawnCorpseBones();

    uint32 _uf, _pb, _pb2, _cfb1, _cfb2;

    Corpse *corpse = new Corpse((m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH) ? CORPSE_RESURRECTABLE_PVP : CORPSE_RESURRECTABLE_PVE);
    SetPvPDeath(false);

    if (!corpse->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_CORPSE), this))
    {
        delete corpse;
        return;
    }

    _uf = GetUInt32Value(UNIT_FIELD_BYTES_0);
    _pb = GetUInt32Value(PLAYER_BYTES);
    _pb2 = GetUInt32Value(PLAYER_BYTES_2);

    uint8 race       = (uint8)(_uf);
    uint8 skin       = (uint8)(_pb);
    uint8 face       = (uint8)(_pb >> 8);
    uint8 hairstyle  = (uint8)(_pb >> 16);
    uint8 haircolor  = (uint8)(_pb >> 24);
    uint8 facialhair = (uint8)(_pb2);

    _cfb1 = ((0x00) | (race << 8) | (GetGender() << 16) | (skin << 24));
    _cfb2 = ((face) | (hairstyle << 8) | (haircolor << 16) | (facialhair << 24));

    corpse->SetUInt32Value(CORPSE_FIELD_BYTES_1, _cfb1);
    corpse->SetUInt32Value(CORPSE_FIELD_BYTES_2, _cfb2);

    uint32 flags = CORPSE_FLAG_UNK2;
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
        flags |= CORPSE_FLAG_HIDE_HELM;
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK))
        flags |= CORPSE_FLAG_HIDE_CLOAK;
    if (InBattleGroundOrArena() && !InArena())
        flags |= CORPSE_FLAG_LOOTABLE;                      // to be able to remove insignia
    corpse->SetUInt32Value(CORPSE_FIELD_FLAGS, flags);

    corpse->SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, GetNativeDisplayId());

    corpse->SetUInt32Value(CORPSE_FIELD_GUILD, GetGuildId());

    uint32 iDisplayID;
    uint16 iIventoryType;
    uint32 _cfi;
    for (int i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        if (m_items[i])
        {
            iDisplayID = m_items[i]->GetProto()->DisplayInfoID;
            iIventoryType = (uint16)m_items[i]->GetProto()->InventoryType;

            _cfi =  (uint16(iDisplayID)) | (iIventoryType)<< 24;
            corpse->SetUInt32Value(CORPSE_FIELD_ITEM + i,_cfi);
        }
    }

    // we don't SaveToDB for players in battlegrounds so don't do it for corpses either
    const MapEntry *entry = sMapStore.LookupEntry(corpse->GetMapId());
    ASSERT(entry);
    if (entry->map_type != MAP_BATTLEGROUND)
        corpse->SaveToDB();

    // register for player, but not show
    sObjectAccessor.AddCorpse(corpse);
}

void Player::SpawnCorpseBones()
{
    if (sObjectAccessor.ConvertCorpseForPlayer(GetGUID()))
    {
        if (!GetSession()->PlayerLogoutWithSave())
            SaveToDB();                                         // prevent loading as ghost without corpse
    }
}

Corpse* Player::GetCorpse() const
{
    return sObjectAccessor.GetCorpseForPlayerGUID(GetGUID());
}

void Player::DurabilityLossAll(double percent, bool inventory)
{
    // x100 ? do not lose durability

    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        if (Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            DurabilityLoss(pItem,percent);

    if (inventory)
    {
        // bags not have durability
        // for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)

        for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
            if (Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                DurabilityLoss(pItem,percent);

        // keys not have durability
        //for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                    if (Item* pItem = GetItemByPos(i, j))
                        DurabilityLoss(pItem,percent);
    }
}

void Player::DurabilityLoss(Item* item, double percent)
{
    if (!item)
        return;

    uint32 pMaxDurability =  item ->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

    if (!pMaxDurability)
        return;

    uint32 pDurabilityLoss = uint32(pMaxDurability*percent);

    if (pDurabilityLoss < 1)
        pDurabilityLoss = 1;

    DurabilityPointsLoss(item,pDurabilityLoss);
}

void Player::DurabilityPointsLossAll(int32 points, bool inventory)
{
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        if (Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            DurabilityPointsLoss(pItem,points);

    if (inventory)
    {
        // bags not have durability
        // for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)

        for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
            if (Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                DurabilityPointsLoss(pItem,points);

        // keys not have durability
        //for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                    if (Item* pItem = GetItemByPos(i, j))
                        DurabilityPointsLoss(pItem,points);
    }
}

void Player::DurabilityPointsLoss(Item* item, int32 points)
{
    int32 pMaxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    int32 pOldDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
    int32 pNewDurability = pOldDurability - points;

    if (pNewDurability < 0)
        pNewDurability = 0;
    else if (pNewDurability > pMaxDurability)
        pNewDurability = pMaxDurability;

    if (pOldDurability != pNewDurability)
    {
        // modify item stats _before_ Durability set to 0 to pass _ApplyItemMods internal check
        if (pNewDurability == 0 && pOldDurability > 0 && item->IsEquipped())
            _ApplyItemMods(item,item->GetSlot(), false);

        item->SetUInt32Value(ITEM_FIELD_DURABILITY, pNewDurability);

        // modify item stats _after_ restore durability to pass _ApplyItemMods internal check
        if (pNewDurability > 0 && pOldDurability == 0 && item->IsEquipped())
            _ApplyItemMods(item,item->GetSlot(), true);

        item->SetState(ITEM_CHANGED, this);
    }
}

void Player::DurabilityPointLossForEquipSlot(EquipmentSlots slot)
{
    if (Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        DurabilityPointsLoss(pItem,1);
}

uint32 Player::DurabilityRepairAll(bool cost, float discountMod, bool guildBank)
{
    uint32 TotalCost = 0;
    // equipped, backpack, bags itself
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
        TotalCost += DurabilityRepair(((INVENTORY_SLOT_BAG_0 << 8) | i),cost,discountMod, guildBank);

    // bank, buyback and keys not repaired

    // items in inventory bags
    for (int j = INVENTORY_SLOT_BAG_START; j < INVENTORY_SLOT_BAG_END; j++)
        for (int i = 0; i < MAX_BAG_SIZE; i++)
            TotalCost += DurabilityRepair(((j << 8) | i),cost,discountMod, guildBank);
    return TotalCost;
}

uint32 Player::DurabilityRepair(uint16 pos, bool cost, float discountMod, bool guildBank)
{
    Item* item = GetItemByPos(pos);

    uint32 TotalCost = 0;
    if (!item)
        return TotalCost;

    uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    if (!maxDurability)
        return TotalCost;

    uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

    if (cost)
    {
        uint32 LostDurability = maxDurability - curDurability;
        if (LostDurability>0)
        {
            ItemPrototype const *ditemProto = item->GetProto();

            DurabilityCostsEntry const *dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
            if (!dcost)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: RepairDurability: Wrong item lvl %u", ditemProto->ItemLevel);
                return TotalCost;
            }

            uint32 dQualitymodEntryId = (ditemProto->Quality+1)*2;
            DurabilityQualityEntry const *dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
            if (!dQualitymodEntry)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: RepairDurability: Wrong dQualityModEntry %u", dQualitymodEntryId);
                return TotalCost;
            }

            uint32 dmultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class,ditemProto->SubClass)];
            uint32 costs = uint32(LostDurability*dmultiplier*double(dQualitymodEntry->quality_mod));

            costs = uint32(costs * discountMod);

            if (costs==0)                                   //fix for ITEM_QUALITY_ARTIFACT
                costs = 1;

            if (guildBank)
            {
                if (GetGuildId()==0)
                {
                    debug_log("You are not member of a guild");
                    return TotalCost;
                }

                Guild *pGuild = sGuildMgr.GetGuildById(GetGuildId());
                if (!pGuild)
                    return TotalCost;

                if (!pGuild->HasRankRight(GetRank(), GR_RIGHT_WITHDRAW_REPAIR))
                {
                    debug_log("You do not have rights to withdraw for repairs");
                    return TotalCost;
                }

                if (pGuild->GetMemberMoneyWithdrawRem(GetGUIDLow()) < costs)
                {
                    debug_log("You do not have enough money withdraw amount remaining");
                    return TotalCost;
                }

                if (pGuild->GetGuildBankMoney() < costs)
                {
                    debug_log("There is not enough money in bank");
                    return TotalCost;
                }

                pGuild->MemberMoneyWithdraw(costs, GetGUIDLow());
                TotalCost = costs;
            }
            else if (GetMoney() < costs)
            {
                debug_log("You do not have enough money");
                return TotalCost;
            }
            else
                ModifyMoney(-int32(costs));
        }
    }

    item->SetUInt32Value(ITEM_FIELD_DURABILITY, maxDurability);
    item->SetState(ITEM_CHANGED, this);

    // reapply mods for total broken and repaired item if equipped
    if (IsEquipmentPos(pos) && !curDurability)
        _ApplyItemMods(item,pos & 255, true);
    return TotalCost;
}

void Player::RepopAtGraveyard()
{
    // note: this can be called also when the player is alive
    // for example from WorldSession::HandleMovementOpcodes

    AreaTableEntry const *zone = GetAreaEntryByAreaID(GetCachedArea());

    // Such zones are considered unreachable as a ghost and the player must be automatically revived
    if (!isAlive() && zone && zone->flags & AREA_FLAG_NEED_FLY || GetTransport() || GetPositionZ() < FALL_UNDER_MAP_HEIGHT || GetMapId() == 409/*MC*/ || GetMapId() == 469/*BWL*/)
    {
        ResurrectPlayer(0.5f);
        SpawnCorpseBones();
    }

    TeleportToNearestGraveyard();
}

void Player::TeleportToNearestGraveyard()
{
    WorldSafeLocsEntry const *ClosestGrave = NULL;

    // Special handle for battleground maps
    if (BattleGround *bg = GetBattleGround())
    {
        switch (bg->GetTypeID())
        {
            case BATTLEGROUND_AB:
            case BATTLEGROUND_EY:
            case BATTLEGROUND_AV:
            case BATTLEGROUND_WS:
                ClosestGrave = bg->GetClosestGraveYard(GetPositionX(), GetPositionY(), GetPositionZ(), GetBGTeam() ? GetBGTeam() : GetTeam());
                break;
            default:
                ClosestGrave = sObjectMgr.GetClosestGraveYard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetBGTeam() ? GetBGTeam() : GetTeam());
                break;

        }
    }
    else
        ClosestGrave = sObjectMgr.GetClosestGraveYard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetTeam());

    // stop countdown until repop
    m_deathTimer = 0;

    // if no grave found, stay at the current location
    // and don't show spirit healer location
    if (ClosestGrave)
    {
        bool updateVisibility = IsInWorld() && GetMapId() == ClosestGrave->map_id;
        TeleportTo(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, GetOrientation());
        if (isDead())                                        // not send if alive, because it used in TeleportTo()
        {
            WorldPacket data(SMSG_DEATH_RELEASE_LOC, 4*4);  // show spirit healer position on minimap
            data << ClosestGrave->map_id;
            data << ClosestGrave->x;
            data << ClosestGrave->y;
            data << ClosestGrave->z;
            SendPacketToSelf(&data);
        }
        if (updateVisibility && IsInWorld())
            UpdateVisibilityAndView();
    }
}

void Player::JoinedChannel(Channel *c)
{
    m_channels.push_back(c);
}

void Player::LeftChannel(Channel *c)
{
    m_channels.remove(c);
}

void Player::CleanupChannels()
{
    while (!m_channels.empty())
    {
        Channel* ch = *m_channels.begin();
        m_channels.erase(m_channels.begin());               // remove from player's channel list
        ch->Leave(GetGUID(), false);                        // not send to client, not remove from player's channel list
        if (ChannelMgr* cMgr = channelMgr(GetTeam()))
            cMgr->LeftChannel(ch->GetName());               // deleted channel if empty
    }
    sLog.outDebug("Player: channels cleaned up!");
}

void Player::UpdateLocalChannels(uint32 newZone)
{
    if (m_channels.empty())
        return;

    AreaTableEntry const* current_zone = GetAreaEntryByAreaID(newZone);
    if (!current_zone)
        return;

    ChannelMgr* cMgr = channelMgr(GetTeam());
    if (!cMgr)
        return;

    std::string current_zone_name = current_zone->area_name[GetSession()->GetSessionDbcLocale()];

    for (JoinedChannelsList::iterator i = m_channels.begin(), next; i != m_channels.end(); i = next)
    {
        next = i; ++next;

        // skip non built-in channels
        if (!(*i)->IsConstant())
            continue;

        ChatChannelsEntry const* ch = GetChannelEntryFor((*i)->GetChannelId());
        if (!ch)
            continue;

        if (ch->flags & CHANNEL_DBC_FLAG_GLOBAL)//Global channels
            continue;

        if ((ch->flags & CHANNEL_DBC_FLAG_TRADE) && sWorld.getConfig(CONFIG_GLOBAL_TRADE_CHANNEL))//trade channel
            continue;

        //  new channel
        char new_channel_name_buf[100];
        snprintf(new_channel_name_buf,100,ch->pattern[m_session->GetSessionDbcLocale()],current_zone_name.c_str());
        Channel* new_channel = cMgr->GetJoinChannel(new_channel_name_buf,ch->ChannelID);

        if ((*i)!=new_channel)
        {
            new_channel->Join(GetGUID(),"");                // will output Changed Channel: N. Name
            //if ((*i)->IsLFG())
            //    sLog.outLog(LOG_CHEAT," Left LFG channel 2");
            // leave old channel
            (*i)->Leave(GetGUID(),false);                   // not send leave channel, it already replaced at client
            std::string name = (*i)->GetName();             // store name, (*i)erase in LeftChannel
            LeftChannel(*i);                                // remove from player's channel list
            cMgr->LeftChannel(name);                        // delete if empty
        }
    }
    sLog.outDebug("Player: channels cleaned up!");
}

void Player::LeaveLFGChannel()
{
    if (!sWorld.getConfig(CONFIG_RESTRICTED_LFG_CHANNEL) || GetSession()->HasPermissions(PERM_GMT))
        return;

    // don't kick if on lfg or lfm
    if (!m_lookingForGroup.Empty())
        return;

    for (JoinedChannelsList::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
    {
        if ((*i)->IsLFG())
        {
            (*i)->Leave(GetGUID());
            break;
        }
    }
}

void Player::JoinLFGChannel()
{
    if (m_lookingForGroup.Empty())
        return;

    for (JoinedChannelsList::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
        if ((*i)->IsLFG())
            return;

    /*if (ChannelMgr* cMgr = channelMgr(GetTeam()))
        if (Channel *chn = cMgr->GetJoinChannel("LookingForGroup", 26))
            chn->Invite(GetGUID(), GetName());*/

    WorldPacket data;

    data.Initialize(SMSG_CHANNEL_NOTIFY, 17);   // "LookingForGroup count + 2
    data << uint8(0x18);        // CHAT_INVITE_NOTICE
    data << "LookingForGroup";  // channel name
    data << GetGUID();          // player guid

    SendPacketToSelf(&data);
}

void Player::UpdateDefense()
{
    uint32 defense_skill_gain = sWorld.getConfig(CONFIG_SKILL_GAIN_DEFENSE);
    if (sWorld.getConfig(CONFIG_FACTION_MINORITY) == GetTeam())
        defense_skill_gain *= 2;

    if (UpdateSkill(SKILL_DEFENSE,defense_skill_gain))
    {
        // update dependent from defense skill part
        UpdateDefenseBonusesMod();
    }
}

void Player::HandleBaseModValue(BaseModGroup modGroup, BaseModType modType, float amount, bool apply)
{
    if (modGroup >= BASEMOD_END || modType >= MOD_END)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ERROR in HandleBaseModValue(): non existed BaseModGroup of wrong BaseModType!");
        return;
    }

    float val = 1.0f;

    switch (modType)
    {
        case FLAT_MOD:
            m_auraBaseMod[modGroup][modType] += apply ? amount : -amount;
            break;
        case PCT_MOD:
            if (amount <= -100.0f)
                amount = -200.0f;

            val = (100.0f + amount) / 100.0f;
            m_auraBaseMod[modGroup][modType] *= apply ? val : (1.0f/val);
            break;
    }

    if (!CanModifyStats())
        return;

    switch (modGroup)
    {
        case CRIT_PERCENTAGE:              UpdateCritPercentage(BASE_ATTACK);                          break;
        case RANGED_CRIT_PERCENTAGE:       UpdateCritPercentage(RANGED_ATTACK);                        break;
        case OFFHAND_CRIT_PERCENTAGE:      UpdateCritPercentage(OFF_ATTACK);                           break;
        case SHIELD_BLOCK_VALUE:           UpdateShieldBlockValue();                                   break;
        default: break;
    }
}

float Player::GetBaseModValue(BaseModGroup modGroup, BaseModType modType) const
{
    if (modGroup >= BASEMOD_END || modType > MOD_END)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: trial to access non existed BaseModGroup or wrong BaseModType!");
        return 0.0f;
    }

    if (modType == PCT_MOD && m_auraBaseMod[modGroup][PCT_MOD] <= 0.0f)
        return 0.0f;

    return m_auraBaseMod[modGroup][modType];
}

float Player::GetTotalBaseModValue(BaseModGroup modGroup) const
{
    if (modGroup >= BASEMOD_END)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: wrong BaseModGroup in GetTotalBaseModValue()!");
        return 0.0f;
    }

    if (m_auraBaseMod[modGroup][PCT_MOD] <= 0.0f)
        return 0.0f;

    return m_auraBaseMod[modGroup][FLAT_MOD] * m_auraBaseMod[modGroup][PCT_MOD];
}

uint32 Player::GetShieldBlockValue() const
{
    BaseModGroup modGroup = SHIELD_BLOCK_VALUE;

    float value = (m_auraBaseMod[SHIELD_BLOCK_VALUE][FLAT_MOD] + GetStat(STAT_STRENGTH) * 0.05f - 1)*m_auraBaseMod[SHIELD_BLOCK_VALUE][PCT_MOD];

    value = (value < 0) ? 0 : value;

    return uint32(value);
}

float Player::GetMeleeCritFromAgility()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level > GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    GtChanceToMeleeCritBaseEntry const* critBase = sGtChanceToMeleeCritBaseStore.LookupEntry(pclass - 1);
    GtChanceToMeleeCritEntry     const* critRatio = sGtChanceToMeleeCritStore.LookupEntry((pclass - 1) * GT_MAX_LEVEL + level - 1);
    if (critBase == NULL || critRatio == NULL)
        return 0.0f;

    float crit = critBase->base + GetStat(STAT_AGILITY) * critRatio->ratio;
    return crit * 100.0f;
}

float Player::GetDodgeFromAgility()
{
    // Table for base dodge values
    float dodge_base[MAX_CLASSES] = {
         0.0075f,   // Warrior
         0.00652f,  // Paladin
        -0.0545f,   // Hunter
        -0.0059f,   // Rogue
         0.03183f,  // Priest
         0.0114f,   // DK
         0.0167f,   // Shaman
         0.034575f, // Mage
         0.02011f,  // Warlock
         0.0f,      // ??
        -0.0187f    // Druid
    };
    // Crit/agility to dodge/agility coefficient multipliers
    float crit_to_dodge[MAX_CLASSES] = {
         1.1f,      // Warrior
         1.0f,      // Paladin
         1.6f,      // Hunter
         2.0f,      // Rogue
         1.0f,      // Priest
         1.0f,      // DK?
         1.0f,      // Shaman
         1.0f,      // Mage
         1.0f,      // Warlock
         0.0f,      // ??
         1.7f       // Druid
    };

    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level > GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    // Dodge per agility for most classes equal crit per agility (but for some classes need apply some multiplier)
    GtChanceToMeleeCritEntry  const* dodgeRatio = sGtChanceToMeleeCritStore.LookupEntry((pclass - 1) * GT_MAX_LEVEL + level - 1);
    if (dodgeRatio == NULL || pclass > MAX_CLASSES)
        return 0.0f;

    float dodge = dodge_base[pclass - 1] + GetStat(STAT_AGILITY) * dodgeRatio->ratio * crit_to_dodge[pclass - 1];
    return dodge * 100.0f;
}

float Player::GetSpellCritFromIntellect()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level > GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    GtChanceToSpellCritBaseEntry const* critBase = sGtChanceToSpellCritBaseStore.LookupEntry(pclass - 1);
    GtChanceToSpellCritEntry     const* critRatio = sGtChanceToSpellCritStore.LookupEntry((pclass - 1) * GT_MAX_LEVEL + level - 1);
    if (critBase == NULL || critRatio == NULL)
        return 0.0f;

    float crit = critBase->base + GetStat(STAT_INTELLECT) * critRatio->ratio;
    return crit * 100.0f;
}

float Player::GetRatingCoefficient(CombatRating cr) const
{
    uint32 level = GetLevel();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    GtCombatRatingsEntry const *Rating = sGtCombatRatingsStore.LookupEntry(cr*GT_MAX_LEVEL+level-1);
    if (Rating == NULL)
        return 1.0f;                                        // By default use minimum coefficient (not must be called)

    return Rating->ratio;
}

float Player::GetRatingBonusValue(CombatRating cr) const
{
    return float(GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + cr)) / GetRatingCoefficient(cr);
}

uint32 Player::GetMeleeCritDamageReduction(uint32 damage) const
{
    float melee  = GetRatingBonusValue(CR_CRIT_TAKEN_MELEE)*2.0f;
    if (melee>25.0f) melee = 25.0f;
    return uint32 (melee * damage /100.0f);
}

uint32 Player::GetRangedCritDamageReduction(uint32 damage) const
{
    float ranged = GetRatingBonusValue(CR_CRIT_TAKEN_RANGED)*2.0f;
    if (ranged>25.0f) ranged=25.0f;
    return uint32 (ranged * damage /100.0f);
}

uint32 Player::GetSpellCritDamageReduction(uint32 damage) const
{
    float spell = GetRatingBonusValue(CR_CRIT_TAKEN_SPELL)*2.0f;
    // In wow script resilience limited to 25%
    if (spell>25.0f)
        spell = 25.0f;
    return uint32 (spell * damage / 100.0f);
}

uint32 Player::GetDotDamageReduction(uint32 damage) const
{
    float spellDot = GetRatingBonusValue(CR_CRIT_TAKEN_SPELL);
    // Dot resilience not limited (limit it by 100%)
    if (spellDot > 100.0f)
        spellDot = 100.0f;
    return uint32 (spellDot * damage / 100.0f);
}

float Player::GetExpertiseDodgeOrParryReduction(WeaponAttackType attType) const
{
    switch (attType)
    {
        case BASE_ATTACK:
            return GetUInt32Value(PLAYER_EXPERTISE) / 4.0f;
        case OFF_ATTACK:
            return GetUInt32Value(PLAYER_OFFHAND_EXPERTISE) / 4.0f;
        default:
            break;
    }
    return 0.0f;
}

float Player::OCTRegenHPPerSpirit()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    GtOCTRegenHPEntry     const *baseRatio = sGtOCTRegenHPStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    GtRegenHPPerSptEntry  const *moreRatio = sGtRegenHPPerSptStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    if (baseRatio==NULL || moreRatio==NULL)
        return 0.0f;

    // Formula from PaperDollFrame script
    float spirit = GetStat(STAT_SPIRIT);
    float baseSpirit = spirit;
    if (baseSpirit>50) baseSpirit = 50;
    float moreSpirit = spirit - baseSpirit;
    float regen = baseSpirit * baseRatio->ratio + moreSpirit * moreRatio->ratio;
    return regen;
}

float Player::OCTRegenMPPerSpirit()
{
    uint32 level = GetLevel();
    uint32 pclass = GetClass();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

//    GtOCTRegenMPEntry     const *baseRatio = sGtOCTRegenMPStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    GtRegenMPPerSptEntry  const *moreRatio = sGtRegenMPPerSptStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    if (moreRatio==NULL)
        return 0.0f;

    // Formula get from PaperDollFrame script
    float spirit    = GetStat(STAT_SPIRIT);
    float regen     = spirit * moreRatio->ratio;
    return regen;
}

void Player::ApplyRatingMod(CombatRating cr, int32 value, bool apply)
{
    ApplyModUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + cr, value, apply);

    float RatingCoeffecient = GetRatingCoefficient(cr);
    float RatingChange = 0.0f;

    bool affectStats = CanModifyStats();

    switch (cr)
    {
        case CR_WEAPON_SKILL:                               // Implemented in Unit::RollMeleeOutcomeAgainst
        case CR_DEFENSE_SKILL:
            UpdateDefenseBonusesMod();
            break;
        case CR_DODGE:
            UpdateDodgePercentage();
            break;
        case CR_PARRY:
            UpdateParryPercentage();
            break;
        case CR_BLOCK:
            UpdateBlockPercentage();
            break;
        case CR_HIT_MELEE:
            RatingChange = value / RatingCoeffecient;
            m_modMeleeHitChance += apply ? RatingChange : -RatingChange;
            break;
        case CR_HIT_RANGED:
            RatingChange = value / RatingCoeffecient;
            m_modRangedHitChance += apply ? RatingChange : -RatingChange;
            break;
        case CR_HIT_SPELL:
            RatingChange = value / RatingCoeffecient;
            m_modSpellHitChance += apply ? RatingChange : -RatingChange;
            break;
        case CR_CRIT_MELEE:
            if (affectStats)
            {
                UpdateCritPercentage(BASE_ATTACK);
                UpdateCritPercentage(OFF_ATTACK);
            }
            break;
        case CR_CRIT_RANGED:
            if (affectStats)
                UpdateCritPercentage(RANGED_ATTACK);
            break;
        case CR_CRIT_SPELL:
            if (affectStats)
                UpdateAllSpellCritChances();
            break;
        case CR_HIT_TAKEN_MELEE:                            // Implemented in Unit::MeleeMissChanceCalc
        case CR_HIT_TAKEN_RANGED:
            break;
        case CR_HIT_TAKEN_SPELL:                            // Implemented in Unit::MagicSpellHitResult
            break;
        case CR_CRIT_TAKEN_MELEE:                           // Implemented in Unit::RollMeleeOutcomeAgainst (only for chance to crit)
        case CR_CRIT_TAKEN_RANGED:
            break;
        case CR_CRIT_TAKEN_SPELL:                           // Implemented in Unit::SpellCriticalBonus (only for chance to crit)
            break;
        case CR_HASTE_MELEE:
            RatingChange = value / RatingCoeffecient;
            ApplyAttackTimePercentMod(BASE_ATTACK, RatingChange, apply);
            ApplyAttackTimePercentMod(OFF_ATTACK, RatingChange, apply);
            break;
        case CR_HASTE_RANGED:
            RatingChange = value / RatingCoeffecient;
            ApplyAttackTimePercentMod(RANGED_ATTACK, RatingChange, apply);
            break;
        case CR_HASTE_SPELL:
            RatingChange = value / RatingCoeffecient;
            ApplyCastTimePercentMod(RatingChange, apply);
            break;
        case CR_WEAPON_SKILL_MAINHAND:                      // Implemented in Unit::RollMeleeOutcomeAgainst
        case CR_WEAPON_SKILL_OFFHAND:
        case CR_WEAPON_SKILL_RANGED:
            break;
        case CR_EXPERTISE:
            if (affectStats)
            {
                UpdateExpertise(BASE_ATTACK);
                UpdateExpertise(OFF_ATTACK);
            }
            break;
    }
}

void Player::SetRegularAttackTime()
{
    for (int i = 0; i < MAX_ATTACK; ++i)
    {
        Item *tmpitem = GetWeaponForAttack(WeaponAttackType(i));
        if (tmpitem && !tmpitem->IsBroken())
        {
            ItemPrototype const *proto = tmpitem->GetProto();
            if (proto->Delay)
                SetAttackTime(WeaponAttackType(i), proto->Delay);
            else
                SetAttackTime(WeaponAttackType(i), BASE_ATTACK_TIME);
        }
    }
    if (!GetWeaponForAttack(BASE_ATTACK))
        SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);
}

//skill+step, checking for max value
bool Player::UpdateSkill(uint32 skill_id, uint32 step)
{
    if (!skill_id)
        return false;

    if (skill_id == SKILL_FIST_WEAPONS)
        skill_id = SKILL_UNARMED;

    uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill_id)
            break;

    if (i>=PLAYER_MAX_SKILLS)
        return false;

    uint32 data = GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i));
    uint32 value = SKILL_VALUE(data);
    uint32 max = SKILL_MAX(data);

    if ((!max) || (!value) || (value >= max))
        return false;

    //if (value*512 < max*urand(0,512))
    {
        uint32 new_value = value+step;
        if (new_value > max)
            new_value = max;

        SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i),MAKE_SKILL_VALUE(new_value,max));
        return true;
    }

    return false;
}

inline int SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if (SkillValue >= GrayLevel)
        return sWorld.getConfig(CONFIG_SKILL_CHANCE_GREY)*10;
    if (SkillValue >= GreenLevel)
        return sWorld.getConfig(CONFIG_SKILL_CHANCE_GREEN)*10;
    if (SkillValue >= YellowLevel)
        return sWorld.getConfig(CONFIG_SKILL_CHANCE_YELLOW)*10;
    return sWorld.getConfig(CONFIG_SKILL_CHANCE_ORANGE)*10;
}

bool Player::UpdateCraftSkill(uint32 spellid)
{
    sLog.outDebug("UpdateCraftSkill spellid %d", spellid);

    SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(spellid);
    SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(spellid);

    for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
    {
        if (_spell_idx->second->skillId)
        {
            uint32 SkillValue = GetPureSkillValue(_spell_idx->second->skillId);

            // Alchemy Discoveries here
            SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
            if (spellEntry && spellEntry->Mechanic==MECHANIC_DISCOVERY)
            {
                if (uint32 discoveredSpell = GetSkillDiscoverySpell(_spell_idx->second->skillId, spellid, this))
                    learnSpell(discoveredSpell);
            }

            uint32 craft_skill_gain = sWorld.getConfig(CONFIG_SKILL_GAIN_CRAFTING);
            if ((rand() % 2) && sWorld.getConfig(CONFIG_FACTION_MINORITY) == GetTeam())
                craft_skill_gain *= 2;

            return UpdateSkillPro(_spell_idx->second->skillId, SkillGainChance(SkillValue,
                _spell_idx->second->max_value,
                (_spell_idx->second->max_value + _spell_idx->second->min_value)/2,
                _spell_idx->second->min_value),
                craft_skill_gain);
        }
    }
    return false;
}

bool Player::UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator)
{
    sLog.outDebug("UpdateGatherSkill(SkillId %d SkillLevel %d RedLevel %d)", SkillId, SkillValue, RedLevel);

    uint32 gathering_skill_gain = sWorld.getConfig(CONFIG_SKILL_GAIN_GATHERING);
    if ((rand() % 2) && sWorld.getConfig(CONFIG_FACTION_MINORITY) == GetTeam())
        gathering_skill_gain *= 2;

    // For skinning and Mining chance decrease with level. 1-74 - no decrease, 75-149 - 2 times, 225-299 - 8 times
    switch (SkillId)
    {
        case SKILL_HERBALISM:
        case SKILL_LOCKPICKING:
        case SKILL_JEWELCRAFTING:
            return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator,gathering_skill_gain);
        case SKILL_SKINNING:
            if (sWorld.getConfig(CONFIG_SKILL_CHANCE_SKINNING_STEPS)==0)
                return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator,gathering_skill_gain);
            else
                return UpdateSkillPro(SkillId, (SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator) >> (SkillValue/sWorld.getConfig(CONFIG_SKILL_CHANCE_SKINNING_STEPS)), gathering_skill_gain);
        case SKILL_MINING:
            if (sWorld.getConfig(CONFIG_SKILL_CHANCE_MINING_STEPS)==0)
                return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator,gathering_skill_gain);
            else
                return UpdateSkillPro(SkillId, (SkillGainChance(SkillValue, RedLevel+100, RedLevel+50, RedLevel+25)*Multiplicator) >> (SkillValue/sWorld.getConfig(CONFIG_SKILL_CHANCE_MINING_STEPS)),gathering_skill_gain);
    }
    return false;
}

bool Player::UpdateFishingSkill()
{
    sLog.outDebug("UpdateFishingSkill");

    uint32 SkillValue = GetPureSkillValue(SKILL_FISHING);

    int32 chance = SkillValue < 75 ? 100 : 2500/(SkillValue-50);

    uint32 gathering_skill_gain = sWorld.getConfig(CONFIG_SKILL_GAIN_GATHERING);
    if ((rand() % 2) && sWorld.getConfig(CONFIG_FACTION_MINORITY) == GetTeam())
        gathering_skill_gain *= 2;

    return UpdateSkillPro(SKILL_FISHING,chance*10,gathering_skill_gain);
}

bool Player::UpdateSkillPro(uint16 SkillId, int32 Chance, uint32 step)
{
    sLog.outDebug("UpdateSkillPro(SkillId %d, Chance %3.1f%%)", SkillId, Chance/10.0);
    if (!SkillId)
        return false;

    if (Chance <= 0)                                         // speedup in 0 chance case
    {
        sLog.outDebug("Player::UpdateSkillPro Chance=%3.1f%% missed", Chance/10.0);
        return false;
    }

    uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
        if (SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_INDEX(i))) == SkillId) break;
    if (i >= PLAYER_MAX_SKILLS)
        return false;

    uint32 data = GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i));
    uint16 SkillValue = SKILL_VALUE(data);
    uint16 MaxValue   = SKILL_MAX(data);

    if (!MaxValue || !SkillValue || SkillValue >= MaxValue)
        return false;

    int32 Roll = irand(1,1000);

    if (Roll <= Chance)
    {
        uint32 new_value = SkillValue+step;
        if (new_value > MaxValue)
            new_value = MaxValue;

        SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i),MAKE_SKILL_VALUE(new_value,MaxValue));
        sLog.outDebug("Player::UpdateSkillPro Chance=%3.1f%% taken", Chance/10.0);
        return true;
    }

    sLog.outDebug("Player::UpdateSkillPro Chance=%3.1f%% missed", Chance/10.0);
    return false;
}

void Player::UpdateWeaponSkill (WeaponAttackType attType)
{

    if (IsInFeralForm(true))
        return;                                             // always maximized SKILL_FERAL_COMBAT in fact

    if (m_form == FORM_TREE)
        return;                                             // use weapon but not skill up

    uint32 weapon_skill_gain = sWorld.getConfig(CONFIG_SKILL_GAIN_WEAPON);
    if (sWorld.getConfig(CONFIG_FACTION_MINORITY) == GetTeam())
        weapon_skill_gain *= 2;

    switch (attType)
    {
        case BASE_ATTACK:
        {
            Item *tmpitem = GetWeaponForAttack(attType,true);

            if (!tmpitem)
                UpdateSkill(SKILL_UNARMED,weapon_skill_gain);
            else if (tmpitem->GetProto()->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE)
                UpdateSkill(tmpitem->GetSkill(),weapon_skill_gain);
            break;
        }
        case OFF_ATTACK:
        case RANGED_ATTACK:
        {
            Item *tmpitem = GetWeaponForAttack(attType,true);
            if (tmpitem)
                UpdateSkill(tmpitem->GetSkill(),weapon_skill_gain);
            break;
        }
    }
    UpdateAllCritPercentages();
}

void Player::UpdateCombatSkills(Unit *pVictim, WeaponAttackType attType, bool defence) //if defense than pVictim == attacker
{
    if (pVictim->isCharmedOwnedByPlayerOrPlayer()) // no skill ups in pvp
        return;

    uint32 plevel = GetLevel();
    uint32 moblevel = pVictim->getLevelForTarget(this);

    if (moblevel > plevel + 5)
        moblevel = plevel + 5;

    float lvldif = 1.5f;
    if (moblevel >= plevel)
    {
        lvldif = moblevel - plevel;
        if (lvldif < 3)
            lvldif = 3;
    }

    uint32 skilldif = 5 * plevel - (defence ? GetPureSkillValue(SKILL_DEFENSE) : GetBaseWeaponSkillValue(attType));
    if (skilldif <= 0)
        return;

    float chance = (float(lvldif * skilldif) / plevel) * 1.5f * GetXPRate(RATE_XP_KILL);
    if (!defence)
        chance += 0.02f * GetStat(STAT_INTELLECT);

    chance = chance < 1.0f ? 1.0f : chance;                 //minimum chance to increase skill is 1%

    if (!defence)
        SendCombatStats(1<<COMBAT_STATS_WEAPON_SKILL, "Weapon skill update [ skill: %u, chance %f, skilldif: %u ]", pVictim, attType, chance, skilldif);

    if (roll_chance_f(chance))
    {
        if (defence)
            UpdateDefense();
        else
            UpdateWeaponSkill(attType);
    }
}

void Player::ModifySkillBonus(uint32 skillid,int32 val, bool talent)
{
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skillid)
    {
        uint32 bonus_val = GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i));
        int16 temp_bonus = SKILL_TEMP_BONUS(bonus_val);
        int16 perm_bonus = SKILL_PERM_BONUS(bonus_val);

        if (talent)                                          // permanent bonus stored in high part
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i),MAKE_SKILL_BONUS(temp_bonus,perm_bonus+val));
        else                                                // temporary/item bonus stored in low part
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i),MAKE_SKILL_BONUS(temp_bonus+val,perm_bonus));
        return;
    }
}

void Player::UpdateSkillsForLevel()
{
    uint16 maxconfskill = sWorld.GetConfigMaxSkillValue();
    uint32 maxSkill = GetMaxSkillValueForLevel();

    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
        if (GetUInt32Value(PLAYER_SKILL_INDEX(i)))
    {
        uint32 pskill = GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF;

        SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(pskill);
        if (!pSkill)
            continue;

        if (GetSkillRangeType(pSkill,false) != SKILL_RANGE_LEVEL)
            continue;

        uint32 data = GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i));
        uint32 max = SKILL_MAX(data);
        uint32 val = SKILL_VALUE(data);

        /// update only level dependent max skill values
        if (max!=1 && max!=maxconfskill)
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i),MAKE_SKILL_VALUE(val,maxSkill));
    }
}

void Player::SetWeaponSkillsToMax()
{
    for (uint16 i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        uint32 index = GetUInt32Value(PLAYER_SKILL_INDEX(i));
        if (index)
        {
            uint32 pskill = index & 0x0000FFFF;

            bool combat_skills = false;
            switch (pskill)
            {
            case SKILL_SWORDS:
            case SKILL_AXES:
            case SKILL_BOWS:
            case SKILL_GUNS:
            case SKILL_MACES:
            case SKILL_2H_SWORDS:
            case SKILL_DEFENSE:
            case SKILL_STAVES:
            case SKILL_2H_MACES:
            case SKILL_UNARMED:
            case SKILL_2H_AXES:
            case SKILL_DAGGERS:
            case SKILL_THROWN:
            case SKILL_CROSSBOWS:
            case SKILL_SPEARS:
            case SKILL_WANDS:
            case SKILL_POLEARMS:
            case SKILL_FIST_WEAPONS:
                combat_skills = true;
            }

            if (!combat_skills)
                continue;

            uint32 data = GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i));
            uint32 max = SKILL_MAX(data);

            if (max > 1)
                SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i), MAKE_SKILL_VALUE(max, max));

            if (pskill == SKILL_DEFENSE)
                UpdateDefenseBonusesMod();
        }
    }
}

// This functions sets a skill line value (and adds if doesn't exist yet)
// To "remove" a skill line, set it's values to zero
void Player::SetSkill(uint32 id, uint16 currVal, uint16 maxVal)
{
    if (!id)
        return;

    uint16 i=0;
    for (; i < PLAYER_MAX_SKILLS; i++)
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == id) break;

    if (i<PLAYER_MAX_SKILLS)                                 //has skill
    {
        if (currVal)
        {
            if (sWorld.getConfig(CONFIG_ALWAYS_MAX_WEAPON_SKILL) && !SpellMgr::IsPrimaryProfessionSkill(id))
                SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i), MAKE_SKILL_VALUE(maxVal, maxVal));
            else
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i),MAKE_SKILL_VALUE(currVal, maxVal));
        }
        else                                                //remove
        {
            // clear skill fields
            SetUInt32Value(PLAYER_SKILL_INDEX(i),0);
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i),0);
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i),0);

            // remove spells that depend on this skill when removing the skill
            for (PlayerSpellMap::const_iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
            {
                ++next;
                if (itr->second.state == PLAYERSPELL_REMOVED)
                    continue;

                SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(itr->first);
                SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(itr->first);

                for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
                {
                    if (_spell_idx->second->skillId == id)
                    {
                        // this may remove more than one spell (dependents)
                        removeSpell(itr->first);
                        next = m_spells.begin();
                        break;
                    }
                }
            }

            // remove quests that require this skill
            for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            {
                uint32 questId = GetQuestSlotQuestId(slot);
                if (!questId)
                    continue;

                Quest const* qInfo = sObjectMgr.GetQuestTemplate(questId);
                if (!qInfo)
                    continue;

                if (qInfo->GetSkillOrClass() == (int32)id)
                {
                    SetQuestSlot(slot, 0);

                    // we ignore unequippable quest items in this case, its' still be equipped
                    TakeQuestSourceItem(questId, false);

                    SetQuestStatus(questId, QUEST_STATUS_NONE);
                }
            }
        }
    }
    else if (currVal)                                        //add
    {
        for (i=0; i < PLAYER_MAX_SKILLS; i++)
            if (!GetUInt32Value(PLAYER_SKILL_INDEX(i)))
        {
            SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(id);
            if (!pSkill)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Skill not found in SkillLineStore: skill #%u", id);
                return;
            }
            // enable unlearn button for primary professions only
            if (pSkill->categoryId == SKILL_CATEGORY_PROFESSION)
                SetUInt32Value(PLAYER_SKILL_INDEX(i), MAKE_PAIR32(id,1));
            else
                SetUInt32Value(PLAYER_SKILL_INDEX(i), MAKE_PAIR32(id,0));
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i),MAKE_SKILL_VALUE(currVal,maxVal));

            // apply skill bonuses
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i),0);

            // temporary bonuses
            AuraList const& mModSkill = GetAurasByType(SPELL_AURA_MOD_SKILL);
            for (AuraList::const_iterator j = mModSkill.begin(); j != mModSkill.end(); ++j)
                if ((*j)->GetModifier()->m_miscvalue == int32(id))
                    (*j)->ApplyModifier(true);

            // permanent bonuses
            AuraList const& mModSkillTalent = GetAurasByType(SPELL_AURA_MOD_SKILL_TALENT);
            for (AuraList::const_iterator j = mModSkillTalent.begin(); j != mModSkillTalent.end(); ++j)
                if ((*j)->GetModifier()->m_miscvalue == int32(id))
                    (*j)->ApplyModifier(true);

            // Learn all spells for skill
            learnSkillRewardedSpells(id);
            return;
        }
    }
}

bool Player::HasSkill(uint32 skill) const
{
    if (!skill)return false;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill)
        {
            return true;
        }
    }
    return false;
}

uint16 Player::GetSkillValue(uint32 skill) const
{
    if (!skill)
        return 0;

    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill)
        {
            uint32 bonus = GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i));

            int32 result = int32(SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i))));
            result += SKILL_TEMP_BONUS(bonus);
            result += SKILL_PERM_BONUS(bonus);
            return result < 0 ? 0 : result;
        }
    }
    return 0;
}

uint16 Player::GetMaxSkillValue(uint32 skill) const
{
    if (!skill)return 0;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill)
        {
            uint32 bonus = GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i));

            int32 result = int32(SKILL_MAX(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i))));
            result += SKILL_TEMP_BONUS(bonus);
            result += SKILL_PERM_BONUS(bonus);
            return result < 0 ? 0 : result;
        }
    }
    return 0;
}

uint16 Player::GetPureMaxSkillValue(uint32 skill) const
{
    if (!skill)return 0;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill)
        {
            return SKILL_MAX(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i)));
        }
    }
    return 0;
}

uint16 Player::GetBaseSkillValue(uint32 skill) const
{
    if (!skill)return 0;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill)
        {
            int32 result = int32(SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i))));
            result +=  SKILL_PERM_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i)));
            return result < 0 ? 0 : result;
        }
    }
    return 0;
}

uint16 Player::GetPureSkillValue(uint32 skill) const
{
    if (!skill)return 0;
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill)
        {
            return SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i)));
        }
    }
    return 0;
}

int16 Player::GetSkillTempBonusValue(uint32 skill) const
{
    if (!skill)
        return 0;

    for (int i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        if ((GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF) == skill)
        {
            return SKILL_TEMP_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i)));
        }
    }

    return 0;
}

void Player::SendActionButtons(uint32 state) const
{   
    /*
    state can be 0, 1
    0 - Clears the action bars client sided. This is sent during spec swap before unlearning and before sending the new buttons. Doesn't work in 2.4.3
    1 - Used in any SMSG_ACTION_BUTTONS packet with button data.
    */

    WorldPacket data(SMSG_ACTION_BUTTONS, (MAX_ACTION_BUTTONS*4));

    if (state) {
        for (uint8 button = 0; button < MAX_ACTION_BUTTONS; ++button)
        {
            ActionButtonList::const_iterator itr = m_actionButtons.find(button);
            if (itr != m_actionButtons.end() && itr->second.uState != ACTIONBUTTON_DELETED)
            {
                data << uint16(itr->second.action);
                data << uint8(itr->second.misc);
                data << uint8(itr->second.type);
            }
            else
                data << uint32(0);
        }
    } else
        data << uint32(0);

    SendPacketToSelf(&data);
    sLog.outDetail("Action Buttons for '%u' Initialized", GetGUIDLow());
}

void Player::addActionButton(const uint8 button, const uint16 action, const uint8 type, const uint8 misc)
{
    if (button >= MAX_ACTION_BUTTONS)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Action %u not added into button %u for player %s: button must be < 132", action, button, GetName());
        return;
    }

    // check cheating with adding non-known spells to action bar
    if (type==ACTION_BUTTON_SPELL)
    {
        if (!sSpellTemplate.LookupEntry<SpellEntry>(action))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Action %u not added into button %u for player %s: spell not exist", action, button, GetName());
            return;
        }

        if (!HasSpell(action))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Action %u not added into button %u for player %s: player don't known this spell", action, button, GetName());
            return;
        }
    }

    ActionButtonList::iterator buttonItr = m_actionButtons.find(button);

    if (buttonItr==m_actionButtons.end())
    {                                                       // just add new button
        m_actionButtons[button] = ActionButton(action,type,misc);
    }
    else
    {                                                       // change state of current button
        ActionButtonUpdateState uState = buttonItr->second.uState;
        buttonItr->second = ActionButton(action,type,misc);
        if (uState != ACTIONBUTTON_NEW) buttonItr->second.uState = ACTIONBUTTON_CHANGED;
    };

    sLog.outDetail("Player '%u' Added Action '%u' to Button '%u'", GetGUIDLow(), action, button);
}

void Player::removeActionButton(uint8 button)
{
    ActionButtonList::iterator buttonItr = m_actionButtons.find(button);
    if (buttonItr==m_actionButtons.end())
        return;

    if (buttonItr->second.uState==ACTIONBUTTON_NEW)
        m_actionButtons.erase(buttonItr);                   // new and not saved
    else
        buttonItr->second.uState = ACTIONBUTTON_DELETED;    // saved, will deleted at next save

    sLog.outDetail("Action Button '%u' Removed from Player '%u'", button, GetGUIDLow());
}

bool Player::SetPosition(float x, float y, float z, float orientation, bool teleport)
{
    bool groupUpdate = (GetGroup() && (teleport || fabs(GetPositionX() - x) > 1.0f || fabs(GetPositionY() - y) > 1.0f));
    if (!Unit::SetPosition(x, y, z, orientation, teleport))
        return false;

    if(GetTrader() && !IsWithinDistInMap(GetTrader(), 5.0f))
        GetSession()->SendCancelTrade();

    if (!positionStatus.Passed())
        return true;

    positionStatus.Reset(100);

    // code block for underwater state update
    // Unit::SetPosition() checks for validity and updates our coordinates
    // so we re-fetch them instead of using "raw" coordinates from function params
    UpdateUnderwaterState(GetMap(), GetPositionX(), GetPositionY(), GetPositionZ());

    // group update
    if (groupUpdate)
        SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POSITION);

    CheckAreaExploreAndOutdoor();
    return true;
}

WorldLocation Player::GetSafeRecallPosition()
{
    WorldLocation loc;

    if (GetSession()->GetPermissions() > PERM_MODERATOR)
    {
        GetPosition(loc); // for GMs return current pos either way
        return loc;
    }

    bool ClosestGraveOrHome = false;

    Map* map = GetMap();
    // if in instance (any instance, including battlegrounds) -> find out of instance point
    if (map && map->Instanceable())
    {
        // if an instance is battleground -> use battlegroundEntryPoint as a reference
        if (map->IsBattleGroundOrArena())
        {
            loc = GetBattleGroundEntryPoint();
            return loc;
        }
        else 
            ClosestGraveOrHome = true;
    }
    else if (uint32 node_id = m_taxi.GetTaxiSource())
    {
        TaxiNodesEntry const* taxinode = sTaxiNodesStore.LookupEntry(node_id);
        if (taxinode)
        {
            loc.mapid = taxinode->map_id;
            loc.coord_x = taxinode->x;
            loc.coord_y = taxinode->y;
            loc.coord_z = taxinode->z;
            loc.orientation = GetOrientation();
            return loc;
        }
        else
            ClosestGraveOrHome = true;
    }

    // used for instances, raids and open maps when in flight. Do not use for battlegrounds
    if (ClosestGraveOrHome)
    {
        WorldSafeLocsEntry const *ClosestGrave = sObjectMgr.GetClosestGraveYard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetTeam());
        if (ClosestGrave)
        {
            loc.mapid = ClosestGrave->map_id;
            loc.coord_x = ClosestGrave->x;
            loc.coord_y = ClosestGrave->y;
            loc.coord_z = ClosestGrave->z;
            loc.orientation = GetOrientation();
        }
        // if no closest grave found -> use homebind coordinates of the player
        else
        {
            loc.mapid = m_homebindMapId;
            loc.coord_x = m_homebindX;
            loc.coord_y = m_homebindY;
            loc.coord_z = m_homebindZ;
            loc.orientation = GetOrientation();
        }
    }
    else // no map found AND not instanseable AND not flying, just use our current coordinates
        GetPosition(loc);

    return loc;
}

void Player::SendPacketToSelf(WorldPacket *data) const
{
    if (GetSession())
        GetSession()->SendPacket(data);
    else
        sLog.outLog(LOG_CRASH, "Crash on SendPacketToSelf possible - no session of player!");
}

void Player::SendCinematicStart(uint32 CinematicSequenceId)
{
    WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
    data << uint32(CinematicSequenceId);
    SendPacketToSelf(&data);

    setWatchingCinematic(CinematicSequenceId);
    GetCamera().UpdateVisibilityForOwner();
}

void Player::CheckAreaExploreAndOutdoor()
{
    if (!isAlive())
        return;

    if (IsTaxiFlying())
        return;

    bool isOutdoor;
    uint16 areaFlag = GetTerrain()->GetAreaFlag(GetPositionX(),GetPositionY(),GetPositionZ(), &isOutdoor);

    if (sWorld.getConfig(CONFIG_VMAP_INDOOR_CHECK) && m_outdoors != isOutdoor)
    {
        if (!isGameMaster())
        {
            if (!isOutdoor && sWorld.getConfig(CONFIG_VMAP_INDOOR_CHECK))
                RemoveAurasWithAttribute_NonPassive(SPELL_ATTR_OUTDOORS_ONLY);

            UpdateSpeed(MOVE_RUN, true);
            UpdateSpeed(MOVE_SWIM, true);
            UpdateSpeed(MOVE_FLIGHT, true);
        }

        m_outdoors = isOutdoor;
    }

    if (areaFlag==0xffff)
        return;

    int offset = areaFlag / 32;

    if (offset >= 128)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: Wrong area flag %u in map data for (X: %f Y: %f) point to field PLAYER_EXPLORED_ZONES_1 + %u (%u must be < 64).",areaFlag,GetPositionX(),GetPositionY(),offset,offset);
        return;
    }

    uint32 val = (uint32)(1 << (areaFlag % 32));
    uint32 currFields = GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    if (!(currFields & val))
    {
        SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

        AreaTableEntry const *p = GetAreaEntryByAreaFlagAndMap(areaFlag,GetMapId());
        if (!p)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: PLAYER: Player %u discovered unknown area (x: %f y: %f map: %u", GetGUIDLow(), GetPositionX(),GetPositionY(),GetMapId());
        }
        else if (p->area_level > 0)
        {
            uint32 area = p->ID;
            if (GetLevel() >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
            {
                SendExplorationExperience(area,0);
            }
            else
            {
                int32 diff = int32(GetLevel()) - p->area_level;
                uint32 XP = 0;
                if (diff < -5)
                {
                    XP = uint32(sObjectMgr.GetBaseXP(GetLevel()+5)*GetXPRate(RATE_XP_EXPLORE));
                }
                else if (diff > 5)
                {
                    int32 exploration_percent = (100-((diff-5)*5));
                    if (exploration_percent > 100)
                        exploration_percent = 100;
                    else if (exploration_percent < 0)
                        exploration_percent = 0;

                    XP = uint32(sObjectMgr.GetBaseXP(p->area_level)*exploration_percent/100*GetXPRate(RATE_XP_EXPLORE));
                }
                else
                {
                    XP = uint32(sObjectMgr.GetBaseXP(p->area_level)*GetXPRate(RATE_XP_EXPLORE));
                }

                GiveXP(XP, NULL);
                SendExplorationExperience(area,XP);
            }
            sLog.outDetail("PLAYER: Player %u discovered a new area: %u", GetGUIDLow(), area);
        }
    }
}

PlayerTeam Player::TeamForRace(uint8 race)
{
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    if (!rEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Race %u not found in DBC: wrong DBC files?",uint32(race));
        return ALLIANCE;
    }

    switch (rEntry->TeamID)
    {
        case 7: return ALLIANCE;
        case 1: return HORDE;
    }

    sLog.outLog(LOG_DEFAULT, "ERROR: Race %u have wrong team id %u in DBC: wrong DBC files?",uint32(race),rEntry->TeamID);
    return ALLIANCE;
}

uint32 Player::getFactionForRace(uint8 race)
{
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    if (!rEntry)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Race %u not found in DBC: wrong DBC files?",uint32(race));
        return 0;
    }

    return rEntry->FactionID;
}

void Player::setFactionForRace(uint8 race)
{
    setFaction(getFactionForRace(race));
}

ReputationRank Player::GetReputationRank(uint32 faction) const
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction);
    return GetReputationMgr().GetRank(factionEntry);
}

//Calculate total reputation percent player gain with quest/creature level
int32 Player::CalculateReputationGain(ReputationSource source, int32 rep, int32 faction, uint32 creatureOrQuestLevel, bool noAuraBonus)
{
    float percent = 100.0f;

    // should only work for reputation GAIN
    if (rep > 0)
    {
        float repMod = noAuraBonus ? 0.0f : (float)GetTotalAuraModifier(SPELL_AURA_MOD_REPUTATION_GAIN);

        // faction specific auras only seem to apply to kills
        if (source == REPUTATION_SOURCE_KILL)
            repMod += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_FACTION_REPUTATION_GAIN, faction);

        percent += repMod;
    }

    float rate = 1.0f;
    switch (source)
    {
        case REPUTATION_SOURCE_KILL:
            rate = sWorld.getConfig(RATE_REPUTATION_LOWLEVEL_KILL);
            break;
        case REPUTATION_SOURCE_QUEST:
            rate = sWorld.getConfig(RATE_REPUTATION_LOWLEVEL_QUEST);
            break;
    }

    if (rate != 1.0f && creatureOrQuestLevel <= Hellground::XP::GetGrayLevel(GetLevel()))
        percent *= rate;

    if (percent <= 0.0f)
        return 0;

    // Multiply result with the faction specific rate
    if (const RepRewardRate *repData = sObjectMgr.GetRepRewardRate(faction))
    {
        float repRate = 0.0f;
        switch (source)
        {
            case REPUTATION_SOURCE_KILL:
                repRate = repData->creature_rate;
                break;
            case REPUTATION_SOURCE_QUEST:
                repRate = repData->quest_rate;
                break;
            case REPUTATION_SOURCE_SPELL:
                repRate = repData->spell_rate;
                break;
            default:
                repRate = 1.0;
                break;
        }

        // for custom, a rate of 0.0 will totally disable reputation gain for this faction/type
        if (repRate <= 0.0f)
            return 0;

        percent *= repRate;
    }

    return int32(0.01f + sWorld.getConfig(RATE_REPUTATION_GAIN)*rep*percent/100.0f);
}

//Calculates how many reputation points player gains in victim's enemy factions
void Player::RewardReputation(Unit *pVictim, float rate)
{
    if (!pVictim || pVictim->GetTypeId() == TYPEID_PLAYER)
        return;

    if (((Creature*)pVictim)->IsReputationGainDisabled())
        return;

    ReputationOnKillEntry const* Rep = sObjectMgr.GetReputationOnKillEntry(((Creature*)pVictim)->GetCreatureInfo()->Entry);

    if (!Rep)
        return;

    if (Rep->repfaction1 && (!Rep->team_dependent || GetTeam()==ALLIANCE))
    {
        int32 donerep1 = CalculateReputationGain(REPUTATION_SOURCE_KILL, Rep->repvalue1, Rep->repfaction1, pVictim->GetLevel());
        donerep1 = int32(donerep1*rate);
        FactionEntry const *factionEntry1 = sFactionStore.LookupEntry(Rep->repfaction1);
        uint32 current_reputation_rank1 = GetReputationMgr().GetRank(factionEntry1);
        if (factionEntry1 && current_reputation_rank1 <= Rep->reputation_max_cap1)
            GetReputationMgr().ModifyReputation(factionEntry1, donerep1);

        // Wiki: Team factions value divided by 2
        if (factionEntry1 && Rep->is_teamaward1)
        {
            FactionEntry const *team1_factionEntry = sFactionStore.LookupEntry(factionEntry1->team);
            if (team1_factionEntry)
                GetReputationMgr().ModifyReputation(factionEntry1, donerep1 / 2);
        }
    }

    if (Rep->repfaction2 && (!Rep->team_dependent || GetTeam() == HORDE))
    {
        int32 donerep2 = CalculateReputationGain(REPUTATION_SOURCE_KILL, Rep->repvalue2, Rep->repfaction2, pVictim->GetLevel());
        donerep2 = int32(donerep2*rate);
        FactionEntry const *factionEntry2 = sFactionStore.LookupEntry(Rep->repfaction2);
        uint32 current_reputation_rank2 = GetReputationMgr().GetRank(factionEntry2);
        if (factionEntry2 && current_reputation_rank2 <= Rep->reputation_max_cap2)
            GetReputationMgr().ModifyReputation(factionEntry2, donerep2);

        // Wiki: Team factions value divided by 2
        if (factionEntry2 && Rep->is_teamaward2)
        {
            FactionEntry const *team2_factionEntry = sFactionStore.LookupEntry(factionEntry2->team);
            if (team2_factionEntry)
                GetReputationMgr().ModifyReputation(team2_factionEntry, donerep2 / 2);
        }
    }
}

//Calculate how many reputation points player gain with the quest
void Player::RewardReputation(Quest const *pQuest)
{
    // quest reputation reward/loss
    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        if (!pQuest->RewRepFaction[i])
            continue;

        if (pQuest->RewRepValue[i])
        {
            int32 rep = CalculateReputationGain(REPUTATION_SOURCE_QUEST, pQuest->RewRepValue[i], pQuest->RewRepFaction[i], GetQuestOrPlayerLevel(pQuest));
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(pQuest->RewRepFaction[i]))
                GetReputationMgr().ModifyReputation(factionEntry, rep);
        }
    }

    // TODO: implement reputation spillover
}

void Player::UpdateArenaFields(void)
{
    /* arena calcs go here */
}

void Player::UpdatePvpTitles()
{
    uint32 kills = GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
    uint64 titles = GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES);
    uint64 pvp_title = titles & PLAYER_TITLE_PVP;

    uint32 offset = TeamForRace(GetRace()) == HORDE ? MAX_PVP_RANKS : 0;

    uint64 new_title = 0;
    //bool search = pvp_title;
    for (uint32 i = 1 + offset; i <= MAX_PVP_RANKS + offset; ++i)
    {
        // first find the rank we already have
        /*if (search)
        {
            if (pvp_title & uint64(1) << i)
            {
                index = i;
                search = false;
            }
        }
        else*/
        //{
            if (kills >= sWorld.m_honorRanks[i-offset-1]) // case when saving old titles
                new_title += uint64(1) << i;
            else
                break;
        //}
    }
    
    if (pvp_title != new_title)
        SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles | new_title);
}

void Player::UpdateBgTitle()
{
    uint64 titles = GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES);

    uint32 index = GetUInt32Value(PLAYER_CHOSEN_TITLE); //shouldn't we use that ? i don't know, but we can try it

    if (GetTeam() == HORDE && (~titles & PLAYER_TITLE_CONQUEROR) && m_reputationMgr.GetRank(729) == REP_EXALTED && m_reputationMgr.GetRank(510) == REP_EXALTED && m_reputationMgr.GetRank(889) == REP_EXALTED)
    {
        SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles | PLAYER_TITLE_CONQUEROR);
        SetUInt32Value(PLAYER_CHOSEN_TITLE, index);
    }
    else if (GetTeam() == ALLIANCE && (~titles & PLAYER_TITLE_JUSTICAR) && m_reputationMgr.GetRank(730) == REP_EXALTED && m_reputationMgr.GetRank(509) == REP_EXALTED && m_reputationMgr.GetRank(890) == REP_EXALTED)
    {
        SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles | PLAYER_TITLE_JUSTICAR);
        SetUInt32Value(PLAYER_CHOSEN_TITLE, index);
    }
}

void Player::UpdateHonorFields()
{
    /// called when rewarding honor and at each save
    uint64 now = time(NULL);
    uint64 today = uint64(time(NULL) / DAY) * DAY;

    if (m_lastHonorUpdateTime < today)
    {
        uint64 yesterday = today - DAY;

        uint16 kills_today = PAIR32_LOPART(GetUInt32Value(PLAYER_FIELD_KILLS));

        // update yesterday's contribution
        if (m_lastHonorUpdateTime >= yesterday)
        {
            SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, GetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION));

            // this is the first update today, reset today's contribution
            SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
            SetUInt32Value(PLAYER_FIELD_KILLS, MAKE_PAIR32(0,kills_today));
        }
        else
        {
            // no honor/kills yesterday or today, reset
            SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);
            SetUInt32Value(PLAYER_FIELD_KILLS, 0);
        }
    }

    m_lastHonorUpdateTime = now;
}

void Player::UpdateConsecutiveKills()
{
    if (sWorld.getConfig(CONFIG_ENABLE_GANKING_PENALTY) == false)
        return;

    Map* map = GetMap();
    if (map == nullptr || map->IsBattleGroundOrArena())
        return;

    uint64 currentTime = sWorld.GetGameTime();
    uint64 expireTime = sWorld.getConfig(CONFIG_GANKING_PENALTY_EXPIRE);

    for (auto itr = m_consecutiveKills.begin(); itr != m_consecutiveKills.end();)
    {
        uint32 diff = currentTime - itr->second.second;
        if (diff > expireTime)
            m_consecutiveKills.erase(itr++);
        else
            ++itr;
    }
}

void Player::AddConsecutiveKill(uint64 victimGuid)
{
    if (sWorld.getConfig(CONFIG_ENABLE_GANKING_PENALTY) == false)
        return;

    auto itr = m_consecutiveKills.find(victimGuid);
    if (itr == m_consecutiveKills.end())
        return void(m_consecutiveKills.insert(std::make_pair(victimGuid, std::make_pair(1, sWorld.GetGameTime()))));

    ++itr->second.first;
    itr->second.second = sWorld.GetGameTime();
}

uint32 Player::GetConsecutiveKillsCount(uint64 victimGuid)
{
    auto itr = m_consecutiveKills.find(victimGuid);
    if (itr != m_consecutiveKills.end())
        return itr->second.first;

    return 0;
}

///Calculate the amount of honor gained based on the victim
///and the size of the group for which the honor is divided
///An exact honor value can also be given (overriding the calcs)
bool Player::RewardHonor(Unit *uVictim, uint32 groupsize, float honor, bool pvptoken, bool killer, bool force)
{
    // do not reward honor in world on x100, except mark quest
    //if (sWorld.isEasyRealm() && !force && !InBattleGroundOrArena())
    //    return false;
    
    // do not reward honor in arenas, but enable onkill spellproc
    if (InArena())
    {
        if (!uVictim || uVictim == this || uVictim->GetTypeId() != TYPEID_PLAYER)
            return false;

        if (GetBGTeam() == ((Player*)uVictim)->GetBGTeam())
            return false;

        return true;
    }

    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    //if (GetDummyAura(SPELL_AURA_PLAYER_INACTIVE))
    //    return false;

    uint64 victim_guid = 0;
    uint32 victim_rank = 0;
    //uint32 rank_diff = 0;
    time_t now = time(NULL);

    // need call before fields update to have chance move yesterday data to appropriate fields before today data change.
    UpdateHonorFields();

    // do not reward honor in arenas, but return true to enable onkill spellproc
    if (InBattleGroundOrArena() && GetBattleGround() && GetBattleGround()->isArena())
        return true;

    // don't reward honor in world
    if (!force && !InBattleGroundOrArena() && !sWorld.getConfig(CONFIG_HONOR_IN_WORLD))
        return false;

    uint8 DummyStack = 0;
    if (honor <= 0)
    {
        if (!uVictim || uVictim == this || uVictim->HasAuraType(SPELL_AURA_NO_PVP_CREDIT) || (uVictim->WorthHonor && uVictim->GetTypeId() == TYPEID_PLAYER && uVictim->ToPlayer()->InBattleGroundOrArena()) || uVictim->HasAura(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS))
            return false;

        //if (Aura * dummy = uVictim->GetDummyAura(55115))
        //    DummyStack = dummy->GetStackAmount();
        //if (DummyStack > 4)
        //    return false;

        victim_guid = uVictim->GetGUID();

        if (uVictim->GetTypeId() == TYPEID_PLAYER)
        {
            Player *pVictim = (Player *)uVictim;

            // In BG check by BG team
            if (InBattleGroundOrArena())
            {
                if (GetBGTeam() == pVictim->GetBGTeam())
                    return false;
            }
            else if (GetTeam() == pVictim->GetTeam() && !sWorld.IsFFAPvPRealm()) // out of BG check by faction-team (FFA gets honor for everyone)
                return false;

            uint32 killsCount = GetConsecutiveKillsCount(victim_guid);

            AddConsecutiveKill(victim_guid);

            float f = 1;                                    //need for total kills (?? need more info)
            uint32 k_grey = 0;
            uint32 k_level = GetLevel();
            uint32 v_level = pVictim->GetLevel();

            {

                // PLAYER_CHOSEN_TITLE VALUES DESCRIPTION
                //  [0]      Just name
                //  [1..14]  Alliance honor titles and player name
                //  [15..28] Horde honor titles and player name
                //  [29..38] Other title and player name
                //  [39+]    Nothing
                uint32 victim_title = pVictim->GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES) & PLAYER_TITLE_PVP;//pVictim->GetUInt32Value(PLAYER_CHOSEN_TITLE);

                // Ranks:
                //  title[1..14]  -> rank[5..18]
                //  title[15..28] -> rank[5..18]
                //  title[other]  -> 0
                if (!killer || victim_title == 0)
                    victim_guid = 0;                        // Don't show HK: <rank> message, only log.
                else
                {
                    uint32 offset = TeamForRace(pVictim->GetRace()) == HORDE ? MAX_PVP_RANKS : 0;
                    for (uint32 i = 1 + offset; i <= MAX_PVP_RANKS + offset; ++i)
                    {
                        if (victim_title & (uint64(1) << i))
                        {
                            victim_rank = i - offset;
                            //break; // If we're saving old titles - do not break
                        }
                        else
                            break; // If we're saving old titles - break when no title
                    }
                }
            }

            if (k_level <= 5)
                k_grey = 0;
            else if (k_level <= 39)
                k_grey = k_level - 5 - k_level/10;
            else
                k_grey = k_level - 1 - k_level/5;

            if (v_level<=k_grey)
                return false;

            float diff_level = (k_level == k_grey) ? 1 : ((float(v_level) - float(k_grey)) / (float(k_level) - float(k_grey)));

            int32 v_rank =1;                                //need more info

            honor = ((f * diff_level * (190 + v_rank*10))/6);
            honor *= ((float)k_level) / 70.0f;              //factor of dependence on levels of the killer

            honor *= 1 + /*sWorld.getRate(RATE_PVP_RANK_EXTRA_HONOR)**/(((float)victim_rank) / 10.0f);
            honor *= ((5.0f-(float(DummyStack)))/5.0f);

            if (!InBattleGroundOrArena())
            {
                if (sWorld.getConfig(CONFIG_ENABLE_GANKING_PENALTY))
                    honor *= 1.0f - killsCount * sWorld.getConfig(CONFIG_GANKING_PENALTY_PER_KILL);

                if (killsCount >= sWorld.getConfig(CONFIG_GANKING_KILLS_ALERT))
                {
                    // warn killer about possible honor farming
                    //ChatHandler(this).PSendSysMessage(LANG_HONOR_FARMING_WARNING, pVictim->GetName());

                    std::stringstream stream;
                    std::string killer_ip = GetSession()->GetRemoteAddress().c_str();
                    std::string victim_ip = pVictim->GetSession()->GetRemoteAddress().c_str();

                    std::string sameip = (killer_ip == victim_ip) ? " [SAME IP]" : "";
                    stream << "Possible kill farm (killer: " << GetName() << ", victim: " << pVictim->GetName() << ") kills count: " << killsCount << sameip;
                
                    sWorld.SendGMText(LANG_POSSIBLE_CHEAT, stream.str().c_str(), GetName(), GetName());
                    sLog.outLog(LOG_CHEAT, "%s", stream.str().c_str());
                }

                if (honor <= 0.0f)
                    return false;
            }

            // count the number of playerkills in one day
            ApplyModUInt32Value(PLAYER_FIELD_KILLS, 1, true);
            // and those in a lifetime
            ApplyModUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 1, true);
        }
        else
        {
            Creature *cVictim = (Creature *)uVictim;

            if (!cVictim->isRacialLeader())
                return false;

            honor = 100;                                    // ??? need more info
            victim_rank = 15;                               // HK: Leader
        }

    }
    
    honor *= (GetTeam() == ALLIANCE ? sWorld.getConfig(RATE_HONOR_A) : sWorld.getConfig(RATE_HONOR_H));
	float init_honor = honor;

	// bonus honor in Alterac!
	if (GetMapId() == MAP_ALTERAC)
		honor += init_honor * 1.25; // 25%

	honor += CalculateBonus(honor);

	if (!sWorld.isEasyRealm())
	{
		tm localTm = *localtime(&sWorld.GetGameTime());
		if (localTm.tm_wday == 0 || localTm.tm_wday == 6) // 0-6 saturday sunday
			honor += init_honor / 2;
	}

    if (uVictim != NULL)
    {
        if (groupsize > 1)
            honor /= groupsize;

        honor *= (((float)urand(8,12))/10);                 // approx honor: 80% - 120% of real honor
        //RemoveAurasDueToSpell(55115); // Remove Left for Dead from the attacker
    }

    // honor - for show honor points in log
    // victim_guid - for show victim name in log
    // victim_rank [1..4]  HK: <dishonored rank>
    // victim_rank [5..19] HK: <alliance\horde rank>
    // victim_rank [0,20+] HK: <>
    WorldPacket data(SMSG_PVP_CREDIT,4+8+4);
    data << (uint32) honor;
    data << (uint64) victim_guid;
    data << (uint32) (victim_rank+4); // cause 1-4 is dishonored

    SendPacketToSelf(&data);

    // add honor points
    ModifyHonorPoints(int32(honor));
    UpdatePvpTitles();
    ApplyModUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, uint32(honor), true);

    std::string bg_name = "NOT_BG";
    if (GetBattleGround())
        bg_name = GetBattleGround()->GetName();

    sLog.outLog(LOG_ARENA_FLUSH, "Player %s received honor points: %u (init %u) in %s", GetName(), (uint32)honor, (uint32)init_honor, bg_name.c_str());

    //if (sWorld.getConfig(CONFIG_PVP_TOKEN_ENABLE) && pvptoken)
    //{
    //    if (!uVictim || uVictim == this || uVictim->HasAuraType(SPELL_AURA_NO_PVP_CREDIT) || 
    //        (uVictim->WorthHonor && uVictim->GetTypeId() == TYPEID_PLAYER && uVictim->ToPlayer()->InBattleGroundOrArena()) ||
    //        uVictim->HasAura(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS) ||
    //        sWorld.isEasyRealm() && !killer)
    //        return true;

    //    /*for (uint8 i = 0; i<100; i++)
    //        sLog.outLog(LOG_DEFAULT, "RANDOM %u from %u", urand(0,DummyStack), DummyStack);
    //    if (urand(0,4) < DummyStack)
    //        return true;*/

    //    if (DummyStack < 4)
    //        return true;

    //    if (uVictim->GetTypeId() == TYPEID_PLAYER)
    //    {
    //        // Check if allowed to receive it in current map
    //        uint8 MapType = sWorld.getConfig(CONFIG_PVP_TOKEN_MAP_TYPE);
    //        if ((MapType == 1 && !InBattleGroundOrArena() && !IsFFAPvP())
    //            || (MapType == 2 && !IsFFAPvP())
    //            || (MapType == 3) && !InBattleGroundOrArena()) // For MapType 5 Arena check already was up there, where arena breaks the function
    //            return true;
    //        
    //        if (uint32 itemId = sWorld.getConfig(CONFIG_PVP_TOKEN_ID))
    //        {
    //            /*if (MapType == 5 && !InBattleGroundOrArena())
    //                itemId = 24245;*/

    //            uint32 noSpaceForCount = 0;
    //            int32 count = sWorld.getConfig(CONFIG_PVP_TOKEN_COUNT);

    //            // check space and find places
    //            ItemPosCountVec dest;
    //            uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
    //            if (msg != EQUIP_ERR_OK)   // convert to possible store amount
    //                count -= noSpaceForCount;

    //            if (count == 0 || dest.empty()) // can't add any
    //            {
    //                ChatHandler(this).SendSysMessage("You have no space in your bags for PvP token.");
    //                return true;
    //            }

    //            Item* item = StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
    //            SendNewItem(item,count,true,false);
    //        }
    //    }
    //}

    return true;
}

void Player::ModifyHonorPoints(int32 value)
{
    if (value < 0)
    {
        if (GetHonorPoints() > sWorld.getConfig(CONFIG_MAX_HONOR_POINTS))
            SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, sWorld.getConfig(CONFIG_MAX_HONOR_POINTS) + value);
        else
            SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, GetHonorPoints() > uint32(-value) ? GetHonorPoints() + value : 0);
    }
    else
    {
        if (value == 0)
            value = 1;
        SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, GetHonorPoints() < sWorld.getConfig(CONFIG_MAX_HONOR_POINTS) - value ? GetHonorPoints() + value : sWorld.getConfig(CONFIG_MAX_HONOR_POINTS));
    }

	sLog.outLog(LOG_CHARACTER_ACTIONS, "#money Player %s (guid %u) modified honor points amount %d", GetName(), GetGUIDLow(), value);
}

void Player::ModifyArenaPoints(int32 value)
{
    if (value < 0)
    {
        if (GetArenaPoints() > sWorld.getConfig(CONFIG_MAX_ARENA_POINTS))
            SetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, sWorld.getConfig(CONFIG_MAX_ARENA_POINTS) + value);
        else
            SetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, GetArenaPoints() > uint32(-value) ? GetArenaPoints() + value : 0);
    }
    else
        SetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, GetArenaPoints() < sWorld.getConfig(CONFIG_MAX_ARENA_POINTS) - value ? GetArenaPoints() + value : sWorld.getConfig(CONFIG_MAX_ARENA_POINTS));

	sLog.outLog(LOG_CHARACTER_ACTIONS, "#money Player %s (guid %u) modified arena points amount %d", GetName(), GetGUIDLow(), value);
}

uint32 Player::GetGuildIdFromDB(uint64 guid)
{
    std::ostringstream ss;
    ss<<"SELECT guildid FROM guild_member WHERE guid='"<<guid<<"'";
    QueryResultAutoPtr result = RealmDataDatabase.Query(ss.str().c_str());
    if (result)
    {
        uint32 v = result->Fetch()[0].GetUInt32();
        return v;
    }
    else
        return 0;
}

uint32 Player::GetRankFromDB(uint64 guid)
{
    std::ostringstream ss;
    ss<<"SELECT rank FROM guild_member WHERE guid='"<<guid<<"'";
    QueryResultAutoPtr result = RealmDataDatabase.Query(ss.str().c_str());
    if (result)
    {
        uint32 v = result->Fetch()[0].GetUInt32();
        return v;
    }
    else
        return 0;
}

void Player::SetArenaTeamInfoField(uint8 slot, uint32 type, uint32 value)    
{
    //if(ArenaTeam::GetSlotByType(ARENA_TEAM_1v1) == slot)
    //    _custom1v1ArenaTeamInfo[type] = value;
    //else
    SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (slot * 6) + type, value);
}

uint32 Player::GetArenaTeamIdFromDB(uint64 guid, uint8 type)
{
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT arena_team_member.arenateamid FROM arena_team_member JOIN arena_team ON arena_team_member.arenateamid = arena_team.arenateamid WHERE guid='%u' AND type='%u' LIMIT 1", GUID_LOPART(guid), type);
    if (!result)
        return 0;

    uint32 id = (*result)[0].GetUInt32();
    return id;
}

uint32 Player::GetZoneIdFromDB(uint64 guid)
{
    std::ostringstream ss;

    ss<<"SELECT zone FROM characters WHERE guid='"<<GUID_LOPART(guid)<<"'";
    QueryResultAutoPtr result = RealmDataDatabase.Query(ss.str().c_str());
    if (!result)
        return 0;
    Field* fields = result->Fetch();
    uint32 zone = fields[0].GetUInt32();

    if (!zone)
    {
        // stored zone is zero, use generic and slow zone detection
        ss.str("");
        ss<<"SELECT map,position_x,position_y,position_z FROM characters WHERE guid='"<<GUID_LOPART(guid)<<"'";
        result = RealmDataDatabase.Query(ss.str().c_str());
        if (!result)
            return 0;
        fields = result->Fetch();
        uint32 map  = fields[0].GetUInt32();
        float posx = fields[1].GetFloat();
        float posy = fields[2].GetFloat();

        float posz = fields[3].GetFloat();
        zone = sTerrainMgr.GetZoneId(map,posx,posy,posz);

        ss.str("");
        ss << "UPDATE characters SET zone='"<<zone<<"' WHERE guid='"<<GUID_LOPART(guid)<<"'";
        RealmDataDatabase.Execute(ss.str().c_str());
    }

    return zone;
}

uint32 Player::GetLevelFromDB(uint64 guid)

{
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT level FROM characters WHERE guid='%u'", GUID_LOPART(guid));
    if (!result)
        return 0;

    Field* fields = result->Fetch();
    uint32 level = fields[0].GetUInt32();
    return level;
}

void Player::UpdateArea(uint32 newArea)
{
    // FFA_PVP flags are area and not zone id dependent
    // so apply them accordingly
    m_areaUpdateId    = newArea;

    AreaTableEntry const* area = GetAreaEntryByAreaID(newArea);

    if (area && (area->flags & AREA_FLAG_ARENA))
    {
        if (!isGameMaster() && !IsSpectator())
        {
            UpdatePvP(true,true);
            if (!sWorld.IsFFAPvPRealm())
                SetFFAPvP(true);
        }
    }
    else
    {
        if (sWorld.isPvPArea(m_areaUpdateId) || sWorld.isPvPArea(newArea))
        {
            AreaTableEntry const* zone = GetAreaEntryByAreaID(GetZoneId());
            if (!zone)
            {
                sLog.outLog(LOG_DEFAULT, "No zone in UpdateArea on non-pvp area update");
                return;
            }
            pvpInfo.inHostileArea =
            (GetMap()->IsDungeon() || GetTeam() == ALLIANCE && zone->team == AREATEAM_HORDE ||
            GetTeam() == HORDE    && zone->team == AREATEAM_ALLY  ||
            sWorld.IsPvPRealm()   && zone->team == AREATEAM_NONE && 
            (!sWorld.IsFFAPvPRealm() || !(zone->flags & AREA_FLAG_SANCTUARY)) ||
            InBattleGroundOrArena()) && !sWorld.isPvPArea(newArea);         // overwrite for battlegrounds, maybe batter some zone flags but current known not 100% fit to this
            
            if (sWorld.isPvPArea(newArea))    // in duel zone
            {
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP);
                UpdatePvP(false);
				SetFFAPvP(false);
            }
            if (pvpInfo.inHostileArea)                               // in hostile area
            {
                if (!IsPvP() || pvpInfo.endTimer != 0)
                    UpdatePvP(true, true);
            }
            else if (!(zone->flags & AREA_FLAG_SANCTUARY))             // in friendly area
            {
                if (IsPvP() && !HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_IN_PVP) && pvpInfo.endTimer == 0)
                    pvpInfo.endTimer = time(0);                     // start toggle-off
            }
        }
        // remove ffa flag only if not ffapvp realm
        // removal in sanctuaries and capitals is handled in zone update
        else if (IsFFAPvP())
        {
            UpdatePvP(true, pvpInfo.inHostileArea);
            if (!sWorld.IsFFAPvPRealm())
                SetFFAPvP(false);
        }
    }

    UpdateAreaDependentAuras(newArea);
}

class LocalChannelUpdate : public WorldEvent
{
    public:
        LocalChannelUpdate(Player* player, uint32 zone) : WorldEvent(player), _zone(zone) {}

        bool Execute()/* override*/
        {
            if (!_owner->IsInWorld())
                return false;

            _owner->UpdateLocalChannels(_zone);
            return true;
        }

    private:
        uint32 _zone;
};

void Player::UpdateZone(uint32 newZone)
{
    uint32 oldZoneId  = m_zoneUpdateId;
    m_zoneUpdateId    = newZone;
    m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL;

    // zone changed, so area changed as well, update it
    uint32 newArea = GetAreaId();
    UpdateArea(newArea);

    // ressurect dead players when entering Shattrath, because there's no corpse
    if (newZone == 3703 && isDead())
    {
        ResurrectPlayer(0.5f);
    }

    AreaTableEntry const* zone = GetAreaEntryByAreaID(newZone);
    if (!zone)
        return;

    // inform outdoor pvp
    if (oldZoneId != m_zoneUpdateId)
    {
        sOutdoorPvPMgr.HandlePlayerLeaveZone(this, oldZoneId);
        sOutdoorPvPMgr.HandlePlayerEnterZone(this, m_zoneUpdateId);
    }

    if (sWorld.getConfig(CONFIG_WEATHER))
    {
        Weather *wth = sWorld.FindWeather(zone->ID);
        if (wth)
        {
            wth->SendWeatherUpdateToPlayer(this);
        }
        else
        {
            if (!sWorld.AddWeather(zone->ID))
            {
                // send fine weather packet to remove old zone's weather
                Weather::SendFineWeatherUpdateToPlayer(this);
            }
        }
    }

    pvpInfo.inHostileArea =
        (GetMap()->IsDungeon() || GetTeam() == ALLIANCE && zone->team == AREATEAM_HORDE ||
        GetTeam() == HORDE    && zone->team == AREATEAM_ALLY  ||
        sWorld.IsPvPRealm()   && zone->team == AREATEAM_NONE && 
        (!sWorld.IsFFAPvPRealm() || (GetLevel() >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL) && !(zone->flags & AREA_FLAG_SANCTUARY))) ||
        InBattleGroundOrArena()) && !sWorld.isPvPArea(GetCachedArea());                                   // overwrite for battlegrounds, maybe batter some zone flags but current known not 100% fit to this

    if (pvpInfo.inHostileArea)                               // in hostile area
    {
        if (!IsSpectator() && (!IsPvP() || pvpInfo.endTimer != 0))
            UpdatePvP(true, true);
    }
    else if (!(zone->flags & AREA_FLAG_SANCTUARY))             // in friendly area
    {
        if (IsPvP() && !HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_IN_PVP) && pvpInfo.endTimer == 0)
            pvpInfo.endTimer = time(0);                     // start toggle-off
    }

    if (zone->flags & AREA_FLAG_SANCTUARY)                   // in sanctuary
    {
        RemoveFlag(PLAYER_FLAGS,PLAYER_FLAGS_IN_PVP);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY);
        UpdatePvP(false);
    }
    else
    {
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY);
    }

    if (zone->flags & AREA_FLAG_CAPITAL)                     // in capital city
    {
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
        SetRestType(REST_TYPE_IN_CITY);
        InnEnter(time(NULL),GetMapId(),0,0,0);
    }
    else                                                    // anywhere else
    {
        if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))     // but resting (walk from city or maybe in tavern or leave tavern recently)
        {
            if (GetRestType()==REST_TYPE_IN_CITY) // was resting in city -> but left it
            {
                RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);
                SetRestType(REST_TYPE_NO);
            }
        }
    }

    // remove items with area/map limitations (delete only for alive player to allow back in ghost mode)
    // if player resurrected at teleport this will be applied in resurrect code
    if (isAlive())
        DestroyZoneLimitedItem(true, newZone);

    // recent client version not send leave/join channel packets for built-in local channels
    sWorldEventProcessor.ScheduleEvent(this, new LocalChannelUpdate(this, newZone));

    // group update
    if (GetGroup())
        SetGroupUpdateFlag(GROUP_UPDATE_FLAG_ZONE);

    UpdateZoneDependentAuras(newZone);
}

//If players are too far way of duel flag... then player loose the duel
void Player::CheckDuelDistance(time_t currTime)
{
    if (!duel)
        return;

    uint64 duelFlagGUID = GetUInt64Value(PLAYER_DUEL_ARBITER);
    GameObject* obj = GetMap()->GetGameObject(duelFlagGUID);
    if (!obj)
        return;

    if (duel->outOfBound == 0)
    {
        if (!IsWithinDistInMap(obj, 75))
        {
            duel->outOfBound = currTime;

            WorldPacket data(SMSG_DUEL_OUTOFBOUNDS, 0);
            SendPacketToSelf(&data);
        }
    }
    else
    {
        if (IsWithinDistInMap(obj, 75))
        {
            duel->outOfBound = 0;

            WorldPacket data(SMSG_DUEL_INBOUNDS, 0);
            SendPacketToSelf(&data);
        }
        else if (currTime >= (duel->outOfBound+10))
        {
            CombatStopWithPets(true);
            if (duel->opponent)
                duel->opponent->CombatStopWithPets(true);
            DuelComplete(DUEL_FLED);
        }
    }
}

bool Player::IsOutdoorPvPActive()
{
    return (isAlive() && !HasInvisibilityAura() && !HasStealthAura() && (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP) || sWorld.IsPvPRealm() && (!sWorld.isEasyRealm() || GetLevel() > sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL)-11))  && !HasUnitMovementFlag(MOVEFLAG_FLYING) && !IsTaxiFlying());
}

void Player::DuelComplete(DuelCompleteType type)
{
    // duel not requested
    if (!duel)
        return;

    WorldPacket data(SMSG_DUEL_COMPLETE, (1));
    data << (uint8)((type != DUEL_INTERUPTED) ? 1 : 0);
    SendPacketToSelf(&data);
    duel->opponent->SendPacketToSelf(&data);

    if (type != DUEL_INTERUPTED)
    {
        data.Initialize(SMSG_DUEL_WINNER, (1+20));          // we guess size
        data << (uint8)((type==DUEL_WON) ? 0 : 1);          // 0 = just won; 1 = fled
        data << duel->opponent->GetName();
        data << GetName();
        BroadcastPacket(&data,true);
        if (type == DUEL_WON)
            HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
    }

    // cool-down duel spell
    /*data.Initialize(SMSG_SPELL_COOLDOWN, 17);

    data<<GetGUID();
    data<<uint8(0x0);

    data<<(uint32)7266;
    data<<uint32(0x0);
    SendPacketToSelf(&data);
    data.Initialize(SMSG_SPELL_COOLDOWN, 17);
    data<<duel->opponent->GetGUID();
    data<<uint8(0x0);
    data<<(uint32)7266;
    data<<uint32(0x0);
    duel->opponent->SendPacketToSelf(&data);*/

    //Remove Duel Flag object
    GameObject* obj = GetMap()->GetGameObject(GetUInt64Value(PLAYER_DUEL_ARBITER));
    if (obj)
        duel->initiator->RemoveGameObject(obj,true);

    /* remove auras */
    std::vector<uint32> auras2remove;
    AuraMap const& vAuras = duel->opponent->GetAuras();
    for (AuraMap::const_iterator i = vAuras.begin(); i != vAuras.end(); ++i)
    {
        if (!i->second->IsPositive() && i->second->GetCasterGUID() == GetGUID() && i->second->GetAuraApplyTime() >= duel->startTime)
            auras2remove.push_back(i->second->GetId());
    }

    for (size_t i=0; i<auras2remove.size(); i++)
        duel->opponent->RemoveAurasDueToSpell(auras2remove[i]);

    auras2remove.clear();
    AuraMap const& auras = GetAuras();
    for (AuraMap::const_iterator i = auras.begin(); i != auras.end(); ++i)
    {
        if (!i->second->IsPositive() && i->second->GetCasterGUID() == duel->opponent->GetGUID() && i->second->GetAuraApplyTime() >= duel->startTime)
            auras2remove.push_back(i->second->GetId());
    }
    for (size_t i=0; i<auras2remove.size(); i++)
        RemoveAurasDueToSpell(auras2remove[i]);

    // cleanup combo points
    if (GetComboTarget()==duel->opponent->GetGUID())
        ClearComboPoints();
    else if (GetComboTarget()==duel->opponent->GetPetGUID())
        ClearComboPoints();

    if (duel->opponent->GetComboTarget()==GetGUID())
        duel->opponent->ClearComboPoints();
    else if (duel->opponent->GetComboTarget()==GetPetGUID())
        duel->opponent->ClearComboPoints();

    // Honor points after duel (the winner) - ImpConfig
    if (uint32 amount = sWorld.getConfig(CONFIG_HONOR_AFTER_DUEL))
        duel->opponent->RewardHonor(NULL,1,amount);

    //cleanups
    SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);
    duel->opponent->SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    duel->opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    delete duel->opponent->duel;
    duel->opponent->duel = NULL;
    delete duel;
    duel = NULL;
}

//---------------------------------------------------------//

void Player::_ApplyItemMods(Item *item, uint8 slot,bool apply)
{
    if (slot >= INVENTORY_SLOT_BAG_END || !item)
        return;

    ItemPrototype const *proto = item->GetProto();

    if (!proto)
        return;

    //// race shirt
    //if (proto->Spells[0].SpellId == 55401)
    //{
    //    if (apply && proto->Spells[0].SpellPPMRate > 0)
    //    {
    //        SetDisplayId(proto->Spells[0].SpellPPMRate);
    //    }
    //    else
    //    {
    //        SetDisplayId(GetNativeDisplayId());
    //    }
    //}

    // not apply/remove mods for broken item (_ApplyItemMods is used before setting durability to 0 when item
    // loss durability so there is no need to check for 'apply' (prevent bug abuse by stats stacking))
    if (item->IsBroken())
    {
        if (proto->Socket[0].Color)
            CorrectMetaGemEnchants(slot, apply);
        return;
    }

    sLog.outDetail("applying mods for item %u ",item->GetGUIDLow());

    uint32 attacktype = Player::GetAttackBySlot(slot);
    if (attacktype < MAX_ATTACK)
        _ApplyWeaponDependentAuraMods(item,WeaponAttackType(attacktype),apply);

    _ApplyItemBonuses(proto,slot,apply);

    if (slot==EQUIPMENT_SLOT_RANGED)
        _ApplyAmmoBonuses();

    ApplyItemEquipSpell(item,apply);
    ApplyEnchantment(item, apply);

    if (proto->Socket[0].Color)                              //only (un)equipping of items with sockets can influence metagems, so no need to waste time with normal items
        CorrectMetaGemEnchants(slot, apply);

    sLog.outDebug("_ApplyItemMods complete.");
}

void Player::_ApplyItemBonuses(ItemPrototype const *proto,uint8 slot,bool apply)
{
    if (slot >= INVENTORY_SLOT_BAG_END || !proto)
        return;

    for (int i = 0; i < MAX_ITEM_PROTO_STATS; i++)
    {
        float val = float (proto->ItemStat[i].ItemStatValue);

        if (val==0)
            continue;

        switch (proto->ItemStat[i].ItemStatType)
        {
            case ITEM_MOD_MANA:
                HandleStatModifier(UNIT_MOD_MANA, BASE_VALUE, float(val), apply);
                break;
            case ITEM_MOD_HEALTH:                           // modify HP
                HandleStatModifier(UNIT_MOD_HEALTH, BASE_VALUE, float(val), apply);
                break;
            case ITEM_MOD_AGILITY:                          // modify agility
                HandleStatModifier(UNIT_MOD_STAT_AGILITY, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_AGILITY, float(val), apply);
                break;
            case ITEM_MOD_STRENGTH:                         //modify strength
                HandleStatModifier(UNIT_MOD_STAT_STRENGTH, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_STRENGTH, float(val), apply);
                break;
            case ITEM_MOD_INTELLECT:                        //modify intellect
                HandleStatModifier(UNIT_MOD_STAT_INTELLECT, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_INTELLECT, float(val), apply);
                break;
            case ITEM_MOD_SPIRIT:                           //modify spirit
                HandleStatModifier(UNIT_MOD_STAT_SPIRIT, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_SPIRIT, float(val), apply);
                break;
            case ITEM_MOD_STAMINA:                          //modify stamina
                HandleStatModifier(UNIT_MOD_STAT_STAMINA, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_STAMINA, float(val), apply);
                break;
            case ITEM_MOD_DEFENSE_SKILL_RATING:
                ApplyRatingMod(CR_DEFENSE_SKILL, int32(val), apply);
                break;
            case ITEM_MOD_DODGE_RATING:
                ApplyRatingMod(CR_DODGE, int32(val), apply);
                break;
            case ITEM_MOD_PARRY_RATING:
                ApplyRatingMod(CR_PARRY, int32(val), apply);
                break;
            case ITEM_MOD_BLOCK_RATING:
                ApplyRatingMod(CR_BLOCK, int32(val), apply);
                break;
            case ITEM_MOD_HIT_MELEE_RATING:
                ApplyRatingMod(CR_HIT_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_HIT_RANGED_RATING:
                ApplyRatingMod(CR_HIT_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_HIT_SPELL_RATING:
                ApplyRatingMod(CR_HIT_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_MELEE_RATING:
                ApplyRatingMod(CR_CRIT_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_RANGED_RATING:
                ApplyRatingMod(CR_CRIT_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_SPELL_RATING:
                ApplyRatingMod(CR_CRIT_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HIT_TAKEN_MELEE_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_HIT_TAKEN_RANGED_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_HIT_TAKEN_SPELL_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_MELEE_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_RANGED_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_SPELL_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_MELEE_RATING:
                ApplyRatingMod(CR_HASTE_MELEE, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_RANGED_RATING:
                ApplyRatingMod(CR_HASTE_RANGED, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_SPELL_RATING:
                ApplyRatingMod(CR_HASTE_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HIT_RATING:
                ApplyRatingMod(CR_HIT_MELEE, int32(val), apply);
                ApplyRatingMod(CR_HIT_RANGED, int32(val), apply);
                //ApplyRatingMod(CR_HIT_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_RATING:
                ApplyRatingMod(CR_CRIT_MELEE, int32(val), apply);
                ApplyRatingMod(CR_CRIT_RANGED, int32(val), apply);
                //ApplyRatingMod(CR_CRIT_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HIT_TAKEN_RATING:
                ApplyRatingMod(CR_HIT_TAKEN_MELEE, int32(val), apply);
                ApplyRatingMod(CR_HIT_TAKEN_RANGED, int32(val), apply);
                ApplyRatingMod(CR_HIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_CRIT_TAKEN_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_MELEE, int32(val), apply);
                ApplyRatingMod(CR_CRIT_TAKEN_RANGED, int32(val), apply);
                ApplyRatingMod(CR_CRIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_RESILIENCE_RATING:
                ApplyRatingMod(CR_CRIT_TAKEN_MELEE, int32(val), apply);
                ApplyRatingMod(CR_CRIT_TAKEN_RANGED, int32(val), apply);
                ApplyRatingMod(CR_CRIT_TAKEN_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_HASTE_RATING:
                ApplyRatingMod(CR_HASTE_MELEE, int32(val), apply);
                ApplyRatingMod(CR_HASTE_RANGED, int32(val), apply);
                //ApplyRatingMod(CR_HASTE_SPELL, int32(val), apply);
                break;
            case ITEM_MOD_EXPERTISE_RATING:
                ApplyRatingMod(CR_EXPERTISE, int32(val), apply);
                break;
            case ITEM_MOD_SPELL_PENETRATION:
                ApplyModInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE, int32(-val), apply);
                m_spellPenetrationItemMod += apply ? int32(val) : int32(-val);
                break;
        }
    }

    if (proto->Armor)
        HandleStatModifier(UNIT_MOD_ARMOR, BASE_VALUE, float(proto->Armor), apply);

    if (proto->Block)
        HandleBaseModValue(SHIELD_BLOCK_VALUE, FLAT_MOD, float(proto->Block), apply);

    if (proto->HolyRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_HOLY, BASE_VALUE, float(proto->HolyRes), apply);

    if (proto->FireRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_FIRE, BASE_VALUE, float(proto->FireRes), apply);

    if (proto->NatureRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(proto->NatureRes), apply);

    if (proto->FrostRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_FROST, BASE_VALUE, float(proto->FrostRes), apply);

    if (proto->ShadowRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(proto->ShadowRes), apply);

    if (proto->ArcaneRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(proto->ArcaneRes), apply);

    WeaponAttackType attType = BASE_ATTACK;
    float damage = 0.0f;

    if (slot == EQUIPMENT_SLOT_RANGED && (
        proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_THROWN ||
        proto->InventoryType == INVTYPE_RANGEDRIGHT))
    {
        attType = RANGED_ATTACK;
    }
    else if (slot==EQUIPMENT_SLOT_OFFHAND)
    {
        attType = OFF_ATTACK;
    }

    if (proto->Damage[0].DamageMin > 0)
    {
        damage = apply ? proto->Damage[0].DamageMin : BASE_MINDAMAGE;
        SetBaseWeaponDamage(attType, MINDAMAGE, damage);
        //sLog.outLog(LOG_DEFAULT, "ERROR: applying mindam: assigning %f to weapon mindamage, now is: %f", damage, GetWeaponDamageRange(attType, MINDAMAGE));
    }

    if (proto->Damage[0].DamageMax  > 0)
    {
        damage = apply ? proto->Damage[0].DamageMax : BASE_MAXDAMAGE;
        SetBaseWeaponDamage(attType, MAXDAMAGE, damage);
    }

    if (!IsUseEquipedWeapon(slot==EQUIPMENT_SLOT_MAINHAND))
        return;

    if (proto->Delay)
    {
        if (slot == EQUIPMENT_SLOT_RANGED)
            SetAttackTime(RANGED_ATTACK, apply ? proto->Delay: BASE_ATTACK_TIME);
        else if (slot==EQUIPMENT_SLOT_MAINHAND)
            SetAttackTime(BASE_ATTACK, apply ? proto->Delay: BASE_ATTACK_TIME);
        else if (slot==EQUIPMENT_SLOT_OFFHAND)
            SetAttackTime(OFF_ATTACK, apply ? proto->Delay: BASE_ATTACK_TIME);
    }

    if (CanModifyStats() && (damage || proto->Delay))
        UpdateDamagePhysical(attType);
}

void Player::_ApplyWeaponDependentAuraMods(Item *item,WeaponAttackType attackType,bool apply)
{
    AuraList const& auraCritList = GetAurasByType(SPELL_AURA_MOD_CRIT_PERCENT);
    for (AuraList::const_iterator itr = auraCritList.begin(); itr!=auraCritList.end();++itr)
        _ApplyWeaponDependentAuraCritMod(item,attackType,*itr,apply);

    AuraList const& auraDamageFlatList = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for (AuraList::const_iterator itr = auraDamageFlatList.begin(); itr!=auraDamageFlatList.end();++itr)
        _ApplyWeaponDependentAuraDamageMod(item,attackType,*itr,apply);

    AuraList const& auraDamagePCTList = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for (AuraList::const_iterator itr = auraDamagePCTList.begin(); itr!=auraDamagePCTList.end();++itr)
        _ApplyWeaponDependentAuraDamageMod(item,attackType,*itr,apply);

    AuraList const& auraHasteList = GetAurasByType(SPELL_AURA_MOD_RANGED_AMMO_HASTE);
    for (AuraList::const_iterator itr = auraHasteList.begin(); itr!=auraHasteList.end();++itr)
        _ApplyWeaponDependentAuraRangedHasteMod(item,*itr,apply);
}

void Player::_ApplyWeaponDependentAuraCritMod(Item *item, WeaponAttackType attackType, Aura* aura, bool apply)
{
    // generic not weapon specific case processes in aura code
    if (aura->GetSpellProto()->EquippedItemClass == -1)
        return;

    BaseModGroup mod = BASEMOD_END;
    switch (attackType)
    {
        case BASE_ATTACK:   mod = CRIT_PERCENTAGE;        break;
        case OFF_ATTACK:    mod = OFFHAND_CRIT_PERCENTAGE;break;
        case RANGED_ATTACK: mod = RANGED_CRIT_PERCENTAGE; break;
        default: return;
    }

    if (item->IsFitToSpellRequirements(aura->GetSpellProto()))
    {
        HandleBaseModValue(mod, FLAT_MOD, float (aura->GetModifierValue()), apply);
    }
}

void Player::_ApplyWeaponDependentAuraDamageMod(Item *item, WeaponAttackType attackType, Aura* aura, bool apply)
{
    // ignore spell mods for not wands
    Modifier const* modifier = aura->GetModifier();
    if ((modifier->m_miscvalue & SPELL_SCHOOL_MASK_NORMAL)==0 && (GetClassMask() & CLASSMASK_WAND_USERS)==0)
        return;

    // generic not weapon specific case processes in aura code
    if (aura->GetSpellProto()->EquippedItemClass == -1)
        return;

    UnitMods unitMod = UNIT_MOD_END;
    switch (attackType)
    {
        case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
        case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
        case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
        default: return;
    }

    UnitModifierType unitModType = TOTAL_VALUE;
    switch (modifier->m_auraname)
    {
        case SPELL_AURA_MOD_DAMAGE_DONE:         unitModType = TOTAL_VALUE; break;
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE: unitModType = TOTAL_PCT;   break;
        default: return;
    }

    if (item->IsFitToSpellRequirements(aura->GetSpellProto()))
    {
        HandleStatModifier(unitMod, unitModType, float(aura->GetModifierValue()),apply);
        if (unitModType == TOTAL_VALUE)
        {
            if (aura->IsPositive())
                ApplyModUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS,aura->GetModifierValue(),apply);
            else
                ApplyModInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG,aura->GetModifierValue(),apply);
        }
        else
            ApplyModSignedFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT,aura->GetModifierValue()/100.0f,apply);
    }
}

void Player::_ApplyWeaponDependentAuraRangedHasteMod(Item *item, Aura* aura, bool apply)
{
    // generic not weapon specific case processes in aura code
    if (aura->GetSpellProto()->EquippedItemClass == -1)
        return;

    if (item->IsFitToSpellRequirements(aura->GetSpellProto()))
        ApplyAttackTimePercentMod(RANGED_ATTACK,aura->GetModifierValue(), apply);
}

void Player::ApplyItemEquipSpell(Item *item, bool apply, bool form_change)
{
    if (!item)
        return;

    ItemPrototype const *proto = item->GetProto();
    if (!proto)
        return;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
    {
        _Spell const& spellData = proto->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type
        if (apply && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_EQUIP)
            continue;

        // check if it is valid spell
        SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
        if (!spellproto)
            continue;

        ApplyEquipSpell(spellproto,item,apply,form_change);
    }
}

void Player::ApplyEquipSpell(SpellEntry const* spellInfo, Item* item, bool apply, bool form_change)
{
    if (spellInfo->Category == SPELL_CATEGORY_MORPH_SHIRT && InClassForm())
        return;
    
    if (apply)
    {
        // Cannot be used in this stance/form
        if (SpellMgr::GetErrorAtShapeshiftedCast(spellInfo, m_form) != SPELL_CAST_OK)
            return;

        if (form_change)                                     // check aura active state from other form
        {
            for (int k = 0; k < 3; ++k)
            {
                spellEffectPair spair = spellEffectPair(spellInfo->Id, k);
                for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair); ++iter)
                    if (!item || iter->second->GetCastItemGUID() == item->GetGUID())
                        return;     // and skip re-cast already active aura at form change
            }
        }

        debug_log("WORLD: cast %s Equip spellId - %i", (item ? "item" : "itemset"), spellInfo->Id);

        CastSpell(this, spellInfo, true, item);
    }
    else
    {
        if (form_change)                                     // check aura compatibility
        {
            // Cannot be used in this stance/form
            if (SpellMgr::GetErrorAtShapeshiftedCast(spellInfo, m_form) == SPELL_CAST_OK)
                return;                                     // and remove only not compatible at form change
        }

        if (item)
            RemoveAurasDueToItemSpell(item,spellInfo->Id);  // un-apply all spells , not only at-equipped
        else
            RemoveAurasDueToSpell(spellInfo->Id);           // un-apply spell (item set case)
    }
}

void Player::UpdateEquipSpellsAtFormChange()
{
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (m_items[i] && !m_items[i]->IsBroken())
        {
            ApplyItemEquipSpell(m_items[i],false,true);     // remove spells that not fit to form
            ApplyItemEquipSpell(m_items[i],true,true);      // add spells that fit form but not active
        }
    }

    // item set bonuses not dependent from item broken state
    for (size_t setindex = 0; setindex < ItemSetEff.size(); ++setindex)
    {
        ItemSetEffect* eff = ItemSetEff[setindex];
        if (!eff)
            continue;

        for (uint32 y=0;y<8; ++y)
        {
            SpellEntry const* spellInfo = eff->spells[y];
            if (!spellInfo)
                continue;

            ApplyEquipSpell(spellInfo,NULL,false,true);       // remove spells that not fit to form
            ApplyEquipSpell(spellInfo,NULL,true,true);        // add spells that fit form but not active
        }
    }
}
void Player::CastItemCombatSpell(Unit *target, WeaponAttackType attType, uint32 procVictim, uint32 procEx, SpellEntry const *spellInfo)
{
    if (spellInfo && ((spellInfo->Attributes & SPELL_ATTR_STOP_ATTACK_TARGET) ||
      (spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC || spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE)))
        return;

    if (!target || !target->isAlive() || target == this) //IsInFeralForm(true)
        return;
    
    if (IsInFeralForm(true))
    {
        // @!legendary_weapon - meele proc for druid
        if (GetClass() == CLASS_DRUID && attType == BASE_ATTACK)
        {
            Item* item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            if (item && !item->IsBroken())
            {
                ItemPrototype const* proto = item->GetProto();
                if (proto && proto->ItemId == ITEM_LEGENDARY_STAFF_FERAL)
                {
                    ((Player*)this)->CastItemCombatSpell(target, attType, procVictim, procEx, item, proto, spellInfo);
                    return;
                }
            }
        }

        return;
    }

    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        // If usable, try to cast item spell
        if (Item * item = ((Player*)this)->GetItemByPos(INVENTORY_SLOT_BAG_0,i))
        {
            if (!item->IsBroken())
            {
                if (ItemPrototype const *proto = item->GetProto())
                {
                    // Additional check for weapons
                    if (proto->Class == ITEM_CLASS_WEAPON)
                    {
                        // offhand item cannot proc from main hand hit etc
                        EquipmentSlots slot;
                        switch (attType)
                        {
                            case BASE_ATTACK:   slot = EQUIPMENT_SLOT_MAINHAND; break;
                            case OFF_ATTACK:    slot = EQUIPMENT_SLOT_OFFHAND;  break;
                            case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED;   break;
                            default: continue;
                        }


                        if (slot != i)
                            continue;

                        // Check if item is useable (forms or disarm)
                        if (attType == BASE_ATTACK && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED))
                            continue;

                    }
                    ((Player*)this)->CastItemCombatSpell(target, attType, procVictim, procEx, item, proto, spellInfo);
                }
            }
        }
    }
}

void Player::CastItemCombatSpellFromCast(Item* item, ItemPrototype const* proto, uint32 procFlag)
{
    // @!legendary_weapon - CastItemCombatSpellFromCast
    if (!IsCustomLegendaryWeapon(proto->ItemId,true))
        return;
    
    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
    {
        _Spell const& spellData = proto->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_CHANCE_ON_HIT)
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
        if (!spellInfo)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: unknown Item spellid %i", spellData.SpellId);
            continue;
        }

        SpellProcEventEntry const* spellProcEvent = sSpellMgr.GetSpellProcEvent(spellData.SpellId);
        if (!spellProcEvent)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: unknown spellProcEvent Item spellid %i", spellData.SpellId);
            continue;
        }

        if ((spellProcEvent->procFlags & procFlag) == 0)
            continue;

        // check for proc cooldown (used only for legendary items)
        if (spellData.SpellCooldown > 0 && HasSpellCooldown(spellInfo->Id))
            continue;

        float chance = spellData.SpellPPMRate;

        SendCombatStats(1 << COMBAT_STATS_PROC, "item %u spell %u - proc chance: %.2f (item_spell_from_cast)", this, proto->ItemId, spellInfo->Id, chance);
        if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_ALWAYS_PROC || roll_chance_f(chance))
        {
            CastSpell(this, spellInfo->Id, true, item);
            AddSpellCooldown(spellInfo->Id, time(NULL) + spellData.SpellCooldown / MILLISECONDS);
        }

    }
}

void Player::CastItemCombatSpell(Unit *target, WeaponAttackType attType, uint32 procVictim, uint32 procEx, Item *item, ItemPrototype const * proto, SpellEntry const *spell)
{
    // Can do effect if any damage done to target
    bool custom_leg = false;

    if (procVictim & PROC_FLAG_TAKEN_ANY_DAMAGE || procEx & PROC_EX_ABSORB)
    {
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
        {
            _Spell const& spellData = proto->Spells[i];

            // no spell
            if (!spellData.SpellId)
                continue;

            // wrong triggering type
            if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_CHANCE_ON_HIT)
                continue;

            SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
            if (!spellInfo)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: WORLD: unknown Item spellid %i", spellData.SpellId);
                continue;
            }

            // not allow proc extra attack spell at extra attack
            if (m_extraAttacks && spellInfo->HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS))
                return;

            if (!custom_leg)
                custom_leg = IsCustomLegendaryWeapon(proto->ItemId);

            // check for proc cooldown (used only for legendary items)
            if (custom_leg && spellData.SpellCooldown > 0 && HasSpellCooldown(spellInfo->Id))
                continue;

            //procdebug (melee weapon)
            float chance = spellInfo->procChance;

            if (spellData.SpellPPMRate)
            {
                uint32 WeaponSpeed = GetAttackTime(attType, spellInfo->AttributesEx6 & SPELL_ATTR_EX6_REAL_PPM_SPEED);

                // Gensen: procs were broken by this case @bugged_proc
                //chance = GetPPMProcChance((procVictim & (PROC_FLAG_TAKEN_MELEE_HIT | PROC_FLAG_TAKEN_RANGED_HIT)) ? WeaponSpeed : 2400, spellData.SpellPPMRate);
                chance = GetPPMProcChance(WeaponSpeed, spellData.SpellPPMRate);
            }
            else if (chance > 100.0f)
            {
                chance = GetWeaponProcChance();
            }

            SendCombatStats(1 << COMBAT_STATS_PROC, "item %u spell %u - proc chance: %.2f (item_spell)", target, proto->ItemId, spellInfo->Id, chance);
            if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_ALWAYS_PROC || roll_chance_f(chance))
            {
                CastSpell(target, spellInfo->Id, true, item);

                if (custom_leg)
                    AddSpellCooldown(spellInfo->Id, time(NULL) + spellData.SpellCooldown / MILLISECONDS);
            }
                
        }
    }

    if (custom_leg && IsInFeralForm(true))
        return;

    // item combat enchantments
    for (int e_slot = 0; e_slot < MAX_ENCHANTMENT_SLOT; ++e_slot)
    {
        uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(e_slot));
        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!pEnchant) continue;
        for (int s = 0; s < 3; ++s)
        {
            if (pEnchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                continue;

            SpellEnchantProcEntry const* entry = sSpellMgr.GetSpellEnchantProcEvent(enchant_id);
            if (entry && (entry->procEx || entry->procFlags))
            {
                if (entry->procFlags)
                {
                    if ((procVictim & entry->procFlags) == 0)
                        continue;
                }

                if (entry->procEx)
                {
                    // Check hit/crit/dodge/parry requirement
                    if ((entry->procEx & procEx) == 0)
                        continue;
                }
            }
            else
            {
                // Can do effect if any damage done to target
                if (!(procVictim & PROC_FLAG_TAKEN_ANY_DAMAGE) && !(procEx & PROC_EX_ABSORB))
                    continue;
            }

            uint32 spell_id = pEnchant->spellid[s];

            SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
            if (!spellInfo)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Player::CastItemCombatSpell Enchant %i, cast unknown spell %i", pEnchant->ID, spell_id);
                continue;
            }

            // not allow proc extra attack spell at extra attack
            if (m_extraAttacks && spellInfo->HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS))
                return;

            //procdebug (enchants mongoose)
            float chance = pEnchant->amount[s] != 0 ? float(pEnchant->amount[s]) : GetWeaponProcChance();

            if (entry && entry->PPMChance)
            {
                uint32 WeaponSpeed = GetAttackTime(attType, spellInfo->AttributesEx6 & SPELL_ATTR_EX6_REAL_PPM_SPEED);

                // Gensen: procs were broken by this case @bugged_proc
                //chance = GetPPMProcChance((procVictim & (PROC_FLAG_TAKEN_MELEE_HIT | PROC_FLAG_TAKEN_RANGED_HIT)) ? WeaponSpeed : 2400, entry->PPMChance);
                chance = GetPPMProcChance(WeaponSpeed, entry->PPMChance);
            }
            else if (entry && entry->customChance)
                chance = entry->customChance;

            // Apply spell mods
            ApplySpellMod(spell_id,SPELLMOD_CHANCE_OF_SUCCESS,chance);

            SendCombatStats(1 << COMBAT_STATS_PROC, "item %u spell %u - proc chance: %.2f (item_enchant)", target, proto->ItemId, spellInfo->Id, chance);
            if (sWorld.getConfig(CONFIG_DEBUG_MASK) & DEBUG_MASK_ALWAYS_PROC || roll_chance_f(chance))
            {
                if (SpellMgr::IsPositiveSpell(spell_id))
                    CastSpell(this, spell_id, true, item);
                else
                    CastSpell(target, spell_id, true, item);
            }
        }
    }
}

void Player::_RemoveAllItemMods()
{
    sLog.outDebug("_RemoveAllItemMods start.");

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (m_items[i])
        {
            ItemPrototype const *proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            // item set bonuses not dependent from item broken state
            if (proto->ItemSet)
                RemoveItemsSetItem(this,proto);

            if (i < 10 && i != 1 && i != 3)
                CheckLevel5Reinforcement(true);

            if (m_items[i]->IsBroken())
                continue;

            ApplyItemEquipSpell(m_items[i],false);
            ApplyEnchantment(m_items[i], false);
        }
    }

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (m_items[i])
        {
            if (m_items[i]->IsBroken())
                continue;
            ItemPrototype const *proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            uint32 attacktype = Player::GetAttackBySlot(i);
            if (attacktype < MAX_ATTACK)
                _ApplyWeaponDependentAuraMods(m_items[i],WeaponAttackType(attacktype),false);

            _ApplyItemBonuses(proto,i, false);

            if (i == EQUIPMENT_SLOT_RANGED)
                _ApplyAmmoBonuses();
        }
    }

    sLog.outDebug("_RemoveAllItemMods complete.");
}

void Player::_ApplyAllItemMods()
{
    sLog.outDebug("_ApplyAllItemMods start.");

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (m_items[i])
        {
            if (m_items[i]->IsBroken())
                continue;

            ItemPrototype const *proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            //// race shirt
            //if (i == EQUIPMENT_SLOT_BODY && proto->Spells[0].SpellId == 55401 && proto->Spells[0].SpellPPMRate > 0)
            //    SetDisplayId(proto->Spells[0].SpellPPMRate);

            uint32 attacktype = Player::GetAttackBySlot(i);
            if (attacktype < MAX_ATTACK)
                _ApplyWeaponDependentAuraMods(m_items[i],WeaponAttackType(attacktype),true);

            _ApplyItemBonuses(proto,i, true);

            if (i == EQUIPMENT_SLOT_RANGED)
                _ApplyAmmoBonuses();
        }
    }

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (m_items[i])
        {
            ItemPrototype const *proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            // item set bonuses not dependent from item broken state
            if (proto->ItemSet)
                AddItemsSetItem(this,m_items[i]);

            //if (i < 10 && i != 1 && i != 3)
            //    CheckLevel5Reinforcement();

            if (m_items[i]->IsBroken())
                continue;

            ApplyItemEquipSpell(m_items[i],true);
            ApplyEnchantment(m_items[i], true);
        }
    }

    sLog.outDebug("_ApplyAllItemMods complete.");
}

void Player::_ApplyAmmoBonuses()
{
    // check ammo
    uint32 ammo_id = GetUInt32Value(PLAYER_AMMO_ID);
    if (!ammo_id)
        return;

    float currentAmmoDPS;

    ItemPrototype const *ammo_proto = ObjectMgr::GetItemPrototype(ammo_id);
    if (!ammo_proto || ammo_proto->Class!=ITEM_CLASS_PROJECTILE || !CheckAmmoCompatibility(ammo_proto))
        currentAmmoDPS = 0.0f;
    else
        currentAmmoDPS = ammo_proto->Damage[0].DamageMin;

    if (currentAmmoDPS == GetAmmoDPS())
        return;

    m_ammoDPS = currentAmmoDPS;

    if (CanModifyStats())
        UpdateDamagePhysical(RANGED_ATTACK);
}

bool Player::CheckAmmoCompatibility(const ItemPrototype *ammo_proto) const
{
    if (!ammo_proto)
        return false;

    // check ranged weapon
    Item *weapon = GetWeaponForAttack(RANGED_ATTACK);
    if (!weapon  || weapon->IsBroken())
        return false;

    ItemPrototype const* weapon_proto = weapon->GetProto();
    if (!weapon_proto || weapon_proto->Class!=ITEM_CLASS_WEAPON)
        return false;

    // check ammo ws. weapon compatibility
    switch (weapon_proto->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            if (ammo_proto->SubClass!=ITEM_SUBCLASS_ARROW)
                return false;
            break;
        case ITEM_SUBCLASS_WEAPON_GUN:
            if (ammo_proto->SubClass!=ITEM_SUBCLASS_BULLET)
                return false;
            break;
        default:
            return false;
    }

    return true;
}

/*  If in a battleground a player dies, and an enemy removes the insignia, the player's bones is lootable
    Called by remove insignia spell effect    */
void Player::RemovedInsignia(Player* looterPlr)
{
    if (!GetBattleGroundId())
        return;

    // If not released spirit, do it !
    if (m_deathTimer > 0)
    {
        m_deathTimer = 0;
        BuildPlayerRepop();
        RepopAtGraveyard();
    }

    Corpse *corpse = GetCorpse();
    if (!corpse)
        return;

    // We have to convert player corpse to bones, not to be able to resurrect there
    // SpawnCorpseBones isn't handy, 'cos it saves player while he in BG
    Corpse *bones = sObjectAccessor.ConvertCorpseForPlayer(GetGUID(),true);
    if (!bones)
        return;

    // Now we must make bones lootable, and send player loot
    bones->SetFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);

    // We store the level of our player in the gold field
    // We retrieve this information at Player::SendLoot()
    bones->loot.gold = GetLevel();
    bones->lootRecipient = looterPlr;
    looterPlr->SendLoot(bones->GetGUID(), LOOT_INSIGNIA);
}

/*Loot type MUST be
1-corpse, go
2-skinning
3-Fishing
*/

void Player::SendLootRelease(uint64 guid)
{
    WorldPacket data(SMSG_LOOT_RELEASE_RESPONSE, (8+1));
    data << uint64(guid) << uint8(1);
    SendPacketToSelf(&data);
}

void Player::SendLoot(uint64 guid, LootType loot_type, uint32 cast_item_entry)
{
    // release old loot
    if (uint64 lguid = GetLootGUID())
        m_session->DoLootRelease(lguid);

    Loot    *loot = 0;
    PermissionTypes permission = ALL_PERMISSION;

    sLog.outDebug("Player::SendLoot");
    if (IS_GAMEOBJECT_GUID(guid))
    {
        sLog.outDebug("       IS_GAMEOBJECT_GUID(guid)");
        GameObject* go = GetMap()->GetGameObject(guid);

        // not check distance for GO in case owned GO (fishing bobber case, for example)
        // And permit out of range GO with no owner in case fishing hole
        if (!go || (loot_type != LOOT_FISHINGHOLE && (loot_type != LOOT_FISHING || go->GetOwnerGUID() != GetGUID()) && (go->GetDistance(this) > INTERACTION_DISTANCE)))
        {
            SendLootRelease(guid);
            return;
        }

        loot = &go->loot;

        if (go->getLootState() == GO_READY)
        {
            uint32 lootid =  go->GetLootId();

            //TODO: fix this big hack
            if ((go->GetEntry() == BG_AV_OBJECTID_MINE_N || go->GetEntry() == BG_AV_OBJECTID_MINE_S))
                if (BattleGround *bg = GetBattleGround())
                    if (bg->GetTypeID() == BATTLEGROUND_AV)
                        if (!(((BattleGroundAV*)bg)->PlayerCanDoMineQuest(go->GetEntry(),GetBGTeam())))
                        {
                            SendLootRelease(guid);
                            return;
                        }

            if (lootid)
            {
                sLog.outDebug("       if (lootid)");
                loot->clear();
				
                loot->FillLoot(lootid, LootTemplates_Gameobject, this, false, go->GetEntry());

                //if chest apply 2.1.x rules
                if ((go->GetGoType() == GAMEOBJECT_TYPE_CHEST)&&(go->GetGOInfo()->chest.groupLootRules))
                {
                    if (Group* group = this->GetGroup())
                    {
                        group->UpdateLooterGuid((WorldObject*)go, true);

                        group->PrepareLootRolls(this->GetGUID(), loot, (WorldObject*) go);
                        if (group->GetLootMethod() == MASTER_LOOT)
                            group->SendMasterLoot(loot, (WorldObject*) go);

                    }
                }
            }

            if (loot_type == LOOT_FISHING)
                go->getFishLoot(loot, this);

        }
    }
    else if (IS_ITEM_GUID(guid))
    {
        Item *item = GetItemByGuid(guid);

        if (!item)
        {
            SendLootRelease(guid);
            return;
        }

        if (loot_type == LOOT_DISENCHANTING)
        {
            loot = &item->loot;

            if (!item->m_lootGenerated)
            {
                item->m_lootGenerated = true;
                loot->clear();
                loot->FillLoot(item->GetProto()->DisenchantID, LootTemplates_Disenchant, this,true);
            }
        }
        else if (loot_type == LOOT_PROSPECTING)
        {
            loot = &item->loot;

            if (!item->m_lootGenerated)
            {
                item->m_lootGenerated = true;
                loot->clear();
				loot->FillLoot(item->GetEntry(), LootTemplates_Prospecting, this, true);
            }
        }
        else
        {
            loot = &item->loot;

            //if (!item->m_lootGenerated)
            //  If item doesn't already have loot, attempt to load it. If that
            //  fails then this is first time opening, generate loot
            if (!item->m_lootGenerated && !item->ItemContainerLoadLootFromDB())
            {
                item->SetBinding(true);
                item->m_lootGenerated = true;
                loot->clear();

                if (cast_item_entry)
                    loot->cast_item_entry = cast_item_entry;

                loot->FillLoot(item->GetEntry(), LootTemplates_Item, this,true, item->GetEntry(), cast_item_entry);

                loot->generateMoneyLoot(item->GetProto()->MinMoneyLoot,item->GetProto()->MaxMoneyLoot);

                // Force save the loot and money items that were just rolled
                //  Also saves the container item ID in Loot struct (not to DB)
                if (loot->gold > 0 || loot->unlootedCount > 0)
                    item->ItemContainerSaveLootToDB();
            }
        }
    }
    else if (IS_CORPSE_GUID(guid))                          // remove insignia
    {
        Corpse *bones = ObjectAccessor::GetCorpse(*this, guid);

        if (!bones || !((loot_type == LOOT_CORPSE) || (loot_type == LOOT_INSIGNIA)) || (bones->GetType() != CORPSE_BONES))
        {
            SendLootRelease(guid);
            return;
        }

        loot = &bones->loot;

        if (!bones->lootForBody)
        {
            bones->lootForBody = true;
            uint32 pLevel = bones->loot.gold;
            bones->loot.clear();
            if (BattleGround *bg = GetBattleGround())
				if (bg->GetTypeID() == BATTLEGROUND_AV)
				{
					loot->FillLoot(1, LootTemplates_Creature, this, true);
				}
            // It may need a better formula
            // Now it works like this: lvl10: ~6copper, lvl70: ~9silver
            bones->loot.gold = (uint32)(urand(50, 150) * 0.016f * pow(((float)pLevel)/5.76f, 2.5f) * sWorld.getConfig(RATE_DROP_MONEY));
        }

        if (bones->lootRecipient != this)
            permission = NONE_PERMISSION;
    }
    else
    {
        Creature *creature = GetMap()->GetCreature(guid);

        // must be in range and creature must be alive for pickpocket and must be dead for another loot
        if (!creature || creature->isAlive() != (loot_type == LOOT_PICKPOCKETING) || !creature->IsWithinDistInMap(this,INTERACTION_DISTANCE))
        {
            SendLootRelease(guid);
            return;
        }

        if (loot_type == LOOT_PICKPOCKETING && IsFriendlyTo(creature))
        {
            SendLootRelease(guid);
            return;
        }

		//if (sWorld.getConfig(CONFIG_CAPTCHA_ENABLED))
		//{
		//	if (loot_type == LOOT_CORPSE && NeedCaptcha(CAPTCHA_LOOT_CORPSE))
		//	{
		//		SendLootRelease(guid);
		//		return;
		//	}
		//}

        loot = &creature->loot;

        if (loot_type == LOOT_PICKPOCKETING)
        {
            if (!creature->lootForPickPocketed)
            {
                creature->lootForPickPocketed = true;
                loot->clear();

				if (uint32 lootid = creature->GetCreatureInfo()->pickpocketLootId)
				{
					loot->FillLoot(lootid, LootTemplates_Pickpocketing, this, false);
				}

                // Generate extra money for pick pocket loot
                const uint32 a = urand(0, creature->GetLevel()/2);
                const uint32 b = urand(0, GetLevel()/2);
                loot->gold = uint32(10 * (a + b) * sWorld.getConfig(RATE_DROP_MONEY));
            }
        }
        else
        {
			// the player whose group may loot the corpse
			Player *recipient = creature->GetLootRecipient();
			if (!recipient)
			{
				creature->SetLootRecipient(this);
				recipient = this;
			}
			
			if (!loot->IsPlayerAllowedToLoot(this, NULL))
            {
                sLog.outLog(LOG_WARDEN, "Player::SendLoot Player %s (GUID: %u) is trying to open creature (Entry %u lowGUID %u)"
                    " for looting, but not allowed to (X: %f Y: %f Z: %f Map %u Id %u)", GetName(), GetGUIDLow(), creature->GetEntry(),
                    creature->GetGUIDLow(), GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetInstanciableInstanceId());
                SendLootRelease(guid);
                return;
            }

			// 11.02.23 old
            //if (!creature->lootForBody)
            //{
            //    creature->lootForBody = true;
            //    if (Group* recGroup = recipient->GetGroup())
            //        recGroup->PrepareLootRolls(recipient->GetGUID(), loot, creature);
            //}
			if (!creature->lootForBody)
			{
				creature->lootForBody = true;
				if (Group* group = recipient->GetGroup())
				{
					group->PrepareLootRolls(recipient->GetGUID(), loot, creature);
					if (group->GetLootMethod() == MASTER_LOOT)
						group->SendMasterLoot(loot, creature);
				}
			}

            // possible only if creature->lootForBody && loot->empty() at spell cast check
            if (loot_type == LOOT_SKINNING)
            {
                loot->clear();
                loot->FillLoot(creature->GetCreatureInfo()->SkinLootId, LootTemplates_Skinning, this, false);
            }
            // set group rights only for loot_type != LOOT_SKINNING
            else
            {
                if (Group* group = GetGroup())
                {
                    permission = NONE_PERMISSION;
                    if (group == recipient->GetGroup() && creature->IsPlayerAllowedToLoot(this))
                    {
                        // Trentone loot bug here OR
                        switch(group->GetLootMethod())
                        {
                            case FREE_FOR_ALL:
                                permission = ALL_PERMISSION;
                                break;
                            case MASTER_LOOT:
                                if (group->GetLooterGuid() == GetGUID())
                                {
                                    permission = MASTER_PERMISSION;
                                    group->SendMasterLoot(loot, creature); // always resend master loot - this will ensure that there's only players that can loot displayed there. Also need to make check for hacks
                                }
                                else
                                    permission = GROUP_NONE_PERMISSION;
                                break;
                            case GROUP_LOOT:
                            case NEED_BEFORE_GREED:
                            case ROUND_ROBIN:
                                if (!loot->looterGUID || loot->looterGUID == GetGUID())
                                    permission = GROUP_LOOTER_PERMISSION;   // can take items below threshold
                                else
                                    permission = GROUP_NONE_PERMISSION;     // only see items, cant take
                                break;
                        }
                    }
                }
                else if (recipient == this) // not in group
                    permission = ALL_PERMISSION;
                else if (creature->IsPlayerAllowedToLoot(this))
                    permission = GROUP_NONE_PERMISSION;
                else
                    permission = NONE_PERMISSION;
            }
        }
    }
    SendCombatStats(1 << COMBAT_STATS_LOOTING, "Player::SendLoot looted Guid %llu (%u); loot_type %u; permission %u",
        NULL, guid,GUID_HIPART(guid), loot_type, permission);

    if (permission == NONE_PERMISSION)
    {
        SendLootRelease(guid);
        return;
    }

    SetLootGUID(guid);

    // LOOT_PICKPOCKETING, LOOT_PROSPECTING, LOOT_DISENCHANTING and LOOT_INSIGNIA unsupported by client, sending LOOT_SKINNING instead
    if (loot_type == LOOT_PICKPOCKETING || loot_type == LOOT_DISENCHANTING || loot_type == LOOT_PROSPECTING || loot_type == LOOT_INSIGNIA)
        loot_type = LOOT_SKINNING;

    if (loot_type == LOOT_FISHINGHOLE)
        loot_type = LOOT_FISHING;

    WorldPacket data(SMSG_LOOT_RESPONSE, (9+50));           // we guess size

    data << uint64(guid);
    data << uint8(loot_type);
    data << LootView(*loot, this, permission);

    SendPacketToSelf(&data);

    // add 'this' player as one of the players that are looting 'loot'
    if (permission != NONE_PERMISSION)
        loot->AddLooter(GetGUID());

    if (loot_type == LOOT_CORPSE && !IS_ITEM_GUID(guid))
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
}

void Player::SendNotifyLootMoneyRemoved()
{
    WorldPacket data(SMSG_LOOT_CLEAR_MONEY, 0);
    SendPacketToSelf(&data);
}

void Player::SendNotifyLootItemRemoved(uint8 lootSlot)
{
    WorldPacket data(SMSG_LOOT_REMOVED, 1);
    data << uint8(lootSlot);
    SendPacketToSelf(&data);
}

void Player::SendUpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
    data << Field;
    data << Value;
    SendPacketToSelf(&data);
}

void Player::SendInitWorldStates(bool forceZone, uint32 forceZoneId)
{
    // data depends on zoneid/mapid...
    BattleGround* bg = GetBattleGround();
    uint16 NumberOfFields = 0;
    uint32 mapid = GetMapId();
    uint32 zoneid;
    if (forceZone)
        zoneid = forceZoneId;
    else
        zoneid = GetZoneId();
    OutdoorPvP * pvp = sOutdoorPvPMgr.GetOutdoorPvPToZoneId(zoneid);
    uint32 areaid = GetCachedArea();
    sLog.outDebug("Sending SMSG_INIT_WORLD_STATES to Map:%u, Zone: %u", mapid, zoneid);
    // may be exist better way to do this...
    switch (zoneid)
    {
        case 0:
        case 1:
        case 4:
        case 8:
        case 10:
        case 11:
        case 12:
        case 36:
        case 38:
        case 40:
        case 41:
        case 51:
        case 267:
        case 1519:
        case 1537:
        case 2257:
        case 2918:
            NumberOfFields = 6;
            break;
        case 139:
            NumberOfFields = 39;
            break;
        case 1377:
            NumberOfFields = 13;
            break;
        case 2597:
            NumberOfFields = 81;
            break;
        case 3277:
            NumberOfFields = 14;
            break;
        case 3358:
        case 3820:
            NumberOfFields = 38;
            break;
        case 3483:
            NumberOfFields = 25;
            break;
        case 3518:
            NumberOfFields = 37;
            break;
        case 3519:
            NumberOfFields = 36;
            break;
        case 3521:
            NumberOfFields = 35;
            break;
        case 3522:
            NumberOfFields = 8;
            break;
        case 3698:
        case 3702:
        case 3968:
            NumberOfFields = 9;
            break;
        case 3703:
            NumberOfFields = 9;
            break;
        default:
            NumberOfFields = 10;
            break;
    }

    WorldPacket data(SMSG_INIT_WORLD_STATES, (4+4+4+2+(NumberOfFields*8)));
    data << uint32(mapid);                                  // mapid
    data << uint32(zoneid);                                 // zone id
    data << uint32(areaid);                                 // area id, new 2.1.0
    data << uint16(NumberOfFields);                         // count of uint64 blocks
    data << uint32(0x8d8) << uint32(0x0);                   // 1
    data << uint32(0x8d7) << uint32(0x0);                   // 2
    data << uint32(0x8d6) << uint32(0x0);                   // 3
    data << uint32(0x8d5) << uint32(0x0);                   // 4
    data << uint32(0x8d4) << uint32(0x0);                   // 5
    data << uint32(0x8d3) << uint32(0x0);                   // 6
    if (mapid == 530)                                        // Outland
    {
        data << uint32(0x9bf) << uint32(0x0);               // 7
        data << uint32(0x9bd) << uint32(0xF);               // 8
        data << uint32(0x9bb) << uint32(0xF);               // 9
    }
    switch (zoneid)
    {
        case 1:
        case 11:
        case 12:
        case 38:
        case 40:
        case 51:
        case 1519:
        case 1537:
        case 2257:
            break;
        case 3522:
            data << uint32(-4581) << uint32(0x0);
            break;
        case 139: // EPL
            {
                if (pvp && pvp->GetTypeId() == OUTDOOR_PVP_EP)
                    pvp->FillInitialWorldStates(data);
                else
                {
                    data << uint32(0x97a) << uint32(0x0); // 10 2426
                    data << uint32(0x917) << uint32(0x0); // 11 2327
                    data << uint32(0x918) << uint32(0x0); // 12 2328
                    data << uint32(0x97b) << uint32(0x32); // 13 2427
                    data << uint32(0x97c) << uint32(0x32); // 14 2428
                    data << uint32(0x933) << uint32(0x1); // 15 2355
                    data << uint32(0x946) << uint32(0x0); // 16 2374
                    data << uint32(0x947) << uint32(0x0); // 17 2375
                    data << uint32(0x948) << uint32(0x0); // 18 2376
                    data << uint32(0x949) << uint32(0x0); // 19 2377
                    data << uint32(0x94a) << uint32(0x0); // 20 2378
                    data << uint32(0x94b) << uint32(0x0); // 21 2379
                    data << uint32(0x932) << uint32(0x0); // 22 2354
                    data << uint32(0x934) << uint32(0x0); // 23 2356
                    data << uint32(0x935) << uint32(0x0); // 24 2357
                    data << uint32(0x936) << uint32(0x0); // 25 2358
                    data << uint32(0x937) << uint32(0x0); // 26 2359
                    data << uint32(0x938) << uint32(0x0); // 27 2360
                    data << uint32(0x939) << uint32(0x1); // 28 2361
                    data << uint32(0x930) << uint32(0x1); // 29 2352
                    data << uint32(0x93a) << uint32(0x0); // 30 2362
                    data << uint32(0x93b) << uint32(0x0); // 31 2363
                    data << uint32(0x93c) << uint32(0x0); // 32 2364
                    data << uint32(0x93d) << uint32(0x0); // 33 2365
                    data << uint32(0x944) << uint32(0x0); // 34 2372
                    data << uint32(0x945) << uint32(0x0); // 35 2373
                    data << uint32(0x931) << uint32(0x1); // 36 2353
                    data << uint32(0x93e) << uint32(0x0); // 37 2366
                    data << uint32(0x931) << uint32(0x1); // 38 2367 ??  grey horde not in dbc! send for consistency's sake, and to match field count
                    data << uint32(0x940) << uint32(0x0); // 39 2368
                    data << uint32(0x941) << uint32(0x0); // 7 2369
                    data << uint32(0x942) << uint32(0x0); // 8 2370
                    data << uint32(0x943) << uint32(0x0); // 9 2371
                }
            }
            break;
        case 1377: // Silithus
            {
                if (pvp && pvp->GetTypeId() == OUTDOOR_PVP_SI)
                    pvp->FillInitialWorldStates(data);
                else
                {
                    // states are always shown
                    data << uint32(2313) << uint32(0x0); // 7 ally silityst gathered
                    data << uint32(2314) << uint32(0x0); // 8 horde silityst gathered
                    data << uint32(2317) << uint32(0x0); // 9 max silithyst
                }
                // dunno about these... aq opening event maybe?
                data << uint32(2322) << uint32(0x0); // 10 sandworm N
                data << uint32(2323) << uint32(0x0); // 11 sandworm S
                data << uint32(2324) << uint32(0x0); // 12 sandworm SW
                data << uint32(2325) << uint32(0x0); // 13 sandworm E
            }
            break;
        case 2597:                                          // AV
            if (bg && bg->GetTypeID() == BATTLEGROUND_AV)
                bg->FillInitialWorldStates(data);
            else
            {
                data << uint32(0x7ae) << uint32(0x1);           // 7 snowfall n
                data << uint32(0x532) << uint32(0x1);           // 8 frostwolfhut hc
                data << uint32(0x531) << uint32(0x0);           // 9 frostwolfhut ac
                data << uint32(0x52e) << uint32(0x0);           // 10 stormpike firstaid a_a
                data << uint32(0x571) << uint32(0x0);           // 11 east frostwolf tower horde assaulted -unused
                data << uint32(0x570) << uint32(0x0);           // 12 west frostwolf tower horde assaulted - unused
                data << uint32(0x567) << uint32(0x1);           // 13 frostwolfe c
                data << uint32(0x566) << uint32(0x1);           // 14 frostwolfw c
                data << uint32(0x550) << uint32(0x1);           // 15 irondeep (N) ally
                data << uint32(0x544) << uint32(0x0);           // 16 ice grave a_a
                data << uint32(0x536) << uint32(0x0);           // 17 stormpike grave h_c
                data << uint32(0x535) << uint32(0x1);           // 18 stormpike grave a_c
                data << uint32(0x518) << uint32(0x0);           // 19 stoneheart grave a_a
                data << uint32(0x517) << uint32(0x0);           // 20 stoneheart grave h_a
                data << uint32(0x574) << uint32(0x0);           // 21 1396 unk
                data << uint32(0x573) << uint32(0x0);           // 22 iceblood tower horde assaulted -unused
                data << uint32(0x572) << uint32(0x0);           // 23 towerpoint horde assaulted - unused
                data << uint32(0x56f) << uint32(0x0);           // 24 1391 unk
                data << uint32(0x56e) << uint32(0x0);           // 25 iceblood a
                data << uint32(0x56d) << uint32(0x0);           // 26 towerp a
                data << uint32(0x56c) << uint32(0x0);           // 27 frostwolfe a
                data << uint32(0x56b) << uint32(0x0);           // 28 froswolfw a
                data << uint32(0x56a) << uint32(0x1);           // 29 1386 unk
                data << uint32(0x569) << uint32(0x1);           // 30 iceblood c
                data << uint32(0x568) << uint32(0x1);           // 31 towerp c
                data << uint32(0x565) << uint32(0x0);           // 32 stoneh tower a
                data << uint32(0x564) << uint32(0x0);           // 33 icewing tower a
                data << uint32(0x563) << uint32(0x0);           // 34 dunn a
                data << uint32(0x562) << uint32(0x0);           // 35 duns a
                data << uint32(0x561) << uint32(0x0);           // 36 stoneheart bunker alliance assaulted - unused
                data << uint32(0x560) << uint32(0x0);           // 37 icewing bunker alliance assaulted - unused
                data << uint32(0x55f) << uint32(0x0);           // 38 dunbaldar south alliance assaulted - unused
                data << uint32(0x55e) << uint32(0x0);           // 39 dunbaldar north alliance assaulted - unused
                data << uint32(0x55d) << uint32(0x0);           // 40 stone tower d
                data << uint32(0x3c6) << uint32(0x0);           // 41 966 unk
                data << uint32(0x3c4) << uint32(0x0);           // 42 964 unk
                data << uint32(0x3c2) << uint32(0x0);           // 43 962 unk
                data << uint32(0x516) << uint32(0x1);           // 44 stoneheart grave a_c
                data << uint32(0x515) << uint32(0x0);           // 45 stonheart grave h_c
                data << uint32(0x3b6) << uint32(0x0);           // 46 950 unk
                data << uint32(0x55c) << uint32(0x0);           // 47 icewing tower d
                data << uint32(0x55b) << uint32(0x0);           // 48 dunn d
                data << uint32(0x55a) << uint32(0x0);           // 49 duns d
                data << uint32(0x559) << uint32(0x0);           // 50 1369 unk
                data << uint32(0x558) << uint32(0x0);           // 51 iceblood d
                data << uint32(0x557) << uint32(0x0);           // 52 towerp d
                data << uint32(0x556) << uint32(0x0);           // 53 frostwolfe d
                data << uint32(0x555) << uint32(0x0);           // 54 frostwolfw d
                data << uint32(0x554) << uint32(0x1);           // 55 stoneh tower c
                data << uint32(0x553) << uint32(0x1);           // 56 icewing tower c
                data << uint32(0x552) << uint32(0x1);           // 57 dunn c
                data << uint32(0x551) << uint32(0x1);           // 58 duns c
                data << uint32(0x54f) << uint32(0x0);           // 59 irondeep (N) horde
                data << uint32(0x54e) << uint32(0x0);           // 60 irondeep (N) ally
                data << uint32(0x54d) << uint32(0x1);           // 61 mine (S) neutral
                data << uint32(0x54c) << uint32(0x0);           // 62 mine (S) horde
                data << uint32(0x54b) << uint32(0x0);           // 63 mine (S) ally
                data << uint32(0x545) << uint32(0x0);           // 64 iceblood h_a
                data << uint32(0x543) << uint32(0x1);           // 65 iceblod h_c
                data << uint32(0x542) << uint32(0x0);           // 66 iceblood a_c
                data << uint32(0x540) << uint32(0x0);           // 67 snowfall h_a
                data << uint32(0x53f) << uint32(0x0);           // 68 snowfall a_a
                data << uint32(0x53e) << uint32(0x0);           // 69 snowfall h_c
                data << uint32(0x53d) << uint32(0x0);           // 70 snowfall a_c
                data << uint32(0x53c) << uint32(0x0);           // 71 frostwolf g h_a
                data << uint32(0x53b) << uint32(0x0);           // 72 frostwolf g a_a
                data << uint32(0x53a) << uint32(0x1);           // 73 frostwolf g h_c
                data << uint32(0x539) << uint32(0x0);           // 74 frostwolf g a_c
                data << uint32(0x538) << uint32(0x0);           // 75 stormpike grave h_a
                data << uint32(0x537) << uint32(0x0);           // 76 stormpike grave a_a
                data << uint32(0x534) << uint32(0x0);           // 77 frostwolf hut h_a
                data << uint32(0x533) << uint32(0x0);           // 78 frostwolf hut a_a
                data << uint32(0x530) << uint32(0x0);           // 79 stormpike first aid h_a
                data << uint32(0x52f) << uint32(0x0);           // 80 stormpike first aid h_c
                data << uint32(0x52d) << uint32(0x1);           // 81 stormpike first aid a_c
            }

            break;
        case 3277:                                          // WS
            if (bg && bg->GetTypeID() == BATTLEGROUND_WS)
                bg->FillInitialWorldStates(data);
            else
            {
                data << uint32(0x62d) << uint32(0x0);       // 7 1581 alliance flag captures
                data << uint32(0x62e) << uint32(0x0);       // 8 1582 horde flag captures
                data << uint32(0x609) << uint32(0x0);       // 9 1545 unk, set to 1 on alliance flag pickup...
                data << uint32(0x60a) << uint32(0x0);       // 10 1546 unk, set to 1 on horde flag pickup, after drop it's -1
                data << uint32(0x60b) << uint32(0x2);       // 11 1547 unk
                data << uint32(0x641) << uint32(0x3);       // 12 1601 unk (max flag captures?)
                data << uint32(0x922) << uint32(0x1);       // 13 2338 horde (0 - hide, 1 - flag ok, 2 - flag picked up (flashing), 3 - flag picked up (not flashing)
                data << uint32(0x923) << uint32(0x1);       // 14 2339 alliance (0 - hide, 1 - flag ok, 2 - flag picked up (flashing), 3 - flag picked up (not flashing)
            }
            break;
        case 3358:                                          // AB
            if (bg && bg->GetTypeID() == BATTLEGROUND_AB)
                bg->FillInitialWorldStates(data);
            else
            {
                data << uint32(0x6e7) << uint32(0x0);       // 7 1767 stables alliance
                data << uint32(0x6e8) << uint32(0x0);       // 8 1768 stables horde
                data << uint32(0x6e9) << uint32(0x0);       // 9 1769 unk, ST?
                data << uint32(0x6ea) << uint32(0x0);       // 10 1770 stables (show/hide)
                data << uint32(0x6ec) << uint32(0x0);       // 11 1772 farm (0 - horde controlled, 1 - alliance controlled)
                data << uint32(0x6ed) << uint32(0x0);       // 12 1773 farm (show/hide)
                data << uint32(0x6ee) << uint32(0x0);       // 13 1774 farm color
                data << uint32(0x6ef) << uint32(0x0);       // 14 1775 gold mine color, may be FM?
                data << uint32(0x6f0) << uint32(0x0);       // 15 1776 alliance resources
                data << uint32(0x6f1) << uint32(0x0);       // 16 1777 horde resources
                data << uint32(0x6f2) << uint32(0x0);       // 17 1778 horde bases
                data << uint32(0x6f3) << uint32(0x0);       // 18 1779 alliance bases
                data << uint32(0x6f4) << uint32(0x7d0);     // 19 1780 max resources (2000)
                data << uint32(0x6f6) << uint32(0x0);       // 20 1782 blacksmith color
                data << uint32(0x6f7) << uint32(0x0);       // 21 1783 blacksmith (show/hide)
                data << uint32(0x6f8) << uint32(0x0);       // 22 1784 unk, bs?
                data << uint32(0x6f9) << uint32(0x0);       // 23 1785 unk, bs?
                data << uint32(0x6fb) << uint32(0x0);       // 24 1787 gold mine (0 - horde contr, 1 - alliance contr)
                data << uint32(0x6fc) << uint32(0x0);       // 25 1788 gold mine (0 - conflict, 1 - horde)
                data << uint32(0x6fd) << uint32(0x0);       // 26 1789 gold mine (1 - show/0 - hide)
                data << uint32(0x6fe) << uint32(0x0);       // 27 1790 gold mine color
                data << uint32(0x700) << uint32(0x0);       // 28 1792 gold mine color, wtf?, may be LM?
                data << uint32(0x701) << uint32(0x0);       // 29 1793 lumber mill color (0 - conflict, 1 - horde contr)
                data << uint32(0x702) << uint32(0x0);       // 30 1794 lumber mill (show/hide)
                data << uint32(0x703) << uint32(0x0);       // 31 1795 lumber mill color color
                data << uint32(0x732) << uint32(0x1);       // 32 1842 stables (1 - uncontrolled)
                data << uint32(0x733) << uint32(0x1);       // 33 1843 gold mine (1 - uncontrolled)
                data << uint32(0x734) << uint32(0x1);       // 34 1844 lumber mill (1 - uncontrolled)
                data << uint32(0x735) << uint32(0x1);       // 35 1845 farm (1 - uncontrolled)
                data << uint32(0x736) << uint32(0x1);       // 36 1846 blacksmith (1 - uncontrolled)
                data << uint32(0x745) << uint32(0x2);       // 37 1861 unk
                data << uint32(0x7a3) << uint32(0x708);     // 38 1955 warning limit (1800)
            }
            break;
        case 3820:                                          // EY
            if (bg && bg->GetTypeID() == BATTLEGROUND_EY)
                bg->FillInitialWorldStates(data);
            else
            {
                data << uint32(0xac1) << uint32(0x0);       // 7  2753 Horde Bases
                data << uint32(0xac0) << uint32(0x0);       // 8  2752 Alliance Bases
                data << uint32(0xab6) << uint32(0x0);       // 9  2742 Mage Tower - Horde conflict
                data << uint32(0xab5) << uint32(0x0);       // 10 2741 Mage Tower - Alliance conflict
                data << uint32(0xab4) << uint32(0x0);       // 11 2740 Fel Reaver - Horde conflict
                data << uint32(0xab3) << uint32(0x0);       // 12 2739 Fel Reaver - Alliance conflict
                data << uint32(0xab2) << uint32(0x0);       // 13 2738 Draenei - Alliance conflict
                data << uint32(0xab1) << uint32(0x0);       // 14 2737 Draenei - Horde conflict
                data << uint32(0xab0) << uint32(0x0);       // 15 2736 unk // 0 at start
                data << uint32(0xaaf) << uint32(0x0);       // 16 2735 unk // 0 at start
                data << uint32(0xaad) << uint32(0x0);       // 17 2733 Draenei - Horde control
                data << uint32(0xaac) << uint32(0x0);       // 18 2732 Draenei - Alliance control
                data << uint32(0xaab) << uint32(0x1);       // 19 2731 Draenei uncontrolled (1 - yes, 0 - no)
                data << uint32(0xaaa) << uint32(0x0);       // 20 2730 Mage Tower - Alliance control
                data << uint32(0xaa9) << uint32(0x0);       // 21 2729 Mage Tower - Horde control
                data << uint32(0xaa8) << uint32(0x1);       // 22 2728 Mage Tower uncontrolled (1 - yes, 0 - no)
                data << uint32(0xaa7) << uint32(0x0);       // 23 2727 Fel Reaver - Horde control
                data << uint32(0xaa6) << uint32(0x0);       // 24 2726 Fel Reaver - Alliance control
                data << uint32(0xaa5) << uint32(0x1);       // 25 2725 Fel Reaver uncontrolled (1 - yes, 0 - no)
                data << uint32(0xaa4) << uint32(0x0);       // 26 2724 Boold Elf - Horde control
                data << uint32(0xaa3) << uint32(0x0);       // 27 2723 Boold Elf - Alliance control
                data << uint32(0xaa2) << uint32(0x1);       // 28 2722 Boold Elf uncontrolled (1 - yes, 0 - no)
                data << uint32(0xac5) << uint32(0x1);       // 29 2757 Flag (1 - show, 0 - hide) - doesn't work exactly this way!
                data << uint32(0xad2) << uint32(0x1);       // 30 2770 Horde top-stats (1 - show, 0 - hide) // 02 -> horde picked up the flag
                data << uint32(0xad1) << uint32(0x1);       // 31 2769 Alliance top-stats (1 - show, 0 - hide) // 02 -> alliance picked up the flag
                data << uint32(0xabe) << uint32(0x0);       // 32 2750 Horde resources
                data << uint32(0xabd) << uint32(0x0);       // 33 2749 Alliance resources
                data << uint32(0xa05) << uint32(0x8e);      // 34 2565 unk, constant?
                data << uint32(0xaa0) << uint32(0x0);       // 35 2720 Capturing progress-bar (100 -> empty (only grey), 0 -> blue|red (no grey), default 0)
                data << uint32(0xa9f) << uint32(0x0);       // 36 2719 Capturing progress-bar (0 - left, 100 - right)
                data << uint32(0xa9e) << uint32(0x0);       // 37 2718 Capturing progress-bar (1 - show, 0 - hide)
                data << uint32(0xc0d) << uint32(0x17b);     // 38 3085 unk
                // and some more ... unknown
            }
            break;
        // any of these needs change! the client remembers the prev setting!
        // ON EVERY ZONE LEAVE, RESET THE OLD ZONE'S WORLD STATE, BUT AT LEAST THE UI STUFF!
        case 3483:                                          // Hellfire Peninsula
            {
                if (pvp && pvp->GetTypeId() == OUTDOOR_PVP_HP)
                    pvp->FillInitialWorldStates(data);
                else
                {
                    data << uint32(0x9ba) << uint32(0x1);           // 10 // add ally tower main gui icon       // maybe should be sent only on login?
                    data << uint32(0x9b9) << uint32(0x1);           // 11 // add horde tower main gui icon      // maybe should be sent only on login?
                    data << uint32(0x9b5) << uint32(0x0);           // 12 // show neutral broken hill icon      // 2485
                    data << uint32(0x9b4) << uint32(0x1);           // 13 // show icon above broken hill        // 2484
                    data << uint32(0x9b3) << uint32(0x0);           // 14 // show ally broken hill icon         // 2483
                    data << uint32(0x9b2) << uint32(0x0);           // 15 // show neutral overlook icon         // 2482
                    data << uint32(0x9b1) << uint32(0x1);           // 16 // show the overlook arrow            // 2481
                    data << uint32(0x9b0) << uint32(0x0);           // 17 // show ally overlook icon            // 2480
                    data << uint32(0x9ae) << uint32(0x0);           // 18 // horde pvp objectives captured      // 2478
                    data << uint32(0x9ac) << uint32(0x0);           // 19 // ally pvp objectives captured       // 2476
                    data << uint32(2475)  << uint32(100); //: ally / horde slider grey area                              // show only in direct vicinity!
                    data << uint32(2474)  << uint32(50);  //: ally / horde slider percentage, 100 for ally, 0 for horde  // show only in direct vicinity!
                    data << uint32(2473)  << uint32(0);   //: ally / horde slider display                                // show only in direct vicinity!
                    data << uint32(0x9a8) << uint32(0x0);           // 20 // show the neutral stadium icon      // 2472
                    data << uint32(0x9a7) << uint32(0x0);           // 21 // show the ally stadium icon         // 2471
                    data << uint32(0x9a6) << uint32(0x1);           // 22 // show the horde stadium icon        // 2470
                }
            }
            break;
        case 3518:
            {
                if (pvp && pvp->GetTypeId() == OUTDOOR_PVP_NA)
                    pvp->FillInitialWorldStates(data);
                else
                {
                    data << uint32(2503) << uint32(0x0);    // 10
                    data << uint32(2502) << uint32(0x0);    // 11
                    data << uint32(2493) << uint32(0x0);    // 12
                    data << uint32(2491) << uint32(0x0);    // 13

                    data << uint32(2495) << uint32(0x0);    // 14
                    data << uint32(2494) << uint32(0x0);    // 15
                    data << uint32(2497) << uint32(0x0);    // 16

                    data << uint32(2762) << uint32(0x0);    // 17
                    data << uint32(2662) << uint32(0x0);    // 18
                    data << uint32(2663) << uint32(0x0);    // 19
                    data << uint32(2664) << uint32(0x0);    // 20

                    data << uint32(2760) << uint32(0x0);    // 21
                    data << uint32(2670) << uint32(0x0);    // 22
                    data << uint32(2668) << uint32(0x0);    // 23
                    data << uint32(2669) << uint32(0x0);    // 24

                    data << uint32(2761) << uint32(0x0);    // 25
                    data << uint32(2667) << uint32(0x0);    // 26
                    data << uint32(2665) << uint32(0x0);    // 27
                    data << uint32(2666) << uint32(0x0);    // 28

                    data << uint32(2763) << uint32(0x0);    // 29
                    data << uint32(2659) << uint32(0x0);    // 30
                    data << uint32(2660) << uint32(0x0);    // 31
                    data << uint32(2661) << uint32(0x0);    // 32

                    data << uint32(2671) << uint32(0x0);    // 33
                    data << uint32(2676) << uint32(0x0);    // 34
                    data << uint32(2677) << uint32(0x0);    // 35
                    data << uint32(2672) << uint32(0x0);    // 36
                    data << uint32(2673) << uint32(0x0);    // 37
                }
            }
            break;
        case 3519:                                          // Terokkar Forest
            {
                if (pvp && pvp->GetTypeId() == OUTDOOR_PVP_TF)
                    pvp->FillInitialWorldStates(data);
                else
                {
                    data << uint32(0xa41) << uint32(0x0);           // 10 // 2625 capture bar pos
                    data << uint32(0xa40) << uint32(0x14);          // 11 // 2624 capture bar neutral
                    data << uint32(0xa3f) << uint32(0x0);           // 12 // 2623 show capture bar
                    data << uint32(0xa3e) << uint32(0x0);           // 13 // 2622 horde towers controlled
                    data << uint32(0xa3d) << uint32(0x5);           // 14 // 2621 ally towers controlled
                    data << uint32(0xa3c) << uint32(0x0);           // 15 // 2620 show towers controlled
                    data << uint32(0xa88) << uint32(0x0);           // 16 // 2696 SE Neu
                    data << uint32(0xa87) << uint32(0x0);           // 17 // SE Horde
                    data << uint32(0xa86) << uint32(0x0);           // 18 // SE Ally
                    data << uint32(0xa85) << uint32(0x0);           // 19 //S Neu
                    data << uint32(0xa84) << uint32(0x0);           // 20 S Horde
                    data << uint32(0xa83) << uint32(0x0);           // 21 S Ally
                    data << uint32(0xa82) << uint32(0x0);           // 22 NE Neu
                    data << uint32(0xa81) << uint32(0x0);           // 23 NE Horde
                    data << uint32(0xa80) << uint32(0x0);           // 24 NE Ally
                    data << uint32(0xa7e) << uint32(0x0);           // 25 // 2686 N Neu
                    data << uint32(0xa7d) << uint32(0x0);           // 26 N Horde
                    data << uint32(0xa7c) << uint32(0x0);           // 27 N Ally
                    data << uint32(0xa7b) << uint32(0x0);           // 28 NW Ally
                    data << uint32(0xa7a) << uint32(0x0);           // 29 NW Horde
                    data << uint32(0xa79) << uint32(0x0);           // 30 NW Neutral
                    data << uint32(0x9d0) << uint32(0x5);           // 31 // 2512 locked time remaining seconds first digit
                    data << uint32(0x9ce) << uint32(0x0);           // 32 // 2510 locked time remaining seconds second digit
                    data << uint32(0x9cd) << uint32(0x0);           // 33 // 2509 locked time remaining minutes
                    data << uint32(0x9cc) << uint32(0x0);           // 34 // 2508 neutral locked time show
                    data << uint32(0xad0) << uint32(0x0);           // 35 // 2768 horde locked time show
                    data << uint32(0xacf) << uint32(0x1);           // 36 // 2767 ally locked time show
                }
            }
            break;
        case 3521:                                          // Zangarmarsh
            {
                if (pvp && pvp->GetTypeId() == OUTDOOR_PVP_ZM)
                    pvp->FillInitialWorldStates(data);
                else
                {
                    data << uint32(0x9e1) << uint32(0x0);           // 10 //2529
                    data << uint32(0x9e0) << uint32(0x0);           // 11
                    data << uint32(0x9df) << uint32(0x0);           // 12
                    data << uint32(0xa5d) << uint32(0x1);           // 13 //2653
                    data << uint32(0xa5c) << uint32(0x0);           // 14 //2652 east beacon neutral
                    data << uint32(0xa5b) << uint32(0x1);           // 15 horde
                    data << uint32(0xa5a) << uint32(0x0);           // 16 ally
                    data << uint32(0xa59) << uint32(0x1);           // 17 // 2649 Twin spire graveyard horde  12???
                    data << uint32(0xa58) << uint32(0x0);           // 18 ally     14 ???
                    data << uint32(0xa57) << uint32(0x0);           // 19 neutral  7???
                    data << uint32(0xa56) << uint32(0x0);           // 20 // 2646 west beacon neutral
                    data << uint32(0xa55) << uint32(0x1);           // 21 horde
                    data << uint32(0xa54) << uint32(0x0);           // 22 ally
                    data << uint32(0x9e7) << uint32(0x0);           // 23 // 2535
                    data << uint32(0x9e6) << uint32(0x0);           // 24
                    data << uint32(0x9e5) << uint32(0x0);           // 25
                    data << uint32(0xa00) << uint32(0x0);           // 26 // 2560
                    data << uint32(0x9ff) << uint32(0x1);           // 27
                    data << uint32(0x9fe) << uint32(0x0);           // 28
                    data << uint32(0x9fd) << uint32(0x0);           // 29
                    data << uint32(0x9fc) << uint32(0x1);           // 30
                    data << uint32(0x9fb) << uint32(0x0);           // 31
                    data << uint32(0xa62) << uint32(0x0);           // 32 // 2658
                    data << uint32(0xa61) << uint32(0x1);           // 33
                    data << uint32(0xa60) << uint32(0x1);           // 34
                    data << uint32(0xa5f) << uint32(0x0);           // 35
                }
            }
            break;
        case 3698:                                          // Nagrand Arena
            if (bg && bg->GetTypeID() == BATTLEGROUND_NA)
                bg->FillInitialWorldStates(data);
            else
            {
                data << uint32(0xa0f) << uint32(0x0);           // 7
                data << uint32(0xa10) << uint32(0x0);           // 8
                data << uint32(0xa11) << uint32(0x0);           // 9 show
            }
            break;
        case 3702:                                          // Blade's Edge Arena
            if (bg && bg->GetTypeID() == BATTLEGROUND_BE)
                bg->FillInitialWorldStates(data);
            else
            {
                data << uint32(0x9f0) << uint32(0x0);           // 7 gold
                data << uint32(0x9f1) << uint32(0x0);           // 8 green
                data << uint32(0x9f3) << uint32(0x0);           // 9 show
            }
            break;
        case 3968:                                          // Ruins of Lordaeron
            if (bg && bg->GetTypeID() == BATTLEGROUND_RL)
                bg->FillInitialWorldStates(data);
            else
            {
                data << uint32(0xbb8) << uint32(0x0);           // 7 gold
                data << uint32(0xbb9) << uint32(0x0);           // 8 green
                data << uint32(0xbba) << uint32(0x0);           // 9 show
            }
            break;
        case 3703:                                          // Shattrath City
            break;
        default:
            data << uint32(0x914) << uint32(0x0);           // 7
            data << uint32(0x913) << uint32(0x0);           // 8
            data << uint32(0x912) << uint32(0x0);           // 9
            data << uint32(0x915) << uint32(0x0);           // 10
            break;
    }
    SendPacketToSelf(&data);
}

uint32 Player::GetXPRestBonus(uint32 xp)
{
    uint32 rested_bonus = (uint32)GetRestBonus();           // xp for each rested bonus

    if (rested_bonus > xp)                                   // max rested_bonus == xp or (r+x) = 200% xp
        rested_bonus = xp;

    SetRestBonus(GetRestBonus() - rested_bonus);

    sLog.outDetail("Player gain %u xp (+ %u Rested Bonus). Rested points=%f",xp+rested_bonus,rested_bonus,GetRestBonus());
    return rested_bonus;
}

void Player::SetBindPoint(uint64 guid)
{
    WorldPacket data(SMSG_BINDER_CONFIRM, 8);
    data << uint64(guid);
    SendPacketToSelf(&data);
}

void Player::SendTalentWipeConfirm(uint64 guid)
{
    if (InArenaQueue())
    {
        ChatHandler(this).SendSysMessage(16636);
        return;
    }
    
    WorldPacket data(MSG_TALENT_WIPE_CONFIRM, (8+4));
    data << uint64(guid);
    uint32 cost = sWorld.getConfig(CONFIG_NO_RESET_TALENT_COST) ? 0 : resetTalentsCost();
    data << cost;
    SendPacketToSelf(&data);
}

void Player::SendPetSkillWipeConfirm()
{
    Pet* pet = GetPet();
    if (!pet)
        return;
    WorldPacket data(SMSG_PET_UNLEARN_CONFIRM, (8+4));
    data << pet->GetGUID();
    data << uint32(pet->resetTalentsCost());
    SendPacketToSelf(&data);
}

/*********************************************************/
/***                    STORAGE SYSTEM                 ***/
/*********************************************************/

void Player::SetVirtualItemSlot(uint8 i, Item* item)
{
    ASSERT(i < 3);
    if (i < 2 && item)
    {
        if (!item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
            return;
        uint32 charges = item->GetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT);
        if (charges == 0)
            return;
        if (charges > 1)
            item->SetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT,charges-1);
        else if (charges <= 1)
        {
            ApplyEnchantment(item,TEMP_ENCHANTMENT_SLOT,false);
            item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
        }
    }
}

void Player::SetSheath(uint32 sheathed)
{
    switch (sheathed)
    {
        case SHEATH_STATE_UNARMED:                          // no prepared weapon
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,NULL);
            break;
        case SHEATH_STATE_MELEE:                            // prepared melee weapon
        {
            SetVirtualItemSlot(0,GetWeaponForAttack(BASE_ATTACK,true));
            SetVirtualItemSlot(1,GetWeaponForAttack(OFF_ATTACK,true));
            SetVirtualItemSlot(2,NULL);
        };  break;
        case SHEATH_STATE_RANGED:                           // prepared ranged weapon
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,GetWeaponForAttack(RANGED_ATTACK,true));
            break;
        default:
            SetVirtualItemSlot(0,NULL);
            SetVirtualItemSlot(1,NULL);
            SetVirtualItemSlot(2,NULL);
            break;
    }
    SetByteValue(UNIT_FIELD_BYTES_2, 0, sheathed);          // this must visualize Sheath changing for other players...
}

uint8 Player::FindEquipSlot(ItemPrototype const* proto, uint32 slot, bool swap) const
{
    uint8 pClass = GetClass();

    uint8 slots[4];
    slots[0] = NULL_SLOT;
    slots[1] = NULL_SLOT;
    slots[2] = NULL_SLOT;
    slots[3] = NULL_SLOT;
    switch (proto->InventoryType)
    {
        case INVTYPE_HEAD:
            slots[0] = EQUIPMENT_SLOT_HEAD;
            break;
        case INVTYPE_NECK:
            slots[0] = EQUIPMENT_SLOT_NECK;
            break;
        case INVTYPE_SHOULDERS:
            slots[0] = EQUIPMENT_SLOT_SHOULDERS;
            break;
        case INVTYPE_BODY:
            slots[0] = EQUIPMENT_SLOT_BODY;
            break;
        case INVTYPE_CHEST:
            slots[0] = EQUIPMENT_SLOT_CHEST;
            break;
        case INVTYPE_ROBE:
            slots[0] = EQUIPMENT_SLOT_CHEST;
            break;
        case INVTYPE_WAIST:
            slots[0] = EQUIPMENT_SLOT_WAIST;
            break;
        case INVTYPE_LEGS:
            slots[0] = EQUIPMENT_SLOT_LEGS;
            break;
        case INVTYPE_FEET:
            slots[0] = EQUIPMENT_SLOT_FEET;
            break;
        case INVTYPE_WRISTS:
            slots[0] = EQUIPMENT_SLOT_WRISTS;
            break;
        case INVTYPE_HANDS:
            slots[0] = EQUIPMENT_SLOT_HANDS;
            break;
        case INVTYPE_FINGER:
            slots[0] = EQUIPMENT_SLOT_FINGER1;
            slots[1] = EQUIPMENT_SLOT_FINGER2;
            break;
        case INVTYPE_TRINKET:
            slots[0] = EQUIPMENT_SLOT_TRINKET1;
            slots[1] = EQUIPMENT_SLOT_TRINKET2;
            break;
        case INVTYPE_CLOAK:
            slots[0] =  EQUIPMENT_SLOT_BACK;
            break;
        case INVTYPE_WEAPON:
        {
            slots[0] = EQUIPMENT_SLOT_MAINHAND;

            // suggest offhand slot only if know dual wielding
            // (this will be replace mainhand weapon at auto equip instead unwonted "you don't known dual wielding" ...
            if (CanDualWield())
                slots[1] = EQUIPMENT_SLOT_OFFHAND;
        };break;
        case INVTYPE_SHIELD:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_RANGED:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_2HWEAPON:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            break;
        case INVTYPE_TABARD:
            slots[0] = EQUIPMENT_SLOT_TABARD;
            break;
        case INVTYPE_WEAPONMAINHAND:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            break;
        case INVTYPE_WEAPONOFFHAND:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_HOLDABLE:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_THROWN:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_RANGEDRIGHT:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_BAG:
            slots[0] = INVENTORY_SLOT_BAG_1;
            slots[1] = INVENTORY_SLOT_BAG_2;
            slots[2] = INVENTORY_SLOT_BAG_3;
            slots[3] = INVENTORY_SLOT_BAG_4;
            break;
        case INVTYPE_RELIC:
        {
            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_LIBRAM:
                    if (pClass == CLASS_PALADIN)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_IDOL:
                    if (pClass == CLASS_DRUID)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_TOTEM:
                    if (pClass == CLASS_SHAMAN)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_MISC:
                    if (pClass == CLASS_WARLOCK)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
            }
            break;
        }
        default :
            return NULL_SLOT;
    }

    if (slot != NULL_SLOT)
    {
        if (swap || !GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            for (int i = 0; i < 4; i++)
            {
                if (slots[i] == slot)
                    return slot;
            }
        }
    }
    else
    {
        // search free slot at first
        for (int i = 0; i < 4; i++)
        {
            if (slots[i] != NULL_SLOT && !GetItemByPos(INVENTORY_SLOT_BAG_0, slots[i]))
            {
                // in case 2hand equipped weapon offhand slot empty but not free
                if (slots[i]==EQUIPMENT_SLOT_OFFHAND)
                {
                    Item* mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (!mainItem || mainItem->GetProto()->InventoryType != INVTYPE_2HWEAPON)
                        return slots[i];
                }
                else
                    return slots[i];
            }
        }

        // if not found free and can swap return first appropriate from used
        for (int i = 0; i < 4; i++)
        {
            if (slots[i] != NULL_SLOT && swap)
                return slots[i];
        }
    }

    // no free position
    return NULL_SLOT;
}

uint8 Player::CanUnequipItems(uint32 item, uint32 count) const
{
    Item *pItem;
    uint32 tempcount = 0;

    uint8 res = EQUIP_ERR_OK;

    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            uint8 ires = CanUnequipItem(INVENTORY_SLOT_BAG_0 << 8 | i, false);
            if (ires==EQUIP_ERR_OK)
            {
                tempcount += pItem->GetCount();
                if (tempcount >= count)
                    return EQUIP_ERR_OK;
            }
            else
                res = ires;
        }
    }
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return EQUIP_ERR_OK;
        }
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return EQUIP_ERR_OK;
        }
    }
    Bag *pBag;
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                pItem = GetItemByPos(i, j);
                if (pItem && pItem->GetEntry() == item)
                {
                    tempcount += pItem->GetCount();
                    if (tempcount >= count)
                        return EQUIP_ERR_OK;
                }
            }
        }
    }

    // not found req. item count and have unequippable items
    return res;
}

uint32 Player::GetItemCount(uint32 item, bool inBankAlso, Item* skipItem) const
{
    uint32 count = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem != skipItem &&  pItem->GetEntry() == item)
            count += pItem->GetCount();
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem != skipItem && pItem->GetEntry() == item)
            count += pItem->GetCount();
    }
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
            count += pBag->GetItemCount(item,skipItem);
    }

    if (skipItem && skipItem->GetProto()->GemProperties)
    {
        for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
        {
            Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem != skipItem && pItem->GetProto()->Socket[0].Color)
                count += pItem->GetGemCountWithID(item);
        }
    }

    if (inBankAlso)
    {
        for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem != skipItem && pItem->GetEntry() == item)
                count += pItem->GetCount();
        }
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pBag)
            {
                count += pBag->GetItemCount(item, skipItem);
                if (pBag->GetEntry() == item && pBag != skipItem) // count the bag itself
                    count++;
            }
        }

        if (skipItem && skipItem->GetProto()->GemProperties)
        {
            for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
            {
                Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pItem && pItem != skipItem && pItem->GetProto()->Socket[0].Color)
                    count += pItem->GetGemCountWithID(item);
            }
        }
    }

    return count;
}

Item* Player::GetItemByGuid(uint64 guid) const
{
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetGUID() == guid)
            return pItem;
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetGUID() == guid)
            return pItem;
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetGUID() == guid)
                    return pItem;
            }
        }
    }

	for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
	{
		Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
		if (pItem && pItem->GetGUIDLow() == guid)
			return pItem;
	}

    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetGUID() == guid)
                    return pItem;
            }
        }
    }

    return NULL;
}

Item* Player::GetItemByGuidLow(uint32 guid) const
{
	for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
	{
		Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
		if (pItem && pItem->GetGUIDLow() == guid)
			return pItem;
	}
	for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
	{
		Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
		if (pItem && pItem->GetGUIDLow() == guid)
			return pItem;
	}

	for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
	{
		Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
		if (pBag)
		{
			for (uint32 j = 0; j < pBag->GetBagSize(); j++)
			{
				Item* pItem = pBag->GetItemByPos(j);
				if (pItem && pItem->GetGUIDLow() == guid)
					return pItem;
			}
		}
	}

	for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
	{
		Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
		if (pItem && pItem->GetGUIDLow() == guid)
			return pItem;
	}

	for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
	{
		Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
		if (pBag)
		{
			for (uint32 j = 0; j < pBag->GetBagSize(); j++)
			{
				Item* pItem = pBag->GetItemByPos(j);
				if (pItem && pItem->GetGUIDLow() == guid)
					return pItem;
			}
		}
	}

	return NULL;
}

Item* Player::GetItemByPos(uint16 pos) const
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    return GetItemByPos(bag, slot);
}

Item* Player::GetItemByPos(uint8 bag, uint8 slot) const
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot < BANK_SLOT_BAG_END || slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END))
        return m_items[slot];
    else if (bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END
        || bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
            return pBag->GetItemByPos(slot);
    }
    return NULL;
}

uint32 Player::GetItemDisplayIdInSlot(uint8 bag, uint8 slot) const
{
    // Why not just GetUint32value from visual items? it will be needed when implementing model system with race support
    const Item* pItem = GetItemByPos(bag, slot);

    if (!pItem)
        return 0;

    return pItem->GetProto()->DisplayInfoID;
}

Item* Player::GetWeaponForAttack(WeaponAttackType attackType, bool useable) const
{
    uint16 slot;
    switch (attackType)
    {
        case BASE_ATTACK:   slot = EQUIPMENT_SLOT_MAINHAND; break;
        case OFF_ATTACK:    slot = EQUIPMENT_SLOT_OFFHAND;  break;
        case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED;   break;
        default: return NULL;
    }

    Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!item || item->GetProto()->Class != ITEM_CLASS_WEAPON)
        return NULL;

    if (!useable)
        return item;

    if (item->IsBroken() || !IsUseEquipedWeapon(attackType==BASE_ATTACK))
        return NULL;

    return item;
}

Item* Player::GetShield(bool useable) const
{
    Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!item || item->GetProto()->Class != ITEM_CLASS_ARMOR)
        return NULL;

    if (!useable)
        return item;

    if (item->IsBroken())
        return NULL;

    return item;
}

uint32 Player::GetAttackBySlot(uint8 slot)
{
    switch (slot)
    {
        case EQUIPMENT_SLOT_MAINHAND: return BASE_ATTACK;
        case EQUIPMENT_SLOT_OFFHAND:  return OFF_ATTACK;
        case EQUIPMENT_SLOT_RANGED:   return RANGED_ATTACK;
        default:                      return MAX_ATTACK;
    }
}

bool Player::HasBankBagSlot(uint8 slot) const
{
    uint32 maxslot = GetByteValue(PLAYER_BYTES_2, 2) + BANK_SLOT_BAG_START;
    if (slot < maxslot)
        return true;
    return false;
}

bool Player::IsInventoryPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && slot == NULL_SLOT)
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END))
        return true;
    if (bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END))
        return true;
    return false;
}

bool Player::IsEquipmentPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot < EQUIPMENT_SLOT_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END))
        return true;
    return false;
}

bool Player::IsBankPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
        return true;
    if (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END)
        return true;
    return false;
}

bool Player::IsBagPos(uint16 pos)
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
        return true;
    return false;
}

bool Player::IsValidPos(uint8 bag, uint8 slot) const
{
    // post selected
    if (bag == NULL_BAG)
        return true;

    if (bag == INVENTORY_SLOT_BAG_0)
    {
        // any post selected
        if (slot == NULL_SLOT)
            return true;

        // equipment
        if (slot < EQUIPMENT_SLOT_END)
            return true;

        // bag equip slots
        if (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END)
            return true;

        // backpack slots
        if (slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END)
            return true;

        // keyring slots
        if (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END)
            return true;

        // bank main slots
        if (slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END)
            return true;

        // bank bag slots
        if (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END)
            return true;

        return false;
    }

    // bag content slots
    if (bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
    {
        Bag* pBag = (Bag*)GetItemByPos (INVENTORY_SLOT_BAG_0, bag);
        if (!pBag)
            return false;

        // any post selected
        if (slot == NULL_SLOT)
            return true;

        return slot < pBag->GetBagSize();
    }

    // bank bag content slots
    if (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END)
    {
        Bag* pBag = (Bag*)GetItemByPos (INVENTORY_SLOT_BAG_0, bag);
        if (!pBag)
            return false;

        // any post selected
        if (slot == NULL_SLOT)
            return true;

        return slot < pBag->GetBagSize();
    }

    // where this?
    return false;
}

Item * Player::HasEquiped(uint32 item) const
{
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
            return pItem;
    }

    return NULL;
}

bool Player::HasItemCount(uint32 item, uint32 count, bool inBankAlso) const
{
    uint32 tempcount = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return true;
        }
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return true;
        }
    }
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = GetItemByPos(i, j);
                if (pItem && pItem->GetEntry() == item)
                {
                    tempcount += pItem->GetCount();
                    if (tempcount >= count)
                        return true;
                }
            }
        }
    }

    if (inBankAlso)
    {
        for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem->GetEntry() == item)
            {
                tempcount += pItem->GetCount();
                if (tempcount >= count)
                    return true;
            }
        }
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    Item* pItem = GetItemByPos(i, j);
                    if (pItem && pItem->GetEntry() == item)
                    {
                        tempcount += pItem->GetCount();
                        if (tempcount >= count)
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

Item* Player::GetItemOrItemWithGemEquipped(uint32 item) const
{
    Item *pItem;
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
            return pItem;
    }

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(item);
    if (pProto && pProto->GemProperties)
    {
        for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
        {
            pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem->GetProto()->Socket[0].Color)
            {
                if (pItem->GetGemCountWithID(item) > 0)
                    return pItem;
            }
        }
    }

    return NULL;
}

/* ToDo: Trentone: Make this method trigger-like so it will save itemLevel in Player, but only recalculate it on item equip*/
//int32 Player::GetLegendaryItemsNumber(bool Penalty) const
//{
//    // disabled
//    return 0;
//
//    Item *pItem;
//    int32 QualityItemsNumber = 0;
//    int32 MaxItemLevel = 0;
//    bool Legendary;
//    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
//    {
//        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
//        if (pItem)
//        {
//            ItemPrototype const *ItemProto = pItem->GetProto();
//            Legendary = (ItemProto->Quality == ITEM_QUALITY_LEGENDARY || ItemProto->Quality == ITEM_QUALITY_ARTIFACT);
//            switch (ItemProto->InventoryType)
//            {
//                case INVTYPE_HOLDABLE:
//                case INVTYPE_SHIELD:
//                    MaxItemLevel += 190;
//                    if (Legendary)
//                        QualityItemsNumber += 190;
//                    break;
//                case INVTYPE_HEAD:
//                    MaxItemLevel += 312;
//                    if (Legendary)
//                        QualityItemsNumber += 312;
//                    break;
//                case INVTYPE_NECK:
//                    MaxItemLevel += 203;
//                    if (Legendary)
//                        QualityItemsNumber += 203;
//                    break;
//                case INVTYPE_SHOULDERS:
//                    MaxItemLevel += 227;
//                    if (Legendary)
//                        QualityItemsNumber += 227;
//                    break;
//                case INVTYPE_CHEST:
//                case INVTYPE_ROBE:
//                    MaxItemLevel += 303;
//                    if (Legendary)
//                        QualityItemsNumber += 303;
//                    break;
//                case INVTYPE_WAIST:
//                case INVTYPE_FEET:
//                case INVTYPE_HANDS:
//                    MaxItemLevel += 231;
//                    if (Legendary)
//                        QualityItemsNumber += 231;
//                    break;
//                case INVTYPE_LEGS:
//                    MaxItemLevel += 292;
//                    if (Legendary)
//                        QualityItemsNumber += 292;
//                    break;
//                case INVTYPE_WRISTS:
//                    MaxItemLevel += 161;
//                    if (Legendary)
//                        QualityItemsNumber += 161;
//                    break;
//                case INVTYPE_FINGER:
//                    MaxItemLevel += 184;
//                    if (Legendary)
//                        QualityItemsNumber += 184;
//                    break;
//                case INVTYPE_TRINKET:
//                    MaxItemLevel += 160;
//                    if (Legendary)
//                        QualityItemsNumber += 160;
//                    break;
//                case INVTYPE_WEAPON:
//                case INVTYPE_WEAPONMAINHAND:
//                    MaxItemLevel += 524;
//                    if (Legendary)
//                    {
//                        QualityItemsNumber += 524;
//                        bool PhysItem = false;
//                        for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
//                            if (((ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin) * 500 / ItemProto->Delay) > 100) // Phys weapons 1h have more than 100 DPS - really ~150
//                            {
//                                PhysItem = true;
//                                break;
//                            }
//                        if (PhysItem)
//                        {
//                            QualityItemsNumber += 173;
//                            MaxItemLevel += 173;
//                        }
//                    }
//                    else
//                    {
//                        bool PhysItem = false;
//                        for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
//                            if (((ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin) * 500 / ItemProto->Delay) > 70) // Phys weapons 1h have more than 70 - really ~100
//                            {
//                                PhysItem = true;
//                                break;
//                            }
//                        if (PhysItem)
//                            MaxItemLevel += 173;
//                    }
//                    break;
//                case INVTYPE_CLOAK:
//                    MaxItemLevel += 196;
//                    if (Legendary)
//                        QualityItemsNumber += 196;
//                    break;
//                case INVTYPE_2HWEAPON: // Feral staff counted as SPD
//                    MaxItemLevel += 747;
//                    if (Legendary)
//                    {
//                        QualityItemsNumber += 747;
//                        bool PhysItem = false;
//                        for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
//                            if (((ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin) * 500 / ItemProto->Delay) > 150) // Phys weapons 2h have more than 150 DPS - really ~220, most SPD DPS staff is feral staff - 115 dps
//                            {
//                                PhysItem = true;
//                                break;
//                            }
//                        if (PhysItem)
//                        {
//                            QualityItemsNumber += 317;
//                            MaxItemLevel += 317;
//                        }
//                    }
//                    else
//                    {
//                        bool PhysItem = false;
//                        for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
//                            if (((ItemProto->Damage[i].DamageMax + ItemProto->Damage[i].DamageMin) * 500 / ItemProto->Delay) > 100) // Phys weapons 2h have more than 100 DPS - really ~130
//                            {
//                                PhysItem = true;
//                                break;
//                            }
//                        if (PhysItem)
//                            MaxItemLevel += 317;
//                    }
//                    break;
//                case INVTYPE_RELIC:
//                    MaxItemLevel += 120;
//                    if (Legendary)
//                        QualityItemsNumber += 120;
//                    break;
//                case INVTYPE_RANGEDRIGHT:
//                case INVTYPE_RANGED:
//                case INVTYPE_THROWN:
//                    MaxItemLevel += 573;
//                    if (Legendary)
//                        QualityItemsNumber += 573;
//                    if (ItemProto->SubClass == ITEM_SUBCLASS_WEAPON_WAND)
//                    {
//                        MaxItemLevel += 477;
//                        if (Legendary)
//                            QualityItemsNumber += 477;
//                    }
//                    break;
//                default:
//                    break;
//            }
//        }
//        else
//        {
//            switch (i)
//            {
//                case EQUIPMENT_SLOT_HEAD:
//                    MaxItemLevel += 312;
//                    break;
//                case EQUIPMENT_SLOT_NECK:
//                    MaxItemLevel += 203;
//                    break;
//                case EQUIPMENT_SLOT_SHOULDERS:
//                    MaxItemLevel += 227;
//                    break;
//                case EQUIPMENT_SLOT_CHEST:
//                    MaxItemLevel += 303;
//                    break;
//                case EQUIPMENT_SLOT_LEGS:
//                    MaxItemLevel += 292;
//                    break;
//                case EQUIPMENT_SLOT_FEET:
//                case EQUIPMENT_SLOT_HANDS:
//                case EQUIPMENT_SLOT_WAIST:
//                    MaxItemLevel += 231;
//                    break;
//                case EQUIPMENT_SLOT_WRISTS:
//                    MaxItemLevel += 161;
//                    break;
//                case EQUIPMENT_SLOT_FINGER1:
//                case EQUIPMENT_SLOT_FINGER2:
//                    MaxItemLevel += 184;
//                    break;
//                case EQUIPMENT_SLOT_TRINKET1:
//                case EQUIPMENT_SLOT_TRINKET2:
//                    MaxItemLevel += 160;
//                    break;
//                case EQUIPMENT_SLOT_BACK:
//                    MaxItemLevel += 196;
//                    break;
//                case EQUIPMENT_SLOT_MAINHAND:
//                    if (GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
//                        MaxItemLevel += 697;
//                    else
//                        MaxItemLevel += 1064;
//                    break;
//                case EQUIPMENT_SLOT_OFFHAND:
//                    if (Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
//                        if (!pItem || pItem->GetProto()->InventoryType != INVTYPE_2HWEAPON)
//                            MaxItemLevel += 120;
//                    break;
//                case EQUIPMENT_SLOT_RANGED:
//                {
//                    switch (GetClass())
//                    {
//                        case CLASS_WARRIOR:
//                        case CLASS_HUNTER:
//                        case CLASS_ROGUE:
//                            MaxItemLevel += 573;
//                            break;
//                        case CLASS_SHAMAN:
//                        case CLASS_PALADIN:
//                        case CLASS_DRUID:
//                            MaxItemLevel += 120;
//                            break;
//                        case CLASS_MAGE:
//                        case CLASS_WARLOCK:
//                        case CLASS_PRIEST:
//                            MaxItemLevel += 1050;
//                            break;
//                    }
//                    break;
//                }
//
//            }
//        }
//    }
//    if (Penalty)
//    {
//        QualityItemsNumber = QualityItemsNumber * 3 / 2;
//        if (QualityItemsNumber > MaxItemLevel)
//            return 100;
//    }
//    return QualityItemsNumber*100/MaxItemLevel;
//}

uint8 Player::_CanTakeMoreSimilarItems(uint32 entry, uint32 count, Item* pItem, uint32* no_space_count) const
{
    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(entry);
    if (!pProto)
    {
        if (no_space_count)
            *no_space_count = count;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    // no maximum
    if (pProto->MaxCount == 0)
        return EQUIP_ERR_OK;

    uint32 curcount = GetItemCount(pProto->ItemId,true,pItem);

    if (curcount + count > pProto->MaxCount)
    {
        if (no_space_count)
            *no_space_count = count +curcount - pProto->MaxCount;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    return EQUIP_ERR_OK;
}

bool Player::HasItemTotemCategory(uint32 TotemCategory) const
{
    Item *pItem;
    for (uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && IsTotemCategoryCompatiableWith(pItem->GetProto()->TotemCategory,TotemCategory))
            return true;
    }
    for (uint8 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && IsTotemCategoryCompatiableWith(pItem->GetProto()->TotemCategory,TotemCategory))
            return true;
    }
    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                pItem = GetItemByPos(i, j);
                if (pItem && IsTotemCategoryCompatiableWith(pItem->GetProto()->TotemCategory,TotemCategory))
                    return true;
            }
        }
    }
    return false;
}

uint8 Player::_CanStoreItem_InSpecificSlot(uint8 bag, uint8 slot, ItemPosCountVec &dest, ItemPrototype const *pProto, uint32& count, bool swap, Item* pSrcItem) const
{
    Item* pItem2 = GetItemByPos(bag, slot);

    // ignore move item (this slot will be empty at move)
    if (pItem2==pSrcItem)
        pItem2 = NULL;

    uint32 need_space;

    if (pSrcItem && pSrcItem->IsBag() && !((Bag*)pSrcItem)->IsEmpty() && !IsBagPos(uint16(bag) << 8 | slot))
        return EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS;

    // empty specific slot - check item fit to slot
    if (!pItem2 || swap)
    {
        if (bag == INVENTORY_SLOT_BAG_0)
        {
            // keyring case
            if (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_START+GetMaxKeyringSize() && !(pProto->BagFamily & BAG_FAMILY_MASK_KEYS))
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            // prevent cheating
            if (slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END || slot >= PLAYER_SLOT_END)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
        }
        else
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (!pBag)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            ItemPrototype const* pBagProto = pBag->GetProto();
            if (!pBagProto)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            if (slot >= pBagProto->ContainerSlots)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            if (!ItemCanGoIntoBag(pProto,pBagProto))
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
        }

        // non empty stack with space
        need_space = pProto->Stackable;
    }
    // non empty slot, check item type
    else
    {
        // check item type
        if (pItem2->GetEntry() != pProto->ItemId)
            return EQUIP_ERR_ITEM_CANT_STACK;

        // check free space
        if (pItem2->GetCount() >= pProto->Stackable)
            return EQUIP_ERR_ITEM_CANT_STACK;

        need_space = pProto->Stackable - pItem2->GetCount();
    }

    if (need_space > count)
        need_space = count;

    ItemPosCount newPosition = ItemPosCount((bag << 8) | slot, need_space);
    if (!newPosition.isContainedIn(dest))
    {
        dest.push_back(newPosition);
        count -= need_space;
    }
    return EQUIP_ERR_OK;
}

uint8 Player::_CanStoreItem_InBag(uint8 bag, ItemPosCountVec &dest, ItemPrototype const *pProto, uint32& count, bool merge, bool non_specialized, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const
{
    // skip specific bag already processed in first called _CanStoreItem_InBag
    if (bag==skip_bag)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
    if (!pBag || pBag==pSrcItem)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    if (pSrcItem && pSrcItem->IsBag() && !((Bag*)pSrcItem)->IsEmpty())
        return EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS;

    ItemPrototype const* pBagProto = pBag->GetProto();
    if (!pBagProto)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    // specialized bag mode or non-specilized
    if (non_specialized != (pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass == ITEM_SUBCLASS_CONTAINER))
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    if (!ItemCanGoIntoBag(pProto,pBagProto))
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    for (uint32 j = 0; j < pBag->GetBagSize(); j++)
    {
        // skip specific slot already processed in first called _CanStoreItem_InSpecificSlot
        if (j==skip_slot)
            continue;

        Item* pItem2 = GetItemByPos(bag, j);

        // ignore move item (this slot will be empty at move)
        if (pItem2==pSrcItem)
            pItem2 = NULL;

        // if merge skip empty, if !merge skip non-empty
        if ((pItem2!=NULL)!=merge)
            continue;

        if (pItem2)
        {
            if (pItem2->GetEntry() == pProto->ItemId && pItem2->GetCount() < pProto->Stackable)
            {
                uint32 need_space = pProto->Stackable - pItem2->GetCount();
                if (need_space > count)
                    need_space = count;

                ItemPosCount newPosition = ItemPosCount((bag << 8) | j, need_space);
                if (!newPosition.isContainedIn(dest))
                {
                    dest.push_back(newPosition);
                    count -= need_space;

                    if (count==0)
                        return EQUIP_ERR_OK;
                }
            }
        }
        else
        {
            uint32 need_space = pProto->Stackable;
            if (need_space > count)
                need_space = count;

            ItemPosCount newPosition = ItemPosCount((bag << 8) | j, need_space);
            if (!newPosition.isContainedIn(dest))
            {
                dest.push_back(newPosition);
                count -= need_space;

                if (count==0)
                    return EQUIP_ERR_OK;
            }
        }
    }
    return EQUIP_ERR_OK;
}

uint8 Player::_CanStoreItem_InInventorySlots(uint8 slot_begin, uint8 slot_end, ItemPosCountVec &dest, ItemPrototype const *pProto, uint32& count, bool merge, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const
{
    // this is never called for non-bag slots so we can do this
    if (pSrcItem && pSrcItem->IsBag() && !((Bag*)pSrcItem)->IsEmpty())
        return EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS;

    for (uint32 j = slot_begin; j < slot_end; j++)
    {
        // skip specific slot already processed in first called _CanStoreItem_InSpecificSlot
        if (INVENTORY_SLOT_BAG_0==skip_bag && j==skip_slot)
            continue;

        Item* pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, j);

        // ignore move item (this slot will be empty at move)
        if (pItem2==pSrcItem)
            pItem2 = NULL;

        // if merge skip empty, if !merge skip non-empty
        if ((pItem2!=NULL)!=merge)
            continue;

        if (pItem2)
        {
            if (pItem2->GetEntry() == pProto->ItemId && pItem2->GetCount() < pProto->Stackable)
            {
                uint32 need_space = pProto->Stackable - pItem2->GetCount();
                if (need_space > count)
                    need_space = count;
                ItemPosCount newPosition = ItemPosCount((INVENTORY_SLOT_BAG_0 << 8) | j, need_space);
                if (!newPosition.isContainedIn(dest))
                {
                    dest.push_back(newPosition);
                    count -= need_space;

                    if (count==0)
                        return EQUIP_ERR_OK;
                }
            }
        }
        else
        {
            uint32 need_space = pProto->Stackable;
            if (need_space > count)
                need_space = count;

            ItemPosCount newPosition = ItemPosCount((INVENTORY_SLOT_BAG_0 << 8) | j, need_space);
            if (!newPosition.isContainedIn(dest))
            {
                dest.push_back(newPosition);
                count -= need_space;

                if (count==0)
                    return EQUIP_ERR_OK;
            }
        }
    }
    return EQUIP_ERR_OK;
}

uint8 Player::_CanStoreItem(uint8 bag, uint8 slot, ItemPosCountVec &dest, uint32 entry, uint32 count, Item *pItem, bool swap, uint32* no_space_count) const
{
    sLog.outDebug("STORAGE: CanStoreItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, entry, count);

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(entry);
    if (!pProto)
    {
        if (no_space_count)
            *no_space_count = count;
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED :EQUIP_ERR_ITEM_NOT_FOUND;
    }

    if (pItem && pItem->IsBindedNotWith(GetGUID()))
    {
        if (no_space_count)
            *no_space_count = count;
        return EQUIP_ERR_DONT_OWN_THAT_ITEM;
    }

    // check count of items (skip for auto move for same player from bank)
    uint32 no_similar_count = 0;                            // can't store this amount similar items
    uint8 res = _CanTakeMoreSimilarItems(entry,count,pItem,&no_similar_count);
    if (res!=EQUIP_ERR_OK)
    {
        if (count==no_similar_count)
        {
            if (no_space_count)
                *no_space_count = no_similar_count;
            return res;
        }
        count -= no_similar_count;
    }

    // in specific slot
    if (bag != NULL_BAG && slot != NULL_SLOT)
    {
        if (!IsValidPos(bag, slot))
            return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;
        res = _CanStoreItem_InSpecificSlot(bag,slot,dest,pProto,count,swap,pItem);
        if (res!=EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count==0)
        {
            if (no_similar_count==0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

    // not specific slot or have space for partly store only in specific slot

    // in specific bag
    if (bag != NULL_BAG)
    {
        // search stack in bag for merge to
        if (pProto->Stackable > 1)
        {
            if (bag == INVENTORY_SLOT_BAG_0)               // inventory
            {
                res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START,KEYRING_SLOT_END,dest,pProto,count,true,pItem,bag,slot);
                if (res!=EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count==0)
                {
                    if (no_similar_count==0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }

                res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START,INVENTORY_SLOT_ITEM_END,dest,pProto,count,true,pItem,bag,slot);
                if (res!=EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count==0)
                {
                    if (no_similar_count==0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
            else                                            // equipped bag
            {
                // we need check 2 time (specialized/non_specialized), use NULL_BAG to prevent skipping bag
                res = _CanStoreItem_InBag(bag,dest,pProto,count,true,false,pItem,NULL_BAG,slot);
                if (res!=EQUIP_ERR_OK)
                    res = _CanStoreItem_InBag(bag,dest,pProto,count,true,true,pItem,NULL_BAG,slot);

                if (res!=EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count==0)
                {
                    if (no_similar_count==0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
        }

        // search free slot in bag for place to
        if (bag == INVENTORY_SLOT_BAG_0)                   // inventory
        {
            // if src item is bag don't search empty slot to avoid puting bag into self
            // it can happen because bag is removed after finding free slot which can be in swaping bag
            //if (pItem->IsBag())
            //    return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;

            // search free slot - keyring case
            if (pProto->BagFamily & BAG_FAMILY_MASK_KEYS)
            {
                uint32 keyringSize = GetMaxKeyringSize();
                res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START,KEYRING_SLOT_START+keyringSize,dest,pProto,count,false,pItem,bag,slot);
                if (res!=EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count==0)
                {
                    if (no_similar_count==0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }

            res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START,INVENTORY_SLOT_ITEM_END,dest,pProto,count,false,pItem,bag,slot);
            if (res!=EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count==0)
            {
                if (no_similar_count==0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
        else                                                // equipped bag
        {
            res = _CanStoreItem_InBag(bag,dest,pProto,count,false,false,pItem,NULL_BAG,slot);
            if (res!=EQUIP_ERR_OK)
                res = _CanStoreItem_InBag(bag,dest,pProto,count,false,true,pItem,NULL_BAG,slot);

            if (res!=EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count==0)
            {
                if (no_similar_count==0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // not specific bag or have space for partly store only in specific bag

    // search stack for merge to
    if (pProto->Stackable > 1)
    {
        res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START,KEYRING_SLOT_END,dest,pProto,count,true,pItem,bag,slot);
        if (res!=EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count==0)
        {
            if (no_similar_count==0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }

        res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START,INVENTORY_SLOT_ITEM_END,dest,pProto,count,true,pItem,bag,slot);
        if (res!=EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count==0)
        {
            if (no_similar_count==0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }

        if (pProto->BagFamily)
        {
            for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            {
                res = _CanStoreItem_InBag(i,dest,pProto,count,true,false,pItem,bag,slot);
                if (res!=EQUIP_ERR_OK)
                    continue;

                if (count==0)
                {
                    if (no_similar_count==0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i,dest,pProto,count,true,true,pItem,bag,slot);
            if (res!=EQUIP_ERR_OK)
                continue;

            if (count==0)
            {
                if (no_similar_count==0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // search free slot - special bag case
    if (pProto->BagFamily)
    {
        if (pProto->BagFamily & BAG_FAMILY_MASK_KEYS)
        {
            uint32 keyringSize = GetMaxKeyringSize();
            res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START,KEYRING_SLOT_START+keyringSize,dest,pProto,count,false,pItem,bag,slot);
            if (res!=EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count==0)
            {
                if (no_similar_count==0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i,dest,pProto,count,false,false,pItem,bag,slot);
            if (res!=EQUIP_ERR_OK)
                continue;

            if (count==0)
            {
                if (no_similar_count==0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

/*  TODO: rewrite to avoid crash when adding any item to bag
    // Normally it would be impossible to autostore not empty bags
    if (pItem->IsBag() && !((Bag*)pItem)->IsEmpty())
        return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
*/

    // search free slot
    res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START,INVENTORY_SLOT_ITEM_END,dest,pProto,count,false,pItem,bag,slot);
    if (res!=EQUIP_ERR_OK)
    {
        if (no_space_count)
            *no_space_count = count + no_similar_count;
        return res;
    }

    if (count==0)
    {
        if (no_similar_count==0)
            return EQUIP_ERR_OK;

        if (no_space_count)
            *no_space_count = count + no_similar_count;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        res = _CanStoreItem_InBag(i,dest,pProto,count,false,true,pItem,bag,slot);
        if (res!=EQUIP_ERR_OK)
            continue;

        if (count==0)
        {
            if (no_similar_count==0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

    if (no_space_count)
        *no_space_count = count + no_similar_count;

    return EQUIP_ERR_INVENTORY_FULL;
}

//////////////////////////////////////////////////////////////////////////
uint8 Player::CanStoreItems(Item **pItems,int count) const
{
    Item    *pItem2;

    // fill space table
    int inv_slot_items[INVENTORY_SLOT_ITEM_END-INVENTORY_SLOT_ITEM_START];
    int inv_bags[INVENTORY_SLOT_BAG_END-INVENTORY_SLOT_BAG_START][MAX_BAG_SIZE];
    int inv_keys[KEYRING_SLOT_END-KEYRING_SLOT_START];

    memset(inv_slot_items,0,sizeof(int)*(INVENTORY_SLOT_ITEM_END-INVENTORY_SLOT_ITEM_START));
    memset(inv_bags,0,sizeof(int)*(INVENTORY_SLOT_BAG_END-INVENTORY_SLOT_BAG_START)*MAX_BAG_SIZE);
    memset(inv_keys,0,sizeof(int)*(KEYRING_SLOT_END-KEYRING_SLOT_START));

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (pItem2 && !pItem2->IsInTrade())
        {
            inv_slot_items[i-INVENTORY_SLOT_ITEM_START] = pItem2->GetCount();
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (pItem2 && !pItem2->IsInTrade())
        {
            inv_keys[i-KEYRING_SLOT_START] = pItem2->GetCount();
        }
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                pItem2 = GetItemByPos(i, j);
                if (pItem2 && !pItem2->IsInTrade())
                {
                    inv_bags[i-INVENTORY_SLOT_BAG_START][j] = pItem2->GetCount();
                }
            }
        }
    }

    // check free space for all items
    for (int k=0;k<count;k++)
    {
        Item  *pItem = pItems[k];

        // no item
        if (!pItem)  continue;

        sLog.outDebug("STORAGE: CanStoreItems %i. item = %u, count = %u", k+1, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const *pProto = pItem->GetProto();

        // strange item
        if (!pProto)
            return EQUIP_ERR_ITEM_NOT_FOUND;

        // item it 'bind'
        if (pItem->IsBindedNotWith(GetGUID()))
            return EQUIP_ERR_DONT_OWN_THAT_ITEM;

        Bag *pBag;
        ItemPrototype const *pBagProto;

        // item is 'one item only'
        uint8 res = CanTakeMoreSimilarItems(pItem);
        if (res != EQUIP_ERR_OK)
            return res;

        // search stack for merge to
        if (pProto->Stackable > 1)
        {
            bool b_found = false;

            for (int t = KEYRING_SLOT_START; t < KEYRING_SLOT_END; t++)
            {
                pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_keys[t-KEYRING_SLOT_START] + pItem->GetCount() <= pProto->Stackable)
                {
                    inv_keys[t-KEYRING_SLOT_START] += pItem->GetCount();
                    b_found = true;
                    break;
                }
            }
            if (b_found) continue;

            for (int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; t++)
            {
                pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_slot_items[t-INVENTORY_SLOT_ITEM_START] + pItem->GetCount() <= pProto->Stackable)
                {
                    inv_slot_items[t-INVENTORY_SLOT_ITEM_START] += pItem->GetCount();
                    b_found = true;
                    break;
                }
            }
            if (b_found) continue;

            for (int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
            {
                pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pBag)
                {
                    for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                    {
                        pItem2 = GetItemByPos(t, j);
                        if (pItem2 && pItem2->GetEntry() == pItem->GetEntry() && inv_bags[t-INVENTORY_SLOT_BAG_START][j] + pItem->GetCount() <= pProto->Stackable)
                        {
                            inv_bags[t-INVENTORY_SLOT_BAG_START][j] += pItem->GetCount();
                            b_found = true;
                            break;
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // special bag case
        if (pProto->BagFamily)
        {
            bool b_found = false;
            if (pProto->BagFamily & BAG_FAMILY_MASK_KEYS)
            {
                uint32 keyringSize = GetMaxKeyringSize();
                for (uint32 t = KEYRING_SLOT_START; t < KEYRING_SLOT_START+keyringSize; ++t)
                {
                    if (inv_keys[t-KEYRING_SLOT_START] == 0)
                    {
                        inv_keys[t-KEYRING_SLOT_START] = 1;
                        b_found = true;
                        break;
                    }
                }
            }

            if (b_found) continue;

            for (int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
            {
                pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pBag)
                {
                    pBagProto = pBag->GetProto();

                    // not plain container check
                    if (pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER) &&
                        ItemCanGoIntoBag(pProto,pBagProto))
                    {
                        for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                        {
                            if (inv_bags[t-INVENTORY_SLOT_BAG_START][j] == 0)
                            {
                                inv_bags[t-INVENTORY_SLOT_BAG_START][j] = 1;
                                b_found = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // search free slot
        bool b_found = false;
        for (int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; t++)
        {
            if (inv_slot_items[t-INVENTORY_SLOT_ITEM_START] == 0)
            {
                inv_slot_items[t-INVENTORY_SLOT_ITEM_START] = 1;
                b_found = true;
                break;
            }
        }
        if (b_found) continue;

        // search free slot in bags
        for (int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; t++)
        {
            pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, t);
            if (pBag)
            {
                pBagProto = pBag->GetProto();

                // special bag already checked
                if (pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER))
                    continue;

                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    if (inv_bags[t-INVENTORY_SLOT_BAG_START][j] == 0)
                    {
                        inv_bags[t-INVENTORY_SLOT_BAG_START][j] = 1;
                        b_found = true;
                        break;
                    }
                }
            }
        }

        // no free slot found?
        if (!b_found)
            return EQUIP_ERR_INVENTORY_FULL;
    }

    return EQUIP_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
uint8 Player::CanEquipNewItem(uint8 slot, uint16 &dest, uint32 item, bool swap) const
{
    dest = 0;
    Item *pItem = Item::CreateItem(item, 1, this);
    if (pItem)
    {
        uint8 result = CanEquipItem(slot, dest, pItem, swap);
        delete pItem;
        return result;
    }

    return EQUIP_ERR_ITEM_NOT_FOUND;
}

uint8 Player::CanEquipItem(uint8 slot, uint16 &dest, Item *pItem, bool swap, bool not_loading) const
{
    dest = 0;
    if (pItem)
    {
        sLog.outDebug("STORAGE: CanEquipItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const *pProto = pItem->GetProto();
        if (pProto)
        {
            if (pItem->IsBindedNotWith(GetGUID()))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            // check count of items (skip for auto move for same player from bank)
            uint8 res = CanTakeMoreSimilarItems(pItem);
            if (res != EQUIP_ERR_OK)
                return res;

            // check this only in game
            if (not_loading)
            {
                // May be here should be more stronger checks; STUNNED checked
                // ROOT, CONFUSED, DISTRACTED, FLEEING this needs to be checked.
                // && !HasAura(9454) - for solo 3v3 resilience in arena check
                if (HasUnitState(UNIT_STAT_STUNNED))
                    return EQUIP_ERR_YOU_ARE_STUNNED;

                // do not allow equipping gear except weapons, offhands, projectiles, relics in
                // - combat
                // - in-progress arenas
                BattleGround* bg = GetBattleGround();

                if (!pProto->CanChangeEquipStateInCombat())
                {
                    if (IsInCombat())
                        return EQUIP_ERR_NOT_IN_COMBAT;

                    if (bg)
                        if (bg->isArena() && bg->GetStatus() == STATUS_IN_PROGRESS)
                            return EQUIP_ERR_NOT_DURING_ARENA_MATCH;
                }

                if (IsNonMeleeSpellCast(false))
                {
                    // exclude spells with transform item effect
                    if (!m_currentSpells[CURRENT_GENERIC_SPELL] ||
                        (m_currentSpells[CURRENT_GENERIC_SPELL]->GetSpellEntry()->Effect[0] != SPELL_EFFECT_SUMMON_CHANGE_ITEM &&
                        m_currentSpells[CURRENT_GENERIC_SPELL]->GetSpellEntry()->Effect[1] != SPELL_EFFECT_SUMMON_CHANGE_ITEM &&
                        m_currentSpells[CURRENT_GENERIC_SPELL]->GetSpellEntry()->Effect[2] != SPELL_EFFECT_SUMMON_CHANGE_ITEM))

                        return EQUIP_ERR_CANT_DO_RIGHT_NOW;
                }

                //// check resilience for solo 3v3
                //if (InArena() && bg->GetArenaType() == ARENA_TYPE_3v3)
                //{
                //    uint32 old_item_resilience = 0;

                //    if (uint8 eslot = FindEquipSlot(pProto, slot, swap))
                //    {
                //        if (Item* it = GetItemByPos(INVENTORY_SLOT_BAG_0, eslot))
                //        {
                //            if (ItemPrototype const *iProto = it->GetProto())
                //            {
                //                for (int j = 0; j < MAX_ITEM_PROTO_STATS; j++)
                //                {
                //                    if (iProto->ItemStat[j].ItemStatValue != 0) {
                //                        switch (iProto->ItemStat[j].ItemStatType)
                //                        {
                //                        case ITEM_MOD_RESILIENCE_RATING:
                //                            old_item_resilience += iProto->ItemStat[j].ItemStatValue;
                //                        }
                //                    }
                //                }
                //            }
                //        }
                //    }

                //    uint32 new_item_resilience = 0;
                //    for (int j = 0; j < MAX_ITEM_PROTO_STATS; j++)
                //    {
                //        if (pProto->ItemStat[j].ItemStatValue != 0) {
                //            switch (pProto->ItemStat[j].ItemStatType)
                //            {
                //            case ITEM_MOD_RESILIENCE_RATING:
                //                new_item_resilience += pProto->ItemStat[j].ItemStatValue;
                //            }
                //        }
                //    }

                //    uint32 a = GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_TAKEN_SPELL);
                //    if (GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + CR_CRIT_TAKEN_SPELL) - old_item_resilience + new_item_resilience < SOLO_3V3_REQUIRED_RESILIENCE)
                //    {
                //        if (!HasAura(9454))
                //            ((Unit*)this)->AddAura(9454, (Unit*)this);

                //        ChatHandler((Player*)this).PSendSysMessage(LANG_NOT_ENOUGH_RESILIENCE, uint32(SOLO_3V3_REQUIRED_RESILIENCE));
                //    }
                //    else
                //    {
                //        if (HasAura(9454))
                //            ((Unit*)this)->RemoveAurasDueToSpell(9454);
                //    }
                //}
            }

            uint8 eslot = FindEquipSlot(pProto, slot, swap);
            if (eslot == NULL_SLOT)
                return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

            uint8 msg = CanUseItem(pItem , not_loading);
            if (msg != EQUIP_ERR_OK)
                return msg;
            if (!swap && GetItemByPos(INVENTORY_SLOT_BAG_0, eslot))
                return EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE;

            // check unique-equipped on item
            if (pProto->Flags & ITEM_FLAGS_UNIQUE_EQUIPPED)
            {
                // there is an equip limit on this item
                Item* tItem = GetItemOrItemWithGemEquipped(pProto->ItemId);
                if (tItem && (!swap || tItem->GetSlot() != eslot))
                    return EQUIP_ERR_ITEM_UNIQUE_EQUIPABLE;
            }

            // check unique-equipped on gems
            for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
            {
                uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if (!enchant_id)
                    continue;
                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if (!enchantEntry)
                    continue;

                ItemPrototype const* pGem = ObjectMgr::GetItemPrototype(enchantEntry->GemID);
                if (pGem && (pGem->Flags & ITEM_FLAGS_UNIQUE_EQUIPPED))
                {
                    Item* tItem = GetItemOrItemWithGemEquipped(enchantEntry->GemID);
                    if (tItem && (!swap || tItem->GetSlot() != eslot))
                        return EQUIP_ERR_ITEM_UNIQUE_EQUIPABLE;
                }
            }

            // check unique-equipped special item classes
            if (pProto->Class == ITEM_CLASS_QUIVER)
            {
                for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                {
                    if (Item* pBag = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    {
                        if(pBag != pItem)
                        {
                            if (ItemPrototype const* pBagProto = pBag->GetProto())
                            {
                                if (pBagProto->Class==pProto->Class && (!swap || pBag->GetSlot() != eslot))
                                {
                                    if (pBagProto->SubClass == ITEM_SUBCLASS_AMMO_POUCH)
                                        return EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH;
                                    else
                                        return EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER;
                                }
                            }
                        }
                    }
                }
            }

            uint32 type = pProto->InventoryType;

            if (eslot == EQUIPMENT_SLOT_OFFHAND)
            {
                if (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND)
                {
                    if (!CanDualWield())
                        return EQUIP_ERR_CANT_DUAL_WIELD;
                }

                Item *mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                if (mainItem)
                {
                    if (mainItem->GetProto()->InventoryType == INVTYPE_2HWEAPON)
                        return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED;
                }
            }

            // equip two-hand weapon case (with possible unequip 2 items)
            if (type == INVTYPE_2HWEAPON)
            {
                if (eslot != EQUIPMENT_SLOT_MAINHAND)
                    return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

                // offhand item must can be stored in inventory for offhand item and it also must be unequipped
                Item *offItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                ItemPosCountVec off_dest;
                if (offItem && (!not_loading ||
                    CanUnequipItem(uint16(INVENTORY_SLOT_BAG_0) << 8 | EQUIPMENT_SLOT_OFFHAND,false) !=  EQUIP_ERR_OK ||
                    CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false) !=  EQUIP_ERR_OK))
                    return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_INVENTORY_FULL;
            }

            // any moment of arena
            //if (BattleGround *bg = GetBattleGround()) 
            //{
            //    if (bg->isArena())
            //    {
            //        if (const_cast<Player*>(this)->IsCustomLegendaryWeapon(pProto->ItemId))
            //            return EQUIP_ERR_NOT_DURING_ARENA_MATCH;
            //    }
            //}

            dest = ((INVENTORY_SLOT_BAG_0 << 8) | eslot);
            return EQUIP_ERR_OK;
        }
    }
    if (!swap)
        return EQUIP_ERR_ITEM_NOT_FOUND;
    else
        return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

uint8 Player::CanUnequipItem(uint16 pos, bool swap) const
{
    // Applied only to equipped items and bank bags
    if (!IsEquipmentPos(pos) && !IsBagPos(pos))
        return EQUIP_ERR_OK;

    Item* pItem = GetItemByPos(pos);

    // Applied only to existed equipped item
    if (!pItem)
        return EQUIP_ERR_OK;

    sLog.outDebug("STORAGE: CanUnequipItem slot = %u, item = %u, count = %u", pos, pItem->GetEntry(), pItem->GetCount());

    ItemPrototype const *pProto = pItem->GetProto();
    if (!pProto)
        return EQUIP_ERR_ITEM_NOT_FOUND;

    // do not allow unequipping gear except weapons, offhands, projectiles, relics in
    // - combat
    // - in-progress arenas
    if (!pProto->CanChangeEquipStateInCombat())
    {
        if (IsInCombat())
            return EQUIP_ERR_NOT_IN_COMBAT;

        if (BattleGround* bg = GetBattleGround())
            if (bg->isArena() && bg->GetStatus() == STATUS_IN_PROGRESS)
                return EQUIP_ERR_NOT_DURING_ARENA_MATCH;
    }

    // prevent swaping bags and equipped items if player is trading
    if (pTrader && swap)
    {
        if (pItem->IsBag() || pItem->IsEquipped())
            return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
    }

    if (!swap && pItem->IsBag() && !((Bag*)pItem)->IsEmpty())
        return EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS;

    return EQUIP_ERR_OK;
}

uint8 Player::CanBankItem(uint8 bag, uint8 slot, ItemPosCountVec &dest, Item *pItem, bool swap, bool not_loading) const
{
    if (!pItem)
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;

    //if (pItem->m_lootGenerated)
    //{
    //    GetSession()->DoLootRelease(GetLootGUID());
    //    return EQUIP_ERR_OK;
    //}

    uint32 count = pItem->GetCount();

    sLog.outDebug("STORAGE: CanBankItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), pItem->GetCount());
    ItemPrototype const *pProto = pItem->GetProto();
    if (!pProto)
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;

    if (pItem->IsBindedNotWith(GetGUID()))
        return EQUIP_ERR_DONT_OWN_THAT_ITEM;

    // check count of items (skip for auto move for same player from bank)
    uint8 res = CanTakeMoreSimilarItems(pItem);
    if (res != EQUIP_ERR_OK)
        return res;

    // in specific slot
    if (bag != NULL_BAG && slot != NULL_SLOT)
    {
        if (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END)
        {
            if (!pItem->IsBag())
                 return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;

            if(!HasBankBagSlot(slot))
                return EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT;

            if (uint8 cantuse = CanUseItem(pItem, not_loading) != EQUIP_ERR_OK)
                return cantuse;
        }

        res = _CanStoreItem_InSpecificSlot(bag,slot,dest,pProto,count,swap,pItem);
        if (res!=EQUIP_ERR_OK)
            return res;

        if (count==0)
            return EQUIP_ERR_OK;
    }

    // not specific slot or have space for partly store only in specific slot

    // in specific bag
    if (bag != NULL_BAG)
    {
        if (pProto->InventoryType == INVTYPE_BAG)
        {
            Bag *pBag = (Bag*)pItem;
            if (pBag && !pBag->IsEmpty())
                return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
        }

        // search stack in bag for merge to
        if (pProto->Stackable > 1)
        {
            if (bag == INVENTORY_SLOT_BAG_0)
            {
                res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START,BANK_SLOT_ITEM_END,dest,pProto,count,true,pItem,bag,slot);
                if (res!=EQUIP_ERR_OK)
                    return res;

                if (count==0)
                    return EQUIP_ERR_OK;
            }
            else
            {
                res = _CanStoreItem_InBag(bag,dest,pProto,count,true,false,pItem,NULL_BAG,slot);
                if (res!=EQUIP_ERR_OK)
                    res = _CanStoreItem_InBag(bag,dest,pProto,count,true,true,pItem,NULL_BAG,slot);

                if (res!=EQUIP_ERR_OK)
                    return res;

                if (count==0)
                    return EQUIP_ERR_OK;
            }
        }

        // search free slot in bag
        if (bag == INVENTORY_SLOT_BAG_0)
        {
            res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START,BANK_SLOT_ITEM_END,dest,pProto,count,false,pItem,bag,slot);
            if (res!=EQUIP_ERR_OK)
                return res;

            if (count==0)
                return EQUIP_ERR_OK;
        }
        else
        {
            res = _CanStoreItem_InBag(bag,dest,pProto,count,false,false,pItem,NULL_BAG,slot);
            if (res!=EQUIP_ERR_OK)
                res = _CanStoreItem_InBag(bag,dest,pProto,count,false,true,pItem,NULL_BAG,slot);

            if (res!=EQUIP_ERR_OK)
                return res;

            if (count==0)
                return EQUIP_ERR_OK;
        }
    }

    // not specific bag or have space for partly store only in specific bag

    // search stack for merge to
    if (pProto->Stackable > 1)
    {
        // in slots
        res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START,BANK_SLOT_ITEM_END,dest,pProto,count,true,pItem,bag,slot);
        if (res!=EQUIP_ERR_OK)
            return res;

        if (count==0)
            return EQUIP_ERR_OK;

        // in special bags
        if (pProto->BagFamily)
        {
            for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
            {
                res = _CanStoreItem_InBag(i,dest,pProto,count,true,false,pItem,bag,slot);
                if (res!=EQUIP_ERR_OK)
                    continue;

                if (count==0)
                    return EQUIP_ERR_OK;
            }
        }

        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i,dest,pProto,count,true,true,pItem,bag,slot);
            if (res!=EQUIP_ERR_OK)
                continue;

            if (count==0)
                return EQUIP_ERR_OK;
        }
    }

    // search free place in special bag
    if (pProto->BagFamily)
    {
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            res = _CanStoreItem_InBag(i,dest,pProto,count,false,false,pItem,bag,slot);
            if (res!=EQUIP_ERR_OK)
                continue;

            if (count==0)
                return EQUIP_ERR_OK;
        }
    }

    // search free space
    res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START,BANK_SLOT_ITEM_END,dest,pProto,count,false,pItem,bag,slot);
    if (res!=EQUIP_ERR_OK)
        return res;

    if (count==0)
        return EQUIP_ERR_OK;

    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        res = _CanStoreItem_InBag(i,dest,pProto,count,false,true,pItem,bag,slot);
        if (res!=EQUIP_ERR_OK)
            continue;

        if (count==0)
            return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_BANK_FULL;
}

uint8 Player::CanUseItem(Item *pItem, bool not_loading) const
{
    if (pItem)
    {
        sLog.outDebug("STORAGE: CanUseItem item = %u", pItem->GetEntry());
        if (!isAlive() && not_loading)
            return EQUIP_ERR_YOU_ARE_DEAD;
        //if(isStunned())
        //    return EQUIP_ERR_YOU_ARE_STUNNED;
        if (pItem->IsBindedNotWith(GetGUID()))
            return EQUIP_ERR_DONT_OWN_THAT_ITEM;

        if (pItem->GetSkill() != 0 && GetSkillValue(pItem->GetSkill()) == 0)
            return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;

        ItemPrototype const *pProto = pItem->GetProto();
        if (pProto)
        {
            if ((pProto->AllowableClass & GetClassMask()) == 0 || (pProto->AllowableRace & GetRaceMask()) == 0)
                return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;

            if (pProto->RequiredSkill != 0 )
            {
                if (GetSkillValue(pProto->RequiredSkill) == 0)
                    return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
                else if (GetSkillValue(pProto->RequiredSkill) < pProto->RequiredSkillRank)
                    return EQUIP_ERR_ERR_CANT_EQUIP_SKILL;
            }
            if (pProto->RequiredSpell != 0 && !HasSpell(pProto->RequiredSpell))
                return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;

            if (pProto->RequiredReputationFaction && uint32(GetReputationMgr().GetRank(pProto->RequiredReputationFaction)) < pProto->RequiredReputationRank)
                return EQUIP_ERR_CANT_EQUIP_REPUTATION;

            if (GetLevel() < pProto->RequiredLevel)
                return EQUIP_ERR_CANT_EQUIP_LEVEL_I;
            return EQUIP_ERR_OK;
        }
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

bool Player::CanUseItem(ItemPrototype const *pProto)
{
    // Used by group, function NeedBeforeGreed, to know if a prototype can be used by a player
    // and by AH sorting

    if (!pProto)
        return false;

    if ((pProto->AllowableClass & GetClassMask()) == 0 || (pProto->AllowableRace & GetRaceMask()) == 0)
        return false;

    if (pProto->RequiredSkill != 0 )
    {
        if (GetSkillValue(pProto->RequiredSkill) == 0)
            return false;
        else if (GetSkillValue(pProto->RequiredSkill) < pProto->RequiredSkillRank)
            return false;
    }

    if (pProto->RequiredSpell != 0 && !HasSpell(pProto->RequiredSpell))
        return false;

    if (GetLevel() < pProto->RequiredLevel)
        return false;

    if (pProto->Class == ITEM_CLASS_RECIPE)
    {
        if (pProto->Spells[0].SpellId == SPELL_ID_GENERIC_LEARN)
        {
            if (HasSpell(pProto->Spells[1].SpellId))
                return false;
        }
        else
        {
            SpellEntry const * spellInfo = GetSpellStore()->LookupEntry<SpellEntry>(pProto->Spells[0].SpellId);

            if (spellInfo)
                for (uint8 i = 0; i < 3; ++i)
                    if (spellInfo->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
                        if (HasSpell(spellInfo->EffectTriggerSpell[i]))
                            return false;
        }
    }
    return true;
}

uint8 Player::CanUseAmmo(uint32 item) const
{
    sLog.outDebug("STORAGE: CanUseAmmo item = %u", item);
    if (!isAlive())
        return EQUIP_ERR_YOU_ARE_DEAD;
    //if(isStunned())
    //    return EQUIP_ERR_YOU_ARE_STUNNED;
    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(item);
    if (pProto)
    {
        if (pProto->InventoryType!= INVTYPE_AMMO)
            return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;
        if ((pProto->AllowableClass & GetClassMask()) == 0 || (pProto->AllowableRace & GetRaceMask()) == 0)
            return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
        if (pProto->RequiredSkill != 0 )
        {
            if (GetSkillValue(pProto->RequiredSkill) == 0)
                return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            else if (GetSkillValue(pProto->RequiredSkill) < pProto->RequiredSkillRank)
                return EQUIP_ERR_ERR_CANT_EQUIP_SKILL;
        }
        if (pProto->RequiredSpell != 0 && !HasSpell(pProto->RequiredSpell))
            return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
        /*if(GetReputation() < pProto->RequiredReputation)
        return EQUIP_ERR_CANT_EQUIP_REPUTATION;
        */
        if (GetLevel() < pProto->RequiredLevel)
            return EQUIP_ERR_CANT_EQUIP_LEVEL_I;

        // Requires No Ammo
        if (GetDummyAura(46699))
            return EQUIP_ERR_BAG_FULL6;

        return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

void Player::SetAmmo(uint32 item)
{
    if (!item)
        return;

    // already set
    if (GetUInt32Value(PLAYER_AMMO_ID) == item)
        return;

    // check ammo
    if (item)
    {
        uint8 msg = CanUseAmmo(item);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL);
            return;
        }
    }

    SetUInt32Value(PLAYER_AMMO_ID, item);

    _ApplyAmmoBonuses();
}

void Player::RemoveAmmo()
{
    SetUInt32Value(PLAYER_AMMO_ID, 0);

    m_ammoDPS = 0.0f;

    if (CanModifyStats())
        UpdateDamagePhysical(RANGED_ATTACK);
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::StoreNewItem(ItemPosCountVec const& dest, uint32 item, bool update,int32 randomPropertyId, std::string from_source)
{
    uint32 count = 0;
    for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
        count += itr->count;

    Item *pItem = Item::CreateItem(item, count, this);
    if (pItem)
    {
        ItemAddedQuestCheck(item, count);
        if (randomPropertyId)
            pItem->SetItemRandomProperties(randomPropertyId);
        pItem = StoreItem(dest, pItem, update);
    }

	sLog.outLog(LOG_CHARACTER_ACTIONS, "#item Player %s (guid %u) stored item %u:%u - %s", GetName(), GetGUIDLow(), item, count, from_source.c_str());

    return pItem;
}

Item* Player::StoreItem(ItemPosCountVec const& dest, Item* pItem, bool update)
{
    if (!pItem)
        return NULL;

    Item* lastItem = pItem;

    for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end();)
    {
        uint16 pos = itr->pos;
        uint32 count = itr->count;

        ++itr;

        if (itr == dest.end())
        {
            lastItem = _StoreItem(pos,pItem,count,false,update);
            break;
        }

        lastItem = _StoreItem(pos,pItem,count,true,update);
    }

    return lastItem;
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::_StoreItem(uint16 pos, Item *pItem, uint32 count, bool clone, bool update)
{
    if (!pItem)
        return NULL;

    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;

    sLog.outDebug("STORAGE: StoreItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), count);
    sLog.outLog(LOG_ITEM_STORE, "STORAGE: StoreItem bag = %u, slot = %u, item = %u, count = %u, player %s (%u),", bag, slot, pItem->GetEntry(), count, GetName(), GetGUIDLow());

    Item *pItem2 = GetItemByPos(bag, slot);

    if (!pItem2)
    {
        if (clone)
            pItem = pItem->CloneItem(count,this);
        else
            pItem->SetCount(count);

        if (!pItem)
            return NULL;

        if (pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP ||
            pItem->GetProto()->Bonding == BIND_QUEST_ITEM ||
            pItem->GetProto()->Bonding == BIND_WHEN_EQUIPED && IsBagPos(pos))
            pItem->SetBinding(true);

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            m_items[slot] = pItem;
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2)), pItem->GetGUID());
            pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
            pItem->SetUInt64Value(ITEM_FIELD_OWNER, GetGUID());

            pItem->SetSlot(slot);
            pItem->SetContainer(NULL);

            if (IsInWorld() && update)
            {
                pItem->AddToWorld();
                pItem->SendCreateUpdateToPlayer(this);
            }

            pItem->SetState(ITEM_CHANGED, this);
        }
        else
        {
            Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (pBag)
            {
                pBag->StoreItem(slot, pItem, update);
                if (IsInWorld() && update)
                {
                    pItem->AddToWorld();
                    pItem->SendCreateUpdateToPlayer(this);
                }
                pItem->SetState(ITEM_CHANGED, this);
                pBag->SetState(ITEM_CHANGED, this);
            }
        }

        AddEnchantmentDurations(pItem);
        AddItemDurations(pItem);

        return pItem;
    }
    else if (pItem2 != pItem)
    {
        if (pItem2->GetProto()->Bonding == BIND_WHEN_PICKED_UP ||
            pItem2->GetProto()->Bonding == BIND_QUEST_ITEM ||
            pItem2->GetProto()->Bonding == BIND_WHEN_EQUIPED && IsBagPos(pos))
            pItem2->SetBinding(true);

        pItem2->SetCount(pItem2->GetCount() + count);
        if (IsInWorld() && update)
            pItem2->SendCreateUpdateToPlayer(this);

        if (!clone)
        {
            // delete item (it not in any slot currently)
            if (IsInWorld() && update)
            {
                pItem->RemoveFromWorld();
                pItem->DestroyForPlayer(this);
            }

            RemoveEnchantmentDurations(pItem);
            RemoveItemDurations(pItem);

            pItem->SetOwnerGUID(GetGUID());                 // prevent error at next SetState in case trade/mail/buy from vendor
            pItem->SetState(ITEM_REMOVED, this);
        }
        // AddItemDurations(pItem2); - pItem2 already have duration listed for player
        AddEnchantmentDurations(pItem2);

        pItem2->SetState(ITEM_CHANGED, this);

        return pItem2;
    }
    else
    {
        //item is in this place already ... for me its WTF but it does happens for bugged items when picking them from mail
        return pItem2;
    }
}

Item* Player::EquipNewItem(uint16 pos, uint32 item, bool update)
{
    Item *pItem = Item::CreateItem(item, 1, this);
    if (pItem)
    {
        ItemAddedQuestCheck(item, 1);
        Item * retItem = EquipItem(pos, pItem, update);

        return retItem;
    }
    return NULL;
}

Item* Player::EquipItem(uint16 pos, Item *pItem, bool update)
{
    AddEnchantmentDurations(pItem);
    AddItemDurations(pItem);

    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;

    Item *pItem2 = GetItemByPos(bag, slot);

    if (!pItem2)
    {
        VisualizeItem(slot, pItem);

        if ((slot < 10 && slot != 1 || slot == 14 || slot == 18) && HasAura(55119))
            CastSpell(this, 55118, true);

        if (isAlive())
        {
            ItemPrototype const *pProto = pItem->GetProto();

            // item set bonuses applied only at equip and removed at unequip, and still active for broken items
            if (pProto && pProto->ItemSet)
                AddItemsSetItem(this,pItem);

            //if (slot < 10 && slot != 1 && slot != 3)
            //    CheckLevel5Reinforcement();

            _ApplyItemMods(pItem, slot, true);
            
            // all items that can be equipped in combat do cooldown for you
            if (IsInCombat())
            {
                resetAttackTimer(BASE_ATTACK);
                resetAttackTimer(OFF_ATTACK);
                resetAttackTimer(RANGED_ATTACK);

                uint32 cooldownSpell = SPELL_ID_WEAPON_SWITCH_COOLDOWN_1_5s;

                if (GetClass() == CLASS_ROGUE)
                    cooldownSpell = SPELL_ID_WEAPON_SWITCH_COOLDOWN_1_0s;

                SpellEntry const* spellProto = sSpellTemplate.LookupEntry<SpellEntry>(cooldownSpell);

                if (!spellProto)
                    sLog.outLog(LOG_DEFAULT, "ERROR: Weapon switch cooldown spell %u couldn't be found in Spell.dbc", cooldownSpell);
                else
                {
                    GetCooldownMgr().AddGlobalCooldown(spellProto, spellProto->StartRecoveryTime);

                    WorldPacket data(SMSG_SPELL_COOLDOWN, 8+1+4);
                    data << uint64(GetGUID());
                    data << uint8(1);
                    data << uint32(cooldownSpell);
                    data << uint32(0);
                    SendPacketToSelf(&data);
                }
            }
        }

        if (IsInWorld() && update)
        {
            pItem->AddToWorld();
            pItem->SendCreateUpdateToPlayer(this);
        }

        ApplyEquipCooldown(pItem);

        if (slot == EQUIPMENT_SLOT_MAINHAND)
            UpdateExpertise(BASE_ATTACK);
        else if (slot == EQUIPMENT_SLOT_OFFHAND)
            UpdateExpertise(OFF_ATTACK);
    }
    else
    {
        pItem2->SetCount(pItem2->GetCount() + pItem->GetCount());
        if (IsInWorld() && update)
            pItem2->SendCreateUpdateToPlayer(this);

        // delete item (it not in any slot currently)
        //pItem->DeleteFromDB();
        if (IsInWorld() && update)
        {
            pItem->RemoveFromWorld();
            pItem->DestroyForPlayer(this);
        }

        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        pItem->SetOwnerGUID(GetGUID());                 // prevent error at next SetState in case trade/mail/buy from vendor
        pItem->SetState(ITEM_REMOVED, this);
        pItem2->SetState(ITEM_CHANGED, this);

        ApplyEquipCooldown(pItem2);

        return pItem2;
    }

    return pItem;
}

void Player::QuickEquipItem(uint16 pos, Item *pItem)
{
    if (pItem)
    {
        AddEnchantmentDurations(pItem);
        AddItemDurations(pItem);

        uint8 slot = pos & 255;
        VisualizeItem(slot, pItem);

        if (IsInWorld())
        {
            pItem->AddToWorld();
            pItem->SendCreateUpdateToPlayer(this);
        }
    }
}

void Player::SetVisibleItemSlot(uint8 slot, Item *pItem)
{
    // PLAYER_VISIBLE_ITEM_i_CREATOR    // Size: 2
    // PLAYER_VISIBLE_ITEM_i_0          // Size: 12
    //    entry                         //      Size: 1
    //    inspected enchantments        //      Size: 6
    //    ?                             //      Size: 5
    // PLAYER_VISIBLE_ITEM_i_PROPERTIES // Size: 1 (property,suffix factor)
    // PLAYER_VISIBLE_ITEM_i_PAD        // Size: 1
    //                                  //     = 16

    if (pItem)
    {
        SetUInt64Value(PLAYER_VISIBLE_ITEM_1_CREATOR + (slot * MAX_VISIBLE_ITEM_OFFSET), pItem->GetUInt64Value(ITEM_FIELD_CREATOR));

        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
        uint32 itemEntry = pItem->GetEntry();

        bool NeedForce = false;
        // mainhand slot 15, offhand 16, ranged 17
        if (slot > 14 && slot < 18) // 15 16 17
        {
            const ItemPrototype* proto = pItem->GetProto();
            if (proto)
            {
                uint8 subClass = proto->SubClass;
                uint32 activeItemEntry = GetTransmogManager()->GetActiveTransEntry(slot - 15, GetTransmogManager()->GetWeaponTypeFromSubclass(subClass, slot - 15, proto->Class), true);
                if (activeItemEntry)
                {
                    itemEntry = activeItemEntry;
                    if (GetUInt32Value(VisibleBase) == itemEntry)
                    {
                        if (HasAura(55148, 0))
                            NeedForce = true;
                        else if (BattleGround* bg = GetBattleGround())
                        {
                            if (bg->isArena())
                                NeedForce = true;
                        }
                    }
                }
            }
        }

        if (NeedForce)
            ForceValuesUpdateAtIndex(VisibleBase);
        else
            SetUInt32Value(VisibleBase, itemEntry);

        for (int i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            SetUInt32Value(VisibleBase + 1 + i, pItem->GetEnchantmentId(EnchantmentSlot(i)));

        // Use SetInt16Value to prevent set high part to FFFF for negative value
        SetInt16Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + (slot * MAX_VISIBLE_ITEM_OFFSET), 0, pItem->GetItemRandomPropertyId());
        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (slot * MAX_VISIBLE_ITEM_OFFSET), pItem->GetItemSuffixFactor());
    }
    else
    {
        SetUInt64Value(PLAYER_VISIBLE_ITEM_1_CREATOR + (slot * MAX_VISIBLE_ITEM_OFFSET), 0);

        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(VisibleBase + 0, 0);

        for (int i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            SetUInt32Value(VisibleBase + 1 + i, 0);

        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 0 + (slot * MAX_VISIBLE_ITEM_OFFSET), 0);
        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (slot * MAX_VISIBLE_ITEM_OFFSET), 0);
    }
}

void Player::VisualizeItem(uint8 slot, Item *pItem)
{
    if (!pItem)
        return;

    // check also  BIND_WHEN_PICKED_UP and BIND_QUEST_ITEM for .additem or .additemset case by GM (not binded at adding to inventory)
    if (pItem->GetProto()->Bonding == BIND_WHEN_EQUIPED || pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP || pItem->GetProto()->Bonding == BIND_QUEST_ITEM)
        pItem->SetBinding(true);

    sLog.outDebug("STORAGE: EquipItem slot = %u, item = %u", slot, pItem->GetEntry());

    m_items[slot] = pItem;
    SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2)), pItem->GetGUID());
    pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, GetGUID());
    pItem->SetUInt64Value(ITEM_FIELD_OWNER, GetGUID());
    pItem->SetSlot(slot);
    pItem->SetContainer(NULL);

    if (slot < EQUIPMENT_SLOT_END)
        SetVisibleItemSlot(slot,pItem);

    pItem->SetState(ITEM_CHANGED, this);
}

void Player::RemoveItem(uint8 bag, uint8 slot, bool update)
{
    // note: removeitem does not actually change the item
    // it only takes the item out of storage temporarily
    // note2: if removeitem is to be used for delinking
    // the item must be removed from the player's updatequeue

    Item *pItem = GetItemByPos(bag, slot);
    if (pItem)
    {
        sLog.outDebug("STORAGE: RemoveItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());

        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            if (slot < INVENTORY_SLOT_BAG_END)
            {
                ItemPrototype const *pProto = pItem->GetProto();
                // item set bonuses applied only at equip and removed at unequip, and still active for broken items

                if (pProto && pProto->ItemSet)
                    RemoveItemsSetItem(this,pProto);

                //if (slot < 10 && slot != 1 && slot != 3)
                //    CheckLevel5Reinforcement(true);

                _ApplyItemMods(pItem, slot, false);

                // remove item dependent auras and casts (only weapon and armor slots)
                if (slot < EQUIPMENT_SLOT_END)
                {
                    RemoveItemDependentAurasAndCasts(pItem);

                    // remove held enchantments
                    if (slot == EQUIPMENT_SLOT_MAINHAND)
                    {
                        if (pItem->GetItemSuffixFactor())
                        {
                            pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_3);
                            pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_4);
                        }
                        else
                        {
                            pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_0);
                            pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_1);
                        }

                        UpdateExpertise(BASE_ATTACK, true);
                    }
                    else if( slot == EQUIPMENT_SLOT_OFFHAND )
                        UpdateExpertise(OFF_ATTACK, true);
                }
            }

            m_items[slot] = NULL;
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            if (slot < EQUIPMENT_SLOT_END)
            {
                SetVisibleItemSlot(slot,NULL);
                if ((slot < 10 && slot != 1 || slot == 14 || slot == 18) && HasAura(55119))
                {
                    Unit::AuraList const& Transforms = GetAurasByType(SPELL_AURA_TRANSFORM);
                    if (!Transforms.empty())
                    {
                        for (Unit::AuraList::const_iterator i = Transforms.begin();i != Transforms.end(); ++i)
                        {
                            if (CreatureInfo const * ci = ObjectMgr::GetCreatureTemplate((*i)->GetMiscValue()))
                            {                                           // Will use the default model here
                                if (uint32 modelid = ci->GetRandomValidModelId())
                                {
                                    if (GetTransmogManager()->IsRaceShirtModel(modelid))
                                    {
                                        CastSpell(this, 55118, true); // some model for unequip
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (pBag)
                pBag->RemoveItem(slot, update);
        }

        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);
        // pItem->SetUInt64Value(ITEM_FIELD_OWNER, 0); not clear owner at remove (it will be set at store). This used in mail and auction code
        pItem->SetSlot(NULL_SLOT);
        if (IsInWorld() && update)
            pItem->SendCreateUpdateToPlayer(this);
    }
}

/*void Player::RemoveItemByGuid(uint64 guid)
{
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetGUID() == guid)
        {
            RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
            return;
        }
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetGUID() == guid)
        {
            RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
            return;
        }
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetGUID() == guid)
                {
                    RemoveItem(i, j, true);
                    return;
                }
            }
        }
    }
    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetGUID() == guid)
                {
                    RemoveItem(i, j, true);
                    return;
                }
            }
        }
    }
}*/

// Common operation need to remove item from inventory without delete in trade, auction, guild bank, mail....
void Player::MoveItemFromInventory(uint8 bag, uint8 slot, bool update, std::string source)
{
    if (Item* it = GetItemByPos(bag,slot))
    {
        RemoveItem(bag,slot,update);
        it->RemoveFromUpdateQueueOf(this);
        if (it->IsInWorld())
        {
            it->RemoveFromWorld();
            it->DestroyForPlayer(this);
        }

		ItemRemovedCheck(it->GetEntry(), it->GetCount(), source.c_str());
    }
}

// Common operation need to add item from inventory without delete in trade, guild bank, mail....
void Player::MoveItemToInventory(ItemPosCountVec const& dest, Item* pItem, bool update, bool in_characterInventoryDB)
{
    // update quest counters
    ItemAddedQuestCheck(pItem->GetEntry(),pItem->GetCount());

    // store item
    Item* pLastItem = StoreItem(dest, pItem, update);

    // only set if not merged to existed stack (pItem can be deleted already but we can compare pointers any way)
    if (pLastItem==pItem)
    {
        // update owner for last item (this can be original item with wrong owner
        if (pLastItem->GetOwnerGUID() != GetGUID())
            pLastItem->SetOwnerGUID(GetGUID());

        // if this original item then it need create record in inventory
        // in case trade we already have item in other player inventory
        pLastItem->SetState(in_characterInventoryDB ? ITEM_CHANGED : ITEM_NEW, this);
    }
}

void Player::DestroyItem(uint8 bag, uint8 slot, bool update, std::string removed_from)
{
    Item *pItem = GetItemByPos(bag, slot);
    if (pItem)
    {
        sLog.outDebug("STORAGE: DestroyItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());

        // start from destroy contained items (only equipped bag can have its)
        if (pItem->IsBag() && IsBagPos(uint16(bag) << 8 | slot))          // this also prevent infinity loop if empty bag stored in bag==slot
        {
            if (!((Bag*)pItem)->IsEmpty())
                outStackTrace("ERROR-STACK: DestroyItemStack");

            // Fully clear the bag, even unavailable slots
            for (int i = 0; i < MAX_BAG_SIZE; i++)
                DestroyItem(slot,i,update);
        }

        if (pItem->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_WRAPPED))
            RealmDataDatabase.PExecute("DELETE FROM character_gifts WHERE item_guid = '%u'", pItem->GetGUIDLow());

        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        ItemRemovedCheck(pItem->GetEntry(), pItem->GetCount(), removed_from);

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot*2)), 0);

            // equipment and equipped bags can have applied bonuses
            if (slot < INVENTORY_SLOT_BAG_END)
            {
                ItemPrototype const *pProto = pItem->GetProto();

                // item set bonuses applied only at equip and removed at unequip, and still active for broken items
                if (pProto && pProto->ItemSet)
                    RemoveItemsSetItem(this,pProto);

                if (slot < 10 && slot != 1 && slot != 3)
                    CheckLevel5Reinforcement(true);

                _ApplyItemMods(pItem, slot, false);
            }

            if (slot < EQUIPMENT_SLOT_END)
            {
                // remove item dependent auras and casts (only weapon and armor slots)
                RemoveItemDependentAurasAndCasts(pItem);

                // update expertise
                if ( slot == EQUIPMENT_SLOT_MAINHAND )
                    UpdateExpertise(BASE_ATTACK, true);
                else if( slot == EQUIPMENT_SLOT_OFFHAND )
                    UpdateExpertise(OFF_ATTACK, true);

                // equipment visual show
                SetVisibleItemSlot(slot,NULL);
                if ((slot < 10 && slot != 1 || slot == 14 || slot == 18) && HasAura(55119))
                {
                    Unit::AuraList const& Transforms = GetAurasByType(SPELL_AURA_TRANSFORM);
                    if (!Transforms.empty())
                    {
                        for (Unit::AuraList::const_iterator i = Transforms.begin();i != Transforms.end(); ++i)
                        {
                            if (CreatureInfo const * ci = ObjectMgr::GetCreatureTemplate((*i)->GetMiscValue()))
                            {                                           // Will use the default model here
                                if (uint32 modelid = ci->GetRandomValidModelId())
                                {
                                    if (GetTransmogManager()->IsRaceShirtModel(modelid))
                                    {
                                        CastSpell(this, 55118, true); // some model for unequip
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            m_items[slot] = NULL;
        }
        else if (Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
            pBag->RemoveItem(slot, update);

        // Delete rolled money / loot from db.
        // MUST be done before RemoveFromWorld() or GetTemplate() fails
        if(pItem->GetProto()->Flags & ITEM_FLAGS_OPENABLE)
            pItem->ItemContainerDeleteLootMoneyAndLootItemsFromDB();

        if (IsInWorld() && update)
        {
            pItem->RemoveFromWorld();
            bool Destroy = true;
            if (ItemPrototype const *pProto = pItem->GetProto())
            {
                for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
                {
                    if (uint32 spellId = pProto->Spells[i].SpellId)
                        if (SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId))
                            if (spellInfo->HasEffect(SPELL_EFFECT_OPEN_LOCK) || spellInfo->HasEffect(SPELL_EFFECT_OPEN_LOCK_ITEM))
                            {
                                Destroy = false;
                                break;
                            }
                }
            }
            if (Destroy)
                pItem->DestroyForPlayer(this);
        }

        //pItem->SetOwnerGUID(0);
        pItem->SetUInt64Value(ITEM_FIELD_CONTAINED, 0);
        pItem->SetSlot(NULL_SLOT);
        pItem->SetState(ITEM_REMOVED, this);
    }
}

void Player::DestroyItemCount(uint32 item, uint32 count, bool update, bool unequip_check, std::string removed_from)
{
    sLog.outDebug("STORAGE: DestroyItemCount item = %u, count = %u", item, count);
    uint32 remcount = 0;

    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            if (pItem->GetCount() + remcount <= count)
            {
                // all items in inventory can unequipped
                remcount += pItem->GetCount();
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

                if (remcount >=count)
                    return;
            }
            else
            {
                ItemRemovedCheck(pItem->GetEntry(), count - remcount, removed_from);
                pItem->SetCount(pItem->GetCount() - count + remcount);
                if (IsInWorld() && update)
                    pItem->SendCreateUpdateToPlayer(this);
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            if (pItem->GetCount() + remcount <= count)
            {
                // all keys can be unequipped
                remcount += pItem->GetCount();
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

                if (remcount >=count)
                    return;
            }
            else
            {
                ItemRemovedCheck(pItem->GetEntry(), count - remcount, removed_from);
                pItem->SetCount(pItem->GetCount() - count + remcount);
                if (IsInWorld() && update)
                    pItem->SendCreateUpdateToPlayer(this);
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        if (Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item *pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetEntry() == item)
                {
                    // all items in bags can be unequipped
                    if (pItem->GetCount() + remcount <= count)
                    {
                        remcount += pItem->GetCount();
                        DestroyItem(i, j, update);

                        if (remcount >=count)
                            return;
                    }
                    else
                    {
                        ItemRemovedCheck(pItem->GetEntry(), count - remcount, removed_from);
                        pItem->SetCount(pItem->GetCount() - count + remcount);
                        if (IsInWorld() && update)
                            pItem->SendCreateUpdateToPlayer(this);
                        pItem->SetState(ITEM_CHANGED, this);
                        return;
                    }
                }
            }
        }
    }

    // in equipment and bag list
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            if (pItem->GetCount() + remcount <= count)
            {
                if (!unequip_check || CanUnequipItem(INVENTORY_SLOT_BAG_0 << 8 | i,false) == EQUIP_ERR_OK)
                {
                    remcount += pItem->GetCount();
                    DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

                    if (remcount >=count)
                        return;
                }
            }
            else
            {
                ItemRemovedCheck(pItem->GetEntry(), count - remcount, removed_from);
                pItem->SetCount(pItem->GetCount() - count + remcount);
                if (IsInWorld() && update)
                    pItem->SendCreateUpdateToPlayer(this);
                pItem->SetState(ITEM_CHANGED, this);
                return;
            }
        }
    }
}

void Player::DestroyZoneLimitedItem(bool update, uint32 new_zone)
{
    sLog.outDebug("STORAGE: DestroyZoneLimitedItem in map %u and area %u", GetMapId(), new_zone);

    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(),new_zone))
            DestroyItem(INVENTORY_SLOT_BAG_0, i, update, "ZONE_DESTROY");
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(),new_zone))
            DestroyItem(INVENTORY_SLOT_BAG_0, i, update, "ZONE_DESTROY");
    }

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(),new_zone))
                    DestroyItem(i, j, update, "ZONE_DESTROY");
            }
        }
    }

    // in equipment and bag list
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->IsLimitedToAnotherMapOrZone(GetMapId(),new_zone))
            DestroyItem(INVENTORY_SLOT_BAG_0, i, update, "ZONE_DESTROY");
    }
}

void Player::DestroyConjuredItems(bool update)
{
    // used when entering arena
    // destroys all conjured items
    sLog.outDebug("STORAGE: DestroyConjuredItems");

    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->IsConjuredConsumable())
            DestroyItem(INVENTORY_SLOT_BAG_0, i, update, "CONJURED_DESTROY");
    }

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item* pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->IsConjuredConsumable())
                    DestroyItem(i, j, update, "CONJURED_DESTROY");
            }
        }
    }

    // in equipment and bag list
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->IsConjuredConsumable())
            DestroyItem(INVENTORY_SLOT_BAG_0, i, update, "CONJURED_DESTROY");
    }
}

void Player::DestroyItemCount(Item* pItem, uint32 &count, bool update, std::string removed_from)
{
    if (!pItem)
        return;

    sLog.outDebug("STORAGE: DestroyItemCount item (GUID: %u, Entry: %u) count = %u", pItem->GetGUIDLow(),pItem->GetEntry(), count);

    if (pItem->GetCount() <= count)
    {
        count-= pItem->GetCount();

        DestroyItem(pItem->GetBagSlot(),pItem->GetSlot(), update);
    }
    else
    {
        ItemRemovedCheck(pItem->GetEntry(), count, removed_from);
        pItem->SetCount(pItem->GetCount() - count);
        count = 0;
        if (IsInWorld() && update)
            pItem->SendCreateUpdateToPlayer(this);
        pItem->SetState(ITEM_CHANGED, this);
    }
}

void Player::SplitItem(uint16 src, uint16 dst, uint32 count)
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item *pSrcItem = GetItemByPos(srcbag, srcslot);
    if (!pSrcItem)
    {
        SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pSrcItem, NULL);
        return;
    }

    // not let split all items (can be only at cheating)
    if (pSrcItem->GetCount() == count)
    {
        SendEquipError(EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL);
        return;
    }

    // not let split more existed items (can be only at cheating)
    if (pSrcItem->GetCount() < count)
    {
        SendEquipError(EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT, pSrcItem, NULL);
        return;
    }

    if (pSrcItem->m_lootGenerated)                           // prevent split looting item (item
    {
        //best error message found for attempting to split while looting
        SendEquipError(EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL);
        return;
    }

    sLog.outDebug("STORAGE: SplitItem bag = %u, slot = %u, item = %u, count = %u", dstbag, dstslot, pSrcItem->GetEntry(), count);
    Item *pNewItem = pSrcItem->CloneItem(count, this);
    if (!pNewItem)
    {
        SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pSrcItem, NULL);
        return;
    }

    if (IsInventoryPos(dst))
    {
        // change item amount before check (for unique max count check)
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        ItemPosCountVec dest;
        uint8 msg = CanStoreItem(dstbag, dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, NULL);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendCreateUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        StoreItem(dest, pNewItem, true);
    }
    else if (IsBankPos (dst))
    {
        // change item amount before check (for unique max count check)
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        ItemPosCountVec dest;
        uint8 msg = CanBankItem(dstbag, dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, NULL);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendCreateUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        BankItem(dest, pNewItem, true);
    }
    else if (IsEquipmentPos (dst))
    {
        // change item amount before check (for unique max count check), provide space for splitted items
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        uint16 dest;
        uint8 msg = CanEquipItem(dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, NULL);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendCreateUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        EquipItem(dest, pNewItem, true);
        AutoUnequipOffhandIfNeed();
    }
}

void Player::SwapItem(uint16 src, uint16 dst)
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item *pSrcItem = GetItemByPos(srcbag, srcslot);
    Item *pDstItem = GetItemByPos(dstbag, dstslot);

    if (!pSrcItem)
        return;

    sLog.outDebug("STORAGE: SwapItem bag = %u, slot = %u, item = %u", dstbag, dstslot, pSrcItem->GetEntry());

    if (!isAlive())
    {
        SendEquipError(EQUIP_ERR_YOU_ARE_DEAD, pSrcItem, pDstItem);
        return;
    }

    if (pSrcItem->m_lootGenerated && !(pSrcItem->GetProto()->Flags & ITEM_FLAGS_OPENABLE))                           // prevent swap looting item
    {
        //best error message found for attempting to swap while looting
        SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, pSrcItem, NULL);
        return;
    }

    // check unequip potability for equipped items and bank bags
    if (IsEquipmentPos (src) || IsBagPos (src))
    {
        // bags can be swapped with empty bag slots
        uint8 msg = CanUnequipItem(src, !IsBagPos (src) || IsBagPos (dst) || (pDstItem && pDstItem->IsBag() && ((Bag*)pDstItem)->IsEmpty()));
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, pSrcItem, pDstItem);
            return;
        }
    }

    // prevent put equipped/bank bag in self
    if (IsBagPos (src) && srcslot == dstbag)
    {
        SendEquipError(EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG, pSrcItem, pDstItem);
        return;
    }

    // prevent equipping bag in the same slot from its inside
    if (IsBagPos(dst) && srcbag == dstslot)
    {
        SendEquipError(EQUIP_ERR_ITEMS_CANT_BE_SWAPPED, pSrcItem, pDstItem);
        return;
    }

    // DST checks
    if (pDstItem)
    {
        if (pDstItem->m_lootGenerated && !(pDstItem->GetProto()->Flags & ITEM_FLAGS_OPENABLE))                       // prevent swap looting item
        {
            //best error message found for attempting to swap while looting
            SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, pDstItem, NULL);
            return;
        }

        // check unequip potability for equipped items and bank bags
        if (IsEquipmentPos (dst) || IsBagPos (dst))
        {
            // bags can be swapped with empty bag slots, or with empty bag (items move possibility checked later)
            uint8 msg = CanUnequipItem(dst, !IsBagPos (dst) || IsBagPos (src) || (pSrcItem->IsBag() && ((Bag*)pSrcItem)->IsEmpty()));
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, pDstItem);
                return;
            }
        }
    }

    // NOW this is or item move (swap with empty), or swap with another item (including bags in bag possitions)
    // or swap empty bag with another empty or not empty bag (with items exchange)

    // Move case
    if (!pDstItem)
    {
        if (IsInventoryPos(dst))
        {
            ItemPosCountVec dest;
            uint8 msg = CanStoreItem(dstbag, dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            if (dest.empty())
            {
                sLog.outLog(LOG_SPECIAL, "DEST empty in IsInventoryPos! Possible dupe!");
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            StoreItem(dest, pSrcItem, true);
        }
        else if (IsBankPos (dst))
        {
            ItemPosCountVec dest;
            uint8 msg = CanBankItem(dstbag, dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            if (dest.empty())
            {
                sLog.outLog(LOG_SPECIAL, "DEST empty in IsBankPos! Possible dupe!");
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            BankItem(dest, pSrcItem, true);
        }
        else if (IsEquipmentPos (dst))
        {
            uint16 dest;
            uint8 msg = CanEquipItem(dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            EquipItem(dest, pSrcItem, true);
            AutoUnequipOffhandIfNeed();
        }
        return;
    }

    // attempt merge to / fill target item
    if (!pSrcItem->IsBag() && !pDstItem->IsBag())
    {
        uint8 msg;
        ItemPosCountVec sDest;
        uint16 eDest = 0;
        if (IsInventoryPos(dst))
            msg = CanStoreItem(dstbag, dstslot, sDest, pSrcItem, false);
        else if (IsBankPos (dst))
            msg = CanBankItem(dstbag, dstslot, sDest, pSrcItem, false);
        else if (IsEquipmentPos (dst))
            msg = CanEquipItem(dstslot, eDest, pSrcItem, false);
        else
            return;

        // can be merge/fill
        if (msg == EQUIP_ERR_OK)
        {
            if (pSrcItem->GetCount() + pDstItem->GetCount() <= pSrcItem->GetProto()->Stackable)
            {
                RemoveItem(srcbag, srcslot, true);

                if (IsInventoryPos(dst))
                    StoreItem(sDest, pSrcItem, true);
                else if (IsBankPos (dst))
                    BankItem(sDest, pSrcItem, true);
                else if (IsEquipmentPos (dst))
                {
                    EquipItem(eDest, pSrcItem, true);
                    AutoUnequipOffhandIfNeed();
                }
            }
            else
            {
                pSrcItem->SetCount(pSrcItem->GetCount() + pDstItem->GetCount() - pSrcItem->GetProto()->Stackable);
                pDstItem->SetCount(pSrcItem->GetProto()->Stackable);
                pSrcItem->SetState(ITEM_CHANGED, this);
                pDstItem->SetState(ITEM_CHANGED, this);
                if (IsInWorld())
                {
                    pSrcItem->SendCreateUpdateToPlayer(this);
                    pDstItem->SendCreateUpdateToPlayer(this);
                }
            }
            return;
        }
    }

    // impossible merge/fill, do real swap
    uint8 msg = EQUIP_ERR_OK;

    // check src->dest move possibility
    ItemPosCountVec sDest;
    uint16 eDest = 0;
    if (IsInventoryPos(dst))
        msg = CanStoreItem(dstbag, dstslot, sDest, pSrcItem, true);
    else if (IsBankPos(dst))
        msg = CanBankItem(dstbag, dstslot, sDest, pSrcItem, true);
    else if (IsEquipmentPos(dst))
    {
        msg = CanEquipItem(dstslot, eDest, pSrcItem, true);
        if (msg == EQUIP_ERR_OK)
            msg = CanUnequipItem(eDest, true);
    }

    if (msg != EQUIP_ERR_OK)
    {
        SendEquipError(msg, pSrcItem, pDstItem);
        return;
    }

    // check dest->src move possibility
    ItemPosCountVec sDest2;
    uint16 eDest2 = 0;
    if (IsInventoryPos(src))
        msg = CanStoreItem(srcbag, srcslot, sDest2, pDstItem, true);
    else if (IsBankPos(src))
        msg = CanBankItem(srcbag, srcslot, sDest2, pDstItem, true);
    else if (IsEquipmentPos(src))
    {
        msg = CanEquipItem(srcslot, eDest2, pDstItem, true);
        if (msg == EQUIP_ERR_OK)
            msg = CanUnequipItem(eDest2, true);
    }

    if (msg != EQUIP_ERR_OK)
    {
        SendEquipError(msg, pDstItem, pSrcItem);
        return;
    }

    // Check bag swap with item exchange (one from empty in not bag possition (equipped (not possible in fact) or store)
    if (pSrcItem->IsBag() && pDstItem->IsBag())
    {
        Bag* emptyBag = NULL;
        Bag* fullBag = NULL;
        if (((Bag*)pSrcItem)->IsEmpty() && !IsBagPos(src))
        {
            emptyBag = (Bag*)pSrcItem;
            fullBag  = (Bag*)pDstItem;
        }
        else if (((Bag*)pDstItem)->IsEmpty() && !IsBagPos(dst))
        {
            emptyBag = (Bag*)pDstItem;
            fullBag  = (Bag*)pSrcItem;
        }

        // bag swap (with items exchange) case
        if (emptyBag && fullBag)
        {
            ItemPrototype const* emptyProto = emptyBag->GetProto();

            uint32 count = 0;

            for (uint32 i=0; i < fullBag->GetBagSize(); ++i)
            {
                Item *bagItem = fullBag->GetItemByPos(i);
                if (!bagItem)
                    continue;

                ItemPrototype const* bagItemProto = bagItem->GetProto();
                if (!bagItemProto || !ItemCanGoIntoBag(bagItemProto, emptyProto))
                {
                    // one from items not go to empty target bag
                    SendEquipError(EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG, pSrcItem, pDstItem);
                    return;
                }

                ++count;
            }

            if (count > emptyBag->GetBagSize())
            {
                // too small targeted bag
                SendEquipError(EQUIP_ERR_ITEMS_CANT_BE_SWAPPED, pSrcItem, pDstItem);
                return;
            }

            // Items swap
            count = 0;                                      // will pos in new bag
            for (uint32 i = 0; i< fullBag->GetBagSize(); ++i)
            {
                Item *bagItem = fullBag->GetItemByPos(i);
                if (!bagItem)
                    continue;

                fullBag->RemoveItem(i, true);
                emptyBag->StoreItem(count, bagItem, true);
                bagItem->SetState(ITEM_CHANGED, this);

                ++count;
            }
        }
    }

    // now do moves, remove...
    RemoveItem(dstbag, dstslot, false);
    RemoveItem(srcbag, srcslot, false);

    // add to dest
    if (IsInventoryPos(dst))
        StoreItem(sDest, pSrcItem, true);
    else if (IsBankPos(dst))
        BankItem(sDest, pSrcItem, true);
    else if (IsEquipmentPos(dst))
        EquipItem(eDest, pSrcItem, true);

    // add to src
    if (IsInventoryPos(src))
        StoreItem(sDest2, pDstItem, true);
    else if (IsBankPos(src))
        BankItem(sDest2, pDstItem, true);
    else if (IsEquipmentPos(src))
        EquipItem(eDest2, pDstItem, true);

    // if player is moving bags and is looting an item inside this bag
    // release the loot
    if (GetLootGUID())
    {
        bool released = false;
        if (IsBagPos(src))
        {
            Bag* bag = (Bag*)pSrcItem;
            for (int i=0; i < bag->GetBagSize(); ++i)
            {
                if (Item *bagItem = bag->GetItemByPos(i))
                {
                    if (bagItem->m_lootGenerated)
                    {
                        m_session->DoLootRelease(GetLootGUID());
                        released = true;                    // so we don't need to look at dstBag
                        break;
                    }
                }
            }
        }

        if (!released && IsBagPos(dst) && pDstItem)
        {
            Bag* bag = (Bag*)pDstItem;
            for (int i=0; i < bag->GetBagSize(); ++i)
            {
                if (Item *bagItem = bag->GetItemByPos(i))
                {
                    if (bagItem->m_lootGenerated)
                    {
                        m_session->DoLootRelease(GetLootGUID());
                        released = true;                    // not realy needed here
                        break;
                    }
                }
            }
        }
    }

    AutoUnequipOffhandIfNeed();
}

void Player::AddItemToBuyBackSlot(Item *pItem)
{
    if (pItem)
    {
        uint32 slot = m_currentBuybackSlot;
        // if current back slot non-empty search oldest or free
        if (m_items[slot])
        {
            uint32 oldest_time = GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1);
            uint32 oldest_slot = BUYBACK_SLOT_START;

            for (uint32 i = BUYBACK_SLOT_START+1; i < BUYBACK_SLOT_END; ++i)
            {
                // found empty
                if (!m_items[i])
                {
                    slot = i;
                    break;
                }

                uint32 i_time = GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + i - BUYBACK_SLOT_START);

                if (oldest_time > i_time)
                {
                    oldest_time = i_time;
                    oldest_slot = i;
                }
            }

            // find oldest
            slot = oldest_slot;
        }

        RemoveItemFromBuyBackSlot(slot, true);
        sLog.outDebug("STORAGE: AddItemToBuyBackSlot item = %u, slot = %u", pItem->GetEntry(), slot);

        m_items[slot] = pItem;
        time_t base = time(NULL);
        uint32 etime = uint32(base - m_logintime + (30 * 3600));
        uint32 eslot = slot - BUYBACK_SLOT_START;

        SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, pItem->GetGUID());
        ItemPrototype const *pProto = pItem->GetProto();
        if (pProto)
            SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, pProto->SellPrice * pItem->GetCount());
        else
            SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, (uint32)etime);

        // move to next (for non filled list is move most optimized choice)
        if (m_currentBuybackSlot < BUYBACK_SLOT_END-1)
            ++m_currentBuybackSlot;
    }
}

Item* Player::GetItemFromBuyBackSlot(uint32 slot)
{
    sLog.outDebug("STORAGE: GetItemFromBuyBackSlot slot = %u", slot);
    if (slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END)
        return m_items[slot];
    return NULL;
}

void Player::RemoveItemFromBuyBackSlot(uint32 slot, bool del)
{
    sLog.outDebug("STORAGE: RemoveItemFromBuyBackSlot slot = %u", slot);
    if (slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END)
    {
        Item *pItem = m_items[slot];
        if (pItem)
        {
            pItem->RemoveFromWorld();
            if (del)
            {
                pItem->SetState(ITEM_REMOVED, this);

                /*if(pItem->GetProto()->Flags & ITEM_FLAGS_OPENABLE)
                    pItem->ItemContainerDeleteLootMoneyAndLootItemsFromDB();*/
            }
        }

        m_items[slot] = NULL;

        uint32 eslot = slot - BUYBACK_SLOT_START;
        SetUInt64Value(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + eslot * 2, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, 0);

        // if current backslot is filled set to now free slot
        if (m_items[m_currentBuybackSlot])
            m_currentBuybackSlot = slot;
    }
}

void Player::SendEquipError(uint8 msg, Item* pItem, Item *pItem2) const
{
    sLog.outDebug("WORLD: Sent SMSG_INVENTORY_CHANGE_FAILURE (%u)",msg);
    WorldPacket data(SMSG_INVENTORY_CHANGE_FAILURE, (msg == EQUIP_ERR_CANT_EQUIP_LEVEL_I ? 22 : 18));
    data << uint8(msg);

    if (msg)
    {
        data << uint64(pItem ? pItem->GetGUID() : 0);
        data << uint64(pItem2 ? pItem2->GetGUID() : 0);
        data << uint8(0);                                   // not 0 there...

        if (msg == EQUIP_ERR_CANT_EQUIP_LEVEL_I)
        {
            uint32 level = 0;

            if (pItem)
                if (ItemPrototype const* proto =  pItem->GetProto())
                    level = proto->RequiredLevel;

            data << uint32(level);                          // new 2.4.0
        }
    }
    SendPacketToSelf(&data);
}

void Player::SendBuyError(uint8 msg, Creature* pCreature, uint32 item, uint32 param)
{
    sLog.outDebug("WORLD: Sent SMSG_BUY_FAILED");
    WorldPacket data(SMSG_BUY_FAILED, (8+4+4+1));
    data << uint64(pCreature ? pCreature->GetGUID() : 0);
    data << uint32(item);
    if (param > 0)
        data << uint32(param);
    data << uint8(msg);
    SendPacketToSelf(&data);
}

void Player::SendSellError(uint8 msg, Creature* pCreature, uint64 guid, uint32 param)
{
    sLog.outDebug("WORLD: Sent SMSG_SELL_ITEM");
    WorldPacket data(SMSG_SELL_ITEM,(8+8+(param?4:0)+1));  // last check 2.0.10
    data << uint64(pCreature ? pCreature->GetGUID() : 0);
    data << uint64(guid);
    if (param > 0) // never used
        data << uint32(param);
    data << uint8(msg);
    SendPacketToSelf(&data);
}

void Player::ClearTrade()
{
    tradeGold = 0;
    pTrader_lastmove = 0;
    acceptTrade = false;
    for (int i = 0; i < TRADE_SLOT_COUNT; i++)
        tradeItems[i] = NULL_SLOT;
}

void Player::TradeCancel(bool sendback)
{
    if (pTrader)
    {
        // send yellow "Trade canceled" message to both traders
        WorldSession* ws;
        ws = GetSession();
        if (sendback)
            ws->SendCancelTrade();
        ws = pTrader->GetSession();
        if (!ws->PlayerLogout())
            ws->SendCancelTrade();

        // cleanup
        ClearTrade();
        pTrader->ClearTrade();
        // prevent loss of reference
        pTrader->pTrader = NULL;
        pTrader = NULL;
    }
}

void Player::UpdateItemDuration(uint32 time, bool realtimeonly)
{
    if (m_itemDuration.empty())
        return;

    sLog.outDebug("Player::UpdateItemDuration(%u,%u)", time,realtimeonly);

    for (ItemDurationList::iterator itr = m_itemDuration.begin();itr != m_itemDuration.end();)
    {
        Item* item = *itr;
        ++itr;                                              // current element can be erased in UpdateDuration

        if (realtimeonly && item->GetProto()->Duration < 0 || !realtimeonly)
            item->UpdateDuration(this,time);
    }
}

void Player::UpdateEnchantTime(uint32 time)
{
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin(),next;itr != m_enchantDuration.end();itr=next)
    {
        ASSERT(itr->item);
        next=itr;
        if (!itr->item->GetEnchantmentId(itr->slot))
        {
            next = m_enchantDuration.erase(itr);
        }
        else if (itr->leftduration <= time)
        {
            bool DoDestroy = itr->leftduration != 1; // permanent temp enchants :D
            if (!DoDestroy)
                if (ItemPrototype const *ItemProto = itr->item->GetProto())
                {
                    if (ItemProto->Quality == ITEM_QUALITY_LEGENDARY || ItemProto->Quality == ITEM_QUALITY_ARTIFACT)
                    {
                        switch (ItemProto->InventoryType)
                        {
                            case INVTYPE_HEAD:
                            case INVTYPE_SHOULDERS:
                            case INVTYPE_CHEST:
                            case INVTYPE_WAIST:
                            case INVTYPE_LEGS:
                            case INVTYPE_FEET:
                            case INVTYPE_WRISTS:
                            case INVTYPE_HANDS:
                            case INVTYPE_ROBE:
                                break;
                            default:
                            {
                                DoDestroy = true;
                                break;
                            }
                        }
                    }
                    else
                        DoDestroy = true;
                }
            if (DoDestroy)
            {
                ApplyEnchantment(itr->item,itr->slot,false,false);
                itr->item->ClearEnchantment(itr->slot);
            }
            next = m_enchantDuration.erase(itr); 
        }
        else if (itr->leftduration > time)
        {
            itr->leftduration -= time;
            ++next;
        }
    }
}

void Player::AddEnchantmentDurations(Item *item)
{
    ASSERT(item);

    for (int x=0;x<MAX_ENCHANTMENT_SLOT;++x)
    {
        if (!item->GetEnchantmentId(EnchantmentSlot(x)))
            continue;

        uint32 duration = item->GetEnchantmentDuration(EnchantmentSlot(x));
        if (duration > 0)
            AddEnchantmentDuration(item,EnchantmentSlot(x),duration);
    }
}

void Player::RemoveEnchantmentDurations(Item *item)
{
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin();itr != m_enchantDuration.end();)
    {
        if (itr->item == item)
        {
            // save duration in item
            item->SetEnchantmentDuration(EnchantmentSlot(itr->slot),itr->leftduration);
            itr = m_enchantDuration.erase(itr);
        }
        else
            ++itr;
    }
}

void Player::RemoveAllEnchantments()
{
    // remove enchantments from equipped items first to clean up the m_enchantDuration list
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin(),next;itr != m_enchantDuration.end();itr=next)
    {
        next = itr;
        if (itr->slot == TEMP_ENCHANTMENT_SLOT)
        {
            if (itr->item && itr->item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
            {
                uint32 enchant_id = itr->item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT);
                if (enchant_id)
                {
                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if (pEnchant && pEnchant->aura_id == ITEM_ENCHANTMENT_AURAID_POISON || itr->leftduration == 1) // for perm temp enchants, though it is removed from list, check anyway for sure
                    {
                        ++next;
                        continue;
                    }
                }
                // remove from stats
                ApplyEnchantment(itr->item,TEMP_ENCHANTMENT_SLOT,false,false);
                // remove visual
                itr->item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
            }
            // remove from update list
            next = m_enchantDuration.erase(itr);
        }
        else
            ++next;
    }

    // remove enchants from inventory items
    // NOTE: no need to remove these from stats, since these aren't equipped
    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
    {
        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
        {
            SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT));
            if (pEnchant && pEnchant->aura_id == ITEM_ENCHANTMENT_AURAID_POISON || pItem->GetEnchantmentDuration(TEMP_ENCHANTMENT_SLOT) == 1) // for perm temp enchants
                continue;
            pItem->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
        }
    }

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
    {
        Bag *pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); j++)
            {
                Item *pItem = pBag->GetItemByPos(j);
                if (pItem && pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                {
                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT));
                    if (pEnchant && pEnchant->aura_id == ITEM_ENCHANTMENT_AURAID_POISON || pItem->GetEnchantmentDuration(TEMP_ENCHANTMENT_SLOT) == 1) // perm temp enchant
                        continue;
                    pItem->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
                }
            }
        }
    }
}

// duration == 0 will remove item enchant
void Player::AddEnchantmentDuration(Item *item,EnchantmentSlot slot,uint32 duration)
{
    if (!item)
        return;

    if (slot >= MAX_ENCHANTMENT_SLOT)
        return;

    for (EnchantDurationList::iterator itr = m_enchantDuration.begin();itr != m_enchantDuration.end();++itr)
    {
        if (itr->item == item && itr->slot == slot)
        {
            itr->item->SetEnchantmentDuration(itr->slot,itr->leftduration);
            m_enchantDuration.erase(itr);
            break;
        }
    }
    if (item && duration > 0)
    {
        GetSession()->SendItemEnchantTimeUpdate(GetGUID(), item->GetGUID(),slot,uint32(duration/1000));
        m_enchantDuration.push_back(EnchantDuration(item,slot,duration));
    }
}

void Player::ApplyEnchantment(Item *item,bool apply)
{
    for (uint32 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
        ApplyEnchantment(item, EnchantmentSlot(slot), apply);
}

void Player::ApplyEnchantment(Item *item,EnchantmentSlot slot,bool apply, bool apply_dur, bool ignore_condition)
{
    if (!item)
        return;

    if (!item->IsEquipped())
        return;

    if (slot >= MAX_ENCHANTMENT_SLOT)
        return;

    uint32 enchant_id = item->GetEnchantmentId(slot);
    if (!enchant_id)
        return;

    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if (!pEnchant)
        return;

    if (!ignore_condition && pEnchant->EnchantmentCondition && !((Player*)this)->EnchantmentFitsRequirements(pEnchant->EnchantmentCondition, -1))
        return;

    if (!item->IsBroken() || !apply)
    {
        for (int s=0; s<3; s++)
        {
            uint32 enchant_display_type = pEnchant->type[s];
            uint32 enchant_amount = pEnchant->amount[s];
            uint32 enchant_spell_id = pEnchant->spellid[s];

            switch (enchant_display_type)
            {
            case ITEM_ENCHANTMENT_TYPE_NONE:
                break;
            case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
                // processed in Player::CastItemCombatSpell
                break;
            case ITEM_ENCHANTMENT_TYPE_DAMAGE:
                if (item->GetSlot() == EQUIPMENT_SLOT_MAINHAND)
                    HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(enchant_amount), apply);
                else if (item->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                    HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(enchant_amount), apply);
                else if (item->GetSlot() == EQUIPMENT_SLOT_RANGED)
                    HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                break;
            case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
                if (enchant_spell_id)
                {
                    if (apply)
                    {
                        int32 basepoints = 0;
                        // Random Property Exist - try found basepoints for spell (basepoints depends from item suffix factor)
                        if (item->GetItemRandomPropertyId())
                        {
                            ItemRandomSuffixEntry const *item_rand = sItemRandomSuffixStore.LookupEntry(abs(item->GetItemRandomPropertyId()));
                            if (item_rand)
                            {
                                // Search enchant_amount
                                for (int k=0; k<3; k++)
                                {
                                    if (item_rand->enchant_id[k] == enchant_id)
                                    {
                                        basepoints = int32((item_rand->prefix[k]*item->GetItemSuffixFactor()) / 10000);
                                        break;
                                    }
                                }
                            }
                        }
                        // Cast custom spell vs all equal basepoints getted from enchant_amount
                        if (basepoints)
                            CastCustomSpell(this,enchant_spell_id,&basepoints,&basepoints,&basepoints,true,item);
                        else
                            CastSpell(this,enchant_spell_id,true,item);
                    }
                    else
                        RemoveAurasDueToItemSpell(item,enchant_spell_id);
                }
                break;
            case ITEM_ENCHANTMENT_TYPE_RESISTANCE:
                if (!enchant_amount)
                {
                    ItemRandomSuffixEntry const *item_rand = sItemRandomSuffixStore.LookupEntry(abs(item->GetItemRandomPropertyId()));
                    if (item_rand)
                    {
                        for (int k=0; k<3; k++)
                        {
                            if (item_rand->enchant_id[k] == enchant_id)
                            {
                                enchant_amount = uint32((item_rand->prefix[k]*item->GetItemSuffixFactor()) / 10000);
                                break;
                            }
                        }
                    }
                }

                HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + enchant_spell_id), TOTAL_VALUE, float(enchant_amount), apply);
                break;
            case ITEM_ENCHANTMENT_TYPE_STAT:
                {
                    if (!enchant_amount)
                    {
                        ItemRandomSuffixEntry const *item_rand_suffix = sItemRandomSuffixStore.LookupEntry(abs(item->GetItemRandomPropertyId()));
                        if (item_rand_suffix)
                        {
                            for (int k=0; k<3; k++)
                            {
                                if (item_rand_suffix->enchant_id[k] == enchant_id)
                                {
                                    enchant_amount = uint32((item_rand_suffix->prefix[k]*item->GetItemSuffixFactor()) / 10000);
                                    break;
                                }
                            }
                        }
                    }

                    sLog.outDebug("Adding %u to stat nb %u",enchant_amount,enchant_spell_id);
                    switch (enchant_spell_id)
                    {
                    case ITEM_MOD_AGILITY:
                        sLog.outDebug("+ %u AGILITY",enchant_amount);
                        HandleStatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, float(enchant_amount), apply);
                        ApplyStatBuffMod(STAT_AGILITY, enchant_amount, apply);
                        break;
                    case ITEM_MOD_STRENGTH:
                        sLog.outDebug("+ %u STRENGTH",enchant_amount);
                        HandleStatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, float(enchant_amount), apply);
                        ApplyStatBuffMod(STAT_STRENGTH, enchant_amount, apply);
                        break;
                    case ITEM_MOD_INTELLECT:
                        sLog.outDebug("+ %u INTELLECT",enchant_amount);
                        HandleStatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, float(enchant_amount), apply);
                        ApplyStatBuffMod(STAT_INTELLECT, enchant_amount, apply);
                        break;
                    case ITEM_MOD_SPIRIT:
                        sLog.outDebug("+ %u SPIRIT",enchant_amount);
                        HandleStatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, float(enchant_amount), apply);
                        ApplyStatBuffMod(STAT_SPIRIT, enchant_amount, apply);
                        break;
                    case ITEM_MOD_STAMINA:
                        sLog.outDebug("+ %u STAMINA",enchant_amount);
                        HandleStatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, float(enchant_amount), apply);
                        ApplyStatBuffMod(STAT_STAMINA, enchant_amount, apply);
                        break;
                    case ITEM_MOD_DEFENSE_SKILL_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_DEFENSE_SKILL, enchant_amount, apply);
                        sLog.outDebug("+ %u DEFENCE", enchant_amount);
                        break;
                    case  ITEM_MOD_DODGE_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_DODGE, enchant_amount, apply);
                        sLog.outDebug("+ %u DODGE", enchant_amount);
                        break;
                    case ITEM_MOD_PARRY_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_PARRY, enchant_amount, apply);
                        sLog.outDebug("+ %u PARRY", enchant_amount);
                        break;
                    case ITEM_MOD_BLOCK_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_BLOCK, enchant_amount, apply);
                        sLog.outDebug("+ %u SHIELD_BLOCK", enchant_amount);
                        break;
                    case ITEM_MOD_HIT_MELEE_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_HIT_MELEE, enchant_amount, apply);
                        sLog.outDebug("+ %u MELEE_HIT", enchant_amount);
                        break;
                    case ITEM_MOD_HIT_RANGED_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_HIT_RANGED, enchant_amount, apply);
                        sLog.outDebug("+ %u RANGED_HIT", enchant_amount);
                        break;
                    case ITEM_MOD_HIT_SPELL_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_HIT_SPELL, enchant_amount, apply);
                        sLog.outDebug("+ %u SPELL_HIT", enchant_amount);
                        break;
                    case ITEM_MOD_CRIT_MELEE_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                        sLog.outDebug("+ %u MELEE_CRIT", enchant_amount);
                        break;
                    case ITEM_MOD_CRIT_RANGED_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                        sLog.outDebug("+ %u RANGED_CRIT", enchant_amount);
                        break;
                    case ITEM_MOD_CRIT_SPELL_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                        sLog.outDebug("+ %u SPELL_CRIT", enchant_amount);
                        break;
                        //                    Values from ITEM_STAT_MELEE_HA_RATING to ITEM_MOD_HASTE_RANGED_RATING are never used
                        //                    in Enchantments
                        //                    case ITEM_MOD_HIT_TAKEN_MELEE_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_HIT_TAKEN_MELEE, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_HIT_TAKEN_RANGED_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_HIT_TAKEN_RANGED, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_HIT_TAKEN_SPELL_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_HIT_TAKEN_SPELL, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_CRIT_TAKEN_MELEE_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_CRIT_TAKEN_RANGED_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_CRIT_TAKEN_SPELL_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_HASTE_MELEE_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_HASTE_MELEE, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_HASTE_RANGED_RATING:
                        //                        ((Player*)this)->ApplyRatingMod(CR_HASTE_RANGED, enchant_amount, apply);
                        //                        break;
                    case ITEM_MOD_HASTE_SPELL_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_HASTE_SPELL, enchant_amount, apply);
                        break;
                    case ITEM_MOD_HIT_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_HIT_MELEE, enchant_amount, apply);
                        ((Player*)this)->ApplyRatingMod(CR_HIT_RANGED, enchant_amount, apply);
                        //((Player*)this)->ApplyRatingMod(CR_HIT_SPELL, enchant_amount, apply);
                        sLog.outDebug("+ %u HIT", enchant_amount);
                        break;
                    case ITEM_MOD_CRIT_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_MELEE, enchant_amount, apply);
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_RANGED, enchant_amount, apply);
                        //((Player*)this)->ApplyRatingMod(CR_CRIT_SPELL, enchant_amount, apply);
                        sLog.outDebug("+ %u CRITICAL", enchant_amount);
                        break;
                        //                    Values ITEM_MOD_HIT_TAKEN_RATING and ITEM_MOD_CRIT_TAKEN_RATING are never used in Enchantment
                        //                    case ITEM_MOD_HIT_TAKEN_RATING:
                        //                          ((Player*)this)->ApplyRatingMod(CR_HIT_TAKEN_MELEE, enchant_amount, apply);
                        //                          ((Player*)this)->ApplyRatingMod(CR_HIT_TAKEN_RANGED, enchant_amount, apply);
                        //                          ((Player*)this)->ApplyRatingMod(CR_HIT_TAKEN_SPELL, enchant_amount, apply);
                        //                        break;
                        //                    case ITEM_MOD_CRIT_TAKEN_RATING:
                        //                          ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                        //                          ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                        //                          ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                        //                        break;
                    case ITEM_MOD_RESILIENCE_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_MELEE, enchant_amount, apply);
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_RANGED, enchant_amount, apply);
                        ((Player*)this)->ApplyRatingMod(CR_CRIT_TAKEN_SPELL, enchant_amount, apply);
                        sLog.outDebug("+ %u RESILIENCE", enchant_amount);
                        break;
                    case ITEM_MOD_HASTE_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_HASTE_MELEE, enchant_amount, apply);
                        ((Player*)this)->ApplyRatingMod(CR_HASTE_RANGED, enchant_amount, apply);
                        ((Player*)this)->ApplyRatingMod(CR_HASTE_SPELL, enchant_amount, apply);
                        sLog.outDebug("+ %u HASTE", enchant_amount);
                        break;
                    case ITEM_MOD_EXPERTISE_RATING:
                        ((Player*)this)->ApplyRatingMod(CR_EXPERTISE, enchant_amount, apply);
                        sLog.outDebug("+ %u EXPERTISE", enchant_amount);
                        break;
                    default:
                        break;
                    }
                    break;
                }
            case ITEM_ENCHANTMENT_TYPE_TOTEM:               // Shaman Rockbiter Weapon
                {
                    if (GetClass() == CLASS_SHAMAN)
                    {
                        float addValue = 0.0f;
                        if (item->GetSlot() == EQUIPMENT_SLOT_MAINHAND)
                        {
                            addValue = float(enchant_amount * item->GetProto()->Delay/1000.0f);
                            HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, addValue, apply);
                        }
                        else if (item->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                        {
                            addValue = float(enchant_amount * item->GetProto()->Delay/1000.0f);
                            HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, addValue, apply);
                        }
                    }
                    break;
                }
            default:
                sLog.outLog(LOG_DEFAULT, "ERROR: Unknown item enchantment display type: %d",enchant_display_type);
                break;
            }                                                   /*switch (enchant_display_type)*/
        }                                                       /*for*/
    }

    // visualize enchantment at player and equipped items
    if (slot < MAX_INSPECTED_ENCHANTMENT_SLOT)
    {
        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (item->GetSlot() * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(VisibleBase + 1 + slot, apply? item->GetEnchantmentId(slot) : 0);
    }

    if (apply_dur)
    {
        if (apply)
        {
            // set duration
            uint32 duration = item->GetEnchantmentDuration(slot);
            if (duration > 0)
                AddEnchantmentDuration(item,slot,duration);
        }
        else
        {
            // duration == 0 will remove EnchantDuration
            AddEnchantmentDuration(item,slot,0);
        }
    }
}

void Player::SendEnchantmentDurations()
{
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin();itr != m_enchantDuration.end();++itr)
    {
        GetSession()->SendItemEnchantTimeUpdate(GetGUID(), itr->item->GetGUID(),itr->slot,uint32(itr->leftduration)/1000);
    }
}

void Player::SendItemDurations()
{
    for (ItemDurationList::iterator itr = m_itemDuration.begin();itr != m_itemDuration.end();++itr)
    {
        (*itr)->SendTimeUpdate(this);
    }
}

void Player::SendNewItem(Item *item, uint32 count, bool received, bool created, bool broadcast /*=false*/, bool showInChat /*=true*/)
{
    if (!item)                                               // prevent crash
        return;

                                                            // last check 2.0.10
    WorldPacket data(SMSG_ITEM_PUSH_RESULT, (8+4+4+4+1+4+4+4+4+4));
    data << GetGUID();                                      // player GUID
    data << uint32(received);                               // 0=looted, 1=from npc
    data << uint32(created);                                // 0=received, 1=created
    data << uint32(showInChat);                             // showInChat
    data << item->GetBagSlot();                             // bagslot
                                                            // item slot, but when added to stack: 0xFFFFFFFF
    data << (uint32) ((item->GetCount()==count) ? item->GetSlot() : -1);
    data << uint32(item->GetEntry());                       // item id
    data << uint32(item->GetItemSuffixFactor());            // SuffixFactor
    data << uint32(item->GetItemRandomPropertyId());        // random item property id
    data << uint32(count);                                  // count of items
    data << GetItemCount(item->GetEntry());                 // count of items in inventory

    if (broadcast && GetGroup())
        GetGroup()->BroadcastPacket(&data, false);
    else
        SendPacketToSelf(&data);
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

void Player::PrepareQuestMenu(uint64 guid)
{
    Object *pObject;
    QuestRelations* pObjectQR;
    QuestRelations* pObjectQIR;
    Creature *pCreature = GetMap()->GetCreatureOrPet(guid);
    if (pCreature)
    {
        pObject = (Object*)pCreature;
        pObjectQR  = &sObjectMgr.mCreatureQuestRelations;
        pObjectQIR = &sObjectMgr.mCreatureQuestInvolvedRelations;
    }
    else
    {
        //we should obtain map pointer from GetMap() in 99% of cases. Special case
        //only for quests which cast teleport spells on player
        Map * _map = IsInWorld() ? GetMap() : sMapMgr.FindMap(GetMapId(), GetAnyInstanceId());
        ASSERT(_map);
        GameObject *pGameObject = _map->GetGameObject(guid);
        if (pGameObject)
        {
            pObject = (Object*)pGameObject;
            pObjectQR  = &sObjectMgr.mGOQuestRelations;
            pObjectQIR = &sObjectMgr.mGOQuestInvolvedRelations;
        }
        else
            return;
    }

    QuestMenu &qm = PlayerTalkClass->GetQuestMenu();
    qm.ClearMenu();

    for (QuestRelations::const_iterator i = pObjectQIR->lower_bound(pObject->GetEntry()); i != pObjectQIR->upper_bound(pObject->GetEntry()); ++i)
    {
        uint32 quest_id = i->second;
        QuestStatus status = GetQuestStatus(quest_id);
        if (status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(quest_id))
            qm.AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if (status == QUEST_STATUS_INCOMPLETE)
            qm.AddMenuItem(quest_id, DIALOG_STATUS_INCOMPLETE);
        else if (status == QUEST_STATUS_AVAILABLE)
            qm.AddMenuItem(quest_id, DIALOG_STATUS_CHAT);
    }

    for (QuestRelations::const_iterator i = pObjectQR->lower_bound(pObject->GetEntry()); i != pObjectQR->upper_bound(pObject->GetEntry()); ++i)
    {
        uint32 quest_id = i->second;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);
        if (!pQuest) continue;

        QuestStatus status = GetQuestStatus(quest_id);

        if (pQuest->IsAutoComplete() && CanTakeQuest(pQuest, false))
            qm.AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if (status == QUEST_STATUS_NONE && CanTakeQuest(pQuest, false))
            qm.AddMenuItem(quest_id, DIALOG_STATUS_AVAILABLE);
    }
}

void Player::SendPreparedQuest(uint64 guid)
{
    QuestMenu& questMenu = PlayerTalkClass->GetQuestMenu();
    if (questMenu.Empty())
        return;

    QuestMenuItem const& qmi0 = questMenu.GetItem(0);

    uint32 status = qmi0.m_qIcon;

    // single element case
    if (questMenu.MenuItemCount() == 1)
    {
        // Auto open -- maybe also should verify there is no greeting
        uint32 quest_id = qmi0.m_qId;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);
        if (pQuest)
        {
            if (status == DIALOG_STATUS_REWARD_REP && !GetQuestRewardStatus(quest_id))
                PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, CanRewardQuest(pQuest,false), true);
            else if (status == DIALOG_STATUS_INCOMPLETE)
                PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, false, true);
            // Send completable on repeatable quest if player don't have quest
            else if (pQuest->IsRepeatable() && !pQuest->IsDaily())
                PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, CanCompleteRepeatableQuest(pQuest), true);
            else
                PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, guid, true);
        }
    }
    // multiply entries
    else
    {
        QEmote qe;
        qe._Delay = 0;
        qe._Emote = 0;
        std::string title = "";
        Creature *pCreature = GetMap()->GetCreature(guid);
        if (pCreature)
        {
            uint32 textid = pCreature->GetNpcTextId();
            GossipText const * gossiptext = sObjectMgr.GetGossipText(textid);
            if (!gossiptext)
            {
                qe._Delay = 0;                              //TEXTEMOTE_MESSAGE;              //zyg: player emote
                qe._Emote = 0;                              //TEXTEMOTE_HELLO;                //zyg: NPC emote
                title = "";
            }
            else
            {
                qe = gossiptext->Options[0].Emotes[0];

                int loc_idx = GetSession()->isRussian() ? 0 : GetSession()->GetSessionDbLocaleIndex();

                std::string title0 = gossiptext->Options[0].Text_0;
                std::string title1 = gossiptext->Options[0].Text_1;
                sObjectMgr.GetNpcTextLocaleStrings0(textid, loc_idx, &title0, &title1);

                title = !title0.empty() ? title0 : title1;
            }
        }
        PlayerTalkClass->SendQuestGiverQuestList(qe, title, guid);
    }
}

bool Player::IsActiveQuest(uint32 quest_id) const
{
    QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);

    return itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE;
}

Quest const * Player::GetNextQuest(uint64 guid, Quest const *pQuest)
{
    Object *pObject;
    QuestRelations* pObjectQR;
    QuestRelations* pObjectQIR;

    Creature *pCreature = GetMap()->GetCreature(guid);
    if (pCreature)
    {
        pObject = (Object*)pCreature;
        pObjectQR  = &sObjectMgr.mCreatureQuestRelations;
        pObjectQIR = &sObjectMgr.mCreatureQuestInvolvedRelations;
    }
    else
    {
        //we should obtain map pointer from GetMap() in 99% of cases. Special case
        //only for quests which cast teleport spells on player
        Map * _map = IsInWorld() ? GetMap() : sMapMgr.FindMap(GetMapId(), GetAnyInstanceId());
        ASSERT(_map);
        GameObject *pGameObject = _map->GetGameObject(guid);
        if (pGameObject)
        {
            pObject = (Object*)pGameObject;
            pObjectQR  = &sObjectMgr.mGOQuestRelations;
            pObjectQIR = &sObjectMgr.mGOQuestInvolvedRelations;
        }
        else
            return NULL;
    }

    uint32 nextQuestID = pQuest->GetNextQuestInChain();
    for (QuestRelations::const_iterator itr = pObjectQR->lower_bound(pObject->GetEntry()); itr != pObjectQR->upper_bound(pObject->GetEntry()); ++itr)
    {
        if (itr->second == nextQuestID)
            return sObjectMgr.GetQuestTemplate(nextQuestID);
    }

    return NULL;
}

bool Player::CanSeeStartQuest(Quest const *pQuest)
{
    if (SatisfyQuestRace(pQuest, false) && SatisfyQuestSkillOrClass(pQuest, false) &&
        SatisfyQuestExclusiveGroup(pQuest, false) && SatisfyQuestReputation(pQuest, false) &&
        SatisfyQuestPreviousQuest(pQuest, false) && SatisfyQuestNextChain(pQuest, false) &&
        SatisfyQuestPrevChain(pQuest, false) && SatisfyQuestDay(pQuest, false) && SatisfySpecialQuestConditions(pQuest, false))
    {
        return GetLevel() + sWorld.getConfig(CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF) >= pQuest->GetMinLevel();
    }

    return false;
}

bool Player::CanTakeQuest(Quest const *pQuest, bool msg, bool skipStatusCheck /*false*/) const
{
    // TODO Release/Trentone: this is wrong, because it break up alot of pvp quests.
    // there should be special flag that define BG quests by bracket
    // examples of such quests: 8080, 8154, 8155, 8156, 8297, 10535
    /*if (pQuest->GetType() == QUEST_TYPE_PVP && pQuest->GetQuestLevel() && pQuest->GetQuestLevel() < GetLevel())
        return false;*/
    return (skipStatusCheck || SatisfyQuestStatus(pQuest, msg)) && SatisfyQuestExclusiveGroup(pQuest, msg)
        && SatisfyQuestRace(pQuest, msg) && SatisfyQuestLevel(pQuest, msg)
        && SatisfyQuestSkillOrClass(pQuest, msg) && SatisfyQuestReputation(pQuest, msg)
        && SatisfyQuestPreviousQuest(pQuest, msg) && SatisfyQuestTimed(pQuest, msg)
        && SatisfyQuestNextChain(pQuest, msg) && SatisfyQuestPrevChain(pQuest, msg)
        && SatisfyQuestDay(pQuest, msg) && SatisfySpecialQuestConditions(pQuest, msg);
}

void Player::SendPetTameFailure(PetTameFailureReason reason) const
{
    WorldPacket data(SMSG_PET_TAME_FAILURE, 1);
    data << uint8(reason);
    GetSession()->SendPacket(&data);
}

bool Player::CanAddQuest(Quest const *pQuest, bool msg)
{
    if (!SatisfyQuestLog(msg))
        return false;

    uint32 srcitem = pQuest->GetSrcItemId();
    if (srcitem > 0)
    {
        uint32 count = pQuest->GetSrcItemCount();
        ItemPosCountVec dest;
        uint8 msg2 = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, srcitem, count);

        // player already have max number (in most case 1) source item, no additional item needed and quest can be added.
        if (msg2 == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
            return true;
        else if (msg2 != EQUIP_ERR_OK)
        {
            SendEquipError(msg2, NULL, NULL);
            return false;
        }
    }
    return true;
}

bool Player::CanCompleteQuest(uint32 quest_id)
{
    if (quest_id)
    {
        QuestStatusData& q_status = mQuestStatus[quest_id];
        if (q_status.m_status == QUEST_STATUS_COMPLETE)
            return false;                                   // not allow re-complete quest

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);

        if (!qInfo)
            return false;

        // auto complete quest
        if (qInfo->IsAutoComplete() && CanTakeQuest(qInfo, false))
            return true;

        if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
        {

            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_DELIVER))
            {
                for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if (qInfo->ReqItemCount[i]!= 0 && q_status.m_itemcount[i] < qInfo->ReqItemCount[i])
                        return false;
                }
            }

            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_KILL_OR_CAST | QUEST_HELLGROUND_FLAGS_SPEAKTO))
            {
                for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
                {
                    if (qInfo->ReqCreatureOrGOId[i] == 0)
                        continue;

                    if (qInfo->ReqCreatureOrGOCount[i] != 0 && q_status.m_creatureOrGOcount[i] < qInfo->ReqCreatureOrGOCount[i])
                        return false;
                }
            }

            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_EXPLORATION_OR_EVENT) && !q_status.m_explored)
                return false;

            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_TIMED) && q_status.m_timer == 0)
                return false;

            if (qInfo->GetRewOrReqMoney() < 0)
            {
                if (GetMoney() < uint32(-qInfo->GetRewOrReqMoney()))
                    return false;
            }

            uint32 repFacId = qInfo->GetRepObjectiveFaction();
            if (repFacId && m_reputationMgr.GetReputation(repFacId) < qInfo->GetRepObjectiveValue())
                return false;

            return true;
        }
    }
    return false;
}

bool Player::CanCompleteRepeatableQuest(Quest const *pQuest)
{
    // Solve problem that player don't have the quest and try complete it.
    // if repeatable she must be able to complete event if player don't have it.
    // Seem that all repeatable quest are DELIVER Flag so, no need to add more.
    if (!CanTakeQuest(pQuest, false))
        return false;

    if (pQuest->HasFlag(QUEST_HELLGROUND_FLAGS_DELIVER))
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
            if (pQuest->ReqItemId[i] && pQuest->ReqItemCount[i] && !HasItemCount(pQuest->ReqItemId[i],pQuest->ReqItemCount[i]))
                return false;

    if (!CanRewardQuest(pQuest, false))
        return false;

    return true;
}

bool Player::CanRewardQuest(Quest const *pQuest, bool msg)
{
    // Prevent packet-editing exploits
    // Players must meet prereqs for AutoComplete quests
    if (pQuest->IsAutoComplete())
    {
        if (!CanTakeQuest(pQuest, false, true))
            return false;
    }
    else
    {
        // Normal quests must be accepted and have a complete status
        if (GetQuestStatus(pQuest->GetQuestId()) != QUEST_STATUS_COMPLETE)
            return false;
    }

    // daily quest can't be rewarded (25 daily quest already completed)
    if (!SatisfyQuestDay(pQuest,true))
        return false;

    // Prevent completing the same quest twice
    if (GetQuestRewardStatus(pQuest->GetQuestId()))
        return false;

    // prevent receive reward with quest items in bank
    if (pQuest->HasFlag(QUEST_HELLGROUND_FLAGS_DELIVER))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
        {
            if (pQuest->ReqItemCount[i]!= 0 &&
                GetItemCount(pQuest->ReqItemId[i]) < pQuest->ReqItemCount[i])
            {
                if (msg)
                    SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL);
                return false;
            }
        }
    }

    // prevent receive reward with low money and GetRewOrReqMoney() < 0
    if (pQuest->GetRewOrReqMoney() < 0 && GetMoney() < uint32(-pQuest->GetRewOrReqMoney()))
    {
        SendCanTakeQuestResponse(INVALIDREASON_QUEST_FAILED_NOT_ENOUGH_MONEY);
        return false;
    }
    else if (pQuest->GetRewOrReqMoney() > 0 && GetMoney() + pQuest->GetRewOrReqMoney() >= MAX_MONEY_AMOUNT)
    {
        SendEquipError(EQUIP_ERR_TOO_MUCH_GOLD,NULL,NULL);
        return false;
    }

    return true;
}

bool Player::CanRewardQuest(Quest const *pQuest, uint32 reward, bool msg)
{
    // prevent receive reward with quest items in bank or for not completed quest
    if (!CanRewardQuest(pQuest,msg))
        return false;

    if (pQuest->GetRewChoiceItemsCount() > 0)
    {
        if (pQuest->RewChoiceItemId[reward])
        {
            uint32 count = pQuest->RewChoiceItemCount[reward];
            // it is important to be an EVEN NUMBER to avoid misinformation in the quest menu!
            if (pQuest->RewChoiceItemId[reward] == ITEM_EMBLEM_OF_TRIUMPH && GetSession()->isPremium())
            {
                uint32 bonus = CalculateBonus(count);
                count += bonus;
            }
            
            ItemPosCountVec dest;
            uint8 res = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pQuest->RewChoiceItemId[reward], count); //pQuest->RewChoiceItemCount[reward]
            if (res != EQUIP_ERR_OK)
            {
                SendEquipError(res, NULL, NULL);
                return false;
            }
        }
    }

    if (pQuest->GetRewItemsCount() > 0)
    {
        for (uint32 i = 0; i < pQuest->GetRewItemsCount(); ++i)
        {
            if (pQuest->RewItemId[i])
            {
                uint32 count = pQuest->RewItemCount[reward];
                // it is important to be an EVEN NUMBER to avoid misinformation in the quest menu!
                if (pQuest->RewItemId[reward] == ITEM_EMBLEM_OF_TRIUMPH && GetSession()->isPremium())
                {
                    uint32 bonus = CalculateBonus(count);
                    count += bonus;
                }
                
                ItemPosCountVec dest;
                uint8 res = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pQuest->RewItemId[i], count); //pQuest->RewItemCount[i]
                if (res != EQUIP_ERR_OK)
                {
                    SendEquipError(res, NULL, NULL);
                    return false;
                }
            }
        }
    }

    return true;
}

void Player::AddQuest(Quest const *pQuest, Object *questGiver)
{
    uint16 log_slot = FindQuestSlot(0);
    ASSERT(log_slot < MAX_QUEST_LOG_SIZE);

    uint32 quest_id = pQuest->GetQuestId();

    // if not exist then created with set uState==NEW and rewarded=false
    QuestStatusData& questStatusData = mQuestStatus[quest_id];

    // check for repeatable quests status reset
    questStatusData.m_status = QUEST_STATUS_INCOMPLETE;
    questStatusData.m_explored = false;

    if (pQuest->HasFlag(QUEST_HELLGROUND_FLAGS_DELIVER))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
            questStatusData.m_itemcount[i] = 0;
    }

    if (pQuest->HasFlag(QUEST_HELLGROUND_FLAGS_KILL_OR_CAST | QUEST_HELLGROUND_FLAGS_SPEAKTO))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
            questStatusData.m_creatureOrGOcount[i] = 0;
    }

    GiveQuestSourceItem(pQuest);
    AdjustQuestReqItemCount(pQuest, questStatusData);

    if (pQuest->GetRepObjectiveFaction())
        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(pQuest->GetRepObjectiveFaction()))
            m_reputationMgr.SetVisible(factionEntry);

    uint32 qtime = 0;
    if (pQuest->HasFlag(QUEST_HELLGROUND_FLAGS_TIMED))
    {
        uint32 limittime = pQuest->GetLimitTime();

        // shared timed quest
        if (questGiver && questGiver->GetTypeId()==TYPEID_PLAYER)
            limittime = ((Player*)questGiver)->getQuestStatusMap()[quest_id].m_timer / 1000;

        AddTimedQuest(quest_id);
        questStatusData.m_timer = limittime * 1000;
        qtime = static_cast<uint32>(time(NULL)) + limittime;
    }
    else
        questStatusData.m_timer = 0;

    SetQuestSlot(log_slot, quest_id, qtime);

    if (questStatusData.uState != QUEST_NEW)
        questStatusData.uState = QUEST_CHANGED;

    //starting initial quest script
    if (questGiver && pQuest->GetQuestStartScript()!=0)
        GetMap()->ScriptsStart(sQuestStartScripts, pQuest->GetQuestStartScript(), questGiver, this);

    UpdateForQuestsGO();

	sLog.outLog(LOG_CHARACTER_ACTIONS, "#quest Player %s (guid %u) started %u", GetName(), GetGUIDLow(), pQuest->GetQuestId());
}

void Player::CompleteQuest(uint32 quest_id)
{
    if (quest_id)
    {
        SetQuestStatus(quest_id, QUEST_STATUS_COMPLETE);

        uint16 log_slot = FindQuestSlot(quest_id);
        if (log_slot < MAX_QUEST_LOG_SIZE)
            SetQuestSlotState(log_slot,QUEST_STATE_COMPLETE);

        if (Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id))
        {
            if (qInfo->HasFlag(QUEST_FLAGS_AUTO_REWARDED))
                RewardQuest(qInfo,0,this,false);
            else
                SendQuestComplete(quest_id);

			sLog.outLog(LOG_CHARACTER_ACTIONS, "#quest Player %s (guid %u) complete %u", GetName(), GetGUIDLow(), quest_id);
			ClearCaptchaAction(CAPTCHA_LOOT_CORPSE);
        }
    }
}

void Player::IncompleteQuest(uint32 quest_id)
{
    if (quest_id)
    {
        SetQuestStatus(quest_id, QUEST_STATUS_INCOMPLETE);

        uint16 log_slot = FindQuestSlot(quest_id);
        if (log_slot < MAX_QUEST_LOG_SIZE)
            RemoveQuestSlotState(log_slot,QUEST_STATE_COMPLETE);

		sLog.outLog(LOG_CHARACTER_ACTIONS, "#quest Player %s (guid %u) incomplete %u", GetName(), GetGUIDLow(), quest_id);
    }
}

void Player::RewardQuest(Quest const *pQuest, uint32 reward, Object* questGiver, bool announce)
{
    uint32 quest_id = pQuest->GetQuestId();

    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (pQuest->ReqItemId[i])
            DestroyItemCount(pQuest->ReqItemId[i], pQuest->ReqItemCount[i], true, false, "QUEST_DESTROY");
    }

    //if(qInfo->HasSpecialFlag(QUEST_FLAGS_TIMED))
    //    SetTimedQuest(0);
    m_timedquests.erase(pQuest->GetQuestId());

    if (pQuest->GetRewChoiceItemsCount() > 0)
    {
        if (pQuest->RewChoiceItemId[reward])
        {
            ItemPosCountVec dest;

            uint32 count = pQuest->RewChoiceItemCount[reward];
            
            uint32 bonus = 0;
            if (pQuest->GetQuestId() == 693017 || pQuest->GetQuestId() == 693018 || pQuest->GetQuestId() == 693019)
                if (pQuest->RewChoiceItemId[reward] == ITEM_EMBLEM_OF_TRIUMPH && GetSession()->isPremium())
                {
                    bonus = CalculateBonus(count);
                    count += bonus;

                    // send ad message
                    if (!bonus)
                        ChatHandler(this).SendSysMessage(15623);
                }

            if (CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pQuest->RewChoiceItemId[reward], count) == EQUIP_ERR_OK)
            {
                Item* item = StoreNewItem(dest, pQuest->RewChoiceItemId[reward], true, 0, "QUEST_REWARD");
                SendNewItem(item, count, true, false);

                if (bonus)
                    AddEvent(new PSendSysMessageEvent(*this, 16653, GetItemLink(ITEM_EMBLEM_OF_TRIUMPH).c_str(), bonus), 2000);
            }
        }
    }

    if (pQuest->GetRewItemsCount() > 0)
    {
        for (uint32 i=0; i < pQuest->GetRewItemsCount(); ++i)
        {
            if (pQuest->RewItemId[i])
            {
                ItemPosCountVec dest;

                uint32 count = pQuest->RewItemCount[i];

                // it is important to be an EVEN NUMBER to avoid misinformation in the quest menu!
                uint32 bonus = 0;
                if (pQuest->GetQuestId() == 693017 || pQuest->GetQuestId() == 693018 || pQuest->GetQuestId() == 693019)
                    if (pQuest->RewItemId[reward] == ITEM_EMBLEM_OF_TRIUMPH && GetSession()->isPremium())
                    {
                        bonus = CalculateBonus(count);
                        count += bonus;

                        // send ad message
                        if (!bonus)
                            ChatHandler(this).SendSysMessage(15623);
                    }

                if (CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pQuest->RewItemId[i], count) == EQUIP_ERR_OK)
                {
                    Item* item = StoreNewItem(dest, pQuest->RewItemId[i], true, 0, "QUEST_REWARD");
                    SendNewItem(item, count, true, false, false, false);
                       
                    // make chest personal
                    if (item->GetEntry() == EPIC_RAID_CHEST)
                    {
                        for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
                        {
                            uint16 pos = itr->pos;
                            uint32 count = itr->count;

                            const Item* pItem = GetItemByPos(pos);
                            if (pItem && pItem->GetEntry() == EPIC_RAID_CHEST)
                                item->SetBinding(true); 
                        }
                    }

                    if (bonus)
                        AddEvent(new PSendSysMessageEvent(*this, 16653, GetItemLink(ITEM_EMBLEM_OF_TRIUMPH).c_str(), bonus), 2000);
                }
            }
        }
    }

    RewardReputation(pQuest);

    uint16 log_slot = FindQuestSlot(quest_id);
    if (log_slot < MAX_QUEST_LOG_SIZE)
        SetQuestSlot(log_slot,0);

    QuestStatusData& q_status = mQuestStatus[quest_id];

    // Not give XP in case already completed once repeatable quest
    uint32 XP = q_status.m_rewarded ? 0 : uint32(pQuest->XPValue(this)*GetXPRate(RATE_XP_QUEST));

    if (GetLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        GiveXP(XP , NULL);
    else
        ModifyMoney(int32(pQuest->GetRewMoneyMaxLevel() * sWorld.getConfig(RATE_DROP_MONEY)));

    // Give player extra money if GetRewOrReqMoney > 0 and get ReqMoney if negative
    ModifyMoney(pQuest->GetRewOrReqMoney());

    // honor reward
    if (pQuest->GetRewHonorableKills())
    {        
        // gain no honor for any quest except BG quests on x100
        //if (sWorld.isEasyRealm() && (quest_id == 8385 || quest_id == 8388))
            RewardHonor(NULL, 0, Hellground::Honor::hk_honor_at_level(GetLevel(), pQuest->GetRewHonorableKills()), false, true, true);
    }
        
        
    // title reward
    if (pQuest->GetCharTitleId())
    {
        if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(pQuest->GetCharTitleId()))
            SetTitle(titleEntry);
    }

    // Send reward mail
    if (uint32 mail_template_id = pQuest->GetRewMailTemplateId())
        MailDraft(mail_template_id).SendMailTo(this, questGiver, MAIL_CHECK_MASK_NONE, pQuest->GetRewMailDelaySecs());

    if (pQuest->IsDaily())
        SetDailyQuestStatus(quest_id);

    if (!pQuest->IsRepeatable())
        SetQuestStatus(quest_id, QUEST_STATUS_COMPLETE);
    else
        SetQuestStatus(quest_id, QUEST_STATUS_NONE);

    q_status.m_rewarded = true;

    if (announce)
        SendQuestReward(pQuest, XP, questGiver);

    bool handled = false;

    switch (questGiver->GetTypeId())
    {
        case TYPEID_UNIT:
            handled = sScriptMgr.OnQuestRewarded(this, (Creature*)questGiver, pQuest);
            break;
        case TYPEID_GAMEOBJECT:
            handled = sScriptMgr.OnQuestRewarded(this, (GameObject*)questGiver, pQuest);
            break;
    }

    if (!handled && pQuest->GetQuestCompleteScript() != 0)
        GetMap()->ScriptsStart(sQuestEndScripts, pQuest->GetQuestCompleteScript(), questGiver, this);

    if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

    if (pQuest->GetRewSpellCast() > 0)
        CastSpell(this, pQuest->GetRewSpellCast(), true);
    else if (pQuest->GetRewSpell() > 0)
        CastSpell(this, pQuest->GetRewSpell(), true);

    // TODO: need a virtual function ?
    if (InBattleGroundOrArena())
        if (BattleGround* bg = GetBattleGround())
            if (bg->GetTypeID() == BATTLEGROUND_AV)
                ((BattleGroundAV*)bg)->HandleQuestComplete(pQuest->GetQuestId(), this);

    SendQuestGiverStatusMultiple();

    // @!newbie_quest
    // instantly add next quest in some cases for better experience & locks
    if (quest_id == 693030 || quest_id == 693032)
    {
        if (quest_id == 693032)
            AddPlayerCustomFlag(PL_CUSTOM_NEWBIE_QUEST_LOCK);

        uint32 next_quest = ++quest_id;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(next_quest);

        if (pQuest && CanAddQuest(pQuest, true))
            AddQuest(pQuest, NULL);
    }
    else if (quest_id == 693033)
        RemovePlayerCustomFlag(PL_CUSTOM_NEWBIE_QUEST_LOCK);

	sLog.outLog(LOG_CHARACTER_ACTIONS, "#quest Player %s (guid %u) reward %u", GetName(), GetGUIDLow(), quest_id);
}

void Player::SendQuestGiverStatusMultiple()
{
    uint32 count = 0;

    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 4);
    data << uint32(count);                                  // placeholder

    for (Player::ClientGUIDs::iterator itr = this->m_clientGUIDs.begin(); itr != this->m_clientGUIDs.end(); ++itr)
    {
        uint8 questStatus = DIALOG_STATUS_NONE;
        uint8 defstatus = DIALOG_STATUS_NONE;

        if (IS_CREATURE_GUID(*itr))
        {
            Creature *questgiver = GetMap()->GetCreature(*itr);
            if (!questgiver || questgiver->IsHostileTo(this))
                continue;
            if (!questgiver->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                continue;
            questStatus = sScriptMgr.GetDialogStatus(this, questgiver);
            if (questStatus == DIALOG_STATUS_SCRIPTED_NO_STATUS)
                questStatus = GetSession()->getDialogStatus(this, questgiver, defstatus);

            data << uint64(questgiver->GetGUID());
            data << uint8(questStatus);
            ++count;
        }
        else if (IS_GAMEOBJECT_GUID(*itr))
        {
            GameObject *questgiver = GetMap()->GetGameObject(*itr);
            if (!questgiver)
                continue;
            if (questgiver->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
                continue;
            questStatus = sScriptMgr.GetDialogStatus(this, questgiver);
           if (questStatus == DIALOG_STATUS_SCRIPTED_NO_STATUS)
                questStatus = GetSession()->getDialogStatus(this, questgiver, defstatus);

            data << uint64(questgiver->GetGUID());
            data << uint8(questStatus);
            ++count;
        }
    }

    data.put<uint32>(0, count);                             // write real count
    GetSession()->SendPacket(&data);
}

void Player::ModifyMoney(int32 d)
{
	if (d == 0)
		return;
	
	if (d < 0)
        SetMoney (GetMoney() > uint32(-d) ? GetMoney() + d : 0);
    else
        SetMoney (GetMoney() < uint32(MAX_MONEY_AMOUNT - d) ? GetMoney() + d : MAX_MONEY_AMOUNT);

    // "At Gold Limit"
    if (GetMoney() >= MAX_MONEY_AMOUNT)
        SendEquipError(EQUIP_ERR_TOO_MUCH_GOLD,NULL,NULL);

	sLog.outLog(LOG_CHARACTER_ACTIONS, "#money Player %s (guid %u) modified money amount %d", GetName(), GetGUIDLow(), d);
}

void Player::RewardDNDQuest(uint32 questId)
{
    const Quest * tmpQ = sObjectMgr.GetQuestTemplate(questId);
    if (!tmpQ)
        return;

    QuestStatusData& q_status = mQuestStatus[questId];

    // Not give XP in case already completed once repeatable quest
    uint32 XP = q_status.m_rewarded ? 0 : uint32(tmpQ->XPValue(this)*GetXPRate(RATE_XP_QUEST));

    if (GetLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        GiveXP(XP , NULL);
    else
        ModifyMoney(int32(tmpQ->GetRewMoneyMaxLevel() * sWorld.getConfig(RATE_DROP_MONEY)));

    RewardReputation(tmpQ);

    // Give player extra money if GetRewOrReqMoney > 0 and get ReqMoney if negative
    ModifyMoney(tmpQ->GetRewOrReqMoney());

    q_status.m_rewarded = true;
    q_status.m_status = QUEST_STATUS_COMPLETE;

    if (q_status.uState != QUEST_NEW)
        q_status.uState = QUEST_CHANGED;

    if (tmpQ->GetRewSpellCast() > 0)
        CastSpell(this, tmpQ->GetRewSpellCast(), true);
    else if (tmpQ->GetRewSpell() > 0)
        CastSpell(this, tmpQ->GetRewSpell(), true);
}

void Player::FailQuest(uint32 quest_id)
{
    if (quest_id)
    {
        IncompleteQuest(quest_id);

        uint16 log_slot = FindQuestSlot(quest_id);
        if (log_slot < MAX_QUEST_LOG_SIZE)
        {
            SetQuestSlotTimer(log_slot, 1);
            SetQuestSlotState(log_slot,QUEST_STATE_FAIL);
        }
        SendQuestFailed(quest_id);

		sLog.outLog(LOG_CHARACTER_ACTIONS, "#quest Player %s (guid %u) failed %u", GetName(), GetGUIDLow(), quest_id);
    }
}

void Player::FailTimedQuest(uint32 quest_id)
{
    if (quest_id)
    {
        QuestStatusData& q_status = mQuestStatus[quest_id];

        q_status.m_timer = 0;
        if (q_status.uState != QUEST_NEW)
            q_status.uState = QUEST_CHANGED;

        IncompleteQuest(quest_id);

        uint16 log_slot = FindQuestSlot(quest_id);
        if (log_slot < MAX_QUEST_LOG_SIZE)
        {
            SetQuestSlotTimer(log_slot, 1);
            SetQuestSlotState(log_slot,QUEST_STATE_FAIL);
        }
        SendQuestTimerFailed(quest_id);

		sLog.outLog(LOG_CHARACTER_ACTIONS, "#quest Player %s (guid %u) failed timed %u", GetName(), GetGUIDLow(), quest_id);
    }
}

bool Player::SatisfyQuestSkillOrClass(Quest const* qInfo, bool msg) const
{
    int32 zoneOrSort   = qInfo->GetZoneOrSort();
    int32 skillOrClass = qInfo->GetSkillOrClass();

    // skip zone zoneOrSort and 0 case skillOrClass
    if (zoneOrSort >= 0 && skillOrClass == 0)
        return true;

    int32 questSort = -zoneOrSort;
    uint8 reqSortClass = ClassByQuestSort(questSort);

    // check class sort cases in zoneOrSort
    if (reqSortClass != 0 && GetClass() != reqSortClass)
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
        return false;
    }

    // check class
    if (skillOrClass < 0)
    {
        uint8 reqClass = -int32(skillOrClass);
        if (GetClass() != reqClass)
        {
            if (msg)
                SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
            return false;
        }
    }
    // check skill
    else if (skillOrClass > 0)
    {
        uint32 reqSkill = skillOrClass;
        if (GetSkillValue(reqSkill) < qInfo->GetRequiredSkillValue())
        {
            if (msg)
                SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
            return false;
        }
    }

    return true;
}

bool Player::SatisfySpecialQuestConditions(Quest const* qInfo, bool msg) const
{
    bool Satisfy = true;
    switch(qInfo->GetQuestId())
    {
        case 7483:
        case 7484:
        case 7485:
        {
            if((GetQuestStatus(7481) == QUEST_STATUS_COMPLETE) || (GetQuestStatus(7482) == QUEST_STATUS_COMPLETE))
                Satisfy = true;
            else
            {
                if (msg)
                    SendCanTakeQuestResponse(INVALIDREASON_QUEST_FAILED_EXPANSION);
                Satisfy = false;
            }
            break;
        }
        case 8767:
        {
            if(GetClass() == CLASS_WARRIOR || GetClass() == CLASS_ROGUE)
                Satisfy = true;
            else
                Satisfy = false;
            break;
        }
        case 8788:
        {
            if(GetClass() == CLASS_WARRIOR || GetClass() == CLASS_ROGUE)
                Satisfy = false;
            else
                Satisfy = true;
            break;
        }
        default:
        {
            Satisfy = true;
            break;
        }
    }
    return Satisfy;
}

bool Player::SatisfyQuestLevel(Quest const* qInfo, bool msg) const
{
    if (GetLevel() < qInfo->GetMinLevel())
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
        return false;
    }
    return true;
}

bool Player::SatisfyQuestLog(bool msg) const
{
    // exist free slot
    if (FindQuestSlot(0) < MAX_QUEST_LOG_SIZE)
        return true;

    if (msg)
    {
        WorldPacket data(SMSG_QUESTLOG_FULL, 0);
        SendPacketToSelf(&data);
        sLog.outDebug("WORLD: Sent QUEST_LOG_FULL_MESSAGE");
    }
    return false;
}

bool Player::SatisfyQuestPreviousQuest(Quest const* qInfo, bool msg) const
{
    // No previous quest (might be first quest in a series)
    if (qInfo->prevQuests.empty())
        return true;

    for (Quest::PrevQuests::const_iterator iter = qInfo->prevQuests.begin(); iter != qInfo->prevQuests.end(); ++iter)
    {
        uint32 prevId = abs(*iter);

        QuestStatusMap::const_iterator i_prevstatus = mQuestStatus.find(prevId);
        Quest const* qPrevInfo = sObjectMgr.GetQuestTemplate(prevId);

        if (qPrevInfo && i_prevstatus != mQuestStatus.end())
        {
            // If any of the positive previous quests completed, return true
            if (*iter > 0 && i_prevstatus->second.m_rewarded)
            {
                // skip one-from-all exclusive group
                if (qPrevInfo->GetExclusiveGroup() >= 0)
                    return true;

                // each-from-all exclusive group (< 0)
                // can be start if only all quests in prev quest exclusive group completed and rewarded
                ObjectMgr::ExclusiveQuestGroups::iterator iter2 = sObjectMgr.mExclusiveQuestGroups.lower_bound(qPrevInfo->GetExclusiveGroup());
                ObjectMgr::ExclusiveQuestGroups::iterator end  = sObjectMgr.mExclusiveQuestGroups.upper_bound(qPrevInfo->GetExclusiveGroup());

                ASSERT(iter2!=end);                          // always must be found if qPrevInfo->ExclusiveGroup != 0

                for (; iter2 != end; ++iter2)
                {
                    uint32 exclude_Id = iter2->second;

                    // skip checked quest id, only state of other quests in group is interesting
                    if (exclude_Id == prevId)
                        continue;

                    QuestStatusMap::const_iterator i_exstatus = mQuestStatus.find(exclude_Id);

                    // alternative quest from group also must be completed and rewarded(reported)
                    if (i_exstatus == mQuestStatus.end() || !i_exstatus->second.m_rewarded)
                    {
                        if (msg)
                            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
                        return false;
                    }
                }
                return true;
            }
            // If any of the negative previous quests active, return true
            if (*iter < 0 && (i_prevstatus->second.m_status == QUEST_STATUS_INCOMPLETE
                || (i_prevstatus->second.m_status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(prevId))))
            {
                // skip one-from-all exclusive group
                if (qPrevInfo->GetExclusiveGroup() >= 0)
                    return true;

                // each-from-all exclusive group (< 0)
                // can be start if only all quests in prev quest exclusive group active
                ObjectMgr::ExclusiveQuestGroups::iterator iter2 = sObjectMgr.mExclusiveQuestGroups.lower_bound(qPrevInfo->GetExclusiveGroup());
                ObjectMgr::ExclusiveQuestGroups::iterator end  = sObjectMgr.mExclusiveQuestGroups.upper_bound(qPrevInfo->GetExclusiveGroup());

                ASSERT(iter2!=end);                          // always must be found if qPrevInfo->ExclusiveGroup != 0

                for (; iter2 != end; ++iter2)
                {
                    uint32 exclude_Id = iter2->second;

                    // skip checked quest id, only state of other quests in group is interesting
                    if (exclude_Id == prevId)
                        continue;

                    QuestStatusMap::const_iterator i_exstatus = mQuestStatus.find(exclude_Id);

                    // alternative quest from group also must be active
                    if (i_exstatus == mQuestStatus.end() ||
                        i_exstatus->second.m_status != QUEST_STATUS_INCOMPLETE &&
                        (i_prevstatus->second.m_status != QUEST_STATUS_COMPLETE || GetQuestRewardStatus(prevId)))
                    {
                        if (msg)
                            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
                        return false;
                    }
                }
                return true;
            }
        }
    }

    // Has only positive prev. quests in non-rewarded state
    // and negative prev. quests in non-active state
    if (msg)
        SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

    return false;
}

bool Player::SatisfyQuestRace(Quest const* qInfo, bool msg) const
{
    uint32 reqraces = qInfo->GetRequiredRaces();
    if (reqraces == 0)
        return true;
    if ((reqraces & GetRaceMask()) == 0)
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_QUEST_FAILED_WRONG_RACE);
        return false;
    }
    return true;
}

bool Player::SatisfyQuestReputation(Quest const* qInfo, bool msg) const
{
    uint32 fIdMin = qInfo->GetRequiredMinRepFaction();      //Min required rep
    if (fIdMin && m_reputationMgr.GetReputation(fIdMin) < qInfo->GetRequiredMinRepValue())
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
        return false;
    }

    uint32 fIdMax = qInfo->GetRequiredMaxRepFaction();      //Max required rep
    if (fIdMax && m_reputationMgr.GetReputation(fIdMax) >= qInfo->GetRequiredMaxRepValue())
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
        return false;
    }

    return true;
}

bool Player::SatisfyQuestStatus(Quest const* qInfo, bool msg) const
{
    QuestStatusMap::const_iterator itr = mQuestStatus.find(qInfo->GetQuestId());
    if  (itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE)
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_QUEST_ALREADY_ON);
        return false;
    }
    return true;
}

bool Player::SatisfyQuestTimed(Quest const* qInfo, bool msg) const
{
    if ((find(m_timedquests.begin(), m_timedquests.end(), qInfo->GetQuestId()) != m_timedquests.end()) && qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_TIMED))
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_QUEST_ONLY_ONE_TIMED);
        return false;
    }
    return true;
}

bool Player::SatisfyQuestExclusiveGroup(Quest const* qInfo, bool msg) const
{
    // non positive exclusive group, if > 0 then can be start if any other quest in exclusive group already started/completed
    if (qInfo->GetExclusiveGroup() <= 0)
        return true;

    if (qInfo->IsDaily() && !sWorld.getConfig(CONFIG_DAILY_BLIZZLIKE))
        return true;

    ObjectMgr::ExclusiveQuestGroups::iterator iter = sObjectMgr.mExclusiveQuestGroups.lower_bound(qInfo->GetExclusiveGroup());
    ObjectMgr::ExclusiveQuestGroups::iterator end  = sObjectMgr.mExclusiveQuestGroups.upper_bound(qInfo->GetExclusiveGroup());

    ASSERT(iter!=end);                                      // always must be found if qInfo->ExclusiveGroup != 0

    for (; iter != end; ++iter)
    {
        uint32 exclude_Id = iter->second;

        // skip checked quest id, only state of other quests in group is interesting
        if (exclude_Id == qInfo->GetQuestId())
            continue;

        // not allow have daily quest if daily quest from exclusive group already recently completed
        Quest const* Nquest = sObjectMgr.GetQuestTemplate(exclude_Id);
        if (!SatisfyQuestDay(Nquest, false))
        {
            if (msg)
                SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
            return false;
        }

        QuestStatusMap::const_iterator i_exstatus = mQuestStatus.find(exclude_Id);

        // alternative quest already started or completed
        if (i_exstatus != mQuestStatus.end()
            && (i_exstatus->second.m_status == QUEST_STATUS_COMPLETE || i_exstatus->second.m_status == QUEST_STATUS_INCOMPLETE))
        {
            if (msg)
                SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
            return false;
        }
    }
    return true;
}

bool Player::SatisfyQuestNextChain(Quest const* qInfo, bool msg) const
{
    if (!qInfo->GetNextQuestInChain())
        return true;

    // next quest in chain already started or completed
    QuestStatusMap::const_iterator itr = mQuestStatus.find(qInfo->GetNextQuestInChain());
    if (itr != mQuestStatus.end()
        && (itr->second.m_status == QUEST_STATUS_COMPLETE || itr->second.m_status == QUEST_STATUS_INCOMPLETE))
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
        return false;
    }

    // check for all quests further up the chain
    // only necessary if there are quest chains with more than one quest that can be skipped
    //return SatisfyQuestNextChain(qInfo->GetNextQuestInChain(), msg);
    return true;
}

bool Player::SatisfyQuestPrevChain(Quest const* qInfo, bool msg) const
{
    // No previous quest in chain
    if (qInfo->prevChainQuests.empty())
        return true;

    for (Quest::PrevChainQuests::const_iterator iter = qInfo->prevChainQuests.begin(); iter != qInfo->prevChainQuests.end(); ++iter)
    {
        uint32 prevId = *iter;

        QuestStatusMap::const_iterator i_prevstatus = mQuestStatus.find(prevId);

        if (i_prevstatus != mQuestStatus.end())
        {
            // If any of the previous quests in chain active, return false
            if (i_prevstatus->second.m_status == QUEST_STATUS_INCOMPLETE
                || (i_prevstatus->second.m_status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(prevId)))
            {
                if (msg)
                    SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);
                return false;
            }
        }

        // check for all quests further down the chain
        // only necessary if there are quest chains with more than one quest that can be skipped
        //if(!SatisfyQuestPrevChain(prevId, msg))
        //    return false;
    }

    // No previous quest in chain active
    return true;
}

bool Player::SatisfyQuestDay(Quest const* qInfo, bool msg) const
{
    if (!qInfo->IsDaily())
        return true;

    bool have_slot = false;
    for (uint32 quest_daily_idx = 0; quest_daily_idx < sWorld.getConfig(CONFIG_DAILY_MAX_PER_DAY); ++quest_daily_idx)
    {
        uint32 id = GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx);
        if (qInfo->GetQuestId()==id)
            return false;

        if (!id)
            have_slot = true;
    }

    if (!have_slot)
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DAILY_QUESTS_REMAINING);
        return false;
    }

    return true;
}

bool Player::GiveQuestSourceItem(Quest const *pQuest)
{
    uint32 srcitem = pQuest->GetSrcItemId();
    if (srcitem > 0)
    {
        uint32 count = pQuest->GetSrcItemCount();
        if (count <= 0)
            count = 1;

        ItemPosCountVec dest;
        uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, srcitem, count);
        if (msg == EQUIP_ERR_OK)
        {
            Item * item = StoreNewItem(dest, srcitem, true, 0, "QUEST_SOURCE");
            SendNewItem(item, count, true, false);
            return true;
        }
        // player already have max amount required item, just report success
        else if (msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
            return true;
        else
            SendEquipError(msg, NULL, NULL);
        return false;
    }

    return true;
}

bool Player::TakeQuestSourceItem(uint32 quest_id, bool msg)
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (qInfo)
    {
        uint32 srcitem = qInfo->GetSrcItemId();
        if (srcitem > 0)
        {
            uint32 count = qInfo->GetSrcItemCount();
            if (count <= 0)
                count = 1;

            // exist one case when destroy source quest item not possible:
            // non un-equippable item (equipped non-empty bag, for example)
            uint8 res = CanUnequipItems(srcitem,count);
            if (res != EQUIP_ERR_OK)
            {
                if (msg)
                    SendEquipError(res, NULL, NULL);
                return false;
            }

            DestroyItemCount(srcitem, count, true, true, "QUEST_DESTROY");
        }
    }
    return true;
}

bool Player::GetQuestRewardStatus(uint32 quest_id) const
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (qInfo)
    {
        // for repeatable quests: rewarded field is set after first reward only to prevent getting XP more than once
        QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);
        if (itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE
            && !qInfo->IsRepeatable())
            return itr->second.m_rewarded;

        return false;
    }
    return false;
}

QuestStatus Player::GetQuestStatus(uint32 quest_id) const
{
    if (quest_id)
    {
        QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);
        if (itr != mQuestStatus.end())
            return itr->second.m_status;
    }
    return QUEST_STATUS_NONE;
}

bool Player::CanShareQuest(uint32 quest_id) const
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (qInfo && qInfo->HasFlag(QUEST_FLAGS_SHARABLE))
    {
		return true;
		
		//QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);
        //if (itr != mQuestStatus.end())
        //    return itr->second.m_status == QUEST_STATUS_NONE || itr->second.m_status == QUEST_STATUS_INCOMPLETE;
    }
    return false;
}

void Player::SetQuestStatus(uint32 quest_id, QuestStatus status)
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (qInfo)
    {
        if (status == QUEST_STATUS_NONE || status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE)
        {
            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_TIMED))
                m_timedquests.erase(qInfo->GetQuestId());
        }

        QuestStatusData& q_status = mQuestStatus[quest_id];

        q_status.m_status = status;
        if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;
    }

    UpdateForQuestsGO();
}

// not used in TrinIty, but used in scripting code
uint32 Player::GetReqKillOrCastCurrentCount(uint32 quest_id, int32 entry)
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (!qInfo)
        return 0;

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
        if (qInfo->ReqCreatureOrGOId[j] == entry)
            return mQuestStatus[quest_id].m_creatureOrGOcount[j];

    return 0;
}

void Player::AdjustQuestReqItemCount(Quest const* pQuest, QuestStatusData& questStatusData)
{
    if (pQuest->HasFlag(QUEST_HELLGROUND_FLAGS_DELIVER))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
        {
            uint32 reqitemcount = pQuest->ReqItemCount[i];
            if (reqitemcount != 0)
            {
                uint32 quest_id = pQuest->GetQuestId();
                uint32 curitemcount = GetItemCount(pQuest->ReqItemId[i],true);

                questStatusData.m_itemcount[i] = std::min(curitemcount, reqitemcount);
                if (questStatusData.uState != QUEST_NEW)
                    questStatusData.uState = QUEST_CHANGED;
            }
        }
    }
}

uint16 Player::FindQuestSlot(uint32 quest_id) const
{
    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
        if (GetQuestSlotQuestId(i) == quest_id)
            return i;

    return MAX_QUEST_LOG_SIZE;
}

void Player::AreaExploredOrEventHappens(uint32 questId)
{
    if (questId)
    {
        uint16 log_slot = FindQuestSlot(questId);
        if (log_slot < MAX_QUEST_LOG_SIZE)
        {
            QuestStatusData& q_status = mQuestStatus[questId];

            if (!q_status.m_explored)
            {
                q_status.m_explored = true;
                if (q_status.uState != QUEST_NEW)
                    q_status.uState = QUEST_CHANGED;
            }

            if (CanCompleteQuest(questId))
                CompleteQuest(questId);
            else
                SendQuestComplete(questId);
        }
    }
}

//not used in Trinityd, function for external script library
void Player::GroupEventHappens(uint32 questId, WorldObject const* pEventObject)
{
    if (Group *pGroup = GetGroup())
    {
        for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player *pGroupGuy = itr->getSource();

            // for any leave or dead (with not released body) group member at appropriate distance
            if (pGroupGuy && pGroupGuy->IsAtGroupRewardDistance(pEventObject) && !pGroupGuy->GetCorpse())
                pGroupGuy->AreaExploredOrEventHappens(questId);
        }
    }
    else
        AreaExploredOrEventHappens(questId);
}

void Player::ItemAddedQuestCheck(uint32 entry, uint32 count)
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (questid == 0)
            continue;

        QuestStatusData& q_status = mQuestStatus[questid];

        if (q_status.m_status != QUEST_STATUS_INCOMPLETE)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo || !qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_DELIVER))
            continue;

        for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
        {
            uint32 reqitem = qInfo->ReqItemId[j];
            if (reqitem == entry)
            {
                uint32 reqitemcount = qInfo->ReqItemCount[j];
                uint32 curitemcount = q_status.m_itemcount[j];
                if (curitemcount < reqitemcount)
                {
                    uint32 additemcount = (curitemcount + count <= reqitemcount ? count : reqitemcount - curitemcount);
                    q_status.m_itemcount[j] += additemcount;
                    if (q_status.uState != QUEST_NEW)
                        q_status.uState = QUEST_CHANGED;

                    SendQuestUpdateAddItem(qInfo, j, curitemcount, additemcount);
                }
                if (CanCompleteQuest(questid))
                    CompleteQuest(questid);
                else if (q_status.m_itemcount[j] == reqitemcount)
                    UpdateForQuestsGO(); // call UpdateForQuestsGO to remove sparkles from finished objective on client
            }
        }
    }
}

void Player::ItemRemovedCheck(uint32 entry, uint32 count, std::string removed_from)
{
	if (!removed_from.empty())
		sLog.outLog(LOG_CHARACTER_ACTIONS, "#item Player %s (guid %u) removed item %u:%u - %s", GetName(), GetGUIDLow(), entry, count, removed_from.c_str());

	for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;
        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo)
            continue;
        if (!qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_DELIVER))
            continue;

        for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
        {
            uint32 reqitem = qInfo->ReqItemId[j];
            if (reqitem == entry)
            {
                QuestStatusData& q_status = mQuestStatus[questid];

                uint32 reqitemcount = qInfo->ReqItemCount[j];
                uint32 curitemcount;
                if (q_status.m_status != QUEST_STATUS_COMPLETE)
                    curitemcount = q_status.m_itemcount[j];
                else
                    curitemcount = GetItemCount(entry,true);
                if (curitemcount < reqitemcount + count)
                {
                    uint32 remitemcount = (curitemcount <= reqitemcount ? count : count + reqitemcount - curitemcount);
                    q_status.m_itemcount[j] = curitemcount - remitemcount;
                    if (q_status.uState != QUEST_NEW)
                        q_status.uState = QUEST_CHANGED;

                    IncompleteQuest(questid);
                    UpdateForQuestsGO();
                }
            }
        }
    }
}

void Player::KilledMonster(uint32 entry, uint64 guid, uint32 only_quest_id)
{
    // @!newbie_quest optimization
    if (only_quest_id && GetQuestStatus(only_quest_id) != QUEST_STATUS_INCOMPLETE)
        return;
    
    uint32 addkillcount = 1;
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo)
            continue;
        // just if !ingroup || !noraidgroup || raidgroup
        QuestStatusData& q_status = mQuestStatus[questid];
        if (q_status.m_status == QUEST_STATUS_INCOMPLETE && (!GetGroup() || !GetGroup()->isRaidGroup() || GetGroup()->isBGGroup() || qInfo->GetType() == QUEST_TYPE_RAID))
        {
            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_KILL_OR_CAST))
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    // skip GO activate objective or none
                    if (qInfo->ReqCreatureOrGOId[j] <=0)
                        continue;

                    // skip Cast at creature objective
                    if (qInfo->ReqSpell[j] !=0)
                        continue;

                    uint32 reqkill = qInfo->ReqCreatureOrGOId[j];

                    if (reqkill == entry)
                    {
                        uint32 reqkillcount = qInfo->ReqCreatureOrGOCount[j];
                        uint32 curkillcount = q_status.m_creatureOrGOcount[j];
                        if (curkillcount < reqkillcount)
                        {
                            q_status.m_creatureOrGOcount[j] = curkillcount + addkillcount;
                            if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreatureOrGo(qInfo, guid, j, curkillcount, addkillcount);
                        }
                        if (CanCompleteQuest(questid))
                            CompleteQuest(questid);

                        // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                        continue;
                    }
                }
            }
        }
    }
}

void Player::CastCreatureOrGO(uint32 entry, uint64 guid, uint32 spell_id)
{
    bool isCreature = IS_CREATURE_GUID(guid);

    uint32 addCastCount = 1;
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo )
            continue;

        QuestStatusData& q_status = mQuestStatus[questid];

        if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
        {
            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_KILL_OR_CAST))
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                    // skip kill creature objective (0) or wrong spell casts
                    if (qInfo->ReqSpell[j] != spell_id)
                        continue;

                    uint32 reqTarget = 0;

                    if (isCreature)
                    {
                        // creature activate objectives
                        if (qInfo->ReqCreatureOrGOId[j] > 0)
                            // checked at quest_template loading
                            reqTarget = qInfo->ReqCreatureOrGOId[j];
                    }
                    else
                    {
                        // GO activate objective
                        if (qInfo->ReqCreatureOrGOId[j] < 0)
                            // checked at quest_template loading
                            reqTarget = - qInfo->ReqCreatureOrGOId[j];
                    }

                    // other not this creature/GO related objectives
                    if (reqTarget != entry)
                        continue;

                    uint32 reqCastCount = qInfo->ReqCreatureOrGOCount[j];
                    uint32 curCastCount = q_status.m_creatureOrGOcount[j];
                    if (curCastCount < reqCastCount)
                    {
                        q_status.m_creatureOrGOcount[j] = curCastCount + addCastCount;
                        if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                        SendQuestUpdateAddCreatureOrGo(qInfo, guid, j, curCastCount, addCastCount);
                    }

                    if (CanCompleteQuest(questid))
                        CompleteQuest(questid);

                    // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                    break;
                }
            }
        }
    }
}

void Player::TalkedToCreature(uint32 entry, uint64 guid)
{
    uint32 addTalkCount = 1;
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo)
            continue;

        QuestStatusData& q_status = mQuestStatus[questid];

        if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
        {
            if (qInfo->HasFlag(QUEST_HELLGROUND_FLAGS_KILL_OR_CAST | QUEST_HELLGROUND_FLAGS_SPEAKTO))
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
                {
                                                            // skip spell casts and Gameobject objectives
                    if (qInfo->ReqSpell[j] > 0 || qInfo->ReqCreatureOrGOId[j] < 0)
                        continue;

                    uint32 reqTarget = 0;

                    if (qInfo->ReqCreatureOrGOId[j] > 0)     // creature activate objectives
                                                            // checked at quest_template loading
                        reqTarget = qInfo->ReqCreatureOrGOId[j];
                    else
                        continue;

                    if (reqTarget == entry)
                    {
                        uint32 reqTalkCount = qInfo->ReqCreatureOrGOCount[j];
                        uint32 curTalkCount = q_status.m_creatureOrGOcount[j];
                        if (curTalkCount < reqTalkCount)
                        {
                            q_status.m_creatureOrGOcount[j] = curTalkCount + addTalkCount;
                            if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreatureOrGo(qInfo, guid, j, curTalkCount, addTalkCount);
                        }
                        if (CanCompleteQuest(questid))
                            CompleteQuest(questid);

                        // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                        continue;
                    }
                }
            }
        }
    }
}

void Player::MoneyChanged(uint32 count)
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (qInfo && qInfo->GetRewOrReqMoney() < 0)
        {
            QuestStatusData& q_status = mQuestStatus[questid];

            if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
            {
                if (int32(count) >= -qInfo->GetRewOrReqMoney())
                {
                    if (CanCompleteQuest(questid))
                        CompleteQuest(questid);
                }
            }
            else if (q_status.m_status == QUEST_STATUS_COMPLETE)
            {
                if (int32(count) < -qInfo->GetRewOrReqMoney())
                    IncompleteQuest(questid);
            }
        }
    }
}

void Player::ReputationChanged(FactionEntry const* factionEntry )
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        if (uint32 questid = GetQuestSlotQuestId(i))
        {
            if (Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid))
            {
                if (qInfo->GetRepObjectiveFaction() == factionEntry->ID)
                {
                    QuestStatusData& q_status = mQuestStatus[questid];
                    if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
                    {
                        if (m_reputationMgr.GetReputation(factionEntry) >= qInfo->GetRepObjectiveValue())
                            if (CanCompleteQuest(questid))
                                CompleteQuest(questid);
                    }
                    else if (q_status.m_status == QUEST_STATUS_COMPLETE)
                    {
                        if (m_reputationMgr.GetReputation(factionEntry) < qInfo->GetRepObjectiveValue())
                            IncompleteQuest(questid);
                    }
                }
            }
        }
    }
}

bool Player::HasQuestForItem(uint32 itemid) const
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (questid == 0)
            continue;

        QuestStatusMap::const_iterator qs_itr = mQuestStatus.find(questid);
        if(qs_itr == mQuestStatus.end())
            continue;

        QuestStatusData const& q_status = qs_itr->second;

        if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
        {
            Quest const* qinfo = sObjectMgr.GetQuestTemplate(questid);
            if (!qinfo)
                continue;

            // hide quest if player is in raid-group and quest is no raid quest
            if (GetGroup() && GetGroup()->isRaidGroup() && qinfo->GetType() != QUEST_TYPE_RAID)
                if (!InBattleGroundOrArena()) //there are two ways.. we can make every bg-quest a raidquest, or add this code here.. i don't know if this can be exploited by other quests, but i think all other quests depend on a specific area.. but keep this in mind, if something strange happens later
                    continue;

            // There should be no mixed ReqItem/ReqSource drop
            // This part for ReqItem drop
            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                if (itemid == qinfo->ReqItemId[j] && q_status.m_itemcount[j] < qinfo->ReqItemCount[j])
                    return true;
            }
            // This part - for ReqSource
            for (int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; j++)
            {
                // examined item is a source item
                if (qinfo->ReqSourceId[j] == itemid && qinfo->ReqSourceRef[j] > 0 && qinfo->ReqSourceRef[j] <= QUEST_OBJECTIVES_COUNT)
                {
                    uint32 idx = qinfo->ReqSourceRef[j]-1;

                    // total count of created ReqItems and SourceItems is less than ReqItemCount
                    if (qinfo->ReqItemId[idx] != 0 &&
                        q_status.m_itemcount[idx] * qinfo->ReqSourceCount[j] + GetItemCount(itemid,true) < qinfo->ReqItemCount[idx] * qinfo->ReqSourceCount[j])
                        return true;

                    // total count of cast ReqCreatureOrGOs and SourceItems is less than ReqCreatureOrGOCount
                    if (qinfo->ReqCreatureOrGOId[idx] != 0)
                    {
                        if (q_status.m_creatureOrGOcount[idx] * qinfo->ReqSourceCount[j] + GetItemCount(itemid,true) < qinfo->ReqCreatureOrGOCount[idx] * qinfo->ReqSourceCount[j])
                            return true;
                    }
                    // spell with SPELL_EFFECT_QUEST_COMPLETE or SPELL_EFFECT_SEND_EVENT (with script) case
                    else if (qinfo->ReqSpell[idx] != 0)
                    {
                        // not cast and need more reagents/item for use.
                        if (!q_status.m_explored && GetItemCount(itemid,true) < qinfo->ReqSourceCount[j])
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

void Player::SendQuestComplete(uint32 quest_id)
{
    if (quest_id)
    {
        WorldPacket data(SMSG_QUESTUPDATE_COMPLETE, 4);
        data << uint32(quest_id);
        SendPacketToSelf(&data);
        sLog.outDebug("WORLD: Sent SMSG_QUESTUPDATE_COMPLETE quest = %u", quest_id);
    }
}

void Player::SendQuestReward(Quest const *pQuest, uint32 XP, Object * questGiver)
{
    uint32 questid = pQuest->GetQuestId();
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_COMPLETE quest = %u", questid);
    sGameEventMgr.HandleQuestComplete(questid);
    WorldPacket data(SMSG_QUESTGIVER_QUEST_COMPLETE, (4+4+4+4+4+4+pQuest->GetRewItemsCount()*8));
    data << questid;
    data << uint32(0x03);

    if (GetLevel() < sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
    {
        data << XP;
        data << uint32(pQuest->GetRewOrReqMoney());
    }
    else
    {
        data << uint32(0);
        data << uint32(pQuest->GetRewOrReqMoney() + int32(pQuest->GetRewMoneyMaxLevel() * sWorld.getConfig(RATE_DROP_MONEY)));
    }
    data << uint32(0);                                      // new 2.3.0, HonorPoints?
    data << uint32(pQuest->GetRewItemsCount());           // max is 5

    for (uint32 i = 0; i < pQuest->GetRewItemsCount(); ++i)
    {
        if (pQuest->RewItemId[i] > 0)
            data << pQuest->RewItemId[i] << pQuest->RewItemCount[i];
        else
            data << uint32(0) << uint32(0);
    }
    SendPacketToSelf(&data);
}

void Player::SendQuestFailed(uint32 quest_id)
{
    if (quest_id)
    {
        WorldPacket data(SMSG_QUESTGIVER_QUEST_FAILED, 4);
        data << quest_id;
        SendPacketToSelf(&data);
        sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_FAILED");
    }
}

void Player::SendQuestTimerFailed(uint32 quest_id)
{
    if (quest_id)
    {
        WorldPacket data(SMSG_QUESTUPDATE_FAILEDTIMER, 4);
        data << quest_id;
        SendPacketToSelf(&data);
        sLog.outDebug("WORLD: Sent SMSG_QUESTUPDATE_FAILEDTIMER");
    }
}

void Player::SendQuestConfirmAccept(const Quest* pQuest, Player* pReceiver)
{
    if (pReceiver)
    {
        int loc_idx = pReceiver->GetSession()->isRussian() ? 0 : pReceiver->GetSession()->GetSessionDbLocaleIndex();
        std::string name = pQuest->GetName();
        sObjectMgr.GetQuestLocaleStrings(pQuest->GetQuestId(), loc_idx, &name);

        WorldPacket data(SMSG_QUEST_CONFIRM_ACCEPT, (4 + name.size() + 8));
        data << uint32(pQuest->GetQuestId());
        data << name;
        data << uint64(GetGUID());
        pReceiver->SendPacketToSelf(&data);

        sLog.outDebug("WORLD: Sent SMSG_QUEST_CONFIRM_ACCEPT");
    }
}

void Player::SendCanTakeQuestResponse(uint32 msg) const
{
    WorldPacket data(SMSG_QUESTGIVER_QUEST_INVALID, 4);
    data << uint32(msg);
    SendPacketToSelf(&data);
    sLog.outDebug("WORLD: Sent SMSG_QUESTGIVER_QUEST_INVALID");
}

void Player::SendPushToPartyResponse(Player *pPlayer, uint32 msg)
{
    if (pPlayer)
    {
        WorldPacket data(MSG_QUEST_PUSH_RESULT, (8+1));
        data << uint64(pPlayer->GetGUID());
        data << uint8(msg);                                 // valid values: 0-8
        SendPacketToSelf(&data);
        sLog.outDebug("WORLD: Sent MSG_QUEST_PUSH_RESULT");
    }
}

void Player::SendQuestUpdateAddItem(Quest const* pQuest, uint32 item_idx, uint32 current, uint32 count)
{
    ASSERT(count < 256 && "Quest slot count store is limited to 8 bits 2^8 = 256 (0..255)");
    WorldPacket data(SMSG_QUESTUPDATE_ADD_ITEM, (4+4));
    sLog.outDebug("WORLD: Sent SMSG_QUESTUPDATE_ADD_ITEM");
    data << pQuest->ReqItemId[item_idx];
    data << count;
    SendPacketToSelf(&data);
    // Update player field and fire UNIT_QUEST_LOG_CHANGED for self
    uint16 slot = FindQuestSlot(pQuest->GetQuestId());

	// DANGEROUS? need to find propper position for item
    //if (slot < MAX_QUEST_LOG_SIZE)
    //    SetQuestSlotCounter(slot, item_idx, GetQuestSlotCounter(slot,item_idx) + current + count);
}

void Player::SendQuestUpdateAddCreatureOrGo(Quest const* pQuest, uint64 guid, uint32 creatureOrGO_idx, uint32 old_count, uint32 add_count)
{
    ASSERT(old_count + add_count < 256 && "mob/GO count store in 8 bits 2^8 = 256 (0..256)");

    int32 entry = pQuest->ReqCreatureOrGOId[ creatureOrGO_idx ];
    if (entry < 0)
        // client expected gameobject template id in form (id|0x80000000)
        entry = (-entry) | 0x80000000;

    WorldPacket data(SMSG_QUESTUPDATE_ADD_KILL, (4*4+8));
    sLog.outDebug("WORLD: Sent SMSG_QUESTUPDATE_ADD_KILL");
    data << uint32(pQuest->GetQuestId());
    data << uint32(entry);
    data << uint32(old_count + add_count);
    data << uint32(pQuest->ReqCreatureOrGOCount[ creatureOrGO_idx ]);
    data << uint64(guid);
    SendPacketToSelf(&data);

    uint16 log_slot = FindQuestSlot(pQuest->GetQuestId());
    if (log_slot < MAX_QUEST_LOG_SIZE)
        SetQuestSlotCounter(log_slot,creatureOrGO_idx,GetQuestSlotCounter(log_slot,creatureOrGO_idx)+add_count);
}

/*********************************************************/
/***                   LOAD SYSTEM                     ***/
/*********************************************************/

void Player::_LoadDeclinedNames(QueryResultAutoPtr result)
{
    if (!result)
        return;

    if (m_declinedname)
        delete m_declinedname;

    m_declinedname = new DeclinedName;
    Field *fields = result->Fetch();
    for (int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        m_declinedname->name[i] = fields[i].GetCppString();
}

void Player::_LoadArenaTeamInfo(QueryResultAutoPtr result)
{
    // arenateamid, played_week, played_season, personal_rating
    memset((void*)&m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1], 0, sizeof(uint32)*18);
    //memset(&_custom1v1ArenaTeamInfo, 0, sizeof(uint32)*6);
    if (!result)
        return;

    do
    {
        Field *fields = result->Fetch();

        uint32 arenateamid     = fields[0].GetUInt32();
        uint32 played_week     = fields[1].GetUInt32();
        uint32 played_season   = fields[2].GetUInt32();
        uint32 personal_rating = fields[3].GetUInt32();

        ArenaTeam* aTeam = sObjectMgr.GetArenaTeamById(arenateamid);
        if (!aTeam)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: FATAL: couldn't load arenateam %u", arenateamid);
            continue;
        }

        if (!aTeam->HaveMember(GetGUID()))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: arenateam %u has no member %u (probably race change)", arenateamid, GetGUIDLow());
            continue;
        }

        uint8  arenaSlot = aTeam->GetSlot();

        /*m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6]     = arenateamid;      // TeamID
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 1] = ((aTeam->GetCaptain() == GetGUID()) ? (uint32)0 : (uint32)1); // Captain 0, member 1
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 2] = played_week;      // Played Week
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 3] = played_season;    // Played Season
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 4] = 0;                // Won season
        m_uint32Values[PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arenaSlot * 6 + 5] = personal_rating;  // Personal Rating
        */
        SetArenaTeamInfoField(arenaSlot, 0, arenateamid);
        SetArenaTeamInfoField(arenaSlot, 1, ((aTeam->GetCaptain() == GetGUID()) ? (uint32)0 : (uint32)1));
        SetArenaTeamInfoField(arenaSlot, 2, played_week);    
        SetArenaTeamInfoField(arenaSlot, 3, played_season);
        SetArenaTeamInfoField(arenaSlot, 4, 0);    
        SetArenaTeamInfoField(arenaSlot, 5, personal_rating);
    }while (result->NextRow());
}

bool Player::LoadPositionFromDB(uint32& mapid, float& x,float& y,float& z,float& o, bool& in_flight, uint64 guid)
{
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map,taxi_path FROM characters WHERE guid = '%u'",GUID_LOPART(guid));
    if (!result)
        return false;

    Field *fields = result->Fetch();

    x = fields[0].GetFloat();
    y = fields[1].GetFloat();
    z = fields[2].GetFloat();
    o = fields[3].GetFloat();
    mapid = fields[4].GetUInt32();
    in_flight = !fields[5].GetCppString().empty();

    return true;
}

bool Player::LoadValuesArrayFromDB(Tokens& data, uint64 guid)
{
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT data FROM characters WHERE guid='%u'",GUID_LOPART(guid));
    if (!result)
        return false;

    Field *fields = result->Fetch();

    data = StrSplit(fields[0].GetCppString(), " ");

    return true;
}

uint32 Player::GetUInt32ValueFromArray(Tokens const& data, uint16 index)
{
    if (index >= data.size())
        return 0;

    return (uint32)atoi(data[index].c_str());
}

float Player::GetFloatValueFromArray(Tokens const& data, uint16 index)
{
    float result;
    uint32 temp = Player::GetUInt32ValueFromArray(data,index);
    memcpy(&result, &temp, sizeof(result));

    return result;
}

uint32 Player::GetUInt32ValueFromDB(uint16 index, uint64 guid)
{
    Tokens data;
    if (!LoadValuesArrayFromDB(data,guid))
        return 0;

    return GetUInt32ValueFromArray(data,index);
}

float Player::GetFloatValueFromDB(uint16 index, uint64 guid)
{
    float result;
    uint32 temp = Player::GetUInt32ValueFromDB(index, guid);
    memcpy(&result, &temp, sizeof(result));

    return result;
}

void Player::LoadFakeBot(uint64 guid)
{
	// see HandleFakeBotLogin

    Object::_Create(guid, 0, HIGHGUID_PLAYER);

    // Lambda function to create a discrete distribution based on weights
    auto discreteDistribution = [](const std::vector<uint32>& weights) {
        // Sum up the weights
        uint32 totalWeight = 0;
        for (uint32 weight : weights) {
            totalWeight += weight;
        }

        // Generate a random number in the range from 0 to the total weight
        std::uniform_int_distribution<uint32> distribution(0, totalWeight - 1);

        std::random_device rd;
        std::mt19937 gen(rd()); // Change to std::mt19937 for the engine
        uint32 randomNumber = distribution(gen); // Remove the extra parentheses

        // Find the value corresponding to the generated number
        uint32 cumulativeWeight = 0;
        for (size_t i = 0; i < weights.size(); ++i) {
            cumulativeWeight += weights[i];
            if (randomNumber < cumulativeWeight) {
                return static_cast<uint32>(i);
            }
        }

        sLog.outLog(LOG_CRITICAL, "LoadFakeBot discreteDistribution error");
        return static_cast<uint32>(1);
    };

    // name
    m_name = sWorld.GetRandomFakeBotName();
    if (m_name.empty())
    {
        sLog.outLog(LOG_CRITICAL,"Can't get name for bot!");
        return;
    }

    // level
    std::vector<uint32> levels;
    std::vector<uint32> weights;
    for (const auto& pair : sWorld.fakebot_levelcount) {
        levels.push_back(pair.first);
        weights.push_back(pair.second);
    }
    uint32 level = levels[discreteDistribution(weights)];

    // any kill instantly gives you 4 levels at level 1, so there will be never 2-4 level characters
    if (sWorld.isEasyRealm() && level > 1 && level < 5)
        level += 4;

    SetUInt32Value(UNIT_FIELD_LEVEL, level);

    std::vector<uint32> probabilities;
    for (int i = 0; i < 10; ++i) {
        probabilities.push_back(race_count[i][1]);
    }
    uint32 race = uint32(race_count[discreteDistribution(probabilities)][0]);
    if (race == 0)
    {
        sLog.outLog(LOG_CRITICAL, "Bot race error!");
        return;
    }

    // set race and class
    std::vector<uint32> secondParams;
    for (int i = 0; i < MAX_RACECLASS_PAIRS; ++i) {
        if (raceclass_pairs[i][0] == race) {
            secondParams.push_back(raceclass_pairs[i][1]);
        }
    }
    uint8 Class = secondParams[urand(0, secondParams.size() - 1)];
    if (Class == 0)
    {
        sLog.outLog(LOG_CRITICAL, "Bot class error!");
        return;
    }

    uint8 gender = 0;

    // overwrite some data fields
    uint32 bytes0 = GetUInt32Value(UNIT_FIELD_BYTES_0) & 0xFF000000;
    bytes0 |= (uint8)race;                         // race
    bytes0 |= Class << 8;                    // class
    bytes0 |= gender << 16;                   // gender

    // set location
    if (sWorld.fakebot_locations.count(level) == 0)
    {
        sLog.outLog(LOG_CRITICAL, "fakebot_locations is empty for level %u", level);
        return;
    }

    fakebot_zone_id = 0;

    // send to heroic dungeon on x100
    if (sWorld.isEasyRealm() && level == 70 && urand(0, 8) == 0)
    {
        fakebot_zone_id = GetRandomHeroicDungeonZone();
    }
    else
    {
        std::multimap<uint32, uint32> multimap = sWorld.fakebot_locations[level];
        auto range = multimap.equal_range(race);
        if (range.first != range.second) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(0, std::distance(range.first, range.second) - 1);
            auto it = range.first;
            std::advance(it, dist(gen));
            fakebot_zone_id = it->second;
        }
    }

    if (!fakebot_zone_id)
    {
        sLog.outLog(LOG_CRITICAL, "fakebot_zone_id is null!");
        return;
    }

    // all in one map - less pain!
    Relocate(16227.07f, 16403.32f, -64.37f, 0.f);
    SetMapId(1);

	//// overwrite possible wrong/corrupted guid
	SetUInt64Value(OBJECT_FIELD_GUID, MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER));
    SetUInt32Value(UNIT_FIELD_BYTES_0, bytes0);

	SetByteValue(PLAYER_BYTES_3, 0, gender);

	m_guid = guid;

    Map* map = sMapMgr.CreateMap(GetMapId(), this);
    if (!map)
    {
        sLog.outLog(LOG_CRITICAL, "Can't create map %u for bot", GetMapId());
        return;
    }

    SetMap(map);

	for (int i = 0; i < PLAYER_SLOTS_COUNT; i++)
		m_items[i] = NULL;

	SetInstanceId(0);

	// clear charm/summon related fields
	SetCharm(NULL);
	SetPet(NULL);
	SetCharmerGUID(0);
	SetOwnerGUID(0);
	SetCreatorGUID(0);
	ClearInCombat();

    QueryResultAutoPtr nullq;
	m_social = sSocialMgr.LoadFromDB(nullq, GetGUIDLow());
}

bool Player::LoadFromDB(uint32 guid, SqlQueryHolder *holder, bool &guildLeft)
{
    QueryResultAutoPtr result = holder->GetResult(PLAYER_LOGIN_QUERY_LOADFROM);

    if (!result)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player (GUID: %u) not found in table `characters`, can't load. ",guid);
        return false;
    }

    Field *fields = result->Fetch();

    uint32 dbAccountId = fields[1].GetUInt32();

    // check if the character's account in the db and the logged in account match.
    // player should be able to load/delete character only with correct account!
    if (dbAccountId != GetSession()->GetAccountId())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player (GUID: %u) loading from wrong account (is: %u, should be: %u)", guid, GetSession()->GetAccountId(), dbAccountId);
        return false;
    }

    Object::_Create(guid, 0, HIGHGUID_PLAYER);

    m_name = fields[3].GetCppString();
	m_guid = guid;

    // if bad name or namebanned
    if (!ObjectMgr::IsValidName(m_name) || sObjectMgr.IsReservedName(m_name, GetSession()->GetAccountId()))
    {
        RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid ='%u'", uint32(AT_LOGIN_RENAME),guid);
        return false;
    }

    if (!LoadValues(fields[2].GetString()))
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player #%d have broken data in `data` field. Can't be loaded.",GUID_LOPART(guid));
        return false;
    }

    // overwrite possible wrong/corrupted guid
    SetUInt64Value(OBJECT_FIELD_GUID, MAKE_NEW_GUID(guid, 0, HIGHGUID_PLAYER));

    // overwrite some data fields
    uint32 bytes0 = GetUInt32Value(UNIT_FIELD_BYTES_0) & 0xFF000000;
    bytes0 |= fields[4].GetUInt8();                         // race
    bytes0 |= fields[5].GetUInt8() << 8;                    // class
    bytes0 |= fields[6].GetUInt8() << 16;                   // gender

    uint32 pBytes = fields[10].GetUInt32();
    uint32 pBytes2 = fields[11].GetUInt32();

    m_atLoginFlags = fields[34].GetUInt32();
    m_activeSpec = fields[48].GetUInt32();

	if (fields[49].GetUInt8())
		GenerateCaptcha(100);

    uint8 gender = fields[6].GetUInt8();

    if (HasAtLoginFlag(AT_LOGIN_DISPLAY_CHANGE))
    {
        std::string name = std::string("Hg") + GetName();
        name[2] = std::tolower(name[2]);
        if (name.length() > 12)
            name.resize(12, '\0');

        uint64 guid = sObjectMgr.GetPlayerGUIDByName(name);
        if (guid && sObjectMgr.GetPlayerAccountIdByGUID(guid) == GetSession()->GetAccountId())
        {
           // & will clear class and rest of stuff
            uint32 newBytes0 = GetUInt32ValueFromDB(UNIT_FIELD_BYTES_0, guid) & 0xFF0000FF;
            // same race, continue
            if ((newBytes0 & 0x000000FF) == fields[4].GetUInt8())
            {
                newBytes0 |= fields[5].GetUInt8() << 8;  // class
                newBytes0 |= bytes0 & 0xFF000000; // powertype

                bytes0 = newBytes0;

                pBytes = GetUInt32ValueFromDB(PLAYER_BYTES, guid);
                pBytes2 = (pBytes2 & 0xFFFFFF00) | (GetUInt32ValueFromDB(PLAYER_BYTES_2, guid) & 0x000000FF);

                gender = (newBytes0 & 0x00FF0000) >> 16;
                uint32 display = GetUInt32ValueFromDB(UNIT_FIELD_NATIVEDISPLAYID, guid);

                SetNativeDisplayId(display);
                SetDisplayId(display);

                DeleteFromDB(guid, GetSession()->GetAccountId(), true);
                m_atLoginFlags = m_atLoginFlags & ~AT_LOGIN_DISPLAY_CHANGE;
                RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login & ~ %u WHERE guid ='%u'", uint32(AT_LOGIN_DISPLAY_CHANGE), GetGUIDLow());
                sLog.outLog(LOG_RACE_CHANGE, "Player: %s [%u] changed character display successfully.", GetName(), GetGUIDLow());
            }
        }
    }
    SetUInt32Value(UNIT_FIELD_BYTES_0, bytes0); // race and class are set, can load homebind

    uint8 changeRaceTo = fields[43].GetUInt8(); // used in LoadHomeBind and later for actual race change

    if (changeRaceTo)
    {
        // m_team is not yet set. Determine by TeamForRace. Race was set just above
        uint32 oldTeamCheck = TeamForRace(GetRace());
        uint32 newTeamCheck = TeamForRace(changeRaceTo);
        // Deal with the guild and arena teams on faction change
        if (oldTeamCheck != newTeamCheck)
        {
            if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD))
            {
                // guild is loaded after character load, so need to know if guild exists at this moment by db query
                QueryResultAutoPtr resultGuild = holder->GetResult(PLAYER_LOGIN_QUERY_LOADGUILD);
                if (resultGuild)
                {
                    Field *fieldsG = resultGuild->Fetch();
                    if (uint32 guildId = fieldsG[0].GetUInt32())
                    {
                        if (Guild *guild = sGuildMgr.GetGuildById(guildId))
                        {
                            if (GetGUID() == guild->GetLeader())
                            {
                                // do error if not alone in guild. If alone -> simply allow faction change without guild leave
                                if (guild->GetMemberSize() > 1)
                                {
                                    GetSession()->SendGuildCommandResult(GUILD_QUIT_S, "", GUILD_LEADER_LEAVE);
                                    // if error - save m_changeRaceTo, so it will be available again at next login
                                    m_changeRaceTo = changeRaceTo;
                                    // do NOT change race at THIS login!
                                    changeRaceTo = 0;
                                }
                            }
                            // just leave guild without any errors
                            else
                            {
                                guildLeft = true;
                                guild->DelMember(GetGUID());
                                // Put record into guildlog
                                guild->LogGuildEvent(GUILD_EVENT_LOG_LEAVE_GUILD, GetGUIDLow(), 0, 0);

                                WorldPacket data(SMSG_GUILD_EVENT, (2 + 10));             // guess size
                                data << (uint8)GE_LEFT;
                                data << (uint8)1;
                                data << GetName();
                                guild->BroadcastPacket(&data);

                                //sLog.outDebug("WORLD: Sent (SMSG_GUILD_EVENT)");

                                GetSession()->SendGuildCommandResult(GUILD_QUIT_S, guild->GetName(), GUILD_PLAYER_NO_MORE_IN_GUILD);
                            }
                        }
                    }
                }
            }

            if (!sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_ARENA))
            {
                // arena is loaded after character load, so need to know if teams exist at this moment by db query. Can't use holder query cause gonna need to call NextRow()
                QueryResultAutoPtr resultArena = RealmDataDatabase.PQuery("SELECT arenateamid WHERE guid = '%u'", GetGUIDLow());
                if (resultArena)
                {
                    do
                    {
                        Field *fieldsA = resultArena->Fetch();
                        uint32 arenateamid = fieldsA[0].GetUInt32();

                        ArenaTeam* aTeam = sObjectMgr.GetArenaTeamById(arenateamid);
                        if (!aTeam)
                        {
                            sLog.outLog(LOG_DEFAULT, "ERROR: FATAL: couldn't find arenateam on race change. Team %u", arenateamid);
                            continue;
                        }

                        if (aTeam->CantLeave())
                        {
                            // if error - save m_changeRaceTo, so it will be available again at next login
                            m_changeRaceTo = changeRaceTo;
                            // do NOT change race at THIS login!
                            changeRaceTo = 0;
                            break;
                        }

                        if (aTeam->GetCaptain() == GetGUID())
                        {
                            if (aTeam->GetMembersSize() > 1)
                            {
                                // check for correctness
                                GetSession()->SendArenaTeamCommandResult(ERR_ARENA_TEAM_QUIT_S, "", "", ERR_ARENA_TEAM_LEADER_LEAVE_S);

                                // if error - save m_changeRaceTo, so it will be available again at next login
                                m_changeRaceTo = changeRaceTo;
                                // do NOT change race at THIS login!
                                changeRaceTo = 0;
                                break;
                            }
                        }
                        else
                            aTeam->DelMember(GetGUID());
                    } while (resultArena->NextRow());
                }
            }
        }
    }

    if (!_LoadHomeBind(changeRaceTo, holder->GetResult(PLAYER_LOGIN_QUERY_LOADHOMEBIND)))
        return false;

    SetUInt32Value(UNIT_FIELD_LEVEL, fields[7].GetUInt8());
    SetUInt32Value(PLAYER_XP, fields[8].GetUInt32());
    SetUInt32Value(PLAYER_FIELD_COINAGE, fields[9].GetUInt32());
    SetUInt32Value(PLAYER_BYTES, pBytes);
    SetUInt32Value(PLAYER_BYTES_2, pBytes2);
    SetUInt32Value(PLAYER_BYTES_3, (GetUInt32Value(PLAYER_BYTES_3) & ~1));
    SetByteValue(PLAYER_BYTES_3, 0, gender);

    SetUInt32Value(PLAYER_FLAGS, fields[12].GetUInt32());

    // cleanup inventory related item value fields (its will be filled correctly in _LoadInventory)
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        SetUInt64Value((uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2)), 0);
        SetVisibleItemSlot(slot, NULL);

        if (m_items[slot])
        {
            delete m_items[slot];
            m_items[slot] = NULL;
        }
    }

    // update money limits
    if (GetMoney() > MAX_MONEY_AMOUNT)
        SetMoney(MAX_MONEY_AMOUNT);

    sLog.outDebug("Load Basic value of player %s is: ", m_name.c_str());
    outDebugValues();

    // Need to call it to initialize m_team (m_team can be calculated from race)
    // Other way is to saves m_team into characters table.
    m_team = TeamForRace(GetRace());
    // restoreFaction will be called after BG load -> with different faction BG's we need to know if a player is on BG
    SetCharm(NULL);

    InitPrimaryProffesions();                               // to max set before any spell loaded

    uint32 transGUID = fields[31].GetUInt32();
    Relocate(fields[13].GetFloat(),fields[14].GetFloat(),fields[15].GetFloat(),fields[17].GetFloat());
    SetMapId(fields[16].GetUInt32());
    SetFallInformation(0, fields[15].GetFloat());
    SetInstanceId(fields[41].GetUInt32());
    SetDifficulty(fields[39].GetUInt32());                  // may be changed in _LoadGroup
    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, fields[44].GetUInt32());
    SetUInt16Value(PLAYER_FIELD_KILLS, 0, fields[45].GetUInt16()); // todaykills
    SetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY, fields[46].GetUInt32());

    _LoadGroup(holder->GetResult(PLAYER_LOGIN_QUERY_LOADGROUP));

    _LoadArenaTeamInfo(holder->GetResult(PLAYER_LOGIN_QUERY_LOADARENAINFO));

    uint32 arena_currency = fields[40].GetUInt32();
    if (arena_currency > sWorld.getConfig(CONFIG_MAX_ARENA_POINTS))
        arena_currency = sWorld.getConfig(CONFIG_MAX_ARENA_POINTS);

    SetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY, arena_currency);

    // check arena teams integrity
    for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
    {
        uint32 arena_team_id = GetArenaTeamId(arena_slot);
        if (!arena_team_id)
            continue;

        if (ArenaTeam * at = sObjectMgr.GetArenaTeamById(arena_team_id))
            if (at->HaveMember(GetGUID()))
                continue;

        // arena team not exist or not member, cleanup fields
        for (int j = 0; j < 6; ++j)
            SetArenaTeamInfoField(arena_slot, j, 0);
            // SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + arena_slot * 6 + j, 0);
    }

    _LoadBoundInstances(holder->GetResult(PLAYER_LOGIN_QUERY_LOADBOUNDINSTANCES));

    if (!IsPositionValid())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: Player (guidlow %d) have invalid coordinates (X: %f Y: %f Z: %f O: %f). Teleport to default race/class locations.",guid,GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());

        RelocateToHomebind();

        transGUID = 0;

        m_movementInfo.ClearTransportData();
    }

    ////                                                            0     1       2      3    4    5    6
    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT bgid, bgteam, bgmap, bgx, bgy, bgz, bgo FROM character_bgcoord WHERE guid = '%u'", GUID_LOPART(m_guid));
    QueryResultAutoPtr resultbg = holder->GetResult(PLAYER_LOGIN_QUERY_LOADBGCOORD);

    if (resultbg)
    {
        Field *fieldsbg = resultbg->Fetch();

        uint32 bgid = fieldsbg[0].GetUInt32();
        PlayerTeam bgteam = PlayerTeam(fieldsbg[1].GetUInt32());

        if (bgid) //saved in BattleGround
        {
            WorldLocation loc(fieldsbg[2].GetUInt32(), fieldsbg[3].GetFloat(), fieldsbg[4].GetFloat(), fieldsbg[5].GetFloat(), fieldsbg[6].GetFloat());
            SetBattleGroundEntryPoint(loc);

            BattleGround *currentBg = sBattleGroundMgr.GetBattleGround(bgid, BATTLEGROUND_TYPE_NONE);

            if (currentBg && currentBg->IsPlayerInBattleGround(GetGUID()))
            {
                BattleGroundQueueTypeId bgQueueTypeId = sBattleGroundMgr.BGQueueTypeId(currentBg->GetTypeID(), currentBg->GetArenaType());
                uint32 queueSlot = AddBattleGroundQueueId(bgQueueTypeId);

                SetBattleGroundId(currentBg->GetBgInstanceId(), currentBg->GetTypeID());
                SetBGTeam(bgteam);
                currentBg->AddOrSetPlayerToCorrectBgGroup(this, GetGUID(), bgteam);

                SetInviteForBattleGroundQueueType(bgQueueTypeId,currentBg->GetBgInstanceId());
            }
            else
            {
                WorldLocation const& loc = GetBattleGroundEntryPoint();
                SetMapId(loc.mapid);
                Relocate(loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation);

                //RemoveArenaAuras(true);
                if (!isAlive())// resurrect on bg exit
                {
                    ResurrectPlayer(1.0f);
                    SpawnCorpseBones();
                }
            }
        }
    }

    RestoreFaction();

    if (transGUID != 0)
    {
        m_movementInfo.SetTransportData(transGUID, fields[27].GetFloat(), fields[28].GetFloat(), fields[29].GetFloat(), fields[30].GetFloat(), 0);

        if (!Hellground::IsValidMapCoord(
            GetPositionX() + m_movementInfo.GetTransportPos()->x, GetPositionY() + m_movementInfo.GetTransportPos()->y,
            GetPositionZ() + m_movementInfo.GetTransportPos()->z, GetOrientation() + m_movementInfo.GetTransportPos()->o) ||

            // transport size limited
            m_movementInfo.GetTransportPos()->x > 50 || m_movementInfo.GetTransportPos()->y > 50 || m_movementInfo.GetTransportPos()->z > 50)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: Player (guidlow %d) have invalid transport coordinates (X: %f Y: %f Z: %f O: %f). Teleport to default race/class locations.",
                 guid, GetPositionX() + m_movementInfo.GetTransportPos()->x, GetPositionY() + m_movementInfo.GetTransportPos()->y,
                GetPositionZ() + m_movementInfo.GetTransportPos()->z, GetOrientation() + m_movementInfo.GetTransportPos()->o);

            RelocateToHomebind();

            m_movementInfo.ClearTransportData();

            transGUID = 0;
        }
    }

    if (transGUID != 0)
    {
        for (MapManager::TransportSet::iterator iter = sMapMgr.m_Transports.begin(); iter != sMapMgr.m_Transports.end(); ++iter)
        {
            if ((*iter)->GetGUIDLow() == transGUID)
            {
                MapEntry const* transMapEntry = sMapStore.LookupEntry((*iter)->GetMapId());
                // client without expansion support
                if(!transMapEntry || GetSession()->Expansion() < transMapEntry->Expansion())
                {
                    sLog.outDebug("Player %s using client without required expansion tried login at transport at non accessible map %u, or there is no transport", GetName(), (*iter)->GetMapId());
                    break;
                }

                m_transport = *iter;
                m_transport->AddPassenger(this);
                SetMapId(m_transport->GetMapId());
                break;
            }
        }

        if (!m_transport)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: Player (guidlow %d) have problems with transport guid (%u). Teleport to default race/class locations.",
                guid,transGUID);

            RelocateToHomebind();

            m_movementInfo.ClearTransportData();

            transGUID = 0;
        }
    }
    else                                                    // not transport case
    {
        MapEntry const* mapEntry = sMapStore.LookupEntry(GetMapId());
        // client without expansion support
        if (!mapEntry || GetSession()->Expansion() < mapEntry->Expansion())
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Player %s (%u acc: %u) using client without required expansion tried login at non accessible map %u", GetName(), GetGUIDLow(), GetSession()->GetAccountId(), GetMapId());
            RelocateToHomebind();
        }
    }

    if (changeRaceTo)
        RelocateToHomebind();

    if (InstanceSave *pSave = GetInstanceSave(GetMapId()))
        if (pSave->GetSaveInstanceId() != GetInstanciableInstanceId())
            SetInstanceId(pSave->GetSaveInstanceId());

    // load the player's map here if it's not already loaded
    SetMap(sMapMgr.CreateMap(GetMapId(), this));

    Map *map = GetMap();
    if (!map)
    {
        AreaTrigger const* at = sObjectMgr.GetGoBackTrigger(GetMapId());
        if (at)
        {
            SetMapId(at->target_mapId);
            Relocate(at->target_X, at->target_Y, at->target_Z, GetOrientation());
            sLog.outLog(LOG_DEFAULT, "ERROR: Player (guidlow %d) is teleported to gobacktrigger (Map: %u X: %f Y: %f Z: %f O: %f).",guid,GetMapId(),GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());
        }
        else
        {
            SetMapId(m_homebindMapId);
            Relocate(m_homebindX, m_homebindY, m_homebindZ, GetOrientation());
            sLog.outLog(LOG_DEFAULT, "ERROR: Player (guidlow %d) is teleported to home (Map: %u X: %f Y: %f Z: %f O: %f).",guid,GetMapId(),GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());
        }

        map = sMapMgr.CreateMap(GetMapId(), this);
        if (!map)
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: ERROR: Player (guidlow %d) have invalid coordinates (X: %f Y: %f Z: %f O: %f). Teleport to default race/class locations.",guid,GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());

            RelocateToHomebind();

            map = sMapMgr.CreateMap(GetMapId(), this);
            if (!map)
            {
                sLog.outLog(LOG_CRASH, "ERROR: ERROR: Player (guidlow %d) have invalid coordinates (X: %f Y: %f Z: %f O: %f). Teleport to default race/class locations.",guid,GetPositionX(),GetPositionY(),GetPositionZ(),GetOrientation());
                sLog.outLog(LOG_CRASH, "ERROR: CRASH.");
                ASSERT(false);
            }
        }
    }

    SetInstanceId(map->GetAnyInstanceId());

    time_t logoutTime = time_t(fields[23].GetUInt64());

    if (GetInstanciableInstanceId())
    {
        if (map->IsBattleGroundOrArena())
        {
            if (!sBattleGroundMgr.GetBattleGround(GetInstanciableInstanceId(), BATTLEGROUND_TYPE_NONE))
            {
                WorldLocation const& loc = GetBattleGroundEntryPoint();
                SetMapId(loc.mapid);
                Relocate(loc.coord_x, loc.coord_y, loc.coord_z, loc.orientation);

                RemoveFromBattleGroundRaid();
                map = sMapMgr.CreateMap(GetMapId(), this);
            }
        }
        else if (!sInstanceSaveManager.GetInstanceSave(GetInstanciableInstanceId()))
        {
            PlayerInfo const * tmpPlInfo = sObjectMgr.GetPlayerInfo(GetRace(), GetClass());
            if (tmpPlInfo)
            {
                SetMapId(tmpPlInfo->mapId);
                Relocate(tmpPlInfo->positionX, tmpPlInfo->positionY, tmpPlInfo->positionZ);
            }
            else
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Player %s(GUID: %u) logged in to a reset instance (map: %u) and there is no area-trigger leading to this map. Thus he can't be ported back to the entrance. This _might_ be an exploit attempt. Relocate to homebind.", GetName(), GetGUIDLow(), GetMapId());
                RelocateToHomebind();
            }

            map = sMapMgr.CreateMap(GetMapId(), this);
        }
		else
		{
		    // logout in resetted instance fix
		    if (time_t resetTime = sInstanceSaveManager.GetResetTimefor(GetMapId(), GetMap()->IsHeroic()))
		    {
		        if (InstanceTemplate const* temp = sObjectMgr.GetInstanceTemplate(GetMapId()))
		        {
		            uint32 reset = (GetMap()->IsHeroic()) ? temp->reset_delay_heroic : temp->reset_delay_raid;
		
		            if (reset && logoutTime < resetTime - reset * DAY)
		            {
		                RelocateToHomebind();
		                map = sMapMgr.CreateMap(GetMapId(), this);
		            }
		        }
		    }
		}
    }

    SetMap(map);

    SaveRecallPosition();

    // since last logout (in seconds)
    time_t now = time(NULL);
    uint64 time_diff = uint64(now - logoutTime);

    // set value, including drunk invisibility detection
    // calculate sobering. after 15 minutes logged out, the player will be sober again
    float soberFactor;
    if (time_diff > 15*MINUTE)
        soberFactor = 0;
    else
        soberFactor = 1-time_diff/(15.0f*MINUTE);
    uint16 newDrunkenValue = uint16(soberFactor*(GetUInt32Value(PLAYER_BYTES_3) & 0xFFFE));
    SetDrunkValue(newDrunkenValue);

    m_rest_bonus = fields[22].GetFloat();
    //speed collect rest bonus in offline, in logout, far from tavern, city (section/in hour)
    float bubble0 = 0.031;
    //speed collect rest bonus in offline, in logout, in tavern, city (section/in hour)
    float bubble1 = 0.125;

    m_ChCustomFlags = fields[47].GetUInt16();

    if ((int32)fields[23].GetUInt32() > 0)
    {
        float bubble = fields[24].GetUInt32() > 0
            ? bubble1 * (!IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1) ? sWorld.getConfig(RATE_REST_OFFLINE_IN_TAVERN_OR_CITY) : 1.0f)
            : bubble0 * (!IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1) ? sWorld.getConfig(RATE_REST_OFFLINE_IN_WILDERNESS) : 1.0f);

        SetRestBonus(GetRestBonus()+ time_diff*((float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP)/72000)*bubble);
    }

    m_cinematic = fields[19].GetBool();
    m_Played_time[PLAYED_TIME_TOTAL]= fields[20].GetUInt32();
    m_Played_time[PLAYED_TIME_LEVEL]= fields[21].GetUInt32();
    m_resetTalentsCost = fields[25].GetUInt32();
    m_resetTalentsTime = time_t(fields[26].GetUInt64());

    // reserve some flags
    uint32 old_safe_flags = GetUInt32Value(PLAYER_FLAGS) & (PLAYER_FLAGS_HIDE_CLOAK | PLAYER_FLAGS_HIDE_HELM);

    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM))
        SetUInt32Value(PLAYER_FLAGS, 0 | old_safe_flags);

    m_taxi.LoadTaxiMask(fields[18].GetString());          // must be before InitTaxiNodesForLevel

    uint32 extraflags = fields[32].GetUInt32();

    m_stableSlots = fields[33].GetUInt32();
    if (m_stableSlots > 2)
    {
        //sLog.outLog(LOG_SPECIAL,"StablePetLog: Player.cpp: 1, PlayerGUID %u", GetGUIDLow());
        sLog.outLog(LOG_DEFAULT, "ERROR: Player can have not more 2 stable slots, but have in DB %u",uint32(m_stableSlots));
        m_stableSlots = 2;
    }

    // Honor system
    // Update Honor kills data
    m_lastHonorUpdateTime = logoutTime;
    UpdateHonorFields();

    m_deathExpireTime = (time_t)fields[37].GetUInt64();
    if (m_deathExpireTime > now+MAX_DEATH_COUNT*DEATH_EXPIRE_STEP)
        m_deathExpireTime = now+MAX_DEATH_COUNT*DEATH_EXPIRE_STEP-1;

    std::string taxi_nodes = fields[38].GetCppString();

    // clear channel spell data (if saved at channel spell casting)
    SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
    SetUInt32Value(UNIT_CHANNEL_SPELL,0);

    // clear charm/summon related fields
    SetCharm(NULL);
    SetPet(NULL);
    SetCharmerGUID(0);
    SetOwnerGUID(0);
    SetCreatorGUID(0);

    // reset some aura modifiers before aura apply
    SetFarSight(0);
    SetUInt32Value(PLAYER_TRACK_CREATURES, 0);
    SetUInt32Value(PLAYER_TRACK_RESOURCES, 0);

    // reset skill modifiers and set correct unlearn flags
    for (uint32 i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i),0);

        // set correct unlearn bit
        uint32 id = GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF;
        if (!id) continue;

        SkillLineEntry const *pSkill = sSkillLineStore.LookupEntry(id);
        if (!pSkill) continue;

        // enable unlearn button for primary professions only
        if (pSkill->categoryId == SKILL_CATEGORY_PROFESSION)
            SetUInt32Value(PLAYER_SKILL_INDEX(i), MAKE_PAIR32(id,1));
        else
            SetUInt32Value(PLAYER_SKILL_INDEX(i), MAKE_PAIR32(id,0));
    }

    // make sure the unit is considered out of combat for proper loading
    ClearInCombat();

    // make sure the unit is considered not in duel for proper loading
    SetUInt64Value(PLAYER_DUEL_ARBITER, 0);
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    // remember loaded power/health values to restore after stats initialization and modifier applying
    uint32 savedHealth = GetHealth();
    uint32 savedPower[MAX_POWERS];
    for (uint32 i = 0; i < MAX_POWERS; ++i)
        savedPower[i] = GetPower(Powers(i));

    // reset stats before loading any modifiers
    InitStatsForLevel();
    InitTaxiNodesForLevel();

    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()

    // Mail

    _LoadMails(holder->GetResult(PLAYER_LOGIN_QUERY_LOADMAILS));
    _LoadMailedItems(holder->GetResult(PLAYER_LOGIN_QUERY_LOADMAILEDITEMS));
    UpdateNextMailTimeAndUnreads();

    _LoadAuras(holder->GetResult(PLAYER_LOGIN_QUERY_LOADAURAS), time_diff);

    m_GrantableLevelsCount = fields[42].GetUInt32();

    // refer-a-friend flag - maybe wrong and hacky
    //LoadAccountLinkedState();
    if (GetSession()->IsRaf())
        SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_REFER_A_FRIEND);

    SetCanGrantLevelsFlagIfNeeded();

    // add ghost flag (must be after aura load: PLAYER_FLAGS_GHOST set in aura)
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        m_deathState = DEAD;
    
    _LoadTalents(holder->GetResult(PLAYER_LOGIN_QUERY_LOADTALENTS));
    _LoadSpells(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSPELLS));

    //Ugly hacky one - give summon friend spell to players created before RAF implement
    if (GetSession()->IsRaf())
        learnSpell(45927);

    // after spell load
    InitTalentForLevel();
    learnSkillRewardedSpells();

    if (changeRaceTo)
    {
        uint8 oldRace = GetRace();

        ChangeRace(changeRaceTo);
        RealmDataDatabase.PExecute("UPDATE characters SET changeRaceTo = '0' WHERE guid ='%u'", GetGUIDLow());

        m_reputationMgr.LoadFromDB(holder->GetResult(PLAYER_LOGIN_QUERY_LOADREPUTATION), oldRace);
        _LoadQuestStatus(holder->GetResult(PLAYER_LOGIN_QUERY_LOADQUESTSTATUS), oldRace);
    }
    else
    {
        // must be before inventory (some items required reputation check)
        // must be after ChangeRace
        m_reputationMgr.LoadFromDB(holder->GetResult(PLAYER_LOGIN_QUERY_LOADREPUTATION));

        // after spell load, learn rewarded spell if need also
        _LoadQuestStatus(holder->GetResult(PLAYER_LOGIN_QUERY_LOADQUESTSTATUS));
    }

    _LoadDailyQuestStatus(holder->GetResult(PLAYER_LOGIN_QUERY_LOADDAILYQUESTSTATUS));

    _LoadTutorials(holder->GetResult(PLAYER_LOGIN_QUERY_LOADTUTORIALS));

    GetTransmogManager()->LoadTransmog();

    _LoadInventory(holder->GetResult(PLAYER_LOGIN_QUERY_LOADINVENTORY), time_diff);

    // update items with duration and realtime
    UpdateItemDuration(time_diff, true);

    // After quests and inventory loaded: force update on quest item counters
    // Fixes cases when quest status data was not correctly saved during last session (crash, db connection lost, etc)
    for (auto& data : mQuestStatus)
    {
        if (Quest const* quest = sObjectMgr.GetQuestTemplate(data.first))
            AdjustQuestReqItemCount(quest, data.second);
    }

    //_LoadActions(holder->GetResult(PLAYER_LOGIN_QUERY_LOADACTIONS));

    QueryResultAutoPtr actionResult = RealmDataDatabase.PQuery("SELECT button, action, type, misc FROM character_action WHERE guid = '%u' AND spec = '%u' ORDER BY button", GetGUIDLow(), m_activeSpec);
    _LoadActions(actionResult);

    m_social = sSocialMgr.LoadFromDB(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSOCIALLIST), GetGUIDLow());

    // check PLAYER_CHOSEN_TITLE compatibility with PLAYER__FIELD_KNOWN_TITLES
    // note: PLAYER__FIELD_KNOWN_TITLES updated at quest status loaded
    /*SetUInt32Value(PLAYER__FIELD_KNOWN_TITLES, (GetUInt32Value(PLAYER__FIELD_KNOWN_TITLES) & ~PLAYER_TITLE_PVP));
    if (uint32 curTitle = GetUInt32Value(PLAYER_CHOSEN_TITLE))
    {
        if (!HasTitle(curTitle))
            SetUInt32Value(PLAYER_CHOSEN_TITLE,0);
    }*/ // This thing removed all PVP titles

    // Not finish taxi flight path
    {
        bool taxiOk = m_taxi.LoadTaxiDestinationsFromString(taxi_nodes, GetMapId());
        // do a SaveRecallPosition (which finds safe recall position) and teleport to its location
        // SaveRecallPosition must be called after LoadTaxiDestinationsFromString in order to find taxi source node
        SaveRecallPosition();                           // save as recall also to prevent recall and fall from sky
        
        // problems with taxi path loading -> do a teleport
        if (!taxiOk)
        {
            SetMapId(_recallPosition.mapid);
            Relocate(_recallPosition.coord_x, _recallPosition.coord_y, _recallPosition.coord_z, _recallPosition.orientation);

            SetMap(sMapMgr.CreateMap(GetMapId(), this));
            CleanupAfterTaxiFlight();
        }
    }

    _LoadSpellCooldowns(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSPELLCOOLDOWNS));

    // Spell code allow apply any auras to dead character in load time in aura/spell/item loading
    // Do now before stats re-calculation cleanup for ghost state unexpected auras
    if (!isAlive())
        RemoveAllAurasOnDeath();

    //apply all stat bonuses from items and auras
    SetCanModifyStats(true);
    UpdateAllStats();

    // prevent restoring wrong HP for ghosts
    if (HasAura(20584) || HasAura(8326))
        savedHealth = 1;

    // restore remembered power/health values (but not more max values)
    SetHealth(savedHealth > GetMaxHealth() ? GetMaxHealth() : savedHealth, true);

    for (uint32 i = 0; i < MAX_POWERS; ++i)
        SetPower(Powers(i),savedPower[i] > GetMaxPower(Powers(i)) ? GetMaxPower(Powers(i)) : savedPower[i]);

    sLog.outDebug("The value of player %s after load item and aura is: ", m_name.c_str());
    outDebugValues();

    // GM state
    if (GetSession()->HasPermissions(PERM_GMT_DEV))
    {
        switch (sWorld.getConfig(CONFIG_GM_LOGIN_STATE))
        {
        default:
        case 0:                      break;             // disable
        case 1: SetGameMaster(true); break;             // enable
        case 2:                                         // save state
            if (extraflags & PLAYER_EXTRA_GM_ON)
                SetGameMaster(true);
            break;
        }
    }

    if (GetSession()->HasPermissions(PERM_GMT_HDEV))
    {
        // on player create there's a check for "GetSession()->HasPermissions(PERM_GMT_HDEV)"
        // everyone but PERM_GMT_HDEV gets "SetAcceptWhispers(true)";
        switch (sWorld.getConfig(CONFIG_GM_WISPERING_TO))
        {
            default:
            case 0:
                SetAcceptWhispers(false);
                break;         // disable
            case 1:
                SetAcceptWhispers(true);
                break;         // enable
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_ACCEPT_WHISPERS)
                    SetAcceptWhispers(true);
                // gotta also check "sSocialMgr.shouldHavePartialWhisperFlag(GetGUIDLow())" cause it clears at server restart.
                else if ((extraflags & PLAYER_EXTRA_PARTIAL_WHISPER) && sSocialMgr.shouldHavePartialWhisperFlag(GetGUIDLow()))
                {
                    setPartialWhispers(true);
                    ChatHandler(this).SendSysMessage("You have someone in your partial list whisper acceptor.");
                }
                break;
        }
    }

    if (GetSession()->HasPermissions(PERM_GMT))
    {
        switch (sWorld.getConfig(CONFIG_GM_VISIBLE_STATE))
        {
            default:
            case 0: SetGMVisible(false); break;             // invisible
            case 1:                      break;             // visible
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_GM_INVISIBLE)
                    SetGMVisible(false);
                break;
        }

        switch (sWorld.getConfig(CONFIG_GM_CHAT))
        {
            default:
            case 0:                  break;                 // disable
            case 1: SetGMChat(true); break;                 // enable
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_GM_CHAT)
                    SetGMChat(true);
                break;
        }
    }

    if ((GetSession()->GetPermissions() >= PERM_GM_TRIAL) && !GetSession()->HasPermissions(PERM_ADM))
        sTicketMgr.AddOrUpdateGm(GetSession(), GetGUID());

    _LoadDeclinedNames(holder->GetResult(PLAYER_LOGIN_QUERY_LOADDECLINEDNAMES));

    if (GetClass() == CLASS_SHAMAN && GetRace() == RACE_DRAENEI)
    {
        if (!HasAura(28878,0))
            learnSpell(28878);

        removeSpell(6562);
    }

    // save retarded rogues from pain
    if (GetClass() == CLASS_ROGUE && GetLevel() == 70)
    {
        if (!HasSpell(1787))
            learnSpell(1787);
    }

    //HACK restore Netherwing aura
    if (!HasAura(40214) && m_reputationMgr.GetRank(1015) >= REP_NEUTRAL && GetQuestRewardStatus(10870))
        CastSpell(this, 40214, true);

    //HACK restore Arcane Cloaking aura (Naxx attu)
    if (!HasAura(28006) && (GetQuestRewardStatus(9121) || GetQuestRewardStatus(9122) || GetQuestRewardStatus(9123)))
        CastSpell(this, 28006, true);

    // remove auras
    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING);

    if (map->IsRaid())
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_SPELL_ATTR_EX6_NOT_IN_RAID_INSTANCE);

    if (GetClass() == CLASS_ROGUE && map->IsDungeon())
    {
        if (!HasAura(SPELL_ROGUE_RAID_BUFF))
            AddAuraCreate(SPELL_ROGUE_RAID_BUFF, this);
    }
    else
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ONLY_IN_RAID);

    SetSpectator(false);

    if (IsTaxiFlying())
        m_anti_ontaxipath = true;
    else
        m_anti_ontaxipath = false;
    result = holder->GetResult(PLAYER_LOGIN_QUERY_LOADDAILYARENA);
    if (result)
        m_DailyArenasWon = result->Fetch()->GetUInt16();

    LoadRaidChestInfo();

    RewardRAF();

    GivePremiumItemIfNeeded();

    // First learn any custom spells if set
    QueryResultAutoPtr resultLearn = RealmDataDatabase.PQuery("SELECT `spell`, `skill_level` FROM `character_login_learn` WHERE `guid` = '%u'", GetGUIDLow());
    if (resultLearn)
    {
        do
        {
            Field *fields = resultLearn->Fetch();
            uint32 spellId = fields[0].GetUInt32();
            uint16 skill_level = fields[1].GetUInt16();

            // check if valid
            SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
            if (!spellproto)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: character_login_learn spellId %u does not exist, tried to learn for character %u, ignore.", spellId, GetGUIDLow());
                continue;
            }

            // learn spell
            learnSpell(spellId);

            // if skill_level is set -> get skill number from the spell and set it to specified value
            if (skill_level)
            {
                uint8 eff = 3;
                for (uint32 i = 0; i < 3; ++i)
                {
                    if (spellproto->Effect[i] == SPELL_EFFECT_SKILL)
                    {
                        eff = i;
                        break;
                    }
                }
                if (eff == 3)
                    sLog.outLog(LOG_DEFAULT, "ERROR: character_login_learn spellId %u has skill_level specified, but no skill found for character %u, ignore.", spellId, GetGUIDLow());
                else
                {
                    uint32 skillId = spellproto->EffectMiscValue[eff];
                    uint16 max = GetPureMaxSkillValue(skillId);
                    SetSkill(skillId, skill_level, max);
                }
            }
        } while (resultLearn->NextRow());

        // delete learned data
        RealmDataDatabase.PExecute("DELETE FROM `character_login_learn` WHERE `guid` = '%u'", GetGUIDLow());
    }

    if (IsPlayerCustomFlagged(PL_CUSTOM_LOGIN_LEARN_SPELLS))
    {
        LearnAllSpells(true, false);
        SetWeaponSkillsToMax();
        RemovePlayerCustomFlag(PL_CUSTOM_LOGIN_LEARN_SPELLS);
    }

    QueryResultAutoPtr resultCustomData = RealmDataDatabase.PQuery("SELECT `souls_quests_done` FROM `character_custom_data` WHERE `guid` = '%u'", GetGUIDLow());
    if (resultCustomData)
    {
        CharacterCustomData ccd;
        ccd.souls_quests_done = (*resultCustomData)[0].GetUInt32();
        custom_data = ccd;
    }

    debug_info = { GetGUIDLow(), 0, GetName() };

    return true;
}

bool Player::isAllowedToLoot(Creature* creature)
{
    // loot check for allowed also here
    if (creature->isDead() && !creature->IsDamageEnoughForLootingAndReward())
        return false;

    if (Player* recipient = creature->GetLootRecipient())
    {
		if (Group* otherGroup = recipient->GetGroup())
		{
			Group* thisGroup = GetGroup();

			if (!thisGroup)
				return false;

			if (thisGroup != otherGroup)
				return false;

			if (!creature->IsPlayerAllowedToLoot(this))
				return false;

			// if cant loot LOOT - return false;

			if (creature->isAlive())
				return true;

			if (!thisGroup->IsRoundRobinLootType())
				return true;

			// round robin rules
			if (!creature->loot.looterGUID)
				return true;

			ItemQualities threshold = thisGroup->GetLootMethod() == ROUND_ROBIN ? ITEM_QUALITY_ARTIFACT : thisGroup->GetLootThreshold();
			if (creature->loot.everyone_can_open || creature->loot.max_quality >= threshold)
				return true;

			return GetGUID() == creature->loot.looterGUID;
		}
		else if (recipient == this)
			return true;
		// was participated in battle
		else if (!creature->m_PlayersAllowedToLoot.empty() && creature->m_PlayersAllowedToLoot.find(GetGUID()) != creature->m_PlayersAllowedToLoot.end())
			return true;
        else
            return false;
    }
    else
    {
        // recipient may be offline, maybe there is list of players allowed to loot
        if (creature->HasPlayersAllowedToLoot() && creature->IsPlayerAllowedToLoot(this))
            return true;

        // prevent other players from looting if the recipient got disconnected
        return !creature->hasLootRecipient();
    }
}

void Player::_LoadActions(QueryResultAutoPtr result)
{
    m_actionButtons.clear();

    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT button,action,type,misc FROM character_action WHERE guid = '%u' ORDER BY button",GetGUIDLow());

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint8 button = fields[0].GetUInt8();

            addActionButton(button, fields[1].GetUInt16(), fields[2].GetUInt8(), fields[3].GetUInt8());

            m_actionButtons[button].uState = ACTIONBUTTON_UNCHANGED;
        }
        while (result->NextRow());
    }
}

void Player::_LoadAuras(QueryResultAutoPtr result, uint32 timediff)
{
    m_Auras.clear();
    for (int i = 0; i < TOTAL_AURAS; i++)
        m_modAuras[i].clear();

    // all aura related fields
    for (int i = UNIT_FIELD_AURA; i <= UNIT_FIELD_AURASTATE; ++i)
        SetUInt32Value(i, 0);

    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT caster_guid,item_guid,spell,effect_index,stackcount,amount,maxduration,remaintime,remaincharges FROM character_aura WHERE guid = '%u'",GetGUIDLow());

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            uint64 caster_guid = fields[0].GetUInt64();
            uint32 low_item_guid = fields[1].GetUInt32();
            uint64 item_guid = low_item_guid ? MAKE_NEW_GUID(low_item_guid, 0, HIGHGUID_ITEM) : 0;
            uint32 spellid = fields[2].GetUInt32();
            uint32 effindex = fields[3].GetUInt32();
            uint32 stackcount = fields[4].GetUInt32();
            int32 damage     = (int32)fields[5].GetUInt32();
            int32 maxduration = (int32)fields[6].GetUInt32();
            int32 remaintime = (int32)fields[7].GetUInt32();
            int32 remaincharges = (int32)fields[8].GetUInt32();

            if (spellid == SPELL_ARENA_PREPARATION || spellid == SPELL_PREPARATION ||
                /*55169 + ((Player*)m_target)->GetClass() --- all class-dependent preparation spells*/
                (spellid >= 55170 && spellid <= 55180))
            {
               if (BattleGround const *bg = GetBattleGround())
                   if (bg->GetStatus() == STATUS_IN_PROGRESS)
                       continue;
            }

            if (spellid == 55153/*Warden Warning Freeze*/) // always skip it
                continue;

            SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
            if (!spellproto)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Unknown aura (spellid %u, effindex %u), ignore.",spellid,effindex);
                continue;
            }

            if (effindex >= 3)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Invalid effect index (spellid %u, effindex %u), ignore.",spellid,effindex);
                continue;
            }

            if (spellproto->Effect[effindex] == SPELL_EFFECT_APPLY_AURA)
            {
                switch (spellproto->EffectApplyAuraName[effindex])
                {
                case SPELL_AURA_MOD_CHARM:
                case SPELL_AURA_MOD_POSSESS:
                    continue;
                }
            }

            // negative effects should continue counting down after logout
            if (remaintime != -1 && SpellMgr::IsAuraCountdownContinueOffline(spellid, effindex))
            {
                if (remaintime/MILLISECONDS <= int32(timediff))
                    continue;

                remaintime -= timediff*MILLISECONDS;
            }

            // prevent wrong values of remaincharges
            if (spellproto->procCharges)
            {
                if (remaincharges <= 0 || remaincharges > spellproto->procCharges)
                    remaincharges = spellproto->procCharges;
            }
            else
                remaincharges = -1;

            for (uint32 i=0; i<stackcount; i++)
            {
                Aura* aura = CreateAura(spellproto, effindex, NULL, this, NULL);
                if (!damage)
                    damage = aura->GetModifier()->m_amount;

                // reset stolen single target auras
                if (caster_guid != GetGUID() && aura->IsSingleTarget())
                    aura->SetIsSingleTarget(false);

                aura->SetLoadedState(caster_guid, damage, maxduration, remaintime, remaincharges, item_guid);
                AddAura(aura);
                sLog.outDetail("Added aura spellid %u, effect %u", spellproto->Id, effindex);
            }
        }
        while (result->NextRow());
    }

    if (GetClass() == CLASS_WARRIOR)
        CastSpell(this,SPELL_ID_PASSIVE_BATTLE_STANCE,true);
}

void Player::LoadCorpse()
{
    if (isAlive())
    {
        sObjectAccessor.ConvertCorpseForPlayer(GetGUID());
    }
    else
    {
        if (Corpse *corpse = GetCorpse())
        {
            ApplyModFlag(PLAYER_FIELD_BYTES, PLAYER_FIELD_BYTE_RELEASE_TIMER, corpse && !sMapStore.LookupEntry(corpse->GetMapId())->Instanceable());
        }
        else
        {
            //Prevent Dead Player login without corpse
            ResurrectPlayer(0.5f);
        }
    }
}

void Player::_LoadInventory(QueryResultAutoPtr result, uint32 timediff)
{
    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT data,bag,slot,item,item_template FROM character_inventory JOIN item_instance ON character_inventory.item = item_instance.guid WHERE character_inventory.guid = '%u' ORDER BY bag,slot", GetGUIDLow());
    std::map<uint64, Bag*> bagMap;                          // fast guid lookup for bags
    //NOTE: the "order by `bag`" is important because it makes sure
    //the bagMap is filled before items in the bags are loaded
    //NOTE2: the "order by `slot`" is needed because mainhand weapons are (wrongly?)
    //expected to be equipped before offhand items (TODO: fixme)

    uint32 zone = GetZoneId();

    if (result)
    {
        std::list<Item*> problematicItems;

        // prevent items from being added to the queue when stored
        m_itemUpdateQueueBlocked = true;
        do
        {
            Field *fields = result->Fetch();
            uint32 bag_guid  = fields[1].GetUInt32();
            uint8  slot      = fields[2].GetUInt8();
            uint32 item_guid = fields[3].GetUInt32();
            uint32 item_id   = fields[4].GetUInt32();

            ItemPrototype const * proto = ObjectMgr::GetItemPrototype(item_id);

            if (!proto)
            {
                RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
                RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", item_guid);
                sLog.outLog(LOG_DEFAULT, "ERROR: Player::_LoadInventory: Player %s has an unknown item (id: #%u) in inventory, deleted.", GetName(),item_id);
                continue;
            }

            Item *item = NewItemOrBag(proto);

            if (!item->LoadFromDB(item_guid, GetGUID(), result))
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Player::_LoadInventory: Player %s has broken item (id: #%u) in inventory, deleted.", GetName(),item_id);
                RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
                item->FSetState(ITEM_REMOVED);
                item->SaveToDB();                           // it also deletes item object !
                continue;
            }

            // not allow have in alive state item limited to another map/zone
            if (isAlive() && item->IsLimitedToAnotherMapOrZone(GetMapId(),zone))
            {
                RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
                item->FSetState(ITEM_REMOVED);
                item->SaveToDB();                           // it also deletes item object !
                continue;
            }

            // "Conjured items disappear if you are logged out for more than 15 minutes"
            if ((timediff > 15*60) && (item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAGS_CONJURED)))
            {
                RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
                item->FSetState(ITEM_REMOVED);
                item->SaveToDB();                           // it also deletes item object !
                continue;
            }

            uint8 success = EQUIP_ERR_OK;

            if (!bag_guid)
            {
                // the item is not in a bag
                item->SetContainer(NULL);
                item->SetSlot(slot);

                if (IsInventoryPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    ItemPosCountVec dest;
                    success = CanStoreItem(INVENTORY_SLOT_BAG_0, slot, dest, item, false);
                    if (success == EQUIP_ERR_OK)
                        item = StoreItem(dest, item, true);
                }
                else if (IsEquipmentPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    uint16 dest;
                    success = CanEquipItem(slot, dest, item, false, false);
                    if (success == EQUIP_ERR_OK)
                        QuickEquipItem(dest, item);
                }
                else if (IsBankPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    ItemPosCountVec dest;
                    success = CanBankItem(INVENTORY_SLOT_BAG_0, slot, dest, item, false, false);
                    if (success == EQUIP_ERR_OK)
                        item = BankItem(dest, item, true);
                }

                if (success == EQUIP_ERR_OK)
                {
                    // store bags that may contain items in them
                    if (item->IsBag() && IsBagPos(item->GetPos()))
                        bagMap[item_guid] = (Bag*)item;
                }
            }
            else
            {
                item->SetSlot(NULL_SLOT);
                // the item is in a bag, find the bag
                std::map<uint64, Bag*>::iterator itr = bagMap.find(bag_guid);
                if (itr != bagMap.end())
                {
                    if (Item * oldItem = itr->second->GetItemByPos(slot))
                    {
                        sLog.outLog(LOG_DEFAULT, "ERROR: Player::_LoadInventory: Player %s is loading item (GUID: %u Entry: %u) bun in its place there was (GUID: %u Entry: %u), will send old by mail",
                            GetName(), item_guid, item_id, oldItem->GetGUIDLow(), oldItem->GetEntry());
                        // other cleaning stuff will be done by StoreItem
                        oldItem->SetContainer(NULL);
                        oldItem->SetSlot(NULL_SLOT);
                        // player is not in game yet, no update needed
                        RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", oldItem->GetGUIDLow());
                        problematicItems.push_back(oldItem);
                    }
                    itr->second->StoreItem(slot, item, true);
                    AddItemDurations(item); // FIXME shouldn't be here. As for now fixes a bug with an infinity of items which should have time duration limit.
                }
                else
                    success = EQUIP_ERR_DEBUG_SPECIAL;
            }

            // item's state may have changed after stored
            if (success == EQUIP_ERR_OK)
                item->SetState(ITEM_UNCHANGED, this);
            else
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Player::_LoadInventory: Player %s has item (GUID: %u Entry: %u) can't be loaded to inventory (Bag GUID: %u Slot: %u) result %u, will send by mail.", GetName(),item_guid, item_id, bag_guid, slot, success);
                RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
                problematicItems.push_back(item);
            }
        } while (result->NextRow());

        m_itemUpdateQueueBlocked = false;

        // send by mail problematic items
        while (!problematicItems.empty())
        {
            std::string subject = GetSession()->GetHellgroundString(LANG_NOT_EQUIPPED_ITEM);

            // fill mail
            MailDraft draft(subject);

            for (int i = 0; !problematicItems.empty() && i < MAX_MAIL_ITEMS; ++i)
            {
                Item* item = problematicItems.front();
                problematicItems.pop_front();

                draft.AddItem(item);
            }

            draft.SendMailTo(this, MailSender(this, MAIL_STATIONERY_GM));
        }
    }
    //if(isAlive())
    _ApplyAllItemMods();
}

// load mailed item which should receive current player
void Player::_LoadMailedItems(QueryResultAutoPtr result)
{
    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT item_guid, item_template FROM mail_items WHERE mail_id='%u'", mail->messageID);
    if (!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        uint32 mail_id       = fields[1].GetUInt32();
        uint32 item_guid_low = fields[2].GetUInt32();
        uint32 item_template = fields[3].GetUInt32();

        Mail* mail = GetMail(mail_id);
        if(!mail)
            continue;
        mail->AddItem(item_guid_low, item_template);

        ItemPrototype const *proto = ObjectMgr::GetItemPrototype(item_template);

        if(!proto)
        {
            sLog.outLog(LOG_DEFAULT, "Player %u has unknown item_template (ProtoType) in mailed items(GUID: %u template: %u) in mail (%u), deleted.", GetGUIDLow(), item_guid_low, item_template, mail->messageID);
            RealmDataDatabase.PExecute("DELETE FROM mail_items WHERE item_guid = '%u'", item_guid_low);
            RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", item_guid_low);
            continue;
        }

        Item *item = NewItemOrBag(proto);

        if (!item->LoadFromDB(item_guid_low, 0))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Player::_LoadMailedItems - Item in mail (%u) doesn't exist !!!! - item guid: %u, deleted from mail", mail->messageID, item_guid_low);
            RealmDataDatabase.PExecute("DELETE FROM mail_items WHERE item_guid = '%u'", item_guid_low);
            item->FSetState(ITEM_REMOVED);
            item->SaveToDB();                               // it also deletes item object !
            continue;
        }

        AddMItem(item);
    } while (result->NextRow());
}

void Player::_LoadMails(QueryResultAutoPtr result)
{
    m_mail.clear();
    //mails are in right order                                     0      1         2       3        4       5           6           7           8   9   10         11            12         13
    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT id,messageType,sender,receiver,subject,itemTextId,expire_time,deliver_time,money,cod,checked,stationery,mailTemplateId,has_items FROM mail WHERE receiver = '%u' ORDER BY id DESC",GetGUIDLow());
    if(!result)
        return;

    do
    {
        Field *fields = result->Fetch();
        Mail *m = new Mail;
        m->messageID = fields[0].GetUInt32();
        m->messageType = fields[1].GetUInt8();
        m->sender = fields[2].GetUInt32();
        m->receiverGuid = ObjectGuid(HIGHGUID_PLAYER, fields[3].GetUInt32());
        m->subject = fields[4].GetCppString();
        m->itemTextId = fields[5].GetUInt32();
        m->expire_time = (time_t)fields[6].GetUInt64();
        m->deliver_time = (time_t)fields[7].GetUInt64();
        m->money = fields[8].GetUInt32();
        m->COD = fields[9].GetUInt32();
        m->checked = fields[10].GetUInt32();
        m->stationery = fields[11].GetUInt8();
        m->mailTemplateId = fields[12].GetInt16();
        m->has_items = fields[13].GetBool();                // true, if mail have items or mail have template and items generated (maybe none)

        if (m->mailTemplateId && !sMailTemplateStore.LookupEntry(m->mailTemplateId))
        {
            sLog.outLog(LOG_DEFAULT, "ERROR: Player::_LoadMail - Mail (%u) have not existed MailTemplateId (%u), remove at load", m->messageID, m->mailTemplateId);
            m->mailTemplateId = 0;
        }

        m->state = MAIL_STATE_UNCHANGED;

        m_mail.push_back(m);

        if (m->mailTemplateId && !m->has_items)
            m->prepareTemplateItems(this);

    }
    while (result->NextRow());
}

void Player::LoadPet()
{
    //fixme: the pet should still be loaded if the player is not in world
    // just not added to the map
    if (IsInWorld())
    {
        Pet *pet = new Pet;
        if (!pet->LoadPetFromDB(this,0,0,true))
            delete pet;
    }
}

void Player::_LoadQuestStatus(QueryResultAutoPtr result, uint8 old_race)
{
    mQuestStatus.clear();

    uint32 slot = 0;

    ////                                                            0      1       2         3         4      5          6          7          8          9           10          11          12
    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT quest, status, rewarded, explored, timer, mobcount1, mobcount2, mobcount3, mobcount4, itemcount1, itemcount2, itemcount3, itemcount4 FROM character_queststatus WHERE guid = '%u'", GetGUIDLow());

    std::map<uint32, uint32> questSwaps;
    bool teamChanged = false;
    if (old_race)
    {
        teamChanged = Player::TeamForRace(old_race) != Player::TeamForRace(GetRace());

        // on faction change - remove all old faction quests and copy rewarded state to quest analogs of the new faction
        if (teamChanged)
        {
            QueryResultAutoPtr resQuest = GameDataDatabase.Query("SELECT old_id, new_id FROM race_change_swap_faction_quests");
            if (resQuest)
            {
                do
                {
                    Field *fields = resQuest->Fetch();
                    uint32 oldId = fields[0].GetUInt32();
                    uint32 newId = fields[1].GetUInt32();

                    questSwaps[oldId] = newId;
                } while (resQuest->NextRow());
            }
        }
    }

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            uint32 quest_id = fields[0].GetUInt32();
            uint32 new_quest_id = 0;
            if (teamChanged && questSwaps.find(quest_id) != questSwaps.end())
                new_quest_id = questSwaps[quest_id];

                                                            // used to be new, no delete?
            Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);
            if (pQuest)
            {
                // find or create
                QuestStatusData& questStatusData = mQuestStatus[quest_id];

                uint32 qstatus = fields[1].GetUInt32();
                if (qstatus < MAX_QUEST_STATUS)
                    questStatusData.m_status = QuestStatus(qstatus);
                else
                {
                    questStatusData.m_status = QUEST_STATUS_NONE;
                    sLog.outLog(LOG_DEFAULT, "ERROR: Player %s have invalid quest %d status (%d), replaced by QUEST_STATUS_NONE(0).",GetName(),quest_id,qstatus);
                }

                questStatusData.m_rewarded = (fields[2].GetUInt8() > 0);
                questStatusData.m_explored = (fields[3].GetUInt8() > 0);

                time_t quest_time = time_t(fields[4].GetUInt64());

                if (pQuest->HasFlag(QUEST_HELLGROUND_FLAGS_TIMED) && !GetQuestRewardStatus(quest_id) &&  questStatusData.m_status != QUEST_STATUS_NONE)
                {
                    AddTimedQuest(quest_id);

                    if (quest_time <= sWorld.GetGameTime())
                        questStatusData.m_timer = 1;
                    else
                        questStatusData.m_timer = (quest_time - sWorld.GetGameTime()) * 1000;
                }
                else
                    quest_time = 0;

                questStatusData.m_creatureOrGOcount[0] = fields[5].GetUInt32();
                questStatusData.m_creatureOrGOcount[1] = fields[6].GetUInt32();
                questStatusData.m_creatureOrGOcount[2] = fields[7].GetUInt32();
                questStatusData.m_creatureOrGOcount[3] = fields[8].GetUInt32();
                questStatusData.m_itemcount[0] = fields[9].GetUInt32();
                questStatusData.m_itemcount[1] = fields[10].GetUInt32();
                questStatusData.m_itemcount[2] = fields[11].GetUInt32();
                questStatusData.m_itemcount[3] = fields[12].GetUInt32();

                questStatusData.uState = QUEST_UNCHANGED;

                // add to quest log
                if (slot < MAX_QUEST_LOG_SIZE &&
                    (questStatusData.m_status==QUEST_STATUS_INCOMPLETE ||
                    questStatusData.m_status==QUEST_STATUS_COMPLETE &&
                    (!questStatusData.m_rewarded || pQuest->IsDaily())))
                {
                    if (old_race && !SatisfyQuestRace(pQuest, false))
                    {
                        // TakeQuestSourceItem(quest_id, false); // inventory is loaded after quests, can't take it here

                        // set quest status to not started, calling SetQuestStatus to also make it save in the DB later
                        SetQuestStatus(quest_id, QUEST_STATUS_NONE);
                    }
                    else
                    {
                        SetQuestSlot(slot,quest_id,quest_time);

                        if (questStatusData.m_status == QUEST_STATUS_COMPLETE)
                            SetQuestSlotState(slot,QUEST_STATE_COMPLETE);

                        for (uint8 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
                            if (questStatusData.m_creatureOrGOcount[idx])
                                SetQuestSlotCounter(slot,idx,questStatusData.m_creatureOrGOcount[idx]);

                        ++slot;
                    }
                }

                if (questStatusData.m_rewarded)
                {
					// DANGEROUS?
					// what the fuck is this?
					// disable it to prevent dual spec bugs like
					// complete quest https://x5.moonwell.su/?quest=10902 -> remove spell 28677 -> relog and spell appears in book again
					
					// learn rewarded spell if unknown
                    //learnQuestRewardedSpells(pQuest);
				
                    // set rewarded title if any
                    if (pQuest->GetCharTitleId())
                    {
                        if (CharTitlesEntry const* titleEntry = sCharTitlesStore.LookupEntry(pQuest->GetCharTitleId()))
                            SetTitle(titleEntry);
                    }
				
                    if (new_quest_id)
                    {
                        SetQuestStatus(new_quest_id, questStatusData.m_status);
                        getQuestStatusMap()[new_quest_id].m_rewarded = true;
                    }
                }

                sLog.outDebug("Quest status is {%u} for quest {%u} for player (GUID: %u)", questStatusData.m_status, quest_id, GetGUIDLow());
            }
        }
        while (result->NextRow());
    }

    // clear quest log tail
    for (uint16 i = slot; i < MAX_QUEST_LOG_SIZE; ++i)
        SetQuestSlot(i,0);
}

void Player::_LoadDailyQuestStatus(QueryResultAutoPtr result)
{
    for (uint32 quest_daily_idx = 0; quest_daily_idx < sWorld.getConfig(CONFIG_DAILY_MAX_PER_DAY); ++quest_daily_idx)
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,0);

    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT quest,time FROM character_queststatus_daily WHERE guid = '%u'", GetGUIDLow());

    if (result)
    {
        uint32 quest_daily_idx = 0;

        do
        {
            if (quest_daily_idx >= sWorld.getConfig(CONFIG_DAILY_MAX_PER_DAY))  // max amount with exist data in query
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Player (GUID: %u) have more 25 daily quest records in `charcter_queststatus_daily`",GetGUIDLow());
                break;
            }

            Field *fields = result->Fetch();

            uint32 quest_id = fields[0].GetUInt32();

            // save _any_ from daily quest times (it must be after last reset anyway)
            m_lastDailyQuestTime = (time_t)fields[1].GetUInt64();

            Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);
            if (!pQuest)
                continue;

            SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,quest_id);
            ++quest_daily_idx;

            sLog.outDebug("Daily quest {%u} cooldown for player (GUID: %u)", quest_id, GetGUIDLow());
        }
        while (result->NextRow());
    }

    m_DailyQuestChanged = false;
}

void Player::_LoadTalents(QueryResultAutoPtr result)
{
    //QueryResult *result = CharacterDatabase.PQuery("SELECT spell,spec FROM character_talents WHERE guid = '%u'",GetGUIDLow());

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            addTalent(fields[0].GetUInt32(), fields[1].GetUInt8(), false);
        } while (result->NextRow());
    }
}

void Player::_LoadSpells(QueryResultAutoPtr result)
{
    m_spells.clear();

    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT spell,slot,active FROM character_spell WHERE guid = '%u'",GetGUIDLow());

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            addSpell(fields[0].GetUInt16(), fields[2].GetBool(), false, true, fields[1].GetUInt16(), fields[3].GetBool());
        }
        while (result->NextRow());
    }
}

void Player::_LoadTutorials(QueryResultAutoPtr result)
{
    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT tut0,tut1,tut2,tut3,tut4,tut5,tut6,tut7 FROM character_tutorial WHERE account = '%u' AND realmid = '%u'", GetAccountId(), realmid);

    if (result)
    {
        do
        {
            Field *fields = result->Fetch();

            for (int iI=0; iI<8; iI++)
                m_Tutorials[iI] = fields[iI].GetUInt32();
        }
        while (result->NextRow());
    }

    m_TutorialsChanged = false;
}

void Player::_LoadGroup(QueryResultAutoPtr result)
{
    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT leaderGuid FROM group_member WHERE memberGuid='%u'", GetGUIDLow());
    if (result)
    {
        uint64 leaderGuid = MAKE_NEW_GUID((*result)[0].GetUInt32(), 0, HIGHGUID_PLAYER);
        Group* group = sObjectMgr.GetGroupByLeader(leaderGuid);
        if (group)
        {
            uint8 subgroup = group->GetMemberGroup(GetGUID());
            SetGroup(group, subgroup);
            if (GetLevel() >= LEVELREQUIREMENT_HEROIC)
            {
                // the group leader may change the instance difficulty while the player is offline
                SetDifficulty(group->GetDifficulty());
            }
        }
    }
}

void Player::_LoadBoundInstances(QueryResultAutoPtr result)
{
    for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        m_boundInstances[i].clear();

    Group *group = GetGroup();

    //QueryResultAutoPtr result = CharacterDatabase.PQuery("SELECT id, permanent, map, difficulty, resettime FROM character_instance LEFT JOIN instance ON instance = id WHERE guid = '%u'", GUID_LOPART(m_guid));
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            bool perm = fields[1].GetBool();
            uint32 mapId = fields[2].GetUInt32();
            uint32 instanceId = fields[0].GetUInt32();
            uint8 difficulty = fields[3].GetUInt8();
            time_t resetTime = (time_t)fields[4].GetUInt64();
            // the resettime for normal instances is only saved when the InstanceSave is unloaded
            // so the value read from the DB may be wrong here but only if the InstanceSave is loaded
            // and in that case it is not used

            MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
            if (!mapEntry || !mapEntry->IsDungeon())
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: _LoadBoundInstances: player %s(%d) has bind to not existed or not dungeon map %d (instanceId %d)", GetName(), GetGUIDLow(), mapId, instanceId);
                RealmDataDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%d' AND instance = '%d'", GetGUIDLow(), instanceId);
                continue;
            }

            if (!perm && group)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: _LoadBoundInstances: player %s(%d) is in group %d but has a non-permanent character bind to map %d,%d,%d", GetName(), GetGUIDLow(), GUID_LOPART(group->GetLeaderGUID()), mapId, instanceId, difficulty);
                RealmDataDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%d' AND instance = '%d'", GetGUIDLow(), instanceId);
                continue;
            }

            // since non permanent binds are always solo bind, they can always be reset
            InstanceSave *save = sInstanceSaveManager.AddInstanceSave(mapId, instanceId, difficulty, resetTime, !perm, true);
            if (save) BindToInstance(save, perm, true);
        } while (result->NextRow());
    }
}

InstancePlayerBind* Player::GetBoundInstance(uint32 mapid, uint8 difficulty)
{
    // some instances only have one difficulty
    const MapEntry* entry = sMapStore.LookupEntry(mapid);
    if (!entry || !entry->SupportsHeroicMode()) difficulty = DIFFICULTY_NORMAL;

    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    if (itr != m_boundInstances[difficulty].end())
        return &itr->second;
    else
        return NULL;
}

InstanceSave * Player::GetInstanceSave(uint32 mapid)
{
    InstancePlayerBind *pBind = GetBoundInstance(mapid, GetDifficulty());
    InstanceSave *pSave = pBind ? pBind->save : NULL;
    if (!pBind || !pBind->perm)
    {
        if (Group *group = GetGroup())
            if (InstanceGroupBind *groupBind = group->GetBoundInstance(mapid, GetDifficulty()))
                pSave = groupBind->save;
    }
    return pSave;
}

void Player::UnbindInstance(uint32 mapid, uint8 difficulty, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    UnbindInstance(itr, difficulty, unload);
}

void Player::UnbindInstance(BoundInstancesMap::iterator &itr, uint8 difficulty, bool unload)
{
    if (itr != m_boundInstances[difficulty].end())
    {
        if (!unload)
            RealmDataDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u' AND instance = '%u'", GetGUIDLow(), itr->second.save->GetSaveInstanceId());

        InstanceSave* save = itr->second.save;
        if (save != nullptr)
            save->RemovePlayer(GetGUID());

        m_boundInstances[difficulty].erase(itr++);
        sLog.outLog(LOG_DEFAULT, "Player UnbindInstance, removed m_bound instances ITR++ after %u", itr);
    }
}

InstancePlayerBind* Player::BindToInstance(InstanceSave *save, bool permanent, bool load)
{
    if (save != nullptr)
    {
        InstancePlayerBind& bind = m_boundInstances[save->GetDifficulty()][save->GetMapId()];
        if (bind.save)
        {
            // update the save when the group kills a boss
            if (permanent != bind.perm || save != bind.save)
                if (!load) RealmDataDatabase.PExecute("UPDATE character_instance SET instance = '%u', permanent = '%u' WHERE guid = '%u' AND instance = '%u'", save->GetSaveInstanceId(), permanent, GetGUIDLow(), bind.save->GetSaveInstanceId());
        }
        else
            if (!load) RealmDataDatabase.PExecute("INSERT INTO character_instance (guid, instance, permanent) VALUES ('%u', '%u', '%u')", GetGUIDLow(), save->GetSaveInstanceId(), permanent);

        if (bind.save != save)
        {
            if (bind.save != nullptr)
                bind.save->RemovePlayer(GetGUID());

            save->AddPlayer(GetGUID());
        }

        if (permanent) save->SetCanReset(false);

        bind.save = save;
        bind.perm = permanent;
        if (!load) sLog.outDebug("Player::BindToInstance: %s(%d) is now bound to map %d, instance %d, difficulty %d", GetName(), GetGUIDLow(), save->GetMapId(), save->GetSaveInstanceId(), save->GetDifficulty());
        return &bind;
    }
    else
        return nullptr;
}

void Player::SendRaidInfo()
{
    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 4);

    uint32 counter = 0, i;
    for (i = 0; i < TOTAL_DIFFICULTIES; i++)
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
            if (itr->second.perm) counter++;

    data << counter;
    for (i = 0; i < TOTAL_DIFFICULTIES; i++)
    {
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
        {
            if (itr->second.perm)
            {
                InstanceSave *save = itr->second.save;
                data << (save->GetMapId());
                data << (uint32)(save->GetResetTime() - time(NULL));
                data << save->GetSaveInstanceId();
                data << uint32(counter);
                counter--;
            }
        }
    }
    SendPacketToSelf(&data);
}

/*
- called on every successful teleportation to a map
*/
void Player::SendSavedInstances()
{
    bool hasBeenSaved = false;
    WorldPacket data;

    for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
    {
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
        {
            if (itr->second.perm)                                // only permanent binds are sent
            {
                hasBeenSaved = true;
                break;
            }
        }
    }

    //Send opcode 811. true or false means, whether you have current raid/heroic instances
    data.Initialize(SMSG_UPDATE_INSTANCE_OWNERSHIP);
    data << uint32(hasBeenSaved);
    SendPacketToSelf(&data);

    if (!hasBeenSaved)
        return;

    for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
    {
        for (BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end(); ++itr)
        {
            if (itr->second.perm)
            {
                data.Initialize(SMSG_UPDATE_LAST_INSTANCE);
                data << uint32(itr->second.save->GetMapId());
                SendPacketToSelf(&data);
            }
        }
    }
}

/// convert the player's binds to the group
void Player::ConvertInstancesToGroup(Player *player, Group *group, uint64 player_guid)
{
    bool has_binds = false;
    bool has_solo = false;

    if (player) { player_guid = player->GetGUID(); if (!group) group = player->GetGroup(); }
    ASSERT(player_guid);

    // copy all binds to the group, when changing leader it's assumed the character
    // will not have any solo binds

    if (player)
    {
        for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
        {
            for (BoundInstancesMap::iterator itr = player->m_boundInstances[i].begin(); itr != player->m_boundInstances[i].end();)
            {
                if (group)
                {
                    if (InstanceGroupBind *igb = group->GetBoundInstance(itr->second.save->GetMapId(), itr->second.save->GetDifficulty()))
                    {
                        // IF there is save - we go on
                        if (igb->save->GetSaveInstanceId() != itr->second.save->GetSaveInstanceId())
                        // IF save has instanceId and we had another -> this means new leader has cooldown for instance
                        {
                            Map* oldInst = sMapMgr.FindMap(itr->second.save->GetMapId(), igb->save->GetSaveInstanceId());
                            if (oldInst)
                            {
                                // ALL ONLINE PLAYERS
                                // some players might be kicked for some time (but still online - we need to get all players from map)
                                Map::PlayerList const &playersToTeleport = oldInst->GetPlayers(); 
                                std::list<Player *> teleportList; 
                                if (!playersToTeleport.isEmpty())
                                {
                                    for (Map::PlayerList::const_iterator itr = playersToTeleport.begin(); itr != playersToTeleport.end(); ++itr)
                                    {
                                        if (Player* plr = itr->getSource())
                                            teleportList.push_back(plr);
                                    }
                                    if (!teleportList.empty())
                                    {
                                        for (std::list<Player *>::iterator itr = teleportList.begin(); itr != teleportList.end(); ++itr)
                                            (*itr)->TeleportToNearestGraveyard();
                                    }
                                }
                            }
                        }
                    }
                }
                
                has_binds = true;
                if (group) 
                    group->BindToInstance(itr->second.save, itr->second.perm, true);
                // permanent binds are not removed
                if (!itr->second.perm)
                {
                    player->UnbindInstance(itr, i, true);   // increments itr
                    has_solo = true;
                }
                else
                    ++itr;
            }
        }
    }

    // if the player's not online we don't know what binds it has
    if (!player || !group || has_binds) RealmDataDatabase.PExecute("REPLACE INTO group_instance SELECT guid, instance, permanent FROM character_instance WHERE guid = '%u'", GUID_LOPART(player_guid));
    // the following should not get executed when changing leaders
    if (!player || has_solo) RealmDataDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%d' AND permanent = 0", GUID_LOPART(player_guid));
}

bool Player::Satisfy(AccessRequirement const *ar, uint32 target_map, bool report)
{
	if (isGameMaster() || sWorld.getConfig(CONFIG_IS_BETA))
		return true;
	
	//if (!sWorld.isEasyRealm() && IsCustomHeroicMap(target_map))
	//{
	//	uint8 other_difficulty = GetDifficulty() == DIFFICULTY_HEROIC ? DIFFICULTY_NORMAL : DIFFICULTY_HEROIC;
	//	
	//	Player::BoundInstancesMap &binds = GetBoundInstances(other_difficulty);
	//	for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
	//	{
	//		if (itr->first == target_map)
	//		{
	//			uint32 message = GetDifficulty() == DIFFICULTY_HEROIC ? 16594 : 16587;
	//			ChatHandler(this).SendSysMessage(message);
	//			return false;
	//		}
	//		else
	//			++itr;
	//	}
	//}

	if (ar)
    {
        uint32 LevelMin = 0;
        if (GetLevel() < ar->levelMin && !sWorld.getConfig(CONFIG_INSTANCE_IGNORE_LEVEL))
            LevelMin = ar->levelMin;

        uint32 LevelMax = 0;
        if (ar->levelMax >= ar->levelMin && GetLevel() > ar->levelMax && !sWorld.getConfig(CONFIG_INSTANCE_IGNORE_LEVEL))
            LevelMax = ar->levelMax;

        uint32 missingItem = 0;
        if (GetDifficulty() == DIFFICULTY_NORMAL)
        {
            if (ar->item)
            {
                if (!HasItemCount(ar->item, 1) &&
                    (!ar->item2 || !HasItemCount(ar->item2, 1)))
                    missingItem = ar->item;
            }
            else if (ar->item2 && !HasItemCount(ar->item2, 1))
                missingItem = ar->item2;
        }

        uint32 missingKey = 0;
        uint32 missingHeroicQuest = 0;
        if (GetDifficulty() == DIFFICULTY_HEROIC)
        {
            if (ar->heroicKey)
            {
                if (!HasItemCount(ar->heroicKey, 1) &&
                    (!ar->heroicKey2 || !HasItemCount(ar->heroicKey2, 1)))
                    missingKey = ar->heroicKey;
            }
            else if (ar->heroicKey2 && !HasItemCount(ar->heroicKey2, 1))
                missingKey = ar->heroicKey2;

            if (ar->heroicQuest && !GetQuestRewardStatus(ar->heroicQuest))
                missingHeroicQuest = ar->heroicQuest;
        }

        uint32 missingQuest = 0;
        if (ar->quest && !GetQuestRewardStatus(ar->quest))
            missingQuest = ar->quest;

        uint32 missingAura = 0;
        if (ar->auraId && !HasAura(ar->auraId))
            missingAura = ar->auraId;

        // no req for any instance on x100
        if (sWorld.isEasyRealm())
            return true;

        bool levelMissing = LevelMin || LevelMax;
        bool additionalMissing = missingItem || missingKey || missingQuest || missingHeroicQuest || missingAura;

		if (target_map == 269 && HasItemCount(5126, 1))
		{
            if (report && LevelMin)
            {
                GetSession()->SendAreaTriggerMessage(GetSession()->GetHellgroundString(LANG_LEVEL_MINREQUIRED), LevelMin);
                return false;
            }
            else
                return true;
		}

        if (levelMissing || additionalMissing)
        {
            if (report)
            {
                if (missingItem)
                    GetSession()->SendAreaTriggerMessage(GetSession()->GetHellgroundString(LANG_LEVEL_MINREQUIRED_AND_ITEM), ar->levelMin, ObjectMgr::GetItemPrototype(missingItem)->Name1);
                else if (missingKey)
                    SendTransferAborted(target_map, TRANSFER_ABORT_DIFFICULTY2);
                else if (missingHeroicQuest)
                    GetSession()->SendAreaTriggerMessage("%s", ar->heroicQuestFailedText.c_str());
                else if (missingQuest)
                    GetSession()->SendAreaTriggerMessage("%s", ar->questFailedText.c_str());
                else if (LevelMin)
                    GetSession()->SendAreaTriggerMessage(GetSession()->GetHellgroundString(LANG_LEVEL_MINREQUIRED), LevelMin);
                else if (missingAura)
                    GetSession()->SendAreaTriggerMessage("%s", ar->missingAuraText.c_str());
            }
            return false;
        }
    }
    return true;
}

bool Player::_LoadHomeBind(uint8 new_race, QueryResultAutoPtr result)
{
    PlayerInfo const *info = sObjectMgr.GetPlayerInfo(new_race ? new_race : GetRace(), GetClass());
    if (!info)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player have incorrect race/class pair. Can't be loaded.");
        return false;
    }

    bool ok = false;
    if (result)
    {
        if (new_race)
        {
            bool Gurubashi = 0;

            static const uint32 arr[2][2] =
            {
                { 530, 3703 },
                { 0, 33 }
            };
            static const float arrf[2][3] =
            {
                { -1755.090454f, 5152.325f, -37.204384f },
                { -13179.616211f, 320.826538f, 33.243084f }
            };

            // Update homebind - set Shattrath tavern(there are no inkeeper, but thats good neutral place)
            RealmDataDatabase.PExecute("UPDATE character_homebind set `map`='%u', `zone`='%u', `position_x`='%f', `position_y`='%f', `position_z`='%f' WHERE `guid`='%u'", arr[Gurubashi][0], arr[Gurubashi][1],
                arrf[Gurubashi][0], arrf[Gurubashi][1], arrf[Gurubashi][2], GetGUIDLow());

            m_homebindMapId = arr[Gurubashi][0];
            m_homebindZoneId = arr[Gurubashi][1];
            m_homebindX = arrf[Gurubashi][0];
            m_homebindY = arrf[Gurubashi][1];
            m_homebindZ = arrf[Gurubashi][2];
        }
        else
        {
            Field *fields = result->Fetch();
            m_homebindMapId = fields[0].GetUInt32();
            m_homebindZoneId = fields[1].GetUInt16();
            m_homebindX = fields[2].GetFloat();
            m_homebindY = fields[3].GetFloat();
            m_homebindZ = fields[4].GetFloat();
        }

        MapEntry const* bindMapEntry = sMapStore.LookupEntry(m_homebindMapId);

        // accept saved data only for valid position (and non instanceable), and accessable
        if (MapManager::IsValidMapCoord(m_homebindMapId,m_homebindX,m_homebindY,m_homebindZ) &&
            !bindMapEntry->Instanceable() && GetSession()->Expansion() >= bindMapEntry->Expansion())
        {
            ok = true;
        }
        else
            RealmDataDatabase.PExecute("DELETE FROM character_homebind WHERE guid = '%u'", GetGUIDLow());
    }

    if (!ok)
    {
        m_homebindMapId = info->mapId;
        m_homebindZoneId = info->zoneId;
        m_homebindX = info->positionX;
        m_homebindY = info->positionY;
        m_homebindZ = info->positionZ;

        RealmDataDatabase.PExecute("INSERT INTO character_homebind (guid,map,zone,position_x,position_y,position_z) VALUES ('%u', '%u', '%u', '%f', '%f', '%f')", GetGUIDLow(), m_homebindMapId, (uint32)m_homebindZoneId, m_homebindX, m_homebindY, m_homebindZ);
    }

    debug_log("Setting player home position: mapid is: %u, zoneid is %u, X is %f, Y is %f, Z is %f\n",
        m_homebindMapId, m_homebindZoneId, m_homebindX, m_homebindY, m_homebindZ);

    return true;
}

/*********************************************************/
/***                   SAVE SYSTEM                     ***/
/*********************************************************/

void Player::SaveToDB()
{
    if (_preventSave)
        return;

    _preventSave = true;

    // delay auto save at any saves (manual, in code, or autosave)
    m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

    // first save/honor gain after midnight will also update the player's honor fields
    UpdateHonorFields();

    uint32 mapid = IsBeingTeleported() ? GetTeleportDest().mapid : GetMapId();
    const MapEntry * me = sMapStore.LookupEntry(mapid);
    // players aren't saved on arena maps
    if (!me || me->IsBattleArena() || (GetSession() && GetSession()->isFakeBot()))
    {
        _preventSave = false;
        return;
    }

    int is_save_resting = HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ? 1 : 0;
                                                            //save, far from tavern/city
                                                            //save, but in tavern/city
    sLog.outDebug("The value of player %s at save: ", m_name.c_str());
    outDebugValues();

    // save state (after auras removing), if aura remove some flags then it must set it back by self)
    uint32 tmp_bytes = GetUInt32Value(UNIT_FIELD_BYTES_1);
    uint32 tmp_bytes2 = GetUInt32Value(UNIT_FIELD_BYTES_2);
    uint32 tmp_flags = GetUInt32Value(UNIT_FIELD_FLAGS);
    uint32 tmp_pflags = GetUInt32Value(PLAYER_FLAGS);
    uint32 tmp_displayid = GetDisplayId();

    // Set player sit state to standing on save, also stealth and shifted form
    SetByteValue(UNIT_FIELD_BYTES_1, 0, 0);                 // stand state
    SetByteValue(UNIT_FIELD_BYTES_2, 3, 0);                 // shapeshift
    SetByteValue(UNIT_FIELD_BYTES_1, 3, 0);                 // stand flags?
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_ROTATE);
    SetDisplayId(GetNativeDisplayId());

    bool inworld = IsInWorld();

    RealmDataDatabase.BeginTransaction();

    //CharacterDatabase.PExecute("DELETE FROM characters WHERE guid = '%u'",GetGUIDLow());

    static SqlStatementID deleteStats;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteStats, "DELETE FROM character_stats_ro WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    //static SqlStatementID updateStats;
    //stmt = RealmDataDatabase.CreateStatement(updateStats, "INSERT INTO character_stats_ro VALUES (?, ?, ?, ?)");
    //stmt.PExecute(GetGUIDLow(), GetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY), GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS),m_DailyArenasWon);

    static SqlStatementID deleteCharacter;
    static SqlStatementID insertCharacter;

    stmt = RealmDataDatabase.CreateStatement(deleteCharacter, "DELETE FROM characters WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    stmt = RealmDataDatabase.CreateStatement(insertCharacter, "INSERT INTO characters (guid, account, name, race, class, gender, level, xp, money, playerBytes, playerBytes2, playerFlags, totalKills, todayKills,"
                                            "map, instance_id, dungeon_difficulty, position_x, position_y, position_z, orientation, data, "
                                            "taximask, online, cinematic, "
                                            "totaltime, leveltime, rest_bonus, logout_time, is_logout_resting, resettalents_cost, resettalents_time, "
                                            "trans_x, trans_y, trans_z, trans_o, transguid, extra_flags, stable_slots, at_login, zone, "
                                            "death_expire_time, taxi_path, arenaPoints, latency, title, grantableLevels, totalHonorPoints, changeRaceTo, char_custom_flags, activeSpec, needCaptcha) "
                                            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
                                                "?, ?, ?, ?, ?, ?, ?, ?, "
                                                "?, ?, ?, "
                                                "?, ?, ?, ?, ?, ?, ?, "
                                                "?, ?, ?, ?, ?, ?, ?, ?, ?, "
                                                "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    auto a = uint32(GetRace());
    auto b = uint32(GetClass());
    auto c = uint32(GetGender());
    auto d = uint32(GetLevel());
    
    stmt.addUInt32(GetGUIDLow());
    stmt.addUInt32(GetSession()->GetAccountId());
    stmt.addString(m_name);
    stmt.addUInt32(uint32(GetRace()));
    stmt.addUInt32(uint32(GetClass()));
    stmt.addUInt32(uint32(GetGender()));
    stmt.addUInt32(uint32(GetLevel()));
    stmt.addUInt32(GetUInt32Value(PLAYER_XP));
    stmt.addUInt32(GetMoney());
    stmt.addUInt32(GetUInt32Value(PLAYER_BYTES));
    stmt.addUInt32(GetUInt32Value(PLAYER_BYTES_2));
    stmt.addUInt32(GetUInt32Value(PLAYER_FLAGS));
    stmt.addUInt32(GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS));
    stmt.addUInt16(GetUInt16Value(PLAYER_FIELD_KILLS, 0));

    bool save_to_dest = false;
    if (IsBeingTeleported())
    {
        // don't save to battlegrounds or arenas
        //const MapEntry *entry = sMapStore.LookupEntry(GetTeleportDest().mapid);
        //if(entry && entry->map_type != MAP_BATTLEGROUND && entry->map_type != MAP_ARENA)
            save_to_dest = true;
    }

    if (!save_to_dest)
    {
        stmt.addUInt32(GetMapId());
        stmt.addUInt32(GetInstanciableInstanceId());
        stmt.addUInt32(uint32(GetDifficulty()));
        stmt.addFloat(finiteAlways(GetPositionX()));
        stmt.addFloat(finiteAlways(GetPositionY()));
        stmt.addFloat(finiteAlways(GetPositionZ()));
        stmt.addFloat(finiteAlways(GetOrientation()));
    }
    else
    {
        stmt.addUInt32(GetTeleportDest().mapid);
        stmt.addUInt32(0);
        stmt.addUInt32(uint32(GetDifficulty()));
        stmt.addFloat(finiteAlways(GetTeleportDest().coord_x));
        stmt.addFloat(finiteAlways(GetTeleportDest().coord_y));
        stmt.addFloat(finiteAlways(GetTeleportDest().coord_z));
        stmt.addFloat(finiteAlways(GetTeleportDest().orientation));", '";
    }

    // set transmogged item ids to display in character selection menu
    // need this implementation, because otherwise in character sheet will display transmogged item info
    // yes, this is bad code, but easy way
    std::map<int, int32> default_items;
    if (TransmogrificationStruct* transmog = GetTransmogManager()->GetActiveTransmogrStructure())
    {
        for (uint8 i = 0; i < 11; ++i)
        {
            if (uint32 item_id = transmog[i].itemid)
            {
                int32 tmp = GetTransmogManager()->TransmogSlotToInventorySlot(i);     
                if (tmp < 0)
                    continue;
                
                uint32 slot = (uint32)tmp;

                int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);

                int32 current_item = GetInt32Value(VisibleBase);

                // save items to restore it later
                default_items[VisibleBase] = current_item;
                SetUInt32Value(VisibleBase, item_id);
            }
        }
    }

    std::string tmpStr = GetUInt32ValuesString();

    // restore
    for (const auto& item : default_items)
    {
        SetUInt32Value(item.first, item.second);
    }

    stmt.addString(tmpStr); //data insert

    tmpStr = m_taxi.GetTaxiMaskString();
    stmt.addString(tmpStr);
    stmt.addBool(inworld ? true : false);
    stmt.addBool(m_cinematic);
    stmt.addUInt32(m_Played_time[0]);
    stmt.addUInt32(m_Played_time[1]);
    stmt.addFloat(finiteAlways(m_rest_bonus));
    stmt.addUInt64(uint64(time(NULL)));
    stmt.addBool(is_save_resting);
    stmt.addUInt32(m_resetTalentsCost);
    stmt.addUInt64(uint64(m_resetTalentsTime));
    stmt.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->x));
    stmt.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->y));
    stmt.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->z));
    stmt.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->o));

    if (m_transport)
        stmt.addUInt32(m_transport->GetGUIDLow());
    else
        stmt.addUInt32(0);

    stmt.addUInt32(m_ExtraFlags);
    stmt.addUInt8(m_stableSlots);                  // to prevent save uint8 as char
    stmt.addUInt32(m_atLoginFlags);
    stmt.addUInt32(GetCachedZone());
    stmt.addUInt64(uint64(m_deathExpireTime));
    stmt.addString(m_taxi.SaveTaxiDestinationsToString());
    stmt.addUInt32(GetArenaPoints());
    stmt.addUInt32(GetSession()->GetLatency());

    stmt.addUInt64(GetUInt64Value(PLAYER__FIELD_KNOWN_TITLES));
    stmt.addUInt32(m_GrantableLevelsCount);
    stmt.addUInt32(GetHonorPoints());
    stmt.addUInt8(m_changeRaceTo);
    stmt.addUInt16(m_ChCustomFlags);

    stmt.addUInt32(m_activeSpec);

	stmt.addUInt32(!captcha_current.empty() ? 1 : 0);

    stmt.Execute();

    if (m_mailsUpdated)                                      //save mails only when needed
        _SaveMail();

    _SaveBattleGroundCoord();
    _SaveInventory();
    _SaveQuestStatus();
    _SaveDailyQuestStatus();
    _SaveTutorials();
    _SaveTalents();
    _SaveSpells();
    _SaveSpellCooldowns();
    _SaveActions();
    _SaveAuras();
    m_reputationMgr.SaveToDB(false);

    RealmDataDatabase.CommitTransaction();

    // restore state (before aura apply, if aura remove flag then aura must set it ack by self)
    SetDisplayId(tmp_displayid);
    SetUInt32Value(UNIT_FIELD_BYTES_1, tmp_bytes);
    SetUInt32Value(UNIT_FIELD_BYTES_2, tmp_bytes2);
    SetUInt32Value(UNIT_FIELD_FLAGS, tmp_flags);
    SetUInt32Value(PLAYER_FLAGS, tmp_pflags);

    if (WorldSession *ses = GetSession())
    {
        if (ses->m_muteRemain > 1000)
            AccountsDatabase.PExecute("UPDATE account_punishment SET muteRemain ='%u' WHERE account_id = '%u' AND punishment_type_id = '%u' AND active = 1", uint32(ses->m_muteRemain / 1000), ses->GetAccountId(), PUNISHMENT_MUTE);
        else if (ses->m_muteRemain) // exists but is lower than second - remove
        {
            ses->m_muteRemain = 0;
            AccountsDatabase.PExecute("UPDATE account_punishment SET muteRemain ='0', active = '0' WHERE account_id = '%u' AND punishment_type_id = '%u' AND active = 1 AND muteRemain != '0'", ses->GetAccountId(), PUNISHMENT_MUTE);
        }

        if (ses->m_trollMuteRemain > 1000)
            AccountsDatabase.PExecute("UPDATE account_punishment SET muteRemain ='%u' WHERE account_id = '%u' AND punishment_type_id = '%u' AND active = 1", uint32(ses->m_trollMuteRemain / 1000), ses->GetAccountId(), PUNISHMENT_TROLLMUTE);
        else if (ses->m_trollMuteRemain) // exists but is lower than second - remove
        {
            ses->m_trollMuteRemain = 0;
            AccountsDatabase.PExecute("UPDATE account_punishment SET muteRemain ='0', active = '0' WHERE account_id = '%u' AND punishment_type_id = '%u' AND active = 1 AND muteRemain != '0'", ses->GetAccountId(), PUNISHMENT_TROLLMUTE);
        }

        time_t cur = time(NULL);
        uint32 timeToAdd = uint32(cur - m_lastInGameSaveTime);
        m_lastInGameSaveTime = cur;

        AccountsDatabase.PExecute("UPDATE account_login SET in_game = in_game + '%u' WHERE account_id = '%u' AND login_id = '%u'", timeToAdd, ses->GetAccountId(), ses->GetLoginId());
    }

    // save pet (hunter pet level and experience and all type pets health/mana).
    if (Pet* pet = GetPet())
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);

    _preventSave = false;
}

// fast save function for item/money cheating preventing - save only inventory and money state
void Player::SaveInventoryAndGoldToDB()
{
    _SaveInventory();
    SaveGoldToDB();
}

void Player::SaveGoldToDB()
{
    static SqlStatementID updateMoney;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(updateMoney, "UPDATE characters SET money = ? WHERE guid = ?");
    stmt.PExecute(GetMoney(), GetGUIDLow());
}

void Player::_SaveActions()
{
    static SqlStatementID insertCharacterAction;
    static SqlStatementID updateCharacterAction;
    static SqlStatementID deleteCharacterAction;

    for (ActionButtonList::iterator itr = m_actionButtons.begin(); itr != m_actionButtons.end();)
    {
        switch (itr->second.uState)
        {
            case ACTIONBUTTON_NEW:
            {
                //for 'primary-key already exists' errors, they find a way.. :P
                {
                    SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteCharacterAction, "DELETE FROM character_action where guid = ? AND button = ? AND spec = ?");
                    stmt.addUInt32(GetGUIDLow());
                    stmt.addUInt32(uint32(itr->first));
                    stmt.addUInt32(uint32(m_activeSpec));
                    stmt.Execute();
                }

                SqlStatement stmt = RealmDataDatabase.CreateStatement(insertCharacterAction, "INSERT INTO character_action (guid, spec, button, action, type, misc) VALUES (?, ?, ?, ?, ?, ?)");
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(uint32(m_activeSpec));
                stmt.addUInt32(uint32(itr->first));
                stmt.addUInt32(uint32(itr->second.action));
                stmt.addUInt32(uint32(itr->second.type));
                stmt.addUInt32(uint32(itr->second.misc));
                stmt.Execute();
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
                break;
            }
            case ACTIONBUTTON_CHANGED:
            {
                SqlStatement stmt = RealmDataDatabase.CreateStatement(updateCharacterAction, "UPDATE character_action SET action = ?, type = ?, misc = ? WHERE guid = ? AND button = ? AND spec = ?");
                stmt.addUInt32(uint32(itr->second.action));
                stmt.addUInt32(uint32(itr->second.type));
                stmt.addUInt32(uint32(itr->second.misc));
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(uint32(itr->first));
                stmt.addUInt32(uint32(m_activeSpec));
                stmt.Execute();
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
                break;
            }
            case ACTIONBUTTON_DELETED:
            {
                SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteCharacterAction, "DELETE FROM character_action WHERE guid = ? AND button = ? AND spec = ?");
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(uint32(itr->first));
                stmt.addUInt32(uint32(m_activeSpec));
                stmt.Execute();
                m_actionButtons.erase(itr++);
                break;
            }
            default:
                ++itr;
                break;
        };
    }
}

void Player::_SaveAuras()
{
    static SqlStatementID deleteAuras;
    static SqlStatementID insertAura;

    SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteAuras, "DELETE FROM character_aura WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    AuraMap const& auras = GetAuras();

    if (auras.empty())
        return;

    spellEffectPair lastEffectPair = auras.begin()->first;
    uint32 stackCounter = 1;

    for (AuraMap::const_iterator itr = auras.begin(); ; ++itr)
    {
        if (itr == auras.end() || lastEffectPair != itr->first)
        {
            AuraMap::const_iterator itr2 = itr;
            // save previous spellEffectPair to db
            itr2--;
            SpellEntry const *spellInfo = itr2->second->GetSpellProto();

            bool dresser_spell = spellInfo->Category == SPELL_CATEGORY_AURA_TABARD || spellInfo->Category == SPELL_CATEGORY_MORPH_SHIRT;

            //skip all auras from spells that are passive or need a shapeshift
            if (!(itr2->second->IsPassive() || itr2->second->IsRemovedOnShapeLost()) && !dresser_spell)
            {
                //do not save single target auras (unless they were cast by the player)
                if (!(itr2->second->GetCasterGUID() != GetGUID() && itr2->second->IsSingleTarget()))
                {
                    uint8 i;
                    // or apply at cast SPELL_AURA_MOD_SHAPESHIFT(and it's not flight form) or SPELL_AURA_MOD_STEALTH auras

					// why it's disabled? any reason? DANGEROUS!
                    for (i = 0; i < 3; i++)
                        if ((!itr2->second->IsSpellFlightShapeshift() && (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)) ||
                        spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_STEALTH)
                            break;

                    if (i == 3)
                    {
                        stmt = RealmDataDatabase.CreateStatement(insertAura, "INSERT INTO character_aura (guid, caster_guid, item_guid, spell, effect_index, stackcount, amount, maxduration, remaintime, remaincharges) "
                                                                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
                        stmt.addUInt32(GetGUIDLow());
                        stmt.addUInt64(itr2->second->GetCasterGUID());
                        stmt.addUInt32(GUID_LOPART(itr2->second->GetCastItemGUID()));
                        stmt.addUInt32(uint32(itr2->second->GetId()));
                        stmt.addUInt32(uint32(itr2->second->GetEffIndex()));
                        stmt.addUInt32(uint32(itr2->second->GetStackAmount()));
                        stmt.addInt32(itr2->second->GetModifier()->m_amount);
                        stmt.addInt32(itr2->second->GetAuraMaxDuration());
                        stmt.addInt32(itr2->second->GetAuraDuration());
                        stmt.addInt32(itr2->second->m_procCharges);
                        stmt.Execute();
                    }
                }
            }

            if (itr == auras.end())
                break;
        }

        //TODO: if need delete this
        if (lastEffectPair == itr->first)
            stackCounter++;
        else
        {
            lastEffectPair = itr->first;
            stackCounter = 1;
        }
    }
}

void Player::_SaveBattleGroundCoord()
{
    static SqlStatementID deleteBGCoord;
    static SqlStatementID insertBGCoord;

    SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteBGCoord, "DELETE FROM character_bgcoord WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    // don't save if not needed
    if (!InBattleGroundOrArena())
        return;

    WorldLocation const& loc = GetBattleGroundEntryPoint();

    stmt = RealmDataDatabase.CreateStatement(insertBGCoord, "INSERT INTO character_bgcoord (guid, bgid, bgteam, bgmap, bgx, bgy, bgz, bgo) "
                                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    stmt.addUInt32(GetGUIDLow());
    stmt.addUInt32(GetBattleGroundId());
    stmt.addUInt32(GetBGTeam());
    stmt.addUInt32(loc.mapid);
    stmt.addFloat(finiteAlways(loc.coord_x));
    stmt.addFloat(finiteAlways(loc.coord_y));
    stmt.addFloat(finiteAlways(loc.coord_z));
    stmt.addFloat(finiteAlways(loc.orientation));
    stmt.Execute();
}

void Player::_SaveInventory()
{
    static SqlStatementID deleteCharInvByItem;
    static SqlStatementID deleteItemInstance;
    static SqlStatementID deleteCharInvByPlace;
    static SqlStatementID insertBan;
    static SqlStatementID insertCharInv;
    static SqlStatementID updateCharInv;
    // force items in buyback slots to new state
    // and remove those that aren't already
    for (uint8 i = BUYBACK_SLOT_START; i < BUYBACK_SLOT_END; i++)
    {
        Item *item = m_items[i];
        if (!item || item->GetState() == ITEM_NEW)
            continue;

        SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteCharInvByItem, "DELETE FROM character_inventory WHERE item = ?");
        stmt.PExecute(item->GetGUIDLow());

        stmt = RealmDataDatabase.CreateStatement(deleteItemInstance, "DELETE FROM item_instance WHERE guid = ?");
        stmt.PExecute(item->GetGUIDLow());

        m_items[i]->FSetState(ITEM_NEW);
    }

    // update enchantment durations
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin(); itr != m_enchantDuration.end(); ++itr)
        itr->item->SetEnchantmentDuration(itr->slot, itr->leftduration);

    // if no changes
    if (m_itemUpdateQueue.empty())
        return;

    // do not save if the update queue is corrupt
    uint32 lowGuid = GetGUIDLow();
    for (size_t i = 0; i < m_itemUpdateQueue.size(); ++i)
    {
        Item *item = m_itemUpdateQueue[i];
        if (!item)
            continue;

        Bag *container = item->GetContainer();
        uint32 bag_guid = container ? container->GetGUIDLow() : 0;

        //temporary
        /*sLog.outLog(LOG_CHEAT, "InventoryLog: player lowguid %u, num in queue %u, itemstate %u, bag_guid %u, item entry %u, itemLowGuid %u, bagSlot %u, slot %u, queueItemPos %u", 
            lowGuid, (uint32)i, (uint32)item->GetState(), bag_guid, item->GetEntry(), item->GetGUIDLow(), item->GetBagSlot(), item->GetSlot(), item->GetQueuePos());*/

        if (item->GetState() != ITEM_REMOVED)
        {
            Item *test = GetItemByPos(item->GetBagSlot(), item->GetSlot());
            if (test == NULL)
            {
                uint32 bagTestGUID = 0;

                if (Item* test2 = GetItemByPos(INVENTORY_SLOT_BAG_0, item->GetBagSlot()))
                    bagTestGUID = test2->GetGUIDLow();

                sLog.outLog(LOG_DEFAULT, "ERROR: Player(GUID: %u Name: %s)::_SaveInventory - the bag(%u) and slot(%u) values for the item with guid %u (state %d) id %u are incorrect, the player doesn't have an item at that position!", lowGuid, GetName(), item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), (int32)item->GetState(), item->GetEntry());
                sLog.outLog(LOG_CHEAT, "ERROR: Player(GUID: %u Name: %s)::_SaveInventory - the bag(%u) and slot(%u) values for the item with guid %u (state %d) id %u are incorrect, the player doesn't have an item at that position!", lowGuid, GetName(), item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), (int32)item->GetState(), item->GetEntry());
                
                // according to the test that was just performed nothing should be in this slot, delete
                SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteCharInvByPlace, "DELETE FROM character_inventory WHERE guid = ? AND bag = ? AND slot = ?");
                stmt.addUInt32(lowGuid);
                stmt.addUInt32(bagTestGUID);
                stmt.addUInt8(item->GetSlot());
                stmt.Execute();

                // also THIS item should be somewhere else, cheat attempt
                item->FSetState(ITEM_REMOVED); // we are IN updateQueue right now, can't use SetState which modifies the queue
                item->RemoveFromWorld();

                // gotta remove item by guid to remove pointer mentions of the item, cause it will be deleted in item->SaveToDB()
                //RemoveItemByGuid(item->GetGUID());

                // don't skip, let the switch delete it
                //continue;

                /*AccountsDatabase.BeginTransaction();

                stmt = AccountsDatabase.CreateStatement(insertBan, "INSERT INTO account_punishment VALUES (?, ?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP(), '[CONSOLE]', 'Cheat CON-01', 1, 0)");

                stmt.PExecute(GetSession()->GetAccountId(), uint32(PUNISHMENT_BAN));

                AccountsDatabase.CommitTransaction();*/


                //GetSession()->KickPlayer();
            }
            else if (test != item)
            {
                sLog.outLog(LOG_DEFAULT, "ERROR: Player(GUID: %u Name: %s)::_SaveInventory - the bag(%u) and slot(%u) values for the item with guid %u (state %d) id %u are incorrect, the item with guid %u (state %d) id %u is there instead!", lowGuid, GetName(), item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), (int32)item->GetState(), item->GetEntry(), test->GetGUIDLow(), (int32)test->GetState(), test->GetEntry());
                sLog.outLog(LOG_CHEAT, "ERROR: Player(GUID: %u Name: %s)::_SaveInventory - the bag(%u) and slot(%u) values for the item with guid %u (state %d) id %u are incorrect, the item with guid %u (state %d) id %u is there instead!", lowGuid, GetName(), item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), (int32)item->GetState(), item->GetEntry(), test->GetGUIDLow(), (int32)test->GetState(), test->GetEntry());

                // save all changes to the item...
                if (item->GetState() != ITEM_NEW) // only for existing items, no dupes
                    item->SaveToDB();

                // ...but do not save position in invntory

                /*AccountsDatabase.BeginTransaction();

                SqlStatement stmt = AccountsDatabase.CreateStatement(insertBan, "INSERT INTO account_punishment VALUES (?, ?, UNIX_TIMESTAMP(), UNIX_TIMESTAMP(), '[CONSOLE]', 'Cheat CON-02', 1, 0)");

                stmt.PExecute(GetSession()->GetAccountId(), uint32(PUNISHMENT_BAN));

                AccountsDatabase.CommitTransaction();
                GetSession()->KickPlayer();*/

                continue;
            }
        }

        switch (item->GetState())
        {
            case ITEM_NEW:
            {
                SqlStatement stmt = RealmDataDatabase.CreateStatement(insertCharInv, "INSERT INTO character_inventory (guid, bag, slot, item, item_template) VALUES (?, ?, ?, ?, ?)");
                stmt.addUInt32(lowGuid);
                stmt.addUInt32(bag_guid);
                stmt.addUInt32(item->GetSlot());
                stmt.addUInt32(item->GetGUIDLow());
                stmt.addUInt32(item->GetEntry());
                stmt.Execute();
                break;
            }
            case ITEM_CHANGED:
            {
                SqlStatement stmt = RealmDataDatabase.CreateStatement(updateCharInv, "UPDATE character_inventory SET guid = ?, bag = ?, slot = ?, item_template = ? WHERE item = ?");
                stmt.addUInt32(lowGuid);
                stmt.addUInt32(bag_guid);
                stmt.addUInt32(item->GetSlot());
                stmt.addUInt32(item->GetEntry());
                stmt.addUInt32(item->GetGUIDLow());
                stmt.Execute();
                break;
            }
            case ITEM_REMOVED:
            {
                SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteCharInvByItem, "DELETE FROM character_inventory WHERE item = ?");
                stmt.PExecute(item->GetGUIDLow());
                break;
            }
            case ITEM_UNCHANGED:
                break;
        }

        item->SaveToDB();                                   // item have unchanged inventory record and can be save standalone
    }
    m_itemUpdateQueue.clear();
}

void Player::_SaveMail()
{
    static SqlStatementID updateMail;
    static SqlStatementID deleteMailItemsByGuid;
    static SqlStatementID deleteMailItemsById;
    static SqlStatementID deleteMail;
    static SqlStatementID deleteItemText;
    static SqlStatementID deleteItemInstance;

    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        Mail *m = (*itr);
        if (m->state == MAIL_STATE_CHANGED)
        {
            SqlStatement stmt = RealmDataDatabase.CreateStatement(updateMail, "UPDATE mail SET itemTextId = ?, has_items = ?, expire_time = ?, deliver_time = ?, money = ?, cod = ?, checked = ? WHERE id = ?");
            stmt.addUInt32(m->itemTextId);
            stmt.addBool(m->HasItems() ? true : false);
            stmt.addUInt64(uint64(m->expire_time));
            stmt.addUInt64(uint64(m->deliver_time));
            stmt.addUInt32(m->money);
            stmt.addUInt32(m->COD);
            stmt.addBool(m->checked);
            stmt.addUInt32(m->messageID);
            stmt.Execute();

            if (m->removedItems.size())
            {
                for (std::vector<uint32>::iterator itr2 = m->removedItems.begin(); itr2 != m->removedItems.end(); ++itr2)
                {
                    stmt = RealmDataDatabase.CreateStatement(deleteMailItemsByGuid, "DELETE FROM mail_items WHERE item_guid = ?");
                    stmt.PExecute(*itr2);
                }
                m->removedItems.clear();
            }
            m->state = MAIL_STATE_UNCHANGED;
        }
        else if (m->state == MAIL_STATE_DELETED)
        {
            SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteMail, "DELETE FROM mail WHERE id = ?");
            stmt.PExecute(m->messageID);

            stmt = RealmDataDatabase.CreateStatement(deleteMailItemsById, "DELETE FROM mail_items WHERE mail_id = ?");
            stmt.PExecute(m->messageID);

            if (m->HasItems())
                for (std::vector<MailItemInfo>::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
                {
                    stmt = RealmDataDatabase.CreateStatement(deleteItemInstance, "DELETE FROM item_instance WHERE guid = ?");
                    stmt.PExecute(itr2->item_guid);
                }
            if (m->itemTextId)
            {
                stmt = RealmDataDatabase.CreateStatement(deleteItemText, "DELETE FROM item_text WHERE id = ?");
                stmt.PExecute(m->itemTextId);
            }
        }
    }

    //deallocate deleted mails...
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end();)
    {
        if ((*itr)->state == MAIL_STATE_DELETED)
        {
            Mail* m = *itr;
            m_mail.erase(itr);
            delete m;
            itr = m_mail.begin();
        }
        else
            ++itr;
    }

    m_mailsUpdated = false;
}

void Player::_SaveQuestStatus()
{
    static SqlStatementID insertQuestStatus;
    static SqlStatementID updateQuestStatus;
    for (QuestStatusMap::iterator i = mQuestStatus.begin(); i != mQuestStatus.end(); ++i)
    {
        switch (i->second.uState)
        {
            case QUEST_NEW:
            {
                SqlStatement stmt = RealmDataDatabase.CreateStatement(insertQuestStatus, "INSERT INTO character_queststatus (guid, quest, status, rewarded, explored, timer, mobcount1, mobcount2, mobcount3, mobcount4, itemcount1, itemcount2, itemcount3, itemcount4) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(i->first);
                stmt.addUInt8(i->second.m_status);
                stmt.addBool(i->second.m_rewarded);
                stmt.addBool(i->second.m_explored);
                stmt.addUInt64(uint64(i->second.m_timer / 1000 + sWorld.GetGameTime()));
                stmt.addUInt32(i->second.m_creatureOrGOcount[0]);
                stmt.addUInt32(i->second.m_creatureOrGOcount[1]);
                stmt.addUInt32(i->second.m_creatureOrGOcount[2]);
                stmt.addUInt32(i->second.m_creatureOrGOcount[3]);
                stmt.addUInt32(i->second.m_itemcount[0]);
                stmt.addUInt32(i->second.m_itemcount[1]);
                stmt.addUInt32(i->second.m_itemcount[2]);
                stmt.addUInt32(i->second.m_itemcount[3]);
                stmt.Execute();

                break;
            }
            case QUEST_CHANGED:
            {
                SqlStatement stmt = RealmDataDatabase.CreateStatement(updateQuestStatus, "UPDATE character_queststatus SET status = ?, rewarded = ?, explored = ?, timer = ?, mobcount1 = ?, mobcount2 = ?, mobcount3 = ?, mobcount4 = ?, itemcount1 = ?, itemcount2 = ?, itemcount3 = ?, itemcount4 = ? WHERE guid = ? AND quest = ?");
                stmt.addUInt8(i->second.m_status);
                stmt.addBool(i->second.m_rewarded);
                stmt.addBool(i->second.m_explored);
                stmt.addUInt64(uint64(i->second.m_timer / 1000 + sWorld.GetGameTime()));
                stmt.addUInt32(i->second.m_creatureOrGOcount[0]);
                stmt.addUInt32(i->second.m_creatureOrGOcount[1]);
                stmt.addUInt32(i->second.m_creatureOrGOcount[2]);
                stmt.addUInt32(i->second.m_creatureOrGOcount[3]);
                stmt.addUInt32(i->second.m_itemcount[0]);
                stmt.addUInt32(i->second.m_itemcount[1]);
                stmt.addUInt32(i->second.m_itemcount[2]);
                stmt.addUInt32(i->second.m_itemcount[3]);
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(i->first);
                stmt.Execute();

                break;
            }
            case QUEST_UNCHANGED:
                break;
        };

        i->second.uState = QUEST_UNCHANGED;
    }
}

void Player::_SaveDailyQuestStatus()
{
    if (!m_DailyQuestChanged)
        return;

    m_DailyQuestChanged = false;

    // save last daily quest time for all quests: we need only mostly reset time for reset check anyway

    static SqlStatementID deleteDailies;
    static SqlStatementID insertDaily;

    SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteDailies, "DELETE FROM character_queststatus_daily WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());
    for (uint32 quest_daily_idx = 0; quest_daily_idx < sWorld.getConfig(CONFIG_DAILY_MAX_PER_DAY); ++quest_daily_idx)
    {
        if (GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx))
        {
            stmt = RealmDataDatabase.CreateStatement(insertDaily, "INSERT INTO character_queststatus_daily (guid, quest, time) VALUES (?, ?, ?)");
            stmt.addUInt32(GetGUIDLow());
            stmt.addUInt32(GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx));
            stmt.addUInt64(uint64(m_lastDailyQuestTime));
            stmt.Execute();
        }
    }
}

void Player::_SaveSpells()
{
    static SqlStatementID deleteSpell;
    static SqlStatementID insertSpell;
    for (PlayerSpellMap::iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        ++next;

        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.state == PLAYERSPELL_CHANGED)
        {
            SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteSpell, "DELETE FROM character_spell WHERE guid = ? and spell = ?");
            stmt.PExecute(GetGUIDLow(), itr->first);
        }
        if (itr->second.state == PLAYERSPELL_NEW || itr->second.state == PLAYERSPELL_CHANGED)
        {
            {
                //in some way we have primary-key insert errors, due to what characters are not saved (!!!)
                SqlStatement stmt = RealmDataDatabase.CreateStatement(deleteSpell, "DELETE FROM character_spell WHERE guid = ? and spell = ?");
                stmt.PExecute(GetGUIDLow(), itr->first);
            }

            SqlStatement stmt = RealmDataDatabase.CreateStatement(insertSpell, "INSERT INTO character_spell (guid, spell, slot, active, disabled) VALUES (?, ?, ?, ?, ?)");
            stmt.addUInt32(GetGUIDLow());
            stmt.addUInt32(itr->first);
            stmt.addUInt32(itr->second.slotId);
            stmt.addBool(itr->second.active);
            stmt.addBool(itr->second.disabled);
            stmt.Execute();
        }

        if (itr->second.state == PLAYERSPELL_REMOVED)
            _removeSpell(itr->first);
        else
            itr->second.state = PLAYERSPELL_UNCHANGED;
    }
}

void Player::_SaveTutorials()
{
    if (!m_TutorialsChanged)
        return;

    static SqlStatementID updateTutorial;
    static SqlStatementID insertTutorial;

    uint32 Rows=0;
    // it's better than rebuilding indexes multiple times
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT count(*) AS r FROM character_tutorial WHERE account = '%u' AND realmid = '%u'", GetSession()->GetAccountId(), realmID);
    if (result)
        Rows = result->Fetch()[0].GetUInt32();

    if (Rows)
    {
        SqlStatement stmt = RealmDataDatabase.CreateStatement(updateTutorial, "UPDATE character_tutorial SET tut0 = ?, tut1 = ?, tut2 = ? , tut3 = ?, tut4 = ?, tut5 = ?, tut6 = ?, tut7 = ? WHERE account = ? AND realmid = ?");
        stmt.addUInt32(m_Tutorials[0]);
        stmt.addUInt32(m_Tutorials[1]);
        stmt.addUInt32(m_Tutorials[2]);
        stmt.addUInt32(m_Tutorials[3]);
        stmt.addUInt32(m_Tutorials[4]);
        stmt.addUInt32(m_Tutorials[5]);
        stmt.addUInt32(m_Tutorials[6]);
        stmt.addUInt32(m_Tutorials[7]);
        stmt.addUInt32(GetSession()->GetAccountId());
        stmt.addUInt32(realmID);
        stmt.Execute();
    }
    else
    {
        SqlStatement stmt = RealmDataDatabase.CreateStatement(insertTutorial, "INSERT INTO character_tutorial (account, realmid, tut0, tut1, tut2, tut3, tut4, tut5, tut6, tut7) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        stmt.addUInt32(GetSession()->GetAccountId());
        stmt.addUInt32(realmID);
        stmt.addUInt32(m_Tutorials[0]);
        stmt.addUInt32(m_Tutorials[1]);
        stmt.addUInt32(m_Tutorials[2]);
        stmt.addUInt32(m_Tutorials[3]);
        stmt.addUInt32(m_Tutorials[4]);
        stmt.addUInt32(m_Tutorials[5]);
        stmt.addUInt32(m_Tutorials[6]);
        stmt.addUInt32(m_Tutorials[7]);
        stmt.Execute();
    }

    m_TutorialsChanged = false;
}

void Player::outDebugValues() const
{
    if (!sLog.IsOutDebug())                                  // optimize disabled debug output
        return;

    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u",GetMaxHealth(), GetMaxPower(POWER_MANA));
    sLog.outDebug("AGILITY is: \t\t%f\t\tSTRENGTH is: \t\t%f",GetStat(STAT_AGILITY), GetStat(STAT_STRENGTH));
    sLog.outDebug("INTELLECT is: \t\t%f\t\tSPIRIT is: \t\t%f",GetStat(STAT_INTELLECT), GetStat(STAT_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%f\t\tSPIRIT is: \t\t%f",GetStat(STAT_STAMINA), GetStat(STAT_SPIRIT));
    sLog.outDebug("Armor is: \t\t%u\t\tBlock is: \t\t%f",GetArmor(), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%u\t\tFireRes is: \t\t%u",GetResistance(SPELL_SCHOOL_HOLY), GetResistance(SPELL_SCHOOL_FIRE));
    sLog.outDebug("NatureRes is: \t\t%u\t\tFrostRes is: \t\t%u",GetResistance(SPELL_SCHOOL_NATURE), GetResistance(SPELL_SCHOOL_FROST));
    sLog.outDebug("ShadowRes is: \t\t%u\t\tArcaneRes is: \t\t%u",GetResistance(SPELL_SCHOOL_SHADOW), GetResistance(SPELL_SCHOOL_ARCANE));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f",GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f",GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u",GetAttackTime(BASE_ATTACK), GetAttackTime(RANGED_ATTACK));
}

/*********************************************************/
/***               FLOOD FILTER SYSTEM                 ***/
/*********************************************************/

void Player::UpdateSpeakTime()
{
    // ignore chat spam protection for GMs in any mode
    if (GetSession()->HasPermissions(PERM_GMT))
        return;

    time_t current = time (NULL);
    if (m_speakTime > current)
    {
        uint32 max_count = sWorld.getConfig(CONFIG_CHATFLOOD_MESSAGE_COUNT);
        if (!max_count)
            return;

        ++m_speakCount;
        if (m_speakCount >= max_count)
        {
            // prevent overwrite mute time, if message send just before mutes set, for example.
            if (CanSpeak())
            {
                GetSession()->m_muteRemain = sWorld.getConfig(CONFIG_CHATFLOOD_MUTE_TIME)*1000;
                GetSession()->m_muteReason = "Flood protection";
            }

            m_speakCount = 0;
        }
    }
    else
        m_speakCount = 0;

    m_speakTime = current + sWorld.getConfig(CONFIG_CHATFLOOD_MESSAGE_DELAY);
}

bool Player::CanSpeak() const
{
    return !GetSession()->m_muteRemain;
}

bool Player::IsTrollmuted() const
{
    return GetSession()->m_trollMuteRemain;
}

/*********************************************************/
/***              LOW LEVEL FUNCTIONS:Notifiers        ***/
/*********************************************************/

void Player::SendAttackSwingNotInRange()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTINRANGE, 0);
    SendPacketToSelf(&data);
}

void Player::SavePositionInDB(uint32 mapid, float x,float y,float z,float o,uint32 zone,uint64 guid)
{
    static SqlStatementID updatePosition;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(updatePosition, "UPDATE characters SET position_x = ?, position_y = ?, position_z = ?, "
                                                            "orientation = ?, map = ?, zone = ?, trans_x = '0', trans_y = '0', trans_z = '0', transguid = '0', taxi_path = '' WHERE guid = ?");

    stmt.addFloat(x);
    stmt.addFloat(y);
    stmt.addFloat(z);
    stmt.addFloat(o);
    stmt.addUInt32(mapid);
    stmt.addUInt32(zone);
    stmt.addUInt32(GUID_LOPART(guid));
    stmt.Execute();
}

void Player::SaveDataFieldToDB()
{
    static SqlStatementID updateCharData;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(updateCharData, "UPDATE characters SET data = ?  WHERE guid = ?");
    stmt.addString(GetUInt32ValuesString());
    stmt.addUInt32(GetGUIDLow());
    stmt.Execute();
}

bool Player::SaveValuesArrayInDB(Tokens const& tokens, uint64 guid)
{
    static SqlStatementID updateCharData;

    int cnt = tokens.size();
    std::ostringstream ss;
    for (int i = 0; i < cnt; ++i)
        ss << tokens[i] << " ";

    SqlStatement stmt = RealmDataDatabase.CreateStatement(updateCharData, "UPDATE characters SET data = ?  WHERE guid = ?");
    stmt.addString(ss);
    stmt.addUInt32(GUID_LOPART(guid));

    return stmt.Execute();
}

void Player::SetUInt32ValueInArray(Tokens& tokens,uint16 index, uint32 value)
{
    char buf[11];
    snprintf(buf,11,"%u",value);

    if (index >= tokens.size())
        return;

    tokens[index] = buf;
}

void Player::SetUInt32ValueInDB(uint16 index, uint32 value, uint64 guid)
{
    Tokens tokens;
    if (!LoadValuesArrayFromDB(tokens,guid))
        return;

    if (index >= tokens.size())
        return;

    char buf[11];
    snprintf(buf,11,"%u",value);
    tokens[index] = buf;

    SaveValuesArrayInDB(tokens,guid);
}

void Player::SetFloatValueInDB(uint16 index, float value, uint64 guid)
{
    uint32 temp;
    memcpy(&temp, &value, sizeof(value));
    Player::SetUInt32ValueInDB(index, temp, guid);
}

void Player::SendAttackSwingNotStanding()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTSTANDING, 0);
    SendPacketToSelf(&data);
}

void Player::SendAttackSwingDeadTarget()
{
    WorldPacket data(SMSG_ATTACKSWING_DEADTARGET, 0);
    SendPacketToSelf(&data);
}

void Player::SendAttackSwingCantAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_CANT_ATTACK, 0);
    SendPacketToSelf(&data);
}

void Player::SendAttackSwingCancelAttack()
{
    WorldPacket data(SMSG_CANCEL_COMBAT, 0);
    SendPacketToSelf(&data);
}

void Player::SendAttackSwingBadFacingAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_BADFACING, 0);
    SendPacketToSelf(&data);
}

void Player::SendAutoRepeatCancel()
{
    WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, 0);
    SendPacketToSelf(&data);
}

void Player::SendExplorationExperience(uint32 Area, uint32 Experience)
{
    WorldPacket data(SMSG_EXPLORATION_EXPERIENCE, 8);
    data << Area;
    data << Experience;
    SendPacketToSelf(&data);
}

void Player::SendDungeonDifficulty(bool IsInGroup)
{
    uint8 val = 0x00000001;
    WorldPacket data(MSG_SET_DUNGEON_DIFFICULTY, 12);
    data << (uint32)GetDifficulty();
    data << uint32(val);
    data << uint32(IsInGroup);
    SendPacketToSelf(&data);
}

void Player::SendResetFailedNotify(uint32 mapid)
{
    WorldPacket data(SMSG_RESET_FAILED_NOTIFY, 4);
    data << uint32(mapid);
    SendPacketToSelf(&data);
}

/// Reset all solo instances and optionally send a message on success for each
void Player::ResetInstances(uint8 method)
{
    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_JOIN

    // we assume that when the difficulty changes, all instances that can be reset will be
    uint8 dif = GetDifficulty();

    for (BoundInstancesMap::iterator itr = m_boundInstances[dif].begin(); itr != m_boundInstances[dif].end();)
    {
        InstanceSave *p = itr->second.save;
        const MapEntry *entry = sMapStore.LookupEntry(itr->first);
        if (!entry || !p->CanReset() || p == nullptr)
        {
            ++itr;
            continue;
        }

        if (method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (dif == DIFFICULTY_HEROIC || entry->map_type == MAP_RAID)
            {
                ++itr;
                continue;
            }
        }

        // if the map is loaded, reset it
        Map *map = sMapMgr.FindMap(p->GetMapId(), p->GetSaveInstanceId());
        if (map && map->IsDungeon())
        {
            if (!((InstanceMap*)map)->Reset(method))
            {
                ++itr;
                continue;
            }
        }

        // since this is a solo instance there should not be any players inside
        if (method == INSTANCE_RESET_ALL || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
            SendResetInstanceSuccess(p->GetMapId());

        p->DeleteFromDB();
        m_boundInstances[dif].erase(itr++);

        // the following should remove the instance save from the manager and delete it as well
        p->RemovePlayer(GetGUID());
    }
}

void Player::SendResetInstanceSuccess(uint32 MapId)
{
    WorldPacket data(SMSG_INSTANCE_RESET, 4);
    data << MapId;
    SendPacketToSelf(&data);
}

void Player::SendResetInstanceFailed(uint32 reason, uint32 MapId)
{
    // TODO: find what other fail reasons there are besides players in the instance
    WorldPacket data(SMSG_INSTANCE_RESET_FAILED, 4);
    data << reason;
    data << MapId;
    SendPacketToSelf(&data);
}

/** Implementation of hourly maximum instances per account */
bool Player::CheckInstanceCount(uint32 instanceId, bool notMaxLevel)
{
    uint32 maxCount = sWorld.isEasyRealm() ? MAX_INSTANCE_PER_ACCOUNT_PER_HOUR * 2 : MAX_INSTANCE_PER_ACCOUNT_PER_HOUR;
    if (notMaxLevel)
        maxCount *= 2;
    return isGameMaster() || sWorld.CheckInstanceCount(GetSession()->GetAccountId(), instanceId, maxCount, notMaxLevel);
}

void Player::AddInstanceEnterTime(uint32 instanceId, time_t enterTime, bool notMaxLevel)
{
    sWorld.AddInstanceEnterTime(GetSession()->GetAccountId(), instanceId, enterTime, notMaxLevel);
}

/*********************************************************/
/***              Update timers                        ***/
/*********************************************************/

void Player::UpdateAfkReport(time_t currTime)
{
    if (m_bgAfkReportedCount)
    {
        if (m_bgAfkReportedTimer <= currTime)
        {
            --m_bgAfkReportedCount;            
        }
    }
}

void Player::UpdateContestedPvP(uint32 diff)
{
    if (!m_contestedPvPTimer.GetInterval()||IsInCombat())
        return;
    if (m_contestedPvPTimer.Expired(diff))
    {
        ResetContestedPvP();
    }

}

void Player::UpdatePvPFlag(time_t currTime)
{
    if (!IsPvP())
        return;
    if (pvpInfo.endTimer == 0 || currTime < (pvpInfo.endTimer + (sWorld.isEasyRealm() ? 120 : 300)))
        return;

    UpdatePvP(false);
}

void Player::SetFFAPvP(bool state)
{
    if (state){
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
        if (sWorld.getConfig(CONFIG_FFA_DISALLOWGROUP) && !GetMap()->IsBattleGroundOrArena())
            RemoveFromGroup();
    }
    else
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
}

void Player::UpdateDuelFlag(time_t currTime)
{
    if (!duel || duel->startTimer == 0 ||currTime < duel->startTimer + 3)
        return;

    SetUInt32Value(PLAYER_DUEL_TEAM, 1);
    duel->opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 2);

    duel->startTimer = 0;
    duel->startTime  = currTime;
    duel->opponent->duel->startTimer = 0;
    duel->opponent->duel->startTime  = currTime;
}

void Player::RemovePet(Pet* pet, PetSaveMode mode, bool returnreagent, bool isDying)
{
    if (!pet)
        pet = GetPet();

    // Remove auras off unsummoned pet when owner dies
    if (isDying && !pet && GetTemporaryUnsummonedPetNumber())
        RealmDataDatabase.PExecute("DELETE FROM pet_aura WHERE guid = '%u'", GetTemporaryUnsummonedPetNumber());

    if (returnreagent && (pet || m_temporaryUnsummonedPetNumber) && !InBattleGroundOrArena())
    {
        //returning of reagents only for players, so best done here
        uint32 spellId = pet ? pet->GetUInt32Value(UNIT_CREATED_BY_SPELL) : m_oldpetspell;
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

        if (spellInfo)
        {
            for (uint32 i = 0; i < 7; ++i)
            {
                if (spellInfo->Reagent[i] > 0)
                {
                    ItemPosCountVec dest;                   //for succubus, voidwalker, felhunter and felguard credit soulshard when despawn reason other than death (out of range, logout)
                    uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, spellInfo->Reagent[i], spellInfo->ReagentCount[i]);
                    if (msg == EQUIP_ERR_OK)
                    {
                        Item* item = StoreNewItem(dest, spellInfo->Reagent[i], true, 0, "REMOVE_PET");
                        if (IsInWorld())
                            SendNewItem(item,spellInfo->ReagentCount[i],true,false);
                    }
                }
            }
        }
        m_temporaryUnsummonedPetNumber = 0;
    }

    if (!pet || pet->GetOwnerGUID()!=GetGUID())
        return;

    // only if current pet in slot
    switch (pet->getPetType())
    {
        case MINI_PET:
            m_miniPet = 0;
            break;
        case GUARDIAN_PET:
            m_guardianPets.erase(pet->GetGUID());
            break;
        case POSSESSED_PET:
            m_guardianPets.erase(pet->GetGUID());
            pet->RemoveCharmedOrPossessedBy(NULL);
            break;
        default:
            if (GetPetGUID() == pet->GetGUID())
                SetPet(NULL);
            break;
    }

    pet->CombatStop();

    if (returnreagent)
    {
        switch (pet->GetEntry())
        {
            //warlock pets except imp are removed(?) when logging out
            case 1860:
            case 1863:
            case 417:
            case 17252:
                mode = PET_SAVE_NOT_IN_SLOT;
                break;
        }
    }

    // Clear auras when owner dies
    if (isDying)
        pet->RemoveAllAuras();

    pet->SavePetToDB(mode);

    pet->AddObjectToRemoveList();
    pet->m_removed = true;

    if (pet->isControlled())
    {
        WorldPacket data(SMSG_PET_SPELLS, 8);
        data << uint64(0);
        SendPacketToSelf(&data);

        if (GetGroup())
            SetGroupUpdateFlag(GROUP_UPDATE_PET);
    }
}

void Player::RemoveMiniPet()
{
    if (Pet* pet = GetMiniPet())
    {
        pet->Remove(PET_SAVE_AS_DELETED);
        m_miniPet = 0;
    }
}

Pet* Player::GetMiniPet()
{
    if (!m_miniPet)
        return NULL;
    return ObjectAccessor::GetPet(m_miniPet);
}

void Player::RemoveGuardians()
{
    while (!m_guardianPets.empty())
    {
        uint64 guid = *m_guardianPets.begin();
        if (Pet* pet = ObjectAccessor::GetPet(guid))
            pet->Remove(PET_SAVE_AS_DELETED);

        m_guardianPets.erase(guid);
    }
}

uint32 Player::CountGuardianWithEntry(uint32 entry)
{
    uint32 cnt = 0;
    // pet guid middle part is pet_entry, IT IS NOT creature_template entry!
    // and in guardian list must be guardians with same entry _always_
    for (GuardianPetList::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end(); ++itr)
    {
        if (Unit* guardian = Unit::GetUnit(*this,*itr))
            if (guardian->GetEntry()==entry)
                ++cnt;
    }

    return cnt;
}

void Player::Uncharm()
{
    Unit* charm = GetCharm();
    if (!charm)
        return;

    if (charm->GetTypeId() == TYPEID_UNIT && ((Creature*)charm)->isPet()
        && ((Pet*)charm)->getPetType() == POSSESSED_PET)
    {
        ((Pet*)charm)->Remove(PET_SAVE_AS_DELETED);
        if (((Player*)this)->GetTemporaryUnsummonedPetNumber())
        {
            Pet* NewPet = new Pet;
            if (!NewPet->LoadPetFromDB(this, 0, ((Player*)this)->GetTemporaryUnsummonedPetNumber(), true))
                delete NewPet;
            ((Player*)this)->SetTemporaryUnsummonedPetNumber(0);
        }
    }
    else
    {
        charm->RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM);
        charm->RemoveSpellsCausingAura(SPELL_AURA_AOE_CHARM);
        charm->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS_PET);
        charm->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS);
    }

    if (GetCharmGUID())
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: CRASH ALARM! Player %s is not able to uncharm unit (Entry: %u, Type: %u)", GetName(), charm->GetEntry(), charm->GetTypeId());
    }
}

void Player::BuildPlayerChat(WorldPacket *data, uint8 msgtype, const std::string& text, uint32 language) const
{
    *data << (uint8)msgtype;
    *data << (uint32)language;
    *data << (uint64)GetGUID();
    *data << (uint32)language;                               //language 2.1.0 ?
    *data << (uint64)GetGUID();
    *data << (uint32)(text.length()+1);
    *data << text;
    *data << (uint8)chatTag();
}

void Player::Say(const std::string& text, const uint32 language, bool bad_lexics)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildPlayerChat(&data, CHAT_MSG_SAY, text, language);

    if (bad_lexics)
        SendPacketToSelf(&data);
    else
        BroadcastPacketInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_SAY),true);
}

void Player::Yell(const std::string& text, const uint32 language, bool bad_lexics)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildPlayerChat(&data, CHAT_MSG_YELL, text, language);

    if (bad_lexics)
        SendPacketToSelf(&data);
    else
        BroadcastPacketInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_YELL),true);
}

void Player::TextEmote(const std::string& text, bool bad_lexics)
{
    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildPlayerChat(&data, CHAT_MSG_EMOTE, text, LANG_UNIVERSAL);

    if (bad_lexics)
        SendPacketToSelf(&data);
    else
        BroadcastPacketInRange(&data,sWorld.getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), true, !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT));
}

void Player::Whisper(const std::string& text, uint32 language,uint64 receiver, bool bad_lexics)
{
    if (language != LANG_ADDON)                             // if not addon data
        language = LANG_UNIVERSAL;                          // whispers should always be readable

    Player *rPlayer = sObjectMgr.GetPlayerInWorld(receiver);

    std::string tmpText = text;

    // when player you are whispering to is dnd, he cannot receive your message, unless you are in gm mode
    if (!rPlayer->isDND() || isGameMaster())
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);

        if (!bad_lexics)
        {
            BuildPlayerChat(&data, CHAT_MSG_WHISPER, text, language);
            rPlayer->SendPacketToSelf(&data);
        }

        data.Initialize(SMSG_MESSAGECHAT, 200);
        rPlayer->BuildPlayerChat(&data, CHAT_MSG_REPLY, text, language);
        SendPacketToSelf(&data);
    }
    else
    {
        // announce to player that player he is whispering to is dnd and cannot receive his message
        ChatHandler(this).PSendSysMessage(LANG_PLAYER_DND, rPlayer->GetName(), rPlayer->dndMsg.c_str());

        tmpText = "receiver DND ! text: " + tmpText;
    }

    // announce to player that player he is whispering to is afk
    if (rPlayer->isAFK() && language != LANG_ADDON)
        ChatHandler(this).PSendSysMessage(LANG_PLAYER_AFK, rPlayer->GetName(), rPlayer->afkMsg.c_str());

    // if player whisper someone, auto turn of dnd to be able to receive an answer
    if (isDND() && !rPlayer->isGameMaster())
        ToggleDND();

    if (!isAcceptWhispers())
    {
        if (!isPartialWhispers())
            setPartialWhispers(true);
        if (!sSocialMgr.canWhisperToPartialWhisperGM(GetGUIDLow(), rPlayer->GetGUIDLow()))
        {
            sSocialMgr.addAllowedWhisperer(GetGUIDLow(), rPlayer->GetGUIDLow());
            ChatHandler(this).PSendSysMessage("Player %s added to your partial whisper list", rPlayer->GetName());
        }
    }
}

void Player::PetSpellInitialize()
{
    Pet* pet = GetPet();

    if (pet)
    {
        uint8 addlist = 0;

        sLog.outDebug("Pet Spells Groups");

        CreatureInfo const *cinfo = pet->GetCreatureInfo();

        // && (pet->getPetType() == HUNTER_PET || cinfo && cinfo->type == CREATURE_TYPE_DEMON && GetClass() == CLASS_WARLOCK)
        if (pet->isControlled())
        {
            for (PetSpellMap::iterator itr = pet->m_spells.begin();itr != pet->m_spells.end();++itr)
            {
                if (itr->second->state == PETSPELL_REMOVED)
                    continue;
                ++addlist;
            }
        }

        // first line + actionbar + spellcount + spells + last adds
        WorldPacket data(SMSG_PET_SPELLS, 16+40+1+4*addlist+25);

        CharmInfo *charmInfo = pet->GetCharmInfo();

                                                            //16
        data << (uint64)pet->GetGUID() << uint32(0x00000000) << uint8(pet->GetReactState()) << uint8(charmInfo->GetCommandState()) << uint16(0);

        for (uint32 i = 0; i < 10; i++)                      //40
        {
            data << uint16(charmInfo->GetActionBarEntry(i)->SpellOrAction) << uint16(charmInfo->GetActionBarEntry(i)->Type);
        }

        data << uint8(addlist);                             //1

        if (addlist && pet->isControlled())
        {
            for (PetSpellMap::iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
            {
                if (itr->second->state == PETSPELL_REMOVED)
                    continue;

                data << uint16(itr->first);
                data << uint16(itr->second->active);        // pet spell active state isn't boolean
            }
        }

        //data << uint8(0x01) << uint32(0x6010) << uint32(0x01) << uint32(0x05) << uint16(0x00);    //15
        uint8 count = 3;                                    //1+8+8+8=25

        // if count = 0, then end of packet...
        data << count;
        // uint32 value is spell id...
        // uint64 value is constant 0, unknown...
        data << uint32(0x6010) << uint64(0);                // if count = 1, 2 or 3
        //data << uint32(0x5fd1) << uint64(0);  // if count = 2
        data << uint32(0x8e8c) << uint64(0);                // if count = 3
        data << uint32(0x8e8b) << uint64(0);                // if count = 3

        SendPacketToSelf(&data);
    }
}

void Player::PossessSpellInitialize()
{
    Unit* charm = GetCharm();

    if (!charm)
        return;

    CharmInfo *charmInfo = charm->GetCharmInfo();

    if (!charmInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player::PossessSpellInitialize(): charm (%llu) has no charminfo!", charm->GetGUID());
        return;
    }

    uint8 addlist = 0;
    WorldPacket data(SMSG_PET_SPELLS, 16+40+1+4*addlist+25);// first line + actionbar + spellcount + spells + last adds

                                                            //16
    data << (uint64)charm->GetGUID() << uint32(0x00000000) << uint8(0) << uint8(0) << uint16(0);

    for (uint32 i = 0; i < 10; i++)                          //40
    {
        data << uint16(charmInfo->GetActionBarEntry(i)->SpellOrAction) << uint16(charmInfo->GetActionBarEntry(i)->Type);
    }

    data << uint8(addlist);                                 //1

    uint8 count = 3;
    data << count;
    data << uint32(0x6010) << uint64(0);                    // if count = 1, 2 or 3
    data << uint32(0x8e8c) << uint64(0);                    // if count = 3
    data << uint32(0x8e8b) << uint64(0);                    // if count = 3

    SendPacketToSelf(&data);
}

void Player::CharmSpellInitialize()
{
    Unit* charm = GetCharm();

    if (!charm)
        return;

    CharmInfo *charmInfo = charm->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player::CharmSpellInitialize(): the player's charm (%llu) has no charminfo!", charm->GetGUID());
        return;
    }

    uint8 addlist = 0;

    if (charm->GetTypeId() != TYPEID_PLAYER)
    {
        CreatureInfo const *cinfo = ((Creature*)charm)->GetCreatureInfo();

        if (cinfo && cinfo->type == CREATURE_TYPE_DEMON && GetClass() == CLASS_WARLOCK)
        {
            for (uint32 i = 0; i < CREATURE_MAX_SPELLS; ++i)
            {
                if (charmInfo->GetCharmSpell(i)->spellId)
                    ++addlist;
            }
        }
    }

    WorldPacket data(SMSG_PET_SPELLS, 16+40+1+4*addlist+25);// first line + actionbar + spellcount + spells + last adds

    data << (uint64)charm->GetGUID() << uint32(0x00000000);

    if (charm->GetTypeId() != TYPEID_PLAYER)
        data << uint8(((Creature*)charm)->GetReactState()) << uint8(charmInfo->GetCommandState());
    else
        data << uint8(0) << uint8(0);

    data << uint16(0);

    for (uint32 i = 0; i < 10; i++)                          //40
    {
        data << uint16(charmInfo->GetActionBarEntry(i)->SpellOrAction) << uint16(charmInfo->GetActionBarEntry(i)->Type);
    }

    data << uint8(addlist);                                 //1

    if (addlist)
    {
        for (uint32 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        {
            CharmSpellEntry *cspell = charmInfo->GetCharmSpell(i);
            if (cspell->spellId)
            {
                data << uint16(cspell->spellId);
                data << uint16(cspell->active);
            }
        }
    }

    uint8 count = 3;
    data << count;
    data << uint32(0x6010) << uint64(0);                    // if count = 1, 2 or 3
    data << uint32(0x8e8c) << uint64(0);                    // if count = 3
    data << uint32(0x8e8b) << uint64(0);                    // if count = 3

    SendPacketToSelf(&data);
}

int32 Player::GetTotalFlatMods(uint32 spellId, SpellModOp op)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo) return 0;
    int32 total = 0;
    for (SpellModList::iterator itr = m_spellMods[op].begin(); itr != m_spellMods[op].end(); ++itr)
    {
        SpellModifier *mod = *itr;

        if (!IsAffectedBySpellmod(spellInfo,mod))
            continue;

        if (mod->type == SPELLMOD_FLAT)
            total += mod->value;
    }
    return total;
}

int32 Player::GetTotalPctMods(uint32 spellId, SpellModOp op)
{
    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo) return 0;
    int32 total = 0;
    for (SpellModList::iterator itr = m_spellMods[op].begin(); itr != m_spellMods[op].end(); ++itr)
    {
        SpellModifier *mod = *itr;

        if (!IsAffectedBySpellmod(spellInfo,mod))
            continue;

        if (mod->type == SPELLMOD_PCT)
            total += mod->value;
    }
    return total;
}

bool Player::IsAffectedBySpellmod(SpellEntry const *spellInfo, SpellModifier *mod, Spell const* spell)
{
    if (!mod || !spellInfo)
        return false;

    if (mod->charges == -1 && mod->lastAffected)            // marked as expired but locked until spell casting finish
    {
        // prevent apply to any spell except spell that trigger expire
        if (spell)
        {
            if (mod->lastAffected != spell)
                return false;
        }
        else if (mod->lastAffected != FindCurrentSpellBySpellId(spellInfo->Id))
            return false;
    }

    return sSpellMgr.IsAffectedBySpell(spellInfo,mod->spellId,mod->effectId,mod->mask);
}

void Player::AddSpellMod(SpellModifier* mod, bool apply)
{
    Opcodes Opcode= (mod->type == SPELLMOD_FLAT) ? SMSG_SET_FLAT_SPELL_MODIFIER : SMSG_SET_PCT_SPELL_MODIFIER;

    for (int eff=0;eff<64;++eff)
    {
        uint64 _mask = uint64(1) << eff;
        if (mod->mask & _mask)
        {
            int32 val = 0;
            for (SpellModList::iterator itr = m_spellMods[mod->op].begin(); itr != m_spellMods[mod->op].end(); ++itr)
            {
                if ((*itr)->type == mod->type && (*itr)->mask & _mask)
                    val += (*itr)->value;
            }
            val += apply ? mod->value : -(mod->value);
            WorldPacket data(Opcode, (1+1+4));
            data << uint8(eff);
            data << uint8(mod->op);
            data << int32(val);
            SendPacketToSelf(&data);
        }
    }

    if (apply)
        m_spellMods[mod->op].push_back(mod);
    else
    {
        if (mod->charges == -1)
            --m_SpellModRemoveCount;
        m_spellMods[mod->op].remove(mod);
        delete mod;
    }
}

// Restore spellmods in case of failed cast
void Player::RestoreSpellMods(Spell const* spell)
{
    if (!spell || (m_SpellModRemoveCount == 0))
        return;

    for (int i=0;i<MAX_SPELLMOD;++i)
    {
        for (SpellModList::iterator itr = m_spellMods[i].begin(); itr != m_spellMods[i].end();++itr)
        {
            SpellModifier *mod = *itr;

            if (mod && mod->charges == -1 && mod->lastAffected == spell)
            {
                mod->lastAffected = NULL;
                mod->charges = 1;
                m_SpellModRemoveCount--;
            }
        }
    }
}

void Player::RemoveSpellMods(Spell const* spell)
{
    if (!spell || (m_SpellModRemoveCount == 0))
        return;

    if (spell->GetSpellEntry()->AttributesEx2 & SPELL_ATTR_EX2_AUTOREPEAT_FLAG) // use main spell, not triggered one
        spell = m_currentSpells[CURRENT_AUTOREPEAT_SPELL];

    for (int i=0;i<MAX_SPELLMOD;++i)
    {
        for (SpellModList::iterator itr = m_spellMods[i].begin(); itr != m_spellMods[i].end();)
        {
            SpellModifier *mod = *itr;
            ++itr;

            if (mod && mod->charges == -1 && (mod->lastAffected == spell || mod->lastAffected==NULL))
            {
                RemoveAurasDueToSpell(mod->spellId);
                if (m_spellMods[i].empty())
                    break;
                else
                    itr = m_spellMods[i].begin();
            }
        }
    }
}

// send Proficiency
void Player::SendProficiency(uint8 pr1, uint32 pr2)
{
    WorldPacket data(SMSG_SET_PROFICIENCY, 8);
    data << pr1 << pr2;
    SendPacketToSelf(&data);
}

void Player::RemovePetitionsAndSigns(uint64 guid, uint32 type)
{
    QueryResultAutoPtr result = QueryResultAutoPtr(NULL);
    if (type==10)
        result = RealmDataDatabase.PQuery("SELECT ownerguid,petitionguid FROM petition_sign WHERE playerguid = '%u'", GUID_LOPART(guid));
    else
        result = RealmDataDatabase.PQuery("SELECT ownerguid,petitionguid FROM petition_sign WHERE playerguid = '%u' AND type = '%u'", GUID_LOPART(guid), type);
    if (result)
    {
        do                                                  // this part effectively does nothing, since the deletion / modification only takes place _after_ the PetitionQuery. Though I don't know if the result remains intact if I execute the delete query beforehand.
        {                                                   // and SendPetitionQueryOpcode reads data from the DB
            Field *fields = result->Fetch();
            uint64 ownerguid   = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, HIGHGUID_PLAYER);
            uint64 petitionguid = MAKE_NEW_GUID(fields[1].GetUInt32(), 0, HIGHGUID_ITEM);

            // send update if charter owner in game
            Player* owner = sObjectMgr.GetPlayerInWorld(ownerguid);
            if (owner)
                owner->GetSession()->SendPetitionQueryOpcode(petitionguid);

        } while (result->NextRow());

        if (type==10)
            RealmDataDatabase.PExecute("DELETE FROM petition_sign WHERE playerguid = '%u'", GUID_LOPART(guid));
        else
            RealmDataDatabase.PExecute("DELETE FROM petition_sign WHERE playerguid = '%u' AND type = '%u'", GUID_LOPART(guid), type);
    }

    RealmDataDatabase.BeginTransaction();
    if (type == 10)
    {
        RealmDataDatabase.PExecute("DELETE FROM petition WHERE ownerguid = '%u'", GUID_LOPART(guid));
        RealmDataDatabase.PExecute("DELETE FROM petition_sign WHERE ownerguid = '%u'", GUID_LOPART(guid));
    }
    else
    {
        RealmDataDatabase.PExecute("DELETE FROM petition WHERE ownerguid = '%u' AND type = '%u'", GUID_LOPART(guid), type);
        RealmDataDatabase.PExecute("DELETE FROM petition_sign WHERE ownerguid = '%u' AND type = '%u'", GUID_LOPART(guid), type);
    }
    RealmDataDatabase.CommitTransaction();
}

void Player::SetRestBonus (float rest_bonus_new)
{
    // Prevent resting on max level
    if (GetLevel() >= sWorld.getConfig(CONFIG_MAX_PLAYER_LEVEL))
        rest_bonus_new = 0;

    if (rest_bonus_new < 0)
        rest_bonus_new = 0;

    float rest_bonus_max = (float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP)*1.5/2 * (!IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1) ? sWorld.getConfig(RATE_REST_LIMIT) : 1.0f);

    if (rest_bonus_new > rest_bonus_max)
        m_rest_bonus = rest_bonus_max;
    else
        m_rest_bonus = rest_bonus_new;

    // When RAF is active do not go to other xp-bar options, just update the rest bonus exp
    if (GetByteValue(PLAYER_BYTES_2, 3) != REST_STATE_RAF)
    {
        if (m_rest_bonus > 10)
            SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_RESTED);             // Set Reststate = Rested
        else if (m_rest_bonus <= 1)
            SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_NORMAL);             // Set Reststate = Normal
    }

    //RestTickUpdate
    SetUInt32Value(PLAYER_REST_STATE_EXPERIENCE, uint32(m_rest_bonus));
}

void Player::HandleStealthedUnitsDetection()
{
    std::list<Unit*> stealthedUnits;
    Hellground::AnyStealthedCheck u_check;
    Hellground::UnitListSearcher<Hellground::AnyStealthedCheck > searcher(stealthedUnits, u_check);

    Cell::VisitAllObjects(this, searcher, MAX_PLAYER_STEALTH_DETECT_RANGE);

    // NYI
    //WorldObject const* viewPoint = GetCamera().GetBody();

    for (std::list<Unit*>::const_iterator i = stealthedUnits.begin(); i != stealthedUnits.end(); ++i)
    {
        if ((*i) == this)
            continue;

        bool hasAtClient = HaveAtClient((*i));
        bool hasDetected = canSeeOrDetect(*i, GetCamera().GetBody(), true);

        if (hasDetected)
        {
            if (!hasAtClient)
            {
                (*i)->SendCreateUpdateToPlayer(this);
                m_clientGUIDs.insert((*i)->GetGUID());

                // target aura duration for caster show only if target exist at caster client
                // send data at target visibility change (adding to client)
                SendInitialVisiblePackets(*i);
            }
        }
        else
        {
            if (hasAtClient)
            {
                (*i)->DestroyForPlayer(this);
                m_clientGUIDs.erase((*i)->GetGUID());
            }
        }
    }
}

bool Player::ActivateTaxiPathTo(std::vector<uint32> const& nodes, uint32 mount_id, Creature* npc)
{
    if (nodes.size() < 2)
        return false;

    // not let cheating with start flight mounted
    if (IsMounted())
    {
        WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
        data << uint32(ERR_TAXIPLAYERALREADYMOUNTED);
        SendPacketToSelf(&data);
        return false;
    }

    if (GetSession()->isLogingOut() || IsInCombat() || HasUnitState(UNIT_STAT_STUNNED) || HasUnitState(UNIT_STAT_ROOT))
    {
        WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
        data << uint32(ERR_TAXIPLAYERBUSY);
        SendPacketToSelf(&data);
        return false;
    }

    if (m_ShapeShiftFormSpellId && m_form != FORM_BATTLESTANCE && m_form != FORM_BERSERKERSTANCE && m_form != FORM_DEFENSIVESTANCE && m_form != FORM_SHADOW)
    {
        WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
        data << uint32(ERR_TAXIPLAYERSHAPESHIFTED);
        SendPacketToSelf(&data);
        return false;
    }

    // not let cheating with start flight in time of logout process || if casting not finished || while in combat || if not use Spell's with EffectSendTaxi
    if (GetSession()->isLogingOut() ||
        (!m_currentSpells[CURRENT_GENERIC_SPELL] ||
        m_currentSpells[CURRENT_GENERIC_SPELL]->GetSpellEntry()->Effect[0] != SPELL_EFFECT_SEND_TAXI)&&
        IsNonMeleeSpellCast(false) ||
        IsInCombat())
    {
        WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
        data << uint32(ERR_TAXIPLAYERBUSY);
        SendPacketToSelf(&data);
        return false;
    }

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return false;

    uint32 sourcenode = nodes[0];

    // starting node too far away (cheat?)
    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(sourcenode);
    if (!node || node->map_id != GetMapId() ||
        (node->x - GetPositionX())*(node->x - GetPositionX())+
        (node->y - GetPositionY())*(node->y - GetPositionY())+
        (node->z - GetPositionZ())*(node->z - GetPositionZ()) >
        (2*INTERACTION_DISTANCE)*(2*INTERACTION_DISTANCE)*(2*INTERACTION_DISTANCE))
    {
        WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
        data << uint32(ERR_TAXIUNSPECIFIEDSERVERERROR);
        SendPacketToSelf(&data);
        return false;
    }

    // Prepare to flight start now

    // stop combat at start taxi flight if any
    CombatStop();

    // stop trade (client cancel trade at taxi map open but cheating tools can be used for reopen it)
    TradeCancel(true);

    // clean not finished taxi path if any
    CleanupAfterTaxiFlight();

    // 0 element current node
    m_taxi.AddTaxiDestination(sourcenode);

    // fill destinations path tail
    uint32 sourcepath = 0;
    uint32 totalcost = 0;

    uint32 prevnode = sourcenode;
    uint32 lastnode = 0;

    for (uint32 i = 1; i < nodes.size(); ++i)
    {
        uint32 path, cost;

        lastnode = nodes[i];
        sObjectMgr.GetTaxiPath(prevnode, lastnode, path, cost);

        if (!path)
        {
            CleanupAfterTaxiFlight();
            return false;
        }

        totalcost += cost;

        if (prevnode == sourcenode)
            sourcepath = path;

        m_taxi.AddTaxiDestination(lastnode);

        prevnode = lastnode;
    }

    if (!mount_id)                                           // if not provide then attempt use default.
        mount_id = sObjectMgr.GetTaxiMount(sourcenode, GetTeam());

    if (mount_id == 0 || sourcepath == 0)
    {
        WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
        data << uint32(ERR_TAXIUNSPECIFIEDSERVERERROR);
        SendPacketToSelf(&data);
        CleanupAfterTaxiFlight();
        return false;
    }

    uint32 money = GetMoney();

    if (npc)
    {
        totalcost = (uint32)ceil(totalcost*GetReputationPriceDiscount(npc));
    }

    if (money < totalcost)
    {
        WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
        data << uint32(ERR_TAXINOTENOUGHMONEY);
        SendPacketToSelf(&data);
        CleanupAfterTaxiFlight();
        return false;
    }

    //Checks and preparations done, DO FLIGHT
    ModifyMoney(-(int32)totalcost);

    // prevent stealth flight
    //RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);

    WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);
    data << uint32(ERR_TAXIOK);
    SendPacketToSelf(&data);

    sLog.outDebug("WORLD: Sent SMSG_ACTIVATETAXIREPLY");

    GetSession()->SendDoFlight(mount_id, sourcepath);

    return true;
}

void Player::CleanupAfterTaxiFlight()
{
    m_taxi.ClearTaxiDestinations();        // not destinations, clear source node
    Unmount();
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_TAXI_FLIGHT);
    getHostileRefManager().setOnlineOfflineState(true);
}

void Player::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{                                                // last check 2.0.10
    WorldPacket data(SMSG_SPELL_COOLDOWN, 8+1+m_spells.size()*8);
    data << GetGUID();
    data << uint8(0x0);                                     // flags (0x1, 0x2)
    time_t curTime = time(NULL);
    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second.state == PLAYERSPELL_REMOVED)
            continue;
        uint32 unSpellId = itr->first;
        SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(unSpellId);
        if (!spellInfo)
        {
            ASSERT(spellInfo);
            continue;
        }

        // Not send cooldown for this spells
        if (spellInfo->Attributes & SPELL_ATTR_DISABLED_WHILE_ACTIVE)
            continue;

        if (spellInfo->PreventionType != SPELL_PREVENTION_TYPE_SILENCE)
            continue;

        if ((idSchoolMask & SpellMgr::GetSpellSchoolMask(spellInfo)) && GetSpellCooldownDelay(unSpellId) <= (unTimeMs/1000))
        {
            data << unSpellId;
            data << unTimeMs;                               // in m.secs
            AddSpellCooldown(unSpellId, curTime + unTimeMs/1000);
        }
    }
    SendPacketToSelf(&data);
}

void Player::InitDataForForm(bool reapplyMods)
{
    SpellShapeshiftEntry const* ssEntry = sSpellShapeshiftStore.LookupEntry(m_form);
    if (ssEntry && ssEntry->attackSpeed)
    {
        SetAttackTime(BASE_ATTACK,ssEntry->attackSpeed);
        SetAttackTime(OFF_ATTACK,ssEntry->attackSpeed);
        SetAttackTime(RANGED_ATTACK, BASE_ATTACK_TIME);
    }
    else
        SetRegularAttackTime();

    switch (m_form)
    {
        case FORM_CAT:
        {
            if (getPowerType()!=POWER_ENERGY)
                setPowerType(POWER_ENERGY);
            break;
        }
        case FORM_BEAR:
        case FORM_DIREBEAR:
        {
            if (getPowerType()!=POWER_RAGE)
                setPowerType(POWER_RAGE);
            break;
        }
        default:                                            // 0, for example
        {
            ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(GetClass());
            if (cEntry && cEntry->powerType < MAX_POWERS && uint32(getPowerType()) != cEntry->powerType)
                setPowerType(Powers(cEntry->powerType));
            break;
        }
    }

    // update auras at form change, ignore this at mods reapply (.reset stats/etc) when form not change.
    if (!reapplyMods)
        UpdateEquipSpellsAtFormChange();

    UpdateAllStats();
}

// Return true is the bought item has a max count to force refresh of window by caller
bool Player::BuyItemFromCustomVendor(uint64 vendorguid, uint32 item, uint8 count, uint64 bagguid, uint8 slot, uint32 creature_entry)
{
    if (creature_entry == 693017 && IsPlayerCustomFlagged(PL_CUSTOM_NEWBIE_QUEST_LOCK) && item != BUFF_SCROLL)
    {
        ChatHandler(this).PSendSysMessage(16700);
        return false;
    }
    
    // buy item @!leg_vendor
    uint32 price = 0;

    std::vector<CustomVendor> const* items = sWorld.GetCustomVendorItems(creature_entry);
    if (!items)
    {
        return false;
    }

    auto it = std::find_if(items->begin(), items->end(), [item](const CustomVendor& vendor) {
        return vendor.item == item;
    });

    if (it == items->end()) {
        return false;
    }

    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(item);
    if (!pProto)
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, NULL, item, 0);
        return false;
    }

    if (!HasItemCount(it->need_item1, it->need_count1 * count) || it->need_item2 && !HasItemCount(it->need_item2, it->need_count2 * count))
    {
        std::string item_name1 = GetItemLink(it->need_item1);
        
        if (it->need_item2)
        {
            std::string item_name2 = GetItemLink(it->need_item2);
            ChatHandler(this).PSendSysMessage(16627, item_name1.c_str(), it->need_count1 * count, item_name2.c_str(), it->need_count2 * count);
        }
        else
            ChatHandler(this).PSendSysMessage(16628, item_name1.c_str(), it->need_count1 * count);

        SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, NULL, item, 0);
        return false;
    }

    uint8 bag = 0;                                          // init for case invalid bagGUID

    if (bagguid != NULL_BAG && slot != NULL_SLOT)
    {
        Bag* pBag;
        if (bagguid == GetGUID())
        {
            bag = INVENTORY_SLOT_BAG_0;
        }
        else
        {
            for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            {
                pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pBag)
                {
                    if (bagguid == pBag->GetGUID())
                    {
                        bag = i;
                        break;
                    }
                }
            }
        }
    }

    if (IsInventoryPos(bag, slot) || (bagguid == NULL_BAG && slot == NULL_SLOT))
    {
        ItemPosCountVec dest;
        uint8 msg = CanStoreNewItem(bag, slot, dest, item, pProto->BuyCount * count);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL);
            return false;
        }
        
        DestroyItemCount(it->need_item1, it->need_count1 * count, true, false, "VENDOR_BUY_DESTROY");

        if (it->need_item2)
            DestroyItemCount(it->need_item2, it->need_count2 * count, true, false, "VENDOR_BUY_DESTROY");

        if (Item* it = StoreNewItem(dest, item, true, 0, "VENDOR_BUY"))
        {
            WorldPacket data(SMSG_BUY_ITEM, (8 + 4 + 4 + 4));
            data << vendorguid;
            data << (uint32)(1);                // numbered from 1 at client
            data << (uint32)(0xFFFFFFFF);
            data << (uint32)count;
            SendPacketToSelf(&data);

            SendNewItem(it, pProto->BuyCount * count, true, false, false);
        }
    }
    else if (IsEquipmentPos(bag, slot))
    {
        if (pProto->BuyCount * count != 1)
        {
            SendEquipError(EQUIP_ERR_ITEM_CANT_BE_EQUIPPED, NULL, NULL);
            return false;
        }

        uint16 dest;
        uint8 msg = CanEquipNewItem(slot, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL);
            return false;
        }

        DestroyItemCount(it->need_item1, it->need_count1 * count, true, false, "VENDOR_BUY_DESTROY");

        if (it->need_item2)
            DestroyItemCount(it->need_item2, it->need_count2 * count, true, false, "VENDOR_BUY_DESTROY");

        if (Item* it = EquipNewItem(dest, item, true))
        {
            WorldPacket data(SMSG_BUY_ITEM, (8 + 4 + 4 + 4));
            data << vendorguid;
            data << (uint32)(1);                // numbered from 1 at client
            data << (uint32)(0xFFFFFFFF);
            data << (uint32)count;
            SendPacketToSelf(&data);

            SendNewItem(it, pProto->BuyCount * count, true, false, false);

            AutoUnequipOffhandIfNeed();
        }
    }
    else
    {
        SendEquipError(EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT, NULL, NULL);
        return false;
    }

    if (creature_entry == NPC_GRITHENA && item == BUFF_SCROLL)
        KilledMonster(690709, 0, 693033);


    return true;
}

bool Player::BuyItemFromVendor(uint64 vendorguid, uint32 item, uint8 count, uint64 bagguid, uint8 slot)
{
    // cheating attempt
    if (count < 1) count = 1;

    // cheating attempt
    if (slot > MAX_BAG_SIZE && slot != NULL_SLOT)
        return false;

    if (!isAlive())
        return false;

    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(item);
    if (!pProto)
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, NULL, item, 0);
        return false;
    }

    if (!vendorguid)
    {
        sLog.outDebug("WORLD: BuyItemFromVendor - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(vendorguid)));
        SendBuyError(BUY_ERR_DISTANCE_TOO_FAR, NULL, item, 0);
        return false;
    }

    Creature *pCreature = GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
    if (!pCreature)
    {
        if (vendorguid == GetTransmogManager()->GetItemGUID())
        {
            GetTransmogManager()->HandleSelectTransmogPacket(item);
            return true;
        }
        sLog.outDebug("WORLD: BuyItemFromVendor - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(vendorguid)));
        SendBuyError(BUY_ERR_DISTANCE_TOO_FAR, NULL, item, 0);
        return false;
    }

    if(pCreature->GetEntry() == 27914)
    {
        if(pCreature->GetCharmerOrOwnerGUID() != this->GetGUID())
        {
            SendBuyError(BUY_ERR_SELLER_DONT_LIKE_YOU, pCreature, item, 0);
            return false;
        }
    }
    //else if (pCreature->GetEntry() == 693092) // Blue Qiraji only for guild members
    //{
    //    Guild* guild = sGuildMgr.GetGuildById(GetGuildId());
    //    
    //    if (!guild || guild->GetLeader() != GetGUID() || !IsGuildHouseOwnerMember())
    //    {
    //        SendBuyError(BUY_ERR_SELLER_DONT_LIKE_YOU, pCreature, item, 0);
    //        return false;
    //    }
    //}

    // send and return @!leg_vendor
    if (pCreature->GetEntry() == 693127 || pCreature->GetEntry() == 693017)
        return BuyItemFromCustomVendor(vendorguid, item, count, bagguid, slot, pCreature->GetEntry());

    VendorItemData const* vItems;

    uint32 multivendor_id = 0;
    auto& mv = sWorld.multivendors;
    auto it = mv.find(pCreature->GetEntry());
    if (it != mv.end())
    {
        for (auto& vendors : it->second)
        {
            // yes, i need to check every vendor_id for multivendor... so bad
            // dont add same items to one vendor due to cost bugs!!!
            VendorItemData const* tmpItem = sObjectMgr.GetNpcVendorItemList(vendors.first);
            if (tmpItem && tmpItem->FindItem(item))
            {
                vItems = tmpItem;
                multivendor_id = vendors.first;
                break;
            }
        }
    }
    else
        vItems = pCreature->GetVendorItems();

    if (!vItems || vItems->Empty())
    {
        if (pCreature->isInnkeeper() || pCreature->GetScriptName() == "DeathSide_npc_shop") // Donations. Donation vendors have no items
        {
            sWorld.GetShop()->HandleBuyShopPacket(pCreature, this, item, pProto, count);
            return true;
        }
        //else if (pCreature->GetScriptName() == "DeathSide_pvp_master") // PvP Master
        //{
        //    sBattleGroundMgr.ArenaRestrictedHandleBuyPacket(pCreature, this, item, pProto);
        //    return true;
        //}
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    size_t vendor_slot = vItems->FindItemSlot(item);
    if (vendor_slot >= vItems->GetItemCount())
    {
        if (pCreature->isInnkeeper() || pCreature->GetScriptName() == "DeathSide_npc_shop") // Donations. Donation vendors have no items
        {
            sWorld.GetShop()->HandleBuyShopPacket(pCreature, this, item, pProto, count);
            return true;
        }
        //else if (pCreature->GetScriptName() == "DeathSide_pvp_master") // PvP Master
        //{
        //    sBattleGroundMgr.ArenaRestrictedHandleBuyPacket(pCreature, this, item, pProto);
        //    return true;
        //}
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    VendorItem const* crItem = vItems->m_items[vendor_slot];

    // check current item amount if it limited
    if (crItem->maxcount != 0)
    {
        if (pCreature->GetVendorItemCurrentCount(crItem) < pProto->BuyCount * count)
        {
            SendBuyError(BUY_ERR_ITEM_ALREADY_SOLD, pCreature, item, 0);
            return false;
        }
    }

	// captcha for recipes with restock time
	if (sWorld.getConfig(CONFIG_CAPTCHA_ENABLED))
	{
		if (crItem->incrtime > 0 && pProto->InventoryType == 0 && pProto->Class == 9)
		{
			if (NeedCaptcha(CAPTCHA_BUY_VENDOR_RESTOCKED))
			{
				//SendBuyError(BUY_ERR_SELLER_DONT_LIKE_YOU, pCreature, item, 0);
				return false;
			}
		}
	}

	uint32 price = 0;
	bool boj_equip = false;

	if (!sWorld.getConfig(CONFIG_IS_BETA))
	{
		if (uint32(m_reputationMgr.GetRank(pProto->RequiredReputationFaction)) < pProto->RequiredReputationRank)
		{
			SendBuyError(BUY_ERR_REPUTATION_REQUIRE, pCreature, item, 0);
			return false;
		}
		
		if (crItem->ExtendedCost)
		{
			ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(crItem->ExtendedCost);
			if (!iece)
			{
				sLog.outLog(LOG_DEFAULT, "ERROR: Item %u have wrong ExtendedCost field value %u", pProto->ItemId, crItem->ExtendedCost);
				return false;
			}

			// honor points price
			if (GetHonorPoints() < (iece->reqhonorpoints * count))
			{
				SendEquipError(EQUIP_ERR_NOT_ENOUGH_HONOR_POINTS, NULL, NULL);
				return false;
			}

			// arena points price
			if (GetArenaPoints() < (iece->reqarenapoints * count))
			{
				SendEquipError(EQUIP_ERR_NOT_ENOUGH_ARENA_POINTS, NULL, NULL);
				return false;
			}

			// item base price
			for (uint8 i = 0; i < 5; ++i)
			{
				bool has = false;

				if (iece->reqitem[i])
				{
					// if BOJ, add second (personal BOJ)
					// available only for buy equipment
					if (iece->reqitem[i] == 29434 && pProto->Bonding == 1)
					{
						if (GetItemCount(29434) + GetItemCount(SECOND_BADGE) >= iece->reqitemcount[i] * count)
							has = true;

						boj_equip = true;
					}
					else
					{
						has = HasItemCount(iece->reqitem[i], (iece->reqitemcount[i] * count));
					}

					// pCreature if has aura or not different extendedcosts
					if (!has)
					{
						if (GetItemCount(SECOND_BADGE) && iece->reqitem[i] == 29434 && pProto->Bonding == 0)
							ChatHandler(this).SendSysMessage(15498);

						SendEquipError(EQUIP_ERR_VENDOR_MISSING_TURNINS, NULL, NULL);
						return false;
					}
				}
			}

			// check for personal arena rating requirement
			if (GetMaxPersonalArenaRatingRequirement() < iece->reqpersonalarenarating)
			{
				// probably not the proper equip err
				SendEquipError(EQUIP_ERR_CANT_EQUIP_RANK, NULL, NULL);
				return false;
			}
		}

		if (pProto->BuyPrice)
		{
			uint32 maxCount = MAX_MONEY_AMOUNT / pProto->BuyPrice; //why price is int32? can be negative? 
			if ((uint32)count > maxCount)
			{
				sLog.outLog(LOG_CHEAT, "Player %s tried to buy %u item id %u, causing overflow", GetName(), (uint32)count, pProto->ItemId);
				count = (uint8)maxCount; // set count to max possible to buy without overflow of price
			}
		}
		price = pProto->BuyPrice * count; //it should not exceed 0xFFFFFFFF

		// reputation discount
		price = uint32(floor(price * GetReputationPriceDiscount(pCreature)));

        if (GetMoney() < price)
		{
			SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, item, 0);
			return false;
		}
	}

    uint8 bag = 0;                                          // init for case invalid bagGUID

    if (bagguid != NULL_BAG && slot != NULL_SLOT)
    {
        Bag *pBag;
        if (bagguid == GetGUID())
        {
            bag = INVENTORY_SLOT_BAG_0;
        }
        else
        {
            for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END;i++)
            {
                pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0,i);
                if (pBag)
                {
                    if (bagguid == pBag->GetGUID())
                    {
                        bag = i;
                        break;
                    }
                }
            }
        }
    }

    if (IsInventoryPos(bag, slot) || (bagguid == NULL_BAG && slot == NULL_SLOT))
    {
        ItemPosCountVec dest;
        uint8 msg = CanStoreNewItem(bag, slot, dest, item, pProto->BuyCount * count);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL);
            return false;
        }

        ModifyMoney(-(int32)price);
        if (crItem->ExtendedCost)                            // case for new honor system
        {
            ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(crItem->ExtendedCost);
            if (iece->reqhonorpoints)
                ModifyHonorPoints(- int32(iece->reqhonorpoints * count));
            if (iece->reqarenapoints)
                ModifyArenaPoints(- int32(iece->reqarenapoints * count));
            for (uint8 i = 0; i < 5; ++i)
            {
                if (iece->reqitem[i])
                {
                    // if BOJ, add second (personal BOJ)
                    if (iece->reqitem[i] == 29434)
                    {
                        uint32 boj_count = GetItemCount(29434);
                        uint32 boj_second_count = GetItemCount(SECOND_BADGE);
                        uint32 boj_total = boj_count + boj_second_count;

                        uint32 boj_required = iece->reqitemcount[i] * count;

                        uint32 boj_to_destroy = 0;
                        uint32 boj_second_to_destroy = 0;

                        //if (boj_required < boj_total)
                        //    sLog.outLog(LOG_SPECIAL, "Player %s --- ERROR! in boj_required < boj_total", GetName());

                        do {
                            if (boj_second_count > 0 && boj_second_to_destroy < boj_second_count && boj_equip)
                            {
                                ++boj_second_to_destroy;
                            }
                            else if (boj_count > 0 && boj_to_destroy < boj_count)
                            {
                                ++boj_to_destroy;
                            }
                            --boj_required;
                        } while (boj_required > 0);

                        if (boj_second_to_destroy)
                            DestroyItemCount(SECOND_BADGE, boj_second_to_destroy, true, false, "VENDOR_BUY_DESTROY");

                        if (boj_to_destroy)
                            DestroyItemCount(iece->reqitem[i], boj_to_destroy, true, false, "VENDOR_BUY_DESTROY");

                        if (!boj_to_destroy && !boj_second_to_destroy)
                            sLog.outLog(LOG_SPECIAL, "Player %s --- ERROR! in !boj_to_destroy && !boj_second_to_destroy", GetName());
                    }
                    else
                    {
                        DestroyItemCount(iece->reqitem[i], (iece->reqitemcount[i] * count), true, false, "VENDOR_BUY_DESTROY");
                    }
                }
            }
        }

        if (Item *it = StoreNewItem(dest, item, true, 0, "VENDOR_BUY"))
        {
            uint32 new_count = pCreature->UpdateVendorItemCurrentCount(crItem,pProto->BuyCount * count);

            WorldPacket data(SMSG_BUY_ITEM, (8+4+4+4));
            data << pCreature->GetGUID();
            data << (uint32)(vendor_slot+1);                // numbered from 1 at client
            data << (uint32)(crItem->maxcount > 0 ? new_count : 0xFFFFFFFF);
            data << (uint32)count;
            SendPacketToSelf(&data);

            SendNewItem(it, pProto->BuyCount*count, true, false, false);
        }
    }
    else if (IsEquipmentPos(bag, slot))
    {
        if (pProto->BuyCount * count != 1)
        {
            SendEquipError(EQUIP_ERR_ITEM_CANT_BE_EQUIPPED, NULL, NULL);
            return false;
        }

        uint16 dest;
        uint8 msg = CanEquipNewItem(slot, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL);
            return false;
        }

        ModifyMoney(-(int32)price);
        if (crItem->ExtendedCost)                            // case for new honor system
        {
            ItemExtendedCostEntry const* iece = sItemExtendedCostStore.LookupEntry(crItem->ExtendedCost);
            if (iece->reqhonorpoints)
                ModifyHonorPoints(- int32(iece->reqhonorpoints));
            if (iece->reqarenapoints)
                ModifyArenaPoints(- int32(iece->reqarenapoints));
            for (uint8 i = 0; i < 5; ++i)
            {
                if (iece->reqitem[i])
                {
                    // if BOJ, add second (personal BOJ)
                    if (iece->reqitem[i] == 29434)
                    {
                        uint32 boj_count = GetItemCount(29434);
                        uint32 boj_second_count = GetItemCount(SECOND_BADGE);
                        uint32 boj_total = boj_count + boj_second_count;

                        uint32 boj_required = iece->reqitemcount[i];

                        uint32 boj_to_destroy = 0;
                        uint32 boj_second_to_destroy = 0;

                        do {
                            if (boj_second_count > 0 && boj_second_to_destroy < boj_second_count && boj_equip)
                            {
                                ++boj_second_to_destroy;
                            } 
                            else if (boj_count > 0 && boj_to_destroy < boj_count)
                            {
                                ++boj_to_destroy;
                            }
                            --boj_required;
                        } while (boj_required > 0);

                        if (boj_second_to_destroy)
                            DestroyItemCount(SECOND_BADGE, boj_second_to_destroy, true, false, "VENDOR_BUY_DESTROY");
                        
                        if (boj_to_destroy)
                            DestroyItemCount(iece->reqitem[i], boj_to_destroy, true, false, "VENDOR_BUY_DESTROY");

                        if (!boj_to_destroy && !boj_second_to_destroy)
                            sLog.outLog(LOG_SPECIAL, "Player %s --- ERROR! if BOJ, add second (personal BOJ)", GetName());
                    }
                    else
                    {
                        DestroyItemCount(iece->reqitem[i], iece->reqitemcount[i], true, false, "VENDOR_BUY_DESTROY");
                    }
                } 
            }
        }

        if (Item *it = EquipNewItem(dest, item, true))
        {
            uint32 new_count = pCreature->UpdateVendorItemCurrentCount(crItem,pProto->BuyCount * count);

            WorldPacket data(SMSG_BUY_ITEM, (8+4+4+4));
            data << pCreature->GetGUID();
            data << uint32(vendor_slot+1);                // numbered from 1 at client
            data << uint32(crItem->maxcount > 0 ? new_count : 0xFFFFFFFF);
            data << uint32(count);
            SendPacketToSelf(&data);

            SendNewItem(it, pProto->BuyCount*count, true, false, false);

            AutoUnequipOffhandIfNeed();
        }
    }
    else
    {
        SendEquipError(EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT, NULL, NULL);
        return false;
    }

    //@!newbie_quest
    if (pCreature->GetEntry() == NPC_VIOLET)
        KilledMonster(690700, 0, 693030);
    else if (pCreature->GetEntry() == NPC_MAJOR_DUNTELO)
        KilledMonster(690701, 0, 693030);
    else if (pCreature->GetEntry() == NPC_NODALON)
    {
        if (pProto->InventoryType > 0)
            KilledMonster(690724, 0, 693031);
    }
    else if (multivendor_id == 690903) // A1
        KilledMonster(690721, 0, 693030);
    else if (multivendor_id == 690904) // Trinket
        KilledMonster(690722, 0, 693030);

    // vendor log for certain creatures
    switch (pCreature->GetEntry())
    {
    case 693074:
    case 693075:
    case 693076:
    case 693077:
    case 693078:
    case 693079:
    case 693080:
    case 693081:
    case 693082:
    case 693083:
    case 693084:
        sLog.outLog(LOG_VENDOR, "Player %s (guid: %u) bought item %u (count: %u) from vendor %u (guid: %u) for extendedcost %u and %u copper", GetName(), GetGUIDLow(), item, count, pCreature->GetEntry(), pCreature->GetGUIDLow(), crItem->ExtendedCost, price);
    }

    if (sBattleGroundMgr.ShouldArenaItemLog(item, crItem->ExtendedCost))
    {
        static SqlStatementID arenaLogItem;

        SqlStatement stmt = RealmDataDatabase.CreateStatement(arenaLogItem, "INSERT IGNORE INTO character_arena_items_log (guid, item, date) VALUES (?, ?, NOW())");
        stmt.addUInt32(GetGUIDLow());
        stmt.addUInt32(item);
        stmt.Execute();
    }

    return crItem->maxcount!=0;
}

uint32 Player::GetMaxPersonalArenaRatingRequirement()
{
    // returns the maximal personal arena rating that can be used to purchase items requiring this condition
    // the personal rating of the arena team must match the required limit as well
    // so return max[in arenateams](min(personalrating[teamtype], teamrating[teamtype]))
    uint32 max_personal_rating = 0;

    for (int i = 0; i < MAX_ARENA_SLOT; ++i)
    {
        // disallow to buy items for 5v5 rating (rated BG)
        if (i == ArenaTeam::GetSlotByType(ARENA_TEAM_5v5))
            continue;

        if (ArenaTeam * at = sObjectMgr.GetArenaTeamById(GetArenaTeamId(i)))
        {
            // uint32 p_rating = GetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (i * 6) + 5);
            uint32 p_rating = GetArenaPersonalRating(i);
            uint32 t_rating = at->GetRating();
            p_rating = p_rating < t_rating ? p_rating : t_rating;
            if (max_personal_rating < p_rating)
                max_personal_rating = p_rating;
        }
    }
    return max_personal_rating;
}

void Player::UpdateHomebindTime(uint32 time)
{
    // GMs never get homebind timer online
    if (m_InstanceValid || isGameMaster())
    {
        if (m_HomebindTimer)                                 // instance valid, but timer not reset
        {
            // hide reminder
            WorldPacket data(SMSG_RAID_GROUP_ONLY, 4+4);
            data << uint32(0);
            data << uint32(0);
            SendPacketToSelf(&data);
        }
        // instance is valid, reset homebind timer
        m_HomebindTimer = 0;
    }
    else if (m_HomebindTimer > 0)
    {
        if (time >= m_HomebindTimer)
        {
            // teleport to homebind location
            TeleportToHomebind();
        }
        else
            m_HomebindTimer -= time;
    }
    else
    {
        // timer enabled: add stun
        AddAura(55161, this); // 20 sec stun - so he DOESN'T MOVE
        // instance is invalid, start homebind timer
        m_HomebindTimer = 20000; // Blizz on TBC 60.
        // send message to player
        WorldPacket data(SMSG_RAID_GROUP_ONLY, 4+4);
        data << m_HomebindTimer;
        data << uint32(1);
        SendPacketToSelf(&data);
        sLog.outDebug("PLAYER: Player '%s' (GUID: %u) will be teleported to homebind in 20 seconds", GetName(),GetGUIDLow());
    }
}

void Player::UpdatePvP(bool state, bool ovrride)
{
    if (sWorld.getConfig(CONFIG_DISABLE_PVP) || sWorld.isPvPArea(GetCachedArea()))
    {
        state = false;
        ovrride = true;
        ApplyModFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP, false);
    }

    SetPvP(state);
    if (Pet* pet = GetPet())
        pet->SetPvP(state);
    if (Unit* charmed = GetCharm())
        charmed->SetPvP(state);

    if (!state || ovrride)
        pvpInfo.endTimer = 0;
    else if (!pvpInfo.inHostileArea && !HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP))
        pvpInfo.endTimer = time(NULL);

    if (sWorld.IsFFAPvPRealm())
        SetFFAPvP(state);

	// not needed?
	//else if (!state && IsFFAPvP())
	//{
	//	AreaTableEntry const* area = GetAreaEntryByAreaID(GetCachedArea());
	//	if (area && !(area->flags & AREA_FLAG_ARENA))
	//	{
	//		SetFFAPvP(false);
	//	}
	//}
    // if you are in FFA group and getting PVP flag - you should be kicked out of group and not be able to accept invite of such group or create FFA group
    // i suppose this could break FFA groups on BGs. Needs a deep thinking
    // Trentone
}

bool Player::HasSpellCooldown(uint32 spell_id) const // for normal spell
{
    SpellEntry const * entry = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
    // check if spellentry is present and if the cooldown is less than 15 mins
    if (!entry)
        return HasSpellDirectCooldown(spell_id);

    return (HasSpellDirectCooldown(spell_id) || (entry->Category ? HasCategorySpellCooldown(entry->Category) : false));
}

bool Player::HasSpellCooldown(uint32 spell_id, uint32 category) const
{
    return (HasSpellDirectCooldown(spell_id) || (category ? HasCategorySpellCooldown(category) : false));
}

bool Player::HasSpellItemCooldown(uint32 spell_id, uint32 category) const // for item case
{
    return (HasSpellItemDirectCooldown(spell_id) || (category ? HasCategorySpellCooldown(category) : false));
}

bool Player::HasSpellDirectCooldown(uint32 spell_id) const
{
    SpellCooldowns::const_iterator itr = m_spellCooldowns.find(spell_id);
    return itr != m_spellCooldowns.end() && itr->second > time(NULL);
}

bool Player::HasSpellItemDirectCooldown(uint32 spell_id) const
{
    SpellItemCooldowns::const_iterator itr = m_spellItemCooldowns.find(spell_id);
    return itr != m_spellItemCooldowns.end() && itr->second.end > time(NULL);
}

uint32 Player::GetSpellCooldownDelay(uint32 spell_id) const
{
    SpellCooldowns::const_iterator itr = m_spellCooldowns.find(spell_id);
    time_t t = time(NULL);
    return itr != m_spellCooldowns.end() && itr->second > t ? itr->second - t : 0;
}

bool Player::HasCategorySpellCooldown(uint32 category) const
{
    SpellCategoryCooldowns::const_iterator itr = m_spellCategoryCooldowns.find(category);
    return itr != m_spellCategoryCooldowns.end() && itr->second > time(NULL);
}

void Player::AddSpellCooldown(uint32 spellid, time_t end_time)
{
    m_spellCooldowns[spellid] = end_time;
}

void Player::AddSpellItemCooldown(uint32 spellid, uint32 itemid, uint32 itemSpellCategory, time_t end_time)
{
    SpellItemCooldown sc;
    sc.category = itemSpellCategory;
    sc.end = end_time;
    sc.itemid = itemid;
    m_spellItemCooldowns[spellid] = sc;
}

void Player::AddSpellCategoryCooldown(uint32 category, time_t endtime)
{
    m_spellCategoryCooldowns[category] = endtime;
}

void Player::SendCooldownEvent(SpellEntry const *spellInfo) // used for spells like nature's swiftness/stealth - start cooldown on spell disable.
{
    if (!(spellInfo->Attributes & SPELL_ATTR_DISABLED_WHILE_ACTIVE))
        return;

    uint32 cat = spellInfo->Category;
    uint32 rec = spellInfo->RecoveryTime;
    uint32 catrec = spellInfo->CategoryRecoveryTime;

    // Now we have cooldown data (if found any), time to apply mods
    if (rec > 0)
        ApplySpellMod(spellInfo->Id, SPELLMOD_COOLDOWN, rec);

    if (catrec > 0)
        ApplySpellMod(spellInfo->Id, SPELLMOD_COOLDOWN, catrec);

    // replace negative cooldowns by 0
    if (rec < 0) rec = 0;
    if (catrec < 0) catrec = 0;

    // no cooldown after applying spell mods
    if (!(rec == 0 && catrec == 0))
    {
        time_t curTime = time(NULL);

        time_t catrecTime = catrec ? curTime+catrec/1000 : 0;   // in secs
        time_t recTime    = rec ? curTime+rec/1000 : catrecTime;// in secs

        // self spell cooldown
        if (recTime > 0)
            AddSpellCooldown(spellInfo->Id, recTime);

        // category spells
        if (cat && catrec > 0)
            AddSpellCategoryCooldown(cat, catrecTime);
    }
    // Send activate
    WorldPacket data(SMSG_COOLDOWN_EVENT, (4+8));
    data << spellInfo->Id;
    data << GetGUID();
    SendPacketToSelf(&data);
}

std::string Player::SendCooldownsDebug()
{
    std::ostringstream ss;
    uint64 timethis = time(NULL);
    if (!(m_spellCooldowns.empty() && m_spellCategoryCooldowns.empty() && m_spellItemCooldowns.empty()))
    {
        for (SpellCooldowns::const_iterator itr = m_spellCooldowns.begin();itr != m_spellCooldowns.end(); ++itr)
        {
            if (itr->second > timethis)
                ss << "id: " << itr->first << " " << (itr->second - timethis) << " \n";
        }

        for (SpellCategoryCooldowns::const_iterator itr = m_spellCategoryCooldowns.begin();itr != m_spellCategoryCooldowns.end(); ++itr)
        {
            if (itr->second > timethis)
                ss << "category: " << itr->first << " " << (itr->second - timethis) << " \n";
        }

        for (SpellItemCooldowns::const_iterator itr = m_spellItemCooldowns.begin();itr != m_spellItemCooldowns.end(); ++itr)
        {
            if (itr->second.end > timethis)
                ss << "spell: " << itr->first << " item: " << itr->second.itemid << "category: " << itr->second.category << " " << (itr->second.end - timethis) << " \n";
        }
    }
    return ss.str();
}
                                                           //slot to be excluded while counting
bool Player::EnchantmentFitsRequirements(uint32 enchantmentcondition, int8 slot)
{
    if (!enchantmentcondition)
        return true;

    SpellItemEnchantmentConditionEntry const *Condition = sSpellItemEnchantmentConditionStore.LookupEntry(enchantmentcondition);

    if (!Condition)
        return true;

    uint8 curcount[4] = {0, 0, 0, 0};

    //counting current equipped gem colors
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (i == slot)
            continue;
        Item *pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem2 && !pItem2->IsBroken() && pItem2->GetProto()->Socket[0].Color)
        {
            for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
            {
                uint32 enchant_id = pItem2->GetEnchantmentId(EnchantmentSlot(enchant_slot));
                if (!enchant_id)
                    continue;

                SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                if (!enchantEntry)
                    continue;

                uint32 gemid = enchantEntry->GemID;
                if (!gemid)
                    continue;

                ItemPrototype const* gemProto = sItemStorage.LookupEntry<ItemPrototype>(gemid);
                if (!gemProto)
                    continue;

                GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
                if (!gemProperty)
                    continue;

                uint8 GemColor = gemProperty->color;

                for (uint8 b = 0, tmpcolormask = 1; b < 4; b++, tmpcolormask <<= 1)
                {
                    if (tmpcolormask & GemColor)
                        ++curcount[b];
                }
            }
        }
    }

    bool activate = true;

    for (int i = 0; i < 5; i++)
    {
        if (!Condition->Color[i])
            continue;

        uint32 _cur_gem = curcount[Condition->Color[i] - 1];

        // if have <CompareColor> use them as count, else use <value> from Condition
        uint32 _cmp_gem = Condition->CompareColor[i] ? curcount[Condition->CompareColor[i] - 1]: Condition->Value[i];

        switch (Condition->Comparator[i])
        {
            case 2:                                         // requires less <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem < _cmp_gem) ? true : false;
                break;
            case 3:                                         // requires more <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem > _cmp_gem) ? true : false;
                break;
            case 5:                                         // requires at least <color> than (<value> || <comparecolor>) gems
                activate &= (_cur_gem >= _cmp_gem) ? true : false;
                break;
        }
    }

    sLog.outDebug("Checking Condition %u, there are %u Meta Gems, %u Red Gems, %u Yellow Gems and %u Blue Gems, Activate:%s", enchantmentcondition, curcount[0], curcount[1], curcount[2], curcount[3], activate ? "yes" : "no");

    return activate;
}

void Player::CorrectMetaGemEnchants(uint8 exceptslot, bool apply)
{
                                                            //cycle all equipped items
    for (uint32 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        //enchants for the slot being socketed are handled by Player::ApplyItemMods
        if (slot == exceptslot)
            continue;

        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        if (!pItem || !pItem->GetProto()->Socket[0].Color)
            continue;

        for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
        {
            uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if (!enchant_id)
                continue;

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (!enchantEntry)
                continue;

            uint32 condition = enchantEntry->EnchantmentCondition;
            if (condition)
            {
                                                            //was enchant active with/without item?
                bool wasactive = EnchantmentFitsRequirements(condition, apply ? exceptslot : -1);
                                                            //should it now be?
                if (wasactive ^ EnchantmentFitsRequirements(condition, apply ? -1 : exceptslot))
                {
                    // ignore item gem conditions
                                                            //if state changed, (dis)apply enchant
                    ApplyEnchantment(pItem,EnchantmentSlot(enchant_slot),!wasactive,true,true);
                }
            }
        }
    }
}

                                                            //if false -> then toggled off if was on| if true -> toggled on if was off AND meets requirements
void Player::ToggleMetaGemsActive(uint8 exceptslot, bool apply)
{
    //cycle all equipped items
    for (int slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        //enchants for the slot being socketed are handled by WorldSession::HandleSocketOpcode(WorldPacket& recv_data)
        if (slot == exceptslot)
            continue;

        Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, slot);

        if (!pItem || !pItem->GetProto()->Socket[0].Color)   //if item has no sockets or no item is equipped go to next item
            continue;

        //cycle all (gem)enchants
        for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT+3; ++enchant_slot)
        {
            uint32 enchant_id = pItem->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if (!enchant_id)                                 //if no enchant go to next enchant(slot)
                continue;

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (!enchantEntry)
                continue;

            //only metagems to be (de)activated, so only enchants with condition
            uint32 condition = enchantEntry->EnchantmentCondition;
            if (condition)
                ApplyEnchantment(pItem,EnchantmentSlot(enchant_slot), apply);
        }
    }
}

void Player::LeaveBattleground(bool teleportToEntryPoint)
{
    ClearComboPoints();
    
	if (IsSpectator())
		SetSpectator(false);
	
	if (BattleGround *bg = GetBattleGround())
    {
        if (bg->isArena() && !isGameMaster())
        {
            SetVisibility(VISIBILITY_ON);
            SetFlying(false);

            GetCamera().ResetView(true);

            //restore buff
            if (IsGuildHouseOwnerMember())
                AddAura(SPELL_GUILD_HOUSE_STATS_BUFF, this);
        }

        bool punish = bg->isBattleGround() && !isGameMaster() && (bg->GetStatus() == STATUS_IN_PROGRESS || bg->GetStatus() == STATUS_WAIT_JOIN);
        bg->RemovePlayerAtLeave(GetGUID(), teleportToEntryPoint, true, punish);
    }
}

const char* Player::GetBattlegroundJoinError(ArenaType arenatype, bool at_teleport, WorldSession* to_leader)
{
    if (isGameMaster())
        return nullptr;
    
    // check Deserter debuff or Resurrection Sickness
    //if (GetDummyAura(15007))
    //{
    //    return false;
    //}

    WorldSession* session = to_leader ? to_leader : GetSession();
    if (!session)
        return "SESSION ERROR!";

    if (arenatype)
    {
        if (!sWorld.isEasyRealm())
            return nullptr;

        if (sBattleGroundMgr.GetDebugArenaId())
            return nullptr;

        //if (RestrictedLegendaryEquipped())
        //    return session->GetHellgroundString(16625);

        if (arenatype == ARENA_TYPE_3v3)
        {
            uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_3v3);

            if (sWorld.m_arenareports.count(GetGUID()) >= MAX_ARENA_REPORTS)
            {
                AddAura(SPELL_ARENA_DESERTER, this, DESERTER_ARENA_DURATION_REPORT);
                sLog.outLog(LOG_ARENA, "ArenaReport: Player %s (%u) banned", GetName(), GetGUIDLow());
                sWorld.m_arenareports.erase(GetGUID());
                return session->GetHellgroundString(LANG_SQ_HAVE_DESERTER);
            }

            if (!at_teleport && !CanAddArenaQueueHashIP(1))
                return session->GetHellgroundString(16683);

            // check Deserter debuff
            if (sWorld.getConfig(CONFIG_SOLO_3V3_CAST_DESERTER) && HasAura(SPELL_ARENA_DESERTER) && !getBattleGroundMgr()->GetDebugArenaId())
            {
                // draw existing arena
                if (at_teleport)
                {
                    BattleGround* bg = GetBattleGround();
                    if (bg && bg->GetStatus() > STATUS_IN_PROGRESS)
                    {
                        bg->SetRated(false);
                        bg->m_ShouldEndTime = 1000;
                    } 
                }
                
                return session->GetHellgroundString(LANG_SQ_HAVE_DESERTER);
            }

            // check if banned
            auto& bannedPlayers = sWorld.m_arena_3v3_banned;
            const auto account = session->GetAccountId();

            auto it = bannedPlayers.find(account);
            if (it != bannedPlayers.end() && it->second > time(NULL))
                return session->PGetHellgroundString(16608, session->secondsToTimeString(it->second - time(NULL)).c_str());

            if (GetLevel() < 70)
                return session->GetHellgroundString(LANG_SQ_NEED_MAX_LVL);

            // check bg queue
            if (!at_teleport && InBattleGroundOrArenaQueue())
            {
                return session->GetHellgroundString(LANG_ALREADY_IN_QUEUE);
            }

            // can't queue as group
            if (!at_teleport && GetGroup())
            {
                return session->GetHellgroundString(LANG_SQ_AS_GROUP);
            }

            if (getBattleGroundMgr()->GetDebugArenaId())
                return nullptr;

            // need to have at least 1 hour to join
            if (GetTotalPlayedTime() < HOUR)
            {
                return session->GetHellgroundString(LANG_SQ_FEW_PLAYED_TIME);
            }

            // check for talents
            if (GetFreeTalentPoints() > 0)
            {
                return session->GetHellgroundString(LANG_SQ_HAS_FREE_TALENT_POINTS);
            }

            if (!at_teleport)
            {
                // check items
                uint32 itemlevelsum = 0;
                uint8 defenciveitemscount = 0;
                bool haspvptrinket = false;
                
                for (int i = EQUIPMENT_SLOT_HEAD; i <= EQUIPMENT_SLOT_RANGED; i++)
                {
                    Item* itemTarget = GetItemByPos(INVENTORY_SLOT_BAG_0, i);

                    // check if all slots are equipped
                    // offhand slot can be empty, body = robe (skip)
                    if (!itemTarget)
                    {
                        if (i != EQUIPMENT_SLOT_OFFHAND && i != EQUIPMENT_SLOT_BODY)
                        {
                            return session->GetHellgroundString(15135);
                        }
                        else
                            continue;
                    }

                    // check RARE and less items except trinkets
                    if (itemTarget->GetProto()->Quality < ITEM_QUALITY_EPIC &&
                        (i != EQUIPMENT_SLOT_TRINKET1 && i != EQUIPMENT_SLOT_TRINKET2 && i != EQUIPMENT_SLOT_RANGED && i != EQUIPMENT_SLOT_TABARD && i != EQUIPMENT_SLOT_BODY))
                    {
                        // some blue items
                        switch (itemTarget->GetProto()->ItemId)
                        {
                            case 23825:
                            case 10588:
                            case 16009:
                            case 13953:
                            case 32508:
                                break;
                            default:
                                return session->GetHellgroundString(15501);
                        }
                    }

                    //pvp trinket check
                    if (!haspvptrinket && (i == EQUIPMENT_SLOT_TRINKET1 || i == EQUIPMENT_SLOT_TRINKET2))
                    {
                        for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++) {
                            if (itemTarget->GetProto()->Spells[j].SpellId == 42292)
                            {
                                haspvptrinket = true;
                                break;
                            }
                        }
                    }

                    // no need to check it for tank items and itemlevel
                    if (i == EQUIPMENT_SLOT_TRINKET1 || i == EQUIPMENT_SLOT_TRINKET2 || i == EQUIPMENT_SLOT_RANGED)
                        continue;

                    // tank items check
                    for (int j = 0; j < MAX_ITEM_PROTO_STATS; j++)
                    {
                        if (itemTarget->GetProto()->ItemStat[j].ItemStatValue != 0) {
                            switch (itemTarget->GetProto()->ItemStat[j].ItemStatType)
                            {
                            case ITEM_MOD_DEFENSE_SKILL_RATING:
                            case ITEM_MOD_DODGE_RATING:
                            case ITEM_MOD_PARRY_RATING:
                            case ITEM_MOD_BLOCK_RATING:
                                ++defenciveitemscount;
                            }
                        }
                    }

                    itemlevelsum += itemTarget->GetProto()->ItemLevel;
                }

                if (!haspvptrinket)
                {
                    return session->GetHellgroundString(LANG_SQ_NEED_PVP_TRINKET);
                }

                if (defenciveitemscount > 3 ||
                    HasSpell(27168) || //paladin proto key tal
                    HasSpell(12809)) //warr tank key tal
                {
                    return session->GetHellgroundString(LANG_SQ_TANK_ITEMS);
                }

                uint32 current_gs = GetGearScore();
                uint32 needed_gs = GS_BADGE;
                if (current_gs < needed_gs)
                {
                    return session->PGetHellgroundString(15136, current_gs, needed_gs);
                }
            }
        }

    }
    else
    {
        if (sBattleGroundMgr.isTesting())
            return nullptr;
        
        // BG DESERTER
        if (GetDummyAura(SPELL_BG_DESERTER))
        {
            return session->GetHellgroundString(15489);
        }

        if (IsDoubleLogin())
        {
            return session->GetHellgroundString(16683);
        }

        if (!sWorld.isEasyRealm())
            return nullptr;

        // check items
        if (!at_teleport)
        {
            for (int i = EQUIPMENT_SLOT_HEAD; i <= EQUIPMENT_SLOT_RANGED; i++)
            {
                Item* itemTarget = GetItemByPos(INVENTORY_SLOT_BAG_0, i);

                // check if all slots are equipped
                // offhand slot can be empty, body = robe (skip)
                if (!itemTarget)
                {
                    if (i != EQUIPMENT_SLOT_OFFHAND && i != EQUIPMENT_SLOT_BODY)
                        return session->GetHellgroundString(15135);
                    else
                        continue;
                }

                // no need to check it for itemlevel
                if (i == EQUIPMENT_SLOT_TRINKET1 || i == EQUIPMENT_SLOT_TRINKET2 || i == EQUIPMENT_SLOT_RANGED)
                    continue;
            }

            uint32 current_gs = GetGearScore();
            uint32 needed_gs = GS_EPIC_START;
            if (current_gs < needed_gs)
                return session->PGetHellgroundString(15136, current_gs, needed_gs);
        }
    }
 
    return nullptr;
}

bool Player::CanReportAfkDueToLimit()
{
    // a player can complain about 3 people instantly, a complaint restores charge 1 per min.
    // cooldown starts when you make a first complaint

    // no, 1 charge is better!
    if (m_bgAfkReportedCount >= 1)
        return false;

    ++m_bgAfkReportedCount;
    m_bgAfkReportedTimer = time(NULL) + MINUTE;
    return true;
}

///This player has been blamed to be inactive in a battleground
void Player::ReportedAfkBy(Player* reporter)
{
    // this - is reported player

    BattleGround *bg = GetBattleGround();
    if (!bg || bg->GetStatus() != STATUS_IN_PROGRESS || bg->isArena())
        return;
    
    if (bg != reporter->GetBattleGround() || GetBGTeam() != reporter->GetBGTeam())
    {
        ChatHandler(reporter).SendSysMessage(16731);
        return;
    }

    // don't allow to report same player for multiple times
    if (m_bgAfkReportedBy.find(reporter->GetGUIDLow()) != m_bgAfkReportedBy.end() || m_bgAfkReports.find(reporter->GetGUIDLow()) != m_bgAfkReports.end())
    {
        ChatHandler(reporter).SendSysMessage(15485);
        return;
    }

    time_t now = time(NULL);
    // does reporter have report-charges?
    if (!reporter->CanReportAfkDueToLimit())
    {
        ChatHandler(reporter).PSendSysMessage(15487, GetSession()->secondsToTimeString(reporter->m_bgAfkReportedTimer - now).c_str());
        return;
    }

    //uint32 reports_to_kick = BG_REPORTS_TO_KICK;

    //if (m_bgAfkReports.size() >= reports_to_kick)
    //{
    //    bg->SendMessageToTeam(GetTeam(), 15508, GetName());
    //    LeaveBattleground();
    //    return;
    //}

    // check if player has 'Idle' or 'Inactive' debuff
    //if (HasAura(43680, 0) || HasAura(SPELL_AURA_PLAYER_INACTIVE, 0))
    //{
    //    ChatHandler(reporter).SendSysMessage(15486);
    //    return;
    //}

    m_bgAfkReports.insert(reporter->GetGUIDLow());
    m_bgAfkReportedBy.insert(reporter->GetGUIDLow());

    bg->SendMessageToTeam(GetBGTeam(), 15179, reporter->GetName(), GetName(), m_bgAfkReportedBy.size(), BG_REPORTS_TO_DEBUFF);

    ChatHandler(reporter).SendSysMessage(15546);

    sLog.outLog(LOG_BG, "ID %u: Player %s was reported by %s (total %u)", bg->GetBgInstanceId(), GetName(), reporter->GetName(), m_bgAfkReports.size());

    // X players have to complain to apply debuff
    if (m_bgAfkReportedBy.size() >= BG_REPORTS_TO_DEBUFF)
    {
        // cast 'Idle' spell
        if (!HasAura(SPELL_AURA_BG_PLAYER_IDLE))
        {
            SpellCastResult result = CastSpell(this, SPELL_AURA_BG_PLAYER_IDLE, true);
            if (result != SPELL_CAST_OK)
                sLog.outLog(LOG_CRITICAL, "ID %u: Player %s error for SPELL_AURA_BG_PLAYER_IDLE, result: %x", bg->GetBgInstanceId(), GetName(), result);

            if (Aura* Aur = GetAura(SPELL_AURA_BG_PLAYER_IDLE, 0))
            {
                uint32 duration = bg->GetMapId() == MAP_ALTERAC ? BG_IDLE_AURA_DURATION_ALTERAC : BG_IDLE_AURA_DURATION;
                Aur->SetAuraDuration(duration);
                Aur->SetPeriodicTimer(duration);
                Aur->SetPeriodic(true);
                Aur->UpdateAuraDuration();

                std::string msg = ChatHandler(this).PGetSysMessage(16732, duration / 1000);

                ChatHandler(this).SendSysMessage(msg.c_str());
                SendCenterMessage(msg.c_str());
                
                bg->SendMessageToTeam(GetBGTeam(), 16759, GetName(), duration / 1000);
            }
        }

        m_bgAfkReportedBy.clear();
    }
}

// @!research removing const causing invis bugs like with outland blinkers
// rogues can re-appear from stealth
bool Player::canSeeOrDetect(Unit const* u, WorldObject const* viewPoint, bool detect, bool inVisibleList, bool is3dDistance) const
{
    // Always can see self
    if (u == this)
        return true;

    if (u->GetObjectGuid().IsAnyTypeCreature() && u->ToCreature()->IsAIEnabled && !u->ToCreature()->AI()->IsVisible() && !isGameMaster())
        return false;

    if (InArena())
    {
        if (const Player* target = u->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            if (GetBattleGround()->GetStatus() == STATUS_WAIT_JOIN)
            {
                bool see = GetBGTeam() == target->GetBGTeam() && target->isGMVisible() && !IsSpectator() && !target->IsSpectator();
                if (!see)
                    return false;
            }
            // if target is spectator and you're not spectator -> you cant see him
            // if target is spectator and you're a spectator, but not same raid group -> you cant see him
            else if ((!IsSpectator() || !IsInSameRaidWith(target)) && target->IsSpectator())
                return false;
        }
    }    

    // player visible for other player if not logout and at same transport
    // including case when player is out of world
    bool at_same_transport =
        GetTransport() && u->GetTypeId() == TYPEID_PLAYER
        && !GetSession()->PlayerLogout() && !((Player*)u)->GetSession()->PlayerLogout()
        && !GetSession()->PlayerLoading() && !((Player*)u)->GetSession()->PlayerLoading()
        && GetTransport() == ((Player*)u)->GetTransport();

    // not in world
    if (!at_same_transport && (!IsInWorld() || !u->IsInWorld()))
        return false;

    // forbidden to seen (at GM respawn command)
    if (u->GetVisibility() == VISIBILITY_RESPAWN)
        return false;

    // always seen by owner
    if (GetGUID() == u->GetCharmerOrOwnerGUID())
        return true;

	// pets always seen by patry members
	if (u->GetTypeId() == TYPEID_UNIT)
	{
		Player* owner = (Player*)u->ToCreature()->GetCharmerOrOwner();
		if (owner && IsInSameRaidWith(owner))
			return true;
	}

    Map& _map = *u->GetMap();
    // Grid dead/alive checks
    // non visible at grid for any stealth state
    if (!u->IsVisibleInGridForPlayer(this))
        return false;

    if (duel)
    {
        if (u->IsFriendlyTo(this)) // means if WE CANNOT attack HIM (if he's GREEN/BLUE)
        {
            if (Player *plr = u->GetCharmerOrOwnerPlayerOrPlayerItself())
            {
                if (!plr->duel || duel->opponent != plr)
                    return false;
            }
        }
    }

    // different visible distance checks
    if (IsTaxiFlying())                                     // what see player in flight
     {
        if (!viewPoint->IsWithinDistInMap(u, _map.GetVisibilityDistance(const_cast<Unit*>(u), const_cast<Player*>(this)) + (inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f), is3dDistance))
             return false;
     }
     else if (!u->isAlive())                                     // distance for show body
     {
        if (!viewPoint->IsWithinDistInMap(u, _map.GetVisibilityDistance(const_cast<Unit*>(u), const_cast<Player*>(this)) + (inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f), is3dDistance))
             return false;
     }
     else if (u->GetTypeId()==TYPEID_PLAYER)                     // distance for show player
     {
         // Players far than max visible distance for player or not in our map are not visible too
        if (!at_same_transport && !viewPoint->IsWithinDistInMap(u, _map.GetVisibilityDistance(const_cast<Unit*>(u), const_cast<Player*>(this)) + (inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f), is3dDistance))
             return false;
     }
     else if (u->GetCharmerOrOwnerGUID())                        // distance for show pet/charmed
     {
         // Pet/charmed far than max visible distance for player or not in our map are not visible too
        if (!viewPoint->IsWithinDistInMap(u, _map.GetVisibilityDistance(const_cast<Unit*>(u), const_cast<Player*>(this)) + (inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f), is3dDistance))
             return false;
     }
     else                                                    // distance for show creature
     {
         // Units far than max visible distance for creature or not in our map are not visible too
         if (!viewPoint->IsWithinDistInMap(u, u->isActiveObject() ? (MAX_VISIBILITY_DISTANCE - (inVisibleList ? 0.0f : World::GetVisibleObjectGreyDistance()))
            : (_map.GetVisibilityDistance(const_cast<Unit*>(u), const_cast<Player*>(this)) + (inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f))
             , is3dDistance))
             return false;
     }

    if (u->GetVisibility() == VISIBILITY_OFF)
    {
        // GMs see any players, not higher GMs and all units
        if (isGameMaster())
        {
            if (u->GetTypeId() == TYPEID_PLAYER)
            {
                WorldSession* s = ((Player *)u)->GetSession();

                return s && !s->isFakeBot() && s->GetPermissions() <= GetSession()->GetPermissions();
            }
            else
                return true;
        }
        return false;
    }

    // GM's can see everyone with invisibilitymask with less or equal security level
    if (m_invisibilityMask || u->m_invisibilityMask)
    {
        if (isGameMaster())
        {
            if (u->GetTypeId() == TYPEID_PLAYER)
                return ((Player*)u)->GetSession()->GetPermissions() <= GetSession()->GetPermissions();
            else
                return true;
        }

        // why do we need this?
        if (u->GetTypeId() == TYPEID_PLAYER && u->canDetectInvisibilityOf(this))
            return true;

        // player see other player with stealth/invisibility only if he in same group or raid or same team (raid/team case dependent from conf setting)
        if (!canDetectInvisibilityOf(u))
            if (!(u->GetTypeId()==TYPEID_PLAYER && !IsHostileTo(u) && IsGroupVisiblefor (((Player*)u))))
                return false;
    }

    // GM invisibility checks early, invisibility if any detectable, so if not stealth then visible
    if (u->GetVisibility() == VISIBILITY_GROUP_STEALTH && !isGameMaster())
    {
        // if player is dead then he can't detect anyone in any cases
        //do not know what is the use of this detect
        // stealth and detected and visible for some seconds
        if (!isAlive() || IsSpectator())
            detect = false;
        if (m_DetectInvTimer < 300 || !HaveAtClient(u))
            if (!(u->GetTypeId()==TYPEID_PLAYER && !IsHostileTo(u) && IsGroupVisiblefor (((Player*)u))))
                if (!detect || !canDetectStealthOf(u, viewPoint, viewPoint->GetExactDist(u)))
                    return false;
    }

    // If use this server will be too laggy
    // Now check is target visible with LoS
    //return u->IsWithinLOS(GetPositionX(),GetPositionY(),GetPositionZ());
    return true;
}

bool Player::IsVisibleInGridForPlayer(Player const * pl) const
{
    // gamemaster in GM mode see all, including ghosts
    if (pl->isGameMaster() && GetSession()->GetPermissions() <= pl->GetSession()->GetPermissions())
        return true;

    // It seems in battleground everyone sees everyone, except the enemy-faction ghosts
    if (InBattleGroundOrArena())
    {
        if (!(isAlive() || m_deathTimer > 0) && !IsFriendlyTo(pl))
            return false;
        return true;
    }

    // Live player see live player or dead player with not realized corpse
    if (pl->isAlive() || pl->m_deathTimer > 0)
    {
        return isAlive() || m_deathTimer > 0;
    }

    // Ghost see other friendly ghosts, that's for sure
    if (!(isAlive() || m_deathTimer > 0) && IsFriendlyTo(pl))
        return true;

    // Dead player see live players near own corpse
    if (isAlive())
    {
        Corpse *corpse = pl->GetCorpse();
        if (corpse)
        {
            // 20 - aggro distance for same level, 25 - max additional distance if player level less that creature level
            if (corpse->IsWithinDistInMap(this,(20+25)*sWorld.getConfig(RATE_CREATURE_AGGRO)))
                return true;
        }
    }

    // and not see any other
    return false;
}

bool Player::IsVisibleGloballyfor (Player* u) const
{
    if (!u)
        return false;

    // Always can see self
    if (u==this)
        return true;

    // Visible units, always are visible for all players
    if (GetVisibility() == VISIBILITY_ON)
        return true;

    // GMs are visible for higher gms (or players are visible for gms)
    if (u->GetSession()->HasPermissions(PERM_GMT_HDEV))
        return GetSession()->GetPermissions() <= u->GetSession()->GetPermissions();

    // non faction visibility non-breakable for non-GMs
    if (GetVisibility() == VISIBILITY_OFF)
        return false;

    // non-gm stealth/invisibility not hide from global player lists
    return true;
}

template<class T>
inline void UpdateVisibilityOf_helper(std::set<uint64>& s64, T* target, std::set<WorldObject*>& v)
{
    s64.insert(target->GetGUID());
}

template<>
inline void UpdateVisibilityOf_helper(std::set<uint64>& s64, GameObject* target, std::set<WorldObject*>& v)
{
    if(!target->IsTransport())
        s64.insert(target->GetGUID());
}

template<>
inline void UpdateVisibilityOf_helper(std::set<uint64>& s64, Creature* target, std::set<WorldObject*>& v)
{
    s64.insert(target->GetGUID());
    v.insert(target);
}

template<>
inline void UpdateVisibilityOf_helper(std::set<uint64>& s64, Player* target, std::set<WorldObject*>& v)
{
    s64.insert(target->GetGUID());
    v.insert(target);
}

template<class T>
inline void BeforeVisibilityDestroy(T* /*t*/, Player* /*p*/)
{
}

template<>
inline void BeforeVisibilityDestroy<Creature>(Creature* t, Player* p)
{
    if (p->GetPetGUID() == t->GetGUID() && t->GetObjectGuid().IsPet())
        p->RemovePet(t->ToPet(), PET_SAVE_NOT_IN_SLOT, true);
}

void Player::UpdateVisibilityOf(WorldObject const* viewPoint, WorldObject* target)
{
    if (HaveAtClient(target))
    {
        if (!target->isVisibleForInState(this, viewPoint, true))
        {
            if (Creature* c = target->ToCreature())
                BeforeVisibilityDestroy<Creature>(c, this);

            UpdateData udata;
            WorldPacket packet;
            target->BuildValuesUpdateBlockForPlayer(&udata,this);
            udata.BuildPacket(&packet);
            SendPacketToSelf(&packet);

            target->DestroyForPlayer(this);
            m_clientGUIDs.erase(target->GetGUID());
        }
    }
    else
    {
        if (target->isVisibleForInState(this, viewPoint, false))
        {
            target->SendCreateUpdateToPlayer(this);
            if (!target->GetObjectGuid().IsTransport())
                m_clientGUIDs.insert(target->GetGUID());

            // target aura duration for caster show only if target exist at caster client
            // send data at target visibility change (adding to client)
            if (target->isType(TYPEMASK_UNIT))
                SendInitialVisiblePackets((Unit*)target);
        }
    }
}

void Player::RewardPlayerAndGroupAtEvent(uint32 creature_id, WorldObject* pRewardSource)
{
    uint64 creature_guid = pRewardSource->GetTypeId()==TYPEID_UNIT ? pRewardSource->GetGUID() : uint64(0);

    // prepare data for near group iteration
    if (Group *pGroup = GetGroup())
    {
        for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* pGroupGuy = itr->getSource();
            if (!pGroupGuy)
                continue;

            if (!pGroupGuy->IsAtGroupRewardDistance(pRewardSource))
                continue;                               // member (alive or dead) or his corpse at req. distance

            // quest objectives updated only for alive group member or dead but with not released body
            if (pGroupGuy->isAlive()|| !pGroupGuy->GetCorpse())
                pGroupGuy->KilledMonster(creature_id, creature_guid);
        }
    }
    else                                                    // if (!pGroup)
        KilledMonster(creature_id, creature_guid);
}

void Player::SendInitialVisiblePackets(Unit* target)
{
    SendAuraDurationsForTarget(target);
    if (target->isAlive())
    {
        if (target->HasUnitState(UNIT_STAT_MELEE_ATTACKING) && target->GetVictim())
            target->SendMeleeAttackStart(target->getVictimGUID());
    }
}

template<class T>
void Player::UpdateVisibilityOf(WorldObject const* viewPoint, T* target, UpdateData& data, std::set<WorldObject*>& visibleNow)
{
    if (!target)
        return;

    if (HaveAtClient(target))
    {
        if (!target->isVisibleForInState(this, viewPoint, true))
        {
            target->BuildOutOfRangeUpdateBlock(&data);
            m_clientGUIDs.erase(target->GetGUID());
        }
    }
    else
    {
        if (target->isVisibleForInState(this, viewPoint, false))
        {
            target->BuildCreateUpdateBlockForPlayer(&data, this);
            UpdateVisibilityOf_helper(m_clientGUIDs, target, visibleNow);
        }
    }
}

template void Player::UpdateVisibilityOf(WorldObject const*, Player*       , UpdateData&, std::set<WorldObject*>&);
template void Player::UpdateVisibilityOf(WorldObject const*, Creature*     , UpdateData&, std::set<WorldObject*>&);
template void Player::UpdateVisibilityOf(WorldObject const*, Corpse*       , UpdateData&, std::set<WorldObject*>&);
template void Player::UpdateVisibilityOf(WorldObject const*, GameObject*   , UpdateData&, std::set<WorldObject*>&);
template void Player::UpdateVisibilityOf(WorldObject const*, DynamicObject*, UpdateData&, std::set<WorldObject*>&);

void Player::InitPrimaryProffesions()
{
    SetFreePrimaryProfessions(sWorld.getConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL));
}

void Player::SendComboPoints()
{
    Unit *combotarget = GetMap()->GetUnit(m_comboTarget);
    if (combotarget)
    {
        WorldPacket data(SMSG_UPDATE_COMBO_POINTS, combotarget->GetPackGUID().size()+1);
        data << combotarget->GetPackGUID();
        data << uint8(m_comboPoints);
        SendPacketToSelf(&data);
    }
}

void Player::AddComboPoints(Unit* target, int8 count)
{
    if (!count)
        return;

    // without combo points lost (duration checked in aura)
    RemoveSpellsCausingAura(SPELL_AURA_RETAIN_COMBO_POINTS);

    if (target->GetGUID() == m_comboTarget)
    {
        m_comboPoints += count;
    }
    else
    {
        if (m_comboTarget)
            if (Unit* target2 = GetMap()->GetUnit(m_comboTarget))
                target2->RemoveComboPointHolder(GetGUIDLow());

        m_comboTarget = target->GetGUID();
        m_comboPoints = count;

        target->AddComboPointHolder(GetGUIDLow());
    }

    if (m_comboPoints > 5) m_comboPoints = 5;
    if (m_comboPoints < 0) m_comboPoints = 0;

    SendComboPoints();
}

void Player::ClearComboPoints()
{
    if (!m_comboTarget)
        return;

    // without combopoints lost (duration checked in aura)
    RemoveSpellsCausingAura(SPELL_AURA_RETAIN_COMBO_POINTS);

    m_comboPoints = 0;

    Unit* target = GetMap()->GetUnit(m_comboTarget);
    SendComboPoints();

    if (target)
        target->RemoveComboPointHolder(GetGUIDLow());

    m_comboTarget = 0;
}

void Player::SetGroup(Group *group, int8 subgroup)
{
    if (group == NULL)
        m_group.unlink();
    else
    {
        // never use SetGroup without a subgroup unless you specify NULL for group
        ASSERT(subgroup >= 0);
        m_group.link(group, this);
        m_group.setSubGroup((uint8)subgroup);
    }
}

void Player::SendInitialPacketsBeforeAddToMap()
{
    WorldPacket data(SMSG_SET_REST_START, 4);
    data << uint32(0);                                      // unknown, may be rest state time or experience
    SendPacketToSelf(&data);

    // Homebind
    data.Initialize(SMSG_BINDPOINTUPDATE, 5*4);
    data << m_homebindX << m_homebindY << m_homebindZ;
    data << (uint32) m_homebindMapId;
    data << (uint32) m_homebindZoneId;
    SendPacketToSelf(&data);

    // SMSG_SET_PROFICIENCY
    // SMSG_UPDATE_AURA_DURATION

    // tutorial stuff
    data.Initialize(SMSG_TUTORIAL_FLAGS, 8*4);
    for (int i = 0; i < 8; ++i)
        data << uint32(GetTutorialInt(i));
    SendPacketToSelf(&data);

    SendInitialSpells();

    data.Initialize(SMSG_SEND_UNLEARN_SPELLS, 4);
    data << uint32(0);                                      // count, for (count) uint32;
    SendPacketToSelf(&data);

    SendInitialActionButtons();

    m_reputationMgr.SendInitialReputations();

    UpdateZone(GetZoneId());
    SendInitWorldStates();

    // SMSG_SET_AURA_SINGLE

    data.Initialize(SMSG_LOGIN_SETTIMESPEED, 8);
    data << uint32(secsToTimeBitFields(sWorld.GetGameTime()));
    data << (float)0.01666667f;                             // game speed
    SendPacketToSelf(&data);

    // set fly flag if in fly form or taxi flight to prevent visually drop at ground in showup moment
    if (HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED) || IsTaxiFlying())
        AddUnitMovementFlag(MOVEFLAG_FLYING);
}

class VisibilityAndViewUpdateEvent : public BasicEvent
{
    public:
        VisibilityAndViewUpdateEvent(Player& owner) : BasicEvent(), _owner(owner) {}

    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
    {
        _owner.UpdateVisibilityAndView();
        return true;
    }

    private:
        Player& _owner;
};

void Player::SendInitialPacketsAfterAddToMap()
{
    // Delay visibility update by 5s after porting to new map

    UpdateVisibilityAndView();
    AddEvent(new VisibilityAndViewUpdateEvent(*this), 3000);
        
    ResetTimeSync();
    SendTimeSync();

    CastSpell(this, 836, true);                             // LOGINEFFECT

    // set some aura effects that send packet to player client after add player to map
    // SendMessageToSet not send it to player not it map, only for aura that not changed anything at re-apply
    // same auras state lost at far teleport, send it one more time in this case also
    static const AuraType auratypes[] =
    {
        SPELL_AURA_MOD_FEAR,     SPELL_AURA_TRANSFORM,                 SPELL_AURA_WATER_WALK,
        SPELL_AURA_FEATHER_FALL, SPELL_AURA_HOVER,                     SPELL_AURA_SAFE_FALL,
        SPELL_AURA_FLY,          SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED, SPELL_AURA_NONE
    };

    for (AuraType const* itr = &auratypes[0]; itr && itr[0] != SPELL_AURA_NONE; ++itr)
    {
        Unit::AuraList const& auraList = GetAurasByType(*itr);
        if (!auraList.empty())
            auraList.front()->ApplyModifier(true,true);
    }

    if (HasAuraType(SPELL_AURA_MOD_STUN))
        SetMovement(MOVE_ROOT);

    // Gensen: namreeb said: i would get rid of this thoug
    // manual send package (have code in ApplyModifier(true,true); that don't must be re-applied.
    //if (HasAuraType(SPELL_AURA_MOD_ROOT))
    //{
    //    WorldPacket data(SMSG_FORCE_MOVE_ROOT, 10);
    //    data << GetPackGUID();
    //    data << (uint32)2;
    //    BroadcastPacket(&data,true);
    //}

    SendEnchantmentDurations();                             // must be after add to map
    SendItemDurations();                                    // must be after add to map
    SendQuestGiverStatusMultiple();

    // summon golem guardian at relog, revive, entering instance
    if (sWorld.isEasyRealm() && IsLowHeroicDungeonOrNonactualRaid(GetMapId(), false))
        CastSpell(this, 55200, true);
}

void Player::SendUpdateToOutOfRangeGroupMembers()
{
    if (m_groupUpdateMask == GROUP_UPDATE_FLAG_NONE)
        return;

    if (Group* group = GetGroup())
        group->UpdatePlayerOutOfRange(this);

    m_groupUpdateMask = GROUP_UPDATE_FLAG_NONE;
    m_auraUpdateMask = 0;
    if (Pet *pet = GetPet())
        pet->ResetAuraUpdateMask();
}

void Player::SendTransferAborted(uint32 mapid, uint16 reason)
{
    WorldPacket data(SMSG_TRANSFER_ABORTED, 4+2);
    data << uint32(mapid);
    data << uint16(reason);                                 // transfer abort reason
    SendPacketToSelf(&data);
}

void Player::SendInstanceResetWarning(uint32 mapid, uint32 time)
{
    // type of warning, based on the time remaining until reset
    uint32 type;
    if (time > 3600)
        type = RAID_INSTANCE_WELCOME;
    else if (time > 900 && time <= 3600)
        type = RAID_INSTANCE_WARNING_HOURS;
    else if (time > 300 && time <= 900)
        type = RAID_INSTANCE_WARNING_MIN;
    else
        type = RAID_INSTANCE_WARNING_MIN_SOON;
    WorldPacket data(SMSG_RAID_INSTANCE_MESSAGE, 4+4+4);
    data << uint32(type);
    data << uint32(mapid);
    data << uint32(time);
    SendPacketToSelf(&data);
}

void Player::ApplyEquipCooldown(Item * pItem)
{
    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        _Spell const& spellData = pItem->GetProto()->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type (note: ITEM_SPELLTRIGGER_ON_NO_DELAY_USE not have cooldown)
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
            continue;

        if (HasSpellItemCooldown(spellData.SpellId, spellData.SpellCategory)) // do not overlay old cooldown
            continue;

        AddSpellItemCooldown(spellData.SpellId, pItem->GetEntry(), spellData.SpellCategory, time(NULL) + 30);

        WorldPacket data(SMSG_ITEM_COOLDOWN, 12);
        data << pItem->GetGUID();
        data << uint32(spellData.SpellId);
        SendPacketToSelf(&data);
    }
}

void Player::resetSpells()
{
    // not need after this call
    if (HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
    {
        m_atLoginFlags = m_atLoginFlags & ~AT_LOGIN_RESET_SPELLS;
        RealmDataDatabase.PExecute("UPDATE characters SET at_login = at_login & ~ %u WHERE guid ='%u'", uint32(AT_LOGIN_RESET_SPELLS), GetGUIDLow());
    }

    // make full copy of map (spells removed and marked as deleted at another spell remove
    // and we can't use original map for safe iterative with visit each spell at loop end
    PlayerSpellMap smap = GetSpellMap();

    for (PlayerSpellMap::const_iterator iter = smap.begin();iter != smap.end(); ++iter)
        removeSpell(iter->first);                           // only iter->first can be accessed, object by iter->second can be deleted already

    learnDefaultSpells();
    learnQuestRewardedSpells();
}

void Player::learnDefaultSpells(bool loading)
{
    // learn default race/class spells
    PlayerInfo const *info = sObjectMgr.GetPlayerInfo(GetRace(),GetClass());
    std::list<CreateSpellPair>::const_iterator spell_itr;
    for (spell_itr = info->spell.begin(); spell_itr!=info->spell.end(); ++spell_itr)
    {
        uint16 tspell = spell_itr->first;
        if (tspell)
        {
            sLog.outDebug("PLAYER: Adding initial spell, id = %u",tspell);
            if (loading || !spell_itr->second)               // not care about passive spells or loading case
                addSpell(tspell,spell_itr->second);
            else                                            // but send in normal spell in game learn case
                learnSpell(tspell);
        }
    }
}

void Player::learnQuestRewardedSpells(Quest const* quest)
{
    uint32 spell_id = quest->GetRewSpellCast();

    // skip quests without rewarded spell
    if (!spell_id)
        return;

    SpellEntry const *spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
    if (!spellInfo)
        return;

    // check learned spells state
    bool found = false;
    for (int i=0; i < 3; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_LEARN_SPELL && !HasSpell(spellInfo->EffectTriggerSpell[i]))
        {
            found = true;
            break;
        }
    }

    // skip quests with not teaching spell or already known spell
    if (!found)
        return;

    // prevent learn non first rank unknown profession and second specialization for same profession)
    uint32 learned_0 = spellInfo->EffectTriggerSpell[0];
    if (sSpellMgr.GetSpellRank(learned_0) > 1 && !HasSpell(learned_0))
    {
        // not have first rank learned (unlearned prof?)
        uint32 first_spell = sSpellMgr.GetFirstSpellInChain(learned_0);
        if (!HasSpell(first_spell))
            return;

        SpellEntry const *learnedInfo = sSpellTemplate.LookupEntry<SpellEntry>(learned_0);
        if (!learnedInfo)
            return;

        // specialization
        if (learnedInfo->Effect[0]==SPELL_EFFECT_TRADE_SKILL && learnedInfo->Effect[1]==0)
        {
            // search other specialization for same prof
            for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
            {
                if (itr->second.state == PLAYERSPELL_REMOVED || itr->first==learned_0)
                    continue;

                SpellEntry const *itrInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                if (!itrInfo)
                    return;

                // compare only specializations
                if (itrInfo->Effect[0]!=SPELL_EFFECT_TRADE_SKILL || itrInfo->Effect[1]!=0)
                    continue;

                // compare same chain spells
                if (sSpellMgr.GetFirstSpellInChain(itr->first) != first_spell)
                    continue;

                // now we have 2 specialization, learn possible only if found is lesser specialization rank
                if (!sSpellMgr.IsHighRankOfSpell(learned_0,itr->first))
                    return;
            }
        }
    }

    CastSpell(this, spell_id, true);
}

void Player::learnQuestRewardedSpells()
{
    // learn spells received from quest completing
    for (QuestStatusMap::const_iterator itr = mQuestStatus.begin(); itr != mQuestStatus.end(); ++itr)
    {
        // skip no rewarded quests
        if (!itr->second.m_rewarded)
            continue;

        Quest const* quest = sObjectMgr.GetQuestTemplate(itr->first);
        if (!quest)
            continue;

        learnQuestRewardedSpells(quest);
    }
}

void Player::learnSkillRewardedSpells(uint32 skill_id)
{
    uint32 raceMask  = GetRaceMask();
    uint32 classMask = GetClassMask();
    for (uint32 j=0; j<sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const *pAbility = sSkillLineAbilityStore.LookupEntry(j);
        if (!pAbility || pAbility->skillId!=skill_id || pAbility->learnOnGetSkill != ABILITY_LEARNED_ON_GET_PROFESSION_SKILL)
            continue;
        // Check race if set
        if (pAbility->racemask && !(pAbility->racemask & raceMask))
            continue;
        // Check class if set
        if (pAbility->classmask && !(pAbility->classmask & classMask))
            continue;

        if (sSpellTemplate.LookupEntry<SpellEntry>(pAbility->spellId))
        {
            // Ok need learn spell
            learnSpell(pAbility->spellId);
        }
    }
}

void Player::learnSkillRewardedSpells()
{
    for (uint16 i=0; i < PLAYER_MAX_SKILLS; i++)
    {
        if (!GetUInt32Value(PLAYER_SKILL_INDEX(i)))
            continue;

        uint32 pskill = GetUInt32Value(PLAYER_SKILL_INDEX(i)) & 0x0000FFFF;

        learnSkillRewardedSpells(pskill);
    }
}

void Player::SendAuraDurationsForTarget(Unit* target)
{
    for (Unit::AuraMap::const_iterator itr = target->GetAuras().begin(); itr != target->GetAuras().end(); ++itr)
    {
        Aura* aura = itr->second;
        if (aura->GetAuraSlot() >= MAX_AURAS || aura->IsPassive() || aura->GetCasterGUID()!=GetGUID())
            continue;

        aura->SendAuraDurationForCaster(this);
    }
}

void Player::SetDailyQuestStatus(uint32 quest_id)
{
    for (uint32 quest_daily_idx = 0; quest_daily_idx < sWorld.getConfig(CONFIG_DAILY_MAX_PER_DAY); ++quest_daily_idx)
    {
        if (!GetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx))
        {
            SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,quest_id);
            m_lastDailyQuestTime = time(NULL);              // last daily quest time
            m_DailyQuestChanged = true;
            break;
        }
    }
}

void Player::ResetDailyQuestStatus()
{
    for (uint32 quest_daily_idx = 0; quest_daily_idx < sWorld.getConfig(CONFIG_DAILY_MAX_PER_DAY); ++quest_daily_idx)
        SetUInt32Value(PLAYER_FIELD_DAILY_QUESTS_1+quest_daily_idx,0);

    // DB data deleted in caller
    m_DailyQuestChanged = false;
    m_lastDailyQuestTime = 0;
    m_DailyArenasWon = 0;
}

BattleGround* Player::GetBattleGround() const
{
    if (GetBattleGroundId() == 0)
        return NULL;

    return sBattleGroundMgr.GetBattleGround(GetBattleGroundId(), m_bgTypeID);
}

bool Player::GetBGAccessByLevel(BattleGroundTypeId bgTypeId) const
{
    // get a template bg instead of running one
    BattleGround *bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return false;

    if (sWorld.getConfig(CONFIG_19_LVL_ADAPTATIONS) && (GetLevel() >= TWINK_LEVEL_MIN && GetLevel() <= TWINK_LEVEL_MAX))
        return true;
    
    if (GetLevel() < bg->GetMinLevel() || GetLevel() > bg->GetMaxLevel())
        return false;

    return true;
}

uint32 Player::GetMinLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId)
{
    if (bracket_id < 1)
        return 0;

    if (bracket_id > BG_BRACKET_ID_LAST)
        bracket_id = BG_BRACKET_ID_LAST;

    BattleGround *bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    ASSERT(bg);
    return 10 * bracket_id + bg->GetMinLevel();
}

uint32 Player::GetMaxLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId)
{
    if (bracket_id >= BG_BRACKET_ID_LAST)
        return 255;                                         // hardcoded max level

    return GetMinLevelForBattleGroundBracketId(bracket_id, bgTypeId) + 10;
}

Creature* Player::GetBGCreature(uint32 type)
{
    if (BattleGround* pBG = GetBattleGround())
        return pBG->GetBGCreature(type);

    return NULL;
}

float Player::GetReputationPriceDiscount(Creature const* pCreature) const
{
    if (sWorld.getSeason() < SEASON_4)
        return 1.0f;

    FactionTemplateEntry const* vendor_faction = pCreature->getFactionTemplateEntry();
    if (!vendor_faction || !vendor_faction->faction)
        return 1.0f;

    ReputationRank rank = m_reputationMgr.GetRank(vendor_faction->faction);
    if (rank <= REP_NEUTRAL)
        return 1.0f;

    return 1.0f - 0.05f* (rank - REP_NEUTRAL);
}

void Player::ApplyTrainerPriceDiscount(TrainerType type, float &value) const
{
    if (sWorld.getConfig(CONFIG_FACTION_MINORITY) == GetTeam())
        value *= 0.9f;

    //if (!sWorld.isMaxLvlMostlyReached() && (type == TRAINER_TYPE_CLASS || type == TRAINER_TYPE_PETS) && !IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1))
    //    value *= ((float)(100 - sWorld.getConfig(CONFIG_SPELL_TRAINER_START_DISCOUNT))) / 100.0f;
}

bool Player::IsSpellFitByClassAndRace(uint32 spell_id) const
{
    uint32 racemask  = GetRaceMask();
    uint32 classmask = GetClassMask();

    SkillLineAbilityMap::const_iterator lower = sSpellMgr.GetBeginSkillLineAbilityMap(spell_id);
    SkillLineAbilityMap::const_iterator upper = sSpellMgr.GetEndSkillLineAbilityMap(spell_id);

    for (SkillLineAbilityMap::const_iterator _spell_idx = lower; _spell_idx != upper; ++_spell_idx)
    {
        // skip wrong race skills
        if (_spell_idx->second->racemask && (_spell_idx->second->racemask & racemask) == 0)
            return false;

        // skip wrong class skills
        if (_spell_idx->second->classmask && (_spell_idx->second->classmask & classmask) == 0)
            return false;
    }
    return true;
}

bool Player::HasQuestForGO(int32 GOId) const
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (questid == 0)
            continue;

        QuestStatusMap::const_iterator qs_itr = mQuestStatus.find(questid);
        if (qs_itr == mQuestStatus.end())
            continue;

        QuestStatusData const& qs = qs_itr->second;

        if (qs.m_status == QUEST_STATUS_INCOMPLETE)
        {
            Quest const* qinfo = sObjectMgr.GetQuestTemplate(questid);
            if (!qinfo)
                continue;

            if (GetGroup() && GetGroup()->isRaidGroup() && qinfo->GetType() != QUEST_TYPE_RAID)
                continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                if (qinfo->ReqCreatureOrGOId[j]>=0)         //skip non GO case
                    continue;

                if ((-1)*GOId == qinfo->ReqCreatureOrGOId[j] && qs.m_creatureOrGOcount[j] < qinfo->ReqCreatureOrGOCount[j])
                    return true;
            }
        }
    }
    return false;
}

void Player::UpdateForQuestsGO()
{
    if (m_clientGUIDs.empty())
        return;

    UpdateData udata;
    WorldPacket packet;
    for (ClientGUIDs::iterator itr=m_clientGUIDs.begin(); itr!=m_clientGUIDs.end(); ++itr)
    {
        if (IS_GAMEOBJECT_GUID(*itr))
        {
            GameObject *obj = GetMap()->GetGameObject(*itr);
            if (obj)
                obj->BuildValuesUpdateBlockForPlayer(&udata,this);
        }
    }
    udata.BuildPacket(&packet);
    SendPacketToSelf(&packet);
}

bool Player::CanBeSummonedBy(const Unit * summoner)
{
    if (!summoner)
        return false;

    // if summoning to instance
    if (summoner->GetMap()->IsDungeon())
    {
        // if both in same map-dungeon and have different instanceId's - don't summon for both dungeons/raids
        if (GetMap()->IsDungeon() && GetMapId() == summoner->GetMapId() && GetInstanciableInstanceId() != summoner->GetInstanciableInstanceId()) // for dungeons
            return false;

        // check for permbinded id's
        InstanceSave * tmpInst = GetInstanceSave(summoner->GetMapId()); // dungeons save only on heroic AND there are no heroics for raids - thus it will only get bound instances
        if (tmpInst) // if there IS save on the summoned player
        {
            if (tmpInst->GetSaveInstanceId() != summoner->GetInstanciableInstanceId()) // if summoned player inst id != summoner inst id
                return false;
        }
        // summoned player had no cooldown yet - then check summoner
        else
        {
            if (const Player* summPlr = summoner->ToPlayer())
            {
                if (const Group* grp = summPlr->GetGroup())
                {
                    // summonner and summonned are not in the same group
                    if (grp != GetGroup())
                        return false;

                    if (Player* leader = GetPlayerInWorld(grp->GetLeaderGUID()))
                    {
                        if (!leader->IsInMap(summoner)) // leader must be in instance for proper binding
                            return false;
                        InstanceSave * tmpLeaderInst = leader->GetInstanceSave(summoner->GetMapId());
                        if (tmpLeaderInst)
                        {
                            if (tmpLeaderInst->GetSaveInstanceId() != summoner->GetInstanciableInstanceId()) // if raid leader inst id != summoner inst id
                                return false;
                        }
                        else // warlock has cooldown, and raid leader not? - definetely trying to exploit!
                            return false;
                    }
                    else // leader is offline, so why should we care?
                        return false;
                }
                else // well, should have group
                    return false;
            }
            // no else, might creature/GO/item be a summoner?
        }
    }

    return true;
}

void Player::SummonIfPossible(bool agree, uint64 summonerGUID)
{
    Unit* summoner = GetUnit(summonerGUID);
    if (summoner)
    {
        if (summoner->m_currentSpells[CURRENT_CHANNELED_SPELL])
        {
            summoner->m_currentSpells[CURRENT_CHANNELED_SPELL]->SendChannelUpdate(0);
            summoner->m_currentSpells[CURRENT_CHANNELED_SPELL]->finish();
        }

        if (!CanBeSummonedBy(summoner))
            return;
    }
    else
        return;

    if (!agree)
    {
        m_summon_expire = 0;
        return;
    }

    // expire and auto declined
    if (m_summon_expire < time(NULL))
        return;

    // stop taxi flight at summon
    InterruptTaxiFlying();

    // drop flag at summon
    if (BattleGround *bg = GetBattleGround())
        bg->EventPlayerDroppedFlag(this);

    m_summon_expire = 0;

    TeleportTo(m_summon_mapid, m_summon_x, m_summon_y, m_summon_z,GetOrientation());
}

void Player::RemoveItemDurations(Item *item)
{
    for (ItemDurationList::iterator itr = m_itemDuration.begin();itr != m_itemDuration.end(); ++itr)
    {
        if (*itr==item)
        {
            m_itemDuration.erase(itr);
            break;
        }
    }
}

void Player::AddItemDurations(Item *item)
{
    if (item->GetUInt32Value(ITEM_FIELD_DURATION))
    {
        m_itemDuration.push_back(item);
        item->SendTimeUpdate(this);
    }
}

void Player::AutoUnequipOffhandIfNeed()
{
    Item *offItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offItem)
        return;

    Item *mainItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);

    if (!mainItem || mainItem->GetProto()->InventoryType != INVTYPE_2HWEAPON)
        return;

    ItemPosCountVec off_dest;
    uint8 off_msg = CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false);
    if (off_msg == EQUIP_ERR_OK)
    {
        RemoveItem(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND, true);
        StoreItem(off_dest, offItem, true);
    }
    else // TRENTONE FIXME CHEATCON01
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Player::EquipItem: Can's store offhand item at 2hand item equip for player (GUID: %u).",GetGUIDLow());
    }
}

OutdoorPvP * Player::GetOutdoorPvP() const
{
    return sOutdoorPvPMgr.GetOutdoorPvPToZoneId(GetCachedZone());
}

bool Player::HasItemFitToSpellReqirements(SpellEntry const* spellInfo, Item const* ignoreItem)
{
    if (spellInfo->EquippedItemClass < 0)
        return true;

    // scan other equipped items for same requirements (mostly 2 daggers/etc)
    // for optimize check 2 used cases only
    switch (spellInfo->EquippedItemClass)
    {
        case ITEM_CLASS_WEAPON:
        {
            for (int i= EQUIPMENT_SLOT_MAINHAND; i < EQUIPMENT_SLOT_TABARD; ++i)
                if (Item *item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    if (item!=ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                        return true;
            break;
        }
        case ITEM_CLASS_ARMOR:
        {
            // tabard not have dependent spells
            for (int i= EQUIPMENT_SLOT_START; i< EQUIPMENT_SLOT_MAINHAND; ++i)
                if (Item *item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    if (item!=ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                        return true;

            // shields can be equipped to offhand slot
            if (Item *item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                if (item!=ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                    return true;

            // ranged slot can have some armor subclasses
            if (Item *item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
                if (item!=ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                    return true;

            break;
        }
        default:
            sLog.outLog(LOG_DEFAULT, "ERROR: HasItemFitToSpellReqirements: Not handled spell requirement for item class %u",spellInfo->EquippedItemClass);
            break;
    }

    return false;
}

bool Player::CanNoReagentCast(SpellEntry const* spellInfo) const
{
    // don't take reagents for spells with SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP
    if (spellInfo->AttributesEx5 & SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP &&
        HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION))
        return true;

    return false;
}

void Player::RemoveItemDependentAurasAndCasts(Item * pItem)
{
    AuraMap& auras = GetAuras();
    for (AuraMap::iterator itr = auras.begin(); itr != auras.end();)
    {
        Aura* aura = itr->second;

        // skip passive (passive item dependent spells work in another way) and not self applied auras
        SpellEntry const* spellInfo = aura->GetSpellProto();
        if (aura->IsPassive() ||  aura->GetCasterGUID()!=GetGUID())
        {
            ++itr;
            continue;
        }

        // skip if not item dependent or have alternative item
        if (HasItemFitToSpellReqirements(spellInfo,pItem))
        {
            ++itr;
            continue;
        }

        // no alt item, remove aura, restart check
        RemoveAurasDueToSpell(aura->GetId());
        itr = auras.begin();
    }

    // currently cast spells can be dependent from item
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
    {
        if (m_currentSpells[i] && m_currentSpells[i]->getState()!=SPELL_STATE_DELAYED &&
            !HasItemFitToSpellReqirements(m_currentSpells[i]->GetSpellEntry(),pItem))
            InterruptSpell(i);
    }
}

uint32 Player::GetResurrectionSpellId()
{
    // search priceless resurrection possibilities
    uint32 prio = 0;
    uint32 spell_id = 0;
    AuraList const& dummyAuras = GetAurasByType(SPELL_AURA_DUMMY);
    for (AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
    {
        // Soulstone Resurrection                           // prio: 3 (max, non death persistent)
        if (prio < 2 && (*itr)->GetSpellProto()->SpellVisual == 99 && (*itr)->GetSpellProto()->SpellIconID == 92)
        {
            switch ((*itr)->GetId())
            {
                case 20707: spell_id =  3026; break;        // rank 1
                case 20762: spell_id = 20758; break;        // rank 2
                case 20763: spell_id = 20759; break;        // rank 3
                case 20764: spell_id = 20760; break;        // rank 4
                case 20765: spell_id = 20761; break;        // rank 5
                case 27239: spell_id = 27240; break;        // rank 6
                default:
                    sLog.outLog(LOG_DEFAULT, "ERROR: Unhandled spell %u: S.Resurrection",(*itr)->GetId());
                    continue;
            }

            prio = 3;
        }
        // Twisting Nether                                  // prio: 2 (max)
        else if ((*itr)->GetId()==23701 && roll_chance_i(10))
        {
            prio = 2;
            spell_id = 23700;
        }
    }

    // Reincarnation (passive spell)                        // prio: 1
    if (prio < 1 && HasSpell(20608) && !HasSpellCooldown(21169) && HasItemCount(17030,1))
        spell_id = 21169;

    return spell_id;
}

bool Player::RewardPlayerAndGroupAtKill(Unit* pVictim)
{
    bool PvP = pVictim->isCharmedOwnedByPlayerOrPlayer();

    // prepare data for near group iteration (PvP and !PvP cases)
    uint32 xp = 0;
    bool honored_kill = false;

    if (Group *pGroup = GetGroup())
    {
        uint32 count = 0;
        uint32 sum_level = 0;
        Player* member_with_max_level = NULL;
        Player* not_gray_member_with_max_level = NULL;
        Player* not_gray_with_def_rates = NULL;

        pGroup->GetDataForXPAtKill(pVictim,count,sum_level,member_with_max_level,not_gray_member_with_max_level, not_gray_with_def_rates);

        if (member_with_max_level)
        {
            // PvP kills doesn't yield experience
            // also no XP gained if there is no member below gray level
			if (PvP || !not_gray_member_with_max_level)
			{
				xp = 0;
			}
			else
			{
				if (not_gray_with_def_rates)
					xp = Hellground::XP::Gain(not_gray_with_def_rates, pVictim);
				else
					xp = Hellground::XP::Gain(not_gray_member_with_max_level, pVictim);
			}

            /// skip in check PvP case (for speed, not used)
            bool is_raid = PvP ? false : sMapStore.LookupEntry(GetMapId())->IsRaid() && pGroup->isRaidGroup();
            bool is_dungeon = PvP ? false : sMapStore.LookupEntry(GetMapId())->IsDungeon();
            float group_rate = Hellground::XP::xp_in_group_rate(count,is_raid);

            for (GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player* pGroupGuy = itr->getSource();
                if (!pGroupGuy)
                    continue;

                if (!pGroupGuy->IsAtGroupRewardDistance(pVictim))
                    continue;                               // member (alive or dead) or his corpse at req. distance

                // honor can be in PvP and !PvP (racial leader) cases (for alive)
                if (pGroupGuy->isAlive() && pGroupGuy->RewardHonor(pVictim,count, -1, true, pGroupGuy == this) && pGroupGuy==this)
                    honored_kill = true;

                // xp and reputation only in !PvP case
                if (!PvP)
                {
                    float rate = group_rate * float(pGroupGuy->GetLevel()) / sum_level;

                    // if is in dungeon then all receive full reputation at kill
                    // rewarded any alive/dead/near_corpse group member
                    pGroupGuy->RewardReputation(pVictim,is_dungeon ? 1.0f : rate);

                    // XP updated only for alive group member
                    if (pGroupGuy->isAlive() && not_gray_member_with_max_level &&
                       pGroupGuy->GetLevel() <= not_gray_member_with_max_level->GetLevel())
                    {
                        // not able to gain any exp from this mob
                        if (Hellground::XP::Gain(pGroupGuy, pVictim))
                        {
							uint32 tmp_exp = (xp && pGroupGuy->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1)) ? Hellground::XP::Gain(pGroupGuy, pVictim) : xp;
							uint32 itr_xp = (member_with_max_level == not_gray_member_with_max_level) ? uint32(tmp_exp*rate) : uint32((tmp_exp*rate / 2) + 1);

                            pGroupGuy->GiveXP(itr_xp, pVictim);
                            if (Pet* pet = pGroupGuy->GetPet())
                                pet->GivePetXP(itr_xp);
                        }
                    }

                    // quest objectives updated only for alive group member or dead but with not released body
                    if (pGroupGuy->isAlive()|| !pGroupGuy->GetCorpse())
                    {
                        // normal creature (not pet/etc) can be only in !PvP case
                        if (pVictim->GetTypeId() == TYPEID_UNIT)
                        {
                            pGroupGuy->KilledMonster(pVictim->GetEntry(), pVictim->GetGUID());

                            if (uint32 KillCredit = ((Creature*)pVictim)->GetCreatureInfo()->KillCredit)
                                pGroupGuy->KilledMonster(KillCredit, pVictim->GetGUID());
                        }
                    }
                }
            }
        }
    }
    else                                                    // if (!pGroup)
    {
        xp = PvP ? 0 : Hellground::XP::Gain(this, pVictim);

        // honor can be in PvP and !PvP (racial leader) cases
        if (RewardHonor(pVictim,1, -1, true))
            honored_kill = true;

        // xp and reputation only in !PvP case
        if (!PvP)
        {
            RewardReputation(pVictim,1);
            GiveXP(xp, pVictim);

            if (Pet* pet = GetPet())
                pet->GivePetXP(xp);

            // normal creature (not pet/etc) can be only in !PvP case
            if (pVictim->GetTypeId()==TYPEID_UNIT)
            {
                KilledMonster(pVictim->GetEntry(),pVictim->GetGUID());

                if (uint32 KillCredit = ((Creature*)pVictim)->GetCreatureInfo()->KillCredit)
                    KilledMonster(KillCredit, pVictim->GetGUID());
            }
        }
    }
    //if (pVictim->GetTypeId() == TYPEID_PLAYER && !pVictim->ToPlayer()->InBattleGroundOrArena())
    //    pVictim->CastSpell(pVictim, 55115, true);
    return xp || honored_kill;
}

bool Player::IsAtGroupRewardDistance(WorldObject const* pRewardSource) const
{
    WorldObject const* player = GetCorpse();
    if (!player || isAlive())
        player = this;

    if (player->GetMapId() != pRewardSource->GetMapId() || player->GetAnyInstanceId() != pRewardSource->GetAnyInstanceId())
        return false;

    return pRewardSource->GetDistance(player) <= sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE);
}

uint32 Player::GetBaseWeaponSkillValue (WeaponAttackType attType) const
{
    Item* item = GetWeaponForAttack(attType,true);

    // unarmed only with base attack
    if (attType != BASE_ATTACK && !item)
        return 0;

    // weapon skill or (unarmed for base attack)
    uint32  skill = (item && item->GetSkill() != SKILL_FIST_WEAPONS) ? item->GetSkill() : uint32(SKILL_UNARMED);
    return GetBaseSkillValue(skill);
}

void Player::ResurectUsingRequestData()
{
    SpawnCorpseBones();

    /// Teleport before resurrecting by player, otherwise the player might get attacked from creatures near his corpse
    if (IS_PLAYER_GUID(m_resurrectGUID))
        TeleportTo(m_resurrectMap, m_resurrectX, m_resurrectY, m_resurrectZ, GetOrientation(), TELE_TO_RESURRECT);

    ResurrectPlayer(0.0f,false);

    if (GetMaxHealth() > m_resurrectHealth)
        SetHealth(m_resurrectHealth);
    else
        SetHealth(GetMaxHealth());

    if (GetMaxPower(POWER_MANA) > m_resurrectMana)
        SetPower(POWER_MANA, m_resurrectMana);
    else
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));

    SetPower(POWER_RAGE, 0);

    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
}

void Player::SetClientControl(Unit* target, uint8 allowMove)
{
    if (target->GetObjectGuid().IsPlayer())
    {
        if (allowMove && this == target && target->isPossessed())
            return;
    }

    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, target->GetPackGUID().size()+1);
    data << target->GetPackGUID();
    data << uint8(allowMove);
    SendPacketToSelf(&data);
}

void Player::UpdateZoneDependentAuras(uint32 newZone)
{    
    if(!isGameMaster() && !IsSpectator())
    {
        if (GetVirtualMapForMapAndZone(GetMapId(),newZone) != 530)
        {
            RemoveSpellsCausingAura(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED);
            RemoveSpellsCausingAura(SPELL_AURA_FLY);
        }
    }

    // set/restore AV crossfaction
	if (BattleGround* bg = GetBattleGround())
	{
        if (bg && bg->isBattleGround())
        {
            if (!HasAura(SPELL_BG_HORDE_ICON) && !HasAura(SPELL_BG_ALLIANCE_ICON))
                CastSpell(this, GetBGTeam() == HORDE ? SPELL_BG_HORDE_ICON : SPELL_BG_ALLIANCE_ICON, true);

            if (bg->GetTypeID() == BATTLEGROUND_AV)
            {
                if (GetBGTeam() != GetTeam())
                    CastSpell(this, GetBGTeam() == ALLIANCE ? SPELL_AV_CROSSFAC_H_A : SPELL_AV_CROSSFAC_A_H, true);
            }
        }
	}

    // Some spells applied at enter into zone (with subzones)
    // Human Illusion
    // NOTE: these are removed by RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP);
    if (newZone == 2367)                                  // Old Hillsbrad Foothills
    {
        uint32 spellid = 0;
        // all horde races
        if (GetTeam() == HORDE)
            spellid = (GetGender() == GENDER_FEMALE ? 35481 : 35480);
        // and some alliance races
        else if (GetRace() == RACE_NIGHTELF || GetRace() == RACE_DRAENEI)
            spellid = (GetGender() == GENDER_FEMALE ? 35483 : 35482);

        if (spellid && !HasAura(spellid,0))
            CastSpell(this,spellid,true);
    }
}

void Player::UpdateAreaDependentAuras(uint32 newArea)
{
    // remove auras from spells with area limitations
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        // use m_zoneUpdateId for speed: UpdateArea called from UpdateZone or instead UpdateZone in both cases m_zoneUpdateId up-to-date
        if (!SpellMgr::IsSpellAllowedInLocation(iter->second->GetSpellProto(),GetMapId(),m_zoneUpdateId,newArea))
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                if (iter->second->GetSpellProto()->Effect[i] == SPELL_EFFECT_TRIGGER_SPELL && HasAura(iter->second->GetSpellProto()->EffectTriggerSpell[i],0))
                    RemoveAurasDueToSpell(iter->second->GetSpellProto()->EffectTriggerSpell[i]);
            }
            // also added unstable flasks here, i suppose they shouldn't be removed too
            if (sSpellMgr.GetSpellElixirMask(iter->second->GetSpellProto()->Id) & (ELIXIR_UNSTABLE_MASK | ELIXIR_SHATTRATH_MASK))        // for shattrath flasks we want only to remove it's triggered effect, not flask itself.
                iter++;
            else
                RemoveAura(iter);
        }
        else
        {
            // reapply bonus for shattrath flask if we are back in allowed location
            if (sSpellMgr.GetSpellElixirMask(iter->second->GetSpellProto()->Id) & (ELIXIR_UNSTABLE_MASK | ELIXIR_SHATTRATH_MASK))
            {
                for (uint8 i = 1; i < 3; ++i)
                {
                    if (iter->second->GetSpellProto()->Effect[i] == SPELL_EFFECT_TRIGGER_SPELL &&
                            !HasAura(iter->second->GetSpellProto()->EffectTriggerSpell[i]))
                    {
                        CastSpell(this, iter->second->GetSpellProto()->EffectTriggerSpell[i], true);
                    }
                }
            }
            ++iter;
        }
    }

    switch (newArea)
    {
        // unmount if enter in this subzone
        case 35:
            RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);
            break;
        // Dragonmaw Illusion
        case 3759:
        case 3966:
        case 3939:
        case 3965:
        case 3967:
            if (GetDummyAura(40214))
            {
                if (!HasAura(40216,0))
                    CastSpell(this, 40216, true);
                if (!HasAura(42016,0))
                    CastSpell(this, 42016, true);
            }
            break;
        case 3518:
        {
            if(GetQuestStatus(10168) == QUEST_STATUS_AVAILABLE || GetQuestStatus(10168) == QUEST_STATUS_INCOMPLETE)
            {
                if (!HasAura(32649,0))
                    CastSpell(this, 32649, true);
            }
            break;
        }
        case 3733:
        {
            if(GetQuestStatus(10176) != QUEST_STATUS_COMPLETE)
            {
                if (!HasAura(34102,0))
                    CastSpell(this, 34102, true);
            }
            break;
        }
    }
}

uint32 Player::GetCorpseReclaimDelay(bool pvp) const
{
    if (pvp)
    {
        if (!sWorld.getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP))
            return copseReclaimDelay[0];
    }
    else if (!sWorld.getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE))
        return 0;

    time_t now = time(NULL);
    // 0..2 full period
    // should be ceil(x)-1 but not floor(x)
    uint32 count = (now < m_deathExpireTime - 1) ? (m_deathExpireTime - 1 - now)/DEATH_EXPIRE_STEP : 0;
    return copseReclaimDelay[count];
}

void Player::UpdateCorpseReclaimDelay()
{
    bool pvp = m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH;

    if (pvp && !sWorld.getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP) ||
        !pvp && !sWorld.getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE))
        return;

    time_t now = time(NULL);
    if (now < m_deathExpireTime)
    {
        // full and partly periods 1..3
        uint32 count = (m_deathExpireTime - now)/DEATH_EXPIRE_STEP +1;
        if (count < MAX_DEATH_COUNT)
            m_deathExpireTime = now+(count+1)*DEATH_EXPIRE_STEP;
        else
            m_deathExpireTime = now+MAX_DEATH_COUNT*DEATH_EXPIRE_STEP;
    }
    else
        m_deathExpireTime = now+DEATH_EXPIRE_STEP;
}

void Player::SendCorpseReclaimDelay(bool load)
{
    Corpse* corpse = GetCorpse();
    if (load && !corpse)
        return;

    bool pvp;
    if (corpse)
        pvp = (corpse->GetType() == CORPSE_RESURRECTABLE_PVP);
    else
        pvp = (m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH);

    uint32 delay;
    if (load)
    {
        if (corpse->GetGhostTime() > m_deathExpireTime)
            return;

        uint32 count;
        if (pvp && sWorld.getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP) ||
           !pvp && sWorld.getConfig(CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE))
        {
            count = (m_deathExpireTime-corpse->GetGhostTime())/DEATH_EXPIRE_STEP;
            if (count>=MAX_DEATH_COUNT)
                count = MAX_DEATH_COUNT-1;
        }
        else
            count=0;

        time_t expected_time = corpse->GetGhostTime()+copseReclaimDelay[count];

        time_t now = time(NULL);
        if (now >= expected_time)
            return;

        delay = expected_time-now;
    }
    else
        delay = GetCorpseReclaimDelay(pvp);

    if (!delay) return;

    //! corpse reclaim delay 30 * 1000ms or longer at often deaths
    WorldPacket data(SMSG_CORPSE_RECLAIM_DELAY, 4);
    data << uint32(delay*1000);
    SendPacketToSelf(&data);
}

PartyResult Player::CanUninviteFromGroup() const
{
    const Group* grp = GetGroup();
    if (!grp)
        return PARTY_RESULT_YOU_NOT_IN_GROUP;

    if (!grp->IsLeader(GetGUID()) && !grp->IsAssistant(GetGUID()))
        return PARTY_RESULT_YOU_NOT_LEADER;

    if (InBattleGroundOrArena())
        return PARTY_RESULT_INVITE_RESTRICTED;

    return PARTY_RESULT_OK;
}

void Player::LFGAttemptJoin()
{
    // skip autojoin disabled and player in group cases
    if (m_lookingForGroup.canAutoJoin() || GetGroup())
        return;

    bool found = false;
    std::list<uint64> fullList;

    LfgContainerType & lfgContainer = sWorld.GetLfgContainer(GetTeam());
    for (uint8 i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
    {
        // skip empty slot
        if (m_lookingForGroup.slots[i].Empty())
            continue;

        LfgContainerType::const_accessor a; // const_accessor -> write lock only

        // skip if container doesn't exist
        if (!lfgContainer.find(a, m_lookingForGroup.slots[i].Combine()))
            continue;

        for (std::list<uint64>::const_iterator itr = a->second.begin(); itr != a->second.end(); ++itr)
        {
            // skip self
            if ((*itr) == GetGUID())
                continue;

            Player * plr = ObjectAccessor::GetPlayerInWorldOrNot(*itr);

            // skip not existed
            if (!plr)
                continue;

            if (!plr->IsInWorld())
                continue;

             // skip not auto add, not group leader cases
            if (!plr->GetSession()->LookingForGroup_auto_add || plr->GetGroup() && plr->GetGroup()->GetLeaderGUID()!=plr->GetGUID())
                continue;

            // skip non auto-join or empty slots, or non compatible slots
            if (!plr->m_lookingForGroup.more.canAutoJoin() || !m_lookingForGroup.HaveInSlot(plr->m_lookingForGroup.more))
                continue;

            // attempt create group, or skip
            if (!plr->GetGroup())
            {
                Group* group = new Group;
                if (!group->Create(plr->GetGUID(), plr->GetName(), true))
                {
                    delete group;
                    continue;
                }

                sObjectMgr.AddGroup(group);
            }

            // stop at success join
            if (plr->GetGroup()->AddMember(GetGUID(), GetName(), true))
            {
                LeaveLFGChannel();
                found = true;
                break;
            }
            // full
            else
            {
                plr->LeaveLFGChannel();

                fullList.push_back(*itr);
            }
        }
    }

    if (found)
    {
        ClearLFG();
        ClearLFM();
    }

    for (std::list<uint64>::const_iterator itr = fullList.begin(); itr != fullList.end(); ++itr)
    {
        Player * plr = ObjectAccessor::GetPlayerInWorldOrNot(*itr);

        if (!plr)
            continue;

        plr->ClearLFM();
    }
}

void Player::LFMAttemptAddMore()
{
     // skip not group leader case
    if (GetGroup() && GetGroup()->GetLeaderGUID() != GetGUID())
        return;

    // skip not autojoin and empty cases
    if (!m_lookingForGroup.more.canAutoJoin() || m_lookingForGroup.more.Empty())
        return;

    LfgContainerType::const_accessor a;

    // get player container for LFM id
    LfgContainerType & lfgContainer = sWorld.GetLfgContainer(GetTeam());
    if (!lfgContainer.find(a, m_lookingForGroup.more.Combine()))
        return;

    std::list<uint64> joinedList;

    for (std::list<uint64>::const_iterator iter = a->second.begin(); iter != a->second.end(); ++iter)
    {
        Player *plr = ObjectAccessor::GetPlayerInWorldOrNot(*iter);

        if (!plr || !plr->IsInWorld())
            continue;

        // skip self
        if (plr->GetGUID() == GetGUID())
            continue;

        // skip not auto join or in group
        if (!plr->GetSession()->LookingForGroup_auto_join || plr->GetGroup())
            continue;

        if (!plr->m_lookingForGroup.HaveInSlot(m_lookingForGroup.more))
            continue;

        // attempt create group if need, or stop attempts
        if (!GetGroup())
        {
            Group* group = new Group;
            if (!group->Create(GetGUID(), GetName(), true))
            {
                delete group;
                return;                                     // can't create group (??)
            }

            sObjectMgr.AddGroup(group);
        }

        // stop at join fail (full)
        if (!GetGroup()->AddMember(plr->GetGUID(), plr->GetName(), true))
        {
            LeaveLFGChannel();

            break;
        }

        // joined
        plr->LeaveLFGChannel();

        joinedList.push_back(*iter);

        // and group full
        if (GetGroup()->IsFull())
        {
            LeaveLFGChannel();
            break;
        }
    }

    a.release();

    // clear LFG and LFM for players joined to our pt
    for (std::list<uint64>::const_iterator itr = joinedList.begin(); itr != joinedList.end(); ++itr)
    {
        Player *plr = ObjectAccessor::GetPlayerInWorldOrNot(*itr);

        if (!plr)
            continue;

        plr->ClearLFG();
        plr->ClearLFM();
    }
}

void Player::LFGSet(uint8 slot, uint32 entry, uint32 type)
{
    if (slot >= MAX_LOOKING_FOR_GROUP_SLOT)
        return;

    // don't add GM to lfg list
    if (GetSession()->HasPermissions(PERM_GMT))
        return;

    LfgContainerType::accessor a;
    uint64 guid = GetGUID();
    uint32 combined;

    // if not empty then clear slot
    LfgContainerType & lfgContainer = sWorld.GetLfgContainer(GetTeam());
    if (!m_lookingForGroup.slots[slot].Empty())
    {
        combined = m_lookingForGroup.slots[slot].Combine();

        if (lfgContainer.find(a, combined))
        {
            // remove player from list
            for (std::list<uint64>::iterator itr = a->second.begin(); itr != a->second.end();)
            {
                std::list<uint64>::iterator tmpItr = itr;
                ++itr;

                if ((*tmpItr) == guid)
                    a->second.erase(tmpItr);
            }
        }

        m_lookingForGroup.slots[slot].Clear();
        a.release();
    }

    combined = LFG_COMBINE(entry, type);

    // if he want set empty then only clean slot
    if (!combined)
    {
        for (uint8 i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
            if (!m_lookingForGroup.slots[i].Empty())
                return;

        // clear LFM (for sure, client resets presets in LFM when LFG is empty) if lfg is cleaned
        ClearLFM(false);
        LeaveLFGChannel();
        return;
    }

    // if we can't find list in container or add new list
    if (!lfgContainer.find(a, combined))
        if (!lfgContainer.insert(a, combined))
            return;

    m_lookingForGroup.slots[slot].Set(entry, type);
    a->second.push_back(guid);

    JoinLFGChannel();
}

void Player::LFMSet(uint32 entry, uint32 type)
{
    // don't add GM to lfm list
    if (GetSession()->HasPermissions(PERM_GMT))
        return;

    // don't add to lfm list if still in lfg
    for (uint8 i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
        if (!m_lookingForGroup.slots[i].Empty())
            return;

    // don't add to lfm if in group and not leader (for cases when group member wants check instance list or for raid assists)
    if (GetGroup() && !GetGroup()->IsLeader(GetGUID()))
        return;

    // clear lfg when player want looking for more
    ClearLFG(false);
    LfgContainerType::accessor a;   // accessor - read and write lock

    uint64 guid = GetGUID();
    uint32 combined;

    LfgContainerType & lfgContainer = sWorld.GetLfgContainer(GetTeam());
    if (!m_lookingForGroup.more.Empty())
    {
        combined = m_lookingForGroup.more.Combine();

        if (lfgContainer.find(a, combined))
        {
            // remove player from list
            for (std::list<uint64>::iterator itr = a->second.begin(); itr != a->second.end();)
            {
                std::list<uint64>::iterator tmpItr = itr;
                ++itr;

                if ((*tmpItr) == guid)
                    a->second.erase(tmpItr);
            }
        }

        m_lookingForGroup.more.Clear();
        a.release();
    }

    combined = LFG_COMBINE(entry, type);

    // if we can't find list in container or add new list
    if (!lfgContainer.find(a, combined))
        if (!lfgContainer.insert(a, combined))
            return;

    m_lookingForGroup.more.Set(entry, type);
    a->second.push_back(guid);
    GetSession()->SendUpdateLFM();

    JoinLFGChannel();
}

void Player::ClearLFG(bool leaveChannel)
{
    bool wasEmpty = true;
    LfgContainerType & lfgContainer = sWorld.GetLfgContainer(GetTeam());
    for (uint8 i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
    {
        if (m_lookingForGroup.slots[i].Empty())
            continue;

        wasEmpty = false;

        LfgContainerType::accessor a;

        if (!lfgContainer.find(a, GetLFGCombined(i)))
            continue;

        // remove player from list
        for (std::list<uint64>::iterator itr = a->second.begin(); itr != a->second.end();)
        {
            std::list<uint64>::iterator tmpItr = itr;
            ++itr;

            if ((*tmpItr) == GetGUID())
                a->second.erase(tmpItr);
        }

        m_lookingForGroup.slots[i].Clear();
    }

    if (leaveChannel)
        LeaveLFGChannel();

    // don't send update lfg if lfg was empty
    if (!wasEmpty)
        GetSession()->SendUpdateLFG();
}

void Player::ClearLFM(bool leaveChannel)
{
    // don't clear empty slot
    if (m_lookingForGroup.more.Empty())
        return;

    LfgContainerType::accessor a;

    LfgContainerType & lfgContainer = sWorld.GetLfgContainer(GetTeam());
    if (!lfgContainer.find(a, m_lookingForGroup.more.Combine()))
        return;

    // remove player from list
    for (std::list<uint64>::iterator itr = a->second.begin(); itr != a->second.end();)
    {
        std::list<uint64>::iterator tmpItr = itr;
        ++itr;

        if ((*tmpItr) == GetGUID())
            a->second.erase(tmpItr);
    }

    m_lookingForGroup.more.Clear();

    if (leaveChannel)
        LeaveLFGChannel();

    GetSession()->SendUpdateLFM();
}

uint8 Player::IsLFM(uint32 type, uint32 entry)
{
    if (m_lookingForGroup.more.Is(entry, type))
        return 1;

    return 0;
}

uint32 Player::GetLFGCombined(uint8 slot)
{
    if (slot >= MAX_LOOKING_FOR_GROUP_SLOT)
        return 0;

    return m_lookingForGroup.slots[slot].Combine();
}

uint32 Player::GetLFMCombined()
{
    return m_lookingForGroup.more.Combine();
}

void Player::SetBattleGroundRaid(Group* group, int8 subgroup)
{
    //we must move references from m_group to m_originalGroup
    SetOriginalGroup(GetGroup(), GetSubGroup());

    m_group.unlink();
    m_group.link(group, this);
    m_group.setSubGroup((uint8)subgroup);
}

void Player::RemoveFromBattleGroundRaid()
{
    //remove existing reference
    m_group.unlink();
    if (Group* group = GetOriginalGroup())
    {
         m_group.link(group, this);
        m_group.setSubGroup(GetOriginalSubGroup());
    }
    SetOriginalGroup(NULL);
}

void Player::SetOriginalGroup(Group *group, int8 subgroup)
{
    if (group == NULL)
        m_originalGroup.unlink();
    else
    {
        // never use SetOriginalGroup without a subgroup unless you specify NULL for group
        ASSERT(subgroup >= 0);
        m_originalGroup.link(group, this);
        m_originalGroup.setSubGroup((uint8)subgroup);
    }
}

// Called on player-teleport, Spline movement(server-side movement), or ANY player relocation - VERY OFTEN
void Player::UpdateUnderwaterState(Map* m, float x, float y, float z)
{
    GridMapLiquidData liquid_status;
    GridMapLiquidStatus res = m->GetTerrain()->getLiquidStatusIfSwimmingOrSpecial(x, y, z, &liquid_status, IsSwimming());
    if (!res)
    {
        m_MirrorTimerFlags &= ~(UNDERWATER_INWATER|UNDERWATER_INLAVA|UNDERWATER_INSLIME|UNDERWATER_INDARKWATER);
        return;
    }

    // All liquids type - check under water position
    if (liquid_status.type&(MAP_LIQUID_TYPE_WATER|MAP_LIQUID_TYPE_OCEAN|MAP_LIQUID_TYPE_MAGMA|MAP_LIQUID_TYPE_SLIME))
    {
        if (res & LIQUID_MAP_UNDER_WATER)
            m_MirrorTimerFlags |= UNDERWATER_INWATER;
        else
            m_MirrorTimerFlags &= ~UNDERWATER_INWATER;
    }

    // Allow travel in dark water on taxi or transport
    if ((liquid_status.type & MAP_LIQUID_TYPE_DARK_WATER) && !IsTaxiFlying() && !GetTransport())
        m_MirrorTimerFlags |= UNDERWATER_INDARKWATER;
    else
        m_MirrorTimerFlags &= ~UNDERWATER_INDARKWATER;

    // in lava check, anywhere in lava level
    if (liquid_status.type&MAP_LIQUID_TYPE_MAGMA)
    {
        if (res & (LIQUID_MAP_UNDER_WATER|LIQUID_MAP_IN_WATER|LIQUID_MAP_WATER_WALK))
            m_MirrorTimerFlags |= UNDERWATER_INLAVA;
        else
            m_MirrorTimerFlags &= ~UNDERWATER_INLAVA;
    }
    // in slime check, anywhere in slime level
    if (liquid_status.type&MAP_LIQUID_TYPE_SLIME)
    {
        if (res & (LIQUID_MAP_UNDER_WATER|LIQUID_MAP_IN_WATER|LIQUID_MAP_WATER_WALK))
            m_MirrorTimerFlags |= UNDERWATER_INSLIME;
        else
            m_MirrorTimerFlags &= ~UNDERWATER_INSLIME;
    }
}

void Player::SetCanParry(bool value)
{
    if (m_canParry == value)
        return;

    m_canParry = value;
    UpdateParryPercentage();
}

void Player::SetCanBlock(bool value)
{
    if (m_canBlock == value)
        return;

    m_canBlock = value;
    UpdateBlockPercentage();
}

bool ItemPosCount::isContainedIn(ItemPosCountVec const& vec) const
{
    for (ItemPosCountVec::const_iterator itr = vec.begin(); itr != vec.end();++itr)
    {
        if (itr->pos == pos)
            return true;
    }

    return false;
}

//***********************************
//-------------TRINITY---------------
//***********************************

void Player::HandleFallDamage(MovementInfo& movementInfo)
{
    if (movementInfo.GetFallTime() < 1500)
        return;

    // calculate total z distance of the fall
    float z_diff = m_lastFallZ - movementInfo.GetPos()->z;
    uint32 areaID = GetMap()->GetId();
    sLog.outDebug("zDiff = %f", z_diff);

    //Players with low fall distance, Feather Fall or physical immunity (charges used) are ignored
    // 14.57 can be calculated by resolving damageperc formular below to 0
    if (z_diff >= 14.57f && !isDead() && !isGameMaster() &&
        !HasAuraType(SPELL_AURA_HOVER) && !HasAuraType(SPELL_AURA_FEATHER_FALL) &&
        !(HasAuraType(SPELL_AURA_FLY) && areaID != 550) && !IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))  //do not check for fly aura in Tempest Keep:Eye to properly deal fall dmg when after knockback (Gravity Lapse)
    {
        //Safe fall, fall height reduction
        int32 safe_fall = GetTotalAuraModifier(SPELL_AURA_SAFE_FALL);

        float damageperc = 0.018f*(z_diff-safe_fall)-0.2426f;

        if (damageperc >0)
        {
            uint32 damage = (uint32)(damageperc * GetMaxHealth()*sWorld.getConfig(RATE_DAMAGE_FALL));

            if (damage > 0)
            {
                //Prevent fall damage from being more than the player maximum health
                if (damage > GetMaxHealth())
                    damage = GetMaxHealth();

                // Gust of Wind
                if (GetDummyAura(43621))
                    damage = GetMaxHealth()/2;

                EnvironmentalDamage(DAMAGE_FALL, damage);
            }
        }
    }
}

void Player::HandleFallUnderMap(float z)
{
    if (z > FALL_UNDER_MAP_HEIGHT)
        return;

    if (InBattleGroundOrArena() && GetBattleGround())
        GetBattleGround()->HandlePlayerUnderMap(this, z);
    else
    {
        // NOTE: this is actually called many times while falling
        // even after the player has been teleported away
        // TODO: discard movement packets after the player is rooted
        if (isAlive())
        {
            EnvironmentalDamage(DAMAGE_FALL_TO_VOID, GetMaxHealth());
            // change the death state to CORPSE to prevent the death timer from
            // starting in the next player update
            KillPlayer();
            BuildPlayerRepop();
        }

        // cancel the death timer here if started
        RepopAtGraveyard();
    }
}

WorldObject* Player::GetFarsightTarget() const
{
    // Players can have in farsight field another player's guid, a creature's guid, or a dynamic object's guid
    if (uint64 guid = GetUInt64Value(PLAYER_FARSIGHT))
        return (WorldObject*)GetMap()->GetObjectByTypeMask(*this, guid, TYPEMASK_PLAYER | TYPEMASK_UNIT | TYPEMASK_DYNAMICOBJECT);
    return NULL;
}

void Player::StopCastingBindSight()
{
    if (WorldObject* target = GetFarsightTarget())
    {
        if (target->isType(TYPEMASK_UNIT))
        {
            ((Unit*)target)->RemoveAuraTypeByCaster(SPELL_AURA_BIND_SIGHT, GetGUID());
            ((Unit*)target)->RemoveAuraTypeByCaster(SPELL_AURA_MOD_POSSESS, GetGUID());
            ((Unit*)target)->RemoveAuraTypeByCaster(SPELL_AURA_MOD_POSSESS_PET, GetGUID());
        }
    }
}

void Player::ClearFarsight()
{
    if (GetUInt64Value(PLAYER_FARSIGHT))
    {
        SetUInt64Value(PLAYER_FARSIGHT, 0);
        WorldPacket data(SMSG_CLEAR_FAR_SIGHT_IMMEDIATE, 0);
        SendPacketToSelf(&data);
    }
}

void Player::SetFarsightTarget(WorldObject* obj)
{
    if (!obj || !obj->isType(TYPEMASK_PLAYER | TYPEMASK_UNIT | TYPEMASK_DYNAMICOBJECT))
        return;

    // Remove the current target if there is one
    StopCastingBindSight();

    SetUInt64Value(PLAYER_FARSIGHT, obj->GetGUID());
}

bool Player::isAllowUseBattleGroundObject()
{
	return (//InBattleGround() &&                               // in battleground - not need, check in other cases
		!HasAura(30452) &&								   // rocket boots aura
		!IsMounted() &&                                    // not mounted
		!isTotalImmunity() &&                              // not totally immuned
		!HasStealthAura() &&                               // not stealthed
		!HasInvisibilityAura() &&                          // not invisible
		!HasAura(SPELL_RECENTLY_DROPPED_FLAG) &&           // can't pickup
		isAlive() &&                                       // live player
		!HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION)      // isn't in Spirit of Redemption form
		);
}

bool Player::HasTitle(uint32 bitIndex)
{
    if (bitIndex > 128)
        return false;

    uint32 fieldIndexOffset = bitIndex/32;
    uint32 flag = 1 << (bitIndex%32);
    return HasFlag(PLAYER__FIELD_KNOWN_TITLES+fieldIndexOffset, flag);
}

void Player::SetTitle(CharTitlesEntry const* title, bool lost)
{
    uint32 fieldIndexOffset = title->bit_index / 32;
    uint32 flag = 1 << (title->bit_index % 32);

    if (lost)
    {
        if (!HasFlag(PLAYER__FIELD_KNOWN_TITLES + fieldIndexOffset, flag))
            return;

        RemoveFlag(PLAYER__FIELD_KNOWN_TITLES + fieldIndexOffset, flag);
    }
    else
    {
        if (HasFlag(PLAYER__FIELD_KNOWN_TITLES + fieldIndexOffset, flag))
            return;

        SetFlag(PLAYER__FIELD_KNOWN_TITLES + fieldIndexOffset, flag);
    }

    WorldPacket data(SMSG_TITLE_EARNED, 4 + 4);
    data << uint32(title->bit_index);
    data << uint32(lost ? 0 : 1);                           // 1 - earned, 0 - lost
    SendPacketToSelf(&data);
}

void Player::AutoStoreLoot(uint8 bag, uint8 slot, uint32 loot_id, LootStore const& store, bool broadcast)
{
    Loot loot;
    loot.FillLoot (loot_id,store,this,true, 0);
    if(loot.items.empty ())
        return;
    LootItem const* lootItem = &loot.items[0];

    ItemPosCountVec dest;
    uint8 msg = CanStoreNewItem (bag,slot,dest,lootItem->itemid,lootItem->count);
    if(msg != EQUIP_ERR_OK && slot != NULL_SLOT)
        msg = CanStoreNewItem( bag, NULL_SLOT,dest,lootItem->itemid,lootItem->count);
    if( msg != EQUIP_ERR_OK && bag != NULL_BAG)
        msg = CanStoreNewItem( NULL_BAG, NULL_SLOT,dest,lootItem->itemid,lootItem->count);
    if(msg != EQUIP_ERR_OK)
    {
        SendEquipError( msg, NULL, NULL );
        return;
    }

    Item* pItem = StoreNewItem (dest,lootItem->itemid,true,lootItem->randomPropertyId, "LOOT_AUTOSTORE");
    SendNewItem(pItem, lootItem->count, false, false,broadcast);
}


/*-----------------------TRINITY--------------------------*/
bool Player::isTotalImmunity()
{
    AuraList const& immune = GetAurasByType(SPELL_AURA_SCHOOL_IMMUNITY);

    for (AuraList::const_iterator itr = immune.begin(); itr != immune.end(); ++itr)
    {
        if (((*itr)->GetModifier()->m_miscvalue & SPELL_SCHOOL_MASK_ALL) !=0)   // total immunity
        {
            return true;
        }
        if (((*itr)->GetModifier()->m_miscvalue & SPELL_SCHOOL_MASK_NORMAL) !=0)   // physical damage immunity
        {
            for (AuraList::const_iterator i = immune.begin(); i != immune.end(); ++i)
            {
                if (((*i)->GetModifier()->m_miscvalue & SPELL_SCHOOL_MASK_MAGIC) !=0)   // magic immunity
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void Player::BuildTeleportAckMsg(WorldPacket& data, float x, float y, float z, float ang) const
{
    MovementInfo mi = m_movementInfo;
    mi.ChangePosition(x, y, z, ang);
    data.Initialize(MSG_MOVE_TELEPORT_ACK, 41);
    data << GetPackGUID();
    data << uint32(0); // this value increments every time
    data << mi;
}

void Player::ResetTimeSync()
{
    m_timeSyncCounter = 0;
    m_timeSyncTimer = 0;
    m_timeSyncClient = 0;
    m_timeSyncServer = WorldTimer::getMSTime();
}

void Player::SendTimeSync()
{
    WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
    data << uint32(m_timeSyncCounter++);
    SendPacketToSelf(&data);

    // Schedule next sync in 10 sec
    m_timeSyncTimer = 10000;
    m_timeSyncServer = WorldTimer::getMSTime();
}

float Player::GetXPRate(Rates rate) const
{
    return IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1) ? 1.0f : sWorld.getConfig(Rates(rate));
}

Position Player::GetShadowstepPoint(Unit* target)
{
    Position pos;
    target->GetValidPointInAngle(pos, 2.0f, M_PI, true);

    // Case: Target is standing with back against the end of a cliff, bridge or
    //        wall
    if (target->GetDistance(pos.x, pos.y, pos.z) < 1.5f)
    {
        Position far_away2d = target->GetPoint2d(M_PI, 5.0f);
        auto far_away = Position(far_away2d.x, far_away2d.y, GetPositionZ());
        auto to = target->GetOrientation();
        // If LOS is not blocked and ADT is below target Z: we can use 2 yards
        // away exactly
        if (IsWithinLOS(far_away.x, far_away.y, far_away.z + 2.0f))
        {
            auto new_pos = Position(target->GetPositionX() + 2.0f * cos(to + M_PI), target->GetPositionY() + 2.0f * sin(to + M_PI), target->GetPositionZ());
            if (GetTerrain()->GetHeight(new_pos.x, new_pos.y, new_pos.z, false) - 0.5f < new_pos.z)
            {
                UpdateAllowedPositionZ_WithUndermap(new_pos.x, new_pos.y, new_pos.z, true);
                return new_pos;
            }
        }
    }
    // Good ol' pos works fine
    return pos;
}

uint32 Player::CalculateTalentsPoints() const
{
    uint32 talentPointsForLevel = GetLevel() < 10 ? 0 : GetLevel()-9;
    return uint32(talentPointsForLevel * sWorld.getConfig(RATE_TALENT));
}

BattleGroundBracketId Player::GetBattleGroundBracketIdFromLevel(BattleGroundTypeId bgTypeId) const
{
    BattleGround *bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    ASSERT(bg);
    if (GetLevel() < bg->GetMinLevel())
        return BG_BRACKET_ID_FIRST;

    uint32 bracket_id = (GetLevel() - bg->GetMinLevel()) / 10;
    if (bracket_id > MAX_BATTLEGROUND_BRACKETS)
        return BG_BRACKET_ID_LAST;

    return BattleGroundBracketId(bracket_id);
}

void Player::InterruptTaxiFlying()
{
    // stop flight if need
    if (IsTaxiFlying())
    {
        GetUnitStateMgr().DropAction(UNIT_ACTION_TAXI);
        m_taxi.ClearTaxiDestinations();
        GetUnitStateMgr().InitDefaults(false);
    }
    // save only in non-flight case
    else
        SaveRecallPosition();
}

// Refer-A-Friend
void Player::SendReferFriendError(ReferAFriendError err, Player * target)
{
    WorldPacket data(SMSG_REFER_A_FRIEND_ERROR, 24);
    data << uint32(err);
    if (target && (err == ERR_REFER_A_FRIEND_NOT_IN_GROUP || err == ERR_REFER_A_FRIEND_SUMMON_OFFLINE_S))
        data << target->GetName();

    GetSession()->SendPacket(&data);
}

ReferAFriendError Player::GetReferFriendError(Player * target, bool summon)
{
    if (!target || target->GetTypeId() != TYPEID_PLAYER)
        return summon ? ERR_REFER_A_FRIEND_SUMMON_OFFLINE_S : ERR_REFER_A_FRIEND_NO_TARGET;

    if (!GetSession()->isRAFConnectedWith(target->GetSession()))
        return ERR_REFER_A_FRIEND_NOT_REFERRED_BY;

    if (!IsInSameRaidWith(target))
        return ERR_REFER_A_FRIEND_NOT_IN_GROUP;

    if (summon)
    {
        if (target->GetLevel() > sWorld.getConfig(CONFIG_UINT32_RAF_MAXGRANTLEVEL))
            return ERR_REFER_A_FRIEND_SUMMON_LEVEL_MAX_I;

		if (GetLevel() > sWorld.getConfig(CONFIG_UINT32_RAF_MAXGRANTLEVEL))
			return ERR_REFER_A_FRIEND_SUMMON_LEVEL_MAX_I;

        if (MapEntry const* mEntry = sMapStore.LookupEntry(GetMapId()))
        if (mEntry->Expansion() > target->GetSession()->Expansion())
            return ERR_REFER_A_FRIEND_INSUF_EXPAN_LVL;
    }
    else
    {
        if (GetTeam() != target->GetTeam() && !sWorld.getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
            return ERR_REFER_A_FRIEND_DIFFERENT_FACTION;
        if (GetLevel() <= target->GetLevel())
            return ERR_REFER_A_FRIEND_TARGET_TOO_HIGH;
        if (!GetGrantableLevels())
            return ERR_REFER_A_FRIEND_INSUFFICIENT_GRANTABLE_LEVELS;
        if (GetDistance(target) > DEFAULT_VISIBILITY_DISTANCE || !target->IsVisibleGloballyfor(this))
            return ERR_REFER_A_FRIEND_TOO_FAR;
        if (target->GetLevel() >= sWorld.getConfig(CONFIG_UINT32_RAF_MAXGRANTLEVEL))
            return ERR_REFER_A_FRIEND_GRANT_LEVEL_MAX_I;
        if (target->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_X1) || target->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_FROZEN))
            return ERR_REFER_A_FRIEND_NOT_NOW;
    }

    return ERR_REFER_A_FRIEND_NONE;
}

void Player::ChangeGrantableLevels(uint8 increase)
{
    if (increase)
    {
        if (m_GrantableLevelsCount <= uint32(sWorld.getConfig(CONFIG_UINT32_RAF_MAXGRANTLEVEL) * sWorld.getConfig(CONFIG_FLOAT_RATE_RAF_LEVELPERLEVEL)))
            m_GrantableLevelsCount += increase;
    }
    else
    {
        m_GrantableLevelsCount -= 1;

        if (m_GrantableLevelsCount < 0)
            m_GrantableLevelsCount = 0;
    }

    SetCanGrantLevelsFlagIfNeeded();
}

void Player::SetCanGrantLevelsFlagIfNeeded()
{
    if (m_GrantableLevelsCount > 0)
    {
        if (!CanGrantLevels())
            SetByteFlag(PLAYER_FIELD_BYTES, 1, 0x01);
    }
    else
    {
        if (CanGrantLevels())
            RemoveByteFlag(PLAYER_FIELD_BYTES, 1, 0x01);
    }
}

bool Player::CanGrantLevels()
{
    return HasByteFlag(PLAYER_FIELD_BYTES, 1, 0x01);
}

bool Player::CheckRAFConditions_XP()
{
    if (Group * grp = GetGroup())
    {
        for (GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* member = itr->getSource();

            if (!member || !member->isAlive())
                continue;

            if (GetObjectGuid() == member->GetObjectGuid())
                continue;

            if (!GetSession()->isRAFConnectedWith(member->GetSession()))
                continue;

            if (GetDistance(member) < 100 && abs((int32)GetLevel() - (int32)member->GetLevel()) < 10)
                return true;
        }
    }

    return false;
}

/*AccountLinkedState Player::GetAccountLinkedState()
{

    if (!m_referredAccounts.empty() && !m_referalAccounts.empty())
        return STATE_DUAL;

    if (!m_referredAccounts.empty())
        return STATE_REFER;

    if (!m_referalAccounts.empty())
        return STATE_REFERRAL;

    return STATE_NOT_LINKED;
}*/

/*void Player::LoadAccountLinkedState()
{
    m_referredAccounts.clear();
    m_referredAccounts = AccountMgr::GetRAFAccounts(GetSession()->GetAccountId(), true);

    if (m_referredAccounts.size() > sWorld.getConfig(CONFIG_UINT32_RAF_MAXREFERERS))
        sLog.outLog(LOG_DEFAULT, "Player:RAF:Warning: loaded %llu referred accounts instead of %u for player %u", m_referredAccounts.size(), sWorld.getConfig(CONFIG_UINT32_RAF_MAXREFERERS), GetObjectGuid().GetCounter());
    else
        debug_log("Player:RAF: loaded %u referred accounts for player %u", m_referredAccounts.size(), GetObjectGuid().GetCounter());

    m_referalAccounts.clear();
    m_referalAccounts = AccountMgr::GetRAFAccounts(GetSession()->GetAccountId(), false);

    if (m_referalAccounts.size() > sWorld.getConfig(CONFIG_UINT32_RAF_MAXREFERALS))
        sLog.outLog(LOG_DEFAULT, "Player:RAF:Warning: loaded %llu referal accounts instead of %u for player %u", m_referalAccounts.size(), sWorld.getConfig(CONFIG_UINT32_RAF_MAXREFERALS), GetObjectGuid().GetCounter());
    else
        debug_log("Player:RAF: loaded %u referal accounts for player %u", m_referalAccounts.size(), GetObjectGuid().GetCounter());
}*/

/*bool Player::IsReferAFriendLinked(Player* target)
{
    // check link this(refer) - target(referral)
    for (std::vector<uint32>::const_iterator itr = m_referalAccounts.begin(); itr != m_referalAccounts.end(); ++itr)
    {
        if ((*itr) == target->GetSession()->GetAccountId())
            return true;
    }

    // check link target(refer) - this(referral)
    for (std::vector<uint32>::const_iterator itr = m_referredAccounts.begin(); itr != m_referredAccounts.end(); ++itr)
    {
        if ((*itr) == target->GetSession()->GetAccountId())
            return true;
    }

    return false;
}*/

void Player::ChangeRace(uint8 new_race)
{
    sLog.outLog(LOG_RACE_CHANGE,"[%u] Starting race change for player %s from %u to %u",GetGUIDLow(),GetName(),GetRace(),new_race);
    Races old_race = Races(GetRace());

    /*if (bool((1 << new_race) & 0x89A) != bool((1 << old_race) & 0x89A))
    {
        sLog.outLog(LOG_RACE_CHANGE,"[%u] Invalid race change, trans-faction NYI",GetGUIDLow());
        return;
    }*/

    const PlayerInfo* new_info = sObjectMgr.GetPlayerInfo(new_race,GetClass());
    if (!new_info)
    {
        sLog.outLog(LOG_RACE_CHANGE,"[%u] Invalid race/class pair: %u / %u",GetGUIDLow(),new_race,GetClass());
        return;
    }

    QueryResultAutoPtr result = GameDataDatabase.PQuery(
        "SELECT skin1,skin2,skin3 FROM race_change_skins WHERE race = %u AND gender = %u",new_race,GetGender());

    if (!result)
    {
        sLog.outLog(LOG_RACE_CHANGE,"[%u] skins not found (race %u gender %u)",GetGUIDLow(),new_race,GetGender());
        return;
    }
    SetUInt32Value(PLAYER_BYTES,result->Fetch()[urand(0,2)].GetUInt32()); //face, hair, skin and hair color
    SetByteValue(PLAYER_BYTES_2,0,0); //facial hair

    if (GetGender() == GENDER_FEMALE)
    {
        SetDisplayId(new_info->displayId_f);
        SetNativeDisplayId(new_info->displayId_f);
    }
    else
    {
        SetDisplayId(new_info->displayId_m);
        SetNativeDisplayId(new_info->displayId_m);
    }
    uint32 unitbytes0 = GetUInt32Value(UNIT_FIELD_BYTES_0) & 0xFFFFFF00;
    unitbytes0 |= new_race;
    SetUInt32Value(UNIT_FIELD_BYTES_0, unitbytes0);

    //spells
    result = GameDataDatabase.PQuery("SELECT spell FROM race_change_spells WHERE race = %u AND class = %u",old_race,GetClass());
    Field* fields;
    if (result)
    {
        do
        {
            fields = result->Fetch();
            removeSpell(fields[0].GetUInt32());
        }
        while (result->NextRow());
    }

    PlayerTeam old_team = m_team;
    PlayerTeam new_team = TeamForRace(new_race);

    if (old_team != new_team)
    {
        uint32 curIdx = new_team == HORDE ? 0 : 1;
        uint32 swapTo = new_team == HORDE ? 1 : 0;
        bool has75riding = HasSpell(33388);
        bool has150riding = HasSpell(33391);
        bool has225riding = HasSpell(34090);
        bool has300riding = HasSpell(34091);

        result = GameDataDatabase.Query("SELECT spell_a, spell_h FROM race_change_swap_spells");
        if (result)
        {
            do
            {
                fields = result->Fetch();
                uint32 curSpell = fields[curIdx].GetUInt32();
                uint32 swapSpell = fields[swapTo].GetUInt32();

                if (HasSpell(curSpell))
                {
                    removeSpell(curSpell);
                    learnSpell(swapSpell);
                }                

            } while (result->NextRow());

            // re-learn riding after swapping land mount class spells
            if (has75riding)
                learnSpell(33388);

            if (has150riding)
                learnSpell(33391);

            if (has225riding)
                learnSpell(34090);

            if (has300riding)
                learnSpell(34091);
        }
    }

    result = GameDataDatabase.PQuery("SELECT spell_source, spell_target FROM race_change_swap_class_spells WHERE race_target = %u", new_race);
    if (result)
    {
        do
        {
            fields = result->Fetch();
            uint32 curSpell = fields[0].GetUInt32();
            uint32 swapSpell = fields[1].GetUInt32();

            if (HasSpell(curSpell))
            {
                removeSpell(curSpell);
                learnSpell(swapSpell);
            }

        } while (result->NextRow());
    }

    result = GameDataDatabase.PQuery("SELECT spell FROM race_change_spells WHERE race = %u AND class = %u",new_race,GetClass());
    if (result)
    {
        do
        {
            fields = result->Fetch();
            learnSpell(fields[0].GetUInt32());
        }
        while (result->NextRow());
    }

    //reps
    m_team = new_team;
    setFactionForRace(new_race);

    if (old_team != new_team && GetLevel() >= 60)
        AddPlayerCustomFlag(PL_CUSTOM_TAXICHEAT);

    sLog.outLog(LOG_RACE_CHANGE,"[%u] Race change for player %s succesful",GetGUIDLow(),GetName());
}

void Player::ChangeRaceSwapItems(uint8 new_race)
{
    static uint16 MountsForRace[12][7] = {
        {0,0,0,0,0,0,0},
        {2411,2414,5655,5656,18776,18777,18778},
        {1132,5665,5668,1132,18796,18797,18798},
        {5864,5872,5873,5864,18785,18786,18787},
        {8629,8631,8632,8629,18766,18767,18902},
        {13331,13332,13333,13331,13334,18791,13334},
        {15277,15290,15277,15290,18793,18794,18795},
        {8595,8563,13321,13322,18772,18773,18774},
        {8588,8591,8592,8588,18788,18789,18790},
        {0,0,0,0,0,0,0},
        {28927,29220,29221,29222,29223,29224,28936},
        {28481,29743,29744,28481,29745,29746,29747}
    };

    uint8 old_race = GetRace();

    //Mounts
    for (uint8 type = 0; type < 7; type++) {
        for (uint16 i = INVENTORY_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
        {
            Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem->GetEntry() == MountsForRace[old_race][type])
            {
                DestroyItem(INVENTORY_SLOT_BAG_0, i, true, "RACE_SWAP_DESTROY");
                SendItemByMail(MountsForRace[new_race][type]);
            }
        }
        for (uint16 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pBag)
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    Item* pItem = pBag->GetItemByPos(j);
                    if (pItem &&  pItem->GetEntry() == MountsForRace[old_race][type])
                    {
                        DestroyItem(i, j, true, "RACE_SWAP_DESTROY");
                        SendItemByMail(MountsForRace[new_race][type]);
                    }
                }
            }
        }
        for (uint16 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pBag)
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                {
                    Item* pItem = pBag->GetItemByPos(j);
                    if (pItem &&  pItem->GetEntry() == MountsForRace[old_race][type])
                    {
                        DestroyItem(i, j, true, "RACE_SWAP_DESTROY");
                        SendItemByMail(MountsForRace[new_race][type]);
                    }
                }
            }
        }
    }

    uint32 old_team = m_team;
    uint32 new_team = TeamForRace(new_race);

    if (old_team != new_team)
    {
        #define ITEM_PAIR_CNT 45
        // PvP Trinkets, Tabards, weapons and armor swap
        static const uint32 itemIds[ITEM_PAIR_CNT][2] =
        {
            {24551, 25829},
            {18834, 18854},
            {18845, 29593},
            {18846, 18856},
            {18849, 18857},
            {18850, 18859},
            {18851, 18862},
            {18852, 18858},
            {18853, 18863},
            {29592, 18864},
            {28239, 28238},
            {28240, 28234},
            {28241, 28235},
            {28242, 28236},
            {28243, 28237},
            {30343, 30348},
            {30344, 30350},
            {30345, 30351},
            {30346, 30349},
            {18607, 18606},
            {19046, 19045},
            //{20131, 20132},
            {24004, 23999},
            {15199, 15198},
            {31773, 31774},
            {15197, 15196},
            {19505, 19506},
            {19031, 19032},
            {29155, 29153},
            {29147, 29148},
            {29145, 29146},
            {30568, 30599},
            {29139, 29140},
            {29135, 29136},
            {28926, 28952},
            {28929, 28954},
            {28930, 28955},
            {30570, 30597},
            {29167, 29166},
            {29141, 29142},
            {29168, 29169},
            {28378, 28379},
            {28377, 28380},
            {29143, 29144},
            {30637, 30622},
			{37865, 37864},
        };

        for (uint32 k = 0; k < ITEM_PAIR_CNT; ++k)
        {
            // in inventory bags
            for (uint16 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
            {
                Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pBag)
                {
                    for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                    {
                        Item* pItem = pBag->GetItemByPos(j);
                        if (pItem && (pItem->GetEntry() == itemIds[k][0] || pItem->GetEntry() == itemIds[k][1]))
                        {
                            bool idxNew = pItem->GetEntry() == itemIds[k][0];
                            DestroyItem(i, j, true, "RACE_SWAP_DESTROY");
                            SendItemByMail(itemIds[k][idxNew]);
                        }
                    }
                }
            }

            // in bank bags
            for (uint16 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; i++)
            {
                Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pBag)
                {
                    for (uint32 j = 0; j < pBag->GetBagSize(); j++)
                    {
                        Item* pItem = pBag->GetItemByPos(j);
                        if (pItem && (pItem->GetEntry() == itemIds[k][0] || pItem->GetEntry() == itemIds[k][1]))
                        {
                            bool idxNew = pItem->GetEntry() == itemIds[k][0];
                            DestroyItem(i, j, true, "RACE_SWAP_DESTROY");
                            SendItemByMail(itemIds[k][idxNew]);
                        }
                    }
                }
            }

            // keys
            for (uint16 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; i++)
            {
                Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pItem && (pItem->GetEntry() == itemIds[k][0] || pItem->GetEntry() == itemIds[k][1]))
                {
                    bool idxNew = pItem->GetEntry() == itemIds[k][0];
                    DestroyItem(INVENTORY_SLOT_BAG_0, i, true, "RACE_SWAP_DESTROY");
                    SendItemByMail(itemIds[k][idxNew]);
                }
            }

            // equipped (except bags)
            for (uint16 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
            {
                Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pItem && (pItem->GetEntry() == itemIds[k][0] || pItem->GetEntry() == itemIds[k][1]))
                {
                    bool idxNew = pItem->GetEntry() == itemIds[k][0];
                    DestroyItem(INVENTORY_SLOT_BAG_0, i, true, "RACE_SWAP_DESTROY");
                    SendItemByMail(itemIds[k][idxNew]);
                }
            }

            // in inventory and bank
            for (uint16 i = INVENTORY_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
            {
                Item *pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pItem && (pItem->GetEntry() == itemIds[k][0] || pItem->GetEntry() == itemIds[k][1]))
                {
                    bool idxNew = pItem->GetEntry() == itemIds[k][0];
                    DestroyItem(INVENTORY_SLOT_BAG_0, i, true, "RACE_SWAP_DESTROY");
                    SendItemByMail(itemIds[k][idxNew]);
                }
            }
        }
    }
}

void Player::SendItemByMail(uint32 itemId)
{
    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(itemId);
    if (!proto)
        return;

    if (Item* item = Item::CreateItem(itemId, 1, this))
    {
        item->SaveToDB();

        int loc_idx = GetSession()->GetSessionDbLocaleIndex();
        // subject: item name
        std::string subject = proto->Name1;
        sObjectMgr.GetItemLocaleStrings(proto->ItemId, loc_idx, &subject);

        // no text
        uint32 itemTextId = sObjectMgr.CreateItemText("");

        MailDraft(subject, itemTextId)
            .AddItem(item)
            .SendMailTo(this, MailSender(MAIL_NORMAL, (uint32)0, MAIL_STATIONERY_GM));
    }
}

void Player::SetAcceptWhispers(bool on, bool init)
{
    if (on)
    {
        m_ExtraFlags |= PLAYER_EXTRA_ACCEPT_WHISPERS;

        if (!init && HasAura(45350, 0))
            RemoveAurasDueToSpell(45350);
    }
    else
    {
        m_ExtraFlags &= ~PLAYER_EXTRA_ACCEPT_WHISPERS;

        setPartialWhispers(false);
        sSocialMgr.clearAllowedWhisperers(GetGUIDLow());

        if (!init)
            AddAura(45350, this); // there is aura when whispers are open -> whispers shouldn't be open! That's kind of a signal for it
    }
}

bool Player::isInSanctuary()
{
    return HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_SANCTUARY);
}

void Player::SetSpectator(bool bSpectator)
{
    if (bSpectator)
    {
        if (IsSpectator())
        {
            return;
        }
        AddAura(54869, this); //AddFakeArenaQueue activator (1 sec delay - it must be called when player IS ALREADY in arena) - triggers 54824
        AddAura(55194, this); // Pacify and silence    
        SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
    }
    else
    {
        RemoveFakeArenaQueue(this);
        RemoveAurasDueToSpell(54824); // Fly and morph
        RemoveAurasDueToSpell(55194); // Pacify and silence
        SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
    }

    m_spectator = bSpectator;
}

void Player::InterruptSpellsOnMove()
{
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        if(Spell* spell = GetCurrentSpell(CurrentSpellTypes(i)))
            spell->InterruptSpellOnMove(true);
}

void Player::AddFakeArenaQueue(Player *Plr, uint32 MapId) // This function IS and MUST be called only if there IS one empty slot for queue.
{
    WorldPacket data;
    data.Initialize(SMSG_BATTLEFIELD_STATUS, (4+1+1+4+2+4+1+4+4+4));
    BattleGroundQueueTypeId bgTypeId1 = Plr->GetBattleGroundQueueTypeId(0);
    BattleGroundQueueTypeId bgTypeId2 = Plr->GetBattleGroundQueueTypeId(1);
    BattleGroundQueueTypeId bgTypeId3 = Plr->GetBattleGroundQueueTypeId(2);
    if (!bgTypeId1)
        data << uint32(0);
    else if (!bgTypeId2)
        data << uint32(1);
    else if (!bgTypeId3)
        data << uint32(2);
    else
    {
        sLog.outLog(LOG_DEFAULT, "ERROR: Somehow in call of AddFakeArenaQueue player had all three types of queue active - returned from adding fake.");
        return;
    }
    data << uint64(uint64(ARENA_TYPE_5v5) | (uint64(0x0D) << 8) | (uint64(ARENA_TYPE_5v5) << 16) | (uint64(0x1F90) << 48));
    data << uint32(0);                                     // unknown
    data << uint8(0);
    data << uint32(STATUS_IN_PROGRESS);                              // status
    data << uint32(MapId);                // map id
    data << uint32(0);                         // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
    data << uint32(0);                         // time from bg start, milliseconds
    data << uint8(0x1);                            // Lua_GetBattlefieldArenaFaction (bool)
    Plr->SendPacketToSelf(&data);
}

void Player::RemoveFakeArenaQueue(Player *Plr) // Call of this function can't break anything - it just sets no queue if there's really no queue at the server.
{
    BattleGroundQueueTypeId bgTypeId = BATTLEGROUND_QUEUE_NONE;
    for (uint8 i = 0; i < 3; i++)
    {
        BattleGroundQueueTypeId bgTypeId = Plr->GetBattleGroundQueueTypeId(i);
        if (!bgTypeId)
        {
            WorldPacket data;
            data.Initialize(SMSG_BATTLEFIELD_STATUS, 4*3);
            data << uint32(i);                         // queue id (0...2)
            data << uint64(0);
            Plr->SendPacketToSelf(&data);
        }
    }
}

ItemPrototype const* Player::GetProtoFromScriptsByEntry(uint32 entry)
{
    return sItemStorage.LookupEntry<ItemPrototype>(entry);
}

const MapManager::MapMapType& Player::GetMapsFromScripts() 
{ 
    return sMapMgr.Maps(); 
}

Map* Player::FindMapFromScripts(uint32 mapid, uint32 instanceId)
{
    return sMapMgr.FindMap(mapid, instanceId); 
}

void Player::CheckLevel5Reinforcement(bool RemoveForSure)
{
    return;

    uint8 HowMuch5LevelHas = 0;
    if (!RemoveForSure)
    {
        for (uint8 slot = 0; slot < 10; slot++)
        {
            if (slot == 3 || slot == 1)
                continue;
            Item *pItem = GetItemByPos(255, slot);
            if (pItem)
            {
                if (ItemPrototype const* proto = pItem->GetProto())
                {
                    if (proto->Quality == 5 && (proto->ItemId >= 903180 || proto->ItemId == 339111))
                    {
                        if (uint16 EnchantId = pItem->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                        {
                            bool Increased = false;
                            for (uint8 i = 0; i < 10; i++)
                            {
                                if (EnchantId == ReinforcementIDs[i][4])
                                {
                                    HowMuch5LevelHas++;
                                    Increased = true;
                                    break;
                                }
                            }
                            if (!Increased)
                                break;
                        }
                        else
                        {
                            break;
                        }
                    }
                    else
                        break;
                }
                else
                    break;
            }
            else
                break;
        }
    }
    if (HowMuch5LevelHas == 8)
    {
        if (HasAura(LEVEL_5_REINFORCE_VISUAL))
            return;
        CastSpell(this, LEVEL_5_REINFORCE_VISUAL, true);
    }
    else
    {
        if (!HasAura(LEVEL_5_REINFORCE_VISUAL))
            return;
        RemoveAurasDueToSpell(LEVEL_5_REINFORCE_VISUAL);
    }
}

//bool Player::CreateDuelArenateam()
//{
//    uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_1v1);
//    if (slot >= MAX_ARENA_SLOT)
//        return false;
//
//    // Check if player is already in an arena team
//    if (GetArenaTeamId(slot))
//    {
//        GetSession()->SendArenaTeamCommandResult(ERR_ARENA_TEAM_CREATE_S, GetName(), "", ERR_ALREADY_IN_ARENA_TEAM);
//        return false;
//    }
//
//    // Teamname = playername
//    // if teamname exist, we have to choose another name (playername + number)
//    uint32 i = 1;
//    std::stringstream teamName;
//    teamName << GetName();
//    do
//    {
//        if(sObjectMgr.GetArenaTeamByName(teamName.str())) // teamname exist, so choose another name
//        {
//            teamName.str(std::string());
//            teamName << GetName() << i;
//            i++;
//        }
//        else
//            break;
//    } while (i < 100); // should never happen
//
//    // Create arena team
//    ArenaTeam* at = new ArenaTeam();
//    if (!at->Create(GetGUID(), ARENA_TEAM_1v1, teamName.str()))
//    {
//        delete at;
//        return false;
//    }
//    at->SetEmblem(4283124816, 45, 4294242303, 5, 4294705149);
//    // Register arena team
//    sObjectMgr.AddArenaTeam(at); // Captain should be added in this function
//    ChatHandler(this).SendSysMessage("1v1 Arena Team successfuly created!");
//
//    return true;
//}

//bool Player::Send1v1ArenaTeamStats()
//{
//    ArenaTeam* at = sObjectMgr.GetArenaTeamById(GetArenaTeamId(3/*1v1 slot is 3*/));
//    if(at)
//    {
//        std::stringstream s;
//        s << "Rating: " << at->GetStats().rating;
//        s << "\nRank: " << at->GetStats().rank;
//        s << "\nSeason Games: " << at->GetStats().games_season;
//        s << "\nSeason Wins: " << at->GetStats().wins_season;
//        s << "\nToday Games: " << at->GetStats().games_week;
//        s << "\nToday Wins: " << at->GetStats().wins_week;
//        ChatHandler(this).SendSysMessage(s.str().c_str());
//        return true;
//    }
//    return false;
//}

const char* Player::GetItemNameLocale(ItemPrototype const* itemProto, std::string* name)
{
    *name = itemProto->Name1;
    sObjectMgr.GetItemLocaleStrings(itemProto->ItemId, GetSession()->GetSessionDbLocaleIndex(), name);
    return (*name).c_str();
}

bool Player::LearnAllSpells(bool multiRank, bool for_pet)
{
    uint32 trainer_entry;

    bool any_learned = false;

    if (for_pet)
        trainer_entry = 3352;
    else
    {
        switch (GetClass())
        {
        case CLASS_WARRIOR:
            trainer_entry = 939007;
            break;
        case CLASS_HUNTER:
            trainer_entry = 939005;
            break;
        case CLASS_ROGUE:
            trainer_entry = 939009;
            break;
        case CLASS_SHAMAN:
            trainer_entry = 939006;
            break;
        case CLASS_PALADIN:
            trainer_entry = 939000;
            break;
        case CLASS_DRUID:
            trainer_entry = 939010;
            break;
        case CLASS_MAGE:
            trainer_entry = 939008;
            break;
        case CLASS_WARLOCK:
            trainer_entry = 939003;
            break;
        case CLASS_PRIEST:
            trainer_entry = 939001;
            break;
        default:
            trainer_entry = 0;
            break;
        }
    }

    for (uint8 i = 0; (i < 1 || i < 2 && GetClass() == CLASS_HUNTER); i++)
    {
        if (i == 1)
            trainer_entry = 939004;
        if (trainer_entry != 0)
        {
            if (TrainerSpellData const* trainer_spells = sObjectMgr.GetNpcTrainerSpells(trainer_entry))
            {
                for (uint8 j = 0; j < (multiRank ? 10 : 1); ++j)
                {
                    for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
                    {
                        TrainerSpell const* tSpell = &itr->second;
                        if (GetTrainerSpellState(tSpell) != TRAINER_SPELL_GREEN)
                            continue;
                        WorldPacket data2(SMSG_PLAY_SPELL_VISUAL, 12); // visual effect on player
                        data2 << uint64(GetGUID()) << uint32(0x016A);
                        BroadcastPacket(&data2, true);
                        // learn explicitly to prevent lost money at lags, learning spell will be only show spell animation
                        learnSpell(tSpell->spell);

                        if (!any_learned)
                            any_learned = true;
                    }
                }
            }
        }
    }

    return any_learned;
}

void Player::restorePowers()
{
    SetHealth(GetMaxHealth());
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));

    switch (getPowerType())
    {
        case POWER_RAGE: 
            SetPower(POWER_RAGE, 0);
            break;
        case POWER_ENERGY:
            SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
            break;
        default:
            break;
    }
}

void Player::SavePlayerCustomFlags(uint32 CharGuid, uint16 flags)
{
    static SqlStatementID savePlayerCustomFlags;
    SqlStatement stmt = RealmDataDatabase.CreateStatement(savePlayerCustomFlags, "UPDATE characters SET char_custom_flags = ? WHERE guid = ?");
    stmt.PExecute(flags, CharGuid);
}

void Player::SavePlayerCustomFlags()
{
    SavePlayerCustomFlags(GetGUIDLow(), m_ChCustomFlags);
}

void Player::AddPlayerCustomFlag(PlayerCustomFlags flag)
{
    m_ChCustomFlags |= flag;
    SavePlayerCustomFlags();
}

void Player::RemovePlayerCustomFlag(PlayerCustomFlags flag)
{
    m_ChCustomFlags &= ~flag;
    SavePlayerCustomFlags();
}

std::string Player::getTicketTextByItemGUID(uint32 itemGUID)
{
    GM_Ticket* ticket = sTicketMgr.GetClosedTicketByItemGUID(itemGUID);
    return ticket ? ticket->message : "";
}

bool Player::setTicketApprovedByItemGUID(uint32 itemGUID)
{
    GM_Ticket* ticket = sTicketMgr.GetClosedTicketByItemGUID(itemGUID);
    if (ticket)
    {
        ticket->approved = true;
        sTicketMgr.SaveGMTicket(ticket);
        return true;
    }
    else
        return false;
}

void Player::SendAddonMessage(std::string& text, const char* prefix)
{
    std::string message;
    message.append(prefix);
    message.push_back(9);
    message.append(text);

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    data << uint8(CHAT_MSG_WHISPER);
    data << uint32(LANG_ADDON);
    data << uint64(0); // guid
    data << uint32(LANG_ADDON);                               //language 2.1.0 ?
    data << uint64(0); // guid
    data << uint32(message.length() + 1);
    data << message;
    data << uint8(0);

    BroadcastPacketInRange(&data, GetMap()->GetVisibilityDistance(), false, false);
}

void Player::SendGladdyNotification()
{
    std::stringstream sstream;
    sstream << "0x" << std::setfill('0') << std::setw(sizeof(uint64) * 2) << std::hex << std::uppercase << GetGUID();
    std::string result = sstream.str();
    SendAddonMessage(result, "GladdyTrinketUsed");
}

void Player::LogGroupLootAction(uint8 type, uint32 item_entry)
{
	Group *group = this->GetGroup();
	uint32 instId = GetInstanciableInstanceId();
	char instIdChr[32];
	sprintf(instIdChr, "#%u ", instId);

	std::string log_str = instIdChr;

	log_str.append("[");
	log_str.append(GetName());

	log_str.append("]->LOOT_GROUP_VOTE: ");

	log_str.append("Item ");
	log_str.append(std::to_string(item_entry));
	log_str.append(" ");

	if (type == 128)
		log_str.append("(pass)");
	else if (type == 0)
		log_str.append("(need)");
	else if (type == 2)
		log_str.append("(greed)");
	else
		log_str.append("(unknow)");

	log_str.append(" (class: ");
	log_str.append(PrintNameAndClassInitials(0, GetClass(), ""));
	log_str.append(", ");

	log_str.append("spec: ");
	log_str.append(PrintTalentCount(GetActiveSpec()));
	log_str.append(")");

	sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
}

void Player::LogRollWon(uint64 winner_guid, uint32 item_entry, uint8 type)
{
	std::string player_name;

	if (winner_guid)
		sObjectMgr.GetPlayerNameByGUID(winner_guid, player_name);
	else
		player_name = GetName();

	if (player_name.empty())
		player_name = "UNKNOWN";

	Group *group = GetGroup();
	uint32 instId = GetInstanciableInstanceId();
	char instIdChr[32];
	sprintf(instIdChr, "#%u ", instId);

	std::string log_str = instIdChr;

	log_str.append("[");
	log_str.append(GetName());

	log_str.append("]->ROLL_GROUP_WON:");

	log_str.append(" Player ");
	log_str.append(player_name.c_str());

	if (winner_guid)
	{
		log_str.append("(guid ");
		log_str.append(std::to_string(winner_guid));
		log_str.append(") ");
		log_str.append("UNABLE to receive");
	}
	else
	{
		log_str.append("(guid ");
		log_str.append(std::to_string(GetGUID()));
		log_str.append(") ");
		log_str.append("received");
	}

	log_str.append(" Item ");
	log_str.append(std::to_string(item_entry));
	log_str.append(" ");

	if (type == 1)
		log_str.append("(need)");
	else
		log_str.append("(greed)");

	log_str.append(" (class: ");
	log_str.append(PrintNameAndClassInitials(0, GetClass(), ""));
	log_str.append(", ");

	log_str.append("spec: ");
	log_str.append(PrintTalentCount(GetActiveSpec()));
	log_str.append(")");

	sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
}

void Player::LogRoll(uint32 min, uint32 max, uint32 roll, bool master_loot, uint32 item_entry)
{
	Group *group = GetGroup();
    uint32 instId = GetInstanciableInstanceId();
    char instIdChr[32];
    sprintf(instIdChr, "#%u ", instId);

    std::string log_str = instIdChr;

    log_str.append("[");
    log_str.append(GetName());

	if(master_loot)
		log_str.append("]->ROLL:");
	else
		log_str.append("]->ROLL_GROUP: ");

    //if (!group)
    //{
    //    log_str.append("[unknown raid] ");
    //}
    //else
    //{
    //    // obtain group information
    //    log_str.append("[");

    //    uint8 gm_count = group->GetMembersCount();
    //    uint8 gm_count_m1 = gm_count - 1;
    //    uint64 gm_leader_GUID = group->GetLeaderGUID();
    //    Player *gm_member;

    //    gm_member = sObjectMgr.GetPlayerInWorld(gm_leader_GUID);
    //    if (gm_member)
    //    {
    //        log_str.append(gm_member->GetName());
    //        log_str.append(",");
    //    }

    //    Group::MemberSlotList g_members = group->GetMemberSlots();

    //    for (Group::member_citerator itr = g_members.begin(); itr != g_members.end(); itr++)
    //    {
    //        if (itr->guid == gm_leader_GUID) continue;

    //        gm_member = sObjectMgr.GetPlayerInWorld(itr->guid);
    //        if (gm_member)
    //        {
    //            log_str.append(itr->name);
    //            log_str.append(",");
    //        }
    //    }

    //    log_str.erase(log_str.length() - 1);
    //    log_str.append("] ");
    //}

	if (master_loot)
	{
		char out[30];
		sprintf(out, "%u-%u = %u ", min, max, roll);
		log_str.append(out);
	}
	else
	{
		char out[30];
		sprintf(out, "item %u - %u ", item_entry, roll);
		log_str.append(out);
	}

	log_str.append("(class: ");
	log_str.append(PrintNameAndClassInitials(0,GetClass(),""));
	log_str.append(", ");

    log_str.append("spec: ");
    log_str.append(PrintTalentCount(GetActiveSpec()));
    log_str.append(")");

    sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
}

void Player::LogLootMethod(LootMethod method)
{
    Group *group = GetGroup();
    uint32 instId = GetInstanciableInstanceId();
    char instIdChr[32];
    sprintf(instIdChr, "#%u ", instId);

    std::string log_str = instIdChr;

    log_str.append("[");
    log_str.append(GetName());
    log_str.append("]->LOOT_METHOD: ");

    std::string method_name;
    switch (method)
    {
    case 0: method_name = "FREE_FOR_ALL";
        break;
    case 1: method_name = "ROUND_ROBIN";
        break;
    case 2: method_name = "MASTER_LOOT";
        break;
    case 3: method_name = "GROUP_LOOT";
        break;
    case 4: method_name = "NEED_BEFORE_GREED";
        break;
    default: method_name = "UNKNOWN";
    }

    log_str.append(method_name);

    sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
}

void Player::LogKicked(uint64 kicked_guid)
{
    Player* kicked = sObjectMgr.GetPlayerInWorld(kicked_guid); // might not exist (be offline)

    Group* group = GetGroup();
    uint32 instId = kicked ? kicked->GetInstanciableInstanceId() : GetInstanciableInstanceId();
    char instIdChr[32];
    sprintf(instIdChr, "#%u ", instId);

    std::string log_str = instIdChr;

    log_str.append("[");
    log_str.append(GetName());
    if (kicked)
        log_str.append("]->KICKED:");
    else
        log_str.append("]->KICKED_OFFLINE:");

    std::string plrName = "";
    if (!kicked)
        sObjectMgr.GetPlayerNameByGUID(kicked_guid, plrName);

    log_str.append("[");
    log_str.append(kicked ? kicked->GetName() : plrName);
    log_str.append("]: XYZM: ");

    if (kicked)
    {
        float x, y, z;
        uint32 mapId = kicked->GetMapId();
        kicked->GetPosition(x, y, z);
        char posChr[32];
        sprintf(posChr, "%.2f %.2f %.2f %u ", x, y, z, mapId);
        log_str.append(posChr);
    }
    else
        log_str.append("0 0 0 0 ");

    log_str.append("[Boss: ");
    std::string bossName = "NOT_FOUND";
    char bossId[16] = "0";
    char bposChr[32] = "0 0 0 0";
    char healthPct[8] = "0%%";
    if (kicked)
    {
        HostileReference* ref = kicked->getHostileRefManager().getFirst();
        while (ref)
        {
            if (Unit* pUnit = ref->getSource()->getOwner())
            {
                if (!pUnit || !pUnit->GetObjectGuid().IsCreature())
                {
                    ref = ref->next();
                    continue;
                }

                if (pUnit->ToCreature()->isWorldBoss())
                {
                    bossName = pUnit->GetName();
                    sprintf(bossId, "%u ", pUnit->GetEntry());
                    float x, y, z;
                    uint32 mapId = pUnit->GetMapId();
                    pUnit->GetPosition(x, y, z);
                    sprintf(bposChr, "%.2f %.2f %.2f %u ", x, y, z, mapId);
                    sprintf(healthPct, "%.2f%%", pUnit->GetHealthPercent());
                    break;
                }
            }
            ref = ref->next();
        }
    }

    log_str.append(bossName);
    log_str.append(" (ID: ");
    log_str.append(bossId);
    log_str.append(", health: ");
    log_str.append(healthPct);
    log_str.append(")]: XYZM: ");
    log_str.append(bposChr);

    sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
}

void Player::LogInstanceBound(uint32 instId, Group* group)
{
    if (!group) // there will be group in most cases. but sometimes there could be no group
        return;

    char instIdChr[32];
    sprintf(instIdChr, "#%u ", instId);

    std::string log_str = instIdChr;

    Map* m = GetMap();

    log_str.append("[");
    log_str.append(GetName());
    log_str.append("]->BOUND_TO_INSTANCE: Map: ");
    log_str.append(m->GetMapName());
    log_str.append(", Leader [");
    log_str.append(group->GetLeaderName());
    log_str.append("]");

    sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());

    //Player* leader = sObjectAccessor.GetPlayerInWorld(group->GetLeaderGUID());
    //// if player is leader -> send to leader and dont send to player
    //// if player is not leader -> send to both leader and player

    //if (group->SetBoundRules(instId)) // if true - rules just added -> need to message leader about it
    //{
    //    if (leader) 
    //        ChatHandler(leader).SendSysMessage(leader->GetSession()->GetHellgroundString(LANG_RAID_RULES_BOUND));
    //}

    //if (player != leader) // if this IS leader -> do not send
    //    ChatHandler(player).SendSysMessage(player->GetSession()->GetHellgroundString(LANG_RAID_RULES_BOUND));
}

void Player::LogGroupJoin()
{
	Group *group = GetGroup();
	uint32 instId = GetInstanciableInstanceId();
	char instIdChr[32];
	sprintf(instIdChr, "#%u ", instId);

	std::string log_str = instIdChr;

	log_str.append("[");
	log_str.append(GetName());
	log_str.append("]->GROUP_JOIN: ");

	if (!group)
	{
		log_str.append("[unknown raid] ");
	}
	else
	{
		// obtain group information
		log_str.append("[");

		uint8 gm_count = group->GetMembersCount();
		uint8 gm_count_m1 = gm_count - 1;
		uint64 gm_leader_GUID = group->GetLeaderGUID();
		Player *gm_member;

		gm_member = sObjectMgr.GetPlayerInWorld(gm_leader_GUID);
		if (gm_member)
		{
			log_str.append(gm_member->GetName());
			log_str.append(",");
		}

		Group::MemberSlotList g_members = group->GetMemberSlots();

		for (Group::member_citerator itr = g_members.begin(); itr != g_members.end(); itr++)
		{
			if (itr->guid == gm_leader_GUID) continue;

			gm_member = sObjectMgr.GetPlayerInWorld(itr->guid);
			if (gm_member)
			{
				log_str.append(itr->name);
				log_str.append(",");
			}
		}

		log_str.erase(log_str.length() - 1);
		log_str.append("] ");
	}

	sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
}

void Player::LogLootMasterGive(Player *receiver, uint32 item)
{
    Group *group = GetGroup();
    uint32 instId = GetInstanciableInstanceId();
    char instIdChr[32];
    sprintf(instIdChr, "#%u ", instId);

    std::string log_str = instIdChr;

    log_str.append("[");
    log_str.append(GetName());
    log_str.append("]->LOOT_MASTER_GIVE: ");

    log_str.append(std::to_string(item));

    log_str.append(" to ");

    log_str.append(receiver->GetName());

	log_str.append(" (class: ");
	log_str.append(receiver->PrintNameAndClassInitials(0, receiver->GetClass(), ""));
	log_str.append(", ");

	log_str.append("spec: ");
	log_str.append(receiver->PrintTalentCount(receiver->GetActiveSpec()));
	log_str.append(")");

    sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
}

//void Player::LogRaidRulesChange(Player *player, std::string* rules, bool changed)
//{
//    Group* group = player->GetGroup();
//    uint32 instId = player->GetInstanciableInstanceId();
//    char instIdChr[32];
//    sprintf(instIdChr, "#%u #%u ", group ? group->GetTempGroupGUID() : 0, instId);
//
//    std::string log_str = instIdChr;
//
//    log_str.append("[");
//    log_str.append(player->GetName());
//    log_str.append("]->RAID_RULES_CHANGE: ");
//    log_str.append(changed ? "AFTER CHANGE: " : "BEFORE CHANGE: ");
//    log_str.append(rules[0]);
//    if (rules[1] != "")
//    {
//        log_str.append(" !Second Part!: ");
//        log_str.append(rules[1]);
//    }
//
//    sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());
//}

void Player::LogLeaderChange(const std::string &oldName, const std::string &newName, Group* gr, Player* newLeader)
{
    uint32 instId;
    if (newLeader)
        instId = newLeader->GetInstanciableInstanceId();
    else
        instId = 0;

    char instIdChr[32];
    sprintf(instIdChr, "#%u ", instId);

    std::string log_str = instIdChr;

    log_str.append("[");
    log_str.append(oldName);
    log_str.append("]->LEADER_CHANGED_TO: [");
    log_str.append(newName);
    log_str.append("]");

    sLog.outLog(LOG_RAID_ACTIONS, log_str.c_str());

    //if (gr->RaidRulesExist())
    //{
    //    for (GroupReference *itr = gr->GetFirstMember(); itr != NULL; itr = itr->next())
    //    {
    //        Player *pl = itr->getSource();
    //        if (!pl)
    //            continue;

    //        if (WorldSession* sess = pl->GetSession())
    //            ChatHandler(sess).SendSysMessage(sess->GetHellgroundString(LANG_RAID_RULES_NEW_LEADER));
    //    }
    //}
}

void Player::_LoadArenaRestrictedSwaps(QueryResultAutoPtr result)
{
    if (result)
    {
        m_arena_restricted_swaps_size = result->GetRowCount();
        m_arena_restricted_swaps = new uint32[m_arena_restricted_swaps_size];
        uint32 idx = 0;

        do
        {
            Field *fields = result->Fetch();

            uint32 item_lowguid = fields[0].GetUInt32();

            m_arena_restricted_swaps[idx] = item_lowguid;
            ++idx;
        } while (result->NextRow());
    }
}

bool Player::ArenaRestrictedCanSwap(uint32 item_low_guid)
{
    for (uint32 i = 0; i < m_arena_restricted_swaps_size; ++i)
    {
        if (m_arena_restricted_swaps[i] == item_low_guid)
            return false;
    }
    return true;
}

void Player::ArenaRestrictedAddSwap(Item* old_item, Item* analog_item)
{
    // one will be added
    m_arena_restricted_swaps_size = m_arena_restricted_swaps_size + 1;

    uint32* new_arr = new uint32[m_arena_restricted_swaps_size];

    if (m_arena_restricted_swaps) // can be NULL
    {
        memcpy(new_arr, m_arena_restricted_swaps, sizeof(*m_arena_restricted_swaps)/*sizeof element*/*(m_arena_restricted_swaps_size - 1)/*elements count*/);
        delete[] m_arena_restricted_swaps;
    }

    m_arena_restricted_swaps = new_arr;

    m_arena_restricted_swaps[m_arena_restricted_swaps_size - 1] = old_item->GetGUIDLow();

    RealmDataDatabase.PExecute("INSERT INTO arena_restrictions_swapped (plr_guid, item_guid, item_entry, swap_guid, swap_entry) VALUES ('%u', '%u', '%u', '%u', '%u')",
        GetGUIDLow(), old_item->GetGUIDLow(), old_item->GetEntry(), analog_item->GetGUIDLow(), analog_item->GetEntry());
}

bool Player::IsInGurubashiEvent()
{
    return isGameEventActive(1011)/*Gurubashi arena event*/ && (GetCachedArea() == 1741/*Gurubashi arena*/ || GetCachedArea() == 2177/*Gurubashi arena*/);
}

void Player::LoadRaidChestInfo()
{
    QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT item_guid, chest_entry FROM character_raidchest WHERE removed=0 and owner_guid='%u'", GetGUIDLow());
    if (result)
    {
        do
        {
            Field *fields = result->Fetch();
            raidChestLoot.insert({ fields[0].GetUInt32(), fields[1].GetUInt32() });
        } while (result->NextRow());
    }
}

void Player::GivePremiumItemIfNeeded()
{
    if (WorldSession* s = GetSession())
    {
        uint32 item1 = PREMIUM_REMOTE;
        
        if (s->isPremium() && !HasItemCount(item1, 1, true))
        {
            // Give premium item
            if (ObjectMgr::GetItemPrototype(item1))
            {
                ItemPosCountVec dest;
                uint32 no_space_count = 0;
                uint32 thisCount = 1;
                uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item1, thisCount, &no_space_count);
                if (msg != EQUIP_ERR_OK)                       // convert to possible store amount
                    thisCount -= no_space_count;

                if (!dest.empty())                // can add some
                    if (Item* item = StoreNewItem(dest, item1, true, 0, "PREMIUM"))
                    {
                        SendNewItem(item, thisCount, true, false);
                        //AddEvent(new SendSysMessageEvent(*this, LANG_PREMIUM_ITEM_ADDED_SYSMSG), 3000);
                    }
            }
        }

        if (sWorld.isEasyRealm())
        {
            uint32 item2 = BOOK_OF_TELEPORTATION;

            if (s->isPremium() && !HasItemCount(item2, 1, true))
            {
                // Give premium item
                if (ObjectMgr::GetItemPrototype(item2))
                {
                    ItemPosCountVec dest;
                    uint32 no_space_count = 0;
                    uint32 thisCount = 1;
                    uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item2, thisCount, &no_space_count);
                    if (msg != EQUIP_ERR_OK)                       // convert to possible store amount
                        thisCount -= no_space_count;

                    if (!dest.empty())                // can add some
                        if (Item* item = StoreNewItem(dest, item2, true, 0, "PREMIUM"))
                        {
                            SendNewItem(item, thisCount, true, false);
                            AddEvent(new SendSysMessageEvent(*this, LANG_PREMIUM_ITEM_ADDED_SYSMSG), 4000);
                        }
                }
            }
        }
    }
}

void Player::ChangeSingleInstance(uint32 sInst)
{
    // TeleportTo START

    uint32 mapid = GetMapId();

    // preparing unsummon pet if lost (we must get pet before teleportation or will not find it later)
    Pet* pet = GetPet();

    // if we were on a transport, leave
    /*if (!(options & TELE_TO_NOT_LEAVE_TRANSPORT) && m_transport)
    {
        m_transport->RemovePassenger(this);
        m_transport = NULL;
        m_movementInfo.ClearTransportData();
    }*/ // Not removing from transports

    // The player was ported to another map and looses the duel immediatly.
    // We have to perform this check before the teleport, otherwise the
    // ObjectAccessor won't find the flag.
    if (duel)
    {
        GameObject* obj = GetMap()->GetGameObject(GetUInt64Value(PLAYER_DUEL_ARBITER));
        if (obj)
            DuelComplete(DUEL_FLED);
    }

    // reset movement flags at teleport, because player will continue move with these flags after teleport
    // m_movementInfo.SetMovementFlags(MOVEFLAG_NONE);
    // DisableSpline(); // Not needed here. Let player continue moving by spline. Map terrain is the same

    // far teleport to another map
    Map* oldmap = GetMap();

    CombatStop();

    // remove pet on map change
    if (pet)
    {
        //leaving map -> delete pet right away (doing this later will cause problems)
        if (pet->isControlled() && !pet->isTemporarySummoned())
            m_temporaryUnsummonedPetNumber = pet->GetCharmInfo()->GetPetNumber();
        else
            m_temporaryUnsummonedPetNumber = 0;

        RemovePet(pet, PET_SAVE_NOT_IN_SLOT);
    }

    // remove all dyn objects
    RemoveAllDynObjects();

    // stop spellcasting
    if (IsNonMeleeSpellCast(true))
        InterruptNonMeleeSpells(true);

    if (getFollowingGM())
    {
        setGMFollow(0);
    }
    else if (getFollowTarget())
    {
        setFollowTarget(0);
        GetMotionMaster()->Clear(true);
    }

    // remove from old map now
    oldmap->Remove(this, false);

    // new final coordinates
    float final_x = GetPositionX();
    float final_y = GetPositionY();
    float final_z = GetPositionZ();
    float final_o = GetOrientation();

    if (m_transport)
    {
        final_x += m_movementInfo.GetTransportPos()->x;
        final_y += m_movementInfo.GetTransportPos()->y;
        final_z += m_movementInfo.GetTransportPos()->z;
        final_o += m_movementInfo.GetTransportPos()->o;
    }

    SetFallInformation(0, final_z);

    // TeleportTo END

    //////////////////////////////////////////////////////////////////////////////
    
    // HandleMoveWorldportAckOpcode() start.

    Map *map = sMapMgr.FindMap(mapid, sInst);

    if (!map)
    {
        sLog.outLog(LOG_CRASH, "Could not find map on ChangeSingleInstance, map %u, xyz %f %f %f, instId %u", mapid, final_x, final_y, final_z, sInst);
        ASSERT(false);
        // Teleport to previous place, if cannot be ported back TP to homebind place
        // if (!TeleportTo(old_loc)) // cant teleport to previous loc, cause it's the same, and on teleport it will again try to change sInstance
            TeleportToHomebind();

        return;
    }

    Relocate(final_x, final_y, final_z, GetOrientation());

    SetMapId(mapid);
    SetMap(map);

    // since the MapId is set before the GetInstance call, the InstanceId must be set to 0
    // to let GetInstance() determine the proper InstanceId based on the player's binds
    SetInstanceId(map->GetAnyInstanceId());

    // SendInitialPacketsBeforeAddToMap(); // We only need to send packets which are needed on seemless teleport
    {
        UpdateZone(GetZoneId()); // update zone just in case, but it shouldn't be needed here
        SendInitWorldStates(); // this sends PvP state of the world (towers, etc.)

        // set fly flag if in fly form or taxi flight to prevent visually drop at ground in showup moment
        /*if (HasAuraType(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED) || IsTaxiFlying())
            AddUnitMovementFlag(MOVEFLAG_FLYING);*/ // We don't need to restore anything. Client already is flying and didn't lose any flags
    }

    if (!GetMap()->Add(this))
    {
        sLog.outLog(LOG_CRASH, "Could not add to a map on ChangeSingleInstance, map %u, xyz %f %f %f, instId %u", mapid, final_x, final_y, final_z, sInst);
        ASSERT(false);
        // Teleport to previous place, if cannot be ported back TP to homebind place
        // if (!TeleportTo(old_loc)) // cant teleport to previous loc, cause it's the same, and on teleport it will again try to change sInstance
            TeleportToHomebind();

        return;
    }

    //SendInitialPacketsAfterAddToMap(); // Should not be needed here, only needed after real load between maps. Only need UpdateVisibilityAndView() from it
    {
        UpdateVisibilityAndView();
    }

    // flight fast teleport case
    //if (GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
    //{
    //    // short preparations to continue flight
    //    FlightPathMovementGenerator* flight = (FlightPathMovementGenerator*)(GetMotionMaster()->top());
    //
    //    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0); // need to remove mount model before Reset, so m_previewDisplayId in the FlightPath is not set. Player will be remounted in Reset
    //
    //    // and finally - continue flight path
    //    flight->Reset(*this);
    //    return;
    //}

    // resummon pet
    if (m_temporaryUnsummonedPetNumber)
    {
        Pet* NewPet = new Pet;
        if (!NewPet->LoadPetFromDB(this, 0, m_temporaryUnsummonedPetNumber, true))
            delete NewPet;

        m_temporaryUnsummonedPetNumber = 0;
    }
}

void Player::LevelReached(uint32 newLevel)
{
	WorldSession* s = GetSession();
	if (!s)
		return;

	if (!sWorld.getConfig(CONFIG_IS_BETA) && newLevel >= 20)
	{
		// Give premium code for level 10 for non-raf players
		if (!s->IsRaf() && !s->IsAccountFlagged(ACC_FREE_PREMIUM_CODE_GIVEN))
		{
			// generate code
			uint32 code = urand(1000, 9999);

			// generate the expire time
			time_t expire = time(NULL) + DAY * 3;
			tm* aTm = localtime(&expire);
			char exp_chr[32];
			snprintf(exp_chr, 32, "%-4d-%02d-%02d %02d:%02d:%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);

			// save this code
			static SqlStatementID insertCode;
			SqlStatement stmt = AccountsDatabase.CreateStatement(insertCode, "INSERT INTO account_premium_codes VALUES (?, ?, ?)");
			stmt.PExecute(s->GetAccountId(), code, (uint64)expire);

			// subject
			std::string subject = s->GetHellgroundString(LANG_FREE_PREMIUM_CODE_MAIL_TITLE);

			// text
			std::string textFormat = s->GetHellgroundString(LANG_FREE_PREMIUM_CODE_MAIL_TEXT);
			char textBuf[512];
			snprintf(textBuf, 512, textFormat.c_str(), code, exp_chr);
			uint32 TextId = sObjectMgr.CreateItemText(textBuf);

			// send the mail
			MailDraft(subject, TextId)
				.SendMailTo(this, MailSender(MAIL_NORMAL, (uint32)0, MAIL_STATIONERY_GM));

			// send sys message that mail is sent (gotta wait 1 sec, cause otherwise chat is flooded with level-up info)
			AddEvent(new SendSysMessageEvent(*this, LANG_FREE_PREMIUM_CODE_SYSMSG), 1000);

			// set the 'given' flag. Flags are saved automaticly in the method
			s->AddAccountFlag(ACC_FREE_PREMIUM_CODE_GIVEN);
		}
	}

	// ----------- x100 
	if (sWorld.isEasyRealm())
	{
        // levelup npc
		if (newLevel == 3 || newLevel == 20 || newLevel == 50)
		{
            if (!InBattleGroundOrArena())
            {
                Position dest;
                GetValidPointInAngle(dest, 30.0f, frand(0.0f, 2 * M_PI), true);
                Creature* creature = SummonCreature(693131, dest.x, dest.y, dest.z, 0.f, TEMPSUMMON_TIMED_DESPAWN, 600 * MILLISECONDS);
                creature->GetMotionMaster()->MoveFollow(this, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
            }
		}
	}
	// ----------- x5
	else
	{
		// if this player has hardcore flag
		if (IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1) && newLevel == 70)
		{
			sWorld.SendWorldText(16552, 0, GetName());
		}

		if (ClassQuestAvailable(newLevel))
			ChatHandler(this).SendSysMessage(16570);
	}


    //// if this player is the first who reached lvl 70 in their class
    //bool* thisClassReached = &(sWorld.getMaxLvlReachedPerClass()[GetClass()]);
    //if (!(*thisClassReached))
    //{
    //    *thisClassReached = true;

    //    // Give some stuff
    //    if (WorldSession* s = GetSession())
    //    {
    //        ItemPrototype const* tabardProto = ObjectMgr::GetItemPrototype(695002);
    //        ItemPrototype const* rocketPetProto = ObjectMgr::GetItemPrototype(23770);
    //        Item* tabardItem = NULL;
    //        Item* rocketPetItem = NULL;

    //        if (tabardProto && rocketPetProto)
    //        {
    //            tabardItem = Item::CreateItem(695002, 1, this);
    //            rocketPetItem = Item::CreateItem(23770, 1, this);
    //        }

    //        if (tabardItem && rocketPetItem)
    //        {
    //            tabardItem->SaveToDB();
    //            rocketPetItem->SaveToDB();

    //            // subject
    //            std::string subject = s->GetHellgroundString(LANG_FIRST_LEVEL_70_CLASS_MAIL_TITLE);

    //            // text
    //            const char* textFormat = s->GetHellgroundString(LANG_FIRST_LEVEL_70_CLASS_MAIL_TEXT);
    //            uint32 TextId = sObjectMgr.CreateItemText(textFormat);

    //            // send the mail
    //            MailDraft(subject, TextId)
    //                .AddItem(tabardItem)
    //                .AddItem(rocketPetItem)
    //                .SetMoney(1000 * GOLD)
    //                .SendMailTo(this, MailSender(MAIL_NORMAL, (uint32)0, MAIL_STATIONERY_GM));

    //            // send sys message that mail is sent (gotta wait 1 sec, cause otherwise chat is flooded with level-up info)
    //            AddEvent(new SendSysMessageEvent(*this, LANG_FIRST_LEVEL_70_CLASS_SYSMSG), 1000);
    //        }
    //        else
    //            sLog.outLog(LOG_CHAR, "First-level-70 prize for player %u error: did not give", GetGUIDLow());
    //    }
    //    else
    //        sLog.outLog(LOG_CHAR, "First-level-70 prize for player %u error: did not give", GetGUIDLow());
    //}
    
    // don't needed huh
    //// give mini pet
    //if (newLevel == 20 && s->IsRaf())
    //{ 
    //    // send item LAMP_WITH_A_WISP
    //    ItemPrototype const* minipetProto = ObjectMgr::GetItemPrototype(LAMP_WITH_A_WISP);
    //    Item* minipettItem = NULL;
    //    if (minipetProto)
    //        minipettItem = Item::CreateItem(LAMP_WITH_A_WISP, 1, this);

    //    if (minipettItem)
    //    {
    //        minipettItem->SaveToDB();

    //        // subject
    //        std::string subject = s->GetHellgroundString(LANG_RAF_LVL_10_MINIPET_MAIL_TITLE);

    //        // text
    //        std::string textFormat = s->GetHellgroundString(LANG_RAF_LVL_10_MINIPET_MAIL_TEXT);
    //        uint32 TextId = sObjectMgr.CreateItemText(textFormat.c_str());

    //        // send the mail
    //        MailDraft(subject, TextId)
    //            .AddItem(minipettItem)
    //            .SendMailTo(this, MailSender(MAIL_NORMAL, (uint32)0, MAIL_STATIONERY_GM));

    //        AddEvent(new SendSysMessageEvent(*this, LANG_RAF_LVL_10_MINIPET_SYSMSG), 1000);
    //    }
    //    else
    //        sLog.outLog(LOG_CRITICAL, "Level-20 RAF minipet could not create item for player %u", GetGUIDLow());
    //}
}

std::vector<Item*> Player::GetItemListByEntry(uint32 entry, bool inBankAlso) const
{
    std::vector<Item*> itemList = std::vector<Item*>();

    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (item->GetEntry() == entry)
                itemList.push_back(item);

    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* bag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                if (Item* item = bag->GetItemByPos(j))
                    if (item->GetEntry() == entry)
                        itemList.push_back(item);

    for (uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (item->GetEntry() == entry)
                itemList.push_back(item);

    if (inBankAlso)
    {
        for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_BAG_END; ++i)
            if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                if (item->GetEntry() == entry)
                    itemList.push_back(item);
    }

    return itemList;
}

void Player::SendMailToSelf(uint32 title_string, uint32 subject_string)
{
	if (!GetSession())
		return;

	uint32 text = sObjectMgr.CreateItemText(GetSession()->GetHellgroundString(subject_string));

	// send mail
	MailDraft(GetSession()->GetHellgroundString(title_string), text)
		.SendMailTo(this, MailSender(MAIL_NORMAL, (uint32)0, MAIL_STATIONERY_GM));
}

bool Player::IsSemiHealer()
{
    // semi-healers
    switch (GetClass())
    {
    case CLASS_PALADIN:
        // Holy Shock || (!Crusader Strike && !Reckoning)
        //if (HasSpellChained(20473) || (!HasSpell(35395) && !HasSpellChained(20177)))

        // !Repentance && !Crusader Strike
        if (!HasSpell(20066) && !HasSpell(35395))
            return true;
        break;
    case CLASS_PRIEST:
        // !Vampiric Touch
        if (!HasSpellChained(34914))
            return true;
    case CLASS_DRUID:
        // !Mangle 
        //  && !HasSpell(33607) // && !Wrath of Cenarius (Rank 5)
        if (!HasSpell(33917))
            return true;

        // check diff healing-spd?
    case CLASS_SHAMAN:
        // Nature's Blessing
        if (HasSpellChained(30867))
            return true;
        break;
    }

    return false;
}

bool Player::IsHealer()
{
    // all spells must be rank 1! (NOT WORKING! find a clean way to check it)
    
    // true healers
    switch (GetClass())
    {
    case CLASS_PALADIN:
        // Holy Shock || (Illumination && !Improved Judgement && !Sanctity Aura)
        if ((HasSpellChained(20473) || (HasSpellChained(20210)) && !HasSpellChained(25956) && !HasSpell(20218)))
            return true;
        break;
    case CLASS_PRIEST:
        if (HasSpell(10060) // Power Infusion || Empowered Healing
            || (HasSpell(33158) || HasSpell(33159) || HasSpell(33160) || HasSpell(33161) || HasSpell(33162))) // all ranks
            return true;
        break;
    case CLASS_DRUID:
        //33597 dreamstate
        //33589 lunar guidance
        // https://wowwiki-archive.fandom.com/wiki/Druid_builds/Pre_3.0#Balance-Healing_.28Not_Restokin.29_.2824.2F0.2F37.29
        // Swiftmend || Nature's Swiftness && Moonkin Form && !Wrath of Cenarius
        if (HasSpell(18562) || (HasSpell(17116) && HasSpell(24858) &&
            (!HasSpell(33603) && !HasSpell(33604) && !HasSpell(33605) && !HasSpell(33606) && !HasSpell(33607)))) // all ranks
            return true;
        break;
    case CLASS_SHAMAN:
        if (HasSpell(974) || HasSpell(32593) || HasSpell(32594)) // Earth Shield
            return true;
        break;
    }

    return false;
}

std::pair<uint8, uint8> Player::GetMorphShirtRaceGender(Player* _owner)
{
    uint8 race = 0;
    uint8 gender = 0;
    uint32 shirt_model;

    if (Item *pItem = _owner->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BODY))
    {
        if (ItemPrototype const *proto = pItem->GetProto())
        {
            if (proto->Spells[0].SpellId == 55401)
            {
                shirt_model = (uint32)proto->Spells[0].SpellPPMRate;

                if (shirt_model > 0)
                {
                    switch (shirt_model)
                    {
                    case 20317:
                    {
                        race = RACE_DWARF;
                        gender = GENDER_MALE;
                        break;
                    }
                    case 20318:
                    {
                        race = RACE_NIGHTELF;
                        gender = GENDER_MALE;
                        break;
                    }
                    case 20316:
                    {
                        race = RACE_ORC;
                        gender = GENDER_FEMALE;
                        break;
                    }
                    case 20585:
                    {
                        race = RACE_TAUREN;
                        gender = GENDER_MALE;
                        break;
                    }
                    case 20584:
                    {
                        race = RACE_TAUREN;
                        gender = GENDER_FEMALE;
                        break;
                    }
                    case 20578:
                    {
                        race = RACE_BLOODELF;
                        gender = GENDER_MALE;
                        break;
                    }
                    case 20579:
                    {
                        race = RACE_BLOODELF;
                        gender = GENDER_FEMALE;
                        break;
                    }
                    case 20581:
                    {
                        race = RACE_GNOME;
                        gender = GENDER_FEMALE;
                        break;
                    }
                    case 20580:
                    {
                        race = RACE_GNOME;
                        gender = GENDER_MALE;
                        break;
                    }
                    case 19724:
                    {
                        race = RACE_HUMAN;
                        gender = GENDER_FEMALE;
                        break;
                    }
                    case 19723:
                    {
                        race = RACE_HUMAN;
                        gender = GENDER_MALE;
                        break;
                    }
                    case 20321:
                    {
                        race = RACE_TROLL;
                        gender = GENDER_MALE;
                        break;
                    }
                    case 20323:
                    {
                        race = RACE_DRAENEI;
                        gender = GENDER_FEMALE;
                        break;
                    }
                    default: 
                        break;
                    }
                }
            }
        }
    }

    if (race > 0)
    {
        //_owner->SetDisplayId(shirt_model);
        return std::make_pair(race, gender);
    }
    
    //data << model;
    return std::make_pair((uint8)_owner->GetRace(), (uint8)_owner->GetGender());
}

uint32 Player::GetGearScore() const
{
    // check in db
    // set @name = 'Eses';
    // select c.account as account_id, c.guid, c.name, sum(item.itemlevel) as sumilvl,count(*) from mangos3.item_template item, characters3.characters c, characters3.character_inventory inv where inv.slot between 0 and 17 and c.name = @name and inv.slot not in(18,16,3,12,13,17) and inv.item_template = item.entry and c.guid = inv.guid and inv.bag = 0;

    uint32 gs = 0;

    for (int i = EQUIPMENT_SLOT_HEAD; i <= EQUIPMENT_SLOT_RANGED; i++)
    {
        switch (i)
        {
        case EQUIPMENT_SLOT_TABARD:
        case EQUIPMENT_SLOT_OFFHAND:
        case EQUIPMENT_SLOT_BODY:
            continue;
        }

        Item* itemTarget = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (!itemTarget)
            continue;

        ItemPrototype const* proto = itemTarget->GetProto();
        if (!proto)
            continue;

        // libram, idol, totem - always same value, because of randeg weapons for other classes
        if (i == EQUIPMENT_SLOT_RANGED && proto->Class == 4 && (proto->SubClass == 7 || proto->SubClass == 8 || proto->SubClass == 9))
        {
            gs += 150;
            continue;
        }
        // trinkets can be also blue
        else if (i == EQUIPMENT_SLOT_TRINKET1 || i == EQUIPMENT_SLOT_TRINKET2)
        {
            if (proto->ItemLevel < 115)
            {
                gs += 115;
                continue;
            }
        }
        
        gs += itemTarget->GetProto()->ItemLevel;
    }

    return gs;
}

void Player::RewardRAF()
{
    WorldSession* s = GetSession();
    if (!s || !s->IsRaf())
        return;

    if (s->IsAccountFlagged(ACC_RAF_HIT) || s->IsAccountFlagged(ACC_RAF_REJECTED))
        return;

    uint32 recruiterAcc = s->GetRAF_Recruiter();
    if (!recruiterAcc)
        return;

    uint32 acc = s->GetAccountId();

	// INTERVAL 1 DAY!
    auto isSamePlayer = [](uint32 first, uint32 second)
    {
		// check
		// SELECT a.account_id as id1,a.login_date as date1,Concat(a.ip, '_', a.local_ip) as ip1,b.account_id as id2,b.login_date as date2,b.cc as ip2 FROM account_login a INNER JOIN (SELECT DISTINCT account_id,Concat(ip, '_', local_ip) cc,login_date FROM account_login WHERE account_id = %u and login_date>now() - interval 1 day) b ON Concat(a.ip, '_', a.local_ip) = b.cc WHERE a.login_date>now() - interval 1 day and a.account_id = %u LIMIT 1;
		
		return AccountsDatabase.PQuery("SELECT a.account_id,b.account_id FROM account_login a INNER JOIN (SELECT DISTINCT account_id,Concat(ip, '_', local_ip) cc FROM account_login WHERE account_id = %u and login_date>now() - interval 1 day) b ON Concat(a.ip, '_', a.local_ip) = b.cc WHERE a.login_date>now() - interval 1 day and a.account_id = %u LIMIT 1", first, second);
    };

    // if inviter IP = invited IP
    if (isSamePlayer(acc, recruiterAcc))
    {
        s->AddAccountFlag(ACC_RAF_REJECTED);
        AccountsDatabase.DirectPExecute("UPDATE account SET recruiter = 0 WHERE account_id = '%u'", acc);       
        sLog.outLog(LOG_RAF, "RAF status cancelled for account %u by LOGIN_SAME_COMPUTER (invited by %u)", acc, recruiterAcc);
		SendMailToSelf(16575, 16576);
        return;
    }

    // reward
    if (!s->IsAccountFlagged(ACC_RAF_HIT) && GetLevel() == 70 && GetGearScore() >= GS_GOOD && GetTotalPlayedTime() > 3 * HOUR)
    {
        // find other RAF accounts on this IP
        // multibox case (if multiboxers reg for other person referral link)
        // can be heavy, so execute only after GetGearScore() passed
        QueryResultAutoPtr result = AccountsDatabase.PQuery("SELECT account_id FROM account WHERE recruiter = '%u' AND account_flags & 0x00100000 AND account_id != '%u'", recruiterAcc, acc);
        if (result)
        {
            if (result.count() > 10)
                sLog.outLog(LOG_CRITICAL, "OMG there's > 10 referral accounts for %u", recruiterAcc);

            do
            {
                if (isSamePlayer(acc, (*result)[0].GetUInt32()))
                {
                    s->AddAccountFlag(ACC_RAF_REJECTED);
                    AccountsDatabase.DirectPExecute("UPDATE account SET recruiter = 0 WHERE account_id = '%u'", acc);
					sLog.outLog(LOG_RAF, "RAF status cancelled for account %u by LOGIN_SAME_COMPUTER on reward (invited by %u)", acc, recruiterAcc);
                    return;
                }
            } while (result->NextRow());
        }

        s->AddAccountFlag(ACC_RAF_HIT);

        if (WorldSession* rec = sWorld.FindSession(recruiterAcc))
            rec->modifyRAFCoins(1);
        else
            AccountsDatabase.DirectPExecute("UPDATE account SET rafcoins = rafcoins + 1 WHERE account_id = '%u'", recruiterAcc);

        sLog.outLog(LOG_RAF, "Referral reward for account %u (for invited account %u)", recruiterAcc, acc);
    }
}

bool Player::CanStoreItemCount(uint32 entry, uint32 count, bool exact_amount)
{
    ItemPosCountVec dest;
    uint32 no_space_count = 0;
    uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, count, &no_space_count);
    if (msg != EQUIP_ERR_OK)
    {
        // meaning in one stack
        if (exact_amount)
        {
            ChatHandler(GetSession()).SendSysMessage(LANG_SCRIPT_INVENTORY_NO_SPACE);
            return false;
        }

        count -= no_space_count;
    }

    if (count == 0 || dest.empty())                         // can't add any
    {
        ChatHandler(GetSession()).SendSysMessage(LANG_SCRIPT_INVENTORY_NO_SPACE);
        return false;
    }
    
    return true;
}

Item* Player::GiveItem(uint32 entry, uint32 count, uint32 need_entry, uint32 need_count)
{
    if (count > 0)
    {
		if (!ObjectMgr::GetItemPrototype(entry) || (need_entry && !ObjectMgr::GetItemPrototype(need_entry)))
		{
			ChatHandler(GetSession()).SendSysMessage(LANG_SCRIPT_ERROR);
			return nullptr;
		}

		ItemPosCountVec dest;
        uint32 no_space_count = 0;
        uint8 msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, count, &no_space_count);
        if (msg != EQUIP_ERR_OK)
        {
            ChatHandler(GetSession()).SendSysMessage(LANG_SCRIPT_INVENTORY_NO_SPACE);
            return nullptr;
        }
        
        // check if we need to take an item
        if (need_entry && need_count && ObjectMgr::GetItemPrototype(need_entry) && need_count > 0)
        {
            if (!HasItemCount(need_entry, need_count))
            {
                ChatHandler(GetSession()).SendSysMessage(LANG_SCRIPT_NOT_ENOUGH_ITEMS);
                return nullptr;
            }

            DestroyItemCount(need_entry, need_count, true, false, "GIVE_ITEM_DESTROY");
        }      

        // send item
        if (Item* item = StoreNewItem(dest, entry, true, 0, "GIVE_ITEM"))
        {
            SendNewItem(item, count, true, false);
            return item;
        }  
    }

    ChatHandler(GetSession()).SendSysMessage(LANG_SCRIPT_ERROR);
    return nullptr;
}

void Player::ProfessionSkillUpgradeOnLearn(uint32 spell_id, uint32 upgrade_to)
{
    uint16 maxskill = GetMaxSkillValueForLevel();

    SpellLearnSkillNode const* spellLearnSkill = sSpellMgr.GetSpellLearnSkill(spell_id);

    if (spellLearnSkill)
    {
        uint32 skill_value = GetPureSkillValue(spellLearnSkill->skill);
        uint32 skill_max_value = GetPureMaxSkillValue(spellLearnSkill->skill);

        if (skill_value < spellLearnSkill->value)
            skill_value = spellLearnSkill->value;

        uint32 new_skill_max_value = spellLearnSkill->maxvalue == 0 ? maxskill : spellLearnSkill->maxvalue;

        if (skill_max_value < new_skill_max_value)
            skill_max_value = new_skill_max_value;

        
        uint32 skill_id = spellLearnSkill->skill;

        if (new_skill_max_value <= upgrade_to && (
            skill_id == SKILL_ALCHEMY ||
            skill_id == SKILL_BLACKSMITHING ||
            skill_id == SKILL_ENGINERING ||
            skill_id == SKILL_JEWELCRAFTING ||
            skill_id == SKILL_LEATHERWORKING ||
            skill_id == SKILL_TAILORING ||
            skill_id == SKILL_ENCHANTING ||
            skill_id == SKILL_HERBALISM ||
            skill_id == SKILL_MINING ||
            skill_id == SKILL_SKINNING ||
            skill_id == SKILL_COOKING ||
            skill_id == SKILL_FISHING ||
            skill_id == SKILL_FIRST_AID ||
            skill_id == SKILL_POISONS
            ))
        {
            SetSkill(spellLearnSkill->skill, new_skill_max_value, skill_max_value);
        }
    }
}

void Player::addTalent(uint32 spellId, uint8 spec, bool learning)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
    {
        sLog.outDetail("Player::addTalent: Non-existed in SpellStore spell #%u request.", spellId);
        return;
    }

    if (!SpellMgr::IsSpellValid(spellInfo, this, false))
    {
        sLog.outDetail("Player::addTalent: Broken spell #%u learning not allowed.", spellId);
        return;
    }

    PlayerTalentMap::iterator itr = m_talents[spec].find(spellId);
    if (itr != m_talents[spec].end())
        itr->second->state = PLAYERSPELL_UNCHANGED;
    else if (TalentSpellPos const* talentPos = GetTalentSpellPos(spellId))
    {
        if (TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id))
        {
            for (uint8 rank = 0; rank < MAX_TALENT_RANK; ++rank)
            {
                // skip learning spell and no rank spell case
                uint32 rankSpellId = talentInfo->RankID[rank];
                if (!rankSpellId || rankSpellId == spellId)
                    continue;

                itr = m_talents[spec].find(rankSpellId);
                if (itr != m_talents[spec].end())
                    itr->second->state = PLAYERSPELL_REMOVED;
            }
        }

        PlayerSpellState state = learning ? PLAYERSPELL_NEW : PLAYERSPELL_UNCHANGED;
        PlayerTalent* newtalent = new PlayerTalent();

        newtalent->state = state;
        newtalent->spec = spec;

        (m_talents[spec])[spellId] = newtalent;
    }
}

void Player::_SaveTalents()
{
    for (uint8 i = 0; i < MAX_TALENT_SPECS; ++i)
    {
        for (PlayerTalentMap::const_iterator itr = m_talents[i].begin(); itr != m_talents[i].end();)
        {
            if (itr->second->state == PLAYERSPELL_REMOVED || itr->second->state == PLAYERSPELL_CHANGED)
                RealmDataDatabase.PExecute("DELETE FROM character_talent WHERE guid = '%u' and spell = '%u' and spec = '%u'", GetGUIDLow(), itr->first, itr->second->spec);
            if (itr->second->state == PLAYERSPELL_NEW || itr->second->state == PLAYERSPELL_CHANGED)
                RealmDataDatabase.PExecute("INSERT INTO character_talent (guid,spell,spec) VALUES ('%u', '%u', '%u')", GetGUIDLow(), itr->first, itr->second->spec);

            if (itr->second->state == PLAYERSPELL_REMOVED)
            {
                delete itr->second;
                m_talents[i].erase(itr++);
            }
            else
            {
                itr->second->state = PLAYERSPELL_UNCHANGED;
                ++itr;
            }
        }
    }
}

bool Player::HasTalent(uint32 spell, uint8 spec) const
{
    PlayerTalentMap::const_iterator itr = m_talents[spec].find(spell);
    return (itr != m_talents[spec].end() && itr->second->state != PLAYERSPELL_REMOVED);
}

void Player::ActivateSpec(uint8 spec)
{
    //if (GetFreeTalentPoints() > 0)
    //{
    //    GetSession()->SendNotification(LANG_SPEND_ALL_TAL);
    //    return;
    //}     
    
    if (GetActiveSpec() == spec)
    {
        GetSession()->SendNotification(LANG_SAME_SPEC);
        return;
    }

    if (IsNonMeleeSpellCast(false))
        InterruptNonMeleeSpells(false);

    // Save current Actions
    _SaveActions();

    // TO-DO: We need more research to know what happens with warlock's reagent
    if (Pet* pet = GetPet())
        RemovePet(pet, PET_SAVE_NOT_IN_SLOT, true);

    ClearComboPointHolders();
    ClearAllReactives();
    UnsummonAllTotems();

    // REMOVE TALENTS
    for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(talentId);

        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

        if (!talentTabInfo)
            continue;

        // unlearn only talents for character class
        // some spell learned by one class as normal spells or know at creation but another class learn it as talent,
        // to prevent unexpected lost normal learned spell skip another class talents
        if ((GetClassMask() & talentTabInfo->ClassMask) == 0)
            continue;

        for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
        {
            uint32 spell_id = talentInfo->RankID[rank];
            
            if (spell_id == 0)
                continue;
            removeSpell(spell_id, !SpellMgr::IsPassiveSpell(spell_id)); // removes the talent, and all dependant, learned, and chained spells..
            if (SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell_id))
                for (uint8 i = 0; i < 3; ++i)                  // search through the SpellInfo for valid trigger spells
                    if (spellInfo->EffectTriggerSpell[i] > 0 && spellInfo->Effect[i] == SPELL_EFFECT_LEARN_SPELL)
                        removeSpell(spellInfo->EffectTriggerSpell[i], !SpellMgr::IsPassiveSpell(spell_id)); // and remove any spells that the talent teaches
        }
    }

    SetActiveSpec(spec);
    uint32 spentTalents = 0;

    // ADD TALENTS
    for (uint32 talentId = 0; talentId < sTalentStore.GetNumRows(); ++talentId)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(talentId);

        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

        if (!talentTabInfo)
            continue;

        // learn only talents for character class
        if ((GetClassMask() & talentTabInfo->ClassMask) == 0)
            continue;

        for (int8 rank = 0; rank < 5; rank++)
        {
            // skip non-existant talent ranks
            if (talentInfo->RankID[rank] == 0)
                continue;
            // if the talent can be found in the newly activated PlayerTalentMap
            if (HasTalent(talentInfo->RankID[rank], m_activeSpec))
            {
                learnSpell(talentInfo->RankID[rank]);
                spentTalents += (rank + 1);             // increment the spentTalents count
            }
        }
    }

    m_usedTalentCount = spentTalents;
    InitTalentForLevel();

    // Need to relog player ???: TODO fix packet sending
    //if (!isGameMaster())
    //GetSession()->LogoutPlayer(true);

    //QueryResultAutoPtr actionResult = RealmDataDatabase.PQuery("SELECT button, action, type, misc FROM character_action WHERE guid = '%u' AND spec = '%u' ORDER BY button", GetGUIDLow(), m_activeSpec);
    //_LoadActions(actionResult);

    GetSession()->LogoutPlayer(true);

    //SaveRecallPosition();
    //TeleportTo(13, 0, 0, 0, 0);
    // AFTER: // teleport back after respec
}

void Player::_ResetTalentMap(uint8 specEntry)
{
    for (PlayerTalentMap::const_iterator itr = m_talents[specEntry].begin(); itr != m_talents[specEntry].end();)
    {
        delete itr->second;
        m_talents[specEntry].erase(itr++);
    }

    RealmDataDatabase.PExecute("DELETE FROM character_talent WHERE guid = '%u' AND spec = '%u'", GetGUIDLow(), specEntry);
}

std::string Player::PrintTalentCount(uint8 spec)
{
    uint32 talents[][2] = {
        0, 0,
        1, 0,
        2, 0,
    };

    for (PlayerTalentMap::const_iterator itr = m_talents[spec].begin(); itr != m_talents[spec].end(); ++itr)
    {
        if (itr->second->state == PLAYERSPELL_REMOVED)
            continue;

        TalentSpellPos const* talentPos = GetTalentSpellPos(itr->first);
        if (!talentPos)
            continue;

        TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id);
        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        int rank = 4;
        for (; rank >= 0; --rank)
        {
            if (talentInfo->RankID[rank] && talentInfo->RankID[rank] == itr->first)
            {
                for (uint32 *val : talents)
                    if (val[0] == talentTabInfo->tabpage)
                    {
                        val[1] += rank + 1;
                        break;
                    }

                break;
            }
        }
    }

    std::stringstream tal;
    tal << talents[0][1] << "/" << talents[1][1] << "/" << talents[2][1];

    return tal.str();
}

std::string Player::PrintNameAndClassInitials(uint8 race, uint8 Class, std::string name)
{
    std::string prace;
	std::string pclass;

	switch (race)
	{
	case 1: prace = "Human"; break;
	case 2: prace = "Orc"; break;
	case 3: prace = "Dwarf"; break;
	case 4: prace = "Nigth Elf"; break;
	case 5: prace = "Unded"; break;
	case 6: prace = "Tauren"; break;
	case 7: prace = "Gnome"; break;
	case 8: prace = "Troll"; break;
	case 10: prace = "Blood Elf"; break;
	case 11: prace = "Draenei"; break;
	default: prace = "UNKNOWN"; break;
	}

	switch (Class)
	{
	case 1: pclass = "Warrior"; break;
	case 2: pclass = "Paladin"; break;
	case 3: pclass = "Hunter"; break;
	case 4: pclass = "Rogue"; break;
	case 5: pclass = "Priest"; break;
	case 7: pclass = "Shaman"; break;
	case 8: pclass = "Mage"; break;
	case 9: pclass = "Warlock"; break;
	case 11: pclass = "Druid"; break;
	default: pclass = "UNKNOWN"; break;
	}
	
	if (race == 0)
		return pclass;
	else if (Class == 0)
		return prace;
	else
		return name + " (" + prace + ", " + pclass + ")";
}

bool Player::IsGuildHouseOwnerMember()
{
    return GetGuildId() && sWorld.m_guild_house_owner == GetGuildId();
}

bool Player::CanAddArenaQueueHashIP(uint8 debug)
{
    if (sWorld.getConfig(CONFIG_HASHIP_DISABLED) || isGameMaster())
        return true;

    auto& container = sWorld.haship_arena_queue;

    auto itr = container.find(GetHashIP());
    if (itr != container.end())
    {
        // same player, allow to reg multiple BG
        //if ((*itr).second == GetGUID())
        //    return true;

        // remove from queue
		sLog.outLog(LOG_CRITICAL, "Player name %s (guid %u) cannot be added to haship_arena_queue, hash %s, debug %u AT CanAddArenaQueueHashIP", GetName(), GetGUIDLow(), GetHashIP(), debug);
        return false;
    }

    return true;
}

void Player::AddArenaQueueHashIP(uint8 debug)
{
    if (sWorld.getConfig(CONFIG_HASHIP_DISABLED) || isGameMaster())
        return;
    
    const char* ip = GetHashIP();
    
    if (CanAddArenaQueueHashIP(2))
    {
        auto& container = sWorld.haship_arena_queue;

        container[ip] = GetGUID();
        sLog.outLog(LOG_SPECIAL, "Player name %s (guid %u) added to haship_arena_queue, hash %s", GetName(), GetGUIDLow(), ip);
    }
    else
        sLog.outLog(LOG_CRITICAL, "Player name %s (guid %u) cannot be added to haship_arena_queue, hash %s, debug %u", GetName(), GetGUIDLow(), ip, debug);
}

void Player::RemovArenaQueueHashIP(Player* player, uint64 guid)
{
	// skip player->isGameMaster(), because we should always remove even if entered as GM and exit as player
    
    if (sWorld.getConfig(CONFIG_HASHIP_DISABLED))
        return;
    
    auto& container = sWorld.haship_arena_queue;

    if (guid) 
    {
        for (auto& value : container)
        {
            if (value.second == guid)
            {
                container.erase(value.first);
                sLog.outLog(LOG_SPECIAL, "Player name ??? (guid %u) removed from haship_arena_queue, hash %s", guid, value.first.c_str());
            }
        }
    }
    else 
    {
        const char* ip = player->GetHashIP();
        if (container.erase(ip) > 0)
            sLog.outLog(LOG_SPECIAL, "Player name %s (guid %u) removed from haship_arena_queue, hash %s", player->GetName(), player->GetGUIDLow(), ip);
    }
}

void Player::RemoveItemCount(uint32 char_guid, uint32 item_guid, uint32 count)
{
	QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT item,SUBSTRING_INDEX(SUBSTRING_INDEX(data, ' ', 4),' ',-1) as entry,SUBSTRING_INDEX(SUBSTRING_INDEX(data, ' ', 15),' ',-1) as count FROM item_instance ii INNER JOIN character_inventory ci ON ii.guid=ci.item AND ii.owner_guid=ci.guid WHERE ii.guid=%u and ii.owner_guid=%u LIMIT 1;", item_guid, char_guid);
	if (!result)
	{
		ChatHandler(this).PSendSysMessage("Player does not have an item with this GUID");
		return;
	}

	uint32 item_entry = (*result)[1].GetUInt32();
	uint32 db_count = (*result)[2].GetUInt32();

	if (db_count == 0)
	{
		ChatHandler(this).PSendSysMessage("Error! Write Gensen!");
		return;
	}

	if (db_count < count)
	{
		ChatHandler(this).PSendSysMessage("Player does not have an item GUID with such count (has %u)", db_count);
		return;
	}

	if (count == 0)
		count = 1;

	Player* target = sObjectAccessor.GetPlayerInWorldOrNot(char_guid);

	if (target)
	{
		Item* pItem = target->GetItemByGuidLow(item_guid);

		if (!pItem)
		{
			ChatHandler(this).PSendSysMessage("Player (online) does not have this item in inventory");
			return;
		}

		uint32 cnt = count;
		target->DestroyItemCount(pItem, cnt, true, "DELETED_BY_GM");

		ChatHandler(this).PSendSysMessage("Item entry %u (count %u) was removed from player (online)", item_entry, count);
		target->SaveToDB();
	}
	else
	{
		// remove from DB
		if (count == db_count || db_count == 1)
		{
			RealmDataDatabase.PExecute("DELETE FROM item_instance WHERE guid='%u'", item_guid);
			RealmDataDatabase.PExecute("DELETE FROM character_inventory WHERE item='%u'", item_guid);
		}
		else
		{
			RealmDataDatabase.PExecute("UPDATE item_instance SET data=CONCAT(SUBSTRING(data,1,LENGTH(SUBSTRING_INDEX(data, ' ', 15-1))+1),%u,SUBSTRING(data,LENGTH(SUBSTRING_INDEX(data, ' ', 15))+1)) WHERE guid=%u AND owner_guid=%u;", int32(db_count - count), item_guid, char_guid);
		}

		ItemRemovedCheck(item_entry, count, "DELETED_BY_GM");

		ChatHandler(this).PSendSysMessage("Item entry %u (count %u) was removed from player (offline)", item_entry, count);
	}
}

uint32 Player::CalculateBonus(uint32 number)
{
	float multiplier = 0;

    // need to affect EoT? don't think so...
	//if (sWorld.getConfig(BONUS_RATES))
	//	multiplier += 0.5; // don't change manually because it's depended

	if (sWorld.isEasyRealm() && GetSession()->isPremium())
		multiplier += 0.5;

	// no bonus
	if (!multiplier)
		return 0;

	// 3*1.5 = 4 or 5
	if (number % 2)
	{
		if (urand(0, 1))
			return ceil(multiplier * number);
		else
			return floor(multiplier * number);
	}
	else
	// 4*1.5 = 6
	{
		return round(multiplier * number);
	}
}

uint32 Player::ClassQuestAvailable(uint32 level)
{
	switch (GetClass())
	{
	case CLASS_SHAMAN:
	{
		if (level == 4 || level == 10 || level == 20 || level == 30)
			return true;
	}
	case CLASS_WARLOCK:
	{
		if (level == 10 || level == 20 || level == 30 || level == 50)
			return true;
	}
	case CLASS_ROGUE:
	{
		if (level == 20)
			return true;
	}
	case CLASS_DRUID:
	{
		if (level == 10 || level == 14 || level == 16)
			return true;
	}
	case CLASS_WARRIOR:
	{
		if (level == 10 || level == 30)
			return true;
	}
	case CLASS_HUNTER:
	{
		if (level == 10)
			return true;
	}
	case CLASS_PALADIN:
	{
		if (level == 12)
			return true;
	}
	}

	return false;
}

void Player::IntiveToGlobalChannel(uint8 lang)
{
	if (!IsPlayerCustomFlagged(PL_CUSTOM_INV_TO_GLOBAL))
	{
		// join LFG
		if (ChannelMgr* cMgr = channelMgr(GetTeam()))
		{
			Channel *chn = nullptr;

			//if (lang == 1)
			//	chn = cMgr->GetJoinChannel("English", 0);
			//else
			//	chn = cMgr->GetJoinChannel("Russian", 0);

			chn = cMgr->GetJoinChannel("Global", 0);

			if (chn) {
				chn->Invite(GetGUID(), GetName(), false);
			}
		}

		// don't check anything, just add flag
		AddPlayerCustomFlag(PL_CUSTOM_INV_TO_GLOBAL);
	}
}

void Player::ClearCaptchaAction(CaptchaActions action)
{
	if (!sWorld.getConfig(CONFIG_CAPTCHA_ENABLED))
		return;

	// exit if have captcha
	if (!captcha_current.empty())
		return;

	// clear all
	if (action == 0)
	{
		sWorld.player_captchas.erase(GetGUIDLow());
		return;
	}

	auto it = sWorld.player_captchas.find(GetGUIDLow());
	if (it != sWorld.player_captchas.end())
	{
		auto it2 = sWorld.player_captchas[GetGUIDLow()].find(action);
		if (it2 != sWorld.player_captchas[GetGUIDLow()].end())
			sWorld.player_captchas[GetGUIDLow()].erase(action);
	}
}

bool Player::NeedCaptcha(CaptchaActions action)
{
	if (!sWorld.getConfig(CONFIG_CAPTCHA_ENABLED))
		return false;

	//sLog.outLog(LOG_TMP,"NeedCaptcha for %s action %u",GetName(),action);

	if (InBattleGroundOrArena() || GetMap()->IsDungeon())
		return false;

	if (!captcha_current.empty())
	{
		ChatHandler(this).PSendSysMessage(16580, captcha_current.c_str());
		GetSession()->SendNotification(16584, captcha_current.c_str());
		return true;
	}

	uint32 counter = 0;
	uint32 safetimer = MINUTE * 5;
	switch (action)
	{
	case CAPTCHA_BUY_VENDOR_RESTOCKED:
		counter = 0;
		safetimer = MINUTE * 10;
		break;
	case CAPTCHA_LOOT_PROF:
		//counter = 1;
		counter = urand(70, 110);
		safetimer = MINUTE * 60;
		return false;
		break;
	//case CAPTCHA_LOOT_CORPSE:
	//	counter = urand(100, 120);
	//	safetimer = MINUTE * 60;
	//	return false;
	//	break;
	//case CAPTCHA_SEND_MAIL:
	//	counter = 20;
	//	safetimer = 0; //generate captcha every $counter mails
	//	return false;
	//	break;
	}

	if (safetimer && time(NULL) - captcha_lastused < safetimer)
		return false;

	// check if needed counter is < player action counter
	if (counter > 0)
	{
		auto it = sWorld.player_captchas.find(GetGUIDLow());
		if (it != sWorld.player_captchas.end())
		{
			auto it2 = sWorld.player_captchas[GetGUIDLow()].find(action);
			if (it2 != sWorld.player_captchas[GetGUIDLow()].end())
			{
				if (it2->second < counter)
				{
					++it2->second;
					return false;
				}
			}
			else
			{
				sWorld.player_captchas[GetGUIDLow()] = std::map<uint32, uint32>{{action, 1}};
				return false;
			}
		}
		else
		{
			sWorld.player_captchas[GetGUIDLow()] = std::map<uint32, uint32>{{action, 1}};
			return false;
		}
	}

	GenerateCaptcha(action);

	ChatHandler(this).PSendSysMessage(16580, captcha_current.c_str());
	GetSession()->SendNotification(16584, captcha_current.c_str());

	return true;
}

bool Player::DoCaptcha(uint32 answer)
{
	if (answer != captcha_answer)
	{
		sLog.outLog(LOG_SPECIAL, "Captcha check failed for Player %s", GetName());
		return false;
	}

	sWorld.player_captchas.erase(GetGUIDLow());
	captcha_lastused = time(NULL);
	captcha_current.clear();
	sLog.outLog(LOG_SPECIAL, "Captcha check success for Player %s", GetName());
	return true;
}

void Player::GenerateCaptcha(uint32 from)
{
	if (!sWorld.getConfig(CONFIG_CAPTCHA_ENABLED))
		return;
	
	// _old
	//uint32 length = 4;
	//const char *required_chars = "0123456789"; 	//DFJLQUWXYZ
	//std::string captcha = "";
	//while (length--)
	//	captcha.push_back(required_chars[urand(0,9)]);

	uint32 first = urand(100, 990);
	uint32 second = urand(1, 9);
	bool plus = urand(0, 1);
	int32 answer = plus ? first + second : first - second;
	if (answer < 0)
		sLog.outLog(LOG_CRITICAL, "GenerateCaptcha < 0");		

	captcha_current = std::to_string(first) + (plus ? "+" : "-") + std::to_string(second);
	captcha_answer = (uint32)answer;

	sLog.outLog(LOG_SPECIAL, "Captcha %u generated for Player %s", from, GetName());
}

bool Player::CheckBadLexics(std::string msg)
{
    if (sWorld.Lexics && sWorld.Lexics->CheckLexics(msg))
    {
        sLog.outLog(LOG_INNORMATIVE, "Player %s (guid: %u): %s", GetName(), GetGUIDLow(), msg.c_str());
        return true;
    }

    return false;
}

//bool Player::RestrictedLegendaryEquipped()
//{
//    uint32 slots[3] = {EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED};
//    
//    for (uint32 slot : slots)
//    {
//        Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
//        if (item && IsCustomLegendaryWeapon(item->GetEntry()))
//            return true;
//    }
//
//    return false;
//}

bool Player::IsCustomLegendaryWeapon(uint32 entry, bool for_caster)
{
    if (for_caster)
    {
        switch (entry)
        {
        //case ITEM_LEGENDARY_DAGGER_SPD: not using CastItemCombatSpellFromCast()
        case ITEM_LEGENDARY_MACE_SPD:
        case ITEM_LEGENDARY_STAFF_SPD:
        case ITEM_LEGENDARY_STAFF_HEAL:
        case ITEM_LEGENDARY_MACE_HEAL:
            return true;
        }

        return false;
    }
    
    switch (entry)
    {
    case ITEM_LEGENDARY_AXE:
    case ITEM_LEGENDARY_DAGGER_ROGUE:
    case ITEM_LEGENDARY_FIST_MH:
    case ITEM_LEGENDARY_FIST_OH:
    case ITEM_LEGENDARY_FIST_OH_SLOW:
    case ITEM_LEGENDARY_DAGGER_SPD:
    case ITEM_LEGENDARY_MACE_SPD:
    case ITEM_LEGENDARY_STAFF_SPD:
    case ITEM_LEGENDARY_STAFF_HEAL:
    case ITEM_LEGENDARY_MACE_HEAL:
    case ITEM_LEGENDARY_STAFF_FERAL:
    case ITEM_LEGENDARY_SWORD_TANK:
        return true;
    }

    return false;
}

bool Player::CreateRated3v3Team()
{
    // Teamname = playername
    // if team name exist, we have to choose another name (playername + number)
    int i = 1;
    std::stringstream teamName;

    teamName << "[ Solo Queue 3v3 ]";

    // Create arena team
    ArenaTeam* arenaTeam = new ArenaTeam();

    if (!arenaTeam->Create(GetGUID(), ARENA_TEAM_3v3, teamName.str(), true))
    {
        //should never happen
        GetSession()->SendNotification(GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 12);
        delete arenaTeam;
        return false;
    }

    // looks cool
    arenaTeam->SetEmblem(4278190080, 94, 4294901769, 2, 4294905344);
    getObjectMgr()->AddArenaTeam(arenaTeam);

    return true;
}

bool Player::CreateRatedBGTeam()
{
    std::stringstream teamName;
    teamName << "[ Battleground Stats ]";

    // Create arena team
    ArenaTeam* arenaTeam = new ArenaTeam();

    if (!arenaTeam->Create(GetGUID(), ARENA_TEAM_5v5, teamName.str(), true))
    {
        //should never happen
        ChatHandler(GetSession()).SendSysMessage(LANG_SCRIPT_ERROR);
        delete arenaTeam;
        return false;
    }

    // looks cool
    arenaTeam->SetEmblem(4278190080, 23, 4293916141, 2, 4294245375);
    getObjectMgr()->AddArenaTeam(arenaTeam);

    return true;
}

bool Player::HasSpellChained(uint32 spell_id)
{
    SpellChainNode const* node = sSpellMgr.GetSpellChainNode(spell_id);

    if (node)
    {
        if (node->first != spell_id)
        {
            sLog.outLog(LOG_CRITICAL, "Spell id %u is not first rank! Getting first rank manually...", spell_id);
            return HasSpellChained(node->first);
        }

        while (node)
        {
            if (HasSpell(node->cur))
                return true;
            node = sSpellMgr.GetSpellChainNode(node->next);
        }
    }

    return false;
}

std::string Player::GetLocalizedItemName(uint32 entry, bool force_en)
{
    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(entry);
    if (proto)
    {
        int loc_idx = force_en ? -1 : GetSession()->GetSessionDbLocaleIndex();
        std::string item_name = proto->Name1;
        sObjectMgr.GetItemLocaleStrings(entry, loc_idx, &item_name);

        return item_name;
    }

    return "[ERROR]";
}

std::string Player::GetItemLink(uint32 entry, bool force_en, ItemLinkColorSettings color_settings)
{
    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(entry);
    if (proto)
    {
        int loc_idx = force_en ? -1 : GetSession()->GetSessionDbLocaleIndex();
        std::string item_name = proto->Name1;
        sObjectMgr.GetItemLocaleStrings(entry, loc_idx, &item_name);
        
        std::ostringstream oss;

        if (color_settings == ITEM_LINK_NO_COLOR)
        {
            oss << "[" << item_name << "]";
            
            if (proto->ItemId == 32837)
                oss << " " << GetSession()->GetHellgroundString(16708);
            else if (proto->ItemId == 32838)
                oss << " " << GetSession()->GetHellgroundString(16707);
        }
        else
            oss << "|c" << std::hex << (color_settings == ITEM_LINK_COLORED ? ItemQualityColors[proto->Quality] : ItemQualityColorsSpecial[proto->Quality]) << std::dec <<
            "|Hitem:" << proto->ItemId << ":0:0:0:0:0:0:0|h[" << item_name << "]|h|r";

        return oss.str();
    }

    return "[ERROR]";
}

void Player::SendCustomRaidInfo(uint32 map, bool is_heroic)
{
    if (!sWorld.isEasyRealm())
        return;

    uint32 normal_max_players = Map::GetMaxPlayers(map, false);
    uint32 heroic_max_players = Map::GetMaxPlayers(map, true);

    //bool heroic_raid = false;  
    //if (raid_info != sWorld.heroic_raids_info.end())
    //    heroic_raid = is_heroic;

    // player count
    std::string msg = ChatHandler(this).PGetSysMessage(16642, is_heroic ? heroic_max_players : normal_max_players);
    msg += ChatHandler(this).PGetSysMessage(is_heroic ? 16667 : 16668, is_heroic ? normal_max_players : heroic_max_players);

    ChatHandler(this).SendSysMessage(msg.c_str());
        
    // additional drop
    auto raid_info = sWorld.creature_map_mod.find(-static_cast<int32>(map));
    if (raid_info != sWorld.creature_map_mod.end())
    {
        std::stringstream ss(raid_info->second.legendary_weapons);
        std::string item;

        std::string weapon_links;
        while (ss >> item) 
        {
            weapon_links += GetItemLink(std::stoi(item), false, ITEM_LINK_COLORED);
			weapon_links += " ";
        }

        ChatHandler(this).PSendSysMessage(16749, weapon_links.c_str());

        uint8 count_pre = raid_info->second.plus_drop_pre;
        uint8 count_final = raid_info->second.plus_drop_final;

        if (count_pre || count_final)
        {
            ChatHandler(this).PSendSysMessage(16663, count_final, count_pre);
        }
    }
}

//bool Player::HasActionCooldown(uint32 action_id)
//{
//    
//    
//    auto it = action_cooldown.find(action_id);
//    if (it != action_cooldown.end()) {
//        if (it->second > time(NULL))
//            return true;
//    }
//    else 
//    {
//        action_cooldown[action_id] = time(NULL);
//    }
//}

std::set<Pet*> Player::GetAllPets() const
{
    std::set<Pet*> all_pets;

    if (uint64 pet_guid = GetPetGUID())
    {
        if (Pet* pet = ObjectAccessor::GetPet(pet_guid))
            all_pets.insert(pet);
    }

    for (GuardianPetList::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end(); ++itr)
    {
        if (Pet* pet = ObjectAccessor::GetPet(*itr))
            all_pets.insert(pet);
    }

    if (m_miniPet)
    {
        if (Pet* pet = ObjectAccessor::GetPet(m_miniPet))
            all_pets.insert(pet);
    }

    return all_pets;
}

uint32 Player::GetRandomHeroicDungeonZone()
{
    auto& container = sWorld.heroicDungeonZones;

    auto it = container.begin();
    std::advance(it, urand(0, container.size() - 1));
    return *it;
}

uint32 Player::GetRandomRaidZone()
{
    auto& container = sWorld.raidZones;
    
    auto it = container.begin();
    std::advance(it, urand(0, container.size() - 1));
    return *it;
}

void Player::AddPlayerIPHash()
{
    sWorld.players_haship[GetHashIP()].push_back(GetGUIDLow());
}

void Player::RemovePlayerIPHash()
{
    auto& same_ip = sWorld.players_haship;
    std::string ip = GetHashIP();

    auto it = same_ip.find(ip);
    if (it != same_ip.end())
    {
        auto& ip_vec = it->second;
        auto it_vec = std::find(ip_vec.begin(), ip_vec.end(), GetGUIDLow());
        if (it_vec != ip_vec.end())
        {
            ip_vec.erase(it_vec);

            if (ip_vec.empty())
                same_ip.erase(it);
        }
        else
            sLog.outLog(LOG_CRITICAL, "Can't remove Player %s from RemovePlayerIPHash", GetName());
    }
}

TwinkGuids Player::GetSamePlayers()
{
    return sWorld.players_haship[GetHashIP()];
}

bool Player::IsDoubleLogin()
{
    if (sWorld.getConfig(CONFIG_HASHIP_DISABLED))
        return false;
    
    return sWorld.players_haship[GetHashIP()].size() > 1;
}

bool Player::IsDoubleLogin(std::string haship)
{
    if (sWorld.getConfig(CONFIG_HASHIP_DISABLED))
        return false;
    
    return sWorld.players_haship[haship].size() > 1;
}

void Player::SendCenterMessage(uint32 string_id)
{
    if (!GetSession())
        return;

    int loc_idx = GetSession()->GetSessionDbLocaleIndex();
    char const* text = sObjectMgr.GetHellgroundString(string_id, loc_idx);

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data, CHAT_MSG_RAID_BOSS_WHISPER, text, LANG_UNIVERSAL, GetNameForLocaleIdx(loc_idx), GetGUID());
    SendPacketToSelf(&data);
}

void Player::SendCenterMessage(const char* text)
{
    if (!GetSession())
        return;
       
    int loc_idx = GetSession()->GetSessionDbLocaleIndex();

    WorldPacket data(SMSG_MESSAGECHAT, 200);
    BuildMonsterChat(&data, CHAT_MSG_RAID_BOSS_WHISPER, text, LANG_UNIVERSAL, GetNameForLocaleIdx(loc_idx), GetGUID());
    SendPacketToSelf(&data);
}
