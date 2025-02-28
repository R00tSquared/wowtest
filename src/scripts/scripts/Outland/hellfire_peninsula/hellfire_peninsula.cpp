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
SDName: Hellfire_Peninsula
SD%Complete: 99
 SDComment: Quest support: 9375, 9410, 9418, 10129, 10146, 10162, 10163, 10340, 10346, 10347, 10382 (Special flight paths), 11516, 10909, 10935, 9545, 10351, 10838, 9472, 9483, 10629, 10286
SDCategory: Hellfire Peninsula
EndScriptData */

/* ContentData
npc_aeranas
go_haaleshi_altar
npc_wing_commander_dabiree
npc_gryphoneer_windbellow
npc_wing_commander_brack
npc_wounded_blood_elf
npc_demoniac_scryer
npc_magic_sucker_device_spawner
npc_ancestral_spirit_wolf & npc_earthcaller_ryga
npc_living_flare
npc_felblood_initiate & npc_emaciated_felblood
Ice Stone
npc_hand_berserker
npc_anchorite_relic_bunny
npc_anchorite_barada
npc_darkness_released
npc_foul_purge
npc_sedai_quest_credit_marker
npc_vindicator_sedai
npc_pathaleon_image
npc_viera
npc_deranged_helboar
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"
#include "CreatureGroups.h"

/*######
## npc_aeranas
######*/

#define SAY_SUMMON                      -1000138
#define SAY_FREE                        -1000139

#define FACTION_HOSTILE                 16
#define FACTION_FRIENDLY                35

#define SPELL_ENVELOPING_WINDS          15535
#define SPELL_SHOCK                     12553

#define C_AERANAS                       17085

#define QUEST_ARELIONS_SECRET           10286

struct npc_aeranasAI : public ScriptedAI
{
    npc_aeranasAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked Faction_Timer;
    Timer_UnCheked EnvelopingWinds_Timer;
    Timer_UnCheked Shock_Timer;

    void Reset()
    {
        Faction_Timer.Reset(8000);
        EnvelopingWinds_Timer.Reset(9000);
        Shock_Timer.Reset(5000);

        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->setFaction(FACTION_FRIENDLY);

        DoScriptText(SAY_SUMMON, me);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
            if (Faction_Timer.Expired(diff))
            {
                me->setFaction(FACTION_HOSTILE);
                Faction_Timer = 0;
            }

        if (!UpdateVictim())
            return;

        if (me->GetHealthPercent() < 30)
        {
            me->setFaction(FACTION_FRIENDLY);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->RemoveAllAuras();
            me->DeleteThreatList();
            me->CombatStop();
            DoScriptText(SAY_FREE, me);
            return;
        }

        if (Shock_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SHOCK);
            Shock_Timer = 10000;
        }

        if (EnvelopingWinds_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_ENVELOPING_WINDS);
            EnvelopingWinds_Timer = 25000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_aeranas(Creature *creature)
{
    return new npc_aeranasAI (creature);
}

/*######
## go_haaleshi_altar
######*/

bool GOUse_go_haaleshi_altar(Player *player, GameObject* go)
{
    if (player->GetQuestStatus(9418) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(9418))
        go->SummonCreature(C_AERANAS,-1321.79, 4043.80, 116.24, 1.25, TEMPSUMMON_TIMED_DESPAWN, 180000);
    return false;
}

/*######
## npc_wing_commander_dabiree
######*/

#define GOSSIP_ITEM1_DAB 16436
#define GOSSIP_ITEM2_DAB 16437

bool GossipHello_npc_wing_commander_dabiree(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu( creature->GetGUID() );

    //Mission: The Murketh and Shaadraz Gateways
    if (player->GetQuestStatus(10146) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM1_DAB), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Shatter Point
    if (!player->GetQuestRewardStatus(10340))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM2_DAB), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_wing_commander_dabiree(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33768,true);               //TaxiPath 585 (Gateways Murket and Shaadraz)
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,35069,true);               //TaxiPath 612 (Taxi - Hellfire Peninsula - Expedition Point to Shatter Point)
    }
    return true;
}

/*######
## npc_gryphoneer_windbellow
######*/

#define GOSSIP_ITEM1_WIN 16438
#define GOSSIP_ITEM2_WIN 16439

bool GossipHello_npc_gryphoneer_windbellow(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu( creature->GetGUID() );

    //Mission: The Abyssal Shelf || Return to the Abyssal Shelf
    if (player->GetQuestStatus(10163) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10346) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM1_WIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Go to the Front
    if (player->GetQuestStatus(10382) != QUEST_STATUS_NONE && !player->GetQuestRewardStatus(10382))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM2_WIN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_gryphoneer_windbellow(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33899,true);               //TaxiPath 589 (Aerial Assault Flight (Alliance))
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,35065,true);               //TaxiPath 607 (Taxi - Hellfire Peninsula - Shatter Point to Beach Head)
    }
    return true;
}

/*######
## npc_wing_commander_brack
######*/

#define GOSSIP_ITEM1_BRA 16440
#define GOSSIP_ITEM2_BRA 16441
#define GOSSIP_ITEM3_BRA 16442

bool GossipHello_npc_wing_commander_brack(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu( creature->GetGUID());

    //Mission: The Murketh and Shaadraz Gateways
    if (player->GetQuestStatus(10129) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM1_BRA), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Mission: The Abyssal Shelf || Return to the Abyssal Shelf
    if (player->GetQuestStatus(10162) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10347) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM2_BRA), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    //Spinebreaker Post
    if (player->GetQuestStatus(10242) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(10242))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM3_BRA), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_wing_commander_brack(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player,33659,true);               //TaxiPath 584 (Gateways Murket and Shaadraz)
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player,33825,true);               //TaxiPath 587 (Aerial Assault Flight (Horde))
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player,34578,true);               //TaxiPath 604 (Taxi - Reaver's Fall to Spinebreaker Ridge)
            break;
        default:
            break;
    }
        return true;
}

/*######
## npc_wounded_blood_elf
######*/

#define SAY_ELF_START               -1000117
#define SAY_ELF_SUMMON1             -1000118
#define SAY_ELF_RESTING             -1000119
#define SAY_ELF_SUMMON2             -1000120
#define SAY_ELF_COMPLETE            -1000121
#define SAY_ELF_AGGRO               -1000122

#define QUEST_ROAD_TO_FALCON_WATCH  9375

struct npc_wounded_blood_elfAI : public npc_escortAI
{
    npc_wounded_blood_elfAI(Creature *creature) : npc_escortAI(creature) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();
        if (!player)
            return;

        switch (i)
        {
        case 0:
            DoScriptText(SAY_ELF_START, me, player);
            break;
        case 9:
            DoScriptText(SAY_ELF_SUMMON1, me, player);
            // Spawn two Haal'eshi Talonguard
            DoSpawnCreature(16967, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            DoSpawnCreature(16967, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            break;
        case 13:
            DoScriptText(SAY_ELF_RESTING, me, player);
            // make the NPC kneel
            me->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
            break;
        case 14:
            DoScriptText(SAY_ELF_SUMMON2, me, player);
            // Spawn two Haal'eshi Windwalker
            DoSpawnCreature(16966, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            DoSpawnCreature(16966, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            break;
        case 27:
            DoScriptText(SAY_ELF_COMPLETE, me, player);
            // Award quest credit
            player->GroupEventHappens(QUEST_ROAD_TO_FALCON_WATCH,me);
            break;
        }
    }

    void Reset() { }

    void EnterCombat(Unit* who)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            DoScriptText(SAY_ELF_AGGRO, me);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(me);
    }
};

CreatureAI* GetAI_npc_wounded_blood_elf(Creature* creature)
{
    return new npc_wounded_blood_elfAI(creature);
}

bool QuestAccept_npc_wounded_blood_elf(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ROAD_TO_FALCON_WATCH)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_wounded_blood_elfAI, creature->AI()))
            pEscortAI->Start(true, false, player->GetGUID());

        // Change faction so mobs attack
        creature->setFaction(775);
    }

    return true;
}


/*######
## npc_demoniac_scryer
######*/

#define GOSSIP_ITEM_ATTUNE    16443
#define FINISHED_WHISPER    -1200499

enum
{
    GOSSIP_TEXTID_PROTECT           = 10659,
    GOSSIP_TEXTID_ATTUNED           = 10643,

    QUEST_DEMONIAC                  = 10838,
    NPC_HELLFIRE_WARDLING           = 22259,
    NPC_ORC_HA                      = 22273,
    NPC_BUTTRESS                    = 22267,
    NPC_BUTTRESS_SPAWNER            = 22260,

    MAX_BUTTRESS                    = 4,
    TIME_TOTAL                      = MINUTE*10*MILLISECONDS,

    SPELL_SUMMONED                  = 7741,
    SPELL_DEMONIAC_VISITATION       = 38708,
    SPELL_BUTTRESS_APPERANCE        = 38719,
    SPELL_SUCKER_CHANNEL            = 38721,
    SPELL_SUCKER_DESPAWN_MOB        = 38691,
};

struct npc_demoniac_scryerAI : public ScriptedAI
{
    npc_demoniac_scryerAI(Creature* creature) : ScriptedAI(creature) {}

    bool IfIsComplete;

    Timer_UnCheked SpawnDemonTimer;
    Timer_UnCheked SpawnOrcTimer;
    Timer_UnCheked SpawnButtressTimer;
    Timer_UnCheked EndTimer;
    uint32 ButtressCount;
    std::list<uint64> PlayersWithQuestList;

    void Reset()
    {
        IfIsComplete = false;
        SpawnDemonTimer.Reset(15000);
        SpawnOrcTimer.Reset(30000);
        SpawnButtressTimer.Reset(45000);
        EndTimer.Reset(262000);
        ButtressCount = 0;
        PlayersWithQuestList.clear();

        std::list<Unit*> PlayerList;
        uint32 questDist = 60;                      // sWorld.getConfig(CONFIG_GROUP_XP_DISTANCE);
        Hellground::AnyUnitInObjectRangeCheck  check(me, questDist);
        Hellground::UnitListSearcher<Hellground::AnyUnitInObjectRangeCheck > searcher(PlayerList, check);
        Cell::VisitAllObjects(me, searcher, questDist);

        for(std::list<Unit*>::iterator i = PlayerList.begin(); i != PlayerList.end(); i++)
        {
            if((*i)->GetTypeId() != TYPEID_PLAYER)
                continue;
            Player *player = (Player*)(*i);
            if(player->GetQuestStatus(QUEST_DEMONIAC) == QUEST_STATUS_INCOMPLETE)
                PlayersWithQuestList.push_back(player->GetGUID());
        }
    }

    void AttackedBy(Unit* pEnemy) {}
    void AttackStart(Unit* pEnemy) {}
 
    void DoSpawnButtress()
    {
        ++ButtressCount;

        float fAngle = 0.0f;

        switch(ButtressCount)
        {
            case 1: fAngle = 0.0f; break;
            case 2: fAngle = 4.6f; break;
            case 3: fAngle = 1.5f; break;
            case 4: fAngle = 3.1f; break;
        }

        float fX, fY, fZ;
        me->GetNearPoint(fX, fY, fZ, 0.0f, 5.0f, fAngle);

        uint32 m_Time = TIME_TOTAL - (SpawnButtressTimer.GetTimeLeft() * ButtressCount);
        me->SummonCreature(NPC_BUTTRESS, fX, fY, fZ, me->GetOrientationTo(fX, fY), TEMPSUMMON_TIMED_DESPAWN, m_Time);
        me->SummonCreature(NPC_BUTTRESS_SPAWNER, fX, fY, fZ, me->GetOrientationTo(fX, fY), TEMPSUMMON_TIMED_DESPAWN, m_Time);
    }

    void DoSpawnDemon()
    {
        float fX, fY, fZ;
        me->GetRandomPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 20.0f, fX, fY, fZ);
        me->SummonCreature(NPC_HELLFIRE_WARDLING, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
    }

    void DospawnOrc()
    {
        float fX, fY, fZ;
        me->GetRandomPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 20.0f, fX, fY, fZ);
        me->SummonCreature(NPC_ORC_HA, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_HELLFIRE_WARDLING)
        {
            summoned->CastSpell(summoned, SPELL_SUMMONED, false);
            summoned->AI()->AttackStart(me);
        }
        if (summoned->GetEntry() == NPC_ORC_HA)
        {
            summoned->CastSpell(summoned, SPELL_SUMMONED, false);
            summoned->AI()->AttackStart(me);
        }
        if (summoned->GetEntry() == NPC_BUTTRESS)
        {
            summoned->CastSpell(summoned, SPELL_BUTTRESS_APPERANCE, false);
        }
        else
        {
            if (summoned->GetEntry() == NPC_BUTTRESS_SPAWNER)
            {
                summoned->CastSpell(me, SPELL_SUCKER_CHANNEL, true);
            }
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry* spell)
    {
        if (target->GetEntry() == NPC_BUTTRESS && spell->Id == SPELL_SUCKER_DESPAWN_MOB)
            ((Creature*)target)->setDeathState(CORPSE);
        if (target->GetEntry() == NPC_BUTTRESS_SPAWNER && spell->Id == SPELL_SUCKER_DESPAWN_MOB)
            ((Creature*)target)->setDeathState(CORPSE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (EndTimer.Expired(diff))
        {
            me->setDeathState(CORPSE);
            EndTimer = 262000;
        }

        if (IfIsComplete)
            return;

        if (SpawnButtressTimer.Expired(diff))
        {
            if (ButtressCount >= MAX_BUTTRESS)
            {
                DoCast(me, SPELL_SUCKER_DESPAWN_MOB);
                for(std::list<uint64>::iterator i = PlayersWithQuestList.begin(); i != PlayersWithQuestList.end(); i++)
                {
                    if(Unit* player = Unit::GetUnit(*me, (*i)))
                    {      
                        me->Whisper(-1200499, (*i));
                        //me->CastSpell(player, 38708, true);
                    }
                }
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                IfIsComplete = true;
                return;
            }
            SpawnButtressTimer = 45000;
            DoSpawnButtress();
        }

        if (SpawnDemonTimer.Expired(diff))
        {
            DoSpawnDemon();
            SpawnDemonTimer = 15000;
        }

        if (SpawnOrcTimer.Expired(diff))
        {
            DospawnOrc();
            SpawnOrcTimer = 30000;
        }
    }
};

CreatureAI* GetAI_npc_demoniac_scryer(Creature* creature)
{
    return new npc_demoniac_scryerAI(creature);
}

bool GossipHello_npc_demoniac_scryer(Player* player, Creature* creature)
{
    if (npc_demoniac_scryerAI* pScryerAI = dynamic_cast<npc_demoniac_scryerAI*>(creature->AI()))
    {
        if (pScryerAI->IfIsComplete)
        {
            if (player->GetQuestStatus(QUEST_DEMONIAC) == QUEST_STATUS_INCOMPLETE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_ATTUNE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ATTUNED, creature->GetObjectGuid());
            return true;
        }
    }
    player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_PROTECT, creature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_demoniac_scryer(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        creature->CastSpell(player, SPELL_DEMONIAC_VISITATION, false);
    }
    return true;
}

/*######
## npc_magic_sucker_device_spawner
######*/

enum
{
    SPELL_EFFECT    = 38724,
    NPC_SCRYER      = 22258,
    NPC_BUTTRES     = 22267
};

struct npc_magic_sucker_device_spawnerAI : public ScriptedAI
{
    npc_magic_sucker_device_spawnerAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked CastTimer;
    Timer_UnCheked CheckTimer;

    void Reset()
    {
        CastTimer.Reset(1800);
        CheckTimer.Reset(5000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (CastTimer.Expired(diff))
        {
            DoCast(me, SPELL_EFFECT);
            CastTimer = 1800;
        }

        if (CheckTimer.Expired(diff))
        {
            if (Creature* pScr = GetClosestCreatureWithEntry(me, NPC_SCRYER, 15.0f, false))
            {
                if (Creature* pBut = GetClosestCreatureWithEntry(me, NPC_BUTTRES, 5.0f))
                {
                    pBut->setDeathState(CORPSE);
                    me->setDeathState(CORPSE);
                }
            }

            CheckTimer = 5000;
        }
    }
};
CreatureAI* GetAI_npc_magic_sucker_device_spawner(Creature* creature)
{
    return new npc_magic_sucker_device_spawnerAI(creature);
}

/*######
## npc_ancestral_spirit_wolf & npc_earthcaller_ryga
######*/

enum AncestralSpiritWolf
{
    EMOTE_HEAD_UP                           = -1811000,
    EMOTE_HOWL                              = -1811001,
    EMOTE_RYGA                              = -1811002,
    SPELL_ANCESTRAL_SPIRIT_WOLF_BUFF_TIMER  = 29981,
};

struct npc_earthcaller_rygaAI : public npc_escortAI
{
    npc_earthcaller_rygaAI(Creature *creature) : npc_escortAI(creature) {}

    void Reset() { }

    void WaypointReached(uint32 i)
    {
        switch (i)
        {
            case 1:
                DoScriptText(EMOTE_RYGA, me);
                break;
        }
    }
};

CreatureAI* GetAI_npc_earthcaller_ryga(Creature *creature)
{
    CreatureAI* newAI = new npc_earthcaller_rygaAI(creature);
    return newAI;
}

struct npc_ancestral_spirit_wolfAI : public npc_escortAI
{
    npc_ancestral_spirit_wolfAI(Creature *creature) : npc_escortAI(creature) {}

    void Reset()
    {
        if(!me->HasAura(SPELL_ANCESTRAL_SPIRIT_WOLF_BUFF_TIMER, 0))
            me->AddAura(SPELL_ANCESTRAL_SPIRIT_WOLF_BUFF_TIMER, me);
        me->setFaction(7);
        if(Unit* owner = me->GetOwner())
        {
            if(owner->GetTypeId() == TYPEID_PLAYER)
            {
                Start(false, false, owner->GetGUID());
                SetMaxPlayerDistance(40.0f);
            }
        }
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();
        if (!player)
            return;

        switch (i)
        {
            case 2:
                DoScriptText(EMOTE_HEAD_UP, me);
                break;
            case 6:
                SetRun();
                me->SetSpeed(MOVE_RUN, 0.75);
                DoScriptText(EMOTE_HOWL, me);
                break;
            case 36:
                if(Unit* Ryga = FindCreature(17123, 50, me))
                {
                    if (npc_escortAI* pEscortAI = CAST_AI(npc_earthcaller_rygaAI, ((Creature*)Ryga)->AI()))
                        pEscortAI->Start(false, false, 0, 0, true);
                }
                break;
        }
    }

    void EnterCombat(Unit* who) { return; }
};

CreatureAI* GetAI_npc_ancestral_spirit_wolf(Creature *creature)
{
    CreatureAI* newAI = new npc_ancestral_spirit_wolfAI(creature);
    return newAI;
}

/*######
## npc_living_flare
######*/

enum LivingFlare
{
    NPC_LIVING_FLARE                        = 24916,
    NPC_UNSTABLE_LIVING_FLARE               = 24958,
    NPC_GENERIC_TRIGGER                     = 24959,

    OBJECT_LARGE_FIRE                       = 187084,
    UNSTABLE_LIVING_FLARE_EXPLODE_EMOTE     = -1811010,

    SPELL_FEL_FLAREUP                       = 44944,
    SPELL_LIVING_FLARE_COSMETIC             = 44880,
    SPELL_LIVING_FLARE_MASTER               = 44877,
    SPELL_UNSTABLE_LIVING_FLARE_COSMETIC    = 46196,
    SPELL_LIVING_FLARE_TO_UNSTABLE          = 44943,

    QUEST_BLAST_THE_GATEWAY                 = 11516
};

float FirePos[3][3] = 
{
    {840.9, 2521.0, 293.4},
    {836.5, 2508.0, 292.0},
    {826.5, 2513.4, 291.7}
};

struct npc_living_flareAI : public FollowerAI
{
    npc_living_flareAI(Creature *creature) : FollowerAI(creature) {}

    void Reset()
    {
        if(Unit* owner = me->GetOwner())
        {
            if(owner->GetTypeId() == TYPEID_PLAYER)
                StartFollow(((Player*)owner));
        }
        if(me->GetEntry() == NPC_UNSTABLE_LIVING_FLARE)
            return;

        if(!me->HasAura(SPELL_LIVING_FLARE_COSMETIC, 0))
            me->AddAura(SPELL_LIVING_FLARE_COSMETIC, me);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(who->GetEntry() == NPC_GENERIC_TRIGGER && me->IsWithinDistInMap(who, 10.0f, true))
            Detonate();
    }

    void Detonate()
    {
        if(Unit* owner = me->GetOwner())
        {
            for(uint8 i=0;i<3;++i)
                owner->SummonGameObject(OBJECT_LARGE_FIRE, FirePos[i][0], FirePos[i][1], FirePos[i][2], owner->GetOrientation(), 0, 0, 0, 0, 30);
            if(owner->GetTypeId() == TYPEID_PLAYER)
                if(((Player*)owner)->GetQuestStatus(QUEST_BLAST_THE_GATEWAY) == QUEST_STATUS_INCOMPLETE)
                    ((Player*)owner)->AreaExploredOrEventHappens(QUEST_BLAST_THE_GATEWAY);
        }
        DoCast(me, SPELL_LIVING_FLARE_TO_UNSTABLE);
        DoScriptText(UNSTABLE_LIVING_FLARE_EXPLODE_EMOTE, me);
        me->SetVisibility(VISIBILITY_OFF);
        me->setDeathState(JUST_DIED);
        me->RemoveCorpse();
    }

    void MorphToUnstable()
    {
        if (me->GetEntry() == NPC_UNSTABLE_LIVING_FLARE)
            return;

        DoCast(me, SPELL_FEL_FLAREUP);
        me->UpdateEntry(NPC_UNSTABLE_LIVING_FLARE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        if(Unit* owner = me->GetOwner())
            me->setFaction(owner->getFaction());
        me->RemoveAurasDueToSpell(SPELL_LIVING_FLARE_COSMETIC);
        DoCast(me, SPELL_LIVING_FLARE_TO_UNSTABLE);
        DoCast(me, SPELL_UNSTABLE_LIVING_FLARE_COSMETIC);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_LIVING_FLARE_MASTER)
        {
            uint32 stack = 0;
            if(me->HasAura(SPELL_FEL_FLAREUP, 0))
            {
                if(Aura* Flare = me->GetAura(SPELL_FEL_FLAREUP, 0))
                    stack = Flare->GetStackAmount();
            }
            if(stack < 7)
                DoCast(me, SPELL_FEL_FLAREUP);
            else
                MorphToUnstable();
        }
    }

    void EnterCombat(Unit* who) { return; }
};

CreatureAI* GetAI_npc_living_flare(Creature *creature)
{
    CreatureAI* newAI = new npc_living_flareAI(creature);
    return newAI;
}

struct npc_abyssal_shelf_questAI : public ScriptedAI
{
    npc_abyssal_shelf_questAI(Creature* creature) : ScriptedAI(creature) {}

    void UpdateAI(const uint32 diff)
    {
         if (!UpdateVictim())
             return;

         DoMeleeAttackIfReady();
    }

    void JustDied(Unit* killer)
    {
        me->RemoveCorpse();
    }
};

CreatureAI* GetAI_npc_abyssal_shelf_quest(Creature *creature)
{
    CreatureAI* newAI = new npc_abyssal_shelf_questAI(creature);
    return newAI;
}

/*######
## npc_felblood_initiate & npc_emaciated_felblood
######*/

#define SPELL_FELBLOOD_CHANNEL      44864
#define SPELL_BITTER_WITHDRAWAL     29098
#define SPELL_SINISTER_STRIKE       14873
#define SPELL_SPELLBREAKER          35871
#define SPELL_FEL_SIPHON_QUEST      44936
#define SPELL_SELF_STUN             45066

#define MOB_EMACIATED_FELBLOOD      24955

int32 YellChange[3] = 
{
    -1200500,
    -1200501,
    -1200502
};
int32 YellSiphon[4] = 
{
    -1200503,
    -1200504,
    -1200505,
    -1200506
};

struct npc_felblood_initiateAI : public ScriptedAI
{
    npc_felblood_initiateAI(Creature *creature) : ScriptedAI(creature) { }

    Timer_UnCheked SpellbreakerTimer;
    Timer_UnCheked ChangeTimer;
    Timer_UnCheked OOCTimer;
    Timer_UnCheked BitterWithdrawalTimer;
    Timer_UnCheked SinisterStrikeTimer;

    void Reset()
    {
        SpellbreakerTimer.Reset(urand(6000, 10000));
        ChangeTimer = 0;
        OOCTimer.Reset(5000);
        BitterWithdrawalTimer.Reset(urand(10000, 15000));
        SinisterStrikeTimer.Reset(urand(5000, 15000));
    }

    void HandleOffCombatEffects()
    {
        DoCast((Unit*)NULL, SPELL_FELBLOOD_CHANNEL);
    }

    void EnterCombat(Unit* who)
    {
        if(me->IsNonMeleeSpellCast(false))
            me->InterruptNonMeleeSpells(false);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_FEL_SIPHON_QUEST)
        {
            DoCast(me, SPELL_SELF_STUN);
            ChangeTimer = 3000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
      if(!me->IsInCombat())
      {
          if(OOCTimer.Expired(diff))
          {
              if(!me->IsNonMeleeSpellCast(false))
                  HandleOffCombatEffects();
              if(roll_chance_i(3))
                  me->Yell(YellSiphon[urand(0,3)], 0, 0);
              OOCTimer = 60000;
          }
      }


      if(ChangeTimer.Expired(diff))
      {
          me->UpdateEntry(MOB_EMACIATED_FELBLOOD);
          me->Yell(YellChange[urand(0,2)], 0, 0);
          me->RemoveAurasDueToSpell(SPELL_SELF_STUN);
          me->AI()->AttackStart(me->GetVictim());
          ChangeTimer = 0;
      }

      if (!UpdateVictim())
          return;

      if(me->GetEntry() == MOB_EMACIATED_FELBLOOD)
      {
            if(BitterWithdrawalTimer.Expired(diff))
            {
                AddSpellToCast(SPELL_BITTER_WITHDRAWAL);
                BitterWithdrawalTimer = urand(12000, 18000);
            }

            if(SinisterStrikeTimer.Expired(diff))
            {
                AddSpellToCast(SPELL_SINISTER_STRIKE);
                SinisterStrikeTimer = urand(10000, 15000);
            }
      }
      else
      {
          if(SpellbreakerTimer.Expired(diff))
          {
              AddSpellToCast(SPELL_SPELLBREAKER);
              SpellbreakerTimer = urand(8000, 12000);
          }
      }

       CastNextSpellIfAnyAndReady();
       DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_felblood_initiate(Creature *creature)
{
    CreatureAI* newAI = new npc_felblood_initiateAI(creature);
    return newAI;
}

///////
/// Ice Stone
///////

#define GOSSIP_ICE_STONE        16444

enum
{
    NPC_FROSTWAVE_LIEUTENANT    = 26116,
    NPC_HAILSTONE_LIEUTENANT    = 26178,
    NPC_CHILLWIND_LIEUTENANT    = 26204,
    NPC_FRIGID_LIEUTENANT       = 26214,
    NPC_GLACIAL_LIEUTENANT      = 26215,
    NPC_GLACIAL_TEMPLAR         = 26216,
};

uint32 GetIceStoneQuestID(uint32 zone)
{
    switch (zone)
    {
        case 33:    return 11948;   // Stranglethorn Vale
        case 51:    return 11952;   // Searing Gorge
        case 331:   return 11917;   // Ashenvale
        case 405:   return 11947;   // Desolace
        case 1377:  return 11953;   // Silithus
        case 3483:  return 11954;   // Hellfire Peninsula
        default:    return 0;
    }
}

bool GossipHello_go_ice_stone(Player *player, GameObject *go)
{
    if (uint32 quest = GetIceStoneQuestID(player->GetZoneId()))
    {
        player->PlayerTalkClass->ClearMenus();

        if (player->GetQuestStatus(quest) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ICE_STONE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->SEND_GOSSIP_MENU(go->GetGOInfo()->questgiver.gossipID, go->GetGUID());
    }

    return true;
}

void SendActionMenu_go_ice_stone(Player *player, GameObject* go, uint32 action)
{
    go->SetGoState(GO_STATE_ACTIVE);
    go->SetRespawnTime(300);
    player->CLOSE_GOSSIP_MENU();

    if (action == GOSSIP_ACTION_INFO_DEF)
    {
        uint32 npcId;

        switch (player->GetZoneId())
        {
            case 33:    npcId = NPC_CHILLWIND_LIEUTENANT;   break;  // Stranglethorn Vale
            case 51:    npcId = NPC_FRIGID_LIEUTENANT;      break;  // Searing Gorge
            case 331:   npcId = NPC_FROSTWAVE_LIEUTENANT;   break;  // Ashenvale
            case 405:   npcId = NPC_HAILSTONE_LIEUTENANT;   break;  // Desolace
            case 1377:  npcId = NPC_GLACIAL_LIEUTENANT;     break;  // Silithus
            case 3483:  npcId = NPC_GLACIAL_TEMPLAR;        break;  // Hellfire Peninsula
            default:    return;
        }

        if (GetClosestCreatureWithEntry(player, npcId, 20.0f))
            return;

        float x,y,z;
        player->GetNearPoint(x,y,z, 0.0f, 2.0f, frand(0, M_PI));
        player->SummonCreature(npcId, x, y, z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 600000);
    }
}

bool GossipSelect_go_ice_stone(Player *player, GameObject *go, uint32 sender, uint32 action)
{
    if (sender == GOSSIP_SENDER_MAIN)
        SendActionMenu_go_ice_stone(player, go, action);

    return true;
}

/*######
## npc_hand_berserker
######*/

enum
{
    SPELL_SOUL_BURDEN       = 38879,
    SPELL_ENRAGE            = 8599,
    SPELL_CHARGE            = 35570,

    NPC_BUNNY               = 22444
};

struct npc_hand_berserkerAI : public ScriptedAI
{
    npc_hand_berserkerAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset() {}

    void EnterCombat(Unit* who)
    {
        if (rand()%60)
        {
            DoCast(who, SPELL_CHARGE);
        }
    }   

    void DamageTaken(Unit* doneby, uint32 & damage)
    {
        if (me->HasAura(SPELL_ENRAGE))
            return;

        if (doneby->GetTypeId() == TYPEID_PLAYER && (me->GetHealth()*100 - damage) / me->GetMaxHealth() < 30)
        {
            DoCast(me, SPELL_ENRAGE);
        }
    }

    void JustDied(Unit* who)
    {
        if (Creature* Bunny = GetClosestCreatureWithEntry(me, NPC_BUNNY, 17.5f))
        {
            Bunny->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            DoCast(Bunny, SPELL_SOUL_BURDEN);
        }
    }
};

CreatureAI* GetAI_npc_hand_berserker(Creature* creature)
{
    return new npc_hand_berserkerAI(creature);
}

/*######
## npc_anchorite_relic_bunny
######*/

enum
{
    NPC_HAND_BERSERKER      = 16878,
    NPC_FEL_SPIRIT          = 22454,
    SPELL_CHANNELS          = 39184,

    GO_RELIC                = 185298,
    SAY_SP                  = -1900130
};

struct npc_anchorite_relic_bunnyAI : public ScriptedAI
{
    npc_anchorite_relic_bunnyAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked ChTimer;
    Timer_UnCheked EndTimer;

    void Reset()
    {
        ChTimer.Reset(2000);
        EndTimer.Reset(60000);
    }

    void AttackedBy(Unit* pEnemy) {}
    void AttackStart(Unit* pEnemy) {}

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_FEL_SPIRIT)
        {
            DoScriptText(SAY_SP, summoned);
            summoned->AI()->AttackStart(summoned->GetVictim());
        }
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_SOUL_BURDEN)
        {
            me->InterruptNonMeleeSpells(false);
            me->SummonCreature(NPC_FEL_SPIRIT, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            ChTimer = 2000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (ChTimer.Expired(diff))
        {
            if (Creature* Ber = GetClosestCreatureWithEntry(me, NPC_HAND_BERSERKER, 17.5f, true))
            {
                {
                    DoCast(Ber, SPELL_CHANNELS, false);
                    ChTimer = 95000;
                }
            }
            else me->InterruptNonMeleeSpells(false);
        }

        if (EndTimer.Expired(diff))
        {
            if (GameObject* relic = GetClosestGameObjectWithEntry(me, GO_RELIC, 5.0f))
            {
                relic->RemoveFromWorld();
                me->setDeathState(JUST_DIED);
                me->RemoveCorpse();
            }

            EndTimer = 60000;
        }
    }
};

CreatureAI* GetAI_npc_anchorite_relic_bunny(Creature* creature)
{
    return new npc_anchorite_relic_bunnyAI(creature);
}

/*######
## npc_anchorite_barada
######*/

#define GOSSIP_ITEM_START      16445
#define SAY_BARADA1            -1900100
#define SAY_BARADA2            -1900101
#define SAY_BARADA3            -1900104
#define SAY_BARADA4            -1900105
#define SAY_BARADA5            -1900106
#define SAY_BARADA6            -1900107
#define SAY_BARADA7            -1900108
#define SAY_BARADA8            -1900109
#define SAY_COLONEL1           -1900110
#define SAY_COLONEL2           -1900111
#define SAY_COLONEL3           -1900112
#define SAY_COLONEL4           -1900113
#define SAY_COLONEL5           -1900114
#define SAY_COLONEL6           -1900115
#define SAY_COLONEL7           -1900116
#define SAY_COLONEL8           -1900117

enum
{
    QUEST_THE_EXORCIM          = 10935,
    NPC_COLONEL_JULES          = 22432,
    NPC_DARKNESS_RELEASED      = 22507,
    NPC_FOUL_PURGE             = 22506,

    SPELL_EXORCIM              = 39277,
    SPELL_EXORCIM2             = 39278,
    SPELL_COLONEL1             = 39283,         
    SPELL_COLONEL2             = 39294,
    SPELL_COLONEL3             = 39284,
    SPELL_COLONEL4             = 39295,
    SPELL_COLONEL5             = 39381,
    SPELL_COLONEL6             = 39380
};

struct Points
{
    float x, y, z;
};

static Points P[]=
{
    {-710.111f, 2754.346f, 102.367f},
    {-710.611f, 2753.435f, 103.774f},
    {-707.784f, 2749.562f, 103.774f},
    {-708.558f, 2744.923f, 103.774f},
    {-713.406f, 2744.240f, 103.774f},
    {-714.212f, 2750.606f, 103.774f},
    {-710.792f, 2750.693f, 103.774f},

    {-707.702f, 2749.038f, 101.590f},
    {-710.810f, 2748.376f, 101.590f},
    {-706.726f, 2751.632f, 101.591f},
    {-707.382f, 2753.994f, 101.591f},

    {-710.924f, 2754.683f, 105.0f}
};

struct npc_anchorite_baradaAI : public ScriptedAI
{
    npc_anchorite_baradaAI(Creature* creature) : ScriptedAI(creature) {}

    bool Exorcim;

    Timer_UnCheked StepsTimer;
    uint32 Steps;

    void Reset()
    {
        Exorcim = false;
        StepsTimer.Reset(1);
        Steps = 0;
    }

    void AttackedBy(Unit* who) {}
    void AttackStart(Unit* who) {}

    void DoSpawnDarkness()
    {
        me->SummonCreature(NPC_DARKNESS_RELEASED, P[11].x, P[11].y, P[11].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
    }

    uint32 NextStep(uint32 Steps)
    {
        if (Creature* Colonel = GetClosestCreatureWithEntry(me, NPC_COLONEL_JULES, 15))
        {
            switch(Steps)
            {
                case 1:
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    Colonel->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    return 2000;
                case 2:
                    DoScriptText(SAY_BARADA1, me, 0);
                    return 5000;
                case 3:
                    DoScriptText(SAY_BARADA2, me, 0);
                    return 3000;
                case 4:
                    DoScriptText(SAY_COLONEL1, Colonel, 0);
                    return 3000;
                case 5:
                    me->SetWalk(true);
                    return 3000;
                case 6:
                    me->GetMotionMaster()->MovePoint(0, P[7].x, P[7].y, P[7].z);
                    return 2000;
                case 7:
                    me->GetMotionMaster()->MovePoint(0, P[8].x, P[8].y, P[8].z);
                    return 2100;
                case 8:
                    me->SetFacingToObject(Colonel);
                    return 2000;
                case 9:
                    me->CastSpell(me, SPELL_EXORCIM, false);
                    return 10000;
                case 10:
                    DoScriptText(SAY_BARADA3, me, 0);
                    return 10000;
                case 11:
                    DoScriptText(SAY_COLONEL2, Colonel, 0);
                    return 8000;
                case 12:
                    me->RemoveAllAuras(); 
                    return 1;
                case 13:
                    me->CastSpell(me, SPELL_EXORCIM2, false); 
                    return 1;
                case 14:
                    Colonel->RemoveAllAuras();
                    Colonel->SetSpeed(MOVE_FLIGHT, 0.15f);
                    Colonel->SetLevitate(true);
                    Colonel->CastSpell(Colonel, SPELL_COLONEL3, false);
                    Colonel->GetMotionMaster()->MovePoint(0, P[1].x, P[1].y, P[1].z);
                    return 14000;
                case 15:
                    DoScriptText(SAY_COLONEL3, Colonel, 0);
                    DoSpawnDarkness();
                    DoSpawnDarkness();
                    return 14000;
                case 16:
                    DoScriptText(SAY_BARADA4, me, 0);
                    DoSpawnDarkness();
                    DoSpawnDarkness();
                    return 14000;
                case 17:
                    DoScriptText(SAY_COLONEL4, Colonel, 0);
                    DoSpawnDarkness();
                    return 14000;
                case 18:
                    DoScriptText(SAY_BARADA5, me, 0); 
                    return 14000;
                case 19:
                    Colonel->CastSpell(Colonel, SPELL_COLONEL2, false);
                    DoSpawnDarkness();
                    return 1500;
                case 20:
                    Colonel->GetMotionMaster()->MovePoint(0, P[4].x, P[4].y, P[4].z);
                    return 7000;
                case 21:
                    DoScriptText(SAY_COLONEL5, Colonel, 0);
                    return 1000;
                case 22:
                    Colonel->GetMotionMaster()->MovePoint(0, P[2].x, P[2].y, P[2].z);
                    DoSpawnDarkness();
                    return 5000;
                case 23:
                    Colonel->GetMotionMaster()->MovePoint(0, P[3].x, P[3].y, P[3].z);
                    Colonel->CastSpell(me,SPELL_COLONEL4, false);
                    return 3500;
                case 24:
                    DoScriptText(SAY_BARADA6, me, 0); 
                    return 1;
                case 25:
                    Colonel->GetMotionMaster()->MovePoint(0, P[4].x, P[4].y, P[4].z);
                    DoSpawnDarkness();
                    return 2000;
                case 26:
                    Colonel->GetMotionMaster()->MovePoint(0, P[5].x, P[5].y, P[4].z);
                    return 4000;
                case 27:
                    Colonel->GetMotionMaster()->MovePoint(0, P[2].x, P[2].y, P[2].z);
                    DoScriptText(SAY_COLONEL6, Colonel, 0);
                    DoSpawnDarkness();
                    return 4000;
                case 28:
                    Colonel->GetMotionMaster()->MovePoint(0, P[5].x, P[5].y, P[5].z);
                    return 4000;
                case 29:
                    Colonel->GetMotionMaster()->MovePoint(0, P[2].x, P[2].y, P[2].z);
                    return 4000;
                case 30:
                    DoScriptText(SAY_BARADA7, me, 0); 
                    Colonel->GetMotionMaster()->MovePoint(0, P[3].x, P[3].y, P[3].z);
                    DoSpawnDarkness();
                    return 4000;
                case 31:
                    Colonel->GetMotionMaster()->MovePoint(0, P[4].x, P[4].y, P[4].z);
                    return 4000;
                case 32:
                    Colonel->GetMotionMaster()->MovePoint(0, P[5].x, P[5].y, P[5].z);
                    DoScriptText(SAY_COLONEL7, Colonel, 0);
                    DoSpawnDarkness();
                    return 4000;
                case 33:
                    Colonel->GetMotionMaster()->MovePoint(0, P[4].x, P[4].y, P[4].z);
                    return 4000;
                case 34:
                    Colonel->GetMotionMaster()->MovePoint(0, P[5].x, P[5].y, P[5].z);
                    DoSpawnDarkness();
                    return 4000;
                case 35:
                    DoScriptText(SAY_BARADA5, me, 0);
                    Colonel->GetMotionMaster()->MovePoint(0, P[2].x, P[2].y, P[2].z);
                    return 2500;
                case 36:
                    Colonel->GetMotionMaster()->MovePoint(0, P[3].x, P[3].y, P[3].z);
                    return 4000;
                case 37:
                    Colonel->GetMotionMaster()->MovePoint(0, P[4].x, P[4].y, P[4].z);
                    DoScriptText(SAY_COLONEL8, Colonel, 0);
                    return 4000;
                case 38:
                    Colonel->GetMotionMaster()->MovePoint(0, P[5].x, P[5].y, P[5].z);
                    return 4000;
                case 39:
                    Colonel->GetMotionMaster()->MovePoint(0, P[2].x, P[2].y, P[2].z);
                    return 4000;
                case 40:
                    DoScriptText(SAY_BARADA6, me, 0); 
                    return 1000;
                case 41:
                    Colonel->GetMotionMaster()->MovePoint(0, P[3].x, P[3].y, P[3].z);
                    return 4000;
                case 42:
                    Colonel->GetMotionMaster()->MovePoint(0, P[4].x, P[4].y, P[4].z);
                    Colonel->CastSpell(Colonel, SPELL_COLONEL6, false);
                    return 4000;
                case 43:
                    Colonel->GetMotionMaster()->MovePoint(0, P[5].x, P[5].y, P[5].z);
                    Colonel->CastSpell(Colonel, SPELL_COLONEL5, false);
                    return 4000;
                case 44:
                    Colonel->GetMotionMaster()->MovePoint(0, P[6].x, P[6].y, P[6].z);
                    return 5000;
                case 45:
                    DoScriptText(SAY_BARADA8, me, 0); 
                    Colonel->GetMotionMaster()->MovePoint(0, P[1].x, P[1].y, P[1].z);
                    return 3000;
                case 46:
                    Colonel->GetMotionMaster()->MovePoint(0, P[0].x, P[0].y, P[0].z);
                    Colonel->RemoveAllAuras();
                    me->RemoveAllAuras();
                    me->SetWalk(true);
                    return 2000;
                case 47:
                    me->GetMotionMaster()->MovePoint(0, P[9].x, P[9].y, P[9].z);
                    return 2200;
                case 48:
                     Colonel->CastSpell(Colonel, SPELL_COLONEL1, false); 
                    me->GetMotionMaster()->MovePoint(0, P[10].x, P[10].y, P[10].z);
                    return 7000;
                case 49:
                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                    me->CombatStop();
                    return 3000;
                case 50:
                    Colonel->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    Colonel->SetWalk(true);
                    return 20000;
                case 51:
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    Colonel->SetWalk(false);
                    return 1;
                case 52:
                    Reset();

                default: 
                    return 1;
            }
        }
        Reset();
        return 1;
    }

    void JustDied(Unit* who)
    {
        if (Creature* Colonel = GetClosestCreatureWithEntry(me, NPC_COLONEL_JULES, 15.0f))
        {
            Colonel->GetMotionMaster()->MovePoint(0, P[0].x, P[0].y, P[0].z);
            Colonel->RemoveAllAuras();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (StepsTimer.Expired(diff))
        {
            if (Exorcim)
                StepsTimer.Reset(NextStep(++Steps));
        }
    }
};

CreatureAI* GetAI_npc_anchorite_barada(Creature* creature)
{
    return new npc_anchorite_baradaAI(creature);
}

bool GossipHello_npc_anchorite_barada(Player *player, Creature *creature)
{
    if (player->GetQuestStatus(QUEST_THE_EXORCIM) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_START), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    
    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_anchorite_barada(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        CAST_AI(npc_anchorite_baradaAI, creature->AI())->Exorcim = true;
        player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

/*######
## npc_22432
######*/

enum
{
    SPELL_AURA_ME     = 39303,
    SPELL_DARKNESS    = 39307,
    NPC_BARADA        = 22431
};

bool GossipHello_npc_22432(Player *player, Creature *creature)
{
    if (player->GetQuestStatus(QUEST_THE_EXORCIM) == QUEST_STATUS_INCOMPLETE && creature->IsWalking())
    {
        player->CompleteQuest(QUEST_THE_EXORCIM);
        player->SEND_GOSSIP_MENU(10708, creature->GetGUID());
    }
    else 
        player->SEND_GOSSIP_MENU(10707, creature->GetGUID());

    return true;
}

/*######
## npc_darkness_released
######*/

struct Move
{
    float x, y, z;
};

static Move M[]=
{
    {-714.212f, 2750.606f, 105.0f},
    {-713.406f, 2744.240f, 105.0f},
    {-707.784f, 2749.562f, 105.0f},
    {-708.558f, 2744.923f, 105.0f}
};

struct npc_darkness_releasedAI : public ScriptedAI
{
    npc_darkness_releasedAI(Creature* creature) : ScriptedAI(creature) 
    {
        ChTimer = 5000;
        AtTimer = 10000;
        DoCast(me, SPELL_AURA_ME);
        me->SetLevitate(true);
        me->SetSpeed(MOVE_FLIGHT, 0.08f);
        
        switch(urand(0,3))
        {
            case 0: me->GetMotionMaster()->MovePoint(0, M[0].x, M[0].y, M[0].z); break;
            case 1: me->GetMotionMaster()->MovePoint(0, M[1].x, M[1].y, M[1].z); break;
            case 2: me->GetMotionMaster()->MovePoint(0, M[2].x, M[2].y, M[2].z); break;
            case 3: me->GetMotionMaster()->MovePoint(0, M[3].x, M[3].y, M[3].z); break;
        }
    }

    Timer_UnCheked AtTimer;
    Timer_UnCheked ChTimer;

    void Reset() { }

    void AttackedBy(Unit* who) {}
    void AttackStart(Unit* who) {}

    void JustDied(Unit* who)
    {
        me->RemoveCorpse();
    }

    void UpdateAI(const uint32 diff)
    {
        if (AtTimer.Expired(diff))
        {
            DoCast(me, SPELL_DARKNESS);
            switch (urand(0,3))
            {
                case 0: me->GetMotionMaster()->MovePoint(0, M[0].x, M[0].y, M[0].z); break;
                case 1: me->GetMotionMaster()->MovePoint(0, M[1].x, M[1].y, M[1].z); break;
                case 2: me->GetMotionMaster()->MovePoint(0, M[2].x, M[2].y, M[2].z); break;
                case 3: me->GetMotionMaster()->MovePoint(0, M[3].x, M[3].y, M[3].z); break;
            }

        AtTimer = 10000;
        }

        if (ChTimer.Expired(diff))
        {
            if (Creature* Bar = GetClosestCreatureWithEntry(me, NPC_BARADA, 15.0f, false))
            {
                me->setDeathState(CORPSE);
            }

            if (Creature* Bara = GetClosestCreatureWithEntry(me, NPC_BARADA, 15.0f))
            {
                if (!Bara->HasAura(SPELL_EXORCIM2))
                    me->setDeathState(CORPSE);
            }

            ChTimer = 5000;
        }
    }
};

CreatureAI* GetAI_npc_darkness_released(Creature* creature)
{
    return new npc_darkness_releasedAI(creature);
}

/*######
## npc_foul_purge
######*/

struct npc_foul_purgeAI : public ScriptedAI
{
    npc_foul_purgeAI(Creature* creature) : ScriptedAI(creature) 
    {
        if (Creature* Bara = GetClosestCreatureWithEntry(me, NPC_BARADA, 15.0f))
            AttackStart(Bara);

        ChTimer = 4000;
    }

    Timer_UnCheked ChTimer;

    void Reset() { }

    void JustDied(Unit* who)
    {
        me->RemoveCorpse();
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (ChTimer.Expired(diff))
        {
            if (Creature* Bar = GetClosestCreatureWithEntry(me, NPC_BARADA, 15.0f, false))
            {
                me->setDeathState(CORPSE);
            }

            if (Creature* Bara = GetClosestCreatureWithEntry(me, NPC_BARADA, 15.0f))
            {
                if (!Bara->HasAura(SPELL_EXORCIM2))
                    me->setDeathState(CORPSE);
            }

            ChTimer = 4000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_foul_purge(Creature* creature)
{
    return new npc_foul_purgeAI(creature);
}

/*######
## npc_sedai_quest_credit_marker
######*/

enum
{
    NPC_ESCORT1    = 17417,
    NPC_SEDAI      = 17404
};

struct npc_sedai_quest_credit_markerAI : public ScriptedAI
{
    npc_sedai_quest_credit_markerAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset()
    {
        DoSpawn();
    }

    void DoSpawn()
    {
        me->SummonCreature(NPC_SEDAI, 225.908, 4124.034, 82.505, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 100000);
        me->SummonCreature(NPC_ESCORT1, 229.257, 4125.271, 83.388, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 40000);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_ESCORT1)
        {
            summoned->SetWalk(true);
            summoned->GetMotionMaster()->MovePoint(0, 208.029f, 4134.618f, 77.763f);
        }
    }
};

CreatureAI* GetAI_npc_sedai_quest_credit_marker(Creature* creature)
{
    return new npc_sedai_quest_credit_markerAI(creature);
}

/*######
## npc_vindicator_sedai
######*/

#define SAY_MAG_ESSCORT    -1900125
#define SAY_SEDAI1         -1900126
#define SAY_SEDAI2         -1900127
#define SAY_KRUN           -1900128

enum
{
    NPC_ESCORT        = 17417,
    NPC_AMBUSHER      = 17418,
    NPC_KRUN          = 17405,

    SPELL_STUN        = 13005,
    SPELL_HOLYFIRE    = 17141
};

struct npc_vindicator_sedaiAI : public ScriptedAI
{
    npc_vindicator_sedaiAI(Creature* creature) : ScriptedAI(creature) {}

    bool Vision;

    ObjectGuid PlayerGUID;
    Timer_UnCheked StepsTimer;
    uint32 Steps;

    void Reset()
    {
        Vision = true;
        StepsTimer.Reset(1);
        Steps = 0;
        me->SetWalk(true);
    }

    void DoSpawnEscort()
    {
        me->SummonCreature(NPC_ESCORT, 227.188f, 4121.116f, 82.745f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 40000);
    }

    void DoSpawnAmbusher()
    {
        me->SummonCreature(NPC_AMBUSHER, 223.408f, 4120.086f, 81.843f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000);
    }

    void DoSpawnKrun()
    {
        me->SummonCreature(NPC_KRUN, 192.872f, 4129.417f, 73.655f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 8000);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_ESCORT)
        {
            summoned->SetWalk(true);
            summoned->GetMotionMaster()->MovePoint(0, 205.660f, 4130.663f, 77.175f);
        }

        if (summoned->GetEntry() == NPC_AMBUSHER)
        {
            Creature* pEscort = GetClosestCreatureWithEntry(summoned, NPC_ESCORT, 15);
            summoned->AI()->AttackStart(pEscort);
        }
        else
        {
            if (summoned->GetEntry() == NPC_KRUN)
            {
                summoned->SetWalk(false);
                summoned->GetMotionMaster()->MovePoint(0, 194.739868f, 4143.145996f, 73.798088f);
                DoScriptText(SAY_KRUN, summoned,0);
                summoned->AI()->AttackStart(me);
            }
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)who)->GetQuestStatus(9545) == QUEST_STATUS_INCOMPLETE)
            {
                if (me->IsWithinDistInMap(((Player *)who), 10))
                {
                    PlayerGUID = who->GetObjectGuid();
                }
            }
        }
    }

    uint32 NextStep(uint32 Steps)
    {
        switch(Steps)
        {
            case 1:DoSpawnEscort();
            case 2:me->GetMotionMaster()->MovePoint(0, 204.877f, 4133.172f, 76.897f);return 2900;
            case 3:if (Creature* Esc = GetClosestCreatureWithEntry(me, NPC_ESCORT, 20))
                   {
                       DoScriptText(SAY_MAG_ESSCORT, Esc,0);
                   };
                   return 1000;
            case 4:if (Creature* Esc = GetClosestCreatureWithEntry(me, NPC_ESCORT, 20))
                   {
                       Esc->GetMotionMaster()->MovePoint(0, 229.257f, 4125.271f, 83.388f);
                   };
                   return 1500;
            case 5:if (Creature* Esc = GetClosestCreatureWithEntry(me, NPC_ESCORT, 20))
                   {
                       Esc->GetMotionMaster()->MovePoint(0, 227.188f, 4121.116f, 82.745f);
                   };
                   return 1000;
            case 6:DoScriptText(SAY_SEDAI1, me,0);return 1000;
            case 7:DoSpawnAmbusher();return 3000;
            case 8:DoSpawnAmbusher();return 1000;
            case 9:if (Creature* Amb = GetClosestCreatureWithEntry(me, NPC_AMBUSHER, 35))
                   {
                       me->AI()->AttackStart(Amb);
                   };
                   return 2000;
            case 10:if (Creature* Amb = GetClosestCreatureWithEntry(me, NPC_AMBUSHER, 35))
                    {
                        me->CastSpell(Amb, SPELL_STUN , false);
                    };
                    return 2000;
            case 11:if (Creature* Amb = GetClosestCreatureWithEntry(me, NPC_AMBUSHER, 35))
                    {
                        Amb->setDeathState(JUST_DIED);
                    };
                    return 1500;
            case 12:if (Creature* Esc = GetClosestCreatureWithEntry(me, NPC_ESCORT, 20))
                    {
                        Esc->setDeathState(JUST_DIED);
                    };
            case 13:if (Creature* Amb = GetClosestCreatureWithEntry(me, NPC_AMBUSHER, 35))
                    {
                        me->AI()->AttackStart(Amb);
                    };
            case 14:if (Creature* Amb = GetClosestCreatureWithEntry(me, NPC_AMBUSHER, 35))
                    {
                        if (Creature* pEsc = GetClosestCreatureWithEntry(me, NPC_ESCORT, 20))
                        {
                            pEsc->AI()->AttackStart(Amb);
                        }
                    };
                    return 1000;
            case 15:if (Creature* Amb = GetClosestCreatureWithEntry(me, NPC_AMBUSHER, 35))
                    {
                        me->CastSpell(Amb, SPELL_HOLYFIRE , false);
                    };
                    return 6000;
            case 16:if (Creature* Amb = GetClosestCreatureWithEntry(me, NPC_AMBUSHER, 35))
                    {
                        Amb->setDeathState(JUST_DIED);
                    };
                    return 1000;
            case 17:if (Creature* Esc = GetClosestCreatureWithEntry(me, NPC_ESCORT, 20))
                    {
                        Esc->GetMotionMaster()->MovePoint(0, 235.063f, 4117.826f, 84.471f);
                    };
                    return 1000;
            case 18:me->SetWalk(true);
                    me->GetMotionMaster()->MovePoint(0, 199.706f, 4134.302f, 75.404f);return 6000;       
            case 19:me->GetMotionMaster()->MovePoint(0, 193.524f, 4147.451f, 73.605f);return 7000;              
            case 21:me->SetStandState(UNIT_STAND_STATE_KNEEL);
                    DoScriptText(SAY_SEDAI2, me,0);return 5000;
            case 22:DoSpawnKrun();return 1000;
            case 23:if (Creature* Krun = GetClosestCreatureWithEntry(me, NPC_KRUN, 20))
                    {
                        me->CastSpell(Krun, SPELL_HOLYFIRE, false);
                    };
                    return 3000;
            case 24:if (Creature * Cr = GetClosestCreatureWithEntry(me, 17413, 6.0f))
                    {
                        if (Player* player = me->GetPlayerInWorld(PlayerGUID))
                        {
                            float Radius = 10.0f;
                            if (me->IsWithinDistInMap(player, Radius))
                            {
                                ((Player*)player)->KilledMonster(17413, Cr->GetObjectGuid());
                            }
                        }
                    };
                    return 1500;
            case 25:me->setDeathState(JUST_DIED);
        default: return 1;
        }
    }

    void UpdateAI(const uint32 diff)
    {

        if (StepsTimer.Expired(diff))
        {
            if (Vision)
                StepsTimer = NextStep(++Steps);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_vindicator_sedai(Creature* creature)
{
    return new npc_vindicator_sedaiAI(creature);
}

/*######
## npc_pathaleon_image
######*/

enum
{
    SAY_PATHALEON1         = -1900165,
    SAY_PATHALEON2         = -1900166,
    SAY_PATHALEON3         = -1900167,
    SAY_PATHALEON4         = -1900168,

    SPELL_ROOTS            = 35468,
    SPELL_INSECT           = 35471,
    SPELL_LIGHTING         = 35487,
    SPELL_TELE             = 7741,

    NPC_TARGET_TRIGGER     = 20781,
    NPC_CRYSTAL_TRIGGER    = 20617,
    NPC_GOLIATHON          = 19305,
};

struct Pos
{
    float x, y, z;
};

static Pos S[]=
{
    {113.29f, 4858.19f, 74.37f},
    {81.20f, 4806.26f, 51.75f},
    {106.21f, 4834.39f, 79.56f},
    {124.98f, 4813.17f, 79.66f},
    {124.01f, 4778.61f, 77.86f},
    {46.37f, 4795.72f, 66.73f},
    {60.14f, 4830.46f, 77.83f}
};

struct npc_pathaleon_imageAI : public ScriptedAI
{
    npc_pathaleon_imageAI(Creature* creature) : ScriptedAI(creature) {}

    bool Event;
    bool SummonTrigger;

    Timer SumTimer;
    Timer StepsTimer;
    uint32 Steps;

    Timer TerokkarPhraseTimer;
    uint32 TerokkarPhase;

    void Reset()
    {
        SumTimer.Reset(5000);
        StepsTimer.Reset(1);
        Steps = 0;
        Event = true;
        SummonTrigger = false;
        TerokkarPhraseTimer.Reset(0);
        TerokkarPhase = 0;
    }

    void DoSpawnGoliathon()
    {
        me->SummonCreature(NPC_GOLIATHON, S[0].x, S[0].y, S[0].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
    }

    void DoSpawnTrigger()
    {
        me->SummonCreature(NPC_TARGET_TRIGGER, S[1].x, S[1].y, S[1].z, 2.25f, TEMPSUMMON_TIMED_DESPAWN, 120000);
    }

    void DoSpawnCtrigger()
    {
        me->SummonCreature(NPC_CRYSTAL_TRIGGER, S[2].x, S[2].y, S[2].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 7000);
        me->SummonCreature(NPC_CRYSTAL_TRIGGER, S[3].x, S[3].y, S[3].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 7000);
        me->SummonCreature(NPC_CRYSTAL_TRIGGER, S[4].x, S[4].y, S[4].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 7000);
        me->SummonCreature(NPC_CRYSTAL_TRIGGER, S[5].x, S[5].y, S[5].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 7000);
        me->SummonCreature(NPC_CRYSTAL_TRIGGER, S[6].x, S[6].y, S[6].z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 7000);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_GOLIATHON)
        {
            summoned->CastSpell(summoned, SPELL_TELE, false);
            summoned->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
        }
        if (summoned->GetEntry() == NPC_CRYSTAL_TRIGGER)
        {
            summoned->CastSpell(summoned, SPELL_INSECT, false);
            summoned->CastSpell(summoned, SPELL_LIGHTING, false);
        }
        else
        {
            if (summoned->GetEntry() == NPC_TARGET_TRIGGER)
            {
                summoned->CastSpell(summoned, SPELL_ROOTS, false);
            }
        }
    }

    int32 NextStep(uint32 Steps)
    {              
        switch (Steps)
        {
            case 1:
                return 10000;
            case 2:
                DoSpawnTrigger();
                SummonTrigger = true;
                return 2000;
            case 3:
                DoScriptText(SAY_PATHALEON1, me, 0);
                return 15000;
            case 4:
                DoScriptText(SAY_PATHALEON2, me, 0);
                return 15000;
            case 5:
                DoScriptText(SAY_PATHALEON3, me, 0);
                return 15000;
            case 6:
                DoScriptText(SAY_PATHALEON4, me, 0);
                return 5000;
            case 7:
                DoSpawnGoliathon();
                return 1000;
            case 8:
                DoCast(me, SPELL_TELE);
                return 600;
            case 9:
                me->SetVisibility(VISIBILITY_OFF);
                return 60000;
            case 10:
                me->setDeathState(CORPSE);
            default: return 1;
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
            TerokkarPhraseTimer.Reset(1000);
        else
        {
            me->Say(-1200507, LANG_UNIVERSAL, 0);
            DoCast(me, SPELL_TELE, false);
            me->ForcedDespawn(2000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(me->GetZoneId() == 3519)
        {
            if(!UpdateVictim())
            {
                if(TerokkarPhraseTimer.Expired(diff))
                {
                    switch(TerokkarPhase)
                    {
                        case 0:
                            if(Unit* voldoun = FindCreature(18554, 10, me))
                                ((Creature*)voldoun)->Say(-1200508, LANG_UNIVERSAL, 0);
                            TerokkarPhraseTimer = 2000;
                            TerokkarPhase++;
                            break;
                        case 1:
                            me->Say(-1200509, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 5000;
                            break;
                        case 2:
                            if(Unit* voldoun = FindCreature(18554, 10, me))
                                ((Creature*)voldoun)->Say(-1200510, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 7000;
                            break;
                        case 3:
                            if(Unit* voldoun = FindCreature(18554, 10, me))
                                ((Creature*)voldoun)->Say(-1200511, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 1000;
                            break;
                        case 4:
                            me->Say(-1200512, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 5000;
                            break;
                        case 5:
                            me->Say(-1200513, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 5000;
                            break;
                        case 6:
                            me->Say(-1200514, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 3000;
                            break;
                        case 7:
                            if(Unit* voldoun = FindCreature(18554, 10, me))
                                ((Creature*)voldoun)->Say(-1200515, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 5000;
                            break;
                        case 8:
                            me->Say(-1200516, LANG_UNIVERSAL, 0);
                            TerokkarPhase++;
                            TerokkarPhraseTimer = 2000;
                            break;
                        case 9:
                            DoCast(me, SPELL_TELE, false);
                            me->ForcedDespawn(2000);
                            TerokkarPhase = 0;
                            TerokkarPhraseTimer = 0;
                            break;
                    }
                }
                return;
            }
        }

        if (m_creature->GetDistance(S[0].x, S[0].y, S[0].z) > 200)
            return; // this npc is also summoned in netherstorm

        if (StepsTimer.Expired(diff))
        {
            if (Event)
                StepsTimer.Reset(NextStep(++Steps));
        }

        if (SumTimer.Expired(diff))
        {
            DoSpawnCtrigger();
            SumTimer = 5000;
        }
    }
};

CreatureAI* GetAI_npc_pathaleon_image(Creature* creature)
{
    return new npc_pathaleon_imageAI(creature);
}

/*######
## npc_viera
######*/

#define SAY_VIERA1                       -1900172
#define SAY_VIERA2                       -1900173

#define QUEST_LIVE_IS_FINER_PLEASURES    9483

#define NPC_CAT                          17230

struct npc_vieraAI : public npc_escortAI
{
    npc_vieraAI(Creature* creature) : npc_escortAI(creature) {}

    Timer_UnCheked EndsTimer;

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch (i)
        {
            case 0:
                me->SetStandState(UNIT_STAND_STATE_STAND);
                DoSpawnCreature(NPC_CAT, 5, 5, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 85000);
                break;
            case 9:
                me->SetFacingToObject(player);
                DoScriptText(SAY_VIERA1, me, player);
                me->SetStandState(UNIT_STAND_STATE_SIT);
                EndsTimer = 40000;
                SetEscortPaused(true);
                SetRun();
                break;
            case 10:
                if (Creature* Cat = GetClosestCreatureWithEntry(me, NPC_CAT, 20))
                {
                    Cat->ForcedDespawn();
                }
                break;
        }
    }

    void Reset()
    {
        EndsTimer = 0;
    }

    void EnterCombat(Unit* who) {}

    void JustSummoned(Creature* summoned)
    {
        summoned->GetMotionMaster()->MoveFollow(me, PET_FOLLOW_DIST,  summoned->GetFollowAngle());
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == 30077)
        {
            DoScriptText(SAY_VIERA2, me, 0);
            SetEscortPaused(false);
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (EndsTimer.Expired(diff))
            SetEscortPaused(false);
    }
};

bool QuestRewarded_npc_viera(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_LIVE_IS_FINER_PLEASURES)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_vieraAI, creature->AI()))
            pEscortAI->Start(false, false, player->GetGUID(), quest);
    }
    return true;
}

CreatureAI* GetAI_npc_viera(Creature* creature)
{
    return new npc_vieraAI(creature);
}

/*######
## npc_deranged_helboar
######*/

enum
{
    SPELL_BURNING_SPOKES                           = 33908,
    SPELL_ENRAGES                                  = 8599,
    //SPELL_TELL_DOG_I_JUST_DEAD                     = 37689,
    SPELL_SUMMON_POODAD                            = 37688,

    NPC_FEL_GUARD_HOUND                            = 21847
};

struct npc_deranged_helboarAI : public ScriptedAI
{
    npc_deranged_helboarAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset()
    {

    }

    void EnterCombat(Unit* who)
    {
        DoCast(me, SPELL_BURNING_SPOKES);
    } 

    void DamageTaken(Unit* doneby, uint32 & damage)
    {
        if (me->HasAura(SPELL_ENRAGES))
            return;

        if (doneby->GetTypeId() == TYPEID_PLAYER && (me->GetHealth()*100 - damage) / me->GetMaxHealth() < 30)
        {
            DoCast(me, SPELL_ENRAGES);
        }
    }

    void JustDied(Unit* slayer)
    {
        if(slayer->GetTypeId()==TYPEID_PLAYER && ((Player*)(slayer))->GetQuestStatus(10629)==QUEST_STATUS_INCOMPLETE)
        {
            Unit* Hound = FindCreature(NPC_FEL_GUARD_HOUND, 45, me);

            if(Hound && Hound->GetOwner() == slayer)
            {
                Hound->GetMotionMaster()->MovePoint(100, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                ((Creature*)Hound)->AI()->EventPulse(me, 1);
            }
        }
    }
};

CreatureAI* GetAI_npc_deranged_helboar(Creature* creature)
{
    return new npc_deranged_helboarAI(creature);
}

struct npc_21847AI : public ScriptedAI
{
    npc_21847AI(Creature* creature) : ScriptedAI(creature) {}

    Timer EmotionTimer;
    bool EmotionFlag;
    uint8 EmotionPhase;

    void Reset()
    {
        EmotionTimer.Reset(0);
        EmotionFlag = false;
        EmotionPhase = 0;
        me->SetRooted(false);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        if (PointId == 100)
        {
            me->SetRooted(true);
            EmotionTimer = 100;
            EmotionPhase = 0;
        }
    }

    void EventPulse(Unit* caller, uint32 eventid)
    {
        EmotionFlag = true;
    }

    void UpdateAI(const uint32 diff)
    {
        if(EmotionFlag)
        {
            if(EmotionTimer.Expired(diff))
            {
                switch(EmotionPhase)
                {
                    case 0:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELLCAST);
                        EmotionTimer = 1500;
                        EmotionPhase++;
                        break;
                    case 1:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELLCAST);
                        EmotionTimer = 1000;
                        EmotionPhase++;
                        break;
                    case 2:
                        me->CastSpell(me, SPELL_SUMMON_POODAD, false);
                        Reset();
                        break;
                }
            }
        }

        Unit *pOwner = me->GetOwner();

        if (pOwner)
        {
            if (!pOwner->isAlive())
            {
                me->ForcedDespawn();
                return;
            }
            if (!me->IsWithinDistInMap(pOwner, 41.0f) || !EmotionFlag)
            {
                if (!me->GetVictim()|| !me->IsWithinDistInMap(pOwner, 30.0f))
                {
                    if (!me->HasUnitState(UNIT_STAT_FOLLOW))
                    {
                        me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, urand(M_PI, M_PI/2));
                        Reset();
                        return;
                    }
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_21847(Creature* creature)
{
    return new npc_21847AI(creature);
}

/*###
# Quest 10792 "Zeth'Gor Must Burn!" (Horde) - Visual Effect
####*/

enum Quest10792
{
    SPELL_FIRE                  = 35724,
    GO_FIRE                     = 183816
};
struct npc_east_hovelAI : public ScriptedAI
{
    npc_east_hovelAI(Creature* creature) : ScriptedAI(creature) {}

    bool Summon;
    Timer_UnCheked ResetTimer;
    void Reset()
    {
        Summon = true;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_FIRE)
        {
            if(Summon)
            {
                me->SummonGameObject(GO_FIRE, -934.393005, 1934.01001, 82.031601, 3.35103, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -927.877991, 1927.44995, 81.048897, 5.25344, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -935.54303, 1921.160034, 82.4132, 2.67035, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -944.015015, 1928.160034, 82.105499, 5.98648, 0, 0, 0, 0, 15);
                ResetTimer = 15000;
                Summon = false;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Summon)
        {
            if (ResetTimer.Expired(diff))
            {
                Summon = true;
            }
        }
    }
};
CreatureAI* GetAI_npc_east_hovel(Creature* creature)
{
    return new npc_east_hovelAI(creature);
}

struct npc_west_hovelAI : public ScriptedAI
{
    npc_west_hovelAI(Creature* creature) : ScriptedAI(creature) {}

    bool Summon;
    Timer_UnCheked ResetTimer;
    void Reset()
    {
        Summon = true;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_FIRE)
        {
            if(Summon)
            {
                me->SummonGameObject(GO_FIRE, -1145.410034, 2064.830078, 80.782600, 5.044, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1156.839966, 2060.870117, 79.176399, 3.83972, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1152.719971, 2073.5, 80.622902, 2.00713, 0, 0, 0, 0, 15);
                ResetTimer = 15000;
                Summon = false;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Summon)
        {
            if (ResetTimer.Expired(diff))
            {
                Summon = true;
            }
        }
    }
};
CreatureAI* GetAI_npc_west_hovel(Creature* creature)
{
    return new npc_west_hovelAI(creature);
}

struct npc_stableAI : public ScriptedAI
{
    npc_stableAI(Creature* creature) : ScriptedAI(creature) {}

    bool Summon;
    Timer_UnCheked ResetTimer;
    void Reset()
    {
        Summon = true;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_FIRE)
        {
            if(Summon)
            {
                me->SummonGameObject(GO_FIRE, -1067.280029, 1998.949951, 76.286301, 5.86431, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1052.189941, 2012.099976, 80.946198, 5.95157, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1043.439941, 2002.140015, 76.030502, 2.00713, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1052.26001, 1996.339966, 79.377502, 0.628319, 0, 0, 0, 0, 15);
                ResetTimer = 15000;
                Summon = false;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Summon)
        {
            if (ResetTimer.Expired(diff))
            {
                Summon = true;
            }
        }
    }
};
CreatureAI* GetAI_npc_stable(Creature* creature)
{
    return new npc_stableAI(creature);
}

struct npc_barracksAI : public ScriptedAI
{
    npc_barracksAI(Creature* creature) : ScriptedAI(creature) {}

    bool Summon;
    Timer_UnCheked ResetTimer;
    void Reset()
    {
        Summon = true;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_FIRE)
        {
            if(Summon)
            {
                me->SummonGameObject(GO_FIRE, -1176.709961, 1972.189941, 107.182999, 5.18363, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1120.219971, 1929.890015, 92.360901, 0.89011, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1137.099976, 1951.25, 94.115898, 2.32129, 0, 0, 0, 0, 15);
                me->SummonGameObject(GO_FIRE, -1152.890015, 1961.48999, 92.9795, 0.994838, 0, 0, 0, 0, 15);
                ResetTimer = 15000;
                Summon = false;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Summon)
        {
            if (ResetTimer.Expired(diff))
            {
                Summon = true;
            }
        }
    }
};
CreatureAI* GetAI_npc_barracks(Creature* creature)
{
    return new npc_barracksAI(creature);
}

enum PitCommander
{
    SPELL_SUMMON_INFERNO            = 33393,
    SPELL_INFERNAL_RAIN             = 32785,
    
    SPELL_ARCANE_CHANNELING         = 32783,
    SPELL_ORGRIMMAR_PORTAL_STATE    = 33409,
    SPELL_STORMWIND_PORTAL_STATE    = 33410,
    SUMMON_HORDE_SOLDIER            = 40360,
    SUMMON_ALLIANCE_SOLDIER         = 40361,

    SPELL_INFERNAL_IMMOLATION       = 33650,

    SPELL_EXORCISM                  = 33632,
    SPELL_HAMMER_OF_JUSTICE         = 13005,
    SPELL_SEAL_OF_SACRIFICE         = 13903,
    SPELL_DEMORALIZING_SHOUT        = 23511,
    SPELL_STRIKE                    = 33626,
    SPELL_SHOOT                     = 15620,
    SPELL_FLAME_SHOCK               = 15616,
    SPELL_THROW                     = 10277,
    SPELL_ICEBOLT                   = 33463,
    SPELL_BLIZZARD                  = 33624,
    SPELL_CHAIN_HEAL                = 33642,
    SPELL_CHAIN_LIGHTNING           = 33643,
    SPELL_EARTH_SHOCK               = 22885,
    SPELL_MAGMA_FLOW_TOTEM          = 33560,
    SPELL_REVIVE_SELF               = 32343,
    SPELL_STRENGHTOFTHESTORM_TOTEM  = 33570,
    SPELL_CONSECRATION              = 33559,
    SPELL_DIVINE_SHIELD             = 33581,
    SPELL_FLASH_LIGHT               = 33641,
    SPELL_GREATER_BLESSING_OF_LIGHT = 33564,
    SPELL_JUDGEMENT_OF_COMMAND      = 33554,
    SPELL_CLEAVE                    = 16044,
    SPELL_RAIN_OF_FIRE              = 33627,

    NPC_PORTAL_TRIGGER              = 19871,
    NPC_INFERNAL                    = 18946,
    NPC_INFERNO_RELAY               = 19215,
    NPC_WRATH_MASTER                = 19005,
    NPC_FEL_SOLDIER                 = 18944,
    NPC_IRONFORGE_PALADIN           = 18986,
    NPC_STORMWIND_SOLDIER           = 18948,
    NPC_DARNASSIAN_ARCHER           = 18965,
    NPC_ORGRIMMAR_SHAMAN            = 18972,
    NPC_ORGRIMMAR_GRUNT             = 18950,
    NPC_DARKSPEAR_AXE_THROWER       = 18970,
    MAX_DESYNC                      = 5
};

struct summonCoordType
{
    uint32 creatureId;
    float XYZO[4]; // X Y Z O
};

// 23 different places
const summonCoordType DemonCoords[23] = 
{
    //1 wave
    { NPC_FEL_SOLDIER, -217.816055, 1581.712402, 29.687401, 4.368334 },
    { NPC_FEL_SOLDIER, -215.690277, 1574.370850, 28.797295, 4.292298 },
    { NPC_FEL_SOLDIER, -206.968719, 1571.464111, 28.492876, 4.370838 },
    { NPC_FEL_SOLDIER, -200.960999, 1576.022583, 29.098625, 4.345520 },
    { NPC_WRATH_MASTER, -213.888016, 1564.858765, 27.639652, 4.306069 },

    //2 wave
    { NPC_FEL_SOLDIER, -223.891006, 1576.979980, 29.024300, 4.319790 },
    { NPC_FEL_SOLDIER, -217.385849, 1575.474487, 28.916435, 4.441129 },
    { NPC_FEL_SOLDIER, -205.270020, 1571.724854, 28.536106, 4.417567 },
    { NPC_FEL_SOLDIER, -199.173996, 1568.989990, 28.271700, 4.313210 },
    { NPC_FEL_SOLDIER, -221.716003, 1582.599976, 29.770201, 4.176480 },
    { NPC_FEL_SOLDIER, -215.606705, 1580.905151, 29.603056, 4.387187 },
    { NPC_WRATH_MASTER, -209.444214, 1578.783447, 29.37199, 4.393982 },
    { NPC_FEL_SOLDIER, -203.162476, 1577.285156, 29.234215, 4.446093 },
    { NPC_FEL_SOLDIER, -196.923996, 1574.650024, 28.984900, 4.335130 },
    { NPC_WRATH_MASTER, -211.199997, 1573.270020, 28.687401, 4.409590 },

    //3 wave
    { NPC_FEL_SOLDIER, -223.891006, 1576.979980, 29.024300, 4.319790 },
    { NPC_FEL_SOLDIER, -206.968719, 1571.464111, 28.492876, 4.370838 },
    { NPC_FEL_SOLDIER, -199.173996, 1568.989990, 28.271700, 4.313210 },
    { NPC_FEL_SOLDIER, -221.716003, 1582.599976, 29.770201, 4.176480 },
    { NPC_FEL_SOLDIER, -214.067581, 1580.083252, 29.505596, 4.409739 },
    { NPC_FEL_SOLDIER, -204.930756, 1577.133179, 29.200289, 4.298509 },
    { NPC_FEL_SOLDIER, -196.923996, 1574.650024, 28.984900, 4.335130 },
    { NPC_FEL_SOLDIER, -215.606705, 1580.905151, 29.603056, 4.387187 }
};

struct waves
{
    uint32 startIndex;
    uint32 length;
};

const waves wave[3] =
{
    {0, 5},
    {5, 10},
    {15, 8}
};

struct npc_pit_commanderAI : public ScriptedAI
{
    npc_pit_commanderAI(Creature* creature) : ScriptedAI(creature) 
    {
        activeWaveCreatureGUID = 0;
    }

    Timer SummonInfernoTimer;
    Timer SummonWaveTimer;
    uint64 activeWaveCreatureGUID; // to determine if we should reset timers or not.
    Timer CheckForcesTimer;

    Timer CleaveTimer;
    Timer RainOfFireTimer;
    uint32 WaveType;

    void Reset()
    {
        Unit* active = Unit::GetUnit(*me, activeWaveCreatureGUID);
        if (!active) 
            // wave is still alive -> just continue timer, don't reset it.
        {
            SummonInfernoTimer.Reset(500);
            SummonWaveTimer.Reset(1000);
            CheckForcesTimer.Reset(2000);
            CleaveTimer.Reset(1000);
            RainOfFireTimer.Reset(5000);
            WaveType = WaveType % 3; // any of 0 to 2, when it is initialized already - it won't change
            me->setActive(true);
        }       
    }

    void SummonForce(uint32 Type/*0 - A, 1 - H*/)
    {
        switch(Type)
        {
            case 0:
            {
                uint32 creatureID = 0;
                switch(urand(0,2))
                {
                    case 0: creatureID = NPC_STORMWIND_SOLDIER; break;
                    case 1: creatureID = NPC_IRONFORGE_PALADIN; break;
                    case 2: creatureID = NPC_DARNASSIAN_ARCHER; break;
                    default: break;
                }
                if (Creature* AllianceForce = me->SummonCreature(creatureID, -337.133, 962.42, 54.291561, 1.56, TEMPSUMMON_TIMED_DESPAWN, 85000))
                {
                    AllianceForce->SetWalk(false);
                    AllianceForce->setActive(true);
                    AllianceForce->GetMotionMaster()->MovePath(190051, false);
                }
                break;
            }
            case 1:
            {
                uint32 creatureID = 0;
                switch(urand(0,2))
                {
                    case 0: creatureID = NPC_ORGRIMMAR_GRUNT; break;
                    case 1: creatureID = NPC_ORGRIMMAR_SHAMAN; break;
                    case 2: creatureID = NPC_DARKSPEAR_AXE_THROWER; break;
                    default: break;
                }
                if (Creature* HordeForce = me->SummonCreature(creatureID, -162.525, 963.86, 54.296169, 1.52, TEMPSUMMON_TIMED_DESPAWN, 85000))
                {
                    HordeForce->SetWalk(false);
                    HordeForce->setActive(true);
                    HordeForce->GetMotionMaster()->MovePath(190052, false);
                }
                break;
            }
            default: break;
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 MotionType, uint32 uiData)
    {
        if(MotionType != WAYPOINT_MOTION_TYPE) // LeaderMoveTo in Formation does not have point id's, so can't just add POINT_MOTION_TYPE into consideration
            return;

        if ((pSummoned->GetEntry() == NPC_WRATH_MASTER) || (pSummoned->GetEntry() == NPC_FEL_SOLDIER))
        {
            if(uiData >= 21)
            {
                if (CreatureGroup* gr = pSummoned->GetFormation()) // must stop everyone within our formation and set their home here
                {
                    std::list<Creature*> WrathMasters = FindAllCreaturesWithEntry(NPC_WRATH_MASTER, 100);
                    for (std::list<Creature *>::iterator i = WrathMasters.begin(); i != WrathMasters.end(); i++)
                    {
                        if ((*i)->GetFormation() == gr)
                        {
                            ((*i)->SetHomePosition((*i)->GetPositionX(), (*i)->GetPositionY(), (*i)->GetPositionZ(), (*i)->GetOrientation()));
                            (*i)->SetIsDistanceToHomeEvadable(false);
                            CreatureGroupManager::LeaveGroupIfHas((*i));
                            (*i)->SetAggroRange(80);
                            if (Unit *target = (*i)->SelectNearestTarget(80))
                                (*i)->AI()->AttackStart(target);
                            else
                                (*i)->DisappearAndDie();
                        }
                    }

                    std::list<Creature*> FelSoldiers = FindAllCreaturesWithEntry(NPC_FEL_SOLDIER, 100);
                    for (std::list<Creature *>::iterator i = FelSoldiers.begin(); i != FelSoldiers.end(); i++)
                    {
                        if ((*i)->GetFormation() == gr)
                        {
                            ((*i)->SetHomePosition((*i)->GetPositionX(), (*i)->GetPositionY(), (*i)->GetPositionZ(), (*i)->GetOrientation()));
                            (*i)->SetIsDistanceToHomeEvadable(false);
                            CreatureGroupManager::LeaveGroupIfHas((*i));
                            (*i)->SetAggroRange(80);
                            if (Unit *target = (*i)->SelectNearestTarget(80))
                                (*i)->AI()->AttackStart(target); 
                            else
                                (*i)->DisappearAndDie();
                        }
                    }
                }
                else // no group -> just attack
                {
                    pSummoned->SetHomePosition(pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), pSummoned->GetOrientation());
                    pSummoned->SetIsDistanceToHomeEvadable(false);
                    pSummoned->SetAggroRange(80);
                    if (Unit *target = pSummoned->SelectNearestTarget(80))
                        pSummoned->AI()->AttackStart(target);
                    else
                        pSummoned->DisappearAndDie();
                }
            }
        }

        if ((pSummoned->GetEntry() == NPC_STORMWIND_SOLDIER) || (pSummoned->GetEntry() == NPC_IRONFORGE_PALADIN) || (pSummoned->GetEntry() == NPC_DARNASSIAN_ARCHER) || (pSummoned->GetEntry() == NPC_ORGRIMMAR_GRUNT) || (pSummoned->GetEntry() == NPC_ORGRIMMAR_SHAMAN) || (pSummoned->GetEntry() == NPC_DARKSPEAR_AXE_THROWER))
        {
            if(uiData >= 5)
            {
                pSummoned->SetHomePosition(pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), pSummoned->GetOrientation());
                pSummoned->SetAggroRange(80); // so we can actually findNearestTarget within this range
                switch (pSummoned->GetEntry())
                {
                    case NPC_STORMWIND_SOLDIER:
                    case NPC_IRONFORGE_PALADIN:
                    case NPC_DARNASSIAN_ARCHER:
                        CreatureGroupManager::AddTempCreatureToDBgroup(pSummoned, -24); // Alliance
                        break;
                    case NPC_ORGRIMMAR_GRUNT:
                    case NPC_ORGRIMMAR_SHAMAN:
                    case NPC_DARKSPEAR_AXE_THROWER:
                        CreatureGroupManager::AddTempCreatureToDBgroup(pSummoned, -25); // Horde
                        break;
                }

                if(Unit* target = pSummoned->SelectNearestTarget(80))
                    pSummoned->AI()->AttackStart(target);
            }
        }
    }

    void SummonWaveFormation()
    {
        uint32 groupID = 0; // variable for using
        Creature* varCreature = NULL;
        uint32 last = wave[WaveType].startIndex + wave[WaveType].length - 1;
        for (int32 i = wave[WaveType].startIndex; i <= last; ++i)
        {
            varCreature = me->SummonCreature(
                DemonCoords[i].creatureId,
                DemonCoords[i].XYZO[0], DemonCoords[i].XYZO[1], DemonCoords[i].XYZO[2], DemonCoords[i].XYZO[3],
                TEMPSUMMON_TIMED_DESPAWN, 300000);

            if (!varCreature) // couldn't create!
                continue;

            varCreature->setActive(true);
            if (i == last) // leader must be added last
            {
                groupID = CreatureGroupManager::AddTempCreatureToTempGroup(groupID, varCreature, -1);
                if (CreatureGroup *formation = varCreature->GetFormation())
                {
                    formation->SetLeader(varCreature);
                    varCreature->GetMotionMaster()->MovePath(190050, false);
                    activeWaveCreatureGUID = varCreature->GetGUID();
                }
            }
            else
                groupID = CreatureGroupManager::AddTempCreatureToTempGroup(groupID, varCreature, -2 - i); // -1 is on leader
                                                                                                    // wave 1 = -2 to -5, assigned -1 on last
                                                                                                    // wave 2 = -7 to -15, assigned -1 on last
                                                                                                    // wave 3 =  -17 to -23, assigned -1 on last
        }
        ++WaveType;
        WaveType = WaveType % 3;   
    }

    void UpdateAI(const uint32 diff)
    {
        if(SummonInfernoTimer.Expired(diff))
        {
            std::list<Creature*> InfernoRelay = FindAllCreaturesWithEntry(NPC_INFERNO_RELAY, 150);
            for(std::list<Creature *>::iterator i = InfernoRelay.begin(); i != InfernoRelay.end(); i++)
                DoCast((*i), SPELL_INFERNAL_RAIN, true);
            me->Yell(-1200517, LANG_UNIVERSAL, 0);
            SummonInfernoTimer = 60000;
        }

        if(SummonWaveTimer.Expired(diff))
        {
            SummonWaveFormation();
            SummonWaveTimer = 60000;
        }

        if(CheckForcesTimer.Expired(diff))
        {
            SummonForce(0);
            SummonForce(1);
            CheckForcesTimer = urand(40000, 45000);
        }

        if(!UpdateVictim())
            return;

        if(CleaveTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CLEAVE);
            CleaveTimer = 3000;
        }

        if(RainOfFireTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_RAIN_OF_FIRE, true);
            RainOfFireTimer = 15000;
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_pit_commander(Creature* creature)
{
    return new npc_pit_commanderAI(creature);
}

struct npc_infernal_relayAI : public ScriptedAI
{
    npc_infernal_relayAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset()
    {
    }

    void JustSummoned(Creature* summoned)
    {
        if(!summoned->HasAura(SPELL_INFERNAL_IMMOLATION, 0))
            summoned->AddAura(SPELL_INFERNAL_IMMOLATION, summoned);
        if(Unit* target = summoned->SelectNearestTarget(summoned->GetAggroRange()))
            summoned->AI()->AttackStart(target);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        me->AI()->SendDebug("SpellHit hook called");
        if(spell->Id == SPELL_INFERNAL_RAIN)
        {
            me->AI()->SendDebug("SpellHit with infernal rain");
            if(me->GetDBTableGUIDLow() == 68744 || me->GetDBTableGUIDLow() == 68745)
                me->SummonCreature(NPC_INFERNAL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
            else
                me->SummonCreature(19259, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
        } else me->AI()->SendDebug("SpellHit failed");
    }

    void UpdateAI(const uint32 diff)
    {
    }
};

CreatureAI* GetAI_npc_infernal_relay(Creature* creature)
{
    return new npc_infernal_relayAI(creature);
}

struct npc_19006AI : public ScriptedAI // Silvermoon Magister
{
    npc_19006AI(Creature* creature) : ScriptedAI(creature) {}

    Timer PortalTimer;

    void Reset()
    {
        PortalTimer.Reset(1000);
        me->CastSpell(me, SPELL_ARCANE_CHANNELING, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(PortalTimer.Expired(diff))
        {
            if(Unit* PortalTrigger = FindCreature(NPC_PORTAL_TRIGGER, 10, me))
            {
                if(!PortalTrigger->HasAura(SPELL_ORGRIMMAR_PORTAL_STATE, 0))
                    PortalTrigger->AddAura(SPELL_ORGRIMMAR_PORTAL_STATE, PortalTrigger);
            }
        }
    }
};

CreatureAI* GetAI_npc_19006(Creature* creature)
{
    return new npc_19006AI(creature);
}

struct npc_19007AI : public ScriptedAI // Gnomeregan Conjuror
{
    npc_19007AI(Creature* creature) : ScriptedAI(creature) {}

    Timer PortalTimer;

    void Reset()
    {
        PortalTimer.Reset(1000);
        me->CastSpell(me, SPELL_ARCANE_CHANNELING, true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(PortalTimer.Expired(diff))
        {
            if(Unit* PortalTrigger = FindCreature(NPC_PORTAL_TRIGGER, 10, me))
            {
                if(!PortalTrigger->HasAura(SPELL_STORMWIND_PORTAL_STATE, 0))
                    PortalTrigger->AddAura(SPELL_STORMWIND_PORTAL_STATE, PortalTrigger);
            }
        }
    }
};

CreatureAI* GetAI_npc_19007(Creature* creature)
{
    return new npc_19007AI(creature);
}

struct npc_18986AI : public ScriptedAI // Ironforge Paladin
{
    npc_18986AI(Creature* creature) : ScriptedAI(creature) {}

    Timer ExorcismTimer;
    Timer HammerOfJusticeTimer;
    Timer SealOfSacrificeTimer;
    bool didAttack;

    void Reset()
    {
        ExorcismTimer.Reset(urand(1000, 15000));
        HammerOfJusticeTimer.Reset(urand(1000, 15000));
        SealOfSacrificeTimer.Reset(urand(1000, 15000));
        didAttack = false;
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStart(who);
        didAttack = true;
    }

    void EnterEvadeMode()
    {
        if (me->IsTemporarySummon() && didAttack)
            me->DisappearAndDie();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ExorcismTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_EXORCISM, true);
            ExorcismTimer = urand(5000, 20000);
        }
        
        if(HammerOfJusticeTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_HAMMER_OF_JUSTICE, true);
            HammerOfJusticeTimer = urand(10000, 20000);
        }
        
        if(SealOfSacrificeTimer.Expired(diff))
        {
            std::list<Creature*> MissingBuff= FindFriendlyMissingBuff(50, SPELL_SEAL_OF_SACRIFICE);
            for(std::list<Creature *>::iterator i = MissingBuff.begin(); i != MissingBuff.end(); i++)
            {
                DoCast((*i), SPELL_SEAL_OF_SACRIFICE, true);
                break;
            }
            SealOfSacrificeTimer = urand(3000, 15000);
        }
        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18986(Creature* creature)
{
    return new npc_18986AI(creature);
}

struct npc_18948AI : public ScriptedAI // Stormwind Soldier
{
    npc_18948AI(Creature* creature) : ScriptedAI(creature) {}

    Timer DemoralizingShoutTimer;
    Timer StrikeTimer;
    bool didAttack;

    void Reset()
    {
        DemoralizingShoutTimer.Reset(urand(1000, 30000));
        StrikeTimer.Reset(5000);
        didAttack = false;
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStart(who);
        didAttack = true;
    }

    void EnterEvadeMode()
    {
        if (me->IsTemporarySummon() && didAttack)
            me->DisappearAndDie();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(DemoralizingShoutTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_DEMORALIZING_SHOUT, true);
            DemoralizingShoutTimer = urand(15000, 30000);
        }
        
        if(StrikeTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_STRIKE, true);
            StrikeTimer = 5000;
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18948(Creature* creature)
{
    return new npc_18948AI(creature);
}

struct npc_18965AI : public ScriptedAI // Darnassian Archer
{
    npc_18965AI(Creature* creature) : ScriptedAI(creature) {}

    Timer ShootTimer;
    bool didAttack;

    void Reset()
    {
        ShootTimer.Reset(2000);
        didAttack = false;
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
        didAttack = true;
    }

    void EnterEvadeMode()
    {
        if(me->IsTemporarySummon() && didAttack)
            me->DisappearAndDie();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ShootTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SHOOT, true);
            ShootTimer = 2000;
        }
        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18965(Creature* creature)
{
    return new npc_18965AI(creature);
}

struct npc_18950AI : public ScriptedAI // Orgrimmar Grunt
{
    npc_18950AI(Creature* creature) : ScriptedAI(creature) {}

    Timer DemoralizingShoutTimer;
    Timer StrikeTimer;
    bool didAttack;

    void Reset()
    {
        DemoralizingShoutTimer.Reset(urand(1000, 30000));
        StrikeTimer.Reset(5000);
        didAttack = false;
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStart(who);
        didAttack = true;
    }

    void EnterEvadeMode()
    {
        if(me->IsTemporarySummon() && didAttack)
            me->DisappearAndDie();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(DemoralizingShoutTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_DEMORALIZING_SHOUT, true);
            DemoralizingShoutTimer = urand(15000, 30000);
        }
        
        if(StrikeTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_STRIKE, true);
            StrikeTimer = 5000;
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18950(Creature* creature)
{
    return new npc_18950AI(creature);
}

struct npc_18972AI : public ScriptedAI // Orgrimmar Shaman
{
    npc_18972AI(Creature* creature) : ScriptedAI(creature) {}

    Timer FlameShockTimer;
    bool didAttack;

    void Reset()
    {
        FlameShockTimer.Reset(urand(3000, 5000));
        didAttack = false;
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStart(who);
        didAttack = true;
    }

    void EnterEvadeMode()
    {
        if(me->IsTemporarySummon() && didAttack)
            me->DisappearAndDie();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(FlameShockTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_FLAME_SHOCK, true);
            FlameShockTimer = urand(5000, 8000);
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18972(Creature* creature)
{
    return new npc_18972AI(creature);
}

struct npc_18970AI : public ScriptedAI // Darkspear Axe Thrower
{
    npc_18970AI(Creature* creature) : ScriptedAI(creature) {}

    Timer ShootTimer;
    bool didAttack;

    void Reset()
    {
        ShootTimer.Reset(2000);
        didAttack = false;
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_SHOOTER);
        didAttack = true;
    }
    
    void EnterEvadeMode()
    {
        if(me->IsTemporarySummon() && didAttack)
            me->DisappearAndDie();
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ShootTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_THROW, true);
            ShootTimer = 2000;
        }
        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18970(Creature* creature)
{
    return new npc_18970AI(creature);
}

struct npc_18971_18949AI : public ScriptedAI // Undercity/Stormwind Mage
{
    npc_18971_18949AI(Creature* creature) : ScriptedAI(creature) {}

    Timer IceboltTimer;
    Timer BlizzardTimer;

    void Reset()
    {
        IceboltTimer.Reset(1000);
        BlizzardTimer.Reset(urand(5000, 8000));
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStartNoMove(who, CHECK_TYPE_CASTER);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(IceboltTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_ICEBOLT, true);
            IceboltTimer = 5000;
        }

        if(BlizzardTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_BLIZZARD, true);
            BlizzardTimer = urand(12000, 25000);
        }
        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18971_18949(Creature* creature)
{
    return new npc_18971_18949AI(creature);
}

struct npc_18969AI : public ScriptedAI // Melgromm Highmountain
{
    npc_18969AI(Creature* creature) : ScriptedAI(creature) {}

    Timer ChainHealTimer;
    Timer ChainLightningTimer;
    Timer EarthShockTimer;
    Timer MagmaFlowTotemTimer;
    Timer ReviveSelfTimer;
    Timer StrenghtTotemTimer;
    bool FakeDeath;

    void Reset()
    {
        ChainHealTimer.Reset(5000);
        ChainLightningTimer.Reset(urand(1000, 4000));
        EarthShockTimer.Reset(urand(1000, 3000));
        MagmaFlowTotemTimer.Reset(1000);
        ReviveSelfTimer.Reset(0);
        StrenghtTotemTimer.Reset(urand(5000, 10000));
        FakeDeath = false;
    }

    void DamageTaken(Unit* pKiller, uint32& damage)
    {
        if (damage < me->GetHealth())
            return;

        // prevent death
        damage = 0;
        me->InterruptNonMeleeSpells(false);
        me->StopMoving();
        me->ClearComboPointHolders();
        me->RemoveAllAurasOnDeath();
        me->ClearAllReactives();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
        me->SetStandState(UNIT_STAND_STATE_DEAD);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        ReviveSelfTimer = 3000;
        FakeDeath = true;
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(StrenghtTotemTimer.Expired(diff))
            {
                DoCast(me, SPELL_STRENGHTOFTHESTORM_TOTEM, true);
                StrenghtTotemTimer = 120000;
            }
            return;
        }

        if(!FakeDeath)
        {
            if(ChainHealTimer.Expired(diff))
            {
                if(Unit* pTarget = SelectLowestHpFriendly(50, 0))
                    DoCast(pTarget, SPELL_CHAIN_HEAL, true);
                ChainHealTimer = urand(5000, 12000);
            }
    
            if(ChainLightningTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_CHAIN_LIGHTNING, true);
                ChainLightningTimer = urand(5000, 15000);
            }
    
            if(EarthShockTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_EARTH_SHOCK, true);
                EarthShockTimer = urand(4000, 8000);
            }
    
            if(MagmaFlowTotemTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_MAGMA_FLOW_TOTEM, true);
                MagmaFlowTotemTimer = 21000;
            }
        }
        else
        {
            if(ReviveSelfTimer.Expired(diff))
            {
                DoCast(me, SPELL_REVIVE_SELF, true);
                me->SetHealth(me->GetMaxHealth());
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                ReviveSelfTimer = 0;
                FakeDeath = false;
            }
        }
        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18969(Creature* creature)
{
    return new npc_18969AI(creature);
}

struct npc_18966AI : public ScriptedAI // Justinius the Harbringer
{
    npc_18966AI(Creature* creature) : ScriptedAI(creature) {}

    Timer ConsecrationTimer;
    Timer FlashOfLightTimer;
    Timer BlessingOfMightTimer;
    Timer JudgementOfCommandTimer;

    void Reset()
    {
        ConsecrationTimer.Reset(5000);
        FlashOfLightTimer.Reset(urand(4000, 8000));
        BlessingOfMightTimer.Reset(1000);
        JudgementOfCommandTimer.Reset(5000);
    }

    void EnterCombat(Unit *who)
    {
        me->Yell(-1200518, LANG_UNIVERSAL, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(FlashOfLightTimer.Expired(diff))
        {
            if(Unit* pTarget = SelectLowestHpFriendly(50, 0))
                DoCast(pTarget, SPELL_FLASH_LIGHT, true);
            FlashOfLightTimer = urand(4000, 8000);
        }
    
        if(ConsecrationTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CONSECRATION, true);
            ConsecrationTimer = 21000;
        }
    
        if(BlessingOfMightTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_GREATER_BLESSING_OF_LIGHT, true);
            BlessingOfMightTimer = 31000;
        }
    
        if(JudgementOfCommandTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_JUDGEMENT_OF_COMMAND, true);
            JudgementOfCommandTimer = urand(5000, 10000);
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18966(Creature* creature)
{
    return new npc_18966AI(creature);
}

struct npc_18944_19005AI : public ScriptedAI // Warth Master/Fel Soldier
{
    npc_18944_19005AI(Creature* creature) : ScriptedAI(creature) {}

    Timer OOCTImer;

    void Reset()
    {
        OOCTImer.Reset(0);
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStart(who);
        OOCTImer.Reset(3000);
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (!me->IsInCombat() && OOCTImer.Expired(diff))
                me->DisappearAndDie();
            return;
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_18944_19005(Creature* creature)
{
    return new npc_18944_19005AI(creature);
}

/*######
## Warlord Mokh & Eye of the Citadel
######*/

enum EyeOfTheCitadel
{
    SPAWN_EMOTE = -1811021,
    SAY_SPAWN_1 = -1811022,
    SAY_SPAWN_2 = -1811023,
};

struct npc_eye_of_the_citadelAI : public ScriptedAI
{
    npc_eye_of_the_citadelAI(Creature* creature) : ScriptedAI(creature) {}

    Timer SayTimer;
    bool Said1;
    uint64 pKiller;

    void Reset()
    {
        SayTimer.Reset(3000);
        Said1 = false;
        DoScriptText(SPAWN_EMOTE, me, me);
    }

    void UpdateAI(const uint32 diff)
    {
        if (SayTimer.Expired(diff))
        {
            if(!Said1)
            {
                DoScriptText(SAY_SPAWN_1, me, me->GetUnit(pKiller));
                Said1 = true;
                SayTimer = 4000;
            }
            else
            {
                DoScriptText(SAY_SPAWN_2, me, me->GetUnit(pKiller));
                SayTimer = 0;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_eye_of_the_citadelAI(Creature *creature)
{
    return new npc_eye_of_the_citadelAI (creature);
}

enum WarlordMokh
{
    SPELL_RIPOSTE_STANCE    = 34080,
    SPELL_KICK              = 11978,
    SPELL_ENRAGE_MOKH       = 8599,

    NPC_EYE_OF_THE_CITADEL  = 21134,

    SAY_NEAR_DEATH          = -1811020,
};

struct npc_warlord_mokhAI : public ScriptedAI
{
    npc_warlord_mokhAI(Creature* creature) : ScriptedAI(creature) {}

    Timer RiposteStanceTimer;
    Timer KickTimer;
    bool Enraged;

    void Reset()
    {
        RiposteStanceTimer.Reset(9000);
        KickTimer.Reset(7000);
        Enraged = false;
    }

    void JustDied(Unit* pKiller)
    {
        if (Creature* spawn = me->SummonCreature(NPC_EYE_OF_THE_CITADEL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientationTo(pKiller->GetPositionX(), pKiller->GetPositionY()), TEMPSUMMON_TIMED_DESPAWN, 13000, false))
        {
            if (npc_eye_of_the_citadelAI* eotc = dynamic_cast<npc_eye_of_the_citadelAI*>(spawn->AI()))
                eotc->pKiller = pKiller->GetGUID();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!Enraged && me->GetHealthPercent() < 20)
        {
            DoCast(me, SPELL_ENRAGE_MOKH);
            DoScriptText(SAY_NEAR_DEATH, me);
            Enraged = true;
        }

        if (RiposteStanceTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_RIPOSTE_STANCE);
            RiposteStanceTimer = 17000;
        }

        if (KickTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_KICK);
            KickTimer = 8000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_warlord_mokhAI(Creature *creature)
{
    return new npc_warlord_mokhAI (creature);
}

enum RuinsOfShanaara
{
    SPELL_DEMORALIZING_SHOUT_S  = 16244,
    SPELL_HASTEN                = 34186
};

struct npc_17058_16938AI : public ScriptedAI
{
    npc_17058_16938AI(Creature* creature) : ScriptedAI(creature) {}

    Timer DemoralizingShoutTimer;
    Timer SayOOC;
    bool Hasten;

    void Reset()
    {
        DemoralizingShoutTimer.Reset(2000);
        SayOOC.Reset(urand(10000, 60000));
        Hasten = false;
    }

    void EnterCombat(Unit *)
    {
        std::list<Creature*> alist;
        alist.splice(alist.begin(), FindAllCreaturesWithEntry(16938, 20));
        alist.splice(alist.begin(), FindAllCreaturesWithEntry(16937, 20));

        if(!alist.empty())
        {
            switch(urand(0,2))
            {
                case 0:
                    me->Say(-1200519, LANG_UNIVERSAL, 0);
                    break;
                case 1:
                    me->Say(-1200520, LANG_UNIVERSAL, 0);
                    break;
                case 2:
                    me->Say(-1200521, LANG_UNIVERSAL, 0);
                    break;
            }

            for(std::list<Creature*>::iterator itr = alist.begin(); itr != alist.end(); itr++)
            {
                if ((*itr)->isAlive())
                {
                    (*itr)->AI()->AttackStart(me->GetVictim());
                    switch(urand(0,3))
                    {
                        case 0:
                            (*itr)->Say(-1200522, LANG_UNIVERSAL, 0);
                            break;
                        case 1:
                            (*itr)->Say(-1200523, LANG_UNIVERSAL, 0);
                            break;
                        case 2:
                            (*itr)->Say(-1200524, LANG_UNIVERSAL, 0);
                            break;
                        case 3: // to prevent spam
                            break;
                    }
                }
            }
        }
        else
            me->Say(-1200525, LANG_UNIVERSAL, 0);
    }

    void JustDied(Unit* )
    {
        std::list<Creature*> alist;
        alist.splice(alist.begin(), FindAllCreaturesWithEntry(16938, 15));
        alist.splice(alist.begin(), FindAllCreaturesWithEntry(16937, 15));
        for(std::list<Creature*>::iterator itr = alist.begin(); itr != alist.end(); itr++)
        {
            if ((*itr)->isAlive())
            {
                (*itr)->RemoveAllAuras();
                (*itr)->DeleteThreatList();
                (*itr)->CombatStop();
                (*itr)->GetMotionMaster()->MovePoint(1, -434.6f, 4717.91, 25.417);
                (*itr)->ForcedDespawn(7000);
                (*itr)->setFaction(35);
                switch(urand(0,2))
                {
                    case 0:
                        (*itr)->Say(-1200526, LANG_UNIVERSAL, 0);
                        break;
                    case 1:
                        (*itr)->Say(-1200527, LANG_UNIVERSAL, 0);
                        break;
                    case 2:
                        (*itr)->Say(-1200528, LANG_UNIVERSAL, 0);
                        break;
                }
            } 
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(SayOOC.Expired(diff))
            {
                // say next only if 16938 near
                std::list<Creature*> alist = FindAllCreaturesWithEntry(16938, 20);
                if(!alist.empty())
                {
                    switch(urand(0,2))
                    {
                        case 0:
                            me->Say(-1200529, LANG_UNIVERSAL, 0);
                            break;
                        case 1:
                            me->Say(-1200530, LANG_UNIVERSAL, 0);
                            break;
                        case 2:
                            me->Say(-1200531, LANG_UNIVERSAL, 0);
                            break;
                    }
                }
                SayOOC = urand(25000, 90000);
            }
            return;
        }

        if(DemoralizingShoutTimer.Expired(diff))
        {
            DoCast(me, SPELL_DEMORALIZING_SHOUT_S, false);
            DemoralizingShoutTimer = 16000;
        }

        if(!Hasten && me->GetHealthPercent() < 20.0)
        {
            DoCast(me, SPELL_HASTEN, false);
            Hasten = true;
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_17058_16938(Creature* creature)
{
    return new npc_17058_16938AI(creature);
}

struct npc_19294AI : public ScriptedAI
{
    npc_19294AI(Creature *c) : ScriptedAI(c), Summons(me)
    {
        MoveTimer.Reset(0);
        WaitTimer.Reset(0);
        Action = 0;
    }

    Timer MoveTimer;
    Timer WaitTimer;
    uint8 Action;
    SummonList Summons;

    void Reset()
    {
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned)
        {
            summoned->AI()->AttackStart(me);
            me->AI()->AttackStart(summoned);
        }
        Summons.Summon(summoned);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        if (PointId == 1)
        {
            me->HandleEmoteCommand(16);
            WaitTimer = 1500;
        }
    }

    void SummonedCreatureDies(Creature* summon, Unit*)
    {
        WaitTimer = 1000;
    }

    void JustReachedHome()
    {
        MoveTimer.Reset(0);
        WaitTimer.Reset(0);
        Action = 0;
        Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if(MoveTimer.Expired(diff))
        {
            me->GetMotionMaster()->MovePoint(1, -286.7666, 4729.429, 18.4418, true, true, UNIT_ACTION_DOWAYPOINTS);
            MoveTimer = 0;
        }

        if(WaitTimer.Expired(diff))
        {
            switch(Action)
            {
                case 0:
                    me->HandleEmoteCommand(0);
                    me->SummonGameObject(184450, -287.0193, 4731.628, 18.21704, 2.583081, 0, 0, 0, 0, 30000);
                    WaitTimer = 3000;
                    Action++;
                    break;
                case 1:
                    me->Say(-1200532, LANG_UNIVERSAL, 0);
                    me->SummonCreature(20599, -288.1897, 4733.63, 18.29823, 5.044002, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
                    WaitTimer = 0;
                    Action++;
                    break;
                case 2:
                    me->GetMotionMaster()->MoveTargetedHome();
                    WaitTimer = 0;
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19294(Creature* creature)
{
    return new npc_19294AI(creature);
}

bool QuestComplete_npc_19294(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 10349)
    {
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->SetOrientation(creature->GetOrientationTo(player));
        creature->SetFacingTo(creature->GetOrientationTo(player));
        creature->Say(-1200533, LANG_UNIVERSAL, 0);
        creature->HandleEmoteCommand(1);
        ((npc_19294AI*)creature->AI())->MoveTimer = 3000;
    }
    return true;
}

struct npc_19293AI : public ScriptedAI
{
    npc_19293AI(Creature *c) : ScriptedAI(c)
    {
    }

    void Reset()
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19293(Creature* creature)
{
    return new npc_19293AI(creature);
}

bool QuestAccept_npc_19293(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 10349)
    {
        if(Unit* qtaker = FindCreature(19294, 50, pCreature))
            ((Creature*)qtaker)->Whisper(-1200534, pPlayer->GetGUID());
    }
    return true;
}

struct npc_20679AI : public ScriptedAI
{
    npc_20679AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer KneelTimer;

    void Reset()
    {
        KneelTimer.Reset(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(KneelTimer.Expired(diff))
        {
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
            KneelTimer = 0;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_20679(Creature* creature)
{
    return new npc_20679AI(creature);
}

bool GossipHello_npc_20679(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(10368) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10368, 20679) < 1)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16448), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(25055, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_20679(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->KilledMonster(20679, creature->GetGUID());
        creature->Say(-1200536, LANG_UNIVERSAL, player->GetGUID());
        if (npc_20679AI* questnpc = CAST_AI(npc_20679AI, creature->AI()))
        {
            questnpc->KneelTimer = 5000;
            creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
        }
    }
    return true;
}

struct npc_20678AI : public ScriptedAI
{
    npc_20678AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer KneelTimer;

    void Reset()
    {
        KneelTimer.Reset(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(KneelTimer.Expired(diff))
        {
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
            KneelTimer = 0;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_20678(Creature* creature)
{
    return new npc_20678AI(creature);
}

bool GossipHello_npc_20678(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(10368) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10368, 20678) < 1)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16446), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(25053, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_20678(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->KilledMonster(20678, creature->GetGUID());
        creature->Say(-1200537, LANG_UNIVERSAL, player->GetGUID());
        if (npc_20678AI* questnpc = CAST_AI(npc_20678AI, creature->AI()))
        {
            questnpc->KneelTimer = 5000;
            creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
        }
    }
    return true;
}

struct npc_20677AI : public ScriptedAI
{
    npc_20677AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer KneelTimer;

    void Reset()
    {
        KneelTimer.Reset(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(KneelTimer.Expired(diff))
        {
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_KNEEL);
            KneelTimer = 0;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_20677(Creature* creature)
{
    return new npc_20677AI(creature);
}

bool GossipHello_npc_20677(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(10368) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(10368, 20677) < 1)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16447), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(25054, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_20677(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->KilledMonster(20677, creature->GetGUID());
        creature->Say(-1200535, LANG_UNIVERSAL, player->GetGUID());
        if (npc_20677AI* questnpc = CAST_AI(npc_20677AI, creature->AI()))
        {
            questnpc->KneelTimer = 5000;
            creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
        }
    }
    return true;
}

struct npc_16925_19424AI : public ScriptedAI
{
    npc_16925_19424AI(Creature *c) : ScriptedAI(c), Summons(me)
    {
    }

    SummonList Summons;
    Timer SpellTimer;
    bool Fled;

    void Reset()
    {
        me->Mount(me->GetEntry() == 16925 ? 17408 : 9562);
        Summons.DespawnAll();
        SpellTimer.Reset(5000);
        Fled = false;
    }

    void EnterCombat(Unit* who)
    {
        me->CastSpell(me, me->GetEntry() == 16925 ? 32723 : 34368, false);
        me->Unmount();
        me->SetSpeed(MOVE_RUN, 1.1);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned)
        {
            if(me->GetVictim())
                summoned->AI()->AttackStart(me->GetVictim());
        }
        Summons.Summon(summoned);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(me->GetEntry() == 19424)
        {
            if(SpellTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), 33924, false);
                SpellTimer = 10000;
            }

            if(!Fled && me->HealthBelowPct(15))
            {
                me->CastSpell(me->GetVictim(), 31553, false);
                me->GetMotionMaster()->MoveFleeing(me->GetVictim(), 10);
                DoTextEmote(-1200538, NULL);
                Fled = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_16925_19424(Creature* creature)
{
    return new npc_16925_19424AI(creature);
}

struct npc_18706AI : public ScriptedAI
{
    npc_18706AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer FesteringBiteTimer;
    Timer FuriousHowlTimer;

    void Reset()
    {
        FesteringBiteTimer.Reset(3000);
        FuriousHowlTimer.Reset(5000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(FesteringBiteTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 16460, false);
            FesteringBiteTimer = urand(15000, 20000);
        }

        if(FuriousHowlTimer.Expired(diff))
        {
            DoCast(me, 3149, false);
            FuriousHowlTimer = urand(15000, 20000);
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_18706(Creature* creature)
{
    return new npc_18706AI(creature);
}

struct npc_19422AI : public ScriptedAI
{
    npc_19422AI(Creature *c) : ScriptedAI(c), Summons(c)
    {
    }

    SummonList Summons;
    Timer CurseOfBHTimer;
    Timer FireballTimer;
    bool RaiseDead;
    bool RunAway;

    void Reset()
    {
        CurseOfBHTimer.Reset(1000);
        FireballTimer.Reset(7000);
        RaiseDead = false;
        RunAway = false;
        SummonFormation();
    }

    void JustSummoned(Creature* summoned)
    {
        Summons.Summon(summoned);
    }

    void SummonFormation()
    {
        Summons.DespawnAll();
        for(uint8 i = 0; i < 2; ++i)
        {
            float px, py, pz;
            me->GetNearPoint(px, py, pz, 1, urand(1.0, 3.0), urand(1.0, 5.0));
            Creature* skeleton = me->SummonCreature(19460, px, py, pz, urand(0, 5), TEMPSUMMON_MANUAL_DESPAWN, 0, true);
            if(skeleton)
                skeleton->GetMotionMaster()->MoveFollow(me, PET_FOLLOW_DIST, urand(1.0, 4.0));
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CurseOfBHTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 34073, false);
            CurseOfBHTimer = urand(30000, 45000);
        }

        if(FireballTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 9053, false);
            FireballTimer = urand(6000, 7000);
        }

        if(!RaiseDead && me->HealthBelowPct(75))
        {
            RaiseDead = true;
            float px, py, pz;
            me->GetNearPoint(px,py,pz,urand(1.0, 5.0));
            me->SummonCreature(19460, px, py, pz, 0, TEMPSUMMON_MANUAL_DESPAWN, 0, true);
        }

        if(!RunAway && me->HealthBelowPct(15))
        {
            me->DoFleeToGetAssistance();
            DoTextEmote(-1200539, NULL);
            RunAway = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19422(Creature* creature)
{
    return new npc_19422AI(creature);
}

struct npc_16978AI : public ScriptedAI
{
    npc_16978AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer ShieldSlamTimer;
    Timer StrikeTimer;

    void Reset()
    {
        me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);
        ShieldSlamTimer.Reset(4000);
        StrikeTimer.Reset(5000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(ShieldSlamTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 8242, false);
            ShieldSlamTimer = 8000;
        }

        if(StrikeTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 11976, false);
            StrikeTimer = 10000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_16978(Creature* creature)
{
    return new npc_16978AI(creature);
}

enum
{
    // Crust bursters visual spells
    SPELL_FULGORGE_SUBMERGE = 33928, // Makes fulgorge appear under ground
    SPELL_TUNNEL_BORE =
        29147, // Makes mob appear as boring tunnels under ground
    SPELL_GRAY_TUNNEL_BORE =
        37989, // Makes mob appear as boring tunnels under ground (Auchindoun)
    SPELL_FULGORGE_BORE = 34039, // There's no passive spell for this, so we
                                 // hack it by repeatedly casting it in the
                                 // script
    SPELL_SUBMERGE = 37751, // Plays animation that subemerges the crust burster
                            // into the ground
    SPELL_BIRTH =
        35177, // Plays animation that emerges the crust burster from the ground
    SPELL_STAND = 37752, // Makes sure the mob is targettable
    SPELL_SELF_ROOT =
        42716, // Used to allow out of range retargetting in combat

    // Combat spells
    // Poison needs to do more damage
    SPELL_POISON = 31747,
    SPELL_POISON_SPIT = 32330,
    SPELL_BORE = 32738,
    SPELL_TUNNEL = 33932,
    SPELL_ENRAGE_TUN = 32714,
    CRUST_BURSTER_EVADE_RANGE = 50
};

struct npc_16968AI : public ScriptedAI
{
    npc_16968AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer EmergeTimer;
    Timer DoEmergeTimer;
    Timer TunnelTimer;
    Timer EnrageTimer;
    Timer PoisonTimer;
    Timer BoreTimer;
    Timer FulgorgeHackTimer;
    Timer UnauraTimer;

    bool DidEmerge;
    bool HasTunnel;

    uint64 TargetGUID;

    void Reset()
    {
        SetCombatMovement(false);
        me->SetRooted(false);
        EmergeTimer.Reset(1000);
        DoEmergeTimer.Reset(0);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        DidEmerge = false;
        TunnelTimer.Reset(urand(4000, 8000));
        EnrageTimer.Reset(urand(10000, 20000));
        PoisonTimer.Reset(urand(4000, 8000));
        BoreTimer.Reset(urand(10000, 20000));
        FulgorgeHackTimer.Reset(1000);
        UnauraTimer.Reset(0);
        switch (me->GetEntry())
        {
            case 18678: // Fulgorge
            case 16968: // Tunneler
            case 22038: // Hai'shulud
            case 22482: // Mature Bone Sifter (Q Fumping)
                HasTunnel = true;
                break;
            default:
                HasTunnel = false;
                break;
        }
    }

    void IsSummonedBy(Unit *pSummoner)
    {
        if (pSummoner->GetTypeId() == TYPEID_PLAYER)
        {
            switch (me->GetEntry())
            {
                case 22038: // Hai'shulud
                case 22482: // Mature Bone Sifter (Q Fumping)
                    me->AI()->AttackStart(pSummoner);
                    break;
            }
        }
    }

    void EnterEvadeMode()
    {
        ScriptedAI::EnterEvadeMode();
        switch (me->GetEntry())
        {
            case 22038: // Hai'shulud
            case 22482: // Mature Bone Sifter (Q Fumping)
                me->ForcedDespawn();
                break;
            default:
                me->Respawn();
                break;
        }        
    }
    void EnterCombat(Unit* who)
    {
        switch (m_creature->GetEntry())
        {
            case 18678:
                me->RemoveAurasDueToSpell(SPELL_FULGORGE_SUBMERGE);
                break;
            case 21849: // Bone Crawler
                me->RemoveAurasDueToSpell(SPELL_GRAY_TUNNEL_BORE);
                break;
            default:
                UnauraTimer = 1000;
                me->RemoveAurasDueToSpell(SPELL_TUNNEL_BORE);
                break;
        }
        me->CastSpell(me, SPELL_STAND, false);
        me->CastSpell(me, SPELL_BIRTH, false);
        TargetGUID = who->GetGUID();

        DoEmergeTimer = 300;
    }

    void UpdateAI(const uint32 diff)
    {        
        // Fulgorge red bore hack
        if (me->GetEntry() == 18678)
        {
            if (me->HasAura(SPELL_FULGORGE_SUBMERGE))
            {
                if (FulgorgeHackTimer.Expired(diff))
                {
                    me->CastSpell(me, SPELL_FULGORGE_BORE, true);
                    FulgorgeHackTimer = 1000;
                }
            }
        }

        if(EmergeTimer.Expired(diff))
        {
            switch (m_creature->GetEntry())
            {
                case 18678:
                    me->CastSpell(me, SPELL_FULGORGE_SUBMERGE, false);
                    break;
                case 21849: // Bone Crawler
                    me->CastSpell(me, SPELL_GRAY_TUNNEL_BORE, false);
                    break;
                default:
                    me->CastSpell(me, SPELL_TUNNEL_BORE, false);
                    break;
            }
            EmergeTimer = 0;
        }

        if(DoEmergeTimer.Expired(diff))
        {
            me->SetRooted(true);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->AI()->AttackStart(me->GetUnit(TargetGUID));
            DoEmergeTimer = 0;
            DidEmerge = true;
        }

        if(!DidEmerge)
            return;

		// do we really need this?
        //if(!UpdateVictim())
        //    return;
		UpdateVictim();
		if(!me->GetVictim())
		    return;

        if(UnauraTimer.Expired(diff))
        {
            me->RemoveAurasDueToSpell(SPELL_TUNNEL_BORE);
            UnauraTimer = 0;
        }

        if(HasTunnel)
        {
            if(TunnelTimer.Expired(diff))
            {
                me->CastSpell(me, SPELL_TUNNEL, false);
                TunnelTimer = urand(15000, 20000);
                return;
            }
        }

        if (me->GetEntry() == 16968)
        {
            if(EnrageTimer.Expired(diff))
            {
                me->CastSpell(me, SPELL_ENRAGE_TUN, false);
                EnrageTimer = urand(10000, 20000);
            }
        }

        if(me->IsWithinMeleeRange(me->GetVictim()))
        {
            //if(PoisonTimer.Expired(diff))
            //{
            //    me->CastSpell(me->GetVictim(), me->GetEntry() == 18678 ? SPELL_POISON_SPIT : SPELL_POISON, false);
            //    PoisonTimer = urand(5000, 10000);
            //}

            if(BoreTimer.Expired(diff))
            {
                me->CastSpell(me->GetVictim(), SPELL_BORE, false);
                BoreTimer = urand(10000, 20000);
            }

            DoMeleeAttackIfReady();
        }
        else
        {
            if(!me->IsNonMeleeSpellCast(true))
                me->CastSpell(me->GetVictim(), me->GetEntry() == 18678 ? SPELL_POISON_SPIT : SPELL_POISON, false);
        }
    }
};

CreatureAI* GetAI_npc_16968(Creature* creature)
{
    return new npc_16968AI(creature);
}

struct npc_19763_19764_19766AI : public ScriptedAI
{
    npc_19763_19764_19766AI(Creature *c) : ScriptedAI(c)
    {
    }

    void Reset()
    {
        me->SetWalk(true);
        me->SetVisibility(VISIBILITY_ON);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        if (PointId == 100)
        {
            if (Player* nearestplayer = GetClosestPlayer(me, 30))
            {
                me->SetOrientation(me->GetOrientationTo(nearestplayer)); // serverside
                me->SetFacingTo(me->GetOrientationTo(nearestplayer)); // clientside
            }
            switch(me->GetEntry())
            {
                case 19763:
                    me->Say(-1200540, LANG_UNIVERSAL, 0);
                    if (GameObject* cage = GetClosestGameObjectWithEntry(me, 183936, 30))
                        cage->SetGoState(GO_STATE_READY);
                    break;
                case 19764:
                    me->Say(-1200541, LANG_UNIVERSAL, 0);
                    if (GameObject* cage = GetClosestGameObjectWithEntry(me, 183940, 30))
                        cage->SetGoState(GO_STATE_READY);
                    break;
                case 19766:
                    me->Say(-1200542, LANG_UNIVERSAL, 0);
                    if (GameObject* cage = GetClosestGameObjectWithEntry(me, 183941, 30))
                        cage->SetGoState(GO_STATE_READY);
                    break;
            }
            me->HandleEmoteCommand(4);
            me->SetVisibility(VISIBILITY_OFF);
            me->ForcedDespawn(5000);
            
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19763_19764_19766(Creature* creature)
{
    return new npc_19763_19764_19766AI(creature);
}


bool GOUse_go_183936(Player* pPlayer, GameObject* pGo)
{
    switch(pGo->GetEntry())
    {
        case 183936:
            if (pPlayer->GetQuestStatus(10238) == QUEST_STATUS_INCOMPLETE && pPlayer->GetReqKillOrCastCurrentCount(10238, 19763) < 1)
            {
                pGo->SetGoState(GO_STATE_ACTIVE);
                pGo->SetRespawnTime(5);
                Unit *Prisoner = FindCreature(19763, 10.0f, pPlayer);
                if(Prisoner && Prisoner->isAlive())
                {
                    Prisoner->GetMotionMaster()->MovePoint(100, 73.042, 3208.073, 32.172);
                    pPlayer->KilledMonster(19763, Prisoner->GetGUID());
                }
            }
            break;
        case 183940:
            if (pPlayer->GetQuestStatus(10238) == QUEST_STATUS_INCOMPLETE && pPlayer->GetReqKillOrCastCurrentCount(10238, 19764) < 1)
            {
                pGo->SetGoState(GO_STATE_ACTIVE);
                pGo->SetRespawnTime(5);
                Unit *Prisoner = FindCreature(19764, 10.0f, pPlayer);
                if(Prisoner && Prisoner->isAlive())
                {
                    Prisoner->GetMotionMaster()->MovePoint(100, -71.033, 3135.695, -4.565);
                    pPlayer->KilledMonster(19764, Prisoner->GetGUID());
                }
            }
            break;
        case 183941:
            if (pPlayer->GetQuestStatus(10238) == QUEST_STATUS_INCOMPLETE && pPlayer->GetReqKillOrCastCurrentCount(10238, 19766) < 1)
            {
                pGo->SetGoState(GO_STATE_ACTIVE);
                pGo->SetRespawnTime(5);
                Unit *Prisoner = FindCreature(19766, 10.0f, pPlayer);
                if(Prisoner && Prisoner->isAlive())
                {
                    Prisoner->GetMotionMaster()->MovePoint(100, -118.843, 3089.47, 3.379);
                    pPlayer->KilledMonster(19766, Prisoner->GetGUID());
                }
            }
            break;
    }
    return true;
}

#define SPELL_HEALING_SLAVE 29314
#define NPC_MAGHAR_GRUNT    16846

struct npc_16847AI : public ScriptedAI
{
    npc_16847AI(Creature* c) : ScriptedAI(c) {}

    bool DespawnTime;
    Timer despawnTimer;

    void Reset()
    {
        DespawnTime = true;
        despawnTimer.Reset(0);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_HEALING_SLAVE && caster->GetTypeId() == TYPEID_PLAYER)
        {
            ((Player*)caster)->KilledMonster(16847, me->GetGUID());
            me->UpdateEntry(NPC_MAGHAR_GRUNT);
            switch(urand(0,1))
            {
                case 0:
                    me->Say(-1200543, LANG_UNIVERSAL, 0);
                    break;
                case 1:
                    me->Say(-1200544, LANG_UNIVERSAL, caster->GetGUID());
                    break;
            }
            DespawnTime = false;
            despawnTimer = 10000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (DespawnTime == false)
        {
            if (despawnTimer.Expired(diff))
            {
                me->DisappearAndDie();
                despawnTimer = 0;
            }
        }
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_16847(Creature* c)
{
    return new npc_16847AI(c);
}

struct npc_20159AI : public ScriptedAI
{
    enum Constants
    {
        SPELL_FIREBALL          = 20823,
        SPELL_FROSTNOVA         = 11831
    };

    explicit npc_20159AI(Creature *c) : ScriptedAI(c), PlayerGUID(0) {}

    Timer TalkTimer;
    Timer StartFightTimer;
    Timer FireballTimer;
    Timer FrostNovaTimer;
    Timer EvadeTimer;

    uint64 PlayerGUID;

    bool IsInQuestFightPhase() { return PlayerGUID != 0; }

    void Reset() override
    {
        TalkTimer.Reset(0);
        StartFightTimer.Reset(0);
        FrostNovaTimer.Reset(20000);
        FireballTimer.Reset(5000);
        EvadeTimer.Reset(0);
        PlayerGUID = 0;
        me->setFaction(1602);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, true);
    }

    // Quest 10286 fight

    void StartQuestFight(Player* player)
    {
        if (IsInQuestFightPhase())
            return;

        me->Unmount();
        me->SetFacingToObject(player);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        PlayerGUID = player->GetGUID();
        TalkTimer = 2000;
    }

    void StopQuestFightIfNeeded(uint32 &damage)
    {
        if (!me->HealthBelowPct(20) && damage < me->GetHealth())
            return;

        damage = 0;
        me->RestoreFaction();
        me->RemoveAllAuras();
        me->DeleteThreatList();
        me->CombatStop(true);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
        me->Say(-1200545, LANG_UNIVERSAL, 0);

        Player *player = Unit::GetPlayerInWorld(PlayerGUID);
        if (player && player->GetQuestStatus(QUEST_ARELIONS_SECRET) == QUEST_STATUS_INCOMPLETE)
            player->CompleteQuest(QUEST_ARELIONS_SECRET);

        PlayerGUID = 0;
        EvadeTimer = 30000;
    }

    void DamageTaken(Unit* /*attacker*/, uint32 &damage) override
    {
        if (IsInQuestFightPhase())
            StopQuestFightIfNeeded(damage);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (IsInQuestFightPhase())
            UpdateQuestFightTimers(diff);

        if (EvadeTimer.Expired(diff))
        {
            EnterEvadeMode();
            EvadeTimer = 0;
        }

        if (!UpdateVictim())
            return;

        if (FrostNovaTimer.Expired(diff)) 
        {
            DoCast(me->GetVictim(), SPELL_FROSTNOVA);
            FrostNovaTimer = 20000;
        }

        if (FireballTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_FIREBALL);
            FireballTimer = 5000;
        }

        DoMeleeAttackIfReady();
    }

    void UpdateQuestFightTimers(const uint32 diff)
    {
        if (TalkTimer.Expired(diff))
        {
            // TODO: add localized text
            me->Say(-1200546, LANG_UNIVERSAL, 0);
            StartFightTimer = 3000;
            TalkTimer = 0;
        }

        if (StartFightTimer.Expired(diff))
        {
            me->setFaction(14);
            me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
            me->AI()->AttackStart(Unit::GetPlayerInWorld(PlayerGUID));
            DoCast(me->GetVictim(), SPELL_FIREBALL);
            FireballTimer = 5000;
            FrostNovaTimer = 20000;
            StartFightTimer = 0;
        }
    }
};

CreatureAI* GetAI_npc_20159(Creature* creature)
{
    return new npc_20159AI(creature);
}

bool GossipHello_npc_20159(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(QUEST_ARELIONS_SECRET) == QUEST_STATUS_INCOMPLETE)
    {
        creature->StopMoving();
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16549), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_20159(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        if (npc_20159AI* ai = CAST_AI(npc_20159AI, creature->AI()))
            ai->StartQuestFight(player);
    }
    return true;
}

/*######
## go_kaliri_nest
######*/

bool GOHello_go_kaliri_nest(Player *pPlayer, GameObject *pGO)
{
    if (pGO->GetGoType() == GAMEOBJECT_TYPE_GOOBER && pPlayer->GetQuestStatus(9397) == QUEST_STATUS_INCOMPLETE)
    {
        uint32 creature_id = 0;
        uint32 rand = urand(0, 99);

        if (rand < 10)
            creature_id = 17034;
        else if (rand < 60)
            creature_id = 17035;
        else
            creature_id = 17039;

        pGO->SummonCreature(creature_id,pGO->GetPositionX(),pGO->GetPositionY(),pGO->GetPositionZ(),pGO->GetOrientationTo(pPlayer),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
        pGO->DestroyForPlayer(pPlayer);
    }
    return true;
};

struct npc_16974_16975AI : public ScriptedAI
{
    npc_16974_16975AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer SetInvisibleTimer;
    Timer SetVisibleTimer;
    Timer ShadowstrikeTimer;
    Timer VoidDrainTimer;
    Timer CheckCombatTimer;

    void Reset()
    {
        SetInvisibleTimer.Reset(urand(5000, 15000));
        SetVisibleTimer.Reset(0);
        ShadowstrikeTimer.Reset(6000);
        VoidDrainTimer.Reset(8000);
        CheckCombatTimer.Reset(0);
        me->SetVisibility(VISIBILITY_ON);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if(SetInvisibleTimer.Expired(diff))
            {
                if(Unit* voidportal = FindCreature(19681, 20, me))
                {   
                    me->CastSpell(me, 34842, false);
                    SetInvisibleTimer = 0;
                    SetVisibleTimer = urand(9000, 17000);
                    CheckCombatTimer = 3000;
                }
                else
                    SetInvisibleTimer = urand(1000, 10000);
            }

            if(SetVisibleTimer.Expired(diff))
            {
                me->CastSpell(me, 34302, false);
                me->SetVisibility(VISIBILITY_ON);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                me->SetReactState(REACT_AGGRESSIVE);
                SetVisibleTimer = 0;
                CheckCombatTimer = 0;
                SetInvisibleTimer = urand(8000, 16000);
            }
            return;
        }

        if (CheckCombatTimer.Expired(diff))
        {
            if (me->GetVisibility() == VISIBILITY_OFF)
            {
                me->CastSpell(me, 34302, false);
                me->SetVisibility(VISIBILITY_ON);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                me->SetReactState(REACT_AGGRESSIVE);
                CheckCombatTimer = 0;
            }
        }

        if(ShadowstrikeTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 33914, false);
            ShadowstrikeTimer = 10000;
        }

        if(me->GetEntry() == 16975)
        {
            if(VoidDrainTimer.Expired(diff))
            {
                me->CastSpell(me->GetVictim(), 33916, false);
                VoidDrainTimer = urand(15000, 20000);
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_16974_16975(Creature* creature)
{
    return new npc_16974_16975AI(creature);
}

struct npc_19527AI : public ScriptedAI
{
    npc_19527AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer SetInvisibleTimer;
    Timer SetVisibleTimer;
    SpellSchools school;
    Timer SpellDamageTimer;

    void Reset()
    {
        SetInvisibleTimer.Reset(urand(5000, 15000));
        SetVisibleTimer.Reset(0);
        me->SetVisibility(VISIBILITY_ON);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        me->SetReactState(REACT_AGGRESSIVE);

        school = SPELL_SCHOOL_NORMAL;
        me->RemoveSpellsCausingAura(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
        SpellDamageTimer.Reset(0);
    }

    uint32 GetSpellToCast()
    {
        switch (school)
        {
            case SPELL_SCHOOL_HOLY:     return 34346;
            case SPELL_SCHOOL_FIRE:     return 34348;
            case SPELL_SCHOOL_NATURE:   return 34345;
            case SPELL_SCHOOL_FROST:    return 34347;
            case SPELL_SCHOOL_SHADOW:   return 34344;
            case SPELL_SCHOOL_ARCANE:   return 34446;
            default:                    return 34446;
        }
    }

    void SpellHit(Unit*, const SpellEntry* spell)
    {
        if (school != SPELL_SCHOOL_NORMAL ||
            spell->SchoolMask & SPELL_SCHOOL_MASK_NORMAL)
            return;

        if (spell->SchoolMask & SPELL_SCHOOL_MASK_HOLY)
        {
            me->CastSpell(me, 34336, true);
            school = SPELL_SCHOOL_HOLY;
            SpellDamageTimer = 3000;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_FIRE)
        {
            me->CastSpell(me, 34333, true);
            school = SPELL_SCHOOL_FIRE;
            SpellDamageTimer = 3000;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_NATURE)
        {
            me->CastSpell(me, 34335, true);
            school = SPELL_SCHOOL_NATURE;
            SpellDamageTimer = 3000;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_FROST)
        {
            me->CastSpell(me, 34334, true);
            school = SPELL_SCHOOL_FROST;
            SpellDamageTimer = 3000;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_SHADOW)
        {
            me->CastSpell(me, 34338, true);
            school = SPELL_SCHOOL_SHADOW;
            SpellDamageTimer = 3000;
        }
        else if (spell->SchoolMask & SPELL_SCHOOL_MASK_ARCANE)
        {
            me->CastSpell(me, 34331, true);
            school = SPELL_SCHOOL_ARCANE;
            SpellDamageTimer = 5000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if(SetInvisibleTimer.Expired(diff))
            {
                if(Unit* voidportal = FindCreature(19681, 20, me))
                {
                    me->CastSpell(me, 34842, false);
                    SetInvisibleTimer = 0;
                    SetVisibleTimer = urand(9000, 17000);
                }
                else
                    SetInvisibleTimer = urand(1000, 10000);
            }

            if(SetVisibleTimer.Expired(diff))
            {
                me->CastSpell(me, 34302, false);
                me->SetVisibility(VISIBILITY_ON);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                me->SetReactState(REACT_AGGRESSIVE);
                SetVisibleTimer = 0;
                SetInvisibleTimer = urand(8000, 16000);
            }
            return;
        }

        if (SpellDamageTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), GetSpellToCast());
            if(school != SPELL_SCHOOL_ARCANE)
                SpellDamageTimer = 3000; // will be reset in SpellHitTarget
            else
                SpellDamageTimer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19527(Creature* creature)
{
    return new npc_19527AI(creature);
}

struct npc_19823AI : public ScriptedAI
{
    npc_19823AI(Creature *c) : ScriptedAI(c), Summons(me)
    {
    }

    bool SummonOne;
    bool SummonTwo;
    bool SummonThree;
    Timer CheckTimer;
    SummonList Summons;

    void Reset()
    {
        CheckTimer.Reset(3000);
        SummonOne = false;
        SummonTwo = false;
        SummonThree = false;
        const CreatureInfo *cinfo = me->GetCreatureInfo();
        me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg * me->GetCreatureDamageMod());
        me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg * me->GetCreatureDamageMod());
        me->UpdateDamagePhysical(BASE_ATTACK);
        Summons.DespawnAll();
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned)
        {
            summoned->AI()->AttackStart(me->GetVictim());
        }
        Summons.Summon(summoned);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CheckTimer.Expired(diff))
        {
            if(!me->isCrowdControlled())
            {
                if(!SummonOne && me->HealthBelowPct(75))
                {
                    DoSpawnCreature(21936, 0, 0, 0, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                    const CreatureInfo *cinfo = me->GetCreatureInfo();
                    me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg - ((cinfo->mindmg/100) * 20)) * me->GetCreatureDamageMod());
                    me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg - ((cinfo->maxdmg/100) * 20)) * me->GetCreatureDamageMod());
                    me->UpdateDamagePhysical(BASE_ATTACK);
                    me->CastSpell(me, 44339, false);
                    SummonOne = true;
                }

                if(!SummonTwo && me->HealthBelowPct(50))
                {
                    DoSpawnCreature(21936, 0, 0, 0, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                    const CreatureInfo *cinfo = me->GetCreatureInfo();
                    me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg - ((cinfo->mindmg/100) * 40)) * me->GetCreatureDamageMod());
                    me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg - ((cinfo->maxdmg/100) * 40)) * me->GetCreatureDamageMod());
                    me->UpdateDamagePhysical(BASE_ATTACK);
                    me->CastSpell(me, 44339, false);
                    SummonTwo = true;
                }

                if(!SummonThree && me->HealthBelowPct(25))
                {
                    DoSpawnCreature(21936, 0, 0, 0, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                    const CreatureInfo *cinfo = me->GetCreatureInfo();
                    me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, (cinfo->mindmg - ((cinfo->mindmg/100) * 60)) * me->GetCreatureDamageMod());
                    me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, (cinfo->maxdmg - ((cinfo->maxdmg/100) * 60)) * me->GetCreatureDamageMod());
                    me->UpdateDamagePhysical(BASE_ATTACK);
                    me->CastSpell(me, 44339, false);
                    SummonThree = true;
                }
            }
            CheckTimer = 2000;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19823(Creature* creature)
{
    return new npc_19823AI(creature);
}


struct npc_16973AI : public ScriptedAI
{
    npc_16973AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer GutRipTimer;
    Timer EmoteTimer;
    Timer ResetTimer;

    void Reset()
    {
        GutRipTimer.Reset(2000);
        EmoteTimer.Reset(0);
        ResetTimer.Reset(0);
        me->SetRooted(false);
    }

    void EnterCombat(Unit* who)
    {
        EmoteTimer = 0;
        ResetTimer = 0;
    }

    void JustDied(Unit* killer)
    {
        if (Unit* bird = GetClosestCreatureWithEntry(me, me->GetEntry() == 16973 ? 16973 : 16972, 40))
        {
            if (!bird->IsInCombat() && bird->isAlive() && !((Creature*)bird)->IsInEvadeMode() && !bird->IsInRoots())
                me->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, me, (Creature*)bird, 0);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 miscValue)
    {
        if (eventType == AI_EVENT_CUSTOM_EVENTAI_A)
        {
            me->SetWalk(false);
            me->GetMotionMaster()->MovePoint(100, sender->GetPositionX(), sender->GetPositionY(), sender->GetPositionZ());
        }
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;
        if (PointId == 100)
        {
            me->SetRooted(true);
            me->HandleEmote(35);
            EmoteTimer = 1000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EmoteTimer.Expired(diff))
            {
                me->HandleEmote(35);
                EmoteTimer = 0;
                ResetTimer = 2000;
            }

            if (ResetTimer.Expired(diff))
            {
                me->HandleEmote(35);
                me->SetRooted(false);
                EnterEvadeMode();
                ResetTimer = 0;
            }
            return;
        }

        //if (me->IsInRoots())
        //    me->SetRooted(false);

        if (GutRipTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), me->GetEntry() == 16973 ? 32022 : 37012, false);
            GutRipTimer = urand(5000, 7000);
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_16973(Creature* creature)
{
    return new npc_16973AI(creature);
}

struct npc_16977AI : public ScriptedAI
{
    npc_16977AI(Creature *c) : ScriptedAI(c), Summons(me)
    {
    }

    Timer EventTimer;
    uint8 Phase;
    SummonList Summons;
    Timer FireballTimer;
    Timer ArcaneMissilesTimer;
    bool IceBarrier;
    bool flee;

    void Reset()
    {
        Summons.DespawnAll();
        EventTimer.Reset(15000);
        Phase = 0;
        FireballTimer.Reset(1000);
        ArcaneMissilesTimer.Reset(6000);
        IceBarrier = false;
        flee = false;
    }

    void JustSummoned(Creature* summoned)
    {
        Summons.Summon(summoned);
    }
    
    void SummonedMovementInform(Creature* sum, uint32 type, uint32 id)
    {
        if (id == 100)
            sum->DisappearAndDie();
    }

    void EnterCombat(Unit* who)
    {
        for (SummonList::const_iterator i = Summons.begin(); i != Summons.end(); i++)
        {
            if (Creature* sum = me->GetCreature(*i))
            {
                float x, y, z;
                sum->GetRandomPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 20.0f, x, y, z);
                sum->GetMotionMaster()->MovePoint(100, x, y, z);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EventTimer.Expired(diff))
            {
                switch (Phase)
                {
                    case 0:
                    {
                        if (Creature* denOne = me->SummonCreature(16976, -1316.21, 2762.65, -27.15, 1.2, TEMPSUMMON_MANUAL_DESPAWN, 0))
                            denOne->SetStandState(UNIT_STAND_STATE_SIT);
                        if (Creature* denTwo = me->SummonCreature(16976, -1311.71, 2762.65, -27.109, 1.4, TEMPSUMMON_MANUAL_DESPAWN, 0))
                            denTwo->SetStandState(UNIT_STAND_STATE_SIT);
                        if (Creature* denThree = me->SummonCreature(16976, -1306.99, 2762.72, -27.00, 2.1, TEMPSUMMON_MANUAL_DESPAWN, 0))
                            denThree->SetStandState(UNIT_STAND_STATE_SIT);
                        EventTimer = 3000;
                        Phase++;
                        break;
                    }
                    case 1:
                    {
                        me->SetFacingTo(4.59);
                        me->Say(-1200548, LANG_UNIVERSAL, 0);
                        me->HandleEmote(1);
                        EventTimer = 8000;
                        Phase++;
                        break;
                    }
                    case 2:
                    {
                        me->SetFacingTo(4.59);
                        me->Say(-1200549, LANG_UNIVERSAL, 0);
                        me->HandleEmote(1);
                        EventTimer = 5000;
                        Phase++;
                        break;
                    }
                    case 3:
                    {
                        me->SetFacingTo(4.59);
                        me->Say(-1200550, LANG_UNIVERSAL, 0);
                        me->HandleEmote(1);
                        EventTimer = 3000;
                        Phase++;
                        break;
                    }
                    case 4:
                    {
                        me->SetFacingTo(2.58);
                        if (Unit* dummy = FindCreature(17059, 30, me))
                            me->CastSpell(dummy, 29456, false);
                        EventTimer = 3000;
                        Phase++;
                        break;
                    }
                    case 5:
                    {
                        me->SetFacingTo(4.59);
                        me->Say(-1200551, LANG_UNIVERSAL, 0);
                        me->HandleEmote(1);
                        EventTimer = 4000;
                        Phase++;
                        break;
                    }
                    case 6:
                    {
                        me->SetFacingTo(2.16);
                        if (Unit* dummy = FindCreature(41001, 30, me))
                            me->CastSpell(dummy, 29457, false);
                        EventTimer = 3000;
                        Phase++;
                        break;
                    }
                    case 7:
                    {
                        me->SetFacingTo(4.59);
                        me->Say(-1200552, LANG_UNIVERSAL, 0);
                        me->HandleEmote(1);
                        EventTimer = 4000;
                        Phase++;
                        break;
                    }
                    case 8:
                    {
                        me->SetFacingTo(1.29);
                        if (Unit* dummy = FindCreature(41002, 30, me))
                            me->CastSpell(dummy, 29458, false);
                        EventTimer = 12000;
                        Phase++;
                        break;
                    }
                    case 9:
                    {
                        me->SetFacingTo(4.59);
                        me->Say(-1200553, LANG_UNIVERSAL, 0);
                        me->HandleEmote(1);
                        EventTimer = 4000;
                        Phase++;
                        break;
                    }
                    case 10:
                    {
                        me->SetFacingTo(0.6);
                        if (Unit* dummy = FindCreature(41003, 30, me))
                            me->CastSpell(dummy, 29459, false);
                        EventTimer = 6000;
                        Phase++;
                        break;
                    }
                    case 11:
                    {
                        for (SummonList::const_iterator i = Summons.begin(); i != Summons.end(); i++)
                        {
                            if (Creature* sum = me->GetCreature(*i))
                            {
                                float x, y, z;
                                sum->GetRandomPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 10.0f, x, y, z);
                                sum->GetMotionMaster()->MovePoint(100, x, y, z);
                            }
                        }
                        EventTimer = 5000;
                        Phase++;
                        break;
                    }
                    case 12:
                    {
                        Reset();
                        break;
                    }
                    default: break;
                }
            }
            return;
        }

        if (FireballTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 20823, false);
            FireballTimer = 3000;
        }

        if (ArcaneMissilesTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 15736, false);
            ArcaneMissilesTimer = 9000;
        }

        if (!IceBarrier)
        {
            if (me->GetHealthPercent() <= 50)
            {
                me->CastSpell(me, 33245, false);
                me->CastSpell(me->GetVictim(), 27646, false);
                IceBarrier = true;
            }
        }

        if (!flee)
        {
            if (me->GetHealthPercent() <= 15)
            {
                me->DoFleeToGetAssistance();
                DoScriptText(-1901007, me);
                flee = true;
            }
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_16977(Creature* creature)
{
    return new npc_16977AI(creature);
}

struct npc_19392AI : public ScriptedAI
{
    npc_19392AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer EventTimer;

    void Reset()
    {
        EventTimer.Reset(480000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventTimer.Expired(diff))
        {
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_F, me, 0, 200);
            if (urand(0, 1) == 1)
                me->Yell(-1200554, LANG_UNIVERSAL, 0);
            else
                me->Yell(-1200555, LANG_UNIVERSAL, 0);
            EventTimer = 480000;
        }

        if (!UpdateVictim())
            return;
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19392(Creature* creature)
{
    return new npc_19392AI(creature);
}

struct npc_21075AI : public ScriptedAI
{
    npc_21075AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer EventTimer;
    uint8 Phase;

    void Reset()
    {
        EventTimer.Reset(0);
        Phase = 0;
    }

    void ReceiveAIEvent(AIEventType evType, Creature* sender, Unit* invoker, uint32 misc)
    {
        if (evType == 11)
            EventTimer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventTimer.Expired(diff))
        {
            std::list<Creature*> iTargetsList = FindAllCreaturesWithEntry(18729, 50);
            std::vector<Creature*> iTargetsVector;
            for (std::list<Creature*>::iterator i = iTargetsList.begin(); i != iTargetsList.end(); i++)
                iTargetsVector.push_back(*i);
            std::random_shuffle(iTargetsVector.begin(), iTargetsVector.end());
            if (Phase <= 30)
            {
                if (!iTargetsVector.empty())
                    me->CastSpell(iTargetsVector.front(), 32785, false);

                EventTimer = urand(1000, 2000);
                Phase++;
            }
            else
            {
                EventTimer = 0;
                Phase = 0;
                for(int i = 0; i < 3; i++)
                {
                    std::list<Creature*> receivers;
                    Hellground::AnyUnitInObjectRangeCheck u_check(me, 250);
                    Hellground::ObjectListSearcher<Creature, Hellground::AnyUnitInObjectRangeCheck> searcher(receivers, u_check);
                    Cell::VisitGridObjects(me, searcher, 250);
                    if (!receivers.empty())
                    {
                        for (std::list<Creature*>::const_iterator itr = receivers.begin(); itr != receivers.end(); ++itr)
                        {
                            if (!(*itr)->IsInCombat() && !(*itr)->IsInEvadeMode())
                                (*itr)->AI()->EnterEvadeMode();
                        }
                    }
                }
                EnterEvadeMode();
            }
            std::list<Creature*> iTargetsListThrallmar = FindAllCreaturesWithEntry(19215, 50);
            std::vector<Creature*> iTargetsVectorThrallmar;
            for (std::list<Creature*>::iterator i = iTargetsListThrallmar.begin(); i != iTargetsListThrallmar.end(); i++)
                iTargetsVectorThrallmar.push_back(*i);
            std::random_shuffle(iTargetsVectorThrallmar.begin(), iTargetsVectorThrallmar.end());
            if (!iTargetsVectorThrallmar.empty())
            {
                    me->CastSpell(iTargetsVectorThrallmar.front(), 32785, false);
                    EventTimer = 0;
            }
        }
    }
};

CreatureAI* GetAI_npc_21075(Creature* creature)
{
    return new npc_21075AI(creature);
}


struct npc_18729AI : public Scripted_NoMovementAI
{
    npc_18729AI(Creature *c) : Scripted_NoMovementAI(c)
    {
    }

    Timer ResetCombat;

    void Reset()
    {
        ResetCombat.Reset(0);
    }

    void EnterCombat(Unit* who)
    {
        EnterEvadeMode();
        me->CombatStop();
        who->CombatStop();
        ResetCombat.Reset(30000);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == 32785)
        {
            me->CastSpell(me, 33637, false);
            std::list<Unit*> creatures;
            Hellground::AnyFriendlyNonSelfUnitInObjectRangeCheck creature_check(me, me, 12);
            Hellground::UnitListSearcher<Hellground::AnyFriendlyNonSelfUnitInObjectRangeCheck> creature_searcher(creatures, creature_check);
            Cell::VisitGridObjects(me, creature_searcher, 12);
            if (!creatures.empty())
            {
                for (std::list<Unit*>::iterator itr = creatures.begin(); itr != creatures.end(); ++itr)
                {
                    if (!(*itr)->HasAura(33637, 0) && !(*itr)->HasUnitState(UNIT_STAT_FLEEING))
                        (*itr)->GetMotionMaster()->MoveFleeing(*itr, 4000);
                }
            }
            std::list<Unit*> creatur;
            Hellground::AnyUnfriendlyNoTotemUnitInObjectRangeCheck creatur_check(me, me, 12);
            Hellground::UnitListSearcher<Hellground::AnyUnfriendlyNoTotemUnitInObjectRangeCheck> creatur_searcher(creatur, creatur_check);
            Cell::VisitGridObjects(me, creatur_searcher, 12);
            if (!creatur.empty())
            {
                for (std::list<Unit*>::iterator itr = creatur.begin(); itr != creatur.end(); ++itr)
                {
                    if (!(*itr)->HasAura(33637, 0) && !(*itr)->HasUnitState(UNIT_STAT_FLEEING))
                        (*itr)->GetMotionMaster()->MoveFleeing(*itr, 4000);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(ResetCombat.Expired(diff))
        {
            Unit *target = NULL;
            std::list<HostileReference *> t_list = me->getThreatManager().getThreatList();
            for(std::list<HostileReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
            {
                target = me->GetUnit((*itr)->getUnitGuid());
                if(target && target->IsInCombat())
                    target->CombatStop();
            }
            me->CombatStop();
            me->getThreatManager().clearReferences();
            me->RemoveAllAuras();
            EnterEvadeMode();
            ResetCombat = 30000;
        }
    }
};

CreatureAI* GetAI_npc_18729(Creature* creature)
{
    return new npc_18729AI(creature);
}


struct npc_19188AI : public ScriptedAI
{
    npc_19188AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer CrystalStrikeTimer;
    Timer KnockdownTimer;
    bool Enrage;
    bool summonOne;
    bool summonTwo;
    bool SummonThree;

    void Reset()
    {
        CrystalStrikeTimer.Reset(6000);
        KnockdownTimer.Reset(9000);
        Enrage = false;
        summonOne = false;
        summonTwo = false;
        SummonThree = false;
        me->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
        ClearCastQueue();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        if(CrystalStrikeTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 33688, false);
            CrystalStrikeTimer = 7000;
        }

        if(KnockdownTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 11428, false);
            KnockdownTimer = 10000;
        }

        if(!Enrage)
        {
            if(me->GetHealthPercent() <= 20)
            {
                me->CastSpell(me, 18501, false);
                Enrage = true;
            }
        }

        if(!summonOne)
        {
            if(me->GetHealthPercent() <= 75)
            {
                me->CastSpell(me, 24240, false);
                if(Creature* shard = me->SummonCreature(19419, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10000))
                    shard->AI()->AttackStart(me->GetVictim());
                me->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.75);
                summonOne = true;
            }
        }

        if(!summonTwo)
        {
            if(me->GetHealthPercent() <= 50)
            {
                me->CastSpell(me, 24240, false);
                if(Creature* shard = me->SummonCreature(19419, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10000))
                    shard->AI()->AttackStart(me->GetVictim());
                me->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.50);
                summonTwo = true;
            }
        }

        if(!SummonThree)
        {
            if(me->GetHealthPercent() <= 25)
            {
                me->CastSpell(me, 24240, false);
                if(Creature* shard = me->SummonCreature(19419, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10000))
                    shard->AI()->AttackStart(me->GetVictim());
                me->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.25);
                SummonThree = true;
            }
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19188(Creature* creature)
{
    return new npc_19188AI(creature);
}

void AddSC_hellfire_peninsula()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_19188";
    newscript->GetAI = &GetAI_npc_19188;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18729";
    newscript->GetAI = &GetAI_npc_18729;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21075";
    newscript->GetAI = &GetAI_npc_21075;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19392";
    newscript->GetAI = &GetAI_npc_19392;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_16977";
    newscript->GetAI = &GetAI_npc_16977;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_16973";
    newscript->GetAI = &GetAI_npc_16973;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19527";
    newscript->GetAI = &GetAI_npc_19527;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_abyssal_shelf_quest";
    newscript->GetAI = &GetAI_npc_abyssal_shelf_quest;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_aeranas";
    newscript->GetAI = &GetAI_npc_aeranas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_haaleshi_altar";
    newscript->pGOUse = &GOUse_go_haaleshi_altar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wing_commander_dabiree";
    newscript->pGossipHello =   &GossipHello_npc_wing_commander_dabiree;
    newscript->pGossipSelect =  &GossipSelect_npc_wing_commander_dabiree;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_gryphoneer_windbellow";
    newscript->pGossipHello =   &GossipHello_npc_gryphoneer_windbellow;
    newscript->pGossipSelect =  &GossipSelect_npc_gryphoneer_windbellow;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wing_commander_brack";
    newscript->pGossipHello =   &GossipHello_npc_wing_commander_brack;
    newscript->pGossipSelect =  &GossipSelect_npc_wing_commander_brack;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wounded_blood_elf";
    newscript->GetAI = &GetAI_npc_wounded_blood_elf;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_wounded_blood_elf;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_demoniac_scryer";
    newscript->pGossipHello =  &GossipHello_npc_demoniac_scryer;
    newscript->pGossipSelect = &GossipSelect_npc_demoniac_scryer;
    newscript->GetAI = &GetAI_npc_demoniac_scryer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_magic_sucker_device_spawner";
    newscript->GetAI = &GetAI_npc_magic_sucker_device_spawner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ancestral_spirit_wolf";
    newscript->GetAI = &GetAI_npc_ancestral_spirit_wolf;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_earthcaller_ryga";
    newscript->GetAI = &GetAI_npc_earthcaller_ryga;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_living_flare";
    newscript->GetAI = &GetAI_npc_living_flare;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_felblood_initiate";
    newscript->GetAI = &GetAI_npc_felblood_initiate;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_ice_stone";
    newscript->pGOUse  = &GossipHello_go_ice_stone;
    newscript->pGossipSelectGO = &GossipSelect_go_ice_stone;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_hand_berserker";
    newscript->GetAI = &GetAI_npc_hand_berserker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_anchorite_relic_bunny";
    newscript->GetAI = &GetAI_npc_anchorite_relic_bunny;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_anchorite_barada";
    newscript->pGossipHello =  &GossipHello_npc_anchorite_barada;
    newscript->pGossipSelect = &GossipSelect_npc_anchorite_barada;
    newscript->GetAI = &GetAI_npc_anchorite_barada;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_darkness_released";
    newscript->GetAI = &GetAI_npc_darkness_released;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_foul_purge";
    newscript->GetAI = &GetAI_npc_foul_purge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_sedai_quest_credit_marker";
    newscript->GetAI = &GetAI_npc_sedai_quest_credit_marker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_vindicator_sedai";
    newscript->GetAI = &GetAI_npc_vindicator_sedai;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_pathaleon_image";
    newscript->GetAI = &GetAI_npc_pathaleon_image;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_viera";
    newscript->GetAI = &GetAI_npc_viera;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_viera;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_deranged_helboar";
    newscript->GetAI = &GetAI_npc_deranged_helboar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_east_hovel";
    newscript->GetAI = &GetAI_npc_east_hovel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_west_hovel";
    newscript->GetAI = &GetAI_npc_west_hovel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stable";
    newscript->GetAI = &GetAI_npc_stable;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_barracks";
    newscript->GetAI = &GetAI_npc_barracks;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_pit_commander";
    newscript->GetAI = &GetAI_npc_pit_commander;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_infernal_relay";
    newscript->GetAI = &GetAI_npc_infernal_relay;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19006";
    newscript->GetAI = &GetAI_npc_19006;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19007";
    newscript->GetAI = &GetAI_npc_19007;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18986";
    newscript->GetAI = &GetAI_npc_18986;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18948";
    newscript->GetAI = &GetAI_npc_18948;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18965";
    newscript->GetAI = &GetAI_npc_18965;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18950";
    newscript->GetAI = &GetAI_npc_18950;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18972";
    newscript->GetAI = &GetAI_npc_18972;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18970";
    newscript->GetAI = &GetAI_npc_18970;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18971_18949";
    newscript->GetAI = &GetAI_npc_18971_18949;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18969";
    newscript->GetAI = &GetAI_npc_18969;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18966";
    newscript->GetAI = &GetAI_npc_18966;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18944_19005";
    newscript->GetAI = &GetAI_npc_18944_19005;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_warlord_mokh";
    newscript->GetAI = &GetAI_npc_warlord_mokhAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_eye_of_the_citadel";
    newscript->GetAI = &GetAI_npc_eye_of_the_citadelAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_17058_16938";
    newscript->GetAI = &GetAI_npc_17058_16938;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19294";
    newscript->GetAI = &GetAI_npc_19294;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_19294;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19293";
    newscript->GetAI = &GetAI_npc_19293;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_19293;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_20679";
    newscript->GetAI = &GetAI_npc_20679;
    newscript->pGossipHello =   &GossipHello_npc_20679;
    newscript->pGossipSelect =  &GossipSelect_npc_20679;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_20678";
    newscript->GetAI = &GetAI_npc_20678;
    newscript->pGossipHello =   &GossipHello_npc_20678;
    newscript->pGossipSelect =  &GossipSelect_npc_20678;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_20677";
    newscript->GetAI = &GetAI_npc_20677;
    newscript->pGossipHello =   &GossipHello_npc_20677;
    newscript->pGossipSelect =  &GossipSelect_npc_20677;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_16925_19424";
    newscript->GetAI = &GetAI_npc_16925_19424;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18706";
    newscript->GetAI = &GetAI_npc_18706;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19422";
    newscript->GetAI = &GetAI_npc_19422;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_16978";
    newscript->GetAI = &GetAI_npc_16978;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_16968";
    newscript->GetAI = &GetAI_npc_16968;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19763_19764_19766";
    newscript->GetAI = &GetAI_npc_19763_19764_19766;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_183936";
    newscript->pGOUse = &GOUse_go_183936;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_16847";
    newscript->GetAI = &GetAI_npc_16847;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21847";
    newscript->GetAI = &GetAI_npc_21847;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_20159";
    newscript->GetAI = &GetAI_npc_20159;
    newscript->pGossipHello =   &GossipHello_npc_20159;
    newscript->pGossipSelect =  &GossipSelect_npc_20159;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_kaliri_nest";
    newscript->pGOUse = &GOHello_go_kaliri_nest;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_16974_16975";
    newscript->GetAI = &GetAI_npc_16974_16975;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19823";
    newscript->GetAI = &GetAI_npc_19823;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_22432";
    newscript->pGossipHello = &GossipHello_npc_22432;
    newscript->RegisterSelf();
}
