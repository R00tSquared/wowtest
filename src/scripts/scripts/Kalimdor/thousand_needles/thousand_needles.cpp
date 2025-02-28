// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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
SDName: Thousand Needles
SD%Complete: 100
SDComment: Support for Quest: 4770, 1950, 4904, 4966, 5151.
SDCategory: Thousand Needles
EndScriptData */

/* ContentData
npc_kanati
npc_lakota_windsong
npc_swiftmountain
npc_plucky
go_panther_cage
npc_enraged_panther
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "follower_ai.h"
#include "Language.h"

/*#####
# npc_kanati
######*/

enum eKanati
{
    SAY_KAN_START              = -1600410,

    QUEST_PROTECT_KANATI        = 4966,
    NPC_GALAK_ASS               = 10720
};

const float m_afGalakLoc[]= {-4867.387695, -1357.353760, -48.226 };

struct npc_kanatiAI : public ScriptedAI
{
    npc_kanatiAI(Creature* pCreature) : ScriptedAI(pCreature), Summons(me) { }

    SummonList Summons;
    uint32 SummonsDeadCounter;
    uint64 PlayerGUID;

    void Reset()
    {
        me->GetMotionMaster()->MoveTargetedHome();
        Summons.DespawnAll();
        SummonsDeadCounter = 0;
        PlayerGUID = 0;
    }

    void MovementInform(uint32 Type, uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 100:
                DoScriptText(SAY_KAN_START, me);
                DoSpawnGalak();
                break;
            case 101:
                if (Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID))
                    pPlayer->GroupEventHappens(QUEST_PROTECT_KANATI, me);
                Reset();
                me->SetFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
                break;
        }
    }

    void DoSpawnGalak()
    {
        for (int i = 0; i < 3; ++i)
        {
            me->SummonCreature(NPC_GALAK_ASS, m_afGalakLoc[0], m_afGalakLoc[1], m_afGalakLoc[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
        }
    }

    void SummonedCreatureDies(Creature* /*unit*/, Unit* /*killer*/)
    {
        SummonsDeadCounter++;
        if(SummonsDeadCounter >= 3)
            me->GetMotionMaster()->MovePoint(101, -4906, -1367.05, -52.611);
    }

    void JustSummoned(Creature* pSummoned)
    {
        Summons.Summon(pSummoned);
        pSummoned->AI()->AttackStart(me);
    }
};

CreatureAI* GetAI_npc_kanati(Creature* pCreature)
{
    npc_kanatiAI* kanatiAI = new npc_kanatiAI(pCreature);

    return (CreatureAI*)kanatiAI;
}

bool QuestAccept_npc_kanati(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_PROTECT_KANATI)
    {
        
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
        pCreature->GetMotionMaster()->MovePoint(100, -4903.52, -1368.34, -52.611);
        if (npc_kanatiAI* kanati = CAST_AI(npc_kanatiAI, pCreature->AI()))
            kanati->PlayerGUID = pPlayer->GetGUID();
        //if (npc_kanatiAI* pEscortAI = CAST_AI(npc_kanatiAI, pCreature->AI()))
            //pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest, true, false);
    }
    return true;
}

bool QuestComplete_npc_kanati(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 4881)
        ChatHandler(player).PSendSysMessage("%s", player->GetSession()->GetHellgroundString(LANG_QUEST_4881_COMPLETE));

    return true;
}

/*######
# npc_lakota_windsong
######*/

enum eLakota
{
    SAY_LAKO_START              = -1600365,
    SAY_LAKO_LOOK_OUT           = -1600366,
    SAY_LAKO_HERE_COME          = -1600367,
    SAY_LAKO_MORE               = -1600368,
    SAY_LAKO_END                = -1600369,

    QUEST_FREE_AT_LAST          = 4904,
    NPC_GRIM_BANDIT             = 10758,
    FACTION_ESCORTEE_LAKO       = 232,                      //guessed

    ID_AMBUSH_1                 = 0,
    ID_AMBUSH_2                 = 2,
    ID_AMBUSH_3                 = 4
};

float m_afBanditLoc[6][6]=
{
    {-4905.479492, -2062.732666, 84.352},
    {-4915.201172, -2073.528320, 84.733},
    {-4878.883301, -1986.947876, 91.966},
    {-4877.503906, -1966.113403, 91.859},
    {-4767.985352, -1873.169189, 90.192},
    {-4788.861328, -1888.007813, 89.888}
};

struct npc_lakota_windsongAI : public npc_escortAI
{
    npc_lakota_windsongAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    void Reset() { }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 8:
                DoScriptText(SAY_LAKO_LOOK_OUT, me);
                DoSpawnBandits(ID_AMBUSH_1);
                break;
            case 14:
                DoScriptText(SAY_LAKO_HERE_COME, me);
                DoSpawnBandits(ID_AMBUSH_2);
                break;
            case 21:
                DoScriptText(SAY_LAKO_MORE, me);
                DoSpawnBandits(ID_AMBUSH_3);
                break;
            case 29:
            case 33:
                me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);
                break;
            case 30:
            case 38:
                me->ClearUnitState(UNIT_STAT_IGNORE_PATHFINDING);
                break;
            case 44:
                DoScriptText(SAY_LAKO_END, me);
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_FREE_AT_LAST, me);
                break;
        }
    }

    void DoSpawnBandits(int uiAmbushId)
    {
        for (int i = 0; i < 2; ++i)
            me->SummonCreature(NPC_GRIM_BANDIT,
            m_afBanditLoc[i+uiAmbushId][0], m_afBanditLoc[i+uiAmbushId][1], m_afBanditLoc[i+uiAmbushId][2], 0.0f,
            TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
    }
};

CreatureAI* GetAI_npc_lakota_windsong(Creature* pCreature)
{
    npc_lakota_windsongAI* lakota_windsongAI = new npc_lakota_windsongAI(pCreature);

    lakota_windsongAI->AddWaypoint(0, -4792.4, -2137.78, 82.423);
    lakota_windsongAI->AddWaypoint(1, -4813.51, -2141.54, 80.774);
    lakota_windsongAI->AddWaypoint(2, -4828.63, -2154.31, 82.074);
    lakota_windsongAI->AddWaypoint(3, -4833.77, -2149.18, 81.676);
    lakota_windsongAI->AddWaypoint(4, -4846.42, -2136.05, 77.871);
    lakota_windsongAI->AddWaypoint(5, -4865.08, -2116.55, 76.483);
    lakota_windsongAI->AddWaypoint(6, -4888.43, -2090.73, 80.907);
    lakota_windsongAI->AddWaypoint(7, -4893.07, -2085.47, 82.094);
    lakota_windsongAI->AddWaypoint(8, -4907.26, -2074.93, 84.437, 5000);
    lakota_windsongAI->AddWaypoint(9, -4899.9, -2062.14, 83.78);
    lakota_windsongAI->AddWaypoint(10, -4897.76, -2056.52, 84.184);
    lakota_windsongAI->AddWaypoint(11, -4888.33, -2033.18, 83.654);
    lakota_windsongAI->AddWaypoint(12, -4876.34, -2003.92, 90.887);
    lakota_windsongAI->AddWaypoint(13, -4872.23, -1994.17, 91.513);
    lakota_windsongAI->AddWaypoint(14, -4879.57, -1976.99, 92.185, 5000);
    lakota_windsongAI->AddWaypoint(15, -4879.05, -1964.35, 92.001);
    lakota_windsongAI->AddWaypoint(16, -4874.72, -1956.94, 90.737);
    lakota_windsongAI->AddWaypoint(17, -4869.47, -1952.61, 89.206);
    lakota_windsongAI->AddWaypoint(18, -4842.47, -1929, 84.147);
    lakota_windsongAI->AddWaypoint(19, -4804.44, -1897.3, 89.362);
    lakota_windsongAI->AddWaypoint(20, -4798.07, -1892.38, 89.368);
    lakota_windsongAI->AddWaypoint(21, -4779.45, -1882.76, 90.169, 5000);
    lakota_windsongAI->AddWaypoint(22, -4762.08, -1866.53, 89.481);
    lakota_windsongAI->AddWaypoint(23, -4766.27, -1861.87, 87.847);
    lakota_windsongAI->AddWaypoint(24, -4782.93, -1852.17, 78.354);
    lakota_windsongAI->AddWaypoint(25, -4793.61, -1850.96, 77.658);
    lakota_windsongAI->AddWaypoint(26, -4803.32, -1855.1, 78.958);
    lakota_windsongAI->AddWaypoint(27, -4807.97, -1854.5, 77.743);
    lakota_windsongAI->AddWaypoint(28, -4837.21, -1848.49, 64.488);
    lakota_windsongAI->AddWaypoint(29, -4884.62, -1840.4, 56.219);
    lakota_windsongAI->AddWaypoint(30, -4892.15, -1839.44, 54.639);
    lakota_windsongAI->AddWaypoint(31, -4893.9, -1843.69, 53.012);
    lakota_windsongAI->AddWaypoint(32, -4903.14, -1872.38, 32.266);
    lakota_windsongAI->AddWaypoint(33, -4910.94, -1879.86, 29.94);
    lakota_windsongAI->AddWaypoint(34, -4920.05, -1880.94, 30.597);
    lakota_windsongAI->AddWaypoint(35, -4924.46, -1881.45, 29.292);
    lakota_windsongAI->AddWaypoint(37, -4966.12, -1886.03, 10.977);
    lakota_windsongAI->AddWaypoint(38, -4999.37, -1890.85, 4.43);
    lakota_windsongAI->AddWaypoint(39, -5007.27, -1891.67, 2.771);
    lakota_windsongAI->AddWaypoint(40, -5013.33, -1879.59, -1.947);
    lakota_windsongAI->AddWaypoint(41, -5023.33, -1855.96, -17.103);
    lakota_windsongAI->AddWaypoint(42, -5038.51, -1825.99, -35.821);
    lakota_windsongAI->AddWaypoint(43, -5048.73, -1809.8, -46.457);
    lakota_windsongAI->AddWaypoint(44, -5052.66, -1797.04, -54.734, 5000);

    return (CreatureAI*)lakota_windsongAI;
}

bool QuestAccept_npc_lakota_windsong(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_FREE_AT_LAST)
    {
        DoScriptText(SAY_LAKO_START, pCreature, pPlayer);
        pCreature->setFaction(FACTION_ESCORT_H_NEUTRAL_ACTIVE);

        if (npc_lakota_windsongAI* pEscortAI = CAST_AI(npc_lakota_windsongAI, pCreature->AI()))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest);
    }
    return true;
}

/*#####
# npc_swiftmountain
######*/

enum ePacka
{
    SAY_START           = -1000147,
    SAY_WYVERN          = -1000148,
    SAY_COMPLETE        = -1000149,

    QUEST_HOMEWARD      = 4770,
    NPC_WYVERN          = 4107,
    FACTION_ESCORTEE    = 232                               //guessed
};

float m_afWyvernLoc[3][3]=
{
    {-4990.606, -906.057, -5.343},
    {-4970.241, -927.378, -4.951},
    {-4985.364, -952.528, -5.199}
};

struct npc_swiftmountainAI : public npc_escortAI
{
    npc_swiftmountainAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    void Reset() { }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 15:
                DoScriptText(SAY_WYVERN, me);
                DoSpawnWyvern();
                break;
            case 26:
                DoScriptText(SAY_COMPLETE, me);
                break;
            case 27:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_HOMEWARD, me);
                break;
        }
    }

    void DoSpawnWyvern()
    {
        for (int i = 0; i < 3; ++i)
            me->SummonCreature(NPC_WYVERN,
            m_afWyvernLoc[i][0], m_afWyvernLoc[i][1], m_afWyvernLoc[i][2], 0.0f,
            TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
    }
};


bool QuestAccept_npc_swiftmountain(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_HOMEWARD)
    {
        DoScriptText(SAY_START, pCreature, pPlayer);
        pCreature->setFaction(FACTION_ESCORT_H_NEUTRAL_ACTIVE);

        if (npc_swiftmountainAI* pEscortAI = CAST_AI(npc_swiftmountainAI,pCreature->AI()))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest);
    }
    return true;
}

CreatureAI* GetAI_npc_swiftmountain(Creature* pCreature)
{
    npc_swiftmountainAI* thisAI = new npc_swiftmountainAI(pCreature);

   thisAI->AddWaypoint(0, -5185.46, -1185.93, 45.951, 0);
   thisAI->AddWaypoint(1, -5184.88, -1154.21, 45.035, 0);
   thisAI->AddWaypoint(2, -5175.88, -1126.53, 43.701, 0);
   thisAI->AddWaypoint(3, -5138.65, -1111.87, 44.024, 0);
   thisAI->AddWaypoint(4, -5134.73, -1104.8 , 47.365, 0);
   thisAI->AddWaypoint(5, -5129.68, -1097.88, 49.449, 2500);
   thisAI->AddWaypoint(6, -5125.3, -1080.57, 47.033, 0);
   thisAI->AddWaypoint(7, -5146.67, -1053.69, 28.415, 0);
   thisAI->AddWaypoint(8, -5147.46, -1027.54, 13.818, 0);
   thisAI->AddWaypoint(9, -5139.24, -1018.89, 8.22, 0);
   thisAI->AddWaypoint(10, -5121.17, -1013.13, -0.619, 0);
   thisAI->AddWaypoint(11, -5091.92, -1014.21, -4.902, 0);
   thisAI->AddWaypoint(12, -5069.24, -994.299, -4.631, 0);
   thisAI->AddWaypoint(13, -5059.98, -944.112, -5.377, 0);
   thisAI->AddWaypoint(14, -5013.55, -906.184, -5.49, 0);
   thisAI->AddWaypoint(15, -4992.46, -920.983, -4.98, 5000);
   thisAI->AddWaypoint(16, -4976.35, -1003, -5.38, 0);
   thisAI->AddWaypoint(17, -4958.48, -1033.19, -5.433, 0);
   thisAI->AddWaypoint(18, -4953.35, -1052.21, -10.83, 0);
   thisAI->AddWaypoint(19, -4937.45, -1056.35, -22.13, 0);
   thisAI->AddWaypoint(20, -4908.46, -1050.43, -33.45, 0);
   thisAI->AddWaypoint(21, -4905.53, -1056.89, -33.72, 0);
   thisAI->AddWaypoint(22, -4920.83, -1073.28, -45.51, 0);
   thisAI->AddWaypoint(23, -4933.37, -1082.7, -50.18, 0);
   thisAI->AddWaypoint(24, -4935.31, -1092.35, -52.78, 0);
   thisAI->AddWaypoint(25, -4929.55, -1101.27, -50.63, 0);
   thisAI->AddWaypoint(26, -4920.68, -1100.03, -51.94, 10000);
   thisAI->AddWaypoint(27, -4920.68, -1100.03, -51.94, 0);

    return (CreatureAI*)thisAI;
}

/*#####
# npc_plucky
######*/

#define GOSSIP_P    16387

#define SPELL_TRANSFORM_HUMAN 9192
#define QUEST_GET_THE_SCOOP 1950

struct npc_pluckyAI : public ScriptedAI
{
    npc_pluckyAI(Creature *c) : ScriptedAI(c) {}

    bool Transformed;
    bool Chicken;

    uint32 Timer;
    uint32 ChickenTimer;

    void Reset()   {

       Transformed = false;
       Chicken     = false;
       me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
       Timer        = 0;
       ChickenTimer = 0;
       }

    void EnterCombat(Unit *who){}

    void TransformHuman(uint32 emoteid)
    {
         if (!Transformed)
         {
             Transformed = true;
             DoCast(me, SPELL_TRANSFORM_HUMAN);
             Timer = 120000;
             if (emoteid == TEXTEMOTE_BECKON)
                 me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
             else
             {
                 ChickenTimer = 1500;
                 Chicken = true;
             }
         }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Transformed)
        {
            if (Timer <= diff)
                Reset();
            else Timer-=diff;
        }

       if (Chicken)
       {
           if (ChickenTimer <= diff)
           {
               me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
               Chicken = false;
           } else ChickenTimer-=diff;
       }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
   }
};

bool ReceiveEmote_npc_plucky(Player *player, Creature* pCreature, uint32 emote)
{
    if ((emote == TEXTEMOTE_BECKON || emote == TEXTEMOTE_CHICKEN &&
        player->GetQuestStatus(QUEST_GET_THE_SCOOP) == QUEST_STATUS_INCOMPLETE))
    {
        pCreature->SetInFront(player);
        ((npc_pluckyAI*)CAST_CRE(pCreature)->AI())->TransformHuman(emote);
    }

    return true;
}

bool GossipHello_npc_plucky(Player *player, Creature* pCreature)
{
    if (player->GetQuestStatus(QUEST_GET_THE_SCOOP) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_P), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(738, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_plucky(Player *player, Creature* pCreature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CLOSE_GOSSIP_MENU();
            player->CompleteQuest(QUEST_GET_THE_SCOOP);
        break;
    }
    return true;
}

CreatureAI* GetAI_npc_plucky(Creature* pCreature)
{
return new npc_pluckyAI(pCreature);
}

/*#####
#npc_panter
######*/

enum ePantherCage
{
    ENRAGED_PANTHER = 10992
};

bool go_panther_cage(Player* pPlayer, GameObject* pGo)
{

    if (pPlayer->GetQuestStatus(5151) == QUEST_STATUS_INCOMPLETE)
    {
        if (Creature* panther = GetClosestCreatureWithEntry(pGo, ENRAGED_PANTHER, 5))
        {
            //pGo->DestroyForPlayer(pPlayer);
            panther->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            panther->SetReactState(REACT_AGGRESSIVE);
            panther->AI()->AttackStart(pPlayer);
        }
    }

    return true ;
}

struct npc_enraged_pantherAI : public ScriptedAI
{
    npc_enraged_pantherAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_enraged_panther(Creature* pCreature)
{
    return new npc_enraged_pantherAI(pCreature);
}

/*#####
# npc_zameek
######*/

struct npc_zamekAI : public npc_escortAI
{
    npc_zamekAI(Creature *c) : npc_escortAI(c) {}

    uint32 DestroyTimer;
    Timer ObjectRespawnTimer;

    void Reset()
    {
        DestroyTimer = 0;
        ObjectRespawnTimer.Reset(0);
    }

    void WaypointReached(uint32 i)
    {
        switch (i)
        {
            case 2:
            {
                me->SummonGameObject(182088, -6264.922363, -3844.470703, -58.750019, 1.272766, 0, 0, 0, 0, 3000);

                if (GameObject* goRizzlesGuardedPlans = FindGameObject(179888, 150, me))
                    me->RemoveGameObject(goRizzlesGuardedPlans, true);

                me->SummonGameObject(20805, -6236.64, -3830.48, -58.1364, -0.907571, 0, 0, 0.438371, -0.898794, 30000);

                if(Creature* Rizzle = (Creature*)FindCreature(4720, 100, me))
                    Rizzle->GetMotionMaster()->MovePoint(1, -6252.988281, -3864.691406, -58.749641);

                DestroyTimer = 2000;  
                ObjectRespawnTimer = 30000;
                break;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        
        if(DestroyTimer)
        {
            if(DestroyTimer < diff)
            {
                if(GameObject* explosive = FindGameObject(182088, 20, me))
                    me->RemoveGameObject(explosive, true);
            } else DestroyTimer -= diff;
        }

        if (ObjectRespawnTimer.Expired(diff))
        {
            if (GameObject* goRizzlesUnguardedPlans = FindGameObject(20805, 150, me))
                me->RemoveGameObject(goRizzlesUnguardedPlans, true);

            me->SummonGameObject(179888, -6236.64, -3830.48, -58.1364, -0.907571, 0, 0, 0.438371, -0.898794, 2000);

            if(Creature* Rizzle = (Creature*)FindCreature(4720, 150, me))
                Rizzle->GetMotionMaster()->MoveTargetedHome();
            
            ObjectRespawnTimer = 0;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_zamek(Creature* pCreature)
{
    npc_zamekAI* zamekAI = new npc_zamekAI(pCreature);

    zamekAI->AddWaypoint(0, -6239.451172, -3920.153320, -58.902615);
    zamekAI->AddWaypoint(1, -6259.392578, -3882.922363, -59.392162);
    zamekAI->AddWaypoint(2, -6265.989746, -3847.951416, -58.74958, 3000);
    zamekAI->AddWaypoint(3, -6259.392578, -3882.922363, -59.392162);
    zamekAI->AddWaypoint(4, -6239.451172, -3920.153320, -58.902615);
    zamekAI->AddWaypoint(5, -6226.129883, -3944.939941, -58.625099, 35000);

    return (CreatureAI*)zamekAI;
}

bool QuestComplete_npc_zamek(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 1191)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_zamekAI, creature->AI()))
        {
            pEscortAI->SetDespawnAtFar(false);
            pEscortAI->Start(true, true, player->GetGUID(), quest, true, false);
        }
    }

    return true;
}

void AddSC_thousand_needles()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_kanati";
    newscript->GetAI = &GetAI_npc_kanati;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kanati;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_kanati;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lakota_windsong";
    newscript->GetAI = &GetAI_npc_lakota_windsong;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_lakota_windsong;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_swiftmountain";
    newscript->GetAI = &GetAI_npc_swiftmountain;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_swiftmountain;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_plucky";
    newscript->GetAI = &GetAI_npc_plucky;
    newscript->pReceiveEmote =  &ReceiveEmote_npc_plucky;
    newscript->pGossipHello =   &GossipHello_npc_plucky;
    newscript->pGossipSelect = &GossipSelect_npc_plucky;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_enraged_panther";
    newscript->GetAI = &GetAI_npc_enraged_panther;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_panther_cage";
    newscript->pGOUse = &go_panther_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_zamek";
    newscript->GetAI = &GetAI_npc_zamek;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_zamek;
    newscript->RegisterSelf();
}
