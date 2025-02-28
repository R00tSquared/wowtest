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
SDName: Zangarmarsh
SD%Complete: 99
SDComment: Quest support: 9785, 9803, 10009, 9752. Mark Of ... buffs. 9816
SDCategory: Zangarmarsh
EndScriptData */

/* ContentData
npcs_ashyen_and_keleth
npc_cooshcoosh
npc_elder_kuruti
npc_mortog_steamhead
npc_kayra_longmane
npc_baby_murloc
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npcs_ashyen_and_keleth
######*/

#define GOSSIP_ITEM_BLESS_ASH     16493
#define GOSSIP_ITEM_BLESS_KEL     16494
#define GOSSIP_REWARD_BLESS       -1000359
//#define TEXT_BLESSINGS        "<You need higher standing with Cenarion Expedition to recive a blessing.>"

bool GossipHello_npcs_ashyen_and_keleth(Player *player, Creature *creature )
{
    if (player->GetReputationMgr().GetRank(942) > REP_NEUTRAL)
    {
        if ( creature->GetEntry() == 17900)
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_BLESS_ASH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        if ( creature->GetEntry() == 17901)
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_BLESS_KEL), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    }
    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npcs_ashyen_and_keleth(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        creature->setPowerType(POWER_MANA);
        creature->SetMaxPower(POWER_MANA,200);             //set a "fake" mana value, we can't depend on database doing it in this case
        creature->SetPower(POWER_MANA,200);

        if ( creature->GetEntry() == 17900)                //check which creature we are dealing with
        {
            switch (player->GetReputationMgr().GetRank(942))
            {                                               //mark of lore
                case REP_FRIENDLY:
                    creature->CastSpell(player, 31808, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
                case REP_HONORED:
                    creature->CastSpell(player, 31810, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
                case REP_REVERED:
                    creature->CastSpell(player, 31811, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
                case REP_EXALTED:
                    creature->CastSpell(player, 31815, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
            }
        }

        if ( creature->GetEntry() == 17901)
        {
            switch (player->GetReputationMgr().GetRank(942))         //mark of war
            {
                case REP_FRIENDLY:
                    creature->CastSpell(player, 31807, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
                case REP_HONORED:
                    creature->CastSpell(player, 31812, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
                case REP_REVERED:
                    creature->CastSpell(player, 31813, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
                case REP_EXALTED:
                    creature->CastSpell(player, 31814, true);
                    DoScriptText(GOSSIP_REWARD_BLESS, creature);
                    break;
            }
        }
        player->CLOSE_GOSSIP_MENU();
        player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
    }
    return true;
}

/*######
## npc_cooshcoosh
######*/

#define GOSSIP_COOSH            16495

#define FACTION_HOSTILE_CO      45
#define FACTION_FRIENDLY_CO     35

#define SPELL_LIGHTNING_BOLT    9532

struct npc_cooshcooshAI : public ScriptedAI
{
    npc_cooshcooshAI(Creature* creature) : ScriptedAI(creature) {}

    Timer_UnCheked LightningBolt_Timer;

    void Reset()
    {
        LightningBolt_Timer.Reset(2000);
        me->setFaction(FACTION_FRIENDLY_CO);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (LightningBolt_Timer.Expired(diff))
        {
            DoCast(me->GetVictim(),SPELL_LIGHTNING_BOLT);
            LightningBolt_Timer = 5000;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_cooshcoosh(Creature *creature)
{
    return new npc_cooshcooshAI (creature);
}

bool GossipHello_npc_cooshcoosh(Player *player, Creature *creature )
{
    if( player->GetQuestStatus(10009) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(0, player->GetSession()->GetHellgroundString(GOSSIP_COOSH), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());
    return true;
}

bool GossipSelect_npc_cooshcoosh(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->CLOSE_GOSSIP_MENU();
        creature->setFaction(FACTION_HOSTILE_CO);
        ((npc_cooshcooshAI*)creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## npc_elder_kuruti
######*/

#define GOSSIP_ITEM_KUR1 16496
#define GOSSIP_ITEM_KUR2 16497
#define GOSSIP_ITEM_KUR3 16498

#define SPELL_CHAIN_LIGHTING    12058
#define SPELL_HEALING_WAVE      11986
#define SPELL_LIGHTING_SHIELD   12550
#define SPELL_SPITE_UMBRAFEN    32056

struct npc_elder_kurutiAI : public ScriptedAI
{
    npc_elder_kurutiAI(Creature* creature) : ScriptedAI(creature) {}

    Timer ChainLightingTimer;
    Timer HealingWaveTimer;
    Timer SpiteUmbaffenTimer;   

    void Reset()
    {
        ChainLightingTimer.Reset(1);
        HealingWaveTimer.Reset(1);
        SpiteUmbaffenTimer.Reset(1);

        ClearCastQueue();
    }

    void EnterCombat(Unit *who) 
    {
        m_creature->CastSpell(m_creature, SPELL_LIGHTING_SHIELD, false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (m_creature->GetHealthPercent() <= 50)
        {
            if (HealingWaveTimer.Expired(diff))
            {
                AddSpellToCast(m_creature, SPELL_HEALING_WAVE);
                HealingWaveTimer = 30000;
            }
        }

        if (ChainLightingTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_CHAIN_LIGHTING);
            ChainLightingTimer = 12000;
        }

        if (SpiteUmbaffenTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_SPITE_UMBRAFEN);
            SpiteUmbaffenTimer = 120000;
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_elder_kuruti(Creature *creature)
{
    return new npc_elder_kurutiAI (creature);
}

bool GossipHello_npc_elder_kuruti(Player *player, Creature *creature )
{
    if( player->GetQuestStatus(9803) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KUR1), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(9226,creature->GetGUID());

    return true;
}

bool GossipSelect_npc_elder_kuruti(Player *player, Creature *creature, uint32 sender, uint32 action )
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KUR2), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(9227, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM( 0, player->GetSession()->GetHellgroundString(GOSSIP_ITEM_KUR3), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(9229, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
        {
            if( !player->HasItemCount(24573,1) )
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 24573, 1);
                if( msg == EQUIP_ERR_OK )
                {
                    player->StoreNewItem( dest, 24573, true);
                }
                else
                    player->SendEquipError( msg,NULL,NULL );
            }
            player->SEND_GOSSIP_MENU(9231, creature->GetGUID());
            break;
        }
    }
    return true;
}

/*######
## npc_chieftain_mummaki
######*/

#define SPELL_VANISH    35205
#define SPELL_BACKSTAB  7159

struct npc_chieftain_mummakiAI : public ScriptedAI
{
    npc_chieftain_mummakiAI(Creature* creature) : ScriptedAI(creature) {}

    Timer VanishTimer;
    Timer BackstabTimer;

    void Reset()
    {
        VanishTimer.Reset(1);
        BackstabTimer.Reset(1);

        if (m_creature->HasAura(SPELL_VANISH))
            m_creature->RemoveAurasDueToSpell(SPELL_VANISH);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (m_creature->GetHealthPercent() <= 50)
        {
            if (VanishTimer.Expired(diff))
            {
                m_creature->CastSpell(m_creature, SPELL_VANISH, true);

                VanishTimer = 15000;
            }
        }

        if (BackstabTimer.Expired(diff))
        {
            if(!m_creature->GetVictim()->HasInArc(M_PI, m_creature))
            {
                if (m_creature->HasAura(SPELL_VANISH))
                    m_creature->RemoveAurasDueToSpell(SPELL_VANISH);

                m_creature->CastSpell(m_creature->GetVictim(), SPELL_BACKSTAB, false);
                BackstabTimer = 3000;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_chieftain_mummaki(Creature *creature)
{
    return new npc_chieftain_mummakiAI (creature);
}

/*######
## npc_coilfang_emissary
######*/

#define SPELL_FROSTBOLT         20297
#define SPELL_FROST_NOVA        11831
#define SPELL_WATER_SPOUT       39207
#define SPELL_ARCANE_EXPLOSION  33860

struct npc_coilfang_emissaryAI : public ScriptedAI
{
    npc_coilfang_emissaryAI(Creature* creature) : ScriptedAI(creature) {}

    Timer FrostboltTimer;
    Timer WaterSpoutTimer;
    Timer FrostNovaTimer;
    Timer ArcaneExplosionTimer;

    void Reset()
    {
        FrostboltTimer.Reset(1);
        WaterSpoutTimer.Reset(urand(10000, 20000));
        FrostNovaTimer.Reset(urand(2000, 4000));
        ArcaneExplosionTimer.Reset(1);

        ClearCastQueue();
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if (ArcaneExplosionTimer.Expired(diff))
        {
            if (m_creature->GetDistance(m_creature->GetVictim()) <= 5)
            {
                AddSpellToCast(m_creature->GetVictim(), SPELL_ARCANE_EXPLOSION);
                ArcaneExplosionTimer = urand(10000, 15000);
            }
        }

        if (FrostNovaTimer.Expired(diff))
        {
            if (m_creature->GetDistance(m_creature->GetVictim()) <= 8)
            {
                AddSpellToCast(m_creature->GetVictim(), SPELL_FROST_NOVA);
                FrostNovaTimer = urand(8000, 15000);
            }
        }

        if (WaterSpoutTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_WATER_SPOUT);
            WaterSpoutTimer = urand(15000, 25000);
        }

        
        if (FrostboltTimer.Expired(diff))
        {
            AddSpellToCast(m_creature->GetVictim(), SPELL_FROSTBOLT);
            FrostboltTimer = urand(3500, 4000);
        }

        CastNextSpellIfAnyAndReady();
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_coilfang_emissary(Creature *creature)
{
    return new npc_coilfang_emissaryAI (creature);
}

/*######
## npc_mortog_steamhead
######*/

bool GossipHello_npc_mortog_steamhead(Player *player, Creature *creature)
{
    if (creature->isVendor() && player->GetReputationMgr().GetRank(942) == REP_EXALTED)
        player->ADD_GOSSIP_ITEM(1, player->GetSession()->GetNpcOptionLocaleString(GOSSIP_TEXT_BROWSE_GOODS), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    player->SEND_GOSSIP_MENU(creature->GetNpcTextId(), creature->GetGUID());

    return true;
}

bool GossipSelect_npc_mortog_steamhead(Player *player, Creature *creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
    {
        player->SEND_VENDORLIST( creature->GetGUID() );
    }
    return true;
}

/*######
## npc_kayra_longmane
######*/

#define SAY_PROGRESS_1  -1000360
#define SAY_PROGRESS_2  -1000361
#define SAY_PROGRESS_3  -1000362
#define SAY_PROGRESS_4  -1000363
#define SAY_PROGRESS_5  -1000364
#define SAY_PROGRESS_6  -1000365

#define QUEST_EFU   9752
#define MOB_AMBUSH  18042

struct npc_kayra_longmaneAI : public npc_escortAI
{
    npc_kayra_longmaneAI(Creature* creature) : npc_escortAI(creature) {}

    bool Completed;

    void Reset()
    {
        me->setFaction(1660);
    }

    void EnterCombat(Unit* who){}

    void JustSummoned(Creature *summoned)
    {
        summoned->AI()->AttackStart(me);
        summoned->setFaction(14);
    }

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        switch(i)
        {
        case 0: DoScriptText(SAY_PROGRESS_1, me, player); break;
        case 5: DoScriptText(SAY_PROGRESS_2, me, player);
            me->SummonCreature(MOB_AMBUSH, -922.24, 5357.98, 17.93, 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
            me->SummonCreature(MOB_AMBUSH, -922.24, 5357.98, 17.93, 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
            break;
        case 6: DoScriptText(SAY_PROGRESS_3, me, player);
            me->SetWalk(false);
            break;
        case 18: DoScriptText(SAY_PROGRESS_4, me, player);
            me->SummonCreature(MOB_AMBUSH, -671.86, 5379.81, 22.12, 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
            me->SummonCreature(MOB_AMBUSH, -671.86, 5379.81, 22.12, 5.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
            break;
        case 19: me->SetWalk(false);
            DoScriptText(SAY_PROGRESS_5, me, player); break;
        case 26: DoScriptText(SAY_PROGRESS_6, me, player);
            if(player)
                player->GroupEventHappens(QUEST_EFU, me);
            break;
        }
    }
};

CreatureAI* GetAI_npc_kayra_longmane(Creature* creature)
{
    return new npc_kayra_longmaneAI(creature);
}

bool QuestAccept_npc_kayra_longmane(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_EFU)
    {
        if (npc_escortAI* pEscortAI = CAST_AI(npc_kayra_longmaneAI, creature->AI()))
            pEscortAI->Start(true, true, player->GetGUID(), quest);
        creature->setFaction(113);
    }
    return true;
}

/*######
## npc_baby_murloc
######*/

enum
{
    NPC_PURPLE_MURLOC          = 15357,
    NPC_GREEN_MURLOC           = 15360,
    NPC_BLUE_MURLOC            = 15356,
    NPC_PINK_MURLOC            = 15359,
    NPC_ORANGE_MURLOC          = 15361,

    SPELL_SING                 = 32041
};

struct Pos
{
    float x, y, z;
};

static Pos M[]=
{
    {1206.926f, 8139.298f, 19.70f},
    {1206.927f, 8158.908f, 19.51f},
    {1220.742f, 8093.757f, 18.120f},
    {1128.926f, 8137.008f, 20.664f},
    {1230.289f, 8156.368f, 18.40f},
    {1216.511f, 8188.199f, 18.70f}
};

struct npc_baby_murlocAI : public ScriptedAI
{
    npc_baby_murlocAI(Creature* creature) : ScriptedAI(creature) {}

    ObjectGuid PlayerGUID;
    Timer_UnCheked CheckTimer;
    Timer_UnCheked EndTimer;

    void Reset()
    {
        PlayerGUID = 0;
        CheckTimer.Reset(7000);
        EndTimer.Reset(10000);
        DoSummon();
        me->GetMotionMaster()->MovePoint(0, M[0].x, M[0].y, M[0].z);
        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DANCE);
    }

    void DoSummon()
    {
        me->SummonCreature(NPC_PURPLE_MURLOC, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000);
        me->SummonCreature(NPC_GREEN_MURLOC, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 9000);
        me->SummonCreature(NPC_BLUE_MURLOC, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 9000);
        me->SummonCreature(NPC_PINK_MURLOC, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 9000);
        me->SummonCreature(NPC_ORANGE_MURLOC, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 9000);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned->GetEntry() == NPC_PURPLE_MURLOC)
        {
            summoned->SetWalk(false);
            summoned->GetMotionMaster()->MovePoint(0, M[1].x, M[1].y, M[1].z);
            summoned->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_DANCE);
        }

        if (summoned->GetEntry() == NPC_GREEN_MURLOC)
        {
            summoned->SetWalk(false);
            summoned->GetMotionMaster()->MovePoint(0, M[2].x, M[2].y, M[2].z);
        }
        if (summoned->GetEntry() == NPC_BLUE_MURLOC)
        {
            summoned->SetWalk(false);
            summoned->GetMotionMaster()->MovePoint(0, M[3].x, M[3].y, M[3].z);
        }
        if (summoned->GetEntry() == NPC_ORANGE_MURLOC)
        {
            summoned->SetWalk(false);
            summoned->GetMotionMaster()->MovePoint(0, M[4].x, M[4].y, M[4].z);
        }
        else
        {
            if (summoned->GetEntry() == NPC_PINK_MURLOC)
            {
                summoned->SetWalk(false);
                summoned->GetMotionMaster()->MovePoint(0, M[5].x, M[5].y, M[5].z);
            }
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (((Player*)who)->GetQuestStatus(9816) == QUEST_STATUS_INCOMPLETE)
            {
                if (me->IsWithinDistInMap(((Player *)who), 15))
                {
                    PlayerGUID = who->GetObjectGuid();
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (CheckTimer.Expired(diff))
        {
            if (Player* player = me->GetPlayerInWorld(PlayerGUID))
            {
                me->SetFacingToObject(player);
                DoCast(me, SPELL_SING);

                if (Creature * Purple = GetClosestCreatureWithEntry(me, NPC_PURPLE_MURLOC, 20.0f))
                {
                    Purple->SetFacingToObject(player);
                    Purple->CastSpell(Purple, SPELL_SING, true);
                }
            }

            CheckTimer = 15000;
        }

        if (EndTimer.Expired(diff))
            if (Player* player = me->GetPlayerInWorld(PlayerGUID))
                player->AreaExploredOrEventHappens(9816);
    }
};

CreatureAI* GetAI_npc_baby_murloc(Creature* creature)
{
    return new npc_baby_murlocAI(creature);
}
/*###########
# npc_fhwoor
############*/
struct npc_fhwoorAI : public npc_escortAI
{
    npc_fhwoorAI(Creature *c) : npc_escortAI(c), Summons(me) {}

    SummonList Summons;
    uint32 PauseTimer;

    void Reset()
    {
        PauseTimer = 0;
    }

    void WaypointReached(uint32 i)
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (i)
        {
            case 0:
            {
                SetRun(true);
                break;
            }
            case 10:
            {
                me->Say(-1200581, LANG_UNIVERSAL, 0);
                SetEscortPaused(true);
                PauseTimer = 15000;
                SetRun(false);
                break;
            }
            case 18:
            {
                SetEscortPaused(true);
                PauseTimer = 10000;
                if(GameObject* go = FindGameObject(182082, 15, me))
                    go->DestroyForPlayer(pPlayer);
                break;
            }
            case 29:
            {
                me->Say(-1200582, LANG_UNIVERSAL, 0);
                SetRun(true);
                SetEscortPaused(true);
                PauseTimer = 10000;
                me->SummonCreature(18154, 206.462585, 8184.628906, 22.647760, 6.279466, TEMPSUMMON_TIMED_DESPAWN, 45000);
                me->SummonCreature(18088, 207.054123, 8189.833008, 21.466009, 6.168726, TEMPSUMMON_TIMED_DESPAWN, 45000);
                me->SummonCreature(18089, 205.491837, 8178.919922, 23.547256, 6.095684, TEMPSUMMON_TIMED_DESPAWN, 45000);
                break;
            }
            case 38:
            {
                me->Say(-1200583, LANG_UNIVERSAL, 0);
                if (pPlayer)
                    pPlayer->GroupEventHappens(9729, m_creature);
                break;
            }
        }
    }
   
    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/)
    {
        if (summon->GetEntry() == 18154)
            me->Say(-1200584, LANG_UNIVERSAL, 0);
    }

    void JustSummoned(Creature* summoned)
    {
        if (summoned)
            summoned->AI()->AttackStart(m_creature);
        Summons.Summon(summoned);
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        if(PauseTimer)
        {
            if(PauseTimer <= diff)
            {
                SetEscortPaused(false);
                PauseTimer = 0;
            } else PauseTimer -= diff;
        }
        DoMeleeAttackIfReady();
    }
};

bool QuestAccept_npc_fhwoor(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (pPlayer && quest->GetQuestId() == 9729)
    {
        pCreature->Say(-1200585, LANG_UNIVERSAL, pPlayer->GetGUID());
        if (npc_escortAI* pEscortAI = CAST_AI(npc_fhwoorAI, pCreature->AI()))
            pEscortAI->Start(true, true, pPlayer->GetGUID(), quest, true, true);
    }

    return true;
}

CreatureAI* GetAI_npc_fhwoor(Creature *_Creature)
{
    npc_fhwoorAI* thisAI = new npc_fhwoorAI(_Creature);

    thisAI->AddWaypoint(0, 210.772003, 8464.719727, 24.308901);
    thisAI->AddWaypoint(1, 182.962189, 8416.904297, 23.185053);
    thisAI->AddWaypoint(2, 165.650452, 8397.886719, 23.257469);
    thisAI->AddWaypoint(3, 177.736664, 8338.668945, 20.137768);
    thisAI->AddWaypoint(4, 189.480728, 8318.741211, 19.86161);
    thisAI->AddWaypoint(5, 184.241776, 8307.351563, 19.576712);
    thisAI->AddWaypoint(6, 170.258087, 8293.339844, 19.722382);
    thisAI->AddWaypoint(7, 191.593781, 8261.304688, 18.844393);
    thisAI->AddWaypoint(8, 217.924301, 8254.028320, 20.84317);
    thisAI->AddWaypoint(9, 248.916763, 8210.000977, 19.372663);
    thisAI->AddWaypoint(10, 285.294739, 8204.210938, 21.847265); // Contol point 15sec: "Take moment... get ready." -Set Walk.
    thisAI->AddWaypoint(11, 340.059265, 8176.282227, 18.284857);
    thisAI->AddWaypoint(12, 375.512634, 8190.774902, 23.286556);
    thisAI->AddWaypoint(13, 399.716705, 8183.301270, 18.258577);
    thisAI->AddWaypoint(14, 423.794128, 8158.826660, 18.851616);
    thisAI->AddWaypoint(15, 470.353424, 8153.452148, 21.678846);
    thisAI->AddWaypoint(16, 501.699951, 8147.590332, 20.112808);
    thisAI->AddWaypoint(17, 541.115723, 8159.491211, 23.121338);
    thisAI->AddWaypoint(18, 559.124023, 8158.987305, 23.750059); // CP 10s, remove object 182082, return back
    thisAI->AddWaypoint(19, 533.315918, 8139.716797, 22.194769);
    thisAI->AddWaypoint(20, 514.343384, 8134.147461, 20.54624);
    thisAI->AddWaypoint(21, 497.244598, 8138.025879, 21.027016);
    thisAI->AddWaypoint(22, 475.564667, 8127.246094, 22.60535);
    thisAI->AddWaypoint(23, 448.807678, 8128.781250, 20.882103);
    thisAI->AddWaypoint(24, 431.998444, 8120.838379, 18.179377);
    thisAI->AddWaypoint(25, 393.614655, 8120.073242, 17.965488);
    thisAI->AddWaypoint(26, 353.548889, 8105.858398, 17.793951);
    thisAI->AddWaypoint(27, 312.205841, 8147.023438, 21.130239);
    thisAI->AddWaypoint(28, 271.260376, 8162.994629, 17.800726);
    thisAI->AddWaypoint(29, 220.373917, 8181.403320, 19.722895); // CP, : "Uh oh..." 15sec -SetRun
    thisAI->AddWaypoint(30, 197.996567, 8218.105469, 23.084532);
    thisAI->AddWaypoint(31, 196.537415, 8255.055664, 19.707302 );
    thisAI->AddWaypoint(32, 172.059769, 8295.379883, 19.16762);
    thisAI->AddWaypoint(33, 191.426926, 8321.498047, 19.311628);
    thisAI->AddWaypoint(34, 174.402786, 8349.534180, 19.635693);
    thisAI->AddWaypoint(35, 171.063019, 8376.228516, 19.239479);
    thisAI->AddWaypoint(36, 209.969711, 8421.449219, 19.189375);
    thisAI->AddWaypoint(37, 205.011429, 8456.572266, 25.012835);
    thisAI->AddWaypoint(38, 231.595413, 8479.372070, 17.771519, 1.827039); // "Fhwoor do good"

    return (CreatureAI*)thisAI;
}


struct npc_quest_9720AI : public ScriptedAI
{
    npc_quest_9720AI(Creature* creature) : ScriptedAI(creature) {}

    void Reset()
    {
    }

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(spell->Id == 31736)
        {
            float x,y,z;
            caster->GetNearPoint(x, y, z, 5);
            if(Creature* guard = me->SummonCreature(18340, x, y, z, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30000))
            {
                guard->Say(-1200586, LANG_UNIVERSAL, 0);
                guard->AI()->AttackStart(caster);
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;
    }
};

CreatureAI* GetAI_npc_quest_9720(Creature *creature)
{
    return new npc_quest_9720AI (creature);
}

struct npc_18046AI : public ScriptedAI
{
    npc_18046AI(Creature* creature) : ScriptedAI(creature) {}

    Timer ShieldWallTimer;
    Timer ShieldChargeTimer;
    Timer MortalStrikeTimer;
    Timer ForcefulStrikeTimer;
    bool ChangedEquip;

    void Reset()
    {
        me->LoadEquipment(18046, true);
        ShieldWallTimer.Reset(urand(10000, 20000));
        ShieldChargeTimer.Reset(urand(5000, 10000));
        MortalStrikeTimer.Reset(urand(5000, 10000));
        ForcefulStrikeTimer.Reset(urand(10000, 20000));
        ChangedEquip = false;
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(me->GetHealthPercent() > 50)
        {
            if (ShieldWallTimer.Expired(diff))
            {
                DoCast(me, 15062);
                ShieldWallTimer = urand(45000, 55000);
            }

            if (ShieldChargeTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), 35472);
                ShieldChargeTimer = urand(10000, 20000);
            }
        }
        else
        {
            if(!ChangedEquip)
            {
                me->LoadEquipment(1804601, true);
                ChangedEquip = true;
            }

            if (MortalStrikeTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), 16856);
                MortalStrikeTimer = urand(10000, 20000);
            }

            if (ForcefulStrikeTimer.Expired(diff))
            {
                DoCast(me->GetVictim(), 35473);
                ForcefulStrikeTimer = urand(10000, 20000);
            }
        }
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_18046(Creature *creature)
{
    return new npc_18046AI (creature);
}

struct npc_20061AI : public Scripted_NoMovementAI
{
    npc_20061AI(Creature* creature) : Scripted_NoMovementAI(creature) { Phase = 0; }

    Timer ChangeOriTimer;
    Timer CastBlockTimer;
    Timer LastSpellTimer;
    uint8 Phase;

    void Reset()
    {
        ChangeOriTimer.Reset(500);
        CastBlockTimer.Reset(0);
        LastSpellTimer.Reset(0);
        me->CastSpell(me, 34872, false);
    }

    void EnterCombat(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if (ChangeOriTimer.Expired(diff))
        {
            switch(Phase)
            {
                case 0:
                    ChangeOriTimer = 0;
                    me->SetOrientation(0.785);
                    me->SetFacingTo(0.785);
                    CastBlockTimer = 500;
                    break;
                case 1:
                    ChangeOriTimer = 0;
                    me->SetOrientation(1.57);
                    me->SetFacingTo(1.57);
                    CastBlockTimer = 500;
                    break;
                case 2:
                    ChangeOriTimer = 0;
                    me->SetOrientation(2.355);
                    me->SetFacingTo(2.355);
                    CastBlockTimer = 500;
                    break;
                case 3:
                    ChangeOriTimer = 0;
                    me->SetOrientation(3.14);
                    me->SetFacingTo(3.14);
                    CastBlockTimer = 400;
                    break;
                default: break;
            }
        }

        if(CastBlockTimer.Expired(diff))
        {
            CastBlockTimer = 0;
            me->CastSpell(me, 34746, false);
            me->CastSpell(me, 34740, false);
            Phase++;
            if(Phase > 3)
            {
                ChangeOriTimer = 0;
                LastSpellTimer = 100;
            }
            else
                ChangeOriTimer = 500;
        }

        if(LastSpellTimer.Expired(diff))
        {
            me->CastSpell(me, 34779, false);
            LastSpellTimer = 0;
        }

        if(!UpdateVictim())
            return;
    }
};
CreatureAI* GetAI_npc_20061(Creature *creature)
{
    return new npc_20061AI (creature);
}

struct npc_18123AI : public ScriptedAI
{
    npc_18123AI(Creature* creature) : ScriptedAI(creature) {}

    uint64 WrektSlaveGUID;
    Timer KickTimer;

    void Reset()
    {
        WrektSlaveGUID = 0;
        KickTimer.Reset(1000);
    }

    void JustDied(Unit* Killer)
    {
        if (WrektSlaveGUID)
        {
            if (Unit* wrektslave = Unit::GetUnit((*me), WrektSlaveGUID))
                if (wrektslave->isAlive())
                    Killer->Kill(wrektslave, false);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if(eventType == AI_EVENT_CUSTOM_EVENTAI_A)
            me->AI()->AttackStart(invoker);
    }

    void EnterCombat(Unit *who)
    {
        if(Unit* wrektslave = FindCreature(18123, 15, me))
        {
            if ((me->GetGUID() != wrektslave->GetGUID()) && !me->HasAura(41363, 0) && !wrektslave->HasAura(41363, 0))
            {
                wrektslave->CastSpell(me, 41363, false);
                me->CastSpell(wrektslave, 41363, false);
                WrektSlaveGUID = wrektslave->GetGUID();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(KickTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 11978, false);
            KickTimer = 4000;
        }
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_18123(Creature *creature)
{
    return new npc_18123AI (creature);
}


struct npc_18122AI : public ScriptedAI
{
    npc_18122AI(Creature* creature) : ScriptedAI(creature) {}

    uint64 DreghoodDrudgeGUID;
    Timer KickTimer;

    void Reset()
    {
        DreghoodDrudgeGUID = 0;
        KickTimer.Reset(1000);
    }

    void JustDied(Unit* Killer)
    {
        if (DreghoodDrudgeGUID)
        {
            if (Unit* dreghooddrudge = Unit::GetUnit((*me), DreghoodDrudgeGUID))
                if (dreghooddrudge->isAlive())
                    Killer->Kill(dreghooddrudge, false);
        }
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /*miscValue*/)
    {
        if(eventType == AI_EVENT_CUSTOM_EVENTAI_A)
            me->AI()->AttackStart(invoker);
    }

    void EnterCombat(Unit *who)
    {
        if(Unit* dreghooddrudge = FindCreature(18122, 15, me))
        {
            if ((me->GetGUID() != dreghooddrudge->GetGUID()) && !me->HasAura(41363, 0) && !dreghooddrudge->HasAura(41363, 0))
            {
                dreghooddrudge->CastSpell(me, 41363, false);
                me->CastSpell(dreghooddrudge, 41363, false);
                DreghoodDrudgeGUID = dreghooddrudge->GetGUID();
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        if(KickTimer.Expired(diff))
        {
            me->CastSpell(me->GetVictim(), 11978, false);
            KickTimer = 4000;
        }
        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_18122(Creature *creature)
{
    return new npc_18122AI (creature);
}


struct npc_17973AI : public ScriptedAI
{
    npc_17973AI(Creature* creature) : ScriptedAI(creature) {}

    Timer ResetBoolTimer;
    bool reset;

    void Reset()
    {
        reset = false;
        ResetBoolTimer.Reset(0);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if(!reset)
        {
            if(who->GetTypeId() == TYPEID_PLAYER)
            {
                if(who->HasUnitState(UNIT_STAT_TAXI_FLIGHT))
                {
                    if (((Player*)who)->GetQuestStatus(9718) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (me->IsWithinDistInMap(who, 3))
                        {
                            switch(me->GetAreaId())
                            {
                                case 3819:
                                    if(Creature* c = me->GetMap()->GetCreatureById(17841))
                                        c->Whisper(-1200587, who->GetObjectGuid());
                                    reset = true;
                                    ResetBoolTimer = 1000;
                                    break;
                                case 3818:
                                    if(Creature* c = me->GetMap()->GetCreatureById(17841))
                                        c->Whisper(-1200588, who->GetObjectGuid());
                                    reset = true;
                                    ResetBoolTimer = 2000;
                                    break;
                                case 3653:
                                    if(Creature* c = me->GetMap()->GetCreatureById(17841))
                                        c->Whisper(-1200589, who->GetObjectGuid());
                                    reset = true;
                                    ResetBoolTimer = 2000;
                                    break;
                                case 3648:
                                    if(Creature* c = me->GetMap()->GetCreatureById(17841))
                                        c->Whisper(-1200590, who->GetObjectGuid());
                                    reset = true;
                                    ResetBoolTimer = 2000;
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(ResetBoolTimer.Expired(diff))
        {
            reset = false;
            ResetBoolTimer = 0;
        }
    }
};
CreatureAI* GetAI_npc_17973(Creature *creature)
{
    return new npc_17973AI (creature);
}

struct npc_20279AI : public ScriptedAI
{
    npc_20279AI(Creature* creature) : ScriptedAI(creature) {}

    Timer FeignDeathTimer;
    Timer FDRemoveTimer;
    bool FeignDeath;
    bool Enraged;

    void Reset()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        me->SetRooted(false);
        FeignDeathTimer.Reset(5000);
        FDRemoveTimer.Reset(0);
        FeignDeath = false;
        Enraged = false;
    }

    void JustDied(Unit* Killer)
    {
    }

    void DamageTaken(Unit* done_by, uint32 & damage)
    {
        if (FeignDeath)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(false);
            FeignDeath = false;
            if (me->GetVictim())
                me->CastSpell(me->GetVictim(), 35385, false);
            FDRemoveTimer = 0;
            DoScriptText(-1811024, me);
        }
    }
    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (FeignDeathTimer.Expired(diff))
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(true);
            FeignDeathTimer = 0;
            FeignDeath = true;
            FDRemoveTimer = 4000;
        }

        if (!FeignDeath)
        {
            if (!Enraged)
            {
                if (me->GetHealthPercent() <= 20)
                {
                    me->CastSpell(me, 3019, false);
                    DoScriptText(-1564054, me);
                    Enraged = true;
                }
            }
            DoMeleeAttackIfReady();
        }
        else
        {
            if (FDRemoveTimer.Expired(diff))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                me->SetRooted(false);
                FeignDeath = false;
                if (me->GetVictim())
                    me->CastSpell(me->GetVictim(), 35385, false);
                FDRemoveTimer = 0;
                DoScriptText(-1811024, me);
            }
        }
    }
};
CreatureAI* GetAI_npc_20279(Creature *creature)
{
    return new npc_20279AI(creature);
}

struct npc_20280AI : public ScriptedAI
{
    npc_20280AI(Creature* creature) : ScriptedAI(creature) {}

    Timer FeignDeathTimer;
    Timer FDRemoveTimer;
    Timer TrampleTimer;
    bool FeignDeath;
    bool Enraged;

    void Reset()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        me->SetRooted(false);
        FeignDeathTimer.Reset(5000);
        FDRemoveTimer.Reset(0);
        FeignDeath = false;
        Enraged = false;
        TrampleTimer.Reset(12000);
    }

    void JustDied(Unit* Killer)
    {
    }

    void DamageTaken(Unit* done_by, uint32 & damage)
    {
        if (FeignDeath)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(false);
            FeignDeath = false;
            if (me->GetVictim())
                me->CastSpell(me->GetVictim(), 35385, false);
            FDRemoveTimer = 0;
            DoScriptText(-1811024, me);
        }
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (FeignDeathTimer.Expired(diff))
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(true);
            FeignDeathTimer = 0;
            FeignDeath = true;
            FDRemoveTimer = 4000;
        }

        if (!FeignDeath)
        {
            if (!Enraged)
            {
                if (me->GetHealthPercent() <= 30)
                {
                    me->CastSpell(me, 3019, false);
                    DoScriptText(-1564054, me);
                    Enraged = true;
                }
            }

            if (TrampleTimer.Expired(diff))
            {
                me->CastSpell(me->GetVictim(), 5568, false);
                TrampleTimer = 12000;
            }

            DoMeleeAttackIfReady();
        }
        else
        {
            if (FDRemoveTimer.Expired(diff))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                me->SetRooted(false);
                FeignDeath = false;
                if (me->GetVictim())
                    me->CastSpell(me->GetVictim(), 35385, false);
                FDRemoveTimer = 0;
                DoScriptText(-1811024, me);
            }
        }
    }
};
CreatureAI* GetAI_npc_20280(Creature *creature)
{
    return new npc_20280AI(creature);
}

struct npc_19730AI : public ScriptedAI
{
    npc_19730AI(Creature* creature) : ScriptedAI(creature) {}

    Timer FeignDeathTimer;
    Timer FDRemoveTimer;
    Timer HypnoticGaze;
    bool FeignDeath;

    void Reset()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        me->SetRooted(false);
        FeignDeathTimer.Reset(5000);
        FDRemoveTimer.Reset(0);
        FeignDeath = false;
        HypnoticGaze.Reset(12000);
    }

    void JustDied(Unit* Killer)
    {
    }

    void DamageTaken(Unit* done_by, uint32 & damage)
    {
        if (FeignDeath)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(false);
            FeignDeath = false;
            if (me->GetVictim())
                me->CastSpell(me->GetVictim(), 35385, false);
            FDRemoveTimer = 0;
            DoScriptText(-1811024, me);
        }
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!me->IsInCombat() || me->IsInEvadeMode())
            return;

        if (FeignDeathTimer.Expired(diff))
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(true);
            FeignDeathTimer = 0;
            FeignDeath = true;
            FDRemoveTimer = 4000;
        }

        if (!FeignDeath)
        {
            if (HypnoticGaze.Expired(diff))
            {
                me->CastSpell(me->GetVictim(), 35313, false);
                HypnoticGaze = 12000;
            }

            DoMeleeAttackIfReady();
        }
        else
        {
            if (FDRemoveTimer.Expired(diff))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                me->SetRooted(false);
                FeignDeath = false;
                if (me->GetVictim())
                    me->CastSpell(me->GetVictim(), 35385, false);
                FDRemoveTimer = 0;
                DoScriptText(-1811024, me);
            }
        }
    }
};
CreatureAI* GetAI_npc_19730(Creature *creature)
{
    return new npc_19730AI(creature);
}

struct npc_19729AI : public ScriptedAI
{
    npc_19729AI(Creature* creature) : ScriptedAI(creature) {}

    Timer FeignDeathTimer;
    Timer FDRemoveTimer;
    bool FeignDeath;

    void Reset()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        me->SetRooted(false);
        FeignDeathTimer.Reset(5000);
        FDRemoveTimer.Reset(0);
        FeignDeath = false;
    }

    void JustDied(Unit* Killer)
    {
    }

    void DamageTaken(Unit* done_by, uint32 & damage)
    {
        if (FeignDeath)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(false);
            FeignDeath = false;
            if (me->GetVictim())
                me->CastSpell(me->GetVictim(), 35385, false);
            FDRemoveTimer = 0;
            DoScriptText(-1811024, me);
        }
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (FeignDeathTimer.Expired(diff))
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(true);
            FeignDeathTimer = 0;
            FeignDeath = true;
            FDRemoveTimer = 4000;
        }

        if (!FeignDeath)
        {
            DoMeleeAttackIfReady();
        }
        else
        {
            if (FDRemoveTimer.Expired(diff))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                me->SetRooted(false);
                FeignDeath = false;
                if (me->GetVictim())
                    me->CastSpell(me->GetVictim(), 35385, false);
                FDRemoveTimer = 0;
                DoScriptText(-1811024, me);
            }
        }
    }
};
CreatureAI* GetAI_npc_19729(Creature *creature)
{
    return new npc_19729AI(creature);
}


struct npc_20924AI : public ScriptedAI
{
    npc_20924AI(Creature* creature) : ScriptedAI(creature) {}

    Timer FDRemoveTimer;
    bool FeignDeath;
    bool SoftenOne;
    bool SoftenTwo;

    void Reset()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        me->SetRooted(false);
        FDRemoveTimer.Reset(0);
        FeignDeath = false;
        SoftenOne = false;
        SoftenTwo = false;
    }

    void JustDied(Unit* Killer)
    {
    }

    void DamageTaken(Unit* done_by, uint32 & damage)
    {
        if (FeignDeath)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
            me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
            me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
            me->SetRooted(false);
            FeignDeath = true;
            FDRemoveTimer = 0;
        }
    }

    void EnterCombat(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (!FeignDeath)
        {
            if (me->GetHealthPercent() <= 30)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                me->SetRooted(true);
                FeignDeath = true;
                FDRemoveTimer = 4000;
            }
        }

        if (!FeignDeath)
        {
            if (!SoftenOne)
            {
                if (me->GetHealthPercent() <= 80)
                {
                    me->CastSpell(me, 37590, false);
                    SoftenOne = true;
                }
            }
            if (!SoftenTwo)
            {
                if (me->GetHealthPercent() <= 50)
                {
                    me->CastSpell(me, 37590, false);
                    SoftenTwo = true;
                }
            }
            DoMeleeAttackIfReady();
        }
        else
        {
            if (FDRemoveTimer.Expired(diff))
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNKNOWN6 | UNIT_FLAG_DISABLE_ROTATE);
                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
                me->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                me->SetRooted(false);
                FeignDeath = true;
                FDRemoveTimer = 0;
            }
        }
    }
};

CreatureAI* GetAI_npc_20924(Creature *creature)
{
    return new npc_20924AI(creature);
}

/*######
## AddSC
######*/

void AddSC_zangarmarsh()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_coilfang_emissary";
    newscript->GetAI = &GetAI_npc_coilfang_emissary;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_chieftain_mummaki";
    newscript->GetAI = &GetAI_npc_chieftain_mummaki;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_20924";
    newscript->GetAI = &GetAI_npc_20924;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19729";
    newscript->GetAI = &GetAI_npc_19729;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_19730";
    newscript->GetAI = &GetAI_npc_19730;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_20280";
    newscript->GetAI = &GetAI_npc_20280;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_20279";
    newscript->GetAI = &GetAI_npc_20279;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_17973";
    newscript->GetAI = &GetAI_npc_17973;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18122";
    newscript->GetAI = &GetAI_npc_18122;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18123";
    newscript->GetAI = &GetAI_npc_18123;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_20061";
    newscript->GetAI = &GetAI_npc_20061;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_18046";
    newscript->GetAI = &GetAI_npc_18046;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npcs_ashyen_and_keleth";
    newscript->pGossipHello =  &GossipHello_npcs_ashyen_and_keleth;
    newscript->pGossipSelect = &GossipSelect_npcs_ashyen_and_keleth;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_cooshcoosh";
    newscript->GetAI = &GetAI_npc_cooshcoosh;
    newscript->pGossipHello =  &GossipHello_npc_cooshcoosh;
    newscript->pGossipSelect = &GossipSelect_npc_cooshcoosh;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_elder_kuruti";
    newscript->GetAI = &GetAI_npc_elder_kuruti;
    newscript->pGossipHello =  &GossipHello_npc_elder_kuruti;
    newscript->pGossipSelect = &GossipSelect_npc_elder_kuruti;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_mortog_steamhead";
    newscript->pGossipHello =  &GossipHello_npc_mortog_steamhead;
    newscript->pGossipSelect = &GossipSelect_npc_mortog_steamhead;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_kayra_longmane";
    newscript->GetAI = &GetAI_npc_kayra_longmane;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_kayra_longmane;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_baby_murloc";
    newscript->GetAI = &GetAI_npc_baby_murloc;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_fhwoor";
    newscript->GetAI = &GetAI_npc_fhwoor;
    newscript->pQuestAcceptNPC = &QuestAccept_npc_fhwoor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_quest_9720";
    newscript->GetAI = &GetAI_npc_quest_9720;
    newscript->RegisterSelf();
}
