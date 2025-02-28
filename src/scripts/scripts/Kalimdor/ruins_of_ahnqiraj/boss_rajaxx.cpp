// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* 
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2014 Hellground <http://hellground.net/>
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
SDName: Boss_Rajaxx
SD%Complete:
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "def_ruins_of_ahnqiraj.h"

enum BossRajax
{
    // Event yells
    SAY_ANDOROV_INTRO_1         = -1509004,
    SAY_ANDOROV_INTRO_2         = -1509031,
    SAY_ANDOROV_INTRO_3         = -1509003,
    SAY_ANDOROV_INTRO_4         = -1509029,
    SAY_ANDOROV_ATTACK_START    = -1509030,

    // Rajaxx kills Andorov
    SAY_KILLS_ANDOROV           = -1509016,

    // probably related to the opening of AQ event
    SAY_UNK1                    = -1509011,
    SAY_UNK2                    = -1509012,
    SAY_UNK3                    = -1509013,
    SAY_UNK4                    = -1509014,

    // gossip items
    GOSSIP_TEXT_ID_INTRO        = 7883,
    GOSSIP_TEXT_ID_TRADE        = 8305,

    GOSSIP_ITEM_START           = -3509000,
    GOSSIP_ITEM_TRADE           = -3509001,

    // Andorov spells
    SPELL_AURA_OF_COMMAND       = 25516,
    SPELL_BASH                  = 25515,
    SPELL_STRIKE                = 22591,

    // Kaldorei spell
    SPELL_CLEAVE                = 26350,
    SPELL_MORTAL_STRIKE         = 16856,

    POINT_ID_MOVE_INTRO         = 2,
    POINT_ID_MOVE_ATTACK        = 4,
};

static const DialogueEntry aIntroDialogue[] =
{
    {SAY_ANDOROV_INTRO_1,       NPC_GENERAL_ANDOROV,    7000},
    {SAY_ANDOROV_INTRO_2,       NPC_GENERAL_ANDOROV,    0},
    {SAY_ANDOROV_INTRO_3,       NPC_GENERAL_ANDOROV,    4000},
    {SAY_ANDOROV_INTRO_4,       NPC_GENERAL_ANDOROV,    6000},
    {SAY_ANDOROV_ATTACK_START,  NPC_GENERAL_ANDOROV,    0},
    {0, 0, 0},
};

struct npc_general_andorovAI : public ScriptedAI, private DialogueHelper
{
    npc_general_andorovAI(Creature* pCreature) : ScriptedAI(pCreature),
        DialogueHelper(aIntroDialogue), summons(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    SummonList summons;

    uint32 CommandAuraTimer;
    uint32 BashTimer;
    uint32 StrikeTimer;
    uint32 CheckForAlivePlayers;
    uint32 StartAttackTimer;

    void Reset()
    {
        CheckForAlivePlayers = 10000;
        CommandAuraTimer     = urand(1000, 3000);
        BashTimer            = urand(8000, 11000);
        StrikeTimer          = urand(2000, 5000);
        StartAttackTimer     = 0;

        if(pInstance->GetData(DATA_KURINNAXX) != DONE)
        {
            if(me->GetVisibility() != VISIBILITY_OFF)
                me->SetVisibility(VISIBILITY_OFF);
        }
        else if(pInstance->GetData(DATA_KURINNAXX) == DONE && me->GetVisibility() != VISIBILITY_OFF)
            summons.DespawnAll();

        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        if(pInstance)
        {
            if(Creature* Rajax = me->GetMap()->GetCreatureById(NPC_RAJAXX))
                if(Rajax->isAlive())
                    pInstance->SetData(DATA_GENERAL_RAJAXX, NOT_STARTED);
        }
        
        me->setFaction(250);
    }

    void SummonAllWaves()
    {
        if(pInstance)
        {
            for(int i = 0; i < 49; i++)
            {
                Creature* pWaveSummon = me->SummonCreature(AllRajaxWaves[i].Entry, AllRajaxWaves[i].m_fX, AllRajaxWaves[i].m_fY, AllRajaxWaves[i].m_fZ, AllRajaxWaves[i].m_fO, TEMPSUMMON_MANUAL_DESPAWN, 0);
                if(AllRajaxWaves[i].SpecialFlags == 1)
                {
                    if(pWaveSummon)
                    {
                        if(!pWaveSummon->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                            pWaveSummon->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
                        pWaveSummon->SetReactState(REACT_PASSIVE);
                    }
                }
            }
        }
    }

    void EventPulse(Unit* pSender, uint32 PulseEventNumber)
    {
        if(!me->isAlive())
            me->Respawn();

        if(me->GetVisibility() != VISIBILITY_ON)
            me->SetVisibility(VISIBILITY_ON);

        summons.DespawnAll();
        for (uint8 i = 0; i < MAX_HELPERS; ++i)
            me->SummonCreature(aAndorovSpawnLocs[i].Entry, aAndorovSpawnLocs[i].m_fX, aAndorovSpawnLocs[i].m_fY, aAndorovSpawnLocs[i].m_fZ, aAndorovSpawnLocs[i].m_fO, TEMPSUMMON_MANUAL_DESPAWN, 0);
        me->SetWalk(false);
        me->SetSpeed(MOVE_RUN, 1.5);
        
    }

    void JustSummoned(Creature* summon)
    {
        summons.Summon(summon);
    }

    void AttackStart(Unit* who)
    {
        std::list<uint64> KaldoreiGuids;
        pInstance->GetKaldoreiGuidList(KaldoreiGuids);

        for (std::list<uint64>::const_iterator itr = KaldoreiGuids.begin(); itr != KaldoreiGuids.end(); ++itr)
        {
            if (Creature* pKaldorei = me->GetMap()->GetCreature(*itr))
                pKaldorei->AI()->AttackStart(who);
        }
        ScriptedAI::AttackStart(who);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void JustDied(Unit* pKiller)
    {
        summons.DespawnAll();
        if (pKiller->GetEntry() != NPC_RAJAXX)
            return;

        // Yell when killed by Rajaxx
        if (pInstance)
        {
            if (Creature* pRajaxx = Unit::GetCreature(*me, pInstance->GetData64(DATA_GENERAL_RAJAXX)))
                DoScriptText(SAY_KILLS_ANDOROV, pRajaxx);
        }
        
        if(pInstance)
            pInstance->SetData(DATA_GENERAL_RAJAXX, NOT_STARTED);
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if (uiType != POINT_MOTION_TYPE)
            return;

        switch (uiPointId)
        {
            case 1:
            {
                me->GetMotionMaster()->MovePoint(2, aAndorovMoveLocs[1].m_fX, aAndorovMoveLocs[1].m_fY, aAndorovMoveLocs[1].m_fZ);
                break;
            }
            case 2:
            {
                StartNextDialogueText(SAY_ANDOROV_INTRO_3);
                if (pInstance)
                {
                    SummonAllWaves();
                    if(pInstance->GetData(DATA_GENERAL_RAJAXX) != DONE || pInstance->GetData(DATA_GENERAL_RAJAXX) != IN_PROGRESS)
                        pInstance->SetData(DATA_GENERAL_RAJAXX, IN_PROGRESS);
                    StartAttackTimer = 3000;
                }
                break;
            }
            default: break;
        }
    }

    void EnterEvadeMode()
    {
        if (!pInstance)
            return;

        Map *map = me->GetMap();

        Map::PlayerList const &PlayerList = map->GetPlayers();
        for(Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
        {
            if (Player* i_pl = i->getSource())
            {
                if(me->GetDistance2d(i_pl) <= 170 && i_pl->isAlive())
                    return;
            }
        }

        me->RemoveAllAurasNotCreatureAddon();
        me->DeleteThreatList();
        me->CombatStop(true);

        /*if (me->isAlive())
        {
            // reset to combat position
            if (PointId >= 4)
                me->GetMotionMaster()->MovePoint(POINT_ID_MOVE_ATTACK, aAndorovMoveLocs[4].m_fX, aAndorovMoveLocs[4].m_fY, aAndorovMoveLocs[4].m_fZ);
            // reset to intro position
            else
                me->GetMotionMaster()->MovePoint(POINT_ID_MOVE_INTRO, aAndorovMoveLocs[2].m_fX, aAndorovMoveLocs[2].m_fY, aAndorovMoveLocs[2].m_fZ);
        }*/

        me->SetLootRecipient(NULL);
        pInstance->SetData(DATA_GENERAL_RAJAXX, FAIL);
        Reset();
    }

    // Wrapper to start initialize Kaldorei followers
    void DoInitializeFollowers()
    {
        if (!pInstance)
            return;

        std::list<uint64> KaldoreiGuids;
        pInstance->GetKaldoreiGuidList(KaldoreiGuids);

        uint32 Count = 0;
        for (std::list<uint64>::const_iterator itr = KaldoreiGuids.begin(); itr != KaldoreiGuids.end(); ++itr)
        {
            if (Creature* pKaldorei = me->GetMap()->GetCreature(*itr))
            {
                float fAngle = Count < 4 ? M_PI/4 - (Count*2*M_PI/4) : 0.0f;
                pKaldorei->GetMotionMaster()->MoveFollow(me, 1.5f, fAngle);
            }
            ++Count;
        }
    }

    // Wrapper to start the event
    void DoMoveToEventLocation()
    {
        me->GetMotionMaster()->MovePoint(1, aAndorovMoveLocs[0].m_fX, aAndorovMoveLocs[0].m_fY, aAndorovMoveLocs[0].m_fZ);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        StartNextDialogueText(SAY_ANDOROV_INTRO_1);
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry)
    {
        switch (uiEntry)
        {
            case NPC_GENERAL_ANDOROV:        return me;
            default:
                return NULL;
        }

    }

    void UpdateAI(const uint32 diff)
    {
        DialogueUpdate(diff);

        if(StartAttackTimer)
        {
            if(StartAttackTimer <= diff)
            {
                if (Creature* pQeez = me->GetMap()->GetCreatureById(NPC_CAPTAIN_QEEZ))
                {
                    if (!pQeez->isAlive())
                        return;

                    if(pQeez->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                        pQeez->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                
                    pQeez->SetWalk(false);
                
                        if(!pQeez->IsInCombat())
                            pQeez->SetInCombatWith(me);
                        pQeez->AI()->AttackStart(me);
                }
                StartAttackTimer = 0;
            } else StartAttackTimer -= diff;
        }

        if(CheckForAlivePlayers <= diff)
        {
            if(pInstance->GetData(DATA_GENERAL_RAJAXX) == IN_PROGRESS)
            {
                if(me->GetMap()->GetAlivePlayersCountExceptGMs() == 0)
                    me->Kill(me);
                CheckForAlivePlayers = 5000;
            }
        } else CheckForAlivePlayers -= diff;

        if (!UpdateVictim())
            return;

        if (BashTimer < diff)
        {
            AddSpellToCast(SPELL_BASH, CAST_TANK);
            BashTimer = urand(12000, 15000);
        }
        else
            BashTimer -= diff;

        if (StrikeTimer < diff)
        {
            AddSpellToCast(SPELL_STRIKE, CAST_TANK);
            StrikeTimer = urand(4000, 6000);
        }
        else
            StrikeTimer -= diff;

        if (CommandAuraTimer < diff)
        {
            AddSpellToCast(SPELL_AURA_OF_COMMAND, CAST_SELF);
            CommandAuraTimer = urand(30000, 45000);
        }
        else
            CommandAuraTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_general_andorov(Creature* pCreature)
{
    return new npc_general_andorovAI(pCreature);
}

bool GossipHello_npc_general_andorov(Player* pPlayer, Creature* pCreature)
{
    if (instance_ruins_of_ahnqiraj* pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData())
    {
        if (pInstance->GetData(DATA_GENERAL_RAJAXX) == IN_PROGRESS)
            return true;

        if (pInstance->GetData(DATA_GENERAL_RAJAXX) == DONE)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, pPlayer->GetSession()->GetHellgroundString(16342)/*GOSSIP_ITEM_TRADE*/, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        if (pInstance->GetData(DATA_GENERAL_RAJAXX) == NOT_STARTED || pInstance->GetData(DATA_GENERAL_RAJAXX) == FAIL)
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, pPlayer->GetSession()->GetHellgroundString(16343)/*GOSSIP_ITEM_START*/, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        CAST_AI(npc_general_andorovAI, pCreature->AI())->EventPulse(pPlayer, 0);

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_INTRO, pCreature->GetGUID());
    }

    return true;
}

bool GossipSelect_npc_general_andorov(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_TRADE:
            pPlayer->SEND_VENDORLIST(pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            if (npc_general_andorovAI* pAndorovAI = dynamic_cast<npc_general_andorovAI*>(pCreature->AI()))
                pAndorovAI->DoMoveToEventLocation();
            pPlayer->CLOSE_GOSSIP_MENU();
            break;
        }
        default: break;
    }
    return true;
}

struct npc_kaldorei_eliteAI : public ScriptedAI
{
    npc_kaldorei_eliteAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 StrikeTimer;

    void Reset()
    {
        CleaveTimer = urand(2000, 4000);
        StrikeTimer = urand(8000, 11000);
        if (me->isAlive())
        {
            if (Creature* pAndorov = me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV))
            {
                if (pAndorov->isAlive())
                    me->GetMotionMaster()->MoveFollow(pAndorov, me->GetDistance(pAndorov), M_PI/4 - (4*2*M_PI/4));
            }
        }
    }

    void EnterEvadeMode()
    {
        if (!pInstance)
            return;

        me->RemoveAllAurasNotCreatureAddon();
        me->DeleteThreatList();
        me->CombatStop(true);

        // reset only to the last position
        if (me->isAlive())
        {
            if (Creature* pAndorov = me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV))
            {
                if (pAndorov->isAlive())
                    me->GetMotionMaster()->MoveFollow(pAndorov, me->GetDistance(pAndorov), M_PI/4 - (4*2*M_PI/4));
            }
        }

        me->SetLootRecipient(NULL);

        Reset();
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (CleaveTimer < diff)
        {
            AddSpellToCast(SPELL_CLEAVE, CAST_TANK);
            CleaveTimer = urand(5000, 7000);
        }
        else
            CleaveTimer -= diff;

        if (StrikeTimer < diff)
        {
            AddSpellToCast(SPELL_MORTAL_STRIKE, CAST_TANK);
            StrikeTimer = urand(9000, 13000);
        }
        else
            StrikeTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kaldorei_elite(Creature* pCreature)
{
    return new npc_kaldorei_eliteAI(pCreature);
}

struct npc_colonel_zerranAI : public ScriptedAI
{
    npc_colonel_zerranAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 SundenArmorTimer;
    uint32 EnlargeTimer;

    void Reset()
    {
        CleaveTimer = 3000;
        SundenArmorTimer = 1000;
        EnlargeTimer = 15000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void SetFormation()
    {
        Unit* pVictim = me->GetVictim() ? me->GetVictim() : me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV);
        std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
        for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
        std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
        for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->InsertGuidInArmyList(me->GetGUID());
        SetFormation();
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        if(SundenArmorTimer < diff)
        {
            AddSpellToCast(24317, CAST_TANK);
            SundenArmorTimer = urand(1000, 3000);
        } else SundenArmorTimer -= diff;

        if(EnlargeTimer < diff)
        {
            AddSpellToCast(25462, CAST_SELF);
            std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
            for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
            {
                if ((*itr)->isAlive())
                    AddSpellToCast((*itr), 25462);
            }
            std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
            for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
            {
                if ((*itr)->isAlive())
                    AddSpellToCast((*itr), 25462);
            }
            EnlargeTimer = 60000;
        } else EnlargeTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_colonel_zerran(Creature* pCreature)
{
    return new npc_colonel_zerranAI(pCreature);
}

struct npc_major_pakkonAI : public ScriptedAI
{
    npc_major_pakkonAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 SundenArmorTimer;
    uint32 SweepingSlamTimer;

    void Reset()
    {
        CleaveTimer = 3000;
        SundenArmorTimer = 1000;
        SweepingSlamTimer = 5000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void SetFormation()
    {
        Unit* pVictim = me->GetVictim() ? me->GetVictim() : me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV);
        std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
        for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
        std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
        for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->InsertGuidInArmyList(me->GetGUID());
        SetFormation();
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        if(SundenArmorTimer < diff)
        {
            AddSpellToCast(24317, CAST_TANK);
            SundenArmorTimer = urand(1000, 3000);
        } else SundenArmorTimer -= diff;

        if(SweepingSlamTimer < diff)
        {
            AddSpellToCast(25322, CAST_TANK);
            SweepingSlamTimer = urand(7000, 10000);
        } else SweepingSlamTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_major_pakkon(Creature* pCreature)
{
    return new npc_major_pakkonAI(pCreature);
}

struct npc_major_yeggethAI : public ScriptedAI // wowwiki says about blessing of protection, but wowhead not
{
    npc_major_yeggethAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 SundenArmorTimer;

    void Reset()
    {
        CleaveTimer = 3000;
        SundenArmorTimer = 1000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void SetFormation()
    {
        Unit* pVictim = me->GetVictim() ? me->GetVictim() : me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV);
        std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
        for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
        std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
        for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->InsertGuidInArmyList(me->GetGUID());
        SetFormation();
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        if(SundenArmorTimer < diff)
        {
            AddSpellToCast(24317, CAST_TANK);
            SundenArmorTimer = urand(1000, 3000);
        } else SundenArmorTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_major_yeggeth(Creature* pCreature)
{
    return new npc_major_yeggethAI(pCreature);
}

struct npc_captain_xurremAI : public ScriptedAI
{
    npc_captain_xurremAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 SundenArmorTimer;
    uint32 ShockwaveTimer;

    void Reset()
    {
        CleaveTimer = 3000;
        SundenArmorTimer = 1000;
        ShockwaveTimer = 5000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void SetFormation()
    {
        Unit* pVictim = me->GetVictim() ? me->GetVictim() : me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV);
        std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
        for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
        std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
        for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->InsertGuidInArmyList(me->GetGUID());
        SetFormation();
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        if(SundenArmorTimer < diff)
        {
            AddSpellToCast(24317, CAST_TANK);
            SundenArmorTimer = urand(1000, 3000);
        } else SundenArmorTimer -= diff;

        if(ShockwaveTimer < diff)
        {
            AddSpellToCast(25425, CAST_TANK);
            ShockwaveTimer = urand(5000, 7000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captain_xurrem(Creature* pCreature)
{
    return new npc_captain_xurremAI(pCreature);
}

struct npc_captain_drennAI : public ScriptedAI
{
    npc_captain_drennAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 SundenArmorTimer;
    uint32 LightningCloudTimer;

    void Reset()
    {
        CleaveTimer = 3000;
        SundenArmorTimer = 1000;
        LightningCloudTimer = 2000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void SetFormation()
    {
        Unit* pVictim = me->GetVictim() ? me->GetVictim() : me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV);
        std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
        for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
        std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
        for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->InsertGuidInArmyList(me->GetGUID());
        SetFormation();
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(LightningCloudTimer < diff)
        {
            AddSpellToCast(26550, CAST_RANDOM);
            LightningCloudTimer = urand(15000, 20000);
        } else LightningCloudTimer -= diff;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        if(SundenArmorTimer < diff)
        {
            AddSpellToCast(24317, CAST_TANK);
            SundenArmorTimer = urand(1000, 3000);
        } else SundenArmorTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captain_drenn(Creature* pCreature)
{
    return new npc_captain_drennAI(pCreature);
}

struct npc_captain_tuubidAI : public ScriptedAI
{
    npc_captain_tuubidAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 SundenArmorTimer;
    uint32 AttackOrderTimer;

    void Reset()
    {
        CleaveTimer = 2000;
        SundenArmorTimer = 1000;
        AttackOrderTimer = 5000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void SetFormation()
    {
        Unit* pVictim = me->GetVictim() ? me->GetVictim() : me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV);
        std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
        for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
        std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
        for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    if(!(*itr)->IsInCombat())
                        (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                (*itr)->AI()->AttackStart(me->GetVictim());
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->InsertGuidInArmyList(me->GetGUID());
        SetFormation();
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(AttackOrderTimer < diff)
        {
            if(Unit* pTarget = SelectUnit(SELECT_TARGET_RANDOM, 0, 200.0f, true))
            {
                AddSpellToCast(pTarget, 25471);
                std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
                for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
                {
                    if ((*itr)->isAlive())
                    {
                        DoResetThreat();
                        (*itr)->AI()->AttackStart(pTarget);
                    }
                }
                std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
                for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
                {
                    if ((*itr)->isAlive())
                    {
                        DoResetThreat();
                        (*itr)->AI()->AttackStart(pTarget);
                    }
                }
            }
            AttackOrderTimer = urand(30000, 45000);
        } else AttackOrderTimer -= diff;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        if(SundenArmorTimer < diff)
        {
            AddSpellToCast(24317, CAST_TANK);
            SundenArmorTimer = urand(1000, 3000);
        } else SundenArmorTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captain_tuubid(Creature* pCreature)
{
    return new npc_captain_tuubidAI(pCreature);
}

struct npc_captain_qeezAI : public ScriptedAI
{
    npc_captain_qeezAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
        Reset();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;
    uint32 SundenArmorTimer;

    void Reset()
    {
        CleaveTimer = 2000;
        SundenArmorTimer = 1000;
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void SetFormation()
    {
        Unit* pVictim = me->GetVictim() ? me->GetVictim() : me->GetMap()->GetCreatureById(NPC_GENERAL_ANDOROV);
        std::list<Creature*> SwarmguardNeedlerList = FindAllCreaturesWithEntry(NPC_SWARMGUARD_NEEDLER, 15);
        for (std::list<Creature*>::iterator itr = SwarmguardNeedlerList.begin(); itr != SwarmguardNeedlerList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
        std::list<Creature*> QirajiWarriorList = FindAllCreaturesWithEntry(NPC_QIRAJI_WARRIOR, 15);
        for (std::list<Creature*>::iterator itr = QirajiWarriorList.begin(); itr != QirajiWarriorList.end(); ++itr)
        {
            if ((*itr)->isAlive())
            {
                if(pVictim)
                {
                    (*itr)->SetInCombatWith(pVictim);
                    (*itr)->AI()->AttackStart(pVictim);
                }
                if((*itr)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                    (*itr)->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE, false);
                if(pInstance)
                    pInstance->InsertGuidInArmyList((*itr)->GetGUID());
            }
        }
    }

    void EnterCombat(Unit* who)
    {
        if(pInstance)
            pInstance->InsertGuidInArmyList(me->GetGUID());
        SetFormation();
        DoZoneInCombat(80.0f);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        if(SundenArmorTimer < diff)
        {
            AddSpellToCast(24317, CAST_TANK);
            SundenArmorTimer = urand(1000, 3000);
        } else SundenArmorTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_captain_qeez(Creature* pCreature)
{
    return new npc_captain_qeezAI(pCreature);
}

struct npc_qiraji_warriorAI : public ScriptedAI
{
    npc_qiraji_warriorAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 UppercutTimer;
    uint32 ThunderclapTimer;
    bool Enraged;

    void Reset()
    {
        UppercutTimer = 5000;
        ThunderclapTimer = 3000;
        Enraged = false;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(UppercutTimer < diff)
        {
            AddSpellToCast(10966, CAST_RANDOM);
            UppercutTimer = 10000;
        } else UppercutTimer -= diff;

        if(ThunderclapTimer < diff)
        {
            AddSpellToCast(15588, CAST_TANK);
            ThunderclapTimer = urand(6000, 8000);
        } else ThunderclapTimer -= diff;
        
        if(HealthBelowPct(25) && !Enraged)
        {
            AddSpellToCast(8599, CAST_SELF);
            Enraged = true;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_qiraji_warrior(Creature* pCreature)
{
    return new npc_qiraji_warriorAI(pCreature);
}

struct npc_swarmguard_needlerAI : public ScriptedAI
{
    npc_swarmguard_needlerAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)pCreature->GetInstanceData();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 CleaveTimer;

    void Reset()
    {
        CleaveTimer = 1000;
    }

    void EnterCombat(Unit*)
    {
        DoZoneInCombat(80.0f);
    }

    void DamageTaken(Unit* done_by, uint32 &damage)
    {
        /*if(done_by->GetTypeId() == TYPEID_UNIT)
            DoModifyThreatPercent(done_by, -101);*/
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(CleaveTimer < diff)
        {
            AddSpellToCast(40504, CAST_TANK);
            CleaveTimer = urand(3000, 5000);
        } else CleaveTimer -= diff;

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_swarmguard_needler(Creature* pCreature)
{
    return new npc_swarmguard_needlerAI(pCreature);
}

struct boss_rajaxxAI : public ScriptedAI
{
    boss_rajaxxAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (instance_ruins_of_ahnqiraj*)c->GetInstanceData();
        Reset();
    }

    instance_ruins_of_ahnqiraj* pInstance;

    uint32 Disarm_Timer;
    uint32 Thundercrash_Timer;
    
    void Reset()
    {
        Thundercrash_Timer = 30000; //propably wrong
        Disarm_Timer = 20000;
        /*if(!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetNonAttackableFlag(UNIT_FLAG_NON_ATTACKABLE);
        }*/ // turned off due to bad scripts. doesnt activate a lot of the times. part 1/2
        if(pInstance)
            pInstance->SetData(DATA_GENERAL_RAJAXX, NOT_STARTED);
    }
    
    void EnterCombat(Unit* who)
    {
        if(pInstance)
            DoZoneInCombat(80.0f);
    }

    void EnterEvadeMode()
    {
        DoScriptText(SAY_DEAGGRO, me);
        ScriptedAI::EnterEvadeMode();
    }
    
    void JustDied(Unit * killer)
    {
        if (pInstance)
            pInstance->SetData(DATA_GENERAL_RAJAXX, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        //Thundercrash_Timer
        if (Thundercrash_Timer < diff)
        {
            AddSpellToCast(25599, CAST_NULL);
            Thundercrash_Timer = 30000;
        } else Thundercrash_Timer -= diff;

        if (Disarm_Timer < diff)
        {
            AddSpellToCast(6713, CAST_TANK);
            Disarm_Timer = 20000;
        } else Disarm_Timer -= diff;
        
        //keep being frenzied if hp<30%
        if(me->GetHealthPercent() < 30)
        {
            if (!me->HasAura(8269))
                DoCast(me, 8269);
        }
        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_rajaxx(Creature *_Creature)
{
    return new boss_rajaxxAI (_Creature);
}

void AddSC_boss_rajaxx()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_general_andorov";
    pNewScript->GetAI = &GetAI_npc_general_andorov;
    pNewScript->pGossipHello = &GossipHello_npc_general_andorov;
    pNewScript->pGossipSelect = &GossipSelect_npc_general_andorov;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_kaldorei_elite";
    pNewScript->GetAI = &GetAI_npc_kaldorei_elite;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_colonel_zerran";
    pNewScript->GetAI = &GetAI_npc_colonel_zerran;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_major_pakkon";
    pNewScript->GetAI = &GetAI_npc_major_pakkon;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_major_yeggeth";
    pNewScript->GetAI = &GetAI_npc_major_yeggeth;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_captain_xurrem";
    pNewScript->GetAI = &GetAI_npc_captain_xurrem;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_captain_drenn";
    pNewScript->GetAI = &GetAI_npc_captain_drenn;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_captain_tuubid";
    pNewScript->GetAI = &GetAI_npc_captain_tuubid;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_captain_qeez";
    pNewScript->GetAI = &GetAI_npc_captain_qeez;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_qiraji_warrior";
    pNewScript->GetAI = &GetAI_npc_qiraji_warrior;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_swawrmguard_needler";
    pNewScript->GetAI = &GetAI_npc_swarmguard_needler;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_rajaxx";
    pNewScript->GetAI = &GetAI_boss_rajaxx;
    pNewScript->RegisterSelf();
}