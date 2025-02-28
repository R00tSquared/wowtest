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
SDName: Zulfarrak
SD%Complete: 70
SDComment: TODO: gossips ; make blastfuse blast the doors with spell
SDCategory: Zul'Farrak
EndScriptData */

/* ContentData
npc_sergeant_bly
npc_weegli_blastfuse
EndContentData */

#include "precompiled.h"
#include "def_zul_farrak.h"

/*######
## npc_sergeant_bly
######*/

#define FACTION_HOSTILE             14
#define FACTION_FRIENDLY            250

#define SPELL_SHIELD_BASH           11972
#define SPELL_REVENGE               12170

#define GOSSIP_BLY                  16403

struct npc_sergeant_blyAI : public ScriptedAI
{
    npc_sergeant_blyAI(Creature *c) : ScriptedAI(c)
    {
        me->SetReactState(REACT_PASSIVE);
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;
    Timer ShieldBash_Timer;
    Timer Revenge_Timer;                                   //this is wrong, spell should never be used unless me->GetVictim() dodge, parry or block attack. Trinity support required.
    Timer EventTimer;
    Timer HearthstoneTimer;
    uint8 Phase;

    void Reset()
    {
        ShieldBash_Timer.Reset(5000);
        Revenge_Timer.Reset(8000);
        EventTimer.Reset(0);
        HearthstoneTimer.Reset(0);
        Phase = 0;

        me->setFaction(FACTION_FRIENDLY);
    }

    void EnterCombat(Unit *who)
    {
        if(who->GetTypeId() == TYPEID_UNIT && Phase == 0)
        {
            if(Unit* support1 = FindCreature(7605, 20, me))
                ((Creature*)support1)->AI()->AttackStart(who);

            if(Unit* support2 = FindCreature(7606, 20, me))
                ((Creature*)support2)->AI()->AttackStart(who);

            if(Unit* support3 = FindCreature(7608, 20, me))
                ((Creature*)support3)->AI()->AttackStart(who);

            if(Unit* support4 = FindCreature(7607, 20, me))
               ((Creature*)support4)->AI()->AttackStart(who);
        }
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (spell->Id == 8690)
            me->DisappearAndDie();
    }

    void DoMeleeAttackIfReady()
    {
        if (me->HasUnitState(UNIT_STAT_CASTING))
            return;

        //Make sure our attack is ready and we aren't currently casting before checking distance
        if (me->isAttackReady())
        {
            if(!me->GetVictim())
                return;

            //If we are within range melee the target
            if (me->IsWithinMeleeRange(me->GetVictim()))
            {
                me->AttackerStateUpdate(me->GetVictim());
                me->resetAttackTimer();

                if(Unit* support1 = FindCreature(7605, 20, me))
                    ((Creature*)support1)->AI()->AttackStart(me->GetVictim());

                if(Unit* support2 = FindCreature(7606, 20, me))
                    ((Creature*)support2)->AI()->AttackStart(me->GetVictim());

                if(Unit* support3 = FindCreature(7608, 20, me))
                    ((Creature*)support3)->AI()->AttackStart(me->GetVictim());

                if(Phase == 0)
                {
                    if(Unit* support4 = FindCreature(7607, 20, me))
                        ((Creature*)support4)->AI()->AttackStart(me->GetVictim());
                }
            }
        }
    }

    void JustDied(Unit *victim) {}

    void MovementInform(uint32 motiontype, uint32 wpid)
    {
        if (motiontype != POINT_MOTION_TYPE)
            return;

        if(wpid == 0)
        {
            me->SetReactState(REACT_AGGRESSIVE);
        }

        if (wpid == 2)
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            HearthstoneTimer = 120000;
            if (pInstance)
                pInstance->SetData(DATA_PYRAMID_BATTLE, DONE);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventTimer.Expired(diff))
        {
            switch(Phase)
            {
                case 0:
                {
                    me->Say(-1200447, LANG_UNIVERSAL, 0);
                    Phase++;
                    EventTimer = 4000;
                    break;
                }
                case 1:
                {
                    me->Say(-1200448, LANG_UNIVERSAL, 0);
                    Phase++;
                    EventTimer = 6000;
                }
                case 2:
                {
                    if(Unit* support4 = FindCreature(7607, 20, me))
                    {
                        ((Creature*)support4)->Say(-1200449, LANG_UNIVERSAL, 0);
                        ((Creature*)support4)->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        me->SetWalk(false);
                        ((Creature*)support4)->GetMotionMaster()->MovePoint(3, 1856.92, 1146.26, 15.15);
                    }
                    me->setFaction(FACTION_HOSTILE);
                    me->AI()->AttackStart(me->SelectNearestTarget(15));
                    if(Unit* support1 = FindCreature(7605, 20, me))
                    {
                        ((Creature*)support1)->setFaction(FACTION_HOSTILE);
                        ((Creature*)support1)->AI()->AttackStart(me->SelectNearestTarget(15));
                    }
                    if(Unit* support2 = FindCreature(7606, 20, me))
                    {
                        ((Creature*)support2)->setFaction(FACTION_HOSTILE);
                        ((Creature*)support2)->AI()->AttackStart(me->SelectNearestTarget(15));
                    }
                    if(Unit* support3 = FindCreature(7608, 20, me))
                    {
                        ((Creature*)support3)->setFaction(FACTION_HOSTILE);
                        ((Creature*)support3)->AI()->AttackStart(me->SelectNearestTarget(15));
                    }
                    Phase = 0;
                    EventTimer = 0;
                    break;
                }
            }
        }

        if(!UpdateVictim())
        {
            if(HearthstoneTimer.Expired(diff))
            {
                me->CastSpell(me, 8690, false);
                if(Unit* support1 = FindCreature(7605, 20, me))
                {
                    ((Creature*)support1)->CastSpell(support1, 8690, false);
                    ((Creature*)support1)->ForcedDespawn(10000);
                }

                if(Unit* support2 = FindCreature(7606, 20, me))
                {
                    ((Creature*)support2)->CastSpell(support2, 8690, false);
                    ((Creature*)support2)->ForcedDespawn(10000);
                }

                if(Unit* support3 = FindCreature(7608, 20, me))
                {
                    ((Creature*)support3)->CastSpell(support3, 8690, false);
                    ((Creature*)support3)->ForcedDespawn(10000);
                }

                if(Unit* support4 = FindCreature(7607, 20, me))
                {
                    ((Creature*)support4)->Say(-1200450, LANG_UNIVERSAL, 0);
                    ((Creature*)support4)->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    me->SetWalk(false);
                    ((Creature*)support4)->GetMotionMaster()->MovePoint(3, 1856.92, 1146.26, 15.15);
                }

                HearthstoneTimer = 0;
            }
            return;
        }

        if(ShieldBash_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_SHIELD_BASH);
            ShieldBash_Timer = 15000;
        }

        if (Revenge_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_REVENGE);
            Revenge_Timer = 10000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_sergeant_bly(Creature *_Creature)
{
    return new npc_sergeant_blyAI (_Creature);
}

bool GossipHello_npc_sergeant_bly(Player *player, Creature *_Creature )
{
    ScriptedInstance* pInstance = (ScriptedInstance*)_Creature->GetInstanceData();
    if(!pInstance)
        return false;

    if(pInstance->GetData(DATA_PYRAMID_BATTLE) == DONE)
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_BLY), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(1517, _Creature->GetGUID());
    }
    else if(pInstance->GetData(DATA_PYRAMID_BATTLE) == IN_PROGRESS)
        player->SEND_GOSSIP_MENU(1516, _Creature->GetGUID());
    else
        player->SEND_GOSSIP_MENU(1515, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_sergeant_bly(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->InterruptNonMeleeSpells(true);
        ((npc_sergeant_blyAI*)_Creature->AI())->EventTimer = 1000;
        _Creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }
    return true;
}

/*######
## npc_weegli_blastfuse
######*/

#define SPELL_BOMB                  8858
#define SPELL_GOBLIN_LAND_MINE      21688
#define SPELL_SHOOT                 6660
#define SPELL_WEEGLIS_BARREL        10772 // this one should open door

struct npc_weegli_blastfuseAI : public ScriptedAI
{
    npc_weegli_blastfuseAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    void Reset() {}

    void EnterCombat(Unit *who) {}

    void MovementInform(uint32 motiontype, uint32 wpid)
    {
        if (motiontype != POINT_MOTION_TYPE)
            return;

        if(wpid == 0)
        {
            me->GetMotionMaster()->MovePoint(1, 1887.35, 1263.67, 41.48);
        }
        if(wpid == 1)
        {
            me->Say(-1200451, LANG_UNIVERSAL, 0);
            me->GetMotionMaster()->MovePoint(100, 1890.72, 1268.39, 41.47);
        }
        if (wpid == 2)
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
        if (wpid == 3)
        {
            if (GameObject* door = FindGameObject(DOOR_ENTRY, 15, me))
                door->UseDoorOrButton();

            if(pInstance)
                pInstance->SetData(DATA_DOOR_EVENT, DONE);

            me->GetMotionMaster()->MovePoint(4, 1831.53, 1187.87, 8.98);
        }
        if (wpid == 4)
            me->DisappearAndDie();
    }

    void JustDied(Unit *victim)
    {
        if (pInstance)
            pInstance->SetData(DATA_DOOR_EVENT, FAIL);
    }

    void UpdateAI(const uint32 diff)
    {
        if( !UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_weegli_blastfuse(Creature *_Creature)
{
    return new npc_weegli_blastfuseAI (_Creature);
}

bool GossipHello_npc_weegli_blastfuse(Player *player, Creature *_Creature )
{
    ScriptedInstance* pInstance = (ScriptedInstance*)_Creature->GetInstanceData();
    if(!pInstance)
        return false;

    if(pInstance->GetData(DATA_PYRAMID_BATTLE) == DONE)
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16404), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(1514, _Creature->GetGUID());
    }
    else if(pInstance->GetData(DATA_PYRAMID_BATTLE) == IN_PROGRESS)
        player->SEND_GOSSIP_MENU(1513, _Creature->GetGUID());
    else
        player->SEND_GOSSIP_MENU(1511, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_weegli_blastfuse(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->Say(-1200452, LANG_UNIVERSAL, 0);
        _Creature->GetMotionMaster()->MovePoint(3, 1856.92, 1146.26, 15.15);
        _Creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }
    return true;
}

void AddSC_zulfarrak()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_sergeant_bly";
    newscript->GetAI = &GetAI_npc_sergeant_bly;
    newscript->pGossipHello =  &GossipHello_npc_sergeant_bly;
    newscript->pGossipSelect = &GossipSelect_npc_sergeant_bly;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_weegli_blastfuse";
    newscript->GetAI = &GetAI_npc_weegli_blastfuse;
    newscript->pGossipHello =  &GossipHello_npc_weegli_blastfuse;
    newscript->RegisterSelf();
}

