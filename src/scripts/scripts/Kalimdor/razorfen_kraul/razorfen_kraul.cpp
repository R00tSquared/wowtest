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
SDName: Razorfen Kraul
SD%Complete: 100
SDComment: Quest support: 1144
SDCategory: Razorfen Kraul
EndScriptData */

/* ContentData
npc_willix
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"
#include "def_razorfen_kraul.h"
#include "follower_ai.h"

#define SAY_READY -1047000
#define SAY_POINT -10470001
#define SAY_AGGRO1 -1047002
#define SAY_BLUELEAF -1047003
#define SAY_DANGER -1047004
#define SAY_BAD -1047005
#define SAY_THINK -1047006
#define SAY_SOON -1047007
#define SAY_FINALY -1047008
#define SAY_WIN -1047009
#define SAY_END -1047010

#define QUEST_WILLIX_THE_IMPORTER 1144
#define ENTRY_BOAR 4514
#define SPELL_QUILLBOAR_CHANNELING 7083

struct npc_willixAI : public npc_escortAI
{
    npc_willixAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch (i)
        {
        case 3:
            m_creature->HandleEmoteCommand(EMOTE_STATE_POINT);
            DoScriptText(SAY_POINT, m_creature, player);
            break;
        case 4:
            m_creature->SummonCreature(ENTRY_BOAR, 2137.66, 1843.98, 48.08, 1.54, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            break;
        case 8:
            DoScriptText(SAY_BLUELEAF, m_creature, player);
            break;
        case 9:
            DoScriptText(SAY_DANGER, m_creature, player);
            break;
        case 13:
            DoScriptText(SAY_BAD, m_creature, player);
            break;
        case 14:
            m_creature->SummonCreature(ENTRY_BOAR, 2078.91, 1704.54, 56.77, 1.54, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            break;
        case 25:
            DoScriptText(SAY_THINK, m_creature, player);
            break;
        case 31:
            DoScriptText(SAY_SOON, m_creature, player);
            break;
        case 42:
            DoScriptText(SAY_FINALY, m_creature, player);
            break;
        case 43:
            m_creature->SummonCreature(ENTRY_BOAR, 1956.43, 1596.97, 81.75, 1.54,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            break;
        case 45:
            DoScriptText(SAY_WIN, m_creature, player);
            player->GroupEventHappens(QUEST_WILLIX_THE_IMPORTER,m_creature);
            break;
        case 46:
            DoScriptText(SAY_END, m_creature, player);
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            break;
        }
    }

    void Reset() {}

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_AGGRO1, m_creature, NULL);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void JustDied(Unit* killer)
    {
        if (Player* pPlayer = GetPlayerForEscort())
            CAST_PLR(pPlayer)->FailQuest(QUEST_WILLIX_THE_IMPORTER);
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_willix(Player* player, Creature* creature, Quest const* quest)
{
    if (player && quest->GetQuestId() == QUEST_WILLIX_THE_IMPORTER)
    {
        CAST_AI(npc_escortAI, (creature->AI()))->Start(true, true, player->GetGUID());
        CAST_AI(npc_escortAI, (creature->AI()))->SetDespawnAtEnd(false);
        DoScriptText(SAY_READY, creature, player);
        creature->setFaction(113);
    }

    return true;
}

struct npc_deaths_head_ward_keeperAI : public ScriptedAI
{
    npc_deaths_head_ward_keeperAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (c->GetInstanceData());
        Reset();
    }

    ScriptedInstance *pInstance;
    Timer QuillboarChanneling_Timer;

    void Reset()
    {
        QuillboarChanneling_Timer.Reset(1500);
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->isAlive())
            return;

        if (pInstance)
            pInstance->SetData(TYPE_WARD_KEEPERS, NOT_STARTED);

        if (QuillboarChanneling_Timer.Expired(diff))
        {
            if( m_creature->IsNonMeleeSpellCast(false) )
                m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature, SPELL_QUILLBOAR_CHANNELING);
            QuillboarChanneling_Timer = 1100;
        }

    }
};

CreatureAI* GetAI_npc_deaths_head_ward_keeper(Creature *_Creature)
{
    return new npc_deaths_head_ward_keeperAI(_Creature);
}

CreatureAI* GetAI_npc_willix(Creature *_Creature)
{
    npc_willixAI* thisAI = new npc_willixAI(_Creature);

    thisAI->AddWaypoint(0, 2194.38, 1791.65, 65.48, 5000);
    thisAI->AddWaypoint(1, 2188.56, 1805.87, 64.45);
    thisAI->AddWaypoint(2, 2187, 1843.49, 59.33);
    thisAI->AddWaypoint(3, 2163.27, 1851.67, 56.73, 5000);
    thisAI->AddWaypoint(4, 2137.66, 1843.98, 48.08, 5000);
    thisAI->AddWaypoint(5, 2140.22, 1845.02, 48.32);
    thisAI->AddWaypoint(6, 2131.5, 1804.29, 46.85);
    thisAI->AddWaypoint(7, 2096.18, 1789.03, 51.13);
    thisAI->AddWaypoint(8, 2074.46, 1780.09, 55.64, 3000);
    thisAI->AddWaypoint(9, 2055.12, 1768.67, 58.46, 5000);
    thisAI->AddWaypoint(10, 2037.83, 1748.62, 60.27);
    thisAI->AddWaypoint(11, 2037.51, 1728.94, 60.85);
    thisAI->AddWaypoint(12, 2044.7, 1711.71, 59.71);
    thisAI->AddWaypoint(13, 2067.66, 1701.84, 57.77, 3000);
    thisAI->AddWaypoint(14, 2078.91, 1704.54, 56.77, 3000);
    thisAI->AddWaypoint(15, 2097.65, 1715.24, 54.74);
    thisAI->AddWaypoint(16, 2106.44, 1720.98, 54.41);
    thisAI->AddWaypoint(17, 2123.96, 1732.56, 52.27);
    thisAI->AddWaypoint(18, 2153.82, 1728.73, 51.92);
    thisAI->AddWaypoint(19, 2163.49, 1706.33, 54.42);
    thisAI->AddWaypoint(20, 2158.75, 1695.98, 55.70);
    thisAI->AddWaypoint(21, 2142.6, 1680.72, 58.24);
    thisAI->AddWaypoint(22, 2118.31, 1671.54, 59.21);
    thisAI->AddWaypoint(23, 2086.02, 1672.04, 61.24);
    thisAI->AddWaypoint(24, 2068.81, 1658.93, 61.24);
    thisAI->AddWaypoint(25, 2062.82, 1633.31, 64.35, 3000);
    thisAI->AddWaypoint(26, 2063.05, 1589.16, 63.26);
    thisAI->AddWaypoint(27, 2063.67, 1577.22, 65.89);
    thisAI->AddWaypoint(28, 2057.94, 1560.68, 68.40);
    thisAI->AddWaypoint(29, 2052.56, 1548.05, 73.35);
    thisAI->AddWaypoint(30, 2045.22, 1543.4, 76.65);
    thisAI->AddWaypoint(31, 2034.35, 1543.01, 79.70);
    thisAI->AddWaypoint(32, 2029.95, 1542.94, 80.79);
    thisAI->AddWaypoint(33, 2021.34, 1538.67, 80.8);
    thisAI->AddWaypoint(34, 2012.45, 1549.48, 79.93);
    thisAI->AddWaypoint(35, 2008.05, 1554.92, 80.44);
    thisAI->AddWaypoint(36, 2006.54, 1562.72, 81.11);
    thisAI->AddWaypoint(37, 2003.8, 1576.43, 81.57);
    thisAI->AddWaypoint(38, 2000.57, 1590.06, 80.62);
    thisAI->AddWaypoint(39, 1998.96, 1596.87, 80.22);
    thisAI->AddWaypoint(40, 1991.19, 1600.82, 79.39);
    thisAI->AddWaypoint(41, 1980.71, 1601.44, 79.77, 3000);
    thisAI->AddWaypoint(42, 1967.22, 1600.18, 80.62, 3000);
    thisAI->AddWaypoint(43, 1956.43, 1596.97, 81.75, 3000);
    thisAI->AddWaypoint(44, 1954.87, 1592.02, 82.18);
    thisAI->AddWaypoint(45, 1948.35, 1571.35, 80.96, 0);
    thisAI->AddWaypoint(46, 1947.02, 1566.42, 81.80, 0);

    return (CreatureAI*)thisAI;
}


/*######
## npc_snufflenose_gopher
######*/

enum
{
    SPELL_SNUFFLENOSE_COMMAND   = 8283,
    NPC_SNUFFLENOSE_GOPHER      = 4781,
    GO_BLUELEAF_TUBER           = 20920,

    SAY_GOPHER_SPAWN            = -1780223,
    SAY_GOPHER_COMMAND          = -1780224,
    SAY_GOPHER_FOUND            = -1780225
};

struct ObjectDistanceOrderGO : public std::binary_function<const WorldObject, const WorldObject, bool>
{
    const Unit* m_pSource;
    ObjectDistanceOrderGO(const Unit* pSource) : m_pSource(pSource) {};

    bool operator()(const WorldObject* pLeft, const WorldObject* pRight) const
    {
        return m_pSource->GetDistanceOrder(pLeft, pRight, false);
    }
};

struct npc_snufflenose_gopherAI : public FollowerAI
{
    npc_snufflenose_gopherAI(Creature* pCreature) :  FollowerAI(pCreature)
    {
        Reset();
        DoScriptText(SAY_GOPHER_SPAWN, m_creature);
        pCreature->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        // Follow player by default
        if(Unit* owner = me->GetOwner())
        {
            if(owner->GetTypeId() == TYPEID_PLAYER)
                StartFollow(((Player*)owner));
        }

        SetFollowPaused(true);
    }

    bool m_bIsMovementActive;

    ObjectGuid m_targetTuberGuid;
    std::list<uint64> m_foundTubers;
    uint32 m_followPausedTimer;

    void Reset()
    {
        m_creature->setFaction(35);
        m_bIsMovementActive  = false;
        m_followPausedTimer = 3000;
    }

    void MovementInform(uint32 uiMoveType, uint32 uiPointId)
    {
        if (uiMoveType != POINT_MOTION_TYPE || !uiPointId)
            return;

        if (!HasFollowState(STATE_FOLLOW_PAUSED))
            return;

        if (GameObject* pGo = m_creature->GetMap()->GetGameObject(m_targetTuberGuid))
        {
            pGo->SetRespawnTime(3 * MINUTE);
            pGo->Refresh();

            pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
            m_foundTubers.push_back(m_targetTuberGuid);
        }

        // Wait for 5 seconds after uncovering tuber before following again
        m_followPausedTimer = 5000; 
        m_bIsMovementActive = false;
    }

    // Function to search for new tuber in range
    void DoFindNewTuber()
    {   
        std::list<GameObject*> lTubersInRange;
        Hellground::AllGameObjectsWithEntryInGrid go_check(GO_BLUELEAF_TUBER);
        Hellground::ObjectListSearcher<GameObject, Hellground::AllGameObjectsWithEntryInGrid> go_search(lTubersInRange, go_check);
        Cell::VisitGridObjects(me, go_search, 60.0f);

        if (lTubersInRange.empty())
            return;

        lTubersInRange.sort(ObjectDistanceOrderGO(m_creature));
        GameObject* pNearestTuber = NULL;

        // Always need to find new ones
        for (std::list<GameObject*>::const_iterator itr = lTubersInRange.begin(); itr != lTubersInRange.end(); ++itr)
        {
            if (IsValidTuber(*itr))
            {
                pNearestTuber = *itr;
                break;
            }

        }

        if (!pNearestTuber)
            return;

        DoScriptText(SAY_GOPHER_FOUND, m_creature);

        m_targetTuberGuid = pNearestTuber->GetObjectGuid();

        float fX, fY, fZ;
        pNearestTuber->GetNearPoint(fX, fY, fZ, 5.0f);
        m_creature->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
        m_bIsMovementActive = true;
        SetFollowPaused(true);
    }

    bool IsValidTuber(GameObject* tuber)
    {
        Unit* viewPoint = m_creature;

        // Do LOS checks from Player if exists
        if (Unit* owner = m_creature->GetOwner())
            viewPoint = owner;

        if (tuber->isSpawned() || !tuber->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND) || !tuber->IsWithinLOSInMap(viewPoint))
            return false;

        // Check if tuber is in list of already found tubers
        for (std::list<uint64>::const_iterator itr2 = m_foundTubers.begin(); itr2 != m_foundTubers.end(); ++itr2)
            if (tuber->GetObjectGuid() == (*itr2))
                return false;

        // Check that tuber is not more than 15 yards above or below current position
        if (fabs(viewPoint->GetPositionZ() - tuber->GetPositionZ()) > 15)
            return false;

        return true;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_bIsMovementActive)
            return;

        if (HasFollowState(STATE_FOLLOW_PAUSED))
        {
            if (m_followPausedTimer < uiDiff)
                SetFollowPaused(false);
            else
                m_followPausedTimer -= uiDiff;
        }

        FollowerAI::UpdateAI(uiDiff);
    }

};

CreatureAI* GetAI_npc_snufflenose_gopher(Creature* pCreature)
{
    return new npc_snufflenose_gopherAI(pCreature);
}

bool EffectDummyCreature_npc_snufflenose_gopher(Unit* pCaster, uint32 uiSpellId, uint32 uiEffIndex, Creature* pCreatureTarget)
{
    // always check spellid and effectindex
    if (uiSpellId == SPELL_SNUFFLENOSE_COMMAND && uiEffIndex == 0)
    {
        if (pCreatureTarget->GetEntry() == NPC_SNUFFLENOSE_GOPHER)
        {
            // Do nothing if player has not targeted gopher
            if (pCaster->GetSelection() != pCreatureTarget->GetObjectGuid())
            {
                // Send Spell_FAILED_BAD_TARGETS
                pCreatureTarget->SendPetCastFail(uiSpellId, (SpellCastResult)0x0A);
                return false;
            }

            DoScriptText(SAY_GOPHER_COMMAND, pCreatureTarget, pCaster);

            if (npc_snufflenose_gopherAI* pGopherAI = dynamic_cast<npc_snufflenose_gopherAI*>(pCreatureTarget->AI()))
            {
                if (pGopherAI->HasFollowState(STATE_FOLLOW_PAUSED))
                {
                    pGopherAI->SetFollowPaused(false);
                    pGopherAI->m_bIsMovementActive = false;
                    pGopherAI->m_targetTuberGuid = 0;
                }
                else
                    pGopherAI->DoFindNewTuber();
            }
        }
        // always return true when we are handling this spell and effect
        return true;
    }

    return false;
}

void AddSC_razorfen_kraul()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_willix";
    newscript->GetAI = &GetAI_npc_willix;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_willix;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_deaths_head_ward_keeper";
    newscript->GetAI = &GetAI_npc_deaths_head_ward_keeper;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_snufflenose_gopher";
    newscript->GetAI = &GetAI_npc_snufflenose_gopher;
    newscript->pEffectDummyNPC = &EffectDummyCreature_npc_snufflenose_gopher;
    newscript->RegisterSelf();
}

