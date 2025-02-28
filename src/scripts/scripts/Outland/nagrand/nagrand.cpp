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
SDName: Nagrand
SD%Complete: 99
SDComment: Quest support: 9849, 9868, 9879, 9918, 9874, 9923, 9924, 9954, 9991, 10107, 10108, 10044, 10168, 10172, 10646, 10085, 10987. TextId's unknown for altruis_the_sufferer and greatmother_geyah (npc_text), 9932, 10011
SDCategory: Nagrand
EndScriptData */

/* ContentData
mob_shattered_rumbler
mob_ancient_orc_ancestor
mob_lump
npc_altruis_the_sufferer
npc_greatmother_geyah
npc_lantresor_of_the_blade
npc_creditmarker_visit_with_ancestors
mob_sparrowhawk
npc_corki_capitive
go_corki_cage
npc_nagrand_captive (npc_maghar_captive and npc_kurenai_captive in 1)
go_maghar_prison
npc_maghar_prisoner
npc_warmaul_pyre
npc_fel_cannon
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## mob_shattered_rumbler - this should be done with ACID
######*/

#define SPELL_EARTH_RUMBLE  33840

struct mob_shattered_rumblerAI : public ScriptedAI
{
    bool Spawn;
    Timer EarthRumbleTimer;

    mob_shattered_rumblerAI(Creature *creature) : ScriptedAI(creature) {}

    void Reset()
    {
        Spawn = false;
        EarthRumbleTimer.Reset(1000);
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == 32001 && !Spawn)
        {
            float x = me->GetPositionX();
            float y = me->GetPositionY();
            float z = me->GetPositionZ();

            caster->SummonCreature(18181,x+(0.7 * (rand()%30)),y+(rand()%5),z,0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000);
            caster->SummonCreature(18181,x+(rand()%5),y-(rand()%5),z,0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000);
            caster->SummonCreature(18181,x-(rand()%5),y+(0.5 *(rand()%60)),z,0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000);
            me->DisappearAndDie();
            Spawn = true;
        }
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        if (EarthRumbleTimer.Expired(diff))
        {
            if (m_creature->IsWithinMeleeRange(m_creature->GetVictim()))
            {
                m_creature->CastSpell(m_creature->GetVictim(), SPELL_EARTH_RUMBLE, false);
                EarthRumbleTimer = urand(8000, 9000);
            }
        }
        
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_shattered_rumbler(Creature *creature)
{
    return new mob_shattered_rumblerAI (creature);
}

/*######
## mob_ancient_orc_ancestor - this should be done with ACID also
######*/

struct mob_ancient_orc_ancestorAI : public ScriptedAI
{
    bool Spawn;

    mob_ancient_orc_ancestorAI(Creature *creature) : ScriptedAI(creature) {}

    void Reset()
    {
    }

    void SpellHit(Unit *hitter, const SpellEntry *spellkind)
    {
        if(spellkind->Id == 34063)
        {
            float x = me->GetPositionX();
            float y = me->GetPositionY();
            float z = me->GetPositionZ();
            hitter->SummonCreature(19480,x,y,z,0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,60000);
            me->DisappearAndDie();
            DoScriptText(RAND(-1018688, -1018689, -1018690, -1018691, -1018692, -1018693, -1018694, -1018695, -1018696, -1018697, -1018698, -1018699, -1018700, -1018701), me, 0);
        }
        return;
    }
};
CreatureAI* GetAI_mob_ancient_orc_ancestor(Creature *creature)
{
    return new mob_ancient_orc_ancestorAI (creature);
}

/*######
## mob_lump
######*/

#define SPELL_VISUAL_SLEEP  16093
#define SPELL_SPEAR_THROW   32248

#define LUMP_SAY0 -1000293
#define LUMP_SAY1 -1000294

#define LUMP_DEFEAT -1000295

#define GOSSIP_HL  16449
#define GOSSIP_SL1 16450
#define GOSSIP_SL2 16451
#define GOSSIP_SL3 16452

struct mob_lumpAI : public ScriptedAI
{
    mob_lumpAI(Creature *creature) : ScriptedAI(creature)
    {
        bReset = false;
    }

    Timer_UnCheked Reset_Timer;
    Timer_UnCheked Spear_Throw_Timer;
    bool bReset;

    void Reset()
    {
        Reset_Timer.Reset(60000);
        Spear_Throw_Timer.Reset(2000);

        me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
    }

    void DamageTaken(Unit *done_by, uint32 & damage)
    {
        if (done_by->GetTypeId() == TYPEID_PLAYER && (me->GetHealth() - damage)*100 / me->GetMaxHealth() < 30)
        {
            if (!bReset && ((Player*)done_by)->GetQuestStatus(9918) == QUEST_STATUS_INCOMPLETE)
            {
                //Take 0 damage
                damage = 0;

                ((Player*)done_by)->AttackStop();
                me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveAllAuras();
                me->DeleteThreatList();
                me->CombatStop();
                me->setFaction(1080);               //friendly
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_SIT);
                DoScriptText(LUMP_DEFEAT, me);

                bReset = true;
            }
        }
    }

    void EnterCombat(Unit *who)
    {
        if (me->HasAura(SPELL_VISUAL_SLEEP,0))
            me->RemoveAura(SPELL_VISUAL_SLEEP,0);

        if (!me->IsStandState())
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);

        DoScriptText(RAND(LUMP_SAY0, LUMP_SAY1), me);
    }

    void UpdateAI(const uint32 diff)
    {
        //check if we waiting for a reset
        if (bReset)
        {
            if (Reset_Timer.Expired(diff))
            {
                EnterEvadeMode();
                bReset = false;
                me->setFaction(1711);               //hostile
                return;
            }
        }

        //Return since we have no target
        if (!UpdateVictim())
            return;

        //Spear_Throw_Timer
        if (Spear_Throw_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_SPEAR_THROW);
            Spear_Throw_Timer = 20000;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_lump(Creature *creature)
{
    return new mob_lumpAI(creature);
}

bool GossipHello_mob_lump(Player *player, Creature *creature)
{
    if (player->GetQuestStatus(9918) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HL), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(9352, creature->GetGUID());

    return true;
}

bool GossipSelect_mob_lump(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(9353, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(9354, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SL3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(9355, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->SEND_GOSSIP_MENU(9356, creature->GetGUID());
            player->TalkedToCreature(18354, creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_altruis_the_sufferer
######*/

#define GOSSIP_HATS1 16453
#define GOSSIP_HATS2 16454
// Illidan's Pupil
#define GOSSIP_IP_0  16455
#define GOSSIP_IP_1  16456
#define GOSSIP_IP_2  16457
#define GOSSIP_IP_3  16458
#define GOSSIP_IP_4  16459
#define GOSSIP_IP_5  16460

#define GOSSIP_SATS1 16461
#define GOSSIP_SATS2 16462
#define GOSSIP_SATS3 16463
#define GOSSIP_SATS4 16464
#define GOSSIP_SATS5 16465
//#define GOSSIP_SATS6 "[PH] Story done"

bool GossipHello_npc_altruis_the_sufferer(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu( creature->GetGUID() );

    //gossip before obtaining Survey the Land
    if ( player->GetQuestStatus(9991) == QUEST_STATUS_NONE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HATS1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+10);

    //gossip when Survey the Land is incomplete (technically, after the flight)
    if (player->GetQuestStatus(9991) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HATS2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+20);

    //wowwiki.com/Varedis
    if (player->GetQuestStatus(10646) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_IP_0), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+30);

    player->SEND_GOSSIP_MENU(9419, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_altruis_the_sufferer(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+10:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SATS1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            player->SEND_GOSSIP_MENU(9420, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SATS2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
            player->SEND_GOSSIP_MENU(9421, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+12:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SATS3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
            player->SEND_GOSSIP_MENU(9422, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+13:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SATS4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
            player->SEND_GOSSIP_MENU(9423, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+14:
            player->SEND_GOSSIP_MENU(9424, creature->GetGUID());
            break;

        case GOSSIP_ACTION_INFO_DEF+20:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SATS5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            player->SEND_GOSSIP_MENU(9427, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(9991);
            break;

        case GOSSIP_ACTION_INFO_DEF+30:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_IP_1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 31);
            player->SEND_GOSSIP_MENU(10492, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+31:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_IP_2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 32);
            player->SEND_GOSSIP_MENU(10493, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+32:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_IP_3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 33);
            player->SEND_GOSSIP_MENU(10494, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+33:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_IP_4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 34);
            player->SEND_GOSSIP_MENU(10495, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+34:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_IP_5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 35);
            player->SEND_GOSSIP_MENU(10497, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+35:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(10646);
            break;
    }
    return true;
}

bool QuestAccept_npc_altruis_the_sufferer(Player *player, Creature *creature, Quest const *quest )
{
    if (quest->GetQuestId() == 9991)
    {
        if (!player->GetQuestRewardStatus(9991))              //Survey the Land, q-id 9991
        {
            player->CLOSE_GOSSIP_MENU();

            std::vector<uint32> nodes;

            nodes.resize(2);
            nodes[0] = 113;                                     //from
            nodes[1] = 114;                                     //end at
            player->ActivateTaxiPathTo(nodes);                  //TaxiPath 532
        }
    }
    return true;
}

/*######
## npc_greatmother_geyah
######*/

#define GOSSIP_HGG1  16466
#define GOSSIP_HGG2  16467

#define GOSSIP_SGG1  16468
#define GOSSIP_SGG2  16469
#define GOSSIP_SGG3  16470
#define GOSSIP_SGG4  16471
#define GOSSIP_SGG5  16472
#define GOSSIP_SGG6  16473
#define GOSSIP_SGG7  16474
#define GOSSIP_SGG8  16475
#define GOSSIP_SGG9  16476
#define GOSSIP_SGG10 16477
#define GOSSIP_SGG11 16478

//all the textId's for the below is unknown, but i do believe the gossip item texts are proper.
bool GossipHello_npc_greatmother_geyah(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu( creature->GetGUID() );

    if (player->GetQuestStatus(10044) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HGG1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(creature->GetNpcTextId(),creature->GetGUID());
    }
    else if (player->GetQuestStatus(10172) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HGG2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
        player->SEND_GOSSIP_MENU(creature->GetNpcTextId(),creature->GetGUID());
    }
    else

        player->SEND_GOSSIP_MENU(creature->GetNpcTextId(),creature->GetGUID());

    return true;
}

bool GossipSelect_npc_greatmother_geyah(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 6:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 7:
            player->AreaExploredOrEventHappens(10044);
            player->CLOSE_GOSSIP_MENU();
            break;

        case GOSSIP_ACTION_INFO_DEF + 10:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 11:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG8), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 12);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 12:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG9), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 13);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 13:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG10), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 14);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 14:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SGG11), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);
            player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 15:
            player->AreaExploredOrEventHappens(10172);
            player->CLOSE_GOSSIP_MENU();
            break;
    }
    return true;
}

/*######
## npc_lantresor_of_the_blade
######*/

#define GOSSIP_HLB  16479
#define GOSSIP_SLB1 16480
#define GOSSIP_SLB2 16481
#define GOSSIP_SLB3 16482
#define GOSSIP_SLB4 16483
#define GOSSIP_SLB5 16484
#define GOSSIP_SLB6 16485
#define GOSSIP_SLB7 16486

bool GossipHello_npc_lantresor_of_the_blade(Player *player, Creature *creature)
{
    if (creature->isQuestGiver())
        player->PrepareQuestMenu( creature->GetGUID() );

    if (player->GetQuestStatus(10107) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10108) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_HLB), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(9361, creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lantresor_of_the_blade(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SLB1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(9362, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SLB2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(9363, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SLB3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(9364, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SLB4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(9365, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SLB5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            player->SEND_GOSSIP_MENU(9366, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SLB6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            player->SEND_GOSSIP_MENU(9367, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_SLB7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            player->SEND_GOSSIP_MENU(9368, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->SEND_GOSSIP_MENU(9369, creature->GetGUID());
            if (player->GetQuestStatus(10107) == QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(10107);
            if (player->GetQuestStatus(10108) == QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(10108);
            break;
    }
    return true;
}

/*######
## npc_creditmarker_visist_with_ancestors
######*/

struct npc_creditmarker_visit_with_ancestorsAI : public ScriptedAI
{
    npc_creditmarker_visit_with_ancestorsAI(Creature* creature) : ScriptedAI(creature) {}

    void Reset() {}

    void MoveInLineOfSight(Unit *who)
    {
        if(!who)
            return;

        if(who->GetTypeId() == TYPEID_PLAYER)
        {
            if(((Player*)who)->GetQuestStatus(10085) == QUEST_STATUS_INCOMPLETE)
            {
                uint32 creditMarkerId = me->GetEntry();
                if((creditMarkerId >= 18840) && (creditMarkerId <= 18843))
                {
                    // 18840: Sunspring, 18841: Laughing, 18842: Garadar, 18843: Bleeding
                    if(!((Player*)who)->GetReqKillOrCastCurrentCount(10085, creditMarkerId))
                        ((Player*)who)->KilledMonster(creditMarkerId, me->GetGUID());
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_creditmarker_visit_with_ancestors(Creature *creature)
{
    return new npc_creditmarker_visit_with_ancestorsAI (creature);
}

/*######
## mob_sparrowhawk
######*/

#define SPELL_SPARROWHAWK_NET 39810
#define SPELL_ITEM_CAPTIVE_SPARROWHAWK 39812

struct mob_sparrowhawkAI : public ScriptedAI
{

    mob_sparrowhawkAI(Creature *creature) : ScriptedAI(creature) {}

    Timer_UnCheked Check_Timer;
    uint64 PlayerGUID;
    bool fleeing;

    void Reset()
    {
        me->RemoveAurasDueToSpell(SPELL_SPARROWHAWK_NET);
        Check_Timer.Reset(1000);
        PlayerGUID = 0;
        fleeing = false;
    }
    void AttackStart(Unit *who)
    {
        if(PlayerGUID)
            return;

        ScriptedAI::AttackStart(who);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(!who || PlayerGUID)
            return;

        if(!PlayerGUID && who->GetTypeId() == TYPEID_PLAYER && me->IsWithinDistInMap(((Player *)who), 30) && ((Player *)who)->GetQuestStatus(10987) == QUEST_STATUS_INCOMPLETE)
        {
            PlayerGUID = who->GetGUID();
            return;
        }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(Check_Timer.Expired(diff))
        {
            if(PlayerGUID)
            {
                if(fleeing && me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FLEEING_MOTION_TYPE)
                    fleeing = false;

                Player *player = (Player *)Unit::GetUnit((*me), PlayerGUID);
                if(player && me->IsWithinDistInMap(player, 30))
                {
                    if(!fleeing)
                    {
                        me->DeleteThreatList();
                        me->GetMotionMaster()->MoveFleeing(player);
                        fleeing = true;
                    }
                }
                else if(fleeing)
                {
                    me->GetMotionMaster()->MovementExpired();
                    PlayerGUID = 0;
                    fleeing = false;
                }
            }
            Check_Timer = 1000;
        }

        if (PlayerGUID)
            return;

        ScriptedAI::UpdateAI(diff);
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (caster->GetTypeId() == TYPEID_PLAYER)
        {
            if(spell->Id == SPELL_SPARROWHAWK_NET && ((Player*)caster)->GetQuestStatus(10987) == QUEST_STATUS_INCOMPLETE)
            {
                me->CastSpell(caster, SPELL_ITEM_CAPTIVE_SPARROWHAWK, true);
                me->DealDamage(me, me->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            }
        }
        return;
    }
};

CreatureAI* GetAI_mob_sparrowhawk(Creature *creature)
{
    return new mob_sparrowhawkAI (creature);
}

/*########
## Quests: HELP!, Corki's Gone Missing Again!, Cho'war the Pillager
########*/

enum CorkiCage
{
    QUEST_HELP1            = 9923, // HELP!
    NPC_CORKI_CAPITIVE1    = 18445,
    GO_CORKI_CAGE1         = 182349,

    QUEST_HELP2            = 9924, // Corki's Gone Missing Again!
    NPC_CORKI_CAPITIVE2    = 20812,
    GO_CORKI_CAGE2         = 182350,

    QUEST_HELP3            = 9955, // Cho'war the Pillager
    NPC_CORKI_CAPITIVE3    = 18369,
    GO_CORKI_CAGE3         = 182521,

    SAY_THANKS             = -1900133,
    SAY_KORKI2             = -1900134,
    SAY_KORKI3             = -1900135,
    SAY_KORKI4             = -1900136,
    SAY_KORKI5             = -1900137,
    SAY_KORKI6             = -1900138,
    SAY_THANKS1            = -1900139
};



struct npc_corki_capitiveAI : public ScriptedAI
{
    npc_corki_capitiveAI(Creature *creature) : ScriptedAI(creature){}

    uint64 PlayerGUID;

    void Reset()
    {
        PlayerGUID = 0;
        me->SetWalk(false);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER && ((Player *)who)->GetTeam() == ALLIANCE && me->IsWithinDistInMap(((Player *)who), 20.0f))
        {
            if (PlayerGUID == who->GetGUID())
            {
                return;
            }
            else PlayerGUID = 0;

            switch (urand(0,4))
            {
                case 0:
                    DoScriptText(SAY_KORKI2, me);
                    break;
                case 1: 
                    DoScriptText(SAY_KORKI3, me);
                    break;
                case 2:
                    DoScriptText(SAY_KORKI4, me);
                    break;
                case 3:
                    DoScriptText(SAY_KORKI5, me);
                    break;
                case 4:
                    DoScriptText(SAY_KORKI6, me);
                    break;
            }

            PlayerGUID = who->GetGUID();
        }
    }

    void MovementInform(uint32 MotionType, uint32 i)
    {
        if (MotionType == POINT_MOTION_TYPE)
            me->ForcedDespawn();
    }
};

CreatureAI* GetAI_npc_corki_capitiveAI(Creature* creature)
{
    return new npc_corki_capitiveAI(creature);
}

bool go_corki_cage(Player* player, GameObject* go)
{
   Creature* Creature = NULL;
    switch(go->GetEntry())
    {
        case GO_CORKI_CAGE1:
            if(player->GetQuestStatus(QUEST_HELP1) == QUEST_STATUS_INCOMPLETE)
                Creature = GetClosestCreatureWithEntry(go, NPC_CORKI_CAPITIVE1, 5.0f);
            break;
        case GO_CORKI_CAGE2:
            if(player->GetQuestStatus(QUEST_HELP2) == QUEST_STATUS_INCOMPLETE)
                Creature = GetClosestCreatureWithEntry(go, NPC_CORKI_CAPITIVE2, 5.0f);
            break;
        case GO_CORKI_CAGE3:
            if(player->GetQuestStatus(QUEST_HELP3) == QUEST_STATUS_INCOMPLETE)
                Creature = GetClosestCreatureWithEntry(go, NPC_CORKI_CAPITIVE3, 5.0f);
            break;
    }
    if(Creature)
    {
        switch (Creature->GetEntry())
        {
            case NPC_CORKI_CAPITIVE1:
                DoScriptText(SAY_THANKS, Creature, player);
                Creature->GetMotionMaster()->Clear();
                Creature->GetMotionMaster()->MovePoint(0, -2534.01f, 6269.02f, 17.14f);
                break;
            case NPC_CORKI_CAPITIVE2:
                DoScriptText(SAY_THANKS1, Creature, player);
                Creature->GetMotionMaster()->Clear();
                Creature->GetMotionMaster()->MovePoint(0, -1000.26f, 8113.16f, -95.85f);
                break;
            case NPC_CORKI_CAPITIVE3:
                DoScriptText(SAY_THANKS, Creature, player);
                Creature->GetMotionMaster()->Clear();
                Creature->GetMotionMaster()->MovePoint(0, -897.06f, 8688.03f, 170.47f);
                break;
        }
        player->KilledMonster(Creature->GetEntry(), Creature->GetGUID());
        return false;
    }
    return true;
}

/*#####
## npc_nagrand_captive
#####*/

enum eNagrandCaptive
{
    SAY_MAG_START               = -1000482,
    SAY_MAG_NO_ESCAPE           = -1000483,
    SAY_MAG_MORE                = -1000484,
    SAY_MAG_MORE_REPLY          = -1000485,
    SAY_MAG_LIGHTNING           = -1000486,
    SAY_MAG_SHOCK               = -1000487,
    SAY_MAG_COMPLETE            = -1000488,
    SAY_KUR_COMPLETE            = -1900132,

    SPELL_CHAIN_LIGHTNING       = 16006,
    SPELL_EARTHBIND_TOTEM       = 15786,
    SPELL_FROST_SHOCK           = 12548,
    SPELL_HEALING_WAVE          = 12491,

    QUEST_TOTEM_KARDASH_H       = 9868,
    QUEST_TOTEM_KARDASH_A       = 9879,

    NPC_KUR_CAPTIVE             = 18209,
    NPC_MAG_CAPTIVE             = 18210,
    NPC_MURK_RAIDER             = 18203,
    NPC_MURK_BRUTE              = 18211,
    NPC_MURK_SCAVENGER          = 18207,
    NPC_MURK_PUTRIFIER          = 18202
};

static float AmbushAH[]= {-1568.805786, 8533.873047, 1.958};
static float AmbushAA[]= {-1517.302246, 8439.866211, -4.035};
static float AmbushB[]= {-1442.524780, 8500.364258, 6.381};

struct npc_nagrand_captiveAI : public npc_escortAI
{
    npc_nagrand_captiveAI(Creature* creature) : npc_escortAI(creature) { Reset(); }

    Timer_UnCheked ChainLightningTimer;
    Timer_UnCheked HealTimer;
    Timer_UnCheked FrostShockTimer;;

    void Reset()
    {
        ChainLightningTimer.Reset(1000);
        HealTimer.Reset(1);
        FrostShockTimer.Reset(6000);
    }

    void EnterCombat(Unit* /*who*/)
    {
        DoCast(me, SPELL_EARTHBIND_TOTEM, false);
    }

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 7:
                if (me->GetEntry() == NPC_KUR_CAPTIVE)
                {
                    DoScriptText(SAY_MAG_MORE, me);

                    if (Creature* pTemp = me->SummonCreature(NPC_MURK_PUTRIFIER, AmbushB[0], AmbushB[1], AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000))
                        DoScriptText(SAY_MAG_MORE_REPLY, pTemp);

                    me->SummonCreature(NPC_MURK_PUTRIFIER, AmbushB[0]-2.5f, AmbushB[1]-2.5f, AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    me->SummonCreature(NPC_MURK_SCAVENGER, AmbushB[0]+2.5f, AmbushB[1]+2.5f, AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    me->SummonCreature(NPC_MURK_SCAVENGER, AmbushB[0]+2.5f, AmbushB[1]-2.5f, AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                }
                break;
             case 10:
                if (me->GetEntry() == NPC_MAG_CAPTIVE)
                {
                    DoScriptText(SAY_MAG_MORE, me);

                    if (Creature* pTemp = me->SummonCreature(NPC_MURK_PUTRIFIER, AmbushB[0], AmbushB[1], AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000))
                        DoScriptText(SAY_MAG_MORE_REPLY, pTemp);

                    me->SummonCreature(NPC_MURK_PUTRIFIER, AmbushB[0]-2.5f, AmbushB[1]-2.5f, AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    me->SummonCreature(NPC_MURK_SCAVENGER, AmbushB[0]+2.5f, AmbushB[1]+2.5f, AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                    me->SummonCreature(NPC_MURK_SCAVENGER, AmbushB[0]+2.5f, AmbushB[1]-2.5f, AmbushB[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                }
                break;
            case 16:
                switch (me->GetEntry())
                {
                    case NPC_MAG_CAPTIVE:
                        DoScriptText(SAY_MAG_COMPLETE, me);
                        break;
                    case NPC_KUR_CAPTIVE:
                        DoScriptText(SAY_KUR_COMPLETE, me);
                        break;
                }

                if (Player* player = GetPlayerForEscort())
                    player->GroupEventHappens(player->GetTeam() == ALLIANCE ? QUEST_TOTEM_KARDASH_A : QUEST_TOTEM_KARDASH_H, me);

                SetRun();
                break;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_MURK_BRUTE)
            DoScriptText(SAY_MAG_NO_ESCAPE, summoned);

        if (summoned->isTotem())
            return;

        summoned->SetWalk(false);
        summoned->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
        summoned->AI()->AttackStart(me);

    }

    void SpellHitTarget(Unit* /*target*/, const SpellEntry* spell)
    {
        if (spell->Id == SPELL_CHAIN_LIGHTNING)
        {
            if (rand()%10)
                return;

            DoScriptText(SAY_MAG_LIGHTNING, me);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        if (!me->GetVictim())
            return;

        if (ChainLightningTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_CHAIN_LIGHTNING);
            ChainLightningTimer = urand(7000, 14000);
        }

        if (me->GetHealth()*100 < me->GetMaxHealth()*30)
        {
            if (HealTimer.Expired(diff))
            {
                DoCast(me, SPELL_HEALING_WAVE);
                HealTimer = 5000;
            }
        }

        if (FrostShockTimer.Expired(diff))
        {
            DoCast(me->GetVictim(), SPELL_FROST_SHOCK);
            FrostShockTimer = urand(7500, 15000);
        }

        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_nagrand_captive(Player* player, Creature* creature, const Quest* quest)
{
    switch (quest->GetQuestId())
    {
        case QUEST_TOTEM_KARDASH_H:
            if (npc_nagrand_captiveAI* EscortAI = dynamic_cast<npc_nagrand_captiveAI*>(creature->AI()))
            {
                creature->SetStandState(UNIT_STAND_STATE_STAND);
                creature->setFaction(232);

                EscortAI->Start(true, false, player->GetGUID(), quest);

                DoScriptText(SAY_MAG_START, creature);

                creature->SummonCreature(NPC_MURK_RAIDER, AmbushAH[0]+2.5f, AmbushAH[1]-2.5f, AmbushAH[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                creature->SummonCreature(NPC_MURK_PUTRIFIER, AmbushAH[0]-2.5f, AmbushAH[1]+2.5f, AmbushAH[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                creature->SummonCreature(NPC_MURK_BRUTE, AmbushAH[0], AmbushAH[1], AmbushAH[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            }
            break;
        case QUEST_TOTEM_KARDASH_A:
            if (npc_nagrand_captiveAI* EscortAI = dynamic_cast<npc_nagrand_captiveAI*>(creature->AI()))
            {
                creature->SetStandState(UNIT_STAND_STATE_STAND);
                creature->setFaction(231);

                EscortAI->Start(true, false, player->GetGUID(), quest);

                DoScriptText(SAY_MAG_START, creature);

                creature->SummonCreature(NPC_MURK_RAIDER, AmbushAA[0]+2.5f, AmbushAA[1]-2.5f, AmbushAA[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                creature->SummonCreature(NPC_MURK_PUTRIFIER, AmbushAA[0]-2.5f, AmbushAA[1]+2.5f, AmbushAA[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                creature->SummonCreature(NPC_MURK_BRUTE, AmbushAA[0], AmbushAA[1], AmbushAA[2], 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            }
            break;
        default:
            return false;
    }

    return true;
}

CreatureAI* GetAI_npc_nagrand_captive(Creature* creature)
{
    return new npc_nagrand_captiveAI(creature);
}

/*####
#  npc_multiphase_disurbanceAI
####*/

#define SPELL_TAKE_MULTIPHASE_READING   46281

struct npc_multiphase_disurbanceAI : public ScriptedAI
{
    npc_multiphase_disurbanceAI(Creature *creature) : ScriptedAI(creature){}

    Timer_UnCheked despawnTimer;

    void Reset()
    {
        despawnTimer = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (despawnTimer.Expired(diff))
        {
            me->ForcedDespawn();
            despawnTimer = 0;
        }
    }

    void SpellHit(Unit * /*caster*/, const SpellEntry * spell)
    {
        if (spell && spell->Id == SPELL_TAKE_MULTIPHASE_READING)
            despawnTimer = 2500;
    }
};

CreatureAI* GetAI_npc_multiphase_disturbance(Creature* creature)
{
    return new npc_multiphase_disurbanceAI(creature);
}

/*#####
## go_maghar_prison & npc_maghar_prisoner
#####*/

enum
{
    QUEST_SURVIVORS       = 9948,
    NPC_MPRISONER         = 18428,

    SAY_MAG_PRISONER1     = -1900148,
    SAY_MAG_PRISONER2     = -1900149,
    SAY_MAG_PRISONER3     = -1900150,
    SAY_MAG_PRISONER4     = -1900151,
    SAYT_MAG_PRISONER1    = -1900152,
    SAYT_MAG_PRISONER2    = -1900153,
    SAYT_MAG_PRISONER3    = -1900154,
    SAYT_MAG_PRISONER4    = -1900155
};

struct WP
{
    float x, y, z;
};

static WP M[]=
{
    {-1076.000f, 8945.270f, 101.891f},
    {-782.796f, 8875.171f, 181.745f},
    {-670.298f, 8810.587f, 196.057f},
    {-710.969f, 8763.471f, 186.513f},
    {-865.144f, 8713.610f, 248.041f},
    {-847.285f, 8722.406f, 177.255f},
    {-897.005f, 8689.280f, 170.527},
    {-838.047f, 8691.124f, 180.549f}
};

struct npc_maghar_prisonerAI : public ScriptedAI
{
    npc_maghar_prisonerAI(Creature *creature) : ScriptedAI(creature) {}

    uint64 PlayerGUID;

    void Reset()
    {
        PlayerGUID = 0;
        me->SetWalk(false);
    }

    void MoveInLineOfSight(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER && ((Player *)who)->GetTeam() == HORDE && me->IsWithinDistInMap(((Player *)who), 20.0f))
        {
            if (PlayerGUID == who->GetGUID())
            {
                return;
            }
            else PlayerGUID = 0;

            switch (urand(0,3))
            {
                case 0:
                    DoScriptText(SAY_MAG_PRISONER1, me);
                    break;
                case 1:
                    DoScriptText(SAY_MAG_PRISONER2, me);
                    break;
                case 2:
                    DoScriptText(SAY_MAG_PRISONER3, me);
                    break;
                case 3:
                    DoScriptText(SAY_MAG_PRISONER4, me);
                    break;
            }
            PlayerGUID = who->GetGUID();
        }
    }

    uint32 WaypointID()
    {
        switch (me->GetGUIDLow())
        {
            case 65828:
            case 65826:
            case 65827:
            case 65825:
            case 65829:
                return 1;
                break;
            case 65823:
            case 65824:
            case 65821:
            case 65815:
                return 2;
                break;
            case 65814:
                return 3;
                break;
            case 65813:
                return 4;
                break;
            case 65819:
            case 65820:
                return 5;
                break;
            case 65817:
            case 65822:
            case 65816:
                return 6;
                break;
            case 65831:
            case 65832:
            case 65830:
                return 7;
                break;
            case 65818:
                return 8;
                break;
            default:
                return 1;
                break;
        }
    }

    void StartRun(Player* player)
    {
        switch (WaypointID())
        {
            case 1:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[0].x, M[0].y, M[0].z);
                break;
            case 2:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[1].x, M[1].y, M[1].z);
                break;
            case 3:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[2].x, M[2].y, M[2].z);
                break;
            case 4:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[3].x, M[3].y, M[3].z);
                break;
            case 5:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[4].x, M[4].y, M[4].z);
                break;
            case 6:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[5].x, M[5].y, M[5].z);
                break;
            case 7:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[6].x, M[6].y, M[6].z);
                break;
            case 8:
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(0, M[7].x, M[7].y, M[7].z);
                break;
        }
        return;
    }

    void MovementInform(uint32 MotionType, uint32 i)
    {
        if (MotionType == POINT_MOTION_TYPE)
        {
            me->ForcedDespawn();
        }
    }
};

CreatureAI* GetAI_npc_maghar_prisoner(Creature* creature)
{
    return new npc_maghar_prisonerAI(creature);
}

bool go_maghar_prison(Player* player, GameObject* go)
{
    Creature* Prisoner = NULL;

    if (player->GetQuestStatus(QUEST_SURVIVORS) == QUEST_STATUS_INCOMPLETE)
    {
        Prisoner = GetClosestCreatureWithEntry(go, NPC_MPRISONER, 5.0f);

        if(Prisoner)
        {
            player->KilledMonster(NPC_MPRISONER, Prisoner->GetGUID());

            switch (urand(0,3))
            {
                case 0:
                    DoScriptText(SAYT_MAG_PRISONER1, Prisoner, player);
                    break;
                case 1:
                    DoScriptText(SAYT_MAG_PRISONER2, Prisoner, player);
                    break;
                case 2:
                    DoScriptText(SAYT_MAG_PRISONER3, Prisoner, player);
                    break;
                case 3:
                    DoScriptText(SAYT_MAG_PRISONER4, Prisoner, player);
                    break;
            }

            if (npc_maghar_prisonerAI* scriptedAI = CAST_AI(npc_maghar_prisonerAI, Prisoner->AI()))
            {
                scriptedAI->StartRun(player);
            }

            return false;
        }
    }
    return true;
};

/*#####
## npc_warmaul_pyre Q 9932
#####*/

enum
{
    NPC_SABOTEUR         = 18396,
    NPC_CORPSE           = 18397,

    SAY_SABOTEUR1        = -1900192,
    SAY_SABOTEUR2        = -1900193,
    SAY_SABOTEUR3        = -1900194,
    SAY_SABOTEUR4        = -1900195,
    SAY_SABOTEUR5        = -1900196,
    SAY_SABOTEUR6        = -1900197,
    SAY_SABOTEUR7        = -1900198,
    SAY_SABOTEUR8        = -1900199,
    SAY_SABOTEUR9        = -1900200,
    SAY_SABOTEUR10       = -1900201
};

struct Move
{
    float x, y, z;
};

static Move Z[]=
{
    {-885.76f, 7717.75f, 35.24f},
    {-882.96f, 7723.00f, 34.78f},
    {-871.40f, 7724.87f, 33.36f},
    {-873.16F, 7727.59f, 33.35f},
    {-855.66f, 7732.36f, 33.42f},
    {-855.44f, 7735.44f, 33.44f},
    {-843.39f, 7726.59f, 34.50f},
    {-840.20f, 7728.34f, 34.39f},
    {-848.31f, 7714.37f, 34.42f},
    {-845.44f, 7710.70f, 35.05f},
    {-859.99f, 7713.96f, 35.94f},
    {-859.70f, 7710.61f, 36.68f},
    {-873.74f, 7720.35f, 33.98f},
    {-875.16f, 7717.15f, 34.39f}
};

struct npc_warmaul_pyreAI : public ScriptedAI
{
    npc_warmaul_pyreAI(Creature* creature) : ScriptedAI(creature) {}

    std::list<uint64> SaboteurList;
    uint64 PlayerGUID;
    Timer_UnCheked StepsTimer;
    uint32 Steps;
    uint32 CorpseCount;
    uint32 MoveCount;

    void Reset()
    {
        PlayerGUID = 0;
        StepsTimer.Reset(1);
        Steps = 0;
        CorpseCount = 0;
        MoveCount = 1;
        me->SetVisibility(VISIBILITY_OFF);
    }

    void EnterCombat(Unit *who) {}

    void IsSummonedBy(Unit* summoner)
    {
        if (Player* plr = summoner->ToPlayer())
        {
            if (plr->GetQuestStatus(9932) == QUEST_STATUS_INCOMPLETE)
            {
                StepsTimer.Reset(1);
                PlayerGUID = plr->GetGUID();
            }
        }
    }

    void DoSummon()
    {
        ++CorpseCount;

        uint32 Time = 100000 - (10000 *CorpseCount);

        if (Creature* Saboteur = GetSaboteur(2))
        {
            Saboteur->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
            me->SummonCreature(NPC_CORPSE, Saboteur->GetPositionX(), Saboteur->GetPositionY(), Saboteur->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, Time);
        }

        if (Creature* Saboteur = GetSaboteur(1))
        {
            Saboteur->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
            me->SummonCreature(NPC_CORPSE, Saboteur->GetPositionX(), Saboteur->GetPositionY(), Saboteur->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, Time);
        }
    }

    void Move()
    {
        ++MoveCount;
        if (Creature* Saboteur = GetSaboteur(2))
            Saboteur->GetMotionMaster()->MovePoint(0, Z[MoveCount].x, Z[MoveCount].y, Z[MoveCount].z);

        ++MoveCount;
        if (Creature* Saboteur = GetSaboteur(1))
            Saboteur->GetMotionMaster()->MovePoint(0, Z[MoveCount].x, Z[MoveCount].y, Z[MoveCount].z);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_SABOTEUR)
        {
            summoned->SetWalk(true);
            SaboteurList.push_back(summoned->GetGUID());
        }
    }

    Creature* GetSaboteur(uint8 ListNum)
    {
        if (SaboteurList.empty())
            return NULL;

        uint8 Num = 1;

        for (std::list<uint64>::iterator itr = SaboteurList.begin(); itr != SaboteurList.end(); ++itr)
        {
            if (ListNum && ListNum != Num)
            {
                ++Num;
                continue;
            }
            if (Unit* Saboteur = m_creature->GetUnit(*itr))
                if (Saboteur->isAlive() && Saboteur->IsWithinDistInMap(me, 40.0f))
                    return Saboteur->ToCreature();
        }

        return NULL;
    }

    uint32 NextStep(uint32 Steps)
    {
        switch (Steps)
        {
            case 1:
                SaboteurList.clear();
                me->SummonCreature(NPC_SABOTEUR, Z[0].x, Z[0].y, Z[0].z, 0.6f, TEMPSUMMON_CORPSE_DESPAWN, 60000);
                me->SummonCreature(NPC_SABOTEUR, Z[1].x, Z[1].y, Z[1].z, 3.8f, TEMPSUMMON_CORPSE_DESPAWN, 60000);
                return 4000;

            case 2:
                if (Creature* Saboteur = GetSaboteur(2))
                    DoScriptText(SAY_SABOTEUR1, Saboteur);
                return 5000;

            case 3:
                if (Creature* Saboteur = GetSaboteur(1))
                    DoScriptText(SAY_SABOTEUR2, Saboteur);
                return 5000;

            case 4:
                if (Creature* Saboteur = GetSaboteur(2))
                    DoScriptText(SAY_SABOTEUR3, Saboteur);
                return 5000;

            case 5:
                if (Creature* Saboteur = GetSaboteur(1))
                    DoScriptText(SAY_SABOTEUR4, Saboteur);
                return 4000;

            case 6:
                Move();
                return 6000;

            case 7:
                DoSummon();
                return 2000;

            case 8:
                if (Creature* Saboteur = GetSaboteur(2))
                    DoScriptText(SAY_SABOTEUR5, Saboteur);
                return 2000;

            case 9:
                Move();
                return 7000;

            case 10:
                DoSummon();
                return 2000;

            case 11:
                if (Creature* Saboteur = GetSaboteur(1))
                    DoScriptText(SAY_SABOTEUR6, Saboteur);
                return 2000;

            case 12:
                Move();
                return 7000;

            case 13:
                DoSummon();
                return 2000;

            case 14:
                if (Creature* Saboteur = GetSaboteur(2))
                    DoScriptText(SAY_SABOTEUR7, Saboteur);
                return 3000;

            case 15:
                if (Creature* Saboteur = GetSaboteur(1))
                    DoScriptText(SAY_SABOTEUR7, Saboteur);
                return 2000;

            case 16:
                Move();
                return 7000;

            case 17:
                DoSummon();
                return 2000;     

            case 18:
                if (Creature* Saboteur = GetSaboteur(2))
                    DoScriptText(SAY_SABOTEUR8, Saboteur);
                return 3000;           

            case 19:
                if (Creature* Saboteur = GetSaboteur(1))
                    DoScriptText(SAY_SABOTEUR9, Saboteur);
                return 2000; 

            case 20:
                Move();
                return 7000;

            case 21:
                DoSummon();
                return 2000;

            case 22:
                if (Creature* Saboteur = GetSaboteur(2))
                    DoScriptText(SAY_SABOTEUR10, Saboteur);
                return 2000; 

            case 23:
                Move();
                return 7000;

            case 24:
                if (Player* player = me->GetPlayerInWorld(PlayerGUID))
                    if (me->IsWithinDistInMap(player, 15.0f))
                        ((Player*) player)->KilledMonster(18395, me->GetObjectGuid());
                return 2000;

            case 25:
                for (std::list<uint64>::iterator itr = SaboteurList.begin(); itr != SaboteurList.end(); ++itr)
                {
                    if (Unit* Saboteur = m_creature->GetUnit(*itr))
                        Saboteur->Kill(Saboteur);
                }
                me->DisappearAndDie();

            default:
                return 0;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (StepsTimer.Expired(diff))
            StepsTimer = NextStep(++Steps);
    }
};

CreatureAI* GetAI_npc_warmaul_pyre(Creature *creature)
{
    return new npc_warmaul_pyreAI (creature);
}

/*######
## npc_fel_cannon
######*/

enum
{
    NPC_CANNON_FEAR           = 19210,
    NPC_FEAR_TARGET           = 19211,
    NPC_CANNON_HATE           = 19067,
    NPC_HATE_TARGET           = 19212,

    SPELL_BOLT                = 40109,
    SPELL_HATE                = 33531,
    SPELL_FEAR                = 33532,

    OBJECT_LARGE_FIRE         = 187084,
};

struct npc_fel_cannonAI : public ScriptedAI
{
    npc_fel_cannonAI(Creature *creature) : ScriptedAI(creature) {}

    void Reset(){}

    void EnterCombat(Unit *who){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_HATE && me->GetEntry() == NPC_CANNON_HATE)
        {
            if (Creature* Target = GetClosestCreatureWithEntry(me, NPC_HATE_TARGET, 85.5f))
            {
                me->SetFacingToObject(Target);
                DoCast(Target, SPELL_BOLT);
                me->SummonGameObject(OBJECT_LARGE_FIRE, Target->GetPositionX(), Target->GetPositionY(), Target->GetPositionZ(), Target->GetOrientation(), 0, 0, 0, 0, 30);
            }
        }

        if(spell->Id == SPELL_FEAR && me->GetEntry() == NPC_CANNON_FEAR)
        {
            if (Creature* Target = GetClosestCreatureWithEntry(me, NPC_FEAR_TARGET, 85.5f))
            {
                me->SetFacingToObject(Target);
                DoCast(Target, SPELL_BOLT);
                me->SummonGameObject(OBJECT_LARGE_FIRE, Target->GetPositionX(), Target->GetPositionY(), Target->GetPositionZ(), Target->GetOrientation(), 0, 0, 0, 0, 30);
            }
        }
        return;
    }
};
CreatureAI* GetAI_npc_fel_cannon(Creature *creature)
{
    return new npc_fel_cannonAI (creature);
}

/*######
## npc_rethhedron
######*/

enum
{
    SAY_LOW_HP                      = -1000966,
    SAY_EVENT_END                   = -1000967,

    SPELL_CRIPPLE                   = 41281,
    SPELL_SHADOW_BOLT               = 41280,
    SPELL_ABYSSAL_TOSS              = 41283,                // summon npc 23416 at target position
    SPELL_ABYSSAL_IMPACT            = 41284,
    // SPELL_GROUND_AIR_PULSE       = 41270,                // spell purpose unk
    // SPELL_AGGRO_CHECK            = 41285,                // spell purpose unk
    // SPELL_AGGRO_BURST            = 41286,                // spell purpose unk

    SPELL_COSMETIC_LEGION_RING      = 41339,
    SPELL_QUEST_COMPLETE            = 41340,
    
    SPELL_PURPLE_BEAM               = 39123,

    NPC_SPELLBINDER                 = 22342,
    NPC_RETHHEDRONS_TARGET          = 23416,

    POINT_ID_PORTAL_FRONT           = 0,
    POINT_ID_PORTAL                 = 1,
};

static const float afRethhedronPos[2][3] =
{
    { -1502.39f, 9772.33f, 200.421f},
    { -1557.93f, 9834.34f, 200.949f}
};

struct npc_rethhedronAI : public ScriptedAI
{
    npc_rethhedronAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    uint32 CrippleTimer;
    uint32 ShadowBoltTimer;
    uint32 AbyssalTossTimer;
    uint32 DelayTimer;
    uint64 playerGuid;

    bool LowHpYell;
    bool EventFinished;

    void Reset()
    {
        CrippleTimer     = urand(5000, 9000);
        ShadowBoltTimer  = urand(1000, 3000);
        AbyssalTossTimer = 0;
        DelayTimer       = 0;
        playerGuid       = 0;

        LowHpYell        = false;
        EventFinished    = false;
    }

    void AttackStart(Unit* pWho)
    {
        if (me->Attack(pWho, true))
        {
            me->AddThreat(pWho, 0.0f);
            me->SetInCombatWith(pWho);
            pWho->SetInCombatWith(me);
            DoStartMovement(pWho, 30.0f);
            playerGuid = pWho->GetGUID();
        }
        ScriptedAI::AttackStartNoMove(pWho, CHECK_TYPE_CASTER);
    }

    void DamageTaken(Unit* /*pDealer*/, uint32& uiDamage)
    {
        // go to epilog at 10% health
        if (!EventFinished && me->GetHealthPercent() < 10.0f)
        {
            me->InterruptNonMeleeSpells(false);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(POINT_ID_PORTAL_FRONT, afRethhedronPos[0][0], afRethhedronPos[0][1], afRethhedronPos[0][2]);
            EventFinished = true;
        }

        // npc is not allowed to die
        if (me->GetHealth() && me->GetHealth() <= uiDamage)
            uiDamage = me->GetHealth() - 1;
    }

    void MovementInform(uint32 uiMoveType, uint32 uiPointId)
    {
        if (uiMoveType != POINT_MOTION_TYPE)
            return;

        if (uiPointId == POINT_ID_PORTAL_FRONT)
        {
            me->SummonCreature(NPC_RETHHEDRONS_TARGET, -1557.23, 9833.63, 215.107, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
            DoScriptText(SAY_EVENT_END, me);
            DelayTimer = 2000;
        }
        else if (uiPointId == POINT_ID_PORTAL)
        {
            if (Player* pPlayer = Unit::GetPlayerInWorld(playerGuid))
                    pPlayer->GroupEventHappens(11090, me);
            DoCast(me, SPELL_COSMETIC_LEGION_RING, false);
            me->ForcedDespawn(2000);
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() == NPC_RETHHEDRONS_TARGET)
        {
            pSummoned->CastSpell(pSummoned, SPELL_ABYSSAL_IMPACT, true);
            std::list<Creature*> SpellbinderList = FindAllCreaturesWithEntry(NPC_SPELLBINDER, 40);
            if(!SpellbinderList.empty())
            {
                for (std::list<Creature*>::iterator itr = SpellbinderList.begin(); itr != SpellbinderList.end(); ++itr)
                    (*itr)->CastSpell(pSummoned, SPELL_PURPLE_BEAM, false);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (DelayTimer)
        {
            if (DelayTimer <= diff)
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(POINT_ID_PORTAL, afRethhedronPos[1][0], afRethhedronPos[1][1], afRethhedronPos[1][2]);
                DelayTimer = 0;
            }
            else
                DelayTimer -= diff;
        }

        if (!UpdateVictim())
            return;

        if (EventFinished)
            return;

        if (CrippleTimer < diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_CRIPPLE);
            CrippleTimer = urand(20000, 30000);
        }
        else
            CrippleTimer -= diff;

        if (ShadowBoltTimer < diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_SHADOW_BOLT);
            ShadowBoltTimer = urand(3000, 5000);
        }
        else
            ShadowBoltTimer -= diff;

        if (AbyssalTossTimer < diff)
        {
            AddSpellToCast(me->GetVictim(), SPELL_ABYSSAL_TOSS);
            AbyssalTossTimer = urand(500, 2000);
        }
        else
            AbyssalTossTimer -= diff;

        if (!LowHpYell && me->GetHealthPercent() < 40.0f)
        {
            DoScriptText(SAY_LOW_HP, me);
            LowHpYell = true;
        }

        CheckCasterNoMovementInRange(diff, 30.0);
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_rethhedron(Creature* pCreature)
{
    return new npc_rethhedronAI(pCreature);
}

struct npc_18142AI : public ScriptedAI
{
    npc_18142AI(Creature *creature) : ScriptedAI(creature) {}

    void Reset(){}

    void EnterCombat(Unit *who){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == 31927)
        {
            me->SummonCreature(18109, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            me->SummonGameObject(182146, -1842, 6378, 50, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1845, 6383, 56.5, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1839, 6389, 65, 0, 0, 0, 0, 0, 30000);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, me, 0, 5);
        }
    }
};

CreatureAI* GetAI_npc_18142(Creature *creature)
{
    return new npc_18142AI (creature);
}

struct npc_18143AI : public ScriptedAI
{
    npc_18143AI(Creature *creature) : ScriptedAI(creature) {}

    void Reset(){}

    void EnterCombat(Unit *who){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == 31927)
        {
            me->SummonCreature(18109, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            me->SummonGameObject(182146, -1922, 6364, 56, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1919.84, 6361.48, 64, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1915, 6353, 59, 0, 0, 0, 0, 0, 30000);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, me, 0, 5);
        }
    }
};

CreatureAI* GetAI_npc_18143(Creature *creature)
{
    return new npc_18143AI (creature);
}

struct npc_18144AI : public ScriptedAI
{
    npc_18144AI(Creature *creature) : ScriptedAI(creature) {}

    void Reset(){}

    void EnterCombat(Unit *who){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == 31927)
        {
            me->SummonCreature(18109, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            me->SummonGameObject(182146, -1967.25, 6274, 55, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1970.27, 6278.7, 63, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1975, 6279, 57, 0, 0, 0, 0, 0, 30000);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, me, 0, 5);
        }
    }
};

CreatureAI* GetAI_npc_18144(Creature *creature)
{
    return new npc_18144AI (creature);
}

struct npc_18110AI : public ScriptedAI
{
    npc_18110AI(Creature *creature) : ScriptedAI(creature) {}

    void Reset(){}

    void EnterCombat(Unit *who){}

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == 31927)
        {
            me->SummonCreature(18109, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN, 0);
            me->SummonGameObject(182146, -1816.9, 6283.6, 61 , 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1801.4, 6308, 59, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1819.7, 6305.9, 63, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1794.5, 6300.6, 71, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1808.9, 6307, 72, 0, 0, 0, 0, 0, 30000);
            me->SummonGameObject(182146, -1819.5, 6298, 73, 0, 0, 0, 0, 0, 30000);
            me->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_B, me, 0, 5);
        }
    }
};

CreatureAI* GetAI_npc_18110(Creature *creature)
{
    return new npc_18110AI (creature);
}

struct npc_18109AI : public ScriptedAI
{
    npc_18109AI(Creature *creature) : ScriptedAI(creature) {}

    Timer SizeTimer;
    uint32 Size;
    void Reset()
    {
        SizeTimer.Reset(0);
        Size = 0;
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/)
    {
        me->Say(-1200556, LANG_UNIVERSAL, 0);
        if(eventType == 5)
            SizeTimer.Reset(1000);
        else if(eventType == 6)
        {
            me->SetWalk(false);
            me->setActive(true);
            me->LoadPath(1421);
            me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
            me->GetMotionMaster()->Initialize();
            SizeTimer.Reset(1000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(SizeTimer.Expired(diff))
        {
            me->CastSpell(me, 31698, false);
            if(Size >= 10)
            {
                me->CastSpell(me, 31938, false);
                me->DisappearAndDie();
            }
            Size++;
            SizeTimer.Reset(1000);
        }
    }

};

CreatureAI* GetAI_npc_18109(Creature *creature)
{
    return new npc_18109AI (creature);
}

#define SPELL_CHAIN_LIGHTNING       16033
#define SPELL_FLAME_SHOCK           39529
#define SPELL_HEALING_WAVE          15982
#define SPELL_SUMMON_ICE_TOTEM      18975
#define SPELL_REVIVE_SELF           32343
#define SPELL_FRENZY                28747

struct npc_18069AI : public ScriptedAI
{
    npc_18069AI(Creature *creature) : ScriptedAI(creature) {}

    Timer ChainLightningTimer;
    Timer FlameShockTimer;
    Timer HealingWaveTimer;
    Timer ReviveTimer;

    bool isRevived;
    bool isDead;
    bool Frenzy;

    void Reset()
    {
        ChainLightningTimer.Reset(1000);
        FlameShockTimer.Reset(4000);
        HealingWaveTimer.Reset(3000);
        ReviveTimer.Reset(0);

        isRevived = false;
        isDead = false;
        Frenzy = false;

        m_creature->setFaction(35);
        m_creature->SetHomePosition(-715.55, 7928.78, 58.66, 3.95);
        m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
        m_creature->SetAttackTime(BASE_ATTACK, m_creature->GetAttackTime(BASE_ATTACK));
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);

        if (Creature *sum = (Creature*)FindCreature(12141, 100, m_creature))
            sum->ForcedDespawn(1000);
    }

    void EnterCombat(Unit *who) 
    {
        m_creature->CastSpell(m_creature, SPELL_SUMMON_ICE_TOTEM, false);
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if (m_creature->GetHealth() <= damage && !isRevived)
        {
            damage = 0;
            m_creature->SetHealth(0);
            m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
            m_creature->SetAttackTime(OFF_ATTACK, m_creature->GetAttackTime(BASE_ATTACK));
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
            m_creature->InterruptNonMeleeSpells(true);
            m_creature->SetRooted(true);
            isRevived = true;
            isDead = true;

            ReviveTimer = 10000;
        }

        if (isDead)
            damage = 0;
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != POINT_MOTION_TYPE)
            return;

        switch(PointId)
        {
            case 1:
                m_creature->GetMotionMaster()->MovePoint(2, -716.099, 7894.291, 47.874);
                break;
            case 2:
                m_creature->Yell(-1200557, LANG_UNIVERSAL, 0);
                m_creature->SetHomePosition(-706.67, 7875.01, 45.09, 2.0);
                m_creature->setFaction(14);
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (ReviveTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature, SPELL_REVIVE_SELF, true);
            m_creature->SetHealth(m_creature->GetMaxHealth());
            m_creature->SetPower(POWER_MANA, m_creature->GetMaxPower(POWER_MANA));
            m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
            m_creature->SetAttackTime(BASE_ATTACK, m_creature->GetAttackTime(BASE_ATTACK));
            m_creature->SetStandState(UNIT_STAND_STATE_STAND);
            m_creature->SetRooted(false);
            m_creature->Yell(-1200558, LANG_UNIVERSAL, 0);

            isDead = false;
            ReviveTimer = 0;
        }

        if (isDead)
            return;
        
        if (!Frenzy && isRevived)
        {
            m_creature->CastSpell(m_creature, SPELL_FRENZY, false);
            Frenzy = true;
        }
            
        if (ChainLightningTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_CHAIN_LIGHTNING, false);
            ChainLightningTimer = 10000;
        }

        if (FlameShockTimer.Expired(diff))
        {
            m_creature->CastSpell(m_creature->GetVictim(), SPELL_FLAME_SHOCK, false);
            FlameShockTimer = 11000;
        }

        if (HealingWaveTimer.Expired(diff))
        {
            if (m_creature->HealthBelowPct(60))
            {
                m_creature->CastSpell(m_creature, SPELL_HEALING_WAVE, false);
            }
            HealingWaveTimer = 12000;
        }

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_18069(Creature *creature)
{
    return new npc_18069AI (creature);
}

struct npc_18471AI : public ScriptedAI
{
    npc_18471AI(Creature *creature) : ScriptedAI(creature) {}

    Timer EventTimer;
    uint64 PlayerGUID;
    uint8 Phase;

    void Reset()
    {
        EventTimer.Reset(0);
        Phase = 0;
        PlayerGUID = 0;
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* killer)
    {
        if(pSummoned->GetEntry() == 18398)
        {
            me->Yell(-1200559, LANG_UNIVERSAL, killer->GetGUID());
        }
        if(pSummoned->GetEntry() == 18399)
        {
            if(killer->GetTypeId() == TYPEID_PLAYER)
            {
                if ((((Player*)killer)->GetQuestStatus(9967) == QUEST_STATUS_INCOMPLETE && ((Player*)killer)->GetReqKillOrCastCurrentCount(9967, 18399) == 2) || (((Player*)killer)->GetQuestStatus(9967) == QUEST_STATUS_COMPLETE))
                    me->Yell(-1200560, LANG_UNIVERSAL, killer->GetGUID());
            }
        }
        if(pSummoned->GetEntry() == 18400)
        {
            me->Yell(-1200561, LANG_UNIVERSAL, killer->GetGUID());
        }
        if(pSummoned->GetEntry() == 18401)
        {
            me->Yell(-1200562, LANG_UNIVERSAL, killer->GetGUID());
        }
        if(pSummoned->GetEntry() == 18402)
        {
            me->Yell(-1200563, LANG_UNIVERSAL, killer->GetGUID());
        }
    }

    void EventStart(uint64 pPlayerGUID, uint32 questId)
    {
        switch(questId)
        {
            case 9962:
            {
                if(!FindCreature(18398, 100, me))
                {
                    EventTimer = 2000;
                    Phase = 1;
                    PlayerGUID = pPlayerGUID;
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
                break;
            }
            case 9967:
            {
                if(!FindCreature(18399, 100, me))
                {
                    EventTimer = 2000;
                    Phase = 4;
                    PlayerGUID = pPlayerGUID;
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
                break;
            }
            case 9970:
            {
                if(!FindCreature(18400, 100, me))
                {
                    EventTimer = 2000;
                    Phase = 7;
                    PlayerGUID = pPlayerGUID;
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
                break;
            }
            case 9972:
            {
                if(!FindCreature(18401, 100, me))
                {
                    EventTimer = 2000;
                    Phase = 10;
                    PlayerGUID = pPlayerGUID;
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
                break;
            }
            case 9973:
            {
                if(!FindCreature(18402, 100, me))
                {
                    EventTimer = 2000;
                    Phase = 13;
                    PlayerGUID = pPlayerGUID;
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
                break;
            }
            case 9977:
            {
                if(Unit* ogre = FindCreature(18069, 10, me))
                {
                    EventTimer = 1000;
                    Phase = 16;
                    PlayerGUID = pPlayerGUID;
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(EventTimer.Expired(diff))
        {
            switch(Phase)
            {
                case 0:
                    break;
                case 1:
                    me->Say(-1200564, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 5000;
                    Phase = 2;
                    break;
                case 2:
                    me->Yell(-1200565, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 4000;
                    Phase = 3;
                    break;
                case 3:
                    me->SummonCreature(18398, -704.669, 7871.08, 45.0387, 1.59531, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 180000);
                    me->TextEmote(-1200566, PlayerGUID, true);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    EventTimer = 0;
                    Phase = 0;
                    break;
                case 4:
                    me->Say(-1200567, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 5000;
                    Phase = 5;
                    break;
                case 5:
                    me->Yell(-1200568, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 4000;
                    Phase = 6;
                    break;
                case 6:
                    me->SummonCreature(18399, -704.669, 7871.08, 45.0387, 1.59531, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 180000);
                    me->SummonCreature(18399, -708.076, 7870.41, 44.8457, 1.59531, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 180000);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    EventTimer = 0;
                    Phase = 0;
                    break;
                case 7:
                    me->Say(-1200569, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 5000;
                    Phase = 8;
                    break;
                case 8:
                    me->Yell(-1200570, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 4000;
                    Phase = 9;
                    break;
                case 9:
                    me->SummonCreature(18400, -704.669, 7871.08, 45.0387, 1.59531, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 180000);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    EventTimer = 0;
                    Phase = 0;
                    break;
                case 10:
                    me->Say(-1200571, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 5000;
                    Phase = 11;
                    break;
                case 11:
                    me->Yell(-1200572, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 4000;
                    Phase = 12;
                    break;
                case 12:
                    me->SummonCreature(18401, -704.669, 7871.08, 45.0387, 1.59531, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 180000);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    EventTimer = 0;
                    Phase = 0;
                    break;
                case 13:
                    me->Say(-1200573, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 5000;
                    Phase = 14;
                    break;
                case 14:
                    me->Yell(-1200574, LANG_UNIVERSAL, PlayerGUID);
                    EventTimer = 4000;
                    Phase = 15;
                    break;
                case 15:
                    me->SummonCreature(18402, -704.669, 7871.08, 45.0387, 1.59531, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 180000);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    EventTimer = 0;
                    Phase = 0;
                    break;
                case 16:
                    me->Say(-1200575, LANG_UNIVERSAL, 0);
                    EventTimer = 5000;
                    Phase = 17;
                    break;
                case 17:
                    me->Yell(-1200576, LANG_UNIVERSAL, PlayerGUID);
                    if(Unit* ogre = FindCreature(18069, 10, me))
                    {
                        ((Creature*)ogre)->Yell(-1200577, LANG_UNIVERSAL, 0);
                        ogre->GetMotionMaster()->MovePoint(1, -726.096, 7908.643, 51.768);
                    }
                    EventTimer = 3000;
                    Phase = 18;
                    break;
                case 18:
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    EventTimer = 0;
                    Phase = 0;
                    break;
            }
        }
    }

};

CreatureAI* GetAI_npc_18471(Creature *creature)
{
    return new npc_18471AI (creature);
}

bool QuestAccept_npc_18471(Player* player, Creature* creature, const Quest* quest)
{
    switch (quest->GetQuestId())
    {
        case 9962:
        case 9967:
        case 9970:
        case 9972:
        case 9973:
        case 9977:
            ((npc_18471AI*)(creature->AI()))->EventStart(player->GetGUID(), quest->GetQuestId());
            break;
        default:
            return false;
    }

    return true;
}


struct npc_19667AI : public ScriptedAI
{
    npc_19667AI(Creature *creature) : ScriptedAI(creature) {}

    void Reset()
    {
        me->CastSpell(me, 12980, false);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 misc)
    {
        if (eventType == 6)
            DoScriptText(-1811030, me);
    }

    void MovementInform(uint32 MoveType, uint32 PointId)
    {
        if (MoveType != WAYPOINT_MOTION_TYPE)
            return;

        switch (PointId)
        {
            case 0:
                me->CastSpell(me, 12980, false);
                break;
            case 3:
                DoScriptText(RAND(-1811025, -1811026, -1811027, -1811028, -1811029), me);
                SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, me, 10000, 10, 0);
                break;
            case 7:
                me->CastSpell(me, 12980, false);
                me->ForcedDespawn(1000);
                break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_19667(Creature *creature)
{
    return new npc_19667AI(creature);
}

struct npc_18265AI : public ScriptedAI
{
    npc_18265AI(Creature *creature) : ScriptedAI(creature) {}

    void Reset()
    {
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* sender, Unit* invoker, uint32 misc)
    {
        if (eventType == 5)
        {
            SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_B, me, 1000, 10, 0);
            DoScriptText(RAND(-1811031, -1811032, -1811033), me);
        }
    }


    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_18265(Creature *creature)
{
    return new npc_18265AI(creature);
}

struct npc_22342AI : public ScriptedAI
{
    npc_22342AI(Creature *creature) : ScriptedAI(creature) {}

    Timer BeamTimer;
    Timer ArcaneMissilesTimer;
    Timer ChainsOfIceTimer;
    Timer CounterspellTimer;
    bool flee;

    void Reset()
    {
        if (me->GetDBTableGUIDLow() == 78597 || me->GetDBTableGUIDLow() == 78596 ||
            me->GetDBTableGUIDLow() == 78595 || me->GetDBTableGUIDLow() == 78600 ||
            me->GetDBTableGUIDLow() == 78598 || me->GetDBTableGUIDLow() == 78594 ||
            me->GetDBTableGUIDLow() == 78585 || me->GetDBTableGUIDLow() == 78587 ||
            me->GetDBTableGUIDLow() == 78591 || me->GetDBTableGUIDLow() == 78583 ||
            me->GetDBTableGUIDLow() == 78592 || me->GetDBTableGUIDLow() == 78588 ||
            me->GetDBTableGUIDLow() == 78586 || me->GetDBTableGUIDLow() == 78584)
            BeamTimer.Reset(3000);
        else
            BeamTimer.Reset(0);
        ArcaneMissilesTimer.Reset(1000);
        ChainsOfIceTimer.Reset(urand(3000, 7000));
        CounterspellTimer.Reset(urand(6000, 9000));
        flee = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (BeamTimer.Expired(diff))
            {
                if (Unit* beamTarget = GetClosestCreatureWithEntry(me, 22400, 40))
                    me->CastSpell(beamTarget, 39123, false);
                BeamTimer = 60000;
            }
            return;
        }

        if (ArcaneMissilesTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 34446, false);
            ArcaneMissilesTimer = 5000;
        }

        if (ChainsOfIceTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 22744, false);
            ChainsOfIceTimer = urand(10000, 14000);
        }

        if (CounterspellTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 31999, false);
            CounterspellTimer = urand(7000, 9000);
        }

        if (!flee)
        {
            if (me->GetHealthPercent() <= 15)
            {
                me->DoFleeToGetAssistance();
                DoScriptText(-1901007, me);
                flee = true;
            }
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_22342(Creature *creature)
{
    return new npc_22342AI(creature);
}

struct npc_18207AI : public ScriptedAI
{
    npc_18207AI(Creature *creature) : ScriptedAI(creature) {}

    Timer EmotionTimer;

    void Reset()
    {
        if (me->GetDBTableGUIDLow() == 65102 || me->GetDBTableGUIDLow() == 65101 ||
            me->GetDBTableGUIDLow() == 65103 || me->GetDBTableGUIDLow() == 65105 ||
            me->GetDBTableGUIDLow() == 2381318 || me->GetDBTableGUIDLow() == 65106 ||
            me->GetDBTableGUIDLow() == 65107 || me->GetDBTableGUIDLow() == 2381317)
            EmotionTimer.Reset(urand(1000, 5000));
        else
            EmotionTimer.Reset(0);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (EmotionTimer.Expired(diff))
            {
                switch (urand(0, 5))
                {
                case 0:
                    me->HandleEmote(4);
                    break;
                case 1:
                    me->HandleEmote(5);
                    break;
                case 2:
                    me->HandleEmote(14);
                    break;
                case 3:
                    me->HandleEmote(15);
                    break;
                case 4:
                    me->HandleEmote(20);
                    break;
                case 5:
                    me->HandleEmote(45);
                    break;
                }
                EmotionTimer = urand(4000, 8000);
            }
            return;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_18207(Creature *creature)
{
    return new npc_18207AI(creature);
}


struct npc_18202AI : public ScriptedAI
{
    npc_18202AI(Creature *creature) : ScriptedAI(creature) {}

    Timer PutridCloudTimer;
    Timer CorruptedEarthTimer;
    Timer HurricaneTimer;
    Timer ChainLightningTimer;

    void Reset()
    {
        if (me->GetDBTableGUIDLow() == 64994 || me->GetDBTableGUIDLow() == 2381339 ||
            me->GetDBTableGUIDLow() == 2381338 || me->GetDBTableGUIDLow() == 2381337 ||
            me->GetDBTableGUIDLow() == 2381336)
            PutridCloudTimer.Reset(urand(3000, 5000));
        else
            PutridCloudTimer.Reset(0);
        CorruptedEarthTimer.Reset(3000);
        ChainLightningTimer.Reset(1000);
        HurricaneTimer.Reset(6000);
    }

    void EnterCombat(Unit* who)
    {
        me->InterruptNonMeleeSpells(true);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (PutridCloudTimer.Expired(diff))
            {
                if (Unit* cloudTarget = GetClosestCreatureWithEntry(me, 18215, 40))
                    me->CastSpell(cloudTarget, 32087, false);
                PutridCloudTimer = 310000;
            }
            return;
        }

        if (ChainLightningTimer.Expired(diff))
        {
            AddSpellToCast(32132, CAST_TANK, false, true);
            ChainLightningTimer = 12000;
        }

        if (CorruptedEarthTimer.Expired(diff))
        {
            AddSpellToCast(32133, CAST_RANDOM, false, true);
            CorruptedEarthTimer = 10000;
        }

        if (HurricaneTimer.Expired(diff))
        {
            AddSpellToCast(32717, CAST_TANK, false, true);
            HurricaneTimer = 20000;
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_18202(Creature *creature)
{
    return new npc_18202AI(creature);
}

struct npc_21963AI : public ScriptedAI
{
    npc_21963AI(Creature *creature) : ScriptedAI(creature), Summons(me) {}

    SummonList Summons;
    Timer SummonTimer;
    uint8 killed;
    Timer WarStompTimer;
    Timer RainOfFireTimer;
    Timer CrippleTimer;

    void Reset()
    {
        SummonTimer.Reset(2000);
        killed = 0;
        WarStompTimer.Reset(3000);
        RainOfFireTimer.Reset(4000);
        CrippleTimer.Reset(1000);
    }

    void JustSummoned(Creature* sum)
    {
        Summons.Summon(sum);
    }

    void SummonedCreatureDies(Creature* sum, Unit* killer)
    {
        killed++;
        if (killed >= 4)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_PL_SPELL_TARGET | UNIT_FLAG_NOT_SELECTABLE);
            killed = 0;
            SummonTimer = 60000;
        }
    }

    void ReceiveAIEvent(AIEventType event, Creature* sender, Unit* invoker, uint32 misc)
    {
        if (event == 5 && sender->IsTemporarySummon())
            Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
        {
            if (SummonTimer.Expired(diff))
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_PL_SPELL_TARGET | UNIT_FLAG_NOT_SELECTABLE);
                Summons.DespawnAll();
                if (Creature* sumone = me->SummonCreature(21902, -3159.42, 4951.35, -8.98, 4.4, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                    sumone->CastSpell(me, 38014, false);
                if (Creature* sumtwo = me->SummonCreature(21902, -3150.05, 4942.91, -8.91, 3.1, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                    sumtwo->CastSpell(me, 38014, false);
                if (Creature* sumthree = me->SummonCreature(21902, -3168.35, 4943.209, -8.91, 0.15, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                    sumthree->CastSpell(me, 38014, false);
                if (Creature* sumfour = me->SummonCreature(21902, -3158.98, 4934.52, -9.06, 1.46, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000))
                    sumfour->CastSpell(me, 38014, false);
                SummonTimer = 120000;
            }
            return;
        }
        if (WarStompTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 11876, false);
            WarStompTimer = 10000;
        }
        if (RainOfFireTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 31598, false);
            RainOfFireTimer = 18000;
        }
        if (CrippleTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 11443, false);
            CrippleTimer = 15000;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_21963(Creature *creature)
{
    return new npc_21963AI(creature);
}

struct npc_19152AI : public ScriptedAI
{
    npc_19152AI(Creature *creature) : ScriptedAI(creature) {}

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
                std::list<Creature*> guardsList;
                guardsList = FindAllCreaturesWithEntry(18488, 10);
                switch (Phase)
                {
                case 0:
                    me->HandleEmote(25); // point
                    Phase = 1;
                    EventTimer = 2500;
                    break;
                case 1:
                    DoScriptText(-1901060, me);
                    Phase = 2;
                    EventTimer = 4000;
                    break;
                case 2:
                    DoScriptText(-1901061, me);
                    Phase = 3;
                    EventTimer = 3000;
                    break;
                case 3:
                    if (Creature* bKnight = GetClosestCreatureWithEntry(me, 19151, 15))
                        DoScriptText(-1901062, bKnight);
                    Phase = 4;
                    EventTimer = 2500;
                    break;
                case 4:
                    DoScriptText(-1901063, me);
                    me->SetStandState(UNIT_STAND_STATE_KNEEL);
                    Phase = 5;
                    EventTimer = 3000;
                    break;
                case 5:
                    DoScriptText(-1901064, me);
                    Phase = 6;
                    EventTimer = 6000;
                    break;
                case 6:
                    DoScriptText(-1901065, me);
                    Phase = 7;
                    EventTimer = 2000;
                    break;
                case 7:
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    Phase = 8;
                    EventTimer = 4000;
                    break;
                case 8:
                    DoScriptText(-1901066, me);
                    for (std::list<Creature*>::iterator itr = guardsList.begin(); itr != guardsList.end(); ++itr)
                    {
                        DoScriptText(-1901067, (*itr));
                        if (Creature* bKnight = GetClosestCreatureWithEntry(me, 19151, 10))
                            (*itr)->SetFacingTo((*itr)->GetOrientationTo(bKnight));
                    }
                    Phase = 9;
                    EventTimer = 1500;
                    break;
                case 9:
                    for (std::list<Creature*>::iterator itr = guardsList.begin(); itr != guardsList.end(); ++itr)
                    {
                        if (Creature* bKnight = GetClosestCreatureWithEntry(me, 19151, 10))
                            (*itr)->CastSpell(bKnight, 33424, false);
                    }
                    Phase = 10;
                    EventTimer = 1500;
                    break;
                case 10:
                    for (std::list<Creature*>::iterator itr = guardsList.begin(); itr != guardsList.end(); ++itr)
                    {
                        if (Creature* bKnight = GetClosestCreatureWithEntry(me, 19151, 10))
                            (*itr)->CastSpell(bKnight, 33424, false);
                    }
                    Phase = 11;
                    EventTimer = 1500;
                    break;
                case 11:
                    for (std::list<Creature*>::iterator itr = guardsList.begin(); itr != guardsList.end(); ++itr)
                    {
                        if (Creature* bKnight = GetClosestCreatureWithEntry(me, 19151, 10))
                            (*itr)->CastSpell(bKnight, 33424, false);
                    }
                    Phase = 12;
                    EventTimer = 1500;
                    break;
                case 12:
                    for (std::list<Creature*>::iterator itr = guardsList.begin(); itr != guardsList.end(); ++itr)
                    {
                        if (Creature* bKnight = GetClosestCreatureWithEntry(me, 19151, 10))
                            (*itr)->CastSpell(bKnight, 33423, false);
                    }
                    Phase = 13;
                    EventTimer = 1500;
                    break;
                case 13:
                    for (std::list<Creature*>::iterator itr = guardsList.begin(); itr != guardsList.end(); ++itr)
                    {
                        if (Creature* bKnight = GetClosestCreatureWithEntry(me, 19151, 10))
                            (*itr)->CastSpell(bKnight, 33425, false);
                    }
                    Phase = 14;
                    EventTimer = 1500;
                    break;
                case 14:
                    for (std::list<Creature*>::iterator itr = guardsList.begin(); itr != guardsList.end(); ++itr)
                    {
                        (*itr)->AI()->Reset();
                    }
                    Phase = 0;
                    EventTimer = 300000;
                    break;
                }
            }
            return;
        }
        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_19152(Creature *creature)
{
    return new npc_19152AI(creature);
}

struct npc_18180AI : public ScriptedAI
{
    npc_18180AI(Creature *creature) : ScriptedAI(creature), Summons(me)
    {
        GnomeEventTimer.Reset(30000);
        TalbukEventTimer.Reset(240000);
        HaroldTimer.Reset(1000);
        DrinkTimer.Reset(5000);
        Phase = 0;
        countDrink = 0;
        summonDead = 0;
        GnomePhase = 0;
        TalbukEvent = false;
        GnomeEvent = false;
    }

    SummonList Summons;
    Timer GnomeEventTimer;
    Timer TalbukEventTimer;
    Timer HaroldTimer;
    Timer DrinkTimer;

    uint32 Phase;
    uint32 countDrink;
    uint32 summonDead;
    uint32 GnomePhase;

    bool TalbukEvent;
    bool GnomeEvent;

    void Reset()
    {
        me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, true);
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
        if (Creature* fitz = GetClosestCreatureWithEntry(me, 18200, 20))
        {
            fitz->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, true);
            fitz->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
        }
    }

    void JustSummoned(Creature* sum)
    {
        Summons.Summon(sum);
    }

    void SummonedCreatureDies(Creature* sum, Unit* killer)
    {
        if (sum->GetEntry() == 17130)
        {
            summonDead++;
            if (summonDead >= 3)
            {
                summonDead = 0;
                if (Creature* harold = GetClosestCreatureWithEntry(me, 18218, 20))
                    DoScriptText(-1901078, harold);
                TalbukEventTimer = 240000;
                Phase = 0;
                TalbukEvent = false;
            }
        }
        if (sum->GetEntry() == 18297) // gankly dies
        {
            if (Creature* gnome = GetClosestCreatureWithEntry(me, 18294, 30))
            {
                gnome->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                gnome->ForcedDespawn(300000);
                GnomePhase = 0;
                GnomeEvent = false;
                GnomeEventTimer = 1800000; // 30 minutes
            }
        }

        if (sum->GetEntry() == 18294) // gnome dies
        {
            if (Creature* gankly = GetClosestCreatureWithEntry(me, 18297, 30))
            {
                gankly->GetMotionMaster()->MovePoint(103, -1453.1, 6352.33, 37.24);
                gankly->SetHomePosition(-1453.1, 6352.33, 37.24, 3.1);
            }
            GnomeEventTimer = 5000;
        }
    }

    void SummonedMovementInform(Creature* sum, uint32 type, uint32 id)
    {
        if (sum->GetEntry() == 17130) // talbuk
        {
            if (id == 100)
            {
                sum->AI()->AttackStart(me);
                if (Creature* fitz = GetClosestCreatureWithEntry(me, 18200, 20))
                    fitz->AI()->AttackStart(sum);
            }
        }

        if (sum->GetEntry() == 18294) // gnome
        {
            if (id == 100)
            {
                sum->Unmount();
                sum->SetFacingTo(2.41);
                GnomeEventTimer = 2000;
                if (Creature* Gankly = me->SummonCreature(18297, -1455.65, 6367.89, 36.103, 5.38, TEMPSUMMON_DEAD_DESPAWN, 0))
                {
                    Gankly->SetWalk(true);
                    Gankly->GetMotionMaster()->MovePoint(100, -1446.88, 6355.76, 37.27);
                    Gankly->SetHomePosition(-1446.88, 6355.76, 37.27, 3.8);
                }
            }
        }

        if (sum->GetEntry() == 18297) // gankly
        {
            if (id == 100)
            {
                sum->SetWalk(true);
                sum->GetMotionMaster()->MovePoint(101, -1451.101, 6351.72, 37.24);
                sum->SetHomePosition(-1451.101, 6351.72, 37.24, 3.1);
            }
            if (id == 101)
            {
                DoScriptText(-1901080, sum);
                if (Creature* gnome = GetClosestCreatureWithEntry(me, 18294, 20))
                {
                    sum->setFaction(14);
                    sum->AI()->AttackStart(gnome);
                }
                GnomeEventTimer = 2000;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!TalbukEvent && !GnomeEvent)
        {
            if (DrinkTimer.Expired(diff))
            {
                if (countDrink <= 3)
                {
                    me->CastSpell(me, 46583, false);
                    DrinkTimer = 2500;
                    countDrink++;
                }
                else
                    DrinkTimer = 20000;
            }

            if (HaroldTimer.Expired(diff))
            {
                if (Creature* harold = GetClosestCreatureWithEntry(me, 18218, 20))
                    DoScriptText(RAND(-1901068, -1901069, -1901070, -1901071, -1901072), harold);
                HaroldTimer = 30000;
            }
        }

        if (!TalbukEvent)
        {
            if (GnomeEventTimer.Expired(diff))
            {
                switch (GnomePhase)
                {
                case 0:
                    GnomeEvent = true;
                    if (Creature* gnome = me->SummonCreature(18294, -1388.89, 6363.44, 40.09, 3.3, TEMPSUMMON_MANUAL_DESPAWN, 0))
                    {
                        gnome->SetHomePosition(-1452.78, 6350.62, 37.24, 0);
                        gnome->GetMotionMaster()->MovePoint(100, -1452.78, 6350.62, 37.24);
                    }
                    GnomePhase = 1;
                    GnomeEventTimer = 0;
                    break;
                case 1:
                    if (Creature* gnome = GetClosestCreatureWithEntry(me, 18294, 30))
                        DoScriptText(-1901079, gnome);
                    GnomePhase = 2;
                    GnomeEventTimer = 0;
                    break;
                case 2:
                    if (Creature* gnome = GetClosestCreatureWithEntry(me, 18294, 30))
                        DoScriptText(-1901081, gnome);
                    GnomePhase = 3;
                    GnomeEventTimer = 0;
                    break;
                case 3:
                    if (Creature* gankly = GetClosestCreatureWithEntry(me, 18297, 30))
                        DoScriptText(-1901082, gankly);
                    GnomePhase = 4;
                    GnomeEventTimer = 5000;
                    break;
                case 4:
                    if (Creature* gankly = GetClosestCreatureWithEntry(me, 18297, 30))
                        DoScriptText(-1901083, gankly);
                    GnomePhase = 5;
                    GnomeEventTimer = 5000;
                    break;
                case 5:
                    if (Creature* harold = GetClosestCreatureWithEntry(me, 18218, 20))
                        DoScriptText(-1901084, harold);
                    GnomePhase = 6;
                    GnomeEventTimer = 5000;
                    break;
                case 6:
                    if (Creature* fitz = GetClosestCreatureWithEntry(me, 18200, 20))
                        fitz->CastSpell(fitz, 33808, false);
                    me->CastSpell(me, 33808, false);
                    GnomePhase = 7;
                    GnomeEventTimer = 1500;
                    break;
                case 7:
                    if (Creature* gankly = GetClosestCreatureWithEntry(me, 18297, 30))
                        me->Kill(gankly);
                    GnomePhase = 0;
                    GnomeEventTimer = 1800000;
                    GnomeEvent = false;
                    break;
                }
            }
        }

        if (!GnomeEvent)
        {
            if (TalbukEventTimer.Expired(diff))
            {
                switch (Phase)
                {
                case 0:
                    TalbukEvent = true;
                    me->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    if (Creature* fitz = GetClosestCreatureWithEntry(me, 18200, 20))
                    {
                        fitz->SetNonAttackableFlag(UNIT_FLAG_NOT_ATTACKABLE_2, false);
                        fitz->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER | UNIT_NPC_FLAG_GOSSIP);
                    }
                    if (Creature* talO = me->SummonCreature(17130, -1430.108, 6294.71, 45.6, 1.89, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    {
                        talO->GetMotionMaster()->MovePoint(100, -1450.92, 6329.52, 37.7);
                        talO->SetHomePosition(-1450.92, 6329.52, 37.7, 1.97);
                    }
                    if (Creature* talT = me->SummonCreature(17130, -1423.321, 6300.01, 44.17, 2.2, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    {
                        talT->GetMotionMaster()->MovePoint(100, -1448.09, 6332.64, 37.9);
                        talT->SetHomePosition(-1448.09, 6332.64, 37.9, 1.97);
                    }
                    if (Creature* talTh = me->SummonCreature(17130, -1418.306, 6306.95, 41.95, 2.3, TEMPSUMMON_CORPSE_DESPAWN, 0))
                    {
                        talTh->GetMotionMaster()->MovePoint(100, -1445.07, 6334.74, 38.202);
                        talTh->SetHomePosition(-1445.07, 6334.74, 38.202, 1.97);
                    }
                    DoScriptText(-1901073, me);
                    Phase = 1;
                    TalbukEventTimer = 3000;
                    break;
                case 1:
                    if (Creature* fitz = GetClosestCreatureWithEntry(me, 18200, 20))
                    {
                        DoScriptText(-1901074, fitz);
                        fitz->SetByteValue(UNIT_FIELD_BYTES_2, 0, 1);
                        fitz->SetFacingTo(5.16);
                    }
                    me->SetFacingTo(5.1);
                    Phase = 2;
                    TalbukEventTimer = 3000;
                    break;
                case 2:
                    if (Creature* harold = GetClosestCreatureWithEntry(me, 18218, 20))
                        DoScriptText(-1901075, harold);
                    TalbukEventTimer = 3000;
                    Phase = 3;
                    break;
                case 3:
                    if (Creature* harold = GetClosestCreatureWithEntry(me, 18218, 20))
                        DoScriptText(-1901076, harold);
                    TalbukEventTimer = 3000;
                    Phase = 4;
                    break;
                case 4:
                    if (Creature* harold = GetClosestCreatureWithEntry(me, 18218, 20))
                        DoScriptText(-1901077, harold);
                    TalbukEventTimer = 0;
                    Phase = 0;
                    break;
                }
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

};

CreatureAI* GetAI_npc_18180(Creature *creature)
{
    return new npc_18180AI(creature);
}

void AddSC_nagrand()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_18180";
    newscript->GetAI = &GetAI_npc_18180;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_21963";
    newscript->GetAI = &GetAI_npc_21963;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19152";
    newscript->GetAI = &GetAI_npc_19152;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18202";
    newscript->GetAI = &GetAI_npc_18202;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18207";
    newscript->GetAI = &GetAI_npc_18207;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_22342";
    newscript->GetAI = &GetAI_npc_22342;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18265";
    newscript->GetAI = &GetAI_npc_18265;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19667";
    newscript->GetAI = &GetAI_npc_19667;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shattered_rumbler";
    newscript->GetAI = &GetAI_mob_shattered_rumbler;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_ancient_orc_ancestor";
    newscript->GetAI = &GetAI_mob_ancient_orc_ancestor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_lump";
    newscript->GetAI = &GetAI_mob_lump;
    newscript->pGossipHello =  &GossipHello_mob_lump;
    newscript->pGossipSelect = &GossipSelect_mob_lump;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_altruis_the_sufferer";
    newscript->pGossipHello =  &GossipHello_npc_altruis_the_sufferer;
    newscript->pGossipSelect = &GossipSelect_npc_altruis_the_sufferer;
    newscript->pQuestAcceptNPC =  &QuestAccept_npc_altruis_the_sufferer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_greatmother_geyah";
    newscript->pGossipHello =  &GossipHello_npc_greatmother_geyah;
    newscript->pGossipSelect = &GossipSelect_npc_greatmother_geyah;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_lantresor_of_the_blade";
    newscript->pGossipHello =  &GossipHello_npc_lantresor_of_the_blade;
    newscript->pGossipSelect = &GossipSelect_npc_lantresor_of_the_blade;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_creditmarker_visit_with_ancestors";
    newscript->GetAI = &GetAI_npc_creditmarker_visit_with_ancestors;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_sparrowhawk";
    newscript->GetAI = &GetAI_mob_sparrowhawk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_corki_capitive";
    newscript->GetAI = &GetAI_npc_corki_capitiveAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_corki_cage";
    newscript->pGOUse = &go_corki_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_nagrand_captive";
    newscript->GetAI = &GetAI_npc_nagrand_captive;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_nagrand_captive;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_multiphase_disturbance";
    newscript->GetAI = &GetAI_npc_multiphase_disturbance;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_maghar_prison";
    newscript->pGOUse =  &go_maghar_prison;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_maghar_prisoner";
    newscript->GetAI = &GetAI_npc_maghar_prisoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_warmaul_pyre";
    newscript->GetAI = &GetAI_npc_warmaul_pyre;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_fel_cannon";
    newscript->GetAI = &GetAI_npc_fel_cannon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_rethhedron";
    newscript->GetAI = &GetAI_npc_rethhedron;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18142";
    newscript->GetAI = &GetAI_npc_18142;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18143";
    newscript->GetAI = &GetAI_npc_18143;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18144";
    newscript->GetAI = &GetAI_npc_18144;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18110";
    newscript->GetAI = &GetAI_npc_18110;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18109";
    newscript->GetAI = &GetAI_npc_18109;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18471";
    newscript->GetAI = &GetAI_npc_18471;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_18471;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18069";
    newscript->GetAI = &GetAI_npc_18069;
    newscript->RegisterSelf();
}
