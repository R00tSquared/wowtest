// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*####
## Game Event: Brewfest
## ID: 26
## Date: Year/09/20 22:00:00 - Year/10/04 00:00:00
## % done: Unknown
## Comment: Info about event - http://www.wowhead.com/forums&topic=41951/brewfest-2008-sep-20-oct-4-quest-guide-information
##          Missing item "Brew of the Month" Club Membership (Alliance / Horde) 
####*/

#include "precompiled.h"

/*####
## npc_brewfest_reveler
####*/

bool ReceiveEmote_npc_brewfest_reveler( Player *player, Creature *_Creature, uint32 emote )
{
    if( emote == TEXTEMOTE_DANCE )
        _Creature->CastSpell(player, 41586, false);

    return true;
}

/*########
# brewfest triggers
#########*/

struct trigger_appleAI : public ScriptedAI
{
    trigger_appleAI(Creature *c) : ScriptedAI(c) {}

    void MoveInLineOfSight(Unit *who)
    {
        if (!who)
            return;

        if (me->IsWithinDistInMap(who, 7.0f) && who->HasAura(43052, 0))
        {
            who->RemoveAurasDueToSpell(43052);
        }
    }
};

CreatureAI* GetAI_trigger_apple(Creature* pCreature)
{
    return new trigger_appleAI(pCreature);
}
/*####
NPC: Pol Amberstill (A), Driz Tumblequick (H)
ID: 24468, 24510
ScriptName: npc_trigger_delivery
Comment: He's trigger for some quest, also he's selling mounts some mounts
####*/

bool GossipHello_npc_trigger_delivery(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu(creature->GetGUID());

    bool canBuy;
    canBuy = false;
    uint32 vendor = creature->GetEntry();
    switch (vendor)
    {
        case 24468: // Alliance
        {
            if (player->GetQuestStatus(11400) == QUEST_STATUS_COMPLETE)
                canBuy = true;
            break;
        }
        case 24510: // Horde
        {
            if (player->GetQuestStatus(11419) == QUEST_STATUS_COMPLETE)
                canBuy = true;
            break;
        }
    }
    if (canBuy)
    {
        if (creature->isVendor())
            player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
    }
    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_trigger_delivery(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST(creature->GetGUID());
    return true;
}

struct trigger_deliveryAI : public ScriptedAI
{
    trigger_deliveryAI(Creature *c) : ScriptedAI(c) {}

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (me->IsWithinDistInMap(who, 20.0f) && (who->HasAura(43880, 0) || who->HasAura(43883, 0)) && ((Player*)who)->HasItemCount(33797, 1))
        {
            who->CastSpell(me, 43662, true);
            who->CastSpell(who, 44601, true);
            ((Player*)who)->DestroyItemCount(33797, 1, true, false);

            if(who->HasAura(43534, 0))
            {
                who->CastSpell(who, 44501, true);
                who->CastSpell(who, 43755, true);
            }
        }
    }
};

CreatureAI* GetAI_trigger_delivery(Creature* pCreature)
{
    return new trigger_deliveryAI(pCreature);
}

struct trigger_delivery_kegAI : public ScriptedAI
{
    trigger_delivery_kegAI(Creature *c) : ScriptedAI(c) {}

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (me->IsWithinDistInMap(who, 20.0f) && (who->HasAura(43880, 0) || who->HasAura(43883, 0)) && !((Player*)who)->HasItemCount(33797, 1))
        {
            who->CastSpell(who, 43660, true);
        }
    }
};

CreatureAI* GetAI_trigger_delivery_keg(Creature* pCreature)
{
    return new trigger_delivery_kegAI(pCreature);
}

bool GossipHello_npc_delivery_daily(Player *player, Creature *_Creature)
{
    if( _Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(!player->HasAura(44689, 0) && (player->GetQuestStatus(11122) == QUEST_STATUS_COMPLETE || player->GetQuestStatus(11412) == QUEST_STATUS_COMPLETE))
    {
        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(0, player->GetSession()->GetHellgroundString(16499), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF, "", 0);
    }

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_delivery_daily(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if(action == GOSSIP_ACTION_INFO_DEF)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 44368, true);
        _Creature->CastSpell(player, 44262, true);
    }

    return true;
}

struct trigger_barkerAI : public ScriptedAI
{
    trigger_barkerAI(Creature *c) : ScriptedAI(c) {}

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (me->IsWithinDistInMap(who, 10.0f) && who->HasAura(43883, 0))
        {
            ((Player*)who)->CastCreatureOrGO(me->GetEntry(), me->GetGUID(), 0);
        }
    }
};

CreatureAI* GetAI_trigger_barker(Creature* pCreature)
{
    return new trigger_barkerAI(pCreature);
}

/*####
# NPC: Wolpertinger
# ID: 22943
# Item: 32233
# Comment:
###*/

struct npc_wolpertingerAI : public ScriptedAI
{
    npc_wolpertingerAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        me->CastSpell(me, 55152, true);
        me->GetMotionMaster()->MoveFollow(me->GetOwner(), 1.5, M_PI/2);
    }
    void EnterCombat(Unit *who) {}
    void UpdateAI(const uint32 diff)
    {
    }
};

CreatureAI* GetAI_npc_wolpertinger(Creature* pCreature)
{
    return new npc_wolpertingerAI(pCreature);
}

void AddSC_brewfest()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_brewfest_reveler";
    newscript->pReceiveEmote = &ReceiveEmote_npc_brewfest_reveler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trigger_apple";
    newscript->GetAI = GetAI_trigger_apple;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trigger_delivery";
    newscript->pGossipHello =  &GossipHello_npc_trigger_delivery;
    newscript->pGossipSelect = &GossipSelect_npc_trigger_delivery;
    newscript->GetAI = GetAI_trigger_delivery;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trigger_delivery_daily";
    newscript->pGossipHello = &GossipHello_npc_delivery_daily;
    newscript->pGossipSelect = &GossipSelect_npc_delivery_daily;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trigger_delivery_keg";
    newscript->GetAI = GetAI_trigger_delivery_keg;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "trigger_barker";
    newscript->GetAI = GetAI_trigger_barker;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_wolpertinger";
    newscript->GetAI = GetAI_npc_wolpertinger;
    newscript->RegisterSelf();
}