// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*####
## Game Event: Love is in the Air
## ID: 8
## Date: Year/02/06 08:00:00 - Year/12/31 01:00:00
## % done: Unknown
## Comment:
####*/

#include "precompiled.h"
#include "GameEvent.h"

#define NPC_GUNTHERS_VISAGE 5666
#define SPELL_NETHER_GEM    7673
#define SAY_GUNTHER_1       -1901086
#define SAY_GUNTHER_2       -1901087
#define SAY_GUNTHER_3       -1901088
#define SAY_BETHOR          -1901089

struct npc_generic_city_residentAI : public ScriptedAI
{
    npc_generic_city_residentAI(Creature *c) : ScriptedAI(c) {}

    Timer EventTimer;
    uint8 Phase;

    uint32 GossipTimer;
    uint32 CDTimer;
    uint32 NPCFlags;
    uint64 playerGUID;

    void Reset()
    {
        EventTimer.Reset(0);
        Phase = 0;

        GossipTimer = 0;
        CDTimer = 0;
        playerGUID = 0;
        NPCFlags = me->GetUInt32Value(UNIT_NPC_FLAGS);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if (eventType == AI_EVENT_CUSTOM_EVENTAI_A && m_creature->GetEntry() == 1498) 
        {
            playerGUID = invoker->GetGUID();
            EventTimer = 1000;
        }
    }

    void GiveItems()
    {
        if(!CDTimer)
        {
            uint32 crFaction = me->getFaction();
            uint32 itemID;
    
            switch (crFaction)
            {
                case 12: // Stormwind
                case 1575: // Stormwind
                {
                    itemID = 22176;
                    break;
                }
                case 57: // Ironforge
                case 55: // Ironforge
                {
                    itemID = 22173;
                    break;
                }
                case 80: // Darnassus
                case 79: // Darnassus
                {
                    itemID = 21960;
                    break;
                }
                case 29: // Orgrimmar
                case 85: // Orgrimmar
                {
                    itemID = 22175;
                    break;
                }
                case 104: // Thunder Bluff
                case 105: // Thunder Bluff
                {
                    itemID = 22177;
                    break;
                }
                case 68: // Uncercity
                case 71: // Undercity
                {
                    itemID = 22174;
                    break;
                }
            }
            // Give item to player
            if(Player* player = Unit::GetPlayerInWorld(playerGUID))
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemID, 1);
                if(msg == EQUIP_ERR_OK)
                    player->StoreNewItem(dest, itemID, 1, true);
            }
            CDTimer = 10000;
        }
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(isGameEventActive(8))
        {
            if (caster->GetTypeId() == TYPEID_PLAYER && spell->Id == 27741)
            {
                if(!CDTimer)
                {
                    if (!me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, NPCFlags | UNIT_NPC_FLAG_GOSSIP);
                        GossipTimer = 5000;
                    }
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventTimer.Expired(diff))
        {
            switch(Phase)
            {
                case 0:
                    m_creature->CastSpell(m_creature, SPELL_NETHER_GEM, false);
                    m_creature->RemoveFlag(UNIT_NPC_FLAGS, NPCFlags);
                    Phase++;
                    EventTimer = 5000;
                    break;
                case 1:
                    m_creature->SummonCreature(NPC_GUNTHERS_VISAGE, 1764.68, 56.32, -46.31, 1.19, TEMPSUMMON_MANUAL_DESPAWN, 0);
                    if (Creature *sum = (Creature*)FindCreature(NPC_GUNTHERS_VISAGE, 20, m_creature))
                    {
                        sum->SetWalk(true);
                        sum->GetMotionMaster()->MovePoint(0, 1765.33, 64.51, -46.32);
                    }
                    Phase++;
                    EventTimer = 5000;
                    break;
                case 2:
                    if (Unit *sum = (Unit*)FindCreature(NPC_GUNTHERS_VISAGE, 20, m_creature))
                    {
                        m_creature->SetFacingTo(m_creature->GetOrientationTo(sum));
                        DoScriptText(SAY_GUNTHER_1, sum);
                    }
                    Phase++;
                    EventTimer = 4000;
                    break;
                case 3:
                    if (Creature *sum = (Creature*)FindCreature(NPC_GUNTHERS_VISAGE, 20, m_creature))
                        DoScriptText(SAY_GUNTHER_2, sum);
                    Phase++;
                    EventTimer = 3000;
                    break;
                case 4:
                    if (Creature *sum = (Creature*)FindCreature(NPC_GUNTHERS_VISAGE, 20, m_creature))
                    {
                        if (Player *plr = Player::GetPlayerInWorld(playerGUID))
                        {
                            sum->SetFacingTo(sum->GetOrientationTo(plr));
                            DoScriptText(SAY_GUNTHER_3, sum, plr);
                        }
                    }
                    Phase++;
                    EventTimer = 5000;
                    break;
                case 5:
                    if (Creature *sum = (Creature*)FindCreature(NPC_GUNTHERS_VISAGE, 20, m_creature))
                        sum->ForcedDespawn(500);
                    
                    DoScriptText(SAY_BETHOR, m_creature);
                    m_creature->SetFacingTo(5.39307);
                    m_creature->SetFlag(UNIT_NPC_FLAGS, NPCFlags);
                    Reset();
                    break;
            }
        }

        if(GossipTimer && !CDTimer)
        {
            if(GossipTimer <= diff)
            {
                if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                {
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    me->SetFlag(UNIT_NPC_FLAGS, NPCFlags);
                }
                GossipTimer = 0;
                CDTimer = 10000;
            }
            else
                GossipTimer -= diff;
        }
        
        if(CDTimer)
        {
            if(CDTimer <= diff)
                CDTimer = 0;
            else
                CDTimer -= diff;
        }

        if (!UpdateVictim() )
            return;

        DoMeleeAttackIfReady();
    }
};

bool GossipHello_npc_generic_city_resident(Player *player, Creature *creature)
{
    if(creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    if (creature->isVendor())
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    if(creature->HasAura(27741, 0))
        if(CAST_AI(npc_generic_city_residentAI, creature->AI())->CDTimer == 0)
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16500), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if(creature->GetEntry() == 6566 && player->GetQuestStatus(1999) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(5060, 1, true))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16501), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    if(creature->GetEntry() == 6179 && 
        !player->HasItemCount(6916, 1, true) && player->GetQuestStatus(1646) != QUEST_STATUS_COMPLETE &&
        player->GetQuestRewardStatus(1645) && !player->GetQuestRewardStatus(1646))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16502), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_generic_city_resident(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_TRADE:
            player->SEND_VENDORLIST(creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            CAST_AI(npc_generic_city_residentAI, creature->AI())->playerGUID = player->GetGUID();
            CAST_AI(npc_generic_city_residentAI, creature->AI())->GiveItems();
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+2:
        {
            creature->CastSpell(player, 9949, false);
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        case GOSSIP_ACTION_INFO_DEF+3:
        {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 6916, 1);
            if (msg == EQUIP_ERR_OK)
                player->StoreNewItem(dest, 6916, true);
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        default: break;
    }
    return true;
}

bool QuestRewarded_npc_generic_city_resident(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pCreature->GetEntry() == 1498 && pQuest->GetQuestId() == 411)
        pCreature->AI()->SendAIEvent(AI_EVENT_CUSTOM_EVENTAI_A, pPlayer, pCreature);

    return true;
}

CreatureAI* GetAI_npc_generic_city_resident(Creature *creature)
{
    return new npc_generic_city_residentAI(creature);
}

bool ReceiveEmote_npc_kwee_q_peddlefeet(Player *player, Creature *creature, uint32 emote)
{
    if (emote == TEXTEMOTE_KISS)
        creature->CastSpell(player, 27572, true);
    return true;
}

void AddSC_loveisintheair()
{
    Script *newscript;
    
    newscript = new Script;
    newscript->Name = "npc_generic_city_resident";
    newscript->pGossipHello =  &GossipHello_npc_generic_city_resident;
    newscript->pGossipSelect = &GossipSelect_npc_generic_city_resident;
    newscript->pQuestRewardedNPC = &QuestRewarded_npc_generic_city_resident;
    newscript->GetAI = GetAI_npc_generic_city_resident;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_kwee_q_peddlefeet";
    newscript->pReceiveEmote = &ReceiveEmote_npc_kwee_q_peddlefeet;
    newscript->RegisterSelf();
}