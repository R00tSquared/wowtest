// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
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
SDName: Desolace
SD%Complete:
SDComment: Quest support: 1440, 5561, 5381, 6132
SDCategory: Desolace
EndScriptData */

/* ContentData
npc_aged_dying_ancient_kodo
go_iruxos
npc_dalinda_malem
npc_melizza_brimbuzzle
npc_rokaro
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_aged_dying_ancient_kodo. Quest 5561
######*/

enum eDyingKodo
{
    // signed for 9999
    SAY_SMEED_HOME_1                = -1910207,
    SAY_SMEED_HOME_2                = -1910208,
    SAY_SMEED_HOME_3                = -1910209,

    GOSSIP_FIRST_VARIANT            = 11627,
    GOSSIP_SECOND_VARIANT           = 11628,
    GOSSIP_THIRD_VARIANT            = 11629,

    QUEST_KODO                      = 5561,

    NPC_SMEED                       = 11596,
    NPC_AGED_KODO                   = 4700,
    NPC_DYING_KODO                  = 4701,
    NPC_ANCIENT_KODO                = 4702,
    NPC_TAMED_KODO                  = 11627,

    SPELL_KODO_KOMBO_ITEM           = 18153,
    SPELL_KODO_KOMBO_PLAYER_BUFF    = 18172,                //spells here have unclear function, but using them at least for visual parts and checks
    SPELL_KODO_KOMBO_DESPAWN_BUFF   = 18377,
    SPELL_KODO_KOMBO_GOSSIP         = 18362

};

struct npc_aged_dying_ancient_kodoAI : public ScriptedAI
{
    npc_aged_dying_ancient_kodoAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    Timer m_uiDespawnTimer;

    void Reset()
    {
        m_uiDespawnTimer.Reset(0);
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (pWho->GetEntry() == NPC_SMEED)
        {
            if (m_creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                return;

            if (m_creature->IsWithinDistInMap(pWho, 10.0f))
            {
                DoScriptText(RAND(SAY_SMEED_HOME_1,SAY_SMEED_HOME_2,SAY_SMEED_HOME_3), pWho);

                //spell have no implemented effect (dummy), so useful to notify spellHit
                DoCast(m_creature, SPELL_KODO_KOMBO_GOSSIP, true);
            }
        }
    }

    void SpellHit(Unit* pCaster, SpellEntry const* pSpell)
    {
        if (pSpell->Id == SPELL_KODO_KOMBO_GOSSIP)
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        if (pSpell->Id == SPELL_KODO_KOMBO_ITEM)
            m_uiDespawnTimer = 5*MINUTE*MILLISECONDS;
    }

    void UpdateAI(const uint32 diff)
    {
        //timer should always be == 0 unless we already updated entry of creature. Then not expect this updated to ever be in combat.
        if (m_uiDespawnTimer.Expired(diff))
        {
            if (!m_creature->GetVictim() && m_creature->isAlive())
            {
                m_creature->DisappearAndDie();
                return;
            }
        } 

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_aged_dying_ancient_kodo(Creature* pCreature)
{
    return new npc_aged_dying_ancient_kodoAI(pCreature);
}

bool EffectDummyCreature_npc_aged_dying_ancient_kodo(Unit *pCaster, uint32 spellId, uint32 effIndex, Creature *pCreatureTarget)
{
    //always check spellid and effectindex
    if (spellId == SPELL_KODO_KOMBO_ITEM && effIndex == 0)
    {
        //no effect if player/creature already have aura from spells
        if (pCaster->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF, 0) || pCreatureTarget->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF, 0))
            return true;

        if (pCreatureTarget->GetEntry() == NPC_AGED_KODO ||
            pCreatureTarget->GetEntry() == NPC_DYING_KODO ||
            pCreatureTarget->GetEntry() == NPC_ANCIENT_KODO)
        {
            pCaster->CastSpell(pCaster,SPELL_KODO_KOMBO_PLAYER_BUFF,true);

            pCreatureTarget->UpdateEntry(NPC_TAMED_KODO);

            pCreatureTarget->CastSpell(pCreatureTarget,SPELL_KODO_KOMBO_DESPAWN_BUFF,false);

            if (pCreatureTarget->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
                pCreatureTarget->GetMotionMaster()->MoveIdle();

            pCreatureTarget->GetMotionMaster()->MoveFollow(pCaster, PET_FOLLOW_DIST,  pCreatureTarget->GetFollowAngle());
        }

        //always return true when we are handling this spell and effect
        return true;
    }
    return false;
}

bool GossipHello_npc_aged_dying_ancient_kodo(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF, 0) && pCreature->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF, 0))
    {
        //the expected quest objective
        pPlayer->CastCreatureOrGO(pCreature->GetEntry(), pCreature->GetGUID(), SPELL_KODO_KOMBO_GOSSIP);

        pPlayer->RemoveAurasDueToSpell(SPELL_KODO_KOMBO_PLAYER_BUFF);
        pCreature->ForcedDespawn(5000);
    }

    switch(urand(1, 3))
    {
        case 1:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_FIRST_VARIANT, pCreature->GetGUID());
            break;

        case 2:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_SECOND_VARIANT, pCreature->GetGUID());
            break;

        case 3:
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_THIRD_VARIANT, pCreature->GetGUID());
            break;
    }
    return true;
}

/*######
## go_iruxos. Quest 5381
######*/

bool GOUse_go_iruxos(Player *pPlayer, GameObject* pGO)
{
        if (pPlayer->GetQuestStatus(5381) == QUEST_STATUS_INCOMPLETE)
        {
            Creature* Demon = pPlayer->SummonCreature(11876, pPlayer->GetPositionX()+frand(-2,2),pPlayer->GetPositionY()+frand(-2,2),pPlayer->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,60000);
            if(Demon)
            {
                Demon->AI()->AttackStart(pPlayer);
                pGO->SetLootState(GO_JUST_DEACTIVATED);
                return true;
            }
        }
        return false;
}

/*######
## npc_dalinda_malem. Quest 1440
######*/

#define QUEST_RETURN_TO_VAHLARRIEL     1440

struct npc_dalindaAI : public npc_escortAI
{
    npc_dalindaAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();
        switch (i)
        {
            case 1:
                me->SetStandState(UNIT_STAND_STATE_STAND);
                break;
            case 15:
                if (pPlayer)
                pPlayer->GroupEventHappens(QUEST_RETURN_TO_VAHLARRIEL, m_creature);
                break;
            case 16:
                me->Kill(me, false);
                me->Respawn();
                break;
        }
    }

    void EnterCombat(Unit* pWho) { }

    void Reset()
    {
        me->SetStandState(UNIT_STAND_STATE_KNEEL);
    }

    void JustDied(Unit* pKiller)
    {
        if(pKiller->GetGUID() == me->GetGUID())
            return;
        Player* pPlayer = GetPlayerForEscort();
        if (pPlayer)
            pPlayer->FailQuest(QUEST_RETURN_TO_VAHLARRIEL);
        return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        npc_escortAI::UpdateAI(uiDiff);
        if (!UpdateVictim())
            return;
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_dalinda(Creature* pCreature)
{
    return new npc_dalindaAI(pCreature);
}

bool QuestAccept_npc_dalinda(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_RETURN_TO_VAHLARRIEL)
   {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_dalindaAI, pCreature->AI()))
        {
            pEscortAI->Start(true, false, pPlayer->GetGUID(), quest, true);
            pCreature->setFaction(113);
        }
    }
    return false;
}

/*#######
## npc_melizza_brimbuzzle
#######*/

enum
{
    SAY_START                   = -1000607,
    SAY_COMPLETE                = -1000608,
    SAY_POST_EVENT_1            = -1000609,
    SAY_POST_EVENT_2            = -1000610,
    SAY_POST_EVENT_3            = -1000611,

    NPC_MARAUDINE_BONEPAW       = 4660,
    NPC_MARAUDINE_SCOUT         = 4654,

    GO_MELIZZAS_CAGE            = 177706,
    QUEST_GET_ME_OUT_OF_HERE    = 6132
};

static float m_afAmbushSpawn[4][3]=
{
    {-1388.37f, 2427.81f, 88.8286f},
    {-1388.78f, 2431.85f, 88.7838f},
    {-1386.95f, 2429.76f, 88.8444f},
    {-1389.99f, 2429.93f, 88.7692f} 
};

struct npc_melizza_brimbuzzleAI : public npc_escortAI
{
    npc_melizza_brimbuzzleAI(Creature* pCreature) : npc_escortAI(pCreature) { }

    uint32 m_uiPostEventCount;
    uint64 m_uiPostEventTimer;
        
    void Reset()
    {
        m_uiPostEventCount = 0;
        m_uiPostEventTimer = 0;
    }

    void WaypointReached(uint32 uiPointId)
    {
        if (Player* pPlayer = GetPlayerForEscort())
        {
            switch (uiPointId)
            {
            case 1:
                me->setFaction(FACTION_ESCORT_H_NEUTRAL_ACTIVE);
                DoScriptText(SAY_START, me, pPlayer);
                break;
            case 7:
                 me->SummonCreature(NPC_MARAUDINE_SCOUT, m_afAmbushSpawn[0][0], m_afAmbushSpawn[0][1], m_afAmbushSpawn[0][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 me->SummonCreature(NPC_MARAUDINE_SCOUT, m_afAmbushSpawn[1][0], m_afAmbushSpawn[1][1], m_afAmbushSpawn[1][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 me->SummonCreature(NPC_MARAUDINE_SCOUT, m_afAmbushSpawn[2][0], m_afAmbushSpawn[2][1], m_afAmbushSpawn[2][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 me->SummonCreature(NPC_MARAUDINE_BONEPAW, m_afAmbushSpawn[3][0], m_afAmbushSpawn[3][1], m_afAmbushSpawn[3][2], 1.6f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
                 break;
            case 10:
                DoScriptText(SAY_COMPLETE, me);
                me->RestoreFaction();
                SetRun();
                pPlayer->GroupEventHappens(QUEST_GET_ME_OUT_OF_HERE, me);
                break;
            case 15:
                m_uiPostEventCount = 1;
                break;

            }
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {        
        npc_escortAI::UpdateAI(uiDiff);

        if (!UpdateVictim())
        {
            if (m_uiPostEventCount && HasEscortState(STATE_ESCORT_ESCORTING))
            {
                if (m_uiPostEventTimer <= uiDiff)
                {
                    m_uiPostEventTimer = 3000;

                    if (Unit* pPlayer = GetPlayerForEscort())
                    {
                        switch(m_uiPostEventCount)
                        {
                            case 1:
                                DoScriptText(SAY_POST_EVENT_1, me);
                                ++m_uiPostEventCount;
                                break;
                            case 2:
                                DoScriptText(SAY_POST_EVENT_2, me);
                                ++m_uiPostEventCount;
                                break;
                            case 3:
                                DoScriptText(SAY_POST_EVENT_3, me);
                                m_uiPostEventCount = 0;
                                me->ForcedDespawn(60000);
                                break;
                        }
                    }
                }
                else
                    m_uiPostEventTimer -= uiDiff;
            }

            return;
        }

        DoMeleeAttackIfReady();
    }

    void JustSummoned(Creature* pSummoned)
    {
        pSummoned->AI()->AttackStart(me);
    }
};

CreatureAI* GetAI_npc_melizza_brimbuzzle(Creature* pCreature)
{
    return new npc_melizza_brimbuzzleAI(pCreature);
}

bool QuestAccept_npc_melizza_brimbuzzle(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_GET_ME_OUT_OF_HERE)
    {
        if (GameObject* pGo = FindGameObject(GO_MELIZZAS_CAGE, INTERACTION_DISTANCE, pCreature))
            pGo->UseDoorOrButton();

        if (npc_melizza_brimbuzzleAI* pEscortAI = CAST_AI(npc_melizza_brimbuzzleAI, pCreature->AI()))
            pEscortAI->Start(true, false, pPlayer->GetGUID(), pQuest);
    }
    return true;
}

/*####
# npc_rokaro
####*/

#define GOSSIP_ITEM_ROKARO 16017

bool GossipHello_npc_rokaro(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(player->GetQuestRewardStatus(6602) && !player->HasItemCount(16309,1))
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(16017), GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_rokaro(Player *player, Creature *_Creature, uint32 sender, uint32 action )
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

struct npc_magram_spectreAI : public ScriptedAI
{
    npc_magram_spectreAI(Creature* c) : ScriptedAI(c) {}
    
    Timer checker;

    void Reset()
    {
        m_creature->SetWalk(true);
        m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetVisibility(VISIBILITY_OFF);
        checker = 1000;
    }

    void MovementInform(uint32 type, uint32 id)
    {
        if (type == POINT_MOTION_TYPE && id == 1)
            me->GetMotionMaster()->MoveRandomAroundPoint(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 10.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim() && checker.Expired(diff))
        {
            checker = 0;
            GameObject* gob = NULL;
            Hellground::NearestGameObjectEntryInObjectRangeCheck check(*m_creature,177746,90.0f);
            Hellground::ObjectSearcher<GameObject, Hellground::NearestGameObjectEntryInObjectRangeCheck> checker(gob,check);

            Cell::VisitGridObjects(m_creature, checker, 90);
            if (gob)
            {
                m_creature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                m_creature->SetVisibility(VISIBILITY_ON);
                float x, y, z;
                gob->GetNearPoint(x, y, z, 10, 0, frand(0, 2 * M_PI));
                m_creature->GetMotionMaster()->MovePoint(1, x, y, z);
            }
            return;
        }
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_magram_spectre(Creature* crea)
{
    return new npc_magram_spectreAI(crea);
}

/*
 * Gizelton Caravan, Bodyguard For Hire support
 */

struct GizeltonStruct
{
    int32 onLeave, onAnnounce, onAmbush0, onAmbush1, onAmbush2, onComplete;
};

const GizeltonStruct CaravanTalk[] = 
{
    { -1001000, -1001001, -1001002, -1001003, -1001004, -1001005 },
    { -1001006, -1001007, -1001008, -1001009, -1001010, -1001011 }
};

enum
{
    NPC_RIGGER_GIZELTON     = 11626,
    NPC_CORK_GIZELTON       = 11625,
    NPC_SUPER_SELLER_680    = 12246,
    NPC_VENDOR_TRON_1000    = 12245,
    NPC_CARAVAN_KODO        = 11564,
    NPC_DOOMWARDER          = 4677,
    NPC_NETHER_SORCERESS    = 4684,
    NPC_LESSER_INFERNAL     = 4676,
    NPC_KOLKAR_AMBUSHER     = 12977,
    NPC_KOLKAR_WAYLAYER     = 12976,

    POINT_BOT_CAMP          = 279,
    POINT_BOT_ANNOUNCE      = 14,
    POINT_BOT_AMBUSH_0      = 28,
    POINT_BOT_AMBUSH_1      = 34,
    POINT_BOT_AMBUSH_2      = 40,
    POINT_BOT_COMPLETE      = 42,

    POINT_TOP_CAMP          = 141,
    POINT_TOP_ANNOUNCE      = 164,
    POINT_TOP_AMBUSH_0      = 173,
    POINT_TOP_AMBUSH_1      = 181,
    POINT_TOP_AMBUSH_2      = 188,
    POINT_TOP_COMPLETE      = 195,
    POINT_END               = 281,

    QUEST_BOTTOM            = 5943,
    QUEST_TOP               = 5821,
};

struct Coords
{
    uint32 entry;
    float x, y, z, o;
};

struct Formation
{
    float distance, angle;
};

const Coords Ambusher[] = 
{
    { NPC_DOOMWARDER,       -1814.41f, 1983.18f, 58.9549f, 0.0f },
    { NPC_NETHER_SORCERESS, -1814.41f, 1983.18f, 58.9549f, 0.0f },
    { NPC_LESSER_INFERNAL,  -1814.41f, 1983.18f, 58.9549f, 0.0f },

    { NPC_DOOMWARDER,       -1751.9f, 1917.2f, 59.0003f, 0.0f },
    { NPC_NETHER_SORCERESS, -1751.9f, 1917.2f, 59.0003f, 0.0f },
    { NPC_LESSER_INFERNAL,  -1751.9f, 1917.2f, 59.0003f, 0.0f },

    { NPC_DOOMWARDER,       -1684.12f, 1872.66f, 59.0354f, 0.0f },
    { NPC_NETHER_SORCERESS, -1684.12f, 1872.66f, 59.0354f, 0.0f },
    { NPC_LESSER_INFERNAL,  -1684.12f, 1872.66f, 59.0354f, 0.0f },

    { NPC_KOLKAR_AMBUSHER,  -792.515f, 1177.07f, 98.8327f, 0.0f },
    { NPC_KOLKAR_WAYLAYER,  -792.515f, 1177.07f, 98.8327f, 0.0f },

    { NPC_KOLKAR_AMBUSHER,  -931.15f, 1182.17f, 91.8346f, 0.0f },
    { NPC_KOLKAR_WAYLAYER,  -931.15f, 1182.17f, 91.8346f, 0.0f },

    { NPC_KOLKAR_AMBUSHER,  -1073.62f, 1186.33f, 89.7398f, 0.0f },
    { NPC_KOLKAR_WAYLAYER,  -1073.62f, 1186.33f, 89.7398f, 0.0f }
};

const Coords Caravan[] = 
{
    { NPC_CARAVAN_KODO,     -1887.26f, 2465.12f, 59.8224f, 4.48f },
    { NPC_RIGGER_GIZELTON,  -1883.63f, 2471.68f, 59.8224f, 4.48f },
    { NPC_CARAVAN_KODO,     -1882.11f, 2476.80f, 59.8224f, 4.48f }
};

const Formation CaravanFormation[] =
{
    { 32.0f, 6.28f },
    { 26.0f, 3.14f },
    { 18.0f, 3.14f },
    { 8.0f,  3.14f }
};

struct npc_cork_gizeltonAI : npc_escortAI
{
    explicit npc_cork_gizeltonAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        npc_cork_gizeltonAI::Reset();
        npc_cork_gizeltonAI::ResetCreature();
    }

    std::list<uint64> m_lCaravanGuid;
    ObjectGuid m_currentGuid;
    ObjectGuid m_RiggerGuid;
    ObjectGuid m_playerGuid;
    uint8 m_uiEnemiesCount;
    uint8 m_uiAnnounceCount;
    uint32 m_uiInitDelayTimer;
    uint32 m_uiCampTimer;
    uint32 m_uiAnnounceTimer;
    uint32 m_uiDepartTimer;
    bool m_bInit;
    bool m_bCamp;
    bool m_bWaitingForPlayer;
    bool m_bWaitingForDepart;
    bool m_bRigger;

    void Reset()
    {
    }

    void ResetCreature()
    {
        m_lCaravanGuid.clear();
        m_currentGuid.Clear();
        m_RiggerGuid.Clear();
        m_playerGuid.Clear();
        m_uiEnemiesCount = 0;
        m_uiAnnounceCount = 0;
        m_uiInitDelayTimer = 2000;
        m_uiCampTimer = 10 * MINUTE * MILLISECONDS;
        m_uiDepartTimer = MINUTE * MILLISECONDS;
        m_uiAnnounceTimer = 0;
        m_bInit = false;
        m_bCamp = false;
        m_bWaitingForPlayer = false;
        m_bWaitingForDepart = false;
        m_bRigger = true;
    }

    void SummonCaravan()
    {
        uint32 groupID = 0;
        Creature* varCreature = NULL;
        m_lCaravanGuid.push_back(m_creature->GetObjectGuid());
        for (int32 i = 0; i < 3; ++i)
        {
            varCreature = m_creature->SummonCreature(Caravan[i].entry, Caravan[i].x, Caravan[i].y, Caravan[i].z, Caravan[i].o, TEMPSUMMON_DEAD_DESPAWN, 30000, true);
            if (!varCreature) // couldn't create!
                continue;
            varCreature->setActive(true);
            groupID = CreatureGroupManager::AddTempCreatureToTempGroup(groupID, varCreature, -27 - i); // -27, -28, -29
        }
        groupID = CreatureGroupManager::AddTempCreatureToTempGroup(groupID, me, -26); // leader
        if (CreatureGroup *formation = me->GetFormation())
            formation->SetLeader(me);
    }

    void JustDied(Unit* /*pKiller*/)
    {
        FailEscort();
    }

    void FailEscort()
    {
        if (auto pPlayer = m_creature->GetPlayerInWorld(m_playerGuid))
            pPlayer->FailQuest(m_bRigger ? QUEST_BOTTOM : QUEST_TOP);

        DespawnCaravan();
    }

    void DespawnCaravan()
    {
        for (auto itr = m_lCaravanGuid.begin(); itr != m_lCaravanGuid.end(); ++itr)
        {
            if (*itr != m_creature->GetObjectGuid())
            {
                if (auto pKillMe = m_creature->GetMap()->GetCreature(*itr))
                    pKillMe->RemoveFromWorld();
            }
        }

        m_creature->ForcedDespawn();
    }

    void CaravanFaction(bool apply)
    {
        for (auto itr = m_lCaravanGuid.begin(); itr != m_lCaravanGuid.end(); ++itr)
        {
            if (*itr != m_creature->GetObjectGuid())
            {
                if (auto pCreature = m_creature->GetMap()->GetCreature(*itr))
                    if (apply)
                        pCreature->setFaction(FACTION_ESCORT_N_NEUTRAL_PASSIVE);
                    else
                        pCreature->setFaction(pCreature->getFaction());
            }
        }

        if (apply)
            m_creature->setFaction(FACTION_ESCORT_N_NEUTRAL_PASSIVE);
        else
            me->setFaction(me->getFaction());
    }

    void SummonAmbusher(uint8 index) const
    {
        float x = Ambusher[index].x;
        float y = Ambusher[index].y;
        float z = Ambusher[index].z;

        /* for (uint8 i = 0; i < 5; ++i)
        {
            if (m_creature->GetMap()->GetWalkRandomPosition(nullptr, x, y, z, 20.0f, NAV_GROUND))
                break;
        } */

        m_creature->SummonCreature(Ambusher[index].entry, x, y, z, 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000);
    }

    void Ambush(uint8 index) const
    {
        switch (index)
        {
            case POINT_BOT_AMBUSH_0:
                SummonAmbusher(0);
                SummonAmbusher(1);
                SummonAmbusher(2);
                DoTalk(CaravanTalk[0].onAmbush0);
                break;
            case POINT_BOT_AMBUSH_1:
                SummonAmbusher(3);
                SummonAmbusher(4);
                SummonAmbusher(5);
                DoTalk(CaravanTalk[0].onAmbush1);
                break;
            case POINT_BOT_AMBUSH_2:
                SummonAmbusher(6);
                SummonAmbusher(7);
                SummonAmbusher(8);
                DoTalk(CaravanTalk[0].onAmbush2);
                break;
            case POINT_TOP_AMBUSH_0:
                SummonAmbusher(9);
                SummonAmbusher(10);
                SummonAmbusher(9);
                SummonAmbusher(10);
                DoTalk(CaravanTalk[1].onAmbush0);
                break;
            case POINT_TOP_AMBUSH_1:
                SummonAmbusher(11);
                SummonAmbusher(12);
                SummonAmbusher(11);
                SummonAmbusher(12);
                DoTalk(CaravanTalk[1].onAmbush1);
                break;
            case POINT_TOP_AMBUSH_2:
                SummonAmbusher(13);
                SummonAmbusher(14);
                SummonAmbusher(13);
                SummonAmbusher(14);
                DoTalk(CaravanTalk[1].onAmbush2);
                break;
        }
    }

    void JustSummoned(Creature* pSummoned)
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_RIGGER_GIZELTON:
                m_RiggerGuid = pSummoned->GetObjectGuid();
                m_currentGuid = m_RiggerGuid;
            case NPC_CARAVAN_KODO:
                m_lCaravanGuid.push_back(pSummoned->GetObjectGuid());
                return;
				break;
			case NPC_DOOMWARDER:
			case NPC_NETHER_SORCERESS:
			case NPC_LESSER_INFERNAL:
			case NPC_KOLKAR_AMBUSHER:
			case NPC_KOLKAR_WAYLAYER:
				++m_uiEnemiesCount;
				break;
			default: break;
		}

		// pick random caravan target
		uint8 targetIndex = urand(0, 3);

		if (!targetIndex)
			pSummoned->AI()->AttackStart(m_creature);
		else
		{
			if (m_lCaravanGuid.size() > targetIndex)
			{
				auto itr = m_lCaravanGuid.begin();
				std::advance(itr, targetIndex);

				if (auto pCreature = m_creature->GetMap()->GetCreature(*itr))
					pSummoned->AI()->AttackStart(pCreature);
			}
		}
    }

    void SummonedCreatureDies(Creature* pSummoned, Unit* killer)
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_RIGGER_GIZELTON:
            case NPC_CARAVAN_KODO:
                FailEscort();
                return;
        }

        --m_uiEnemiesCount;

        if (!m_uiEnemiesCount)
            SetEscortPaused(false);
    }

    void PlayerHere(Player* pPlayer)
    {
        m_bWaitingForPlayer = false;
        m_bWaitingForDepart = true;
        m_uiAnnounceCount = 0;
        m_playerGuid = pPlayer->GetObjectGuid();
        GiveQuest(false);
    }

    void DoTalk(int32 textId, bool yell = false) const
    {
        auto pTalker = m_bRigger ? m_creature->GetMap()->GetCreature(m_RiggerGuid) : m_creature;

        if (pTalker)
        {
            if (yell)
                pTalker->MonsterYellToZone(textId, LANG_UNIVERSAL, 0);
            else
                pTalker->MonsterSay(textId, LANG_UNIVERSAL, 0);
        }
    }

    void GiveQuest(bool give) const
    {
        auto pGiver = m_bRigger ? m_creature->GetMap()->GetCreature(m_RiggerGuid) : m_creature;

        if (pGiver)
        {
            if (give)
                pGiver->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            else
                pGiver->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        }
    }

    void CaravanWalk(bool walk) const
    {
        auto pLeader = m_bRigger ? m_creature->GetMap()->GetCreature(m_RiggerGuid) : m_creature;

        if (pLeader)
            pLeader->SetWalk(walk);
    }

    void DoVendor(bool visible) const
    {
        auto pVendor = FindCreature(m_bRigger ? NPC_SUPER_SELLER_680 : NPC_VENDOR_TRON_1000, 100.0f, me);

        if (pVendor)
        {
            pVendor->SetVisibility(visible ? VISIBILITY_ON : VISIBILITY_OFF);
        }
    }

    void WaypointReached(uint32 uiPoint)
    {
        switch (uiPoint)
        {
            case POINT_BOT_CAMP:
            case POINT_TOP_CAMP:
                SetEscortPaused(true);
                CaravanWalk(true);
                m_uiCampTimer = 10 * MINUTE * MILLISECONDS;
                m_bCamp = true;
                DoVendor(true);
                break;
            case POINT_BOT_ANNOUNCE:
            case POINT_TOP_ANNOUNCE:
                SetEscortPaused(true);
                GiveQuest(true);
                m_uiAnnounceTimer = 0;
                m_uiDepartTimer = 10 * MILLISECONDS;
                m_bWaitingForPlayer = true;
                break;
            case POINT_BOT_AMBUSH_0:
            case POINT_BOT_AMBUSH_1:
            case POINT_BOT_AMBUSH_2:
            case POINT_TOP_AMBUSH_0:
            case POINT_TOP_AMBUSH_1:
            case POINT_TOP_AMBUSH_2:
                SetEscortPaused(true);
                Ambush(uiPoint);
                break;
            case POINT_BOT_COMPLETE:
            case POINT_TOP_COMPLETE:
                {
                    DoTalk(CaravanTalk[m_bRigger ? 0 : 1].onComplete);

                    if (auto pPlayer = m_creature->GetPlayerInWorld(m_playerGuid))
                    {
                        if (pPlayer->IsInRange(m_creature, 0.0f, 100.0f))
                            pPlayer->GroupEventHappens(m_bRigger ? QUEST_BOTTOM : QUEST_TOP, m_creature);                
                    }

                    CaravanFaction(false);
                    CaravanWalk(false);
                    m_bRigger = !m_bRigger;
                }
                break;
            case POINT_END:
                DespawnCaravan();
                break;
            }
    }

    void UpdateEscortAI(uint32 const uiDiff)
    {
        // just summoned, do init
        if (!m_bInit)
        {
            if (m_uiInitDelayTimer < uiDiff)
            {
                SummonCaravan();
                m_bInit = true;
                Start();
            }
            else
                m_uiInitDelayTimer -= uiDiff;

            return;
        }

        // caravan is at camp point, vendor is available
        if (m_bCamp)
        {
            if (m_uiCampTimer < uiDiff)
            {
                m_bCamp = false;
                DoTalk(CaravanTalk[m_bRigger ? 0 : 1].onLeave);
                DoVendor(false);
                SetEscortPaused(false);
            }
            else
                m_uiCampTimer -= uiDiff;

            return;
        }

        // caravan is at waiting point, announcing every 3 minutes
        if (m_bWaitingForPlayer)
        {
            if (m_uiAnnounceTimer < uiDiff)
            {
                ++m_uiAnnounceCount;

                // caravan stays for 15+ minutes waiting for help
                if (m_uiAnnounceCount > 5)
                    DespawnCaravan();

                DoTalk(CaravanTalk[m_bRigger ? 0 : 1].onAnnounce, true);
                m_uiAnnounceTimer = 3 * MINUTE * MILLISECONDS;
            }
            else
                m_uiAnnounceTimer -= uiDiff;

            return;
        }

        // player is here, 10 seconds more and caravan goes
        if (m_bWaitingForDepart)
        {
            if (m_uiDepartTimer < uiDiff)
            {
                m_bWaitingForDepart = false;
                CaravanFaction(true);
                SetEscortPaused(false);
            }
            else
                m_uiDepartTimer -= uiDiff;

            return;
        }

        npc_escortAI::UpdateEscortAI(uiDiff);
    }
};

CreatureAI* GetAI_npc_cork_gizelton(Creature* pCreature)
{
    return new npc_cork_gizeltonAI(pCreature);
}

bool QuestAccept_npc_cork_gizelton(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TOP)
    {
        if (auto pCorkAI = dynamic_cast<npc_cork_gizeltonAI*>(pCreature->AI()))
            pCorkAI->PlayerHere(pPlayer);
    }

    return true;
}

bool QuestAccept_npc_rigger_gizelton(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_BOTTOM)
    {
        if (Creature* pCork = (Creature*)FindCreature(NPC_CORK_GIZELTON, 100.0f, pCreature))
        {
            if (auto pCorkAI = dynamic_cast<npc_cork_gizeltonAI*>(pCork->AI()))
                pCorkAI->PlayerHere(pPlayer);
        }
    }

    return true;
}

/*
 * Vendor-Tron 1000, Super-Seller 680 (Gizelton Caravan, Bodyguard For Hire support)
 */

struct npc_caravan_vendorAI : ScriptedAI
{
    explicit npc_caravan_vendorAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        npc_caravan_vendorAI::Reset();
        
        m_creature->SetVisibility(VISIBILITY_OFF);
    }

    void Reset()
    {

    }
};

CreatureAI* GetAI_npc_caravan_vendor(Creature* pCreature)
{
    return new npc_caravan_vendorAI(pCreature);
}


void AddSC_desolace()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_aged_dying_ancient_kodo";
    newscript->GetAI = &GetAI_npc_aged_dying_ancient_kodo;
    newscript->pEffectDummyNPC = &EffectDummyCreature_npc_aged_dying_ancient_kodo;
    newscript->pGossipHello = &GossipHello_npc_aged_dying_ancient_kodo;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_iruxos";
    newscript->pGOUse = &GOUse_go_iruxos;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dalinda";
    newscript->GetAI = &GetAI_npc_dalinda;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_dalinda;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_melizza_brimbuzzle";
    newscript->GetAI = &GetAI_npc_melizza_brimbuzzle;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_melizza_brimbuzzle;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_rokaro";
    newscript->pGossipHello = &GossipHello_npc_rokaro;
    newscript->pGossipSelect = &GossipSelect_npc_rokaro;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_magram_spectre";
    newscript->GetAI = &GetAI_npc_magram_spectre;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_cork_gizelton";
    newscript->GetAI = &GetAI_npc_cork_gizelton;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_cork_gizelton;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_rigger_gizelton";
    newscript->pQuestAcceptNPC = &QuestAccept_npc_rigger_gizelton;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_caravan_vendor";
    newscript->GetAI = &GetAI_npc_caravan_vendor;
    newscript->RegisterSelf();
}
