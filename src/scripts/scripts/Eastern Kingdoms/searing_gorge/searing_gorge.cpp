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
SDName: Searing_Gorge
SD%Complete: 80
SDComment: Quest support: 3377, 3441 (More accurate info on Kalaran needed). Lothos Riftwaker teleport to Molten Core.
SDCategory: Searing Gorge
EndScriptData */

/* ContentData
npc_kalaran_windblade
npc_lothos_riftwaker
npc_zamael_lunthistle
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_kalaran_windblade
######*/

bool GossipHello_npc_kalaran_windblade(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(3441) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16188), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_kalaran_windblade(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16189), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(1954, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16190), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(1955, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3441);
            break;
    }
    return true;
}

/*######
## npc_lothos_riftwaker
######*/

bool GossipHello_npc_lothos_riftwaker(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestRewardStatus(7487) || player->GetQuestRewardStatus(7848))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16191), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lothos_riftwaker(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(409, 1096, -467, -104.6, 3.64);
    }

    return true;
}

/*######
## npc_zamael_lunthistle
######*/

bool GossipHello_npc_zamael_lunthistle(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(3377) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16192), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(1920, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_zamael_lunthistle(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16193), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(1921, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16194), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(1922, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3377);
            break;
    }
    return true;
}

/*###########
# npc_dorius_stonetender
############*/

struct npc_dorius_stonetenderAI : public npc_escortAI
{
    npc_dorius_stonetenderAI(Creature *c) : npc_escortAI(c), Summons(me) {}

    SummonList Summons;

    void Reset()
    {
        if(!IsEscorted())
            me->setFaction(FACTION_ESCORT_A_NEUTRAL_PASSIVE);
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (i)
        {
            case 0:
            {
                SetRun(true);
                break;
            }
            case 12:
            {
                me->SummonCreature(5856, -6765.024902, -1800.969360, 256.838715, 2.448910, TEMPSUMMON_TIMED_DESPAWN, 60000);
                me->SummonCreature(5856, -6769.104980, -1808.168457, 256.858612, 2.683745, TEMPSUMMON_TIMED_DESPAWN, 60000);
                break;
            }
            case 23:
            {
                me->SummonGameObject(175704, -6386.890137, -1984.050049, 246.729996, 0, 0, 0, 0, 0, 60000);
                me->SummonCreature(8338, -6369.611816, -1975.034912, 256.752808, 3.607368, TEMPSUMMON_TIMED_DESPAWN, 60000);
                me->Kill(me);
                if (pPlayer)
                    pPlayer->GroupEventHappens(3367, m_creature);
                break;
            }
        }
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned)
            summoned->AI()->AttackStart(m_creature);
        Summons.Summon(summoned);
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_dorius_stonetender(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (pPlayer && quest->GetQuestId() == 3367)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_dorius_stonetenderAI, pCreature->AI()))
        {
            pEscortAI->Start(true, true, pPlayer->GetGUID(), quest);
            pCreature->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);
            pCreature->setFaction(FACTION_ESCORT_A_NEUTRAL_ACTIVE);
        }
    }

    return true;
}

CreatureAI* GetAI_npc_dorius_stonetender(Creature *_Creature)
{
   npc_dorius_stonetenderAI* thisAI = new npc_dorius_stonetenderAI(_Creature);

    thisAI->AddWaypoint(0, -7005.298340, -1725.037476, 234.099106);
    thisAI->AddWaypoint(1, -6991.727051, -1735.322754, 239.646378);
    thisAI->AddWaypoint(2, -6954.391602, -1741.041626, 241.679398);
    thisAI->AddWaypoint(3, -6930.933105, -1716.848633, 242.207489);
    thisAI->AddWaypoint(4, -6899.807617, -1706.367920, 240.418335);
    thisAI->AddWaypoint(5, -6894.433105, -1697.416138, 241.864456);
    thisAI->AddWaypoint(6, -6866.509277, -1694.481934, 248.293289);
    thisAI->AddWaypoint(7, -6812.801758, -1675.378906, 249.985367);
    thisAI->AddWaypoint(8, -6808.298828, -1683.379761, 250.806259);
    thisAI->AddWaypoint(9, -6799.883789, -1696.689331, 259.141266);
    thisAI->AddWaypoint(10, -6778.567383, -1717.242432, 259.550873);
    thisAI->AddWaypoint(11, -6778.197754, -1753.213501, 259.117462);
    thisAI->AddWaypoint(12, -6771.151855, -1794.899414, 256.858459); // CP: Summon Spiders
    thisAI->AddWaypoint(13, -6725.953125, -1817.119751, 253.11084);
    thisAI->AddWaypoint(14, -6687.487793, -1817.744019, 249.614487);
    thisAI->AddWaypoint(15, -6675.508789, -1836.225464, 248.181778);
    thisAI->AddWaypoint(16, -6663.244629, -1870.626587, 244.463196);
    thisAI->AddWaypoint(17, -6641.041016, -1917.315430, 244.149216);
    thisAI->AddWaypoint(18, -6605.729004, -1917.783813, 244.149506);
    thisAI->AddWaypoint(19, -6594.731445, -1920.195923, 244.149506);
    thisAI->AddWaypoint(20, -6568.595703, -1922.639893, 244.175095);
    thisAI->AddWaypoint(21, -6476.798340, -1971.757446, 244.157242);
    thisAI->AddWaypoint(22, -6401.779297, -1987.596558, 246.80246);
    thisAI->AddWaypoint(23, -6388.545898, -1984.522949, 246.738174);

    return (CreatureAI*)thisAI;
}

/*###
# npc_dorius
###*/

enum quest3566
{
    EVENT_3566_1    = 1,
    EVENT_3566_2    = 2,
    EVENT_3566_3    = 3,
    EVENT_3566_4    = 4,
    EVENT_3566_5    = 5,
    EVENT_3566_6    = 6,
    EVENT_3566_7    = 7,
    EVENT_3566_8    = 8
};

struct npc_doriusAI : public ScriptedAI
{
    npc_doriusAI(Creature *c) : ScriptedAI(c) { }

    EventMap events;

    void Reset()
    {
        events.Reset();
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);
        events.ScheduleEvent(EVENT_3566_1, 1000);
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_3566_1:
                {
                    
                    DoScriptText(-1230051, me, 0);
                    events.ScheduleEvent(EVENT_3566_2, 3000);
                    break;
                }
                case EVENT_3566_2:
                {
                    
                    DoScriptText(-1230052, me, 0);
                    events.ScheduleEvent(EVENT_3566_3, 5000);
                    break;
                }
                case EVENT_3566_3:
                {
                    
                    DoScriptText(-1230053, me, 0);
                    events.ScheduleEvent(EVENT_3566_4, 6000);
                    break;
                }
                case EVENT_3566_4:
                {
                    
                    DoScriptText(-1230054, me, 0);
                    events.ScheduleEvent(EVENT_3566_5, 7000);
                    break;
                }
                case EVENT_3566_5:
                {
                    
                    DoScriptText(-1230055, me, 0);
                    events.ScheduleEvent(EVENT_3566_6, 8000);
                    break;
                }
                case EVENT_3566_6:
                {
                    
                    DoScriptText(-1230056, me, 0);
                    events.ScheduleEvent(EVENT_3566_7, 5000);
                    break;
                }
                case EVENT_3566_7:
                {
                    me->UpdateEntry(8391);
                    DoScriptText(-1230057, me, 0);
                    events.ScheduleEvent(EVENT_3566_8, 5000);
                    break;
                }
                case EVENT_3566_8:
                {
                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM,0);
                    DoScriptText(-1230058, me, 0);
                    me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
                    if(Creature* Obsidian = GetClosestCreatureWithEntry(me, 8400, 30))
                    {
                        Obsidian->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
                        Obsidian->SetStandState(UNIT_STAND_STATE_STAND);
                        Obsidian->AI()->AttackStart(target);
                    }
                    me->AI()->AttackStart(target);
                    break;
                }
            }
        }
        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_dorius(Creature *_Creature)
{
    return new npc_doriusAI(_Creature);
}

/*###
# npc_dying_archeologist
###*/

bool QuestAccept_npc_dying_archeologist(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (pPlayer && quest->GetQuestId() == 3566)
    {
        Creature* Dorius = GetClosestCreatureWithEntry(pCreature, 8421, 30);
        if(!Dorius)
            pCreature->SummonCreature(8421, -6469.004883, -1247.15271, 180.320908, 2.632376, TEMPSUMMON_TIMED_DESPAWN, 360000);
    }
    return true;
}

void AddSC_searing_gorge()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_kalaran_windblade";
    newscript->pGossipHello =  &GossipHello_npc_kalaran_windblade;
    newscript->pGossipSelect = &GossipSelect_npc_kalaran_windblade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_lothos_riftwaker";
    newscript->pGossipHello          = &GossipHello_npc_lothos_riftwaker;
    newscript->pGossipSelect         = &GossipSelect_npc_lothos_riftwaker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_zamael_lunthistle";
    newscript->pGossipHello =  &GossipHello_npc_zamael_lunthistle;
    newscript->pGossipSelect = &GossipSelect_npc_zamael_lunthistle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dorius_stonetender";
    newscript->GetAI = &GetAI_npc_dorius_stonetender;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_dorius_stonetender;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dorius";
    newscript->GetAI = &GetAI_npc_dorius;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dying_archeologist";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_dying_archeologist;
    newscript->RegisterSelf();
}

