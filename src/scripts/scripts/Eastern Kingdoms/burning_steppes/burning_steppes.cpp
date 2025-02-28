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
SDName: Burning_Steppes
SD%Complete: 100
SDComment: Quest support: 4224, 4866
SDCategory: Burning Steppes
EndScriptData */

/* ContentData
npc_ragged_john
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_ragged_john
######*/

#define GOSSIP_HELLO    16118
#define GOSSIP_SELECT1  16119
#define GOSSIP_SELECT2  16120
#define GOSSIP_SELECT3  16121
#define GOSSIP_SELECT4  16122
#define GOSSIP_SELECT5  16123
#define GOSSIP_SELECT6  16124
#define GOSSIP_SELECT7  16125
#define GOSSIP_SELECT8  16126
#define GOSSIP_SELECT9  16127
#define GOSSIP_SELECT10 16128
#define GOSSIP_SELECT11 16129

struct npc_ragged_johnAI : public ScriptedAI
{
    npc_ragged_johnAI(Creature *c) : ScriptedAI(c) {}

    void Reset() {}

    void MoveInLineOfSight(Unit *who)
    {
        if( who->HasAura(16468,0) )
        {
            if( who->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(who, 15) && who->isInAccessiblePlacefor(m_creature) )
            {
                DoCast(who,16472);
                ((Player*)who)->AreaExploredOrEventHappens(4866);
            }
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void EnterCombat(Unit *who) {}
};

CreatureAI* GetAI_npc_ragged_john(Creature *_Creature)
{
    return new npc_ragged_johnAI (_Creature);
}

bool GossipHello_npc_ragged_john(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(4224) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HELLO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(2713, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_ragged_john(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(2714, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(2715, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(2716, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(2717, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            player->SEND_GOSSIP_MENU(2718, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            player->SEND_GOSSIP_MENU(2719, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            player->SEND_GOSSIP_MENU(2720, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT8), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
            player->SEND_GOSSIP_MENU(2721, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+8:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT9), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
            player->SEND_GOSSIP_MENU(2722, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+9:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT10), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
            player->SEND_GOSSIP_MENU(2723, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+10:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SELECT11), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            player->SEND_GOSSIP_MENU(2725, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(4224);
            break;
    }
    return true;
}

#define NPC_SHANI_PROUDTUSK 9136 

bool GOUse_go_proudtuskremains(Player *player, GameObject* _GO)
{
    if (!GetClosestCreatureWithEntry(_GO, NPC_SHANI_PROUDTUSK, 30.0f))
    {
        float x,y,z;
        player->GetNearPoint(x,y,z, 0.0f, 5.0f, frand(0, 2*M_PI));
        player->SummonCreature(NPC_SHANI_PROUDTUSK, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 120000);
    }
    else
        return true;
    return false;
}

/*######
## Script for Quest: Broodling Essence
######*/

// Spells
#define SPELL_DRACO_INCARCINATRIX_900   16007
#define SPELL_CREATE_BROODLING_ESSENCE  16027
#define SPELL_FIREBALL                  13375

struct mob_broodlingessenceAI : public ScriptedAI
{

    mob_broodlingessenceAI(Creature *c) : ScriptedAI(c) {}

    bool onSpellEffect;
    Timer_UnCheked Fireball_Timer;

    void Reset()
    {
        Fireball_Timer.Reset(0);
        onSpellEffect = false;
    }

    void EnterCombat(Unit *who){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_DRACO_INCARCINATRIX_900)
        {
            onSpellEffect = true;
        }
    }

    void JustDied(Unit* killer)
    {
        if(onSpellEffect)
        {
            me->CastSpell(me, SPELL_CREATE_BROODLING_ESSENCE, true);
            me->RemoveCorpse();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!UpdateVictim() )
            return;


        if (Fireball_Timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(),SPELL_FIREBALL);
            Fireball_Timer = 10000;
        }
        
            

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_broodlingessence(Creature *_Creature)
{
    return new mob_broodlingessenceAI (_Creature);
}

/*############################
# npc_artorius_the_amiable
#############################*/

bool GossipHello_npc_franklin_the_friendly(Player *player, Creature *_Creature)
{
    if (player->GetClass() == CLASS_HUNTER && player->GetQuestStatus(7636) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16130), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(7043, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_franklin_the_friendly(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        _Creature->UpdateEntry(14534);
        _Creature->SetSpeed(MOVE_WALK, 1.0f, true);
        _Creature->SetSpeed(MOVE_RUN, 1.1f, true);
        player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

struct npc_franklin_the_friendlyAI : public ScriptedAI
{
    npc_franklin_the_friendlyAI(Creature *c) : ScriptedAI(c) {}

    uint32 DemonicFrenzyTimer;
    uint32 FoolsPlightTimer;
    uint32 CheckTimer;

    void Reset()
    {
        DemonicFrenzyTimer = 15000;
        FoolsPlightTimer = 3000;
        CheckTimer = 4000;
        me->ApplySpellImmune(0, IMMUNITY_ID, 11726, true);
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
            pSummoned->AI()->AttackStart(target);
    }
    
    void EnterCombat(Unit* who)
    {
        me->SetIsDistanceToHomeEvadable(false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(me->GetEntry() == 14529)
        {
            if(FoolsPlightTimer <= diff)
            {
                Unit * victim = SelectUnit(SELECT_TARGET_RANDOM, 0, 35, true);
    
                if (victim)
                    me->CastSpell(victim, 23504, false);
    
                FoolsPlightTimer = 3000;
            } else FoolsPlightTimer -= diff;
            DoMeleeAttackIfReady();
            CastNextSpellIfAnyAndReady();
        }
        else
        {
            if(DemonicFrenzyTimer <= diff)
            {
                AddSpellToCast(me, 23257);
                DemonicFrenzyTimer = 15000;
            } else DemonicFrenzyTimer -= diff;

            if(CheckTimer <= diff)
            {
                if(m_creature->getThreatManager().getThreatList().size() >= 2)
                    EnterEvadeMode();
                CheckTimer = 4000;
            } else CheckTimer -= diff;
        
            DoMeleeAttackIfReady();
            CastNextSpellIfAnyAndReady();
        }
    }
};

CreatureAI* GetAI_npc_franklin_the_friendly(Creature *_Creature)
{
    return new npc_franklin_the_friendlyAI (_Creature);
}

/*######
## npc_grark_lorkrub
######*/

enum
{
    SAY_START                   = -1000873,
    SAY_PAY                     = -1000874,
    SAY_FIRST_AMBUSH_START      = -1000875,
    SAY_FIRST_AMBUSH_END        = -1000876,
    SAY_SEC_AMBUSH_START        = -1000877,
    SAY_SEC_AMBUSH_END          = -1000878,
    SAY_THIRD_AMBUSH_START      = -1000879,
    SAY_THIRD_AMBUSH_END        = -1000880,
    EMOTE_LAUGH                 = -1000881,
    SAY_LAST_STAND              = -1000882,
    SAY_LEXLORT_1               = -1000883,
    SAY_LEXLORT_2               = -1000884,
    EMOTE_RAISE_AXE             = -1000885,
    EMOTE_LOWER_HAND            = -1000886,
    SAY_LEXLORT_3               = -1000887,
    SAY_LEXLORT_4               = -1000888,

    EMOTE_SUBMIT                = -1000889,
    SAY_AGGRO                   = -1000890,

    SPELL_CAPTURE_GRARK             = 14250,

    NPC_BLACKROCK_AMBUSHER          = 9522,
    NPC_BLACKROCK_RAIDER            = 9605,
    NPC_FLAMESCALE_DRAGONSPAWN      = 7042,
    NPC_SEARSCALE_DRAKE             = 7046,

    NPC_GRARK_LORKRUB               = 9520,
    NPC_HIGH_EXECUTIONER_NUZARK     = 9538,
    NPC_SHADOW_OF_LEXLORT           = 9539,

    FACTION_FRIENDLY                = 35,

    QUEST_ID_PRECARIOUS_PREDICAMENT = 4121
};

static const DialogueEntry aOutroDialogue[] =
{
    {SAY_LAST_STAND,    NPC_GRARK_LORKRUB,              5000},
    {SAY_LEXLORT_1,     NPC_SHADOW_OF_LEXLORT,          3000},
    {SAY_LEXLORT_2,     NPC_SHADOW_OF_LEXLORT,          5000},
    {EMOTE_RAISE_AXE,   NPC_HIGH_EXECUTIONER_NUZARK,    4000},
    {EMOTE_LOWER_HAND,  NPC_SHADOW_OF_LEXLORT,          3000},
    {SAY_LEXLORT_3,     NPC_SHADOW_OF_LEXLORT,          3000},
    {NPC_GRARK_LORKRUB, 0,                              5000},
    {SAY_LEXLORT_4,     NPC_SHADOW_OF_LEXLORT,          0},
    {0, 0, 0},
};

struct npc_grark_lorkrubAI : public npc_escortAI, private DialogueHelper
{
    npc_grark_lorkrubAI(Creature* pCreature) : npc_escortAI(pCreature),
        DialogueHelper(aOutroDialogue)
    {
        Reset();
    }

    uint64 nuzarkGuid;
    uint64 lexlortGuid;

    std::list<uint64> SearscaleGuidList;

    uint8 KilledCreatures;
    bool IsFirstSearScale;
    bool SpellUsed;

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            KilledCreatures = 0;
            IsFirstSearScale = true;
            SpellUsed = false;

            SearscaleGuidList.clear();

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == SPELL_CAPTURE_GRARK && !SpellUsed)
        {
            SetEscortPaused(false);
            SpellUsed = true;
        }
    }

    void EnterCombat(Unit* /*pWho*/)
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            DoScriptText(SAY_AGGRO, me);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        // No combat during escort
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        npc_escortAI::MoveInLineOfSight(pWho);
    }

    void WaypointReached(uint32 PointId)
    {
        switch (PointId)
        {
            case 1:
                DoScriptText(SAY_START, me);
                break;
            case 7:
                DoScriptText(SAY_PAY, me);
                break;
            case 12:
                DoScriptText(SAY_FIRST_AMBUSH_START, me);
                SetEscortPaused(true);

                me->SummonCreature(NPC_BLACKROCK_AMBUSHER, -7844.3f, -1521.6f, 139.2f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                me->SummonCreature(NPC_BLACKROCK_AMBUSHER, -7860.4f, -1507.8f, 141.0f, 6.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                me->SummonCreature(NPC_BLACKROCK_RAIDER,   -7845.6f, -1508.1f, 138.8f, 6.1f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                me->SummonCreature(NPC_BLACKROCK_RAIDER,   -7859.8f, -1521.8f, 139.2f, 6.2f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                break;
            case 24:
                DoScriptText(SAY_SEC_AMBUSH_START, me);
                SetEscortPaused(true);

                me->SummonCreature(NPC_BLACKROCK_AMBUSHER,     -8035.3f, -1222.2f, 135.5f, 5.1f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                me->SummonCreature(NPC_FLAMESCALE_DRAGONSPAWN, -8037.5f, -1216.9f, 135.8f, 5.1f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                me->SummonCreature(NPC_BLACKROCK_AMBUSHER,     -8009.5f, -1222.1f, 139.2f, 3.9f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                me->SummonCreature(NPC_FLAMESCALE_DRAGONSPAWN, -8007.1f, -1219.4f, 140.1f, 3.9f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 20000);
                break;
            case 28:
                me->SummonCreature(NPC_SEARSCALE_DRAKE, -7897.8f, -1123.1f, 233.4f, 3.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                me->SummonCreature(NPC_SEARSCALE_DRAKE, -7898.8f, -1125.1f, 193.9f, 3.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                me->SummonCreature(NPC_SEARSCALE_DRAKE, -7895.6f, -1119.5f, 194.5f, 3.1f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
                break;
            case 30:
            {
                SetEscortPaused(true);
                DoScriptText(SAY_THIRD_AMBUSH_START, me);

                Player* pPlayer = GetPlayerForEscort();
                if (!pPlayer)
                    return;

                // Set all the dragons in combat
                for (std::list<uint64>::const_iterator itr = SearscaleGuidList.begin(); itr != SearscaleGuidList.end(); ++itr)
                {
                    if (Creature* pTemp = me->GetMap()->GetCreature(*itr))
                        pTemp->AI()->AttackStart(pPlayer);
                }
                break;
            }
            case 36:
                DoScriptText(EMOTE_LAUGH, me);
                break;
            case 45:
                StartNextDialogueText(SAY_LAST_STAND);
                SetEscortPaused(true);

                me->SummonCreature(NPC_HIGH_EXECUTIONER_NUZARK, -7532.3f, -1029.4f, 258.0f, 2.7f, TEMPSUMMON_TIMED_DESPAWN, 40000);
                me->SummonCreature(NPC_SHADOW_OF_LEXLORT,       -7532.8f, -1032.9f, 258.2f, 2.5f, TEMPSUMMON_TIMED_DESPAWN, 40000);
                break;
        }
    }

    void JustDidDialogueStep(int32 iEntry)
    {
        switch (iEntry)
        {
            case SAY_LEXLORT_1:
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
            case SAY_LEXLORT_3:
                // Note: this part isn't very clear. Should he just simply attack him, or charge him?
                if (Creature* pNuzark = me->GetMap()->GetCreature(nuzarkGuid))
                    pNuzark->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK2HTIGHT);
                break;
            case NPC_GRARK_LORKRUB:
                // Fake death creature when the axe is lowered. This will allow us to finish the event
                me->InterruptNonMeleeSpells(true);
                me->SetHealth(1);
                me->StopMoving();
                me->ClearComboPointHolders();
                me->RemoveAllAurasOnDeath();
                me->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
                me->ModifyAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->ClearAllReactives();
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                break;
            case SAY_LEXLORT_4:
                // Finish the quest
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->AreaExploredOrEventHappens(QUEST_ID_PRECARIOUS_PREDICAMENT);
                // Kill self
                me->DisappearAndDie();
                break;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_HIGH_EXECUTIONER_NUZARK:
                nuzarkGuid  = pSummoned->GetGUID();
                break;
            case NPC_SHADOW_OF_LEXLORT:
                lexlortGuid = pSummoned->GetGUID();
                break;
            case NPC_SEARSCALE_DRAKE:
                // If it's the flying drake allow him to move in circles
                if (IsFirstSearScale)
                {
                    IsFirstSearScale = false;

                    pSummoned->SetLevitate(true);
                    // ToDo: this guy should fly in circles above the creature
                }
                SearscaleGuidList.push_back(pSummoned->GetGUID());
                break;

            default:
                // The hostile mobs should attack the player only
                if (Player* pPlayer = GetPlayerForEscort())
                    pSummoned->AI()->AttackStart(pPlayer);
                break;
        }
    }

    void SummonedCreatureDies(Creature* /*pSummoned*/, Unit* killer)
    {
        ++KilledCreatures;

        switch (KilledCreatures)
        {
            case 4:
                DoScriptText(SAY_FIRST_AMBUSH_END, me);
                SetEscortPaused(false);
                break;
            case 8:
                DoScriptText(SAY_SEC_AMBUSH_END, me);
                SetEscortPaused(false);
                break;
            case 11:
                DoScriptText(SAY_THIRD_AMBUSH_END, me);
                SetEscortPaused(false);
                break;
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry)
    {
        switch (uiEntry)
        {
            case NPC_GRARK_LORKRUB:           return me;
            case NPC_HIGH_EXECUTIONER_NUZARK: return me->GetMap()->GetCreature(nuzarkGuid);
            case NPC_SHADOW_OF_LEXLORT:       return me->GetMap()->GetCreature(lexlortGuid);

            default:
                return nullptr;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff)
    {
        DialogueUpdate(uiDiff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_grark_lorkrub(Creature* pCreature)
{

    npc_grark_lorkrubAI* npc_grark_lorkrub = new npc_grark_lorkrubAI(pCreature);

    npc_grark_lorkrub->AddWaypoint(1, -7699.62, -1444.29, 139.87, 4000),
    npc_grark_lorkrub->AddWaypoint(2, -7670.67, -1458.25, 140.74, 0),
    npc_grark_lorkrub->AddWaypoint(3, -7675.26, -1465.58, 140.74, 0),
    npc_grark_lorkrub->AddWaypoint(4, -7685.84, -1472.66, 140.75, 0),
    npc_grark_lorkrub->AddWaypoint(5, -7700.08, -1473.41, 140.79, 0),
    npc_grark_lorkrub->AddWaypoint(6, -7712.55, -1470.19, 140.79, 0),
    npc_grark_lorkrub->AddWaypoint(7, -7717.27, -1481.70, 140.72, 5000),
    npc_grark_lorkrub->AddWaypoint(8, -7726.23, -1500.78, 132.99, 0),
    npc_grark_lorkrub->AddWaypoint(9, -7744.61, -1531.61, 132.69, 0),
    npc_grark_lorkrub->AddWaypoint(10, -7763.08, -1536.22, 131.93, 0),
    npc_grark_lorkrub->AddWaypoint(11, -7815.32, -1522.61, 134.16, 0),
    npc_grark_lorkrub->AddWaypoint(12, -7850.26, -1516.87, 138.17, 0),
    npc_grark_lorkrub->AddWaypoint(13, -7850.26, -1516.87, 138.17, 3000),
    npc_grark_lorkrub->AddWaypoint(14, -7881.01, -1508.49, 142.37, 0),
    npc_grark_lorkrub->AddWaypoint(15, -7888.91, -1458.09, 144.79, 0),
    npc_grark_lorkrub->AddWaypoint(16, -7889.18, -1430.21, 145.31, 0),
    npc_grark_lorkrub->AddWaypoint(17, -7900.53, -1427.01, 150.26, 0),
    npc_grark_lorkrub->AddWaypoint(18, -7904.15, -1429.91, 150.27, 0),
    npc_grark_lorkrub->AddWaypoint(19, -7921.48, -1425.47, 140.54, 0),
    npc_grark_lorkrub->AddWaypoint(20, -7941.43, -1413.10, 134.35, 0),
    npc_grark_lorkrub->AddWaypoint(21, -7964.85, -1367.45, 132.99, 0),
    npc_grark_lorkrub->AddWaypoint(22, -7989.95, -1319.121, 133.71, 0),
    npc_grark_lorkrub->AddWaypoint(23, -8010.43, -1270.23, 133.42, 0),
    npc_grark_lorkrub->AddWaypoint(24, -8025.62, -1243.78, 133.91, 0),
    npc_grark_lorkrub->AddWaypoint(25, -8025.62, -1243.78, 133.91, 3000),
    npc_grark_lorkrub->AddWaypoint(26, -8015.22, -1196.98, 146.76, 0),
    npc_grark_lorkrub->AddWaypoint(27, -7994.68, -1151.38, 160.70, 0),
    npc_grark_lorkrub->AddWaypoint(28, -7970.91, -1132.81, 170.16, 0),
    npc_grark_lorkrub->AddWaypoint(29, -7927.59, -1122.79, 185.86, 0),
    npc_grark_lorkrub->AddWaypoint(30, -7897.67, -1126.67, 194.32, 0),
    npc_grark_lorkrub->AddWaypoint(31, -7897.67, -1126.67, 194.32, 3000),
    npc_grark_lorkrub->AddWaypoint(32, -7864.11, -1135.98, 203.29, 0),
    npc_grark_lorkrub->AddWaypoint(33, -7837.31, -1137.73, 209.63, 0),
    npc_grark_lorkrub->AddWaypoint(34, -7808.72, -1134.90, 214.84, 0),
    npc_grark_lorkrub->AddWaypoint(35, -7786.85, -1127.24, 214.84, 0),
    npc_grark_lorkrub->AddWaypoint(36, -7746.58, -1125.16, 215.08, 5000),
    npc_grark_lorkrub->AddWaypoint(37, -7746.41, -1103.62, 215.62, 0),
    npc_grark_lorkrub->AddWaypoint(38, -7740.25, -1090.51, 216.69, 0),
    npc_grark_lorkrub->AddWaypoint(39, -7730.97, -1085.55, 217.12, 0),
    npc_grark_lorkrub->AddWaypoint(40, -7697.89, -1089.43, 217.62, 0),
    npc_grark_lorkrub->AddWaypoint(41, -7679.30, -1059.15, 220.09, 0),
    npc_grark_lorkrub->AddWaypoint(42, -7661.39, -1038.24, 226.24, 0),
    npc_grark_lorkrub->AddWaypoint(43, -7634.49, -1020.96, 234.30, 0),
    npc_grark_lorkrub->AddWaypoint(44, -7596.22, -1013.16, 244.03, 0),
    npc_grark_lorkrub->AddWaypoint(45, -7556.53, -1021.74, 253.21, 28000);
    npc_grark_lorkrub->AddWaypoint(46, -7554.37, -1022.43, 253.61, 0);
    return (CreatureAI*)npc_grark_lorkrub;
}

bool QuestAccept_npc_grark_lorkrub(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ID_PRECARIOUS_PREDICAMENT)
    {
        if (npc_grark_lorkrubAI* pEscortAI = dynamic_cast<npc_grark_lorkrubAI*>(pCreature->AI()))
        {
            pEscortAI->Start(false, false, pPlayer->GetGUID(), pQuest, true);
            pEscortAI->SetEscortPaused(true);
        }

        return true;
    }

    return false;
}

void AddSC_burning_steppes()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_grark_lorkrub";
    newscript->GetAI = &GetAI_npc_grark_lorkrub;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_grark_lorkrub;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ragged_john";
    newscript->GetAI = &GetAI_npc_ragged_john;
    newscript->pGossipHello =  &GossipHello_npc_ragged_john;
    newscript->pGossipSelect = &GossipSelect_npc_ragged_john;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_proudtuskremains";
    newscript->pGOUse = &GOUse_go_proudtuskremains;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_broodlingessence";
    newscript->GetAI = &GetAI_mob_broodlingessence;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_franklin_the_friendly";
    newscript->pGossipHello =  &GossipHello_npc_franklin_the_friendly;
    newscript->pGossipSelect = &GossipSelect_npc_franklin_the_friendly;
    newscript->GetAI = &GetAI_npc_franklin_the_friendly;
    newscript->RegisterSelf();
}

