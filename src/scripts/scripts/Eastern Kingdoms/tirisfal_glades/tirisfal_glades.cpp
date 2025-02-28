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
SDName: Tirisfal_Glades
SD%Complete: 100
SDComment: Quest support: 590, 1819
SDCategory: Tirisfal Glades
EndScriptData */

/* ContentData
npc_calvin_montague
go_mausoleum_door
go_mausoleum_trigger
EndContentData */

#include "precompiled.h"

/*######
## npc_calvin_montague
######*/

#define QUEST_590           590
#define FACTION_FRIENDLY    68
#define FACTION_HOSTILE     16

struct npc_calvin_montagueAI : public ScriptedAI
{
    npc_calvin_montagueAI(Creature* c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->setFaction(FACTION_FRIENDLY);
        m_creature->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2);
    }

    void EnterCombat(Unit* who) { }

    void JustDied(Unit* Killer)
    {
        if( Killer->GetTypeId() == TYPEID_PLAYER )
            if( ((Player*)Killer)->GetQuestStatus(QUEST_590) == QUEST_STATUS_INCOMPLETE )
                ((Player*)Killer)->AreaExploredOrEventHappens(QUEST_590);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_calvin_montague(Creature *_Creature)
{
    return new npc_calvin_montagueAI (_Creature);
}

bool QuestAccept_npc_calvin_montague(Player* player, Creature* creature, Quest const* quest)
{
    if( quest->GetQuestId() == QUEST_590 )
    {
        creature->setFaction(FACTION_HOSTILE);
        creature->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
        ((npc_calvin_montagueAI*)creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## go_mausoleum_door
## go_mausoleum_trigger
######*/

#define QUEST_ULAG      1819
#define C_ULAG          6390
#define GO_TRIGGER      104593
#define GO_DOOR         176594

GameObject* SearchMausoleumGo(Unit *source, uint32 entry, float range)
{
    GameObject* pGo = NULL;

    Hellground::NearestGameObjectEntryInObjectRangeCheck go_check(*source, entry, range);
    Hellground::ObjectLastSearcher<GameObject, Hellground::NearestGameObjectEntryInObjectRangeCheck> searcher(pGo, go_check);

    Cell::VisitGridObjects(source, searcher, range);

    return pGo;
}

bool GOUse_go_mausoleum_door(Player *player, GameObject* _GO)
{
    if (player->GetQuestStatus(QUEST_ULAG) != QUEST_STATUS_INCOMPLETE)
        return false;

    if (GameObject *trigger = SearchMausoleumGo(player, GO_TRIGGER, 30))
    {
        trigger->SetGoState(GO_STATE_READY);
        player->SummonCreature(C_ULAG, 2390.26, 336.47, 40.01, 2.26, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
        return false;
    }

    return false;
}

bool GOUse_go_mausoleum_trigger(Player *player, GameObject* _GO)
{
    if (player->GetQuestStatus(QUEST_ULAG) != QUEST_STATUS_INCOMPLETE)
        return false;

    if (GameObject *door = SearchMausoleumGo(player, GO_DOOR, 30))
    {
        _GO->SetGoState(GO_STATE_ACTIVE);
        door->RemoveFlag(GAMEOBJECT_FLAGS,GO_FLAG_INTERACT_COND);
        return true;
    }

    return false;
}

struct npc_2211AI : public ScriptedAI
{
    npc_2211AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer DieTimer;

    void Reset()
    {
        DieTimer.Reset(0);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void UpdateAI(const uint32 diff)
    {
        if(DieTimer.Expired(diff))
        {
            me->Kill(me);
            DieTimer = 0;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_2211(Creature* creature)
{
    return new npc_2211AI(creature);
}

bool QuestComplete_npc_2211(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 492)
    {
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->HandleEmoteCommand(7);
        ((npc_2211AI*)creature->AI())->DieTimer = 2000;
    }
    return true;
}


struct npc_1931AI : public ScriptedAI
{
    npc_1931AI(Creature *c) : ScriptedAI(c)
    {
    }

    Timer MoveTimer;

    void Reset()
    {
        MoveTimer.Reset(0);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        me->RemoveAurasDueToSpell(3287);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        switch(PointId)
        {
            case 1:
                me->GetMotionMaster()->MovePoint(2, 2287.97, 236.253, 27.0892);
                break;
            case 2:
                DoScriptText(-1230050, me, 0);
                me->GetMotionMaster()->MovePoint(3, 2292.52, 235.226, 27.0892);
                break;
            case 3:
                me->GetMotionMaster()->MovePoint(4, 2288.96, 237.96 , 27.0892);
            case 4:
                me->CastSpell(me, 5, false);
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(MoveTimer.Expired(diff))
        {
            me->CastSpell(me, 3287, false);
            me->GetMotionMaster()->MovePoint(1, 2292, 239.481, 27.0892);
            MoveTimer = 0;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_1931(Creature* creature)
{
    return new npc_1931AI(creature);
}

bool QuestComplete_npc_1931(Player *player, Creature* creature, const Quest* quest)
{
    if(quest->GetQuestId() == 407)
    {
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        DoScriptText(-1230059, creature, 0);
        ((npc_1931AI*)creature->AI())->MoveTimer = 1000;
    }
    return true;
}

void AddSC_tirisfal_glades()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_calvin_montague";
    newscript->GetAI = &GetAI_npc_calvin_montague;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_calvin_montague;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_mausoleum_door";
    newscript->pGOUse = &GOUse_go_mausoleum_door;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_mausoleum_trigger";
    newscript->pGOUse = &GOUse_go_mausoleum_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_2211";
    newscript->GetAI = &GetAI_npc_2211;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_2211;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_1931";
    newscript->GetAI = &GetAI_npc_1931;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_1931;
    newscript->RegisterSelf();
}

