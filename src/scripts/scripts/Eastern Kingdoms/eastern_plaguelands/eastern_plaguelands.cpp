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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 5211, 5742. Special vendor Augustus the Touched
SDCategory: Eastern Plaguelands
EndScriptData */

/* ContentData
mobs_ghoul_flayer
npc_augustus_the_touched
npc_darrowshire_spirit
npc_tirion_fordring
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

//id8530 - cannibal ghoul
//id8531 - gibbering ghoul
//id8532 - diseased flayer

struct mobs_ghoul_flayerAI : public ScriptedAI
{
    mobs_ghoul_flayerAI(Creature *c) : ScriptedAI(c) {}

    void Reset() { }

    void JustDied(Unit* Killer)
    {
        if( Killer->GetTypeId() == TYPEID_PLAYER )
            DoSpawnCreature(11064, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
    }
};

CreatureAI* GetAI_mobs_ghoul_flayer(Creature *_Creature)
{
    return new mobs_ghoul_flayerAI (_Creature);
}

/*######
## npc_augustus_the_touched
######*/

bool GossipHello_npc_augustus_the_touched(Player *player, Creature *_Creature)
{
    if( _Creature->isQuestGiver() )
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( _Creature->isVendor() && player->GetQuestRewardStatus(6164) )
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(),_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_augustus_the_touched(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_TRADE )
        player->SEND_VENDORLIST( _Creature->GetGUID() );
    return true;
}

/*######
## npc_darrowshire_spirit
######*/

#define SPELL_SPIRIT_SPAWNIN    17321

struct npc_darrowshire_spiritAI : public ScriptedAI
{
    npc_darrowshire_spiritAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        DoCast(m_creature,SPELL_SPIRIT_SPAWNIN);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    void EnterCombat(Unit *who) { }

};
CreatureAI* GetAI_npc_darrowshire_spirit(Creature *_Creature)
{
    return new npc_darrowshire_spiritAI (_Creature);
}

bool GossipHello_npc_darrowshire_spirit(Player *player, Creature *_Creature)
{
    player->SEND_GOSSIP_MENU(3873,_Creature->GetGUID());
    player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
    _Creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    return true;
}

/*######
## npc_tirion_fordring
######*/

bool GossipHello_npc_tirion_fordring(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(5742) == QUEST_STATUS_INCOMPLETE && player->getStandState() == UNIT_STAND_STATE_SIT )
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16131), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_tirion_fordring(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16132), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(4493, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16133), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(4494, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16134), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            player->SEND_GOSSIP_MENU(4495, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(5742);
            break;
    }
    return true;
}

#define SPELL_SHOOT             23073//6660
#define NPC_INJURED_PEASANT     14484
#define NPC_PLAGUED_PEASANT     14485
#define NPC_SCOURGE_FOOTSOLDIER 14486
#define SPELL_DEATHS_DOOR       23127
#define SPELL_SEETHING_PLAGUE   23072
#define NPC_THE_CLEANER         14503
#define QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW   7622

struct mobs_scourge_archerAI : public ScriptedAI
{
    mobs_scourge_archerAI(Creature *c) : ScriptedAI(c) 
    {}

    Timer_UnCheked Shoot_Timer;

    void MoveInLineOfSight(Unit * unit)
    {
        if(me->IsInCombat())
            return;

        if(unit->GetEntry() == NPC_INJURED_PEASANT)
            AttackStart(unit);
    }

    void Reset()
    {
        Shoot_Timer.Reset(1);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!me->GetVictim())
            return;

        if (Shoot_Timer.Expired(diff))
        {
            if(Unit * target = GetClosestCreatureWithEntry(me, RAND(NPC_INJURED_PEASANT, NPC_PLAGUED_PEASANT) , 50))
            {
                AddSpellToCast(target, SPELL_SHOOT, true);
                Shoot_Timer = 2000;
            }
        }
         

        CastNextSpellIfAnyAndReady();    
    }
};

CreatureAI* GetAI_mobs_scourge_archer(Creature *_Creature)
{
    return new mobs_scourge_archerAI (_Creature);
}

struct trigger_epic_staffAI : public TriggerAI
{
    trigger_epic_staffAI(Creature *c) : TriggerAI(c) { }

    Timer_UnCheked Summon_Timer;
    Timer_UnCheked Summon_Footsoldier_Timer;
    uint32 Summon_Counter;
    uint32 Counter;
    uint32 FailCounter;
    uint64 priest;

    void Reset()
    {
        Summon_Timer.Reset(1);
        Summon_Footsoldier_Timer.Reset(1);
        Summon_Counter = 0;
        Counter = 0;
        FailCounter = 0;
        priest = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (Summon_Footsoldier_Timer.Expired(diff))
        {
            for(int i = 0; i<3; i++)
            {
                float x,y,z;
                me->GetNearPoint(x,y,z, 0.0f, 7.0f, frand(0, 2*M_PI));
                me->SummonCreature(NPC_SCOURGE_FOOTSOLDIER, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000);
            }
            Summon_Footsoldier_Timer = 20000;
        }
        
        if (Summon_Counter < 6 && Summon_Timer.Expired(diff))
        {
            for(int i = 0; i < 9; i++)
            {
                float x,y,z;
                me->GetNearPoint(x,y,z, 0.0f, 6.0f, frand(0, 2*M_PI));
                me->SummonCreature(NPC_INJURED_PEASANT, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 45000);
            }
            for(int i = 0; i < 3; i++)
            {
                float x,y,z;
                me->GetNearPoint(x,y,z, 0.0f, 6.0f, frand(0, 2*M_PI));
                me->SummonCreature(NPC_PLAGUED_PEASANT, x,y,z, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 45000);
            }
            Summon_Timer = 40000;
            Summon_Counter++;
        }
        

        if ((Counter >= 50 || FailCounter >= 15) && Summon_Counter < 7)
        {
            if(Player * player = (Player*)(me->GetUnit(priest)))
            {
                if(FailCounter <= 15)
                    player->AreaExploredOrEventHappens(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW);
                else
                    player->FailQuest(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW);
            }
            Summon_Counter = 7;
        }
    }

    void SummonedCreatureDespawn(Creature * creature)
    {
        if(creature->GetEntry() == NPC_INJURED_PEASANT || creature->GetEntry() == NPC_PLAGUED_PEASANT)
        { 
            if(creature->isAlive())
                Counter++;
            else
                FailCounter++;
        }
    }

    void VerifyPlayer(Unit * unit)
    {
        Player * player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player)
            return;
            
        if(priest == player->GetGUID())
            return;

        if(player->GetQuestStatus(QUEST_THE_BALANCE_OF_LIGHT_AND_SHADOW) == QUEST_STATUS_INCOMPLETE)
        {
            if(!priest)
            {
                priest = unit->GetGUID();
                return;
            }
        }

        me->SummonCreature(NPC_THE_CLEANER, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
    }
};

CreatureAI* GetAI_trigger_epic_staff(Creature *_Creature)
{
    return new trigger_epic_staffAI (_Creature);
}

struct mobs_Scourge_FootsoldierAI : public ScriptedAI
{
    mobs_Scourge_FootsoldierAI(Creature *c) : ScriptedAI(c) 
    {}

    uint64 Summoner;

    void MoveInLineOfSight(Unit * unit)
    {
        if(me->IsInCombat())
            return;

        if(unit->GetTypeId() == TYPEID_PLAYER)
            AttackStart(unit);
    }

    void IsSummonedBy(Unit * summoner)
    {
        Summoner = summoner->GetGUID();
    }

    void DamageTaken(Unit *pDone_by, uint32 &uiDamage)
    {
        if(Creature * trigger = (Creature*)me->GetUnit(Summoner))
        {
            ((trigger_epic_staffAI*)trigger->AI())->VerifyPlayer(pDone_by);
        }
    }
};

CreatureAI* GetAI_mobs_Scourge_Footsoldier(Creature *_Creature)
{
    return new mobs_Scourge_FootsoldierAI (_Creature);
}

struct mobs_peasantsAI : public ScriptedAI
{
    mobs_peasantsAI(Creature *c) : ScriptedAI(c) 
    {
    }

    Timer_UnCheked DeathsDoor_Timer;
    uint64 Summoner;

    void EnterEvadeMode()
    {
        return;
    }
    void Reset()
    {
        DeathsDoor_Timer.Reset(1);
    }

    void AttackStart(Unit * unit)
    {
        return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (DeathsDoor_Timer.Expired(diff))
        {
            if(!urand(0, 9))
                AddSpellToCast(me, SPELL_DEATHS_DOOR, true);
            DeathsDoor_Timer = 15000;
        }
        

        CastNextSpellIfAnyAndReady();    
    }

    void IsSummonedBy(Unit * summoner)
    {
        Summoner = summoner->GetGUID();
    }

    void SpellHit(Unit * caster, const SpellEntry * spell)
    {
        if(Creature * trigger = (Creature*)me->GetUnit(Summoner))
        {
            ((trigger_epic_staffAI*)trigger->AI())->VerifyPlayer(caster);
        }
    }
};

CreatureAI* GetAI_mobs_peasants(Creature *_Creature)
{
    return new mobs_peasantsAI(_Creature);   
}

struct mobs_plagued_peasantAI : public mobs_peasantsAI
{
    mobs_plagued_peasantAI(Creature *c) : mobs_peasantsAI(c) { }

    Timer_UnCheked SeethingPlague_Timer;

    void Reset()
    {
        SeethingPlague_Timer.Reset(1);
    }

    void UpdateAI(const uint32 diff)
    {
        if (SeethingPlague_Timer.Expired(diff))
        {
            if(!urand(0, 2))
                AddSpellToCast(me, SPELL_SEETHING_PLAGUE, true);
            SeethingPlague_Timer = 10000;
        }
        

        mobs_peasantsAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_mobs_plagued_peasant(Creature *_Creature)
{
    return new mobs_plagued_peasantAI (_Creature);
}

#define MIND_BLAST          17194
#define DOMINATE_MIND       14515
#define SHADOW_WORD_PAIN    17146
#define MIND_FLAY           17165
#define PSYCHIC_SCREAM      13704
#define MAX_TROOPERS        9

struct npc_demetriaAI : public ScriptedAI
{
    npc_demetriaAI(Creature *c) : ScriptedAI(c), summons(c)
    {
    }

    uint32 MindBlastTimer;
    uint32 DominateMindTimer;
    uint32 ShadowWordTimer;
    uint32 MindFlayTimer;
    bool CastedScream;
    std::list<uint64> ScarletTrooperList;
    SummonList summons;

    void InitializeAI()
    {
        me->LoadPath(12339);
        me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
        me->GetMotionMaster()->Initialize();
    }
    void Reset()
    {
        MindBlastTimer      = urand(3000,5000);
        DominateMindTimer   = 6000;
        ShadowWordTimer     = 9000;
        MindFlayTimer       = urand(15000,18000);
        CastedScream = false;
        me->SetSpeed(MOVE_WALK, 1.0f, true);
        me->SetSpeed(MOVE_RUN, 1.0f, true);
    }
    
    void JustSummoned(Creature* summon)
    {
        if(summon)
            summons.Summon(summon);
    }

    void AttackStart(Unit* who)
    {
        ScriptedAI::AttackStart(who);
        for (std::list<uint64>::iterator itr = ScarletTrooperList.begin(); itr != ScarletTrooperList.end(); ++itr)
        {
            if (Creature* it = me->GetCreature(*itr))
            {
                float x, y, z;
                it->GetPosition(x, y, z);
                it->SetHomePosition(x, y, z, 0);
                it->AI()->AttackStart(who);
            }
        }
    }

    void SetFormation()
    {
        uint32 Count = 0;
        for (std::list<uint64>::iterator itr = ScarletTrooperList.begin(); itr != ScarletTrooperList.end(); ++itr)
        {
            if (Creature* it = me->GetCreature(*itr))
            {
                float fAngle = Count < MAX_TROOPERS ? (M_PI * 2) / MAX_TROOPERS - (Count * 2 * (M_PI * 2) / MAX_TROOPERS) : 0.0f;
                if (it->isAlive())
                {
                    it->GetMotionMaster()->MoveFollow(me, 1.5f, fAngle);
                    it->SetSpeed(MOVE_WALK, 1.1f, true);
                    it->SetSpeed(MOVE_RUN, 1.1f, true);
                }
                ++Count;
            }
        }
    }

    void MovementInform(uint32 type, uint32 data)
    {
        if (type == WAYPOINT_MOTION_TYPE)
        {
            if(data == 14)
            {
                for(int i=0;i<MAX_TROOPERS;i++)
                {
                    if(Creature* ScarletTrooper = me->SummonCreature(12352, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0))
                        ScarletTrooperList.push_back(ScarletTrooper->GetGUID());
                }

                if (!ScarletTrooperList.empty())
                    SetFormation();
            }
            if(data == 278)
            {
                summons.DespawnAll();
                me->DisappearAndDie();
            }
        }
    }
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(MindBlastTimer <= diff)
        {
            AddSpellToCast(me, MIND_BLAST, true);
            MindBlastTimer = urand(3000,5000);
        } else MindBlastTimer -= diff;

        if(DominateMindTimer <= diff)
        {
            if(me->getThreatManager().getThreatList().size() >= 2)
                AddSpellToCast(me, DOMINATE_MIND, true);
            DominateMindTimer = urand(8000,15000);
        } else DominateMindTimer -= diff;

        if(ShadowWordTimer <= diff)
        {
            AddSpellToCast(me, SHADOW_WORD_PAIN, true);
            ShadowWordTimer = urand(13000,18000);
        } else ShadowWordTimer -= diff;

        if(MindFlayTimer <= diff)
        {
            AddSpellToCast(me, MIND_FLAY, true);
            MindFlayTimer = urand(15000,18000);
        } else MindFlayTimer -= diff;

        if(HealthBelowPct(30) && !CastedScream)
        {
            AddSpellToCast(me, PSYCHIC_SCREAM, true);
            CastedScream = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_demetria(Creature *_Creature)
{
    return new npc_demetriaAI(_Creature);   
}

/*####
# Quest 9444
####*/

enum quest9444
{
    QUEST_9444          = 9444,
    NPC_GHOST_OF_UTHER  = 17233,
    NPC_HIGH_PRIEST     = 1854,
    SPELL_DEFILLING     = 30098,
    GO_UTHER_LIGHTBEAM  = 182483,
    EVENT_9444_1        = 1,
    EVENT_9444_2        = 2,
    EVENT_9444_3        = 3,
    EVENT_9444_4        = 4,
    EVENT_9444_5        = 5,
    EVENT_9444_6        = 6
};

struct npc_defile_uthers_tomb_triggerAI : public ScriptedAI
{
    npc_defile_uthers_tomb_triggerAI(Creature *c) : ScriptedAI(c) { }

    EventMap events;

    void Reset()
    {
        events.Reset();
    }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if(spell->Id == SPELL_DEFILLING)
        {
            if(GameObject* UtherLightbeam = FindGameObject(GO_UTHER_LIGHTBEAM, 30, me))
                UtherLightbeam->UseDoorOrButton();
            ((Player*)caster)->CastCreatureOrGO(181653, 0, 30098);
            events.ScheduleEvent(EVENT_9444_1, 1000);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        events.Update(diff);
        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_9444_1:
                {
                    if (Creature* GhostOfUther = GetClosestCreatureWithEntry(me, NPC_GHOST_OF_UTHER, 30))
                    {
                        GhostOfUther->setHover(true);
                        DoScriptText(-1230039, GhostOfUther, 0);
                        events.ScheduleEvent(EVENT_9444_2, 5000);
                        break;
                    }
                }
                case EVENT_9444_2:
                {
                    if (Creature* GhostOfUther = GetClosestCreatureWithEntry(me, NPC_GHOST_OF_UTHER, 30))
                    {
                        DoScriptText(-1230040, GhostOfUther, 0);
                        events.ScheduleEvent(EVENT_9444_3, 10000);
                        break;
                    }
                }
                case EVENT_9444_3:
                {
                    if (Creature* GhostOfUther = GetClosestCreatureWithEntry(me, NPC_GHOST_OF_UTHER, 30))
                    {
                        DoScriptText(-1230041, GhostOfUther, 0);
                        events.ScheduleEvent(EVENT_9444_4, 10000);
                        break;
                    }
                }
                case EVENT_9444_4:
                {
                    if (Creature* GhostOfUther = GetClosestCreatureWithEntry(me, NPC_GHOST_OF_UTHER, 30))
                    {
                        DoScriptText(-1230042, GhostOfUther, 0);
                        events.ScheduleEvent(EVENT_9444_5, 10000);
                        break;
                    }
                }
                case EVENT_9444_5:
                {
                    if(GameObject* UtherLightbeam = FindGameObject(GO_UTHER_LIGHTBEAM, 30, me))
                        UtherLightbeam->UseDoorOrButton();

                    if (Creature* GhostOfUther = GetClosestCreatureWithEntry(me, NPC_GHOST_OF_UTHER, 30))
                        GhostOfUther->DisappearAndDie();
                    break;
                }
            }
        }
        CastNextSpellIfAnyAndReady();
    }
};

CreatureAI* GetAI_npc_defile_uthers_tomb_trigger(Creature *_Creature)
{
    return new npc_defile_uthers_tomb_triggerAI (_Creature);
}


/******************************
*** npc_darrowshire_trigger ***
*** Battle of Darrowshire   ***
*******************************/

struct DarrowshireMove
{
    float X;
    float Y;
    float Z;
    float O;
};

static DarrowshireMove DarrowshireEvent[] =
{
    {1500.04f, -3662.67f, 82.832f, 3.70805f},       // Attacker spawn 1
    {1506.17f, -3686.72f, 82.8769f, 5.75945f},      // Attacker spawn 2
    {1512.81f, -3724.64f, 87.12099f, 1.64164f},     // Attacker spawn 3
    {1537.6f, -3677.1f, 88.7f, 3.14884f},           // Attacker spawn Bloodletter

    {1484.68f, -3668.74f, 80.6953f, 0.236567f},     // Defender spawn 1
    {1493.53f, -3695.01f, 80.1347f, 0.264055f},     // Defender spawn 2
    {1505.28f, -3718.83f, 83.343f, 1.36911f},       // Defender spawn 3
    {1446.8f, -3694.27f, 76.5966f, 0.401503f}       // Defender spawn Davil Lightfire / Captain Redpath / Joseph Redpath
};

enum
{
    // Attacker
    NPC_MARAUDING_CORPSE        = 10951,
    NPC_MARAUDING_SKELETON      = 10952,
    NPC_SERVANT_OF_HORGUS       = 10953,
    NPC_BLOODLETTER             = 10954,
    NPC_HORGUS_THE_RAVAGER      = 10946,
    NPC_MARDUK_THE_BLACK        = 10939,
    NPC_REDPATH_THE_CORRUPTED   = 10938,
    NPC_DARROWSHIRE_BETRAYER    = 10947,

    // Defender
    NPC_DARROWSHIRE_DEFENDER    = 10948,
    NPC_SILVERHAND_DISCIPLE     = 10949,
    NPC_REDPATH_MILITIA         = 10950,
    NPC_DAVIL_LIGHTFIRE         = 10944,
    NPC_CAPTAIN_REDPATH         = 10937,
    NPC_JOSEPH_REDPATH          = 10936,
    NPC_DAVIL_CROKFORD          = 10945,

    NPC_DARROWSHIRE_TRIGGER     = 14495, // Spawned by spell (cf spell_scripts for spell #18987)

    SPELL_SUMMON_MARDUK_THE_BLACK = 18650,

    SAY_HORGUS_DIED             = -1811024,
    SAY_LIGHTFIRE_DIED          = -1811025,
    SAY_REDPATH_DIED            = -1811026,
    SAY_SCOURGE_DEFEATED        = -1811027,
    SAY_MILITIA_RANDOM_1        = -1811028,
    SAY_MILITIA_RANDOM_2        = -1811029,
    SAY_MILITIA_RANDOM_3        = -1811030,
    SAY_MILITIA_RANDOM_4        = -1811031,
    SAY_MILITIA_RANDOM_5        = -1811032,
    SAY_MILITIA_RANDOM_6        = -1811033,
    SAY_MILITIA_RANDOM_7        = -1811034,
    SAY_MILITIA_RANDOM_8        = -1811035,
    SAY_DEFENDER_YELL           = -1811036,
    SAY_LIGHTFIRE_YELL          = -1811037,
    SAY_DAVIL_YELL              = -1811038,
    SAY_HORGUS_YELL             = -1811039,
    SAY_DAVIL_DESPAWN           = -1811040,
    SAY_REDPATH_YELL            = -1811041,
    SAY_REDPATH_CORRUPTED       = -1811042,
    SAY_MARDUK_YELL             = -1811043,

    QUEST_BATTLE_DARROWSHIRE    = 5721,
};

struct npc_darrowshire_triggerAI : public ScriptedAI
{
    explicit npc_darrowshire_triggerAI(Creature* pCreature) : ScriptedAI(pCreature), _cleanupDone(false), _initialized(false)
    {
        DefenderFaction = 113;
        Reset();
        // m_creature->SetCreatureSummonLimit(200);
    }

    uint32 PhaseStep;
    uint32 PhaseTimer;
    uint32 MobTimer[7];
    uint32 DefenderFaction;
    std::list<uint64> summonedMobsList;

    ObjectGuid mardukGuid;
    ObjectGuid redpathGuid;
    ObjectGuid redpathCorruptedGuid;
    ObjectGuid davilGuid;
    ObjectGuid horgusGuid;

    void Reset()
    {
        if(me->GetZoneId() == 139)
        {
            Map::PlayerList const &pl = m_creature->GetMap()->GetPlayers();
            uint32 myArea = m_creature->GetAreaId();
            if (!pl.isEmpty() && myArea)
            {
                for (Map::PlayerList::const_iterator it = pl.begin(); it != pl.end(); ++it)
                {
                    Player* pPlayer =  it->getSource();
                    if (pPlayer && pPlayer->isAlive() && !pPlayer->isGameMaster() && m_creature->IsWithinDist(pPlayer, 20.0f, false))
                    {
                        if (pPlayer->GetQuestStatus(QUEST_BATTLE_DARROWSHIRE) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (pPlayer->GetTeam() == HORDE)
                                DefenderFaction = 85; // Orgrimmar
                            else
                                DefenderFaction = 57; // Ironforge
                            break;
                        }
                    }
                }
            }
            PhaseStep = 0;
            PhaseTimer = 6000;

            MobTimer[0] = 15000;
            MobTimer[1] = 17000;
            MobTimer[2] = MobTimer[3] = MobTimer[4] = MobTimer[5] = MobTimer[6] = 0;
            summonedMobsList.clear();
        }
    }

    bool _cleanupDone;
    bool _initialized;

    void OnRemoveFromWorld()
    {
        if (_cleanupDone || !_initialized)
            return;
        DespawnAll();
    }

    void DespawnGuid(ObjectGuid& g)
    {
        if (Creature* c = m_creature->GetMap()->GetCreature(g))
            c->ForcedDespawn();
        g.Clear();
    }

    void DespawnAll()
    {
        _cleanupDone = true;
        for (int i = 0; i < 7; i++)
            MobTimer[i] = 0;
        PhaseTimer = 0;

        for (std::list<uint64>::const_iterator itr = summonedMobsList.begin(); itr != summonedMobsList.end(); ++itr)
            if (Creature* creature = m_creature->GetMap()->GetCreature(*itr))
                if (creature->isAlive() && creature->GetEntry() != NPC_JOSEPH_REDPATH && creature->GetEntry() != NPC_DAVIL_CROKFORD)
                    creature->ForcedDespawn(5000);

        summonedMobsList.clear();
        DespawnGuid(mardukGuid);
        DespawnGuid(redpathGuid);
        DespawnGuid(redpathCorruptedGuid);
        DespawnGuid(davilGuid);
        DespawnGuid(horgusGuid);
        std::list<Creature*> creatures;
        creatures = FindAllCreaturesWithEntry(NPC_DARROWSHIRE_BETRAYER, 150.0f);
        for (std::list<Creature*>::iterator it = creatures.begin(); it != creatures.end(); ++it)
        {
            if ((*it)->IsInWorld())
            {
                (*it)->DisappearAndDie();
            }
        }
        me->DisappearAndDie();
    }

    void JustSummoned(Creature* summoned)
    {
        if (!summoned)
            return;

        summonedMobsList.push_back(summoned->GetGUID());

        switch (summoned->GetEntry())
        {
            case NPC_DARROWSHIRE_DEFENDER:
            case NPC_SILVERHAND_DISCIPLE:
            case NPC_REDPATH_MILITIA:
                summoned->setFaction(DefenderFaction);
            // no break
            case NPC_MARAUDING_CORPSE:
            case NPC_MARAUDING_SKELETON:
            case NPC_SERVANT_OF_HORGUS:
                summoned->GetMotionMaster()->MoveRandomAroundPoint(summoned->GetPositionX(), summoned->GetPositionY(), summoned->GetPositionZ(), 10);
                break;
            case NPC_BLOODLETTER:
                summoned->SetWalk(true);
                summoned->SetHomePosition(DarrowshireEvent[5].X, DarrowshireEvent[5].Y, DarrowshireEvent[5].Z, DarrowshireEvent[5].O);
                summoned->GetMotionMaster()->MovePoint(0, DarrowshireEvent[5].X, DarrowshireEvent[5].Y, DarrowshireEvent[5].Z, true);
                break;
            case NPC_DAVIL_LIGHTFIRE:
            case NPC_CAPTAIN_REDPATH:
                summoned->setFaction(DefenderFaction);
                summoned->SetWalk(false);
                summoned->SetHomePosition(DarrowshireEvent[4].X, DarrowshireEvent[4].Y, DarrowshireEvent[4].Z, DarrowshireEvent[4].O);
                summoned->GetMotionMaster()->MovePoint(2, DarrowshireEvent[4].X, DarrowshireEvent[4].Y, DarrowshireEvent[4].Z, true);
                break;
            case NPC_MARDUK_THE_BLACK:
                summoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PASSIVE | UNIT_FLAG_NON_ATTACKABLE);
                summoned->ForcedDespawn(12000);
                break;
            default:
                break;
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId)
    {
        if (uiMotionType != POINT_MOTION_TYPE || !pSummoned)
            return;

        switch (pSummoned->GetEntry())
        {
            case NPC_DARROWSHIRE_DEFENDER:
            {
                if (uiPointId == 0)
                    pSummoned->GetMotionMaster()->MoveRandomAroundPoint(pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 10);
                break;
            }
            case NPC_BLOODLETTER:
            {
                switch (uiPointId)
                {
                    case 0:
                        pSummoned->SetWalk(true);
                        pSummoned->GetMotionMaster()->MovePoint(1, DarrowshireEvent[7].X, DarrowshireEvent[7].Y, DarrowshireEvent[7].Z, true);
                        break;
                    case 1:
                        pSummoned->GetMotionMaster()->MoveRandomAroundPoint(pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 10);
                        break;
                    default: break;
                }
            }
            case NPC_DAVIL_LIGHTFIRE:
            case NPC_CAPTAIN_REDPATH:
            {
                switch (uiPointId)
                {
                    case 0:
                        pSummoned->SetWalk(true);
                        pSummoned->GetMotionMaster()->MovePoint(1, DarrowshireEvent[7].X, DarrowshireEvent[7].Y, DarrowshireEvent[7].Z, true);
                        break;
                    case 1:
                        pSummoned->SetWalk(true);
                        pSummoned->GetMotionMaster()->MovePoint(2, DarrowshireEvent[4].X, DarrowshireEvent[4].Y, DarrowshireEvent[4].Z, true);
                        break;
                    case 2:
                        pSummoned->SetWalk(true);
                        pSummoned->GetMotionMaster()->MovePoint(3, DarrowshireEvent[6].X, DarrowshireEvent[6].Y, DarrowshireEvent[6].Z, true);
                        break;
                    case 3:
                        pSummoned->SetWalk(true);
                        pSummoned->GetMotionMaster()->MovePoint(4, DarrowshireEvent[5].X, DarrowshireEvent[5].Y, DarrowshireEvent[5].Z, true);
                        break;
                    case 4:
                        pSummoned->GetMotionMaster()->MoveRandomAroundPoint(pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 5);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* killer)
    {
        if (!pSummoned)
            return;

        switch (pSummoned->GetEntry())
        {
            case NPC_HORGUS_THE_RAVAGER:
            {
                if (Unit* Crea = FindCreature(NPC_DARROWSHIRE_DEFENDER, 100.0f, me))
                    DoScriptText(SAY_HORGUS_DIED, Crea);
                PhaseStep = 3;
                PhaseTimer = 8000;
                break;
            }
            case NPC_DAVIL_LIGHTFIRE:
            {
                if (PhaseStep < 3)
                {
                    // echec de la quete
                    if (Unit* Crea = FindCreature(NPC_DARROWSHIRE_DEFENDER, 100.0f, me))
                        DoScriptText(SAY_LIGHTFIRE_DIED, Crea);
                    DespawnAll();
                }
                break;
            }
            case NPC_CAPTAIN_REDPATH:
            {
                if (PhaseStep < 5)
                {
                    // echec de la quete
                    if (Unit* Crea = FindCreature(NPC_DARROWSHIRE_DEFENDER, 100.0f, me))
                        DoScriptText(SAY_REDPATH_DIED, Crea);
                    DespawnAll();
                }
                break;
            }
            case NPC_REDPATH_THE_CORRUPTED:
            {
                if (Unit* Crea = FindCreature(NPC_DARROWSHIRE_DEFENDER, 100.0f, me))
                    DoScriptText(SAY_SCOURGE_DEFEATED, Crea);
                m_creature->SummonCreature(NPC_JOSEPH_REDPATH, DarrowshireEvent[7].X, DarrowshireEvent[7].Y, DarrowshireEvent[7].Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                m_creature->SummonCreature(NPC_DAVIL_CROKFORD, 1465.43f, -3678.48f, 78.0816f, 0.0402176f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                DespawnAll();
                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if(me->GetZoneId() == 139)
        {
            if (!_initialized)
            {
                // Already summoned ? Do not launch the event twice.
                std::list<Creature*> otherTriggers;
                otherTriggers = FindAllCreaturesWithEntry(NPC_DARROWSHIRE_TRIGGER, 100.0f);
                if (otherTriggers.size() > 1)
                {
                    m_creature->AddObjectToRemoveList();
                    return;
                }
                _initialized = true;
            }
            for (int i = 0; i < 7; i++)
            {
                if (MobTimer[i] && MobTimer[i] <= uiDiff)
                {
                    switch (i)
                    {
                        case 0: // NPC_MARAUDING_CORPSE / NPC_MARAUDING_SKELETON
                        {
                            for (int j = 0; j < 3; j++)
                            {
                                int amount = urand(1, 2);
                                for (int k = 0; k < amount; k++)
                                {
                                    float X, Y, Z;
                                    uint32 entry = urand(0, 1) ? NPC_MARAUDING_CORPSE : NPC_MARAUDING_SKELETON;
                                    m_creature->GetRandomPoint(DarrowshireEvent[j].X, DarrowshireEvent[j].Y, DarrowshireEvent[j].Z, 5.0f, X, Y, Z);
                                    m_creature->SummonCreature(entry, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                                }
                            }
                            MobTimer[i] = 25000;
                            break;
                        }
                        case 1: // NPC_DARROWSHIRE_DEFENDER
                        {
                            for (int j = 4; j < 7; j++)
                            {
                                float X, Y, Z;
                                m_creature->GetRandomPoint(DarrowshireEvent[j].X, DarrowshireEvent[j].Y, DarrowshireEvent[j].Z, 5.0f, X, Y, Z);
                                m_creature->SummonCreature(NPC_DARROWSHIRE_DEFENDER, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                            }
                            MobTimer[i] = 45000;
                            break;
                        }
                        case 2: // NPC_SERVANT_OF_HORGUS
                        {
                            if (PhaseStep != 2)
                            {
                                MobTimer[i] = 0;
                                break;
                            }

                            float X, Y, Z;
                            for (int j = 0; j < 3; j++)
                            {
                                int amount = 0;
                                amount = urand(1, 2);
                                for (int k = 0; k < amount; k++)
                                {
                                    m_creature->GetRandomPoint(DarrowshireEvent[j].X, DarrowshireEvent[j].Y, DarrowshireEvent[j].Z, 5.0f, X, Y, Z);
                                    m_creature->SummonCreature(NPC_SERVANT_OF_HORGUS, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                                }
                            }
                            MobTimer[i] = 35000;
                            break;
                        }
                        case 3: // NPC_SILVERHAND_DISCIPLE
                        {
                            if (PhaseStep <= 2)
                            {
                                MobTimer[i] = 0;
                                break;
                            }

                            for (int j = 4; j < 7; j++)
                            {
                                float X, Y, Z;
                                m_creature->GetRandomPoint(DarrowshireEvent[j].X, DarrowshireEvent[j].Y, DarrowshireEvent[j].Z, 5.0f, X, Y, Z);
                                m_creature->SummonCreature(NPC_SILVERHAND_DISCIPLE, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                            }
                            MobTimer[i] = 45000;
                            break;
                        }
                        case 4: // NPC_BLOODLETTER
                        {
                            for (int j = 0; j < 3; j++)
                            {
                                float X, Y, Z;
                                m_creature->GetRandomPoint(DarrowshireEvent[3].X, DarrowshireEvent[3].Y, DarrowshireEvent[3].Z, 5.0f, X, Y, Z);
                                m_creature->SummonCreature(NPC_BLOODLETTER, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000);
                            }
                            MobTimer[i] = 35000;
                            break;
                        }
                        case 5: // NPC_REDPATH_MILITIA
                        {
                            if (PhaseStep <= 4)
                            {
                                MobTimer[i] = 0;
                                break;
                            }

                            bool yelled = false;
                            for (int j = 4; j < 7; j++)
                            {
                                float X, Y, Z;
                                m_creature->GetRandomPoint(DarrowshireEvent[j].X, DarrowshireEvent[j].Y, DarrowshireEvent[j].Z, 6.0f, X, Y, Z);
                                if (Creature* Militia = m_creature->SummonCreature(NPC_REDPATH_MILITIA, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                                {
                                    if (!yelled)
                                    {
                                        switch (urand(1,8))
                                        {
                                            case 1:
                                                DoScriptText(SAY_MILITIA_RANDOM_1, Militia);
                                                break;
                                            case 2:
                                                DoScriptText(SAY_MILITIA_RANDOM_2, Militia);
                                                break;
                                            case 3:
                                                DoScriptText(SAY_MILITIA_RANDOM_3, Militia);
                                                break;
                                            case 4:
                                                DoScriptText(SAY_MILITIA_RANDOM_4, Militia);
                                                break;
                                            case 5:
                                                DoScriptText(SAY_MILITIA_RANDOM_5, Militia);
                                                break;
                                            case 6:
                                                DoScriptText(SAY_MILITIA_RANDOM_6, Militia);
                                                break;
                                            case 7:
                                                DoScriptText(SAY_MILITIA_RANDOM_7, Militia);
                                                break;
                                            case 8:
                                                DoScriptText(SAY_MILITIA_RANDOM_8, Militia);
                                                break;
                                        }
                                        yelled = true;
                                    }
                                }
                            }
                            MobTimer[i] = 45000;
                            break;
                        }
                        case 6: // gestion patrouille NPC_DAVIL_LIGHTFIRE NPC_BLOODLETTER NPC_CAPTAIN_REDPATH
                        {
                            for (std::list<uint64>::const_iterator itr = summonedMobsList.begin(); itr != summonedMobsList.end(); ++itr)
                            {
                                if (Creature* Crea = m_creature->GetMap()->GetCreature(*itr))
                                {
                                    if (Crea->GetEntry() != NPC_BLOODLETTER && Crea->GetEntry() != NPC_DAVIL_LIGHTFIRE && Crea->GetEntry() != NPC_CAPTAIN_REDPATH)
                                        continue;

                                    if (Crea->isAlive() && !Crea->IsInCombat() && Crea->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                                    {
                                        int point = 0;
                                        int Rand = 0;
                                        point = urand(0, 3);
                                        switch (point)
                                        {
                                            case 0:
                                                Rand = 5;
                                                break;
                                            case 1:
                                                Rand = 7;
                                                break;
                                            case 2:
                                                Rand = 4;
                                                break;
                                            case 3:
                                                Rand = 6;
                                                break;
                                        }
                                        // Crea->GetMotionMaster()->MovePoint(point, DarrowshireEvent[Rand].X, DarrowshireEvent[Rand].Y, DarrowshireEvent[Rand].Z, true);
                                    }
                                }
                            }
                            MobTimer[i] = 5000;
                            break;
                        }
                        default:
                            break;
                    }
                }
                else if (MobTimer[i])
                    MobTimer[i] -= uiDiff;
            }

            if (PhaseTimer && PhaseTimer <= uiDiff)
            {
                switch (PhaseStep)
                {
                    case 0: // pop d'un premier defenseur
                    {
                        if (Creature* Cre = m_creature->SummonCreature(NPC_DARROWSHIRE_DEFENDER, DarrowshireEvent[7].X, DarrowshireEvent[7].Y, DarrowshireEvent[7].Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                        {
                            DoScriptText(SAY_DEFENDER_YELL, Cre);
                            Cre->SetWalk(false);
                            Cre->SetHomePosition(DarrowshireEvent[4].X, DarrowshireEvent[4].Y, DarrowshireEvent[4].Z, DarrowshireEvent[4].O);
                            Cre->GetMotionMaster()->MovePoint(0, DarrowshireEvent[4].X, DarrowshireEvent[4].Y, DarrowshireEvent[4].Z, true);
                            PhaseTimer = urand(120000, 180000);
                            PhaseStep = 1;
                        }
                        break;
                    }
                    case 1: // 2:30 - 3 mn aprиs que Joueur pose le sac
                    {
                        if (Creature* davilLightfire = m_creature->SummonCreature(NPC_DAVIL_LIGHTFIRE, DarrowshireEvent[7].X, DarrowshireEvent[7].Y, DarrowshireEvent[7].Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                        {
                            DoScriptText(SAY_LIGHTFIRE_YELL, davilLightfire);
                            davilGuid = davilLightfire->GetObjectGuid();
                            PhaseTimer = 60000;
                            MobTimer[2] = 4000;
                            MobTimer[3] = 6000;
                            MobTimer[6] = 10000;
                            PhaseStep = 2;
                        }
                        break;
                    }
                    case 2: // Horgus est spawn
                    {
                        Creature* davil = m_creature->GetMap()->GetCreature(davilGuid);
                        if (!davil)
                            break;
                        if (Creature* horgus = m_creature->GetMap()->GetCreature(horgusGuid))
                        {
                            DoScriptText(SAY_DAVIL_YELL, davil);
                            PhaseTimer = 0;
                            break;
                        }

                        float X, Y, Z;
                        m_creature->GetRandomPoint(davil->GetPositionX(), davil->GetPositionY(), davil->GetPositionZ(), 6.0f, X, Y, Z);
                        if (Creature* horgus = m_creature->SummonCreature(NPC_HORGUS_THE_RAVAGER, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                        {
                            horgus->AI()->AttackStart(davil);
                            horgusGuid = horgus->GetObjectGuid();
                            DoScriptText(SAY_HORGUS_YELL, horgus);
                            PhaseTimer = 3000;
                        }
                        break;
                    }
                    case 3: // Horgus the Ravager est tuй, Davil disparait et Redpath pop
                    {
                        if (Creature* davil = m_creature->GetMap()->GetCreature(davilGuid))
                        {
                            davil->ForcedDespawn(2000);
                            DoScriptText(SAY_DAVIL_DESPAWN, davil);
                            PhaseTimer = 10000;
                            break;
                        }

                        if (Creature* redpath = m_creature->SummonCreature(NPC_CAPTAIN_REDPATH, DarrowshireEvent[7].X, DarrowshireEvent[7].Y, DarrowshireEvent[7].Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                        {
                            DoScriptText(SAY_REDPATH_YELL, redpath);
                            redpathGuid = redpath->GetObjectGuid();
                            PhaseTimer = urand(120000, 150000);
                            PhaseStep = 4;
                            MobTimer[4] = 4000;
                            MobTimer[5] = 6000;
                        }
                        break;
                    }
                    case 4: // Marduk spawn, Redpath est tuй et Redpath corrompu pop
                    {
                        Creature* marduk = m_creature->GetMap()->GetCreature(mardukGuid);
                        if (marduk)
                        {
                            if (Creature* redpath = m_creature->GetMap()->GetCreature(redpathGuid))
                            {
                                PhaseStep = 5;
                                PhaseTimer = 0;
                                marduk->DealDamage(redpath, redpath->GetHealth(), DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                                if (Creature* redpathCorrupted = m_creature->SummonCreature(NPC_REDPATH_THE_CORRUPTED, redpath->GetPositionX(), redpath->GetPositionY(), redpath->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                                {
                                    DoScriptText(SAY_REDPATH_CORRUPTED, redpathCorrupted);
                                    redpathCorruptedGuid = redpathCorrupted->GetObjectGuid();
                                }
                            }
                            break;
                        }

                        if (Creature* redpath = m_creature->GetMap()->GetCreature(redpathGuid))
                        {
                            float X, Y, Z;
                            m_creature->GetRandomPoint(redpath->GetPositionX(), redpath->GetPositionY(), redpath->GetPositionZ(), 10.0f, X, Y, Z);
                            if (Creature* marduk = m_creature->SummonCreature(NPC_MARDUK_THE_BLACK, X, Y, Z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                            {
                                DoScriptText(SAY_MARDUK_YELL, marduk);
                                mardukGuid = marduk->GetObjectGuid();
                                PhaseTimer = 5000;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            else if (PhaseTimer)
                PhaseTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_darrowshire_trigger(Creature* pCreature)
{
    return new npc_darrowshire_triggerAI(pCreature);
}

/*************************
*** npc_joseph_redpath ***
*************************/

enum
{
    SAY_JOSEPH_1            = -1900153,
    SAY_PAMELA_1            = -1900154,
    SAY_PAMELA_2            = -1900155,
    SAY_PAMELA_3            = -1900156,
    SAY_JOSEPH_2            = -1900157,
    SAY_PAMELA_4            = -1900158,
    SAY_JOSEPH_3            = -1900159,

    NPC_PAMELA_REDPATH      = 10926
};

struct npc_joseph_redpathAI : public ScriptedAI
{
    explicit npc_joseph_redpathAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        EventStarted = false;
        EventTimer = 0;
        Reset();
    }

    bool EventStarted;
    uint32 EventStep;
    uint32 EventTimer;

    void Reset()
    {
    }

    void BeginEvent()
    {
        if (!EventStarted)
        {
            EventTimer = 30000;
            EventStep = 0;
            EventStarted = true;
        }
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if (uiType != POINT_MOTION_TYPE)
            return;
        
        switch(uiPointId)
        {
            case 0:
            {
                m_creature->GetMotionMaster()->MovePoint(1, 1434.22f, -3668.756f, 76.671f, true);
                break;
            }
            case 1:
            {
                m_creature->GetMotionMaster()->MovePoint(2, 1438.526f, -3632.733f, 78.268f, true);
                DoScriptText(SAY_JOSEPH_1, m_creature);
                EventTimer = 3000;
                break;
            }
            case 2:
            {
                if (Unit* pamela = FindCreature(NPC_PAMELA_REDPATH, 150.0f, me))
                {
                    DoScriptText(SAY_PAMELA_2, pamela);
                    m_creature->SetWalk(false);
                    float x, y, z = 0;
                    m_creature->GetNearPoint(x, y, z, 1.0f);
                    m_creature->GetMotionMaster()->MovePoint(3, x, y, z, true);
                    EventTimer = 0;
                }
                else
                    EventTimer = 1;
                break;
            }
            case 3:
            {
                if (Unit* pamela = FindCreature(NPC_PAMELA_REDPATH, 20.0f, me))
                {
                    m_creature->SetFacingToObject(pamela);
                    pamela->SetFacingToObject(m_creature);
                }
                EventTimer = 2000;
                break;
            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (EventTimer && EventTimer <= uiDiff)
        {
            switch (EventStep)
            {
                case 0:
                {
                    m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    
                    m_creature->GetMotionMaster()->MovePoint(0, 1431.501f, -3684.229f, 75.726f, true);
                    ++EventStep;
                    EventTimer = 0;
                    break;
                }
                case 1:
                {
                    if (Unit* pamela = FindCreature(NPC_PAMELA_REDPATH, 150.0f, me))
                    { 
                        DoScriptText(SAY_PAMELA_1, pamela);
                        pamela->GetMotionMaster()->MovePoint(0, 1450.733f, -3599.974f, 85.621f, true);
                    }
                    ++EventStep;
                    EventTimer = 0;
                    break;
                }
                case 2:
                {
                    if (Unit* pamela = FindCreature(NPC_PAMELA_REDPATH, 150.0f, me))
                    {
                        DoScriptText(SAY_PAMELA_3, pamela);
                    }
                    ++EventStep;
                    EventTimer = 5000;
                    break;
                }
                case 3:
                {
                    DoScriptText(SAY_JOSEPH_2, m_creature);
                    ++EventStep;
                    EventTimer = 3000;
                    break;
                }
                case 4:
                {
                    if (Unit* pamela = FindCreature(NPC_PAMELA_REDPATH, 150.0f, me))
                    {
                        DoScriptText(SAY_PAMELA_4, pamela);
                    }
                    ++EventStep;
                    EventTimer = 4000;
                    break;
                }
                case 5:
                {
                    DoScriptText(SAY_JOSEPH_3, m_creature);
                    m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    m_creature->ForcedDespawn(6000);
                    if (Unit* pamela = FindCreature(NPC_PAMELA_REDPATH, 150.0f, me))
                        ((Creature*)pamela)->ForcedDespawn(4000);
                    EventTimer = 0;
                    break;
                }
                default:
                    break;
            }
        }
        else if (EventTimer)
            EventTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_joseph_redpath(Creature* pCreature)
{
    return new npc_joseph_redpathAI(pCreature);
}

bool GossipHello_npc_joseph_redpath(Player* pPlayer, Creature* pCreature)
{
    pPlayer->SEND_GOSSIP_MENU(10935, pCreature->GetGUID());
    if (pPlayer->GetQuestStatus(QUEST_BATTLE_DARROWSHIRE) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->KilledMonster(10936, 0);
        pPlayer->AreaExploredOrEventHappens(QUEST_BATTLE_DARROWSHIRE);
        pCreature->HandleEmote(EMOTE_ONESHOT_BEG);
        if (npc_joseph_redpathAI* pJosephAI = dynamic_cast<npc_joseph_redpathAI*>(pCreature->AI()))
            pJosephAI->BeginEvent();
    }
    return true;
}

/*****************
*** npc_nathanos
******************/

enum
{
    SPELL_BACKHAND          = 6253,
    SPELL_MULTI_SHOT        = 18651,
    SPELL_PSYCHIC_SCREAM    = 13704,
    SPELL_SHADOW_SHOOT      = 18649,
    SPELL_SHOOT_N           = 16100,
    SPELL_SUMMON_CONQUERED  = 19096
};

struct npc_nathanosAI : public ScriptedAI
{
    explicit npc_nathanosAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        currAddNb=0;//they don't despawn on reset.
        Reset();
    }

    uint32 Backhand_Timer;
    uint32 MultiShot_Timer;
    uint32 PsychicScream_Timer;
    uint32 ShadowShoot_Timer;
    uint32 Shoot_Timer;
    uint8 currAddNb;
    uint8 maxAdds;

    bool Checked;
    uint32 CheckHealth_Timer;
    uint64 PlayerGuids[40];
    uint32 PlayerHealth[40];

    void Reset()
    {
        Backhand_Timer = 8000;
        MultiShot_Timer = 10000;
        PsychicScream_Timer = 12000;
        ShadowShoot_Timer = 6000;
        Shoot_Timer = 4000;

        Checked = false;
        CheckHealth_Timer = 1000;

        for (int i = 0; i < 40; i++)
        {
            PlayerGuids[i] = 0;
            PlayerHealth[i] = 0;
        }
    }

    void EnterCombat(Unit* pWho)
    {
        Map::PlayerList const &pl = m_creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = pl.begin(); it != pl.end(); ++it)
        {
            Player* currPlayer = it->getSource();
            if (currPlayer && m_creature->IsWithinDist(currPlayer, 45.0f, false))
            {
                if (currPlayer->isGameMaster())
                    continue;

                m_creature->AddThreat(currPlayer, 0.0f);
                m_creature->SetInCombatWith(currPlayer);
                currPlayer->SetInCombatWith(m_creature);
            }
        }
    }

    void FillPlayerHealthList()
    {
        for (int i = 0; i < 40; i++)
        {
            PlayerGuids[i] = 0;
            PlayerHealth[i] = 0;
        }

        std::list<HostileReference*>& tList = m_creature->getThreatManager().getThreatList();
        std::list<HostileReference*>::iterator i = tList.begin();
        for(i = tList.begin(); i != tList.end(); i++)
        {
            if (Unit* pPlayer = Unit::GetPlayerInWorld((*i)->getUnitGuid()))
            {
                for (int j = 0; j < 40; j++)
                {
                    if (PlayerGuids[j] == 0)
                    {
                        PlayerGuids[j] = (*i)->getUnitGuid();
                        PlayerHealth[j] = pPlayer->GetHealth();
                        break;
                    }
                }
            }
        }
    }

    bool IsPlayerHealed()
    {
        std::list<HostileReference*>& tList = m_creature->getThreatManager().getThreatList();
        if (tList.empty())
            return false;

        std::list<HostileReference*>::iterator i = tList.begin();
        for(i = tList.begin(); i != tList.end(); i++)
        {
            for (int j = 0; j < 40; j++)
            {
                if (PlayerGuids[j] == (*i)->getUnitGuid())
                {
                    Unit* pPlayer = Unit::GetPlayerInWorld((*i)->getUnitGuid());

                    if (pPlayer && (pPlayer->GetHealth() > PlayerHealth[j]))
                        return true;
                }
            }
        }

        return false;
    }
    void JustSummoned(Creature*)
    {
        currAddNb++;
    }
    void SummonedCreatureDies(Creature* pSummoned, Unit* who)
    {
        currAddNb--;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!Checked)
        {
            FillPlayerHealthList();
            Checked = true;
        }
        else
        {
            if (currAddNb < 50 && IsPlayerHealed())
                DoCast(m_creature, SPELL_SUMMON_CONQUERED);
            Checked = false;
        }

        if (Backhand_Timer < diff)
        {
            DoCast(m_creature->GetVictim(), SPELL_BACKHAND);
            Backhand_Timer = urand(8000, 12000);
        }
        else
            Backhand_Timer -= diff;

        if (PsychicScream_Timer < diff)
        {
            DoCast(m_creature, SPELL_PSYCHIC_SCREAM);
            PsychicScream_Timer = urand(10000, 20000);
        }
        else
            PsychicScream_Timer -= diff;

        if (MultiShot_Timer < diff)
        {
            DoCast(m_creature->GetVictim(), SPELL_MULTI_SHOT);
            MultiShot_Timer = urand(8000, 15000);
            Shoot_Timer = std::max(Shoot_Timer, 1500u);
            ShadowShoot_Timer = std::max(ShadowShoot_Timer, 1500u);
        }
        else
            MultiShot_Timer -= diff;

        if (ShadowShoot_Timer < diff)
        {
            DoCast(m_creature->GetVictim(), SPELL_SHADOW_SHOOT);
            ShadowShoot_Timer = urand(8000, 15000);
            Shoot_Timer = std::max(Shoot_Timer, 1500u);
            MultiShot_Timer = std::max(MultiShot_Timer, 1500u);
        }
        else
            ShadowShoot_Timer -= diff;

        if (Shoot_Timer < diff)
        {
            DoCast(m_creature->GetVictim(), SPELL_SHOOT_N);
            Shoot_Timer = urand(4000, 6000);
            MultiShot_Timer = std::max(MultiShot_Timer, 1500u);
            ShadowShoot_Timer = std::max(ShadowShoot_Timer, 1500u);
        }
        else
            Shoot_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_nathanos(Creature* pCreature)
{
    return new npc_nathanosAI(pCreature);
}

void AddSC_eastern_plaguelands()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_darrowshire_trigger";
    newscript->GetAI = &GetAI_npc_darrowshire_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_joseph_redpath";
    newscript->GetAI = &GetAI_npc_joseph_redpath;
    newscript->pGossipHello = &GossipHello_npc_joseph_redpath;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mobs_ghoul_flayer";
    newscript->GetAI = &GetAI_mobs_ghoul_flayer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_augustus_the_touched";
    newscript->pGossipHello = &GossipHello_npc_augustus_the_touched;
    newscript->pGossipSelect = &GossipSelect_npc_augustus_the_touched;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_darrowshire_spirit";
    newscript->GetAI = &GetAI_npc_darrowshire_spirit;
    newscript->pGossipHello = &GossipHello_npc_darrowshire_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_tirion_fordring";
    newscript->pGossipHello =  &GossipHello_npc_tirion_fordring;
    newscript->pGossipSelect = &GossipSelect_npc_tirion_fordring;
    newscript->RegisterSelf();

    newscript = new Script;                          
    newscript->Name="mobs_scourge_archer";
    newscript->GetAI = &GetAI_mobs_scourge_archer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mobs_Scourge_Footsoldier";
    newscript->GetAI = &GetAI_mobs_Scourge_Footsoldier;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mobs_peasants";
    newscript->GetAI = &GetAI_mobs_peasants;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mobs_plagued_peasant";
    newscript->GetAI = &GetAI_mobs_plagued_peasant;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="trigger_epic_staff";
    newscript->GetAI = &GetAI_trigger_epic_staff;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_demetria";
    newscript->GetAI = &GetAI_npc_demetria;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_defile_uthers_tomb_trigger";
    newscript->GetAI = &GetAI_npc_defile_uthers_tomb_trigger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_nathanos";
    newscript->GetAI = &GetAI_npc_nathanos;
    newscript->RegisterSelf();
}
