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
SDName: Shadowmoon_Valley
SD%Complete: 100
SDComment: Quest support: 10519, 10583, 10601, 10814, 10804, 10854, 10458, 10481, 10480, 11082, 10781, 10451. Vendor Drake Dealer Hurlunk.
SDCategory: Shadowmoon Valley
EndScriptData */

/* ContentData
mob_mature_netherwing_drake
mob_enslaved_netherwing_drake
npc_drake_dealer_hurlunk
npcs_flanis_swiftwing_and_kagrosh
npc_murkblood_overseer
npc_neltharaku
npc_karynaku
npc_oronok_tornheart
npc_overlord_morghor
npc_earthmender_wilda
mob_torloth_the_magnificent
mob_illidari_spawn
npc_lord_illidan_stormrage
go_crystal_prison
npc_enraged_spirit
npc_overlord_orbarokh
npc_thane_yoregar
npc_shadowmoon_slayer
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"


/*#####
# mob_azaloth
#####*/

#define SPELL_CLEAVE                   40504
#define SPELL_CRIPPLE                  11443
#define SPELL_RAIN_OF_FIRE             38741
#define SPELL_WARSTOMP                 38750
#define SPELL_BANISH                   37833
#define TIME_TO_BANISH                 60000
#define SPELL_VISUAL_BANISH            38722

struct mob_azalothAI : public ScriptedAI
{
    mob_azalothAI(Creature* c) : ScriptedAI(c)   {}

    Timer cleave_timer;
    Timer cripple_timer;
    Timer rain_timer;
    Timer warstomp_timer;
    Timer BanishTimer;

    void EnterCombat()
    {
        DoCast(m_creature->GetVictim(), SPELL_CRIPPLE);
    }

    void Reset()
    {
        cleave_timer.Reset(6000);
        cripple_timer.Reset(18000);
        rain_timer.Reset(15000);
        warstomp_timer.Reset(10000);
        BanishTimer.Reset(1000);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
            BanishTimer.Reset(3000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(BanishTimer.Expired(diff))
        {
            me->RemoveAurasDueToSpell(SPELL_BANISH);
            DoCast(me, SPELL_BANISH);
            std::list<Creature*> warlocks = FindAllCreaturesWithEntry(21503, 25.0f);
            for (std::list<Creature*>::iterator itr = warlocks.begin(); itr != warlocks.end(); ++itr)
            {
                if((*itr)->isAlive() && !(*itr)->IsInCombat())
                {
                    (*itr)->InterruptNonMeleeSpells(true);
                    (*itr)->CastSpell(me, SPELL_VISUAL_BANISH, false);
                }
            }
            BanishTimer.Reset(0);
        }

        if (!UpdateVictim())
            return;

        //spell cleave
        if (cleave_timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_CLEAVE);
            cleave_timer = 6000;
        }

        //spell cripple
        if (cripple_timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_CRIPPLE);
           cripple_timer = 40000;
        }

        //This spell has been disabled due to HUGE spam in logs: 
        //2013-11-10 09:41:01 ERROR: SPELL: no destination for spell ID 38741
        //x 20 0000 000 times... or more
        //spell rain of fire
        if (rain_timer.Expired(diff))
        {
            me->CastSpell(m_creature->GetVictim(), SPELL_RAIN_OF_FIRE, false);
            rain_timer = 15000;
        }

        //spell warstomp
        if (warstomp_timer.Expired(diff))
        {
            DoCast(m_creature->GetVictim(), SPELL_WARSTOMP);
            warstomp_timer= 10000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_azaloth(Creature *_creature)
{
    return new mob_azalothAI(_creature);
}

struct npc_21503AI : public ScriptedAI
{
    npc_21503AI(Creature* c) : ScriptedAI(c)   {}

    Timer ShadowboltTimer;
    Timer VisualCheckTimer;
    Timer IncinerateTimer;

    void Reset()
    {
        ClearCastQueue();
        ShadowboltTimer.Reset(1000);
        VisualCheckTimer.Reset(1000);
        IncinerateTimer.Reset(1000);
    }

    void EnterCombat(Unit* who)
    {
        me->InterruptNonMeleeSpells(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(VisualCheckTimer.Expired(diff))
        {
            if(Unit* Azaloth = FindCreature(21506, 25, me))
            {
                me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, me, 0, 25);
            }
            VisualCheckTimer.Reset(0);
        }

        if (!UpdateVictim())
            return;

        if (ShadowboltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 34344, false);
            ShadowboltTimer = 3000;
        }

        if (IncinerateTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 32707, false);
            IncinerateTimer = urand(5000,6000);
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21503(Creature *_creature)
{
    return new npc_21503AI(_creature);
}

/*#####
# mob_mature_netherwing_drake
#####*/

#define SPELL_PLACE_CARCASS             38439
#define SPELL_JUST_EATEN                38502
#define SPELL_NETHER_BREATH             38467

#define SAY_JUST_EATEN                  -1000222
#define GO_FLAYER_CARCASS               185155

struct mob_mature_netherwing_drakeAI : public npc_escortAI
{
    mob_mature_netherwing_drakeAI(Creature* creature) : npc_escortAI(creature) {}

    Timer_UnCheked CastTimer;

    void Reset()
    {
        CastTimer.Reset(5000);
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 0:
                if (GameObject* go = GetClosestGameObjectWithEntry(me, GO_FLAYER_CARCASS, INTERACTION_DISTANCE))
                    me->SetFacingToObject(go);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_ATTACKUNARMED);
                break;
            case 1:
                DoCast(me, SPELL_JUST_EATEN);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                DoScriptText(SAY_JUST_EATEN, m_creature);
                if (GameObject* go = GetClosestGameObjectWithEntry(me, GO_FLAYER_CARCASS, INTERACTION_DISTANCE))
                    go->Delete();
                if (Player* player = GetPlayerForEscort())
                    player->KilledMonster(22131, m_creature->GetGUID());
                me->GetUnitStateMgr().PushAction(UNIT_ACTION_DOWAYPOINTS);
                break;
        }
    }

     void OnAuraRemove(Aura* aur, bool)
    {
        if(aur->GetId() == SPELL_JUST_EATEN)
            me->setFaction(1824);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(!caster)
            return;

        if(caster->GetTypeId() == TYPEID_PLAYER && spell->Id == SPELL_PLACE_CARCASS && !me->HasAura(SPELL_JUST_EATEN, 0))
        {
            if (caster->ToPlayer()->GetQuestStatus(10804) == QUEST_STATUS_INCOMPLETE)
            {
                float x, y, z;
                caster->GetNearPoint(x, y, z, me->GetObjectSize());
                AddWaypoint(0, x, y, z, 15000);
                AddWaypoint(1, x+0.1f, y-0.1f, z, 1000);
                me->GetRespawnCoord(x, y, z);
                AddWaypoint(2, x, y, z, 5000);
                ((npc_escortAI*)(me->AI()))->SetClearWaypoints(true);
                ((npc_escortAI*)(me->AI()))->SetDespawnAtEnd(false);
                ((npc_escortAI*)(me->AI()))->SetDespawnAtFar(false);
                me->setFaction(35);
                me->GetUnitStateMgr().DropAction(UNIT_ACTION_DOWAYPOINTS);
                Start(false, true, caster->GetGUID());
            }
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CastTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_NETHER_BREATH);
            CastTimer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_mature_netherwing_drake(Creature* creature)
{
    return new mob_mature_netherwing_drakeAI(creature);
}

/*###
# mob_enslaved_netherwing_drake
####*/

#define FACTION_DEFAULT     62
#define FACTION_FRIENDLY    1840                            // Not sure if this is correct, it was taken off of Mordenai.

#define SPELL_HIT_FORCE_OF_NELTHARAKU   38762
#define SPELL_FORCE_OF_NELTHARAKU       38775

#define CREATURE_DRAGONMAW_SUBJUGATOR   21718
#define CREATURE_ESCAPE_DUMMY           22317

struct mob_enslaved_netherwing_drakeAI : public ScriptedAI
{
    mob_enslaved_netherwing_drakeAI(Creature* c) : ScriptedAI(c)
    {
        PlayerGUID = 0;
        Tapped = false;
    }

    uint64 PlayerGUID;
    Timer_UnCheked FlyTimer;
    bool Tapped;

    void Reset()
    {
        if(!Tapped)
            m_creature->setFaction(FACTION_DEFAULT);

        FlyTimer.Reset(10000);
        m_creature->SetLevitate(false);
        m_creature->SetVisibility(VISIBILITY_ON);
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(!caster)
            return;

        if(caster->GetTypeId() == TYPEID_PLAYER && spell->Id == SPELL_HIT_FORCE_OF_NELTHARAKU && !Tapped)
        {
            Tapped = true;
            PlayerGUID = caster->GetGUID();

            m_creature->setFaction(FACTION_FRIENDLY);
            DoCast(caster, SPELL_FORCE_OF_NELTHARAKU, true);

            Unit* Dragonmaw = FindCreature(CREATURE_DRAGONMAW_SUBJUGATOR, 50, m_creature);

            if(Dragonmaw)
            {
                m_creature->AddThreat(Dragonmaw, 100000.0f);
                AttackStart(Dragonmaw);
            }

            HostileReference* ref = m_creature->getThreatManager().getOnlineContainer().getReferenceByTarget(caster);
            if(ref)
                ref->removeReference();
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id == 1)
        {
            if(PlayerGUID)
            {
                Unit* plr = Unit::GetUnit((*m_creature), PlayerGUID);
                if(plr)
                    DoCast(plr, SPELL_FORCE_OF_NELTHARAKU, true);

                PlayerGUID = 0;
            }
            m_creature->SetVisibility(VISIBILITY_OFF);
            m_creature->SetLevitate(false);
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->RemoveCorpse();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(Tapped)
                if(FlyTimer.Expired(diff))
            {
                Tapped = false;
                if(PlayerGUID)
                {
                    Player* plr = Unit::GetPlayerInWorld(PlayerGUID);
                    if(plr && plr->GetQuestStatus(10854) == QUEST_STATUS_INCOMPLETE)
                    {
                        plr->KilledMonster(22316, m_creature->GetGUID());
                        /*
                        float x,y,z;
                        m_creature->GetPosition(x,y,z);

                        float dx,dy,dz;
                        m_creature->GetRandomPoint(x, y, z, 20, dx, dy, dz);
                        dz += 20; // so it's in the air, not ground*/

                        float dx, dy, dz;

                        Unit* EscapeDummy = FindCreature(CREATURE_ESCAPE_DUMMY, 30, m_creature);
                        if(EscapeDummy)
                            EscapeDummy->GetPosition(dx, dy, dz);
                        else
                        {
                            m_creature->GetRandomPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 20, dx, dy, dz);
                            dz += 25;
                        }

                        m_creature->SetLevitate(true);
                        m_creature->GetMotionMaster()->MovePoint(1, dx, dy, dz);
                    }
                }
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_enslaved_netherwing_drake(Creature* _Creature)
{
    return new mob_enslaved_netherwing_drakeAI(_Creature);
}

/*#####
# mob_dragonmaw_peon
#####*/

struct mob_dragonmaw_peonAI : public ScriptedAI
{
    mob_dragonmaw_peonAI(Creature* c) : ScriptedAI(c) {}

    uint64 PlayerGUID;
    bool Tapped;
    Timer_UnCheked PoisonTimer;

    void Reset()
    {
        PlayerGUID = 0;
        Tapped = false;
        PoisonTimer = 0;
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(!caster)
            return;

        if(caster->GetTypeId() == TYPEID_PLAYER && spell->Id == 40468 && !Tapped)
        {
            PlayerGUID = caster->GetGUID();

            Tapped = true;
            float x, y, z;
            caster->GetNearPoint(x, y, z, m_creature->GetObjectSize());

            m_creature->SetWalk(false);
            m_creature->GetMotionMaster()->MovePoint(1, x, y, z);
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id)
        {
            m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_EAT);
            PoisonTimer = 15000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(PoisonTimer.Expired(diff))
        {
            if(PlayerGUID)
            {
                Player* plr = Unit::GetPlayerInWorld(PlayerGUID);
                if(plr && plr->GetQuestStatus(11020) == QUEST_STATUS_INCOMPLETE)
                    plr->KilledMonster(23209, m_creature->GetGUID());
            }
            PoisonTimer = 0;
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_dragonmaw_peon(Creature* _Creature)
{
    return new mob_dragonmaw_peonAI(_Creature);
}

/*######
## npc_drake_dealer_hurlunk
######*/

bool GossipHello_npc_drake_dealer_hurlunk(Player *player, Creature *_Creature)
{
    if (_Creature->isVendor() && player->GetReputationMgr().GetRank(1015) == REP_EXALTED)
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_drake_dealer_hurlunk(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*######
## npc_flanis_swiftwing_and_kagrosh
######*/

#define GOSSIP_HSK1 16019
#define GOSSIP_HSK2 16020

bool GossipHello_npcs_flanis_swiftwing_and_kagrosh(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(10583) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(30658,1,true))
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16019), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    if (player->GetQuestStatus(10601) == QUEST_STATUS_INCOMPLETE && !player->HasItemCount(30659,1,true))
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16020), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    player->SEND_GOSSIP_MENU(25062, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npcs_flanis_swiftwing_and_kagrosh(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 30658, 1, NULL);
        if( msg == EQUIP_ERR_OK )
        {
            player->StoreNewItem( dest, 30658, 1, true);
            player->PlayerTalkClass->ClearMenus();
        }
    }
    if (action == GOSSIP_ACTION_INFO_DEF+2)
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 30659, 1, NULL);
        if( msg == EQUIP_ERR_OK )
        {
            player->StoreNewItem( dest, 30659, 1, true);
            player->PlayerTalkClass->ClearMenus();
        }
    }
    return true;
}

/*######
## npc_grand_commander_ruusk
######*/

#define QUEST_10577    10577

#define GOSSIP_HGCR  16021
#define GOSSIP_SGCR1 16022
#define GOSSIP_SGCR2 16023
#define GOSSIP_SGCR3 16024
#define GOSSIP_SGCR4 16025
#define GOSSIP_SGCR5 16026

bool GossipHello_npc_grand_commander_ruusk(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(QUEST_10577) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16021), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(10401, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_grand_commander_ruusk(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16022), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(10405, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16023), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(10406, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16024), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(10407, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16025), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(10408, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(16026), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(10409, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(QUEST_10577);
            break;
    }
    return true;
}

/*######
## npc_murkblood_overseer
######*/

#define QUEST_11082     11082

#define GOSSIP_HMO  16027
#define GOSSIP_SMO1 16028
#define GOSSIP_SMO2 16029
#define GOSSIP_SMO3 16030
#define GOSSIP_SMO4 16031
#define GOSSIP_SMO5 16032

bool GossipHello_npc_murkblood_overseer(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(QUEST_11082) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HMO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(10940, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_murkblood_overseer(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SMO1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                                                            //correct id not known
            player->SEND_GOSSIP_MENU(10940, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SMO2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
                                                            //correct id not known
            player->SEND_GOSSIP_MENU(10940, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SMO3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
                                                            //correct id not known
            player->SEND_GOSSIP_MENU(10940, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SMO4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
                                                            //correct id not known
            player->SEND_GOSSIP_MENU(10940, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_SMO5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
                                                            //correct id not known
            player->SEND_GOSSIP_MENU(10940, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
                                                            //correct id not known
            player->SEND_GOSSIP_MENU(10940, _Creature->GetGUID());
            _Creature->CastSpell(player,41121,false);
            player->AreaExploredOrEventHappens(QUEST_11082);
            break;
    }
    return true;
}

/*######
## npc_neltharaku
######*/

#define GOSSIP_HN  16033
#define GOSSIP_SN1 16034
#define GOSSIP_SN2 16035
#define GOSSIP_SN3 16036

bool GossipHello_npc_neltharaku(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(10814) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HN), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(10613, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_neltharaku(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(10614, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(10615, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SN3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(10616, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(10814);
            break;
    }
    return true;
}

/*######
## npc_oronok
######*/

#define GOSSIP_ORONOK1 16037
#define GOSSIP_ORONOK2 16038
#define GOSSIP_ORONOK3 16039
#define GOSSIP_ORONOK4 16040
#define GOSSIP_ORONOK5 16041
#define GOSSIP_ORONOK6 16042
#define GOSSIP_ORONOK7 16043

bool GossipHello_npc_oronok_tornheart(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );
    if (_Creature->isVendor())
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    if (player->GetQuestStatus(10519) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ORONOK1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(10312, _Creature->GetGUID());
    }else
    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_oronok_tornheart(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_TRADE:
            player->SEND_VENDORLIST( _Creature->GetGUID() );
            break;
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ORONOK2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(10313, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ORONOK3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(10314, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ORONOK4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(10315, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ORONOK5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(10316, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ORONOK6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(10317, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ORONOK7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(10318, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(10519);
            break;
    }
    return true;
}

/*####
# npc_karynaku
####*/

bool QuestAccept_npc_karynaku(Player* player, Creature* creature, Quest const* quest)
{
    if(quest->GetQuestId() == 10870)                        // Ally of the Netherwing
    {
        std::vector<uint32> nodes;

        nodes.resize(2);
        nodes[0] = 161;                                     // From Karynaku
        nodes[1] = 162;                                     // To Mordenai
        player->ActivateTaxiPathTo(nodes, 20811);
    }
    else if(quest->GetQuestId() == 10866)
    {
        Unit* zuluhed = FindCreature(11980, 200, player);
        if(!zuluhed || (zuluhed && !zuluhed->isAlive()))
        {
            player->SummonCreature(11980, -4204.93652, 316.397369, 122.507774, 1.30899692, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
        }
    }

    return true;
}

/*####
# npc_overlord_morghor
####*/

#define QUEST_LORD_ILLIDAN_STORMRAGE 11108

#define C_ILLIDAN 22083
#define C_YARZILL 23141

#define SPELL_ONE 39990 // Red Lightning Bolt
#define SPELL_TWO 41528 // Mark of Stormrage
#define SPELL_THREE 40216 // Dragonaw Faction
#define SPELL_FOUR 42016 // Dragonaw Trasform

#define OVERLORD_SAY_1 -1000206
#define OVERLORD_SAY_2 -1000207
#define OVERLORD_SAY_3 -1000208
#define OVERLORD_SAY_4 -1000209
#define OVERLORD_SAY_5 -1000210
#define OVERLORD_SAY_6 -1000211

#define OVERLORD_YELL_1 -1000212
#define OVERLORD_YELL_2 -1000213

#define LORD_ILLIDAN_SAY_1 -1000214
#define LORD_ILLIDAN_SAY_2 -1000215
#define LORD_ILLIDAN_SAY_3 -1000216
#define LORD_ILLIDAN_SAY_4 -1000217
#define LORD_ILLIDAN_SAY_5 -1000218
#define LORD_ILLIDAN_SAY_6 -1000219
#define LORD_ILLIDAN_SAY_7 -1000220

#define YARZILL_THE_MERC_SAY -1000221

struct npc_overlord_morghorAI : public ScriptedAI
{
    npc_overlord_morghorAI(Creature *c) : ScriptedAI(c) {}

    uint64 PlayerGUID;
    uint64 IllidanGUID;

    Timer_UnCheked ConversationTimer;
    uint32 Step;
    Timer_UnCheked resetTimer;

    bool Event;

    void Reset()
    {
        PlayerGUID = 0;
        IllidanGUID = 0;

        ConversationTimer = 0;
        Step = 0;

        resetTimer.Reset(180000);

        Event = false;
        m_creature->SetUInt32Value(UNIT_NPC_FLAGS, 2);
    }

    void StartEvent()
    {
        m_creature->SetUInt32Value(UNIT_NPC_FLAGS, 0);
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        Unit* Illidan = m_creature->SummonCreature(C_ILLIDAN, -5107.83, 602.584, 85.2393, 4.92598, TEMPSUMMON_CORPSE_DESPAWN, 0);
        if(Illidan)
        {
            IllidanGUID = Illidan->GetGUID();
            Illidan->SetVisibility(VISIBILITY_OFF);
        }
        if(PlayerGUID)
        {
            Player* player = Unit::GetPlayerInWorld(PlayerGUID);
            if(player)
                DoScriptText(OVERLORD_SAY_1, m_creature, player);
        }
        ConversationTimer = 4200;
        Step = 0;
        Event = true;
    }

    uint32 NextStep(uint32 Step)
    {
        Player* plr = Unit::GetPlayerInWorld(PlayerGUID);

        Creature* Illi = Unit::GetCreature((*m_creature), IllidanGUID);

        if(!plr || (!Illi && Step < 23))
        {
            EnterEvadeMode();
            return 0;
        }

        if (plr->isDead())
        {
            EnterEvadeMode();
            Reset();
            return 0;
        }

        switch(Step)
        {
        case 0: return 0; break;
        case 1: m_creature->GetMotionMaster()->MovePoint(0, -5104.41, 595.297, 85.6838); return 9000; break;
        case 2: DoScriptText(OVERLORD_YELL_1, m_creature, plr); return 4500; break;
        case 3: m_creature->SetInFront(plr); return 3200;  break;
        case 4: DoScriptText(OVERLORD_SAY_2, m_creature, plr); return 2000; break;
        case 5: Illi->SetVisibility(VISIBILITY_ON);
             Illi->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE); return 350; break;
        case 6:
            Illi->CastSpell(Illi, SPELL_ONE, true);
            Illi->SetSelection(m_creature->GetGUID());
            m_creature->SetSelection(IllidanGUID);
            return 2000; break;
        case 7: DoScriptText(OVERLORD_YELL_2, m_creature); return 4500; break;
        case 8: m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 8); return 2500; break;
        case 9: DoScriptText(OVERLORD_SAY_3, m_creature); return 6500; break;
        case 10: DoScriptText(LORD_ILLIDAN_SAY_1, Illi); return 5000;  break;
        case 11: DoScriptText(OVERLORD_SAY_4, m_creature, plr); return 6000; break;
        case 12: DoScriptText(LORD_ILLIDAN_SAY_2, Illi); return 5500; break;
        case 13: DoScriptText(LORD_ILLIDAN_SAY_3, Illi); return 4000; break;
        case 14: Illi->SetSelection(PlayerGUID); return 1500; break;
        case 15: DoScriptText(LORD_ILLIDAN_SAY_4, Illi); return 1500; break;
        case 16:
            if (plr)
            {
                Illi->CastSpell(plr, SPELL_TWO, true);
                plr->RemoveAurasDueToSpell(SPELL_THREE);
                plr->RemoveAurasDueToSpell(SPELL_FOUR);
                return 5000;
            }
            else
            {
             // if !plr we can't do that!
             //   plr->FailQuest(QUEST_LORD_ILLIDAN_STORMRAGE);
                Step = 30; return 100;
            }
            break;
        case 17: DoScriptText(LORD_ILLIDAN_SAY_5, Illi); return 5000; break;
        case 18: DoScriptText(LORD_ILLIDAN_SAY_6, Illi); return 5000; break;
        case 19: DoScriptText(LORD_ILLIDAN_SAY_7, Illi); return 5000; break;
        case 20:
            Illi->HandleEmoteCommand(EMOTE_ONESHOT_LIFTOFF);
            Illi->SetLevitate(true);
            return 500; break;
        case 21: DoScriptText(OVERLORD_SAY_5, m_creature); return 500; break;
        case 22:
            Illi->SetVisibility(VISIBILITY_OFF);
            Illi->setDeathState(JUST_DIED);
            return 1000; break;
        case 23: m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1,0); return 2000; break;
        case 24: m_creature->SetSelection(PlayerGUID); return 5000; break;
        case 25: DoScriptText(OVERLORD_SAY_6, m_creature); return 2000; break;
        case 26:
            if(plr)
                plr->GroupEventHappens(QUEST_LORD_ILLIDAN_STORMRAGE, m_creature);
            return 6000; break;
        case 27:
            {
            Unit* Yarzill = FindCreature(C_YARZILL, 50, m_creature);
            if (Yarzill)
                Yarzill->SetUInt64Value(UNIT_FIELD_TARGET, PlayerGUID);
            return 500; }break;
        case 28:
            plr->RemoveAurasDueToSpell(SPELL_TWO);
            plr->RemoveAurasDueToSpell(41519);
            plr->CastSpell(plr, SPELL_THREE, true);
            plr->CastSpell(plr, SPELL_FOUR, true);
            return 1000; break;
        case 29:
            {
            Unit* Yarzill = FindCreature(C_YARZILL, 50, m_creature);
            if(Yarzill)
                DoScriptText(YARZILL_THE_MERC_SAY, Yarzill, plr);
            return 5000; }break;
        case 30:
            {
            Unit* Yarzill = FindCreature(C_YARZILL, 50, m_creature);
            if (Yarzill)
                Yarzill->SetUInt64Value(UNIT_FIELD_TARGET, 0);
            return 5000; }break;
        case 31:
            {
            Unit* Yarzill = FindCreature(C_YARZILL, 50, m_creature);
			if (Yarzill)
			{
				plr->TeleportTo(530, -5141.35986328125, 620.109985351563, 82.6999969482422, 0);
					Yarzill->CastSpell(plr, 41540, true);
			}
            return 1000;}break;
        case 32: m_creature->GetMotionMaster()->MovePoint(0, -5085.77, 577.231, 86.6719); return 5000; break;
        case 33: EnterEvadeMode(); return 100; break;

        default : return 0;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(ConversationTimer.Expired(diff))
        {
            if(Event && IllidanGUID && PlayerGUID)
            {
                ConversationTimer = NextStep(++Step);
            }
        }
    }
};

CreatureAI* GetAI_npc_overlord_morghorAI(Creature *_Creature)
{
return new npc_overlord_morghorAI(_Creature);
}

bool QuestAccept_npc_overlord_morghor(Player *player, Creature *_Creature, const Quest *_Quest )
{
    if(_Quest->GetQuestId() == QUEST_LORD_ILLIDAN_STORMRAGE)
    {
        ((npc_overlord_morghorAI*)_Creature->AI())->PlayerGUID = player->GetGUID();
        ((npc_overlord_morghorAI*)_Creature->AI())->StartEvent();
        return true;
    }
    return false;
}

/*####
# npc_earthmender_wilda
####*/

#define SAY_START -1000223
#define SAY_AGGRO1 -1000224
#define SAY_AGGRO2 -1000225
#define ASSASSIN_SAY_AGGRO1 -1000226
#define ASSASSIN_SAY_AGGRO2 -1000227
#define SAY_PROGRESS1 -1000228
#define SAY_PROGRESS2 -1000229
#define SAY_PROGRESS3 -1000230
#define SAY_PROGRESS4 -1000231
#define SAY_PROGRESS5 -1000232
#define SAY_PROGRESS6 -1000233
#define SAY_END -1000234

#define QUEST_ESCAPE_FROM_COILSKAR_CISTERN 10451
#define NPC_COILSKAR_ASSASSIN 21044
#define SPELL_CHAIN_LIGHT_EW    16006
#define SPELL_EARTHBIND_T_EW    15786
#define SPELL_FROSTSHOCK_EW     12548
#define SPELL_HEALING_WAVE_EW   12491

struct npc_earthmender_wildaAI : public npc_escortAI
{
    npc_earthmender_wildaAI(Creature *c) : npc_escortAI(c) {}

    bool Completed;

    Timer ChainLight;
    Timer EarthBindTotem;
    Timer Frostshock;
    Timer HealingWave;
    Timer IntroTimer;

    void EnterCombat(Unit *who)
    {
        Player* player = GetPlayerForEscort();

        if(who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == NPC_COILSKAR_ASSASSIN)
            DoScriptText(SAY_AGGRO2, m_creature, player);
        else
            DoScriptText(SAY_AGGRO1, m_creature, player);
    }

    void Reset()
    {
        m_creature->setFaction(1726);
        Completed = false;
        ChainLight.Reset(1000);
        EarthBindTotem.Reset(500);
        Frostshock.Reset(4000);
        HealingWave.Reset(5000);
        IntroTimer.Reset(0);
        if(!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            me->CastSpell(me, 35921, false);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetLevitate(true);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
        {
            me->GetMotionMaster()->MovePoint(1000, -2637.438, 1358.385, 35.923);
            IntroTimer = 4000;
        }
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();
        if (!player)
            return;

        switch(i)
        {
               case 0:
                    DoScriptText(SAY_START, m_creature, player);
                    break;
               case 13: DoScriptText(SAY_PROGRESS1, m_creature, player);
                   SummonAssassin();
                   break;
               case 14: SummonAssassin(); break;
               case 15: DoScriptText(SAY_PROGRESS3, m_creature, player); break;
               case 19:
                   DoScriptText(RAND(SAY_PROGRESS2, SAY_PROGRESS4, SAY_PROGRESS5), m_creature, player);
                   break;
               case 20: SummonAssassin(); break;
               case 26:
                   DoScriptText(RAND(SAY_PROGRESS2, SAY_PROGRESS4, SAY_PROGRESS5), m_creature, player);
                   break;
               case 27: SummonAssassin(); break;
               case 33:
                   DoScriptText(RAND(SAY_PROGRESS2, SAY_PROGRESS4, SAY_PROGRESS5), m_creature, player);
                   break;
               case 34: SummonAssassin(); break;
               case 37:
                   DoScriptText(RAND(SAY_PROGRESS2, SAY_PROGRESS4, SAY_PROGRESS5), m_creature, player);
                   break;
               case 38: SummonAssassin(); break;
               case 39: DoScriptText(SAY_PROGRESS6, m_creature, player); break;
               case 43:
                   DoScriptText(RAND(SAY_PROGRESS2, SAY_PROGRESS4, SAY_PROGRESS5), m_creature, player);
                   break;
               case 44: SummonAssassin(); break;
               case 50:
                   DoScriptText(SAY_END, m_creature, player);
                   player->GroupEventHappens(QUEST_ESCAPE_FROM_COILSKAR_CISTERN, m_creature);
                   Completed = true;
                   break;
               }
       }

       void SummonAssassin()
       {
           Player* player = GetPlayerForEscort();

           Unit* CoilskarAssassin = m_creature->SummonCreature(NPC_COILSKAR_ASSASSIN, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
           if( CoilskarAssassin )
           {
               DoScriptText(RAND(ASSASSIN_SAY_AGGRO1, ASSASSIN_SAY_AGGRO2), CoilskarAssassin, player);

               ((Creature*)CoilskarAssassin)->AI()->AttackStart(m_creature);
           }
           else error_log("TSCR ERROR: Coilskar Assassin couldn't be summmoned");
       }

       void JustDied(Unit* killer)
       {
           if (!Completed)
           {
               Player* player = GetPlayerForEscort();
               if (player)
                   player->FailQuest(QUEST_ESCAPE_FROM_COILSKAR_CISTERN);
           }
       }

       void UpdateAI(const uint32 diff)
       {
           if(IntroTimer.Expired(diff))
           {
                me->RemoveAurasDueToSpell(35921);
                me->Say(-1200093, 0, 0);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetLevitate(false);
                IntroTimer = 0;
           }

            npc_escortAI::UpdateAI(diff);

            if(!UpdateVictim())
                return;

            if(ChainLight.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_CHAIN_LIGHT_EW, false);
                ChainLight = urand(5000, 7000);
            }
            if(EarthBindTotem.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_EARTHBIND_T_EW, false);
                EarthBindTotem = urand(10000, 20000);
            }
            if(Frostshock.Expired(diff))
            {
                DoCast(me->GetVictim(), SPELL_FROSTSHOCK_EW, false);
                Frostshock = urand(10000, 20000);
            }
            if(HealingWave.Expired(diff))
            {
                if(me->HealthBelowPct(40))
                    DoCast(me->GetVictim(), SPELL_HEALING_WAVE_EW, false);
                HealingWave = urand(10000, 20000);
            }
       }
};

CreatureAI* GetAI_npc_earthmender_wildaAI(Creature *_Creature)
{
    npc_earthmender_wildaAI* earthmender_wildaAI = new npc_earthmender_wildaAI(_Creature);

       earthmender_wildaAI->AddWaypoint(0, -2637.466064, 1359.977905, 35.889114, 2000); // SAY_START
       earthmender_wildaAI->AddWaypoint(1, -2666.364990, 1348.222656, 34.445557);
       earthmender_wildaAI->AddWaypoint(2, -2693.789307, 1336.964966, 34.445557);
       earthmender_wildaAI->AddWaypoint(3, -2715.495361, 1328.054443, 34.106014);
       earthmender_wildaAI->AddWaypoint(4, -2742.530762, 1314.138550, 33.606144);
       earthmender_wildaAI->AddWaypoint(5, -2745.077148, 1311.108765, 33.630898);
       earthmender_wildaAI->AddWaypoint(6, -2749.855225, 1302.737915, 33.475632);
       earthmender_wildaAI->AddWaypoint(7, -2753.639648, 1294.059448, 33.314930);
       earthmender_wildaAI->AddWaypoint(8, -2756.796387, 1285.122192, 33.391262);
       earthmender_wildaAI->AddWaypoint(9, -2750.042969, 1273.661987, 33.188259);
       earthmender_wildaAI->AddWaypoint(10, -2740.378418, 1258.846680, 33.212521);
       earthmender_wildaAI->AddWaypoint(11, -2733.629395, 1248.259766, 33.640598);
       earthmender_wildaAI->AddWaypoint(12, -2727.212646, 1238.606445, 33.520847);
       earthmender_wildaAI->AddWaypoint(13, -2726.377197, 1237.264526, 33.461823, 4000); // SAY_PROGRESS1
       earthmender_wildaAI->AddWaypoint(14, -2746.383301, 1266.390625, 33.191952, 2000);
       earthmender_wildaAI->AddWaypoint(15, -2746.383301, 1266.390625, 33.191952, 4000); // SAY_PROGRESS3
       earthmender_wildaAI->AddWaypoint(16, -2758.927734, 1285.134155, 33.341728);
       earthmender_wildaAI->AddWaypoint(17, -2761.845703, 1292.313599, 33.209042);
       earthmender_wildaAI->AddWaypoint(18, -2758.871826, 1300.677612, 33.285332);
       earthmender_wildaAI->AddWaypoint(19, -2758.871826, 1300.677612, 33.285332);
       earthmender_wildaAI->AddWaypoint(20, -2753.928955, 1307.755859, 33.452457);
       earthmender_wildaAI->AddWaypoint(20, -2738.612061, 1316.191284, 33.482975);
       earthmender_wildaAI->AddWaypoint(21, -2727.897461, 1320.013916, 33.381111);
       earthmender_wildaAI->AddWaypoint(22, -2709.458740, 1315.739990, 33.301838);
       earthmender_wildaAI->AddWaypoint(23, -2704.658936, 1301.620361, 32.463303);
       earthmender_wildaAI->AddWaypoint(24, -2704.120117, 1298.922607, 32.768162);
       earthmender_wildaAI->AddWaypoint(25, -2691.798340, 1292.846436, 33.852642);
       earthmender_wildaAI->AddWaypoint(26, -2682.879639, 1288.853882, 32.995399);
       earthmender_wildaAI->AddWaypoint(27, -2661.869141, 1279.682495, 26.686783);
       earthmender_wildaAI->AddWaypoint(28, -2648.943604, 1270.272827, 24.147522);
       earthmender_wildaAI->AddWaypoint(29, -2642.506836, 1262.938721, 23.512444);
       earthmender_wildaAI->AddWaypoint(20, -2636.984863, 1252.429077, 20.418257);
       earthmender_wildaAI->AddWaypoint(31, -2648.113037, 1224.984863, 8.691818);
       earthmender_wildaAI->AddWaypoint(32, -2658.393311, 1200.136719, 5.492243);
       earthmender_wildaAI->AddWaypoint(33, -2668.504395, 1190.450562, 3.127407);
       earthmender_wildaAI->AddWaypoint(34, -2685.930420, 1174.360840, 5.163924);
       earthmender_wildaAI->AddWaypoint(35, -2701.613770, 1160.026367, 5.611311);
       earthmender_wildaAI->AddWaypoint(36, -2714.659668, 1149.980347, 4.342373);
       earthmender_wildaAI->AddWaypoint(37, -2721.443359, 1145.002808, 1.913474);
       earthmender_wildaAI->AddWaypoint(38, -2733.962158, 1143.436279, 2.620415);
       earthmender_wildaAI->AddWaypoint(39, -2757.876709, 1146.937500, 6.184002, 2000); // SAY_PROGRESS6
       earthmender_wildaAI->AddWaypoint(40, -2772.300537, 1166.052734, 6.331811);
       earthmender_wildaAI->AddWaypoint(41, -2790.265381, 1189.941650, 5.207958);
       earthmender_wildaAI->AddWaypoint(42, -2805.448975, 1208.663940, 5.557623);
       earthmender_wildaAI->AddWaypoint(43, -2820.617676, 1225.870239, 6.266103);
       earthmender_wildaAI->AddWaypoint(44, -2831.926758, 1237.725830, 5.808506);
       earthmender_wildaAI->AddWaypoint(45, -2842.578369, 1252.869629, 6.807481);
       earthmender_wildaAI->AddWaypoint(46, -2846.344971, 1258.727295, 7.386168);
       earthmender_wildaAI->AddWaypoint(47, -2847.556396, 1266.771729, 8.208790);
       earthmender_wildaAI->AddWaypoint(48, -2841.654541, 1285.809204, 7.933223);
       earthmender_wildaAI->AddWaypoint(49, -2841.754883, 1289.832520, 6.990304);
       earthmender_wildaAI->AddWaypoint(50, -2871.398438, 1302.348145, 6.807335, 8000); // SAY_END

       return (CreatureAI*)earthmender_wildaAI;
}

bool QuestAccept_npc_earthmender_wilda(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ESCAPE_FROM_COILSKAR_CISTERN)
    {
        creature->setFaction(FACTION_ESCORT_N_NEUTRAL_ACTIVE);
        creature->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        creature->GetMotionMaster()->Initialize();
        if (npc_earthmender_wildaAI* pEscortAI = CAST_AI(npc_earthmender_wildaAI, creature->AI()))
            pEscortAI->Start(true, false, player->GetGUID(), quest);
    }
    return true;
}

/*#####
# Quest: Battle of the crimson watch
#####*/

/* ContentData
Battle of the crimson watch - creatures, gameobjects and defines
mob_illidari_spawn : Adds that are summoned in the Crimson Watch battle.
mob_torloth_the_magnificent : Final creature that players have to face before quest is completed
npc_lord_illidan_stormrage : Creature that controls the event.
go_crystal_prison : GameObject that begins the event and hands out quest
EndContentData */

#define END_TEXT -1000366

#define QUEST_BATTLE_OF_THE_CRIMSON_WATCH 10781
#define EVENT_AREA_RADIUS 65 //65yds
#define EVENT_COOLDOWN 30000 //in ms. appear after event completed or failed (should be = Adds despawn time)

struct TorlothCinematic
{
    int32 TextId;
    uint32 Creature, Timer;
};

// Creature 0 - Torloth, 1 - Illidan
static TorlothCinematic TorlothAnim[]=
{
    {-1000367, 0, 2000},
    {-1000368, 1, 7000},
    {-1000369, 0, 3000},
    {0, 0, 2000}, // Torloth stand
    {-1000370, 0, 1000},
    {0, 0, 3000},
    {0, 0, 0}
};

struct Location
{
    float x, y, z, o;
};

//Cordinates for Spawns
static Location SpawnLocation[]=
{
    //Cords used for:
    {-4615.4453, 1354.9262, 139.9, 1.579},//Illidari Soldier
    {-4598.9365, 1377.3182, 139.9, 3.917},//Illidari Soldier
    {-4598.4697, 1360.8999, 139.9, 2.427},//Illidari Soldier
    {-4589.3599, 1369.1061, 139.9, 3.165},//Illidari Soldier
    {-4608.3477, 1386.0076, 139.9, 4.108},//Illidari Soldier
    {-4633.1889, 1359.8033, 139.9, 0.949},//Illidari Soldier
    {-4623.5791, 1351.4574, 139.9, 0.971},//Illidari Soldier
    {-4607.2988, 1351.6099, 139.9, 2.416},//Illidari Soldier
    {-4633.7764, 1376.0417, 139.9, 5.608},//Illidari Soldier
    {-4600.2461, 1369.1240, 139.9, 3.056},//Illidari Mind Breaker
    {-4631.7808, 1367.9459, 139.9, 0.020},//Illidari Mind Breaker
    {-4600.2461, 1369.1240, 139.9, 3.056},//Illidari Highlord
    {-4631.7808, 1367.9459, 139.9, 0.020},//Illidari Highlord
    {-4615.4453, 1354.9262, 139.9, 1.579},//Illidari Highlord
    {-4616.4736, 1384.2170, 139.9, 4.971},//Illidari Highlord
    {-4627.1240, 1378.8752, 139.9, 2.544} //Torloth The Magnificent
};

struct WaveData
{
    uint8 SpawnCount, UsedSpawnPoint;
    uint32 CreatureId;
    int32 WaveTextId;
};

static WaveData WavesInfo[]=
{
    {9, 0, 22075, -1000371},   //Illidari Soldier
    {2, 9, 22074, -1000372},   //Illidari Mind Breaker
    {4, 11, 19797, -1000373},  //Illidari Highlord
    {1, 15, 22076, -1000374}   //Torloth The Magnificent
};

struct SpawnSpells
{
 uint32 Timer1, Timer2, SpellId;
};

static SpawnSpells SpawnCast[]=
{
    {10000, 15000, 35871},  // Illidari Soldier Cast - Spellbreaker
    {10000, 10000, 38985},  // Illidari Mind Breake Cast - Focused Bursts
    {35000, 35000, 22884},  // Illidari Mind Breake Cast - Psychic Scream
    {20000, 20000, 17194},  // Illidari Mind Breake Cast - Mind Blast
    {8000, 15000, 38010},   // Illidari Highlord Cast - Curse of Flames
    {12000, 20000, 16102},  // Illidari Highlord Cast - Flamestrike
    {10000, 15000, 15284},  // Torloth the Magnificent Cast - Cleave
    {18000, 20000, 39082},  // Torloth the Magnificent Cast - Shadowfury
    {25000, 28000, 33961}   // Torloth the Magnificent Cast - Spell Reflection
};

/*######
# mob_illidari_spawn
######*/

struct mob_illidari_spawnAI : public ScriptedAI
{
    mob_illidari_spawnAI(Creature* c) : ScriptedAI(c) {}

    uint64 LordIllidanGUID;
    Timer_UnCheked SpellTimer1, SpellTimer2, SpellTimer3;
    bool Timers;

    void Reset()
    {
        LordIllidanGUID = 0;
        Timers = false;
    }

    void EnterCombat(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(!Timers)
        {
            if(m_creature->GetEntry() == 22075)//Illidari Soldier
            {
                SpellTimer1 = SpawnCast[0].Timer1 + (rand()%4 * 1000);
            }
            if(m_creature->GetEntry() == 22074)//Illidari Mind Breaker
            {
                SpellTimer1 = SpawnCast[1].Timer1 + (rand()%10 * 1000);
                SpellTimer2 = SpawnCast[2].Timer1 + (rand()%4 * 1000);
                SpellTimer3 = SpawnCast[3].Timer1 + (rand()%4 * 1000);
            }
            if(m_creature->GetEntry() == 19797)// Illidari Highlord
            {
                SpellTimer1 = SpawnCast[4].Timer1 + (rand()%4 * 1000);
                SpellTimer2 = SpawnCast[5].Timer1 + (rand()%4 * 1000);
            }
            Timers = true;
        }
        //Illidari Soldier
        if(m_creature->GetEntry() == 22075)
        {
            if(SpellTimer1.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SpawnCast[0].SpellId);//Spellbreaker
                SpellTimer1 = SpawnCast[0].Timer2 + (rand()%5 * 1000);
            }
        }
        //Illidari Mind Breaker
        if(m_creature->GetEntry() == 22074)
        {
            if(SpellTimer1.Expired(diff))
            {
                if(Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0))
                {
                    if(target->GetTypeId() == TYPEID_PLAYER)
                    {
                        DoCast(target, SpawnCast[1].SpellId); //Focused Bursts
                        SpellTimer1 = SpawnCast[1].Timer2 + (rand()%5 * 1000);
                    }
                    else
                        SpellTimer1 = 2000;
                }
            }

            if(SpellTimer2.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SpawnCast[2].SpellId);//Psychic Scream
                SpellTimer2 = SpawnCast[2].Timer2 + (rand()%13 * 1000);
            }

            if(SpellTimer3.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SpawnCast[3].SpellId);//Mind Blast
                SpellTimer3 = SpawnCast[3].Timer2 + (rand()%8 * 1000);
            }
        }
        //Illidari Highlord
        if(m_creature->GetEntry() == 19797)
        {
            if(SpellTimer1.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SpawnCast[4].SpellId);//Curse Of Flames
                SpellTimer1 = SpawnCast[4].Timer2 + (rand()%10 * 1000);
            }

            if(SpellTimer2.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SpawnCast[5].SpellId);//Flamestrike
                SpellTimer2 = SpawnCast[5].Timer2 + (rand()%7 * 13000);
            }
        }

        DoMeleeAttackIfReady();
    }
};

/*######
# mob_torloth_the_magnificent
#####*/

struct mob_torloth_the_magnificentAI : public ScriptedAI
{
    mob_torloth_the_magnificentAI(Creature* c) : ScriptedAI(c) {}

    Timer_UnCheked AnimationTimer, SpellTimer1, SpellTimer2, SpellTimer3;

    uint8 AnimationCount;

    uint64 LordIllidanGUID;
    uint64 AggroTargetGUID;

    bool Timers;

    void Reset()
    {
        AnimationTimer.Reset(4000);
        AnimationCount = 0;
        LordIllidanGUID = 0;
        AggroTargetGUID = 0;
        Timers = false;

        m_creature->addUnitState(UNIT_STAT_ROOT);
        m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetSelection(0);
		m_creature->SetReactState(REACT_PASSIVE);
    }

    void HandleAnimation()
    {
        Creature* pCreature = m_creature;

        if(TorlothAnim[AnimationCount].Creature == 1)
        {
            pCreature = (Unit::GetCreature(*m_creature, LordIllidanGUID));

            if(!pCreature)
                return;
        }

        if(TorlothAnim[AnimationCount].TextId)
            DoScriptText(TorlothAnim[AnimationCount].TextId, pCreature);

        AnimationTimer = TorlothAnim[AnimationCount].Timer;

        switch(AnimationCount)
        {
        case 0:
            m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1,8);
            break;
        case 3:
            m_creature->RemoveFlag(UNIT_FIELD_BYTES_1,8);
            break;
        case 5:
            if(Player* AggroTarget = (Unit::GetPlayerInWorld(AggroTargetGUID)))
            {
                m_creature->SetSelection(AggroTarget->GetGUID());
                m_creature->AddThreat(AggroTarget, 1);
                m_creature->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
            }
            break;
        case 6:
            if(Player* AggroTarget = (Unit::GetPlayerInWorld(AggroTargetGUID)))
            {
                m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                m_creature->ClearUnitState(UNIT_STAT_ROOT);
				m_creature->SetReactState(REACT_AGGRESSIVE);

                float x, y, z;
                AggroTarget->GetPosition(x,y,z);
                m_creature->GetMotionMaster()->MovePoint(0,x,y,z);
            }
            break;
        }
        ++AnimationCount;
    }

    void UpdateAI(const uint32 diff)
    {
        if(AnimationTimer.Expired(diff))
            HandleAnimation();

        if(AnimationCount < 6)
        {
            m_creature->CombatStop();
        }
        else if(!Timers)
        {

            SpellTimer1 = SpawnCast[6].Timer1;
            SpellTimer2 = SpawnCast[7].Timer1;
            SpellTimer3 = SpawnCast[8].Timer1;
            Timers = true;
        }

        if(Timers)
        {
            if(SpellTimer1.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SpawnCast[6].SpellId);//Cleave
                SpellTimer1 = SpawnCast[6].Timer2 + (rand()%10 * 1000);
            }

            if(SpellTimer2.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SpawnCast[7].SpellId);//Shadowfury
                SpellTimer2 = SpawnCast[7].Timer2 + (rand()%5 * 1000);
            }

            if(SpellTimer3.Expired(diff))
            {
                DoCast(m_creature, SpawnCast[8].SpellId);
                SpellTimer3 = SpawnCast[8].Timer2 + (rand()%7 * 1000);//Spell Reflection
            }
        }

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* slayer)
    {
        if(slayer)
            switch(slayer->GetTypeId())
        {
            case TYPEID_UNIT:
                if(((Creature*)slayer)->isPet() && ((Pet*)slayer)->GetOwner()->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)((Pet*)slayer->GetOwner()))->GroupEventHappens(QUEST_BATTLE_OF_THE_CRIMSON_WATCH, m_creature);
                break;

            case TYPEID_PLAYER:
                ((Player*)slayer)->GroupEventHappens(QUEST_BATTLE_OF_THE_CRIMSON_WATCH, m_creature);
                break;
        }

        if(Creature* LordIllidan = (Unit::GetCreature(*m_creature, LordIllidanGUID)))
        {
            DoScriptText(END_TEXT, LordIllidan, slayer);
            LordIllidan->AI()->EnterEvadeMode();
        }
    }
};

/*#####
# npc_lord_illidan_stormrage
#####*/

struct npc_lord_illidan_stormrageAI : public ScriptedAI
{
    npc_lord_illidan_stormrageAI(Creature* c) : ScriptedAI(c), summons(c) {}

    uint64 PlayerGUID;

    Timer_UnCheked WaveTimer;
    Timer_UnCheked AnnounceTimer;

    SummonList summons;
    uint8 WaveCount;

    bool EventStarted;

    void Reset()
    {
        PlayerGUID = 0;

        WaveTimer.Reset(10000);
        AnnounceTimer.Reset(7000);
        summons.DespawnAll();
        WaveCount = 0;

        EventStarted = false;
        m_creature->SetVisibility(VISIBILITY_OFF);
    }

    void MoveInLineOfSight(Unit* who) {}
    void AttackStart(Unit* who) {}

    void SummonNextWave()
    {
        uint8 count = WavesInfo[WaveCount].SpawnCount;
        uint8 locIndex = WavesInfo[WaveCount].UsedSpawnPoint;
        uint8 FelguardCount = 0;
        uint8 DreadlordCount = 0;

        for(uint8 i = 0; i < count; ++i)
        {
            Creature* Spawn = NULL;
            float X = SpawnLocation[locIndex + i].x;
            float Y = SpawnLocation[locIndex + i].y;
            float Z = SpawnLocation[locIndex + i].z;
            float O = SpawnLocation[locIndex + i].o;
            Spawn = m_creature->SummonCreature(WavesInfo[WaveCount].CreatureId, X, Y, Z, O, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);

            if(Spawn)
            {
                summons.Summon(Spawn);
                Spawn->LoadCreaturesAddon();

                if(WaveCount == 0)//1 Wave
                {
                    if (!urand(0, 2) && FelguardCount < 2)
                    {
                        Spawn->SetUInt32Value(UNIT_FIELD_DISPLAYID,18654);
                        ++FelguardCount;
                    }
                    else if (DreadlordCount < 3)
                    {
                        Spawn->SetUInt32Value(UNIT_FIELD_DISPLAYID,19991);
                        ++DreadlordCount;
                    }
                    else if (FelguardCount < 2)
                    {
                        Spawn->SetUInt32Value(UNIT_FIELD_DISPLAYID,18654);
                        ++FelguardCount;
                    }
                }

                if(WaveCount < 3)//1-3 Wave
                {
                    if(PlayerGUID)
                    {
                        if(Player* pTarget = Unit::GetPlayerInWorld(PlayerGUID))
                        {
                            float x, y, z;
                            pTarget->GetPosition(x,y,z);
                            Spawn->GetMotionMaster()->MovePoint(0,x, y, z);
                        }
                    }
                    ((mob_illidari_spawnAI*)Spawn->AI())->LordIllidanGUID = m_creature->GetGUID();
                }

                if(WavesInfo[WaveCount].CreatureId == 22076) // Torloth
                {
                    ((mob_torloth_the_magnificentAI*)Spawn->AI())->LordIllidanGUID = m_creature->GetGUID();
                    if(PlayerGUID)
                        ((mob_torloth_the_magnificentAI*)Spawn->AI())->AggroTargetGUID = PlayerGUID;
                }
            }
        }
        ++WaveCount;

        WaveTimer = 10000;
        AnnounceTimer = 7000;
    }

    bool CheckEventFail()
    {
        Player* pPlayer = Unit::GetPlayerInWorld(PlayerGUID);

        if(!pPlayer)
            return true;

        if(Group *EventGroup = pPlayer->GetGroup())
        {
            bool failed = true;

            const Group::MemberSlotList members = EventGroup->GetMemberSlots();

            Player* GroupMember = NULL;
            for(Group::member_citerator itr = members.begin(); itr!= members.end(); itr++)
            {
                GroupMember = (Unit::GetPlayerInWorld(itr->guid));
                if(!GroupMember)
                    continue;

                if(GroupMember->IsWithinDistInMap(m_creature, EVENT_AREA_RADIUS) && GroupMember->isAlive())
                    failed = false;
            }

            if(failed)
            {
                for(Group::member_citerator itr = members.begin(); itr!= members.end(); itr++)
                {
                    GroupMember = Unit::GetPlayerInWorld(itr->guid);

                    if(GroupMember && GroupMember->GetQuestStatus(QUEST_BATTLE_OF_THE_CRIMSON_WATCH) == QUEST_STATUS_INCOMPLETE)
                    {
                        GroupMember->FailQuest(QUEST_BATTLE_OF_THE_CRIMSON_WATCH);
                        GroupMember->SetQuestStatus(QUEST_BATTLE_OF_THE_CRIMSON_WATCH, QUEST_STATUS_NONE);
                    }
                }
                return true;
            }
        }
        else if (pPlayer->isDead() || !pPlayer->IsWithinDistInMap(m_creature, EVENT_AREA_RADIUS))
        {
            pPlayer->FailQuest(QUEST_BATTLE_OF_THE_CRIMSON_WATCH);
            return true;
        }
        return false;
    }

    void SummonedCreatureDespawn(Creature* despawned)
    {
        summons.Despawn(despawned);
    }

    void UpdateAI(const uint32 diff)
    {
        //there are checks bellow that *SHOULD* check for fail etc etc but i don't have time to read that wall of text
        if (!PlayerGUID || !EventStarted)
            return;

        if (CheckEventFail())
            EnterEvadeMode();


        if(summons.empty() && WaveCount < 4)
        {
            if(AnnounceTimer.Expired(diff))
            {
                DoScriptText(WavesInfo[WaveCount].WaveTextId, m_creature);
                AnnounceTimer = 0;
            }

            if(WaveTimer.Expired(diff))
                SummonNextWave();
        }
    }
};

/*#####
# go_crystal_prison
######*/

bool GOQuestAccept_GO_crystal_prison(Player* plr, GameObject* go, Quest const* quest)
{
    if(quest->GetQuestId() == QUEST_BATTLE_OF_THE_CRIMSON_WATCH )
    {
        Unit* Illidan = FindCreature(22083, 50, plr);

        if(Illidan && !(((npc_lord_illidan_stormrageAI*)((Creature*)Illidan)->AI())->EventStarted))
        {
            ((npc_lord_illidan_stormrageAI*)((Creature*)Illidan)->AI())->Reset();
            ((npc_lord_illidan_stormrageAI*)((Creature*)Illidan)->AI())->PlayerGUID = plr->GetGUID();
            ((npc_lord_illidan_stormrageAI*)((Creature*)Illidan)->AI())->EventStarted=true;
        }
    }
 return true;
}

CreatureAI* GetAI_npc_lord_illidan_stormrage(Creature* c)
{
    return new npc_lord_illidan_stormrageAI(c);
}

CreatureAI* GetAI_mob_illidari_spawn(Creature* c)
{
    return new mob_illidari_spawnAI(c);
}

CreatureAI* GetAI_mob_torloth_the_magnificent(Creature* c)
{
    return new mob_torloth_the_magnificentAI(c);
}

/*####
# npc_enraged_spirits
####*/

/* QUESTS */
#define QUEST_ENRAGED_SPIRITS_FIRE_EARTH 10458
#define QUEST_ENRAGED_SPIRITS_AIR 10481
#define QUEST_ENRAGED_SPIRITS_WATER 10480

/* Totem */
#define ENTRY_TOTEM_OF_SPIRITS 21071
#define RADIUS_TOTEM_OF_SPIRITS 15

/* SPIRITS */
#define ENTRY_ENRAGED_EARTH_SPIRIT 21050
#define ENTRY_ENRAGED_FIRE_SPIRIT 21061
#define ENTRY_ENRAGED_AIR_SPIRIT 21060
#define ENTRY_ENRAGED_WATER_SPIRIT 21059

/* SOULS */
#define ENTRY_EARTHEN_SOUL 21073
#define ENTRY_FIERY_SOUL 21097
#define ENTRY_ENRAGED_AIRY_SOUL 21116
#define ENTRY_ENRAGED_WATERY_SOUL 21109  // wrong model

/* SPELL KILLCREDIT - not working!?! - using KilledMonster */
#define SPELL_EARTHEN_SOUL_CAPTURED_CREDIT 36108
#define SPELL_FIERY_SOUL_CAPTURED_CREDIT 36117
#define SPELL_AIRY_SOUL_CAPTURED_CREDIT 36182
#define SPELL_WATERY_SOUL_CAPTURED_CREDIT 36171

/* KilledMonster Workaround */
#define CREDIT_FIRE 21094
#define CREDIT_WATER 21095
#define CREDIT_AIR 21096
#define CREDIT_EARTH 21092

/* Captured Spell/Buff */
#define SPELL_SOUL_CAPTURED 36115
#define SPELL_STORM_BOLT    38032

/* Factions */
#define ENRAGED_SOUL_FRIENDLY 35
#define ENRAGED_SOUL_HOSTILE 14

struct npc_enraged_spiritAI : public ScriptedAI
{
    npc_enraged_spiritAI(Creature *c) : ScriptedAI(c) {}

    Timer Spell1Timer;
    Timer Spell2Timer;
    Timer StormBoltTimer;

    void Reset()
    {
        switch(me->GetEntry())
        {
            case 21060:
                Spell1Timer.Reset(1000);
                Spell2Timer.Reset(4000);
                break;
            case 21059:
                StormBoltTimer.Reset(urand(2000, 5000));
                break;
            default: 
                Spell1Timer.Reset(0);
                Spell2Timer.Reset(0); 
                StormBoltTimer.Reset(0); 
                break;
        }
    }

    void EnterCombat(Unit *who){}

    void JustDied(Unit* killer)
    {
        // always spawn spirit on death
        // if totem around
        // move spirit to totem and cast kill count
        uint32 entry = 0;
        uint32 credit = 0;

        switch(m_creature->GetEntry()) {
          case ENTRY_ENRAGED_FIRE_SPIRIT:
            entry  = ENTRY_FIERY_SOUL;
            //credit = SPELL_FIERY_SOUL_CAPTURED_CREDIT;
            credit = CREDIT_FIRE;
          break;
          case ENTRY_ENRAGED_EARTH_SPIRIT:
            entry  = ENTRY_EARTHEN_SOUL;
            //credit = SPELL_EARTHEN_SOUL_CAPTURED_CREDIT;
            credit = CREDIT_EARTH;
          break;
          case ENTRY_ENRAGED_AIR_SPIRIT:
            entry  = ENTRY_ENRAGED_AIRY_SOUL;
            //credit = SPELL_AIRY_SOUL_CAPTURED_CREDIT;
            credit = CREDIT_AIR;
          break;
          case ENTRY_ENRAGED_WATER_SPIRIT:
            entry  = ENTRY_ENRAGED_WATERY_SOUL;
            //credit = SPELL_WATERY_SOUL_CAPTURED_CREDIT;
            credit = CREDIT_WATER;
          break;
        }

        // Spawn Soul on Kill ALWAYS!
        Creature* Summoned = NULL;
        Unit* totemOspirits = NULL;

        if ( entry != 0 )
            Summoned = DoSpawnCreature(entry, 0, 0, 1, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 5000);

        // FIND TOTEM, PROCESS QUEST
        if (Summoned)
        {
             totemOspirits = FindCreature(ENTRY_TOTEM_OF_SPIRITS, RADIUS_TOTEM_OF_SPIRITS, m_creature);
             if (totemOspirits)
             {
                 Summoned->setFaction(ENRAGED_SOUL_FRIENDLY);
                 Summoned->GetMotionMaster()->MovePoint(0,totemOspirits->GetPositionX(), totemOspirits->GetPositionY(), Summoned->GetPositionZ());

                 Player* Owner = (Player*)totemOspirits->GetOwner();
                 if (Owner)
                     // DoCast(Owner, credit); -- not working!
                     Owner->KilledMonster(credit, Summoned->GetGUID());
                 DoCast(totemOspirits,SPELL_SOUL_CAPTURED);
             }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        if (Spell1Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 12058, false);
            Spell1Timer = urand(3000, 4000);
        }

        if (Spell2Timer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 32717, false);
            Spell2Timer = urand(10000, 15000);
        }

        if (StormBoltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), SPELL_STORM_BOLT, false);
            StormBoltTimer = urand(4000, 6000);
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_enraged_spirit(Creature *_Creature)
{
return new npc_enraged_spiritAI(_Creature);
}
/*#####
# Akama's cutscene after quest 10628 & Akama BT Prelude after quest 10944
######*/

/*TODO
 - improve Olum's Spirit animation
 - set exact NPC emotes into the db
 - find and setup green glowing flames spell seen with Illidan appearence
*/

// Cutscene after quest 10628 dialogs
#define SAY_DIALOG_VAGATH_1 -1200094
#define SAY_WHISPER_AKAMA_2 -1200095
#define SAY_DIALOG_AKAMA_3  -1200096
#define SAY_DIALOG_VAGATH_4 -1200097
#define SAY_DIALOG_AKAMA_5  -1200098
#define SAY_DIALOG_MAIEV_6  -1200099
#define SAY_DIALOG_AKAMA_7  -1200100
#define SAY_DIALOG_MAIEV_8  -1200101

// Cutscene after quest 10628 creatures & spells
#define VAGATH                   21768
#define ILLIDARI_SUCCUBUS        22860
#define MAIEV_SHADOWSONG         21699
#define SPELL_FAKE_KILL_VISUAL   37071      // not blizzlike, blizzlike unknown
#define SPELL_RESURECTION_VISUAL 21074      // not blizzlike, blizzlike unknown

// Cutscene after quest 10628 postions data
static float VagathPos[4] = {-3726.75,1038.05,55.95,4.60};
static float SuccubPos1[4] = {-3723.55,1041.40,55.95,4.60};
static float SuccubPos2[4] = {-3730.46,1041.40,55.95,4.60};

// BT prelude after quest 10944 dialogs & music
#define SAY_DIALOG_OLUM_1           -1563012
#define SAY_DIALOG_PRE_AKAMA_1      -1563001
#define SAY_DIALOG_OLUM_2           -1563013
#define SAY_DIALOG_PRE_AKAMA_2      -1563002
#define SAY_DIALOG_OLUM_3           -1563014
#define SAY_DIALOG_PRE_AKAMA_3      -1563003
#define SAY_DIALOG_OLUM_4           -1563015
#define SAY_DIALOG_PRE_AKAMA_4      -1563004
#define SAY_DIALOG_OLUM_5           -1563016
#define SAY_DIALOG_PRE_AKAMA_5      -1563005
#define SAY_DIALOG_PRE_AKAMA_6      -1563006
#define SAY_DIALOG_ILLIDAN_1        -1563009
#define SAY_DIALOG_PRE_AKAMA_7      -1563007
#define SAY_DIALOG_ILLIDAN_2        -1563010
#define SAY_DIALOG_ILLIDAN_3        -1563011
#define SAY_DIALOG_PRE_AKAMA_8      -1563008
#define BLACK_TEMPLE_PRELUDE_MUSIC     11716
#define ILLIDAN_APPEARING               5756

// BT prelude after quest 10944 creatures & spells & emotes
#define ILLIDAN                     22083
#define SEER_OLUM                   22820
#define OLUMS_SPIRIT                22870
#define SPELL_OLUMS_SACRIFICE       39552
#define STATE_DROWNED                 383

// BT prelude after quest 10944 postions data
static float OlumPos[4] = {-3729.17,1035.63,55.95,5.82};
static float OlumNewPos[4] = {-3721.87,1031.86,55.95,5.90};
static float AkamaPos[4] = {-3714.50,1028.95,55.95,2.57};
static float AkamaNewPos[4] = {-3718.33,1030.27,55.95,2.77};





struct npc_AkamaAI : public ScriptedAI
{
    npc_AkamaAI(Creature* c) : ScriptedAI(c) {}

    uint64 VagathGUID;
    uint64 Succub1GUID;
    uint64 Succub2GUID;

    uint64 OlumGUID;
    uint64 IllidanGUID;
    uint64 OlumSpiritGUID;

    Timer_UnCheked TalkTimer;
    uint32 Step;

    std::list<uint64> targets;

    bool EventStarted;
    bool PreludeEventStarted;

    void Reset()
    {
        VagathGUID = 0;
        Step = 0;

        TalkTimer = 0;
        EventStarted = false;
        targets.clear();
    }

    void PreludeReset()
    {
        OlumGUID = 0;
        IllidanGUID = 0;
        Step = 0;

        TalkTimer = 0;
        PreludeEventStarted = false;

    }

    void BuildNearbyUnitsList()
    {
        float range = 20.0f;
        std::list<Unit*> tempTargets;
        Hellground::AnyUnitInObjectRangeCheck check(m_creature, range);
        Hellground::UnitListSearcher<Hellground::AnyUnitInObjectRangeCheck> searcher(tempTargets, check);
        Cell::VisitAllObjects(me, searcher, range);
        for (std::list<Unit*>::iterator iter = tempTargets.begin(); iter != tempTargets.end(); ++iter)
            if ((*iter)->GetTypeId() == TYPEID_PLAYER)
                targets.push_back((*iter)->GetGUID());
    }


    void StartEvent()
    {
        Step = 1;
        EventStarted = true;

        Creature* Vagath = m_creature->SummonCreature(VAGATH,VagathPos[0],VagathPos[1],VagathPos[2],VagathPos[3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,0);
        Creature* Succub1 = m_creature->SummonCreature(ILLIDARI_SUCCUBUS,SuccubPos1[0],SuccubPos1[1],SuccubPos1[2],SuccubPos1[3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,0);
        Creature* Succub2 = m_creature->SummonCreature(ILLIDARI_SUCCUBUS,SuccubPos2[0],SuccubPos2[1],SuccubPos2[2],SuccubPos2[3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,0);

        if (!Vagath || !Succub1 || !Succub2)
            return;

        VagathGUID = Vagath->GetGUID();
        Succub1GUID = Succub1->GetGUID();
        Succub2GUID = Succub2->GetGUID();

        Vagath->setFaction(35);
        TalkTimer = 3000;

        BuildNearbyUnitsList();
    }

    uint32 NextStep(uint32 Step)
    {
        Unit* vaga = Unit::GetUnit((*m_creature),VagathGUID);
        Unit* Succub1 = Unit::GetUnit((*m_creature),Succub1GUID);
        Unit* Succub2 = Unit::GetUnit((*m_creature),Succub2GUID);
        Unit* maiev = FindCreature(MAIEV_SHADOWSONG, 50, m_creature);

        switch(Step)
        {
            case 0:
            return 0;

            case 1:
                if (vaga)
                    ((Creature*)vaga)->Say(-1200094,LANG_UNIVERSAL,0);
                return 3000;

            case 2:
                for (std::list<uint64>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
                {
                    if (Unit * target = me->GetUnit(*iter))
                        DoWhisper(-1200095, target);

                }
                return 1000;

            case 3:
                for (std::list<uint64>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
                {
                    if (Unit * target = me->GetUnit(*iter))
                        DoCast(target, SPELL_FAKE_KILL_VISUAL);

                }
                return 1000;

            case 4:
                for (std::list<uint64>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
                {
                    if (Unit * target = me->GetUnit(*iter))
                    {
                        target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                        target->SetHealth(1);
                        ((Player*)target)->setRegenTimer(60000);
                        target->GetUnitStateMgr().PushAction(UNIT_ACTION_STUN);
                    }
                }
                return 3000;

            case 5:
                m_creature->Say(-1200096,LANG_UNIVERSAL,0);
                return 12000;

            case 6:
                if (vaga)
                    ((Creature*)vaga)->Say(-1200097,LANG_UNIVERSAL,0);
                return 15000;

            case 7:
                if (vaga)
                    ((Creature*)vaga)->setDeathState(CORPSE);
                if (Succub1)
                    ((Creature*)Succub1)->setDeathState(CORPSE);
                if (Succub2)
                    ((Creature*)Succub2)->setDeathState(CORPSE);
                return 3000;

            case 8:
                for (std::list<uint64>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
                {
                    if (Unit * target = me->GetUnit(*iter))
                        DoCast(target, SPELL_RESURECTION_VISUAL);
                }
                return 2000;

            case 9:
                for (std::list<uint64>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
                {
                    if (Unit * target = me->GetUnit(*iter))
                    {
                        target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                        target->SetHealth(target->GetMaxHealth());
                        target->GetUnitStateMgr().DropAction(UNIT_ACTION_STUN);
                    }
                }
                m_creature->Say(-1200098, LANG_UNIVERSAL, 0);
                return 12000;

            case 10:
                if (maiev)
                    ((Creature*)maiev)->Say(-1200099,LANG_UNIVERSAL,0);
                return 12000;

            case 11:
                m_creature->Say(-1200100,LANG_UNIVERSAL,0);
                return 12000;

            case 12:
                if (maiev)
                    ((Creature*)maiev)->Say(-1200101,LANG_UNIVERSAL,0);
                return 1000;

            case 13:
                Reset();
                return 100;

            default:
            return 0;
        }
        return 0;
    }

    void StartPreludeEvent()
    {
        Step = 1;
        PreludeEventStarted = true;

        DoPlaySoundToSet(m_creature,BLACK_TEMPLE_PRELUDE_MUSIC);

        Creature* Olum = m_creature->SummonCreature(SEER_OLUM,OlumPos[0],OlumPos[1],OlumPos[2],OlumPos[3],TEMPSUMMON_CORPSE_TIMED_DESPAWN,45000);    // despawn corpse after 45 seconds - Blizzlike

        if (!Olum)
            return;

        OlumGUID = Olum->GetGUID();
        DoScriptText(SAY_DIALOG_OLUM_1,Olum);

        Olum->MonsterMoveWithSpeed(OlumNewPos[0],OlumNewPos[1],OlumNewPos[2],5000,true);

        TalkTimer = 13000;
    }

    uint32 PreludeNextStep(uint32 Step)
    {
        Unit* olum = Unit::GetUnit((*m_creature),OlumGUID);
        Unit* Illidan = Unit::GetUnit((*m_creature), IllidanGUID);

        switch(Step)
        {
            case 0:
                return 0;

            case 1:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_1,m_creature);
                return 4000;
            case 2:
                if (olum)
                    DoScriptText(SAY_DIALOG_OLUM_2,(Creature*)olum);
                return 8000;
            case 3:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_2,m_creature);
                return 7000;
            case 4:
                if (olum)
                    DoScriptText(SAY_DIALOG_OLUM_3,(Creature*)olum);
                return 27500;
            case 5:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_3,m_creature);
                return 5000;
            case 6:
                if (olum)
                    DoScriptText(SAY_DIALOG_OLUM_4,(Creature*)olum);
                return 16000;
            case 7:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_4,m_creature);
                return 8000;
            case 8:
                if (olum)
                    DoScriptText(SAY_DIALOG_OLUM_5,(Creature*)olum);
                return 14500;
            case 9:
                m_creature->MonsterMoveWithSpeed(AkamaNewPos[0],AkamaNewPos[1],AkamaNewPos[2],2000,true);
                return 2500;
            case 10:
                if (olum)
                    DoCast(olum,SPELL_OLUMS_SACRIFICE);
                return 6800;
            case 11:
                if (olum)
                {
                    olum->setDeathState(JUST_DIED);
                    if (Creature* spirit = m_creature->SummonCreature(OLUMS_SPIRIT,OlumNewPos[0],OlumNewPos[1],OlumNewPos[2],OlumNewPos[3]-2.0f,TEMPSUMMON_TIMED_DESPAWN,16000))
                    {
                        spirit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        spirit->SetLevitate(true);
                        // spirit->SetUInt32Value(UNIT_NPC_EMOTESTATE,STATE_DROWNED); // improve Olum's Spirit animation using Drowned State, right movement flag or monster move type needed
                        spirit->MonsterMoveWithSpeed(OlumNewPos[0],OlumNewPos[1],OlumNewPos[2]+8.0f,16000,true);
                    }
                }
                return 7000;
            case 12:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_5,m_creature);
                return 12000;
            case 13:
                m_creature->MonsterMoveWithSpeed((AkamaPos[0]+0.1f), (AkamaPos[1]-0.1f), AkamaPos[2], 2000,true);
                return 2100;
            case 14:
                m_creature->MonsterMoveWithSpeed((AkamaPos[0]-0.05f), (AkamaPos[1]), AkamaPos[2], 200,true);    // just to turn back Akama to Illidan
                return 6000;
            case 15:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_6,m_creature);
                m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE,68);
                return 200;
            case 16:
                if (Illidan)
                    DoPlaySoundToSet(Illidan,ILLIDAN_APPEARING);
                return 7000;
            case 17:
                if (Illidan)
                    DoScriptText(SAY_DIALOG_ILLIDAN_1,(Creature*)Illidan);
                return 14000;
            case 18:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_7,m_creature);
                return 19000;
            case 19:
                if (Illidan)
                    DoScriptText(SAY_DIALOG_ILLIDAN_2,(Creature*)Illidan);
                return 21000;
            case 20:
                if (Illidan)
                    DoScriptText(SAY_DIALOG_ILLIDAN_3,(Creature*)Illidan);
                return 22000;
            case 21:
                DoScriptText(SAY_DIALOG_PRE_AKAMA_8,m_creature);
                return 1000;
            case 22:
                if (Illidan)
                    Illidan->setDeathState(CORPSE);
                return 1000;
            case 23:
                PreludeReset();
                return 100;
            default:
            return 0;
        }
        return 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventStarted && VagathGUID)
        {
            if (TalkTimer.Expired(diff))
                TalkTimer = NextStep(Step++);
        }

        if(PreludeEventStarted && OlumGUID)
        {
            if (Step == 16 && !IllidanGUID && TalkTimer.Passed())
            {
                Creature* Illidan = m_creature->SummonCreature(ILLIDAN,OlumNewPos[0]-3.0f,OlumNewPos[1]+0.5f,OlumNewPos[2],OlumNewPos[3],TEMPSUMMON_CORPSE_DESPAWN,0);
                Illidan->SetFloatValue(OBJECT_FIELD_SCALE_X,0.65f);
                Illidan->SetVisibility(VISIBILITY_ON);
                Illidan->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NOT_SELECTABLE);
                IllidanGUID = Illidan->GetGUID();
            }

            if (TalkTimer.Expired(diff))
                TalkTimer = PreludeNextStep(Step++);
        }
    }
};

CreatureAI* GetAI_npc_Akama(Creature *_Creature)
{
    return new npc_AkamaAI(_Creature);
}
bool ChooseReward_npc_Akama(Player *player, Creature *_Creature, const Quest *_Quest)
{
    bool EventStarted = ((npc_AkamaAI*)_Creature->AI())->EventStarted;
    bool PreludeEventStarted = ((npc_AkamaAI*)_Creature->AI())->PreludeEventStarted;

    if (EventStarted || PreludeEventStarted)
        return false;

    if (!EventStarted && _Quest->GetQuestId() == 10628)
        ((npc_AkamaAI*)_Creature->AI())->StartEvent();

    if (!PreludeEventStarted && _Quest->GetQuestId() == 10944)
        ((npc_AkamaAI*)_Creature->AI())->StartPreludeEvent();

    return false;
}

/*#####
# Quest: The Ata'mal Terrace
#####*/

/* ContentData
npc_shadowlord_trigger : trigger that brings Deathwail when dies, spawns trash
mob_shadowlord_deathwail : main boss, spams fel seeds when soulstelers aggroed, goes down when trigger dies
mob_shadowmoon_soulstealer : not moves, when aggro spawns Retainers
felfire_summoner : invisible NPC to set fel fireball visuals
TO DO: make Heart of Fury GO despawn when Deathwail lands
TO DO: re-check all timers and "crush" test event
EndContentData */

/*#####
# npc_shadowlord_trigger
######*/

#define NPC_RETAINER_ID    22102
#define NPC_SOULSTEALER_ID 22061

struct npc_shadowlord_triggerAI : public Scripted_NoMovementAI
{
    npc_shadowlord_triggerAI(Creature* c) : Scripted_NoMovementAI(c)
    {
        x = m_creature->GetPositionX();
        y = m_creature->GetPositionY();
        z = m_creature->GetPositionZ();
    }

    Timer Check_Timer;
    Timer Wave_Timer;
    Timer Reset_Timer;
    uint32 counterAlive;         //is alive counter
    uint32 counterCombar;        //is in combat counter
    float x, y, z;

    static const int32
        SpawnX = -3249,
        SpawnY = 347,
        SpawnZ = 127;

    void Reset()
    {
        Reset_Timer = 0;
        Check_Timer = 1;
        Wave_Timer = 1;
        m_creature->Relocate(x, y, z);
    }

    void EnterCombat(Unit* who)
    {
        m_creature->GetMotionMaster()->Clear();
        m_creature->GetMotionMaster()->MoveIdle();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(Check_Timer.Expired(diff))
        {
            counterAlive = 0;
            counterCombar = 0;
            m_creature->CallAssistance();
            std::list<Creature*> SoulstealerList = FindAllCreaturesWithEntry(NPC_SOULSTEALER_ID, 80.0f);
            if(!SoulstealerList.empty())
                for(std::list<Creature*>::iterator i = SoulstealerList.begin(); i != SoulstealerList.end(); ++i)
                {
                    if((*i)->isAlive())
                        counterAlive++;
                    if((*i)->IsInCombat())
                        ++counterCombar;
                }
            Check_Timer = 5000;

            float NewX, NewY, NewZ;
            m_creature->GetRandomPoint(x, y, z, 14.0, NewX, NewY, NewZ);
            NewZ = 137.15;  //normalize Z
            DoTeleportTo(NewX, NewY, NewZ);
            m_creature->Relocate(NewX, NewY, NewZ);
            
            if(!counterCombar && counterAlive && !Reset_Timer.GetInterval())
                Reset_Timer = 20000;
                
            if (m_creature->GetVictim()->GetDistance(m_creature) >= 100)
                Reset();
        }

        if(counterAlive)
        {
            if(Wave_Timer.Expired(diff))
            {
                float x,y,z;
                for(uint8 i = 0; i < 3; ++i)
                {
                    m_creature->GetRandomPoint(SpawnX,SpawnY,SpawnZ,3.0f,x,y,z);
                    z = SpawnZ;
                    Unit *Retainer = m_creature->SummonCreature(NPC_RETAINER_ID,x,y,z,m_creature->GetOrientation(),
                    TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,80000);
                    Retainer->GetMotionMaster()->MoveIdle();
                    Retainer->GetMotionMaster()->MovePath(2096+i, false);
                }
                Wave_Timer = 25000;
            }
        }
        else
        {
            Unit* HeartVisual = FindCreature(22058, 80.0, m_creature);
            if(HeartVisual)
            {
                HeartVisual->Kill(HeartVisual, false);
                ((Creature*)HeartVisual)->RemoveCorpse();
            }
            m_creature->Kill(m_creature, false);
            m_creature->RemoveCorpse();
        }


        if (Reset_Timer.Expired(diff))
        {
            Reset();
            Reset_Timer = 0;
        }
        
    }
};

CreatureAI* GetAI_npc_shadowlord_trigger(Creature *_Creature)
{
    return new npc_shadowlord_triggerAI(_Creature);
}

/*#####
# mob_shadowlord_deathwail
######*/

#define DEATHWAIL_FLYPATH            2095
#define SPELL_SHADOWBOLT            12471
#define SPELL_SHADOWBOLT_VALLEY     15245
#define SPELL_DEATHCOIL             32709
#define SPELL_FEAR                  27641
#define SPELL_FEL_FIREBALL          38312

// Trentone - this script needs rework. 'flying' is weird
struct mob_shadowlord_deathwailAI : public ScriptedAI
{
    mob_shadowlord_deathwailAI(Creature* c) : ScriptedAI(c) {}

    Timer_UnCheked Check_Timer;
    Timer_UnCheked Patrol_Timer;
    Timer_UnCheked Shadowbolt_Timer;
    Timer_UnCheked ShadowboltVoley_Timer;
    Timer_UnCheked Fear_Timer;
    Timer_UnCheked Deathcoil_Timer;

    uint64 HeartGUID;

    bool flying;
    bool felfire;

    void Reset()
    {
        flying = true;
        felfire = false;

        m_creature->SetHomePosition(-3247, 284, 138.1, 2.33);

        Check_Timer.Reset(2000);
        Shadowbolt_Timer.Reset(4000);
        ShadowboltVoley_Timer.Reset(13000);
        Fear_Timer.Reset(20000);
        Deathcoil_Timer.Reset(8000);

        m_creature->GetMotionMaster()->Clear();

        Unit* trigger = FindCreature(22096, 100, m_creature);

        if (trigger && !trigger->isAlive())
        { 
            m_creature->GetMotionMaster()->MovePoint(1, -3247, 284, 138.1);
            m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            m_creature->SetReactState(REACT_AGGRESSIVE);
        }
        
        else 
        {
            m_creature->GetMotionMaster()->Initialize();
            m_creature->SetLevitate(true);
            m_creature->GetMotionMaster()->MovePath(DEATHWAIL_FLYPATH, true);
            m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
            m_creature->SetReactState(REACT_PASSIVE); 
        }

        ClearCastQueue();
    }

    void EnterCombat(Unit* who) 
    {
        if (flying) 
            m_creature->SetUInt64Value(UNIT_FIELD_TARGET, 0);
    }

    void AttackStart(Unit* who)
    {
        if(flying)
            return;

        else if (m_creature->Attack(who, true))
        {
            m_creature->AddThreat(who, 0.0f);
            m_creature->SetInCombatWith(who);
            who->SetInCombatWith(m_creature);

            DoStartMovement(who);
        }
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        if (flying) 
            m_creature->SetUInt64Value(UNIT_FIELD_TARGET, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (Check_Timer.Expired(diff))
        {
            Unit* trigger = FindCreature(22096, 100, m_creature);

            if (trigger && !trigger->isAlive() && flying && felfire)
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(1, -3247, 284, 138.1);
                m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                m_creature->SetReactState(REACT_AGGRESSIVE);

                felfire = false;
            }

            if (trigger && !trigger->isAlive() && flying && m_creature->GetPositionZ() < 142)
            {
                m_creature->GetMotionMaster()->MoveIdle();
                m_creature->SetLevitate(false);
                m_creature->Unmount();
                m_creature->SetSpeed(MOVE_RUN, 1.0);

                flying = false;
            }

            if (felfire && trigger && !trigger->IsInCombat())
                felfire = false;

            if (felfire)
                DoCast(m_creature, SPELL_FEL_FIREBALL);
            
            if (trigger && trigger->isAlive() && !m_creature->IsInCombat() && !flying)
            {
                EnterEvadeMode();
                return;
            }

            if (trigger && trigger->isAlive() && !trigger->IsInCombat() && m_creature->IsInCombat())
            {
                EnterEvadeMode();
                return;
            }

            Check_Timer = 5000;
        }

        if (flying || !UpdateVictim())
            return;

        if (Shadowbolt_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_SHADOWBOLT);
            Shadowbolt_Timer = 12000+rand()%6000;
        }

        if (ShadowboltVoley_Timer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_SHADOWBOLT);
            ShadowboltVoley_Timer = 25000+rand()%15000;
        }

        if (Fear_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0, true))
                AddSpellToCast(target, SPELL_FEAR);
            Fear_Timer = 10000+rand()%20000;
        }

        if (Deathcoil_Timer.Expired(diff))
        {
            if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0, 30.0, true, m_creature->getVictimGUID()))
                AddSpellToCast(target, SPELL_DEATHCOIL);
            else
                AddSpellToCast(m_creature->GetVictim(), SPELL_DEATHCOIL);
            Deathcoil_Timer = 15000+rand()%30000;
        }      

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();

    }
};

CreatureAI* GetAI_mob_shadowlord_deathwail(Creature *_Creature)
{
    return new mob_shadowlord_deathwailAI(_Creature);
}

/*#####
# mob_shadowmoon_soulstealer
######*/

struct mob_shadowmoon_soulstealerAI : public Scripted_NoMovementAI
{
    mob_shadowmoon_soulstealerAI(Creature* c) : Scripted_NoMovementAI(c) {}

    void Reset()
    {
        DoCast(m_creature, 38250);
    }

    void MoveInLineOfSight(Unit *who)
    {
        std::list<Unit*> party;

        if(!m_creature->IsInCombat() && who->GetTypeId() == TYPEID_PLAYER  && m_creature->IsWithinDistInMap(who, 15.0f))
        {
            who->GetPartyMember(party, 50.0f);
            for(std::list<Unit*>::iterator i = party.begin(); i != party.end(); ++i)
            {
                AttackStart(*i);
            }
        }

    }

    void EnterCombat(Unit* who)
    {
        m_creature->GetUnitStateMgr().PushAction(UNIT_ACTION_ROOT);
        m_creature->SetUInt64Value(UNIT_FIELD_TARGET, ObjectGuid());
        m_creature->CombatStart(who);
        if(Unit* Deathwail = FindCreature(22006, 100.0, m_creature))
            ((mob_shadowlord_deathwailAI*)((Creature*) Deathwail)->AI())->felfire = true;
        if(Unit* trigger = FindCreature(22096, 100.0, m_creature))
            ((npc_shadowlord_triggerAI*)((Creature*) trigger)->AI())->AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_mob_shadowmoon_soulstealer(Creature *_Creature)
{
    return new mob_shadowmoon_soulstealerAI(_Creature);
}

/*#####
# felfire_summoner
######*/

struct felfire_summonerAI : public NullCreatureAI
{
    felfire_summonerAI(Creature *c) : NullCreatureAI(c) {}
};

CreatureAI* GetAI_felfire_summoner(Creature *_Creature)
{
    return new felfire_summonerAI (_Creature);
}

/*#####
# Quest: A Distraction for Akama
#####*/

/* ContentData

npc_maiev_BT_attu
npc_akama_BT_attu
npc_ashtongue_deathsworn
mob_vagath
mob_illidari_shadowlord
npc_xiri
Xi'ri gossips

EndContentData */

//NPC Entries
#define NPC_MAIEV_SHADOWSONG        22989
#define NPC_AKAMA_BT                22990
#define NPC_ASHTONGUE_DEATHSWORN    21701
#define MOB_VAGATH                  23152
#define MOB_ILLIDARI_SHADOWLORD     22988

//NPC Spells
#define SPELL_FAN_OF_BLADES         39954
#define SPELL_STEALTH               34189
#define CHAIN_LIGHTNING             39945
#define SPELL_BLINGING_LIGHT        36950   //not used
#define SPELL_LIGHT_OF_THE_NAARU_1  39828
#define SPELL_LIGHT_OF_THE_NAARU_2  39831   //spell_linked
#define SPELL_CARRION_SWARM         39942
#define SPELL_INFERNO               39941
#define SPELL_SLEEP                 12098

//NPC text's
#define MAIEV_YELL        -1200102
#define AKAMA_START       -1200103
#define AKAMA_YELL        -1200104
#define AKAMA_KILL        -1200105
#define VAGATH_AGGRO      -1200106
#define VAGATH_INTRO      -1200107
#define VAGATH_DEATH      -1200108
#define XIRI_GOSSIP_HELLO        16044
#define XIRI_GOSSIP_RESTORE_ITEM 16045

//NPC spawn positions and Waypoints
static float MaievBT[4] =
{
    -3554.0, 740.0, -15.4, 4.70
};

static float MaievWaypoint[][3] =
{
    {-3554.0, 731.0, -15.0},
    {-3554.0, 700.0, - 9.3},
    {-3554.0, 650.0,  1.71},
    {-3554.0, 600.0,  9.30},
    {-3554.0, 540.0,  16.5},
    {-3561.8, 523.0,  17.9},
    {-3556.4, 490.0,  22.0},
    {-3570.0, 462.0,  24.5},
    {-3576.6, 428.7,  28.6},
    {-3586.0, 414.2,  31.2},
    {-3593.0, 382.6,  34.0},
    {-3599.1, 362.4,  35.2},
    {-3606.4, 345.7,  39.3},
    {-3633.1, 327.7,  38.8}
};

static float AkamaBT[4] =
{
    -3570.2, 684.5, -5.22, 4.70
};

static float AkamaWaypoint[][3] =
{
    {-3570.2, 654.5, 0.76},
    {-3570.2, 624.5, 5.78},
    {-3570.2, 594.5, 9.71},
    {-3570.2, 564.5, 12.7},
    {-3559.8, 534.5, 16.9},
    {-3559.8, 473.0, 23.3},
    {-3568.5, 423.0, 28.4},
    {-3568.0, 375.0, 32.7},
    {-3614.5, 330.2, 40.3},
    {-3644.6, 315.4, 37.4}
};

static float Deathsworn[8][4] =
{
    {-3561.0, 720.0, -12.0, 1.56},
    {-3557.0, 730.0, -13.6, 1.56},
    {-3553.0, 735.0, -15.8, 1.56},
    {-3549.0, 740.0, -16.7, 1.56},
    {-3546.0, 745.0, -16.7, 1.56},
    {-3565.0, 733.0, -13.0, 1.56},
    {-3569.0, 738.0, -12.6, 1.56},
    {-3573.0, 743.0, -11.9, 1.56}
};

static float DeathswornPath[8][3] =
{
    {-3561.0, 600.0, 9.20},
    {-3557.0, 610.0, 8.00},
    {-3553.0, 615.0, 7.60},
    {-3549.0, 620.0, 7.30},
    {-3545.0, 625.0, 7.10},
    {-3565.0, 613.0, 7.40},
    {-3569.0, 618.0, 6.80},
    {-3573.0, 623.0, 6.10}
};

static float DeathswornWaypoint[][3] =
{
    {-3561.3, 537.2, 16.6},
    {-3553.4, 500.9, 20.0},
    {-3566.0, 484.6, 22.4},
    {-3567.3, 457.0, 25.1},
    {-3571.0, 417.7, 28.9},
    {-3562.5, 379.2, 32.2},
    {-3603.7, 346.0, 39.2},
    {-3631.7, 327.1, 38.8}
};

static float Vagath[4] =
{
    -3570.7, 453.4, 25.72, 1.56
};

static float ShadowlordPos[6][4] =
{
    {-3555.9, 522.2, 18.20, 1.64},
    {-3549.0, 519.4, 19.00, 1.54},
    {-3540.5, 517.0, 20.30, 1.73},
    {-3572.6, 486.4, 22.50, 1.34},
    {-3582.3, 489.8, 23.29, 1.30},
    {-3592.0, 487.5, 24.23, 1.25}
};

/*#####
# npc_maiev_BT_attu
######*/

struct npc_maiev_BT_attuAI : public npc_escortAI
{
    npc_maiev_BT_attuAI(Creature* c) : npc_escortAI(c) {}

    bool moving;
    Timer_UnCheked FanOfBladesTimer;

    void Reset()
    {
        moving = false;
        FanOfBladesTimer.Reset(urand(2000, 6000));
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(!m_creature->IsInCombat() && (who->GetEntry() == 22988 || who->GetEntry() == 23152) && m_creature->IsWithinDistInMap(who, 20))
            m_creature->AI()->AttackStart(who);
    }

    void WaypointReached(uint32 id)
    {
        if(id == 3)
            DoYell(-1200102, 0, 0);
        if(id == 10)
            DoCast(m_creature, SPELL_STEALTH);
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if(!moving)
        {
            if(!HasEscortState(STATE_ESCORT_ESCORTING))
                npc_escortAI::Start(true, true);
            moving = true;
        }

        if(UpdateVictim())
        {
            if(FanOfBladesTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_FAN_OF_BLADES);
                FanOfBladesTimer = urand(8000, 16000);
            }
        }
    }

};

CreatureAI* GetAI_npc_maiev_BT_attu(Creature *_Creature)
{
    npc_maiev_BT_attuAI* Maiev_BTattu_AI = new npc_maiev_BT_attuAI (_Creature);

    for(uint32 i = 0; i < 14; ++i)
    {
        Maiev_BTattu_AI->AddWaypoint(i+1, MaievWaypoint[i][0], MaievWaypoint[i][1], MaievWaypoint[i][2]);
    }

    return (CreatureAI*)Maiev_BTattu_AI;
}

/*#####
# npc_akama_BT_attu
######*/

struct npc_akama_BT_attuAI : public npc_escortAI
{
    npc_akama_BT_attuAI(Creature* c) : npc_escortAI(c) {}

    bool moving;
    bool yell;
    bool say;
    Timer_UnCheked YellTimer;
    Timer_UnCheked ChainLightningTimer;
    Timer_UnCheked KillSayTimer;

    void Reset()
    {
        ChainLightningTimer.Reset(1000);
        moving = false;
        say = false;
        yell = false;
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(!m_creature->IsInCombat() && (who->GetEntry() == 22988 || who->GetEntry() == 23152) && m_creature->IsWithinDistInMap(who, 20))
            m_creature->AI()->AttackStart(who);
    }

    void WaypointReached(uint32 id)
    {
        if(id == 2)
            DoYell(-1200103, 0, 0);
        if(id == 4)
        {
            yell = true;
            YellTimer = 10000;
        }
        if(id == 7)
        {
            say = true;
            KillSayTimer = 3000;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if(yell && YellTimer.Expired(diff))
        {
            DoYell(-1200104, 0, 0);
            yell = false;
        }

        if(say && KillSayTimer.Expired(diff))
        {
            DoSay(-1200105, 0, 0);
            say = false;
        }

        if(!moving)
        {
            if(!HasEscortState(STATE_ESCORT_ESCORTING))
                npc_escortAI::Start(true, true);
            moving = true;
        }

        if(UpdateVictim())
        {
            if(ChainLightningTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), CHAIN_LIGHTNING);
                ChainLightningTimer = urand(5000, 10000);
            }
        }
    }

};

CreatureAI* GetAI_npc_akama_BT_attu(Creature *_Creature)
{
    npc_akama_BT_attuAI* Akama_BTattu_AI = new npc_akama_BT_attuAI (_Creature);

    for(uint32 i = 0; i < 3; ++i)
    {
        Akama_BTattu_AI->AddWaypoint(i+1, AkamaWaypoint[i][0], AkamaWaypoint[i][1], AkamaWaypoint[i][2]);
    }
    Akama_BTattu_AI->AddWaypoint(4, AkamaWaypoint[3][0], AkamaWaypoint[3][1], AkamaWaypoint[3][2], 15000);
    for(uint32 i = 4; i < 6; ++i)
    {
        Akama_BTattu_AI->AddWaypoint(i+1, AkamaWaypoint[i][0], AkamaWaypoint[i][1], AkamaWaypoint[i][2]);
    }
    Akama_BTattu_AI->AddWaypoint(7, AkamaWaypoint[6][0], AkamaWaypoint[6][1], AkamaWaypoint[6][2], 5000);
    for(uint32 i = 7; i < 10; ++i)
    {
        Akama_BTattu_AI->AddWaypoint(i+1, AkamaWaypoint[i][0], AkamaWaypoint[i][1], AkamaWaypoint[i][2]);
    }
    return (CreatureAI*)Akama_BTattu_AI;
}

/*#####
# npc_ashtongue_deathsworn
######*/

struct npc_ashtongue_deathswornAI : public npc_escortAI
{
    npc_ashtongue_deathswornAI(Creature* c) : npc_escortAI(c) {}

    Timer_UnCheked AttackTimer;
    bool intro;

    void Reset()
    {
        if(!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            AttackTimer.Reset(25000);
            intro = false;
        }
    }

    void MoveInLineOfSight(Unit* who)
    {
        if(!m_creature->IsInCombat() && (who->GetEntry() == 22988 || who->GetEntry() == 23152 || who->GetEntry() == 21166) && m_creature->IsWithinDistInMap(who, 30))
            m_creature->AI()->AttackStart(who);
    }

    void WaypointReached(uint32) { }

    void UpdateAI(const uint32 diff)
    {
        if(!intro && AttackTimer.Expired(diff))
        {
            intro = true;
            if(!HasEscortState(STATE_ESCORT_ESCORTING))
                npc_escortAI::Start(true, true);
        }

        npc_escortAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_npc_ashtongue_deathsworn(Creature *_Creature)
{
    npc_ashtongue_deathswornAI* Deathsworn_AI = new npc_ashtongue_deathswornAI (_Creature);

    for(uint32 i = 0; i < 8; ++i)
    {
        Deathsworn_AI->AddWaypoint(i+2, DeathswornWaypoint[i][0], DeathswornWaypoint[i][1], DeathswornWaypoint[i][2]);
    }
    return (CreatureAI*)Deathsworn_AI;
}

/*#####
# mob_vagath
######*/

struct mob_vagathAI : public ScriptedAI
{
    mob_vagathAI(Creature* c) : ScriptedAI(c) {}

    void Reset()
    {
        DoYell(-1200107, 0, 0);
    }

    void EnterCombat(Unit* who)
    {
        DoYell(-1200106, 0, 0);
    }

    void JustDied(Unit* killer)
    {
        DoYell(-1200108, 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(UpdateVictim())
            DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_vagath(Creature* _Creature)
{
    return new mob_vagathAI(_Creature);
}

/*#####
# mob_illidari_shadowlord
######*/

struct mob_illidari_shadowlordAI : public ScriptedAI
{
    mob_illidari_shadowlordAI(Creature* c) : ScriptedAI(c) {}

    Timer_UnCheked CarrionSwarmTimer;
    Timer_UnCheked InfernoTimer;
    Timer_UnCheked SleepTimer;

    void Reset()
    {
        CarrionSwarmTimer.Reset(urand(4000, 10000));
        InfernoTimer.Reset(urand(6000, 15000));
        SleepTimer.Reset(urand(2000, 30000));
    }

    void UpdateAI(const uint32 diff)
    {
        if(UpdateVictim())
        {
            if(CarrionSwarmTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_CARRION_SWARM);
                CarrionSwarmTimer = urand(8000, 16000);
            }

            if(InfernoTimer.Expired(diff))
            {
                DoCast(m_creature->GetVictim(), SPELL_INFERNO);
                InfernoTimer = urand(35000, 50000);
            }

            if(SleepTimer.Expired(diff))
            {
                if(!urand(0, 3))
                    DoCast(m_creature->GetVictim(), SPELL_SLEEP);
                SleepTimer = 60000;
            }

            DoMeleeAttackIfReady();
        }
    }
};

CreatureAI* GetAI_mob_illidari_shadowlord(Creature* _Creature)
{
    return new mob_illidari_shadowlordAI(_Creature);
}

/*#####
# npc_xiri
######*/

struct npc_xiriAI : public Scripted_NoMovementAI
{
    npc_xiriAI(Creature* c) : Scripted_NoMovementAI(c) {}

    bool EventStarted;
    uint64 PlayerGUID;
    Timer_UnCheked QuestTimer;

    void Reset()
    {
        QuestTimer.Reset(140000);
        PlayerGUID = 0;
        EventStarted = false;
    }

    void StartEvent()
    {
        DoPlaySoundToSet(m_creature,BLACK_TEMPLE_PRELUDE_MUSIC);
        SummonEnemies();
        SummonAttackers();
        DoCast(m_creature, SPELL_LIGHT_OF_THE_NAARU_1);
        EventStarted = true;
    }

    void SummonEnemies()
    {
        //Summon Shadowlords
        for(uint32 i = 0; i < 6; i++)
            m_creature->SummonCreature(MOB_ILLIDARI_SHADOWLORD, ShadowlordPos[i][0], ShadowlordPos[i][1], ShadowlordPos[i][2], ShadowlordPos[i][3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
        //Summon Vagath
        m_creature->SummonCreature(MOB_VAGATH, Vagath[0], Vagath[1], Vagath[2], Vagath[3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
    }

    void SummonAttackers()
    {
        //Summon Akama
        m_creature->SummonCreature(NPC_AKAMA_BT, AkamaBT[0], AkamaBT[1], AkamaBT[2], AkamaBT[3], TEMPSUMMON_CORPSE_DESPAWN, 0);
        //Summon Maiev
        m_creature->SummonCreature(NPC_MAIEV_SHADOWSONG, MaievBT[0], MaievBT[1], MaievBT[2], MaievBT[3], TEMPSUMMON_CORPSE_DESPAWN, 0);
        //Summon Ashtongue Deathsworns
        for(uint32 i = 0; i < 8; ++i)
        {
            Creature* DeathswornAttacker = m_creature->SummonCreature(NPC_ASHTONGUE_DEATHSWORN, Deathsworn[i][0], Deathsworn[i][1], Deathsworn[i][2], Deathsworn[i][3], TEMPSUMMON_CORPSE_DESPAWN, 0);
            if(DeathswornAttacker)
            {
                if(DeathswornAttacker->IsWalking())
                    DeathswornAttacker->SetWalk(false);
                DeathswornAttacker->GetMotionMaster()->MovePoint(1, DeathswornPath[i][0], DeathswornPath[i][1], DeathswornPath[i][2]);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventStarted)
        {
            if(QuestTimer.Expired(diff))
            {
                Player* pl = m_creature->GetPlayerInWorld(PlayerGUID);
                if(pl && pl->GetQuestStatus(10985) == QUEST_STATUS_INCOMPLETE)
                    pl->GroupEventHappens(10985, m_creature);
                Reset();
            }
        }

    }
};

CreatureAI* GetAI_npc_xiri(Creature *_Creature)
{
    return new npc_xiriAI (_Creature);
}

/*#####
# Xi'ri gossips
######*/

bool GossipHello_npc_xiri(Player *player, Creature *_Creature)
{
    bool EventStarted = ((npc_xiriAI*)_Creature->AI())->EventStarted;

    if(EventStarted)
        return false;

    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(10985) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(XIRI_GOSSIP_HELLO), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    else if (player->GetQuestStatus(10985) == QUEST_STATUS_COMPLETE && !player->HasItemCount(32649,1,true) && !player->HasItemCount(32757,1,true))
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(XIRI_GOSSIP_RESTORE_ITEM), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_xiri(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->CLOSE_GOSSIP_MENU();
            ((npc_xiriAI*)_Creature->AI())->StartEvent();
            ((npc_xiriAI*)_Creature->AI())->PlayerGUID = player->GetGUID();
            break;
        case GOSSIP_ACTION_INFO_DEF +1:
            player->CLOSE_GOSSIP_MENU();
            // recheck quest status and select medallion version
            if (player->GetQuestStatus(10985) != QUEST_STATUS_COMPLETE)
                break;
            uint32 entry = (player->GetQuestStatus(10959) == QUEST_STATUS_COMPLETE) ? 32757 : 32649;
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, 1);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, entry, true);
                player->SendNewItem(item, 1, true, false, true);
            }
            break;
    }
    return true;
}

/*#######
# Quest: To Legion Hold
########*/

/*#####
# mob_deathbringer_joovan
######*/

#define IMAGE_OF_WARBRINGER     21502

#define DEATHBRINGER_SAY1   -1200109
#define WARBRINGER_SAY1     -1200110
#define DEATHBRINGER_SAY2   -1200111
#define WARBRINGER_SAY2     -1200112
#define DEATHBRINGER_SAY3   -1200113
#define WARBRINGER_SAY3     -1200114
#define DEATHBRINGER_SAY4   -1200115
#define WARBRINGER_SAY4     -1200116

float deathbringer_joovanWP[2][3] = {
    { -3320, 2948, 172 },
    { -3304, 2931, 171 }
};

float imageOfWarbringerSP[4] = { -3300, 2927, 173.4, 2.40 };

struct mob_deathbringer_joovanAI : public ScriptedAI
{
    bool EventStarted;
    bool ContinueMove;
    uint64 ImageOfWarbringerGUID;
    Timer_UnCheked EventTimer;
    uint8 EventCounter;

    mob_deathbringer_joovanAI(Creature* c) : ScriptedAI(c)
    {
        ImageOfWarbringerGUID = 0;
    }


    void Reset()
    {
        me->GetMotionMaster()->MovePoint(0, deathbringer_joovanWP[0][0], deathbringer_joovanWP[0][1], deathbringer_joovanWP[0][2]);
        EventStarted = false;

        if(ImageOfWarbringerGUID)
        {
            if(Unit* unit = me->GetUnit(*me, ImageOfWarbringerGUID))
            {
                unit->CombatStop();
                unit->AddObjectToRemoveList();
            }
            ImageOfWarbringerGUID = 0;
        }
        ContinueMove = false;
    }

    void JustSummoned(Creature *creature)
    {
        ImageOfWarbringerGUID = creature->GetGUID();
        EventStarted = true;
        EventTimer.Reset(1);
        EventCounter = 0;
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        switch(id)
        {
            case 0:
                ContinueMove = true;
                break;
            case 1:
                me->SummonCreature(IMAGE_OF_WARBRINGER, imageOfWarbringerSP[0], imageOfWarbringerSP[1], imageOfWarbringerSP[2], imageOfWarbringerSP[3], TEMPSUMMON_TIMED_DESPAWN, 120000);
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
            case 2:
            {
                if(Creature* warbringer = (Creature*)me->GetUnit(*me, ImageOfWarbringerGUID))
                {
                    warbringer->CombatStop();
                    warbringer->AddObjectToRemoveList();
                }
                me->CombatStop();
                me->AddObjectToRemoveList();
                break;
            }

        }
    }

    void WarbringerSay(int32 text)
    {
        if(Creature* unit = (Creature*)me->GetUnit(*me, ImageOfWarbringerGUID))
            unit->Say(text, 0, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(ContinueMove)
        {
            me->GetMotionMaster()->MovePoint(1, deathbringer_joovanWP[1][0], deathbringer_joovanWP[1][1], deathbringer_joovanWP[1][2]);
            ContinueMove = false;
        }

        if(EventStarted)
        {
            if(EventTimer.Expired(diff))
            {
                switch(EventCounter)
                {
                    case 0:
                        me->Say(-1200109, 0, 0);
                        break;
                    case 1:
                        WarbringerSay(-1200110);
                        break;
                    case 2:
                        me->Say(-1200111, 0, 0);
                        break;
                    case 3:
                        WarbringerSay(-1200112);
                        break;
                    case 4:
                        me->Say(-1200113, 0, 0);
                        break;
                    case 5:
                        WarbringerSay(-1200114);
                        break;
                    case 6:
                        me->Say(-1200115, 0, 0);
                        break;
                    case 7:
                    {
                        WarbringerSay(-1200116);

                        std::list<Unit*> pList;
                        Hellground::AnyUnitInObjectRangeCheck u_check(me, 20.0f);
                        Hellground::UnitListSearcher<Hellground::AnyUnitInObjectRangeCheck> searcher(pList, u_check);

                        Cell::VisitAllObjects(me, searcher, 20.0f);

                        Creature* warbringer = (Creature*)me->GetUnit(*me, ImageOfWarbringerGUID);
                        if(!warbringer)
                            break;

                        for(std::list<Unit*>::iterator it = pList.begin(); it != pList.end(); it++)
                        {
                            if((*it)->GetTypeId() == TYPEID_PLAYER)
                            {
                                Player *p = (Player*)(*it);
                                if(p->HasAura(37097, 0))
                                {
                                    // event happens nie dziala, a powinien!
                                    p->AreaExploredOrEventHappens(10596);
                                    p->AreaExploredOrEventHappens(10563);
                                    // dlatego recznie musimy complete quest dac
                                    p->CompleteQuest(10596);
                                    p->CompleteQuest(10563);
                                }
                            }
                        }
                        break;
                    }
                    case 8:
                    {
                        me->GetMotionMaster()->MovePoint(2, deathbringer_joovanWP[0][0], deathbringer_joovanWP[0][1], deathbringer_joovanWP[0][2]);
                        break;
                    }
                }
                EventCounter++;
                if(EventCounter == 8)
                    EventTimer = 1000;
                else
                    EventTimer = 4000;
            }
        }
/*
        if(!UpdateVictim())
            return;

        me->Say("Deathbringer victim found, stopping event", 0, 0);

        EventStarted = false;
       DoMeleeAttackIfReady();*/
    }
};

CreatureAI* GetAI_mob_deathbringer_joovanAI(Creature *_Creature)
{
    return new mob_deathbringer_joovanAI(_Creature);
}

/*####
# npc_overlord_orbarokh
####*/

#define GOSSIP_ITEM_ORBAROKH 16046

bool GossipHello_npc_overlord_orbarokh(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

        if(player->GetQuestStatus(10751) || player->GetQuestStatus(10765) || player->GetQuestStatus(10768) || player->GetQuestStatus(10769) == QUEST_STATUS_INCOMPLETE )
            if(!player->HasItemCount(31108,1))
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_ORBAROKH), GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );
                player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_overlord_orbarokh(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_SENDER_INFO )
    {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 31108, 1);
            if (msg == EQUIP_ERR_OK)
            {
                 Item* item = player->StoreNewItem(dest, 31108, true);
                     player->SendNewItem(item,1,true,false,true);
            }
    }
    return true;
}

/*####
# npc_thane_yoregar
####*/

#define GOSSIP_ITEM_YOREGAR 16047

bool GossipHello_npc_thane_yoregar(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

        if(player->GetQuestStatus(10773) || player->GetQuestStatus(10774) || player->GetQuestStatus(10775) || player->GetQuestStatus(10776) == QUEST_STATUS_INCOMPLETE )
            if(!player->HasItemCount(31310,1))
                player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_YOREGAR), GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );
                player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_thane_yoregar(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_SENDER_INFO )
    {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 31310, 1);
            if (msg == EQUIP_ERR_OK)
            {
                 Item* item = player->StoreNewItem(dest, 31310, true);
                     player->SendNewItem(item,1,true,false,true);
            }
    }
    return true;
}

/*####
# npc_restore_spectrecles
####*/

#define GOSSIP_RESTORE_SPECTRECLES 16048
#define ITEM_SPECTRECLES 30719

bool GossipHello_npc_restore_spectrecles(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    if ((player->GetQuestStatus(10643) || player->GetQuestStatus(10625)) && !player->HasItemCount(ITEM_SPECTRECLES, 1, true))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_RESTORE_SPECTRECLES), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    else if ((player->GetQuestStatus(10633) ||  player->GetQuestStatus(10639) || player->GetQuestStatus(10644) || player->GetQuestStatus(10645))  && !player->HasItemCount(30721, 1, true))
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_RESTORE_SPECTRECLES), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_restore_spectrecles(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1 && (player->GetQuestStatus(10643) || player->GetQuestStatus(10625)) && !player->HasItemCount(ITEM_SPECTRECLES, 1, true))
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_SPECTRECLES, 1);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, ITEM_SPECTRECLES, true);
            player->SendNewItem(item, 1, true, false, true);
        }
    }
 
    if (action == GOSSIP_ACTION_INFO_DEF + 2 && (player->GetQuestStatus(10633) || player->GetQuestStatus(10639) || player->GetQuestStatus(10644) || player->GetQuestStatus(10645)) && !player->HasItemCount(30721, 1, true))
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 30721, 1);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, 30721, true);
            player->SendNewItem(item, 1, true, false, true);
        }
    }
    return true;
}

/*####
# go_forged_illidari_bane
####*/

bool GOUse_go_forged_illidari_bane(Player *pPlayer, GameObject *pGo)
{
    ItemPosCountVec dest;
    uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 30876, 1);
    if (msg == EQUIP_ERR_OK)
    {
        if (Item* item = pPlayer->StoreNewItem(dest,30876,true))
            pPlayer->SendNewItem(item,1,false,true);
        else
            pPlayer->SendEquipError(msg,NULL,NULL);
    }

    pGo->SetLootState(GO_JUST_DEACTIVATED);
    return true;
}

struct npc_21949AI : public ScriptedAI
{
    npc_21949AI(Creature* creature) : ScriptedAI(creature) { }

    Timer CheckTimer;
    bool UnderControl;

    void Reset()
    {
        CheckTimer.Reset(0);
        UnderControl = false;
    }

    void EnterCombat(Unit*)
    {
        me->NeedChangeAI = true;
        me->IsAIEnabled = false;
    }

    void OnCharmed(bool apply)
    {
        if(apply)
        {
            UnderControl = true;
            CheckTimer.Reset(3000);
        }
    }
    
    void JustDied(Unit *killer)
    {
        me->Respawn();
    }

    void UpdateAI(const uint32 diff)
    {
        if(me->isCharmed() && me->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            // remove charm when not in Invasion Point: Cataclysm
            if(CheckTimer.Expired(diff))
            {
                Unit* charmer = me->GetCharmerOrOwnerPlayerOrPlayerItself();
                if(charmer->GetAreaId() != 3943)
                    me->RemoveCharmedOrPossessedBy(charmer);
                CheckTimer.Reset(2000);
            }
        }
        else
        {
            if(UnderControl)
                me->Kill(me);
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21949(Creature* creature)
{
    return new npc_21949AI(creature);
}
// 36393 - summon visual.

struct npc_21207AI : public ScriptedAI
{
    npc_21207AI(Creature* creature) : ScriptedAI(creature) { }

    Timer BeamTimer;
    Timer ShadowBoltTimer;
    Timer BurnTimer;

    void Reset()
    {
        BeamTimer.Reset(1000);
        ShadowBoltTimer.Reset(2000);
        BurnTimer.Reset(1000);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
            me->Kill(me);
    }

    void EnterCombat(Unit* who)
    {
        me->InterruptNonMeleeSpells(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(BeamTimer.Expired(diff))
            {
                me->CastSpell(me, 36382, false);
                if(Unit* target = FindCreature(21211, 25, me))
                    me->CastSpell(target, 39123, false);
                BeamTimer.Reset(5000);
            }
            return;
        }

        if(BurnTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 38401);
            BurnTimer = 4000;
        }

        if(ShadowBoltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 12471);
            ShadowBoltTimer = 2000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21207(Creature* creature)
{
    return new npc_21207AI(creature);
}

struct npc_21211AI : public ScriptedAI
{
    npc_21211AI(Creature* creature) : ScriptedAI(creature) { }

    Timer VisualBeamTimer;

    void Reset()
    {
        VisualBeamTimer.Reset(1000);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
            me->Kill(me);
    }

    void UpdateAI(const uint32 diff)
    {
        if(VisualBeamTimer.Expired(diff))
        {
            me->CastSpell(me, 36384, false); // green beam
            if(me->GetDBTableGUIDLow() == 84860)
                me->AddAura(36393, me);
            VisualBeamTimer.Reset(5000);
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21211(Creature* creature)
{
    return new npc_21211AI(creature);
}


struct npc_21210AI : public ScriptedAI
{
    npc_21210AI(Creature* creature) : ScriptedAI(creature) { }

    void Reset()
    {
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        if(eventType == 5)
            me->Kill(me);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21210(Creature* creature)
{
    return new npc_21210AI(creature);
}

struct npc_21195AI : public ScriptedAI
{
    npc_21195AI(Creature* creature) : ScriptedAI(creature) { }

    Timer EventTimer;
    Timer Event2Timer;
    uint64 TuberGUID;

    void Reset()
    {
        EventTimer.Reset(0);
        Event2Timer.Reset(0);
        TuberGUID = 0;
    }

    void EnterCombat(Unit* who)
    {
        me->CastSpell(who, 22120, false);
    }

    void JustReachedHome()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
    }

    void SpellHit(Unit* caster, const SpellEntry *spell)
    {
        if(spell->Id == 36652)
        {
            if (GameObject* tubermound = GetClosestGameObjectWithEntry(caster, 184701, 2.5))
            {
                TuberGUID = tubermound->GetGUID();
                me->GetMotionMaster()->MovePoint(100, tubermound->GetPositionX(), tubermound->GetPositionY(), tubermound->GetPositionZ());
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            }
        }
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type != POINT_MOTION_TYPE)
            return;

        if(Id != 100)
            return;

        me->SetRooted(true);
        me->HandleEmoteCommand(35);
        if(TuberGUID)
        {
            if (GameObject* tubermound = GameObject::GetGameObject((*me), TuberGUID))
            {
                EventTimer.Reset(2000);
                tubermound->SetLootState(GO_JUST_DEACTIVATED);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventTimer.Expired(diff))
        {
            me->HandleEmoteCommand(35);
            me->CastSpell(me, 36462, false);
            Event2Timer.Reset(2000);
            EventTimer.Reset(0);
        }

        if(Event2Timer.Expired(diff))
        {
            me->SetRooted(false);
            TuberGUID = 0;
            me->GetMotionMaster()->MoveTargetedHome();
            Event2Timer.Reset(0);
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21195(Creature* creature)
{
    return new npc_21195AI(creature);
}

struct npc_21292AI : public ScriptedAI
{
    npc_21292AI(Creature* creature) : ScriptedAI(creature) { }

    Timer EmotionTimer;

    void Reset()
    {
        EmotionTimer.Reset(3000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(EmotionTimer.Expired(diff))
        {
            me->HandleEmoteCommand(383);
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21292(Creature* creature)
{
    return new npc_21292AI(creature);
}

struct npc_21310AI : public ScriptedAI
{
    npc_21310AI(Creature* creature) : ScriptedAI(creature) { }

    Timer ChannelingTimer;

    void Reset()
    {
        ChannelingTimer.Reset(3000);
    }

    void UpdateAI(const uint32 diff)
    {
        if(ChannelingTimer.Expired(diff))
        {
            if(Unit* artor = FindCreature(21292, 10, me))
                me->CastSpell(artor, 36558, false);
            ChannelingTimer.Reset(0);
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21310(Creature* creature)
{
    return new npc_21310AI(creature);
}

struct npc_21410AI : public ScriptedAI
{
    npc_21410AI(Creature* creature) : ScriptedAI(creature) { }

    Timer MoveTimer;

    void Reset()
    {
        MoveTimer.Reset(1000);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void UpdateAI(const uint32 diff)
    {
        if(MoveTimer.Expired(diff))
        {
            me->GetMotionMaster()->MovePoint(100, -4058.758, 1515.231, 91.292);
            MoveTimer = 0;
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21410(Creature* creature)
{
    return new npc_21410AI(creature);
}
enum
{
    NPC_ICARIUS = 21409,
    NPC_ZARATH  = 21410,
    NPC_BORAK   = 21293,

    ZARATH_SAY_1    = -1901038,
    ZARATH_SAY_2    = -1901039,
    ZARATH_SAY_3    = -1901040,
    ICARIUS_SAY_1   = -1901041,
    ICARIUS_SAY_2   = -1901042,
    ICARIUS_SAY_3   = -1901043,
    ICARIUS_SAY_4   = -1901044,
    ICARIUS_SAY_5   = -1901045,
    ICARIUS_EMOTE   = -1901046,
    ICARIUS_SAY_6   = -1901047,
    BORAK_WHISPER_1 = -1901048,
    BORAK_WHISPER_2 = -1901049
};

static const DialogueEntry aIntroDialogue[] =
{
    {ICARIUS_SAY_1,     NPC_ICARIUS,        1000},
    {ZARATH_SAY_1,      NPC_ZARATH,         2000},
    {ICARIUS_SAY_2,     NPC_ICARIUS,        7000},
    {ZARATH_SAY_2,      NPC_ZARATH,         6000},
    {ICARIUS_SAY_3,     NPC_ICARIUS,        7000},
    {ZARATH_SAY_3,      NPC_ZARATH,         6000}, // despawn here
    {ICARIUS_SAY_4,     NPC_ICARIUS,        5000},
    {ICARIUS_SAY_5,     NPC_ICARIUS,        3000}, 
    {NPC_ICARIUS,                 0,           0},// play emote 275 after this, move to object, emote kneel, say next
    {ICARIUS_SAY_6,     NPC_ICARIUS,         500}, // attack
    {0,                 0,                    0},
};

struct npc_21409AI : public ScriptedAI, private DialogueHelper
{
    npc_21409AI(Creature* creature) : ScriptedAI(creature), DialogueHelper(aIntroDialogue) { }

    Timer MoveTimer;
    uint64 ZarathGUID;
    uint64 BorakGUID;
    uint64 Summoner;

    void Reset()
    {
        MoveTimer.Reset(1000);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit * summoner)
    {
        Summoner = summoner->GetGUID();
    }

    void JustDidDialogueStep(int32 iEntry)
    {
        switch(iEntry)
        {
            case ICARIUS_SAY_4:
                if(Unit* zarath = FindCreature(21410, 15.0, me))
                    ((Creature*)zarath)->DisappearAndDie();
                break;
            case ICARIUS_SAY_5:
                me->HandleEmoteCommand(275);
                if (GameObject* grass = FindGameObject(184798, 30, me))
                    me->GetMotionMaster()->MovePoint(200, grass->GetPositionX(), grass->GetPositionY(), grass->GetPositionZ());
                break;
            case ICARIUS_SAY_6:
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                me->SetReactState(REACT_AGGRESSIVE);
                if(Unit* Borak = FindCreature(21293, 300, me))
                    ((Creature*)Borak)->Whisper("The time to strike is at hand. Terminate Icarius.", Summoner);
                break;
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry)
    {
        switch (uiEntry)
        {
            case NPC_ICARIUS:           return me;
            case NPC_ZARATH:            return me->GetMap()->GetCreature(ZarathGUID);
            case NPC_BORAK:             return me->GetMap()->GetCreature(BorakGUID);
            default:
                return NULL;
        }

    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type != POINT_MOTION_TYPE)
            return;

        if(Id == 100)
        {
            if(Unit* zarath = FindCreature(21410, 15.0, me))
            {
                ZarathGUID = zarath->GetGUID();
                me->SetOrientation(me->GetOrientationTo(zarath));
                me->SetFacingTo(me->GetOrientationTo(zarath));
                StartNextDialogueText(ICARIUS_SAY_1);
            }
        }
        if(Id == 200)
        {
            me->SetHomePosition(me->GetPositionX()-1, me->GetPositionY()-1, me->GetPositionZ(), me->GetOrientation());
            me->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
            DoScriptText(ICARIUS_EMOTE, me, NULL);
            StartNextDialogueText(ICARIUS_SAY_6);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        DialogueUpdate(diff);
        if(MoveTimer.Expired(diff))
        {
            me->GetMotionMaster()->MovePoint(100, -4054.427, 1515.515, 91.511);
            if(Unit* Borak = FindCreature(21293, 300, me))
            {
                BorakGUID = Borak->GetGUID();
                ((Creature*)Borak)->Whisper(-1200117, Summoner);
            }
            MoveTimer = 0;
        }

        if(!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21409(Creature* creature)
{
    return new npc_21409AI(creature);
}

struct npc_21309AI : public ScriptedAI
{
    npc_21309AI(Creature* creature) : ScriptedAI(creature) { }

    Timer CoPTimer;
    Timer SubservienceTimer;
    Timer VisualTimer;

    void Reset()
    {
        CoPTimer.Reset(1000);
        SubservienceTimer.Reset(urand(3000, 5000));
        VisualTimer.Reset(1000);
    }

    void EnterCombat(Unit* whp)
    {
        me->InterruptNonMeleeSpells(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if(VisualTimer.Expired(diff))
            {
                if(Unit* trigger = FindCreature(21310, 10, me))
                    me->CastSpell(trigger, 36578, false);
                VisualTimer.Reset(0);
            }
            return;
        }

        if(CoPTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 38048, false);
            CoPTimer = urand(15000, 18000);
        }

        if(SubservienceTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), 38169, false);
            SubservienceTimer = urand(15000, 20000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21309(Creature* creature)
{
    return new npc_21309AI(creature);
}


struct npc_21316AI : public ScriptedAI
{
    npc_21316AI(Creature* creature) : ScriptedAI(creature) { }

    void Reset()
    {
        me->RemoveAllAuras();
        me->SetDisplayId(me->GetNativeDisplayId());
        me->CastSpell(me, 16245, false);
        me->SetFlying(false);
    }

    void EnterCombat(Unit* whp)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            return;
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21316(Creature* creature)
{
    return new npc_21316AI(creature);
}

struct npc_21302AI : public ScriptedAI
{
    npc_21302AI(Creature* creature) : ScriptedAI(creature) { }

    Timer VisualTimer;
    Timer ShadowBoltTimer;
    Timer DrainLifeTimer;
    bool InfernalController;
    Timer FindInfernal;
    Timer MoveCenterTimer;
    Timer CastChannelingTimer;
    Timer ResetChannelingTimer;
    Timer MoveInfernalTimer;
    Creature* infernal;

    void Reset()
    {
        VisualTimer.Reset(1000);
        ShadowBoltTimer.Reset(urand(1000, 4000));
        DrainLifeTimer.Reset(urand(8000, 12000));
        infernal = NULL;
        if (me->GetDBTableGUIDLow() == 74602)
        {
            InfernalController = true;
            FindInfernal.Reset(30000);
            me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
            me->GetMotionMaster()->Initialize();
        }
        if (me->GetDBTableGUIDLow() == 74601)
        {
            InfernalController = true;
            FindInfernal.Reset(60000);
            me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
            me->GetMotionMaster()->Initialize();
        }
        MoveCenterTimer.Reset(0);
        CastChannelingTimer.Reset(0);
        ResetChannelingTimer.Reset(0);
        MoveInfernalTimer.Reset(0);
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {

            if (id == 102)
            {
                // cant find spell he should cast in here, looks like something cosmetic
                // me->CastSpell()
                me->GetMotionMaster()->Clear();
                MoveCenterTimer = 5000;
            }
            else if (id == 103)
            {
                me->SetOrientation(4.04);
                me->SetFacingTo(4.04);
                if (infernal)
                {
                    infernal->GetMotionMaster()->Clear();
                    infernal->GetMotionMaster()->MovePoint(100, -3411.14, 2979.61, 169.89);
                }
                CastChannelingTimer = 5000;
            }
        }
    }
    void EnterCombat(Unit* whp)
    {
        me->InterruptNonMeleeSpells(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
        {
            if (!InfernalController)
            {
                if (VisualTimer.Expired(diff))
                {
                    if (Unit* trigger = FindCreature(21348, 15, me))
                        me->CastSpell(trigger, 33346, false);
                    VisualTimer.Reset(0);
                }
            }

            if (InfernalController)
            {
                if (FindInfernal.Expired(diff))
                {
                    FindInfernal = 0;
                    infernal = GetClosestCreatureWithEntry(me, 21316, 20, true);
                    if (infernal)
                    {
                        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
                        me->SetReactState(REACT_PASSIVE);
                        float x, y, z;
                        infernal->GetNearPoint(x, y, z, infernal->GetObjectSize());
                        me->GetMotionMaster()->Clear();
                        me->SetWalk(true);
                        me->GetMotionMaster()->MovePoint(102, x, y, z);
                    }
                    else EnterEvadeMode();
                }

                if (MoveCenterTimer.Expired(diff))
                {
                    MoveCenterTimer = 0;
                    if (infernal)
                    {
                        infernal->RemoveAllAuras();
                        infernal->GetMotionMaster()->Clear();
                        infernal->GetMotionMaster()->MoveFollow(me, 2, 2.7);
                        me->GetMotionMaster()->MovePoint(103, -3406.82, 2985.47, 169.88);
                    }
                    else EnterEvadeMode();
                }

                if (CastChannelingTimer.Expired(diff))
                {
                    ResetChannelingTimer = 5000;
                    CastChannelingTimer = 0;
                    if (infernal)
                    {
                        me->CastSpell(infernal, 35756, false);
                        if (Unit* Prophetess = GetClosestCreatureWithEntry(me, 20683, 15, true))
                            Prophetess->CastSpell(infernal, 35756, false);
                    }
                    else EnterEvadeMode();
                }

                if (ResetChannelingTimer.Expired(diff))
                {
                    ResetChannelingTimer = 0;
                    MoveInfernalTimer = 3000;
                    me->InterruptNonMeleeSpells(true);
                    if (Unit* Prophetess = GetClosestCreatureWithEntry(me, 20683, 15, true))
                        Prophetess->InterruptNonMeleeSpells(true);
                    if (infernal)
                    {
                        infernal->CastSpell(infernal, 36055, true);
                        infernal->SetDisplayId(11686);
                    }
                    else EnterEvadeMode();
                }

                if (MoveInfernalTimer.Expired(diff))
                {
                    MoveInfernalTimer = 0;
                    if (infernal)
                    {
                        infernal->SetFlying(true);
                        infernal->GetMotionMaster()->Clear();
                        infernal->GetMotionMaster()->MovePoint(102, -3418.71, 2963.609, 197.81);
                        infernal->ForcedDespawn(4000);
                    }
                    EnterEvadeMode();
                }

            }
            return;
        }

        if(DrainLifeTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 37992, false);
            DrainLifeTimer = urand(15000, 18000);
        }

        if(ShadowBoltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 9613, false);
            ShadowBoltTimer.Reset(3000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21302(Creature* creature)
{
    return new npc_21302AI(creature);
}

struct npc_21315AI : public ScriptedAI
{
    npc_21315AI(Creature* creature) : ScriptedAI(creature) { }

    Timer CleaveTimer;
    Timer DarkfuryTimer;
    Timer SpellbreakerTimer;

    void Reset()
    {
        CleaveTimer.Reset(1000);
        DarkfuryTimer.Reset(5000);
        SpellbreakerTimer.Reset(3000);
    }

    void EnterCombat(Unit* who)
    {
        me->Say(-1200118, LANG_UNIVERSAL, 0);
    }

    void JustDied(Unit* who)
    {
        me->SummonCreature(22106, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 30000);
    }

    void MovementInform(uint32 Type, uint32 Id)
    {
        if (Type != WAYPOINT_MOTION_TYPE)
            return;

        if(Id == 2)
        {
            me->SetFlying(false);
            me->SetWalk(true);
        }
        if(Id >= 16)
        {
            me->SetWalk(false);
            me->SetFlying(true);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(CleaveTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 15284, false);
            CleaveTimer.Reset(urand(4000, 6000));
        }

        if(DarkfuryTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 39153, false);
            DarkfuryTimer.Reset(urand(15000, 20000));
        }

        if(SpellbreakerTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 36073, false);
            SpellbreakerTimer.Reset(urand(15000, 20000));
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21315(Creature* creature)
{
    return new npc_21315AI(creature);
}

struct npc_22138AI : public ScriptedAI
{
    npc_22138AI(Creature* c) : ScriptedAI(c)   {}

    Timer ChannelingTimer;

    void Reset()
    {
        ClearCastQueue();
        ChannelingTimer.Reset(1000);
    }

    void EnterCombat(Unit* who)
    {
        me->InterruptNonMeleeSpells(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if(ChannelingTimer.Expired(diff))
        {
            if(Unit* OldGod = FindCreature(22137, 100, me))
                me->CastSpell(OldGod, 38469, false);
            ChannelingTimer.Reset(60000);
        }

        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_22138(Creature *_creature)
{
    return new npc_22138AI(_creature);
}

struct npc_22137AI : public ScriptedAI
{
    npc_22137AI(Creature* c) : ScriptedAI(c)   {}

    void Reset()
    {
        ClearCastQueue();
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->CastSpell(me, 38457, true);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if(eventType == 5)
        {
            ((Player*)invoker)->KilledMonster(22137, 0);
            me->ForcedDespawn(3000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_22137(Creature *_creature)
{
    return new npc_22137AI(_creature);
}

struct npc_20427AI : public ScriptedAI
{
    npc_20427AI(Creature* c) : ScriptedAI(c)   {}

    void Reset()
    {
        ClearCastQueue();
    }

    void JustDied(Unit* killer)
    {
        if(Unit* trigger = FindCreature(21332, 15, me))
        {
            ((Creature*)trigger)->Say(-1200119, LANG_UNIVERSAL, 0);
            trigger->Kill(trigger);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_20427(Creature *_creature)
{
    return new npc_20427AI(_creature);
}

struct npc_21332AI : public ScriptedAI
{
    npc_21332AI(Creature* c) : ScriptedAI(c)
    {
        Summoned = false;
    }

    Timer CheckTimer;

    bool Summoned;

    void Reset()
    {
        ClearCastQueue();
        me->GetMotionMaster()->MoveFollow(me->GetOwner(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        me->CastSpell(me, 36613, false);
        CheckTimer.Reset(1000);
        me->SetReactState(REACT_DEFENSIVE);
        me->SetAggroRange(0);
        me->CombatStopWithPets();
        me->ClearInCombat();
        me->AttackStop();
    }

    void JustDied(Unit* killer)
    {
        me->Say(-1200120, LANG_UNIVERSAL, 0);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!Summoned && CheckTimer.Expired(diff))
        {
            if(Unit* trigger = FindCreature(21334, 15, me))
            {
                Summoned = true;
                me->SummonCreature(20427, trigger->GetPositionX(), trigger->GetPositionY(), trigger->GetPositionZ(), trigger->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
            }
            CheckTimer = 3000;
        }

        Unit *pOwner = me->GetOwner();
        Unit *victim = me->GetVictim();
        Unit *attacker = pOwner->GetAttackerForHelper();

        if (pOwner)
        {
            if (!pOwner->isAlive())
            {
                me->ForcedDespawn();
                return;
            }
            if (!me->IsWithinDistInMap(pOwner, 30.0f) || (!victim || !attacker))
            {
                if (!me->GetVictim()|| !me->IsWithinDistInMap(pOwner, 30.0f))
                {
                    if (!me->HasUnitState(UNIT_STAT_FOLLOW))
                    {
                        victim = NULL;
                        attacker = NULL;
                        me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, urand(M_PI, M_PI/2));
                        Reset();
                        return;
                    }
                }
            }
            if (me->GetVictim() && me->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself() &&
                (pOwner->isInSanctuary() || me->isInSanctuary() || me->GetVictim()->isInSanctuary()))
            {
                victim = NULL;
                attacker = NULL;
                me->GetMotionMaster()->MoveFollow(pOwner, 2.0f, M_PI);
                Reset();
                return;
            }

            if (victim || attacker)
            {
                if (attacker)
                {
                    me->SetInCombatWith(attacker);
                    AttackStart(attacker);
                }
                else
                {
                    me->SetInCombatWith(victim);
                    AttackStart(victim);
                }
                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                DoMeleeAttackIfReady();
            }
       }
    }
};

CreatureAI* GetAI_npc_21332(Creature *_creature)
{
    return new npc_21332AI(_creature);
}

struct npc_21797AI : public ScriptedAI
{
    npc_21797AI(Creature *c) : ScriptedAI(c) {}

    uint64 PlayerGUID;
    Timer EventTimer;
    uint32 Step;

    void Reset()
    {
        PlayerGUID = 0;
        EventTimer.Reset(0);
        Step = 0;
        me->SetFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
    }

    void StartEvent(Player* pPlayer)
    {
        PlayerGUID = pPlayer->GetGUID();
        me->RemoveFlag(UNIT_NPC_FLAGS, (UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER));
        EventTimer = 2000;
        Step = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventTimer.Expired(diff))
        {
            switch(Step)
            {
                case 0:
                    me->Say(-1200121, LANG_UNIVERSAL, 0);
                    Step++;
                    EventTimer = 2000;
                    break;
                case 1:
                    if(PlayerGUID)
                    {
                        if(Unit* plr = Unit::GetUnit((*me), PlayerGUID))
                            plr->CastSpell(plr, 37747, false);
                    }
                    me->SummonCreature(21877, -4535.794, 1029.284, 8.836361, 3.787364, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000); // Krasius
                    me->DisappearAndDie();
                    Step = 0;
                    EventTimer = 0;
                    break;
            }
        }
    }
};

CreatureAI* GetAI_npc_21797(Creature *pCreature)
{
    return new npc_21797AI(pCreature);
}

bool QuestAccept_npc_21797(Player *pPlayer, Creature *pCreature, const Quest *pQuest)
{
    if(pQuest->GetQuestId() == 10639 || pQuest->GetQuestId() == 10645)
    {
        ((npc_21797AI*)pCreature->AI())->StartEvent(pPlayer);
        return true;
    }
    return false;
}

struct npc_21867AI : public ScriptedAI
{
    npc_21867AI(Creature *c) : ScriptedAI(c) {}

    Timer EventTimer;
    uint32 Step;

    void Reset()
    {
        EventTimer.Reset(0);
        Step = 0;
    }

    void JustDied(Unit* killer)
    {
        Unit* charmer = me->GetCharmerOrOwnerPlayerOrPlayerItself();
        if(charmer)
        {
            charmer->RemoveAurasDueToSpell(37747);
            charmer->RemoveAurasDueToSpell(37748);
        }
    }

    /*void OnCharmed(bool apply)
    {
        if(!apply)
        {
            me->Kill(me);
            Unit* charmer = me->GetCharmerOrOwnerPlayerOrPlayerItself();
            if(charmer)
            {
                charmer->RemoveAurasDueToSpell(37747);
                charmer->RemoveAurasDueToSpell(37748);
            }
        }
    }*/

    void EventPulse(Unit* pUnit, uint32 number)
    {
        EventTimer = 2000;
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventTimer.Expired(diff))
        {
            switch(Step)
            {
                case 0:
                    me->Mount(10720);
                    me->SetVisibility(VISIBILITY_ON);
                    me->Say(-1200122, LANG_UNIVERSAL, 0);
                    Step++;
                    EventTimer = 3000;
                    break;
                case 1:
                    me->Yell(-1200123, LANG_UNIVERSAL, 0);
                    Step++;
                    EventTimer = 5000;
                    break;
                case 2:
                    me->GetMotionMaster()->MovePoint(1, -4520.013, 995.0764, 11.57806);
                    me->ForcedDespawn(3000);
                    EventTimer = 0;
                    break;
            }
        }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21867(Creature *pCreature)
{
    return new npc_21867AI(pCreature);
}

struct npc_21877AI : public ScriptedAI
{
    npc_21877AI(Creature *c) : ScriptedAI(c), summons(c) {}

    uint64 PlayerGUID;
    Timer EventTimer;
    Timer ResetTimer;
    uint32 Step;
    bool EventStarted;
    SummonList summons;

    void Reset()
    {
        PlayerGUID = 0;
        EventTimer.Reset(5000);
        Step = 0;
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
        me->Say(-1200124, LANG_UNIVERSAL, 0);
        me->SummonCreature(21876, -4504.674, 1020.563, 33.07281, 2.932153, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4509.667, 1047.063, 26.45817, 3.787364, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4523.753, 1062.352, 24.30409, 4.433136, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4551.174, 1044.113, 16.52099, 5.218534, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4537.54, 1049.356, 18.74087, 4.415683, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4525, 1045.415, 19.89447, 4.153883, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4515.165, 1033.106, 20.71271, 3.176499, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4515.913, 1020.078, 23.67377, 2.722714, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
        me->SummonCreature(21876, -4524.726, 1009.763, 21.32487, 2.024582, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
    }

    void EnterEvadeMode()
    {
        summons.DespawnAll();
        me->DisappearAndDie();
    }

    void EnterCombat(Unit* who)
    {
        summons.Cast(0, 37784, me->GetVictim());
    }

    void JustSummoned(Creature* pSummon)
    {
        summons.Summon(pSummon);
        pSummon->setFaction(14);
    }

    void JustDied(Unit* killer)
    {
        summons.DespawnAll();
        me->Yell(-1200125, LANG_UNIVERSAL, 0);

        Unit* charmer = killer->GetCharmerOrOwnerPlayerOrPlayerItself();
        if(charmer)
        {
            charmer->RemoveAurasDueToSpell(37747);
            charmer->RemoveAurasDueToSpell(37748);
        }
        killer->RemoveAurasDueToSpell(37747);
        killer->RemoveAurasDueToSpell(37748);
        
        if(Creature* summon = me->SummonCreature(21867, killer->GetPositionX(), killer->GetPositionY(), killer->GetPositionZ(), 0.71, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000))
            summon->AI()->EventPulse(me, 1);
    }

    void KilledUnit(Unit* victim)
    {
        if(victim->GetTypeId() == TYPEID_UNIT && victim->GetEntry() == 21867)
        {
            Unit* charmer = victim->GetCharmerOrOwnerPlayerOrPlayerItself();
            if(charmer)
            {
                charmer->RemoveAurasDueToSpell(37747);
                charmer->RemoveAurasDueToSpell(37748);
            }
            EnterEvadeMode();
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventTimer.Expired(diff))
        {
            switch(Step)
            {
                case 0:
                    me->HandleEmoteCommand(333);
                    Step++;
                    EventTimer = 5000;
                    break;
                case 1:
                    me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                    Step = 0;
                    EventTimer = 0;
                    break;
            }
        }

        if(!UpdateVictim())
            return;

        if(me->GetVictim()->GetEntry() != 21867)
        {
            EnterEvadeMode();
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21877(Creature *pCreature)
{
    return new npc_21877AI(pCreature);
}

struct npc_21876AI : public Scripted_NoMovementAI
{
    npc_21876AI(Creature *c) : Scripted_NoMovementAI(c) {}

    Timer EnforcedSubmissionTimer;

    void Reset()
    {
        EnforcedSubmissionTimer.Reset(urand(1000, 3000));
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(EnforcedSubmissionTimer.Expired(diff))
        {
            Creature* teronplayer = GetClosestCreatureWithEntry(me, 21867, 100);
            me->CastSpell(teronplayer ? teronplayer : me->GetVictim(), 37784, false);
            EnforcedSubmissionTimer = urand(3000, 8000);
        }
    }
};

CreatureAI* GetAI_npc_21876(Creature *pCreature)
{
    return new npc_21876AI(pCreature);
}

/*######
## QUEST_CIPHER_OF_DAMNATION
######*/

//NPC
#define NPC_CYRUKH                   21181
#define NPC_GULDAN                   17008
#define NPC_SPIRIT                   21049
#define NPC_ORONOK                   21685
#define NPC_BORAK                    21686
#define NPC_GROMTOR                  21687
#define NPC_TORLOK                   21024
#define NPC_REDEEMED_SPIRIT_OF_AIR   21738
#define NPC_REDEEMED_SPIRIT_OF_FIRE  21740
#define NPC_REDEEMED_SPIRIT_OF_WATER 21741
#define NPC_REDEEMED_SPIRIT_OF_EARTH 21739

//Text
#define GULDAN_1 -1910194
#define GULDAN_2 -1910195
#define GULDAN_3 -1910196
#define GULDAN_4 -1910197
#define GULDAN_5 -1910199
#define GULDAN_6 -1910200

#define ORONOK_1 -1910202
#define ORONOK_2 -1200007
#define ORONOK_3 -1200008
#define ORONOK_4 -1200009
#define ORONOK_5 -1200010
#define ORONOK_6 -1200011
#define ORONOK_7 -1200012

#define SPIRIT_EARTH_1 -1200013
#define SPIRIT_EARTH_2 -1200014

#define SPIRIT_FIRE_1  -1200015

#define TORLOK_1       -1200016

#define GROMTOR_1      -1200017

#define SPIRIT_STOP    -1910198

#define CYRUKH_1       -1910201
#define CYRUKH_2       -1200018

//Emote
#define EMOTE_VORTEX   -1200126

//Spells
#define SPELL_GULDAN_CHANNEL 35996
#define SPELL_CYRUKH_DEFEAT  37235

#define QUEST_CIPHER_OF_DAMNATION 10588

struct npc_guldanAI : public ScriptedAI
{
    npc_guldanAI(Creature* pCreature) : ScriptedAI(pCreature) {}

    uint32 uiStepsTimer;
    uint32 uiSteps;

    uint64 playerGUID;

    bool GuldanStarted;
    uint32 ResetTimer;

    void Reset()
    {
        Creature* pCyrukh = GetClosestCreatureWithEntry(me, NPC_CYRUKH, 150, true);
        Creature* pOronok = GetClosestCreatureWithEntry(me, NPC_ORONOK, 150, true);
        if (!pCyrukh && !pOronok)
        {
            me->SetStandState(UNIT_STAND_STATE_KNEEL);

            GuldanStarted = false;
            playerGUID = 0;
            uiStepsTimer = 0;
            uiSteps = 0;
            ResetTimer = 0;

            if (Creature* pOronok = GetClosestCreatureWithEntry(me, NPC_ORONOK, 150))
                pOronok->DisappearAndDie();
            if (Creature* pBorak = GetClosestCreatureWithEntry(me, NPC_BORAK, 150))
                pBorak->DisappearAndDie();
            if (Creature* pGromtor = GetClosestCreatureWithEntry(me, NPC_GROMTOR, 150))
                pGromtor->DisappearAndDie();
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if(eventType == 5)
        {
            me->SummonCreature(NPC_CYRUKH, -3587.79, 1805.4, 39.66, 1.65, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 300000);
            playerGUID = invoker->GetObjectGuid();
        }
    }

    uint32 NextStep(uint32 uiSteps)
    {
        Creature* pCyrukh = GetClosestCreatureWithEntry(me, NPC_CYRUKH, 120);
        Creature* pSpirit = GetClosestCreatureWithEntry(me, NPC_SPIRIT, 30);
        Creature* pOronok = GetClosestCreatureWithEntry(me, NPC_ORONOK, 90);
        Creature* pBorak = GetClosestCreatureWithEntry(me, NPC_BORAK, 90);
        Creature* pGromtor = GetClosestCreatureWithEntry(me, NPC_GROMTOR, 90);

        if (!pCyrukh && !pSpirit)
        {
            Reset();
            return 0;
        }

        switch (uiSteps)
        {
            case 1:
                if(pCyrukh)
                {
                    pCyrukh->SetVisibility(VISIBILITY_OFF);
                    pCyrukh->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, true);
                    pCyrukh->SetReactState(REACT_PASSIVE);
                }
                me->SetStandState(UNIT_STAND_STATE_STAND);
                return 1000;
            case 2:
                DoScriptText(GULDAN_1, me);
                me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                return 2000;
            case 3:
                me->MonsterTextEmote(-1200126, 0, false);
                return 3000;
            case 4:
                DoCast(me, SPELL_CYRUKH_DEFEAT);
                me->CastSpell(me, SPELL_GULDAN_CHANNEL, true);
                DoScriptText(GULDAN_2, me);
                return 5500;
            case 5:
                DoScriptText(GULDAN_3, me);
                return 2000;
            case 6:
                pSpirit->HandleEmoteCommand(EMOTE_ONESHOT_BEG);
                DoScriptText(SPIRIT_STOP, pSpirit);
                return 4800;
            case 7:
                DoScriptText(GULDAN_4, me);
                pCyrukh->SetVisibility(VISIBILITY_ON);
                return 500;
            case 8:
                DoScriptText(CYRUKH_1, pCyrukh);
                return 6000;
            case 9:
                DoScriptText(GULDAN_5, me);
                return 1000;
            case 10:
                if (pOronok = me->SummonCreature(NPC_ORONOK, -3605.26f, 1924.91f, 49.53f, 4.88f, TEMPSUMMON_MANUAL_DESPAWN, 0))
                    pOronok->MonsterYellToZone(ORONOK_1, LANG_UNIVERSAL, playerGUID);
                me->SummonCreature(NPC_BORAK, -3601.04f, 1928.46f, 50.054f, 4.85f, TEMPSUMMON_MANUAL_DESPAWN, 0);
                me->SummonCreature(NPC_GROMTOR, -3609.73f, 1926.59f, 50.007f, 4.88f, TEMPSUMMON_MANUAL_DESPAWN, 0);
                return 5000;
            case 11:
                if (me->HasAura(SPELL_GULDAN_CHANNEL))
                    me->RemoveAurasDueToSpell(SPELL_GULDAN_CHANNEL);
                return 1000;
            case 12:
                DoCast(me, SPELL_CYRUKH_DEFEAT);
                DoScriptText(GULDAN_6, me);
                me->SetStandState(UNIT_STAND_STATE_KNEEL);
                if(pOronok)
                {
                    pOronok->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    pOronok->GetMotionMaster()->MovePoint(1, -3595.656, 1865.12, 47.24); // point 1
                    pOronok->SetHomePosition(-3595.656, 1865.12, 47.24, 0);
                }
                if(pGromtor)
                {
                    pGromtor->GetMotionMaster()->MovePoint(1, -3599.113, 1869.13, 47.24); // point 1
                    pGromtor->SetHomePosition(-3599.113, 1869.13, 47.24, 0);
                }
                if(pBorak)
                {
                    pBorak->GetMotionMaster()->MovePoint(1, -3593.113, 1869.13, 47.24); // point 1
                    pBorak->SetHomePosition(-3593.113, 1869.13, 47.24, 0);
                }
                return 5000;
            default:
                return 0;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!GuldanStarted)
        {
            if (Creature* pCyrukh = GetClosestCreatureWithEntry(me, NPC_CYRUKH, 80.0f, true))
                GuldanStarted = true;
        }

        if (uiStepsTimer <= uiDiff)
        {
            if (GuldanStarted)
                uiStepsTimer = NextStep(++uiSteps);
        }
        else uiStepsTimer -= uiDiff;

        if(ResetTimer)
        {
            if(ResetTimer <= uiDiff)
            {
                ResetTimer = 0;
                EnterEvadeMode();
                Reset();
            } else ResetTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_guldan(Creature* pCreature)
{
    return new npc_guldanAI(pCreature);
}

struct npc_oronokAI : public ScriptedAI
{
    npc_oronokAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        PlayerGUID = 0;
    }

    bool StopEvent;
    bool EventResume;
    bool CyrukhYell;
    bool CyrukhDied;
    bool isMoving;

    uint32 uiStepsTimer;
    uint32 uiSteps;
    uint64 PlayerGUID;

    Timer EarthbindTotemTimer;
    Timer ChainLightTimer;
    Timer FrostShockTimer;
    Timer HealingWaveTimer;

    void Reset()
    {
        me->SetReactState(REACT_AGGRESSIVE);

        uiStepsTimer = 0;
        uiSteps = 0;

        StopEvent = false;
        EventResume = false;
        CyrukhYell = false;
        isMoving = false;

        EarthbindTotemTimer.Reset(1000);
        ChainLightTimer.Reset(1200);
        FrostShockTimer.Reset(3000);
        HealingWaveTimer.Reset(5000);
    }

    void JustDied(Unit* pKiller)
    {
        Creature* pGuldan = GetClosestCreatureWithEntry(me, NPC_GULDAN, 150);
        if (pGuldan)
            CAST_AI(npc_guldanAI, pGuldan->AI())->ResetTimer = 1000;
    }
    void MovementInform(uint32 type, uint32 id)
    {
        if(type != POINT_MOTION_TYPE)
            return;

        if(id == 4)
        {
            isMoving = false;
        }
        else if (id == 5)
        {
            me->GetMotionMaster()->MovePoint(6, -3599.98, 1897.96, 47.24);
            me->SetHomePosition(-3599.98, 1897.96, 47.24, 0);

            Creature* pBorak = GetClosestCreatureWithEntry(me, NPC_BORAK, 80);
            Creature* pGromtor = GetClosestCreatureWithEntry(me, NPC_GROMTOR, 80);

            if(pGromtor)
            {
                pGromtor->GetMotionMaster()->MovePoint(6, -3604.79, 1893.76, 47.24);
                pGromtor->SetHomePosition(-3604.79, 1893.76, 47.24, 0);
            }
            if(pBorak)
            {
                pBorak->GetMotionMaster()->MovePoint(6, -3594.05, 1896.014, 47.24);
                pBorak->SetHomePosition(-3594.05, 1896.014, 47.24, 0);
            }
            isMoving = true;
        }
        else if(id == 6)
        {
            isMoving = false;
        }
    }

    uint32 NextStep(uint32 uiSteps)
    {
        Creature* pGuldan = GetClosestCreatureWithEntry(me, NPC_GULDAN, 120);
        Creature* pBorak = GetClosestCreatureWithEntry(me, NPC_BORAK, 80);
        Creature* pGromtor = GetClosestCreatureWithEntry(me, NPC_GROMTOR, 80);
        Creature* pTorlok = GetClosestCreatureWithEntry(me, NPC_TORLOK, 15);
        Creature* pSpiritEarth = GetClosestCreatureWithEntry(me, NPC_REDEEMED_SPIRIT_OF_EARTH, 15);
        Creature* pSpiritFire = GetClosestCreatureWithEntry(me, NPC_REDEEMED_SPIRIT_OF_FIRE, 15);

        switch (uiSteps)
        {
            case 1:
                me->GetMotionMaster()->MovePoint(4, -3656.90, 1837.502, 59.598);
                me->SetHomePosition(-3656.90, 1837.502, 59.598, 0);
                if(pGromtor)
                {
                    pGromtor->GetMotionMaster()->MovePoint(4, -3653.63, 1837.168, 58.003);
                    pGromtor->SetHomePosition(-3653.63, 1837.168, 58.003, 0);
                }
                if(pBorak)
                {
                    pBorak->GetMotionMaster()->MovePoint(4, -3659.502, 1840.149, 61.8);
                    pBorak->SetHomePosition(-3659.502, 1840.149, 61.8, 0);
                }
                isMoving = true;
                return 1000;
            case 2:
                if(pGromtor)
                {
                    pGromtor->MonsterSay(-1200017, LANG_UNIVERSAL, 0);
                    pGromtor->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                }
                return 5000;
            case 3:
                me->SetWalk(false);
                me->SetSpeed(MOVE_RUN, 1.2);
                me->GetMotionMaster()->MovePoint(5, -3638.72, 1861.18, 54.0);
                me->SetHomePosition(-3638.72, 1861.18, 54.0, 0);
                if(pGromtor)
                {
                    pGromtor->SetWalk(false);
                    pGromtor->SetSpeed(MOVE_RUN, 1.2);
                    pGromtor->GetMotionMaster()->MovePoint(5, -3642.19, 1863.061, 55.56);
                    pGromtor->SetHomePosition(-3642.19, 1863.061, 55.56, 0);
                }
                if(pBorak)
                {
                    pBorak->SetWalk(false);
                    pBorak->SetSpeed(MOVE_RUN, 1.2);
                    pBorak->GetMotionMaster()->MovePoint(5, -3638.32, 1856.04, 54.71);
                    pBorak->SetHomePosition(-3638.32, 1856.04, 54.71, 0);
                }
                isMoving = true;
                return 1000;
            case 4:
                me->SetOrientation(1.78);
                me->SetFacingTo(1.78);
                if(pGromtor)
                {
                    pGromtor->SetOrientation(1.704);
                    pGromtor->SetFacingTo(1.704);
                }
                if(pBorak)
                {
                    pBorak->SetOrientation(1.657);
                    pBorak->SetFacingTo(1.657);
                }
                me->MonsterSay(-1200009, LANG_UNIVERSAL, 0);
                return 5000;
            case 5:
                if(pTorlok)
                {
                    pTorlok->MonsterSay(-1200016, LANG_UNIVERSAL, 0);
                    pTorlok->HandleEmoteCommand(ANIM_EMOTE_TALK);
                }
                return 4000;
            case 6:
                if(pTorlok)
                    pTorlok->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                me->SummonCreature(NPC_REDEEMED_SPIRIT_OF_AIR, -3588.31f, 1892.76f, 47.240f, 2.56f, TEMPSUMMON_TIMED_DESPAWN, 34000);
                me->SummonCreature(NPC_REDEEMED_SPIRIT_OF_EARTH, -3599.34f, 1887.27f, 47.240f, 1.82f, TEMPSUMMON_TIMED_DESPAWN, 34000);
                me->SummonCreature(NPC_REDEEMED_SPIRIT_OF_FIRE, -3606.63f, 1887.06f, 47.240f, 1.28f, TEMPSUMMON_TIMED_DESPAWN, 34000);
                me->SummonCreature(NPC_REDEEMED_SPIRIT_OF_WATER, -3592.95f, 1887.88f, 47.240f, 2.044f, TEMPSUMMON_TIMED_DESPAWN, 34000);
                return 4000;
            case 7:
                me->SetOrientation(4.89);
                me->SetFacingTo(4.89);
                if(pGromtor)
                {
                    pGromtor->SetOrientation(4.89);
                    pGromtor->SetFacingTo(4.89);
                }
                if(pBorak)
                {
                    pBorak->SetOrientation(4.89);
                    pBorak->SetFacingTo(4.89);
                }
                me->MonsterSay(-1200010, LANG_UNIVERSAL, 0);
                me->HandleEmoteCommand(EMOTE_ONESHOT_SHOUT);
                return 6000;
            case 8:
                if(pSpiritEarth)
                    pSpiritEarth->MonsterSay(-1200013, LANG_UNIVERSAL, 0);
                return 8000;
            case 9:
                if(pSpiritFire)
                    pSpiritFire->MonsterSay(-1200015, LANG_UNIVERSAL, 0);
                return 10000;
            case 10:
                if(pSpiritEarth)
                    pSpiritEarth->MonsterSay(-1200014, LANG_UNIVERSAL, 0);
                return 5000;
            case 11:
                me->SetOrientation(1.78);
                me->SetFacingTo(1.78);
                if(pGromtor)
                {
                    pGromtor->SetOrientation(1.704);
                    pGromtor->SetFacingTo(1.704);
                }
                if(pBorak)
                {
                    pBorak->SetOrientation(1.657);
                    pBorak->SetFacingTo(1.657);
                }
                me->MonsterSay(-1200011, LANG_UNIVERSAL, 0);
                return 5000;
            case 12:
                me->MonsterSay(-1200012, LANG_UNIVERSAL, 0);
                return 11000;
            case 13:
                if (pGuldan)
                    CAST_AI(npc_guldanAI, pGuldan->AI())->ResetTimer = 3000;
                me->DisappearAndDie();
                if(pBorak)
                    pBorak->DisappearAndDie();
                if(pGromtor)
                    pGromtor->DisappearAndDie();
            default:
                return 0;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
        {
            if (uiStepsTimer <= uiDiff)
            {
                if (CyrukhDied && !isMoving)
                    uiStepsTimer = NextStep(++uiSteps);
            }
            else uiStepsTimer -= uiDiff;

            if (!StopEvent)
            {
                if (GetClosestCreatureWithEntry(me, NPC_GULDAN, 10.0f, true))
                {
                    me->MonsterSay(-1200007, LANG_UNIVERSAL, 0);
                    StopEvent = true;
                }
            }

            if (EventResume)
            {
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->GetMotionMaster()->MovePath(2665340997, false); // point 2
                me->SetHomePosition(-3655.303, 1836.364, 58.598, 0);
                if (Creature* gromtor = GetClosestCreatureWithEntry(me, NPC_GROMTOR, 20.0f, true))
                {
                    gromtor->GetMotionMaster()->MovePath(2665340997, false); // point 2
                    gromtor->SetHomePosition(-3655.303, 1836.364, 58.598, 0);
                }

                if (Creature* borak = GetClosestCreatureWithEntry(me, NPC_BORAK, 20.0f, true))
                {
                    borak->GetMotionMaster()->MovePath(2665340997, false); // point 2
                    borak->SetHomePosition(-3655.303, 1836.364, 58.598, 0);
                }

                EventResume = false;
            }

            if (GetClosestCreatureWithEntry(me, 21348, 5.0f, true))
            {
                if (!CyrukhYell)
                {
                    if (Creature* cyrukh = GetClosestCreatureWithEntry(me, NPC_CYRUKH, 100.0f, true))
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Creature* gromtor = GetClosestCreatureWithEntry(me, NPC_GROMTOR, 100.0f, true))
                        {
                            gromtor->SetReactState(REACT_AGGRESSIVE);
                            gromtor->GetMotionMaster()->MovePoint(7, -3657.8, 1834.25, 58.91);
                        }

                        if (Creature* borak = GetClosestCreatureWithEntry(me, NPC_BORAK, 100.0f, true))
                        {
                            borak->SetReactState(REACT_AGGRESSIVE);
                            borak->GetMotionMaster()->MovePoint(7, -3651.117, 1839.044, 57.114);
                        }
                        cyrukh->MonsterYell(-1200018, LANG_UNIVERSAL, 0);
                        cyrukh->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                        cyrukh->SetReactState(REACT_AGGRESSIVE);
                        cyrukh->SetAggroRange(120.0f);
                        Player* plr = Unit::GetPlayerInWorld(PlayerGUID);
                        if(plr && plr->GetDistance(me) <= 15.0)
                        {
                            cyrukh->AI()->AttackStart(me);
                            cyrukh->AI()->AttackStart(plr);
                        }
                        else
                            cyrukh->AI()->AttackStart(me);
                        CyrukhYell = true;
                    }
                }
            }
            return;
        }

        if (UpdateVictim())
        {
            if(EarthbindTotemTimer.Expired(uiDiff))
            {
                AddSpellToCast(me->GetVictim(), 15786, false);
                EarthbindTotemTimer = 10000;
            }

            if(ChainLightTimer.Expired(uiDiff))
            {
                AddSpellToCast(me->GetVictim(), 16006, false);
                ChainLightTimer = 3000;
            }

            if(FrostShockTimer.Expired(uiDiff))
            {
                AddSpellToCast(me->GetVictim(), 12548, false);
                FrostShockTimer = 7000;
            }

            if (HealingWaveTimer.Expired(uiDiff))
            {
                if (Unit* pTarget = SelectLowestHpFriendly(20.0f, 0, true))
                {
                    AddSpellToCast(pTarget, 12491, false);
                    HealingWaveTimer = 8000;
                }
            }
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_oronok(Creature* pCreature)
{
    return new npc_oronokAI(pCreature);
}

#define GOSSIP_OSE 16049

bool GossipHello_npc_oronok(Player* player, Creature* _Creature)
{
    if (GetClosestCreatureWithEntry(_Creature, NPC_GULDAN, 9.0f, true))
    {
        if (player->GetQuestStatus(QUEST_CIPHER_OF_DAMNATION) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_OSE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        player->SEND_GOSSIP_MENU(10421, _Creature->GetGUID());
    }
    return true;
}

bool GossipSelect_npc_oronok(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
        {
            _Creature->MonsterYell(-1200008, LANG_UNIVERSAL, 0);
            CAST_AI(npc_oronokAI, _Creature->AI())->EventResume = true;
            CAST_AI(npc_oronokAI, _Creature->AI())->PlayerGUID = player->GetObjectGuid();
            player->CLOSE_GOSSIP_MENU();
            break;
        }
        default: break;
    }
    return true;
}

struct npc_cyrukh_the_firelordAI : public ScriptedAI
{
    npc_cyrukh_the_firelordAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    bool CyrukhDied;
    bool GetOroGUID;
    uint64 OronokGUID;
    Timer FelFlamestrikeTimer;
    Timer KnockAwayTimer;
    Timer TrampleTimer;

    void Reset()
    {
        CyrukhDied = false;
        GetOroGUID = false;
        OronokGUID = 0;
        FelFlamestrikeTimer.Reset(15000);
        KnockAwayTimer.Reset(7000);
        TrampleTimer.Reset(8000);
    }

    void JustDied(Unit* killer)
    {    
        if (Creature* oronok = me->GetMap()->GetCreature(OronokGUID))
        {
            CAST_AI(npc_oronokAI, oronok->AI())->CyrukhDied = true;
        }
        Creature* pGuldan = GetClosestCreatureWithEntry(me, NPC_GULDAN, 150);
        if (pGuldan)
            CAST_AI(npc_guldanAI, pGuldan->AI())->ResetTimer = 1000;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (!GetOroGUID)
            {
                if (Creature* oronok = GetClosestCreatureWithEntry(me, NPC_ORONOK, 100.0f, true))
                {
                    OronokGUID = oronok->GetGUID();
                    GetOroGUID = true;
                }
            }
            return;
        }

        if(UpdateVictim())
        {
            if(FelFlamestrikeTimer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), 39429, false);
                FelFlamestrikeTimer = 15000;
            }
            if(KnockAwayTimer.Expired(diff))
            {
                AddSpellToCast(me->GetVictim(), 18945, false);
                KnockAwayTimer = 8000;
            }

            if(TrampleTimer.Expired(diff))
            {
                AddSpellToCast(me, 39425, false);
                TrampleTimer = 9000;
            }
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_cyrukh_the_firelord(Creature* pCreature)
{
    return new npc_cyrukh_the_firelordAI(pCreature);
}


/*######
## npc_disobedient_dragonmaw_peon - ID: 23311
######*/

enum DisobedientDragonmawPeon
{
    QUEST_LAZY_PEONS        = 5441,
    GO_LUMBERPILE           = 23308,
    SPELL_BOOTERANG         = 40742
};

struct npc_23311AI : public ScriptedAI
{
    npc_23311AI(Creature *c) : ScriptedAI(c) {}

    bool work;
    Timer KickTimer;
    Timer SunderArmorTimer;

    void Reset()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_PL_SPELL_TARGET);
        me->RemoveAurasDueToSpell(40735);
        me->CastSpell(me, 40732, false);
        work = false;
        KickTimer.Reset(3000);
        SunderArmorTimer.Reset(1000);
    }

    void EnterCombat(Unit* who)
    {
        me->CastSpell(me, 40735, false);
    }

    void MovementInform(uint32 /*type*/, uint32 id)
    {
        if (id == 1)
        {
            work = true;
            if (Creature* WorkNode = GetClosestCreatureWithEntry(me, 23308, 200))
                me->SetFacingToObject(WorkNode);
            me->ForcedDespawn(30000);
        }
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_BOOTERANG && caster->GetTypeId() == TYPEID_PLAYER)
        {
            switch(urand(0, 6))
            {
                case 0:
                    me->Say(-1200127, LANG_UNIVERSAL, 0);
                    break;
                case 1:
                    me->Say(-1200128, LANG_UNIVERSAL, 0);
                    break;
                case 2:
                    me->Say(-1200129, LANG_UNIVERSAL, 0);
                    break;
                case 3:
                    me->Say(-1200130, LANG_UNIVERSAL, 0);
                    break;
                case 4:
                    me->Say(-1200131, LANG_UNIVERSAL, 0);
                    break;
                case 5:
                    me->Say(-1200132, LANG_UNIVERSAL, 0);
                    break;
                case 6:
                    me->Say(-1200133, LANG_UNIVERSAL, 0);
                    break;
                default: break;
            }
            me->RemoveAllAuras();
            me->CastSpell(me, 40714, false);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_PL_SPELL_TARGET);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            if (Creature* WorkNode = GetClosestCreatureWithEntry(me, 23308, 200))
                me->GetMotionMaster()->MovePoint(1,WorkNode->GetPositionX()-1,WorkNode->GetPositionY(),WorkNode->GetPositionZ());
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (work == true)
            me->HandleEmoteCommand(233);

        if (!UpdateVictim())
            return;

        if(KickTimer.Expired(uiDiff))
        {
            me->CastSpell(me->GetVictim(), 34802, false);
            KickTimer = 10000;
        }

        if(SunderArmorTimer.Expired(uiDiff))
        {
            me->CastSpell(me->GetVictim(), 15572, false);
            SunderArmorTimer = urand(2000, 3000);
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_23311(Creature* pCreature)
{
    return new npc_23311AI(pCreature);
}

// Dragonmaw Race: The Ballad of Oldie Muckjaw
// QuestID: 11064

struct mob_23340AI : public npc_escortAI
{
    mob_23340AI(Creature* creature) : npc_escortAI(creature) {}

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            SetRun(false);
            me->SetWalk(true);
            me->SetFlying(false);
            me->SetSpeed(MOVE_FLIGHT, 1.4f);
        }
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 4:
                SetRun(true);
                me->SetWalk(false);
                me->SetFlying(true);
                break;
            case 7:
                DoCast(me, 40847);
                me->SetSpeed(MOVE_FLIGHT, 2.5f);
                break;
            case 35:
                SetRun(false);
                me->SetWalk(true);
                me->SetFlying(false);
                me->Say(-1200134, LANG_UNIVERSAL, 0);
                if(Player* player = GetPlayerForEscort())
                    player->AreaExploredOrEventHappens(11064);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetSpeed(MOVE_FLIGHT, 1.4f);
                break;
            default:
                switch(urand(0, 2))
                {
                    case 0:
                        break;
                    case 1:
                        if(i > 10)
                        {
                            me->CastSpell(me, 40890, false);
                        }
                        break;
                    case 2:
                        break;
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_23340(Creature* pCreature)
{
    mob_23340AI* thisAI = new mob_23340AI(pCreature);
    thisAI->AddWaypoint(1,  -5100.367, 646.988, 86.75992, 0),
    thisAI->AddWaypoint(2,  -5098.652, 661.8313, 87.08841, 0),
    thisAI->AddWaypoint(3,  -5092.219, 664.3353, 87.73563, 0),
    thisAI->AddWaypoint(4,  -5081.346, 664.3699, 88.98929, 0),
    thisAI->AddWaypoint(5,  -5070.526, 664.555, 92.49088, 0),
    thisAI->AddWaypoint(6,  -5058.811, 664.4887, 96.62975, 0),
    thisAI->AddWaypoint(7,  -5046.153, 664.4279, 97.79643, 0),
    thisAI->AddWaypoint(8,  -5021.002 ,664.601 ,115.265, 0),
    thisAI->AddWaypoint(9,  -4996.027 ,664.6683 ,115.2651, 0),
    thisAI->AddWaypoint(10, -4957.755 ,673.371 ,115.2651, 0),
    thisAI->AddWaypoint(11, -4922.072 ,686.9524 ,115.2651, 0),
    thisAI->AddWaypoint(12, -4892.223 ,711.1226 ,115.2651, 0),
    thisAI->AddWaypoint(13, -4899.887 ,760.2698 ,115.2651, 0),
    thisAI->AddWaypoint(14, -4938.349 ,778.5325 ,115.2651, 0),
    thisAI->AddWaypoint(15, -4971.547 ,797.5603 ,115.2651, 0),
    thisAI->AddWaypoint(16, -5006.853 ,803.2005 ,115.2651, 0),
    thisAI->AddWaypoint(17, -5052.229 ,803.8342 ,115.2651, 0),
    thisAI->AddWaypoint(18, -5084.652 ,784.2522 ,115.2651, 0),
    thisAI->AddWaypoint(19, -5112.79 ,768.5128 ,115.2651, 0),
    thisAI->AddWaypoint(20, -5131.785 ,755.4612 ,115.2651, 0),
    thisAI->AddWaypoint(21, -5156.098 ,731.316 ,115.2651, 0),
    thisAI->AddWaypoint(22, -5151.251 ,703.8534 ,115.2651, 0),
    thisAI->AddWaypoint(23, -5132.957 ,672.912 ,115.2651, 0),
    thisAI->AddWaypoint(24, -5098.931 ,650.7557 ,115.2651, 0),
    thisAI->AddWaypoint(25, -5062.084 ,634.9383 ,135.515, 0),
    thisAI->AddWaypoint(26, -5027.877 ,621.5608 ,129.1817, 0),
    thisAI->AddWaypoint(27, -5011.851 ,622.0031 ,127.8948, 0),
    thisAI->AddWaypoint(28, -4977.219 ,627.5208 ,123.8948, 0),
    thisAI->AddWaypoint(29, -4955.773 ,632.8429 ,106.7004, 0),
    thisAI->AddWaypoint(30, -4959.098 ,655.7993 ,100.4226, 0),
    thisAI->AddWaypoint(31, -4990.86 ,667.564 ,100.3392, 0),
    thisAI->AddWaypoint(32, -5016.541, 664.3353, 95.70948, 0),
    thisAI->AddWaypoint(33, -5030.557, 664.4094, 94.87615, 0),
    thisAI->AddWaypoint(34, -5053.869, 664.2513, 91.20948, 0),
    thisAI->AddWaypoint(35, -5076.172, 664.1518, 89.73725, 0),
    thisAI->AddWaypoint(36, -5088.555, 640.8356, 86.57706, 0);
    return (CreatureAI*)thisAI;
}

bool QuestAccept_mob_23340(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 11064)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(mob_23340AI, creature->AI()))
        {
            pEscortAI->Start(false, false, player->GetGUID(), quest, true, false);
            pEscortAI->SetMaxPlayerDistance(100.0f);
            pEscortAI->SetDespawnAtEnd(true);
            pEscortAI->SetDespawnAtFar(true);
        }
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        creature->Say(-1200135, LANG_UNIVERSAL, player->GetGUID());

    }
    return true;
}

// Dragonmaw Race: Trope the Filth-Belcher
// QuestID: 11067
struct mob_23342AI : public npc_escortAI
{
    mob_23342AI(Creature* creature) : npc_escortAI(creature) {}

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            SetRun(false);
            me->SetWalk(true);
            me->SetFlying(false);
            me->SetSpeed(MOVE_FLIGHT, 1.4f);
        }
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 4:
                SetRun(true);
                me->SetWalk(false);
                me->SetFlying(true);
                break;
            case 7:
                DoCast(me, 40847);
                me->SetSpeed(MOVE_FLIGHT, 2.8f);
                break;
            case 35:
                SetRun(false);
                me->SetWalk(true);
                me->SetFlying(false);
                DoScriptText(-1811045, me);
                if(Player* player = GetPlayerForEscort())
                    player->AreaExploredOrEventHappens(11067);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetSpeed(MOVE_FLIGHT, 1.0f);
                break;
            default:
                switch(urand(0, 2))
                {
                    case 0:
                        break;
                    case 1:
                        if(i > 10)
                        {
                            me->CastSpell(me, 40909, false);
                        }
                        break;
                    case 2:
                        break;
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_23342(Creature* pCreature)
{
    mob_23342AI* thisAI = new mob_23342AI(pCreature);

    thisAI->AddWaypoint(1,  -5095.04, 631.376, 85.98, 0),
    thisAI->AddWaypoint(2,  -5101.51,638.928,86.2226, 0),
    thisAI->AddWaypoint(3,  -5097.51,664.528,87.0487, 0),
    thisAI->AddWaypoint(4,  -5076.98,664.286,89.2126, 0),
    thisAI->AddWaypoint(5,  -5063.02,664.836,92.8208, 0),
    thisAI->AddWaypoint(6,  -5055.49,664.904,95.949, 0),
    thisAI->AddWaypoint(7,  -5039.83,665.182,102.55, 0),
    thisAI->AddWaypoint(8,  -5003.76,716.292,106.139, 0),
    thisAI->AddWaypoint(9,  -5003.92,740.802,112.833, 0),
    thisAI->AddWaypoint(10, -5012.64,754.221,134.72, 0),
    thisAI->AddWaypoint(11, -5039.54,758.968,150.397, 0),
    thisAI->AddWaypoint(12, -5103.13,731.165,141.061, 0),
    thisAI->AddWaypoint(13, -5146.54,669.45,128.54, 0),
    thisAI->AddWaypoint(14, -5163.2,620.59,121.106, 0),
    thisAI->AddWaypoint(15, -5146.05,576.048,127.822, 0),
    thisAI->AddWaypoint(16, -5116.93,527.751,134.292, 0),
    thisAI->AddWaypoint(17, -5082.93,489.763,140.698, 0),
    thisAI->AddWaypoint(18, -5034.08,471.257,138.042, 0),
    thisAI->AddWaypoint(19, -4957.35,537.708,127.94, 0),
    thisAI->AddWaypoint(20, -4920.24,537.923,122.891, 0),
    thisAI->AddWaypoint(21, -4882.15,516.69,115.049, 0),
    thisAI->AddWaypoint(22, -4838.47,463.897,116.005, 0),
    thisAI->AddWaypoint(23, -4854.38,429.28,114.982, 0),
    thisAI->AddWaypoint(24, -4889.74,392.992,115.213, 0),
    thisAI->AddWaypoint(25, -4934.51,394.088,118.871, 0),
    thisAI->AddWaypoint(26, -4965.72,412.182,116.835, 0),
    thisAI->AddWaypoint(27, -4997.18,453.119,107.507, 0),
    thisAI->AddWaypoint(28, -5002.18,511.192,105.102, 0),
    thisAI->AddWaypoint(29, -4964.8,542.731,116.997, 0),
    thisAI->AddWaypoint(30, -4939.05,575.649,117.287, 0),
    thisAI->AddWaypoint(31, -4951.99,620.47,116.653, 0),
    thisAI->AddWaypoint(32, -4958.32,642.701,114.999, 0),
    thisAI->AddWaypoint(33, -4985.8,664.416,107.004, 0),
    thisAI->AddWaypoint(34, -5017.02,664.946,99.5553, 0),
    thisAI->AddWaypoint(35, -5054.12,664.324,89.5707, 0),
    thisAI->AddWaypoint(36, -5094.33,663.136,87.4461, 0),
    thisAI->AddWaypoint(37, -5098.6,636.61,86.165, 0),
    thisAI->AddWaypoint(38, -5081.62,640.985,86.4988, 0);
    return (CreatureAI*)thisAI;
}

bool QuestAccept_mob_23342(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 11067)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(mob_23342AI, creature->AI()))
        {
            pEscortAI->Start(false, false, player->GetGUID(), quest, true, false);
            pEscortAI->SetMaxPlayerDistance(100.0f);
            pEscortAI->SetDespawnAtEnd(true);
            pEscortAI->SetDespawnAtFar(true);
        }
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        DoScriptText(-1811044, creature);
    }
    return true;
}

// Dragonmaw Race: Corlok the Vet
// QuestID: 11068
struct mob_23344AI : public npc_escortAI
{
    mob_23344AI(Creature* creature) : npc_escortAI(creature) {}

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            SetRun(false);
            me->SetWalk(true);
            me->SetFlying(false);
            me->SetSpeed(MOVE_FLIGHT, 1.4f);
        }
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 4:
                SetRun(true);
                me->SetWalk(false);
                me->SetFlying(true);
                DoCast(me, 40847);
                me->SetSpeed(MOVE_FLIGHT, 3.1f);
                break;
            case 50:
                SetRun(false);
                me->SetWalk(true);
                me->SetFlying(false);
                if(Player* player = GetPlayerForEscort())
                    player->AreaExploredOrEventHappens(11068);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetSpeed(MOVE_FLIGHT, 1.4f);
                break;
            default:
                switch(urand(0, 2))
                {
                    case 0:
                        break;
                    case 1:
                        if(i > 6 && i < 50)
                        {
                            me->CastSpell(me, 40894, false);
                        }
                        break;
                    case 2:
                        break;
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_23344(Creature* pCreature)
{
    mob_23344AI* thisAI = new mob_23344AI(pCreature);

    thisAI->AddWaypoint(1,-5101.1,634.919,86.0679),
    thisAI->AddWaypoint(2,-5096.23,665.088,87.1613),
    thisAI->AddWaypoint(3,-5080.62,664.253,88.9773),
    thisAI->AddWaypoint(4,-5067.79,664.484,90.0847),
    thisAI->AddWaypoint(5,-5049.4,664.941,93.8037),
    thisAI->AddWaypoint(6,-5023.62,633.935,108.995),
    thisAI->AddWaypoint(7,-5008.69,620.496,126.899),
    thisAI->AddWaypoint(8,-4963.77,613.346,130.925),
    thisAI->AddWaypoint(9,-4966.16,559.108,128.529),
    thisAI->AddWaypoint(10,-5017.13,511.42,126.704),
    thisAI->AddWaypoint(11,-5108.72,498.009,123.987),
    thisAI->AddWaypoint(12,-5162.83,478.071,119.042),
    thisAI->AddWaypoint(13,-5214.75,445.843,116.492),
    thisAI->AddWaypoint(14,-5273.78,359.428,72.8584),
    thisAI->AddWaypoint(15,-5290.65,321.929,74.8425),
    thisAI->AddWaypoint(16,-5268.61,278.648,73.952),
    thisAI->AddWaypoint(17,-5215.51,261.122,75.2018),
    thisAI->AddWaypoint(18,-5205.11,225.255,77.2236),
    thisAI->AddWaypoint(19,-5210.15,169.279,73.2162),
    thisAI->AddWaypoint(20,-5199.53,124.766,78.2695),
    thisAI->AddWaypoint(21,-5182.41,85.8157,102.115),
    thisAI->AddWaypoint(22,-5135.35,53.1105,127.883),
    thisAI->AddWaypoint(23,-5068.5,46.0242,127.96),
    thisAI->AddWaypoint(24,-5034.17,29.6569,126.013),
    thisAI->AddWaypoint(25,-4983.62,-52.6875,126.012),
    thisAI->AddWaypoint(26,-5057.64,-96.0699,129.252),
    thisAI->AddWaypoint(27,-5076.86,-132.561,127.299),
    thisAI->AddWaypoint(28,-5149.52,-130.523,116.489),
    thisAI->AddWaypoint(29,-5170.51,-101.976,111.818),
    thisAI->AddWaypoint(30,-5196.34,-28.7249,106.596),
    thisAI->AddWaypoint(31,-5214.41,32.5714,119.088),
    thisAI->AddWaypoint(32,-5267.03,83.5835,119.39),
    thisAI->AddWaypoint(33,-5315.6,172.648,88.754),
    thisAI->AddWaypoint(34,-5324.03,231.162,91.1392),
    thisAI->AddWaypoint(35,-5280.99,255.476,93.2841),
    thisAI->AddWaypoint(36,-5245.47,265.518,81.5809),
    thisAI->AddWaypoint(37,-5209,273.942,74.1521),
    thisAI->AddWaypoint(38,-5178.67,305.31,74.859),
    thisAI->AddWaypoint(39,-5175.24,333.119,75.5546),
    thisAI->AddWaypoint(40,-5191.24,376.572,81.5842),
    thisAI->AddWaypoint(41,-5199.72,408.996,85.5877),
    thisAI->AddWaypoint(42,-5180.75,448.672,90.1216),
    thisAI->AddWaypoint(43,-5134.22,501.946,95.5528),
    thisAI->AddWaypoint(44,-5084.14,521.92,99.6632),
    thisAI->AddWaypoint(45,-5012.67,516.93,100.909),
    thisAI->AddWaypoint(46,-4965.6,555.552,89.3292),
    thisAI->AddWaypoint(47,-4967.01,614.967,98.0559),
    thisAI->AddWaypoint(48,-4978.41,646.953,101.696),
    thisAI->AddWaypoint(49,-4996.88,665.704,92.8607),
    thisAI->AddWaypoint(50,-5046.07,664.15,89.5706),
    thisAI->AddWaypoint(51,-5096.81,663.131,87.1709),
    thisAI->AddWaypoint(52,-5094.86,634.796,86.1543),
    thisAI->AddWaypoint(53,-5073.12,640.492,86.4748);

    return (CreatureAI*)thisAI;
}

bool QuestAccept_mob_23344(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 11068)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(mob_23344AI, creature->AI()))
        {
            pEscortAI->Start(false, false, player->GetGUID(), quest, true, false);
            pEscortAI->SetMaxPlayerDistance(100.0f);
            pEscortAI->SetDespawnAtEnd(true);
            pEscortAI->SetDespawnAtFar(true);
        }
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        DoScriptText(-1811046, creature);
    }
    return true;
}

// Dragonmaw Race: Wing Commander Ichman
// QuestID: 11069

struct mob_23345AI : public npc_escortAI
{
    mob_23345AI(Creature* creature) : npc_escortAI(creature) {}

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            SetRun(false);
            me->SetWalk(true);
            me->SetFlying(false);
            me->SetSpeed(MOVE_FLIGHT, 1.4f);
        }
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 4:
                SetRun(true);
                me->SetWalk(false);
                me->SetFlying(true);
                break;
            case 7:
                DoCast(me, 40847);
                me->SetSpeed(MOVE_FLIGHT, 3.4f);
                break;
            case 80:
                SetRun(false);
                me->SetWalk(true);
                me->SetFlying(false);
                if(Player* player = GetPlayerForEscort())
                    player->AreaExploredOrEventHappens(11069);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetSpeed(MOVE_FLIGHT, 1.4f);
                DoScriptText(-1811050, me);
                break;
            default:
                switch(urand(0, 1))
                {
                    case 0:
                        break;
                    case 1:
                        if(i > 10 && i < 79)
                        {
                            me->CastSpell(me, 40928, false);
                        }
                        break;
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_23345(Creature* pCreature)
{
    mob_23345AI* thisAI = new mob_23345AI(pCreature);

    thisAI->AddWaypoint(1,  -5074.62,   639.673,    86.9331),
    thisAI->AddWaypoint(2,  -5091.72,   640.899,    86.9896),
    thisAI->AddWaypoint(3,  -5099.78,   661.451,    87.4432),
    thisAI->AddWaypoint(4,  -5090.78,   664.342,    88.3554),
    thisAI->AddWaypoint(5,  -5067.89,   664.518,    89.9139),
    thisAI->AddWaypoint(6,  -4974.12,   662.561,    93.8796),
    thisAI->AddWaypoint(7,  -4906.46,   640.801,    88.7705),
    thisAI->AddWaypoint(8,  -4921.31,   579.589,    88.8252),
    thisAI->AddWaypoint(9,  -4970.17,   540.006,    100.316),
    thisAI->AddWaypoint(10, -5018.59,   499.593,    97.0844),
    thisAI->AddWaypoint(11, -5001.56,   456.6,      94.0664),
    thisAI->AddWaypoint(12, -4992.14,   436.068,    96.5439),
    thisAI->AddWaypoint(13, -4979.81,   461.129,    96.0511),
    thisAI->AddWaypoint(14, -4995.22,   465.137,    96.0437),
    thisAI->AddWaypoint(15, -4979.78,   422.108,    96.2742),
    thisAI->AddWaypoint(16, -4948.8,    384.394,    97.8985),
    thisAI->AddWaypoint(17, -4941.75,   377.289,    95.2171),
    thisAI->AddWaypoint(18, -4920.82,   377.354,    86.4916),
    thisAI->AddWaypoint(19, -4863.89,   389.621,    100.531),
    thisAI->AddWaypoint(20, -4799.8,    356.848,    100.664),
    thisAI->AddWaypoint(21, -4816.32,   307.672,    98.9476),
    thisAI->AddWaypoint(22, -4860.27,   253.41,     90.9479),
    thisAI->AddWaypoint(23, -4928.57,   234.724,    73.3838),
    thisAI->AddWaypoint(24, -4979.44,   234.729,    99.059),
    thisAI->AddWaypoint(25, -4997.64,   187.217,    108.79),
    thisAI->AddWaypoint(26, -4999.75,   133.452,    92.5512),
    thisAI->AddWaypoint(27, -5006.28,   70.9775,    92.9455),
    thisAI->AddWaypoint(28, -4984.59,   21.7855,    92.4787),
    thisAI->AddWaypoint(29, -4939.45,   -1.70913,   89.5908),
    thisAI->AddWaypoint(30, -4904.5,    -20.3318,   98.7926),
    thisAI->AddWaypoint(31, -4876.37,   -59.5791,   91.4677),
    thisAI->AddWaypoint(32, -4883.15,   -100.723,   103.205),
    thisAI->AddWaypoint(33, -4944.32,   -112.767,   137.646),
    thisAI->AddWaypoint(34, -4971.8,    -78.7316,   115.101),
    thisAI->AddWaypoint(35, -4987.65,   -30.5438,   91.9287),
    thisAI->AddWaypoint(36, -5001.3,    12.3534,    86.9422),
    thisAI->AddWaypoint(37, -5042.09,   46.6422,    105.125),
    thisAI->AddWaypoint(38, -5106.43,   50.7488,    83.1864),
    thisAI->AddWaypoint(39, -5154.69,   50.1247,    82.3862),
    thisAI->AddWaypoint(40, -5217.06,   23.4145,    150.396),
    thisAI->AddWaypoint(41, -5225.16,   -39.4703,   174.652),
    thisAI->AddWaypoint(42, -5307.91,   -25.5939,   145.381),
    thisAI->AddWaypoint(43, -5274.71,   26.4266,    172.198),
    thisAI->AddWaypoint(44, -5208.27,   57.5883,    159.623),
    thisAI->AddWaypoint(45, -5142.88,   97.5982,    149.853),
    thisAI->AddWaypoint(46, -5115.44,   118.069,    136.045),
    thisAI->AddWaypoint(47, -5105.17,   155.088,    134.461),
    thisAI->AddWaypoint(48, -5106.37,   218.731,    147.258),
    thisAI->AddWaypoint(49, -5094.3,    280.021,    161.935),
    thisAI->AddWaypoint(50, -5084.01,   316.753,    172.277),
    thisAI->AddWaypoint(51, -5052.64,   351.108,    172.987),
    thisAI->AddWaypoint(52, -5033.05,   379.481,    174.407),
    thisAI->AddWaypoint(53, -5028.54,   413.128,    179.762),
    thisAI->AddWaypoint(54, -5017.09,   484.101,    168.921),
    thisAI->AddWaypoint(55, -5020.9,    515.958,    120.09),
    thisAI->AddWaypoint(56, -5062.5,    521.002,    97.215),
    thisAI->AddWaypoint(57, -5103.9,    494.01,     104.071),
    thisAI->AddWaypoint(58, -5151.95,   462.724,    111.081),
    thisAI->AddWaypoint(59, -5080.26,   498.915,    109.875),
    thisAI->AddWaypoint(60, -5157.65,   450.443,    106.745),
    thisAI->AddWaypoint(61, -5193.28,   400.018,    108.903),
    thisAI->AddWaypoint(62, -5189.23,   370.25,     113.73),
    thisAI->AddWaypoint(63, -5185.8,    313.755,    132.317),
    thisAI->AddWaypoint(64, -5195.96,   278.514,    132.06),
    thisAI->AddWaypoint(65, -5224.53,   274.103,    131.872),
    thisAI->AddWaypoint(66, -5271.16,   332.176,    120.293),
    thisAI->AddWaypoint(67, -5284.09,   392.53,     121.367),
    thisAI->AddWaypoint(68, -5234.58,   426.247,    124.65),
    thisAI->AddWaypoint(69, -5204.57,   440.941,    125.71),
    thisAI->AddWaypoint(70, -5238.07,   411.06,     122.894),
    thisAI->AddWaypoint(71, -5154.52,   497.25,     110.277),
    thisAI->AddWaypoint(72, -5062.88,   505.464,    107.491),
    thisAI->AddWaypoint(73, -4998.45,   523.002,    102.513),
    thisAI->AddWaypoint(74, -5047.71,   512.549,    101.472),
    thisAI->AddWaypoint(75, -4975.4,    546.978,    98.4481),
    thisAI->AddWaypoint(76, -4940.06,   582.665,    93.8702),
    thisAI->AddWaypoint(77, -4936.29,   629.309,    91.3448),
    thisAI->AddWaypoint(78, -4986.95,   667.351,    96.8044),
    thisAI->AddWaypoint(79, -5036.81,   665.996,    92.8608),
    thisAI->AddWaypoint(80, -5079.51,   664.887,    89.0742),
    thisAI->AddWaypoint(81, -5094.95,   663.808,    87.6571),
    thisAI->AddWaypoint(82, -5100.96,   648.648,    87.1004),
    thisAI->AddWaypoint(83, -5084.34,   638.799,    86.9905);
    return (CreatureAI*)thisAI;
}

bool QuestAccept_mob_23345(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 11069)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(mob_23345AI, creature->AI()))
        {
            pEscortAI->Start(false, false, player->GetGUID(), quest, true, false);
            pEscortAI->SetMaxPlayerDistance(100.0f);
            pEscortAI->SetDespawnAtEnd(true);
            pEscortAI->SetDespawnAtFar(true);
        }
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        DoScriptText(-1811047, creature);
    }
    return true;
}

// Dragonmaw Race: Wing Commander Mulverick
// QuestID: 11070

struct mob_23346AI : public npc_escortAI
{
    mob_23346AI(Creature* creature) : npc_escortAI(creature) {}

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            SetRun(false);
            me->SetWalk(true);
            me->SetFlying(false);
            me->SetSpeed(MOVE_FLIGHT, 1.4f);
        }
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 4:
                SetRun(true);
                me->SetWalk(false);
                me->SetFlying(true);
                break;
            case 7:
                DoCast(me, 40847);
                me->SetSpeed(MOVE_FLIGHT, 3.7f);
                break;
            case 137:
                SetRun(false);
                me->SetWalk(true);
                me->SetFlying(false);
                if(Player* player = GetPlayerForEscort())
                    player->AreaExploredOrEventHappens(11070);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetSpeed(MOVE_FLIGHT, 1.4f);
                DoScriptText(-1811049, me);
                break;
            default:
                switch(urand(0, 1))
                {
                    case 0:
                        break;
                    case 1:
                        if(i > 10 && i < 136)
                        {
                            me->CastSpell(me, 40930, false);
                        }
                        break;
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_23346(Creature* pCreature)
{
    mob_23346AI* thisAI = new mob_23346AI(pCreature);

    thisAI->AddWaypoint(1,-5073.79,640.175,86.9597),
    thisAI->AddWaypoint(2,-5093.49,642.838,87.0724),
    thisAI->AddWaypoint(3,-5097.66,661.13,87.6415),
    thisAI->AddWaypoint(4,-5070.33,664.518,89.9677),
    thisAI->AddWaypoint(5,-4980.29,664.857,93.3151),
    thisAI->AddWaypoint(6,-4923.65,661.955,85.6817),
    thisAI->AddWaypoint(7,-4897.38,687.397,85.903),
    thisAI->AddWaypoint(8,-4894.5,731.107,94.2763),
    thisAI->AddWaypoint(9,-4909.02,768.031,88.1273),
    thisAI->AddWaypoint(10,-4944.72,786.339,89.3095),
    thisAI->AddWaypoint(11,-4978.84,796.752,122.538),
    thisAI->AddWaypoint(12,-5020.1,794.71,104.245),
    thisAI->AddWaypoint(13,-5051.79,790.186,90.4867),
    thisAI->AddWaypoint(14,-5094.32,778.473,79.7089),
    thisAI->AddWaypoint(15,-5139.05,763.19,68.1027),
    thisAI->AddWaypoint(16,-5179.04,773.369,61.2301),
    thisAI->AddWaypoint(17,-5255.2,791.07,49.3913),
    thisAI->AddWaypoint(18,-5276.02,781.002,47.2046),
    thisAI->AddWaypoint(19,-5272.19,763.952,49.0412),
    thisAI->AddWaypoint(20,-5280.86,729.029,52.1453),
    thisAI->AddWaypoint(21,-5269.54,687.2,53.4351),
    thisAI->AddWaypoint(22,-5260.46,632.333,53.8001),
    thisAI->AddWaypoint(23,-5260.06,607.943,59.746),
    thisAI->AddWaypoint(24,-5253.74,659.762,53.9552),
    thisAI->AddWaypoint(25,-5270.44,641.98,52.628),
    thisAI->AddWaypoint(26,-5259.89,607.863,56.4182),
    thisAI->AddWaypoint(27,-5188.08,598.437,88.6164),
    thisAI->AddWaypoint(28,-5176.6,549.729,89.9904),
    thisAI->AddWaypoint(29,-5168.49,522.349,91.4781),
    thisAI->AddWaypoint(30,-5143.6,507.12,92.2422),
    thisAI->AddWaypoint(31,-5091.26,495.271,94.0692),
    thisAI->AddWaypoint(32,-4973.18,421.727,90.9961),
    thisAI->AddWaypoint(33,-4965.04,392.479,89.3456),
    thisAI->AddWaypoint(34,-4971.82,354.271,87.915),
    thisAI->AddWaypoint(35,-4989.4,301.984,86.3694),
    thisAI->AddWaypoint(36,-5014.7,279.451,90.6383),
    thisAI->AddWaypoint(37,-5040.7,225.805,108.337),
    thisAI->AddWaypoint(38,-5062.34,181.557,129.365),
    thisAI->AddWaypoint(39,-5082.6,168.477,130.375),
    thisAI->AddWaypoint(40,-5100.59,180.857,138.101),
    thisAI->AddWaypoint(41,-5101.1,229.947,149.149),
    thisAI->AddWaypoint(42,-5093.44,274.454,160.411),
    thisAI->AddWaypoint(43,-5080.44,309.374,169.419),
    thisAI->AddWaypoint(44,-5056.87,347.893,174.217),
    thisAI->AddWaypoint(45,-5010.93,365.086,177.488),
    thisAI->AddWaypoint(46,-4968.65,380.724,173.173),
    thisAI->AddWaypoint(47,-5027.66,364.737,177.539),
    thisAI->AddWaypoint(48,-4917.15,377.454,173.836),
    thisAI->AddWaypoint(49,-4853.53,360.39,176.391),
    thisAI->AddWaypoint(50,-4835.68,388.264,168.752),
    thisAI->AddWaypoint(51,-4850.91,422.375,158.973),
    thisAI->AddWaypoint(52,-4864.4,460.468,150.407),
    thisAI->AddWaypoint(53,-4902.27,500.825,134.835),
    thisAI->AddWaypoint(54,-4946.1,528.796,121.743),
    thisAI->AddWaypoint(55,-4994.91,527.295,99.9505),
    thisAI->AddWaypoint(56,-5023.6,508.386,119.743),
    thisAI->AddWaypoint(57,-5055,490.278,169.672),
    thisAI->AddWaypoint(58,-5106.63,462.668,165.688),
    thisAI->AddWaypoint(59,-5146.6,430.134,159.699),
    thisAI->AddWaypoint(60,-5167.97,405.528,144.235),
    thisAI->AddWaypoint(61,-5178.58,345.856,127.211),
    thisAI->AddWaypoint(62,-5188.11,313.544,124.417),
    thisAI->AddWaypoint(63,-5199.91,282.739,112.936),
    thisAI->AddWaypoint(64,-5245.22,271.312,97.5307),
    thisAI->AddWaypoint(65,-5270.46,271.393,105.523),
    thisAI->AddWaypoint(66,-5288.75,241.08,101.587),
    thisAI->AddWaypoint(67,-5313.95,195.752,100.433),
    thisAI->AddWaypoint(68,-5289.44,116.574,101.397),
    thisAI->AddWaypoint(69,-5260.55,69.3146,106.89),
    thisAI->AddWaypoint(70,-5237.17,-12.7776,120.017),
    thisAI->AddWaypoint(71,-5279.93,-67.19,109.01),
    thisAI->AddWaypoint(72,-5349.6,-111.473,116.548),
    thisAI->AddWaypoint(73,-5370.63,-98.7298,109.358),
    thisAI->AddWaypoint(74,-5387.68,-63.1046,111.659),
    thisAI->AddWaypoint(75,-5379.2,3.21685,121.526),
    thisAI->AddWaypoint(76,-5314.43,32.0712,113.405),
    thisAI->AddWaypoint(77,-5265.13,11.6882,110.016),
    thisAI->AddWaypoint(78,-5207.98,-36.7408,103.694),
    thisAI->AddWaypoint(79,-5165.57,-80.167,136.178),
    thisAI->AddWaypoint(80,-5118.2,-105.716,122.108),
    thisAI->AddWaypoint(81,-5061.11,-96.3049,117.942),
    thisAI->AddWaypoint(82,-5031.23,-66.0233,104.849),
    thisAI->AddWaypoint(83,-4998.87,-11.7168,81.3507),
    thisAI->AddWaypoint(84,-4980.24,10.3058,77.7177),
    thisAI->AddWaypoint(85,-4998.29,-7.4397,78.78),
    thisAI->AddWaypoint(86,-4971.93,15.0586,76.1905),
    thisAI->AddWaypoint(87,-4926.84,34.6864,66.9967),
    thisAI->AddWaypoint(88,-4895.08,50.6947,60.3591),
    thisAI->AddWaypoint(89,-4874.35,56.7382,2.54341),
    thisAI->AddWaypoint(90,-4845.41,26.3595,16.1138),
    thisAI->AddWaypoint(91,-4839.98,-31.9981,26.751),
    thisAI->AddWaypoint(92,-4866.88,-28.3181,40.2276),
    thisAI->AddWaypoint(93,-4851.33,-5.85707,22.9954),
    thisAI->AddWaypoint(94,-4833.69,-31.3822,25.1184),
    thisAI->AddWaypoint(95,-4874.32,-32.4778,69.1992),
    thisAI->AddWaypoint(96,-4898.07,-42.3574,89.897),
    thisAI->AddWaypoint(97,-4913.59,-54.3562,121.409),
    thisAI->AddWaypoint(98,-4937.62,-54.5098,134.404),
    thisAI->AddWaypoint(99,-4910.76,-34.7961,134.404),
    thisAI->AddWaypoint(100,-4910.34,-64.4031,134.404),
    thisAI->AddWaypoint(101,-4940.61,-38.7189,125.076),
    thisAI->AddWaypoint(102,-4961.5,-28.9198,101.354),
    thisAI->AddWaypoint(103,-4954.74,-7.93548,100.327),
    thisAI->AddWaypoint(104,-4927.17,-6.05419,100.327),
    thisAI->AddWaypoint(105,-4928.98,-37.9751,116.941),
    thisAI->AddWaypoint(106,-4953.41,-22.5698,112.888),
    thisAI->AddWaypoint(107,-4990.22,6.49273,102.639),
    thisAI->AddWaypoint(108,-4961.07,48.5569,99.0104),
    thisAI->AddWaypoint(109,-4945.81,79.1764,101.376),
    thisAI->AddWaypoint(110,-4929.71,150.686,107.66),
    thisAI->AddWaypoint(111,-4917.87,196.516,105.989),
    thisAI->AddWaypoint(112,-4898.51,259.459,103.408),
    thisAI->AddWaypoint(113,-4924.06,249.392,102.664),
    thisAI->AddWaypoint(114,-4897.3,248.342,103.752),
    thisAI->AddWaypoint(115,-4877.38,298.139,108.219),
    thisAI->AddWaypoint(116,-4862.92,327.174,106.609),
    thisAI->AddWaypoint(117,-4852.86,372.216,102.541),
    thisAI->AddWaypoint(118,-4856.24,433.69,96.3819),
    thisAI->AddWaypoint(119,-4860.41,479.956,92.4403),
    thisAI->AddWaypoint(120,-4873.13,528.395,98.0209),
    thisAI->AddWaypoint(121,-4904.29,578.768,96.3431),
    thisAI->AddWaypoint(122,-4950.79,636.883,96.0455),
    thisAI->AddWaypoint(123,-4987.17,659.419,97.2334),
    thisAI->AddWaypoint(124,-5085.9,662.095,98.4426),
    thisAI->AddWaypoint(125,-5133.23,670.744,108.086),
    thisAI->AddWaypoint(126,-5135.8,693.839,108.492),
    thisAI->AddWaypoint(127,-5117.04,714.214,108.977),
    thisAI->AddWaypoint(128,-5090.95,694.526,109.547),
    thisAI->AddWaypoint(129,-5049.35,683.563,107.932),
    thisAI->AddWaypoint(130,-4974.33,663.794,105.02),
    thisAI->AddWaypoint(131,-4948.23,656.914,104.006),
    thisAI->AddWaypoint(132,-4942.41,666.963,103.57),
    thisAI->AddWaypoint(133,-4948.29,679.504,103.05),
    thisAI->AddWaypoint(134,-4971.16,682.781,102.063),
    thisAI->AddWaypoint(135,-4996.74,666.623,90.3214),
    thisAI->AddWaypoint(136,-5058.53,664.951,89.5705),
    thisAI->AddWaypoint(137,-5087.64,664.358,88.4531),
    thisAI->AddWaypoint(138,-5102.46,655.182,87.3748),
    thisAI->AddWaypoint(139,-5097.11,643.767,87.1566),
    thisAI->AddWaypoint(140,-5080.96,641.855,86.9403);


    return (CreatureAI*)thisAI;
}

bool QuestAccept_mob_23346(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 11070)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(mob_23346AI, creature->AI()))
        {
            pEscortAI->Start(false, false, player->GetGUID(), quest, true, false);
            pEscortAI->SetMaxPlayerDistance(100.0f);
            pEscortAI->SetDespawnAtEnd(true);
            pEscortAI->SetDespawnAtFar(true);
        }
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        DoScriptText(-1811048, creature, player);
    }
    return true;
}

// Dragonmaw Race: Captain Skyshatter
// QuestID: 11071

struct mob_23348AI : public npc_escortAI
{
    mob_23348AI(Creature* creature) : npc_escortAI(creature) {}

    void Reset()
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            SetRun(false);
            me->SetWalk(true);
            me->SetFlying(false);
            me->SetSpeed(MOVE_FLIGHT, 1.4f);
        }
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 4:
                SetRun(true);
                me->SetWalk(false);
                me->SetFlying(true);
                break;
            case 7:
                DoCast(me, 40847);
                me->SetSpeed(MOVE_FLIGHT, 4.2f);
                break;
            case 138:
                SetRun(false);
                me->SetWalk(true);
                me->SetFlying(false);
                if(Player* player = GetPlayerForEscort())
                    player->AreaExploredOrEventHappens(11071);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetSpeed(MOVE_FLIGHT, 1.4f);
                DoScriptText(-1811052, me, GetPlayerForEscort());
                break;
            default:
                switch(urand(0, 1))
                {
                    case 0:
                        break;
                    case 1:
                        if(i > 10 && i < 137)
                        {
                            me->CastSpell(me, 40945, false);
                        }
                        break;
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_23348(Creature* pCreature)
{
    mob_23348AI* thisAI = new mob_23348AI(pCreature);

    thisAI->AddWaypoint(1,-5087.56,635.584,86.7302),
    thisAI->AddWaypoint(2,-5094.97,641.565,86.8949),
    thisAI->AddWaypoint(3,-5099.64,657.156,87.3064),
    thisAI->AddWaypoint(4,-5082.76,664.815,89.2039),
    thisAI->AddWaypoint(5,-4987.41,663.473,92.5703),
    thisAI->AddWaypoint(6,-4931.48,678.786,93.1402),
    thisAI->AddWaypoint(7,-4894.17,675.374,88.3474),
    thisAI->AddWaypoint(8,-4883.4,657.438,86.213),
    thisAI->AddWaypoint(9,-4884.02,635.673,84.0022),
    thisAI->AddWaypoint(10,-4911.95,640.443,92.6958),
    thisAI->AddWaypoint(11,-4949.11,665.224,92.2993),
    thisAI->AddWaypoint(12,-4995.33,667.007,95.0879),
    thisAI->AddWaypoint(13,-5042.88,668.841,97.9566),
    thisAI->AddWaypoint(14,-5106.24,671.284,109.361),
    thisAI->AddWaypoint(15,-5129.32,684.316,129.959),
    thisAI->AddWaypoint(16,-5187.61,723.587,139.018),
    thisAI->AddWaypoint(17,-5225.68,749.399,141.018),
    thisAI->AddWaypoint(18,-5241.56,758.859,139.016),
    thisAI->AddWaypoint(19,-5263.38,748.202,136.384),
    thisAI->AddWaypoint(20,-5274.45,729.379,134.006),
    thisAI->AddWaypoint(21,-5264.11,692.833,113.639),
    thisAI->AddWaypoint(22,-5251.51,644.216,98.6951),
    thisAI->AddWaypoint(23,-5227.54,586.549,73.8104),
    thisAI->AddWaypoint(24,-5232.89,617.516,73.3168),
    thisAI->AddWaypoint(25,-5250.13,522.731,97.3681),
    thisAI->AddWaypoint(26,-5293.25,452.003,87.3439),
    thisAI->AddWaypoint(27,-5289.51,475.05,84.5183),
    thisAI->AddWaypoint(28,-5296.74,459.331,82.4244),
    thisAI->AddWaypoint(29,-5321.86,364.547,78.6856),
    thisAI->AddWaypoint(30,-5336.85,260.91,64.3121),
    thisAI->AddWaypoint(31,-5328.95,229.583,84.3653),
    thisAI->AddWaypoint(32,-5307.89,164.544,90.1567),
    thisAI->AddWaypoint(33,-5274.67,124.229,98.2239),
    thisAI->AddWaypoint(34,-5223.08,87.9087,98.6199),
    thisAI->AddWaypoint(35,-5189.22,52.5833,99.0043),
    thisAI->AddWaypoint(36,-5212.18,68.3701,99.2231),
    thisAI->AddWaypoint(37,-5193.04,51.3872,99.4249),
    thisAI->AddWaypoint(38,-5144.63,8.82392,116.142),
    thisAI->AddWaypoint(39,-5113.09,-13.2902,153.451),
    thisAI->AddWaypoint(40,-5106.63,-19.826,179.961),
    thisAI->AddWaypoint(41,-5081.81,-42.1419,175.739),
    thisAI->AddWaypoint(42,-5055.97,-40.2629,173.062),
    thisAI->AddWaypoint(43,-5031.35,-13.8612,169.328),
    thisAI->AddWaypoint(44,-5061.29,13.0471,165.268),
    thisAI->AddWaypoint(45,-5103.88,54.4085,158.421),
    thisAI->AddWaypoint(46,-5125.73,101.755,148.134),
    thisAI->AddWaypoint(47,-5114.57,126.204,141.401),
    thisAI->AddWaypoint(48,-5139,134.611,155.723),
    thisAI->AddWaypoint(49,-5170.2,145.053,144.259),
    thisAI->AddWaypoint(50,-5194.72,147.041,100.983),
    thisAI->AddWaypoint(51,-5210.56,160,87.769),
    thisAI->AddWaypoint(52,-5207.29,209.95,82.5642),
    thisAI->AddWaypoint(53,-5202.63,252.608,116.804),
    thisAI->AddWaypoint(54,-5186.9,303.438,116.804),
    thisAI->AddWaypoint(55,-5191.42,355.32,112.467),
    thisAI->AddWaypoint(56,-5233.37,395.463,112.177),
    thisAI->AddWaypoint(57,-5286.87,437.22,105.525),
    thisAI->AddWaypoint(58,-5266.34,478.182,110.562),
    thisAI->AddWaypoint(59,-5206.61,503.573,111.606),
    thisAI->AddWaypoint(60,-5163.47,534.104,114.552),
    thisAI->AddWaypoint(61,-5182.36,515.241,116.022),
    thisAI->AddWaypoint(62,-5206.81,518.967,117.426),
    thisAI->AddWaypoint(63,-5187.21,521.63,118.514),
    thisAI->AddWaypoint(64,-5155.39,545.64,116.076),
    thisAI->AddWaypoint(65,-5115.6,583.206,129.026),
    thisAI->AddWaypoint(66,-5073.47,649.348,119.881),
    thisAI->AddWaypoint(67,-5052.38,670.897,116.044),
    thisAI->AddWaypoint(68,-4996.43,662.926,113.267),
    thisAI->AddWaypoint(69,-4961.47,656.298,112.929),
    thisAI->AddWaypoint(70,-4943.27,612.649,112.304),
    thisAI->AddWaypoint(71,-4942.63,576.912,111.831),
    thisAI->AddWaypoint(72,-4962.21,556.808,111.463),
    thisAI->AddWaypoint(73,-4991.94,532.689,123.558),
    thisAI->AddWaypoint(74,-5018.77,500.9,128.931),
    thisAI->AddWaypoint(75,-5046,464.881,154.479),
    thisAI->AddWaypoint(76,-5057.32,433.366,183.934),
    thisAI->AddWaypoint(77,-5062.08,423.791,206.805),
    thisAI->AddWaypoint(78,-5073.28,396.567,212.024),
    thisAI->AddWaypoint(79,-5078.93,361.95,212.344),
    thisAI->AddWaypoint(80,-5083.38,319.898,223.106),
    thisAI->AddWaypoint(81,-5091.54,267.262,219.859),
    thisAI->AddWaypoint(82,-5077.74,245.874,217.788),
    thisAI->AddWaypoint(83,-5024.21,225.572,176.249),
    thisAI->AddWaypoint(84,-4999.13,223.503,164.288),
    thisAI->AddWaypoint(85,-4942.19,274.08,168.949),
    thisAI->AddWaypoint(86,-4953.87,301.817,153.814),
    thisAI->AddWaypoint(87,-4966.83,291.411,163.316),
    thisAI->AddWaypoint(88,-4964.36,265.204,163.522),
    thisAI->AddWaypoint(89,-4938.63,264.626,164.217),
    thisAI->AddWaypoint(90,-4931.72,291.768,150.76),
    thisAI->AddWaypoint(91,-4957.72,300.242,162.393),
    thisAI->AddWaypoint(92,-4970.39,252.106,159.856),
    thisAI->AddWaypoint(93,-4998.35,241.729,158.3),
    thisAI->AddWaypoint(94,-4980.71,264.41,156.807),
    thisAI->AddWaypoint(95,-5010.4,241.114,149.504),
    thisAI->AddWaypoint(96,-5037.11,210.487,150.594),
    thisAI->AddWaypoint(97,-5059.4,188.456,168.62),
    thisAI->AddWaypoint(98,-5092.59,173.388,162.922),
    thisAI->AddWaypoint(99,-5105.72,202.446,164.665),
    thisAI->AddWaypoint(100,-5082.91,206.861,166.916),
    thisAI->AddWaypoint(101,-5079.08,185.103,161.787),
    thisAI->AddWaypoint(102,-5090.4,212.133,167.925),
    thisAI->AddWaypoint(103,-5090.4,212.133,167.925),
    thisAI->AddWaypoint(104,-5118.5,223.472,172.949),
    thisAI->AddWaypoint(105,-5114.53,185.571,172.229),
    thisAI->AddWaypoint(106,-5086.57,177.489,165.926),
    thisAI->AddWaypoint(107,-5104.78,204.471,162.304),
    thisAI->AddWaypoint(108,-5100.53,240.85,172.979),
    thisAI->AddWaypoint(109,-5100.42,211.319,159.459),
    thisAI->AddWaypoint(110,-5099.75,238.598,185.354),
    thisAI->AddWaypoint(111,-5100.56,184.533,166.588),
    thisAI->AddWaypoint(112,-5099.49,223.17,183.528),
    thisAI->AddWaypoint(113,-5098.72,251.22,184.03),
    thisAI->AddWaypoint(114,-5102.87,221.21,170.16),
    thisAI->AddWaypoint(115,-5103.02,176.81,149.762),
    thisAI->AddWaypoint(116,-5125.85,178.913,170.682),
    thisAI->AddWaypoint(117,-5105.31,221.506,177.608),
    thisAI->AddWaypoint(118,-5077.74,257.581,188.335),
    thisAI->AddWaypoint(119,-5054.87,298.941,202.73),
    thisAI->AddWaypoint(120,-5025.86,320.998,195.272),
    thisAI->AddWaypoint(121,-4985.94,354.82,187.078),
    thisAI->AddWaypoint(122,-4965.83,393.094,212.127),
    thisAI->AddWaypoint(123,-4936.18,440.955,223.817),
    thisAI->AddWaypoint(124,-4915.47,486.716,162.626),
    thisAI->AddWaypoint(125,-4907.49,506.496,137.118),
    thisAI->AddWaypoint(126,-4909.6,542.633,125.838),
    thisAI->AddWaypoint(127,-4918.57,610.449,132.913),
    thisAI->AddWaypoint(128,-4929.68,650.715,119.914),
    thisAI->AddWaypoint(129,-4970.79,659.978,103.462),
    thisAI->AddWaypoint(130,-4997.78,664.189,97.2824),
    thisAI->AddWaypoint(131,-4975.31,660.466,102.388),
    thisAI->AddWaypoint(132,-4989.89,651.918,98.5794),
    thisAI->AddWaypoint(133,-5002.25,668.847,93.8366),
    thisAI->AddWaypoint(134,-4985.44,684.705,94.677),
    thisAI->AddWaypoint(135,-4966.06,661.69,98.2438),
    thisAI->AddWaypoint(136,-4989.44,661.463,99.1368),
    thisAI->AddWaypoint(137,-5064.1,665.918,89.5543),
    thisAI->AddWaypoint(138,-5084.8,665.122,88.5924),
    thisAI->AddWaypoint(139,-5099,664.576,87.2931),
    thisAI->AddWaypoint(140,-5097.66,650.922,87.2361),
    thisAI->AddWaypoint(141,-5093.08,642.734,86.9699),
    thisAI->AddWaypoint(142,-5082.29,633.93,86.6494);


    return (CreatureAI*)thisAI;
}

bool QuestAccept_mob_23348(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == 11071)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(mob_23348AI, creature->AI()))
        {
            pEscortAI->Start(false, false, player->GetGUID(), quest, true, false);
            pEscortAI->SetMaxPlayerDistance(100.0f);
            pEscortAI->SetDespawnAtEnd(true);
            pEscortAI->SetDespawnAtFar(true);
        }
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        DoScriptText(-1811051, creature, player);
    }
    return true;
}

/*######
## Quests 11101 && 11097
######*/

#define QUEST_DEADLIEST_TRAP_ALDOR  11101
#define QUEST_DEADLIEST_TRAP_SCYER  11097

#define COMMANDER_HOBB_SAY1         -1000745
#define COMMANDER_HOBB_SAY2         -1000746
#define COMMANDER_HOBB_SAY3         -1000756
#define SKYBREAKER_SAY1             -1000747
#define SKYBREAKER_SAY2             -1000748
#define SKYBREAKER_SAY3             -1000749
#define SKYBREAKER_SAYALDOR1        -1000750
#define SKYBREAKER_SAYALDOR2        -1000751
#define SKYBREAKER_SAYSCYER1        -1000752
#define SKYBREAKER_SAYSCYER2        -1000753
#define COMMANDER_ARCUS_SAY1        -1000754
#define COMMANDER_ARCUS_SAY2        -1000755
#define COMMANDER_ARCUS_SAY3        -1000757

#define SPELL_AIMED_SHOT            38370
#define SPELL_MULTI_SHOT            41448
#define SPELL_SHOOT                 41440

#define NPC_DRAGONMAW_SKYBREAKER_SCYER  23440
#define NPC_DRAGONMAW_SKYBREAKER_ALDOR  23441

#define NPC_DEFENDER_SCYER              23435
#define NPC_DEFENDER_ALDOR              23453

float skybreakerPosAldor[10][3] = {
	{ -3150.351807, 756.830444, 37.261200 },
	{ -3158.962646, 745.880676, 37.261200 },
	{ -3170.083252, 731.739441, 37.261200 },
	{ -3176.036865, 716.404663, 37.261200 },
	{ -3179.811768, 706.681763, 37.261200 },
	{ -3183.206543, 697.937683, 37.261200 },
	{ -3185.165039, 684.075378, 37.261200 },
	{ -3186.469482, 674.786499, 37.261200 },
	{ -3187.608398, 666.676086, 37.261200 },
	{ -3128.346436, 763.912537, 37.261200 } };

float skybreakerPosScyer[10][3] = {
	{ -4080.790527, 970.862976, 79.887848 },
	{ -4067.590820, 974.339661, 79.887848 },
	{ -4053.443359, 978.065979, 79.887848 },
	{ -4041.665039, 981.168274, 79.887848 },
	{ -4027.382080, 984.930298, 79.887848 },
	{ -4016.010010, 987.925659, 79.887848 },
	{ -4007.221436, 1001.788818, 77.243546 },
	{ -3996.254883, 1010.603210, 77.243546 },
	{ -3985.991211, 1031.524292, 74.557518 },
	{ -4114.185059, 975.281982, 74.557518 } };

// Orientation 2.337
float defendersPosAldor[10][3] = {
	{ -3081.971924, 700.679260, -16.377661 },
	{ -3083.283691, 697.540344, -16.844158 },
	{ -3084.743896, 694.046204, -17.461650 },
	{ -3085.826416, 691.456299, -17.954538 },
	{ -3086.954834, 688.756592, -18.279066 },
	{ -3088.004883, 686.244202, -18.166872 },
	{ -3089.394043, 682.920410, -17.503175 },
	{ -3093.030029, 677.387085, -16.527822 },
	{ -3095.304199, 675.197266, -16.083666 },
	{ -3097.482422, 673.099731, -15.411659 } };

// Orientation 5.109
float defendersPosScyer[10][3] = {
	{ -4059.349121, 1075.253418, 31.234081 },
	{ -4062.739014, 1074.006592, 30.966137 },
	{ -4064.821533, 1073.240601, 30.761585 },
	{ -4064.855469, 1068.144043, 30.224163 },
	{ -4067.918457, 1069.042358, 30.359905 },
	{ -4069.360596, 1063.678589, 30.409506 },
	{ -4074.669922, 1065.100098, 30.804943 },
	{ -4079.707275, 1065.286499, 31.153090 },
	{ -4082.981689, 1062.328369, 31.237562 },
	{ -4085.491699, 1058.670410, 30.850157 } };
// -4072.696777, 1071.352295, 30.769413

struct npc_23452AI : public ScriptedAI
{
	npc_23452AI(Creature* c) : ScriptedAI(c), summons(me)
	{
		playerGUID = 0;
		isEvent = false;
	}

	float homeX, homeY, homeZ, homeOri;

	bool isEvent;

	uint64 playerGUID;

	Timer aimedShotTimer;
	Timer multiShotTimer;
	Timer shootTimer;
	Timer summonTimer;
	uint32 killCounter;
	SummonList summons;

	void Reset()
	{
		if (!isEvent)
		{
			aimedShotTimer.Reset(5000);
			multiShotTimer.Reset(10000);
			shootTimer.Reset(1000);
			killCounter = 0;
			summonTimer.Reset(0);
		}
	}

	void EnterCombat(Unit* who) {}

	void JustSummoned(Creature* summon)
	{
		summons.Summon(summon);
	}

	void SummonedCreatureDespawn(Creature* summon)
	{
		summons.Despawn(summon);
		if (summon->GetEntry() == NPC_DRAGONMAW_SKYBREAKER_ALDOR)
			killCounter++;
	}

	void MovementInform(uint32 type, uint32 id)
	{
		if (type != POINT_MOTION_TYPE)
			return;

		if (id == 300)
		{
			DoScriptText(COMMANDER_ARCUS_SAY2, me);
			Player* player = Unit::GetPlayerInWorld(playerGUID);
			summonTimer = 15000;
			for (uint8 i = 0; i < 10; i++)
			{
				if (Creature* skybreaker = me->SummonCreature(NPC_DRAGONMAW_SKYBREAKER_ALDOR, skybreakerPosAldor[i][0], skybreakerPosAldor[i][1], skybreakerPosAldor[i][2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
				{
					skybreaker->SetFlying(true);
					if (player)
						skybreaker->AI()->AttackStart(player);
				}
			}
		}
	}

	void StartEvent()
	{
		isEvent = true;
		DoScriptText(COMMANDER_ARCUS_SAY1, me, NULL);
		me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
		me->GetMotionMaster()->MovePoint(0, -3098.666748, 682.178040, -18.633110, true);

		for (uint8 i = 0; i < 10; i++)
			me->SummonCreature(NPC_DEFENDER_ALDOR, defendersPosAldor[i][0], defendersPosAldor[i][1], defendersPosAldor[i][2], 0, TEMPSUMMON_MANUAL_DESPAWN, 0);

		me->GetHomePosition(homeX, homeY, homeZ, homeOri);
		me->SetHomePosition(-3098.666748, 682.178040, -18.633110, me->GetOrientation());
	}

	void EndEvent(bool success)
	{
		Player* player = Unit::GetPlayerInWorld(playerGUID);
		if (success)
		{
			DoScriptText(COMMANDER_ARCUS_SAY3, me);
			if (player)
				player->GroupEventHappens(QUEST_DEADLIEST_TRAP_ALDOR, me);
		}
		else
		{
			if (player)
				player->FailQuest(QUEST_DEADLIEST_TRAP_ALDOR);
		}

		playerGUID = 0;
		me->SetHomePosition(homeX, homeY, homeZ, homeOri);
		me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
		summons.DespawnAll();
		isEvent = false;
		EnterEvadeMode();
	}

	void UpdateAI(uint32 const diff)
	{
		if (!isEvent)
			return;

		Player* player = Unit::GetPlayerInWorld(playerGUID);
		if (!player || player->GetDistance(me) > 110.0f)
		{
			EndEvent(false);
			return;
		}

		if (killCounter >= 15)
		{
			EndEvent(true);
			return;
		}

		if (summonTimer.Expired(diff))
		{
			if (Creature* skybreaker = me->SummonCreature(NPC_DRAGONMAW_SKYBREAKER_ALDOR, skybreakerPosAldor[rand() % 10][0], skybreakerPosAldor[rand() % 10][1], skybreakerPosAldor[rand() % 10][2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
			{
				skybreaker->SetFlying(true);
				skybreaker->AI()->AttackStart(player);
			}

			summonTimer = 15000;
		}

		if (!UpdateVictim())
			return;

		if (aimedShotTimer.Expired(diff))
		{
			DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_AIMED_SHOT);
			aimedShotTimer = 5000;
		}

		if (multiShotTimer.Expired(diff))
		{
			DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_MULTI_SHOT);
			multiShotTimer = 8000;
		}

		if (shootTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), SPELL_SHOOT);
			shootTimer = 5000;
		}

		if (me->GetVictim()->IsWithinMeleeRange(me))
			DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_23452(Creature* creature)
{
	return new npc_23452AI(creature);
}

bool QuestAccept_npc_23452(Player* player, Creature* creature, Quest const* quest)
{
	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());

	if (quest->GetQuestId() == QUEST_DEADLIEST_TRAP_ALDOR)
	{
		((npc_23452AI*)creature->AI())->playerGUID = player->GetGUID();
		((npc_23452AI*)creature->AI())->StartEvent();
	}

	return true;
}

struct npc_23434AI : public ScriptedAI
{
	npc_23434AI(Creature* c) : ScriptedAI(c), summons(me)
	{
		playerGUID = 0;
		isEvent = false;
	}

	float homeX, homeY, homeZ, homeOri;

	bool isEvent;

	uint64 playerGUID;

	Timer aimedShotTimer;
	Timer multiShotTimer;
	Timer shootTimer;
	Timer summonTimer;
	uint32 killCounter;
	SummonList summons;

	void Reset()
	{
		if (!isEvent)
		{
			aimedShotTimer.Reset(5000);
			multiShotTimer.Reset(10000);
			shootTimer.Reset(1000);
			killCounter = 0;
			summonTimer = 0;
		}
	}

	void EnterCombat(Unit* who) {}

	void JustSummoned(Creature* summon)
	{
		summons.Summon(summon);
	}

	void SummonedCreatureDespawn(Creature* summon)
	{
		summons.Despawn(summon);
		if (summon->GetEntry() == NPC_DRAGONMAW_SKYBREAKER_SCYER)
			killCounter++;
	}

	void MovementInform(uint32 type, uint32 id)
	{
		if (type != POINT_MOTION_TYPE)
			return;

		if (id == 300)
		{
			DoScriptText(COMMANDER_HOBB_SAY2, me);
			Player* player = Unit::GetPlayerInWorld(playerGUID);
			summonTimer = 15000;
			for (uint8 i = 0; i < 10; i++)
			{
				if (Creature* skybreaker = me->SummonCreature(NPC_DRAGONMAW_SKYBREAKER_SCYER, skybreakerPosScyer[i][0], skybreakerPosScyer[i][1], skybreakerPosScyer[i][2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
				{
					skybreaker->SetFlying(true);
					if (player)
						skybreaker->AI()->AttackStart(player);
				}
			}
		}
	}

	void StartEvent()
	{
		isEvent = true;
		DoScriptText(COMMANDER_HOBB_SAY1, me, NULL);
		me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
		me->GetMotionMaster()->MovePoint(300, -4072.696777, 1071.352295, 30.769413, true);

		for (uint8 i = 0; i < 10; i++)
			me->SummonCreature(NPC_DEFENDER_SCYER, defendersPosScyer[i][0], defendersPosScyer[i][1], defendersPosScyer[i][2], 0, TEMPSUMMON_MANUAL_DESPAWN, 0);

		me->GetHomePosition(homeX, homeY, homeZ, homeOri);
		me->SetHomePosition(-4072.696777, 1071.352295, 30.769413, me->GetOrientation());
	}

	void EndEvent(bool success)
	{
		Player* player = Unit::GetPlayerInWorld(playerGUID);
		if (success)
		{
			DoScriptText(COMMANDER_HOBB_SAY3, me);
			if (player)
				player->GroupEventHappens(QUEST_DEADLIEST_TRAP_SCYER, me);
		}
		else
		{
			if (player)
				player->FailQuest(QUEST_DEADLIEST_TRAP_SCYER);
		}

		playerGUID = 0;
		me->SetHomePosition(homeX, homeY, homeZ, homeOri);
		me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
		summons.DespawnAll();
		isEvent = false;
		EnterEvadeMode();
	}

	void UpdateAI(uint32 const diff)
	{
		if (!isEvent)
			return;

		Player* player = Unit::GetPlayerInWorld(playerGUID);
		if (!player || player->GetDistance(me) > 110.0f)
		{
			EndEvent(false);
			return;
		}

		if (killCounter >= 15)
		{
			EndEvent(true);
			return;
		}

		if (summonTimer.Expired(diff))
		{
			if (Creature* skybreaker = me->SummonCreature(NPC_DRAGONMAW_SKYBREAKER_SCYER, skybreakerPosScyer[rand() % 10][0], skybreakerPosScyer[rand() % 10][1], skybreakerPosScyer[rand() % 10][2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000))
			{
				skybreaker->SetFlying(true);
				skybreaker->AI()->AttackStart(player);
			}

			summonTimer = 15000;
		}

		if (!UpdateVictim())
			return;

		if (aimedShotTimer.Expired(diff))
		{
			DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_AIMED_SHOT);
			aimedShotTimer = 5000;
		}

		if (multiShotTimer.Expired(diff))
		{
			DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_MULTI_SHOT);
			multiShotTimer = 8000;
		}

		if (shootTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), SPELL_SHOOT);
			shootTimer = 5000;
		}

		if (me->GetVictim()->IsWithinMeleeRange(me))
			DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_23434(Creature* creature)
{
	return new npc_23434AI(creature);
}

bool QuestAccept_npc_23434(Player* player, Creature* creature, Quest const* quest)
{
	if (creature->isQuestGiver())
		player->PrepareQuestMenu(creature->GetGUID());

	if (quest->GetQuestId() == QUEST_DEADLIEST_TRAP_SCYER) {
		((npc_23434AI*)creature->AI())->playerGUID = player->GetGUID();
		((npc_23434AI*)creature->AI())->StartEvent();
	}

	return true;
}

struct npc_dragonmaw_skybreakerAI : public ScriptedAI
{
	npc_dragonmaw_skybreakerAI(Creature* c) : ScriptedAI(c)
	{
		aldor = (me->GetEntry() == NPC_DRAGONMAW_SKYBREAKER_ALDOR) ? true : false;
		me->SetAggroRange(120.0f);
	}

	bool aldor;

	Timer aimedShotTimer;
	Timer multiShotTimer;
	Timer scatterShotTimer;

	Timer randomTauntTimer;

	void Reset()
	{
		me->addUnitState(UNIT_STAT_IGNORE_PATHFINDING);
		randomTauntTimer.Reset(urand(1000, 20000));
		aimedShotTimer.Reset(urand(12000, 17000));
		multiShotTimer.Reset(urand(10000, 14000));
		scatterShotTimer.Reset(urand(5000, 10000));
	}

	void EnterCombat(Unit* who)
	{
		if (rand() % 10 < 8)
			return;

		DoScriptText(RAND(SKYBREAKER_SAY1, SKYBREAKER_SAY2), me, NULL);
	}

	void UpdateAI(uint32 const diff)
	{
		if (!UpdateVictim())
			return;

		if (randomTauntTimer.Expired(diff))
		{
			if (aldor)
				DoScriptText(RAND(SKYBREAKER_SAYALDOR1, SKYBREAKER_SAYALDOR2, SKYBREAKER_SAY3), me);
			else
				DoScriptText(RAND(SKYBREAKER_SAYSCYER1, SKYBREAKER_SAYSCYER2, SKYBREAKER_SAY3), me);
			randomTauntTimer = 0;
		}
		
		if (aimedShotTimer.Expired(diff))
		{
			DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_AIMED_SHOT);
			aimedShotTimer = urand(8000, 15000);
		}

		if (multiShotTimer.Expired(diff))
		{
			DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_MULTI_SHOT);
			multiShotTimer = urand(9000, 14000);
		}

		if (scatterShotTimer.Expired(diff))
		{
			DoCast(me->GetVictim(), 36732);
			scatterShotTimer = urand(8000, 10000);
		}

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_dragonmaw_skybreaker(Creature* creature)
{
	return new npc_dragonmaw_skybreakerAI(creature);
}

struct npc_21416AI : public ScriptedAI
{
    npc_21416AI(Creature* c) : ScriptedAI(c), Summons(me)   {}

    SummonList Summons;

    Timer WaterBoltTimer;
    Timer SummonTimer;
    uint8 Counter;

    void Reset()
    {
        ClearCastQueue();
        WaterBoltTimer.Reset(1000);
        SummonTimer.Reset(1000);
        Summons.DespawnAll();
        Counter = 0;
    }

    void SummonedCreatureDies(Creature* cSummon, Unit* killer)
    {
        cSummon->InterruptNonMeleeSpells(true);
        cSummon->CastSpell(cSummon, 36826, false);
        cSummon->CastSpell(cSummon, 36826, false);
        cSummon->CastSpell(killer, 15063, false);
        Counter++;
        if (Counter >= 3)
            me->RemoveAllAuras();
    }

    void JustSummoned(Creature* who)
    {
        if (who)
            Summons.Summon(who);
    }

    void JustDied(Unit* who)
    {
        Summons.DespawnAll();
    }

    void SummonTotems()
    {
        if (Creature* totemOne = me->SummonCreature(21420, -2795.69, 1486.3, 7.872, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemOne->CastSpell(me, 36817, false);
        if (Creature* totemTwo = me->SummonCreature(21420, -2791.51, 1476.98, 9.137, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemTwo->CastSpell(me, 38105, false);
        if (Creature* totemThree = me->SummonCreature(21420, -2783.87, 1483.09, 9.206, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemThree->CastSpell(me, 38106, false);
        Counter = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (SummonTimer.Expired(diff))
            {
                if (Summons.empty())
                    SummonTotems();
                SummonTimer = 0;
            }
            return;
        }

        if (WaterBoltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 32011, false);
            WaterBoltTimer = 2000;
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21416(Creature *_creature)
{
    return new npc_21416AI(_creature);
}

struct npc_21710AI : public ScriptedAI
{
    npc_21710AI(Creature* c) : ScriptedAI(c), Summons(me)   {}

    SummonList Summons;

    Timer FireShockTimer;
    Timer SummonTimer;
    uint8 Counter;

    void Reset()
    {
        ClearCastQueue();
        FireShockTimer.Reset(1000);
        SummonTimer.Reset(1000);
        Summons.DespawnAll();
        Counter = 0;
    }

    void SummonedCreatureDies(Creature* cSummon, Unit* killer)
    {
        cSummon->InterruptNonMeleeSpells(true);
        cSummon->CastSpell(cSummon, 37201, false);
        cSummon->CastSpell(cSummon, 37201, false);
        cSummon->CastSpell(killer, 11969, false);
        Counter++;
        if (Counter >= 3)
            me->RemoveAllAuras();
    }

    void JustSummoned(Creature* who)
    {
        if (who)
            Summons.Summon(who);
    }

    void JustDied(Unit* who)
    {
        Summons.DespawnAll();
    }

    void SummonTotems()
    {
        if (Creature* totemOne = me->SummonCreature(21703, -3404, 1565, 47.55, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemOne->CastSpell(me, 37206, false);
        if (Creature* totemTwo = me->SummonCreature(21703, -3398, 1576.55, 47, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemTwo->CastSpell(me, 38103, false);
        if (Creature* totemThree = me->SummonCreature(21703, -3391.13, 1567.88, 47.98, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemThree->CastSpell(me, 38104, false);
        Counter = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (SummonTimer.Expired(diff))
            {
                if (Summons.empty())
                    SummonTotems();
                SummonTimer = 0;
            }
            return;
        }

        if (FireShockTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 15039, false);
            FireShockTimer = 5000;
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21710(Creature *_creature)
{
    return new npc_21710AI(_creature);
}

struct npc_21709AI : public ScriptedAI
{
    npc_21709AI(Creature* c) : ScriptedAI(c), Summons(me) {}

    SummonList Summons;

    Timer EarthShock;
    Timer SummonTimer;
    uint8 Counter;

    void Reset()
    {
        ClearCastQueue();
        EarthShock.Reset(1000);
        SummonTimer.Reset(1000);
        Summons.DespawnAll();
        Counter = 0;
    }

    void SummonedCreatureDies(Creature* cSummon, Unit* killer)
    {
        cSummon->InterruptNonMeleeSpells(true);
        cSummon->CastSpell(cSummon, 37203, false);
        cSummon->CastSpell(cSummon, 37203, false);
        cSummon->CastSpell(cSummon, 37203, false);
        Counter++;
        if (Counter >= 3)
            me->RemoveAllAuras();
    }

    void JustSummoned(Creature* who)
    {
        if (who)
            Summons.Summon(who);
    }

    void JustDied(Unit* who)
    {
        Summons.DespawnAll();
    }

    void SummonTotems()
    {
        if (Creature* totemOne = me->SummonCreature(21704, -3883.99, 1404.3, 43.63, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemOne->CastSpell(me, 37204, false);
        if (Creature* totemTwo = me->SummonCreature(21704, -3879.86, 1416.28, 45.317, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemTwo->CastSpell(me, 38101, false);
        if (Creature* totemThree = me->SummonCreature(21704, -3893, 1411.49, 44.157, 0, TEMPSUMMON_DEAD_DESPAWN, true))
            totemThree->CastSpell(me, 38102, false);
        Counter = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (SummonTimer.Expired(diff))
            {
                if (Summons.empty())
                    SummonTotems();
                SummonTimer = 0;
            }
            return;
        }

        if (EarthShock.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 13281, false);
            EarthShock = 7000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21709(Creature *_creature)
{
    return new npc_21709AI(_creature);
}

struct npc_21711AI : public ScriptedAI
{
    npc_21711AI(Creature* c) : ScriptedAI(c), Summons(me) {}

    SummonList Summons;

    Timer ChainLightningTimer;
    Timer LightningBoltTimer;
    Timer SummonTimer;
    uint8 Counter;

    void Reset()
    {
        ClearCastQueue();
        ChainLightningTimer.Reset(1000);
        LightningBoltTimer.Reset(2000);
        SummonTimer.Reset(1000);
        Summons.DespawnAll();
        Counter = 0;
    }

    void SummonedCreatureDies(Creature* cSummon, Unit* killer)
    {
        cSummon->InterruptNonMeleeSpells(true);
        cSummon->CastSpell(cSummon, 37202, false);
        cSummon->CastSpell(cSummon, 37202, false);
        cSummon->CastSpell(cSummon, 37202, false);
        Counter++;
        if (Counter >= 3)
            me->RemoveAllAuras();
    }

    void JustSummoned(Creature* who)
    {
        if (who)
            Summons.Summon(who);
    }

    void JustDied(Unit* who)
    {
        Summons.DespawnAll();
    }

    void SummonTotems()
    {
        if (Creature* totemOne = me->SummonCreature(21705, -4646.82, 1091.07, 0.146, 3.49, TEMPSUMMON_DEAD_DESPAWN, true))
            totemOne->CastSpell(me, 37205, false);
        if (Creature* totemTwo = me->SummonCreature(21705, -4654.15, 1084.94, 1.098, 6.02, TEMPSUMMON_DEAD_DESPAWN, true))
            totemTwo->CastSpell(me, 38099, false);
        if (Creature* totemThree = me->SummonCreature(21705, -4644.95, 1081.28, 1.13, 3.682, TEMPSUMMON_DEAD_DESPAWN, true))
            totemThree->CastSpell(me, 38100, false);
        Counter = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (SummonTimer.Expired(diff))
            {
                if (Summons.empty())
                    SummonTotems();
                SummonTimer = 0;
            }
            return;
        }

        if (ChainLightningTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 12058, false);
            ChainLightningTimer = 7000;
        }

        if (LightningBoltTimer.Expired(diff))
        {
            AddSpellToCast(me->GetVictim(), 9532, false);
            LightningBoltTimer = 2000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21711(Creature *_creature)
{
    return new npc_21711AI(_creature);
}


struct npc_21500AI : public ScriptedAI
{
    npc_21500AI(Creature* c) : ScriptedAI(c) {}


    Timer MortalCleaveTimer;
    Timer RoFTimer;
    Timer WarStompTimer;
    bool WarStomp;

    void Reset()
    {
        MortalCleaveTimer.Reset(7000);
        RoFTimer.Reset(10000);
        WarStompTimer.Reset(0);
        WarStomp = false;
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == WAYPOINT_MOTION_TYPE)
        {
            if (id == 1 || id == 4 || id == 6 || id == 8)
                DoScriptText(RAND(-1800533, -1800534, -1800535, -1800536, -1800537, -1800538), me);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (MortalCleaveTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 22859, false);
            MortalCleaveTimer = 6000;
        }

        if (RoFTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 38741, false);
            RoFTimer = 15000;
        }

        if (WarStompTimer.Expired(diff))
        {
            WarStompTimer = 0;
            WarStomp = false;
        }

        if (!WarStomp)
        {
            if (me->GetHealthPercent() <= 50)
            {
                WarStomp = true;
                me->CastSpell(me->GetVictim(), 38750, false);
                WarStompTimer = 20000;
            }
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21500(Creature *_creature)
{
    return new npc_21500AI(_creature);
}

/*#####
# npc_shadowmoon_slayer
#####*/

#define SPELL_DEBILITING_STRIKE     37577
#define SPELL_FRENZY                3019

struct npc_shadowmoon_slayerAI : public ScriptedAI
{
    npc_shadowmoon_slayerAI(Creature *c) : ScriptedAI(c) { }

    Timer DebilitingStrikeTimer;
    Timer EmoteTimer;
    Timer FightTimer;

    uint8 EmotePhase;
    bool fighting;
    bool isFrenzy;

    void Reset() 
    {
        DebilitingStrikeTimer.Reset(urand(3000, 6000));
        FightTimer.Reset(3000);
        EmoteTimer.Reset(0);

        EmotePhase = 0;
        fighting = false;
        isFrenzy = false;

        if (m_creature->HasAura(SPELL_FRENZY, 0))
            m_creature->RemoveAurasDueToSpell(SPELL_FRENZY);
    }

    void DamageTaken(Unit* doneby, uint32 & damage)
    {
        fighting = true;

        if(doneby->GetTypeId() == TYPEID_PLAYER)
        {
            if (Player *player = doneby->ToPlayer())
            {
                if (player->isGameMaster())
                    return;
            }

            std::list<Creature*> gladiators = FindAllCreaturesWithEntry(22082, 5);

            if (!gladiators.empty())
            {
                for (std::list<Creature*>::iterator it = gladiators.begin(); it != gladiators.end(); it++)
                    (*it)->AI()->AttackStart(doneby);
                
                FightTimer = 0;
                EmoteTimer = 0;
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        std::list<Creature*> gladiators = FindAllCreaturesWithEntry(22082, 5);
        
        if (gladiators.empty())
            return;

        for (std::list<Creature*>::iterator it = gladiators.begin(); it != gladiators.end(); it++)
            (*it)->AI()->AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (FightTimer.Expired(diff))
        {  
            std::list<Creature*> gladiators = FindAllCreaturesWithEntry(22082, 8);

            if (gladiators.empty())
                return;

            if (m_creature->GetHealthPercent() <= 50 && fighting)
            {
                for (std::list<Creature*>::iterator it = gladiators.begin(); it != gladiators.end(); it++)
                {
                    (*it)->CombatStop();
                    
                    if ((*it)->HasAura(SPELL_FRENZY, 0))
                        (*it)->RemoveAurasDueToSpell(SPELL_FRENZY);
                }
                
                fighting = false;
                EmoteTimer = 2000;
                FightTimer = 7000;

                return;
            }

            for (std::list<Creature*>::iterator it = gladiators.begin(); it != gladiators.end(); it++)
            {
                if ((*it)->GetHealthPercent() <= 50)
                {
                    fighting = false;
                    FightTimer = 7000;

                    return;
                }
            }
            
            if (m_creature->GetHealthPercent() == 100 && !fighting)
            {
                for (std::list<Creature*>::iterator it = gladiators.begin(); it != gladiators.end(); it++)
                {
                    if ((*it)->GetHealthPercent() < 100)
                        return;

                    (*it)->AI()->AttackStart(m_creature);
                }
            }

            FightTimer = 3000;
        }

        if (EmoteTimer.Expired(diff))
        {
            if (!fighting)
            {
                switch(EmotePhase)
                {
                    case 0:
                        if (Creature *n_creature = (Creature*)FindCreature(22082, 8, m_creature))
                            n_creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                        
                        EmoteTimer = 2000;
                        break;
                    case 1:
                        if (FindCreature(22082, 8, m_creature))
                            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_TALK);

                        EmotePhase = 0;
                        EmoteTimer = 0;
                        return;
                }

                EmotePhase++;
            }
        }

        if (!m_creature->HasAura(SPELL_FRENZY, 0))
            isFrenzy = false;

        if (!UpdateVictim())
            return;
        
        if (DebilitingStrikeTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_DEBILITING_STRIKE, false);
            DebilitingStrikeTimer = urand(12000, 15000);
        }

        if (m_creature->GetHealthPercent() < 50 && !isFrenzy)
        {
            m_creature->CastSpell(m_creature, SPELL_FRENZY, false);
            isFrenzy = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_shadowmoon_slayer(Creature *_Creature)
{
    return new npc_shadowmoon_slayerAI (_Creature);
}

/*#####
# npc_demon_hunter_initiate
#####*/

#define SPELL_IMMOLATION    33651
#define SPELL_SPELLBREAKER  35871

struct npc_demon_hunter_initiateAI : public ScriptedAI
{
    npc_demon_hunter_initiateAI(Creature *c) : ScriptedAI(c) { }

    Timer FightTimer;
    Timer EmoteTimer;
    Timer ImmolationTimer;
    Timer SpellbreakerTimer;

    bool fighting;
    bool isPlayerCombat;

    void Reset() 
    {
        FightTimer.Reset(3000);
        EmoteTimer.Reset(0);
        ImmolationTimer.Reset(urand(1, 5000));
        SpellbreakerTimer.Reset(urand(3000, 5000));

        fighting = false;
        isPlayerCombat = false;
    }

    void DamageTaken(Unit* doneby, uint32 & damage)
    {
        if(doneby->GetTypeId() == TYPEID_PLAYER)
        {
            if (Player *player = doneby->ToPlayer())
            {
                if (player->isGameMaster())
                    return;
            }

            if (Creature *terror = (Creature*)FindCreature(21908, 5, m_creature))
            {
                terror->AI()->AttackStart(doneby);
                m_creature->AI()->AttackStart(doneby);
                
                isPlayerCombat = true;

                FightTimer = 0;
                EmoteTimer = 0;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (FightTimer.Expired(diff))
        {  
            if (Creature *terror = (Creature*)FindCreature(21908, 8, m_creature))
            {
                if (m_creature->GetHealthPercent() <= 50 && fighting)
                {
                    terror->CombatStop();
                    terror->GetMotionMaster()->MoveTargetedHome();
                    m_creature->CombatStop();
                    m_creature->GetMotionMaster()->MoveTargetedHome();

                    
                    fighting = false;
                    EmoteTimer = 10000;
                    FightTimer = 13000;
                    return;
                }

                if (terror->GetHealthPercent() <= 50 && !isPlayerCombat)
                {
                    terror->CombatStop();
                    terror->GetMotionMaster()->MoveTargetedHome();
                    m_creature->CombatStop();
                    m_creature->GetMotionMaster()->MoveTargetedHome();

                    fighting = false;
                    FightTimer = 13000;

                    return;
                }

                if (m_creature->GetHealthPercent() == 100 && !fighting)
                {
                    terror->AI()->AttackStart(m_creature);
                    m_creature->AI()->AttackStart(terror);

                    fighting = true;
                }

                FightTimer = 3000;
            }
        }

        if (EmoteTimer.Expired(diff))
        {
            if (!fighting)
            {
                if (Creature *terror = (Creature*)FindCreature(21908, 8, m_creature))
                    m_creature->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                    
                EmoteTimer = 0;
            }
        }

        if (!UpdateVictim())
            return;
        
        if (ImmolationTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_IMMOLATION, false);
            ImmolationTimer = 3000;
        }

        if (SpellbreakerTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_SPELLBREAKER, false);
            SpellbreakerTimer = urand(12000, 15000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_demon_hunter_initiate(Creature *_Creature)
{
    return new npc_demon_hunter_initiateAI (_Creature);
}

/*#####
# npc_demon_hunter_supplicant
#####*/

#define SPELL_EVASION   37683

struct npc_demon_hunter_supplicantAI : public ScriptedAI
{
    npc_demon_hunter_supplicantAI(Creature *c) : ScriptedAI(c) { }

    Timer FightTimer;
    Timer EmoteTimer;

    uint8 EmotePhase;
    uint8 Counter;
    bool fighting;
    bool isEvasion;

    void Reset() 
    {
        FightTimer.Reset(3000);
        EmoteTimer.Reset(0);

        EmotePhase = 0;
        fighting = false;
        isEvasion = false;
    }

    void DamageTaken(Unit* doneby, uint32 & damage)
    {
        if(doneby->GetTypeId() == TYPEID_PLAYER)
        {
            if (Player *player = doneby->ToPlayer())
            {
                if (player->isGameMaster())
                    return;
            }

            std::list<Creature*> hunters = FindAllCreaturesWithEntry(21179, 5);
            if (!hunters.empty())
            {
                for (std::list<Creature*>::iterator it = hunters.begin(); it != hunters.end(); it++)
                    (*it)->AI()->AttackStart(doneby);
                
                FightTimer = 0;
                EmoteTimer = 0;
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        fighting = true;

        std::list<Creature*> hunters = FindAllCreaturesWithEntry(21179, 5);
        if (hunters.empty())
            return;

        for (std::list<Creature*>::iterator it = hunters.begin(); it != hunters.end(); it++)
            (*it)->AI()->AttackStart(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if (FightTimer.Expired(diff))
        {  
            std::list<Creature*> hunters = FindAllCreaturesWithEntry(21179, 5);

            if (hunters.empty())
                return;

            if (m_creature->GetHealthPercent() <= 50 && fighting)
            {
                for (std::list<Creature*>::iterator it = hunters.begin(); it != hunters.end(); it++)
                    (*it)->CombatStop();
                
                fighting = false;
                EmoteTimer = 2000;
                FightTimer = 13000;

                return;
            }

            for (std::list<Creature*>::iterator it = hunters.begin(); it != hunters.end(); it++)
            {
                if ((*it)->GetHealthPercent() <= 50)
                {
                    fighting = false;
                    FightTimer = 13000;

                    return;
                }
            }
            
            if (m_creature->GetHealthPercent() == 100 && !fighting)
            {
                for (std::list<Creature*>::iterator it = hunters.begin(); it != hunters.end(); it++)
                {
                    if ((*it)->GetHealthPercent() < 100)
                        return;

                    (*it)->AI()->AttackStart(m_creature);
                }
            }

            FightTimer = 3000;
        }

        if (EmoteTimer.Expired(diff))
        {
            if (!fighting)
            {
                std::list<Creature*> hunters = FindAllCreaturesWithEntry(21179, 5);
                if (hunters.empty())
                    return;

                switch(EmotePhase)
                {
                    case 0:
                        for (std::list<Creature*>::iterator it = hunters.begin(); it != hunters.end(); it++)
                            (*it)->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                        
                        EmoteTimer = 2000;
                        break;
                    case 1:
                        if (urand(0, 2) == 0)   
                            m_creature->SetStandState(UNIT_STAND_STATE_SIT);

                        EmoteTimer = 5000;
                        break;
                    case 2:
                        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

                        EmoteTimer = 3000;
                        break;
                    case 3:
                        for (std::list<Creature*>::iterator it = hunters.begin(); it != hunters.end(); it++)
                            (*it)->HandleEmoteCommand(EMOTE_ONESHOT_BOW);

                        EmotePhase = 0;
                        EmoteTimer = 0;
                        return;
                }

                EmotePhase++;
            }
        }

        if (!UpdateVictim())
            return;
        
        if (m_creature->GetHealthPercent() <= 50 && !isEvasion)
        {
            m_creature->CastSpell(m_creature, SPELL_EVASION, false);
            isEvasion = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_demon_hunter_supplicant(Creature *_Creature)
{
    return new  npc_demon_hunter_supplicantAI (_Creature);
}

/*#####
# npc_sunfury_summoner
#####*/

#define SPELL_ARCANE_BOLT   13901
#define SPELL_BLUE_BEAM     38034

#define ENTRY_TERRORGUARD   21908

struct npc_sunfury_summonerAI : public ScriptedAI
{
    npc_sunfury_summonerAI(Creature *c) : ScriptedAI(c), Summons(c) { }

    Timer ArcaneBoltTimer;
    Timer CheckSummonTimer;

    SummonList Summons;

    void Reset() 
    {
        ArcaneBoltTimer.Reset(1);
        CheckSummonTimer.Reset(1000);

        ClearCastQueue();
    }

    void EnterCombat(Unit *who)
    {
        m_creature->InterruptNonMeleeSpells(true);

        for (SummonList::const_iterator i = Summons.begin(); i != Summons.end(); ++i)
            if (Creature *terrorguard = m_creature->GetCreature(*i))
                terrorguard->AI()->AttackStart(who);
    }

    void JustDied(Unit* Killer)
    {
        Summons.DespawnAll();
    }   

    void JustSummoned(Creature *summoned)
    {
        Summons.Summon(summoned);

        if (!m_creature->IsInCombat() && summoned->GetEntry() == ENTRY_TERRORGUARD)
        {
            if (Creature *terrorguard = (Creature*)FindCreature(ENTRY_TERRORGUARD, 15, m_creature))
            {
                m_creature->InterruptNonMeleeSpells(true);
                m_creature->CastSpell(terrorguard, SPELL_BLUE_BEAM, false);
            }
        }    
    }

    void JustReachedHome()
    {
        Summons.DespawnAll();
        SummonSpellboundTerrorguard();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (CheckSummonTimer.Expired(diff))
            {
                if (Summons.empty())
                    SummonSpellboundTerrorguard();

                for (SummonList::const_iterator i = Summons.begin(); i != Summons.end(); ++i)
                {
                    if (!m_creature->GetCreature(*i))
                    {
                        Summons.DespawnAll();
                        SummonSpellboundTerrorguard();
                        CheckSummonTimer = 10000;
                        return;
                    }
                }

                CheckSummonTimer = 10000;
            }
            return;
        }
        
        if (ArcaneBoltTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_ARCANE_BOLT);
            ArcaneBoltTimer = 3000;
        }
        
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

    void SummonSpellboundTerrorguard()
    {
        float posX, posY, posZ, angle;
        m_creature->GetHomePosition(posX, posY, posZ, angle);
        
        if (posX < -3800)
            return;
        else if (posX < -3730)
            m_creature->SummonCreature(ENTRY_TERRORGUARD, posX + 15, posY, posZ, angle, TEMPSUMMON_CORPSE_DESPAWN, 0);

        else
            m_creature->SummonCreature(ENTRY_TERRORGUARD, posX - 15, posY, posZ, angle, TEMPSUMMON_CORPSE_DESPAWN, 0);
    }
};

CreatureAI* GetAI_npc_sunfury_summoner(Creature *_Creature)
{
    return new  npc_sunfury_summonerAI (_Creature);
}


/*#####
# npc_spellbound_terrorguard
#####*/

#define SPELL_FEL_FLAMES    37488 
#define SPELL_HAMSTING      31553

struct npc_spellbound_terrorguardAI : public ScriptedAI
{
    npc_spellbound_terrorguardAI(Creature *c) : ScriptedAI(c) { }

    Timer FelFlamesTimer;
    Timer HamstingTimer;

    void Reset() 
    {
        FelFlamesTimer.Reset(urand(5000, 10000));
        HamstingTimer.Reset(urand(2000, 4000));
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        if (FelFlamesTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_FEL_FLAMES, false);
            FelFlamesTimer = urand(15000, 20000);
        }

        if (HamstingTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_HAMSTING, false);
            HamstingTimer = urand(9000, 17000);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_spellbound_terrorguard(Creature *_Creature)
{
    return new  npc_spellbound_terrorguardAI (_Creature);
}

struct npc_21656AI : public ScriptedAI
{
    npc_21656AI(Creature *c) : ScriptedAI(c) { }

    bool CurseOfPain;
    Timer ResetChannelTimer;

    void Reset()
    {
        ResetChannelTimer.Reset(0);
        CurseOfPain = false;
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 misc)
    {
        if (me->IsInCombat())
            return;
        if (me->GetDBTableGUIDLow() == 75652)
        {
            if (eventType == 5)
            {
                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(100, -4114.21, 2534.95, 141.24);
            }
        }
        else if (me->GetDBTableGUIDLow() == 75648)
        {
            if (eventType == 5)
            {
                me->SetWalk(true);
                me->GetMotionMaster()->MovePoint(100, -4115.9, 2528.13, 140.74);
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        me->InterruptNonMeleeSpells(true);
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE)
        {
            if (id == 100)
            {
                me->CastSpell(me, 32783, false);
                ResetChannelTimer = 5000;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (ResetChannelTimer.Expired(diff))
            {
                me->InterruptNonMeleeSpells(true);
                ResetChannelTimer = 0;
                EnterEvadeMode();
            }
            return;
        }
        
        if (!CurseOfPain)
        {
            if (me->GetHealthPercent() < 50)
                me->CastSpell(me->GetVictim(), 38048, false);
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21656(Creature *_Creature)
{
    return new npc_21656AI(_Creature);
}


struct npc_19394AI : public ScriptedAI
{
    npc_19394AI(Creature *c) : ScriptedAI(c) { }

    Timer EventTimer;
    uint32 Phase;

    void Reset()
    {
        EventTimer.Reset(10000);
        Phase = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EventTimer.Expired(diff))
            {
                Unit* bron = FindCreature(19395, 20, me);
                if (!bron)
                    return;
                switch (Phase)
                {
                case 0:
                    DoScriptText(-1901050, me);
                    Phase++;
                    EventTimer = 2000;
                    break;
                case 1:
                    if (bron)
                        DoScriptText(-1901051, bron, 0);
                    Phase++;
                    EventTimer = 2000;
                    break;
                case 2:
                    if (bron)
                        DoScriptText(-1901052, bron, 0);
                    Phase++;
                    EventTimer = 3000;
                    break;
                case 3:
                    DoScriptText(-1901053, me);
                    Phase++;
                    EventTimer = 5000;
                    break;
                case 4:
                    if (bron)
                        DoScriptText(-1901054, bron, 0);
                    Phase++;
                    EventTimer = 5000;
                    break;
                case 5:
                    DoScriptText(-1901055, me);
                    Phase++;
                    EventTimer = 3000;
                    break;
                case 6:
                    if (bron)
                        ((Creature*)bron)->CastSpell(me, 33822, false);
                    Phase++;
                    EventTimer = 3000;
                    break;
                case 7:
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    Phase++;
                    EventTimer = 7000;
                    break;
                case 8:
                    if (bron)
                        DoScriptText(-1901056, bron, 0);
                    Phase++;
                    EventTimer = 3000;
                    break;
                case 9: // wake up
                    DoScriptText(-1901057, me);
                    Phase = 0;
                    EventTimer = 240000;
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    break;

                }
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19394(Creature *_Creature)
{
    return new npc_19394AI(_Creature);
}


struct npc_21417AI : public ScriptedAI
{
    npc_21417AI(Creature *c) : ScriptedAI(c), Summons(me) { }

    SummonList Summons;
    Timer EventTimer;
    Timer SummonTimer;
    Timer EvadeTimer;

    void Reset()
    {
        Summons.DespawnAll();
        EventTimer.Reset(urand(1000, 5000));
        SummonTimer.Reset(0);
        EvadeTimer.Reset(240000);
    }

    void JustSummoned(Creature* pSummoned)
    {
        Summons.Summon(pSummoned);
        if (Creature* targetA = GetClosestCreatureWithEntry(me, 21736, 25))
            pSummoned->AI()->AttackStart(targetA);
        if (Creature* targetH = GetClosestCreatureWithEntry(me, 21749, 25))
            pSummoned->AI()->AttackStart(targetH);
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* pKiller)
    {
        EventTimer.Reset(3000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EventTimer.Expired(diff))
            {
                if (Creature* trigger = GetClosestCreatureWithEntry(me, 21348, 25))
                    me->CastSpell(trigger, 32148, false);
                SummonTimer = 3000;
                EventTimer = 0;
            }

            if (SummonTimer.Expired(diff))
            {
                if (Creature* target = GetClosestCreatureWithEntry(me, 21348, 25))
                    me->SummonCreature(21419, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 15000);
                SummonTimer = 0;
            }

            if (EvadeTimer.Expired(diff))
            {
                EnterEvadeMode();
                EvadeTimer = 0;
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21417(Creature *_Creature)
{
    return new npc_21417AI(_Creature);
}

struct npc_21419AI : public ScriptedAI
{
    npc_21419AI(Creature *c) : ScriptedAI(c) { }

    void Reset()
    {
        m_creature->SetPlayerDamageReqInPct(0.2);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21419(Creature *_Creature)
{
    return new npc_21419AI(_Creature);
}


struct npc_19382AI : public ScriptedAI
{
    npc_19382AI(Creature *c) : ScriptedAI(c) { }

    Timer HammerTimer;

    void Reset()
    {
        HammerTimer.Reset(urand(3000, 5000));
    }

    void UpdateAI(const uint32 diff)
    {
        if (HammerTimer.Expired(diff))
        {
            if (Unit* infernal = FindCreature(21419, 100, me))
                me->CastSpell(infernal, 38093, false);
            HammerTimer = 5000;
        }
        if (!UpdateVictim())
        {
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19382(Creature *_Creature)
{
    return new npc_19382AI(_Creature);
}

struct npc_19629_19631AI : public ScriptedAI
{
    npc_19629_19631AI(Creature *c) : ScriptedAI(c) { }

    Timer EmoteTimer;

    void Reset()
    {
        EmoteTimer.Reset(3000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EmoteTimer.Expired(diff))
            {
                switch (urand(0, 5))
                {
                case 0:
                    me->HandleEmote(1);
                    EmoteTimer = 5000;
                    break;
                case 1:
                    me->HandleEmote(5);
                    EmoteTimer = 5000;
                    break;
                case 2:
                    me->HandleEmote(6);
                    EmoteTimer = 5000;
                    break;
                case 3:
                    me->HandleEmote(11);
                    EmoteTimer = 5000;
                    break;
                case 4:
                    me->HandleEmote(274);
                    EmoteTimer = 5000;
                    break;
                case 5:
                    me->HandleEmote(273);
                    EmoteTimer = 5000;
                    break;
                }
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_19629_19631(Creature *_Creature)
{
    return new npc_19629_19631AI(_Creature);
}

struct npc_21860_21822AI : public ScriptedAI
{
    npc_21860_21822AI(Creature *c) : ScriptedAI(c) { }

    Timer SayTimer;
    uint8 Phase;

    void Reset()
    {
        SayTimer.Reset(10000);
        Phase = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (SayTimer.Expired(diff))
            {
                switch (Phase)
                {
                case 0:
                    DoScriptText(-1901058, me);
                    SayTimer = 10000;
                    Phase++;
                    break;
                case 1:
                    if (Unit* Alu = FindCreature(21822, 10, me))
                        DoScriptText(-1901059, Alu, 0);
                    SayTimer = 180000;
                    Phase = 0;
                    break;
                }
            }
            return;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_21860_21822(Creature *_Creature)
{
    return new npc_21860_21822AI(_Creature);
}
void AddSC_shadowmoon_valley()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_21860";
    newscript->GetAI = &GetAI_npc_21860_21822;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19629_19631";
    newscript->GetAI = &GetAI_npc_19629_19631;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19382";
    newscript->GetAI = &GetAI_npc_19382;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21419";
    newscript->GetAI = &GetAI_npc_21419;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21417";
    newscript->GetAI = &GetAI_npc_21417;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19394";
    newscript->GetAI = &GetAI_npc_19394;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21656";
    newscript->GetAI = &GetAI_npc_21656;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_spellbound_terrorguard";
    newscript->GetAI = &GetAI_npc_spellbound_terrorguard;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_sunfury_summoner";
    newscript->GetAI = &GetAI_npc_sunfury_summoner;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_demon_hunter_supplicant";
    newscript->GetAI = &GetAI_npc_demon_hunter_supplicant;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_demon_hunter_initiate";
    newscript->GetAI = &GetAI_npc_demon_hunter_initiate;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_shadowmoon_slayer";
    newscript->GetAI = &GetAI_npc_shadowmoon_slayer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21500";
    newscript->GetAI = &GetAI_npc_21500;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21711";
    newscript->GetAI = &GetAI_npc_21711;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21709";
    newscript->GetAI = &GetAI_npc_21709;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21710";
    newscript->GetAI = &GetAI_npc_21710;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21416";
    newscript->GetAI = &GetAI_npc_21416;
    newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_23452";
	newscript->GetAI = &GetAI_npc_23452;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_23452;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_23434";
	newscript->GetAI = &GetAI_npc_23434;
	newscript->pQuestAcceptNPC = &QuestAccept_npc_23434;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "npc_dragonmaw_skybreaker";
	newscript->GetAI = &GetAI_npc_dragonmaw_skybreaker;
	newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23348";
    newscript->GetAI = &GetAI_mob_23348;
    newscript->pQuestAcceptNPC = &QuestAccept_mob_23348;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23346";
    newscript->GetAI = &GetAI_mob_23346;
    newscript->pQuestAcceptNPC = &QuestAccept_mob_23346;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23345";
    newscript->GetAI = &GetAI_mob_23345;
    newscript->pQuestAcceptNPC = &QuestAccept_mob_23345;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23344";
    newscript->GetAI = &GetAI_mob_23344;
    newscript->pQuestAcceptNPC = &QuestAccept_mob_23344;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23342";
    newscript->GetAI = &GetAI_mob_23342;
    newscript->pQuestAcceptNPC = &QuestAccept_mob_23342;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23340";
    newscript->GetAI = &GetAI_mob_23340;
    newscript->pQuestAcceptNPC = &QuestAccept_mob_23340;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_23311";
    newscript->GetAI = &GetAI_npc_23311;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_azaloth";
    newscript->GetAI = &GetAI_mob_azaloth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21503";
    newscript->GetAI = &GetAI_npc_21503;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_mature_netherwing_drake";
    newscript->GetAI = &GetAI_mob_mature_netherwing_drake;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_enslaved_netherwing_drake";
    newscript->GetAI = &GetAI_mob_enslaved_netherwing_drake;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_dragonmaw_peon";
    newscript->GetAI = &GetAI_mob_dragonmaw_peon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_drake_dealer_hurlunk";
    newscript->pGossipHello =  &GossipHello_npc_drake_dealer_hurlunk;
    newscript->pGossipSelect = &GossipSelect_npc_drake_dealer_hurlunk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npcs_flanis_swiftwing_and_kagrosh";
    newscript->pGossipHello =  &GossipHello_npcs_flanis_swiftwing_and_kagrosh;
    newscript->pGossipSelect = &GossipSelect_npcs_flanis_swiftwing_and_kagrosh;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_grand_commander_ruusk";
    newscript->pGossipHello =  &GossipHello_npc_grand_commander_ruusk;
    newscript->pGossipSelect = &GossipSelect_npc_grand_commander_ruusk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_murkblood_overseer";
    newscript->pGossipHello =  &GossipHello_npc_murkblood_overseer;
    newscript->pGossipSelect = &GossipSelect_npc_murkblood_overseer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_neltharaku";
    newscript->pGossipHello =  &GossipHello_npc_neltharaku;
    newscript->pGossipSelect = &GossipSelect_npc_neltharaku;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_karynaku";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_karynaku;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_oronok_tornheart";
    newscript->pGossipHello =  &GossipHello_npc_oronok_tornheart;
    newscript->pGossipSelect = &GossipSelect_npc_oronok_tornheart;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_overlord_morghor";
    newscript->GetAI = &GetAI_npc_overlord_morghorAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_overlord_morghor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_earthmender_wilda";
    newscript->GetAI = &GetAI_npc_earthmender_wildaAI;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_earthmender_wilda;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_lord_illidan_stormrage";
    newscript->GetAI = &GetAI_npc_lord_illidan_stormrage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_crystal_prison";
    newscript->pQuestAcceptGO = &GOQuestAccept_GO_crystal_prison;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_illidari_spawn";
    newscript->GetAI = &GetAI_mob_illidari_spawn;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_torloth_the_magnificent";
    newscript->GetAI = &GetAI_mob_torloth_the_magnificent;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_enraged_spirit";
    newscript->GetAI = &GetAI_npc_enraged_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_Akama";
    newscript->GetAI = &GetAI_npc_Akama;
    newscript->pQuestRewardedNPC = &ChooseReward_npc_Akama;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_shadowlord_trigger";
    newscript->GetAI = &GetAI_npc_shadowlord_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowmoon_soulstealer";
    newscript->GetAI = &GetAI_mob_shadowmoon_soulstealer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_shadowlord_deathwail";
    newscript->GetAI = &GetAI_mob_shadowlord_deathwail;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="felfire_summoner";
    newscript->GetAI = &GetAI_felfire_summoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_maiev_BT_attu";
    newscript->GetAI = &GetAI_npc_maiev_BT_attu;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_akama_BT_attu";
    newscript->GetAI = &GetAI_npc_akama_BT_attu;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_ashtongue_deathsworn";
    newscript->GetAI = &GetAI_npc_ashtongue_deathsworn;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_vagath";
    newscript->GetAI = &GetAI_mob_vagath;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_illidari_shadowlord";
    newscript->GetAI = &GetAI_mob_illidari_shadowlord;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_xiri";
    newscript->pGossipHello =  &GossipHello_npc_xiri;
    newscript->pGossipSelect = &GossipSelect_npc_xiri;
    newscript->GetAI = &GetAI_npc_xiri;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_deathbringer_joovan";
    newscript->GetAI = &GetAI_mob_deathbringer_joovanAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_overlord_orbarokh";
    newscript->pGossipHello = &GossipHello_npc_overlord_orbarokh;
    newscript->pGossipSelect = &GossipSelect_npc_overlord_orbarokh;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_thane_yoregar";
    newscript->pGossipHello = &GossipHello_npc_thane_yoregar;
    newscript->pGossipSelect = &GossipSelect_npc_thane_yoregar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_forged_illidari_bane";
    newscript->pGOUse = &GOUse_go_forged_illidari_bane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_restore_spectrecles";
    newscript->pGossipHello = &GossipHello_npc_restore_spectrecles;
    newscript->pGossipSelect = &GossipSelect_npc_restore_spectrecles;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21949";
    newscript->GetAI = &GetAI_npc_21949;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21211";
    newscript->GetAI = &GetAI_npc_21211;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21207";
    newscript->GetAI = &GetAI_npc_21207;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21210";
    newscript->GetAI = &GetAI_npc_21210;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21195";
    newscript->GetAI = &GetAI_npc_21195;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21292";
    newscript->GetAI = &GetAI_npc_21292;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21310";
    newscript->GetAI = &GetAI_npc_21310;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21409";
    newscript->GetAI = &GetAI_npc_21409;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21410";
    newscript->GetAI = &GetAI_npc_21410;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21309";
    newscript->GetAI = &GetAI_npc_21309;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21316";
    newscript->GetAI = &GetAI_npc_21316;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21302";
    newscript->GetAI = &GetAI_npc_21302;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21315";
    newscript->GetAI = &GetAI_npc_21315;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_22137";
    newscript->GetAI = &GetAI_npc_22137;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_22138";
    newscript->GetAI = &GetAI_npc_22138;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_20427";
    newscript->GetAI = &GetAI_npc_20427;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21332";
    newscript->GetAI = &GetAI_npc_21332;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21797";
    newscript->GetAI = &GetAI_npc_21797;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_21797;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21867";
    newscript->GetAI = &GetAI_npc_21867;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21877";
    newscript->GetAI = &GetAI_npc_21877;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_21876";
    newscript->GetAI = &GetAI_npc_21876;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_guldan";
    newscript->GetAI = &GetAI_npc_guldan;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_oronok";
    newscript->GetAI = &GetAI_npc_oronok;
    newscript->pGossipHello = &GossipHello_npc_oronok;
    newscript->pGossipSelect = &GossipSelect_npc_oronok;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_cyrukh_the_firelord";
    newscript->GetAI = &GetAI_npc_cyrukh_the_firelord;
    newscript->RegisterSelf();
}
