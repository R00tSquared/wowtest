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
SDName: Winterspring
SD%Complete: 90
SDComment: Quest support: 5126 (Loraxs' tale missing proper gossip items text). Vendor Rivern Frostwind. Obtain Cache of Mau'ari
SDCategory: Winterspring
EndScriptData */

/* ContentData
npc_lorax
npc_rivern_frostwind
npc_witch_doctor_mauari
npc_haleh
npc_earthcaller_franzahl
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_lorax
######*/

#define GOSSIP_HL  16390

#define GOSSIP_SL1 16391
#define GOSSIP_SL2 16392
#define GOSSIP_SL3 16393
#define GOSSIP_SL4 16394
#define GOSSIP_SL5 16395

bool GossipHello_npc_lorax(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(5126) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HL), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lorax(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(3759, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(3760, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(3761, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(3762, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            player->SEND_GOSSIP_MENU(3763, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(5126);
            break;
    }
    return true;
}

/*######
## npc_rivern_frostwind
######*/

bool GossipHello_npc_rivern_frostwind(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (_Creature->isVendor() && player->GetReputationMgr().GetRank(589) == REP_EXALTED)
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_rivern_frostwind(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*######
## npc_witch_doctor_mauari
######*/

#define GOSSIP_HWDM 16396

bool GossipHello_npc_witch_doctor_mauari(Player *player, Creature *_Creature)
{
    if(_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(player->GetQuestRewardStatus(975))
    {
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_HWDM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(3377, _Creature->GetGUID());
    }else
        player->SEND_GOSSIP_MENU(3375, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_witch_doctor_mauari(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action==GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 16351, false);
    }

    return true;
}

/*####
# npc_haleh
####*/

#define GOSSIP_ITEM_HALEH 16397

bool GossipHello_npc_haleh(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(player->GetQuestRewardStatus(6502) && !player->HasItemCount(16309,1))
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_HALEH), GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_haleh(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_SENDER_INFO )
    {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 16309, 1);
            if (msg == EQUIP_ERR_OK)
            {
                 Item* item = player->StoreNewItem(dest, 16309, true);
                 player->SendNewItem(item,1,true,false,true);
            }
    player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

/*############################
# npc_artorius_the_amiable
#############################*/

bool GossipHello_npc_artorius_the_amiable(Player *player, Creature *_Creature)
{
    if (player->GetClass() == CLASS_HUNTER && player->GetQuestStatus(7636) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16402), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(7045, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_artorius_the_amiable(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        _Creature->UpdateEntry(14535);
        _Creature->SetSpeed(MOVE_WALK, 0.9f, true);
        _Creature->SetSpeed(MOVE_RUN, 0.9f, true);
        player->CLOSE_GOSSIP_MENU();
    }
    return true;
}

struct npc_artorius_the_amiableAI : public ScriptedAI
{
    npc_artorius_the_amiableAI(Creature *c) : ScriptedAI(c) {}

    uint32 DemonicFrenzyTimer;
    uint32 DemonicDoomTimer;
    uint32 FoolsPlightTimer;
    uint32 CheckTimer;

    void Reset()
    {
        DemonicFrenzyTimer = 30000;
        DemonicDoomTimer = 45000;
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

        if(me->GetEntry() == 14531)
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
                DemonicFrenzyTimer = urand(40000, 60000);
            } else DemonicFrenzyTimer -= diff;

            if(DemonicDoomTimer <= diff)
            {
                AddSpellToCast(me->GetVictim(), 23298);
                DemonicDoomTimer = 90000;
            } else DemonicDoomTimer -= diff;

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

CreatureAI* GetAI_npc_artorius_the_amiable(Creature *_Creature)
{
    return new npc_artorius_the_amiableAI (_Creature);
}

/*####
# npc_ranshalla
####*/

enum
{
    // Escort texts
    SAY_QUEST_START             = -1000739,
    SAY_ENTER_OWL_THICKET       = -1000707,
    SAY_REACH_TORCH_1           = -1000708,
    SAY_REACH_TORCH_2           = -1000709,
    SAY_REACH_TORCH_3           = -1000710,
    SAY_AFTER_TORCH_1           = -1000711,
    SAY_AFTER_TORCH_2           = -1000712,
    SAY_REACH_ALTAR_1           = -1000713,
    SAY_REACH_ALTAR_2           = -1000714,

    // After lighting the altar cinematic
    SAY_RANSHALLA_ALTAR_1       = -1000715,
    SAY_RANSHALLA_ALTAR_2       = -1000716,
    SAY_PRIESTESS_ALTAR_3       = -1000717,
    SAY_PRIESTESS_ALTAR_4       = -1000718,
    SAY_RANSHALLA_ALTAR_5       = -1000719,
    SAY_RANSHALLA_ALTAR_6       = -1000720,
    SAY_PRIESTESS_ALTAR_7       = -1000721,
    SAY_PRIESTESS_ALTAR_8       = -1000722,
    SAY_PRIESTESS_ALTAR_9       = -1000723,
    SAY_PRIESTESS_ALTAR_10      = -1000724,
    SAY_PRIESTESS_ALTAR_11      = -1000725,
    SAY_PRIESTESS_ALTAR_12      = -1000726,
    SAY_PRIESTESS_ALTAR_13      = -1000727,
    SAY_PRIESTESS_ALTAR_14      = -1000728,
    SAY_VOICE_ALTAR_15          = -1000729,
    SAY_PRIESTESS_ALTAR_16      = -1000730,
    SAY_PRIESTESS_ALTAR_17      = -1000731,
    SAY_PRIESTESS_ALTAR_18      = -1000732,
    SAY_PRIESTESS_ALTAR_19      = -1000733,
    SAY_PRIESTESS_ALTAR_20      = -1000734,
    SAY_PRIESTESS_ALTAR_21      = -1000735,
    SAY_QUEST_END_1             = -1000736,
    SAY_QUEST_END_2             = -1000737,

    EMOTE_CHANT_SPELL           = -1000738,

    SPELL_LIGHT_TORCH           = 18953,        // channeled spell by Ranshalla while waiting for the torches / altar

    NPC_RANSHALLA               = 10300,
    NPC_PRIESTESS_ELUNE         = 12116,
    NPC_VOICE_ELUNE             = 12152,
    NPC_GUARDIAN_ELUNE          = 12140,

    NPC_PRIESTESS_DATA_1        = 1,            // dummy member for the first priestess (right)
    NPC_PRIESTESS_DATA_2        = 2,            // dummy member for the second priestess (left)
    DATA_MOVE_PRIESTESS         = 3,            // dummy member to check the priestess movement
    DATA_EVENT_END              = 4,            // dummy member to indicate the event end

    GO_ELUNE_ALTAR              = 177404,
    GO_ELUNE_FIRE               = 177417,
    GO_ELUNE_GEM                = 177414,       // is respawned in script
    GO_ELUNE_LIGHT              = 177415,       // are respawned in script

    QUEST_GUARDIANS_ALTAR       = 4901,
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_REACH_ALTAR_1,      NPC_RANSHALLA,        2000},
    {SAY_REACH_ALTAR_2,      NPC_RANSHALLA,        3000},
    {NPC_RANSHALLA,          0,                    0},          // start the altar channeling
    {SAY_PRIESTESS_ALTAR_3,  NPC_PRIESTESS_DATA_2, 1000},
    {SAY_PRIESTESS_ALTAR_4,  NPC_PRIESTESS_DATA_1, 4000},
    {SAY_RANSHALLA_ALTAR_5,  NPC_RANSHALLA,        4000},
    {SAY_RANSHALLA_ALTAR_6,  NPC_RANSHALLA,        4000},       // start the escort here
    {SAY_PRIESTESS_ALTAR_7,  NPC_PRIESTESS_DATA_2, 4000},
    {SAY_PRIESTESS_ALTAR_8,  NPC_PRIESTESS_DATA_2, 5000},       // show the gem
    {GO_ELUNE_GEM,           0,                    5000},
    {SAY_PRIESTESS_ALTAR_9,  NPC_PRIESTESS_DATA_1, 4000},       // move priestess 1 near m_creature
    {NPC_PRIESTESS_DATA_1,   0,                    3000},
    {SAY_PRIESTESS_ALTAR_10, NPC_PRIESTESS_DATA_1, 5000},
    {SAY_PRIESTESS_ALTAR_11, NPC_PRIESTESS_DATA_1, 4000},
    {SAY_PRIESTESS_ALTAR_12, NPC_PRIESTESS_DATA_1, 5000},
    {SAY_PRIESTESS_ALTAR_13, NPC_PRIESTESS_DATA_1, 8000},       // summon voice and guard of elune
    {NPC_VOICE_ELUNE,        0,                    12000},
    {SAY_VOICE_ALTAR_15,     NPC_VOICE_ELUNE,      5000},       // move priestess 2 near m_creature
    {NPC_PRIESTESS_DATA_2,   0,                    3000},
    {SAY_PRIESTESS_ALTAR_16, NPC_PRIESTESS_DATA_2, 4000},
    {SAY_PRIESTESS_ALTAR_17, NPC_PRIESTESS_DATA_2, 6000},
    {SAY_PRIESTESS_ALTAR_18, NPC_PRIESTESS_DATA_1, 5000},
    {SAY_PRIESTESS_ALTAR_19, NPC_PRIESTESS_DATA_1, 3000},       // move the owlbeast
    {NPC_GUARDIAN_ELUNE,     0,                    2000},
    {SAY_PRIESTESS_ALTAR_20, NPC_PRIESTESS_DATA_1, 4000},       // move the first priestess up
    {SAY_PRIESTESS_ALTAR_21, NPC_PRIESTESS_DATA_2, 10000},      // move second priestess up
    {DATA_MOVE_PRIESTESS,    0,                    6000},       // despawn the gem
    {DATA_EVENT_END,         0,                    2000},       // turn towards the player
    {SAY_QUEST_END_2,        NPC_RANSHALLA,        0},
    {0, 0, 0},
};

struct EventLocations
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static EventLocations aWingThicketLocations[] =
{
    {5515.98f, -4903.43f, 846.30f, 4.58f},      // 0 right priestess summon loc
    {5501.94f, -4920.20f, 848.69f, 6.15f},      // 1 left priestess summon loc
    {5497.35f, -4906.49f, 850.83f, 2.76f},      // 2 guard of elune summon loc
    {5518.38f, -4913.47f, 845.57f},             // 3 right priestess move loc
    {5510.36f, -4921.17f, 846.33f},             // 4 left priestess move loc
    {5511.31f, -4913.82f, 847.17f},             // 5 guard of elune move loc
    {5518.51f, -4917.56f, 845.23f},             // 6 right priestess second move loc
    {5514.40f, -4921.16f, 845.49f}              // 7 left priestess second move loc
};

struct npc_ranshallaAI : public npc_escortAI, private DialogueHelper
{
    npc_ranshallaAI(Creature* pCreature) : npc_escortAI(pCreature),
        DialogueHelper(aIntroDialogue)
    {
        Reset();
    }

    uint32 DelayTimer;
    uint32 MoveAgainTimer;

    uint64 m_firstPriestessGuid;
    uint64 m_secondPriestessGuid;
    uint64 m_guardEluneGuid;
    uint64 m_voiceEluneGuid;
    uint64 m_altarGuid;

    void Reset()
    {
        DelayTimer = 0;
        MoveAgainTimer = 0;
    }

    // Called when the player activates the torch / altar
    void DoContinueEscort(bool bIsAltarWaypoint = false)
    {
        if (bIsAltarWaypoint)
            DoScriptText(SAY_RANSHALLA_ALTAR_1, m_creature);
        else
        {
            switch(urand(0, 1))
            {
                case 0: DoScriptText(SAY_AFTER_TORCH_1, m_creature); break;
                case 1: DoScriptText(SAY_AFTER_TORCH_2, m_creature); break;
            }
        }

        DelayTimer = 2000;
    }

    // Called when Ranshalla starts to channel on a torch / altar
    void DoChannelTorchSpell(bool bIsAltarWaypoint = false)
    {
        // Check if we are using the fire or the altar and remove the no_interact flag
        if (bIsAltarWaypoint)
        {
            if (GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, GO_ELUNE_ALTAR, 25.0f))
            {
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                m_creature->SetFacingToObject(pGo);
                m_altarGuid = pGo->GetGUID();
            }
        }
        else
        {
            if (GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, GO_ELUNE_FIRE, 25.0f))
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
        }

        // Yell and set escort to pause
        switch(urand(0, 2))
        {
            case 0: DoScriptText(SAY_REACH_TORCH_1, m_creature); break;
            case 1: DoScriptText(SAY_REACH_TORCH_2, m_creature); break;
            case 2: DoScriptText(SAY_REACH_TORCH_3, m_creature); break;
        }

        DoScriptText(EMOTE_CHANT_SPELL, m_creature);
        DoCast(m_creature, SPELL_LIGHT_TORCH);
        SetEscortPaused(true);
    }

    void DoSummonPriestess()
    {
        // Summon 2 Elune priestess and make each of them move to a different spot
        if (Creature* pPriestess = m_creature->SummonCreature(NPC_PRIESTESS_ELUNE, aWingThicketLocations[0].m_fX, aWingThicketLocations[0].m_fY, aWingThicketLocations[0].m_fZ, aWingThicketLocations[0].m_fO, TEMPSUMMON_CORPSE_DESPAWN, 0))
        {
            pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[3].m_fX, aWingThicketLocations[3].m_fY, aWingThicketLocations[3].m_fZ);
            m_firstPriestessGuid = pPriestess->GetGUID();
        }
        if (Creature* pPriestess = m_creature->SummonCreature(NPC_PRIESTESS_ELUNE, aWingThicketLocations[1].m_fX, aWingThicketLocations[1].m_fY, aWingThicketLocations[1].m_fZ, aWingThicketLocations[1].m_fO, TEMPSUMMON_CORPSE_DESPAWN, 0))
        {
            // Left priestess should have a distinct move point because she is the one who starts the dialogue at point reach
            pPriestess->GetMotionMaster()->MovePoint(1, aWingThicketLocations[4].m_fX, aWingThicketLocations[4].m_fY, aWingThicketLocations[4].m_fZ);
            m_secondPriestessGuid = pPriestess->GetGUID();
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiType, uint32 uiPointId)
    {
        if (uiType != POINT_MOTION_TYPE || pSummoned->GetEntry() != NPC_PRIESTESS_ELUNE || uiPointId != 1)
            return;

        // Start the dialogue when the priestess reach the altar (they should both reach the point in the same time)
        StartNextDialogueText(SAY_PRIESTESS_ALTAR_3);
    }

    void WaypointReached(uint32 uiPointId)
    {
        switch(uiPointId)
        {
            case 3:
                DoScriptText(SAY_ENTER_OWL_THICKET, m_creature);
                break;
            case 10: // Cavern 1
                DoChannelTorchSpell();
                break;
            case 15: // Cavern 2
                DoChannelTorchSpell();
                break;
            case 20: // Cavern 3
                DoChannelTorchSpell();
                break;
            case 25: // Cavern 4
                DoChannelTorchSpell();
                break;
            case 36: // Cavern 5
                DoChannelTorchSpell();
                break;
            case 39:
                StartNextDialogueText(SAY_REACH_ALTAR_1);
                SetEscortPaused(true);
                break;
            case 41:
                {
                    // Search for all nearest lights and respawn them
                    std::list<GameObject*> m_lEluneLights;
                    Hellground::AllGameObjectsWithEntryInGrid go_check(GO_ELUNE_LIGHT);
                    Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> go_search(m_lEluneLights, go_check);
                    Cell::VisitGridObjects(me, go_search, 20.0f);
                    for (std::list<GameObject*>::const_iterator itr = m_lEluneLights.begin(); itr != m_lEluneLights.end(); ++itr)
                    {
                        if ((*itr)->isSpawned())
                            continue;

                        (*itr)->SetRespawnTime(115);
                        (*itr)->Refresh();
                    }

                    if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                        m_creature->SetFacingToObject(pAltar);
                    break;
                }
            case 42:
                // Summon the 2 priestess
                SetEscortPaused(true);
                DoSummonPriestess();
                DoScriptText(SAY_RANSHALLA_ALTAR_2, m_creature);
                break;
            case 44:
                // Stop the escort and turn towards the altar
                SetEscortPaused(true);
                if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                    m_creature->SetFacingToObject(pAltar);
                break;
        }
    }

    void JustDidDialogueStep(int32 iEntry)
    {
        switch(iEntry)
        {
            case NPC_RANSHALLA:
                // Start the altar channeling
                DoChannelTorchSpell(true);
                break;
            case SAY_RANSHALLA_ALTAR_6:
                SetEscortPaused(false);
                break;
            case SAY_PRIESTESS_ALTAR_8:
                // make the gem respawn
                if (GameObject* pGem = GetClosestGameObjectWithEntry(m_creature, GO_ELUNE_GEM, 10.0f))
                {
                    if (pGem->isSpawned())
                        break;

                    pGem->SetRespawnTime(90);
                    pGem->Refresh();
                } else me->SummonGameObject(175704, -6386.890137, -1984.050049, 246.729996, 0, 0, 0, 0, 0, 90000);
                break;
            case SAY_PRIESTESS_ALTAR_9:
                // move near the escort npc
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_firstPriestessGuid))
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[6].m_fX, aWingThicketLocations[6].m_fY, aWingThicketLocations[6].m_fZ);
                break;
            case SAY_PRIESTESS_ALTAR_13:
                // summon the Guardian of Elune
                if (Creature* pGuard = m_creature->SummonCreature(NPC_GUARDIAN_ELUNE, aWingThicketLocations[2].m_fX, aWingThicketLocations[2].m_fY, aWingThicketLocations[2].m_fZ, aWingThicketLocations[2].m_fO, TEMPSUMMON_CORPSE_DESPAWN, 0))
                {
                    pGuard->GetMotionMaster()->MovePoint(0, aWingThicketLocations[5].m_fX, aWingThicketLocations[5].m_fY, aWingThicketLocations[5].m_fZ);
                    m_guardEluneGuid = pGuard->GetGUID();
                }
                // summon the Voice of Elune
                if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                {
                    if (Creature* pVoice = m_creature->SummonCreature(NPC_VOICE_ELUNE, pAltar->GetPositionX(), pAltar->GetPositionY(), pAltar->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 30000))
                        m_voiceEluneGuid = pVoice->GetGUID();
                }
                break;
            case SAY_VOICE_ALTAR_15:
                // move near the escort npc and continue dialogue
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_secondPriestessGuid))
                {
                    DoScriptText(SAY_PRIESTESS_ALTAR_14, pPriestess);
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[7].m_fX, aWingThicketLocations[7].m_fY, aWingThicketLocations[7].m_fZ);
                }
                break;
            case SAY_PRIESTESS_ALTAR_19:
                // make the voice of elune leave
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardEluneGuid))
                {
                    pGuard->GetMotionMaster()->MovePoint(0, aWingThicketLocations[2].m_fX, aWingThicketLocations[2].m_fY, aWingThicketLocations[2].m_fZ);
                    pGuard->ForcedDespawn(4000);
                }
                break;
            case SAY_PRIESTESS_ALTAR_20:
                // make the first priestess leave
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_firstPriestessGuid))
                {
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[0].m_fX, aWingThicketLocations[0].m_fY, aWingThicketLocations[0].m_fZ);
                    pPriestess->ForcedDespawn(4000);
                }
                break;
            case SAY_PRIESTESS_ALTAR_21:
                // make the second priestess leave
                if (Creature* pPriestess = m_creature->GetMap()->GetCreature(m_secondPriestessGuid))
                {
                    pPriestess->GetMotionMaster()->MovePoint(0, aWingThicketLocations[1].m_fX, aWingThicketLocations[1].m_fY, aWingThicketLocations[1].m_fZ);
                    pPriestess->ForcedDespawn(4000);
                }
                break;
            case DATA_EVENT_END:
                // Turn towards the player
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    m_creature->SetFacingToObject(pPlayer);
                    DoScriptText(SAY_QUEST_END_1, m_creature, pPlayer);
                }
                break;
            case SAY_QUEST_END_2:
                // Turn towards the altar and kneel - quest complete
                if (GameObject* pAltar = m_creature->GetMap()->GetGameObject(m_altarGuid))
                    m_creature->SetFacingToObject(pAltar);
                m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_GUARDIANS_ALTAR, m_creature);
                MoveAgainTimer = 10000;
                break;
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry)
    {
        switch (uiEntry)
        {
            case NPC_RANSHALLA:        return m_creature;
            case NPC_VOICE_ELUNE:      return m_creature->GetMap()->GetCreature(m_voiceEluneGuid);
            case NPC_PRIESTESS_DATA_1: return m_creature->GetMap()->GetCreature(m_firstPriestessGuid);
            case NPC_PRIESTESS_DATA_2: return m_creature->GetMap()->GetCreature(m_secondPriestessGuid);

            default:
                return NULL;
        }

    }

    void UpdateEscortAI(const uint32 diff)
    {
        DialogueUpdate(diff);

        if (DelayTimer)
        {
            if (DelayTimer <= diff)
            {
                m_creature->InterruptNonMeleeSpells(false);
                SetEscortPaused(false);
                DelayTimer = 0;
            }
            else
                DelayTimer -= diff;

            if(MoveAgainTimer <= diff)
            {
                SetEscortPaused(false);
                MoveAgainTimer = 0;
            } else MoveAgainTimer -= diff;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_ranshalla(Creature* pCreature)
{
    return new npc_ranshallaAI(pCreature);
}

bool QuestAccept_npc_ranshalla(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GUARDIANS_ALTAR)
    {
        DoScriptText(SAY_QUEST_START, pCreature);
        pCreature->setFaction(FACTION_ESCORT_A_NEUTRAL_PASSIVE);

        if (npc_ranshallaAI* pEscortAI = dynamic_cast<npc_ranshallaAI*>(pCreature->AI()))
            pEscortAI->Start(false, true, pPlayer->GetGUID(), pQuest);

        return true;
    }

    return false;
}

bool GOUse_go_elune_fire(Player* pPlayer, GameObject* pGo)
{
    // Check if we are using the torches or the altar
    bool bIsAltar = false;

    if (pGo->GetEntry() == GO_ELUNE_ALTAR)
        bIsAltar = true;

    if (Creature* pRanshalla = GetClosestCreatureWithEntry(pGo, NPC_RANSHALLA, 25.0f))
    {
        if (npc_ranshallaAI* pEscortAI = dynamic_cast<npc_ranshallaAI*>(pRanshalla->AI()))
            pEscortAI->DoContinueEscort(bIsAltar);
    }

    return false;
}

enum
{
    SPELL_UNSUMMON_YETI         = 17163
};

struct npc_umi_yetiAI : public ScriptedAI
{
    npc_umi_yetiAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    void Reset()
    {
    }

    void MoveInLineOfSight(Unit *)
    {
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell)
    {
        if (pSpell->Id == SPELL_UNSUMMON_YETI)
        {
            m_creature->GetMotionMaster()->MoveIdle();
            m_creature->ForcedDespawn(1000);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
    }
};

CreatureAI* GetAI_npc_umi_yeti(Creature* pCreature)
{
    return new npc_umi_yetiAI(pCreature);
}

/*####
# npc_earthcaller_franzahl
####*/

#define GOSSIP_EF1                  16398
#define GOSSIP_EF2                  16399
#define GOSSIP_EF3                  16400
#define GOSSIP_EF4                  16401

#define ITEM_ENTRY_BINDINGS_LEFT    18563
#define ITEM_ENTRY_BINDINGS_RIGHT   18564

bool GossipHello_npc_earthcaller_franzahl(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->HasItemCount(ITEM_ENTRY_BINDINGS_LEFT, 1) || player->HasItemCount(ITEM_ENTRY_BINDINGS_RIGHT, 1))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_EF1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(6985, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_earthcaller_franzahl(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_EF2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(6986, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_EF3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(6987, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_EF4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(6988, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->SEND_GOSSIP_MENU(6989, _Creature->GetGUID());
            break;
    }
    return true;
}

void AddSC_winterspring()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_umi_yeti";
    newscript->GetAI = &GetAI_npc_umi_yeti;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_lorax";
    newscript->pGossipHello =  &GossipHello_npc_lorax;
    newscript->pGossipSelect = &GossipSelect_npc_lorax;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_rivern_frostwind";
    newscript->pGossipHello =  &GossipHello_npc_rivern_frostwind;
    newscript->pGossipSelect = &GossipSelect_npc_rivern_frostwind;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_witch_doctor_mauari";
    newscript->pGossipHello =  &GossipHello_npc_witch_doctor_mauari;
    newscript->pGossipSelect = &GossipSelect_npc_witch_doctor_mauari;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_haleh";
    newscript->pGossipHello = &GossipHello_npc_haleh;
    newscript->pGossipSelect = &GossipSelect_npc_haleh;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_artorius_the_amiable";
    newscript->pGossipHello =  &GossipHello_npc_artorius_the_amiable;
    newscript->pGossipSelect = &GossipSelect_npc_artorius_the_amiable;
    newscript->GetAI = &GetAI_npc_artorius_the_amiable;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_ranshalla";
    newscript->GetAI = &GetAI_npc_ranshalla;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_ranshalla;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_elune_fire";
    newscript->pGOUse = &GOUse_go_elune_fire;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_earthcaller_franzahl";
    newscript->pGossipHello =  &GossipHello_npc_earthcaller_franzahl;
    newscript->pGossipSelect = &GossipSelect_npc_earthcaller_franzahl;
    newscript->RegisterSelf();

}

