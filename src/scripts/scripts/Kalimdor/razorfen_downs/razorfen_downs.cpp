// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Razorfen_Downs
SD%Complete: 70
SDComment: TODO: npc_belnistrasz - combat AI/spells
SDCategory: Razorfen Downs
EndScriptData */

/* ContentData
npc_henry_stern
npc_belnistrasz
EndContentData */

//#include "ScriptedPch.h"
#include "precompiled.h"
#include "GossipDef.h"
#include "razorfen_downs.h"
#include "escort_ai.h"

/*###
# npc_henry_stern
####*/

enum eEnums
{
    SPELL_GOLDTHORN_TEA = 13028,
    SPELL_TEACHING_GOLDTHORN_TEA = 13029,
    SPELL_MIGHT_TROLLS_BLOOD_POTION = 3451,
    SPELL_TEACHING_MIGHTY_TROLLS_BLOOD_POTION = 13030,
    GOSSIP_TEXT_TEA_ANSWER = 2114,
    GOSSIP_TEXT_POTION_ANSWER = 2115,
    SAY_THANKS_AGAIN = -2100038,
};

#define GOSSIP_ITEM_TEA     16340
#define GOSSIP_ITEM_POTION  16341

bool GossipHello_npc_henry_stern(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetBaseSkillValue(SKILL_COOKING) >= 175 && !pPlayer->HasSpell(SPELL_GOLDTHORN_TEA))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_TEA), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (pPlayer->GetBaseSkillValue(SKILL_ALCHEMY) >= 180 && !pPlayer->HasSpell(SPELL_MIGHT_TROLLS_BLOOD_POTION))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(GOSSIP_ITEM_POTION), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_henry_stern(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CastSpell(pPlayer, SPELL_TEACHING_GOLDTHORN_TEA, true);
        DoScriptText(SAY_THANKS_AGAIN, pCreature, pPlayer);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_TEA_ANSWER, pCreature->GetGUID());
    }

    if (uiAction == GOSSIP_ACTION_INFO_DEF + 2)
    {
        pPlayer->CastSpell(pPlayer, SPELL_TEACHING_MIGHTY_TROLLS_BLOOD_POTION, true);
        DoScriptText(SAY_THANKS_AGAIN, pCreature, pPlayer);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_POTION_ANSWER, pCreature->GetGUID());
    }
    return true;
}

/*######
## go_gong
######*/

bool GOUse_go_gong(Player* pPlayer, GameObject* pGO)
{
    //basic support, not blizzlike data is missing...
    ScriptedInstance* pInstance = (pGO->GetInstanceData());

    if (pInstance)
    {
        pInstance->SetData(DATA_GONG_WAVES, pInstance->GetData(DATA_GONG_WAVES) + 1);
        pGO->PlayDistanceSound(4654, pPlayer);   //gong sound
        return true;
    }

    return false;
}

enum eTombCreature
{
    SPELL_WEB = 745,
    SPELL_CURSE_OF_TUTENKASH = 12255,
    SPELL_WEB_SPRAY = 12252
};

struct npc_tomb_creatureAI : public ScriptedAI
{
    npc_tomb_creatureAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (pCreature->GetInstanceData());
    }

    ScriptedInstance* pInstance;

    uint32 uiWebTimer;
    uint32 uiCurseTimer;
    uint32 uiWebSprayTimer;

    void Reset()
    {
        uiWebTimer = urand(5000, 8000);
        uiCurseTimer = 16000;
        uiWebSprayTimer = 5000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!UpdateVictim())
            return;

        //from acid
        if (m_creature->GetEntry() == CREATURE_TOMB_REAVER)
        {
            if (uiWebTimer <= uiDiff)
            {
                DoCast(m_creature->GetVictim(), SPELL_WEB);
                uiWebTimer = urand(7000, 16000);
            }
            else
                uiWebTimer -= uiDiff;
        }

        //Tuten'Kash timers
        if (m_creature->GetEntry() == CREATURE_TUTEN_KASH)
        {
            if (uiCurseTimer < uiDiff)
            {
                DoCast(m_creature, SPELL_CURSE_OF_TUTENKASH);
                uiCurseTimer = urand(40000, 60000);
            }
            else
                uiCurseTimer -= uiDiff;

            if (uiWebSprayTimer < uiDiff)
            {
                DoCast(m_creature, SPELL_WEB_SPRAY);
                uiWebSprayTimer = urand(17000, 25000);
            }
            else
                uiWebSprayTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* pKiller)
    {
        if (pInstance)
        {
            pInstance->SetData(DATA_GONG_WAVES, pInstance->GetData(DATA_GONG_WAVES) + 1);

            if (m_creature->GetEntry() == CREATURE_TUTEN_KASH)
                pInstance->SetData(BOSS_TUTEN_KASH, DONE);
        }

    }
};

CreatureAI* GetAI_npc_tomb_creature(Creature* pCreature)
{
    return new npc_tomb_creatureAI(pCreature);
}

/*######
## npc_belnistrasz
######*/
enum belnistraszEnum
{
    QUEST_BELNISTRASZ               = 3525,
    BELNISTRASZ_SAY_START           = -2100025,
    BELNISTRASZ_SAY_AGGRO           = -2100027,/* should be "You'll rue the day you crossed me, <mob name>" no idea how to do that.
                                           also mobs does not pull on him on the way to event */
    BELNISTRASZ_SAY_IDOL            = -2100026,
    BELNISTRASZ_YELL_3MIN           = -2100028,
    BELNISTRASZ_YELL_2MIN           = -2100029,
    BELNISTRASZ_YELL_1MIN           = -2100030,
    BELNISTRASZ_YELL_COMPLETE       = -2100031,
    BELNISTRASZ_SAY_PLAGUEMAW_1     = -2100032,
    BELNISTRASZ_SAY_PLAGUEMAW_2     = -2100035,
    BELNISTRASZ_SAY_PLAGUEMAW_3     = -2100036,
    BELNISTRASZ_SAY_PLAGUEMAW_4     = -2100037,
    OVEN_TARGET                     = 8662,
    SPELL_BELNISTRASZ_VISUAL        = 12774,
    SPELL_IDOM_ROOM_CAMERA_SHAKE    = 12816,
    SPELL_ARCANE_INTELLECT          =  13326,
    SPELL_FIREBALL                  =  9053,
    SPELL_FROST_NOVA                =  11831,
    MOB_GEOMANCER                   = 7335,
    MOB_BOAR                        = 7333,
    MOB_WARRIOR                     = 7327,
    BOSS_PLAGUEMAW                  = 7356,
    GO_BELNISTRASZ_BRAZIER          = 152097,
};

struct npc_belnistraszAI : public npc_escortAI
{
    npc_belnistraszAI(Creature *c) : npc_escortAI(c) {}

    Timer FireballTimer;
    Timer FrostNovaTimer;
    Timer wavetimer;
    uint8 waves;
    bool onplace;

    void Reset()
    {
        SetDespawnAtEnd(false);
        SetDespawnAtFar(false);
        onplace = false;
        wavetimer.Reset(0);
        FireballTimer.Reset(1000);
        FrostNovaTimer.Reset(urand(8000, 12000));
        waves = 0;

        if (!me->HasAura(SPELL_ARCANE_INTELLECT))
            DoCast(me, SPELL_ARCANE_INTELLECT);
    }
    void WaypointReached(uint32 i)
    {
        if (i == 13)
        {
            DoScriptText(BELNISTRASZ_SAY_IDOL, m_creature);
            Unit* bunny = FindCreature(OVEN_TARGET, 50, m_creature);
            if (bunny)
                DoCast(bunny, SPELL_BELNISTRASZ_VISUAL);
            onplace = true;
            wavetimer = 10000;
            SetCanAttack(false);
        }
    }

    void EnterCombat(Unit* who)
    {
        if (!onplace)
            DoScriptText(RAND(BELNISTRASZ_SAY_PLAGUEMAW_1, BELNISTRASZ_SAY_PLAGUEMAW_2, BELNISTRASZ_SAY_PLAGUEMAW_3, BELNISTRASZ_SAY_PLAGUEMAW_4), m_creature, who);
    }

    void JustSummoned(Creature* summon)
    {
        summon->AI()->AttackStart(m_creature);
        summon->AI()->DoZoneInCombat();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!onplace || waves > 10)
        {
            npc_escortAI::UpdateAI(diff);
            if (!me->GetVictim() && waves > 10)
            {
                m_creature->SummonGameObject(GO_BELNISTRASZ_BRAZIER, 2578, 946, 53.3, 0, 0, 0, 0, 0, 5*MINUTE*MILLISECONDS);
                GetPlayerForEscort()->GroupEventHappens(QUEST_BELNISTRASZ, m_creature);
                if (InstanceData* pInstance = me->GetInstanceData())
                    pInstance->SetData(DATA_BELNISTRASZ, DONE);
                me->ForcedDespawn();
            }

            if (UpdateVictim())
            {
                if (FireballTimer.Expired(diff))
                {
                    DoCast(m_creature->GetVictim(), SPELL_FIREBALL);
                    FireballTimer = 8000;
                }

                if (FrostNovaTimer.Expired(diff))
                {
                    if (m_creature->GetDistance(m_creature->GetVictim()) <= 8)
                    {
                        DoCast(m_creature, SPELL_FROST_NOVA);
                        FrostNovaTimer = 15000;
                    }
                }
            }
        }

        if (wavetimer.Expired(diff))
        {
            waves++;
            switch (waves)
            {
                case 3: DoScriptText(BELNISTRASZ_YELL_3MIN, m_creature); break;
                case 5: DoScriptText(BELNISTRASZ_YELL_2MIN, m_creature); break;
                case 7: DoScriptText(BELNISTRASZ_YELL_1MIN, m_creature); break;
                case 10: DoScriptText(BELNISTRASZ_YELL_COMPLETE, m_creature); break;
                default: break;
            }
            if (waves < 9)
            {
                m_creature->SummonCreature(MOB_GEOMANCER, 2565, 961, 51.7, 5.48, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                m_creature->SummonCreature(MOB_BOAR, 2568, 961, 51.7, 5.48, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                m_creature->SummonCreature(MOB_WARRIOR, 2585, 960, 52.3, 3.86, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                m_creature->SummonCreature(MOB_BOAR, 2585, 950, 52.3, 3.86, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                wavetimer = 30000;
            }
            else if (waves == 9)
            {
                m_creature->SummonCreature(BOSS_PLAGUEMAW, 2585, 956, 52.3, 3.86, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
                wavetimer = 15000;
            }
            else if (waves == 10)
            {
                SetCanAttack(true);
                m_creature->InterruptNonMeleeSpells(false, SPELL_BELNISTRASZ_VISUAL);
                DoCast(me, SPELL_IDOM_ROOM_CAMERA_SHAKE);
            }
        }
    }
};

bool QuestAccept_npc_belnistrasz(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_BELNISTRASZ)
        if (npc_escortAI* pEscortAI = CAST_AI(npc_belnistraszAI, pCreature->AI()))
        {
            pEscortAI->Start(true, true, pPlayer->GetGUID());
            DoScriptText(BELNISTRASZ_SAY_START, pCreature, pPlayer);
        }
    return true;
}

CreatureAI* GetAI_npc_belnistrasz(Creature* pCreature)
{
    return new npc_belnistraszAI(pCreature);
}

void AddSC_razorfen_downs()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "npc_henry_stern";
    newscript->pGossipHello = &GossipHello_npc_henry_stern;
    newscript->pGossipSelect = &GossipSelect_npc_henry_stern;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_gong";
    newscript->pGOUse = &GOUse_go_gong;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_tomb_creature";
    newscript->GetAI = &GetAI_npc_tomb_creature;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_belnistrasz";
    newscript->GetAI = &GetAI_npc_belnistrasz;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_belnistrasz;
    newscript->RegisterSelf();
}
