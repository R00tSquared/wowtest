// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//Owned by DeathSide, Trentone
#include "precompiled.h"
#include "deathside_pvp_master.h"
#include "Language.h"
#include "GameEvent.h"
#include "BattleGroundMgr.h"
#include "ObjectMgr.h"
#include "ArenaTeam.h"

std::string GetPlRaceName(uint8 PlRace)
{
    std::string plRaceName = "";
    switch(PlRace)
    {
        case 1: plRaceName = "Human"; break;
        case 2: plRaceName = "Orc"; break;
        case 3: plRaceName = "Dwarf"; break;
        case 4: plRaceName = "N.Elf"; break;
        case 5: plRaceName = "Und"; break;
        case 6: plRaceName = "Taur"; break;
        case 7: plRaceName = "Gnome"; break;
        case 8: plRaceName = "Troll"; break;
        case 10: plRaceName = "B.Elf"; break;
        case 11: plRaceName = "Draen"; break;
        default: plRaceName = "UNKNOWN"; break;
    }
    return plRaceName;
}

std::string GetPlClassName(uint8 PlClass)
{
    std::string plClassName = "";
    switch(PlClass)
    {
        case 1: plClassName = "Warr"; break;
        case 2: plClassName = "Pala"; break;
        case 3: plClassName = "Hunt"; break;
        case 4: plClassName = "Rog"; break;
        case 5: plClassName = "Pri"; break;
        case 7: plClassName = "Sham"; break;
        case 8: plClassName = "Mage"; break;
        case 9: plClassName = "Warl"; break;
        case 11: plClassName = "Dru"; break;
        default: plClassName = "UNKNOWN"; break;
    }
    return plClassName;
}

void DeathSide_Arena_Spectator_Map_InstId_Manipulation(uint32 mapId, uint32 InstId, Map *map, uint8 &GossipNumber, Player * pPlayer)
{
    Map::PlayerList const &PlayerList = map->GetPlayers();
    if (!PlayerList.isEmpty())
    {
        Player* itrplayer = NULL;
        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            if (!(itr->getSource()->IsInWorld()))
                continue;

            if (itr->getSource()->IsSpectator())
                continue;

            if (itrplayer = itr->getSource())
                break;
        }
        BattleGround * bg = NULL;
        if (itrplayer)
            bg = itrplayer->GetBattleGround();
        if (bg && bg->isRated() && bg->GetStatus() == STATUS_IN_PROGRESS)
        {
            char gossip[256];
            uint8 ArenaId;
            switch (mapId)
            {
            case 572: ArenaId = 0; break;
            case 559: ArenaId = 1; break;
            default: ArenaId = 2; break;
            }
            uint32 aTeamId = bg->GetArenaTeamIdForTeam(ALLIANCE);
            uint32 hTeamId = bg->GetArenaTeamIdForTeam(HORDE);
            if (aTeamId && hTeamId)
            {
                ArenaTeam* aTeam = getObjectMgr()->GetArenaTeamById(aTeamId);
                ArenaTeam* hTeam = getObjectMgr()->GetArenaTeamById(hTeamId);
                if (aTeam && hTeam)
                {
                    sprintf(gossip, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_FIGHT_VERSUS),
                        pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_START_LORD + ArenaId),
                        bg->GetArenaType(), bg->GetArenaType(), aTeam->GetRating(), hTeam->GetRating());
                    pPlayer->ADD_GOSSIP_ITEM(9, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + GossipNumber);
                    //ThereIsArena = true;
                    Arenas[GossipNumber - 20].ArenaId = mapId;
                    Arenas[GossipNumber - 20].InstanceId = InstId;
                    GossipNumber++;
                }
            }
        }
    }
}

void DeathSide_Arena_Spectator_Gossip_Manipulation(uint32 InstanceId, uint32 MapId, Player *pPlayer)
{
    pPlayer->SaveRecallPosition();
    Player *pl = NULL;
    Map *map = pPlayer->FindMapFromScripts(MapId, InstanceId);
    if (!map)
        return;
    Map::PlayerList const &PlayerListForGossip = map->GetPlayers();
    if (!PlayerListForGossip.isEmpty())
        pl = PlayerListForGossip.begin()->getSource();
    if (pl && pl->InArena())
    {
        pPlayer->RemoveAllAurasOnDeath();
        Pet* pet = pPlayer->GetPet();
        if (pet)
            pPlayer->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT);
        pPlayer->SetTemporaryUnsummonedPetNumber(0);
        pPlayer->SetBattleGroundId(pl->GetBattleGroundId(), pl->GetBattleGround()->GetTypeID());
        pPlayer->SaveOwnBattleGroundEntryPoint();
        pPlayer->TeleportTo(pl->GetMapId(), pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ(), pl->GetOrientation());
        pPlayer->SetSpectator(true);
    }
}

bool Create3v3Team(Player* pPlayer, Creature* pCreature) {
    //if (!getBattleGroundMgr()->GetDebugArenaId()) {
    //    if (!pPlayer->HasItemCount(693110, 1, false))
    //    {
    //        pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_NEED_TO_PAY), pPlayer->GetGUID());
    //        return false;
    //    }

    //    pPlayer->DestroyItemCount(693110, 1, true);
    //    pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_PAY_SUCCESS), pPlayer->GetGUID());
    //}

    // Teamname = playername
    // if team name exist, we have to choose another name (playername + number)
    int i = 1;
    std::stringstream teamName;
    teamName << "3v3 Solo - " << pPlayer->GetName();
    // teamName << player->GetName();
    do
    {
        if (getObjectMgr()->GetArenaTeamByName(teamName.str())) // teamname exist, so choose another name
        {
            teamName.str(std::string());
            teamName << "3v3 Solo - " << pPlayer->GetName() << i++;
        }
        else
            break;
    } while (i < 100); // should never happen

    // Create arena team
    ArenaTeam* arenaTeam = new ArenaTeam();

    if (!arenaTeam->Create(pPlayer->GetGUID(), ARENA_TEAM_3v3, teamName.str()))
    {
        //should never happen
        pPlayer->GetSession()->SendNotification(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 12);
        delete arenaTeam;
        return false;
    }

    // looks cool
    arenaTeam->SetEmblem(4278190080, 94, 4294901769, 2, 4294905344);
    getObjectMgr()->AddArenaTeam(arenaTeam);

    return true;
}

void Reg3v3ArenaAndCreateTeamIfNeeded(Player* pPlayer, Creature* pCreature) {
    uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_3v3);

    // check if player in already in arena team, if not - create
    if (!pPlayer->GetArenaTeamId(slot) && !Create3v3Team(pPlayer, pCreature))
        return;
    
    if (!getBattleGroundMgr()->GetDebugArenaId()) {
        // only max level
        // check if arena reported
        if (pPlayer->HasAura(55402)) {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_IS_REPORTED), pPlayer->GetGUID());
            return;
        }

        if (pPlayer->getLevel() < 70) {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_NEED_MAX_LVL), pPlayer->GetGUID());
            return;
        }

        // check bg queue
        if (pPlayer->InBattleGroundQueue())
        {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_ALREADY_IN_QUEUE), pPlayer->GetGUID());
            return;
        }

        // check Deserter debuff
        if (getWorld()->getConfig(CONFIG_SOLO_3V3_CAST_DESERTER) && pPlayer->HasAura(55402)) {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_HAVE_DESERTER), pPlayer->GetGUID());
            return;
        }

        // need to have at least 1 hour to join
        if (pPlayer->GetTotalPlayedTime() < HOUR) {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_FEW_PLAYED_TIME), pPlayer->GetGUID());
            return;
        }

        // check for talents
        if (pPlayer->GetFreeTalentPoints() > 0)
        {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_HAS_FREE_TALENT_POINTS), pPlayer->GetGUID());
            return;
        }

        // can't queue as group
        if (pPlayer->GetGroup())
        {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_AS_GROUP), pPlayer->GetGUID());
            return;
        }

        // check items
        uint32 itemlevelsum = 0;
        uint8 defenciveitemscount = 0;
        bool haspvptrinket = false;

        for (int i = EQUIPMENT_SLOT_HEAD; i < EQUIPMENT_SLOT_RANGED; i++)
        {
            if (i == EQUIPMENT_SLOT_OFFHAND || i == EQUIPMENT_SLOT_BODY)
                continue;
                           
            Item* itemTarget = pPlayer->GetItemByPos(INVENTORY_SLOT_BAG_0, i);

            if (!itemTarget)
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_NAKED), pPlayer->GetGUID());
                return;
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
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_NEED_PVP_TRINKET), pPlayer->GetGUID());
            return;
        }

        if (defenciveitemscount > 3 || 
            pPlayer->HasSpell(27168) || //paladin proto key tal
            pPlayer->HasSpell(12809)) //warr tank key tal
        {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_TANK_ITEMS), pPlayer->GetGUID());
            return;
        }

        if (itemlevelsum < GS_A1)
        {
            pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SQ_ITEMLEVEL), pPlayer->GetGUID());
            return;
        }
    }

    WorldPacket Data;
    Data << (uint64)pCreature->GetGUID() << (uint8)slot/*3v3 slot*/ << (uint8)0/*asGroup*/ << (uint8)1/*isRated*/;
    pPlayer->GetSession()->HandleBattleGroundArenaJoin(Data);
    pPlayer->CLOSE_GOSSIP_MENU();
}

bool DeathSide_pvp_master_Hello(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if(getWorld()->getConfig(CONFIG_REALM_TYPE) == 1)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_REGISTRATION_BG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if (getWorld()->getConfig(CONFIG_19_LVL_ADAPTATIONS) && (pPlayer->getLevel() >= TWINK_LEVEL_MIN && pPlayer->getLevel() <= TWINK_LEVEL_MAX))
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_JOIN_2V2_SKIRMISH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 75);

        if (pPlayer->getLevel() == 70)
        { 
            if (!pPlayer->GetArenaTeamId(ArenaTeam::GetSlotByType(ARENA_TEAM_3v3)))
                pPlayer->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SQ_REG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 74, pPlayer->GetSession()->GetHellgroundString(LANG_SQ_TEAM_WILL_BE_CREATED), 0, false);
            else
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SQ_REG), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 74);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_JOIN_2V2_RATED), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_CRE_REG_BLIZZLIKE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        }

        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_BG_STATS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 76);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 61);
        //if (pPlayer->GetCachedArea() != getWorld()->getConfig(CONFIG_PVP_AREA_ID))
        //    pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_TELEPORT_DUEL_ZONE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+58, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARE_YOU_SURE), 0, false);
    }
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_SPECTATING), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    pPlayer->SEND_GOSSIP_MENU(68,pCreature->GetGUID());
    return true;
}

bool DeathSide_pvp_master_Gossip(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    uint32 PosNumber  = 0;
    uint32 PosNumber1 = 0;
    uint32 Team1Member = 0;

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            // check bg queue
            if (pPlayer->InBattleGroundQueue())
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_ALREADY_IN_QUEUE), pPlayer->GetGUID());
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            }
            
            if (!getWorld()->getConfig(CONFIG_BG_RANDOM_SELECT_PERIOD))
            {
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                pPlayer->SEND_GOSSIP_MENU(68, pCreature->GetGUID());
            }
            else
            {                
                char chr[256];
                bool ending = false;

                if (getBattleGroundMgr()->IsBGSendReinforcements(BATTLEGROUND_WS))
                {
                    sprintf(chr, "|cff0000cc%s [active]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
                }
                else if (getBattleGroundMgr()->IsBGSendReinforcements_Ending(BATTLEGROUND_WS))
                {
                    sprintf(chr, "|cff0000cc%s [ending]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_WARSONG_GULCH));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
                    ending = true;
                }

                if (getBattleGroundMgr()->IsBGSendReinforcements(BATTLEGROUND_AB))
                {
                    sprintf(chr, "|cff0000cc%s [active]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                }
                else if (getBattleGroundMgr()->IsBGSendReinforcements_Ending(BATTLEGROUND_AB))
                {
                    sprintf(chr, "|cff0000cc%s [ending]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARATHI_BASIN));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                    ending = true;
                }

                if (getBattleGroundMgr()->IsBGSendReinforcements(BATTLEGROUND_EY))
                {
                    sprintf(chr, "|cff0000cc%s [active]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                }
                else if (getBattleGroundMgr()->IsBGSendReinforcements_Ending(BATTLEGROUND_EY))
                {
                    sprintf(chr, "|cff0000cc%s [ending]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_EYE_OF_THE_STORM));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
                    ending = true;
                }

                if (getBattleGroundMgr()->IsBGSendReinforcements(BATTLEGROUND_AV))
                {
                    sprintf(chr, "|cff0000cc%s [active]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                }
                else if (getBattleGroundMgr()->IsBGSendReinforcements_Ending(BATTLEGROUND_AV))
                {
                    sprintf(chr, "|cff0000cc%s [ending]|r", pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ALTERAC_VALLEY));
                    pPlayer->ADD_GOSSIP_ITEM(0, chr, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                    ending = true;
                }

                if (ending)
                {
                    pPlayer->SEND_GOSSIP_MENU(68, pCreature->GetGUID());
                    break;
                }
                else // only 1 variant - use it instantly
                {
                    pPlayer->PlayerTalkClass->ClearMenus();
                    pPlayer->CLOSE_GOSSIP_MENU();

                    for (uint32 i = BATTLEGROUND_AV; i <= BATTLEGROUND_EY; ++i)
                    {
                        // skip arenas
                        if (i > BATTLEGROUND_AB && i < BATTLEGROUND_EY)
                            continue;

                        if (getBattleGroundMgr()->IsBGSendReinforcements(BattleGroundTypeId(i)))
                        {
                            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BattleGroundTypeId(i)))
                                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BattleGroundTypeId(i));
                            break;
                        }
                    }
                }
            }
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+2:
        {
            if (!getWorld()->getConfig(CONFIG_ARENA_SPECTATORS_ENABLE))
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_SPECTATE_OFF), pPlayer->GetGUID());
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            }

            // check if should block spectator if 2v2 arena was recently played
            time_t t = time(NULL);
            std::map<std::string, time_t> aps = getWorld()->m_ArenaPlayersIPs;
            if (!aps.empty())
            {
                for (auto val : aps)
                    if (pPlayer->GetSession()->GetRemoteAddress().c_str() == val.first)
                        // not expired
                        if (val.second >= t)
                        {
                            char chr[256];
                            sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SPECTATOR_BLOCKED), pPlayer->GetSession()->secsToTimeString(val.second - t).c_str());
                            pCreature->Whisper(chr, pPlayer->GetGUID());

                            pPlayer->CLOSE_GOSSIP_MENU();
                            return true;
                        }
                        // expired, remove
                        else
                            aps.erase(val.first);
            }

            BattleGroundQueueTypeId bgTypeId1 = pPlayer->GetBattleGroundQueueTypeId(0);
            BattleGroundQueueTypeId bgTypeId2 = pPlayer->GetBattleGroundQueueTypeId(1);
            BattleGroundQueueTypeId bgTypeId3 = pPlayer->GetBattleGroundQueueTypeId(2);
            if (bgTypeId1 == 5 || bgTypeId1 == 6 || bgTypeId1 == 7 || bgTypeId2 == 5 || bgTypeId2 == 6 || bgTypeId2 == 7 || bgTypeId3 == 5 || bgTypeId3 == 6 || bgTypeId3 == 7 ||
                bgTypeId1 == 8 || bgTypeId2 == 8 || bgTypeId3 == 8) // arena
            {    
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_ARENA_QUEUE), pPlayer->GetGUID());    
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            }
            if (bgTypeId1 && bgTypeId2 && bgTypeId3)
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_BG_QUEUE), pPlayer->GetGUID());    
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            }
            //ThereIsArena = false;
            pPlayer->PlayerTalkClass->ClearMenus();

            const MapManager::MapMapType& maps = pPlayer->GetMapsFromScripts();
            uint8 gossipNumber = 20;
            for (uint8 i = 0; i < 3; i++)
            {
                MapManager::MapMapType::const_iterator iter_last = maps.lower_bound(MapID(ArenaIDs[i] + 1));
                for(MapManager::MapMapType::const_iterator mitr = maps.lower_bound(MapID(ArenaIDs[i])); mitr != iter_last && gossipNumber < 35; ++mitr)
                {
                    if (mitr->first.nMapId != ArenaIDs[i])
                    {
                        char chrErr[256];
                        sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), 1);
                        pCreature->Whisper(chrErr, pPlayer->GetGUID());
                        break;
                    }
                    DeathSide_Arena_Spectator_Map_InstId_Manipulation(ArenaIDs[i], mitr->first.nInstanceId, mitr->second, gossipNumber, pPlayer);
                    // gossipNumber increased in function
                }
            }

            //if (!ThereIsArena)
            //{
            //    pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_NO_ARENA_FIGHTS), pPlayer->GetGUID());
            //    pPlayer->CLOSE_GOSSIP_MENU();
            //    return true;
            //}

            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_REFRESH_LIST), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
            pPlayer->SEND_GOSSIP_MENU(68,pCreature->GetGUID());
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+3:
        {
            pPlayer->GetSession()->SendPetitionShowList(pCreature->GetGUID()); // POKUPKA CHARTERA ARENI ILI REGISTRACIYA
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+8:
        {
            // called by pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENAS_BLIZZLIKE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+8);
            // not used at the moment
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AA))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AA); // Arena
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 9:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_WS))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_WS); // WSG
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 10:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AB))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AB); // AB
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 11:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_EY))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_EY); // EYE
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 12:
        {
            if (!pCreature->sendBgNotAvailableByLevel(pPlayer, BATTLEGROUND_AV))
                pPlayer->GetSession()->SendBattlegGroundList(ObjectGuid(pCreature->GetGUID()), BATTLEGROUND_AV); // AV
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 13:
        {
            // Check if player is already in an arena team
            if (!pPlayer->GetArenaTeamId(0))
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_NO_ARENA_TEAM), pPlayer->GetGUID());
                pPlayer->CLOSE_GOSSIP_MENU();
                break;
            }

            // can't queue wihout group
            if (!pPlayer->GetGroup())
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_YOU_NOT_IN_GROUP), pPlayer->GetGUID());
                pPlayer->CLOSE_GOSSIP_MENU();
                break;
            }
            
            WorldPacket Data;
            Data << (uint64)pCreature->GetGUID() << (uint8)0/*2v2 arena slot*/ << (uint8)1/*asGroup*/ << (uint8)1/*isRated*/;
            pPlayer->GetSession()->HandleBattleGroundArenaJoin(Data);
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+55: // Leave Queue
        {
            WorldPacket Data;
            Data << (uint8)0x1 << (uint8)0x0 << (uint32)BATTLEGROUND_AA << (uint16)0x0 << (uint8)0x0;
            pPlayer->GetSession()->HandleBattleGroundPlayerPortOpcode(Data);
            pCreature->MonsterWhisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_YOU_HAVE_LEFT_1V1_QUEUE), pPlayer->GetGUID());
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+58: // duel zone teleport
        {
            pPlayer->CastSpell(pPlayer, 55164, true);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 59: // arena items exchange menu
        {
            uint64 specFlag = 0;
            getBattleGroundMgr()->ArenaRestrictedGetPlayerSpec(pPlayer, specFlag);

            uint32 cnt = 0; // cannot exceed 15 due to interface problems
            for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            {
                Item* pItem = pPlayer->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
                if (pItem)
                {
                    if (getBattleGroundMgr()->IsItemArenaRestricted(pItem->GetEntry(), specFlag))
                    {
                        std::string itemNameLocale; // non-initialized. Initialized in "GetItemNameLocale"
                        const ItemPrototype* pProto = pItem->GetProto();

                        char chr[256];
                        sprintf(chr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ITEM_RESTRICTED), pPlayer->GetItemNameLocale(pProto, &itemNameLocale));
                        pCreature->Whisper(chr, pPlayer->GetGUID());

                        if (pPlayer->ArenaRestrictedCanSwap(pItem->GetGUIDLow()))
                        {
                            // 100000 should be enough, only using blizzlike items here
                            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, pPlayer->GetItemNameLocale(pProto, &itemNameLocale), GOSSIP_SENDER_MAIN, 2000 + i);
                            if (++cnt >= 15)
                                break;
                        }
                    }
                }
            }

            if (cnt)
            {
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 50);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                pPlayer->SEND_GOSSIP_MENU(100000, pCreature->GetGUID());
            }
            else
            {
                pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_RESTRICTION_ALL_SWAPPED), pPlayer->GetGUID());
                DeathSide_pvp_master_Gossip(pPlayer, pCreature, 0, GOSSIP_ACTION_INFO_DEF + 50);
            }
            break;
        }
        case GOSSIP_ACTION_INFO_DEF + 60: // go to main menu
        {
            DeathSide_pvp_master_Hello(pPlayer, pCreature);
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+61: // Arena Ladder. Now support only 1v1 and 2v2
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_SOLO_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 62);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
            pPlayer->SEND_GOSSIP_MENU(990030,pCreature->GetGUID()); // INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES ('990030', 'Choose the bracket you want to view.')
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+62: //3v3 solo ladder
        {
            Ladder_TeamInfo const* ladder3v3 = getBattleGroundMgr()->getLadder(ARENA_TEAM_3v3);

            for(uint32 i = 0; i < LADDER_CNT; i++)
            {
                // no team - break. Means no other teams as well.
                if (!ladder3v3[i].Id)
                    break;

                // there must be a member
                const Ladder_PlayerInfo &member = ladder3v3[i].MembersInfo[0];

                char gossip[256];
                sprintf(gossip, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_1_STATS),
                    i+1, member.PlayerName.c_str(), GetPlRaceName(member.Race).c_str(), GetPlClassName(member.Class).c_str(), member.Wins, member.Loses, member.Rating);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 62); // goes back here again. Click "back" or "main menu" to get out of here
            }

            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 61);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
            pPlayer->SEND_GOSSIP_MENU(990031,pCreature->GetGUID()); // INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES ('990031', 'Position in rating. Name(Race, Class) - Wins / Loses - Rating')
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+63: // 2v2 ladder
        {
            const Ladder_TeamInfo* ladder2v2 = getBattleGroundMgr()->getLadder(ARENA_TEAM_2v2);

            for(uint32 i = 0; i < LADDER_CNT; i++)
            {
                // no team - break. Means no other teams as well.
                if (!ladder2v2[i].Id)
                    break;

                char gossip[256];
                sprintf(gossip, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_2_TEAM_STATS),
                    i+1, ladder2v2[i].TeamName.c_str(), ladder2v2[i].Wins, ladder2v2[i].Loses, ladder2v2[i].Rating);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 64 + i);
            }

            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 61);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
            pPlayer->SEND_GOSSIP_MENU(990032,pCreature->GetGUID()); // INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES ('990032', 'Position in rating. Team name - Wins / Loses - Rating')
            break;
        }
        // 2v2 team members
        case GOSSIP_ACTION_INFO_DEF + 64: 
        case GOSSIP_ACTION_INFO_DEF + 65:
        case GOSSIP_ACTION_INFO_DEF + 66:
        case GOSSIP_ACTION_INFO_DEF + 67:
        case GOSSIP_ACTION_INFO_DEF + 68:
        case GOSSIP_ACTION_INFO_DEF + 69:
        case GOSSIP_ACTION_INFO_DEF + 70:
        case GOSSIP_ACTION_INFO_DEF + 71:
        case GOSSIP_ACTION_INFO_DEF + 72:
        case GOSSIP_ACTION_INFO_DEF + 73:
        {
            const Ladder_TeamInfo &thisTeam = (getBattleGroundMgr()->getLadder(ARENA_TEAM_2v2))[uiAction - (GOSSIP_ACTION_INFO_DEF + 64)];

            for(uint32 i = 0; i < LADDER_MAX_MEMBERS_CNT; i++)
            {
                const Ladder_PlayerInfo &member = thisTeam.MembersInfo[i];
                if (!member.Rating) // faster to check by rating than by name?
                    break; // no other members as well

                char gossip[256];
                sprintf(gossip, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ARENA_LADDER_2_PLAYER), 
                    member.PlayerName.c_str(), GetPlRaceName(member.Race).c_str(), GetPlClassName(member.Class).c_str(), member.Wins, member.Loses, member.Rating);
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, gossip, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 64);
            }

            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_MAIN_MENU), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
            pPlayer->SEND_GOSSIP_MENU(990033,pCreature->GetGUID()); // INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES ('990033', 'Position. Name(Race, Class) - Wins / Loses - Personal Rating')
            break;
        }
        // SOLOQUEUE
        case GOSSIP_ACTION_INFO_DEF + 74:
        {
            Reg3v3ArenaAndCreateTeamIfNeeded(pPlayer, pCreature);
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        // 19 lvl arena
        case GOSSIP_ACTION_INFO_DEF + 75:
        {
            // should never happen
            //if (pPlayer->getLevel() != 19) {
            if (pPlayer->getLevel() < TWINK_LEVEL_MIN || pPlayer->getLevel() > TWINK_LEVEL_MAX) {
                pPlayer->CLOSE_GOSSIP_MENU();
                break;
            }

            uint8 slot = ArenaTeam::GetSlotByType(ARENA_TEAM_2v2);           
            bool isgroup = (pPlayer->GetGroup()) ? true : false;

            WorldPacket Data;
            Data << (uint64)pCreature->GetGUID() << (uint8)slot << (uint8)isgroup/*asGroup*/ << (uint8)0/*isRated*/;
            pPlayer->GetSession()->HandleBattleGroundArenaJoin(Data);
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        // BG stats
        case GOSSIP_ACTION_INFO_DEF + 76:
        {
            uint32 rwins = 0;
            QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT bg_rwins FROM characters_bg WHERE guid = %u", pPlayer->GetGUIDLow());
            if (result)
                rwins = (*result)[0].GetUInt32();

            if (rwins > 10)
                rwins = 10;
            
            char gossip[1024];
            std::string rew;
            uint32 hg_str = LANG_BG_STATS_INNER;

            for (uint32 i = 0; i <= 10; i++)
            {
                if (BG_MC_Rewards[i][1] == 0)
                    rew = pPlayer->GetSession()->GetHellgroundString(LANG_NO_REWARD);
                else
                    rew = std::to_string(BG_MC_Rewards[i][1]) + " " + pPlayer->GetSession()->GetHellgroundString(LANG_MOON_COIN);

                // this is so bad
                if (i >= 10)
                    hg_str = LANG_BG_STATS_INNER_EXT;

                sprintf(gossip, pPlayer->GetSession()->GetHellgroundString(hg_str), i, rew.c_str());
                std::string str = std::string(gossip);

                if (rwins == i)
                {
                    str = "|cff0000cc" + str + "|r";
                }

                pPlayer->ADD_GOSSIP_ITEM(0, str, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 76);
            }

            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_BACK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
            pPlayer->SEND_GOSSIP_MENU(990106, pCreature->GetGUID());
            break;
        }
        default:
        {
            // Show the list of available swaps for this item
            if (uiAction >= 2000 && uiAction < 3000)
            {
                getBattleGroundMgr()->ArenaRestrictedSendSwapList(pCreature, pPlayer, uiAction - 2000);
            }
            // from GOSSIP_ACTION_INFO_DEF+20 to GOSSIP_ACTION_INFO_DEF+34 are arena spectator gossips
            else if (uiAction >= 1020 && ((uiAction - 1020) < 15)) 
            {
                BattleGroundQueueTypeId bgTypeId1 = pPlayer->GetBattleGroundQueueTypeId(0);
                BattleGroundQueueTypeId bgTypeId2 = pPlayer->GetBattleGroundQueueTypeId(1);
                BattleGroundQueueTypeId bgTypeId3 = pPlayer->GetBattleGroundQueueTypeId(2);
                if (bgTypeId1 == 5 || bgTypeId1 == 6 || bgTypeId1 == 7 || bgTypeId2 == 5 || bgTypeId2 == 6 || bgTypeId2 == 7 || bgTypeId3 == 5 || bgTypeId3 == 6 || bgTypeId3 == 7 ||
                    bgTypeId1 == 8 || bgTypeId2 == 8 || bgTypeId3 == 8) // arena
                {
                    pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_ARENA_QUEUE), pPlayer->GetGUID());
                    pPlayer->CLOSE_GOSSIP_MENU();
                    return true;
                }
                if (bgTypeId1 && bgTypeId2 && bgTypeId3)
                {
                    pCreature->Whisper(pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_LEAVE_BG_QUEUE), pPlayer->GetGUID());
                    pPlayer->CLOSE_GOSSIP_MENU();
                    return true;
                }

                DeathSide_Arena_Spectator_Gossip_Manipulation(Arenas[uiAction - 1020].InstanceId, Arenas[uiAction - 1020].ArenaId, pPlayer);
            }
            else
            {
                char chrErr[256];
                sprintf(chrErr, pPlayer->GetSession()->GetHellgroundString(LANG_SCRIPT_ERROR), uiAction);
                pCreature->Whisper(chrErr, pPlayer->GetGUID());
            }
            break;
        }
    }
    return true;
}

void AddSC_DeathSide_pvp_master()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "DeathSide_pvp_master";
    newscript->pGossipHello          = &DeathSide_pvp_master_Hello;
    newscript->pGossipSelect          = &DeathSide_pvp_master_Gossip;
    newscript->RegisterSelf();
}
