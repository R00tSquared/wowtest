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
SDName: Dustwallow_Marsh
SD%Complete: 95
SDComment: Quest support: 1270, 1222, 11180, 558, 11126. Vendor Nat Pagle
SDCategory: Dustwallow Marsh
EndScriptData */

/* ContentData
npc_cassa_crimsonwing
mobs_risen_husk_spirit
npc_restless_apparition
npc_deserter_agitator
npc_dustwallow_lady_jaina_proudmoore
npc_nat_pagle
npc_theramore_combat_dummy
mob_mottled_drywallow_crocolisks
npc_morokk
npc_ogron
npc_private_hendel
npc_stinky
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_cassa_crimsonwing
######*/

#define GOSSIP_SURVEY_ALCAZ_ISLAND 16304

bool GossipHello_npc_cassa_crimsonwing(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(11142) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SURVEY_ALCAZ_ISLAND), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    }

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(),_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_cassa_crimsonwing(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player,42295,false);               //TaxiPath 724
    }
    return true;
}

/*######
## mobs_risen_husk_spirit
######*/

#define SPELL_SUMMON_RESTLESS_APPARITION    42511
#define SPELL_CONSUME_FLESH                 37933           //Risen Husk
#define SPELL_INTANGIBLE_PRESENCE           43127           //Risen Spirit

struct mobs_risen_husk_spiritAI : public ScriptedAI
{
    mobs_risen_husk_spiritAI(Creature *c) : ScriptedAI(c) {}

    Timer ConsumeFlesh_Timer;
    Timer IntangiblePresence_Timer;
    Timer CheckTimer;

    void Reset()
    {
        ConsumeFlesh_Timer.Reset(10000);
        IntangiblePresence_Timer.Reset(5000);
        CheckTimer.Reset(0);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if(done_by->GetTypeId() == TYPEID_PLAYER)
            if(damage >= m_creature->GetHealth() && ((Player*)done_by)->GetQuestStatus(11180) == QUEST_STATUS_INCOMPLETE)
                m_creature->CastSpell(done_by, SPELL_SUMMON_RESTLESS_APPARITION, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(ConsumeFlesh_Timer.Expired(diff))
        {
            if(m_creature->GetEntry() == 23555)
            {
                if(me->GetDistance(me->GetVictim()) < 5)
                    DoCast(m_creature->GetVictim(),SPELL_CONSUME_FLESH);
            }
            ConsumeFlesh_Timer = 15000;
            CheckTimer = 1000;
        } 

        if(CheckTimer.Expired(diff))
        {
            if(me->GetVictim()->HasAura(SPELL_CONSUME_FLESH))
            {
                if(me->GetDistance(me->GetVictim()) > 5)
                    me->InterruptNonMeleeSpells(true);
                CheckTimer = 500;
            }
            else
                CheckTimer = 0;
        }

        if(IntangiblePresence_Timer.Expired(diff))
        {
            if(m_creature->GetEntry() == 23554)
                DoCast(m_creature,SPELL_INTANGIBLE_PRESENCE);
            IntangiblePresence_Timer = 20000;
        } 

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mobs_risen_husk_spirit(Creature *_Creature)
{
    return new mobs_risen_husk_spiritAI (_Creature);
}

/*######
## npc_restless_apparition
######*/

bool GossipHello_npc_restless_apparition(Player *player, Creature *_Creature)
{
    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
    _Creature->SetInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    return true;
}

/*######
## npc_deserter_agitator
######*/

struct npc_deserter_agitatorAI : public ScriptedAI
{
    npc_deserter_agitatorAI(Creature *c) : ScriptedAI(c) {}

    Timer reset_timer;
    Timer RandomSayTimer;
    bool Fled;

    void Reset()
    {
        me->setFaction(894);
        reset_timer.Reset(0);
        RandomSayTimer.Reset(urand(30000, 300000));
        Fled = false;
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type == POINT_MOTION_TYPE && id == 100)
            me->DisappearAndDie();
    }

    void DoAction(const int32 param)
    {
        switch(urand(0, 3))
        {
            case 0:
                me->Say(-1200387, LANG_UNIVERSAL, 0);
                break;
            case 1:
                me->Say(-1200388, LANG_UNIVERSAL, 0);
                break;
            case 2:
                me->Say(-1200389, LANG_UNIVERSAL, 0);
                break;
            case 3:
                me->Say(-1200390, LANG_UNIVERSAL, 0);
                break;
        }

        if (param == 0)
        {
            float x, y, z;
            me->GetNearPoint(x, y, z, 15.0f);
            me->GetMotionMaster()->MovePoint(100, x, y, z);
        }
        else if (param == 1)
        {
            me->setFaction(1883);
			me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
            me->AI()->AttackStart(me->SelectNearestTarget(7.0));
            reset_timer = 15000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (UpdateVictim())
        {
            if (!Fled && HealthBelowPct(15))
            {
                Fled = true;
                me->DoFleeToGetAssistance();
                DoScriptText(-1901007, me);
            }
            DoMeleeAttackIfReady();
            return;
        }

        if(RandomSayTimer.Expired(diff))
        {
            switch(urand(0, 2))
            {
                case 0:
                    me->Say(-1200391, LANG_UNIVERSAL, 0);
                    break;
                case 1:
                    me->Say(-1200392, LANG_UNIVERSAL, 0);
                    break;
                case 2:
                    me->Say(-1200393, LANG_UNIVERSAL, 0);
                    break;
            }
            RandomSayTimer = urand(30000, 300000);
        }

        if (reset_timer.Expired(diff))
            Reset();
    }
};

CreatureAI* GetAI_npc_deserter_agitator(Creature *_Creature)
{
    return new npc_deserter_agitatorAI (_Creature);
}

bool GossipHello_npc_deserter_agitator(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(11126) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16305), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(23602, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_deserter_agitator(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
        _Creature->AI()->DoAction(urand(0, 1));
    }
    return true;
}

/*######
## npc_dustwallow_lady_jaina_proudmoore - TODO: should also have own scripted AI when horde attacks her
######*/

#define GOSSIP_ITEM_JAINA        16306
#define GOSSIP_TELE_TO_STORMWIND 16307

struct npc_dustwallow_lady_jaina_proudmooreAI : public ScriptedAI
{
    npc_dustwallow_lady_jaina_proudmooreAI(Creature *c) : ScriptedAI(c) {}

    void Reset() { }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        /*
        Some AI TODO here
        */

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_dustwallow_lady_jaina_proudmoore(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(558) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_JAINA), GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO);

    if (player->GetQuestStatus(11222) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(11222))   // Warn Bolvar!
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_TELE_TO_STORMWIND), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_dustwallow_lady_jaina_proudmoore(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_SENDER_INFO)
    {
        player->SEND_GOSSIP_MENU( 7012, _Creature->GetGUID() );
        player->CastSpell( player, 23122, false);
    }
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        // teleport to Highlord Bolvar
        _Creature->CastSpell(player, 42710, true);
    }
    return true;
}

CreatureAI* GetAI_npc_dustwallow_lady_jaina_proudmoore(Creature *_creature)
{
    return new npc_dustwallow_lady_jaina_proudmooreAI(_creature);
}

/*######
## npc_nat_pagle
######*/

bool GossipHello_npc_nat_pagle(Player *player, Creature *_Creature)
{
    if(_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(_Creature->isVendor() && player->GetQuestRewardStatus(8227))
    {
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU( 7640, _Creature->GetGUID() );
    }
    else
        player->SEND_GOSSIP_MENU( 7638, _Creature->GetGUID() );

    return true;
}

bool GossipSelect_npc_nat_pagle(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if(action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*######
## npc_theramore_combat_dummy
######*/

struct npc_theramore_combat_dummyAI : public Scripted_NoMovementAI
{
    npc_theramore_combat_dummyAI(Creature *c) : Scripted_NoMovementAI(c)
    {
    }
    Timer Check_Timer;

    void Reset()
    {
        m_creature->SetNoCallAssistance(true);
        m_creature->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STUN, true);
        Check_Timer.Reset(10000);
    }

    void EnterCombat(Unit* who)
    {
        m_creature->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN, UNIT_ACTION_PRIORITY_END);
    }

    void DamageTaken(Unit *attacker, uint32 &damage)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (Check_Timer.Expired(diff))
        {
            Player* target = NULL;
            Hellground::AnyPlayerInObjectRangeCheck u_check(m_creature, 12.0f);
            Hellground::ObjectSearcher<Player,Hellground::AnyPlayerInObjectRangeCheck> searcher(target,u_check);
            Cell::VisitAllObjects(me, searcher, 12.0f);
            if (!target && me->IsInCombat())
                EnterEvadeMode();

            Check_Timer = 10000;
        }

        if (!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_theramore_combat_dummy(Creature *_Creature)
{
    return new npc_theramore_combat_dummyAI (_Creature);
}

#define QUEST_THE_GRIMTOTEM_WEAPON      11169
#define AURA_CAPTURED_TOTEM             42454
#define NPC_CAPTURED_TOTEM              23811

/*######
## mob_mottled_drywallow_crocolisks
######*/

struct mob_mottled_drywallow_crocolisksAI : public ScriptedAI
{
   mob_mottled_drywallow_crocolisksAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}
    void JustDied (Unit* killer)
    {
        Player *pPlayer = NULL;

        if(!IS_PLAYER_GUID(killer->GetGUID()))
        {
            if(!IS_PLAYER_GUID(killer->GetCharmerOrOwnerGUID()))
                return;
            else
                pPlayer = killer->GetPlayerInWorld(killer->GetCharmerOrOwnerGUID());
        }
        else
            pPlayer = (Player*)killer;

        if(!pPlayer)
            return;

        if(pPlayer->GetQuestStatus(QUEST_THE_GRIMTOTEM_WEAPON) == QUEST_STATUS_INCOMPLETE)
        {
            if(Unit* totem = FindCreature(NPC_CAPTURED_TOTEM, 20.0, m_creature))   //blizzlike(?) check by dummy aura is NOT working, mysteriously...
                pPlayer->KilledMonster(NPC_CAPTURED_TOTEM, pPlayer->GetGUID());
        }
    }
    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_mottled_drywallow_crocolisks(Creature *_Creature)
{
    return new mob_mottled_drywallow_crocolisksAI (_Creature);
}

/*######
## npc_morokk
######*/

enum
{
    SAY_MOR_CHALLENGE               = -1800499,
    SAY_MOR_SCARED                  = -1800500,

    QUEST_CHALLENGE_MOROKK          = 1173,

    FACTION_MOR_HOSTILE             = 168,
    FACTION_MOR_RUNNING             = 35
};

struct npc_morokkAI : public npc_escortAI
{
    npc_morokkAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_bIsSuccess = false;
        Reset();
    }

    bool m_bIsSuccess;

    void Reset() { me->setFaction(FACTION_MOR_RUNNING); }

    void WaypointReached(uint32 PointId)
    {
        switch(PointId)
        {
            case 0:
                SetEscortPaused(true);
                break;
            case 1:
                if (m_bIsSuccess)
                    DoScriptText(SAY_MOR_SCARED, me);
                else
                {
                    me->setDeathState(JUST_DIED);
                    me->Respawn();
                }
                break;
        }
    }

    void AttackedBy(Unit* pAttacker)
    {
        if (me->GetVictim())
            return;

        if (me->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
        {
            if (me->GetHealth()*100 < me->GetMaxHealth()*30.0f)
            {
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_CHALLENGE_MOROKK, me);

                me->setFaction(FACTION_MOR_RUNNING);
                SetRun(true);

                m_bIsSuccess = true;
                EnterEvadeMode();

                uiDamage = 0;
            }
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    m_bIsSuccess = false;
                    DoScriptText(SAY_MOR_CHALLENGE, me, pPlayer);
                    me->setFaction(FACTION_MOR_HOSTILE);
                    AttackStart(pPlayer);
                }

                SetEscortPaused(false);
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_morokk(Creature* pCreature)
{
    return new npc_morokkAI(pCreature);
}

bool QuestAccept_npc_morokk(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CHALLENGE_MOROKK)
    {
        if (npc_morokkAI* pEscortAI = CAST_AI(npc_morokkAI, pCreature->AI()))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest);

        return true;
    }

    return false;
}


/*######
## npc_ogron
######*/

enum
{
    SAY_OGR_START                       = -1800452,
    SAY_OGR_SPOT                        = -1800453,
    SAY_OGR_RET_WHAT                    = -1800454,
    SAY_OGR_RET_SWEAR                   = -1800455,
    SAY_OGR_REPLY_RET                   = -1800456,
    SAY_OGR_RET_TAKEN                   = -1800457,
    SAY_OGR_TELL_FIRE                   = -1800458,
    SAY_OGR_RET_NOCLOSER                = -1800459,
    SAY_OGR_RET_NOFIRE                  = -1800460,
    SAY_OGR_RET_HEAR                    = -1800461,
    SAY_OGR_CAL_FOUND                   = -1800462,
    SAY_OGR_CAL_MERCY                   = -1800463,
    SAY_OGR_HALL_GLAD                   = -1800464,
    EMOTE_OGR_RET_ARROW                 = -1800465,
    SAY_OGR_RET_ARROW                   = -1800466,
    SAY_OGR_CAL_CLEANUP                 = -1800467,
    SAY_OGR_NODIE                       = -1800468,
    SAY_OGR_SURVIVE                     = -1800469,
    SAY_OGR_RET_LUCKY                   = -1800470,
    SAY_OGR_THANKS                      = -1800471,

    QUEST_QUESTIONING                   = 1273,

    FACTION_GENERIC_FRIENDLY            = 35,
    FACTION_THER_HOSTILE                = 151,

    NPC_REETHE                          = 4980,
    NPC_CALDWELL                        = 5046,
    NPC_HALLAN                          = 5045,
    NPC_SKIRMISHER                      = 5044,

    SPELL_FAKE_SHOT                     = 7105,

    PHASE_INTRO                         = 0,
    PHASE_GUESTS                        = 1,
    PHASE_FIGHT                         = 2,
    PHASE_COMPLETE                      = 3
};

static float m_afSpawn[] = {-3383.501953f, -3203.383301f, 36.149f};
static float m_afMoveTo[] = {-3371.414795f, -3212.179932f, 34.210f};

struct npc_ogronAI : public npc_escortAI
{
    npc_ogronAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        lCreatureList.clear();
        Phase = 0;
        PhaseCounter = 0;
        Reset();
    }

    std::list<uint64> lCreatureList;

    uint32 Phase;
    uint32 PhaseCounter;
    Timer m_uiGlobalTimer;

    void Reset()
    {
        m_uiGlobalTimer.Reset(5000);

        /*if (HasEscortState(STATE_ESCORT_PAUSED) && Phase == PHASE_FIGHT)
            Phase = PHASE_COMPLETE;*/

        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            lCreatureList.clear();
            Phase = 0;
            PhaseCounter = 0;
        }
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING) && pWho->GetEntry() == NPC_REETHE && lCreatureList.empty())
            lCreatureList.push_back(((Creature*)pWho)->GetGUID());

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    void WaypointReached(uint32 PointId)
    {
        switch(PointId)
        {
            case 9:
                DoScriptText(SAY_OGR_SPOT, me);
                break;
            case 10:
                if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                    DoScriptText(SAY_OGR_RET_WHAT, pReethe);
                break;
            case 11:
                SetEscortPaused(true);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        lCreatureList.push_back(pSummoned->GetGUID());

        pSummoned->setFaction(FACTION_GENERIC_FRIENDLY);

        if (pSummoned->GetEntry() == NPC_CALDWELL)
            pSummoned->GetMotionMaster()->MovePoint(0, m_afMoveTo[0], m_afMoveTo[1], m_afMoveTo[2]);
        else
        {
            if (Creature* pCaldwell = GetClosestCreatureWithEntry(me, NPC_CALDWELL, 15.0f))
            {
                //will this conversion work without compile warning/error?
                size_t iSize = lCreatureList.size();
                pSummoned->GetMotionMaster()->MoveFollow(pCaldwell, 0.5f, (M_PI/2)*(int)iSize);
            }
        }
    }

    void DoStartAttackMe()
    {
        if (!lCreatureList.empty())
        {
            for(std::list<uint64>::iterator itr = lCreatureList.begin(); itr != lCreatureList.end(); ++itr)
            {
                if (Creature* it = me->GetCreature(*itr))
                {
                    if (it->GetEntry() == NPC_REETHE)
                        continue;

                    if (it->isAlive())
                    {
                        it->setFaction(FACTION_THER_HOSTILE);
                        it->AI()->AttackStart(me);
                    }
                }
            }
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (HasEscortState(STATE_ESCORT_PAUSED))
            {
                if (m_uiGlobalTimer.Expired(diff))
                {
                    m_uiGlobalTimer = 5000;

                    switch(Phase)
                    {
                        case PHASE_INTRO:
                        {
                            switch(PhaseCounter)
                            {
                                case 0:
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_SWEAR, pReethe);
                                    break;
                                case 1:
                                    DoScriptText(SAY_OGR_REPLY_RET, me);
                                    break;
                                case 2:
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_TAKEN, pReethe);
                                    break;
                                case 3:
                                    DoScriptText(SAY_OGR_TELL_FIRE, me);
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_NOCLOSER, pReethe);
                                    break;
                                case 4:
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_NOFIRE, pReethe);
                                    break;
                                case 5:
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_HEAR, pReethe);

                                    me->SummonCreature(NPC_CALDWELL, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
                                    me->SummonCreature(NPC_HALLAN, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
                                    me->SummonCreature(NPC_SKIRMISHER, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
                                    me->SummonCreature(NPC_SKIRMISHER, m_afSpawn[0], m_afSpawn[1], m_afSpawn[2], 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);

                                    Phase = PHASE_GUESTS;
                                    break;
                            }
                            break;
                        }

                        case PHASE_GUESTS:
                        {
                            switch(PhaseCounter)
                            {
                                case 6:
                                    if (Creature* pCaldwell = GetClosestCreatureWithEntry(me, NPC_CALDWELL, 15.0f))
                                        DoScriptText(SAY_OGR_CAL_FOUND, pCaldwell);
                                    break;
                                case 7:
                                    if (Creature* pCaldwell = GetClosestCreatureWithEntry(me, NPC_CALDWELL, 15.0f))
                                        DoScriptText(SAY_OGR_CAL_MERCY, pCaldwell);
                                    break;
                                case 8:
                                    if (Creature* pHallan = GetClosestCreatureWithEntry(me, NPC_HALLAN, 15.0f))
                                    {
                                        DoScriptText(SAY_OGR_HALL_GLAD, pHallan);

                                        if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                            pHallan->CastSpell(pReethe, SPELL_FAKE_SHOT, false);
                                    }
                                    break;
                                case 9:
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                    {
                                        DoScriptText(EMOTE_OGR_RET_ARROW, pReethe);
                                        DoScriptText(SAY_OGR_RET_ARROW, pReethe);
                                    }
                                    break;
                                case 10:
                                    if (Creature* pCaldwell = GetClosestCreatureWithEntry(me, NPC_CALDWELL, 15.0f))
                                        DoScriptText(SAY_OGR_CAL_CLEANUP, pCaldwell);

                                    DoScriptText(SAY_OGR_NODIE, me);
                                    break;
                                case 11:
                                    DoStartAttackMe();
                                    Phase = PHASE_COMPLETE;
                                    break;
                            }
                            break;
                        }

                        case PHASE_COMPLETE:
                        {
                            switch(PhaseCounter)
                            {
                                case 12:
                                    if (Player* pPlayer = GetPlayerForEscort())
                                        pPlayer->GroupEventHappens(QUEST_QUESTIONING, me);
                                    DoScriptText(SAY_OGR_SURVIVE, me);
                                    break;
                                case 13:
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                        DoScriptText(SAY_OGR_RET_LUCKY, pReethe);
                                    break;
                                case 14:
                                    if (Creature* pReethe = GetClosestCreatureWithEntry(me, NPC_REETHE, 15.0f))
                                        pReethe->setDeathState(JUST_DIED);
                                    break;
                                case 15:
                                    DoScriptText(SAY_OGR_THANKS, me);
                                    SetRun(true);
                                    SetEscortPaused(false);
                                    break;
                            }
                            break;
                        }
                    }
                        ++PhaseCounter;
                }
            }

            return;
        }

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_ogron(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_QUESTIONING)
    {
        pCreature->setFaction(FACTION_ESCORT_N_FRIEND_PASSIVE);
        DoScriptText(SAY_OGR_START, pCreature, pPlayer);

        if (npc_ogronAI* pEscortAI = CAST_AI(npc_ogronAI, (pCreature->AI())))
            pEscortAI->Start(false, false, pPlayer->GetGUID(), pQuest, true);
    }

    return true;
}

CreatureAI* GetAI_npc_ogron(Creature* pCreature)
{
    return new npc_ogronAI(pCreature);
}

/*######
## npc_private_hendel
######*/

enum eHendel
{
    SAY_PROGRESS_1_TER          = -1600413,
    SAY_PROGRESS_2_HEN          = -1600414,
    SAY_PROGRESS_3_TER          = -1600415,
    SAY_PROGRESS_4_TER          = -1600416,
    EMOTE_SURRENDER             = -1600417,

    QUEST_MISSING_DIPLO_PT16    = 1324,
    FACTION_HOSTILE             = 168,

    NPC_SENTRY                  = 5184,
    NPC_JAINA                   = 4968,
    NPC_TERVOSH                 = 4967,
    NPC_PAINED                  = 4965,

    PHASE_ATTACK                = 1,
    PHASE_COMPLETED             = 2
};

struct EventLocation
{
    float m_fX, m_fY, m_fZ;
};

EventLocation m_afEventMoveTo[] =
{
    {-2943.92f, -3319.41f, 29.8336f},
    {-2933.01f, -3321.05f, 29.5781f}

};

struct npc_private_hendelAI : public ScriptedAI
{
    npc_private_hendelAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 PhaseCounter;
    Timer m_uiEventTimer;
    uint32 Phase;
    uint64 PlayerGUID;

    void Reset()
    {
        PlayerGUID = 0;
        Phase = 0;
        m_uiEventTimer.Reset(0);
        PhaseCounter = 0;
    }

    void AttackedBy(Unit* pAttacker)
    {
        if (me->GetVictim())
            return;

        if (me->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->SetWalk(false);

        if (pSummoned->GetEntry() == NPC_TERVOSH)
        {
            pSummoned->GetMotionMaster()->MovePoint(0, -2889.48f, -3349.37f, 32.0619f);
            return;
        }
        if (pSummoned->GetEntry() == NPC_JAINA)
        {
            pSummoned->GetMotionMaster()->MovePoint(0, -2889.27f, -3347.17f, 32.2615f);
            return;
        }
        pSummoned->GetMotionMaster()->MovePoint(0, -2890.31f,-3345.23f,32.3087f);
    }

    void DoAttackPlayer()
    {
        Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID);
        if(!pPlayer)
            return;

        me->setFaction(FACTION_HOSTILE);
        me->AI()->AttackStart(pPlayer);

        std::list<Creature*> lCreatureListPtr = FindAllCreaturesWithEntry(NPC_SENTRY, 20);

        for(std::list<Creature*>::iterator itr = lCreatureListPtr.begin(); itr != lCreatureListPtr.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                (*itr)->setFaction(FACTION_HOSTILE);
                (*itr)->AI()->AttackStart(pPlayer);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() && Phase == PHASE_ATTACK)
        {
            DoAttackPlayer();
            return;
        }
        if (Phase == PHASE_COMPLETE)
        {
            if (m_uiEventTimer.Expired(diff))
            {
                m_uiEventTimer = 5000;

                switch (PhaseCounter)
                {
                case 0:
                    DoScriptText(EMOTE_SURRENDER, me);
                    break;
                case 1:
                    if (Creature* pTervosh = GetClosestCreatureWithEntry(me, NPC_TERVOSH, 10.0f))
                        DoScriptText(SAY_PROGRESS_1_TER, pTervosh);
                    break;
                case 2:
                    DoScriptText(SAY_PROGRESS_2_HEN, me);
                    break;
                case 3:
                    if (Creature* pTervosh = GetClosestCreatureWithEntry(me, NPC_TERVOSH, 10.0f))
                        DoScriptText(SAY_PROGRESS_3_TER, pTervosh);
                    break;
                case 4:
                    if (Creature* pTervosh = GetClosestCreatureWithEntry(me, NPC_TERVOSH, 10.0f))
                        DoScriptText(SAY_PROGRESS_4_TER, pTervosh);
                    if (Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID))
                        pPlayer->GroupEventHappens(QUEST_MISSING_DIPLO_PT16, me);
                    Reset();
                    break;
                }
                ++PhaseCounter;
            }
            return;
        }
        DoMeleeAttackIfReady();
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (uiDamage > me->GetHealth() || ((me->GetHealth() - uiDamage)*100 / me->GetMaxHealth() < 20))
        {
            uiDamage = 0;
            Phase = PHASE_COMPLETE;
            m_uiEventTimer = 2000;

            me->RestoreFaction();
            me->RemoveAllAuras();
            me->DeleteThreatList();
            me->CombatStop(true);
            me->SetWalk(false);
            me->GetMotionMaster()->MovePoint(0, -2892.28f, -3347.81f, 31.8609f);

            if (Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID))
                pPlayer->CombatStop(true);

            std::list<Creature*> lCreatureListPtr = FindAllCreaturesWithEntry(NPC_SENTRY, 20);

            for (std::list<Creature*>::iterator itr = lCreatureListPtr.begin(); itr != lCreatureListPtr.end(); ++itr)
            {
                (*itr)->RestoreFaction();
                (*itr)->AI()->EnterEvadeMode();
            }

            me->ForcedDespawn(60000);
            me->SummonCreature(NPC_TERVOSH, -2876.66f, -3346.96f, 35.6029f, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
            me->SummonCreature(NPC_JAINA, -2876.95f, -3342.78f, 35.6244f, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
            me->SummonCreature(NPC_PAINED, -2877.67f, -3338.63f, 35.2548f, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
        }
    }
};

bool QuestAccept_npc_private_hendel(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_DIPLO_PT16)
    {
        CAST_AI(npc_private_hendelAI, pCreature->AI())->Phase = PHASE_ATTACK;
        CAST_AI(npc_private_hendelAI, pCreature->AI())->PlayerGUID = pPlayer->GetGUID();
    }

    return true;
}

CreatureAI* GetAI_npc_private_hendel(Creature* pCreature)
{
    return new npc_private_hendelAI(pCreature);
}

/*#####
## npc_stinky
#####*/

enum eStinky
{
    QUEST_STINKYS_ESCAPE_H                       = 1270,
    QUEST_STINKYS_ESCAPE_A                       = 1222,
    SAY_QUEST_ACCEPTED                           = -1000612,
    SAY_STAY_1                                   = -1000613,
    SAY_STAY_2                                   = -1000614,
    SAY_STAY_3                                   = -1000615,
    SAY_STAY_4                                   = -1000616,
    SAY_STAY_5                                   = -1000617,
    SAY_STAY_6                                   = -1000618,
    SAY_QUEST_COMPLETE                           = -1000619,
    SAY_ATTACKED_1                               = -1000620,
    EMOTE_DISAPPEAR                              = -1000621
};

struct npc_stinkyAI : public npc_escortAI
{
    npc_stinkyAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    void JustRespawned()
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);

        npc_escortAI::JustRespawned();
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch (i)
        {
        case 7:
            DoScriptText(SAY_STAY_1, me, pPlayer);
            break;
        case 11:
            DoScriptText(SAY_STAY_2, me, pPlayer);
            break;
        case 25:
            DoScriptText(SAY_STAY_3, me, pPlayer);
            break;
        case 26:
            DoScriptText(SAY_STAY_4, me, pPlayer);
            break;
        case 27:
            DoScriptText(SAY_STAY_5, me, pPlayer);
            break;
        case 28:
            DoScriptText(SAY_STAY_6, me, pPlayer);
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            break;
        case 29:
            me->SetStandState(UNIT_STAND_STATE_STAND);
            break;
        case 37:
            DoScriptText(SAY_QUEST_COMPLETE, me, pPlayer);
            SetRun();
            if (pPlayer && pPlayer->GetQuestStatus(QUEST_STINKYS_ESCAPE_H))
                pPlayer->GroupEventHappens(QUEST_STINKYS_ESCAPE_H, me);
            if (pPlayer && pPlayer->GetQuestStatus(QUEST_STINKYS_ESCAPE_A))
                pPlayer->GroupEventHappens(QUEST_STINKYS_ESCAPE_A, me);
            break;
        case 39:
            DoScriptText(EMOTE_DISAPPEAR, me);
            break;
        }
    }

    void EnterCombat(Unit* pWho)
    {
        DoScriptText(SAY_ATTACKED_1, me, pWho);
    }

    void Reset() {}

    void JustDied(Unit* /*pKiller*/)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (HasEscortState(STATE_ESCORT_ESCORTING) && pPlayer)
        {
            if (pPlayer->GetQuestStatus(QUEST_STINKYS_ESCAPE_H))
                pPlayer->FailQuest(QUEST_STINKYS_ESCAPE_H);
            if (pPlayer->GetQuestStatus(QUEST_STINKYS_ESCAPE_A))
                pPlayer->FailQuest(QUEST_STINKYS_ESCAPE_A);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

            if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_stinky(Creature* pCreature)
{
    return new npc_stinkyAI(pCreature);
}

bool QuestAccept_npc_stinky(Player* pPlayer, Creature* pCreature, Quest const *quest)
{
    if (quest->GetQuestId() == QUEST_STINKYS_ESCAPE_H || QUEST_STINKYS_ESCAPE_A)
    {
        if (npc_stinkyAI* pEscortAI = CAST_AI(npc_stinkyAI, pCreature->AI()))
        {
            pCreature->SetReactState(REACT_AGGRESSIVE);
            pCreature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            pCreature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);

            pCreature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
            pCreature->SetStandState(UNIT_STAND_STATE_STAND);
            DoScriptText(SAY_QUEST_ACCEPTED, pCreature);
            pEscortAI->Start(true, false, pPlayer->GetGUID());
        }
    }
    return true;
}

/*###
# npc_captured_raptor
###*/

struct npc_captured_raptorAI : public ScriptedAI
{
    npc_captured_raptorAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    uint32 DespawnTimer;

    void InitializeAI()
    {
        me->LoadPath(23741);
        me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        me->GetMotionMaster()->Initialize();
        DespawnTimer = 60000;
    }

    void UpdateAI(const uint32 diff)
    {
        if(DespawnTimer <= diff)
        {
            me->DisappearAndDie();
            DespawnTimer = 0;
        } else DespawnTimer -= diff;

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_captured_raptor(Creature* pCreature)
{
    return new npc_captured_raptorAI(pCreature);
}

/*###
# npc_quest_raptor_captor
###*/

struct npc_quest_raptor_captorAI : public ScriptedAI
{
    npc_quest_raptor_captorAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    bool EmoteDone;

    void Reset()
    {
       EmoteDone = false;
    }
    
    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == 42325)
        {
            caster->Kill(me);
            ((Player*)caster)->KilledMonster(23727, me->GetGUID());
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(!EmoteDone && HealthBelowPct(20))
        {
            DoTextEmote(-1200394,NULL);
            EmoteDone = true;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_quest_raptor_captor(Creature* pCreature)
{
    return new npc_quest_raptor_captorAI(pCreature);
}

bool GOUse_go_blackhoof_cage(Player *player, GameObject* go)
{
    if (player->GetQuestStatus(11145) == QUEST_STATUS_INCOMPLETE)
    {
        if(Creature* prisoner = GetClosestCreatureWithEntry(go, 23720, 5))
        {
            go->UseDoorOrButton(60);
            prisoner->Say(-1200395, LANG_UNIVERSAL, 0);
            float x,y,z;
            prisoner->GetNearPoint(x, y, z, 0, 15, prisoner->GetOrientation());
            prisoner->GetMotionMaster()->MovePoint(100,x,y,z);
            prisoner->ForcedDespawn(5000);
            player->KilledMonster(23720, 0);
        }
    }
    return true;
}

bool GossipHello_npc_17119(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (player->GetQuestStatus(9437) == QUEST_STATUS_INCOMPLETE && player->GetReqKillOrCastCurrentCount(9437, 17119) < 1)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16308), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(8808, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_17119(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->KilledMonster(17119, creature->GetGUID());
        creature->Say(-1200396, LANG_UNIVERSAL, player->GetGUID());
        creature->GetMotionMaster()->MovePoint(1, -2863.028, -3422.136, 39.351);
        creature->ForcedDespawn(3000);
    }
    return true;
}

struct npc_4979AI : public ScriptedAI
{
    npc_4979AI(Creature *c) : ScriptedAI(c) {}

    Timer YellTimer;
    bool Yell;
    uint8 Phase;

    void Reset()
    {
        YellTimer.Reset(0);
        Yell = false;
        Phase = 0;
    }

    void DoAction(const int32 param)
    {
        if (param == 1)
        {
            me->CastSpell(me, 42725, false);
            me->AddAura(42246, me);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            DoTextEmote(-1200397, 0);
            YellTimer = 4000;
            Yell = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->HasAura(42246) && !me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        if(Yell)
        {
            if(YellTimer.Expired(diff))
            {
                switch(Phase)
                {
                    case 0:
                        switch(urand(0, 4))
                        {
                            case 0:
                                me->Say(-1200398, LANG_UNIVERSAL, 0);
                                break;
                            case 1:
                                me->Say(-1200399, LANG_UNIVERSAL, 0);
                                break;
                            case 2:
                                me->Say(-1200400, LANG_UNIVERSAL, 0);
                                break;
                            case 3:
                                me->Say(-1200401, LANG_UNIVERSAL, 0);
                                break;
                            case 4:
                                me->Say(-1200402, LANG_UNIVERSAL, 0);
                                break;
                            default: break;
                        }
                        Phase = 1;
                        YellTimer = 3000;
                        break;
                    case 1:
                        switch(urand(0, 4))
                        {
                            case 0:
                                me->Say(-1200403, LANG_UNIVERSAL, 0);
                                break;
                            case 1:
                                me->Say(-1200404, LANG_UNIVERSAL, 0);
                                break;
                            case 2:
                                me->Say(-1200405, LANG_UNIVERSAL, 0);
                                break;
                            case 3:
                                me->Say(-1200406, LANG_UNIVERSAL, 0);
                                break;
                            default: break;
                        }
                        Phase = 0;
                        Yell = false;
                        YellTimer = 0;
                        break;
                }
            }
        }
    
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_4979(Creature *_Creature)
{
    return new npc_4979AI (_Creature);
}

bool GossipHello_npc_4979(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(11133) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16309), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(11492, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_4979(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
        _Creature->AI()->DoAction(1);
    }
    return true;
}

/*######
## boss_tethyr
######*/

enum
{
    SPELL_WATER_BOLT            = 42574,
    SPELL_SPOUT_LEFT            = 42581,                // triggers 42584
    SPELL_SPOUT_RIGHT           = 42582,
    SPELL_CANNON_BLAST          = 42578,                // triggers 42576
    SPELL_CANNON_BLAST_DMG      = 42576,

    NPC_TETHYR                  = 23899,
    NPC_THERAMORE_MARKSMAN      = 23900,
    NPC_THERAMORE_CANNON        = 23907,

    GO_COVE_CANNON              = 186432,               // cast 42578
    QUEST_ID_TETHYR             = 11198,

    WORLD_STATE_TETHYR_SHOW     = 3083,
    WORLD_STATE_TETHYR_COUNT    = 3082,

    MAX_MARKSMEN                = 12,
    PHASE_INTRO_T               = 0,
    PHASE_NORMAL                = 1,
    PHASE_SPOUT                 = 2,
};

struct boss_tethyrAI : public Scripted_NoMovementAI
{
    boss_tethyrAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        me->LoadPath(238990);
        // send world states to player summoner
        if (m_creature->IsTemporarySummon())
            summonerGuid = ((TemporarySummon*)m_creature)->GetSummonerGuid();

        if (Player* pPlayer = m_creature->GetPlayerInWorld(summonerGuid))
        {
            pPlayer->SendUpdateWorldState(WORLD_STATE_TETHYR_SHOW, 1);
            pPlayer->SendUpdateWorldState(WORLD_STATE_TETHYR_COUNT, MAX_MARKSMEN);
        }

        m_creature->SetLevitate(true);
        Reset();
    }

    ObjectGuid summonerGuid;

    uint8 Phase;
    uint8 MarksmenKilled;
    uint32 WaterBoltTimer;
    uint32 SpoutEndTimer;

    void Reset()
    {
        Phase           = PHASE_INTRO_T;
        MarksmenKilled  = 0;
        WaterBoltTimer  = 1000;
        SpoutEndTimer   = 7000;
    }

    void JustReachedHome()
    {
        // cleanup
        DoEncounterCleanup();
        m_creature->ForcedDespawn(5000);
    }

    void JustDied(Unit* /*pVictim*/)
    {
        // quest complete and cleanup
        if (Player* pSummoner = m_creature->GetPlayerInWorld(summonerGuid))
            pSummoner->GroupEventHappens(QUEST_ID_TETHYR, m_creature);

        if(Unit* Mills = FindCreature(23905, 100.0, m_creature))
        {
            switch(urand(0,1))
            {
                case 0:
                    ((Creature*)Mills)->Yell(-1200407, 0, 0);
                    break;
                case 1:
                    ((Creature*)Mills)->Yell(-1200408, 0, 0);
                    break;
                default: break;
            }
        }

        // ToDo: trigger some fireworks!
        DoEncounterCleanup();
    }

    void MovementInform(uint32 MotionType, uint32 PointId)
    {
        if (MotionType == WAYPOINT_MOTION_TYPE)
        {
            if(PointId == 1)
                me->SetWalk(false);
            else if(PointId == 10)
            {
                Phase = PHASE_NORMAL;
                // make cannons usable
                std::list<GameObject*> CannonsInRange;
                Hellground::AllGameObjectsWithEntryInGrid go_check(GO_COVE_CANNON);
                Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> go_search(CannonsInRange, go_check);
                Cell::VisitGridObjects(me, go_search, 150.0f);

                for (std::list<GameObject*>::const_iterator itr = CannonsInRange.begin(); itr != CannonsInRange.end(); ++itr)
                    (*itr)->RemoveFlag(GAMEOBJECT_FLAGS, (GO_FLAG_INTERACT_COND | GO_FLAG_NOTSELECTABLE));

                // attack all marksmen
                std::list<Creature*> MarksmenInRange;
                Hellground::AllCreaturesOfEntryInRange check(me, NPC_THERAMORE_MARKSMAN, 150.0f);
                Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(MarksmenInRange, check);
                Cell::VisitGridObjects(me, searcher, 150.0f);

                for (std::list<Creature*>::const_iterator itr = MarksmenInRange.begin(); itr != MarksmenInRange.end(); ++itr)
                {
                    (*itr)->AI()->AttackStart(m_creature);
                    AttackStart(*itr);
                }
                me->LoadPath(0);
                me->GetMotionMaster()->Clear();
                if(Unit* Mills = FindCreature(23905, 100.0, m_creature))
                {
                    ((Creature*)Mills)->Yell(-1200409, 0, 0);
                    ((Creature*)Mills)->Yell(-1200410, 0, 0);
                }
                me->SetRooted(true);
            }
        }
        if (MotionType == POINT_MOTION_TYPE)
        {
            // Spout on cannon point reach
            if (PointId)
            {
                DoCast(m_creature, urand(0, 1) ? SPELL_SPOUT_LEFT : SPELL_SPOUT_RIGHT, false);
                // Remove the target focus
                m_creature->SetSelection(0);
                Phase = PHASE_SPOUT;
            }
        }
    }

    void EnterCombat(Unit* pEnemy)
    {
        if (Phase == PHASE_INTRO)
            return;
        ScriptedAI::EnterCombat(pEnemy);
    }

    void KilledUnit(Unit* pVictim)
    {
        // count the marksmen
        if (pVictim->GetEntry() != NPC_THERAMORE_MARKSMAN)
            return;

        ++MarksmenKilled;

        // update world state
        if (Player* pSummoner = m_creature->GetPlayerInWorld(summonerGuid))
        {
            pSummoner->SendUpdateWorldState(WORLD_STATE_TETHYR_COUNT, MAX_MARKSMEN - MarksmenKilled);

            // fail quest if all marksmen are killed
            if (MarksmenKilled == MAX_MARKSMEN)
            {
                pSummoner->FailQuest(QUEST_ID_TETHYR);
                EnterEvadeMode();
            }
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        // spout on cannon
        if (pCaster->GetEntry() == NPC_THERAMORE_CANNON && pSpell->Id == SPELL_CANNON_BLAST_DMG)
        {
            if (Phase == PHASE_SPOUT)
                return;

            // not all cannons have same distance range
            uint8 uiDistMod = pCaster->GetPositionY() > -4650.0f ? 6 : 5;

            float fX, fY, fZ;
            pCaster->GetNearPoint(fX, fY, fZ, me->GetObjectBoundingRadius(), uiDistMod * 5.0f + pCaster->GetObjectBoundingRadius() + me->GetObjectBoundingRadius(), pCaster->GetOrientationTo(m_creature));
            m_creature->GetMotionMaster()->MovePoint(1, fX, fY, m_creature->GetPositionZ());

            WaterBoltTimer = 10000;
        }
    }

    // function to cleanup the world states and GO flags
    void DoEncounterCleanup()
    {
        // remove world state
        if (Player* pSummoner = m_creature->GetPlayerInWorld(summonerGuid))
            pSummoner->SendUpdateWorldState(WORLD_STATE_TETHYR_SHOW, 0);

        // reset all cannons
        std::list<GameObject*> CannonsInRange;
        Hellground::AllGameObjectsWithEntryInGrid go_check(GO_COVE_CANNON);
        Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> go_search(CannonsInRange, go_check);
        Cell::VisitGridObjects(me, go_search, 150.0f);

        for (std::list<GameObject*>::const_iterator itr = CannonsInRange.begin(); itr != CannonsInRange.end(); ++itr)
            (*itr)->SetFlag(GAMEOBJECT_FLAGS, (GO_FLAG_INTERACT_COND | GO_FLAG_NOTSELECTABLE));

        // despawn all marksmen
        std::list<Creature*> MarksmenInRange;
        Hellground::AllCreaturesOfEntryInRange check(me, NPC_THERAMORE_MARKSMAN, 150.0f);
        Hellground::ObjectListSearcher<Creature, Hellground::AllCreaturesOfEntryInRange> searcher(MarksmenInRange, check);
        Cell::VisitGridObjects(me, searcher, 150.0f);

        for (std::list<Creature*>::const_iterator itr = MarksmenInRange.begin(); itr != MarksmenInRange.end(); ++itr)
            (*itr)->ForcedDespawn(30000);
    }

    // Custom threat management
    bool SelectCustomHostileTarget()
    {
        // Not started combat or evading prevented
        if (!m_creature->IsInCombat() || m_creature->HasAuraType(SPELL_AURA_MOD_TAUNT))
        {
            me->Yell("3", LANG_UNIVERSAL, 0);
            return false;
        }

        // Check if there are still enemies (marksmen) in the threatList
        std::list<HostileReference*>& threatList = m_creature->getThreatManager().getThreatList();
        for (std::list<HostileReference*>::iterator i = threatList.begin(); i!= threatList.end();++i)
        {
            Unit* pUnit = Unit::GetUnit((*m_creature), (*i)->getUnitGuid());
            if(pUnit && pUnit->GetTypeId() == TYPEID_UNIT)
            {
                me->Yell("2", LANG_UNIVERSAL, 0);
                return true;
            }
            if(summonerGuid)
            {
                if (Player* pSummoner = m_creature->GetPlayerInWorld(summonerGuid))
                    me->SetInCombatWith(pSummoner);
            }
        }
        me->Yell("1", LANG_UNIVERSAL, 0);
        EnterEvadeMode();
        return false;
    }

    void UpdateAI(const uint32 diff)
    {
        /*if (!SelectCustomHostileTarget())
            return;
            */

        if (Phase == PHASE_SPOUT)
        {
            if (SpoutEndTimer <= diff)
            {
                // Remove rotation auras
                m_creature->RemoveAurasDueToSpell(SPELL_SPOUT_LEFT);
                m_creature->RemoveAurasDueToSpell(SPELL_SPOUT_RIGHT);

                Phase = PHASE_NORMAL;
                SpoutEndTimer = 7000;
                WaterBoltTimer = 2000;
            }
            else
                SpoutEndTimer -= diff;
        }
        else if (Phase == PHASE_NORMAL)
        {
            if (WaterBoltTimer <= diff)
            {
                if (Unit* pTarget = SelectUnit(SELECT_TARGET_NEAREST, 0, 200, false, 0, 0))
                {
                    DoCast(pTarget, SPELL_WATER_BOLT);
                    // mimic boss turning because of the missing threat system
                    m_creature->SetSelection(pTarget->GetGUID());
                    m_creature->SetInFront(pTarget);

                    WaterBoltTimer = 2000;
                }
            }
            else
                WaterBoltTimer -= diff;
        }
    }
};

CreatureAI* GetAI_boss_tethyr(Creature* pCreature)
{
    boss_tethyrAI* boss_tethyr = new boss_tethyrAI(pCreature);
    return (CreatureAI*)boss_tethyr;
}

bool QuestAccept_npc_23905(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == 11198)
    {
        Unit* tethyr = FindCreature(23899, 100.0, pCreature);
        if(!tethyr)
        {
            pCreature->Yell("Keep moving!", 0, 0);
            pPlayer->SummonCreature(23899, -3900.415, -4754.055, -12.02, 1.24, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000);
            if(Creature* summon1 = pPlayer->SummonCreature(23900, -3924.35, -4656.55, 9.15409, 5.80749, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon1->SetAggroRange(100.0);
                summon1->SetRooted(true);
            }
            if(Creature* summon2 = pPlayer->SummonCreature(23900, -3917.6, -4648.53, 9.32604, 5.56795,  TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon2->SetAggroRange(100.0);
                summon2->SetRooted(true);
            }
            if(Creature* summon3 = pPlayer->SummonCreature(23900, -3904.77, -4635.09, 9.62735, 5.49334, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon3->SetAggroRange(100.0);
                summon3->SetRooted(true);
            }
            if(Creature* summon4 = pPlayer->SummonCreature(23900, -3890.48, -4620.7, 9.55527, 4.99383,  TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon4->SetAggroRange(100.0);
                summon4->SetRooted(true);
            }
            if(Creature* summon5 = pPlayer->SummonCreature(23900, -3865.94, -4617.2, 9.26262, 4.16523,  TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon5->SetAggroRange(100.0);
                summon5->SetRooted(true);
            }
            if(Creature* summon6 = pPlayer->SummonCreature(23900, -3856.59, -4622.45, 9.24753, 3.87856, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon6->SetAggroRange(100.0);
                summon6->SetRooted(true);
            }
            if(Creature* summon7 = pPlayer->SummonCreature(23900, -3834.8, -4645.41, 9.25827, 3.61152,  TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon7->SetAggroRange(100.0);
                summon7->SetRooted(true);
            }
            if(Creature* summon8 = pPlayer->SummonCreature(23900, -3826.61, -4655.32, 9.21484, 3.13243, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon8->SetAggroRange(100.0);
                summon8->SetRooted(true);
            }
            if(Creature* summon9 = pPlayer->SummonCreature(23900, -3830.76, -4673.74, 9.50962, 2.70832, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon9->SetAggroRange(100.0);
                summon9->SetRooted(true);
            }
            if(Creature* summon10 = pPlayer->SummonCreature(23900, -3843.65, -4687.59, 9.6436, 2.43735,  TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon10->SetAggroRange(100.0);
                summon10->SetRooted(true);
            }
            if(Creature* summon11 = pPlayer->SummonCreature(23900, -3851.97, -4697.24, 9.36834, 2.33525, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon11->SetAggroRange(100.0);
                summon11->SetRooted(true);
            }
            if(Creature* summon12 = pPlayer->SummonCreature(23900, -3858.49, -4703.49, 9.17411, 2.33525, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 90000))
            {
                summon12->SetAggroRange(100.0);
                summon12->SetRooted(true);
            }
        }
    }
    return true;
}

void AddSC_dustwallow_marsh()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_cassa_crimsonwing";
    newscript->pGossipHello = &GossipHello_npc_cassa_crimsonwing;
    newscript->pGossipSelect = &GossipSelect_npc_cassa_crimsonwing;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_tethyr";
    newscript->GetAI = &GetAI_boss_tethyr;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23905";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_23905;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mobs_risen_husk_spirit";
    newscript->GetAI = &GetAI_mobs_risen_husk_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_restless_apparition";
    newscript->pGossipHello =   &GossipHello_npc_restless_apparition;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_deserter_agitator";
    newscript->GetAI = &GetAI_npc_deserter_agitator;
    newscript->pGossipHello = &GossipHello_npc_deserter_agitator;
    newscript->pGossipSelect = &GossipSelect_npc_deserter_agitator;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_dustwallow_lady_jaina_proudmoore";
    newscript->GetAI = &GetAI_npc_dustwallow_lady_jaina_proudmoore;
    newscript->pGossipHello = &GossipHello_npc_dustwallow_lady_jaina_proudmoore;
    newscript->pGossipSelect = &GossipSelect_npc_dustwallow_lady_jaina_proudmoore;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_nat_pagle";
    newscript->pGossipHello = &GossipHello_npc_nat_pagle;
    newscript->pGossipSelect = &GossipSelect_npc_nat_pagle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_theramore_combat_dummy";
    newscript->GetAI = &GetAI_npc_theramore_combat_dummy;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_mottled_drywallow_crocolisks";
    newscript->GetAI = &GetAI_mob_mottled_drywallow_crocolisks;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_morokk";
    newscript->GetAI = &GetAI_npc_morokk;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_morokk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ogron";
    newscript->GetAI = &GetAI_npc_ogron;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_ogron;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_private_hendel";
    newscript->GetAI = &GetAI_npc_private_hendel;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_private_hendel;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_stinky";
    newscript->GetAI = &GetAI_npc_stinky;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_stinky;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_captured_raptor";
    newscript->GetAI = &GetAI_npc_captured_raptor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_quest_raptor_captor";
    newscript->GetAI = &GetAI_npc_quest_raptor_captor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_blackhoof_cage";
    newscript->pGOUse = &GOUse_go_blackhoof_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_17119";
    newscript->pGossipHello = &GossipHello_npc_17119;
    newscript->pGossipSelect = &GossipSelect_npc_17119;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_4979";
    newscript->GetAI = &GetAI_npc_4979;
    newscript->pGossipHello = &GossipHello_npc_4979;
    newscript->pGossipSelect = &GossipSelect_npc_4979;
    newscript->RegisterSelf();
}

