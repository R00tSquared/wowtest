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
SDName: Npc_Innkeeper
SD%Complete: 50
SDComment: This script are currently not in use. EventSystem cannot be used on Windows build of SD2
SDCategory: NPCs
EndScriptData */

#include "precompiled.h"
#include "GameEvent.h"
#include "Shop.h"

#define HALLOWEEN_EVENTID       12
#define SPELL_TRICK_OR_TREATED  24755
#define SPELL_TREAT             24715

#define LOCALE_TRICK_OR_TREAT_0 "Trick or Treat!"
#define LOCALE_TRICK_OR_TREAT_2 "Des bonbons ou des blagues!"
#define LOCALE_TRICK_OR_TREAT_3 "SьЯes oder Saures!"
#define LOCALE_TRICK_OR_TREAT_6 "ЎTruco o trato!"

bool isEventActive()
{
    return isGameEventActive(HALLOWEEN_EVENTID);
}

bool GossipHello_npc_innkeeper(Player *player, Creature *_Creature)
{

    player->TalkedToCreature(_Creature->GetEntry(),_Creature->GetGUID());

    _Creature->prepareGossipMenu(player,0); //send innkeeper menu too
    
    if (isEventActive()&& !player->GetAura(SPELL_TRICK_OR_TREATED,0))
    {
        const char* localizedEntry;
        switch (player->GetSession()->GetSessionDbLocaleIndex())
        {
            case 0:
                localizedEntry=LOCALE_TRICK_OR_TREAT_0;
                break;
            case 2:
                localizedEntry=LOCALE_TRICK_OR_TREAT_2;
                break;
            case 3:
                localizedEntry=LOCALE_TRICK_OR_TREAT_3;
                break;
            case 6:
                localizedEntry=LOCALE_TRICK_OR_TREAT_6;
                break;
            default:
                localizedEntry=LOCALE_TRICK_OR_TREAT_0;
        }

        player->ADD_GOSSIP_ITEM(0, localizedEntry, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+HALLOWEEN_EVENTID);
    }

    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, player->GetSession()->GetHellgroundString(13004), GOSSIP_SENDER_MAIN, 2001); // Shop main menu
    //
    if (player->HasAura(55193))
        player->RemoveAurasDueToSpell(55193);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_innkeeper(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action < 2000 && player->HasAura(55193))
        player->RemoveAurasDueToSpell(55193);

    if (action == GOSSIP_ACTION_INFO_DEF+HALLOWEEN_EVENTID && isEventActive() && !player->GetAura(SPELL_TRICK_OR_TREATED,0))
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player, SPELL_TRICK_OR_TREATED, true);

        // either trick or treat, 50% chance
        if(rand()%2)
        {
            player->CastSpell(player, SPELL_TREAT, true);
        }
        else
        {
            int32 trickspell=0;
            switch (rand()%9)                               // note that female characters can get male costumes and vice versa
            {
                case 0:
                    trickspell=24753;                       // cannot cast, random 30sec
                    break;
                case 1:
                    trickspell=24713;                       // lepper gnome costume
                    break;
                case 2:
                    trickspell=24735;                       // male ghost costume
                    break;
                case 3:
                    trickspell=24736;                       // female ghostcostume
                    break;
                case 4:
                    trickspell=24710;                       // male ninja costume
                    break;
                case 5:
                    trickspell=24711;                       // female ninja costume
                    break;
                case 6:
                    trickspell=24708;                       // male pirate costume
                    break;
                case 7:
                    trickspell=24709;                       // female pirate costume
                    break;
                case 8:
                    trickspell=24723;                       // skeleton costume
                    break;
            }
            player->CastSpell(player, trickspell, true);
        }
        return true;                                        // prevent core handling
    }
    //Trininty Gossip core handling dont work...
    else if (action == GOSSIP_OPTION_VENDOR)
    {
        player->SEND_VENDORLIST( _Creature->GetGUID() );
    return true;
    }
    else if (action == GOSSIP_OPTION_INNKEEPER)
    {
        player->PlayerTalkClass->CloseGossip();
        player->SetBindPoint( _Creature->GetGUID() );
    return true;
    }
    else if (action == 1665)
    {
        float x, y, z;
        uint32 map;
        QueryResultAutoPtr result = RealmDataDatabase.PQuery("SELECT x, y, z, map FROM guildhouses WHERE guildId = %u LIMIT 1", player->GetGuildId());
        if (result)
        {
            Field *fields = result->Fetch();
            x = fields[0].GetFloat();
            y = fields[1].GetFloat();
            z = fields[2].GetFloat();
            map = fields[3].GetUInt32();
            player->PlayerTalkClass->CloseGossip();
            player->TeleportTo(map, x, y, z, 0.0f);
        }
        else
            _Creature->Whisper(-1200664, LANG_UNIVERSAL, player->GetGUID());
        return true;
    }
    else if (action >= 100000) // accepting item buy
    {
        sWorld.GetShop()->HandleShopDoBuy(_Creature, player, action-100000, sender);
        return true;
    }
    else if (action >= 2000) // in game Shop
    {
        sWorld.GetShop()->SendShopList(_Creature, player, action);
        return true;
    }

    return false;  // no player selection
}

enum
{
    FLEX_NPC_H      = 6929,
    FLEX_QUEST_H    = 8359,
    DANCE_NPC_H     = 6746,
    DANCE_QUEST_H   = 8360,
    TRAIN_NPC_H     = 11814,
    TRAIN_QUEST_H   = 8358,
    CHICKEN_NPC_H   = 6741,
    CHICKEN_QUEST_H = 8354,

    TRAIN_NPC_A     = 6826,
    TRAIN_QUEST_A   = 8355,
    CHICKEN_NPC_A   = 5111,
    CHICKEN_QUEST_A = 8353,
    FLEX_NPC_A      = 6740,
    FLEX_QUEST_A    = 8356,
    DANCE_NPC_A     = 6735,
    DANCE_QUEST_A   = 8357,
};

bool ReceiveEmote_npc_innkeeper(Player* plr, Creature* c, uint32 type)
{
    if (!isEventActive())
        return false;

    switch (c->GetEntry())
    {
    case FLEX_NPC_H:
        if (type == TEXTEMOTE_FLEX)
            plr->AreaExploredOrEventHappens(FLEX_QUEST_H);
        break;
    case FLEX_NPC_A:
        if (type == TEXTEMOTE_FLEX)
            plr->AreaExploredOrEventHappens(FLEX_QUEST_A);
        break;
    case DANCE_NPC_H:
        if (type == TEXTEMOTE_DANCE)
            plr->AreaExploredOrEventHappens(DANCE_QUEST_H);
        break;
    case DANCE_NPC_A:
        if (type == TEXTEMOTE_DANCE)
            plr->AreaExploredOrEventHappens(DANCE_QUEST_A);
        break;
        /* core has no support for sending non-text emotes to script
    case TRAIN_NPC_H:
        //if (type == TEXTEMOTE_?)
            plr->AreaExploredOrEventHappens(TRAIN_QUEST_H);
        break;
    case TRAIN_NPC_A:
        //if (type == TEXTEMOTE_?)
            plr->AreaExploredOrEventHappens(TRAIN_QUEST_A);
        break;
        */
    case CHICKEN_NPC_H:
        if (type == TEXTEMOTE_CHICKEN)
            plr->AreaExploredOrEventHappens(CHICKEN_QUEST_H);
        break;
    case CHICKEN_NPC_A:
        if (type == TEXTEMOTE_CHICKEN)
            plr->AreaExploredOrEventHappens(CHICKEN_QUEST_A);
        break;
    }
    return true;
}

struct npc_innkeeperAI : public ScriptedAI
{
    npc_innkeeperAI(Creature* creature) : ScriptedAI(creature) {}

    Timer TalkEmoteTimer;
    Timer TalkHHTimer;
    Timer CastSpellHHTimer;
    Timer OriTimerHH;

    void Reset()
    {
        if(me->GetEntry() == 18907)
            TalkEmoteTimer.Reset(3000);
        else
            TalkEmoteTimer.Reset(0);

        TalkHHTimer.Reset(0);
        CastSpellHHTimer.Reset(0);
        OriTimerHH.Reset(0);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
            me->Say(-1200663, 0, 0);
        else if(eventType == 8)
        {
            TalkHHTimer = 3000;
            CastSpellHHTimer = 14000;
        }
        else if(eventType == 11)
        {
            me->SetOrientation(me->GetOrientationTo(sender));
            me->SetFacingTo(me->GetOrientationTo(sender));
            switch(urand(0, 1))
            {
                case 0:
                    me->Say(-1200665, 0, 0);
                    break;
                case 1:
                    me->Say(-1200666, 0, 0);
                    break;
            }
            OriTimerHH = 2000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(TalkEmoteTimer.Expired(diff))
            {
                me->HandleEmoteCommand(1);
                TalkEmoteTimer = 17000;
            }

            if(me->GetEntry() == 16826)
            {
                if(TalkHHTimer.Expired(diff))
                {
                    me->Say(-1200667, 0, 0);
                    me->HandleEmote(14);
                    TalkHHTimer = 0;
                }
                if(CastSpellHHTimer.Expired(diff))
                {
                    if(Unit* target = FindCreature(16821, 30, me))
                        target->CastSpell(target, 12980, true);
                    CastSpellHHTimer = 0;
                }
                if(OriTimerHH.Expired(diff))
                {
                    me->SetOrientation(5.044);
                    me->SetFacingTo(5.044);
                    OriTimerHH = 0;
                }
            }
            return;
        }

        DoMeleeAttackIfReady();
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_innkeeper(Creature* creature)
{
    return new npc_innkeeperAI(creature);
}

void AddSC_npc_innkeeper()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="npc_innkeeper";
    newscript->pGossipHello = &GossipHello_npc_innkeeper;
    newscript->pGossipSelect = &GossipSelect_npc_innkeeper;
    newscript->pReceiveEmote = &ReceiveEmote_npc_innkeeper;
    newscript->GetAI = &GetAI_npc_innkeeper;
    newscript->RegisterSelf();
}

