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
SDName: Bloodmyst_Isle
SD%Complete: 80
SDComment: Quest support: 9670, 9667, 9756.
SDCategory: Bloodmyst Isle
EndScriptData */

/* ContentData
mob_webbed_creature
npc_captured_sunhawk_agent
npc_exarch_admetius
npc_princess_stillpine
go_princess_stillpine_cage
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## mob_webbed_creature
######*/

//possible creatures to be spawned
const uint32 possibleSpawns[27] = {17322, 17661, 17340, 17352, 17333, 17524, 17348, 17339, 17345, 17353, 17336, 17330, 17321, 17325, 17320, 17342, 17334, 17341, 17338, 17337, 17346, 17344, 17327, 17607, 17608, 17350, 17527};

struct mob_webbed_creatureAI : public ScriptedAI
{
    mob_webbed_creatureAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->GetUnitStateMgr().PushAction(UNIT_ACTION_ROOT);
    }

    void JustDied(Unit* Killer)
    {
        uint32 spawnCreatureID;

        switch(rand()%2)
        {
            case 0:
                spawnCreatureID = 17681;
                if (Killer->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)Killer)->KilledMonster(spawnCreatureID, m_creature->GetGUID());
                break;
            case 1:
                spawnCreatureID = possibleSpawns[rand()%27];
                break;
        }

        if(spawnCreatureID)
            DoSpawnCreature(spawnCreatureID,0,0,0,m_creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
    }
};
CreatureAI* GetAI_mob_webbed_creature(Creature *_Creature)
{
    return new mob_webbed_creatureAI (_Creature);
}

/*######
## npc_captured_sunhawk_agent
######*/

#define C_SUNHAWK_TRIGGER 17974

#define GOSSIP_ITEM1 16277

#define GOSSIP_ITEM2 16278
#define GOSSIP_ITEM3 16279
#define GOSSIP_ITEM4 16280
#define GOSSIP_ITEM5 16281
#define GOSSIP_ITEM6 16282
#define say_captured_sunhawk_agent -1200318

bool GossipHello_npc_captured_sunhawk_agent(Player *player, Creature *_Creature)
{
    if (player->HasAura(31609,1) && player->GetQuestStatus(9756) == QUEST_STATUS_INCOMPLETE)
    {
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(9136, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(9134, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_captured_sunhawk_agent(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(9137, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(9138, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM4), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(9139, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM5), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(9140, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM6), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
            player->SEND_GOSSIP_MENU(9141, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            player->CLOSE_GOSSIP_MENU();
            _Creature->Say(-1200318,LANG_UNIVERSAL,player->GetGUID() );
            player->TalkedToCreature(C_SUNHAWK_TRIGGER, _Creature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_exarch_admetius
######*/

#define GOSSIP_ITEM_EXARCH 16283

bool GossipHello_npc_exarch_admetius(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(player->GetQuestStatus(9756) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_EXARCH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_exarch_admetius(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CastSpell(player, 31609, false);
        player->AddAura(31609, player);
    }
    return true;
}

/*########
## Quest: Saving Princess Stillpine
########*/
struct npc_princess_stillpineAI : public ScriptedAI
{
        npc_princess_stillpineAI(Creature *c) : ScriptedAI(c){}

        Timer FleeTimer;

        void Reset()
        {
            FleeTimer.Reset(0);
        }

        void UpdateAI(const uint32 diff)
        {
            if(FleeTimer.Expired(diff))
                m_creature->ForcedDespawn();  
        }
};

CreatureAI* GetAI_npc_princess_stillpineAI(Creature *_Creature)
{
    return new npc_princess_stillpineAI (_Creature);
}

bool GOUse_go_princess_stillpine_cage(Player* pPlayer, GameObject* pGO)
{
    Unit *Prisoner = FindCreature(17682, 4.0f, pPlayer);
    if(!Prisoner)
        return true;

    if (pGO->GetGoType() == GAMEOBJECT_TYPE_DOOR)
    {
        DoScriptText(-1230010-urand(0, 2), Prisoner, pPlayer);
        pPlayer->CastCreatureOrGO(17682, Prisoner->GetGUID(), 31003);
        ((Creature*)Prisoner)->GetMotionMaster()->MoveFleeing(pPlayer,4000);
        CAST_AI(npc_princess_stillpineAI, ((Creature*)Prisoner)->AI())->FleeTimer = 4000;
    }
        
    return false;
}
 
/*######
## Quest 9759: Ending Their World
######*/

#define SAY_SIRONAS_1       -1200319
#define SAY_LEGOSO_1        -1200320
#define SAY_LEGOSO_2        -1200321
#define SAY_LEGOSO_3        -1200322
#define SAY_LEGOSO_4        -1200323
#define SAY_LEGOSO_5        -1200324
#define SAY_LEGOSO_6        -1200325
#define SAY_LEGOSO_7        -1200326
#define SAY_LEGOSO_8        -1200327
#define SAY_LEGOSO_9        -1200328
#define SAY_LEGOSO_10       -1200329
#define SAY_LEGOSO_11       -1200330
#define SAY_LEGOSO_12       -1200331
#define SAY_LEGOSO_13       -1200332
#define SAY_LEGOSO_14       -1200333
#define SAY_LEGOSO_15       -1200334
#define SAY_LEGOSO_16       -1200335
#define SAY_LEGOSO_17       -1200336
#define SAY_LEGOSO_18       -1200337
#define SAY_LEGOSO_19       -1200338
#define SAY_LEGOSO_20       -1200339
#define SAY_LEGOSO_21       -1200340

enum EndingTheirWorld
{
    SPELL_BLOODMYST_TESLA           = 31611,
    SPELL_SIRONAS_CHANNELING        = 31612,

    SPELL_UPPERCUT                  = 10966,
    SPELL_IMMOLATE                  = 12742,
    SPELL_CURSE_OF_BLOOD            = 8282,

    SPELL_FROST_SHOCK               = 8056,
    SPELL_HEALING_SURGE             = 8004,
    SPELL_SEARING_TOTEM             = 38116,
    SPELL_STRENGTH_OF_EARTH_TOTEM   = 31633,

    NPC_SIRONAS                     = 17678,
    NPC_BLOODMYST_TESLA_COIL        = 17979,
    NPC_LEGOSO                      = 17982,

    GO_DRAENEI_EXPLOSIVES_1         = 182088,
    GO_DRAENEI_EXPLOSIVES_2         = 182091,

    ACTION_SIRONAS_CHANNEL_START    = 1,
    ACTION_SIRONAS_CHANNEL_STOP     = 2,

    ACTION_LEGOSO_SIRONAS_KILLED    = 1,

    EVENT_UPPERCUT                  = 1,
    EVENT_IMMOLATE                  = 2,
    EVENT_CURSE_OF_BLOOD            = 3,

    EVENT_FROST_SHOCK               = 1,
    EVENT_HEALING_SURGE             = 2,
    EVENT_SEARING_TOTEM             = 3,
    EVENT_STRENGTH_OF_EARTH_TOTEM   = 4,

    WP_START                        = 1,
    WP_EXPLOSIVES_FIRST_POINT       = 21,
    WP_EXPLOSIVES_FIRST_PLANT       = 22,
    WP_EXPLOSIVES_FIRST_RUNOFF      = 23,
    WP_EXPLOSIVES_FIRST_DETONATE    = 24,
    WP_DEBUG_1                      = 25,
    WP_DEBUG_2                      = 26,
    WP_SIRONAS_HILL                 = 33,
    WP_EXPLOSIVES_SECOND_BATTLEROAR = 35,
    WP_EXPLOSIVES_SECOND_PLANT      = 36,
    WP_EXPLOSIVES_SECOND_DETONATE   = 37,

    PHASE_NONE                      = 0,
    PHASE_CONTINUE                  = -1,
    PHASE_WP_26                     = 1,
    PHASE_WP_22                     = 2,
    PHASE_PLANT_FIRST_KNEEL         = 3,
    PHASE_PLANT_FIRST_STAND         = 4,
    PHASE_PLANT_FIRST_WORK          = 5,
    PHASE_PLANT_FIRST_FINISH        = 6,
    PHASE_PLANT_FIRST_TIMER_1       = 7,
    PHASE_PLANT_FIRST_TIMER_2       = 8,
    PHASE_PLANT_FIRST_TIMER_3       = 9,
    PHASE_PLANT_FIRST_DETONATE      = 10,
    PHASE_PLANT_FIRST_SPEECH        = 11,
    PHASE_PLANT_FIRST_ROTATE        = 12,
    PHASE_PLANT_FIRST_POINT         = 13,
    PHASE_FEEL_SIRONAS_1            = 14,
    PHASE_FEEL_SIRONAS_2            = 15,
    PHASE_MEET_SIRONAS_ROAR         = 16,
    PHASE_MEET_SIRONAS_TURN         = 17,
    PHASE_MEET_SIRONAS_SPEECH       = 18,
    PHASE_PLANT_SECOND_KNEEL        = 19,
    PHASE_PLANT_SECOND_SPEECH       = 20,
    PHASE_PLANT_SECOND_STAND        = 21,
    PHASE_PLANT_SECOND_FINISH       = 22,
    PHASE_PLANT_SECOND_WAIT         = 23,
    PHASE_PLANT_SECOND_TIMER_1      = 24,
    PHASE_PLANT_SECOND_TIMER_2      = 25,
    PHASE_PLANT_SECOND_TIMER_3      = 26,
    PHASE_PLANT_SECOND_DETONATE     = 27,
    PHASE_FIGHT_SIRONAS_STOP        = 28,
    PHASE_FIGHT_SIRONAS_SPEECH_1    = 29,
    PHASE_FIGHT_SIRONAS_SPEECH_2    = 30,
    PHASE_FIGHT_SIRONAS_START       = 31,
    PHASE_SIRONAS_SLAIN_SPEECH_1    = 32,
    PHASE_SIRONAS_SLAIN_EMOTE_1     = 33,
    PHASE_SIRONAS_SLAIN_EMOTE_2     = 34,
    PHASE_SIRONAS_SLAIN_SPEECH_2    = 35,

    DATA_EVENT_STARTER_GUID         = 0,

    MAX_EXPLOSIVES                  = 5,

    QUEST_ENDING_THEIR_WORLD        = 9759
};

struct Locations
{
    float x, y, z;
};

static Locations ExplosivesPos[2][MAX_EXPLOSIVES] =
{
    {
        {-1954.946, -10654.714, 110.448},
        {-1956.331, -10654.494, 110.869},
        {-1955.906, -10656.221, 110.791},
        {-1957.294, -10656.000, 111.219},
        {-1954.462, -10656.451, 110.404}
    },
    {
        {-1915.137, -10583.651, 178.365},
        {-1914.006, -10582.964, 178.471},
        {-1912.717, -10582.398, 178.658},
        {-1915.056, -10582.251, 178.162},
        {-1913.883, -10581.778, 178.346}
    }
};

/*######
## npc_sironas
######*/

struct npc_sironasAI : public ScriptedAI
{
    npc_sironasAI(Creature* creature) : ScriptedAI(creature) { }

    std::list<uint64> _beamGuidList;
    EventMap _events;

    void Reset()
    {
        _events.Reset();
    }

    void EnterCombat(Unit* /*who*/)
    {
        _events.ScheduleEvent(EVENT_UPPERCUT, 15 * MILLISECONDS);
        _events.ScheduleEvent(EVENT_IMMOLATE, 10 * MILLISECONDS);
        _events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, 5 * MILLISECONDS);
    }

    void JustDied(Unit* killer)
    {
        me->SetFloatValue(OBJECT_FIELD_SCALE_X,1.0f);
        _events.Reset();
        if (Creature* legoso = GetClosestCreatureWithEntry(me, NPC_LEGOSO, 50))
            legoso->AI()->DoAction(ACTION_LEGOSO_SIRONAS_KILLED);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_UPPERCUT:
                    DoCastVictim(SPELL_UPPERCUT);
                    _events.ScheduleEvent(EVENT_UPPERCUT, urand(10, 12) * MILLISECONDS);
                    break;
                case EVENT_IMMOLATE:
                    DoCastVictim(SPELL_IMMOLATE);
                    _events.ScheduleEvent(EVENT_IMMOLATE, urand(15, 20) * MILLISECONDS);
                    break;
                case EVENT_CURSE_OF_BLOOD:
                    DoCastVictim(SPELL_CURSE_OF_BLOOD);
                    _events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, urand(20, 25) * MILLISECONDS);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

    void DoAction(const int32 param)
    {
        switch (param)
        {
            case ACTION_SIRONAS_CHANNEL_START:
            {
                DoCast(me, SPELL_SIRONAS_CHANNELING);
                _beamGuidList.clear();
                std::list<Creature*> BeamList = FindAllCreaturesWithEntry(NPC_BLOODMYST_TESLA_COIL, 50);
                for (std::list<Creature*>::const_iterator itr = BeamList.begin(); itr != BeamList.end(); ++itr)
                {
                    _beamGuidList.push_back((*itr)->GetGUID());
                    (*itr)->CastSpell(*itr, SPELL_BLOODMYST_TESLA, true);
                }
                break;
            }
            case ACTION_SIRONAS_CHANNEL_STOP:
            {
                me->InterruptNonMeleeSpells(true, SPELL_SIRONAS_CHANNELING);
                for (std::list<uint64>::const_iterator itr = _beamGuidList.begin(); itr != _beamGuidList.end(); ++itr)
                    if (Creature* beam = Unit::GetCreature(*me, *itr))
                        beam->InterruptNonMeleeSpells(true, SPELL_BLOODMYST_TESLA);
                break;
            }
            default:
                break;
        }
    }
};

CreatureAI* GetAI_npc_sironasAI(Creature* creature)
{
    return new npc_sironasAI(creature);
}

/*######
## npc_demolitionist_legoso
######*/

struct npc_demolitionist_legosoAI : public npc_escortAI
{
    npc_demolitionist_legosoAI(Creature* creature) : npc_escortAI(creature) { }

    int8 phase;
    uint32 moveTimer;
    uint32 eventStarterGuidLow;
    std::list<uint64> _explosivesGuids;
    EventMap _events;
    
    void ResetEvents()
    {
        _events.Reset();
        _events.ScheduleEvent(EVENT_FROST_SHOCK, 1 * MILLISECONDS);
        _events.ScheduleEvent(EVENT_HEALING_SURGE, 5 * MILLISECONDS);
        _events.ScheduleEvent(EVENT_SEARING_TOTEM, 15 * MILLISECONDS);
        _events.ScheduleEvent(EVENT_STRENGTH_OF_EARTH_TOTEM, 20 * MILLISECONDS);
    }

    uint32 GetData(uint32 id) const
    {
        switch (id)
        {
            case DATA_EVENT_STARTER_GUID:
                return eventStarterGuidLow;
            default:
                return 0;
        }
    }

    void SetData(uint32 data, uint32 value)
    {
        switch (data)
        {
            case DATA_EVENT_STARTER_GUID:
                eventStarterGuidLow = value;
                break;
            default:
                break;
        }
    }

    void Reset()
    {
        phase = PHASE_NONE;
        moveTimer = 0;
        ResetEvents();
        eventStarterGuidLow = 0;

        me->SetCanDualWield(true);
    }

    void UpdateAI(const uint32 diff)
    {
        _events.Update(diff);

        if (UpdateVictim())
        {
            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FROST_SHOCK:
                        DoCastVictim(SPELL_FROST_SHOCK);
                        _events.DelayEvents(1 * MILLISECONDS, 0);
                        _events.ScheduleEvent(EVENT_FROST_SHOCK, urand(10, 15) * MILLISECONDS);
                        break;
                    case EVENT_SEARING_TOTEM:
                        DoCast(me, SPELL_SEARING_TOTEM);
                        _events.DelayEvents(1 * MILLISECONDS, 0);
                        _events.ScheduleEvent(EVENT_SEARING_TOTEM, urand(110, 130) * MILLISECONDS);
                        break;
                    case EVENT_STRENGTH_OF_EARTH_TOTEM:
                        DoCast(me, SPELL_STRENGTH_OF_EARTH_TOTEM);
                        _events.DelayEvents(1 * MILLISECONDS, 0);
                        _events.ScheduleEvent(EVENT_STRENGTH_OF_EARTH_TOTEM, urand(110, 130) * MILLISECONDS);
                        break;
                    case EVENT_HEALING_SURGE:
                    {
                        Unit* target = NULL;
                        if ((me->GetHealthPercent()) < 85)
                            target = me;
                        else if (Player* player = GetPlayerForEscort())
                            if (player->GetHealth()*100 / me->GetMaxHealth() < 85)
                                target = player;
                        if (target)
                        {
                            DoCast(target, SPELL_HEALING_SURGE);
                            _events.ScheduleEvent(EVENT_HEALING_SURGE, 10 * MILLISECONDS);
                        }
                        else
                            _events.ScheduleEvent(EVENT_HEALING_SURGE, 2 * MILLISECONDS);
                        break;
                    }
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

        if (HasEscortState(STATE_ESCORT_NONE))
            return;

        npc_escortAI::UpdateAI(diff);

        if (phase)
        {
            if (moveTimer <= diff)
            {
                switch (phase)
                {
                    case PHASE_WP_26: //debug skip path to point 26, buggy path calculation
                        me->GetMotionMaster()->MovePoint(WP_DEBUG_2, -2021.77f, -10648.8f, 129.903f);
                        moveTimer = 2000;
                        phase = PHASE_CONTINUE;
                        SetCanAttack(true);
                        break;
                    case PHASE_CONTINUE: // continue escort
                        SetEscortPaused(false);
                        moveTimer = 0;
                        phase = PHASE_NONE;
                        break;
                    case PHASE_WP_22: //debug skip path to point 22, buggy path calculation
                        me->GetMotionMaster()->MovePoint(WP_EXPLOSIVES_FIRST_PLANT, -1958.026f, -10660.465f, 111.547f);
                        me->Say(-1200322, LANG_UNIVERSAL, 0);
                        moveTimer = 2000;
                        phase = PHASE_PLANT_FIRST_KNEEL;
                        break;
                    case PHASE_PLANT_FIRST_KNEEL: // plant first explosives stage 1 kneel
                        me->SetStandState(UNIT_STAND_STATE_KNEEL);
                        moveTimer = 10000;
                        phase = PHASE_PLANT_FIRST_STAND;
                        break;
                    case PHASE_PLANT_FIRST_STAND: // plant first explosives stage 1 stand
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        moveTimer = 500;
                        phase = PHASE_PLANT_FIRST_WORK;
                        break;
                    case PHASE_PLANT_FIRST_WORK: // plant first explosives stage 2 work
                        me->Say(-1200323, LANG_UNIVERSAL, GetPlayerForEscort()->GetGUID());
                        moveTimer = 17500;
                        phase = PHASE_PLANT_FIRST_FINISH;
                        break;
                    case PHASE_PLANT_FIRST_FINISH: // plant first explosives finish
                        _explosivesGuids.clear();
                        for (uint8 i = 0; i != MAX_EXPLOSIVES; ++i)
                        {
                            if (GameObject* explosive = me->SummonGameObject(GO_DRAENEI_EXPLOSIVES_1, ExplosivesPos[0][i].x, ExplosivesPos[0][i].y, ExplosivesPos[0][i].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0))
                                _explosivesGuids.push_back(explosive->GetGUID());
                        }
                        me->HandleEmoteCommand(EMOTE_ONESHOT_NONE); // reset anim state
                        // force runoff movement so he will not screw up next waypoint
                        me->GetMotionMaster()->MovePoint(WP_EXPLOSIVES_FIRST_RUNOFF, -1955.6f, -10669.8f, 110.65f);
                        me->Say(-1200324, LANG_UNIVERSAL, 0);
                        moveTimer = 1500;
                        phase = PHASE_CONTINUE;
                        break;
                    case PHASE_PLANT_FIRST_TIMER_1: // first explosives detonate timer 1
                        me->Say(-1200325, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_FIRST_TIMER_2;
                        break;
                    case PHASE_PLANT_FIRST_TIMER_2: // first explosives detonate timer 2
                        me->Say(-1200326, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_FIRST_TIMER_3;
                        break;
                    case PHASE_PLANT_FIRST_TIMER_3: // first explosives detonate timer 3
                        me->Say(-1200327, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_FIRST_DETONATE;
                        break;
                    case PHASE_PLANT_FIRST_DETONATE: // first explosives detonate finish
                        for (std::list<uint64>::iterator itr = _explosivesGuids.begin(); !_explosivesGuids.empty();)
                        {
                            if (GameObject* explosive = GameObject::GetGameObject(*me, *itr))
                                me->RemoveGameObject(explosive, true);
                            itr = _explosivesGuids.erase(itr);
                        }
                        me->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                        moveTimer = 2000;
                        phase = PHASE_PLANT_FIRST_SPEECH;
                        break;
                    case PHASE_PLANT_FIRST_SPEECH: // after detonation 1 speech
                        me->Say(-1200328, LANG_UNIVERSAL, 0);
                        moveTimer = 4000;
                        phase = PHASE_PLANT_FIRST_ROTATE;
                        break;
                    case PHASE_PLANT_FIRST_ROTATE: // after detonation 1 rotate to next point
                        me->SetFacingTo(2.272f);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_FIRST_POINT;
                        break;
                    case PHASE_PLANT_FIRST_POINT: // after detonation 1 send point anim and go on to next point
                        me->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                        moveTimer = 2000;
                        phase = PHASE_CONTINUE;
                        SetCanAttack(true);
                        break;
                    case PHASE_FEEL_SIRONAS_1: // legoso exclamation before sironas 1.1
                        me->Say(-1200329, LANG_UNIVERSAL, 0);
                        moveTimer = 4000;
                        phase = PHASE_FEEL_SIRONAS_2;
                        break;
                    case PHASE_FEEL_SIRONAS_2: // legoso exclamation before sironas 1.2
                        me->Say(-1200330, LANG_UNIVERSAL, GetPlayerForEscort()->GetGUID());
                        moveTimer = 4000;
                        phase = PHASE_CONTINUE;
                        break;
                    case PHASE_MEET_SIRONAS_ROAR: // legoso exclamation before sironas 2.1
                        me->Say(-1200331, LANG_UNIVERSAL, 0);
                        moveTimer = 4000;
                        phase = PHASE_MEET_SIRONAS_TURN;
                        break;
                    case PHASE_MEET_SIRONAS_TURN: // legoso exclamation before sironas 2.2
                        if (Player* player = GetPlayerForEscort())
                            me->SetFacingTo(me->GetOrientationTo(player));
                        moveTimer = 1000;
                        phase = PHASE_MEET_SIRONAS_SPEECH;
                        break;
                    case PHASE_MEET_SIRONAS_SPEECH: // legoso exclamation before sironas 2.3
                        me->Say(-1200332, LANG_UNIVERSAL, 0);
                        moveTimer = 7000;
                        phase = PHASE_CONTINUE;
                        break;
                    case PHASE_PLANT_SECOND_KNEEL: // plant second explosives stage 1 kneel
                        me->SetStandState(UNIT_STAND_STATE_KNEEL);
                        moveTimer = 11000;
                        phase = PHASE_PLANT_SECOND_SPEECH;
                        break;
                    case PHASE_PLANT_SECOND_SPEECH: // plant second explosives stage 2 kneel
                        me->Say(-1200333, LANG_UNIVERSAL, 0);
                        moveTimer = 13000;
                        phase = PHASE_PLANT_SECOND_STAND;
                        break;
                    case PHASE_PLANT_SECOND_STAND: // plant second explosives finish
                        me->SetStandState(UNIT_STAND_STATE_STAND);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_SECOND_FINISH;
                        break;
                    case PHASE_PLANT_SECOND_FINISH: // plant second explosives finish - create explosives
                        _explosivesGuids.clear();
                        for (uint8 i = 0; i != MAX_EXPLOSIVES; ++i)
                        {
                            if (GameObject* explosive = me->SummonGameObject(GO_DRAENEI_EXPLOSIVES_2, ExplosivesPos[1][i].x, ExplosivesPos[1][i].y, ExplosivesPos[1][i].z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0))
                                _explosivesGuids.push_back(explosive->GetGUID());
                        }
                        me->Say(-1200334, LANG_UNIVERSAL, GetPlayerForEscort()->GetGUID());
                        moveTimer = 1000;
                        phase = PHASE_PLANT_SECOND_WAIT;
                        break;
                    case PHASE_PLANT_SECOND_WAIT: // plant second explosives finish - proceed to next point
                        moveTimer = 1000;
                        phase = PHASE_CONTINUE;
                        break;
                    case PHASE_PLANT_SECOND_TIMER_1: // second explosives detonate timer 1
                        me->Say(-1200335, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_SECOND_TIMER_2;
                        break;
                    case PHASE_PLANT_SECOND_TIMER_2: // second explosives detonate timer 2
                        me->Say(-1200336, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_SECOND_TIMER_3;
                        break;
                    case PHASE_PLANT_SECOND_TIMER_3: // second explosives detonate timer 3
                        me->Say(-1200337, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_PLANT_SECOND_DETONATE;
                        break;
                    case PHASE_PLANT_SECOND_DETONATE: // second explosives detonate finish
                        for (std::list<uint64>::iterator itr = _explosivesGuids.begin(); !_explosivesGuids.empty();)
                        {
                            if (GameObject* explosive = GameObject::GetGameObject(*me, *itr))
                                me->RemoveGameObject(explosive, true);
                            itr = _explosivesGuids.erase(itr);
                        }
                        if (Creature* sironas = GetClosestCreatureWithEntry(me, NPC_SIRONAS, 50))
                        {
                            sironas->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_2, false);
                            me->SetFacingTo(me->GetOrientationTo(sironas));
                        }
                        moveTimer = 1000;
                        phase = PHASE_FIGHT_SIRONAS_STOP;
                        SetCanAttack(true);
                        break;
                    case PHASE_FIGHT_SIRONAS_STOP: // sironas channel stop
                        if (Creature* sironas = GetClosestCreatureWithEntry(me, NPC_SIRONAS, 50))
                            sironas->AI()->DoAction(ACTION_SIRONAS_CHANNEL_STOP);
                        moveTimer = 1000;
                        phase = PHASE_FIGHT_SIRONAS_SPEECH_1;
                        break;
                    case PHASE_FIGHT_SIRONAS_SPEECH_1: // sironas exclamation before aggro
                        if (Creature* sironas = GetClosestCreatureWithEntry(me, NPC_SIRONAS, 50))
                            sironas->Say(-1200319, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_FIGHT_SIRONAS_SPEECH_2;
                        break;
                    case PHASE_FIGHT_SIRONAS_SPEECH_2: // legoso exclamation before aggro
                        if (Creature* sironas = GetClosestCreatureWithEntry(me, NPC_SIRONAS, 50))
                            sironas->SetFloatValue(OBJECT_FIELD_SCALE_X, 3.0f);
                        me->Say(-1200338, LANG_UNIVERSAL, 0);
                        moveTimer = 1000;
                        phase = PHASE_FIGHT_SIRONAS_START;
                        break;
                    case PHASE_FIGHT_SIRONAS_START: // legoso exclamation at aggro
                        if (Creature* sironas = GetClosestCreatureWithEntry(me, NPC_SIRONAS, 50))
                        {
                            Unit* target = GetPlayerForEscort();
                            if (!target)
                                target = me;

                            target->AddThreat(sironas, 0.001f);
                            sironas->Attack(target, true);
                            sironas->GetMotionMaster()->MoveChase(target);
                        }
                        moveTimer = 10000;
                        phase = PHASE_CONTINUE;
                        break;
                    case PHASE_SIRONAS_SLAIN_SPEECH_1: // legoso exclamation after battle - stage 1.1
                        me->Say(-1200339, LANG_UNIVERSAL, 0);
                        moveTimer = 2000;
                        phase = PHASE_SIRONAS_SLAIN_EMOTE_1;
                        break;
                    case PHASE_SIRONAS_SLAIN_EMOTE_1: // legoso exclamation after battle - stage 1.2
                        me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
                        moveTimer = 2000;
                        phase = PHASE_SIRONAS_SLAIN_EMOTE_2;
                        break;
                    case PHASE_SIRONAS_SLAIN_EMOTE_2: // legoso exclamation after battle - stage 1.3
                        if (Player* player = GetPlayerForEscort())
                            player->GroupEventHappens(QUEST_ENDING_THEIR_WORLD, me);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                        moveTimer = 5000;
                        phase = PHASE_SIRONAS_SLAIN_SPEECH_2;
                        break;
                    case PHASE_SIRONAS_SLAIN_SPEECH_2: // legoso exclamation after battle - stage 2
                        me->Say(-1200340, LANG_UNIVERSAL, 0);
                        moveTimer = 30000;
                        phase = PHASE_CONTINUE;
                        break;
                    default:
                        break;
                }
            }
            else if (!me->IsInCombat())
                moveTimer -= diff;
        }
    }

    void WaypointReached(uint32 waypointId)
    {
        Player* player = GetPlayerForEscort();
        if (!player)
            return;

        switch (waypointId)
        {
            case WP_START:
                SetEscortPaused(true);
                SetCanAttack(false);
                me->SetFacingTo(me->GetOrientationTo(player));
                me->Say(-1200320, LANG_UNIVERSAL, player->GetGUID());
                moveTimer = 2500;
                phase = PHASE_CONTINUE;
                break;
            case WP_EXPLOSIVES_FIRST_POINT:
                SetEscortPaused(true);
                SetCanAttack(false);
                me->Say(-1200321, LANG_UNIVERSAL, 0);
                moveTimer = 8000;
                phase = PHASE_WP_22;
                break;
            case WP_EXPLOSIVES_FIRST_PLANT:
                me->SetFacingTo(1.46f);
                break;
            case WP_EXPLOSIVES_FIRST_DETONATE:
                SetEscortPaused(true);
                SetCanAttack(false);
                me->SetFacingTo(1.05f);
                moveTimer = 1000;
                phase = PHASE_PLANT_FIRST_TIMER_1;
                break;
            case WP_DEBUG_1:
                SetEscortPaused(true);
                SetCanAttack(false);
                moveTimer = 500;
                phase = PHASE_WP_26;
                break;
            case WP_SIRONAS_HILL:
            {
                SetEscortPaused(true);
                SetCanAttack(false);

                //Find Sironas and make it respawn if needed
                if (Creature* sironas = GetClosestCreatureWithEntry(me, NPC_SIRONAS, 100))
                {
                    if (!sironas->isAlive())
                        sironas->Respawn();

                    sironas->AI()->DoAction(ACTION_SIRONAS_CHANNEL_START);
                    me->SetFacingTo(me->GetOrientationTo(sironas));
                }
                moveTimer = 1000;
                phase = PHASE_FEEL_SIRONAS_1;
                break;
            }
            case WP_EXPLOSIVES_SECOND_BATTLEROAR:
                SetEscortPaused(true);
                SetCanAttack(false);
                moveTimer = 200;
                phase = PHASE_MEET_SIRONAS_ROAR;
                break;
            case WP_EXPLOSIVES_SECOND_PLANT:
                SetEscortPaused(true);
                SetCanAttack(false);
                moveTimer = 500;
                phase = PHASE_PLANT_SECOND_KNEEL;
                break;
            case WP_EXPLOSIVES_SECOND_DETONATE:
                SetEscortPaused(true);
                SetCanAttack(false);
                me->SetFacingTo(5.7f);
                moveTimer = 2000;
                phase = PHASE_PLANT_SECOND_TIMER_1;
                break;
            default:
                break;
        }
    }

    void DoAction(const int32 param)
    {
        switch (param)
        {
            case ACTION_LEGOSO_SIRONAS_KILLED:
                phase = PHASE_SIRONAS_SLAIN_SPEECH_1;
                moveTimer = 5000;
                break;
            default:
                break;
        }
    }
};

CreatureAI* GetAI_npc_demolitionist_legosoAI(Creature* creature)
{
    return new npc_demolitionist_legosoAI(creature);
}

bool QuestAccept_npc_demolitionist_legoso(Player* player, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ENDING_THEIR_WORLD)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_demolitionist_legosoAI, pCreature->AI()))
        {
            pCreature->GetMotionMaster()->MovementExpired();
            pEscortAI->SetMaxPlayerDistance(120.0f);
            pEscortAI->SetData(DATA_EVENT_STARTER_GUID, player->GetGUIDLow());
            pEscortAI->Start(true, true, player->GetGUID(), quest, true);
        }
    }
    return true;
}
 
bool ChooseReward_npc_exarach_admetius(Player *player, Creature *creature, const Quest *pQuest)
{
    if (pQuest->GetQuestId() == QUEST_ENDING_THEIR_WORLD)
        creature->Say(-1200341, LANG_UNIVERSAL, player->GetGUID());
    return true;
}

bool GOUse_go_184850(Player *player, GameObject* go)
{
    if (player->GetQuestStatus(9740) != QUEST_STATUS_INCOMPLETE)
        return false;

    if(Creature* controller = GetClosestCreatureWithEntry(go, 17886, 5))
    {
        go->SetLootState(GO_JUST_DEACTIVATED);
        controller->Kill(controller);
    }
    return true;
}

/*#####
# npc_17889
######*/

#define SPELL_RED_BEAM  30944

struct npc_17889AI : public ScriptedAI
{
    npc_17889AI(Creature* creature) : ScriptedAI(creature) {}
    
    Timer CheckTimer;
    uint8 deadCounter;

    void Reset()
    {
        CheckTimer.Reset(3000);
        deadCounter = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (CheckTimer.Expired(diff))
        {
            std::list<Creature*> triggerCreatures = FindAllCreaturesWithEntry(17886, 100.0f);

            for (std::list<Creature*>::iterator it = triggerCreatures.begin(); it != triggerCreatures.end(); ++it)
            {
                if (!(*it)->isAlive())
                    deadCounter++;
            }

            if (deadCounter == 4)
            {
                GameObject *triggerObject = FindGameObject(182026, 50.0f, m_creature);
                if (triggerObject)
                {
                    triggerObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE);
                    triggerObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
                }
            }
            else 
            {
                GameObject *triggerObject = FindGameObject(182026, 50.0f, m_creature);
                if (triggerObject)
                    triggerObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOTSELECTABLE);
            }
            
            deadCounter = 0;
            CheckTimer = 3000;
        }
    }
};

CreatureAI* GetAI_npc_17889(Creature* creature)
{
    return new npc_17889AI(creature);
}

/*######
## go_galaens_journal
######*/

#define GOSSIP_ITEM_NEXT_PAGE 16284

enum GalaensJournalPages
{
    GOSSIP_PAGE_0       = 990035,
    GOSSIP_PAGE_1       = 990036,
    GOSSIP_PAGE_2       = 990037,
    GOSSIP_PAGE_3       = 990038
};

bool GOUse_go_galaens_journal(Player* pPlayer, GameObject* pGO)
{
    pPlayer->PlayerTalkClass->ClearMenus();
    
    if (pGO->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        pPlayer->PrepareQuestMenu(pGO->GetGUID());
        pPlayer->SendPreparedQuest(pGO->GetGUID());   
    }

    pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NEXT_PAGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    pPlayer->SEND_GOSSIP_MENU(GOSSIP_PAGE_0, pGO->GetGUID());
    return true;
}

bool GOGossipSelect_go_galaens_journal(Player* pPlayer, GameObject* pGO, uint32 Sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NEXT_PAGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_PAGE_1, pGO->GetGUID());
        break;

        case GOSSIP_ACTION_INFO_DEF + 2: 
            pPlayer->ADD_GOSSIP_ITEM(0, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_NEXT_PAGE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_PAGE_2, pGO->GetGUID());
        break;

        case GOSSIP_ACTION_INFO_DEF + 3:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_PAGE_3, pGO->GetGUID());
        break;
    }
    return true;
}

void AddSC_bloodmyst_isle()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_17889";
    newscript->GetAI = &GetAI_npc_17889;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_webbed_creature";
    newscript->GetAI = &GetAI_mob_webbed_creature;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_captured_sunhawk_agent";
    newscript->pGossipHello =  &GossipHello_npc_captured_sunhawk_agent;
    newscript->pGossipSelect = &GossipSelect_npc_captured_sunhawk_agent;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_exarch_admetius";
    newscript->pGossipHello =  &GossipHello_npc_exarch_admetius;
    newscript->pGossipSelect = &GossipSelect_npc_exarch_admetius;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_princess_stillpine";
    newscript->GetAI = &GetAI_npc_princess_stillpineAI;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_princess_stillpine_cage";
    newscript->pGOUse = &GOUse_go_princess_stillpine_cage;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_sironas";
    newscript->GetAI = &GetAI_npc_sironasAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_demolitionist_legoso";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_demolitionist_legoso;
    newscript->GetAI = &GetAI_npc_demolitionist_legosoAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_exarach_admetius";
    newscript->pQuestRewardedNPC = &ChooseReward_npc_exarach_admetius;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_184850";
    newscript->pGOUse = &GOUse_go_184850;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_galaens_journal";
    newscript->pGOUse = &GOUse_go_galaens_journal;
    newscript->pGossipSelectGO =  &GOGossipSelect_go_galaens_journal;
    newscript->RegisterSelf();
}

