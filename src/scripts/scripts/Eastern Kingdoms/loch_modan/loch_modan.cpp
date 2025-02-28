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
SDName: Loch_Modan
SD%Complete: 100
SDComment: Quest support: 3181
SDCategory: Loch Modan
EndScriptData */

/* ContentData
npc_mountaineer_pebblebitty
npc_miran
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_mountaineer_pebblebitty
######*/

#define GOSSIP_MP  16176

#define GOSSIP_MP1 16177
#define GOSSIP_MP2 16178
#define GOSSIP_MP3 16179
#define GOSSIP_MP4 16180
#define GOSSIP_MP5 16181
#define GOSSIP_MP6 16182

bool GossipHello_npc_mountaineer_pebblebitty(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (!player->GetQuestRewardStatus(3181) == 1)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_MP), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_mountaineer_pebblebitty(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_MP1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(1833, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_MP2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(1834, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_MP3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(1835, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_MP4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            player->SEND_GOSSIP_MENU(1836, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_MP5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            player->SEND_GOSSIP_MENU(1837, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_MP6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            player->SEND_GOSSIP_MENU(1838, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}

/*######
## npc_miran
######*/

#define SAY_ATTACK             -1910203
#define SAY_DONE               -1910204
#define SAY_COMPLETE           -1910205

#define QUEST_PTS              309
#define NPC_DARK_IRON_RAIDER   2149

struct npc_miranAI : public npc_escortAI
{
    npc_miranAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    Timer StartCombatTimer;
    bool battleWithRaiders;

    void Reset()
    {
        battleWithRaiders = false;
        StartCombatTimer.Reset(0);
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if(!player)
            return;

        switch (i)
        {
        case 12:
            m_creature->SummonCreature(NPC_DARK_IRON_RAIDER, -5686.42, -3754.83, 322.21, 1.74, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
            m_creature->SummonCreature(NPC_DARK_IRON_RAIDER, -5699.81, -3753.28, 321.57, 1.45, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
            
            DoScriptText(SAY_ATTACK, m_creature);

            battleWithRaiders = true;
            StartCombatTimer = 2000;
            break;
        case 15:
            DoScriptText(SAY_COMPLETE, m_creature);
            if (player)
            {
                player->GroupEventHappens(QUEST_PTS, m_creature);
            } 
            break;
        }
    }

    void EnterCombat(Unit* who){}

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_DARK_IRON_RAIDER)
        {
            pSummoned->Unmount();
            pSummoned->AI()->AttackStart(m_creature);
        }
    }

    void JustDied(Unit* killer)
    {
        if (Player* player = GetPlayerForEscort())
        {
            if (player->GetQuestStatus(QUEST_PTS) != QUEST_STATUS_COMPLETE)
                player->FailQuest(QUEST_PTS);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (battleWithRaiders)
            {
                if (StartCombatTimer.Expired(diff))
                {
                    DoScriptText(SAY_DONE, m_creature);
                    battleWithRaiders = false;
                    StartCombatTimer = 0;
                }
            }
        }

        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_miran(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_PTS)
    {
		if (npc_escortAI* pEscortAI = CAST_AI(npc_miranAI, creature->AI()))
		{
			pEscortAI->Start(true, false, player->GetGUID(), quest);
		}
    }
    return true;
}

CreatureAI* GetAI_npc_miran(Creature* pCreature)
{
    npc_miranAI* thisAI = new npc_miranAI(pCreature);

    thisAI->AddWaypoint(0, -5764.81, -3433.92, 305.89, 0);
    thisAI->AddWaypoint(1, -5763.25, -3436.67, 306.12, 0);
    thisAI->AddWaypoint(2, -5751.73, -3442.27, 301.98, 0);
    thisAI->AddWaypoint(3, -5738.72, -3477.4, 301.71, 0);
    thisAI->AddWaypoint(4, -5738.68, -3483.66, 302.15, 0);
    thisAI->AddWaypoint(5, -5709.01, -3527.46, 304.71, 0);
    thisAI->AddWaypoint(6, -5707.91, -3541.81, 304.78, 0);
    thisAI->AddWaypoint(7, -5696.07, -3562.45, 307.76, 0);
    thisAI->AddWaypoint(8, -5679.9, -3588.74, 310.94, 0);
    thisAI->AddWaypoint(9, -5672.35, -3602.27, 311.94, 0);
    thisAI->AddWaypoint(10, -5680.43, -3643.35, 314.94, 0);
    thisAI->AddWaypoint(11, -5698.14, -3718.03, 316.05, 0);
    thisAI->AddWaypoint(12, -5697.19, -3735.3, 318.49, 0); //Spawn raider's
    thisAI->AddWaypoint(13, -5696.71, -3758.41, 323.13, 0);
    thisAI->AddWaypoint(14, -5685.43, -3781.09, 323.11, 0);
    thisAI->AddWaypoint(15, -5702.09, -3793.86, 322.69, 30000);

    return (CreatureAI*)thisAI;
}

void AddSC_loch_modan()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_mountaineer_pebblebitty";
    newscript->pGossipHello =  &GossipHello_npc_mountaineer_pebblebitty;
    newscript->pGossipSelect = &GossipSelect_npc_mountaineer_pebblebitty;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name= "npc_miran";
    newscript->GetAI = &GetAI_npc_miran;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_miran;
    newscript->RegisterSelf();
}

