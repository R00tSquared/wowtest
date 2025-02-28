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
SDName: Azshara
SD%Complete: 90
SDComment: Quest support: 2744, 3141, 9364, 10994
SDCategory: Azshara
EndScriptData */

/* ContentData
mobs_spitelashes
npc_loramus_thalipedes
mob_rizzle_sprysprocket
mob_depth_charge
mob_spirit_of_azuregos
EndContentData */

#include "precompiled.h"

/*######
## mobs_spitelashes
######*/

struct mobs_spitelashesAI : public ScriptedAI
{
    mobs_spitelashesAI(Creature *c) : ScriptedAI(c) {}

    uint32 morphtimer;
    bool spellhit;

    Timer Ability1Timer;
    Timer Ability2Timer;
    Timer Ability3Timer;

    void Reset()
    {
        morphtimer = 0;
        spellhit = false;
        switch(me->GetEntry())
        {
            case 6190:
                Ability1Timer.Reset(1000);
                Ability2Timer.Reset(0);
                Ability3Timer.Reset(0);
                break;
            case 6193:
                Ability1Timer.Reset(urand(2000, 5000));
                Ability2Timer.Reset(0);
                Ability3Timer.Reset(0);
                break;
            case 12204:
            case 6194:
                Ability1Timer.Reset(urand(1000, 3000));
                Ability2Timer.Reset(0);
                Ability3Timer.Reset(0);
                break;
            case 6197:
            case 12205:
                Ability1Timer.Reset(urand(3000, 5000));
                Ability2Timer.Reset(1000);
                Ability3Timer.Reset(500);
                break;
            case 6196:
                Ability1Timer.Reset(urand(1000, 3000));
                Ability2Timer.Reset(0);
                Ability3Timer.Reset(0);
                break;
            case 6195:
                Ability1Timer.Reset(urand(1000, 2000));
                Ability2Timer.Reset(urand(5000, 6000));
                Ability3Timer.Reset(0);
                break;
            case 7885:
                Ability1Timer.Reset(100);
                Ability2Timer.Reset(urand(5000, 6000));
                Ability3Timer.Reset(0);
                break;
            case 7886:
                Ability1Timer.Reset(urand(1000, 3000));
                Ability2Timer.Reset(urand(3000, 5000));
                Ability3Timer.Reset(0);
                break;
        }
    }

    void EnterCombat(Unit *who) { }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)
    {
        if(!spellhit && Hitter->GetTypeId() == TYPEID_PLAYER && 
            ((Player*)Hitter)->GetQuestStatus(9364) == QUEST_STATUS_INCOMPLETE &&
            (Spellkind->Id == 118 || Spellkind->Id == 12824 || Spellkind->Id == 12825 || Spellkind->Id == 12826))
        {
            spellhit = true;
            DoCast(m_creature, 29124);  //become a sheep
        }
    }

    void UpdateAI(const uint32 diff)
    {
        // we mustn't remove the creature in the same round in which we cast the summon spell, otherwise there will be no summons
        if(spellhit && morphtimer >= 5000)
        {
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->RemoveCorpse();                     //you don't see any corpse on off.
            EnterEvadeMode();                               //spellhit will be set to false
            return;
        }

        // walk 5 seconds before summoning
        if(spellhit && morphtimer < 5000)
        {
            morphtimer += diff;
            if(morphtimer >= 5000)
            {
                DoCast(m_creature,28406);                   //summon copies
                DoCast(m_creature,6924);                    //visual explosion
            }
        }

        if (!UpdateVictim())
            return;

        switch(me->GetEntry())
        {
            case 6190:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 6713, false);
                    Ability1Timer = urand(5000, 8000);
                }
                break;
            case 6193:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 3589, false);
                    Ability1Timer = urand(12000, 16000);
                }
                break;
            case 12204:
            case 6194:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 12548, false);
                    Ability1Timer = urand(10000, 12000);
                }
                break;
            case 6197:
            case 12205:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 12549, false);
                    Ability1Timer = urand(8000, 10000);
                }

                if(Ability2Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 9672, false);
                    Ability2Timer = urand(3000, 5000);
                }

                if(Ability3Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 19514, false);
                    Ability3Timer = urand(15000, 17000);
                }
                break;
            case 6196:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 11976, false);
                    Ability1Timer = urand(3000, 4000);
                }
                break;
            case 6195:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 9672, false);
                    Ability1Timer = urand(2000, 3000);
                }
                if(Ability2Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 3589, false);
                    Ability2Timer = urand(12000, 15000);
                }
                break;
            case 7885:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 22120, false);
                    Ability1Timer = 0;
                }
                if(Ability2Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 38556, false);
                    Ability2Timer = 4000;
                }
                break;
            case 7886:
                if(Ability1Timer.Expired(diff))
                {
                    AddSpellToCast(me, 3443, false);
                    Ability1Timer = urand(15000, 20000);
                }
                if(Ability2Timer.Expired(diff))
                {
                    AddSpellToCast(me->GetVictim(), 12548, false);
                    Ability2Timer = urand(8000, 12000);
                }
                break;
            default: break;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mobs_spitelashes(Creature *_Creature)
{
    return new mobs_spitelashesAI (_Creature);
}

/*######
## npc_loramus_thalipedes
######*/

bool GossipHello_npc_loramus_thalipedes(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(2744) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16244), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    if (player->GetQuestStatus(3141) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16245), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_loramus_thalipedes(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(2744);
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16246), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            player->SEND_GOSSIP_MENU(1813, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16247), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            player->SEND_GOSSIP_MENU(1814, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16248), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
            player->SEND_GOSSIP_MENU(1815, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+23:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16249), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
            player->SEND_GOSSIP_MENU(1816, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+24:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16250), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
            player->SEND_GOSSIP_MENU(1817, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+25:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(3141);
            break;
    }
    return true;
}

/*####
# mob_rizzle_sprysprocket
####*/

#define MOB_DEPTH_CHARGE               23025
#define SPELL_RIZZLE_BLACKJACK         39865
#define SPELL_RIZZLE_ESCAPE            39871
#define SPELL_RIZZLE_FROST_GRENADE     40525
#define SPELL_DEPTH_CHARGE_TRAP        38576
#define SPELL_PERIODIC_DEPTH_CHARGE    39912
#define SPELL_GIVE_SOUTHFURY_MOONSTONE 39886

#define SAY_RIZZLE_START     -1000245
#define SAY_RIZZLE_GRENADE   -1000246
#define SAY_RIZZLE_FINAL     -1000247

#define GOSSIP_GET_MOONSTONE 16251
#define MSG_ESCAPE_NOTICE    -1200313

float WPs[58][4] =
{
//pos_x   pos_y     pos_z    orien
{3691.97, -3962.41, 35.9118, 3.67},
{3675.02, -3960.49, 35.9118, 3.67},
{3653.19, -3958.33, 33.9118, 3.59},
{3621.12, -3958.51, 29.9118, 3.48},
{3604.86, -3963,    29.9118, 3.48},
{3569.94, -3970.25, 29.9118, 3.44},
{3541.03, -3975.64, 29.9118, 3.41},
{3510.84, -3978.71, 29.9118, 3.41},
{3472.7,  -3997.07, 29.9118, 3.35},
{3439.15, -4014.55, 29.9118, 3.29},
{3412.8,  -4025.87, 29.9118, 3.25},
{3384.95, -4038.04, 29.9118, 3.24},
{3346.77, -4052.93, 29.9118, 3.22},
{3299.56, -4071.59, 29.9118, 3.20},
{3261.22, -4080.38, 30.9118, 3.19},
{3220.68, -4083.09, 31.9118, 3.18},
{3187.11, -4070.45, 33.9118, 3.16},
{3162.78, -4062.75, 33.9118, 3.15},
{3136.09, -4050.32, 33.9118, 3.07},
{3119.47, -4044.51, 36.0363, 3.07},
{3098.95, -4019.8,  33.9118, 3.07},
{3073.07, -4011.42, 33.9118, 3.07},
{3051.71, -3993.37, 33.9118, 3.02},
{3027.52, -3978.6,  33.9118, 3.00},
{3003.78, -3960.14, 33.9118, 2.98},
{2977.99, -3941.98, 31.9118, 2.96},
{2964.57, -3932.07, 30.9118, 2.96},
{2947.9,  -3921.31, 29.9118, 2.96},
{2924.91, -3910.8,  29.9118, 2.94},
{2903.04, -3896.42, 29.9118, 2.93},
{2884.75, -3874.03, 29.9118, 2.90},
{2868.19, -3851.48, 29.9118, 2.82},
{2854.62, -3819.72, 29.9118, 2.80},
{2825.53, -3790.4,  29.9118, 2.744},
{2804.31, -3773.05, 29.9118, 2.71},
{2769.78, -3763.57, 29.9118, 2.70},
{2727.23, -3745.92, 30.9118, 2.69},
{2680.12, -3737.49, 30.9118, 2.67},
{2647.62, -3739.94, 30.9118, 2.66},
{2616.6,  -3745.75, 30.9118, 2.64},
{2589.38, -3731.97, 30.9118, 2.61},
{2562.94, -3722.35, 31.9118, 2.56},
{2521.05, -3716.6,  31.9118, 2.55},
{2485.26, -3706.67, 31.9118, 2.51},
{2458.93, -3696.67, 31.9118, 2.51},
{2432,    -3692.03, 31.9118, 2.46},
{2399.59, -3681.97, 31.9118, 2.45},
{2357.75, -3666.6,  31.9118, 2.44},
{2311.99, -3656.88, 31.9118, 2.94},
{2263.41, -3649.55, 31.9118, 3.02},
{2209.05, -3641.76, 31.9118, 2.99},
{2164.83, -3637.64, 31.9118, 3.15},
{2122.42, -3639,    31.9118, 3.21},
{2075.73, -3643.59, 31.9118, 3.22},
{2033.59, -3649.52, 31.9118, 3.42},
{1985.22, -3662.99, 31.9118, 3.42},
{1927.09, -3679.56, 33.9118, 3.42},
{1873.57, -3695.32, 33.9118, 3.44}
};

struct mob_rizzle_sprysprocketAI : public ScriptedAI
{
    mob_rizzle_sprysprocketAI(Creature *c) : ScriptedAI(c) { me->SetIsDistanceToHomeEvadable(false); me->SetAggroRange(90.0f); }

    Timer spellEscape_Timer;
    Timer Teleport_Timer;
    Timer Check_Timer;
    Timer Grenade_Timer;
    Timer Must_Die_Timer;
    uint32 CurrWP;

    uint64 PlayerGUID;

    bool Must_Die;
    bool Escape;
    bool ContinueWP;
    bool Reached;

    void Reset()
    {
        spellEscape_Timer.Reset(1300);
        Teleport_Timer.Reset(3500);
        Check_Timer.Reset(1000);
        Grenade_Timer.Reset(30000);
        Must_Die_Timer.Reset(3000);
        CurrWP = 0;

        PlayerGUID = 0;

        Must_Die = false;
        Escape = false;
        ContinueWP = false;
        Reached = false;
    }

    void Despawn()
    {
        m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        m_creature->RemoveCorpse();
    }

    void EnterEvadeMode()
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (Must_Die)
        {
            if (Must_Die_Timer.Expired(diff))
            {
                Despawn();
                return;
            }
        }
        if (Reached)
            return;

        if(!Escape)
        {
            if(!PlayerGUID)
                return;

            if(spellEscape_Timer.Expired(diff))
            {
                DoCast(m_creature, SPELL_RIZZLE_ESCAPE, false);
                spellEscape_Timer = 10000;
            }

            if(Teleport_Timer.Expired(diff))
            {
                DoTeleportTo(3706.39, -3969.15, 35.9118, 0);

                //begin swimming and summon depth charges
                me->TextEmote(-1200313, PlayerGUID);
                DoCast(m_creature, SPELL_PERIODIC_DEPTH_CHARGE);
                m_creature->SetLevitate(true);
                m_creature->SetSpeed(MOVE_RUN, 0.85f, true);
                m_creature->SetSpeed(MOVE_SWIM, 0.85f, true);
                m_creature->SetSpeed(MOVE_FLIGHT, 0.85f, true);
                m_creature->GetMotionMaster()->MovementExpired();
                m_creature->GetMotionMaster()->MovePoint(CurrWP, WPs[CurrWP][0], WPs[CurrWP][1], WPs[CurrWP][2]);
                Escape = true;
            }

            return;
        }

        if(ContinueWP)
        {
            m_creature->GetMotionMaster()->MovePoint(CurrWP, WPs[CurrWP][0], WPs[CurrWP][1], WPs[CurrWP][2]);
            ContinueWP = false;
        }

        if(Grenade_Timer.Expired(diff))
        {
            Player *player = (Player *)Unit::GetUnit((*m_creature), PlayerGUID);
            if(player && Reached == false)
            {
               DoScriptText(SAY_RIZZLE_GRENADE, m_creature, player);
               DoCast(player, SPELL_RIZZLE_FROST_GRENADE, true);
            }
            Grenade_Timer = 30000;
        }

        if(Check_Timer.Expired(diff))
        {
            Unit *player = m_creature->GetUnit(PlayerGUID);
            if(!player)
            {
                Despawn();
                return;
            }

            if (((me->GetDistance(player) < 5) || (me->GetDistance(player) < 15 && me->GetPositionX() - 3 < player->GetPositionX())) && !Reached)
            {
                DoScriptText(SAY_RIZZLE_FINAL, m_creature);
                m_creature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                m_creature->setFaction(35);
                m_creature->RemoveAurasDueToSpell(SPELL_PERIODIC_DEPTH_CHARGE);
                me->CombatStop();
                player->CombatStop();
                Reached = true;
            }
            Check_Timer = 500;
        }
    }

    void AttackStart(Unit *who)
    {
        if (!who || PlayerGUID)
            return;

        if(who->GetTypeId() == TYPEID_PLAYER && ((Player *)who)->GetQuestStatus(10994) == QUEST_STATUS_INCOMPLETE)
        {
            PlayerGUID = who->GetGUID();
            DoScriptText(SAY_RIZZLE_START, m_creature);
            DoCast(who, SPELL_RIZZLE_BLACKJACK, false);
            return;
        }
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if ((type != POINT_MOTION_TYPE) || Reached == true)
            return;

        if(id == 57)
        {
            Despawn();
            return;
        }

        ++CurrWP;
        ContinueWP = true;
    }

};

bool GossipHello_mob_rizzle_sprysprocket(Player *player, Creature *_Creature)
{
    if(player->GetQuestStatus(10994) != QUEST_STATUS_INCOMPLETE)
        return true;
    player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_GET_MOONSTONE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    player->SEND_GOSSIP_MENU(10811,_Creature->GetGUID());
    return true;
}

bool GossipSelect_mob_rizzle_sprysprocket(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1 && player->GetQuestStatus(10994) == QUEST_STATUS_INCOMPLETE)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, SPELL_GIVE_SOUTHFURY_MOONSTONE, true);
        ((mob_rizzle_sprysprocketAI*)_Creature->AI())->Must_Die_Timer.Reset(3000);
        ((mob_rizzle_sprysprocketAI*)_Creature->AI())->Must_Die = true;
    }
    return true;
}

CreatureAI* GetAI_mob_rizzle_sprysprocket(Creature *_Creature)
{
    return new mob_rizzle_sprysprocketAI (_Creature);
}

/*####
# mob_depth_charge
####*/

struct mob_depth_chargeAI : public ScriptedAI
{
    mob_depth_chargeAI(Creature *c) : ScriptedAI(c) {}

    bool we_must_die;
    Timer must_die_timer;

    void Reset()
    {
        m_creature->SetLevitate(true);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        we_must_die = false;
        must_die_timer.Reset(1000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (we_must_die)
        {
            if (must_die_timer.Expired(diff))
            {
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                m_creature->RemoveCorpse();
            }
            return;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who)
            return;

        if(who->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(who, 5))
        {
            DoCast(who, SPELL_DEPTH_CHARGE_TRAP);
            we_must_die = true;
            return;
        }
    }

    void AttackStart(Unit *who)
    {
        return;
    }

    void EnterCombat(Unit* who)
    {
        return;
    }
};

CreatureAI* GetAI_mob_depth_charge(Creature *_Creature)
{
    return new mob_depth_chargeAI (_Creature);
}

/*########
# mob_spirit_of_azuregos
#########*/

#define GOSSIP_ITEM_AZUREGOS1  16252
#define GOSSIP_ITEM_AZUREGOS2  16253
#define GOSSIP_ITEM_AZUREGOS3  16254
#define GOSSIP_ITEM_AZUREGOS4  16255
#define GOSSIP_ITEM_AZUREGOS5  16256
#define GOSSIP_ITEM_AZUREGOS6  16257
#define GOSSIP_ITEM_AZUREGOS7  16258
#define GOSSIP_ITEM_AZUREGOS8  16259
#define GOSSIP_ITEM_AZUREGOS9  16260
#define GOSSIP_ITEM_AZUREGOS10 16261
#define GOSSIP_ITEM_AZUREGOS11 16262
#define GOSSIP_ITEM_AZUREGOS12 16263
#define GOSSIP_ITEM_AZUREGOS13 16264
#define AZUREGOS_SAY_BYE       -1230072

bool GossipHello_mob_spirit_of_azuregos(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );
    if(player->GetQuestRewardStatus(8555) && !player->HasItemCount(20949,1))
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1 );
        player->SEND_GOSSIP_MENU(25005, _Creature->GetGUID());
    }
    else
    {
        player->SEND_GOSSIP_MENU(25006, _Creature->GetGUID());
    }
return true;
}

bool GossipSelect_mob_spirit_of_azuregos(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
  switch (action)
  {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(25007, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(25008, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(25009, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(25010, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(25011, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS7), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
            player->SEND_GOSSIP_MENU(25012, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS8), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+8);
            player->SEND_GOSSIP_MENU(25013, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+8:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS9), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+9);
            player->SEND_GOSSIP_MENU(25014, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+9:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS10), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+10);
            player->SEND_GOSSIP_MENU(25015, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+10:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS11), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+11);
            player->SEND_GOSSIP_MENU(25016, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS12), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+12);
            player->SEND_GOSSIP_MENU(25017, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+12:
        {
            ItemPosCountVec dest;
            uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 20949, 1);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = player->StoreNewItem(dest, 20949, true);
                player->SendNewItem(item,1,true,false,true);
            }
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_AZUREGOS13), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+13);
        player->SEND_GOSSIP_MENU(25018, _Creature->GetGUID());
        break;
        }
        case GOSSIP_ACTION_INFO_DEF+13:
            player->CLOSE_GOSSIP_MENU();
            DoScriptText(AZUREGOS_SAY_BYE, _Creature, player);
            break;
  }
return true;
}

/*######
## npc_duke_hydraxis
######*/

#define GOSSIP_AQ 16265
#define GOSSIP_ET 16266

bool GossipHello_npc_duke_hydraxis(Player *player, Creature *_Creature)
{
    if( _Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->GetQuestStatus(6824) == QUEST_STATUS_COMPLETE)
    {
        if (player->GetReputationMgr().GetRank(749) == REP_HONORED)
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_AQ), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        else if (player->GetReputationMgr().GetRank(749) >= REP_REVERED)
            player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ET), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    }

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_duke_hydraxis(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 21357, false);
    }
    else if ( action == GOSSIP_ACTION_INFO_DEF + 1 )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player, 28439, false);
    }

    return true;
}

struct npc_captain_vanessa_beltisAI : public ScriptedAI
{
    npc_captain_vanessa_beltisAI(Creature* c) : ScriptedAI(c)
    {
        EventStarted = false;
    }

    uint32 SummonTimer;
    uint8 Wave;
    bool EventStarted;
    std::list<uint64> AssistantsList;
    uint64 PlayerGUID;
    uint32 failResetTimer;

    void Reset()
    {
        if(!EventStarted)
        {
            PlayerGUID = 0;
            Wave = 0;
            SummonTimer = 0;
            failResetTimer = 0;
        }
    }
    
    void EventStart()
    {
        if(!EventStarted)
        {
            EventStarted = true;
            SummonTimer = 1000;
            me->setFaction(495);
            std::list<Creature*> AssistantsListPtr = FindAllCreaturesWithEntry(8386, 15);
            if(Creature* Blazen = (Creature*)FindCreature(8378, 15, me))
                AssistantsListPtr.push_back(Blazen);
            if(Creature* Mills = (Creature*)FindCreature(8382, 15, me))
                AssistantsListPtr.push_back(Mills);
            if(Creature* Lindros = (Creature*)FindCreature(8381, 15, me))
                AssistantsListPtr.push_back(Lindros);

            for (std::list<Creature*>::iterator itr = AssistantsListPtr.begin(); itr != AssistantsListPtr.end(); ++itr)
            {
                (*itr)->setFaction(495);
                AssistantsList.push_back((*itr)->GetGUID());
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (EventStarted)
        {
            if (SummonTimer <= diff)
            {
                failResetTimer = 300000;
                if(Wave <= 6)
                {
                    Creature* Naga1 = me->SummonCreature(12205, 2379.566162, -5911.434082, 10.732326, 4.436193, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga2 = me->SummonCreature(12204, 2386.224854, -5907.332031, 11.664396, 4.292467, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga3 = me->SummonCreature(12204, 2377.083008, -5907.523926, 11.199107, 4.542221, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Creature* Naga4 = me->SummonCreature(12205, 2380.184326, -5908.753418, 10.815639, 4.581490, TEMPSUMMON_TIMED_DESPAWN, 120000);
                    Naga1->GetMotionMaster()->MovePoint(1, 2375.106201, -5955.277344, 9.555182, 4.495089);
                    Naga2->SetHomePosition(2375.106201, -5955.277344, 9.555182, 4.495089);
                    Naga3->SetHomePosition(2375.106201, -5955.277344, 9.555182, 4.495089);
                    Naga4->SetHomePosition(2375.106201, -5955.277344, 9.555182, 4.495089);
                    Naga1->SetHomePosition(2375.106201, -5955.277344, 9.555182, 4.495089);
                    Naga2->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    Naga3->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    Naga4->GetMotionMaster()->MoveFollow(Naga1, 1.5f, urand(M_PI, M_PI/2));
                    SummonTimer = 45000;
                    Wave++;
                }
                else
                {
                    for (std::list<uint64>::iterator itr = AssistantsList.begin(); itr != AssistantsList.end(); ++itr)
                    {
                        if (Creature* it = me->GetCreature(*itr))
                            it->AI()->Reset();
                    }
                    if(Player* plr = Unit::GetPlayerInWorld(PlayerGUID))
                        plr->AreaExploredOrEventHappens(3382);
                    AssistantsList.clear();
                    SummonTimer = 0;
                    Wave = 0;
                    EventStarted = false;
                    me->AI()->Reset();
                }
            }
            else
                SummonTimer -= diff;
        }

        if (failResetTimer)
        {
            if (failResetTimer > diff)
                failResetTimer -= diff;
            else
            {
                EventStarted = false;
                Reset();
                me->Kill(me);
            }
        }

        if(!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captain_vanessa_beltis(Creature *_Creature)
{
    return new npc_captain_vanessa_beltisAI(_Creature);
}

bool QuestAccept_npc_captain_vanessa_beltis(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == 3382)
    {
        CAST_AI(npc_captain_vanessa_beltisAI, pCreature->AI())->EventStart();
        CAST_AI(npc_captain_vanessa_beltisAI, pCreature->AI())->PlayerGUID = pPlayer->GetGUID();
    }
    return true;
}

/*######
## npc_felhound_tracker
######*/

#define GAMEOBJECT_AZSHARITE_FORMATION  152631

struct npc_felhound_trackerAI : public ScriptedAI
{
    npc_felhound_trackerAI(Creature *creature) : ScriptedAI(creature) {}

    uint32 DespawnTimer;
    uint32 CheckTimer;
    bool checked;

    void Reset()
    {
        DespawnTimer = 1800000;
        CheckTimer = 3000;
        checked = false;
        me->SetSpeed(MOVE_WALK, 0.9);
        me->SetSpeed(MOVE_RUN, 0.9);
        me->SetSpeed(MOVE_FLIGHT, 0.9);
    }

    void ReceiveEmote(Player* pPlayer, uint32 text_emote)
    {
        if(text_emote == TEXTEMOTE_ROAR)
        {
            if(!checked)
            {
				std::list<GameObject*> AzshariteFormationList0;
                std::list<GameObject*> AzshariteFormationList;
				std::list<GameObject*> AzshariteFormationList1;
				std::list<GameObject*> AzshariteFormationList2;
				std::list<GameObject*> AzshariteFormationList3;

                Hellground::AllGameObjectsWithEntryInGrid go_check(GAMEOBJECT_AZSHARITE_FORMATION);
                Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher(AzshariteFormationList, go_check);
                Cell::VisitGridObjects(me, searcher, 100);
				AzshariteFormationList0.merge(AzshariteFormationList);

				Hellground::AllGameObjectsWithEntryInGrid go_check1(152622);
				Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher1(AzshariteFormationList1, go_check1);
				Cell::VisitGridObjects(me, searcher1, 100);
				AzshariteFormationList0.merge(AzshariteFormationList1);

				Hellground::AllGameObjectsWithEntryInGrid go_check2(152621);
				Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher2(AzshariteFormationList2, go_check2);
				Cell::VisitGridObjects(me, searcher2, 100);
				AzshariteFormationList0.merge(AzshariteFormationList2);

				Hellground::AllGameObjectsWithEntryInGrid go_check3(152620);
				Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> searcher3(AzshariteFormationList3, go_check3);
				Cell::VisitGridObjects(me, searcher3, 100);
				AzshariteFormationList0.merge(AzshariteFormationList3);
            
				if (!AzshariteFormationList0.empty())
                {
					for (std::list<GameObject*>::const_iterator i = AzshariteFormationList0.begin(); i != AzshariteFormationList0.end(); ++i)
                    {
                        if (GameObject* go = (*i)->ToGameObject())
                        {
                            if(go->getLootState() == GO_READY)
                            {
                                me->GetMotionMaster()->MovePoint(0, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                                checked = true;
                                break;
                            }
                        }
                    }
                }
                DespawnTimer = 240000;
            }
            me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
        }
    }
    void UpdateAI(const uint32 diff)
    {
        if(DespawnTimer < diff)
        {
            me->ForcedDespawn();
        }
        else
            DespawnTimer -= diff;
    }
};
CreatureAI* GetAI_npc_felhound_tracker(Creature *creature)
{
    return new npc_felhound_trackerAI (creature);
}

void AddSC_azshara()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mobs_spitelashes";
    newscript->GetAI = &GetAI_mobs_spitelashes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_loramus_thalipedes";
    newscript->pGossipHello =  &GossipHello_npc_loramus_thalipedes;
    newscript->pGossipSelect = &GossipSelect_npc_loramus_thalipedes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_rizzle_sprysprocket";
    newscript->GetAI = &GetAI_mob_rizzle_sprysprocket;
    newscript->pGossipHello =  &GossipHello_mob_rizzle_sprysprocket;
    newscript->pGossipSelect = &GossipSelect_mob_rizzle_sprysprocket;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_depth_charge";
    newscript->GetAI = &GetAI_mob_depth_charge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_spirit_of_azuregos";
    newscript->pGossipHello =  &GossipHello_mob_spirit_of_azuregos;
    newscript->pGossipSelect = &GossipSelect_mob_spirit_of_azuregos;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_duke_hydraxis";
    newscript->pGossipHello = &GossipHello_npc_duke_hydraxis;
    newscript->pGossipSelect = &GossipSelect_npc_duke_hydraxis;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_captain_vanessa_beltis";
    newscript->GetAI = &GetAI_npc_captain_vanessa_beltis;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_captain_vanessa_beltis;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_felhound_tracker";
    newscript->GetAI = &GetAI_npc_felhound_tracker;
    newscript->RegisterSelf();
}