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
SDName: Shattrath_City
SD%Complete: 99
SDComment: Quest support: 10004, 10009, 10211, 10231. Flask vendors, Teleport to Caverns of Time
SDCategory: Shattrath City
EndScriptData */

/* ContentData
npc_raliq_the_drunk
npc_salsalabim
npc_shattrathflaskvendors
npc_zephyr
npc_kservant
npc_dirty_larry
npc_ishanah
npc_khadgar
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_raliq_the_drunk
######*/

#define GOSSIP_RALIQ            16051

#define FACTION_HOSTILE_RD      45
#define FACTION_FRIENDLY_RD     35

#define SPELL_UPPERCUT          10966

struct npc_raliq_the_drunkAI : public ScriptedAI
{
    npc_raliq_the_drunkAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked Uppercut_Timer;

    void Reset()
    {
        Uppercut_Timer.Reset(5000);
        me->setFaction(FACTION_FRIENDLY_RD);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (Uppercut_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_UPPERCUT);
            Uppercut_Timer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_raliq_the_drunk(Creature *creature)
{
    return new npc_raliq_the_drunkAI (creature);
}

bool GossipHello_npc_raliq_the_drunk(Player *player, Creature *creature )
{
    if( player->GetQuestStatus(10009) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_RALIQ), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_raliq_the_drunk(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF+1 )
    {
        player->CLOSE_GOSSIP_MENU();
        creature->setFaction(FACTION_HOSTILE_RD);
        ((npc_raliq_the_drunkAI*)creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
# npc_salsalabim
######*/

#define FACTION_HOSTILE_SA              90
#define FACTION_FRIENDLY_SA             35
#define QUEST_10004                     10004

#define SPELL_MAGNETIC_PULL             31705
#define DEMON_SAY                       -1200136

struct npc_salsalabimAI : public ScriptedAI
{
    npc_salsalabimAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked MagneticPull_Timer;

    void Reset()
    {
        MagneticPull_Timer.Reset(15000);
        me->setFaction(FACTION_FRIENDLY_SA);
    }

    void EnterCombat(Unit *who) 
    {
        me->MonsterSay(-1200136, LANG_DEMONIC);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if( done_by->GetTypeId() == TYPEID_PLAYER )
            if( (me->GetHealth()-damage)*100 / me->GetMaxHealth() < 20 )
        {
            ((Player*)done_by)->GroupEventHappens(QUEST_10004,me);
            damage = 0;
            EnterEvadeMode();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (MagneticPull_Timer.Expired(diff))
        {
            if (m_creature->GetDistance(m_creature->GetVictim()) >= 8 &&
                m_creature->GetDistance(m_creature->GetVictim()) <= 40)
                DoCast(me->GetVictim(), SPELL_MAGNETIC_PULL);
            
            MagneticPull_Timer = 15000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_salsalabim(Creature *creature)
{
    return new npc_salsalabimAI (creature);
}

bool GossipHello_npc_salsalabim(Player *player, Creature *creature)
{
    if( player->GetQuestStatus(QUEST_10004) == QUEST_STATUS_INCOMPLETE )
    {
        creature->setFaction(FACTION_HOSTILE_SA);
        ((npc_salsalabimAI*)creature->AI())->AttackStart(player);
    }
    else
    {
        if( creature->isQuestGiver() )
            player->PrepareQuestMenu( creature->GetGUID() );
        player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    }
    return true;
}

/*
##################################################
Shattrath City Flask Vendors provides flasks to people exalted with 3 factions:
Haldor the Compulsive
Arcanist Xorith
Both sell special flasks for use in Outlands 25man raids only,
purchasable for one Mark of Illidari each
Purchase requires exalted reputation with Scryers/Aldor, Cenarion Expedition and The Sha'tar
##################################################
*/

bool GossipHello_npc_shattrathflaskvendors(Player *player, Creature *creature)
{
    if(creature->GetEntry() == 23484)
    {
        // Aldor vendor
        if( creature->isVendor() && (player->GetReputationMgr().GetRank(932) == REP_EXALTED) && (player->GetReputationMgr().GetRank(935) == REP_EXALTED) && (player->GetReputationMgr().GetRank(942) == REP_EXALTED) )
        {
            player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            player->SEND_GOSSIP_MENU(11085, creature->GetGUID());
        }
        else
        {
            player->SEND_GOSSIP_MENU(11083, creature->GetGUID());
        }
    }

    if(creature->GetEntry() == 23483)
    {
        // Scryers vendor
        if( creature->isVendor() && (player->GetReputationMgr().GetRank(934) == REP_EXALTED) && (player->GetReputationMgr().GetRank(935) == REP_EXALTED) && (player->GetReputationMgr().GetRank(942) == REP_EXALTED) )
        {
            player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            player->SEND_GOSSIP_MENU(11085, creature->GetGUID());
        }
        else
        {
            player->SEND_GOSSIP_MENU(11084, creature->GetGUID());
        }
    }

    return true;
}

bool GossipSelect_npc_shattrathflaskvendors(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if( action == GOSSIP_ACTION_TRADE )
        player->SEND_VENDORLIST( creature->GetGUID() );

    return true;
}

/*######
# npc_zephyr
######*/

#define GOSSIP_HZ 16052

bool GossipHello_npc_zephyr(Player *player, Creature *creature)
{
    if( player->GetReputationMgr().GetRank(989) >= REP_REVERED )
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HZ), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_zephyr(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if( action == GOSSIP_ACTION_INFO_DEF+1 )
        player->CastSpell(player,37778,false);

    return true;
}

/*######
# npc_kservant
######*/

enum
{
    SAY_KHAD_START          = -1900185,
    SAY_KHAD_SERV_0         = -1000306,
    SAY_KHAD_SERV_1         = -1000307,
    SAY_KHAD_SERV_2         = -1000308,
    SAY_KHAD_SERV_3         = -1000309,
    SAY_KHAD_SERV_4         = -1000310,
    SAY_KHAD_SERV_5         = -1000311,
    SAY_KHAD_SERV_6         = -1000312,
    SAY_KHAD_SERV_7         = -1000313,
    SAY_KHAD_SERV_8         = -1000314,
    SAY_KHAD_SERV_9         = -1000315,
    SAY_KHAD_SERV_10        = -1000316,
    SAY_KHAD_SERV_11        = -1000317,
    SAY_KHAD_SERV_12        = -1000318,
    SAY_KHAD_SERV_13        = -1000319,
    SAY_KHAD_SERV_14        = -1000320,
    SAY_KHAD_SERV_15        = -1000321,
    SAY_KHAD_SERV_16        = -1000322,
    SAY_KHAD_SERV_17        = -1000323,
    SAY_KHAD_SERV_18        = -1000324,
    SAY_KHAD_SERV_19        = -1000325,
    SAY_KHAD_SERV_20        = -1000326,
    SAY_KHAD_SERV_21        = -1000327,
    SAY_KHAD_INJURED        = -1900186,
    SAY_KHAD_MIND_YOU       = -1900187,
    SAY_KHAD_MIND_ALWAYS    = -1900188,
    SAY_KHAD_ALDOR_GREET    = -1900189,
    SAY_KHAD_SCRYER_GREET   = -1900190,
    SAY_KHAD_HAGGARD        = -1900191,

    NPC_KHADGAR             = 18166,
    NPC_SHANIR              = 18597,
    NPC_IZZARD              = 18622,
    NPC_ADYRIA              = 18596,
    NPC_ANCHORITE           = 19142,
    NPC_ARCANIST            = 18547,
    NPC_HAGGARD             = 19684,

    QUEST_CITY_LIGHT        = 10211
};

struct npc_kservantAI : public npc_escortAI
{
    npc_kservantAI(Creature *creature) : npc_escortAI(creature) {}

    uint32 PointId;
    Timer_UnCheked TalkTimer;
    uint32 TalkCount;
    Timer_UnCheked RandomTalkCooldown;

    void Reset()
    {
        TalkTimer.Reset(2500);
        TalkCount = 0;
        PointId = 0;
        RandomTalkCooldown = 0;

        me->SetReactState(REACT_PASSIVE);
        Start(false, false, me->GetCharmerOrOwnerGUID());
    }

    void EnterCombat(Unit* who) {}

    void MoveInLineOfSight(Unit* who)
    {
        if (!RandomTalkCooldown.GetInterval() && who->GetTypeId() == TYPEID_UNIT && me->IsWithinDistInMap(who, 10.0f))
        {
            switch(who->GetEntry())
            {
                case NPC_HAGGARD:
                    if (Player* player = GetPlayerForEscort())
                        DoScriptText(SAY_KHAD_HAGGARD, who, player);
                    RandomTalkCooldown = 7500;
                    break;
                case NPC_ANCHORITE:
                    if (Player* player = GetPlayerForEscort())
                        DoScriptText(SAY_KHAD_ALDOR_GREET, who, player);
                    RandomTalkCooldown = 7500;
                    break;
                case NPC_ARCANIST:
                    if (Player* player = GetPlayerForEscort())
                        DoScriptText(SAY_KHAD_SCRYER_GREET, who, player);
                    RandomTalkCooldown = 7500;
                    break;
            }
        }
    }

    void WaypointReached(uint32 i)
    {
        PointId = i;

        switch(i)
        {
            case 0:
                if (Creature* Khadgar = GetClosestCreatureWithEntry(me, NPC_KHADGAR, 10.0f))
                    DoScriptText(SAY_KHAD_START, Khadgar);
                break;
            case 1:
                DoScriptText(SAY_KHAD_SERV_0, me);
                break;
            case 5:
            case 24:
            case 50:
            case 63:
            case 74:
            case 75:
                SetEscortPaused(true);
                break;
            case 34:
                if (Creature* Izzard = GetClosestCreatureWithEntry(me, NPC_IZZARD, 10.0f))
                    DoScriptText(SAY_KHAD_MIND_YOU, Izzard);
                break;
            case 35:
                if (Creature* Adyria = GetClosestCreatureWithEntry(me, NPC_ADYRIA, 10.0f))
                    DoScriptText(SAY_KHAD_MIND_ALWAYS, Adyria);
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (RandomTalkCooldown.Expired(diff))
            RandomTalkCooldown = 0;

        if (HasEscortState(STATE_ESCORT_PAUSED))
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            if (TalkTimer.Expired(diff))
            {
                TalkTimer = 7500;

                switch(PointId)
                {
                    case 5:
                    {
                        switch(TalkCount)
                        {
                            case 1:
                                DoScriptText(SAY_KHAD_SERV_1, me, player);
                                break;
                            case 2:
                                DoScriptText(SAY_KHAD_SERV_2, me, player);
                                break;
                            case 3:
                                DoScriptText(SAY_KHAD_SERV_3, me, player);
                                break;
                            case 4:
                                DoScriptText(SAY_KHAD_SERV_4, me, player);
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                    }
                    case 24:
                    {
                        switch(TalkCount)
                        {
                            case 5:
                                if (Creature* Shanir = GetClosestCreatureWithEntry(me, NPC_SHANIR, 10.0f))
                                    DoScriptText(SAY_KHAD_INJURED, Shanir, player);

                                DoScriptText(SAY_KHAD_SERV_5, me, player);
                                break;
                            case 6:
                                DoScriptText(SAY_KHAD_SERV_6, me, player);
                                break;
                            case 7:
                                DoScriptText(SAY_KHAD_SERV_7, me, player);
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                    }
                    case 50:
                    {
                        switch(TalkCount)
                        {
                            case 8:
                                DoScriptText(SAY_KHAD_SERV_8, me, player);
                                break;
                            case 9:
                                DoScriptText(SAY_KHAD_SERV_9, me, player);
                                break;
                            case 10:
                                DoScriptText(SAY_KHAD_SERV_10, me, player);
                                break;
                            case 11:
                                DoScriptText(SAY_KHAD_SERV_11, me, player);
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                    }
                    case 63:
                    {
                        switch(TalkCount)
                        {
                            case 12:
                                DoScriptText(SAY_KHAD_SERV_12, me, player);
                                break;
                            case 13:
                                DoScriptText(SAY_KHAD_SERV_13, me, player);
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                    }
                    case 74:
                    {
                        switch(TalkCount)
                        {
                            case 14:
                                DoScriptText(SAY_KHAD_SERV_14, me, player);
                                break;
                            case 15:
                                DoScriptText(SAY_KHAD_SERV_15, me, player);
                                break;
                            case 16:
                                DoScriptText(SAY_KHAD_SERV_16, me, player);
                                break;
                            case 17:
                                DoScriptText(SAY_KHAD_SERV_17, me, player);
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                    }
                    case 75:
                    {
                        switch(TalkCount)
                        {
                            case 18:
                                DoScriptText(SAY_KHAD_SERV_18, me, player);
                                break;
                            case 19:
                                DoScriptText(SAY_KHAD_SERV_19, me, player);
                                break;
                            case 20:
                                DoScriptText(SAY_KHAD_SERV_20, me, player);
                                break;
                            case 21:
                                DoScriptText(SAY_KHAD_SERV_21, me, player);
                                player->GroupEventHappens(QUEST_CITY_LIGHT, me);
                                SetEscortPaused(false);
                                break;
                        }
                        break;
                    }
                }
                ++TalkCount;
            }
        }
        return;
    }
};

CreatureAI* GetAI_npc_kservant(Creature* creature)
{
    return new npc_kservantAI(creature);
}

/*######
# npc_dirty_larry
######*/

#define GOSSIP_BOOK 16053

#define SAY_1       -1000328
#define SAY_2       -1000329
#define SAY_3       -1000330
#define SAY_4       -1000331
#define SAY_5       -1000332
#define SAY_GIVEUP  -1000333

#define QUEST_WBI       10231
#define NPC_CREEPJACK   19726
#define NPC_MALONE      19725

struct npc_dirty_larryAI : public ScriptedAI
{
    npc_dirty_larryAI(Creature* creature) : ScriptedAI(creature) {}

    bool Event;

    uint64 PlayerGUID;

    Timer_UnCheked SayTimer;
    Timer_UnCheked EvadeTimer;
    uint32 Step;

    void Reset()
    {
        Event = false;
        PlayerGUID = 0;
        SayTimer.Reset(1);
        Step = 0;

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->setFaction(1194);

        Creature* Creepjack = (Creature*)FindCreature(NPC_CREEPJACK, 50, me);
        if (Creepjack)
        {
            if (Creepjack->isDead())
                Creepjack->Respawn();

            Creepjack->AI()->EnterEvadeMode();
            Creepjack->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            Creepjack->setFaction(1194);
            
        }

        Creature* Malone = (Creature*)FindCreature(NPC_MALONE, 50, me);
        if (Malone)
        {
            if (Malone->isDead())
                Malone->Respawn();

            Malone->AI()->EnterEvadeMode();
            Malone->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            Malone->setFaction(1194);  
        }
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (m_creature->GetHealth() <= damage)
        {
            damage = 0;
            m_creature->SetHealth(1);
        }
    }

    uint32 NextStep(uint32 Step)
    {
        Player *player = Unit::GetPlayerInWorld(PlayerGUID);

        switch(Step)
        {
            case 1:
            {
                me->SetFacingTo(m_creature->GetOrientationTo(player));
                
                Unit* Creepjack = FindCreature(NPC_CREEPJACK, 20, me);
                if (Creepjack)
                    Creepjack->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                
                Unit* Malone = FindCreature(NPC_MALONE, 20, me);
                if (Malone)
                    Malone->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                
                return 2000;
            }
            case 2: 
                DoScriptText(SAY_1, me, player); 
                return 3000;
            case 3: 
                DoScriptText(SAY_2, me, player); 
                return 5000;
            case 4: 
                DoScriptText(SAY_3, me, player); 
                return 2000;
            case 5: 
                DoScriptText(SAY_4, me, player); 
                return 2000;
            case 6: 
                DoScriptText(SAY_5, me, player); 
                return 2000;
            case 7: 
            {
                Player *player = Unit::GetPlayerInWorld(PlayerGUID);
                me->setFaction(14);
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);

                if (player)
                {
                    Unit* Creepjack = FindCreature(NPC_CREEPJACK, 20, me);
                    if (Creepjack)
                    {
                        if (Creepjack->isDead())
                            ((Creature*)Creepjack)->Respawn();

                        Creepjack->Attack(player, true);
                        Creepjack->setFaction(14);
                        Creepjack->GetMotionMaster()->MoveChase(player);
                        Creepjack->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                    }

                    Unit* Malone = FindCreature(NPC_MALONE, 20, me);
                    if (Malone)
                    {
                        if (Malone->isDead())
                            ((Creature*)Malone)->Respawn();

                        Malone->Attack(player, true);
                        Malone->setFaction(14);
                        Malone->GetMotionMaster()->MoveChase(player);
                        Malone->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                    }

                    DoStartMovement(player);
                    AttackStart(player);
                }

                return 0;
            }

            default: 
                return 0;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (SayTimer.Expired(diff))
        {
            if (Event)
                SayTimer.Reset(NextStep(++Step));
        }

        if (!UpdateVictim())
            return; 

        if (me->GetHealthPercent() < 5)
        {
            DoScriptText(SAY_GIVEUP, me, NULL);
            
            Player* player = Unit::GetPlayerInWorld(PlayerGUID);
            if (player)
                player->GroupEventHappens(QUEST_WBI, me);

            me->AI()->EnterEvadeMode();
        }

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_dirty_larry(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if(player->GetQuestStatus(QUEST_WBI) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_BOOK), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_dirty_larry(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        CAST_AI(npc_dirty_larryAI, creature->AI())->Event = true;
        CAST_AI(npc_dirty_larryAI, creature->AI())->PlayerGUID = player->GetGUID();
        player->CLOSE_GOSSIP_MENU();
    }

    return true;
}

CreatureAI* GetAI_npc_dirty_larryAI(Creature *creature)
{
    return new npc_dirty_larryAI (creature);
}

/*######
# npc_ishanah
######*/

#define ISANAH_GOSSIP_1 16054
#define ISANAH_GOSSIP_2 16055

bool GossipHello_npc_ishanah(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(ISANAH_GOSSIP_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(ISANAH_GOSSIP_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_ishanah(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if(action == GOSSIP_ACTION_INFO_DEF+1)
        player->SEND_GOSSIP_MENU(9458, creature->GetGUID());
    else if(action == GOSSIP_ACTION_INFO_DEF+2)
        player->SEND_GOSSIP_MENU(9459, creature->GetGUID());

    return true;
}

/*######
# npc_khadgar
######*/

#define KHADGAR_GOSSIP_1    16056
#define KHADGAR_GOSSIP_2    16057
#define KHADGAR_GOSSIP_3    16058 //6th too this
#define KHADGAR_GOSSIP_4    16059
#define KHADGAR_GOSSIP_5    16060
#define KHADGAR_GOSSIP_7    16061

bool GossipHello_npc_khadgar(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if(!player->hasQuest(10211))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(9243, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_khadgar(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch(action)
    {
    case GOSSIP_ACTION_INFO_DEF+1:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->SEND_GOSSIP_MENU(9876, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+2:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
        player->SEND_GOSSIP_MENU(9877, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+3:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
        player->SEND_GOSSIP_MENU(9878, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+4:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
        player->SEND_GOSSIP_MENU(9879, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+5:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
        player->SEND_GOSSIP_MENU(9880, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+6:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
        player->SEND_GOSSIP_MENU(9881, creature->GetGUID());
        break;
    case GOSSIP_ACTION_INFO_DEF+7:
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(KHADGAR_GOSSIP_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(9243, creature->GetGUID());
        break;
    }
    return true;
}

/*######
 # Event after rewarding quest Kael'thas and the Verdant Sphere
 ######*/

#define QUEST_VERDANT_SPHERE                11007

#define GO_FIRE                             185170
#define CREATURE_ADAL                       18481
#define CREATURE_KAEL                       23054

#define SPELL_ADALS_SONG_OF_BATTLE          39953
#define SPELL_KAELTHAS_DEFEATED             39966
#define SPELL_OTHERWORLDLY_PORTAL           39952
#define SPELL_MARK_OF_KAELTHAS              37364

#define ADAL_WHISPER1                       -1200141
#define ADAL_WHISPER2                       -1200142
#define ADAL_WHISPER3_1                     -1200143
#define ADAL_WHISPER3_2                     -1200144
#define ADAL_WHISPER3_3                     -1200145
#define ADAL_WHISPER4                       -1200146
#define KAEL_YELL1                          -1200147
#define KAEL_YELL2                          -1200148
#define KAEL_YELL3                          -1200149
#define KAEL_YELL4                          -1200150



struct npc_kaelthas_imageAI : public ScriptedAI
{
    npc_kaelthas_imageAI(Creature* creature) : ScriptedAI(creature) {}

    uint8 Step;
    Timer_UnCheked NextStep_Timer;
    std::list<uint64> PlayersInCity;
    bool Init;
    uint64 Defeater_GUID;
    //std::string Defeater_Name;
    //std::string Defeater_Gender;
    uint64 FireGO;

    void Reset()
    {
        Step = 0;
        NextStep_Timer.Reset(2000);
        PlayersInCity.clear();
        Init = false;
        FireGO = 0;
    }

    void EmoteTo(Creature* sender, const char *text, Player *target)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        sender->BuildMonsterChat(&data,CHAT_MSG_MONSTER_EMOTE,text,0,sender->GetName(), target->GetGUID(), true);
        target->GetSession()->SendPacket(&data);
    }

    void YellTo(Creature* sender, const char* text, Player *target)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        sender->BuildMonsterChat(&data,CHAT_MSG_MONSTER_YELL,text,0,sender->GetName(), target->GetGUID());
        target->GetSession()->SendPacket(&data);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Init)
        {
            //std::list<Unit*> PlayerList;
            //uint32 shattrathRadius = 1000;
            //Hellground::AnyUnitInObjectRangeCheck  check(me, shattrathRadius);
            //Hellground::UnitListSearcher<Hellground::AnyUnitInObjectRangeCheck > searcher(PlayerList, check);
            //Cell::VisitWorldObjects(me, searcher, shattrathRadius);

            //for(std::list<Unit*>::iterator i = PlayerList.begin(); i != PlayerList.end(); i++)
            //{
            //    if((*i)->GetTypeId() != TYPEID_PLAYER || ((Player*)(*i))->GetCachedZone() != 3703)
            //        continue;
            //    PlayersInCity.push_back((*i)->GetGUID());
            //}

			PlayersInCity.push_back(Defeater_GUID);
			
            me->SetLevitate(true);
            me->SetVisibility(VISIBILITY_OFF);
            me->StopMoving();
            //DoCast(me, SPELL_OTHERWORLDLY_PORTAL, true);
            DoCast(me, SPELL_KAELTHAS_DEFEATED, true);
            //DoCast(me, SPELL_MARK_OF_KAELTHAS, true);
            Init = true;
        }

        if(NextStep_Timer.Expired(diff))
        {
			if (sWorld.isEasyRealm())
			{
				if (FireGO)
					if (GameObject* fire = me->GetMap()->GetGameObject(FireGO))
						fire->Delete();
				Reset();
				return;
			}
			
			NextStep_Timer = 13000;
            Creature* adal = NULL;

            if(Step < 4)
            {
                if(Unit *u = FindCreature(CREATURE_ADAL, 100, me))
                    if(u->GetTypeId() == TYPEID_UNIT)
                        adal = (Creature*)u;

                if(!adal)
                    return;
            }

            std::string s;
            switch(Step)
            {
                case 0:
					for (std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
						if (Unit *u = me->GetUnit(*me, (*i)))
							adal->MonsterTextEmoteToZone(-1200141, 0, u->GetGUID());
                            //EmoteTo(adal, -1200141, (Player*)u);
                    NextStep_Timer = 7000;
                    break;
                case 1:
                    for(std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
                        if(Unit *u = me->GetUnit(*me, (*i)))
							u->MonsterWhisper(-1200142, u->GetGUID(), true);
							//EmoteTo(adal, -1200142, (Player*)u);
                    NextStep_Timer = 7000;
                    break;
                case 2:
                    for(std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
                        if(Unit *u = me->GetUnit(*me, (*i)))
							adal->MonsterYellToZone(-1200143, 0, u->GetGUID());
							//YellTo(adal, s.c_str(), (Player*)u);
                    break;
                case 3:
                    for(std::list<uint64>::iterator i = PlayersInCity.begin(); i != PlayersInCity.end(); i++)
                    {
                        if(Unit *u = me->GetUnit(*me, (*i)))
                        {
                            u->CastSpell(u, 39953, true);
							adal->MonsterYellToZone(-1200146, 0, u->GetGUID());
                        }
                    }

                    me->SetVisibility(VISIBILITY_ON);
                    break;
				case 4:
					me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
					me->Yell(-1200147, 0, 0);
					break;
				case 5:
					me->HandleEmoteCommand(EMOTE_ONESHOT_NO);
					me->Yell(-1200148, 0, 0);
					NextStep_Timer = 2000;
					break;
				case 6:
					me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
					break;
				case 7:
					me->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
					me->Yell(-1200149, 0, 0);
					NextStep_Timer = 3000;
					break;
				case 8:
					me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
					break;
				case 9:
					me->HandleEmoteCommand(EMOTE_ONESHOT_RUDE);
					me->Yell(-1200150, 0, 0);
					NextStep_Timer = 6000;
					break;
				case 10:
					if (FireGO)
						if (GameObject* fire = me->GetMap()->GetGameObject(FireGO))
							fire->Delete();
					break;

            }
            Step++;
        }
    }
};

CreatureAI* GetAI_npc_kaelthas_imageAI(Creature *creature)
{
    return new npc_kaelthas_imageAI (creature);
}

bool ChooseReward_npc_Adal(Player *player, Creature *creature, const Quest *quest)
{
    if(quest->GetQuestId() == QUEST_VERDANT_SPHERE)
    {
        GameObject *fire = creature->SummonGameObject(185170, -1831.9, 5429.7, -1.5, 0, 0, 0, 0, 0, 0);
		Creature* kael = creature->SummonCreature(CREATURE_KAEL, -1839.6, 5429.8, -1.5, 3.1214, TEMPSUMMON_TIMED_DESPAWN, 90000);

        ((npc_kaelthas_imageAI*)kael->AI())->Defeater_GUID = player->GetGUID();
        //((npc_kaelthas_imageAI*)kael->AI())->Defeater_Name = player->GetName();
        //((npc_kaelthas_imageAI*)kael->AI())->Defeater_Gender = player->GetGender() == GENDER_MALE ? "his" : "her";
        if(fire)
            ((npc_kaelthas_imageAI*)kael->AI())->FireGO = fire->GetGUID();
    }
    return false;
}

struct npc_defender_grashnaAI : public ScriptedAI
{
    npc_defender_grashnaAI(Creature* pCreature) : ScriptedAI(pCreature), Summons(pCreature)
    {
    }

    uint32 SummonTimer;
    uint8 WaveCounter;
    uint64 PlayerGUID;
    SummonList Summons;

    void Reset()
    {
        WaveCounter = 0;
        PlayerGUID = 0;
    }
    void StartEvent(Player* pPlayer)
    {
        Unit* BirdsInRange = FindCreature(22376, 100, me);
        Unit* BossBirdInRange = FindCreature(22375, 100, me);
        if(!BirdsInRange || !BossBirdInRange)
        {
            PlayerGUID = pPlayer->GetGUID();
            SummonTimer = 3000;
        }
    }

    void JustSummoned(Creature *summoned)
    {
        Summons.Summon(summoned);
        summoned->AI()->AttackStart(Unit::GetPlayerInWorld(PlayerGUID));
        summoned->SetPlayerDamageReqInPct(0.05);
    }

    void UpdateAI(const uint32 diff)
    {
        if(SummonTimer)
        {
            if(SummonTimer <= diff)
            {
                switch(WaveCounter)
                {
                    case 0:
                    {
                        me->Say(-1200137, LANG_UNIVERSAL, 0);
                        me->SummonCreature(22376, -1626.12, 5402.47, -42.31, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        me->SummonCreature(22376, -1631.94, 5400.56, -43.863, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        SummonTimer = 30000;
                        WaveCounter++;
                        break;
                    }
                    case 1:
                    {
                        me->Say(-1200138, LANG_UNIVERSAL, 0);
                        me->SummonCreature(22376, -1626.12, 5402.47, -42.31, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        me->SummonCreature(22376, -1631.94, 5400.56, -43.863, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        SummonTimer = 30000;
                        WaveCounter++;
                        break;
                    }
                    case 2:
                    {
                        me->Say(-1200139, LANG_UNIVERSAL, 0);
                        me->SummonCreature(22376, -1626.12, 5402.47, -42.31, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        me->SummonCreature(22376, -1631.94, 5400.56, -43.863, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        me->SummonCreature(22376, -1627.66, 5403.44, -42.6365, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        SummonTimer = 30000;
                        WaveCounter++;
                        break;
                    }
                    case 3:
                    {
                        me->Say(-1200140, LANG_UNIVERSAL, 0);
                        me->SummonCreature(22375, -1631.94, 5400.56, -43.863, 2.08131, TEMPSUMMON_TIMED_DESPAWN, 60000);
                        WaveCounter = 0;
                        SummonTimer = 0;
                        break;
                    }
                }
            } else SummonTimer -= diff;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_defender_grashna(Creature* pCreature)
{
    return new npc_defender_grashnaAI(pCreature);
}

bool QuestAccept_npc_defender_grashna(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == 10879)
    {
        if (npc_defender_grashnaAI* pGrashna = dynamic_cast<npc_defender_grashnaAI*>(pCreature->AI()))
            pGrashna->StartEvent(pPlayer);
    }

    return true;
}

void AddSC_shattrath_city()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_raliq_the_drunk";
    newscript->GetAI =  &GetAI_npc_raliq_the_drunk;
    newscript->pGossipHello =  &GossipHello_npc_raliq_the_drunk;
    newscript->pGossipSelect = &GossipSelect_npc_raliq_the_drunk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_salsalabim";
    newscript->GetAI = &GetAI_npc_salsalabim;
    newscript->pGossipHello =  &GossipHello_npc_salsalabim;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_shattrathflaskvendors";
    newscript->pGossipHello =  &GossipHello_npc_shattrathflaskvendors;
    newscript->pGossipSelect = &GossipSelect_npc_shattrathflaskvendors;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_zephyr";
    newscript->pGossipHello =  &GossipHello_npc_zephyr;
    newscript->pGossipSelect = &GossipSelect_npc_zephyr;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kservant";
    newscript->GetAI = &GetAI_npc_kservant;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_dirty_larry";
    newscript->GetAI = &GetAI_npc_dirty_larryAI;
    newscript->pGossipHello =   &GossipHello_npc_dirty_larry;
    newscript->pGossipSelect = &GossipSelect_npc_dirty_larry;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ishanah";
    newscript->pGossipHello =  &GossipHello_npc_ishanah;
    newscript->pGossipSelect = &GossipSelect_npc_ishanah;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_khadgar";
    newscript->pGossipHello =  &GossipHello_npc_khadgar;
    newscript->pGossipSelect = &GossipSelect_npc_khadgar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kaelthas_image";
    newscript->GetAI = &GetAI_npc_kaelthas_imageAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_adal";
    newscript->pQuestRewardedNPC = &ChooseReward_npc_Adal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_defender_grashna";
    newscript->GetAI = &GetAI_npc_defender_grashna;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_defender_grashna;
    newscript->RegisterSelf();
}

