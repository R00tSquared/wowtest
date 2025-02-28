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
SDName: Stormwind_City
SD%Complete: 100
SDComment: Quest support: 1640, 1447, 4185, 11223. Receive emote General Marcus
SDCategory: Stormwind City
EndScriptData */

/* ContentData
npc_archmage_malin
npc_bartleby
npc_dashel_stonefist
npc_general_marcus_jonathan
npc_lady_katrana_prestor
npc_doc_mixilpixil
npc_duthorian_rall
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_archmage_malin
######*/

#define GOSSIP_ITEM_MALIN 16001

bool GossipHello_npc_archmage_malin(Player *player, Creature *_Creature)
{
    if(_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(player->GetQuestStatus(11223) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(11223))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16001), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_archmage_malin(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if(action = GOSSIP_ACTION_INFO_DEF)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 42711, true);
    }

    return true;
}

/*######
## npc_bartleby
######*/

struct npc_bartlebyAI : public ScriptedAI
{
    npc_bartlebyAI(Creature *c) : ScriptedAI(c) {}

    uint64 PlayerGUID;

    void Reset()
    {
        m_creature->setFaction(11);
        m_creature->HandleEmoteCommand(7);

        PlayerGUID = 0;
    }

    void JustDied(Unit *who)
    {
        m_creature->setFaction(11);
    }

    void DamageTaken(Unit *done_by, uint32 & damage)
    {
        if(damage > m_creature->GetHealth() || ((m_creature->GetHealth() - damage)*100 / m_creature->GetMaxHealth() < 15))
        {
            //Take 0 damage
            damage = 0;

            if (done_by->GetTypeId() == TYPEID_PLAYER && done_by->GetGUID() == PlayerGUID)
            {
                ((Player*)done_by)->AttackStop();
                ((Player*)done_by)->AreaExploredOrEventHappens(1640);
            }
            m_creature->CombatStop();
            EnterEvadeMode();
        }
    }

    void EnterCombat(Unit *who) {}
};

bool QuestAccept_npc_bartleby(Player *player, Creature *_Creature, Quest const *_Quest)
{
    if(_Quest->GetQuestId() == 1640)
    {
        _Creature->setFaction(168);
        ((npc_bartlebyAI*)_Creature->AI())->PlayerGUID = player->GetGUID();
        ((npc_bartlebyAI*)_Creature->AI())->AttackStart(player);
    }
    return true;
}

CreatureAI* GetAI_npc_bartleby(Creature *_creature)
{
    return new npc_bartlebyAI(_creature);
}

/*######
## npc_dashel_stonefist
######*/

struct npc_dashel_stonefistAI : public ScriptedAI
{
    npc_dashel_stonefistAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->setFaction(11);
        m_creature->HandleEmoteCommand(7);
    }

    void DamageTaken(Unit *done_by, uint32 & damage)
    {
        if((damage > m_creature->GetHealth()) || (m_creature->GetHealth() - damage)*100 / m_creature->GetMaxHealth() < 15)
        {
            //Take 0 damage
            damage = 0;

            if (done_by->GetTypeId() == TYPEID_PLAYER)
            {
                ((Player*)done_by)->AttackStop();
                ((Player*)done_by)->AreaExploredOrEventHappens(1447);
            }
            //m_creature->CombatStop();
            EnterEvadeMode();
        }
    }

    void EnterCombat(Unit *who) {}
};

bool QuestAccept_npc_dashel_stonefist(Player *player, Creature *_Creature, Quest const *_Quest)
{
    if(_Quest->GetQuestId() == 1447)
    {
        _Creature->setFaction(168);
        ((npc_dashel_stonefistAI*)_Creature->AI())->AttackStart(player);
    }
    return true;
}

CreatureAI* GetAI_npc_dashel_stonefist(Creature *_creature)
{
    return new npc_dashel_stonefistAI(_creature);
}

/*######
## npc_general_marcus_jonathan
######*/

bool ReceiveEmote_npc_general_marcus_jonathan(Player *player, Creature *_Creature, uint32 emote)
{
    if(player->GetTeam() == ALLIANCE)
    {
        if (emote == TEXTEMOTE_SALUTE)
        {
            _Creature->SetOrientation(_Creature->GetOrientationTo(player));
            _Creature->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
        }
        if (emote == TEXTEMOTE_WAVE)
        {
            _Creature->MonsterSay(-1200043,LANG_COMMON,0);
        }
    }
    return true;
}

/*######
## npc_lady_katrana_prestor
######*/

#define GOSSIP_ITEM_KAT_1 16002
#define GOSSIP_ITEM_KAT_2 16003
#define GOSSIP_ITEM_KAT_3 16004
#define GOSSIP_ITEM_KAT_4 16005

bool GossipHello_npc_lady_katrana_prestor(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(4185) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16002), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(2693, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lady_katrana_prestor(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16003), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(2694, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16004), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(2695, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16005), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(2696, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(4185);
            break;
    }
    return true;
}

/*######
## npc_highlord_bolvar_fordragon - TODO: should also have own scripted AI when horde attacks him
######*/

#define NPC_LADY_KATRANA_PRESTOR    1749

struct npc_highlord_bolvar_fordragonAI : public ScriptedAI
{
    npc_highlord_bolvar_fordragonAI(Creature *c) : ScriptedAI(c) {}

    Timer speechTimer;
    uint8 step;

    void Reset()
    {
        speechTimer.Reset(0);
        step = 0;
    }

    void WarnBolvar_questEpilog()
    {
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        speechTimer.Reset(1000);
        step = 1;
    }

    uint32 DoSpeech(uint8 step)
    {
        switch(step)
        {
            case 1:
                me->Say(-1200044, 0, 0);
                return 5000;
            case 2:
                me->Say(-1200045, 0, 0);
                return 7000;
            case 3:
                me->Say(-1200046, 0, 0);
                return 3500;
            case 4:
                if(Unit* Prestor = FindCreature(NPC_LADY_KATRANA_PRESTOR, 10.0, me))
                    Prestor->ToCreature()->Say(-1200047, 0, 0);
                return 4500;
            case 5:
                if(Unit* Prestor = FindCreature(NPC_LADY_KATRANA_PRESTOR, 10.0, me))
                    Prestor->ToCreature()->Say(-1200048, 0, 0);
                return 6000;
            case 6:
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                return 1;
            default:
                return 1;
        }
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if (speechTimer.Expired(diff))
        {
            speechTimer = DoSpeech(step);
            step++;
        }


        if(!UpdateVictim())
            return;

        /*
        Some AI TODO here
        */

        DoMeleeAttackIfReady();
    }
};

bool QuestComplete_npc_highlord_bolvar_fordragon(Player *player, Creature *_Creature, const Quest *_Quest)
{
    if( _Quest->GetQuestId() == 11222 ) // Warn Bolvar!
        ((npc_highlord_bolvar_fordragonAI*)_Creature->AI())->WarnBolvar_questEpilog();

    return true;
}

CreatureAI* GetAI_npc_highlord_bolvar_fordragon(Creature *_creature)
{
    return new npc_highlord_bolvar_fordragonAI(_creature);
}

/*######
## npc_marzon_silent_blade
######*/

enum eLordGregorLescovar
{
    SAY_LESCOVAR_2 = -1000457,
    SAY_GUARD_2    = -1000458,
    SAY_LESCOVAR_3 = -1000459,
    SAY_MARZON_1   = -1000460,
    SAY_LESCOVAR_4 = -1000461,
    SAY_TYRION_2   = -1000462,
    SAY_LESCOVAR_5 = -1000463,
    SAY_MARZON_2   = -1000464,

    NPC_STORMWIND_ROYAL      = 1756,
    NPC_MARZON_BLADE         = 1755,
    NPC_LORD_GREGOR_LESCOVAR = 1754,
    NPC_TYRION               = 7766,
    NPC_TYRION_SPYBOT        = 8856,

    QUEST_THE_ATTACK    = 434
};

/*######
## npc_lord_gregor_lescovar
######*/

struct npc_lord_gregor_lescovarAI : public npc_escortAI
{
    npc_lord_gregor_lescovarAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        pCreature->RestoreFaction();
    }

    Timer uiTimer;
    uint32 uiPhase;

    uint64 MarzonGUID;

    void Reset()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        uiTimer.Reset(0);
        uiPhase = 0;

        MarzonGUID = 0;
    }

    void EnterEvadeMode()
    {
        me->DisappearAndDie();

        if (Creature *pMarzon = Unit::GetCreature(*me, MarzonGUID))
        {
            if (pMarzon->isAlive())
                pMarzon->DisappearAndDie();
        }
    }

    void EnterCombat(Unit* pWho)
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        DoScriptText(SAY_LESCOVAR_5, me);
        if (Creature *pMarzon = Unit::GetCreature(*me, MarzonGUID))
        {
            if (pMarzon->isAlive() && !pMarzon->IsInCombat())
                pMarzon->AI()->AttackStart(pWho);
        }
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 1:
                me->setFaction(35);
                break;
            case 14:
                SetEscortPaused(true);
                DoScriptText(SAY_LESCOVAR_2, me);
                uiTimer.Reset(3000);
                uiPhase = 1;
                break;
            case 16:
                SetEscortPaused(true);
                if (Creature *pMarzon = me->SummonCreature(NPC_MARZON_BLADE,-8411.360352, 480.069733, 123.760895, 4.941504, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
                {
                    pMarzon->GetMotionMaster()->MovePoint(0,-8408.000977, 468.611450, 123.759903);
                    MarzonGUID = pMarzon->GetGUID();
                }
                uiTimer.Reset(2000);
                uiPhase = 4;
                break;
        }
    }
    //TO-DO: We don't have movemaps, also we can't make 2 npcs walks to one point propperly (and we can not use escort ai, because they are 2 different spawns and with same entry), because of it we make them, disappear.
    void DoGuardsDisappearAndDie()
    {
        std::list<Creature*> GuardList;
        GuardList = FindAllCreaturesWithEntry(NPC_STORMWIND_ROYAL,80.0f);
        if (!GuardList.empty())
        {
            for (std::list<Creature*>::const_iterator itr = GuardList.begin(); itr != GuardList.end(); ++itr)
            {
                if (Creature* pGuard = *itr)
                    pGuard->DisappearAndDie();
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (uiPhase)
        {
            if (uiTimer.Expired(uiDiff))
            {
                switch(uiPhase)
                {
                    case 1:
                        if (Creature* pGuard = GetClosestCreatureWithEntry(me, NPC_STORMWIND_ROYAL, 30.0f))
                            DoScriptText(SAY_GUARD_2, pGuard);
                        DoGuardsDisappearAndDie();
                        uiTimer = 3000;
                        uiPhase = 2;
                        break;
                    case 2:
                        me->RestoreFaction();
                        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                        uiTimer = 2000;
                        uiPhase = 3;
                        break;
                    case 3:
                        SetEscortPaused(false);
                        uiTimer = 0;
                        uiPhase = 0;
                        break;
                    case 4:
                        DoScriptText(SAY_LESCOVAR_3, me);
                        uiTimer = 5000;
                        uiPhase = 5;
                        break;
                    case 5:
                        if (Creature *pMarzon = Unit::GetCreature(*me, MarzonGUID))
                            DoScriptText(SAY_MARZON_1, pMarzon);
                        uiTimer = 3000;
                        uiPhase = 6;
                        break;
                    case 6:
                        DoScriptText(SAY_LESCOVAR_4, me);
                        uiTimer = 2000;
                        uiPhase = 7;
                        break;
                    case 7:
                        if (Creature* pTyrion = GetClosestCreatureWithEntry(me, NPC_TYRION, 20.0f))
                            DoScriptText(SAY_TYRION_2, pTyrion);
                        if (Creature *pMarzon = Unit::GetCreature(*me, MarzonGUID))
                            pMarzon->setFaction(14);
                        me->setFaction(14);
                        uiTimer = 0;
                        uiPhase = 0;
                        break;
                }
            } 
        }
        npc_escortAI::UpdateAI(uiDiff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_lord_gregor_lescovar(Creature* pCreature)
{
    return new npc_lord_gregor_lescovarAI(pCreature);
}

struct npc_marzon_silent_bladeAI : public ScriptedAI
{
    npc_marzon_silent_bladeAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        me->SetWalk(true);
    }

    void Reset()
    {
        me->RestoreFaction();
    }

    void EnterCombat(Unit* pWho)
    {
        DoScriptText(SAY_MARZON_2, me);

        if (Creature* pLord = GetClosestCreatureWithEntry(me, NPC_LORD_GREGOR_LESCOVAR, 30.0f))
        {
            if (pLord && pLord->isAlive() && !pLord->IsInCombat())
                pLord->AI()->AttackStart(pWho);
        }
    }

    void EnterEvadeMode()
    {
        me->DisappearAndDie();

        if (Creature* pLord = GetClosestCreatureWithEntry(me, NPC_LORD_GREGOR_LESCOVAR, 30.0f))
        {
            if (pLord && pLord->isAlive())
                pLord->DisappearAndDie();
        }
    }

    void UpdateAI(const uint32 /*diff*/)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_marzon_silent_blade(Creature* pCreature)
{
    return new npc_marzon_silent_bladeAI(pCreature);
}

/*######
## npc_tyrion_spybot
######*/

enum eTyrionSpybot
{
    SAY_QUEST_ACCEPT_ATTACK  = -1000499,
    SAY_TYRION_1             = -1000450,
    SAY_SPYBOT_1             = -1000451,
    SAY_GUARD_1              = -1000452,
    SAY_SPYBOT_2             = -1000453,
    SAY_SPYBOT_3             = -1000454,
    SAY_LESCOVAR_1           = -1000455,
    SAY_SPYBOT_4             = -1000456,

    NPC_PRIESTESS_TYRIONA    = 7779,
};

struct npc_tyrion_spybotAI : public npc_escortAI
{
    npc_tyrion_spybotAI(Creature* pCreature) : npc_escortAI(pCreature) {}

    uint32 uiTimer;
    uint32 uiPhase;

    void Reset()
    {
        uiTimer = 0;
        uiPhase = 0;
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 1:
                SetEscortPaused(true);
                uiTimer = 2000;
                uiPhase = 1;
                break;
            case 5:
                SetEscortPaused(true);
                DoScriptText(SAY_SPYBOT_1, me);
                uiTimer = 2000;
                uiPhase = 5;
                break;
            case 17:
                SetEscortPaused(true);
                DoScriptText(SAY_SPYBOT_3, me);
                uiTimer = 3000;
                uiPhase = 8;
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (uiPhase)
        {
            if (uiTimer <= uiDiff)
            {
                switch(uiPhase)
                {
                    case 1:
                        DoScriptText(SAY_QUEST_ACCEPT_ATTACK, me);
                        uiTimer = 3000;
                        uiPhase = 2;
                        break;
                    case 2:
                        if (Creature* pTyrion = GetClosestCreatureWithEntry(me, NPC_TYRION,10.0f))
                            DoScriptText(SAY_TYRION_1, pTyrion);
                        uiTimer = 3000;
                        uiPhase = 3;
                        break;
                    case 3:
                        me->UpdateEntry(NPC_PRIESTESS_TYRIONA, ALLIANCE);
                        uiTimer = 2000;
                        uiPhase = 4;
                        break;
                    case 4:
                       SetEscortPaused(false);
                       uiPhase = 0;
                       uiTimer = 0;
                       break;
                    case 5:
                        if (Creature* pGuard = GetClosestCreatureWithEntry(me, NPC_STORMWIND_ROYAL, 10.0f))
                            DoScriptText(SAY_GUARD_1, pGuard);
                        uiTimer = 3000;
                        uiPhase = 6;
                        break;
                    case 6:
                        DoScriptText(SAY_SPYBOT_2, me);
                        uiTimer = 3000;
                        uiPhase = 7;
                        break;
                    case 7:
                        SetEscortPaused(false);
                        uiTimer = 0;
                        uiPhase = 0;
                        break;
                    case 8:
                        if (Creature* pLescovar = GetClosestCreatureWithEntry(me, NPC_LORD_GREGOR_LESCOVAR,10.0f))
                            DoScriptText(SAY_LESCOVAR_1, pLescovar);
                        uiTimer = 3000;
                        uiPhase = 9;
                        break;
                    case 9:
                        DoScriptText(SAY_SPYBOT_4, me);
                        uiTimer = 3000;
                        uiPhase = 10;
                        break;
                    case 10:
                        if (Creature* pLescovar = GetClosestCreatureWithEntry(me, NPC_LORD_GREGOR_LESCOVAR,10.0f))
                        {
                            if (Player* pPlayer = GetPlayerForEscort())
                            {
                                CAST_AI(npc_lord_gregor_lescovarAI,pLescovar->AI())->Start(false, false, pPlayer->GetGUID());
                                CAST_AI(npc_lord_gregor_lescovarAI, pLescovar->AI())->SetMaxPlayerDistance(200.0f);
                            }
                        }
                        me->DisappearAndDie();
                        me->Respawn();
                        uiTimer = 0;
                        uiPhase = 0;
                        break;
                }
            } else uiTimer -= uiDiff;
        }
        npc_escortAI::UpdateAI(uiDiff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_tyrion_spybot(Creature* pCreature)
{
    return new npc_tyrion_spybotAI(pCreature);
}

/*######
## npc_tyrion
######*/

bool QuestAccept_npc_tyrion(Player* pPlayer, Creature* pCreature, Quest const *pQuest)
{
    if (pQuest->GetQuestId() == QUEST_THE_ATTACK)
    {
        if (Creature* pSpybot = GetClosestCreatureWithEntry(pCreature, NPC_TYRION_SPYBOT, 5.0f))
        {
            CAST_AI(npc_tyrion_spybotAI,pSpybot->AI())->Start(false, false, pPlayer->GetGUID());
            CAST_AI(npc_tyrion_spybotAI,pSpybot->AI())->SetMaxPlayerDistance(200.0f);
        }
        return true;
    }
    return false;
}


/******
*   npc_squire_rowe
**********/

#define NPC_REGINAL_WINDSOR                 12580
#define QUEST_STORMWIND_RENDEZVOUS          6402
#define QUEST_THE_GREAT_THE_MASQUERADE      6403
#define GOSSIP_SQUIRE_ROWE_REGINALD         16006

#define REGINALD_SPAWN_COORDS       -9179.8, 308.65, 78.92

struct npc_squire_roweAI : public ScriptedAI
{
    npc_squire_roweAI(Creature *c) : ScriptedAI(c)
    {
    }

    uint8 eventStage;
    Timer eventTimer;
    uint64 playerGUID;

    void MovementInform(uint32 type, uint32 i)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch(i)
        {
        case 1:
            eventStage = 2;
            eventTimer = 1;
            break;
        case 2:
            eventStage = 3;
            eventTimer = 2000;
            break;
        case 3:
            m_creature->SetOrientation(2.23f);
            Reset();
            break;
        default:
            break;
        }
    }

    void Reset()
    {
        me->setActive(true);
        eventStage = 0;
        eventTimer = 0;
        playerGUID = 0;
    }

    void EnterCombat(Unit* who){}

    void JustDied(Unit *slayer){}

    void StartEvent(uint64 plGUID)
    {
        if (!plGUID)
            return;
        playerGUID = plGUID;
        eventStage = 1;
        m_creature->GetMotionMaster()->MovePoint(1, -9057, 441.2, 93.1);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!eventStage)
            return;

        if (eventTimer.Expired(diff))
        {
            switch(eventStage)
            {
                case 2:
                {
                    m_creature->GetMotionMaster()->MovePoint(2, -9086, 419.0, 92.4);
                    eventTimer = 0;
                    break;
                }
                case 3:
                {
                    m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                    eventStage++;
                    eventTimer = 4000;
                    break;
                }
                case 4:
                {
                    m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                    Creature* pUnit = m_creature->SummonCreature(NPC_REGINAL_WINDSOR, REGINALD_SPAWN_COORDS, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 900);
                    if (pUnit)
                    {
                        ((npc_escortAI*)pUnit->AI())->Start(false, true, playerGUID);
                        ((npc_escortAI*)pUnit->AI())->SetMaxPlayerDistance(200);
                    }
                    eventStage++;
                    eventTimer = 10000;
                    break;
                }
                case 5:
                {
                    eventTimer = 0;
                    m_creature->GetMotionMaster()->MovePoint(3, -9042, 434.2, 93.4);
                    break;
                }
                default:
                    break;
            }
        }
    }
};

CreatureAI* GetAI_npc_squire_rowe(Creature *_Creature)
{
    return new npc_squire_roweAI(_Creature);
}

bool GossipHello_npc_squire_rowe(Player *player, Creature *_Creature)
{
    if (((npc_squire_roweAI*)_Creature->AI())->eventStage)
        return false;

    if (player->GetQuestStatus(QUEST_STORMWIND_RENDEZVOUS)
        && !player->IsActiveQuest(QUEST_THE_GREAT_THE_MASQUERADE))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16006), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(9065, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(68, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_squire_rowe(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            ((npc_squire_roweAI*)_Creature->AI())->StartEvent(player->GetGUID());
            break;
    }
    return true;
}

/********
* Stormwind Elite Guard
*****/

float StormwindEliteGuardMoveCoords[6][4] =
{
    // right
    {-8970.5, 520.3, 96.7, 5.38},
    {-8972.0, 519.2, 96.7, 5.38},
    {-8974.3, 517.4, 96.7, 5.38},
    // left
    {-8958.3, 505.4, 96.7, 2.24},
    {-8960.2, 504.1, 96.7, 2.24},
    {-8962.0, 502.6, 96.7, 2.24}
};

struct npc_stormwind_elite_guardAI : public ScriptedAI
{
    npc_stormwind_elite_guardAI(Creature *c) : ScriptedAI(c) {}

    int npcNumber;

    void Reset() {}

    void EnterCombat(Unit* who){}

    void JustDied(Unit *slayer){}

    void MoveAndKneel()
    {
        me->GetMotionMaster()->MovePoint(0, StormwindEliteGuardMoveCoords[npcNumber][0], StormwindEliteGuardMoveCoords[npcNumber][1], StormwindEliteGuardMoveCoords[npcNumber][2]);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case 0:
                m_creature->SetOrientation(StormwindEliteGuardMoveCoords[npcNumber][3]);
                m_creature->GetMap()->CreatureRelocation(m_creature, StormwindEliteGuardMoveCoords[npcNumber][0], StormwindEliteGuardMoveCoords[npcNumber][1], StormwindEliteGuardMoveCoords[npcNumber][2], StormwindEliteGuardMoveCoords[npcNumber][3]);
                DoTeleportTo(StormwindEliteGuardMoveCoords[npcNumber][0], StormwindEliteGuardMoveCoords[npcNumber][1], StormwindEliteGuardMoveCoords[npcNumber][2]);
                m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
        }
    }

    void UpdateAI(const uint32 diff) {}
};

CreatureAI* GetAI_npc_stormwind_elite_guard(Creature *_Creature)
{
    npc_stormwind_elite_guardAI* eliteGuardAI = new npc_stormwind_elite_guardAI(_Creature);

    return (CreatureAI*)eliteGuardAI;
}


/******
*   npc_reginald_windsor
**********/

#define SAY_REGINALD_1_1    -1200049
#define SAY_REGINALD_1_2    -1200050
#define SAY_REGINALD_1_3    -1200051

#define SAY_REGINALD_2_1    -1200052
#define SAY_REGINALD_2_2    -1200053
#define SAY_REGINALD_2_3    -1200054
#define SAY_REGINALD_2_4    -1200055
#define SAY_REGINALD_2_5    -1200056
#define SAY_REGINALD_2_6    -1200057

#define SAY_REGINALD_3_1    -1200058
#define SAY_REGINALD_3_2    -1200059

#define SAY_REGINALD_4_1    -1200060
#define SAY_REGINALD_4_2    -1200061
#define SAY_REGINALD_4_3    -1200062
#define SAY_REGINALD_4_4    -1200063
#define SAY_REGINALD_4_5    -1200064
#define SAY_REGINALD_4_6    -1200065
#define SAY_REGINALD_4_7    -1200066

#define REGINALD_EMOTE      -1200067


#define SAY_GENERAL_MARCUS_1        -1200068
#define SAY_GENERAL_MARCUS_2        -1200069
#define SAY_GENERAL_MARCUS_3        -1200070
#define SAY_GENERAL_MARCUS_4        -1200071
#define SAY_GENERAL_MARCUS_5        -1200072
#define SAY_GENERAL_MARCUS_6        -1200073
#define SAY_GENERAL_MARCUS_7        -1200074

#define MARCUS_EMOTE                -1200075


#define SAY_HIGHLORD_FORDRAGON_1    -1200076
#define SAY_HIGHLORD_FORDRAGON_2    -1200077
#define SAY_HIGHLORD_FORDRAGON_3    -1200078


#define SAY_LADY_KATRANA_1          -1200079
#define SAY_LADY_KATRANA_2          -1200080
#define SAY_LADY_KATRANA_3          -1200081

#define ANDUIN_WRYN_EMOTE           -1200082

#define NPC_LADY_ONYXIA_ID                  12756
#define NPC_LADY_ONYXIA_GUARD_ID            12739
#define NPC_STORMWIND_ELITE_GUARD_ID        16396
#define NPC_MARCUS_ID                       466
#define NPC_LADY_KATRANA_ID                 1749
#define NPC_FORDRAGON_ID                    1748
#define NPC_MAJESTY_ID                      1747
#define NPC_STORMWIND_ROYAL_GUARD_ID        1756
#define NPC_STORMWIND_CITY_GUARD_ID         68
#define NPC_STORMWIND_CITY_PATROLLER_ID     1976

#define SPELL_WINDSOR_READING_TABLETS       20358
#define SPELL_LADY_ONYXIA_DESPAWNS          20466
#define SPELL_WINDSOR_DEATH                 20465

#define HORSE_COORDS                        -9052.0, 445.5, 93.1
#define MAJESTY_MOVE_COORDS                 -8501.2, 334.7, 120.9
#define FORDRAGON_MOVE_COORDS               -8449.4, 337.4, 121.4

enum Event
{
    EVENT_NONE              = 0,
    EVENT_STORMWIND         = 1,
    EVENT_GENERAL_MARCUS    = 2,
    EVENT_STORMWIND_KEEP    = 3,
    EVENT_ONYXIA            = 4
};

float StormwindGuardsCoords[7][4] =
{
    // Marcus
    {-8966.6, 511.3, 96.4, 3.78},
    // right
    {-8968.1, 513.0, 96.4, 3.78},
    {-8969.8, 515.1, 96.6, 3.78},
    {-8972.9, 518.3, 96.7, 3.78},
    // left
    {-8965.0, 509.0, 96.4, 3.78},
    {-8963.6, 507.3, 96.6, 3.78},
    {-8960.7, 503.6, 96.7, 3.78}
};

struct npc_reginald_windsorAI : public npc_escortAI
{
    npc_reginald_windsorAI(Creature *c) : npc_escortAI(c) {}

    uint8 event;
    uint8 eventPhase;
    Timer phaseTimer;
    uint8 onyxiaDespawnEvent;
    bool onyxia;
    bool marcusEventEnd;
    Timer onyxiaDespawnTimer;
    uint64 npcLeft[3];
    uint64 npcRight[3];
    std::list<uint64> npcSay;

    void GetDebugInfo(ChatHandler& reader)
    {
        std::ostringstream str;
        str << "Event " << event << " Phase " << eventPhase << " Timer " << phaseTimer.GetTimeLeft() << " " << phaseTimer.GetInterval();
        reader.SendSysMessage(str.str().c_str());
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();
        if(!player)
            return;

        switch(i)
        {
            case 3:
                event = EVENT_STORMWIND;
                eventPhase = 0;
                phaseTimer.Reset(2000);
                SetRun(false);
                SetEscortPaused(true);
                break;
            case 7:
                event = EVENT_GENERAL_MARCUS;
                eventPhase = 0;
                phaseTimer.Reset(3000);
                SetEscortPaused(true);
                break;
            case 35:
                event = EVENT_STORMWIND_KEEP;
                eventPhase = 0;
                phaseTimer.Reset(5000);
                SetEscortPaused(true);
                break;
            case 41:
                event = EVENT_ONYXIA;
                eventPhase = 0;
                phaseTimer.Reset(3000);
                SetEscortPaused(true);
                break;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(!IsEscorted())
            return;

        if (marcusEventEnd && me->IsWithinDistInMap(who, 10) && who->isGuard() && who->GetTypeId() == TYPEID_UNIT)
        {
            for (std::list<uint64>::iterator itr = npcSay.begin(); itr != npcSay.end(); ++itr)
                if ((*itr) == who->GetGUID())
                    return;

            switch (rand()%6)
            {
                case 0:
                    ((Creature*)who)->Say(-1200083, LANG_UNIVERSAL, me->GetGUID());
                    break;
                case 1:
                    ((Creature*)who)->Say(-1200084, LANG_UNIVERSAL, me->GetGUID());
                    break;
                case 2:
                    ((Creature*)who)->Say(-1200085, LANG_UNIVERSAL, me->GetGUID());
                    break;
                case 3:
                    ((Creature*)who)->Say(-1200086, LANG_UNIVERSAL, me->GetGUID());
                    break;
                case 4:
                    ((Creature*)who)->Say(-1200087, LANG_UNIVERSAL, me->GetGUID());
                    break;
                case 5:
                    ((Creature*)who)->Say(-1200088, LANG_UNIVERSAL, me->GetGUID());
                    break;
            }

            who->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
            npcSay.push_back(who->GetGUID());
        }
    }

    void Reset()
    {
        me->setActive(true);
        event = EVENT_NONE;
        eventPhase = 0;
        phaseTimer.Reset(1);
        onyxiaDespawnEvent = 0;
        onyxia = false;
        marcusEventEnd = false;
    }

    void EnterCombat(Unit* who){}

    void JustDied(Unit *slayer)
    {
        Creature * tmp = NULL;
        Map * tmpMap = me->GetMap();
        if (!tmpMap)
            return;

        for (int i = 0; i < 3; ++i)
        {
            if (tmp = tmpMap->GetCreature(npcLeft[i]))
            {
                tmp->SetVisibility(VISIBILITY_OFF);
                tmp->DestroyForNearbyPlayers();
                tmp->Kill(tmp, false);
            }

            if (tmp = tmpMap->GetCreature(npcRight[i]))
            {
                tmp->SetVisibility(VISIBILITY_OFF);
                tmp->DestroyForNearbyPlayers();
                tmp->Kill(tmp, false);
            }
        }

        if (tmp = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_MARCUS_ID)))
        {
            tmp->GetMotionMaster()->MoveTargetedHome();
            tmp->Mount(2410);
        }
    }

    void SpawnGuards()
    {
        SetEscortPaused(false);
        SetRun(false);
        SetMaxPlayerDistance(30);
        m_creature->SetUInt64Value(UNIT_FIELD_TARGET, 0);
        m_creature->Yell(-1200051, LANG_UNIVERSAL, 0);
        Map* tmpMap = m_creature->GetMap();
        Creature* tmpCreature = tmpMap->GetCreatureById(NPC_MAJESTY_ID);


        if (tmpCreature)
        {
            if (!tmpCreature->isAlive())
                tmpCreature->Respawn();
            tmpCreature->SetVisibility(VISIBILITY_ON);
        }

        if (tmpCreature = tmpMap->GetCreatureById(NPC_LADY_KATRANA_ID))
        {
            if (!tmpCreature->isAlive())
                tmpCreature->Respawn();
            tmpCreature->SetVisibility(VISIBILITY_ON);
        }

        if (tmpMap)
        {
            tmpCreature = tmpMap->GetCreatureById(NPC_MARCUS_ID);
            if (tmpCreature)
            {
                tmpCreature->Unmount();
                tmpCreature->GetMotionMaster()->MovePoint(0, StormwindGuardsCoords[0][0], StormwindGuardsCoords[0][1], StormwindGuardsCoords[0][2]);
            }
        }

        for (int i = 0; i < 6; i++)
        {
            tmpCreature = me->SummonCreature(NPC_STORMWIND_ELITE_GUARD_ID, StormwindGuardsCoords[i+1][0], StormwindGuardsCoords[i+1][1], StormwindGuardsCoords[i+1][2], StormwindGuardsCoords[i+1][3], TEMPSUMMON_TIMED_DESPAWN, 300000);
            ((npc_stormwind_elite_guardAI*)(tmpCreature->AI()))->npcNumber = i;
            tmpCreature->SetWalk(true);
            if (i < 3)
                npcRight[i] = tmpCreature->GetGUID();
            else
                npcLeft[i - 3] = tmpCreature->GetGUID();
        }

        event = EVENT_NONE;
        phaseTimer = 60000; // dont let despawn, event will be changed on waypointreached
    }

    void SpellHit(Unit * caster, const SpellEntry * spell)
    {
        if (spell && spell->Id == SPELL_WINDSOR_DEATH)
        {
            onyxiaDespawnEvent = 1;
            onyxiaDespawnTimer.Reset(1500);
            //m_creature->setDeathState(DEAD);
            m_creature->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        }
    }

    void SpellHitTarget(Unit* target, const SpellEntry * spell)
    {
        if (target && spell && spell->Id == SPELL_WINDSOR_READING_TABLETS)
        {
            //target->SummonCreature(NPC_LADY_ONYXIA_ID, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TEMPSUMMON_CORPSE_DESPAWN, 0);
            //target->SetVisibility(VISIBILITY_OFF);
            //target->DestroyForNearbyPlayers();
            //target->Kill(target, false);
            ((Creature*)target)->UpdateEntry(NPC_LADY_ONYXIA_ID);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        Player* player = GetPlayerForEscort();
        if(!player)
            return;

        Unit * tmpU = NULL;
        Map * tmpMap = NULL;

        if (onyxiaDespawnEvent)
        {
            if (onyxiaDespawnTimer.Expired(diff))
            {
                tmpMap = me->GetMap();
                if (tmpMap)
                {
                    Creature * onyxia = (Creature*)FindCreature(NPC_LADY_ONYXIA_ID, 20, me);
                    Creature * fordragon = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_FORDRAGON_ID));
                    switch (onyxiaDespawnEvent)
                    {
                        case 1:
                            if (fordragon)
                                fordragon->TextEmote(-1200089, fordragon->GetGUID(), false);

                            if (onyxia)
                                onyxia->CastSpell(onyxia, SPELL_LADY_ONYXIA_DESPAWNS, false);

                            onyxiaDespawnTimer = 1500;
                            onyxiaDespawnEvent = 2;
                            break;

                        case 2:
                            if (onyxia)
                            {
                                onyxia->SetVisibility(VISIBILITY_OFF);
                                onyxia->DestroyForNearbyPlayers();
                                onyxia->Kill(onyxia, false);
                                onyxia->RemoveCorpse();
                            }

                            onyxiaDespawnEvent = 0;
                            break;
                    }
                }
            }
        }

        switch(event)
        {
            case EVENT_STORMWIND:
            {
                if (phaseTimer.Expired(diff))
                {
                    switch (eventPhase)
                    {
                        case 0:
                            m_creature->Unmount();
                            m_creature->SetSpeed(MOVE_RUN, 1.0);
                            m_creature->SetSpeed(MOVE_WALK, 1.0);
                            SetRun(false);
                            phaseTimer = 2000;
                            break;
                        case 1:
                            if (Creature * horse = m_creature->SummonCreature(305, HORSE_COORDS, me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 10000))
                                horse->GetMotionMaster()->MovePoint(0, REGINALD_SPAWN_COORDS);

                            m_creature->Say(-1200049, LANG_UNIVERSAL, player->GetGUID());
                            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_ATTACKOFF);
                            phaseTimer = 1500;
                            break;
                        case 2:
                            m_creature->SetUInt64Value(UNIT_FIELD_TARGET, player->GetGUID());
                            m_creature->Say(-1200050, LANG_UNIVERSAL, player->GetGUID());
                            phaseTimer = 45000;
                            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                            break;
                        case 3:
                            m_creature->SetVisibility(VISIBILITY_OFF);
                            m_creature->DestroyForNearbyPlayers();
                            m_creature->Kill(m_creature, false);
                            break;
                    }
                    eventPhase++;
                }
            }
            break;
            case EVENT_GENERAL_MARCUS:
            {
                if (phaseTimer.Expired(diff))
                {
                    Creature * marcus = NULL;
                    if (!(tmpMap = m_creature->GetMap()))
                        break;

                    if (!(marcus = tmpMap->GetCreatureById(NPC_MARCUS_ID)))
                        break;

                    switch (eventPhase)
                    {
                        case 0:
                            marcus->Say(-1200068, LANG_UNIVERSAL, 0);
                            phaseTimer = 6000;
                            break;
                        case 1:
                            m_creature->Say(-1200052, LANG_UNIVERSAL, 0);
                            phaseTimer = 6000;
                            break;
                        case 2:
                            m_creature->Say(-1200053, LANG_UNIVERSAL, 0);
                            phaseTimer = 1000;
                            break;
                        case 3:
                            marcus->TextEmote(-1200075, 0);
                            phaseTimer = 6000;
                            break;
                        case 4:
                            marcus->Say(-1200069, LANG_UNIVERSAL, 0);
                            phaseTimer = 6000;
                            break;
                        case 5:
                            marcus->Say(-1200070, LANG_UNIVERSAL, 0);
                            phaseTimer = 10000;
                            break;
                        case 6:
                            m_creature->Say(-1200054, LANG_UNIVERSAL, 0);
                            phaseTimer = 9000;
                            break;
                        case 7:
                            m_creature->Say(-1200055, LANG_UNIVERSAL, 0);
                            phaseTimer = 4000;
                            break;
                        case 8:
                            marcus->SetUInt64Value(UNIT_FIELD_TARGET, npcLeft[0]);

                            for (int i = 0; i < 3; ++i)
                            {
                                if (tmpU = tmpMap->GetCreature(npcLeft[i]))
                                {
                                    ((npc_stormwind_elite_guardAI*)((Creature*)tmpU)->AI())->MoveAndKneel();
                                    tmpU->SetUInt64Value(UNIT_FIELD_TARGET, npcRight[i]);
                                }
                            }

                            marcus->Say(-1200071, LANG_UNIVERSAL, 0);
                            marcus->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                            phaseTimer = 5000;
                            break;
                        case 9:
                            marcus->SetUInt64Value(UNIT_FIELD_TARGET, npcRight[0]);

                            for (int i = 0; i < 3; ++i)
                            {
                                if (tmpU = tmpMap->GetCreature(npcRight[i]))
                                {
                                    ((npc_stormwind_elite_guardAI*)((Creature*)tmpU)->AI())->MoveAndKneel();
                                    tmpU->SetUInt64Value(UNIT_FIELD_TARGET, npcLeft[i]);
                                }
                            }

                            marcus->Say(-1200072, LANG_UNIVERSAL, 0);
                            marcus->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                            phaseTimer = 5000;
                            break;
                        case 10:
                            marcus->SetUInt64Value(UNIT_FIELD_TARGET, m_creature->GetGUID());
                            marcus->Yell(-1200073, LANG_UNIVERSAL, 0);
                            marcus->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);
                            phaseTimer = 5000;
                            break;
                        case 11:
                            marcus->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                            phaseTimer = 3000;
                            break;
                        case 12:
                            marcus->Say(-1200074, LANG_UNIVERSAL, 0);
                            phaseTimer = 4000;
                            break;
                        case 13:
                            marcus->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                            marcus->GetMotionMaster()->MovePoint(0, -8977.6, 514.2, 96.6);
                            phaseTimer = 3000;
                            break;
                        case 14:
                            marcus->GetMotionMaster()->MovePoint(0, -8976.7, 514.0, 96.6);
                            m_creature->Say(-1200056, LANG_UNIVERSAL, 0);
                            phaseTimer = 3000;
                            break;
                        case 15:
                            m_creature->Say(-1200057, LANG_UNIVERSAL, 0);
                            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                            phaseTimer = 4000;
                            break;
                        case 16:
                            //marcus->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                            marcus->SetOrientation(6.17);
                            marcus->GetMap()->CreatureRelocation(marcus, marcus->GetPositionX(), marcus->GetPositionY(), marcus->GetPositionZ(), 6.17);
                            SetEscortPaused(false);
                            phaseTimer = 0;
                            eventPhase = 0;
                            marcusEventEnd = true;
                            event = EVENT_NONE;
                            break;
                    }
                    eventPhase++;
                }
            }
            break;
            case EVENT_STORMWIND_KEEP:
            {
                if (phaseTimer.Expired(diff))
                {
                    switch (eventPhase)
                    {
                        case 0:
                            m_creature->Say(-1200058, LANG_UNIVERSAL, 0);
                            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                            phaseTimer = 4000;
                            break;
                        case 1:
                            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            phaseTimer = 45000;
                            break;
                        case 2:
                            m_creature->SetVisibility(VISIBILITY_OFF);
                            m_creature->DestroyForNearbyPlayers();
                            m_creature->Kill(m_creature, false);
                            break;
                    }
                    eventPhase++;
                }
            }
            break;
            case EVENT_ONYXIA:
            {
                Creature * ladyOnyxia = NULL;
                Creature * fordragon = NULL;
                if (phaseTimer.Expired(diff))
                {
                    if (!(tmpMap = m_creature->GetMap()))
                        break;

                    if (!onyxia)
                    {
                        if (!(ladyOnyxia = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_LADY_KATRANA_ID))))
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (!(ladyOnyxia = (Creature*)FindCreature(NPC_LADY_ONYXIA_ID, 20, me)))
                        {
                            break;
                        }
                    }


                    if (!(fordragon = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_FORDRAGON_ID))))
                    {
                        break;
                    }

                    switch (eventPhase)
                    {
                        case 0:
                            m_creature->Say(-1200060, LANG_UNIVERSAL, 0);
                            phaseTimer = 2000;
                            break;
                        case 1:
                        {
                            fordragon->Say(-1200076, LANG_UNIVERSAL, 0);
                            Creature * majesty = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_MAJESTY_ID));

                            if (majesty)
                            {
                                majesty->SetWalk(false);
                                majesty->SetSpeed(MOVE_RUN, 1.5);
                                majesty->GetMotionMaster()->MovePoint(0, MAJESTY_MOVE_COORDS);
                                majesty->TextEmote(-1200082, 0);
                            }

                            phaseTimer = 4000;
                            break;
                        }
                        case 2:
                        {
                            Creature * majesty = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_MAJESTY_ID));

                            if (majesty)
                            {
                                majesty->SetVisibility(VISIBILITY_OFF);
                                majesty->DestroyForNearbyPlayers();
                                majesty->Kill(majesty, false);
                                majesty->RemoveCorpse();
                            }

                            m_creature->Say(-1200061, LANG_UNIVERSAL, 0);
                            phaseTimer = 4000;
                            break;
                        }
                        case 3:
                            ladyOnyxia->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                            phaseTimer = 3000;
                            break;
                        case 4:
                            ladyOnyxia->Say(-1200079, LANG_UNIVERSAL, 0);
                            phaseTimer = 8000;
                            break;
                        case 5:
                            ladyOnyxia->Say(-1200080, LANG_UNIVERSAL, 0);
                            phaseTimer = 8000;
                            break;
                        case 6:
                            m_creature->Say(-1200062, LANG_UNIVERSAL, 0);
                            phaseTimer = 5000;
                            break;
                        case 7:
                            m_creature->Say(-1200063, LANG_UNIVERSAL, 0);
                            phaseTimer = 5000;
                            break;
                        case 8:
                            m_creature->Say(-1200064, LANG_UNIVERSAL, 0);
                            phaseTimer = 5000;
                            break;
                        case 9:
                            m_creature->TextEmote(-1200067, 0);
                            m_creature->CastSpell(ladyOnyxia, SPELL_WINDSOR_READING_TABLETS, false);
                            phaseTimer = 13000;
                            break;
                        case 10:
                            fordragon->SetSpeed(MOVE_RUN, 2.0);
                            fordragon->SetWalk(false);
                            fordragon->GetMotionMaster()->MovePoint(0, FORDRAGON_MOVE_COORDS);
                            phaseTimer = 1000;
                            break;
                        case 11:
                            fordragon->SetUInt64Value(UNIT_FIELD_TARGET, ladyOnyxia->GetGUID());
                            ladyOnyxia->Say(-1200081, LANG_UNIVERSAL, 0);
                            phaseTimer = 1000;
                            break;
                        case 12:
                            fordragon->Say(-1200077, LANG_UNIVERSAL, 0);
                            phaseTimer = 1500;
                            break;
                        case 13:
                        {
                            std::list<Creature*> guardList = FindAllCreaturesWithEntry(NPC_STORMWIND_ROYAL_GUARD_ID, 27.0);
                            //Creature * tmpc = NULL;
                            for (std::list<Creature*>::iterator i = guardList.begin(); i != guardList.end(); ++i)
                            {
                                if (*i)
                                {
                                    WorldLocation guardWLoc;

                                    (*i)->GetPosition(guardWLoc);

                                    /*tmpc = */ladyOnyxia->SummonCreature(NPC_LADY_ONYXIA_GUARD_ID, guardWLoc.coord_x, guardWLoc.coord_y, guardWLoc.coord_z, guardWLoc.orientation, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 15000);
                                    //if (tmpc)
                                    //{
                                    //    tmpc->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                                    //    tmpc->SetNoCallAssistance(true);
                                    //}

                                    (*i)->SetVisibility(VISIBILITY_OFF);
                                    (*i)->DestroyForNearbyPlayers();
                                    (*i)->Kill((*i), false);
                                    (*i)->RemoveCorpse();
                                }
                            }

                            phaseTimer = 2000;
                            break;
                        }
                        case 14:
                        {
                            std::list<Creature*> guardList = FindAllCreaturesWithEntry(NPC_LADY_ONYXIA_GUARD_ID, 40.0);

                            std::list<Player*> playerList = FindAllPlayersInRange(40, ladyOnyxia);

                            for (std::list<Player*>::iterator itr = playerList.begin(); itr != playerList.end();)
                            {
                                std::list<Player*>::iterator tmpItr = itr;
                                ++itr;

                                if (!(*tmpItr) || !(*tmpItr)->isAlive())
                                    playerList.erase(tmpItr);
                            }

                            std::list<Player*>::iterator tmpItr;
                            int randN;
                            for (std::list<Creature*>::iterator i = guardList.begin(); i != guardList.end(); ++i)
                            {
                                if (*i)
                                {
                                    randN = irand(0, playerList.size());
                                    if (randN == 0)
                                        (*i)->AI()->AttackStart(fordragon);
                                    else
                                    {
                                        tmpItr = playerList.begin();
                                        advance(tmpItr, randN - 1);
                                        (*i)->AI()->AttackStart(*tmpItr);
                                    }

                                    //(*i)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                                }
                            }

                            ladyOnyxia->CastSpell((Unit*)NULL, SPELL_WINDSOR_DEATH, false);
                            fordragon->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                            m_creature->Say(-1200065, LANG_UNIVERSAL, 0);
                            phaseTimer = 2000;
                            break;
                        }
                        case 15:
                            if (player->IsInCombat() || fordragon->IsInCombat())
                                eventPhase--;
                            phaseTimer = 2000;
                            break;
                        case 16:
                            fordragon->SetSpeed(MOVE_RUN, 2.0);
                            fordragon->SetWalk(false);
                            fordragon->GetMotionMaster()->MovePoint(0, FORDRAGON_MOVE_COORDS);
                            phaseTimer = 1500;
                            break;
                        case 17:
                            fordragon->SetUInt64Value(UNIT_FIELD_TARGET, me->GetGUID());
                            fordragon->SetStandState(UNIT_STAND_STATE_KNEEL);
                            fordragon->Say(-1200078, LANG_UNIVERSAL, 0);
                            phaseTimer = 2000;
                            break;
                        case 18:
                            m_creature->Say(-1200066, LANG_UNIVERSAL, 0);
                            phaseTimer = 2000;
                            break;
                        case 19:
                        {
                            fordragon->SetStandState(UNIT_STAND_STATE_STAND);
                            fordragon->GetMotionMaster()->MoveTargetedHome();
                            ladyOnyxia->SetVisibility(VISIBILITY_ON);
                            ladyOnyxia->RemoveCorpse();

                            if (Creature * majesty = tmpMap->GetCreature(tmpMap->GetCreatureGUID(NPC_MAJESTY_ID)))
                                majesty->SetVisibility(VISIBILITY_ON);

                            player->CompleteQuest(QUEST_THE_GREAT_THE_MASQUERADE);
                            phaseTimer = 2000;
                            break;
                        }
                        case 20:
                            m_creature->SetVisibility(VISIBILITY_OFF);
                            m_creature->DestroyForNearbyPlayers();
                            m_creature->Kill(m_creature, false);
                            m_creature->RemoveCorpse();
                            break;

                    }
                    eventPhase++;
                }
            }
            break;
        }

        npc_escortAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_npc_reginald_windsor(Creature *_Creature)
{
    npc_reginald_windsorAI* reginald_windsorAI = new npc_reginald_windsorAI(_Creature);

    reginald_windsorAI->AddWaypoint(0, -9177.7, 330.3, 82, 1000);
    reginald_windsorAI->AddWaypoint(1, -9154.8, 362.1, 90, 0);
    reginald_windsorAI->AddWaypoint(2, -9075.2, 425.2, 93.1, 0);
    reginald_windsorAI->AddWaypoint(3, -9051.1, 444.4, 93.1, 0);

    reginald_windsorAI->AddWaypoint(4, -9033.2, 459.2, 93.1, 0);
    reginald_windsorAI->AddWaypoint(5, -9012.1, 474.8, 96.5, 0);
    reginald_windsorAI->AddWaypoint(6, -8981.5, 499.2, 96.5, 500);
    reginald_windsorAI->AddWaypoint(7, -8970.9, 507.8, 96.4, 4000);

    reginald_windsorAI->AddWaypoint(8, -8954.9, 520, 96.4, 0);
    reginald_windsorAI->AddWaypoint(9, -8927.1, 493.4, 93.9, 0);
    reginald_windsorAI->AddWaypoint(10, -8906.7, 510, 93.9, 0);
    reginald_windsorAI->AddWaypoint(11, -8921, 529.8, 94.8, 0);
    reginald_windsorAI->AddWaypoint(12, -8925.4, 543.0, 94.3, 0);
    reginald_windsorAI->AddWaypoint(13, -8828.1, 623.3, 93.9, 0);
    reginald_windsorAI->AddWaypoint(14, -8798.4, 592.4, 97.5, 0);
    reginald_windsorAI->AddWaypoint(15, -8789.6, 592.4, 97.6, 0);
    reginald_windsorAI->AddWaypoint(16, -8775.4, 605.1, 97.3, 0);
    reginald_windsorAI->AddWaypoint(17, -8768.4, 606.4, 97.0, 0);
    reginald_windsorAI->AddWaypoint(18, -8740.2, 578.2, 97.5, 0);
    reginald_windsorAI->AddWaypoint(19, -8740.7, 571.3, 97.4, 0);
    reginald_windsorAI->AddWaypoint(20, -8749.2, 560.1, 97.5, 0);
    reginald_windsorAI->AddWaypoint(21, -8744.5, 555.7, 98.1, 0);
    reginald_windsorAI->AddWaypoint(22, -8738.6, 550.0, 100.3, 0);
    reginald_windsorAI->AddWaypoint(23, -8732.6, 543.1, 101.2, 0);
    reginald_windsorAI->AddWaypoint(24, -8725.1, 534.3, 100.4, 0);
    reginald_windsorAI->AddWaypoint(25, -8719.5, 527.5, 98.9, 0);
    reginald_windsorAI->AddWaypoint(26, -8713.1, 519.9, 97.2, 0);
    reginald_windsorAI->AddWaypoint(27, -8699.8, 529.0, 97.8, 0);
    reginald_windsorAI->AddWaypoint(28, -8674.8, 551.2, 97.4, 0);
    reginald_windsorAI->AddWaypoint(29, -8657.3, 553.4, 97.0, 0);
    reginald_windsorAI->AddWaypoint(30, -8649.6, 549.0, 97.4, 0);
    reginald_windsorAI->AddWaypoint(31, -8613.1, 512.8, 103.5, 0);
    reginald_windsorAI->AddWaypoint(32, -8578.6, 476.5, 104.3, 0);
    reginald_windsorAI->AddWaypoint(33, -8564.2, 467.5, 104.5, 0);
    reginald_windsorAI->AddWaypoint(34, -8548.8, 467.7, 104.6, 0);
    reginald_windsorAI->AddWaypoint(35, -8547.2, 465.6, 104.6, 0);

    reginald_windsorAI->AddWaypoint(36, -8535.9, 452.0, 105, 3000);
    reginald_windsorAI->AddWaypoint(37, -8508.0, 416.6, 108.4, 0);
    reginald_windsorAI->AddWaypoint(38, -8487.9, 391.5, 108.4, 0);
    reginald_windsorAI->AddWaypoint(39, -8455.5, 350.9, 120.9, 0);
    reginald_windsorAI->AddWaypoint(40, -8448.0, 341.3, 120.9, 0);
    reginald_windsorAI->AddWaypoint(41, -8446.6, 339.5, 121.3, 0);
    reginald_windsorAI->AddWaypoint(42, -8446.6, 339.5, 121.3, 0);

    return (CreatureAI*)reginald_windsorAI;
}

bool GossipHello_npc_reginald_windsor(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->IsActiveQuest(QUEST_THE_GREAT_THE_MASQUERADE))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16007), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(5633, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(68, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_reginald_windsor(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            if (player->IsActiveQuest(QUEST_THE_GREAT_THE_MASQUERADE))
            {
                ((npc_reginald_windsorAI*)creature->AI())->SetEscortPaused(false);
                creature->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                creature->Say(-1200059, LANG_UNIVERSAL, 0);
                ((npc_reginald_windsorAI*)creature->AI())->event = EVENT_NONE;
            }
            break;
    }

    return true;
}

bool QuestAccept_npc_reginald_windsor(Player * player, Creature * creature, Quest const * quest)
{
    if (quest->GetQuestId() == QUEST_THE_GREAT_THE_MASQUERADE)
    {
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        ((npc_reginald_windsorAI*)creature->AI())->SpawnGuards();
    }

    return true;
}

/*#####
# npc_doc_mixilpixil
######*/
#define QUEST_THE_TOUCH_OF_ZANZIL_STEP2     2608
#define SAY_DOC_1                           -1000099
#define SAY_DOC_2                           -1000098

struct npc_doc_mixilpixilAI : public ScriptedAI
{
    npc_doc_mixilpixilAI(Creature* creature) : ScriptedAI(creature) {}
    
    Timer EventTimer;
    uint32 EventTarget;

    bool StartEvent;
    int EventPhase;

    void Reset()
    {
        EventTimer.Reset(1000);
        StartEvent = false;
        EventPhase = 0;
        EventTarget = 0;
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if (eventType == 5) 
        {
            EventTarget = invoker->GetGUID();
            StartEvent = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (StartEvent && EventTimer.Expired(diff))
        {
            switch(EventPhase)
            {
                case 0:
                    m_creature->SetWalk(true);
                    if (Player *player = Unit::GetPlayerInWorld(EventTarget))
                        m_creature->GetMotionMaster()->MovePoint(1, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());
                    EventTimer = 3000;
                    break;
                case 1:
                    m_creature->HandleEmoteCommand(EMOTE_STATE_USESTANDING);
                    EventTimer = 2000;
                    break;
                case 2:
                    DoScriptText(SAY_DOC_1, m_creature);
                    EventTimer = 2000;
                    break;
                case 3:
                    m_creature->GetMotionMaster()->MovePoint(2, -8805.57,  338.27,  95.09);
                    EventTimer = 5000;
                    break;
                case 4:
                    m_creature->GetMotionMaster()->MovePoint(3, -8803.56,  325.36,  95.09);
                    EventTimer = 6000;
                    break;
                case 5:
                    m_creature->GetMotionMaster()->MovePoint(4, -8805.56, 331.968, 95.0986);
                    EventTimer = 2000;
                    break;
                case 6:
                    m_creature->GetMotionMaster()->MoveTargetedHome();
                    if (Player *player = Unit::GetPlayerInWorld(EventTarget))
                        player->CompleteQuest(QUEST_THE_TOUCH_OF_ZANZIL_STEP2);
                    DoScriptText(SAY_DOC_2, m_creature);
                    EventTimer = 1000;
                    break;
                case 7:
                    Reset();
                    return;
            }
            EventPhase++;
        }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_doc_mixilpixil(Creature* creature)
{
    return new npc_doc_mixilpixilAI(creature);
}

bool ReceiveEmote_npc_doc_mixilpixil(Player *player, Creature* pCreature, uint32 emote)
{
    if (emote == TEXTEMOTE_LAYDOWN 
        && player->GetQuestStatus(QUEST_THE_TOUCH_OF_ZANZIL_STEP2) == QUEST_STATUS_INCOMPLETE
        && pCreature->GetDistance(player) <= 10)

        pCreature->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, player, pCreature);

    return true;
}

/*#####
# npc_duthorian_rall
######*/

bool GossipHello_npc_duthorian_rall(Player *player, Creature *creature)
{
    if(creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if(!player->HasItemCount(6775, 1, true) && player->GetQuestStatus(1642) != QUEST_STATUS_COMPLETE &&
        player->GetQuestRewardStatus(1641) && !player->GetQuestRewardStatus(1642))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16050), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_duthorian_rall(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
        {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 6775, 1);
            if (msg == EQUIP_ERR_OK)
                player->StoreNewItem(dest, 6775, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        default: break;
    }
    return true;
}


void AddSC_stormwind_city()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_archmage_malin";
    newscript->pGossipHello = &GossipHello_npc_archmage_malin;
    newscript->pGossipSelect = &GossipSelect_npc_archmage_malin;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_bartleby";
    newscript->GetAI = &GetAI_npc_bartleby;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_bartleby;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dashel_stonefist";
    newscript->GetAI = &GetAI_npc_dashel_stonefist;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_dashel_stonefist;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_general_marcus_jonathan";
    newscript->pReceiveEmote = &ReceiveEmote_npc_general_marcus_jonathan;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_lady_katrana_prestor";
    newscript->pGossipHello = &GossipHello_npc_lady_katrana_prestor;
    newscript->pGossipSelect = &GossipSelect_npc_lady_katrana_prestor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tyrion";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_tyrion;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tyrion_spybot";
    newscript->GetAI = &GetAI_npc_tyrion_spybot;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lord_gregor_lescovar";
    newscript->GetAI = &GetAI_npc_lord_gregor_lescovar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_marzon_silent_blade";
    newscript->GetAI = &GetAI_npc_marzon_silent_blade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_highlord_bolvar_fordragon";
    newscript->GetAI = &GetAI_npc_highlord_bolvar_fordragon;
    newscript->pQuestRewardedNPC = &QuestComplete_npc_highlord_bolvar_fordragon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_reginald_windsor";
    newscript->pGossipHello = &GossipHello_npc_reginald_windsor;
    newscript->pGossipSelect = &GossipSelect_npc_reginald_windsor;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_reginald_windsor;
    newscript->GetAI = &GetAI_npc_reginald_windsor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_squire_rowe";
    newscript->pGossipHello = &GossipHello_npc_squire_rowe;
    newscript->pGossipSelect = &GossipSelect_npc_squire_rowe;
    newscript->GetAI = &GetAI_npc_squire_rowe;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stormwind_elite_guard";
    newscript->GetAI = &GetAI_npc_stormwind_elite_guard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_doc_mixilpixil";
    newscript->GetAI = &GetAI_npc_doc_mixilpixil;
    newscript->pReceiveEmote =  &ReceiveEmote_npc_doc_mixilpixil;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_duthorian_rall";
    newscript->pGossipHello = &GossipHello_npc_duthorian_rall;
    newscript->pGossipSelect = &GossipSelect_npc_duthorian_rall;
    newscript->RegisterSelf();
}

