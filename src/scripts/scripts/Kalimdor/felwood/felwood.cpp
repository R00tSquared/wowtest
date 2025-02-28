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
SDName: Felwood
SD%Complete: 95
SDComment: Quest support: 4101, 4102, 4506
SDCategory: Felwood
EndScriptData */

#include "precompiled.h"
#include "ObjectMgr.h"
#include "follower_ai.h"
#include "escort_ai.h"

/*######
## npcs_riverbreeze_and_silversky
######*/

#define GOSSIP_ITEM_BEACON  16310

bool GossipHello_npcs_riverbreeze_and_silversky(Player *player, Creature *_Creature)
{
    uint32 eCreature = _Creature->GetEntry();

    if( _Creature->isQuestGiver() )
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( eCreature==9528 )
    {
        if( player->GetQuestRewardStatus(4101) )
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_BEACON), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(2848, _Creature->GetGUID());
        }else if( player->GetTeam()==HORDE )
        player->SEND_GOSSIP_MENU(2845, _Creature->GetGUID());
        else
            player->SEND_GOSSIP_MENU(2844, _Creature->GetGUID());
    }

    if( eCreature==9529 )
    {
        if( player->GetQuestRewardStatus(4102) )
        {
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_BEACON), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(2849, _Creature->GetGUID());
        }else if( player->GetTeam() == ALLIANCE )
        player->SEND_GOSSIP_MENU(2843, _Creature->GetGUID());
        else
            player->SEND_GOSSIP_MENU(2842, _Creature->GetGUID());
    }

    return true;
}

bool GossipSelect_npcs_riverbreeze_and_silversky(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if( action==GOSSIP_ACTION_INFO_DEF+1 )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 15120, false);
    }
    return true;
}

struct npc_ancient_treesAI : public ScriptedAI
{
    npc_ancient_treesAI(Creature* creature) : ScriptedAI(creature) {}

    ObjectGuid PlayerGUID;
    uint32 CheckTimer;
    bool Check;

    void Reset()
    {
        me->SetVisibility(VISIBILITY_OFF);
        CheckTimer = 0;
        Check = false;
    }
    
    void MoveInLineOfSight(Unit *who)
    {
        if(!Check)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (((Player*)who)->GetQuestStatus(7632) == QUEST_STATUS_COMPLETE)
                {
                    if (me->IsWithinDistInMap(((Player *)who), 40))
                    {
                        me->SetVisibility(VISIBILITY_ON);
                        PlayerGUID = who->GetObjectGuid();
                        Check = true;
                        CheckTimer = 60000;
                    }
                }
            }
        }
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (Check)
        {
            if (CheckTimer <= diff)
            {
                if (Player* player = me->GetPlayerInWorld(PlayerGUID))
                {
                    if (!me->IsWithinDistInMap(player, 40))
                    {
                       // me->SetVisibility(VISIBILITY_OFF);
                        Reset();
                    }
                    else
                        CheckTimer = 10000;
                }
            }
            else CheckTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_ancient_trees(Creature* creature)
{
    return new npc_ancient_treesAI(creature);
}

/*********
# QuestName: Corrupted Sabers
# Id: 4506
# Wowhead: http://www.wowhead.com/quest=4506/deprecated-corrupted-sabers
*********/

/*####
# npc_kitten
####*/

enum
{
    EMOTE_SAB_JUMP              = -1000541,
    EMOTE_SAB_FOLLOW            = -1000542,

    SPELL_CORRUPT_SABER_VISUAL  = 16510,

    QUEST_CORRUPT_SABER         = 4506,
    NPC_WINNA                   = 9996,
    NPC_CORRUPT_SABER           = 10042,
    MODEL_ID_CORRUPTED_SABER    = 9230
};

#define GOSSIP_ITEM_RELEASE     16311

struct npc_kittenAI : public FollowerAI
{
    npc_kittenAI(Creature* c) : FollowerAI(c)
    {
        if (c->GetOwner() && c->GetOwner()->GetTypeId() == TYPEID_PLAYER)
        {
            StartFollow((Player*)c->GetOwner());
            SetFollowPaused(true);
            DoScriptText(EMOTE_SAB_JUMP, me);

            c->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        }

        MoonwellCooldown = 7500;
        Reset();
    }

    uint32 MoonwellCooldown;

    void Reset() { }

    void MoveInLineOfSight(Unit* pWho)
    {
        // should not have npcflag by default, so set when expected
        if (!me->GetVictim() && !me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP) && HasFollowState(STATE_FOLLOW_INPROGRESS) && pWho->GetEntry() == NPC_WINNA)
        {
            if (me->IsWithinDistInMap(pWho, INTERACTION_DISTANCE))
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    }

    void UpdateFollowerAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (HasFollowState(STATE_FOLLOW_PAUSED))
            {
                if (MoonwellCooldown < diff)
                {
                    me->CastSpell(me, SPELL_CORRUPT_SABER_VISUAL, false);
                    SetFollowPaused(false);
                }
                else
                    MoonwellCooldown -= diff;
            }
            return;
        }
        else
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kitten(Creature* c)
{
    return new npc_kittenAI(c);
}

bool EffectDummyCreature_npc_kitten(Unit* /*pCaster*/, uint32 spellId, uint32 effIndex, Creature* pCreatureTarget)
{
    if (spellId == SPELL_CORRUPT_SABER_VISUAL && effIndex == 0)
    {
        if (CreatureInfo const* pInfo = GetCreatureTemplateStore(NPC_CORRUPT_SABER))
        {
            pCreatureTarget->SetEntry(NPC_CORRUPT_SABER);
            pCreatureTarget->SetDisplayId(MODEL_ID_CORRUPTED_SABER);
            pCreatureTarget->SetName(pInfo->Name);
            pCreatureTarget->SetFloatValue(OBJECT_FIELD_SCALE_X, pInfo->scale);
        }

        if (Unit* pOwner = pCreatureTarget->GetOwner())
            DoScriptText(EMOTE_SAB_FOLLOW, pCreatureTarget, pOwner);

        return true;
    }
    return false;
}

bool GossipHello_npc_corrupt_saber(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_CORRUPT_SABER) == QUEST_STATUS_INCOMPLETE)
    {
        if (GetClosestCreatureWithEntry(pCreature, NPC_WINNA, INTERACTION_DISTANCE))
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_RELEASE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_corrupt_saber(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        if (npc_kittenAI* pKittenAI = dynamic_cast<npc_kittenAI*>(pCreature->AI()))
            pKittenAI->SetFollowComplete();

        pPlayer->AreaExploredOrEventHappens(QUEST_CORRUPT_SABER);
    }

    return true;
}

/*********
# QuestName: Kroshius' Infernal Core
# Id: 7603
# Wowhead: http://www.wowhead.com/quest=7603
*********/

/*######
## npc_niby_the_almighty (summons el pollo grande)
######*/

enum
{
    QUEST_KROSHIUS     = 7603,

    NPC_IMPSY          = 14470,

    SPELL_SUMMON_POLLO = 23056,

    SAY_NIBY_1         = -1000566,
    SAY_NIBY_2         = -1000567,
    EMOTE_IMPSY_1      = -1000568,
    SAY_IMPSY_1        = -1000569,
    SAY_NIBY_3         = -1000570
};

struct npc_niby_the_almightyAI : public ScriptedAI
{
    npc_niby_the_almightyAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    uint32 SummonTimer;
    uint8  Speech;

    bool EventStarted;

    void Reset()
    {
        SummonTimer = 500;
        Speech = 0;

        EventStarted = false;
    }

    void StartEvent()
    {
        Reset();
        EventStarted = true;
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventStarted)
        {
            if (SummonTimer <= diff)
            {
                switch (Speech)
                {
                    case 1:
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(0, 5407.19f, -753.00f, 350.82f);
                        SummonTimer = 6200;
                        break;
                    case 2:
                        me->SetFacingTo(1.2f);
                        DoScriptText(SAY_NIBY_1, me);
                        SummonTimer = 3000;
                        break;
                    case 3:
                        DoScriptText(SAY_NIBY_2, me);
                        DoCast(me, SPELL_SUMMON_POLLO);
                        SummonTimer = 2000;
                        break;
                    case 4:
                        if (Creature* pImpsy = GetClosestCreatureWithEntry(me, NPC_IMPSY, 20.0))
                        {
                            DoScriptText(EMOTE_IMPSY_1, pImpsy);
                            DoScriptText(SAY_IMPSY_1, pImpsy);
                            SummonTimer = 2500;
                        }
                        else
                        {
                            // Skip Speech 5
                            SummonTimer = 40000;
                            ++Speech;
                        }
                        break;
                    case 5:
                        DoScriptText(SAY_NIBY_3, me);
                        SummonTimer = 40000;
                        break;
                    case 6:
                        me->GetMotionMaster()->MoveTargetedHome();
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        EventStarted = false;
                        break;
                }
                ++Speech;
            }
            else
                SummonTimer -= diff;
        }
    }
};

CreatureAI* GetAI_npc_niby_the_almighty(Creature* pCreature)
{
    return new npc_niby_the_almightyAI(pCreature);
}

bool QuestRewarded_npc_niby_the_almighty(Player* /*pPlayer*/, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_KROSHIUS)
    {
        if (npc_niby_the_almightyAI* pNibyAI = dynamic_cast<npc_niby_the_almightyAI*>(pCreature->AI()))
            pNibyAI->StartEvent();
    }
    return true;
}

/*######
## npc_kroshius
######*/

enum
{
    NPC_KROSHIUS            = 14467,
    SPELL_KNOCKBACK         = 10101,
    SAY_KROSHIUS_REVIVE     = -1000589,
    EVENT_KROSHIUS_REVIVE   = 8328,
    FACTION_HOSTILE         = 16,
};

struct npc_kroshiusAI : public ScriptedAI
{
    npc_kroshiusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
    }

    uint64 playerGuid;
    uint32 KnockBackTimer;
    uint32 PhaseTimer;

    uint8 Phase;

    void Reset()
    {
        Phase = 0;
        KnockBackTimer = urand(5000, 8000);
        playerGuid = 0;

        if (Phase == 0)
        {
            me->setFaction(35);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            me->SetStandState(UNIT_STAND_STATE_DEAD);
        }
    }

    void DoRevive(Player* pSource)
    {
        if (Phase)
            return;

        Phase = 1;
        PhaseTimer = 2500;
        playerGuid = pSource->GetGUID();

        // Release TODO: A visual Flame Circle around the mob
    }

    void JustDied(Unit* /*pKiller*/)
    {
        Phase = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!Phase)
            return;

        if (Phase < 4)
        {
            if (PhaseTimer < diff)
            {
                switch (Phase)
                {
                    case 1:                                 // Revived
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        PhaseTimer = 1000;
                        break;
                    case 2:
                        DoScriptText(SAY_KROSHIUS_REVIVE, me);
                        PhaseTimer = 3500;
                        break;
                    case 3:                                 // Attack
                        me->setFaction(FACTION_HOSTILE);
                        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                        if (Player* pPlayer = Unit::GetPlayerInWorld(playerGuid))
                        {
                            if (me->IsWithinDistInMap(pPlayer, 30.0f))
                                AttackStart(pPlayer);
                        }
                        break;
                }
                ++Phase;
            }
            else
                PhaseTimer -= diff;
        }
        else
        {
            if (!UpdateVictim())
                return;

            if (KnockBackTimer < diff)
            {
                DoCast(me->GetVictim(), SPELL_KNOCKBACK);
                KnockBackTimer = urand(9000, 12000);
            }
            else
                KnockBackTimer -= diff;

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_npc_kroshius(Creature* pCreature)
{
    return new npc_kroshiusAI(pCreature);
}

bool ProcessEventId_npc_kroshius(uint32 uiEventId, Object* pSource, Object* /*pTarget*/, bool /*bIsStart*/)
{
    if (uiEventId == EVENT_KROSHIUS_REVIVE)
    {
        if (pSource->GetTypeId() == TYPEID_PLAYER)
        {
            if (Creature* pKroshius = GetClosestCreatureWithEntry((Player*)pSource, NPC_KROSHIUS, 20.0f))
            {
                if (npc_kroshiusAI* pKroshiusAI = dynamic_cast<npc_kroshiusAI*>(pKroshius->AI()))
                    pKroshiusAI->DoRevive((Player*)pSource);
            }
        }

        return true;
    }
    return false;
}

/*********
# QuestName: Rescue From Jaedenar
# Id: 5203
# Wowhead: http://www.wowhead.com/quest=5203
*********/

/*####
# npc_captured_arkonarin
####*/

enum
{
    SAY_ESCORT_START                = -1001148,
    SAY_FIRST_STOP                  = -1001149,
    SAY_SECOND_STOP                 = -1001150,
    SAY_AGGRO                       = -1001151,
    SAY_FOUND_EQUIPMENT             = -1001152,
    SAY_ESCAPE_DEMONS               = -1001153,
    SAY_FRESH_AIR                   = -1001154,
    SAY_TREY_BETRAYER               = -1001155,
    SAY_TREY                        = -1001156,
    SAY_TREY_ATTACK                 = -1001157,
    SAY_ESCORT_COMPLETE             = -1001158,

    SPELL_STRENGTH_ARKONARIN        = 18163,
    SPELL_MORTAL_STRIKE             = 16856,
    SPELL_CLEAVE                    = 15496,

    QUEST_ID_RESCUE_JAEDENAR        = 5203,
    NPC_JAEDENAR_LEGIONNAIRE        = 9862,
    NPC_SPIRT_TREY                  = 11141,
    NPC_ARKONARIN                   = 11018,
    GO_ARKONARIN_CHEST              = 176225,
    GO_ARKONARIN_CAGE               = 176306,
};

struct npc_captured_arkonarinAI : public npc_escortAI
{
    npc_captured_arkonarinAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint64 treyGuid;

    bool CanAttack;

    uint32 MortalStrikeTimer;
    uint32 CleaveTimer;

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            CanAttack = false;

        MortalStrikeTimer = urand(5000, 7000);
        CleaveTimer = urand(1000, 4000);
    }

    void EnterCombat(Unit* pWho)
    {
        if (pWho->GetEntry() == NPC_SPIRT_TREY)
            DoScriptText(SAY_TREY_ATTACK, me);
        else if (roll_chance_i(25))
            DoScriptText(SAY_AGGRO, me, pWho);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_JAEDENAR_LEGIONNAIRE)
            pSummoned->AI()->AttackStart(me);
        else if (pSummoned->GetEntry() == NPC_SPIRT_TREY)
        {
            DoScriptText(SAY_TREY_BETRAYER, pSummoned);
            treyGuid = pSummoned->GetGUID();
        }
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch (uiPointId)
        {
            case 0:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_ESCORT_START, me, pPlayer);
                break;
            case 14:
                DoScriptText(SAY_FIRST_STOP, me);
                break;
            case 34:
                DoScriptText(SAY_SECOND_STOP, me);
                SetRun();
                break;
            case 38:
                if (GameObject* pChest = GetClosestGameObjectWithEntry(me, GO_ARKONARIN_CHEST, 5.0f))
                    pChest->Use(me);
                me->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
                break;
            case 39:
                DoCast(me, SPELL_STRENGTH_ARKONARIN);
                break;
            case 40:
                me->UpdateEntry(NPC_ARKONARIN);
                me->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE); // we need to reset it here, `cause updateentry break faction
                if (Player* pPlayer = GetPlayerForEscort())
                    me->SetFacingToObject(pPlayer);
                CanAttack = true;
                DoScriptText(SAY_FOUND_EQUIPMENT, me);
                break;
            case 41:
                DoScriptText(SAY_ESCAPE_DEMONS, me);
                me->SummonCreature(NPC_JAEDENAR_LEGIONNAIRE, 5082.068f, -490.084f, 296.856f, 5.15f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
                me->SummonCreature(NPC_JAEDENAR_LEGIONNAIRE, 5084.135f, -489.187f, 296.832f, 5.15f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
                me->SummonCreature(NPC_JAEDENAR_LEGIONNAIRE, 5085.676f, -488.518f, 296.824f, 5.15f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
                break;
            case 43:
                SetRun(false);
                break;
            case 104:
                DoScriptText(SAY_FRESH_AIR, me);
                break;
            case 105:
                me->SummonCreature(NPC_SPIRT_TREY, 4844.839f, -395.763f, 350.603f, 6.25f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
                break;
            case 106:
                DoScriptText(SAY_TREY, me);
                break;
            case 107:
                if (Creature* pTrey = me->GetMap()->GetCreature(treyGuid))
                    AttackStart(pTrey);
                break;
            case 108:
                if (Player* pPlayer = GetPlayerForEscort())
                    me->SetFacingToObject(pPlayer);
                DoScriptText(SAY_ESCORT_COMPLETE, me);
                break;
            case 109:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_ID_RESCUE_JAEDENAR, me);
                SetRun();
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;

        if (CanAttack)
        {
            if (MortalStrikeTimer < uiDiff)
            {
                DoCast(me->GetVictim(), SPELL_MORTAL_STRIKE);
                MortalStrikeTimer = urand(7000, 10000);
            }
            else
                MortalStrikeTimer -= uiDiff;

            if (CleaveTimer < uiDiff)
            {
                DoCast(me->GetVictim(), SPELL_CLEAVE);
                CleaveTimer = urand(3000, 6000);
            }
            else
                CleaveTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captured_arkonarin(Creature* pCreature)
{
    npc_captured_arkonarinAI* captured_arkonarin = new npc_captured_arkonarinAI(pCreature);
    
    captured_arkonarin->AddWaypoint(0, 5004.985, -440.237, 319.059, 4000);
    captured_arkonarin->AddWaypoint(1, 4992.224, -449.964, 317.057);
    captured_arkonarin->AddWaypoint(2, 4988.549, -457.438, 316.289);
    captured_arkonarin->AddWaypoint(3, 4989.978, -464.297, 316.846);
    captured_arkonarin->AddWaypoint(4, 4994.038, -467.754, 318.055);
    captured_arkonarin->AddWaypoint(5, 5002.307, -466.318, 319.965);
    captured_arkonarin->AddWaypoint(6, 5011.801, -462.889, 321.501);
    captured_arkonarin->AddWaypoint(7, 5020.533, -460.797, 321.970);
    captured_arkonarin->AddWaypoint(8, 5026.836, -463.171, 321.345);
    captured_arkonarin->AddWaypoint(9, 5028.663, -476.805, 318.726);
    captured_arkonarin->AddWaypoint(10, 5029.503, -487.131, 318.179);
    captured_arkonarin->AddWaypoint(11, 5031.178, -497.678, 316.533);
    captured_arkonarin->AddWaypoint(12, 5032.720, -504.748, 314.744);
    captured_arkonarin->AddWaypoint(13, 5034.997, -513.138, 314.372);
    captured_arkonarin->AddWaypoint(14, 5037.493, -521.733, 313.221, 6000);
    captured_arkonarin->AddWaypoint(15, 5049.055, -519.546, 313.221);
    captured_arkonarin->AddWaypoint(16, 5059.170, -522.930, 313.221);
    captured_arkonarin->AddWaypoint(17, 5062.755, -529.933, 313.221);
    captured_arkonarin->AddWaypoint(18, 5063.896, -538.827, 313.221);
    captured_arkonarin->AddWaypoint(19, 5062.223, -545.635, 313.221);
    captured_arkonarin->AddWaypoint(20, 5061.690, -552.333, 313.101);
    captured_arkonarin->AddWaypoint(21, 5060.333, -560.349, 310.873);
    captured_arkonarin->AddWaypoint(22, 5055.621, -565.541, 308.737);
    captured_arkonarin->AddWaypoint(23, 5049.803, -567.604, 306.537);
    captured_arkonarin->AddWaypoint(24, 5043.011, -564.946, 303.682);
    captured_arkonarin->AddWaypoint(25, 5038.221, -559.823, 301.463);
    captured_arkonarin->AddWaypoint(26, 5039.456, -548.675, 297.824);
    captured_arkonarin->AddWaypoint(27, 5043.437, -538.807, 297.801);
    captured_arkonarin->AddWaypoint(28, 5056.397, -528.954, 297.801);
    captured_arkonarin->AddWaypoint(29, 5064.397, -521.904, 297.801);
    captured_arkonarin->AddWaypoint(30, 5067.616, -512.999, 297.196);
    captured_arkonarin->AddWaypoint(31, 5065.990, -505.329, 297.214);
    captured_arkonarin->AddWaypoint(32, 5062.238, -499.086, 297.448);
    captured_arkonarin->AddWaypoint(33, 5065.087, -492.069, 298.054);
    captured_arkonarin->AddWaypoint(34, 5071.195, -491.173, 297.666, 5000);
    captured_arkonarin->AddWaypoint(35, 5087.474, -496.478, 296.677);
    captured_arkonarin->AddWaypoint(36, 5095.552, -508.639, 296.677);
    captured_arkonarin->AddWaypoint(37, 5104.300, -521.014, 296.677);
    captured_arkonarin->AddWaypoint(38, 5110.132, -532.123, 296.677, 4000);
    captured_arkonarin->AddWaypoint(39, 5110.132, -532.123, 296.677, 4000);
    captured_arkonarin->AddWaypoint(40, 5110.132, -532.123, 296.677, 4000);
    captured_arkonarin->AddWaypoint(41, 5110.132, -532.123, 296.677);
    captured_arkonarin->AddWaypoint(42, 5099.748, -510.823, 296.677);
    captured_arkonarin->AddWaypoint(43, 5091.944, -497.516, 296.677);
    captured_arkonarin->AddWaypoint(44, 5079.375, -486.811, 297.638);
    captured_arkonarin->AddWaypoint(45, 5069.212, -488.770, 298.082);
    captured_arkonarin->AddWaypoint(46, 5064.242, -496.051, 297.275);
    captured_arkonarin->AddWaypoint(47, 5065.084, -505.239, 297.361);
    captured_arkonarin->AddWaypoint(48, 5067.818, -515.245, 297.125);
    captured_arkonarin->AddWaypoint(49, 5064.617, -521.170, 297.801);
    captured_arkonarin->AddWaypoint(50, 5053.221, -530.739, 297.801);
    captured_arkonarin->AddWaypoint(51, 5045.725, -538.311, 297.801);
    captured_arkonarin->AddWaypoint(52, 5039.695, -548.112, 297.801);
    captured_arkonarin->AddWaypoint(53, 5038.778, -557.588, 300.787);
    captured_arkonarin->AddWaypoint(54, 5042.014, -566.749, 303.838);
    captured_arkonarin->AddWaypoint(55, 5050.555, -568.149, 306.782);
    captured_arkonarin->AddWaypoint(56, 5056.979, -564.674, 309.342);
    captured_arkonarin->AddWaypoint(57, 5060.791, -556.801, 311.936);
    captured_arkonarin->AddWaypoint(58, 5059.581, -551.626, 313.221);
    captured_arkonarin->AddWaypoint(59, 5062.826, -541.994, 313.221);
    captured_arkonarin->AddWaypoint(60, 5063.554, -531.288, 313.221);
    captured_arkonarin->AddWaypoint(61, 5057.934, -523.088, 313.221);
    captured_arkonarin->AddWaypoint(62, 5049.471, -519.360, 313.221);
    captured_arkonarin->AddWaypoint(63, 5040.789, -519.809, 313.221);
    captured_arkonarin->AddWaypoint(64, 5034.299, -515.361, 313.948);
    captured_arkonarin->AddWaypoint(65, 5032.001, -505.532, 314.663);
    captured_arkonarin->AddWaypoint(66, 5029.915, -495.645, 316.821);
    captured_arkonarin->AddWaypoint(67, 5028.871, -487.000, 318.179);
    captured_arkonarin->AddWaypoint(68, 5028.109, -475.531, 318.839);
    captured_arkonarin->AddWaypoint(69, 5027.759, -465.442, 320.643);
    captured_arkonarin->AddWaypoint(70, 5019.955, -460.892, 321.969);
    captured_arkonarin->AddWaypoint(71, 5009.426, -464.793, 321.248);
    captured_arkonarin->AddWaypoint(72, 4999.567, -468.062, 319.426);
    captured_arkonarin->AddWaypoint(73, 4992.034, -468.128, 317.894);
    captured_arkonarin->AddWaypoint(74, 4988.168, -461.293, 316.369);
    captured_arkonarin->AddWaypoint(75, 4990.624, -447.459, 317.104);
    captured_arkonarin->AddWaypoint(76, 4993.475, -438.643, 318.272);
    captured_arkonarin->AddWaypoint(77, 4995.451, -430.178, 318.462);
    captured_arkonarin->AddWaypoint(78, 4993.564, -422.876, 318.864);
    captured_arkonarin->AddWaypoint(79, 4985.401, -420.864, 320.205);
    captured_arkonarin->AddWaypoint(80, 4976.515, -426.168, 323.112);
    captured_arkonarin->AddWaypoint(81, 4969.832, -429.755, 325.029);
    captured_arkonarin->AddWaypoint(82, 4960.702, -425.440, 325.834);
    captured_arkonarin->AddWaypoint(83, 4955.447, -418.765, 327.433);
    captured_arkonarin->AddWaypoint(84, 4949.702, -408.796, 328.004);
    captured_arkonarin->AddWaypoint(85, 4940.017, -403.222, 329.956);
    captured_arkonarin->AddWaypoint(86, 4934.982, -401.475, 330.898);
    captured_arkonarin->AddWaypoint(87, 4928.693, -399.302, 331.744);
    captured_arkonarin->AddWaypoint(88, 4926.935, -398.436, 333.079);
    captured_arkonarin->AddWaypoint(89, 4916.163, -393.822, 333.729);
    captured_arkonarin->AddWaypoint(90, 4908.393, -396.217, 333.217);
    captured_arkonarin->AddWaypoint(91, 4905.610, -396.535, 335.050);
    captured_arkonarin->AddWaypoint(92, 4897.876, -395.245, 337.346);
    captured_arkonarin->AddWaypoint(93, 4895.206, -388.203, 339.295);
    captured_arkonarin->AddWaypoint(94, 4896.945, -382.429, 341.040);
    captured_arkonarin->AddWaypoint(95, 4901.885, -378.799, 342.771);
    captured_arkonarin->AddWaypoint(96, 4908.087, -380.635, 344.597);
    captured_arkonarin->AddWaypoint(97, 4911.910, -385.818, 346.491);
    captured_arkonarin->AddWaypoint(98, 4910.104, -393.444, 348.798);
    captured_arkonarin->AddWaypoint(99, 4903.500, -396.947, 350.812);
    captured_arkonarin->AddWaypoint(100, 4898.083, -394.226, 351.821);
    captured_arkonarin->AddWaypoint(101, 4891.333, -393.436, 351.801);
    captured_arkonarin->AddWaypoint(102, 4881.203, -395.211, 351.590);
    captured_arkonarin->AddWaypoint(103, 4877.843, -395.536, 349.713);
    captured_arkonarin->AddWaypoint(104, 4873.972, -394.919, 349.844, 5000);
    captured_arkonarin->AddWaypoint(105, 4873.972, -394.919, 349.844, 3000);
    captured_arkonarin->AddWaypoint(106, 4873.972, -394.919, 349.844, 2000);
    captured_arkonarin->AddWaypoint(107, 4873.972, -394.919, 349.844);
    captured_arkonarin->AddWaypoint(108, 4873.972, -394.919, 349.844, 5000);
    captured_arkonarin->AddWaypoint(109, 4873.972, -394.919, 349.844, 1000);
    captured_arkonarin->AddWaypoint(110, 4863.016, -394.521, 350.650);
    captured_arkonarin->AddWaypoint(111, 4848.696, -397.612, 351.215);
    return (CreatureAI*)captured_arkonarin;
}

bool QuestAccept_npc_captured_arkonarin(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_RESCUE_JAEDENAR)
    {
        if (npc_captured_arkonarinAI* pEscortAI = CAST_AI(npc_captured_arkonarinAI, pCreature->AI()))
        {
            pCreature->SetStandState(UNIT_STAND_STATE_STAND);
            pCreature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest, true);

            if (GameObject* pCage = GetClosestGameObjectWithEntry(pCreature, GO_ARKONARIN_CAGE, 5.0f))
                pCage->Use(pCreature);
        }
    }
    return true;
}

bool GossipHello_npc_11555(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    bool canBuy;
    canBuy = false;

    if (player->GetReputationMgr().GetRank(576) <= REP_HONORED)
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    else canBuy = true;

    if (canBuy)
    {
        if (_Creature->isVendor())
            player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_11555(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*####
# npc_arei
####*/

enum
{
    SAY_AREI_ESCORT_START           = -1001159,
    SAY_ATTACK_IRONTREE             = -1001160,
    SAY_ATTACK_TOXIC_HORROR         = -1001161,
    SAY_EXIT_WOODS                  = -1001162,
    SAY_CLEAR_PATH                  = -1001163,
    SAY_ASHENVALE                   = -1001164,
    SAY_TRANSFORM                   = -1001165,
    SAY_LIFT_CURSE                  = -1001166,
    SAY_AREI_ESCORT_COMPLETE        = -1001167,

    SPELL_WITHER_STRIKE             = 5337,
    SPELL_AREI_TRANSFORM            = 14888,

    NPC_AREI                        = 9598,
    NPC_TOXIC_HORROR                = 7132,
    NPC_IRONTREE_WANDERER           = 7138,
    NPC_IRONTREE_STOMPER            = 7139,
    QUEST_ID_ANCIENT_SPIRIT         = 4261,
};

static const DialogueEntry aEpilogDialogue[] =
{
    {SAY_CLEAR_PATH,            NPC_AREI,   4000},
    {SPELL_WITHER_STRIKE,       0,          5000},
    {SAY_TRANSFORM,             NPC_AREI,   3000},
    {SPELL_AREI_TRANSFORM,      0,          7000},
    {QUEST_ID_ANCIENT_SPIRIT,   0,          0},
    {0, 0, 0},
};

struct npc_areiAI : public npc_escortAI, private DialogueHelper
{
    npc_areiAI(Creature* pCreature) : npc_escortAI(pCreature),
        DialogueHelper(aEpilogDialogue)
    {
        AggroIrontree = false;
        AggroHorror = false;
        Reset();
    }

    Timer WitherStrikeTimer;

    bool AggroIrontree;
    bool AggroHorror;

    std::list<uint64> SummonsGuids;

    void Reset()
    {
        WitherStrikeTimer.Reset(urand(1000, 4000));
    }

    void EnterCombat(Unit* pWho)
    {
        if ((pWho->GetEntry() == NPC_IRONTREE_WANDERER || pWho->GetEntry() == NPC_IRONTREE_STOMPER) && !AggroIrontree)
        {
            DoScriptText(SAY_ATTACK_IRONTREE, me);
            AggroIrontree = true;
        }
        else if (pWho->GetEntry() == NPC_TOXIC_HORROR && ! AggroHorror)
        {
            if (Player* pPlayer = GetPlayerForEscort())
                DoScriptText(SAY_ATTACK_TOXIC_HORROR, me, pPlayer);
            AggroHorror = true;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_IRONTREE_STOMPER:
                DoScriptText(SAY_EXIT_WOODS, me, pSummoned);
            // no break;
            case NPC_IRONTREE_WANDERER:
                pSummoned->AI()->AttackStart(me);
                SummonsGuids.push_back(pSummoned->GetGUID());
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_IRONTREE_STOMPER || pSummoned->GetEntry() == NPC_IRONTREE_WANDERER)
        {
            SummonsGuids.remove(pSummoned->GetGUID());

            if (SummonsGuids.empty())
                StartNextDialogueText(SAY_CLEAR_PATH);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue)
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            DoScriptText(SAY_AREI_ESCORT_START, me, pInvoker);

            me->setFaction(FACTION_ESCORT_N_NEUTRAL_PASSIVE);
            Start(true, true, pInvoker->GetGUID(), GetQuestTemplateStore(uiMiscValue));
        }
    }

    void WaypointReached(uint32 uiPointId)
    {
        if (uiPointId == 36)
        {
            SetEscortPaused(true);

            me->SummonCreature(NPC_IRONTREE_STOMPER, 6573.321f, -1195.213f, 442.489f, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
            me->SummonCreature(NPC_IRONTREE_WANDERER, 6573.240f, -1213.475f, 443.643f, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
            me->SummonCreature(NPC_IRONTREE_WANDERER, 6583.354f, -1209.811f, 444.769f, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry)
    {
        if (uiEntry == NPC_AREI)
            return me;

        return NULL;
    }

    void JustDidDialogueStep(int32 iEntry)
    {
        switch (iEntry)
        {
            case SPELL_WITHER_STRIKE:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_ASHENVALE, me, pPlayer);
                break;
            case SPELL_AREI_TRANSFORM:
                DoCast(me, SPELL_AREI_TRANSFORM);
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_LIFT_CURSE, me, pPlayer);
                break;
            case QUEST_ID_ANCIENT_SPIRIT:
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_AREI_ESCORT_COMPLETE, me, pPlayer);
                    pPlayer->RewardPlayerAndGroupAtEvent(QUEST_ID_ANCIENT_SPIRIT, me);
                    me->ForcedDespawn(10000);
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        DialogueUpdate(diff);

        if (!UpdateVictim())
            return;

        if (WitherStrikeTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_WITHER_STRIKE);
            WitherStrikeTimer = urand(3000, 6000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_arei(Creature* pCreature)
{
    npc_areiAI* npc_arei = new npc_areiAI(pCreature);
    npc_arei->AddWaypoint(0, 6004.265, -1180.494, 376.377, 0),
    npc_arei->AddWaypoint(1, 6002.512, -1157.294, 381.407, 0),
    npc_arei->AddWaypoint(2, 6029.228, -1139.720, 383.127, 0),
    npc_arei->AddWaypoint(3, 6042.479, -1128.963, 386.582, 0),
    npc_arei->AddWaypoint(4, 6062.820, -1115.522, 386.850, 0),
    npc_arei->AddWaypoint(5, 6089.188, -1111.907, 383.105, 0),
    npc_arei->AddWaypoint(6, 6104.390, -1114.561, 380.490, 0),
    npc_arei->AddWaypoint(7, 6115.080, -1128.572, 375.779, 0),
    npc_arei->AddWaypoint(8, 6119.352, -1147.314, 372.518, 0),
    npc_arei->AddWaypoint(9, 6119.003, -1176.082, 371.072, 0),
    npc_arei->AddWaypoint(10, 6120.982, -1198.408, 373.432, 0),
    npc_arei->AddWaypoint(11, 6123.521, -1226.636, 374.119, 0),
    npc_arei->AddWaypoint(12, 6127.737, -1246.035, 373.574, 0),
    npc_arei->AddWaypoint(13, 6133.433, -1253.642, 369.100, 0),
    npc_arei->AddWaypoint(14, 6150.787, -1269.151, 369.240, 0),
    npc_arei->AddWaypoint(15, 6155.805, -1275.029, 373.627, 0),
    npc_arei->AddWaypoint(16, 6163.544, -1307.130, 376.234, 0),
    npc_arei->AddWaypoint(17, 6174.800, -1340.885, 379.039, 0),
    npc_arei->AddWaypoint(18, 6191.144, -1371.260, 378.453, 0),
    npc_arei->AddWaypoint(19, 6215.652, -1397.517, 376.012, 0),
    npc_arei->AddWaypoint(20, 6243.784, -1407.675, 371.594, 0),
    npc_arei->AddWaypoint(21, 6280.775, -1408.259, 370.554, 0),
    npc_arei->AddWaypoint(22, 6325.360, -1406.688, 370.082, 0),
    npc_arei->AddWaypoint(23, 6355.000, -1404.337, 370.646, 0),
    npc_arei->AddWaypoint(24, 6374.679, -1399.176, 372.105, 0),
    npc_arei->AddWaypoint(25, 6395.803, -1367.057, 374.910, 0),
    npc_arei->AddWaypoint(26, 6408.569, -1333.487, 376.616, 0),
    npc_arei->AddWaypoint(27, 6409.075, -1312.168, 379.598, 0),
    npc_arei->AddWaypoint(28, 6418.689, -1277.697, 381.638, 0),
    npc_arei->AddWaypoint(29, 6441.689, -1247.316, 387.246, 0),
    npc_arei->AddWaypoint(30, 6462.136, -1226.316, 397.610, 0),
    npc_arei->AddWaypoint(31, 6478.021, -1216.260, 408.284, 0),
    npc_arei->AddWaypoint(32, 6499.632, -1217.087, 419.461, 0),
    npc_arei->AddWaypoint(33, 6523.165, -1220.780, 430.549, 0),
    npc_arei->AddWaypoint(34, 6542.716, -1216.997, 437.788, 0),
    npc_arei->AddWaypoint(35, 6557.279, -1211.125, 441.452, 0),
    npc_arei->AddWaypoint(36, 6574.568, -1204.589, 443.216, 0);
    return (CreatureAI*)npc_arei;
}

bool QuestAccept_npc_arei(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_ANCIENT_SPIRIT)
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());

    return true;
}

void AddSC_felwood()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_arei";
    newscript->GetAI = &GetAI_npc_arei;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_arei;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_11555";
    newscript->pGossipHello = &GossipHello_npc_11555;
    newscript->pGossipSelect = &GossipSelect_npc_11555;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npcs_riverbreeze_and_silversky";
    newscript->pGossipHello = &GossipHello_npcs_riverbreeze_and_silversky;
    newscript->pGossipSelect = &GossipSelect_npcs_riverbreeze_and_silversky;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ancient_trees";
    newscript->GetAI = &GetAI_npc_ancient_trees;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_kitten";
    newscript->GetAI = &GetAI_npc_kitten;
    newscript->pEffectDummyNPC = &EffectDummyCreature_npc_kitten;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_corrupt_saber";
    newscript->pGossipHello = &GossipHello_npc_corrupt_saber;
    newscript->pGossipSelect = &GossipSelect_npc_corrupt_saber;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_niby_the_almighty";
    newscript->GetAI = &GetAI_npc_niby_the_almighty;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_niby_the_almighty;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_kroshius";
    newscript->GetAI = &GetAI_npc_kroshius;
    newscript->pProcessEventId = &ProcessEventId_npc_kroshius;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_captured_arkonarin";
    newscript->GetAI = &GetAI_npc_captured_arkonarin;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_captured_arkonarin;
    newscript->RegisterSelf();
}

