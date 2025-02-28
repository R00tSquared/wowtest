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

/* ScriptData
SDName: Npcs_Special
SD%Complete: 100
SDComment: To be used for special NPCs that are located globally. Support for quest 3861 (Cluck!), 6622 and 6624 (Triage)
SDCategory: NPCs
EndScriptData
*/

/* ContentData
npc_chicken_cluck           100%    support for quest 3861 (Cluck!)
npc_dancing_flames          100%    midsummer event NPC
npc_guardian                100%    guardianAI used to prevent players from accessing off-limits areas. Not in use by SD2
npc_injured_patient         100%    patients for triage-quests (6622 and 6624)
npc_doctor                  100%    Gustaf Vanhowzen and Gregory Victor, quest 6622 and 6624 (Triage)
npc_mount_vendor            100%    Regular mount vendors all over the world. Display gossip if player doesn't meet the requirements to buy
npc_rogue_trainer           80%     Scripted trainers, so they are able to offer item 17126 for class quest 6681
npc_sayge                   100%    Darkmoon event fortune teller, buff player based on answers given
npc_snake_trap_serpents     80%     AI for snakes that summoned by Snake Trap
npc_flight_master           100%    AI for flight masters.
npc_lazy_peon               100%    AI for peons for quest 5441 (Lazy Peons)
npc_mojo                    100%    AI for companion Mojo (summoned by item: 33993)
npc_master_omarion          100%    Master Craftsman Omarion, patterns menu
npc_lorekeeper_lydros       100%    Dialogue (story) + add A Dull and Flat Elven Blade
npc_crashin_thrashin_robot  100%    AI for Crashin' Thrashin' Robot from engineering
npc_gnomish_flame_turret
npc_soul_trader_beacon
EndContentData */

#include "precompiled.h"
#include "BattleGround.h"
#include "Totem.h"
#include "PetAI.h"
#include "Language.h"
#include "ObjectMgr.h"
#include <list>
#include "Guild.h"
#include "GuildMgr.h"
#include <cstring>
#include "scripts/Eastern Kingdoms/karazhan/def_karazhan.h"

/*########
# npc_chicken_cluck
#########*/

#define QUEST_CLUCK         3861
#define CHICKEN_HELLO_TEXT  50
#define EMOTE_A_HELLO       -1200176
#define EMOTE_H_HELLO       -1200177
#define CLUCK_TEXT2         -1200178
#define FACTION_FRIENDLY    84
#define FACTION_CHICKEN     31

#define SPELL_LIGHTNING_BOLT 55409 // ~1150 damage
#define SPELL_HEAL 55188 // ~1700 heal
#define SPELL_DISENGAGE 55189
#define SPELL_RESTORE_MANA 28722 // restores mana with castCustomSpell BP
#define SPELL_CHAIN_LIGHTNING 55408

struct npc_chicken_cluckAI : public ScriptedAI
{
    npc_chicken_cluckAI(Creature *c) : ScriptedAI(c) {}

    Timer_UnCheked ResetFlagTimer;

    void Reset()
    {
        ResetFlagTimer.Reset(120000);

        me->setFaction(FACTION_CHICKEN);
        me->RemoveFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        // Reset flags after a certain time has passed so that the next player has to start the 'event' again
        if(me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
        {
            if(ResetFlagTimer.Expired(diff))
            {
                EnterEvadeMode();
                return;
            }
        }

        if(UpdateVictim())
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_chicken_cluck(Creature *_Creature)
{
    return new npc_chicken_cluckAI(_Creature);
}

bool ReceiveEmote_npc_chicken_cluck( Player *player, Creature *_Creature, uint32 emote )
{
    if( emote == TEXTEMOTE_CHICKEN )
    {
        if( player->GetTeam() == ALLIANCE )
        {
            if( rand()%30 == 1 )
            {
                if( player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_NONE )
                {
                    _Creature->SetFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
                    _Creature->setFaction(FACTION_FRIENDLY);
                    _Creature->MonsterTextEmote(-1200176, 0);
                }
            }
        }
        else
            _Creature->MonsterTextEmote(-1200177,0);
    }
    if( emote == TEXTEMOTE_CHEER && player->GetTeam() == ALLIANCE )
        if( player->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_COMPLETE )
        {
            _Creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            _Creature->setFaction(FACTION_FRIENDLY);
            _Creature->MonsterTextEmote(-1200178, 0);
        }

    return true;
}

bool GossipHello_npc_chicken_cluck(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    player->SEND_GOSSIP_MENU(CHICKEN_HELLO_TEXT, _Creature->GetGUID());

    return true;
}

bool QuestAccept_npc_chicken_cluck(Player *player, Creature *_Creature, const Quest *_Quest )
{
    if(_Quest->GetQuestId() == QUEST_CLUCK)
        ((npc_chicken_cluckAI*)_Creature->AI())->Reset();

    return true;
}

bool QuestComplete_npc_chicken_cluck(Player *player, Creature *_Creature, const Quest *_Quest)
{
    if(_Quest->GetQuestId() == QUEST_CLUCK)
    {
        _Creature->CastSpell(_Creature, 13563, false);  // summon chicken egg as reward
        ((npc_chicken_cluckAI*)_Creature->AI())->Reset();
    }

    return true;
}

/*######
## Triage quest
######*/

#define SAY_DOC1 -1200179
//#define SAY_DOC2 "HOORAY! I AM SAVED!"
//#define SAY_DOC3 "Sweet, sweet embrace... take me..."

struct Location
{
    float x, y, z, o;
};

#define DOCTOR_ALLIANCE     12939

static Location AllianceCoords[]=
{
    {
        // Top-far-right bunk as seen from entrance
        -3757.38, -4533.05, 14.16, 3.62
    },
    {
        // Top-far-left bunk
        -3754.36, -4539.13, 14.16, 5.13
    },
    {
        // Far-right bunk
        -3749.54, -4540.25, 14.28, 3.34
    },
    {
        // Right bunk near entrance
        -3742.10, -4536.85, 14.28, 3.64
    },
    {
        // Far-left bunk
        -3755.89, -4529.07, 14.05, 0.57
    },
    {
        // Mid-left bunk
        -3749.51, -4527.08, 14.07, 5.26
    },
    {
        // Left bunk near entrance
        -3746.37, -4525.35, 14.16, 5.22
    },
};

#define ALLIANCE_COORDS     7

//alliance run to where
#define A_RUNTOX -3742.96
#define A_RUNTOY -4531.52
#define A_RUNTOZ 11.91

#define DOCTOR_HORDE    12920

static Location HordeCoords[]=
{
    {
        // Left, Behind
        -1013.75, -3492.59, 62.62, 4.34
    },
    {
        // Right, Behind
        -1017.72, -3490.92, 62.62, 4.34
    },
    {
        // Left, Mid
        -1015.77, -3497.15, 62.82, 4.34
    },
    {
        // Right, Mid
        -1019.51, -3495.49, 62.82, 4.34
    },
    {
        // Left, front
        -1017.25, -3500.85, 62.98, 4.34
    },
    {
        // Right, Front
        -1020.95, -3499.21, 62.98, 4.34
    }
};

#define HORDE_COORDS        6

//horde run to where
#define H_RUNTOX -1016.44
#define H_RUNTOY -3508.48
#define H_RUNTOZ 62.96

const uint32 AllianceSoldierId[3] =
{
    12938,                                                  // 12938 Injured Alliance Soldier
    12936,                                                  // 12936 Badly injured Alliance Soldier
    12937                                                   // 12937 Critically injured Alliance Soldier
};

const uint32 HordeSoldierId[3] =
{
    12923,                                                  //12923 Injured Soldier
    12924,                                                  //12924 Badly injured Soldier
    12925                                                   //12925 Critically injured Soldier
};

/*######
## npc_doctor (handles both Gustaf Vanhowzen and Gregory Victor)
######*/

struct npc_doctorAI : public ScriptedAI
{
    npc_doctorAI(Creature *c) : ScriptedAI(c) {}

    uint64 Playerguid;

    Timer_UnCheked SummonPatient_Timer;
    uint32 SummonPatientCount;
    uint32 PatientDiedCount;
    uint32 PatientSavedCount;

    bool Event;

    std::list<uint64> Patients;
    std::vector<Location*> Coordinates;

    void Reset()
    {
        Event = false;
        SummonPatient_Timer.Reset(10000);
        PatientSavedCount = 0;
        PatientDiedCount = 0;
        Playerguid = 0;
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void BeginEvent(Player* player);
    void PatientDied(Location* Point);
    void PatientSaved(Creature* soldier, Player* player, Location* Point);
    void UpdateAI(const uint32 diff);
};

/*#####
## npc_injured_patient (handles all the patients, no matter Horde or Alliance)
#####*/

struct npc_injured_patientAI : public ScriptedAI
{
    npc_injured_patientAI(Creature *c) : ScriptedAI(c) {}

    uint64 Doctorguid;
    Timer_UnCheked BleedTimer;
    uint8  Status;
    Location* Coord;

    void Reset()
    {
        Doctorguid = 0;
        BleedTimer.Reset(1000);
        Coord = NULL;
        //no select
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        //no regen health
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        //to make them lay with face down
        me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);

        uint32 mobId = me->GetEntry();

        switch (mobId)
        {
            //lower max health
        case 12923:
        case 12938:                                     //Injured Soldier
            me->SetHealth(uint32(me->GetMaxHealth()*.75));
            Status = 75;
            break;
        case 12924:
        case 12936:                                     //Badly injured Soldier
            me->SetHealth(uint32(me->GetMaxHealth()*.50));
            Status = 50;
            break;
        case 12925:
        case 12937:                                     //Critically injured Soldier
            me->SetHealth(uint32(me->GetMaxHealth()*.25));
            Status = 25;
            break;
        }
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (caster->GetTypeId() == TYPEID_PLAYER && me->isAlive() && spell->Id == 20804)
        {
            if( (((Player*)caster)->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (((Player*)caster)->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
            {
                if(Doctorguid)
                {
                    Creature* Doctor = Unit::GetCreature((*me), Doctorguid);
                    if(Doctor)
                        ((npc_doctorAI*)Doctor->AI())->PatientSaved(me, ((Player*)caster), Coord);
                }
            }
            //make not selectable
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            //regen health
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            //stand up
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
            DoSay(-1200179,LANG_UNIVERSAL,NULL);

            uint32 mobId = me->GetEntry();
            me->SetWalk(false);
            switch (mobId)
            {
            case 12923:
            case 12924:
            case 12925:
                me->GetMotionMaster()->MovePoint(0, H_RUNTOX, H_RUNTOY, H_RUNTOZ);
                break;
            case 12936:
            case 12937:
            case 12938:
                me->GetMotionMaster()->MovePoint(0, A_RUNTOX, A_RUNTOY, A_RUNTOZ);
                break;
            }
        }
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->isAlive())
            return;

        if (BleedTimer.Expired(diff))
        {
            Status--;
            if (Status > 0)
            {
                me->SetHealth(uint32(me->GetMaxHealth()* Status / 100));
                BleedTimer = 650; // guess, gives about 15 secs for criticaly injured ones
            }
            else
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->setDeathState(JUST_DIED);
                me->SetFlag(UNIT_DYNAMIC_FLAGS, 32);
                if (Doctorguid)
                {
                    Creature* Doctor = (Unit::GetCreature((*me), Doctorguid));
                    if (Doctor)
                        ((npc_doctorAI*)Doctor->AI())->PatientDied(Coord);
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_injured_patient(Creature *_Creature)
{
    return new npc_injured_patientAI (_Creature);
}

/*
npc_doctor (continue)
*/

void npc_doctorAI::BeginEvent(Player* player)
{
    Playerguid = player->GetGUID();

    SummonPatient_Timer = 10000;
    SummonPatientCount = 0;
    PatientDiedCount = 0;
    PatientSavedCount = 0;

    switch(me->GetEntry())
    {
    case DOCTOR_ALLIANCE:
        for(uint8 i = 0; i < ALLIANCE_COORDS; ++i)
            Coordinates.push_back(&AllianceCoords[i]);
        break;

    case DOCTOR_HORDE:
        for(uint8 i = 0; i < HORDE_COORDS; ++i)
            Coordinates.push_back(&HordeCoords[i]);
        break;
    }

    Event = true;

    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
}

void npc_doctorAI::PatientDied(Location* Point)
{
    Player* player = Unit::GetPlayerInWorld(Playerguid);
    if(player && ((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)))
    {
        PatientDiedCount++;
        if (PatientDiedCount > 5 && Event)
        {
            if(player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(6624);
            else if(player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                player->FailQuest(6622);

            Event = false;
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        Coordinates.push_back(Point);
    }
    else
        Reset();
}

void npc_doctorAI::PatientSaved(Creature* soldier, Player* player, Location* Point)
{
    if(player && Playerguid == player->GetGUID())
    {
        if((player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
        {
            PatientSavedCount++;
            if(PatientSavedCount == 15)
            {
                if(!Patients.empty())
                {
                    std::list<uint64>::iterator itr;
                    for(itr = Patients.begin(); itr != Patients.end(); ++itr)
                    {
                        Creature* Patient = Unit::GetCreature((*me), *itr);
                        if( Patient )
                            Patient->setDeathState(JUST_DIED);
                    }
                }

                if(player->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                    player->AreaExploredOrEventHappens(6624);
                else if(player->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                    player->AreaExploredOrEventHappens(6622);

                Reset();
            }

            Coordinates.push_back(Point);
        }
    }
    else
        Reset();
}

void npc_doctorAI::UpdateAI(const uint32 diff)
{
    if(Event && SummonPatientCount >= 20)
        Reset();

    if(Event)
        if(SummonPatient_Timer.Expired(diff))
        {
            Creature* Patient = NULL;
            Location* Point = NULL;

            if(Coordinates.empty())
                return;

            std::vector<Location*>::iterator itr = Coordinates.begin()+rand()%Coordinates.size();
            uint32 patientEntry = 0;

            switch(me->GetEntry())
            {
            case DOCTOR_ALLIANCE:
                patientEntry = AllianceSoldierId[rand()%3];
                break;
            case DOCTOR_HORDE:
                patientEntry = HordeSoldierId[rand()%3];
                break;
            default:
                error_log("TSCR: Invalid entry for Triage doctor. Please check your database");
                return;
            }

            Point = *itr;

            Patient = me->SummonCreature(patientEntry, Point->x, Point->y, Point->z, Point->o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);

            if(Patient)
            {
                Patients.push_back(Patient->GetGUID());
                ((npc_injured_patientAI*)Patient->AI())->Doctorguid = me->GetGUID();
                if(Point)
                    ((npc_injured_patientAI*)Patient->AI())->Coord = Point;
                Coordinates.erase(itr);
            }
            SummonPatient_Timer = 10000;
            SummonPatientCount++;
        }
}

bool QuestAccept_npc_doctor(Player *player, Creature *creature, Quest const *quest )
{
    if((quest->GetQuestId() == 6624) || (quest->GetQuestId() == 6622))
        ((npc_doctorAI*)creature->AI())->BeginEvent(player);

    return true;
}

CreatureAI* GetAI_npc_doctor(Creature *_Creature)
{
    return new npc_doctorAI (_Creature);
}

/*######
## npc_guardian
######*/

#define SPELL_DEATHTOUCH                5
#define SAY_AGGRO                       -1200180

struct npc_guardianAI : public ScriptedAI
{
    npc_guardianAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
    }

    void EnterCombat(Unit *who)
    {
        DoYell(-1200180, LANG_UNIVERSAL, NULL);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (me->isAttackReady())
        {
            me->CastSpell(me->GetVictim(), SPELL_DEATHTOUCH, true);
            me->resetAttackTimer();
        }
    }
};

CreatureAI* GetAI_npc_guardian(Creature *_Creature)
{
    return new npc_guardianAI (_Creature);
}

/*######
## npc_mount_vendor
######*/

bool GossipHello_npc_mount_vendor(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    bool canBuy;
    canBuy = false;
    uint32 vendor = _Creature->GetEntry();
    uint8 race = player->GetRace();

    switch (vendor)
    {
    case 384:                                           //Katie Hunter
    case 1460:                                          //Unger Statforth
    case 2357:                                          //Merideth Carlson
    case 4885:                                          //Gregor MacVince
        if (player->GetReputationMgr().GetRank(72) != REP_EXALTED && race != RACE_HUMAN)
            player->SEND_GOSSIP_MENU(5855, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 1261:                                          //Veron Amberstill
        if (player->GetReputationMgr().GetRank(47) != REP_EXALTED && race != RACE_DWARF)
            player->SEND_GOSSIP_MENU(5856, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 3362:                                          //Ogunaro Wolfrunner
        if (player->GetReputationMgr().GetRank(76) != REP_EXALTED && race != RACE_ORC)
            player->SEND_GOSSIP_MENU(5841, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 3685:                                          //Harb Clawhoof
        if (player->GetReputationMgr().GetRank(81) != REP_EXALTED && race != RACE_TAUREN)
            player->SEND_GOSSIP_MENU(5843, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 4730:                                          //Lelanai
        if (player->GetReputationMgr().GetRank(69) != REP_EXALTED && race != RACE_NIGHTELF)
            player->SEND_GOSSIP_MENU(5844, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 4731:                                          //Zachariah Post
        if (player->GetReputationMgr().GetRank(68) != REP_EXALTED && race != RACE_UNDEAD_PLAYER)
            player->SEND_GOSSIP_MENU(5840, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 7952:                                          //Zjolnir
        if (player->GetReputationMgr().GetRank(530) != REP_EXALTED && race != RACE_TROLL)
            player->SEND_GOSSIP_MENU(5842, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 7955:                                          //Milli Featherwhistle
        if (player->GetReputationMgr().GetRank(54) != REP_EXALTED && race != RACE_GNOME)
            player->SEND_GOSSIP_MENU(5857, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 16264:                                         //Winaestra
        if (player->GetReputationMgr().GetRank(911) != REP_EXALTED && race != RACE_BLOODELF)
            player->SEND_GOSSIP_MENU(10305, _Creature->GetGUID());
        else canBuy = true;
        break;
    case 17584:                                         //Torallius the Pack Handler
        if (player->GetReputationMgr().GetRank(930) != REP_EXALTED && race != RACE_DRAENEI)
            player->SEND_GOSSIP_MENU(10239, _Creature->GetGUID());
        else canBuy = true;
        break;
    }

    if (canBuy)
    {
        if (_Creature->isVendor())
            player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_mount_vendor(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*######
## npc_rogue_trainer
######*/

bool GossipHello_npc_rogue_trainer(Player *player, Creature *_Creature)
{
    _Creature->prepareGossipMenu(player); // why to rewrite other function? just add new line if nessessary

    if( player->GetClass() == CLASS_ROGUE && player->GetLevel() >= 24 && !player->HasItemCount(17126,1) && !player->GetQuestRewardStatus(6681) )
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16086), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(5996, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_rogue_trainer(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF +1:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, 21100, false);
        break;
    case GOSSIP_OPTION_TRAINER:
        player->SEND_TRAINERLIST(_Creature->GetGUID());
        break;
    case GOSSIP_OPTION_UNLEARNTALENTS:
        player->CLOSE_GOSSIP_MENU();
        player->SendTalentWipeConfirm(_Creature->GetGUID());
        break;
    }
    // TODO: returning false should force core to handle casual options normal way, it does not
    return true;
}

/*######
## npc_sayge
######*/

#define SPELL_DMG       23768                               //dmg
#define SPELL_RES       23769                               //res
#define SPELL_ARM       23767                               //arm
#define SPELL_SPI       23738                               //spi
#define SPELL_INT       23766                               //int
#define SPELL_STM       23737                               //stm
#define SPELL_STR       23735                               //str
#define SPELL_AGI       23736                               //agi
#define SPELL_FORTUNE   23765                               //faire fortune

bool GossipHello_npc_sayge(Player *player, Creature *_Creature)
{
    if(_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->HasSpellCooldown(SPELL_INT) ||
            player->HasSpellCooldown(SPELL_ARM) ||
            player->HasSpellCooldown(SPELL_DMG) ||
            player->HasSpellCooldown(SPELL_RES) ||
            player->HasSpellCooldown(SPELL_STR) ||
            player->HasSpellCooldown(SPELL_AGI) ||
            player->HasSpellCooldown(SPELL_STM) ||
            player->HasSpellCooldown(SPELL_SPI) )
        player->SEND_GOSSIP_MENU(7393, _Creature->GetGUID());
    else
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16087), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(7339, _Creature->GetGUID());
    }

    return true;
}

void SendAction_npc_sayge(Player *player, Creature *_Creature, uint32 action)
{
    switch(action)
    {
    case GOSSIP_ACTION_INFO_DEF+1:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16532), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16533), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16534), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16535), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
        player->SEND_GOSSIP_MENU(7340, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+2:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16536), GOSSIP_SENDER_MAIN+1, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16537), GOSSIP_SENDER_MAIN+2, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16538), GOSSIP_SENDER_MAIN+3, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(7341, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+3:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16539), GOSSIP_SENDER_MAIN+4, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16540), GOSSIP_SENDER_MAIN+5, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16541), GOSSIP_SENDER_MAIN+2, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(7361, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+4:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16542), GOSSIP_SENDER_MAIN+6, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16543), GOSSIP_SENDER_MAIN+7, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16544), GOSSIP_SENDER_MAIN+8, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(7362, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+5:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16545), GOSSIP_SENDER_MAIN+5, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16546), GOSSIP_SENDER_MAIN+4, GOSSIP_ACTION_INFO_DEF);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16547), GOSSIP_SENDER_MAIN+3, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(7363, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16548), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
        player->SEND_GOSSIP_MENU(7364, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+6:
        _Creature->CastSpell(player, SPELL_FORTUNE, false);
        player->SEND_GOSSIP_MENU(7365, _Creature->GetGUID());
        break;
    }
}

bool GossipSelect_npc_sayge(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch(sender)
    {
    case GOSSIP_SENDER_MAIN:
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+1:
        _Creature->CastSpell(player, SPELL_DMG, false);
        player->AddSpellCooldown(SPELL_DMG,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+2:
        _Creature->CastSpell(player, SPELL_RES, false);
        player->AddSpellCooldown(SPELL_RES,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+3:
        _Creature->CastSpell(player, SPELL_ARM, false);
        player->AddSpellCooldown(SPELL_ARM,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+4:
        _Creature->CastSpell(player, SPELL_SPI, false);
        player->AddSpellCooldown(SPELL_SPI,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+5:
        _Creature->CastSpell(player, SPELL_INT, false);
        player->AddSpellCooldown(SPELL_INT,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+6:
        _Creature->CastSpell(player, SPELL_STM, false);
        player->AddSpellCooldown(SPELL_STM,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+7:
        _Creature->CastSpell(player, SPELL_STR, false);
        player->AddSpellCooldown(SPELL_STR,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    case GOSSIP_SENDER_MAIN+8:
        _Creature->CastSpell(player, SPELL_AGI, false);
        player->AddSpellCooldown(SPELL_AGI,time(NULL) + 7200);
        SendAction_npc_sayge(player, _Creature, action);
        break;
    }
    return true;
}

#define SPELL_TONK_MINE_DETONATE 25099
#define NPC_STEAM_TONK 19405

struct npc_tonk_mineAI : public ScriptedAI
{
    npc_tonk_mineAI(Creature *c) : ScriptedAI(c)
    {
        me->SetReactState(REACT_PASSIVE);
    }

    Timer_UnCheked ArmingTimer;
    Timer_UnCheked CheckTimer;

    void Reset()
    {
        ArmingTimer.Reset(3000);
        CheckTimer.Reset(1000);
    }

    void EnterCombat(Unit *who) {}
    void AttackStart(Unit *who) {}
    void MoveInLineOfSight(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if (ArmingTimer.GetInterval())
        {
            if (ArmingTimer.Expired(diff))
                ArmingTimer = 0;
        }
        else if (CheckTimer.Expired(diff))
        {
            if(GetClosestCreatureWithEntry(me, NPC_STEAM_TONK, 2))
            {
                me->CastSpell(me, SPELL_TONK_MINE_DETONATE, true);
                me->setDeathState(DEAD);
            }
            CheckTimer = 1000;
        }
    }
};

CreatureAI* GetAI_npc_tonk_mine(Creature *_Creature)
{
    return new npc_tonk_mineAI(_Creature);
}

/*####
## npc_winter_reveler
####*/

bool ReceiveEmote_npc_winter_reveler( Player *player, Creature *_Creature, uint32 emote )
{
    if ((!player->HasAura(26218))&&(emote == TEXTEMOTE_KISS))
    {
        _Creature->CastSpell(player, 26218, false);
        return true;
    }

    return false;
}

/*####
## npc_snake_trap_serpents
####*/

#define SPELL_MIND_NUMBING_POISON  8692    // Viper
#define SPELL_DEADLY_POISON        34655   // Venomous Snake
#define SPELL_CRIPPLING_POISON     3409    // Viper

#define SNAKE_VIPER                19921

struct npc_snake_trap_serpentsAI : public ScriptedAI
{
    npc_snake_trap_serpentsAI(Creature *c) : ScriptedAI(c) { me->SetAggroRange(15.0f); me->SetIsDistanceToHomeEvadable(false); }

    Timer_UnCheked checkTimer;

    void EnterCombat(Unit*)
    {
        if (roll_chance_f(66.0f))
        {
            SetAutocast(me->GetEntry() == SNAKE_VIPER ? RAND(SPELL_MIND_NUMBING_POISON, SPELL_CRIPPLING_POISON) : SPELL_DEADLY_POISON, urand(1000, 3000));
            StartAutocast();
        }

        checkTimer.Reset(2000);

        me->SetReactState(REACT_AGGRESSIVE);
        me->setAttackTimer(BASE_ATTACK, urand(500, 1500));
    }

    bool UpdateVictim()
    {
        if (ScriptedAI::UpdateVictim())
            return true;

        if (Unit* target = me->SelectNearestTarget(15.0f))
            AttackStart(target);

        return me->GetVictim();
    }

    void UpdateAI(const uint32 diff)
    {
        if (checkTimer.Expired(diff))
        {
            Unit* owner = me->GetOwner();
            if (!owner || !owner->IsInMap(me))
            {
                me->ForcedDespawn();
                return;
            }
            checkTimer = 2000;
        }

        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady(diff);
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_snake_trap_serpents(Creature *_Creature)
{
    return new npc_snake_trap_serpentsAI(_Creature);
}

/*################
# Flight Master  #
################*/

uint32 ADVISOR[] =
{
    9297,      // ENRAGED_WYVERN
    9526,      // ENRAGED_GRYPHON
    9521,      // ENRAGED_FELBAT
    9527,      // ENRAGED_HIPPOGRYPH
    27946      // SILVERMOON_DRAGONHAWK
};
const char type[] = "WGBHD";

struct npc_flight_masterAI : public ScriptedAI
{
    npc_flight_masterAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}
    void SummonAdvisor()
    {
        const char *subname = me->GetSubName();
        for(uint8 i = 0; i<5; i++)
        {
            if(subname[0] == type[i])
            {
                DoSpawnCreature(ADVISOR[i], 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1);
                DoSpawnCreature(ADVISOR[i], 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1);
                break;
            }
        }
    }

    void JustSummoned(Creature *add)
    {
        if(add)
        {
            add->setFaction(me->getFaction());
            add->SetLevel(me->GetLevel());
            add->AI()->AttackStart(me->GetVictim());
        }
    }

    void EnterCombat(Unit *who)
    {
        SummonAdvisor();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_flight_master(Creature *_Creature)
{
    return new npc_flight_masterAI(_Creature);
}

/*######
## npc_garments_of_quests
######*/

enum
{
    SPELL_LESSER_HEAL_R2    = 2052,
    SPELL_FORTITUDE_R1      = 1243,

    QUEST_MOON              = 5621,
    QUEST_LIGHT_1           = 5624,
    QUEST_LIGHT_2           = 5625,
    QUEST_SPIRIT            = 5648,
    QUEST_DARKNESS          = 5650,

    ENTRY_SHAYA             = 12429,
    ENTRY_ROBERTS           = 12423,
    ENTRY_DOLF              = 12427,
    ENTRY_KORJA             = 12430,
    ENTRY_DG_KEL            = 12428,

    SAY_COMMON_HEALED       = -1000531,
    SAY_DG_KEL_THANKS       = -1000532,
    SAY_DG_KEL_GOODBYE      = -1000533,
    SAY_ROBERTS_THANKS      = -1000556,
    SAY_ROBERTS_GOODBYE     = -1000557,
    SAY_KORJA_THANKS        = -1000558,
    SAY_KORJA_GOODBYE       = -1000559,
    SAY_DOLF_THANKS         = -1000560,
    SAY_DOLF_GOODBYE        = -1000561,
    SAY_SHAYA_THANKS        = -1000562,
    SAY_SHAYA_GOODBYE       = -1000563,
};

float RunTo[5][3]=
{
    {9661.724, 869.803, 1270.742},                          //shaya
    {-9543.747, -117.770, 57.893},                          //roberts
    {-5650.226, -473.517, 397.027},                         //dolf
    {189.175, -4747.069, 11.215},                           //kor'ja
    {2471.303, 371.101, 30.919},                            //kel
};

struct npc_garments_of_questsAI : public ScriptedAI
{
    npc_garments_of_questsAI(Creature *c) : ScriptedAI(c)
    {
    }

    uint64 caster;

    bool IsHealed;
    bool CanRun;

    Timer RunAwayTimer;

    void Reset()
    {
        caster = 0;
        IsHealed = false;
        CanRun = false;

        RunAwayTimer.Reset(5000);

        me->SetStandState(UNIT_STAND_STATE_KNEEL);
        me->SetHealth(int(me->GetMaxHealth()*0.7));
    }

    void EnterCombat(Unit *who) {}

    void SpellHit(Unit* pCaster, const SpellEntry *Spell)
    {
        if(Spell->Id == SPELL_LESSER_HEAL_R2 || Spell->Id == SPELL_FORTITUDE_R1)
        {
            //not while in combat
            if(me->IsInCombat())
                return;

            //nothing to be done now
            if(IsHealed && CanRun)
                return;

            if(pCaster->GetTypeId() == TYPEID_PLAYER)
            {
                switch(me->GetEntry())
                {
                case ENTRY_SHAYA:
                    if (((Player*)pCaster)->GetQuestStatus(QUEST_MOON) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                        {
                            DoScriptText(SAY_SHAYA_THANKS,me,pCaster);
                            CanRun = true;
                        }
                        else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                        {
                            caster = pCaster->GetGUID();
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            DoScriptText(SAY_COMMON_HEALED,me,pCaster);
                            IsHealed = true;
                        }
                    }
                    break;
                case ENTRY_ROBERTS:
                    if (((Player*)pCaster)->GetQuestStatus(QUEST_LIGHT_1) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                        {
                            DoScriptText(SAY_ROBERTS_THANKS,me,pCaster);
                            CanRun = true;
                        }
                        else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                        {
                            caster = pCaster->GetGUID();
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            DoScriptText(SAY_COMMON_HEALED,me,pCaster);
                            IsHealed = true;
                        }
                    }
                    break;
                case ENTRY_DOLF:
                    if (((Player*)pCaster)->GetQuestStatus(QUEST_LIGHT_2) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                        {
                            DoScriptText(SAY_DOLF_THANKS,me,pCaster);
                            CanRun = true;
                        }
                        else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                        {
                            caster = pCaster->GetGUID();
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            DoScriptText(SAY_COMMON_HEALED,me,pCaster);
                            IsHealed = true;
                        }
                    }
                    break;
                case ENTRY_KORJA:
                    if (((Player*)pCaster)->GetQuestStatus(QUEST_SPIRIT) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                        {
                            DoScriptText(SAY_KORJA_THANKS,me,pCaster);
                            CanRun = true;
                        }
                        else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                        {
                            caster = pCaster->GetGUID();
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            DoScriptText(SAY_COMMON_HEALED,me,pCaster);
                            IsHealed = true;
                        }
                    }
                    break;
                case ENTRY_DG_KEL:
                    if (((Player*)pCaster)->GetQuestStatus(QUEST_DARKNESS) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                        {
                            DoScriptText(SAY_DG_KEL_THANKS,me,pCaster);
                            CanRun = true;
                        }
                        else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                        {
                            caster = pCaster->GetGUID();
                            me->SetStandState(UNIT_STAND_STATE_STAND);
                            DoScriptText(SAY_COMMON_HEALED,me,pCaster);
                            IsHealed = true;
                        }
                    }
                    break;
                }

                //give quest credit, not expect any special quest objectives
                if (CanRun)
                    ((Player*)pCaster)->TalkedToCreature(me->GetEntry(),me->GetGUID());
            }
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        //we reached destination, kill ourselves
        if (id == 1)
        {
            me->DisappearAndDie();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (CanRun && !me->IsInCombat())
        {
            if (RunAwayTimer.Expired(diff))
            {
                if (Unit *pUnit = Unit::GetUnit(*me,caster))
                {
                    switch(me->GetEntry())
                    {
                    case ENTRY_SHAYA:
                        DoScriptText(SAY_SHAYA_GOODBYE,me,pUnit);
                        me->GetMotionMaster()->MovePoint(1, RunTo[0][0], RunTo[0][1], RunTo[0][2]);
                        break;
                    case ENTRY_ROBERTS:
                        DoScriptText(SAY_ROBERTS_GOODBYE,me,pUnit);
                        me->GetMotionMaster()->MovePoint(1, RunTo[1][0], RunTo[1][1], RunTo[1][2]);
                        break;
                    case ENTRY_DOLF:
                        DoScriptText(SAY_DOLF_GOODBYE,me,pUnit);
                        me->GetMotionMaster()->MovePoint(1, RunTo[2][0], RunTo[2][1], RunTo[2][2]);
                        break;
                    case ENTRY_KORJA:
                        DoScriptText(SAY_KORJA_GOODBYE,me,pUnit);
                        me->GetMotionMaster()->MovePoint(1, RunTo[3][0], RunTo[3][1], RunTo[3][2]);
                        break;
                    case ENTRY_DG_KEL:
                        DoScriptText(SAY_DG_KEL_GOODBYE,me,pUnit);
                        me->GetMotionMaster()->MovePoint(1, RunTo[4][0], RunTo[4][1], RunTo[4][2]);
                        break;
                    }
                }
                else
                    EnterEvadeMode();                       //something went wrong

                RunAwayTimer = 30000;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_garments_of_quests(Creature* pCreature)
{
    return new npc_garments_of_questsAI(pCreature);
}

/*########
# npc_mojo
#########*/

#define SPELL_FEELING_FROGGY    43906
#define SPELL_HEARTS            20372   //wrong ?
#define MOJO_WHISPS_COUNT       8

struct npc_mojoAI : public ScriptedAI
{
    npc_mojoAI(Creature *c) : ScriptedAI(c) {}

    Timer_UnCheked heartsResetTimer;
    bool hearts;

    void Reset()
    {
        heartsResetTimer.Reset(15000);
        hearts = false;
        me->GetMotionMaster()->MoveFollow(me->GetOwner(), 2.0, M_PI/2);
    }

    void EnterCombat(Unit *who) {}

    void OnAuraApply(Aura* aur, Unit* caster, bool stackApply)
    {
        if (aur->GetId() == SPELL_HEARTS)
        {
            hearts = true;
            heartsResetTimer = 15000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (hearts)
        {
            if (heartsResetTimer.Expired(diff))
            {
                me->RemoveAurasDueToSpell(SPELL_HEARTS);
                hearts = false;
                me->GetMotionMaster()->MoveFollow(me->GetOwner(), 2.0, M_PI/2);
                me->SetSelection(0);
            }
        }
    }
};

bool ReceiveEmote_npc_mojo( Player *player, Creature *_Creature, uint32 emote )
{
    if( emote == TEXTEMOTE_KISS )
    {
        if (!_Creature->HasAura(SPELL_HEARTS, 0))
        {
            //affect only the same conflict side (horde -> horde or ally -> ally)
            if( player->GetTeam() == _Creature->GetCharmerOrOwnerPlayerOrPlayerItself()->GetTeam() )
            {
                player->CastSpell(player, SPELL_FEELING_FROGGY, false);
                _Creature->CastSpell(_Creature, SPELL_HEARTS, false);
                _Creature->SetSelection(player->GetGUID());

                _Creature->GetMotionMaster()->MoveFollow(player, 1.0, 0);

                int32 text;

                switch (urand(0, MOJO_WHISPS_COUNT))
                {
                case 0:
                    text = -1200676;
                    break;
                case 1:
                    text = -1200677;
                    break;
                case 2:
                    text = -1200678;
                    break;
                case 3:
                    text = -1200679;
                    break;
                case 4:
                    text = -1200680;
                    break;
                case 5:
                    text = -1200681;
                    break;
                case 6:
                    text = -1200682;
                    break;
                default:
                    text = -1200683;
                    break;
                }

                _Creature->Whisper(text, player->GetGUID(), false);
            }
        }
    }

    return true;
}

CreatureAI* GetAI_npc_mojo(Creature *_Creature)
{
    return new npc_mojoAI(_Creature);
}


/*########
# npc_woeful_healer
#########*/

#define SPELL_PREYER_OF_HEALING     30604

struct npc_woeful_healerAI : public ScriptedAI
{
    npc_woeful_healerAI(Creature *c) : ScriptedAI(c)
    {
        Reset();
    }

    Timer_UnCheked healTimer;

    void Reset()
    {
        healTimer.Reset(urand(2500, 7500));
        me->GetMotionMaster()->MoveFollow(me->GetOwner(), 2.0, M_PI/2);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        Unit * owner = me->GetCharmerOrOwner();

        if (healTimer.Expired(diff))
        {
            healTimer = urand(2500, 7500);
            if (!owner || !owner->IsInCombat())
                return;
            me->CastSpell(me, SPELL_PREYER_OF_HEALING, false);
        }
    }
};

CreatureAI* GetAI_npc_woeful_healer(Creature* pCreature)
{
    return new npc_woeful_healerAI(pCreature);
}

#define GOSSIP_VIOLET_SIGNET    16530
#define GOSSIP_ETERNAL_BAND     16531

// Archmage Leryda from Karazhan and Soridormi from Mount Hyjal
bool GossipHello_npc_ring_specialist(Player* player, Creature* _Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );
    uint32 entry = _Creature->GetEntry();
    switch(entry)
    {
        // Archmage Leryda
    case 18253:
        // player has none of the rings
        if((!player->HasItemCount(29287, 1, true) && !player->HasItemCount(29279, 1, true) && !player->HasItemCount(29283, 1, true) && !player->HasItemCount(29290, 1, true))
                && // and had completed one of the chains
                (player->GetQuestRewardStatus(10725) || player->GetQuestRewardStatus(10728) || player->GetQuestRewardStatus(10727) || player->GetQuestRewardStatus(10726)))
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_VIOLET_SIGNET), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        break;
        // Soridormi
    case 19935:
        // player has none of the rings
        if((!player->HasItemCount(29305, 1, true) && !player->HasItemCount(29309, 1, true) && !player->HasItemCount(29301, 1, true) && !player->HasItemCount(29297, 1, true))
                && // and had completed one of the chains
                (player->GetQuestRewardStatus(10472) || player->GetQuestRewardStatus(10473) || player->GetQuestRewardStatus(10474) || player->GetQuestRewardStatus(10475)))
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ETERNAL_BAND),   GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        break;
    }

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

void RestoreQuestRingItem(Player* player, uint32 id)
{
    ItemPosCountVec dest;
    uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, id, 1);
    if( msg == EQUIP_ERR_OK )
    {
        Item* item = player->StoreNewItem( dest, id, true);
        player->SendNewItem(item, 1, true, false);
    }
}

bool GossipSelect_npc_ring_specialist(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    switch( action )
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
        if(player->GetQuestRewardStatus(10725))
            RestoreQuestRingItem(player, 29287);
        if(player->GetQuestRewardStatus(10728))
            RestoreQuestRingItem(player, 29279);
        if(player->GetQuestRewardStatus(10727))
            RestoreQuestRingItem(player, 29283);
        if(player->GetQuestRewardStatus(10726))
            RestoreQuestRingItem(player, 29290);
        player->CLOSE_GOSSIP_MENU();
        break;
    case GOSSIP_ACTION_INFO_DEF + 2:
        if(player->GetQuestRewardStatus(10472))
            RestoreQuestRingItem(player, 29305);
        if(player->GetQuestRewardStatus(10473))
            RestoreQuestRingItem(player, 29309);
        if(player->GetQuestRewardStatus(10474))
            RestoreQuestRingItem(player, 29301);
        if(player->GetQuestRewardStatus(10475))
            RestoreQuestRingItem(player, 29297);
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    return true;
}

/*######
# npc_fire_elemental_guardian
######*/
#define AURA_OF_FLAMES          22436
#define SPELL_FIRE_NOVA         8503                    // ~500 damage when shaman has 1100 SPD
#define SPELL_FIREBLAST         8413                    // around 900-1000 damage when shaman has 1100 SPD

struct npc_fire_elemental_guardianAI : public ScriptedAI
{
    npc_fire_elemental_guardianAI(Creature* c) : ScriptedAI(c){}

    Timer FireBlast_Timer;
    Timer FireNova_Timer;
    uint64 totemGUID;

    void EventPulse(Unit* totem, uint32 i)
    {
        FireBlast_Timer.Reset(1000); // 10-15 sec cd
        FireNova_Timer.Reset(1000); // 1 tick/ 5sec
        me->AddAura(AURA_OF_FLAMES, me);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        me->SetAggroRange(0);
        totemGUID = totem->GetGUID();
    }

    void UpdateAI(const uint32 diff)
    {
       Creature *pTotem = me->GetCreature(totemGUID);

       if (pTotem)
       {
            if (!pTotem->isAlive())
            {
                me->DisappearAndDie();
                return;
            }
            if ((!me->IsWithinDistInMap(pTotem, 30.0f) && !me->HasUnitState(UNIT_STAT_FOLLOW)) 
                || 
                (me->GetVictim() && me->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself() &&
                (pTotem->isInSanctuary() || me->isInSanctuary() || me->GetVictim()->isInSanctuary()))
                )
            {
                me->ClearInCombat();
                me->AttackStop();
                me->GetMotionMaster()->MoveFollow(pTotem, 2.0f, M_PI);
                return;
            }

            if (!me->GetVictim())
            {
                Unit *victim = pTotem->SelectNearestTarget(15.0f);
                Unit *attacker = pTotem->GetAttackerForHelper();
                if ((victim || attacker))
                {
                    if (attacker)
                        AttackStart(attacker);
                    else
                        AttackStart(victim);
                }
            }

            if (me->GetVictim())
            {
                if (FireNova_Timer.Expired(diff))
                {
                    DoCast(me->GetVictim(), SPELL_FIRE_NOVA);
                    FireNova_Timer = 6000 + rand() % 6000; // 6 - 12 sec
                }

                if (FireBlast_Timer.Expired(diff))
                {
                    DoCast(me->GetVictim(), SPELL_FIREBLAST);
                    FireBlast_Timer = 4000 + rand() % 4000; // 4-8 sec cd
                }

                DoMeleeAttackIfReady();
            }
        }
       else // no totem found
       {
            me->DisappearAndDie();
            return;
       }
    }
};

CreatureAI *GetAI_npc_fire_elemental_guardian(Creature* c)
{
     return new npc_fire_elemental_guardianAI(c);
};

/*######
# npc_earth_elemental_guardian
######*/
#define SPELL_ANGEREDEARTH        36213

struct npc_earth_elemental_guardianAI : public ScriptedAI
{
    npc_earth_elemental_guardianAI(Creature* c) : ScriptedAI(c) {}

    Timer AngeredEarth_Timer;
    uint64 totemGUID;

    void EventPulse(Unit* totem, uint32 i)
    {
        AngeredEarth_Timer.Reset(1000);
        me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);
        totemGUID = totem->GetGUID();
    }

    void UpdateAI(const uint32 diff)
    {
       Creature *pTotem = me->GetCreature(totemGUID);

       if (pTotem)
       {
            if (!pTotem->isAlive())
            {
                me->DisappearAndDie();
                return;
            }
            if ((!me->IsWithinDistInMap(pTotem, 30.0f) && !me->HasUnitState(UNIT_STAT_FOLLOW)) 
                || 
                (me->GetVictim() && me->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself() &&
                (pTotem->isInSanctuary() || me->isInSanctuary() || me->GetVictim()->isInSanctuary()))
                )
            {
                me->ClearInCombat();
                me->AttackStop();
                me->GetMotionMaster()->MoveFollow(pTotem, 2.0f, M_PI);
                return;
            }

            if (!me->GetVictim())
            {
                Unit *victim = pTotem->SelectNearestTarget(15.0f);
                Unit *attacker = pTotem->GetAttackerForHelper();
                if ((victim || attacker))
                {
                    if (attacker)
                        AttackStart(attacker);
                    else
                        AttackStart(victim);
                }
            }

            if (me->GetVictim())
            {
                if (AngeredEarth_Timer.Expired(diff))
                {
                    DoCast(me->GetVictim(), SPELL_ANGEREDEARTH);
                    AngeredEarth_Timer = 4000 + rand() % 4000; // 4-8 sec cd
                }

                DoMeleeAttackIfReady();
            }
        }
       else
       {
            me->DisappearAndDie();
            return;
       }
    }
};

CreatureAI *GetAI_npc_earth_elemental_guardian(Creature* c)
{
    return new npc_earth_elemental_guardianAI(c);
};

/*########
# npc_master_omarion
#########*/

//Blacksmithing
#define GOSSIP_ITEM_OMARION0  16517
#define GOSSIP_ITEM_OMARION1  16518
#define GOSSIP_ITEM_OMARION2  16519
//Leatherworking
#define GOSSIP_ITEM_OMARION3  16520
#define GOSSIP_ITEM_OMARION4  16521
#define GOSSIP_ITEM_OMARION5  16522
#define GOSSIP_ITEM_OMARION6  16523
#define GOSSIP_ITEM_OMARION7  16524
#define GOSSIP_ITEM_OMARION8  16525
//Tailoring
#define GOSSIP_ITEM_OMARION9  16526
#define GOSSIP_ITEM_OMARION10 16527
#define GOSSIP_ITEM_OMARION11 16528
#define GOSSIP_ITEM_OMARION12 16529

bool GossipHello_npc_master_omarion(Player *player, Creature *_Creature)
{
    bool isexalted,isrevered;
    isexalted = false;
    isrevered = false;
    if(player->GetReputationMgr().GetReputation(529) >= 21000)
    {
        isrevered = true;
        if(player->GetReputationMgr().GetReputation(529) >= 42000)
            isexalted = true;
    }

    if(player->GetBaseSkillValue(SKILL_BLACKSMITHING)>=300) // Blacksmithing +300
    {
        if(isrevered)
        {
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION0)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION1)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            if(isexalted)
            {
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION2)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            }
        }
    }
    if(player->GetBaseSkillValue(SKILL_LEATHERWORKING)>=300) // Leatherworking +300
    {
        if(isrevered)
        {
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION3)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION4)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            if(isexalted)
            {
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION5)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            }
        }
        if(isrevered)
        {
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION6)   , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION7)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
            if(isexalted)
            {
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION8)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
            }
        }
    }
    if(player->GetBaseSkillValue(SKILL_TAILORING)>=300) // Tailoring +300
    {
        if(isrevered)
        {
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION9)    , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION10)   , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            if(isexalted)
            {
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION11)   , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_OMARION12)   , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
            }
        }
    }
    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_master_omarion(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:         // Icebane Bracers
        player->learnSpell( 28244 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 2:         // Icebane Gauntlets
        player->learnSpell( 28243 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 3:         // Icebane Breastplate
        player->learnSpell( 28242 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 4:         // Polar Bracers
        player->learnSpell( 28221 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 5:         // Polar Gloves
        player->learnSpell( 28220 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 6:         // Polar Tunic
        player->learnSpell( 28219 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 7:         // Icy Scale Bracers
        player->learnSpell( 28224 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 8:         // Icy Scale Gauntlets
        player->learnSpell( 28223 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 9:         // Icy Scale Breastplate
        player->learnSpell( 28222 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 10:        // Glacial Wrists
        player->learnSpell( 28209 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 11:        // Glacial Gloves
        player->learnSpell( 28205 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 12:        // Glacial Vest
        player->learnSpell( 28207 );
        break;
    case GOSSIP_ACTION_INFO_DEF + 13:        // Glacial Cloak
        player->learnSpell( 28208 );
        break;
    }
    player->CLOSE_GOSSIP_MENU();
    return true;
}


/*########
# npc_lorekeeper_lydros
#########*/

#define GOSSIP_ITEM_LOREKEEPER1 16513
#define GOSSIP_ITEM_LOREKEEPER2 16514
#define GOSSIP_ITEM_LOREKEEPER3 16515
#define GOSSIP_ITEM_LOREKEEPER4 16516

bool GossipHello_npc_lorekeeper_lydros(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

	if (player->GetQuestRewardStatus(7507) && !player->HasItemCount(18513, 1) && !player->GetQuestRewardStatus(7508))
	{
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_LOREKEEPER1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		player->SEND_GOSSIP_MENU(24999, _Creature->GetGUID());
	}
	else 
		player->SEND_GOSSIP_MENU(14368, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lorekeeper_lydros(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF+1:
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_LOREKEEPER2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->SEND_GOSSIP_MENU(25000, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+2:
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_LOREKEEPER2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
        player->SEND_GOSSIP_MENU(25001, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+3:
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_LOREKEEPER2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
        player->SEND_GOSSIP_MENU(25002, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+4:
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_LOREKEEPER3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
        player->SEND_GOSSIP_MENU(25003, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+5:
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_LOREKEEPER4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
        player->SEND_GOSSIP_MENU(25004, _Creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+6:
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 18513, 1);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, 18513, true);
            player->SendNewItem(item,1,true,false,true);
        }
        player->CLOSE_GOSSIP_MENU();
        break;
    }
    return true;
}


/*########
# npc_crashin_trashin_robot
#########*/

#define SPELL_MACHINE_GUN           42382
#define SPELL_NET                   41580
#define SPELL_ELECTRICAL            42372
#define CRASHIN_TRASHIN_ROBOT_ID    17299

struct npc_crashin_trashin_robotAI : public ScriptedAI
{
    npc_crashin_trashin_robotAI(Creature *c) : ScriptedAI(c) {}

    Timer_UnCheked machineGunTimer;
    Timer_UnCheked netTimer;
    Timer_UnCheked electricalTimer;
    Timer_UnCheked checkTimer;
    Timer_UnCheked moveTimer;
    Timer_UnCheked despawnTimer;
    Timer_UnCheked waitTimer;

    void Reset()
    {
        waitTimer.Reset(5000);
        machineGunTimer.Reset(urand(1000, 3000));
        netTimer.Reset(urand(10000, 20000));
        electricalTimer.Reset(urand(5000, 35000));
        checkTimer.Reset(3000);
        me->SetDefaultMovementType(RANDOM_MOTION_TYPE);

        me->GetMotionMaster()->MoveRandomAroundPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 10.0f);
        moveTimer.Reset(urand(1000, 10000));
        despawnTimer.Reset(180000);
    }

    void EnterCombat(Unit *who)
    {
        checkTimer = 0;
    }

    std::list<Creature*> FindCrashinTrashinRobots()
    {
        std::list<Creature*> crashinTrashinRobots = FindAllCreaturesWithEntry(CRASHIN_TRASHIN_ROBOT_ID, 10.0);

        for (std::list<Creature*>::iterator itr = crashinTrashinRobots.begin(); itr != crashinTrashinRobots.end();)
        {
            std::list<Creature*>::iterator tmpItr = itr;
            ++itr;
            if ((*tmpItr)->GetGUID() == me->GetGUID())
            {
                crashinTrashinRobots.erase(tmpItr);
                break;
            }
        }

        return crashinTrashinRobots;
    }

    void SpellHit(Unit * caster, const SpellEntry * spell)
    {
        if (me->IsInCombat() || !caster || !spell || caster->GetEntry() != CRASHIN_TRASHIN_ROBOT_ID)
            return;

        me->SetInCombatWith(caster);
        caster->SetInCombatWith(me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (waitTimer.GetInterval())
        {
            if (waitTimer.Expired(diff))
                waitTimer = 0;

            return;
        }

        if (!me->isAlive())
            return;

        if (despawnTimer.Expired(diff))
        {
            me->Kill(me, false);
            return;
        }

        if (checkTimer.GetInterval())
        {
            if (checkTimer.Expired(diff))
            {
                if (!(FindCrashinTrashinRobots().empty()))
                    checkTimer = 0;
                else
                    checkTimer = 3000;
            }

            return;
        }

        std::list<Creature*> otherCrashinTrashinRobots;
        std::list<Creature*>::iterator itr;

        if (moveTimer.Expired(diff))
        {
            if (!me->HasAura(SPELL_NET, 0))
                otherCrashinTrashinRobots = FindCrashinTrashinRobots();

            int count = otherCrashinTrashinRobots.size();

            if (count)
            {
                float x, y, z;
                itr = otherCrashinTrashinRobots.begin();

                if (count > 1)
                    advance(itr, rand()%(count - 1));

                Creature * tmp = *(itr);

                tmp->GetNearPoint(x, y, z, 0, 5.0f, frand(0.0f, M_PI*2));
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, x, y, z);
            }

            moveTimer = urand(5000, 15000);
        }

        if (machineGunTimer.Expired(diff))
        {
            if (otherCrashinTrashinRobots.empty())
                otherCrashinTrashinRobots = FindCrashinTrashinRobots();

            int count = otherCrashinTrashinRobots.size();

            if (count)
            {
                itr = otherCrashinTrashinRobots.begin();

                if (count > 1)
                    advance(itr, rand()%(count - 1));

                AddSpellToCast(*itr, SPELL_MACHINE_GUN, false, true);
            }

            machineGunTimer = urand(500, 2000);
        }

        if (netTimer.Expired(diff))
        {
            if (otherCrashinTrashinRobots.empty())
                otherCrashinTrashinRobots = FindCrashinTrashinRobots();

            int count = otherCrashinTrashinRobots.size();

            if (count)
            {
                itr = otherCrashinTrashinRobots.begin();

                if (count > 1)
                    advance(itr, rand()%(count - 1));

                AddSpellToCast(*itr, SPELL_NET, false, true);
            }

            netTimer = urand(10000, 30000);
        }

        if (electricalTimer.Expired(diff))
        {
            if (otherCrashinTrashinRobots.empty())
                otherCrashinTrashinRobots = FindCrashinTrashinRobots();

            int count = otherCrashinTrashinRobots.size();

            if (count)
            {
                itr = otherCrashinTrashinRobots.begin();
                if (count > 1)
                    advance(itr, rand()%(count - 1));

                AddSpellToCast(*itr, SPELL_ELECTRICAL, false, true);
            }

            electricalTimer = urand(5000, 45000);
        }

        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_crashin_trashin_robot(Creature* pCreature)
{
    return new npc_crashin_trashin_robotAI(pCreature);
}

/*########
# npc_Oozeling
#########*/

#define GO_DARK_IRON_ALE_MUG    165578

struct pet_AleMugDrinkerAI : public ScriptedAI
{
    pet_AleMugDrinkerAI(Creature *c) : ScriptedAI(c) {}

    Timer_UnCheked wait;
    bool aleMug_drink;

    void Reset()
    {
        wait = 0;
        aleMug_drink = false;
        me->GetMotionMaster()->MoveFollow(me->GetOwner(), 2.0, M_PI/2);
    }

    void SpellHit(Unit * caster, const SpellEntry * spell)
    {
        if(spell->Id == 14813 && caster)
        {
            wait = 3000;
            aleMug_drink = true;
            float x, y, z;
            caster->GetPosition(x,y,z);
            me->GetMotionMaster()->MovePoint(0, x, y, z);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!aleMug_drink)
            return;

        if (wait.Expired(diff))
        {
            if (GameObject* mug = FindGameObject(GO_DARK_IRON_ALE_MUG, 20.0, me))
                mug->Delete();

            Reset();
        }
    }
};

CreatureAI* GetAI_pet_AleMugDrinker(Creature* pCreature)
{
    return new pet_AleMugDrinkerAI(pCreature);
}

/*###
# npc_land_mine
# UPDATE `creature_template` SET `ScriptName` = 'npc_land_mine' WHERE `entry` = 7527;
###*/

struct npc_land_mineAI : public Scripted_NoMovementAI
{
    npc_land_mineAI(Creature *c) : Scripted_NoMovementAI(c), _done(false)
    {
        me->SetAggroRange(5.0f);
    }

    bool _done;
    void IsSummonedBy(Unit *summoner)
    {
        me->setFaction(summoner->getFaction());
        me->SetOwnerGUID(summoner->GetGUID());

        // despawn after 10s
        me->ForcedDespawn(10000);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (_done || !me->canStartAttack(who))
            return;

        // int32 damage = urand(394, 507);
        // me->CastCustomSpell(me, 27745, &damage, 0, 0, true);
        me->CastSpell(me, 4043, true);
        me->ForcedDespawn();

        _done = true;
    }
};

CreatureAI* GetAI_npc_land_mine(Creature* pCreature)
{
    return new npc_land_mineAI(pCreature);
}

#define SPELL_ARCANITE_DRAGONLING 9658
struct npc_arcanite_dragonlingAI : public ScriptedAI
{
    npc_arcanite_dragonlingAI(Creature *c) : ScriptedAI(c) {}

    Timer spellTimer;
    Timer checkTimer;

    void Reset()
    {
        spellTimer.Reset(25000);
        checkTimer.Reset(1000);
    }

    Unit* selectTarget(Unit* m_owner)
    {
        if (m_owner->GetVictim())
            return m_owner->GetVictim();
        if (!m_owner->GetAttackers().empty())
            return m_owner->GetAttackerForHelper();
        if (!me->GetAttackers().empty())
            return me->GetAttackerForHelper();
        return NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if (checkTimer.Expired(diff))
        {
            Unit* victim = me->GetVictim();
            if (!victim || !me->canAttack(victim))
            {
                Unit* m_owner = me->GetCharmerOrOwner();

                if (Unit* newvictim = selectTarget(m_owner))
                    AttackStart(newvictim);
                else if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
                {
                    me->CombatStop();
                    me->GetMotionMaster()->MoveFollow(m_owner, 2.0f, M_PI * 3 / 4);
                    spellTimer.Reset(25000);
                }
            }

            checkTimer.Reset(1000);
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();

        if (spellTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_ARCANITE_DRAGONLING);
            spellTimer = 25000;
        }
    }
};

CreatureAI* GetAI_npc_arcanite_dragonling(Creature* pCreature)
{
    return new npc_arcanite_dragonlingAI(pCreature);
}

enum MiniPetsInfo
{
    NPC_PANDA                   = 11325,
    SPELL_PANDA_SLEEP           = 19231,
    SPELL_PANDA_ROAR            = 40664,

    NPC_DIABLO                  = 11326,
    SPELL_DIABLO_FLAME          = 18874,

    NPC_ZERGLING                = 11327,
    SPELL_ZERGLING              = 19227,

    NPC_WILLY                   = 23231,
    SPELL_WILLY_SLEEP           = 40663,
    SPELL_WILLY_TRIGGER         = 40619,

    NPC_DRAGON_KITE             = 25110,
    SPELL_DRAGON_KITE_LIGHTNING = 45197,
    SPELL_DRAGON_KITE_STRING    = 45192,

    NPC_MURKY                   = 15186,
    NPC_LURKY                   = 15358,
    NPC_GURKY                   = 16069,
    SPELL_MURKY_DANCE           = 25165,

    NPC_EGBERT                  = 23258,
    SPELL_EGBERT_HAPPYNESS      = 40669,

    NPC_SCORCHLING              = 25706,
    SPELL_SCORCHLING_BLAST      = 45889,

    NPC_DISGUSTING_OOZELING     = 15429,
};

struct npc_small_pet_handlerAI : public ScriptedAI
{
    npc_small_pet_handlerAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    bool m_bIsIdle;
    bool m_bIsInAction;

    Timer_UnCheked m_uiCheckTimer;
    Timer_UnCheked m_uiActionTimer;

    void Reset()
    {
        ClearCastQueue();

        m_bIsIdle = false;
        m_bIsInAction = false;

        m_uiCheckTimer.Reset(1000);
        m_uiActionTimer.Reset(urand(10000, 30000));

        me->GetMotionMaster()->MoveFollow(me->GetOwner(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);

        PetCreateAction(me->GetEntry());
    }

    void AttackStart(Unit* who) {}

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        // Check if pet is moving
        if (m_uiCheckTimer.Expired(diff))
        {
            if (Unit* pUnit = me->GetOwner())
            {
                Player *pPlayer = pUnit->ToPlayer();

                // Change speed if owner is mounted
                if (pPlayer->IsMounted())
                    me->SetSpeed(MOVE_RUN, 2.0f, true);
                else
                    me->SetSpeed(MOVE_RUN, 1.0f, true);

                // Check if owner is stopped
                if (pPlayer->isMoving() && m_bIsIdle)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);

                    if (me->IsNonMeleeSpellCast(false))
                        me->InterruptNonMeleeSpells(false);

                    m_bIsIdle = false;
                    m_uiActionTimer = urand(10000, 20000);
                }
                else if (me->IsWithinDistInMap(pPlayer, 1.5f) && !m_bIsIdle)
                {
                    m_bIsIdle = true;
                }
            }
            m_uiCheckTimer = 1000;
        }

        // Return if pet is moving
        if (!m_bIsIdle)
        {
            m_bIsInAction = false;
            return;
        }

        // Do pet's action
        if (m_uiActionTimer.Expired(diff))
        {
            // Do action
            if (!m_bIsInAction)
            {
                m_uiActionTimer = urand(30000, 60000); // Prevent stopping action too early
                m_bIsInAction = true;
                PetAction(me->GetEntry());
            }
            // Stop action
            else
            {
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                if (me->IsNonMeleeSpellCast(false))
                    me->InterruptNonMeleeSpells(false);

                m_uiActionTimer = urand(10000, 30000);
                m_bIsInAction = false;
            }
        }

        CastNextSpellIfAnyAndReady();
    }

    void PetCreateAction(uint32 uiPetEntry)
    {
        if (!uiPetEntry)
            return;

        switch (uiPetEntry)
        {
            case NPC_DRAGON_KITE:
            {
                AddSpellToCast(me->GetOwner(), SPELL_DRAGON_KITE_STRING);
                break;
            }
            case NPC_WILLY:
            {
                AddSpellToCast(me,SPELL_WILLY_TRIGGER);
                break;
            }
            default:
                break;
        }
    }

    void PetAction(uint32 uiPetEntry)
    {
        if (!uiPetEntry)
            return;

        switch (uiPetEntry)
        {
            case NPC_PANDA:
            {
                AddSpellToCast(RAND(SPELL_PANDA_SLEEP, SPELL_PANDA_ROAR), CAST_SELF);
                break;
            }
            case NPC_DIABLO:
            {
                AddSpellToCast(SPELL_DIABLO_FLAME, CAST_SELF);
                break;
            }
            case NPC_ZERGLING:
            {
                AddSpellToCast(SPELL_ZERGLING, CAST_SELF);
                break;
            }
            case NPC_WILLY:
            {
                AddSpellToCast(SPELL_WILLY_SLEEP, CAST_SELF);
                break;
            }
            case NPC_DRAGON_KITE:
            {
                if (Unit* pOwner = me->GetCharmerOrOwner())
                    AddSpellToCast(pOwner, SPELL_DRAGON_KITE_LIGHTNING);
                break;
            }
            case NPC_MURKY:
            case NPC_LURKY:
            case NPC_GURKY:
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                AddSpellToCast(SPELL_MURKY_DANCE);
                break;
            }
            case NPC_EGBERT:
            {
                AddSpellToCast(SPELL_EGBERT_HAPPYNESS, CAST_SELF);
                break;
            }
            case NPC_SCORCHLING:
            {
                AddSpellToCast(SPELL_SCORCHLING_BLAST, CAST_SELF);
                break;
            }
        }
    }
};

CreatureAI* GetAI_npc_small_pet_handler(Creature* pCreature)
{
    return new npc_small_pet_handlerAI(pCreature);
}

bool GossipHello_npc_combatstop(Player* player, Creature* _Creature)
{
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16512), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    // Hey there, $N. How can I help you?
    player->SEND_GOSSIP_MENU(2, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_combatstop(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        _Creature->MonsterWhisper(-1200675, player->GetGUID());
        player->CombatStop(true);
    }

    return true;
}

struct npc_resurrectAI : public Scripted_NoMovementAI
{
    npc_resurrectAI(Creature* c) : Scripted_NoMovementAI(c) {}

    Timer_UnCheked timer;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        timer.Reset(2000);
    }

    void MoveInLineOfSight(Unit *who) override {}
    void AttackStart(Unit* who) override {}
    void EnterCombat(Unit *who) override {}

    void UpdateAI(const uint32 diff) override
    {
        if (timer.Expired(diff))
        {
            std::list<Player*> players;
            Hellground::AnyPlayerInObjectRangeCheck check(me, 15.0f, false);
            Hellground::ObjectListSearcher<Player, Hellground::AnyPlayerInObjectRangeCheck> searcher(players, check);

            Cell::VisitAllObjects(me, searcher, 15.0f);

            players.remove_if([this](Player* plr) -> bool { return me->IsHostileTo(plr); });

            while (!players.empty())
            {
                Player* player = players.front();

                players.pop_front();

                player->ResurrectPlayer(10.0f);
                player->CastSpell(player, SPELL_RESURRECTION_VISUAL, true);   // Resurrection visual
            }
            timer = 2000;
        }
    }
};

CreatureAI* GetAI_npc_resurrect(Creature* pCreature)
{
    return new npc_resurrectAI(pCreature);
}

enum TargetDummySpells
{
    TARGET_DUMMY_PASSIVE = 4044,
    TARGET_DUMMY_SPAWN_EFFECT = 4507,

    ADVANCED_TARGET_DUMMY_PASSIVE = 4048,
    ADVANCED_TARGET_DUMMY_SPAWN_EFFECT = 4092,

    MASTER_TARGET_DUMMY_PASSIVE = 19809,
};

enum TargetDummyEntry
{
    TARGET_DUMMY = 2673,
    ADV_TARGET_DUMMY = 2674,
    MASTER_TARGET_DUMMY = 12426
};

struct npc_target_dummyAI : public Scripted_NoMovementAI
{
    npc_target_dummyAI(Creature* c) : Scripted_NoMovementAI(c) {}

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);

        ClearCastQueue();

        TargetDummySpells spawneffect;
        TargetDummySpells passive;

        switch (me->GetEntry())
        {
            case TARGET_DUMMY:
            {
                spawneffect = TARGET_DUMMY_SPAWN_EFFECT;
                passive = TARGET_DUMMY_PASSIVE;
                break;
            }
            case ADV_TARGET_DUMMY:
            {
                spawneffect = ADVANCED_TARGET_DUMMY_SPAWN_EFFECT;
                passive = ADVANCED_TARGET_DUMMY_PASSIVE;
                break;
            }
            case MASTER_TARGET_DUMMY:
            {
                spawneffect = ADVANCED_TARGET_DUMMY_SPAWN_EFFECT;
                passive = MASTER_TARGET_DUMMY_PASSIVE;
                break;
            }
        }

        AddSpellToCast(passive, CAST_SELF);
        AddSpellToCast(spawneffect, CAST_SELF);
    }

    void AttackStart(Unit* who) override {}
    void EnterCombat(Unit *who) override {}
    void MoveInLineOfSight(Unit* who) override {}

    void UpdateAI(const uint32 diff) override
    {
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_target_dummy(Creature* pCreature)
{
    return new npc_target_dummyAI(pCreature);
}

enum ExplosiveSheepExplosion
{
    EXPLOSIVE_SHEEP_EXPLOSION = 4050,
    HIGH_EXPLOSIVE_SHEEP_EXPLOSION = 44279,
};

enum ExplosiveSheepEntry
{
    EXPLOSIVE_SHEEP = 2675,
    HIGH_EXPLOSIVE_SHEEP = 24715
};

struct npc_explosive_sheepAI : public ScriptedAI
{
    npc_explosive_sheepAI(Creature* c) : ScriptedAI(c) {}

    Timer_UnCheked explosionTimer;

    void JustRespawned() override
    {
        explosionTimer = 10000;
    }

    void Reset() override
    {
        ClearCastQueue();
    }

    void AttackStart(Unit* who) override {}
    void EnterCombat(Unit *who) override {}
    void MoveInLineOfSight(Unit* who) override {}

    void UpdateAI(const uint32 diff) override
    {
        if (explosionTimer.Expired(diff))
        {			
			me->CastSpell(me, me->GetEntry() == EXPLOSIVE_SHEEP ? EXPLOSIVE_SHEEP_EXPLOSION : HIGH_EXPLOSIVE_SHEEP_EXPLOSION, true);
            me->ForcedDespawn();
            return;
        }

        if (!me->GetVictim())
        {
            if (Unit* target = me->SelectNearestTarget(30.0f))
                ScriptedAI::AttackStart(target);
        }
        else
        {
            if (me->IsWithinDistInMap(me->GetVictim(), 2.0f))
            {
				me->CastSpell(me->GetVictim(), me->GetEntry() == EXPLOSIVE_SHEEP ? EXPLOSIVE_SHEEP_EXPLOSION : HIGH_EXPLOSIVE_SHEEP_EXPLOSION, true);
                me->ForcedDespawn();
                return;
            }
        }

        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_explosive_sheep(Creature* pCreature)
{
    return new npc_explosive_sheepAI(pCreature);
}

/*######
## Meridith the Mermaiden
######*/

#define GOSSIP_HELLO       16511
#define LOVE_SONG_QUEST_ID 8599
#define SIREN_SONG         25678

bool GossipHello_npc_meridith_the_mermaiden(Player *player, Creature *creature)
{
    if(creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if(player->GetQuestRewardStatus(LOVE_SONG_QUEST_ID))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HELLO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    }
    player->PlayerTalkClass->SendGossipMenu(7916, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_meridith_the_mermaiden(Player *player, Creature * creature, uint32 sender, uint32 action )
{
    if(action == GOSSIP_ACTION_INFO_DEF+1)
    {
        creature->Say(-1200674, LANG_UNIVERSAL, 0);
        creature->CastSpell(player, SIREN_SONG, false);
        player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

// npc_gnomish_flame_turret
#define SPELL_GNOMISH_FLAME_TURRET 43050

struct npc_gnomish_flame_turret : public Scripted_NoMovementAI
{
    npc_gnomish_flame_turret(Creature* c) : Scripted_NoMovementAI(c)
    {
        me->SetAggroRange(10.0f); // radius of spell
    }
    Timer_UnCheked CheckTimer;

    void Reset()
    {
        SetAutocast(SPELL_GNOMISH_FLAME_TURRET, 1000);
        StartAutocast();
        me->SetReactState(REACT_AGGRESSIVE);
        CheckTimer.Reset(2000);
    }

    bool UpdateVictim()
    {
        if (ScriptedAI::UpdateVictim())
            return true;

        if (Unit* target = me->SelectNearestTarget(10.0f))
            AttackStart(target);

        return me->GetVictim();
    }

    void UpdateAI(const uint32 diff)
    {
        if (CheckTimer.Expired(diff))
        {
            Unit* owner = me->GetOwner();
            if (!owner || !owner->IsInMap(me))
            {
                me->ForcedDespawn();
                return;
            }
            CheckTimer = 2000;
        }

        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady(diff);
    }
};

CreatureAI* GetAI_npc_gnomish_flame_turret(Creature *_Creature)
{
    return new npc_gnomish_flame_turret(_Creature);
}

struct npc_bad_santaAI : public ScriptedAI
{
    npc_bad_santaAI(Creature *c) : ScriptedAI(c)
    {
    }

#define SPELL_BLIZZARD         41482
#define SPELL_ICE_ARMOR        36881 // 30 min
#define SPELL_ICEBOLT          22357 // bolt stun
#define SPELL_ICE_CHAINS       29991 // root
#define SPELL_ENRAGE           47008 // 900/150
#define SPELL_FROST_NOVA       44177 // 8s
#define SPELL_FROSTBOLT_VOLLEY 38837
#define SPELL_FROST_MIST       29292
#define SPELL_FROST_AURA       28531 // 3x600 36yd
#define SPELL_FROST_WEAKNESS   25178
#define SPELL_FROST_BUFFET     38142

#define NPC_EVIL_REVELER       66715

    Timer Frost_Buffet_Timer;
    Timer Weakness_Timer;
    Timer Blizzard_Timer;
    Timer Volley_Timer;
    Timer Armor_Timer;
    Timer Nova_Timer;
    Timer IceBolt_Timer;
    Timer Enrage_Timer;

    void Reset()
    {
        if (me->HasAura(SPELL_ENRAGE))
            for (uint8 i = 0; i < 3; i++)
                me->RemoveAura(SPELL_ENRAGE, i);
        ClearCastQueue();
        Frost_Buffet_Timer.Reset(3000);
        Weakness_Timer.Reset(5000);
        Blizzard_Timer.Reset(15000);
        Volley_Timer.Reset(20000);
        Armor_Timer.Reset(30000);
        Nova_Timer.Reset(10000);
        IceBolt_Timer.Reset(45000);
        Enrage_Timer.Reset(1000*60*15);
    }

    void EnterEvadeMode()
    {
        CreatureAI::EnterEvadeMode();
        Reset();
    }

    void EnterCombat(Unit* who)
    {
        ForceSpellCast(SPELL_ICE_ARMOR, CAST_SELF);
        me->MonsterSay(-1200673, 0, 0);
        me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
        AddSpellToCast(me->GetVictim(), SPELL_FROSTBOLT_VOLLEY);
    }

    void SpellHitTarget(Unit* who, const SpellEntry* SpellID)
    {
        if (SpellID->Id != SPELL_FROST_BUFFET)
            if (SpellID->Id == SPELL_FROSTBOLT_VOLLEY && who != me->GetVictim())
                me->AddAura(SPELL_FROST_BUFFET, who);

        if (SpellID->Id == SPELL_FROSTBOLT_VOLLEY)
            ForceSpellCast(who, SPELL_ICE_CHAINS);

        if (SpellID->Id != SPELL_FROST_MIST)
            if (!who->HasAura(SPELL_FROST_MIST))
                if (Unit* uglyhack = me->SummonTrigger(who->GetPositionX(), who->GetPositionY(), who->GetPositionZ(), 0, 5.0))   //so ugly it hurts
                    uglyhack->CastSpell(who, SPELL_FROST_MIST, true, 0, 0, me->GetGUID());



    }

    void KilledUnit(Unit* who)
    {
        if (who->GetObjectGuid().IsPlayer())
            me->MonsterSay(-1200672, 0, 0);
        if (Unit* uglyhack = me->SummonTrigger(who->GetPositionX(), who->GetPositionY(), who->GetPositionZ(), 0, 5.0))
        {
            uglyhack->CastSpell(who, SPELL_FROST_MIST, true, 0, 0, me->GetGUID());
            uglyhack->CastSpell(who, SPELL_FROST_NOVA, true, 0, 0, me->GetGUID());
        }

    }

    void JustDied(Unit* who)
    {
     while (Creature* reveler = GetClosestCreatureWithEntry(me, NPC_EVIL_REVELER, 200.0f))
     {
         me->Kill(reveler);
     }
     me->MonsterSay(-1200671,0,0);
    }

    void UpdateAI(const uint32 diff)
    {
          if (!me->IsInCombat())
              return;
          if (!UpdateVictim())
              return;


          if (Frost_Buffet_Timer.Expired(diff))
          {
              if(me->GetVictim())
              {
                  me->AddAura(SPELL_FROST_BUFFET, me->GetVictim());
                  Frost_Buffet_Timer = 3000;
              }

          }

          if (Blizzard_Timer.Expired(diff))
          {
              AddSpellToCast(SPELL_BLIZZARD, CAST_RANDOM);
              Blizzard_Timer = 13000;
          }

          if (Volley_Timer.Expired(diff))
          {
              AddSpellToCast(SPELL_FROSTBOLT_VOLLEY, CAST_TANK);
              Volley_Timer = 20000;
          }

          if (Armor_Timer.Expired(diff))
          {
              AddSpellToCast(SPELL_ICE_ARMOR, CAST_SELF);
              Armor_Timer = 30000;
          }

          if (Nova_Timer.Expired(diff))
          {
              AddSpellToCast(SPELL_FROST_NOVA, CAST_TANK);
              Nova_Timer = 8000;
          }

          if (IceBolt_Timer.Expired(diff))
          {
              AddSpellToCast(SPELL_ICEBOLT, CAST_TANK);
              IceBolt_Timer = 45000;
          }

          if (Weakness_Timer.Expired(diff))
          {
              if (Unit* target = me->GetVictim())
                  if (target->GetAura(SPELL_FROST_BUFFET, 1) && target->GetAura(SPELL_FROST_BUFFET, 1)->GetStackAmount() == 20)
                  {
                      me->MonsterSay("TASTE THE TRUE MEANINGNESS OF COLD!",0,0);
                      me->AddAura(SPELL_FROST_WEAKNESS, target);
                      me->CastSpell(target, SPELL_BLIZZARD, true);
                      me->CastSpell(target, SPELL_ICEBOLT, true);
                      Weakness_Timer = 10000;
                  }
                  else
                      Weakness_Timer = 1000;
          }

          if (Enrage_Timer.Expired(diff))
          {
              AddSpellToCast(SPELL_ENRAGE, CAST_SELF);
              Enrage_Timer = 1000*60*15;
          }

          CastNextSpellIfAnyAndReady();
          DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_bad_santaAI(Creature *_Creature)
{
    return new npc_bad_santaAI(_Creature);
}

struct npc_nearly_dead_combat_dummyAI : public Scripted_NoMovementAI
{
    npc_nearly_dead_combat_dummyAI(Creature *c) : Scripted_NoMovementAI(c)
    {
    }

    uint64 AttackerGUID;
    Timer_UnCheked Check_Timer;

    void Reset()
    {
        m_creature->SetHealth(m_creature->GetMaxHealth()/11);
        m_creature->SetNoCallAssistance(true);
        m_creature->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STUN, true);
        AttackerGUID = 0;
        Check_Timer = 0;
    }

    void EnterCombat(Unit* who)
    {
        AttackerGUID = ((Player*)who)->GetGUID();
        m_creature->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN, UNIT_ACTION_PRIORITY_END);
    }

    void DamageTaken(Unit *attacker, uint32 &damage)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        Player* attacker = Player::GetPlayerInWorld(AttackerGUID);

        if (!UpdateVictim())
            return;

        if (attacker && Check_Timer.Expired(diff))
        {
            if(m_creature->GetDistance(attacker) > 5.0f)
                EnterEvadeMode();

            Check_Timer = 3000;
        }
    }
};

CreatureAI* GetAI_npc_nearly_dead_combat_dummy(Creature *_Creature)
{
    return new npc_nearly_dead_combat_dummyAI(_Creature);
}

struct npc_instakill_guardianAI : public Scripted_NoMovementAI
{
    npc_instakill_guardianAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        //me->SetReactState(REACT_PASSIVE);
    }

    float kill_distance = 30;

    void MoveInLineOfSight(Unit* who)
    {
        if (me->isAlive())
        {
            Player* player = who->GetCharmerOrOwnerPlayerOrPlayerItself();
            if (!player || player->isGameMaster() || player->isDead())
                return;

            if (player->IsGuildHouseOwnerMember())
                return;

            WorldLocation p_pos, c_pos;
            player->GetPosition(p_pos);
            who->GetPosition(c_pos);

            if ((who->GetCharmerOrOwner() && who->GetCharmerOrOwner()->GetTypeId() == TYPEID_PLAYER && me->GetExactDist(&c_pos) < kill_distance) ||
                m_creature->GetExactDist(&p_pos) < kill_distance
                )
            {
                me->CastSpell(player, 44776, true);
                player->Kill(player);
                
                me->MonsterSay(player->GetSession()->GetHellgroundString(15522), 0, 0);
            }
        }
    }
};

CreatureAI* GetAI_npc_instakill_guardian(Creature *_Creature)
{
    return new npc_instakill_guardianAI(_Creature);
}

struct npc_ogrecasterarenarealmAI : public Scripted_NoMovementAI
{
    npc_ogrecasterarenarealmAI(Creature *c) : Scripted_NoMovementAI(c)
    {
    }
    
    uint32 CheckTimer;

    void Reset()
    {
        CheckTimer = 3000;
        me->CastSpell(me, 39550, true); // 39218
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(CheckTimer)
        {
            if(CheckTimer <= diff)
            {
                if(Unit* trigger1 = FindCreature(992032, 20, me))
                    trigger1->CastSpell(me, 30465, true);//34209
                if(Unit* trigger2 = FindCreature(992033, 20, me))
                    trigger2->CastSpell(me, 30464, true);
                if(Unit* trigger3 = FindCreature(992034, 20, me))
                    trigger3->CastSpell(me, 30463, true);
                CheckTimer = 0;
            } else CheckTimer -= diff;
        }
    }
};

CreatureAI* GetAI_npc_ogrecasterarenarealm(Creature *_Creature)
{
    return new npc_ogrecasterarenarealmAI(_Creature);
}

bool GossipHello_npc_ogrecasterarenarealm(Player *player, Creature *creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(LANG_SCRIPT_TRANS_GIVE_ITEM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_ogrecasterarenarealm(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 933001, 1);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, 933001, true);
                player->SendNewItem(item,1,true,false,true);
            }
            player->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}
/*######
# npc_voodoo_servant
######*/

#define SPELL_LIGHTING_BLAST 43996

struct npc_voodoo_servantAI : public ScriptedAI
{
    npc_voodoo_servantAI(Creature* c) : ScriptedAI(c){}

    Timer_UnCheked LightingBlast_Timer;

    void Reset()
    {
        LightingBlast_Timer.Reset(100);
        me->SetReactState(REACT_DEFENSIVE);
        me->SetAggroRange(0);
        me->CombatStopWithPets();
        me->ClearInCombat();
        me->AttackStop();
    }

    void UpdateAI(const uint32 diff)
    {
       Unit *pOwner = me->GetOwner();
       Unit *victim = me->GetVictim();
       Unit *attacker = pOwner->GetAttackerForHelper();

       if (pOwner)
       {
            if (!pOwner->isAlive())
            {
                me->ForcedDespawn();
                return;
            }
            if (!me->IsWithinDistInMap(pOwner, 30.0f) || (!victim || !attacker))
            {
                if (!me->GetVictim()|| !me->IsWithinDistInMap(pOwner, 30.0f))
                    if (!me->HasUnitState(UNIT_STAT_FOLLOW))
                    {
                    victim = NULL;
                    attacker = NULL;
                    me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, urand(M_PI, M_PI/2));
                    Reset();
                    return;
                    }
            }
            if (me->GetVictim() && me->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself() &&
                (pOwner->isInSanctuary() || me->isInSanctuary() || me->GetVictim()->isInSanctuary()))
            {
                victim = NULL;
                attacker = NULL;
                me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, M_PI);
                Reset();
                return;
            }

            if (victim || attacker)
            {
                if (attacker)
                {
                    me->SetInCombatWith(attacker);
                    AttackStart(attacker);
                }
                else
                {
                    me->SetInCombatWith(victim);
                    AttackStart(victim);
                }
                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;


                if (LightingBlast_Timer.Expired(diff))
                {
                    DoCast(me->GetVictim(), SPELL_LIGHTING_BLAST);
                    LightingBlast_Timer = 2000;
                }

                DoMeleeAttackIfReady();
            }
       }
    }
};

CreatureAI *GetAI_npc_voodoo_servant(Creature* c)
{
     return new npc_voodoo_servantAI(c);
};

/*########
# npc_air_force_bots
#########*/

enum SpawnType
{
    SPAWNTYPE_TRIPWIRE_ROOFTOP,                             // no warning, summon creature at smaller range
    SPAWNTYPE_ALARMBOT                                      // cast guards mark and summon npc - if player shows up with that buff duration < 5 seconds attack
};

struct SpawnAssociation
{
    uint32 ThisCreatureEntry;
    uint32 SpawnedCreatureEntry;
    SpawnType spawnType;
};

#define SPELL_GUARDS_MARK           38067

const float RANGE_TRIPWIRE          = 15.0f;
const float RANGE_GUARDS_MARK       = 50.0f;

SpawnAssociation SpawnAssociations[] =
{
    {2614,  15241, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Alliance)
    {2615,  15242, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Horde)
    {21974, 21976, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Area 52)
    {21993, 15242, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Horde - Bat Rider)
    {21996, 15241, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Alliance - Gryphon)
    {21997, 21976, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Goblin - Area 52 - Zeppelin)
    {21999, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Rooftop (Alliance)
    {22001, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Rooftop (Horde)
    {22002, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Ground (Horde)
    {22003, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Ground (Alliance)
    {22063, 21976, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Rooftop (Goblin - Area 52)
    {22065, 22064, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Ethereal - Stormspire)
    {22066, 22067, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Scryer - Dragonhawk)
    {22068, 22064, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Rooftop (Ethereal - Stormspire)
    {22069, 22064, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Stormspire)
    {22070, 22067, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Rooftop (Scryer)
    {22071, 22067, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Scryer)
    {22078, 22077, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Aldor)
    {22079, 22077, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Aldor - Gryphon)
    {22080, 22077, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Rooftop (Aldor)
    {22086, 22085, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Sporeggar)
    {22087, 22085, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Sporeggar - Spore Bat)
    {22088, 22085, SPAWNTYPE_TRIPWIRE_ROOFTOP},             // Air Force Trip Wire - Rooftop (Sporeggar)
    {22090, 22089, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Toshley's Station - Flying Machine)
    {22124, 22122, SPAWNTYPE_ALARMBOT},                     // Air Force Alarm Bot (Cenarion)
    {22125, 22122, SPAWNTYPE_ALARMBOT},                     // Air Force Guard Post (Cenarion - Stormcrow)
    {22126, 22122, SPAWNTYPE_ALARMBOT}                      // Air Force Trip Wire - Rooftop (Cenarion Expedition)
};

struct npc_air_force_botsAI : public ScriptedAI
{
    npc_air_force_botsAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        SpawnAssoc = NULL;
        spawnedGuid = 0;
        // find the correct spawnhandling
        for (uint8 i = 0; i < 27; ++i)
        {
            if (SpawnAssociations[i].ThisCreatureEntry == me->GetEntry())
            {
                SpawnAssoc = &SpawnAssociations[i];
                break;
            }
        }

        if (!SpawnAssoc)
            error_db_log("SD2: Creature template entry %u has ScriptName npc_air_force_bots, but it's not handled by that script", me->GetEntry());
        else
        {
            CreatureInfo const* spawnedTemplate = GetCreatureTemplateStore(SpawnAssoc->SpawnedCreatureEntry);

            if (!spawnedTemplate)
            {
                error_db_log("SD2: Creature template entry %u does not exist in DB, which is required by npc_air_force_bots", SpawnAssoc->SpawnedCreatureEntry);
                SpawnAssoc = NULL;
                return;
            }
        }
    }

    SpawnAssociation* SpawnAssoc;
    uint64 spawnedGuid;

    void Reset() { }

    Creature* SummonGuard()
    {
        Creature* pSummoned = m_creature->SummonCreature(SpawnAssoc->SpawnedCreatureEntry, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 300000);
        if (pSummoned)
        {
            spawnedGuid = pSummoned->GetGUID();
        }
        else
        {
            error_db_log("SD2: npc_air_force_bots: wasn't able to spawn creature %u", SpawnAssoc->SpawnedCreatureEntry);
            SpawnAssoc = NULL;
        }

        return pSummoned;
    }

    Creature* GetSummonedGuard()
    {
        Creature* pCreature = Unit::GetCreature(*me, spawnedGuid);

        if (pCreature && pCreature->isAlive())
            return pCreature;

        return NULL;
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (!SpawnAssoc)
            return;

        if (pWho->isTargetableForAttack() && m_creature->IsHostileTo(pWho))
        {
            Player* pPlayerTarget = pWho->ToPlayer();

            // airforce guards only spawn for players
            if (!pPlayerTarget)
                return;

            Creature* pLastSpawnedGuard = spawnedGuid == 0 ? NULL : GetSummonedGuard();

            // prevent calling GetCreature at next MoveInLineOfSight call - speedup
            if (!pLastSpawnedGuard)
                spawnedGuid = 0;

            switch (SpawnAssoc->spawnType)
            {
                case SPAWNTYPE_ALARMBOT:
                {
                    if (!pWho->IsWithinDistInMap(m_creature, RANGE_GUARDS_MARK))
                        return;

                    Aura* pMarkAura = pWho->GetAura(SPELL_GUARDS_MARK, 0);
                    if (pMarkAura)
                    {
                        // the target wasn't able to move out of our range within 25 seconds
                        if (!pLastSpawnedGuard)
                        {
                            pLastSpawnedGuard = SummonGuard();

                            if (!pLastSpawnedGuard)
                                return;
                        }

                        if (!pLastSpawnedGuard->GetVictim())
                            pLastSpawnedGuard->AI()->AttackStart(pWho);
                    }
                    else
                    {
                        if (!pLastSpawnedGuard)
                            pLastSpawnedGuard = SummonGuard();

                        if (!pLastSpawnedGuard)
                            return;

                        pLastSpawnedGuard->CastSpell(pWho, SPELL_GUARDS_MARK, true);
                    }
                    break;
                }
                case SPAWNTYPE_TRIPWIRE_ROOFTOP:
                {
                    if (!pWho->IsWithinDistInMap(m_creature, RANGE_TRIPWIRE))
                        return;

                    if (!pLastSpawnedGuard)
                        pLastSpawnedGuard = SummonGuard();

                    if (!pLastSpawnedGuard)
                        return;

                    // ROOFTOP only triggers if the player is on the ground
                    if (!pPlayerTarget->IsFlying())
                    {
                        if (!pLastSpawnedGuard->GetVictim())
                            pLastSpawnedGuard->AI()->AttackStart(pWho);
                    }
                    break;
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_air_force_bots(Creature* pCreature)
{
    return new npc_air_force_botsAI(pCreature);
}

//bool GossipHello_npc_xp_manager(Player *player, Creature *creature)
//{
//    if(player->IsPlayerCustomFlagged(PL_CUSTOM_XP_RATE_FROZEN))
//        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(LANG_SCRIPT_SET_DEFAULT_XP), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
//    else
//        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(LANG_SCRIPT_FREEZE_XP), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
//
//    player->SEND_GOSSIP_MENU(8431, creature->GetGUID());
//    return true;
//}

//bool GossipSelect_npc_xp_manager(Player *player, Creature *creature, uint32 sender, uint32 action)
//{
//    switch (action)
//    {
//        case GOSSIP_ACTION_INFO_DEF+1:
//        {
//            player->RemovePlayerCustomFlag(PL_CUSTOM_XP_RATE_FROZEN);
//            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_XP_SETTED_DEFAULT), player->GetGUID(), true);
//            player->CLOSE_GOSSIP_MENU();
//            break;
//        }
//        case GOSSIP_ACTION_INFO_DEF+2:
//        {
//            player->AddPlayerCustomFlag(PL_CUSTOM_XP_RATE_FROZEN);
//            creature->Whisper(player->GetSession()->GetHellgroundString(LANG_SCRIPT_XP_FROZEN), player->GetGUID(), true);
//            player->CLOSE_GOSSIP_MENU();
//            break;
//        }
//    }
//    return true;
//}

/*######
# npc_soul_trader_beacon
######*/

enum
{
    ETHEREAL_GIVE_TOKEN         = 50063,
    ETHEREAL_STEAL_ESSENCE      = 50101
};

bool GossipHello_npc_soul_trader_beacon(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16509), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetHellgroundString(16510), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
    player->SEND_GOSSIP_MENU(25051, creature->GetGUID());
    return true;
}

bool GossipSelect_npc_soul_trader_beacon(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
        player->SEND_GOSSIP_MENU(25052, creature->GetGUID());
    else if (action == GOSSIP_ACTION_INFO_DEF+2)
        player->SEND_VENDORLIST(creature->GetGUID());

    return true;
}

struct npc_soul_trader_beaconAI : public ScriptedAI
{
    npc_soul_trader_beaconAI(Creature* c) : ScriptedAI(c){}

    Timer GiveTokenTimer;
    Timer SayTimer;
    Timer CheckTimer;
    bool IsIdle;

    void Reset()
    {
        ClearCastQueue();
        GiveTokenTimer = 0;
        SayTimer.Reset(500);
        CheckTimer.Reset(1000);
        IsIdle = false;
        me->GetMotionMaster()->MoveFollow(me->GetOwner(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
    }

    void AttackStart(Unit* who) {}

    void EnterCombat(Unit *who) {}

    void OwnerKilledAndGotXpOrHp(Unit* pOwner, Unit* pTarget)
    {
        me->CastSpell(pTarget, ETHEREAL_STEAL_ESSENCE, false);
        GiveTokenTimer = 3000;
    }

    void UpdateAI(const uint32 diff)
    {
        if(SayTimer.Expired(diff))
        {
            me->Say(-1200670, LANG_UNIVERSAL, 0);
            SayTimer = 0;
        }
        // Check if pet is moving
        if (CheckTimer.Expired(diff))
        {
            if (Unit* pUnit = me->GetOwner())
            {
                Player *pPlayer = pUnit->ToPlayer();

                // Change speed if owner is mounted
                if (pPlayer->IsMounted())
                    me->SetSpeed(MOVE_RUN, 2.0f, true);
                else
                    me->SetSpeed(MOVE_RUN, 1.0f, true);

                // Check if owner is stopped
                if (pPlayer->isMoving() && IsIdle)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_NONE);

                    if (me->IsNonMeleeSpellCast(false))
                        me->InterruptNonMeleeSpells(false);

                    IsIdle = false;
                }
                else if (me->IsWithinDistInMap(pPlayer, 1.5f) && !IsIdle)
                {
                    IsIdle = true;
                }
            }
            CheckTimer = 1000;
        }

        if (GiveTokenTimer.Expired(diff))
        {
            GiveTokenTimer = 0;
            if(Unit *pOwner = me->GetOwner())
            {
                if(pOwner->GetTypeId() == TYPEID_PLAYER)
                    me->CastSpell(pOwner, ETHEREAL_GIVE_TOKEN, false);
            }
            switch (urand(0, 3)) // 50% chance to say random text
            {
                case 0:
                    me->Say(-1200669, LANG_UNIVERSAL, 0);
                    break;
                case 1:
                    me->Say(-1200668, LANG_UNIVERSAL, 0);
                    break;
                default: break;
            }
        }
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI *GetAI_npc_soul_trader_beacon(Creature* c)
{
     return new npc_soul_trader_beaconAI(c);
};

/* hardcore reward */

bool GossipHello_npc_hardcore_reward(Player *player, Creature *creature)
{
	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());

	if (player->GetLevel() == 70 && player->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1))
		player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16569), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
	
	player->SEND_GOSSIP_MENU(995000, creature->GetGUID());
	return true;
}

bool GossipSelect_npc_hardcore_reward(Player *player, Creature *creature, uint32 sender, uint32 action)
{
	if (action == GOSSIP_ACTION_INFO_DEF + 1)
	{
		if (player->GetLevel() == 70 && player->IsPlayerCustomFlagged(PL_CUSTOM_HARDCORE_X1))
		{
			uint32 bag = 23162;
			uint32 mount = 8166;
			uint32 tabard = 695000;
			
			if (!player->CanStoreItemCount(bag, 1) || !player->CanStoreItemCount(mount, 1) || !player->CanStoreItemCount(tabard, 1))
			{
				creature->Whisper(player->GetSession()->GetHellgroundString(802), player->GetGUID());
				return false;
			}

			player->GiveItem(bag, 1);
			player->GiveItem(mount, 1);
			player->GiveItem(tabard, 1);
			player->RemovePlayerCustomFlag(PL_CUSTOM_HARDCORE_X1);
			sLog.outLog(LOG_SPECIAL, "Player %s (guid %u) hardcore rewarded", player->GetName(), player->GetGUIDLow());
		}
	}

	return true;
}
/* hardcore reward */

// npc_heroic_mode
bool GossipHello_npc_heroic_mode(Player *player, Creature *creature)
{
	//if (!player->IsPlayerCustomFlagged(PL_HEROIC_RAID))
	//	player->ADD_GOSSIP_ITEM_EXTENDED(0, player->GetSession()->GetHellgroundString(16589), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1, player->GetSession()->GetHellgroundString(16591), 0, false);
	//	//player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_BATTLE, player->GetSession()->GetHellgroundString(16589), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1, player->GetSession()->GetHellgroundString(16591), 0, false);
	//else
	//	player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16590), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());

	player->SEND_GOSSIP_MENU(990121, creature->GetGUID());
	return true;
}

//bool GossipSelect_npc_heroic_mode(Player *player, Creature *creature, uint32 sender, uint32 action)
//{
//	for (uint8 i = 0; i < TOTAL_DIFFICULTIES; i++)
//	{
//		Player::BoundInstancesMap &binds = player->GetBoundInstances(i);
//		for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
//		{
//			if (IsCustomHeroicMap(itr->first))
//			{
//				ChatHandler(player).SendSysMessage(16588);
//				player->CLOSE_GOSSIP_MENU();
//				return true;
//			}
//			else
//				++itr;
//		}
//	}	
//	
//	if (action == GOSSIP_ACTION_INFO_DEF + 1)
//	{		
//		player->AddPlayerCustomFlag(PL_HEROIC_RAID);
//		ChatHandler(player).SendSysMessage(16593);
//	}
//	else if (action == GOSSIP_ACTION_INFO_DEF + 2)
//	{
//		player->RemovePlayerCustomFlag(PL_HEROIC_RAID);
//		ChatHandler(player).SendSysMessage(16592);
//	}
//
//	player->CLOSE_GOSSIP_MENU();
//	return true;
//}
// npc_heroic_mode

/*######
# pet_shahram
######*/

#define SPELL_BLESSING_OF_SHARAM    16599
#define SPELL_CURSE_OF_SHARAM       16597
#define SPELL_FIST_OF_SHARAM        16601
#define SPELL_FLAMES_OF_SHARAM      16596
#define SPELL_MIGHT_OF_SHARAM       16600
#define SPELL_WILL_OF_SHARAM        16598

struct pet_shahramAI : public PetAI 
{
    pet_shahramAI(Creature* c) : PetAI(c) {}

    Timer RandomSpellTimer;
    Timer DespawnTimer;

    void Reset()
    {
        RandomSpellTimer.Reset(urand(5000, 8000));
        DespawnTimer.Reset(0);
    }

    void UpdateAI(const uint32 diff)
    {
        PetAI::UpdateAI(diff);

        if (RandomSpellTimer.Expired(diff))
        {
            switch(urand(0, 5))
            {
                case 0:
                    m_creature->CastSpell(m_creature, SPELL_BLESSING_OF_SHARAM, false);
                    break;
                case 1:
                    m_creature->CastSpell(m_creature, SPELL_CURSE_OF_SHARAM, false);
                    break;
                case 2:
                    m_creature->CastSpell(m_creature, SPELL_FIST_OF_SHARAM, false);
                    break;
                case 3:
                    m_creature->CastSpell(m_creature, SPELL_FLAMES_OF_SHARAM, false);
                    break;
                case 4:
                    m_creature->CastSpell(m_creature, SPELL_MIGHT_OF_SHARAM, false);
                    break;
                case 5:
                    m_creature->CastSpell(m_creature, SPELL_WILL_OF_SHARAM, false);
                    break;
            }

            DespawnTimer = 2000;
            RandomSpellTimer = 0;
        }

        if (DespawnTimer.Expired(diff))
        {
            ((Pet*)m_creature)->Remove(PET_SAVE_AS_DELETED);

            DespawnTimer = 0;
        }

        if (!UpdateVictim())
            return; 
        
        DoMeleeAttackIfReady();
    }
};

CreatureAI *GetAI_pet_shahram(Creature* c)
{
    return new pet_shahramAI(c);
};

/*######
# pet_furious_mr_pinchy
######*/

#define SPELL_SUNDER_ARMOR  13444

struct pet_furious_mr_pinchyAI : public PetAI 
{
    pet_furious_mr_pinchyAI(Creature* c) : PetAI(c) {}

    Timer SunderArmorTimer;

    void Reset()
    {
        SunderArmorTimer.Reset(urand(2500, 4000));
    }

    void UpdateAI(const uint32 diff)
    {
        PetAI::UpdateAI(diff);

        if (!UpdateVictim())
            return; 

        if (SunderArmorTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_SUNDER_ARMOR, false); 

            SunderArmorTimer = urand(3000, 5000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI *GetAI_pet_furious_mr_pinchy(Creature* c)
{
    return new pet_furious_mr_pinchyAI(c);
};

struct battle_chickenAI : public PetAI
{
	battle_chickenAI(Creature *c) : PetAI(c)
	{
		buff = false;
	}

	bool buff;

	void DamageMade(Unit* target, uint32 &damage, bool direct_damage, uint8 school_mask)
	{
		if (damage && direct_damage)
		{
			if (!buff && roll_chance_f(7))
			{
				m_creature->CastSpell(m_creature, 23060, false); // Battle Squawk
				buff = true;
			}
			else
			if (!m_creature->HasAura(13168) && roll_chance_f(8))
				m_creature->CastSpell(m_creature, 13168, false); // Enrage
		}

	}
};

CreatureAI* GetAI_battle_chicken(Creature* pCreature)
{
	return new battle_chickenAI(pCreature);
}

struct npc_flesh_eating_worm : public ScriptedAI
{
	npc_flesh_eating_worm(Creature* c) : ScriptedAI(c) {}

	void IsSummonedBy(Unit *pSummoner)
	{
		me->SetHealth(1);
		me->SetMaxHealth(1);
		me->SetLevel(pSummoner->GetLevel());
	}
};

CreatureAI *GetAI_npc_flesh_eating_worm(Creature* c)
{
	return new npc_flesh_eating_worm(c);
};

struct npc_debug : public ScriptedAI
{
	npc_debug(Creature* c) : ScriptedAI(c) {}

	Timer t;

	void Reset()
	{
		t.Reset(1000);
	}

	void UpdateAI(const uint32 diff)
	{
		if (!sWorld.getConfig(CONFIG_IS_LOCAL))
			return;
		
		//if (t.Expired(diff))
		//{
		//	float x, y, z;
		//
		//	for (uint8 i = 0; i < 50; ++i)
		//	{
		//		me->GetNearPoint(x, y, z, 0.0f, frand(1.0f,15.0f), frand(0, 2 * M_PI));
		//		Creature * summon = me->SummonCreature(me->GetEntry(), x, y, z, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
		//		me->Kill(summon);
		//	}
		//
		//	t.Reset(100);
		//}

		//if (t.Expired(diff))
		//{
        //    if (me->GetVictim())
        //        me->CastSpell(me, 30210, false);
        //
        //    t = 6000;
		//}

        if (!UpdateVictim())
            return;

        Unit* victim = me->GetVictim();
        if (victim && victim->isFlying())
        {
            return;
        }

        DoMeleeAttackIfReady();

        //CastNextSpellIfAnyAndReady();

	}
};

CreatureAI *GetAI_npc_debug(Creature* c)
{
	return new npc_debug(c);
};

bool GossipSelect_npc_welcome_courier(Player* player, Creature* creature, uint32 sender, uint32 action);
bool GossipHello_npc_welcome_courier(Player* player, Creature* creature)
{
    if (player->GetLevel() == 70)
    {
        GossipSelect_npc_welcome_courier(player, creature, 0, GOSSIP_ACTION_INFO_DEF + 2);
        return true;
    }
    
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16698), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1); // levelup
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15173), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5); // bye
    player->SEND_GOSSIP_MENU(990129, creature->GetGUID()); // Hello, traveler!
    return true;
}

bool GossipSelect_npc_welcome_courier(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    switch (action)
    {
    case GOSSIP_ACTION_INFO_DEF + 1: // levelup
        if (player->GetLevel() < 70)
            player->GiveLevel(70);
        GossipSelect_npc_welcome_courier(player, creature, 0, GOSSIP_ACTION_INFO_DEF + 2);
        break;
    case GOSSIP_ACTION_INFO_DEF + 2: // teleport?
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16699), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(15173), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        player->SEND_GOSSIP_MENU(990130, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF + 3: // teleport
        player->GetSession()->SendBindPoint(creature, -1963.590, 5496.124, -12.427, 3703, 530);
        ChatHandler(player).SendSysMessage(16675);
        player->TeleportTo(530, -1981.874390f, 5505.848145f, -12.427388f, 0.f);
        creature->ForcedDespawn();
        player->CLOSE_GOSSIP_MENU();
        return true;
    case GOSSIP_ACTION_INFO_DEF + 5: // goodbye
        creature->MonsterSay(player->GetSession()->GetHellgroundString(15495), 0, 0);
        creature->ForcedDespawn();
        player->CLOSE_GOSSIP_MENU();
        break;
    }

    return true;
}

bool GossipHello_npc_karazhan_spirit(Player* player, Creature* creature)
{
    ScriptedInstance* pInstance = creature->GetInstanceData();

    if (!pInstance)
        return false;

    bool allBossesDefeated = true;

    std::vector<std::pair<uint32, uint32>> bossData = {
        {DATA_ATTUMEN_EVENT, 16735},
        {DATA_MOROES_EVENT, 16736},
        {DATA_MAIDENOFVIRTUE_EVENT, 16737},
        {DATA_OPERA_EVENT, 16738},
        {DATA_CURATOR_EVENT, 16739},
        {DATA_CHESS_EVENT, 16740},
        {DATA_SHADEOFARAN_EVENT, 16742},
        {DATA_TERESTIAN_EVENT, 16741},
        {DATA_NETHERSPITE_EVENT, 16743}
    };

    for (const auto& boss : bossData) {
        if (pInstance->GetData(boss.first) != DONE) {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(boss.second), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            allBossesDefeated = false;
        }
    }

    player->SEND_GOSSIP_MENU(allBossesDefeated ? 990139 : 990138, creature->GetGUID());
    return true;
}


bool GossipSelect_npc_karazhan_spirit(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    GossipHello_npc_karazhan_spirit(player, creature);
    return true;
}


void AddSC_npc_scripts()
{
    Script *newscript;

	newscript = new Script;
	newscript->Name = "battle_chicken";
	newscript->GetAI = &GetAI_battle_chicken;
	newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "pet_furious_mr_pinchy";
    newscript->GetAI = &GetAI_pet_furious_mr_pinchy;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "pet_shahram";
    newscript->GetAI = &GetAI_pet_shahram;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_garments_of_quests";
    newscript->GetAI = &GetAI_npc_garments_of_quests;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_chicken_cluck";
    newscript->GetAI = &GetAI_npc_chicken_cluck;
    newscript->pReceiveEmote =  &ReceiveEmote_npc_chicken_cluck;
    newscript->pGossipHello =  &GossipHello_npc_chicken_cluck;
    newscript->pQuestAcceptNPC =   &QuestAccept_npc_chicken_cluck;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_chicken_cluck;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_injured_patient";
    newscript->GetAI = &GetAI_npc_injured_patient;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_doctor";
    newscript->GetAI = &GetAI_npc_doctor;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_doctor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_guardian";
    newscript->GetAI = &GetAI_npc_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mount_vendor";
    newscript->pGossipHello =  &GossipHello_npc_mount_vendor;
    newscript->pGossipSelect = &GossipSelect_npc_mount_vendor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_rogue_trainer";
    newscript->pGossipHello =  &GossipHello_npc_rogue_trainer;
    newscript->pGossipSelect = &GossipSelect_npc_rogue_trainer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_sayge";
    newscript->pGossipHello = &GossipHello_npc_sayge;
    newscript->pGossipSelect = &GossipSelect_npc_sayge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tonk_mine";
    newscript->GetAI = &GetAI_npc_tonk_mine;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_winter_reveler";
    newscript->pReceiveEmote =  &ReceiveEmote_npc_winter_reveler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_snake_trap_serpents";
    newscript->GetAI = &GetAI_npc_snake_trap_serpents;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_flight_master";
    newscript->GetAI = &GetAI_npc_flight_master;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_mojo";
    newscript->GetAI = &GetAI_npc_mojo;
    newscript->pReceiveEmote =  &ReceiveEmote_npc_mojo;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_woeful_healer";
    newscript->GetAI = &GetAI_npc_woeful_healer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ring_specialist";
    newscript->pGossipHello = &GossipHello_npc_ring_specialist;
    newscript->pGossipSelect = &GossipSelect_npc_ring_specialist;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_fire_elemental_guardian";
    newscript->GetAI = &GetAI_npc_fire_elemental_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_earth_elemental_guardian";
    newscript->GetAI = &GetAI_npc_earth_elemental_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_master_omarion";
    newscript->pGossipHello =  &GossipHello_npc_master_omarion;
    newscript->pGossipSelect = &GossipSelect_npc_master_omarion;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lorekeeper_lydros";
    newscript->pGossipHello =  &GossipHello_npc_lorekeeper_lydros;
    newscript->pGossipSelect = &GossipSelect_npc_lorekeeper_lydros;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_crashin_trashin_robot";
    newscript->GetAI = &GetAI_npc_crashin_trashin_robot;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "pet_AleMugDrinker";
    newscript->GetAI = GetAI_pet_AleMugDrinker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_land_mine";
    newscript->GetAI = &GetAI_npc_land_mine;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_small_pet_handler";
    newscript->GetAI = &GetAI_npc_small_pet_handler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_combatstop";
    newscript->pGossipHello =  &GossipHello_npc_combatstop;
    newscript->pGossipSelect = &GossipSelect_npc_combatstop;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_resurrect";
    newscript->GetAI = &GetAI_npc_resurrect;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_target_dummy";
    newscript->GetAI = &GetAI_npc_target_dummy;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_explosive_sheep";
    newscript->GetAI = &GetAI_npc_explosive_sheep;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_meridith_the_mermaiden";
    newscript->pGossipHello = &GossipHello_npc_meridith_the_mermaiden;
    newscript->pGossipSelect = &GossipSelect_npc_meridith_the_mermaiden;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_gnomish_flame_turret";
    newscript->GetAI = &GetAI_npc_gnomish_flame_turret;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_nearly_dead_combat_dummy";
    newscript->GetAI = &GetAI_npc_nearly_dead_combat_dummy;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_instakill_guardian";
    newscript->GetAI = &GetAI_npc_instakill_guardian;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="ogrecasterarenarealm";
    newscript->GetAI = &GetAI_npc_ogrecasterarenarealm;
    newscript->pGossipHello = &GossipHello_npc_ogrecasterarenarealm;
    newscript->pGossipSelect = &GossipSelect_npc_ogrecasterarenarealm;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bad_santa";
    newscript->GetAI = &GetAI_npc_bad_santaAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_voodoo_servant";
    newscript->GetAI = &GetAI_npc_voodoo_servant;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_air_force_bots";
    newscript->GetAI = &GetAI_npc_air_force_bots;
    newscript->RegisterSelf();

    //newscript = new Script;
    //newscript->Name="npc_xp_manager";
    //newscript->pGossipHello = &GossipHello_npc_xp_manager;
    //newscript->pGossipSelect = &GossipSelect_npc_xp_manager;
    //newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_arcanite_dragonling";
    newscript->GetAI = &GetAI_npc_arcanite_dragonling;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_soul_trader_beacon";
    newscript->GetAI = &GetAI_npc_soul_trader_beacon;
    newscript->pGossipHello = &GossipHello_npc_soul_trader_beacon;
    newscript->pGossipSelect = &GossipSelect_npc_soul_trader_beacon;
    newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_hardcore_reward";
	newscript->pGossipHello = &GossipHello_npc_hardcore_reward;
	newscript->pGossipSelect = &GossipSelect_npc_hardcore_reward;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_flesh_eating_worm";
	newscript->GetAI = &GetAI_npc_flesh_eating_worm;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_debug";
	newscript->GetAI = &GetAI_npc_debug;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_heroic_mode";
	newscript->pGossipHello = &GossipHello_npc_heroic_mode;
	//newscript->pGossipSelect = &GossipSelect_npc_heroic_mode;
	newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_welcome_courier";
    newscript->pGossipHello = &GossipHello_npc_welcome_courier;
    newscript->pGossipSelect = &GossipSelect_npc_welcome_courier;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_karazhan_spirit";
    newscript->pGossipHello = &GossipHello_npc_karazhan_spirit;
    newscript->pGossipSelect = &GossipSelect_npc_karazhan_spirit;
    newscript->RegisterSelf();
}
